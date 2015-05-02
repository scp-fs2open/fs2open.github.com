/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "graphics/shadows.h"
#include "globalincs/pstypes.h"
#include "math/vecmat.h"
#include "object/object.h"
#include "lighting/lighting.h"
#include "graphics/gropengltnl.h"
#include "cmdline/cmdline.h"
#include "render/3d.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "debris/debris.h"
#include "asteroid/asteroid.h"
#include "graphics/gropengldraw.h"

extern vec3d check_offsets[8];

matrix4 Shadow_view_matrix;
matrix4 Shadow_proj_matrix[MAX_SHADOW_CASCADES];
float Shadow_cascade_distances[MAX_SHADOW_CASCADES];

light_frustum_info Shadow_frustums[MAX_SHADOW_CASCADES];

bool shadows_obj_in_frustum(object *objp, matrix *light_orient, vec3d *min, vec3d *max)
{
	int i;
	vec3d pts[8], tmp, pt, pos_lightspace, pos_viewspace;
	vec3d obj_min = ZERO_VECTOR;
	vec3d obj_max = ZERO_VECTOR;

	vec3d pos, pos_rot;

	vm_vec_sub(&pos, &objp->pos, &Eye_position);
	vm_vec_rotate(&pos_rot, &pos, light_orient);

	if ( (pos_rot.xyz.x - objp->radius) > max->xyz.x 
		|| (pos_rot.xyz.x + objp->radius) < min->xyz.x 
		|| (pos_rot.xyz.y - objp->radius) > max->xyz.y 
		|| (pos_rot.xyz.y + objp->radius) < min->xyz.y 
		|| (pos_rot.xyz.z - objp->radius) > max->xyz.z ) {
		return false;
	}

	return true;
}

void shadows_construct_light_proj(light_frustum_info *shadow_data)
{
	memset(&shadow_data->proj_matrix, 0, sizeof(matrix4));

	shadow_data->proj_matrix.a1d[0] = 2.0f / ( shadow_data->max.xyz.x - shadow_data->min.xyz.x );
	shadow_data->proj_matrix.a1d[5] = 2.0f / ( shadow_data->max.xyz.y - shadow_data->min.xyz.y );
	shadow_data->proj_matrix.a1d[10] = -2.0f / ( shadow_data->max.xyz.z - shadow_data->min.xyz.z );
	shadow_data->proj_matrix.a1d[12] = -(shadow_data->max.xyz.x + shadow_data->min.xyz.x) / ( shadow_data->max.xyz.x - shadow_data->min.xyz.x );
	shadow_data->proj_matrix.a1d[13] = -(shadow_data->max.xyz.y + shadow_data->min.xyz.y) / ( shadow_data->max.xyz.y - shadow_data->min.xyz.y );
	shadow_data->proj_matrix.a1d[14] = -(shadow_data->max.xyz.z + shadow_data->min.xyz.z) / ( shadow_data->max.xyz.z - shadow_data->min.xyz.z );
	shadow_data->proj_matrix.a1d[15] = 1.0f;
}

