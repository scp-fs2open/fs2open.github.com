/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _LIGHTING_H
#define _LIGHTING_H

// Light stuff works like this:
// At the start of the frame, call light_reset.
// For each light source, call light_add_??? functions.
// To calculate lighting, do:
// call light_filter_reset or light_filter.
// set up matrices with g3 functions
// call light_rotatate_all to rotate all valid
// lights into current coordinates.
// call light_apply to fill in lighting for a point.

#define LT_DIRECTIONAL	0		// A light like a sun
#define LT_POINT		1		// A point light, like an explosion
#define LT_TUBE			2		// A tube light, like a fluorescent light
#define LT_CONE			3		// A cone light, like a flood light

#define MAX_LIGHT_LEVELS 16

typedef struct light {
	int		type;							// What type of light this is
	vec3d	vec;							// location in world space of a point light or the direction of a directional light or the first point on the tube for a tube light
	vec3d	vec2;							// second point on a tube light or direction of a cone light
	vec3d	local_vec;						// rotated light vector
	vec3d	local_vec2;						// rotated 2nd light vector for a tube light
	float	intensity;						// How bright the light is.
	float	rada, rada_squared;				// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float	radb, radb_squared;				// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float	r,g,b;							// The color components of the light
	float	spec_r,spec_g,spec_b;			// The specular color components of the light
	int		light_ignore_objnum;			// Don't light this object.  Used to optimize weapons casting light on parents.
	int		affected_objnum;				// for "unique lights". ie, lights which only affect one object (trust me, its useful)
	float	cone_angle;						// angle for cone lights
	float	cone_inner_angle;				// the inner angle for calculating falloff
	bool	dual_cone;						// should the cone be shining in both directions?
	int instance;
} light;

extern SCP_vector<light*> Static_light;

struct light_indexing_info
{
	int index_start;
	int num_lights;
};

class scene_lights
{
	SCP_vector<light> AllLights;
	
	SCP_vector<int> StaticLightIndices;

	SCP_vector<int> FilteredLights;

	SCP_vector<int> BufferedLights;

	int current_light_index;
	int current_num_lights;
public:
	scene_lights()
	{
		resetLightState();
	}
	void addLight(const light *light_ptr);
	void setLightFilter(int objnum, const vec3d *pos, float rad);
	bool setLights(const light_indexing_info *info);
	void resetLightState();
	int getNumStaticLights();
	light_indexing_info bufferLights();
};

extern void light_reset();
extern void light_set_ambient(float ambient_light);

// Intensity - how strong the light is.  1.0 will cast light around 5meters or so.
// r,g,b - only used for colored lighting. Ignored currently.
extern void light_add_directional(const vec3d *dir, float intensity, float r, float g, float b, float spec_r = 0.0f, float spec_g = 0.0f, float spec_b = 0.0f, bool specular = false);
extern void light_add_point(const vec3d * pos, float r1, float r2, float intensity, float r, float g, float b, int light_ignore_objnum, float spec_r = 0.0f, float spec_g = 0.0f, float spec_b = 0.0f, bool specular = false);
extern void light_add_point_unique(const vec3d * pos, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum, float spec_r = 0.0f, float spec_g = 0.0f, float spec_b = 0.0f, bool specular = false);
extern void light_add_tube(const vec3d *p0, const vec3d *p1, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum, float spec_r = 0.0f, float spec_g = 0.0f, float spec_b = 0.0f, bool specular = false);
extern void light_add_cone(const vec3d * pos, const vec3d * dir, float angle, float inner_angle, bool dual_cone, float r1, float r2, float intensity, float r, float g, float b, int light_ignore_objnum, float spec_r = 0.0f, float spec_g = 0.0f, float spec_b = 0.0f, bool specular = false);
extern void light_rotate_all();

// Makes a list of only the lights that will affect
// the sphere specified by 'pos' and 'rad' and 'objnum'.
// Returns number of lights active.
extern int light_filter_push( int objnum, const vec3d *pos, float rad );
extern int light_filter_push_box(const vec3d *min, const vec3d *max);
extern void light_filter_pop();

// Applies light to a vertex.   In order for this to work, 
// it assumes that one of light_filter have been called.
// It only uses 'vert' to fill in it's light
// fields.  'pos' is position of point, 'norm' is the norm.
ubyte light_apply(const vec3d *pos, const vec3d *norm, float static_light_val);

void light_apply_specular(ubyte *param_r, ubyte *param_g, ubyte *param_b, const vec3d *pos, const vec3d * norm, const vec3d * cam);

// Same as above only does RGB.
void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, const vec3d *pos, const vec3d *norm, float static_light_val );

// return the # of global light sources
extern int light_get_global_count();

// Fills direction of global light source N in pos.
// Returns 0 if there is no global light.
extern int light_get_global_dir(vec3d *pos, int n);

// Set to non-zero if we're in a shadow.
extern void light_set_shadow( int state );

bool light_compare_by_type(const light &a, const light &b);
#endif
