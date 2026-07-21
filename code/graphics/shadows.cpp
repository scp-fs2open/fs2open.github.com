/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "graphics/shadows.h"

#include "graphics/2d.h"
#include "graphics/uniforms.h"
#include "asteroid/asteroid.h"
#include "camera/camera.h"
#include "cmdline/cmdline.h"
#include "debris/debris.h"
#include "graphics/matrix.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "object/objectdock.h"
#include "options/Option.h"
#include "prop/prop.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#include "util/uniform_structs.h"

extern vec3d check_offsets[8];

matrix4 Shadow_view_matrix_light;
matrix4 Shadow_view_matrix_render;
SCP_vector<matrix4> Shadow_proj_matrix;
SCP_vector<float> Shadow_cascade_distances;

static SCP_vector<light_frustum_info> Shadow_frustums;

ShadowQuality Shadow_quality = ShadowQuality::Disabled;

bool Shadow_quality_uses_mod_option = false; 

static void parse_shadow_quality_func()
{
	SCP_string mode;
	stuff_string(mode, F_NAME);

	// Convert to lowercase once
	SCP_tolower(mode);

	// Use a map to associate strings with their respective actions
	static const std::unordered_map<std::string, std::function<void()>> effectActions = {
		{"disabled", []() { Shadow_quality = ShadowQuality::Disabled; }},
		{"low", []() { Shadow_quality = ShadowQuality::Low; }},
		{"medium", []() { Shadow_quality = ShadowQuality::Medium; }},
		{"high", []() { Shadow_quality = ShadowQuality::High; }},
		{"ultra", []() { Shadow_quality = ShadowQuality::Ultra; }}
	};

	auto it = effectActions.find(mode);
	if (it != effectActions.end()) {
		it->second(); // Execute the corresponding action
	} else {
		error_display(0, "%s is not a valid shadow quality setting", mode.c_str());
	}
}

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto ShadowQualityOption = options::OptionBuilder<ShadowQuality>("Graphics.Shadows",
                     std::pair<const char*, int>{"Shadow Quality", 1750},
                     std::pair<const char*, int>{"The quality of the shadows", 1751})
                     .values({{ShadowQuality::Disabled, {"Disabled", 413}},
                              {ShadowQuality::Low, {"Low", 1160}},
                              {ShadowQuality::Medium, {"Medium", 1161}},
                              {ShadowQuality::High, {"High", 1162}},
                              {ShadowQuality::Ultra, {"Ultra", 1721}}})
                     .change_listener([](ShadowQuality val, bool initial) {if (initial) {Shadow_quality = val;}return initial;})
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return ShadowQuality::Disabled; } )
                     .importance(80)
                     .parser(parse_shadow_quality_func)
                     .finish();

bool shadows_obj_in_frustum(const object *objp, const matrix *light_orient, const vec3d *min, const vec3d *max)
{
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
	float near_height = tanf(fov * 0.5f) * z_near;
	float near_width = near_height * aspect;

	float far_height = tanf(fov * 0.5f) * z_far;
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
// 	g3_draw_htl_line(&near_bottom_left, &near_bottom_right);
// 	g3_draw_htl_line(&near_bottom_right, &near_top_right);
// 	g3_draw_htl_line(&near_bottom_right, &near_top_left);
// 	g3_draw_htl_line(&near_top_right, &near_top_left);
// 	g3_draw_htl_line(&far_top_left, &far_top_right);
// 	g3_draw_htl_line(&far_top_right, &far_bottom_right);
// 	g3_draw_htl_line(&far_bottom_right, &far_bottom_left);
// 	g3_draw_htl_line(&far_bottom_left, &far_top_left);

	g3_render_line_3d(true, &near_bottom_left, &near_bottom_right);
 	g3_render_line_3d(true, &near_bottom_right, &near_top_right);
 	g3_render_line_3d(true, &near_bottom_right, &near_top_left);
 	g3_render_line_3d(true, &near_top_right, &near_top_left);
 	g3_render_line_3d(true, &far_top_left, &far_top_right);
 	g3_render_line_3d(true, &far_top_right, &far_bottom_right);
 	g3_render_line_3d(true, &far_bottom_right, &far_bottom_left);
 	g3_render_line_3d(true, &far_bottom_left, &far_top_left);
}

