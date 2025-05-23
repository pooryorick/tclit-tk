# tkInt.decls --
#
#	This file contains the declarations for all unsupported functions that
#	are exported by the Tk library. This file is used to generate the
#	tkIntDecls.h, tkIntPlatDecls.h, tkIntStub.c, and tkPlatStub.c files.
#
# Copyright © 1998-1999 Scriptics Corporation.
# Copyright © 2007 Daniel A. Steffen <das@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

library tk

##############################################################################

# Define the unsupported generic interfaces.

interface tkInt
scspec EXTERN

# Declare each of the functions in the unsupported internal Tcl interface.
# These interfaces are allowed to changed between versions. Use at your own
# risk. Note that the position of functions should not be changed between
# versions to avoid gratuitous incompatibilities.

declare 0 {
    TkWindow *TkAllocWindow(TkDisplay *dispPtr, int screenNum,
	    TkWindow *parentPtr)
}
declare 1 {
    void TkBezierPoints(double control[], int numSteps, double *coordPtr)
}
declare 2 {
    void TkBezierScreenPoints(Tk_Canvas canvas, double control[],
	    int numSteps, XPoint *xPointPtr)
}
#
# Slot 3 unused (WAS: TkBindDeadWindow)
#
declare 4 {
    void TkBindEventProc(TkWindow *winPtr, XEvent *eventPtr)
}
declare 5 {
    void TkBindFree(TkMainInfo *mainPtr)
}
declare 6 {
    void TkBindInit(TkMainInfo *mainPtr)
}
declare 7 {
    void TkChangeEventWindow(XEvent *eventPtr, TkWindow *winPtr)
}
declare 8 {
    int TkClipInit(Tcl_Interp *interp, TkDisplay *dispPtr)
}
declare 9 {
    void TkComputeAnchor(Tk_Anchor anchor, Tk_Window tkwin, int padX, int padY,
	    int innerWidth, int innerHeight, int *xPtr, int *yPtr)
}
#
# Slot 10 unused (WAS: TkCopyAndGlobalEval)
# Slot 11 unused (WAS: TkCreateBindingProcedure)
#
declare 12 {
    TkCursor *TkCreateCursorFromData(Tk_Window tkwin,
	    const char *source, const char *mask, int width, int height,
	    int xHot, int yHot, XColor fg, XColor bg)
}
declare 13 {
    int TkCreateFrame(void *clientData, Tcl_Interp *interp,
	    Tcl_Size objc, Tcl_Obj *const objv[], int type, const char *appName)
}
declare 14 {
    Tk_Window TkCreateMainWindow(Tcl_Interp *interp,
	    const char *screenName, const char *baseName)
}
declare 15 {
    Time TkCurrentTime(TkDisplay *dispPtr)
}
declare 16 {
    void TkDeleteAllImages(TkMainInfo *mainPtr)
}
declare 17 {
    void TkDoConfigureNotify(TkWindow *winPtr)
}
declare 18 {
    void TkDrawInsetFocusHighlight(Tk_Window tkwin, GC gc, int width,
	    Drawable drawable, int padding)
}
declare 19 {
    void TkEventDeadWindow(TkWindow *winPtr)
}
declare 20 {
    void TkFillPolygon(Tk_Canvas canvas, double *coordPtr, int numPoints,
	    Display *display, Drawable drawable, GC gc, GC outlineGC)
}
declare 21 {
    int TkFindStateNum(Tcl_Interp *interp, const char *option,
	    const TkStateMap *mapPtr, const char *strKey)
}
declare 22 {
    const char *TkFindStateString(const TkStateMap *mapPtr, int numKey)
}
declare 23 {
    void TkFocusDeadWindow(TkWindow *winPtr)
}
declare 24 {
    int TkFocusFilterEvent(TkWindow *winPtr, XEvent *eventPtr)
}
declare 25 {
    TkWindow *TkFocusKeyEvent(TkWindow *winPtr, XEvent *eventPtr)
}
declare 26 {
    void TkFontPkgInit(TkMainInfo *mainPtr)
}
declare 27 {
    void TkFontPkgFree(TkMainInfo *mainPtr)
}
declare 28 {
    void TkFreeBindingTags(TkWindow *winPtr)
}

# Name change only, TkFreeCursor in Tcl 8.0.x now TkpFreeCursor
declare 29 {
    void TkpFreeCursor(TkCursor *cursorPtr)
}
declare 30 {
    char *TkGetBitmapData(Tcl_Interp *interp, const char *string,
	    const char *fileName, int *widthPtr, int *heightPtr,
	    int *hotXPtr, int *hotYPtr)
}
declare 31 {
    void TkGetButtPoints(double p1[], double p2[],
	    double width, int project, double m1[], double m2[])
}
declare 32 {
    TkCursor *TkGetCursorByName(Tcl_Interp *interp,
	    Tk_Window tkwin, const char *string)
}
declare 33 {
    const char *TkGetDefaultScreenName(Tcl_Interp *interp,
	    const char *screenName)
}
declare 34 {
    TkDisplay *TkGetDisplay(Display *display)
}
declare 35 {
    Tcl_Size TkGetDisplayOf(Tcl_Interp *interp, Tcl_Size objc, Tcl_Obj *const objv[],
	    Tk_Window *tkwinPtr)
}
declare 36 {
    TkWindow *TkGetFocusWin(TkWindow *winPtr)
}
declare 37 {
    int TkGetInterpNames(Tcl_Interp *interp, Tk_Window tkwin)
}
declare 38 {
    int TkGetMiterPoints(double p1[], double p2[], double p3[],
	    double width, double m1[], double m2[])
}
declare 39 {
    void TkGetPointerCoords(Tk_Window tkwin, int *xPtr, int *yPtr)
}
declare 40 {
    void TkGetServerInfo(Tcl_Interp *interp, Tk_Window tkwin)
}
declare 41 {
    void TkGrabDeadWindow(TkWindow *winPtr)
}
declare 42 {
    int TkGrabState(TkWindow *winPtr)
}
declare 43 {
    void TkIncludePoint(Tk_Item *itemPtr, double *pointPtr)
}
declare 44 {
    void TkInOutEvents(XEvent *eventPtr, TkWindow *sourcePtr,
	    TkWindow *destPtr, int leaveType, int enterType,
	    Tcl_QueuePosition position)
}
declare 45 {
    void TkInstallFrameMenu(Tk_Window tkwin)
}
declare 46 {
    const char *TkKeysymToString(KeySym keysym)
}
declare 47 {
    int TkLineToArea(double end1Ptr[], double end2Ptr[], double rectPtr[])
}
declare 48 {
    double TkLineToPoint(double end1Ptr[], double end2Ptr[], double pointPtr[])
}
declare 49 {
    int TkMakeBezierCurve(Tk_Canvas canvas, double *pointPtr, int numPoints,
	    int numSteps, XPoint xPoints[], double dblPoints[])
}
declare 50 {
    void TkMakeBezierPostscript(Tcl_Interp *interp,
	    Tk_Canvas canvas, double *pointPtr, int numPoints)
}
declare 51 {
    void TkOptionClassChanged(TkWindow *winPtr)
}
declare 52 {
    void TkOptionDeadWindow(TkWindow *winPtr)
}
declare 53 {
    int TkOvalToArea(double *ovalPtr, double *rectPtr)
}
declare 54 {
    double TkOvalToPoint(double ovalPtr[],
	    double width, int filled, double pointPtr[])
}
declare 55 {
    int TkpChangeFocus(TkWindow *winPtr, int force)
}
declare 56 {
    void TkpCloseDisplay(TkDisplay *dispPtr)
}
declare 57 {
    void TkpClaimFocus(TkWindow *topLevelPtr, int force)
}
declare 58 {
    void TkpDisplayWarning(const char *msg, const char *title)
}
declare 59 {
    void TkpGetAppName(Tcl_Interp *interp, Tcl_DString *name)
}
declare 61 {
    TkWindow *TkpGetWrapperWindow(TkWindow *winPtr)
}
declare 62 {
    int TkpInit(Tcl_Interp *interp)
}
declare 63 {
    void TkpInitializeMenuBindings(Tcl_Interp *interp,
	    Tk_BindingTable bindingTable)
}
declare 65 {
    void TkpMakeMenuWindow(Tk_Window tkwin, int transient)
}
declare 67 {
    void TkpMenuNotifyToplevelCreate(Tcl_Interp *interp, const char *menuName)
}
declare 68 {
    TkDisplay *TkpOpenDisplay(const char *display_name)
}
declare 69 {
    int TkPointerEvent(XEvent *eventPtr, TkWindow *winPtr)
}
declare 70 {
    int TkPolygonToArea(double *polyPtr, int numPoints, double *rectPtr)
}
declare 71 {
    double TkPolygonToPoint(double *polyPtr, int numPoints, double *pointPtr)
}
declare 72 {
    int TkPositionInTree(TkWindow *winPtr, TkWindow *treePtr)
}
declare 73 {
    void TkpRedirectKeyEvent(TkWindow *winPtr, XEvent *eventPtr)
}
declare 77 {
    void TkQueueEventForAllChildren(TkWindow *winPtr, XEvent *eventPtr)
}
declare 78 {
    int TkReadBitmapFile(Display *display, Drawable d, const char *filename,
	    unsigned int *width_return, unsigned int *height_return,
	    Pixmap *bitmap_return, int *x_hot_return, int *y_hot_return)
}
declare 79 {
    int TkScrollWindow(Tk_Window tkwin, GC gc, int x, int y,
	    int width, int height, int dx, int dy, Region damageRgn)
}
declare 80 {
    void TkSelDeadWindow(TkWindow *winPtr)
}
declare 81 {
    void TkSelEventProc(Tk_Window tkwin, XEvent *eventPtr)
}
declare 82 {
    void TkSelInit(Tk_Window tkwin)
}
declare 83 {
    void TkSelPropProc(XEvent *eventPtr)
}
declare 86 {
    KeySym TkStringToKeysym(const char *name)
}
declare 87 {
    int TkThickPolyLineToArea(double *coordPtr, int numPoints,
	    double width, int capStyle, int joinStyle, double *rectPtr)
}
declare 88 {
    void TkWmAddToColormapWindows(TkWindow *winPtr)
}
declare 89 {
    void TkWmDeadWindow(TkWindow *winPtr)
}
declare 90 {
    TkWindow *TkWmFocusToplevel(TkWindow *winPtr)
}
declare 91 {
    void TkWmMapWindow(TkWindow *winPtr)
}
declare 92 {
    void TkWmNewWindow(TkWindow *winPtr)
}
declare 93 {
    void TkWmProtocolEventProc(TkWindow *winPtr, XEvent *evenvPtr)
}
declare 94 {
    void TkWmRemoveFromColormapWindows(TkWindow *winPtr)
}
declare 95 {
    void TkWmRestackToplevel(TkWindow *winPtr, int aboveBelow,
	    TkWindow *otherPtr)
}
declare 96 {
    void TkWmSetClass(TkWindow *winPtr)
}
declare 97 {
    void TkWmUnmapWindow(TkWindow *winPtr)
}

