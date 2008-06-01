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
 * $Revision: 1.23.2.4 $
 * $Date: 2008-01-19 00:27:10 $
 * $Author: Goober5000 $
 *
 * C module containg functions to display and manage the "orb" radar mode
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.23.2.3  2007/08/17 03:29:51  Goober5000
 * generalize the way radar ranges are handled (inspired by Shade's fix)
 *
 * Revision 1.23.2.2  2007/02/11 09:12:12  taylor
 * little bit of cleanup
 * more fixage for hidden jumpnodes (Mantis #1149)
 *
 * Revision 1.23.2.1  2007/02/10 00:16:57  taylor
 * phreak missed fixing this here when he fixed it in unstable
 *
 * Revision 1.23  2006/04/12 22:23:41  taylor
 * compiler warning fixes to make GCC 4.1 shut the hell up
 *
 * Revision 1.22  2006/01/16 11:02:23  wmcoolmon
 * Various warning fixes, scripting globals fix; added "plr" and "slf" global variables for in-game hooks; various lua functions; GCC fixes for scripting.
 *
 * Revision 1.21  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 1.20  2006/01/14 04:44:29  phreak
 * Tried to make the sphere size more uniform on opposite ends of the orb
 * Modified the radar disruption angle scale a bit.  Went from 200 degree to 20 degree disruption.  Otherwise, it was very confusing to find hidden objects.
 *
 * Revision 1.19  2006/01/14 03:28:29  phreak
 * Make orb radar mode drawing again.
 * Clarify "fudge" variable used for viewing transformations
 *
 * Revision 1.18  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 1.17  2005/10/10 17:21:09  taylor
 * remove NO_NETWORK
 *
 * Revision 1.16  2005/09/17 19:12:37  phreak
 * radar bitmap should render the correct color using -orbradar
 *
 * Revision 1.15  2005/07/25 03:13:25  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 1.14  2005/07/22 10:18:40  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 1.13  2005/07/13 03:35:35  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.12  2005/07/07 16:36:58  taylor
 * various compiler warning fixes (some of these from dizzy)
 *
 * Revision 1.11  2005/04/24 12:56:43  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
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
 * more network/inferno goodness for Windows, takes care of a few warnings too
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

extern int Cmdline_nohtl;

vec3d orb_ring_yz[25];
vec3d orb_ring_xy[25];
vec3d orb_ring_xz[25];
vec3d vec_extents[]=
{
	{ { { 1.0f, 0.0f, 0.0f } } },
	{ { { 0.0f, 1.0f, 0.0f } } },
	{ { { 0.0f, 0.0f, 1.0f } } },
	{ { { -1.0f, 0.0f, 0.0f } } },
	{ { { 0.0f, -1.0f, 0.0f } } },
	{ { { 0.0f, 0.0f, -1.0f } } }
};

//special view matrix to get the orb rotating the correct war
static matrix view_perturb = { { { { { { 1.0f, 0.0f, 0.0f } } }, { { { 0.0f,1.0f,0.0f } } }, { { { 0.0f,0.0f,-1.0f } } } } } };
static vec3d Orb_eye_position = { { { 0.0f, 0.0f, -2.5f } } };

// forward declarations
void draw_radar_blips_orb(int blip_type, int bright, int distort = 0);

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
			gr_init_alphacolor( &Radar_colors[i][j], Radar_color_rgb[i][j].r, Radar_color_rgb[i][j].g, Radar_color_rgb[i][j].b, 255 );
		}
	}

	memset(orb_ring_xy, 0, sizeof(orb_ring_xy));
	memset(orb_ring_xz, 0, sizeof(orb_ring_xz));
	memset(orb_ring_yz, 0, sizeof(orb_ring_yz));
	
    for (i=0; i < 25; i++)
    {
        s=(float)sin(float(i*PI)/12.0);
        c=(float)cos(float(i*PI)/12.0);

        orb_ring_xy[i].xyz.x = c;
        orb_ring_xy[i].xyz.y = s;

        orb_ring_yz[i].xyz.y = c;
        orb_ring_yz[i].xyz.z = s;

        orb_ring_xz[i].xyz.x = c;
        orb_ring_xz[i].xyz.z = s;
    }

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
				*blip_color = iff_get_color_by_team(shipp->team, Player_ship->team, is_bright);
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
			if ( !(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_BOMB) )
				return;

			// if we don't attack the bomb, return
			if ( !iff_x_attacks_y(Player_ship->team, obj_team(objp)) )
				return;

			// if a local ssm is in subspace, return
			if (Weapons[objp->instance].lssm_stage == 3)
				return;

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

	vm_vec_normalize(&pos);

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

	// see if blip should be drawn distorted
	if (objp->type == OBJ_SHIP)
	{
		// ships specifically hidden from sensors
		if (Ships[objp->instance].flags & SF_HIDDEN_FROM_SENSORS)
			b->flags |= BLIP_DRAW_DISTORTED;

		// determine if its AWACS distorted
		if (awacs_level < 1.0f)
			b->flags |= BLIP_DRAW_DISTORTED;
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

    gr_set_proj_matrix( .65 * PI_2, float(w)/float(h), 0.001, 5.0);
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
	g3_start_frame(0);
	hud_save_restore_camera_data(0);
	HUD_reset_clip();
    gr_zbuffer_set(1);
}

void radar_orb_draw_outlines()
{
	int i;
	vertex center;
//	vertex extents[6];
	vertex proj_orb_lines_xy[25];
	vertex proj_orb_lines_xz[25];
	vertex proj_orb_lines_yz[25];

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

void radar_orb_draw_outlines_htl()
{
	int i;

	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, true);
	g3_start_instance_matrix(&vmd_zero_vector, &Player_obj->orient, true);

	gr_set_color(255,255,255);
	g3_draw_htl_sphere(&vmd_zero_vector, .05f);

	for (i=1; i < 25; i++)
	{
		gr_set_color(192,96,32);
		g3_draw_htl_line(&orb_ring_xy[i-1],&orb_ring_xy[i]);

        gr_set_color(48,160,96);
		g3_draw_htl_line(&orb_ring_xz[i-1],&orb_ring_xz[i]);

		gr_set_color(112,16,192);
		g3_draw_htl_line(&orb_ring_yz[i-1],&orb_ring_yz[i]);
	}

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
