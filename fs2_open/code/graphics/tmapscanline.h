/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/TmapScanline.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * Header file for tmapscanline.cpp.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 25    5/13/98 2:53p John
 * Made subspace effect work under software.  Had to add new inner loop to
 * tmapper.  Added glows to end of subspace effect.  Made subspace effect
 * levels use gamepalette-subspace palette.
 * 
 * 24    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 23    3/22/98 2:33p John
 * Took out fx_v/v_right.  Made fx_u etc get calculated in tmapper.
 * 
 * 22    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 21    12/15/97 11:32a John
 * New Laser Code
 * 
 * 20    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 19    11/30/97 3:57p John
 * Made fixed 32-bpp translucency.  Made BmpMan always map translucent
 * color into 255 even if you aren't supposed to remap and make it's
 * palette black.
 * 
 * 18    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 17    10/16/97 10:55a John
 * added tmapper to draw a monochrome alpha blended bitmap.
 * 
 * 16    3/14/97 3:55p John
 * Made tiled tmapper not always be zbuffered.
 * 
 * 15    3/13/97 10:32a John
 * Added code for tiled 256x256 textures in certain models.
 * 
 * 14    3/05/97 7:15p John
 * took out the old z stop tmapper used for briefing. 
 * 
 * 13    1/20/97 4:17p John
 * 
 * 12    1/06/97 2:44p John
 * Added in slow (but correct) zbuffering
 * 
 * 11    12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 10    11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 9     11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 8     11/05/96 4:05p John
 * Added roller.  Added code to draw a distant planet.  Made bm_load
 * return -1 if invalid bitmap.
 * 
 * 7     10/31/96 7:20p John
 * Added per,tiled tmapper.  Made models tile if they use 64x64 textures.
 * 
 * 6     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */


#ifndef _TMAPSCANLINE_H
#define _TMAPSCANLINE_H


typedef struct tmapper_vertex {
	float sx, sy, sw, u, v, b;
} tmapper_vertex;

typedef struct tmapper_data {
	// These are filled once for each texture map being drawn.
	// The inner loop cannot change any of these!!
	int		nv;						// number of vertices
	ubyte		*pixptr;
	bitmap	*bp;
	int		src_offset;
	uint		flags;
	float		FixedScale;				// constant for asm inner loop
	float		FixedScale8;			// constant for asm inner loop
	float		One;						// constant for asm inner loop

	// This are filled in by the outer loop before each scan line.
	int		loop_count;
	tmapper_vertex	l, r, dl, dr, deltas;
	int		lx, rx, y;
	float		clipped_left;			// how many pixels were clipped off the left in 2d.

	// These are used internally by the assembly texture mapper.
	fix		fx_l, fx_l_right, fx_dl_dx;
	fix		fx_u, fx_v, fx_du_dx, fx_dv_dx;
	float		fl_dudx_wide;
	float		fl_dvdx_wide;
	float		fl_dwdx_wide;
	uint		dest_row_data;
	int		num_big_steps;
	uint		uv_delta[2];
	float		FloatTemp;
	uint		Subdivisions;
	uint		WidthModLength;
	uint		BlendLookup;
	uint		FadeLookup;
	uint		DeltaU;
	uint		DeltaV;
	uint		DeltaUFrac, DeltaVFrac;
   uint		UFixed;
	uint		VFixed;
   ushort	FPUCW;
	ushort	OldFPUCW;
	int		InnerLooper;
	uint		pScreenBits;
	int		fx_w;
	int		fx_dwdx;

	uint		saved_esp;
	uint		lookup;

} tmapper_data;

extern tmapper_data Tmap;

extern void tmapscan_generic();
extern void tmapscan_generic8();
//extern void tmapscan_pnn();
extern void tmapscan_pln();
extern void tmapscan_lln();
extern void tmapscan_flat();
extern void tmapscan_nebula8();
extern void tmapscan_flat_z();

extern void tmapscan_flat8();
extern void tmapscan_lln8();
extern void tmapscan_lnt8();
extern void tmapscan_lnn8();
extern void tmapscan_lnt8();
extern void tmapscan_lln8_tiled();
extern void tmapscan_llt8_tiled();
extern void tmapscan_pln8();
extern void tmapscan_plt8();

extern void tmapscan_lnaa8();

extern void tmapscan_pln8_tiled();

extern void tmapscan_lnn8_tiled_256x256();
extern void tmapscan_pnn8_tiled_256x256_subspace();

extern void tmapscan_flat_gouraud();

extern void tmapscan_nebula8();

#endif
