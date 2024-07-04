
#include "events.h"

namespace events {

util::event<void> EngineUpdate;

util::event<void> EngineShutdown;

util::event<void, int, int> GameLeaveState;

util::event<void, int, int> GameEnterState;

util::event<void, const char*> GameMissionLoad;
}