void shadows_debug_show_frustum(matrix* orient, vec3d *pos, float fov, float aspect, float z_near, float z_far)
{
	// find the widths and heights of the near plane and far plane to determine the points of this frustum
	float near_height = (float)tan((double)fov * 0.5) * z_near;
	float near_width = near_height * aspect;

	float far_height = (float)tan((double)fov * 0.5) * z_far;
	float far_width = far_height * aspect;

	vec3d up_scale = ZERO_VECTOR;
	vec3d right_scale = ZERO_VECTOR;
	vec3d forward_scale_near = orient->vec.fvec;
	vec3d forward_scale_far = orient->vec.fvec;

	vm_vec_scale(&forward_scale_near, z_near);
	vm_vec_scale(&forward_scale_far, z_far);

	// find the eight points using eye orientation and position
	vec3d near_top_left = ZERO_VECTOR;
	vec3d near_top_right = ZERO_VECTOR;
	vec3d near_bottom_left = ZERO_VECTOR;
	vec3d near_bottom_right = ZERO_VECTOR;

	// near top left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -near_width);

	vm_vec_add(&near_top_left, &up_scale, &right_scale);
	vm_vec_add2(&near_top_left, &forward_scale_near);

	// near top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_width);

	vm_vec_add(&near_top_right, &up_scale, &right_scale);
	vm_vec_add2(&near_top_right, &forward_scale_near);

	// near bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -near_width);

	vm_vec_add(&near_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_left, &forward_scale_near);

	// near bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_width);

	vm_vec_add(&near_bottom_right, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_right, &forward_scale_near);

	vec3d far_top_left = ZERO_VECTOR;
	vec3d far_top_right = ZERO_VECTOR;
	vec3d far_bottom_left = ZERO_VECTOR;
	vec3d far_bottom_right = ZERO_VECTOR;

	// far top left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -far_width);

	vm_vec_add(&far_top_left, &up_scale, &right_scale);
	vm_vec_add2(&far_top_left, &forward_scale_far);

	// far top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_width);

	vm_vec_add(&far_top_right, &up_scale, &right_scale);
	vm_vec_add2(&far_top_right, &forward_scale_far);

	// far bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -far_width);

	vm_vec_add(&far_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&far_bottom_left, &forward_scale_far);

	// far bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_width);

	vm_vec_add(&far_bottom_right, &up_scale, &right_scale);
	vm_vec_add2(&far_bottom_right, &forward_scale_far);

	// translate frustum
	vm_vec_add2(&near_bottom_left, pos);
	vm_vec_add2(&near_bottom_right, pos);
	vm_vec_add2(&near_top_right, pos);
	vm_vec_add2(&near_top_left, pos);
	vm_vec_add2(&far_top_left, pos);
	vm_vec_add2(&far_top_right, pos);
	vm_vec_add2(&far_bottom_right, pos);
	vm_vec_add2(&far_bottom_left, pos);

	gr_set_color(0, 255, 255);
	g3_draw_htl_line(&near_bottom_left, &near_bottom_right);
	g3_draw_htl_line(&near_bottom_right, &near_top_right);
	g3_draw_htl_line(&near_bottom_right, &near_top_left);
	g3_draw_htl_line(&near_top_right, &near_top_left);
	g3_draw_htl_line(&far_top_left, &far_top_right);
	g3_draw_htl_line(&far_top_right, &far_bottom_right);
	g3_draw_htl_line(&far_bottom_right, &far_bottom_left);
	g3_draw_htl_line(&far_bottom_left, &far_top_left);
}

