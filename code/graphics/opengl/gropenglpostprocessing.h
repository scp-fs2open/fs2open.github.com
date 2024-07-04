
#ifndef _GROPENGLPOSTPROCESSING_H
#define _GROPENGLPOSTPROCESSING_H

#include "globalincs/pstypes.h"
#include "graphics/opengl/gropenglshader.h"

void opengl_post_process_init();
void opengl_post_process_shutdown();

void gr_opengl_post_process_set_effect(const char *name, int x, const vec3d *rgb);
void gr_opengl_post_process_set_defaults();
void gr_opengl_post_process_save_zbuffer();
void gr_opengl_post_process_restore_zbuffer();
void gr_opengl_post_process_begin();
void gr_opengl_post_process_end();

void opengl_post_shader_header(SCP_stringstream &sflags, shader_type shader_t, int flags);

#endif	// _GROPENGLPOSTPROCESSING_H
