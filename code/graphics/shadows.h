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

class ship_info;

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

// Maximum number of directional lights that will cast raytraced shadows in a given frame.
// Only meaningful when shadows_use_raytracing() is true -- see MaxRtShadowLightsOption's
// enumerator in shadows.cpp.
extern int Max_rt_shadow_lights;

// Low: only directional lights cast raytraced shadows (capped by Max_rt_shadow_lights).
// High: additionally, point/tube/cone lights cast raytraced shadows (capped by
// Max_rt_shadow_local_lights). See RTShadowQualityOption's enumerator in shadows.cpp.
enum class RTShadowQuality { Low = 0, High = 1 };

extern RTShadowQuality Rt_shadow_quality;

// Maximum number of point/tube/cone lights that will cast raytraced shadows in a given frame.
// Independent of Max_rt_shadow_lights (which only counts directional lights). Only meaningful
// when shadows_use_raytracing() && Rt_shadow_quality == RTShadowQuality::High -- see
// MaxRtShadowLocalLightsOption's enumerator in shadows.cpp.
extern int Max_rt_shadow_local_lights;

// World-unit offset applied along the surface normal to the raytraced shadow ray's
// origin, to avoid self-intersection against the source triangle. Scales with the
// fragment's distance from the camera between these two bounds -- see
// computeRtShadowBias() in shadows.sdr -- since geometry close to the camera needs
// much less offset to clear acne than distant geometry does. Tunable in the in-game
// options menu (Min/Max Raytraced Shadow Bias) and, for quick iteration, via the
// LabUi "RT Shadow bias" slider (session-only override, does not touch the option).
extern float Rt_shadow_bias_min;
extern float Rt_shadow_bias_max;

// Whether the current hardware/renderer can do anything with ShadowRenderMethod::Raytraced
// at all (Vulkan + VK_KHR_acceleration_structure + VK_KHR_ray_query support). Independent
// of which method is currently selected -- use this to decide whether to offer the choice.
bool shadows_raytracing_supported();

// Removes the raytraced-shadow options (method selector, light counts, bias) from the
// options menu when the current renderer/hardware can't do raytraced shadows, so they don't
// clutter the UI with settings that have no effect. Call once after the renderer is up
// (gr_init), when shadows_raytracing_supported() is meaningful.
void shadows_remove_unsupported_options();

// Whether shading should actually sample the raytraced-shadow TLAS right now, i.e. both
// the user has selected it AND the hardware supports it. This is the single source of
// truth for that decision -- gate any new raytraced-shadow shader-flag or resource-binding
// code on this, not on Shadow_render_method/shadows_raytracing_supported() separately.
bool shadows_use_raytracing();

// Whether point/tube/cone lights should additionally cast raytraced shadows this frame,
// i.e. shadows_use_raytracing() is true AND the user has selected RTShadowQuality::High.
bool shadows_use_raytraced_local_lights();

extern matrix4 Shadow_view_matrix_light;
extern matrix4 Shadow_view_matrix_render;

extern bool Shadow_quality_uses_mod_option; 

extern SCP_vector<matrix4> Shadow_proj_matrix;
extern SCP_vector<float> Shadow_cascade_distances;
extern int Shadow_cascade_count;

// TLAS instance masks. Shared by the TLAS build (VulkanRaytracingManager) and the shadow-ray
// cull mask uploaded by shadow_cascade_params_bind(). An instance is a ray candidate iff
// (cullMask & instance.mask) != 0.
constexpr uint8_t TLAS_MASK_ALL = 0xFF;
// Reserved for the viewer ship's own hull, so shadow rays can exclude it (see traceShadowRay()).
constexpr uint8_t TLAS_MASK_VIEWER_HULL = 0x80;
constexpr uint8_t SHADOW_RAY_CULL_MASK_EXCLUDE_VIEWER_HULL = static_cast<uint8_t>(~TLAS_MASK_VIEWER_HULL);

void shadows_construct_light_frustum(vec3d *min_out, vec3d *max_out, vec3d light_vec, matrix *orient, vec3d *pos, fov_t fov, float aspect, float z_near, float z_far);
bool shadows_obj_in_frustum(object *objp, vec3d *min, vec3d *max, matrix *light_orient);
void shadows_render_all(fov_t fov, matrix *eye_orient, vec3d *eye_pos,
                        const vec3d* cam_offset = nullptr, const matrix* rot_offset = nullptr, const fov_t* fov_override = nullptr);

// Origin of the TLAS coordinate frame: every TLAS instance is translated by -this, so
// coordinates near the viewer stay small and float32-precise no matter how far the viewer
// is from the mission origin. Set once per frame when the TLAS is built
// (VulkanRaytracingManager::buildTlas()); zero until then.
extern vec3d Shadow_rt_tlas_origin;

// Converts a true world-space position to TLAS-relative space.
vec3d shadow_rt_relative(const vec3d& world_pos);

void shadow_cascade_params_init();
void shadow_cascade_params_shutdown();

// view_origin: the camera position in TLAS-relative space (see Shadow_rt_tlas_origin and
// shadow_cascade_static_data::shadow_ray_view_origin, uniform_structs.h). Ordinary passes
// use shadow_rt_relative(Eye_position); the cockpit pass renders in its own frame and
// must add the viewer ship's position (see ship_render_player_ship()). A pass with draws in
// more than one internal frame -- e.g. ship_render_player_ship()'s hull vs. cockpit draws --
// must rebind before each one. Only the RT shadow shaders read it, so the rasterized
// shadow-map passes just pass vmd_zero_vector.
// allow_viewer_self_shadow: lets the viewer ship's own hull (TLAS_MASK_VIEWER_HULL,
// shadows.h) cast shadows in this pass. True only for the cockpit's own shading.
// Ray-traced shadow state that goes with a cascade bind. Only the RT shadow shaders read it.
struct shadow_ray_params {
	// Camera position in TLAS-relative space (see Shadow_rt_tlas_origin and
	// shadow_cascade_static_data::shadow_ray_view_origin in uniform_structs.h).
	vec3d view_origin;
	// Which TLAS instances the shadow rays can hit.
	uint8_t cull_mask = SHADOW_RAY_CULL_MASK_EXCLUDE_VIEWER_HULL;
};

// Ray view origin of the cockpit frame (view anchored at leaning_position, models offset in
// that same frame and never combined with objp->pos), so it adds the viewer's position back.
shadow_ray_params shadow_ray_params_cockpit(const object* viewer);

// Ordinary bind: the ray view origin is the current camera, the viewer hull casts no shadow.
void shadow_cascade_params_bind(int cascade_offset, int cascade_count);
// Bind with explicit ray state. A pass that draws in more than one internal frame (the hull
// vs. the cockpit in ship_render_player_ship()) must rebind before each draw.
void shadow_cascade_params_bind(int cascade_offset, int cascade_count, const shadow_ray_params& ray);

// Binds the cascade range and ray state for the current Lighting_mode. For the deferred
// full-screen passes, which shade one frame per pass.
void shadow_cascade_params_bind_deferred();

// True when the viewer's cockpit model also gets a shadow pass (raster and RT).
bool shadows_cockpit_casts_shadow(const ship_info* sip);

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
