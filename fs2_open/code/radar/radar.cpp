/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Radar/Radar.cpp $
 * $Revision: 2.19 $
 * $Date: 2005-10-10 17:21:09 $
 * $Author: taylor $
 *
 * C module containg functions to display and manage the radar
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.18  2005/07/25 03:13:25  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 2.17  2005/07/22 10:18:40  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.16  2005/07/13 03:35:35  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.15  2005/04/05 05:53:23  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.14  2005/03/24 23:29:33  taylor
 * (re)move some uneeded variables to fix compiler warnings
 *
 * Revision 2.13  2005/03/02 21:24:46  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.12  2005/02/04 20:06:07  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.11  2004/08/02 22:54:07  phreak
 * orb radar rendering style
 *
 * Revision 2.10  2004/07/26 20:47:50  Kazan
 * remove MCD complete
 *
 * Revision 2.9  2004/07/12 16:33:03  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.8  2004/07/03 06:08:54  wmcoolmon
 * Removed function pointer w/ default arguments for compatibility with .NET Sorry, you'll just have to add that "0" by hand. :p
 *
 * Revision 2.7  2004/07/01 01:51:54  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.6  2004/03/05 09:02:12  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.5  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.4  2003/06/11 03:15:17  phreak
 * i hate to update again so soon, but i forgot to mark hostile local ssms that
 * were in stages 2 or 4 as warping so their blip color would then be blue
 *
 * Revision 2.3  2003/06/11 03:01:27  phreak
 * the radar will not show stage 3 local ssms
 *
 * Revision 2.2  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/13 21:09:28  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    8/23/99 8:38p Andsager
 * Added see_all debug console command for turning radar range infinite in
 * nebula (but not targeting).
 * 
 * 13    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 12    6/07/99 4:21p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 11    6/02/99 12:52p Andsager
 * Added team-wide ship visibility.  Implemented for player.
 * 
 * 10    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 9     1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 8     12/30/98 9:34a Jasen
 * updated coords for hi res
 * 
 * 7     12/29/98 7:29p Dave
 * Added some missing hi-res hud coord globalizations.
 * 
 * 6     12/29/98 2:30p Jasen
 * added some new coords for 1024 HUD stuff
 * 
 * 5     12/28/98 3:17p Dave
 * Support for multiple hud bitmap filenames for hi-res mode.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 90    8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 89    8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 88    6/12/98 4:52p Hoffoss
 * Added support for special characters in in forgeign languages.
 * 
 * 87    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 86    5/19/98 10:26a John
 * Fixed bug with radar blips not drawing in hardware.
 * 
 * 85    5/19/98 9:12a John
 * Made radar blips render as font characters 132 and 133.   Needs a new
 * font01.vf in the data tree.
 * 
 * 84    5/08/98 11:22a Allender
 * fix ingame join trouble.  Small messaging fix.  Enable collisions for
 * friendlies again
 * 
 * 83    5/01/98 12:24p Jim
 * don't process radar_plot_obj on the standalone server
 * 
 * 82    4/07/98 4:05p Lawrance
 * Only show hostile bombs on radar.
 * 
 * 81    3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 80    3/15/98 3:11p Lawrance
 * Always draw target radar blip bright.
 * 
 * 79    3/11/98 5:33p Lawrance
 * Support rendering and targeting of jump nodes
 * 
 * 78    3/03/98 8:12p Lawrance
 * Draw cargo as gray dots
 * 
 * 77    2/22/98 4:30p John
 * More string externalization classification
 * 
 * 76    2/22/98 2:48p John
 * More String Externalization Classification
 * 
 * 75    2/21/98 3:26p Lawrance
 * Improve how blips get drawn for ships immune to sensors.
 * 
 * 74    2/16/98 11:58p Lawrance
 * Add support for SF_HIDDEN_FROM_SENSORS flag.
 * 
 * 73    2/13/98 4:08p Lawrance
 * Use more accurate distance formula when plotting radar dots... fixes
 * "dead zone" black spot.
 * 
 * 72    2/12/98 4:58p Lawrance
 * Change to new flashing method.
 * 
 * 71    2/11/98 12:04a Lawrance
 * Only show bombs on radar, change code to use much less data.
 * 
 * 70    2/10/98 11:46a Lawrance
 * Ensure TEAM_TRAITOR views other TEAM_TRAITOR ships as hostile.
 * 
 *
 * $NoKeywords: $
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

// forward declarations
void draw_radar_blips(int desired_color, int is_dim, int distort=0);

void radar_init_std()
{
	int i,j;

	Radar_gauge.first_frame = bm_load_animation(Current_radar_global->Radar_fname[gr_screen.res], &Radar_gauge.num_frames);
	if ( Radar_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", Current_radar_global->Radar_fname[gr_screen.res]);
	}

	for (i=0; i<MAX_RADAR_LEVELS; i++ )	{
		for (j=0; j<MAX_RADAR_COLORS; j++ )	{
			gr_init_alphacolor( &Radar_colors[i][j], Radar_color_rgb[i][j].r, Radar_color_rgb[i][j].g, Radar_color_rgb[i][j].b, 255 );
		}
	}

	Blip_mutate_id	= 1;
}

// determine what color the object blip should be drawn as
int radar_blip_color_std(object *objp)
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

