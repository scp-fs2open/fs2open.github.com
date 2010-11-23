/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "radar/radarorb.h"
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
#include "render/3d.h"
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
extern int	N_blips;											// next blip index to take from pool

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

extern int Cmdline_nohtl;

static const int NUM_ORB_RING_SLICES = 16;

vec3d orb_ring_yz[NUM_ORB_RING_SLICES];
vec3d orb_ring_xy[NUM_ORB_RING_SLICES];
vec3d orb_ring_xz[NUM_ORB_RING_SLICES];
vec3d vec_extents[]=
{
	{ { { 1.0f, 0.0f, 0.0f } } },
	{ { { 0.0f, 1.0f, 0.0f } } },
	{ { { 0.0f, 0.0f, 1.0f } } },
	{ { { -1.0f, 0.0f, 0.0f } } },
	{ { { 0.0f, -1.0f, 0.0f } } },
	{ { { 0.0f, 0.0f, -1.0f } } }
};

color Orb_color_orange;
color Orb_color_teal;
color Orb_color_purple;
color Orb_crosshairs;

//special view matrix to get the orb rotating the correct way
static matrix view_perturb = { { { { { { 1.0f, 0.0f, 0.0f } } },
                                   { { { 0.0f, -1.0f, 0.0f } } },
                                   { { { 0.0f, 0.0f, -1.0f } } } } } };

static vec3d Orb_eye_position = { { { 0.0f, 0.0f, -3.0f } } };

vec3d target_position;

// forward declarations
void draw_radar_blips_orb(int blip_type, int bright, int distort = 0);
void draw_radar3d_crosshairs( vec3d pnt );

void radar_init_orb()
{
	int i,j;
	float s,c;

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

	memset(orb_ring_xy, 0, sizeof(orb_ring_xy));
	memset(orb_ring_xz, 0, sizeof(orb_ring_xz));
	memset(orb_ring_yz, 0, sizeof(orb_ring_yz));
	
    for (i=0; i < NUM_ORB_RING_SLICES; i++)
    {
        s=(float)sin(float(i*PI2)/NUM_ORB_RING_SLICES);
        c=(float)cos(float(i*PI2)/NUM_ORB_RING_SLICES);

        orb_ring_xy[i].xyz.x = c;
        orb_ring_xy[i].xyz.y = s;

        orb_ring_yz[i].xyz.y = c;
        orb_ring_yz[i].xyz.z = s;

        orb_ring_xz[i].xyz.x = c;
        orb_ring_xz[i].xyz.z = s;
    }

    gr_init_alphacolor(&Orb_color_orange, 192, 96, 32,  192);
    gr_init_alphacolor(&Orb_color_teal,   48, 160, 96,  192);
    gr_init_alphacolor(&Orb_color_purple, 112, 16, 192, 192);
	gr_init_alphacolor(&Orb_crosshairs,   255, 255,255, 192);

	Blip_mutate_id	= 1;

	//WMC - Try and get rid of stupid radar list errors.
	radar_null_nblips_orb();
}

// determine how the object blip should be drawn
void radar_stuff_blip_info_orb(object *objp, int is_bright, color **blip_color, int *blip_type)
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

