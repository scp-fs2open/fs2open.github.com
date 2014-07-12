/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "radar/radar.h"
#include "radar/radarorb.h"
#include "graphics/font.h"
#include "bmpman/bmpman.h"
#include "object/object.h"
#include "jumpnode/jumpnode.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "io/timer.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "ship/subsysdamage.h"
#include "gamesnd/gamesnd.h"
#include "network/multi.h"
#include "weapon/emp.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"
#include "ship/awacs.h"
#include "radar/radarsetup.h"
#include "iff_defs/iff_defs.h"
#include "globalincs/linklist.h"
#include "debugconsole/console.h"

int Radar_static_looping = -1;

rcol Radar_color_rgb[MAX_RADAR_COLORS][MAX_RADAR_LEVELS] =
{
	// homing missile (yellow)
	{	
		{ 0x40, 0x40, 0x00 },		// dim
		{ 0x7f, 0x7f, 0x00 },		// bright
	},

	// navbuoy or cargo (gray)
	{
		{ 0x40, 0x40, 0x40 },		// dim
		{ 0x7f, 0x7f, 0x7f },		// bright
	},

	// warping ship (blue)
	{
		{ 0x00, 0x00, 0x7f },		// dim
		{ 0x00, 0x00, 0xff },		// bright
	},

	// jump node (gray)
	{
		{ 0x40, 0x40, 0x40 },		// dim
		{ 0x7f, 0x7f, 0x7f },		// bright
	},

	// tagged (yellow)
	{
		{ 0x7f, 0x7f, 0x00 },		// dim
		{ 0xff, 0xff, 0x00 },		// bright
	},
};

int		radar_target_id_flags = 0;

color Radar_colors[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];

blip	Blip_bright_list[MAX_BLIP_TYPES];		// linked list of bright blips
blip	Blip_dim_list[MAX_BLIP_TYPES];			// linked list of dim blips

blip	Blips[MAX_BLIPS];								// blips pool
int	N_blips;											// next blip index to take from pool

float	Radar_bright_range;					// range at which we start dimming the radar blips
int		Radar_calc_bright_dist_timer;		// timestamp at which we recalc Radar_bright_range

extern int radar_iff_color[5][2][4];

int See_all = 0;

DCF_BOOL(see_all, See_all);

void radar_stuff_blip_info(object *objp, int is_bright, color **blip_color, int *blip_type)
{
	ship *shipp = NULL;

	switch(objp->type)
	{
		case OBJ_SHIP:
			shipp = &Ships[objp->instance];

			if (shipp->flags & SF_ARRIVING_STAGE_1)
			{
				*blip_color = &Radar_colors[RCOL_WARPING_SHIP][is_bright];
				*blip_type = BLIP_TYPE_WARPING_SHIP;
			}
			else if (ship_is_tagged(objp))
			{
				*blip_color = &Radar_colors[RCOL_TAGGED][is_bright];
				*blip_type = BLIP_TYPE_TAGGED_SHIP;
			}
			else if (Ship_info[shipp->ship_info_index].flags & (SIF_NAVBUOY|SIF_CARGO))
			{
				*blip_color = &Radar_colors[RCOL_NAVBUOY_CARGO][is_bright];
				*blip_type = BLIP_TYPE_NAVBUOY_CARGO;
			}
			else
			{
				*blip_color = iff_get_color_by_team_and_object(shipp->team, Player_ship->team, is_bright, objp);
				*blip_type = BLIP_TYPE_NORMAL_SHIP;
			}

			break;

		case OBJ_WEAPON:
			if ((Weapons[objp->instance].lssm_stage == 2) || (Weapons[objp->instance].lssm_stage == 4))
			{
				*blip_color = &Radar_colors[RCOL_WARPING_SHIP][is_bright];
				*blip_type = BLIP_TYPE_WARPING_SHIP;
			}
			else
			{
				*blip_color = &Radar_colors[RCOL_BOMB][is_bright];
				*blip_type = BLIP_TYPE_BOMB;
			}

			break;

		case OBJ_JUMP_NODE:
			*blip_color = &Radar_colors[RCOL_JUMP_NODE][is_bright];
			*blip_type = BLIP_TYPE_JUMP_NODE;

			break;

		default:
			Error(LOCATION, "Illegal blip type in radar.");
			break;
	}
}

