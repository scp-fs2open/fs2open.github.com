/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hud.h"
#include "hud/hudparse.h"
#include "hud/hudartillery.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudtarget.h"
#include "hud/hudreticle.h"
#include "hud/hudbrackets.h"
#include "object/object.h"
#include "ship/ship.h"
#include "render/3dinternal.h"
#include "globalincs/linklist.h"
#include "weapon/weapon.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"	// for flFrametime
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "debris/debris.h"
#include "mission/missionmessage.h"
#include "hud/hudtargetbox.h"
#include "ship/subsysdamage.h"
#include "hud/hudshield.h"
#include "mission/missionhotkey.h"
#include "asteroid/asteroid.h"
#include "jumpnode/jumpnode.h"
#include "weapon/emp.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "ship/awacs.h"
#include "parse/parselo.h"
#include "cmdline/cmdline.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "graphics/font.h"
#include "network/multiutil.h"
#include "model/model.h"

// If any of these bits in the ship->flags are set, ignore this ship when targeting
int TARGET_SHIP_IGNORE_FLAGS = (SF_EXPLODED|SF_DEPART_WARP|SF_DYING|SF_ARRIVING_STAGE_1|SF_HIDDEN_FROM_SENSORS);

// Global values for the target bracket width and height, used for debugging
int Hud_target_w, Hud_target_h;

// offscreen triangle that point the the off-screen target
float Offscreen_tri_base[GR_NUM_RESOLUTIONS] = {
	6.0f,
	9.5f
};
float Offscreen_tri_height[GR_NUM_RESOLUTIONS] = {
	7.0f,
	11.0f
};
float Max_offscreen_tri_seperation[GR_NUM_RESOLUTIONS] = {
	10.0f,
	16.0f
};
float Max_front_seperation[GR_NUM_RESOLUTIONS] = {
	10.0f,
	16.0f
};

SCP_vector<target_display_info> target_display_list;

// The following variables are global to this file, and do not need to be persistent from frame-to-frame
// This means the variables are not player-specific
object* hostile_obj = NULL;

static int ballistic_hud_index = 0;	// Goober5000

extern object obj_used_list;		// dummy node in linked list of active objects
extern char *Cargo_names[];

// shader is used to shade the target box 
shader Training_msg_glass;

// the target triangle (that orbits the reticle) dimensions
float Target_triangle_base[GR_NUM_RESOLUTIONS] = {
	6.0f,
	9.5f
};
float Target_triangle_height[GR_NUM_RESOLUTIONS] = {
	7.0f,
	11.0f
};

// stuff for hotkey targeting lists
htarget_list htarget_items[MAX_HOTKEY_TARGET_ITEMS];
htarget_list htarget_free_list;

/*
// coordinates and widths used to render the HUD afterburner energy gauge
int current_hud->Aburn_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		171, 265, 60, 60
	},
	{ // GR_1024
		274, 424, 86, 96
	}
};

// coordinates and widths used to render the HUD weapons energy gauge
int current_hud->Wenergy_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		416, 265, 60, 60
	},
	{ // GR_1024
		666, 424, 86, 96
	}
};*/

#define MIN_DISTANCE_TO_CONSIDER_THREAT	1500	// min distance to show hostile warning triangle

//////////////////////////////////////////////////////////////////////////
// lists for target in reticle cycling
//////////////////////////////////////////////////////////////////////////
#define	RL_USED		(1<<0)
#define	RL_USE_DOT	(1<<1)	// use dot product result, not distance

typedef struct _reticle_list {
	_reticle_list	*next, *prev;
	object			*objp;
	float				dist, dot;
	int				flags;
} reticle_list;

#define			RESET_TARGET_IN_RETICLE	750
int				Reticle_save_timestamp;
reticle_list	Reticle_cur_list;
reticle_list	Reticle_save_list;
#define			MAX_RETICLE_TARGETS	50
reticle_list	Reticle_list[MAX_RETICLE_TARGETS];

//////////////////////////////////////////////////////////////////////////
// used for closest target cycling
//////////////////////////////////////////////////////////////////////////
#define	TL_RESET			1500
#define	TURRET_RESET	1000
static int Tl_hostile_reset_timestamp;
static int Tl_friendly_reset_timestamp;
static int Target_next_uninspected_object_timestamp;
static int Target_newest_ship_timestamp;
static int Target_next_turret_timestamp;

// animation frames for the hud targeting gauges
// frames:	0	=>		out of range lead
//				1	=>		in range lead
float Lead_indicator_half[NUM_HUD_RETICLE_STYLES][GR_NUM_RESOLUTIONS][2] =
{
	{
		{ // GR_640
			12.5f,		// half-width
			12.5f			// half-height
		},
		{ // GR_1024
			20.0f,		// half-width
			20.0f			// half-height
		}
	},
	{
		{ // GR_640
			8.0f,		// half-width
			8.0f			// half-height
		},
		{ // GR_1024
			13.0f,		// half-width
			13.0f			// half-height
		}
	}
};
hud_frames Lead_indicator_gauge;
int Lead_indicator_gauge_loaded = 0;
char Lead_fname[NUM_HUD_RETICLE_STYLES][GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] =
{
	{ "lead1_fs1", "2_lead1_fs1" },
	{ "lead1", "2_lead1" }
};

// animation frames for the countermeasures gauge
// frames:	0	=>		background
hud_frames Cmeasure_gauge;
int Cmeasure_gauge_loaded = 0;
int Cm_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		497, 343
	},
	{ // GR_1024
		880, 602
	}
};
int Cm_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		533, 347
	},
	{ // GR_1024
		916, 606
	}
};
int Cm_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		506, 347
	},
	{ // GR_1024
		889, 606
	}
};
char Cm_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"countermeasure1",
	"countermeasure1"
};

#define TOGGLE_TEXT_AUTOT		0
#define TOGGLE_TEXT_TARGET		1
#define TOGGLE_TEXT_AUTOS		2
#define TOGGLE_TEXT_SPEED		3

int Toggle_text_alpha = 255;


// animation files for the weapons gauge
#define NUM_WEAPON_GAUGES	5
hud_frames Weapon_gauges[NUM_HUD_SETTINGS][NUM_WEAPON_GAUGES];
hud_frames New_weapon;
int Weapon_gauges_loaded = 0;
// for primaries
int Weapon_gauge_primary_coords[NUM_HUD_SETTINGS][GR_NUM_RESOLUTIONS][3][2] =
{
	{ // normal HUD
		{ // GR_640
			// based on the # of primaries
			{509, 273},				// top of weapon gauge, first frame, always
			{497, 293},				// for the first primary
			{497, 305}				// for the second primary
		},
		{ // GR_1024
			// based on the # of primaries
			{892, 525},				// top of weapon gauge, first frame, always
			{880, 545},				// for the first primary
			{880, 557}				// for the second primary
		}
	},
	{ // ballistic HUD - slightly different alignment
		{ // GR_640
			// based on the # of primaries
			{485, 273},				// top of weapon gauge, first frame, always
			{485, 293},				// for the first primary
			{485, 305}				// for the second primary
		},
		{ // GR_1024
			// based on the # of primaries
			{868, 525},				// top of weapon gauge, first frame, always
			{868, 545},				// for the first primary
			{868, 557}				// for the second primary
		}
	}
};
int Weapon_gauge_secondary_coords[NUM_HUD_SETTINGS][GR_NUM_RESOLUTIONS][5][2] =
{
	{ // normal HUD
		{ // GR_640
			// based on the # of secondaries
			{497, 318},				// bottom of gauge, 0 secondaries
			{497, 318},				// bottom of gauge, 1 secondaries
			{497, 317},				// middle of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{497, 326},				// bottom of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{497, 335}				// bottom of gauge, 3 secondaries
		},
		{ // GR_1024
			// based on the # of secondaries
			{880, 570},				// bottom of gauge, 0 secondaries
			{880, 570},				// bottom of gauge, 1 secondaries
			{880, 569},				// middle of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{880, 578},				// bottom of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{880, 587}				// bottom of gauge, 3 secondaries
		}
	},
	{ // ballistic HUD - slightly different alignment
		{ // GR_640
			// based on the # of secondaries
			{485, 318},				// bottom of gauge, 0 secondaries
			{485, 318},				// bottom of gauge, 1 secondaries
			{485, 317},				// middle of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{485, 326},				// bottom of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{485, 335}				// bottom of gauge, 3 secondaries
		},
		{ // GR_1024
			// based on the # of secondaries
			{868, 570},				// bottom of gauge, 0 secondaries
			{868, 570},				// bottom of gauge, 1 secondaries
			{868, 569},				// middle of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{868, 578},				// bottom of gauge, 2 secondaries AND middle of gauge, 3 secondaries
			{868, 587}				// bottom of gauge, 3 secondaries
		}
	}
};
int Weapon_title_coords[NUM_HUD_SETTINGS][GR_NUM_RESOLUTIONS][2] =
{
	{ // normal HUD
		{ // GR_640
			518, 274
		},
		{ // GR_1024
			901, 527
		}
	},
	{ // ballistic HUD - slightly different alignment
		{ // GR_640
			487, 274
		},
		{ // GR_1024
			870, 527
		}
	}
};
int Weapon_plink_coords[GR_NUM_RESOLUTIONS][2][2] = {
	{ // GR_640
		{530, 285},				// fire-linked thingie, for the first primary
		{530, 295}				// fire-linked thingie, for the second primary
	},
	{ // GR_1024
		{913, 537},				// fire-linked thingie, for the first primary
		{913, 547}				// fire-linked thingie, for the second primary
	}
};
int Weapon_pname_coords[GR_NUM_RESOLUTIONS][2][2] = {
	{ // GR_640
		{536, 285},				// weapon name, first primary
		{536, 295}				// weapon name, second primary
	},
	{ // GR_1024
		{919, 537},				// weapon name, first primary
		{919, 547}				// weapon name, second primary
	}
};
int Weapon_slinked_x[GR_NUM_RESOLUTIONS] = {
	525,							// where to draw the second thingie if this weapon is fire-linked
	908
};
int Weapon_sunlinked_x[GR_NUM_RESOLUTIONS] = {
	530,							// where to draw the first thingie if this weapon is selected at all (fire-linked or not)
	913
};
int Weapon_secondary_y[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		309,						// y location of where to draw text for the first secondary
		318,						// y location of where to draw text for the second secondary
		327						// y location of where to draw text for the third secondary
	},
	{ // GR_1024
		561,						// y location of where to draw text for the third secondary
		570,						// y location of where to draw text for the third secondary
		579						// y location of where to draw text for the third secondary
	}
};
int Weapon_primary_y[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		285,						// y location of where to draw text for the first primary
		295							// y location of where to draw text for the second primary
	},
	{ // GR_1024
		537,						// y location of where to draw text for the first primary
		547							// y location of where to draw text for the second primary
	}
};
int Weapon_secondary_name_x[GR_NUM_RESOLUTIONS] = {
	536,							// x location of where to draw weapon name
	919
};
int Weapon_secondary_ammo_x[GR_NUM_RESOLUTIONS] = {
	525,							// x location of where to draw weapon ammo count
	908
};
int Weapon_primary_ammo_x[GR_NUM_RESOLUTIONS] = {
	525,							// x location of where to draw primary weapon ammo count
	908
};
int Weapon_secondary_reload_x[GR_NUM_RESOLUTIONS] = {
	615,							// x location of where to draw the weapon reload time
	998
};
char *Weapon_gauge_fnames[NUM_HUD_SETTINGS][GR_NUM_RESOLUTIONS][NUM_WEAPON_GAUGES] = 
{
//XSTR:OFF
	{ // normal HUD
		{ // GR_640
			"weapons1",
			"weapons2",
			"weapons3",
			"weapons4",
			"weapons5"
		}, 
		{ // GR_1024
			"weapons1",
			"weapons2",
			"weapons3",
			"weapons4",
			"weapons5"
		}
	},
	{ // ballistic HUD - slightly different alignment
		{ // GR_640
			"weapons1_b",
			"weapons2_b",
			"weapons3_b",
			"weapons4_b",
			"weapons5_b"
		}, 
		{ // GR_1024
			"weapons1_b",
			"weapons2_b",
			"weapons3_b",
			"weapons4_b",
			"weapons5_b"
		}
	}
//XSTR:ON
};

// Flash the line for a weapon.  This normally occurs when the player tries to fire that
// weapon, but the firing fails (due to lack of energy or damaged weapons subsystem).
#define MAX_WEAPON_FLASH_LINES 7		// 3 primary and 4 secondary
typedef struct weapon_flash
{
	int flash_duration[MAX_WEAPON_FLASH_LINES];
	int flash_next[MAX_WEAPON_FLASH_LINES];
	int is_bright;
} weapon_flash;
weapon_flash Weapon_flash_info;

// Data used for the proximity warning
typedef struct homing_beep_info
{
	int	snd_handle;				// sound handle for last played beep
	fix	last_time_played;		//	time beep was last played
	int	min_cycle_time;		// time (in ms) for fastest cycling of the sound
	int	max_cycle_time;		// time (in ms) for slowest cycling of the sound
	float min_cycle_dist;		// distance at which fastest cycling occurs 
	float max_cycle_dist;		// distance at which slowest cycling occurs
	float	precalced_interp;		// a precalculated value used in a linear interpretation
} homing_beep_info;

homing_beep_info Homing_beep = { -1, 0, 150, 1000, 30.0f, 1500.0f, 1.729412f };

// Set at the start of a mission, used to decide how to draw the separation for the warning missile indicators
float Min_warning_missile_dist;
float	Max_warning_missile_dist;

void hud_maybe_flash_weapon(int index);	

// if a given object should be ignored because of AWACS effects
int hud_target_invalid_awacs(object *objp)
{
	// if objp is ship object, first check if can be targeted with team info
	if (objp->type == OBJ_SHIP) {
		if (Player_ship != NULL) {
			if (ship_is_visible_by_team(objp, Player_ship)) {
				return 0;
			}
		}
	}

	// check for invalid status
	if((Player_ship != NULL) && (awacs_get_level(objp, Player_ship) < 1.0f)){
		return 1;
	}

	// valid
	return 0;
}

ship_subsys *advance_subsys(ship_subsys *cur, int next_flag)
{
	if (next_flag) {
		return GET_NEXT(cur);
	} else {
		return GET_LAST(cur);
	}
}

// select a sorted turret subsystem on a ship if no other subsys has been selected
void hud_maybe_set_sorted_turret_subsys(ship *shipp)
{
	Assert((Player_ai->target_objnum >= 0) && (Player_ai->target_objnum < MAX_OBJECTS));
	if (!((Player_ai->target_objnum >= 0) && (Player_ai->target_objnum < MAX_OBJECTS))) {
		return;
	}
	Assert(Objects[Player_ai->target_objnum].type == OBJ_SHIP);
	if (Objects[Player_ai->target_objnum].type != OBJ_SHIP) {
		return;
	}

	if (Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) {
		if (shipp->last_targeted_subobject[Player_num] == NULL) {
			hud_target_live_turret(1, 1);
		}
	}

}

// -----------------------------------------------------------------------
//	clear out the linked list of targets in the reticle
void hud_reticle_clear_list(reticle_list *rlist)
{
	reticle_list *cur;
	for ( cur = GET_FIRST(rlist); cur != END_OF_LIST(rlist); cur = GET_NEXT(cur) ) {
		cur->flags = 0;
	}
	list_init(rlist);
}

// --------------------------------------------------------------------------------------
//	hud_reticle_list_init()
void hud_reticle_list_init()
{
	int i;

	for ( i = 0; i < MAX_RETICLE_TARGETS; i++ ) {
		Reticle_list[i].flags = 0;
	}

	Reticle_save_timestamp = 1;
	list_init(&Reticle_save_list);
	list_init(&Reticle_cur_list);
}

// --------------------------------------------------------------------------------------
//	hud_check_reticle_list()
//
//
void	hud_check_reticle_list()
{
	reticle_list	*rl, *temp;

	// cull dying objects from reticle list
	rl = GET_FIRST(&Reticle_cur_list);
	while( rl !=END_OF_LIST(&Reticle_cur_list) )	{
		temp = GET_NEXT(rl);
		if ( rl->objp->flags & OF_SHOULD_BE_DEAD ) {
			list_remove( &Reticle_cur_list, rl );
			rl->flags = 0;
		}
		rl = temp;
	}

	if ( timestamp_elapsed(Reticle_save_timestamp) ) {
		hud_reticle_clear_list(&Reticle_save_list);
		Reticle_save_timestamp = timestamp(RESET_TARGET_IN_RETICLE);
	}
}

// --------------------------------------------------------------------------------------
//	hud_reticle_list_find_free()
//
//
int hud_reticle_list_find_free()
{
	int i;

	// find a free reticle_list element
	for ( i = 0; i < MAX_RETICLE_TARGETS; i++ ) {
		if ( !(Reticle_list[i].flags & RL_USED) ) {
			break;
		}
	}

	if ( i == MAX_RETICLE_TARGETS ) {
//		nprintf(("Warning","Warning ==> Ran out of reticle target elements...\n"));
		return -1;
	}

	return i;
}

// --------------------------------------------------------------------------------------
//	hud_stuff_reticle_list()
//
//
#define	RETICLE_DEFAULT_DIST		100000.0f
#define	RETICLE_DEFAULT_DOT		1.0f
void hud_stuff_reticle_list(reticle_list *rl, object *objp, float measure, int dot_flag)
{
	if ( dot_flag ) {
		rl->dot = measure;
		rl->dist = RETICLE_DEFAULT_DIST;
		rl->flags |= RL_USE_DOT;
	}
	else {
		rl->dist = measure;
		rl->dot = RETICLE_DEFAULT_DOT;		
	}
	rl->objp = objp;
}

// --------------------------------------------------------------------------------------
//	hud_reticle_list_update()
//
//	Update Reticle_cur_list with an object that lies in the reticle
//
//	parmeters:	objp		=>		object pointer to target
//					measure	=>		distance or dot product, depending on dot_flag
//					dot_flag	=>		if 0, measure is distance, if 1 measure is dot
//
void hud_reticle_list_update(object *objp, float measure, int dot_flag)
{
	reticle_list	*rl, *new_rl;
	int				i;
	SCP_list<CJumpNode>::iterator jnp;
	
	if (objp->type == OBJ_JUMP_NODE) {
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if( jnp->GetSCPObject() != objp )
				continue;
			
			if( jnp->IsHidden() )
				return;
		}
	}

	for ( rl = GET_FIRST(&Reticle_cur_list); rl != END_OF_LIST(&Reticle_cur_list); rl = GET_NEXT(rl) ) {
		if ( rl->objp == objp )
			return;
	}

	i = hud_reticle_list_find_free();
	if ( i == -1 )
		return;

	new_rl = &Reticle_list[i];
	new_rl->flags |= RL_USED;
	hud_stuff_reticle_list(new_rl, objp, measure, dot_flag);

	int was_inserted = 0;
	
	if ( EMPTY(&Reticle_cur_list) ) {
		list_insert(&Reticle_cur_list, new_rl);
		was_inserted = 1;
	}
	else {
		for ( rl = GET_FIRST(&Reticle_cur_list); rl != END_OF_LIST(&Reticle_cur_list); rl = GET_NEXT(rl) ) {
			if ( !dot_flag ) {	
				// compare based on distance
				if ( measure < rl->dist ) {
					list_insert_before(rl, new_rl);
					was_inserted = 1;
					break;
				}
			}			
			else {
				// compare based on dot
				if ( measure > rl->dot ) {
					list_insert_before(rl, new_rl);
					was_inserted = 1;
					break;
				}
			}		
		}	// end for
	}

	if ( !was_inserted ) {
		list_append(&Reticle_cur_list, new_rl);
	}
}

// --------------------------------------------------------------------------------------
//	hud_reticle_pick_target()
//
//	Pick a target from Reticle_cur_list, based on what is in Reticle_save_list
//
//
object *hud_reticle_pick_target()
{
	reticle_list	*cur_rl, *save_rl, *new_rl;
	object			*return_objp;
	int				in_save_list, i;

	return_objp = NULL;

	// As a first step, see if both ships and debris are in the list.  If so, cull the debris.
	int debris_in_list = 0;
	int ship_in_list = 0;
	for ( cur_rl = GET_FIRST(&Reticle_cur_list); cur_rl != END_OF_LIST(&Reticle_cur_list); cur_rl = GET_NEXT(cur_rl) ) {
		if ( (cur_rl->objp->type == OBJ_SHIP) || (cur_rl->objp->type == OBJ_JUMP_NODE) ) {
			ship_in_list = 1;
			continue;
		}

		if ( cur_rl->objp->type == OBJ_WEAPON ) {
			if ( Weapon_info[Weapons[cur_rl->objp->instance].weapon_info_index].subtype == WP_MISSILE ) {
				ship_in_list = 1;
				continue;
			}
		}
			
		if ( (cur_rl->objp->type == OBJ_DEBRIS) || (cur_rl->objp->type == OBJ_ASTEROID) ) {
			debris_in_list = 1;
			continue;
		}
	}

	if ( ship_in_list && debris_in_list ) {
		// cull debris
		reticle_list	*rl, *next;
		
		rl = GET_FIRST(&Reticle_cur_list);
		while ( rl != &Reticle_cur_list ) {
			next = rl->next;
			if ( (rl->objp->type == OBJ_DEBRIS) || (rl->objp->type == OBJ_ASTEROID) ){
				list_remove(&Reticle_cur_list,rl);
				rl->flags = 0;
			}
			rl = next;
		}
	}
	
	for ( cur_rl = GET_FIRST(&Reticle_cur_list); cur_rl != END_OF_LIST(&Reticle_cur_list); cur_rl = GET_NEXT(cur_rl) ) {
		in_save_list = 0;
		for ( save_rl = GET_FIRST(&Reticle_save_list); save_rl != END_OF_LIST(&Reticle_save_list); save_rl = GET_NEXT(save_rl) ) {
			if ( cur_rl->objp == save_rl->objp ) {
				in_save_list = 1;
				break;
			}
		}

		if ( !in_save_list ) {
			return_objp = cur_rl->objp;
			i = hud_reticle_list_find_free();
			if ( i == -1 )
				break;

			new_rl = &Reticle_list[i];
			new_rl->flags |= RL_USED;
			if ( cur_rl->flags & RL_USE_DOT ) {
				hud_stuff_reticle_list(new_rl, cur_rl->objp, cur_rl->dot, 1);
			}
			else {
				hud_stuff_reticle_list(new_rl, cur_rl->objp, cur_rl->dist, 0);
			}

			list_append(&Reticle_save_list, new_rl);
			break;
		}
	}	// end for

	if ( return_objp == NULL && !EMPTY(&Reticle_cur_list) ) {
			i = hud_reticle_list_find_free();
			if ( i == -1 ) 
				return NULL;
			new_rl = &Reticle_list[i];
			cur_rl = GET_FIRST(&Reticle_cur_list);
			*new_rl = *cur_rl;
			return_objp = cur_rl->objp;
			hud_reticle_clear_list(&Reticle_save_list);
			list_append(&Reticle_save_list, new_rl);
	}

	return return_objp;
}

// hud_target_hotkey_add_remove takes as its parameter which hotkey (1-0) to add/remove the current
// target from.  This function behaves like the Shift-<selection> does in Windows -- using shift # will toggle
// the current target in and out of the selection set.
void hud_target_hotkey_add_remove( int k, object *ctarget, int how_to_add )
{
	htarget_list *hitem, *plist;

	// don't do anything if a standalone multiplayer server
	if ( MULTIPLAYER_STANDALONE )
		return;

	if ( (k < 0) || (k >= MAX_KEYED_TARGETS) ) {
		nprintf(("Warning", "Bogus hotkey %d sent to hud_target_hotkey_add_remove\n", k));
		return;
	}

	plist = &(Players[Player_num].keyed_targets[k]);
	
	// we must operate only on ships
	if ( ctarget->type != OBJ_SHIP )
		return;

	// don't allow player into hotkey set
	if ( ctarget == Player_obj )
		return;

	// don't put dying or departing
	if ( Ships[ctarget->instance].flags & (SF_DYING|SF_DEPARTING) )
		return;

	// don't add mission file added hotkey assignments if there are player added assignments 
	// already in the list
	if ( (how_to_add == HOTKEY_MISSION_FILE_ADDED) && NOT_EMPTY(plist) ) {
		for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
			if ( hitem->how_added == HOTKEY_USER_ADDED )
				return;
		}
	}

	// determine if the current target is currently in the set or not
	for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
		if ( hitem->objp == ctarget )
			break;
	}

	// if hitem == end of the list, then the target should be added, else it should be removed
	if ( hitem == END_OF_LIST(plist) ) {
		if ( EMPTY(&htarget_free_list) ) {
			Int3();			// get Allender -- no more free hotkey target items
			return;
		}

		nprintf(("network", "Hotkey: Adding %s\n", Ships[ctarget->instance].ship_name));
		hitem = GET_FIRST( &htarget_free_list );
		list_remove( &htarget_free_list, hitem );
		list_append( plist, hitem );
		hitem->objp = ctarget;
		hitem->how_added = how_to_add;
	} else {
		nprintf(("network", "Hotkey: Removing %s\n", Ships[ctarget->instance].ship_name));
		list_remove( plist, hitem );
		list_append( &htarget_free_list, hitem );
		hitem->objp = NULL;									// for safety
	}
}

// the following function clears the hotkey set given by parameter passed in
void hud_target_hotkey_clear( int k )
{
	htarget_list *hitem, *plist, *temp;

	plist = &(Players[Player_num].keyed_targets[k]);
	hitem = GET_FIRST(plist);
	while ( hitem != END_OF_LIST(plist) ) {
		temp = GET_NEXT(hitem);
		list_remove( plist, hitem );
		list_append( &htarget_free_list, hitem );
		hitem->objp = NULL;
		hitem = temp;
	}
	if ( Players[Player_num].current_hotkey_set == k )		// clear this variable if we removed the bindings
		Players[Player_num].current_hotkey_set = -1;
}

// the next function sets the current selected set to be N.  If there is just one ship in the selection
// set, this ship will become the new target.  If there is more than one ship in the selection set,
// then the current_target will remain what it was.
void hud_target_hotkey_select( int k )
{
	int visible_count = 0;
	htarget_list *hitem, *plist, *target, *next_target, *first_target;
	int target_objnum;

	plist = &(Players[Player_num].keyed_targets[k]);
	
	if ( EMPTY( plist ) )			// no items in list, then do nothing
		return;

	// a simple walk of the list to get the count
	for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ){
		if (awacs_get_level(hitem->objp, Player_ship, 1) > 1) {
			visible_count++;
		}
	}

	// no visible ships in list
	if (visible_count == 0) {
		return;
	}

	// set the current target to be the "next" ship in the list.  Scan the list to see if our
	// current target is in the set.  If so, target the next ship in the list, otherwise target
	// the first
	// set	first_target - first visible item in list
	//			target - item in list that is the player's currently selected target
	//			next_target -	next visible item in list following target
	target_objnum = Player_ai->target_objnum;
	target = NULL;
	next_target = NULL;
	first_target = NULL;
	for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
		
		if (awacs_get_level(hitem->objp, Player_ship, 1) > 1) {
			// get the first valid target
			if (first_target == NULL) {
				first_target = hitem;
			}

			// get the next target in the list following the player currently selected target
			if (target != NULL) {
				next_target = hitem;
				break;
			}
		}

		// mark the player currently selected target
		if ( OBJ_INDEX(hitem->objp) == target_objnum ) {
			target = hitem;
		}
	}

	// if current target is not in list, then target and next_target will be NULL
	// so we use the first found target
	if (target == NULL) {
		Assert(first_target != NULL);
		if (first_target != NULL) {
			target = first_target;
			next_target = first_target;
		} else {
			// this should not happen
			return;
		}
	}
	
	// update target if more than 1 is visible
	if (visible_count > 1) {
		// next already found (after current target in list)
		if (next_target != NULL) {
			target = next_target;
		} else {

		// next is before current target, so search from start of list
			for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
				if (awacs_get_level(hitem->objp, Player_ship, 1) > 1) {
					target = hitem;
					break;
				}
			}
		}
	}

	Assert( target != END_OF_LIST(plist) );

	if ( Player_obj != target->objp ){
		set_target_objnum( Player_ai, OBJ_INDEX(target->objp) );
	}

	Players[Player_num].current_hotkey_set = k;
}

// hud_init_targeting_colors() will initialize the shader and gradient objects used
// on the HUD
//

color HUD_color_homing_indicator;

void hud_make_shader(shader *sh, ubyte r, ubyte g, ubyte b, float dimmer = 1000.0f)
{
	// The m matrix converts all colors to shades of green
	//float tmp = 16.0f*(0.0015625f * i2fl(HUD_color_alpha+1.0f));
	float tmp = 0.025f * i2fl(HUD_color_alpha+1.0f);

	ubyte R = ubyte(r * tmp);
	ubyte G = ubyte(r * tmp);
	ubyte B = ubyte(r * tmp);
	ubyte A = ubyte((float(r) / dimmer)*(i2fl(HUD_color_alpha) / 15.0f) * 255.0f);

	gr_create_shader( sh, R, G, B, A );
}

void hud_init_targeting_colors()
{
	gr_init_color( &HUD_color_homing_indicator, 0x7f, 0x7f, 0 );	// yellow

	hud_make_shader(&Training_msg_glass, 61, 61, 85, 500.0f);

	hud_init_brackets();
}

void hud_keyed_targets_clear()
{
	int i;

	// clear out the keyed target list
	for (i = 0; i < MAX_KEYED_TARGETS; i++ )
		list_init( &(Players[Player_num].keyed_targets[i]) );

	Players[Player_num].current_hotkey_set = -1;

	// place all of the hoykey target items back onto the free list
	list_init( &htarget_free_list );
	for ( i = 0; i < MAX_HOTKEY_TARGET_ITEMS; i++ )
		list_append( &htarget_free_list, &htarget_items[i] );
}

