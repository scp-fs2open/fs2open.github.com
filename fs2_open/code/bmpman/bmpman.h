/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Bmpman/BmpMan.h $
 *
 * $Revision: 2.37.2.6 $
 * $Date: 2007-03-22 20:14:16 $
 * $Author: taylor $
 *
 * Prototypes for Bitmap Manager functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.37.2.5  2007/02/11 09:51:21  taylor
 * remove some dead code
 * better texture memory handling (a work in-progress)
 * new image finding/loading
 * get rid of -pcx32 and -jpgtga
 * change the game_busy() reporting during bitmap page-in to only catch one frame of an animation
 * fix numerous little bugs in gropenglbmpman relating to -img2dds
 *
 * Revision 2.37.2.4  2007/01/07 12:07:40  taylor
 * fix bm_page_in_texture() so that it will load all frames of an animation (caused slowdowns in-game with it)
 *
 * Revision 2.37.2.3  2006/07/05 23:41:02  Goober5000
 * spelling
 *
 * Revision 2.37.2.2  2006/07/05 23:36:55  Goober5000
 * cvs comment tweaks
 *
 * Revision 2.37.2.1  2006/06/22 14:59:44  taylor
 * fix various things that Valgrind has been complaining about
 *
 * Revision 2.37  2006/05/27 17:20:48  taylor
 * clean up BM_TYPE_* stuff so it's a little easier to tell what is what
 * bm_load_sub_fast() doesn't need to lowercase filenames, so don't
 * byte-swap 16-bit DDS on big endian (they still don't look right though)
 * update bm_has_alpha_channel() to be less dumb
 *
 * Revision 2.36  2006/05/13 07:29:51  taylor
 * OpenGL envmap support
 * newer OpenGL extension support
 * add GL_ARB_texture_rectangle support for non-power-of-2 textures as interface graphics
 * add cubemap reading and writing support to DDS loader
 * fix bug in DDS loader that made compressed images with mipmaps use more memory than they really required
 * add support for a default envmap named "cubemap.dds"
 * new mission flag "$Environment Map:" to use a pre-existing envmap
 * minor cleanup of compiler warning messages
 * get rid of wasteful math from gr_set_proj_matrix()
 * remove extra gr_set_*_matrix() calls from starfield.cpp as there was no longer a reason for them to be there
 * clean up bmpman flags in reguards to cubemaps and render targets
 * disable D3D envmap code until it can be upgraded to current level of code
 * remove bumpmap code from OpenGL stuff (sorry but it was getting in the way, if it was more than copy-paste it would be worth keeping)
 * replace gluPerspective() call with glFrustum() call, it's a lot less math this way and saves the extra function call
 *
 * Revision 2.35  2006/02/16 05:00:01  taylor
 * various bmpman related fixes
 *  - some new error checking (and fixes related to that) and cleanup
 *  - fix EFFs not getting released/unloaded properly (was in a local tree but apparently missed CVS)
 *  - minor fixes for bm_release() to produce a more properly cleaned slot
 *  - use fast unloading for page_in stuff since we don't actually want really want the load count changing for texture maps
 *    and to make sure that we free the memory usage regardless of load count
 *
 * Revision 2.34  2006/01/21 02:22:04  wmcoolmon
 * Scripting updates; Special scripting image list; Better operator meta; Orientation type; Wing type; Texture type. Fix for MSVC7 compiling.
 *
 * Revision 2.33  2005/12/06 03:05:53  taylor
 * add base support for 8-bit DDS images
 * clean up some DDS error messages and other minor bug/comment fixes
 * move bitmap stuff from pstypes.h, it makes more sense here anyway
 * start of 8-bit palette conversion, needs work but is here for WMCoolmon to look at and play with
 *
 * Revision 2.32  2005/12/04 19:07:48  wmcoolmon
 * Final commit of codebase
 *
 * Revision 2.31  2005/11/08 01:03:59  wmcoolmon
 * More warnings instead of Int3s/Asserts, better Lua scripting, weapons_expl.tbl is no longer needed nor read, added "$Disarmed ImpactSnd:", fire-beam fix
 *
 * Revision 2.30  2005/09/26 04:53:19  Goober5000
 * moved these per taylor's recommendation
 * --Goober5000
 *
 * Revision 2.29  2005/09/26 04:08:53  Goober5000
 * some more cleanup
 * --Goober5000
 *
 * Revision 2.28  2005/09/26 02:15:02  Goober5000
 * okay, this should all be working :)
 * --Goober5000
 *
 * Revision 2.27  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.26  2005/09/24 07:07:17  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 2.25  2005/08/20 20:34:49  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.24  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.23  2005/06/19 02:28:55  taylor
 * add a _fast version of bm_unload() to be used in modelinterp and future graphics API code
 * clean up some modelinterp code to not use memcpy() everywhere so it's more platform compatible and matches old code (Jens Granseuer)
 * NaN check to catch shards-of-death and prevent hitting an Assert() (Jens Granseuer)
 * fix jumpnode code to catch model errors and close a memory leak
 * make the call to bm_unload_all() after model_free_all() since we will get bmpman screwups otherwise
 * don't show hardware sound RAM when using OpenAL build, it will always be 0
 * print top-right memory figures in debug builds slighly further over when 1024+ res
 *
 * Revision 2.22  2005/04/24 12:56:42  taylor
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
 * Revision 2.21  2005/04/21 15:49:20  taylor
 * update of bmpman and model bitmap management, well tested but things may get a bit bumpy
 *  - use VM_* macros for bmpman since it didn't seem to register the memory correctly (temporary)
 *  - a little "stupid" fix for dds bitmap reading
 *  - fix it so that memory is released properly on bitmap read errors
 *  - some cleanup to model texture loading
 *  - allow model textures to get released rather than just unloaded, saves bitmap slots
 *  - bump MAX_BITMAPS to 4750, should be able to decrease after public testing of new code
 *
 * Revision 2.20  2005/03/07 13:10:19  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.19  2005/02/10 14:38:50  taylor
 * fix an issue with bm_set_components()
 * abs is for ints fabsf is for floats (camera.cpp)
 * make the in-cockpit stuff OGL friendly
 *
 * Revision 2.18  2004/11/23 00:10:06  taylor
 * try and protect the bitmap_entry stuff a bit better
 * fix the transparent support ship, again, but correctly this time
 *
 * Revision 2.17  2004/11/21 15:59:45  Kazan
 * conflict res
 *
 * Revision 2.16  2004/10/31 21:26:27  taylor
 * bmpman merge, EFF animation support, better page in stuff, dozen or so smaller fixes and cleanup
 *
 * Revision 2.15  2004/10/06 22:02:54  Kazan
 * interface corruption fix - thanks taylor (MAX_BITMAPS upped to 7000)
 *
 * Revision 2.14  2004/08/11 05:06:18  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.13  2004/05/06 22:35:26  taylor
 * DDS mipmap reading, remove unneeded bm_unlock() during page in
 *
 * Revision 2.12  2004/04/01 15:31:20  taylor
 * don't use interface anis as ship textures
 *
 * Revision 2.11  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.10  2003/12/04 20:39:08  randomtiger
 * Added DDS image support for D3D
 * Added new command flag '-ship_choice_3d' to activate 3D models instead of ani's in ship choice, feature now off by default
 * Hopefully have fixed D3D text batching bug that caused old values to appear
 * Added Hud_target_object_factor variable to control 3D object sizes of zoom in HUD target
 * Fixed jump nodes not showing
 *
 * Revision 2.9  2003/11/19 20:37:22  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.8  2003/11/07 18:31:01  randomtiger
 * Fixed a nohtl call to htl funcs (crash with NULL pointer)
 * Fixed a bug with 32bit PCX code.
 * Fixed a bug in the d3d_string batch system that was messing up screen shaking.
 * Added a couple of checks to try and stop timerbar push and pop overloads, check returns missing pops if you use the system.
 * Put in a higher res icon until we get something better sorted out.
 *
 * Revision 2.7  2003/10/24 17:35:04  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.6  2003/08/16 03:52:22  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.5  2003/08/12 03:18:32  bobboau
 * Specular 'shine' mapping;
 * useing a phong lighting model I have made specular highlights
 * that are mapped to the model,
 * rendering them is still slow, but they look real purdy
 *
 * also 4 new (untested) comand lines, the XX is a floating point value
 * -spec_exp XX
 * the n value, you can set this from 0 to 200 (actualy more than that, but this is the recomended range), it will make the highlights bigger or smaller, defalt is 16.0 so start playing around there
 * -spec_point XX
 * -spec_static XX
 * -spec_tube XX
 * these are factors for the three diferent types of lights that FS uses, defalt is 1.0,
 * static is the local stars,
 * point is weapons/explosions/warp in/outs,
 * tube is beam weapons,
 * for thouse of you who think any of these lights are too bright you can configure them you're self for personal satisfaction
 *
 * Revision 2.4  2003/03/18 10:07:00  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.3  2003/01/19 01:07:41  bobboau
 * redid the way glow maps are handled; you now must set a global variable before you render a poly that uses a glow map, then set it to -1 when you're done with it
 * fixed a few other misc bugs too
 *
 * Revision 2.2  2003/01/18 19:55:16  phreak
 * fixed around the bmpman system to now accept compressed textures
 *
 * Revision 2.1  2003/01/05 23:41:50  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.0.2.10  2002/11/04 21:24:59  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who don't want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.0.2.9  2002/11/04 16:04:20  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.0.2.8  2002/11/04 03:02:28  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.0.2.7  2002/10/26 01:24:22  randomtiger
 * Fixed debug bitmap compiling bug.
 * Fixed tga bug. - RT
 *
 * Revision 2.0.2.6  2002/10/22 17:46:17  randomtiger
 * Fixed new TGA code texturing bug. - RT
 *
 * Revision 2.0.2.5  2002/10/21 16:33:40  randomtiger
 * Added D3D only 32 bit TGA functionality. Will load a texture as big as your graphics card allows. Code not finished yet and will forge the beginnings of the new texture system. - RT
 *
 * Revision 2.0.2.4  2002/10/19 23:56:39  randomtiger
 * Changed generic bitmap code to allow maximum dimensions to be determined by 3D's engines maximum texture size query.
 * Defaults to 256 as it was before. Also added base code for reworking the texture code to be more efficient. - RT
 *
 * Revision 2.0.2.3  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 16    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 15    8/06/99 1:52p Dave
 * Bumped up MAX_BITMAPS for the demo.
 * 
 * 14    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 13    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 12    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 11    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 10    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 9     2/08/99 5:07p Dave
 * FS2 chat server support. FS2 specific validated missions.
 * 
 * 8     2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 5     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 4     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 32    4/16/98 6:31p Hoffoss
 * Added function to get filename of a bitmap handle, which we don't have
 * yet and I need.
 * 
 * 31    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 30    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to ke7ep track of how much RAM we've malloc'd.
 * 
 * 29    3/29/98 4:05p John
 * New paging code that loads everything necessary at level startup.
 * 
 * 28    3/26/98 5:21p John
 * Added new code to preload all bitmaps at the start of a level.
 * Commented it out, though.
 * 
 * 27    3/24/98 6:18p John
 * Hacked MAX_BITMAPS up to 3500
 * 
 * 26    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 25    3/02/98 6:46p John
 * Upped MAX_BITMAPS to 2000
 * 
 * 24    3/02/98 6:00p John
 * Moved MAX_BITMAPS into BmpMan.h so the stuff in the graphics code that
 * is dependent on it won't break if it changes.   Made ModelCache slots
 * be equal to MAX_OBJECTS which is what it is.
 * 
 * 23    2/06/98 8:25p John
 * Added code for new bitmaps since last frame
 * 
 * 22    2/06/98 8:10p John
 * Added code to show amout of texture usage each frame.
 * 
 * 21    1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 20    1/11/98 2:14p John
 * Changed a lot of stuff that had to do with bitmap loading.   Made cfile
 * not do callbacks, I put that in global code.   Made only bitmaps that
 * need to load for a level load.
 * 
 * 19    9/03/97 4:19p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 18    8/25/97 11:14p Lawrance
 * added support for .ani files in bm_load_animation()
 * 
 * 17    7/16/97 3:07p John
 * 
 * 16    6/17/97 8:58p Lawrance
 * fixed bug with not nulling bm.data with USER bitmaps
 * 
 * 15    6/12/97 2:44a Lawrance
 * changed bm_unlock() to take an index into bm_bitmaps().  Added
 * ref_count to bitmap_entry struct
 * 
 * 14    5/20/97 10:36a John
 * Fixed problem with user bitmaps and direct3d caching.
 * 
 * 13    3/24/97 3:25p John
 * Cleaned up and restructured model_collide code and fvi code.  In fvi
 * made code that finds uvs work..  Added bm_get_pixel to BmpMan.
 * 
 * 12    2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#ifndef _BMPMAN_H
