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
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"


typedef struct tcache_slot_opengl {
	GLuint texture_id;
	GLenum texture_target;
	GLenum wrap_mode;
	float u_scale, v_scale;
	int	bitmap_handle;
	int	size;
	ushort w, h;
	ubyte bpp;
	ubyte mipmap_levels;

	tcache_slot_opengl() :
		texture_id(0), texture_target(GL_TEXTURE_2D), wrap_mode(GL_REPEAT),
		u_scale(1.0f), v_scale(1.0f), bitmap_handle(-1), size(0), w(0), h(0),
		bpp(0), mipmap_levels(0)
	{
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
	}
} tcache_slot_opengl;

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
int opengl_make_render_target(int handle, int slot, int *w, int *h, ubyte *bpp, int *mm_lvl, int flags);
int opengl_set_render_target(int slot, int face = -1, int is_static = 0);
int opengl_export_image(int slot, int width, int height, int alpha, int num_mipmaps, ubyte *image_data = NULL);
void opengl_set_texture_target(GLenum target = GL_TEXTURE_2D);
void opengl_set_texture_face(GLenum face = GL_TEXTURE_2D);

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int stage = 0);
int gr_opengl_preload(int bitmap_num, int is_aabitmap);
void gr_opengl_set_texture_panning(float u, float v, bool enable);
void gr_opengl_set_texture_addressing(int mode);
GLuint opengl_get_rtt_framebuffer();

#endif	//_GROPENGLTEXTURE_H
