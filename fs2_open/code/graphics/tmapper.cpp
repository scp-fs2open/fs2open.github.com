/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Tmapper.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Routines to draw a texture map.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/05/08 02:36:01  mharris
 * porting
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/06/98 3:08p Dave
 * Fixed grx_tmapper to handle pixel fog flag. First run fog support for
 * D3D.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 78    5/13/98 2:53p John
 * Made subspace effect work under software.  Had to add new inner loop to
 * tmapper.  Added glows to end of subspace effect.  Made subspace effect
 * levels use gamepalette-subspace palette.
 * 
 * 77    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 76    4/09/98 7:58p John
 * Cleaned up tmapper code a bit.   Put NDEBUG around some ndebug stuff.
 * Took out XPARENT flag settings in all the alpha-blended texture stuff.
 * 
 * 75    4/09/98 7:16p John
 * Fixed bug causing software to not render
 * 
 * 74    3/27/98 8:34p Mike
 * Minor optimization.
 * 
 * 73    3/25/98 8:08p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 72    3/23/98 5:00p John
 * Improved missile trails.  Made smooth alpha under hardware.  Made end
 * taper.  Made trail touch weapon.
 * 
 * 71    3/22/98 2:33p John
 * Took out fx_v/v_right.  Made fx_u etc get calculated in tmapper.
 * 
 * 70    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 69    2/10/98 5:34p John
 * 
 * 68    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 67    1/29/98 8:18a John
 * Put in some commented out hooks for RGB lighting
 * 
 * 66    1/28/98 1:27p John
 * Really fixed bug with exception on mov al, [esi]
 * 
 * 65    1/28/98 1:22p John
 * Fixed bug with unitialized dwdx_wide.
 * 
 * 64    1/27/98 5:13p John
 * Moved all float to int conversions out of inner loops and into outer.
 * Made outer loop use FISTP instead of ftol, saved about 10%.
 * 
 * 63    1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 62    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 61    12/15/97 11:32a John
 * New Laser Code
 * 
 * 60    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 59    11/30/97 4:40p John
 * made 32-bpp tiled tmapper call scanline
 * 
 * 58    11/30/97 3:57p John
 * Made fixed 32-bpp translucency.  Made BmpMan always map translucent
 * color into 255 even if you aren't supposed to remap and make it's
 * palette black.
 * 
 * 57    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 56    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 55    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 54    11/14/97 12:30p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 53    11/06/97 11:18a John
 * added 16-bpp gouraud flat shader
 * 
 * 52    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 51    10/16/97 10:55a John
 * added tmapper to draw a monochrome alpha blended bitmap.
 * 
 * 50    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 49    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 48    9/30/97 2:30p John
 * test code for texture fading
 * 
 * 47    9/24/97 10:37a John
 * made tmapper not trash uv values anymore.
 * 
 * 46    9/09/97 11:01a Sandeep
 * fixed warning level 4 bugs
 * 
 * 45    7/11/97 11:54a John
 * added rotated 3d bitmaps.
 * 
 * 44    6/18/97 5:02p John
 * fixed bug with 32x32 and 16x16 tmapper
 * 
 * 43    6/12/97 5:04p John
 * Initial rev of Glide support
 * 
 * 42    6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 41    6/02/97 11:45a John
 * fixed bugs with 64x64 and 128x128 tmappers.
 * 
 * 40    6/01/97 3:41p John
 * made non-tilable textures on tilable models bash uvs to 0-1.
 * 
 * 39    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 38    5/07/97 4:14p John
 * Reenabled calls to gr_start/end_frame.
 * 
 * 37    4/21/97 10:06a John
 * Got capital ships working again.
 * 
 * 36    4/17/97 6:06p John
 * New code/data for v19 of BSPGEN with smoothing and zbuffer
 * optimizations.
 * 
 * 35    4/08/97 5:18p John
 * First rev of decent (dynamic, correct) lighting in FreeSpace.
 * 
 * 34    3/18/97 9:42a John
 * 
 * 33    3/15/97 2:44p John
 * got scanline sorting method working.  Bummer it is slower than zbuffer!
 * 
 * 32    3/14/97 3:55p John
 * Made tiled tmapper not always be zbuffered.
 * 
 * 31    3/13/97 10:32a John
 * Added code for tiled 256x256 textures in certain models.
 * 
 * 30    3/12/97 2:51p John
 * Added some test code for tmapper.  
 * 
 * 29    3/12/97 9:25a John
 * fixed a bug with zbuffering.  Reenabled it by default.
 * 
 * 28    3/11/97 4:36p John
 * added zbuffering to textest.  Made zbuffered tmapper a bit faster by
 * rearranging some instructions.
 * 
 * 27    3/10/97 5:20p John
 * Differentiated between Gouraud and Flat shading.  Since we only do flat
 * shading as of now, we don't need to interpolate L in the outer loop.
 * This should save a few percent.
 * 
 * 26    3/10/97 2:24p John
 * added some commets about precompiled inner loop
 * 
 * 25    3/05/97 7:15p John
 * took out the old z stop tmapper used for briefing. 
 * 
 * 24    1/20/97 4:17p John
 * 
 * 23    1/06/97 2:44p John
 * Added in slow (but correct) zbuffering
 * 
 * 22    12/30/96 3:46p John
 * 
 * 21    12/23/96 10:56a John
 * Totally restructured the POF stuff to support multiple 
 * detail levels in one POF file.
 *  
 * 
 * 20    12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 19    11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 18    11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 17    11/07/96 3:08p John
 * Inlined more Tmapper functions in preparation for cleaning up the Tmap1
 * interface to the assembly.
 * 
 * 16    11/07/96 2:17p John
 * Took out the OldTmapper stuff.
 * 
 * 15    11/07/96 12:04p John
 * Sped up outer loop by 35% by inlining the incrementing of the variables
 * for each scanline step and inlined the calculation for deltas at the
 * start of each scanline.
 * 
 * 14    11/06/96 2:33p John
 * Added more asserts for checking that non-tiled UV's are between 0 and
 * 1.0.    Put code in the model_init code that checks for polys that have
 * a vertex duplicated and throws them out.
 * 
 * 13    11/05/96 4:05p John
 * Added roller.  Added code to draw a distant planet.  Made bm_load
 * return -1 if invalid bitmap.
 * 
 * 12    10/31/96 7:20p John
 * Added per,tiled tmapper.  Made models tile if they use 64x64 textures.
 * 
 * 11    10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "2d.h"