void radar_plot_object( object *objp )
{
	vec3d pos, tempv;
	float awacs_level, dist, max_radar_dist;
	vec3d world_pos = objp->pos;
	SCP_list<CJumpNode>::iterator jnp;

	// don't process anything here.  Somehow, a jumpnode object caused this function
	// to get entered on server side.
	if( Game_mode & GM_STANDALONE_SERVER ){
		return;
	}

	// multiplayer clients ingame joining should skip this function
	if ( MULTIPLAYER_CLIENT && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN) ){
		return;
	}

	// get team-wide awacs level for the object if not ship
	int ship_is_visible = 0;
	if (objp->type == OBJ_SHIP) {
		if (Player_ship != NULL) {
			if (ship_is_visible_by_team(objp, Player_ship)) {
				ship_is_visible = 1;
			}
		}
	}

	// only check awacs level if ship is not visible by team
	awacs_level = 1.5f;
	if (Player_ship != NULL && !ship_is_visible) {
		awacs_level = awacs_get_level(objp, Player_ship);
	}

	// if the awacs level is unviewable - bail
	if(awacs_level < 0.0f && !See_all){
		return;
	}

	// Apply object type filters	
	switch (objp->type)
	{
		case OBJ_SHIP:
			// Place to cull ships, such as NavBuoys		
			break;
		
		case OBJ_JUMP_NODE:
		{
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == objp)
					break;
			}
			
			// don't plot hidden jump nodes
			if ( jnp->IsHidden() )
				return;

			// filter jump nodes here if required
			break;
		}

		case OBJ_WEAPON:
		{
			// if not a bomb, return
			if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_SHOWN_ON_RADAR) )
				if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_BOMB) )
					return;

			// if explicitly hidden, return
			if (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_DONT_SHOW_ON_RADAR)
				return;

			// if we don't attack the bomb, return
			if ( (!(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_SHOW_FRIENDLY)) && (!iff_x_attacks_y(Player_ship->team, obj_team(objp))))
				return;

			// if a local ssm is in subspace, return
			if (Weapons[objp->instance].lssm_stage == 3)
				return;

			// if corkscrew missile use last frame pos for pos
			if ( (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CORKSCREW) )
				world_pos = objp->last_pos;

			break;
		}

		// if any other kind of object, don't show it on radar
		default:
			return;
	}

	// Retrieve the eye orientation so we can position the blips relative to it
	matrix eye_orient;

	if (Player_obj->type == OBJ_SHIP) 
		ship_get_eye(&tempv, &eye_orient, Player_obj, false , false);
	else
		eye_orient = Player_obj->orient;

	// JAS -- new way of getting the rotated point that doesn't require this to be
	// in a g3_start_frame/end_frame block.
	vm_vec_sub(&tempv, &world_pos, &Player_obj->pos);
	vm_vec_rotate(&pos, &tempv, &eye_orient);

	// Apply range filter
	dist = vm_vec_dist(&world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if (dist > max_radar_dist) {
		return;
	}

	// determine the range within which the radar blip is bright
	if (timestamp_elapsed(Radar_calc_bright_dist_timer))
	{
		Radar_calc_bright_dist_timer = timestamp(1000);
		Radar_bright_range = player_farthest_weapon_range();
		if (Radar_bright_range <= 0)
			Radar_bright_range = 1500.0f;
	}

	blip *b;
	int blip_bright = 0;
	int blip_type = 0;

	if (N_blips >= MAX_BLIPS)
	{
		return;
	}

	b = &Blips[N_blips];
	b->flags = 0;

	// bright if within range
	blip_bright = (dist <= Radar_bright_range);

	// flag the blip as a current target if it is
	if (OBJ_INDEX(objp) == Player_ai->target_objnum)
	{
		b->flags |= BLIP_CURRENT_TARGET;
		blip_bright = 1;
	}

	radar_stuff_blip_info(objp, blip_bright, &b->blip_color, &blip_type);

	if (blip_bright)
		list_append(&Blip_bright_list[blip_type], b);
	else
		list_append(&Blip_dim_list[blip_type], b);

	b->position = pos;
	b->dist = dist;
	b->objp = objp;
	b->radar_image_2d = -1;
	b->radar_color_image_2d = -1;
	b->radar_image_size = -1;
	b->radar_projection_size = 1.0f;

	// see if blip should be drawn distorted
	// also determine if alternate image was defined for this ship
	if (objp->type == OBJ_SHIP)
	{
		// ships specifically hidden from sensors
		if (Ships[objp->instance].flags & SF_HIDDEN_FROM_SENSORS)
			b->flags |= BLIP_DRAW_DISTORTED;

		// determine if its AWACS distorted
		if (awacs_level < 1.0f)
			b->flags |= BLIP_DRAW_DISTORTED;

		ship_info Iff_ship_info = Ship_info[Ships[objp->instance].ship_info_index];

		if (Iff_ship_info.radar_image_2d_idx >= 0 || Iff_ship_info.radar_color_image_2d_idx >= 0)
		{
			b->radar_image_2d = Iff_ship_info.radar_image_2d_idx;
			b->radar_color_image_2d = Iff_ship_info.radar_color_image_2d_idx;
			b->radar_image_size = Iff_ship_info.radar_image_size;
			b->radar_projection_size = Iff_ship_info.radar_projection_size_mult;
		}
	}

	// don't distort the sensor blips if the player has primitive sensors and the nebula effect
	// is not active
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
	{
		if (!(The_mission.flags & MISSION_FLAG_FULLNEB))
			b->flags &= ~BLIP_DRAW_DISTORTED;
	}

	N_blips++;
}

