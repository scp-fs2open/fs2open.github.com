/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Radar/Radarorb.cpp $
 * $Revision: 1.11 $
 * $Date: 2005-04-24 12:56:43 $
 * $Author: taylor $
 *
 * C module containg functions to display and manage the "orb" radar mode
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2005/04/05 05:53:23  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.9  2005/03/25 06:57:37  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 1.8  2005/03/24 23:29:33  taylor
 * (re)move some uneeded variables to fix compiler warnings
 *
 * Revision 1.7  2005/03/21 23:19:55  phreak
 * Orb correctly displays in 640x480
 *
 * Revision 1.6  2005/03/13 08:33:55  taylor
 * gotta use MIN/MAX and not min/max now
 *
 * Revision 1.5  2005/03/12 06:03:07  phreak
 * Fixed a bug where the orb would incorrectly render under alternate FOVs.
 * Updated the functions that scramble the contacts so the blips show up
 * somewhat near where they should be.
 *
 * Revision 1.4  2005/03/02 21:24:46  taylor
 * more NO_NETWORK/INF_BUILD goodness for Windows, takes care of a few warnings too
 *
 * Revision 1.3  2005/02/04 20:06:07  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.2  2004/08/23 07:48:09  Goober5000
 * fix0red some warnings
 * --Goober5000
 *
 * Revision 1.1  2004/08/02 22:42:45  phreak
 * orb radar rendering style
 *

 *
 * $NoKeywords: $
 *
 */

#include "PreProcDefines.h"

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

extern float radx, rady;

extern float	Radar_dim_range;					// range at which we start dimming the radar blips
extern int		Radar_calc_dim_dist_timer;		// timestamp at which we recalc Radar_dim_range

extern int Radar_flicker_timer[NUM_FLICKER_TIMERS];					// timestamp used to flicker blips on and off
extern int Radar_flicker_on[NUM_FLICKER_TIMERS];						// status of flickering

extern rcol Radar_color_rgb[MAX_RADAR_LEVELS][MAX_RADAR_COLORS];
extern color Radar_colors[MAX_RADAR_LEVELS][MAX_RADAR_COLORS];

extern blip	Blip_bright_list[MAX_RADAR_COLORS];		// linked list of bright blips
extern blip	Blip_dim_list[MAX_RADAR_COLORS];			// linked list of dim blips
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

vec3d orb_ring_yz[25];
vec3d orb_ring_xy[25];
vec3d orb_ring_xz[25];
vec3d vec_extents[]=
{
	{1,0,0},
	{0,1,0},
	{0,0,1},
	{-1,0,0},
	{0,-1,0},
	{0,0,-1}
};

// forward declarations
void draw_radar_blips_orb(int desired_color, int is_dim, int distort=0);

void radar_init_orb()
{
	int i,j;
	float s,c;

	Radar_gauge.first_frame = bm_load_animation(Current_radar_global->Radar_fname[gr_screen.res], &Radar_gauge.num_frames);
	if ( Radar_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", Current_radar_global->Radar_fname[gr_screen.res]);
	}

	for (i=0; i<MAX_RADAR_LEVELS; i++ )	{
		for (j=0; j<MAX_RADAR_COLORS; j++ )	{
			gr_init_alphacolor( &Radar_colors[i][j], Radar_color_rgb[i][j].r, Radar_color_rgb[i][j].g, Radar_color_rgb[i][j].b, 255 );
		}
	}

	memset(orb_ring_xy,25*sizeof(vec3d),0);
	memset(orb_ring_yz,25*sizeof(vec3d),0);
	memset(orb_ring_xz,25*sizeof(vec3d),0);
	
	for (i=0; i < 25; i++)
	{
		s=(float)sin(float(i*PI)/12.0);
		c=(float)cos(float(i*PI)/12.0);

		orb_ring_xy[i].xyz.x=orb_ring_yz[i].xyz.y=orb_ring_xz[i].xyz.x=c;
		orb_ring_xy[i].xyz.y=orb_ring_yz[i].xyz.z=orb_ring_xz[i].xyz.z=s;
	}

	Blip_mutate_id	= 1;
}

