/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/aaline.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-07 19:55:59 $
 * $Author: penguin $
 *
 * Code to draw antialiased lines
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
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
 * 1     10/07/98 10:48a Dave
 * 
 * 13    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 12    3/24/98 4:03p Lawrance
 * JOHN: Fix up outline drawing code to support different colors
 * 
 * 11    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 10    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 9     11/30/97 4:26p John
 * Added 32-bpp antialiased line.   Took gamma out of alphacolor
 * calculation.
 * 
 * 8     11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 7     10/20/97 8:47a John
 * fixed gr_lock bug in aaline
 * 
 * 6     10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 5     10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 4     10/04/97 11:27a John
 * took out debug code
 * 
 * 3     10/03/97 9:50a John
 * enabled antialiasing lines in alphacolor set.
 * 
 * 2     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 1     10/03/97 9:07a John
 *
 * $NoKeywords: $
 */

/*

 Code for drawing antialiased lines.  Taken from some code
 published in the Journal of Graphic Tools at www.acm.org/jgt

 Here is the README that came with the source code:

	Sample code to draw antialiased lines as described in the Journal of
	Graphic Tools article High Quality Hardware Line Antialiasing by
	Scott R. Nelson of Sun Microsystems.

	The code is written in C and designed to run on any machine with the
	addition of a proper "display" module.  Currently, display modules
	exist for Macintosh, Unix, and Wintel machines.  Thanks to Sanjay Gupta
	(sanjay.gupta@eng.sun.com) for the Unix X11 display code and Chris
	Babcock (babcock@rtp.idt.com) for the Windows code.

	This code is not 100% bug free and is definitely not optimized for
	performance.  It does, however, illustrate all of the points made in
	the JGT article.
*/


#include <stdio.h>
#include <stdlib.h>

#include "pstypes.h"
#include "2d.h"
#include "line.h"
#include "grinternal.h"
#include "palman.h"

// Convert from floating-point to internal fixed-point formats
#define ONE_XY			(long int) 0x00100000
#define FIX_XY_SHIFT	(long int) 20
#define ONEHALF_XY	(long int) 0x00080000
#define ONE_Z			(long int) 0x40000000
#define ONE_RGB		(long int) 0x40000000
#define ONE_16			(long int) 0x4000

#define FLOAT_TO_FIX_XY(x)	((long int) ((x) * (float) ONE_XY))

#define FLOAT_TO_FIX_RGB(x)	((long int) ((x) * (float) ONE_RGB))
#define FLOAT_TO_FIX_16(x)	((long int) ((x) * (float) ONE_16))
#define FIX_TO_INT_XY(x)	((x) >> FIX_XY_SHIFT)
#define FIX_16_TO_FLOAT(x)	((float) (x) / (float) ONE_16)
#define FIX_TO_FLOAT_XY(x)	((float) (x) / (float) ONE_XY)
#define FIX_TO_FLOAT_RGB(x)	((float) (x) / (float) ONE_RGB)

// Get fractional part, next lowest integer part
#define FRACT_XY(x)		((x) & (long int) 0x000fffff)
#define FLOOR_XY(x)		((x) & (long int) 0xfff00000)
#define FIX_XY_TO_INT(x)	((long int) (x) >> (long int) FIX_XY_SHIFT)

// Sizes for tables in Draw
#define FILTER_WIDTH	0.75		// Line filter width adjustment	// .75		// .5 works good with 5.0 gamma
#define F_TABLE_SIZE	64			// Filter table size
#define SC_TABLE_SIZE	32		// Slope correction table size
#define SRT_INT		5			// Sqrt table index integer bits
#define SRT_FRACT	4				//   ...fraction bits
#define SR_INT		3				// Square root result integer bits
#define SR_FRACT	5				//   ...fraction bits
#define SR_TABLE_SIZE	(1 << (SRT_INT + SRT_FRACT))
#define INV_FILTER 47

#define EP_MASK	(long int) 0x000f0000u	// AA line end-point filter mask
#define EP_SHIFT	13u			// Number of bits to shift end-point


typedef long int fix_xy;	// S11.20

// One vertex at any of the various stages of the pipeline

typedef struct aa_vertex {
	float x, y;
} aa_vertex;

