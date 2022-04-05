#ifndef SHIP_FLAGS_H
#define SHIP_FLAGS_H

#include "globalincs/flagset.h"

namespace Ship {

	FLAG_LIST(Weapon_Flags){
		Beam_Free = 0,	// if this is a beam weapon, its free to fire
		Turret_Lock,	// is this turret is free to fire or locked
		Tagged_Only,	// only fire if target is tagged
		Trigger_Lock,	// // indicates that the trigger is held down

		NUM_VALUES
	};

	FLAG_LIST(Subsystem_Flags) {
		Cargo_revealed = 0,
		Untargetable,
		No_SS_targeting,
		Has_fired,					// used by scripting to flag a turret as having been fired
		FOV_Required,
		FOV_edge_check,
		No_replace,					// prevents 'destroyed' submodel from being rendered if subsys is destroyed.
		No_live_debris,				// prevents subsystem from generating live debris
		Vanished,					// allows subsystem to be made to disappear without a trace (for swapping it for a true model for example.
		Missiles_ignore_if_dead,	// forces homing missiles to target hull if subsystem is dead before missile hits it.
		Rotates,
		Damage_as_hull,				// Applies armor damage instead of subsystem damge. - FUBAR
		No_aggregate,				// exclude this subsystem from the aggregate subsystem-info tracking - Goober5000
		Play_sound_for_player,		// If this subsystem is a turret on a player ship, play firing sounds - The E 
		No_disappear,				// prevents submodel from disappearing when subsys destroyed
        Autorepair_if_disabled,     // Allows the subsystem to repair itself even when disabled - MageKing17
        No_autorepair_if_disabled,  // Inversion of the above; allow a specific subsystem not to repair itself after being disabled if the ship has the "repair disabled subsystems" flag - MageKing17
		Forced_target,				// The turrets current target is being forced by SEXP, and won't let it go until it dies or is cleared by SEXP
		Forced_subsys_target,		// The turrets current subsystem target is being forced by SEXP, implies Forced_target

		NUM_VALUES
	};
	
	FLAG_LIST(System_Flags) {
		Alive = 0,				// subsystem has active alive sound
		Dead,					// subsystem has active dead sound
		Rotate,					// subsystem has active rotation sound
		Turret_rotation,		// rotation sound to be scaled like turrets do

		NUM_VALUES
	};

