/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Line.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-29 20:12:31 $
 * $Author: penguin $
 *
 * Routines for drawing lines.
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
 * 21    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 20    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 19    1/13/98 10:20a John
 * Added code to support "glass" in alphacolors
 * 
 * 18    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 17    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 16    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 15    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 14    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 13    9/09/97 10:39a Sandeep
 * Fixed compiler warnings level 4 (sorta, john is fixing most of it)
 * 
 * 12    6/13/97 5:35p John
 * added some antialiased bitmaps and lines
 * 
 * 11    6/06/97 2:40p John
 * Made all the radar dim in/out
 * 
 * 10    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 9     11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 8     10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 7     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include "2d.h"
#include "grinternal.h"
#include "floating.h"
#include "line.h"
#include "key.h"


void gr8_uline(int x1,int y1,int x2,int y2)
{
	int i;
   int xstep,ystep;
   int dy=y2-y1;
   int dx=x2-x1;
   int error_term=0;

	gr_lock();
	ubyte *dptr = GR_SCREEN_PTR(ubyte,x1,y1);
	ubyte color = gr_screen.current_color.raw8;
		
	if(dy<0)	{
		dy=-dy;
      ystep=-gr_screen.rowsize / gr_screen.bytes_per_pixel;
	}	else	{
      ystep=gr_screen.rowsize / gr_screen.bytes_per_pixel;
	}

   if(dx<0)	{
      dx=-dx;
      xstep=-1;
   } else {
      xstep=1;
	}

	/* HARDWARE_ONLY - removed alpha color table stuff
	if ( Current_alphacolor )	{
		if(dx>dy)	{

			for(i=dx+1;i>0;i--) {
				*dptr = Current_alphacolor->table.lookup[14][*dptr];
				dptr += xstep;
				error_term+=dy;

				if(error_term>dx)	{
					error_term-=dx;
					dptr+=ystep;
				}
			}
		} else {

			for(i=dy+1;i>0;i--)	{
				*dptr = Current_alphacolor->table.lookup[14][*dptr];
				dptr += ystep;
				error_term+=dx;
				if(error_term>0)	{
					error_term-=dy;
					dptr+=xstep;
				}

			}

		}
	} else {
	*/
		if(dx>dy)	{

			for(i=dx+1;i>0;i--) {
				*dptr = color;
				dptr += xstep;
				error_term+=dy;

				if(error_term>dx)	{
					error_term-=dx;
					dptr+=ystep;
				}
			}
		} else {

			for(i=dy+1;i>0;i--)	{
				*dptr = color;
				dptr += ystep;
				error_term+=dx;
				if(error_term>0)	{
					error_term-=dy;
					dptr+=xstep;
				}

			}

		}	
	gr_unlock();
}  



void gr8_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	gr8_uline(x1,y1,x2,y2);
}



