/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __PACKUNPACK_H__
#define __PACKUNPACK_H__

#include "globalincs/pstypes.h"

struct CFILE;

#define ANI_STREAM_CACHE_SIZE			4096

#define PACKER_CODE						0xEE	// Use'd by PACKING_METHOD_RLE
#define PACKING_METHOD_RLE				0		// Hoffoss's RLE format
#define PACKING_METHOD_RLE_KEY		1		// Hoffoss's key frame RLE format
#define PACKING_METHOD_STD_RLE		2		// Standard RLE format (high bit is count)
#define PACKING_METHOD_STD_RLE_KEY	3		// Standard RLE format key frame

#define STD_RLE_CODE						0x80

typedef struct key_frame {	
	int frame_num;  // which frame number this key frame is
	int offset;  // offset from the start of data block	
} key_frame;

#define ANF_MEM_MAPPED		(1<<0)	// animation is memory-mapped file
#define ANF_STREAMED			(1<<1)
#define ANF_XPARENT			(1<<2)	// animation has transparency
#define ANF_ALL_KEYFRAMES  (1<<3)   // all the frames are keyframes (this is necessary if we want to play the file backwards)

typedef struct anim {
	anim			*next;
	char			name[MAX_PATH_LEN];
	ubyte			packer_code;
	int			width;
	int			height;
	int			total_frames;
	int			instance_count;		// number of instances that are currently playing
	int			ref_count;				// number of times this anim has been loaded
	float			time;						// playback time in seconds
	int			num_keys;
	key_frame	*keys;
	ubyte			palette[768];
	ubyte			palette_translation[256];
	ubyte			*data;		// points to compressed data
	CFILE*		cfile_ptr;
	int			version;
	int			fps;
	ubyte			xparent_r;		// red component for the transparent color in source image
	ubyte			xparent_g;		// green component for the transparent color in source image
	ubyte			xparent_b;		// blue component for the transparent color in source image
	int			flags;
	uint			screen_sig;	
	int			file_offset;	// file offset to start of frame data
	int			cache_file_offset;
	ubyte			*cache;
} anim;

// the direction to play the anim (forwards or backwards)
#define ANIM_DIRECT_FORWARD 0
#define ANIM_DIRECT_REVERSE 1

typedef struct anim_instance {
	anim_instance *next, *prev;
	int		x,y;				// coordinates anim is played at (top left corner of anim)
	vec3d	*world_pos;		// world (x,y,z) position of explosion
	float		radius;			// radius of image, needed for scaling
	int		frame_num;		// current frame, or last frame if between frames (first frame is 0)
	int		last_frame_num;// last frame rendered
	anim		*parent;			// pointer to anim structure, which holds compressed data
	ubyte		*data;			// pointer to next frame's compressed data
	ubyte		*frame;			// uncompressed frame
	float		time_elapsed;	// how long the anim has played for (in seconds)
	int		start_at;		// frame anim playing should start
	int		stop_at;			// frame anim playing should stop
	int		framerate_independent;	// animation should play back in same amount of time, regardless
	int		skip_frames;	// should anim skip frames during framerate independent playback
	int		looped;			// should anim keep playing over and over...
	int		stop_now;		// flag to indicate time to stop the animation
	int		last_bitmap;	// id of last bitmap that was rendered out from animation
	int		screen_id;		// 0 means all screens should render, otherwise screen specific
	void		*aa_color;		// anti-aliased bitmap color
	int		xlate_pal;	
	int		direction;		// playing forwards or backwards ?
	int		ping_pong;     // should be played ping-pong style
	int      paused;        // pause the anim
	int		file_offset;	// current offset into frame (like data, put offset into file)
	int		loop_count;		// starts at 0, and is incremented each time it loops
} anim_instance;

int	pack_key_frame(ubyte *frame, ubyte *save, long size, long max, int compress_type);
int	pack_frame(ubyte *frame, ubyte *frame2, ubyte *save, long size, long max, int compress_type);

ubyte	*unpack_frame(anim_instance *ai, ubyte *ptr, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp);
int unpack_frame_from_file(anim_instance *ai, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp);

void	anim_init();
anim_instance *init_anim_instance(anim *ptr, int bpp);
void	free_anim_instance(anim_instance *inst);
int	anim_get_next_frame(anim_instance *inst);
ubyte *anim_get_next_raw_buffer(anim_instance *inst, int xlate_pal, int aabitmap, int bpp);
void	anim_set_palette(anim *a);
void	anim_check_for_palette_change(anim_instance *inst);


#endif  /* __PACKUNPACK_H__ */
