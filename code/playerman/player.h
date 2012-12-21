/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PLAYER_H
#define _PLAYER_H

#include "physics/physics.h"
#include "hud/hudtarget.h"				// for targeting hotkey lists
#include "stats/scoring.h"             // for scoring/stats
#include "io/keycontrol.h"				// for button_info
#include "network/multi_options.h"
#include "parse/sexp.h"
#include "globalincs/globals.h"

struct campaign_info;

#define MAX_KEYED_TARGETS			8		// number of hot keys available to assign targets to

// player image defines
#define PLAYER_PILOT_PIC_W					160
#define PLAYER_PILOT_PIC_H					120

#define PLAYER_SQUAD_PIC_W					128
#define PLAYER_SQUAD_PIC_H					128

// player flags follow
#define PLAYER_FLAGS_MATCH_TARGET			(1<<0)		// currently matching speed with selected target
#define PLAYER_FLAGS_MSG_MODE					(1<<1)		// is the player in messaging mode?
#define PLAYER_FLAGS_AUTO_TARGETING			(1<<2)		// is auto targeting on?
#define PLAYER_FLAGS_AUTO_MATCH_SPEED		(1<<3)		// is auto speed matching on?
#define PLAYER_FLAGS_STRUCTURE_IN_USE		(1<<4)		// is this structure in use -- for multiplayer games
#define PLAYER_FLAGS_PROMOTED					(1<<5)		// possibly set in mission to automatically give player promotion
#define PLAYER_FLAGS_IS_MULTI             (1<<6)      // this is a multiplayer pilot
#define PLAYER_FLAGS_DIST_WARNING			(1<<7)		// is this player under warning for being too far from battle
#define PLAYER_FLAGS_FORCE_MISSION_OVER	(1<<8)		// mission is being forced over for this player
#define PLAYER_FLAGS_LINK_PRIMARY			(1<<9)		// primary weapons were linked last mission
#define PLAYER_FLAGS_LINK_SECONDARY			(1<<10)		// secondary weapons were linked last mission
#define PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG	(1<<11)	//	player can't receive 'you're all alone...' message from Terran Command
#define PLAYER_FLAGS_KILLED_BY_EXPLOSION	(1<<12)		// player was killed by an instantaneous area-effect explosion
#define PLAYER_FLAGS_HAS_PLAYED_PXO			(1<<13)		// this pilot has at least played PXO once in the past.
#define PLAYER_FLAGS_DIST_TO_BE_KILLED		(1<<14)		// the pilot has been warned about distance and will be killed after message finishes playing
#define PLAYER_FLAGS_KILLED_BY_ENGINE_WASH	(1<<15)	// player was killed by engine wash
#define PLAYER_FLAGS_KILLED_SELF_UNKNOWN			(1<<16)		// player died by his own hand
#define PLAYER_FLAGS_KILLED_SELF_MISSILES			(1<<17)		// player died by his own missile
#define PLAYER_FLAGS_KILLED_SELF_SHOCKWAVE		(1<<18)		// player died by his own shockwave

#define PLAYER_KILLED_SELF						( PLAYER_FLAGS_KILLED_SELF_MISSILES | PLAYER_FLAGS_KILLED_SELF_SHOCKWAVE )

#define PCM_NORMAL				0	// normal flying mode
#define PCM_WARPOUT_STAGE1		1	// speed up to 40 km/s
#define PCM_WARPOUT_STAGE2		2	// flying towards and through warp hole
#define PCM_WARPOUT_STAGE3		3	// through warp hole, waiting for it to disapper.
#define PCM_SUPERNOVA			4	// supernova. lock everything to where it is.

// 'lua game control' settings
#define LGC_NORMAL				(1<<0)	// normal controls
#define LGC_STEERING			(1<<1)	// allow lua to fully override steering controls
#define LGC_FULL				(1<<2)	// allow lua to fully override controls

#define LGC_B_NORMAL			(1<<3)	// allow lua to keep recording button commands
#define LGC_B_OVERRIDE			(1<<4)	// allow lua to override current button commands
#define LGC_B_ADDITIVE			(1<<5)	// allow lua add current lua button commands to current commands

#define LGC_B_POLL_ALL			(1<<6)	// tell game code to pass all button commands to lua.

// number of times dude can fail a mission in a session before 
// having the opportunity to skip it
#define PLAYER_MISSION_FAILURE_LIMIT		5