// Init data used for the weapons display on the HUD
void hud_weapons_init()
{
	Weapon_flash_info.is_bright = 0;
	for ( int i = 0; i < MAX_WEAPON_FLASH_LINES; i++ ) {
		Weapon_flash_info.flash_duration[i] = 1;
		Weapon_flash_info.flash_next[i] = 1;
	}

	// The E: There used to be a number of checks here. They are no longer needed, as the new HUD code handles it on its own.
	Weapon_gauges_loaded = 1;
}

// init data used to play the homing "proximity warning" sound
void hud_init_homing_beep()
{
	Homing_beep.snd_handle = -1;
	Homing_beep.last_time_played  = 0;
	Homing_beep.precalced_interp = (Homing_beep.max_cycle_dist-Homing_beep.min_cycle_dist) / (Homing_beep.max_cycle_time - Homing_beep.min_cycle_time );
}

// hud_init_targeting() will set the current target to point to the dummy node
// in the object used list
//
void hud_init_targeting()
{
	Assert(Player_ai != NULL);

	int i;

	// decide whether to realign HUD for ballistic primaries
	ballistic_hud_index = 0;
	for (i = 0; i <	Player_ship->weapons.num_primary_banks; i++)
	{
		if (Weapon_info[Player_ship->weapons.primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC)
		{
			ballistic_hud_index = 1;
			break;
		}
	}

	// make sure there is no current target
	set_target_objnum( Player_ai, -1 );
	Player_ai->last_target = -1;
	Player_ai->last_subsys_target = NULL;
	Player_ai->last_dist = 0.0f;
	Player_ai->last_speed = 0.0f;

	hud_keyed_targets_clear();
	hud_init_missile_lock();
	hud_init_artillery();
	
	// Init the lists that hold targets in reticle (to allow cycling of targets in reticle)
	hud_reticle_list_init();
	hud_init_homing_beep();

	hud_weapons_init();

	Min_warning_missile_dist = 2.5f*Player_obj->radius;
	Max_warning_missile_dist = 1500.0f;

	Tl_hostile_reset_timestamp = timestamp(0);
	Tl_friendly_reset_timestamp = timestamp(0);
	Target_next_uninspected_object_timestamp = timestamp(0);
	Target_newest_ship_timestamp = timestamp(0);
	Target_next_turret_timestamp = timestamp(0);

	if(The_mission.flags & MISSION_FLAG_FULLNEB) {
		Toggle_text_alpha = TOGGLE_TEXT_NEBULA_ALPHA; 
	} else {
		Toggle_text_alpha = TOGGLE_TEXT_NORMAL_ALPHA;
	}
}

//	Target the next or previous subobject on the currently selected ship, based on next_flag.
void hud_target_subobject_common(int next_flag)
{
	if (Player_ai->target_objnum == -1) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No target selected.", 322));
		snd_play( &Snds[SND_TARGET_FAIL] );
		return;
	}

	if (Objects[Player_ai->target_objnum].type != OBJ_SHIP) {
		snd_play( &Snds[SND_TARGET_FAIL]);
		return;
	}

	ship_subsys	*start, *start2, *A;
	ship_subsys	*subsys_to_target=NULL;
	ship			*target_shipp;

	target_shipp = &Ships[Objects[Player_ai->target_objnum].instance];

	if (!Player_ai->targeted_subsys) {
		start = GET_FIRST(&target_shipp->subsys_list);
	} else {
		start = Player_ai->targeted_subsys;
	}

	start2 = advance_subsys(start, next_flag);

	for ( A = start2; A != start; A = advance_subsys(A, next_flag) ) {

		if ( A == &target_shipp->subsys_list ) {
			continue;
		}

		// ignore turrets
		if ( A->system_info->type == SUBSYSTEM_TURRET ) {
			continue;
		}

		if ( A->flags & SSF_UNTARGETABLE ) {
			continue;
		}

		subsys_to_target = A;
		break;

	} // end for

	if ( subsys_to_target == NULL ) {
		snd_play( &Snds[SND_TARGET_FAIL]);
	} else {
		set_targeted_subsys(Player_ai, subsys_to_target, Player_ai->target_objnum);
		target_shipp->last_targeted_subobject[Player_num] =  Player_ai->targeted_subsys;
	}	
}

object *advance_fb(object *objp, int next_flag)
{
	if (next_flag)
		return GET_NEXT(objp);
	else
		return GET_LAST(objp);
}

//	Target the previous subobject on the currently selected ship.
//

void hud_target_prev_subobject()
{
	hud_target_subobject_common(0);
}

void hud_target_next_subobject()
{
	hud_target_subobject_common(1);
}

// hud_target_next() will set the Players[Player_num].current_target to the next target in the object
// used list whose team matches the team parameter.  The player is NOT included in the target list.
//
//	parameters:		team_mask => team(s) of ship to target next
//

void hud_target_common(int team_mask, int next_flag)
{
	object	*A, *start, *start2;
	ship		*shipp;
	int		is_ship, target_found = FALSE;
	SCP_list<CJumpNode>::iterator jnp;

	if (Player_ai->target_objnum == -1)
		start = &obj_used_list;
	else
		start = &Objects[Player_ai->target_objnum];

	start2 = advance_fb(start, next_flag);

	for ( A = start2; A != start; A = advance_fb(A, next_flag) ) {
		is_ship = 0;

		if (A == &obj_used_list)
			continue;

		if ( (A == Player_obj) || ((A->type != OBJ_SHIP) && (A->type != OBJ_WEAPON) && (A->type != OBJ_JUMP_NODE)) )
			continue;

		if ( hud_target_invalid_awacs(A) )
			continue;

		if (A->type == OBJ_WEAPON) {
			if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags2 & WIF2_CAN_BE_TARGETED) )
				if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags & WIF_BOMB) )
					continue;

			if (Weapons[A->instance].lssm_stage == 3)
				continue;
		}

		if (A->type == OBJ_SHIP) {
			if (Ships[A->instance].flags & TARGET_SHIP_IGNORE_FLAGS)
				continue;

			is_ship = 1;
		}

		if (A->type == OBJ_JUMP_NODE) {
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if( jnp->GetSCPObject() == A )
					break;
			}
			
			if( jnp->IsHidden() )
				continue;
		}

		if ( vm_vec_same(&A->pos, &Eye_position) )
			continue;

		if ( is_ship ) {
			shipp = &Ships[A->instance];	// get a pointer to the ship information

			if (!iff_matches_mask(shipp->team, team_mask)) {
				continue;
			}

			if ( A == Player_obj || (shipp->flags & TARGET_SHIP_IGNORE_FLAGS) ){
				continue;
			}

			// if we've reached here, it is a valid next target
			if ( Player_ai->target_objnum != A-Objects ) {
				target_found = TRUE;
				set_target_objnum( Player_ai, OBJ_INDEX(A) );
				// if ship is BIG|HUGE and last subsys is NULL, get turret
				hud_maybe_set_sorted_turret_subsys(shipp);
				hud_restore_subsystem_target(shipp);
			}
		} else {
			target_found = TRUE;
			set_target_objnum( Player_ai, OBJ_INDEX(A) );
		}

		break;
	}

	if ( target_found == FALSE ) {
		snd_play( &Snds[SND_TARGET_FAIL] );
	}
}

void hud_target_next(int team_mask)
{
	hud_target_common(team_mask, 1);
}

void hud_target_prev(int team_mask)
{
	hud_target_common(team_mask, 0);
}

// -------------------------------------------------------------------
// advance_missile_obj()
//
missile_obj *advance_missile_obj(missile_obj *mo, int next_flag)
{
	if (next_flag){
		return GET_NEXT(mo);
	} 
	
	return GET_LAST(mo);
}

ship_obj *advance_ship(ship_obj *so, int next_flag)
{
	if (next_flag){
		return GET_NEXT(so);
	} 
	
	return GET_LAST(so);
}

/// \brief Iterates down to and selects the next target in a linked list 
///        fashion ordered from closest to farthest from the 
///        attacked_object_number, returning the next valid target.
/// 
/// 
/// \param targeting_from_closest_to_farthest[in]   targets the closest object 
///                                     if true. Targets the farthest 
///                                     away object if false.
/// \param valid_team_mask[in]          A bit mask that defines the desired 
///                                     victim team.
/// \param attacked_object_number[in]   The objectid that is under attack. 
///                                     Defaults to -1.
/// \param target_filters               Applies a bit filter to exclude certain
///                                     classes of objects from being targeted.
///                                     Defaults to (SIF_CARGO | SIF_NAVBUOY)
///                                        
/// \returns         The next object to target if targeting was successful. 
///                  Returns NULL if targeting was unsuccessful.
static object* select_next_target_by_distance( const bool targeting_from_closest_to_farthest, const int valid_team_mask, const int attacked_object_number = -1, const int target_filters = (SIF_CARGO | SIF_NAVBUOY)) {
	object *minimum_object_ptr, *maximum_object_ptr, *nearest_object_ptr;
	minimum_object_ptr = maximum_object_ptr = nearest_object_ptr = NULL;
	float current_distance = hud_find_target_distance(&Objects[Player_ai->target_objnum], Player_obj);
	float minimum_distance = 1e20f;
	float maximum_distance = 0.0f;
	int player_object_index = OBJ_INDEX(Player_obj);
	
	float nearest_distance;
	if ( targeting_from_closest_to_farthest ) {
		nearest_distance = 1e20f;
	} else {
		nearest_distance = 0.0f;
	}
	
	ship_obj *ship_object_ptr;
	object   *prospective_victim_ptr;
	ship     *prospective_victim_ship_ptr;
	for ( ship_object_ptr = GET_FIRST(&Ship_obj_list);   ship_object_ptr != END_OF_LIST(&Ship_obj_list);   ship_object_ptr = GET_NEXT(  ship_object_ptr) ) {
		prospective_victim_ptr = &Objects[  ship_object_ptr->objnum];
		  // get a pointer to the ship information
		prospective_victim_ship_ptr = &Ships[prospective_victim_ptr->instance];
	
		float new_distance;
			if ( (prospective_victim_ptr == Player_obj) || (prospective_victim_ship_ptr->flags & TARGET_SHIP_IGNORE_FLAGS) ) {
				continue;
			}
	
			// choose from the correct team
			if ( !iff_matches_mask(prospective_victim_ship_ptr->team, valid_team_mask) ) {
				continue;
			}
				
			// don't use object if it is already a target
			if ( OBJ_INDEX(prospective_victim_ptr) == Player_ai->target_objnum ) {
				continue;
			}
				
	
		if( attacked_object_number == -1 ) {
			// always ignore navbuoys and cargo
			if ( Ship_info[prospective_victim_ship_ptr->ship_info_index].flags & target_filters ) {
				continue;
			}
				
			if(hud_target_invalid_awacs(prospective_victim_ptr)){
				continue;
			}
	
			new_distance = hud_find_target_distance(prospective_victim_ptr,Player_obj);
		} else {
				// Filter out any target that is not targeting the player  --Mastadon
			if ( (attacked_object_number == player_object_index) && (Ai_info[prospective_victim_ship_ptr->ai_index].target_objnum != player_object_index) ) {
				continue;
			}
			esct eval_ship_as_closest_target_args;
			eval_ship_as_closest_target_args.attacked_objnum = attacked_object_number;
			eval_ship_as_closest_target_args.check_all_turrets = (attacked_object_number == player_object_index);
			eval_ship_as_closest_target_args.check_nearest_turret = FALSE;
				// We don't ever filter our target selection to just bombers or fighters
				// because the select next attacker logic doesn't.  --Mastadon
			eval_ship_as_closest_target_args.filter = 0;
			eval_ship_as_closest_target_args.team_mask = valid_team_mask;
				// We always get the turret attacking, since that's how the select next 
				// attacker logic does it.  --Mastadon
			eval_ship_as_closest_target_args.turret_attacking_target = 1;
			eval_ship_as_closest_target_args.shipp = prospective_victim_ship_ptr;
			evaluate_ship_as_closest_target( &eval_ship_as_closest_target_args );
	
			new_distance = eval_ship_as_closest_target_args.min_distance;
		}
	
	
		if (new_distance <= minimum_distance) {
			minimum_distance = new_distance;
			minimum_object_ptr = prospective_victim_ptr;
		}
	
		if (new_distance >= maximum_distance) {
			maximum_distance = new_distance;
			maximum_object_ptr = prospective_victim_ptr;
		}
			
		float diff = 0.0f;
		if ( targeting_from_closest_to_farthest ) {
			diff = new_distance - current_distance;
			if ( diff > 0.0f ) {
				if ( diff < ( nearest_distance - current_distance ) ) {
					nearest_distance = new_distance;
					nearest_object_ptr = prospective_victim_ptr;
				}
			}
		} else {
			diff = current_distance - new_distance;
			if ( diff > 0.0f ) {
				if ( diff < ( current_distance - nearest_distance ) ) {
					nearest_distance = new_distance;
					nearest_object_ptr = const_cast<object *>(prospective_victim_ptr);
				}
			}
		}
	}

	if ( nearest_object_ptr == NULL ) {

		if ( targeting_from_closest_to_farthest ) {
			if ( minimum_object_ptr != NULL ) {
				nearest_object_ptr = minimum_object_ptr;
			}
		} else {
			if ( maximum_object_ptr != NULL ) {
				nearest_object_ptr = maximum_object_ptr;
			}
		}
	}

	return nearest_object_ptr;
}

ship_obj *get_ship_obj_ptr_from_index(int index);
// -------------------------------------------------------------------
// hud_target_missile()
//
// Target the closest locked missile that is locked on locked_obj
//
//	input:	source_obj	=>		pointer to object that fired weapon
//				next_flag	=>		0 -> previous 1 -> next
//									
// NOTE: this function is only allows targeting bombs
void hud_target_missile(object *source_obj, int next_flag)
{
	missile_obj	*end, *start, *mo;
	object		*A, *target_objp;
	ai_info		*aip;
	weapon		*wp;
	weapon_info	*wip;
	int			target_found = 0;	

	if ( source_obj->type != OBJ_SHIP )
		return;

	Assert( Ships[source_obj->instance].ai_index != -1 );
	aip = &Ai_info[Ships[source_obj->instance].ai_index];
	
	end = &Missile_obj_list;
	if (aip->target_objnum != -1) {
		target_objp = &Objects[aip->target_objnum];
		if ( target_objp->type == OBJ_WEAPON && Weapon_info[Weapons[target_objp->instance].weapon_info_index].subtype == WP_MISSILE )	{	// must be a missile
			end = missile_obj_return_address(Weapons[target_objp->instance].missile_list_index);
		}
	}

	start = advance_missile_obj(end, next_flag);

	for ( mo = start; mo != end; mo = advance_missile_obj(mo, next_flag) ) {
		if ( mo == &Missile_obj_list ){
			continue;
		}

		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		A = &Objects[mo->objnum];

		Assert(A->type == OBJ_WEAPON);
		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));
		wp = &Weapons[A->instance];
		wip = &Weapon_info[wp->weapon_info_index];

		// only allow targeting of bombs
		if ( !(wip->wi_flags2 & WIF2_CAN_BE_TARGETED) ) {
			if ( !(wip->wi_flags & WIF_BOMB) ) {
				continue;
			}
		}

		if (wp->lssm_stage==3){
			continue;
		}

		// only allow targeting of hostile bombs
		if (!iff_x_attacks_y(Player_ship->team, obj_team(A))) {
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		// if we've reached here, got a new target
		target_found = TRUE;
		set_target_objnum( aip, OBJ_INDEX(A) );
		break;
	}	// end for

	if ( !target_found ) {
	// if no bomb is found, search for bombers
		ship_obj *startShip, *so;

		if ( (aip->target_objnum != -1)
			&& (Objects[aip->target_objnum].type == OBJ_SHIP)
			&& ((Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index].flags & SIF_BOMBER)
				|| (Objects[aip->target_objnum].flags & OF_TARGETABLE_AS_BOMB))) {
			int index = Ships[Objects[aip->target_objnum].instance].ship_list_index;
			startShip = get_ship_obj_ptr_from_index(index);
		} else {
			startShip = GET_FIRST(&Ship_obj_list);
		}

		for (so=advance_ship(startShip, next_flag); so!=startShip; so=advance_ship(so, next_flag)) {
			A = &Objects[so->objnum];

			// don't look at header
			if (so == &Ship_obj_list) {
				continue;
			}

			// only allow targeting of hostile bombs
			if (!iff_x_attacks_y(Player_ship->team, obj_team(A))) {
				continue;
			}

			if(hud_target_invalid_awacs(A)){
				continue;
			}

			// check if ship type is bomber
			if ( !(Ship_info[Ships[A->instance].ship_info_index].flags & SIF_BOMBER) && !(A->flags & OF_TARGETABLE_AS_BOMB) ) {
				continue;
			}

			// check if ignore
			if ( Ships[A->instance].flags & TARGET_SHIP_IGNORE_FLAGS ){
				continue;
			}

			// found a good one
			target_found = TRUE;
			set_target_objnum( aip, OBJ_INDEX(A) );
			break;
		}
	}

	if ( !target_found ) {
		snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
	}		
}

// Return !0 if shipp can be scanned, otherwise return 0
int hud_target_ship_can_be_scanned(ship *shipp)
{
	ship_info *sip;

	sip = &Ship_info[shipp->ship_info_index];

	// ignore cargo that has already been scanned
	if (shipp->flags & SF_CARGO_REVEALED)
		return 0;

	// allow ships with scannable flag set
	if (shipp->flags & SF_SCANNABLE)
		return 1;

	// ignore ships that don't carry cargo
	if ((sip->class_type < 0) || !(Ship_types[sip->class_type].ship_bools & STI_SHIP_SCANNABLE))
		return 0;

	return 1;
}

// target the next/prev uninspected cargo container
void hud_target_uninspected_cargo(int next_flag)
{
	object	*A, *start, *start2;
	ship		*shipp;
	int		target_found = 0;	

	if (Player_ai->target_objnum == -1) {
		start = &obj_used_list;
	}  else {
		start = &Objects[Player_ai->target_objnum];
	}

	start2 = advance_fb(start, next_flag);

	for ( A = start2; A != start; A = advance_fb(A, next_flag) ) {
		if ( A == &obj_used_list ) {
			continue;
		}

		if (A == Player_obj || (A->type != OBJ_SHIP) ) {
			continue;
		}

		shipp = &Ships[A->instance];	// get a pointer to the ship information

		if ( shipp->flags & TARGET_SHIP_IGNORE_FLAGS ) {
			continue;
		}

		// ignore all non-cargo carrying craft
		if ( !hud_target_ship_can_be_scanned(shipp) ) {
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		// if we've reached here, it is a valid next target
		if ( Player_ai->target_objnum != OBJ_INDEX(A) ) {
			target_found = TRUE;
			set_target_objnum( Player_ai, OBJ_INDEX(A) );
		}
	}

	if ( target_found == FALSE ) {
		snd_play( &Snds[SND_TARGET_FAIL]);
	}
}

// target the newest ship in the area
void hud_target_newest_ship()
{
	object	*A, *player_target_objp;
	object	*newest_obj=NULL;
	ship		*shipp;
	ship_obj	*so;
	uint		current_target_arrived_time = 0xffffffff, newest_time = 0;

	if ( Player_ai->target_objnum >= 0 ) {
		player_target_objp = &Objects[Player_ai->target_objnum];
		if ( player_target_objp->type == OBJ_SHIP ) {
			current_target_arrived_time = Ships[player_target_objp->instance].create_time;
		}
	} else {
		player_target_objp = NULL;
	}

	// If no target is selected, then simply target the closest uninspected cargo
	if ( Player_ai->target_objnum == -1 || timestamp_elapsed(Target_newest_ship_timestamp) ) {
		current_target_arrived_time = 0xffffffff;
	}

	Target_newest_ship_timestamp = timestamp(TL_RESET);

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		A = &Objects[so->objnum];
		shipp = &Ships[A->instance];	// get a pointer to the ship information

		if ( (A == Player_obj) || (shipp->flags & TARGET_SHIP_IGNORE_FLAGS) )
			continue;

		// ignore navbuoys
		if ( Ship_info[shipp->ship_info_index].flags & SIF_NAVBUOY ) {
			continue;
		}

		if ( A == player_target_objp ) {
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		if ( (shipp->create_time >= newest_time) && (shipp->create_time <= current_target_arrived_time) ) {
			newest_time = shipp->create_time;
			newest_obj = A;
		}
	}

	if (newest_obj) {
		set_target_objnum( Player_ai, OBJ_INDEX(newest_obj) );
		// if BIG|HUGE and no selected subsystem, get sorted turret
		hud_maybe_set_sorted_turret_subsys(&Ships[newest_obj->instance]);
		hud_restore_subsystem_target(&Ships[newest_obj->instance]);
	}
	else {
		snd_play( &Snds[SND_TARGET_FAIL]);
	}
}

#define TYPE_NONE						0
#define TYPE_FACING_BEAM			1
#define TYPE_FACING_FLAK			2
#define TYPE_FACING_MISSILE		3
#define TYPE_FACING_LASER			4
#define TYPE_NONFACING_BEAM		5
#define TYPE_NONFACING_FLAK		6
#define TYPE_NONFACING_MISSILE	7
#define TYPE_NONFACING_LASER		8
#define TYPE_NONFACING_INC			4

typedef struct eval_next_turret {
	ship_subsys *ss;
	int type;
	float dist;
} eval_next_turret;

int turret_compare_func(const void *e1, const void *e2)
{
	eval_next_turret *p1 = (eval_next_turret*)e1;
	eval_next_turret *p2 = (eval_next_turret*)e2;

	Assert(p1->type != TYPE_NONE);
	Assert(p2->type != TYPE_NONE);

	if (p1->type != p2->type) {
		return (p1->type - p2->type);
	} else {
		float delta_dist = p1->dist - p2->dist;
		if (delta_dist < 0) {
			return -1;
		} else if (delta_dist > 0) {
			return 1;
		} else {
			return 0;
		}
	}
}

extern bool turret_weapon_has_flags(ship_weapon *swp, int flags);
extern bool turret_weapon_has_subtype(ship_weapon *swp, int subtype);
// target the next/prev live turret on the current target
// auto_advance from hud_update_closest_turret
void hud_target_live_turret(int next_flag, int auto_advance, int only_player_target)
{
	ship_subsys	*A;
	ship_subsys	*live_turret=NULL;
	ship			*target_shipp;
	object		*objp;
	eval_next_turret ent[MAX_MODEL_SUBSYSTEMS];
	int num_live_turrets = 0;

	// make sure we're targeting a ship
	if (Player_ai->target_objnum == -1 && !auto_advance) {
		snd_play(&Snds[SND_TARGET_FAIL]);
		return;
	}
	
	// only targeting subsystems on ship
	if ((Objects[Player_ai->target_objnum].type != OBJ_SHIP) && (!auto_advance)) {
		snd_play( &Snds[SND_TARGET_FAIL]);
		return;
	}

	// set some pointers
	objp = &Objects[Player_ai->target_objnum];
	target_shipp = &Ships[objp->instance];

	// set timestamp
	int timestamp_val = 0;
	if (!auto_advance) {
		timestamp_val = Target_next_turret_timestamp;
		Target_next_turret_timestamp = timestamp(TURRET_RESET);
	}

	// If no target is selected, then simply target the closest (or facing) turret
	int last_subsys_turret = FALSE;
	if (Player_ai->targeted_subsys != NULL) {
		if (Player_ai->targeted_subsys->system_info->type == SUBSYSTEM_TURRET) {
			if (Player_ai->targeted_subsys->weapons.num_primary_banks > 0 || Player_ai->targeted_subsys->weapons.num_secondary_banks > 0) {
				last_subsys_turret = TRUE;
			}
		}
	}

	// do we want the closest turret (or the one our ship is pointing at)
	int get_closest_turret = (auto_advance || !last_subsys_turret || timestamp_elapsed(timestamp_val));

	// initialize eval struct
	memset(ent,0, sizeof(ent));
	int use_straight_ahead_turret = FALSE;

	// go through list of turrets
	for (A=GET_FIRST(&target_shipp->subsys_list); A!=END_OF_LIST(&target_shipp->subsys_list); A=GET_NEXT(A))  {
		// get a turret
		if (A->system_info->type == SUBSYSTEM_TURRET) {
			// niffiwan: ignore untargetable turrets 
			if ( A->flags & SSF_UNTARGETABLE ) {
				continue;
			}
			// check turret has hit points and has a weapon
			if ( (A->current_hits > 0) && (A->weapons.num_primary_banks > 0 || A->weapons.num_secondary_banks > 0) ) {
				if ( !only_player_target || (A->turret_enemy_objnum == OBJ_INDEX(Player_obj)) ) {
					vec3d gsubpos, vec_to_subsys;
					float distance, dot;
					// get world pos of subsystem and its distance
					get_subsystem_world_pos(objp, A, &gsubpos);
					distance = vm_vec_normalized_dir(&vec_to_subsys, &gsubpos, &View_position);

					// check if facing and in view
					int facing = ship_subsystem_in_sight(objp, A, &View_position, &gsubpos, 0);
					
					if (!auto_advance && get_closest_turret && !only_player_target) {
						// if within 3 degrees and not previous subsys, use subsys in front
						dot = vm_vec_dotprod(&vec_to_subsys, &Player_obj->orient.vec.fvec);
						if ((dot > 0.9986) && facing) {
							use_straight_ahead_turret = TRUE;
							break;
						}
					}

					// set weapon_type to allow sort of ent on type
					if (turret_weapon_has_flags(&A->weapons, WIF_BEAM)) {
						ent[num_live_turrets].type = TYPE_FACING_BEAM;
					} else  if (turret_weapon_has_flags(&A->weapons, WIF_FLAK)) {
						ent[num_live_turrets].type = TYPE_FACING_FLAK;
					} else {
						if (turret_weapon_has_subtype(&A->weapons, WP_MISSILE)) {
							ent[num_live_turrets].type = TYPE_FACING_MISSILE;
						} else if (turret_weapon_has_subtype(&A->weapons, WP_LASER)) {
							ent[num_live_turrets].type = TYPE_FACING_LASER;
						} else {
							//Turret not live, bail
							continue;
						}
					}

					// fill out ent struct
					ent[num_live_turrets].ss = A;
					ent[num_live_turrets].dist = distance;
					if (!facing) {
						ent[num_live_turrets].type += TYPE_NONFACING_INC;
					}
					num_live_turrets++;
				}
			}
		}
	}

	// sort the list if we're not using turret straight ahead of us
	if (!use_straight_ahead_turret) {
		insertion_sort(ent, num_live_turrets, sizeof(eval_next_turret), turret_compare_func);
	}

	if (use_straight_ahead_turret) {
	// use the straight ahead turret
		live_turret = A;
	} else {
	// check if we have a currently targeted turret and find its position after the sort
		int i, start_index, next_index;
		if (get_closest_turret) {
			start_index = 0;
		} else {
			start_index = -1;
			for (i=0; i<num_live_turrets; i++) {
				if (ent[i].ss == Player_ai->targeted_subsys) {
					start_index = i;
					break;
				}
			}
			// check that we started with a turret
			if (start_index == -1) {
				start_index = 0;
			}
		}

		// set next live turret
		if (num_live_turrets == 0) {
			// no live turrets
			live_turret = NULL;
		} else if (num_live_turrets == 1 || get_closest_turret) {
			// only 1 live turret, so set it
			live_turret = ent[0].ss;
		} else {
			if (next_flag) {
				// advance to next closest turret
				next_index = start_index + 1;
				if (next_index == num_live_turrets) {
					next_index = 0;
				}
			} else {
				// go to next farther turret
				next_index = start_index - 1;
				if (next_index == -1) {
					next_index = num_live_turrets - 1;
				}
			}

			// set the next turret to be targeted based on next_index
			live_turret = ent[next_index].ss;
		}

		//if (live_turret) {
			// debug info
		//	mprintf(("name %s, index: %d, type: %d\n", live_turret->system_info->subobj_name, next_index, ent[next_index].type));
		//}
	}

	if ( live_turret != NULL ) {
		set_targeted_subsys(Player_ai, live_turret, Player_ai->target_objnum);
		target_shipp->last_targeted_subobject[Player_num] = Player_ai->targeted_subsys;
	} else {
		if (!auto_advance) {
			snd_play( &Snds[SND_TARGET_FAIL]);
		}
	}
}


// -------------------------------------------------------------------
// hud_target_closest_locked_missile()
//
// Target the closest locked missile that is locked on locked_obj
//
//	input:	locked_obj	=>		pointer to object that you want to find 
//										closest missile to
//									
void hud_target_closest_locked_missile(object *locked_obj)
{
	object		*A, *nearest_obj=NULL;
	weapon		*wp;
	weapon_info	*wip;
	float			nearest_dist, dist;
	int			target_found = FALSE;
	missile_obj	*mo;

	nearest_dist = 10000.0f;

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		A = &Objects[mo->objnum];

		if (A->type != OBJ_WEAPON){
			continue;
		}

		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));
		wp = &Weapons[A->instance];
		wip = &Weapon_info[wp->weapon_info_index];

		if ( wip->subtype != WP_MISSILE ){
			continue;
		}

		if ( !(wip->wi_flags & WIF_HOMING ) ){
			continue;
		}

		if (wp->lssm_stage==3){
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}
		

		if (wp->homing_object == locked_obj) {
			dist = vm_vec_dist_quick(&A->pos, &locked_obj->pos);		// Find distance!

			if (dist < nearest_dist) {
				nearest_obj = A;
				nearest_dist = dist;
			}
		}
	}	// end for 

	if (nearest_dist < 10000.0f) {
		Assert(nearest_obj);
		set_target_objnum( Player_ai, OBJ_INDEX(nearest_obj) );
		target_found = TRUE;
	}

	if ( !target_found ){
		snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
	}
}

