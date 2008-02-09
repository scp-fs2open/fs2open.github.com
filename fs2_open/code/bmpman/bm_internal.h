/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/bmpman/bm_internal.h $
 * $Revision: 2.7 $
 * $Date: 2007-01-10 01:40:06 $
 * $Author: taylor $
 *
 * bmpman info that's internal to bmpman related files only
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2006/05/27 17:20:48  taylor
 * clean up BM_TYPE_* stuff so it's a little easier to tell what is what
 * bm_load_sub_fast() doesn't need to lowercase filenames, so don't
 * byte-swap 16-bit DDS on big endian (they still don't look right though)
 * update bm_has_alpha_channel() to be less dumb
 *
 * Revision 2.5  2006/05/13 07:29:51  taylor
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
 * Revision 2.4  2005/11/13 06:44:17  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.3  2005/04/21 15:49:20  taylor
 * update of bmpman and model bitmap management, well tested but things may get a bit bumpy
 *  - use VM_* macros for bmpman since it didn't seem to register the memory correctly (temporary)
 *  - a little "stupid" fix for dds bitmap reading
 *  - fix it so that memory is released properly on bitmap read errors
 *  - some cleanup to model texture loading
 *  - allow model textures to get released rather than just unloaded, saves bitmap slots
 *  - bump MAX_BITMAPS to 4750, should be able to decrease after public testing of new code
 *
 * Revision 2.2  2005/03/03 14:29:37  bobboau
 * fixed a small error from my earlier commit.
 *
 * Revision 2.1  2004/11/23 00:10:06  taylor
 * try and protect the bitmap_entry stuff a bit better
 * fix the transparent support ship, again, but correctly this time
 *
 * 
 * $NoKeywords: $
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
#define BM_TYPE_JPG				5		// 32 bit jpeg
#define BM_TYPE_ANI				6		// in-house ANI format
#define BM_TYPE_EFF				7		// specifies any type of animated image, the EFF itself is just text
// c-type types		( used in: bm_bitmaps[i].c_type )
#define BM_TYPE_DXT1			8		// 24 bit with switchable alpha		(compressed)
#define BM_TYPE_DXT3			9		// 32 bit with 4 bit alpha			(compressed)
#define BM_TYPE_DXT5			10		// 32 bit with 8 bit alpha			(compressed)
#define BM_TYPE_CUBEMAP_DDS		11		// generic DDS cubemap	(uncompressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT1	12		// 24-bit cubemap		(compressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT3	13		// 32-bit cubemap		(compressed cubemap surface)
#define BM_TYPE_CUBEMAP_DXT5	14		// 32-bit cubemap		(compressed cubemap surface)
// special types	( used in: bm_bitmaps[i].type )
#define BM_TYPE_RENDER_TARGET_STATIC	16		// 24/32 bit setup internally as a static render target
#define BM_TYPE_RENDER_TARGET_DYNAMIC	17		// 24/32 bit setup internally as a dynamic render target


/// Moved from cpp file ///////////////////
// Consider these 'protected' structures and functions that should only be used by special bitmap functions
typedef union bm_extra_info {
	struct {
		// Stuff needed for animations
		int		first_frame;								// used for animations -- points to index of first frame
		ubyte	num_frames;									// used for animation -- number of frames in the animation
		ubyte	fps;										// used for animation -- frames per second

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
void bm_lock_jpg( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );
void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags );


#endif // BMPMAN_INTERNAL

#endif // __BM_INTERNAL_H__