	FLAG_LIST(Ship_Flags) {
		Ignore_count = 0,			// ignore this ship when counting ship types for goals
		Reinforcement,				// this ship is a reinforcement ship
		Escort,						// this ship is an escort ship
		No_arrival_music,			// don't play arrival music when ship arrives
		No_arrival_warp,			// no arrival warp in effect
		No_departure_warp,			// no departure warp in effect
		Kill_before_mission,
		Dying,
		Disabled,
		Depart_warp,				// ship is departing via warp-out
		Depart_dockbay,				// ship is departing via docking bay
		Arriving_stage_1,			// ship is arriving. In other words, doing warp in effect, stage 1
		Arriving_stage_1_dock_follower,		// "Arriving but Not the Dock Leader"; these guys need some warp stuff done but not all
		Arriving_stage_2,			// ship is arriving. In other words, doing warp in effect, stage 2             
		Arriving_stage_2_dock_follower,		// "Arriving but Not the Dock Leader"; these guys need some warp stuff done but not all
		Engines_on,					// engines sound should play if set
		Dock_leader,				// Goober5000 - this guy is in charge of everybody he's docked to
		Cargo_revealed,				// ship's cargo is revealed to all friendly ships
		From_player_wing,			// set for ships that are members of any player starting wing
		Primary_linked,				// ships primary weapons are linked together
		Secondary_dual_fire,		// ship is firing two missiles from the current secondary bank
		Warp_broken,				// set when warp drive is not working, but is repairable
		Warp_never,					// set when ship can never warp
		Trigger_down,				// ship has its "trigger" held down
		Ammo_count_recorded,		// we've recorded the initial secondary weapon count (which is used to limit support ship rearming)
		Hidden_from_sensors,		// ship doesn't show up on sensors, blinks in/out on radar
		Scannable,					// ship is "scannable".  Play scan effect and report as "Scanned" or "not scanned".
		Warped_support,				// set when this is a support ship which was warped in automatically
		Exploded,					// ship has exploded (needed for kill messages)
		Ship_has_screamed,			// ship has let out a death scream
		Red_alert_store_status,		// ship status should be stored/restored if red alert mission
		Vaporize,					// ship is vaporized by beam - alternative death sequence
		Departure_ordered,			// departure of this ship was ordered by player - Goober5000, similar to WF_DEPARTURE_ORDERED
		Primitive_sensors,			// Goober5000 - primitive sensor display
		Friendly_stealth_invis,		// Goober5000 - when stealth, don't appear on radar even if friendly
		Stealth,					// Goober5000 - is this particular ship stealth
		Dont_collide_invis,			// Goober5000 - is this particular ship don't-collide-invisible
		No_subspace_drive,			// Goober5000 - this ship has no subspace drive
		Navpoint_carry,				// Kazan      - This ship autopilots with the player
		Affected_by_gravity,		// Goober5000 - ship affected by gravity points
		Toggle_subsystem_scanning,	// Goober5000 - switch whether subsystems are scanned
		No_builtin_messages,		// Karajorma - ship should not send built-in messages
		Primaries_locked,			// Karajorma - This ship can't fire primary weapons
		Secondaries_locked,			// Karajorma - This ship can't fire secondary weapons
		Glowmaps_disabled,			// taylor - to disable glow maps
		No_death_scream,			// Goober5000 - for WCS
		Always_death_scream,		// Goober5000 - for WCS
		Navpoint_needslink,			// Kazan	- This ship requires "linking" for autopilot (when player ship gets within specified distance NAVPOINT_NEEDSLINK is replaced by NAVPOINT_CARRY)
		Hide_ship_name,				// Karajorma - Hides the ships name (like the -wcsaga command line used to but for any selected ship)
		Afterburner_locked,			// KeldorKatarn - This ship can't use its afterburners
		Set_class_dynamically,		// Karajorma - This ship should have its class assigned rather than simply read from the mission file 
		Lock_all_turrets_initially,	// Karajorma - Lock all turrets on this ship at mission start or on arrival
		Force_shields_on,
		No_ets,						// The E - This ship does not have an ETS
		Cloaked,					// The E - This ship will not be rendered
		No_thrusters,				// The E - Thrusters on this ship are not rendered.
		Ship_locked,				// Karajorma - Prevents the player from changing the ship class on loadout screen
		Weapons_locked,				// Karajorma - Prevents the player from changing the weapons on the ship on the loadout screen
		Ship_selective_linking,		// RSAXVC - Allow pilot to pick firing configuration
		Scramble_messages,			// Goober5000 - all messages sent from this ship appear scrambled
        No_secondary_lockon,        // zookeeper - secondary lock-on disabled
        No_disabled_self_destruct,  // Goober5000 - ship will not self-destruct after 90 seconds if engines or weapons destroyed (c.f. ai_maybe_self_destruct)
		Subsystem_movement_locked,	// The_E -- Rotating subsystems are locked in place.
		Draw_as_wireframe,			// The_E -- Ship will be rendered in wireframe mode
		Render_without_diffuse,		// The_E -- Ship will be rendered without diffuse map (needed for the lab)
		Render_without_glowmap,
		Render_without_specmap,
		Render_without_normalmap,
		Render_without_heightmap,
		Render_without_ambientmap,
		Render_without_miscmap,
		Render_without_reflectmap,
		Render_full_detail, 
		Render_without_light,
		Render_without_weapons,		// The_E -- Skip weapon model rendering
		Render_with_alpha_mult,
		Has_display_name,			// Goober5000
		Attempting_to_afterburn,    // set and unset by afterburner_start and stop, used by afterburner_min_fuel_to_consume
		Hide_mission_log,			// Goober5000 - mission log events generated for this ship will not be viewable
		No_passive_lightning,		// Asteroth - disables ship passive lightning
		Same_arrival_warp_when_docked,		// Goober5000
		Same_departure_warp_when_docked,	// Goober5000

		NUM_VALUES

	};