void shadows_construct_light_frustum(light_frustum_info *shadow_data, matrix *light_matrix, matrix *orient, vec3d * /*pos*/, fov_t fov, float aspect, float z_near, float z_far)
{
	// find the widths and heights of the near plane and far plane to determine the points of this frustum
	float near_l, near_r, near_u, near_d;
	float far_l, far_r, far_u, far_d;
	if (std::holds_alternative<float>(fov)) {
		near_d = tanf(std::get<float>(fov) * 0.5f) * z_near;
		near_u = -near_d;
		near_r = near_d * aspect;
		near_l = -near_r;

		far_d = tanf(std::get<float>(fov) * 0.5f) * z_far;
		far_u = -far_d;
		far_r = far_d * aspect;
		far_l = -far_r;
	}
	else {
		const auto& afov = std::get<asymmetric_fov>(fov);
		near_d = tanf(-afov.down) * z_near;
		near_u = -tanf(afov.up) * z_near;
		near_r = tanf(-afov.left) * z_near;
		near_l = -tanf(afov.right) * z_near;

		far_d = tanf(-afov.down) * z_far;
		far_u = -tanf(afov.up) * z_far;
		far_r = tanf(-afov.left) * z_far;
		far_l = -tanf(afov.right) * z_far;
	}

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
	vm_vec_scale(&up_scale, near_u);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_l);

	vm_vec_add(&near_top_left, &up_scale, &right_scale);
	vm_vec_add2(&near_top_left, &forward_scale_near);

	// near top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_u);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_r);

	vm_vec_add(&near_top_right, &up_scale, &right_scale);
	vm_vec_add2(&near_top_right, &forward_scale_near);

	// near bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_d);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_l);

	vm_vec_add(&near_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_left, &forward_scale_near);

	// near bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, near_d);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, near_r);

	vm_vec_add(&near_bottom_right, &up_scale, &right_scale);
	vm_vec_add2(&near_bottom_right, &forward_scale_near);

	vec3d far_top_left = ZERO_VECTOR;
	vec3d far_top_right = ZERO_VECTOR;
	vec3d far_bottom_left = ZERO_VECTOR;
	vec3d far_bottom_right = ZERO_VECTOR;

	// far top left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_u);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_l);

	vm_vec_add(&far_top_left, &up_scale, &right_scale);
	vm_vec_add2(&far_top_left, &forward_scale_far);

	// far top right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_u);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_r);

	vm_vec_add(&far_top_right, &up_scale, &right_scale);
	vm_vec_add2(&far_top_right, &forward_scale_far);

	// far bottom left
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_d);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_l);

	vm_vec_add(&far_bottom_left, &up_scale, &right_scale);
	vm_vec_add2(&far_bottom_left, &forward_scale_far);

	// far bottom right
	up_scale = orient->vec.uvec;
	vm_vec_scale(&up_scale, far_d);

	right_scale = orient->vec.rvec;
	vm_vec_scale(&right_scale, far_r);

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

