#pragma once

#include "globalincs/flagset.h"

namespace CoordinatePoint {

FLAG_LIST(Flags){
	Visible_in_mission,    // Render the shape in-game (not just in the editor) and allow target-in-front pickup.

	NUM_VALUES};

} // namespace CoordinatePoint
