/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/openil/il_func.cpp $
 * $Revision: 1.3 $
 * $Date: 2004-07-17 18:38:04 $
 * $Author: taylor $
 *
 * Callback functions for OpenIL (DevIL) to use CFILE
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.1  2004/04/26 02:05:17  taylor
 * initial checkin
 *
 * 
 * $NoKeywords: $
 */


#include "openil/il_func.h"
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

int il_inited = 0;

#ifdef USE_DEVIL

// callback function to open a file
ILHANDLE ILAPIENTRY icfOpen(char *const filename)
{
	CFILE *cfh;

	cfh = cfopen(filename, "rb");

	if (!cfh)
		return NULL;

	return (ILHANDLE)cfh;
}

// close
ILvoid ILAPIENTRY icfClose(ILHANDLE cfh)
{
	cfclose( (CFILE*)cfh );
	
	return;
}

// eof
ILboolean ILAPIENTRY icfEof(ILHANDLE cfh)
{
	int rc = -1;
	
	rc = cfeof( (CFILE*)cfh );

	if (rc && rc != -1)
		return (ubyte)rc;
	
	return 0;
}

// get a single character
ILint ILAPIENTRY icfGetc(ILHANDLE cfh)
{
	int rc;
	
	rc = cfgetc( (CFILE*)cfh );

	return rc;
}

// read
ILint ILAPIENTRY icfRead(ILvoid *buf, ILuint size, ILuint number, ILHANDLE cfh)
{
	if (number == 0)
		return 0;

	return cfread(buf, (int)size, (int)number, (CFILE*)cfh);
}

// seek in a file
ILint ILAPIENTRY icfSeek(ILHANDLE cfh, ILint offset, ILint mode)
{
	int cf_mode = -1;

	switch (mode) {
		case SEEK_SET:
			cf_mode = CF_SEEK_SET;
			break;
		case SEEK_CUR:
			cf_mode = CF_SEEK_CUR;
			break;
		case SEEK_END:
			cf_mode = CF_SEEK_END;
			break;
		default:
			break;
	}

	return cfseek( (CFILE*)cfh, offset, cf_mode );
}

// give current position in a file
ILint ILAPIENTRY icfTell(ILHANDLE cfh)
{
	return cftell( (CFILE*)cfh );
}



// initialize the library and setup callbacks
void openil_init()
{
	if (il_inited)
		return;

	// initialize libraries
	ilInit();
	
	iluInit();
	
	ilutInit(); // not sure if we need ILUT but leave it for now
	
	// setup the read callbacks
	ilSetRead(icfOpen, icfClose, icfEof, icfGetc, icfRead, icfSeek, icfTell);

	// force origin to upper left on all images - our OpenGL stuff requires this
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	il_inited = 1;
}

// close out libs
void openil_close()
{
	if (!il_inited)
		return;

	ilShutDown();
}

#endif // USE_DEVIL
