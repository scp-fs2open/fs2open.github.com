/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Playerman/Player.h $
 * $Revision: 2.6 $
 * $Date: 2005-04-05 05:53:23 $
 * $Author: taylor $
 *
 *  Header file for player information
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2004/08/11 05:06:32  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.4  2004/03/05 09:02:05  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/09/05 04:25:28  Goober5000
 * well, let's see here...
 *
 * * persistent variables
 * * rotating gun barrels
 * * positive/negative numbers fixed
 * * sexps to trigger whether the player is controlled by AI
 * * sexp for force a subspace jump
 *
 * I think that's it :)
 * --Goober5000
 *
 * Revision 2.2  2003/01/14 04:00:15  Goober5000
 * allowed for up to 256 main halls
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 18    9/12/99 1:27p Jefff
 * upped debug player_mission_failure_limit to 5
 * 
 * 17    9/07/99 10:50a Jefff
 * 
 * 16    9/06/99 9:43p Jefff
 * skip mission support
 * 
 * 15    8/29/99 4:18p Andsager
 * New "burst" limit for friendly damage.  Also credit more damage done
 * against large friendly ships.
 * 
 * 14    8/27/99 10:36a Dave
 * Impose a 2% penalty for hitting the shield balance key.
 * 
 * 13    8/02/99 9:13p Dave
 * Added popup tips.
 * 
 * 12    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 11    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 10    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 9     5/21/99 5:03p Andsager
 * Add code to display engine wash death.  Modify ship_kill_packet
 * 
 * 8     5/18/99 10:08a Andsager
 * Modified single maximum range before blown up to also be multi
 * friendly.
 * 
 * 7     3/28/99 12:37p Dave
 * Tentative beginnings to warpin effect.
 * 
 * 6     3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 5     1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 4     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 3     11/12/98 12:13a Dave
 * Tidied code up for multiplayer test. Put in network support for flak
 * guns.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 96    9/17/98 3:08p Dave
 * PXO to non-pxo game warning popup. Player icon stuff in create and join
 * game screens. Upped server count refresh time in PXO to 35 secs (from
 * 20).
 * 
 * 95    5/19/98 12:19p Mike
 * Cheat codes!
 * 
 * 94    5/09/98 4:52p Lawrance
 * Implement padlock view (up/rear/left/right)
 * 
 * 93    5/04/98 5:52p Comet
 * Fixed bug with Galatea/Bastion selection when finishing missions.
 * 
 * 92    4/25/98 3:49p Lawrance
 * Save briefing auto-advance pref
 * 
 * 91    4/21/98 11:55p Dave
 * Put in player deaths statskeeping. Use arrow keys in the ingame join
 * ship select screen. Don't quit the game if in the debriefing and server
 * leaves.
 * 
 * 90    4/20/98 6:07p Hoffoss
 * Added code to track if player is stationed on the galatea or the
 * bastion.
 * 
 * 89    4/08/98 10:34p Allender
 * make threat indicators work in multiplayer.  Fix socket problem (once
 * and for all???)
 * 
 * 88    4/05/98 7:43p Lawrance
 * fix up saving/restoring of link status and auto-target/match-speed.
 * 
 * 87    4/01/98 7:42p Lawrance
 * Enable auto-targeting by default when a new pilot is created.
 * 
 * 86    3/31/98 4:42p Allender
 * mission objective support for team v. team mode.  Chatbox changes to
 * make input box be correct length when typing
 * 
 * 85    3/30/98 6:27p Dave
 * Put in a more official set of multiplayer options, including a system
 * for distributing netplayer and netgame settings.
 * 
 * 84    3/27/98 3:59p Hoffoss
 * Due to everyone using different assumptions about what a length define
 * means, changed code to account for safest assumption.
 * 
 * 83    3/24/98 4:25p Lawrance
 * Make finding out if player killed self easier and more reliable
 * 
 * 82    3/19/98 5:35p Lawrance
 * Correctly inform player if killed by ship explosion.
 * 
 * 81    3/16/98 5:54p Lawrance
 * Play cargo scanning sound
 * 
 * 80    3/10/98 5:08p Allender
 * fixed up multiplayer death messages (I hope).  changes in object update
 * packets
 * 
 * 79    3/07/98 3:49p Lawrance
 * store killer species in player struct
 * 
 * 78    3/05/98 10:16p Lawrance
 * Add 'save_flags' to player struct to save/restore certain player flags
 * cleanly.
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
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

// number of times dude can fail a mission in a session before 
// having the opportunity to skip it
#ifdef RELEASE_REAL
	#define PLAYER_MISSION_FAILURE_LIMIT		5
#else
	#define PLAYER_MISSION_FAILURE_LIMIT		5
#endif  // RELEASE_REAL


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

	int				ask_help_count;							// number of praises received this mission
	int				allow_ask_help_timestamp;				// timestamp marking time until next 'ask help' is allowed

	int				scream_count;								// number of wingman screams received this mission
	int				allow_scream_timestamp;					// timestamp marking time until next wingman scream is allowed

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
	char				killer_parent_name[NAME_LENGTH];		// name of parent object that killed the player

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
} player;

extern player Players[MAX_PLAYERS];

extern int Player_num;								// player num of person playing on this machine
extern player *Player;								// pointer to my information
//extern control_info PlayerControls;

extern int Player_use_ai;

extern void player_init();							// initialization per level
extern void player_level_init();
extern void player_controls_init();				// initialize Descent style controls for use in various places
extern void player_match_target_speed(char *no_target_text=NULL, char *match_off_text=NULL, char *match_on_text=NULL);		// call to continually match speed with selected target
extern void player_clear_speed_matching();

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

char *player_generate_death_text( player *player_p, char *text );
void player_show_death_message();
void player_maybe_fire_turret(object *objp);
void player_maybe_play_all_alone_msg();
void player_set_next_all_alone_msg_timestamp();

void player_get_padlock_orient(matrix *eye_orient);
void player_display_packlock_view();

// get the player's eye position and orient
void player_get_eye(vec3d *eye_pos, matrix *eye_orient);

//=============================================================
//===================== PLAYER WARPOUT STUFF ==================
#define TARGET_WARPOUT_SPEED 40.0f		// speed you need to be going to warpout
#define TARGET_WARPOUT_MATCH_PERCENT 0.05f	// how close to TARGET_WARPOUT_SPEED you need to be
#define MINIMUM_PLAYER_WARPOUT_TIME	3.0f		// How long before you can press 'ESC' to abort warpout

extern float Warpout_time;							// Declared in Freespace.cpp
extern int Warpout_forced;							// If non-zero, bash the player to speed and go through effect
//=============================================================


#endif