#define _BMPMAN_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif


// this is causing problems - let's UP IT -- Kazan
// bumped from 3500 to 4500 2004/09/21 - taylor
// bumped to 4750 2005/02/03 - taylor
//#define MAX_BITMAPS 3500			// How many bitmaps the game can handle
#define MAX_BITMAPS 4750			// How many bitmaps the game can handle
// NOTE:  MAX_BITMAPS shouldn't need to be bumped again.  With the fixed bm_release() and it's
// proper use even the largest missions should stay under this number.  With the largest retail
// missions and wasteful content we should still have about 20% of the slots free.  If it still
// goes over then it's something the artists need to fix.  For instance the Terran Mara fighter,
// with -spec and -glow and using the Shinepack, needs 117 bitmap slots alone.  111 of those is
// just for the glowmaps.  This number can be greatly reduced if the number of ani frames for
// another LOD than LOD0 has fewer or no ani frames.  A 37 frame glow ani for LOD2 is little
// more than a waste of resources.  Future reports of texture corruption should be initially
// approached with content as the cause and not code.  If anything we could/should reduce
// MAX_BITMAPS in the future.  Where it's at now should accomidate even the largest mods.
//  --  Taylor


#define	BMP_AABITMAP						(1<<0)				// antialiased bitmap
#define	BMP_TEX_XPARENT						(1<<1)				// transparent texture
#define	BMP_TEX_OTHER						(1<<2)				// so we can identify all "normal" textures
#define BMP_TEX_DXT1						(1<<3)				// dxt1 compressed 8r8g8b1a (24bit)
#define BMP_TEX_DXT3						(1<<4)				// dxt3 compressed 8r8g8b4a (32bit)
#define BMP_TEX_DXT5						(1<<5)				// dxt5 compressed 8r8g8b8a (32bit)
#define BMP_TEX_CUBEMAP						(1<<6)				// a texture made for cubic environment map
// ***** NOTE:  bitmap.flags is an 8-bit value, no more BMP_TEX_* flags can be added unless the type is changed!! ******

