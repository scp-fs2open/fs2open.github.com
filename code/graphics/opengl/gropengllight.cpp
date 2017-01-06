/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>

#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include <globalincs/systemvars.h>
#include "graphics/2d.h"
#include "gropengllight.h"
#include "gropenglstate.h"
#include "gropengltnl.h"
#include "lighting/lighting.h"
#include "render/3d.h"



// Variables
opengl_light *opengl_lights = NULL;
opengl_light_uniform_data opengl_light_uniforms;

bool lighting_is_enabled = true;
int Num_active_gl_lights = 0;

extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;
extern double specular_exponent_value;
extern float Cmdline_ogl_spec;

const GLint GL_max_lights = 8;

// OGL defaults
const float GL_light_color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
const float GL_light_spec[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float GL_light_zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float GL_light_emission[4] = { 0.09f, 0.09f, 0.09f, 1.0f };
const float GL_light_true_zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float GL_light_ambient[4] = { 0.47f, 0.47f, 0.47f, 1.0f };

void FSLight2GLLight(light *FSLight, opengl_light *GLLight)
{
	GLLight->Ambient[0] = 0.0f;
	GLLight->Ambient[1] = 0.0f;
	GLLight->Ambient[2] = 0.0f;
	GLLight->Ambient[3] = 1.0f;

	GLLight->Diffuse[0] = FSLight->r * FSLight->intensity;
	GLLight->Diffuse[1] = FSLight->g * FSLight->intensity;
	GLLight->Diffuse[2] = FSLight->b * FSLight->intensity;
	GLLight->Diffuse[3] = 1.0f;

	GLLight->Specular[0] = FSLight->spec_r * FSLight->intensity;
	GLLight->Specular[1] = FSLight->spec_g * FSLight->intensity;
	GLLight->Specular[2] = FSLight->spec_b * FSLight->intensity;
	GLLight->Specular[3] = 1.0f;

	GLLight->type = FSLight->type;

	// GL default values...
	// spot direction
	GLLight->SpotDir[0] = 0.0f;
	GLLight->SpotDir[1] = 0.0f;
	GLLight->SpotDir[2] = -1.0f;
	// spot exponent
	GLLight->SpotExp = Cmdline_ogl_spec * 0.5f;
	// spot cutoff
	GLLight->SpotCutOff = 180.0f; // special value, light in all directions
	// defaults to disable attenuation
	GLLight->ConstantAtten = 1.0f;
	GLLight->LinearAtten = 0.0f;
	GLLight->QuadraticAtten = 0.0f;
	// position
	GLLight->Position[0] = FSLight->vec.xyz.x;
	GLLight->Position[1] = FSLight->vec.xyz.y;
	GLLight->Position[2] = FSLight->vec.xyz.z; // flipped axis for FS2
	GLLight->Position[3] = 1.0f;


	switch (FSLight->type) {
		case LT_POINT: {
			// this crap still needs work...
			GLLight->ConstantAtten = 1.0f;
			GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;

			GLLight->Specular[0] *= static_point_factor;
			GLLight->Specular[1] *= static_point_factor;
			GLLight->Specular[2] *= static_point_factor;

			break;
		}

		case LT_TUBE: {
			GLLight->ConstantAtten = 1.0f;
			GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;
			GLLight->QuadraticAtten = (1.0f / MAX(FSLight->rada_squared, FSLight->radb_squared)) * 1.25f;

			GLLight->Specular[0] *= static_tube_factor;
			GLLight->Specular[1] *= static_tube_factor;
			GLLight->Specular[2] *= static_tube_factor;

			GLLight->Position[0] = FSLight->vec2.xyz.x; // Valathil: Use endpoint of tube as light position
			GLLight->Position[1] = FSLight->vec2.xyz.y;
			GLLight->Position[2] = FSLight->vec2.xyz.z;
			GLLight->Position[3] = 1.0f;

			// Valathil: When using shaders pass the beam direction (not normalized IMPORTANT for calculation of tube)
			vec3d a;
			vm_vec_sub(&a, &FSLight->vec2, &FSLight->vec);
			GLLight->SpotDir[0] = a.xyz.x;
			GLLight->SpotDir[1] = a.xyz.y;
			GLLight->SpotDir[2] = a.xyz.z;
			GLLight->SpotCutOff = 90.0f; // Valathil: So shader dectects tube light
			break;
		}

		case LT_DIRECTIONAL: {
			GLLight->Position[0] = -FSLight->vec.xyz.x;
			GLLight->Position[1] = -FSLight->vec.xyz.y;
			GLLight->Position[2] = -FSLight->vec.xyz.z;
			GLLight->Position[3] = 0.0f; // Directional lights in OpenGL have w set to 0

			GLLight->Specular[0] *= static_light_factor;
			GLLight->Specular[1] *= static_light_factor;
			GLLight->Specular[2] *= static_light_factor;

			break;
		}
		
		case LT_CONE:
			break;

		default:
			Int3();
			break;
	}
}

void opengl_set_light(int light_num, opengl_light *ltp)
{
	Assert(light_num < GL_max_lights);
		
	vec4 light_pos_world;
	light_pos_world.xyzw.x = ltp->Position[0];
	light_pos_world.xyzw.y = ltp->Position[1];
	light_pos_world.xyzw.z = ltp->Position[2];
	light_pos_world.xyzw.w = ltp->Position[3];

	vec3d light_dir_world;
	light_dir_world.xyz.x = ltp->SpotDir[0];
	light_dir_world.xyz.y = ltp->SpotDir[1];
	light_dir_world.xyz.z = ltp->SpotDir[2];

	vm_vec_transform(&opengl_light_uniforms.Position[light_num], &light_pos_world, &GL_view_matrix);
	vm_vec_transform(&opengl_light_uniforms.Direction[light_num], &light_dir_world, &GL_view_matrix, false);

	opengl_light_uniforms.Diffuse_color[light_num].xyz.x = ltp->Diffuse[0];
	opengl_light_uniforms.Diffuse_color[light_num].xyz.y = ltp->Diffuse[1];
	opengl_light_uniforms.Diffuse_color[light_num].xyz.z = ltp->Diffuse[2];

	opengl_light_uniforms.Spec_color[light_num].xyz.x = ltp->Specular[0];
	opengl_light_uniforms.Spec_color[light_num].xyz.y = ltp->Specular[1];
	opengl_light_uniforms.Spec_color[light_num].xyz.z = ltp->Specular[2];

	opengl_light_uniforms.Light_type[light_num] = ltp->type;

	opengl_light_uniforms.Attenuation[light_num] = ltp->LinearAtten;
}

bool opengl_sort_active_lights(const opengl_light &la, const opengl_light &lb)
{
	// directional lights always go first
	if ( (la.type != LT_DIRECTIONAL) && (lb.type == LT_DIRECTIONAL) )
		return false;
	else if ( (la.type == LT_DIRECTIONAL) && (lb.type != LT_DIRECTIONAL) )
		return true;

	// tube lights go next, they are generally large and intense
	if ( (la.type != LT_TUBE) && (lb.type == LT_TUBE) )
		return false;
	else if ( (la.type == LT_TUBE) && (lb.type != LT_TUBE) )
		return true;

	// everything else is sorted by linear atten (light size)
	// NOTE: smaller atten is larger light radius!
	if ( la.LinearAtten > lb.LinearAtten )
		return false;
	else if ( la.LinearAtten < lb.LinearAtten )
		return true;

	// as one extra check, if we're still here, go with overall brightness of light

	float la_value = la.Diffuse[0] + la.Diffuse[1] + la.Diffuse[2];
	float lb_value = lb.Diffuse[0] + lb.Diffuse[1] + lb.Diffuse[2];

	if ( la_value < lb_value )
		return false;
	else if ( la_value > lb_value )
		return true;

	// the two are equal
	return false;
}

void opengl_pre_render_init_lights()
{
	// sort the lights to try and get the most visible lights on the first pass
	std::sort(opengl_lights, opengl_lights + Num_active_gl_lights, opengl_sort_active_lights);
}

void gr_opengl_set_light(light *fs_light)
{
	if (Num_active_gl_lights >= MAX_LIGHTS)
		return;

//if (fs_light->type == LT_POINT)
//	return;

	// init the light
	FSLight2GLLight(fs_light, &opengl_lights[Num_active_gl_lights]);
	opengl_lights[Num_active_gl_lights++].occupied = true;
}

void gr_opengl_set_center_alpha(int type)
{
	if (!type)
		return;

	if(Num_active_gl_lights == MAX_LIGHTS)
		return;
	opengl_light glight;

	vec3d dir;
	vm_vec_sub(&dir, &Eye_position, &Object_position);
	vm_vec_normalize(&dir);

	if (type == 1) {
		glight.Diffuse[0] = 0.0f;
		glight.Diffuse[1] = 0.0f;
		glight.Diffuse[2] = 0.0f;
		glight.Ambient[0] = gr_screen.current_alpha;
		glight.Ambient[1] = gr_screen.current_alpha;
		glight.Ambient[2] = gr_screen.current_alpha;
	} else {
		glight.Diffuse[0] = gr_screen.current_alpha;
		glight.Diffuse[1] = gr_screen.current_alpha;
		glight.Diffuse[2] = gr_screen.current_alpha;
		glight.Ambient[0] = 0.0f;
		glight.Ambient[1] = 0.0f;
		glight.Ambient[2] = 0.0f;
	}
	glight.type = type;

	glight.Specular[0] = 0.0f;
	glight.Specular[1] = 0.0f;
	glight.Specular[2] = 0.0f;
	glight.Specular[3] = 0.0f;

	glight.Ambient[3] = 1.0f;
	glight.Diffuse[3] = 1.0f;

	glight.Position[0] = -dir.xyz.x;
	glight.Position[1] = -dir.xyz.y;
	glight.Position[2] = -dir.xyz.z;
	glight.Position[3] = 0.0f;

	// defaults
	glight.SpotDir[0] = 0.0f;
	glight.SpotDir[1] = 0.0f;
	glight.SpotDir[2] = -1.0f;
	glight.SpotExp = Cmdline_ogl_spec * 0.5f;
	glight.SpotCutOff = 180.0f;
	glight.ConstantAtten = 1.0f;
	glight.LinearAtten = 0.0f;
	glight.QuadraticAtten = 0.0f;
	glight.occupied = false;

	// first light
	memcpy( &opengl_lights[Num_active_gl_lights], &glight, sizeof(opengl_light) );
	opengl_lights[Num_active_gl_lights++].occupied = true;

	// second light
	glight.Position[0] = dir.xyz.x;
	glight.Position[1] = dir.xyz.y;
	glight.Position[2] = dir.xyz.z;

	memcpy( &opengl_lights[Num_active_gl_lights], &glight, sizeof(opengl_light) );
	opengl_lights[Num_active_gl_lights++].occupied = true;
}

void gr_opengl_reset_lighting()
{
	int i;

	for (i = 0; i < GL_max_lights; i++) {
		opengl_lights[i].occupied = false;
	}

	Num_active_gl_lights = 0;
}

void opengl_calculate_ambient_factor()
{
	float amb_user = 0.0f;

	// assuming that the default is "128", just skip this if not a user setting
	if ( Cmdline_ambient_factor == 128 )
		return;

	amb_user = (float)((Cmdline_ambient_factor * 2) - 255) / 255.0f;

	// set the ambient light
	GL_light_ambient[0] += amb_user;
	GL_light_ambient[1] += amb_user;
	GL_light_ambient[2] += amb_user;

	CLAMP( GL_light_ambient[0], 0.02f, 1.0f );
	CLAMP( GL_light_ambient[1], 0.02f, 1.0f );
	CLAMP( GL_light_ambient[2], 0.02f, 1.0f );

	GL_light_ambient[3] = 1.0f;
}

void opengl_light_shutdown()
{
	if (opengl_lights != NULL) {
		vm_free(opengl_lights);
		opengl_lights = NULL;
	}

	if ( opengl_light_uniforms.Position != NULL ) {
		vm_free(opengl_light_uniforms.Position);
		opengl_light_uniforms.Position = NULL;
	}

	if ( opengl_light_uniforms.Diffuse_color != NULL ) {
		vm_free(opengl_light_uniforms.Diffuse_color);
		opengl_light_uniforms.Diffuse_color = NULL;
	}

	if ( opengl_light_uniforms.Spec_color != NULL ) {
		vm_free(opengl_light_uniforms.Spec_color);
		opengl_light_uniforms.Spec_color = NULL;
	}

	if ( opengl_light_uniforms.Direction != NULL ) {
		vm_free(opengl_light_uniforms.Direction);
		opengl_light_uniforms.Direction = NULL;
	}

	if ( opengl_light_uniforms.Light_type != NULL ) {
		vm_free(opengl_light_uniforms.Light_type);
		opengl_light_uniforms.Light_type = NULL;
	}

	if ( opengl_light_uniforms.Attenuation != NULL ) {
		vm_free(opengl_light_uniforms.Attenuation);
		opengl_light_uniforms.Attenuation = NULL;
	}
}

void opengl_light_init()
{
	opengl_calculate_ambient_factor();

	// allocate memory for enabled lights
	if ( opengl_lights == NULL )
		opengl_lights = (opengl_light *) vm_malloc(MAX_LIGHTS * sizeof(opengl_light), memory::quiet_alloc);

	if (opengl_lights == NULL)
		Error( LOCATION, "Unable to allocate memory for lights!\n");

	memset( opengl_lights, 0, MAX_LIGHTS * sizeof(opengl_light) );

	opengl_light_uniforms.Position = (vec4 *)vm_malloc(GL_max_lights * sizeof(vec4));
	opengl_light_uniforms.Diffuse_color = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Spec_color = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Direction = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Light_type = (int *)vm_malloc(GL_max_lights * sizeof(int));
	opengl_light_uniforms.Attenuation = (float *)vm_malloc(GL_max_lights * sizeof(float));
}

void gr_opengl_set_lighting(bool set, bool state)
{
	lighting_is_enabled = set;

	if ( !state ) {
		return;
	}

	//Valathil: Sort lights by priority
	extern bool Deferred_lighting;
	if ( !Deferred_lighting )
		opengl_pre_render_init_lights();
	
	int i = 0;

	for ( i = 0; i < GL_max_lights; i++ ) {
		if ( i >= Num_active_gl_lights ) {
			break;
		}

		if ( opengl_lights[i].occupied ) {
			opengl_set_light(i, &opengl_lights[i]);
		}
	}

	opengl_light zero;
	memset(&zero, 0, sizeof(opengl_light));
	zero.Position[0] = 1.0f;

	// make sure that we turn off any lights that we aren't using right now
	for ( ; i < GL_max_lights; i++ ) {
		opengl_set_light(i, &zero);
	}
}

void gr_opengl_set_ambient_light(int red, int green, int blue)
{
	GL_light_ambient[0] = i2fl(red)/255.0f;
	GL_light_ambient[1] = i2fl(green)/255.0f;
	GL_light_ambient[2] = i2fl(blue)/255.0f;
	GL_light_ambient[3] = 1.0f;

	opengl_calculate_ambient_factor();
}
