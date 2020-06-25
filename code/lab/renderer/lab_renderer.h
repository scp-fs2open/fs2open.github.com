#pragma once

#include "globalincs/pstypes.h"


class LabRenderer {
public:
	LabRenderer() {
	}

	void onFrame(float frametime);
	
	void useBackground(SCP_string* mission_name);
};