/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Rect.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Routines to draw rectangles.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 3     3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 2     10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 1     10/26/96 1:32p John
 * Initial rev
 *
 * $NoKeywords: $
 */

#include "graphics/2d.h"
#include "graphics/grinternal.h"

void grx_rect(int x,int y,int w,int h)
{
	int i;
	int x1 = x, x2;
	int y1 = y, y2;

	if ( w > 0 )
		 x2 = x + w - 1;
	else
		 x2 = x + w + 1;

	if ( h > 0 )
		y2 = y + h - 1;
	else
		y2 = y + h + 1;
		
	if ( x2 < x1 )	{
		int tmp;	
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}

	if ( y2 < y1 )	{
		int tmp;	
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	// Check for completely offscreen!
	if ( x1 > gr_screen.clip_right )
		return;

	if ( x2 < gr_screen.clip_left )
		return;

	if ( y1 > gr_screen.clip_bottom )
		return;

	if ( y2 < gr_screen.clip_top )
		return;

	// Now clip
	if ( x1 < gr_screen.clip_left ) 
		x1 = gr_screen.clip_left;

	if ( x2 > gr_screen.clip_right ) 
		x2 = gr_screen.clip_right;

	if ( y1 < gr_screen.clip_top ) 
		y1 = gr_screen.clip_top;

	if ( y2 > gr_screen.clip_bottom ) 
		y2 = gr_screen.clip_bottom;

	w = x2-x1+1;
	if ( w < 1 ) return;

	h = y2-y1+1;
	if ( h < 1 ) return;

	gr_lock();

	ubyte *dptr;

	/* HARDWARE_ONLY
	if ( Current_alphacolor )	{
		for (i=0; i<h; i++ )	{
			dptr = GR_SCREEN_PTR(ubyte,x1,y1+i);

			int j;
			for( j=0; j<w; j++ )	{
				*dptr++ = Current_alphacolor->table.lookup[14][*dptr];
			}
		}
	} else {
	*/
		for (i=0; i<h; i++ )	{
			dptr = GR_SCREEN_PTR(ubyte,x1,y1+i);
			memset( dptr, gr_screen.current_color.raw8, w );
		}	
	gr_unlock();

}

