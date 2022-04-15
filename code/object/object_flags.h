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
		Immobile,				// Goober5000 - doesn't move, no matter what
		Marked,					// Object is marked (Fred).  Can be reused in FreeSpace for anything that won't be used by Fred.
		Temp_marked,			// Temporarily marked (Fred).
		Hidden,					// Object is hidden (not shown) and can't be manipulated
		Collides_with_parent,	// Asteroth - Only used for weapons with 'Can_damage_shooter'
		Attackable_if_no_collide,	// Cyborg - Allows the AI to attack this object, even if no-collide is set (Cue Admiral Ackbar)

		NUM_VALUES
	};
}

#endif