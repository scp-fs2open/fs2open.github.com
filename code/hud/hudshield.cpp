/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudshield.h"
#include "graphics/2d.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "hud/hud.h"
#include "hud/hudparse.h"
#include "hud/hudtargetbox.h"
#include "playerman/player.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "hud/hudescort.h"
#include "weapon/emp.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "render/3d.h"	//For g3_start_frame
#include "network/multi.h"




#define NUM_SHIELD_LEVELS		8

#define SHIELD_TRANSFER_PERCENT	0.083f		// 1/12 total shield strength

#define SHIELD_HIT_DURATION_SHORT	300	// time a shield quadrant flashes after being hit
#define SHIELD_FLASH_INTERVAL_FAST	200	// time between shield quadrant flashes

// CommanderDJ - now dynamic
// #define MAX_SHIELD_ICONS		80	

SCP_vector<SCP_string> Hud_shield_filenames;

SCP_vector<hud_frames> Shield_gauges;

static int Hud_shield_inited = 0;
/*static int Player_shield_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		396, 379
	},
	{ // GR_1024
		634, 670
	}
};

static int Target_shield_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		142, 379
	},
	{ // GR_1024
		292, 670
	}
};*/


int Shield_mini_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		305, 291
	},
	{ // GR_1024
		497, 470
	}
};

// draw on the mini shield icon what the ship integrity is
int Hud_mini_3digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		310, 298, 0
	},
	{ // GR_1024
		502, 477, 0
	}
};
int Hud_mini_2digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		313, 298, 2
	},
	{ // GR_1024
		506, 477, 2
	}
};
int Hud_mini_1digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		316, 298, 6
	},
	{ // GR_1024
		511, 477, 6
	}
};
int Hud_mini_base[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		310, 298
	},
	{ // GR_1024
		502, 477
	}
};
	
int Shield_mini_loaded = 0;
hud_frames Shield_mini_gauge;

#define	SHIELD_HIT_PLAYER		0
#define	SHIELD_HIT_TARGET		1
static shield_hit_info	Shield_hit_data[2];

// translate between clockwise-from-top shield quadrant ordering to way quadrants are numbered in the game
ubyte Quadrant_xlate[MAX_SHIELD_SECTIONS] = {1,0,2,3};

// called at the start of each level from HUD_init.  Use Hud_shield_init so we only init Shield_gauges[] once.
void hud_shield_level_init()
{
	unsigned int i;
	hud_frames temp;

	hud_shield_hit_reset(1);	// reset for the player

	if ( !Hud_shield_inited ) {
		for ( i = 0; i < Hud_shield_filenames.size(); i++ ) {
			Shield_gauges.push_back(temp);
			Shield_gauges.at(i).first_frame = -1;
			Shield_gauges.at(i).num_frames  = 0;
		}
		
		Hud_shield_inited = 1;
	}

	Shield_mini_gauge.first_frame = bm_load_animation("targhit1", &Shield_mini_gauge.num_frames);
	if ( Shield_mini_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the HUD shield ani: targethit1\n");
		return;
	}
	Shield_mini_loaded = 1;
}

int hud_shield_maybe_flash(int gauge, int target_index, int shield_offset)
{
	int					flashed = 0;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[target_index];

	if ( !timestamp_elapsed(shi->shield_hit_timers[shield_offset]) ) {
		if ( timestamp_elapsed(shi->shield_hit_next_flash[shield_offset]) ) {
			shi->shield_hit_next_flash[shield_offset] = timestamp(SHIELD_FLASH_INTERVAL_FAST);
			shi->shield_show_bright ^= (1<<shield_offset);	// toggle between default and bright frames
		}

		if ( shi->shield_show_bright & (1<<shield_offset) ) {
			// hud_set_bright_color();
			hud_set_gauge_color(gauge, HUD_C_BRIGHT);
			flashed = 1;
		} else {
			hud_set_gauge_color(gauge, HUD_C_NORMAL);
			// hud_set_default_color();
		}
	}

	return flashed;
}

extern int Cmdline_nohtl;
bool shield_ani_warning_displayed_already = false;

