
#ifndef _GROPENGLPOSTPROCESSING_H
#define _GROPENGLPOSTPROCESSING_H

#include "globalincs/pstypes.h"
#include "graphics/opengl/gropenglshader.h"

void opengl_post_process_init();
void opengl_post_process_shutdown();

/**
 * @brief Gives the bytes that the bloom and SMAA render textures hold
 *
 * The profiler overlay's memory panel adds this value to the "render targets" group. The function
 * counts only the textures that exist at the moment of the call.
 */
size_t opengl_get_postprocessing_render_target_bytes();

void gr_opengl_post_process_set_effect(const char *name, int x, const vec3d *rgb);
void gr_opengl_post_process_set_defaults();
void gr_opengl_post_process_save_zbuffer();
void gr_opengl_post_process_restore_zbuffer();
void gr_opengl_post_process_begin();
void gr_opengl_post_process_end();

void opengl_post_shader_header(SCP_stringstream &sflags, shader_type shader_t, int flags);

#endif	// _GROPENGLPOSTPROCESSING_H
