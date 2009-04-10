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
 * $Revision: 2.16.2.1 $
 * $Date: 2006-10-06 09:52:05 $
 * $Author: taylor $
 *
 * Header file for Tmapper.h
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.16  2006/05/27 17:07:48  taylor
 * remove grd3dparticle.* and grd3dbatch.*, they are obsolete
 * allow us to build without D3D support under Windows (just define NO_DIRECT3D)
 * clean up TMAP flags
 * fix a couple of minor OpenGL state change issues with spec and env map rendering
 * make sure we build again for OS X (OGL extension functions work a little different there)
 * render targets always need to be power-of-2 to avoid incomplete buffer issues in the code
 * when we disable culling in opengl_3dunlit be sure that we re-enable it on exit of function
 * re-fix screenshots
 * add true alpha blending support (with cmdline for now since the artwork has the catch up)
 * draw lines with float positioning, to be more accurate with resizing on non-standard resolutions
 * don't load cubemaps from file for D3D, not sure how to do it anyway
 * update geometry batcher code, memory fixes, dynamic stuff, basic fixage, etc.
 *
 * Revision 2.15  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.14  2005/04/24 12:56:43  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 2.13  2005/03/10 08:00:05  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.12  2005/03/09 03:23:31  bobboau
 * added a new interface render funtion
 *
 * Revision 2.11  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.10  2005/03/03 02:39:14  bobboau
 * added trilist suport to the poly rendering functions
 * and a gr_bitmap_list function that uses it
 *
 * Revision 2.9  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.8  2004/03/21 09:41:54  randomtiger
 * Fixed a bug that was causing windowed movie playback and a crash.
 * Added some batching redirection.
 *
 * Revision 2.7  2004/03/05 09:02:01  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.6  2004/02/16 11:47:33  randomtiger
 * Removed a lot of files that we don't need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.5  2004/02/15 06:02:32  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.4  2003/11/02 05:50:08  bobboau
 * modified trails to render with tristrips now rather than with stinky old trifans,
 * MUCH faster now, at least one order of magnatude.
 *
 * Revision 2.3  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.2  2003/10/16 00:17:14  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
 * Revision 2.1  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.0.2.1  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
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

#include "globalincs/pstypes.h"

/*
struct vertex;

// call this to reinit the scanline function pointers.
extern void tmapper_setup();

// Used to tell the tmapper what the current lighting values are
// if the TMAP_FLAG_RAMP or TMAP_FLAG_RGB are set and the TMAP_FLAG_GOURAUD 
// isn't set.   
void tmapper_set_light(vertex *v, uint flags);

// DO NOT CALL grx_tmapper DIRECTLY!!!! Only use the 
// gr_tmapper equivalent!!!!
extern void grx_tmapper( int nv, vertex * verts[], uint flags );
*/

#define TMAP_MAX_VERTS	25		// Max number of vertices per polygon

// Flags to pass to g3_draw_??? routines
#define TMAP_FLAG_TEXTURED			(1<<0)	// Uses texturing (Interpolate uv's)
#define TMAP_FLAG_CORRECT			(1<<1)	// Perspective correct (Interpolate sw)
#define TMAP_FLAG_RAMP				(1<<2)	// Use RAMP lighting (interpolate L)
#define TMAP_FLAG_RGB				(1<<3)	// Use RGB lighting (interpolate RGB)
#define TMAP_FLAG_GOURAUD			(1<<4)	// Lighting values differ on each vertex. 
											//   If this is not set, then the texture mapper will use
											//   the lighting parameters in each vertex, otherwise it
											//   will use the ones specified in tmapper_set_??
#define TMAP_FLAG_XPARENT			(1<<5)	// texture could have transparency
#define TMAP_FLAG_TILED				(1<<6)	// This means uv's can be > 1.0
#define TMAP_FLAG_NEBULA			(1<<7)	// Must be used with RAMP and GOURAUD.  Means l 0-1 is 0-31 palette entries

//#define TMAP_HIGHEST_FLAG_BIT		7		// The highest bit used in the TMAP_FLAGS
//#define TMAP_MAX_SCANLINES		(1<<(TMAP_HIGHEST_FLAG_BIT+1))

// Add any entries that don't work for software under here:
// Make sure to disable them at top of grx_tmapper
#define TMAP_FLAG_ALPHA				(1<<8)	// Has an alpha component
#define TMAP_FLAG_NONDARKENING		(1<<9)	// RGB=255,255,255 doesn't darken

// Interface specific stuff (for separate filtering, sizing, etc.), replaces old TMAP_FLAG_BITMAP_SECTION 
#define TMAP_FLAG_INTERFACE			(1<<10)

// flags for full nebula effect
#define TMAP_FLAG_PIXEL_FOG			(1<<11)	// fog the polygon based upon the average pixel colors of the backbuffer behind it

// RT Flags added to determine whats being drawn for HT&L
#define TMAP_HTL_3D_UNLIT			(1<<12)
#define TMAP_HTL_2D					(1<<13)

//tristrips, for trails mostly, might find other uses eventualy
#define TMAP_FLAG_TRISTRIP			(1<<14)
#define TMAP_FLAG_TRILIST			(1<<15)

#define TMAP_ADDRESS_WRAP			1
#define TMAP_ADDRESS_MIRROR			2
#define TMAP_ADDRESS_CLAMP			3

//WMC - moved this here so it'd be in 2d.h and 3d.h
//bitmap_2d_list, 
//x and y: the 2d position of the upper left hand corner
//w and h: the hidth and hight of the bitmap (some functions 
//will overide these, others will only overide if givein an invalid size like 0 or -1)
struct bitmap_2d_list{
	bitmap_2d_list(int X=0, int Y=0, int W=-1, int H=-1):x(X),y(Y),w(W),h(H){}
	int x;
	int y;
	int w;
	int h;
};

//texture_rect
//defines a rectangular reagon within a texture
//similar to the above structure only all values are relitive 
//from 0,0 in the upper left to 1,1 in the lowwer right
//out of range values are valid
struct texture_rect_list{
	texture_rect_list(float X=0, float Y=0, float W=1.0f, float H=1.0f):x(X),y(Y),w(W),h(H){}
	float x;
	float y;
	float w;
	float h;
};

struct bitmap_rect_list{
	bitmap_rect_list(float X, float Y, float W, float H):texture_rect(X,Y,W,H){}
	bitmap_rect_list(int X=0, int Y=0, int W=-1, int H=-1, float TX=0.0f, float TY=0.0f, float TW=1.0f, float TH=1.0f):screen_rect(X,Y,W,H),texture_rect(TX,TY,TW,TH){}
	bitmap_2d_list screen_rect;
	texture_rect_list texture_rect;
};

#endif
