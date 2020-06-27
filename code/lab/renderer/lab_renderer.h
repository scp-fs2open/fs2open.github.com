#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

class LabRenderer {
public:
	LabRenderer() {
	}

	void onFrame(float frametime);
	
	void useBackground(SCP_string* mission_name);

	void toggleInsigniaRendering() {}

	void setAAMode(AntiAliasMode mode) {}

	void useNextTeamColorPreset() {}

	void usePreviousTeamColorPreset() {}

	void resetView() {}
};