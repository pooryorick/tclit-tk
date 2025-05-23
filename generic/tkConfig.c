/*
 * tkConfig.c --
 *
 *	This file contains functions that manage configuration options for
 *	widgets and other things.
 *
 * Copyright © 1997-1998 Sun Microsystems, Inc.
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

/*
 * Temporary flag for working on new config package.
 */

#if 0

/*
 * used only for removing the old config code
 */

#define __NO_OLD_CONFIG
#endif

#include "tkInt.h"
#include "tkFont.h"

#ifdef _WIN32
#include "tkWinInt.h"
#endif

/*
 * The following encoding is used in TK_OPTION_VAR:
 *
 * if sizeof(type) == sizeof(int)        =>    TK_OPTION_VAR(type) = 0
 * if sizeof(type) == 1                  =>    TK_OPTION_VAR(type) = 64
 * if sizeof(type) == 2                  =>    TK_OPTION_VAR(type) = 128
 * if sizeof(type) == sizeof(long long)  =>    TK_OPTION_VAR(type) = 192
 */
#define TYPE_MASK        (((((int)sizeof(int)-1))|3)<<6)

/*
 * The following definition keeps track of all of
 * the option tables that have been created for a thread.
 */

typedef struct {
    int initialized;		/* 0 means table below needs initializing. */
    Tcl_HashTable hashTable;
} ThreadSpecificData;
static Tcl_ThreadDataKey dataKey;


/*
 * The following two structures are used along with Tk_OptionSpec structures
 * to manage configuration options. Tk_OptionSpec is static templates that are
 * compiled into the code of a widget or other object manager. However, to
 * look up options efficiently we need to supplement the static information
 * with additional dynamic information, and this dynamic information may be
 * different for each application. Thus we create structures of the following
 * two types to hold all of the dynamic information; this is done by
 * Tk_CreateOptionTable.
 *
 * One of the following structures corresponds to each Tk_OptionSpec. These
 * structures exist as arrays inside TkOptionTable structures.
 */

typedef struct TkOption {
    const Tk_OptionSpec *specPtr;
				/* The original spec from the template passed
				 * to Tk_CreateOptionTable.*/
    Tk_Uid dbNameUID;		/* The Uid form of the option database
				 * name. */
    Tk_Uid dbClassUID;		/* The Uid form of the option database class
				 * name. */
    Tcl_Obj *defaultPtr;	/* Default value for this option. */
    union {
	Tcl_Obj *monoColorPtr;	/* For color and border options, this is an
				 * alternate default value to use on
				 * monochrome displays. */
	struct TkOption *synonymPtr;
				/* For synonym options, this points to the
				 * original entry. */
	const struct Tk_ObjCustomOption *custom;
				/* For TK_OPTION_CUSTOM. */
    } extra;
    int flags;			/* Miscellaneous flag values; see below for
				 * definitions. */
} Option;

/*
 * Flag bits defined for Option structures:
 *
 * OPTION_NEEDS_FREEING -	1 means that FreeResources must be invoked to
 *				free resources associated with the option when
 *				it is no longer needed.
 */

#define OPTION_NEEDS_FREEING		1

/*
 * One of the following exists for each Tk_OptionSpec array that has been
 * passed to Tk_CreateOptionTable.
 */

typedef struct OptionTable {
    size_t refCount;		/* Counts the number of uses of this table
				 * (the number of times Tk_CreateOptionTable
				 * has returned it). This can be greater than
				 * 1 if it is shared along several option
				 * table chains, or if the same table is used
				 * for multiple purposes. */
    Tcl_HashEntry *hashEntryPtr;/* Hash table entry that refers to this table;
				 * used to delete the entry. */
    struct OptionTable *nextPtr;/* If templatePtr was part of a chain of
				 * templates, this points to the table
				 * corresponding to the next template in the
				 * chain. */
    size_t numOptions;		/* The number of items in the options array
				 * below. */
    Option options[1];		/* Information about the individual options in
				 * the table. This must be the last field in
				 * the structure: the actual size of the array
				 * will be numOptions, not 1. */
} OptionTable;

/*
 * Forward declarations for functions defined later in this file:
 */

static int		DoObjConfig(Tcl_Interp *interp, void *recordPtr,
			    Option *optionPtr, Tcl_Obj *valuePtr,
			    Tk_Window tkwin, Tk_SavedOption *savePtr);
static void		FreeResources(Option *optionPtr, Tcl_Obj *objPtr,
			    void *internalPtr, Tk_Window tkwin);
static Tcl_Obj *	GetConfigList(void *recordPtr,
			    Option *optionPtr, Tk_Window tkwin);
static Tcl_Obj *	GetObjectForOption(void *recordPtr,
			    Option *optionPtr, Tk_Window tkwin);
static Option *		GetOption(const char *name, OptionTable *tablePtr);
static Option *		GetOptionFromObj(Tcl_Interp *interp,
			    Tcl_Obj *objPtr, OptionTable *tablePtr);
static void		FreeOptionInternalRep(Tcl_Obj *objPtr);
static void		DupOptionInternalRep(Tcl_Obj *, Tcl_Obj *);

/*
 * The structure below defines an object type that is used to cache the result
 * of looking up an option name. If an object has this type, then its
 * internalPtr1 field points to the OptionTable in which it was looked up, and
 * the internalPtr2 field points to the entry that matched.
 */

static const TkObjType optionObjType = {
	NULL, 0
};


