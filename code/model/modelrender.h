/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _MODELRENDER_H
#define _MODELRENDER_H

#include "graphics/material.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "mission/missionparse.h"
#include "graphics/util/UniformBuffer.h"

extern SCP_vector<light> Lights;
extern int Num_lights;

extern bool Rendering_to_shadow_map;

extern matrix Object_matrix;
extern vec3d Object_position;

extern color Wireframe_color;

inline int in_box(vec3d *min, vec3d *max, vec3d *pos, vec3d *view_position)
{
	vec3d point;

	vm_vec_sub(&point, view_position, pos);

	if ( (point.xyz.x >= min->xyz.x) && (point.xyz.x <= max->xyz.x)
		&& (point.xyz.y >= min->xyz.y) && (point.xyz.y <= max->xyz.y)
		&& (point.xyz.z >= min->xyz.z) && (point.xyz.z <= max->xyz.z) )
	{
		return 1;
	}

	return -1;
}

inline int in_sphere(vec3d *pos, float radius, vec3d *view_position)
{
	if ( vm_vec_dist(view_position, pos) <= radius )
		return 1;
	else
		return -1;
}

extern int model_interp_get_texture(texture_info *tinfo, int elapsed_time);

class model_render_params
{
	uint Model_flags;
	uint Debug_flags;

	int Objnum;
	
	int Detail_level_locked;

	float Depth_scale;

	int Warp_bitmap;
	float Warp_alpha;
	vec3d Warp_scale;

	color Color;

	float Xparent_alpha;

	int Forced_bitmap;

	int Insignia_bitmap;

	int *Replacement_textures;
	bool Manage_replacement_textures; // This is set when we are rendering a model without an associated ship object;
									  // in that case, model_render_params is responsible for allocating and destroying
									  // the Replacement_textures array (this is handled elsewhere otherwise)

	bool Team_color_set;
	team_color Current_team_color;

	bool Clip_plane_set;
	vec3d Clip_normal;
	vec3d Clip_pos;

	int Animated_effect;
	float Animated_timer;

	mst_info Thruster_info;

	float Outline_thickness = -1.0f;

	bool Use_alpha_mult;
	float Alpha_mult;

	model_render_params(const model_render_params&) = delete;
	model_render_params& operator=(const model_render_params&) = delete;
public:
	model_render_params();
	~model_render_params();

	void set_flags(uint flags);
	void set_debug_flags(uint flags);
	void set_object_number(int num);
	void set_detail_level_lock(int detail_level_lock);
	void set_depth_scale(float scale);
	void set_warp_params(int bitmap, float alpha, vec3d &scale);
	void set_color(color &clr);
	void set_color(int r, int g, int b);
	void set_alpha(float alpha);
	void set_forced_bitmap(int bitmap);
	void set_insignia_bitmap(int bitmap);
	void set_replacement_textures(int *textures);
	void set_replacement_textures(int modelnum, SCP_vector<texture_replace>& replacement_textures);
	void set_team_color(team_color &clr);
	void set_team_color(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime);
	void set_clip_plane(vec3d &pos, vec3d &normal);
	void set_animated_effect(int effect_num, float timer);
	void set_thruster_info(mst_info &info);
	void set_outline_thickness(float thick);
	void set_alpha_mult(float alpha);

	bool is_clip_plane_set();
	bool is_team_color_set();
	bool is_alpha_mult_set();
	bool uses_thick_outlines();

	uint get_model_flags();
	uint get_debug_flags();
	int get_object_number();
	int get_detail_level_lock();
	float get_depth_scale();
	int get_warp_bitmap();
	float get_warp_alpha();
	const vec3d& get_warp_scale();
	const color& get_color();
	float get_alpha();
	int get_forced_bitmap();
	int get_insignia_bitmap();
	const int* get_replacement_textures();
	const team_color& get_team_color();
	const vec3d& get_clip_plane_pos();
	const vec3d& get_clip_plane_normal();
	int get_animated_effect_num();
	float get_animated_effect_timer();
	const mst_info& get_thruster_info();
	float get_outline_thickness();
	float get_alpha_mult();
};

struct arc_effect
{
	matrix4 transform;
	vec3d v1;
	vec3d v2;
	color primary;
	color secondary;
	float width;
};

struct insignia_draw_data
{
	matrix4 transform;
	polymodel *pm;
	int detail_level;
	int bitmap_num;

	// if there's a clip plane
	bool clip;
	vec3d clip_normal;
	vec3d clip_position;
};

struct queued_buffer_draw
{
	size_t transform_buffer_offset = 0;
	size_t uniform_buffer_offset = 0;

	model_material render_material;

	matrix4 transform;
	vec3d scale;

	indexed_vertex_source *vert_src;
	vertex_buffer *buffer;
	size_t texi;
	int flags;
	int sdr_flags;

	light_indexing_info lights;

	queued_buffer_draw()
	{
	}
};

struct outline_draw
{
	vertex* vert_array;
	int n_verts;

