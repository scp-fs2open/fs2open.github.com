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
#define SDR_FLAG_FOG			(1<<1)
#define SDR_FLAG_DIFFUSE_MAP	(1<<2)
#define SDR_FLAG_GLOW_MAP		(1<<3)
#define SDR_FLAG_SPEC_MAP		(1<<4)
#define SDR_FLAG_NORMAL_MAP		(1<<5)
#define SDR_FLAG_HEIGHT_MAP		(1<<6)
#define SDR_FLAG_ENV_MAP		(1<<7)
#define SDR_FLAG_ANIMATED		(1<<8)
#define SDR_FLAG_SOFT_QUAD		(1<<9)
#define SDR_FLAG_DISTORTION		(1<<10)

#define MAX_SHADER_UNIFORMS		15

#define SDR_ATTRIB_RADIUS		0

#define MAX_SDR_ATTRIBUTES		1

struct opengl_shader_flag_t {
	char *vert;
	char *frag;

	int flags;
};

struct opengl_shader_uniform_reference_t {
	int flag;

	int num_uniforms;
	char* uniforms[MAX_SHADER_UNIFORMS];

	int num_attributes;
	char* attributes[MAX_SDR_ATTRIBUTES];
	SCP_string name;
};

typedef struct opengl_shader_uniform_t {
	SCP_string text_id;
	GLint location;

	opengl_shader_uniform_t() : location(-1) {}
} opengl_shader_uniform_t;

typedef struct opengl_shader_t {
	GLhandleARB program_id;
	int flags;
	int flags2;

	SCP_vector<opengl_shader_uniform_t> uniforms;
	SCP_vector<opengl_shader_uniform_t> attributes; // using the uniform data structure to keep track of vert attribs

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

void opengl_compile_main_shader(int flags);
GLhandleARB opengl_shader_create(const char *vs, const char *fs);

void opengl_shader_init_attribute(const char *attribute_text);
GLint opengl_shader_get_attribute(const char *attribute_text);

void opengl_shader_init_uniform(const char *uniform_text);
GLint opengl_shader_get_uniform(const char *uniform_text);

void opengl_shader_set_animated_effect(int effect);
int opengl_shader_get_animated_effect();
void opengl_shader_set_animated_timer(float timer);
float opengl_shader_get_animated_timer();

#define ANIMATED_SHADER_LOADOUTSELECT_FS1	0
#define ANIMATED_SHADER_LOADOUTSELECT_FS2	1
#define ANIMATED_SHADER_CLOAK				2

#endif	// _GROPENGLSHADER_H