// called at beginning of level to page in all ship icons
// used in this level
void hud_ship_icon_page_in(ship_info *sip)
{
	hud_frames	*sgp;

	if ( sip->shield_icon_index == 255 ) {
		return;
	}

	// load in shield frames if not already loaded
	Assert(sip->shield_icon_index < (ubyte)Hud_shield_filenames.size());
	sgp = &Shield_gauges.at(sip->shield_icon_index);

	if ( sgp->first_frame == -1 ) {
		sgp->first_frame = bm_load_animation(const_cast<char*>(Hud_shield_filenames.at(sip->shield_icon_index).c_str()), &sgp->num_frames);
		if ( sgp->first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames.at(sip->shield_icon_index).c_str());
			return;
		}
	}

	int i;
	for (i=0; i<sgp->num_frames; i++ )	{
		bm_page_in_aabitmap(sgp->first_frame+i, 1);
	}

}

// ------------------------------------------------------------------
// hud_shield_equalize()
//
// Equalize the four shield quadrants for an object
//
void hud_shield_equalize(object *objp, player *pl)
{
	float strength;
	int idx;
	int all_equal;

	Assert(objp != NULL);
	if (objp == NULL)
		return;

	Assert(pl != NULL);
	if (pl == NULL)
		return;

	Assert(objp->type == OBJ_SHIP);
	if (objp->type != OBJ_SHIP)
		return;

	// Goober5000 - quick out if we have no shields
	if (objp->flags & OF_NO_SHIELDS)
		return;

	// are all quadrants equal?
	all_equal = 1;
	for (idx = 0; idx < MAX_SHIELD_SECTIONS - 1; idx++) {
		if (objp->shield_quadrant[idx] != objp->shield_quadrant[idx + 1]) {
			all_equal = 0;
			break;
		}
	}

	if (all_equal)
		return;

	strength = shield_get_strength(objp);
	if (strength == 0.0f)
		return;

	// maybe impose a 2% penalty - server side and single player only
	if (!MULTIPLAYER_CLIENT && ((pl->shield_penalty_stamp < 0) || timestamp_elapsed_safe(pl->shield_penalty_stamp, 1000)) ) {
		strength *= 0.98f;

		// reset the penalty timestamp
		pl->shield_penalty_stamp = timestamp(1000);
	}
			
	shield_set_strength(objp, strength);					

	// beep
	if (objp == Player_obj) {
		snd_play(&Snds[SND_SHIELD_XFER_OK]);
	}
}

// ------------------------------------------------------------------
// hud_augment_shield_quadrant()
//
// Transfer shield energy to a shield quadrant from the three other
//	quadrants.  Works by trying to transfer a fixed amount of shield
//	energy from the other three quadrants, taking the same percentage
// from each quadrant.
//
//	input:	objp			=>		object to perform shield transfer on
//				direction	=>		which quadrant to augment:
//										0 - right
//										1 - top
//										2 - bottom
//										3 - left
//
void hud_augment_shield_quadrant(object *objp, int direction)
{
	float	xfer_amount, energy_avail, percent_to_take, delta;
	float	max_quadrant_val;
	int	i;

	Assert(direction >= 0 && direction < MAX_SHIELD_SECTIONS);
	Assert(objp->type == OBJ_SHIP);
	
	xfer_amount = Ships[objp->instance].ship_max_shield_strength * SHIELD_TRANSFER_PERCENT;
	max_quadrant_val = get_max_shield_quad(objp);

	if ( (objp->shield_quadrant[direction] + xfer_amount) > max_quadrant_val )
		xfer_amount = max_quadrant_val - objp->shield_quadrant[direction];

	Assert(xfer_amount >= 0);
	if ( xfer_amount == 0 ) {
		// TODO: provide a feedback sound
		return;
	}
	else {
		snd_play( &Snds[SND_SHIELD_XFER_OK] );
	}

	energy_avail = 0.0f;
	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
		if ( i == direction )
			continue;
		energy_avail += objp->shield_quadrant[i];
	}

	percent_to_take = xfer_amount/energy_avail;
	if ( percent_to_take > 1.0f )
		percent_to_take = 1.0f;

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
		if ( i == direction )
			continue;
		delta = percent_to_take * objp->shield_quadrant[i];
		objp->shield_quadrant[i] -= delta;
		Assert(objp->shield_quadrant[i] >= 0 );
		objp->shield_quadrant[direction] += delta;
		if ( objp->shield_quadrant[direction] > max_quadrant_val )
			objp->shield_quadrant[direction] = max_quadrant_val;
	}
}