#include "grinternal.h"
#include "3d.h"
#include "tmapper.h"
#include "bmpman.h"
#include "tmapscanline.h"
#include "key.h"
#include "floating.h"
#include "palman.h"

typedef void (* pscanline)();

pscanline tmap_scanline;

int Tmap_screen_flags = -1;
int Tmap_npolys=0;
int Tmap_nverts=0;
int Tmap_nscanlines=0;
int Tmap_npixels=0;
tmapper_data Tmap;
int Tmap_show_layers=0;

typedef struct tmap_scan_desc {
	uint			flags;
	pscanline	scan_func;
} tmap_scan_desc;

// Convert from a 0-255 byte to a 0-1.0 float.
float Light_table[256];


//====================== 8-BPP SCANLINES ========================
tmap_scan_desc tmap_scanlines8[] = {
	{ 0, tmapscan_flat8 },
	{ TMAP_FLAG_TEXTURED, tmapscan_lnn8 },
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_XPARENT, tmapscan_lnt8 },
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP, tmapscan_lln8 },
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP|TMAP_FLAG_CORRECT, tmapscan_pln8 },
	
	{ TMAP_FLAG_RAMP|TMAP_FLAG_GOURAUD, tmapscan_flat_gouraud },

	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP|TMAP_FLAG_GOURAUD, tmapscan_lln8 },
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP|TMAP_FLAG_GOURAUD|TMAP_FLAG_CORRECT, tmapscan_pln8 },

	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP|TMAP_FLAG_CORRECT|TMAP_FLAG_TILED, tmapscan_pln8_tiled },
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_RAMP|TMAP_FLAG_CORRECT|TMAP_FLAG_GOURAUD|TMAP_FLAG_TILED, tmapscan_pln8_tiled },

	{ TMAP_FLAG_RAMP|TMAP_FLAG_GOURAUD|TMAP_FLAG_NEBULA, tmapscan_nebula8 },

