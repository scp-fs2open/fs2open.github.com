/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef _GROPENGLTEXTURE_H
#define _GROPENGLTEXTURE_H

#include "globalincs/pstypes.h"
#include "gropengl.h"

#include <glad/glad.h>

typedef struct tcache_slot_opengl {
	GLuint texture_id;
	GLenum texture_target;
	GLenum wrap_mode;
	float u_scale, v_scale;
	int	bitmap_handle;
	int	size;
	ushort w, h;
	int bpp;
	int mipmap_levels;
	uint32_t array_index;
	bool used;

	int fbo_id;

	tcache_slot_opengl()
	{
		this->reset();
	}

	void reset()
	{
		texture_id = 0;
		texture_target = GL_TEXTURE_2D;
		wrap_mode = GL_REPEAT;
		u_scale = 1.0f;
		v_scale = 1.0f;
		bitmap_handle = -1;
		size = 0;
		w = 0;
		h = 0;
		bpp = 0;
		mipmap_levels = 0;
		array_index = 0;
		used = false;
		fbo_id = -1;
	}
} tcache_slot_opengl;

extern glm::mat4 GL_texture_matrix;

extern int GL_min_texture_width;
extern GLint GL_max_texture_width;
extern int GL_min_texture_height;
extern GLint GL_max_texture_height;
extern GLint GL_supported_texture_units;
extern int GL_mipmap_filter;
extern GLenum GL_texture_target;
extern GLenum GL_texture_face;
extern GLfloat GL_anisotropy;
extern bool GL_rendering_to_texture;
extern GLint GL_max_renderbuffer_size;

void opengl_switch_arb(int unit, int state);
void opengl_tcache_init();
void opengl_free_texture_slot(int n);
void opengl_tcache_flush();
void opengl_tcache_shutdown();
void opengl_tcache_frame();
void opengl_set_additive_tex_env();
void opengl_set_modulate_tex_env();
void opengl_preload_init();
GLfloat opengl_get_max_anisotropy();
void opengl_kill_render_target(int slot);
int opengl_make_render_target(int handle, int slot, int *w, int *h, int *bpp, int *mm_lvl, int flags);
int opengl_set_render_target(int slot, int face = -1, int is_static = 0);
void gr_opengl_get_bitmap_from_texture(void* data_out, int bitmap_num);
size_t opengl_export_render_target( int slot, int width, int height, int alpha, int num_mipmaps, ubyte *image_data );
void opengl_set_texture_target(GLenum target = GL_TEXTURE_2D);
void opengl_set_texture_face(GLenum face = GL_TEXTURE_2D);

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, uint32_t *array_index, int stage = 0);
int gr_opengl_preload(int bitmap_num, int is_aabitmap);
void gr_opengl_set_texture_panning(float u, float v, bool enable);
void gr_opengl_set_texture_addressing(int mode);
GLuint opengl_get_rtt_framebuffer();
void gr_opengl_bm_generate_mip_maps(int slot);
void gr_opengl_get_texture_scale(int bitmap_handle, float *u_scale, float *v_scale);

#endif	//_GROPENGLTEXTURE_H