// Try to find a match between filename and the names inside
// of Hud_shield_filenames.  This will provide us with an 
// association of ship class to shield icon information.
void hud_shield_assign_info(ship_info *sip, char *filename)
{
	ubyte i;

	for ( i = 0; i < (ubyte)Hud_shield_filenames.size(); i++ ) {
		if ( !stricmp(filename, Hud_shield_filenames.at(i).c_str()) ) {
			sip->shield_icon_index = i;
			return;
		}
	}

	//No HUD icon found. Add one!
	sip->shield_icon_index = (unsigned char) Hud_shield_filenames.size();
	Hud_shield_filenames.push_back((SCP_string)filename);
}

void hud_show_mini_ship_integrity(object *objp, int x_force, int y_force)
{
	char	text_integrity[64];
	int	numeric_integrity;
	float p_target_integrity;
	int	final_pos[2];

	p_target_integrity = get_hull_pct(objp);

	numeric_integrity = fl2i(p_target_integrity*100 + 0.5f);
	if(numeric_integrity > 100){
		numeric_integrity = 100;
	}
	// Assert(numeric_integrity <= 100);

	// 3 digit hull strength
	if ( numeric_integrity == 100 ) {
		final_pos[0] = x_force + Hud_mini_3digit[gr_screen.res][0] - Shield_mini_coords[gr_screen.res][0];
		final_pos[1] = y_force + Hud_mini_3digit[gr_screen.res][1] - Shield_mini_coords[gr_screen.res][1];
	} 
	// 1 digit hull strength
	else if ( numeric_integrity < 10 ) {
		final_pos[0] = x_force + Hud_mini_1digit[gr_screen.res][0] - Shield_mini_coords[gr_screen.res][0];
		final_pos[1] = y_force + Hud_mini_1digit[gr_screen.res][1] - Shield_mini_coords[gr_screen.res][1];
	}
	// 2 digit hull strength
	else {
		final_pos[0] = x_force + Hud_mini_2digit[gr_screen.res][0] - Shield_mini_coords[gr_screen.res][0];
		final_pos[1] = y_force + Hud_mini_2digit[gr_screen.res][1] - Shield_mini_coords[gr_screen.res][1];
	}	

	if ( numeric_integrity == 0 ) {
		if ( p_target_integrity > 0 ) {
			numeric_integrity = 1;
		}
	}

	sprintf(text_integrity, "%d", numeric_integrity);
	if ( numeric_integrity < 100 ) {
		hud_num_make_mono(text_integrity);
	}	

	gr_string(final_pos[0], final_pos[1], text_integrity);
}

// Draw the miniature shield icon that is drawn near the reticle
// this function is only used by multi_ingame_join_display_ship() in multi_ingame.cpp as of the new HudGauge implementation (Swifty)
void hud_shield_show_mini(object *objp, int x_force, int y_force, int x_hull_offset, int y_hull_offset)
{
	float			max_shield;
	int			hud_color_index, range, frame_offset;
	int			sx, sy, i;

	if ( objp->type != OBJ_SHIP ) {
		return;
	}

	hud_set_gauge_color(HUD_TARGET_MINI_ICON);

	if (!Shield_mini_loaded)
		return;

	sx = (x_force == -1) ? Shield_mini_coords[gr_screen.res][0]+fl2i(HUD_offset_x) : x_force;
	sy = (y_force == -1) ? Shield_mini_coords[gr_screen.res][1]+fl2i(HUD_offset_y) : y_force;

	// draw the ship first
	hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_HIT_TARGET, HULL_HIT_OFFSET);
	hud_show_mini_ship_integrity(objp, x_force + x_hull_offset,y_force + y_hull_offset);

	// draw the four quadrants
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		if ( hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_HIT_TARGET, i) ) {
			frame_offset = i+MAX_SHIELD_SECTIONS;
		} else {
			frame_offset = i;
		}
				
		range = HUD_color_alpha;
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range + 0.5);
		Assert(hud_color_index >= 0 && hud_color_index <= range);
	
		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		if ( hud_gauge_maybe_flash(HUD_TARGET_MINI_ICON) == 1) {
			// hud_set_bright_color();
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, HUD_C_BRIGHT);
		} else {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, hud_color_index);
		}					 

		GR_AABITMAP(Shield_mini_gauge.first_frame + frame_offset, sx + HUD_nose_x, sy + HUD_nose_y);		
	}
	
	// hud_set_default_color();
}

