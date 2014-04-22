/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "math/vecmat.h"
#include "render/3d.h"
#include "lighting/lighting.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"
#include "debugconsole/console.h"



#define MAX_LIGHT_LEVELS 16


int cell_shaded_lightmap = -1;

light Lights[MAX_LIGHTS];
int Num_lights=0;
extern int Cmdline_nohtl;

light *Relevent_lights[MAX_LIGHTS][MAX_LIGHT_LEVELS];
int Num_relevent_lights[MAX_LIGHT_LEVELS];
int Num_light_levels = 0;

SCP_vector<light*> Static_light;

static int Light_in_shadow = 0;	// If true, this means we're in a shadow

#define LM_BRIGHTEN  0
#define LM_DARKEN    1

#define MIN_LIGHT 0.03f	// When light drops below this level, ignore it.  Must be non-zero! (1/32)


int Lighting_off = 0;

// For lighting values, 0.75 is full intensity

#if 1		// ADAM'S new stuff
	int Lighting_mode = LM_BRIGHTEN;
	#define AMBIENT_LIGHT_DEFAULT		0.15f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.75f		//0.90f
#else
	int Lighting_mode = LM_DARKEN;
	#define AMBIENT_LIGHT_DEFAULT		0.75f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.50f		//0.90f
#endif

float Ambient_light = AMBIENT_LIGHT_DEFAULT;
float Reflective_light = REFLECTIVE_LIGHT_DEFAULT;

int Lighting_flag = 1;

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
		dc_printf( "light mode [light|darken]   Changes the lighting mode.\n" );
		dc_printf( "   Where 'light' means the global light adds light.\n");
		dc_printf( "   and 'darken' means the global light subtracts light.\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Ambient light is set to %.2f\n", Ambient_light );
		dc_printf( "Reflective light is set to %.2f\n", Reflective_light );
		dc_printf( "Dynamic lighting is: %s\n", (Lighting_flag?"on":"off") );
		switch( Lighting_mode ) {
		case LM_BRIGHTEN:
			dc_printf( "Lighting mode is: light\n" );
			break;
		case LM_DARKEN:
			dc_printf( "Lighting mode is: darken\n" );
			break;
		default:
			dc_printf( "Lighting mode is: UNKNOWN\n" );
		}
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
		Lighting_mode = LM_BRIGHTEN;
		Ambient_light = AMBIENT_LIGHT_DEFAULT;
		Reflective_light = REFLECTIVE_LIGHT_DEFAULT;
		Lighting_flag = 0;
	
	} else if (dc_optional_string("mode")) {
		dc_stuff_string_white(arg_str);
		if (arg_str == "light") {
			Lighting_mode = LM_BRIGHTEN;
	
		} else if (arg_str == "darken") {
			Lighting_mode = LM_DARKEN;
		
		} else {
			dc_printf(" Error: unknown light mode: '%s'\n", arg_str.c_str());
		}
	
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

	Num_lights = 0;
	light_filter_reset();
}
extern vec3d Object_position;

/**
 * Rotates the light into the current frame of reference
 *
 * @param l Light to rotate
 */
void light_rotate(light * l)
{
	switch( l->type )	{
	case LT_DIRECTIONAL:
		// Rotate the light direction into local coodinates
		
		vm_vec_rotate(&l->local_vec, &l->vec, &Light_matrix );
		break;
	
	case LT_POINT:	{
			vec3d tempv;
			// Rotate the point into local coordinates
	
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
		}
		break;
	
	case LT_TUBE:{
			vec3d tempv;

			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
			
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec2, &Light_base );
			vm_vec_rotate(&l->local_vec2, &tempv, &Light_matrix );
		}
		break;

	default:
		Int3();	// Invalid light type
	}
}

// Sets the ambient lighting level.
// Ignored for now.
void light_set_ambient(float ambient_light)
{
}

void light_add_directional( vec3d *dir, float intensity, float r, float g, float b, float spec_r, float spec_g, float spec_b, bool specular )
{
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if ( Num_lights >= MAX_LIGHTS ) return;

	l = &Lights[Num_lights++];

	l->type = LT_DIRECTIONAL;

	if ( Lighting_mode == LM_BRIGHTEN )	{
		vm_vec_copy_scale( &l->vec, dir, -1.0f );
	} else {
		vm_vec_copy_scale( &l->vec, dir, 1.0f );
	}

	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rada = 0.0f;
	l->radb = 0.0f;
	l->rada_squared = l->rada*l->rada;
	l->radb_squared = l->radb*l->radb;
	l->light_ignore_objnum = -1;
	l->affected_objnum = -1;
	l->instance = Num_lights-1;
		
	Assert( Num_light_levels <= 1 );

	Static_light.push_back(l);
}