//	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED, tmapscan_lnn8_tiled_256x256 },
	// Totally non-general specific inner loop for subspace effect
	{ TMAP_FLAG_TEXTURED|TMAP_FLAG_CORRECT|TMAP_FLAG_TILED, tmapscan_pnn8_tiled_256x256_subspace },
	

	{ 0, NULL },	// Dummy element to mark the end of fast scanlines.
};


pscanline tmap_scanline_table[TMAP_MAX_SCANLINES];


// -------------------------------------------------------------------------------------
// This sets up the tmapper at the start of a given frame, so everything
// can can be pre-calculated should be calculated in here.
// This just fills in the tmap_scanline_table for the
// appropriate scan lines.

void tmapper_setup()
{
	int i;
	tmap_scan_desc * func_table = NULL;

	Tmap_screen_flags = gr_screen.mode;

	// Some constants for the inner loop
	Tmap.FixedScale = 65536.0f;
	Tmap.FixedScale8 =	2048.0f;	//8192.0f;	// 2^16 / 8
	Tmap.One = 1.0f;

	// Set tmap_scanline to not call a function
	for (i=0; i<TMAP_MAX_SCANLINES; i++ )	{
		tmap_scanline_table[i] = NULL;
	}

	func_table = tmap_scanlines8;

	while(func_table->scan_func != NULL)	{
		tmap_scanline_table[func_table->flags] = func_table->scan_func;
		func_table++;
	}

	for (i=0; i<256; i++ )	{
		Light_table[i] = i2fl(i)/255.0f;
	}


}

// Sets up flat-shaded lighting
void tmapper_set_light(vertex *v, uint flags)
{
	if ( flags & TMAP_FLAG_GOURAUD ) return;

	if ( (flags & (TMAP_FLAG_RAMP|TMAP_FLAG_RGB))==(TMAP_FLAG_RAMP|TMAP_FLAG_RGB))	{
		Int3();		// You're doing RGB and RAMP lighting!!!
	}

	if ( flags & TMAP_FLAG_RAMP )	{
		Tmap.l.b = Tmap.r.b = i2fl(v->b)/255.0f;
		Tmap.deltas.b = 0.0f;
	}
}

void tmapper_show_layers()
{
	int i;
	ubyte * ptr = (ubyte *)Tmap.dest_row_data;

	for (i=0; i<Tmap.loop_count; i++, ptr++ )	{
		*ptr = (unsigned char)(*ptr + 1);
	}

}

/*
void tmap_scan_generic()
{
	int ui,vi,i;
	ubyte * dptr,c;
	float l, dl;
	float u, v, w, du, dv, dw;
	
	dptr = (ubyte *)Tmap.dest_row_data;

	Tmap.fx_w = fl2i(Tmap.l.sw * GR_Z_RANGE)+gr_zoffset;
	Tmap.fx_dwdx = fl2i(Tmap.deltas.sw * GR_Z_RANGE);

	l = Tmap.l.b;
	dl = Tmap.deltas.b;

	u = Tmap.l.u;
	v = Tmap.l.v;
	w = Tmap.l.sw;
	du = Tmap.deltas.u;
	dv = Tmap.deltas.v;
	dw = Tmap.deltas.sw;
	
	for (i=0; i<Tmap.loop_count; i++ )	{
		int tmp = (uint)dptr-Tmap.pScreenBits;
		if ( Tmap.fx_w > (int)gr_zbuffer[tmp] )	{
			gr_zbuffer[tmp] = Tmap.fx_w;

			ui = fl2i( u / w ) % Tmap.bp->w;
			vi = fl2i( v / w ) % Tmap.bp->h;

			c = Tmap.pixptr[vi*Tmap.bp->w+ui];
			*dptr = gr_fade_table[fl2i(l*31)*256+c];

		}
		Tmap.fx_w += Tmap.fx_dwdx;
		l+=dl;
		u+=du;
		v+=dv;
		w+=dw;
		dptr++;
	}
}
*/


// Same as ftol except that it might round up or down, 
// unlike C's ftol, which must always round down.  
// But, in the tmapper, we don't care, since this is
// just for Z and L.
static inline int tmap_ftol(float f)
{
	  int x;

#if defined(WIN32) && defined(MSVC)
	_asm fld f
	_asm fistp x
#elif defined(__GNUC__) && defined(__i386__)
	 asm ( "fistl %0" : "=m" (x) : "t" (f) );
#else
#error unknown processor/compiler
#endif

	return x; 
}

