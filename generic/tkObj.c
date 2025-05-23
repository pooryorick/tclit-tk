/*
 * tkObj.c --
 *
 *	This file contains functions that implement the common Tk object types
 *
 * Copyright © 1997 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * Copyright © 2024-2025 Nathan Coulter

 * You may distribute and/or modify this program under the terms of the GNU
 * Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.

 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "tkInt.h"

/*
 * The following structure is the internal representation for pixel objects.
 */

typedef struct PixelRep {
    double value;
    int units;
    Tk_Window tkwin;
    int returnValue;
} PixelRep;

#define SIMPLE_PIXELREP(objPtr)				\
    ((objPtr)->internalRep.twoPtrValue.ptr2 == 0)

#define SET_SIMPLEPIXEL(objPtr, intval)			\
    (objPtr)->internalRep.twoPtrValue.ptr1 = INT2PTR(intval);	\
    (objPtr)->internalRep.twoPtrValue.ptr2 = 0

#define GET_SIMPLEPIXEL(objPtr)				\
    (PTR2INT((objPtr)->internalRep.twoPtrValue.ptr1))

#define SET_COMPLEXPIXEL(objPtr, repPtr)		\
    (objPtr)->internalRep.twoPtrValue.ptr1 = NULL;		\
    (objPtr)->internalRep.twoPtrValue.ptr2 = repPtr

#define GET_COMPLEXPIXEL(objPtr)			\
    ((PixelRep *) (objPtr)->internalRep.twoPtrValue.ptr2)

/*
 * One of these structures is created per thread to store thread-specific
 * data. In this case, it is used to contain references to selected
 * Tcl_ObjTypes that we can use as screen distances without conversion. The
 * "dataKey" below is used to locate the ThreadSpecificData for the current
 * thread.
 */

typedef struct {
    Tcl_ObjType *doubleTypePtr;
} ThreadSpecificData;
static Tcl_ThreadDataKey dataKey;

/*
 * The following structure is the internal representation for mm objects.
 */

typedef struct MMRep {
    double value;
    int units;
    Tk_Window tkwin;
    double returnValue;
} MMRep;

/*
 * The following structure is the internal representation for window objects.
 * A WindowRep caches name-to-window lookups. The cache is invalid if tkwin is
 * NULL or if mainPtr->deletionEpoch does not match epoch.
 */

typedef struct WindowRep {
    Tk_Window tkwin;		/* Cached window; NULL if not found. */
    TkMainInfo *mainPtr;	/* MainWindow associated with tkwin. */
    size_t epoch;			/* Value of mainPtr->deletionEpoch at last
				 * successful lookup. */
} WindowRep;

/*
 * Prototypes for functions defined later in this file:
 */

static void		DupMMInternalRep(Tcl_Obj *srcPtr, Tcl_Obj *copyPtr);
static void		DupPixelInternalRep(Tcl_Obj *srcPtr, Tcl_Obj*copyPtr);
static void		DupWindowInternalRep(Tcl_Obj *srcPtr,Tcl_Obj*copyPtr);
static void		FreeMMInternalRep(Tcl_Obj *objPtr);
static void		FreePixelInternalRep(Tcl_Obj *objPtr);
static void		FreeWindowInternalRep(Tcl_Obj *objPtr);
static ThreadSpecificData *GetTypeCache(void);
static void		UpdateStringOfMM(Tcl_Obj *objPtr);
static int		SetMMFromAny(Tcl_Interp *interp, Tcl_Obj *objPtr);
static int		SetPixelFromAny(Tcl_Interp *interp, Tcl_Obj *objPtr);
static int		SetWindowFromAny(Tcl_Interp *interp, Tcl_Obj *objPtr);

/*
 * The following structure defines the implementation of the "pixel" Tcl
 * object, used for measuring distances. The pixel object remembers its
 * initial display-independent settings.
 */

TkObjType pixelObjType = {
	NULL, 0
};

int
TkLengthOne(
	TCL_UNUSED(Tcl_Interp *)
    ,TCL_UNUSED(Tcl_Obj *)
	,Tcl_Size *lengthPtr
) {
	*lengthPtr = 1;
	return TCL_OK;
}

/*
 * The following structure defines the implementation of the "pixel" Tcl
 * object, used for measuring distances. The pixel object remembers its
 * initial display-independent settings.
 */

TkObjType mmObjType = {
	NULL, 0
};

/*
 * The following structure defines the implementation of the "window"
 * Tcl object.
 */

TkObjType windowObjType = {
	NULL, 0
};


