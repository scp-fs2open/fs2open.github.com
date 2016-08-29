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
#include <osapi/osapi.h>
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "cfile/cfile.h"
#include "math/vecmat.h"
#include "io/cursor.h"

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

struct transform
{
	matrix basis;
	vec3d origin;
	vec3d scale;

	transform() : basis(vmd_identity_matrix), origin(vmd_zero_vector), scale(vmd_scale_identity_vector) {}
	transform(matrix *m, vec3d *v) : basis(*m), origin(*v) {}

	matrix4 get_matrix4()
	{
		matrix4 new_mat;
		vm_matrix4_set_identity(&new_mat);

		new_mat.a1d[0] = basis.vec.rvec.xyz.x;   new_mat.a1d[4] = basis.vec.uvec.xyz.x;   new_mat.a1d[8] = basis.vec.fvec.xyz.x;
		new_mat.a1d[1] = basis.vec.rvec.xyz.y;   new_mat.a1d[5] = basis.vec.uvec.xyz.y;   new_mat.a1d[9] = basis.vec.fvec.xyz.y;
		new_mat.a1d[2] = basis.vec.rvec.xyz.z;   new_mat.a1d[6] = basis.vec.uvec.xyz.z;   new_mat.a1d[10] = basis.vec.fvec.xyz.z;
		new_mat.a1d[12] = origin.xyz.x;
		new_mat.a1d[13] = origin.xyz.y;
		new_mat.a1d[14] = origin.xyz.z;

		return new_mat;
	}
};


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
	SDR_TYPE_PASSTHROUGH_RENDER,

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

struct vertex_format_data
{
	enum vertex_format {
		POSITION4,
		POSITION3,
		POSITION2,
		SCREEN_POS,
		COLOR3,
		COLOR4,
		TEX_COORD,
		NORMAL,
		TANGENT,
		MODEL_ID,
		RADIUS,
		UVEC
	};

	vertex_format format_type;
	size_t stride;
	void *data_src;
	int offset;

	vertex_format_data(vertex_format i_format_type, size_t i_stride, void *i_data_src) : 
	format_type(i_format_type), stride(i_stride), data_src(i_data_src), offset(-1) {}

	vertex_format_data(vertex_format i_format_type, size_t i_stride, int i_offset) : 
	format_type(i_format_type), stride(i_stride), data_src(NULL), offset(i_offset) {}

	static inline uint mask(vertex_format v_format) { return 1 << v_format; }
};

class vertex_layout
{
	SCP_vector<vertex_format_data> Vertex_components;

	uint Vertex_mask;
public:
	vertex_layout(): Vertex_mask(0) {}

	vertex_layout(void* init_ptr): Vertex_mask(0) {}

	size_t get_num_vertex_components() { return Vertex_components.size(); }

	vertex_format_data* get_vertex_component(size_t index) { return &Vertex_components[index]; }
	
	bool resident_vertex_format(vertex_format_data::vertex_format format_type)
	{ 
		return ( Vertex_mask & vertex_format_data::mask(format_type) ) ? true : false; 
	} 

	void add_vertex_component(vertex_format_data::vertex_format format_type, void* src)
	{
		add_vertex_component(format_type, 0, src);
	}

	void add_vertex_component(vertex_format_data::vertex_format format_type, size_t stride, void* src) 
	{
		if ( resident_vertex_format(format_type) ) {
			// we already have a vertex component of this format type
			return;
		}

		Vertex_mask |= (1 << format_type);
		Vertex_components.push_back(vertex_format_data(format_type, stride, src));
	}

	void add_vertex_component(vertex_format_data::vertex_format format_type, size_t stride, int offset) 
	{
		if ( resident_vertex_format(format_type) ) {
			// we already have a vertex component of this format type
			return;
		}

		Vertex_mask |= (1 << format_type);
		Vertex_components.push_back(vertex_format_data(format_type, stride, offset));
	}
};

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
	CAPABILITY_POINT_PARTICLES
} gr_capability;

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
	poly_list& operator = (poly_list&);

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

