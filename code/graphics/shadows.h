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

struct light_frustum_info
{
	matrix4 proj_matrix;

	vec3d min;
	vec3d max;

	float start_dist;
};

enum class ShadowQuality { Disabled = 0, Low = 1, Medium = 2, High = 3, Ultra = 4 };

extern ShadowQuality Shadow_quality;

// Which technique is used to render shadows when Shadow_quality != Disabled.
// Raytraced is only ever selectable when gr_is_capable(CAPABILITY_RAYTRACED_SHADOWS)
// is true (Vulkan + hardware ray query support) -- see ShadowRenderMethodOption's
// enumerator in shadows.cpp.
enum class ShadowRenderMethod { ShadowMap = 0, Raytraced = 1 };

extern ShadowRenderMethod Shadow_render_method;

// Whether the current hardware/renderer can do anything with ShadowRenderMethod::Raytraced
// at all (Vulkan + VK_KHR_acceleration_structure + VK_KHR_ray_query support). Independent
// of which method is currently selected -- use this to decide whether to offer the choice.
bool shadows_raytracing_supported();

// Whether shading should actually sample the raytraced-shadow TLAS right now, i.e. both
// the user has selected it AND the hardware supports it. This is the single source of
// truth for that decision -- gate any new raytraced-shadow shader-flag or resource-binding
// code on this, not on Shadow_render_method/shadows_raytracing_supported() separately.
bool shadows_use_raytracing();

extern matrix4 Shadow_view_matrix_light;
extern matrix4 Shadow_view_matrix_render;

extern bool Shadow_quality_uses_mod_option; 

extern SCP_vector<matrix4> Shadow_proj_matrix;
extern SCP_vector<float> Shadow_cascade_distances;
extern int Shadow_cascade_count;

void shadows_construct_light_frustum(vec3d *min_out, vec3d *max_out, vec3d light_vec, matrix *orient, vec3d *pos, fov_t fov, float aspect, float z_near, float z_far);
bool shadows_obj_in_frustum(object *objp, vec3d *min, vec3d *max, matrix *light_orient);
void shadows_render_all(fov_t fov, matrix *eye_orient, vec3d *eye_pos,
                        const vec3d* cam_offset = nullptr, const matrix* rot_offset = nullptr, const fov_t* fov_override = nullptr);

void shadow_cascade_params_init();
void shadow_cascade_params_shutdown();
void shadow_cascade_params_bind(int cascade_offset, int cascade_count);

matrix shadows_start_render(matrix *eye_orient, vec3d *eye_pos, fov_t fov, fov_t cockpit_fov, float aspect, const std::optional<SCP_vector<float>>& cascade_distances_override = std::nullopt);
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
								const clip_plane_info* clip,
								int detail_level_lock = -1,
								const vec3d* view_pos = nullptr);

private:
	void build_uniform_buffer();
	void render_buffer(const shadow_batch_entry& entry);
	bool sort_draw_pair(int a, int b) const;

	void sort_draws() {}

	static void render_submodel_children(shadow_render_list* list,
										 polymodel* pm,
										 polymodel_instance* pmi,
										 int mn,
										 const clip_plane_info* clip,
										 const vec3d* view_pos);
};

#endif
