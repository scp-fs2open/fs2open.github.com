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

#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"

extern const float Default_min_draw_distance;
extern const float Default_max_draw_distance;
extern float Min_draw_distance;
extern float Max_draw_distance;
extern int Gr_inited;

// z-buffering stuff
extern int gr_zbuffering, gr_zbuffering_mode;
extern int gr_global_zbuffering;

// Shader flags
#define SDR_FLAG_LIGHT			(1<<0)
#define SDR_FLAG_FOG			(1<<1)
#define SDR_FLAG_DIFFUSE_MAP	(1<<2)
#define SDR_FLAG_GLOW_MAP		(1<<3)
#define SDR_FLAG_SPEC_MAP		(1<<4)
#define SDR_FLAG_NORMAL_MAP		(1<<5)
#define SDR_FLAG_HEIGHT_MAP		(1<<6)
#define SDR_FLAG_ENV_MAP		(1<<7)
#define SDR_FLAG_ANIMATED		(1<<8)
#define SDR_FLAG_SOFT_QUAD		(1<<9)
#define SDR_FLAG_DISTORTION		(1<<10)
#define SDR_FLAG_MISC_MAP		(1<<11)
#define SDR_FLAG_TEAMCOLOR		(1<<12)
#define SDR_FLAG_THRUSTER		(1<<13)

// stencil buffering stuff
extern int gr_stencil_mode;

// alpha test
extern int gr_alpha_test;

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
	ubyte		red;
	ubyte		green;
	ubyte		blue;
	ubyte		alpha;
	ubyte		ac_type;							// The type of alphacolor.  See AC_TYPE_??? defines
	int		is_alphacolor;
	ubyte		raw8;
	int		alphacolor;
	int		magic;		
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
class poly_list
{
public:
	poly_list(): n_verts(0), vert(NULL), norm(NULL), tsb(NULL), currently_allocated(0) {}
	~poly_list();
	poly_list& operator = (poly_list&);

	void allocate(int size);
	void make_index_buffer(SCP_vector<int> &vertex_list);
	void calculate_tangent();
	int n_verts;
	vertex *vert;
	vec3d *norm;
	tsb_t *tsb;

	int find_index(poly_list *plist, int idx);

private:
	int currently_allocated;
	int find_first_vertex(int idx);
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
	int n_verts;

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

	void assign(int i, uint j)
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
	buffer_data(int n_vrts) :
		flags(0), texture(-1), n_verts(n_vrts), index_offset(0),
		i_first(1), i_last(0)
	{
		index = new(std::nothrow) uint[n_verts];
	}
    
