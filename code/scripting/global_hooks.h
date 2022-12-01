
#include "hook_api.h"
#include "scripting/hook_conditions.h"

namespace scripting {
namespace hooks {

extern const std::shared_ptr<Hook<>>									OnGameInit;

extern const std::shared_ptr<Hook<ShipSourceConditions>>				OnDebrisCreated;

extern const std::shared_ptr<OverridableHook<CollisionConditions>>		OnShipCollision;

extern const std::shared_ptr<Hook<ShipDeathConditions>>					OnShipDeathStarted;
extern const std::shared_ptr<OverridableHook<ShipDeathConditions>>		OnShipDeath;
extern const std::shared_ptr<Hook<WeaponDeathConditions>>				OnMissileDeathStarted;
extern const std::shared_ptr<Hook<WeaponDeathConditions>>				OnMissileDeath;
extern const std::shared_ptr<Hook<>>									OnAsteroidDeath;
extern const std::shared_ptr<Hook<>>									OnDebrisDeath;

extern const std::shared_ptr<OverridableHook<>>							OnDialogInit;
extern const std::shared_ptr<OverridableHook<>>							OnDialogFrame;
extern const std::shared_ptr<Hook<>>									OnDialogClose;

extern const std::shared_ptr<Hook<>>									OnCheat;

// deprecated
extern const std::shared_ptr<OverridableHook<ObjectDeathConditions>>	OnDeath;

}
} // namespace scripting