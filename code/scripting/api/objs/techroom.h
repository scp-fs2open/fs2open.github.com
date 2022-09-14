#pragma once

#include "menuui/readyroom.h"
#include "cutscene/cutscenes.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct sim_mission_h {
	int missionIdx;
	bool isCMission;
	sim_mission_h();
	explicit sim_mission_h(int index, bool cmission);
	bool IsValid() const;
	sim_mission* getStage() const;
};

struct cutscene_info_h {
	int cutscene;
	cutscene_info_h();
	explicit cutscene_info_h(int scene);
	bool IsValid() const;
	cutscene_info* getStage() const;
};

DECLARE_ADE_OBJ(l_TechRoomMission, sim_mission_h);
DECLARE_ADE_OBJ(l_TechRoomCutscene, cutscene_info_h);

} // namespace api
} // namespace scripting