// reset the shield_hit_info data structure
void shield_info_reset(shield_hit_info *shi)
{
	int i;

	shi->shield_hit_status = 0;
	shi->shield_show_bright = 0;
	for ( i = 0; i < NUM_SHIELD_HIT_MEMBERS; i++ ) {
		shi->shield_hit_timers[i] = 1;
		shi->shield_hit_next_flash[i] = 1;
	}
}

// reset the timers and hit flags for the shield gauges
//
// This needs to be called whenever the player selects a new target
//
// input:	player	=>	optional parameter (default value 0).  This is to indicate that player shield hit
//								info should be reset.  This is normally not the case.
//								is for the player's current target
void hud_shield_hit_reset(int player)
{
	shield_hit_info	*shi;

	if (player) {
		shi = &Shield_hit_data[SHIELD_HIT_PLAYER];
	} else {
		shi = &Shield_hit_data[SHIELD_HIT_TARGET];
	}

	shield_info_reset(shi);
}

// called once per frame to update the state of Shield_hit_status based on the Shield_hit_timers[]
void hud_shield_hit_update()
{
	int i, j, limit;		

	limit = 1;
	if ( Player_ai->target_objnum >= 0 ) {
		limit = 2;
	}

	for ( i = 0; i < limit; i++ ) {
		for ( j = 0; j < NUM_SHIELD_HIT_MEMBERS; j++ ) {
			if ( timestamp_elapsed(Shield_hit_data[i].shield_hit_timers[j]) ) {
				Shield_hit_data[i].shield_hit_status &= ~(1<<j);
				Shield_hit_data[i].shield_show_bright &= ~(1<<j);
			} else { 
				if ( timestamp_elapsed(Shield_hit_data[i].shield_hit_next_flash[j]) ) {
					Shield_hit_data[i].shield_hit_next_flash[j] = timestamp(SHIELD_FLASH_INTERVAL_FAST);
					Shield_hit_data[i].shield_show_bright ^= (1<<j);	// toggle between default and bright frames
				}
			}
		}
	}
}

// called when a shield quadrant is struct, so we can update the timer that will draw the quadrant
// as flashing
//
// input:
//				objp		=>	object pointer for ship that has been hit
//				quadrant	=> quadrant of shield getting hit (-1 if no shield is present)
void hud_shield_quadrant_hit(object *objp, int quadrant)
{
	shield_hit_info	*shi;
	int					num;

	if ( objp->type != OBJ_SHIP )
		return;

	hud_escort_ship_hit(objp, quadrant);
	hud_gauge_popup_start(HUD_TARGET_MINI_ICON);

	if ( OBJ_INDEX(objp) == Player_ai->target_objnum ) {
		shi = &Shield_hit_data[SHIELD_HIT_TARGET];
	} else if ( objp == Player_obj ) {
		shi = &Shield_hit_data[SHIELD_HIT_PLAYER];
	} else {
		return;
	}

	if ( quadrant >= 0 ) {
		num = Quadrant_xlate[quadrant];
		shi->shield_hit_timers[num] = timestamp(SHIELD_HIT_DURATION_SHORT);
	} else {
		shi->shield_hit_timers[HULL_HIT_OFFSET] = timestamp(SHIELD_HIT_DURATION_SHORT);
		hud_targetbox_start_flash(TBOX_FLASH_HULL); 
	}
}

