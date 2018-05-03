/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "graphics/grinternal.h"
#include "osapi/osapi.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "cfile/cfile.h"
#include "math/vecmat.h"
#include "io/cursor.h"

// Forward definition
namespace graphics {
namespace util {
class UniformBuffer;
class GPUMemoryHeap;
}
}

extern const float Default_min_draw_distance;
extern const float Default_max_draw_distance;
extern float Min_draw_distance;
extern float Max_draw_distance;
extern int Gr_inited;

// z-buffering stuff
extern int gr_zbuffering, gr_zbuffering_mode;
extern int gr_global_zbuffering;

class material;
class model_material;
class particle_material;
class distortion_material;
class shield_material;
class movie_material;
class batched_bitmap_material;
class nanovg_material;
class decal_material;

class transform_stack {

	matrix4 Current_transform;
	SCP_vector<matrix4> Stack;
public:
	transform_stack()
	{
		vm_matrix4_set_identity(&Current_transform);

		Stack.clear();
		Stack.push_back(Current_transform);
	}

	matrix4 &get_transform()
	{
		return Current_transform;
	}

	void clear()
	{
		vm_matrix4_set_identity(&Current_transform);

		Stack.clear();
		Stack.push_back(Current_transform);
	}

	void push_and_replace(matrix4 new_transform)
	{
		Current_transform = new_transform;
		Stack.push_back(Current_transform);
	}

	void push(const vec3d *pos, const matrix *orient, const vec3d *scale = NULL)
	{
		vec3d new_scale = SCALE_IDENTITY_VECTOR;
		matrix new_orient = IDENTITY_MATRIX;
		vec3d new_pos = ZERO_VECTOR;

		matrix4 current_transform_copy = Current_transform;
		matrix4 new_transform;

		if ( pos != NULL ) {
			new_pos = *pos;
		}

		if ( orient != NULL ) {
			new_orient = *orient;
		}

		if ( scale != NULL ) {
			new_scale = *scale;
		}

		vm_vec_scale(&new_orient.vec.rvec, new_scale.xyz.x);
		vm_vec_scale(&new_orient.vec.uvec, new_scale.xyz.y);
		vm_vec_scale(&new_orient.vec.fvec, new_scale.xyz.z);

		vm_matrix4_set_transform(&new_transform, &new_orient, &new_pos);

		vm_matrix4_x_matrix4(&Current_transform, &current_transform_copy, &new_transform);
		Stack.push_back(Current_transform);
	}

	void pop()
	{
		if ( Stack.size() > 1 ) {
			Stack.pop_back();
		}

		Current_transform = Stack.back();
	}

	size_t depth() {
		return Stack.size();
	}
};

enum primitive_type {
	PRIM_TYPE_POINTS,
	PRIM_TYPE_LINES,
	PRIM_TYPE_LINESTRIP,
	PRIM_TYPE_TRIS,
	PRIM_TYPE_TRISTRIP,
	PRIM_TYPE_TRIFAN,
	PRIM_TYPE_QUADS,
	PRIM_TYPE_QUADSTRIP
};

enum shader_type {
	SDR_TYPE_NONE = -1,
	SDR_TYPE_MODEL,
	SDR_TYPE_EFFECT_PARTICLE,
	SDR_TYPE_EFFECT_DISTORTION,
	SDR_TYPE_POST_PROCESS_MAIN,
	SDR_TYPE_POST_PROCESS_BLUR,
	SDR_TYPE_POST_PROCESS_BLOOM_COMP,
	SDR_TYPE_POST_PROCESS_BRIGHTPASS,
	SDR_TYPE_POST_PROCESS_FXAA,
	SDR_TYPE_POST_PROCESS_FXAA_PREPASS,
	SDR_TYPE_POST_PROCESS_LIGHTSHAFTS,
	SDR_TYPE_POST_PROCESS_TONEMAPPING,
	SDR_TYPE_DEFERRED_LIGHTING,
	SDR_TYPE_DEFERRED_CLEAR,
	SDR_TYPE_VIDEO_PROCESS,
	SDR_TYPE_PASSTHROUGH_RENDER, //!< Shader for doing the old style fixed-function rendering. Only used internally, use SDR_TYPE_DEFAULT_MATERIAL.
	SDR_TYPE_SHIELD_DECAL,
	SDR_TYPE_BATCHED_BITMAP,
	SDR_TYPE_DEFAULT_MATERIAL,
	SDR_TYPE_NANOVG,
	SDR_TYPE_DECAL,
	SDR_TYPE_SCENE_FOG,
	NUM_SHADER_TYPES
};

// Shader flags
#define SDR_FLAG_MODEL_LIGHT		(1<<0)
#define SDR_FLAG_MODEL_FOG			(1<<1)
#define SDR_FLAG_MODEL_DIFFUSE_MAP	(1<<2)
#define SDR_FLAG_MODEL_GLOW_MAP		(1<<3)
#define SDR_FLAG_MODEL_SPEC_MAP		(1<<4)
#define SDR_FLAG_MODEL_NORMAL_MAP	(1<<5)
#define SDR_FLAG_MODEL_HEIGHT_MAP	(1<<6)
#define SDR_FLAG_MODEL_ENV_MAP		(1<<7)
#define SDR_FLAG_MODEL_ANIMATED		(1<<8)
#define SDR_FLAG_MODEL_MISC_MAP		(1<<9)
#define SDR_FLAG_MODEL_TEAMCOLOR	(1<<10)
#define SDR_FLAG_MODEL_TRANSFORM	(1<<11)
#define SDR_FLAG_MODEL_DEFERRED		(1<<12)
#define SDR_FLAG_MODEL_SHADOW_MAP	(1<<13)
#define SDR_FLAG_MODEL_GEOMETRY		(1<<14)
#define SDR_FLAG_MODEL_SHADOWS		(1<<15)
#define SDR_FLAG_MODEL_THRUSTER		(1<<16)
#define SDR_FLAG_MODEL_CLIP			(1<<17)
#define SDR_FLAG_MODEL_HDR			(1<<18)
#define SDR_FLAG_MODEL_AMBIENT_MAP	(1<<19)
#define SDR_FLAG_MODEL_NORMAL_ALPHA	(1<<20)
#define SDR_FLAG_MODEL_NORMAL_EXTRUDE (1<<21)

