#ifndef WEAPON_FLAGS_H
#define WEAPON_FLAGS_H

#include "globalincs/flagset.h"

namespace Weapon {

	FLAG_LIST(Info_Flags) {
		Homing_heat,						// if set, this weapon homes via seeking heat
		Homing_aspect,						// if set, this weapon homes via chasing aspect
		Electronics,						// Takes out electronics systems.
		Spawn,								// Spawns projectiles on detonation.
		Remote,								// Can be remotely detonated by parent.
		Puncture,							// Punctures armor, damaging subsystems.
		Supercap,							// This is a weapon which does supercap class damage (meaning, it applies real damage to supercap ships)
		Cmeasure,							// Weapon acts as a countermeasure
		Homing_javelin,						// WC Saga Javelin HS style heatseeker, locks only on target's engines
		Turns,								// Set this if the weapon ever changes heading. If you don't set this and the weapon turns, collision detection won't work, I promise!
		Swarm,								// Missile "swarms".. ie changes heading and twists on way to target
		Trail,								// Has a trail
		Big_only,							// Only big ships (cruiser, capital, etc.) can arm this weapon
		Child,								// No ship can have this weapon.  It gets created by weapon detonations.
		Bomb,								// Bomb-type missile, can be targeted
		Huge,								// Huge damage (generally 500+), probably only fired at huge ships.
		No_dumbfire,						// Missile cannot be fired dumbfire (ie requires aspect lock)
		No_doublefire,						// Disables linked firing for secondaries - EatThePath
		Thruster,							// Has thruster cone and/or glow
		In_tech_database,
		Player_allowed,						// allowed to be on starting wing ships/in weaponry pool
		Bomber_plus,						// Fire this missile only at a bomber or big ship.  But not a fighter.
		Corkscrew,							// corkscrew style missile
		Particle_spew,						// spews particles as it travels
		Emp,								// weapon explodes with a serious EMP effect
		Energy_suck,						// energy suck primary (impact effect)
		Flak,								// use for big-ship turrets - flak gun
		Beam,								// if this is a beam weapon : NOTE - VERY SPECIAL CASE
		Tag,								// this weapon has a tag effect when it hits
		Shudder,							// causes the weapon to shudder. shudder is proportional to the mass and damage of the weapon
		Mflash,								// has muzzle flash
		Lockarm,							// if the missile was fired without a lock, it does significanlty less damage on impact
		Stream,								// handled by "trigger down/trigger up" instead of "fire - wait - fire - wait"
		Ballistic,							// ballistic primaries - Goober5000
		Pierce_shields,						// shield pierceing -Bobboau
		Default_in_tech_database,			// default in tech database - Goober5000
		Local_ssm,							// localized ssm. ship that fires ssm is in mission.  ssms also warp back in during mission
		Tagged_only,						// can only fire if target is tagged
		Cycle,								// will only fire from (shots (defalts to 1)) points at a time
		Small_only,							// can only be used against small ships like fighters or bombers
		Same_turret_cooldown,				// the weapon has the same cooldown time on turrets
		Mr_no_lighting,						// don't render with lighting, regardless of user options
		Transparent,						// render as transparent
		Training,							// Weapon does shield/hull damage, but doesn't hurt subsystems, whack you, or put marks on your ship.
		Smart_spawn,						// Spawn weapon that is fired via turrets like normal weapons
		Inherit_parent_target,				// child weapons home in on the target their parent is homing on.
		No_emp_kill,						// though weapon has hitpoints it can not be disabled by EMP
		Variable_lead_homing,				// allows user defined scaler to be added to lead (to enable, lead, pure or lag pursuit for missiles)
		Untargeted_heat_seeker,				// forces heat seeker to lose target immeadiately (and acquire a random new one)
		No_radius_doubling,					// removes the radius doubling effect bombs have for collisions
		Non_subsys_homing,					// spreads fired missiles around the target ships hull
		No_life_lost_if_missed,				// prevents game from shortening the lifeleft of the missed but still homing missiles
		Custom_seeker_str,					// sets the game to use custom seeker strengths instead of default values
		Can_be_targeted,					// allows non-bomb weapons to be targeted
		Shown_on_radar,						// allows non-bombs be visible on radar
		Show_friendly,						// allows friendly weapon radar dots be drawn
		Capital_plus,						// AI will not use this weapon on fighters or bombers
		External_weapon_fp,					// will try to use external models FPs if possible
		External_weapon_lnch,				// render external secondary as a launcher
		Takes_blast_damage,					// This weapon can take blast damage
		Takes_shockwave_damage,				// This weapon can take shockwave damage
		Dont_show_on_radar,					// Force a weapon to not show on radar
		Render_flak,						// Even though this is a flak weapon, render the shell
		Ciws,								// This weapons' burst and shockwave damage can damage bombs (Basically, a reverse for TAKES_BLAST/SHOCKWAVE_DAMAGE
		Antisubsysbeam,						// This beam can target subsystems as per normal
		Nolink,								// This weapon can not be linked with others
		Use_emp_time_for_capship_turrets,	// override MAX_TURRET_DISRUPT_TIME in emp.cpp - Goober5000
		No_linked_penalty,					// This weapon does not count into linked firing penalty
		No_homing_speed_ramp,				// Disables the 1s long speed ramping when firing locked-on secondaries
		Cmeasure_aspect_home_on,			// This countermeasure flag makes aspect seekers home on the countermeasure instead of going into dumbfire mode
		Turret_Interceptable,				// These two flags mark a weapon as being interceptable by the AI
		Fighter_Interceptable,				// (like WIF_BOMB), without forcing it to be tagetable -MageKing17
		Aoe_Electronics,					// Apply electronics effect across the weapon's entire area of effect instead of just on the impacted ship -MageKing17
		Apply_Recoil,						// Apply Recoil using weapon and ship info
        Dont_spawn_if_shot,                 // Prevent shot down parent weapons from spawning children (DahBlount)
        Die_on_lost_lock,                   // WIF_LOCKED_HOMING missiles will die if they lose their lock
		Has_display_name,					// Goober5000
		No_impact_spew,						// Goober5000
		Require_exact_los,					// If secondary or in turret, will only fire if ship has line of sight to target
		Can_damage_shooter,					// this weapon and any of its descendants can damage its shooter - Asteroth
		Heals,								// 'damage' heals instead of actually damaging - Asteroth
		SecondaryNoAmmo,					// Secondaries that only use energy
		No_collide,
		Multilock_target_dead_subsys,

        NUM_VALUES
	};