// select a new target, by auto-targeting
void hud_target_auto_target_next()
{
	if ( Framecount < 2 ) {
		return;
	}

	//	No auto-targeting after dead.
	if (Game_mode & (GM_DEAD | GM_DEAD_BLEW_UP))
		return;

	// try target next ship in hotkey set, if any -- Backslash
	if ( Player->current_hotkey_set != -1 ) {
		hud_target_hotkey_select(Player->current_hotkey_set);
	}

	int valid_team_mask = iff_get_attackee_mask(Player_ship->team);

	// if none, try targeting closest hostile fighter/bomber
	if ( Player_ai->target_objnum == -1 ) { //-V581
		hud_target_closest(valid_team_mask, -1, FALSE, TRUE);
	}

	// No fighter/bombers exists, so go ahead an target the closest hostile
	if ( Player_ai->target_objnum == -1 ) { //-V581
		hud_target_closest(valid_team_mask, -1, FALSE);
	}

	// um, ok.  Try targeting asteroids that are on a collision course for an escort ship
	if ( Player_ai->target_objnum == -1 ) { //-V581
		asteroid_target_closest_danger();
	}
}


// Given that object 'targeter' is targeting object 'targetee', 
// how far are they?   This uses the point closest to the targeter
// object on the targetee's bounding box.  So if targeter is inside
// targtee's bounding box, the distance is 0.
float hud_find_target_distance( object *targetee, object *targeter )
{
	vec3d tmp_pnt;

	int model_num = -1;
	
	// Which model is it?
	switch( targetee->type )	{
	case OBJ_SHIP:
		model_num = Ship_info[Ships[targetee->instance].ship_info_index].model_num;
		break;
	case OBJ_DEBRIS:
//		model_num = Debris[targetee->instance].model_num;
		break;
	case OBJ_WEAPON:
		// Don't find model_num since circles would work better
		//model_num = Weapon_info[Weapons[targetee->instance].weapon_info_index].model_num;
		break;
	case OBJ_ASTEROID:
		// Don't find model_num since circles would work better
		//model_num = Asteroid_info[Asteroids[targetee->instance].type].model_num;
		break;
	case OBJ_JUMP_NODE:
		// Don't find model_num since circles would work better
		//model_num = Jump_nodes[targetee->instance].modelnum;
		break;
	}

	float dist = 0.0f;

	// New way, that uses bounding box.	
	if ( model_num > -1 )	{
		dist = model_find_closest_point( &tmp_pnt, model_num, -1, &targetee->orient, &targetee->pos, &targeter->pos );
	}  else {
		// Old way, that uses radius.
		dist = vm_vec_dist_quick(&targetee->pos, &targeter->pos) - targetee->radius;
		if ( dist < 0.0f )	{
			dist = 0.0f;
		}
	}
	return dist;
}

//

/// \brief evaluate a ship (and maybe turrets) as a potential target
/// 
/// Check if shipp (or its turrets) is attacking attacked_objnum
/// Provides a special case for player trying to select target (don't check if 
/// turrets are aimed at player)
/// 
/// \param[in, out] *esct The Evaluate Ship as Closest Target (esct) that will 
///                       be used to determine if a target is a valid, harmful 
///                       target and, if so, sets the min_distance attribute 
///                       to the distance of either the attacker or the 
///                       closest attacker's turret. Otherwise, min_distance 
///                       is set to FLT_MAX
/// 
/// \return true if either the ship or one of it's turrets are attacking the 
///                       player. Otherwise, returns false.
bool evaluate_ship_as_closest_target(esct *esct)
{
	int targeting_player, turret_is_attacking;
	ship_subsys *ss;
	float new_distance;

	// initialize
	esct->min_distance = FLT_MAX;
	esct->check_nearest_turret = FALSE;
	turret_is_attacking = FALSE;


	object *objp = &Objects[esct->shipp->objnum];
	Assert(objp->type == OBJ_SHIP);
	if (objp->type != OBJ_SHIP) {
		return false;
	}

	// player being targeted, so we will want closest distance from player
	targeting_player = (esct->attacked_objnum == OBJ_INDEX(Player_obj));

	// filter on team
	if ( !iff_matches_mask(esct->shipp->team, esct->team_mask) ) {
		return false;
	}

	// check if player or ignore ship
	if ( (esct->shipp->objnum == OBJ_INDEX(Player_obj)) || (esct->shipp->flags & TARGET_SHIP_IGNORE_FLAGS) ) {
		return false;
	}

	// bail if harmless
	if ( Ship_info[esct->shipp->ship_info_index].class_type > -1 && !(Ship_types[Ship_info[esct->shipp->ship_info_index].class_type].hud_bools & STI_HUD_TARGET_AS_THREAT)) {
		return false;
	}

	// only look at targets that are AWACS valid
	if (hud_target_invalid_awacs(&Objects[esct->shipp->objnum])) {
		return false;
	}

	// If filter is set, only target fighters and bombers
	if ( esct->filter ) {
		if ( !(Ship_info[esct->shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER)) ) {
			return false;
		}
	}

	// find closest turret to player if BIG or HUGE ship
	if (Ship_info[esct->shipp->ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) {
		for (ss=GET_FIRST(&esct->shipp->subsys_list); ss!=END_OF_LIST(&esct->shipp->subsys_list); ss=GET_NEXT(ss)) {

			if (ss->flags & SSF_UNTARGETABLE)
				continue;

			if ( (ss->system_info->type == SUBSYSTEM_TURRET) && (ss->current_hits > 0) ) {

				if (esct->check_all_turrets || (ss->turret_enemy_objnum == esct->attacked_objnum)) {
					turret_is_attacking = 1;
					esct->check_nearest_turret = TRUE;

					if ( !esct->turret_attacking_target || (esct->turret_attacking_target && (ss->turret_enemy_objnum == esct->attacked_objnum)) ) {
						vec3d gsubpos;
						// get world pos of subsystem
						vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &objp->orient);
						vm_vec_add2(&gsubpos, &objp->pos);
						new_distance = vm_vec_dist_quick(&gsubpos, &Player_obj->pos);
						
						/*
						// GET TURRET TYPE, FAVOR BEAM, FLAK, OTHER
						int turret_type = ss->system_info->turret_weapon_type;
						if (Weapon_info[turret_type].wi_flags & WIF_BEAM) {
							new_distance *= 0.3f;
						} else if (Weapon_info[turret_type].wi_flags & WIF_FLAK) {
							new_distance *= 0.6f;
						} */

						// get the closest distance
						if (new_distance <= esct->min_distance) {
							esct->min_distance = new_distance;
						}
					}
				}
			}
		}
	}

	// If no turret is attacking, check if objp is actually targeting attacked_objnum
	// don't bail if targeting is for player
	if ( !targeting_player && !turret_is_attacking ) {
		ai_info *aip = &Ai_info[esct->shipp->ai_index];

		if (aip->target_objnum != esct->attacked_objnum) {
			return false;
		}

		if ( (Game_mode & GM_NORMAL) && ( aip->mode != AIM_CHASE ) && (aip->mode != AIM_STRAFE) && (aip->mode != AIM_EVADE) && (aip->mode != AIM_EVADE_WEAPON) && (aip->mode != AIM_AVOID)) {
			return false;
		}
	}

	// consider the ship alone if there are no attacking turrets
	if ( !turret_is_attacking ) {
		//new_distance = hud_find_target_distance(objp, Player_obj);
		new_distance = vm_vec_dist_quick(&objp->pos, &Player_obj->pos);
			
		if (new_distance <= esct->min_distance) {
			esct->min_distance = new_distance;
			esct->check_nearest_turret = FALSE;
		}
	}

	return true;
}

/// \brief Sets the Players[Player_num].current_target to the closest ship to 
///        the player that matches the team passed as a paramater.
///
/// The current algorithm is to simply iterate through the objects and 
/// calculate the magnitude of the vector that connects the player to the 
/// target. The smallest magnitude is tracked, and then used to locate the 
/// closest hostile ship. Note only the square of the magnitude is required, 
/// since we are only comparing magnitudes.
///
/// \param[in] team_mask       team of closest ship that should be targeted.
///                            Default value is -1, if team doesn't matter.
///
/// \param[in] attacked_objnum object number of ship that is being attacked
/// \param[in] play_fail_snd   boolean, whether to play SND_TARGET_FAIL
///                            (needed, since function called repeatedly when 
///                            auto-targeting is enabled, and we don't want a 
///                            string of fail sounds playing). This is a 
///                            default parameter with a value of TRUE.
/// \param[in] filter          OPTIONAL parameter (default value 0): when set 
///                            to TRUE, only fighters and bombers are 
///                            considered for new targets.
/// \param[in] get_closest_turret_attacking_player Finds the closest turret 
///                            attacking the player if true. Otherwise, only 
///                            finds the closest attacking ship, targeting the 
///                            turret closest to the player.
///
/// \return: true (non-zero) if a target was acquired. Returns false (zero) if 
///          no target was acquired.
int hud_target_closest(int team_mask, int attacked_objnum, int play_fail_snd, int filter, int get_closest_turret_attacking_player)
{
	object	*A;
	object	*nearest_obj = &obj_used_list;
	ship		*shipp;
	ship_obj	*so;
	int		check_nearest_turret = FALSE;

	// evaluate ship closest target struct
	esct		esct;

	float		min_distance = FLT_MAX;
	int		target_found = FALSE;	

	int		player_obj_index = OBJ_INDEX(Player_obj);
	ship_subsys *ss;

	int initial_attacked_objnum = attacked_objnum;

	if ( (attacked_objnum >= 0) && (attacked_objnum != player_obj_index) ) {
		// bail if player does not have target
		if ( Player_ai->target_objnum == -1 ) {
			goto Target_closest_done;
		}

		if ( Objects[attacked_objnum].type != OBJ_SHIP ) {
			goto Target_closest_done;
		}

		// bail if ship is to be ignored
		if (Ships[Objects[attacked_objnum].instance].flags & TARGET_SHIP_IGNORE_FLAGS) {
			goto Target_closest_done;
		}
	}

	if (attacked_objnum == -1) {
		attacked_objnum = player_obj_index;
	}

	// check all turrets if for player.
	esct.check_all_turrets = (attacked_objnum == player_obj_index);
	esct.filter = filter;
	esct.team_mask = team_mask;
	esct.attacked_objnum = attacked_objnum;
	esct.turret_attacking_target = get_closest_turret_attacking_player;

	for ( so=GET_FIRST(&Ship_obj_list); so!=END_OF_LIST(&Ship_obj_list); so=GET_NEXT(so) ) {

		A = &Objects[so->objnum];
		shipp = &Ships[A->instance];	// get a pointer to the ship information

		// fill in rest of esct
		esct.shipp = shipp;

		// Filter out any target that is not targeting the player  --Mastadon
		if ( (initial_attacked_objnum == player_obj_index) && (Ai_info[shipp->ai_index].target_objnum != player_obj_index) ) {
			continue;
		}
		// check each shipp on list and update nearest obj and subsys
		evaluate_ship_as_closest_target(&esct);
		if (esct.min_distance < min_distance) {
			target_found = TRUE;
			min_distance = esct.min_distance;
			nearest_obj = A;
			check_nearest_turret = esct.check_nearest_turret;
		}
	}

	Target_closest_done:

	// maybe ignore target if too far away
	// DKA 9/8/99 Remove distance check
	/*
	if (target_found) {
		// get distance to nearest attacker
		float dist = vm_vec_dist_quick(&Objects[attacked_objnum].pos, &nearest_obj->pos);

		// no distance limit for player obj
		if ((attacked_objnum != player_obj_index) && (dist > MIN_DISTANCE_TO_CONSIDER_THREAT)) {
			target_found = FALSE;
		}
	} */

	if (target_found) {
		set_target_objnum(Player_ai, OBJ_INDEX(nearest_obj));
		if ( check_nearest_turret ) {

			// if former subobject was not a turret do, not change subsystem
			ss = Ships[nearest_obj->instance].last_targeted_subobject[Player_num];
			if (ss == NULL || get_closest_turret_attacking_player) {
				// update nearest turret with later func
				hud_target_live_turret(1, 1, get_closest_turret_attacking_player);
				Ships[nearest_obj->instance].last_targeted_subobject[Player_num] = Player_ai->targeted_subsys;
			}
		} else {
			hud_restore_subsystem_target(&Ships[nearest_obj->instance]);
		}
	} else {
		// no target found, maybe play fail sound
		if (play_fail_snd == TRUE) {
			snd_play(&Snds[SND_TARGET_FAIL]);
		}
	}

	return target_found;
}

// auto update closest turret to attack on big or huge ships
void hud_update_closest_turret()
{
	hud_target_live_turret(1, 1);

/*
	float nearest_distance, new_distance;
	ship_subsys	*ss, *closest_subsys;
	ship	*shipp;
	object *objp;

	nearest_distance = FLT_MAX;
	objp = &Objects[Player_ai->target_objnum];
	shipp = &Ships[objp->instance];
	closest_subsys = NULL;


	Assert(Ship_info[shipp->ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP));

	for (ss=GET_FIRST(&shipp->subsys_list); ss!=END_OF_LIST(&shipp->subsys_list); ss=GET_NEXT(ss)) {
		if ( (ss->system_info->type == SUBSYSTEM_TURRET) && (ss->current_hits > 0) ) {
			// make sure turret is not "unused"
			if (ss->system_info->turret_weapon_type >= 0) {
				vec3d gsubpos;
				// get world pos of subsystem
				vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &objp->orient);
				vm_vec_add2(&gsubpos, &objp->pos);
				new_distance = vm_vec_dist_quick(&gsubpos, &Player_obj->pos);

				// GET TURRET TYPE, FAVOR BEAM, FLAK, OTHER
				int turret_type = ss->system_info->turret_weapon_type;
				if (Weapon_info[turret_type].wi_flags & WIF_BEAM) {
					new_distance *= 0.3f;
				} else if (Weapon_info[turret_type].wi_flags & WIF_FLAK) {
					new_distance *= 0.6f;
				}

				// check if facing and in view
				int facing = ship_subsystem_in_sight(objp, ss, &View_position, &gsubpos, 0);

				if (facing) {
					new_distance *= 0.5f;
				}
				
				// get the closest distance
				if (new_distance <= nearest_distance) {
					nearest_distance = new_distance;
					closest_subsys = ss;
				}
			}
		}
	}

	// check if new subsys to target
	if (Player_ai->targeted_subsys != NULL) {
		set_targeted_subsys(Player_ai, closest_subsys, Player_ai->target_objnum);			
		shipp->last_targeted_subobject[Player_num] = Player_ai->targeted_subsys;
	}	
	*/
}


// --------------------------------------------------------------------
// hud_target_targets_target()
//
// Target your target's target.  Your target is specified by objnum passed
// as a parameter.
//
void hud_target_targets_target()
{
	object *objp = NULL;
	object *tt_objp = NULL;
	int		tt_objnum;

	if ( Player_ai->target_objnum < 0 || Player_ai->target_objnum >= MAX_OBJECTS ) {
		goto ttt_fail;
	}

	objp = &Objects[Player_ai->target_objnum];
	if ( objp->type != OBJ_SHIP ) {
		goto ttt_fail;
	}

	tt_objnum = Ai_info[Ships[objp->instance].ai_index].target_objnum;
	if ( tt_objnum < 0 || tt_objnum >= MAX_OBJECTS ) {
		goto ttt_fail;
	}

	if ( tt_objnum == OBJ_INDEX(Player_obj) ) {
		goto ttt_fail;
	}

	tt_objp = &Objects[tt_objnum]; 

	if (hud_target_invalid_awacs(tt_objp)) {
		goto ttt_fail;
	}

	if ( tt_objp->type != OBJ_SHIP ) {
		goto ttt_fail;
	}

	if ( Ships[tt_objp->instance].flags & TARGET_SHIP_IGNORE_FLAGS ) {
		goto ttt_fail;
	}

	// if we've reached here, found player target's target
	set_target_objnum( Player_ai, tt_objnum );
	if (Objects[tt_objnum].type == OBJ_SHIP) {
		hud_maybe_set_sorted_turret_subsys(&Ships[Objects[tt_objnum].instance]);
	}
	hud_restore_subsystem_target(&Ships[Objects[tt_objnum].instance]);
	return;

	ttt_fail:
	snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
}

// Return !0 if target_objp is a valid object type for targeting in reticle, otherwise return 0
int object_targetable_in_reticle(object *target_objp)
{
	int obj_type;
	SCP_list<CJumpNode>::iterator jnp;
	
	if (target_objp == Player_obj ) {
		return 0;
	}

	obj_type = target_objp->type;

	if ( (obj_type == OBJ_SHIP) || (obj_type == OBJ_DEBRIS) || (obj_type == OBJ_WEAPON) || (obj_type == OBJ_ASTEROID) )
	{
		return 1;
	} else if ( obj_type == OBJ_JUMP_NODE )
	{
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if(jnp->GetSCPObject() == target_objp)
				break;
		}
		
		if (!jnp->IsHidden())
			return 1;
	}

	return 0;
}

// hud_target_in_reticle_new() will target the object that is closest to the player, and who is 
// intersected by a ray passed from the center of the reticle out along the forward vector of the 
// player.
//
// targeting of objects of type OBJ_SHIP and OBJ_DEBRIS are supported
//
// Method: A ray is cast from the center of the reticle, and we keep track of any eligible object
//         the ray intersects.  We take the ship closest to us that intersects an object.
//
//         Since this method may work poorly with objects that are far away, hud_target_in_reticle_old()
//         is called if no intersections are found.
//
//
#define TARGET_IN_RETICLE_DISTANCE	10000.0f

void hud_target_in_reticle_new()
{
	vec3d	terminus;
	object	*A;
	mc_info	mc;
	float		dist;
	SCP_list<CJumpNode>::iterator jnp;

	hud_reticle_clear_list(&Reticle_cur_list);
	Reticle_save_timestamp = timestamp(RESET_TARGET_IN_RETICLE);

	//	Get 3d vector through center of reticle
	vm_vec_scale_add(&terminus, &Eye_position, &Player_obj->orient.vec.fvec, TARGET_IN_RETICLE_DISTANCE);

	mc.model_instance_num = -1;
	mc.model_num = 0;
	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {
		if ( !object_targetable_in_reticle(A) ) {
			continue;
		}

		if ( A->type == OBJ_WEAPON ) {
			if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags2 & WIF2_CAN_BE_TARGETED) ) {
				if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags & WIF_BOMB) ){
					continue;
				}
			}
			if (Weapons[A->instance].lssm_stage==3){
				continue;
			}
		}


		if ( A->type == OBJ_SHIP ) {
			if ( Ships[A->instance].flags & TARGET_SHIP_IGNORE_FLAGS ){
				continue;
			}
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		switch (A->type) {
		case OBJ_SHIP:
			mc.model_num = Ship_info[Ships[A->instance].ship_info_index].model_num;
			break;
		case OBJ_DEBRIS:
			mc.model_num = Debris[A->instance].model_num;
			break;
		case OBJ_WEAPON:
			mc.model_num = Weapon_info[Weapons[A->instance].weapon_info_index].model_num;
			break;
		case OBJ_ASTEROID:
			{
				int pof = 0;
				pof = Asteroids[A->instance].asteroid_subtype;
				mc.model_num = Asteroid_info[Asteroids[A->instance].asteroid_type].model_num[pof];
			}
			break;
		case OBJ_JUMP_NODE:
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == A)
					break;
			}	
			
			mc.model_num = jnp->GetModelNumber();
			break;
		default:
			Int3();	//	Illegal object type.
		}

		if (mc.model_num == -1) {
			// so just check distance of a point
			vec3d temp_v;
			float angle;
			vm_vec_sub(&temp_v, &A->pos, &Eye_position);
			vm_vec_normalize(&temp_v);
			angle = vm_vec_dot(&Player_obj->orient.vec.fvec, &temp_v);
			if (angle > 0.99f) {
				dist = vm_vec_mag_squared(&temp_v);
				hud_reticle_list_update(A, dist, 0);				
			}

		} else {

			model_clear_instance( mc.model_num );
			mc.orient = &A->orient;										// The object's orient
			mc.pos = &A->pos;												// The object's position
			mc.p0 = &Eye_position;										// Point 1 of ray to check
			mc.p1 = &terminus;											// Point 2 of ray to check
			mc.flags = MC_CHECK_MODEL;	// | MC_ONLY_BOUND_BOX;		// check the model, but only its bounding box

			model_collide(&mc);
			if ( mc.num_hits ) {
				dist = vm_vec_dist_squared(&mc.hit_point_world, &Eye_position);
				hud_reticle_list_update(A, dist, 0);
			}
		}
	}	// end for (go to next object)

	hud_target_in_reticle_old();	// try the old method (works well with ships far away)
}

// hud_target_in_reticle_old() will target the object that is closest to the reticle center and inside 
// the reticle 
//
// targeting of objects of type OBJ_SHIP and OBJ_DEBRIS are supported
//
// 
// Method:  take the dot product of the foward vector and the vector to target.  Take 
//          the one that is closest to 1 and at least MIN_DOT_FOR_TARGET
//
//	IMPORTANT:  The MIN_DOT_FOR_TARGET value was arrived at by trial and error and
//             is only valid for the HUD reticle in use at that time.

#define MIN_DOT_FOR_TARGET		0.9726// fov for targeting in reticle

void hud_target_in_reticle_old()
{
	object	*A, *target_obj;
	float	dot;
	vec3d	vec_to_target;

	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {
		if ( !object_targetable_in_reticle(A) ) {
			continue;
		}

		if ( A->type == OBJ_WEAPON ) {
			if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags2 & WIF2_CAN_BE_TARGETED) ){
				if ( !(Weapon_info[Weapons[A->instance].weapon_info_index].wi_flags & WIF_BOMB) ){
					continue;
				}
			}

			if (Weapons[A->instance].lssm_stage==3){
				continue;
			}
		}

		if ( A->type == OBJ_SHIP ) {
			if ( Ships[A->instance].flags & TARGET_SHIP_IGNORE_FLAGS ){
				continue;
			}
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		if ( vm_vec_same( &A->pos, &Eye_position ) ) {
			continue;
		}

		vm_vec_normalized_dir(&vec_to_target, &A->pos, &Eye_position);
		dot = vm_vec_dot(&Player_obj->orient.vec.fvec, &vec_to_target);

		if ( dot > MIN_DOT_FOR_TARGET ) {
			hud_reticle_list_update(A, dot, 1);
		}
	}

	target_obj = hud_reticle_pick_target();
	if ( target_obj != NULL ) {
		set_target_objnum( Player_ai, OBJ_INDEX(target_obj) );
		if ( target_obj->type == OBJ_SHIP ) {
			// if BIG|HUGE, maybe set subsys to turret
			hud_maybe_set_sorted_turret_subsys(&Ships[target_obj->instance]);
			hud_restore_subsystem_target(&Ships[target_obj->instance]);
		}
	}	
	else {
			snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
	}
}

// hud_target_subsystem_in_reticle() will target the subsystem that is within the reticle and 
// is closest to the reticle center.  The current target is the only object that is searched for
// subsystems
// 
// Method:  take the dot product of the foward vector and the vector to target.  Take 
//          the one that is closest to 1 and at least MIN_DOT_FOR_TARGET
//
//	IMPORTANT:  The MIN_DOT_FOR_TARGET value was arrived at by trial and error and
//             is only valid for the HUD reticle in use at that time.
//

void hud_target_subsystem_in_reticle()
{
	object* targetp;
	ship_subsys	*subsys;
	ship_subsys *nearest_subsys = NULL;
	vec3d subobj_pos;

	float dot, best_dot;
	vec3d vec_to_target;
	best_dot = -1.0f;

	if ( Player_ai->target_objnum == -1){
		hud_target_in_reticle_old();
	}

	if ( Player_ai->target_objnum == -1) { //-V581
		snd_play( &Snds[SND_TARGET_FAIL]);
		return;
	}
	
	targetp = &Objects[Player_ai->target_objnum];

	if ( targetp->type != OBJ_SHIP ){		// only targeting subsystems on ship
		return;
	}

	int shipnum = targetp->instance;

	if ( targetp->type == OBJ_SHIP ) {
		if ( Ships[shipnum].flags & TARGET_SHIP_IGNORE_FLAGS ) {
			return;
		}
	}

	for (subsys = GET_FIRST(&Ships[shipnum].subsys_list); subsys != END_OF_LIST(&Ships[shipnum].subsys_list)  ; subsys = GET_NEXT( subsys ) ) {
		
		//if the subsystem isn't targetable, skip it
		if (subsys->flags & SSF_UNTARGETABLE)
			continue;

		get_subsystem_world_pos(targetp, subsys, &subobj_pos);

		vm_vec_normalized_dir(&vec_to_target, &subobj_pos, &Eye_position);
		dot = vm_vec_dot(&Player_obj->orient.vec.fvec, &vec_to_target);

		if ( dot > best_dot ) {
			best_dot = dot;
			if ( best_dot > MIN_DOT_FOR_TARGET )
				nearest_subsys = subsys;
		}

		Assert(best_dot <= 1.0f);
	} // end for

	if ( nearest_subsys != NULL ) {
		set_targeted_subsys(Player_ai, nearest_subsys, Player_ai->target_objnum);
		char r_name[NAME_LENGTH];
		int i;
		strcpy_s(r_name, ship_subsys_get_name(Player_ai->targeted_subsys));
		for (i = 0; r_name[i] > 0; i++) {
			if (r_name[i] == '|')
				r_name[i] = ' ';
		}
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Targeting subsystem %s.", 323), r_name);
		Ships[shipnum].last_targeted_subobject[Player_num] =  Player_ai->targeted_subsys;
	}	
	else {
		snd_play( &Snds[SND_TARGET_FAIL]);
	}
}

#define T_LENGTH					8
#define T_OFFSET_FROM_CIRCLE	-13
#define T_BASE_LENGTH			4

//	On entry:
//		color set
void HudGaugeOrientationTee::renderOrientation(object *from_objp, object *to_objp, matrix *from_orientp)
{
	float		dot_product;
	vec3d	target_to_obj;
	float		x1,y1,x2,y2,x3,y3,x4,y4;

	vm_vec_sub(&target_to_obj, &from_objp->pos, &to_objp->pos);

	vm_vec_normalize(&target_to_obj);

	// calculate the dot product between the target_to_player vector and the targets forward vector
	//
	// 0 - vectors are perpendicular
	// 1 - vectors are collinear and in the same direction (target is facing player)
	// -1 - vectors are collinear and in the opposite direction (target is facing away from player)
	dot_product = vm_vec_dotprod(&from_orientp->vec.fvec, &target_to_obj);

	if (vm_vec_dotprod(&from_orientp->vec.rvec, &target_to_obj) >= 0) {
		if (dot_product >= 0){
			dot_product = -PI_2*dot_product + PI;
		} else {
			dot_product = -PI_2*dot_product - PI;
		}
	} else {
		dot_product *= PI_2; //(range is now -PI/2 => PI/2)
	}

	y1 = (float)sin(dot_product) * (Radius - T_OFFSET_FROM_CIRCLE);
	x1 = (float)cos(dot_product) * (Radius - T_OFFSET_FROM_CIRCLE);

	y1 += position[1];
	x1 += position[0];

	x1 += HUD_offset_x;
	y1 += HUD_offset_y;

	y2 = (float)sin(dot_product) * (Radius - T_OFFSET_FROM_CIRCLE - T_LENGTH);
	x2 = (float)cos(dot_product) * (Radius - T_OFFSET_FROM_CIRCLE - T_LENGTH);

	y2 += position[1];
	x2 += position[0];

	x2 += HUD_offset_x;
	y2 += HUD_offset_y;

	x3 = x1 - T_BASE_LENGTH * (float)sin(dot_product);
	y3 = y1 + T_BASE_LENGTH * (float)cos(dot_product);
	x4 = x1 + T_BASE_LENGTH * (float)sin(dot_product);
	y4 = y1 - T_BASE_LENGTH * (float)cos(dot_product);

	// HACK! Should be antialiased!
	renderLine(fl2i(x3),fl2i(y3),fl2i(x4),fl2i(y4));	// bottom of T
	renderLine(fl2i(x1),fl2i(y1),fl2i(x2),fl2i(y2));	// part of T pointing towards center
}

