#pragma once


#include "globalincs/pstypes.h"
#include "lighting/lighting.h"
#include "cmdline/cmdline.h"

//Variables
extern int Num_active_gr_lights;

extern const float gr_light_color[4];
extern const float gr_light_zero[4];
extern const float gr_light_emission[4];
extern float gr_light_ambient[4];
extern float gr_user_ambient;

//Functions
void gr_set_light(light *fs_light);
void gr_reset_lighting();
void gr_set_lighting(bool set, bool state);
void gr_set_center_alpha(int type);
void gr_set_ambient_light(int red, int green, int blue);

void gr_calculate_ambient_factor(int ambient_factor = Cmdline_ambient_factor);

void gr_lighting_fill_uniforms(void* data_out, size_t buffer_size);

void gr_light_init();
void gr_light_shutdown();
