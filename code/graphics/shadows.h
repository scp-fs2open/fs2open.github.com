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

// Rays traced per pixel per shadowed light, for lights that have a source size --
// suns from their drawn size, or from $SunAngularSize in stars.tbl when that
// overrides it; local lights via their source_radius. 1 means the single hard ray
// in traceShadowRayCone() (shadows.sdr); above that it samples a penumbra.
//
// This is a cost knob, not an appearance knob: how *wide* a penumbra is comes
// solely from the light's source size, which is what also drives the shadow-map
// path and keeps the two methods consistent for content authors. The sample count
// only decides how well that penumbra resolves.
//
// Which is why it is not a setting of its own: it follows Shadow_quality
// (Low/Medium/High/Ultra -> 1/4/8/16), the same tier that picks the shadow map's
// resolution, so one control covers shadow cost whichever method is active.
// Capped in-shader at RT_SHADOW_MAX_SAMPLES (16).
int shadows_rt_sample_count();

// Session-only override for shadows_rt_sample_count(), for iterating in the lab
// without touching a persisted setting. <= 0 means "follow Shadow_quality".
extern int Rt_shadow_samples_override;

// Edge length, in texels, of one square cascade of the shadow map, as chosen by
// Shadow_quality. Meaningless when Shadow_quality is Disabled (callers guard on that
// first); the value returned in that case is only there to keep the result usable as a
// divisor.
int shadows_map_resolution();

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

// Per-cascade PCSS penumbra scale: world-space blocker/receiver depth separation, scaled
// by the sun's tanθ and converted to UV space, gives the contact-hardened penumbra radius
// directly (see the derivation comment above shadow_smoothness_scale() in shadows.cpp).
// A negative value is a sentinel meaning shadow_contact_hardening_enabled() was false
// when it was computed -- the shader falls back to the fixed smoothness_factors[cascade].
extern SCP_vector<float> Shadow_penumbra_scale;

// Whether the current renderer can do a raw (uncompared) depth read off the shadow map,
// which PCSS blocker search needs. True on Vulkan always; on OpenGL requires
// GL 3.3 (a second sampler object with compare mode off, bound to the
// same shadow texture -- see shadow_map_raw in shadows.sdr). Independent of whether the
// user currently has contact hardening turned on -- use this to decide whether to offer
// the option at all.
bool shadow_contact_hardening_supported();

// User-facing toggle for shadow contact hardening (PCSS blocker search): sizes each
// cascade's penumbra from actual occluder distance instead of the fixed
// $Shadow Smoothness Factor: width. Costs an extra raw-depth search per shadowed pixel
// (see pcssBlockerSearch() in shadows.sdr), so this is exposed as a setting rather than
// being unconditional -- persisted via Graphics.ShadowContactHardening. See
// shadow_contact_hardening_enabled() for the flag that actually gates the per-frame cost.
extern bool Shadow_contact_hardening_enabled;

// Whether shadow rendering should actually do the contact-hardening blocker search this
// frame, i.e. shadow_contact_hardening_supported() is true AND the user has the option on.
// This is the single source of truth -- gate any per-cascade penumbra-scale computation on
// this, not on shadow_contact_hardening_supported()/Shadow_contact_hardening_enabled
// separately. When false, Shadow_penumbra_scale gets the same negative sentinel as
// "unsupported", so the shader takes the pre-contact-hardening fixed-width path at zero
// extra cost (see pcssPenumbraRadius() in shadows.sdr).
bool shadow_contact_hardening_enabled();

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
