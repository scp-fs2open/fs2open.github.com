/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifndef _GROPENGLLIGHT_H
#define _GROPENGLLIGHT_H

#include "gropengl.h"
#include "lighting/lighting.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

// Structures
struct opengl_light
{
	GLfloat Ambient[4], Diffuse[4], Specular[4];

	// light position
	GLfloat Position[4];

	// spotlight direction (for tube lights)
	GLfloat SpotDir[3];

	float SpotExp, SpotCutOff;
	float ConstantAtten, LinearAtten, QuadraticAtten;

	bool occupied;
	int type;
};

struct opengl_light_uniform_data {
	glm::vec4 *Position;
	vec3d *Diffuse_color;
	vec3d *Spec_color;
	vec3d *Direction;
	int *Light_type;
	float *Attenuation;
};

extern opengl_light_uniform_data opengl_light_uniforms;

//Variables
extern bool lighting_is_enabled;
extern const GLint GL_max_lights;
extern int Num_active_gl_lights;

extern const float GL_light_color[4];
extern const float GL_light_spec[4];
extern const float GL_light_zero[4];
extern const float GL_light_emission[4];
extern const float GL_light_true_zero[4];
extern float GL_light_ambient[4];

//Functions
void gr_opengl_set_light(light *fs_light);
void gr_opengl_reset_lighting();
void gr_opengl_set_lighting(bool set, bool state);
void gr_opengl_set_center_alpha(int type);
void gr_opengl_set_ambient_light(int red, int green, int blue);

void opengl_change_active_lights(int pos, int d_offset = 0);
void opengl_light_init();
void opengl_light_shutdown();

#endif //_GROPENGLLIGHT_H
