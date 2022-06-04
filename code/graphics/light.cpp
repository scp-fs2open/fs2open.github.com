/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#include "light.h"

#include "globalincs/pstypes.h"

#include "matrix.h"

#include "cmdline/cmdline.h"
#include "graphics/2d.h"
#include "math/bitarray.h"
#include "render/3d.h"
#include "util/uniform_structs.h"

// Structures
struct gr_light
{
	vec4 Ambient;
	vec4 Diffuse;
	vec4 Specular;

	// light position
	vec4 Position;

	// spotlight direction (for tube lights)
	vec3d SpotDir;

	float SpotCutOff;
	float ConstantAtten, LinearAtten, QuadraticAtten;

	int type;
};

// Variables
SCP_vector<gr_light> gr_lights;
graphics::model_light gr_light_uniforms[graphics::MAX_UNIFORM_LIGHTS];

int Num_active_gr_lights = 0;

// OGL defaults
const float gr_light_color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
const float gr_light_zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float gr_light_emission[4] = { 0.09f, 0.09f, 0.09f, 1.0f };
float gr_light_ambient[4] = { 0.47f, 0.47f, 0.47f, 1.0f };
float gr_user_ambient = 0.0f;

void FSLight2GLLight(light* FSLight, gr_light* GLLight) {
	GLLight->Ambient.xyzw.x = 0.0f;
	GLLight->Ambient.xyzw.y = 0.0f;
	GLLight->Ambient.xyzw.z = 0.0f;
	GLLight->Ambient.xyzw.w = 1.0f;

	GLLight->Diffuse.xyzw.x = FSLight->r * FSLight->intensity;
	GLLight->Diffuse.xyzw.y = FSLight->g * FSLight->intensity;
	GLLight->Diffuse.xyzw.z = FSLight->b * FSLight->intensity;
	GLLight->Diffuse.xyzw.w = 1.0f;

	GLLight->Specular.xyzw.x = FSLight->spec_r * FSLight->intensity;
	GLLight->Specular.xyzw.y = FSLight->spec_g * FSLight->intensity;
	GLLight->Specular.xyzw.z = FSLight->spec_b * FSLight->intensity;
	GLLight->Specular.xyzw.w = 1.0f;

	GLLight->type = static_cast<int>(FSLight->type);

	// GL default values...
	// spot direction
	GLLight->SpotDir.xyz.x = 0.0f;
	GLLight->SpotDir.xyz.y = 0.0f;
	GLLight->SpotDir.xyz.z = -1.0f;
	// spot cutoff
	GLLight->SpotCutOff = 180.0f; // special value, light in all directions
	// defaults to disable attenuation
	GLLight->ConstantAtten = 1.0f;
	GLLight->LinearAtten = 0.0f;
	GLLight->QuadraticAtten = 0.0f;
	// position
	GLLight->Position.xyzw.x = FSLight->vec.xyz.x;
	GLLight->Position.xyzw.y = FSLight->vec.xyz.y;
	GLLight->Position.xyzw.z = FSLight->vec.xyz.z; // flipped axis for FS2
	GLLight->Position.xyzw.w = 1.0f;


	switch (FSLight->type) {
	case Light_Type::Point: {
		// this crap still needs work...
		GLLight->ConstantAtten = 1.0f;
		GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;

		GLLight->Specular.xyzw.x *= static_point_factor;
		GLLight->Specular.xyzw.y *= static_point_factor;
		GLLight->Specular.xyzw.z *= static_point_factor;

		break;
	}

	case Light_Type::Tube: {
		GLLight->ConstantAtten = 1.0f;
		GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;
		GLLight->QuadraticAtten = (1.0f / MAX(FSLight->rada_squared, FSLight->radb_squared)) * 1.25f;

		GLLight->Specular.xyzw.x *= static_tube_factor;
		GLLight->Specular.xyzw.y *= static_tube_factor;
		GLLight->Specular.xyzw.z *= static_tube_factor;

		GLLight->Position.xyzw.x = FSLight->vec2.xyz.x; // Valathil: Use endpoint of tube as light position
		GLLight->Position.xyzw.y = FSLight->vec2.xyz.y;
		GLLight->Position.xyzw.z = FSLight->vec2.xyz.z;
		GLLight->Position.xyzw.w = 1.0f;

		// Valathil: When using shaders pass the beam direction (not normalized IMPORTANT for calculation of tube)
		vec3d a;
		vm_vec_sub(&a, &FSLight->vec2, &FSLight->vec);
		GLLight->SpotDir.xyz.x = a.xyz.x;
		GLLight->SpotDir.xyz.y = a.xyz.y;
		GLLight->SpotDir.xyz.z = a.xyz.z;
		GLLight->SpotCutOff = 90.0f; // Valathil: So shader dectects tube light
		break;
	}

	case Light_Type::Directional: {
		GLLight->Position.xyzw.x = -FSLight->vec.xyz.x;
		GLLight->Position.xyzw.y = -FSLight->vec.xyz.y;
		GLLight->Position.xyzw.z = -FSLight->vec.xyz.z;
		GLLight->Position.xyzw.w = 0.0f; // This is a direction so the w part must be 0

		GLLight->Specular.xyzw.x *= static_light_factor;
		GLLight->Specular.xyzw.y *= static_light_factor;
		GLLight->Specular.xyzw.z *= static_light_factor;

		break;
	}

	case Light_Type::Cone:
		break;

	default:
		Error(LOCATION, "Unknown light type in FSLight2GLLight. Expected was 0, 1, 2 or 3, we got %i", static_cast<int>(FSLight->type));
		break;
	}
}