class colored_vector
{
public:
	colored_vector()
		: pad(1.0f)
	{}

	vec3d vec;
	float pad;	//needed so I can just memcpy it in d3d
	ubyte color[4];
};


struct line_list {
	int n_line;
	vertex *vert;
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
	float *Vertex_list;	// interleaved array
	ubyte *Index_list;

	int Vbuffer_handle;
	int Ibuffer_handle;

	uint Vertex_list_size;
	uint Index_list_size;

	indexed_vertex_source() :
		Vertex_list(NULL), Index_list(NULL),
		Vbuffer_handle(-1), Ibuffer_handle(-1), Vertex_list_size(0), Index_list_size(0)
	{
	}
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
	int		current_fog_mode;					// See GR_FOGMODE_* defines above
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

	//switch onscreen, offscreen
	void (*gf_flip)();

	// Flash the screen
	void (*gf_flash)( int r, int g, int b );
	void (*gf_flash_alpha)(int r, int g, int b, int a);

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h, int resize_mode);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	// clears entire clipping region to current color
	void (*gf_clear)();

	// void (*gf_bitmap)(int x, int y, int resize_mode);
	void (*gf_bitmap_ex)(int x, int y, int w, int h, int sx, int sy, int resize_mode);

	void (*gf_aabitmap)(int x, int y, int resize_mode, bool mirror);
	void (*gf_aabitmap_ex)(int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror);

	void(*gf_string)(float x, float y, const char * text, int resize_mode, int length);

	// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
	void (*gf_gradient)(int x1, int y1, int x2, int y2, int resize_mode);
 
	void (*gf_circle)(int x, int y, int r, int resize_mode);
	void (*gf_unfilled_circle)(int x, int y, int r, int resize_mode);
	void (*gf_arc)(int x, int y, float r, float angle_start, float angle_end, bool fill, int resize_mode);
	void (*gf_curve)(int x, int y, int r, int direction, int resize_mode);

	// Integer line. Used to draw a fast but pixely line.  
	void (*gf_line)(int x1, int y1, int x2, int y2, int resize_mode);

	// Draws an antialiased line is the current color is an 
	// alphacolor, otherwise just draws a fast line.  This
	// gets called internally by g3_draw_line.   This assumes
	// the vertex's are already clipped, so call g3_draw_line
	// not this if you have two 3d points.
	void (*gf_aaline)(vertex *v1, vertex *v2);

	void (*gf_pixel)( int x, int y, int resize_mode );
	
	// dumps the current screen to a file
	void (*gf_print_screen)(const char * filename);

	// Call once before rendering anything.
	void (*gf_start_frame)();

	// Call after rendering is over.
	void (*gf_stop_frame)();

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

	// set fog attributes
	void (*gf_fog_set)(int fog_mode, int r, int g, int b, float fog_near, float fog_far);	

	// poly culling
	int (*gf_set_cull)(int cull);

	// color buffer writes
	int (*gf_set_color_buffer)(int mode);

	// cross fade
	void (*gf_cross_fade)(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct, int resize_mode);

	// set a texture into cache. for sectioned bitmaps, pass in sx and sy to set that particular section of the bitmap
	int (*gf_tcache_set)(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int stage);	

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

	int (*gf_create_vertex_buffer)(bool static_buffer);
	int (*gf_create_index_buffer)(bool static_buffer);
	void (*gf_delete_buffer)(int handle);

	void (*gf_update_buffer_data)(int handle, size_t size, void* data);
	void (*gf_update_transform_buffer)(void* data, size_t size);
	void (*gf_set_transform_buffer_offset)(size_t offset);

	void (*gf_render_stream_buffer)(int buffer_handle, size_t offset, size_t n_verts, int flags);
	
	//the projection matrix; fov, aspect ratio, near, far
 	void (*gf_set_proj_matrix)(float, float, float, float);
  	void (*gf_end_proj_matrix)();
	//the view matrix
 	void (*gf_set_view_matrix)(const vec3d*, const matrix*);
  	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(const vec3d*);
 	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(const vec3d*, const matrix*);
	void (*gf_start_angles_instance_matrix)(const vec3d*, const angles*);
	void (*gf_end_instance_matrix)();

	void (*gf_set_light)(light*);
	void (*gf_reset_lighting)();
	void (*gf_set_ambient_light)(int,int,int);

	// postprocessing effects
	void (*gf_post_process_set_effect)(const char*, int);
	void (*gf_post_process_set_defaults)();

	void (*gf_post_process_begin)();
	void (*gf_post_process_end)();
	void (*gf_post_process_save_zbuffer)();

	void (*gf_deferred_lighting_begin)();
	void (*gf_deferred_lighting_end)();
	void (*gf_deferred_lighting_finish)();

	void (*gf_scene_texture_begin)();
	void (*gf_scene_texture_end)();
	void (*gf_copy_effect_texture)();

	void (*gf_lighting)(bool,bool);

	void (*gf_start_clip_plane)();
	void (*gf_end_clip_plane)();

	void (*gf_zbias)(int zbias);

	void (*gf_set_fill_mode)(int);
	void (*gf_set_texture_panning)(float u, float v, bool enable);
	
	void (*gf_set_line_width)(float width);

	void (*gf_line_htl)(const vec3d *start, const vec3d *end);
	void (*gf_sphere_htl)(float rad);
	void (*gf_sphere)(material *material_def, float rad);

	int (*gf_maybe_create_shader)(shader_type type, unsigned int flags);
	
	void (*gf_clear_states)();
	
	void (*gf_update_texture)(int bitmap_handle, int bpp, const ubyte* data, int width, int height);
	void (*gf_get_bitmap_from_texture)(void* data_out, int bitmap_num);

	void (*gf_shadow_map_start)(matrix4 *shadow_view_matrix, const matrix *light_matrix);
	void (*gf_shadow_map_end)();

	// new drawing functions
	void (*gf_render_model)(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi);
	void (*gf_render_primitives)(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_primitives_immediate)(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size);
	void (*gf_render_primitives_particle)(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_primitives_distortion)(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_primitives_2d)(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle);
	void (*gf_render_primitives_2d_immediate)(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size);

	bool (*gf_is_capable)(gr_capability capability);

	void (*gf_push_debug_group)(const char* name);
	void (*gf_pop_debug_group)();
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