void TkConfigInit() {
	TkObjType *tmpPtr = (TkObjType *)&optionObjType;
	Tcl_ObjType *otPtr = Tcl_NewObjType();
	Tcl_ObjTypeSetName(otPtr, (char *)"option");
	Tcl_ObjTypeSetVersion(otPtr, 1);
	Tcl_ObjTypeSetFreeInternalRepProc(otPtr, FreeOptionInternalRep);
	Tcl_ObjTypeSetDupInternalRepProc(otPtr, DupOptionInternalRep);
	tmpPtr->objTypePtr = otPtr;
	return;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_CreateOptionTable --
 *
 *	Given a template for configuration options, this function creates a
 *	table that may be used to look up options efficiently.
 *
 * Results:
 *	Returns a token to a structure that can be passed to functions such as
 *	Tk_InitOptions, Tk_SetOptions, and Tk_FreeConfigOptions.
 *
 * Side effects:
 *	Storage is allocated.
 *
 *--------------------------------------------------------------
 */

Tk_OptionTable
Tk_CreateOptionTable(
    Tcl_Interp *interp,		/* Interpreter associated with the application
				 * in which this table will be used. */
    const Tk_OptionSpec *templatePtr)
				/* Static information about the configuration
				 * options. */
{
    Tcl_HashEntry *hashEntryPtr;
    int newEntry;
    OptionTable *tablePtr;
    const Tk_OptionSpec *specPtr, *specPtr2;
    Option *optionPtr;
    size_t numOptions, i;
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
	    Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

    /*
     * We use an TSD in the thread to keep a hash table of
     * all the option tables we've created for this application. This is
     * used for allowing us to share the tables (e.g. in several chains).
     * The code below finds the hash table or creates a new one if it
     * doesn't already exist.
     */

    if (!tsdPtr->initialized) {
	Tcl_InitHashTable(&tsdPtr->hashTable, TCL_ONE_WORD_KEYS);
	tsdPtr->initialized = 1;
    }

    /*
     * See if a table has already been created for this template. If so, just
     * reuse the existing table.
     */

    hashEntryPtr = Tcl_CreateHashEntry(&tsdPtr->hashTable, (char *)templatePtr,
	    &newEntry);
    if (!newEntry) {
	tablePtr = (OptionTable *)Tcl_GetHashValue(hashEntryPtr);
	tablePtr->refCount++;
	return (Tk_OptionTable) tablePtr;
    }

    /*
     * Count the number of options in the template, then create the table
     * structure.
     */

    numOptions = 0;
    for (specPtr = templatePtr; specPtr->type != TK_OPTION_END; specPtr++) {
	numOptions++;
    }
    tablePtr = (OptionTable *)ckalloc(sizeof(OptionTable) + (numOptions * sizeof(Option)));
    tablePtr->refCount = 1;
    tablePtr->hashEntryPtr = hashEntryPtr;
    tablePtr->nextPtr = NULL;
    tablePtr->numOptions = numOptions;

    /*
     * Initialize all of the Option structures in the table.
     */

    for (specPtr = templatePtr, optionPtr = tablePtr->options;
	    specPtr->type != TK_OPTION_END; specPtr++, optionPtr++) {
	optionPtr->specPtr = specPtr;
	optionPtr->dbNameUID = NULL;
	optionPtr->dbClassUID = NULL;
	optionPtr->defaultPtr = NULL;
	optionPtr->extra.monoColorPtr = NULL;
	optionPtr->flags = 0;

	if (specPtr->type == TK_OPTION_SYNONYM) {
	    /*
	     * This is a synonym option; find the original option that it refers
	     * to and create a pointer from the synonym to the origin.
	     */

	    for (specPtr2 = templatePtr, i = 0; ; specPtr2++, i++) {
		if (specPtr2->type == TK_OPTION_END) {
		    Tcl_Panic("Tk_CreateOptionTable couldn't find synonym");
		}
		if (strcmp(specPtr2->optionName,
			(char *)specPtr->clientData) == 0) {
		    optionPtr->extra.synonymPtr = tablePtr->options + i;
		    break;
		}
	    }
	} else {
	    if (specPtr->dbName != NULL) {
		optionPtr->dbNameUID = Tk_GetUid(specPtr->dbName);
	    }
	    if (specPtr->dbClass != NULL) {
		optionPtr->dbClassUID = Tk_GetUid(specPtr->dbClass);
	    }
	    if (specPtr->defValue != NULL) {
		optionPtr->defaultPtr = Tcl_NewStringObj(specPtr->defValue, TCL_INDEX_NONE);
		Tcl_IncrRefCount(optionPtr->defaultPtr);
	    }
	    if (((specPtr->type == TK_OPTION_COLOR)
		    || (specPtr->type == TK_OPTION_BORDER))
		    && (specPtr->clientData != NULL)) {
		optionPtr->extra.monoColorPtr =
			Tcl_NewStringObj((const char *)specPtr->clientData, TCL_INDEX_NONE);
		Tcl_IncrRefCount(optionPtr->extra.monoColorPtr);
	    }

	    if (specPtr->type == TK_OPTION_CUSTOM) {
		/*
		 * Get the custom parsing, etc., functions.
		 */

		optionPtr->extra.custom = (const Tk_ObjCustomOption *)specPtr->clientData;
	    }
	}
	if (((specPtr->type == TK_OPTION_STRING)
		&& (specPtr->internalOffset != TCL_INDEX_NONE))
		|| (specPtr->type == TK_OPTION_COLOR)
		|| (specPtr->type == TK_OPTION_FONT)
		|| (specPtr->type == TK_OPTION_BITMAP)
		|| (specPtr->type == TK_OPTION_BORDER)
		|| (specPtr->type == TK_OPTION_CURSOR)
		|| (specPtr->type == TK_OPTION_CUSTOM)) {
	    optionPtr->flags |= OPTION_NEEDS_FREEING;
	}
    }
    tablePtr->hashEntryPtr = hashEntryPtr;
    Tcl_SetHashValue(hashEntryPtr, tablePtr);

    /*
     * Finally, check to see if this template chains to another template with
     * additional options. If so, call ourselves recursively to create the
     * next table(s).
     */

    if (specPtr->clientData != NULL) {
	tablePtr->nextPtr = (OptionTable *)
		Tk_CreateOptionTable(interp, (Tk_OptionSpec *)specPtr->clientData);
    }

    return (Tk_OptionTable) tablePtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_DeleteOptionTable --
 *
 *	Called to release resources used by an option table when the table is
 *	no longer needed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The option table and associated resources (such as additional option
 *	tables chained off it) are destroyed.
 *
 *----------------------------------------------------------------------
 */

void
Tk_DeleteOptionTable(
    Tk_OptionTable optionTable)	/* The option table to delete. */
{
    OptionTable *tablePtr = (OptionTable *) optionTable;
    Option *optionPtr;
    size_t count;

    if (tablePtr->refCount-- > 1) {
	return;
    }

    if (tablePtr->nextPtr != NULL) {
	Tk_DeleteOptionTable((Tk_OptionTable) tablePtr->nextPtr);
    }

    for (count = tablePtr->numOptions, optionPtr = tablePtr->options;
	    count > 0;  count--, optionPtr++) {
	if (optionPtr->defaultPtr != NULL) {
	    Tcl_DecrRefCount(optionPtr->defaultPtr);
	}
	if (((optionPtr->specPtr->type == TK_OPTION_COLOR)
		|| (optionPtr->specPtr->type == TK_OPTION_BORDER))
		&& (optionPtr->extra.monoColorPtr != NULL)) {
	    Tcl_DecrRefCount(optionPtr->extra.monoColorPtr);
	}
    }
    Tcl_DeleteHashEntry(tablePtr->hashEntryPtr);
    ckfree(tablePtr);
}

/*
 *--------------------------------------------------------------
 *
 * Tk_InitOptions --
 *
 *	This function is invoked when an object such as a widget is created.
 *	It supplies an initial value for each configuration option (the value
 *	may come from the option database, a system default, or the default in
 *	the option table).
 *
 * Results:
 *	The return value is TCL_OK if the function completed successfully, and
 *	TCL_ERROR if one of the initial values was bogus. If an error occurs
 *	and interp isn't NULL, then an error message will be left in its
 *	result.
 *
 * Side effects:
 *	Fields of recordPtr are filled in with initial values.
 *
 *--------------------------------------------------------------
 */

int
Tk_InitOptions(
    Tcl_Interp *interp,		/* Interpreter for error reporting. NULL means
				 * don't leave an error message. */
    void *recordPtr,		/* Pointer to the record to configure. Note:
				 * the caller should have properly initialized
				 * the record with NULL pointers for each
				 * option value. */
    Tk_OptionTable optionTable,	/* The token which matches the config specs
				 * for the widget in question. */
    Tk_Window tkwin)		/* Certain options types (such as
				 * TK_OPTION_COLOR) need fields out of the
				 * window they are used in to be able to
				 * calculate their values. Not needed unless
				 * one of these options is in the configSpecs
				 * record. */
{
    OptionTable *tablePtr = (OptionTable *) optionTable;
    Option *optionPtr;
    size_t count;
    Tk_Uid value;
    Tcl_Obj *valuePtr;
    enum {
	OPTION_DATABASE, SYSTEM_DEFAULT, TABLE_DEFAULT
    } source;

    /*
     * If this table chains to other tables, handle their initialization
     * first. That way, if both tables refer to the same field of the record,
     * the value in the first table will win.
     */

    if (tablePtr->nextPtr != NULL) {
	if (Tk_InitOptions(interp, recordPtr,
		(Tk_OptionTable) tablePtr->nextPtr, tkwin) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    /*
     * Iterate over all of the options in the table, initializing each in
     * turn.
     */

    for (optionPtr = tablePtr->options, count = tablePtr->numOptions;
	    count > 0; optionPtr++, count--) {
	/*
	 * If we specify TK_OPTION_DONT_SET_DEFAULT, then the user has
	 * processed and set a default for this already.
	 */

	if ((optionPtr->specPtr->type == TK_OPTION_SYNONYM) ||
		(optionPtr->specPtr->flags & TK_OPTION_DONT_SET_DEFAULT)) {
	    continue;
	}
	source = TABLE_DEFAULT;

	/*
	 * We look in three places for the initial value, using the first
	 * non-NULL value that we find. First, check the option database.
	 */

	valuePtr = NULL;
	if (optionPtr->dbNameUID != NULL) {
	    value = Tk_GetOption(tkwin, optionPtr->dbNameUID,
		    optionPtr->dbClassUID);
	    if (value != NULL) {
		valuePtr = Tcl_NewStringObj(value, TCL_INDEX_NONE);
		source = OPTION_DATABASE;
	    }
	}

	/*
	 * Second, check for a system-specific default value.
	 */

	if ((valuePtr == NULL)
		&& (optionPtr->dbNameUID != NULL)) {
	    valuePtr = Tk_GetSystemDefault(tkwin, optionPtr->dbNameUID,
		    optionPtr->dbClassUID);
	    if (valuePtr != NULL) {
		source = SYSTEM_DEFAULT;
	    }
	}

	/*
	 * Third and last, use the default value supplied by the option table.
	 * In the case of color objects, we pick one of two values depending
	 * on whether the screen is mono or color.
	 */

	if (valuePtr == NULL) {
	    if ((tkwin != NULL)
		    && ((optionPtr->specPtr->type == TK_OPTION_COLOR)
		    || (optionPtr->specPtr->type == TK_OPTION_BORDER))
		    && (Tk_Depth(tkwin) <= 1)
		    && (optionPtr->extra.monoColorPtr != NULL)) {
		valuePtr = optionPtr->extra.monoColorPtr;
	    } else {
		valuePtr = optionPtr->defaultPtr;
	    }
	}

	if (valuePtr == NULL) {
	    continue;
	}

	/*
	 * Bump the reference count on valuePtr, so that it is strongly
	 * referenced here, and will be properly free'd when finished,
	 * regardless of what DoObjConfig does.
	 */

	Tcl_IncrRefCount(valuePtr);

	if (DoObjConfig(interp, recordPtr, optionPtr, valuePtr, tkwin,
		NULL) != TCL_OK) {
	    if (interp != NULL) {
		char msg[200];

		switch (source) {
		case OPTION_DATABASE:
		    snprintf(msg, 200, "\n    (database entry for \"%.50s\")",
			    optionPtr->specPtr->optionName);
		    break;
		case SYSTEM_DEFAULT:
		    snprintf(msg, 200, "\n    (system default for \"%.50s\")",
			    optionPtr->specPtr->optionName);
		    break;
		case TABLE_DEFAULT:
		    snprintf(msg, 200, "\n    (default value for \"%.50s\")",
			    optionPtr->specPtr->optionName);
		}
		if (tkwin != NULL) {
		    snprintf(msg + strlen(msg) - 1, 200 - (strlen(msg) - 1), " in widget \"%.50s\")",
			    Tk_PathName(tkwin));
		}
		Tcl_AddErrorInfo(interp, msg);
	    }
	    Tcl_DecrRefCount(valuePtr);
	    return TCL_ERROR;
	}
	Tcl_DecrRefCount(valuePtr);
    }
    return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * DoObjConfig --
 *
 *	This function applies a new value for a configuration option to the
 *	record being configured.
 *
 * Results:
 *	The return value is TCL_OK if the function completed successfully. If
 *	an error occurred then TCL_ERROR is returned and an error message is
 *	left in interp's result, if interp isn't NULL. In addition, if
 *	oldValuePtrPtr isn't NULL then it *oldValuePtrPtr is filled in with a
 *	pointer to the option's old value.
 *
 * Side effects:
 *	RecordPtr gets modified to hold the new value in the form of a
 *	Tcl_Obj, an internal representation, or both. The old value is freed
 *	if oldValuePtrPtr is NULL.
 *
 *--------------------------------------------------------------
 */

static int
DoObjConfig(
    Tcl_Interp *interp,		/* Interpreter for error reporting. If NULL,
				 * then no message is left if an error
				 * occurs. */
    void *recordPtr,		/* The record to modify to hold the new option
				 * value. */
    Option *optionPtr,		/* Pointer to information about the option. */
    Tcl_Obj *valuePtr,		/* New value for option. */
    Tk_Window tkwin,		/* Window in which option will be used (needed
				 * to allocate resources for some options).
				 * May be NULL if the option doesn't require
				 * window-related resources. */
    Tk_SavedOption *savedOptionPtr)
				/* If NULL, the old value for the option will
				 * be freed. If non-NULL, the old value will
				 * be stored here, and it becomes the property
				 * of the caller (the caller must eventually
				 * free the old value). */
{
    Tcl_Obj **slotPtrPtr, *oldPtr;
    void *internalPtr;		/* Points to location in record where internal
				 * representation of value should be stored,
				 * or NULL. */
    void *oldInternalPtr;	/* Points to location in which to save old
				 * internal representation of value. */
    Tk_SavedOption internal;	/* Used to save the old internal
				 * representation of the value if
				 * savedOptionPtr is NULL. */
    const Tk_OptionSpec *specPtr;
    int nullOK;

    /*
     * Save the old object form for the value, if there is one.
     */

    specPtr = optionPtr->specPtr;
    if (specPtr->objOffset != TCL_INDEX_NONE) {
	slotPtrPtr = (Tcl_Obj **) ((char *)recordPtr + specPtr->objOffset);
	oldPtr = *slotPtrPtr;
    } else {
	slotPtrPtr = NULL;
	oldPtr = NULL;
    }

    /*
     * Apply the new value in a type-specific way. Also remember the old
     * object and internal forms, if they exist.
     */

    if (specPtr->internalOffset != TCL_INDEX_NONE) {
	internalPtr = (char *)recordPtr + specPtr->internalOffset;
    } else {
	internalPtr = NULL;
    }
    if (savedOptionPtr != NULL) {
	savedOptionPtr->optionPtr = optionPtr;
	savedOptionPtr->valuePtr = oldPtr;
	oldInternalPtr = (char *)&savedOptionPtr->internalForm;
    } else {
	oldInternalPtr = (char *)&internal.internalForm;
    }
    nullOK = (optionPtr->specPtr->flags & (TK_OPTION_NULL_OK|TCL_NULL_OK|1));
    switch (optionPtr->specPtr->type) {
    case TK_OPTION_BOOLEAN: {
	int newBool;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newBool = -1;
	} else if (Tcl_GetBooleanFromObj(nullOK ? NULL : interp, valuePtr, &newBool) != TCL_OK) {
	    if (nullOK && interp) {
		Tcl_AppendResult(interp, "expected boolean value or \"\" but got \"",
			Tcl_GetString(valuePtr), "\"", NULL);
	    }
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    *((char *)oldInternalPtr) = *((char *)internalPtr);
		    *((char *)internalPtr) = (char)newBool;
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    *((short *)oldInternalPtr) = *((short *)internalPtr);
		    *((short *)internalPtr) = (short)newBool;
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_BOOLEAN");
		}
	    } else {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newBool;
	    }
	}
	break;
    }
    case TK_OPTION_INT: {
	int newInt;

	if ((optionPtr->specPtr->flags & TYPE_MASK) == 0) {
	    if (nullOK && TkObjIsEmpty(valuePtr)) {
		valuePtr = NULL;
		newInt = INT_MIN;
	    } else if (Tcl_GetIntFromObj(nullOK ? NULL : interp, valuePtr, &newInt) != TCL_OK) {
	    invalidIntValue:
		if (nullOK && interp) {
		    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
			    "expected integer or \"\" but got \"%.50s\"", Tcl_GetString(valuePtr)));
		    Tcl_SetErrorCode(interp, "TCL", "VALUE", "NUMBER", (char *)NULL);
		}
		return TCL_ERROR;
	    }
	    if (internalPtr != NULL) {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newInt;
	    }
	} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TYPE_MASK) {
	    Tcl_WideInt newWideInt;
	    if (nullOK && TkObjIsEmpty(valuePtr)) {
		valuePtr = NULL;
		newWideInt = (sizeof(long) > sizeof(int)) ? LONG_MIN : LLONG_MIN;
	    } else if (Tcl_GetWideIntFromObj(nullOK ? NULL : interp, valuePtr, &newWideInt) != TCL_OK) {
		goto invalidIntValue;
	    }
		if (internalPtr != NULL) {
			if (sizeof(long) > sizeof(int)) {
			    *((long *)oldInternalPtr) = *((long *)internalPtr);
			    *((long *)internalPtr) = (long)newWideInt;
			} else {
			    *((long long *)oldInternalPtr) = *((long long *)internalPtr);
			    *((long long *)internalPtr) = (long long)newWideInt;
			}
		}
	} else {
	    Tcl_Panic("Invalid flags for %s", "TK_OPTION_INT");
	}
	break;
    }
    case TK_OPTION_INDEX: {
	Tcl_Size newIndex;

	if (TkGetIntForIndex(valuePtr, TCL_INDEX_NONE, 0, &newIndex) != TCL_OK) {
	    if (interp) {
		Tcl_AppendResult(interp, "bad index \"", Tcl_GetString(valuePtr),
			"\": must be integer?[+-]integer?, end?[+-]integer?, or \"\"", NULL);
	    }
	    return TCL_ERROR;
	}
	if (newIndex < INT_MIN) {
	    newIndex = INT_MIN;
	} else if (newIndex > INT_MAX) {
	    newIndex = INT_MAX;
	}
	if (internalPtr != NULL) {
	    *((int *)oldInternalPtr) = *((int *)internalPtr);
	    *((int *)internalPtr) = (int)newIndex;
	}
	break;
    }
    case TK_OPTION_DOUBLE: {
	double newDbl;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
#if defined(NAN)
	    newDbl = NAN;
#else
	    newDbl = 0.0;
#endif
	} else {
	    if (Tcl_GetDoubleFromObj(nullOK ? NULL : interp, valuePtr, &newDbl) != TCL_OK) {
		if (nullOK && interp) {
		    Tcl_Obj *msg = Tcl_NewStringObj("expected floating-point number or \"\" but got \"", TCL_INDEX_NONE);

		    Tcl_AppendLimitedToObj(msg, Tcl_GetString(valuePtr), TCL_INDEX_NONE, 50, "");
		    Tcl_AppendToObj(msg, "\"", TCL_INDEX_NONE);
		    Tcl_SetObjResult(interp, msg);
		    Tcl_SetErrorCode(interp, "TCL", "VALUE", "NUMBER", (char *)NULL);
		}
		return TCL_ERROR;
	    }
	}

	if (internalPtr != NULL) {
	    *((double *)oldInternalPtr) = *((double *)internalPtr);
	    *((double *)internalPtr) = newDbl;
	}
	break;
    }
    case TK_OPTION_STRING: {
	char *newStr;
	const char *value;
	Tcl_Size length;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	}
	if (internalPtr != NULL) {
	    if (valuePtr != NULL) {
		value = Tcl_GetStringFromObj(valuePtr, &length);
		newStr = (char *)ckalloc(length + 1);
		strcpy(newStr, value);
	    } else {
		newStr = NULL;
	    }
	    *((char **)oldInternalPtr) = *((char **)internalPtr);
	    *((char **)internalPtr) = newStr;
	}
	break;
    }
    case TK_OPTION_STRING_TABLE: {
	int newValue;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newValue = -1;
	} else {
	    if (Tcl_GetIndexFromObjStruct(interp, valuePtr,
		    optionPtr->specPtr->clientData, sizeof(char *),
		    optionPtr->specPtr->optionName+1, (nullOK ? TCL_NULL_OK : 0), &newValue) != TCL_OK) {
		return TCL_ERROR;
	    }
	    if (slotPtrPtr != NULL && valuePtr != NULL) {
		valuePtr = Tcl_DuplicateObj(valuePtr);
		Tcl_InvalidateStringRep(valuePtr);
	    }
	}
	if (internalPtr != NULL) {
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    *((char *)oldInternalPtr) = *((char *)internalPtr);
		    *((char *)internalPtr) = (char)newValue;
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    *((short *)oldInternalPtr) = *((short *)internalPtr);
		    *((short *)internalPtr) = (short)newValue;
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_STRING_TABLE");
		}
	    } else {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newValue;
	    }
	}
	break;
    }
    case TK_OPTION_COLOR: {
	XColor *newPtr;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newPtr = NULL;
	} else {
	    newPtr = Tk_AllocColorFromObj(interp, tkwin, valuePtr);
	    if (newPtr == NULL) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((XColor **)oldInternalPtr) = *((XColor **)internalPtr);
	    *((XColor **)internalPtr) = newPtr;
	}
	break;
    }
    case TK_OPTION_FONT: {
	Tk_Font newFont;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newFont = NULL;
	} else {
	    newFont = Tk_AllocFontFromObj(interp, tkwin, valuePtr);
	    if (newFont == NULL) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((Tk_Font *)oldInternalPtr) = *((Tk_Font *)internalPtr);
	    *((Tk_Font *)internalPtr) = newFont;
	}
	break;
    }
    case TK_OPTION_STYLE: {
	Tk_Style newStyle;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newStyle = NULL;
	} else {
	    newStyle = Tk_AllocStyleFromObj(interp, valuePtr);
	    if (newStyle == NULL) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((Tk_Style *)oldInternalPtr) = *((Tk_Style *)internalPtr);
	    *((Tk_Style *)internalPtr) = newStyle;
	}
	break;
    }
    case TK_OPTION_BITMAP: {
	Pixmap newBitmap;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newBitmap = None;
	} else {
	    newBitmap = Tk_AllocBitmapFromObj(interp, tkwin, valuePtr);
	    if (newBitmap == None) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((Pixmap *)oldInternalPtr) = *((Pixmap *)internalPtr);
	    *((Pixmap *)internalPtr) = newBitmap;
	}
	break;
    }
    case TK_OPTION_BORDER: {
	Tk_3DBorder newBorder;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newBorder = NULL;
	} else {
	    newBorder = Tk_Alloc3DBorderFromObj(interp, tkwin, valuePtr);
	    if (newBorder == NULL) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((Tk_3DBorder *)oldInternalPtr) = *((Tk_3DBorder *)internalPtr);
	    *((Tk_3DBorder *)internalPtr) = newBorder;
	}
	break;
    }
    case TK_OPTION_RELIEF: {
	int newRelief;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newRelief = TK_RELIEF_NULL;
	} else if (Tcl_GetIndexFromObj(interp, valuePtr, tkReliefStrings,
		"relief", (nullOK ? TCL_NULL_OK : 0), &newRelief) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    *((char *)oldInternalPtr) = *((char *)internalPtr);
		    *((char *)internalPtr) = (char)newRelief;
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    *((short *)oldInternalPtr) = *((short *)internalPtr);
		    *((short *)internalPtr) = (short)newRelief;
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_RELIEF");
		}
	    } else {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newRelief;
	    }
	}
	if (slotPtrPtr != NULL && valuePtr != NULL) {
	    valuePtr = Tcl_DuplicateObj(valuePtr);
	    Tcl_InvalidateStringRep(valuePtr);
	}
	break;
    }
    case TK_OPTION_CURSOR: {
	Tk_Cursor newCursor;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    newCursor = NULL;
	    valuePtr = NULL;
	} else {
	    newCursor = Tk_AllocCursorFromObj(interp, tkwin, valuePtr);
	    if (newCursor == NULL) {
		return TCL_ERROR;
	    }
	}
	if (internalPtr != NULL) {
	    *((Tk_Cursor *) oldInternalPtr) = *((Tk_Cursor *) internalPtr);
	    *((Tk_Cursor *) internalPtr) = newCursor;
	}
	Tk_DefineCursor(tkwin, newCursor);
	break;
    }
    case TK_OPTION_JUSTIFY: {
	int newJustify;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newJustify = -1;
	} else if (Tcl_GetIndexFromObj(interp, valuePtr, tkJustifyStrings,
		"justification", (nullOK ? TCL_NULL_OK : 0), &newJustify) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    *((char *)oldInternalPtr) = *((char *)internalPtr);
		    *((char *)internalPtr) = (char)newJustify;
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    *((short *)oldInternalPtr) = *((short *)internalPtr);
		    *((short *)internalPtr) = (short)newJustify;
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_JUSTIFY");
		}
	    } else {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newJustify;
	    }
	}
	if (slotPtrPtr != NULL && valuePtr != NULL) {
	    valuePtr = Tcl_DuplicateObj(valuePtr);
	    Tcl_InvalidateStringRep(valuePtr);
	}
	break;
    }
    case TK_OPTION_ANCHOR: {
	int newAnchor;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newAnchor = -1;
	} else if (Tcl_GetIndexFromObj(interp, valuePtr, tkAnchorStrings,
		"anchor", (nullOK ? TCL_NULL_OK : 0), &newAnchor) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    *((char *)oldInternalPtr) = *((char *)internalPtr);
		    *((char *)internalPtr) = (char)newAnchor;
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    *((short *)oldInternalPtr) = *((short *)internalPtr);
		    *((short *)internalPtr) = (short)newAnchor;
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_JUSTIFY");
		}
	    } else {
		*((int *)oldInternalPtr) = *((int *)internalPtr);
		*((int *)internalPtr) = newAnchor;
	    }
	}
	if (slotPtrPtr != NULL && valuePtr != NULL) {
	    valuePtr = Tcl_DuplicateObj(valuePtr);
	    Tcl_InvalidateStringRep(valuePtr);
	}
	break;
    }
    case TK_OPTION_PIXELS: {
	int newPixels;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newPixels = INT_MIN;
	} else if (Tk_GetPixelsFromObj(nullOK ? NULL : interp, tkwin, valuePtr,
		&newPixels) != TCL_OK) {
	    if (nullOK) {
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		    "expected screen distance or \"\" but got \"%.50s\"", Tcl_GetString(valuePtr)));
		Tcl_SetErrorCode(interp, "TK", "VALUE", "PIXELS", (char *)NULL);
	    }
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    *((int *)oldInternalPtr) = *((int *)internalPtr);
	    *((int *)internalPtr) = newPixels;
	}
	break;
    }
    case TK_OPTION_WINDOW: {
	Tk_Window newWin;

	if (nullOK && TkObjIsEmpty(valuePtr)) {
	    valuePtr = NULL;
	    newWin = NULL;
	} else if (TkGetWindowFromObj(interp, tkwin, valuePtr,
		&newWin) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (internalPtr != NULL) {
	    *((Tk_Window *)oldInternalPtr) = *((Tk_Window *)internalPtr);
	    *((Tk_Window *)internalPtr) = newWin;
	}
	break;
    }
    case TK_OPTION_CUSTOM: {
	const Tk_ObjCustomOption *custom = optionPtr->extra.custom;

	if (custom->setProc(custom->clientData, interp, tkwin,
		&valuePtr, (char *)recordPtr, optionPtr->specPtr->internalOffset,
		(char *)oldInternalPtr, optionPtr->specPtr->flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    }

    default:
	Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"bad config table: unknown type %d",
		optionPtr->specPtr->type));
	Tcl_SetErrorCode(interp, "TK", "BAD_CONFIG", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * Release resources associated with the old value, if we're not returning
     * it to the caller, then install the new object value into the record.
     */

    if (savedOptionPtr == NULL) {
	if (optionPtr->flags & OPTION_NEEDS_FREEING) {
	    FreeResources(optionPtr, oldPtr, oldInternalPtr, tkwin);
	}
	if (oldPtr != NULL) {
	    Tcl_DecrRefCount(oldPtr);
	}
    }
    if (slotPtrPtr != NULL) {
	*slotPtrPtr = valuePtr;
	if (valuePtr != NULL) {
	    Tcl_IncrRefCount(valuePtr);
	}
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TkObjIsEmpty --
 *
 *	This function tests whether the string value of an object is empty.
 *
 * Results:
 *	The return value is 1 if the string value of objPtr has length zero,
 *	and 0 otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#if defined(USE_TCL_STUBS)
# undef Tcl_IsEmpty
# define Tcl_IsEmpty \
    ((int (*)(Tcl_Obj *))(void *)((&(tclStubsPtr->tcl_PkgProvideEx))[690]))
#endif

int
TkObjIsEmpty(
    Tcl_Obj *objPtr)		/* Object to test. May be NULL. */
{
    if (objPtr == NULL) {
	return 1;
    }
    if (objPtr->bytes == NULL) {
#if defined(USE_TCL_STUBS)
	if (Tcl_IsEmpty) {
	    return Tcl_IsEmpty(objPtr);
	}
#endif
	Tcl_GetString(objPtr);
    }
    return (objPtr->length == 0);
}

/*
 *----------------------------------------------------------------------
 *
 * GetOption --
 *
 *	This function searches through a chained option table to find the
 *	entry for a particular option name.
 *
 * Results:
 *	The return value is a pointer to the matching entry, or NULL if no
 *	matching entry could be found. Note: if the matching entry is a
 *	synonym then this function returns a pointer to the synonym entry,
 *	*not* the "real" entry that the synonym refers to.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static Option *
GetOption(
    const char *name,		/* String balue to be looked up in the option
				 * table. */
    OptionTable *tablePtr)	/* Table in which to look up name. */
{
    Option *bestPtr, *optionPtr;
    OptionTable *tablePtr2;
    const char *p1, *p2;
    size_t count;

    /*
     * Search through all of the option tables in the chain to find the best
     * match. Some tricky aspects:
     *
     * 1. We have to accept unique abbreviations.
     * 2. The same name could appear in different tables in the chain. If this
     *    happens, we use the entry from the first table. We have to be
     *    careful to distinguish this case from an ambiguous abbreviation.
     */

    bestPtr = NULL;
    for (tablePtr2 = tablePtr; tablePtr2 != NULL;
	    tablePtr2 = tablePtr2->nextPtr) {
	for (optionPtr = tablePtr2->options, count = tablePtr2->numOptions;
		count > 0; optionPtr++, count--) {
	    for (p1 = name, p2 = optionPtr->specPtr->optionName;
		    *p1 == *p2; p1++, p2++) {
		if (*p1 == 0) {
		    /*
		     * This is an exact match. We're done.
		     */

		    return optionPtr;
		}
	    }
	    if (*p1 == 0) {
		/*
		 * The name is an abbreviation for this option. Keep to make
		 * sure that the abbreviation only matches one option name.
		 * If we've already found a match in the past, then it is an
		 * error unless the full names for the two options are
		 * identical; in this case, the first option overrides the
		 * second.
		 */

		if (bestPtr == NULL) {
		    bestPtr = optionPtr;
		} else if (strcmp(bestPtr->specPtr->optionName,
			optionPtr->specPtr->optionName) != 0) {
		    return NULL;
		}
	    }
	}
    }

    /*
     * Return whatever we have found, which could be NULL if nothing
     * matched. The multiple-matching case is handled above.
     */

    return bestPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * GetOptionFromObj --
 *
 *	This function searches through a chained option table to find the
 *	entry for a particular option name.
 *
 * Results:
 *	The return value is a pointer to the matching entry, or NULL if no
 *	matching entry could be found. If NULL is returned and interp is not
 *	NULL than an error message is left in its result. Note: if the
 *	matching entry is a synonym then this function returns a pointer to
 *	the synonym entry, *not* the "real" entry that the synonym refers to.
 *
 * Side effects:
 *	Information about the matching entry is cached in the object
 *	containing the name, so that future lookups can proceed more quickly.
 *
 *----------------------------------------------------------------------
 */

static Option *
GetOptionFromObj(
    Tcl_Interp *interp,		/* Used only for error reporting; if NULL no
				 * message is left after an error. */
    Tcl_Obj *objPtr,		/* Object whose string value is to be looked
				 * up in the option table. */
    OptionTable *tablePtr)	/* Table in which to look up objPtr. */
{
    Option *bestPtr;
    const char *name;

    /*
     * First, check to see if the object already has the answer cached.
     */

    if (objPtr->typePtr == optionObjType.objTypePtr) {
	if (objPtr->internalRep.twoPtrValue.ptr1 == (void *) tablePtr) {
	    return (Option *) objPtr->internalRep.twoPtrValue.ptr2;
	}
    }

    /*
     * The answer isn't cached.
     */

    name = Tcl_GetString(objPtr);
    bestPtr = GetOption(name, tablePtr);
    if (bestPtr == NULL) {
	goto error;
    }

    if ((objPtr->typePtr != NULL)
	    && (objPtr->typePtr->freeIntRepProc != NULL)) {
	objPtr->typePtr->freeIntRepProc(objPtr);
    }
    objPtr->internalRep.twoPtrValue.ptr1 = (void *) tablePtr;
    objPtr->internalRep.twoPtrValue.ptr2 = (void *) bestPtr;
    objPtr->typePtr = optionObjType.objTypePtr;
    tablePtr->refCount++;
    return bestPtr;

  error:
    if (interp != NULL) {
	Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"unknown option \"%s\"", name));
	Tcl_SetErrorCode(interp, "TK", "LOOKUP", "OPTION", name, (char *)NULL);
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * TkGetOptionSpec --
 *
 *	This function searches through a chained option table to find the
 *	option spec for a particular option name.
 *
 * Results:
 *	The return value is a pointer to the option spec of the matching
 *	entry, or NULL if no matching entry could be found. Note: if the
 *	matching entry is a synonym then this function returns a pointer to
 *	the option spec of the synonym entry, *not* the "real" entry that the
 *	synonym refers to. Note: this call is primarily used by the style
 *	management code (tkStyle.c) to look up an element's option spec into a
 *	widget's option table.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

const Tk_OptionSpec *
TkGetOptionSpec(
    const char *name,		/* String value to be looked up. */
    Tk_OptionTable optionTable)	/* Table in which to look up name. */
{
    Option *optionPtr;

    optionPtr = GetOption(name, (OptionTable *) optionTable);
    if (optionPtr == NULL) {
	return NULL;
    }
    return optionPtr->specPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * FreeOptionInternalRep --
 *
 *	Part of the option Tcl object type implementation. Frees the storage
 *	associated with a option object's internal representation unless it
 *	is still in use.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The option object's internal rep is marked invalid and its memory
 *	gets freed unless it is still in use somewhere. In that case the
 *	cleanup is delayed until the last reference goes away.
 *
 *----------------------------------------------------------------------
 */

static void
FreeOptionInternalRep(
    Tcl_Obj *objPtr)	/* Object whose internal rep to free. */
{
    Tk_OptionTable tablePtr = (Tk_OptionTable) objPtr->internalRep.twoPtrValue.ptr1;

    Tk_DeleteOptionTable(tablePtr);
    objPtr->typePtr = NULL;
    objPtr->internalRep.twoPtrValue.ptr1 = NULL;
    objPtr->internalRep.twoPtrValue.ptr2 = NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * DupOptionInternalRep --
 *
 *	When a cached option object is duplicated, this is called to update the
 *	internal reps.
 *
 *---------------------------------------------------------------------------
 */

static void
DupOptionInternalRep(
    Tcl_Obj *srcObjPtr,		/* The object we are copying from. */
    Tcl_Obj *dupObjPtr)		/* The object we are copying to. */
{
    OptionTable *tablePtr = (OptionTable *) srcObjPtr->internalRep.twoPtrValue.ptr1;
    tablePtr->refCount++;
    dupObjPtr->typePtr = srcObjPtr->typePtr;
    dupObjPtr->internalRep = srcObjPtr->internalRep;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_SetOptions --
 *
 *	Process one or more name-value pairs for configuration options and
 *	fill in fields of a record with new values.
 *
 * Results:
 *	If all goes well then TCL_OK is returned and the old values of any
 *	modified objects are saved in *savePtr, if it isn't NULL (the caller
 *	must eventually call Tk_RestoreSavedOptions or Tk_FreeSavedOptions to
 *	free the contents of *savePtr). In addition, if maskPtr isn't NULL
 *	then *maskPtr is filled in with the OR of the typeMask bits from all
 *	modified options. If an error occurs then TCL_ERROR is returned and a
 *	message is left in interp's result unless interp is NULL; nothing is
 *	saved in *savePtr or *maskPtr in this case.
 *
 * Side effects:
 *	The fields of recordPtr get filled in with object pointers from
 *	objc/objv. Old information in widgRec's fields gets recycled.
 *	Information may be left at *savePtr.
 *
 *--------------------------------------------------------------
 */

int
Tk_SetOptions(
    Tcl_Interp *interp,		/* Interpreter for error reporting. If NULL,
				 * then no error message is returned.*/
    void *recordPtr,		/* The record to configure. */
    Tk_OptionTable optionTable,	/* Describes valid options. */
    Tcl_Size objc,			/* The number of elements in objv. */
    Tcl_Obj *const objv[],	/* Contains one or more name-value pairs. */
    Tk_Window tkwin,		/* Window associated with the thing being
				 * configured; needed for some options (such
				 * as colors). */
    Tk_SavedOptions *savePtr,	/* If non-NULL, the old values of modified
				 * options are saved here so that they can be
				 * restored after an error. */
    int *maskPtr)		/* It non-NULL, this word is modified on a
				 * successful return to hold the bit-wise OR
				 * of the typeMask fields of all options that
				 * were modified by this call. Used by the
				 * caller to figure out which options actually
				 * changed. */
{
    OptionTable *tablePtr = (OptionTable *) optionTable;
    Option *optionPtr;
    Tk_SavedOptions *lastSavePtr, *newSavePtr;
    int mask;

    if (savePtr != NULL) {
	savePtr->recordPtr = recordPtr;
	savePtr->tkwin = tkwin;
	savePtr->numItems = 0;
	savePtr->nextPtr = NULL;
    }
    lastSavePtr = savePtr;

    /*
     * Scan through all of the arguments, processing those that match entries
     * in the option table.
     */

    mask = 0;
    for ( ; objc > 0; objc -= 2, objv += 2) {
	optionPtr = GetOptionFromObj(interp, objv[0], tablePtr);
	if (optionPtr == NULL) {
	    goto error;
	}
	if (optionPtr->specPtr->type == TK_OPTION_SYNONYM) {
	    optionPtr = optionPtr->extra.synonymPtr;
	}

	if (objc < 2) {
	    if (interp != NULL) {
		Tcl_SetObjResult(interp, Tcl_ObjPrintf(
			"value for \"%s\" missing",
			Tcl_GetString(*objv)));
		Tcl_SetErrorCode(interp, "TK", "VALUE_MISSING", (char *)NULL);
		goto error;
	    }
	}
	if ((savePtr != NULL)
		&& (lastSavePtr->numItems >= TK_NUM_SAVED_OPTIONS)) {
	    /*
	     * We've run out of space for saving old option values. Allocate
	     * more space.
	     */

	    newSavePtr = (Tk_SavedOptions *)ckalloc(sizeof(Tk_SavedOptions));
	    newSavePtr->recordPtr = recordPtr;
	    newSavePtr->tkwin = tkwin;
	    newSavePtr->numItems = 0;
	    newSavePtr->nextPtr = NULL;
	    lastSavePtr->nextPtr = newSavePtr;
	    lastSavePtr = newSavePtr;
	}
	if (DoObjConfig(interp, recordPtr, optionPtr, objv[1], tkwin,
		(savePtr != NULL) ? &lastSavePtr->items[lastSavePtr->numItems]
		: NULL) != TCL_OK) {
	    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
		    "\n    (processing \"%.40s\" option)",
		    Tcl_GetString(*objv)));
	    goto error;
	}
	if (savePtr != NULL) {
	    lastSavePtr->numItems++;
	}
	mask |= optionPtr->specPtr->typeMask;
    }
    if (maskPtr != NULL) {
	*maskPtr = mask;
    }
    return TCL_OK;

  error:
    if (savePtr != NULL) {
	Tk_RestoreSavedOptions(savePtr);
    }
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_RestoreSavedOptions --
 *
 *	This function undoes the effect of a previous call to Tk_SetOptions by
 *	restoring all of the options to their value before the call to
 *	Tk_SetOptions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The configutation record is restored and all the information stored in
 *	savePtr is freed.
 *
 *----------------------------------------------------------------------
 */

void
Tk_RestoreSavedOptions(
    Tk_SavedOptions *savePtr)	/* Holds saved option information; must have
				 * been passed to Tk_SetOptions. */
{
    Tcl_Size i;
    Option *optionPtr;
    Tcl_Obj *newPtr;		/* New object value of option, which we
				 * replace with old value and free. Taken from
				 * record. */
    void *internalPtr;		/* Points to internal value of option in
				 * record. */
    const Tk_OptionSpec *specPtr;

    /*
     * Be sure to restore the options in the opposite order they were set.
     * This is important because it's possible that the same option name was
     * used twice in a single call to Tk_SetOptions.
     */

    if (savePtr->nextPtr != NULL) {
	Tk_RestoreSavedOptions(savePtr->nextPtr);
	ckfree(savePtr->nextPtr);
	savePtr->nextPtr = NULL;
    }
    for (i = savePtr->numItems - 1; i >= 0; i--) {
	optionPtr = savePtr->items[i].optionPtr;
	specPtr = optionPtr->specPtr;

	/*
	 * First free the new value of the option, which is currently in the
	 * record.
	 */

	if (specPtr->objOffset >= 0) {
	    newPtr = *((Tcl_Obj **) ((char *)savePtr->recordPtr + specPtr->objOffset));
	} else {
	    newPtr = NULL;
	}
	if (specPtr->internalOffset >= 0) {
	    internalPtr = (char *)savePtr->recordPtr + specPtr->internalOffset;
	} else {
	    internalPtr = NULL;
	}
	if (optionPtr->flags & OPTION_NEEDS_FREEING) {
	    FreeResources(optionPtr, newPtr, internalPtr, savePtr->tkwin);
	}
	if (newPtr != NULL) {
	    Tcl_DecrRefCount(newPtr);
	}

	/*
	 * Now restore the old value of the option.
	 */

	if (specPtr->objOffset != TCL_INDEX_NONE) {
	    *((Tcl_Obj **) ((char *)savePtr->recordPtr + specPtr->objOffset))
		    = savePtr->items[i].valuePtr;
	}
	if (specPtr->internalOffset != TCL_INDEX_NONE) {
	    char *ptr = (char *)&savePtr->items[i].internalForm;

	    CLANG_ASSERT(internalPtr);
	    switch (specPtr->type) {
	    case TK_OPTION_BOOLEAN:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
			*((char *)internalPtr) = *((char *)ptr);
		    } else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
			*((short *)internalPtr) = *((short *)ptr);
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_BOOLEAN");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_INT:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TYPE_MASK) {
			if (sizeof(long) > sizeof(int)) {
			    *((long *)internalPtr) = *((long *)ptr);
			} else {
			    *((long long *)internalPtr) = *((long long *)ptr);
			}
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_INT");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_INDEX:
		*((int *)internalPtr) = *((int *)ptr);
		break;
	    case TK_OPTION_DOUBLE:
		*((double *)internalPtr) = *((double *)ptr);
		break;
	    case TK_OPTION_STRING:
		*((char **)internalPtr) = *((char **)ptr);
		break;
	    case TK_OPTION_STRING_TABLE:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
			*((char *)internalPtr) = *((char *)ptr);
		    } else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
			*((short *)internalPtr) = *((short *)ptr);
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_STRING_TABLE");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_COLOR:
		*((XColor **)internalPtr) = *((XColor **)ptr);
		break;
	    case TK_OPTION_FONT:
		*((Tk_Font *)internalPtr) = *((Tk_Font *)ptr);
		break;
	    case TK_OPTION_STYLE:
		*((Tk_Style *)internalPtr) = *((Tk_Style *)ptr);
		break;
	    case TK_OPTION_BITMAP:
		*((Pixmap *)internalPtr) = *((Pixmap *)ptr);
		break;
	    case TK_OPTION_BORDER:
		*((Tk_3DBorder *)internalPtr) = *((Tk_3DBorder *)ptr);
		break;
	    case TK_OPTION_RELIEF:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
			*((char *)internalPtr) = *((char *)ptr);
		    } else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
			*((short *)internalPtr) = *((short *)ptr);
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_RELIEF");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_CURSOR:
		*((Tk_Cursor *) internalPtr) = *((Tk_Cursor *) ptr);
		Tk_DefineCursor(savePtr->tkwin, *((Tk_Cursor *) internalPtr));
		break;
	    case TK_OPTION_JUSTIFY:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
			*((char *)internalPtr) = *((char *)ptr);
		    } else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
			*((short *)internalPtr) = *((short *)ptr);
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_JUSTIFY");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_ANCHOR:
		if (optionPtr->specPtr->flags & TYPE_MASK) {
		    if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
			*((char *)internalPtr) = *((char *)ptr);
		    } else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
			*((short *)internalPtr) = *((short *)ptr);
		    } else {
			Tcl_Panic("Invalid flags for %s", "TK_OPTION_ANCHOR");
		    }
		} else {
		    *((int *)internalPtr) = *((int *)ptr);
		}
		break;
	    case TK_OPTION_PIXELS:
		*((int *)internalPtr) = *((int *)ptr);
		break;
	    case TK_OPTION_WINDOW:
		*((Tk_Window *)internalPtr) = *((Tk_Window *)ptr);
		break;
	    case TK_OPTION_CUSTOM: {
		const Tk_ObjCustomOption *custom = optionPtr->extra.custom;

		if (custom->restoreProc != NULL) {
		    custom->restoreProc(custom->clientData, savePtr->tkwin,
			    (char *)internalPtr, ptr);
		}
		break;
	    }
	    default:
		Tcl_Panic("bad option type in Tk_RestoreSavedOptions");
	    }
	}
    }
    savePtr->numItems = 0;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_FreeSavedOptions --
 *
 *	Free all of the saved configuration option values from a previous call
 *	to Tk_SetOptions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Storage and system resources are freed.
 *
 *--------------------------------------------------------------
 */

