/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/stdafx.h $
 * $Revision: 1.2 $
 * $Date: 2007-07-23 15:16:49 $
 * $Author: Kazan $
 *
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
 *
 * Above is the MFC generated notes on this file.  Basically, you probably don't need
 * do touch anything here.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/19 02:27:32  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 2     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

// #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define WINVER 0x0500		// windows 2000
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows 95 Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
  