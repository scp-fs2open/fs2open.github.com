/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/color.h"
#include "graphics/light.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "math/vecmat.h"
#include "model/modelrender.h"
#include "render/3d.h"


SCP_vector<light> Lights;
SCP_vector<light> Static_light;

static int Light_in_shadow = 0;	// If true, this means we're in a shadow

#define MIN_LIGHT 0.03f	// When light drops below this level, ignore it.  Must be non-zero! (1/32)

static int Lighting_off = 0;

// For lighting values, 0.75 is full intensity
#define AMBIENT_LIGHT_DEFAULT		0.15f		//0.10f
#define REFLECTIVE_LIGHT_DEFAULT 0.75f		//0.90f

static float Ambient_light = AMBIENT_LIGHT_DEFAULT;
static float Reflective_light = REFLECTIVE_LIGHT_DEFAULT;

int Lighting_flag = 1;
int Num_lights = 0;

DCF(light,"Changes lighting parameters")
{
	SCP_string arg_str;
	float val_f;
	bool  val_b;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: light keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "light on|off          Turns all lighting on/off\n" );
		dc_printf( "light default         Resets lighting to all default values\n" );
		dc_printf( "light ambient X       Where X is the ambient light between 0 and 1.0\n" );
		dc_printf( "light reflect X       Where X is the material reflectiveness between 0 and 1.0\n" );
		dc_printf( "light dynamic [bool]  Toggles dynamic lighting on/off\n" );
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Ambient light is set to %.2f\n", Ambient_light );
		dc_printf( "Reflective light is set to %.2f\n", Reflective_light );
		dc_printf( "Dynamic lighting is: %s\n", (Lighting_flag?"on":"off") );
		return;
	}
	
	if (dc_optional_string("ambient")) {
		dc_stuff_float(&val_f);
		if ((val_f < 0.0f) || (val_f > 1.0f)) {
			dc_printf(" Error: ambient value must be between 0.0 and 1.0\n");
		} else {
			Ambient_light = val_f;
		}
	
	} else if (dc_optional_string("reflect")) {
		dc_stuff_float(&val_f);
		if ( (val_f < 0.0f) || (val_f > 1.0f))	{
			dc_printf(" Error: reflect value mus be between 0.0 and 1.0\n");
		} else {
			Reflective_light = val_f;
		}
	
	} else if (dc_optional_string("default")) {
		Ambient_light = AMBIENT_LIGHT_DEFAULT;
		Reflective_light = REFLECTIVE_LIGHT_DEFAULT;
		Lighting_flag = 0;
	
	} else if (dc_optional_string("dynamic")) {
		dc_stuff_boolean(&val_b);
		Lighting_flag = val_b;

	} else if(dc_maybe_stuff_boolean(&Lighting_off)) {
		Lighting_off = !Lighting_off;

	} else {
		dc_stuff_string_white(arg_str);
		dc_printf("Error: Unknown argument '%s'\n", arg_str.c_str());
	}
}

void light_reset()
{
	Static_light.clear();
	Lights.clear();
	Num_lights = 0;
}
extern vec3d Object_position;

/**
 * Rotates the light into the current frame of reference
 *
 * @param l Light to rotate
 */
static void light_rotate(light * l)
{
	switch( l->type )	{
	case Light_Type::Directional:
		// Rotate the light direction into local coodinates
		
		vm_vec_rotate(&l->local_vec, &l->vec, &Light_matrix );
		break;
	
	case Light_Type::Point:	{
			vec3d tempv;
			// Rotate the point into local coordinates
	
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
		}
		break;
	
	case Light_Type::Tube:{
			vec3d tempv;

			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
			
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec2, &Light_base );
			vm_vec_rotate(&l->local_vec2, &tempv, &Light_matrix );
		}
		break;

	case Light_Type::Cone:
			break;

	default:
		Int3();	// Invalid light type
	}
}

void light_add_directional(const vec3d* dir, const hdr_color* new_color)
{
	Assert(new_color!= nullptr);
	light_add_directional(dir, new_color->i(), new_color->r(), new_color->g(), new_color->b());
}