/*
#define tmap_ftol(f) ((int)(f))

__ftol:
004569c0   push      ebp
004569c1   mov       ebp,esp
004569c3   add       esp,fffffff4
004569c6   wait
004569c7   fnstcw    [ebp-02]
004569ca   wait
004569cb   mov       ax,word ptr [ebp-02]
004569cf   or        ah,0c
004569d2   mov       word ptr [ebp-04],ax
004569d6   fldcw     [ebp-04]
004569d9   fistp     qword ptr [ebp-0c]
004569dc   fldcw     [ebp-02]
004569df   mov       eax,dword ptr [ebp-0c]
004569e2   mov       edx,dword ptr [ebp-08]
004569e5   leave
004569e6   ret
*/

#ifdef RGB_LIGHTING

extern ubyte gr_palette[768];

uint last_code = 0xf;
void change_fade_table(uint code)
{
	int i,l;

	if ( last_code == code ) return;
	last_code = code;
	
	int r1=0, g1=0, b1=0;

	for (i=0; i<256; i++ )	{
		int r, g, b;
		int ur, ug, ub;
		r = gr_palette[i*3+0];
		g = gr_palette[i*3+1];
		b = gr_palette[i*3+2];

		if ( (r == 255) && (g == 255) && (b == 255) )	{
			// Make pure white not fade
			for (l=0; l<32; l++ )	{
				gr_fade_table[((l+1)*256)+i] = (unsigned char)i;
			}
		} else {
			for (l=24; l<32; l++ )	{

				int x,y;
				int gi, gr, gg, gb;
	
				gi = (r+g+b)/3;

				//gr = r*2;
				//gg = g*2;
				//gb = b*2;

				if ( code & 4 ) gr = gi*2; else gr = r;
				if ( code & 2 ) gg = gi*2; else gg = g;
				if ( code & 1 ) gb = gi*2; else gb = b;

//				gr = r1;
//				gg = g1;
//				gb = b1;	//gi*2;
		
				x = l-24;			// x goes from 0 to 7
				y = 31-l;			// y goes from 7 to 0

				ur = ((gr*x)+(r*y))/7; if ( ur > 255 ) ur = 255;
				ug = ((gg*x)+(g*y))/7; if ( ug > 255 ) ug = 255;
				ub = ((gb*x)+(b*y))/7; if ( ub > 255 ) ub = 255;

				gr_fade_table[((l+1)*256)+i] = (unsigned char)palette_find( ur, ug, ub );

			}
		}
		gr_fade_table[ (0*256)+i ] = gr_fade_table[ (1*256)+i ];
		gr_fade_table[ (33*256)+i ] = gr_fade_table[ (32*256)+i ];
	}

	// Mirror the fade table
	for (i=0; i<34; i++ )	{
		for ( l = 0; l < 256; l++ )	{
			gr_fade_table[ ((67-i)*256)+l ] = gr_fade_table[ (i*256)+l ];
		}
	}

}
#endif


