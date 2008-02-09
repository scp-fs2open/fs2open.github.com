/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Pixel.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:57 $
 * $Author: penguin $
 *
 * Routines to plot a dot.
 *
 * $Log: not supported by cvs2svn $
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
 * 14    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 13    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 12    1/13/98 10:20a John
 * Added code to support "glass" in alphacolors
 * 
 * 11    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 10    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 9     10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 8     6/06/97 2:40p John
 * Made all the radar dim in/out
 * 
 * 7     5/29/97 3:09p John
 * Took out debug menu.  
 * Made software scaler draw larger bitmaps.
 * Optimized Direct3D some.
 * 
 * 6     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 5     1/09/97 11:35a John
 * Added some 2d functions to get/put screen images.
 * 
 * 4     11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 3     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#include "2d.h"
#include "grinternal.h"
#include "pixel.h"
#include "palman.h"

void gr8_pixel( int x, int y )
{
	ubyte * dptr;

	if ( x < gr_screen.clip_left ) return;
	if ( x > gr_screen.clip_right ) return;
	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;

	gr_lock();

	dptr = GR_SCREEN_PTR(ubyte,x, y);
	if ( Current_alphacolor )	{
		// *dptr = Current_alphacolor->table.lookup[14][*dptr];
	} else {
		*dptr = gr_screen.current_color.raw8;
	}

	gr_unlock();
}



