#ifndef MISSION_FLAGS_H
#define MISSION_FLAGS_H

#include "globalincs/flagset.h"

namespace Mission {

	FLAG_LIST(Mission_Flags) {
		Subspace,					// mission takes place in subspace
		No_promotion,				// cannot get promoted or badges in this mission
		Fullneb,					// mission is a full nebula mission
		No_builtin_msgs,			// disables builtin msgs
		No_traitor,					// player cannot become a traitor
		Toggle_ship_trails,			// toggles ship trails (off in nebula, on outside nebula)
		Support_repairs_hull,		// Toggles support ship repair of ship hulls
		Beam_free_all_by_default,	// Beam-free-all by default - Goober5000
		Unused_1,					// Necessary to not break parsing
		Unused_2,					// Necessary to not break parsing
		No_briefing,				// no briefing, jump right into mission - Goober5000
		Toggle_debriefing,			// Turn on debriefing for dogfight. Off for everything else - Goober5000
		Unused_3,					// Necessary to not break parsing
		Allow_dock_trees,			// toggle between hub and tree model for ship docking (see objectdock.cpp) - Gooober5000
		Mission_2d,					// Mission is meant to be played top-down style; 2D physics and movement.
		Unused_4,					// Necessary to not break parsing
		Red_alert,					// a red-alert mission - Goober5000
		Scramble,					// a scramble mission - Goober5000
		No_builtin_command,			// turns off Command without turning off pilots - Karajorma
		Player_start_ai,			// Player starts mission under AI Control (NOT MULTI COMPATABLE) - Kazan
		All_attack,					// all teams at war - Goober5000
		Use_ap_cinematics,			// use autopilot cinematics - Kazan
		Deactivate_ap,				// deactivate autopilot - KeldorKatarn (patch approved by Kazan)
		Always_show_goals,			// Show the mission goals, even for training missions - Karajorma
		End_to_mainhall,			// Return to the mainhall after debrief - niffiwan
		Override_hashcommand,		// Override #Command with the Command info in Mission Specs - Goober5000
		Player_start_chase_view,	// Player starts mission in chase view - Goober5000
		Neb2_fog_color_override,	// Whether to use explicit fog colors instead of checking the palette - Goober5000
		
		NUM_VALUES
	};

	FLAG_LIST(Parse_Object_Flags) {
		SF_Cargo_known,
		SF_Ignore_count,
		SF_Reinforcement,
		SF_Escort,
		SF_No_arrival_music,
		SF_No_arrival_warp,
		OF_No_shields,
		OF_Protected,
		OF_Player_start,
		SF_No_departure_warp,
		SF_Locked,
		OF_Invulnerable,
		SF_Hidden_from_sensors,
		SF_Scannable,	// ship is a "scannable" ship
		AIF_Kamikaze,
		AIF_No_dynamic,
		SF_Red_alert_store_status,
		OF_Beam_protected,
		OF_Flak_protected,
		OF_Laser_protected,
		OF_Missile_protected,
		SF_Guardian,
		Knossos_warp_in,
		SF_Vaporize,
		SF_Stealth,
		SF_Friendly_stealth_invis,
		SF_Dont_collide_invis,
		SF_Use_unique_orders,		// tells a newly created ship to use the default orders for that ship
		SF_Dock_leader,				// Goober5000 - a docked parse object that is the leader of its group
		SF_Cannot_arrive,			// used to indicate that this ship's arrival cue will never be true
		SF_Warp_broken,				// warp engine should be broken for this ship
		SF_Warp_never,				// warp drive is destroyed
		SF_Primitive_sensors,
		SF_No_subspace_drive,
		SF_Nav_carry_status,
		SF_Affected_by_gravity,
		SF_Toggle_subsystem_scanning,
		OF_Targetable_as_bomb,
		SF_No_builtin_messages,
		SF_Primaries_locked,
		SF_Secondaries_locked,
		SF_No_death_scream,
		SF_Always_death_scream,
		SF_Nav_needslink,
		SF_Hide_ship_name,
		SF_Set_class_dynamically,
		SF_Lock_all_turrets_initially,		
		SF_Afterburner_locked,	
		OF_Force_shields_on,
		OF_Immobile,
		SF_No_ets,
		SF_Cloaked,
		SF_Ship_locked,
		SF_Weapons_locked,
		SF_Scramble_messages,
		Red_alert_deleted,	// Goober5000 - used analogously to SEF_PLAYER_DELETED
		Already_handled,	// Goober5000 - used for docking currently, but could be used generically
		OF_No_collide,
        SF_No_disabled_self_destruct,
		SF_Has_display_name,
		SF_Hide_mission_log,
		SF_Same_arrival_warp_when_docked,
		SF_Same_departure_warp_when_docked,

		NUM_VALUES
	};
}

#endif