void radar_plot_object_orb( object *objp )	
{
	vec3d	pos, tempv;
	float		dist, max_radar_dist;
	//float rscale, zdist;
	//int		xpos, ypos;
	vec3d	world_pos = objp->pos;	
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
				world_pos = objp->last_pos;

			break;
		}

		// if any other kind of object, don't show it on radar
		default:
			return;
	}


	// JAS -- new way of getting the rotated point that doesn't require this to be
	// in a g3_start_frame/end_frame block.
	vm_vec_sub(&tempv, &world_pos, &Player_obj->pos);
	vm_vec_rotate(&pos, &tempv, &Player_obj->orient);

	// Apply range filter
	dist = vm_vec_dist(&world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if (dist > max_radar_dist) {
		return;
	}

	if (IS_VEC_NULL_SQ_SAFE(&pos)) {
			vm_vec_make(&pos, 1.0f, 0.0f, 0.0f);
	} else {
			vm_vec_normalize(&pos);
	}

	float scale = dist / Radar_bright_range;
	if (scale > 1.25f) scale = 1.25f;
	if (scale < .75f) scale = .75f;

	vm_vec_scale(&pos, scale);


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

	radar_stuff_blip_info(objp, blip_bright, &b->blip_color, &blip_type);

	if (blip_bright)
		list_append(&Blip_bright_list[blip_type], b);
	else
		list_append(&Blip_dim_list[blip_type], b);

//	b->x = xpos;
//	b->y = ypos;
	b->position = pos;
	b->radar_image_2d = -1;
	b->radar_projection_size = 1.0f;

	// see if blip should be drawn distorted
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

// set N_blips for each color/brightness level to zero
void radar_null_nblips_orb()
{
	int i;

	N_blips=0;

	for (i=0; i<MAX_BLIP_TYPES; i++) {
		list_init(&Blip_bright_list[i]);
		list_init(&Blip_dim_list[i]);
	}
}

// radar_mission_init() is called at the start of each mission.  
void radar_mission_init_orb()
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

static int Small_blip_offset_x = 0;
static int Small_blip_offset_y = 0;
static int Large_blip_offset_x = 0;
static int Large_blip_offset_y = 0;

static char Small_blip_string[2];
static char Large_blip_string[2];

void radar_frame_init_orb()
{
	radar_null_nblips();
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

void radar_orb_draw_contact(vec3d *pnt, int rad)
{
	vertex verts[2];
	vec3d p;

	p=*pnt;
	vm_vec_normalize(&p);

	g3_rotate_vertex(&verts[0], &p);
	g3_project_vertex(&verts[0]);

	g3_rotate_vertex(&verts[1], pnt);
	g3_project_vertex(&verts[1]);

	float size = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);
	if (size < i2fl(rad))	size = i2fl(rad);
 
	if (rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res])
	{
		g3_draw_sphere(&verts[1],size/100.0f);
	}
	else
	{
		g3_draw_sphere(&verts[1],size/300.0f);
	}

	g3_draw_line(&verts[0],&verts[1]);

}

void radar_orb_draw_contact_htl(vec3d *pnt, int rad)
{
	vec3d p;

	p=*pnt;

	vm_vec_normalize(&p);

    float size = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);
	if (size < i2fl(rad))	size = i2fl(rad);
 
	if (rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res])
	{
		if (radar_target_id_flags & RTIF_PULSATE) {
			// use mask to make the darn thing work faster
			size *= 1.3f + (sinf(10 * f2fl(Missiontime)) * 0.3f);
		}
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		g3_draw_htl_sphere(pnt,size/100.0f);
	}
	else
	{
		g3_draw_htl_sphere(pnt,size/300.0f);
	}

	g3_draw_htl_line(&p,pnt);

}

void radar_draw_circle_orb( int x, int y, int rad )
{
	Int3();
	//This shouldn't be called in orb mode since we need the 3d position of the contact to draw
}

// radar is damaged, so make blips dance around
void radar_blip_draw_distorted_orb(blip *b)
{
	float scale;
	float dist=vm_vec_normalize(&b->position);
	vec3d out;
	float distortion_angle=20;
	
	// maybe alter the effect if EMP is active
	if(emp_active_local()){
		scale = emp_current_intensity();
		distortion_angle *= frand_range(-3.0f,3.0f)*frand_range(0.0f, scale);
		dist *= frand_range(MAX(0.75f, 0.75f*scale), MIN(1.25f, 1.25f*scale));

		if (dist > 1.25f) dist = 1.25f;
		if (dist < 0.75f) dist = 0.75f;
	}

	vm_vec_random_cone(&out,&b->position,distortion_angle);
	vm_vec_scale(&out,dist);

    if (Cmdline_nohtl)
    {
	    radar_orb_draw_contact(&out,b->rad);
    }
    else
    {
        radar_orb_draw_contact_htl(&out,b->rad);
    }
}

