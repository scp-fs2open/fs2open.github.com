
#include "global_hooks.h"

#include "hook_conditions.h"

namespace scripting {
namespace hooks {

const std::shared_ptr<Hook<>> OnGameInit = Hook<>::Factory("On Game Init",
	"Executed at the start of the engine after all game data has been loaded.",
	{},
	CHA_GAMEINIT);

const std::shared_ptr<OverridableHook<>> OnSplashScreen = OverridableHook<>::Factory("On Splash Screen",
	"Will be called once when the splash screen shows for the first time.",
	{},
	CHA_SPLASHSCREEN);

const std::shared_ptr<OverridableHook<>> OnStateStart = OverridableHook<>::Factory("On State Start",
	"Executed whenever a new state is entered.",
	{ 
		{"OldState", "object", "The gamestate object of the state that was executing."}, 
		{"NewState", "object", "The gamestate object of the state that will be executing."}
	});

const std::shared_ptr<Hook<>> OnMissionStart = Hook<>::Factory("On Mission Start",
	"Invoked when a mission starts.",
	{ {"Player", "object", "The player object."} });

const std::shared_ptr<Hook<ControlActionConditions>> OnAction = Hook<ControlActionConditions>::Factory("On Action",
	"Invoked whenever a user action was invoked through control input.",
	{ {"Action", "string", "The name of the action that was executed."} });

const std::shared_ptr<Hook<ControlActionConditions>> OnActionStopped = Hook<ControlActionConditions>::Factory("On Action Stopped",
	"Invoked whenever a user action is no longer invoked through control input.",
	{ {"Action", "string", "The name of the action that was stopped."} });

const std::shared_ptr<Hook<>> OnKeyPressed = Hook<>::Factory("On Key Pressed",
	"Invoked whenever a key is pressed.",
	{ {"Key", "string", "The scancode of the key that has been pressed."} });

const std::shared_ptr<Hook<>> OnKeyReleased = Hook<>::Factory("On Key Released",
	"Invoked whenever a key is released.",
	{ {"Key", "string", "The scancode of the key that has been released."} });

const std::shared_ptr<Hook<>> OnMouseMoved = Hook<>::Factory("On Mouse Moved",
	"Invoked whenever the mouse is moved.",
	{});

const std::shared_ptr<Hook<>> OnMousePressed = Hook<>::Factory("On Mouse Pressed",
	"Invoked whenever a mouse button is pressed.",
	{});

const std::shared_ptr<Hook<>> OnMouseReleased = Hook<>::Factory("On Mouse Released",
	"Invoked whenever a mouse button is released.",
	{});

const std::shared_ptr<Hook<ShipSourceConditions>> OnDebrisCreated = Hook<ShipSourceConditions>::Factory(
	"On Debris Created",
	"Invoked when a piece of debris is created.",
	{
		{"Debris", "debris", "The newly created debris object"},
		{"Source", "object", "The object (probably a ship) from which this debris piece was spawned."},
	});

const std::shared_ptr<OverridableHook<CollisionConditions>> OnShipCollision = OverridableHook<CollisionConditions>::Factory("On Ship Collision",
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

const std::shared_ptr<OverridableHook<CollisionConditions>> OnWeaponCollision = OverridableHook<CollisionConditions>::Factory("On Weapon Collision",
	"Invoked when a ship collides with another object. Note: When two weapons collide this will be called twice, once "
	"with each weapon object as the \"Weapon\" parameter.",
	{ {"Self", "object", "The \"other\" object that collided with the weapon."},
		{"Object",
			"weapon",
			"The weapon object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Weapon", "weapon", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the colission was detected"},
		{"Debris", "object", "The debris object with which the ship collided (only set for debris collisions)"},
		{"Asteroid", "object", "The asteroid object with which the ship collided (only set for asteroid collisions)"},
		{"Ship", "ship", "The ship object with which the weapon collided (only set for ship collisions)."},
		{"WeaponB", "weapon", "For weapon on weapon collisions, the \"other\" weapon."},
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnDebrisCollision = OverridableHook<CollisionConditions>::Factory("On Debris Collision",
	"Invoked when a ship collides with another object.",
	{ {"Self", "object", "The \"other\" object that collided with the ship."},
		{"Object",
			"debris",
			"The debris object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Debris", "debris", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the colission was detected"},
		{"Asteroid", "object", "The asteroid object with which the ship collided (only set for asteroid collisions)"},
		{"Ship", "ship", "The ship object with which the ship collided (only set for ship collisions)."},
		{"Weapon", "weapon", "The weapon object with which the ship collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnAsteroidCollision = OverridableHook<CollisionConditions>::Factory("On Asteroid Collision",
	"Invoked when a ship collides with another object.",
	{ {"Self", "object", "The \"other\" object that collided with the ship."},
		{"Object",
			"object",
			"The asteroid object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Asteroid", "object", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the colission was detected"},
		{"Debris", "object", "The debris object with which the ship collided (only set for debris collisions)"},
		{"Ship", "ship", "The ship object with which the ship collided (only set for ship collisions)"},
		{"Weapon", "weapon", "The weapon object with which the ship collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnBeamCollision = OverridableHook<CollisionConditions>::Factory("On Beam Collision",
	"Invoked when a ship collides with another object. Note: When two ships collide this will be called twice, once "
	"with each ship object as the \"Ship\" parameter.",
	{ {"Self", "object", "The \"other\" object that collided with the ship."},
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
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"} });

const std::shared_ptr<Hook<ShipDeathConditions>> OnShipDeathStarted = Hook<ShipDeathConditions>::Factory(
	"On Ship Death Started", "Called when a ship starts the death process.",
	{
		{"Ship", "ship", "The ship that has begun the death process."},
		{"Killer", "object", "The object responsible for killing the ship.  Always set but could be invalid if there is no killer."},
		{"Hitpos", "vector", "The world coordinates of the killing blow.  Could be nil."},
	});

const std::shared_ptr<OverridableHook<ShipDeathConditions>> OnShipDeath = OverridableHook<ShipDeathConditions>::Factory(
	"On Ship Death", "Called when a ship has been destroyed.  Supersedes On Death for ships.",
	{
		{"Ship", "ship", "The ship that was destroyed."},
		{"Killer", "object", "The object responsible for killing the ship.  Always set but could be invalid if there is no killer."},
		{"Hitpos", "vector", "The world coordinates of the killing blow.  Could be nil."},
	});

const std::shared_ptr<Hook<WeaponDeathConditions>> OnMissileDeathStarted = Hook<WeaponDeathConditions>::Factory(
	"On Missile Death Started", "Called when a missile is about to be destroyed (whether by impact, interception, or expiration).",
	{
		{"Weapon", "weapon", "The weapon that was destroyed."},
		{"Object", "object", "The object that the weapon hit - a ship, asteroid, or piece of debris.  Always set but could be invalid if there is no other object.  If this missile was destroyed by another weapon, the 'other object' will be invalid but the DestroyedByWeapon flag will be set."},
	});

const std::shared_ptr<Hook<WeaponDeathConditions>> OnMissileDeath = Hook<WeaponDeathConditions>::Factory(
	"On Missile Death", "Called when a missile has been destroyed (whether by impact, interception, or expiration).",
	{
		{"Weapon", "weapon", "The weapon that was destroyed."},
		{"Object", "object", "The object that the weapon hit - a ship, asteroid, or piece of debris.  Always set but could be invalid if there is no other object.  If this missile was destroyed by another weapon, the 'other object' will be invalid but the DestroyedByWeapon flag will be set."},
	});

const std::shared_ptr<Hook<>> OnAsteroidDeath = Hook<>::Factory(
	"On Asteroid Death", "Called when an asteroid has been destroyed.  Supersedes On Death for asteroids.",
	{
		{"Asteroid", "asteroid", "The asteroid that was destroyed."},
		{"Hitpos", "vector", "The world coordinates of the killing blow."},
	});

const std::shared_ptr<Hook<>> OnDebrisDeath = Hook<>::Factory(
	"On Debris Death", "Called when a piece of debris has been destroyed.",
	{
		{"Debris", "debris", "The piece of debris that was destroyed."},
		{"Hitpos", "vector", "The world coordinates of the killing blow.  Could be nil."},
	});

const std::shared_ptr<OverridableHook<ObjectDrawConditions>> OnHudDraw = OverridableHook<ObjectDrawConditions>::Factory("On HUD Draw",
	"Invoked when the HUD is rendered.",
	{ 
		{"Self", "object", "The object from which the scene is viewed."},
		{"Player", "object", "The player object."} 
	},
	CHA_HUDDRAW);

const std::shared_ptr<OverridableHook<>> OnDialogInit = OverridableHook<>::Factory("On Dialog Init",
	"Invoked when a system dialog initalizes. Override to prevent the system dialog to load dialog-related resources (requires retail files)",
	{   
		{"Choices",
			"table",
			"A table containing the different choices for this dialog. Contains subtables, each consisting of "
			"Positivity (an int, 0 if neutral, 1 if positive, and -1 if negative) and "
			"Text (a string, the text of the button)."},
		{"Title", "string", "The title of the popup window. Nil for a death popup."},
		{"Text", "string", "The text to be displayed in the popup window. Nil for a death popup."},
		{"IsTimeStopped", "boolean", "True if mission time was interrupted for this popup."},
		{"IsStateRunning", "boolean", "True if the underlying state is still being processed and rendered."},
		{"IsInputPopup", "boolean", "True if this popup is for entering text."},
		{"IsDeathPopup", "boolean", "True if this popup is an in-mission death popup and should be styled as such."},
		{"AllowedInput", "string", "A string of characters allowed to be present in the input popup. Nil if not an input popup."}
	 });

const std::shared_ptr<OverridableHook<>> OnDialogFrame = OverridableHook<>::Factory("On Dialog Frame",
	"Invoked each frame for a system dialog. Override to prevent the system dialog from rendering and evaluating.",
	{
		{"Submit", "function(number | string | nil result) -> nil", "A callback function that should be called if the popup resolves. Should be string only if it is an input popup. Pass nil to abort."},
		{"IsDeathPopup", "boolean", "True if this popup is an in-mission death popup and should be styled as such."},
		{"Freeze", "boolean", "If not nil and true, the popup should not process any inputs but just render."}
	});

const std::shared_ptr<Hook<>> OnDialogClose = Hook<>::Factory("On Dialog Close",
	"Invoked when a dialog closes.",
	{
		{"IsDeathPopup", "boolean", "True if this popup is an in-mission death popup and should be styled as such."}
	});

const std::shared_ptr<Hook<>> OnCheat = Hook<>::Factory("On Cheat",
	"Called when a cheat is used",
	{
		{ "Cheat", "string", "The cheat code the user typed" },
	});

// ========== DEPRECATED ==========

const std::shared_ptr<OverridableHook<ObjectDeathConditions>> OnDeath = OverridableHook<ObjectDeathConditions>::Factory("On Death",
	"Invoked when an object (ship or asteroid) has been destroyed.  Deprecated in favor of On Ship Destroyed and On Asteroid Destroyed.",
	{
		{"Self", "object", "The object that was killed"},
		{"Ship", "ship", "The ship that was destroyed (only set for ships)"},
		{"Killer", "object", "The object that caused the death (only set for ships)"},
		{"Hitpos",
			"vector",
			"The position of the hit that caused the death (only set for ships and only if available)"},
	});

} // namespace hooks
} // namespace scripting