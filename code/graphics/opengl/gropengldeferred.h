#pragma once

#include "globalincs/pstypes.h"
#include "graphics/util/UniformAligner.h"
#include "graphics/util/uniform_structs.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
namespace ltp = lighting_profiles;
using namespace ltp; 

void gr_opengl_deferred_init();

void opengl_clear_deferred_buffers();
void gr_opengl_deferred_lighting_begin(bool clearNonColorBufs = false);
void gr_opengl_deferred_lighting_msaa();
void gr_opengl_deferred_lighting_end();
void gr_opengl_deferred_lighting_finish();
graphics::deferred_light_data* prepare_light_uniforms(light& l, graphics::util::UniformAligner& uniformAligner, ltp::profile lp);

void gr_opengl_deferred_light_sphere_init(int rings, int segments);
void gr_opengl_deferred_light_cylinder_init(int segments);

void gr_opengl_draw_deferred_light_sphere(const vec3d *position);
void gr_opengl_draw_deferred_light_cylinder(const vec3d *position, const matrix *orient);

void gr_opengl_deferred_shutdown();

void gr_opengl_override_fog(bool set_override);

void opengl_draw_sphere();