HudGaugeShield::HudGaugeShield():
HudGauge(HUD_OBJECT_PLAYER_SHIELD, HUD_PLAYER_SHIELD_ICON, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

HudGaugeShield::HudGaugeShield(int _gauge_object, int _gauge_config):
HudGauge(_gauge_object, _gauge_config, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeShield::render(float frametime)
{
}

void HudGaugeShield::showShields(object *objp, int mode)
{
//	static int fod_model = -1;
	float			max_shield;
	int			hud_color_index, range;
	int			sx, sy, i;
	ship			*sp;
	ship_info	*sip;
	hud_frames	*sgp=NULL;

	if ( objp->type != OBJ_SHIP )
		return;

	// Goober5000 - don't show if primitive sensors
	if ( Ships[Player_obj->instance].flags2 & SF2_PRIMITIVE_SENSORS )
		return;

	sp = &Ships[objp->instance];
	sip = &Ship_info[sp->ship_info_index];

//	bool digitus_improbus = (fod_model != -2 && strstr(sp->ship_name, "Sathanas") != NULL);
	if ( sip->shield_icon_index == 255 && !(sip->flags2 & SIF2_GENERATE_HUD_ICON) /*&& !digitus_improbus*/) {
		return;
	}

	setGaugeColor();

	// load in shield frames if not already loaded
	if (sip->shield_icon_index != 255) {
		sgp = &Shield_gauges.at(sip->shield_icon_index);

		if ( (sgp->first_frame == -1) && (sip->shield_icon_index < Hud_shield_filenames.size()) ) {
			sgp->first_frame = bm_load_animation(const_cast<char*>(Hud_shield_filenames.at(sip->shield_icon_index).c_str()), &sgp->num_frames);
			if (sgp->first_frame == -1) {
				if (!shield_ani_warning_displayed_already) {
					shield_ani_warning_displayed_already = true;
					Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames.at(sip->shield_icon_index).c_str());
				}
				return;
			}
		}
	}

	sx = position[0];
	sy = position[1];

	sx += fl2i(HUD_offset_x);
	sy += fl2i(HUD_offset_y);

	// draw the ship first
	maybeFlashShield(SHIELD_HIT_PLAYER, HULL_HIT_OFFSET);

	if(sip->shield_icon_index != 255)
	{
		renderBitmap(sgp->first_frame, sx, sy);
	}
	else
	{
		bool g3_yourself = !g3_in_frame();
		angles rot_angles = {-1.570796327f,0.0f,0.0f};
		matrix	object_orient;

		vm_angles_2_matrix(&object_orient, &rot_angles);

		gr_screen.clip_width = 112;
		gr_screen.clip_height = 93;

		//Fire it up
		if(g3_yourself)
			g3_start_frame(1);
		hud_save_restore_camera_data(1);
		setClip(sx, sy, 112, 93);
		model_set_detail_level(1);

		//if(!digitus_improbus)
			g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 2.5f);
		/*else
		{
			vec3d finger_vec = {0.0f, 0.0f, 176.0f};
			g3_set_view_matrix( &finger_vec, &vmd_identity_matrix, 1.0f);
		}*/

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(0.5f*Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}

		//We're ready to show stuff
		ship_model_start(objp);
		//if(!digitus_improbus)
		{
			model_render( sip->model_num, &object_orient, &vmd_zero_vector, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1, sp->ship_replacement_textures);
		}
		/*else
		{
			if(fod_model == -1)
			{
				fod_model = model_load(NOX("FoD.pof"), 0, NULL);
				if(fod_model == -1)
				{
					fod_model = -2;
					return;
				}
			}
			model_render(fod_model, &object_orient, &vmd_zero_vector, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1);
		}*/
		ship_model_stop( objp );

		//We're done
		if(!Cmdline_nohtl)
		{
			gr_end_view_matrix();
			gr_end_proj_matrix();
		}
		if(g3_yourself)
			g3_end_frame();
		hud_save_restore_camera_data(0);

		resetClip();
	}

	if(!sip->max_shield_strength)
		return;

	// draw the four quadrants
	//
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);

	coord2d shield_icon_coords[6];

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		range = MAX(HUD_COLOR_ALPHA_MAX, HUD_color_alpha + 4);
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range);
		Assert(hud_color_index >= 0 && hud_color_index <= range);

		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		int flash=0;
		flash = maybeFlashShield(mode, i);
		
				
		if ( !flash ) {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			setGaugeColor(hud_color_index);
			

			if(sip->shield_icon_index != 255)
			{
				renderBitmap(sgp->first_frame+i+1, sx, sy);
			}
			else
			{
				//Ugh, draw four shield quadrants
				static const int TRI_EDGE = 6;
				static const int BAR_LENGTH = 112;
				static const int BAR_HEIGHT = 54;
				static const int BAR_WIDTH = 6;
				static const int SHIELD_OFFSET = BAR_WIDTH + TRI_EDGE + 3;

				switch(i)
				{
					//Top
					case 0:
						shield_icon_coords[0].x = sx;                     shield_icon_coords[0].y = sy+BAR_WIDTH+TRI_EDGE;
						shield_icon_coords[1].x = sx;                     shield_icon_coords[1].y = sy;
						shield_icon_coords[2].x = sx+TRI_EDGE;            shield_icon_coords[2].y = sy+BAR_WIDTH;
						shield_icon_coords[3].x = sx+BAR_LENGTH;          shield_icon_coords[3].y = sy;
						shield_icon_coords[4].x = sx+BAR_LENGTH-TRI_EDGE; shield_icon_coords[4].y = sy+BAR_WIDTH;
						shield_icon_coords[5].x = sx+BAR_LENGTH;          shield_icon_coords[5].y = sy+BAR_WIDTH+TRI_EDGE;
						renderShieldIcon(shield_icon_coords);
						break;
					//Left
					case 3:
						sy += SHIELD_OFFSET;
						shield_icon_coords[0].x = sx+BAR_WIDTH+TRI_EDGE; shield_icon_coords[0].y = sy+BAR_HEIGHT;
						shield_icon_coords[1].x = sx;                    shield_icon_coords[1].y = sy+BAR_HEIGHT;
						shield_icon_coords[2].x = sx+BAR_WIDTH;          shield_icon_coords[2].y = sy+BAR_HEIGHT-TRI_EDGE;
						shield_icon_coords[3].x = sx;                    shield_icon_coords[3].y = sy;
						shield_icon_coords[4].x = sx+BAR_WIDTH;          shield_icon_coords[4].y = sy+TRI_EDGE;
						shield_icon_coords[5].x = sx+BAR_WIDTH+TRI_EDGE; shield_icon_coords[5].y = sy;
						renderShieldIcon(shield_icon_coords);
						sy -= SHIELD_OFFSET + BAR_WIDTH + TRI_EDGE;
						break;
					//Right
					case 1:
						sx += BAR_LENGTH;
						sy += SHIELD_OFFSET;
						shield_icon_coords[0].x = sx-BAR_WIDTH-TRI_EDGE; shield_icon_coords[0].y = sy;
						shield_icon_coords[1].x = sx;                    shield_icon_coords[1].y = sy;
						shield_icon_coords[2].x = sx-BAR_WIDTH;          shield_icon_coords[2].y = sy+TRI_EDGE;
						shield_icon_coords[3].x = sx;                    shield_icon_coords[3].y = sy+BAR_HEIGHT;
						shield_icon_coords[4].x = sx-BAR_WIDTH;          shield_icon_coords[4].y = sy+BAR_HEIGHT-TRI_EDGE;
						shield_icon_coords[5].x = sx-BAR_WIDTH-TRI_EDGE; shield_icon_coords[5].y = sy+BAR_HEIGHT;
						renderShieldIcon(shield_icon_coords);
						sx -= BAR_LENGTH;
						sy -= SHIELD_OFFSET;
						break;
					//Bottom
					case 2:
						sy += BAR_HEIGHT + SHIELD_OFFSET*2 - BAR_WIDTH - TRI_EDGE;
						shield_icon_coords[0].x = sx+BAR_LENGTH;          shield_icon_coords[0].y = sy;
						shield_icon_coords[1].x = sx+BAR_LENGTH;          shield_icon_coords[1].y = sy+BAR_WIDTH+TRI_EDGE;
						shield_icon_coords[2].x = sx+BAR_LENGTH-TRI_EDGE; shield_icon_coords[2].y = sy+TRI_EDGE;
						shield_icon_coords[3].x = sx;                     shield_icon_coords[3].y = sy+BAR_WIDTH+TRI_EDGE;
						shield_icon_coords[4].x = sx+TRI_EDGE;            shield_icon_coords[4].y = sy+TRI_EDGE;
						shield_icon_coords[5].x = sx;                     shield_icon_coords[5].y = sy;
						renderShieldIcon(shield_icon_coords);
						sy -= BAR_HEIGHT + SHIELD_OFFSET*2 - BAR_WIDTH - TRI_EDGE;
						break;
					//Whoops?
					default:
						nprintf(("HUD", "Invalid shield quadrant %d specified!\n", i));
						break;
				}
			}
		}
	}

	// hud_set_default_color();
}