#define SDR_FLAG_PARTICLE_POINT_GEN			(1<<0)

#define SDR_FLAG_BLUR_HORIZONTAL			(1<<0)
#define SDR_FLAG_BLUR_VERTICAL				(1<<1)

#define SDR_FLAG_NANOVG_EDGE_AA		(1<<0)

#define SDR_FLAG_DECAL_USE_NORMAL_MAP (1<<0)

enum class uniform_block_type {
	Lights = 0,
	ModelData = 1,
	NanoVGData = 2,
	DecalInfo = 3,
	DecalGlobals = 4,
	DeferredGlobals = 5,

	NUM_BLOCK_TYPES
};

struct vertex_format_data
{
	enum vertex_format {
		POSITION4,
		POSITION3,
		POSITION2,
		SCREEN_POS,
		COLOR3,
		COLOR4,
		COLOR4F,
		TEX_COORD2,
		TEX_COORD3,
		NORMAL,
		TANGENT,
		MODEL_ID,
		RADIUS,
		UVEC,
		WORLD_MATRIX,
	};

	vertex_format format_type;
	size_t stride;
	size_t offset;

	vertex_format_data(vertex_format i_format_type, size_t i_stride, size_t i_offset) :
	format_type(i_format_type), stride(i_stride), offset(i_offset) {}

	static inline uint mask(vertex_format v_format) { return 1 << v_format; }

	bool operator==(const vertex_format_data& other) const {
		return format_type == other.format_type && stride == other.stride && offset == other.offset;
	}
};
class vertex_layout
{
	SCP_vector<vertex_format_data> Vertex_components;

	uint Vertex_mask = 0;
	size_t Vertex_stride = 0;
public:
	vertex_layout() {}

	size_t get_num_vertex_components() const { return Vertex_components.size(); }

	const vertex_format_data* get_vertex_component(size_t index) const { return &Vertex_components[index]; }
	
	bool resident_vertex_format(vertex_format_data::vertex_format format_type) const;

	void add_vertex_component(vertex_format_data::vertex_format format_type, size_t stride, size_t offset);

	size_t get_vertex_stride() { return Vertex_stride; }

	bool operator==(const vertex_layout& other) const;

	size_t hash() const;
};
namespace std {
template<> struct hash<vertex_format_data> {
	size_t operator()(const vertex_format_data& data) const;
};
template<> struct hash<vertex_layout> {
	size_t operator()(const vertex_layout& data) const;
};
}

typedef enum gr_capability {
	CAPABILITY_ENVIRONMENT_MAP,
	CAPABILITY_NORMAL_MAP,
	CAPABILITY_HEIGHT_MAP,
	CAPABILITY_SOFT_PARTICLES,
	CAPABILITY_DISTORTION,
	CAPABILITY_POST_PROCESSING,
	CAPABILITY_DEFERRED_LIGHTING,
	CAPABILITY_SHADOWS,
	CAPABILITY_BATCHED_SUBMODELS,
	CAPABILITY_POINT_PARTICLES,
	CAPABILITY_TIMESTAMP_QUERY,
	CAPABILITY_SEPARATE_BLEND_FUNCTIONS,
} gr_capability;

enum class gr_property {
	UNIFORM_BUFFER_OFFSET_ALIGNMENT,
	UNIFORM_BUFFER_MAX_SIZE,
};

// stencil buffering stuff
extern int gr_stencil_mode;

/**
 * This is a structure used by the shader to keep track
 * of the values you want to use in the shade primitive.
 */
typedef struct shader {
	uint	screen_sig;					// current mode this is in
	ubyte	r,g,b,c;						// factors and constant
	ubyte	lookup[256];
} shader;

#define AC_TYPE_NONE		0		// Not an alphacolor
#define AC_TYPE_HUD		1		// Doesn't change hue depending on background.  Used for HUD stuff.
#define AC_TYPE_BLEND	2		// Changes hue depending on background.  Used for stars, etc.

// NEVER REFERENCE THESE VALUES OUTSIDE OF THE GRAPHICS LIBRARY!!!
// If you need to get the rgb values of a "color" struct call
// gr_get_colors after calling gr_set_colors_fast.
typedef struct color {
	uint		screen_sig;
	int		is_alphacolor;
	int		alphacolor;
	int		magic;
	ubyte		red;
	ubyte		green;
	ubyte		blue;
	ubyte		alpha;
	ubyte		ac_type;							// The type of alphacolor.  See AC_TYPE_??? defines
	ubyte		raw8;
} color;

// Used by the team coloring code
typedef struct team_color {
	struct {
		float r, g, b;
	} base;
	struct {
		float r, g, b;
	} stripe;
} team_color;



