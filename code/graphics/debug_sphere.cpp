#include "debug_sphere.h"
#include "globalincs/vmallocator.h"
#include "render/3d.h"

SCP_vector<debug_sphere> debug_sphere::Spheres;

void debug_sphere::add(vec3d pos, float rad)
{
	debug_sphere s;
	s.pos = pos;
	s.radius = rad;
	debug_sphere::Spheres.push_back(s);
}

void debug_sphere::clear()
{
	debug_sphere::Spheres.clear();
}

void debug_sphere::render()
{
	for(auto s : Spheres){
		g3_draw_sphere_ez(&(s.pos),s.radius);
	}
	clear();
}