void radar_mission_init()
{
	for (int i=0; i<MAX_RADAR_COLORS; i++ )	{
		for (int j=0; j<MAX_RADAR_LEVELS; j++ )	{
			if (radar_iff_color[i][j][0] >= 0) {
				gr_init_alphacolor( &Radar_colors[i][j], radar_iff_color[i][j][0], radar_iff_color[i][j][1], radar_iff_color[i][j][2], radar_iff_color[i][j][3] );
			} else {
				gr_init_alphacolor( &Radar_colors[i][j], Radar_color_rgb[i][j].r, Radar_color_rgb[i][j].g, Radar_color_rgb[i][j].b, 255 );
			}
		}
	}

	Radar_calc_bright_dist_timer = timestamp(0);
}

void radar_null_nblips()
{
	int i;

	N_blips=0;

	for (i=0; i<MAX_BLIP_TYPES; i++) {
		list_init(&Blip_bright_list[i]);
		list_init(&Blip_dim_list[i]);
	}
}

void radar_frame_init()
{
	radar_null_nblips();
}

HudGaugeRadar::HudGaugeRadar():
HudGauge(HUD_OBJECT_RADAR_STD, HUD_RADAR, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

HudGaugeRadar::HudGaugeRadar(int _gauge_object, int r, int g, int b):
HudGauge(_gauge_object, HUD_RADAR, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), r, g, b)
{
}

void HudGaugeRadar::initRadius(int w, int h)
{
	Radar_radius[0] = w;
	Radar_radius[1] = h;
}

void HudGaugeRadar::initBlipRadius(int normal, int target)
{
	Radar_blip_radius_normal = normal;
	Radar_blip_radius_target = target;
}

void HudGaugeRadar::initDistanceShortOffsets(int x, int y)
{
	Radar_dist_offsets[RR_SHORT][0] = x;
	Radar_dist_offsets[RR_SHORT][1] = y;
}

void HudGaugeRadar::initDistanceLongOffsets(int x, int y)
{
	Radar_dist_offsets[RR_LONG][0] = x;
	Radar_dist_offsets[RR_LONG][1] = y;
}

void HudGaugeRadar::initDistanceInfinityOffsets(int x, int y)
{
	Radar_dist_offsets[RR_INFINITY][0] = x;
	Radar_dist_offsets[RR_INFINITY][1] = y;
}

void HudGaugeRadar::render(float frametime)
{
}

void HudGaugeRadar::pageIn()
{
}

void HudGaugeRadar::initialize()
{
	int i;

	Radar_death_timer			= 0;
	Radar_static_playing		= 0;
	Radar_static_next			= 0;
	Radar_avail_prev_frame	= 1;
	Radar_calc_bright_dist_timer = timestamp(0);

	for ( i=0; i<NUM_FLICKER_TIMERS; i++ ) {
		Radar_flicker_timer[i]=timestamp(0);
		Radar_flicker_on[i]=0;
	}

	int w,h;
	gr_set_font(FONT1);

	Small_blip_string[0] = ubyte(SMALL_BLIP_CHAR);
	Small_blip_string[1] = 0;
	gr_get_string_size( &w, &h, Small_blip_string );
	Small_blip_offset_x = -w/2;
	Small_blip_offset_y = -h/2;

	Large_blip_string[0] = ubyte(LARGE_BLIP_CHAR);
	Large_blip_string[1] = 0;
	gr_get_string_size( &w, &h, Large_blip_string );
	Large_blip_offset_x = -w/2;
	Large_blip_offset_y = -h/2;

	HudGauge::initialize();
}