typedef struct tsb_t {
	vec3d tangent;
	float scaler;
} tsb_t;

/**
 * This should be basicly just like it is in the VB
 * a list of triangles and their associated normals
 */
class poly_list {
	// helper function struct that let's us sort the indices.
	// an instance is fed into std::sort and std::lower_bound.
	// overloaded operator() is used for the comparison function.
	struct finder {
		poly_list* search_list;
		bool compare_indices;
		vertex* vert_to_find;
		vec3d* norm_to_find;

		finder(poly_list* _search_list): search_list(_search_list), compare_indices(true), vert_to_find(NULL), norm_to_find(NULL) {}
		finder(poly_list* _search_list, vertex* _vert, vec3d* _norm): search_list(_search_list), compare_indices(false), vert_to_find(_vert), norm_to_find(_norm) {}

		bool operator()(const uint a, const uint b);
	};
public:
	poly_list(): n_verts(0), vert(NULL), norm(NULL), tsb(NULL), submodels(NULL), sorted_indices(NULL), currently_allocated(0) {}
	~poly_list();
	poly_list& operator=(const poly_list&);

	void allocate(int size);
	void make_index_buffer(SCP_vector<int> &vertex_list);
	void calculate_tangent();
	int n_verts;
	vertex *vert;
	vec3d *norm;
	tsb_t *tsb;
	int *submodels;

	uint *sorted_indices;

	int find_index(poly_list *plist, int idx);
	int find_index_fast(poly_list *plist, int idx);
private:
	int currently_allocated;
	int find_first_vertex(int idx);
	int find_first_vertex_fast(int idx);
	void generate_sorted_index_list();
};

class buffer_data
{
public:
	int flags;

	int texture;		// this is the texture the vertex buffer will use
	size_t n_verts;

	size_t index_offset;

	const uint *get_index() const
	{
		return index;
	}

        uint i_first, i_last;

	void release()
	{
		if (index) {
			delete [] index;
			index = NULL;
		}
	}

	void assign(size_t i, uint j)
	{
		const_cast<uint *>(index)[i] = j;
		if (i_first > i_last)
			i_first = i_last = j;
		else if (i_first > j)
			i_first = j;
		else if (i_last < j)
			i_last = j;
	}

	// Constructor

	buffer_data() :
	flags(0), texture(-1), n_verts(0), index_offset(0),
		i_first(1), i_last(0), index(NULL)
	{
	}

	explicit buffer_data(size_t n_vrts) :
		flags(0), texture(-1), n_verts(n_vrts), index_offset(0),
		i_first(1), i_last(0), index(NULL)
	{
		if ( n_verts > 0 ) {
			index = new(std::nothrow) uint[n_verts];
		} else {
			index = NULL;
		}
	}

	// Copy-constructor
	buffer_data(const buffer_data& other)
	{
		if ( other.index ) {
			index = new(std::nothrow) uint[other.n_verts];
			for (size_t i=0; i < other.n_verts; i++)
			{
				index[i] = other.index[i];
			}
		} else {
			index = NULL;
		}

		flags   = other.flags;
		texture = other.texture;
		n_verts = other.n_verts;

		i_first = other.i_first;
		i_last  = other.i_last;

		index_offset = other.index_offset;
	}

	// Copy-assignment operator
	buffer_data& operator=(const buffer_data& rhs)
	{
		if (this != &rhs)
		{
			if ( index ) {
				delete [] index;
			}

			if ( rhs.index && rhs.n_verts > 0 ) {
				index = new(std::nothrow) uint[rhs.n_verts];
				for (size_t i=0; i < rhs.n_verts; i++)
				{
					index[i] = rhs.index[i];
				}
			}

			flags   = rhs.flags;
			texture = rhs.texture;
			n_verts = rhs.n_verts;

			i_first = rhs.i_first;
			i_last  = rhs.i_last;

			index_offset = rhs.index_offset;
		}
		return *this;
	}

	// Destructor
	~buffer_data()
	{
		release();
	}

private:
	uint *index;
};

class vertex_buffer
{
public:
	int flags;

	size_t stride;
	size_t vertex_offset;
	size_t vertex_num_offset;

	poly_list *model_list;

	SCP_vector<buffer_data> tex_buf;

	vertex_layout layout;

	vertex_buffer() :
		flags(0), stride(0), vertex_offset(0), vertex_num_offset(0), model_list(NULL)
	{
	}

	~vertex_buffer()
	{
		clear();
	}

	void release()
	{
		if (model_list) {
			delete model_list;
			model_list = NULL;
		}

		for (SCP_vector<buffer_data>::iterator tbi = tex_buf.begin(); tbi != tex_buf.end(); ++tbi) {
			tbi->release();
		}
	}

	void clear()
	{
		release();
		tex_buf.clear();
	}
};

struct indexed_vertex_source {
	void *Vertex_list = nullptr;
	void *Index_list = nullptr;

	int Vbuffer_handle = -1;
	size_t Vertex_offset = 0;
	size_t Base_vertex_offset = 0;

	int Ibuffer_handle = -1;
	size_t Index_offset = 0;

	uint Vertex_list_size = 0;
	uint Index_list_size = 0;
};

struct light;

#define FIND_SCALED_NUM(x, x0, x1, y0, y1) ( ((((x) - (x0)) * ((y1) - (y0))) / ((x1) - (x0))) + (y0) )

