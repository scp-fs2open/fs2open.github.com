/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Gradient.cpp $
 * $Revision: 2.2 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Routines to draw rectangular gradients.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/29 20:12:31  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 17    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 16    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 15    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 14    11/30/97 4:04p John
 * 
 * 13    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 12    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 11    10/14/97 4:50p John
 * more 16 bpp stuff.
 * 
 * 10    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 9     10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 8     6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 7     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 6     12/12/96 4:59p Lawrance
 * made clipping for horizontal and vertical gradient lines right
 * 
 * 5     11/19/96 2:46p Allender
 * fix up gradient support for 15bpp
 * 
 * 4     11/18/96 4:34p Allender
 * new 16 bit gradient functions
 * 
 * 3     10/27/96 1:21a Lawrance
 * added check to avoid divide by zero when calculating gradients
 * 
 * 2     10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 1     10/26/96 1:45p John
 * Initial skeletion code.
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gradient.h"
#include "math/floating.h"
#include "graphics/line.h"
#include "palman/palman.h"
		
void gr8_gradient(int x1,int y1,int x2,int y2)
{
#ifndef HARDWARE_ONLY
	int i;
   int xstep,ystep;
	int clipped = 0, swapped=0;
	ubyte *xlat_table;

	int l=0, dl=0;

	if (!Current_alphacolor) {
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

   int dy=y2-y1;
   int dx=x2-x1;
   int error_term=0;

	if ( dx==0 && dy==0 )	{
		return;
	}

	gr_lock();

	ubyte *dptr = GR_SCREEN_PTR(ubyte,x1,y1);

	xlat_table = (ubyte *)&Current_alphacolor->table.lookup[0][0];

	
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

	if(dx>dy)	{

		if (!swapped)	{
			l = 14<<8;
			dl = (-14<<8) / dx;
		} else {
			l = 0;
			dl = (14<<8) / dx;
		}	

		for(i=0;i<dx;i++) {
			*dptr = xlat_table[(l&0xF00)|*dptr];
			l += dl;
			dptr += xstep;
         error_term+=dy;

         if(error_term>dx)	{
				error_term-=dx;
            dptr+=ystep;
         }
      }
		*dptr = xlat_table[(l&0xF00)|*dptr];

   } else {

		if (!swapped)	{
			l = 14<<8;
			dl = (-14<<8) / dy;
		} else {
			l = 0;
			dl = (14<<8) / dy;
		}	

      for(i=0;i<dy;i++)	{
			*dptr = xlat_table[(l&0xF00)|*dptr];
			l += dl;
			dptr += ystep;
         error_term+=dx;
         if(error_term>0)	{
            error_term-=dy;
            dptr+=xstep;
         }

      }
		*dptr = xlat_table[(l&0xF00)|*dptr];

   }
	gr_unlock();
#else 
	Int3();
#endif
}


		


