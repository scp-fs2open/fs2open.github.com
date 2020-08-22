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

#include "bmpman/bmpman.h"

#include <array>

/**
 * @brief Container class for graphics API specific bitmap data
 */
class gr_bitmap_info {
 public:
	virtual ~gr_bitmap_info() = 0;
};

union bm_extra_info {
	struct {
		// Stuff needed for animations
		int first_frame;    //!< used for animations -- points to index of first frame
		int num_frames;     //!< used for animations -- number of frames in the animation
		int keyframe;       //!< used for animations -- keyframe info
		float total_time;   //!< used for animations -- total animation time (not always derived from num_frames/fps)
		ubyte fps;          //!< used for animations -- frames per second
		bool is_array;      //!< Flag for if all frames of an animation have the same size which means that it can be put into a texture array

		struct {
			// stuff for static animations
			BM_TYPE type;                         //!< type for individual images
			char  filename[MAX_FILENAME_LEN];   //!< filename for individual images
		} eff;
		struct {
			bool  is_apng;      //!< Is this animation an APNG?
			float frame_delay;  //!< cumulative frame delay
		} apng;
	} ani;

	struct {
		// Stuff needed for user bitmaps
		void* data;         //!< For user bitmaps, this is where the data comes from
		ubyte bpp;          //!< For user bitmaps, this is what format the data is
		ushort flags;        //!< For user bitmaps, Flags passed to bm_create
	} user;
};

struct bitmap_entry {
	// identification
	char filename[MAX_FILENAME_LEN];    //!< filename for this bitmap

	uint signature;         //!< a unique signature identifying the data
	int  handle;            //!< Handle = id*MAX_BITMAPS + bitmapnum

	BM_TYPE type;             //!< PCX, USER, ANI, etc
	BM_TYPE comp_type;        //!< What sort of compressed type, BM_TYPE_NONE if not compressed
	signed char ref_count;  //!< Number of locks on bitmap.  Can't unload unless ref_count is 0.

	int dir_type;           //!< which directory this was loaded from (to skip other locations with same name)

	// compressed bitmap stuff (.dds) - RT please take a look at this and tell me if we really need it
	size_t mem_taken;          //!< How much memory does this bitmap use? - UnknownPlayer
	int num_mipmaps;        //!< number of mipmap levels, we need to read all of them

	// Stuff to keep track of usage
	ubyte preloaded;        //!< If set, then this was loaded from the lst file
	int   preload_count;    //!< how many times this gets used in game, for unlocking
	ushort used_flags;       //!< What flags it was accessed thru
	int   load_count;

	bitmap bm;              //!< Bitmap info

	bm_extra_info info;     //!< Data for animations and user bitmaps

#ifdef BMPMAN_NDEBUG
	// bookeeping
	ubyte used_last_frame;  // If set, then it was used last frame
	ubyte used_this_frame;  // If set, then it was used this frame
	size_t data_size;        // How much data this bitmap uses
	int   used_count;       // How many times it was accessed
#endif
};

struct bitmap_slot {
	bitmap_entry entry;

	gr_bitmap_info* gr_info = nullptr;
};

// image specific lock functions
void bm_lock_ani( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_dds( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_png( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_apng( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_jpg( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_pcx( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_tga( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );
void bm_lock_user( int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags );

const size_t BM_BLOCK_SIZE = 4096;

extern SCP_vector<std::array<bitmap_slot, BM_BLOCK_SIZE>> bm_blocks;

bitmap_slot* bm_get_slot(int handle, bool separate_ani_frames = true);

inline bitmap_entry* bm_get_entry(int handle, bool separate_ani_frames = true) {
	return &bm_get_slot(handle, separate_ani_frames)->entry;
}

template<typename T>
T* bm_get_gr_info(int handle, bool separate_ani_frames = true) {
	return static_cast<T*>(bm_get_slot(handle, separate_ani_frames)->gr_info);
}

#endif // __BM_INTERNAL_H__