void hud_tri(float x1,float y1,float x2,float y2,float x3,float y3)
{
	int i;

	// Make the triangle always be the correct handiness so
	// the tmapper won't think its back-facing and throw it out.
	float det = (y2-y1)*(x3-x1) - (x2-x1)*(y3-y1);
	if ( det >= 0.0f )	{
		float tmp;

		// swap y1 & y3
		tmp = y1;
		y1 = y3;
		y3 = tmp;

		// swap x1 & x3
		tmp = x1;
		x1 = x3;
		x3 = tmp;
	}

	vertex * vertlist[3];
	vertex verts[3];

	// zero verts[] out, this is a faster way (nods to Kazan) to make sure that
	// the specular colors are set to 0 to avoid rendering problems - taylor
	memset(verts, 0, sizeof(vertex)*3);

	for (i=0; i<3; i++ )	
		vertlist[i] = &verts[i];

	verts[0].screen.xyw.x = x1;
	verts[0].screen.xyw.y = y1;
	verts[0].screen.xyw.w = 0.0f;
	verts[0].texture_position.u = 0.0f;
	verts[0].texture_position.v = 0.0f;
	verts[0].flags = PF_PROJECTED;
	verts[0].codes = 0;
	verts[0].r = (ubyte)gr_screen.current_color.red;
	verts[0].g = (ubyte)gr_screen.current_color.green;
	verts[0].b = (ubyte)gr_screen.current_color.blue;
	verts[0].a = (ubyte)gr_screen.current_color.alpha;

	verts[1].screen.xyw.x = x2;	
	verts[1].screen.xyw.y = y2;
	verts[1].screen.xyw.w = 0.0f;
	verts[1].texture_position.u = 0.0f;
	verts[1].texture_position.v = 0.0f;
	verts[1].flags = PF_PROJECTED;
	verts[1].codes = 0;
	verts[1].r = (ubyte)gr_screen.current_color.red;
	verts[1].g = (ubyte)gr_screen.current_color.green;
	verts[1].b = (ubyte)gr_screen.current_color.blue;
	verts[1].a = (ubyte)gr_screen.current_color.alpha;

	verts[2].screen.xyw.x = x3;
	verts[2].screen.xyw.y = y3;
	verts[2].screen.xyw.w = 0.0f;
	verts[2].texture_position.u = 0.0f;
	verts[2].texture_position.v = 0.0f;
	verts[2].flags = PF_PROJECTED;
	verts[2].codes = 0;
	verts[2].r = (ubyte)gr_screen.current_color.red;
	verts[2].g = (ubyte)gr_screen.current_color.green;
	verts[2].b = (ubyte)gr_screen.current_color.blue;
	verts[2].a = (ubyte)gr_screen.current_color.alpha;

	for (i=0; i<3; i++)
		gr_resize_screen_posf(&verts[i].screen.xyw.x, &verts[i].screen.xyw.y);

	uint saved_mode = gr_zbuffer_get();
	int cull = gr_set_cull(0);
	
	gr_zbuffer_set( GR_ZBUFF_NONE );
	
	//gr_tmapper( 3, vertlist, TMAP_FLAG_TRILIST );
	g3_draw_poly_constant_sw(3, vertlist, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA, 0.1f);	

	gr_zbuffer_set( saved_mode );
	gr_set_cull(cull);
}


void hud_tri_empty(float x1,float y1,float x2,float y2,float x3,float y3)
{
	gr_line(fl2i(x1),fl2i(y1),fl2i(x2),fl2i(y2));
	gr_line(fl2i(x2),fl2i(y2),fl2i(x3),fl2i(y3));
	gr_line(fl2i(x3),fl2i(y3),fl2i(x1),fl2i(y1));
}

HudGaugeReticleTriangle::HudGaugeReticleTriangle():
HudGauge(HUD_OBJECT_HOSTILE_TRI, HUD_HOSTILE_TRIANGLE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{

}

HudGaugeReticleTriangle::HudGaugeReticleTriangle(int _gauge_object, int _gauge_config):
HudGauge(_gauge_object, _gauge_config, true, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{

}

void HudGaugeReticleTriangle::initRadius(int length)
{
	Radius = length;
}

void HudGaugeReticleTriangle::initTriBase(float length)
{
	Target_triangle_base = length;
}

void HudGaugeReticleTriangle::initTriHeight(float h)
{
	Target_triangle_height = h;
}

void HudGaugeReticleTriangle::render(float frametime)
{
}

// Render a missile warning triangle that has a tail on it to indicate distance
void HudGaugeReticleTriangle::renderTriangleMissileTail(float ang, float xpos, float ypos, float cur_dist, int draw_solid, int draw_inside)
{
	float x1=0.0f;
	float x2=0.0f;
	float y1=0.0f;
	float y2=0.0f;
	float xtail=0.0f;
	float ytail=0.0f;

	float sin_ang, cos_ang, tail_len;

	float max_tail_len=20.0f;

	sin_ang=(float)sin(ang);
	cos_ang=(float)cos(ang);

	if ( cur_dist < Min_warning_missile_dist ) {
		tail_len = 0.0f;
	} else if ( cur_dist > Max_warning_missile_dist ) {
		tail_len = max_tail_len;
	} else {
		tail_len = cur_dist/Max_warning_missile_dist * max_tail_len;
	}

	if ( draw_inside ) {				
		x1 = xpos - Target_triangle_base * -sin_ang;
		y1 = ypos + Target_triangle_base * cos_ang;
		x2 = xpos + Target_triangle_base * -sin_ang;
		y2 = ypos - Target_triangle_base * cos_ang;

		xpos -= Target_triangle_height * cos_ang;
		ypos += Target_triangle_height * sin_ang;

		if ( tail_len > 0 ) {
			xtail = xpos - tail_len * cos_ang;
			ytail = ypos + tail_len * sin_ang;
		}

	} else {				
		x1 = xpos - Target_triangle_base * -sin_ang;
		y1 = ypos + Target_triangle_base * cos_ang;
		x2 = xpos + Target_triangle_base * -sin_ang;
		y2 = ypos - Target_triangle_base * cos_ang;

		xpos += Target_triangle_height * cos_ang;
		ypos -= Target_triangle_height * sin_ang;

		if ( tail_len > 0 ) {
			xtail = xpos + tail_len * cos_ang;
			ytail = ypos - tail_len * sin_ang;
		}
	}

	gr_set_screen_scale(base_w, base_h);
	if (draw_solid) {
		hud_tri(xpos,ypos,x1,y1,x2,y2);
	} else {
		hud_tri_empty(xpos,ypos,x1,y1,x2,y2);
	}

	// draw the tail indicating length
	if ( tail_len > 0 ) {
		gr_line(fl2i(xpos), fl2i(ypos), fl2i(xtail), fl2i(ytail));
	}
	gr_reset_screen_scale();
}

//	Render a triangle on the outside of the targeting circle.
//	Must be inside a g3_start_frame().
//	If aspect_flag !0, then render filled, indicating aspect lock.
// If show_interior !0, then point inwards to positions inside reticle
void HudGaugeReticleTriangle::renderTriangle(vec3d *hostile_pos, int aspect_flag, int show_interior, int split_tri)
{
	vertex	hostile_vertex;
	float		ang;
	float		xpos,ypos,cur_dist,sin_ang,cos_ang;
	int		draw_inside=0;

	// determine if the given object is within the targeting reticle 
	// (which means the triangle is not drawn)

	cur_dist = vm_vec_dist_quick(&Player_obj->pos, hostile_pos);

	g3_rotate_vertex(&hostile_vertex, hostile_pos);
	g3_project_vertex(&hostile_vertex);

	if (hostile_vertex.codes == 0)  { // on screen
		int		projected_x, projected_y;

		if (!(hostile_vertex.flags & PF_OVERFLOW)) {  // make sure point projected
			int mag_squared;

			projected_x = fl2i(hostile_vertex.screen.xyw.x);
			projected_y = fl2i(hostile_vertex.screen.xyw.y);

			unsize(&projected_x, &projected_y);

			mag_squared = (projected_x - position[0]) * (projected_x - position[0]) + 
							  (projected_y - position[1]) * (projected_y - position[1]);

			if ( mag_squared < Radius*Radius ) {
				if ( show_interior ) {
					draw_inside=1;
				} else {
					return;
				}
			}
		}
	}

	int HUD_nose_scaled_x = HUD_nose_x;
	int HUD_nose_scaled_y = HUD_nose_y;

	gr_resize_screen_pos(&HUD_nose_scaled_x, &HUD_nose_scaled_y);

	unsize( &hostile_vertex.world.xyz.x, &hostile_vertex.world.xyz.y );

	ang = atan2_safe(hostile_vertex.world.xyz.y,hostile_vertex.world.xyz.x);
	sin_ang=(float)sin(ang);
	cos_ang=(float)cos(ang);
	
	if ( draw_inside ) {
		xpos = position[0] + cos_ang*(Radius-7);
		ypos = position[1] - sin_ang*(Radius-7);
	} else {
		xpos = position[0] + cos_ang*(Radius+4);
		ypos = position[1] - sin_ang*(Radius+4);
	}

	xpos += HUD_offset_x + HUD_nose_x;
	ypos += HUD_offset_y + HUD_nose_y;
	
	if ( split_tri ) {
		// renderTriangleMissileSplit(ang, xpos, ypos, cur_dist, aspect_flag, draw_inside);
		renderTriangleMissileTail(ang, xpos, ypos, cur_dist, aspect_flag, draw_inside);
	} else {
		float x1=0.0f;
		float x2=0.0f;
		float y1=0.0f;
		float y2=0.0f;

		if ( draw_inside ) {				
			x1 = xpos - Target_triangle_base * -sin_ang;
			y1 = ypos + Target_triangle_base * cos_ang;
			x2 = xpos + Target_triangle_base * -sin_ang;
			y2 = ypos - Target_triangle_base * cos_ang;

			xpos -= Target_triangle_height * cos_ang;
			ypos += Target_triangle_height * sin_ang;

		} else {				
			x1 = xpos - Target_triangle_base * -sin_ang;
			y1 = ypos + Target_triangle_base * cos_ang;
			x2 = xpos + Target_triangle_base * -sin_ang;
			y2 = ypos - Target_triangle_base * cos_ang;

			xpos += Target_triangle_height * cos_ang;
			ypos -= Target_triangle_height * sin_ang;
		}

		//renderPrintf(position[0], position[1], "%d", fl2i((360*ang)/(2*PI)));
		gr_set_screen_scale(base_w, base_h);
		if (aspect_flag) {
			hud_tri(xpos,ypos,x1,y1,x2,y2);
		} else {
			hud_tri_empty(xpos,ypos,x1,y1,x2,y2);
		}
		gr_reset_screen_scale();
	}
}

//	Show all homing missiles locked onto the player.
//	Also, play the beep!
void hud_process_homing_missiles()
{
	object		*A;
	missile_obj	*mo;
	weapon		*wp;
	float			dist, nearest_dist;
	int			closest_is_aspect=0;

	nearest_dist = Homing_beep.max_cycle_dist;

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		A = &Objects[mo->objnum];
		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));

		wp = &Weapons[A->instance];

		if (wp->homing_object == Player_obj) {
			dist = vm_vec_dist_quick(&A->pos, &Player_obj->pos);

			if (dist < nearest_dist) {
				nearest_dist = dist;
				if ( Weapon_info[wp->weapon_info_index].wi_flags & WIF_LOCKED_HOMING ) {
					closest_is_aspect=1;
				} else {
					closest_is_aspect=0;
				}
			}
		}
	}

	//	See if need to play warning beep.
	if (nearest_dist < Homing_beep.max_cycle_dist ) {
		float	delta_time;
		float cycle_time;

		delta_time = f2fl(Missiontime - Homing_beep.last_time_played);

		// figure out the cycle time by doing a linear interpretation
		cycle_time = Homing_beep.min_cycle_time + (nearest_dist-Homing_beep.min_cycle_dist) * Homing_beep.precalced_interp;

		// play a new 'beep' if cycle time has elapsed
		if ( (delta_time*1000) > cycle_time ) {
			Homing_beep.last_time_played = Missiontime;
			if ( snd_is_playing(Homing_beep.snd_handle) ) {
				snd_stop(Homing_beep.snd_handle);
			}

			if ( closest_is_aspect ) {
				Homing_beep.snd_handle = snd_play(&Snds[ship_get_sound(Player_obj, SND_PROXIMITY_ASPECT_WARNING)]);
			} else {
				Homing_beep.snd_handle = snd_play(&Snds[ship_get_sound(Player_obj, SND_PROXIMITY_WARNING)]);
			}
		}
	}
}

HudGaugeMissileTriangles::HudGaugeMissileTriangles():
HudGaugeReticleTriangle(HUD_OBJECT_MISSILE_TRI, HUD_MISSILE_WARNING_ARROW)
{
}

void HudGaugeMissileTriangles::render(float frametime)
{
	object		*A;
	missile_obj	*mo;
	weapon		*wp;

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);
	gr_set_screen_scale(base_w, base_h);

	gr_set_color_fast(&HUD_color_homing_indicator);

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		A = &Objects[mo->objnum];
		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));

		wp = &Weapons[A->instance];

		if (wp->homing_object == Player_obj) {
			renderTriangle(&A->pos, Weapon_info[wp->weapon_info_index].wi_flags & WIF_LOCKED_HOMING, 1, 1);
		}
	}

	gr_reset_screen_scale();
	if(!in_frame)
		g3_end_frame();
}

HudGaugeOrientationTee::HudGaugeOrientationTee():
HudGauge(HUD_OBJECT_ORIENTATION_TEE, HUD_ORIENTATION_TEE, true, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{
}

void HudGaugeOrientationTee::initRadius(int length)
{
	Radius = length;
}

void HudGaugeOrientationTee::pageIn()
{
}

// hud_show_orientation_tee() will draw the orientation gauge that orbits the inside of the 
// outer reticle ring.  If the T is at 12 o'clock, the target is facing the player, if the T
// is at 6 o'clock the target is facing away from the player.  If the T is at 3 or 9 o'clock 
// the target is facing 90 away from the player.
void HudGaugeOrientationTee::render(float frametime)
{
	object* targetp;
	
	if (Player_ai->target_objnum == -1 || Player->target_is_dying)
		return;

	targetp = &Objects[Player_ai->target_objnum];
	
	if ( maybeFlashSexp() == 1 ) {
		hud_set_iff_color( targetp );
	} else {
		hud_set_iff_color( targetp, 1);
	}
	renderOrientation(targetp, Player_obj, &targetp->orient);
}

// routine to draw a bounding box around a remote detonate missile and distance to
void hud_process_remote_detonate_missile()
{
	missile_obj	*mo;
	object	*mobjp;
	vertex target_point;

	// check for currently locked missiles (highest precedence)
	for ( mo = GET_FIRST(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		mobjp = &Objects[mo->objnum];

		if ((Player_obj != NULL) && (mobjp->parent_sig == Player_obj->parent_sig)) {
			if (Weapon_info[Weapons[mobjp->instance].weapon_info_index].wi_flags & WIF_REMOTE) {
				// get box center point
				g3_rotate_vertex(&target_point,&mobjp->pos);

				// project vertex
				g3_project_vertex(&target_point);

				if (!(target_point.flags & PF_OVERFLOW)) {  // make sure point projected
					switch ( mobjp->type ) {
					case OBJ_WEAPON:
						hud_target_add_display_list(mobjp, &target_point, &mobjp->pos, 0, iff_get_color(IFF_COLOR_MESSAGE, 1), NULL, TARGET_DISPLAY_DIST);
						break;

					default:
						Int3();	// should never happen
						return;
					}

					// do only for the first remote detonate missile
					break;
				}
			}
		}
	}
}

// routine to possibly draw a bounding box around a ship sending a message to the player
void hud_show_message_sender()
{
	object *targetp;
	vertex target_point;					// temp vertex used to find screen position for 3-D object;

	// don't draw brackets if no ship sending a message
	if ( Message_shipnum == -1 )
		return;

	targetp = &Objects[Ships[Message_shipnum].objnum];
	Assert ( targetp != NULL );

	Assert ( targetp->type == OBJ_SHIP );

	// Don't do this for the ship you're flying!
	if ( targetp == Player_obj ) {
		return;
	}

	// Goober5000 - don't draw if primitive sensors
	if ( Ships[Player_obj->instance].flags2 & SF2_PRIMITIVE_SENSORS ) {
		return;
	}

	// Karajorma - If we've gone to all the trouble to make our friendly ships stealthed they shouldn't then give away 
	// their position cause they're feeling chatty
	if ( Ships[Message_shipnum].flags2 & SF2_FRIENDLY_STEALTH_INVIS ) {
		return;
	}

	Assert ( targetp->instance >=0 && targetp->instance < MAX_SHIPS );

	// check the object flags to see if this ship is gone.  If so, then don't do this stuff anymore
	if ( targetp->flags & OF_SHOULD_BE_DEAD ) {
		Message_shipnum = -1;
		return;
	}

	// find the current target vertex 
	g3_rotate_vertex(&target_point, &targetp->pos);
	g3_project_vertex(&target_point);

	if (!(target_point.flags & PF_OVERFLOW)) {  // make sure point projected
		hud_target_add_display_list(targetp, &target_point, &targetp->pos, 10, iff_get_color(IFF_COLOR_MESSAGE, 1), NULL, 0);
	} else if (target_point.codes != 0) { // target center is not on screen
		// draw the offscreen indicator at the edge of the screen where the target is closest to
		// AL 11-19-97: only show offscreen indicator if player sensors are functioning
		if ( (OBJ_INDEX(targetp) != Player_ai->target_objnum) || (Message_shipnum == Objects[Player_ai->target_objnum].instance) ) {
			if ( hud_sensors_ok(Player_ship, 0) ) {
				hud_target_add_display_list(targetp, &target_point, &targetp->pos, 0, iff_get_color(IFF_COLOR_MESSAGE, 1), NULL, TARGET_DISPLAY_DIST);
			}
		}
	}
}

// hud_prune_hotkeys()
//
// Check for ships that are dying, departed or dead.  These should be removed from the player's
// hotkey lists.
void hud_prune_hotkeys()
{
	int				i;
	htarget_list	*hitem, *plist;
	object			*objp;
	ship				*sp;

	for ( i = 0; i < MAX_KEYED_TARGETS; i++ ) {
		plist = &(Players[Player_num].keyed_targets[i]);
		if ( EMPTY( plist ) )			// no items in list, then do nothing
			continue;

		hitem = GET_FIRST(plist);
		while ( hitem != END_OF_LIST(plist) ) {
			int remove_item;

			remove_item = 0;

			objp = hitem->objp;
			Assert ( objp != NULL );
			if ( objp->type == OBJ_SHIP ) {
				Assert ( objp->instance >=0 && objp->instance < MAX_SHIPS );
				sp = &Ships[objp->instance];
			} else {
				// if the object isn't a ship, it shouldn't be on the list, so remove it without question
				remove_item = 1;
				sp = NULL;
			}

			// check to see if the object is dying -- if so, remove it from the list
			// check to see if the ship is departing -- if so, remove it from the list
			if ( remove_item || (objp->flags & OF_SHOULD_BE_DEAD) || (sp->flags & (SF_DEPARTING|SF_DYING)) ) {
				if ( sp != NULL ) {
					nprintf(("Network", "Hotkey: Pruning %s\n", sp->ship_name));
				}

				htarget_list *temp;
				temp = GET_NEXT(hitem);
				list_remove( plist, hitem );
				list_append( &htarget_free_list, hitem );
				hitem->objp = NULL;
				hitem = temp;
				continue;
			}
			hitem = GET_NEXT( hitem );
		}	// end while
	}	// end for

	// save the hotkey sets with mission time reaches a certain point.  Code was put here because this
	// function always called for both single/multiplayer.  Maybe not the best location, but whatever.
	mission_hotkey_maybe_save_sets();
}

int HUD_drew_selection_bracket_on_target;

// hud_show_selection_set draws some indicator around all the ships in the current selection set.  No
// indicators will be drawn if there is only 1 ship in the set.
void hud_show_selection_set()
{
	htarget_list *hitem, *plist;
	object *targetp;
	int set, count;
	vertex target_point;					// temp vertex used to find screen position for 3-D object;

	HUD_drew_selection_bracket_on_target = 0;

	set = Players[Player_num].current_hotkey_set;
	if ( set == -1 )
		return;

	Assert ( (set >= 0) && (set < MAX_KEYED_TARGETS) );
	plist = &(Players[Player_num].keyed_targets[set]);

	count = 0;
	for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) )
		count++;

	if ( count == 0 )	{	// only one ship, do nothing
		Players[Player_num].current_hotkey_set = -1;
		return;
	}

	for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
		targetp = hitem->objp;
		Assert ( targetp != NULL );
	
		ship	*target_shipp = NULL;

		Assert ( targetp->type == OBJ_SHIP );
		Assert ( targetp->instance >=0 && targetp->instance < MAX_SHIPS );
		target_shipp = &Ships[targetp->instance];

		if ( (Game_mode & GM_MULTIPLAYER) && (target_shipp == Player_ship) ) {
			continue;
		}

		// Goober5000 - don't draw indicators for non-visible ships, per Mantis #1972
		// (the only way we could get here is if the hotkey set contained a mix of visible
		// and invisible ships)
		if (awacs_get_level(targetp, Player_ship, 1) < 1) {
			continue;
		}

		// find the current target vertex 
		//
		g3_rotate_vertex(&target_point,&targetp->pos);
		g3_project_vertex(&target_point);

		if (!(target_point.flags & PF_OVERFLOW)) {  // make sure point projected

			switch ( targetp->type ) {
			case OBJ_SHIP:
				break;

			default:
				Int3();	// should never happen
				return;
			}
			if ( OBJ_INDEX(targetp) == Player_ai->target_objnum ) {
				hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, iff_get_color(IFF_COLOR_SELECTION, 1), NULL, 0);
				HUD_drew_selection_bracket_on_target = 1;
			} else if ( Cmdline_targetinfo ) {		//Backslash -- show the distance and a lead indicator
				hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, iff_get_color(IFF_COLOR_SELECTION, 1), NULL, TARGET_DISPLAY_DIST | TARGET_DISPLAY_LEAD);
			} else {
				hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, iff_get_color(IFF_COLOR_SELECTION, 1), NULL, 0);
			}
		}

		if (target_point.codes != 0) { // target center is not on screen
			// draw the offscreen indicator at the edge of the screen where the target is closest to
			// AL 11-19-97: only show offscreen indicator if player sensors are functioning

			if ( OBJ_INDEX(targetp) != Player_ai->target_objnum ) {
				if ( hud_sensors_ok(Player_ship, 0) ) {
					hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, iff_get_color(IFF_COLOR_SELECTION, 1), NULL, 0);
				}
			}
		}
	}
}

// hud_show_targeting_gauges() will display the targeting information on the HUD.  Called once per frame.
//
// Must be inside a g3_start_frame()
// input:	frametime	=>		time in seconds since last update
//				in_cockpit	=>		flag (default value 1) indicating whether viewpoint is from cockpit or external
void hud_show_targeting_gauges(float frametime)
{
	vertex target_point;					// temp vertex used to find screen position for 3-D object;
	vec3d target_pos;

	hud_show_hostile_triangle();

	if (Player_ai->target_objnum == -1)
		return;

	object * targetp = &Objects[Player_ai->target_objnum];
	Players[Player_num].lead_indicator_active = 0;

	// check to see if there is even a current target
	if ( targetp == &obj_used_list ) {
		return;
	}

	// AL 1/20/97: Point to targted subsystem if one exists
	if ( Player_ai->targeted_subsys != NULL ) {
		get_subsystem_world_pos(targetp, Player_ai->targeted_subsys, &target_pos);
	} else {
		target_pos = targetp->pos;
	}

	// find the current target vertex 
	//
	// The 2D screen pos depends on the current viewer position and orientation.  
	g3_rotate_vertex(&target_point,&target_pos);

	hud_set_iff_color( targetp, 1 );
	g3_project_vertex(&target_point);

	if (!(target_point.flags & PF_OVERFLOW)) {  // make sure point projected
		if (target_point.codes == 0) { // target center is not on screen
			int target_display_flags;

			if(Cmdline_targetinfo) {
				target_display_flags = TARGET_DISPLAY_DIST | TARGET_DISPLAY_DOTS | TARGET_DISPLAY_SUBSYS | TARGET_DISPLAY_NAME | TARGET_DISPLAY_CLASS;
			} else {
				target_display_flags = TARGET_DISPLAY_DIST | TARGET_DISPLAY_DOTS | TARGET_DISPLAY_SUBSYS;
			}

			hud_target_add_display_list(targetp, &target_point, &targetp->pos, 0, NULL, NULL, target_display_flags);
		}
	} else {
		Hud_target_w = 0;
		Hud_target_h = 0;
	}

	// update cargo scanning
	hud_cargo_scan_update(targetp, frametime);

	// display the lock indicator
	if (!Player->target_is_dying) {
		hud_do_lock_indicator(frametime);

		// update and render artillery 
		hud_artillery_update();
		hud_artillery_render();
	}

	if (target_point.codes != 0) {
		// draw the offscreen indicator at the edge of the screen where the target is closest to
		Assert(Player_ai->target_objnum >= 0);

		// AL 11-11-97:	don't draw the indicator if the ship is messaging, the indicator is drawn
		// in the message sending color in hud_show_message_sender()
		if ( Message_shipnum != Objects[Player_ai->target_objnum].instance ) {
			hud_target_add_display_list(targetp, &target_point, &targetp->pos, 0, NULL, NULL, 0);
		}
	}
}

// hud_show_hostile_triangle() will draw an empty triangle that oribits around the outer
// circle of the reticle.  It will point to the closest enemy that is firing on the player.
// Currently, it points to the closest enemy that has the player as its target_objnum and has
// SM_ATTACK or SM_SUPER_ATTACK as its ai submode.
void hud_show_hostile_triangle()
{
	object* A;
	float min_distance=1e20f;
	float new_distance=0.0f;
	object* nearest_obj = &obj_used_list;
	ai_info *aip;
	ship_obj	*so;
	ship		*sp;
	ship_subsys *ss;

	int player_obj_index = OBJ_INDEX(Player_obj);
	int turret_is_attacking = 0;

	hostile_obj = NULL;
	
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list);  so = GET_NEXT(so) ) {

		A = &Objects[so->objnum];
		sp = &Ships[A->instance];

		// only look at ships who attack us
		if ( (A == Player_obj) || !(iff_x_attacks_y(Ships[A->instance].team, Player_ship->team)) ) {
			continue;
		}

		aip = &Ai_info[Ships[A->instance].ai_index];

		// don't look at ignore ships
		if ( sp->flags & TARGET_SHIP_IGNORE_FLAGS ) {
			continue;
		}

		// always ignore cargo containers and navbuoys
		if ( Ship_info[sp->ship_info_index].class_type > -1 && !(Ship_types[Ship_info[sp->ship_info_index].class_type].hud_bools & STI_HUD_SHOW_ATTACK_DIRECTION) ) {
			continue;
		}

		// check if ship is stealthy
		if (awacs_get_level(&Objects[sp->objnum], Player_ship, 1) < 1) {
			continue;
		}

		turret_is_attacking = 0;

		// check if any turrets on ship are firing at the player (only on non fighter-bombers)
		if ( !(Ship_info[sp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER)) ) {
			for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
				if (ss->flags & SSF_UNTARGETABLE)
					continue;

				if ( (ss->system_info->type == SUBSYSTEM_TURRET) && (ss->current_hits > 0) ) {

					if ( ss->turret_enemy_objnum == player_obj_index ) {
						turret_is_attacking = 1;						

						vec3d		gsubpos;
						// get world pos of subsystem
						vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &A->orient);
						vm_vec_add2(&gsubpos, &A->pos);
						new_distance = vm_vec_dist_quick(&gsubpos, &Player_obj->pos);

						if (new_distance <= min_distance) {
							min_distance=new_distance;
							nearest_obj = A;
						}
					}
				}
			}
		}

		if ( !turret_is_attacking ) {
			// check for ships attacking the player
			if ( aip->target_objnum != Player_ship->objnum ) {
				continue;
			}

			// ignore enemy if not in chase mode
			if ( (Game_mode & GM_NORMAL) && (aip->mode != AIM_CHASE) ) {
				continue;
			}

			new_distance = vm_vec_dist_quick(&A->pos, &Player_obj->pos);

			if (new_distance <= min_distance) {
				min_distance=new_distance;
				nearest_obj = A;
			}
		}
	}

	if ( nearest_obj == &obj_used_list ) {
		return;
	}

	if ( min_distance > MIN_DISTANCE_TO_CONSIDER_THREAT ) {
		return;
	}

	hostile_obj = nearest_obj;

	// hook to maybe warn player about this attacking ship
	ship_maybe_warn_player(&Ships[nearest_obj->instance], min_distance);
}

HudGaugeHostileTriangle::HudGaugeHostileTriangle():
HudGaugeReticleTriangle(HUD_OBJECT_HOSTILE_TRI, HUD_HOSTILE_TRIANGLE)
{
}

void HudGaugeHostileTriangle::render(float frametime)
{
	if (hostile_obj && maybeFlashSexp() != 1) {
		bool in_frame = g3_in_frame() > 0;
		if(!in_frame)
			g3_start_frame(0);
		
		// hud_set_iff_color( TEAM_HOSTILE, 1 );	//	Note: This should really be TEAM_HOSTILE, not opposite of Player_ship->team.
		hud_set_iff_color( hostile_obj, 1 );
		renderTriangle(&hostile_obj->pos, 0, 1, 0);

		if(!in_frame)
			g3_end_frame();
	}
}

void hud_calculate_lead_pos(vec3d *lead_target_pos, vec3d *target_pos, object *targetp, weapon_info	*wip, float dist_to_target, vec3d *rel_pos)
{
	vec3d target_moving_direction;
	vec3d last_delta_vector;
	float time_to_target, target_moved_dist;

	if(wip->max_speed != 0) {
		time_to_target = dist_to_target / wip->max_speed;
	} else {
		time_to_target = 0;
	}
	
	target_moved_dist = targetp->phys_info.speed * time_to_target;

	target_moving_direction = targetp->phys_info.vel;

	if(The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY)
		vm_vec_sub2(&target_moving_direction, &Player_obj->phys_info.vel);
	
	// test if the target is moving at all
	if ( vm_vec_mag_quick(&target_moving_direction) < 0.1f ) { // Find distance!
		*lead_target_pos =  *target_pos;
	} else {
		vm_vec_normalize(&target_moving_direction);
		vm_vec_scale(&target_moving_direction, target_moved_dist);
		vm_vec_add(lead_target_pos, target_pos, &target_moving_direction );
		polish_predicted_target_pos(wip, targetp, target_pos, lead_target_pos, dist_to_target, &last_delta_vector, 1); // Not used:, float time_to_enemy)

		if(rel_pos) { // needed for quick lead indicators, not needed for normal lead indicators.
			vm_vec_add2(lead_target_pos, rel_pos);
		}
	}
}