void shadows_construct_light_frustum(light_frustum_info *shadow_data, matrix *light_matrix, matrix *orient, vec3d *pos, float fov, float aspect, float z_near, float z_far)
{
	// find the widths and heights of the near plane and far plane to determine the points of this frustum
	float near_height = (float)tan((double)fov * 0.5) * z_near;
	float near_width = near_height * aspect;

	float far_height = (float)tan((double)fov * 0.5f) * z_far;
	float far_width = far_height * aspect;

	vec3d up_scale = ZERO_VECTOR;
	vec3d right_scale = ZERO_VECTOR;
	vec3d forward_scale_near = orient->vec.fvec;
	vec3d forward_scale_far = orient->vec.fvec;

	vm_vec_scale(&forward_scale_near, z_near);
	vm_vec_scale(&forward_scale_far, z_far);

	// find the eight points using eye orientation and position
	vec3d near_top_left = ZERO_VECTOR;
	vec3d near_top_right = ZERO_VECTOR;
	vec3d near_bottom_left = ZERO_VECTOR;
	vec3d near_bottom_right = ZERO_VECTOR;

	// near top left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -near_width);

	vm_vec_add(&near_top_left, &up_scale, &right_scale);
	vm_vec_add2(&near_top_left, &forward_scale_near);

	// near top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_width);

	vm_vec_add(&near_top_right, &up_scale, &right_scale);
	vm_vec_add2(&near_top_right, &forward_scale_near);

	// near bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -near_width);

	vm_vec_add(&near_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_left, &forward_scale_near);

	// near bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_width);

	vm_vec_add(&near_bottom_right, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_right, &forward_scale_near);

	vec3d far_top_left = ZERO_VECTOR;
	vec3d far_top_right = ZERO_VECTOR;
	vec3d far_bottom_left = ZERO_VECTOR;
	vec3d far_bottom_right = ZERO_VECTOR;

	// far top left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -far_width);

	vm_vec_add(&far_top_left, &up_scale, &right_scale);
	vm_vec_add2(&far_top_left, &forward_scale_far);

	// far top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, -far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_width);

	vm_vec_add(&far_top_right, &up_scale, &right_scale);
	vm_vec_add2(&far_top_right, &forward_scale_far);

	// far bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, -far_width);

	vm_vec_add(&far_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&far_bottom_left, &forward_scale_far);

	// far bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_height);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_width);

	vm_vec_add(&far_bottom_right, &up_scale, &right_scale);
	vm_vec_add2(&far_bottom_right, &forward_scale_far);
	
	vec3d frustum_pts[8];

	// bring frustum points into light space
	vm_vec_rotate(&frustum_pts[0], &near_bottom_left, light_matrix);
	vm_vec_rotate(&frustum_pts[1], &near_bottom_right, light_matrix);
	vm_vec_rotate(&frustum_pts[2], &near_top_right, light_matrix);
	vm_vec_rotate(&frustum_pts[3], &near_top_left, light_matrix);
	vm_vec_rotate(&frustum_pts[4], &far_top_left, light_matrix);
	vm_vec_rotate(&frustum_pts[5], &far_top_right, light_matrix);
	vm_vec_rotate(&frustum_pts[6], &far_bottom_right, light_matrix);
	vm_vec_rotate(&frustum_pts[7], &far_bottom_left, light_matrix);

	vec3d min = ZERO_VECTOR;
	vec3d max = ZERO_VECTOR;

	min = frustum_pts[0];
	max = frustum_pts[0];

	// find min and max of frustum points
	for (int i = 0; i < 8; ++i) {
		if ( frustum_pts[i].xyz.x < min.xyz.x ) {
			min.xyz.x = frustum_pts[i].xyz.x;
		}

		if ( frustum_pts[i].xyz.x > max.xyz.x ) {
			max.xyz.x = frustum_pts[i].xyz.x;
		}

		if ( frustum_pts[i].xyz.y < min.xyz.y ) {
			min.xyz.y = frustum_pts[i].xyz.y;
		}

		if ( frustum_pts[i].xyz.y > max.xyz.y ) {
			max.xyz.y = frustum_pts[i].xyz.y;
		}

		if ( frustum_pts[i].xyz.z < min.xyz.z ) {
			min.xyz.z = frustum_pts[i].xyz.z;
		}

		if ( frustum_pts[i].xyz.z > max.xyz.z ) {
			max.xyz.z = frustum_pts[i].xyz.z;
		}
	}

	shadow_data->min = min;
	shadow_data->max = max;

	shadows_construct_light_proj(shadow_data);
}

