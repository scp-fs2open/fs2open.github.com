/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "gamesnd/gamesnd.h"
#include "hud/hudescort.h"
#include "hud/hudparse.h"
#include "hud/hudshield.h"
#include "hud/hudtargetbox.h"
#include "io/timer.h"
#include "network/multi.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "render/3d.h"	//For g3_start_frame
#include "ship/ship.h"
#include "weapon/emp.h"
#include "graphics/matrix.h"




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

static shield_hit_info	Shield_hit_data[num_shield_gauge_types];

// translate between clockwise-from-top shield quadrant ordering to way quadrants are numbered in the game
ubyte Quadrant_xlate[DEFAULT_SHIELD_SECTIONS] = {1,0,2,3};

// called at the start of each level from HUD_init.  Use Hud_shield_init so we only init Shield_gauges[] once.
void hud_shield_level_init()
{
	unsigned int i;
	hud_frames temp;

	hud_shield_hit_reset(Player_obj, 1);	// reset for the player

	if ( !Hud_shield_inited ) {
		for ( i = 0; i < Hud_shield_filenames.size(); i++ ) {
			Shield_gauges.push_back(temp);
			Shield_gauges.at(i).first_frame = -1;
			Shield_gauges.at(i).num_frames  = 0;
		}
		
		Hud_shield_inited = 1;
	}

	// the shield mini gauge is loaded in HudGaugeShieldMini::pageIn()
	// though it wouldn't hurt to keep this old failsafe code here
	// since we are setting the status of if the mini shield was loaded
	// --wookieejedi
	if ( Shield_mini_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the HUD shield ani \n");
		return;
	}
	Shield_mini_loaded = 1;
}

