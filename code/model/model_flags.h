#ifndef MODEL_FLAGS_H
#define MODEL_FLAGS_H

#include "globalincs/pstypes.h"

namespace Model {

	FLAG_LIST(Subsystem_Flags) {
		Rotates,			// This means the object rotates automatically with "turn_rate"
		Stepped_rotate,		// This means that the rotation occurs in steps
		Ai_rotate,			// This means that the rotation is controlled by ai
		Crewpoint,			// If set, this is a crew point.
		Turret_matrix,		// If set, this has it's turret matrix created correctly.
		Awacs,				// If set, this subsystem has AWACS capability
		Artillery,			// if this rotates when weapons are fired - Goober5000
		Triggered,			// rotates when triggered by something
		Untargetable,		// Goober5000
		Carry_no_damage,	// WMC
		Use_multiple_guns,	// WMC
		Fire_on_normal,		// forces a turret to fire down its normal vecs
		Turret_hull_check,	// makes the turret check to see if it's going to shoot through it's own hull before fireing - Bobboau
		Turret_fixed_fp,	// forces turret (when defined with multiple weapons) to prevent the firepoints from alternating
		Turret_salvo,		// forces turret to fire salvos (all guns simultaneously) - independent targeting
		Fire_on_target,		// prevents turret from firing unless it is pointing at the firingpoints are pointing at the target
		No_ss_targeting,	// toggles the subsystem targeting for the turret
		Turret_reset_idle,	// makes turret reset to their initial position if the target is out of field of view
		Turret_alt_math,	// tells the game to use additional calculations should turret have a defined y fov
		Dum_rotates,		// Bobboau
		Carry_shockwave,	// subsystem - even with 'carry no damage' flag - will carry shockwave damage to the hull
		Allow_landing,		// This subsystem can be landed on
		Fov_edge_check,		// Tells the game to use better FOV edge checking with this turret
		Fov_required,		// Tells game not to allow this turret to attempt targeting objects out of FOV
		No_replace,			// set the subsys not to draw replacement ('destroyed') model
		No_live_debris,		// sets the subsys not to release live debris
		Ignore_if_dead,		// tells homing missiles to ignore the subsys if its dead and home on to hull instead of earlier subsys pos
		Allow_vanishing,	// allows subsystem to vanish (prevents explosions & sounds effects from being played)
		Damage_as_hull,		// applies armor damage to subsystem instead of subsystem damage - FUBAR
		Turret_locked,      // Turret starts locked by default - Sushi
		No_aggregate,		// Don't include with aggregate subsystem types - Goober5000
		Turret_anim_wait,	// Turret won't fire until animation is complete - Sushi
		Player_turret_sound,
		Turret_only_target_if_can_fire,// Turrets only target things they're allowed to shoot at (e.g. if check-hull fails, won't keep targeting)
		No_disappear,		// Submodel won't disappear when subsystem destroyed
		Collide_submodel,	// subsystem takes damage only from hits which impact the associated submodel
		Destroyed_rotation, // allows subobjects to continue to rotate even if they have been destroyed

		NUM_VALUES
	};
}

#endif