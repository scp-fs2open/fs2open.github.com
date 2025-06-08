/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "hud/hudconfig.h"
#include "hud/hudtargetbox.h"
#include "hud/hudwingmanstatus.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/emp.h"


#define HUD_WINGMAN_STATUS_NUM_FRAMES	5
#define BACKGROUND_LEFT						0
#define BACKGROUND_MIDDLE					1
#define BACKGROUND_RIGHT					2
#define WINGMAN_STATUS_DOTS				3
#define WINGMAN_STATUS_NAMES				4

#define HUD_WINGMAN_STATUS_NONE			0		// wingman doesn't exist
#define HUD_WINGMAN_STATUS_DEAD			1		// wingman has died
#define HUD_WINGMAN_STATUS_ALIVE			2		// wingman is in the mission
#define HUD_WINGMAN_STATUS_NOT_HERE		3		// wingman hasn't arrived, or has departed

struct wingman_status
{
	bool	ignore;													// set to true when we should ignore this item -- used in team v. team
	bool	used;
	float hull[MAX_SHIPS_PER_WING];			// 0.0 -> 1.0
	int	status[MAX_SHIPS_PER_WING];		// HUD_WINGMAN_STATUS_* 
	int dot_anim_override[MAX_SHIPS_PER_WING]; // optional wingmen dot status to use instead of default --wookieejedi
};

wingman_status HUD_wingman_status[MAX_SQUADRON_WINGS];

#define HUD_WINGMAN_UPDATE_STATUS_INTERVAL	200
static int HUD_wingman_update_timer;

static int HUD_wingman_flash_duration[MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING];
static int HUD_wingman_flash_next[MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING];
static int HUD_wingman_flash_is_bright;

// flag a player wing ship as destroyed
void hud_set_wingman_status_dead(int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_pos < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_DEAD;
}

// flags a given player wing ship as departed
void hud_set_wingman_status_departed(int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_pos < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_NOT_HERE;
}

// flags a given player wing ship as not existing
void hud_set_wingman_status_none( int wing_index, int wing_pos)
{
	int i;

	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_pos < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_NONE;

	bool used = false;
	for ( i = 0; i < MAX_SHIPS_PER_WING; i++ ) {
		if ( HUD_wingman_status[wing_index].status[i] != HUD_WINGMAN_STATUS_NONE ) {
			used = true;
			break;
		}
	}

	HUD_wingman_status[wing_index].used = used;
}

// flags a given player wing ship as "alive" (for multiplayer respawns )
void hud_set_wingman_status_alive( int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_pos < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_ALIVE;
}

// function which marks the other team wing as not used for the wingman status gauge
void hud_wingman_kill_multi_teams()
{
	int wing_index;

	// do nothing in single player or non team v. team games
	if ( Game_mode & GM_NORMAL )
		return;

	if ( !IS_MISSION_MULTI_TEAMS )
		return;

	Assert(MAX_TVT_WINGS == 2);	// Goober5000

	wing_index = -1;
	if ( Net_player->p_info.team == 0 )
		wing_index = 1;
	else if ( Net_player->p_info.team == 1 )
		wing_index = 0;

	if ( wing_index == -1 )
		return;

	HUD_wingman_status[wing_index].ignore = true;
}


// called once per level to init the wingman status gauge.  Loads in the frames the first time
void hud_init_wingman_status_gauge()
{
	int	i, j;

	hud_wingman_status_init_flash();

	HUD_wingman_update_timer=timestamp(0);	// update status right away

	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		HUD_wingman_status[i].ignore = false;
		HUD_wingman_status[i].used = false;
		for ( j = 0; j < MAX_SHIPS_PER_WING; j++ ) {
			HUD_wingman_status[i].status[j] = HUD_WINGMAN_STATUS_NONE;
			HUD_wingman_status[i].dot_anim_override[j] = -1;
		}
	}

	hud_wingman_kill_multi_teams();
	hud_wingman_status_update();

	//  --wookieejedi
	// also check the wingmen dot override animation and set if needed
	// this section accounts for ships that are present at mission start
	// the wingmen dot override for ships not present at mission start are set hud_wingman_status_set_index() 
	for (auto& p_obj : Parse_objects) {
		int dot_override = Ship_info[p_obj.ship_class].wingmen_status_dot_override;
		if (dot_override >= 0) {
			// note, the wingmen_status_dot_override value will only have been set
			// during ship table parse if the number of frames was 2
			bm_page_in_aabitmap(dot_override, 2);

			int wing_index = p_obj.wing_status_wing_index;
			int wing_pos = p_obj.wing_status_wing_pos;
			if ((wing_index >= 0) && (wing_pos >= 0)) {
				HUD_wingman_status[wing_index].dot_anim_override[wing_pos] = dot_override;
			}
		}
	}
}

