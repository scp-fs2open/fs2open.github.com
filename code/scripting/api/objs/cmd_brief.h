#pragma once

#include "missionui/missioncmdbrief.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct cmd_brief_stage_h {
	int cmd_brief, cmd_stage;

	cmd_brief_stage_h();
	explicit cmd_brief_stage_h(int brief, int stage);
	bool IsValid() const;
	cmd_brief_stage* getStage() const;
};

DECLARE_ADE_OBJ(l_CmdBriefStage, cmd_brief_stage_h);

DECLARE_ADE_OBJ(l_CmdBrief, int);

} // namespace api
} // namespace scripting