void grx_tmapper( int nverts, vertex **verts, uint flags )	
{
	int i, y, li, ri, ly, ry, top, rem;
	float ymin;
	int next_break;
	float ulist[TMAP_MAX_VERTS], vlist[TMAP_MAX_VERTS], llist[TMAP_MAX_VERTS];

	flags &= (~TMAP_FLAG_ALPHA);
	flags &= (~TMAP_FLAG_NONDARKENING);
	flags &= (~TMAP_FLAG_PIXEL_FOG);

	// Check for invalid flags
#ifndef NDEBUG
	if ( (flags & (TMAP_FLAG_RAMP|TMAP_FLAG_RGB))==(TMAP_FLAG_RAMP|TMAP_FLAG_RGB))	{
		Int3();		// You're doing RGB and RAMP lighting!!!
	}

	if ( flags & TMAP_FLAG_RGB )	{
		Int3();		// RGB not supported!
	}

	if ( (flags & TMAP_FLAG_GOURAUD) && (!(flags & TMAP_FLAG_RAMP)) )	{
		Int3();		// Ramp mode required for gouraud!
	}

	if ( gr_screen.bits_per_pixel != 8 )	{
		Int3();			// Only 8-bpp tmapper supported
	}

	Tmap_npolys++;
	Tmap_nverts += nverts;

	Assert(nverts <= TMAP_MAX_VERTS );

#endif
	
	if ( flags & (TMAP_FLAG_RAMP|TMAP_FLAG_GOURAUD) )	{
		for (i=0; i<nverts; i++ )	{
			llist[i] = Light_table[verts[i]->b];
		}
	}

	if ( Tmap_screen_flags != gr_screen.mode )	{
		tmapper_setup();
	}

	tmap_scanline = tmap_scanline_table[flags];
//	tmap_scanline = tmap_scan_generic;

#ifndef NDEBUG
	Assert( tmap_scanline != NULL );

	if (Tmap_show_layers)
		tmap_scanline = tmapper_show_layers;
#endif

	if ( tmap_scanline == NULL ) return;

	Tmap.FadeLookup = (uint)palette_get_fade_table();
	Tmap.BlendLookup = (uint)palette_get_blend_table(gr_screen.current_alpha);

	if ( flags & TMAP_FLAG_TEXTURED )	{

		Tmap.bp  = bm_lock( gr_screen.current_bitmap, 8, 0 );

		int was_tiled = 0, can_tile = 0;
		if ( flags & TMAP_FLAG_TILED )	{
			if ( (Tmap.bp->w==16) && (Tmap.bp->h==16) ) can_tile = 1;
			if ( (Tmap.bp->w==32) && (Tmap.bp->h==32) ) can_tile = 1;
			if ( (Tmap.bp->w==64) && (Tmap.bp->h==64) ) can_tile = 1;
			if ( (Tmap.bp->w==128) && (Tmap.bp->h==128) ) can_tile = 1;
			if ( (Tmap.bp->w==256) && (Tmap.bp->h==256) ) can_tile = 1;
		
			if ( !can_tile )	{
				was_tiled = 1;
				flags &= (~TMAP_FLAG_TILED);
			}
		}

		float max_u = i2fl(Tmap.bp->w) - 0.5f;
		float max_v = i2fl(Tmap.bp->h) - 0.5f;

		for (i=0; i<nverts; i++ )	{
			ulist[i] = verts[i]->u * Tmap.bp->w;
			vlist[i] = verts[i]->v * Tmap.bp->h;

			if ( !(flags & TMAP_FLAG_TILED) )	{
				if ( ulist[i] < 1.0f ) ulist[i] = 1.0f;
				if ( vlist[i] < 1.0f ) vlist[i] = 1.0f;
				if ( ulist[i] > max_u ) ulist[i] = max_u;
				if ( vlist[i] > max_v ) vlist[i] = max_v;
			}
	
			// Multiply all u,v's by sw for perspective correction
			if ( flags & TMAP_FLAG_CORRECT )	{
				ulist[i] *= verts[i]->sw;
				vlist[i] *= verts[i]->sw;
			}
		}

		Tmap.pixptr = (unsigned char *)Tmap.bp->data;
		Tmap.src_offset = Tmap.bp->rowsize;
	}
	
	// Find the topmost vertex
	//top = -1;			// Initialize to dummy value to avoid compiler warning
	//ymin = 0.0f;		// Initialize to dummy value to avoid compiler warning
	//	Instead of initializing to avoid compiler warnings, set to first value outside loop and remove (i==0)
	//	comparison, which otherwise happens nverts times.  MK, 3/20/98 (was tracing code figuring out my shield effect bug...)
	ymin = verts[0]->sy;
	top = 0;
	for (i=1; i<nverts; i++ ) {
		if (verts[i]->sy < ymin) {
			ymin = verts[i]->sy;
			top = i;
		}
	}	

	li = ri = top;
	rem = nverts;
	y = fl_round_2048(ymin);		//(int)floor(ymin + 0.5);
	ly = ry = y - 1;

	gr_lock();
	Tmap.pScreenBits = (uint)gr_screen.offscreen_buffer_base;

	while( rem > 0 )	{
		while ( ly<=y && rem>0 )	{	// Advance left edge?
			float dy, frac, recip;
			rem--;
			i = li-1;	
			if ( i<0 ) i = nverts-1;
			ly = fl_round_2048(verts[i]->sy);	//(int)floor(verts[i]->sy+0.5);

			dy = verts[i]->sy - verts[li]->sy;
			if ( dy == 0.0f ) dy = 1.0f;

			frac = y + 0.5f - verts[li]->sy;
			recip = 1.0f / dy;

			Tmap.dl.sx = (verts[i]->sx - verts[li]->sx)*recip; 
			Tmap.l.sx = verts[li]->sx + Tmap.dl.sx*frac;

			if ( flags & TMAP_FLAG_TEXTURED )	{
				Tmap.dl.u = (ulist[i] - ulist[li])*recip; 
				Tmap.l.u = ulist[li] + Tmap.dl.u*frac;
				Tmap.dl.v = (vlist[i] - vlist[li])*recip; 
				Tmap.l.v = vlist[li] + Tmap.dl.v*frac;
			}

			if ( (flags & TMAP_FLAG_CORRECT) || gr_zbuffering  )	{
				Tmap.dl.sw = (verts[i]->sw - verts[li]->sw)*recip;
				Tmap.l.sw = verts[li]->sw + Tmap.dl.sw*frac;
			}

			if ( flags & TMAP_FLAG_GOURAUD )	{
				if ( flags & TMAP_FLAG_RAMP )	{
					Tmap.dl.b = (llist[i] - llist[li])*recip;
					Tmap.l.b = llist[li]  + Tmap.dl.b*frac;
				}
			}

			li = i;
		}
		while ( ry<=y && rem>0 )	{	// Advance right edge?
			float dy, frac, recip;
			rem--;
			i = ri+1;
			if ( i>=nverts ) i = 0;
			ry = fl_round_2048(verts[i]->sy);	//(int)floor(verts[i]->sy+0.5);

			dy = verts[i]->sy - verts[ri]->sy;
			if ( dy == 0.0f ) dy = 1.0f;

			frac = y + 0.5f - verts[ri]->sy;
			recip = 1.0f / dy;

			Tmap.dr.sx = (verts[i]->sx - verts[ri]->sx)*recip; 
			Tmap.r.sx = verts[ri]->sx + Tmap.dr.sx*frac;

			if ( flags & TMAP_FLAG_TEXTURED )	{
				Tmap.dr.u = (ulist[i] - ulist[ri])*recip;
				Tmap.r.u = ulist[ri] + Tmap.dr.u*frac;
				Tmap.dr.v = (vlist[i] - vlist[ri])*recip;
				Tmap.r.v = vlist[ri] + Tmap.dr.v*frac;
			}

			if ( (flags & TMAP_FLAG_CORRECT) || gr_zbuffering  )	{
				Tmap.dr.sw = (verts[i]->sw - verts[ri]->sw)*recip;
				Tmap.r.sw = verts[ri]->sw + Tmap.dr.sw*frac;
			}

			if ( flags & TMAP_FLAG_GOURAUD )	{
				if ( flags & TMAP_FLAG_RAMP )	{
					Tmap.dr.b = (llist[i] - llist[ri])*recip;
					Tmap.r.b = llist[ri] + Tmap.dr.b*frac;
				}
			}

			ri = i;
		}

		if ( ly < ry )	
			next_break = ly;
		else
			next_break = ry;

		for ( ; y<next_break; y++ )	{
			if ( (y >= gr_screen.clip_top) && ( y<=gr_screen.clip_bottom) )	{
				int lx, rx;


				lx = fl_round_2048(Tmap.l.sx);
				if ( lx < gr_screen.clip_left ) {	
					Tmap.clipped_left = i2fl(gr_screen.clip_left) - Tmap.l.sx;
					lx = gr_screen.clip_left;
				} else {
					Tmap.clipped_left = 0.0f;
				}
				rx = fl_round_2048(Tmap.r.sx-1.0f);
	
				if ( rx > gr_screen.clip_right ) rx = gr_screen.clip_right;
				if ( lx <= rx ) {
					float dx, recip;	//frac;

					dx = Tmap.r.sx - Tmap.l.sx;
					if ( dx == 0.0f ) dx = 1.0f;

					//frac = lx + 0.5f - Tmap.l.sx;
					recip = 1.0f / dx;

					Tmap.y = y;
					Tmap.rx = rx;
					Tmap.lx = lx;
					Tmap.loop_count = rx - lx + 1;
					#ifndef NDEBUG
						Tmap_npixels += Tmap.loop_count;
						Tmap_nscanlines++;
					#endif
					
					if ( (flags & TMAP_FLAG_CORRECT) || gr_zbuffering  )	{
						Tmap.deltas.sw = (Tmap.r.sw - Tmap.l.sw)*recip;
						Tmap.fl_dwdx_wide = Tmap.deltas.sw*32.0f;
					}
	
					if ( flags & TMAP_FLAG_TEXTURED )	{
						Tmap.deltas.u = (Tmap.r.u - Tmap.l.u)*recip;
						Tmap.deltas.v = (Tmap.r.v - Tmap.l.v)*recip;

						if ( flags & TMAP_FLAG_CORRECT )	{
							Tmap.fl_dudx_wide = Tmap.deltas.u*32.0f;
							Tmap.fl_dvdx_wide = Tmap.deltas.v*32.0f;
						} else {
							Tmap.fx_u = tmap_ftol((Tmap.l.u+Tmap.clipped_left*Tmap.deltas.u)*65536.0f);
							Tmap.fx_v = tmap_ftol((Tmap.l.v+Tmap.clipped_left*Tmap.deltas.v)*65536.0f);
							Tmap.fx_du_dx = tmap_ftol(Tmap.deltas.u*65536.0f);
							Tmap.fx_dv_dx = tmap_ftol(Tmap.deltas.v*65536.0f);
						}
					}

					if ( flags & TMAP_FLAG_GOURAUD )	{
						if ( flags & TMAP_FLAG_RAMP )	{
							Tmap.deltas.b = (Tmap.r.b - Tmap.l.b)*recip;

							Tmap.fx_l = tmap_ftol(Tmap.l.b*32.0f*65536.0f); 
							Tmap.fx_l_right = tmap_ftol(Tmap.r.b*32.0f*65536.0f); 
							Tmap.fx_dl_dx = tmap_ftol(Tmap.deltas.b*32.0f*65536.0f);

							if ( Tmap.fx_dl_dx < 0 )	{
								Tmap.fx_dl_dx = -Tmap.fx_dl_dx;
								Tmap.fx_l = (67*F1_0)-Tmap.fx_l;
								Tmap.fx_l_right = (67*F1_0)-Tmap.fx_l_right;
						//		Assert( Tmap.fx_l > 31*F1_0 );
						//		Assert( Tmap.fx_l < 66*F1_0 );
						//		Assert( Tmap.fx_dl_dx >= 0 );
						//		Assert( Tmap.fx_dl_dx < 31*F1_0 );
							}
						}
					}

					if ( gr_zbuffering )	{
						Tmap.fx_w = tmap_ftol(Tmap.l.sw * GR_Z_RANGE)+gr_zoffset;
						Tmap.fx_dwdx = tmap_ftol(Tmap.deltas.sw * GR_Z_RANGE);
					}

					Tmap.dest_row_data = GR_SCREEN_PTR_SIZE(gr_screen.bytes_per_pixel,Tmap.lx,Tmap.y);
	
					(*tmap_scanline)();

				} 

			}

			Tmap.l.sx += Tmap.dl.sx;
			Tmap.r.sx += Tmap.dr.sx;

			if ( flags & TMAP_FLAG_TEXTURED )	{
				Tmap.l.u += Tmap.dl.u;
				Tmap.l.v += Tmap.dl.v;

				Tmap.r.u += Tmap.dr.u;
				Tmap.r.v += Tmap.dr.v;
			}

			if ( (flags & TMAP_FLAG_CORRECT) || gr_zbuffering  )	{
				Tmap.l.sw += Tmap.dl.sw;
				Tmap.r.sw += Tmap.dr.sw;
			}

			if ( flags & TMAP_FLAG_GOURAUD )	{
				if ( flags & TMAP_FLAG_RAMP )	{
					Tmap.l.b += Tmap.dl.b;
					Tmap.r.b += Tmap.dr.b;
				}
			}
		}
	}

	gr_unlock();

	if ( flags & TMAP_FLAG_TEXTURED )	{
		bm_unlock(gr_screen.current_bitmap);
	}


}