#define GR_ALPHABLEND_NONE			0		// no blending
#define GR_ALPHABLEND_FILTER		1		// 50/50 mix of foreground, background, using intensity as alpha

#define GR_BITBLT_MODE_NORMAL		0		// Normal bitblting
#define GR_BITBLT_MODE_RLE			1		// RLE would be faster

// fog modes
#define GR_FOGMODE_NONE				0		// set this to turn off fog
#define GR_FOGMODE_FOG				1		// linear fog

enum class QueryType {
	Timestamp
};

enum class BufferType {
	Vertex,
	Index,
	Uniform
};

enum class BufferUsageHint {
	Static,
	Dynamic,
	Streaming
};

/**
 * @brief Type of a graphics sync object
 */
typedef void* gr_sync;

typedef struct screen {
	uint	signature;			// changes when mode or palette or width or height changes
	int	max_w, max_h;		// Width and height
	int max_w_unscaled, max_h_unscaled;
	int max_w_unscaled_zoomed, max_h_unscaled_zoomed;
	int center_w, center_h;	// Width and height of center monitor
	int center_offset_x, center_offset_y;
	int	save_max_w, save_max_h;		// Width and height
	int save_max_w_unscaled, save_max_h_unscaled;
	int save_max_w_unscaled_zoomed, save_max_h_unscaled_zoomed;
	int save_center_w, save_center_h;	// Width and height of center monitor
	int save_center_offset_x, save_center_offset_y;
	int	res;					// GR_640 or GR_1024
	int	mode;					// What mode gr_init was called with.
	float	aspect, clip_aspect;				// Aspect ratio, aspect of clip_width/clip_height
	int	rowsize;				// What you need to add to go to next row (includes bytes_per_pixel)
	int	bits_per_pixel;	// How many bits per pixel it is. (7,8,15,16,24,32)
	int	bytes_per_pixel;	// How many bytes per pixel (1,2,3,4)
	int	offset_x, offset_y;		// The offsets into the screen
	int offset_x_unscaled, offset_y_unscaled;	// Offsets into the screen, in unscaled dimensions
	int	clip_width, clip_height;
	int clip_width_unscaled, clip_height_unscaled;	// Height and width of clip aread, in unscaled dimensions
	// center of clip area
	float	clip_center_x, clip_center_y;

	float fog_near, fog_far;

	// the clip_l,r,t,b are used internally.  left and top are
	// actually always 0, but it's nice to have the code work with
	// arbitrary clipping regions.
	int		clip_left, clip_right, clip_top, clip_bottom;
	// same as above except in unscaled dimensions
	int		clip_left_unscaled, clip_right_unscaled, clip_top_unscaled, clip_bottom_unscaled;

	int		current_alphablend_mode;		// See GR_ALPHABLEND defines above
	int		current_bitblt_mode;				// See GR_BITBLT_MODE defines above
	int		current_bitmap;
	color		current_color;
	color		current_fog_color;				// current fog color
	color		current_clear_color;				// current clear color
	shader	current_shader;
	float		current_alpha;

	bool custom_size;
	int		rendering_to_texture;		//wich texture we are rendering to, -1 if the back buffer
	int		rendering_to_face;			//wich face of the texture we are rendering to, -1 if the back buffer

	int envmap_render_target;

	float line_width;

	//switch onscreen, offscreen
	void (*gf_flip)();

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h, int resize_mode);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	// clears entire clipping region to current color
	void (*gf_clear)();

	// dumps the current screen to a file
	void (*gf_print_screen)(const char * filename);

	// Retrieves the zbuffer mode.
	int (*gf_zbuffer_get)();

	// Sets mode.  Returns previous mode.
	int (*gf_zbuffer_set)(int mode);

	// Clears the zbuffer.  If use_zbuffer is FALSE, then zbuffering mode is ignored and zbuffer is always off.
	void (*gf_zbuffer_clear)(int use_zbuffer);

	// Set the stencil buffer mode. Returns previous mode
	int (*gf_stencil_set)(int mode);

	// Clears the stencil buffer.
	void (*gf_stencil_clear)();

	int (*gf_alpha_mask_set)(int mode, float alpha);

	// Saves screen. Returns an id you pass to restore and free.
	int (*gf_save_screen)();

	// Resets clip region and copies entire saved screen to the screen.
	void (*gf_restore_screen)(int id);

	// Frees up a saved screen.
	void (*gf_free_screen)(int id);

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// grab a region of the screen. assumes data is large enough
	void (*gf_get_region)(int front, int w, int h, ubyte *data);

	// poly culling
	int (*gf_set_cull)(int cull);

	// color buffer writes
	int (*gf_set_color_buffer)(int mode);

	// preload a bitmap into texture memory
	int (*gf_preload)(int bitmap_num, int is_aabitmap);

	// set the color to be used when clearing the background
	void (*gf_set_clear_color)(int r, int g, int b);

	// Here be the bitmap functions
	void (*gf_bm_free_data)(int n, bool release);
	void (*gf_bm_create)(int n);
	void (*gf_bm_init)(int n);
	void (*gf_bm_page_in_start)();
	bool (*gf_bm_data)(int n, bitmap* bm);

	int (*gf_bm_make_render_target)(int n, int *width, int *height, int *bpp, int *mm_lvl, int flags );
	int (*gf_bm_set_render_target)(int n, int face);

	void (*gf_translate_texture_matrix)(int unit, const vec3d *shift);
	void (*gf_push_texture_matrix)(int unit);
	void (*gf_pop_texture_matrix)(int unit);

	void (*gf_set_texture_addressing)(int);

	int (*gf_create_buffer)(BufferType type, BufferUsageHint usage);
	void (*gf_delete_buffer)(int handle);

	void (*gf_update_buffer_data)(int handle, size_t size, void* data);
	void (*gf_update_buffer_data_offset)(int handle, size_t offset, size_t size, void* data);
	void (*gf_update_transform_buffer)(void* data, size_t size);

	// postprocessing effects
	void (*gf_post_process_set_effect)(const char*, int, const vec3d*);
	void (*gf_post_process_set_defaults)();

	void (*gf_post_process_begin)();
	void (*gf_post_process_end)();
	void (*gf_post_process_save_zbuffer)();
	void (*gf_post_process_restore_zbuffer)();

	void (*gf_deferred_lighting_begin)();
	void (*gf_deferred_lighting_end)();
	void (*gf_deferred_lighting_finish)();

	void (*gf_scene_texture_begin)();
	void (*gf_scene_texture_end)();
	void (*gf_copy_effect_texture)();

	void (*gf_zbias)(int zbias);

	void (*gf_set_fill_mode)(int);

	void (*gf_set_line_width)(float width);

	void (*gf_sphere)(material *material_def, float rad);

	int  (*gf_maybe_create_shader)(shader_type type, unsigned int flags);
	void (*gf_recompile_all_shaders)(const std::function<void(size_t, size_t)>& progress_callback);

	void (*gf_clear_states)();

	void (*gf_update_texture)(int bitmap_handle, int bpp, const ubyte* data, int width, int height);
	void (*gf_get_bitmap_from_texture)(void* data_out, int bitmap_num);

	void (*gf_shadow_map_start)(matrix4 *shadow_view_matrix, const matrix *light_matrix);
	void (*gf_shadow_map_end)();

	void (*gf_start_decal_pass)();
	void (*gf_stop_decal_pass)();

	// new drawing functions
	void (*gf_render_model)(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi);
	void (*gf_render_shield_impact)(shield_material *material_info, primitive_type prim_type, vertex_layout *layout, int buffer_handle, int n_verts);
	void (*gf_render_primitives)(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle, size_t buffer_offset);
	void (*gf_render_primitives_particle)(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_primitives_distortion)(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_movie)(movie_material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer);
	void (*gf_render_primitives_batched)(batched_bitmap_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_nanovg)(nanovg_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_decals)(decal_material* material_info, primitive_type prim_type, vertex_layout* layout, int num_elements, const indexed_vertex_source& buffers);

	bool (*gf_is_capable)(gr_capability capability);
	bool (*gf_get_property)(gr_property property, void* destination);

	void (*gf_push_debug_group)(const char* name);
	void (*gf_pop_debug_group)();

	int (*gf_create_query_object)();
	void (*gf_query_value)(int obj, QueryType type);
	bool (*gf_query_value_available)(int obj);
	std::uint64_t (*gf_get_query_value)(int obj);
	void (*gf_delete_query_object)(int obj);

	std::unique_ptr<os::Viewport> (*gf_create_viewport)(const os::ViewPortProperties& props);
	void (*gf_use_viewport)(os::Viewport* view);

	void (*gf_bind_uniform_buffer)(uniform_block_type bind_point, size_t offset, size_t size, int buffer);

	gr_sync (*gf_sync_fence)();
	bool (*gf_sync_wait)(gr_sync sync, uint64_t timeoutns);
	void (*gf_sync_delete)(gr_sync sync);

	void (*gf_set_viewport)(int x, int y, int width, int height);
} screen;

