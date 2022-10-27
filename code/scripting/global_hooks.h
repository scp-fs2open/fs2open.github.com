
#include "hook_api.h"

namespace scripting {
namespace hooks {

extern const std::shared_ptr<Hook>            OnGameInit;

extern const std::shared_ptr<Hook>            OnDebrisCreated;

extern const std::shared_ptr<OverridableHook> OnShipCollision;

extern const std::shared_ptr<Hook>            OnShipDeathStarted;
extern const std::shared_ptr<OverridableHook> OnShipDeath;
extern const std::shared_ptr<Hook>            OnMissileDeathStarted;
extern const std::shared_ptr<Hook>            OnMissileDeath;
extern const std::shared_ptr<Hook>            OnAsteroidDeath;
extern const std::shared_ptr<Hook>            OnDebrisDeath;

extern const std::shared_ptr<OverridableHook> OnDialogInit;
extern const std::shared_ptr<OverridableHook> OnDialogFrame;
extern const std::shared_ptr<Hook> OnDialogClose;

// deprecated
extern const std::shared_ptr<OverridableHook> OnDeath;

}
} // namespace scripting