static void set_light(int light_num, gr_light* ltp) {
	Assert(light_num < (int)graphics::MAX_UNIFORM_LIGHTS);

	vm_vec_transform(&gr_light_uniforms[light_num].position, &ltp->Position, &gr_view_matrix);
	vm_vec_transform(&gr_light_uniforms[light_num].direction, &ltp->SpotDir, &gr_view_matrix, false);

	gr_light_uniforms[light_num].diffuse_color = vm_vec4_to_vec3(ltp->Diffuse);

	gr_light_uniforms[light_num].spec_color = vm_vec4_to_vec3(ltp->Specular);

	gr_light_uniforms[light_num].light_type = ltp->type;

	gr_light_uniforms[light_num].attenuation = ltp->LinearAtten;
}

static bool sort_active_lights(const gr_light& la, const gr_light& lb) {
	// directional lights always go first
	if ((la.type != LT_DIRECTIONAL) && (lb.type == LT_DIRECTIONAL)) {
		return false;
	} else if ((la.type == LT_DIRECTIONAL) && (lb.type != LT_DIRECTIONAL)) {
		return true;
	}

	// tube lights go next, they are generally large and intense
	if ((la.type != LT_TUBE) && (lb.type == LT_TUBE)) {
		return false;
	} else if ((la.type == LT_TUBE) && (lb.type != LT_TUBE)) {
		return true;
	}

	// everything else is sorted by linear atten (light size)
	// NOTE: smaller atten is larger light radius!
	if (la.LinearAtten > lb.LinearAtten) {
		return false;
	} else if (la.LinearAtten < lb.LinearAtten) {
		return true;
	}

	// as one extra check, if we're still here, go with overall brightness of light

	float la_value = la.Diffuse.xyzw.x + la.Diffuse.xyzw.y + la.Diffuse.xyzw.z;
	float lb_value = lb.Diffuse.xyzw.x + lb.Diffuse.xyzw.y + lb.Diffuse.xyzw.z;

	if (la_value < lb_value) {
		return false;
	} else if (la_value > lb_value) {
		return true;
	}

	// the two are equal
	return false;
}

static void pre_render_init_lights() {
	// sort the lights to try and get the most visible lights on the first pass
	std::sort(gr_lights.begin(), gr_lights.end(), sort_active_lights);
}

void gr_set_light(light* fs_light) {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	// init the light
	gr_light grl;
	FSLight2GLLight(fs_light, &grl);
	gr_lights.push_back(grl);
	Num_active_gr_lights++;
}