matrix shadows_start_render(matrix *eye_orient, vec3d *eye_pos, fov_t fov, fov_t cockpit_fov, float aspect, const std::optional<SCP_vector<float>>& cascade_distances_override)
{	
	if(Static_light.empty())
		return vmd_identity_matrix; 
	
	auto& lp = Static_light.front();

	vec3d light_dir;
	matrix light_matrix;

	vm_vec_copy_normalize(&light_dir, &lp.vec);
	vm_vector_2_matrix_norm(&light_matrix, &light_dir, &eye_orient->vec.uvec, nullptr);

	const int num_cascades = Num_shadow_cascades + Num_cockpit_shadow_cascades;

	SCP_vector<float> cascade_distances_rebuild;
	int max_skip_override = 0;
	if (cascade_distances_override.has_value()) {
		cascade_distances_rebuild.reserve(num_cascades);
		// If we have override cascades, this is the techroom or similar.
		// In this case, we must fit the override cascades into the given cascade count allocated for the main game.
		// Cockpit cascades are skipped. If there is too few cascades in the override, prepend with zeroes.
		// If there's too many cascades in the override, skip the excess.

		max_skip_override = std::max(Num_cockpit_shadow_cascades, num_cascades - static_cast<int>(cascade_distances_override->size()));
		for (int i = 0; i < max_skip_override; i++) {
			cascade_distances_rebuild.emplace_back(0.0f);
		}
		for (int i = 0; i < num_cascades - max_skip_override; i++) {
			cascade_distances_rebuild.emplace_back(cascade_distances_override->at(i));
		}
	}
	const auto& cascade_distances_actual = cascade_distances_override.has_value() ? cascade_distances_rebuild : Shadow_distances;

	// Only ever do cockpit cascades if there's no override
	bool render_cockpit_cascades = !cascade_distances_override.has_value() && ship_render_player_has_closeup_visuals();

	for (int i = 0; i < num_cascades; i++) {
		float z_near;
		if (i == 0 || (!render_cockpit_cascades && i == Num_cockpit_shadow_cascades)) {
			z_near = 0.0f;
		} else {
			z_near = cascade_distances_actual[i - 1] - (cascade_distances_actual[i - 1] - (i >= 2 ? cascade_distances_actual[i - 2] : 0.0f)) * 0.2f;
		}
		float z_far = cascade_distances_actual[i];

		shadows_construct_light_frustum(&Shadow_frustums[i], &light_matrix, eye_orient, nullptr, i < Num_cockpit_shadow_cascades ? cockpit_fov : fov, aspect, z_near, z_far);
		Shadow_cascade_distances[i] = cascade_distances_actual[i];
		Shadow_proj_matrix[i] = Shadow_frustums[i].proj_matrix;
	}

	gr_shadow_map_start(&Shadow_view_matrix_light, &light_matrix, eye_pos, true);
	if (cascade_distances_override)
		shadow_cascade_params_bind(max_skip_override, num_cascades - max_skip_override);
	else if (render_cockpit_cascades)
		shadow_cascade_params_bind(0, num_cascades);
	else
		shadow_cascade_params_bind(Num_cockpit_shadow_cascades, Num_shadow_cascades);

	return light_matrix;
}

void shadows_end_render()
{
	gr_shadow_map_end();
}

static bool shadow_obj_clip_plane(const object* objp, shadow_render_list::clip_plane_info* clip)
{
	if (objp->type != OBJ_SHIP)
		return false;

	ship* shipp = &Ships[objp->instance];

	if (shipp->is_arriving(ship::warpstage::BOTH, true)) {
		model_render_params dummy;
		if (shipp->warpin_effect->warpShipClip(&dummy)) {
			clip->normal = dummy.get_clip_plane_normal();
			clip->position = dummy.get_clip_plane_pos();
			return true;
		}
	}
	if (shipp->flags[Ship::Ship_Flags::Depart_warp]) {
		model_render_params dummy;
		if (shipp->warpout_effect->warpShipClip(&dummy)) {
			clip->normal = dummy.get_clip_plane_normal();
			clip->position = dummy.get_clip_plane_pos();
			return true;
		}
	}

	dock_function_info dfi;
	dock_evaluate_all_docked_objects(const_cast<object*>(objp), &dfi,
		[](object* docked, dock_function_info* info) {
			if (docked->type != OBJ_SHIP) return;
			auto* dship = &Ships[docked->instance];
			if (dship->is_arriving(ship::warpstage::BOTH, true) || dship->flags[Ship::Ship_Flags::Depart_warp]) {
				info->maintained_variables.bool_value = true;
				info->maintained_variables.objp_value = docked;
			}
		});

	if (dfi.maintained_variables.bool_value) {
		auto* dship = &Ships[dfi.maintained_variables.objp_value->instance];
		auto* warp_effect = (dship->is_arriving(ship::warpstage::BOTH, true)
			? dship->warpin_effect : dship->warpout_effect).get();
		model_render_params dummy;
		if (warp_effect->warpShipClip(&dummy)) {
			clip->normal = dummy.get_clip_plane_normal();
			clip->position = dummy.get_clip_plane_pos();
			return true;
		}
	}

	return false;
}