// All values needed to draw one line
typedef struct aa_setup_line {
	int x_major;
	int negative;

	fix_xy vs;			// Starting point
	fix_xy us;
	fix_xy ue;			// End (along major axis)
	fix_xy dvdu;		// Delta for minor axis step

} aa_setup_line;


// Tables that need to be initialized
long int slope_corr_table[SC_TABLE_SIZE];
long int filter_table[F_TABLE_SIZE];
long int sqrt_table[SR_TABLE_SIZE];

ubyte new_table[F_TABLE_SIZE*512];

int aaline_inited = 0;

// Initialize the tables normally found in ROM in the hardware.
void aaline_init_tables()
{
	int i,j;					// Iterative counter
	double m;				// Slope
	double d;				// Distance from center of curve
	double v;				// Value to put in table
	double sr;				//	The square root value

	aaline_inited = 1;

	// Build slope correction table.  The index into this table
	// is the truncated 5-bit fraction of the slope used to draw
	// the line.  Round the computed values here to get the closest
	// fit for all slopes matching an entry.

	for (i = 0; i < SC_TABLE_SIZE; i++) {
		// Round and make a fraction
		m = ((double) i + 0.5) / (float) SC_TABLE_SIZE;
		v = sqrt(m * m + 1) * 0.707106781; /* (m + 1)^2 / sqrt(2) */
		slope_corr_table[i] = (long int) (v * 256.0);
	}

	// Build the Gaussian filter table, round to the middle of the sample region.
	for (i = 0; i < F_TABLE_SIZE; i++) {
		d = ((double) i + 0.5) / (float) (F_TABLE_SIZE / 2.0);
		d = d / FILTER_WIDTH;
		v = 1.0 / exp(d * d);		// Gaussian function
		filter_table[i] = (long int) (v * 256.0);
	}

	for ( i=0; i<512; i++ )	{
		long int corr_slope = i<<8;
		for (j=0; j<F_TABLE_SIZE; j++ )	{
			new_table[i*F_TABLE_SIZE+j] = (ubyte)(((corr_slope * filter_table[j]) & 0xf00000) >> (16+4));
			if (new_table[i*F_TABLE_SIZE+j]==15 )	{
				// HACK!!! Account for "glass" pixel for hud bitmaps.
				new_table[i*F_TABLE_SIZE+j] = 14;
			}
		}
	}
	

	// Build the square root table for big dots.
	for (i = 0; i < SR_TABLE_SIZE; i++) {
		v = (double) ((i << 1) + 1) / (double) (1 << (SRT_FRACT + 1));
		sr = sqrt(v);
		sqrt_table[i] = (long int) (sr * (double) (1 << SR_FRACT));
	}


}



// Multiply a fixed-point number by a s11.20 fixed-point
// number.  The actual multiply uses less bits for the
// multiplier, since it always represents a fraction
// less than 1.0 and less total bits are sufficient.
// Some of the steps here are not needed.  This was originally
// written to simulate exact hardware behavior.
#if defined( _WIN32 ) && defined( _MSC_VER )		
long int fix_xy_mult(long int oa, fix_xy ob)
{
	int retval;

	_asm {
		mov	edx, oa
		mov	eax, ob
		imul	edx
		shrd	eax,edx,20
		mov	retval, eax
	}
	return retval;
}
#else
long int fix_xy_mult(long int oa, fix_xy ob);
#endif // if defined( _WIN32 ) && defined( _MSC_VER )		



