/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _BMPMAN_H
#define _BMPMAN_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif

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
extern int bm_load_animation( char * filename, int * nframes = NULL, int *fps = NULL, int *keyframe = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY );

//Loads either animation (bm_load_animation) or still image (bm_load)
extern int bm_load_either(char *filename, int *nframes = NULL, int *fps = NULL, int *keyframe = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY);

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
extern bitmap * bm_lock( int bitmapnum, ubyte bpp, ubyte flags );

// The signature is a field that gets filled in with 
// a unique signature for each bitmap.  The signature for each bitmap
// will also change when the bitmap's data changes.
extern uint bm_get_signature( int bitmapnum);

//gets the image type
ubyte bm_get_type(int handle);

// Unlocks a bitmap
extern void bm_unlock( int bitmapnum );

//WMC - Returns 0 if invalid, nonzero if valid
extern int bm_is_valid(int handle);

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
//WMC - Returns -1 on failure, handle or first frame handle on success.
int bm_get_info( int bitmapnum, int *w=NULL, int * h=NULL, ubyte * flags=NULL, int *nframes=NULL, int *fps=NULL );

// get filename
extern void bm_get_filename(int bitmapnum, char *filename);	 

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