	FLAG_LIST(Exit_Flags) {
		Destroyed = 0,
		Departed,
		Cargo_known,
		Player_deleted,
		Been_tagged,
		Red_alert_carry,

		NUM_VALUES
	};

	FLAG_LIST(Info_Flags) {
		No_collide = 0,
		Player_ship,
		Default_player_ship,
		Path_fixup,						// when set, path verts have been set for this ship's model
		Support,						// this ship can perform repair/rearm functions
		Afterburner,					// this ship has afterburners
		Ballistic_primaries,			// this ship can equip ballistic primaries - Goober5000
		Cargo,							// is this ship a cargo type ship -- used for docking purposes
		Fighter,						// this ship is a fighter
		Bomber,							// this ship is a bomber
		Cruiser,						// this ship is a cruiser
		Freighter,						// this ship is a freighter
		Capital,						// this ship is a capital/installation ship
		Transport,						// this ship is a transport
		Navbuoy,						// AL 11-24-97: this is a navbuoy
		Sentrygun,						// AL 11-24-97: this is a navbuoy with turrets
		Escapepod,						// AL 12-09-97: escape pods that fire from big ships
		No_ship_type,					// made distinct to help trap errors
		Ship_copy,						// this ship is a copy of another ship in the table -- meaningful for scoring and possible other things
		In_tech_database,				// is ship type to be listed in the tech database?
		In_tech_database_m,				// is ship type to be listed in the tech database for multiplayer?
		Stealth,						// the ship has stealth capabilities
		Supercap,						// the ship is a supercap
		Drydock,						// the ship is a drydock
		Ship_class_dont_collide_invis,	// Don't collide with this ship's invisible polygons
		Big_damage,						// this ship is classified as a big damage ship
		Has_awacs,						// ship has an awacs subsystem
		Corvette,						// corvette class (currently this only means anything for briefing icons)
		Gas_miner,						// also just for briefing icons
		Awacs,							// ditto
		Knossos_device,					// this is the knossos device
		No_fred,						// not available in fred
		Default_in_tech_database,		// default in tech database - Goober5000
		Default_in_tech_database_m,		// ditto - Goober5000
		Flash,							// makes a flash when it explodes
		Show_ship_model,				// Show ship model even in first person view
		Surface_shields,				// _argv[-1], 16 Jan 2005: Enable surface shields for this ship.
		Generate_hud_icon,				// Enable generation of a HUD shield icon
		Disable_weapon_damage_scaling,	// WMC - Disable weapon scaling based on flags
		Gun_convergence,				// WMC - Gun convergence based on model weapon norms.
		No_thruster_geo_noise,			// Echelon9 - No thruster geometry noise.
		Intrinsic_no_shields,			// Chief - disables shields for this ship even without No Shields in mission.
		No_primary_linking,				// Chief - slated for 3.7 originally, but this looks pretty simple to implement.
		No_pain_flash,					// The E - disable red pain flash
		Allow_landings,					// SUSHI: Automatically set if any subsystems allow landings (as a shortcut)
		No_ets,							// The E - No ETS on this ship class
		No_lighting,					// Valathil - No lighting for this ship
		Dyn_primary_linking,			// RSAXVC - Dynamically generate weapon linking options
		Auto_spread_shields,			// zookeeper - auto spread shields
		Draw_weapon_models,				// the ship draws weapon models of any sort (used to be a boolean)
		Model_point_shields,			// zookeeper - uses model-defined shield points instead of quadrants
        Subsys_repair_when_disabled,    // MageKing17 - Subsystems auto-repair themselves even when disabled.
		Dont_bank_when_turning,			// Goober5000
		Dont_clamp_max_velocity,		// Goober5000
		Instantaneous_acceleration,		// Goober5000
		Has_display_name,				// Goober5000
		Large_ship_deathroll,			// Asteroth - big ships dont normally deathroll, this makes them do it!
		No_impact_debris,				// wookieejedi - Don't spawn the small debris on impact

		NUM_VALUES
	};