// determine what color the object blip should be drawn as
int radar_blip_color_orb(object *objp)
{
	int	color = 0;
	ship	*shipp = NULL;

	switch(objp->type) {
	case OBJ_SHIP:
		shipp = &Ships[objp->instance];
		if ( shipp->flags & SF_ARRIVING_STAGE_1 )	{
			color = RCOL_WARPING_SHIP;
		} else if ( ship_is_tagged(objp) ) {
			color = RCOL_TAGGED;
		} else if ( Ship_info[shipp->ship_info_index].flags & (SIF_NAVBUOY|SIF_CARGO) ) {
			color = RCOL_NAVBUOYS;
		} else {
			if ( (Player_ship->team == shipp->team) && (Player_ship->team != TEAM_TRAITOR) ) {
				color = RCOL_FRIENDLY;
			} else {
				switch (shipp->team) {
				case TEAM_FRIENDLY:
				case TEAM_HOSTILE:
				case TEAM_TRAITOR:
					color = RCOL_HOSTILE;
					break;
				case TEAM_NEUTRAL:
					color = RCOL_NEUTRAL;
					break;
				case TEAM_UNKNOWN:
					color = RCOL_UNKNOWN;
					break;
				default:
					color = RCOL_HOSTILE;
					Int3();	//	Bogus team id in shipp->team
					break;
				}
			}
		}
		break;
	case OBJ_WEAPON:
		if ((Weapons[objp->instance].lssm_stage==2) || (Weapons[objp->instance].lssm_stage==4))
			color=RCOL_WARPING_SHIP;
		else
			color = RCOL_BOMB;
		break;
	case OBJ_JUMP_NODE:
		color = RCOL_JUMP_NODE;
		break;
	default:
		Error(LOCATION, "Illegal ship type in radar.");
		break;
	}

	return color;
}


void radar_plot_object_orb( object *objp )	
{
	vec3d	pos, tempv;
	float		dist, max_radar_dist;
	//float rscale, zdist;
	//int		xpos, ypos;
	int color=0;
	vec3d	world_pos = objp->pos;	
	float		awacs_level;

#ifndef NO_NETWORK
	// don't process anything here.  Somehow, a jumpnode object caused this function
	// to get entered on server side.
	if( Game_mode & GM_STANDALONE_SERVER ){
		return;
	}

	// multiplayer clients ingame joining should skip this function
	if ( MULTIPLAYER_CLIENT && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN) ){
		return;
	}
