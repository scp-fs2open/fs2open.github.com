#pragma once

#include "mission/missionbriefcommon.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct redalert_stage_h {
	int ra_brief, ra_stage;
	redalert_stage_h();
	explicit redalert_stage_h(int brief, int stage);
	bool IsValid() const;
	brief_stage* getStage() const;
};

DECLARE_ADE_OBJ(l_RedAlertStage, redalert_stage_h);

} // namespace api
} // namespace scripting