# new for 8.1

declare 98 {
    Tcl_Obj *TkDebugBitmap(Tk_Window tkwin, const char *name)
}
declare 99 {
    Tcl_Obj *TkDebugBorder(Tk_Window tkwin, const char *name)
}
declare 100 {
    Tcl_Obj *TkDebugCursor(Tk_Window tkwin, const char *name)
}
declare 101 {
    Tcl_Obj *TkDebugColor(Tk_Window tkwin, const char *name)
}
declare 102 {
    Tcl_Obj *TkDebugConfig(Tcl_Interp *interp, Tk_OptionTable table)
}
declare 103 {
    Tcl_Obj *TkDebugFont(Tk_Window tkwin, const char *name)
}
declare 104 {
    int TkFindStateNumObj(Tcl_Interp *interp, Tcl_Obj *optionPtr,
	    const TkStateMap *mapPtr, Tcl_Obj *keyPtr)
}
declare 105 {
    Tcl_HashTable *TkGetBitmapPredefTable(void)
}
declare 106 {
    TkDisplay *TkGetDisplayList(void)
}
declare 107 {
    TkMainInfo *TkGetMainInfoList(void)
}
declare 108 {
    int TkGetWindowFromObj(Tcl_Interp *interp, Tk_Window tkwin,
	    Tcl_Obj *objPtr, Tk_Window *windowPtr)
}
declare 109 {
    const char *TkpGetString(TkWindow *winPtr, XEvent *eventPtr, Tcl_DString *dsPtr)
}
declare 110 {
    void TkpGetSubFonts(Tcl_Interp *interp, Tk_Font tkfont)
}
declare 112 {
    void TkpMenuThreadInit(void)
}
declare 113 {
    int XClipBox(Region rgn, XRectangle *rect_return)
}
declare 114 {
    Region XCreateRegion(void)
}
declare 115 {
    int XDestroyRegion(Region rgn)
}
declare 116 {
    int XIntersectRegion(Region sra, Region srcb, Region dr_return)
}
declare 117 {
    int XRectInRegion(Region rgn, int x, int y, unsigned int width,
	    unsigned int height)
}
declare 118 {
    int XSetRegion(Display *display, GC gc, Region rgn)
}
declare 119 {
    int XUnionRectWithRegion(XRectangle *rect,
	    Region src, Region dr_return)
}
declare 121 {
    Pixmap TkpCreateNativeBitmap(Display *display, const void *source)
}
declare 122 {
    void TkpDefineNativeBitmaps(void)
}
declare 124 {
    Pixmap TkpGetNativeAppBitmap(Display *display,
	    const char *name, int *width, int *height)
}
declare 136 {
    void TkSetFocusWin(TkWindow *winPtr, int force)
}
declare 137 {
    void TkpSetKeycodeAndState(Tk_Window tkwin, KeySym keySym,
	    XEvent *eventPtr)
}
declare 138 {
    KeySym TkpGetKeySym(TkDisplay *dispPtr, XEvent *eventPtr)
}
declare 139 {
    void TkpInitKeymapInfo(TkDisplay *dispPtr)
}
declare 140 {
    Region TkPhotoGetValidRegion(Tk_PhotoHandle handle)
}
declare 141 {
    TkWindow **TkWmStackorderToplevel(TkWindow *parentPtr)
}
declare 142 {
    void TkFocusFree(TkMainInfo *mainPtr)
}
declare 143 {
    void TkClipCleanup(TkDisplay *dispPtr)
}
declare 144 {
    void TkGCCleanup(TkDisplay *dispPtr)
}
declare 145 {
    int XSubtractRegion(Region sra, Region srcb, Region dr_return)
}
declare 146 {
    void TkStylePkgInit(TkMainInfo *mainPtr)
}
declare 147 {
    void TkStylePkgFree(TkMainInfo *mainPtr)
}
declare 148 {
    Tk_Window TkToplevelWindowForCommand(Tcl_Interp *interp,
	    const char *cmdName)
}
declare 149 {
    const Tk_OptionSpec *TkGetOptionSpec(const char *name,
	    Tk_OptionTable optionTable)
}

# TIP#168
declare 150 {
    int TkMakeRawCurve(Tk_Canvas canvas, double *pointPtr, int numPoints,
	    int numSteps, XPoint xPoints[], double dblPoints[])
}
declare 151 {
    void TkMakeRawCurvePostscript(Tcl_Interp *interp,
	    Tk_Canvas canvas, double *pointPtr, int numPoints)
}
declare 152 {
    void TkpDrawFrame(Tk_Window tkwin, Tk_3DBorder border,
	    int highlightWidth, int borderWidth, int relief)
}
declare 153 {
    void TkCreateThreadExitHandler(Tcl_ExitProc *proc, void *clientData)
}
declare 154 {
    void TkDeleteThreadExitHandler(Tcl_ExitProc *proc, void *clientData)
}

