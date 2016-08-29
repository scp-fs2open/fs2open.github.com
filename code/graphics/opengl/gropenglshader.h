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
#include "graphics/2d.h"
#include "graphics/opengl/gropengl.h"
#include "ShaderProgram.h"

#include <string>
#include <glad/glad.h>

enum shader_stage {
	SDR_STAGE_VERTEX,
	SDR_STAGE_FRAGMENT,
	SDR_STAGE_GEOMETRY
};

struct opengl_vert_attrib {
	enum attrib_id {
		POSITION,
		COLOR,
		TEXCOORD,
		NORMAL,
		TANGENT,
		MODEL_ID,
		RADIUS,
		UVEC,
		NUM_ATTRIBS
	};

	attrib_id attribute_id;
	SCP_string name;
	vec4 default_value;
};

extern opengl_vert_attrib GL_vertex_attrib_info[];

struct geometry_sdr_params
{
	int input_type;
	int output_type;
	int vertices_out;
};

struct opengl_shader_type_t {
	shader_type type_id;

	const char *vert;
	const char *frag;
	const char *geo;

	SCP_vector<const char*> uniforms;

	SCP_vector<opengl_vert_attrib::attrib_id> attributes;

	const char* description;
};

struct opengl_shader_variant_t {
	shader_type type_id;

	bool use_geometry_sdr;
	bool use_define;

	int flag;
	SCP_string flag_text;

	SCP_vector<const char*> uniforms;

	SCP_vector<opengl_vert_attrib::attrib_id> attributes;

	const char* description;
};

struct opengl_shader_file_t {
	const char *vert;
	const char *frag;
	const char *geo;

	int flags;

	SCP_vector<const char*> uniforms;

	SCP_vector<const char*> attributes;

	const char* description;
};

struct opengl_shader_uniform_reference_t {
	unsigned int flag;

	SCP_vector<const char*> uniforms;

	SCP_vector<const char*> attributes;

	SCP_vector<const char*> uniform_blocks;

	const char* name;
};

typedef struct opengl_shader_uniform_t {
	SCP_string text_id;
	GLint location;

	opengl_shader_uniform_t() : location(-1) {}
} opengl_shader_uniform_t;

typedef struct opengl_shader_t {
	opengl::ShaderProgram* program;

	SCP_vector<std::pair<SCP_string, bool>> variant_uniforms;

	shader_type shader;
	unsigned int flags;
	int flags2;

	opengl_shader_t() : program(nullptr), shader(SDR_TYPE_NONE), flags(0), flags2(0)
	{
	}
} opengl_shader_t;

extern SCP_vector<opengl_shader_t> GL_shader;

extern opengl_shader_t *Current_shader;

int gr_opengl_maybe_create_shader(shader_type shader_t, unsigned int flags);
void opengl_delete_shader(int sdr_handle);
void opengl_shader_set_current(opengl_shader_t *shader_obj = NULL);
void opengl_shader_set_current(int handle);

void opengl_shader_init();
void opengl_shader_shutdown();

int opengl_compile_shader(shader_type sdr, uint flags);

GLint opengl_shader_get_attribute(const char *attribute_text);

void opengl_program_check_info_log(GLuint program_object);
void opengl_shader_check_info_log(GLuint shader_object);

void opengl_shader_compile_deferred_light_shader();
void opengl_shader_compile_deferred_light_clear_shader();

void opengl_shader_compile_passthrough_shader();
void opengl_shader_set_passthrough(bool textured = true, bool alpha = false, vec4* clr = NULL, float color_scale = 1.0f);
void opengl_shader_set_passthrough(bool textured, bool alpha, color *clr);

#define ANIMATED_SHADER_LOADOUTSELECT_FS1	0
#define ANIMATED_SHADER_LOADOUTSELECT_FS2	1
#define ANIMATED_SHADER_CLOAK				2

#endif	// _GROPENGLSHADER_H
