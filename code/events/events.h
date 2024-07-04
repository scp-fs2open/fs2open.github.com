#pragma once

#include "utils/event.h"

namespace events {

extern util::event<void> EngineUpdate;

extern util::event<void> EngineShutdown;

extern util::event<void, int, int> GameLeaveState;

extern util::event<void, int, int> GameEnterState;

extern util::event<void, const char*> GameMissionLoad;
}