// blip is for a target immune to sensors, so cause to flicker in/out with mild distortion
void radar_blip_draw_flicker_orb(blip *b)
{
	int flicker_index;

	float dist=vm_vec_normalize(&b->position);
	vec3d out;
	float distortion_angle=10;

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

		distortion_angle *= frand_range(0.1f,2.0f);
		dist *= frand_range(0.75f, 1.25f);

		if (dist > 1.25f) dist = 1.25f;
		if (dist < 0.75f) dist = 0.75f;
	}
	
	vm_vec_random_cone(&out,&b->position,distortion_angle);
	vm_vec_scale(&out,dist);

	if (Cmdline_nohtl)
    {
	    radar_orb_draw_contact(&out,b->rad);
    }
    else
    {
        radar_orb_draw_contact_htl(&out,b->rad);
    }
}

// Draw all the active radar blips
void draw_radar_blips_orb(int blip_type, int bright, int distort)
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
			target_position = b->position;
		}
		else
		{
			b->rad = Current_radar_global->Radar_blip_radius_normal[gr_screen.res];
		}

		// maybe distort blip
		if (distort)
		{
			radar_blip_draw_distorted(b);
		}
		else if (b->flags & BLIP_DRAW_DISTORTED)
		{
			radar_blip_draw_flicker(b);
		}
		else
		{
            if (Cmdline_nohtl)
            {
                radar_orb_draw_contact(&b->position,b->rad);
            }
            else if (b->radar_image_2d >= 0)
			{
				radar_orb_draw_image(&b->position, b->rad, b->radar_image_2d, b->radar_projection_size);
			}
            else
            {
                radar_orb_draw_contact_htl(&b->position,b->rad);
            }
        }
	}
}

// Draw the radar blips
// input:	distorted	=>		0 (default) to draw normal, 1 to draw distorted 
void radar_draw_blips_sorted_orb(int distort)
{
	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, false);

	vm_vec_zero(&target_position);
	// draw dim blips first, then bright blips
	for (int is_bright = 0; is_bright < 2; is_bright++)
	{
		draw_radar_blips_orb(BLIP_TYPE_JUMP_NODE, is_bright, distort);
		draw_radar_blips_orb(BLIP_TYPE_WARPING_SHIP, is_bright, distort);
		draw_radar_blips_orb(BLIP_TYPE_NAVBUOY_CARGO, is_bright, distort);
		draw_radar_blips_orb(BLIP_TYPE_NORMAL_SHIP, is_bright, distort);
		draw_radar_blips_orb(BLIP_TYPE_BOMB, is_bright, distort);
		draw_radar_blips_orb(BLIP_TYPE_TAGGED_SHIP, is_bright, distort);
	}

	if (radar_target_id_flags & RTIF_CROSSHAIRS) {
		draw_radar3d_crosshairs(target_position);
	}

	g3_done_instance(false);
}

void radar_draw_range_orb()
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

extern void hud_save_restore_camera_data(int);
extern float View_zoom;

void radar_orb_setup_view()
{
	hud_save_restore_camera_data(1);

	g3_end_frame();

	int w,h;
	bm_get_info(Radar_gauge.first_frame,&w, &h, NULL, NULL, NULL);
	
	HUD_set_clip(Current_radar_global->Radar_coords[gr_screen.res][0], Current_radar_global->Radar_coords[gr_screen.res][1],w, h);
	g3_start_frame(1);
	
	float old_zoom=View_zoom;
	View_zoom=.75;

	g3_set_view_matrix( &Orb_eye_position, &vmd_identity_matrix, View_zoom);

	View_zoom=old_zoom;
}

void radar_orb_setup_view_htl()
{
    int w,h;
	bm_get_info(Radar_gauge.first_frame,&w, &h, NULL, NULL, NULL);
    
    HUD_set_clip(Current_radar_global->Radar_coords[gr_screen.res][0],
                 Current_radar_global->Radar_coords[gr_screen.res][1],
                 w, h);

    gr_set_proj_matrix( .625f * PI_2, float(w)/float(h), 0.001f, 5.0f);
	gr_set_view_matrix( &Orb_eye_position, &vmd_identity_matrix );

    gr_zbuffer_set(0);
}