static void render_viewer_shadow(object* objp, const matrix* light_matrix,
                                 const vec3d* cam_offset, const matrix* rot_offset)
{
	if (objp == nullptr || objp->type != OBJ_SHIP || objp->instance < 0)
		return;

	ship* shipp = &Ships[objp->instance];
	ship_info* sip = &Ship_info[shipp->ship_info_index];

	vec3d eye_pos_local;
	matrix eye_orient;
	object_get_eye(&eye_pos_local, &eye_orient, objp, true, true, false);
	if (cam_offset != nullptr) {
		vec3d offset_local;
		vm_vec_unrotate(&offset_local, cam_offset, &eye_orient);
		(void)offset_local;
	}
	if (rot_offset != nullptr) {
		eye_orient = *rot_offset * eye_orient;
	}

	vec3d eye_offset;
	vm_vec_copy_scale(&eye_offset, &eye_pos_local, -1.0f);
	if (!Disable_cockpit_sway)
		eye_offset += sip->cockpit_sway_val * objp->phys_info.acceleration;

	vec3d view_pos_local;
	vm_vec_rotate(&view_pos_local, &eye_pos_local, &objp->orient);

	if (Show_ship_casts_shadow) {
		matrix4 dummy_view;
		bool casts_shadow_on_cockpit = ship_render_player_ship_casts_shadow_on_cockpit();

		gr_shadow_map_start(&dummy_view, light_matrix, &vmd_zero_vector, false);
		if (casts_shadow_on_cockpit)
			shadow_cascade_params_bind(0, Num_cockpit_shadow_cascades + Num_shadow_cascades);
		else
			shadow_cascade_params_bind(Num_cockpit_shadow_cascades, Num_shadow_cascades);

		model_clear_instance(sip->model_num);
		polymodel_instance* pmi = nullptr;
		if (shipp->model_instance_num >= 0) {
			pmi = model_get_instance(shipp->model_instance_num);
		}
		auto pm = model_get(sip->model_num);

		shadow_render_list viewer_list;
		shadow_render_list::add_model_draws(&viewer_list, pm, pmi, OBJ_INDEX(objp),
											&eye_offset, &objp->orient, nullptr, 0, &view_pos_local);
		viewer_list.init_render(false);
		viewer_list.render_all();
	}

	const bool renderCockpitModel = (Viewer_mode != VM_TOPDOWN) && sip->cockpit_model_num >= 0 && !Disable_cockpits;

	if (renderCockpitModel && !Shadow_disable_overrides.disable_cockpit) {
		matrix4 dummy_view;
		gr_shadow_map_start(&dummy_view, light_matrix, &vmd_zero_vector, false);
		shadow_cascade_params_bind(0, Num_cockpit_shadow_cascades);

		vec3d cockpit_offset = sip->cockpit_offset;
		vm_vec_unrotate(&cockpit_offset, &cockpit_offset, &objp->orient);
		if (!Disable_cockpit_sway)
			cockpit_offset += sip->cockpit_sway_val * objp->phys_info.acceleration;

		model_clear_instance(sip->cockpit_model_num);
		polymodel_instance* cockpit_pmi = nullptr;
		if (shipp->cockpit_model_instance >= 0)
			cockpit_pmi = model_get_instance(shipp->cockpit_model_instance);
		auto cockpit_pm = model_get(sip->cockpit_model_num);

		vec3d cockpit_view_local = view_pos_local;
		vm_vec_sub2(&cockpit_view_local, &sip->cockpit_offset);

		shadow_render_list cockpit_list;
		shadow_render_list::add_model_draws(&cockpit_list, cockpit_pm, cockpit_pmi, OBJ_INDEX(objp),
		                                    &cockpit_offset, &objp->orient, nullptr, 0, &cockpit_view_local);
		cockpit_list.init_render(false);
		cockpit_list.render_all();
	}
}

