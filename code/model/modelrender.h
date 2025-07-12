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

typedef enum {
	TECH_SHIP,
	TECH_WEAPON,
	TECH_POF,
	TECH_JUMP_NODE
} tech_render_type;

inline int in_box(const vec3d *min, const vec3d *max, const vec3d *pos, const vec3d *view_position)
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

inline int in_sphere(const vec3d *pos, float radius, const vec3d *view_position)
{
	if ( vm_vec_dist(view_position, pos) <= radius )
		return 1;
	else
		return -1;
}

extern int model_interp_get_texture(const texture_info *tinfo, int elapsed_time);

class model_render_params
{
	uint64_t Model_flags;
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

	std::shared_ptr<const model_texture_replace> Replacement_textures;

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

	void set_flags(uint64_t flags);
	void set_debug_flags(uint flags);
	void set_object_number(int num);
	void set_detail_level_lock(int detail_level_lock);
	void set_depth_scale(float scale);
	void set_warp_params(int bitmap, float alpha, const vec3d &scale);
	void set_color(const color &clr);
	void set_color(int r, int g, int b);
	void set_alpha(float alpha);
	void set_forced_bitmap(int bitmap);
	void set_insignia_bitmap(int bitmap);
	void set_replacement_textures(std::shared_ptr<const model_texture_replace> textures);
	void set_replacement_textures(int modelnum, const SCP_vector<texture_replace>& replacement_textures);
	void set_team_color(const team_color &clr);
	void set_team_color(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime);
	void set_clip_plane(const vec3d &pos, const vec3d &normal);
	void set_animated_effect(int effect_num, float timer);
	void set_thruster_info(const mst_info &info);
	void set_outline_thickness(float thick);
	void set_alpha_mult(float alpha);

	bool is_clip_plane_set() const;
	bool is_team_color_set() const;
	bool is_alpha_mult_set() const;
	bool uses_thick_outlines() const;

	uint64_t get_model_flags() const;
	uint get_debug_flags() const;
	int get_object_number() const;
	int get_detail_level_lock() const;
	float get_depth_scale() const;
	int get_warp_bitmap() const;
	float get_warp_alpha() const;
	const vec3d& get_warp_scale() const;
	const color& get_color() const;
	float get_alpha() const;
	int get_forced_bitmap() const;
	int get_insignia_bitmap() const;
	std::shared_ptr<const model_texture_replace> get_replacement_textures() const;
	const team_color& get_team_color() const;
	const vec3d& get_clip_plane_pos() const;
	const vec3d& get_clip_plane_normal() const;
	int get_animated_effect_num() const;
	float get_animated_effect_timer() const;
	const mst_info& get_thruster_info() const;
	float get_outline_thickness() const;
	float get_alpha_mult() const;
};

struct arc_effect
{
	matrix4 transform;
	vec3d v1;
	vec3d v2;
	color primary;
	color secondary;
	float width;
	ubyte segment_depth;

	const SCP_vector<vec3d> *persistent_arc_points;
};

struct insignia_draw_data
{
	matrix4 transform;
	const polymodel *pm;
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

	const indexed_vertex_source *vert_src;
	const vertex_buffer *buffer;
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
	const vertex* vert_array;
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

	size_t get_buffer_offset() const;
	void set_num_models(int n_models);
	void set_model_transform(const matrix4 &transform, int model_id);

	void submit_buffer_data();

	void add_matrix(const matrix4 &mat);
};

class model_draw_list
{
	vec3d Current_scale;
	transform_stack Transformations;

	scene_lights Scene_light_handler;
	light_indexing_info Current_lights_set;

	void render_arc(const arc_effect &arc);
	void render_insignia(const insignia_draw_data &insignia_info);
	void render_outline(const outline_draw &outline_info);
	void render_buffer(const queued_buffer_draw &render_elements);
	
	SCP_vector<queued_buffer_draw> Render_elements;
	SCP_vector<int> Render_keys;

	SCP_vector<arc_effect> Arcs;
	SCP_vector<insignia_draw_data> Insignias;
	SCP_vector<outline_draw> Outlines;

	graphics::util::UniformBuffer _dataBuffer;

