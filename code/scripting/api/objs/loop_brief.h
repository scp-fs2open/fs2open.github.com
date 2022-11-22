#pragma once

#include "mission/missioncampaign.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct cmission_h {
	int l_stage;
	cmission_h();
	explicit cmission_h(int stage);
	bool IsValid() const;
	cmission* getStage() const;
};

DECLARE_ADE_OBJ(l_LoopBriefStage, cmission_h);

} // namespace api
} // namespace scripting