typedef struct campaign_stats {
	char campaign_name[MAX_FILENAME_LEN+1];	// insurance
	scoring_struct stats;
} campaign_stats;

typedef struct player {

	char				callsign[CALLSIGN_LEN + 1];
	char				short_callsign[CALLSIGN_LEN + 1];	// callsign truncated to SHORT_CALLSIGN_PIXEL_W pixels
	int				short_callsign_width;					// useful for mutliplayer chat boxes.
	char				image_filename[MAX_FILENAME_LEN];	// filename of the image for this pilot
	char				squad_filename[MAX_FILENAME_LEN];	// filename of the squad image for this pilot
	char				squad_name[NAME_LENGTH + 1];			// pilot's squadron name
	int				num_campaigns;								// tells how many array entries in the campaigns field
	char				current_campaign[MAX_FILENAME_LEN]; // Name of the currently active campaign, or zero-length string if none
	campaign_info	*campaigns;									// holds information regarding all active campaigns the player is playing
	int				readyroom_listing_mode;

	ubyte			main_hall;							// Goober5000 - now allows 256 halls; I didn't make this int because it would mess up the file compatibility
	int				flags;
	int				save_flags;

	htarget_list	keyed_targets[MAX_KEYED_TARGETS];	// linked list of hot-keyed targets
	int				current_hotkey_set;						// currently hotkey set in use, -1 if none

	vec3d			lead_target_pos;							// (x,y,z) of the lead target indicator
	int				lead_target_cheat;						// whether cheat for firing at lead indicator is active
	int				lead_indicator_active;					// flag that indicates the lead indicator is enabled

	int				lock_indicator_x;							// 2D screen x-coordinate of the lock indicator
	int				lock_indicator_y;							// 2D screen y-coordinate of the lock indicator
	int				lock_indicator_start_x;					// 2D screen x-coordinate of where lock indicator originated
	int				lock_indicator_start_y;					// 2D screen y-coordinate of where lock indicator originated
	int				lock_indicator_visible;					// flag indicating if the lock indicator is on screen or not
	float				lock_time_to_target;						// time left (in milliseconds) before minimum time to lock elapsed
	float				lock_dist_to_target;						//	distance from lock indicator to target (in pixels)

	int				last_ship_flown_si_index;				// ship info index of ship most recently flown on a mission

	int				objnum;										// object number for this player
	button_info		bi;												// structure that holds bit vectors for button presses
	control_info	ci;											// control info structure for this player
	scoring_struct	stats;										// scoring and stats info for the player (points to multi_stats or single_stats)	
	
	int				friendly_hits;								//	Number of times hit a friendly ship this mission.
	float				friendly_damage;							//	Total friendly damage done in mission.  Diminishes over time.
	fix				friendly_last_hit_time;					//	Missiontime of last hit on friendly.  Used to decay friendly damage.
	fix				last_warning_message_time;				//	Time at which last message to player was sent regarding friendly damage.

	int				control_mode;								// Used to determine what mode player control is in.  For worm holes mainly.
	int				saved_viewer_mode;						// used to save viewer mode when warping out	

	int				check_warn_timestamp;					// Timestamp used to determine when to check for possible warning,
																		//	done so we don't check each frame

	int				distance_warning_count;					// Number of distance warings 

	int				distance_warning_time;					// Time at which distance warning was given
																
	int				allow_warn_timestamp;					// Timestamp used to regulate how often a player might receive
																		// warning messages about ships attacking.
	int				warn_count;									// number of attack warnings player has received this mission
	float				damage_this_burst;						// amount of damage done this frame to friendly craft
	int				repair_sound_loop;						// Sound id for ship repair looping sound, this is in the player 
																		// file since the repair sound only plays when Player ship is getting repaired
	
	int				cargo_scan_loop;							// Sound id for scanning cargo looping sound

	int				praise_count;								// number of praises received this mission
	int				allow_praise_timestamp;					// timestamp marking time until next praise is allowed
	int				praise_delay_timestamp;					// timestamp used to delay a praise by a second or two

	int				ask_help_count;							// number of times wingmen have asked for help this mission
	int				allow_ask_help_timestamp;				// timestamp marking time until next 'ask help' is allowed

	int				scream_count;								// number of wingman screams received this mission
	int				allow_scream_timestamp;					// timestamp marking time until next wingman scream is allowed

	int				low_ammo_complaint_count;							// number of complaints about low ammo received in this mission
	int				allow_ammo_timestamp;					// timestamp marking time until next 'low ammo' complaint is allowed

	int				praise_self_count;							// number of boasts about kills received in this mission
	int				praise_self_timestamp;					// timestamp marking time until next boast is allowed

	int				subsys_in_view;							// set to -1 when this information needs to be re-evaluated
	int				request_repair_timestamp;				// timestamp marking time until next time we can be informed of a repair ship getting called in

	int				cargo_inspect_time;						// time that current cargo has been inspected for
	int				target_is_dying;							// The player target is dying, set to -1 if no target
	int				current_target_sx;						// Screen x-pos of current target (or subsystem if applicable)
	int				current_target_sy;						// Screen y-pos of current target (or subsystem if applicable)
	int				target_in_lock_cone;						// Is the current target in secondary weapon lock cone?
	ship_subsys		*locking_subsys;							// Subsystem pointer that missile lock is trying to seek
	int				locking_subsys_parent;					// objnum of the parent of locking_subsystem
	int				locking_on_center;						// boolean, whether missile lock is trying for center of ship or not

	int				killer_objtype;							// type of object that killed player
	int				killer_species;							// Species which killed player
	int				killer_weapon_index;						// weapon used to kill player (if applicable)
	char			killer_parent_name[NAME_LENGTH];		// name of parent object that killed the player

	int				check_for_all_alone_msg;				// timestamp to check for playing of 'all alone' msg

	int				update_dumbfire_time;					// when to update dumbfire threat indicators
	int				update_lock_time;							// when to update lock threat indicators
	int				threat_flags;								// threat flags
	int				auto_advance;								// auto-advance through briefing?

	multi_local_options m_local_options;					// options for local player in multiplayer mode (ignore for single player pilots)
	multi_server_options m_server_options;					// options for netgame host/server in multiplayer mode

	int				insignia_texture;							// player's insignia bitmap (or -1 if none). should correspond to squad filename
																		// NOTE : this bitmap is in TEXTURE format. do not try to use this bitmap to 
																		//			 render in screen format
	int				tips;											// show tips or not

	int				shield_penalty_stamp;					// timestamp for when we can next apply a shield balance penalty

	int				failures_this_session;					// number of times dude has failed the mission he is on this session
	ubyte				show_skip_popup;							// false if dude clicked "don't show this again" -- persists for current mission only

	// player-persistent variables - Goober5000
	int				num_variables;
	sexp_variable	player_variables[MAX_SEXP_VARIABLES];

	SCP_string		death_message;								// Goober5000

	control_info	lua_ci;				// copy of control info for scripting purposes (not to disturb real controls).
	button_info		lua_bi;				// copy of button info for scripting purposes (not to disturb real controls).
	button_info		lua_bi_full;		// gets all the button controls, not just the ones usually allowed

	//CommanderDJ - constructor to initialise everything to safe defaults.
	//mostly copied from player::reset() in Antipodes 8
	player()
	{
		memset(callsign, 0, sizeof(callsign));
		memset(short_callsign, 0, sizeof(short_callsign));

		short_callsign_width = 0;
		memset(image_filename, 0, sizeof(image_filename));
		memset(squad_filename, 0, sizeof(squad_filename));
		memset(squad_name, 0, sizeof(squad_name));

		memset(current_campaign, 0, sizeof(current_campaign));

		main_hall = 0;

		readyroom_listing_mode = 0;
		flags = 0;
		save_flags = 0;

		memset(keyed_targets, 0, sizeof(keyed_targets));
		current_hotkey_set = -1;

		lead_target_pos = vmd_zero_vector;
		lead_target_cheat = 0;
		lead_indicator_active = 0;

		lock_indicator_x = 0;
		lock_indicator_y = 0;
		lock_indicator_start_x = 0;
		lock_indicator_start_y = 0;
		lock_indicator_visible = 0;
		lock_time_to_target = 0.0f;
		lock_dist_to_target = 0.0f;

		last_ship_flown_si_index = -1;

		objnum = -1;

		memset(&bi, 0, sizeof(button_info));
		memset(&ci, 0, sizeof(control_info));

		init_scoring_element(&stats);

		friendly_hits = 0;
		friendly_damage = 0.0f;
		friendly_last_hit_time = 0;
		last_warning_message_time = 0;

		control_mode = 0;
		saved_viewer_mode = 0;

		check_warn_timestamp = -1;

		distance_warning_count = 0;
		distance_warning_time = -1;

		allow_warn_timestamp = -1;
		warn_count = 0;

		damage_this_burst = 0.0f;

		repair_sound_loop = -1;
		cargo_scan_loop = -1;

		praise_count = 0;
		allow_praise_timestamp = -1;
		praise_delay_timestamp = -1;

		ask_help_count = 0;
		allow_ask_help_timestamp = -1;

		scream_count = 0;
		allow_scream_timestamp = -1;

		low_ammo_complaint_count = 0;
		allow_ammo_timestamp = -1;

		praise_self_count = 0;
		praise_self_timestamp = -1;

		subsys_in_view = -1;
		request_repair_timestamp = -1;

		cargo_inspect_time = -1;
		target_is_dying = -1;
		current_target_sx = 0;
		current_target_sy = 0;
		target_in_lock_cone = 0;
		locking_subsys = NULL;
		locking_subsys_parent = -1;
		locking_on_center = 0;

		killer_objtype = -1;
		killer_species = 0;
		killer_weapon_index = -1;
		memset(killer_parent_name, 0, sizeof(killer_parent_name));

		check_for_all_alone_msg = -1;

		update_dumbfire_time = -1;
		update_lock_time = -1;
		threat_flags = 0;
		auto_advance = 1;

		multi_options_set_local_defaults(&m_local_options);
		multi_options_set_netgame_defaults(&m_server_options);

		insignia_texture = -1;

		tips = 1;

		shield_penalty_stamp = 0;

		failures_this_session = 0;
		show_skip_popup = 0;

		num_variables = 0;
		memset(player_variables, 0, sizeof(player_variables));

		death_message = "";

		memset(&lua_ci, 0, sizeof(control_info));
		memset(&lua_bi, 0, sizeof(button_info));
		memset(&lua_bi_full, 0, sizeof(button_info));
	}

} player;

