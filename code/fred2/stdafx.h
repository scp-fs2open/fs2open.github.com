/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



// #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _AFX_NO_DEBUG_CRT	// don't want extra memory debugging from MFC

#define W_ASSUME(x)
#ifndef _DEBUG
	#define _ASSERTE(x)	do { W_ASSUME(x) } while (0)
#else
	extern void _cdecl WinAssert(char * text,char *filename, int line);
	#define _ASSERTE(x) do { if (!(x)){ WinAssert(#x,__FILE__,__LINE__); } W_ASSUME( x ); } while (0)
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows 95 Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
  