matrix shadows_start_render(matrix *eye_orient, vec3d *eye_pos, float fov, float aspect, float veryneardist, float neardist, float middist, float fardist)
{	
	if(Static_light.empty())
		return vmd_identity_matrix; 
	
	light *lp = *(Static_light.begin());

	if ( lp == NULL ) {
		return vmd_identity_matrix;
	}

	vec3d light_dir;
	matrix light_matrix;

	vm_vec_copy_normalize(&light_dir, &lp->vec);
	vm_vector_2_matrix(&light_matrix, &light_dir, &eye_orient->vec.uvec, NULL);

	shadows_construct_light_frustum(&Shadow_frustums[0], &light_matrix, eye_orient, eye_pos, fov, aspect, 0.0f, veryneardist);
	shadows_construct_light_frustum(&Shadow_frustums[1], &light_matrix, eye_orient, eye_pos, fov, aspect, veryneardist - (veryneardist - 0.0f)* 0.2f, neardist);
	shadows_construct_light_frustum(&Shadow_frustums[2], &light_matrix, eye_orient, eye_pos, fov, aspect, neardist - (neardist - veryneardist) * 0.2f, middist);
	shadows_construct_light_frustum(&Shadow_frustums[3], &light_matrix, eye_orient, eye_pos, fov, aspect, middist - (middist - neardist) * 0.2f, fardist);
	
	Shadow_cascade_distances[0] = veryneardist;
	Shadow_cascade_distances[1] = neardist;
	Shadow_cascade_distances[2] = middist;
	Shadow_cascade_distances[3] = fardist;

	Shadow_proj_matrix[0] = Shadow_frustums[0].proj_matrix;
	Shadow_proj_matrix[1] = Shadow_frustums[1].proj_matrix;
	Shadow_proj_matrix[2] = Shadow_frustums[2].proj_matrix;
	Shadow_proj_matrix[3] = Shadow_frustums[3].proj_matrix;

	gr_shadow_map_start(&Shadow_view_matrix, &light_matrix);

	return light_matrix;
}

void shadows_end_render()
{
	gr_shadow_map_end();
}

void shadows_render_all(float fov, matrix *eye_orient, vec3d *eye_pos)
{
	if ( Static_light.empty() ) {
		return;
	}

	light *lp = *(Static_light.begin());

	if( Cmdline_nohtl || !Cmdline_shadow_quality || !lp ) {
		return;
	}

	//shadows_debug_show_frustum(&Player_obj->orient, &Player_obj->pos, fov, gr_screen.clip_aspect, Min_draw_distance, 3000.0f);

	gr_end_proj_matrix();
	gr_end_view_matrix();

	// these cascade distances are a result of some arbitrary tuning to give a good balance of quality and banding. 
	// maybe we could use a more programmatic algorithim? 
	matrix light_matrix = shadows_start_render(eye_orient, eye_pos, fov, gr_screen.clip_aspect, 200.0f, 600.0f, 2500.0f, 8000.0f);

	draw_list scene;
	object *objp = Objects;

	for ( int i = 0; i <= Highest_object_index; i++, objp++ ) {
		bool cull = true;

		for ( int j = 0; j < MAX_SHADOW_CASCADES; ++j ) {
			if ( shadows_obj_in_frustum(objp, &light_matrix, &Shadow_frustums[j].min, &Shadow_frustums[j].max) ) {
				cull = false;
				break;
			}
		}

		if ( cull ) {
			continue;
		}

		switch(objp->type)
		{
		case OBJ_SHIP:
			{
				obj_queue_render(objp, &scene);
			}
			break;
		case OBJ_ASTEROID:
			{
				model_render_params render_info;

				render_info.set_object_number(OBJ_INDEX(objp));
				render_info.set_flags(MR_IS_ASTEROID | MR_NO_TEXTURING | MR_NO_LIGHTING);
				
				model_clear_instance( Asteroid_info[Asteroids[objp->instance].asteroid_type].model_num[Asteroids[objp->instance].asteroid_subtype]);
				model_render_queue(&render_info, &scene, Asteroid_info[Asteroids[objp->instance].asteroid_type].model_num[Asteroids[objp->instance].asteroid_subtype], &objp->orient, &objp->pos);
			}
			break;

		case OBJ_DEBRIS:
			{
				debris *db;
				db = &Debris[objp->instance];

				if ( !(db->flags & DEBRIS_USED)){
					continue;
				}
								
				objp = &Objects[db->objnum];

				model_render_params render_info;

				render_info.set_flags(MR_NO_TEXTURING | MR_NO_LIGHTING);

				submodel_render_queue(&render_info, &scene, db->model_num, db->submodel_num, &objp->orient, &objp->pos);
			}
			break; 
		}
	}

	scene.init_render();
	scene.render_all(GR_ZBUFF_FULL);

	shadows_end_render();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);

	gr_clear_states();
	gr_set_buffer(-1);

	GL_state.Texture.DisableAll();

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);
}