	// Copy-constructor
	buffer_data(const buffer_data& other)
	{
		index = new(std::nothrow) uint[other.n_verts];
		for (size_t i=0; i < (size_t) other.n_verts; i++)
		{
			index[i] = other.index[i];
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
			delete [] index;
            
			index = new(std::nothrow) uint[rhs.n_verts];
			for (size_t i=0; i < (size_t) rhs.n_verts; i++)
			{
				index[i] = rhs.index[i];
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

	uint stride;
	size_t vertex_offset;

	poly_list *model_list;

	SCP_vector<buffer_data> tex_buf;

	vertex_buffer() :
		flags(0), stride(0), vertex_offset(0), model_list(NULL)
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

struct light;


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
	int	save_max_w, save_max_h;		// Width and height
	int save_max_w_unscaled, save_max_h_unscaled;
	int save_max_w_unscaled_zoomed, save_max_h_unscaled_zoomed;
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

	bool recording_state_block;
	int current_state_block;

	void (*gf_start_state_block)();
	int (*gf_end_state_block)();
	void (*gf_set_state_block)(int);

	//switch onscreen, offscreen
	void (*gf_flip)();

	// Sets the current palette
	void (*gf_set_palette)(ubyte * new_pal, int restrict_alphacolor);

	// Fade the screen in/out
	void (*gf_fade_in)(int instantaneous);
	void (*gf_fade_out)(int instantaneous);

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

	void (*gf_string)(int x, int y, const char * text,int resize_mode);

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

	// Scales current bitmap between va and vb with clipping
	void (*gf_scaler)(vertex *va, vertex *vb, bool bw_bitmap );

	// Scales current bitmap between va and vb with clipping, draws an aabitmap
	void (*gf_aascaler)(vertex *va, vertex *vb );

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_tmapper)(int nv, vertex *verts[], uint flags );

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_render)(int nv, vertex *verts, uint flags);

	void (*gf_render_effect)(int nv, vertex *verts, float *radius_list, uint flags);

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

	// CODE FOR DUMPING FRAMES TO A FILE
	// Begin frame dumping
	void (*gf_dump_frame_start)( int first_frame_number, int nframes_between_dumps );

	// Dump the current frame to file
	void (*gf_dump_frame)();

	// Dump the current frame to file
	void (*gf_dump_frame_stop)();

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// Lock/unlock the screen
	// Returns non-zero if sucessful (memory pointer)
	uint (*gf_lock)();
	void (*gf_unlock)();

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
	int (*gf_bm_load)(ubyte type, int n, const char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *c_type, int *mm_lvl, int *size);
	void (*gf_bm_init)(int n);
	void (*gf_bm_page_in_start)();
	int (*gf_bm_lock)(const char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags, bool nodebug);

	int (*gf_bm_make_render_target)(int n, int *width, int *height, ubyte *bpp, int *mm_lvl, int flags );
	int (*gf_bm_set_render_target)(int n, int face);

	void (*gf_translate_texture_matrix)(int unit, vec3d *shift);
	void (*gf_push_texture_matrix)(int unit);
	void (*gf_pop_texture_matrix)(int unit);

	void (*gf_set_texture_addressing)(int);

	int (*gf_create_buffer)();
	bool (*gf_pack_buffer)(const int buffer_id, vertex_buffer *vb);
	bool (*gf_config_buffer)(const int buffer_id, vertex_buffer *vb);
	void (*gf_destroy_buffer)(int);
	void (*gf_set_buffer)(int);
	void (*gf_render_buffer)(int, const vertex_buffer*, int, int);

	int (*gf_create_stream_buffer)();
	void (*gf_update_stream_buffer)(int buffer, effect_vertex *buffer_data, uint size);
	void (*gf_render_stream_buffer)(int offset, int n_verts, int flags);
	void (*gf_render_stream_buffer_start)(int buffer_id);
	void (*gf_render_stream_buffer_end)();

	int	 (*gf_make_flat_buffer)(poly_list*);
	int	 (*gf_make_line_buffer)(line_list*);
	

	//the projection matrix; fov, aspect ratio, near, far
 	void (*gf_set_proj_matrix)(float, float, float, float);
  	void (*gf_end_proj_matrix)();
	//the view matrix
 	void (*gf_set_view_matrix)(vec3d *, matrix*);
  	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(vec3d *);
 	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(vec3d *, matrix*);
	void (*gf_start_angles_instance_matrix)(vec3d *, angles*);
	void (*gf_end_instance_matrix)();

	int	 (*gf_make_light)(light*, int, int );
	void (*gf_modify_light)(light*, int, int );
	void (*gf_destroy_light)(int);
	void (*gf_set_light)(light*);
	void (*gf_reset_lighting)();
	void (*gf_set_ambient_light)(int,int,int);

	// postprocessing effects
	void (*gf_post_process_set_effect)(const char*, int);
	void (*gf_post_process_set_defaults)();

	void (*gf_post_process_begin)();
	void (*gf_post_process_end)();
	void (*gf_post_process_save_zbuffer)();

	void (*gf_scene_texture_begin)();
	void (*gf_scene_texture_end)();

	void (*gf_lighting)(bool,bool);
	void (*gf_center_alpha)(int);

	void (*gf_start_clip_plane)();
	void (*gf_end_clip_plane)();

	void (*gf_zbias)(int zbias);
	void (*gf_setup_background_fog)(bool);

	void (*gf_set_fill_mode)(int);
	void (*gf_set_texture_panning)(float u, float v, bool enable);

	void (*gf_draw_line_list)(colored_vector*lines, int num);

	void (*gf_set_line_width)(float width);

	void (*gf_line_htl)(vec3d *start, vec3d* end);
	void (*gf_sphere_htl)(float rad);

	int (*gf_maybe_create_shader)(int flags);

	void (*gf_flush_data_states)();

	void (*gf_set_team_color)(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime);
	void (*gf_enable_team_color)();
	void (*gf_disable_team_color)();

	void (*gf_update_texture)(int bitmap_handle, int bpp, ubyte* data, int width, int height);
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

extern bool gr_init(int d_mode = GR_DEFAULT, int d_width = GR_DEFAULT, int d_height = GR_DEFAULT, int d_depth = GR_DEFAULT);
extern void gr_screen_resize(int width, int height);

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
#define GR_RESIZE_MENU				2
#define GR_RESIZE_MENU_ZOOMED		3
#define GR_RESIZE_MENU_NO_OFFSET	4

void gr_set_screen_scale(int x, int y, int zoom_x = -1, int zoom_y = -1, int max_x = gr_screen.max_w, int max_y = gr_screen.max_h, bool force_stretch = false);
void gr_reset_screen_scale();
bool gr_unsize_screen_pos(int *x, int *y, int *w = NULL, int *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_resize_screen_pos(int *x, int *y, int *w = NULL, int *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_unsize_screen_posf(float *x, float *y, float *w = NULL, float *h = NULL, int resize_mode = GR_RESIZE_FULL);
bool gr_resize_screen_posf(float *x, float *y, float *w = NULL, float *h = NULL, int resize_mode = GR_RESIZE_FULL);

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void _cdecl gr_printf( int x, int y, const char * format, ... );
// same as gr_printf but positions text correctly in menus
extern void _cdecl gr_printf_menu( int x, int y, const char * format, ... );
// same as gr_printf_menu but accounts for menu zooming
extern void _cdecl gr_printf_menu_zoomed( int x, int y, const char * format, ... );
// same as gr_printf but doesn't resize for non-standard resolutions
extern void _cdecl gr_printf_no_resize( int x, int y, const char * format, ... );

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size( int *w, int *h, const char * text, int len = 9999 );

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(const char *name, ubyte *palette, int restrict_to_128 = 0);

// These two functions use a Windows mono font.  Only for use
// in the editor, please.
void gr_get_string_size_win(int *w, int *h, const char *text);
void gr_string_win(int x, int y, const char *s );

// set the mouse pointer to a specific bitmap, used for animating cursors
#define GR_CURSOR_LOCK		1
#define GR_CURSOR_UNLOCK	2
void gr_set_cursor_bitmap(int n, int lock = 0);
void gr_unset_cursor_bitmap(int n);
int gr_get_cursor_bitmap();
extern int Web_cursor_bitmap;

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

__inline void gr_string(int x, int y, const char* string, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_string)(x,y,string,resize_mode);
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
#define gr_scaler				GR_CALL(gr_screen.gf_scaler)
#define gr_aascaler			GR_CALL(gr_screen.gf_aascaler)
#define gr_tmapper			GR_CALL(gr_screen.gf_tmapper)
#define gr_render			GR_CALL(gr_screen.gf_render)
#define gr_render_effect	GR_CALL(gr_screen.gf_render_effect)

__inline void gr_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL)
{
	(*gr_screen.gf_gradient)(x1, y1, x2, y2, resize_mode);
}

#define gr_fade_in			GR_CALL(gr_screen.gf_fade_in)
#define gr_fade_out			GR_CALL(gr_screen.gf_fade_out)
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

#define gr_dump_frame_start	GR_CALL(gr_screen.gf_dump_frame_start)
#define gr_dump_frame_stop		GR_CALL(gr_screen.gf_dump_frame_stop)
#define gr_dump_frame			GR_CALL(gr_screen.gf_dump_frame)

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
__inline int gr_bm_load(ubyte type, int n, const char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *c_type = 0, int *mm_lvl = 0, int *size = 0)
{
	return (*gr_screen.gf_bm_load)(type, n, filename, img_cfp, w, h, bpp, c_type, mm_lvl, size);
}
#define gr_bm_page_in_start			GR_CALL(*gr_screen.gf_bm_page_in_start)
#define gr_bm_lock					GR_CALL(*gr_screen.gf_bm_lock)          

#define gr_bm_make_render_target					GR_CALL(*gr_screen.gf_bm_make_render_target)          
        
__inline int gr_bm_set_render_target(int n, int face = -1)
{
	return (*gr_screen.gf_bm_set_render_target)(n, face);
}

#define gr_set_texture_addressing					 GR_CALL(*gr_screen.gf_set_texture_addressing)            

#define gr_create_buffer				GR_CALL(*gr_screen.gf_create_buffer)
#define gr_pack_buffer					GR_CALL(*gr_screen.gf_pack_buffer)
#define gr_config_buffer				GR_CALL(*gr_screen.gf_config_buffer)
#define gr_destroy_buffer				 GR_CALL(*gr_screen.gf_destroy_buffer)
__inline void gr_render_buffer(int start, const vertex_buffer *bufferp, int texi, int flags = TMAP_FLAG_TEXTURED)
{
	(*gr_screen.gf_render_buffer)(start, bufferp, texi, flags);
}

#define gr_create_stream_buffer			GR_CALL(*gr_screen.gf_create_stream_buffer)
#define gr_update_stream_buffer			GR_CALL(*gr_screen.gf_update_stream_buffer)
#define gr_render_stream_buffer			GR_CALL(*gr_screen.gf_render_stream_buffer)
#define gr_render_stream_buffer_start	GR_CALL(*gr_screen.gf_render_stream_buffer_start)
#define gr_render_stream_buffer_end		GR_CALL(*gr_screen.gf_render_stream_buffer_end)

#define gr_set_buffer					GR_CALL(*gr_screen.gf_set_buffer)      
      
#define gr_make_flat_buffer				GR_CALL(*gr_screen.gf_make_flat_buffer)            
#define gr_make_line_buffer				GR_CALL(*gr_screen.gf_make_line_buffer)            

#define gr_set_proj_matrix					GR_CALL(*gr_screen.gf_set_proj_matrix)            
#define gr_end_proj_matrix					GR_CALL(*gr_screen.gf_end_proj_matrix)            
#define gr_set_view_matrix					GR_CALL(*gr_screen.gf_set_view_matrix)            
#define gr_end_view_matrix					GR_CALL(*gr_screen.gf_end_view_matrix)            
#define gr_push_scale_matrix				GR_CALL(*gr_screen.gf_push_scale_matrix)            
#define gr_pop_scale_matrix					GR_CALL(*gr_screen.gf_pop_scale_matrix)            
#define gr_start_instance_matrix			GR_CALL(*gr_screen.gf_start_instance_matrix)            
#define gr_start_angles_instance_matrix		GR_CALL(*gr_screen.gf_start_angles_instance_matrix)            
#define gr_end_instance_matrix				GR_CALL(*gr_screen.gf_end_instance_matrix)            

#define	gr_make_light					GR_CALL(*gr_screen.gf_make_light)
#define	gr_modify_light					GR_CALL(*gr_screen.gf_modify_light)
#define	gr_destroy_light				GR_CALL(*gr_screen.gf_destroy_light)
#define	gr_set_light					GR_CALL(*gr_screen.gf_set_light)
#define gr_reset_lighting				GR_CALL(*gr_screen.gf_reset_lighting)
#define gr_set_ambient_light			GR_CALL(*gr_screen.gf_set_ambient_light)

#define gr_scene_texture_begin			GR_CALL(*gr_screen.gf_scene_texture_begin)
#define gr_scene_texture_end			GR_CALL(*gr_screen.gf_scene_texture_end)

#define gr_post_process_set_effect		GR_CALL(*gr_screen.gf_post_process_set_effect)
#define gr_post_process_set_defaults	GR_CALL(*gr_screen.gf_post_process_set_defaults)
#define gr_post_process_begin			GR_CALL(*gr_screen.gf_post_process_begin)
#define gr_post_process_end				GR_CALL(*gr_screen.gf_post_process_end)
#define gr_post_process_save_zbuffer	GR_CALL(*gr_screen.gf_post_process_save_zbuffer)

#define	gr_set_lighting					GR_CALL(*gr_screen.gf_lighting)
#define	gr_center_alpha					GR_CALL(*gr_screen.gf_center_alpha)

#define	gr_start_clip					GR_CALL(*gr_screen.gf_start_clip_plane)
#define	gr_end_clip						GR_CALL(*gr_screen.gf_end_clip_plane)

#define	gr_zbias						GR_CALL(*gr_screen.gf_zbias)
#define	gr_set_fill_mode				GR_CALL(*gr_screen.gf_set_fill_mode)
#define	gr_set_texture_panning			GR_CALL(*gr_screen.gf_set_texture_panning)

#define	gr_start_state_block			GR_CAL(*gr_screen.gf_start_state_block)
#define	gr_end_state_block				GR_CALL(*gr_screen.gf_end_state_block)
#define	gr_set_state_block				GR_CALL(*gr_screen.gf_set_state_block)

#define gr_setup_background_fog			GR_CALL(*gr_screen.gf_setup_background_fog)

#define gr_draw_line_list				GR_CALL(*gr_screen.gf_draw_line_list)

#define gr_set_line_width				GR_CALL(*gr_screen.gf_set_line_width)

#define gr_line_htl						GR_CALL(*gr_screen.gf_line_htl)
#define gr_sphere_htl					GR_CALL(*gr_screen.gf_sphere_htl)

#define gr_maybe_create_shader			GR_CALL(*gr_screen.gf_maybe_create_shader)

#define gr_flush_data_states			GR_CALL(*gr_screen.gf_flush_data_states)

#define gr_set_team_color				GR_CALL(*gr_screen.gf_set_team_color)
#define gr_disable_team_color			GR_CALL(*gr_screen.gf_disable_team_color)

#define gr_update_texture				GR_CALL(*gr_screen.gf_update_texture)

// color functions
void gr_get_color( int *r, int *g, int  b );
void gr_init_color(color *c, int r, int g, int b);
void gr_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type = AC_TYPE_HUD );
void gr_set_color( int r, int g, int b );
void gr_set_color_fast(color *dst);

// shader functions
void gr_create_shader(shader *shade, ubyte r, ubyte g, ubyte b, ubyte c);
void gr_set_shader(shader *shade);

// new bitmap functions
void gr_bitmap(int x, int y, int resize_mode = GR_RESIZE_FULL);
void gr_bitmap_uv(int _x, int _y, int _w, int _h, float _u0, float _v0, float _u1, float _v1, int resize_mode = GR_RESIZE_FULL);
void gr_bitmap_list(bitmap_2d_list* list, int n_bm, int resize_mode);
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, int resize_mode);

// texture update functions
ubyte* gr_opengl_get_texture_update_pointer(int bitmap_handle);
void gr_opengl_update_texture(int bitmap_handle, int bpp, ubyte* data, int width, int height);

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

void gr_clear_shaders_cache();

#endif
