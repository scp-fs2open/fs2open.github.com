/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifndef __BM_INTERNAL_H__
#define __BM_INTERNAL_H__


// extra check to make sure this stuff doesn't end up in normal files
// don't use any of this unless BMPMAN_INTERNAL is defined
#ifdef BMPMAN_INTERNAL

#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"


// no-type			( used in: bm_bitmaps[i].type )
#define BM_TYPE_NONE			0
// in-memory type	( used in: bm_bitmaps[i].type )
#define BM_TYPE_USER			1
// file-type types	( used in: bm_bitmaps[i].type )
#define BM_TYPE_PCX				2
#define BM_TYPE_TGA				3		// 16 or 32 bit targa
#define BM_TYPE_DDS				4		// generic identifier for DDS
#define BM_TYPE_PNG				5		// PNG
#define BM_TYPE_JPG				6		// 32 bit jpeg
#define BM_TYPE_ANI				7		// in-house ANI format
#define BM_TYPE_EFF				8		// specifies any type of animated image, the EFF itself is just text
// c-type types		( used in: bm_bitmaps[i].c_type )
#define BM_TYPE_DXT1			9		// 24 bit with switchable alpha		(compressed)
#define BM_TYPE_DXT3			10		// 32 bit with 4 bit alpha			(compressed)
#define BM_TYPE_DXT5			11		// 32 bit with 8 bit alpha			(compressed)
#define BM_TYPE_CUBEMAP_DDS		12		// generic DDS cubemap	(uncompressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT1	13		// 24-bit cubemap		(compressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT3	14		// 32-bit cubemap		(compressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT5	15		// 32-bit cubemap		(compressed cubemap surface)
// special types	( used in: bm_bitmaps[i].type )
#define BM_TYPE_RENDER_TARGET_STATIC	16		// 24/32 bit setup internally as a static render target
#define BM_TYPE_RENDER_TARGET_DYNAMIC	17		// 24/32 bit setup internally as a dynamic render target


/// Moved from cpp file ///////////////////
// Consider these 'protected' structures and functions that should only be used by special bitmap functions
typedef union bm_extra_info {
	struct {
		// Stuff needed for animations
		int		first_frame;								// used for animations -- points to index of first frame
		int	num_frames;									// used for animation -- number of frames in the animation
		ubyte	fps;										// used for animation -- frames per second
		int	keyframe;									// used for animation -- keyframe info

		struct {
			// stuff for static animations
			ubyte	type;									// type for individual images
			char	filename[MAX_FILENAME_LEN];				// filename for individual images
		} eff;
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

	ubyte		type;									// PCX, USER, ANI, etc
	ubyte		comp_type;								// What sort of compressed type, BM_TYPE_NONE if not compressed
	signed char	ref_count;								// Number of locks on bitmap.  Can't unload unless ref_count is 0.

	int		dir_type;								// which directory this was loaded from (to skip other locations with same name)

	// compressed bitmap stuff (.dds) - RT please take a look at this and tell me if we really need it
	int		mem_taken;									// How much memory does this bitmap use? - UnknownPlayer
	int		num_mipmaps;								// number of mipmap levels, we need to read all of them

	// Stuff to keep track of usage
	ubyte		preloaded;									// If set, then this was loaded from the lst file
	int			preload_count;								// how many times this gets used in game, for unlocking
	ubyte		used_flags;									// What flags it was accessed thru
	int			load_count;

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

extern bitmap_entry bm_bitmaps[MAX_BITMAPS];


// image specific lock functions
void bm_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_dds( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_png( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_jpg( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );


#endif // BMPMAN_INTERNAL

#endif // __BM_INTERNAL_H__