/*
 * Render a shield icon - basic shape is:
 *   1                             3
 *     ***************************
 *     ***************************
 *     ** 2                   4 **
 *     *                         *
 *     0                         5
 *
 * Defined by 6 points, must be passed in the order show above (i.e. a valid triangle strip)
 *
 */
void HudGaugeShield::renderShieldIcon(coord2d coords[6])
{
	int nx = 0, ny = 0, i;

	if ( gr_screen.rendering_to_texture != -1 ) {
		gr_set_screen_scale(canvas_w, canvas_h, target_w, target_h);
	} else {
		if ( reticle_follow ) {
			nx = HUD_nose_x;
			ny = HUD_nose_y;

			gr_resize_screen_pos(&nx, &ny);
			gr_set_screen_scale(base_w, base_h);
			gr_unsize_screen_pos(&nx, &ny);
		} else {
			gr_set_screen_scale(base_w, base_h);
		}
	}

	for (i = 0; i < 6; ++i) {
		coords[i].x += nx;
		coords[i].y += ny;
	}

	gr_shield_icon(coords);
	gr_reset_screen_scale();
}

int HudGaugeShield::maybeFlashShield(int target_index, int shield_offset)
{
	int	flashed = 0;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[target_index];

	if ( !timestamp_elapsed(shi->shield_hit_timers[shield_offset]) ) {
		if ( shi->shield_show_bright & (1<<shield_offset) ) {
			// hud_set_bright_color();
			setGaugeColor(HUD_C_BRIGHT);
			flashed = 1;
		} else {
			setGaugeColor(HUD_C_NORMAL);
			// hud_set_default_color();
		}
	}

	return flashed;
}

