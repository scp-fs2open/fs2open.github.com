#pragma once

#include "mission/missionbriefcommon.h"
#include "mission/missiongoals.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct brief_stage_h {
	int br_brief, br_stage;
	brief_stage_h();
	explicit brief_stage_h(int brief, int stage);
	bool IsValid() const;
	brief_stage* getStage() const;
};

DECLARE_ADE_OBJ(l_BriefStage, brief_stage_h);

DECLARE_ADE_OBJ(l_Brief, int);

DECLARE_ADE_OBJ(l_Goals, int);

} // namespace api
} // namespace scripting