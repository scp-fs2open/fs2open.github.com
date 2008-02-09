/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrZbuffer.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Code for the software renderer's zbuffer
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/05/08 02:36:01  mharris
 * porting
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 1     3/25/98 8:07p John
 * Split software renderer into Win32 and DirectX
 *
 * $NoKeywords: $
 */

#include "pstypes.h"
#include "osapi.h"
#include "2d.h"
#include "floating.h"
#include "grsoft.h"
#include "grinternal.h"

int gr_zcount=GR_Z_COUNT;
int gr_zoffset=0;

uint *gr_zbuffer = NULL;
int gr_zbuffer_w = 0;
int gr_zbuffer_h = 0;

int gr_zbuffering = 0;
int gr_zbuffering_mode = 0;
int gr_global_zbuffering = 0;

// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr8_zbuffer_clear(int mode)
{
	if ( mode )	{
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		if ( (!gr_zbuffer) || (gr_screen.max_w!=gr_zbuffer_w) || (gr_screen.max_h!=gr_zbuffer_h) )	{
			//mprintf(( "Allocating a %d x %d zbuffer\n", gr_screen.max_w, gr_screen.max_h ));
			if ( gr_zbuffer )	{
				free(gr_zbuffer);
				gr_zbuffer = NULL;
			}
			gr_zbuffer_w = gr_screen.max_w;
			gr_zbuffer_h = gr_screen.max_h;
			gr_zbuffer = (uint *)malloc(gr_zbuffer_w*gr_zbuffer_h*sizeof(uint));
			if ( !gr_zbuffer )	{
				Error( LOCATION, "Couldn't allocate zbuffer\n" );
				gr_zbuffering = 0;
				return;
			}
			memset( gr_zbuffer, 0, gr_zbuffer_w*gr_zbuffer_h*sizeof(uint) );
		}


		gr_zcount++;
		gr_zoffset += GR_Z_RANGE;
		if ( gr_zcount >= (GR_Z_COUNT-16) )	{
			//mprintf(( "Bing!\n" ));
			memset( gr_zbuffer, 0, gr_zbuffer_w*gr_zbuffer_h*sizeof(uint) );
			gr_zcount = 0;
			gr_zoffset = GR_Z_RANGE*16;
		}
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}


int gr8_zbuffer_get()
{
	if ( !gr_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr8_zbuffer_set(int mode)
{
	if ( !gr_global_zbuffering )	{
		gr_zbuffering = 0;
		return GR_ZBUFF_NONE;
	}

	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if ( gr_zbuffering_mode == GR_ZBUFF_NONE )	{
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}
