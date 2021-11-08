#include "debug_sphere.h"
#include "globalincs/vmallocator.h"
#include "render/3d.h"

namespace debug_spheres{

SCP_vector<debug_sphere> Spheres;

void add(vec3d pos, float rad){
	debug_sphere s;
	s.pos = pos;
	s.radius = rad;
	Spheres.push_back(s);
}

void clear(){
	Spheres.clear();
}
void render(){
	for(auto s : Spheres){
		g3_draw_sphere_ez(&(s.pos),s.radius);
	}
	clear();
}

}