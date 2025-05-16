#ifndef OBJECT_FLAGS_H
#define OBJECT_FLAGS_H

#include "globalincs/flagset.h"

namespace Object {
	FLAG_LIST(Object_Flags) {
		Renders,				// It renders as something ( objtype_render gets called)
		Collides,				// It collides with stuff (objtype_check_impact & objtype_hit gets called)
		Physics,				// It moves with standard physics.
		Should_be_dead,			// this object should be dead, so next time we can, we should delete this object.
		Invulnerable,			// invulnerable
		Protected,				// Don't kill this object, probably mission-critical.
		Player_ship,			// this object under control of some player -- don't do ai stuff on it!!!
		No_shields,				// object has no shield generator system (i.e. no shields)
		Could_be_player,		// for multiplayer -- indicates that it is selectable ingame joiners as their ship
		Was_rendered,			// Set if this object was rendered this frame.  Only gets set if OF_RENDERS set.  Gets cleared or set in obj_render_all().
		Not_in_coll,			// object has not been added to collision list
		Beam_protected,			// don't fire beam weapons at this type of object, probably mission critical.
		Special_warpin,			// Object has special warp-in enabled.
		Docked_already_handled,	// Goober5000 - a docked object that we already moved
		Targetable_as_bomb,
		Flak_protected,			// Goober5000 - protected from flak turrets
		Laser_protected,		// Goober5000 - protected from laser turrets
		Missile_protected,		// Goober5000 - protected from missile turrets
		Immobile,				// Goober5000 - doesn't change position or orientation, no matter what (legacy flag, but not deprecated due to the difficulty of mapping one flag to two for compatibility)
		Dont_change_position,	// Goober5000 - doesn't change position, no matter what
		Dont_change_orientation,	// Goober5000 - doesn't change orientation, no matter what
		Marked,					// Object is marked (Fred).  Can be reused in FreeSpace for anything that won't be used by Fred.
		Temp_marked,			// Temporarily marked (Fred).
		Hidden,					// Object is hidden (not shown in Fred) and can't be manipulated
		Locked_from_editing,	// Object cannot be edited (Fred)
		Collides_with_parent,	// Asteroth - Only used for weapons with 'Can_damage_shooter'
		Attackable_if_no_collide,	// Cyborg - Allows the AI to attack this object, even if no-collide is set (Cue Admiral Ackbar)
		Collision_cache_stale,	// This object has a stale collision cache, and will be recalculated this frame

		NUM_VALUES
	};

	FLAG_LIST(Aiming_Flags){
		Autoaim = 0,           // has autoaim
		Auto_convergence,      // has automatic convergence
		Std_convergence,       // has standard - ie. non-automatic - convergence
		Autoaim_convergence,   // has autoaim with convergence
		Convergence_offset,    // marks that convergence has offset valuem, only used for ships not weapons

		NUM_VALUES
	};

	FLAG_LIST(Raw_Pof_Flags){Glowmaps_disabled, // No glowmaps for this weapon instance
		Draw_as_wireframe,                      // Render wireframe for this weapon instance
		Render_full_detail,                     // Render full detail for this weapon instance
		Render_without_light,                   // Render without light for this weapon instance
		Render_without_diffuse,                 // Render without diffuse for this weapon instance
		Render_without_glowmap,                 // Render without glowmap for this weapon instance
		Render_without_normalmap,               // Render without normal map for this weapon instance
		Render_without_heightmap,               // Render without height map for this weapon instance
		Render_without_ambientmap,              // Render without ambient for this weapon instance
		Render_without_specmap,                 // Render without spec for this weapon instance
		Render_without_reflectmap,              // Render without reflect for this weapon instance

		NUM_VALUES};
	}

#endif