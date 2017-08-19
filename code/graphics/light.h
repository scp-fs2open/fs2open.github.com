#pragma once


#include "globalincs/pstypes.h"
#include "lighting/lighting.h"

struct gr_light_uniform_data {
	vec4 *Position;
	vec3d *Diffuse_color;
	vec3d *Spec_color;
	vec3d *Direction;
	int *Light_type;
	float *Attenuation;
};

extern gr_light_uniform_data gr_light_uniforms;

//Variables
extern const int gr_max_lights;
extern int Num_active_gr_lights;

extern const float gr_light_color[4];
extern const float gr_light_zero[4];
extern const float gr_light_emission[4];
extern float gr_light_ambient[4];

//Functions
void gr_set_light(light *fs_light);
void gr_reset_lighting();
void gr_set_lighting(bool set, bool state);
void gr_set_center_alpha(int type);
void gr_set_ambient_light(int red, int green, int blue);

void gr_light_init();
void gr_light_shutdown();
