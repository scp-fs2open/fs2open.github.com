#pragma once

#include "menuui/readyroom.h"
#include "cutscene/cutscenes.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ(l_TechRoomMission, sim_mission);
DECLARE_ADE_OBJ(l_TechRoomCutscene, cutscene_info);

} // namespace api
} // namespace scripting