void gr_set_center_alpha(int type) {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	if (!type) {
		return;
	}

	gr_light glight;

	vec3d dir;
	vm_vec_sub(&dir, &Eye_position, &Object_position);
	vm_vec_normalize(&dir);

	if (type == 1) {
		glight.Diffuse.xyzw.x = 0.0f;
		glight.Diffuse.xyzw.y = 0.0f;
		glight.Diffuse.xyzw.z = 0.0f;
		glight.Ambient.xyzw.x = gr_screen.current_alpha;
		glight.Ambient.xyzw.y = gr_screen.current_alpha;
		glight.Ambient.xyzw.z = gr_screen.current_alpha;
	}
	else {
		glight.Diffuse.xyzw.x = gr_screen.current_alpha;
		glight.Diffuse.xyzw.y = gr_screen.current_alpha;
		glight.Diffuse.xyzw.z = gr_screen.current_alpha;
		glight.Ambient.xyzw.x = 0.0f;
		glight.Ambient.xyzw.y = 0.0f;
		glight.Ambient.xyzw.z = 0.0f;
	}
	glight.type = type;

	glight.Specular.xyzw.x = 0.0f;
	glight.Specular.xyzw.y = 0.0f;
	glight.Specular.xyzw.z = 0.0f;
	glight.Specular.xyzw.w = 0.0f;

	glight.Ambient.xyzw.w = 1.0f;
	glight.Diffuse.xyzw.w = 1.0f;

	glight.Position.xyzw.x = -dir.xyz.x;
	glight.Position.xyzw.y = -dir.xyz.y;
	glight.Position.xyzw.z = -dir.xyz.z;
	glight.Position.xyzw.w = 0.0f;

	// defaults
	glight.SpotDir.xyz.x = 0.0f;
	glight.SpotDir.xyz.y = 0.0f;
	glight.SpotDir.xyz.z = -1.0f;
	glight.SpotCutOff = 180.0f;
	glight.ConstantAtten = 1.0f;
	glight.LinearAtten = 0.0f;
	glight.QuadraticAtten = 0.0f;

	// first light
	gr_lights.push_back(glight);

	// second light
	glight.Position.xyzw.x = dir.xyz.x;
	glight.Position.xyzw.y = dir.xyz.y;
	glight.Position.xyzw.z = dir.xyz.z;

	gr_lights.push_back(glight);
}

void gr_reset_lighting() {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	gr_lights.clear();
	Num_active_gr_lights = 0;
}

void gr_calculate_ambient_factor(int ambient_factor) {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	gr_user_ambient = (float) ((ambient_factor * 2) - 255) / 255.0f;
}

void gr_light_shutdown() {
	gr_lights.clear();
}

void gr_light_init() {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	gr_calculate_ambient_factor();

	// allocate memory for enabled lights
	gr_lights.reserve(1024);
}

void gr_set_lighting() {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	//Valathil: Sort lights by priority
	extern bool Deferred_lighting;
	if (!Deferred_lighting) {
		pre_render_init_lights();
	}

	int i = 0;

	for (i = 0; i < (int)graphics::MAX_UNIFORM_LIGHTS; i++) {
		if (i >= Num_active_gr_lights) {
			break;
		}

		if (gr_lights[i].type != LT_DIRECTIONAL)
			continue;

		set_light(i, &gr_lights[i]);
	}

	gr_light zero;
	memset(&zero, 0, sizeof(gr_light));
	zero.Position.xyzw.x = 1.0f;

	// make sure that we turn off any lights that we aren't using right now
	for (; i < (int)graphics::MAX_UNIFORM_LIGHTS; i++) {
		set_light(i, &zero);
	}
}

void gr_set_ambient_light(int red, int green, int blue) {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	gr_light_ambient[0] = i2fl(red) / 255.0f;
	gr_light_ambient[1] = i2fl(green) / 255.0f;
	gr_light_ambient[2] = i2fl(blue) / 255.0f;
	gr_light_ambient[3] = 1.0f;

	gr_calculate_ambient_factor();
}

void gr_lighting_fill_uniforms(void* data_out, size_t buffer_size) {
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	Assertion(sizeof(gr_light_uniforms) <= buffer_size, "Insufficient buffer supplied.");

	memcpy(reinterpret_cast<graphics::model_light*>(data_out), gr_light_uniforms, sizeof(gr_light_uniforms));
}
