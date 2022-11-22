#pragma once

#include "missionui/fictionviewer.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct fiction_viewer_stage_h {
	int f_stage;
	fiction_viewer_stage_h();
	explicit fiction_viewer_stage_h(int stage);
	bool IsValid() const;
	fiction_viewer_stage* getStage() const;
};

DECLARE_ADE_OBJ(l_FictionViewerStage, fiction_viewer_stage_h);

} // namespace api
} // namespace scripting
