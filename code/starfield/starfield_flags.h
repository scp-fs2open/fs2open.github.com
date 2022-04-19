#ifndef STARFIELD_FLAGS_H
#define STARFIELD_FLAGS_H

#include "globalincs/flagset.h"

namespace Starfield {

	FLAG_LIST(Background_Flags){
		Fixed_angles_in_mission_file = 0,	// angles are saved to the mission file correctly; if this flag is not present they are saved using incorrect math

		NUM_VALUES
	};

}
#endif
