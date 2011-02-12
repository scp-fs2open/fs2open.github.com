/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef _GROPENGLSHADER_H
#define _GROPENGLSHADER_H

#include "globalincs/pstypes.h"

#include "graphics/gropengl.h"

#include <string>


#define SDR_FLAG_LIGHT			(1<<0)
#define SDR_FLAG_NUM_LIGHTS_1	(1<<1)
#define SDR_FLAG_NUM_LIGHTS_2	(1<<2)
#define SDR_FLAG_NUM_LIGHTS_3	(1<<3)
#define SDR_FLAG_NUM_LIGHTS_4	(1<<4)
#define SDR_FLAG_NUM_LIGHTS_5	(1<<5)
#define SDR_FLAG_NUM_LIGHTS_6	(1<<6)
#define SDR_FLAG_NUM_LIGHTS_7	(1<<7)
#define SDR_FLAG_NUM_LIGHTS_8	(1<<8)
#define SDR_FLAG_FOG			(1<<9)
#define SDR_FLAG_DIFFUSE_MAP	(1<<10)
#define SDR_FLAG_GLOW_MAP		(1<<11)
#define SDR_FLAG_SPEC_MAP		(1<<12)
#define SDR_FLAG_NORMAL_MAP		(1<<13)
#define SDR_FLAG_HEIGHT_MAP		(1<<14)
#define SDR_FLAG_ENV_MAP		(1<<15)


#define MAX_SHADER_UNIFORMS		15

struct opengl_shader_file_t {
	char *vert;
	char *frag;

	int flags;

	int num_uniforms;
	char *uniforms[MAX_SHADER_UNIFORMS];
};

typedef struct opengl_shader_uniform_t {
	std::string text_id;
	GLint location;

	opengl_shader_uniform_t() : location(-1) {}
} opengl_shader_uniform_t;

typedef struct opengl_shader_t {
	GLhandleARB program_id;
	int flags;
	int flags2;

	SCP_vector<opengl_shader_uniform_t> uniforms;

	opengl_shader_t() :
		program_id(0), flags(0), flags2(0)
	{
	}
} opengl_shader_t;

extern SCP_vector<opengl_shader_t> GL_shader;

extern opengl_shader_t *Current_shader;

int opengl_shader_get_index(int flags);
void opengl_shader_set_current(opengl_shader_t *shader_obj = NULL);

void opengl_shader_init();
void opengl_shader_shutdown();

GLhandleARB opengl_shader_create(const char *vs, const char *fs);

void opengl_shader_init_uniform(const char *uniform_text);
GLint opengl_shader_get_uniform(const char *uniform_text);

#endif	// _GROPENGLSHADER_H