int hud_shield_maybe_flash(int gauge, int target_index, int shield_offset)
{
	int					flashed = 0;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[target_index];

	if ( shi->shield_hit_timers.empty() ) {
		return 0;
	}

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

bool shield_ani_warning_displayed_already = false;

// called at beginning of level to page in all ship icons
// used in this level
void hud_ship_icon_page_in(const ship_info *sip)
{
	hud_frames	*sgp;

	if ( sip->shield_icon_index == 255 ) {
		return;
	}

	// load in shield frames if not already loaded
	Assert(sip->shield_icon_index < (ubyte)Hud_shield_filenames.size());
	sgp = &Shield_gauges.at(sip->shield_icon_index);

	if ( sgp->first_frame == -1 ) {
		sgp->first_frame = bm_load_animation(Hud_shield_filenames.at(sip->shield_icon_index).c_str(), &sgp->num_frames);
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
// Equalize all shield quadrants for an object
//
void hud_shield_equalize(object *objp, player *pl)
{
	float penalty;

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
	if (objp->flags[Object::Object_Flags::No_shields])
		return;

	// maybe impose a 2% penalty - server side and single player only
	if (!MULTIPLAYER_CLIENT && ((pl->shield_penalty_stamp < 0) || timestamp_elapsed_safe(pl->shield_penalty_stamp, 1000)) ) {
		penalty = 0.02f;

		// reset the penalty timestamp
		pl->shield_penalty_stamp = timestamp(1000);

	} else {
		penalty = 0.0f;
	}

	shield_balance(objp, 1, penalty);

	// beep
	if (objp == Player_obj) {
		snd_play(gamesnd_get_game_sound(GameSounds::SHIELD_XFER_OK));
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
	Assertion((direction >= 0) && (direction < 4), "Invalid quadrant index %i!", direction);

	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	if (sip->flags[Ship::Info_Flags::Model_point_shields]) {
		// Using model point shields, so map to the correct quadrant
		direction = sip->shield_point_augment_ctrls[direction];

		if (direction < 0) {
			// This quadrant cannot be augmented, ignore request and bail
			return;
		}
	}	// Else, using standard shields.

	shield_transfer(objp, direction, SHIELD_TRANSFER_PERCENT);
}

// Try to find a match between filename and the names inside
// of Hud_shield_filenames.  This will provide us with an 
// association of ship class to shield icon information.
void hud_shield_assign_info(ship_info *sip, const char *filename)
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
	Hud_shield_filenames.emplace_back(filename);
}

void hud_show_mini_ship_integrity(const object *objp, int x_force, int y_force)
{
	char	text_integrity[64];
	int	numeric_integrity;
	float p_target_integrity;
	int	final_pos[2];

	p_target_integrity = get_hull_pct(objp);

	numeric_integrity = (int)std::lround(p_target_integrity * 100);
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
void hud_shield_show_mini(const object *objp, int x_force, int y_force, int x_hull_offset, int y_hull_offset)
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

	sx = (x_force == -1) ? Shield_mini_coords[gr_screen.res][0] : x_force;
	sy = (y_force == -1) ? Shield_mini_coords[gr_screen.res][1] : y_force;

	// draw the ship first
	hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_GAUGE_TARGET, Shield_hit_data[SHIELD_GAUGE_TARGET].hull_hit_index);
	hud_show_mini_ship_integrity(objp, x_force + x_hull_offset,y_force + y_hull_offset);

	// draw the four quadrants
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = shield_get_max_quad(objp);

	int n_quadrants = static_cast<int>(objp->shield_quadrant.size());
	for ( i = 0; i < n_quadrants; i++ ) {

		if ( objp->flags[Object::Object_Flags::No_shields] || i >= DEFAULT_SHIELD_SECTIONS) {
			break;
		}

		int num;
		if (!(Ship_info[Ships[objp->instance].ship_info_index].flags[Ship::Info_Flags::Model_point_shields]))
			num = Quadrant_xlate[i];
		else
			num = i;

		if ( (max_shield > 0.0f) && (objp->shield_quadrant[num]/max_shield < Shield_percent_skips_damage) ) {
			continue;
		}

		if ( hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_GAUGE_TARGET, i) ) {
			frame_offset = i+n_quadrants;
		} else {
			frame_offset = i;
		}
				
		range = HUD_color_alpha;
		hud_color_index = static_cast<int>(std::lround((objp->shield_quadrant[num] / max_shield) * range));
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
// pass NULL as objp if you only need to initialize a shield_hit_info without an
// associated ship
void shield_info_reset(const object *objp, shield_hit_info *shi)
{
	int n_quadrants = (objp != nullptr) ? static_cast<int>(objp->shield_quadrant.size()) : 0;

	shi->members = n_quadrants + 1;
	shi->hull_hit_index = n_quadrants;
	shi->shield_hit_timers.resize(shi->members);
	shi->shield_hit_next_flash.resize(shi->members);

	for ( int i = 0; i < shi->members; i++ ) {
		shi->shield_hit_timers[i] = timestamp(0);
		shi->shield_hit_next_flash[i] = timestamp(0);
	}

	shi->shield_hit_status = 0;
	shi->shield_show_bright = 0;
}

// reset the timers and hit flags for the shield gauges
//
// This needs to be called whenever the player selects a new target
//
// input:	player	=>	optional parameter (default value 0).  This is to indicate that player shield hit
//								info should be reset.  This is normally not the case.
//								is for the player's current target
void hud_shield_hit_reset(const object *objp, int player)
{
	shield_hit_info	*shi;

	if (player) {
		shi = &Shield_hit_data[SHIELD_GAUGE_PLAYER];
	} else {
		shi = &Shield_hit_data[SHIELD_GAUGE_TARGET];
	}

	shield_info_reset(objp, shi);
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
		for ( j = 0; j < Shield_hit_data[i].members; j++ ) {
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
void hud_shield_quadrant_hit(const object *objp, int quadrant)
{
	shield_hit_info	*shi;
	int					num;

	if (Game_mode & GM_STANDALONE_SERVER)
		return;

	Assertion(objp != NULL, "hud_shield_quadrant_hit() called with a NULL objp; get a coder!\n");

	if ( objp->type != OBJ_SHIP )
		return;

	hud_escort_ship_hit(objp, quadrant);
	hud_gauge_popup_start(HUD_TARGET_MINI_ICON);

	if ( OBJ_INDEX(objp) == Player_ai->target_objnum ) {
		shi = &Shield_hit_data[SHIELD_GAUGE_TARGET];
	} else if ( objp == Player_obj ) {
		shi = &Shield_hit_data[SHIELD_GAUGE_PLAYER];
	} else {
		return;
	}

	Assertion(!shi->shield_hit_timers.empty(), "Shield hit info object for object '%s' has a size " SIZE_T_ARG " shield_hit_timers; get a coder!\n", Ships[objp->instance].ship_name, shi->shield_hit_timers.size());
	Assertion(shi->hull_hit_index < (int) shi->shield_hit_timers.size(), "Shield hit info object for object '%s' has a hull_hit_index of %d (should be between 0 and " SIZE_T_ARG "); get a coder!\n", Ships[objp->instance].ship_name, shi->hull_hit_index, shi->shield_hit_timers.size() - 1);

	if ( quadrant >= 0 ) {
		if ( !(Ship_info[Ships[objp->instance].ship_info_index].flags[Ship::Info_Flags::Model_point_shields]) )
			num = Quadrant_xlate[quadrant];
		else
			num = quadrant;

		Assertion(num < shi->hull_hit_index, "Shield hit info object for object '%s' hit on quadrant #%d, despite having a hull_hit_index of %d; get a coder!\n", Ships[objp->instance].ship_name, num, shi->hull_hit_index);
		shi->shield_hit_timers[num] = timestamp(SHIELD_HIT_DURATION_SHORT);
	} else {
		shi->shield_hit_timers[shi->hull_hit_index] = timestamp(SHIELD_HIT_DURATION_SHORT);
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

void HudGaugeShield::render(float  /*frametime*/, bool /*config*/)
{
}

void HudGaugeShield::showShields(const object *objp, ShieldGaugeType mode, bool config)
{
	if (!config && objp->type != OBJ_SHIP )
		return;

	// Goober5000 - don't show if primitive sensors
	if (!config && Ships[Player_obj->instance].flags[Ship::Ship_Flags::Primitive_sensors] )
		return;

	ship* sp = nullptr;
	ship_info* sip = nullptr;
	if (!config) {
		sp = &Ships[objp->instance];
		sip = &Ship_info[sp->ship_info_index];
	} else {
		SCP_string ship_name;

		// Now check if the current HUD specifies a different ship
		if (SCP_vector_inbounds(HC_available_huds, HC_chosen_hud)) {
			SCP_string hud = HC_available_huds[HC_chosen_hud].second;
			if (auto it = HC_hud_shield_ships.find(hud); it != HC_hud_shield_ships.end()) {
				ship_name = it->second[mode];
			}
		}

		// If we don't have a specific setting then let's try using the global setting
		if (ship_name.empty()) {
			if (auto it = HC_hud_shield_ships.find("default"); it != HC_hud_shield_ships.end()) {
				ship_name = it->second[mode];
			}
		}

		// If we still don't have something then try to use the FS2 retail default
		if (ship_name.empty()) {
			ship_name = "gtf myrmidon";
		}

		// Try to find the ship with the name specified in the config
		for (ship_info& ship : Ship_info) {
			if (!stricmp(ship.name, ship_name.c_str()) &&
				(ship.shield_icon_index != 255 || (ship.flags[Ship::Info_Flags::Generate_hud_icon]))) {
				sip = &ship;
				break;
			}
		}

		// Couldn't find it so just get the first ship with shields
		if (sip == nullptr) {
			for (ship_info& ship : Ship_info) {
				if (ship.shield_icon_index != 255 || (ship.flags[Ship::Info_Flags::Generate_hud_icon])) {
					sip = &ship;
					break;
				}
			}
		}
	}

	// If we still don't have a ship to display, bail
	if (sip == nullptr) {
		return;
	}

	if (sip->shield_icon_index == 255 && !(sip->flags[Ship::Info_Flags::Generate_hud_icon]) /*&& !digitus_improbus*/) {
		return;
	}

	setGaugeColor(HUD_C_NONE, config);

	std::shared_ptr<hud_frames> sgp = nullptr;
	// load in shield frames if not already loaded
	if (sip->shield_icon_index != 255) {
		if (!config) {
			sgp = std::shared_ptr<hud_frames>(&Shield_gauges.at(sip->shield_icon_index), [](hud_frames*) {
				/* Do nothing, managed externally */
			});
		} else {
			sgp = std::make_unique<hud_frames>();
			sgp->first_frame = -1;
			sgp->num_frames = 0;
		}

		if (config || (sgp != nullptr && (sgp->first_frame < 0) && (sip->shield_icon_index < Hud_shield_filenames.size())) ) {
			sgp->first_frame = bm_load_animation(Hud_shield_filenames.at(sip->shield_icon_index).c_str(), &sgp->num_frames);
			if (sgp->first_frame == -1) {
				if (!shield_ani_warning_displayed_already) {
					shield_ani_warning_displayed_already = true;
					Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames.at(sip->shield_icon_index).c_str());
				}
				return;
			}
		}
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	// Make copies so we can modify them
	int sx = x;
	int sy = y;

	// draw the ship first
	if (!config) {
		maybeFlashShield(SHIELD_GAUGE_PLAYER, Shield_hit_data[SHIELD_GAUGE_PLAYER].hull_hit_index);
	}

	if(sip->shield_icon_index != 255)
	{
		renderBitmap(sgp->first_frame, sx, sy, scale, config);
	}
	else
	{
		GR_DEBUG_SCOPE("Render generated shield icon");

		bool g3_yourself = !g3_in_frame();
		angles rot_angles = {-1.570796327f,0.0f,0.0f};
		matrix	object_orient;

		vm_angles_2_matrix(&object_orient, &rot_angles);

		const int CLIP_WIDTH = fl2i(112 * scale);
		const int CLIP_HEIGHT = fl2i(93 * scale);
		gr_screen.clip_width = CLIP_WIDTH;
		gr_screen.clip_height = CLIP_HEIGHT;

		//Fire it up
		if(g3_yourself)
			g3_start_frame(1);
		hud_save_restore_camera_data(1);
		setClip(sx, sy, CLIP_WIDTH, CLIP_HEIGHT);

		g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 2.5f);

			gr_set_proj_matrix(Proj_fov * 0.5f, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);

		//We're ready to show stuff
		{
			model_render_params render_info;

			// If this comment is here then I have not tested this
			int mi = -1;
			if (!config) {
				mi = sp->model_instance_num;
			}else{
				mi = model_load(sip, false);
			}

			render_info.set_flags(MR_NO_LIGHTING | MR_AUTOCENTER | MR_NO_FOGGING);
			render_info.set_replacement_textures(model_get_instance(mi)->texture_replace);
			render_info.set_detail_level_lock(1);
			render_info.set_object_number(OBJ_INDEX(objp));

			model_render_immediate( &render_info, sip->model_num, &object_orient, &vmd_zero_vector );
		}

		//We're done
		gr_end_view_matrix();
		gr_end_proj_matrix();
		if(g3_yourself)
			g3_end_frame();
		hud_save_restore_camera_data(0);

		resetClip();
	}

	if(!sip->max_shield_strength)
		return;

	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	float max_shield = config ? 100.0f : shield_get_max_quad(objp);

	coord2d shield_icon_coords[6];

	int n_quadrants = config ? DEFAULT_SHIELD_SECTIONS : static_cast<int>(objp->shield_quadrant.size());

	for (int i = 0; i < n_quadrants; i++) {

		if (!config && objp->flags[Object::Object_Flags::No_shields]) {
			break;
		}

		if ( (!config) && (max_shield > 0.0f) ) {
			if (!(sip->flags[Ship::Info_Flags::Model_point_shields])) {
				if (objp->shield_quadrant[Quadrant_xlate[i]]/max_shield < Shield_percent_skips_damage)
					continue;
			} else {
				if (objp->shield_quadrant[i]/max_shield < Shield_percent_skips_damage)
					continue;
			}
		}
		GR_DEBUG_SCOPE("Render shield quadrant");

		int range = MAX(HUD_COLOR_ALPHA_MAX, HUD_color_alpha + n_quadrants);

		int hud_color_index = HUD_C_NONE;
		if (!config) {
			if (!(sip->flags[Ship::Info_Flags::Model_point_shields]))
				hud_color_index = fl2i((objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range);
			else
				hud_color_index = fl2i((objp->shield_quadrant[i] / max_shield) * range);

			Assert(hud_color_index >= 0 && hud_color_index <= range);

			if (hud_color_index < 0) {
				hud_color_index = 0;
			}
			if (hud_color_index >= HUD_NUM_COLOR_LEVELS) {
				hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
			}
		}

		int flash=0;
		if (!config) {
			flash = maybeFlashShield(mode, i);
		}
				
		if ( !flash ) {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			setGaugeColor(hud_color_index, config);
			

			if(sip->shield_icon_index != 255)
			{
				int framenum = sgp->first_frame+i+1;
				if (framenum < sgp->first_frame + sgp->num_frames) {
					renderBitmap(framenum, sx, sy, scale, config);
				}
				if (config) {
					int bmw, bmh;
					bm_get_info(sgp->first_frame, &bmw, &bmh);
					hud_config_set_mouse_coords(gauge_config_id, x, x + fl2i(bmw * scale), y, y + fl2i(bmh * scale));
				}

			}
			else
			{
				//Ugh, draw four shield quadrants
				static const int TRI_EDGE = fl2i(6 * scale);
				static const int BAR_LENGTH = fl2i(112 * scale);
				static const int BAR_HEIGHT = fl2i(63 * scale);
				static const int BAR_WIDTH = fl2i(6 * scale);
				static const int SHIELD_OFFSET = fl2i(BAR_WIDTH + TRI_EDGE + 3 * scale);

				// If this comment is here then I have not tested this
				if (config) {
					hud_config_set_mouse_coords(gauge_config_id, x, x + BAR_LENGTH, y, y + BAR_HEIGHT);
				}

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
		gr_set_screen_scale(canvas_w, canvas_h, -1, -1, target_w, target_h, target_w, target_h, true);

		// Respect the rendering display offset specified in the table
		nx = display_offset_x;
		ny = display_offset_y;

		// Transfer the offset position into actual texture coordinates
		gr_unsize_screen_pos(&nx, &ny);
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

	//gr_shield_icon(coords);
	g3_render_shield_icon(coords);
	gr_reset_screen_scale();
}

int HudGaugeShield::maybeFlashShield(ShieldGaugeType target_index, int shield_offset)
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

void HudGaugeShieldPlayer::render(float  /*frametime*/, bool config)
{
	object* player = nullptr;
	if (!config) {
		player = Player_obj;
	}
	showShields(player, SHIELD_GAUGE_PLAYER, config);
}

HudGaugeShieldTarget::HudGaugeShieldTarget():
HudGaugeShield(HUD_OBJECT_TARGET_SHIELD, HUD_TARGET_SHIELD_ICON)
{

}

void HudGaugeShieldTarget::render(float  /*frametime*/, bool config)
{
	object* targetp = nullptr;

	if (!config) {
		if (Player_ai->target_objnum == -1)
			return;

		targetp = &Objects[Player_ai->target_objnum];

		// check to see if there is even a current target
		if (targetp == &obj_used_list) {
			return;
		}

		if (targetp == Player_obj)
			return;
	}

	showShields(targetp, SHIELD_GAUGE_TARGET, config);
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

void HudGaugeShieldMini::initBitmaps(const char *fname)
{
	Shield_mini_gauge.first_frame = bm_load_animation(fname, &Shield_mini_gauge.num_frames);
	if ( Shield_mini_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", fname);
	}
}

void HudGaugeShieldMini::render(float  /*frametime*/, bool config)
{
	object* targetp = nullptr;

	if (!config) {
		if (Player_ai->target_objnum == -1)
			return;

		targetp = &Objects[Player_ai->target_objnum];

		// check to see if there is even a current target
		if (targetp == &obj_used_list) {
			return;
		}
	}

	showMiniShields(targetp, config);
}

void HudGaugeShieldMini::pageIn()
{
	bm_page_in_aabitmap( Shield_mini_gauge.first_frame, Shield_mini_gauge.num_frames );
}

// Draw the miniature shield icon that is drawn near the reticle
void HudGaugeShieldMini::showMiniShields(const object *objp, bool config)
{
	if (Shield_mini_gauge.first_frame < 0)
		return;

	if (!config && objp->type != OBJ_SHIP ) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
		int bmw, bmh;
		bm_get_info(Shield_mini_gauge.first_frame, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id, x, x + static_cast<int>(bmw * scale), y, y + static_cast<int>(bmh * scale));
	}

	setGaugeColor(HUD_C_NONE, config);

	// draw the ship first
	if (!config) {
		maybeFlashShield(SHIELD_GAUGE_TARGET, Shield_hit_data[SHIELD_GAUGE_TARGET].hull_hit_index);
	}
	showIntegrity(config ? 1.0f : get_hull_pct(objp), config);

	// draw the four quadrants
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	float max_shield;
	if (!config) {
		max_shield = shield_get_max_quad(objp);
	} else {
		max_shield = 100.0f;
	}

	int n_quadrants = config ? DEFAULT_SHIELD_SECTIONS : static_cast<int>(objp->shield_quadrant.size());

	for (int i = 0; i < n_quadrants; i++) {

		if (!config && objp->flags[Object::Object_Flags::No_shields] ) {
			break;
		}

		int num;
		if (!config && !(Ship_info[Ships[objp->instance].ship_info_index].flags[Ship::Info_Flags::Model_point_shields]))
			num = Quadrant_xlate[i];
		else
			num = i;

		if ( (!config) && (max_shield > 0.0f) && (objp->shield_quadrant[num]/max_shield < Shield_percent_skips_damage) ) {
			continue;
		}

		int frame_offset;
		if (!config && maybeFlashShield(SHIELD_GAUGE_TARGET, i) ) {
			frame_offset = i+n_quadrants;
		} else {
			frame_offset = i;
		}
		
		int hud_color_index = HUD_C_NONE;
		if (!config) {
			int range = HUD_color_alpha;
			hud_color_index = (int)std::lround((objp->shield_quadrant[num] / max_shield) * range);
			Assert(hud_color_index >= 0 && hud_color_index <= range);

			if (hud_color_index < 0) {
				hud_color_index = 0;
			}
			if (hud_color_index >= HUD_NUM_COLOR_LEVELS) {
				hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
			}
		}

		if (!config && maybeFlashSexp() == 1) {
			// hud_set_bright_color();
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			setGaugeColor(hud_color_index, config);
		}	 

		if (frame_offset < Shield_mini_gauge.num_frames)
			renderBitmap(Shield_mini_gauge.first_frame + frame_offset, x, y, scale, config);		
	}
	
	// hud_set_default_color();
}

void HudGaugeShieldMini::showIntegrity(float p_target_integrity, bool config)
{
	int numeric_integrity = (int)std::lround(p_target_integrity * 100);
	if(numeric_integrity > 100){
		numeric_integrity = 100;
	}
	// Assert(numeric_integrity <= 100);

	// 3 digit hull strength
	int final_pos[2];
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

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	final_pos[0] = static_cast<int>(final_pos[0] * scale);
	final_pos[1] = static_cast<int>(final_pos[1] * scale);

	final_pos[0] += x;
	final_pos[1] += y;

	char text_integrity[64];
	sprintf(text_integrity, "%d", numeric_integrity);
	if ( numeric_integrity < 100 ) {
		hud_num_make_mono(text_integrity, font_num);
	}	

	renderString(final_pos[0], final_pos[1], text_integrity, scale, config);
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
