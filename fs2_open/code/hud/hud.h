/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __HUD_H__
#define __HUD_H__

#include "hud/hudgauges.h"
#include "graphics/2d.h"
#include "hud/hudparse.h"

struct object;

typedef struct hud_anim {
	char filename[MAX_FILENAME_LEN];
	int first_frame;	// the bitmap id for the first frame in the animation... note that
							// all bitmap id's following this frame are numbered sequentially
	int num_frames;		// number of frames in the animation
	int sx, sy;			// screen (x,y) of top-left corner of animation
	float total_time;	// total time in seconds for the animation (depends on animation fps)
	float time_elapsed;	// time that has elapsed (in seconds) since animation started playing

	hud_anim( )
		: first_frame( 0 ), num_frames( 0 ), sx( 0 ), sy( 0 ),
		  total_time( 0 ), time_elapsed( 0 )
	{
		filename[ 0 ] = NULL;
	}
} hud_anim;

typedef struct hud_frames {
	int	first_frame;
	int	num_frames;
} hud_frames;

extern int HUD_draw;
extern int HUD_contrast;


//Current HUD to use for info -C
extern hud_info* current_hud;


#define HUD_NUM_COLOR_LEVELS	16
extern color HUD_color_defaults[HUD_NUM_COLOR_LEVELS];

// extern globals that will control the color of the HUD gauges
#define HUD_COLOR_ALPHA_USER_MAX		13		// max user-settable alpha, absolute max is 15
#define HUD_COLOR_ALPHA_USER_MIN		3		// min user-settable alpha, absolute min is 0

#define HUD_COLOR_ALPHA_MAX			15
#define HUD_COLOR_ALPHA_DEFAULT		8

#define HUD_BRIGHT_DELTA				7		// Level added to HUD_color_alpha to make brightness used for flashing

// hud macro for maybe flickering all gauges
#define GR_AABITMAP(a, b, c)						{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap(jx, jy); } }
#define GR_AABITMAP_EX(a, b, c, d, e, f, g)	{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap_ex(jx, jy, d, e, f, g); } }

// radar target identification flags
#define RTIF_CROSSHAIRS	(1<<0)		// draw crosshairs
#define RTIF_BLINK		(1<<1)		// make targeted blip blink
#define RTIF_PULSATE	(1<<2)		// make targeted blips size pulsate
#define RTIF_ENLARGE	(1<<3)		// make targeted blip appear larger than the other blips

extern int radar_target_id_flags;

extern int HUD_color_red;
extern int HUD_color_green;
extern int HUD_color_blue;
extern int HUD_color_alpha;

extern color HUD_color_debug;

// animations for damages gauges
extern hud_anim Target_static;
extern hud_anim Radar_static;

// Values used "wiggle" the HUD.  In the 2D HUD case, the clip region accounts
// for these, but for the 3d-type hud stuff, you need to add these in manually.
extern float HUD_offset_x;
extern float HUD_offset_y;

// the offset of the player's view vector and the ship forward vector in pixels (Swifty)
extern int HUD_nose_x;
extern int HUD_nose_y;
// Global: integrity of player's target
extern float Pl_target_integrity;
extern float Player_rearm_eta;

extern int Hud_max_targeting_range;

void HUD_init_colors();
void HUD_init();
void hud_update_frame();		// updates hud systems not dependant on rendering
void HUD_render_3d(float frametime);			// renders 3d dependant gauges
void HUD_render_2d(float frametime);			// renders the 2d gauges
void hud_stop_looped_engine_sounds();
void hud_show_messages();
void hud_damage_popup_toggle();

// set the offset values for this render frame
void HUD_set_offsets(object *viewer_obj, int wiggedy_wack, matrix *eye_orient);
// returns the offset between the player's view vector and the forward vector of the ship in pixels
// (Swifty)
void HUD_get_nose_coordinates(int *x, int *y);

// Basically like gr_reset_clip only it accounts for hud jittering
void HUD_reset_clip();
void hud_save_restore_camera_data(int save);

// Basically like gr_set_clip only it accounts for hud jittering
void HUD_set_clip(int x, int y, int w, int h);

// do flashing text gauge
void hud_start_text_flash(char *txt, int t);

// convert a string to use mono spaced numbers
void hud_num_make_mono(char *num_str);

// functions for handling hud animations
void hud_anim_init(hud_anim *ha, int sx, int sy, char *filename);
void hud_frames_init(hud_frames *hf);
int	hud_anim_render(hud_anim *ha, float frametime, int draw_alpha=0, int loop=1, int hold_last=0, int reverse=0,bool resize=true, bool mirror = false);
int	hud_anim_load(hud_anim *ha);

// flash text at the given y
void hud_show_text_flash_icon(char *txt, int y, int bright);

// functions for displaying the support view popup
void hud_support_view_start();
void hud_support_view_stop(int stop_now=0);
void hud_support_view_abort();
void hud_support_view_blit();

void HUD_init_hud_color_array();

// setting HUD colors
void hud_set_default_color();
void hud_set_iff_color(object *objp, int is_bright=0);
void hud_set_bright_color();
void hud_set_dim_color();

// HUD gauge functions
#define HUD_C_NONE			-4
#define HUD_C_BRIGHT			-3
#define HUD_C_DIM				-2
#define HUD_C_NORMAL			-1
void	hud_set_gauge_color(int gauge_index, int bright_index = HUD_C_NONE);
int	hud_gauge_active(int gauge_index);
void	hud_gauge_start_flash(int gauge_index);
int	hud_gauge_maybe_flash(int gauge_index);

// popup gauges
void	hud_init_popup_timers();
void	hud_gauge_popup_start(int gauge_index, int time=4000);
int	hud_gauge_is_popup(int gauge_index);
int	hud_gauge_popup_active(int gauge_index);

// objective status gauge
void hud_add_objective_messsage(int type, int status);

void	hud_maybe_clear_head_area();

int	hud_get_dock_time( object *docker_objp );
void	hud_show_radar();
void	hud_show_target_model();
void	hud_show_voice_status();

void	hud_subspace_notify_abort();

// render multiplayer text message currently being entered if any
void hud_maybe_render_multi_text();

int hud_get_draw();
void hud_toggle_draw();
int	hud_disabled();
int hud_support_find_closest( int objnum );

// Goober5000
void hud_set_draw(int draw);
void hud_disable_except_messages(int disable);
int hud_disabled_except_messages();

// contrast stuff
void hud_toggle_contrast();
void hud_set_contrast(int high);

#endif	/* __HUD_H__ */

