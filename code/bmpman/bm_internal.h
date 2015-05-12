#ifndef __BM_INTERNAL_H__
#define __BM_INTERNAL_H__
/*
* Copyright (C) Volition, Inc. 1999.  All rights reserved.
*
* All source code herein is the property of Volition, Inc. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source.
*
*/

/**
 * @file bm_internal.h
 * These are functions and types used by bmpman and a few others.
 *
 * @details It is a "protected" header that requires BMPMAN_INTERNAL to be defined before it can be included. This is to
 * provide a small measure of safety since this functions can cause problems if not used correctly
 */

#ifndef BMPMAN_INTERNAL
#error The file header "bmpman/bm_internal.h" is protected. Make sure you know what you are doing!
#endif

#include "bmpman/bmpman.h"
#include "globalincs/pstypes.h"

// Bitmap types
enum BM_TYPE
{
	BM_TYPE_NONE = 0,   //!< No type
	BM_TYPE_USER,       //!< in-memory
	BM_TYPE_PCX,        //!< PCX
	BM_TYPE_TGA,        //!< 16 or 32 bit targa
	BM_TYPE_DDS,        //!< generic identifier for DDS
	BM_TYPE_PNG,        //!< PNG
	BM_TYPE_JPG,        //!< 32 bit jpeg
	BM_TYPE_ANI,        //!< in-house ANI format
	BM_TYPE_EFF,        //!< specifies any type of animated image, the EFF itself is just text

	// special types
	BM_TYPE_RENDER_TARGET_STATIC,   //!< 24/32 bit setup internally as a static render target
	BM_TYPE_RENDER_TARGET_DYNAMIC,  //!< 24/32 bit setup internally as a dynamic render target

	// Compressed types (bitmap.c_type)
	BM_TYPE_DXT1,           //!< 24 bit with switchable alpha
	BM_TYPE_DXT3,           //!< 32 bit with 4 bit alpha
	BM_TYPE_DXT5,           //!< 32 bit with 8 bit alpha
	BM_TYPE_CUBEMAP_DDS,    //!< generic DDS cubemap (uncompressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT1,   //!< 24-bit cubemap        (compressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT3,   //!< 32-bit cubemap        (compressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT5    //!< 32-bit cubemap        (compressed cubemap surface)
};

union bm_extra_info {
	struct {
		// Stuff needed for animations
		int first_frame;    //!< used for animations -- points to index of first frame
		int num_frames;     //!< used for animations -- number of frames in the animation
		int keyframe;       //!< used for animations -- keyframe info
		ubyte fps;          //!< used for animations -- frames per second

		struct {
			// stuff for static animations
			ubyte type;                         //!< type for individual images
			char  filename[MAX_FILENAME_LEN];   //!< filename for individual images
		} eff;
	} ani;

	struct {
		// Stuff needed for user bitmaps
		void* data;         //!< For user bitmaps, this is where the data comes from
		ubyte bpp;          //!< For user bitmaps, this is what format the data is
		ubyte flags;        //!< For user bitmaps, Flags passed to bm_create
	} user;
};

struct bitmap_entry {
	// identification
	char filename[MAX_FILENAME_LEN];    //!< filename for this bitmap

	uint signature;         //!< a unique signature identifying the data
	uint palette_checksum;  //!< checksum used to be sure bitmap is in current palette
	int  handle;            //!< Handle = id*MAX_BITMAPS + bitmapnum
	int  last_used;         //!< When this bitmap was last used

	ubyte type;             //!< PCX, USER, ANI, etc
	ubyte comp_type;        //!< What sort of compressed type, BM_TYPE_NONE if not compressed
	signed char ref_count;  //!< Number of locks on bitmap.  Can't unload unless ref_count is 0.

	int dir_type;           //!< which directory this was loaded from (to skip other locations with same name)

	// compressed bitmap stuff (.dds) - RT please take a look at this and tell me if we really need it
	int mem_taken;          //!< How much memory does this bitmap use? - UnknownPlayer
	int num_mipmaps;        //!< number of mipmap levels, we need to read all of them

	// Stuff to keep track of usage
	ubyte preloaded;        //!< If set, then this was loaded from the lst file
	int   preload_count;    //!< how many times this gets used in game, for unlocking
	ubyte used_flags;       //!< What flags it was accessed thru
	int   load_count;

	bitmap bm;              //!< Bitmap info

	bm_extra_info info;     //!< Data for animations and user bitmaps

#ifdef BMPMAN_NDEBUG
	// bookeeping
	ubyte used_last_frame;  // If set, then it was used last frame
	ubyte used_this_frame;  // If set, then it was used this frame
	int   data_size;        // How much data this bitmap uses
	int   used_count;       // How many times it was accessed
#endif
};

extern bitmap_entry bm_bitmaps[MAX_BITMAPS];

// image specific lock functions
void bm_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_dds( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_png( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_jpg( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );


#endif // __BM_INTERNAL_H__
