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

#include "model/model.h"
#include "math/vecmat.h"
#include "lighting/lighting.h"
#include "graphics/gropengltnl.h"

extern light Lights[MAX_LIGHTS];
extern int Num_lights;

extern bool Rendering_to_shadow_map;

extern matrix Object_matrix;
extern vec3d Object_position;

extern team_color* Current_team_color;

inline int in_box(vec3d *min, vec3d *max, vec3d *pos, vec3d *view_pos)
{
	vec3d point;

	vm_vec_sub(&point, view_pos, pos);

	if ( (point.xyz.x >= min->xyz.x) && (point.xyz.x <= max->xyz.x)
		&& (point.xyz.y >= min->xyz.y) && (point.xyz.y <= max->xyz.y)
		&& (point.xyz.z >= min->xyz.z) && (point.xyz.z <= max->xyz.z) )
	{
		return 1;
	}

	return -1;
}

inline int in_sphere(vec3d *pos, float radius, vec3d *view_pos)
{
	if ( vm_vec_dist(view_pos, pos) <= radius )
		return 1;
	else
		return -1;
}

extern int model_interp_get_texture(texture_info *tinfo, fix base_frametime);

struct transform
{
	matrix basis;
	vec3d origin;

	transform(): basis(vmd_identity_matrix), origin(vmd_zero_vector) {}
	transform(matrix *m, vec3d *v): basis(*m), origin(*v) {}
};

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

	color Outline_color;

	float Xparent_alpha;

	int Forced_bitmap;

	int Insignia_bitmap;

	int *Replacement_textures;

	bool Team_color_set;
	team_color Current_team_color;

	bool Clip_plane_set;
	vec3d Clip_normal;
	vec3d Clip_pos;

	int Animated_effect;
	float Animated_timer;

	mst_info Thruster_info;
public:
	model_render_params();

	void set_flags(uint flags);
	void set_debug_flags(uint flags);
	void set_object_number(int num);
	void set_detail_level_lock(int detail_level_lock);
	void set_depth_scale(float scale);
	void set_warp_params(int bitmap, float alpha, vec3d &scale);
	void set_outline_color(color &clr);
	void set_outline_color(int r, int g, int b);
	void set_alpha(float alpha);
	void set_forced_bitmap(int bitmap);
	void set_insignia_bitmap(int bitmap);
	void set_replacement_textures(int *textures);
	void set_team_color(team_color &clr);
	void set_team_color(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime);
	void set_clip_plane(vec3d &pos, vec3d &normal);
	void set_animated_effect(int effect_num, float timer);
	void set_thruster_info(mst_info &info);

	bool is_clip_plane_set();
	bool is_team_color_set();

	uint get_model_flags();
	uint get_debug_flags();
	int get_object_number();
	int get_detail_level_lock();
	float get_depth_scale();
	int get_warp_bitmap();
	float get_warp_alpha();
	const vec3d& get_warp_scale();
	const color& get_outline_color();
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
};

struct clip_plane_state
{
	vec3d normal;
	vec3d point;
};

struct arc_effect
{
	transform transformation;
	vec3d v1;
	vec3d v2;
	color primary;
	color secondary;
	float width;
};

struct insignia_draw_data
{
	transform transformation;
	polymodel *pm;
	int detail_level;
	int bitmap_num;

	// if there's a clip plane
	int clip_plane;
};

struct render_state
{
	int clip_plane_handle;
	int texture_addressing;
	int fill_mode;
	int cull_mode;
	int center_alpha;
	int zbias;
	int buffer_id;

	float animated_timer;
	int animated_effect;

	bool lighting;
	light_indexing_info lights;
	float light_factor;
	
	float thrust_scale;

	bool using_team_color;
	team_color tm_color;
	color clr;

	// fog state maybe shouldn't belong here. if we have fog, then it's probably occurring for all objects in scene.
	int fog_mode;
	int r;
	int g;
	int b;
	float fog_near;
	float fog_far;

	render_state()
	{
		clip_plane_handle = -1;
		texture_addressing = TMAP_ADDRESS_WRAP;
		fill_mode = GR_FILL_MODE_SOLID;

		buffer_id = -1;

		lighting = false;

		fog_mode = GR_FOGMODE_NONE;
		r = 0;
		g = 0;
		b = 0;
		fog_near = -1.0f;
		fog_far = -1.0f;

		lights.index_start = 0;
		lights.num_lights = 0;
		light_factor = 1.0f;

		animated_timer = 0.0f;
		animated_effect = 0;

		thrust_scale = -1.0f;

		gr_init_color(&clr, 255, 255, 255);
	}
};

struct queued_buffer_draw
{
	int render_state_handle;
	int texture_maps[TM_NUM_TYPES];
	int transform_buffer_offset;

	color clr;
	int blend_filter;
	float alpha;
	int depth_mode;

	transform transformation;
	vec3d scale;

	vertex_buffer *buffer;
	int texi;
	int flags;
	int sdr_flags;

	float thrust_scale;

	queued_buffer_draw()
	{
		depth_mode = GR_ZBUFF_FULL;

		texture_maps[TM_BASE_TYPE]		= -1;
		texture_maps[TM_GLOW_TYPE]		= -1;
		texture_maps[TM_HEIGHT_TYPE]	= -1;
		texture_maps[TM_MISC_TYPE]		= -1;
		texture_maps[TM_NORMAL_TYPE]	= -1;
		texture_maps[TM_SPECULAR_TYPE]	= -1;
	}
};

struct outline_draw
{
	vertex* vert_array;
	int n_verts;