// Return the bank number for the primary weapon that can fire the farthest, from
// the number of active primary weapons
// input: range	=>	output parameter... it is the range of the selected bank
int hud_get_best_primary_bank(float *range)
{
	int	i, best_bank, bank_to_fire, num_to_test;
	float	weapon_range, farthest_weapon_range;
	ship_weapon	*swp;
	weapon_info	*wip;

	swp = &Player_ship->weapons;

	farthest_weapon_range = 0.0f;
	best_bank = -1;

	if ( Player_ship->flags & SF_PRIMARY_LINKED ) {
		num_to_test = swp->num_primary_banks;
	} else {
		num_to_test = MIN(1, swp->num_primary_banks);
	}

	for ( i = 0; i < num_to_test; i++ )
	{	
		bank_to_fire = (swp->current_primary_bank + i) % swp->num_primary_banks;

		// calculate the range of the weapon, and only display the lead target indicator
		// if the weapon can actually hit the target
		Assert(bank_to_fire >= 0 && bank_to_fire < swp->num_primary_banks);
		Assert(swp->primary_bank_weapons[bank_to_fire] < MAX_WEAPON_TYPES);

		if (swp->primary_bank_weapons[bank_to_fire] < 0)
			continue;

		wip = &Weapon_info[swp->primary_bank_weapons[bank_to_fire]];
		weapon_range = MIN((wip->max_speed * wip->lifetime), wip->weapon_range);

		// don't consider this primary if it's a ballistic that's out of ammo - Goober5000
		if ( wip->wi_flags2 & WIF2_BALLISTIC )
		{
			if ( swp->primary_bank_ammo[bank_to_fire] <= 0)
			{
				continue;
			}
		}

		if ( weapon_range > farthest_weapon_range )
		{
			best_bank = bank_to_fire;
			farthest_weapon_range = weapon_range;
		}
	}

	*range = farthest_weapon_range;
	return best_bank;
}

// -----------------------------------------------------------------------------
//	polish_predicted_target_pos()
// 
// Called by the draw lead indicator code to predict where the enemy is going to be
//
void polish_predicted_target_pos(weapon_info *wip, object *targetp, vec3d *enemy_pos, vec3d *predicted_enemy_pos, float dist_to_enemy, vec3d *last_delta_vec, int num_polish_steps) 
{
	int	iteration;
	vec3d	player_pos = Player_obj->pos;	
	float		time_to_enemy;
	vec3d	last_predicted_enemy_pos = *predicted_enemy_pos;

	float	weapon_speed = wip->max_speed;

	vm_vec_zero(last_delta_vec);

	// additive velocity stuff
	// not just the player's main target
	vec3d enemy_vel = targetp->phys_info.vel;
	if (The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY) {
		vm_vec_sub2( &enemy_vel, &Player_obj->phys_info.vel );
	}
	
	for (iteration=0; iteration < num_polish_steps; iteration++) {
		dist_to_enemy = vm_vec_dist_quick(predicted_enemy_pos, &player_pos);
		time_to_enemy = dist_to_enemy/weapon_speed;
		vm_vec_scale_add(predicted_enemy_pos, enemy_pos, &enemy_vel, time_to_enemy);
		vm_vec_sub(last_delta_vec, predicted_enemy_pos, &last_predicted_enemy_pos);
		last_predicted_enemy_pos= *predicted_enemy_pos;
	}
}

HudGaugeLeadIndicator::HudGaugeLeadIndicator():
HudGauge(HUD_OBJECT_LEAD, HUD_LEAD_INDICATOR, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{

}

void HudGaugeLeadIndicator::initHalfSize(float w, float h)
{
	Lead_indicator_half[0] = w;
	Lead_indicator_half[1] = h;
}

void HudGaugeLeadIndicator::initBitmaps(char *fname)
{
	Lead_indicator_gauge.first_frame = bm_load_animation(fname, &Lead_indicator_gauge.num_frames);
	if ( Lead_indicator_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeLeadIndicator::pageIn()
{
	bm_page_in_aabitmap(Lead_indicator_gauge.first_frame, Lead_indicator_gauge.num_frames);
}

// determine the correct frame to draw for the lead indicator
// 0 -> center only	(in secondary range only)
// 1 -> full			(in secondary and primary range)
//	2 -> oustide only	(in primary range only)
//
// input:	prange	=>	range of current primary weapon
//				srange	=>	range of current secondary weapon
//				dist_to_target	=>	current dist to target
//
// exit:		0-2	=>	frame offset
//				-1		=>	don't draw anything
int HudGaugeLeadIndicator::pickFrame(float prange, float srange, float dist_to_target)
{
	int frame_offset=-1;
	int in_prange=0, in_srange=0;

	if ( dist_to_target < prange ) {
		in_prange=1;
	}

	if ( dist_to_target < srange ) {
		in_srange=1;
	}

	if ( in_prange && in_srange ) {
		frame_offset=1;
	} else if ( in_prange && !in_srange ) {
		frame_offset=2;
	} else if ( !in_prange && in_srange ) {
		frame_offset=0;
	} else {
		frame_offset=-1;
	}

	return frame_offset;
}

void HudGaugeLeadIndicator::render(float frametime)
{
	if(Player->target_is_dying) {
		return;
	}

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);

	// first render the current target the player has selected.
	renderLeadCurrentTarget();

	// if extra targeting info is enabled, render lead indicators for objects in the target display list.
	for(size_t i = 0; i < target_display_list.size(); i++) {
		if ( (target_display_list[i].flags & TARGET_DISPLAY_LEAD) && target_display_list[i].objp ) {

			// set the color
			if( target_display_list[i].bracket_clr.red && target_display_list[i].bracket_clr.green &&
				target_display_list[i].bracket_clr.blue ) {
				gr_set_color_fast(&target_display_list[i].bracket_clr);
			} else {
				// use IFF colors if none defined.
				hud_set_iff_color(target_display_list[i].objp, 1);
			}

			renderLeadQuick(&target_display_list[i].target_pos, target_display_list[i].objp);
		}
	}

	if(!in_frame)
		g3_end_frame();
}

void HudGaugeLeadIndicator::renderIndicator(int frame_offset, object *targetp, vec3d *lead_target_pos)
{
	vertex lead_target_vertex;
	int sx, sy;

	g3_rotate_vertex(&lead_target_vertex, lead_target_pos);

	if (lead_target_vertex.codes == 0) { // on screen
		g3_project_vertex(&lead_target_vertex);

		if (!(lead_target_vertex.flags & PF_OVERFLOW)) {
			if ( maybeFlashSexp() == 1 ) {
				hud_set_iff_color(targetp, 0);
			} else {
				hud_set_iff_color(targetp, 1);
			}

			if ( Lead_indicator_gauge.first_frame + frame_offset >= 0 ) {
				sx = fl2i(lead_target_vertex.screen.xyw.x);
				sy = fl2i(lead_target_vertex.screen.xyw.y);

				unsize(&sx, &sy);
				renderBitmap(Lead_indicator_gauge.first_frame + frame_offset, fl2i(sx - Lead_indicator_half[0]),  fl2i(sy - Lead_indicator_half[1]));
			}
		}
	}
}

// HudGaugeLeadIndicator::renderTargetLead() determine where to draw the lead target box and display it
void HudGaugeLeadIndicator::renderLeadCurrentTarget()
{
	vec3d		target_pos;
	vec3d		source_pos;
	vec3d		*rel_pos;
	vec3d		lead_target_pos;
	object		*targetp;
	polymodel	*pm;
	ship_weapon	*swp;
	weapon_info	*wip;
	weapon_info	*tmp=NULL;
	float			dist_to_target, prange, srange;
	int			bank_to_fire, frame_offset;

	if (Player_ai->target_objnum == -1)
		return;

	targetp = &Objects[Player_ai->target_objnum];
	if ( (targetp->type != OBJ_SHIP) && (targetp->type != OBJ_WEAPON) && (targetp->type != OBJ_ASTEROID) ) {
		return;
	}

	// only allow bombs to have lead indicator displayed
	if ( targetp->type == OBJ_WEAPON ) {
		if ( !(Weapon_info[Weapons[targetp->instance].weapon_info_index].wi_flags2 & WIF2_CAN_BE_TARGETED) ) {
			if ( !(Weapon_info[Weapons[targetp->instance].weapon_info_index].wi_flags & WIF_BOMB) ) {
				return;
			}
		}
	}

	// If the target is out of range, then draw the correct frame for the lead indicator
	if ( Lead_indicator_gauge.first_frame == -1 ) {
		Int3();
		return;
	}

	// AL 1/20/97: Point to targted subsystem if one exists
	if ( Player_ai->targeted_subsys != NULL ) {
		get_subsystem_world_pos(targetp, Player_ai->targeted_subsys, &target_pos);
	} else {
		target_pos = targetp->pos;
	}
	
	pm = model_get(Ship_info[Player_ship->ship_info_index].model_num);
	swp = &Player_ship->weapons;

	// Added to take care of situation where there are no primary banks on the player ship
	// (this may not be possible, depending on what we decide for the weapons loadout rules)
	if ( swp->num_primary_banks == 0 )
		return;

	bank_to_fire = hud_get_best_primary_bank(&prange);

	if ( bank_to_fire < 0 )
		return;

	wip = &Weapon_info[swp->primary_bank_weapons[bank_to_fire]];
			
	if (pm->n_guns && bank_to_fire != -1 ) {
		rel_pos = &pm->gun_banks[bank_to_fire].pnt[0];
	} else {
		rel_pos = NULL;
	}

	// source_pos will contain the world coordinate of where to base the lead indicator prediction
	// from.  Normally, this will be the world pos of the gun turret of the currently selected primary
	// weapon.
	source_pos = Player_obj->pos;
	if (rel_pos != NULL) {
		vec3d	gun_point;
		vm_vec_unrotate(&gun_point, rel_pos, &Player_obj->orient);
		vm_vec_add2(&source_pos, &gun_point);
	} 
	
	// Determine "accurate" distance to target.  This is the distance from the player ship
	// to the closest point on the bounding box of the target
	dist_to_target = hud_find_target_distance(targetp, Player_obj);

	srange = ship_get_secondary_weapon_range(Player_ship);

	if ( swp->current_secondary_bank >= 0 )
	{
		int bank = swp->current_secondary_bank;
		tmp = &Weapon_info[swp->secondary_bank_weapons[bank]];
		if ( !(tmp->wi_flags & WIF_HOMING) && !(tmp->wi_flags & WIF_LOCKED_HOMING && Player->target_in_lock_cone) ) {
			//The secondary lead indicator is handled farther below if it is a non-locking type
			srange = -1.0f;
		}
	}

	frame_offset = pickFrame(prange, srange, dist_to_target);
	if ( frame_offset < 0 ) {
		return;
	}

	hud_calculate_lead_pos(&lead_target_pos, &target_pos, targetp, wip, dist_to_target);
	renderIndicator(frame_offset, targetp, &lead_target_pos);

	//do dumbfire lead indicator - color is orange (255,128,0) - bright, (192,96,0) - dim
	//phreak changed 9/01/02
	if(swp->current_secondary_bank>=0) {
		int bank=swp->current_secondary_bank;
		wip=&Weapon_info[swp->secondary_bank_weapons[bank]];

		//get out of here if the secondary weapon is a homer or if its out of range
		if ( wip->wi_flags & WIF_HOMING )
			return;

		double max_dist = MIN((wip->lifetime * wip->max_speed), wip->weapon_range);

		if (dist_to_target > max_dist)
			return;
	}

	hud_calculate_lead_pos(&lead_target_pos, &target_pos, targetp, wip, dist_to_target);
	renderIndicator(0, targetp, &lead_target_pos);
}

//Backslash
// A stripped-down version of the lead indicator, only shows primary weapons
// and works for a specified target (not just the current selected target).
// Ideally I'd like to later turn this into something (or make a new function) that would actually WORK with gun convergence/normals
// instead of the existing code (copied from above) that does some calculations and then is ignored ;-)
// (Go look, what's it actually DO with source_pos?)
// And also, something that could be called for multiple weapons, ITTS style.
void HudGaugeLeadIndicator::renderLeadQuick(vec3d *target_world_pos, object *targetp)
{
	vec3d		source_pos;
	vec3d		*rel_pos;
	vec3d		lead_target_pos;
	polymodel	*pm;
	ship_weapon	*swp;
	weapon_info	*wip;
	float			dist_to_target, prange;
	int			bank_to_fire, frame_offset;

	if ( (targetp->type != OBJ_SHIP) && (targetp->type != OBJ_WEAPON) && (targetp->type != OBJ_ASTEROID) ) {
		return;
	}

	// only allow bombs to have lead indicator displayed
	if ( targetp->type == OBJ_WEAPON ) {
		if ( !(Weapon_info[Weapons[targetp->instance].weapon_info_index].wi_flags2 & WIF2_CAN_BE_TARGETED) ) {
			if ( !(Weapon_info[Weapons[targetp->instance].weapon_info_index].wi_flags & WIF_BOMB) ) {
				return;
			}
		}
	}

	// If the target is out of range, then draw the correct frame for the lead indicator
	if ( Lead_indicator_gauge.first_frame == -1 ) {
		Int3();
		return;
	}
	
	pm = model_get(Ship_info[Player_ship->ship_info_index].model_num);
	swp = &Player_ship->weapons;

	// Added to take care of situation where there are no primary banks on the player ship
	// (this may not be possible, depending on what we decide for the weapons loadout rules)
	if ( swp->num_primary_banks == 0 )
		return;

	bank_to_fire = hud_get_best_primary_bank(&prange);	//Backslash note: this!
	if ( bank_to_fire < 0 )
		return;
	wip = &Weapon_info[swp->primary_bank_weapons[bank_to_fire]];
			
	if (pm->n_guns && bank_to_fire != -1 ) {
		rel_pos = &pm->gun_banks[bank_to_fire].pnt[0];
	} else {
		rel_pos = NULL;
	}
//							vec3d firing_vec;
//							vm_vec_unrotate(&firing_vec, &po->gun_banks[bank_to_fire].norm[pt], &obj->orient);
//							vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);

	// source_pos will contain the world coordinate of where to base the lead indicator prediction
	// from.  Normally, this will be the world pos of the gun turret of the currently selected primary
	// weapon.
	source_pos = Player_obj->pos;
	if (rel_pos != NULL) {
		vec3d	gun_point;
		vm_vec_unrotate(&gun_point, rel_pos, &Player_obj->orient);
		vm_vec_add2(&source_pos, &gun_point);
	} 
	
	// Determine "accurate" distance to target.  This is the distance from the player ship
	// to the closest point on the bounding box of the target
	dist_to_target = hud_find_target_distance(targetp, Player_obj);

	frame_offset = pickFrame(prange, -1.0f, dist_to_target);
	if ( frame_offset < 0 ) {
		return;
	}

	hud_calculate_lead_pos(&lead_target_pos, target_world_pos, targetp, wip, dist_to_target, rel_pos);
	renderIndicator(frame_offset, targetp, &lead_target_pos);
}

HudGaugeLeadSight::HudGaugeLeadSight():
HudGauge(HUD_OBJECT_LEAD_SIGHT, HUD_LEAD_INDICATOR, true, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{
}

void HudGaugeLeadSight::initBitmaps(char *fname)
{
	Lead_sight.first_frame = bm_load_animation(fname, &Lead_sight.num_frames);

	if ( Lead_sight.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	} else {
		int w, h;

		bm_get_info(Lead_sight.first_frame, &w, &h);
		Lead_sight_half[0] = fl2i(w * 0.5f);
		Lead_sight_half[1] = fl2i(h * 0.5f);
	}
}

void HudGaugeLeadSight::renderSight(int frame_offset, vec3d *target_pos, vec3d *lead_target_pos)
{
	vertex target_vertex;
	float target_sx;
	float target_sy;

	vertex lead_target_vertex;
	float target_lead_sx;
	float target_lead_sy;
	
	// first see if the lead is on screen
	g3_rotate_vertex(&lead_target_vertex, lead_target_pos);

	if (lead_target_vertex.codes != 0)  
		return;

	g3_project_vertex(&lead_target_vertex);

	if (lead_target_vertex.flags & PF_OVERFLOW) 
		return;

	target_lead_sx = lead_target_vertex.screen.xyw.x;
	target_lead_sy = lead_target_vertex.screen.xyw.y; 

	// now see if the target is on screen
	g3_rotate_vertex(&target_vertex, target_pos);
	
	if (target_vertex.codes != 0) 
		return;

	g3_project_vertex(&target_vertex);

	if (target_vertex.flags & PF_OVERFLOW) 
		return;

	target_sx = target_vertex.screen.xyw.x;
	target_sy = target_vertex.screen.xyw.y;

	// render the lead sight
	if ( Lead_sight.first_frame >= 0 ) {

		unsize(&target_lead_sx, &target_lead_sy);
		unsize(&target_sx, &target_sy);
		
		float reticle_target_sx = target_sx - Lead_sight_half[0] - target_lead_sx;
		float reticle_target_sy = target_sy - Lead_sight_half[1] - target_lead_sy;
								
		reticle_target_sx += position[0] + 0.5f;
		reticle_target_sy += position[1] + 0.5f;
		
		setGaugeColor();
		renderBitmap(Lead_sight.first_frame + frame_offset, fl2i(reticle_target_sx) + fl2i(HUD_offset_x), fl2i(reticle_target_sy) + fl2i(HUD_offset_y));
	}
}

void HudGaugeLeadSight::pageIn()
{
	bm_page_in_aabitmap(Lead_sight.first_frame, Lead_sight.num_frames);
}

void HudGaugeLeadSight::render(float frametime)
{
	vec3d		target_pos;
	vec3d		source_pos;
	vec3d		*rel_pos;
	vec3d		lead_target_pos;
	object		*targetp;
	polymodel	*pm;
	ship_weapon	*swp;
	weapon_info	*wip;
	weapon_info	*tmp=NULL;
	float		dist_to_target, prange, srange;
	int			bank_to_fire;

	if (Player_ai->target_objnum == -1)
		return;

	targetp = &Objects[Player_ai->target_objnum];
	if ( (targetp->type != OBJ_SHIP) && (targetp->type != OBJ_WEAPON) && (targetp->type != OBJ_ASTEROID) ) {
		return;
	}

	// only allow bombs to have lead indicator displayed
	if ( targetp->type == OBJ_WEAPON ) {
		if ( !(Weapon_info[Weapons[targetp->instance].weapon_info_index].wi_flags & WIF_BOMB) ) {
			return;
		}
	}

	// If the target is out of range, then draw the correct frame for the lead indicator
	if ( Lead_sight.first_frame == -1 ) {
		Int3();
		return;
	}

	// AL 1/20/97: Point to targeted subsystem if one exists
	if ( Player_ai->targeted_subsys != NULL ) {
		get_subsystem_world_pos(targetp, Player_ai->targeted_subsys, &target_pos);
	} else {
		target_pos = targetp->pos;
	}
	
	pm = model_get(Ship_info[Player_ship->ship_info_index].model_num);
	swp = &Player_ship->weapons;

	// Added to take care of situation where there are no primary banks on the player ship
	// (this may not be possible, depending on what we decide for the weapons loadout rules)
	if ( swp->num_primary_banks == 0 )
		return;

	bank_to_fire = hud_get_best_primary_bank(&prange);
	if ( bank_to_fire < 0 )
		return;
	wip = &Weapon_info[swp->primary_bank_weapons[bank_to_fire]];
			
	if (pm->n_guns && bank_to_fire != -1 ) {
		rel_pos = &pm->gun_banks[bank_to_fire].pnt[0];
	} else {
		rel_pos = NULL;
	}

	// source_pos will contain the world coordinate of where to base the lead indicator prediction
	// from.  Normally, this will be the world pos of the gun turret of the currently selected primary
	// weapon.
	source_pos = Player_obj->pos;
	if (rel_pos != NULL) {
		vec3d	gun_point;
		vm_vec_unrotate(&gun_point, rel_pos, &Player_obj->orient);
		vm_vec_add2(&source_pos, &gun_point);
	} 
	
	// Determine "accurate" distance to target.  This is the distance from the player ship
	// to the closest point on the bounding box of the target
	dist_to_target = hud_find_target_distance(targetp, Player_obj);

	srange = ship_get_secondary_weapon_range(Player_ship);

	if ( swp->current_secondary_bank >= 0 ) {
		int bank = swp->current_secondary_bank;
		tmp = &Weapon_info[swp->secondary_bank_weapons[bank]];
		if ( !(tmp->wi_flags & WIF_HOMING) && !(tmp->wi_flags & WIF_LOCKED_HOMING && Player->target_in_lock_cone) ) {
			//The secondary lead indicator is handled farther below if it is a non-locking type
			srange = -1.0f;
		}
	}
	
	bool in_frame;
	if ( dist_to_target < prange ) {
		// fire it up
		in_frame = g3_in_frame() > 0;
		if(!in_frame) {
			g3_start_frame(0);
		}

		hud_calculate_lead_pos(&lead_target_pos, &target_pos, targetp, wip, dist_to_target);
		renderSight(1, &target_pos, &lead_target_pos); // render the primary weapon lead sight

		if(!in_frame) {
			g3_end_frame();
		}
	}

	//do dumbfire lead indicator - color is orange (255,128,0) - bright, (192,96,0) - dim
	//phreak changed 9/01/02
	if(swp->current_secondary_bank>=0)
	{
		int bank=swp->current_secondary_bank;
		wip=&Weapon_info[swp->secondary_bank_weapons[bank]];

		//get out of here if the secondary weapon is a homer or if its out of range
		if ( wip->wi_flags & WIF_HOMING ) {
			return;
		}

		double max_dist = MIN((wip->lifetime * wip->max_speed), wip->weapon_range);

		if (dist_to_target > max_dist) {
			return;
		}
	} else {
		return;
	}
	
	// fire it up
	in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);
	}
		
	//give it the "in secondary range frame

	hud_calculate_lead_pos(&lead_target_pos, &target_pos, targetp, wip, dist_to_target);
	renderSight(0, &target_pos, &lead_target_pos); // now render the secondary weapon lead sight

	if(!in_frame) {
		g3_end_frame();
	}
}

// hud_cease_subsystem_targeting() will cease targeting the current targets subsystems
//
void hud_cease_subsystem_targeting(int print_message)
{
	int ship_index;

	ship_index = Objects[Player_ai->target_objnum].instance;
	if ( ship_index < 0 )
		return;

	Ships[ship_index].last_targeted_subobject[Player_num] = NULL;
	Player_ai->targeted_subsys = NULL;
	Player_ai->targeted_subsys_parent = -1;
	if ( print_message ) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Deactivating sub-system targeting", 324));
	}

	hud_stop_looped_locking_sounds();
	hud_lock_reset();
}

// hud_cease_targeting() will cease all targeting (main target and subsystem)
//
void hud_cease_targeting()
{
	set_target_objnum( Player_ai, -1 );
	Players[Player_num].flags &= ~PLAYER_FLAGS_AUTO_TARGETING;
	hud_cease_subsystem_targeting(0);
	HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Deactivating targeting system", 325));
	hud_lock_reset();
}

// hud_restore_subsystem_target() will remember the last targeted subsystem
// on a target.
//
void hud_restore_subsystem_target(ship* shipp)
{
	// check if there was a previously targeted sub-system for this target
	if ( shipp->last_targeted_subobject[Player_num] != NULL ) {
		Player_ai->targeted_subsys = shipp->last_targeted_subobject[Player_num];
		Player_ai->targeted_subsys_parent = Player_ai->target_objnum;
	}
	else {
		Player_ai->targeted_subsys = NULL;
		Player_ai->targeted_subsys_parent = -1;
	}
}
 
// --------------------------------------------------------------------------------
// get_subsystem_world_pos() returns the world position for a given subsystem on a ship
//
vec3d* get_subsystem_world_pos(object* parent_obj, ship_subsys* subsys, vec3d* world_pos)
{
	get_subsystem_pos(world_pos, parent_obj, subsys);

	return world_pos;
}

// ----------------------------------------------------------------------------
// hud_target_change_check()
//
// called once per frame to account for when the target changes
//
void hud_target_change_check()
{
	float current_speed=0.0f;

	// Check if player subsystem target has changed, and reset necessary player flag
	if ( Player_ai->targeted_subsys != Player_ai->last_subsys_target ) {
		Player->subsys_in_view=-1;
	}

	// check if the main target has changed
	if (Player_ai->last_target != Player_ai->target_objnum) {

		if ( Player_ai->target_objnum != -1){
			snd_play( &Snds[ship_get_sound(Player_obj, SND_TARGET_ACQUIRE)], 0.0f );
		}

		// if we have a hotkey set active, see if new target is in set.  If not in
		// set, deselect the current hotkey set.
		if (	Player->current_hotkey_set != -1 ) {
			htarget_list *hitem, *plist;

			plist = &(Player->keyed_targets[Player->current_hotkey_set]);
			for ( hitem = GET_FIRST(plist); hitem != END_OF_LIST(plist); hitem = GET_NEXT(hitem) ) {
				if ( OBJ_INDEX(hitem->objp) == Player_ai->target_objnum ){
					break;
				}
			}
			if ( hitem == END_OF_LIST(plist) ){
				Player->current_hotkey_set = -1;
			}
		}

		player_stop_cargo_scan_sound();
		hud_shield_hit_reset();
		hud_targetbox_init_flash();
		hud_targetbox_start_flash(TBOX_FLASH_NAME);
		hud_gauge_popup_start(HUD_TARGET_MINI_ICON);
		Player->cargo_inspect_time=0;
		Player->locking_subsys=NULL;
		Player->locking_on_center=0;
		Player->locking_subsys_parent=-1;

		Player_ai->current_target_dist_trend = NO_CHANGE;
		Player_ai->current_target_speed_trend = NO_CHANGE;

		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
			Players[Player_num].flags &= ~PLAYER_FLAGS_MATCH_TARGET;
//			player_match_target_speed("", "", XSTR("Matching speed of newly acquired target",-1));
			player_match_target_speed();
		}
		else {
			if ( Players[Player_num].flags & PLAYER_FLAGS_MATCH_TARGET )
				Players[Player_num].flags &= ~PLAYER_FLAGS_MATCH_TARGET;		// no more target matching.
		}

		hud_lock_reset();

		if ( Player_ai->target_objnum != -1) {
			if ( Objects[Player_ai->target_objnum].type == OBJ_SHIP ) {
				hud_restore_subsystem_target(&Ships[Objects[Player_ai->target_objnum].instance]);
			}
		}
	}
	else {
		if (Player_ai->current_target_distance < Player_ai->last_dist-0.01){
			Player_ai->current_target_dist_trend = DECREASING;
		} else if (Player_ai->current_target_distance > Player_ai->last_dist+0.01){
			Player_ai->current_target_dist_trend = INCREASING;
		} else {
			Player_ai->current_target_dist_trend = NO_CHANGE;
		}

		current_speed = Objects[Player_ai->target_objnum].phys_info.speed;

		if (current_speed < Player_ai->last_speed-0.01){
			Player_ai->current_target_speed_trend = DECREASING;
		} else if (current_speed > Player_ai->last_speed+0.01) {
			Player_ai->current_target_speed_trend = INCREASING;
		} else {
			Player_ai->current_target_speed_trend = NO_CHANGE;
		}

		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
			if ( !(Players[Player_num].flags & PLAYER_FLAGS_MATCH_TARGET) ) {
//				player_match_target_speed("", "", XSTR("Matching target speed",-1));
				player_match_target_speed();
			}
		}
	}

	Player_ai->last_dist = Player_ai->current_target_distance;
	Player_ai->last_speed = current_speed;

	Player_ai->last_target = Player_ai->target_objnum;
	Player_ai->last_subsys_target = Player_ai->targeted_subsys;
}

HudGaugeTargetTriangle::HudGaugeTargetTriangle():
HudGaugeReticleTriangle(HUD_OBJECT_TARGET_TRI, HUD_TARGET_TRIANGLE)
{
}

void HudGaugeTargetTriangle::render(float frametime)
{
	if ( Player_ai->target_objnum == -1)
		return;

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);

	object *targetp = &Objects[Player_ai->target_objnum];

	// draw the targeting triangle that orbits the outside of the outer circle of the reticle
	if (!Player->target_is_dying && maybeFlashSexp() != 1) {

		hud_set_iff_color(targetp, 1);
		renderTriangle(&targetp->pos, 1, 0, 0);
	}

	if(!in_frame)
		g3_end_frame();
}

// start the weapon line (on the HUD) flashing
void hud_start_flash_weapon(int index)
{
	if ( index >= MAX_WEAPON_FLASH_LINES ) {
		Int3();	// Get Alan
		return;
	}

	if ( timestamp_elapsed(Weapon_flash_info.flash_duration[index]) ) {
		Weapon_flash_info.flash_next[index] = timestamp(TBOX_FLASH_INTERVAL);
		Weapon_flash_info.is_bright &= ~(1<<index);
	}

	Weapon_flash_info.flash_duration[index] = timestamp(TBOX_FLASH_DURATION);
}

