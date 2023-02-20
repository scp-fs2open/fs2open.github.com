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
	Spheres.emplace_back(pos, rad);
}

void debug_sphere::clear()
{
	Spheres.clear();
}

void debug_sphere::render()
{
	color uicolor = gr_screen.current_color;
	color render_color;
	render_color.red = 255;
	render_color.green = 255;
	render_color.blue = 255;
	render_color.alpha = 64;
	gr_set_color_fast(&render_color);
	for (auto& s : Spheres) {
		g3_draw_sphere_ez(&(s.pos), s.radius);
	}
	gr_set_color_fast(&uicolor);
	clear();
}