extern bool gr_init(os::GraphicsOperations* graphicsOps, int d_mode = GR_DEFAULT,
					int d_width = GR_DEFAULT, int d_height = GR_DEFAULT, int d_depth = GR_DEFAULT);

extern void gr_screen_resize(int width, int height);
extern int gr_get_resolution_class(int width, int height);

// Call this when your app ends.
extern void gr_close(os::GraphicsOperations* graphicsOps);

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
extern void gr_printf( int x, int y, const char * format, ... );
// same as gr_printf but positions text correctly in menus
extern void gr_printf_menu( int x, int y, const char * format, ... );
// same as gr_printf_menu but accounts for menu zooming
extern void gr_printf_menu_zoomed( int x, int y, const char * format, ... );
// same as gr_printf but doesn't resize for non-standard resolutions
extern void gr_printf_no_resize( int x, int y, const char * format, ... );

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
void gr_flip();

//#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
__inline void gr_set_clip(int x, int y, int w, int h, int resize_mode=GR_RESIZE_FULL)
{
	(*gr_screen.gf_set_clip)(x,y,w,h,resize_mode);
}
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)

void gr_set_bitmap(int bitmap_num, int alphablend = GR_ALPHABLEND_NONE, int bitbltmode = GR_BITBLT_MODE_NORMAL, float alpha = 1.0f);

#define gr_clear				GR_CALL(gr_screen.gf_clear)
__inline void gr_aabitmap(int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false)
{
	(*gr_screen.gf_aabitmap)(x,y,resize_mode,mirror);
}

__inline void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false)
{
	(*gr_screen.gf_aabitmap_ex)(x,y,w,h,sx,sy,resize_mode,mirror);
}

__inline void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_bitmap_ex)(x, y, w, h, sx, sy, resize_mode);
}

