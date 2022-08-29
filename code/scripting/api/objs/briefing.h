#pragma once

#include "mission/missionbriefcommon.h"
#include "mission/missiongoals.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ(l_BriefStage, brief_stage);

DECLARE_ADE_OBJ(l_Brief, briefing);

DECLARE_ADE_OBJ(l_Goals, mission_goal);

} // namespace api
} // namespace scripting