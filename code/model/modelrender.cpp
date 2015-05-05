/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <algorithm>

#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "cmdline/cmdline.h"
#include "nebula/neb.h"
#include "graphics/tmapper.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropengldraw.h"
#include "particle/particle.h"
#include "gamesequence/gamesequence.h"
#include "render/3dinternal.h"
#include "math/staticrand.h"

extern int Model_texturing;
extern int Model_polys;
extern int tiling;
extern float model_radius;

extern const int MAX_ARC_SEGMENT_POINTS;
extern int Num_arc_segment_points;
extern vec3d Arc_segment_points[];

extern bool Scene_framebuffer_in_frame;

extern void interp_render_arc_segment( vec3d *v1, vec3d *v2, int depth );

extern int Interp_thrust_scale_subobj;
extern float Interp_thrust_scale;

draw_list *draw_list::Target = NULL;

model_batch_buffer TransformBufferHandler;

model_render_params::model_render_params():
	Model_flags(MR_NORMAL),
	Debug_flags(0),
	Objnum(-1),
	Detail_level_locked(-1),
	Depth_scale(1500.0f),
	Warp_bitmap(-1),
	Warp_alpha(-1.0f),
	Xparent_alpha(1.0f),
	Forced_bitmap(-1),
	Replacement_textures(NULL),
	Insignia_bitmap(-1),
	Team_color_set(false),
	Clip_plane_set(false),
	Animated_effect(-1),
	Animated_timer(0.0f),
	Thruster_info()
{
	Warp_scale.xyz.x = 1.0f;
	Warp_scale.xyz.y = 1.0f;
	Warp_scale.xyz.z = 1.0f;
	
	Clip_normal = vmd_zero_vector;
	Clip_pos = vmd_zero_vector;

	if ( !Model_texturing )
		Model_flags |= MR_NO_TEXTURING;

	if ( !Model_polys )	{
		Model_flags |= MR_NO_POLYS;
	}
}

uint model_render_params::get_model_flags()
{
	return Model_flags; 
}

uint model_render_params::get_debug_flags()
{
	return Debug_flags;
}

int model_render_params::get_object_number()
{ 
	return Objnum; 
}

int model_render_params::get_detail_level_lock()
{ 
	return Detail_level_locked; 
}

float model_render_params::get_depth_scale()
{ 
	return Depth_scale; 
}

int model_render_params::get_warp_bitmap()
{ 
	return Warp_bitmap; 
}

float model_render_params::get_warp_alpha()
{ 
	return Warp_alpha; 
}

const vec3d& model_render_params::get_warp_scale()
{ 
	return Warp_scale; 
}

const color& model_render_params::get_outline_color()
{ 
	return Outline_color; 
}
float model_render_params::get_alpha()
{ 
	return Xparent_alpha; 
}

int model_render_params::get_forced_bitmap()
{ 
	return Forced_bitmap; 
}

int model_render_params::get_insignia_bitmap()
{ 
	return Insignia_bitmap; 
}

const int* model_render_params::get_replacement_textures()
{ 
	return Replacement_textures; 
}

const team_color& model_render_params::get_team_color()
{ 
	return Current_team_color; 
}

const vec3d& model_render_params::get_clip_plane_pos()
{ 
	return Clip_pos; 
}

const vec3d& model_render_params::get_clip_plane_normal()
{ 
	return Clip_normal; 
}

int model_render_params::get_animated_effect_num()
{ 
	return Animated_effect; 
}

float model_render_params::get_animated_effect_timer()
{ 
	return Animated_timer; 
}

void model_render_params::set_animated_effect(int effect_num, float timer)
{
	Animated_effect = effect_num;
	Animated_timer = timer;
}

void model_render_params::set_clip_plane(vec3d &pos, vec3d &normal)
{
	Clip_plane_set = true;

	Clip_normal = normal;
	Clip_pos = pos;
}

bool model_render_params::is_clip_plane_set()
{
	return Clip_plane_set;
}

void model_render_params::set_team_color(team_color &clr)
{
	Team_color_set = true;

	Current_team_color = clr;
}

void model_render_params::set_team_color(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime)
{
	Team_color_set = model_get_team_color(&Current_team_color, team, secondaryteam, timestamp, fadetime);
}

bool model_render_params::is_team_color_set()
{
	return Team_color_set;
}

void model_render_params::set_replacement_textures(int *textures)
{
	Replacement_textures = textures;
}

void model_render_params::set_insignia_bitmap(int bitmap)
{
	Insignia_bitmap = bitmap;
}

void model_render_params::set_forced_bitmap(int bitmap)
{
	Forced_bitmap = bitmap;
}

void model_render_params::set_alpha(float alpha)
{
	Xparent_alpha = alpha;
}

void model_render_params::set_outline_color(color &clr)
{
	Outline_color = clr;
}

void model_render_params::set_outline_color(int r, int g, int b)
{
	gr_init_color( &Outline_color, r, g, b );
}

void model_render_params::set_warp_params(int bitmap, float alpha, vec3d &scale)
{
	Warp_bitmap = bitmap;
	Warp_alpha = alpha;
	Warp_scale = scale;
}

void model_render_params::set_depth_scale(float scale)
{
	Depth_scale = scale;
}

void model_render_params::set_debug_flags(uint flags)
{
	Debug_flags = flags;
}

void model_render_params::set_object_number(int num)
{
	Objnum = num;
}

void model_render_params::set_flags(uint flags)
{
	Model_flags = flags;
}

void model_render_params::set_detail_level_lock(int detail_level_lock)
{
	Detail_level_locked = detail_level_lock;
}

void model_render_params::set_thruster_info(mst_info &info)
{
	Thruster_info = info;

	CLAMP(Thruster_info.length.xyz.z, 0.1f, 1.0f);
}

const mst_info& model_render_params::get_thruster_info()
{
	return Thruster_info;
}

void model_batch_buffer::reset()
{
	Submodel_matrices.clear();

	Current_offset = 0;
}

void model_batch_buffer::set_num_models(int n_models)
{
	matrix4 init_mat;

	memset(&init_mat, 0, sizeof(matrix4));

	init_mat.a1d[0] = 1.0f;
	init_mat.a1d[5] = 1.0f;
	init_mat.a1d[10] = 1.0f;
	init_mat.a1d[15] = 1.0f;	// set this to zero to indicate it's not to be drawn in the shader

	Current_offset = Submodel_matrices.size();

	for ( int i = 0; i < n_models; ++i ) {
		Submodel_matrices.push_back(init_mat);
	}
}

void model_batch_buffer::set_model_transform(matrix4 &transform, int model_id)
{
	Submodel_matrices[Current_offset + model_id] = transform;
}

void model_batch_buffer::add_matrix(matrix4 &mat)
{
	Submodel_matrices.push_back(mat);
}

int model_batch_buffer::get_buffer_offset()
{
	return Current_offset;
}

void model_batch_buffer::allocate_memory()
{
	uint size = Submodel_matrices.size() * sizeof(matrix4);

	if ( Mem_alloc == NULL || Mem_alloc_size < size ) {
		if ( Mem_alloc != NULL ) {
			vm_free(Mem_alloc);
		}

		Mem_alloc = vm_malloc(size);
	}

	Mem_alloc_size = size;
	memcpy(Mem_alloc, &Submodel_matrices[0], size);
}

void model_batch_buffer::submit_buffer_data()
{
	if ( Submodel_matrices.size() == 0 ) {
		return;
	}

	allocate_memory();

	gr_update_transform_buffer(Mem_alloc, Mem_alloc_size);
}

draw_list::draw_list()
{
	reset();
}

void draw_list::reset()
{
	Current_set_clip_plane = -1;

	Dirty_render_state = true;

	Current_render_state = render_state();

	Current_textures[TM_BASE_TYPE] = -1;
	Current_textures[TM_GLOW_TYPE] = -1;
	Current_textures[TM_SPECULAR_TYPE] = -1;
	Current_textures[TM_NORMAL_TYPE] = -1;
	Current_textures[TM_HEIGHT_TYPE] = -1;
	Current_textures[TM_MISC_TYPE] = -1;

	Clip_planes.clear();
	Render_states.clear();
	Render_elements.clear();
	Render_keys.clear();

	clear_transforms();

	Current_scale.xyz.x = 1.0f;
	Current_scale.xyz.y = 1.0f;
	Current_scale.xyz.z = 1.0f;
}

void draw_list::sort_draws()
{
	Target = this;
	std::sort(Target->Render_keys.begin(), Target->Render_keys.end(), draw_list::sort_draw_pair);
}

void draw_list::start_model_batch(int n_models)
{
	TransformBufferHandler.set_num_models(n_models);
}

void draw_list::add_submodel_to_batch(int model_num)
{
	matrix4 transform;

	memset(&transform, 0, sizeof(matrix4));

	// set basis
	transform.a1d[0] = Current_transform.basis.a1d[0] * Current_scale.xyz.x;
	transform.a1d[1] = Current_transform.basis.a1d[1];
	transform.a1d[2] = Current_transform.basis.a1d[2];

	transform.a1d[4] = Current_transform.basis.a1d[3];
	transform.a1d[5] = Current_transform.basis.a1d[4] * Current_scale.xyz.y;
	transform.a1d[6] = Current_transform.basis.a1d[5];

	transform.a1d[8] = Current_transform.basis.a1d[6];
	transform.a1d[9] = Current_transform.basis.a1d[7];
	transform.a1d[10] = Current_transform.basis.a1d[8] * Current_scale.xyz.z;

	// set position
	transform.a1d[12] = Current_transform.origin.a1d[0];
	transform.a1d[13] = Current_transform.origin.a1d[1];
	transform.a1d[14] = Current_transform.origin.a1d[2];

	// set visibility
	transform.a1d[15] = 0.0f;

	TransformBufferHandler.set_model_transform(transform, model_num);
}

void draw_list::set_depth_mode(int depth_set)
{
// 	if ( !dirty_render_state && depth_set != current_render_state.depth_mode ) {
// 		dirty_render_state = true;
// 	}

	Current_depth_mode = depth_set;
}

void draw_list::add_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width)
{
	arc_effect new_arc;

	new_arc.transformation = Current_transform;
	new_arc.v1 = *v1;
	new_arc.v2 = *v2;
	new_arc.primary = *primary;
	new_arc.secondary = *secondary;
	new_arc.width = arc_width;

	Arcs.push_back(new_arc);
}

void draw_list::set_light_filter(int objnum, vec3d *pos, float rad)
{
	Scene_light_handler.setLightFilter(objnum, pos, rad);

	Dirty_render_state = true;
	Current_render_state.lights = Scene_light_handler.bufferLights();
}

void draw_list::set_light_factor(float factor)
{
	Current_render_state.light_factor = factor;
}

void draw_list::set_clip_plane(const vec3d &position, const vec3d &normal)
{
	clip_plane_state clip_normal;

	clip_normal.point = position;
	clip_normal.normal = normal;

	Clip_planes.push_back(clip_normal);

	Current_render_state.clip_plane_handle = Clip_planes.size() - 1;
}

void draw_list::set_clip_plane()
{
	Current_render_state.clip_plane_handle = -1;
}

void draw_list::set_thrust_scale(float scale)
{
	Current_render_state.thrust_scale = scale;
}