void radar_orb_done_drawing()
{
	g3_done_instance(false);
	g3_end_frame();
	g3_start_frame(0);
	hud_save_restore_camera_data(0);
	HUD_reset_clip();
}

void radar_orb_done_drawing_htl()
{
	gr_end_view_matrix();
	gr_end_proj_matrix();
	hud_save_restore_camera_data(0);
	HUD_reset_clip();
    gr_zbuffer_set(1);
}

void radar_orb_draw_outlines()
{
	int i;
	vertex center;
//	vertex extents[6];
	vertex proj_orb_lines_xy[NUM_ORB_RING_SLICES];
	vertex proj_orb_lines_xz[NUM_ORB_RING_SLICES];
	vertex proj_orb_lines_yz[NUM_ORB_RING_SLICES];

	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, false);
	g3_start_instance_matrix(&vmd_zero_vector, &Player_obj->orient, false);

	g3_rotate_vertex(&center, &vmd_zero_vector);
	g3_rotate_vertex(&proj_orb_lines_xy[0], &orb_ring_xy[0]);
	g3_rotate_vertex(&proj_orb_lines_yz[0], &orb_ring_yz[0]);
	g3_rotate_vertex(&proj_orb_lines_xz[0], &orb_ring_xz[0]);

	g3_project_vertex(&center);
	gr_set_color(255,255,255);
	g3_draw_sphere(&center, .05f);

	g3_project_vertex(&proj_orb_lines_xy[0]);
	g3_project_vertex(&proj_orb_lines_yz[0]);
	g3_project_vertex(&proj_orb_lines_xz[0]);

	for (i=1; i < NUM_ORB_RING_SLICES; i++)
	{
		g3_rotate_vertex(&proj_orb_lines_xy[i], &orb_ring_xy[i]);
		g3_rotate_vertex(&proj_orb_lines_yz[i], &orb_ring_yz[i]);
		g3_rotate_vertex(&proj_orb_lines_xz[i], &orb_ring_xz[i]);

		g3_project_vertex(&proj_orb_lines_xy[i]);
		g3_project_vertex(&proj_orb_lines_yz[i]);
		g3_project_vertex(&proj_orb_lines_xz[i]);
		
		gr_set_color(192,96,32);
		g3_draw_sphere(&proj_orb_lines_xy[i-1], .01f);
		g3_draw_sphere(&proj_orb_lines_xz[i-1], .01f);
		g3_draw_line(&proj_orb_lines_xy[i-1],&proj_orb_lines_xy[i]);
		g3_draw_line(&proj_orb_lines_xz[i-1],&proj_orb_lines_xz[i]);

		gr_set_color(112,16,192);
		g3_draw_sphere(&proj_orb_lines_yz[i-1], .01f);
		g3_draw_line(&proj_orb_lines_yz[i-1],&proj_orb_lines_yz[i]);
	}

	g3_done_instance(false);
}

int radar_orb_calc_alpha(vec3d* pt)
{
    Assert(pt);
    Assert(Player_obj);

    vec3d new_pt;
    vec3d fvec = {0.0f, 0.0f, 1.0f};

    vm_vec_unrotate(&new_pt, pt, &Player_obj->orient);
    vm_vec_normalize(&new_pt);

    float dot = vm_vec_dotprod(&fvec, &new_pt);
    float angle = fabs(acos(dot));
    int alpha = int(angle*192.0f/PI);
    
    return alpha;
}

void radar_orb_draw_outlines_htl()
{
	int i, last = NUM_ORB_RING_SLICES - 1;

	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, true);
	g3_start_instance_matrix(&vmd_zero_vector, &Player_obj->orient, true);

	gr_set_color(255, 255, 255);
	g3_draw_htl_sphere(&vmd_zero_vector, .05f);

    gr_set_line_width(2.0f);

	for (i = 0; i < NUM_ORB_RING_SLICES; i++)
	{
        gr_init_alphacolor(&Orb_color_orange, 192, 96, 32, radar_orb_calc_alpha(&orb_ring_xy[last]));
		gr_set_color_fast(&Orb_color_orange);
		g3_draw_htl_line(&orb_ring_xy[last],&orb_ring_xy[i]);

        gr_init_alphacolor(&Orb_color_teal, 48, 160, 96, radar_orb_calc_alpha(&orb_ring_xz[last]));
        gr_set_color_fast(&Orb_color_teal);
		g3_draw_htl_line(&orb_ring_xz[last],&orb_ring_xz[i]);

        gr_init_alphacolor(&Orb_color_purple, 112, 16, 192, radar_orb_calc_alpha(&orb_ring_yz[last]));
		gr_set_color_fast(&Orb_color_purple);
		g3_draw_htl_line(&orb_ring_yz[last],&orb_ring_yz[i]);

        last = i;
	}

    gr_set_line_width(1.0f);

	g3_done_instance(true);
    g3_done_instance(true);
}