void HudGaugeRadar::drawRange()
{
	char buf[32];

	// hud_set_bright_color();
	setGaugeColor(HUD_C_BRIGHT);

	switch ( HUD_config.rp_dist ) {

	case RR_SHORT:
		renderPrintf(position[0] + Radar_dist_offsets[RR_SHORT][0], position[1] + Radar_dist_offsets[RR_SHORT][1], XSTR( "2k", 467));
		break;

	case RR_LONG:
		renderPrintf(position[0] + Radar_dist_offsets[RR_LONG][0], position[1] + Radar_dist_offsets[RR_LONG][1], XSTR( "10k", 468));
		break;

	case RR_INFINITY:
		sprintf(buf, NOX("%c"), Lcl_special_chars);
		renderPrintf(position[0] + Radar_dist_offsets[RR_INFINITY][0], position[1] + Radar_dist_offsets[RR_INFINITY][1], buf);
		break;

	default:
		Int3();	// can't happen (get Alan if it does)
		break;
	}
}

/**
 * @brief Return if the specified object is visible on the radar
 *
 * @param objp The object which should be checked
 * @return A RadarVisibility enum specifying the visibility of the specified object
 */
RadarVisibility radar_is_visible( object *objp )
{
	Assert( objp != NULL );

	if (objp->flags & OF_SHOULD_BE_DEAD)
	{
		return NOT_VISIBLE;
	}

	vec3d pos, tempv;
	float awacs_level, dist, max_radar_dist;
	vec3d world_pos = objp->pos;
	SCP_list<CJumpNode>::iterator jnp;

	// get team-wide awacs level for the object if not ship
	int ship_is_visible = 0;
	if (objp->type == OBJ_SHIP) {
		if (Player_ship != NULL) {
			if (ship_is_visible_by_team(objp, Player_ship)) {
				ship_is_visible = 1;
			}
		}
	}

	// only check awacs level if ship is not visible by team
	awacs_level = 1.5f;
	if (Player_ship != NULL && !ship_is_visible) {
		awacs_level = awacs_get_level(objp, Player_ship);
	}

	// if the awacs level is unviewable - bail
	if(awacs_level < 0.0f && !See_all){
		return NOT_VISIBLE;
	}

	// Apply object type filters	
	switch (objp->type)
	{
		case OBJ_SHIP:
			if (Ships[objp->instance].flags & SIF_STEALTH)
				return NOT_VISIBLE;

			// Ships that are warp in in are not visible on the radar
			if (Ships[objp->instance].flags & SF_ARRIVING_STAGE_1)
				return NOT_VISIBLE;

			break;
		
		case OBJ_JUMP_NODE:
		{
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == objp)
					break;
			}
			
			// don't plot hidden jump nodes
			if ( jnp->IsHidden() )
				return NOT_VISIBLE;

			// filter jump nodes here if required
			break;
		}

		case OBJ_WEAPON:
		{
			// if not a bomb, return
			if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_SHOWN_ON_RADAR) )
				if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_BOMB) )
					return NOT_VISIBLE;

			// if explicitly hidden, return
			if (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_DONT_SHOW_ON_RADAR)
				return NOT_VISIBLE;

			// if we don't attack the bomb, return
			if ( (!(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_SHOW_FRIENDLY)) && (!iff_x_attacks_y(Player_ship->team, obj_team(objp))))
				return NOT_VISIBLE;

			// if a local ssm is in subspace, return
			if (Weapons[objp->instance].lssm_stage == 3)
				return NOT_VISIBLE;

			break;
		}

		// if any other kind of object, don't show it on radar
		default:
			return NOT_VISIBLE;
	}
	
	vm_vec_sub(&tempv, &world_pos, &Player_obj->pos);
	vm_vec_rotate(&pos, &tempv, &Player_obj->orient);

	// Apply range filter
	dist = vm_vec_dist(&world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if (dist > max_radar_dist) {
		return NOT_VISIBLE;
	}
	
	if (objp->type == OBJ_SHIP)
	{
		// ships specifically hidden from sensors
		if (Ships[objp->instance].flags & SF_HIDDEN_FROM_SENSORS)
			return DISTORTED;

		// determine if its AWACS distorted
		if (awacs_level < 1.0f)
			return DISTORTED;
	}
	
	return VISIBLE;
}