	transform transformation;
	color clr;
};

class model_batch_buffer
{
	SCP_vector<matrix4> Submodel_matrices;
	void* Mem_alloc;
	uint Mem_alloc_size;

	int Current_offset;

	void allocate_memory();
public:
	model_batch_buffer() : Mem_alloc(NULL), Mem_alloc_size(0), Current_offset(0) {};

	void reset();

	int get_buffer_offset();
	void set_num_models(int n_models);
	void set_model_transform(matrix4 &transform, int model_id);

	void submit_buffer_data();

	void add_matrix(matrix4 &mat);
};

class draw_list
{
	transform Current_transform;
	vec3d Current_scale;
	SCP_vector<transform> Transform_stack;

	render_state Current_render_state;
	bool Dirty_render_state;

	scene_lights Scene_light_handler;
	
	int Current_textures[TM_NUM_TYPES];
	int Current_blend_filter;
	float Current_alpha;
	int Current_depth_mode;

	int Current_set_clip_plane;
	light_indexing_info Current_lights_set;

	void render_arc(arc_effect &arc);
	void render_insignia(insignia_draw_data &insignia_info);
	void render_outline(outline_draw &outline_info);
	void render_buffer(queued_buffer_draw &render_elements);
	uint determine_shader_flags(render_state *state, queued_buffer_draw *draw_info, vertex_buffer *buffer, int tmap_flags);
	
	SCP_vector<clip_plane_state> Clip_planes;
	SCP_vector<render_state> Render_states;
	SCP_vector<queued_buffer_draw> Render_elements;
	SCP_vector<int> Render_keys;

	SCP_vector<arc_effect> Arcs;
	SCP_vector<insignia_draw_data> Insignias;
	SCP_vector<outline_draw> Outlines;

	static draw_list *Target;
	static bool sort_draw_pair(const int a, const int b);
	void sort_draws();
public:
	draw_list();
	void init();

	void reset_state();
	void set_clip_plane(const vec3d &position, const vec3d &normal);
	void set_clip_plane();
	void set_thrust_scale(float scale = -1.0f);
	void set_texture(int texture_type, int texture_handle);
	void set_depth_mode(int depth_set);
	void set_blend_filter(int filter, float alpha);
	void set_texture_addressing(int addressing);
	void set_fog(int fog_mode, int r, int g, int b, float fog_near = -1.0f, float fog_far = -1.0f);
	void set_fill_mode(int mode);
	void set_cull_mode(int mode);
	void set_zbias(int bias);
	void set_center_alpha(int center_alpha);
	void set_lighting(bool mode);
	void set_buffer(int buffer);
	void set_team_color(const team_color &color);
	void set_team_color();
	void set_color(const color &clr);
	void set_animated_timer(float time);
	void set_animated_effect(int effect);
	void add_submodel_to_batch(int model_num);
	void start_model_batch(int n_models);

	void add_buffer_draw(vertex_buffer *buffer, int texi, uint tmap_flags, model_render_params *interp);
	
	vec3d get_view_position();
	void clear_transforms();
	void push_transform(vec3d* pos, matrix* orient);
	void pop_transform();
	void set_scale(vec3d *scale = NULL);

	void add_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width);
	void render_arcs();

	void add_insignia(polymodel *pm, int detail_level, int bitmap_num);
	void render_insignias();

	void add_outline(vertex* vert_array, int n_verts, color *clr);
	void render_outlines();

	void set_light_filter(int objnum, vec3d *pos, float rad);
	void set_light_factor(float factor);

	void init_render();
	void render_all(int depth_mode = -1);
	void reset();
};

class DrawListSorter
{
	static draw_list *Target;
public:
	static void sort(draw_list *target);
	static int sortDrawPair(const void* a, const void* b);
};

//void model_immediate_render(int model_num, matrix *orient, vec3d * pos, uint flags = MR_NORMAL, int objnum = -1, int lighting_skip = -1, int *replacement_textures = NULL);
void model_render_immediate(model_render_params *render_info, int model_num, matrix *orient, vec3d * pos, int render = MODEL_RENDER_ALL);
void model_render_queue(model_render_params *render_info, draw_list* scene, int model_num, matrix *orient, vec3d *pos);
//void model_queue_render(DrawList* scene, int model_num, int model_instance_num, matrix *orient, vec3d *pos, uint flags, int objnum, int *replacement_textures, const bool is_skybox = false);
//void submodel_immediate_render(int model_num, int submodel_num, matrix *orient, vec3d * pos, uint flags = MR_NORMAL, int objnum = -1, int *replacement_textures = NULL);
void submodel_render_immediate(model_render_params *render_info, int model_num, int submodel_num, matrix *orient, vec3d * pos);
void submodel_render_queue(model_render_params *render_info, draw_list *scene, int model_num, int submodel_num, matrix *orient, vec3d * pos);
//void submodel_queue_render(model_render_params *interp, DrawList *scene, int model_num, int submodel_num, matrix *orient, vec3d * pos, uint flags, int objnum = -1);
void model_render_buffers(draw_list* scene, model_render_params* interp, vertex_buffer *buffer, polymodel *pm, int mn, int detail_level, uint tmap_flags);
void model_render_set_thrust(model_render_params *interp, int model_num, mst_info *mst);
void model_render_set_clip_plane(model_render_params *interp, vec3d *pos = NULL, vec3d *normal = NULL);
fix model_render_determine_base_frametime(int objnum, uint flags);
bool model_render_check_detail_box(vec3d *view_pos, polymodel *pm, int submodel_num, uint flags);

#endif