void shadows_render_all(fov_t fov, matrix *eye_orient, vec3d *eye_pos,
                        const vec3d* cam_offset, const matrix* rot_offset, const fov_t* fov_override)
{
	if (gr_screen.mode == GR_STUB) {
		return;
	}

	GR_DEBUG_SCOPE("Render shadows");
	TRACE_SCOPE(tracing::BuildShadowMap);

	if ( Static_light.empty() ) {
		return;
	}

	if (Shadow_quality == ShadowQuality::Disabled) {
		return;
	}

	Shadow_view_matrix_render = gr_view_matrix;

	gr_end_proj_matrix();
	gr_end_view_matrix();

	fov_t cockpit_fov;
	if (fov_override)
		cockpit_fov = *fov_override;
	else if (Sexp_fov > 0.0f)
		cockpit_fov = Sexp_fov;
	else
		cockpit_fov = COCKPIT_ZOOM_DEFAULT;
	cockpit_fov = cockpit_fov * PROJ_FOV_FACTOR;

	matrix light_matrix = shadows_start_render(eye_orient, eye_pos, fov, cockpit_fov, gr_screen.clip_aspect);

	shadow_render_list shadow_list;

	object *objp = Objects;

	for ( int i = 0; i <= Highest_object_index; i++, objp++ ) {
		if ( objp->flags[Object::Object_Flags::Should_be_dead] )
			continue;

		bool cull = true;
		for (const auto& Shadow_frustum : Shadow_frustums) {
			if ( shadows_obj_in_frustum(objp, &light_matrix, &Shadow_frustum.min, &Shadow_frustum.max) ) {
				cull = false;
				break;
			}
		}
		if ( cull ) continue;

		switch (objp->type) {
		case OBJ_SHIP: {
			if (objp == Viewer_obj) {
				continue;
			}

			ship* shipp = &Ships[objp->instance];

			if (shipp->large_ship_blowup_index >= 0) {
				shipfx_shadow_render_blowup(&shadow_list, shipp);
				continue;
			}

			model_clear_instance(Ship_info[shipp->ship_info_index].model_num);

			shadow_render_list::clip_plane_info clip;
			bool has_clip = shadow_obj_clip_plane(objp, &clip);

			auto pm = model_get(Ship_info[shipp->ship_info_index].model_num);
			polymodel_instance* pmi = nullptr;
			if (shipp->model_instance_num >= 0) {
				pmi = model_get_instance(shipp->model_instance_num);
			}

			vec3d eye_rel, view_pos_local;
			vm_vec_sub(&eye_rel, eye_pos, &objp->pos);
			vm_vec_rotate(&view_pos_local, &eye_rel, &objp->orient);

			shadow_render_list::add_model_draws(&shadow_list, pm, pmi, OBJ_INDEX(objp), &objp->pos, &objp->orient, has_clip ? &clip : nullptr, -1, &view_pos_local);
			break;
		}

		case OBJ_RAW_POF:
		case OBJ_PROP: {
			int model_num = object_get_model_num(objp);
			auto pm = model_get(model_num);
			model_clear_instance(model_num);

			polymodel_instance* pmi = nullptr;
			int instance_num = object_get_model_instance_num(objp);
			if (instance_num >= 0) {
				pmi = model_get_instance(instance_num);
			}

			vec3d eye_rel, view_pos_local;
			vm_vec_sub(&eye_rel, eye_pos, &objp->pos);
			vm_vec_rotate(&view_pos_local, &eye_rel, &objp->orient);

			shadow_render_list::add_model_draws(&shadow_list, pm, pmi, OBJ_INDEX(objp), &objp->pos, &objp->orient, nullptr, -1, &view_pos_local);
			break;
		}

		case OBJ_ASTEROID: {
			int num = objp->instance;
			auto* ast = &Asteroids[num];
			int model_num = Asteroid_info[ast->asteroid_type].subtypes[ast->asteroid_subtype].model_number;
			model_clear_instance(model_num);
			auto pm = model_get(model_num);

			vec3d eye_rel, view_pos_local;
			vm_vec_sub(&eye_rel, eye_pos, &objp->pos);
			vm_vec_rotate(&view_pos_local, &eye_rel, &objp->orient);

			shadow_render_list::add_model_draws(&shadow_list, pm, nullptr, OBJ_INDEX(objp), &objp->pos, &objp->orient, nullptr, -1, &view_pos_local);
			break;
		}

		case OBJ_DEBRIS: {
			debris* db = &Debris[objp->instance];
			if ( !(db->flags[Debris_Flags::Used]) ) continue;

			auto pm = model_get(db->model_num);
			object* debris_obj = &Objects[db->objnum];

			polymodel_instance* pmi = nullptr;
			if (db->model_instance_num >= 0) {
				pmi = model_get_instance(db->model_instance_num);
			}

			vec3d eye_rel, view_pos_local;
			vm_vec_sub(&eye_rel, eye_pos, &debris_obj->pos);
			vm_vec_rotate(&view_pos_local, &eye_rel, &debris_obj->orient);

			shadow_render_list::add_model_draws(&shadow_list, pm, pmi, db->objnum, &debris_obj->pos, &debris_obj->orient, nullptr, -1, &view_pos_local);
			break;
		}

		default:
			break;
		}
	}

	shadow_list.init_render(false);
	shadow_list.render_all();

	render_viewer_shadow(Viewer_obj, &light_matrix, cam_offset, rot_offset);


	shadows_end_render();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);

	gr_clear_states();

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);
}