	FLAG_LIST(Aiming_Flags) {
		Autoaim = 0,			// has autoaim
		Auto_convergence,		// has automatic convergence
		Std_convergence,		// has standard - ie. non-automatic - convergence
		Autoaim_convergence,	// has autoaim with convergence
		Convergence_offset,		// marks that convergence has offset value

		NUM_VALUES
	};

    FLAG_LIST(Type_Info_Flags) {
        Counts_for_alone,
        Praise_destruction,
        Hotkey_on_list,
        Target_as_threat,
        Show_attack_direction,
        No_class_display,
        Scannable,
        Warp_pushes,
        Warp_pushable,
        Turret_tgt_ship_tgt,
        Beams_easily_hit,
        No_huge_impact_eff,
        AI_accept_player_orders,
        AI_auto_attacks,
        AI_attempt_broadside,
        AI_guards_attack,
        AI_turrets_attack,
        AI_can_form_wing,
        AI_protected_on_cripple,
		Targeted_by_huge_Ignored_by_small_only,

        NUM_VALUES
    };

	FLAG_LIST(Thruster_Flags) {
		Bank_right,
		Bank_left,
		Pitch_up,
		Pitch_down,
		Roll_right,
		Roll_left,
		Slide_right,
		Slide_left,
		Slide_up,
		Slide_down,
		Forward,
		Reverse,

		NUM_VALUES
	};


    // Not all wing flags are parseable or saveable in mission files. Right now, the only ones which can be set by mission designers are:
    // ignore_count, reinforcement, no_arrival_music, no_arrival_message, no_arrival_warp, no_departure_warp,
	// same_arrival_warp_when_docked, same_departure_warp_when_docked, no_dynamic, and nav_carry_status
    // Should that change, bump this variable and make sure to make the necessary changes to parse_wing (in missionparse)
#define PARSEABLE_WING_FLAGS 9
	
    FLAG_LIST(Wing_Flags) {
		Gone,					// all ships were either destroyed or departed
		Departing,				// wing's departure cue turned true
		Ignore_count,			// ignore all ships in this wing for goal counting purposes.
		Reinforcement,			// is this wing a reinforcement wing
		Reset_reinforcement,	// needed when we need to reset the wing's reinforcement flag (after calling it in)
		No_arrival_music,		// don't play arrival music when wing arrives
		Expanded,				// wing expanded in hotkey select screen
		No_arrival_message,		// don't play any arrival message
		No_arrival_warp,		// don't play warp effect for any arriving ships in this wing.
		No_departure_warp,		// don't play warp effect for any departing ships in this wing.
		No_dynamic,				// members of this wing relentlessly pursue their ai goals
		Departure_ordered,		// departure of this wing was ordered by player
		Never_existed,			// this wing never existed because something prevented it from being created (like its mother ship being destroyed)
		Nav_carry,				// Kazan - Wing has nav-carry-status
		Same_arrival_warp_when_docked,		// Goober5000
		Same_departure_warp_when_docked,	// Goober5000

		NUM_VALUES
	};

	FLAG_LIST(Subsys_Sound_Flags) {
		Alive,
		Dead,
		Rotate,
		Turret_rotation,

		NUM_VALUES
	};

	FLAG_LIST(Awacs_Warning_Flags) {
		Warn_25,
		Warn_75,

		NUM_VALUES
	};
}
#endif