//compressed texture types
#define BMP_TEX_COMP			( BMP_TEX_DXT1 | BMP_TEX_DXT3 | BMP_TEX_DXT5 )

//non compressed textures
#define BMP_TEX_NONCOMP			( BMP_TEX_XPARENT | BMP_TEX_OTHER )

// any texture type
#define	BMP_TEX_ANY				( BMP_TEX_COMP | BMP_TEX_NONCOMP )

#define BMP_FLAG_RENDER_TARGET_STATIC		(1<<0)
#define BMP_FLAG_RENDER_TARGET_DYNAMIC		(1<<1)
#define BMP_FLAG_CUBEMAP					(1<<2)

typedef struct bitmap {
	short	w, h;		// Width and height
	short	rowsize;	// What you need to add to go to next row
	ubyte	bpp;		// How many bits per pixel it is. (7,8,15,16,24,32) (what is requested)
	ubyte	true_bpp;	// How many bits per pixel the image actually is.
	ubyte	flags;		// See the BMP_???? defines for values (this isn't for the BMP_FLAG_* stuff)
	ptr_u	data;		// Pointer to data, or maybe offset into VRAM.
	ubyte *palette;		// If bpp==8, this is pointer to palette.   If the BMP_NO_PALETTE_MAP flag
						// is not set, this palette just points to the screen palette. (gr_palette)
} bitmap;