#endif

	// get team-wide awacs level for the object if not ship
	int ship_is_visible = 0;
	if (objp->type == OBJ_SHIP) {
		if (Player_ship != NULL) {
			if (ship_is_visible_by_team_new(objp, Player_ship)) {
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
	switch ( objp->type ) {
	case OBJ_SHIP:
		// Place to cull ships, such as NavBuoys		
		break;
		
	case OBJ_JUMP_NODE:
		// filter jump nodes here if required
		break;

	case OBJ_WEAPON: {
		// if not a bomb, return
		if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_BOMB) ) {
			return;
		}

		// if bomb is on same team as player, return
		if ( (obj_team(objp) == Player_ship->team) && (Player_ship->team != TEAM_TRAITOR) ) {
			return;
		}

		//if a local ssm is in subspace, return
		if (Weapons[objp->instance].lssm_stage==3)
			return;
		break;
	}

	default:
		return;			// if any other kind of object, don't want to show on radar
		break;
	} // end switch

		// Apply range filter
	dist = vm_vec_dist(&world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if ( dist > max_radar_dist ){
		return;
	}
	
	color = radar_blip_color(objp);

	// Determine the distance at which we will dim the radar blip
	if ( timestamp_elapsed(Radar_calc_dim_dist_timer) ) {
		Radar_calc_dim_dist_timer=timestamp(1000);
		Radar_dim_range = player_farthest_weapon_range();
		if ( Radar_dim_range <= 0 ) {
			Radar_dim_range=1500.0f;
		}
	}
	
	blip	*b;
	int blip_dim=0;

	if ( dist > Radar_dim_range ) {
		blip_dim=1;
	}

	if ( N_blips >= MAX_BLIPS ) {
		// out of blips, don't plot
		//Gahhh, this is bloody annoying -WMC
		//Int3();
		return;
	}

	b = &Blips[N_blips];
	b->flags=0;


	// JAS -- new way of getting the rotated point that doesn't require this to be
	// in a g3_start_frame/end_frame block.

	vm_vec_sub(&tempv,&world_pos,&Player_obj->pos);
	vm_vec_rotate( &pos, &tempv, &Player_obj->orient);
	vm_vec_normalize(&pos);
	float scale = dist/Radar_dim_range;
	if (scale > 1.25f) scale = 1.25f;
	if (scale < .75f) scale = .75f;

	b->position=pos;

	vm_vec_scale(&b->position, scale);

	// flag the blip as a current target if it is
	if (OBJ_INDEX(objp) == Player_ai->target_objnum)	{
		b->flags |= BLIP_CURRENT_TARGET;
		blip_dim = 0;
	}

	if ( blip_dim ) {
		list_append( &Blip_dim_list[color], b );
	} else {
		list_append( &Blip_bright_list[color], b );
	}

//	b->x = xpos;
//	b->y = ypos;


	// see if blip should be drawn distorted
	if (objp->type == OBJ_SHIP)
	{
		// ships specifically hidden from sensors
		if ( Ships[objp->instance].flags & SF_HIDDEN_FROM_SENSORS ) {
			b->flags |= BLIP_DRAW_DISTORTED;
		}

		// determine if its AWACS distorted
		if ( awacs_level < 1.0f )
		{
			// check if it's 
			b->flags |= BLIP_DRAW_DISTORTED;
		}
	}				

	// don't distort the sensor blips if the player has primitive sensors and the nebula effect
	// is not active
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
	{
		if (!(The_mission.flags & MISSION_FLAG_FULLNEB))
		{
			b->flags &= ~BLIP_DRAW_DISTORTED;
		}
	}

	N_blips++;
}

// set N_blips for each color/brightness level to zero
void radar_null_nblips_orb()
{
	int i;

	N_blips=0;

	for (i=0; i<MAX_RADAR_COLORS; i++) {
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
	Radar_calc_dim_dist_timer = timestamp(0);

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
 
	if (rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res])
	{
		g3_draw_sphere(&verts[1],(float)Current_radar_global->Radar_blip_radius_target[gr_screen.res]/100.0f);
	}
	else
	{
		g3_draw_sphere(&verts[1],(float)Current_radar_global->Radar_blip_radius_target[gr_screen.res]/300.0f);
	}

	g3_draw_line(&verts[0],&verts[1]);

}

void radar_draw_circle_orb( int x, int y, int rad )
{
	Int3();
	if ( rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res] )	{
		gr_string( Large_blip_offset_x+x, Large_blip_offset_y+y, Large_blip_string );
	} else {
		// rad = RADAR_BLIP_RADIUS_NORMAL;
		gr_string( Small_blip_offset_x+x, Small_blip_offset_y+y, Small_blip_string );
	}
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

	radar_orb_draw_contact(&out,b->rad);
}

// blip is for a target immune to sensors, so cause to flicker in/out with mild distortion
void radar_blip_draw_flicker_orb(blip *b)
{
	int flicker_index;

	float dist=vm_vec_normalize(&b->position);
	vec3d out;
	float distortion_angle=20;

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

		distortion_angle *= frand_range(-2.0f,2.0f)*frand_range(0.0f, 10.0f);
		dist *= frand_range(0.75f, 1.25f);

		if (dist > 1.25f) dist = 1.25f;
		if (dist < 0.75f) dist = 0.75f;
	}
	
	vm_vec_random_cone(&out,&b->position,distortion_angle);
	vm_vec_scale(&out,dist);

	radar_orb_draw_contact(&out,b->rad);
}

// Draw all the active radar blips
void draw_radar_blips_orb(int rcol, int is_dim, int distort)
{
	blip	*b=NULL;
	blip	*blip_head=NULL;

	// Need to set font.
	gr_set_font(FONT1);

	if ( is_dim ) {
		blip_head = &Blip_dim_list[rcol];
	} else {
		blip_head = &Blip_bright_list[rcol];
	}

	for ( b = GET_FIRST(blip_head); b !=END_OF_LIST(blip_head); b = GET_NEXT(b) )	{

		Assert((rcol >= 0) && (rcol < MAX_RADAR_COLORS));

		if ( is_dim ) {
			gr_set_color_fast( &Radar_colors[RADAR_BLIP_DIM][rcol] );
		} else {
			gr_set_color_fast( &Radar_colors[RADAR_BLIP_BRIGHT][rcol] );
		}

		if (b->flags & BLIP_CURRENT_TARGET) {
			// draw cool blip to indicate current target
			b->rad = Current_radar_global->Radar_blip_radius_target[gr_screen.res];				
		} else {
			b->rad = Current_radar_global->Radar_blip_radius_normal[gr_screen.res];
		}

		if ( distort ) { 
			radar_blip_draw_distorted(b);
		} else if ( b->flags & BLIP_DRAW_DISTORTED ) {
			radar_blip_draw_flicker(b);
		} else{
			radar_orb_draw_contact(&b->position, b->rad);
			//radar_draw_circle( b->x, b->y, b->rad );
		}
	}
}

