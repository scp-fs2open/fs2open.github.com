
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

const std::shared_ptr<Hook<>> OnLoadScreen = Hook<>::Factory("On Load Screen",
	"Executed regularly during loading of a mission.",
	{ {"Progress", "number", "A number from 0 to 1 indicating how far along the loading process the game is."}});

const std::shared_ptr<Hook<>> OnCampaignMissionAccept = Hook<>::Factory("On Campaign Mission Accept",
	"Invoked after a campaign mission once the player accepts the result and moves on to the next mission instead of replaying it.",
	{});

const std::shared_ptr<Hook<>> OnBriefStage = Hook<>::Factory("On Briefing Stage",
	"Invoked for each briefing stage what it is shown.",
	{
		{"OldStage", "number", "The index of the previous briefing stage."},
		{"NewStage", "number", "The index of the new briefing stage."}
	});

const std::shared_ptr<Hook<>> OnMissionStart = Hook<>::Factory("On Mission Start",
	"Invoked when a mission starts.",
	{ {"Player", "object", "The player object."} });

const std::shared_ptr<Hook<>> OnGameplayStart = Hook<>::Factory("On Gameplay Start",
	"Invoked when the gameplay portion of a mission starts.",
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

const std::shared_ptr<Hook<ShipSourceConditions>> OnAfterburnerStart = Hook<ShipSourceConditions>::Factory("On Afterburner Engage",
	"Invoked whenever a ship engages its afterburners",
	{ { "Ship", "ship", "The ship engaging its afterburners" } });

const std::shared_ptr<Hook<ShipSourceConditions>> OnAfterburnerEnd = Hook<ShipSourceConditions>::Factory("On Afterburner Stop",
	"Invoked whenever a ship stops using its afterburners",
	{ { "Ship", "ship", "The ship which had been using its afterburners" } });

const std::shared_ptr<Hook<ShipSourceConditions>> OnWaypointsDone = Hook<ShipSourceConditions>::Factory("On Waypoints Done",
	"Invoked whenever a ship stops using its afterburners",
	{
		{ "Ship", "ship", "The ship which has completed the waypoints." },
		{ "Wing", "wing", "The wing which the ship belongs to. Can be invalid." },
		{ "Waypointlist", "waypointlist", "The set of waypoints which was completed." }
	});

const std::shared_ptr<Hook<ShipSourceConditions>> OnGoalsCleared = Hook<ShipSourceConditions>::Factory("On Goals Cleared",
	"Invoked whenever a ship has its goals cleared.",
	{ { "Ship", "ship", "The ship whose goals are cleared." } });

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
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"Debris", "object", "The debris object with which the ship collided (only set for debris collisions)"},
		{"Asteroid", "object", "The asteroid object with which the ship collided (only set for asteroid collisions)"},
		{"ShipB", "ship", "For ship on ship collisions, the \"other\" ship."},
		{"Weapon", "weapon", "The weapon object with which the ship collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the ship collided (only set for beam collisions)"}});

