
#include "global_hooks.h"

namespace scripting {
namespace hooks {

std::shared_ptr<scripting::Hook> OnGameInit = scripting::Hook::Factory("On Game Init",
	"Executed at the start of the engine after all game data has been loaded.",
	{},
	CHA_GAMEINIT);

std::shared_ptr<OverridableHook> OnDeath = OverridableHook::Factory("On Death",
	"Invoked when an object (ship or asteroid) has been destroyed.",
	{
		{"Self", "object", "The object that was killed"},
		{"Ship", "ship", "The ship that was destroyed (only set for ships)"},
		{"Killer", "object", "The object that caused the death (only set for ships)"},
		{"Hitpos",
			"vector",
			"The position of the hit that caused the death (only set for ships and only if available)"},
	});

std::shared_ptr<OverridableHook> OnShipCollision = OverridableHook::Factory("On Ship Collision",
	"Invoked when a ship collides with another object. Note: When two ships collide this will be called twice, once "
	"with each ship object as the \"Ship\" parameter.",
	{{"Self", "object", "The \"other\" object that collided with the ship."},
		{"Object",
			"ship",
			"The ship object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Ship", "ship", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the colission was detected"},
		{"Debris", "object", "The debris object with which the ship collided (only set for debris collisions)"},
		{"Asteroid", "object", "The asteroid object with which the ship collided (only set for asteroid collisions)"},
		{"ShipB", "ship", "For ship on ship collisions, the \"other\" ship."},
		{"Weapon", "weapon", "The weapon object with which the ship collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"}});

} // namespace hooks
} // namespace scripting