void TkObjInit() {
    TkObjType *tmpPtr = (TkObjType *)&pixelObjType;
    Tcl_ObjInterface *oiPtr = Tcl_NewObjInterface();
    Tcl_ObjType *otPtr = Tcl_NewObjType();
    Tcl_ObjTypeSetName(otPtr, (char *)"pixel");
    Tcl_ObjTypeSetVersion(otPtr, 2);
    Tcl_ObjTypeSetFreeInternalRepProc(otPtr, FreePixelInternalRep);
    Tcl_ObjTypeSetDupInternalRepProc(otPtr, DupPixelInternalRep);
    Tcl_ObjInterfaceSetFnListLength(oiPtr ,TkLengthOne);
    Tcl_ObjTypeSetInterface(otPtr ,oiPtr);
    tmpPtr->objTypePtr = otPtr;

    tmpPtr = (TkObjType *)&mmObjType;
    oiPtr = Tcl_NewObjInterface();
    otPtr = Tcl_NewObjType();
    Tcl_ObjTypeSetName(otPtr, (char *)"mm");
    Tcl_ObjTypeSetVersion(otPtr, 2);
    Tcl_ObjTypeSetFreeInternalRepProc(otPtr, FreeMMInternalRep);
    Tcl_ObjTypeSetDupInternalRepProc(otPtr, DupMMInternalRep);
	Tcl_ObjTypeSetUpdateStringProc(otPtr, UpdateStringOfMM);
    Tcl_ObjInterfaceSetFnListLength(oiPtr ,TkLengthOne);
    Tcl_ObjTypeSetInterface(otPtr ,oiPtr);
    tmpPtr->objTypePtr = otPtr;

    tmpPtr = (TkObjType *)&windowObjType;
    otPtr = Tcl_NewObjType();
    Tcl_ObjTypeSetName(otPtr, (char *)"window");
    Tcl_ObjTypeSetVersion(otPtr, 1);
    Tcl_ObjTypeSetFreeInternalRepProc(otPtr, FreeWindowInternalRep);
    Tcl_ObjTypeSetDupInternalRepProc(otPtr, DupWindowInternalRep);
    tmpPtr->objTypePtr = otPtr;

    return;
};

/*
 *----------------------------------------------------------------------
 *
 * GetTypeCache --
 *
 *	Get (and build if necessary) the cache of useful Tcl object types for
 *	comparisons in the conversion functions.  This allows optimized checks
 *	for standard cases.
 *
 *----------------------------------------------------------------------
 */