void light_add_directional(const vec3d *dir, float intensity, float r, float g, float b)
{
	if (Lighting_off) return;

	Num_lights++;

	light l;

	l.type = Light_Type::Directional;

	vm_vec_copy_scale( &l.vec, dir, -1.0f );

	l.r = r;
	l.g = g;
	l.b = b;

	//Direcional lights not yet switched to use lighting profiles multipliers as that's part of a seperate project
	auto lp = lighting_profile::current();
	l.intensity = lp->directional_light_brightness.handle(intensity);
	l.intensity = lp->overall_brightness.handle(l.intensity);
	l.rada = 0.0f;
	l.radb = 0.0f;
	l.rada_squared = l.rada*l.rada;
	l.radb_squared = l.radb*l.radb;
	l.instance = Num_lights-1;

	Lights.push_back(l);
	Static_light.push_back(l);
}


void light_add_point(const vec3d* pos, float r1, float r2, const hdr_color* new_color)
{
	Assert(new_color!= nullptr);
	light_add_point(pos, r1, r2, new_color->i(), new_color->r(), new_color->g(), new_color->b());
}

void light_add_point(const vec3d *pos, float r1, float r2, float intensity, float r, float g, float b)
{
	Assertion( r1 > 0.0f, "Invalid radius r1 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r1 );
	Assertion( r2 > 0.0f, "Invalid radius r2 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r2 );

	if (Lighting_off) return;

	if (!Lighting_flag) return;

	light l;

	Num_lights++;
	l.type = Light_Type::Point;
	l.vec = *pos;
	l.r = r;
	l.g = g;
	l.b = b;

	//configurable global tuning of light qualities
	auto lp = lighting_profile::current();
	l.intensity = lp->point_light_brightness.handle(intensity);
	l.intensity = lp->overall_brightness.handle(l.intensity);
	l.rada = lp->point_light_radius.handle(r1);
	l.radb = lp->point_light_radius.handle(r2);
	l.rada_squared = l.rada*l.rada;
	l.radb_squared = l.radb*l.radb;
	l.instance = Num_lights-1;

	Lights.push_back(l);
}

void light_add_tube(const vec3d* p0, const vec3d* p1, float r1, float r2, const hdr_color* new_color)
{
	Assert(new_color!= nullptr);
	light_add_tube(p0, p1, r1, r2, new_color->i(), new_color->r(), new_color->g(), new_color->b());
}

void light_add_tube(const vec3d *p0, const vec3d *p1, float r1, float r2, float intensity, float r, float g, float b)
{
	Assertion(r1 > 0.0f, "Invalid radius r1 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r1);
	Assertion(r2 > 0.0f, "Invalid radius r2 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r2);

	if (Lighting_off) return;

	if (!Lighting_flag) return;

	light l;

	Num_lights++;

	l.type = Light_Type::Tube;
	l.vec = *p0;
	l.vec2 = *p1;
	l.r = r;
	l.g = g;
	l.b = b;

	//configurable global tuning of light qualities
	auto lp = lighting_profile::current();
	l.intensity = lp->tube_light_brightness.handle(intensity);
	l.intensity = lp->overall_brightness.handle(l.intensity);
	l.rada = lp->tube_light_radius.handle(r1);
	l.radb = lp->tube_light_radius.handle(r2);
	l.rada_squared = l.rada*l.rada;
	l.radb_squared = l.radb*l.radb;
	l.instance = Num_lights-1;

	Lights.push_back(l);
}


void light_rotate_all()
{
	if ( Lighting_off ) return;

	for (auto& l : Lights)
		light_rotate(&l);
}

/**
 * Return the # of global light sources
 */
int light_get_global_count()
{
	return (int)Static_light.size();
}

/**
 * Fills direction of global light source N in pos.
 *
 * @param pos   Position
 * @param n     Light source
 *
 * Returns 0 if there is no global light.
 */
int light_get_global_dir(vec3d *pos, int n)
{
	if ( (n < 0) || (n >= (int)Static_light.size()) ) {
		return 0;
	}

	if (pos) {
		*pos = Static_light[n].vec;
		vm_vec_scale( pos, -1.0f );
	}
	return 1;
}

void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, const vec3d *pos, const vec3d *norm, float static_light_level )
{
	int idx;
	float rval, gval, bval;

	if ( Lighting_off ) {
		*param_r = 255;
		*param_g = 255;
		*param_b = 255;
		return;
	}

	// Factor in ambient light
	rval = Ambient_light;
	gval = Ambient_light;
	bval = Ambient_light;

	// Factor in light from sun if there is one
	if ( !Light_in_shadow ){
		// apply all sun lights
		for (idx = 0; idx < (int)Static_light.size(); idx++) {
			float ltmp;

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx].local_vec, norm )*Static_light[idx].intensity*Reflective_light;		// reflective light

			if ( ltmp > 0.0f )	{
				rval += Static_light[idx].r * ltmp;
				gval += Static_light[idx].g * ltmp;
				bval += Static_light[idx].b * ltmp;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
	CLAMP(rval, 0.0f, 0.75f);
    CLAMP(gval, 0.0f, 0.75f);
    CLAMP(bval, 0.0f, 0.75f);

	rval *= static_light_level;
	gval *= static_light_level;
	bval *= static_light_level;

	vec3d to_light;
	float dot, dist;
	vec3d temp;
	for (auto l : Lights) {

		dist = -1.0f;
		switch(l.type){
		// point lights
		case Light_Type::Point:
			vm_vec_sub( &to_light, &l.local_vec, pos );
			break;

		// tube lights
		case Light_Type::Tube:
			if(vm_vec_dist_to_line(pos, &l.local_vec, &l.local_vec2, &temp, &dist) != 0){
				continue;
			}
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		case Light_Type::Directional:
			continue;

		case Light_Type::Cone:
			continue;

		// others. BAD
		default:
			Error("Unknown light type in light_apply_rgb!\n");
			continue;
		}

		dot = vm_vec_dot(&to_light, norm);
	//		dot = 1.0f;
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l.rada_squared )	{
				float ratio;
				ratio = l.intensity*dot;
				ratio *= 0.25f;
				rval += l.r*ratio;
				gval += l.g*ratio;
				bval += l.b*ratio;
			} else if ( dist < l.radb_squared )	{
				float ratio;
				// dist from 0 to 
				float nnum = dist - l.rada_squared;
				float dden = l.radb_squared - l.rada_squared;
				ratio = (1.0f - nnum / dden)*dot*l.intensity;
				ratio *= 0.25f;
				rval += l.r*ratio;
				gval += l.g*ratio;
				bval += l.b*ratio;
			}
		}
	}

	float m = rval;
	if ( gval > m ) m = gval;
	if ( bval > m ) m = bval;

	if ( m > 1.0f )	{
		float im = 1.0f / m;

		rval *= im;
		gval *= im;
		bval *= im;
	}
	
    CLAMP(rval, 0.0f, 1.0f);
    CLAMP(gval, 0.0f, 1.0f);
    CLAMP(bval, 0.0f, 1.0f);

	*param_r = ubyte(fl2i(rval*255.0f));
	*param_g = ubyte(fl2i(gval*255.0f));
	*param_b = ubyte(fl2i(bval*255.0f));
}


void light_add_cone(const vec3d *pos, const vec3d *dir, float angle, float inner_angle, bool dual_cone, float r1, float r2, const hdr_color* new_color)
{
	Assert(new_color!= nullptr);
	light_add_cone(pos, dir, angle, inner_angle, dual_cone, r1, r2, new_color->i(), new_color->r(), new_color->g(), new_color->b());
}

void light_add_cone(const vec3d *pos, const vec3d *dir, float angle, float inner_angle, bool dual_cone, float r1, float r2, float intensity, float r, float g, float b)
{
	Assertion( r1 > 0.0f, "Invalid radius r1 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r1 );
	Assertion( r2 > 0.0f, "Invalid radius r2 specified for light: %f. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r2 );

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

	light l;

	Num_lights++;

	l.type = Light_Type::Cone;
	l.vec = *pos;
	l.vec2= *dir;
	l.cone_angle = angle;
	l.cone_inner_angle = inner_angle;
	l.dual_cone = dual_cone;
	l.r = r;
	l.g = g;
	l.b = b;

	auto lp = lighting_profile::current();
	l.intensity = lp->cone_light_brightness.handle(intensity);
	l.intensity = lp->overall_brightness.handle(l.intensity);
	l.rada = lp->cone_light_radius.handle(r1);
	l.radb = lp->cone_light_radius.handle(r2);
	l.rada_squared = l.rada*l.rada;
	l.radb_squared = l.radb*l.radb;
	l.instance = Num_lights-1;

	Lights.push_back(l);
}

bool light_compare_by_type(const light &a, const light &b)
{
	return a.type < b.type;
}

void scene_lights::addLight(const light *light_ptr)
{
	Assert(light_ptr != NULL);

	AllLights.push_back(*light_ptr);

	if ( light_ptr->type == Light_Type::Directional ) {
		StaticLightIndices.push_back(AllLights.size() - 1);
	}
}

void scene_lights::setLightFilter(const vec3d *pos, float rad)
{
	size_t i = 0;
	// clear out current filtered lights
	FilteredLights.clear();

	for ( auto& l : AllLights ) {
		switch ( l.type ) {
			case Light_Type::Directional:
				++i;
				continue;
			case Light_Type::Point: {
				vec3d to_light;
				float dist_squared, max_dist_squared;
				vm_vec_sub( &to_light, &l.vec, pos );
				dist_squared = vm_vec_mag_squared(&to_light);

				max_dist_squared = l.radb+rad;
				max_dist_squared *= max_dist_squared;

				if ( dist_squared < max_dist_squared )	{
					FilteredLights.push_back(i);
				}
			}
			break;
			case Light_Type::Tube: {
				vec3d nearest;
				float dist_squared, max_dist_squared;
				vm_vec_dist_squared_to_line(pos,&l.vec,&l.vec2,&nearest,&dist_squared);

				max_dist_squared = l.radb+rad;
				max_dist_squared *= max_dist_squared;

				if ( dist_squared < max_dist_squared ) {
					FilteredLights.push_back(i);
				}
			}
			break;

			case Light_Type::Cone:
				break;

			default:
				break;
		}
		++i;
	}
}

light_indexing_info scene_lights::bufferLights()
{
	size_t i;

	light_indexing_info light_info;

	light_info.index_start = 0;
	light_info.num_lights = 0;

	// make sure that there are lights to bind?
	if ( FilteredLights.empty() ) {
		return light_info;
	}

	light_info.index_start = BufferedLights.size();
	
	for ( i = 0; i < FilteredLights.size(); ++i ) {
		BufferedLights.push_back(FilteredLights[i]);
	}

	light_info.num_lights = FilteredLights.size();

	return light_info;
}

void scene_lights::resetLightState()
{
	current_light_index = static_cast<size_t>(-1);
	current_num_lights = static_cast<size_t>(-1);
}

bool scene_lights::setLights(const light_indexing_info *info)
{
	if ( info->index_start == current_light_index && info->num_lights == current_num_lights ) {
		// don't need to set new lights since the ones requested to be set are currently set
		return false;
	}

	current_light_index = info->index_start;
	current_num_lights = info->num_lights;

	gr_reset_lighting();

	for ( size_t i = 0; i < StaticLightIndices.size(); ++i) {
		auto light_index = StaticLightIndices[i];
		
		gr_set_light( &AllLights[light_index] );
	}

	extern bool Deferred_lighting;
	if ( Deferred_lighting ) {
		gr_set_lighting();
		return false;
	}

	auto index_start = info->index_start;
	auto num_lights = info->num_lights;

	// check if there are any lights to actually set
	if ( num_lights <= 0 ) {
		gr_set_lighting();
		return false;
	}

	// we definitely shouldn't be exceeding the number of buffered lights
	Assert(index_start + num_lights <= BufferedLights.size());

	for ( size_t i = 0; i < num_lights; ++i ) {
		auto buffered_light_index = index_start + i;
		auto light_index = BufferedLights[buffered_light_index];

		gr_set_light(&AllLights[light_index]);
	}

	gr_set_lighting();

	return true;
}
