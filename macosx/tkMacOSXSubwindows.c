/*
 * tkMacOSXSubwindows.c --
 *
 *	Implements subwindows for the macintosh version of Tk.
 *
 * Copyright © 1995-1997 Sun Microsystems, Inc.
 * Copyright © 2001-2009 Apple Inc.
 * Copyright © 2006-2009 Daniel A. Steffen <das@users.sourceforge.net>
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tkMacOSXPrivate.h"
#include "tkMacOSXDebug.h"
#include "tkMacOSXWm.h"
#include "tkMacOSXConstants.h"

/*
#ifdef TK_MAC_DEBUG
#define TK_MAC_DEBUG_CLIP_REGIONS
#endif
*/

/*
 * Prototypes for functions used only in this file.
 */

static void		MoveResizeWindow(MacDrawable *macWin);
static void		GenerateConfigureNotify(TkWindow *winPtr,
			    int includeWin);
static void		UpdateOffsets(TkWindow *winPtr, int deltaX,
			    int deltaY);
static void		NotifyVisibility(TkWindow *winPtr, XEvent *eventPtr);


/*
 *----------------------------------------------------------------------
 *
 * XDestroyWindow --
 *
 *	Deallocates the given X Window.
 *
 * Results:
 *	The window id is returned.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
XDestroyWindow(
    TCL_UNUSED(Display *),		/* Display. */
    Window window)		/* Window. */
{
    MacDrawable *macWin = (MacDrawable *)window;
    TKContentView *view = (TKContentView *)TkMacOSXGetNSViewForDrawable(macWin);
    //fprintf(stderr, "XDestroyWindow: %s with parent %s\n",
    //	    Tk_PathName(macWin->winPtr),
    //	    Tk_PathName(macWin->winPtr->parentPtr));

    /*
     * Remove any dangling pointers that may exist if the window we are
     * deleting is being tracked by the grab code.
     */

    TkMacOSXSelDeadWindow(macWin->winPtr);
    TkPointerDeadWindow(macWin->winPtr);
    macWin->toplevel->referenceCount--;

    if (!Tk_IsTopLevel(macWin->winPtr)) {
	if (macWin->winPtr->parentPtr != NULL) {
	    TkMacOSXInvalClipRgns((Tk_Window)macWin->winPtr->parentPtr);
	    Tcl_CancelIdleCall(TkMacOSXRedrawViewIdleTask, (void *) view);
	    Tcl_DoWhenIdle(TkMacOSXRedrawViewIdleTask, (void *) view);
	}
	if (macWin->visRgn) {
	    CFRelease(macWin->visRgn);
	    macWin->visRgn = NULL;
	}
	if (macWin->aboveVisRgn) {
	    CFRelease(macWin->aboveVisRgn);
	    macWin->aboveVisRgn = NULL;
	}
	if (macWin->drawRgn) {
	    CFRelease(macWin->drawRgn);
	    macWin->drawRgn = NULL;
	}

	if (macWin->toplevel->referenceCount == 0) {
	    ckfree(macWin->toplevel);
	}
	macWin->winPtr->privatePtr = NULL;
	ckfree(macWin);
	return Success;
    }
    if (macWin->visRgn) {
	CFRelease(macWin->visRgn);
	macWin->visRgn = NULL;
    }
    if (macWin->aboveVisRgn) {
	CFRelease(macWin->aboveVisRgn);
	macWin->aboveVisRgn = NULL;
    }
    if (macWin->drawRgn) {
	CFRelease(macWin->drawRgn);
	macWin->drawRgn = NULL;
    }
    macWin->view = nil;
    macWin->winPtr->privatePtr = NULL;

    /*
     * Delay deletion of a toplevel data structure until all children have
     * been deleted.
     */

    if (macWin->toplevel->referenceCount == 0) {
	ckfree(macWin->toplevel);
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XMapWindow --
 *
 *	This X11 stub maps the given X11 Window but does not update any of
 *      the Tk structures describing the window.  Tk applications should
 *	never call this directly, but it is called by Tk_MapWindow and
 *      Tk_WmMapWindow.
 *
 * Results:
 *	Always returns Success or BadWindow.
 *
 * Side effects:
 *	The subwindow or toplevel may appear on the screen.  VisibilityNotify
 *      events are generated.
 *
 *
 *----------------------------------------------------------------------
 */

int
XMapWindow(
    Display *display,		/* Display. */
    Window window)		/* Window. */
{
    if (!window) {
	return BadWindow;
    }
    MacDrawable *macWin = (MacDrawable *)window;
    static Bool initialized = NO;
    NSPoint mouse = [NSEvent mouseLocation];
    int x = mouse.x, y = TkMacOSXZeroScreenHeight() - mouse.y;
    //fprintf(stderr, "XMapWindow: %s\n", Tk_PathName(macWin->winPtr));

    /*
     * Under certain situations it's possible for this function to be called
     * before the toplevel window it's associated with has actually been
     * mapped. In that case we need to create the real Macintosh window now as
     * this function as well as other X functions assume that the portPtr is
     * valid.
     */

    if (!TkMacOSXHostToplevelExists(macWin->toplevel->winPtr)) {
	TkMacOSXMakeRealWindowExist(macWin->toplevel->winPtr);
    }

    TkWindow *winPtr = macWin->winPtr;
    NSWindow *win = TkMacOSXGetNSWindowForDrawable(window);
    TKContentView *view = [win contentView];
    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(winPtr)) {
	if (!Tk_IsEmbedded(winPtr)) {

	    /*
	     * We want to activate Tk when a toplevel is mapped but we can't
	     * always specify activateIgnoringOtherApps to be YES.  This is
	     * because during Tk initialization the root window is mapped
	     * before applicationDidFinishLaunching returns. Forcing the app to
	     * activate too early can make the menu bar unresponsive.
	     */

	    TkMacOSXApplyWindowAttributes(winPtr, win);
	    [win setExcludedFromWindowsMenu:NO];
	    [NSApp activateIgnoringOtherApps:initialized];
	    if (initialized) {
		if ([win canBecomeKeyWindow]) {
		    [win makeKeyAndOrderFront:NSApp];
		    [NSApp setTkEventTarget:TkMacOSXGetTkWindow(win)];
		} else {
		    [win orderFrontRegardless];
		}

		/*
		 * Delay for up to 20 milliseconds until the toplevel has
		 * actually become the highest toplevel.  This is to ensure
		 * that the Visibility event occurs after the toplevel is
		 * visible.
		 */

		for (int try = 0; try < 20; try++) {
		    if ([[NSApp orderedWindows] firstObject] == win) {
			break;
		    }
		    [NSThread sleepForTimeInterval:.001];
		}
	    }

	    /*
	     * Call Tk_UpdatePointer to tell Tk whether the pointer is in the
	     * new window.
	     */

	    NSPoint viewLocation = [view convertPoint:mouse fromView:nil];
	    if (NSPointInRect(viewLocation, NSInsetRect([view bounds], 2, 2))) {
		Tk_UpdatePointer((Tk_Window) winPtr, x, y, [NSApp tkButtonState]);
	    }
	} else {
	    Tk_Window contWinPtr = Tk_GetOtherWindow((Tk_Window)winPtr);

	    /*
	     * Rebuild the container's clipping region and display
	     * the window.
	     */

	    TkMacOSXInvalClipRgns(contWinPtr);
	}
    } else {

	/*
	 * For non-toplevel windows, rebuild the parent's clipping region
	 * and redisplay the window.
	 */

	TkMacOSXInvalClipRgns((Tk_Window)winPtr->parentPtr);
    }

    /*
     * If a geometry manager is mapping hundreds of windows we
     * don't want to redraw the view hundreds of times, so do
     * it in an idle task.
     */

    Tcl_CancelIdleCall(TkMacOSXRedrawViewIdleTask, (void *) view);
    Tcl_DoWhenIdle(TkMacOSXRedrawViewIdleTask, (void *) view);

    /*
     * Generate VisibilityNotify events for window and all mapped children.
     */

    if (initialized) {
	XEvent event;
	event.xany.send_event = False;
	event.xany.display = display;
	event.xvisibility.type = VisibilityNotify;
	event.xvisibility.state = VisibilityUnobscured;
	NotifyVisibility(winPtr, &event);
    } else {
	initialized = YES;
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * NotifyVisibility --
 *
 *	Helper for XMapWindow().  Generates VisibilityNotify events
 *      for the window and all of its descendants.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	VisibilityNotify events are queued.
 *
 *----------------------------------------------------------------------
 */

static void
NotifyVisibility(
    TkWindow *winPtr,
    XEvent *eventPtr)
{
    if (winPtr->atts.event_mask & VisibilityChangeMask) {
	eventPtr->xany.serial = LastKnownRequestProcessed(winPtr->display);
	eventPtr->xvisibility.window = winPtr->window;
	Tk_QueueWindowEvent(eventPtr, TCL_QUEUE_TAIL);
    }
    for (winPtr = winPtr->childList; winPtr != NULL;
	    winPtr = winPtr->nextPtr) {
	if (winPtr->flags & TK_MAPPED) {
	    NotifyVisibility(winPtr, eventPtr);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * XUnmapWindow --
 *
 *	This X11 stub maps the given X11 Window but does not update any of
 *	The Tk structures describing the window.  Tk applications should
 *	never call this directly, but it is called by Tk_UnmapWindow and
 *      Tk_WmUnmapWindow.
 *
 * Results:
 *	Always returns Success or BadWindow.
 *
 * Side effects:
 *	The subwindow or toplevel may be removed from the screen.
 *
 *----------------------------------------------------------------------
 */

int
XUnmapWindow(
    Display *display,		/* Display. */
    Window window)		/* Window. */
{
    MacDrawable *macWin = (MacDrawable *)window;
    TkWindow *winPtr = macWin->winPtr;
    NSWindow *win = TkMacOSXGetNSWindowForDrawable(window);

    if (!window) {
	return BadWindow;
    }
    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(winPtr)) {
	if (!Tk_IsEmbedded(winPtr) &&
	    winPtr->wmInfoPtr->hints.initial_state!=IconicState) {
	    [win setExcludedFromWindowsMenu:YES];
	    [win orderOut:NSApp];
	    if ([win isKeyWindow]) {

		/*
		 * If we are unmapping the key window then we need to make sure
		 * that a new key window is assigned, if possible.  This is
		 * supposed to happen when a key window is ordered out, but as
		 * noted in tkMacOSXWm.c this does not happen, in spite of
		 * Apple's claims to the contrary.
		 */

		for (NSWindow *w in [NSApp orderedWindows]) {
		    TkWindow *winPtr2 = TkMacOSXGetTkWindow(w);
		    WmInfo *wmInfoPtr;

		    BOOL isOnScreen;

		    if (!winPtr2 || !winPtr2->wmInfoPtr) {
			continue;
		    }
		    wmInfoPtr = winPtr2->wmInfoPtr;
		    isOnScreen = (wmInfoPtr->hints.initial_state != IconicState &&
				  wmInfoPtr->hints.initial_state != WithdrawnState);
		    if (w != win && isOnScreen && [w canBecomeKeyWindow]) {
			[w makeKeyAndOrderFront:NSApp];
			[NSApp setTkEventTarget:TkMacOSXGetTkWindow(win)];
			break;
		    }
		}
	    }
	}
	TkMacOSXInvalClipRgns((Tk_Window)winPtr);
    } else {

	/*
	 * Rebuild the clip regions for the parent so it will be allowed
	 * to draw in the space from which this subwindow was removed and then
	 * redraw the window.
	 */

	TkMacOSXInvalClipRgns((Tk_Window)winPtr->parentPtr);
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XResizeWindow --
 *
 *	Resize a given X window. See X windows documentation for further
 *	details.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
XResizeWindow(
    Display *display,		/* Display. */
    Window window,		/* Window. */
    unsigned int width,
    unsigned int height)
{
    MacDrawable *macWin = (MacDrawable *)window;

    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(macWin->winPtr) && !Tk_IsEmbedded(macWin->winPtr)) {
	TKWindow *w = (TKWindow *)macWin->winPtr->wmInfoPtr->window;

	if (w) {
	    if ([w styleMask] & NSFullScreenWindowMask) {
		[(TKWindow *)w tkLayoutChanged];
	    } else {
		NSRect r = [w contentRectForFrameRect:[w frame]];
		r.origin.y += r.size.height - height;
		r.size.width = width;
		r.size.height = height;
		[w setFrame:[w frameRectForContentRect:r] display:NO];
	    }
	}
    } else {
	MoveResizeWindow(macWin);
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XMoveResizeWindow --
 *
 *	Move or resize a given X window. See X windows documentation for
 *	further details.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
XMoveResizeWindow(
    Display *display,		/* Display. */
    Window window,		/* Window. */
    int x, int y,
    unsigned int width,
    unsigned int height)
{
    MacDrawable *macWin = (MacDrawable *)window;

    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(macWin->winPtr) && !Tk_IsEmbedded(macWin->winPtr)) {
	NSWindow *w = macWin->winPtr->wmInfoPtr->window;

	if (w) {
	    /*
	     * We explicitly convert everything to doubles so we don't get
	     * surprised (again) by what happens when you do arithmetic with
	     * unsigned ints.
	     */

	    CGFloat X = (CGFloat) x;
	    CGFloat Y = (CGFloat) y;
	    CGFloat Width = (CGFloat) width;
	    CGFloat Height = (CGFloat) height;
	    CGFloat XOff = (CGFloat) macWin->winPtr->wmInfoPtr->xInParent;
	    CGFloat YOff = (CGFloat) macWin->winPtr->wmInfoPtr->yInParent;
	    NSRect r = NSMakeRect(
		    X + XOff, TkMacOSXZeroScreenHeight() - Y - YOff - Height,
		    Width, Height);

	    [w setFrame:[w frameRectForContentRect:r] display:NO];
	}
    } else {
	MoveResizeWindow(macWin);
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XMoveWindow --
 *
 *	Move a given X window. See X windows documentation for further details.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
XMoveWindow(
    Display *display,		/* Display. */
    Window window,		/* Window. */
    int x, int y)
{
    MacDrawable *macWin = (MacDrawable *)window;

    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(macWin->winPtr) && !Tk_IsEmbedded(macWin->winPtr)) {
	NSWindow *w = macWin->winPtr->wmInfoPtr->window;

	if (w) {
	    [w setFrameTopLeftPoint: NSMakePoint(
		    x, TkMacOSXZeroScreenHeight() - y)];
	}
    } else {
	MoveResizeWindow(macWin);
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * MoveResizeWindow --
 *
 *	Helper proc for XResizeWindow, XMoveResizeWindow and XMoveWindow.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
MoveResizeWindow(
    MacDrawable *macWin)
{
    int deltaX = 0, deltaY = 0, parentBorderwidth = 0;
    MacDrawable *macParent = NULL;
    NSWindow *macWindow = TkMacOSXGetNSWindowForDrawable((Drawable)macWin);

    /*
     * Find the Parent window, for an embedded window it will be its container.
     */

    if (Tk_IsEmbedded(macWin->winPtr)) {
	TkWindow *contWinPtr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)macWin->winPtr);

	if (contWinPtr) {
	    macParent = contWinPtr->privatePtr;
	} else {
	    /*
	     * Here we should handle out of process embedding. At this point,
	     * we are assuming that the changes.x,y is not maintained, if you
	     * need the info get it from Tk_GetRootCoords, and that the
	     * toplevel sits at 0,0 when it is drawn.
	     */
	}
    } else {
	/*
	 * TODO: update all xOff & yOffs
	 */

	macParent = macWin->winPtr->parentPtr->privatePtr;
	parentBorderwidth = macWin->winPtr->parentPtr->changes.border_width;
    }

    if (macParent) {
	deltaX = macParent->xOff + parentBorderwidth +
		macWin->winPtr->changes.x - macWin->xOff;
	deltaY = macParent->yOff + parentBorderwidth +
		macWin->winPtr->changes.y - macWin->yOff;
    }
    if (macWindow) {
	TkMacOSXInvalidateWindow(macWin, TK_PARENT_WINDOW);
	if (macParent) {
	    TkMacOSXInvalClipRgns((Tk_Window)macParent->winPtr);
	}
    }
    UpdateOffsets(macWin->winPtr, deltaX, deltaY);
    if (macWindow) {
	TkMacOSXInvalidateWindow(macWin, TK_PARENT_WINDOW);
    }
    GenerateConfigureNotify(macWin->winPtr, 0);
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateConfigureNotify --
 *
 *	Generates ConfigureNotify events for all the child widgets of the
 *	widget passed in the winPtr parameter. If includeWin is true, also
 *	generates ConfigureNotify event for the widget itself.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	ConfigureNotify events will be posted.
 *
 *----------------------------------------------------------------------
 */

static void
GenerateConfigureNotify(
    TkWindow *winPtr,
    int includeWin)
{
    TkWindow *childPtr;

    for (childPtr = winPtr->childList; childPtr != NULL;
	    childPtr = childPtr->nextPtr) {
	if (!Tk_IsMapped(childPtr) || Tk_IsTopLevel(childPtr)) {
	    continue;
	}
	GenerateConfigureNotify(childPtr, 1);
    }
    if (includeWin) {
	TkDoConfigureNotify(winPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * XRaiseWindow --
 *
 *	Change the stacking order of a window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the stacking order of the specified window.
 *
 *----------------------------------------------------------------------
 */

int
XRaiseWindow(
    Display *display,		/* Display. */
    Window window)		/* Window. */
{
    MacDrawable *macWin = (MacDrawable *)window;

    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(macWin->winPtr) && !Tk_IsEmbedded(macWin->winPtr)) {
	TkWmRestackToplevel(macWin->winPtr, Above, NULL);
    } else {
	/*
	 * TODO: this should generate damage
	 */
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XLowerWindow --
 *
 *	Change the stacking order of a window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the stacking order of the specified window.
 *
 *----------------------------------------------------------------------
 */

int
XLowerWindow(
    Display *display,		/* Display. */
    Window window)		/* Window. */
{
    MacDrawable *macWin = (MacDrawable *)window;

    LastKnownRequestProcessed(display)++;
    if (Tk_IsTopLevel(macWin->winPtr) && !Tk_IsEmbedded(macWin->winPtr)) {
	TkWmRestackToplevel(macWin->winPtr, Below, NULL);
    } else {
	/*
	 * TODO: this should generate damage
	 */
    }
    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * XConfigureWindow --
 *
 *	Change the size, position, stacking, or border of the specified window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the attributes of the specified window. Note that we ignore the
 *	passed in values and use the values stored in the TkWindow data
 *	structure.
 *
 *----------------------------------------------------------------------
 */

int
XConfigureWindow(
    Display *display,		/* Display. */
    Window w,			/* Window. */
    unsigned int value_mask,
    TCL_UNUSED(XWindowChanges *))
{
    MacDrawable *macWin = (MacDrawable *)w;
    TkWindow *winPtr = macWin->winPtr;

    LastKnownRequestProcessed(display)++;

    /*
     * Change the shape and/or position of the window.
     */

    if (value_mask & (CWX|CWY|CWWidth|CWHeight)) {
	XMoveResizeWindow(display, w, winPtr->changes.x, winPtr->changes.y,
		winPtr->changes.width, winPtr->changes.height);
    }

    /*
     * Change the stacking order of the window. Tk actually keeps all the
     * information we need for stacking order. All we need to do is make sure
     * the clipping regions get updated and generate damage that will ensure
     * things get drawn correctly.
     */

    if (value_mask & CWStackMode) {
	NSView *view = TkMacOSXGetNSViewForDrawable(macWin);

	if (view) {
	    TkMacOSXInvalidateWindow(macWin, TK_PARENT_WINDOW);
	}
    }

    return Success;
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXUpdateClipRgn --
 *
 *	This function updates the clipping regions for a given window and all of
 *	its children. Once updated the TK_CLIP_INVALID flag in the subwindow
 *	data structure is unset. The TK_CLIP_INVALID flag should always be
 *	unset before any drawing is attempted.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The clip regions for the window and its children are updated.
 *
 *----------------------------------------------------------------------
 */

void
TkMacOSXUpdateClipRgn(
    TkWindow *winPtr)
{
    MacDrawable *macWin;

    if (winPtr == NULL) {
	return;
    }
    macWin = winPtr->privatePtr;
    if (macWin && macWin->flags & TK_CLIP_INVALID) {
	TkWindow *win2Ptr;

#ifdef TK_MAC_DEBUG_CLIP_REGIONS
	TkMacOSXDbgMsg("%s", winPtr->pathName);
#endif
	if (Tk_IsMapped(winPtr)) {
	    int rgnChanged = 0;
	    CGRect bounds;
	    HIMutableShapeRef rgn;

	    /*
	     * Start with a region defined by the window bounds.
	     */

	    TkMacOSXWinCGBounds(winPtr, &bounds);
	    rgn = HIShapeCreateMutableWithRect(&bounds);

	    /*
	     * Clip away the area of any windows that may obscure this window.
	     * For a non-toplevel window, first, clip to the parent's visible
	     * clip region. Second, clip away any siblings that are higher in
	     * the stacking order. For an embedded toplevel, just clip to the
	     * container's visible clip region. Remember, we only allow one
	     * contained window in a frame, and don't support any other widgets
	     * in the frame either. This is not currently enforced, however.
	     */

	    if (!Tk_IsTopLevel(winPtr)) {
		if (winPtr->parentPtr) {
		    TkMacOSXUpdateClipRgn(winPtr->parentPtr);
		    ChkErr(HIShapeIntersect,
			    winPtr->parentPtr->privatePtr->aboveVisRgn,
			    rgn, rgn);
		}
		win2Ptr = winPtr;
		while ((win2Ptr = win2Ptr->nextPtr)) {
		    if (Tk_IsTopLevel(win2Ptr) || !Tk_IsMapped(win2Ptr)) {
			continue;
		    }
		    TkMacOSXWinCGBounds(win2Ptr, &bounds);
		    ChkErr(TkMacOSHIShapeDifferenceWithRect, rgn, &bounds);
		}
	    } else if (Tk_IsEmbedded(winPtr)) {
		win2Ptr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)winPtr);
		if (win2Ptr) {
		    TkMacOSXUpdateClipRgn(win2Ptr);
		    ChkErr(HIShapeIntersect,
			    win2Ptr->privatePtr->aboveVisRgn, rgn, rgn);
		}

		/*
		 * TODO: Here we should handle out of process embedding.
		 */
	    }
	    macWin->aboveVisRgn = HIShapeCreateCopy(rgn);

	    /*
	     * The final clip region is the aboveVis region (or visible region)
	     * minus all the children of this window. If the window is a
	     * container, we must also subtract the region of the embedded
	     * window.
	     */

	    win2Ptr = winPtr->childList;
	    while (win2Ptr) {
		if (Tk_IsTopLevel(win2Ptr) || !Tk_IsMapped(win2Ptr)) {
		    win2Ptr = win2Ptr->nextPtr;
		    continue;
		}
		TkMacOSXWinCGBounds(win2Ptr, &bounds);
		ChkErr(TkMacOSHIShapeDifferenceWithRect, rgn, &bounds);
		rgnChanged = 1;
		win2Ptr = win2Ptr->nextPtr;
	    }

	    if (Tk_IsContainer(winPtr)) {
		win2Ptr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)winPtr);
		if (win2Ptr) {
		    if (Tk_IsMapped(win2Ptr)) {
			TkMacOSXWinCGBounds(win2Ptr, &bounds);
			ChkErr(TkMacOSHIShapeDifferenceWithRect, rgn, &bounds);
			rgnChanged = 1;
		    }
		}

		/*
		 * TODO: Here we should handle out of process embedding.
		 */
	    }

	    if (rgnChanged) {
		HIShapeRef diffRgn = HIShapeCreateDifference(
			macWin->aboveVisRgn, rgn);

		if (!HIShapeIsEmpty(diffRgn)) {
		    macWin->visRgn = HIShapeCreateCopy(rgn);
		}
		CFRelease(diffRgn);
	    }
	    CFRelease(rgn);
	} else {
	    /*
	     * An unmapped window has empty clip regions to prevent any
	     * (erroneous) drawing into it or its children from becoming
	     * visible. [Bug 940117]
	     */

	    if (!Tk_IsTopLevel(winPtr)) {
		TkMacOSXUpdateClipRgn(winPtr->parentPtr);
	    } else if (Tk_IsEmbedded(winPtr)) {
		win2Ptr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)winPtr);
		if (win2Ptr) {
		    TkMacOSXUpdateClipRgn(win2Ptr);
		}
	    }
	    macWin->aboveVisRgn = HIShapeCreateEmpty();
	}
	if (!macWin->visRgn) {
	    macWin->visRgn = HIShapeCreateCopy(macWin->aboveVisRgn);
	}
	macWin->flags &= ~TK_CLIP_INVALID;
    }
}

// Unused and misspelled stub function
/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXVisableClipRgn --
 *
 *	This function returns the Macintosh clipping region for the given
 *	window. The caller is responsible for disposing of the returned region
 *	via XDestroyRegion().
 *
 * Results:
 *	The region.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Region
TkMacOSXVisableClipRgn(
    TkWindow *winPtr)
{
    if (winPtr->privatePtr->flags & TK_CLIP_INVALID) {
	TkMacOSXUpdateClipRgn(winPtr);
    }
    return (Region) HIShapeCreateMutableCopy(winPtr->privatePtr->visRgn);
}

#if 0
//This code is not currently used.  But it shows how to iterate over the
//rectangles in a region described by an HIShape.

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXInvalidateViewRegion --
 *
 *	This function invalidates the given region of a view.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Damage is created.
 *
 *----------------------------------------------------------------------
 */

static OSStatus
InvalViewRect(
    int msg,
    TCL_UNUSED(HIShapeRef),
    const CGRect *rect,
    void *ref)
{
    static CGAffineTransform t;
    TKContentView *view = (TKContentView *)ref;
    NSRect dirtyRect;

    if (!view) {
	return paramErr;
    }
    switch (msg) {
    case kHIShapeEnumerateInit:
	t = CGAffineTransformMake(1.0, 0.0, 0.0, -1.0, 0.0,
		NSHeight([view bounds]));
	break;
    case kHIShapeEnumerateRect:
	dirtyRect = NSRectFromCGRect(CGRectApplyAffineTransform(*rect, t));
	break;
    }
    [view generateExposeEvents:[view bounds]];
    return noErr;
}

void
TkMacOSXInvalidateViewRegion(
    NSView *view,
    HIShapeRef rgn)
{
    if (view && !HIShapeIsEmpty(rgn)) {
	ChkErr(HIShapeEnumerate, rgn,
		kHIShapeParseFromBottom|kHIShapeParseFromLeft,
		InvalViewRect, view);
    }
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXInvalidateWindow --
 *
 *	This stub function redraws the part of the toplevel window
 *      covered by a given Tk window.  (Except currently it redraws
 *      the entire toplevel.)
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The window is redrawn.
 *
 *----------------------------------------------------------------------
 */
void
TkMacOSXRedrawViewIdleTask(
    void *clientData)
{
    TKContentView *view = (TKContentView *) clientData;
    //    fprintf(stderr, "idle redraw for %p\n", view);
    [view generateExposeEvents:[view bounds]];
}

void
TkMacOSXInvalidateWindow(
    MacDrawable *macWin,	/* Window to be invalidated. */
    int flag)			/* Should be TK_WINDOW_ONLY or
				 * TK_PARENT_WINDOW */
{
#ifdef TK_MAC_DEBUG_CLIP_REGIONS
    TkMacOSXDbgMsg("%s", macWin->winPtr->pathName);
#endif
    TKContentView *view = (TKContentView *)TkMacOSXGetNSViewForDrawable(macWin);
    TkWindow *winPtr = macWin->winPtr;
    Tk_Window tkwin = (Tk_Window) winPtr;
    Tk_Window parent = (Tk_Window) winPtr->parentPtr;
    TkMacOSXInvalClipRgns(tkwin);
    if ((flag == TK_PARENT_WINDOW) && parent){
	TkMacOSXInvalClipRgns(parent);
    }
    [view generateExposeEvents:[view bounds]];
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXGetNSWindowForDrawable --
 *
 *	This function returns the NSWindow for a given X drawable, if the
 *      drawable is a window.  If the drawable is a pixmap it returns nil.
 *
 * Results:
 *	A NSWindow, or nil for off screen pixmaps.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void *
Tk_MacOSXGetNSWindowForDrawable(
    Drawable drawable)
{
    MacDrawable *macWin = (MacDrawable *)drawable;
    NSWindow *result = nil;

    if (!macWin || macWin->flags & TK_IS_PIXMAP) {
	result = nil;

    } else if (macWin->toplevel && macWin->toplevel->winPtr &&
	    macWin->toplevel->winPtr->wmInfoPtr &&
	    macWin->toplevel->winPtr->wmInfoPtr->window) {
	result = macWin->toplevel->winPtr->wmInfoPtr->window;

    } else if (macWin->winPtr && macWin->winPtr->wmInfoPtr &&
	    macWin->winPtr->wmInfoPtr->window) {
	result = macWin->winPtr->wmInfoPtr->window;

    } else if (macWin->toplevel && (macWin->toplevel->flags & TK_EMBEDDED)) {
	TkWindow *contWinPtr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)macWin->toplevel->winPtr);
	if (contWinPtr && contWinPtr->privatePtr) {
	    result = TkMacOSXGetNSWindowForDrawable((Drawable)contWinPtr->privatePtr);
	}
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXGetNSViewForDrawable/TkMacOSXGetRootControl --
 *
 *	The function name TkMacOSXGetRootControl is being preserved only
 *      because it exists in a stubs table.  Nobody knows what it means to
 *      get a "RootControl".  The macro TkMacOSXGetNSViewForDrawable calls
 *      this function and should always be used rather than directly using
 *      the obscure official name of this function.
 *
 *      It returns the NSView for a given X drawable in the case that the
 *      drawable is a window.  If the drawable is a pixmap it returns nil.
 *
 * Results:
 *	A NSView* or nil.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void *
TkMacOSXGetRootControl(
    Drawable drawable)
{
    void *result = NULL;
    MacDrawable *macWin = (MacDrawable *)drawable;

    if (!macWin) {
	result = NULL;
    } else if (!macWin->toplevel) {
	result = macWin->view;
    } else if (!(macWin->toplevel->flags & TK_EMBEDDED)) {
	result = macWin->toplevel->view;
    } else {
	TkWindow *contWinPtr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)macWin->toplevel->winPtr);

	if (contWinPtr) {
	    result = TkMacOSXGetRootControl((Drawable)contWinPtr->privatePtr);
	}
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXInvalClipRgns --
 *
 *	This function invalidates the clipping regions for a given window and
 *	all of its children. This function should be called whenever changes
 *	are made to subwindows that would affect the size or position of
 *	windows.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The clipping regions for the window and its children are marked invalid.
 *	(Make sure they are valid before drawing.)
 *
 *----------------------------------------------------------------------
 */

void
TkMacOSXInvalClipRgns(
    Tk_Window tkwin)
{
    TkWindow *winPtr = (TkWindow *) tkwin;
    TkWindow *childPtr;
    MacDrawable *macWin = winPtr->privatePtr;

    /*
     * If already marked we can stop because all descendants will also already
     * be marked.
     */

#ifdef TK_MAC_DEBUG_CLIP_REGIONS
	TkMacOSXDbgMsg("%s", winPtr->pathName);
#endif

    if (!macWin || macWin->flags & TK_CLIP_INVALID) {
	return;
    }

    macWin->flags |= TK_CLIP_INVALID;
    if (macWin->visRgn) {
	CFRelease(macWin->visRgn);
	macWin->visRgn = NULL;
    }
    if (macWin->aboveVisRgn) {
	CFRelease(macWin->aboveVisRgn);
	macWin->aboveVisRgn = NULL;
    }
    if (macWin->drawRgn) {
	CFRelease(macWin->drawRgn);
	macWin->drawRgn = NULL;
    }

    /*
     * Invalidate clip regions for all children & their descendants, unless the
     * child is a toplevel.
     */

    childPtr = winPtr->childList;
    while (childPtr) {
	if (!Tk_IsTopLevel(childPtr)) {
	    TkMacOSXInvalClipRgns((Tk_Window)childPtr);
	}
	childPtr = childPtr->nextPtr;
    }

    /*
     * Also, if the window is a container, mark its embedded window.
     */

    if (Tk_IsContainer(winPtr)) {
	childPtr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)winPtr);

	if (childPtr) {
	    TkMacOSXInvalClipRgns((Tk_Window)childPtr);
	}

	/*
	 * TODO: Here we should handle out of process embedding.
	 */
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXWinBounds --
 *
 *	Given a Tk window this function determines the window's bounds in
 *	relation to the Macintosh window's coordinate system. This is also the
 *	same coordinate system as the Tk toplevel window in which this window
 *	is contained.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Fills in a Rect.
 *
 *----------------------------------------------------------------------
 */

void
TkMacOSXWinBounds(
    TkWindow *winPtr,
    void *bounds)
{
    Rect *b = (Rect *) bounds;

    b->left = winPtr->privatePtr->xOff;
    b->top = winPtr->privatePtr->yOff;
    b->right = b->left + winPtr->changes.width;
    b->bottom = b->top + winPtr->changes.height;
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXWinCGBounds --
 *
 *	Given a Tk window this function determines the window's bounds in
 *	the coordinate system of the Tk toplevel window in which this window
 *	is contained.  This fills in a CGRect struct.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Fill in a CGRect.
 *
 *----------------------------------------------------------------------
 */

void
TkMacOSXWinCGBounds(
    TkWindow *winPtr,
    CGRect *bounds)
{
    bounds->origin.x = winPtr->privatePtr->xOff;
    bounds->origin.y = winPtr->privatePtr->yOff;
    bounds->size.width = winPtr->changes.width;
    bounds->size.height = winPtr->changes.height;
}
/*
 *----------------------------------------------------------------------
 *
 * TkMacOSXWinNSBounds --
 *
 *	Given a Tk window this function determines the window's bounds in
 *	the coordinate system of the TKContentView in which this Tk window
 *	is contained, which has the origin at the lower left corner.  This
 *      fills in an NSRect struct and requires the TKContentView as a
 *      parameter
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Fills in an NSRect.
 *
 *----------------------------------------------------------------------
 */

void
TkMacOSXWinNSBounds(
    TkWindow *winPtr,
    NSView *view,
    NSRect *bounds)
{
    bounds->size.width = winPtr->changes.width;
    bounds->size.height = winPtr->changes.height;
    bounds->origin.x = winPtr->privatePtr->xOff;
    bounds->origin.y = ([view bounds].size.height -
		       bounds->size.height -
		       winPtr->privatePtr->yOff);
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateOffsets --
 *
 *	Updates the X & Y offsets of the given TkWindow from the TopLevel it is
 *	a descendant of.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The xOff & yOff fields for the Mac window datastructure is updated to
 *	the proper offset.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateOffsets(
    TkWindow *winPtr,
    int deltaX,
    int deltaY)
{
    TkWindow *childPtr;

    if (winPtr->privatePtr == NULL) {
	/*
	 * We haven't called Tk_MakeWindowExist for this window yet. The offset
	 * information will be postponed and calulated at that time. (This will
	 * usually only happen when a mapped parent is being moved but has
	 * child windows that have yet to be mapped.)
	 */

	return;
    }

    winPtr->privatePtr->xOff += deltaX;
    winPtr->privatePtr->yOff += deltaY;

    childPtr = winPtr->childList;
    while (childPtr != NULL) {
	if (!Tk_IsTopLevel(childPtr)) {
	    UpdateOffsets(childPtr, deltaX, deltaY);
	}
	childPtr = childPtr->nextPtr;
    }

    if (Tk_IsContainer(winPtr)) {
	childPtr = (TkWindow *)Tk_GetOtherWindow((Tk_Window)winPtr);
	if (childPtr != NULL) {
	    UpdateOffsets(childPtr,deltaX,deltaY);
	}

	/*
	 * TODO: Here we should handle out of process embedding.
	 */
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetPixmap --
 *
 *	Creates an in memory drawing surface.
 *
 * Results:
 *	Returns a handle to a new pixmap.
 *
 * Side effects:
 *	Allocates a new CGBitmapContext.
 *
 *----------------------------------------------------------------------
 */

Pixmap
Tk_GetPixmap(
    Display *display,	/* Display for new pixmap (can be null). */
    TCL_UNUSED(Drawable),		/* Drawable where pixmap will be used (ignored). */
    int width,		/* Dimensions of pixmap. */
    int height,
    int depth)		/* Bits per pixel for pixmap. */
{
    MacDrawable *macPix;

    if (display != NULL) {
	LastKnownRequestProcessed(display)++;
    }
    macPix = (MacDrawable *)ckalloc(sizeof(MacDrawable));
    macPix->winPtr = NULL;
    macPix->xOff = 0;
    macPix->yOff = 0;
    macPix->visRgn = NULL;
    macPix->aboveVisRgn = NULL;
    macPix->drawRgn = NULL;
    macPix->referenceCount = 0;
    macPix->toplevel = NULL;
    macPix->flags = TK_IS_PIXMAP | (depth == 1 ? TK_IS_BW_PIXMAP : 0);
    macPix->view = nil;
    macPix->context = NULL;
    macPix->size = CGSizeMake(width, height);

    return (Pixmap) macPix;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_FreePixmap --
 *
 *	Release the resources associated with a pixmap.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Deletes the CGBitmapContext created by Tk_GetPixmap.
 *
 *----------------------------------------------------------------------
 */

void
Tk_FreePixmap(
    Display *display,		/* Display. */
    Pixmap pixmap)		/* Pixmap to destroy */
{
    MacDrawable *macPix = (MacDrawable *)pixmap;

    LastKnownRequestProcessed(display)++;
    if (macPix->context) {
	CFRelease(macPix->context);
    }
    ckfree(macPix);
}

/*
 * Local Variables:
 * mode: objc
 * c-basic-offset: 4
 * fill-column: 79
 * coding: utf-8
 * End:
 */
