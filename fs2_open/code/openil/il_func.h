/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/openil/il_func.h $
 * $Revision: 1.1 $
 * $Date: 2004-04-26 02:04:30 $
 * $Author: taylor $
 *
 * Callback functions for OpenIL (DevIL) to use CFILE
 * 
 * $Log: not supported by cvs2svn $
 * 
 * $NoKeywords: $
 */


#ifndef _IL_FUNC_H
#define _IL_FUNC_H


extern int il_inited;

#ifdef USE_DEVIL

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>


// for individual format support
//#define USE_DEVIL_PCX  // removed for the time being
#define USE_DEVIL_TGA
#define USE_DEVIL_DDS
#define USE_DEVIL_JPG

// init libs and the callbacks
void openil_init();

// close out
void openil_close();

#else

// stub defines when DevIL isn't used
#define openil_init()
#define openil_close()

#endif // USE_DEVIL

#endif // _IL_FUNC_H