void radar_plot_object_std( object *objp )	
{
	vec3d	pos, tempv;
	float		dist, rscale, zdist, max_radar_dist;
	int		xpos, ypos, color=0;
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

	
	// JAS -- new way of getting the rotated point that doesn't require this to be
	// in a g3_start_frame/end_frame block.
	vm_vec_sub(&tempv,world_pos,&Player_obj->pos);
	vm_vec_rotate( &pos, &tempv, &Player_obj->orient );

	// Apply range filter
	dist = vm_vec_dist(world_pos, &Player_obj->pos);
	max_radar_dist = Radar_ranges[HUD_config.rp_dist];
	if ( dist > max_radar_dist ){
		return;
	}

	if ( dist < pos.xyz.z ) {
		rscale = 0.0f;
	} else {
		rscale = (float) acos( pos.xyz.z/dist ) / 3.14159f;		//2.0f;	 
	}

	zdist = fl_sqrt( (pos.xyz.x*pos.xyz.x)+(pos.xyz.y*pos.xyz.y) );

	float new_x_dist, clipped_x_dist;
	float new_y_dist, clipped_y_dist;

	if (zdist < 0.01f ) {
		new_x_dist = 0.0f;
		new_y_dist = 0.0f;
	}
	else {
		new_x_dist = (pos.xyz.x/zdist) * rscale * radx;
		new_y_dist = (pos.xyz.y/zdist) * rscale * rady;

		// force new_x_dist and new_y_dist to be inside the radar

		float hypotenuse;
		float max_radius;

		hypotenuse = (float)_hypot(new_x_dist, new_y_dist);
		max_radius = i2fl(Current_radar_global->Radar_radius[gr_screen.res][0] - 5);

		if (hypotenuse >= (max_radius) ) {
			clipped_x_dist = max_radius * (new_x_dist / hypotenuse);
			clipped_y_dist = max_radius * (new_y_dist / hypotenuse);
			new_x_dist = clipped_x_dist;
			new_y_dist = clipped_y_dist;
		}
	}

	xpos = fl2i( Current_radar_global->Radar_center[gr_screen.res][0] + new_x_dist );
	ypos = fl2i( Current_radar_global->Radar_center[gr_screen.res][1] - new_y_dist );

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
		Int3();
		return;
	}

	b = &Blips[N_blips];
	b->flags=0;

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

	b->x = xpos;
	b->y = ypos;

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
void radar_null_nblips_std()
{
	int i;

	N_blips=0;

	for (i=0; i<MAX_RADAR_COLORS; i++) {
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
	Radar_calc_dim_dist_timer = timestamp(0);

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

void radar_draw_circle_std( int x, int y, int rad )
{
	if ( rad == Current_radar_global->Radar_blip_radius_target[gr_screen.res] )	{
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

	radar_draw_circle( b->x+xdiff, b->y+ydiff, b->rad ); 
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

	radar_draw_circle( b->x+xdiff, b->y+ydiff, b->rad ); 
}

// Draw all the active radar blips
void draw_radar_blips(int rcol, int is_dim, int distort)
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
			radar_draw_circle( b->x, b->y, b->rad );
		}
	}
}

// Draw the radar blips
// input:	distorted	=>		0 (default) to draw normal, 1 to draw distorted 
void radar_draw_blips_sorted_std(int distort)
{
	// draw dim blips first
	draw_radar_blips(RCOL_JUMP_NODE, 1, distort);
	draw_radar_blips(RCOL_WARPING_SHIP, 1, distort);
	draw_radar_blips(RCOL_NAVBUOYS, 1, distort);
	draw_radar_blips(RCOL_FRIENDLY, 1, distort);
	draw_radar_blips(RCOL_UNKNOWN, 1, distort);
	draw_radar_blips(RCOL_BOMB, 1, distort);
	draw_radar_blips(RCOL_NEUTRAL, 1, distort);
	draw_radar_blips(RCOL_HOSTILE, 1, distort);
	draw_radar_blips(RCOL_TAGGED, 1, distort);

	// draw bright blips
	draw_radar_blips(RCOL_JUMP_NODE, 0, distort);
	draw_radar_blips(RCOL_WARPING_SHIP, 0, distort);
	draw_radar_blips(RCOL_NAVBUOYS, 0, distort);
	draw_radar_blips(RCOL_FRIENDLY, 0, distort);
	draw_radar_blips(RCOL_UNKNOWN, 0, distort);
	draw_radar_blips(RCOL_BOMB, 0, distort);
	draw_radar_blips(RCOL_NEUTRAL, 0, distort);
	draw_radar_blips(RCOL_HOSTILE, 0, distort);
	draw_radar_blips(RCOL_TAGGED, 0, distort);
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
	radar_blit_gauge();
	radar_draw_range();

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
	SPECMAP = -1;
	GLOWMAP = -1;

	gr_set_bitmap(Radar_gauge.first_frame+1);
	gr_aabitmap( Current_radar_global->Radar_coords[gr_screen.res][0], Current_radar_global->Radar_coords[gr_screen.res][1] );
} 

void radar_page_in_std()
{
	bm_page_in_aabitmap( Radar_gauge.first_frame, Radar_gauge.num_frames );
}
