/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Anim/AnimPlay.h $
 * $Revision: 2.2 $
 * $Date: 2003-11-11 02:15:41 $
 * $Author: Goober5000 $
 *
 * Header file for playing back anim files
 *
 * $Log: not supported by cvs2svn $
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
 * 3     10/22/98 6:14p Dave
 * Optimized some #includes in Anim folder. Put in the beginnings of
 * parse/localization support for externalized strings and tstrings.tbl
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 11    5/07/98 3:11a Lawrance
 * Implement custom streaming code
 * 
 * 10    4/27/98 3:36p Dave
 * 
 * 9     3/25/98 8:43p Hoffoss
 * Changed anim_play() to not be so complex when you try and call it.
 * 
 * 8     12/24/97 8:57p Lawrance
 * Added anim_ignore_next_frametime()
 * 
 * 7     11/19/97 8:28p Dave
 * Hooked in Main Hall screen. Put in Anim support for ping ponging
 * animations as well as general reversal of anim direction.
 * 
 * 6     8/30/97 2:11p Lawrance
 * allow animations to loop
 * 
 * 5     8/25/97 11:13p Lawrance
 * support framerate independent playback with the option of now advancing
 * more than one frame at a time
 * 
 * 4     7/21/97 11:41a Lawrance
 * make playback time of .ani files keyed of frametime
 * 
 * 3     7/20/97 6:57p Lawrance
 * supporting new RLE format
 * 
 * 2     6/26/97 12:12a Lawrance
 * supporting anti-aliased bitmap animations
 * 
 * 1     6/23/97 5:09p Lawrance
 * 
 * 10    5/19/97 2:28p Lawrance
 * changes some variables to flags
 * 
 * 9     5/15/97 5:58p Lawrance
 * fix some bugs that were present with animations when playing multiple
 * missions
 * 
 * 8     5/15/97 4:42p Lawrance
 * suporting anims in-game
 * 
 * 7     5/15/97 11:46a Lawrance
 * add function to check if an anim is playing
 * 
 * 6     2/28/97 12:17p Lawrance
 * supporting mapping file to memory
 * 
 * 5     2/25/97 11:06a Lawrance
 * moved some higher level functions to from PackUnpack to AnimPlay
 * 
 * 4     2/19/97 9:27p Lawrance
 * added pause capability to anim playback
 * 
 * 3     2/17/97 4:19p Lawrance
 * using frame numbers instead of percentages for accessing keyframes
 * 
 * 2     2/17/97 3:01p Lawrance
 * code for playing back an anim
 *
 * $NoKeywords: $
 */

#ifndef __ANIMPLAY_H__
#define __ANIMPLAY_H__

#include "globalincs/pstypes.h"

struct anim;
struct anim_info;
struct anim_instance;
struct CFILE;

// structure passed in when playing an anim.  Talk about overkill..
typedef struct {
	anim *anim_info;
	int x;
	int y;
	int start_at;
	int stop_at;
	int screen_id;
	vector *world_pos;
	float radius;
	int framerate_independent;
	void *color;
	int skip_frames;
	int looped;
	int ping_pong;
} anim_play_struct;

extern int Anim_paused;

void				anim_init();
void				anim_level_init();
void				anim_level_close();
void				anim_render_all(int screen_id, float frametime);
void				anim_render_one(int screen_id, anim_instance *ani, float frametime);
void				anim_play_init(anim_play_struct *aps, anim *a_info, int x, int y);
anim_instance *anim_play(anim_play_struct *aps);
void				anim_ignore_next_frametime();
int				anim_stop_playing(anim_instance* anim_instance);
int				anim_show_next_frame(anim_instance *instance, float frametime);
void				anim_release_all_instances(int screen_id = 0);
void				anim_release_render_instance(anim_instance* instance);
anim			  *anim_load(char *name, int file_mapped = 0);
int				anim_free(anim *ptr);
int				anim_playing(anim_instance *ai);
int				anim_write_frames_out(char *filename);
void				anim_display_info(char *filename);
void				anim_read_header(anim *ptr, CFILE *fp);
void				anim_reverse_direction(anim_instance *ai);						// called automatically for ping-ponging, and can also be called externally
void				anim_pause(anim_instance *ai);
void				anim_unpause(anim_instance *ai);

int	anim_instance_is_streamed(anim_instance *ai);
unsigned char anim_instance_get_byte(anim_instance *ai, int offset);

#endif /* __ANIMPLAY_H__ */