	matrix4 transform;
	color clr;
};

class model_batch_buffer
{
	SCP_vector<matrix4> Submodel_matrices;
	void* Mem_alloc;
	size_t Mem_alloc_size;

	size_t Current_offset;

	void allocate_memory();
public:
	model_batch_buffer() : Mem_alloc(NULL), Mem_alloc_size(0), Current_offset(0) {};

	void reset();

	size_t get_buffer_offset();
	void set_num_models(int n_models);
	void set_model_transform(matrix4 &transform, int model_id);

	void submit_buffer_data();

	void add_matrix(matrix4 &mat);
};

class model_draw_list
{
	vec3d Current_scale;
	transform_stack Transformations;

	scene_lights Scene_light_handler;
	light_indexing_info Current_lights_set;

	void render_arc(arc_effect &arc);
	void render_insignia(insignia_draw_data &insignia_info);
	void render_outline(outline_draw &outline_info);
	void render_buffer(queued_buffer_draw &render_elements);
	
	SCP_vector<queued_buffer_draw> Render_elements;
	SCP_vector<int> Render_keys;

	SCP_vector<arc_effect> Arcs;
	SCP_vector<insignia_draw_data> Insignias;
	SCP_vector<outline_draw> Outlines;

	graphics::util::UniformBuffer _dataBuffer;

	bool Render_initialized = false; //!< A flag for checking if init_render has been called before a render_all call
	
	static bool sort_draw_pair(model_draw_list* target, const int a, const int b);
	void sort_draws();

	void build_uniform_buffer();
public:
	model_draw_list();
	~model_draw_list();

	model_draw_list(const model_draw_list&) = delete;
	model_draw_list& operator=(const model_draw_list&) = delete;

	void init();

	void add_submodel_to_batch(int model_num);
	void start_model_batch(int n_models);

	void add_buffer_draw(model_material *render_material, indexed_vertex_source *vert_src, vertex_buffer *buffer, size_t texi, uint tmap_flags);
	
	vec3d get_view_position();
	void push_transform(vec3d* pos, matrix* orient);
	void pop_transform();
	void set_scale(vec3d *scale = NULL);

	void add_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width);
	void render_arcs();

	void add_insignia(model_render_params *params, polymodel *pm, int detail_level, int bitmap_num);
	void render_insignias();

	void add_outline(vertex* vert_array, int n_verts, color *clr);
	void render_outlines();

	void set_light_filter(vec3d *pos, float rad);

	void init_render(bool sort = true);
	void render_all(gr_zbuffer_type depth_mode = ZBUFFER_TYPE_DEFAULT);
	void reset();
};
void model_render_only_glowpoint_lights(model_render_params* interp, int model_num, int model_instance_num, matrix* orient, vec3d* pos);
float model_render_get_point_activation(glow_point_bank* bank, glow_point_bank_override* gpo);
void model_render_glowpoint_add_light(int point_num, vec3d *pos, matrix *orient, glow_point_bank *bank, glow_point_bank_override *gpo, polymodel *pm, polymodel_instance *pmi, ship* shipp);
void model_render_immediate(model_render_params* render_info, int model_num, matrix* orient, vec3d* pos, int render = MODEL_RENDER_ALL, bool sort = true);
void model_render_immediate(model_render_params* render_info, int model_num, int model_instance_num, matrix* orient, vec3d* pos, int render = MODEL_RENDER_ALL, bool sort = true);
void model_render_queue(model_render_params* render_info, model_draw_list* scene, int model_num, matrix* orient, vec3d* pos);
void model_render_queue(model_render_params* render_info, model_draw_list* scene, int model_num, int model_instance_num, matrix* orient, vec3d* pos);
void submodel_render_immediate(model_render_params *render_info, polymodel *pm, polymodel_instance *pmi, int submodel_num, matrix *orient, vec3d * pos);
void submodel_render_queue(model_render_params *render_info, model_draw_list *scene, polymodel *pm, polymodel_instance *pmi, int submodel_num, matrix *orient, vec3d * pos);
void model_render_buffers(model_draw_list* scene, model_material *rendering_material, model_render_params* interp, vertex_buffer *buffer, polymodel *pm, int mn, int detail_level, uint tmap_flags);
bool model_render_check_detail_box(vec3d *view_pos, polymodel *pm, int submodel_num, uint flags);
void model_render_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width);
void model_render_insignias(insignia_draw_data *insignia);
void model_render_set_wireframe_color(color* clr);
bool render_tech_model(bool is_ship, int x1, int y1, int x2, int y2, float zoom, bool lighting, int class_idx, matrix* orient);

float model_render_get_diameter_clamped_to_min_pixel_size(const vec3d* pos, float diameter, float min_pixel_size);

void model_render_determine_color(color *clr, float alpha, gr_alpha_blend blend_mode, bool no_texturing, bool desaturate);
gr_alpha_blend model_render_determine_blend_mode(int base_bitmap, bool blending);

#endif
