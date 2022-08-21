
#include "hook_api.h"

namespace scripting {
namespace hooks {

extern std::shared_ptr<scripting::Hook> OnGameInit;

extern std::shared_ptr<OverridableHook> OnDeath;

extern std::shared_ptr<OverridableHook> OnShipCollision;

}
} // namespace scripting