// Update the status of the wingman status
void hud_wingman_status_update()
{
	if ( timestamp_elapsed(HUD_wingman_update_timer) ) {

		HUD_wingman_update_timer=timestamp(HUD_WINGMAN_UPDATE_STATUS_INTERVAL);

		for (int wing_index = 0; wing_index < MAX_SQUADRON_WINGS; wing_index++) {
			if (Squadron_wings[wing_index] < 0)
				continue;

			auto wingp = &Wings[Squadron_wings[wing_index]];
			for (int shipnum: wingp->ship_index) {
				if (shipnum < 0)
					continue;

				auto shipp = &Ships[shipnum];
				auto ship_objp = &Objects[shipp->objnum];
				int wing_pos = shipp->wing_status_wing_pos;

				HUD_wingman_status[wing_index].used = true;
				if (!(shipp->is_departing()) ) {
					HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_ALIVE;	
				}
				HUD_wingman_status[wing_index].hull[wing_pos] = get_hull_pct(ship_objp);
				if ( HUD_wingman_status[wing_index].hull[wing_pos] <= 0 ) {
					HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_DEAD;
				}
			}
		}
	}
}

HudGaugeWingmanStatus::HudGaugeWingmanStatus():
HudGauge(HUD_OBJECT_WINGMAN_STATUS, HUD_WINGMEN_STATUS, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeWingmanStatus::initialize()
{
	initFlash();

	HudGauge::initialize();
}
void HudGaugeWingmanStatus::initHeaderOffsets(int x, int y)
{
	header_offsets[0] = x;
	header_offsets[1] = y;
}

void HudGaugeWingmanStatus::initFixedHeaderPosition(bool fixed)
{
	fixed_header_position = fixed;
}

void HudGaugeWingmanStatus::initLeftFrameEndX(int x)
{
	left_frame_end_x = x;
}

void HudGaugeWingmanStatus::initSingleWingOffsets(int x, int y)
{
	single_wing_offsets[0] = x;
	single_wing_offsets[1] = y;
}

void HudGaugeWingmanStatus::initMultipleWingOffsets(int x, int y)
{
	multiple_wing_offsets[0] = x;
	multiple_wing_offsets[1] = y;
}

void HudGaugeWingmanStatus::initWingWidth(int w)
{
	wing_width = w;
}

void HudGaugeWingmanStatus::initRightBgOffset(int offset)
{
	right_frame_start_offset = offset;
}

void HudGaugeWingmanStatus::initWingNameOffsets(int x, int y)
{
	wing_name_offsets[0] = x;
	wing_name_offsets[1] = y;
}

void HudGaugeWingmanStatus::initWingmate1Offsets(int x, int y)
{
	wingmate_offsets[0][0] = x;
	wingmate_offsets[0][1] = y;
}

void HudGaugeWingmanStatus::initWingmate2Offsets(int x, int y)
{
	wingmate_offsets[1][0] = x;
	wingmate_offsets[1][1] = y;
}

void HudGaugeWingmanStatus::initWingmate3Offsets(int x, int y)
{
	wingmate_offsets[2][0] = x;
	wingmate_offsets[2][1] = y;
}

void HudGaugeWingmanStatus::initWingmate4Offsets(int x, int y)
{
	wingmate_offsets[3][0] = x;
	wingmate_offsets[3][1] = y;
}

void HudGaugeWingmanStatus::initWingmate5Offsets(int x, int y)
{
	wingmate_offsets[4][0] = x;
	wingmate_offsets[4][1] = y;
}

void HudGaugeWingmanStatus::initWingmate6Offsets(int x, int y)
{
	wingmate_offsets[5][0] = x;
	wingmate_offsets[5][1] = y;
}

void HudGaugeWingmanStatus::initBitmaps(char *fname_left, char *fname_middle, char *fname_right, char *fname_dots)
{
	Wingman_status_left.first_frame = bm_load_animation(fname_left, &Wingman_status_left.num_frames);
	if ( Wingman_status_left.first_frame == -1 ) {
		Warning(LOCATION, "Error loading %s\n", fname_left);
	}

	Wingman_status_middle.first_frame = bm_load_animation(fname_middle, &Wingman_status_middle.num_frames);
	if ( Wingman_status_middle.first_frame == -1 ) {
		Warning(LOCATION, "Error loading %s\n", fname_middle);
	}

	Wingman_status_right.first_frame = bm_load_animation(fname_right, &Wingman_status_right.num_frames);
	if ( Wingman_status_right.first_frame == -1 ) {
		Warning(LOCATION, "Error loading %s\n", fname_right);
	}

	Wingman_status_dots.first_frame = bm_load_animation(fname_dots, &Wingman_status_dots.num_frames);
	if ( Wingman_status_dots.first_frame == -1 ) {
		Warning(LOCATION, "Error loading %s\n", fname_dots);
	}
}

void HudGaugeWingmanStatus::initGrowMode(int mode) {
	grow_mode = mode;
}

void HudGaugeWingmanStatus::initWingnameAlignMode(int mode)
{
	wingname_align_mode = mode;
}

void HudGaugeWingmanStatus::initUseFullWingnames(bool usefullname)
{
	use_full_wingnames = usefullname;
}

void HudGaugeWingmanStatus::initUseExpandedColors(bool useexpandedcolors)
{
	use_expanded_colors = useexpandedcolors;
}


void HudGaugeWingmanStatus::renderBackground(int num_wings_to_draw, bool config)
{
	if((num_wings_to_draw < 1) || (num_wings_to_draw > 5)){
		Int3();
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int sx, sy;
	if((num_wings_to_draw > 2) && (grow_mode == GROW_LEFT)) {
		// make some room for the spacers
		sx = x - (num_wings_to_draw - 2) * fl2i(wing_width * scale); 
	} else {
		sx = x;
	}
	sy = y;

	int bitmap = Wingman_status_left.first_frame;

	if ( bitmap > -1 ) {
		renderBitmap(bitmap, sx, sy, scale, config);
	}
	
	//Tell renderDots() where to start
	actual_origin[0] = sx;
	actual_origin[1] = sy;

	// write "wingmen" on gauge
	int header_x, header_y;
	if (fixed_header_position) {
		header_x = x + fl2i(header_offsets[0] * scale);
		header_y = y + fl2i(header_offsets[1] * scale);
	} else {
		header_x = sx + fl2i(header_offsets[0] * scale);
		header_y = sy + fl2i(header_offsets[1] * scale);
	}
	renderString(header_x, header_y, XSTR( "wingmen", 352), scale, config);

	// bring us to the end of the left portion so we can draw the last or middle bits depending on how many wings we have to draw
	if ( grow_mode == GROW_DOWN ) {
		sy += fl2i(left_frame_end_x * scale);
	} else {
		sx += fl2i(left_frame_end_x * scale);
	}

	bitmap = Wingman_status_middle.first_frame;

	if ( grow_mode == GROW_DOWN ) {
		for ( int i = 0; i < num_wings_to_draw; i++ ) {
			renderBitmap(bitmap, sx, sy, scale, config);
			sy += fl2i(wing_width * scale);
		}

		sy += fl2i(right_frame_start_offset * scale);
	} else {
		if(num_wings_to_draw > 2 && bitmap >= 0) {
			for(int i = 0; i < num_wings_to_draw - 2; i++){
				renderBitmap(bitmap, sx, sy, scale, config);
				sx += fl2i(wing_width * scale);
			}
		}

		sx += fl2i(right_frame_start_offset * scale);
	}

	bitmap = Wingman_status_right.first_frame;
	if (bitmap >= 0) {
		renderBitmap(bitmap, sx, sy, scale, config);
	}

	if (config) {
		int bmw, bmh;
		bm_get_info(Wingman_status_right.first_frame, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id, actual_origin[0], sx + fl2i(bmw * scale), actual_origin[1], sy + fl2i(bmh * scale));
	}
}

void HudGaugeWingmanStatus::renderDots(int wing_index, int screen_index, int num_wings_to_draw, bool config)
{

	if ( Wingman_status_dots.first_frame < 0 ) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	// Note actual_origin[] scales earlier in renderBackground()
	int sx, sy;
	if(num_wings_to_draw == 1) {
		sx = x + fl2i(single_wing_offsets[0] * scale);
		sy = y + fl2i(single_wing_offsets[1] * scale);
	} else if ( grow_mode == GROW_DOWN ) {
		sx = actual_origin[0] + fl2i(multiple_wing_offsets[0] * scale); // wing_width = 35
		sy = actual_origin[1] + fl2i(multiple_wing_offsets[1] * scale) + screen_index * fl2i(wing_width * scale);
	} else {
		sx = actual_origin[0] + fl2i(multiple_wing_offsets[0] * scale) +
			 (screen_index - 1) * fl2i(wing_width * scale); // wing_width = 35
		sy = actual_origin[1] + fl2i(multiple_wing_offsets[1] * scale);
	}


	// draw wingman dots
	int bitmap, frame_num = -1;

	for (int i = 0; i < MAX_SHIPS_PER_WING; i++ ) {

		bool is_bright = false;
		if (!config && maybeFlashStatus(wing_index, i) ) {
			is_bright=true;
		}

		if (!config) {
			switch (HUD_wingman_status[wing_index].status[i]) {

			case HUD_WINGMAN_STATUS_ALIVE:
				frame_num = 0;
				// set colors depending on HUD table option --wookieejedi
				if (use_expanded_colors) {
					// use expanded colors
					if (HUD_wingman_status[wing_index].hull[i] > 0.67f) {
						// green > 2/3 health
						gr_set_color_fast(is_bright ? &Color_bright_green : &Color_green);
					} else if (HUD_wingman_status[wing_index].hull[i] > 0.34f) {
						// yellow 2/3 - > 1/3 health
						gr_set_color_fast(is_bright ? &Color_bright_yellow : &Color_yellow);
					} else {
						// red <= 1/3 health
						gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
					}
				} else {
					// use default colors
					if (HUD_wingman_status[wing_index].hull[i] > 0.5f) {
						// use gauge color
						setGaugeColor(is_bright ? HUD_C_BRIGHT : HUD_C_NORMAL);
					} else {
						gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
					}
				}
				break;

			case HUD_WINGMAN_STATUS_DEAD:
				gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
				frame_num = 1;
				break;

			case HUD_WINGMAN_STATUS_NOT_HERE:
				setGaugeColor(is_bright ? HUD_C_BRIGHT : HUD_C_NORMAL);
				frame_num = 1;
				break;

			default:
				bitmap = -1;
				frame_num = -1;
				break;

			} // end swtich
		// Config mode
		} else {
			switch (wing_index) {
			// Dead example
			case 1:
				gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
				frame_num = 1;
				break;
			// Exited example
			case 2:
				setGaugeColor(is_bright ? HUD_C_BRIGHT : HUD_C_NORMAL, config);
				frame_num = 1;
				break;
			// Alive example
			default:
				if (use_expanded_colors) {
					switch (i) {
					// yellow
					case 1:
					case 3:
						gr_set_color_fast(is_bright ? &Color_bright_yellow : &Color_yellow);
						break;
					// red
					case 2:
					case 5:
						gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
						break;
					// green
					default:
						gr_set_color_fast(is_bright ? &Color_bright_green : &Color_green);
						break;
					}					
				} else {
					setGaugeColor(HUD_C_NORMAL, config);
					switch (i) {
					// dying
					case 1:
					case 3:
					case 5:
						gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
						break;
					// normal
					default:
						setGaugeColor(HUD_C_NORMAL, config);
						break;
					}
				}
				
				frame_num = 0;
				break;
			}
			
		}

		// draw dot if there is a status to draw
		if (frame_num > -1) {
			// use wingmen dot animation if present, otherwise use default --wookieejedi
			if (!config && HUD_wingman_status[wing_index].dot_anim_override[i] >= 0) {
				bitmap = HUD_wingman_status[wing_index].dot_anim_override[i];
			} else {
				bitmap = Wingman_status_dots.first_frame;
			}

			if (bitmap > -1) {
				renderBitmap(bitmap + frame_num, sx + fl2i(wingmate_offsets[i][0] * scale), sy + fl2i(wingmate_offsets[i][1] * scale), scale, config);
			}
		}
	}

	// draw wing name
	sx += fl2i(wing_name_offsets[0] * scale);
	sy += fl2i(wing_name_offsets[1] * scale);
	
	setGaugeColor(HUD_C_NONE, config);

	// check if using full names or default abbreviations before rendering text
	char wingstr[NAME_LENGTH];

	if (use_full_wingnames) {
		// wookieejedi - use full wing name with unaltered capitalization
		strcpy_s(wingstr, config ? HC_wingam_gauge_status_names[wing_index] : Squadron_wing_names[wing_index]);
	} else {
		// Goober5000 - get the lowercase abbreviation
		char abbrev[4];
		abbrev[0] = SCP_tolower(config ? HC_wingam_gauge_status_names[wing_index][0] : Squadron_wing_names[wing_index][0]);
		abbrev[1] = SCP_tolower(config ? HC_wingam_gauge_status_names[wing_index][1] : Squadron_wing_names[wing_index][1]);
		abbrev[2] = SCP_tolower(config ? HC_wingam_gauge_status_names[wing_index][2] : Squadron_wing_names[wing_index][2]);
		abbrev[3] = '\0';
		strncpy(wingstr, abbrev, 4);
	}

	int wingstr_width;
	gr_get_string_size(&wingstr_width, nullptr, wingstr, scale);

	if (wingname_align_mode == ALIGN_LEFT) {
		renderString(sx, sy, wingstr, scale, config);
	} else if (wingname_align_mode == ALIGN_RIGHT) {
		renderString(sx - wingstr_width, sy, wingstr, scale, config);
	} else {
		// Goober5000 - center it (round the offset rather than truncate it)
		renderString(sx - fl2i(std::lround((float)wingstr_width / 2.0f)), sy, wingstr, scale, config);
	}

}

int hud_wingman_status_wingmen_exist(int num_wings_to_draw)
{
	int i, j, count = 0;

	switch ( num_wings_to_draw ) {
	case 0:
		count = 0;
		break;
	case 1:
		for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
			if ( HUD_wingman_status[i].used ) {
				for ( j = 0; j < MAX_SHIPS_PER_WING; j++ ) {
					if ( HUD_wingman_status[i].status[j] != HUD_WINGMAN_STATUS_NONE ) {
						count++;
					}
				}
			}
		}
		break;
	default:
		count = 2;
		break;

	}

	if ( count > 1 ) {
		return 1;
	}

	return 0;
}

void HudGaugeWingmanStatus::render(float  /*frametime*/, bool config)
{
	int num_wings_to_draw = 0;
	if (!config) {
		for (auto & status : HUD_wingman_status) {
			if (status.used) {
				num_wings_to_draw++;
			}
		}
	} else {
		num_wings_to_draw = 5;
	}

	if (!config && !hud_wingman_status_wingmen_exist(num_wings_to_draw) ) {
		return;
	}

	// hud_set_default_color();
	setGaugeColor(HUD_C_NONE, config);

	// blit the background frames
	renderBackground(num_wings_to_draw, config);

	int count = 0;
	for (int i = 0; i < MAX_SQUADRON_WINGS; i++) {
		if (!config && ( !(HUD_wingman_status[i].used) || (HUD_wingman_status[i].ignore) )) {
			continue;
		}

		if (config && i >= num_wings_to_draw)
			continue;

		renderDots(i, count, num_wings_to_draw, config);
		count++;
	}
}

// init the flashing timers for the wingman status gauge
void hud_wingman_status_init_flash()
{
	int i, j;

	for ( i = 0; i < MAX_SQUADRON_WINGS; i++ ) {
		for ( j = 0; j < MAX_SHIPS_PER_WING; j++ ) {
			HUD_wingman_flash_duration[i][j] = timestamp(0);
			HUD_wingman_flash_next[i][j] = timestamp(0);
		}
	}

	HUD_wingman_flash_is_bright = 0;
}

void HudGaugeWingmanStatus::initFlash()
{
	int i, j;

	for ( i = 0; i < MAX_SQUADRON_WINGS; i++ ) {
		for ( j = 0; j < MAX_SHIPS_PER_WING; j++ ) {
			next_flash[i][j] = timestamp(0);
		}
	}

	flash_status = 0;
}

// start the targetbox item flashing for TBOX_FLASH_DURATION
void hud_wingman_status_start_flash(int wing_index, int wing_pos)
{
	HUD_wingman_flash_duration[wing_index][wing_pos] = timestamp(TBOX_FLASH_DURATION);
}

// set the color for flashing dot
// exit:	1 =>	set bright color
//			0 =>	set default color
int hud_wingman_status_maybe_flash(int wing_index, int wing_pos)
{
	int index, draw_bright=0;

	index = wing_index*MAX_SHIPS_PER_WING + wing_pos;

	if ( !timestamp_elapsed(HUD_wingman_flash_duration[wing_index][wing_pos]) ) {
		if ( timestamp_elapsed(HUD_wingman_flash_next[wing_index][wing_pos]) ) {
			HUD_wingman_flash_next[wing_index][wing_pos] = timestamp(TBOX_FLASH_INTERVAL);
			HUD_wingman_flash_is_bright ^= (1<<index);	// toggle between default and bright frames
		}

		if ( HUD_wingman_flash_is_bright & (1<<index) ) {
			draw_bright=1;
		}
	}

	return draw_bright;
}

bool HudGaugeWingmanStatus::maybeFlashStatus(int wing_index, int wing_pos)
{
	int index;
	bool draw_bright = false;

	index = wing_index*MAX_SHIPS_PER_WING + wing_pos;

	if ( !timestamp_elapsed(HUD_wingman_flash_duration[wing_index][wing_pos]) ) {
		if ( timestamp_elapsed(next_flash[wing_index][wing_pos]) ) {
			next_flash[wing_index][wing_pos] = timestamp(TBOX_FLASH_INTERVAL);
			flash_status ^= (1<<index);	// toggle between default and bright frames
		}

		if ( flash_status & (1<<index) ) {
			draw_bright = true;
		}
	}

	return draw_bright;
}

void hud_wingman_status_set_index(int squad_wing_index, wing *wingp, ship *shipp, p_object *pobjp)
{
	// Check for squadron wings
	if ( squad_wing_index < 0 )
		return;

	// this wing is shown on the squadron display
	shipp->wing_status_wing_index = i2ch(squad_wing_index);

	// for the first wave, if we have a parse object, just use the parse object position
	if (wingp->current_wave == 1 && pobjp != nullptr)
	{
		shipp->wing_status_wing_pos = i2ch(pobjp->pos_in_wing);
	}
	// otherwise, find the first position not taken
	else
	{
		int i, pos, wing_bitfield = 0;

		// fill in all the positions currently used by this wing
		for (i = 0; i < wingp->wave_count; i++)
		{
			pos = Ships[wingp->ship_index[i]].wing_status_wing_pos;
			if (pos >= 0)
				wing_bitfield |= (1<<pos);
		}

		// now assign the first available slot
		for (i = 0; i < MAX_SHIPS_PER_WING; i++)
		{
			if (!(wing_bitfield & (1<<i)))
			{
				shipp->wing_status_wing_pos = i2ch(i);
				break;
			}
		}
	}

	// --wookieejedi
	// also check the wingmen dot override animation and set if needed
	// this section accounts for ships that are not present at mission start
	// the wingmen dot override for ships present at mission start are set hud_init_wingman_status_gauge() 
	int wing_pos = shipp->wing_status_wing_pos;
	if ( (squad_wing_index >= 0) && (wing_pos >= 0) ) {
		int dot_override = Ship_info[shipp->ship_info_index].wingmen_status_dot_override;
		if (dot_override >= 0) {
			// note, the wingmen_status_dot_override value will only have been set
			// during ship table parse if the number of frames was 2
			bm_page_in_aabitmap(dot_override, 2);

			HUD_wingman_status[squad_wing_index].dot_anim_override[wing_pos] = dot_override;
		}
	}
}

void hud_wingman_status_set_index(wing *wingp, ship *shipp, p_object *pobjp)
{
	int squad_wing_index = ship_squadron_wing_lookup(wingp->name);
	hud_wingman_status_set_index(squad_wing_index, wingp, shipp, pobjp);
}

void hud_set_new_squadron_wings(const std::array<int, MAX_SQUADRON_WINGS> &new_squad_wingnums)
{
	// set up a map to tell where wings are going in the new arrangement
	SCP_unordered_map<int, int> new_squad_indexes;
	for (int wingnum: Squadron_wings)
	{
		if (wingnum >= 0)
		{
			auto loc = std::find(new_squad_wingnums.begin(), new_squad_wingnums.end(), wingnum);
			if (loc != new_squad_wingnums.end())
				new_squad_indexes[wingnum] = static_cast<int>(std::distance(new_squad_wingnums.begin(), loc));
		}
	}

	// clear or reassign the ships that are currently in the squadron wings
	for (int wingnum: Squadron_wings)
	{
		if (wingnum < 0)
			continue;
		auto wingp = &Wings[wingnum];
		auto new_squad_index = new_squad_indexes.find(wingnum);

		for (int shipnum: wingp->ship_index)
		{
			if (shipnum < 0)
				continue;
			auto shipp = &Ships[shipnum];

			if (new_squad_index == new_squad_indexes.end())
			{
				shipp->wing_status_wing_index = -1;
				shipp->wing_status_wing_pos = -1;
			}
			else
				shipp->wing_status_wing_index = i2ch(new_squad_index->second);
		}
	}

	// copy the old status to the new locations
	wingman_status Backup_HUD_wingman_status[MAX_SQUADRON_WINGS];
	memcpy(Backup_HUD_wingman_status, HUD_wingman_status, sizeof(HUD_wingman_status));
	for (int cur_squad_index = 0; cur_squad_index < MAX_SQUADRON_WINGS; cur_squad_index++)
	{
		int wingnum = Squadron_wings[cur_squad_index];
		if (wingnum < 0)
			continue;

		auto new_squad_index = new_squad_indexes.find(wingnum);
		if (new_squad_index == new_squad_indexes.end())
			continue;

		memcpy(&HUD_wingman_status[new_squad_index->second], &Backup_HUD_wingman_status[cur_squad_index], sizeof(wingman_status));
	}

	// set the new squadron wings on the HUD
	for (int new_squad_index = 0; new_squad_index < MAX_SQUADRON_WINGS; new_squad_index++)
	{
		int wingnum = new_squad_wingnums[new_squad_index];
		auto wing_name = wingnum < 0 ? "" : Wings[wingnum].name;

		strcpy_s(Squadron_wing_names[new_squad_index], wing_name);
		Squadron_wings[new_squad_index] = wingnum;

		HUD_wingman_status[new_squad_index].used = (wingnum >= 0);
	}

	// set the ships in the new squadron wings, but only for the ones that weren't copied
	for (int new_squad_index = 0; new_squad_index < MAX_SQUADRON_WINGS; new_squad_index++)
	{
		int wingnum = Squadron_wings[new_squad_index];
		if (wingnum < 0)
			continue;
		if (new_squad_indexes.find(wingnum) != new_squad_indexes.end())
			continue;

		// clear the status before we actually add any wingmen
		for (int &status: HUD_wingman_status[new_squad_index].status)
			status = HUD_WINGMAN_STATUS_NONE;

		// now add the existing wingmen
		auto wingp = &Wings[wingnum];
		for (int shipnum: wingp->ship_index)
		{
			if (shipnum >= 0)
				hud_wingman_status_set_index(new_squad_index, wingp, &Ships[shipnum], nullptr);
		}
	}

	// refresh the HUD status for everybody
	HUD_wingman_update_timer = timestamp(0);	// update status right away
	hud_wingman_status_update();
}

void HudGaugeWingmanStatus::pageIn()
{
	bm_page_in_aabitmap( Wingman_status_left.first_frame, Wingman_status_left.num_frames);
	bm_page_in_aabitmap( Wingman_status_middle.first_frame, Wingman_status_middle.num_frames);
	bm_page_in_aabitmap( Wingman_status_right.first_frame, Wingman_status_right.num_frames);
	bm_page_in_aabitmap( Wingman_status_dots.first_frame, Wingman_status_dots.num_frames);
}
