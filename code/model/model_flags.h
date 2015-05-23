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

	FLAG_LIST(Render_Flags) {
		Normal,					// Draw a normal object
		Show_outline,			// Draw the object in outline mode. Color specified by model_set_outline_color
		Show_pivots,			// Show the pivot points
		Show_paths,				// Show the paths associated with a model
		Show_radius,			// Show the radius around the object
		Show_shields,			// Show the shield mesh
		Show_thrusters,			// Show the engine thrusters. See model_set_thrust for how long it draws.
		Lock_detail,			// Only draw the detail level defined in model_set_detail_level
		No_polys,				// Don't draw the polygons.
		No_lighting,			// Don't perform any lighting on the model.
		No_texturing,			// Draw textures as flat-shaded polygons.
		No_correct,				// Don't to correct texture mapping
		No_smoothing,			// Don't perform smoothing on vertices.
		Is_asteroid,			// When set, treat this as an asteroid.  
		Is_missile,				// When set, treat this as a missilie.  No lighting, small thrusters.
		Show_outline_preset,	// Draw the object in outline mode. Color assumed to be set already.	
		Show_invisible_faces,	// Show invisible faces as green...
		Autocenter,				// Always use the center of the hull bounding box as the center, instead of the pivot point
		Bay_paths,				// draw bay paths
		All_xparent,			// render it fully transparent
		No_zbuffer,				// switch z-buffering off completely 
		No_cull,				// don't cull backfacing poly's
		Force_texture,			// force a given texture to always be used
		Force_lower_detail,		// force the model to draw 1 LOD lower, if possible
		Edge_alpha,				// makes norms that are faceing away from you render more transparent -Bobboau
		Center_alpha,			// oposite of above -Bobboau
		No_fogging,				// Don't fog - taylor
		Show_outline_htl,		// Show outlines (wireframe view) using HTL method
		No_glowmaps,			// disable rendering of glowmaps - taylor
		Full_detail,			// render all valid objects, particularly ones that are otherwise in/out of render boxes - taylor
		Force_clamp,			// force clamp - Hery
		Animated_shader,		// Use a animated Shader - Valathil
		
		NUM_VALUES
	};
}

#endif