extern player Players[MAX_PLAYERS];

extern int Player_num;								// player num of person playing on this machine
extern player *Player;								// pointer to my information
//extern control_info PlayerControls;

extern int Player_use_ai;
extern int view_centering;
extern angles chase_slew_angles;					// The viewing angles in which viewer_slew_angles will chase to. 				

extern void player_init();							// initialization per level
extern void player_level_init();
extern void player_controls_init();				// initialize Descent style controls for use in various places
extern void player_match_target_speed(char *no_target_text=NULL, char *match_off_text=NULL, char *match_on_text=NULL);		// call to continually match speed with selected target
extern void player_clear_speed_matching();

extern int lua_game_control;					// defines the level of control set to lua scripting

void player_set_pilot_defaults(player *p);

int player_process_pending_praise();
float	player_farthest_weapon_range();
void player_save_target_and_weapon_link_prefs();
void player_restore_target_and_weapon_link_prefs();

// functions for controlling looping sounds associated with the player
void player_stop_looped_sounds();
void player_maybe_start_repair_sound();
void player_stop_repair_sound();
void player_stop_cargo_scan_sound();
void player_maybe_start_cargo_scan_sound();

// will attempt to load an insignia bitmap and set it as active for the player
void player_set_squad_bitmap(player *p, char *fname);