HudGaugeShieldPlayer::HudGaugeShieldPlayer():
HudGaugeShield(HUD_OBJECT_PLAYER_SHIELD, HUD_PLAYER_SHIELD_ICON)
{
}

void HudGaugeShieldPlayer::render(float frametime)
{
	showShields(Player_obj, SHIELD_HIT_PLAYER);
}

HudGaugeShieldTarget::HudGaugeShieldTarget():
HudGaugeShield(HUD_OBJECT_TARGET_SHIELD, HUD_TARGET_SHIELD_ICON)
{

}

void HudGaugeShieldTarget::render(float frametime)
{
	if (Player_ai->target_objnum == -1)
		return;

	object *targetp = &Objects[Player_ai->target_objnum];
	
	// check to see if there is even a current target
	if ( targetp == &obj_used_list ) {
		return;
	}

	if ( targetp == Player_obj)
		return;

	showShields(targetp, SHIELD_HIT_TARGET);
}

HudGaugeShieldMini::HudGaugeShieldMini(): // HUD_TARGET_MINI_ICON
HudGauge(HUD_OBJECT_MINI_SHIELD, HUD_TARGET_MINI_ICON, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeShieldMini::init3DigitOffsets(int x, int y)
{
	Mini_3digit_offsets[0] = x;
	Mini_3digit_offsets[1] = y;
}

void HudGaugeShieldMini::init1DigitOffsets(int x, int y)
{
	Mini_1digit_offsets[0] = x;
	Mini_1digit_offsets[1] = y;
}

void HudGaugeShieldMini::init2DigitOffsets(int x, int y)
{
	Mini_2digit_offsets[0] = x;
	Mini_2digit_offsets[1] = y;
}

void HudGaugeShieldMini::initBitmaps(char *fname)
{
	Shield_mini_gauge.first_frame = bm_load_animation(fname, &Shield_mini_gauge.num_frames);
	if ( Shield_mini_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", fname);
	}
}

void HudGaugeShieldMini::render(float frametime)
{
	if (Player_ai->target_objnum == -1)
		return;

	object *targetp = &Objects[Player_ai->target_objnum];
	
	// check to see if there is even a current target
	if ( targetp == &obj_used_list ) {
		return;
	}

	showMiniShields(targetp);
}

void HudGaugeShieldMini::pageIn()
{
	bm_page_in_aabitmap( Shield_mini_gauge.first_frame, Shield_mini_gauge.num_frames );
}

// Draw the miniature shield icon that is drawn near the reticle
void HudGaugeShieldMini::showMiniShields(object *objp)
{
	float			max_shield;
	int			hud_color_index, range, frame_offset;
	int			sx, sy, i;

	if ( objp->type != OBJ_SHIP ) {
		return;
	}

	setGaugeColor();

	sx = position[0]+fl2i(HUD_offset_x);
	sy = position[1]+fl2i(HUD_offset_y);

	// draw the ship first
	maybeFlashShield(SHIELD_HIT_TARGET, HULL_HIT_OFFSET);
	showIntegrity(get_hull_pct(objp));

	// draw the four quadrants
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		if ( maybeFlashShield(SHIELD_HIT_TARGET, i) ) {
			frame_offset = i+MAX_SHIELD_SECTIONS;
		} else {
			frame_offset = i;
		}
				
		range = HUD_color_alpha;
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range + 0.5);
		Assert(hud_color_index >= 0 && hud_color_index <= range);
	
		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		if ( maybeFlashSexp() == 1) {
			// hud_set_bright_color();
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			setGaugeColor(hud_color_index);
		}					 

		renderBitmap(Shield_mini_gauge.first_frame + frame_offset, sx, sy);		
	}
	
	// hud_set_default_color();
}