static bool shadow_override_backup = false;
static bool last_override = false;

bool shadow_maybe_start_frame(const bool& override) {
	last_override = override;
	if (last_override) {
		shadow_override_backup = Shadow_override;
		Shadow_override = true;
		return false;
	}
	return Shadow_quality != ShadowQuality::Disabled;
}

void shadow_end_frame() {
	if (last_override) {
		Shadow_override = shadow_override_backup;
		last_override = false;
	}
}

static gr_buffer_handle Shadow_cascade_params_buffer;
static size_t Shadow_cascade_params_buffer_size = 0;

static std::pair<size_t, size_t> compute_cascade_params_size(int num_cascades) {
	size_t padding_required = num_cascades % 4;
	if (padding_required != 0)
		padding_required = 4 - padding_required;

	return {sizeof(graphics::shadow_cascade_static_data)
		+ sizeof(matrix4) * num_cascades
		+ sizeof(float) * (num_cascades + padding_required)
		+ sizeof(float) * (num_cascades + padding_required),
		padding_required};
}

void shadow_cascade_params_init() {
	const int num_cascades = Num_shadow_cascades + Num_cockpit_shadow_cascades;
	Shadow_cascade_params_buffer_size = compute_cascade_params_size(num_cascades).first;
	Shadow_cascade_params_buffer = gr_create_buffer(BufferType::Uniform, BufferUsageHint::Dynamic);

	SCP_vector<uint8_t> zero_data(Shadow_cascade_params_buffer_size, 0);
	gr_update_buffer_data(Shadow_cascade_params_buffer, Shadow_cascade_params_buffer_size, zero_data.data());

	Shadow_frustums.resize(num_cascades);
	Shadow_cascade_distances.resize(num_cascades);
	Shadow_proj_matrix.resize(num_cascades);
}

void shadow_cascade_params_shutdown() {
	if (Shadow_cascade_params_buffer.isValid()) {
		gr_delete_buffer(Shadow_cascade_params_buffer);
		Shadow_cascade_params_buffer = gr_buffer_handle();
	}
}

int Shadow_cascade_count = 0;
void shadow_cascade_params_bind(int cascade_offset, int cascade_count) {
	if (!Shadow_cascade_params_buffer.isValid()) {
		return;
	}

	const int num_cascades = Num_shadow_cascades + Num_cockpit_shadow_cascades;
	const auto [required_size, padding] = compute_cascade_params_size(num_cascades);

	Assertion(required_size <= Shadow_cascade_params_buffer_size, "The shadow cascade parameter buffer grew in size!");
	Assertion(Shadow_proj_matrix.size() == static_cast<size_t>(num_cascades) && Shadow_cascade_distances.size() == static_cast<size_t>(num_cascades) && Shadow_smoothness_factor.size() == static_cast<size_t>(num_cascades), "Shadow cascade data buffers are of incorrect size! (Expected %d, got %d, %d, %d)", num_cascades, static_cast<int>(Shadow_proj_matrix.size()), static_cast<int>(Shadow_cascade_distances.size()), static_cast<int>(Shadow_smoothness_factor.size()));
	Assertion(cascade_count + cascade_offset <= num_cascades, "Requested drawing out-of-range cascades!");

	SCP_vector<uint8_t> buffer(required_size, 0);
	size_t offset = 0;

	auto& static_data = *reinterpret_cast<graphics::shadow_cascade_static_data*>(buffer.data());

	static_data.cascade_offset = cascade_offset;
	static_data.cascade_count = cascade_count;
	static_data.shadow_mv_matrix = Shadow_view_matrix_light;

	Shadow_cascade_count = cascade_count;

	offset += sizeof(graphics::shadow_cascade_static_data);

	for (int i = 0; i < num_cascades; i++) {
		auto& proj_matrix = *reinterpret_cast<matrix4*>(buffer.data() + offset);
		proj_matrix = Shadow_proj_matrix[i];
		offset += sizeof(matrix4);
	}

	for (int i = 0; i < num_cascades; i++) {
		auto& cascade_distance = *reinterpret_cast<float*>(buffer.data() + offset);
		cascade_distance = Shadow_cascade_distances[i];
		offset += sizeof(float);
	}
	offset += sizeof(float) * padding;

	for (int i = 0; i < num_cascades; i++) {
		auto& smoothness_factor = *reinterpret_cast<float*>(buffer.data() + offset);
		smoothness_factor = Shadow_smoothness_factor[i];
		offset += sizeof(float);
	}
	offset += sizeof(float) * padding;

	gr_update_buffer_data_offset(Shadow_cascade_params_buffer, 0, required_size, buffer.data());
	gr_bind_uniform_buffer(uniform_block_type::ShadowCascadeParams, 0, required_size, Shadow_cascade_params_buffer);
}