void
Tk_FreeSavedOptions(
    Tk_SavedOptions *savePtr)	/* Contains options saved in a previous call
				 * to Tk_SetOptions. */
{
    size_t count;
    Tk_SavedOption *savedOptionPtr;

    if (savePtr->nextPtr != NULL) {
	Tk_FreeSavedOptions(savePtr->nextPtr);
	ckfree(savePtr->nextPtr);
    }
    for (count = savePtr->numItems; count > 0; count--) {
	savedOptionPtr = &savePtr->items[count-1];
	if (savedOptionPtr->optionPtr->flags & OPTION_NEEDS_FREEING) {
	    FreeResources(savedOptionPtr->optionPtr, savedOptionPtr->valuePtr,
		    (char *)&savedOptionPtr->internalForm, savePtr->tkwin);
	}
	if (savedOptionPtr->valuePtr != NULL) {
	    Tcl_DecrRefCount(savedOptionPtr->valuePtr);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_FreeConfigOptions --
 *
 *	Free all resources associated with configuration options.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	All of the Tcl_Obj's in recordPtr that are controlled by configuration
 *	options in optionTable are freed.
 *
 *----------------------------------------------------------------------
 */

void
Tk_FreeConfigOptions(
    void *recordPtr,		/* Record whose fields contain current values
				 * for options. */
    Tk_OptionTable optionTable,	/* Describes legal options. */
    Tk_Window tkwin)		/* Window associated with recordPtr; needed
				 * for freeing some options. */
{
    OptionTable *tablePtr;
    Option *optionPtr;
    size_t count;
    Tcl_Obj **oldPtrPtr, *oldPtr;
    void *oldInternalPtr;
    const Tk_OptionSpec *specPtr;

    for (tablePtr = (OptionTable *) optionTable; tablePtr != NULL;
	    tablePtr = tablePtr->nextPtr) {
	for (optionPtr = tablePtr->options, count = tablePtr->numOptions;
		count > 0; optionPtr++, count--) {
	    specPtr = optionPtr->specPtr;
	    if (specPtr->type == TK_OPTION_SYNONYM) {
		continue;
	    }
	    if (specPtr->objOffset != TCL_INDEX_NONE) {
		oldPtrPtr = (Tcl_Obj **) ((char *)recordPtr + specPtr->objOffset);
		oldPtr = *oldPtrPtr;
		*oldPtrPtr = NULL;
	    } else {
		oldPtr = NULL;
	    }
	    if (specPtr->internalOffset != TCL_INDEX_NONE) {
		oldInternalPtr = (char *)recordPtr + specPtr->internalOffset;
	    } else {
		oldInternalPtr = NULL;
	    }
	    if (optionPtr->flags & OPTION_NEEDS_FREEING) {
		FreeResources(optionPtr, oldPtr, oldInternalPtr, tkwin);
	    }
	    if (oldPtr != NULL) {
		Tcl_DecrRefCount(oldPtr);
	    }
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * FreeResources --
 *
 *	Free system resources associated with a configuration option, such as
 *	colors or fonts.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Any system resources associated with objPtr are released. However,
 *	objPtr itself is not freed.
 *
 *----------------------------------------------------------------------
 */

static void
FreeResources(
    Option *optionPtr,		/* Description of the configuration option. */
    Tcl_Obj *objPtr,		/* The current value of the option, specified
				 * as an object. */
    void *internalPtr,		/* A pointer to an internal representation for
				 * the option's value, such as an int or
				 * (XColor *). Only valid if
				 * optionPtr->specPtr->internalOffset != -1. */
    Tk_Window tkwin)		/* The window in which this option is used. */
{
    int internalFormExists;

    /*
     * If there exists an internal form for the value, use it to free
     * resources (also zero out the internal form). If there is no internal
     * form, then use the object form.
     */

    internalFormExists = optionPtr->specPtr->internalOffset != TCL_INDEX_NONE;
    switch (optionPtr->specPtr->type) {
    case TK_OPTION_STRING:
	if (internalFormExists) {
	    if (*((char **)internalPtr) != NULL) {
		ckfree(*((char **)internalPtr));
		*((char **)internalPtr) = NULL;
	    }
	}
	break;
    case TK_OPTION_COLOR:
	if (internalFormExists) {
	    if (*((XColor **)internalPtr) != NULL) {
		Tk_FreeColor(*((XColor **)internalPtr));
		*((XColor **)internalPtr) = NULL;
	    }
	} else if (objPtr != NULL) {
	    Tk_FreeColorFromObj(tkwin, objPtr);
	}
	break;
    case TK_OPTION_FONT:
	if (internalFormExists) {
	    Tk_FreeFont(*((Tk_Font *)internalPtr));
	    *((Tk_Font *)internalPtr) = NULL;
	} else if (objPtr != NULL) {
	    Tk_FreeFontFromObj(tkwin, objPtr);
	}
	break;
    case TK_OPTION_STYLE:
	if (internalFormExists) {
	    Tk_FreeStyle(*((Tk_Style *)internalPtr));
	    *((Tk_Style *)internalPtr) = NULL;
	}
	break;
    case TK_OPTION_BITMAP:
	if (internalFormExists) {
	    if (*((Pixmap *)internalPtr) != None) {
		Tk_FreeBitmap(Tk_Display(tkwin), *((Pixmap *)internalPtr));
		*((Pixmap *)internalPtr) = None;
	    }
	} else if (objPtr != NULL) {
	    Tk_FreeBitmapFromObj(tkwin, objPtr);
	}
	break;
    case TK_OPTION_BORDER:
	if (internalFormExists) {
	    if (*((Tk_3DBorder *)internalPtr) != NULL) {
		Tk_Free3DBorder(*((Tk_3DBorder *)internalPtr));
		*((Tk_3DBorder *)internalPtr) = NULL;
	    }
	} else if (objPtr != NULL) {
	    Tk_Free3DBorderFromObj(tkwin, objPtr);
	}
	break;
    case TK_OPTION_CURSOR:
	if (internalFormExists) {
	    if (*((Tk_Cursor *) internalPtr) != NULL) {
		Tk_FreeCursor(Tk_Display(tkwin), *((Tk_Cursor *) internalPtr));
		*((Tk_Cursor *) internalPtr) = NULL;
	    }
	} else if (objPtr != NULL) {
	    Tk_FreeCursorFromObj(tkwin, objPtr);
	}
	break;
    case TK_OPTION_CUSTOM: {
	const Tk_ObjCustomOption *custom = optionPtr->extra.custom;
	if (internalFormExists && custom->freeProc != NULL) {
	    custom->freeProc(custom->clientData, tkwin, (char *)internalPtr);
	}
	break;
    }
    default:
	break;
    }
}

/*
 *--------------------------------------------------------------
 *
 * Tk_GetOptionInfo --
 *
 *	Returns a list object containing complete information about either a
 *	single option or all the configuration options in a table.
 *
 * Results:
 *	This function normally returns a pointer to an object. If namePtr
 *	isn't NULL, then the result object is a list with five elements: the
 *	option's name, its database name, database class, default value, and
 *	current value. If the option is a synonym then the list will contain
 *	only two values: the option name and the name of the option it refers
 *	to. If namePtr is NULL, then information is returned for every option
 *	in the option table: the result will have one sub-list (in the form
 *	described above) for each option in the table. If an error occurs
 *	(e.g. because namePtr isn't valid) then NULL is returned and an error
 *	message will be left in interp's result unless interp is NULL.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

Tcl_Obj *
Tk_GetOptionInfo(
    Tcl_Interp *interp,		/* Interpreter for error reporting. If NULL,
				 * then no error message is created. */
    void *recordPtr,		/* Record whose fields contain current values
				 * for options. */
    Tk_OptionTable optionTable,	/* Describes all the legal options. */
    Tcl_Obj *namePtr,		/* If non-NULL, the string value selects a
				 * single option whose info is to be returned.
				 * Otherwise info is returned for all options
				 * in optionTable. */
    Tk_Window tkwin)		/* Window associated with recordPtr; needed to
				 * compute correct default value for some
				 * options. */
{
    Tcl_Obj *resultPtr;
    OptionTable *tablePtr = (OptionTable *) optionTable;
    Option *optionPtr;
    size_t count;

    /*
     * If information is only wanted for a single configuration spec, then
     * handle that one spec specially.
     */

    if (namePtr != NULL) {
	optionPtr = GetOptionFromObj(interp, namePtr, tablePtr);
	if (optionPtr == NULL) {
	    return NULL;
	}
	if (optionPtr->specPtr->type == TK_OPTION_SYNONYM) {
	    optionPtr = optionPtr->extra.synonymPtr;
	}
	return GetConfigList(recordPtr, optionPtr, tkwin);
    }

    /*
     * Loop through all the specs, creating a big list with all their
     * information.
     */

    resultPtr = Tcl_NewListObj(0, NULL);
    for (; tablePtr != NULL; tablePtr = tablePtr->nextPtr) {
	for (optionPtr = tablePtr->options, count = tablePtr->numOptions;
		count > 0; optionPtr++, count--) {
	    Tcl_ListObjAppendElement(interp, resultPtr,
		    GetConfigList(recordPtr, optionPtr, tkwin));
	}
    }
    return resultPtr;
}

/*
 *--------------------------------------------------------------
 *
 * GetConfigList --
 *
 *	Create a valid Tcl list holding the configuration information for a
 *	single configuration option.
 *
 * Results:
 *	A Tcl list, dynamically allocated. The caller is expected to arrange
 *	for this list to be freed eventually.
 *
 * Side effects:
 *	Memory is allocated.
 *
 *--------------------------------------------------------------
 */

static Tcl_Obj *
GetConfigList(
    void *recordPtr,		/* Pointer to record holding current values of
				 * configuration options. */
    Option *optionPtr,		/* Pointer to information describing a
				 * particular option. */
    Tk_Window tkwin)		/* Window corresponding to recordPtr. */
{
    Tcl_Obj *listPtr, *elementPtr;

    listPtr = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(NULL, listPtr,
	    Tcl_NewStringObj(optionPtr->specPtr->optionName, TCL_INDEX_NONE));

    if (optionPtr->specPtr->type == TK_OPTION_SYNONYM) {
	elementPtr = Tcl_NewStringObj(
		optionPtr->extra.synonymPtr->specPtr->optionName, TCL_INDEX_NONE);
	Tcl_ListObjAppendElement(NULL, listPtr, elementPtr);
    } else {
	if (optionPtr->dbNameUID == NULL) {
	    elementPtr = Tcl_NewObj();
	} else {
	    elementPtr = Tcl_NewStringObj(optionPtr->dbNameUID, TCL_INDEX_NONE);
	}
	Tcl_ListObjAppendElement(NULL, listPtr, elementPtr);

	if (optionPtr->dbClassUID == NULL) {
	    elementPtr = Tcl_NewObj();
	} else {
	    elementPtr = Tcl_NewStringObj(optionPtr->dbClassUID, TCL_INDEX_NONE);
	}
	Tcl_ListObjAppendElement(NULL, listPtr, elementPtr);

	if ((tkwin != NULL) && ((optionPtr->specPtr->type == TK_OPTION_COLOR)
		|| (optionPtr->specPtr->type == TK_OPTION_BORDER))
		&& (Tk_Depth(tkwin) <= 1)
		&& (optionPtr->extra.monoColorPtr != NULL)) {
	    elementPtr = optionPtr->extra.monoColorPtr;
	} else if (optionPtr->defaultPtr != NULL) {
	    elementPtr = optionPtr->defaultPtr;
	} else {
	    elementPtr = Tcl_NewObj();
	}
	Tcl_ListObjAppendElement(NULL, listPtr, elementPtr);

	if (optionPtr->specPtr->objOffset != TCL_INDEX_NONE) {
	    elementPtr = *((Tcl_Obj **) ((char *)recordPtr
		    + optionPtr->specPtr->objOffset));
	    if (elementPtr == NULL) {
		elementPtr = Tcl_NewObj();
	    }
	} else {
	    elementPtr = GetObjectForOption(recordPtr, optionPtr, tkwin);
	}
	Tcl_ListObjAppendElement(NULL, listPtr, elementPtr);
    }
    return listPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * GetObjectForOption --
 *
 *	This function is called to create an object that contains the value
 *	for an option. It is invoked by GetConfigList and Tk_GetOptionValue
 *	when only the internal form of an option is stored in the record.
 *
 * Results:
 *	The return value is a pointer to a Tcl object. The caller must call
 *	Tcl_IncrRefCount on this object to preserve it.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static Tcl_Obj *
GetObjectForOption(
    void *recordPtr,		/* Pointer to record holding current values of
				 * configuration options. */
    Option *optionPtr,		/* Pointer to information describing an option
				 * whose internal value is stored in
				 * *recordPtr. */
    Tk_Window tkwin)		/* Window corresponding to recordPtr. */
{
    Tcl_Obj *objPtr = NULL;
    void *internalPtr;		/* Points to internal value of option in record. */

    if (optionPtr->specPtr->internalOffset != TCL_INDEX_NONE) {
	internalPtr = (char *)recordPtr + optionPtr->specPtr->internalOffset;
	switch (optionPtr->specPtr->type) {
	case TK_OPTION_BOOLEAN: {
	    int value = -1;
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    value = *((signed char *)internalPtr);
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    value = *((short *)internalPtr);
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_BOOLEAN");
		}
	    } else {
		value = *((int *)internalPtr);
	    }
	    if (value != -1) {
		objPtr = Tcl_NewBooleanObj(value);
	    }
	    break;
	}
	case TK_OPTION_INT: {
	    Tcl_WideInt value = LLONG_MIN;
	    int nullOK = (optionPtr->specPtr->flags & (TK_OPTION_NULL_OK|TCL_NULL_OK|1));
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TYPE_MASK) {
		    if (sizeof(long) > sizeof(int)) {
			value = *((long *)internalPtr);
			if (nullOK && (value == LONG_MIN)) {break;}
		    } else {
			value = *((long long *)internalPtr);
			if (nullOK && (value == LLONG_MIN)) {break;}
		    }
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_INT");
		}
	    } else {
		value = *((int *)internalPtr);
		if (nullOK && (value == INT_MIN)) {break;}
	    }
		objPtr = Tcl_NewWideIntObj(value);
	    break;
	}
	case TK_OPTION_INDEX:
	    if (!(optionPtr->specPtr->flags & (TK_OPTION_NULL_OK|TCL_NULL_OK|1)) || *((int *)internalPtr) != INT_MIN) {
		if (*((int *)internalPtr) == INT_MIN) {
		    objPtr = TkNewIndexObj(TCL_INDEX_NONE);
		} else if (*((int *)internalPtr) == INT_MAX) {
		    objPtr = Tcl_NewStringObj("end+1", TCL_INDEX_NONE);
		} else if (*((int *)internalPtr) == -1) {
		    objPtr = Tcl_NewStringObj("end", TCL_INDEX_NONE);
		} else if (*((int *)internalPtr) < 0) {
		    char buf[32];
		    snprintf(buf, 32, "end%d", 1 + *((int *)internalPtr));
		    objPtr = Tcl_NewStringObj(buf, TCL_INDEX_NONE);
		} else {
		    objPtr = Tcl_NewWideIntObj(*((int *)internalPtr));
		}
	    }
	    break;
	case TK_OPTION_DOUBLE:
	    if (!(optionPtr->specPtr->flags & (TK_OPTION_NULL_OK|TCL_NULL_OK|1)) || !isnan(*((double *)internalPtr))) {
		objPtr = Tcl_NewDoubleObj(*((double *)internalPtr));
	    }
	    break;
	case TK_OPTION_STRING:
	    objPtr = Tcl_NewStringObj(*((char **)internalPtr), TCL_INDEX_NONE);
	    break;
	case TK_OPTION_STRING_TABLE: {
	    int value = 0;
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    value = *((signed char *)internalPtr);
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    value = *((short *)internalPtr);
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_STRING_TABLE");
		}
	    } else {
		value = *((int *)internalPtr);
	    }
	    if (value >= 0) {
		objPtr = Tcl_NewStringObj(((char **)optionPtr->specPtr->clientData)[
			value], TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_COLOR: {
	    XColor *colorPtr = *((XColor **)internalPtr);

	    if (colorPtr != NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfColor(colorPtr), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_FONT: {
	    Tk_Font tkfont = *((Tk_Font *)internalPtr);

	    if (tkfont != NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfFont(tkfont), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_STYLE: {
	    Tk_Style style = *((Tk_Style *)internalPtr);

	    if (style != NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfStyle(style), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_BITMAP: {
	    Pixmap pixmap = *((Pixmap *)internalPtr);

	    if (pixmap != None) {
		objPtr = Tcl_NewStringObj(
		    Tk_NameOfBitmap(Tk_Display(tkwin), pixmap), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_BORDER: {
	    Tk_3DBorder border = *((Tk_3DBorder *)internalPtr);

	    if (border != NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOf3DBorder(border), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_RELIEF: {
	    int value = TK_RELIEF_NULL;
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    value = *((signed char *)internalPtr);
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    value = *((short *)internalPtr);
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_RELIEF");
		}
	    } else {
		value = *((int *)internalPtr);
	    }
	    if (value != TK_RELIEF_NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfRelief(value), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_CURSOR: {
	    Tk_Cursor cursor = *((Tk_Cursor *)internalPtr);

	    if (cursor != NULL) {
		objPtr = Tcl_NewStringObj(
		Tk_NameOfCursor(Tk_Display(tkwin), cursor), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_JUSTIFY: {
	    Tk_Justify value = TK_JUSTIFY_NULL;
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    value = (Tk_Justify)*((signed char *)internalPtr);
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    value = (Tk_Justify)*((short *)internalPtr);
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_JUSTIFY");
		}
	    } else {
		value = (Tk_Justify)*((int *)internalPtr);
	    }
	    if (value != TK_JUSTIFY_NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfJustify(value), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_ANCHOR: {
	    Tk_Anchor value = TK_ANCHOR_NULL;
	    if (optionPtr->specPtr->flags & TYPE_MASK) {
		if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(char)) {
		    value = (Tk_Anchor)*((signed char *)internalPtr);
		} else if ((optionPtr->specPtr->flags & TYPE_MASK) == TK_OPTION_VAR(short)) {
		    value = (Tk_Anchor)*((short *)internalPtr);
		} else {
		    Tcl_Panic("Invalid flags for %s", "TK_OPTION_ANCHOR");
		}
	    } else {
		value = (Tk_Anchor)*((int *)internalPtr);
	    }
	    if (value != TK_ANCHOR_NULL) {
		objPtr = Tcl_NewStringObj(Tk_NameOfAnchor(value), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_PIXELS:
	    if (!(optionPtr->specPtr->flags & (TK_OPTION_NULL_OK|TCL_NULL_OK|1)) || *((int *)internalPtr) != INT_MIN) {
		objPtr = Tcl_NewWideIntObj(*((int *)internalPtr));
	    }
	    break;
	case TK_OPTION_WINDOW: {
	    tkwin = *((Tk_Window *)internalPtr);

	    if (tkwin != NULL) {
		objPtr = Tcl_NewStringObj(Tk_PathName(tkwin), TCL_INDEX_NONE);
	    }
	    break;
	}
	case TK_OPTION_CUSTOM: {
	    const Tk_ObjCustomOption *custom = optionPtr->extra.custom;

	    objPtr = custom->getProc(custom->clientData, tkwin, (char *)recordPtr,
		    optionPtr->specPtr->internalOffset);
	    break;
	}
	default:
	    Tcl_Panic("bad option type in GetObjectForOption");
	}
    }
    if (objPtr == NULL) {
	objPtr = Tcl_NewObj();
    }
    return objPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetOptionValue --
 *
 *	This function returns the current value of a configuration option.
 *
 * Results:
 *	The return value is the object holding the current value of the option
 *	given by namePtr. If no such option exists, then the return value is
 *	NULL and an error message is left in interp's result (if interp isn't
 *	NULL).
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Tcl_Obj *
Tk_GetOptionValue(
    Tcl_Interp *interp,		/* Interpreter for error reporting. If NULL
				 * then no messages are provided for
				 * errors. */
    void *recordPtr,		/* Record whose fields contain current values
				 * for options. */
    Tk_OptionTable optionTable,	/* Describes legal options. */
    Tcl_Obj *namePtr,		/* Gives the command-line name for the option
				 * whose value is to be returned. */
    Tk_Window tkwin)		/* Window corresponding to recordPtr. */
{
    OptionTable *tablePtr = (OptionTable *) optionTable;
    Option *optionPtr;
    Tcl_Obj *resultPtr;

    optionPtr = GetOptionFromObj(interp, namePtr, tablePtr);
    if (optionPtr == NULL) {
	return NULL;
    }
    if (optionPtr->specPtr->type == TK_OPTION_SYNONYM) {
	optionPtr = optionPtr->extra.synonymPtr;
    }
    if (optionPtr->specPtr->objOffset != TCL_INDEX_NONE) {
	resultPtr = *((Tcl_Obj **) ((char *)recordPtr+optionPtr->specPtr->objOffset));
	if (resultPtr == NULL) {
	    /*
	     * This option has a null value and is represented by a null
	     * object pointer. We can't return the null pointer, since that
	     * would indicate an error. Instead, return a new empty object.
	     */

	    resultPtr = Tcl_NewObj();
	}
    } else {
	resultPtr = GetObjectForOption(recordPtr, optionPtr, tkwin);
    }
    return resultPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * TkDebugConfig --
 *
 *	This is a debugging function that returns information about one of the
 *	configuration tables that currently exists for an interpreter.
 *
 * Results:
 *	If the specified table exists in the given interpreter, then a list is
 *	returned describing the table and any other tables that it chains to:
 *	for each table there will be three list elements giving the reference
 *	count for the table, the number of elements in the table, and the
 *	command-line name for the first option in the table. If the table
 *	doesn't exist in the interpreter then an empty object is returned.
 *	The reference count for the returned object is 0.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Tcl_Obj *
TkDebugConfig(
    TCL_UNUSED(Tcl_Interp *),		/* Interpreter in which the table is
				 * defined. */
    Tk_OptionTable table)	/* Table about which information is to be
				 * returned. May not necessarily exist in the
				 * interpreter anymore. */
{
    OptionTable *tablePtr = (OptionTable *) table;
    Tcl_HashEntry *hashEntryPtr;
    Tcl_HashSearch search;
    Tcl_Obj *objPtr;
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
	    Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

    objPtr = Tcl_NewObj();
    if (!tablePtr || !tsdPtr->initialized) {
	return objPtr;
    }

    /*
     * Scan all the tables for this interpreter to make sure that the one we
     * want still is valid.
     */

    for (hashEntryPtr = Tcl_FirstHashEntry(&tsdPtr->hashTable, &search);
	    hashEntryPtr != NULL;
	    hashEntryPtr = Tcl_NextHashEntry(&search)) {
	if (tablePtr == (OptionTable *) Tcl_GetHashValue(hashEntryPtr)) {
	    for ( ; tablePtr != NULL; tablePtr = tablePtr->nextPtr) {
		Tcl_ListObjAppendElement(NULL, objPtr,
			Tcl_NewWideIntObj((Tcl_WideInt)tablePtr->refCount));
		Tcl_ListObjAppendElement(NULL, objPtr,
			Tcl_NewWideIntObj((Tcl_WideInt)tablePtr->numOptions));
		Tcl_ListObjAppendElement(NULL, objPtr, Tcl_NewStringObj(
			tablePtr->options[0].specPtr->optionName, TCL_INDEX_NONE));
	    }
	    break;
	}
    }
    return objPtr;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
