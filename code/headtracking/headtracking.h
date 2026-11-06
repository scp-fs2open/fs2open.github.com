
#ifndef HEADTRACKING_H
#define HEADTRACKING_H

#include "globalincs/pstypes.h"

namespace headtracking
{
	struct HeadTrackingStatus
	{
		float x;
		float y;
		float z;

		// Rotations are normalized: -1 to 1 maps to -180 to 180 degrees
		float pitch;
		float roll;
		float yaw;
	};

	bool init();

	bool isEnabled();

	bool query();

	HeadTrackingStatus* getStatus();

	void shutdown();
}

#endif // HEADTRACKING_H