// maybe change the text color for the weapon line indicated by index 
void hud_maybe_flash_weapon(int index)
{
	if ( index >= MAX_WEAPON_FLASH_LINES ) {
		Int3();	// Get Alan
		return;
	}

	// hud_set_default_color();
	hud_set_gauge_color(HUD_WEAPONS_GAUGE);
	if ( !timestamp_elapsed(Weapon_flash_info.flash_duration[index]) ) {
		if ( timestamp_elapsed(Weapon_flash_info.flash_next[index]) ) {
			Weapon_flash_info.flash_next[index] = timestamp(TBOX_FLASH_INTERVAL);
			Weapon_flash_info.is_bright ^= (1<<index);
		}

		if ( Weapon_flash_info.is_bright & (1<<index) ) {
			hud_set_gauge_color(HUD_WEAPONS_GAUGE, HUD_C_BRIGHT);
			// hud_set_bright_color();
		} else {
			hud_set_gauge_color(HUD_WEAPONS_GAUGE, HUD_C_DIM);
			// hud_set_dim_color();
		}
	}
}

/**
 * @brief Check if targeting is possible based on sensors strength
 */
int hud_sensors_ok(ship *sp, int show_msg)
{
	float	sensors_str;

	// If playing on lowest skill level, sensors don't affect targeting
	// If dead, still allow player to target, despite any subsystem damage
	// If i'm a multiplayer observer, allow me to target
	if ( (Game_skill_level == 0) || (Game_mode & GM_DEAD) || ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)) ) {
		return 1;
	}

	// if the ship is currently being affected by EMP
	if(emp_active_local()){
		return 0;
	}

	// ensure targeting functions are not disabled through damage
	sensors_str = ship_get_subsystem_strength( sp, SUBSYSTEM_SENSORS );
	if ( (sensors_str < MIN_SENSOR_STR_TO_TARGET) || (ship_subsys_disrupted(sp, SUBSYSTEM_SENSORS)) ) {
		if ( show_msg ) {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Targeting is disabled due to sensors damage", 330));
			snd_play(&Snds[SND_TARGET_FAIL]);
		}
		return 0;
	} else {
		return 1;
	}
}

extern bool Sexp_Messages_Scrambled;
int hud_communications_state(ship *sp)
{
	float str;

	// If playing on the lowest skill level, communications always ok
	// If dead, still allow player to communicate, despite any subsystem damage
	if ( Game_skill_level == 0 || (Game_mode & GM_DEAD) ) {
		return COMM_OK;
	}

	// Goober5000 - if the ship is the player, and he's dying, return OK (so laments can be played)
	if ((sp == Player_ship) && (sp->flags & SF_DYING))
		return COMM_OK;

	// Goober5000 - check for scrambled communications
	if ( Sexp_Messages_Scrambled || emp_active_local() )
		return COMM_SCRAMBLED;

	str = ship_get_subsystem_strength( sp, SUBSYSTEM_COMMUNICATION );
	
	if ( (str <= 0.01) || ship_subsys_disrupted(sp, SUBSYSTEM_COMMUNICATION) ) {
		return COMM_DESTROYED;
	} else if ( str < MIN_COMM_STR_TO_MESSAGE ) {
		return COMM_DAMAGED;
	}

	return COMM_OK;
}

// target the next or previous hostile/friendly ship
void hud_target_next_list(int hostile, int next_flag, int team_mask, int attacked_objnum, int play_fail_snd, int filter, int get_closest_turret_attacking_player)
{
	int		timestamp_val, valid_team_mask;

	if ( hostile ) {
		timestamp_val = Tl_hostile_reset_timestamp;
		Tl_hostile_reset_timestamp = timestamp(TL_RESET);
		if ( team_mask == -1 ) {
			valid_team_mask = iff_get_attackee_mask(Player_ship->team);
		} else {
			valid_team_mask = team_mask;
		}
	} else {
		// everyone hates a traitor including other traitors so the friendly target option shouldn't work for them
		if (Player_ship->team == Iff_traitor) {
			snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
			return;
		}

		timestamp_val = Tl_friendly_reset_timestamp;
		Tl_friendly_reset_timestamp = timestamp(TL_RESET);
		valid_team_mask = iff_get_mask(Player_ship->team);
	}

	// If no target is selected, then simply target the closest ship
	if ( Player_ai->target_objnum == -1 || timestamp_elapsed(timestamp_val) ) {
		hud_target_closest(valid_team_mask, attacked_objnum, play_fail_snd, filter, get_closest_turret_attacking_player);
		return;
	}

	object *nearest_object = select_next_target_by_distance((next_flag != 0), valid_team_mask, attacked_objnum);

	if (nearest_object != NULL) {
		// set new target
		set_target_objnum( Player_ai, OBJ_INDEX(nearest_object) );

		// maybe set new turret subsystem
		hud_maybe_set_sorted_turret_subsys(&Ships[nearest_object->instance]);
		hud_restore_subsystem_target(&Ships[nearest_object->instance]);
	}
	else {
		snd_play( &Snds[SND_TARGET_FAIL], 0.0f );
	}
}