void light_add_point( vec3d * pos, float r1, float r2, float intensity, float r, float g, float b, int light_ignore_objnum, float spec_r, float spec_g, float spec_b, bool specular  )
{
	Assertion( r1 > 0.0f, "Invalid radius r1 specified for light: %d. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r1 );
	Assertion( r2 > 0.0f, "Invalid radius r2 specified for light: %d. Radius must be > 0.0f. Examine stack trace to determine culprit.\n", r2 );

	if (r1 < 0.0001f || r2 < 0.0001f)
		return;

	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}

	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rada = r1;
	l->radb = r2;
	l->rada_squared = l->rada*l->rada;
	l->radb_squared = l->radb*l->radb;
	l->light_ignore_objnum = light_ignore_objnum;
	l->affected_objnum = -1;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
}

void light_add_point_unique( vec3d * pos, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum, float spec_r, float spec_g, float spec_b, bool specular )
{
	Assert(r1 >0);
	Assert(r2 >0);
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rada = r1;
	l->radb = r2;
	l->rada_squared = l->rada*l->rada;
	l->radb_squared = l->radb*l->radb;
	l->light_ignore_objnum = -1;
	l->affected_objnum = affected_objnum;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
}

// beams affect every ship except the firing ship
extern int Use_GLSL;
void light_add_tube(vec3d *p0, vec3d *p1, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum, float spec_r, float spec_g, float spec_b, bool specular )
{
	Assert(r1 >0);
	Assert(r2 >0);
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_TUBE;
	l->vec = *p0;
	l->vec2 = *p1;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rada = r1;
	l->radb = r2;
	l->rada_squared = l->rada*l->rada;
	l->radb_squared = l->radb*l->radb;
	l->light_ignore_objnum = Use_GLSL>1 ?affected_objnum : -1;
	l->affected_objnum = Use_GLSL>1 ? -1 : affected_objnum;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
}

/**
 * Reset the list of lights to point to all lights.
 */
void light_filter_reset()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	Num_light_levels = 1;

	int n = Num_light_levels-1;
	Num_relevent_lights[n] = 0;

	l = Lights;
	for (i=0; i<Num_lights; i++, l++ )	{
		Relevent_lights[Num_relevent_lights[n]++][n] = l;
	}
}


/**
 * Makes a list of only the lights that will affect
 * the sphere specified by 'pos' and 'rad' and 'objnum'
 *
 * @param objnum    Object number
 * @param pos       World position
 * @param rad       Radius
 */
int light_filter_push( int objnum, vec3d *pos, float rad )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	light_filter_reset();

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;
	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				// if this is a "unique" light source, it only affects one guy
				if(l->affected_objnum >= 0){
					if(objnum == l->affected_objnum){
						vec3d to_light;
						float dist_squared, max_dist_squared;
						vm_vec_sub( &to_light, &l->vec, pos );
						dist_squared = vm_vec_mag_squared(&to_light);

						max_dist_squared = l->radb+rad;
						max_dist_squared *= max_dist_squared;
						
						if ( dist_squared < max_dist_squared )	{
							Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
						}
					}
				}
				// otherwise check all relevant objects
				else {
                    vec3d to_light;
                    float dist_squared, max_dist_squared;
                    vm_vec_sub( &to_light, &l->vec, pos );
                    dist_squared = vm_vec_mag_squared(&to_light);

                    max_dist_squared = l->radb+rad;
                    max_dist_squared *= max_dist_squared;
						
                    if ( dist_squared < max_dist_squared )	{
                        Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
                    }
				}
			}
			break;

		// hmm. this could probably be more optimal
		case LT_TUBE:
			if(Use_GLSL > 1) {
				if(l->light_ignore_objnum != objnum){
					vec3d nearest;
					float dist_squared, max_dist_squared;
					vm_vec_dist_squared_to_line(pos,&l->vec,&l->vec2,&nearest,&dist_squared);

					max_dist_squared = l->radb+rad;
					max_dist_squared *= max_dist_squared;
					
					if ( dist_squared < max_dist_squared )
						Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
				}
 			}
			else {
 				// all tubes are "unique" light sources for now
				if((l->affected_objnum >= 0) && (objnum == l->affected_objnum))
					Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

int is_inside( vec3d *min, vec3d *max, vec3d * p0, float rad )
{
	float *origin = (float *)&p0->xyz.x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] - rad )	{
			return 0;
		} else if (origin[i] > maxB[i] + rad )	{
			return 0;
		}
	}
	return 1;
}


