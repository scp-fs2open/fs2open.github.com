/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <ctype.h> // for 'tolower'


#include "hud/hud.h"
#include "hud/hudwingmanstatus.h"
#include "ship/ship.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "hud/hudtargetbox.h"
#include "globalincs/linklist.h"
#include "weapon/emp.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "iff_defs/iff_defs.h"
#include "globalincs/alphacolors.h"
#include "network/multi.h"


#define HUD_WINGMAN_STATUS_NUM_FRAMES	5
#define BACKGROUND_LEFT						0
#define BACKGROUND_MIDDLE					1
#define BACKGROUND_RIGHT					2
#define WINGMAN_STATUS_DOTS				3
#define WINGMAN_STATUS_NAMES				4

static char *Wingman_status_filenames[GR_NUM_RESOLUTIONS][HUD_WINGMAN_STATUS_NUM_FRAMES] =
{
//XSTR:OFF
	{ // GR_640
		"wingman1",
		"wingman2",
		"wingman3",
		"wingman4",
		"wingman5",
	}, 
	{ // GR_1024
		"wingman1",
		"wingman2",
		"wingman3",
		"wingman4",
		"wingman5",
	}
//XSTR:ON
};

static hud_frames Wingman_status_frames[HUD_WINGMAN_STATUS_NUM_FRAMES];
static int Wingman_status_gauge_loaded=0;

#define HUD_WINGMAN_STATUS_NONE			0		// wingman doesn't exist
#define HUD_WINGMAN_STATUS_DEAD			1		// wingman has died
#define HUD_WINGMAN_STATUS_ALIVE			2		// wingman is in the mission
#define HUD_WINGMAN_STATUS_NOT_HERE		3		// wingman hasn't arrived, or has departed

typedef struct Wingman_status
{
	int	ignore;													// set to 1 when we should ignore this item -- used in team v. team
	int	used;
	float hull[MAX_SHIPS_PER_WING];			// 0.0 -> 1.0
	int	status[MAX_SHIPS_PER_WING];		// HUD_WINGMAN_STATUS_* 
} wingman_status;

wingman_status HUD_wingman_status[MAX_SQUADRON_WINGS];

#define HUD_WINGMAN_UPDATE_STATUS_INTERVAL	200
static int HUD_wingman_update_timer;

static int HUD_wingman_flash_duration[MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING];
static int HUD_wingman_flash_next[MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING];
static int HUD_wingman_flash_is_bright;


// coords to draw wingman status icons, for 1-5 wings (0-4)
int HUD_wingman_left_coords[GR_NUM_RESOLUTIONS][5][2] = {
	{ // GR_640
		{550, 144},				// where to draw the left part of gauge if we have 1 wing
		{550, 144},				// "" 2 wings
		{515, 144},				// "" 3 wings
		{480, 144},				// "" 4 wings
		{445, 144}				// "" 5 wings
	},
	{ // GR_1024
		{932, 144},
		{932, 144},
		{897, 144},
		{862, 144},
		{827, 144}
	},
};
int HUD_wingman_middle_coords[GR_NUM_RESOLUTIONS][5][2] = {
	{ // GR_640
		{0, 0},					// we never draw this for 1 wing
		{0, 0},					// we never draw this for 2 wings
		{586, 144},				// where to draw the _first_ middle gauge for 3 wings
		{551, 144},				// "" 4 wings
		{516, 144}				// "" 5 wings
	}, 
	{ // GR_1024
		{0, 0},
		{0, 0},
		{968, 144},
		{933, 144},
		{898, 144}
	}
};
int HUD_wingman_right_coords[GR_NUM_RESOLUTIONS][5][2] = {
	{ // GR_640
		{621, 144},			// always drawn in the same spot
		{621, 144},
		{621, 144},
		{621, 144},
		{621, 144},
	}, 
	{ // GR_1024
		{1003, 144},
		{1003, 144},
		{1003, 144},
		{1003, 144},
		{1003, 144},
	}
};

int HUD_wingman_status_name_coords[GR_NUM_RESOLUTIONS][MAX_SQUADRON_WINGS][2] =
{
	{ // GR_640
		{459,185},
		{494,185},
		{529,185},
		{564,185},
		{599,185},
	},
	{ // GR_1024
		{841,185},
		{876,185},
		{911,185},
		{946,185},
		{981,185},
	}
};

// special coordinates if only one wing is present
int HUD_wingman_status_single_coords[GR_NUM_RESOLUTIONS][MAX_SHIPS_PER_WING][2] = 
{
	{ // GR_640
		{589,159},				// where to draw dots if we have only one wing present (special case)
		{582,167},
		{596,167},
		{589,175},
		{578,175},
		{600,175},
	}, 
	{ // GR_1024
		{971,159},
		{964,167},
		{978,167},
		{971,175},
		{960,175},
		{982,175},
	}
};