// handy macro
#define GR_MAYBE_CLEAR_RES(bmap)		do  { int bmw = -1; int bmh = -1; if(bmap != -1){ bm_get_info( bmap, &bmw, &bmh, NULL, NULL, NULL); if((bmw != gr_screen.max_w) || (bmh != gr_screen.max_h)){gr_clear();} } else {gr_clear();} } while(0);

//Window's interface to set up graphics:
//--------------------------------------
// Call this at application startup

// # Software Re-added by Kazan --- THIS HAS TO STAY -- It is used by standalone!
#define GR_DEFAULT				(-1)		// set to use default settings
#define GR_STUB					(100)
#define GR_OPENGL				(104)		// Use OpenGl hardware renderer

// resolution constants   - always keep resolutions in ascending order and starting from 0  
#define GR_NUM_RESOLUTIONS			2
#define GR_640							0		// 640 x 480
#define GR_1024						1		// 1024 x 768

#define GR_1024_THRESHOLD_WIDTH		1024
#define GR_1024_THRESHOLD_HEIGHT	600

extern const char *Resolution_prefixes[GR_NUM_RESOLUTIONS];

extern bool gr_init(std::unique_ptr<os::GraphicsOperations>&& graphicsOps, int d_mode = GR_DEFAULT,
					int d_width = GR_DEFAULT, int d_height = GR_DEFAULT, int d_depth = GR_DEFAULT);

extern void gr_screen_resize(int width, int height);
extern int gr_get_resolution_class(int width, int height);

// Call this when your app ends.
extern void gr_close();

extern screen gr_screen;

#define GR_FILL_MODE_WIRE 1
#define GR_FILL_MODE_SOLID 2

#define GR_ZBUFF_NONE	0
#define GR_ZBUFF_WRITE	(1<<0)
#define GR_ZBUFF_READ	(1<<1)
#define GR_ZBUFF_FULL	(GR_ZBUFF_WRITE|GR_ZBUFF_READ)

