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
 * $Revision: 2.15 $
 * $Date: 2004-10-06 22:02:54 $
 * $Author: Kazan $
 *
 * Prototypes for Bitmap Manager functions
 *
 * $Log: not supported by cvs2svn $
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
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
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
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
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

#include "PreProcDefines.h"
#ifndef _BMPMAN_H
#define _BMPMAN_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif

// this is causing problems - let's DOUBLE IT -- Kazan
//#define MAX_BITMAPS 3500			// How many bitmaps the game can handle
#define MAX_BITMAPS 7000

// 16 bit pixel formats
#define BM_PIXEL_FORMAT_ARGB				0						// for glide - can assume certain things, like 1555 LFB writes, whee!
#define BM_PIXEL_FORMAT_D3D				1						// d3d - card dependant. booo!
#define BM_PIXEL_FORMAT_ARGB_D3D			2						// this card has nice 1555 textures like Glide - ahhhhh!

enum
{
	BM_TYPE_NONE,		
	BM_TYPE_PCX,	// 16 bit PCX		
	BM_TYPE_USER,	// Data created at run time	
	BM_TYPE_ANI,	// In-house ANI format
	// Only list 32 bit formats after this flag please
	BM_TYPE_32_BIT_FORMATS,
	BM_TYPE_TGA,				
	BM_TYPE_JPG,					
	BM_TYPE_DDS,					
};

/// Moved from cpp file ///////////////////
// Consider these 'protected' structures and functions that should only be used by special bitmap functions

typedef union bm_extra_info	{
	struct {
		// Stuff needed for animations
		int		first_frame;								// used for animations -- points to index of first frame
		ubyte		num_frames;									// used for animation -- number of frames in the animation
		ubyte		fps;											// used for animation -- frames per second
	} ani;
	struct {
		// Stuff needed for user bitmaps
		void		*data;									// For user bitmaps, this is where the data comes from
		ubyte		bpp;									// For user bitmaps, this is what format the data is
		ubyte		flags;									// Flags passed to bm_create
	} user;
} bm_extra_info;

typedef struct bitmap_entry	{
	// identification
	char		filename[MAX_FILENAME_LEN];			// filename for this bitmap

	uint		signature;									// a unique signature identifying the data
	uint		palette_checksum;							// checksum used to be sure bitmap is in current palette
	int		handle;										// Handle = id*MAX_BITMAPS + bitmapnum
	int		last_used;									// When this bitmap was last used

	ubyte		type;											// PCX, USER, ANI, etc
	signed char	ref_count;								// Number of locks on bitmap.  Can't unload unless ref_count is 0.

	// compressed bitmap stuff (.dds) - RT please take a look at this and tell me if we really need it
	int		mem_taken;									// How much memory does this bitmap use? - UnknownPlayer
	int		num_mipmaps;								// number of mipmap levels, we need to read all of them

	// Stuff to keep track of usage
	ubyte		preloaded;									// If set, then this was loaded from the lst file
	ubyte		used_flags;									// What flags it was accessed thru

	// Bitmap info
	bitmap	bm;

	// Data for animations and user bitmaps
	bm_extra_info	info;		

#ifdef BMPMAN_NDEBUG
	// bookeeping
	ubyte		used_last_frame;							// If set, then it was used last frame
	ubyte		used_this_frame;							// If set, then it was used this frame
	int		data_size;									// How much data this bitmap uses
	int		used_count;									// How many times it was accessed
#endif

} bitmap_entry;

void bm_gfx_init();
void bm_gfx_close();
int bm_gfx_get_cache_slot( int bitmap_id, int separate_ani_frames );

int bm_gfx_get_next_handle();

// 16 bit pixel formats
extern int Bm_pixel_format;

#define BYTES_PER_PIXEL(x)	((x+7)/8)

// how many bytes of textures are used.
extern int bm_texture_ram;

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.
int bm_gfx_load(char * filename);

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_gfx_load_duplicate(char *filename);

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of data.  The data can be in the following formats:
// 8 bpp (mapped into game palette)
// 32 bpp
// On success, it returns the bitmap number.  You cannot 
// free that RAM until bm_release is called on that bitmap.  
// See example at bottom of this file
int bm_gfx_create( int bpp, int w, int h, void * data, int flags = 0);

// Frees up a bitmap's data, but bitmap number 'n' can
// still be used, it will just have to be paged in next
// time it is locked.
int bm_gfx_unload( int n );

// Frees up a bitmap's data, and it's slot, so bitmap 
// number 'n' cannot be used anymore, and bm_load or
// bm_create might reuse the slot.
void bm_gfx_release(int n);

// This loads a bitmap sequence so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number of the first frame and nframes is set.
extern int bm_gfx_load_animation( char * filename, int * nframes, int *fps = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY );

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
extern bitmap * bm_gfx_lock( int bitmapnum, ubyte bpp, ubyte flags );

// The signature is a field that gets filled in with 
// a unique signature for each bitmap.  The signature for each bitmap
// will also change when the bitmap's data changes.
extern uint bm_get_signature( int bitmapnum);

// Unlocks a bitmap
extern void bm_gfx_unlock( int bitmapnum );

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
extern void bm_gfx_get_info( int bitmapnum, int *w=NULL, int * h=NULL, ubyte * flags=NULL, int *nframes=NULL, int *fps=NULL, bitmap_section_info **sections = NULL );

// get filename
extern void bm_get_filename(int bitmapnum, char *filename);	 

// resyncs all the bitmap palette
extern void bm_update();

// call to load all data for all bitmaps that have been requested to be loaded
extern void bm_gfx_load_all();
extern void bm_gfx_unload_all();

// call to get the palette for a bitmap
void bm_gfx_get_palette(int n, ubyte *pal, char *name);

// Hacked function to get a pixel from a bitmap.
// Only works good in 8bpp mode.
void bm_gfx_get_pixel( int bitmap, float u, float v, ubyte *r, ubyte *g, ubyte *b );

// Returns number of bytes of bitmaps locked this frame
// ntotal = number of bytes of bitmaps locked this frame
// nnew = number of bytes of bitmaps locked this frame that weren't locked last frame
void bm_gfx_get_frame_usage(int *ntotal, int *nnew);

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

void bm_gfx_page_in_start();
void bm_gfx_page_in_stop();

// Paging code in a library should call these functions
// in its page in function.

// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_gfx_page_in_texture( int bitmapnum, int num_frames=1 );

// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_gfx_page_in_nondarkening_texture( int bitmap, int num_frames=1 );

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_gfx_page_in_xparent_texture( int bitmapnum, int num_frames=1 );

// Marks an aabitmap as being used for this level
// If num_frames is passed, assume this is an animation
void bm_gfx_page_in_aabitmap( int bitmapnum, int num_frames=1 );

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
void bm_set_components_argb(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_d3d(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_16_screen(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_32_screen(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_16_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);
void bm_set_components_argb_d3d_32_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_gfx_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

//============================================================================
// section info stuff
//============================================================================

// given a bitmap and a section, return the size (w, h)
void bm_gfx_get_section_size(int bitmapnum, int sx, int sy, int *w, int *h);

extern int GLOWMAP;	//this holds a reference to a map that is a fully lit version of it's index -Bobboau
extern int SPECMAP;	//this holds a reference to a map that is for specular mapping -Bobboau
extern int ENVMAP;	//this holds a reference to a map that is for environment mapping -Bobboau

int bm_is_compressed(int num);
int bm_get_size(int num);
int bm_get_num_mipmaps(int num);

#endif