void radar_frame_render_orb(float frametime)
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
	radar_blit_gauge();
	radar_draw_range();

    if (Cmdline_nohtl)
    {
        radar_orb_setup_view();
        radar_orb_draw_outlines();
    }
    else
    {
        radar_orb_setup_view_htl();
        radar_orb_draw_outlines_htl();
    }
	

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
	
    if (Cmdline_nohtl)
    {
	    radar_orb_done_drawing();
    }
    else
    {
        radar_orb_done_drawing_htl();
    }
}

void radar_blit_gauge_orb()
{
	SPECMAP = -1;
	GLOWMAP = -1;

	gr_set_bitmap(Radar_gauge.first_frame+1);
	gr_aabitmap( Current_radar_global->Radar_coords[gr_screen.res][0], Current_radar_global->Radar_coords[gr_screen.res][1] );
} 

void radar_page_in_orb()
{
	bm_page_in_aabitmap( Radar_gauge.first_frame, Radar_gauge.num_frames );
}

void radar_orb_draw_image(vec3d *pnt, int rad, int idx, float mult)
{
    int tmap_flags = 0;
    int h, w;
    float aspect_mp;

    // need to get bitmap info
    bm_get_info(idx, &w, &h);

    Assert(w > 0);

    // get multiplier
    if (h == w) {
        aspect_mp = 1.0f;
    } else {
        aspect_mp = (((float) h) / ((float) w));
    }

    gr_set_bitmap(idx,GR_ALPHABLEND_NONE,GR_BITBLT_MODE_NORMAL,1.0f);

    float sizef = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);

    // might need checks unless the targeted blip is always wanted to be larger
    float radius = (float) Current_radar_global->Radar_blip_radius_normal[gr_screen.res];

    if (sizef < radius)
        sizef = radius;

    //Make so no evil things happen
    Assert(mult > 0.0f);

    //modify size according to value from tables
    sizef *= mult;

	// animate the targeted icon - option 1 of highlighting the targets
	if ( rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res] ) {
		if (radar_target_id_flags & RTIF_PULSATE) {
			// use mask to make the darn thing work faster
			sizef *= 1.3f + (sinf(10 * f2fl(Missiontime)) * 0.3f);
		}
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		if (radar_target_id_flags & RTIF_ENLARGE) {
			sizef *= 1.3f;
		}
	}

    tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_BW_TEXTURE | TMAP_HTL_3D_UNLIT;

    g3_draw_polygon(pnt, &vmd_identity_matrix, sizef/35.0f, aspect_mp*sizef/35.0f, tmap_flags);
}

void draw_radar3d_crosshairs( vec3d pnt )
{
	int i,j,m;
	vec3d pnt_end, pnt_start;

	gr_set_color_fast(&Orb_crosshairs);

	for(i = 0; i < 2; i++) {
		m = (i * 2) - 1;
		pnt_end = pnt_start = pnt;
		pnt_start.xyz.x += (float) m*0.05f;
		pnt_end.xyz.x += (float) m*0.15f;
		g3_draw_htl_line(&pnt_start, &pnt_end);
	}
	for(j = 0; j < 2; j++) {
		m = (j * 2) - 1;
		pnt_end = pnt_start = pnt;
		pnt_start.xyz.y += (float) m*0.05f;
		pnt_end.xyz.y += (float) m*0.15f;
		g3_draw_htl_line(&pnt_start, &pnt_end);
	}
}