#define GR_STENCIL_NONE		0
#define GR_STENCIL_READ		1
#define GR_STENCIL_WRITE	2

#define GR_RESIZE_NONE				0
#define GR_RESIZE_FULL				1
#define GR_RESIZE_FULL_CENTER		2
#define GR_RESIZE_MENU				3
#define GR_RESIZE_MENU_ZOOMED		4
#define GR_RESIZE_MENU_NO_OFFSET	5

void gr_set_screen_scale(int x, int y, int zoom_x = -1, int zoom_y = -1, int max_x = gr_screen.max_w, int max_y = gr_screen.max_h, int center_x = gr_screen.center_w, int center_y = gr_screen.center_h, bool force_stretch = false);
void gr_reset_screen_scale();
bool gr_unsize_screen_pos(int *x, int *y, int *w = NULL, int *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_resize_screen_pos(int *x, int *y, int *w = NULL, int *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_unsize_screen_posf(float *x, float *y, float *w = NULL, float *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_resize_screen_posf(float *x, float *y, float *w = NULL, float *h = NULL, int resize_mode = GR_RESIZE_FULL);

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void gr_printf( int x, int y, const char * format, SCP_FORMAT_STRING ... ) SCP_FORMAT_STRING_ARGS(3, 4);
// same as gr_printf but positions text correctly in menus
extern void gr_printf_menu( int x, int y, const char * format, SCP_FORMAT_STRING ... )  SCP_FORMAT_STRING_ARGS(3, 4);
// same as gr_printf_menu but accounts for menu zooming
extern void gr_printf_menu_zoomed( int x, int y, const char * format, SCP_FORMAT_STRING ... )  SCP_FORMAT_STRING_ARGS(3, 4);
// same as gr_printf but doesn't resize for non-standard resolutions
extern void gr_printf_no_resize( int x, int y, const char * format, SCP_FORMAT_STRING ... )  SCP_FORMAT_STRING_ARGS(3, 4);

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size( int *w, int *h, const char * text, int len = 9999 );

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(const char *name, ubyte *palette, int restrict_to_128 = 0);

extern io::mouse::Cursor* Web_cursor;

// Called by OS when application gets/looses focus
extern void gr_activate(int active);

#define GR_CALL(x)			(*x)

// These macros make the function indirection look like the
// old Descent-style gr_xxx calls.

#define gr_print_screen		GR_CALL(gr_screen.gf_print_screen)

//#define gr_flip				GR_CALL(gr_screen.gf_flip)
void gr_flip(bool execute_scripting = true);

//#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
__inline void gr_set_clip(int x, int y, int w, int h, int resize_mode=GR_RESIZE_FULL)
{
	(*gr_screen.gf_set_clip)(x,y,w,h,resize_mode);
}
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)

void gr_set_bitmap(int bitmap_num, int alphablend = GR_ALPHABLEND_NONE, int bitbltmode = GR_BITBLT_MODE_NORMAL, float alpha = 1.0f);

#define gr_clear				GR_CALL(gr_screen.gf_clear)

void gr_shield_icon(coord2d coords[6], const int resize_mode = GR_RESIZE_FULL);

#define gr_zbuffer_get		GR_CALL(gr_screen.gf_zbuffer_get)
#define gr_zbuffer_set		GR_CALL(gr_screen.gf_zbuffer_set)
#define gr_zbuffer_clear	GR_CALL(gr_screen.gf_zbuffer_clear)

#define gr_stencil_set		GR_CALL(gr_screen.gf_stencil_set)
#define gr_stencil_clear	GR_CALL(gr_screen.gf_stencil_clear)

#define gr_alpha_mask_set	GR_CALL(gr_screen.gf_alpha_mask_set)

#define gr_save_screen		GR_CALL(gr_screen.gf_save_screen)
#define gr_restore_screen	GR_CALL(gr_screen.gf_restore_screen)
#define gr_free_screen		GR_CALL(gr_screen.gf_free_screen)

#define gr_set_gamma			GR_CALL(gr_screen.gf_set_gamma)

#define gr_get_region		GR_CALL(gr_screen.gf_get_region)

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)
#define gr_set_color_buffer	GR_CALL(gr_screen.gf_set_color_buffer)


#define gr_preload			GR_CALL(gr_screen.gf_preload)

#define gr_set_clear_color	GR_CALL(gr_screen.gf_set_clear_color)

#define gr_translate_texture_matrix		GR_CALL(gr_screen.gf_translate_texture_matrix)
#define gr_push_texture_matrix			GR_CALL(gr_screen.gf_push_texture_matrix)
#define gr_pop_texture_matrix			GR_CALL(gr_screen.gf_pop_texture_matrix)


// Here be the bitmap functions
#define gr_bm_free_data				GR_CALL(*gr_screen.gf_bm_free_data)
#define gr_bm_create				GR_CALL(*gr_screen.gf_bm_create)
#define gr_bm_init					GR_CALL(*gr_screen.gf_bm_init)
#define gr_bm_page_in_start			GR_CALL(*gr_screen.gf_bm_page_in_start)
#define gr_bm_data					GR_CALL(*gr_screen.gf_bm_data)

#define gr_bm_make_render_target					GR_CALL(*gr_screen.gf_bm_make_render_target)

__inline int gr_bm_set_render_target(int n, int face = -1)
{
	return (*gr_screen.gf_bm_set_render_target)(n, face);
}

#define gr_set_texture_addressing					 GR_CALL(*gr_screen.gf_set_texture_addressing)

inline int gr_create_buffer(BufferType type, BufferUsageHint usage)
{
	return (*gr_screen.gf_create_buffer)(type, usage);
}

#define gr_delete_buffer				GR_CALL(*gr_screen.gf_delete_buffer)
#define gr_update_buffer_data			GR_CALL(*gr_screen.gf_update_buffer_data)
#define gr_update_buffer_data_offset	GR_CALL(*gr_screen.gf_update_buffer_data_offset)
#define gr_update_transform_buffer		GR_CALL(*gr_screen.gf_update_transform_buffer)

#define gr_scene_texture_begin			GR_CALL(*gr_screen.gf_scene_texture_begin)
#define gr_scene_texture_end			GR_CALL(*gr_screen.gf_scene_texture_end)
#define gr_copy_effect_texture			GR_CALL(*gr_screen.gf_copy_effect_texture)

#define gr_post_process_set_effect		GR_CALL(*gr_screen.gf_post_process_set_effect)
#define gr_post_process_set_defaults	GR_CALL(*gr_screen.gf_post_process_set_defaults)
#define gr_post_process_begin			GR_CALL(*gr_screen.gf_post_process_begin)
#define gr_post_process_end				GR_CALL(*gr_screen.gf_post_process_end)
#define gr_post_process_save_zbuffer	GR_CALL(*gr_screen.gf_post_process_save_zbuffer)
inline void gr_post_process_restore_zbuffer() {
	gr_screen.gf_post_process_restore_zbuffer();
}

#define gr_deferred_lighting_begin		GR_CALL(*gr_screen.gf_deferred_lighting_begin)
#define gr_deferred_lighting_end		GR_CALL(*gr_screen.gf_deferred_lighting_end)
#define gr_deferred_lighting_finish		GR_CALL(*gr_screen.gf_deferred_lighting_finish)

#define	gr_zbias						GR_CALL(*gr_screen.gf_zbias)
#define	gr_set_fill_mode				GR_CALL(*gr_screen.gf_set_fill_mode)

#define gr_set_line_width				GR_CALL(*gr_screen.gf_set_line_width)

#define gr_sphere						GR_CALL(*gr_screen.gf_sphere)

#define gr_maybe_create_shader			GR_CALL(*gr_screen.gf_maybe_create_shader)
#define gr_recompile_all_shaders		GR_CALL(*gr_screen.gf_recompile_all_shaders)
#define gr_set_animated_effect			GR_CALL(*gr_screen.gf_set_animated_effect)

#define gr_clear_states					GR_CALL(*gr_screen.gf_clear_states)

#define gr_update_texture				GR_CALL(*gr_screen.gf_update_texture)
#define gr_get_bitmap_from_texture		GR_CALL(*gr_screen.gf_get_bitmap_from_texture)

#define gr_shadow_map_start				GR_CALL(*gr_screen.gf_shadow_map_start)
#define gr_shadow_map_end				GR_CALL(*gr_screen.gf_shadow_map_end)
#define gr_render_shield_impact			GR_CALL(*gr_screen.gf_render_shield_impact)

__inline void gr_render_primitives(material* material_info, primitive_type prim_type, vertex_layout* layout, int vert_offset, int n_verts, int buffer_handle = -1, size_t buffer_offset = 0)
{
	(*gr_screen.gf_render_primitives)(material_info, prim_type, layout, vert_offset, n_verts, buffer_handle, buffer_offset);
}

__inline void gr_render_primitives_particle(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_particle)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_batched(batched_bitmap_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_batched)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_nanovg(nanovg_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle)
{
	(*gr_screen.gf_render_nanovg)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_distortion(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_distortion)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

inline void gr_render_movie(movie_material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer)
{
	(*gr_screen.gf_render_movie)(material_info, prim_type, layout, n_verts, buffer);
}

__inline void gr_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi)
{
	(*gr_screen.gf_render_model)(material_info, vert_source, bufferp, texi);
}

__inline bool gr_is_capable(gr_capability capability)
{
	return (*gr_screen.gf_is_capable)(capability);
}

inline bool gr_get_property(gr_property property, void* destination)
{
	return (*gr_screen.gf_get_property)(property, destination);
}

inline void gr_push_debug_group(const char* name)
{
	(*gr_screen.gf_push_debug_group)(name);
}

inline void gr_pop_debug_group()
{
	(*gr_screen.gf_pop_debug_group)();
}

inline int gr_create_query_object()
{
	return (*gr_screen.gf_create_query_object)();
}

inline void gr_query_value(int obj, QueryType type)
{
	(*gr_screen.gf_query_value)(obj, type);
}

inline bool gr_query_value_available(int obj)
{
	return (*gr_screen.gf_query_value_available)(obj);
}

inline std::uint64_t gr_get_query_value(int obj)
{
	return (*gr_screen.gf_get_query_value)(obj);
}

inline void gr_delete_query_object(int obj)
{
	(*gr_screen.gf_delete_query_object)(obj);
}

inline std::unique_ptr<os::Viewport> gr_create_viewport(const os::ViewPortProperties& props) {
	return (*gr_screen.gf_create_viewport)(props);
}
inline void gr_use_viewport(os::Viewport* view) {
	(*gr_screen.gf_use_viewport)(view);
}
inline void gr_set_viewport(int x, int y, int width, int height) {
	(*gr_screen.gf_set_viewport)(x, y, width, height);
}

inline void gr_bind_uniform_buffer(uniform_block_type bind_point, size_t offset, size_t size, int buffer) {
	(*gr_screen.gf_bind_uniform_buffer)(bind_point, offset, size, buffer);
}

inline gr_sync gr_sync_fence() {
	return (*gr_screen.gf_sync_fence)();
}
inline bool gr_sync_wait(gr_sync sync, uint64_t timeoutns) {
	return (*gr_screen.gf_sync_wait)(sync, timeoutns);
}
inline void gr_sync_delete(gr_sync sync) {
	(*gr_screen.gf_sync_delete)(sync);
}

// color functions
void gr_get_color( int *r, int *g, int  b );
void gr_init_color(color *c, int r, int g, int b);
void gr_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type = AC_TYPE_HUD );
void gr_set_color( int r, int g, int b );
void gr_set_color_fast(color *dst);