	bool Render_initialized = false; //!< A flag for checking if init_render has been called before a render_all call
	
	static bool sort_draw_pair(const model_draw_list* target, const int a, const int b);
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

	void add_buffer_draw(const model_material *render_material, const indexed_vertex_source *vert_src, const vertex_buffer *buffer, size_t texi, uint tmap_flags);
	
	vec3d get_view_position() const;
	void push_transform(const vec3d* pos, const matrix* orient);
	void pop_transform();
	void set_scale(const vec3d *scale = NULL);

	void add_arc(const vec3d *v1, const vec3d *v2, const SCP_vector<vec3d> *persistent_arc_points, const color *primary, const color *secondary, float arc_width, ubyte segment_depth);
	void render_arcs();

	void add_insignia(const model_render_params *params, const polymodel *pm, int detail_level, int bitmap_num);
	void render_insignias();

	void add_outline(const vertex* vert_array, int n_verts, const color *clr);
	void render_outlines();

	void set_light_filter(const vec3d *pos, float rad);

	void init_render(bool sort = true);
	void render_all(gr_zbuffer_type depth_mode = ZBUFFER_TYPE_DEFAULT);
	void reset();
};

void model_render_only_glowpoint_lights(const model_render_params* interp, int model_num, int model_instance_num, const matrix* orient, const vec3d* pos);
float model_render_get_point_activation(const glow_point_bank* bank, const glow_point_bank_override* gpo);
void model_render_glowpoint_add_light(int point_num, const vec3d* pos, const matrix* orient, const glow_point_bank* bank, const glow_point_bank_override* gpo, const polymodel* pm, const polymodel_instance* pmi, const ship* shipp);
void model_render_immediate(const model_render_params* render_info, int model_num, const matrix* orient, const vec3d* pos, int render = MODEL_RENDER_ALL, bool sort = true);
void model_render_immediate(const model_render_params* render_info, int model_num, int model_instance_num, const matrix* orient, const vec3d* pos, int render = MODEL_RENDER_ALL, bool sort = true);
void model_render_queue(const model_render_params* render_info, model_draw_list* scene, int model_num, const matrix* orient, const vec3d* pos);
void model_render_queue(const model_render_params* render_info, model_draw_list* scene, int model_num, int model_instance_num, const matrix* orient, const vec3d* pos);
void submodel_render_immediate(const model_render_params* render_info, const polymodel* pm, const polymodel_instance* pmi, int submodel_num, const matrix* orient, const vec3d* pos);
void submodel_render_queue(const model_render_params* render_info, model_draw_list* scene, const polymodel* pm, const polymodel_instance* pmi, int submodel_num, const matrix* orient, const vec3d* pos);
void model_render_buffers(model_draw_list* scene, model_material* rendering_material, const model_render_params* interp, const vertex_buffer* buffer, const polymodel* pm, int mn, int detail_level, uint tmap_flags);
bool model_render_check_detail_box(const vec3d* view_pos, const polymodel* pm, int submodel_num, uint64_t flags);
void model_render_arc(const vec3d* v1, const vec3d* v2, const SCP_vector<vec3d> *persistent_arc_points, const color* primary, const color* secondary, float arc_width, ubyte depth_limit);
void model_render_insignias(const insignia_draw_data* insignia);
void model_render_set_wireframe_color(const color* clr);
bool render_tech_model(tech_render_type model_type, int x1, int y1, int x2, int y2, float zoom, bool lighting, int class_idx, const matrix* orient, const SCP_string& pof_filename = "", float closeup_zoom = 0, const vec3d* closeup_pos = &vmd_zero_vector, const SCP_string& tcolor = "");

float convert_distance_and_diameter_to_pixel_size(float distance, float diameter, float field_of_view_deg, int screen_height);

float model_render_get_diameter_clamped_to_min_pixel_size(const vec3d* pos, float diameter, float min_pixel_size);

void model_render_determine_color(color* clr, float alpha, gr_alpha_blend blend_mode, bool no_texturing, bool desaturate);
gr_alpha_blend model_render_determine_blend_mode(int base_bitmap, bool blending);

#endif
