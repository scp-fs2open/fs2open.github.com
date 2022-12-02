
#include "hook_api.h"
#include "scripting/hook_conditions.h"

namespace scripting {
namespace hooks {

extern const std::shared_ptr<Hook<>>									OnGameInit;
extern const std::shared_ptr<OverridableHook<>>							OnSplashScreen;
//The On State Start hook previously used to pass OldState to the conditions, but no semantically sensible condition read the value, so we pretend it has no local condition
extern const std::shared_ptr<OverridableHook<>>							OnStateStart;

extern const std::shared_ptr<Hook<>>									OnLoadScreen;
extern const std::shared_ptr<Hook<>>									OnCampaignMissionAccept;
extern const std::shared_ptr<Hook<>>									OnBriefStage;
extern const std::shared_ptr<Hook<>>									OnMissionStart; 
extern const std::shared_ptr<Hook<>>									OnGameplayStart;

extern const std::shared_ptr<Hook<ControlActionConditions>>				OnAction;
extern const std::shared_ptr<Hook<ControlActionConditions>>				OnActionStopped;
extern const std::shared_ptr<Hook<>>									OnKeyPressed;
extern const std::shared_ptr<Hook<>>									OnKeyReleased;
extern const std::shared_ptr<Hook<>>									OnMouseMoved;
extern const std::shared_ptr<Hook<>>									OnMousePressed;
extern const std::shared_ptr<Hook<>>									OnMouseReleased;

extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnAfterburnerStart;
extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnAfterburnerEnd;
extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnWaypointsDone;
extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnGoalsCleared;

extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnDebrisCreated;

extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnShipCollision;
extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnWeaponCollision;
extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnBeamCollision;
extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnDebrisCollision;
extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnAsteroidCollision;

extern const std::shared_ptr<Hook<ShipSpawnConditions>>					OnShipArrive;
extern const std::shared_ptr<Hook<WeaponCreatedConditions>>				OnWeaponCreated;

extern const std::shared_ptr<Hook<ShipDeathConditions>>					OnShipDeathStarted;
extern const std::shared_ptr<OverridableHook<ShipDeathConditions>>		OnShipDeath;
extern const std::shared_ptr<Hook<WeaponDeathConditions>>				OnMissileDeathStarted;
extern const std::shared_ptr<Hook<WeaponDeathConditions>>				OnMissileDeath;
extern const std::shared_ptr<Hook<>>									OnAsteroidDeath;
extern const std::shared_ptr<Hook<>>									OnDebrisDeath;
extern const std::shared_ptr<Hook<SubsystemDeathConditions>>			OnSubsystemDestroyed;

extern const std::shared_ptr<Hook<ShipDepartConditions>>				OnShipDepart;
extern const std::shared_ptr<Hook<WeaponDeathConditions>>				OnWeaponDelete;

extern const std::shared_ptr<Hook<WeaponEquippedConditions>>			OnWeaponEquipped;
extern const std::shared_ptr<Hook<WeaponUsedConditions>>				OnWeaponFired;
extern const std::shared_ptr<Hook<WeaponUsedConditions>>				OnPrimaryFired;
extern const std::shared_ptr<Hook<WeaponUsedConditions>>				OnSecondaryFired;
extern const std::shared_ptr<Hook<WeaponSelectedConditions>>			OnWeaponSelected;
extern const std::shared_ptr<Hook<WeaponDeselectedConditions>>			OnWeaponDeselected;
extern const std::shared_ptr<Hook<WeaponUsedConditions>>				OnTurretFired;
extern const std::shared_ptr<Hook<WeaponUsedConditions>>				OnBeamFired;

extern const std::shared_ptr<OverridableHook<ObjectDrawConditions>>		OnHudDraw;
extern const std::shared_ptr<OverridableHook<ObjectDrawConditions>>		OnObjectRender;
extern const std::shared_ptr<Hook<>>									OnSimulation;


extern const std::shared_ptr<OverridableHook<>>							OnDialogInit;
extern const std::shared_ptr<OverridableHook<>>							OnDialogFrame;
extern const std::shared_ptr<Hook<>>									OnDialogClose;

extern const std::shared_ptr<Hook<>>									OnCheat;

// deprecated
extern const std::shared_ptr<OverridableHook<ObjectDeathConditions>>	OnDeath;

}
} // namespace scripting