extern int Bm_paging;

void bm_init();

void bm_close();

int bm_get_cache_slot( int bitmap_id, int separate_ani_frames );

int bm_get_next_handle();

void *bm_malloc(int n, int size);

void bm_update_memory_used(int n, int size);

// how many bytes of textures are used.
extern int bm_texture_ram;

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.
int bm_load(char * filename);

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_load_duplicate(char *filename);

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of data.  The data can be in the following formats:
// 8 bpp (mapped into game palette)
// 32 bpp
// On success, it returns the bitmap number.  You cannot 
// free that RAM until bm_release is called on that bitmap.  
// See example at bottom of this file
int bm_create( int bpp, int w, int h, void *data = NULL, int flags = 0);

// Frees up a bitmap's data, but bitmap number 'n' can
// still be used, it will just have to be paged in next
// time it is locked.
int bm_unload( int n, int clear_render_targets = 0 );

// like bm_unload() except that it's safe to use to free data without worrying about
// load_count so it's safe to use in relation to bm_release() and in gr_*_texture functions
int bm_unload_fast( int n, int clear_render_targets = 0 );

// Frees up a bitmap's data, and it's slot, so bitmap 
// number 'n' cannot be used anymore, and bm_load or
// bm_create might reuse the slot.
int bm_release( int n, int clear_render_targets = 0 );

// This loads a bitmap sequence so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number of the first frame and nframes is set.
extern int bm_load_animation( char * filename, int * nframes = NULL, int *fps = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY );

//Loads either animation (bm_load_animation) or still image (bm_load)
extern int bm_load_either(char *filename, int *nframes = NULL, int *fps = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY);

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
extern bitmap * bm_lock( int bitmapnum, ubyte bpp, ubyte flags );

// The signature is a field that gets filled in with 
// a unique signature for each bitmap.  The signature for each bitmap
// will also change when the bitmap's data changes.
extern uint bm_get_signature( int bitmapnum);

// Unlocks a bitmap
extern void bm_unlock( int bitmapnum );

//WMC - Returns 0 if invalid, nonzero if valid
extern int bm_is_valid(int handle);

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
//WMC - Returns -1 on failure, handle or first frame handle on success.
int bm_get_info( int bitmapnum, int *w=NULL, int * h=NULL, ubyte * flags=NULL, int *nframes=NULL, int *fps=NULL );

// get filename
extern void bm_get_filename(int bitmapnum, char *filename);	 

// resyncs all the bitmap palette
extern void bm_update();

// call to load all data for all bitmaps that have been requested to be loaded
extern void bm_gfx_load_all();
extern void bm_unload_all();

// call to get the palette for a bitmap
void bm_get_palette(int n, ubyte *pal, char *name);

// Hacked function to get a pixel from a bitmap.
// Only works good in 8bpp mode.
void bm_gfx_get_pixel( int bitmap, float u, float v, ubyte *r, ubyte *g, ubyte *b );

// Returns number of bytes of bitmaps locked this frame
// ntotal = number of bytes of bitmaps locked this frame
// nnew = number of bytes of bitmaps locked this frame that weren't locked last frame
void bm_get_frame_usage(int *ntotal, int *nnew);

/* 
 * Example on using bm_create
 * 
	{
		static int test_inited = 0;
		static int test_bmp;
		static uint test_bmp_data[128*64];

		if ( !test_inited )	{
			test_inited = 1;
			// Create the new bitmap and fill in its data.
			// When you're done with it completely, call
			// bm_release to free up the bitmap handle
			test_bmp = bm_create( 32, 128, 64, test_bmp_data );
			int i,j;
			for (i=0; i<64; i++ )	{
				for (j=0; j<64; j++ )	{
					uint r=i*4;
					test_bmp_data[j+i*128] = r;
				}
			}
		}

		bm_unload(test_bmp);	// this pages out the data, so that the
									// next bm_lock will convert the new data to the
									// correct bpp

		// put in new data
		int x,y;
		gr_reset_clip();
		for (y=0; y<64; y++)
			for (x=0; x<128; x++ )
				test_bmp_data[y*128+x] = 15;

		// Draw the bitmap to upper left corner
		gr_set_bitmap(test_bmp);
		gr_bitmap( 0,0 );
	}
*/


//============================================================================
// Paging stuff
//============================================================================

void bm_page_in_start();
void bm_page_in_stop();

// Paging code in a library should call these functions
// in its page in function.

// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_texture( int bitmapnum, int num_frames = 0 );

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_xparent_texture( int bitmapnum, int num_frames=1 );

// Marks an aabitmap as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_aabitmap( int bitmapnum, int num_frames=1 );

// unload a texture that was paged in
int bm_page_out( int handle );

// 
// Mode: 0 = High memory
//       1 = Low memory ( every other frame of ani's)
//       2 = Debug low memory ( only use first frame of each ani )
void bm_set_low_mem( int mode );

char *bm_get_filename(int handle);

void BM_SELECT_SCREEN_FORMAT();
void BM_SELECT_TEX_FORMAT();
void BM_SELECT_ALPHA_TEX_FORMAT();

// set the rgba components of a pixel, any of the parameters can be NULL
extern void (*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
extern void (*bm_set_components_32)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_16_screen(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_32_screen(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_16_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_32_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

extern int GLOWMAP;	//this holds a reference to a map that is a fully lit version of its index -Bobboau
extern int SPECMAP;	//this holds a reference to a map that is for specular mapping -Bobboau
extern int ENVMAP;	//this holds a reference to a map that is for environment mapping -Bobboau
extern int NORMMAP;	// normal mapping
extern int HEIGHTMAP;	// height map for normal mapping

int bm_is_compressed(int num);
int bm_get_tcache_type(int num);
int bm_get_size(int num);
int bm_get_num_mipmaps(int num);
int bm_has_alpha_channel(int handle);

void bm_print_bitmaps();

int bm_make_render_target( int width, int height, int flags );
int bm_is_render_target(int bitmap_id);
int bm_set_render_target(int handle, int face = -1);

#endif