int HUD_wingman_status_coords[GR_NUM_RESOLUTIONS][MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING][2] = 
{
	// we will only ever display up to 5 wings
	{	// GR_640
		// 1 wing present
		{{467,159},						// ship 1
		{460,167},						// ship 2
		{474,167},						// ship 3
		{467,175},						// ship 4
		{456,175},						// ship 5
		{478,175}},						// ship 6

		// 2 wings present
		{{502,159},
		{495,167},
		{509,167},
		{502,175},
		{491,175},
		{513,175}},

		// 3 wings present
		{{537,159},
		{530,167},
		{544,167},
		{537,175},
		{526,175},
		{548,175}},

		// 4 wings present
		{{572,159},
		{565,167},
		{579,167},
		{572,175},
		{561,175},
		{583,175}},
	
		// 5 wings present
		{{607,159},
		{600,167},
		{614,167},
		{607,175},
		{596,175},
		{618,175}},
	}, 
	{	// GR_1024
		{{849,159},
		{842,167},
		{856,167},
		{849,175},
		{838,175},
		{860,175}},

		{{884,159},
		{877,167},
		{891,167},
		{884,175},
		{873,175},
		{895,175}},

		{{919,159},
		{912,167},
		{926,167},
		{919,175},
		{908,175},
		{930,175}},

		{{954,159},
		{947,167},
		{961,167},
		{954,175},
		{943,175},
		{965,175}},
	
		{{989,159},
		{982,167},
		{996,167},
		{989,175},
		{978,175},
		{1000,175}},
	} 
};

// flag a player wing ship as destroyed
void hud_set_wingman_status_dead(int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_index < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_DEAD;
}

// flags a given player wing ship as departed
void hud_set_wingman_status_departed(int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_index < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_NOT_HERE;
}

// flags a given player wing ship as not existing
void hud_set_wingman_status_none( int wing_index, int wing_pos)
{
	int i;

	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_index < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_NONE;

	int used = 0;
	for ( i = 0; i < MAX_SHIPS_PER_WING; i++ ) {
		if ( HUD_wingman_status[wing_index].status[i] != HUD_WINGMAN_STATUS_NONE ) {
			used = 1;
			break;
		}
	}

	HUD_wingman_status[wing_index].used = used;
}

// flags a given player wing ship as "alive" (for multiplayer respawns )
void hud_set_wingman_status_alive( int wing_index, int wing_pos)
{
	Assert(wing_index >= 0 && wing_index < MAX_SQUADRON_WINGS);
	Assert(wing_pos >= 0 && wing_index < MAX_SHIPS_PER_WING);

	HUD_wingman_status[wing_index].status[wing_pos] = HUD_WINGMAN_STATUS_ALIVE;
}

void hud_wingman_status_init_late_wings()
{
/*
	int i, j, wing_index;

	for ( i = 0; i < Num_wings; i++ ) {
		wing_index = ship_squadron_wing_lookup(Wings[i].name);

		if ( (wing_index >= 0) && (Wings[i].total_arrived_count == 0) ) {
			HUD_wingman_status[wing_index].used = 1;
			for (j = 0; j < Wings[i].wave_count; j++) {
				HUD_wingman_status[wing_index].status[j] = HUD_WINGMAN_STATUS_NOT_HERE;
			}
		}
	}
*/
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

	HUD_wingman_status[wing_index].ignore = 1;
}


// called once per level to init the wingman status gauge.  Loads in the frames the first time
void hud_init_wingman_status_gauge()
{
	int	i, j;

	if ( !Wingman_status_gauge_loaded ) {

		for ( i = 0; i < HUD_WINGMAN_STATUS_NUM_FRAMES; i++ ) {
			Wingman_status_frames[i].first_frame = bm_load_animation(Wingman_status_filenames[gr_screen.res][i], &Wingman_status_frames[i].num_frames);
			if ( Wingman_status_frames[i].first_frame == -1 ) {
				Warning(LOCATION, NOX("Error loading Wingman_status_filenames[gr_screen.res][i]'\n"));
				return;
			}
		}
		Wingman_status_gauge_loaded = 1;
	}

	hud_wingman_status_init_flash();

	HUD_wingman_update_timer=timestamp(0);	// update status right away

	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		HUD_wingman_status[i].ignore = 0;
		HUD_wingman_status[i].used = 0;
		for ( j = 0; j < MAX_SHIPS_PER_WING; j++ ) {
			HUD_wingman_status[i].status[j] = HUD_WINGMAN_STATUS_NONE;
		}
	}

	hud_wingman_status_init_late_wings();
	hud_wingman_kill_multi_teams();
	hud_wingman_status_update();
}