const std::shared_ptr<OverridableHook<CollisionConditions>> OnWeaponCollision = OverridableHook<CollisionConditions>::Factory("On Weapon Collision",
	"Invoked when a weapon collides with another object. Note: When two weapons collide this will be called twice, once "
	"with each weapon object as the \"Weapon\" parameter.",
	{ {"Self", "object", "The \"other\" object that collided with the weapon."},
		{"Object",
			"weapon",
			"The weapon object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Weapon", "weapon", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"Debris", "object", "The debris object with which the weapon collided (only set for debris collisions)"},
		{"Asteroid", "object", "The asteroid object with which the weapon collided (only set for asteroid collisions)"},
		{"Ship", "ship", "The ship object with which the weapon collided (only set for ship collisions)."},
		{"WeaponB", "weapon", "For weapon on weapon collisions, the \"other\" weapon."},
		{"Beam", "weapon", "The beam object with which the weapon collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnDebrisCollision = OverridableHook<CollisionConditions>::Factory("On Debris Collision",
	"Invoked when a debris piece collides with another object.",
	{ {"Self", "object", "The \"other\" object that collided with the debris."},
		{"Object",
			"debris",
			"The debris object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Debris", "debris", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"Asteroid", "object", "The asteroid object with which the debris collided (only set for asteroid collisions)"},
		{"Ship", "ship", "The ship object with which the debris collided (only set for ship collisions)."},
		{"Weapon", "weapon", "The weapon object with which the debris collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the debris collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnAsteroidCollision = OverridableHook<CollisionConditions>::Factory("On Asteroid Collision",
	"Invoked when an asteroid collides with another object.",
	{ {"Self", "object", "The \"other\" object that collided with the asteroid."},
		{"Object",
			"object",
			"The asteroid object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Asteroid", "object", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"Debris", "object", "The debris object with which the asteroid collided (only set for debris collisions)"},
		{"Ship", "ship", "The ship object with which the asteroid collided (only set for ship collisions)"},
		{"Weapon", "weapon", "The weapon object with which the asteroid collided (only set for weapon collisions)"},
		{"Beam", "weapon", "The beam object with which the asteroid collided (only set for beam collisions)"} });

const std::shared_ptr<OverridableHook<CollisionConditions>> OnBeamCollision = OverridableHook<CollisionConditions>::Factory("On Beam Collision",
	"Invoked when a beam collides with another object.",
	{ {"Self", "object", "The \"other\" object that collided with the beam."},
		{"Object",
			"weapon",
			"The beam object with which the \"other\" object collided with. Provided for consistency with other "
			"collision hooks."},
		{"Beam", "weapon", "Same as \"Object\""},
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"Debris", "object", "The debris object with which the beam collided (only set for debris collisions)"},
		{"Ship", "ship", "The ship object with which the asteroid collided (only set for ship collisions)"},
		{"Asteroid", "object", "The asteroid object with which the beam collided (only set for asteroid collisions)"},
		{"Weapon", "weapon", "The weapon object with which the beam collided (only set for weapon collisions)"}});

const std::shared_ptr<Hook<ShipArriveConditions>> OnShipArrive = Hook<ShipArriveConditions>::Factory("On Ship Arrive",
	"Invoked when a ship arrives in mission.",
	{
		{"Ship", "ship", "The ship that has arrived."},
		{"Parent", "object", "The object which serves as the arrival anchor of the ship. Could be nil."},
	});

const std::shared_ptr<Hook<WeaponCreatedConditions>> OnWeaponCreated = Hook<WeaponCreatedConditions>::Factory("On Weapon Created",
	"Invoked every time a weapon object is created.",
	{ {"Weapon", "weapon", "The weapon object."} });

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

const std::shared_ptr<Hook<SubsystemDeathConditions>> OnSubsystemDestroyed = Hook<SubsystemDeathConditions>::Factory("On Subsystem Destroyed",
	"Called when a subsystem is destroyed.",
	{
		{"Ship", "ship", "The ship that held the subsystem."},
		{"Subsystem", "subsystem", "The subsystem that has been destroyed."},
	});

const std::shared_ptr<Hook<ShipDepartConditions>> OnShipDepart = Hook<ShipDepartConditions>::Factory("On Ship Depart",
	"Invoked when a ship departs the mission without being destroyed.",
	{
		{"Ship", "ship", "The ship departing the mission"},
		{"JumpNode", "string", "The name of the jump node the ship jumped out of. Can be nil."},
		{"Method", "ship", "The name of the method the ship used to depart. One of: 'SHIP_DEPARTED', 'SHIP_DEPARTED_WARP', 'SHIP_DEPARTED_BAY', 'SHIP_VANISHED', 'SHIP_DEPARTED_REDALERT'."},
	});

const std::shared_ptr<Hook<WeaponDeathConditions>> OnWeaponDelete = Hook<WeaponDeathConditions>::Factory("On Weapon Delete",
	"Invoked whenever a weapon is deleted from the scene.",
	{
		{"Weapon", "weapon", "The weapon that was deleted."},
		{"Self", "weapon", "An alias for \"Weapon\"."},
	});

const std::shared_ptr<Hook<WeaponEquippedConditions>> OnWeaponEquipped = Hook<WeaponEquippedConditions>::Factory("On Weapon Equipped",
	"Invoked for each ship for each frame, allowing to be filtered for whether a weapon is equipped by the ship using conditions.",
	{
		{"User", "ship", "The ship that has a weapon equipped."},
		{"Target", "object", "The current AI target of this ship."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnWeaponFired = Hook<WeaponUsedConditions>::Factory("On Weapon Fired",
	"Invoked when a weapon is fired.",
	{
		{"User", "ship", "The ship that has fired the weapon."},
		{"Target", "object", "The current target of this ship."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnPrimaryFired = Hook<WeaponUsedConditions>::Factory("On Primary Fire",
	"Invoked when a primary weapon is fired.",
	{
		{"User", "ship", "The ship that has fired the weapon."},
		{"Target", "object", "The current target of this ship."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnSecondaryFired = Hook<WeaponUsedConditions>::Factory("On Secondary Fire",
	"Invoked when a secondary weapon is fired.",
	{
		{"User", "ship", "The ship that has fired the weapon."},
		{"Target", "object", "The current target of this ship."},
	});

const std::shared_ptr<Hook<WeaponSelectedConditions>> OnWeaponSelected = Hook<WeaponSelectedConditions>::Factory("On Weapon Selected",
	"Invoked when a new weapon is selected.",
	{
		{"User", "ship", "The ship that has selected the weapon."},
		{"Target", "object", "The current target of this ship."},
	});

const std::shared_ptr<Hook<WeaponDeselectedConditions>> OnWeaponDeselected = Hook<WeaponDeselectedConditions>::Factory("On Weapon Deselected",
	"Invoked when a weapon is deselected.",
	{
		{"User", "ship", "The ship that has deselected the weapon."},
		{"Target", "object", "The current target of this ship."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnTurretFired = Hook<WeaponUsedConditions>::Factory("On Turret Fired",
	"Invoked when a turret is fired.",
	{
		{"Ship", "ship", "The ship that has fired the turret."},
		{"Weapon", "weapon", "The spawned weapon object (nil if the turret fired a beam)."},
		{"Beam", "beam", "The spawned beam object (nil unless the turret fired a beam)."},
		{"Target", "object", "The current target of the shot."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnBeamFired = Hook<WeaponUsedConditions>::Factory("On Beam Fire",
	"Invoked when a beam is fired.",
	{
		{"Ship", "ship", "The ship that has fired the turret."},
		{"Beam", "beam", "The spawned beam object."},
		{"Target", "object", "The current target of the shot."},
	});

const std::shared_ptr<OverridableHook<ObjectDrawConditions>> OnHudDraw = OverridableHook<ObjectDrawConditions>::Factory("On HUD Draw",
	"Invoked when the HUD is rendered.",
	{ 
		{"Self", "object", "The object from which the scene is viewed."},
		{"Player", "object", "The player object."} 
	},
	CHA_HUDDRAW);

const std::shared_ptr<OverridableHook<ObjectDrawConditions>> OnObjectRender = OverridableHook<ObjectDrawConditions>::Factory("On Object Render",
	"Invoked every time an object is rendered.",
	{
		{"Self", "object", "The object which is rendered."}
	});

const std::shared_ptr<Hook<>> OnSimulation = Hook<>::Factory("On Simulation",
	"Invoked every time that FSO processes physics and AI.",
	{},
	CHA_SIMULATION);

const std::shared_ptr<OverridableHook<>> OnDialogInit = OverridableHook<>::Factory("On Dialog Init",
	"Invoked when a system dialog initalizes. Override to prevent the system dialog from loading dialog-related resources (requires retail files)",
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