int light_filter_push_box( vec3d *min, vec3d *max )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;

	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				if ( is_inside( min, max, &l->local_vec, l->radb ) )	{
					Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
				}
			}
			break;

		case LT_TUBE:
			if ( is_inside(min, max, &l->local_vec, l->radb) || is_inside(min, max, &l->local_vec2, l->radb) )	{
				Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

void light_filter_pop()
{
	if ( Lighting_off ) return;

	Num_light_levels--;
	Assert( Num_light_levels > 0 );
}

int l_num_points=0, l_num_lights=0;


void light_rotate_all()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	int n = Num_light_levels-1;

	l = Lights;
	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];
		light_rotate(l);
	}

	for (i = 0; i < (int)Static_light.size(); i++) {
		light_rotate(Static_light[i]);
	}
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
		*pos = Static_light[n]->vec;

		if ( Lighting_mode != LM_DARKEN )	{
			vm_vec_scale( pos, -1.0f );
		}
	}
	return 1;
}


void light_set_shadow( int state )
{
	Light_in_shadow = state;
}


void light_set_all_relevent()
{
	int idx;

	gr_reset_lighting();

	for (idx = 0; idx < (int)Static_light.size(); idx++)
		gr_set_light( Static_light[idx] );

	// for simplicity sake were going to forget about dynamic lights for the moment

	int n = Num_light_levels-1;

	for (idx = 0; idx < Num_relevent_lights[n]; idx++ )
		gr_set_light( Relevent_lights[idx][n] );
}


ubyte light_apply( vec3d *pos, vec3d * norm, float static_light_level )
{
	int i, idx;
	float lval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		return ubyte(fl2i(static_light_level*255.0f));
	}

	if ( Lighting_off ) return 191;

	// Factor in ambient light
	lval = Ambient_light;
	
	// Factor in light from suns if there are any
	if ( !Light_in_shadow ){
		// apply all sun lights
		for (idx = 0; idx < (int)Static_light.size(); idx++) {
			float ltmp;

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )
					lval += ltmp;
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )
					lval -= ltmp;

				if ( lval < 0.0f ) 
					lval = 0.0f;
				break;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
    CLAMP(lval, 0.0f, 0.75f);

	lval *= static_light_level;

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];

		vec3d to_light;
		float dot, dist;
		vm_vec_sub( &to_light, &l->local_vec, pos );
		dot = vm_vec_dot(&to_light, norm );
		if ( dot > 0.0f )	{
			dist = vm_vec_mag_squared(&to_light);
			if ( dist < l->rada_squared )	{
				lval += l->intensity*dot;
			} else if ( dist < l->radb_squared )	{
				// dist from 0 to 
				float nnum = dist - l->rada_squared;
				float dden = l->radb_squared - l->rada_squared;
				float ltmp = (1.0f - nnum / dden )*dot*l->intensity;
				lval += ltmp;
			}
			if ( lval > 1.0f ) {
				return 255;
			}
		}
	}

	return ubyte(fl2i(lval*255.0f));
}

int spec = 0;
float static_light_factor = 1.0f;
float static_tube_factor = 1.0f;
float static_point_factor = 1.0f;
double specular_exponent_value = 16.0;