// Draw one span of an antialiased line (for horizontal lines).
void draw_aa_hspan8(fix_xy x, fix_xy y, long int ep_corr, long int slope)
{
#ifndef HARDWARE_ONLY
	long int sample_dist;		// Distance from line to sample point
	long int filter_index;		// Index into filter table
	long int i;						// Count pixels across span
	long int index;				// Final filter table index
	int a;							// Alpha

	sample_dist = (FRACT_XY(y) >> (FIX_XY_SHIFT - 5)) - 16;
	y = y - ONE_XY;
	filter_index = sample_dist + 32;


	int yi = FIX_XY_TO_INT( y );
	int xi = FIX_XY_TO_INT( x );

	if ( xi < gr_screen.clip_left ) return;
	if ( xi > gr_screen.clip_right ) return;

	int clipped = 0;
	
	if ( yi < gr_screen.clip_top ) clipped++;
	if ( yi+3 > gr_screen.clip_bottom ) clipped++;

	long int corr_slope = (slope * ep_corr) & 0x1ff00;

	ubyte * lookup = (ubyte *)&Current_alphacolor->table.lookup[0][0];

	ubyte * filter_lookup = (ubyte *)&new_table[(corr_slope>>8)*F_TABLE_SIZE];


	if ( clipped )	{
		ubyte * dptr;
		
		for (i = 0; i < 4; i++) {
			if (filter_index < 0)
				index = ~filter_index;	// Invert when negative
			else
				index = filter_index;

			if (index > INV_FILTER)	{
				Assert( i == 3 );
				return;			// Not a valid pixel
			}

			//a = ((corr_slope * filter_table[index]) & 0xf00000) >> (16+4-8);
			a = filter_lookup[index]<<8;

			// Should include the alpha value as well...
			if ( (yi >= gr_screen.clip_top) && (yi <= gr_screen.clip_bottom) )	{
				dptr = GR_SCREEN_PTR(ubyte,xi, yi);

				*dptr = lookup[*dptr+a];
			}

			filter_index -= 32;
			//y += ONE_XY;
			yi++;
		}
	} else {
		ubyte * dptr;

		dptr = GR_SCREEN_PTR(ubyte,xi, yi);
	
		for (i = 0; i < 4; i++) {
			if (filter_index < 0)
				index = ~filter_index;	// Invert when negative
			else
				index = filter_index;

			if (index > INV_FILTER)	{
				Assert( i == 3 );
				return;			// Not a valid pixel
			}

			a = filter_lookup[index]<<8;

			// Should include the alpha value as well...
			*dptr = lookup[*dptr+a];

			dptr += gr_screen.rowsize;

			filter_index -= 32;
		}


	}
#else
	Int3();
#endif
}

// draw_aa_vspan
// Draw one span of an antialiased line (for vertical lines).

void draw_aa_vspan8(fix_xy x, fix_xy y, long int ep_corr, long int slope)
{
#ifndef HARDWARE_ONLY
	long int sample_dist;		// Distance from line to sample point
	long int filter_index;		// Index into filter table 
	long int i;						// Count pixels across span
	long int index;				// Final filter table index
	int a;							// Alpha

	sample_dist = (FRACT_XY(x) >> (FIX_XY_SHIFT - 5)) - 16;
	x = x - ONE_XY;
	filter_index = sample_dist + 32;

	int yi = FIX_XY_TO_INT( y );
	int xi = FIX_XY_TO_INT( x );

	if ( yi < gr_screen.clip_top ) return;
	if ( yi > gr_screen.clip_bottom ) return;

	int clipped = 0;
	
	if ( xi < gr_screen.clip_left ) clipped++;
	if ( xi+3 > gr_screen.clip_right ) clipped++;

	long int corr_slope = (slope * ep_corr) & 0x1ff00;

	ubyte * lookup = (ubyte *)&Current_alphacolor->table.lookup[0][0];

	ubyte * filter_lookup = (ubyte *)&new_table[(corr_slope>>8)*F_TABLE_SIZE];


	if ( clipped )	{
		ubyte * dptr;

		for (i = 0; i < 4; i++) {
			if (filter_index < 0)
				index = ~filter_index;	// Invert when negative
			else
				index = filter_index;

			if (index > INV_FILTER)	{
				Assert( i == 3 );
				return;			// Not a valid pixel
			}

			//a = ((corr_slope * filter_table[index]) & 0xf00000) >> (16+4-8);
			a = filter_lookup[index]<<8;

			// Draw the pixel
			if ( (xi >= gr_screen.clip_left) && (xi <= gr_screen.clip_right) )	{
				dptr = GR_SCREEN_PTR(ubyte,xi, yi);

				*dptr = lookup[*dptr+a];
			}

			filter_index -= 32;
			xi++;
		}
	} else {

		ubyte *dptr = GR_SCREEN_PTR(ubyte,xi, yi);

		for (i = 0; i < 4; i++) {
			if (filter_index < 0)
				index = ~filter_index;	// Invert when negative
			else
				index = filter_index;

			if (index > INV_FILTER)	{
				Assert( i == 3 );
				return;			// Not a valid pixel
			}

			a = filter_lookup[index]<<8;

			// Should include the alpha value as well...

			// Draw the pixel
			*dptr = lookup[*dptr+a];

			filter_index -= 32;
			dptr++;
		}
	}
#else
	Int3();
#endif
}


