#pragma once

#include "globalincs/flagset.h"

#include "actions/expression/ParseContext.h"
#include "actions/expression/ProgramVariables.h"
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

	TIMESTAMP waitTimestamp = TIMESTAMP::invalid();

	expression::ProgramVariables variables;
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

/**
 * @brief Constructs the "table" parse context
 *
 * This is the context that is available when parsing a program from an external table (e.g. ships or weapons).
 *
 * @return The parse context
 */
expression::ParseContext getTableParseContext();

/**
 * @brief Gets a program variables instance initialized with the default values
 *
 * The values are consistent with what is required for the default table parse context;
 *
 * @return The program variables
 */
expression::ProgramVariables getDefaultTableVariables();

} // namespace actions
