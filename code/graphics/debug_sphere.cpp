#include "debug_sphere.h"

#include "globalincs/vmallocator.h"

#include "render/3d.h"


SCP_vector<debug_sphere> debug_sphere::Spheres;

debug_sphere::debug_sphere(const vec3d& pos_in, float rad_in)
{
	pos = pos_in;
	radius = rad_in;
}
void debug_sphere::add(const vec3d& pos, float rad)
{
	debug_sphere::Spheres.emplace_back(debug_sphere(pos, rad));
}

void debug_sphere::clear()
{
	debug_sphere::Spheres.clear();
}

void debug_sphere::render()
{
	for (auto& s : Spheres) {
		g3_draw_sphere_ez(&(s.pos), s.radius);
	}
	clear();
}