void draw_list::add_buffer_draw(vertex_buffer *buffer, int texi, uint tmap_flags, model_render_params *interp)
{
	// need to do a check to see if the top render state matches the current.
	//if ( dirty_render_state ) {
		Render_states.push_back(Current_render_state);
	//}

	Dirty_render_state = false;

	queued_buffer_draw draw_data;

	draw_data.render_state_handle = Render_states.size() - 1;
	draw_data.buffer = buffer;
	draw_data.texi = texi;
	draw_data.flags = tmap_flags;

	draw_data.clr = gr_screen.current_color;
	draw_data.alpha = Current_alpha;
	draw_data.blend_filter = Current_blend_filter;
	draw_data.depth_mode = Current_depth_mode;

	if ( tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS ) {
 		draw_data.transformation = transform();
 
  		draw_data.scale.xyz.x = 1.0f;
  		draw_data.scale.xyz.y = 1.0f;
  		draw_data.scale.xyz.z = 1.0f;

		draw_data.transform_buffer_offset = TransformBufferHandler.get_buffer_offset();
	} else {
		draw_data.transformation = Current_transform;
		draw_data.scale = Current_scale;
		draw_data.transform_buffer_offset = -1;
	}
	
	draw_data.texture_maps[TM_BASE_TYPE] = Current_textures[TM_BASE_TYPE];
	draw_data.texture_maps[TM_GLOW_TYPE] = Current_textures[TM_GLOW_TYPE];
	draw_data.texture_maps[TM_SPECULAR_TYPE] = Current_textures[TM_SPECULAR_TYPE];
	draw_data.texture_maps[TM_NORMAL_TYPE] = Current_textures[TM_NORMAL_TYPE];
	draw_data.texture_maps[TM_HEIGHT_TYPE] = Current_textures[TM_HEIGHT_TYPE];
	draw_data.texture_maps[TM_MISC_TYPE] = Current_textures[TM_MISC_TYPE];

	draw_data.sdr_flags = determine_shader_flags(&Current_render_state, &draw_data, buffer, tmap_flags);

	Render_elements.push_back(draw_data);

	Render_keys.push_back(Render_elements.size() - 1);
}

uint draw_list::determine_shader_flags(render_state *state, queued_buffer_draw *draw_info, vertex_buffer *buffer, int tmap_flags)
{
	bool texture = (tmap_flags & TMAP_FLAG_TEXTURED) && (buffer->flags & VB_FLAG_UV1);
	bool fog = false;
	bool use_thrust_scale = false;

	if ( state->fog_mode == GR_FOGMODE_FOG ) {
		fog = true;
	}

	if ( draw_info->thrust_scale > 0.0f ) {
		use_thrust_scale = true;
	}

	return gr_determine_model_shader_flags(
		state->lighting, 
		fog, 
		texture, 
		Rendering_to_shadow_map, 
		use_thrust_scale,
		tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS && draw_info->transform_buffer_offset >= 0 && buffer->flags & VB_FLAG_MODEL_ID,
		state->using_team_color,
		tmap_flags, 
		draw_info->texture_maps[TM_SPECULAR_TYPE],
		draw_info->texture_maps[TM_GLOW_TYPE],
		draw_info->texture_maps[TM_NORMAL_TYPE],
		draw_info->texture_maps[TM_HEIGHT_TYPE],
		ENVMAP,
		draw_info->texture_maps[TM_MISC_TYPE]
	);
}

void draw_list::render_buffer(queued_buffer_draw &render_elements)
{
	// get the render state for this draw call
	int render_state_num = render_elements.render_state_handle;

	render_state &draw_state = Render_states[render_state_num];

	// set clip plane if necessary
	if ( draw_state.clip_plane_handle >= 0 && draw_state.clip_plane_handle != Current_set_clip_plane ) {
		if ( Current_set_clip_plane >= 0 ) {
			g3_stop_user_clip_plane();
		}

		Current_set_clip_plane = draw_state.clip_plane_handle;

		clip_plane_state *clip_plane = &Clip_planes[Current_set_clip_plane];

		g3_start_user_clip_plane(&clip_plane->point, &clip_plane->normal);
	} else if ( draw_state.clip_plane_handle < 0 && Current_set_clip_plane >= 0 ) {
		// stop the clip plane if this draw call doesn't have clip plane and clip plane is set.
		Current_set_clip_plane = -1;
		g3_stop_user_clip_plane();
	}

	gr_set_animated_effect(draw_state.animated_effect, draw_state.animated_timer);

	if ( draw_state.using_team_color ) {
		gr_set_team_color(&draw_state.tm_color);
	} else {
		gr_set_team_color(NULL);
	}

	gr_set_texture_addressing(draw_state.texture_addressing);

	gr_fog_set(draw_state.fog_mode, draw_state.r, draw_state.g, draw_state.b, draw_state.fog_near, draw_state.fog_far);

	gr_zbuffer_set(render_elements.depth_mode);
	
	gr_set_cull(draw_state.cull_mode);

	gr_set_fill_mode(draw_state.fill_mode);

	gr_center_alpha(draw_state.center_alpha);

	gr_set_transform_buffer_offset(render_elements.transform_buffer_offset);

	gr_set_color_fast(&draw_state.clr);

	gr_set_light_factor(draw_state.light_factor);

	if ( draw_state.lighting ) {
		Scene_light_handler.setLights(&draw_state.lights);
	} else {
		gr_set_lighting(false, false);

		Scene_light_handler.resetLightState();
	}

	gr_set_buffer(draw_state.buffer_id);

	gr_zbias(draw_state.zbias);

	gr_set_thrust_scale(draw_state.thrust_scale);

	g3_start_instance_matrix(&render_elements.transformation.origin, &render_elements.transformation.basis);

	gr_push_scale_matrix(&render_elements.scale);

	gr_set_bitmap(render_elements.texture_maps[TM_BASE_TYPE], render_elements.blend_filter, GR_BITBLT_MODE_NORMAL, render_elements.alpha);

	GLOWMAP = render_elements.texture_maps[TM_GLOW_TYPE];
	SPECMAP = render_elements.texture_maps[TM_SPECULAR_TYPE];
	NORMMAP = render_elements.texture_maps[TM_NORMAL_TYPE];
	HEIGHTMAP = render_elements.texture_maps[TM_HEIGHT_TYPE];
	MISCMAP = render_elements.texture_maps[TM_MISC_TYPE];

	gr_render_buffer(0, render_elements.buffer, render_elements.texi, render_elements.flags);

	GLOWMAP = -1;
	SPECMAP = -1;
	NORMMAP = -1;
	HEIGHTMAP = -1;
	MISCMAP = -1;

	gr_pop_scale_matrix();

	g3_done_instance(true);
}

vec3d draw_list::get_view_position()
{
	matrix basis_world;

	// get the world basis of our current local space.
	vm_matrix_x_matrix(&basis_world, &Object_matrix, &Current_transform.basis);

	vec3d eye_pos_local;
	vm_vec_sub(&eye_pos_local, &Eye_position, &Current_transform.origin);

	vec3d return_val;
	vm_vec_rotate(&return_val, &eye_pos_local, &basis_world);

	return return_val;
}

void draw_list::clear_transforms()
{
	Current_transform = transform();
	Transform_stack.clear();
}

void draw_list::push_transform(vec3d *pos, matrix *orient)
{
	matrix basis;
	vec3d origin;

	if ( orient == NULL ) {
		basis = vmd_identity_matrix;
	} else {
		basis = *orient;
	}

	if ( pos == NULL ) {
		origin = vmd_zero_vector;
	} else {
		origin = *pos;
	}

	if ( Transform_stack.size() == 0 ) {
		Current_transform.basis = basis;
		Current_transform.origin = origin;

		Transform_stack.push_back(Current_transform);

		return;
	}

	vec3d tempv;
	transform newTransform = Current_transform;

	vm_vec_unrotate(&tempv, &origin, &Current_transform.basis);
	vm_vec_add2(&newTransform.origin, &tempv);

	vm_matrix_x_matrix(&newTransform.basis, &Current_transform.basis, &basis);

	Current_transform = newTransform;
	Transform_stack.push_back(Current_transform);
}

void draw_list::pop_transform()
{
	Assert( Transform_stack.size() > 0 );

	Transform_stack.pop_back();

	if ( Transform_stack.size() > 0 ) {
		Current_transform = Transform_stack.back();
	} else {
		Current_transform = transform();
	}
}

void draw_list::set_scale(vec3d *scale)
{
	if ( scale == NULL ) {
		Current_scale.xyz.x = 1.0f;
		Current_scale.xyz.y = 1.0f;
		Current_scale.xyz.z = 1.0f;
		return;
	}

	Current_scale = *scale;
}

void draw_list::set_buffer(int buffer)
{
// 	if ( !dirty_render_state && current_render_state.buffer_id != buffer) {
// 		dirty_render_state = true;
// 	}

	Current_render_state.buffer_id = buffer;
}

void draw_list::set_blend_filter(int filter, float alpha)
{
// 	if ( !dirty_render_state && ( current_render_state.alpha != alpha && current_render_state.blend_filter != filter ) ) {
// 		dirty_render_state = true;
// 	}

// 	current_blend_filter = GR_ALPHABLEND_NONE;
// 	current_alpha = 1.0f;

	Current_blend_filter = filter;
	Current_alpha = alpha;
}

void draw_list::set_texture(int texture_type, int texture_handle)
{
	Assert(texture_type > -1);
	Assert(texture_type < TM_NUM_TYPES);

	Current_textures[texture_type] = texture_handle;
}

void draw_list::set_cull_mode(int mode)
{
	Current_render_state.cull_mode = mode;
}

void draw_list::set_zbias(int bias)
{
	Current_render_state.zbias = bias;
}

void draw_list::set_lighting(bool lighting)
{
	Current_render_state.lighting = lighting;
}

void draw_list::set_team_color(const team_color &color)
{
	Current_render_state.using_team_color = true;
	Current_render_state.tm_color = color;
}

void draw_list::set_team_color()
{
	Current_render_state.using_team_color = false;
}

void draw_list::set_color(const color &clr)
{
	Current_render_state.clr = clr;
}

void draw_list::set_animated_timer(float time)
{
	Current_render_state.animated_timer = time;
}

void draw_list::set_animated_effect(int effect)
{
	Current_render_state.animated_effect = effect;
}

void draw_list::set_texture_addressing(int addressing)
{
	Current_render_state.texture_addressing = addressing;
}

void draw_list::set_fog(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
	Current_render_state.fog_mode = fog_mode;
	Current_render_state.r = r;
	Current_render_state.g = g;
	Current_render_state.b = b;
	Current_render_state.fog_near = fog_near;
	Current_render_state.fog_far = fog_far;
}

void draw_list::set_fill_mode(int mode)
{
	Current_render_state.fill_mode = mode;
}

void draw_list::set_center_alpha(int center_alpha)
{
	Current_render_state.center_alpha = center_alpha;
}

void draw_list::init()
{
	reset();

	for ( int i = 0; i < Num_lights; ++i ) {
		if ( Lights[i].type == LT_DIRECTIONAL || !Deferred_lighting ) {
			Scene_light_handler.addLight(&Lights[i]);
		}	
	}

	TransformBufferHandler.reset();
}

void draw_list::init_render()
{
	sort_draws();

	Scene_light_handler.resetLightState();
	TransformBufferHandler.submit_buffer_data();
}

void draw_list::render_all(int depth_mode)
{
	for ( size_t i = 0; i < Render_keys.size(); ++i ) {
		int render_index = Render_keys[i];

		if ( depth_mode == -1 || Render_elements[render_index].depth_mode == depth_mode ) {
			render_buffer(Render_elements[render_index]);
		}
	}

	if ( Current_set_clip_plane >= 0 ) {
		g3_stop_user_clip_plane();
	}

	gr_alpha_mask_set(0, 1.0f);
}

void draw_list::render_arc(arc_effect &arc)
{
	g3_start_instance_matrix(&arc.transformation.origin, &arc.transformation.basis);	

	interp_render_arc(&arc.v1, &arc.v2, &arc.primary, &arc.secondary, arc.width);

	g3_done_instance(true);
}

void draw_list::render_arcs()
{
	int mode = gr_zbuffer_set(GR_ZBUFF_READ);

	for ( size_t i = 0; i < Arcs.size(); ++i ) {
		render_arc(Arcs[i]);
	}

	gr_zbuffer_set(mode);
}

void draw_list::add_insignia(polymodel *pm, int detail_level, int bitmap_num)
{
	insignia_draw_data new_insignia;

	new_insignia.transformation = Current_transform;
	new_insignia.pm = pm;
	new_insignia.detail_level = detail_level;
	new_insignia.bitmap_num = bitmap_num;

	new_insignia.clip_plane = Current_render_state.clip_plane_handle;

	Insignias.push_back(new_insignia);
}

