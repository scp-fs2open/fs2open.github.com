/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Anim/PackUnpack.h $
 * $Revision: 2.5 $
 * $Date: 2005-07-13 02:50:48 $
 * $Author: Goober5000 $
 *
 * Code for handling packing and unpacking in Hoffoss's RLE format, used for
 * Anim files.  Also handles Anim loading, creating Anim instances (for
 * utilizing an Anim), and getting getting frames of the Anim.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2005/04/05 05:53:14  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.3  2004/08/11 05:06:18  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:07  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 6     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     10/22/98 6:14p Dave
 * Optimized some #includes in Anim folder. Put in the beginnings of
 * parse/localization support for externalized strings and tstrings.tbl
 * 
 * 3     10/16/98 3:42p Andsager
 * increase MAX_WEAPONS and MAX_SHIPS and som header files
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 32    5/18/98 5:59p Hoffoss
 * Made command briefing advanced now once the speech stops and animation
 * has fully played once, whichever is longer.
 * 
 * 31    5/07/98 3:11a Lawrance
 * Implement custom streaming code
 * 
 * 30    1/14/98 6:43p Lawrance
 * Add ref_count to anim struct, so we don't free multiple times
 * 
 * 29    11/19/97 8:28p Dave
 * Hooked in Main Hall screen. Put in Anim support for ping ponging
 * animations as well as general reversal of anim direction.
 * 
 * 28    8/30/97 2:11p Lawrance
 * allow animations to loop
 * 
 * 27    8/25/97 11:13p Lawrance
 * support framerate independent playback with the option of now advancing
 * more than one frame at a time
 * 
 * 26    8/21/97 5:11p Lawrance
 * frame numbering for ANI's now is from 0 -> total_frames-1.
 * 
 * 25    7/28/97 10:42p Lawrance
 * re-did interface to unpack_frame() to make more general
 * 
 * 24    7/21/97 11:41a Lawrance
 * make playback time of .ani files keyed of frametime
 * 
 * 23    7/20/97 6:57p Lawrance
 * supporting new RLE format
 * 
 * 22    6/27/97 4:36p Lawrance
 * update pal translation table when gr_screen.signature changes
 * 
 * 21    6/26/97 12:12a Lawrance
 * supporting anti-aliased bitmap animations
 * 
 * 20    6/25/97 3:03p Lawrance
 * fix palette translation problem with anti-alised bitmaps
 * 
 * 19    5/27/97 3:48p Lawrance
 * don't re-create a bitmap if using the same frame of animation
 * 
 * 18    5/21/97 11:06a Lawrance
 * enabling a user-defined transparent value
 * 
 * 17    5/19/97 3:21p Lawrance
 * add fps parm, version num to anim header
 * 
 * 16    5/19/97 2:28p Lawrance
 * changes some variables to flags
 * 
 * 15    5/15/97 4:42p Lawrance
 * supporting animations in-game
 * 
 * 14    3/01/97 2:08p Lawrance
 * not using windows.h, since memory mapping details moved to cfile
 * 
 * 13    2/28/97 12:17p Lawrance
 * supporting mapping file to memory
 * 
 * 12    2/25/97 11:06a Lawrance
 * moved some higher level functions to from PackUnpack to AnimPlay
 * 
 * 11    2/19/97 9:51p Lawrance
 * made keyframe decompression more effecient, moved 
 * default anim FPS to header file
 * 
 * 10    2/19/97 4:01p Lawrance
 * load_anim returns int, not void
 * 
 * 9     2/17/97 4:19p Lawrance
 * changed stop and start to be actual frame numbers, not percentages
 * 
 * 8     2/17/97 4:17p Hoffoss
 * modified packing internal format and added random access function to an
 * Anim frame.
 * 
 * 7     2/17/97 2:59p Lawrance
 * integrating into game
 * 
 * 6     2/14/97 11:09p Hoffoss
 * Made optimizations.
 * 
 * 5     2/14/97 10:48p Hoffoss
 * fixed bug.
 * 
 * 4     2/14/97 10:38p Lawrance
 * fixing bugs
 * 
 * 3     2/14/97 3:29p Hoffoss
 * Added header for MSDEV to fill in.
 *
 * $NoKeywords: $
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

extern int packer_code;

int	pack_key_frame(ubyte *frame, ubyte *save, long size, long max, int compress_type);
int	pack_frame(ubyte *frame, ubyte *frame2, ubyte *save, long size, long max, int compress_type);

ubyte	*unpack_frame(anim_instance *ai, ubyte *ptr, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp);
int unpack_frame_from_file(anim_instance *ai, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp);

void	anim_init();
anim_instance *init_anim_instance(anim *ptr, int bpp);
void	free_anim_instance(anim_instance *inst);
int	anim_get_next_frame(anim_instance *inst);
int	anim_get_frame(anim_instance *inst, int frame_num, int xlate_pal=1);
ubyte *anim_get_next_raw_buffer(anim_instance *inst, int xlate_pal, int aabitmap, int bpp);
void	anim_set_palette(anim *a);
void	anim_check_for_palette_change(anim_instance *inst);


#endif  /* __PACKUNPACK_H__ */
