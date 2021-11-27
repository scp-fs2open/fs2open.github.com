#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
class debug_sphere
{
	public:
	static void add(vec3d pos, float radius);
	static void render();

	private:
	static SCP_vector<debug_sphere> Spheres;
	static void clear();
	vec3d pos;
	float radius;
};