void draw_line(aa_setup_line *line)
{
	fix_xy x, y;					//  Start value
	fix_xy dudu;					//  Constant 1 or -1 for step
	fix_xy dx, dy;					//  Steps in X and Y
	fix_xy u_off;					//  Offset to starting sample grid
	fix_xy us, vs, ue;			//  Start and end for drawing
	fix_xy count;					//  How many pixels to draw
	long int slope_index;		//  Index into slope correction table
	long int slope;				//  Slope correction value
	long int ep_corr;				//  End-point correction value
	long int scount, ecount;	//  Start/end count for endpoints
	long int sf, ef;				//  Sand and end fractions
	long int ep_code;				//  One of 9 endpoint codes

	// Get directions
	if (line->negative)	{
		dudu = -ONE_XY;
	} else {
		dudu = ONE_XY;
	}

	if (line->x_major) {
		dx = dudu;
		dy = line->dvdu;
	} else {
		dx = line->dvdu;
		dy = dudu;
	}

	// Get initial values and count
	if (line->negative) {
		u_off = FRACT_XY(line->us) - ONE_XY;
		us = line->us + ONE_XY;
		ue = line->ue;
		count = FLOOR_XY(us) - FLOOR_XY(ue);
	} else {
		u_off = 0 - FRACT_XY(line->us);
		us = line->us;
		ue = line->ue + ONE_XY;
		count = FLOOR_XY(ue) - FLOOR_XY(us);
	}

	vs = line->vs + fix_xy_mult(line->dvdu, u_off) + ONEHALF_XY;

	if (line->x_major) {
		x = us;
		y = vs;
	} else {
		x = vs;
		y = us;
	}

	//a = line->as + fix_xy_mult(line->dadu, u_off);

	// Compute slope correction once per line
	slope_index = (line->dvdu >> (FIX_XY_SHIFT - 5)) & 0x3fu;

	if (line->dvdu < 0)	{
		slope_index ^= 0x3fu;
	}

	if ((slope_index & 0x20u) == 0)	{
		slope = slope_corr_table[slope_index];
	} else {
		slope = 0x100;		/* True 1.0 */
	}

	// Set up counters for determining endpoint regions
	scount = 0;
	ecount = FIX_TO_INT_XY(count);

	// Get 4-bit fractions for end-point adjustments
	sf = (us & EP_MASK) >> EP_SHIFT;
	ef = (ue & EP_MASK) >> EP_SHIFT;

	// Interpolate the edges
	while (count >= 0) {

		/*-
		* Compute end-point code (defined as follows):
		*  0 =  0, 0: short, no boundary crossing
		*  1 =  0, 1: short line overlap (< 1.0)
		*  2 =  0, 2: 1st pixel of 1st endpoint
		*  3 =  1, 0: short line overlap (< 1.0)
		*  4 =  1, 1: short line overlap (> 1.0)
		*  5 =  1, 2: 2nd pixel of 1st endpoint
		*  6 =  2, 0: last of 2nd endpoint
		*  7 =  2, 1: first of 2nd endpoint
		*  8 =  2, 2: regular part of line
		*/

		ep_code = ((scount < 2) ? scount : 2) * 3 + ((ecount < 2) ? ecount : 2);

		if (line->negative) {

			// Drawing in the negative direction

			// Compute endpoint information
			switch (ep_code) {
				case 0: ep_corr = 0;									break;
				case 1: ep_corr = ((sf - ef) & 0x78) | 4;		break;
				case 2: ep_corr = sf | 4;							break;
				case 3: ep_corr = ((sf - ef) & 0x78) | 4;		break;
				case 4: ep_corr = ((sf - ef) + 0x80) | 4;		break;
				case 5: ep_corr = (sf + 0x80) | 4;				break;
				case 6: ep_corr = (0x78 - ef) | 4;				break;
				case 7: ep_corr = ((0x78 - ef) + 0x80) | 4;	break;
				case 8: ep_corr = 0x100;							break;
				default:	ep_corr = 0;								break;	
			}

		} else {
			// Drawing in the positive direction

			// Compute endpoint information
			switch (ep_code) {
				case 0: ep_corr = 0;									break;
				case 1: ep_corr = ((ef - sf) & 0x78) | 4;		break;
				case 2: ep_corr = (0x78 - sf) | 4;				break;
				case 3: ep_corr = ((ef - sf) & 0x78) | 4;		break;
				case 4: ep_corr = ((ef - sf) + 0x80) | 4;		break;
				case 5: ep_corr = ((0x78 - sf) + 0x80) | 4;	break;
				case 6: ep_corr = ef | 4;							break;
				case 7: ep_corr = (ef + 0x80) | 4;				break;
				case 8: ep_corr = 0x100;							break;
				default:	ep_corr = 0;								break;	
			} 
		}

		if (line->x_major)	{
			draw_aa_hspan8(x, y, ep_corr, slope);
		} else	{
			draw_aa_vspan8(x, y, ep_corr, slope);
		}

		x += dx;
		y += dy;
		//a += line->dadu;

		scount++;
		ecount--;
		count -= ONE_XY;
	}

}