// Draw the radar blips
// input:	distorted	=>		0 (default) to draw normal, 1 to draw distorted 
void radar_draw_blips_sorted_orb(int distort)
{
	static matrix fudge={1,0,0,0,1,0,0,0,-1};
	matrix m;
	
	vm_vector_2_matrix(&m,&Player_obj->orient.vec.fvec,NULL,NULL);
//	g3_start_instance_matrix(&vmd_zero_vector, &m, false);
	g3_start_instance_matrix(&vmd_zero_vector, &fudge, false);

	// draw dim blips first
	draw_radar_blips_orb(RCOL_JUMP_NODE, 1, distort);
	draw_radar_blips_orb(RCOL_WARPING_SHIP, 1, distort);
	draw_radar_blips_orb(RCOL_NAVBUOYS, 1, distort);
	draw_radar_blips_orb(RCOL_FRIENDLY, 1, distort);
	draw_radar_blips_orb(RCOL_UNKNOWN, 1, distort);
	draw_radar_blips_orb(RCOL_BOMB, 1, distort);
	draw_radar_blips_orb(RCOL_NEUTRAL, 1, distort);
	draw_radar_blips_orb(RCOL_HOSTILE, 1, distort);
	draw_radar_blips_orb(RCOL_TAGGED, 1, distort);

	// draw bright blips
	draw_radar_blips_orb(RCOL_JUMP_NODE, 0, distort);
	draw_radar_blips_orb(RCOL_WARPING_SHIP, 0, distort);
	draw_radar_blips_orb(RCOL_NAVBUOYS, 0, distort);
	draw_radar_blips_orb(RCOL_FRIENDLY, 0, distort);
	draw_radar_blips_orb(RCOL_UNKNOWN, 0, distort);
	draw_radar_blips_orb(RCOL_BOMB, 0, distort);
	draw_radar_blips_orb(RCOL_NEUTRAL, 0, distort);
	draw_radar_blips_orb(RCOL_HOSTILE, 0, distort);
	draw_radar_blips_orb(RCOL_TAGGED, 0, distort);

	//g3_done_instance(false);
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

	//draw the guide circles
	static vec3d eye={0,0,-2.25};
	g3_end_frame();

	int w,h;
	bm_get_info(Radar_gauge.first_frame,&w, &h, NULL, NULL, NULL);
	
	HUD_set_clip(Current_radar_global->Radar_coords[gr_screen.res][0], Current_radar_global->Radar_coords[gr_screen.res][1],w, h);
	g3_start_frame(1);
	
	float old_zoom=View_zoom;
	View_zoom=.75;

	g3_set_view_matrix( &eye, &vmd_identity_matrix, View_zoom);

	View_zoom=old_zoom;
}

void radar_orb_done_drawing()
{
	g3_done_instance(false);
	g3_end_frame();
	g3_start_frame(0);
	hud_save_restore_camera_data(0);
	HUD_reset_clip();
}

void radar_orb_draw_outlines()
{
	int i;
	vertex center;
//	vertex extents[6];
	vertex proj_orb_lines_xy[25];
	vertex proj_orb_lines_xz[25];
	vertex proj_orb_lines_yz[25];

	static matrix fudge={1,0,0,0,1,0,0,0,-1};
	g3_start_instance_matrix(&vmd_zero_vector, &fudge, false);

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

	for (i=1; i < 25; i++)
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
	radar_draw_range();
	radar_blit_gauge();

	radar_orb_setup_view();
	radar_orb_draw_outlines();

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
	
	radar_orb_done_drawing();
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