void draw_list::render_insignia(insignia_draw_data &insignia_info)
{
	if ( insignia_info.clip_plane >= 0 ) {
		clip_plane_state &plane_data = Clip_planes[insignia_info.clip_plane];

		vec3d tmp;
		vm_vec_sub(&tmp, &insignia_info.transformation.origin, &plane_data.point);
		vm_vec_normalize(&tmp);

		if ( vm_vec_dot(&tmp, &plane_data.normal) < 0.0f) {
			return;
		}
	}

	g3_start_instance_matrix(&insignia_info.transformation.origin, &insignia_info.transformation.basis);	

	model_render_insignias(insignia_info.pm, insignia_info.detail_level, insignia_info.bitmap_num);

	g3_done_instance(true);
}

void draw_list::render_insignias()
{
	int mode = gr_zbuffer_set(GR_ZBUFF_READ);
	gr_zbias(1);

	for ( size_t i = 0; i < Insignias.size(); ++i ) {
		render_insignia(Insignias[i]);
	}

	gr_zbias(0);
	gr_zbuffer_set(mode);
}

void draw_list::add_outline(vertex* vert_array, int n_verts, color *clr)
{
	outline_draw draw_info;

	draw_info.vert_array = vert_array;
	draw_info.n_verts = n_verts;
	draw_info.clr = *clr;
	draw_info.transformation = Current_transform;

	Outlines.push_back(draw_info);
}

void draw_list::render_outlines()
{
	gr_clear_states();
	int mode = gr_zbuffer_set(GR_ZBUFF_READ);

	for ( size_t i = 0; i < Outlines.size(); ++i ) {
		render_outline(Outlines[i]);
	}

	gr_zbuffer_set(mode);
}

void draw_list::render_outline(outline_draw &outline_info)
{
	g3_start_instance_matrix(&outline_info.transformation.origin, &outline_info.transformation.basis);

	gr_set_color_fast(&outline_info.clr);

	gr_render(outline_info.n_verts, outline_info.vert_array, TMAP_HTL_3D_UNLIT | TMAP_FLAG_LINES);

	g3_done_instance(true);
}

bool draw_list::sort_draw_pair(const int a, const int b)
{
	queued_buffer_draw *draw_call_a = &Target->Render_elements[a];
	queued_buffer_draw *draw_call_b = &Target->Render_elements[b];

	render_state *render_state_a = &Target->Render_states[draw_call_a->render_state_handle];
	render_state *render_state_b = &Target->Render_states[draw_call_b->render_state_handle];

	if ( draw_call_a->depth_mode != draw_call_b->depth_mode ) {
		return draw_call_a->depth_mode > draw_call_b->depth_mode;
	}

	if ( render_state_a->clip_plane_handle != render_state_b->clip_plane_handle ) {
		return render_state_a->clip_plane_handle < render_state_b->clip_plane_handle;
	}
	
	if ( draw_call_a->blend_filter != draw_call_b->blend_filter ) {
		return draw_call_a->blend_filter < draw_call_b->blend_filter;
	}

	if ( draw_call_a->sdr_flags != draw_call_b->sdr_flags ) {
		return draw_call_a->sdr_flags < draw_call_b->sdr_flags;
	}
	
	if ( render_state_a->buffer_id != render_state_b->buffer_id) {
		return render_state_a->buffer_id < render_state_b->buffer_id;
	}

	if ( draw_call_a->texture_maps[TM_BASE_TYPE] != draw_call_b->texture_maps[TM_BASE_TYPE] ) {
		return draw_call_a->texture_maps[TM_BASE_TYPE] < draw_call_b->texture_maps[TM_BASE_TYPE];
	}

	if ( draw_call_a->texture_maps[TM_SPECULAR_TYPE] != draw_call_b->texture_maps[TM_SPECULAR_TYPE] ) {
		return draw_call_a->texture_maps[TM_SPECULAR_TYPE] < draw_call_b->texture_maps[TM_SPECULAR_TYPE];
	}

	if ( draw_call_a->texture_maps[TM_GLOW_TYPE] != draw_call_b->texture_maps[TM_GLOW_TYPE] ) {
		return draw_call_a->texture_maps[TM_GLOW_TYPE] < draw_call_b->texture_maps[TM_GLOW_TYPE];
	}

	if ( draw_call_a->texture_maps[TM_NORMAL_TYPE] != draw_call_b->texture_maps[TM_NORMAL_TYPE] ) {
		return draw_call_a->texture_maps[TM_NORMAL_TYPE] < draw_call_b->texture_maps[TM_NORMAL_TYPE];
	}

	if ( draw_call_a->texture_maps[TM_HEIGHT_TYPE] != draw_call_b->texture_maps[TM_HEIGHT_TYPE] ) {
		return draw_call_a->texture_maps[TM_HEIGHT_TYPE] < draw_call_b->texture_maps[TM_HEIGHT_TYPE];
	}

	if ( draw_call_a->texture_maps[TM_MISC_TYPE] != draw_call_b->texture_maps[TM_MISC_TYPE] ) {
		return draw_call_a->texture_maps[TM_MISC_TYPE] < draw_call_b->texture_maps[TM_MISC_TYPE];
	}

	return render_state_a->lights.index_start < render_state_b->lights.index_start;
}

void model_render_add_lightning( draw_list *scene, model_render_params* interp, polymodel *pm, bsp_info * sm )
{
	int i;
	float width = 0.9f;
	color primary, secondary;

	const int AR = 64;
	const int AG = 64;
	const int AB = 5;
	const int AR2 = 128;
	const int AG2 = 128;
	const int AB2 = 10;

	Assert( sm->num_arcs > 0 );

	if ( interp->get_model_flags() & MR_SHOW_OUTLINE_PRESET ) {
		return;
	}

	extern int Interp_lightning;
	if ( !Interp_lightning ) {
 		return;
 	}

	// try and scale the size a bit so that it looks equally well on smaller vessels
	if ( pm->rad < 500.0f ) {
		width *= (pm->rad * 0.01f);

		if ( width < 0.2f ) {
			width = 0.2f;
		}
	}

	for ( i = 0; i < sm->num_arcs; i++ ) {
		// pick a color based upon arc type
		switch ( sm->arc_type[i] ) {
			// "normal", FreeSpace 1 style arcs
		case MARC_TYPE_NORMAL:
			if ( (rand()>>4) & 1 )	{
				gr_init_color(&primary, 64, 64, 255);
			} else {
				gr_init_color(&primary, 128, 128, 255);
			}

			gr_init_color(&secondary, 200, 200, 255);
			break;

			// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( (rand()>>4) & 1 )	{
				gr_init_color(&primary, AR, AG, AB);
			} else {
				gr_init_color(&primary, AR2, AG2, AB2);
			}

			gr_init_color(&secondary, 255, 255, 10);
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		scene->add_arc(&sm->arc_pts[i][0], &sm->arc_pts[i][1], &primary, &secondary, width);
	}
}

float model_render_determine_depth(int obj_num, int model_num, matrix* orient, vec3d* pos, int detail_level_locked)
{
	vec3d closest_pos;
	float depth = model_find_closest_point( &closest_pos, model_num, -1, orient, pos, &Eye_position );

	if ( detail_level_locked < 0 ) {
		switch (Detail.detail_distance) {
		case 0:		// lowest
			depth /= The_mission.ai_profile->detail_distance_mult[0];
			break;
		case 1:		// lower than normal
			depth /= The_mission.ai_profile->detail_distance_mult[1];
			break;
		case 2:		// default
			depth /= The_mission.ai_profile->detail_distance_mult[2];
			break;
		case 3:		// above normal
			depth /= The_mission.ai_profile->detail_distance_mult[3];
			break;
		case 4:		// even more normal
			depth /= The_mission.ai_profile->detail_distance_mult[4];
			break;
		}

		// nebula ?
		if (The_mission.flags & MISSION_FLAG_FULLNEB) {
			depth *= neb2_get_lod_scale(obj_num);
		}

	}

	return depth;
}

int model_render_determine_detail(float depth, int obj_num, int model_num, matrix* orient, vec3d* pos, int flags, int detail_level_locked)
{
	int tmp_detail_level = Game_detail_level;

	polymodel *pm = model_get(model_num);

	Assert( pm->n_detail_levels < MAX_MODEL_DETAIL_LEVELS );

	int i;

	if ( pm->n_detail_levels > 1 ) {
		if ( detail_level_locked >= 0 ) {
			i = detail_level_locked+1;
		} else {

#if MAX_DETAIL_LEVEL != 4
#error Code in modelrender.cpp assumes MAX_DETAIL_LEVEL == 4
#endif
			for ( i = 0; i < pm->n_detail_levels; i++ )	{
				if ( depth <= pm->detail_depth[i] ) {
					break;
				}
			}

			// If no valid detail depths specified, use highest.
			if ( (i > 1) && (pm->detail_depth[i-1] < 1.0f) )	{
				i = 1;
			}
		}

		int detail_level = i - 1 - tmp_detail_level;

		if ( detail_level < 0 ) {
			return 0;
		} else if ( detail_level >= pm->n_detail_levels ) {
			return pm->n_detail_levels - 1;
		}

		return detail_level;
	} else {
		return 0;
	}
}

void model_render_buffers(draw_list* scene, model_render_params* interp, vertex_buffer *buffer, polymodel *pm, int mn, int detail_level, uint tmap_flags)
{
	if ( pm->vertex_buffer_id < 0 ) {
		return;
	}

	bsp_info *model = NULL;
	const uint model_flags = interp->get_model_flags();
	const int obj_num = interp->get_object_number();

	Assert(buffer != NULL);
	Assert(detail_level >= 0);

	if ( (mn >= 0) && (mn < pm->n_models) ) {
		model = &pm->submodel[mn];
	}

	bool render_as_thruster = (model != NULL) && model->is_thruster && (model_flags & MR_SHOW_THRUSTERS);

	vec3d scale;

	if ( render_as_thruster ) {
		scale.xyz.x = 1.0f;
		scale.xyz.y = 1.0f;

		if ( Use_GLSL > 1 ) {
			scale.xyz.z = 1.0f;
			scene->set_thrust_scale(interp->get_thruster_info().length.xyz.z);
		} else {
			scale.xyz.z = interp->get_thruster_info().length.xyz.z;
			scene->set_thrust_scale();
		}
	} else {
		scale = interp->get_warp_scale();
		scene->set_thrust_scale();
	}

	scene->set_scale(&scale);

	if ( tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS && (mn >= 0) && (mn < pm->n_models) ) {
		scene->add_submodel_to_batch(mn);
		return;
	}

	fix base_frametime = model_render_determine_base_frametime(obj_num, model_flags);

	texture_info tex_replace[TM_NUM_TYPES];

	int no_texturing = model_flags & MR_NO_TEXTURING;

	int forced_texture = -2;
	float forced_alpha = 1.0f;
	int forced_blend_filter = GR_ALPHABLEND_NONE;

	if ( interp->get_forced_bitmap() >= 0 ) {
		forced_texture = interp->get_forced_bitmap();
	} else if ( interp->get_warp_bitmap() >= 0 ) {
		forced_texture = interp->get_warp_bitmap();
		forced_alpha = interp->get_warp_alpha();
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	} else if ( render_as_thruster ) {
		if ( ( interp->get_thruster_info().primary_bitmap >= 0 ) && ( interp->get_thruster_info().length.xyz.z > 0.0f ) ) {
			forced_texture = interp->get_thruster_info().primary_bitmap;
		} else {
			forced_texture = -1;
		}

		forced_alpha = 1.2f;
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	} else if ( model_flags & MR_ALL_XPARENT ) {
		forced_alpha = interp->get_alpha();
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	}

	int texture_maps[TM_NUM_TYPES] = {-1, -1, -1, -1, -1, -1};
	size_t buffer_size = buffer->tex_buf.size();
	const int *replacement_textures = interp->get_replacement_textures();

	for ( size_t i = 0; i < buffer_size; i++ ) {
		int tmap_num = buffer->tex_buf[i].texture;
		texture_map *tmap = &pm->maps[tmap_num];
		int rt_begin_index = tmap_num*TM_NUM_TYPES;
		float alpha = 1.0f;
		int blend_filter = GR_ALPHABLEND_NONE;

		texture_maps[TM_BASE_TYPE] = -1;
		texture_maps[TM_GLOW_TYPE] = -1;
		texture_maps[TM_SPECULAR_TYPE] = -1;
		texture_maps[TM_NORMAL_TYPE] = -1;
		texture_maps[TM_HEIGHT_TYPE] = -1;
		texture_maps[TM_MISC_TYPE] = -1;

		if (forced_texture != -2) {
			texture_maps[TM_BASE_TYPE] = forced_texture;
			alpha = forced_alpha;
		} else if ( !no_texturing ) {
			// pick the texture, animating it if necessary
			if ( (replacement_textures != NULL) && (replacement_textures[rt_begin_index + TM_BASE_TYPE] == REPLACE_WITH_INVISIBLE) ) {
				// invisible textures aren't rendered, but we still have to skip assigning the underlying model texture
				texture_maps[TM_BASE_TYPE] = -1;
			} else if ( (replacement_textures != NULL) && (replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0) ) {
				// an underlying texture is replaced with a real new texture
				tex_replace[TM_BASE_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_BASE_TYPE]);
				texture_maps[TM_BASE_TYPE] = model_interp_get_texture(&tex_replace[TM_BASE_TYPE], base_frametime);
			} else {
				// we just use the underlying texture
				texture_maps[TM_BASE_TYPE] = model_interp_get_texture(&tmap->textures[TM_BASE_TYPE], base_frametime);
			}

			if ( texture_maps[TM_BASE_TYPE] < 0 ) {
				continue;
			}

			// doing glow maps?
			if ( !(model_flags & MR_NO_GLOWMAPS) ) {
				texture_info *tglow = &tmap->textures[TM_GLOW_TYPE];

				if ( (replacement_textures != NULL) && (replacement_textures[rt_begin_index + TM_GLOW_TYPE] >= 0) ) {
					tex_replace[TM_GLOW_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_GLOW_TYPE]);
					texture_maps[TM_GLOW_TYPE] = model_interp_get_texture(&tex_replace[TM_GLOW_TYPE], base_frametime);
				} else if (tglow->GetTexture() >= 0) {
					// shockwaves are special, their current frame has to come out of the shockwave code to get the timing correct
					if ( (obj_num >= 0) && (Objects[obj_num].type == OBJ_SHOCKWAVE) && (tglow->GetNumFrames() > 1) ) {
						texture_maps[TM_GLOW_TYPE] = tglow->GetTexture() + shockwave_get_framenum(Objects[obj_num].instance, tglow->GetNumFrames());
					} else {
						texture_maps[TM_GLOW_TYPE] = model_interp_get_texture(tglow, base_frametime);
					}
				}
			}

			if ( (Detail.lighting > 2)  && (detail_level < 2) ) {
				// likewise, etc.
				texture_info *spec_map = &tmap->textures[TM_SPECULAR_TYPE];
				texture_info *norm_map = &tmap->textures[TM_NORMAL_TYPE];
				texture_info *height_map = &tmap->textures[TM_HEIGHT_TYPE];
				texture_info *misc_map = &tmap->textures[TM_MISC_TYPE];

				if (replacement_textures != NULL) {
					if (replacement_textures[rt_begin_index + TM_SPECULAR_TYPE] >= 0) {
						tex_replace[TM_SPECULAR_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_SPECULAR_TYPE]);
						spec_map = &tex_replace[TM_SPECULAR_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_NORMAL_TYPE] >= 0) {
						tex_replace[TM_NORMAL_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_NORMAL_TYPE]);
						norm_map = &tex_replace[TM_NORMAL_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_HEIGHT_TYPE] >= 0) {
						tex_replace[TM_HEIGHT_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_HEIGHT_TYPE]);
						height_map = &tex_replace[TM_HEIGHT_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_MISC_TYPE] >= 0) {
						tex_replace[TM_MISC_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_MISC_TYPE]);
						misc_map = &tex_replace[TM_MISC_TYPE];
					}
				}

				texture_maps[TM_SPECULAR_TYPE] = model_interp_get_texture(spec_map, base_frametime);
				texture_maps[TM_NORMAL_TYPE] = model_interp_get_texture(norm_map, base_frametime);
				texture_maps[TM_HEIGHT_TYPE] = model_interp_get_texture(height_map, base_frametime);
				texture_maps[TM_MISC_TYPE] = model_interp_get_texture(misc_map, base_frametime);
			}
		} else {
			alpha = forced_alpha;

			//Check for invisible or transparent textures so they don't show up in the shadow maps - Valathil
			if ( Rendering_to_shadow_map ) {
				if ( (replacement_textures != NULL) && (replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0) ) {
					tex_replace[TM_BASE_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_BASE_TYPE]);
					texture_maps[TM_BASE_TYPE] = model_interp_get_texture(&tex_replace[TM_BASE_TYPE], base_frametime);
				} else {
					texture_maps[TM_BASE_TYPE] = model_interp_get_texture(&tmap->textures[TM_BASE_TYPE], base_frametime);
				}

				if ( texture_maps[TM_BASE_TYPE] <= 0 ) {
					continue;
				}
			}
		}

		if ( (texture_maps[TM_BASE_TYPE] == -1) && !no_texturing ) {
			continue;
		}

		uint alpha_flag = 0;

		// trying to get transparent textures-Bobboau
		if (tmap->is_transparent) {
			// for special shockwave/warp map usage
			alpha = (interp->get_warp_alpha() != -1.0f) ? interp->get_warp_alpha() : 0.8f;
			blend_filter = GR_ALPHABLEND_FILTER;
		} else if ( buffer->flags & VB_FLAG_TRANS ) {
			blend_filter = GR_ALPHABLEND_FILTER;
		}

		if (forced_blend_filter != GR_ALPHABLEND_NONE) {
			blend_filter = forced_blend_filter;
		}

		if (blend_filter != GR_ALPHABLEND_NONE ) {
			scene->set_depth_mode(GR_ZBUFF_READ);
			alpha_flag |= TMAP_FLAG_ALPHA;
		} else {
			if ( (model_flags & MR_NO_ZBUFFER) || (model_flags & MR_ALL_XPARENT) ) {
				scene->set_depth_mode(GR_ZBUFF_NONE);
				alpha_flag |= TMAP_FLAG_ALPHA;
			} else {
				scene->set_depth_mode(GR_ZBUFF_FULL);
			}
		}

		scene->set_blend_filter(blend_filter, alpha);

		scene->set_texture(TM_BASE_TYPE,	texture_maps[TM_BASE_TYPE]);
		scene->set_texture(TM_GLOW_TYPE,	texture_maps[TM_GLOW_TYPE]);
		scene->set_texture(TM_SPECULAR_TYPE, texture_maps[TM_SPECULAR_TYPE]);
		scene->set_texture(TM_NORMAL_TYPE, texture_maps[TM_NORMAL_TYPE]);
		scene->set_texture(TM_HEIGHT_TYPE, texture_maps[TM_HEIGHT_TYPE]);
		scene->set_texture(TM_MISC_TYPE,	texture_maps[TM_MISC_TYPE]);

		scene->add_buffer_draw(buffer, i, tmap_flags | alpha_flag, interp);
	}
}

void model_render_children_buffers(draw_list* scene, model_render_params* interp, polymodel* pm, int mn, int detail_level, uint tmap_flags, bool trans_buffer)
{
	int i;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *model = &pm->submodel[mn];

	if (model->blown_off)
		return;

	const uint model_flags = interp->get_model_flags();

	if (model->is_thruster) {
		if ( !( model_flags & MR_SHOW_THRUSTERS ) ) {
			return;
		}

		scene->set_lighting(false);
	}

	vec3d view_pos = scene->get_view_position();

	if ( !model_render_check_detail_box(&view_pos, pm, mn, model_flags) ) {
		return;
	}

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	angles ang = model->angs;

	// Add barrel rotation if needed
	if ( model->gun_rotation ) {
		if ( pm->gun_submodel_rotation > PI2 ) {
			pm->gun_submodel_rotation -= PI2;
		} else if ( pm->gun_submodel_rotation < 0.0f ) {
			pm->gun_submodel_rotation += PI2;
		}

		ang.b += pm->gun_submodel_rotation;
	}

	// Compute final submodel orientation by using the orientation matrix
	// and the rotation angles.
	// By using this kind of computation, the rotational angles can always
	// be computed relative to the submodel itself, instead of relative
	// to the parent
	matrix rotation_matrix = model->orientation;
	vm_rotate_matrix_by_angles(&rotation_matrix, &ang);

	matrix inv_orientation;
	vm_copy_transpose_matrix(&inv_orientation, &model->orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	scene->push_transform(&model->offset, &submodel_matrix);
	
	if ( (model_flags & MR_SHOW_OUTLINE || model_flags & MR_SHOW_OUTLINE_HTL || model_flags & MR_SHOW_OUTLINE_PRESET) && 
		pm->submodel[mn].outline_buffer != NULL ) {
		color outline_color = interp->get_outline_color();
		scene->add_outline(pm->submodel[mn].outline_buffer, pm->submodel[mn].n_verts_outline, &outline_color);
	} else {
		if ( trans_buffer && pm->submodel[mn].trans_buffer.flags & VB_FLAG_TRANS ) {
			model_render_buffers(scene, interp, &pm->submodel[mn].trans_buffer, pm, mn, detail_level, tmap_flags);
		} else {
			model_render_buffers(scene, interp, &pm->submodel[mn].buffer, pm, mn, detail_level, tmap_flags);
		} 
	}

	if ( model->num_arcs ) {
		model_render_add_lightning( scene, interp, pm, &pm->submodel[mn] );
	}

	i = model->first_child;

	while ( i >= 0 ) {
		if ( !pm->submodel[i].is_thruster ) {
			model_render_children_buffers( scene, interp, pm, i, detail_level, tmap_flags, trans_buffer );
		}

		i = pm->submodel[i].next_sibling;
	}

	if ( model->is_thruster ) {
		scene->set_lighting(true);
	}

	scene->pop_transform();
}

float model_render_determine_light_factor(model_render_params* interp, vec3d *pos, uint flags)
{
	if ( flags & MR_IS_ASTEROID ) {
		// Dim it based on distance
		float depth = vm_vec_dist_quick( pos, &Eye_position );
		if ( depth > interp->get_depth_scale() )	{
			float temp_light = interp->get_depth_scale()/depth;

			if ( temp_light > 1.0f )	{
 				return 1.0f;
 			}

			return temp_light;
		}
	}
	
	return 1.0f;
}

float model_render_determine_box_scale()
{
	float box_scale = 1.2f;

	// scale the render box settings based on the "Model Detail" slider
	switch ( Detail.detail_distance ) {
	case 0:		// 1st dot is 20%
		box_scale = 0.2f;
		break;
	case 1:		// 2nd dot is 50%
		box_scale = 0.5f;
		break;
	case 2:		// 3rd dot is 80%
		box_scale = 0.8f;
		break;
	case 3:		// 4th dot is 100% (this is the default setting for "High" and "Very High" settings)
		box_scale = 1.0f;
		break;
	case 4:		// 5th dot (max) is 120%
	default:
		box_scale = 1.2f;
		break;
	}

	return box_scale;
}

fix model_render_determine_base_frametime(int objnum, uint flags)
{
	// Goober5000
	fix base_frametime = 0;

	if ( objnum >= 0 ) {
		object *objp = &Objects[objnum];

		if ( objp->type == OBJ_SHIP ) {
			base_frametime = Ships[objp->instance].base_texture_anim_frametime;
		}
	} else if ( flags & MR_SKYBOX ) {
		base_frametime = Skybox_timestamp;
	}

	return base_frametime;
}

bool model_render_determine_autocenter(vec3d *auto_back, polymodel *pm, int detail_level, uint flags)
{
	if ( flags & MR_AUTOCENTER ) {
		// standard autocenter using data in model
		if ( pm->flags & PM_FLAG_AUTOCEN ) {
			*auto_back = pm->autocenter;
			vm_vec_scale(auto_back, -1.0f);
			return true;
		} else if ( flags & MR_IS_MISSILE ) {
			// fake autocenter if we are a missile and don't already have autocen info
			auto_back->xyz.x = -( (pm->submodel[pm->detail[detail_level]].max.xyz.x + pm->submodel[pm->detail[detail_level]].min.xyz.x) / 2.0f );
			auto_back->xyz.y = -( (pm->submodel[pm->detail[detail_level]].max.xyz.y + pm->submodel[pm->detail[detail_level]].min.xyz.y) / 2.0f );
			auto_back->xyz.z = -( (pm->submodel[pm->detail[detail_level]].max.xyz.z + pm->submodel[pm->detail[detail_level]].min.xyz.z) / 2.0f );
			return true;
		}
	}

	return false;
}

bool model_render_check_detail_box(vec3d *view_pos, polymodel *pm, int submodel_num, uint flags)
{
	Assert(pm != NULL);

	bsp_info *model = &pm->submodel[submodel_num];

	float box_scale = model_render_determine_box_scale();

	if ( !( flags & MR_FULL_DETAIL ) && model->use_render_box ) {
		vec3d box_min, box_max;

		vm_vec_copy_scale(&box_min, &model->render_box_min, box_scale);
		vm_vec_copy_scale(&box_max, &model->render_box_max, box_scale);

		if ( (-model->use_render_box + in_box(&box_min, &box_max, &model->offset, view_pos)) ) {
			return false;
		}

		return true;
	}

	if ( !(flags & MR_FULL_DETAIL) && model->use_render_sphere ) {
		float sphere_radius = model->render_sphere_radius * box_scale;

		// TODO: doesn't consider submodel rotations yet -zookeeper
		vec3d offset;
		model_find_submodel_offset(&offset, pm->id, submodel_num);
		vm_vec_add2(&offset, &model->render_sphere_offset);

		if ( (-model->use_render_sphere + in_sphere(&offset, sphere_radius, view_pos)) ) {
			return false;
		}

		return true;
	}

	return true;
}

void submodel_render_immediate(model_render_params *render_info, int model_num, int submodel_num, matrix *orient, vec3d * pos)
{
	draw_list model_list;
	
	model_list.init();

	submodel_render_queue(render_info, &model_list, model_num, submodel_num, orient, pos);
	
	model_list.render_all();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();
	gr_set_buffer(-1);

	gr_reset_lighting();
	gr_set_lighting(false, false);
}

void submodel_render_queue(model_render_params *render_info, draw_list *scene, int model_num, int submodel_num, matrix *orient, vec3d * pos)
{
	polymodel * pm;

	//MONITOR_INC( NumModelsRend, 1 );	

	if ( !( Game_detail_flags & DETAIL_FLAG_MODELS ) )	return;
	
	if ( render_info->is_clip_plane_set() ) {
		scene->set_clip_plane(render_info->get_clip_plane_pos(), render_info->get_clip_plane_normal());
	} else {
		scene->set_clip_plane();
	}

	if ( render_info->is_team_color_set() ) {
		scene->set_team_color(render_info->get_team_color());
	} else {
		scene->set_team_color();
	}
		
	uint flags = render_info->get_model_flags();
	int objnum = render_info->get_object_number();

	pm = model_get(model_num);

	// Set the flags we will pass to the tmapper
	uint tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode
	if( ( The_mission.flags & MISSION_FLAG_FULLNEB ) && ( Neb2_render_mode != NEB2_RENDER_NONE ) ) {
		tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}

	if ( !( flags & MR_NO_TEXTURING ) )	{
		tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( ( pm->flags & PM_FLAG_ALLOW_TILING ) && tiling )
			tmap_flags |= TMAP_FLAG_TILED;

		if ( !( flags & MR_NO_CORRECT ) )	{
			tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	if ( render_info->get_animated_effect_num() >= 0 ) {
		tmap_flags |= TMAP_ANIMATED_SHADER;
		scene->set_animated_effect(render_info->get_animated_effect_num());
		scene->set_animated_timer(render_info->get_animated_effect_timer());
	}

	bool is_outlines_only_htl = !Cmdline_nohtl && (flags & MR_NO_POLYS) && (flags & MR_SHOW_OUTLINE_HTL);

	//set to true since D3d and OGL need the api matrices set
	scene->push_transform(pos, orient);

	
	vec3d auto_back = ZERO_VECTOR;
	bool set_autocen = model_render_determine_autocenter(&auto_back, pm, render_info->get_detail_level_lock(), flags);

	if ( set_autocen ) {
		scene->push_transform(&auto_back, NULL);
	}

	if (is_outlines_only_htl) {
		scene->set_fill_mode(GR_FILL_MODE_WIRE);

		color outline_color = render_info->get_outline_color();
		gr_set_color_fast( &outline_color );

		tmap_flags &= ~TMAP_FLAG_RGB;
	} else {
		scene->set_fill_mode(GR_FILL_MODE_SOLID);
	}

	scene->set_light_factor(1.0f);

	if ( !( flags & MR_NO_LIGHTING ) ) {
		scene->set_light_filter(-1, pos, pm->submodel[submodel_num].rad);
		scene->set_lighting(true);
	} else {
		scene->set_lighting(false);
	}

	// fixes disappearing HUD in OGL - taylor
	scene->set_cull_mode(1);

	// RT - Put this here to fog debris
	if( tmap_flags & TMAP_FLAG_PIXEL_FOG ) {
		float fog_near, fog_far;
		object *obj = NULL;

		if (objnum >= 0)
			obj = &Objects[objnum];

		neb2_get_adjusted_fog_values(&fog_near, &fog_far, obj);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		scene->set_fog(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
	} else {
		scene->set_fog(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if(Rendering_to_shadow_map) {
		scene->set_zbias(-1024);
	} else {
		scene->set_zbias(0);
	}

	scene->set_buffer(pm->vertex_buffer_id);

	vec3d view_pos = scene->get_view_position();

	if ( model_render_check_detail_box(&view_pos, pm, submodel_num, flags) ) {
		model_render_buffers(scene, render_info, &pm->submodel[submodel_num].buffer, pm, submodel_num, 0, tmap_flags);

		if ( pm->flags & PM_FLAG_TRANS_BUFFER && pm->submodel[submodel_num].trans_buffer.flags & VB_FLAG_TRANS ) {
			model_render_buffers(scene, render_info, &pm->submodel[submodel_num].trans_buffer, pm, submodel_num, 0, tmap_flags);
		}
	}
	
	if ( pm->submodel[submodel_num].num_arcs )	{
		model_render_add_lightning( scene, render_info, pm, &pm->submodel[submodel_num] );
	}

	if ( set_autocen ) {
		scene->pop_transform();
	}

	scene->pop_transform();
}

void model_render_glowpoint(int point_num, vec3d *pos, matrix *orient, glow_point_bank *bank, glow_point_bank_override *gpo, polymodel *pm, ship* shipp, bool use_depth_buffer)
{
	glow_point *gpt = &bank->points[point_num];
	vec3d loc_offset = gpt->pnt;
	vec3d loc_norm = gpt->norm;
	vec3d world_pnt;
	vec3d world_norm;
	vec3d tempv;
	vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
	bool submodel_rotation = false;

	if ( bank->submodel_parent > 0 && pm->submodel[bank->submodel_parent].can_move && (gameseq_get_state_idx(GS_STATE_LAB) == -1) && shipp != NULL ) {
		model_find_submodel_offset(&submodel_static_offset, Ship_info[shipp->ship_info_index].model_num, bank->submodel_parent);

		submodel_rotation = true;
	}

	if ( submodel_rotation ) {
		vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

		tempv = loc_offset;
		find_submodel_instance_point_normal(&loc_offset, &loc_norm, &Objects[shipp->objnum], bank->submodel_parent, &tempv, &loc_norm);
	}

	vm_vec_unrotate(&world_pnt, &loc_offset, orient);
	vm_vec_add2(&world_pnt, pos);

	vm_vec_unrotate(&world_norm, &loc_norm, orient);

	if ( shipp != NULL ) {
		if ( (shipp->flags & (SF_ARRIVING) ) && (shipp->warpin_effect) && Ship_info[shipp->ship_info_index].warpin_type != WT_HYPERSPACE) {
			vec3d warp_pnt, tmp;
			matrix warp_orient;

			shipp->warpin_effect->getWarpPosition(&warp_pnt);
			shipp->warpin_effect->getWarpOrientation(&warp_orient);
			vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

			if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) < 0.0f ) {
				return;
			}
		}

		if ( (shipp->flags & (SF_DEPART_WARP) ) && (shipp->warpout_effect) && Ship_info[shipp->ship_info_index].warpout_type != WT_HYPERSPACE) {
			vec3d warp_pnt, tmp;
			matrix warp_orient;

			shipp->warpout_effect->getWarpPosition(&warp_pnt);
			shipp->warpout_effect->getWarpOrientation(&warp_orient);
			vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

			if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) > 0.0f ) {
				return;
			}
		}
	}

	switch ((gpo && gpo->type_override)?gpo->type:bank->type)
	{
	case 0:
		{
			float d,pulse = 1.0f;

			if ( IS_VEC_NULL(&world_norm) ) {
				d = 1.0f;	//if given a nul vector then always show it
			} else {
				vm_vec_sub(&tempv,&View_position,&world_pnt);
				vm_vec_normalize(&tempv);

				d = vm_vec_dot(&tempv,&world_norm);
				d -= 0.25;	
			}

			float w = gpt->radius;
			if (d > 0.0f) {
				vertex p;

				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;


				// fade them in the nebula as well
				if ( The_mission.flags & MISSION_FLAG_FULLNEB ) {
					//vec3d npnt;
					//vm_vec_add(&npnt, &loc_offset, pos);

					d *= (1.0f - neb2_get_fog_intensity(&world_pnt));
					w *= 1.5;	//make it bigger in a nebula
				}
				
				if (!Cmdline_nohtl) {
					g3_transfer_vertex(&p, &world_pnt);
				} else {
					g3_rotate_vertex(&p, &world_pnt);
				}

				p.r = p.g = p.b = p.a = (ubyte)(255.0f * MAX(d,0.0f));

				if((gpo && gpo->glow_bitmap_override)?(gpo->glow_bitmap > -1):(bank->glow_bitmap > -1)) {
					int gpflags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT;

					if (use_depth_buffer)
						gpflags |= TMAP_FLAG_SOFT_QUAD;

					batch_add_bitmap(
						(gpo && gpo->glow_bitmap_override)?gpo->glow_bitmap:bank->glow_bitmap,
						gpflags,  
						&p,
						0,
						(w * 0.5f),
						d * pulse,
						w
						);
				}
			} //d>0.0f
			if ( gpo && gpo->pulse_type ) {
				int period;

				if(gpo->pulse_period_override) {
					period = gpo->pulse_period;
				} else {
					if(gpo->on_time_override) {
						period = 2 * gpo->on_time;
					} else {
						period = 2 * bank->on_time;
					}
				}

				int x = 0;

				if ( (gpo && gpo->off_time_override) ? gpo->off_time : bank->off_time ) {
					x = (timestamp() - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % ( ((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) ) - ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time);
				} else {
					x = (timestamp() - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % gpo->pulse_period;
				}

				switch ( gpo->pulse_type ) {

				case PULSE_SIN:
					pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(sin( PI2 / period * x),gpo->pulse_exponent);
					break;

				case PULSE_COS:
					pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(cos( PI2 / period * x),gpo->pulse_exponent);
					break;

				case PULSE_SHIFTTRI:
					x += period / 4;
					if((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) {
						x %= ( ((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) );
					} else {
						x %= gpo->pulse_period;
					}

				case PULSE_TRI:
					float inv;
					if( x > period / 2) {
						inv = -1;
					} else {
						inv = 1;
					}
					if( x > period / 4) {
						pulse = gpo->pulse_bias + gpo->pulse_amplitude * inv * pow( 1.0f - ((x - period / 4.0f) * 4 / period) ,gpo->pulse_exponent);
					} else {
						pulse = gpo->pulse_bias + gpo->pulse_amplitude * inv * pow( (x * 4.0f / period) ,gpo->pulse_exponent);
					}
					break;
				}
			}

			if ( Deferred_lighting && gpo && gpo->is_lightsource ) {
				if ( gpo->lightcone ) {
					vec3d cone_dir_rot;
					vec3d cone_dir_model;
					vec3d cone_dir_world;
					vec3d cone_dir_screen;
					vec3d unused;

					if ( gpo->rotating ) {
						vm_rot_point_around_line(&cone_dir_rot, &gpo->cone_direction, PI * timestamp() * 0.000033333f * gpo->rotation_speed, &vmd_zero_vector, &gpo->rotation_axis);
					} else {
						cone_dir_rot = gpo->cone_direction; 
					}

					find_submodel_instance_point_normal(&unused, &cone_dir_model, &Objects[shipp->objnum], bank->submodel_parent, &unused, &cone_dir_rot);
					vm_vec_unrotate(&cone_dir_world, &cone_dir_model, orient);
					vm_vec_rotate(&cone_dir_screen, &cone_dir_world, &Eye_matrix);
					cone_dir_screen.xyz.z = -cone_dir_screen.xyz.z;
					light_add_cone(&world_pnt, &cone_dir_screen, gpo->cone_angle, gpo->cone_inner_angle, gpo->dualcone, 1.0f, w * gpo->radius_multi, 1, pulse * gpo->light_color.xyz.x + (1.0f-pulse) * gpo->light_mix_color.xyz.x, pulse * gpo->light_color.xyz.y + (1.0f-pulse) * gpo->light_mix_color.xyz.y, pulse * gpo->light_color.xyz.z + (1.0f-pulse) * gpo->light_mix_color.xyz.z, -1);
				} else {
					light_add_point(&world_pnt, 1.0f, w * gpo->radius_multi, 1, pulse * gpo->light_color.xyz.x + (1.0f-pulse) * gpo->light_mix_color.xyz.x, pulse * gpo->light_color.xyz.y + (1.0f-pulse) * gpo->light_mix_color.xyz.y, pulse * gpo->light_color.xyz.z + (1.0f-pulse) * gpo->light_mix_color.xyz.z, -1);
				}
			}
			break;
		}

	case 1:
		{
			vertex verts[4];
			vec3d fvec, top1, bottom1, top2, bottom2, start, end;

			vm_vec_add2(&loc_norm, &loc_offset);

			vm_vec_rotate(&start, &loc_offset, orient);
			vm_vec_rotate(&end, &loc_norm, orient);
			vm_vec_sub(&fvec, &end, &start);

			vm_vec_normalize(&fvec);

			moldel_calc_facing_pts(&top1, &bottom1, &fvec, &loc_offset, gpt->radius, 1.0f, &View_position);
			moldel_calc_facing_pts(&top2, &bottom2, &fvec, &loc_norm, gpt->radius, 1.0f, &View_position);

			int idx = 0;

			if ( Cmdline_nohtl ) {
				g3_rotate_vertex(&verts[0], &bottom1);
				g3_rotate_vertex(&verts[1], &bottom2);
				g3_rotate_vertex(&verts[2], &top2);
				g3_rotate_vertex(&verts[3], &top1);

				for ( idx = 0; idx < 4; idx++ ) {
					g3_project_vertex(&verts[idx]);
				}
			} else {
				g3_transfer_vertex(&verts[0], &bottom1);
				g3_transfer_vertex(&verts[1], &bottom2);
				g3_transfer_vertex(&verts[2], &top2);
				g3_transfer_vertex(&verts[3], &top1);
			}

			verts[0].texture_position.u = 0.0f;
			verts[0].texture_position.v = 0.0f;

			verts[1].texture_position.u = 1.0f;
			verts[1].texture_position.v = 0.0f;

			verts[2].texture_position.u = 1.0f;
			verts[2].texture_position.v = 1.0f;

			verts[3].texture_position.u = 0.0f;
			verts[3].texture_position.v = 1.0f;

			vm_vec_sub(&tempv,&View_position,&loc_offset);
			vm_vec_normalize(&tempv);

			if ( The_mission.flags & MISSION_FLAG_FULLNEB ) {
				batch_add_quad(bank->glow_neb_bitmap, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT, verts);
			} else {
				batch_add_quad(bank->glow_bitmap, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT, verts);
			}

			break;
		}
	}
}

void model_render_set_glow_points(polymodel *pm, int objnum)
{
	int time = timestamp();
	glow_point_bank_override *gpo = NULL;
	bool override_all = false;
	SCP_hash_map<int, void*>::iterator gpoi;
	ship_info *sip = NULL;
	ship *shipp = NULL;

	if ( Glowpoint_override ) {
		return;
	}

	if ( objnum > -1 ) {
		object *objp = &Objects[objnum];

		if ( objp != NULL && objp->type == OBJ_SHIP ) {
			shipp = &Ships[Objects[objnum].instance];
			sip = &Ship_info[shipp->ship_info_index];
			SCP_hash_map<int, void*>::iterator gpoi = sip->glowpoint_bank_override_map.find(-1);

			if (gpoi != sip->glowpoint_bank_override_map.end()) {
				override_all = true;
				gpo = (glow_point_bank_override*)sip->glowpoint_bank_override_map[-1];
			}
		}
	}

	for ( int i = 0; i < pm->n_glow_point_banks; i++ ) { //glow point blink code -Bobboau
		glow_point_bank *bank = &pm->glow_point_banks[i];

		if ( !override_all && sip ) {
			gpoi = sip->glowpoint_bank_override_map.find(i);

			if ( gpoi != sip->glowpoint_bank_override_map.end() ) {
				gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[i];
			} else {
				gpo = NULL;
			}
		}

		if ( bank->glow_timestamp == 0 ) {
			bank->glow_timestamp=time;
		}

		if ( ( gpo && gpo->off_time_override ) ? gpo->off_time : bank->off_time ) {
			if ( bank->is_on ) {
				if( ((gpo && gpo->on_time_override) ? gpo->on_time : bank->on_time) > ((time - ((gpo && gpo->disp_time_override) ? gpo->disp_time : bank->disp_time)) % (((gpo && gpo->on_time_override) ? gpo->on_time : bank->on_time) + ((gpo && gpo->off_time_override) ? gpo->off_time : bank->off_time))) ){
					bank->glow_timestamp = time;
					bank->is_on = 0;
				}
			} else {
				if( ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) < ((time - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % (((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time))) ){
					bank->glow_timestamp = time;
					bank->is_on = 1;
				}
			}
		}
	}
}

void model_render_glow_points(polymodel *pm, ship *shipp, matrix *orient, vec3d *pos, bool use_depth_buffer = true)
{
	if ( Rendering_to_shadow_map ) {
		return;
	}

	int i, j;

	int cull = gr_set_cull(0);

	glow_point_bank_override *gpo = NULL;
	bool override_all = false;
	SCP_hash_map<int, void*>::iterator gpoi;
	ship_info *sip = NULL;

	if ( shipp ) {
		sip = &Ship_info[shipp->ship_info_index];
		SCP_hash_map<int, void*>::iterator gpoi = sip->glowpoint_bank_override_map.find(-1);

		if(gpoi != sip->glowpoint_bank_override_map.end()) {
			override_all = true;
			gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[-1];
		}
	}

	for (i = 0; i < pm->n_glow_point_banks; i++ ) {
		glow_point_bank *bank = &pm->glow_point_banks[i];

		if(!override_all && sip) {
			gpoi = sip->glowpoint_bank_override_map.find(i);
			if(gpoi != sip->glowpoint_bank_override_map.end()) {
				gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[i];
			} else {
				gpo = NULL;
			}
		}

		//Only continue if there actually is a glowpoint bitmap available
		if (bank->glow_bitmap == -1)
			continue;

		if (pm->submodel[bank->submodel_parent].blown_off)
			continue;

		if ((gpo && gpo->off_time_override && !gpo->off_time)?gpo->is_on:bank->is_on) {
			if ( (shipp != NULL) && !(shipp->glow_point_bank_active[i]) )
				continue;

			for (j = 0; j < bank->num_points; j++) {
				Assert( bank->points != NULL );
				int flick;

				if (pm->submodel[pm->detail[0]].num_arcs) {
					flick = static_rand( timestamp() % 20 ) % (pm->submodel[pm->detail[0]].num_arcs + j); //the more damage, the more arcs, the more likely the lights will fail
				} else {
					flick = 1;
				}

				if (flick == 1) {
					model_render_glowpoint(j, pos, orient, bank, gpo, pm, shipp, use_depth_buffer);
				} // flick
			} // for slot
		} // bank is on
	} // for bank

	gr_set_cull(cull);
}

void model_queue_render_thrusters(model_render_params *interp, polymodel *pm, int objnum, ship *shipp, matrix *orient, vec3d *pos)
{
	int i, j;
	int n_q = 0;
	size_t 	k;
	vec3d norm, norm2, fvec, pnt, npnt;
	thruster_bank *bank = NULL;
	vertex p;
	bool do_render = false;

	if ( Rendering_to_shadow_map ) {
		return;
	}

	if ( pm == NULL ) {
		Int3();
		return;
	}

	if ( !(interp->get_model_flags() & MR_SHOW_THRUSTERS) ) {
		return;
	}

	// get an initial count to figure out how man geo batchers we need allocated
	for (i = 0; i < pm->n_thrusters; i++ ) {
		bank = &pm->thrusters[i];
		n_q += bank->num_points;
	}

	if (n_q <= 0) {
		return;
	}

	const mst_info& thruster_info = interp->get_thruster_info();

	// primary_thruster_batcher
	if (thruster_info.primary_glow_bitmap >= 0) {
		do_render = true;
	}

	// secondary_thruster_batcher
	if (thruster_info.secondary_glow_bitmap >= 0) {
		do_render = true;
	}

	// tertiary_thruster_batcher
	if (thruster_info.tertiary_glow_bitmap >= 0) {
		do_render = true;
	}

	if (do_render == false) {
		return;
	}

	// this is used for the secondary thruster glows 
	// it only needs to be calculated once so I'm doing it here -Bobboau
	norm.xyz.z = -1.0f;
	norm.xyz.x = 1.0f;
	norm.xyz.y = -1.0f;
	norm.xyz.x *= thruster_info.rotvel.xyz.y/2;
	norm.xyz.y *= thruster_info.rotvel.xyz.x/2;
	vm_vec_normalize(&norm);

	for (i = 0; i < pm->n_thrusters; i++ ) {
		vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
		bool submodel_rotation = false;

		bank = &pm->thrusters[i];

		// don't draw this thruster if the engine is destroyed or just not on
		if ( !model_should_render_engine_glow(objnum, bank->obj_num) )
			continue;

		// If bank is attached to a submodel, prepare to account for rotations
		//
		// TODO: This won't work in the ship lab, because the lab code doesn't
		// set the the necessary submodel instance info needed here. The second
		// condition is thus a hack to disable the feature while in the lab, and
		// can be removed if the lab is re-structured accordingly. -zookeeper
		if ( bank->submodel_num > -1 && pm->submodel[bank->submodel_num].can_move && (gameseq_get_state_idx(GS_STATE_LAB) == -1) ) {
			model_find_submodel_offset(&submodel_static_offset, Ship_info[shipp->ship_info_index].model_num, bank->submodel_num);

			submodel_rotation = true;
		}

		for (j = 0; j < bank->num_points; j++) {
			Assert( bank->points != NULL );

			float d, D;
			vec3d tempv;
			glow_point *gpt = &bank->points[j];
			vec3d loc_offset = gpt->pnt;
			vec3d loc_norm = gpt->norm;
			vec3d world_pnt;
			vec3d world_norm;

			if ( submodel_rotation ) {
				vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

				tempv = loc_offset;
				find_submodel_instance_point_normal(&loc_offset, &loc_norm, &Objects[objnum], bank->submodel_num, &tempv, &loc_norm);
			}

			vm_vec_unrotate(&world_pnt, &loc_offset, orient);
			vm_vec_add2(&world_pnt, pos);

			if (shipp) {
				// if ship is warping out, check position of the engine glow to the warp plane
				if ( (shipp->flags & (SF_ARRIVING) ) && (shipp->warpin_effect) && Ship_info[shipp->ship_info_index].warpin_type != WT_HYPERSPACE) {
					vec3d warp_pnt, tmp;
					matrix warp_orient;

					shipp->warpin_effect->getWarpPosition(&warp_pnt);
					shipp->warpin_effect->getWarpOrientation(&warp_orient);
					vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

					if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) < 0.0f ) {
						break;
					}
				}

				if ( (shipp->flags & (SF_DEPART_WARP) ) && (shipp->warpout_effect) && Ship_info[shipp->ship_info_index].warpout_type != WT_HYPERSPACE) {
					vec3d warp_pnt, tmp;
					matrix warp_orient;

					shipp->warpout_effect->getWarpPosition(&warp_pnt);
					shipp->warpout_effect->getWarpOrientation(&warp_orient);
					vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

					if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) > 0.0f ) {
						break;
					}
				}
			}

			vm_vec_sub(&tempv, &View_position, &world_pnt);
			vm_vec_normalize(&tempv);
			vm_vec_unrotate(&world_norm, &loc_norm, orient);
			D = d = vm_vec_dot(&tempv, &world_norm);

			// ADAM: Min throttle draws rad*MIN_SCALE, max uses max.
#define NOISE_SCALE 0.5f
#define MIN_SCALE 3.4f
#define MAX_SCALE 4.7f

			float magnitude;
			vec3d scale_vec = { { { 1.0f, 0.0f, 0.0f } } };

			// normalize banks, in case of incredibly big normals
			if ( !IS_VEC_NULL_SQ_SAFE(&world_norm) )
				vm_vec_copy_normalize(&scale_vec, &world_norm);

			// adjust for thrust
			(scale_vec.xyz.x *= thruster_info.length.xyz.x) -= 0.1f;
			(scale_vec.xyz.y *= thruster_info.length.xyz.y) -= 0.1f;
			(scale_vec.xyz.z *= thruster_info.length.xyz.z)   -= 0.1f;

			// get magnitude, which we will use as the scaling reference
			magnitude = vm_vec_normalize(&scale_vec);

			// get absolute value
			if (magnitude < 0.0f)
				magnitude *= -1.0f;

			float scale = magnitude * (MAX_SCALE - MIN_SCALE) + MIN_SCALE;

			if (d > 0.0f){
				// Make glow bitmap fade in/out quicker from sides.
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;
			}

			float fog_int = 1.0f;

			// fade them in the nebula as well
			if (The_mission.flags & MISSION_FLAG_FULLNEB) {
				vm_vec_unrotate(&npnt, &gpt->pnt, orient);
				vm_vec_add2(&npnt, pos);

				fog_int = (1.0f - (neb2_get_fog_intensity(&npnt)));

				if (fog_int > 1.0f)
					fog_int = 1.0f;

				d *= fog_int;

				if (d > 1.0f)
					d = 1.0f;
			}

			float w = gpt->radius * (scale + thruster_info.glow_noise * NOISE_SCALE);

			// these lines are used by the tertiary glows, thus we will need to project this all of the time
			if (Cmdline_nohtl) {
				g3_rotate_vertex( &p, &world_pnt );
			} else {
				g3_transfer_vertex( &p, &world_pnt );
			}

			// start primary thruster glows
			if ( (thruster_info.primary_glow_bitmap >= 0) && (d > 0.0f) ) {
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * d);
				batch_add_bitmap(
					thruster_info.primary_glow_bitmap, 
					TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
					&p,
					0,
					(w * 0.5f * thruster_info.glow_rad_factor),
					1.0f,
					(w * 0.325f)
					);
			}

			// start tertiary thruster glows
			if (thruster_info.tertiary_glow_bitmap >= 0) {
				p.screen.xyw.w -= w;
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * fog_int);
				batch_add_bitmap_rotated(
					thruster_info.tertiary_glow_bitmap,
					TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD,
					&p,
					(magnitude * 4),
					(w * 0.6f * thruster_info.tertiary_glow_rad_factor),
					1.0f,
					(-(D > 0) ? D : -D)
					);
			}

			// begin secondary glows
			if (thruster_info.secondary_glow_bitmap >= 0) {
				pnt = world_pnt;
				scale = magnitude * (MAX_SCALE - (MIN_SCALE / 2)) + (MIN_SCALE / 2);
				vm_vec_unrotate(&world_norm, &norm, orient);
				d = vm_vec_dot(&tempv, &world_norm);
				d += 0.75f;
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;

				if (d > 0.0f) {
					vm_vec_add(&norm2, &world_norm, &pnt);
					vm_vec_sub(&fvec, &norm2, &pnt);
					vm_vec_normalize(&fvec);

					float wVal = gpt->radius * scale * 2;

					vm_vec_scale_add(&norm2, &pnt, &fvec, wVal * 2 * thruster_info.glow_length_factor);

					if (The_mission.flags & MISSION_FLAG_FULLNEB) {
						vm_vec_add(&npnt, &pnt, pos);
						d *= fog_int;
					}

					batch_add_beam(thruster_info.secondary_glow_bitmap,
						TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT,
						&pnt, &norm2, wVal*thruster_info.secondary_glow_rad_factor*0.5f, d
						);
					if (Scene_framebuffer_in_frame && thruster_info.draw_distortion) {
						vm_vec_scale_add(&norm2, &pnt, &fvec, wVal * 2 * thruster_info.distortion_length_factor);
						int dist_bitmap;
						if (thruster_info.distortion_bitmap > 0) {
							dist_bitmap = thruster_info.distortion_bitmap;
						}
						else {
							dist_bitmap = thruster_info.secondary_glow_bitmap;
						}
						float mag = vm_vec_mag(&gpt->pnt); 
						mag -= (float)((int)mag);//Valathil - Get a fairly random but constant number to offset the distortion texture
						distortion_add_beam(dist_bitmap,
							TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_DISTORTION_THRUSTER | TMAP_FLAG_SOFT_QUAD,
							&pnt, &norm2, wVal*thruster_info.distortion_rad_factor*0.5f, 1.0f, mag
							);
					}
				}
			}

			// begin particles
			if (shipp) {
				ship_info *sip = &Ship_info[shipp->ship_info_index];
				particle_emitter pe;
				thruster_particles *tp;
				size_t num_particles = 0;

				if (thruster_info.use_ab)
					num_particles = sip->afterburner_thruster_particles.size();
				else
					num_particles = sip->normal_thruster_particles.size();

				for (k = 0; k < num_particles; k++) {
					if (thruster_info.use_ab)
						tp = &sip->afterburner_thruster_particles[k];
					else
						tp = &sip->normal_thruster_particles[k];

					float v = vm_vec_mag_quick(&Objects[shipp->objnum].phys_info.desired_vel);

					vm_vec_unrotate(&npnt, &gpt->pnt, orient);
					vm_vec_add2(&npnt, pos);

					// Where the particles emit from
					pe.pos = npnt;
					// Initial velocity of all the particles
					pe.vel = Objects[shipp->objnum].phys_info.desired_vel;
					pe.min_vel = v * 0.75f;
					pe.max_vel =  v * 1.25f;
					// What normal the particle emit around
					pe.normal = orient->vec.fvec;
					vm_vec_negate(&pe.normal);

					// Lowest number of particles to create
					pe.num_low = tp->n_low;
					// Highest number of particles to create
					pe.num_high = tp->n_high;
					pe.min_rad = gpt->radius * tp->min_rad;
					pe.max_rad = gpt->radius * tp->max_rad;
					// How close they stick to that normal 0=on normal, 1=180, 2=360 degree
					pe.normal_variance = tp->variance;

					particle_emit( &pe, PARTICLE_BITMAP, tp->thruster_bitmap.first_frame);
				}
			}
		}
	}
}

void model_render_debug_children(polymodel *pm, int mn, int detail_level, uint flags)
{
	int i;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *model = &pm->submodel[mn];

	if ( model->blown_off ) {
		return;
	}

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	angles ang = model->angs;

	// Add barrel rotation if needed
	if ( model->gun_rotation ) {
		if ( pm->gun_submodel_rotation > PI2 ) {
			pm->gun_submodel_rotation -= PI2;
		} else if ( pm->gun_submodel_rotation < 0.0f ) {
			pm->gun_submodel_rotation += PI2;
		}

		ang.b += pm->gun_submodel_rotation;
	}

	// Compute final submodel orientation by using the orientation matrix
	// and the rotation angles.
	// By using this kind of computation, the rotational angles can always
	// be computed relative to the submodel itself, instead of relative
	// to the parent
	matrix rotation_matrix = model->orientation;
	vm_rotate_matrix_by_angles(&rotation_matrix, &ang);

	matrix inv_orientation;
	vm_copy_transpose_matrix(&inv_orientation, &model->orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	g3_start_instance_matrix(&model->offset, &submodel_matrix, true);

// 	if ( flags & MR_SHOW_PIVOTS ) {
// 		model_draw_debug_points( pm, &pm->submodel[mn], flags );
// 	}

	i = model->first_child;

	while ( i >= 0 ) {
		model_render_debug_children( pm, i, detail_level, flags );

		i = pm->submodel[i].next_sibling;
	}

	g3_done_instance(true);
}

void model_render_debug(int model_num, matrix *orient, vec3d * pos, uint flags, uint debug_flags, int objnum, int detail_level_locked )
{
	ship *shipp = NULL;
	object *objp = NULL;

	if ( objnum >= 0 ) {
		objp = &Objects[objnum];

		if ( objp->type == OBJ_SHIP ) {
			shipp = &Ships[objp->instance];
		}
	}

	polymodel *pm = model_get(model_num);	
	
	g3_start_instance_matrix(pos, orient, true);

	if ( debug_flags & MR_DEBUG_RADIUS ) {
		if ( !( flags & MR_SHOW_OUTLINE_PRESET ) ) {
			gr_set_color(0,64,0);
			g3_draw_sphere_ez(&vmd_zero_vector,pm->rad);
		}
	}
	float depth = model_render_determine_depth(objnum, model_num, orient, pos, detail_level_locked);
	int detail_level = model_render_determine_detail(depth, objnum, model_num, orient, pos, flags, detail_level_locked);

	vec3d auto_back = ZERO_VECTOR;
	bool set_autocen = model_render_determine_autocenter(&auto_back, pm, detail_level, flags);
	
	if ( set_autocen ) {
		g3_start_instance_matrix(&auto_back, NULL, true);
	}

	uint save_gr_zbuffering_mode = gr_zbuffer_set(GR_ZBUFF_READ);

	int i = pm->submodel[pm->detail[detail_level]].first_child;

	while ( i >= 0 ) {
		model_render_debug_children( pm, i, detail_level, flags );

		i = pm->submodel[i].next_sibling;
	}

	if ( debug_flags & MR_DEBUG_PIVOTS ) {
		model_draw_debug_points( pm, NULL, flags );
		model_draw_debug_points( pm, &pm->submodel[pm->detail[detail_level]], flags );

		if ( pm->flags & PM_FLAG_AUTOCEN ) {
			gr_set_color(255, 255, 255);
			g3_draw_sphere_ez(&pm->autocenter, pm->rad / 4.5f);
		}
	}

	if ( debug_flags & MR_DEBUG_SHIELDS )	{
		model_render_shields(pm, flags);
	}	

	if ( debug_flags & MR_DEBUG_PATHS ) {
		if ( Cmdline_nohtl ) model_draw_paths(model_num, flags);
		else model_draw_paths_htl(model_num, flags);
	}

	if ( debug_flags & MR_DEBUG_BAY_PATHS ) {
		if ( Cmdline_nohtl ) model_draw_bay_paths(model_num);
		else model_draw_bay_paths_htl(model_num);
	}

	if ( (flags & MR_AUTOCENTER) && (set_autocen) ) {
		g3_done_instance(true);
	}

	g3_done_instance(true);

	gr_zbuffer_set(save_gr_zbuffering_mode);
}

void model_render_immediate(model_render_params *render_info, int model_num, matrix *orient, vec3d * pos, int render)
{
	draw_list model_list;

	model_list.init();

	model_render_queue(render_info, &model_list, model_num, orient, pos);

	model_list.init_render();

	switch ( render ) {
	case MODEL_RENDER_OPAQUE:
		model_list.render_all(GR_ZBUFF_FULL);
		break;
	case MODEL_RENDER_TRANS:
		model_list.render_all(GR_ZBUFF_READ);
		model_list.render_all(GR_ZBUFF_NONE);
		break;
	case MODEL_RENDER_ALL:
		model_list.render_all();
		break;
	}

	model_list.render_outlines();
	model_list.render_insignias();
	model_list.render_arcs();
	
	gr_zbias(0);
	gr_set_cull(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();
	gr_set_buffer(-1);

	gr_reset_lighting();
	gr_set_lighting(false, false);

	GL_state.Texture.DisableAll();

	model_render_debug(model_num, orient, pos, render_info->get_model_flags(), render_info->get_debug_flags(), render_info->get_object_number(), render_info->get_detail_level_lock());
}

void model_render_queue(model_render_params *interp, draw_list *scene, int model_num, matrix *orient, vec3d *pos)
{
	int i;
	int cull = 0;

	const int objnum = interp->get_object_number();
	const int model_flags = interp->get_model_flags();

	polymodel *pm = model_get(model_num);
	polymodel_instance * pmi = NULL;
		
	model_do_dumb_rotation(model_num);

	float light_factor = model_render_determine_light_factor(interp, pos, model_flags);

	if ( light_factor < (1.0f/32.0f) ) {
		// If it is too far, exit
		return;
	}

	scene->set_light_factor(light_factor);

	if ( interp->is_clip_plane_set() ) {
		scene->set_clip_plane(interp->get_clip_plane_pos(), interp->get_clip_plane_normal());
	} else {
		scene->set_clip_plane();
	}

	if ( interp->is_team_color_set() ) {
		scene->set_team_color(interp->get_team_color());
	} else {
		scene->set_team_color();
	}
		
	if ( model_flags & MR_FORCE_CLAMP ) {
		scene->set_texture_addressing(TMAP_ADDRESS_CLAMP);
	} else {
		scene->set_texture_addressing(TMAP_ADDRESS_WRAP);
	}

	model_render_set_glow_points(pm, objnum);

	if ( !(model_flags & MR_NO_LIGHTING) ) {
		scene->set_light_filter( objnum, pos, pm->rad );
	}

	ship *shipp = NULL;
	object *objp = NULL;

	if (objnum >= 0) {
		objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];
		}
	}
	
	int tmp_detail_level = Game_detail_level;
	
	// Set the flags we will pass to the tmapper
	uint tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode, fog everything except for the warp holes and other non-fogged models
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !(model_flags & MR_NO_FOGGING)){
		tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}
	
	if ( !(model_flags & MR_NO_TEXTURING) )	{
		tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			tmap_flags |= TMAP_FLAG_TILED;

		if ( !(model_flags & MR_NO_CORRECT) )	{
			tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	if ( model_flags & MR_DESATURATED ) {
		tmap_flags |= TMAP_FLAG_DESATURATE;
	}

	if ( interp->get_animated_effect_num() >= 0 ) {
		tmap_flags |= TMAP_ANIMATED_SHADER;
		scene->set_animated_effect(interp->get_animated_effect_num());
		scene->set_animated_timer(interp->get_animated_effect_timer());
	}

	bool is_outlines_only = (model_flags & MR_NO_POLYS) && ((model_flags & MR_SHOW_OUTLINE_PRESET) || (model_flags & MR_SHOW_OUTLINE));
	bool is_outlines_only_htl = !Cmdline_nohtl && (model_flags & MR_NO_POLYS) && (model_flags & MR_SHOW_OUTLINE_HTL);
	bool use_api = !is_outlines_only_htl || (gr_screen.mode == GR_OPENGL);

	scene->push_transform(pos, orient);

	float depth = model_render_determine_depth(objnum, model_num, orient, pos, interp->get_detail_level_lock());
	int detail_level = model_render_determine_detail(depth, objnum, model_num, orient, pos, model_flags, interp->get_detail_level_lock());

	// If we're rendering attached weapon models, check against the ships' tabled Weapon Model Draw Distance (which defaults to 200)
	if ( model_flags & MR_ATTACHED_MODEL && shipp != NULL ) {
		if (depth > Ship_info[shipp->ship_info_index].weapon_model_draw_distance) {
			scene->pop_transform();
			return;
		}
	}

// #ifndef NDEBUG
// 	if ( Interp_detail_level == 0 )	{
// 		MONITOR_INC( NumHiModelsRend, 1 );
// 	} else if ( Interp_detail_level == pm->n_detail_levels-1 ) {
// 		MONITOR_INC( NumLowModelsRend, 1 );
// 	}  else {
// 		MONITOR_INC( NumMedModelsRend, 1 );
// 	}
// #endif
	
	vec3d auto_back = ZERO_VECTOR;
	bool set_autocen = model_render_determine_autocenter(&auto_back, pm, detail_level, model_flags);
	
	if ( set_autocen ) {
		scene->push_transform(&auto_back, NULL);
	}

	if ( tmap_flags & TMAP_FLAG_PIXEL_FOG ) {
		float fog_near = 10.0f, fog_far = 1000.0f;
		neb2_get_adjusted_fog_values(&fog_near, &fog_far, objp);

		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		scene->set_fog(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
	} else {
		scene->set_fog(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if ( is_outlines_only_htl ) {
		scene->set_fill_mode(GR_FILL_MODE_WIRE);

		color outline_color = interp->get_outline_color();
		scene->set_color(outline_color);

		tmap_flags &= ~TMAP_FLAG_RGB;
	} else {
		scene->set_fill_mode(GR_FILL_MODE_SOLID);
	}
		
	if ( model_flags & MR_EDGE_ALPHA ) {
		scene->set_center_alpha(-1);
	} else if ( model_flags & MR_CENTER_ALPHA ) {
		scene->set_center_alpha(1);
	} else {
		scene->set_center_alpha(0);
	}

	if ( ( model_flags & MR_NO_CULL ) || ( model_flags & MR_ALL_XPARENT ) || ( interp->get_warp_bitmap() >= 0 ) ) {
		scene->set_cull_mode(0);
	} else {
		scene->set_cull_mode(1);
	}

	if ( !(model_flags & MR_NO_LIGHTING) ) {
		scene->set_lighting(true);
	} else {
		scene->set_lighting(false);
	}
	
	if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
		scene->set_buffer(pm->vertex_buffer_id);
	}

	if ( Rendering_to_shadow_map ) {
		scene->set_zbias(-1024);
	} else {
		scene->set_zbias(0);
	}

	if ( !Cmdline_no_batching && !(model_flags & MR_NO_BATCH) && pm->flags & PM_FLAG_BATCHED 
		&& !(is_outlines_only || is_outlines_only_htl) ) {
		// always set batched rendering on if supported
		tmap_flags |= TMAP_FLAG_BATCH_TRANSFORMS;
	}

	if ( (tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS) ) {
		scene->start_model_batch(pm->n_models);
		model_render_buffers(scene, interp, &pm->detail_buffers[detail_level], pm, -1, detail_level, tmap_flags);
	}
		
	// Draw the subobjects
	bool draw_thrusters = false;
	bool trans_buffer = false;
	i = pm->submodel[pm->detail[detail_level]].first_child;

	while( i >= 0 )	{
		if ( !pm->submodel[i].is_thruster ) {
			model_render_children_buffers( scene, interp, pm, i, detail_level, tmap_flags, trans_buffer );
		} else {
			draw_thrusters = true;
		}

		i = pm->submodel[i].next_sibling;
	}

	model_radius = pm->submodel[pm->detail[detail_level]].rad;

	//*************************** draw the hull of the ship *********************************************
	vec3d view_pos = scene->get_view_position();

	if ( model_render_check_detail_box(&view_pos, pm, pm->detail[detail_level], model_flags) ) {
		int detail_model_num = pm->detail[detail_level];

		if ( (is_outlines_only || is_outlines_only_htl) && pm->submodel[detail_model_num].outline_buffer != NULL ) {
			color outline_color = interp->get_outline_color();
			scene->add_outline(pm->submodel[detail_model_num].outline_buffer, pm->submodel[detail_model_num].n_verts_outline, &outline_color);
		} else {
			model_render_buffers(scene, interp, &pm->submodel[detail_model_num].buffer, pm, detail_model_num, detail_level, tmap_flags);

			if ( pm->submodel[detail_model_num].num_arcs ) {
				model_render_add_lightning( scene, interp, pm, &pm->submodel[detail_model_num] );
			}
		}
	}
	
	// make sure batch rendering is unconditionally off.
	tmap_flags &= ~TMAP_FLAG_BATCH_TRANSFORMS;

	if ( pm->flags & PM_FLAG_TRANS_BUFFER && !(is_outlines_only || is_outlines_only_htl) ) {
		trans_buffer = true;
		i = pm->submodel[pm->detail[detail_level]].first_child;

		while( i >= 0 )	{
			if ( !pm->submodel[i].is_thruster ) {
				model_render_children_buffers( scene, interp, pm, i, detail_level, tmap_flags, trans_buffer );
			}

			i = pm->submodel[i].next_sibling;
		}

		vec3d view_pos = scene->get_view_position();

		if ( model_render_check_detail_box(&view_pos, pm, pm->detail[detail_level], model_flags) ) {
			int detail_model_num = pm->detail[detail_level];
			model_render_buffers(scene, interp, &pm->submodel[detail_model_num].trans_buffer, pm, detail_model_num, detail_level, tmap_flags);
		}
	}

	// Draw the thruster subobjects
	if ( draw_thrusters && !(is_outlines_only || is_outlines_only_htl) ) {
		i = pm->submodel[pm->detail[detail_level]].first_child;
		trans_buffer = false;

		while( i >= 0 ) {
			if (pm->submodel[i].is_thruster) {
				model_render_children_buffers( scene, interp, pm, i, detail_level, tmap_flags, trans_buffer );
			}
			i = pm->submodel[i].next_sibling;
		}
	}

	if ( !( model_flags & MR_NO_TEXTURING ) ) {
		scene->add_insignia(pm, detail_level, interp->get_insignia_bitmap());
	}

	if ( (model_flags & MR_AUTOCENTER) && (set_autocen) ) {
		scene->pop_transform();
	}

	scene->pop_transform();

	// start rendering glow points -Bobboau
	if ( (pm->n_glow_point_banks) && !is_outlines_only && !is_outlines_only_htl && !Glowpoint_override ) {
		model_render_glow_points(pm, shipp, orient, pos, Glowpoint_use_depth_buffer);
	}

	// Draw the thruster glow
	if ( !is_outlines_only && !is_outlines_only_htl ) {
		if ( ( model_flags & MR_AUTOCENTER ) && set_autocen ) {
			vec3d autoback_rotated;

			vm_vec_unrotate(&autoback_rotated, &auto_back, orient);
			vm_vec_add2(&autoback_rotated, pos);

			model_queue_render_thrusters( interp, pm, objnum, shipp, orient, &autoback_rotated );
		} else {
			model_queue_render_thrusters( interp, pm, objnum, shipp, orient, pos );
		}
	}
}
