#pragma once

#include "mission/missionbriefcommon.h"
#include "missionui/missiondebrief.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct debrief_stage_h {
	debrief_stage* stage;
	debrief_stage_h();
	explicit debrief_stage_h(debrief_stage* db_stage);
	bool IsValid() const;
	debrief_stage* getStage() const;
};

DECLARE_ADE_OBJ(l_DebriefStage, debrief_stage_h);

DECLARE_ADE_OBJ(l_Debrief, int);

} // namespace api
} // namespace scripting