// Update the status of the wingman status
void hud_wingman_status_update()
{
	if ( timestamp_elapsed(HUD_wingman_update_timer) ) {
		int		wing_index,wing_pos;
		ship_obj	*so;
		object	*ship_objp;
		ship		*shipp;

		HUD_wingman_update_timer=timestamp(HUD_WINGMAN_UPDATE_STATUS_INTERVAL);

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			ship_objp = &Objects[so->objnum];
			shipp = &Ships[ship_objp->instance];

			wing_index = shipp->wing_status_wing_index;
			wing_pos = shipp->wing_status_wing_pos;

			if ( (wing_index >= 0) && (wing_pos >= 0) ) {

				HUD_wingman_status[wing_index].used = 1;
				if (!(shipp->flags & SF_DEPARTING) ) {
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

void hud_wingman_status_blit_left_frame(int num_wings_to_draw)
{
	int sx, sy, bitmap;

	// draw left side of frame
	if((num_wings_to_draw < 1) || (num_wings_to_draw > 5)){
		Int3();
		return;
	}
	sx = HUD_wingman_left_coords[gr_screen.res][num_wings_to_draw - 1][0];	
	sy = HUD_wingman_left_coords[gr_screen.res][num_wings_to_draw - 1][1];		
	bitmap = Wingman_status_frames[BACKGROUND_LEFT].first_frame;

	if ( bitmap > -1 ) {
		GR_AABITMAP(bitmap, sx, sy);
		// gr_set_bitmap(bitmap);
		// gr_aabitmap(sx, sy);
	}

	// write "wingmen" on gauge
	gr_string(sx+2, sy+2, XSTR( "wingmen", 352));
}

void hud_wingman_status_blit_middle_frame(int num_wings_to_draw)
{
	int sx, sy, bitmap;
	int idx;

	bitmap = Wingman_status_frames[BACKGROUND_MIDDLE].first_frame;
	if ( bitmap < 0 ) {
		return;
	}

	// don't draw for 1 or 2 wings
	if((Num_wings == 1) || (Num_wings == 2)){
		return;
	}

	// draw left side of frame
	if((num_wings_to_draw < 1) || (num_wings_to_draw > 5)){
		Int3();
		return;
	}	
	sx = -1;
	sy = -1;
	for(idx=num_wings_to_draw; idx>=3; idx--){
		sx = HUD_wingman_middle_coords[gr_screen.res][idx - 1][0];	
		sy = HUD_wingman_middle_coords[gr_screen.res][idx - 1][1];		
		GR_AABITMAP(bitmap, sx, sy);	
	}	
}

void hud_wingman_status_blit_right_frame(int num_wings_to_draw)
{
	int sx, sy, bitmap;

	// draw left side of frame
	if((num_wings_to_draw < 1) || (num_wings_to_draw > 5)){
		Int3();
		return;
	}

	sx = HUD_wingman_right_coords[gr_screen.res][num_wings_to_draw - 1][0];	
	sy = HUD_wingman_right_coords[gr_screen.res][num_wings_to_draw - 1][1];			
	bitmap = Wingman_status_frames[BACKGROUND_RIGHT].first_frame;

	if ( bitmap > -1 ) {
		GR_AABITMAP(bitmap, sx, sy);		
	}
}

void hud_wingman_status_blit_dots(int wing_index, int screen_index, int num_wings_to_draw)
{
	int i, sx, sy, is_bright, bitmap = -1, screen_pos;

	// yeah... somebody must have been drunk
	//Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame;

	if ( Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame < 0 ) {
		return;
	}
	
	if ( Wingman_status_frames[WINGMAN_STATUS_NAMES].first_frame < 0 ) {
		return;
	}

	screen_pos = screen_index + (MAX_SQUADRON_WINGS - num_wings_to_draw);
	
	// draw wingman dots
	for ( i = 0; i < MAX_SHIPS_PER_WING; i++ ) {

		if ( hud_wingman_status_maybe_flash(wing_index, i) ) {
			is_bright=1;
		} else {
			is_bright=0;
		}

		switch( HUD_wingman_status[wing_index].status[i] ) {

		case HUD_WINGMAN_STATUS_ALIVE:
			bitmap = Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame;
			if ( HUD_wingman_status[wing_index].hull[i] > 0.5f ) {
				// use gauge color
				hud_set_gauge_color(HUD_WINGMEN_STATUS, is_bright ? HUD_C_BRIGHT : HUD_C_NORMAL);
			} else {
				gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
			}
			break;

		case HUD_WINGMAN_STATUS_DEAD:
			gr_set_color_fast(&Color_red);
			bitmap = Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame+1;
			break;

		case HUD_WINGMAN_STATUS_NOT_HERE:
			hud_set_gauge_color(HUD_WINGMEN_STATUS, is_bright ? HUD_C_BRIGHT : HUD_C_NORMAL);
			bitmap = Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame+1;
			break;

		default:
			bitmap=-1;
			break;

		}	// end swtich

		if ( num_wings_to_draw == 1 ) {
			sx = HUD_wingman_status_single_coords[gr_screen.res][i][0];
			sy = HUD_wingman_status_single_coords[gr_screen.res][i][1]; 
		} else {
			sx = HUD_wingman_status_coords[gr_screen.res][screen_pos][i][0];
			sy = HUD_wingman_status_coords[gr_screen.res][screen_pos][i][1]; 
		}

		if ( bitmap > -1 ) {
			GR_AABITMAP(bitmap, sx, sy);			
		}
	}

	// draw wing name
	// Goober5000 - add 12 to now make these coordinates specify the center rather than the left side
	if ( num_wings_to_draw == 1 ) {
		sx = HUD_wingman_status_single_coords[gr_screen.res][0][0] - 8 + 12;
		sy = HUD_wingman_status_single_coords[gr_screen.res][0][1] + 26;
	} else {
		sx = HUD_wingman_status_name_coords[gr_screen.res][screen_pos][0] + 12;
		sy = HUD_wingman_status_name_coords[gr_screen.res][screen_pos][1]; 
	}
	hud_set_gauge_color(HUD_WINGMEN_STATUS);

	// Goober5000 - get the lowercase abbreviation
	char abbrev[4];
	abbrev[0] = (char) tolower(Squadron_wing_names[wing_index][0]);
	abbrev[1] = (char) tolower(Squadron_wing_names[wing_index][1]);
	abbrev[2] = (char) tolower(Squadron_wing_names[wing_index][2]);
	abbrev[3] = '\0';

	// Goober5000 - center it (round the offset rather than truncate it)
	int abbrev_width;
	gr_get_string_size(&abbrev_width, NULL, abbrev);
	gr_string(sx - (int)((float)abbrev_width/2.0f+0.5f), sy, abbrev);
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
			if ( HUD_wingman_status[i].used > 0 ) {
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

// Draw the wingman status gauge
void hud_wingman_status_render()
{
	int i, count, num_wings_to_draw = 0;

	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		if ( (HUD_wingman_status[i].used > 0) && (HUD_wingman_status[i].ignore == 0) ) {
			num_wings_to_draw++;
		}
	}

	if ( !hud_wingman_status_wingmen_exist(num_wings_to_draw) ) {
		return;
	}

	// hud_set_default_color();
	hud_set_gauge_color(HUD_WINGMEN_STATUS);

	// blit the background frames
	hud_wingman_status_blit_left_frame(num_wings_to_draw);
	hud_wingman_status_blit_middle_frame(num_wings_to_draw);
	hud_wingman_status_blit_right_frame(num_wings_to_draw);

	count = 0;
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		if ( (HUD_wingman_status[i].used <= 0) || (HUD_wingman_status[i].ignore == 1) ) {
			continue;
		}

		hud_wingman_status_blit_dots(i, count, num_wings_to_draw);
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

int hud_wingman_status_wing_pos(int shipnum, int wing_status_index, wing *wingp)
{
	int i, wing_pos = -1;

	for (i = 0; i < wingp->wave_count; i++) {
		if ( wingp->ship_index[i] == shipnum ) {
			wing_pos = i;
			break;
		}
	}

	return wing_pos;
}

void hud_wingman_status_set_index(int shipnum)
{
	int	wing_index, wing_pos;
	ship	*shipp;
	wing	*wingp;

	if ( shipnum < 0 ) {
		return;
	}

	shipp = &Ships[shipnum];

	if ( shipp->wingnum < 0 ) {
		return;
	}

	wingp = &Wings[shipp->wingnum];

	// Check for squadron wings
	wing_index = ship_squadron_wing_lookup(wingp->name);
	if ( wing_index < 0 ) {
		return;
	}

	shipp->wing_status_wing_index = (char)wing_index;

	wing_pos = hud_wingman_status_wing_pos(shipnum, wing_index, wingp);

	shipp->wing_status_wing_pos = (char)wing_pos;
}

void hudwingmanstatus_page_in()
{
	int i;
	for ( i = 0; i < HUD_WINGMAN_STATUS_NUM_FRAMES; i++ ) {
		bm_page_in_aabitmap( Wingman_status_frames[i].first_frame, Wingman_status_frames[i].num_frames );
	}
}

int get_blip_bitmap()
{
	return Wingman_status_frames[WINGMAN_STATUS_DOTS].first_frame+1;
}
