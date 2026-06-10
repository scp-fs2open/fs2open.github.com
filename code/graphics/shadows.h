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

#include "graphics/render_queue.h"
#include "globalincs/pstypes.h"
#include "object/object.h"
#include "render/3d.h"

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


extern matrix4 Shadow_view_matrix_light;
extern matrix4 Shadow_view_matrix_render;

extern bool Shadow_quality_uses_mod_option; 

extern matrix4 Shadow_proj_matrix[MAX_SHADOW_CASCADES];
extern float Shadow_cascade_distances[MAX_SHADOW_CASCADES];

void shadows_construct_light_frustum(vec3d *min_out, vec3d *max_out, vec3d light_vec, matrix *orient, vec3d *pos, fov_t fov, float aspect, float z_near, float z_far);
bool shadows_obj_in_frustum(object *objp, vec3d *min, vec3d *max, matrix *light_orient);
void shadows_render_all(fov_t fov, matrix *eye_orient, vec3d *eye_pos);

matrix shadows_start_render(matrix *eye_orient, vec3d *eye_pos, fov_t fov, float aspect, float veryneardist, float neardist, float middist, float fardist);
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

struct shadow_batch_entry {
	size_t uniform_buffer_offset = 0;
	size_t transform_buffer_offset = 0;
	bool has_clip_plane;
	vec4 clip_equation;
	matrix4 model_matrix;
	vec3d scale;
	int flags;
	const indexed_vertex_source* vert_src;
	vertex_buffer* buffer;
	size_t texi;
};

class shadow_render_list : public render_queue<shadow_render_list, shadow_batch_entry> {
	friend class render_queue<shadow_render_list, shadow_batch_entry>;
public:
	struct clip_plane_info {
		vec3d normal;
		vec3d position;
	};

	shadow_render_list();
	~shadow_render_list() = default;

	void add_draw(const indexed_vertex_source* vert_src,
				  vertex_buffer* buffer,
				  size_t texi,
				  const matrix4& model_matrix,
				  const vec3d& scale,
				  const clip_plane_info* clip);

	static void add_model_draws(shadow_render_list* list,
								polymodel* pm,
								polymodel_instance* pmi,
								int obj_num,
								const vec3d* pos, const matrix* orient,
								const clip_plane_info* clip);

private:
	void build_uniform_buffer();
	void render_buffer(const shadow_batch_entry& entry);
	bool sort_draw_pair(int a, int b) const;

	void sort_draws() {}

	static void render_submodel_children(shadow_render_list* list,
										 polymodel* pm,
										 polymodel_instance* pmi,
										 int mn,
										 const clip_plane_info* clip);
};

#endif
