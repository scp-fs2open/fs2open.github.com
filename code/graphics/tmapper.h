/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/TMAPPER.H $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Header file for Tmapper.h
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 3     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 17    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 16    4/09/98 7:58p John
 * Cleaned up tmapper code a bit.   Put NDEBUG around some ndebug stuff.
 * Took out XPARENT flag settings in all the alpha-blended texture stuff.
 * 
 * 15    4/09/98 4:38p John
 * Made non-darkening and transparent textures work under Glide.  Fixed
 * bug with Jim's computer not drawing any bitmaps.
 * 
 * 14    3/23/98 5:00p John
 * Improved missile trails.  Made smooth alpha under hardware.  Made end
 * taper.  Made trail touch weapon.
 * 
 * 13    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 12    10/15/97 5:08p John
 * added flag for alpha tmap
 * 
 * 11    3/10/97 5:20p John
 * Differentiated between Gouraud and Flat shading.  Since we only do flat
 * shading as of now, we don't need to interpolate L in the outer loop.
 * This should save a few percent.
 * 
 * 10    3/05/97 7:15p John
 * took out the old z stop tmapper used for briefing. 
 * 
 * 9     12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 8     11/07/96 2:17p John
 * Took out the OldTmapper stuff.
 * 
 * 7     11/05/96 4:05p John
 * Added roller.  Added code to draw a distant planet.  Made bm_load
 * return -1 if invalid bitmap.
 * 
 * 6     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifndef _TMAPPER_H
#define _TMAPPER_H

// call this to reinit the scanline function pointers.
extern void tmapper_setup();

// Used to tell the tmapper what the current lighting values are
// if the TMAP_FLAG_RAMP or TMAP_FLAG_RGB are set and the TMAP_FLAG_GOURAUD 
// isn't set.   
void tmapper_set_light(vertex *v, uint flags);

// DO NOT CALL grx_tmapper DIRECTLY!!!! Only use the 
// gr_tmapper equivalent!!!!
extern void grx_tmapper( int nv, vertex * verts[], uint flags );

#define TMAP_MAX_VERTS	25		// Max number of vertices per polygon

// Flags to pass to g3_draw_??? routines
#define TMAP_FLAG_TEXTURED			(1<<0)	// Uses texturing (Interpolate uv's)
#define TMAP_FLAG_CORRECT			(1<<1)	// Perspective correct (Interpolate sw)
#define TMAP_FLAG_RAMP				(1<<2)	// Use RAMP lighting (interpolate L)
#define TMAP_FLAG_RGB				(1<<3)	// Use RGB lighting (interpolate RGB)
#define TMAP_FLAG_GOURAUD			(1<<4)	// Lighting values differ on each vertex. 
														// If this is not set, then the texture mapper will use
														// the lighting parameters in each vertex, otherwise it
														// will use the ones specified in tmapper_set_??
#define TMAP_FLAG_XPARENT			(1<<5)	// texture could have transparency
#define TMAP_FLAG_TILED				(1<<6)	// This means uv's can be > 1.0
#define TMAP_FLAG_NEBULA			(1<<7)	// Must be used with RAMP and GOURAUD.  Means l 0-1 is 0-31 palette entries


#define TMAP_HIGHEST_FLAG_BIT		7			// The highest bit used in the TMAP_FLAGS
#define TMAP_MAX_SCANLINES			(1<<(TMAP_HIGHEST_FLAG_BIT+1))

// Add any entries that don't work for software under here:
// Make sure to disable them at top of grx_tmapper
#define TMAP_FLAG_ALPHA				(1<<8)	// Has an alpha component
#define TMAP_FLAG_NONDARKENING	(1<<9)	// RGB=255,255,255 doesn't darken

// flags for full nebula effect
#define TMAP_FLAG_PIXEL_FOG		(1<<10)	// fog the polygon based upon the average pixel colors of the backbuffer behind it

// bitmap section
#define TMAP_FLAG_BITMAP_SECTION	(1<<11)

#endif