	FLAG_LIST(Weapon_Flags) {
		Lock_warning_played,		// set when a lock warning sound is played for the player (needed since we don't want to play multiple lock sounds)
		Already_applied_stats,		// for use in ship_apply_local and ship_apply_global damage functions so that we don't record multiple hits (stats) for one impact
		Played_flyby_sound,			// flyby sound has been played for this weapon
		Consider_for_flyby_sound,	// consider for flyby
		Dead_in_water,				// a missiles engines have died
		Locked_when_fired,			// fired with a lock
		Destroyed_by_weapon,		// destroyed by damage from other weapon
		Spawned,					//Spawned from a spawning type weapon
        No_homing,                  // this weapon should ignore any homing behavior it'd usually have
		Overridden_homing,          // Homing is overridden by an external source (probably scripting)
		Multi_homing_update_needed, // this is a newly spawned homing weapon which needs to update client machines
		Multi_Update_Sent,			// Marks this missile as already being updated once by the server

		NUM_VALUES
	};

	FLAG_LIST(Burst_Flags) {
		Fast_firing,
		Random_length,
		Resets,
		Num_firepoints_burst_shots, // Burst shots is set to however many firepoints the firer has

		NUM_VALUES
	};

	FLAG_LIST(Beam_Info_Flags) {
		Burst_share_random,
		Track_own_texture_tiling,

		NUM_VALUES
	};
}

#endif
