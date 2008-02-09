/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Circle.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:22 $
 * $Author: penguin $
 *
 * Code to draw circles.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 9     3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 8     11/06/97 11:18a John
 * fixed bug with some scanlines being drawn twice
 * 
 * 7     11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 6     11/26/96 4:30p John
 * Put in some better quality circle_empty code.
 * 
 * 5     11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 4     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#include "2d.h"
#include "pixel.h"
#include "circle.h"

// THIS COULD BE OPTIMIZED BY MOVING THE GR_RECT CODE INTO HERE!!!!!!!!

#define circle_fill(x,y,w) gr_rect((x),(y),(w),1)

void gr8_circle( int xc, int yc, int d )
{
	int p,x, y, r;

	r = d/2;
	p=3-d;
	x=0;
	y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		circle_fill( xc-y, yc-x, y*2+1 );
		if ( x != 0 )	
			circle_fill( xc-y, yc+x, y*2+1 );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			circle_fill( xc-x, yc-y, x*2+1 );
			if ( y != 0 )	
				circle_fill( xc-x, yc+y, x*2+1 );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}

	if(x==y)	{
		circle_fill( xc-x, yc-y, x*2+1 );
		if ( y != 0 )	
			circle_fill( xc-x, yc+y, x*2+1 );
	}

}