HudGaugeAutoTarget::HudGaugeAutoTarget():
HudGauge(HUD_OBJECT_AUTO_TARGET, HUD_AUTO_TARGET, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeAutoTarget::initAutoTextOffsets(int x, int y)
{
	Auto_text_offsets[0] = x;
	Auto_text_offsets[1] = y;
}

void HudGaugeAutoTarget::initTargetTextOffsets(int x, int y)
{
	Target_text_offsets[0] = x;
	Target_text_offsets[1] = y;
}

void HudGaugeAutoTarget::initBitmaps(char *fname)
{
	Toggle_frame.first_frame = bm_load_animation(fname, &Toggle_frame.num_frames);
	if ( Toggle_frame.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeAutoTarget::initOnColor(int r, int g, int b, int a)
{
	if ( r == -1 || g == -1 || b == -1 || a == -1 ) {
		Use_on_color = false;
		gr_init_alphacolor(&On_color, 0, 0, 0, 0);
		return;
	}

	Use_on_color = true;
	gr_init_alphacolor(&On_color, r, g, b, a);
}

void HudGaugeAutoTarget::initOffColor(int r, int g, int b, int a)
{
	if ( r == -1 || g == -1 || b == -1 || a == -1 ) {
		Use_off_color = false;
		gr_init_alphacolor(&Off_color, 0, 0, 0, 0);
		return;
	}

	Use_off_color = true;
	gr_init_alphacolor(&Off_color, r, g, b, a);
}

void HudGaugeAutoTarget::render(float frametime)
{
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
		return;

	int frame_offset;

	if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
		frame_offset = 1;
	} else {
		frame_offset = 0;
	}

	// draw the box background
	setGaugeColor();
	renderBitmap(Toggle_frame.first_frame+frame_offset, position[0], position[1]);

	// draw the text on top
	if (frame_offset == 1) {
		//static color text_color;
		//gr_init_alphacolor(&text_color, 0, 0, 0, Toggle_text_alpha);
		if ( Use_on_color ) {
			gr_set_color_fast(&On_color);
		}
	} else if ( Use_off_color ) {
		gr_set_color_fast(&Off_color);
	}

	renderString(position[0] + Auto_text_offsets[0], position[1] + Auto_text_offsets[1], XSTR("auto", 1463));
	renderString(position[0] + Target_text_offsets[0], position[1] + Target_text_offsets[1], XSTR("target", 1465));
}

void HudGaugeAutoTarget::pageIn()
{
	bm_page_in_aabitmap(Toggle_frame.first_frame, Toggle_frame.num_frames);
}

HudGaugeAutoSpeed::HudGaugeAutoSpeed():
HudGauge(HUD_OBJECT_AUTO_SPEED, HUD_AUTO_SPEED, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeAutoSpeed::initAutoTextOffsets(int x, int y)
{
	Auto_text_offsets[0] = x;
	Auto_text_offsets[1] = y;
}

void HudGaugeAutoSpeed::initSpeedTextOffsets(int x, int y)
{
	Speed_text_offsets[0] = x;
	Speed_text_offsets[1] = y;
}

void HudGaugeAutoSpeed::initBitmaps(char *fname)
{
	Toggle_frame.first_frame = bm_load_animation(fname, &Toggle_frame.num_frames);
	if ( Toggle_frame.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeAutoSpeed::initOnColor(int r, int g, int b, int a)
{
	if ( r == -1 || g == -1 || b == -1 || a == -1 ) {
		Use_on_color = false;
		gr_init_alphacolor(&On_color, 0, 0, 0, 0);
		return;
	}

	Use_on_color = true;
	gr_init_alphacolor(&On_color, r, g, b, a);
}

void HudGaugeAutoSpeed::initOffColor(int r, int g, int b, int a)
{
	if ( r == -1 || g == -1 || b == -1 || a == -1 ) {
		Use_off_color = false;
		gr_init_alphacolor(&Off_color, 0, 0, 0, 0);
		return;
	}

	Use_off_color = true;
	gr_init_alphacolor(&Off_color, r, g, b, a);
}

void HudGaugeAutoSpeed::render(float frametime)
{
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
		return;

	int frame_offset;

	if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
		frame_offset = 3;
	} else {
		frame_offset = 2;
	}

	setGaugeColor();

	renderBitmap(Toggle_frame.first_frame+frame_offset, position[0], position[1]);	

	// draw the text on top
	if (frame_offset == 3) {
		//static color text_color;
		//gr_init_alphacolor(&text_color, 0, 0, 0, Toggle_text_alpha);
		if ( Use_on_color ) {
			gr_set_color_fast(&On_color);
		}
	} else if ( Use_off_color ) {
		gr_set_color_fast(&Off_color);
	}
	renderString(position[0] + Auto_text_offsets[0], position[1] + Auto_text_offsets[1], XSTR("auto", 1463));
	renderString(position[0] + Speed_text_offsets[0], position[1] + Speed_text_offsets[1], XSTR("speed", 1464));
}

void HudGaugeAutoSpeed::pageIn()
{
	bm_page_in_aabitmap(Toggle_frame.first_frame, Toggle_frame.num_frames);
}

// Set the player target to the closest friendly repair ship
// input:	goal_objnum	=>	Try to find repair ship where aip->goal_objnum matches this
// output:	1	=>	A repair ship was targeted
//				0	=>	No targeting change
int hud_target_closest_repair_ship(int goal_objnum)
{
	object	*A;
	object	*nearest_obj=&obj_used_list;
	ship		*shipp;
	ship_obj	*so;
	float		min_distance=1e20f;
	float		new_distance=0.0f;
	int		rval=0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		A = &Objects[so->objnum];
		shipp = &Ships[A->instance];	// get a pointer to the ship information

		// ignore all ships that aren't repair ships 
		if ( !(Ship_info[shipp->ship_info_index].flags & SIF_SUPPORT) ) {
			continue;
		}

		if ( (A == Player_obj) || (shipp->flags & TARGET_SHIP_IGNORE_FLAGS) )
			continue;

		// only consider friendly ships
		if ( !(Player_ship->team == shipp->team)) {
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		if ( goal_objnum >= 0 ) {
			if ( Ai_info[shipp->ai_index].goal_objnum != goal_objnum ) {
				continue;
			}
		}

		new_distance = hud_find_target_distance(A,Player_obj);
			
		if (new_distance <= min_distance) {
			min_distance=new_distance;
			nearest_obj = A;
		}
	}

	if (nearest_obj != &obj_used_list) {
		set_target_objnum( Player_ai, OBJ_INDEX(nearest_obj) );
		hud_restore_subsystem_target(&Ships[nearest_obj->instance]);
		rval=1;
	}
	else {
		// inform player how to get a support ship
		if ( goal_objnum == -1 ) {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No support ships in area.  Use messaging to call one in.", 332));
		}
		rval=0;
	}

	return rval;
}

void hud_target_toggle_hidden_from_sensors()
{
	if ( TARGET_SHIP_IGNORE_FLAGS & SF_HIDDEN_FROM_SENSORS ) {
		TARGET_SHIP_IGNORE_FLAGS &= ~SF_HIDDEN_FROM_SENSORS;
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("Target hiding from sensors disabled"));
	} else {
		TARGET_SHIP_IGNORE_FLAGS |= SF_HIDDEN_FROM_SENSORS;
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("Target hiding from sensors enabled"));
	}
}

// target the closest uninspected object
void hud_target_closest_uninspected_object()
{
	object	*A, *nearest_obj = NULL;
	ship		*shipp;
	ship_obj	*so;
	float		min_distance = 1e20f;
	float		new_distance = 0.0f;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {

		A = &Objects[so->objnum];
		shipp = &Ships[A->instance];	// get a pointer to the ship information

		if ( (A == Player_obj) || (shipp->flags & TARGET_SHIP_IGNORE_FLAGS) ){
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		// ignore all non-cargo carrying craft
		if ( !hud_target_ship_can_be_scanned(shipp) ) {
			continue;
		}

		new_distance = hud_find_target_distance(A,Player_obj);
			
		if (new_distance <= min_distance) {
			min_distance=new_distance;
			nearest_obj = A;
		}
	}

	if (nearest_obj != NULL) {
		set_target_objnum( Player_ai, OBJ_INDEX(nearest_obj) );
		hud_restore_subsystem_target(&Ships[nearest_obj->instance]);
	}
	else {
		snd_play( &Snds[SND_TARGET_FAIL] );
	}
}

// target the next or previous uninspected/unscanned object
void hud_target_uninspected_object(int next_flag)
{
	object	*A, *min_obj, *max_obj, *nearest_obj;
	ship		*shipp;
	ship_obj	*so;
	float		cur_dist, min_dist, max_dist, new_dist, nearest_dist, diff;	

	// If no target is selected, then simply target the closest uninspected cargo
	if ( Player_ai->target_objnum == -1 || timestamp_elapsed(Target_next_uninspected_object_timestamp) ) {
		Target_next_uninspected_object_timestamp = timestamp(TL_RESET);
		hud_target_closest_uninspected_object();
		return;
	}

	Target_next_uninspected_object_timestamp = timestamp(TL_RESET);

	cur_dist = hud_find_target_distance(&Objects[Player_ai->target_objnum], Player_obj);

	min_obj = max_obj = nearest_obj = NULL;
	min_dist = 1e20f;
	max_dist = 0.0f;
	if ( next_flag ) {
		nearest_dist = 1e20f;
	} else {
		nearest_dist = 0.0f;
	}

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		A = &Objects[so->objnum];
		shipp = &Ships[A->instance];	// get a pointer to the ship information

		if ( (A == Player_obj) || (shipp->flags & TARGET_SHIP_IGNORE_FLAGS) )
			continue;

		// ignore all non-cargo carrying craft
		if ( !hud_target_ship_can_be_scanned(shipp) ) {
			continue;
		}

		// don't use object if it is already a target
		if ( OBJ_INDEX(A) == Player_ai->target_objnum ) {
			continue;
		}

		if(hud_target_invalid_awacs(A)){
			continue;
		}

		new_dist = hud_find_target_distance(A, Player_obj);
			
		if (new_dist <= min_dist) {
			min_dist = new_dist;
			min_obj = A;
		}

		if (new_dist >= max_dist) {
			max_dist = new_dist;
			max_obj = A;
		}

		if ( next_flag ) {
			diff = new_dist - cur_dist;
			if ( diff > 0 ) {
				if ( diff < ( nearest_dist - cur_dist ) ) {
					nearest_dist = new_dist;
					nearest_obj = A;
				}
			}
		} else {
			diff = cur_dist - new_dist;
			if ( diff > 0 ) {
				if ( diff < ( cur_dist - nearest_dist ) ) {
					nearest_dist = new_dist;
					nearest_obj = A;
				}
			}
		}
	}

	if ( nearest_obj == NULL ) {

		if ( next_flag ) {
			if ( min_obj != NULL ) {
				nearest_obj = min_obj;
			}
		} else {
			if ( max_obj != NULL ) {
				nearest_obj = max_obj;
			}
		}
	}

	if (nearest_obj != NULL) {
		set_target_objnum( Player_ai, OBJ_INDEX(nearest_obj) );
		hud_restore_subsystem_target(&Ships[nearest_obj->instance]);
	}
	else {
		snd_play( &Snds[SND_TARGET_FAIL] );
	}
}

// ----------------------------------------------------------------
//
// Target Last Transmission Sender code START
//
// ----------------------------------------------------------------

typedef struct transmit_target
{
	int objnum;
	int objsig;
} transmit_target;

static int Transmit_target_next_slot = 0;
static int Transmit_target_current_slot = -1;
static int Transmit_target_reset_timer = timestamp(0);

#define MAX_TRANSMIT_TARGETS	10
static transmit_target Transmit_target_list[MAX_TRANSMIT_TARGETS];

// called once per level to initialize the target last transmission sender list
void hud_target_last_transmit_level_init()
{
	int i;

	for ( i = 0; i < MAX_TRANSMIT_TARGETS; i++ ) {
		Transmit_target_list[i].objnum = -1;
		Transmit_target_list[i].objsig = -1;
	}

	Transmit_target_next_slot = 0;
	Transmit_target_current_slot = 0;
	Transmit_target_reset_timer = timestamp(0);
}

// internal function only.. used to find index for last recorded ship transmission 
int hud_target_last_transmit_newest()
{
	int latest_slot;

	latest_slot = Transmit_target_next_slot - 1;
	if ( latest_slot < 0 ) {
		latest_slot = MAX_TRANSMIT_TARGETS - 1;
	}

	return latest_slot;
}

// called externally to set the player target to the last ship which sent a transmission to the player
void hud_target_last_transmit()
{
	int i;

	if ( Transmit_target_current_slot < 0 ) {
		Transmit_target_current_slot = hud_target_last_transmit_newest();
	}

	// If timed out, then simply target the last ship to transmit
	if ( timestamp_elapsed(Transmit_target_reset_timer) ) {
		Transmit_target_current_slot = hud_target_last_transmit_newest();
	}

	Transmit_target_reset_timer = timestamp(TL_RESET);

	int play_fail_sound = 1;
	int transmit_index = Transmit_target_current_slot;
	Assert(transmit_index >= 0);
	for ( i = 0; i < MAX_TRANSMIT_TARGETS; i++ ) {
		if ( Transmit_target_list[transmit_index].objnum >= 0 ) {
			int transmit_objnum = Transmit_target_list[transmit_index].objnum;
			
			if ( Player_ai->target_objnum == transmit_objnum ) {
				play_fail_sound = 0;
			} else {
				if ( Transmit_target_list[transmit_index].objsig == Objects[Transmit_target_list[transmit_index].objnum].signature ) {
					if ( !(Ships[Objects[transmit_objnum].instance].flags & TARGET_SHIP_IGNORE_FLAGS) ) {
						Transmit_target_current_slot = transmit_index-1;
						if ( Transmit_target_current_slot < 0 ) {
							Transmit_target_current_slot = MAX_TRANSMIT_TARGETS - 1;
						}
						break;
					}
				}
			}
		}

		transmit_index--;
		if ( transmit_index < 0 ) {
			transmit_index = MAX_TRANSMIT_TARGETS - 1;
		}
	}

	if ( i == MAX_TRANSMIT_TARGETS ) {
		if ( play_fail_sound ) {
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		Transmit_target_current_slot = -1;
		return;
	}

	if(hud_target_invalid_awacs(&Objects[Transmit_target_list[transmit_index].objnum])){
		return;
	}

	// target new ship!
	// Fix bug in targeting due to Alt-Y (target last ship sending transmission).
	// Was just bogus code in the call to hud_restore_subsystem_target(). -- MK, 9/15/99, 1:59 pm.
	int targeted_objnum;
	targeted_objnum = Transmit_target_list[transmit_index].objnum;
	Assert((targeted_objnum >= 0) && (targeted_objnum < MAX_OBJECTS));

	if ((targeted_objnum >= 0) && (targeted_objnum < MAX_OBJECTS)) {
		set_target_objnum( Player_ai, Transmit_target_list[transmit_index].objnum );
		hud_restore_subsystem_target(&Ships[Objects[Transmit_target_list[transmit_index].objnum].instance]);
	}
}

// called externally to add a message sender to the list
void hud_target_last_transmit_add(int ship_num)
{
	object	*ship_objp;
	int		ship_objnum;

	ship_objnum = Ships[ship_num].objnum;
	Assert(ship_objnum >= 0 && ship_objnum < MAX_OBJECTS);
	ship_objp = &Objects[ship_objnum];
	Assert(ship_objp->type == OBJ_SHIP);

	// don't add ourselves to the list
	if (Player_obj == ship_objp) {
		return;
	}

	Transmit_target_list[Transmit_target_next_slot].objnum = ship_objnum;
	Transmit_target_list[Transmit_target_next_slot].objsig = ship_objp->signature;
	Transmit_target_next_slot++;
	if ( Transmit_target_next_slot >= MAX_TRANSMIT_TARGETS ) {
		Transmit_target_next_slot = 0;
	}
}

// target a random ship (useful for EMP stuff)
void hud_target_random_ship()
{
	int shipnum;
	int objnum;

	shipnum = ship_get_random_targetable_ship();
	if((shipnum < 0) || (Ships[shipnum].objnum < 0)){
		return;
	}
	objnum = Ships[shipnum].objnum;

	if((objnum >= 0) && (Player_ai != NULL) && !hud_target_invalid_awacs(&Objects[objnum])){	
		// never target yourself
		if(objnum == OBJ_INDEX(Player_obj)){
			set_target_objnum(Player_ai, -1);
		} else {
			set_target_objnum(Player_ai, objnum);
		}
	}
}

// ----------------------------------------------------------------
//
// Target Last Transmission Sender code END
//
// ----------------------------------------------------------------

void hudtarget_page_in()
{
	int i;

	for ( i = 0; i < NUM_WEAPON_GAUGES; i++ ) {
		bm_page_in_aabitmap( Weapon_gauges[ballistic_hud_index][i].first_frame, Weapon_gauges[ballistic_hud_index][i].num_frames);
	}
	bm_page_in_aabitmap( New_weapon.first_frame, New_weapon.num_frames );

	weapon_info* wip;
	for(i = 0; i < Num_weapon_types; i++)
	{
		wip = &Weapon_info[i];
		if(strlen(wip->hud_filename))
		{
			wip->hud_image_index = bm_load(wip->hud_filename);
		}
	}
}

void hud_stuff_ship_name(char *ship_name_text, ship *shipp)
{
	// print ship name
	if ( ((Iff_info[shipp->team].flags & IFFF_WING_NAME_HIDDEN) && (shipp->wingnum != -1)) || (shipp->flags2 & SF2_HIDE_SHIP_NAME) ) {
		*ship_name_text = 0;
	} else {
		strcpy(ship_name_text, shipp->ship_name);

		// handle hash symbol
		end_string_at_first_hash_symbol(ship_name_text);

		// handle translation
		if (Lcl_gr) {
			lcl_translate_targetbox_name(ship_name_text);
		}
	}
}

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];
void hud_stuff_ship_callsign(char *ship_callsign_text, ship *shipp)
{
	// only fighters and bombers have callsigns
	if ( !(Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER)) ) {
		*ship_callsign_text = 0;
		return;
	}

	// handle multiplayer callsign
	if (Game_mode & GM_MULTIPLAYER) {
		// get a player num from the object, then get a callsign from the player structure.
		int pn = multi_find_player_by_object( &Objects[shipp->objnum] );

		if (pn >= 0) {
			strcpy(ship_callsign_text, Net_players[pn].m_player->short_callsign);
			return;
		}
	}

	// try to get callsign
	if (Fred_running) {
		strcpy(ship_callsign_text, Fred_callsigns[shipp-Ships]);
	} else {
		*ship_callsign_text = 0;
		if (shipp->callsign_index >= 0) {
			mission_parse_lookup_callsign_index(shipp->callsign_index, ship_callsign_text);
		}
	}

	// handle hash symbol
	end_string_at_first_hash_symbol(ship_callsign_text);

	// handle translation
	if (Lcl_gr) {
		lcl_translate_targetbox_name(ship_callsign_text);
	}
}

extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
void hud_stuff_ship_class(char *ship_class_text, ship *shipp)
{
	// try to get alt name
	if (Fred_running) {
		strcpy(ship_class_text, Fred_alt_names[shipp-Ships]);
	} else {
		*ship_class_text = 0;
		if (shipp->alt_type_index >= 0) {
			mission_parse_lookup_alt_index(shipp->alt_type_index, ship_class_text);
		}
	}

	// maybe get ship class
	if (!*ship_class_text) {
		strcpy(ship_class_text, (Ship_info[shipp->ship_info_index].alt_name[0]) ? Ship_info[shipp->ship_info_index].alt_name : Ship_info[shipp->ship_info_index].name);
	}

	// handle hash symbol
	end_string_at_first_hash_symbol(ship_class_text);

	// handle translation
	if (Lcl_gr) {
		lcl_translate_targetbox_name(ship_class_text);
	}
}

HudGaugeCmeasures::HudGaugeCmeasures():
HudGauge(HUD_OBJECT_CMEASURES, HUD_CMEASURE_GAUGE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{
}

void HudGaugeCmeasures::initCountTextOffsets(int x, int y)
{
	Cm_text_offsets[0] = x;
	Cm_text_offsets[1] = y;
}

void HudGaugeCmeasures::initCountValueOffsets(int x, int y)
{
	Cm_text_val_offsets[0] = x;
	Cm_text_val_offsets[1] = y;
}

void HudGaugeCmeasures::initBitmaps(char *fname)
{
	Cmeasure_gauge.first_frame = bm_load_animation(fname, &Cmeasure_gauge.num_frames);
	if ( Cmeasure_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeCmeasures::pageIn()
{
	bm_page_in_aabitmap(Cmeasure_gauge.first_frame, Cmeasure_gauge.num_frames);
}

void HudGaugeCmeasures::render(float frametime)
{
	if ( Cmeasure_gauge.first_frame == -1) {
		Int3();	// failed to load coutermeasure gauge background
		return;
	}

	ship_info *sip = &Ship_info[Player_ship->ship_info_index];
	if(sip->cmeasure_max < 0 || sip->cmeasure_type < 0){
		return;
	}

	// hud_set_default_color();
	setGaugeColor();

	// blit the background
	renderBitmap(Cmeasure_gauge.first_frame, position[0], position[1]);	

	// blit text
	renderString(position[0] + Cm_text_offsets[0], position[1] + Cm_text_offsets[1], XSTR( "cm.", 327));
	if ( !Player_ship ) {
		Int3();	// player ship doesn't exist?
		return;
	}
	renderPrintf(position[0] + Cm_text_val_offsets[0], position[1] + Cm_text_val_offsets[1], NOX("%02d"), Player_ship->cmeasure_count);
}

HudGaugeAfterburner::HudGaugeAfterburner():
HudGauge(HUD_OBJECT_AFTERBURNER, HUD_AFTERBURNER_ENERGY, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeAfterburner::initEnergyHeight(int h)
{
	Energy_h = h;
}

void HudGaugeAfterburner::initBitmaps(char *fname)
{
	Energy_bar.first_frame = bm_load_animation(fname, &Energy_bar.num_frames);

	if ( Energy_bar.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

//	Render the HUD afterburner energy gauge
void HudGaugeAfterburner::render(float frametime)
{
	float percent_left;
	int	clip_h,w,h;	

	if ( Energy_bar.first_frame == -1 ){
		return;
	}

	Assert(Player_ship);
	if ( !(Ship_info[Player_ship->ship_info_index].flags & SIF_AFTERBURNER) ) {
		// Goober5000 - instead of drawing an empty burner gauge, don't draw the gauge at all
		return;
	} else {
		percent_left = Player_ship->afterburner_fuel/Ship_info[Player_ship->ship_info_index].afterburner_fuel_capacity;
	}

	if ( percent_left > 1 ) {
		percent_left = 1.0f;
	}
	
	clip_h = fl2i( (1.0f - percent_left) * Energy_h + 0.5f );

	bm_get_info(Energy_bar.first_frame,&w,&h);

	setGaugeColor();
	
	if ( clip_h > 0) {
		renderBitmapEx(Energy_bar.first_frame, position[0], position[1],w,clip_h,0,0);		
	}

	if ( clip_h <= Energy_h ) {		
		renderBitmapEx(Energy_bar.first_frame+1, position[0], position[1] + clip_h,w,h-clip_h,0,clip_h);
	} 	
}

void HudGaugeAfterburner::pageIn()
{
	bm_page_in_aabitmap( Energy_bar.first_frame, Energy_bar.num_frames);
}

HudGaugeWeaponEnergy::HudGaugeWeaponEnergy():
HudGauge(HUD_OBJECT_WEAPON_ENERGY, HUD_WEAPONS_ENERGY, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeWeaponEnergy::initTextOffsets(int x, int y)
{
	Wenergy_text_offsets[0] = x;
	Wenergy_text_offsets[1] = y;
}

void HudGaugeWeaponEnergy::initEnergyHeight(int h)
{
	Wenergy_h = h;
}

void HudGaugeWeaponEnergy::initAlignments(int text_align, int armed_align)
{
	Text_alignment = text_align;
	Armed_alignment = armed_align;
}

void HudGaugeWeaponEnergy::initArmedOffsets(int x, int y, int h, bool show)
{
	Armed_name_offsets[0] = x;
	Armed_name_offsets[1] = y;
	Show_armed = show;
	Armed_name_h = h;
}

void HudGaugeWeaponEnergy::initAlwaysShowText(bool show_text)
{
	Always_show_text = show_text;
}

void HudGaugeWeaponEnergy::initMoveText(bool move_text)
{
	Moving_text = move_text;
}

void HudGaugeWeaponEnergy::initShowBallistics(bool show_ballistics)
{
	Show_ballistic = show_ballistics;
}

void HudGaugeWeaponEnergy::initBitmaps(char *fname)
{
	Energy_bar.first_frame = bm_load_animation(fname, &Energy_bar.num_frames);
	if ( Energy_bar.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeWeaponEnergy::pageIn()
{
	bm_page_in_aabitmap( Energy_bar.first_frame, Energy_bar.num_frames);
}

void HudGaugeWeaponEnergy::render(float frametime)
{
	int x;
	bool use_new_gauge = false;

	// Goober5000 - only check for the new gauge in case of command line + a ballistic-capable ship
	if (Cmdline_ballistic_gauge && Ship_info[Player_ship->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES)
	{
		for(x = 0; x < Player_ship->weapons.num_primary_banks; x++)
		{
			if(Weapon_info[Player_ship->weapons.primary_bank_weapons[x]].wi_flags2 & WIF2_BALLISTIC)
			{
				use_new_gauge = true;
				break;
			}
		}
	}

	if(use_new_gauge)
	{
		int currentx, currenty;
		int y;
		int max_w = 100;
		float remaining;
		currentx = position[0] + 10;
		currenty = position[1];
		if(gr_screen.max_w_unscaled == 640) {
			max_w = 60;
		}

		//*****ENERGY GAUGE
		if(Weapon_info[Player_ship->weapons.current_primary_bank].energy_consumed > 0.0f)
			setGaugeColor(HUD_C_BRIGHT);
		
		//Draw name
		renderString(currentx, currenty, "Energy");
		currenty += 10;

		//Draw background
		setGaugeColor(HUD_C_DIM);
		renderRect(currentx, currenty, max_w, 10);

		//Draw gauge bar
		setGaugeColor(HUD_C_NORMAL);
		remaining = max_w * ((float)Player_ship->weapon_energy/(float)Ship_info[Player_ship->ship_info_index].max_weapon_reserve);
		if(remaining > 0)
		{
			setGaugeColor(HUD_C_BRIGHT);
			for(y = 0; y < 10; y++) {
				renderGradientLine(currentx, currenty + y, currentx + fl2i(remaining), currenty + y);
			}
		}
		currenty += 12;

		char shortened_name[NAME_LENGTH];
		//*****BALLISTIC GAUGES
		for(x = 0; x < Player_ship->weapons.num_primary_banks; x++)
		{
			//Skip all pure-energy weapons
			if(!(Weapon_info[Player_ship->weapons.primary_bank_weapons[x]].wi_flags2 & WIF2_BALLISTIC))
				continue;

			//Draw the weapon bright or normal depending if it's active or not.
			if(x == Player_ship->weapons.current_primary_bank || (Player_ship->flags & SF_PRIMARY_LINKED)) {
				setGaugeColor(HUD_C_BRIGHT);
			} else {
				setGaugeColor(HUD_C_NORMAL);
			}
			if(gr_screen.max_w_unscaled == 640) {
				gr_force_fit_string(shortened_name, NAME_LENGTH, 55);
				renderString(currentx, currenty, shortened_name);
			} else {
				renderString(currentx, currenty, Weapon_info[Player_ship->weapons.primary_bank_weapons[x]].name);
			}

			//Next 'line'
			currenty += 10;

			//Draw the background for the gauge
			setGaugeColor(HUD_C_DIM);
			renderRect(currentx, currenty, max_w, 10);

			//Reset to normal brightness
			setGaugeColor(HUD_C_NORMAL);
			
			//Draw the bar graph
			remaining = (max_w - 4) * ((float) Player_ship->weapons.primary_bank_ammo[x] / (float) Player_ship->weapons.primary_bank_start_ammo[x]);
			if(remaining > 0) {
				renderRect(currentx + 2, currenty + 2, fl2i(remaining), 6);
			}
			//Increment for next 'line'
			currenty += 12;
		}
	}
	else
	{
		float percent_left;
		int ballistic_ammo = 0;
		int max_ballistic_ammo = 0;
		int	clip_h, w, h, i;
		weapon_info *wip;
		ship_weapon *sw;
		char buf[40] = "";

		if ( Energy_bar.first_frame == -1 ) {
			return;
		}

		if ( Player_ship->weapons.num_primary_banks <= 0 ) {
			return;
		}

		sw = &Player_ship->weapons;

		// show ballistic ammunition in energy gauge if need be
		if ( Show_ballistic && Ship_info[Player_ship->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES ) {
			if ( Player_ship->flags & SF_PRIMARY_LINKED ) {

				// go through all ballistic primaries and add up their ammunition totals and max capacities
				for ( i = 0; i < sw->num_primary_banks; i++ ) {

					// skip all pure-energy weapons
					if( ! ( Weapon_info[sw->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC ) ) {
						continue;
					}

					ballistic_ammo += sw->primary_bank_ammo[i]; 
					max_ballistic_ammo += sw->primary_bank_start_ammo[i];
				}
			} else {
				ballistic_ammo = sw->primary_bank_ammo[sw->current_primary_bank];
				max_ballistic_ammo = sw->primary_bank_start_ammo[sw->current_primary_bank];
			}

			percent_left = i2fl(ballistic_ammo) / i2fl(max_ballistic_ammo);
		} else {
			// also leave if no energy can be stored for weapons - Goober5000
			if (!ship_has_energy_weapons(Player_ship))
				return;

			percent_left = Player_ship->weapon_energy/Ship_info[Player_ship->ship_info_index].max_weapon_reserve;
			if ( percent_left > 1 )
			{
				percent_left = 1.0f;
			}
		}

		clip_h = fl2i( (1.0f - percent_left) * Wenergy_h + 0.5f );
		
		if ( percent_left <= 0.3 || Show_ballistic || Always_show_text ) {
			int delta_y = 0, delta_x = 0;

			if ( percent_left < 0.1 ) {
				gr_set_color_fast(&Color_bright_red);
			} else {
				setGaugeColor();
			}

			if ( Show_ballistic ) {
				sprintf(buf, "%d", ballistic_ammo);
			} else {
				sprintf(buf,XSTR( "%d%%", 326), fl2i(percent_left*100+0.5f));
			}

			if ( Moving_text ) {
				delta_y = clip_h;
			}

			hud_num_make_mono(buf);

			if ( Text_alignment ) {
				gr_get_string_size(&w, &h, buf);
				delta_x = -w;
			}

			renderString(position[0] + Wenergy_text_offsets[0] + delta_x, position[1] + Wenergy_text_offsets[1] + delta_y, buf);
		}

		setGaugeColor();

		// list currently armed primary banks if we have to
		if ( Show_armed ) {
			if ( Player_ship->flags & SF_PRIMARY_LINKED ) {
				// show all primary banks
				for ( i = 0; i < Player_ship->weapons.num_primary_banks; i++ ) {
					wip = &Weapon_info[sw->primary_bank_weapons[i]];
					strcpy_s(buf, (wip->alt_name[0]) ? wip->alt_name : wip->name);

					if ( Armed_alignment ) {
						gr_get_string_size(&w, &h, buf);
					} else {
						w = 0;
					}
					
					renderString(position[0] + Armed_name_offsets[0] - w, position[1] + Armed_name_offsets[1] + Armed_name_h * i, buf);
				}
			} else {
				// just show the current armed bank
				i = Player_ship->weapons.current_primary_bank;
				wip = &Weapon_info[sw->primary_bank_weapons[i]];
				strcpy_s(buf, (wip->alt_name[0]) ? wip->alt_name : wip->name);

				if ( Armed_alignment ) {
					gr_get_string_size(&w, &h, buf);
				} else {
					w = 0;
				}

				renderString(position[0] + Armed_name_offsets[0] - w, position[1] + Armed_name_offsets[1], buf);
			}
		}

		for ( i = 0; i < sw->num_primary_banks; i++ )
		{
			if ( !timestamp_elapsed(Weapon_flash_info.flash_duration[i]) )
			{
				if ( Weapon_flash_info.is_bright & (1<<i) )
				{
					// hud_set_bright_color();
					setGaugeColor(HUD_C_BRIGHT);
					break;
				}
			}
		}

		bm_get_info(Energy_bar.first_frame+2,&w,&h);
		
		if ( clip_h > 0 ) {
			renderBitmapEx(Energy_bar.first_frame+2, position[0], position[1], w,clip_h,0,0);
		}

		if ( clip_h <= Wenergy_h ) {
			renderBitmapEx(Energy_bar.first_frame+3, position[0], position[1] + clip_h, w,h-clip_h,0,clip_h);		
		}

		// hud_set_default_color();
	}
}

HudGaugeWeapons::HudGaugeWeapons():
HudGauge(HUD_OBJECT_WEAPONS, HUD_WEAPONS_GAUGE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{
}

void HudGaugeWeapons::initTopOffsetX(int x, int x_b)
{
	top_offset_x[0] = x;
	top_offset_x[1] = x_b;
}

void HudGaugeWeapons::initHeaderOffsets(int x, int y, int x_b, int y_b)
{
	Weapon_header_offsets[0][0] = x;
	Weapon_header_offsets[0][1] = y;
	Weapon_header_offsets[1][0] = x_b;
	Weapon_header_offsets[1][1] = y_b;
}

void HudGaugeWeapons::initFrameOffsetX(int x, int x_b)
{
	frame_offset_x[0] = x;
	frame_offset_x[1] = x_b;
}

void HudGaugeWeapons::initPrimaryWeaponOffsets(int link_x, int name_x, int ammo_x)
{
	Weapon_plink_offset_x = link_x;
	Weapon_pname_offset_x = name_x;
	Weapon_pammo_offset_x = ammo_x;
}

void HudGaugeWeapons::initSecondaryWeaponOffsets(int ammo_x, int name_x, int reload_x, int linked_x, int unlinked_x)
{
	Weapon_sammo_offset_x = ammo_x;
	Weapon_sname_offset_x = name_x;
	Weapon_sreload_offset_x = reload_x;
	Weapon_slinked_offset_x = linked_x;
	Weapon_sunlinked_offset_x = unlinked_x;
}

void HudGaugeWeapons::initStartNameOffsetsY(int p_y, int s_y)
{
	pname_start_offset_y = p_y;
	sname_start_offset_y = s_y;
}

void HudGaugeWeapons::initPrimaryHeights(int top_h, int text_h)
{
	top_primary_h = top_h;
	primary_text_h = text_h;
}

void HudGaugeWeapons::initSecondaryHeights(int top_h, int text_h)
{
	top_secondary_h = top_h;
	secondary_text_h = text_h;
}

void HudGaugeWeapons::initBitmapsPrimaryTop(char *fname, char *fname_ballistic)
{
	// load the graphics for the top portion of the weapons gauge
	primary_top[0].first_frame = bm_load_animation(fname, &primary_top[0].num_frames);
	if(primary_top[0].first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}

	primary_top[1].first_frame = bm_load_animation(fname_ballistic, &primary_top[1].num_frames);
	if(primary_top[1].first_frame < 0) {
		primary_top[1].first_frame = primary_top[0].first_frame;
		primary_top[1].num_frames = primary_top[0].num_frames;
	}
}

void HudGaugeWeapons::initBitmapsPrimaryMiddle(char *fname, char *fname_ballistic)
{
	// load the graphics for the middle portion of the primary weapons listing
	primary_middle[0].first_frame = bm_load_animation(fname, &primary_middle[0].num_frames);
	if(primary_middle[0].first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}

	primary_middle[1].first_frame = bm_load_animation(fname_ballistic, &primary_middle[1].num_frames);
	if(primary_middle[1].first_frame < 0) {
		primary_middle[1].first_frame = primary_middle[0].first_frame;
		primary_middle[1].num_frames = primary_middle[0].num_frames;
	}
}

void HudGaugeWeapons::initBitmapsPrimaryLast(char *fname, char *fname_ballistic)
{
	// load the graphics for the bottom portion of the primary weapons listing if there is one. 
	// Don't bother the user if there isn't one since retail doesn't use this.
	primary_last[0].first_frame = bm_load_animation(fname, &primary_last[0].num_frames);

	primary_last[1].first_frame = bm_load_animation(fname_ballistic, &primary_last[1].num_frames);
	if(primary_last[1].first_frame < 0) {
		primary_last[1].first_frame = primary_last[0].first_frame;
		primary_last[1].num_frames = primary_last[0].num_frames;
	}
}

void HudGaugeWeapons::initBitmapsSecondaryTop(char *fname, char *fname_ballistic)
{
	// top portion of the secondary weapons gauge
	secondary_top[0].first_frame = bm_load_animation(fname, &secondary_top[0].num_frames);
	if(secondary_top[0].first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}

	secondary_top[1].first_frame = bm_load_animation(fname_ballistic, &secondary_top[1].num_frames);
	if(secondary_top[1].first_frame < 0) {
		secondary_top[1].first_frame = secondary_top[0].first_frame;
		secondary_top[1].num_frames = secondary_top[0].num_frames;
	}
}

void HudGaugeWeapons::initBitmapsSecondaryMiddle(char *fname, char *fname_ballistic)
{
	// middle portion of the secondary weapons gauge
	secondary_middle[0].first_frame = bm_load_animation(fname, &secondary_middle[0].num_frames);
	if(secondary_middle[0].first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}

	secondary_middle[1].first_frame = bm_load_animation(fname_ballistic, &secondary_middle[1].num_frames);
	if(secondary_middle[1].first_frame < 0) {
		secondary_middle[1].first_frame = secondary_middle[0].first_frame;
		secondary_middle[1].num_frames = secondary_middle[0].num_frames;
	}
}

void HudGaugeWeapons::initBitmapsSecondaryBottom(char *fname, char *fname_ballistic)
{
	// bottom portion of the entire weapons gauge
	secondary_bottom[0].first_frame = bm_load_animation(fname, &secondary_bottom[0].num_frames);
	if(secondary_bottom[0].first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}

	secondary_bottom[1].first_frame = bm_load_animation(fname_ballistic, &secondary_bottom[1].num_frames);
	if(secondary_bottom[1].first_frame < 0) {
		secondary_bottom[1].first_frame = secondary_bottom[0].first_frame;
		secondary_bottom[1].num_frames = secondary_bottom[0].num_frames;
	}
}

void HudGaugeWeapons::pageIn()
{
	if(primary_top[0].first_frame > -1) {
		bm_page_in_aabitmap(primary_top[0].first_frame,primary_top[0].num_frames);
	}

	if(primary_top[1].first_frame > -1) {
		bm_page_in_aabitmap(primary_top[1].first_frame,primary_top[1].num_frames);
	}

	if(primary_middle[0].first_frame > -1) {
		bm_page_in_aabitmap(primary_middle[0].first_frame,primary_middle[0].num_frames);
	}

	if(primary_middle[1].first_frame > -1) {
		bm_page_in_aabitmap(primary_middle[1].first_frame, primary_middle[1].num_frames);
	}

	if(primary_last[1].first_frame > -1) {
		bm_page_in_aabitmap(primary_last[1].first_frame, primary_last[1].num_frames);
	}

	if(secondary_top[0].first_frame > -1) {
		bm_page_in_aabitmap(secondary_top[0].first_frame, secondary_top[0].num_frames);
	}

	if(secondary_top[1].first_frame > -1) {
		bm_page_in_aabitmap(secondary_top[1].first_frame, secondary_top[1].num_frames);
	}

	if(secondary_middle[0].first_frame > -1) {
		bm_page_in_aabitmap(secondary_middle[0].first_frame, secondary_middle[0].num_frames);
	}

	if(secondary_middle[1].first_frame >-1) {
		bm_page_in_aabitmap(secondary_middle[0].first_frame, secondary_middle[0].num_frames);
	}
	
	if(secondary_bottom[0].first_frame > -1) {
		bm_page_in_aabitmap(secondary_bottom[0].first_frame, secondary_bottom[0].num_frames);
	}

	if(secondary_bottom[1].first_frame > -1) {
		bm_page_in_aabitmap(secondary_bottom[0].first_frame, secondary_bottom[0].num_frames);
	}
}

void HudGaugeWeapons::render(float frametime)
{
	ship_weapon	*sw;
	int			np, ns;		// np == num primary, ns == num secondary
	char			name[NAME_LENGTH];	

	if(Player_obj->type == OBJ_OBSERVER)
		return;

	Assert(Player_obj->type == OBJ_SHIP);
	Assert(Player_obj->instance >= 0 && Player_obj->instance < MAX_SHIPS);

	sw = &Ships[Player_obj->instance].weapons;
	np = sw->num_primary_banks;
	ns = sw->num_secondary_banks;

	setGaugeColor();

	// draw top of primary display
	renderBitmap(primary_top[ballistic_hud_index].first_frame, position[0] + top_offset_x[ballistic_hud_index], position[1]);	
	
	// render the header of this gauge
	renderString(position[0] + Weapon_header_offsets[ballistic_hud_index][0], position[1] + Weapon_header_offsets[ballistic_hud_index][1], EG_WEAPON_TITLE, XSTR( "weapons", 328));		

	char	ammo_str[32];
	int		i, w, h;
	int y = position[1] + top_primary_h;
	int name_y = position[1] + pname_start_offset_y;
	
	// render primaries
	for(i = 0; i < np; i++) {
		setGaugeColor();

		// choose which background to draw for additional primaries. 
		// Note, we don't draw a background for the first primary. 
		// It is assumed that the top primary wep frame already has this rendered.
		if(i == 1) {
			// used to draw the second primary weapon background
			renderBitmap(primary_middle[ballistic_hud_index].first_frame, position[0] + frame_offset_x[ballistic_hud_index], y);
		} else if(i != 0) {
			// used to draw the the third, fourth, fifth, etc...
			if(primary_last[ballistic_hud_index].first_frame != -1)
				renderBitmap(primary_last[ballistic_hud_index].first_frame, position[0] + frame_offset_x[ballistic_hud_index], y);
		}

		strcpy_s(name, (Weapon_info[sw->primary_bank_weapons[i]].alt_name[0]) ? Weapon_info[sw->primary_bank_weapons[i]].alt_name : Weapon_info[sw->primary_bank_weapons[i]].name);
		if (Lcl_gr) {
			lcl_translate_wep_name(name);
		}
		
		// maybe modify name here to fit

		// do we need to flash the text?
		if (HudGauge::maybeFlashSexp() == i ) {
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			maybeFlashWeapon(i);
		}

		// indicate if this is linked or currently armed
		if ( ((sw->current_primary_bank == i) && !(Player_ship->flags & SF_PRIMARY_LINKED)) || ((Player_ship->flags & SF_PRIMARY_LINKED) && !(Weapon_info[sw->primary_bank_weapons[i]].wi_flags3 & WIF3_NOLINK))) {
			renderPrintf(position[0] + Weapon_plink_offset_x, name_y, EG_NULL, "%c", Lcl_special_chars + 2);
		}
		
		// either render this primary's image or its name
		if(Weapon_info[sw->primary_bank_weapons[0]].hud_image_index != -1) {
			renderBitmap(Weapon_info[sw->primary_bank_weapons[i]].hud_image_index, position[0] + Weapon_pname_offset_x, name_y);
		} else {
			renderPrintf(position[0] + Weapon_pname_offset_x, name_y, EG_WEAPON_P2, "%s", name);
		}

		// if this is a ballistic primary with ammo, render the ammo count
		if (Weapon_info[sw->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC) {
			// print out the ammo right justified
			sprintf(ammo_str, "%d", sw->primary_bank_ammo[i]);

			// get rid of #
			end_string_at_first_hash_symbol(ammo_str);

			hud_num_make_mono(ammo_str);
			gr_get_string_size(&w, &h, ammo_str);

			renderString(position[0] + Weapon_pammo_offset_x - w, name_y, EG_NULL, ammo_str);
		}

		if(i != 0) {
			y += primary_text_h;
		}
		name_y += primary_text_h;
	}

	weapon_info	*wip;
	char	weapon_name[NAME_LENGTH + 10];

	if ( HudGauge::maybeFlashSexp() == i ) {
		setGaugeColor(HUD_C_BRIGHT);
	}

	renderBitmap(secondary_top[ballistic_hud_index].first_frame, position[0] + frame_offset_x[ballistic_hud_index], y);
	name_y = y + sname_start_offset_y;
	y += top_secondary_h;

	for(i = 0; i < ns; i++)
	{
		setGaugeColor();
		wip = &Weapon_info[sw->secondary_bank_weapons[i]];

		if(i!=0) {
			renderBitmap(secondary_middle[ballistic_hud_index].first_frame, position[0] + frame_offset_x[ballistic_hud_index], y);
		}

		maybeFlashWeapon(np+i);
		
		// HACK - make Cluster Bomb fit on the HUD.
		if(!stricmp(wip->name,"cluster bomb")){
			strcpy_s(weapon_name, NOX("Cluster"));
		} else {
			strcpy_s(weapon_name, (wip->alt_name[0]) ? wip->alt_name : wip->name);
		}

		// get rid of #
		end_string_at_first_hash_symbol(weapon_name);
		
		if ( sw->current_secondary_bank == i ) {
			// show that this is the current secondary armed
			renderPrintf(position[0] + Weapon_sunlinked_offset_x, name_y, EG_NULL, "%c", Lcl_special_chars + 2);			

			// indicate if this is linked
			if ( Player_ship->flags & SF_SECONDARY_DUAL_FIRE ) {
				renderPrintf(position[0] + Weapon_slinked_offset_x, name_y, EG_NULL, "%c", Lcl_special_chars + 2);				
			}

			// show secondary weapon's image or print its name
			if(wip->hud_image_index != -1) {
				renderBitmap(wip->hud_image_index, position[0] + Weapon_sname_offset_x, name_y);
			} else {
				renderString(position[0] + Weapon_sname_offset_x, name_y, i ? EG_WEAPON_S1 : EG_WEAPON_S2, weapon_name);
			}

			// show the cooldown time
			if ( (sw->secondary_bank_ammo[i] > 0) && (sw->current_secondary_bank >= 0) ) {
				int ms_till_fire = timestamp_until(sw->next_secondary_fire_stamp[sw->current_secondary_bank]);
				if ( (ms_till_fire >= 500) && ((wip->fire_wait >= 1 ) || (ms_till_fire > wip->fire_wait*1000)) ) {
					renderPrintf(position[0] + Weapon_sreload_offset_x, name_y, EG_NULL, "%d", fl2i(ms_till_fire/1000.0f +0.5f));					
				}
			}
		} else {
			// just print the weapon's name since this isn't armed
			renderString(position[0] + Weapon_sname_offset_x, name_y, i ? EG_WEAPON_S1 : EG_WEAPON_S2, weapon_name);			
		}

		int ammo=sw->secondary_bank_ammo[i];
	
		// print out the ammo right justified
		sprintf(ammo_str, "%d", ammo);
		hud_num_make_mono(ammo_str);
		gr_get_string_size(&w, &h, ammo_str);

		renderString(position[0] + Weapon_sammo_offset_x - w, name_y, EG_NULL, ammo_str);		

		if(i != 0) {
			y += secondary_text_h;
		}
		name_y += secondary_text_h;
	}

	// a bit lonely here with no secondaries so just print "<none>"
	if(ns==0)
	{
		renderString(position[0] + Weapon_pname_offset_x, name_y, EG_WEAPON_S1, XSTR( "<none>", 329));	
	}

	y -= 0;
	// finish drawing the background
	renderBitmap(secondary_bottom[ballistic_hud_index].first_frame, position[0] + frame_offset_x[ballistic_hud_index], y);
}

void hud_update_weapon_flash()
{
	ship_weapon	*sw;
	int num_weapons;
	
	sw = &Ships[Player_obj->instance].weapons;
	num_weapons = sw->num_primary_banks + sw->num_secondary_banks;

	if ( num_weapons > MAX_WEAPON_FLASH_LINES ) {
		Int3();	// Get Alan
		return;
	}

	for(int i = 0; i < num_weapons; i++) {
		if ( !timestamp_elapsed(Weapon_flash_info.flash_duration[i]) ) {
			if ( timestamp_elapsed(Weapon_flash_info.flash_next[i]) ) {
				Weapon_flash_info.flash_next[i] = timestamp(TBOX_FLASH_INTERVAL);
				Weapon_flash_info.is_bright ^= (1<<i);
			}
		}
	}
}

void HudGaugeWeapons::maybeFlashWeapon(int index)
{
	if ( index >= MAX_WEAPON_FLASH_LINES ) {
		Int3();	// Get Alan
		return;
	}

	// hud_set_default_color();
	setGaugeColor();
	if ( !timestamp_elapsed(Weapon_flash_info.flash_duration[index]) ) {
		if ( Weapon_flash_info.is_bright & (1<<index) ) {
			setGaugeColor(HUD_C_BRIGHT);
			// hud_set_bright_color();
		} else {
			setGaugeColor(HUD_C_DIM);
			// hud_set_dim_color();
		}
	}
}

void hud_target_add_display_list(object *objp, vertex *target_point, vec3d *target_pos, int correction, color *bracket_clr, char* name, int flags)
{
	target_display_info element;

	element.objp = objp;
	element.target_point = *target_point;
	element.target_pos = *target_pos;
	element.correction = correction;
	element.flags = flags;

	if(bracket_clr) {
		element.bracket_clr = *bracket_clr;
	} else {
		// no color given, so this will tell the target display gauges to use IFF colors. 
		gr_init_color(&element.bracket_clr, 0, 0, 0);
	}

	if(name) {
		strcpy_s(element.name, name);
	} else {
		strcpy_s(element.name, "");
	}

	target_display_list.push_back(element);
}

void hud_target_clear_display_list()
{
	target_display_list.clear();
}

HudGaugeOffscreen::HudGaugeOffscreen():
HudGauge(HUD_OBJECT_OFFSCREEN, HUD_OFFSCREEN_INDICATOR, false, true, VM_DEAD_VIEW | VM_OTHER_SHIP, 255, 255, 255)
{
}

void HudGaugeOffscreen::initMaxTriSeperation(float length)
{
	Max_offscreen_tri_seperation = length;
}

void HudGaugeOffscreen::initMaxFrontSeperation(float length)
{
	Max_front_seperation = length;
}

void HudGaugeOffscreen::initTriBase(float length)
{
	Offscreen_tri_base = length;
}

void HudGaugeOffscreen::initTriHeight(float length)
{
	Offscreen_tri_height = length;
}

void HudGaugeOffscreen::pageIn()
{
}

void HudGaugeOffscreen::render(float frametime)
{
	// don't show offscreen indicator if we're warping out.
	if ( Player->control_mode != PCM_NORMAL ) {
		return;
	}

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);
	gr_set_screen_scale(base_w, base_h);

	for(size_t i = 0; i < target_display_list.size(); i++) {
		if(target_display_list[i].target_point.codes != 0) {
			float dist = 0.0f;

			if(target_display_list[i].objp) {
				dist = hud_find_target_distance( target_display_list[i].objp, Player_obj );
			} else { 
				// if we don't have a corresponding object, use given position to figure out distance
				dist = vm_vec_dist_quick(&Player_obj->pos, &target_display_list[i].target_pos);
			}

			if( target_display_list[i].bracket_clr.red && target_display_list[i].bracket_clr.green &&
				target_display_list[i].bracket_clr.blue ) {
				gr_set_color_fast(&target_display_list[i].bracket_clr);
			} else {
				// use IFF colors if none defined.
				if(target_display_list[i].objp) {
					hud_set_iff_color(target_display_list[i].objp, 1);
				} else {
					// no object so this must mean it's a nav point. but for some odd reason someone forgot to include a color. 
					gr_set_color_fast(&target_display_list[i].bracket_clr);
				}
			}

			renderOffscreenIndicator(&target_display_list[i].target_point, &target_display_list[i].target_pos, dist);
		}
	}

	gr_reset_screen_scale();
	if(!in_frame)
		g3_end_frame();
}

void HudGaugeOffscreen::renderOffscreenIndicator(vertex* target_point, vec3d *tpos, float distance, int draw_solid)
{
	char buf[32];
	int w = 0, h = 0;
	int on_top, on_right, on_left, on_bottom;

	float xpos,ypos;
	// points to draw triangles
	float x1=0.0f;
	float y1=0.0f;
	float x2=0.0f;
	float y2=0.0f;
	float x3=0.0f;
	float y3=0.0f;
	float x4=0.0f;
	float y4=0.0f;
	float x5=0.0f;
	float y5=0.0f;
	float x6=0.0f;
	float y6=0.0f;

	vec3d targ_to_player;
	float dist_behind;
	float triangle_sep;
	float half_gauge_length, half_triangle_sep;
	float displayed_distance;

	// scale by distance modifier from hud_guages.tbl for display purposes
	displayed_distance = distance * Hud_unit_multiplier;

	// calculate the dot product between the players forward vector and the vector connecting
	// the player to the target. Normalize targ_to_player since we want the dot product
	// to range between 0 -> 1.
	vm_vec_sub(&targ_to_player, &Player_obj->pos, tpos);
	vm_vec_normalize(&targ_to_player);
	dist_behind = vm_vec_dot(&Player_obj->orient.vec.fvec, &targ_to_player);

	if (dist_behind < 0) {	// still in front of player, but not in view
		dist_behind = dist_behind + 1.0f;
		if (dist_behind > 0.2 ){
			triangle_sep = ( dist_behind ) * Max_front_seperation;
		} else {
			triangle_sep = 0.0f;
		}
	} else {
		triangle_sep = dist_behind * Max_offscreen_tri_seperation + Max_offscreen_tri_seperation;
	}

	if ( triangle_sep > Max_offscreen_tri_seperation + Max_front_seperation){
		triangle_sep = Max_offscreen_tri_seperation + Max_front_seperation;
	}

	// calculate these values only once, since it will be used in several places
	half_triangle_sep = 0.5f * triangle_sep;
	half_gauge_length = half_triangle_sep + Offscreen_tri_base;

	// We need to find the screen (x,y) for where to draw the offscreen indicator
	//
	// The best way I've found is to draw a line from the eye_pos to the target, and
	// then use clip_line() to find the screen (x,y) for where the line hits the edge
	// of the screen.
	//
	// The weird thing about clip_line() is that is flips around the two verticies,
	// so I use eye_vertex->sx and eye_vertex->sy for the off-screen indicator (x,y)
	//
	vertex *eye_vertex = NULL;
	vertex real_eye_vertex;
	eye_vertex = &real_eye_vertex;	// this is needed since clip line takes a **vertex
	vec3d eye_pos;
	vm_vec_add( &eye_pos, &Eye_position, &View_matrix.vec.fvec);
	g3_rotate_vertex(eye_vertex, &eye_pos);

	ubyte codes_or;
	codes_or = (ubyte)(target_point->codes | eye_vertex->codes);
	clip_line(&target_point,&eye_vertex,codes_or,0);
	
	if (!(target_point->flags&PF_PROJECTED))
		g3_project_vertex(target_point);

	if (!(eye_vertex->flags&PF_PROJECTED))
		g3_project_vertex(eye_vertex);

	if (eye_vertex->flags&PF_OVERFLOW) {
		Int3();			//	This is unlikely to happen, but can if a clip goes through the player's eye.
		Player_ai->target_objnum = -1;
		return;
	} 
	
	if (target_point->flags & PF_TEMP_POINT)
		free_temp_point(target_point);

	if (eye_vertex->flags & PF_TEMP_POINT)
		free_temp_point(eye_vertex);

	xpos = eye_vertex->screen.xyw.x;
	ypos = eye_vertex->screen.xyw.y;

	// we need it unsized here and it will be fixed when things are acutally drawn
	gr_unsize_screen_posf(&xpos, &ypos);

	on_left = on_right = on_top = on_bottom = 0;
	xpos = (xpos<1) ? 0 : xpos;
	ypos = (ypos<1) ? 0 : ypos;

	if ( xpos <= gr_screen.clip_left_unscaled ) {
		xpos = i2fl(gr_screen.clip_left_unscaled);
		on_left = TRUE;

		if ( ypos < (half_gauge_length - gr_screen.clip_top_unscaled) )
			ypos = half_gauge_length;

		if ( ypos > (gr_screen.clip_bottom_unscaled - half_gauge_length) ) 
			ypos = gr_screen.clip_bottom_unscaled - half_gauge_length;

	} else if ( xpos >= gr_screen.clip_right_unscaled) {
		xpos = i2fl(gr_screen.clip_right_unscaled);
		on_right = TRUE;

		if ( ypos < (half_gauge_length - gr_screen.clip_top_unscaled) )
			ypos = half_gauge_length;

		if ( ypos > (gr_screen.clip_bottom_unscaled - half_gauge_length) ) 
			ypos = gr_screen.clip_bottom_unscaled - half_gauge_length;

	} else if ( ypos <= gr_screen.clip_top_unscaled ) {
		ypos = i2fl(gr_screen.clip_top_unscaled);
		on_top = TRUE;

		if ( xpos < ( half_gauge_length - gr_screen.clip_left_unscaled) )
			xpos = half_gauge_length;

		if ( xpos > (gr_screen.clip_right_unscaled - half_gauge_length) ) 
			xpos = gr_screen.clip_right_unscaled - half_gauge_length;

	} else if ( ypos >= gr_screen.clip_bottom_unscaled ) {
		ypos = i2fl(gr_screen.clip_bottom_unscaled);
		on_bottom = TRUE;

		if ( xpos < ( half_gauge_length - gr_screen.clip_left_unscaled) )
			xpos = half_gauge_length;

		if ( xpos > (gr_screen.clip_right_unscaled - half_gauge_length) ) 
			xpos = gr_screen.clip_right_unscaled - half_gauge_length;

	} else {
		Int3();
		return;
	}

	//	The offscreen target triangles are drawn according the the diagram below
	//
	//
	//
	//			  x3				x3
	//		   /	|				| \.
	//		 /		|				|   \.
	//		x1___x2				x2___x1
	//				|				|
	//		......|...........|...............(xpos,ypos)
	//				|				|
	//		x4___x5				x5___x4
	//		 \		|				|	  /
	//		   \ 	|				|	/
	//			  x6				x6
	//
	//

	xpos = (float)floor(xpos);
	ypos = (float)floor(ypos);

	if (displayed_distance > 0.0f) {
		sprintf(buf, "%d", fl2i(displayed_distance + 0.5f));
		hud_num_make_mono(buf);
		gr_get_string_size(&w, &h, buf);	
	} else {
		buf[0] = 0;
	}

	if (on_right) {
		x1 = x4 = (xpos+2);
			
		x2 = x3 = x5 = x6 = x1 - Offscreen_tri_height;
		y1 = y2 = ypos - half_triangle_sep;
		y3 = y2 - Offscreen_tri_base;

		y4 = y5 = ypos + half_triangle_sep;
		y6 = y5 + Offscreen_tri_base;

		if ( buf[0] ) {
			gr_string( fl2i(xpos - w - 10), fl2i(ypos - h/2.0f+0.5f), buf);
		}
	} else if (on_left) {
		x1 = x4 = (xpos-1);
			
		x2 = x3 = x5 = x6 = x1 + Offscreen_tri_height;
		y1 = y2 = ypos - half_triangle_sep;
		y3 = y2 - Offscreen_tri_base;

		y4 = y5 = ypos + half_triangle_sep;
		y6 = y5 + Offscreen_tri_base;

		if ( buf[0] ) {
			gr_string(fl2i(xpos + 10), fl2i(ypos - h/2.0f+0.5f), buf);
		}
	} else if (on_top) {
		y1 = y4 = (ypos-1);
			
		y2 = y3 = y5 = y6 = y1 + Offscreen_tri_height;
		x1 = x2 = xpos - half_triangle_sep;
		x3 = x2 - Offscreen_tri_base;

		x4 = x5 = xpos + half_triangle_sep;
		x6 = x5 + Offscreen_tri_base;

		if ( buf[0] ) {
			gr_string(fl2i(xpos - w/2.0f+0.5f), fl2i(ypos+10), buf);
		}
	} else if (on_bottom) {
		y1 = y4 = (ypos+2);
			
		y2 = y3 = y5 = y6 = y1 - Offscreen_tri_height;
		x1 = x2 = xpos - half_triangle_sep;
		x3 = x2 - Offscreen_tri_base;

		x4 = x5 = xpos + half_triangle_sep;
		x6 = x5 + Offscreen_tri_base;

		if ( buf[0] ) {
			gr_string(fl2i(xpos - w/2.0f+0.5f), fl2i(ypos-h-10), buf);
		}
	}

	if (draw_solid) {
		hud_tri(x3,y3,x2,y2,x1,y1);
		hud_tri(x4,y4,x5,y5,x6,y6);
	} else {
		hud_tri_empty(x3,y3,x2,y2,x1,y1);
		hud_tri_empty(x4,y4,x5,y5,x6,y6);
	}

	if (on_right || on_bottom){
		gr_line(fl2i(x2),fl2i(y2),fl2i(x5),fl2i(y5));
	} else if (on_left) {
		gr_line(fl2i(x2-1),fl2i(y2),fl2i(x5-1),fl2i(y5));
	} else {
		gr_line(fl2i(x2),fl2i(y2-1),fl2i(x5),fl2i(y5-1));
	}
}

HudGaugeWarheadCount::HudGaugeWarheadCount():
HudGauge(HUD_OBJECT_WARHEAD_COUNT, HUD_WEAPONS_GAUGE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY, 255, 255, 255)
{
}

void HudGaugeWarheadCount::initBitmap(char *fname)
{
	Warhead.first_frame = bm_load_animation(fname, &Warhead.num_frames);

	if ( Warhead.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeWarheadCount::initNameOffsets(int x, int y)
{
	Warhead_name_offsets[0] = x;
	Warhead_name_offsets[1] = y;
}

void HudGaugeWarheadCount::initCountOffsets(int x, int y)
{
	Warhead_count_offsets[0] = x;
	Warhead_count_offsets[1] = y;
}

void HudGaugeWarheadCount::initCountSizes(int w, int h)
{
	Warhead_count_size[0] = w;
	Warhead_count_size[1] = h;
}

void HudGaugeWarheadCount::initMaxSymbols(int count)
{
	Max_symbols = count;
}

void HudGaugeWarheadCount::initMaxColumns(int count)
{
	Max_columns = count;
}

void HudGaugeWarheadCount::initTextAlign(int align)
{
	Text_align = align;
}

void HudGaugeWarheadCount::pageIn()
{
	bm_page_in_aabitmap(Warhead.first_frame, Warhead.num_frames);
}

void HudGaugeWarheadCount::render(float frametime)
{
	if(Player_obj->type == OBJ_OBSERVER) {
		return;
	}

	Assert(Player_obj->type == OBJ_SHIP);
	Assert(Player_obj->instance >= 0 && Player_obj->instance < MAX_SHIPS);

	ship_weapon	*sw = &Ships[Player_obj->instance].weapons;

	// don't bother displaying anything if we have no secondaries
	if ( sw->num_secondary_banks <= 0 ) {
		return;
	}

	int wep_num = sw->current_secondary_bank;
	weapon_info *wip = &Weapon_info[sw->secondary_bank_weapons[wep_num]];
	int ammo = sw->secondary_bank_ammo[wep_num];

	// don't bother displaying anything if we have no ammo.
	if ( ammo <= 0 ) {
		return;
	}

	char weapon_name[NAME_LENGTH + 10];
	strcpy_s(weapon_name, (wip->alt_name[0]) ? wip->alt_name : wip->name);
	end_string_at_first_hash_symbol(weapon_name);

	setGaugeColor();

	// display the weapon name
	if ( Text_align ) {
		int w, h;

		gr_get_string_size(&w, &h, weapon_name);
		renderString(position[0] + Warhead_name_offsets[0] - w, position[1] + Warhead_name_offsets[1], weapon_name);
	} else {
		renderString(position[0] + Warhead_name_offsets[0], position[1] + Warhead_name_offsets[1], weapon_name);
	}

	setGaugeColor(HUD_C_BRIGHT);

	// if ammo is greater than the icon display limit, just show a numeric
	if ( ammo > Max_symbols ) {
		char ammo_str[32];

		sprintf(ammo_str, "%d", ammo);
		hud_num_make_mono(ammo_str);

		if ( Text_align ) {
			int w, h;

			gr_get_string_size(&w, &h, ammo_str);
			renderString(position[0] + Warhead_count_offsets[0] - w, position[1] + Warhead_count_offsets[1], ammo_str);
		} else {
			renderString(position[0] + Warhead_count_offsets[0], position[1] + Warhead_count_offsets[1], ammo_str);
		}

		return;
	}

	int delta_x = 0, delta_y = 0;
	if ( Text_align ) {
		delta_x = -Warhead_count_size[0];
	} else {
		delta_x = Warhead_count_size[0];
	}

	int i, column;
	for ( i = 0; i < ammo; i++ ) {
		if ( Max_columns > 0 ) {
			delta_y = Warhead_count_size[1] * (i / Max_columns);
			column = i % Max_columns;
		} else {
			column = i;
		}

		renderBitmap(Warhead.first_frame, position[0] + Warhead_count_offsets[0] + column * delta_x, position[1] + Warhead_count_offsets[1] + delta_y);
	}
}

HudGaugeWeaponList::HudGaugeWeaponList(int gauge_object):
HudGauge(gauge_object, HUD_WEAPONS_GAUGE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY, 255, 255, 255)
{

}

void HudGaugeWeaponList::initBitmaps(char *fname_first, char *fname_entry, char *fname_last)
{
	_background_first.first_frame = bm_load_animation(fname_first, &_background_first.num_frames);
	if(_background_first.first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_first);
	}

	_background_entry.first_frame = bm_load_animation(fname_entry, &_background_entry.num_frames);
	if(_background_entry.first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_entry);
	}

	_background_last.first_frame = bm_load_animation(fname_last, &_background_last.num_frames);
	if(_background_last.first_frame < 0) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_last);
	}
}

void HudGaugeWeaponList::initBgFirstOffsetX(int x)
{
	_bg_first_offset_x = x;
}

void HudGaugeWeaponList::initBgEntryOffsetX(int x)
{
	_bg_entry_offset_x = x;
}

void HudGaugeWeaponList::initBgLastOffsetX(int x)
{
	_bg_last_offset_x = x;
}

void HudGaugeWeaponList::initBgLastOffsetY(int y)
{
	_bg_last_offset_y = y;
}

void HudGaugeWeaponList::initBgFirstHeight(int h)
{
	_background_first_h = h;
}

void HudGaugeWeaponList::initBgEntryHeight(int h)
{
	_background_entry_h = h;
}

void HudGaugeWeaponList::initHeaderText(char *text)
{
	strcpy_s(header_text, text);
}

void HudGaugeWeaponList::initHeaderOffsets(int x, int y)
{
	_header_offsets[0] = x;
	_header_offsets[1] = y;
}

void HudGaugeWeaponList::initEntryStartY(int y)
{
	_entry_start_y = y;
}

void HudGaugeWeaponList::initEntryHeight(int h)
{
	_entry_h = h;
}

void HudGaugeWeaponList::pageIn()
{
	if ( _background_first.first_frame >= 0 ) {
		bm_page_in_aabitmap(_background_first.first_frame, _background_first.num_frames);
	}
	
	if ( _background_entry.first_frame >= 0 ) {
		bm_page_in_aabitmap(_background_entry.first_frame, _background_entry.num_frames);
	}

	if ( _background_last.first_frame >= 0 ) {
		bm_page_in_aabitmap(_background_last.first_frame, _background_last.num_frames);
	}
}

void HudGaugeWeaponList::maybeFlashWeapon(int index)
{
	if ( index >= MAX_WEAPON_FLASH_LINES ) {
		Int3();	// Get Alan
		return;
	}

	// hud_set_default_color();
	setGaugeColor();
	if ( !timestamp_elapsed(Weapon_flash_info.flash_duration[index]) ) {
		if ( Weapon_flash_info.is_bright & (1<<index) ) {
			setGaugeColor(HUD_C_BRIGHT);
			// hud_set_bright_color();
		} else {
			setGaugeColor(HUD_C_DIM);
			// hud_set_dim_color();
		}
	}
}

void HudGaugeWeaponList::render(float frametime)
{

}

HudGaugePrimaryWeapons::HudGaugePrimaryWeapons():
HudGaugeWeaponList(HUD_OBJECT_PRIMARY_WEAPONS)
{

}

void HudGaugePrimaryWeapons::initPrimaryLinkOffsetX(int x)
{
	_plink_offset_x = x;
}

void HudGaugePrimaryWeapons::initPrimaryNameOffsetX(int x)
{
	_pname_offset_x = x;
}

void HudGaugePrimaryWeapons::initPrimaryAmmoOffsetX(int x)
{
	_pammo_offset_x = x;
}

void HudGaugePrimaryWeapons::render(float frametime)
{
	ship_weapon	*sw;
	int ship_is_ballistic;

	int		num_primaries;		// np == num primary
	char	name[NAME_LENGTH];	

	if(Player_obj->type == OBJ_OBSERVER)
		return;

	Assert(Player_obj->type == OBJ_SHIP);
	Assert(Player_obj->instance >= 0 && Player_obj->instance < MAX_SHIPS);

	sw = &Ships[Player_obj->instance].weapons;
	ship_is_ballistic = (Ship_info[Ships[Player_obj->instance].ship_info_index].flags & SIF_BALLISTIC_PRIMARIES);

	num_primaries = sw->num_primary_banks;

	setGaugeColor();

	renderBitmap(_background_first.first_frame, position[0], position[1]);

	// render the header of this gauge
	renderString(position[0] + _header_offsets[0], position[1] + _header_offsets[1], EG_WEAPON_TITLE, header_text);

	char ammo_str[32];
	int i, w, h;
	int bg_y_offset = _background_first_h;
	int text_y_offset = _entry_start_y;

	for ( i = 0; i < num_primaries; ++i ) {
		setGaugeColor();

		renderBitmap(_background_entry.first_frame, position[0], position[1] + bg_y_offset);

		strcpy_s(name, (Weapon_info[sw->primary_bank_weapons[i]].alt_name[0]) ? Weapon_info[sw->primary_bank_weapons[i]].alt_name : Weapon_info[sw->primary_bank_weapons[i]].name);
		
		if (Lcl_gr) {
			lcl_translate_wep_name(name);
		}

		if (HudGauge::maybeFlashSexp() == i ) {
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			maybeFlashWeapon(i);
		}

		// indicate if this is linked or currently armed
		if ( (sw->current_primary_bank == i) || (Player_ship->flags & SF_PRIMARY_LINKED) ) {
			renderPrintf(position[0] + _plink_offset_x, position[1] + text_y_offset, EG_NULL, "%c", Lcl_special_chars + 2);
		}

		// either render this primary's image or its name
		if(Weapon_info[sw->primary_bank_weapons[0]].hud_image_index != -1) {
			renderBitmap(Weapon_info[sw->primary_bank_weapons[i]].hud_image_index, position[0] + _pname_offset_x, text_y_offset);
		} else {
			renderPrintf(position[0] + _pname_offset_x, position[1] + text_y_offset, EG_WEAPON_P2, "%s", name);
		}

		// if this is a ballistic primary with ammo, render the ammo count
		if (Weapon_info[sw->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC) {
			// print out the ammo right justified
			sprintf(ammo_str, "%d", sw->primary_bank_ammo[i]);

			// get rid of #
			end_string_at_first_hash_symbol(ammo_str);

			hud_num_make_mono(ammo_str);
			gr_get_string_size(&w, &h, ammo_str);

			renderString(position[0] + _pammo_offset_x - w, position[1] + text_y_offset, EG_NULL, ammo_str);
		}

		text_y_offset += _entry_h;
		bg_y_offset += _background_entry_h;
	}

	if ( num_primaries == 0 ) {
		renderBitmap(_background_entry.first_frame, position[0], position[1] + bg_y_offset);
		renderString(position[0] + _pname_offset_x, position[1] + text_y_offset, EG_WEAPON_P1, XSTR( "<none>", 329));

		bg_y_offset += _background_entry_h;
	}

	renderBitmap(_background_last.first_frame, position[0], position[1] + bg_y_offset + _bg_last_offset_y);
}

HudGaugeSecondaryWeapons::HudGaugeSecondaryWeapons():
HudGaugeWeaponList(HUD_OBJECT_SECONDARY_WEAPONS)
{

}

void HudGaugeSecondaryWeapons::initSecondaryAmmoOffsetX(int x)
{
	_sammo_offset_x = x;
}

void HudGaugeSecondaryWeapons::initSecondaryNameOffsetX(int x)
{
	_sname_offset_x = x;
}

void HudGaugeSecondaryWeapons::initSecondaryReloadOffsetX(int x)
{
	_sreload_offset_x = x;
}

void HudGaugeSecondaryWeapons::initSecondaryLinkedOffsetX(int x)
{
	_slinked_offset_x = x;
}

void HudGaugeSecondaryWeapons::initSecondaryUnlinkedOffsetX(int x)
{
	_sunlinked_offset_x = x;
}

void HudGaugeSecondaryWeapons::render(float frametime)
{
	ship_weapon	*sw;
	int ship_is_ballistic;

	int num_primaries, num_secondaries;

	Assert(Player_obj->type == OBJ_SHIP);
	Assert(Player_obj->instance >= 0 && Player_obj->instance < MAX_SHIPS);

	sw = &Ships[Player_obj->instance].weapons;
	ship_is_ballistic = (Ship_info[Ships[Player_obj->instance].ship_info_index].flags & SIF_BALLISTIC_PRIMARIES);

	num_primaries = sw->num_primary_banks;
	num_secondaries = sw->num_secondary_banks;

	setGaugeColor();

	renderBitmap(_background_first.first_frame, position[0], position[1]);

	// render the header of this gauge
	renderString(position[0] + _header_offsets[0], position[1] + _header_offsets[1], EG_WEAPON_TITLE, header_text);

	weapon_info	*wip;
	char weapon_name[NAME_LENGTH + 10];
	char ammo_str[32];
	int i, w, h;
	int bg_y_offset = _background_first_h;
	int text_y_offset = _entry_start_y;

	for ( i = 0; i < num_secondaries; ++i ) {
		setGaugeColor();
		wip = &Weapon_info[sw->secondary_bank_weapons[i]];

		renderBitmap(_background_entry.first_frame, position[0], position[1] + bg_y_offset);

		maybeFlashWeapon(num_primaries+i);

		strcpy_s(weapon_name, (wip->alt_name[0]) ? wip->alt_name : wip->name);
		end_string_at_first_hash_symbol(weapon_name);

		if ( sw->current_secondary_bank == i ) {
			// show that this is the current secondary armed
			renderPrintf(position[0] + _sunlinked_offset_x, position[1] + text_y_offset, EG_NULL, "%c", Lcl_special_chars + 2);

			// indicate if this is linked
			if ( Player_ship->flags & SF_SECONDARY_DUAL_FIRE ) {
				renderPrintf(position[0] + _slinked_offset_x, position[1] + text_y_offset, EG_NULL, "%c", Lcl_special_chars + 2);				
			}

			// show secondary weapon's image or print its name
			if(wip->hud_image_index != -1) {
				renderBitmap(wip->hud_image_index, position[0] + _sname_offset_x, position[1] + text_y_offset);
			} else {
				renderString(position[0] + _sname_offset_x, position[1] + text_y_offset, i ? EG_WEAPON_S1 : EG_WEAPON_S2, weapon_name);
			}

			// show the cooldown time
			if ( (sw->secondary_bank_ammo[i] > 0) && (sw->current_secondary_bank >= 0) ) {
				int ms_till_fire = timestamp_until(sw->next_secondary_fire_stamp[sw->current_secondary_bank]);
				if ( (ms_till_fire >= 500) && ((wip->fire_wait >= 1 ) || (ms_till_fire > wip->fire_wait*1000)) ) {
					renderPrintf(position[0] + _sreload_offset_x, position[1] + text_y_offset, EG_NULL, "%d", fl2i(ms_till_fire/1000.0f +0.5f));					
				}
			}
		} else {
			renderString(position[0] + _sname_offset_x, position[1] + text_y_offset, i ? EG_WEAPON_S1 : EG_WEAPON_S2, weapon_name);		
		}

		int ammo = sw->secondary_bank_ammo[i];

		// print out the ammo right justified
		sprintf(ammo_str, "%d", ammo);
		hud_num_make_mono(ammo_str);
		gr_get_string_size(&w, &h, ammo_str);

		renderString(position[0] + _sammo_offset_x - w, position[1] + text_y_offset, EG_NULL, ammo_str);

		bg_y_offset += _background_entry_h;
		text_y_offset += _entry_h;
	}

	if ( num_secondaries == 0 ) {
		renderBitmap(_background_entry.first_frame, position[0], position[1] + bg_y_offset);
		renderString(position[0] + _sname_offset_x, position[1] + text_y_offset, EG_WEAPON_S1, XSTR( "<none>", 329));

		bg_y_offset += _background_entry_h;
	}

	// finish drawing the background
	renderBitmap(_background_last.first_frame, position[0], position[1] + bg_y_offset + _bg_last_offset_y);
}

HudGaugeHardpoints::HudGaugeHardpoints():
HudGauge(HUD_OBJECT_HARDPOINTS, HUD_WEAPONS_GAUGE, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY, 255, 255, 255)
{

}

void HudGaugeHardpoints::initSizes(int w, int h)
{
	_size[0] = w;
	_size[1] = h;
}

void HudGaugeHardpoints::initLineWidth(float w)
{
	_line_width = w;
}

void HudGaugeHardpoints::initViewDir(int dir)
{
	_view_direction = dir;
}

void HudGaugeHardpoints::initDrawOptions(bool primary_models, bool secondary_models)
{
	draw_primary_models = primary_models;
	draw_secondary_models = secondary_models;
}

void HudGaugeHardpoints::render(float frametime)
{
	int			sx, sy;
	ship			*sp;
	ship_info	*sip;
	object *objp = Player_obj;

	sp = &Ships[objp->instance];
	sip = &Ship_info[sp->ship_info_index];

	sx = position[0];
	sy = position[1];

	bool g3_yourself = !g3_in_frame();
	angles top_view = {-PI_2,0.0f,0.0f};
	angles front_view = {PI_2*2.0f,PI_2*2.0f,0.0f};
	matrix	object_orient;

	switch ( _view_direction ) {
		case TOP:
			vm_angles_2_matrix(&object_orient, &top_view);
			break;
		case FRONT:
			vm_angles_2_matrix(&object_orient, &front_view);
			break;
	}

	gr_screen.clip_width = _size[0];
	gr_screen.clip_height = _size[1];

	//Fire it up
	if(g3_yourself)
		g3_start_frame(1);
	hud_save_restore_camera_data(1);
	setClip(sx, sy, _size[0], _size[1]);
	model_set_detail_level(1);

	g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom*1.5f);

	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	setGaugeColor();

	//We're ready to show stuff
	
	int cull = gr_set_cull(0);
	gr_stencil_clear();
	gr_stencil_set(GR_STENCIL_WRITE);
	int zbuffer = gr_zbuffer_set(GR_ZBUFF_NONE);
	gr_set_color_buffer(0);

	ship_model_start(objp);
	model_render( sip->model_num, &object_orient, &vmd_zero_vector, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_TEXTURING | MR_NO_CULL);

	gr_set_color_buffer(1);
	gr_stencil_set(GR_STENCIL_READ);
	gr_set_cull(cull);
	gr_set_line_width(_line_width*2.0f);

	model_set_alpha( gr_screen.current_color.alpha / 255.0f );
	model_set_forced_texture(0);

	model_render( 
		sip->model_num, 
		&object_orient, 
		&vmd_zero_vector, 
		MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_TEXTURING | MR_SHOW_OUTLINE_HTL | MR_NO_POLYS | MR_NO_ZBUFFER | MR_NO_CULL | MR_ALL_XPARENT
	);
	ship_model_stop( objp );

	gr_stencil_set(GR_STENCIL_NONE);
	gr_zbuffer_set(zbuffer);
	gr_set_line_width(1.0f);
	
	// draw weapon models
	int i, k;
	ship_weapon *swp = &sp->weapons;
	vertex draw_point;
	vec3d subobj_pos;
	g3_start_instance_matrix(&vmd_zero_vector, &object_orient, true);

	int render_flags = MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING | MR_NO_TEXTURING | MR_NO_ZBUFFER;

	setGaugeColor();
	model_set_alpha( gr_screen.current_color.alpha / 255.0f );

	//secondary weapons
	int num_secondaries_rendered = 0;
	vec3d secondary_weapon_pos;
	w_bank* bank;

	if ( draw_secondary_models ) {
		for (i = 0; i < swp->num_secondary_banks; i++) {
			if (Weapon_info[swp->secondary_bank_weapons[i]].external_model_num == -1 || !sip->draw_secondary_models[i])
				continue;

			bank = &(model_get(sip->model_num))->missile_banks[i];

			if (Weapon_info[swp->secondary_bank_weapons[i]].wi_flags2 & WIF2_EXTERNAL_WEAPON_LNCH) {
				for(k = 0; k < bank->num_slots; k++) {
					model_render(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k], render_flags);
				}
			} else {
				num_secondaries_rendered = 0;

				for(k = 0; k < bank->num_slots; k++)
				{
					secondary_weapon_pos = bank->pnt[k];

					if (num_secondaries_rendered >= sp->weapons.secondary_bank_ammo[i])
						break;

					if(sp->secondary_point_reload_pct[i][k] <= 0.0)
						continue;

					if ( swp->current_secondary_bank == i && ( swp->secondary_next_slot[i] == k || ( swp->secondary_next_slot[i]+1 == k && sp->flags & SF_SECONDARY_DUAL_FIRE ) ) ) {
						gr_set_color_fast(&Color_bright_blue);
					} else {
						gr_set_color_fast(&Color_bright_white);
						
					}

					num_secondaries_rendered++;

					vm_vec_scale_add2(&secondary_weapon_pos, &vmd_z_vector, -(1.0f-sp->secondary_point_reload_pct[i][k]) * model_get(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num)->rad);

					model_render(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &secondary_weapon_pos, render_flags);
				}
			}
		}
	}
	g3_done_instance(true);
	resetClip();
	model_set_forced_texture(0);

	setGaugeColor(HUD_C_BRIGHT);

	//primary weapons
	if ( draw_primary_models ) {
		for ( i = 0; i < swp->num_primary_banks; i++ ) {
			w_bank *bank = &model_get(sip->model_num)->gun_banks[i];

			for ( k = 0; k < bank->num_slots; k++ ) {	
				if ( ( Weapon_info[swp->primary_bank_weapons[i]].external_model_num == -1 || !sip->draw_primary_models[i] ) ) {
					vm_vec_unrotate(&subobj_pos, &bank->pnt[k], &object_orient);
					//vm_vec_sub(&subobj_pos, &Eye_position, &subobj_pos);
					//g3_rotate_vertex(&draw_point, &bank->pnt[k]);

					g3_rotate_vertex(&draw_point, &subobj_pos);
					g3_project_vertex(&draw_point);

					//resize(&width, &height);

					//unsize(&xc, &yc);
					//unsize(&draw_point.screen.xyw.x, &draw_point.screen.xyw.y);

					renderCircle((int)draw_point.screen.xyw.x + position[0], (int)draw_point.screen.xyw.y + position[1], 10);
					//renderCircle(xc, yc, 25);
				} else {
					polymodel* pm = model_get(Weapon_info[swp->primary_bank_weapons[i]].external_model_num);
					pm->gun_submodel_rotation = sp->primary_rotate_ang[i];
					model_render(Weapon_info[swp->primary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k], render_flags);
					pm->gun_submodel_rotation = 0.0f;
				}
			}
		}
	}
	
	//We're done
	if(!Cmdline_nohtl)
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}
	if(g3_yourself)
		g3_end_frame();

	hud_save_restore_camera_data(0);
}
