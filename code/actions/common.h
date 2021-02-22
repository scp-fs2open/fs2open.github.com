#pragma once

#include "globalincs/flagset.h"

#include "math/vecmat.h"
#include "object/object.h"

class object;
class ship_subsys;

namespace actions {

struct ProgramLocals {
	object_h host;
	vec3d localPosition = vmd_zero_vector;
	matrix localOrient = vmd_identity_matrix;
	int hostSubobject = -1;

	int waitTimestamp = 0;
	vec3d position = vmd_zero_vector;
	vec3d direction = vmd_zero_vector;
};

/**
 * @brief A list of flags for altering the program parsing behavior
 *
 * This should be used for differentiating between different program environments when validating the program (e.g. a
 * particle effect requires an object host).
 */
// clang-format off
FLAG_LIST(ProgramContextFlags) {
	HasObject = 0, //!< The program will be started with an object host
	HasSubobject,  //!< The program will be started with a subobject host

	NUM_VALUES
};
// clang-format on

} // namespace actions