# entries needed only by tktest:
declare 156 {
    int TkpTestembedCmd(void *clientData, Tcl_Interp *interp, Tcl_Size objc,
	    Tcl_Obj *const objv[])
}
declare 157 {
    int TkpTesttextCmd(void *dummy, Tcl_Interp *interp, Tcl_Size objc,
	    Tcl_Obj *const objv[])
}
declare 158 {
    int TkSelGetSelection(Tcl_Interp *interp, Tk_Window tkwin,
	    Atom selection, Atom target, Tk_GetSelProc *proc,
	    void *clientData)
}
declare 159 {
    int TkTextGetIndex(Tcl_Interp *interp, struct TkText *textPtr,
	    const char *string, struct TkTextIndex *indexPtr)
}
declare 160 {
    int TkTextIndexBackBytes(const struct TkText *textPtr,
	    const struct TkTextIndex *srcPtr, Tcl_Size count,
	    struct TkTextIndex *dstPtr)
}
declare 161 {
    int TkTextIndexForwBytes(const struct TkText *textPtr,
	    const struct TkTextIndex *srcPtr, Tcl_Size count,
	    struct TkTextIndex *dstPtr)
}
declare 162 {
    struct TkTextIndex *TkTextMakeByteIndex(TkTextBTree tree,
	    const struct TkText *textPtr, int lineIndex,
	    Tcl_Size byteIndex, struct TkTextIndex *indexPtr)
}
declare 163 {
    Tcl_Size TkTextPrintIndex(const struct TkText *textPtr,
	    const struct TkTextIndex *indexPtr, char *string)
}
declare 164 {
    struct TkTextSegment *TkTextSetMark(struct TkText *textPtr,
	    const char *name, struct TkTextIndex *indexPtr)
}
declare 165 {
    int TkTextXviewCmd(struct TkText *textPtr, Tcl_Interp *interp,
	    Tcl_Size objc, Tcl_Obj *const objv[])
}
declare 166 {
    void TkTextChanged(struct TkSharedText *sharedTextPtr,
	    struct TkText *textPtr, const struct TkTextIndex *index1Ptr,
	    const struct TkTextIndex *index2Ptr)
}
declare 167 {
    int	TkBTreeNumLines(TkTextBTree tree,
	    const struct TkText *textPtr)
}
declare 168 {
    void TkTextInsertDisplayProc(struct TkText *textPtr,
	    struct TkTextDispChunk *chunkPtr, int x, int y,
	    int height, int baseline, Display *display,
	    Drawable dst, int screenY)
}
# Next group of functions exposed due to [Bug 2768945].
declare 169 {
    int TkStateParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 170 {
    const char *TkStatePrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}
declare 171 {
    int TkCanvasDashParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 172 {
    const char *TkCanvasDashPrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}
declare 173 {
    int TkOffsetParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 174 {
    const char *TkOffsetPrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}
declare 175 {
    int TkPixelParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 176 {
    const char *TkPixelPrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}
declare 177 {
    int TkOrientParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 178 {
    const char *TkOrientPrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}
declare 179 {
    int TkSmoothParseProc(void *clientData, Tcl_Interp *interp,
	    Tk_Window tkwin, const char *value, char *widgRec, Tcl_Size offset)
}
declare 180 {
    const char *TkSmoothPrintProc(void *clientData, Tk_Window tkwin,
	    char *widgRec, Tcl_Size offset, Tcl_FreeProc **freeProcPtr)
}

# Angled text API, exposed for Emiliano Gavilán's RBC work.
declare 181 {
    void TkDrawAngledTextLayout(Display *display, Drawable drawable, GC gc,
	    Tk_TextLayout layout, int x, int y, double angle, Tcl_Size firstChar,
	    Tcl_Size lastChar)
}
declare 182 {
    void TkUnderlineAngledTextLayout(Display *display, Drawable drawable,
	    GC gc, Tk_TextLayout layout, int x, int y, double angle,
	    int underline)
}
declare 183 {
    int TkIntersectAngledTextLayout(Tk_TextLayout layout, int x, int y,
	    int width, int height, double angle)
}
declare 184 {
    void TkDrawAngledChars(Display *display,Drawable drawable, GC gc,
	    Tk_Font tkfont, const char *source, Tcl_Size numBytes, double x,
	    double y, double angle)
}

# Support for aqua's inability to draw outside [NSView drawRect:]
declare 185 {
    void TkpRedrawWidget(Tk_Window tkwin)
}
declare 186 {
    int TkpWillDrawWidget(Tk_Window tkwin)
}

# Debugging / testing functions for photo images
declare 187 {
    int TkDebugPhotoStringMatchDef(Tcl_Interp *inter, Tcl_Obj *data,
	    Tcl_Obj *formatString, int *widthPtr, int *heightPtr)
}


##############################################################################

# Define the platform specific internal Tcl interface. These functions are
# only available on the designated platform.

interface tkIntPlat

################################
# Unix specific functions

declare 0 x11 {
    void TkCreateXEventSource(void)
}
declare 2 x11 {
    void TkGenerateActivateEvents(TkWindow *winPtr, int active)
}
declare 6 x11 {
    int TkpScanWindowId(Tcl_Interp *interp, const char *string, Window *idPtr)
}
declare 9 x11 {
    int TkpWmSetState(TkWindow *winPtr, int state)
}
# only needed by tktest:
declare 38 x11 {
    int TkpCmapStressed(Tk_Window tkwin, Colormap colormap)
}
declare 39 x11 {
    void TkpSync(Display *display)
}
declare 40 x11 {
    Window TkUnixContainerId(TkWindow *winPtr)
}
declare 41 x11 {
    int TkUnixDoOneXEvent(Tcl_Time *timePtr)
}
declare 42 x11 {
    void TkUnixSetMenubar(Tk_Window tkwin, Tk_Window menubar)
}
declare 43 x11 {
    void TkWmCleanup(TkDisplay *dispPtr)
}
declare 44 x11 {
    void TkSendCleanup(TkDisplay *dispPtr)
}
# only needed by tktest:
declare 45 x11 {
    int TkpTestsendCmd(void *clientData, Tcl_Interp *interp, Tcl_Size objc,
	    Tcl_Obj *const objv[])
}

################################
# Windows specific functions

declare 0 win {
    void TkCreateXEventSource(void)
}
declare 2 win {
    void TkGenerateActivateEvents(TkWindow *winPtr, int active)
}
declare 3 win {
    unsigned long TkpGetMS(void)
}
declare 4 win {
    void TkPointerDeadWindow(TkWindow *winPtr)
}
declare 5 win {
    void TkpPrintWindowId(char *buf, Window window)
}
declare 6 win {
    int TkpScanWindowId(Tcl_Interp *interp, const char *string, Window *idPtr)
}
declare 7 win {
    void TkpSetCapture(TkWindow *winPtr)
}
declare 8 win {
    void TkpSetCursor(TkpCursor cursor)
}
declare 9 win {
    int TkpWmSetState(TkWindow *winPtr, int state)
}
declare 10 win {
    void TkSetPixmapColormap(Pixmap pixmap, Colormap colormap)
}
declare 11 win {
    void TkWinCancelMouseTimer(void)
}
declare 12 win {
    void TkWinClipboardRender(TkDisplay *dispPtr, UINT format)
}
declare 13 win {
    LRESULT TkWinEmbeddedEventProc(HWND hwnd, UINT message,
	    WPARAM wParam, LPARAM lParam)
}
declare 14 win {
    void TkWinFillRect(HDC dc, int x, int y, int width, int height, int pixel)
}
declare 15 win {
    COLORREF TkWinGetBorderPixels(Tk_Window tkwin, Tk_3DBorder border,
	    int which)
}
declare 16 win {
    HDC TkWinGetDrawableDC(Display *display, Drawable d, TkWinDCState *state)
}
declare 17 win {
    int TkWinGetModifierState(void)
}
declare 18 win {
    HPALETTE TkWinGetSystemPalette(void)
}
declare 19 win {
    HWND TkWinGetWrapperWindow(Tk_Window tkwin)
}
declare 20 win {
    int TkWinHandleMenuEvent(HWND *phwnd, UINT *pMessage, WPARAM *pwParam,
	    LPARAM *plParam, LRESULT *plResult)
}
declare 21 win {
    int TkWinIndexOfColor(XColor *colorPtr)
}
declare 22 win {
    void TkWinReleaseDrawableDC(Drawable d, HDC hdc, TkWinDCState *state)
}
declare 23 win {
    LRESULT TkWinResendEvent(WNDPROC wndproc, HWND hwnd, XEvent *eventPtr)
}
declare 24 win {
    HPALETTE TkWinSelectPalette(HDC dc, Colormap colormap)
}
declare 25 win {
    void TkWinSetMenu(Tk_Window tkwin, HMENU hMenu)
}
declare 26 win {
    void TkWinSetWindowPos(HWND hwnd, HWND siblingHwnd, int pos)
}
declare 27 win {
    void TkWinWmCleanup(HINSTANCE hInstance)
}
declare 28 win {
    void TkWinXCleanup(void *clientData)
}
declare 29 win {
    void TkWinXInit(HINSTANCE hInstance)
}

# new for 8.1

declare 30 win {
    void TkWinSetForegroundWindow(TkWindow *winPtr)
}
declare 31 win {
    void TkWinDialogDebug(int debug)
}
declare 32 win {
    Tcl_Obj *TkWinGetMenuSystemDefault(Tk_Window tkwin,
	    const char *dbName, const char *className)
}
declare 33 win {
    char *TkAlignImageData(XImage *image, int alignment, int bitOrder)
}

# new for 8.4.1

declare 34 win {
    void TkWinSetHINSTANCE(HINSTANCE hInstance)
}
declare 35 win {
    int TkWinGetPlatformTheme(void)
}

# Exported through stub table since Tk 8.4.20/8.5.9

declare 36 win {
    LRESULT __stdcall TkWinChildProc(HWND hwnd,
	    UINT message, WPARAM wParam, LPARAM lParam)
}

declare 38 win {
    int TkpCmapStressed(Tk_Window tkwin, Colormap colormap)
}
declare 39 win {
    void TkpSync(Display *display)
}
declare 40 win {
    Window TkUnixContainerId(TkWindow *winPtr)
}
declare 41 win {
    int TkUnixDoOneXEvent(Tcl_Time *timePtr)
}
declare 42 win {
    void TkUnixSetMenubar(Tk_Window tkwin, Tk_Window menubar)
}
declare 43 win {
    void TkWmCleanup(TkDisplay *dispPtr)
}
declare 44 win {
    void TkSendCleanup(TkDisplay *dispPtr)
}
# only needed by tktest:
declare 45 win {
    int TkpTestsendCmd(void *clientData, Tcl_Interp *interp, Tcl_Size objc,
	    Tcl_Obj *const objv[])
}
declare 47 win {
    Tk_Window TkpGetCapture(void)
}

################################
# Aqua specific functions

declare 1 aqua {
    void TkAboutDlg(void)
}
declare 2 aqua {
    void TkGenerateActivateEvents(TkWindow *winPtr, int active)
}
declare 3 aqua {
    unsigned long TkpGetMS(void)
}
declare 4 aqua {
    void TkPointerDeadWindow(TkWindow *winPtr)
}
declare 5 aqua {
    void TkpSetCursor(TkpCursor cursor)
}
declare 6 aqua {
    int TkpScanWindowId(Tcl_Interp *interp, const char *string, Window *idPtr)
}
declare 7 aqua {
    int TkpWmSetState(TkWindow *winPtr, int state)
}
declare 8 aqua {
    unsigned int TkMacOSXButtonKeyState(void)
}
declare 9 aqua {
    void TkMacOSXClearMenubarActive(void)
}
declare 10 aqua {
    int TkMacOSXDispatchMenuEvent(int menuID, int index)
}
declare 11 aqua {
    void TkpSetCapture(TkWindow *winPtr)
}
declare 12 aqua {
    void TkMacOSXHandleTearoffMenu(void)
}
declare 14 aqua {
    int TkMacOSXDoHLEvent(void *theEvent)
}
declare 16 aqua {
    Window TkMacOSXGetXWindow(void *macWinPtr)
}
declare 17 aqua {
    int TkMacOSXGrowToplevel(void *whichWindow, XPoint start)
}
declare 18 aqua {
    void TkMacOSXHandleMenuSelect(short theMenu, unsigned short theItem,
	    int optionKeyPressed)
}
declare 21 aqua {
    void TkMacOSXInvalidateWindow(MacDrawable *macWin, int flag)
}
declare 23 aqua {
    void TkMacOSXMakeRealWindowExist(TkWindow *winPtr)
}
declare 24 aqua {
    void *TkMacOSXMakeStippleMap(Drawable d1, Drawable d2)
}
declare 25 aqua {
    void TkMacOSXMenuClick(void)
}
declare 27 aqua {
    int TkMacOSXResizable(TkWindow *winPtr)
}
declare 28 aqua {
    void TkMacOSXSetHelpMenuItemCount(void)
}
declare 29 aqua {
    void TkMacOSXSetScrollbarGrow(TkWindow *winPtr, int flag)
}
declare 31 aqua {
    void TkMacOSXSetUpGraphicsPort(GC gc, void *destPort)
}
declare 32 aqua {
    void TkMacOSXUpdateClipRgn(TkWindow *winPtr)
}
declare 34 aqua {
    int TkMacOSXUseMenuID(short macID)
}
declare 35 aqua {
    Region TkMacOSXVisableClipRgn(TkWindow *winPtr)
}
declare 36 aqua {
    void TkMacOSXWinBounds(TkWindow *winPtr, void *geometry)
}
declare 37 aqua {
    void TkMacOSXWindowOffset(void *wRef, int *xOffset, int *yOffset)
}
declare 38 aqua {
    int TkSetMacColor(unsigned long pixel, void *macColor)
}
declare 39 aqua {
    void TkSetWMName(TkWindow *winPtr, Tk_Uid titleUid)
}
declare 41 aqua {
    int TkMacOSXZoomToplevel(void *whichWindow, short zoomPart)
}
declare 42 aqua {
    Tk_Window Tk_TopCoordsToWindow(Tk_Window tkwin, int rootX, int rootY,
	    int *newX, int *newY)
}
declare 43 aqua {
    MacDrawable *TkMacOSXContainerId(TkWindow *winPtr)
}
declare 44 aqua {
    MacDrawable *TkMacOSXGetHostToplevel(TkWindow *winPtr)
}
declare 45 aqua {
    void TkMacOSXPreprocessMenu(void)
}
declare 46 aqua {
    int TkpIsWindowFloating(void *window)
}
declare 47 aqua {
    Tk_Window TkpGetCapture(void)
}
declare 49 aqua {
    Tk_Window TkMacOSXGetContainer(TkWindow *winPtr)
}
declare 50 aqua {
    int TkGenerateButtonEvent(int x, int y, Window window, unsigned int state)
}
declare 51 aqua {
    void TkGenWMDestroyEvent(Tk_Window tkwin)
}
#
# Slot 52 unused (WAS: TkMacOSXSetDrawingEnabled)
#
# Made public as Tk_MacOSXGetNSWindowForDrawable
#declare 54 aqua {
#    void *TkMacOSXDrawable(Drawable drawable)
#}

##############################################################################

# Define the platform specific internal Xlib interfaces. These functions are
# only available on the designated platform.

interface tkIntXlib

################################
# X functions for Windows

declare 0 win {
    int XSetDashes(Display *display, GC gc, int dash_offset,
	    _Xconst char *dash_list, int n)
}
declare 1 win {
    XModifierKeymap *XGetModifierMapping(Display *d)
}
declare 2 win {
    XImage *XCreateImage(Display *d, Visual *v, unsigned int ui1, int i1,
	    int i2, char *cp, unsigned int ui2, unsigned int ui3, int i3,
	    int i4)
}
declare 3 win {
    XImage *XGetImage(Display *d, Drawable dr, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, unsigned long ul, int i3)
}
declare 4 win {
    char *XGetAtomName(Display *d, Atom a)
}
declare 5 win {
    char *XKeysymToString(KeySym k)
}
declare 6 win {
    Colormap XCreateColormap(Display *d, Window w, Visual *v, int i)
}
declare 7 win {
    Cursor XCreatePixmapCursor(Display *d, Pixmap p1, Pixmap p2,
	    XColor *x1, XColor *x2, unsigned int ui1, unsigned int ui2)
}
declare 8 win {
    Cursor XCreateGlyphCursor(Display *d, Font f1, Font f2,
	    unsigned int ui1, unsigned int ui2, XColor _Xconst *x1,
	    XColor _Xconst *x2)
}
declare 9 win {
    GContext XGContextFromGC(GC g)
}
declare 10 win {
    XHostAddress *XListHosts(Display *d, int *i, Bool *b)
}
# second parameter was of type KeyCode
declare 11 win {
    KeySym XKeycodeToKeysym(Display *d, unsigned int k, int i)
}
declare 12 win {
    KeySym XStringToKeysym(_Xconst char *c)
}
declare 13 win {
    Window XRootWindow(Display *d, int i)
}
declare 14 win {
    XErrorHandler XSetErrorHandler(XErrorHandler x)
}
declare 15 win {
    Status XIconifyWindow(Display *d, Window w, int i)
}
declare 16 win {
    Status XWithdrawWindow(Display *d, Window w, int i)
}
declare 17 win {
    Status XGetWMColormapWindows(Display *d, Window w, Window **wpp, int *ip)
}
declare 18 win {
    Status XAllocColor(Display *d, Colormap c, XColor *xp)
}
declare 19 win {
    int XBell(Display *d, int i)
}
declare 20 win {
    int XChangeProperty(Display *d, Window w, Atom a1, Atom a2, int i1,
	    int i2, _Xconst unsigned char *c, int i3)
}
declare 21 win {
    int XChangeWindowAttributes(Display *d, Window w, unsigned long ul,
	    XSetWindowAttributes *x)
}
declare 22 win {
    int XClearWindow(Display *d, Window w)
}
declare 23 win {
    int XConfigureWindow(Display *d, Window w, unsigned int i,
	    XWindowChanges *x)
}
declare 24 win {
    int XCopyArea(Display *d, Drawable dr1, Drawable dr2, GC g, int i1,
	    int i2, unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 25 win {
    int XCopyPlane(Display *d, Drawable dr1, Drawable dr2, GC g, int i1,
	    int i2, unsigned int ui1, unsigned int ui2,
	    int i3, int i4, unsigned long ul)
}
declare 26 win {
    Pixmap XCreateBitmapFromData(Display *display, Drawable d,
	    _Xconst char *data, unsigned int width, unsigned int height)
}
declare 27 win {
    int XDefineCursor(Display *d, Window w, Cursor c)
}
declare 28 win {
    int XDeleteProperty(Display *d, Window w, Atom a)
}
declare 29 win {
    int XDestroyWindow(Display *d, Window w)
}
declare 30 win {
    int XDrawArc(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 31 win {
    int XDrawLines(Display *d, Drawable dr, GC g, XPoint *x, int i1, int i2)
}
declare 32 win {
    int XDrawRectangle(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2)
}
declare 33 win {
    int XFillArc(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 34 win {
    int XFillPolygon(Display *d, Drawable dr, GC g, XPoint *x,
	    int i1, int i2, int i3)
}
declare 35 win {
    int XFillRectangles(Display *d, Drawable dr, GC g, XRectangle *x, int i)
}
declare 36 win {
    int XForceScreenSaver(Display *d, int i)
}
declare 37 win {
    int XFreeColormap(Display *d, Colormap c)
}
declare 38 win {
    int XFreeColors(Display *d, Colormap c,
	    unsigned long *ulp, int i, unsigned long ul)
}
declare 39 win {
    int XFreeCursor(Display *d, Cursor c)
}
declare 40 win {
    int XFreeModifiermap(XModifierKeymap *x)
}
declare 41 win {
    Status XGetGeometry(Display *d, Drawable dr, Window *w, int *i1,
	    int *i2, unsigned int *ui1, unsigned int *ui2, unsigned int *ui3,
	    unsigned int *ui4)
}
declare 42 win {
    int XGetInputFocus(Display *d, Window *w, int *i)
}
declare 43 win {
    int XGetWindowProperty(Display *d, Window w, Atom a1, long l1, long l2,
	    Bool b, Atom a2, Atom *ap, int *ip, unsigned long *ulp1,
	    unsigned long *ulp2, unsigned char **cpp)
}
declare 44 win {
    Status XGetWindowAttributes(Display *d, Window w, XWindowAttributes *x)
}
declare 45 win {
    int XGrabKeyboard(Display *d, Window w, Bool b, int i1, int i2, Time t)
}
declare 46 win {
    int XGrabPointer(Display *d, Window w1, Bool b, unsigned int ui,
	    int i1, int i2, Window w2, Cursor c, Time t)
}
declare 47 win {
    KeyCode XKeysymToKeycode(Display *d, KeySym k)
}
declare 48 win {
    Status XLookupColor(Display *d, Colormap c1, _Xconst char *c2,
	    XColor *x1, XColor *x2)
}
declare 49 win {
    int XMapWindow(Display *d, Window w)
}
declare 50 win {
    int XMoveResizeWindow(Display *d, Window w, int i1, int i2,
	    unsigned int ui1, unsigned int ui2)
}
declare 51 win {
    int XMoveWindow(Display *d, Window w, int i1, int i2)
}
declare 52 win {
    int XNextEvent(Display *d, XEvent *x)
}
declare 53 win {
    int XPutBackEvent(Display *d, XEvent *x)
}
declare 54 win {
    int XQueryColors(Display *d, Colormap c, XColor *x, int i)
}
declare 55 win {
    Bool XQueryPointer(Display *d, Window w1, Window *w2, Window *w3,
	    int *i1, int *i2, int *i3, int *i4, unsigned int *ui)
}
declare 56 win {
    Status XQueryTree(Display *d, Window w1, Window *w2, Window *w3,
	    Window **w4, unsigned int *ui)
}
declare 57 win {
    int XRaiseWindow(Display *d, Window w)
}
declare 58 win {
    int XRefreshKeyboardMapping(XMappingEvent *x)
}
declare 59 win {
    int XResizeWindow(Display *d, Window w, unsigned int ui1,
	    unsigned int ui2)
}
declare 60 win {
    int XSelectInput(Display *d, Window w, long l)
}
declare 61 win {
    Status XSendEvent(Display *d, Window w, Bool b, long l, XEvent *x)
}
declare 62 win {
    int XSetCommand(Display *d, Window w, char **c, int i)
}
declare 63 win {
    int XSetIconName(Display *d, Window w, _Xconst char *c)
}
declare 64 win {
    int XSetInputFocus(Display *d, Window w, int i, Time t)
}
declare 65 win {
    int XSetSelectionOwner(Display *d, Atom a, Window w, Time t)
}
declare 66 win {
    int XSetWindowBackground(Display *d, Window w, unsigned long ul)
}
declare 67 win {
    int XSetWindowBackgroundPixmap(Display *d, Window w, Pixmap p)
}
declare 68 win {
    int XSetWindowBorder(Display *d, Window w, unsigned long ul)
}
declare 69 win {
    int XSetWindowBorderPixmap(Display *d, Window w, Pixmap p)
}
declare 70 win {
    int XSetWindowBorderWidth(Display *d, Window w, unsigned int ui)
}
declare 71 win {
    int XSetWindowColormap(Display *d, Window w, Colormap c)
}
declare 72 win {
    Bool XTranslateCoordinates(Display *d, Window w1, Window w2, int i1,
	    int i2, int *i3, int *i4, Window *w3)
}
declare 73 win {
    int XUngrabKeyboard(Display *d, Time t)
}
declare 74 win {
    int XUngrabPointer(Display *d, Time t)
}
declare 75 win {
    int XUnmapWindow(Display *d, Window w)
}
declare 76 win {
    int XWindowEvent(Display *d, Window w, long l, XEvent *x)
}
declare 77 win {
    void XDestroyIC(XIC x)
}
declare 78 win {
    Bool XFilterEvent(XEvent *x, Window w)
}
declare 79 win {
    int XmbLookupString(XIC xi, XKeyPressedEvent *xk, char *c, int i,
	    KeySym *k, Status *s)
}
declare 80 win {
    int TkPutImage(unsigned long *colors, int ncolors, Display *display,
	    Drawable d, GC gc, XImage *image, int src_x, int src_y,
	    int dest_x, int dest_y, unsigned int width, unsigned int height)
}
declare 81 win {
    int XSetClipRectangles(Display *display, GC gc, int clip_x_origin,
	    int clip_y_origin, XRectangle rectangles[], int n, int ordering)
}
declare 82 win {
    Status XParseColor(Display *display, Colormap map,
	    _Xconst char *spec, XColor *colorPtr)
}
declare 83 win {
    GC XCreateGC(Display *display, Drawable d,
	    unsigned long valuemask, XGCValues *values)
}
declare 84 win {
    int XFreeGC(Display *display, GC gc)
}
declare 85 win {
    Atom XInternAtom(Display *display, _Xconst char *atom_name,
	    Bool only_if_exists)
}
declare 86 win {
    int XSetBackground(Display *display, GC gc, unsigned long foreground)
}
declare 87 win {
    int XSetForeground(Display *display, GC gc, unsigned long foreground)
}
declare 88 win {
    int XSetClipMask(Display *display, GC gc, Pixmap pixmap)
}
declare 89 win {
    int XSetClipOrigin(Display *display, GC gc,
	    int clip_x_origin, int clip_y_origin)
}
declare 90 win {
    int XSetTSOrigin(Display *display, GC gc,
	    int ts_x_origin, int ts_y_origin)
}
declare 91 win {
    int XChangeGC(Display *d, GC gc, unsigned long mask, XGCValues *values)
}
declare 92 win {
    int XSetFont(Display *display, GC gc, Font font)
}
declare 93 win {
    int XSetArcMode(Display *display, GC gc, int arc_mode)
}
declare 94 win {
    int XSetStipple(Display *display, GC gc, Pixmap stipple)
}
declare 95 win {
    int XSetFillRule(Display *display, GC gc, int fill_rule)
}
declare 96 win {
    int XSetFillStyle(Display *display, GC gc, int fill_style)
}
declare 97 win {
    int XSetFunction(Display *display, GC gc, int function)
}
declare 98 win {
    int XSetLineAttributes(Display *display, GC gc, unsigned int line_width,
	    int line_style, int cap_style, int join_style)
}
declare 99 win {
    int _XInitImageFuncPtrs(XImage *image)
}
declare 100 win {
    XIC XCreateIC(XIM xim, ...)
}
declare 101 win {
    XVisualInfo *XGetVisualInfo(Display *display, long vinfo_mask,
	    XVisualInfo *vinfo_template, int *nitems_return)
}
declare 102 win {
    void XSetWMClientMachine(Display *display, Window w,
	    XTextProperty *text_prop)
}
declare 103 win {
    Status XStringListToTextProperty(char **list, int count,
	    XTextProperty *text_prop_return)
}
declare 104 win {
    int XDrawLine(Display *d, Drawable dr, GC g, int x1, int y1,
	    int x2, int y2)
}
declare 105 win {
    int XWarpPointer(Display *d, Window s, Window dw, int sx, int sy,
	    unsigned int sw, unsigned int sh, int dx, int dy)
}
declare 106 win {
    int XFillRectangle(Display *display, Drawable d, GC gc,
	    int x, int y, unsigned int width, unsigned int height)
}

# New in Tk 8.6
declare 107 win {
    int XFlush(Display *display)
}
declare 108 win {
    int XGrabServer(Display *display)
}
declare 109 win {
    int XUngrabServer(Display *display)
}
declare 110 win {
    int XFree(void *data)
}
declare 111 win {
    int XNoOp(Display *display)
}
declare 112 win {
    XAfterFunction XSynchronize(Display *display, Bool onoff)
}
declare 113 win {
    int XSync(Display *display, Bool discard)
}
declare 114 win {
    VisualID XVisualIDFromVisual(Visual *visual)
}

# For tktreectrl
declare 120 win {
    int XOffsetRegion(Region rgn, int dx, int dy)
}
declare 121 win {
    int XUnionRegion(Region srca, Region srcb, Region dr_return)
}

# For 3dcanvas
declare 122 win {
    Window XCreateWindow(Display *display, Window parent, int x, int y,
	    unsigned int width, unsigned int height,
	    unsigned int border_width, int depth, unsigned int clazz,
	    Visual *visual, unsigned long value_mask,
	    XSetWindowAttributes *attributes)
}

# Various, e.g. for stub-enabled BLT
declare 129 win {
    int XLowerWindow(Display *d, Window w)
}
declare 130 win {
    int XFillArcs(Display *d, Drawable dr, GC gc, XArc *a, int n)
}
declare 131 win {
    int XDrawArcs(Display *d, Drawable dr, GC gc, XArc *a, int n)
}
declare 132 win {
    int XDrawRectangles(Display *d, Drawable dr, GC gc, XRectangle *r, int n)
}
declare 133 win {
    int XDrawSegments(Display *d, Drawable dr, GC gc, XSegment *s, int n)
}
declare 134 win {
    int XDrawPoint(Display *d, Drawable dr, GC gc, int x, int y)
}
declare 135 win {
    int XDrawPoints(Display *d, Drawable dr, GC gc, XPoint *p, int n, int m)
}
declare 136 win {
    int XReparentWindow(Display *d, Window w, Window p, int x, int y)
}
declare 137 win {
    int XPutImage(Display *d, Drawable dr, GC gc, XImage *im,
	    int sx, int sy, int dx, int dy,
	    unsigned int w, unsigned int h)
}
declare 138 win {
    Region XPolygonRegion(XPoint *pts, int n, int rule)
}
declare 139 win {
    int XPointInRegion(Region rgn, int x, int y)
}
# For XIM
declare 140 win {
    XVaNestedList XVaCreateNestedList(int dummy, ...)
}
declare 141 win {
    char *XSetICValues(XIC xic, ...)
}
declare 142 win {
    char *XGetICValues(XIC xic, ...)
}
declare 143 win {
    void XSetICFocus(XIC xic)
}
declare 147 win {
    void XFreeFontSet(Display *display, XFontSet fontset)
}
declare 148 win {
    int XCloseIM(XIM im)
}
declare 149 win {
    Bool XRegisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rbd,
	    char *res_name, char *res_class, XIDProc callback, XPointer client_data)
}
declare 150 win {
    Bool XUnregisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rbd,
	    char *res_name, char *res_class, XIDProc callback, XPointer client_data)
}
declare 151 win {
    char *XSetLocaleModifiers(const char *modifier_list)
}
declare 152 win {
    XIM XOpenIM(Display *dpy, struct _XrmHashBucketRec *rdb, char *res_name,
	    char *res_class)
}
declare 153 win {
    char *XGetIMValues(XIM im, ...)
}
declare 154 win {
    char *XSetIMValues(XIM im, ...)
}
declare 155 win {
    XFontSet XCreateFontSet(Display *display, _Xconst char *base_font_name_list,
	    char ***missing_charset_list, int *missing_charset_count, char **def_string)
}
declare 156 win {
    void XFreeStringList(char **list)
}
declare 157 win {
    KeySym XkbKeycodeToKeysym(Display *d, unsigned int k, int g, int i)
}
declare 158 win {
    Display *XkbOpenDisplay(const char *name, int *ev_rtrn, int *err_rtrn,
	    int *major_rtrn, int *minor_rtrn, int *reason)
}

################################
# X functions for MacOSX

declare 0 macosx {
    int XSetDashes(Display *display, GC gc, int dash_offset,
	    _Xconst char *dash_list, int n)
}
declare 1 macosx {
    XModifierKeymap *XGetModifierMapping(Display *d)
}
declare 2 macosx {
    XImage *XCreateImage(Display *d, Visual *v, unsigned int ui1, int i1,
	    int i2, char *cp, unsigned int ui2, unsigned int ui3, int i3,
	    int i4)
}
declare 3 macosx {
    XImage *XGetImage(Display *d, Drawable dr, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, unsigned long ul, int i3)
}
declare 4 macosx {
    char *XGetAtomName(Display *d, Atom a)
}
declare 5 macosx {
    char *XKeysymToString(KeySym k)
}
declare 6 macosx {
    Colormap XCreateColormap(Display *d, Window w, Visual *v, int i)
}
declare 7 macosx {
    Cursor XCreatePixmapCursor(Display *d, Pixmap p1, Pixmap p2,
	    XColor *x1, XColor *x2, unsigned int ui1, unsigned int ui2)
}
declare 8 macosx {
    Cursor XCreateGlyphCursor(Display *d, Font f1, Font f2,
	    unsigned int ui1, unsigned int ui2, XColor _Xconst *x1,
	    XColor _Xconst *x2)
}
declare 9 macosx {
    GContext XGContextFromGC(GC g)
}
declare 10 macosx {
    XHostAddress *XListHosts(Display *d, int *i, Bool *b)
}
# second parameter was of type KeyCode
declare 11 macosx {
    KeySym XKeycodeToKeysym(Display *d, unsigned int k, int i)
}
declare 12 macosx {
    KeySym XStringToKeysym(_Xconst char *c)
}
declare 13 macosx {
    Window XRootWindow(Display *d, int i)
}
declare 14 macosx {
    XErrorHandler XSetErrorHandler(XErrorHandler x)
}
declare 15 macosx {
    Status XIconifyWindow(Display *d, Window w, int i)
}
declare 16 macosx {
    Status XWithdrawWindow(Display *d, Window w, int i)
}
declare 17 macosx {
    Status XGetWMColormapWindows(Display *d, Window w, Window **wpp, int *ip)
}
declare 18 macosx {
    Status XAllocColor(Display *d, Colormap c, XColor *xp)
}
declare 19 macosx {
    int XBell(Display *d, int i)
}
declare 20 macosx {
    int XChangeProperty(Display *d, Window w, Atom a1, Atom a2, int i1,
	    int i2, _Xconst unsigned char *c, int i3)
}
declare 21 macosx {
    int XChangeWindowAttributes(Display *d, Window w, unsigned long ul,
	    XSetWindowAttributes *x)
}
declare 22 macosx {
    int XClearWindow(Display *d, Window w)
}
declare 23 macosx {
    int XConfigureWindow(Display *d, Window w, unsigned int i,
	    XWindowChanges *x)
}
declare 24 macosx {
    int XCopyArea(Display *d, Drawable dr1, Drawable dr2, GC g, int i1,
	    int i2, unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 25 macosx {
    int XCopyPlane(Display *d, Drawable dr1, Drawable dr2, GC g, int i1,
	    int i2, unsigned int ui1, unsigned int ui2,
	    int i3, int i4, unsigned long ul)
}
declare 26 macosx {
    Pixmap XCreateBitmapFromData(Display *display, Drawable d,
	    _Xconst char *data, unsigned int width, unsigned int height)
}
declare 27 macosx {
    int XDefineCursor(Display *d, Window w, Cursor c)
}
declare 28 macosx {
    int XDeleteProperty(Display *d, Window w, Atom a)
}
declare 29 macosx {
    int XDestroyWindow(Display *d, Window w)
}
declare 30 macosx {
    int XDrawArc(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 31 macosx {
    int XDrawLines(Display *d, Drawable dr, GC g, XPoint *x, int i1, int i2)
}
declare 32 macosx {
    int XDrawRectangle(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2)
}
declare 33 macosx {
    int XFillArc(Display *d, Drawable dr, GC g, int i1, int i2,
	    unsigned int ui1, unsigned int ui2, int i3, int i4)
}
declare 34 macosx {
    int XFillPolygon(Display *d, Drawable dr, GC g, XPoint *x,
	    int i1, int i2, int i3)
}
declare 35 macosx {
    int XFillRectangles(Display *d, Drawable dr, GC g, XRectangle *x, int i)
}
declare 36 macosx {
    int XForceScreenSaver(Display *d, int i)
}
declare 37 macosx {
    int XFreeColormap(Display *d, Colormap c)
}
declare 38 macosx {
    int XFreeColors(Display *d, Colormap c,
	    unsigned long *ulp, int i, unsigned long ul)
}
declare 39 macosx {
    int XFreeCursor(Display *d, Cursor c)
}
declare 40 macosx {
    int XFreeModifiermap(XModifierKeymap *x)
}
declare 41 macosx {
    Status XGetGeometry(Display *d, Drawable dr, Window *w, int *i1,
	    int *i2, unsigned int *ui1, unsigned int *ui2, unsigned int *ui3,
	    unsigned int *ui4)
}
declare 42 macosx {
    int XGetInputFocus(Display *d, Window *w, int *i)
}
declare 43 macosx {
    int XGetWindowProperty(Display *d, Window w, Atom a1, long l1, long l2,
	    Bool b, Atom a2, Atom *ap, int *ip, unsigned long *ulp1,
	    unsigned long *ulp2, unsigned char **cpp)
}
declare 44 macosx {
    Status XGetWindowAttributes(Display *d, Window w, XWindowAttributes *x)
}
declare 45 macosx {
    int XGrabKeyboard(Display *d, Window w, Bool b, int i1, int i2, Time t)
}
declare 46 macosx {
    int XGrabPointer(Display *d, Window w1, Bool b, unsigned int ui,
	    int i1, int i2, Window w2, Cursor c, Time t)
}
declare 47 macosx {
    KeyCode XKeysymToKeycode(Display *d, KeySym k)
}
declare 48 macosx {
    Status XLookupColor(Display *d, Colormap c1, _Xconst char *c2,
	    XColor *x1, XColor *x2)
}
declare 49 macosx {
    int XMapWindow(Display *d, Window w)
}
declare 50 macosx {
    int XMoveResizeWindow(Display *d, Window w, int i1, int i2,
	    unsigned int ui1, unsigned int ui2)
}
declare 51 macosx {
    int XMoveWindow(Display *d, Window w, int i1, int i2)
}
declare 52 macosx {
    int XNextEvent(Display *d, XEvent *x)
}
declare 53 macosx {
    int XPutBackEvent(Display *d, XEvent *x)
}
declare 54 macosx {
    int XQueryColors(Display *d, Colormap c, XColor *x, int i)
}
declare 55 macosx {
    Bool XQueryPointer(Display *d, Window w1, Window *w2, Window *w3,
	    int *i1, int *i2, int *i3, int *i4, unsigned int *ui)
}
declare 56 macosx {
    Status XQueryTree(Display *d, Window w1, Window *w2, Window *w3,
	    Window **w4, unsigned int *ui)
}
declare 57 macosx {
    int XRaiseWindow(Display *d, Window w)
}
declare 58 macosx {
    int XRefreshKeyboardMapping(XMappingEvent *x)
}
declare 59 macosx {
    int XResizeWindow(Display *d, Window w, unsigned int ui1,
	    unsigned int ui2)
}
declare 60 macosx {
    int XSelectInput(Display *d, Window w, long l)
}
declare 61 macosx {
    Status XSendEvent(Display *d, Window w, Bool b, long l, XEvent *x)
}
declare 62 macosx {
    int XSetCommand(Display *d, Window w, char **c, int i)
}
declare 63 macosx {
    int XSetIconName(Display *d, Window w, _Xconst char *c)
}
declare 64 macosx {
    int XSetInputFocus(Display *d, Window w, int i, Time t)
}
declare 65 macosx {
    int XSetSelectionOwner(Display *d, Atom a, Window w, Time t)
}
declare 66 macosx {
    int XSetWindowBackground(Display *d, Window w, unsigned long ul)
}
declare 67 macosx {
    int XSetWindowBackgroundPixmap(Display *d, Window w, Pixmap p)
}
declare 68 macosx {
    int XSetWindowBorder(Display *d, Window w, unsigned long ul)
}
declare 69 macosx {
    int XSetWindowBorderPixmap(Display *d, Window w, Pixmap p)
}
declare 70 macosx {
    int XSetWindowBorderWidth(Display *d, Window w, unsigned int ui)
}
declare 71 macosx {
    int XSetWindowColormap(Display *d, Window w, Colormap c)
}
declare 72 macosx {
    Bool XTranslateCoordinates(Display *d, Window w1, Window w2, int i1,
	    int i2, int *i3, int *i4, Window *w3)
}
declare 73 macosx {
    int XUngrabKeyboard(Display *d, Time t)
}
declare 74 macosx {
    int XUngrabPointer(Display *d, Time t)
}
declare 75 macosx {
    int XUnmapWindow(Display *d, Window w)
}
declare 76 macosx {
    int XWindowEvent(Display *d, Window w, long l, XEvent *x)
}
declare 77 macosx {
    void XDestroyIC(XIC x)
}
declare 78 macosx {
    Bool XFilterEvent(XEvent *x, Window w)
}
declare 79 macosx {
    int XmbLookupString(XIC xi, XKeyPressedEvent *xk, char *c, int i,
	    KeySym *k, Status *s)
}
declare 80 macosx {
    int TkPutImage(unsigned long *colors, int ncolors, Display *display,
	    Drawable d, GC gc, XImage *image, int src_x, int src_y,
	    int dest_x, int dest_y, unsigned int width, unsigned int height)
}
declare 81 macosx {
    int XSetClipRectangles(Display *display, GC gc, int clip_x_origin,
	    int clip_y_origin, XRectangle rectangles[], int n, int ordering)
}
declare 82 macosx {
    Status XParseColor(Display *display, Colormap map,
	    _Xconst char *spec, XColor *colorPtr)
}
declare 83 macosx {
    GC XCreateGC(Display *display, Drawable d,
	    unsigned long valuemask, XGCValues *values)
}
declare 84 macosx {
    int XFreeGC(Display *display, GC gc)
}
declare 85 macosx {
    Atom XInternAtom(Display *display, _Xconst char *atom_name,
	    Bool only_if_exists)
}
declare 86 macosx {
    int XSetBackground(Display *display, GC gc, unsigned long foreground)
}
declare 87 macosx {
    int XSetForeground(Display *display, GC gc, unsigned long foreground)
}
declare 88 macosx {
    int XSetClipMask(Display *display, GC gc, Pixmap pixmap)
}
declare 89 macosx {
    int XSetClipOrigin(Display *display, GC gc,
	    int clip_x_origin, int clip_y_origin)
}
declare 90 macosx {
    int XSetTSOrigin(Display *display, GC gc,
	    int ts_x_origin, int ts_y_origin)
}
declare 91 macosx {
    int XChangeGC(Display *d, GC gc, unsigned long mask, XGCValues *values)
}
declare 92 macosx {
    int XSetFont(Display *display, GC gc, Font font)
}
declare 93 macosx {
    int XSetArcMode(Display *display, GC gc, int arc_mode)
}
declare 94 macosx {
    int XSetStipple(Display *display, GC gc, Pixmap stipple)
}
declare 95 macosx {
    int XSetFillRule(Display *display, GC gc, int fill_rule)
}
declare 96 macosx {
    int XSetFillStyle(Display *display, GC gc, int fill_style)
}
declare 97 macosx {
    int XSetFunction(Display *display, GC gc, int function)
}
declare 98 macosx {
    int XSetLineAttributes(Display *display, GC gc, unsigned int line_width,
	    int line_style, int cap_style, int join_style)
}
declare 99 macosx {
    int _XInitImageFuncPtrs(XImage *image)
}
declare 100 macosx {
    XIC XCreateIC(XIM xim, ...)
}
declare 101 macosx {
    XVisualInfo *XGetVisualInfo(Display *display, long vinfo_mask,
	    XVisualInfo *vinfo_template, int *nitems_return)
}
declare 102 macosx {
    void XSetWMClientMachine(Display *display, Window w,
	    XTextProperty *text_prop)
}
declare 103 macosx {
    Status XStringListToTextProperty(char **list, int count,
	    XTextProperty *text_prop_return)
}
declare 104 macosx {
    int XDrawLine(Display *d, Drawable dr, GC g, int x1, int y1,
	    int x2, int y2)
}
declare 105 macosx {
    int XWarpPointer(Display *d, Window s, Window dw, int sx, int sy,
	    unsigned int sw, unsigned int sh, int dx, int dy)
}
declare 106 macosx {
    int XFillRectangle(Display *display, Drawable d, GC gc,
	    int x, int y, unsigned int width, unsigned int height)
}

# New in Tk 8.6
declare 107 macosx {
    int XFlush(Display *display)
}
declare 108 macosx {
    int XGrabServer(Display *display)
}
declare 109 macosx {
    int XUngrabServer(Display *display)
}
declare 110 macosx {
    int XFree(void *data)
}
declare 111 macosx {
    int XNoOp(Display *display)
}
declare 112 macosx {
    XAfterFunction XSynchronize(Display *display, Bool onoff)
}
declare 113 macosx {
    int XSync(Display *display, Bool discard)
}
declare 114 macosx {
    VisualID XVisualIDFromVisual(Visual *visual)
}

# For tktreectrl
declare 120 macosx {
    int XOffsetRegion(Region rgn, int dx, int dy)
}
declare 121 macosx {
    int XUnionRegion(Region srca, Region srcb, Region dr_return)
}

# For 3dcanvas
declare 122 macosx {
    Window XCreateWindow(Display *display, Window parent, int x, int y,
	    unsigned int width, unsigned int height,
	    unsigned int border_width, int depth, unsigned int clazz,
	    Visual *visual, unsigned long value_mask,
	    XSetWindowAttributes *attributes)
}

# Various, e.g. for stub-enabled BLT
declare 129 macosx {
    int XLowerWindow(Display *d, Window w)
}
declare 130 macosx {
    int XFillArcs(Display *d, Drawable dr, GC gc, XArc *a, int n)
}
declare 131 macosx {
    int XDrawArcs(Display *d, Drawable dr, GC gc, XArc *a, int n)
}
declare 132 macosx {
    int XDrawRectangles(Display *d, Drawable dr, GC gc, XRectangle *r, int n)
}
declare 133 macosx {
    int XDrawSegments(Display *d, Drawable dr, GC gc, XSegment *s, int n)
}
declare 134 macosx {
    int XDrawPoint(Display *d, Drawable dr, GC gc, int x, int y)
}
declare 135 macosx {
    int XDrawPoints(Display *d, Drawable dr, GC gc, XPoint *p, int n, int m)
}
declare 136 macosx {
    int XReparentWindow(Display *d, Window w, Window p, int x, int y)
}
declare 137 macosx {
    int XPutImage(Display *d, Drawable dr, GC gc, XImage *im,
	    int sx, int sy, int dx, int dy,
	    unsigned int w, unsigned int h)
}
declare 138 macosx {
    Region XPolygonRegion(XPoint *pts, int n, int rule)
}
declare 139 macosx {
    int XPointInRegion(Region rgn, int x, int y)
}
# For XIM
declare 140 macosx {
    XVaNestedList XVaCreateNestedList(int dummy, ...)
}
declare 141 macosx {
    char *XSetICValues(XIC xic, ...)
}
declare 142 macosx {
    char *XGetICValues(XIC xic, ...)
}
declare 143 macosx {
    void XSetICFocus(XIC xic)
}
declare 147 macosx {
    void XFreeFontSet(Display *display, XFontSet fontset)
}
declare 148 macosx {
    int XCloseIM(XIM im)
}
declare 149 macosx {
    Bool XRegisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rbd,
	    char *res_name, char *res_class, XIDProc callback, XPointer client_data)
}
declare 150 macosx {
    Bool XUnregisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rbd,
	    char *res_name, char *res_class, XIDProc callback, XPointer client_data)
}
declare 151 macosx {
    char *XSetLocaleModifiers(const char *modifier_list)
}
declare 152 macosx {
    XIM XOpenIM(Display *dpy, struct _XrmHashBucketRec *rdb, char *res_name,
	    char *res_class)
}
declare 153 macosx {
    char *XGetIMValues(XIM im, ...)
}
declare 154 macosx {
    char *XSetIMValues(XIM im, ...)
}
declare 155 macosx {
    XFontSet XCreateFontSet(Display *display, _Xconst char *base_font_name_list,
	    char ***missing_charset_list, int *missing_charset_count, char **def_string)
}
declare 156 macosx {
    void XFreeStringList(char **list)
}
declare 157 macosx {
    KeySym XkbKeycodeToKeysym(Display *d, unsigned int k, int g, int i)
}
declare 158 macosx {
    Display *XkbOpenDisplay(const char *name, int *ev_rtrn, int *err_rtrn,
	    int *major_rtrn, int *minor_rtrn, int *reason)
}

# Local Variables:
# mode: tcl
# End:
