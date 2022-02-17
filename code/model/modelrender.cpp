/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "model/modelrender.h"

#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "gamesequence/gamesequence.h"
#include "graphics/light.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "graphics/tmapper.h"
#include "graphics/uniforms.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "math/staticrand.h"
#include "mod_table/mod_table.h"
#include "nebula/neb.h"
#include "particle/particle.h"
#include "render/3dinternal.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "starfield/starfield.h"
#include "tracing/tracing.h"
#include "weapon/weapon.h"

#include <algorithm>

extern int Model_texturing;
extern int Model_polys;
extern int tiling;
extern float model_radius;

extern const int MAX_ARC_SEGMENT_POINTS;
extern int Num_arc_segment_points;
extern vec3d Arc_segment_points[];

extern bool Scene_framebuffer_in_frame;
color Wireframe_color;

extern void interp_render_arc_segment( vec3d *v1, vec3d *v2, int depth );

model_batch_buffer TransformBufferHandler;

model_render_params::model_render_params() :
	Model_flags(MR_NORMAL),
	Debug_flags(0),
	Objnum(-1),
	Detail_level_locked(-1),
	Depth_scale(1500.0f),
	Warp_bitmap(-1),
	Warp_alpha(-1.0f),
	Xparent_alpha(1.0f),
	Forced_bitmap(-1),
	Insignia_bitmap(-1),
	Replacement_textures(NULL),
	Manage_replacement_textures(false),
	Team_color_set(false),
	Clip_plane_set(false),
	Animated_effect(-1),
	Animated_timer(0.0f),
	Thruster_info(),
	Normal_alpha(false),
	Use_alpha_mult(false),
	Alpha_mult(1.0f)
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

	gr_init_color(&Color, 0, 0, 0);
}

model_render_params::~model_render_params() 
{
	if (Manage_replacement_textures)
		vm_free(Replacement_textures);
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

const color& model_render_params::get_color()
{ 
	return Color; 
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

void model_render_params::set_replacement_textures(int modelnum, SCP_vector<texture_replace>& replacement_textures)
{
	Replacement_textures = (int*)vm_malloc(MAX_REPLACEMENT_TEXTURES * sizeof(int));

	for (int i = 0; i < MAX_REPLACEMENT_TEXTURES; i++)
		Replacement_textures[i] = -1;

	Manage_replacement_textures = true;

	polymodel* pm = model_get(modelnum);

	for (auto tr : replacement_textures) 
	{
		for (int i = 0; i < pm->n_textures; ++i) 
		{
			texture_map *tmap = &pm->maps[i];

			int tnum = tmap->FindTexture(tr.old_texture);
			if (tnum > -1)
				Replacement_textures[i * TM_NUM_TYPES + tnum] = bm_load(tr.new_texture);
		}
	}
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

void model_render_params::set_color(color &clr)
{
	Color = clr;
}

void model_render_params::set_color(int r, int g, int b)
{
	gr_init_color( &Color, r, g, b );
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

void model_render_params::set_normal_alpha(float min, float max)
{
	Normal_alpha = true;
	Normal_alpha_min = min;
	Normal_alpha_max = max;
}

bool model_render_params::is_normal_alpha_set()
{
	return Normal_alpha;
}

float model_render_params::get_normal_alpha_min()
{
	return Normal_alpha_min;
}

float model_render_params::get_normal_alpha_max()
{
	return Normal_alpha_max;
}

void model_render_params::set_outline_thickness(float thickness) {
	Outline_thickness = thickness;
}
float model_render_params::get_outline_thickness() {
	return Outline_thickness;
}
bool model_render_params::uses_thick_outlines() {
	return Outline_thickness > 0.0f;
}

void model_render_params::set_alpha_mult(float alpha) {
	Alpha_mult = alpha;
	Use_alpha_mult = true;
}

bool model_render_params::is_alpha_mult_set()
{
	return Use_alpha_mult;
}

float model_render_params::get_alpha_mult()
{
	return Alpha_mult;
}

void model_batch_buffer::reset()
{
	Submodel_matrices.clear();

	Current_offset = 0;
}

void model_batch_buffer::set_num_models(int n_models)
{
	matrix4 init_mat;

	vm_matrix4_set_identity(&init_mat);

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

size_t model_batch_buffer::get_buffer_offset()
{
	return Current_offset;
}

void model_batch_buffer::allocate_memory()
{
	auto size = Submodel_matrices.size() * sizeof(matrix4);

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
	if ( Submodel_matrices.empty() ) {
		return;
	}

	allocate_memory();

	gr_update_transform_buffer(Mem_alloc, Mem_alloc_size);
}

model_draw_list::model_draw_list():
Transformations()
{
	reset();
}

void model_draw_list::reset()
{
	Render_elements.clear();
	Render_keys.clear();

	Transformations.clear();

	Current_scale.xyz.x = 1.0f;
	Current_scale.xyz.y = 1.0f;
	Current_scale.xyz.z = 1.0f;

	Render_initialized = false;
}

void model_draw_list::sort_draws()
{
	std::sort(Render_keys.begin(), Render_keys.end(),
			  [this](const int a, const int b) { return model_draw_list::sort_draw_pair(this, a, b); });
}

void model_draw_list::start_model_batch(int n_models)
{
	TransformBufferHandler.set_num_models(n_models);
}

void model_draw_list::add_submodel_to_batch(int model_num)
{
	matrix4 transform;

	transform = Transformations.get_transform();

	// set scale
	vm_vec_scale(&transform.vec.rvec, Current_scale.xyz.x);
	vm_vec_scale(&transform.vec.uvec, Current_scale.xyz.y);
	vm_vec_scale(&transform.vec.fvec, Current_scale.xyz.z);

	// set visibility
	transform.a1d[15] = 0.0f;

	TransformBufferHandler.set_model_transform(transform, model_num);
}

void model_draw_list::add_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width)
{
	arc_effect new_arc;

	new_arc.transform = Transformations.get_transform();
	new_arc.v1 = *v1;
	new_arc.v2 = *v2;
	new_arc.primary = *primary;
	new_arc.secondary = *secondary;
	new_arc.width = arc_width;

	Arcs.push_back(new_arc);
}

void model_draw_list::set_light_filter(int objnum, vec3d *pos, float rad)
{
	Scene_light_handler.setLightFilter(objnum, pos, rad);

	Current_lights_set = Scene_light_handler.bufferLights();
}

void model_draw_list::add_buffer_draw(model_material *render_material, indexed_vertex_source *vert_src, vertex_buffer *buffer, size_t texi, uint tmap_flags)
{
	queued_buffer_draw draw_data;
	draw_data.render_material = *render_material;

	if (Rendering_to_shadow_map) {
		draw_data.render_material.set_shadow_casting(true);
	} else {
		// If the zbuffer type is FULL then this buffer may be drawn in the deferred lighting part otherwise we need to
		// make sure that the deferred flag is disabled or else some parts of the rendered colors go missing
		// TODO: This should really be handled somewhere else. This feels like a crude hack...
		auto possibly_deferred = draw_data.render_material.get_depth_mode() == ZBUFFER_TYPE_FULL
			&& gr_is_capable(CAPABILITY_DEFERRED_LIGHTING) && !Cmdline_no_deferred_lighting;

		if (possibly_deferred) {
			// Fog is handled differently in deferred shader situations
			draw_data.render_material.set_fog();
		}

		draw_data.render_material.set_deferred_lighting(possibly_deferred ? Deferred_lighting : false);
		draw_data.render_material.set_high_dynamic_range(High_dynamic_range);
		draw_data.render_material.set_shadow_receiving(Shadow_quality != ShadowQuality::Disabled);
	}

	if (tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS && buffer->flags & VB_FLAG_MODEL_ID) {
		vm_matrix4_set_identity(&draw_data.transform);

		draw_data.scale.xyz.x = 1.0f;
		draw_data.scale.xyz.y = 1.0f;
		draw_data.scale.xyz.z = 1.0f;

		draw_data.transform_buffer_offset = TransformBufferHandler.get_buffer_offset();

		draw_data.render_material.set_batching(true);
	} else {
		draw_data.transform = Transformations.get_transform();
		draw_data.scale = Current_scale;
		draw_data.transform_buffer_offset = INVALID_SIZE;
		draw_data.render_material.set_batching(false);
	}

	draw_data.sdr_flags = draw_data.render_material.get_shader_flags();

	draw_data.vert_src = vert_src;
	draw_data.buffer = buffer;
	draw_data.texi = texi;
	draw_data.flags = tmap_flags;
	draw_data.lights = Current_lights_set;

	Render_elements.push_back(draw_data);
	Render_keys.push_back((int) (Render_elements.size() - 1));
}

void model_draw_list::render_buffer(queued_buffer_draw &render_elements)
{
	GR_DEBUG_SCOPE("Render buffer");
	TRACE_SCOPE(tracing::RenderBuffer);

	gr_bind_uniform_buffer(uniform_block_type::ModelData, render_elements.uniform_buffer_offset,
	                       sizeof(graphics::model_uniform_data), _dataBuffer.bufferHandle());

	gr_render_model(&render_elements.render_material, render_elements.vert_src, render_elements.buffer, render_elements.texi);
}

vec3d model_draw_list::get_view_position()
{
	matrix basis_world;
	matrix4 transform_mat = Transformations.get_transform();
	matrix orient;
	vec3d pos;

	vm_matrix4_get_orientation(&orient, &transform_mat);
	vm_matrix4_get_offset(&pos, &transform_mat);

	// get the world basis of our current local space.
	vm_matrix_x_matrix(&basis_world, &Object_matrix, &orient);

	vec3d eye_pos_local;
	vm_vec_sub(&eye_pos_local, &Eye_position, &pos);

	vec3d return_val;
	vm_vec_rotate(&return_val, &eye_pos_local, &basis_world);

	return return_val;
}

void model_draw_list::push_transform(vec3d *pos, matrix *orient)
{
	Transformations.push(pos, orient);
}

void model_draw_list::pop_transform()
{
	Transformations.pop();
}

void model_draw_list::set_scale(vec3d *scale)
{
	if ( scale == NULL ) {
		Current_scale.xyz.x = 1.0f;
		Current_scale.xyz.y = 1.0f;
		Current_scale.xyz.z = 1.0f;
		return;
	}

	Current_scale = *scale;
}

void model_draw_list::init()
{
	reset();

	for (auto& l : Lights) {
		if ( l.type == Light_Type::Directional || !Deferred_lighting ) {
			Scene_light_handler.addLight(&l);
		}	
	}

	TransformBufferHandler.reset();
}

void model_draw_list::init_render(bool sort)
{
	if ( sort ) {
		sort_draws();
	}

	TransformBufferHandler.submit_buffer_data();

	build_uniform_buffer();

	Render_initialized = true;
}

void model_draw_list::render_all(gr_zbuffer_type depth_mode)
{
	GR_DEBUG_SCOPE("Render draw list");
	TRACE_SCOPE(tracing::SubmitDraws);

	Assertion(Render_initialized, "init_render must be called before any render_all call!");

	Scene_light_handler.resetLightState();

	for ( size_t i = 0; i < Render_keys.size(); ++i ) {
		int render_index = Render_keys[i];

		if ( depth_mode == ZBUFFER_TYPE_DEFAULT || Render_elements[render_index].render_material.get_depth_mode() == depth_mode ) {
			render_buffer(Render_elements[render_index]);
		}
	}

	gr_alpha_mask_set(0, 1.0f);
}

void model_draw_list::render_arc(arc_effect &arc)
{
	g3_start_instance_matrix(&arc.transform);	

	model_render_arc(&arc.v1, &arc.v2, &arc.primary, &arc.secondary, arc.width);

	g3_done_instance(true);
}

void model_draw_list::render_arcs()
{
	int mode = gr_zbuffer_set(GR_ZBUFF_READ);

	for ( size_t i = 0; i < Arcs.size(); ++i ) {
		render_arc(Arcs[i]);
	}

	gr_zbuffer_set(mode);
}

void model_draw_list::add_insignia(model_render_params *params, polymodel *pm, int detail_level, int bitmap_num)
{
	insignia_draw_data new_insignia;

	new_insignia.transform = Transformations.get_transform();
	new_insignia.pm = pm;
	new_insignia.detail_level = detail_level;
	new_insignia.bitmap_num = bitmap_num;

	new_insignia.clip = params->is_clip_plane_set();
	new_insignia.clip_normal = params->get_clip_plane_normal();
	new_insignia.clip_position = params->get_clip_plane_pos();

	Insignias.push_back(new_insignia);
}

void model_draw_list::render_insignia(insignia_draw_data &insignia_info)
{
	if ( insignia_info.clip ) {
		vec3d tmp;
		vec3d pos;

		vm_matrix4_get_offset(&pos, &insignia_info.transform);
		vm_vec_sub(&tmp, &pos, &insignia_info.clip_position);
		vm_vec_normalize(&tmp);

		if ( vm_vec_dot(&tmp, &insignia_info.clip_normal) < 0.0f) {
			return;
		}
	}

	g3_start_instance_matrix(&insignia_info.transform);	

	model_render_insignias(&insignia_info);

	g3_done_instance(true);
}

void model_draw_list::render_insignias()
{
	for ( size_t i = 0; i < Insignias.size(); ++i ) {
		render_insignia(Insignias[i]);
	}
}

void model_draw_list::add_outline(vertex* vert_array, int n_verts, color *clr)
{
	outline_draw draw_info;

	draw_info.vert_array = vert_array;
	draw_info.n_verts = n_verts;
	draw_info.clr = *clr;
	draw_info.transform = Transformations.get_transform();

	Outlines.push_back(draw_info);
}

void model_draw_list::render_outlines()
{
	gr_clear_states();

	for ( size_t i = 0; i < Outlines.size(); ++i ) {
		render_outline(Outlines[i]);
	}
}

void model_draw_list::render_outline(outline_draw &outline_info)
{
	g3_start_instance_matrix(&outline_info.transform);

	material material_instance;

	material_instance.set_depth_mode(ZBUFFER_TYPE_READ);
	material_instance.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_instance.set_color(outline_info.clr);

	g3_render_primitives(&material_instance, outline_info.vert_array, outline_info.n_verts, PRIM_TYPE_LINES, false);

	g3_done_instance(true);
}

bool model_draw_list::sort_draw_pair(model_draw_list* target, const int a, const int b)
{
	queued_buffer_draw *draw_call_a = &target->Render_elements[a];
	queued_buffer_draw *draw_call_b = &target->Render_elements[b];

	if ( draw_call_a->sdr_flags != draw_call_b->sdr_flags ) {
		return draw_call_a->sdr_flags < draw_call_b->sdr_flags;
	}

	if ( draw_call_a->vert_src->Vbuffer_handle != draw_call_b->vert_src->Vbuffer_handle ) {
		return draw_call_a->vert_src->Vbuffer_handle.value() < draw_call_b->vert_src->Vbuffer_handle.value();
	}

	if ( draw_call_a->vert_src->Ibuffer_handle != draw_call_b->vert_src->Ibuffer_handle ) {
		return draw_call_a->vert_src->Ibuffer_handle.value() < draw_call_b->vert_src->Ibuffer_handle.value();
	}

	if ( draw_call_a->render_material.get_texture_map(TM_BASE_TYPE) != draw_call_b->render_material.get_texture_map(TM_BASE_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_BASE_TYPE) < draw_call_b->render_material.get_texture_map(TM_BASE_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_SPECULAR_TYPE) != draw_call_b->render_material.get_texture_map(TM_SPECULAR_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_SPECULAR_TYPE) < draw_call_b->render_material.get_texture_map(TM_SPECULAR_TYPE);
	}
	
	if ( draw_call_a->render_material.get_texture_map(TM_SPEC_GLOSS_TYPE) != draw_call_b->render_material.get_texture_map(TM_SPEC_GLOSS_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_SPEC_GLOSS_TYPE) < draw_call_b->render_material.get_texture_map(TM_SPEC_GLOSS_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_GLOW_TYPE) != draw_call_b->render_material.get_texture_map(TM_GLOW_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_GLOW_TYPE) < draw_call_b->render_material.get_texture_map(TM_GLOW_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_NORMAL_TYPE) != draw_call_b->render_material.get_texture_map(TM_NORMAL_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_NORMAL_TYPE) < draw_call_b->render_material.get_texture_map(TM_NORMAL_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_HEIGHT_TYPE) != draw_call_b->render_material.get_texture_map(TM_HEIGHT_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_HEIGHT_TYPE) < draw_call_b->render_material.get_texture_map(TM_HEIGHT_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_AMBIENT_TYPE) != draw_call_b->render_material.get_texture_map(TM_AMBIENT_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_AMBIENT_TYPE) < draw_call_b->render_material.get_texture_map(TM_AMBIENT_TYPE);
	}

	if ( draw_call_a->render_material.get_texture_map(TM_MISC_TYPE) != draw_call_b->render_material.get_texture_map(TM_MISC_TYPE) ) {
		return draw_call_a->render_material.get_texture_map(TM_MISC_TYPE) < draw_call_b->render_material.get_texture_map(TM_MISC_TYPE);
	}

	return draw_call_a->lights.index_start < draw_call_b->lights.index_start;
}
void model_draw_list::build_uniform_buffer() {
	GR_DEBUG_SCOPE("Build model uniform buffer");

	TRACE_SCOPE(tracing::BuildModelUniforms);

	_dataBuffer = gr_get_uniform_buffer(uniform_block_type::ModelData, Render_keys.size());

	for (auto render_index : Render_keys) {
		auto& queued_draw = Render_elements[render_index];

		// Set lighting here so that it can be captured by the uniform conversion below
		if ( queued_draw.render_material.is_lit() ) {
			Scene_light_handler.setLights(&queued_draw.lights);
		} else {

			Scene_light_handler.resetLightState();
		}

		auto element = _dataBuffer.aligner().addTypedElement<graphics::model_uniform_data>();
		graphics::uniforms::convert_model_material(element,
												   queued_draw.render_material,
												   queued_draw.transform,
												   queued_draw.scale,
												   queued_draw.transform_buffer_offset);
		queued_draw.uniform_buffer_offset = _dataBuffer.getCurrentAlignerOffset();
	}

	TRACE_SCOPE(tracing::UploadModelUniforms);

	_dataBuffer.submitData();
}
model_draw_list::~model_draw_list() {
	reset();
}

void model_render_add_lightning( model_draw_list *scene, model_render_params* interp, polymodel *pm, submodel_instance *smi )
{
	int i;
	float width = 0.9f;
	color primary, secondary;

	Assert( smi->num_arcs > 0 );

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

	for ( i = 0; i < smi->num_arcs; i++ ) {
		// pick a color based upon arc type
		switch ( smi->arc_type[i] ) {
			// "normal", FreeSpace 1 style arcs
		case MARC_TYPE_DAMAGED:
		case MARC_TYPE_SHIP:
			if ( Random::flip_coin() )	{
				gr_init_color(&primary, std::get<0>(Arc_color_damage_p1), std::get<1>(Arc_color_damage_p1), std::get<2>(Arc_color_damage_p1));
			} else {
				gr_init_color(&primary, std::get<0>(Arc_color_damage_p2), std::get<1>(Arc_color_damage_p2), std::get<2>(Arc_color_damage_p2));
			}

			gr_init_color(&primary, std::get<0>(Arc_color_damage_s1), std::get<1>(Arc_color_damage_s1), std::get<2>(Arc_color_damage_s1));
			break;

			// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( Random::flip_coin() )	{
				gr_init_color(&primary, std::get<0>(Arc_color_emp_p1), std::get<1>(Arc_color_emp_p1), std::get<2>(Arc_color_emp_p1));
			} else {
				gr_init_color(&primary, std::get<0>(Arc_color_emp_p2), std::get<1>(Arc_color_emp_p2), std::get<2>(Arc_color_emp_p2));
			}

			gr_init_color(&primary, std::get<0>(Arc_color_emp_s1), std::get<1>(Arc_color_emp_s1), std::get<2>(Arc_color_emp_s1));
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		scene->add_arc(&smi->arc_pts[i][0], &smi->arc_pts[i][1], &primary, &secondary, width);
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
		if (The_mission.flags[Mission::Mission_Flags::Fullneb]) {
			depth *= neb2_get_lod_scale(obj_num);
		}

	}

	return depth;
}

int model_render_determine_detail(float depth, int  /*obj_num*/, int model_num, matrix*  /*orient*/, vec3d*  /*pos*/, int  /*flags*/, int detail_level_locked)
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

void model_render_buffers(model_draw_list* scene, model_material *rendering_material, model_render_params* interp, vertex_buffer *buffer, polymodel *pm, int mn, int detail_level, uint tmap_flags)
{
	bsp_info *submodel = nullptr;
	const uint model_flags = interp->get_model_flags();
	const uint debug_flags = interp->get_debug_flags();
	const int obj_num = interp->get_object_number();

	Assert(buffer != nullptr);
	Assert(detail_level >= 0);

	if ( (mn >= 0) && (mn < pm->n_models) ) {
		submodel = &pm->submodel[mn];
	}

	bool render_as_thruster = (submodel != nullptr) && submodel->flags[Model::Submodel_flags::Is_thruster] && (model_flags & MR_SHOW_THRUSTERS);

	vec3d scale;

	if ( render_as_thruster ) {
		scale.xyz.x = 1.0f;
		scale.xyz.y = 1.0f;

		scale.xyz.z = 1.0f;
		rendering_material->set_thrust_scale(interp->get_thruster_info().length.xyz.z);
	} else {
		scale = interp->get_warp_scale();
		rendering_material->set_thrust_scale();
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

	int texture_maps[TM_NUM_TYPES] = { -1 };
	size_t buffer_size = buffer->tex_buf.size();
	const int *replacement_textures = interp->get_replacement_textures();

	for ( size_t i = 0; i < buffer_size; i++ ) {
		int tmap_num = buffer->tex_buf[i].texture;
		texture_map *tmap = &pm->maps[tmap_num];
		int rt_begin_index = tmap_num*TM_NUM_TYPES;
		float alpha = 1.0f;

		texture_maps[TM_BASE_TYPE] = -1;
		texture_maps[TM_GLOW_TYPE] = -1;
		texture_maps[TM_SPECULAR_TYPE] = -1;
		texture_maps[TM_NORMAL_TYPE] = -1;
		texture_maps[TM_HEIGHT_TYPE] = -1;
		texture_maps[TM_MISC_TYPE] = -1;
		texture_maps[TM_SPEC_GLOSS_TYPE] = -1;
		texture_maps[TM_AMBIENT_TYPE] = -1;

		if (forced_texture != -2) {
			texture_maps[TM_BASE_TYPE] = forced_texture;
			alpha = forced_alpha;

			if (interp->get_warp_bitmap() >= 0) {
				texture_maps[TM_GLOW_TYPE] = forced_texture;
			}
			
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
						texture_maps[TM_GLOW_TYPE] = tglow->GetTexture() + shockwave_get_framenum(Objects[obj_num].instance, tglow->GetTexture());
					} else {
						texture_maps[TM_GLOW_TYPE] = model_interp_get_texture(tglow, base_frametime);
					}
				}
			}

			if (!(debug_flags & MR_DEBUG_NO_SPEC)) {
				if (replacement_textures != NULL && replacement_textures[rt_begin_index + TM_SPECULAR_TYPE] >= 0) {
					tex_replace[TM_SPECULAR_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_SPECULAR_TYPE]);
					texture_maps[TM_SPECULAR_TYPE] = model_interp_get_texture(&tex_replace[TM_SPECULAR_TYPE], base_frametime);
				}
				else {
					texture_maps[TM_SPECULAR_TYPE] = model_interp_get_texture(&tmap->textures[TM_SPECULAR_TYPE], base_frametime);
				}
			}

			if ( replacement_textures != NULL && replacement_textures[rt_begin_index + TM_SPEC_GLOSS_TYPE] >= 0 ) {
				tex_replace[TM_SPEC_GLOSS_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_SPEC_GLOSS_TYPE]);
				texture_maps[TM_SPEC_GLOSS_TYPE] = model_interp_get_texture(&tex_replace[TM_SPEC_GLOSS_TYPE], base_frametime);
			} else {
				texture_maps[TM_SPEC_GLOSS_TYPE] = model_interp_get_texture(&tmap->textures[TM_SPEC_GLOSS_TYPE], base_frametime);
			}

			if (detail_level < 2) {
				// likewise, etc.
				texture_info *norm_map = &tmap->textures[TM_NORMAL_TYPE];
				texture_info *height_map = &tmap->textures[TM_HEIGHT_TYPE];
				texture_info *ambient_map = &tmap->textures[TM_AMBIENT_TYPE];
				texture_info *misc_map = &tmap->textures[TM_MISC_TYPE];

				if (replacement_textures != NULL) {
					if (replacement_textures[rt_begin_index + TM_NORMAL_TYPE] >= 0) {
						tex_replace[TM_NORMAL_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_NORMAL_TYPE]);
						norm_map = &tex_replace[TM_NORMAL_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_HEIGHT_TYPE] >= 0) {
						tex_replace[TM_HEIGHT_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_HEIGHT_TYPE]);
						height_map = &tex_replace[TM_HEIGHT_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_AMBIENT_TYPE] >= 0) {
						tex_replace[TM_AMBIENT_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_AMBIENT_TYPE]);
						ambient_map = &tex_replace[TM_AMBIENT_TYPE];
					}

					if (replacement_textures[rt_begin_index + TM_MISC_TYPE] >= 0) {
						tex_replace[TM_MISC_TYPE] = texture_info(replacement_textures[rt_begin_index + TM_MISC_TYPE]);
						misc_map = &tex_replace[TM_MISC_TYPE];
					}
				}

				if (debug_flags & MR_DEBUG_NO_DIFFUSE)  texture_maps[TM_BASE_TYPE] = -1;
				if (debug_flags & MR_DEBUG_NO_GLOW)		  texture_maps[TM_GLOW_TYPE] = -1;
				if (debug_flags & MR_DEBUG_NO_SPEC)		  texture_maps[TM_SPECULAR_TYPE] = -1;
				if (debug_flags & MR_DEBUG_NO_REFLECT)	  texture_maps[TM_SPEC_GLOSS_TYPE] = -1;
				if (!(debug_flags & MR_DEBUG_NO_MISC))    texture_maps[TM_MISC_TYPE] = model_interp_get_texture(misc_map, base_frametime);
				if (!(debug_flags & MR_DEBUG_NO_NORMAL) && Detail.lighting > 0)  texture_maps[TM_NORMAL_TYPE] = model_interp_get_texture(norm_map, base_frametime);
				if (!(debug_flags & MR_DEBUG_NO_AMBIENT) && Detail.lighting > 0) texture_maps[TM_AMBIENT_TYPE] = model_interp_get_texture(ambient_map, base_frametime);
				if (!(debug_flags & MR_DEBUG_NO_HEIGHT) && Detail.lighting > 1)  texture_maps[TM_HEIGHT_TYPE] = model_interp_get_texture(height_map, base_frametime);
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

		if ( (texture_maps[TM_BASE_TYPE] == -1) && !no_texturing && !(debug_flags & MR_DEBUG_NO_DIFFUSE) ) {
			continue;
		}

		bool use_blending = false;

		// trying to get transparent textures-Bobboau
		if (tmap->is_transparent) {
			// for special shockwave/warp map usage
			alpha = (interp->get_warp_alpha() != -1.0f) ? interp->get_warp_alpha() : 0.8f;
			use_blending = true;
		} else if ( buffer->flags & VB_FLAG_TRANS ) {
			use_blending = true;
		}

		if (rendering_material->is_alpha_mult_active()) {
			use_blending = true;
		}

		if (forced_blend_filter != GR_ALPHABLEND_NONE) {
			use_blending = true;
		}

		bool use_depth_test;

		if ( use_blending ) {
			use_depth_test = true;
		} else {
			if ( (model_flags & MR_NO_ZBUFFER) || (model_flags & MR_ALL_XPARENT) ) {
				use_depth_test = false;
			} else {
				use_depth_test = true;
			}
		}

		gr_alpha_blend blend_mode = model_render_determine_blend_mode(texture_maps[TM_BASE_TYPE], use_blending);
		gr_zbuffer_type depth_mode = material_determine_depth_mode(use_depth_test, use_blending);

		if (rendering_material->is_alpha_mult_active()) {
			blend_mode = ALPHA_BLEND_PREMULTIPLIED;
		}

		rendering_material->set_depth_mode(depth_mode);
		rendering_material->set_blend_mode(blend_mode);
		
		color clr = interp->get_color();
		model_render_determine_color(&clr, alpha, blend_mode, no_texturing ? true : false, rendering_material->is_desaturated());
		rendering_material->set_color(clr);

		if ( (tmap_flags & TMAP_FLAG_TEXTURED) && (buffer->flags & VB_FLAG_UV1) ) {
			rendering_material->set_texture_map(TM_BASE_TYPE,	texture_maps[TM_BASE_TYPE]);

			if ( texture_maps[TM_BASE_TYPE] >= 0 && bm_has_alpha_channel(texture_maps[TM_BASE_TYPE]) ) {
				rendering_material->set_texture_type(material::TEX_TYPE_XPARENT);
			}

			rendering_material->set_texture_map(TM_GLOW_TYPE,	texture_maps[TM_GLOW_TYPE]);
			rendering_material->set_texture_map(TM_SPECULAR_TYPE, texture_maps[TM_SPECULAR_TYPE]);
			rendering_material->set_texture_map(TM_SPEC_GLOSS_TYPE, texture_maps[TM_SPEC_GLOSS_TYPE]);
			rendering_material->set_texture_map(TM_NORMAL_TYPE, texture_maps[TM_NORMAL_TYPE]);
			rendering_material->set_texture_map(TM_HEIGHT_TYPE, texture_maps[TM_HEIGHT_TYPE]);
			rendering_material->set_texture_map(TM_AMBIENT_TYPE, texture_maps[TM_AMBIENT_TYPE]);
			rendering_material->set_texture_map(TM_MISC_TYPE,	texture_maps[TM_MISC_TYPE]);
		}

		scene->add_buffer_draw(rendering_material, &pm->vert_source, buffer, i, tmap_flags);
	}
}

void model_render_children_buffers(model_draw_list* scene, model_material *rendering_material, model_render_params* interp, polymodel* pm, polymodel_instance *pmi, int mn, int detail_level, uint tmap_flags, bool trans_buffer)
{
	int i;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *sm = &pm->submodel[mn];
	submodel_instance *smi = nullptr;

	if ( pmi != nullptr ) {
		smi = &pmi->submodel[mn];
		if ( smi->blown_off ) {
			return;
		}
	}

	const uint model_flags = interp->get_model_flags();

	if (sm->flags[Model::Submodel_flags::Is_thruster]) {
		if ( !( model_flags & MR_SHOW_THRUSTERS ) ) {
			return;
		}

		rendering_material->set_lighting(false);
	}

	vec3d view_pos = scene->get_view_position();

	if ( !model_render_check_detail_box(&view_pos, pm, mn, model_flags) ) {
		return;
	}

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	matrix submodel_orient = vmd_identity_matrix;

	if ( smi != nullptr ) {
		submodel_orient = smi->canonical_orient;
	}

	scene->push_transform(&sm->offset, &submodel_orient);
	
	if ( (model_flags & MR_SHOW_OUTLINE || model_flags & MR_SHOW_OUTLINE_HTL || model_flags & MR_SHOW_OUTLINE_PRESET) && 
		sm->outline_buffer != nullptr ) {
		color outline_color = interp->get_color();
		scene->add_outline(sm->outline_buffer, sm->n_verts_outline, &outline_color);
	} else {
		if ( trans_buffer && sm->trans_buffer.flags & VB_FLAG_TRANS ) {
			model_render_buffers(scene, rendering_material, interp, &sm->trans_buffer, pm, mn, detail_level, tmap_flags);
		} else {
			model_render_buffers(scene, rendering_material, interp, &sm->buffer, pm, mn, detail_level, tmap_flags);
		} 
	}

	if ( smi != nullptr && smi->num_arcs > 0 ) {
		model_render_add_lightning( scene, interp, pm, smi );
	}

	i = sm->first_child;

	while ( i >= 0 ) {
		if ( !pm->submodel[i].flags[Model::Submodel_flags::Is_thruster] ) {
			model_render_children_buffers( scene, rendering_material, interp, pm, pmi, i, detail_level, tmap_flags, trans_buffer );
		}

		i = pm->submodel[i].next_sibling;
	}

	if ( sm->flags[Model::Submodel_flags::Is_thruster] ) {
		rendering_material->set_lighting(true);
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

void model_render_determine_color(color *clr, float alpha, gr_alpha_blend blend_mode, bool no_texturing, bool desaturate)
{
	clr->alpha = static_cast<ubyte>((alpha * 255.0f));

	if ( no_texturing || desaturate ) {
		// don't override the given color if we're not texturing or we're desaturating
		return;
	}

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		clr->red = clr->green = clr->blue = clr->alpha;
		clr->alpha = 255;
	} else {
		clr->red = clr->green = clr->blue = 255;
	}
}

gr_alpha_blend model_render_determine_blend_mode(int base_bitmap, bool blending)
{
	if ( blending ) {
		if ( base_bitmap >= 0 && bm_has_alpha_channel(base_bitmap) ) {
			return ALPHA_BLEND_PREMULTIPLIED;
		}

		return ALPHA_BLEND_ADDITIVE;
	}

	return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
}

bool model_render_check_detail_box(vec3d *view_pos, polymodel *pm, int submodel_num, uint flags)
{
	Assert(pm != NULL);

	bsp_info *model = &pm->submodel[submodel_num];

	float box_scale = model_render_determine_box_scale();
	if (model->flags[Model::Submodel_flags::Do_not_scale_detail_distances]) {
		box_scale = 1.0f;
	}

	if ( !( flags & MR_FULL_DETAIL ) && model->use_render_box ) {
		vec3d box_min, box_max, offset;

		if (model->flags[Model::Submodel_flags::Use_render_box_offset]) {
			offset = model->render_box_offset;
		} else {
			model_find_submodel_offset(&offset, pm, submodel_num);
		}

		vm_vec_copy_scale(&box_min, &model->render_box_min, box_scale);
		vm_vec_copy_scale(&box_max, &model->render_box_max, box_scale);

		if ( (-model->use_render_box + in_box(&box_min, &box_max, &offset, view_pos)) ) {
			return false;
		}
	}

	if ( !(flags & MR_FULL_DETAIL) && model->use_render_sphere ) {
		float sphere_radius = model->render_sphere_radius * box_scale;

		// TODO: doesn't consider submodel rotations yet -zookeeper
		vec3d offset;

		if (model->flags[Model::Submodel_flags::Use_render_sphere_offset]) {
			offset = model->render_sphere_offset;
		} else {
			model_find_submodel_offset(&offset, pm, submodel_num);
		}

		if ( (-model->use_render_sphere + in_sphere(&offset, sphere_radius, view_pos)) ) {
			return false;
		}
	}

	return true;
}

void submodel_render_immediate(model_render_params *render_info, polymodel *pm, polymodel_instance *pmi, int submodel_num, matrix *orient, vec3d * pos)
{
	Assert(pm->id == pmi->model_num);

	model_draw_list model_list;
	
	model_list.init();

	submodel_render_queue(render_info, &model_list, pm, pmi, submodel_num, orient, pos);

	model_list.init_render();
	model_list.render_all();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();

	gr_reset_lighting();
}

void submodel_render_queue(model_render_params *render_info, model_draw_list *scene, polymodel *pm, polymodel_instance *pmi, int submodel_num, matrix *orient, vec3d * pos)
{
	Assert(pm->id == pmi->model_num);

	model_material rendering_material;

	//MONITOR_INC( NumModelsRend, 1 );	

	if ( !( Game_detail_flags & DETAIL_FLAG_MODELS ) )	return;
	
	if ( render_info->is_clip_plane_set() ) {
		rendering_material.set_clip_plane(render_info->get_clip_plane_normal(), render_info->get_clip_plane_pos());
	}

	if ( render_info->is_team_color_set() ) {
		rendering_material.set_team_color(render_info->get_team_color());
	}
		
	uint flags = render_info->get_model_flags();
	int objnum = render_info->get_object_number();

	// Set the flags we will pass to the tmapper
	uint tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode
	if( ( The_mission.flags[Mission::Mission_Flags::Fullneb] ) && ( Neb2_render_mode != NEB2_RENDER_NONE ) ) {
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
		rendering_material.set_animated_effect(render_info->get_animated_effect_num(), render_info->get_animated_effect_timer());
	}

	bool is_outlines_only_htl = (flags & MR_NO_POLYS) && (flags & MR_SHOW_OUTLINE_HTL);

	//set to true since D3d and OGL need the api matrices set
	scene->push_transform(pos, orient);

	
	vec3d auto_back = ZERO_VECTOR;
	bool set_autocen = model_render_determine_autocenter(&auto_back, pm, render_info->get_detail_level_lock(), flags);

	if ( set_autocen ) {
		scene->push_transform(&auto_back, NULL);
	}

	if (is_outlines_only_htl) {
		rendering_material.set_fill_mode(GR_FILL_MODE_WIRE);

		color outline_color = render_info->get_color();
		rendering_material.set_color(outline_color);

		tmap_flags &= ~TMAP_FLAG_RGB;
	} else {
		rendering_material.set_fill_mode(GR_FILL_MODE_SOLID);
	}

	rendering_material.set_light_factor(1.0f);
	
	if ( !( flags & MR_NO_LIGHTING ) ) {
		scene->set_light_filter(-1, pos, pm->submodel[submodel_num].rad);
		rendering_material.set_lighting(true);
	} else {
		rendering_material.set_lighting(false);
	}

	// fixes disappearing HUD in OGL - taylor
	rendering_material.set_cull_mode(true);

	// RT - Put this here to fog debris
	if( tmap_flags & TMAP_FLAG_PIXEL_FOG ) {
		float fog_near, fog_far;
		object *obj = NULL;

		if (objnum >= 0)
			obj = &Objects[objnum];

		neb2_get_adjusted_fog_values(&fog_near, &fog_far, nullptr, obj);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		rendering_material.set_fog(r, g, b, fog_near, fog_far);
	} else {
		rendering_material.set_fog();
	}

	if(Rendering_to_shadow_map) {
		rendering_material.set_depth_bias(-1024);
	} else {
		rendering_material.set_depth_bias(0);
	}

	vec3d view_pos = scene->get_view_position();

	if ( model_render_check_detail_box(&view_pos, pm, submodel_num, flags) ) {
		model_render_buffers(scene, &rendering_material, render_info, &pm->submodel[submodel_num].buffer, pm, submodel_num, 0, tmap_flags);

		if ( pm->flags & PM_FLAG_TRANS_BUFFER && pm->submodel[submodel_num].trans_buffer.flags & VB_FLAG_TRANS ) {
			model_render_buffers(scene, &rendering_material, render_info, &pm->submodel[submodel_num].trans_buffer, pm, submodel_num, 0, tmap_flags);
		}
	}
	
	if ( pmi->submodel[submodel_num].num_arcs > 0 )	{
		model_render_add_lightning( scene, render_info, pm, &pmi->submodel[submodel_num] );
	}

	if ( set_autocen ) {
		scene->pop_transform();
	}

	scene->pop_transform();
}

//Renders the sprite for a glowpoint.
void model_render_glowpoint_bitmap(int point_num, vec3d *pos, matrix *orient, glow_point_bank *bank, glow_point_bank_override *gpo, polymodel *pm, polymodel_instance *pmi, ship* shipp, bool use_depth_buffer)
{
	glow_point *gpt = &bank->points[point_num];
	vec3d loc_offset = gpt->pnt;
	vec3d loc_norm = gpt->norm;
	vec3d world_pnt;
	vec3d world_norm;
	vec3d tempv;
	vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
	bool submodel_rotation = false;

	if ( bank->submodel_parent > 0 && pm->submodel[bank->submodel_parent].flags[Model::Submodel_flags::Can_move] && shipp != nullptr ) {
		model_find_submodel_offset(&submodel_static_offset, pm, bank->submodel_parent);

		submodel_rotation = true;
	}

	if ( submodel_rotation ) {
		vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

		tempv = loc_offset;
		model_instance_local_to_global_point_dir(&loc_offset, &loc_norm, &tempv, &loc_norm, pm, pmi, bank->submodel_parent);
	}

	vm_vec_unrotate(&world_pnt, &loc_offset, orient);
	vm_vec_add2(&world_pnt, pos);

	vm_vec_unrotate(&world_norm, &loc_norm, orient);

	if (shipp != nullptr) {
		// don't render if its on the wrong side of the portal
		WarpEffect* warp_effect = nullptr;

		if ((shipp->is_arriving()) && (shipp->warpin_effect != nullptr)
			&& Warp_params[shipp->warpin_params_index].warp_type != WT_HYPERSPACE) {
			warp_effect = shipp->warpin_effect;
		}
		else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect != nullptr) 
			&& Warp_params[shipp->warpout_params_index].warp_type != WT_HYPERSPACE) {
			warp_effect = shipp->warpout_effect;
		}

		if (warp_effect != nullptr && point_is_clipped_by_warp(&world_pnt, warp_effect))
			return;
	}

	switch ((gpo && gpo->type_override)?gpo->type:bank->type)
	{
	case 0:
		{
			float d = 1.0f;
			float pulse = model_render_get_point_activation(bank, gpo);

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
				if ( The_mission.flags[Mission::Mission_Flags::Fullneb] ) {
					//vec3d npnt;
					//vm_vec_add(&npnt, &loc_offset, pos);

					d *= neb2_get_fog_visibility(&world_pnt, Neb2_fog_visibility_glowpoint);
					w *= 1.5;	//make it bigger in a nebula
				}
				
				g3_transfer_vertex(&p, &world_pnt);

				p.r = p.g = p.b = p.a = (ubyte)(255.0f * MAX(d,0.0f));

				if((gpo && gpo->glow_bitmap_override)?(gpo->glow_bitmap > -1):(bank->glow_bitmap > -1)) {
					int gpflags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_EMISSIVE;

					if (use_depth_buffer)
						gpflags |= TMAP_FLAG_SOFT_QUAD;

					int bitmap_id = (gpo && gpo->glow_bitmap_override) ? gpo->glow_bitmap : bank->glow_bitmap;

					if ( use_depth_buffer ) {
						batching_add_volume_bitmap(bitmap_id, &p, 0, (w * 0.5f), d * pulse);
					} else {
						batching_add_bitmap(bitmap_id, &p, 0, (w * 0.5f), d * pulse);
					}
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

			g3_transfer_vertex(&verts[0], &bottom1);
			g3_transfer_vertex(&verts[1], &bottom2);
			g3_transfer_vertex(&verts[2], &top2);
			g3_transfer_vertex(&verts[3], &top1);

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

			if ( The_mission.flags[Mission::Mission_Flags::Fullneb] ) {
				batching_add_quad(bank->glow_neb_bitmap, verts);
			} else {
				batching_add_quad(bank->glow_bitmap, verts);
			}

			break;
		}
	}
}

//adds the glowpoint's lights, if any, to the lights vector.
void model_render_glowpoint_add_light(int point_num, vec3d *pos, matrix *orient, glow_point_bank *bank, glow_point_bank_override *gpo, polymodel *pm, polymodel_instance *pmi, ship* shipp)
{
	if(Detail.lighting <= 3 || !Deferred_lighting || gpo==nullptr || !gpo->is_lightsource) {
		return;
	}
	glow_point *gpt = &bank->points[point_num];
	vec3d loc_offset = gpt->pnt;
	vec3d loc_norm = gpt->norm;
	vec3d world_pnt;
	vec3d world_norm;
	vec3d tempv;
	vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
	bool submodel_rotation = false;

	if ( bank->submodel_parent > 0 && pm->submodel[bank->submodel_parent].flags[Model::Submodel_flags::Can_move] && shipp != nullptr ) {
		model_find_submodel_offset(&submodel_static_offset, pm, bank->submodel_parent);

		submodel_rotation = true;
	}

	if ( submodel_rotation ) {
		vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

		tempv = loc_offset;
		model_instance_local_to_global_point_dir(&loc_offset, &loc_norm, &tempv, &loc_norm, pm, pmi, bank->submodel_parent);
	}

	vm_vec_unrotate(&world_pnt, &loc_offset, orient);
	vm_vec_add2(&world_pnt, pos);

	vm_vec_unrotate(&world_norm, &loc_norm, orient);

	if (shipp != nullptr && shipp->warpout_effect != nullptr) {
		// don't render if its on the wrong side of the portal
		WarpEffect* warp_effect = nullptr;

		if ((shipp->is_arriving()) && Warp_params[shipp->warpin_params_index].warp_type != WT_HYPERSPACE) {
			warp_effect = shipp->warpin_effect;
		}
		else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && Warp_params[shipp->warpout_params_index].warp_type != WT_HYPERSPACE) {
			warp_effect = shipp->warpout_effect;
		}

		if (warp_effect != nullptr && point_is_clipped_by_warp(&world_pnt, warp_effect))
			return;
	}

	if( (gpo->type_override?gpo->type:bank->type)==0)
	{
		float pulse = model_render_get_point_activation(bank, gpo);
		vec3d lightcolor;
		//fade between mix and normal color depending on pulse state
		lightcolor.xyz.x =pulse * gpo->light_color.xyz.x + (1.0f-pulse) * gpo->light_mix_color.xyz.x;
		lightcolor.xyz.y =pulse * gpo->light_color.xyz.y + (1.0f-pulse) * gpo->light_mix_color.xyz.y;
		lightcolor.xyz.z =pulse * gpo->light_color.xyz.z + (1.0f-pulse) * gpo->light_mix_color.xyz.z;
		float light_radius = gpt->radius * gpo->radius_multi;
		if ( gpo->lightcone ) {
			vec3d cone_dir_rot;
			vec3d cone_dir_model;
			vec3d cone_dir_world;
			vec3d cone_dir_screen;

			if ( gpo->rotating ) {
				vm_rot_point_around_line(&cone_dir_rot, &gpo->cone_direction, PI * timestamp() * 0.000033333f * gpo->rotation_speed, &vmd_zero_vector, &gpo->rotation_axis);
			} else {
				cone_dir_rot = gpo->cone_direction;
			}

			model_instance_local_to_global_dir(&cone_dir_model, &cone_dir_rot, pm, pmi, bank->submodel_parent);
			vm_vec_unrotate(&cone_dir_world, &cone_dir_model, orient);
			vm_vec_rotate(&cone_dir_screen, &cone_dir_world, &Eye_matrix);
			cone_dir_screen.xyz.z = -cone_dir_screen.xyz.z;
			light_add_cone(
				&world_pnt, &cone_dir_screen, gpo->cone_angle, gpo->cone_inner_angle, gpo->dualcone, 1.0f, light_radius, 1,
				lightcolor.xyz.x, lightcolor.xyz.y, lightcolor.xyz.z,-1);
		} else {
			light_add_point(&world_pnt, 1.0f, light_radius, 1,	lightcolor.xyz.x, lightcolor.xyz.y, lightcolor.xyz.z, -1);
		}
	}
}

//Returns the current brightness percentage of a glowpoint, from 1.0f to 0.0f
float model_render_get_point_activation(glow_point_bank* bank, glow_point_bank_override* gpo)
{
	if(gpo == nullptr ||  !(gpo->pulse_type)){
		return 1.0f;
	}

	float pulse = 1.0f;
	int period;

	if (gpo->pulse_period_override) {
		period = gpo->pulse_period;
	} else {
		if (gpo->on_time_override) {
			period = 2 * gpo->on_time;
		} else {
			period = 2 * bank->on_time;
		}
	}

	int x = 0;
	int disp_time = gpo->disp_time_override ? gpo->disp_time : bank->disp_time;
  	int on_time = gpo->on_time_override ? gpo->on_time : bank->on_time;
  	int off_time = gpo->off_time_override ? gpo->off_time : bank->off_time;

	if ( off_time) {
  		x = (timestamp() - disp_time) % ( on_time + off_time ) - off_time;
	} else {
  		x = (timestamp() - disp_time) % gpo->pulse_period;
	}

	switch (gpo->pulse_type) {

	case PULSE_SIN:
		pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(sin(PI2 / period * x),gpo->pulse_exponent);
		break;

	case PULSE_COS:
		pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(cos(PI2 / period * x),gpo->pulse_exponent);
		break;

	case PULSE_SHIFTTRI:
		x += period / 4;
		if(off_time) {
			x %= on_time + off_time;
		} else {
			x %= gpo->pulse_period;
		}
		FALLTHROUGH;

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
	return pulse;
}

void model_render_set_glow_points(polymodel *pm, int objnum)
{
	int time = timestamp();
	glow_point_bank_override *gpo = NULL;
	bool override_all = false;
	SCP_unordered_map<int, void*>::iterator gpoi;
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
			gpoi = sip->glowpoint_bank_override_map.find(-1);

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

		int on_time = (gpo && gpo->on_time_override) ? gpo->on_time : bank->on_time;
		int off_time = (gpo && gpo->off_time_override) ? gpo->off_time : bank->off_time;
		int disp_time = (gpo && gpo->disp_time_override) ? gpo->disp_time : bank->disp_time;


		if (off_time) {
			bool glow_state = ((time - disp_time) % (on_time + off_time)) < on_time;

			if ( glow_state != bank->is_on )
					bank->glow_timestamp = time;
			
			bank->is_on = glow_state;
		}
	}
}

//Handle all the glow points of a model being rendered.
void model_render_glow_points(polymodel *pm, polymodel_instance *pmi, ship *shipp, matrix *orient, vec3d *pos, bool use_depth_buffer = true,bool render_sprites = true, bool add_lights= true)
{
	Assert(pmi == nullptr || pm->id == pmi->model_num);

	if ( Rendering_to_shadow_map ) {
		return;
	}

	int i, j;

	int cull = gr_set_cull(0);

	glow_point_bank_override *gpo = nullptr;
	bool override_all = false;
	SCP_unordered_map<int, void*>::iterator gpoi;
	ship_info *sip = nullptr;

	if ( shipp ) {
		sip = &Ship_info[shipp->ship_info_index];
		gpoi = sip->glowpoint_bank_override_map.find(-1);

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
				gpo = nullptr;
			}
		}

		//Only continue if there actually is a glowpoint bitmap available
		if (bank->glow_bitmap == -1)
			continue;

		if (pmi != nullptr) {
			auto smi = &pmi->submodel[bank->submodel_parent];
			if (smi->blown_off) {
				continue;
			}
		}

		if ((gpo && gpo->off_time_override && !gpo->off_time)?gpo->is_on:bank->is_on) {
			if ( (shipp != nullptr) && !(shipp->glow_point_bank_active[i]) )
				continue;

			for (j = 0; j < bank->num_points; j++) {
				Assert( bank->points != nullptr );
				int flick;

				if (pmi != nullptr && pmi->submodel[pm->detail[0]].num_arcs > 0) {
					flick = static_rand( timestamp() % 20 ) % (pmi->submodel[pm->detail[0]].num_arcs + j); //the more damage, the more arcs, the more likely the lights will fail
				} else {
					flick = 1;
				}

				if (flick == 1) {
					//In some cases we want to be able to render only the bitmaps or only the lights
					if(render_sprites)
						model_render_glowpoint_bitmap(j, pos, orient, bank, gpo, pm, pmi, shipp, use_depth_buffer);
					if(add_lights)
						model_render_glowpoint_add_light(j, pos, orient, bank, gpo, pm, pmi, shipp);
				} // flick
			} // for slot
		} // bank is on
	} // for bank

	gr_set_cull(cull);
}

// These scaling functions were adapted from Elecman's code.
// https://forum.unity.com/threads/this-script-gives-you-objects-screen-size-in-pixels.48966/#post-2107126
float convert_pixel_size_and_distance_to_diameter(float pixelsize, float distance, float field_of_view_deg, int screen_height)
{
	float diameter = (pixelsize * distance * field_of_view_deg) / (fl_degrees(screen_height));
	return diameter;
}

// These scaling functions were adapted from Elecman's code.
// https://forum.unity.com/threads/this-script-gives-you-objects-screen-size-in-pixels.48966/#post-2107126
float convert_distance_and_diameter_to_pixel_size(float distance, float diameter, float field_of_view_deg, int screen_height)
{
	float pixel_size = (diameter * fl_degrees(screen_height)) / (distance * field_of_view_deg);
	return pixel_size;
}

float model_render_get_diameter_clamped_to_min_pixel_size(const vec3d* pos, float diameter, float min_pixel_size)
{
	// Don't do any scaling math if the pixel size is set to zero.
	if (min_pixel_size <= FLT_EPSILON)
		return diameter;

	float distance_to_eye = vm_vec_dist(&Eye_position, pos);
	float current_pixel_size = convert_distance_and_diameter_to_pixel_size(
		distance_to_eye,
		diameter,
		fl_degrees(Eye_fov),
		gr_screen.max_h);

	float scaled_diameter = diameter;
	if (current_pixel_size < min_pixel_size) {
		scaled_diameter = convert_pixel_size_and_distance_to_diameter(
			min_pixel_size,
			distance_to_eye,
			fl_degrees(Eye_fov),
			gr_screen.max_h);
	}

	return scaled_diameter;
}

void model_queue_render_thrusters(model_render_params *interp, polymodel *pm, int objnum, ship *shipp, matrix *orient, vec3d *pos)
{
	int i, j;
	int n_q = 0;
	size_t 	k;
	vec3d norm, norm2, fvec, pnt, npnt;
	thruster_bank *bank = nullptr;
	polymodel_instance *pmi = nullptr;
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
		if ( bank->submodel_num > -1 && pm->submodel[bank->submodel_num].flags[Model::Submodel_flags::Can_move] && (gameseq_get_state_idx(GS_STATE_LAB) == -1) ) {
			model_find_submodel_offset(&submodel_static_offset, pm, bank->submodel_num);

			submodel_rotation = true;
		}

		for (j = 0; j < bank->num_points; j++) {
			Assert( bank->points != NULL );

			float d;
			vec3d tempv;
			glow_point *gpt = &bank->points[j];
			vec3d loc_offset = gpt->pnt;
			vec3d loc_norm = gpt->norm;
			vec3d world_pnt;
			vec3d world_norm;

			if ( submodel_rotation ) {
				vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);
				tempv = loc_offset;

				if (pmi == nullptr)
					pmi = model_get_instance(shipp->model_instance_num);

				model_instance_local_to_global_point_dir(&loc_offset, &loc_norm, &tempv, &loc_norm, pm, pmi, bank->submodel_num);
			}

			vm_vec_unrotate(&world_pnt, &loc_offset, orient);
			vm_vec_add2(&world_pnt, pos);

			if (shipp) {
				// if ship is warping out, check position of the engine glow to the warp plane
				WarpEffect* warp_effect = nullptr;

				if ((shipp->is_arriving()) && (shipp->warpin_effect != nullptr)
					&& Warp_params[shipp->warpin_params_index].warp_type != WT_HYPERSPACE) {
					warp_effect = shipp->warpin_effect;
				}
				else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect != nullptr)
					&& Warp_params[shipp->warpout_params_index].warp_type != WT_HYPERSPACE) {
					warp_effect = shipp->warpout_effect;
				}

				if (warp_effect != nullptr && point_is_clipped_by_warp(&world_pnt, warp_effect))
					continue;
			}

			vm_vec_sub(&tempv, &View_position, &world_pnt);
			vm_vec_normalize(&tempv);
			vm_vec_unrotate(&world_norm, &loc_norm, orient);
			d = vm_vec_dot(&tempv, &world_norm);

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
			(scale_vec.xyz.z *= thruster_info.length.xyz.z) -= 0.1f;

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
			if (The_mission.flags[Mission::Mission_Flags::Fullneb]) {
				vm_vec_unrotate(&npnt, &gpt->pnt, orient);
				vm_vec_add2(&npnt, pos);

				fog_int = neb2_get_fog_visibility(&npnt, Neb2_fog_visibility_thruster);

				if (fog_int > 1.0f)
					fog_int = 1.0f;

				d *= fog_int;

				if (d > 1.0f)
					d = 1.0f;
			}

			// Scale the thrusters so they always appears at least some configured amount of pixels wide.
			float scaled_thruster_radius = model_render_get_diameter_clamped_to_min_pixel_size(
				&world_pnt,
				gpt->radius * 2.0f,
				Min_pixel_size_thruster);
			scaled_thruster_radius /= 2.0f;

			float w = scaled_thruster_radius * (scale + thruster_info.glow_noise * NOISE_SCALE);

			// these lines are used by the tertiary glows, thus we will need to project this all of the time
			g3_transfer_vertex( &p, &world_pnt );

			// start primary thruster glows
			if ( (thruster_info.primary_glow_bitmap >= 0) && (d > 0.0f) ) {
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * d);
				batching_add_volume_bitmap(thruster_info.primary_glow_bitmap, &p, 0, (w * 0.5f * thruster_info.glow_rad_factor), d, w * 0.325f);
			}

			// start tertiary thruster glows
			if (thruster_info.tertiary_glow_bitmap >= 0) {
				p.screen.xyw.w -= w;
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * fog_int);
				batching_add_volume_bitmap_rotated(thruster_info.tertiary_glow_bitmap, &p, magnitude * 4, w * 0.6f * thruster_info.tertiary_glow_rad_factor, fog_int, -w*0.5f);
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

					if (The_mission.flags[Mission::Mission_Flags::Fullneb]) {
						vm_vec_add(&npnt, &pnt, pos);
						d *= fog_int;
					}

					batching_add_beam(thruster_info.secondary_glow_bitmap, &pnt, &norm2, wVal*thruster_info.secondary_glow_rad_factor*0.5f, d);

					if (Scene_framebuffer_in_frame && thruster_info.draw_distortion &&
					    Gr_framebuffer_effects[FramebufferEffects::Thrusters]) {
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

						batching_add_distortion_beam(dist_bitmap, &pnt, &norm2, wVal*thruster_info.distortion_rad_factor*0.5f, 1.0f, mag);
					}
				}
			}

			// begin particles
			if (shipp) {
				ship_info *sip = &Ship_info[shipp->ship_info_index];
				particle::particle_emitter pe;
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
					pe.min_life = 0.0f;
					pe.max_life = 1.0f;

					particle::emit( &pe, particle::PARTICLE_BITMAP, tp->thruster_bitmap.first_frame);
				}
			}
		}
	}
}

void model_render_insignias(insignia_draw_data *insignia_data)
{
	polymodel *pm = insignia_data->pm;
	int detail_level = insignia_data->detail_level;
	int bitmap_num = insignia_data->bitmap_num;

	// if the model has no insignias, or we don't have a texture, then bail
	if ( (pm->num_ins <= 0) || (bitmap_num < 0) )
		return;

	int idx, s_idx;
	vertex vecs[3];
	vec3d t1, t2, t3;
	int i1, i2, i3;

	material insignia_material;
	insignia_material.set_depth_bias(1);

	// set the proper texture
	material_set_unlit(&insignia_material, bitmap_num, 0.65f, true, true);

	if ( insignia_data->clip ) {
		insignia_material.set_clip_plane(insignia_data->clip_normal, insignia_data->clip_position);
	}

	// otherwise render them	
	for(idx=0; idx<pm->num_ins; idx++){	
		// skip insignias not on our detail level
		if(pm->ins[idx].detail_level != detail_level){
			continue;
		}

		for(s_idx=0; s_idx<pm->ins[idx].num_faces; s_idx++){
			// get vertex indices
			i1 = pm->ins[idx].faces[s_idx][0];
			i2 = pm->ins[idx].faces[s_idx][1];
			i3 = pm->ins[idx].faces[s_idx][2];

			// transform vecs and setup vertices
			vm_vec_add(&t1, &pm->ins[idx].vecs[i1], &pm->ins[idx].offset);
			vm_vec_add(&t2, &pm->ins[idx].vecs[i2], &pm->ins[idx].offset);
			vm_vec_add(&t3, &pm->ins[idx].vecs[i3], &pm->ins[idx].offset);

			g3_transfer_vertex(&vecs[0], &t1);
			g3_transfer_vertex(&vecs[1], &t2);
			g3_transfer_vertex(&vecs[2], &t3);

			// setup texture coords
			vecs[0].texture_position.u = pm->ins[idx].u[s_idx][0];
			vecs[0].texture_position.v = pm->ins[idx].v[s_idx][0];

			vecs[1].texture_position.u = pm->ins[idx].u[s_idx][1];
			vecs[1].texture_position.v = pm->ins[idx].v[s_idx][1];

			vecs[2].texture_position.u = pm->ins[idx].u[s_idx][2];
			vecs[2].texture_position.v = pm->ins[idx].v[s_idx][2];

			light_apply_rgb( &vecs[0].r, &vecs[0].g, &vecs[0].b, &pm->ins[idx].vecs[i1], &pm->ins[idx].norm[i1], 1.5f );
			light_apply_rgb( &vecs[1].r, &vecs[1].g, &vecs[1].b, &pm->ins[idx].vecs[i2], &pm->ins[idx].norm[i2], 1.5f );
			light_apply_rgb( &vecs[2].r, &vecs[2].g, &vecs[2].b, &pm->ins[idx].vecs[i3], &pm->ins[idx].norm[i3], 1.5f );
			vecs[0].a = vecs[1].a = vecs[2].a = 255;

			// draw the polygon
			g3_render_primitives_colored_textured(&insignia_material, vecs, 3, PRIM_TYPE_TRIFAN, false);
		}
	}
}

void model_render_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width)
{
	Num_arc_segment_points = 0;

	// need need to add the first point
	memcpy( &Arc_segment_points[Num_arc_segment_points++], v1, sizeof(vec3d) );

	// this should fill in all of the middle, and the last, points
	interp_render_arc_segment(v1, v2, 0);

	// use primary color for fist pass
	Assert( primary );

	g3_render_rod(primary, Num_arc_segment_points, Arc_segment_points, arc_width);

	if (secondary) {
		g3_render_rod(secondary, Num_arc_segment_points, Arc_segment_points, arc_width * 0.33f);
	}
}

void model_render_debug_children(polymodel *pm, int mn, int detail_level, uint debug_flags)
{
	int i;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *model = &pm->submodel[mn];

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	// (Not needed here because we're not using model instances)
	matrix submodel_orient = vmd_identity_matrix;

	g3_start_instance_matrix(&model->offset, &submodel_orient, true);

	if ( debug_flags & MR_DEBUG_PIVOTS ) {
		model_draw_debug_points( pm, &pm->submodel[mn], debug_flags );
	}

	i = model->first_child;

	while ( i >= 0 ) {
		model_render_debug_children( pm, i, detail_level, debug_flags );

		i = pm->submodel[i].next_sibling;
	}

	g3_done_instance(true);
}

void model_render_debug(int model_num, matrix *orient, vec3d * pos, uint flags, uint debug_flags, int objnum, int detail_level_locked )
{
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
		model_draw_paths_htl(model_num, flags);
	}

	if ( debug_flags & MR_DEBUG_BAY_PATHS ) {
		model_draw_bay_paths_htl(model_num);
	}

	if ( (flags & MR_AUTOCENTER) && (set_autocen) ) {
		g3_done_instance(true);
	}

	g3_done_instance(true);

	gr_zbuffer_set(save_gr_zbuffering_mode);
}

void model_render_immediate(model_render_params* render_info, int model_num, matrix* orient, vec3d* pos, int render, bool sort)
{
	model_render_immediate(render_info, model_num, -1, orient, pos, render, sort);
}

void model_render_immediate(model_render_params* render_info, int model_num, int model_instance_num, matrix* orient, vec3d* pos, int render, bool sort)
{
	model_draw_list model_list;

	model_list.init();

	model_render_queue(render_info, &model_list, model_num, model_instance_num, orient, pos);

	model_list.init_render(sort);

	switch ( render ) {
	case MODEL_RENDER_OPAQUE:
		model_list.render_all(ZBUFFER_TYPE_FULL);
		break;
	case MODEL_RENDER_TRANS:
		model_list.render_all(ZBUFFER_TYPE_READ);
		model_list.render_all(ZBUFFER_TYPE_NONE);
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

	gr_reset_lighting();

	if ( render_info->get_debug_flags() ) {
		model_render_debug(model_num, orient, pos, render_info->get_model_flags(), render_info->get_debug_flags(), render_info->get_object_number(), render_info->get_detail_level_lock());
	}
}

void model_render_queue(model_render_params* interp, model_draw_list* scene, int model_num, matrix* orient, vec3d* pos)
{
	model_render_queue(interp, scene, model_num, -1, orient, pos);
}

void model_render_queue(model_render_params* interp, model_draw_list* scene, int model_num, int model_instance_num, matrix* orient, vec3d* pos)
{
	int i;

	const int objnum = interp->get_object_number();
	const int model_flags = interp->get_model_flags();

	model_material rendering_material;
	polymodel *pm = model_get(model_num);
	polymodel_instance *pmi = NULL;
		
	float light_factor = model_render_determine_light_factor(interp, pos, model_flags);

	if ( light_factor < (1.0f/32.0f) ) {
		// If it is too far, exit
		return;
	}

	rendering_material.set_light_factor(light_factor);

	if ( interp->is_clip_plane_set() ) {
		rendering_material.set_clip_plane(interp->get_clip_plane_normal(), interp->get_clip_plane_pos());
	}

	if ( interp->is_team_color_set() ) {
		rendering_material.set_team_color(interp->get_team_color());
	}
		
	if ( model_flags & MR_FORCE_CLAMP ) {
		rendering_material.set_texture_addressing(TMAP_ADDRESS_CLAMP);
	} else {
		rendering_material.set_texture_addressing(TMAP_ADDRESS_WRAP);
	}

	model_render_set_glow_points(pm, objnum);

	if ( !(model_flags & MR_NO_LIGHTING) ) {
		scene->set_light_filter(objnum, pos, pm->rad);
	}

	ship *shipp = NULL;
	object *objp = NULL;

	if (objnum >= 0) {
		objp = &Objects[objnum];
		int tentative_num = -1;

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];
			tentative_num = shipp->model_instance_num;
		}
		else {
			tentative_num = object_get_model_instance(objp);
		}

		if (tentative_num >= 0) {
			model_instance_num = tentative_num;
		}
	}

	// is this a skybox with a rotating submodel?
	if (model_num == Nmodel_num && Nmodel_instance_num >= 0) {
		model_instance_num = Nmodel_instance_num;
	}

	if (model_instance_num >= 0) {
		pmi = model_get_instance(model_instance_num);

		// This can happen if we supply a HUD target model for a real ship.
		// The passed parameter was -1 but it was assigned an instance from
		// the actual object.  Set it back to -1 if there is a mismatch.
		if (pmi->model_num != pm->id) {
			model_instance_num = -1;
			pmi = nullptr;
		}
	}
	
	// Set the flags we will pass to the tmapper
	uint tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	if ( !(model_flags & MR_NO_TEXTURING) )	{
		tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			tmap_flags |= TMAP_FLAG_TILED;

		if ( !(model_flags & MR_NO_CORRECT) )	{
			tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	if ( model_flags & MR_DESATURATED ) {
		rendering_material.set_desaturation(true);
	}

	if ( interp->get_animated_effect_num() >= 0 ) {
		rendering_material.set_animated_effect(interp->get_animated_effect_num(), interp->get_animated_effect_timer());
	}

	bool is_outlines_only = (model_flags & MR_NO_POLYS) && ((model_flags & MR_SHOW_OUTLINE_PRESET) || (model_flags & MR_SHOW_OUTLINE));
	bool is_outlines_only_htl = (model_flags & MR_NO_POLYS) && (model_flags & MR_SHOW_OUTLINE_HTL);

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

	// if we're in nebula mode, fog everything except for the warp holes and other non-fogged models
	if ( (The_mission.flags[Mission::Mission_Flags::Fullneb]) && (Neb2_render_mode != NEB2_RENDER_NONE) && !(model_flags & MR_NO_FOGGING) ) {
		float fog_near = 10.0f, fog_far = 1000.0f;
		neb2_get_adjusted_fog_values(&fog_near, &fog_far, nullptr, objp);

		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		rendering_material.set_fog(r, g, b, fog_near, fog_far);
	} else {
		rendering_material.set_fog();
	}

	if (interp->is_alpha_mult_set()) {
		float alphamult = 1.0f;

		if (interp->is_alpha_mult_set()) {
			alphamult *= interp->get_alpha_mult();
		}
		rendering_material.set_alpha_mult(alphamult);
	}

	if ( is_outlines_only_htl ) {
		rendering_material.set_fill_mode(GR_FILL_MODE_WIRE);

		tmap_flags &= ~TMAP_FLAG_RGB;
	} else {
		rendering_material.set_fill_mode(GR_FILL_MODE_SOLID);
	}

	color clr = interp->get_color();
	rendering_material.set_color(clr);
		
	if ( model_flags & MR_EDGE_ALPHA ) {
		rendering_material.set_center_alpha(-1);
	} else if ( model_flags & MR_CENTER_ALPHA ) {
		rendering_material.set_center_alpha(1);
	} else {
		rendering_material.set_center_alpha(0);
	}

	if ( interp->is_normal_alpha_set() ) {
		rendering_material.set_normal_alpha(interp->get_normal_alpha_min(), interp->get_normal_alpha_max());
	}

	if ( interp->uses_thick_outlines() ) {
		rendering_material.set_outline_thickness(interp->get_outline_thickness());
	}

	if ( ( model_flags & MR_NO_CULL ) || ( model_flags & MR_ALL_XPARENT ) || ( interp->get_warp_bitmap() >= 0 ) ) {
		rendering_material.set_cull_mode(false);
	} else {
		rendering_material.set_cull_mode(true);
	}

	if ( !(model_flags & MR_NO_LIGHTING) ) {
		rendering_material.set_lighting(true);
	} else {
		rendering_material.set_lighting(false);
	}

	Assertion(!(model_flags & MR_STENCIL_READ && model_flags & MR_STENCIL_WRITE),
			  "Enabling stencil read and write at the same time is not supported!");

	if (model_flags & MR_STENCIL_READ) {
		rendering_material.set_stencil_test(true);
		rendering_material.set_stencil_func(ComparisionFunction::NotEqual, 1, 0xFFFF);
		rendering_material.set_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
	} else if (model_flags & MR_STENCIL_WRITE) {
		rendering_material.set_stencil_test(true);
		rendering_material.set_stencil_func(ComparisionFunction::Always, 1, 0xFFFF);
		rendering_material.set_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Replace);
	} else {
		rendering_material.set_stencil_test(false);
		rendering_material.set_stencil_func(ComparisionFunction::Never, 1, 0xFFFF);
		rendering_material.set_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
	}

	if (model_flags & MR_NO_COLOR_WRITES) {
		rendering_material.set_color_mask(false, false, false, false);
	} else {
		rendering_material.set_color_mask(true, true, true, true);
	}

	if ( Rendering_to_shadow_map ) {
		rendering_material.set_depth_bias(-1024);
	} else {
		rendering_material.set_depth_bias(0);
	}

	if ( !(model_flags & MR_NO_BATCH) && pm->flags & PM_FLAG_BATCHED
		&& !(is_outlines_only || is_outlines_only_htl) ) {
		// always set batched rendering on if supported
		tmap_flags |= TMAP_FLAG_BATCH_TRANSFORMS;
	}

	if ( (tmap_flags & TMAP_FLAG_BATCH_TRANSFORMS) ) {
		scene->start_model_batch(pm->n_models);
		model_render_buffers(scene, &rendering_material, interp, &pm->detail_buffers[detail_level], pm, -1, detail_level, tmap_flags);
	}
		
	// Draw the subobjects
	bool draw_thrusters = false;
	bool trans_buffer = false;
	i = pm->submodel[pm->detail[detail_level]].first_child;

	while( i >= 0 )	{
		if ( !pm->submodel[i].flags[Model::Submodel_flags::Is_thruster] ) {
			model_render_children_buffers( scene, &rendering_material, interp, pm, pmi, i, detail_level, tmap_flags, trans_buffer );
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
			color outline_color = interp->get_color();
			scene->add_outline(pm->submodel[detail_model_num].outline_buffer, pm->submodel[detail_model_num].n_verts_outline, &outline_color);
		} else {
			model_render_buffers(scene, &rendering_material, interp, &pm->submodel[detail_model_num].buffer, pm, detail_model_num, detail_level, tmap_flags);

			if ( pmi != nullptr && pmi->submodel[detail_model_num].num_arcs > 0 ) {
				model_render_add_lightning( scene, interp, pm, &pmi->submodel[detail_model_num] );
			}
		}
	}
	
	// make sure batch rendering is unconditionally off.
	tmap_flags &= ~TMAP_FLAG_BATCH_TRANSFORMS;

	if ( pm->flags & PM_FLAG_TRANS_BUFFER && !(is_outlines_only || is_outlines_only_htl) ) {
		trans_buffer = true;
		i = pm->submodel[pm->detail[detail_level]].first_child;

		while( i >= 0 )	{
			if ( !pm->submodel[i].flags[Model::Submodel_flags::Is_thruster] ) {
				model_render_children_buffers( scene, &rendering_material, interp, pm, pmi, i, detail_level, tmap_flags, trans_buffer );
			}

			i = pm->submodel[i].next_sibling;
		}

		view_pos = scene->get_view_position();

		if ( model_render_check_detail_box(&view_pos, pm, pm->detail[detail_level], model_flags) ) {
			int detail_model_num = pm->detail[detail_level];
			model_render_buffers(scene, &rendering_material, interp, &pm->submodel[detail_model_num].trans_buffer, pm, detail_model_num, detail_level, tmap_flags);
		}
	}

	// Draw the thruster subobjects
	if ( draw_thrusters && !(is_outlines_only || is_outlines_only_htl) ) {
		i = pm->submodel[pm->detail[detail_level]].first_child;
		trans_buffer = false;

		while( i >= 0 ) {
			if (pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
				model_render_children_buffers( scene, &rendering_material, interp, pm, pmi, i, detail_level, tmap_flags, trans_buffer );
			}
			i = pm->submodel[i].next_sibling;
		}
	}

	if ( !( model_flags & MR_NO_TEXTURING ) ) {
		scene->add_insignia(interp, pm, detail_level, interp->get_insignia_bitmap());
	}

	if ( (model_flags & MR_AUTOCENTER) && (set_autocen) ) {
		scene->pop_transform();
	}

	scene->pop_transform();

	// start rendering glow points -Bobboau
	if ( (pm->n_glow_point_banks) && !is_outlines_only && !is_outlines_only_htl && !Glowpoint_override ) {
		model_render_glow_points(pm, pmi, shipp, orient, pos, Glowpoint_use_depth_buffer);
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

//Renders the glowpoint light sources of a model without rendering anything else of the model.
void model_render_only_glowpoint_lights(model_render_params* interp, int model_num, int model_instance_num, matrix* orient, vec3d* pos)
{
	const int objnum = interp->get_object_number();
	const int model_flags = interp->get_model_flags();

	polymodel *pm = model_get(model_num);
	polymodel_instance *pmi = nullptr;

	model_render_set_glow_points(pm, objnum);

	ship *shipp = nullptr;
	object *objp = nullptr;

	if (objnum >= 0) {
		objp = &Objects[objnum];
		int tentative_num = -1;

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];
			tentative_num = shipp->model_instance_num;
		}
		else {
			tentative_num = object_get_model_instance(objp);
		}

		if (tentative_num >= 0) {
			model_instance_num = tentative_num;
		}
	}

	// is this a skybox with a rotating submodel?
	if (model_num == Nmodel_num && Nmodel_instance_num >= 0) {
		model_instance_num = Nmodel_instance_num;
	}

	if (model_instance_num >= 0) {
		pmi = model_get_instance(model_instance_num);

		// This can happen if we supply a HUD target model for a real ship.
		// The passed parameter was -1 but it was assigned an instance from
		// the actual object.  Set it back to -1 if there is a mismatch.
		if (pmi->model_num != pm->id) {
			model_instance_num = -1;
			pmi = nullptr;
		}
	}

	bool is_outlines_only = (model_flags & MR_NO_POLYS) && ((model_flags & MR_SHOW_OUTLINE_PRESET) || (model_flags & MR_SHOW_OUTLINE));
	bool is_outlines_only_htl = (model_flags & MR_NO_POLYS) && (model_flags & MR_SHOW_OUTLINE_HTL);

	// start rendering glow points -Bobboau
	if ( (pm->n_glow_point_banks) && !is_outlines_only && !is_outlines_only_htl && !Glowpoint_override ) {
		model_render_glow_points(pm, pmi, shipp, orient, pos, Glowpoint_use_depth_buffer,false,true);
	}
}

void model_render_set_wireframe_color(color* clr)
{
	Wireframe_color = *clr;
}