// Perform the setup operation for a line, then draw it

void aaline_setup(aa_vertex *v1, aa_vertex *v2)
{
	float dx, dy;			// Deltas in X and Y
	float udx, udy;		// Positive version of deltas
	float one_du;			// 1.0 / udx or udy
	aa_setup_line line;

	if ( !aaline_inited )
		aaline_init_tables();


	dx = v1->x - v2->x;
	dy = v1->y - v2->y;

	if (dx < 0.0)	{
		udx = -dx;
	} else	{
		udx = dx;
	}

	if (dy < 0.0)	{
		udy = -dy;
	} else	{
		udy = dy;
	}

	if (udx > udy) {
		// X major line
		line.x_major = 1;
		line.negative = (dx < 0.0);
		line.us = FLOAT_TO_FIX_XY(v2->x);
		line.vs = FLOAT_TO_FIX_XY(v2->y);
		line.ue = FLOAT_TO_FIX_XY(v1->x);
		one_du = 1.0f / udx;
		line.dvdu = FLOAT_TO_FIX_XY(dy * one_du);
	} else {
		// Y major line
		line.x_major = 0;
		line.negative = (dy < 0.0);
		line.us = FLOAT_TO_FIX_XY(v2->y);
		line.vs = FLOAT_TO_FIX_XY(v2->x);
		line.ue = FLOAT_TO_FIX_XY(v1->y);
		one_du = 1.0f / udy;
		line.dvdu = FLOAT_TO_FIX_XY(dx * one_du);
	}

	// Convert colors to fixed-point
	//line.as = FLOAT_TO_FIX_RGB(v2->a);

	// Compute delta values for colors
	//line.dadu = FLOAT_TO_FIX_RGB((v1->a - v2->a) * one_du);

	// Now go draw it

	gr_lock();
	draw_line(&line);
	gr_unlock();
}


void gr8_aaline( vertex *v1, vertex *v2 )
{
	aa_vertex aa1, aa2;

	if ( !Current_alphacolor ) {
		gr_line(fl2i(v1->sx),fl2i(v1->sy),fl2i(v2->sx),fl2i(v2->sy));
		return;
	}

//	return;

	aa1.x = v1->sx;
	aa1.y = v1->sy;

	aa2.x = v2->sx;
	aa2.y = v2->sy;

	{
		int clipped = 0, swapped = 0;
		float a1, b1, a2, b2;
		a1 = (float)gr_screen.clip_left;
		b1 = (float)gr_screen.clip_top;
		a2 = (float)gr_screen.clip_right;
		b2 = (float)gr_screen.clip_bottom;

		FL_CLIPLINE(aa1.x,aa1.y,aa2.x,aa2.y,a1,b1,a2,b2,return,clipped=1,swapped=1);
	}

	aaline_setup( &aa1, &aa2 );
}