shadow_render_list::shadow_render_list() {
	reset();
}

void shadow_render_list::add_draw(const indexed_vertex_source* vert_src,
                                  vertex_buffer* buffer,
                                  size_t texi,
                                  const matrix4& model_matrix,
                                  const vec3d& scale,
                                  const clip_plane_info* clip)
{
	shadow_batch_entry entry;

	entry.model_matrix = model_matrix;
	entry.scale = scale;
	entry.transform_buffer_offset = _batchBuffer.get_buffer_offset();

	entry.flags = 0;
	entry.vert_src = vert_src;
	entry.buffer = buffer;
	entry.texi = texi;

	entry.has_clip_plane = (clip != nullptr);
	if (clip != nullptr) {
		entry.clip_equation.xyzw.x = clip->normal.xyz.x;
		entry.clip_equation.xyzw.y = clip->normal.xyz.y;
		entry.clip_equation.xyzw.z = clip->normal.xyz.z;
		entry.clip_equation.xyzw.w = -vm_vec_dot(&clip->normal, &clip->position);
	} else {
		entry.clip_equation.xyzw.x = 0.0f;
		entry.clip_equation.xyzw.y = 0.0f;
		entry.clip_equation.xyzw.z = 0.0f;
		entry.clip_equation.xyzw.w = 0.0f;
	}

	push_element(std::move(entry));
}

bool shadow_render_list::sort_draw_pair(int a, int b) const
{
	auto* draw_call_a = &_elements[a];
	auto* draw_call_b = &_elements[b];

	if (draw_call_a->flags != draw_call_b->flags) {
		return draw_call_a->flags < draw_call_b->flags;
	}

	return a < b;
}

void shadow_render_list::build_uniform_buffer()
{
	GR_DEBUG_SCOPE("Build shadow uniform buffer");

	_dataBuffer = gr_get_uniform_buffer(uniform_block_type::ShadowMapData, _keys.size());

	for (auto render_index : _keys) {
		auto& queued_draw = _elements[render_index];

		_lights.resetLightState();

		auto element = _dataBuffer.aligner().addTypedElement<graphics::shadow_uniform_data>();
		graphics::uniforms::convert_shadow_material(element,
		                                            queued_draw.model_matrix,
		                                            queued_draw.scale,
		                                            queued_draw.transform_buffer_offset);

		element->clip_equation = queued_draw.clip_equation;
		element->use_clip_plane = queued_draw.has_clip_plane ? 1 : 0;

		queued_draw.uniform_buffer_offset = _dataBuffer.getCurrentAlignerOffset();
	}

	_dataBuffer.submitData();
}

void shadow_render_list::render_buffer(const shadow_batch_entry& entry)
{
	GR_DEBUG_SCOPE("Render shadow buffer");

	auto* datap = &entry.buffer->tex_buf[entry.texi];
	if (datap->n_verts == 0) {
		return;
	}

	gr_bind_uniform_buffer(uniform_block_type::ShadowMapData, entry.uniform_buffer_offset,
	                       sizeof(graphics::shadow_uniform_data), _dataBuffer.bufferHandle());

	gr_render_shadow_draw(_dataBuffer.bufferHandle(),
	                      entry.uniform_buffer_offset,
	                      sizeof(graphics::shadow_uniform_data),
	                      entry.buffer,
	                      const_cast<indexed_vertex_source*>(entry.vert_src),
	                      entry.texi);
}

