
#include "global_hooks.h"

#include "hook_conditions.h"

namespace scripting {
namespace hooks {

const std::shared_ptr<Hook<>> OnGameInit = Hook<>::Factory("On Game Init",
	"Executed at the start of the engine after all game data has been loaded.",
	{},
	std::nullopt,
	CHA_GAMEINIT);

const std::shared_ptr<Hook<>> OnSplashEnd = Hook<>::Factory("On Splash End",
	"Executed just after the splash screen fades out.",
	{});

const std::shared_ptr<OverridableHook<>> OnIntroAboutToPlay = OverridableHook<>::Factory("On Intro About To Play",
	"Executed just before the intro movie is played.",
	{});

const std::shared_ptr<OverridableHook<>> OnMovieAboutToPlay = OverridableHook<>::Factory("On Movie About To Play",
	"Executed just before any cutscene movie is played.",
	{
		{"Filename", "string", "The filename of the movie that is about to play."},
		{"ViaTechRoom", "boolean", "Whether the movie player was invoked through the tech room."},
	});

const std::shared_ptr<Hook<>> OnOptionsTabChanged = Hook<>::Factory("On Options Menu Tab Changed",
	"Executed whenever a tab is changed within the Options Menu.",
	{
		{"TabNumber", "number", "The number of the tab that has been changed to, with 0 = Options Tab, 1 = Multi Tab, and 2 = Details Tab. "},
	});

const std::shared_ptr<Hook<>> OnOptionsMenuClosed = Hook<>::Factory("On Options Menu Closed",
	"Executed whenever the Options Menu is closed.",
	{
		{"OptionsAccepted", "boolean", "Whether or not the options are being accepted and saved. Value is true if the options are accepted/saved, false if the options are discarded and not accepted/saved. "},
	});

const std::shared_ptr<Hook<>> OnHUDConfigMenuClosed = Hook<>::Factory("On HUD Config Menu Closed",
	"Executed whenever the HUD Config Menu is closed.",
	{
		{"OptionsAccepted", "boolean", "Whether or not the options are being accepted and saved. Value is true if the options are accepted/saved, false if the options are discarded and not accepted/saved. "},
	});

const std::shared_ptr<Hook<>> OnControlConfigMenuClosed = Hook<>::Factory("On Controls Config Menu Closed",
	"Executed whenever the Control Config Menu is closed.",
	{
		{"OptionsAccepted", "boolean", "Whether or not the options are being accepted and saved. Value is true if the options are accepted/saved, false if the options are discarded and not accepted/saved. "},
	});

const std::shared_ptr<OverridableHook<>> OnStateStart = OverridableHook<>::Factory("On State Start",
	"Executed whenever a new state is entered.",
	{
		{"OldState", "gamestate", "The gamestate that was executing."}, 
		{"NewState", "gamestate", "The gamestate that will be executing."}
	});

const std::shared_ptr<Hook<>> OnLoadScreen = Hook<>::Factory("On Load Screen",
	"Executed regularly during loading of a mission.",
	{ {"Progress", "number", "A number from 0 to 1 indicating how far along the loading process the game is."}});

const std::shared_ptr<Hook<>> OnLoadComplete =
	Hook<>::Factory("On Load Complete", "Executed once a mission load has completed.", {});

const std::shared_ptr<Hook<>> OnCampaignMissionAccept = Hook<>::Factory("On Campaign Mission Accept",
	"Invoked after a campaign mission once the player accepts the result and moves on to the next mission instead of replaying it.",
	{
		{"Mission", "string", "The filename of the mission that was just accepted."}
	});

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

const std::shared_ptr<OverridableHook<KeyPressConditions>> OnKeyPressed = OverridableHook<KeyPressConditions>::Factory("On Key Pressed",
	"Invoked whenever a key is pressed. If overridden, FSO behaves as if this key has simply not been pressed. "
	"The only thing that FSO will do with this key if overridden is fire the corresponding OnKeyReleased hook once the key is released. "
	"Be especially careful if overriding modifier keys (such as Alt and Shift) with this.",
	{
		{"Key", "string", "The scancode of the key that has been pressed."},
		{"RawKey", "string", "The scancode of the key that has been pressed, without modifiers applied."}
	});

const std::shared_ptr<Hook<KeyPressConditions>> OnKeyReleased = Hook<KeyPressConditions>::Factory("On Key Released",
	"Invoked whenever a key is released.",
	{
		{"Key", "string", "The scancode of the key that has been pressed."},
		{"RawKey", "string", "The scancode of the key that has been pressed, without modifiers applied."},
		{"TimeHeld", "number", "The time that this key has been held down in milliseconds. Can be 0 if input latency fluctuates."},
		{"WasOverridden", "boolean", "Whether or not the key press corresponding to this release was overridden."}
	});

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
	"with each ship as the \"Self\" parameter.",
	{{"Self", "object", "The object the ship collided with."},
		{"Object", "ship",
			"The ship that collided with \"Self\". Provided for consistency with other collision hooks."},
		{"Ship", "ship", "For ship-on-ship collisions, the same as \"Self\". For ship-on-object collisions, the same as \"Object\"."},
		{"Hitpos", "vector", "The world position where the collision was detected"},
		{"ShipSubmodel", "submodel", "The submodel of \"Ship\" involved in the collision, if \"Ship\" was the heavier object"},
		{"Debris", "object", "The debris object with which the ship collided (only set for debris collisions)"},
		{"Asteroid", "object", "The asteroid object with which the ship collided (only set for asteroid collisions)"},
		{"ShipB", "ship", "For ship-on-ship collisions, the same as \"Object\" (only set for ship-on-ship collisions)"},
		{"ShipBSubmodel", "submodel", "For ship-on-ship collisions, the submodel of \"ShipB\" involved in the collision, if \"ShipB\" was the heavier object"},
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
		{"Ship", "ship", "The ship object with which the beam collided (only set for ship collisions)"},
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

const std::shared_ptr<Hook<>> OnBeamDeath = Hook<>::Factory(
	"On Beam Death", "Called when a beam has been removed from the mission (whether by finishing firing, destruction of turret, etc.).",
	{
		{"Beam", "beam", "The beam that was removed."},
	});

const std::shared_ptr<Hook<>> OnAsteroidCreated = Hook<>::Factory("On Asteroid Created",
	"Called when an asteroid has been created.",
	{
		{"Asteroid", "asteroid", "The asteroid that was created."},
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

const std::shared_ptr<Hook<WeaponUsedConditions>> OnBeamWarmup = Hook<WeaponUsedConditions>::Factory("On Beam Warmup",
	"Invoked when a beam starts warming up to fire.",
	{
		{"User", "ship", "The ship that is firing the beam."},
		{"Beam", "beam", "The spawned beam object."},
		{"Target", "object", "The current target of the shot."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnBeamFired = Hook<WeaponUsedConditions>::Factory("On Beam Fire",
	"Invoked when a beam starts firing (after warming up).",
	{
		{"User", "ship", "The ship that is firing the beam."},
		{"Beam", "beam", "The spawned beam object."},
		{"Target", "object", "The current target of the shot."},
	});

const std::shared_ptr<Hook<WeaponUsedConditions>> OnBeamWarmdown = Hook<WeaponUsedConditions>::Factory("On Beam Warmdown",
	"Invoked when a beam starts \"warming down\" after firing.",
	{
		{"User", "ship", "The ship that is firing the beam."},
		{"Beam", "beam", "The spawned beam object."},
		{"Target", "object", "The current target of the shot."},
	});

const std::shared_ptr<OverridableHook<>> OnHudCommMenuOpened = OverridableHook<>::Factory("On HUD Comm Menu Opened",
	"Invoked when the HUD comm menu, or squad message menu, is displayed.",
	{
		{"Player", "object", "The player object."}
	});

const std::shared_ptr<OverridableHook<CommOrderConditions>> OnHudCommOrderIssued = OverridableHook<CommOrderConditions>::Factory(
	"On HUD Comm Order Issued",
	"Invoked when the player issues an order through the squad message menu.",
	{
		{"Sender", "ship", "The ship that sent the order. Usually the player."},
		{"Recipient", "oswpt", "The recipient of the order."},
		{"Target", "ship", "The target if the order, if any. Usually the Player's current target."},
		{"Subsystem", "subsystem", "The target subsystem, if any. Usually the Player's current target."},
		{"Order", "enumeration", "The order issued. Will be one of the SQUAD_MESSAGE enumerations."},
		{"Name", "string", "The name of the order as it appears in the squad message menu. Useful for LuaAI orders."}
	});

const std::shared_ptr<OverridableHook<>> OnHudCommMenuClosed = OverridableHook<>::Factory("On HUD Comm Menu Closed",
	"Invoked when the HUD comm menu, or squad message menu, is hidden.",
	{
		{"Player", "object", "The player object."}
	});

const std::shared_ptr<OverridableHook<ObjectDrawConditions>> OnHudDraw = OverridableHook<ObjectDrawConditions>::Factory("On HUD Draw",
	"Invoked when the HUD is rendered.",
	{ 
		{"Self", "object", "The object from which the scene is viewed."},
		{"Player", "object", "The player object."} 
	},
	std::nullopt,
	CHA_HUDDRAW);

const std::shared_ptr<OverridableHook<ObjectDrawConditions>> OnObjectRender = OverridableHook<ObjectDrawConditions>::Factory("On Object Render",
	"Invoked every time an object is rendered.",
	{
		{"Self", "object", "The object which is rendered."}
	});

const std::shared_ptr<Hook<>> OnSimulation = Hook<>::Factory("On Simulation",
	"Invoked every time that FSO processes physics and AI.",
	{},
	std::nullopt,
	CHA_SIMULATION);

const std::shared_ptr<OverridableHook<>> OnDialogInit = OverridableHook<>::Factory("On Dialog Init",
	"Invoked when a system dialog initializes. Override to prevent the system dialog from loading dialog-related resources (requires retail files)",
	{   
		{"Choices",
			"table",
			"A table containing the different choices for this dialog. Contains subtables, each consisting of "
			"Positivity (an int, 0 if neutral, 1 if positive, and -1 if negative), "
			"Text (a string, the text of the button), and "
			"Shorcut (a string, the keypress that should activate the choice or nil if no valid shortcut)."},
		{"Title", "string", "The title of the popup window. Nil for a death popup."},
		{"Text", "string", "The text to be displayed in the popup window. Nil for a death popup."},
		{"IsTimeStopped", "boolean", "True if mission time was interrupted for this popup."},
		{"IsStateRunning", "boolean", "True if the underlying state is still being processed and rendered."},
		{"IsInputPopup", "boolean", "True if this popup is for entering text."},
		{"IsDeathPopup", "boolean", "True if this popup is an in-mission death popup and should be styled as such."},
		{"AllowedInput", "string", "A string of characters allowed to be present in the input popup. Nil if not an input popup."},
		{"DeathMessage", "string", "The death message if the dialog is a death popup. Nil if not a death popup."}
	 });

const std::shared_ptr<OverridableHook<>> OnDialogFrame = OverridableHook<>::Factory("On Dialog Frame",
	"Invoked each frame for a system dialog. Override to prevent the system dialog from rendering and evaluating.",
	{
		{"Submit", "function(number | string | nil result) -> nil", "A callback function that should be called if the popup resolves. Should be string only if it is an input popup. Pass nil to abort."},
		{"IsDeathPopup", "boolean", "True if this popup is an in-mission death popup and should be styled as such."},
		{"Freeze", "boolean", "If not nil and true, the popup should not process any inputs but just render."},
		{"Text", "string", "The dialog text as it may have been updated this frame"}
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

const std::shared_ptr<OverridableHook<>> OnMissionGoalStatusChanged = scripting::OverridableHook<>::Factory(
	"On Mission Goal Status Changed", "Called when a goal is marked as failed or completed.",
	{
		{"Name", "string", "The name of the goal."},
		{"Description", "string", "The description of the goal."},
		{"Type", "number", "The goal type. 1 for Primary, 2 for secondary, 3 for bonus."},
		{"State", "boolean", "True if the goal was completed, false if it was failed."},
	});

const std::shared_ptr<Hook<>> OnMissionAboutToEndHook = Hook<>::Factory("On Mission About To End",
	"Called when a mission is about to end but has not run any mission-ending logic",
	{});

const std::shared_ptr<OverridableHook<>> OnMissionEndHook =
	scripting::OverridableHook<>::Factory("On Mission End", "Called when a mission has ended", {});

const std::shared_ptr<Hook<>> OnStateAboutToEndHook = Hook<>::Factory("On State About To End",
	"Called when a game state is about to end but has not run any state-ending logic",
	{
		{"OldState", "gamestate", "The game state that has ended."},
		{"NewState", "gamestate", "The game state that will begin next."},
	});

const std::shared_ptr<OverridableHook<>> OnStateEndHook = OverridableHook<>::Factory("On State End",
	"Called when a game state has ended",
	{
		{"OldState", "gamestate", "The game state that has ended."},
		{"NewState", "gamestate", "The game state that will begin next."},
	});

const std::shared_ptr<Hook<>> OnCameraSetUpHook = Hook<>::Factory("On Camera Set Up",
	"Called every frame when the camera is positioned and oriented for rendering.",
	{
		{"Camera", "camera", "The camera about to be used for rendering."},
	});

// ========== FRED HOOKS ==========

const std::shared_ptr<Hook<>> FredOnMissionLoad = Hook<>::Factory("FRED On Mission Load",
	"Invoked when a new mission is loaded.",
	{});

const std::shared_ptr<Hook<>> FredOnMissionSpecsSave = Hook<>::Factory("FRED On Mission Specs Save",
	"Invoked when the Mission Specs dialog OK Button has been hit and all data is sucessfully saved.",
	{});

// ========== DEPRECATED ==========

const std::shared_ptr<OverridableHook<>> OnSplashScreen = OverridableHook<>::Factory("On Splash Screen",
	"Will be called once when the splash screen shows for the first time.",
	{},
	HookDeprecationOptions(gameversion::version(23, 0), HookDeprecationOptions::DeprecationLevel::LEVEL_WARN, HookDeprecationOptions::DeprecationLevel::LEVEL_ERROR),
	CHA_SPLASHSCREEN);

const std::shared_ptr<OverridableHook<ObjectDeathConditions>> OnDeath = OverridableHook<ObjectDeathConditions>::Factory("On Death",
	"Invoked when an object (ship or asteroid) has been destroyed.  Deprecated in favor of On Ship Death and On Asteroid Death.",
	{
		{"Self", "object", "The object that was killed"},
		{"Ship", "ship", "The ship that was destroyed (only set for ships)"},
		{"Killer", "object", "The object that caused the death (only set for ships)"},
		{"Hitpos",
			"vector",
			"The position of the hit that caused the death (only set for ships and only if available)"},
	},
	HookDeprecationOptions(gameversion::version(23, 0)));

} // namespace hooks
} // namespace scripting