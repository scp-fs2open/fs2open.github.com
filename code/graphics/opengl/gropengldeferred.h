#pragma once

#include "globalincs/pstypes.h"

void gr_opengl_deferred_init();

void opengl_clear_deferred_buffers();
void gr_opengl_deferred_lighting_begin(bool clearNonColorBufs = false);
void gr_opengl_deferred_lighting_end();
void gr_opengl_deferred_lighting_finish();

void gr_opengl_deferred_light_sphere_init(int rings, int segments);
void gr_opengl_deferred_light_cylinder_init(int segments);

void gr_opengl_draw_deferred_light_sphere(const vec3d *position);
void gr_opengl_draw_deferred_light_cylinder(const vec3d *position, const matrix *orient);

void gr_opengl_deferred_shutdown();

void opengl_draw_sphere();