// set squadron
void player_set_squad(player *p, char *squad_name);

int player_inspect_cargo(float frametime, char *outstr);

//#ifndef NDEBUG
extern int use_descent;						// player is using descent-style physics
extern void toggle_player_object();		// toggles between descent-style ship and player ship
//#endif

extern void read_player_controls( object *obj, float frametime);
extern void player_control_reset_ci( control_info *ci );

void player_generate_death_message(player *player_p);
void player_show_death_message();
void player_maybe_fire_turret(object *objp);
void player_maybe_play_all_alone_msg();
void player_set_next_all_alone_msg_timestamp();

void player_get_padlock_orient(matrix *eye_orient);
void player_display_padlock_view();

// get the player's eye position and orient
camid player_get_cam();

//=============================================================
//===================== PLAYER WARPOUT STUFF ==================
#define PLAYER_WARPOUT_SPEED 40.0f		// speed you need to be going to warpout
#define TARGET_WARPOUT_MATCH_PERCENT 0.05f	// how close to TARGET_WARPOUT_SPEED you need to be
#define MINIMUM_PLAYER_WARPOUT_TIME	3.0f		// How long before you can press 'ESC' to abort warpout

extern float Warpout_time;							// Declared in freespace.cpp
extern int Warpout_forced;							// If non-zero, bash the player to speed and go through effect
//=============================================================


#endif