// shader functions
void gr_create_shader(shader *shade, ubyte r, ubyte g, ubyte b, ubyte c);
void gr_set_shader(shader *shade);

uint gr_determine_model_shader_flags(
	bool lighting,
	bool fog,
	bool textured,
	bool in_shadow_map,
	bool thruster_scale,
	bool transform,
	bool team_color_set,
	int tmap_flags,
	int spec_map,
	int glow_map,
	int normal_map,
	int height_map,
	int ambient_map,
	int env_map,
	int misc_map
);

// new bitmap functions
void gr_bitmap(int x, int y, int resize_mode = GR_RESIZE_FULL);
void gr_bitmap_uv(int _x, int _y, int _w, int _h, float _u0, float _v0, float _u1, float _v1, int resize_mode = GR_RESIZE_FULL);
void gr_bitmap_list(bitmap_2d_list* list, int n_bm, int resize_mode);
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, int resize_mode);

// texture update functions
ubyte* gr_opengl_get_texture_update_pointer(int bitmap_handle);
void gr_opengl_update_texture(int bitmap_handle, int bpp, const ubyte* data, int width, int height);

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(SCP_vector<vec3d> *pts, int thickness,int resize_mode=GR_RESIZE_FULL);

#define VB_FLAG_POSITION	(1<<0)
#define VB_FLAG_RHW			(1<<1)	//incompatable with the next normal
#define VB_FLAG_NORMAL		(1<<2)
#define VB_FLAG_DIFUSE		(1<<3)
#define VB_FLAG_SPECULAR	(1<<4)
#define VB_FLAG_UV1			(1<<5)	//how many UV coords, only use one of these
#define VB_FLAG_UV2			(1<<6)
#define VB_FLAG_UV3			(1<<7)
#define VB_FLAG_UV4			(1<<8)
#define VB_FLAG_TANGENT		(1<<9)
#define VB_FLAG_LARGE_INDEX	(1<<10)
#define VB_FLAG_MODEL_ID	(1<<11)
#define VB_FLAG_TRANS		(1<<12)