void shadow_render_list::add_model_draws(shadow_render_list* list,
                                         polymodel* pm,
                                         polymodel_instance* pmi,
                                         int obj_num,
                                         const vec3d* pos, const matrix* orient,
                                         const clip_plane_info* clip,
                                         int detail_level_lock,
                                         const vec3d* view_pos)
{
	int detail_level;
	if (detail_level_lock >= 0) {
		detail_level = detail_level_lock;
	} else {
		float depth = model_render_determine_depth(obj_num, pm->id, orient, pos, -1);
		detail_level = model_render_determine_detail(depth, pm->id, -1);
	}
	int detail_root = pm->detail[detail_level];

	bool render_root_geometry = true;
	if (view_pos != nullptr) {
		if (!model_render_check_detail_box(view_pos, pm, detail_root, 0)) {
			render_root_geometry = false;
		}
	}

	list->clear_transforms();
	list->push_transform(pos, orient);

	//TODO This might be incorrect for some models
	const vec3d scale_identity = SCALE_IDENTITY_VECTOR;

	matrix4 identity_4;
	vm_matrix4_set_identity(&identity_4);

	{
		list->start_model_batch(pm->n_models);

		auto& detail_buffer = pm->detail_buffers[detail_level];
		for (size_t j = 0; j < detail_buffer.tex_buf.size(); j++) {
			if (detail_buffer.tex_buf[j].n_verts == 0) {
				continue;
			}

			int tmap_num = detail_buffer.tex_buf[j].texture;

			if (pm->maps[tmap_num].is_transparent) {
				continue;
			}

			int base_tex = pm->maps[tmap_num].textures[TM_BASE_TYPE].GetTexture();

			bool skip = false;
			if (pmi != nullptr && pmi->texture_replace != nullptr) {
				int replace = (*pmi->texture_replace)[tmap_num * TM_NUM_TYPES + TM_BASE_TYPE];
				if (replace == REPLACE_WITH_INVISIBLE) {
					skip = true;
				} else if (replace < 0 && base_tex < 0) {
					skip = true;
				} else if (replace >= 0) {
					base_tex = 0;
				}
			} else if (base_tex < 0) {
				skip = true;
			}

			if (skip) {
				continue;
			}

			list->add_draw(&pm->vert_source, &detail_buffer, j, identity_4, scale_identity, clip);
		}
	}

	int i = pm->submodel[detail_root].first_child;

	while (i >= 0) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
			render_submodel_children(list, pm, pmi, i, clip, view_pos);
		}

		i = pm->submodel[i].next_sibling;
	}

	if (render_root_geometry)
		list->add_submodel_to_batch(detail_root);

	list->pop_transform();
}

void shadow_render_list::render_submodel_children(shadow_render_list* list,
                                                  polymodel* pm,
                                                  polymodel_instance* pmi,
                                                  int mn,
                                                  const clip_plane_info* clip,
                                                  const vec3d* view_pos)
{
	bsp_info* sm = &pm->submodel[mn];
	submodel_instance* smi = nullptr;

	if (pmi != nullptr) {
		smi = &pmi->submodel[mn];
		if (smi->blown_off) {
			return;
		}
	}

	if (view_pos != nullptr) {
		if (!model_render_check_detail_box(view_pos, pm, mn, 0)) {
			return;
		}
	}

	matrix submodel_orient = vmd_identity_matrix;
	vec3d submodel_offset = sm->offset;

	if (smi != nullptr) {
		submodel_orient = smi->canonical_orient;
		vm_vec_add2(&submodel_offset, &smi->canonical_offset);
	}

	list->push_transform(&submodel_offset, &submodel_orient);

	list->add_submodel_to_batch(mn);

	// Recurse into children
	int i = sm->first_child;
	while (i >= 0) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
			render_submodel_children(list, pm, pmi, i, clip, view_pos);
		}

		i = pm->submodel[i].next_sibling;
	}

	list->pop_transform();
}