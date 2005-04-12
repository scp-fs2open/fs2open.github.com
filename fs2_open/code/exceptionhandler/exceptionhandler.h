/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/ExceptionHandler/ExceptionHandler.h $
 * $Revision: 2.3 $
 * $Date: 2005-04-12 05:26:36 $
 * $Author: taylor $
 *
 * Header file for program exception handling
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/08/11 05:06:22  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/07 02:58:10  mharris
 * ifdef around pragma
 *
 * Revision 1.1  2002/05/02 18:03:05  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 1     6/29/99 7:42p Dave
 * 
 * 2     1/18/99 4:34p Allender
 * added the exception handler routines from Game Developer for structured
 * exception handling in vsdk code
 *
 * $NoKeywords: $
 */


#ifndef __EXCEPTION_HANDLER_H
#define __EXCEPTION_HANDLER_H

#include "PreProcDefines.h"


// --------------------
//
// Defines
//
// --------------------


// --------------------
//
// Enumerated types
//
// --------------------


// --------------------
//
// Structures
//
// --------------------

// this is a forward declaration so we don't need to include windows.h

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

// --------------------
//
// Classes
//
// --------------------


// --------------------
//
// Variables
//
// --------------------


// --------------------
//
// Prototypes
//
// --------------------

int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, const char *Message);

// --------------------
//
// Helper Macros
//
// --------------------


#endif // __EXCEPTION_HANDLER_H