/**
* @brief Prints the current time
*
* Draws the timestamp of the current #Missiontime in the format @c h:mm:ss at the specified coordinates
*
* @param x The x position where the timestamp should be draw
* @param y The y position where the timestamp should be draw
* @param timestamp The timestamp (in 65536ths of a second) to be printed
* @param resize_mode The resize mode to use
*/
void gr_print_timestamp(int x, int y, fix timestamp, int resize_mode);

namespace graphics {
class DebugScope {
 public:
	explicit DebugScope(const char* name) {
		gr_push_debug_group(name);
	}
	~DebugScope() {
		gr_pop_debug_group();
	}
};
}

#ifndef NDEBUG
#define GR_DEBUG_SCOPE(name) ::graphics::DebugScope SCP_TOKEN_CONCAT(gr_scope, __LINE__)(name)
#else
#define GR_DEBUG_SCOPE(name) do {} while(0)
#endif

enum AnimatedShader {
	ANIMATED_SHADER_LOADOUTSELECT_FS1= 0,
	ANIMATED_SHADER_LOADOUTSELECT_FS2= 1,
	ANIMATED_SHADER_CLOAK = 2,
};

graphics::util::UniformBuffer* gr_get_uniform_buffer(uniform_block_type type);

struct VideoModeData {
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t bit_depth = 0;
};

struct DisplayData {
	uint32_t index;
	SCP_string name;

	int32_t x = 0;
	int32_t y = 0;
	int32_t width = 0;
	int32_t height = 0;

	SCP_vector<VideoModeData> video_modes;
};

SCP_vector<DisplayData> gr_enumerate_displays();

enum class GpuHeap {
	ModelVertex = 0,
	ModelIndex,

	NUM_VALUES
};

/**
 * @brief Allocates storage on the specified GPU heap and stores data in that storage
 * @param heap_type The heap type to store this memory on
 * @param size The size of the data
 * @param data A pointer to the data
 * @param[out] offset_out The offset at which the data is stored in the buffer
 * @param[out] handle_out The handle of the buffer object this data is stored in
 */
void gr_heap_allocate(GpuHeap heap_type, size_t size, void* data, size_t& offset_out, int& handle_out);

/**
 * @brief Deallocates memory previously allocated with gr_heap_allocate.
 * @param heap_type The heap type to deallocate this memory from
 * @param data_offset The offset at which the data is stored.
 */
void gr_heap_deallocate(GpuHeap heap_type, size_t data_offset);

// Include this last to make the 2D rendering function available everywhere
#include "graphics/render.h"

#endif