void HudGaugeShieldMini::showIntegrity(float p_target_integrity)
{
	char	text_integrity[64];
	int	numeric_integrity;
	int	final_pos[2];

	numeric_integrity = fl2i(p_target_integrity*100 + 0.5f);
	if(numeric_integrity > 100){
		numeric_integrity = 100;
	}
	// Assert(numeric_integrity <= 100);

	// 3 digit hull strength
	if ( numeric_integrity == 100 ) {
		memcpy(final_pos, Mini_3digit_offsets, sizeof(final_pos));
	} 
	// 1 digit hull strength
	else if ( numeric_integrity < 10 ) {
		memcpy(final_pos, Mini_1digit_offsets, sizeof(final_pos));		
	}
	// 2 digit hull strength
	else {
		memcpy(final_pos, Mini_2digit_offsets, sizeof(final_pos));
	}	

	if ( numeric_integrity == 0 ) {
		if ( p_target_integrity > 0 ) {
			numeric_integrity = 1;
		}
	}

	final_pos[0] += fl2i( HUD_offset_x ) + position[0];
	final_pos[1] += fl2i( HUD_offset_y ) + position[1];

	sprintf(text_integrity, "%d", numeric_integrity);
	if ( numeric_integrity < 100 ) {
		hud_num_make_mono(text_integrity, font_num);
	}	

	renderString(final_pos[0], final_pos[1], text_integrity);
}

int HudGaugeShieldMini::maybeFlashShield(int target_index, int shield_offset)
{
	int	flashed = 0;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[target_index];

	if ( !timestamp_elapsed(shi->shield_hit_timers[shield_offset]) ) {
		if ( shi->shield_show_bright & (1<<shield_offset) ) {
			// hud_set_bright_color();
			setGaugeColor(HUD_C_BRIGHT);
			flashed = 1;
		} else {
			setGaugeColor(HUD_C_NORMAL);
			// hud_set_default_color();
		}
	}

	return flashed;
}
