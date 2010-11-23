/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/





#include "radar/radar.h"
#include "graphics/font.h"
#include "bmpman/bmpman.h"
#include "object/object.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "io/timer.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "ship/subsysdamage.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "network/multi.h"
#include "weapon/emp.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"
#include "ship/awacs.h"
#include "radar/radarsetup.h"
#include "iff_defs/iff_defs.h"
#include "jumpnode/jumpnode.h"

extern float radx, rady;

extern float	Radar_bright_range;				// range within which the radar blips are bright
extern int		Radar_calc_bright_dist_timer;	// timestamp at which we recalc Radar_bright_range

extern int Radar_flicker_timer[NUM_FLICKER_TIMERS];					// timestamp used to flicker blips on and off
extern int Radar_flicker_on[NUM_FLICKER_TIMERS];						// status of flickering

extern rcol Radar_color_rgb[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];
extern color Radar_colors[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];

extern blip	Blip_bright_list[MAX_BLIP_TYPES];		// linked list of bright blips
extern blip	Blip_dim_list[MAX_BLIP_TYPES];			// linked list of dim blips

extern blip	Blips[MAX_BLIPS];								// blips pool
extern int	N_blips;										// next blip index to take from pool

extern float Radar_farthest_dist;
extern int Blip_mutate_id;

extern int Radar_static_playing;			// is static currently playing on the radar?
extern int Radar_static_next;				// next time to toggle static on radar
extern int Radar_avail_prev_frame;		// was radar active last frame?
extern int Radar_death_timer;				// timestamp used to play static on radar
extern int Radar_static_looping;					// id for looping radar static sound

extern hud_frames Radar_gauge;

extern int radar_iff_color[5][2][4];

extern int radar_target_id_flags;

int current_target_x, current_target_y;

color radar_crosshairs;

// forward declarations
void draw_radar_blips_std(int blip_type, int bright, int distort = 0);
void draw_radar_crosshairs(int x, int y);

void radar_init_std()
{
	int i,j;

	Radar_gauge.first_frame = bm_load_animation(Current_radar_global->Radar_fname[gr_screen.res], &Radar_gauge.num_frames);
	if ( Radar_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", Current_radar_global->Radar_fname[gr_screen.res]);
	}

	for (i=0; i<MAX_RADAR_COLORS; i++ )	{
		for (j=0; j<MAX_RADAR_LEVELS; j++ )	{
			{
				if (radar_iff_color[i][j][0] >= 0)
				{
					gr_init_alphacolor( &Radar_colors[i][j], radar_iff_color[i][j][0], radar_iff_color[i][j][1], radar_iff_color[i][j][2], radar_iff_color[i][j][3] );
				}
				else
				{
					gr_init_alphacolor( &Radar_colors[i][j], Radar_color_rgb[i][j].r, Radar_color_rgb[i][j].g, Radar_color_rgb[i][j].b, 255 );
				}
			}
		}
	}

	gr_init_alphacolor( &radar_crosshairs, 255, 255, 255, 196);

	Blip_mutate_id	= 1;

	//WMC - Try and get rid of stupid radar list errors.
	radar_null_nblips_std();
}

// determine how the object blip should be drawn
void radar_stuff_blip_info_std(object *objp, int is_bright, color **blip_color, int *blip_type)
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

