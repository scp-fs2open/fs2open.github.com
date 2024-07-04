#ifndef STARFIELD_FLAGS_H
#define STARFIELD_FLAGS_H

#include "globalincs/flagset.h"

namespace Starfield {

	FLAG_LIST(Background_Flags) {
		Corrected_angles_in_mission_file = 0,	// If this flag is present, this background will save its angles to the mission file correctly.  If this flag is not present, they are saved with the incorrect math that was used in older missions.

		NUM_VALUES
	};

}
#endif