void gr_shield_icon(coord2d coords[6], const int resize_mode = GR_RESIZE_FULL);
void gr_rect(int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);
void gr_shade(int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);

__inline void gr_string(float x, float y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1)
{
	(*gr_screen.gf_string)(x, y, string, resize_mode, length);
}

__inline void gr_string(int x, int y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1)
{
	gr_string(i2fl(x), i2fl(y), string, resize_mode, length);
}

__inline void gr_circle(int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_circle)(xc,yc,d,resize_mode);
}

__inline void gr_unfilled_circle(int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_unfilled_circle)(xc,yc,d,resize_mode);
}

__inline void gr_arc(int xc, int yc, float r, float angle_start, float angle_end, bool fill, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_arc)(xc,yc,r,angle_start,angle_end,fill,resize_mode);
}

#define gr_curve				GR_CALL(gr_screen.gf_curve)

__inline void gr_line(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_line)(x1, y1, x2, y2, resize_mode);
}

#define gr_aaline				GR_CALL(gr_screen.gf_aaline)

__inline void gr_pixel(int x, int y, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_pixel)(x, y, resize_mode);
}

__inline void gr_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_gradient)(x1, y1, x2, y2, resize_mode);
}

#define gr_flash			GR_CALL(gr_screen.gf_flash)
#define gr_flash_alpha		GR_CALL(gr_screen.gf_flash_alpha)

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

__inline void gr_fog_set(int fog_mode, int r, int g, int b, float fog_near = -1.0f, float fog_far = -1.0f)
{
	(*gr_screen.gf_fog_set)(fog_mode, r, g, b, fog_near, fog_far);
}

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)
#define gr_set_color_buffer	GR_CALL(gr_screen.gf_set_color_buffer)

#define gr_cross_fade		GR_CALL(gr_screen.gf_cross_fade)

__inline int gr_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int stage = 0)
{
	return (*gr_screen.gf_tcache_set)(bitmap_id, bitmap_type, u_scale, v_scale, stage);
}

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

__inline int gr_create_vertex_buffer(bool static_buffer = false)
{
	return (*gr_screen.gf_create_vertex_buffer)(static_buffer);
}

__inline int gr_create_index_buffer(bool static_buffer = false)
{
	return (*gr_screen.gf_create_index_buffer)(static_buffer);
}

#define gr_delete_buffer				GR_CALL(*gr_screen.gf_delete_buffer)
#define gr_update_buffer_data			GR_CALL(*gr_screen.gf_update_buffer_data)
#define gr_update_transform_buffer		GR_CALL(*gr_screen.gf_update_transform_buffer)
#define gr_set_transform_buffer_offset	GR_CALL(*gr_screen.gf_set_transform_buffer_offset)

#define gr_set_proj_matrix					GR_CALL(*gr_screen.gf_set_proj_matrix)            
#define gr_end_proj_matrix					GR_CALL(*gr_screen.gf_end_proj_matrix)            
#define gr_set_view_matrix					GR_CALL(*gr_screen.gf_set_view_matrix)            
#define gr_end_view_matrix					GR_CALL(*gr_screen.gf_end_view_matrix)            
#define gr_push_scale_matrix				GR_CALL(*gr_screen.gf_push_scale_matrix)            
#define gr_pop_scale_matrix					GR_CALL(*gr_screen.gf_pop_scale_matrix)            
#define gr_start_instance_matrix			GR_CALL(*gr_screen.gf_start_instance_matrix)            
#define gr_start_angles_instance_matrix		GR_CALL(*gr_screen.gf_start_angles_instance_matrix)            
#define gr_end_instance_matrix				GR_CALL(*gr_screen.gf_end_instance_matrix)            

#define	gr_set_light					GR_CALL(*gr_screen.gf_set_light)
#define gr_reset_lighting				GR_CALL(*gr_screen.gf_reset_lighting)
#define gr_set_ambient_light			GR_CALL(*gr_screen.gf_set_ambient_light)