void light_apply_specular(ubyte *param_r, ubyte *param_g, ubyte *param_b, vec3d *pos, vec3d * norm, vec3d * cam){

	light *l;
	float rval = 0, gval = 0, bval = 0;
	int idx;

	if ( !Cmdline_spec ) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	if (Detail.lighting==0) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	if ( Lighting_off ) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	vec3d V, N;
	vm_vec_sub(&V, cam,pos);
	vm_vec_normalize(&V);

	N = *norm;
	vm_vec_normalize(&N);

	// Factor in light from sun if there is one
		// apply all sun lights
		for (idx = 0; idx < (int)Static_light.size(); idx++) {
			float ltmp;

			vec3d R;
			vm_vec_sub(&R,&V, &Static_light[idx]->local_vec);
			vm_vec_normalize(&R);

			ltmp = (float)pow((double)vm_vec_dot(&R, &N ), specular_exponent_value) * Static_light[idx]->intensity * static_light_factor;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )	{
					rval += Static_light[idx]->spec_r * ltmp;
					gval += Static_light[idx]->spec_g * ltmp;
					bval += Static_light[idx]->spec_b * ltmp;
				}
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )	{
					rval -= ltmp; if ( rval < 0.0f ) rval = 0.0f;
					gval -= ltmp; if ( gval < 0.0f ) gval = 0.0f;
					bval -= ltmp; if ( bval < 0.0f ) bval = 0.0f; 
				}
				break;
			}
		}


    CLAMP(rval, 0.0f, 0.75f);
    CLAMP(gval, 0.0f, 0.75f);
    CLAMP(bval, 0.0f, 0.75f);

	//dynamic lights

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	vec3d to_light;
	float dot, dist;
	vec3d temp;
	float factor = 1.0f;
	for (idx = 0; idx < Num_relevent_lights[n]; idx++) {
		l = Relevent_lights[idx][n];

		dist = -1.0f;
		switch(l->type){
		// point lights
		case LT_POINT:			
			vm_vec_sub( &to_light, &l->local_vec, pos );
			factor = static_point_factor;
			break;

		// tube lights
		case LT_TUBE:						
			if(vm_vec_dist_to_line(pos, &l->local_vec, &l->local_vec2, &temp, &dist) != 0){
				continue;
			}
			factor = static_tube_factor;
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		// others. BAD
		default:
			Int3();
		}

		vec3d R;
		vm_vec_normalize(&to_light);
		vm_vec_add(&R,&V, &to_light);
		vm_vec_normalize(&R);

		dot = (float)pow((double)vm_vec_dot(&R, &N ), specular_exponent_value) * l->intensity * factor;		// reflective light
	
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l->rada_squared )	{
				float ratio;
				ratio = l->intensity*dot*factor;
				ratio *= 0.25f;
				rval += l->spec_r*ratio;
				gval += l->spec_g*ratio;
				bval += l->spec_b*ratio;
			} else if ( dist < l->radb_squared )	{
				float ratio;
				// dist from 0 to 
				float nnum = dist - l->rada_squared;
				float dden = l->radb_squared - l->rada_squared;
				ratio = (1.0f - nnum / dden)*dot*l->intensity*factor;
				ratio *= 0.25f;
				rval += l->spec_r*ratio;
				gval += l->spec_g*ratio;
				bval += l->spec_b*ratio;
			}
		}
	}

    CLAMP(rval, 0.0f, 1.0f);
    CLAMP(gval, 0.0f, 1.0f);
    CLAMP(bval, 0.0f, 1.0f);

	*param_r = ubyte(fl2i(rval*254.0f));
	*param_g = ubyte(fl2i(gval*254.0f));
	*param_b = ubyte(fl2i(bval*254.0f));
}

void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, vec3d *pos, vec3d * norm, float static_light_level )
{
	int idx;
	float rval, gval, bval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		ubyte lVal = ubyte(fl2i(static_light_level*255.0f));
		*param_r = lVal;
		*param_g = lVal;
		*param_b = lVal;
		return;
	}

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
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )	{
					rval += Static_light[idx]->r * ltmp;
					gval += Static_light[idx]->g * ltmp;
					bval += Static_light[idx]->b * ltmp;
				}
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )	{
					rval -= ltmp; if ( rval < 0.0f ) rval = 0.0f;
					gval -= ltmp; if ( gval < 0.0f ) gval = 0.0f;
					bval -= ltmp; if ( bval < 0.0f ) bval = 0.0f; 
				}
				break;
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

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	vec3d to_light;
	float dot, dist;
	vec3d temp;
	for (idx = 0; idx < Num_relevent_lights[n]; idx++) {
		l = Relevent_lights[idx][n];

		dist = -1.0f;
		switch(l->type){
		// point lights
		case LT_POINT:			
			vm_vec_sub( &to_light, &l->local_vec, pos );
			break;

		// tube lights
		case LT_TUBE:						
			if(vm_vec_dist_to_line(pos, &l->local_vec, &l->local_vec2, &temp, &dist) != 0){
				continue;
			}
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		// others. BAD
		default:
			Int3();
		}

		dot = vm_vec_dot(&to_light, norm);
	//		dot = 1.0f;
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l->rada_squared )	{
				float ratio;
				ratio = l->intensity*dot;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
			} else if ( dist < l->radb_squared )	{
				float ratio;
				// dist from 0 to 
				float nnum = dist - l->rada_squared;
				float dden = l->radb_squared - l->rada_squared;
				ratio = (1.0f - nnum / dden)*dot*l->intensity;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
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
