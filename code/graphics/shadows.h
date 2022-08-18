/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _SHADOWS_H
#define _SHADOWS_H

#include "globalincs/pstypes.h"
#include "object/object.h"

#define MAX_SHADOW_CASCADES 4

struct light_frustum_info
{
	matrix4 proj_matrix;

	vec3d min;
	vec3d max;

	float start_dist;
};

enum class ShadowQuality { Disabled = 0, Low = 1, Medium = 2, High = 3, Ultra = 4 };

extern ShadowQuality Shadow_quality;

extern bool Shadow_quality_uses_mod_option; 

extern matrix4 Shadow_view_matrix;
extern matrix4 Shadow_proj_matrix[MAX_SHADOW_CASCADES];
extern float Shadow_cascade_distances[MAX_SHADOW_CASCADES];

void shadows_construct_light_frustum(vec3d *min_out, vec3d *max_out, vec3d light_vec, matrix *orient, vec3d *pos, float fov, float aspect, float z_near, float z_far);
bool shadows_obj_in_frustum(object *objp, vec3d *min, vec3d *max, matrix *light_orient);
void shadows_render_all(float fov, matrix *eye_orient, vec3d *eye_pos);

matrix shadows_start_render(matrix *eye_orient, vec3d *eye_pos, float fov, float aspect, float veryneardist, float neardist, float middist, float fardist);
void shadows_end_render();

/**
* Function to call when evaluating whether a shadowmap should be drawn or not when starting a new frame that is rendered with shadows enabled.
* A call of this function must always be followed up later with shadow_end_frame once the shadow map and the objects using the shadow map are rendered.
* @params override If true, will override the shadow settings to prevent the following render calls from using shadows until the next shadow_end_frame.
* @returns Whether a shadow map needs to be generated or not.
*/
bool shadow_maybe_start_frame(const bool& override = false);
/**
* The follow-up to shadow_maybe_start_frame, for cleaning up and preparing for the next frame. Always call after shadow_maybe_start_frame as soon as the shadow map and the objects using it are rendered.
*/
void shadow_end_frame();

#endif