#define gr_scene_texture_begin			GR_CALL(*gr_screen.gf_scene_texture_begin)
#define gr_scene_texture_end			GR_CALL(*gr_screen.gf_scene_texture_end)
#define gr_copy_effect_texture			GR_CALL(*gr_screen.gf_copy_effect_texture)

#define gr_post_process_set_effect		GR_CALL(*gr_screen.gf_post_process_set_effect)
#define gr_post_process_set_defaults	GR_CALL(*gr_screen.gf_post_process_set_defaults)
#define gr_post_process_begin			GR_CALL(*gr_screen.gf_post_process_begin)
#define gr_post_process_end				GR_CALL(*gr_screen.gf_post_process_end)
#define gr_post_process_save_zbuffer	GR_CALL(*gr_screen.gf_post_process_save_zbuffer)

#define gr_deferred_lighting_begin		GR_CALL(*gr_screen.gf_deferred_lighting_begin)
#define gr_deferred_lighting_end		GR_CALL(*gr_screen.gf_deferred_lighting_end)
#define gr_deferred_lighting_finish		GR_CALL(*gr_screen.gf_deferred_lighting_finish)

#define	gr_set_lighting					GR_CALL(*gr_screen.gf_lighting)

#define	gr_start_clip					GR_CALL(*gr_screen.gf_start_clip_plane)
#define	gr_end_clip						GR_CALL(*gr_screen.gf_end_clip_plane)

#define	gr_zbias						GR_CALL(*gr_screen.gf_zbias)
#define	gr_set_fill_mode				GR_CALL(*gr_screen.gf_set_fill_mode)
#define	gr_set_texture_panning			GR_CALL(*gr_screen.gf_set_texture_panning)

#define gr_set_line_width				GR_CALL(*gr_screen.gf_set_line_width)

#define gr_line_htl						GR_CALL(*gr_screen.gf_line_htl)
#define gr_sphere_htl					GR_CALL(*gr_screen.gf_sphere_htl)
#define gr_sphere						GR_CALL(*gr_screen.gf_sphere)

#define gr_maybe_create_shader			GR_CALL(*gr_screen.gf_maybe_create_shader)
#define gr_set_animated_effect			GR_CALL(*gr_screen.gf_set_animated_effect)

#define gr_clear_states					GR_CALL(*gr_screen.gf_clear_states)

#define gr_update_texture				GR_CALL(*gr_screen.gf_update_texture)
#define gr_get_bitmap_from_texture		GR_CALL(*gr_screen.gf_get_bitmap_from_texture)

#define gr_shadow_map_start				GR_CALL(*gr_screen.gf_shadow_map_start)
#define gr_shadow_map_end				GR_CALL(*gr_screen.gf_shadow_map_end)

__inline void gr_render_primitives(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_immediate(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size)
{
	(*gr_screen.gf_render_primitives_immediate)(material_info, prim_type, layout, n_verts, data, size);
}

__inline void gr_render_primitives_particle(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_particle)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_distortion(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_distortion)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_2d(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle = -1)
{
	(*gr_screen.gf_render_primitives_2d)(material_info, prim_type, layout, offset, n_verts, buffer_handle);
}

__inline void gr_render_primitives_2d_immediate(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size)
{
	(*gr_screen.gf_render_primitives_2d_immediate)(material_info, prim_type, layout, n_verts, data, size);
}

__inline void gr_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi)
{
	(*gr_screen.gf_render_model)(material_info, vert_source, bufferp, texi);
}

__inline bool gr_is_capable(gr_capability capability)
{
	return (*gr_screen.gf_is_capable)(capability);
}

inline void gr_push_debug_group(const char* name)
{
	(*gr_screen.gf_push_debug_group)(name);
}

inline void gr_pop_debug_group()
{
	(*gr_screen.gf_pop_debug_group)();
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
* @param timestamp The timespamp in milliseconds to be printed
* @param resize_mode The resize mode to use
*/
void gr_print_timestamp(int x, int y, int timestamp, int resize_mode);

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
#define GR_DEBUG_SCOPE(var, name) ::graphics::DebugScope var(name)
#else
#define GR_DEBUG_SCOPE(var, name) do {} while(0)
#endif

#endif