static ThreadSpecificData *
GetTypeCache(void)
{
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
	    Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

    if (tsdPtr->doubleTypePtr == NULL) {
	/* Smart initialization of doubleTypePtr without
	 * hash-table lookup or creating complete Tcl_Obj's */
	Tcl_Obj obj;
	obj.length = 3;
	obj.bytes = (char *)"0.0";
	obj.typePtr = NULL;
	Tcl_GetDoubleFromObj(NULL, &obj, &obj.internalRep.doubleValue);
	tsdPtr->doubleTypePtr = obj.typePtr;
    }
    return tsdPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * TkGetIntForIndex --
 *
 *	Almost the same as Tcl_GetIntForIndex, but it retrieves an int. Accepts
 *	"" (empty string) as well.
 *
 * Results:
 *	The return value is a standard Tcl object result.
 *
 * Side effects:
 *	None
 *
 *----------------------------------------------------------------------
 */

int
TkGetIntForIndex(
    Tcl_Obj *indexObj,
    Tcl_Size end,
    int lastOK,
    Tcl_Size *indexPtr)
{
    if (indexObj == NULL) {
	*indexPtr = TCL_INDEX_NONE;
	return TCL_OK;
    }
    if (Tcl_GetIntForIndex(NULL, indexObj, end + lastOK, indexPtr) != TCL_OK) {
	const char *value = Tcl_GetString(indexObj);
	if (!*value) {
	    /* empty string */
	    *indexPtr = (end == -1) ? -1 - TCL_SIZE_MAX : TCL_INDEX_NONE;
	    return TCL_OK;
	}
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * GetPixelsFromObjEx --
 *
 *	Attempt to return a pixel value from the Tcl object "objPtr". If the
 *	object is not already a pixel value, an attempt will be made to
 *	convert it to one.
 *
 * Results:
 *	The return value is a standard Tcl object result. If an error occurs
 *	during conversion, an error message is left in the interpreter's
 *	result unless "interp" is NULL.
 *
 * Side effects:
 *	If the object is not already a pixel, the conversion will free any old
 *	internal representation.
 *
 *----------------------------------------------------------------------
 */

static
int
GetPixelsFromObjEx(
    Tcl_Interp *interp,	/* Used for error reporting if not NULL. */
    Tk_Window tkwin,
    Tcl_Obj *objPtr,		/* The object from which to get pixels. */
    int *intPtr,
    double *dblPtr)		/* Places to store resulting pixels. */
{
    int result, fresh;
    double d;
    PixelRep *pixelPtr;
    static const double bias[] = {
	1.0,	10.0,	25.4,	0.35277777777777775 /*25.4 / 72.0*/
    };

    if (objPtr->typePtr != pixelObjType.objTypePtr) {

	if (Tcl_GetDoubleFromObj(NULL, objPtr, &d) == TCL_OK) {
	    if (dblPtr != NULL) {
		*dblPtr = d;
	    }
	    *intPtr = (int) (d<0 ? d-0.5 : d+0.5);
	    return TCL_OK;
	}
    }

 retry:
    fresh = (objPtr->typePtr != pixelObjType.objTypePtr);
    if (fresh) {
	result = SetPixelFromAny(interp, objPtr);
	if (result != TCL_OK) {
	    return result;
	}
    }

    if (SIMPLE_PIXELREP(objPtr)) {
	*intPtr = GET_SIMPLEPIXEL(objPtr);
	if (dblPtr) {
	    *dblPtr = (double) (*intPtr);
	}
    } else {
	pixelPtr = GET_COMPLEXPIXEL(objPtr);
	if ((!fresh) && (pixelPtr->tkwin != tkwin)) {
	    /*
	     * In the case of exo-screen conversions of non-pixels, we force a
	     * recomputation from the string.
	     */

	    FreePixelInternalRep(objPtr);
	    goto retry;
	}
	if ((pixelPtr->tkwin != tkwin) || dblPtr) {
	    d = pixelPtr->value;
	    if (pixelPtr->units >= 0) {
		d *= bias[pixelPtr->units] * WidthOfScreen(Tk_Screen(tkwin));
		d /= WidthMMOfScreen(Tk_Screen(tkwin));
	    }
	    pixelPtr->returnValue = (int) (d<0 ? d-0.5 : d+0.5);
	    pixelPtr->tkwin = tkwin;
	    if (dblPtr) {
		*dblPtr = d;
	    }
	}
	*intPtr = pixelPtr->returnValue;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetPixelsFromObj --
 *
 *	Attempt to return a pixel value from the Tcl object "objPtr". If the
 *	object is not already a pixel value, an attempt will be made to
 *	convert it to one.
 *
 * Results:
 *	The return value is a standard Tcl object result. If an error occurs
 *	during conversion, an error message is left in the interpreter's
 *	result unless "interp" is NULL.
 *
 * Side effects:
 *	If the object is not already a pixel, the conversion will free any old
 *	internal representation.
 *
 *----------------------------------------------------------------------
 */

int
Tk_GetPixelsFromObj(
    Tcl_Interp *interp,	/* Used for error reporting if not NULL. */
    Tk_Window tkwin,
    Tcl_Obj *objPtr,		/* The object from which to get pixels. */
    int *intPtr)		/* Place to store resulting pixels. */
{
    return GetPixelsFromObjEx(interp, tkwin, objPtr, intPtr, NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetDoublePixelsFromObj --
 *
 *	Attempt  to  return   a  double  pixel  value  from   the  Tcl  object
 *	"objPtr". If the object is not  already a pixel value, an attempt will
 *	be made to convert it to one, the internal unit being pixels.
 *
 * Results:
 *	The return value is a standard Tcl object result. If an error occurs
 *	during conversion, an error message is left in the interpreter's
 *	result unless "interp" is NULL.
 *
 * Side effects:
 *	If the object is not already a pixel, the conversion will free any old
 *	internal representation.
 *
 *----------------------------------------------------------------------
 */

int
Tk_GetDoublePixelsFromObj(
    Tcl_Interp *interp,	/* Used for error reporting if not NULL. */
    Tk_Window tkwin,
    Tcl_Obj *objPtr,		/* The object from which to get pixels. */
    double *doublePtr)		/* Place to store resulting pixels. */
{
    double d;
    int result, val;

    result = GetPixelsFromObjEx(interp, tkwin, objPtr, &val, &d);
    if (result != TCL_OK) {
	return result;
    }
    if (objPtr->typePtr == pixelObjType.objTypePtr && !SIMPLE_PIXELREP(objPtr)) {
	PixelRep *pixelPtr = GET_COMPLEXPIXEL(objPtr);

	if (pixelPtr->units >= 0) {
	    /*
	     * Internally "shimmer" to pixel units.
	     */

	    pixelPtr->units = -1;
	    pixelPtr->value = d;
	}
    }
    *doublePtr = d;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * FreePixelInternalRep --
 *
 *	Deallocate the storage associated with a pixel object's internal
 *	representation.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Frees objPtr's internal representation and sets objPtr's internalRep
 *	to NULL.
 *
 *----------------------------------------------------------------------
 */

static void
FreePixelInternalRep(
    Tcl_Obj *objPtr)		/* Pixel object with internal rep to free. */
{
    if (!SIMPLE_PIXELREP(objPtr)) {
	PixelRep *pixelPtr = GET_COMPLEXPIXEL(objPtr);

	ckfree(pixelPtr);
    }
    SET_SIMPLEPIXEL(objPtr, 0);
    objPtr->typePtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * DupPixelInternalRep --
 *
 *	Initialize the internal representation of a pixel Tcl_Obj to a copy of
 *	the internal representation of an existing pixel object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	copyPtr's internal rep is set to the pixel corresponding to srcPtr's
 *	internal rep.
 *
 *----------------------------------------------------------------------
 */

static void
DupPixelInternalRep(
    Tcl_Obj *srcPtr,	/* Object with internal rep to copy. */
    Tcl_Obj *copyPtr)	/* Object with internal rep to set. */
{
    copyPtr->typePtr = srcPtr->typePtr;

    if (SIMPLE_PIXELREP(srcPtr)) {
	SET_SIMPLEPIXEL(copyPtr, GET_SIMPLEPIXEL(srcPtr));
    } else {
	PixelRep *oldPtr, *newPtr;

	oldPtr = GET_COMPLEXPIXEL(srcPtr);
	newPtr = (PixelRep *)ckalloc(sizeof(PixelRep));
	newPtr->value = oldPtr->value;
	newPtr->units = oldPtr->units;
	newPtr->tkwin = oldPtr->tkwin;
	newPtr->returnValue = oldPtr->returnValue;
	SET_COMPLEXPIXEL(copyPtr, newPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * SetPixelFromAny --
 *
 *	Attempt to generate a pixel internal form for the Tcl object "objPtr".
 *
 * Results:
 *	The return value is a standard Tcl result. If an error occurs during
 *	conversion, an error message is left in the interpreter's result
 *	unless "interp" is NULL.
 *
 * Side effects:
 *	If no error occurs, a pixel representation of the object is stored
 *	internally and the type of "objPtr" is set to pixel.
 *
 *----------------------------------------------------------------------
 */

static int
SetPixelFromAny(
    Tcl_Interp *interp,		/* Used for error reporting if not NULL. */
    Tcl_Obj *objPtr)		/* The object to convert. */
{
    ThreadSpecificData *typeCache = GetTypeCache();
    const Tcl_ObjType *typePtr;
    char *string;
    char *rest;
    double d;
    int i, units;

    if (objPtr->typePtr != typeCache->doubleTypePtr
	    && Tcl_GetIntFromObj(NULL, objPtr, &units) == TCL_OK) {
	d = (double) units;
	units = -1;

	/*
	 * In the case of ints, we need to ensure that a valid string exists
	 * in order for int-but-not-string objects to be converted back to
	 * ints again from pixel obj types.
	 */

	(void) Tcl_GetString(objPtr);
    } else if (Tcl_GetDoubleFromObj(NULL, objPtr, &d) == TCL_OK) {
	units = -1;
    } else {
	char savechar;
	string = Tcl_GetString(objPtr);

	rest = string + strlen(string);
	while ((rest > string) && isspace(UCHAR(rest[-1]))) {
	    --rest; /* skip all spaces at the end */
	}
	if (rest > string) {
	    --rest; /* point to the character just before the last space */
	}
	if (rest == string) {
	error:
	    if (interp != NULL) {
		Tcl_SetObjResult(interp, Tcl_ObjPrintf(
			"expected screen distance but got \"%.50s\"", string));
		Tcl_SetErrorCode(interp, "TK", "VALUE", "PIXELS", (char *)NULL);
	    }
	    return TCL_ERROR;
	}

	switch (*rest) {
	case 'm':
	    units = 0;
	    break;
	case 'c':
	    units = 1;
	    break;
	case 'i':
	    units = 2;
	    break;
	case 'p':
	    units = 3;
	    break;
	default:
	    goto error;
	}
	savechar = *rest;
	*rest = '\0';
	if (Tcl_GetDouble(NULL, string, &d) != TCL_OK) {
	    *rest = savechar;
	    goto error;
	}
	*rest = savechar;
    }

    /*
     * Free the old internalRep before setting the new one.
     */

    typePtr = objPtr->typePtr;
    if ((typePtr != NULL) && (typePtr->freeIntRepProc != NULL)) {
	typePtr->freeIntRepProc(objPtr);
    }

    objPtr->typePtr = pixelObjType.objTypePtr;

    i = (int) d;
    if ((units < 0) && (i == d)) {
	SET_SIMPLEPIXEL(objPtr, i);
    } else {
	PixelRep *pixelPtr = (PixelRep *)ckalloc(sizeof(PixelRep));

	pixelPtr->value = d;
	pixelPtr->units = units;
	pixelPtr->tkwin = NULL;
	pixelPtr->returnValue = i;
	SET_COMPLEXPIXEL(objPtr, pixelPtr);
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetMMFromObj --
 *
 *	Attempt to return an mm value from the Tcl object "objPtr". If the
 *	object is not already an mm value, an attempt will be made to convert
 *	it to one.
 *
 * Results:
 *	The return value is a standard Tcl object result. If an error occurs
 *	during conversion, an error message is left in the interpreter's
 *	result unless "interp" is NULL.
 *
 * Side effects:
 *	If the object is not already a pixel, the conversion will free any old
 *	internal representation.
 *
 *----------------------------------------------------------------------
 */

int
Tk_GetMMFromObj(
    Tcl_Interp *interp,	/* Used for error reporting if not NULL. */
    Tk_Window tkwin,
    Tcl_Obj *objPtr,		/* The object from which to get mms. */
    double *doublePtr)		/* Place to store resulting millimeters. */
{
    int result;
    double d;
    MMRep *mmPtr;
    static const double bias[] = {
	10.0,	25.4,	1.0,	0.35277777777777775 /*25.4 / 72.0*/
    };

    if (objPtr->typePtr != mmObjType.objTypePtr) {
	result = SetMMFromAny(interp, objPtr);
	if (result != TCL_OK) {
	    return result;
	}
    }

    mmPtr = (MMRep *)objPtr->internalRep.twoPtrValue.ptr1;
    if (mmPtr->tkwin != tkwin) {
	d = mmPtr->value;
	if (mmPtr->units == -1) {
	    d /= WidthOfScreen(Tk_Screen(tkwin));
	    d *= WidthMMOfScreen(Tk_Screen(tkwin));
	} else {
	    d *= bias[mmPtr->units];
	}
	mmPtr->tkwin = tkwin;
	mmPtr->returnValue = d;
    }
    *doublePtr = mmPtr->returnValue;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * FreeMMInternalRep --
 *
 *	Deallocate the storage associated with a mm object's internal
 *	representation.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Frees objPtr's internal representation and sets objPtr's internalRep
 *	to NULL.
 *
 *----------------------------------------------------------------------
 */

static void
FreeMMInternalRep(
    Tcl_Obj *objPtr)		/* MM object with internal rep to free. */
{
    ckfree(objPtr->internalRep.twoPtrValue.ptr1);
    objPtr->internalRep.twoPtrValue.ptr1 = NULL;
    objPtr->typePtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * DupMMInternalRep --
 *
 *	Initialize the internal representation of a pixel Tcl_Obj to a copy of
 *	the internal representation of an existing pixel object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	copyPtr's internal rep is set to the pixel corresponding to srcPtr's
 *	internal rep.
 *
 *----------------------------------------------------------------------
 */

static void
DupMMInternalRep(
    Tcl_Obj *srcPtr,	/* Object with internal rep to copy. */
    Tcl_Obj *copyPtr)	/* Object with internal rep to set. */
{
    MMRep *oldPtr, *newPtr;

    copyPtr->typePtr = srcPtr->typePtr;
    oldPtr = (MMRep *)srcPtr->internalRep.twoPtrValue.ptr1;
    newPtr = (MMRep *)ckalloc(sizeof(MMRep));
    newPtr->value = oldPtr->value;
    newPtr->units = oldPtr->units;
    newPtr->tkwin = oldPtr->tkwin;
    newPtr->returnValue = oldPtr->returnValue;
    copyPtr->internalRep.twoPtrValue.ptr1 = newPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateStringOfMM --
 *
 *	Update the string representation for a pixel Tcl_Obj this function is
 *	only called, if the pixel Tcl_Obj has no unit, because with units the
 *	string representation is created by SetMMFromAny
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The object's string is set to a valid string that results from the
 *	double-to-string conversion.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateStringOfMM(
    Tcl_Obj *objPtr)   /* pixel obj with string rep to update. */
{
    MMRep *mmPtr;
    char buffer[TCL_DOUBLE_SPACE];
    size_t len;

    mmPtr = (MMRep *)objPtr->internalRep.twoPtrValue.ptr1;
    /* assert( mmPtr->units == -1 && objPtr->bytes == NULL ); */
    if ((mmPtr->units != -1) || (objPtr->bytes != NULL)) {
	Tcl_Panic("UpdateStringOfMM: false precondition");
    }

    Tcl_PrintDouble(NULL, mmPtr->value, buffer);
    len = strlen(buffer);

    objPtr->bytes = (char *)ckalloc(len + 1);
    strcpy(objPtr->bytes, buffer);
    objPtr->length = len;
}

/*
 *----------------------------------------------------------------------
 *
 * SetMMFromAny --
 *
 *	Attempt to generate a mm internal form for the Tcl object "objPtr".
 *
 * Results:
 *	The return value is a standard Tcl result. If an error occurs during
 *	conversion, an error message is left in the interpreter's result
 *	unless "interp" is NULL.
 *
 * Side effects:
 *	If no error occurs, a mm representation of the object is stored
 *	internally and the type of "objPtr" is set to mm.
 *
 *----------------------------------------------------------------------
 */

static int
SetMMFromAny(
    Tcl_Interp *interp,		/* Used for error reporting if not NULL. */
    Tcl_Obj *objPtr)		/* The object to convert. */
{
    ThreadSpecificData *typeCache = GetTypeCache();
    const Tcl_ObjType *typePtr;
    char *string;
    char *rest;
    double d;
    int units;
    MMRep *mmPtr;

    if (objPtr->typePtr != typeCache->doubleTypePtr
	    && Tcl_GetIntFromObj(NULL, objPtr, &units) == TCL_OK) {
	d = (double) units;
	units = -1;

	/*
	 * In the case of ints, we need to ensure that a valid string exists
	 * in order for int-but-not-string objects to be converted back to
	 * ints again from mm obj types.
	 */

	(void) Tcl_GetString(objPtr);
    } else if (Tcl_GetDoubleFromObj(NULL, objPtr, &d) == TCL_OK) {
	units = -1;
    } else {
	char savechar;

	/*
	 * It wasn't a known int or double, so parse it.
	 */

	string = Tcl_GetString(objPtr);

	rest = string + strlen(string);
	while ((rest > string) && isspace(UCHAR(rest[-1]))) {
	    --rest; /* skip all spaces at the end */
	}
	if (rest > string) {
	    --rest; /* point to the character just before the last space */
	}
	if (rest == string) {
	error:
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		    "expected screen distance but got \"%.50s\"", string));
	    Tcl_SetErrorCode(interp, "TK", "VALUE", "PIXELS", (char *)NULL);
	    return TCL_ERROR;
	}
	switch (*rest) {
	case 'c':
	    units = 0;
	    break;
	case 'i':
	    units = 1;
	    break;
	case 'm':
	    units = 2;
	    break;
	case 'p':
	    units = 3;
	    break;
	default:
	    goto error;
	}
	savechar = *rest;
	*rest = '\0';
	if (Tcl_GetDouble(NULL, string, &d) != TCL_OK) {
	    *rest = savechar;
	    goto error;
	}
	*rest = savechar;
    }

    /*
     * Free the old internalRep before setting the new one.
     */

    typePtr = objPtr->typePtr;
    if ((typePtr != NULL) && (typePtr->freeIntRepProc != NULL)) {
	typePtr->freeIntRepProc(objPtr);
    }

    objPtr->typePtr = mmObjType.objTypePtr;

    mmPtr = (MMRep *)ckalloc(sizeof(MMRep));
    mmPtr->value = d;
    mmPtr->units = units;
    mmPtr->tkwin = NULL;
    mmPtr->returnValue	= d;

    objPtr->internalRep.twoPtrValue.ptr1 = mmPtr;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TkGetWindowFromObj --
 *
 *	Attempt to return a Tk_Window from the Tcl object "objPtr". If the
 *	object is not already a Tk_Window, an attempt will be made to convert
 *	it to one.
 *
 * Results:
 *	The return value is a standard Tcl object result. If an error occurs
 *	during conversion, an error message is left in the interpreter's
 *	result unless "interp" is NULL.
 *
 * Side effects:
 *	If the object is not already a Tk_Window, the conversion will free any
 *	old internal representation.
 *
 *----------------------------------------------------------------------
 */

int
TkGetWindowFromObj(
    Tcl_Interp *interp,	/* Used for error reporting if not NULL. */
    Tk_Window tkwin,		/* A token to get the main window from. */
    Tcl_Obj *objPtr,		/* The object from which to get window. */
    Tk_Window *windowPtr)	/* Place to store resulting window. */
{
    TkMainInfo *mainPtr = ((TkWindow *) tkwin)->mainPtr;
    WindowRep *winPtr;

    if (objPtr->typePtr != windowObjType.objTypePtr) {
	int result = SetWindowFromAny(interp, objPtr);
	if (result != TCL_OK) {
	    return result;
	}
    }

    winPtr = (WindowRep *)objPtr->internalRep.twoPtrValue.ptr1;
    if (winPtr->tkwin == NULL
	    || winPtr->mainPtr == NULL
	    || winPtr->mainPtr != mainPtr
	    || winPtr->epoch != mainPtr->deletionEpoch) {
	/*
	 * Cache is invalid.
	 */

	winPtr->tkwin = Tk_NameToWindow(interp,
		Tcl_GetString(objPtr), tkwin);
	if (winPtr->tkwin == NULL) {
	    /* ASSERT: Tk_NameToWindow has left error message in interp */
	    return TCL_ERROR;
	}

	winPtr->mainPtr = mainPtr;
	winPtr->epoch = mainPtr ? mainPtr->deletionEpoch : 0;
    }

    *windowPtr = winPtr->tkwin;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetWindowFromAny --
 *
 *	Generate a windowObj internal form for the Tcl object "objPtr".
 *
 * Results:
 *	Always returns TCL_OK.
 *
 * Side effects:
 *	Sets objPtr's internal representation to an uninitialized windowObj.
 *	Frees the old internal representation, if any.
 *
 * See also:
 *	TkGetWindowFromObj, which initializes the WindowRep cache.
 *
 *----------------------------------------------------------------------
 */

static int
SetWindowFromAny(
    TCL_UNUSED(Tcl_Interp *),
    Tcl_Obj *objPtr)	/* The object to convert. */
{
    const Tcl_ObjType *typePtr;
    WindowRep *winPtr;

    /*
     * Free the old internalRep before setting the new one.
     */

    Tcl_GetString(objPtr);
    typePtr = objPtr->typePtr;
    if ((typePtr != NULL) && (typePtr->freeIntRepProc != NULL)) {
	typePtr->freeIntRepProc(objPtr);
    }

    winPtr = (WindowRep *)ckalloc(sizeof(WindowRep));
    winPtr->tkwin = NULL;
    winPtr->mainPtr = NULL;
    winPtr->epoch = 0;

    objPtr->internalRep.twoPtrValue.ptr1 = winPtr;
    objPtr->typePtr = windowObjType.objTypePtr;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * DupWindowInternalRep --
 *
 *	Initialize the internal representation of a window Tcl_Obj to a copy
 *	of the internal representation of an existing window object.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	copyPtr's internal rep is set to refer to the same window as srcPtr's
 *	internal rep.
 *
 *----------------------------------------------------------------------
 */

static void
DupWindowInternalRep(
    Tcl_Obj *srcPtr,
    Tcl_Obj *copyPtr)
{
    WindowRep *oldPtr, *newPtr;

    oldPtr = (WindowRep *)srcPtr->internalRep.twoPtrValue.ptr1;
    newPtr = (WindowRep *)ckalloc(sizeof(WindowRep));
    newPtr->tkwin = oldPtr->tkwin;
    newPtr->mainPtr = oldPtr->mainPtr;
    newPtr->epoch = oldPtr->epoch;
    copyPtr->internalRep.twoPtrValue.ptr1 = newPtr;
    copyPtr->typePtr = srcPtr->typePtr;
}

/*
 *----------------------------------------------------------------------
 *
 * FreeWindowInternalRep --
 *
 *	Deallocate the storage associated with a window object's internal
 *	representation.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Frees objPtr's internal representation and sets objPtr's internalRep
 *	to NULL.
 *
 *----------------------------------------------------------------------
 */

static void
FreeWindowInternalRep(
    Tcl_Obj *objPtr)		/* Window object with internal rep to free. */
{
    ckfree(objPtr->internalRep.twoPtrValue.ptr1);
    objPtr->internalRep.twoPtrValue.ptr1 = NULL;
    objPtr->typePtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_NewWindowObj --
 *
 *	This function allocates a new Tcl_Obj that refers to a particular
 *	Tk window.
 *
 * Results:
 *	A standard Tcl object reference, with refcount 0.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Tcl_Obj *
Tk_NewWindowObj(
    Tk_Window tkwin)
{
    Tcl_Obj *objPtr = Tcl_NewStringObj(Tk_PathName(tkwin), TCL_INDEX_NONE);
    TkMainInfo *mainPtr = ((TkWindow *) tkwin)->mainPtr;
    WindowRep *winPtr;

    SetWindowFromAny(NULL, objPtr);

    winPtr = (WindowRep *)objPtr->internalRep.twoPtrValue.ptr1;
    winPtr->tkwin = tkwin;
    winPtr->mainPtr = mainPtr;
    winPtr->epoch = mainPtr->deletionEpoch;
    return objPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * TkParsePadAmount --
 *
 *	This function parses a padding specification and returns the
 *	appropriate padding values. A padding specification can be either a
 *	single pixel width, or a list of two pixel widths. If a single pixel
 *	width, the amount specified is used for padding on both sides. If two
 *	amounts are specified, then they specify the left/right or top/bottom
 *	padding.
 *
 * Results:
 *	A standard Tcl return value.
 *
 * Side effects:
 *	An error message is written to the interpreter if something is not
 *	right.
 *
 *----------------------------------------------------------------------
 */

int
TkParsePadAmount(
    Tcl_Interp *interp,		/* Interpreter for error reporting. */
    Tk_Window tkwin,		/* A window.  Needed by Tk_GetPixels() */
    Tcl_Obj *specObj,		/* The argument to "-padx", "-pady", "-ipadx",
				 * or "-ipady". The thing to be parsed. */
    int *halfPtr,		/* Write the left/top part of padding here */
    int *allPtr)		/* Write the total padding here */
{
    int firstInt, secondInt;    /* The two components of the padding */
    Tcl_Size objc;			/* The length of the list (should be 1 or 2) */
    Tcl_Obj **objv;		/* The objects in the list */

    /*
     * Check for a common case where a single object would otherwise be
     * shimmered between a list and a pixel spec.
     */

    if (specObj->typePtr == pixelObjType.objTypePtr) {
	if (Tk_GetPixelsFromObj(interp, tkwin, specObj, &firstInt) != TCL_OK){
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		    "bad pad value \"%s\": must be positive screen distance",
		    Tcl_GetString(specObj)));
	    Tcl_SetErrorCode(interp, "TK", "VALUE", "PADDING", "DIST", (char *)NULL);
	    return TCL_ERROR;
	}
	secondInt = firstInt;
	goto done;
    }

    /*
     * Pad specifications are a list of one or two elements, each of which is
     * a pixel specification.
     */

    if (Tcl_ListObjGetElements(interp, specObj, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc != 1 && objc != 2) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		"wrong number of parts to pad specification", TCL_INDEX_NONE));
	Tcl_SetErrorCode(interp, "TK", "VALUE", "PADDING", "PARTS", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * Parse the first part.
     */

    if (Tk_GetPixelsFromObj(interp, tkwin, objv[0], &firstInt) != TCL_OK ||
	    (firstInt < 0)) {
	Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"bad pad value \"%s\": must be positive screen distance",
		Tcl_GetString(objv[0])));
	Tcl_SetErrorCode(interp, "TK", "VALUE", "PADDING", "DIST", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * Parse the second part if it exists, otherwise it is as if it was the
     * same as the first part.
     */

    if (objc == 1) {
	secondInt = firstInt;
    } else if (Tk_GetPixelsFromObj(interp, tkwin, objv[1],
	    &secondInt) != TCL_OK || (secondInt < 0)) {
	Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"bad 2nd pad value \"%s\": must be positive screen distance",
		Tcl_GetString(objv[1])));
	Tcl_SetErrorCode(interp, "TK", "VALUE", "PADDING", "DIST", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * Write the parsed bits back into the receiving variables.
     */

  done:
    if (halfPtr != 0) {
	*halfPtr = firstInt;
    }
    *allPtr = firstInt + secondInt;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TkRegisterObjTypes --
 *
 *	Registers Tk's Tcl_ObjType structures with the Tcl run-time.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	All instances of Tcl_ObjType structures used in Tk are registered with
 *	Tcl.
 *
 *----------------------------------------------------------------------
 */

void
TkRegisterObjTypes(void)
{
	Tk3dInit();
	TkBitmapInit();
	TkColorInit();
	TkConfigInit();
	TkCursorInit();
	TkFontInit();
	TkObjInit();
	TkStyleInit();
	TkTextInit();
	TkUtilInit();

    Tcl_RegisterObjType(tkBorderObjType.objTypePtr);
    Tcl_RegisterObjType(tkBitmapObjType.objTypePtr);
    Tcl_RegisterObjType(tkColorObjType.objTypePtr);
    Tcl_RegisterObjType(tkCursorObjType.objTypePtr);
    Tcl_RegisterObjType(tkFontObjType.objTypePtr);
    Tcl_RegisterObjType(mmObjType.objTypePtr);
    Tcl_RegisterObjType(pixelObjType.objTypePtr);
    Tcl_RegisterObjType(tkStateKeyObjType.objTypePtr);
    Tcl_RegisterObjType(windowObjType.objTypePtr);
    Tcl_RegisterObjType(tkTextIndexType.objTypePtr);
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
