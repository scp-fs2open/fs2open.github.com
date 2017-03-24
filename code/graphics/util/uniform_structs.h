#pragma once

#include "globalincs/pstypes.h"

namespace graphics {

/**
 * @file
 *
 * This file contains definitions for GPU uniform buffer structs. These structs must respect the std140 layout rules.
 * Read the OpenGL specification for the exact layout and padding rules.
 */

/**
 * @brief Data for one deferred light rendering call
 */
struct deferred_light_data {
	vec3d diffuseLightColor;
	float coneAngle;

	vec3d specLightColor;
	float coneInnerAngle;

	vec3d coneDir;
	float dualCone;

	vec3d scale;
	float lightRadius;

	int lightType;

	int pad0[3]; // Struct size must be 16-bytes aligned because we use vec3s
};

}