void radar_plot_object_std( object *objp )	
{
	vec3d	pos, tempv;
	float		dist, rscale, zdist, max_radar_dist;
	int		xpos, ypos;
	vec3d	*world_pos = &objp->pos;	
	float		awacs_level;

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
			// don't plot hidden jump nodes
			if ( objp->jnp->is_hidden() )
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

			// if we don't attack the bomb, return
			if ( (!(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags2 & WIF2_SHOW_FRIENDLY)) && (!iff_x_attacks_y(Player_ship->team, obj_team(objp))))
				return;

			// if a local ssm is in subspace, return
			if (Weapons[objp->instance].lssm_stage == 3)
				return;

			// if corkscrew missile use last frame pos for pos
			if ( (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CORKSCREW) )
				world_pos = &objp->last_pos;

			break;
		}

		// if any other kind of object, don't show it on radar
		default:
			return;
	}

	
	// JAS -- new way of getting the rotated point that doesn't require this to be
	// in a g3_start_frame/end_frame block.
	vm_vec_sub(&tempv, world_pos, &Player_obj->pos);
	vm_vec_rotate(&pos, &tempv, &Player_obj->orient);

	// Apply range filter
	dist = vm_vec_dist(world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if (dist > max_radar_dist) {
		return;
	}

	if (dist < pos.xyz.z) {
		rscale = 0.0f;
	} else {
		rscale = (float) acos(pos.xyz.z / dist) / 3.14159f;		//2.0f;	 
	}

	zdist = fl_sqrt((pos.xyz.x * pos.xyz.x) + (pos.xyz.y * pos.xyz.y));

	float new_x_dist, clipped_x_dist;
	float new_y_dist, clipped_y_dist;

	if (zdist < 0.01f)
	{
		new_x_dist = 0.0f;
		new_y_dist = 0.0f;
	}
	else
	{
		new_x_dist = (pos.xyz.x / zdist) * rscale * radx;
		new_y_dist = (pos.xyz.y / zdist) * rscale * rady;

		// force new_x_dist and new_y_dist to be inside the radar

		float hypotenuse;
		float max_radius;

		hypotenuse = (float) _hypot(new_x_dist, new_y_dist);
		max_radius = i2fl(Current_radar_global->Radar_radius[gr_screen.res][0] - 5);

		if (hypotenuse >= max_radius)
		{
			clipped_x_dist = max_radius * (new_x_dist / hypotenuse);
			clipped_y_dist = max_radius * (new_y_dist / hypotenuse);
			new_x_dist = clipped_x_dist;
			new_y_dist = clipped_y_dist;
		}
	}

	xpos = fl2i(Current_radar_global->Radar_center[gr_screen.res][0] + new_x_dist);
	ypos = fl2i(Current_radar_global->Radar_center[gr_screen.res][1] - new_y_dist);


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
		// out of blips, don't plot
		//Gahhh, this is bloody annoying -WMC
		//Int3();
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

	radar_stuff_blip_info_std(objp, blip_bright, &b->blip_color, &blip_type);

	if (blip_bright)
		list_append(&Blip_bright_list[blip_type], b);
	else
		list_append(&Blip_dim_list[blip_type], b);

	b->x = xpos;
	b->y = ypos;
	b->radar_image_2d = -1;
	b->radar_image_size = -1;

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

		if (Iff_ship_info.radar_image_2d_idx >= 0)
		{
			b->radar_image_2d = Iff_ship_info.radar_image_2d_idx;
			b->radar_image_size = Iff_ship_info.radar_image_size;
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

// set N_blips for each color/brightness level to zero
void radar_null_nblips_std()
{
	int i;

	N_blips=0;

	for (i=0; i<MAX_BLIP_TYPES; i++) {
		list_init(&Blip_bright_list[i]);
		list_init(&Blip_dim_list[i]);
	}
}

// radar_mission_init() is called at the start of each mission.  
void radar_mission_init_std()
{
	int i;

	Blip_mutate_id				= 1;
	Radar_death_timer			= 0;
	Radar_static_playing		= 0;
	Radar_static_next			= 0;
	Radar_avail_prev_frame	= 1;
	Radar_calc_bright_dist_timer = timestamp(0);

	for ( i=0; i<NUM_FLICKER_TIMERS; i++ ) {
		Radar_flicker_timer[i]=timestamp(0);
		Radar_flicker_on[i]=0;
	}
}

#define SMALL_BLIP_CHAR (Lcl_special_chars + 5)
#define LARGE_BLIP_CHAR (Lcl_special_chars + 6)

int Small_blip_offset_x = 0;
int Small_blip_offset_y = 0;
int Large_blip_offset_x = 0;
int Large_blip_offset_y = 0;

char Small_blip_string[2];
char Large_blip_string[2];

void radar_frame_init_std()
{
	radar_null_nblips_std();
	radx = i2fl(Current_radar_global->Radar_radius[gr_screen.res][0])/2.0f;
	rady = i2fl(Current_radar_global->Radar_radius[gr_screen.res][1])/2.0f;

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
}

void radar_draw_circle_std( int x, int y, int rad )
{
	if ( rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res] )	{
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		gr_string( Large_blip_offset_x+x, Large_blip_offset_y+y, Large_blip_string );
	} else {
		// rad = RADAR_BLIP_RADIUS_NORMAL;
		gr_string( Small_blip_offset_x+x, Small_blip_offset_y+y, Small_blip_string );
	}
}

// radar is damaged, so make blips dance around
void radar_blip_draw_distorted_std(blip *b)
{
	int xdiff, ydiff;
	float scale;
	xdiff = -10 + rand()%20;
	ydiff = -10 + rand()%20;

	// maybe scale the effect if EMP is active
	if(emp_active_local()){
		scale = emp_current_intensity();

		xdiff = (int)((float)xdiff * scale);
		ydiff = (int)((float)ydiff * scale);
	}

	radar_draw_circle_std(b->x+xdiff, b->y+ydiff, b->rad);
}

// blip is for a target immune to sensors, so cause to flicker in/out with mild distortion
void radar_blip_draw_flicker_std(blip *b)
{
	int xdiff=0, ydiff=0, flicker_index;

	if ( (b-Blips) & 1 ) {
		flicker_index=0;
	} else {
		flicker_index=1;
	}

	if ( timestamp_elapsed(Radar_flicker_timer[flicker_index]) ) {
		Radar_flicker_timer[flicker_index] = timestamp_rand(50,1000);
		Radar_flicker_on[flicker_index] ^= 1;
	}

	if ( !Radar_flicker_on[flicker_index] ) {
		return;
	}

	if ( rand() & 1 ) {
		xdiff = -2 + rand()%4;
		ydiff = -2 + rand()%4;
	}

	radar_draw_circle_std(b->x+xdiff, b->y+ydiff, b->rad);
}

// Draw all the active radar blips
void draw_radar_blips_std(int blip_type, int bright, int distort)
{
	blip *b = NULL;
	blip *blip_head = NULL;

	Assert((blip_type >= 0) && (blip_type < MAX_BLIP_TYPES));


	// Need to set font.
	gr_set_font(FONT1);


	// get the appropriate blip list
	if (bright)
		blip_head = &Blip_bright_list[blip_type];
	else
		blip_head = &Blip_dim_list[blip_type];


	// draw all blips of this type
	for (b = GET_FIRST(blip_head); b != END_OF_LIST(blip_head); b = GET_NEXT(b))
	{
		gr_set_color_fast(b->blip_color);

		// maybe draw cool blip to indicate current target
		if (b->flags & BLIP_CURRENT_TARGET)
		{
			b->rad = Current_radar_global->Radar_blip_radius_target[gr_screen.res];
			current_target_x = b->x;
			current_target_y = b->y;
		}
		else
		{
			b->rad = Current_radar_global->Radar_blip_radius_normal[gr_screen.res];
		}

		// maybe distort blip
		if (distort)
		{
			radar_blip_draw_distorted_std(b);
		}
		else if (b->flags & BLIP_DRAW_DISTORTED)
		{
			radar_blip_draw_flicker_std(b);
		}
		else
		{
			if (b->radar_image_2d == -1)
				radar_draw_circle_std(b->x, b->y, b->rad);
			else
				radar_draw_image_std(b->x, b->y, b->rad, b->radar_image_2d, b->radar_image_size);
		}
	}
}

// Draw the radar blips
// input:	distorted	=>		0 (default) to draw normal, 1 to draw distorted 
void radar_draw_blips_sorted_std(int distort)
{
	current_target_x = 0;
	current_target_y = 0;
	// draw dim blips first, then bright blips
	for (int is_bright = 0; is_bright < 2; is_bright++)
	{
		draw_radar_blips_std(BLIP_TYPE_JUMP_NODE, is_bright, distort);
		draw_radar_blips_std(BLIP_TYPE_WARPING_SHIP, is_bright, distort);
		draw_radar_blips_std(BLIP_TYPE_NAVBUOY_CARGO, is_bright, distort);
		draw_radar_blips_std(BLIP_TYPE_NORMAL_SHIP, is_bright, distort);
		draw_radar_blips_std(BLIP_TYPE_BOMB, is_bright, distort);
		draw_radar_blips_std(BLIP_TYPE_TAGGED_SHIP, is_bright, distort);
	}
	// draw crosshairs last - if at all.
	if(radar_target_id_flags & RTIF_CROSSHAIRS) {
		draw_radar_crosshairs(current_target_x, current_target_y);
	}
}

void radar_draw_range_std()
{
	char buf[32];

	// hud_set_bright_color();
	hud_set_gauge_color(HUD_RADAR, HUD_C_BRIGHT);

	switch ( HUD_config.rp_dist ) {

	case RR_SHORT:
		gr_printf(Current_radar_global->Radar_dist_coords[gr_screen.res][RR_SHORT][0], Current_radar_global->Radar_dist_coords[gr_screen.res][RR_SHORT][1], XSTR( "2k", 467));
		break;

	case RR_LONG:
		gr_printf(Current_radar_global->Radar_dist_coords[gr_screen.res][RR_LONG][0], Current_radar_global->Radar_dist_coords[gr_screen.res][RR_LONG][1], XSTR( "10k", 468));
		break;

	case RR_INFINITY:
		sprintf(buf, NOX("%c"), Lcl_special_chars);
		gr_printf(Current_radar_global->Radar_dist_coords[gr_screen.res][RR_INFINITY][0], Current_radar_global->Radar_dist_coords[gr_screen.res][RR_INFINITY][1], buf);
		break;

	default:
		Int3();	// can't happen (get Alan if it does)
		break;
	}

	hud_set_default_color();
}

void radar_frame_render_std(float frametime)
{
	float	sensors_str;
	int ok_to_blit_radar;

	ok_to_blit_radar = 1;

	sensors_str = ship_get_subsystem_strength( Player_ship, SUBSYSTEM_SENSORS );

	if ( ship_subsys_disrupted(Player_ship, SUBSYSTEM_SENSORS) ) {
		sensors_str = MIN_SENSOR_STR_TO_RADAR-1;
	}

	// note that on lowest skill level, there is no radar effects due to sensors damage
	if ( (Game_skill_level == 0) || (sensors_str > SENSOR_STR_RADAR_NO_EFFECTS) ) {
		Radar_static_playing = 0;
		Radar_static_next = 0;
		Radar_death_timer = 0;
		Radar_avail_prev_frame = 1;
	} else if ( sensors_str < MIN_SENSOR_STR_TO_RADAR ) {
		if ( Radar_avail_prev_frame ) {
			Radar_death_timer = timestamp(2000);
			Radar_static_next = 1;
		}
		Radar_avail_prev_frame = 0;
	} else {
		Radar_death_timer = 0;
		if ( Radar_static_next == 0 )
			Radar_static_next = 1;
	}

	if ( timestamp_elapsed(Radar_death_timer) ) {
		ok_to_blit_radar = 0;
	}

	hud_set_gauge_color(HUD_RADAR);
	radar_blit_gauge_std();
	radar_draw_range_std();

	if ( timestamp_elapsed(Radar_static_next) ) {
		Radar_static_playing ^= 1;
		Radar_static_next = timestamp_rand(50, 750);
	}

	// if the emp effect is active, always draw the radar wackily
	if(emp_active_local()){
		Radar_static_playing = 1;
	}

	if ( ok_to_blit_radar ) {
		if ( Radar_static_playing ) {
			radar_draw_blips_sorted(1);	// passing 1 means to draw distorted
			if ( Radar_static_looping == -1 ) {
				Radar_static_looping = snd_play_looping(&Snds[SND_STATIC]);
			}
		} else {
			radar_draw_blips_sorted(0);
			if ( Radar_static_looping != -1 ) {
				snd_stop(Radar_static_looping);
				Radar_static_looping = -1;
			}
		}
	} else {
		if ( Radar_static_looping != -1 ) {
			snd_stop(Radar_static_looping);
			Radar_static_looping = -1;
		}
	}
}

void radar_blit_gauge_std()
{
	gr_set_bitmap(Radar_gauge.first_frame+1);
	gr_aabitmap( Current_radar_global->Radar_coords[gr_screen.res][0], Current_radar_global->Radar_coords[gr_screen.res][1] );
} 

void radar_page_in_std()
{
	bm_page_in_aabitmap( Radar_gauge.first_frame, Radar_gauge.num_frames );
}

void radar_draw_image_std( int x, int y, int rad, int idx, int size )
{
	// this we will move as ships.tbl option (or use for radar scaling etc etc)
	//int size = 24; 
	
	int w, h, old_bottom, old_bottom_unscaled, old_right, old_right_unscaled;
	float scalef, wf, hf, xf, yf;
	vec3d blip_scaler;

	if(bm_get_info(idx, &w, &h) < 0)
	{
		// Just if something goes terribly wrong
		radar_draw_circle_std(x, y, rad);
		return;
	}

	// just to make sure the missing casts wont screw the math
	wf = (float) w;
	hf = (float) h;
	xf = (float) x;
	yf = (float) y;

	// make sure we use the larger dimension for the scaling
	// lets go case by case to make sure there are no probs
	if (size == -1)
		scalef = 1.0f;
	else if ((h == w) && (size == h))
		scalef = 1.0f;
	else if ( h > w)
		scalef = ((float) size) / hf;
	else
		scalef = ((float) size) / wf;

	Assert(scalef != 0);

	// animate the targeted icon - option 1 of highlighting the targets
	if ( rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res] ) {
		if (radar_target_id_flags & RTIF_PULSATE) {
			scalef *= 1.3f + (sinf(10 * f2fl(Missiontime)) * 0.3f);
		}
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		if (radar_target_id_flags & RTIF_ENLARGE) {
			scalef *= 1.3f;
		}
	}

	// setup the scaler
	blip_scaler.xyz.x = scalef;
	blip_scaler.xyz.y = scalef;
	blip_scaler.xyz.z = 1.0f;
	
	old_bottom = gr_screen.clip_bottom;
	old_bottom_unscaled = gr_screen.clip_bottom_unscaled;
	gr_screen.clip_bottom = (int) (old_bottom/scalef);
	gr_screen.clip_bottom_unscaled = (int) (old_bottom_unscaled/scalef);

	old_right = gr_screen.clip_right;
	old_right_unscaled = gr_screen.clip_right_unscaled;
	gr_screen.clip_right = (int) (old_right/scalef);
	gr_screen.clip_right_unscaled = (int) (old_right_unscaled/scalef);

	// scale the drawing coordinates
	x = (int) ((xf / scalef) - wf/2.0f);
	y = (int) ((yf / scalef) - hf/2.0f);

	gr_push_scale_matrix(&blip_scaler);
	gr_set_bitmap(idx,GR_ALPHABLEND_NONE,GR_BITBLT_MODE_NORMAL,1.0f);
	gr_aabitmap( x, y );
	gr_pop_scale_matrix();

	gr_screen.clip_bottom = old_bottom;
	gr_screen.clip_bottom_unscaled = old_bottom_unscaled;

	gr_screen.clip_right = old_right;
	gr_screen.clip_right_unscaled = old_right_unscaled;
}

void draw_radar_crosshairs(int x, int y)
{
	int i,j,m;

	gr_set_color_fast(&radar_crosshairs);

	for(i = 0; i < 2; i++) {
		m = (i * 2) - 1;
		gr_gradient(x + m*4,y,x + m*8,y,false);
	}
	for(j = 0; j < 2; j++) {
		m = (j * 2) - 1;
		gr_gradient(x,y + m*4,x,y + m*8,false);
	}
}
