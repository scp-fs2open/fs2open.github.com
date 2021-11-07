#pragma once

#include "globalincs/pstypes.h"
struct debug_sphere {
	vec3d pos;
	float radius;
};

namespace debug_spheres {
	void add(vec3d pos, float radius);
	void clear();
	void render();
}