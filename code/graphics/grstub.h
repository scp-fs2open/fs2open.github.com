

/*
 * $Logfile: /Freespace2/code/Graphics/Grstub.h $
 * $Revision: 2.5 $
 * $Date: 2004-06-28 02:13:07 $
 * $Author: bobboau $
 *
 * $NoKeywords: $
 */

#include "graphics\2d.h"

int	gr_stub_make_light(light_data* light, int idx, int priority){return 0;}
void gr_stub_modify_light(light_data* light, int idx, int priority) {}
void gr_stub_destroy_light(int idx)	{}
void gr_stub_set_light(light_data *light) {}
void gr_stub_reset_lighting() {}
void gr_stub_set_lighting(bool set, bool state) {}
void stub_go_fullscreen(HWND wnd) {}
void stub_minimize() {}
void gr_stub_set_additive_tex_env() {}
void gr_stub_set_tex_env_scale(float scale) {}
void gr_stub_activate(int active) {}
void gr_stub_preload_init() {}
int gr_stub_preload(int bitmap_num, int is_aabitmap){return 0;}
void gr_stub_pixel(int x, int y) {}
void gr_stub_clear() {}
void gr_stub_flip() {}
void gr_stub_flip_window(uint _hdc, int x, int y, int w, int h ) {}
void gr_stub_set_clip(int x,int y,int w,int h) {}
void gr_stub_reset_clip() {}
void gr_stub_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy ) {}
void gr_stub_create_shader(shader * shade, float r, float g, float b, float c ) {}
void gr_stub_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a) {}
void gr_stub_rect(int x,int y,int w,int h) {}
void gr_stub_shade(int x,int y,int w,int h) {}
void gr_stub_aabitmap_ex_internal(int x,int y,int w,int h,int sx,int sy) {}
void gr_stub_aabitmap_ex(int x,int y,int w,int h,int sx,int sy) {}
void gr_stub_aabitmap(int x, int y) {}
void gr_stub_string( int sx, int sy, char *s ) {}
void gr_stub_line(int x1,int y1,int x2,int y2, bool resize = false) {}
void gr_stub_aaline(vertex *v1, vertex *v2) {}
void gr_stub_gradient(int x1,int y1,int x2,int y2) {}
void gr_stub_circle( int xc, int yc, int d ) {}
void gr_stub_stuff_fog_coord(vertex *v) {}
void gr_stub_stuff_secondary_color(vertex *v, ubyte fr, ubyte fg, ubyte fb) {}
void stub_draw_primitive(int nv, vertex ** verts, uint flags, float u_scale, float v_scale, int r, int g, int b, int alpha, int override_primary=0) {}
void stub_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale ) {}
void stub_reset_spec_mapping() {}
void stub_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler) {}
void gr_stub_tmapper( int nverts, vertex **verts, uint flags ) {}
void gr_stub_tmapper_batch_3d_unlit( int nverts, vertex *verts, uint flags) {}
void gr_stub_scaler(vertex *va, vertex *vb ) {}
void gr_stub_set_palette(ubyte *new_palette, int is_alphacolor) {}
void gr_stub_get_color( int * r, int * g, int * b ) {}
void gr_stub_init_color(color *c, int r, int g, int b) {}
void gr_stub_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type ) {}
void gr_stub_set_color( int r, int g, int b ) {}
void gr_stub_set_color_fast(color *dst) {}
void gr_stub_print_screen(char *filename) {}
int gr_stub_supports_res_ingame(int res) {return 1;}
int gr_stub_supports_res_interface(int res) {return 1;}
void gr_stub_cleanup(int minimize){}
void gr_stub_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far) {}
void gr_stub_get_pixel(int x, int y, int *r, int *g, int *b) {}
void gr_stub_set_cull(int cull) {}
void gr_stub_filter_set(int filter) {}
void gr_stub_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct) {}		  
void stub_tcache_init (int use_sections) {}
void stub_free_texture_with_handle(int handle) {}
void stub_tcache_flush () {}
void stub_tcache_cleanup () {}
void stub_tcache_frame () {}
void stub_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out) {}
int gr_stub_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0, int tex_unit = 0) {return 1;}
int gr_stub_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0) {return 0;}
void gr_stub_set_clear_color(int r, int g, int b) {}
void gr_stub_flash(int r, int g, int b) {}
int gr_stub_zbuffer_get() {return 0;}
int gr_stub_zbuffer_set(int mode) {return 0;}
void gr_stub_zbuffer_clear(int mode) {}
void gr_stub_set_gamma(float gamma) {}
void gr_stub_fade_in(int instantaneous) {}
void gr_stub_fade_out(int instantaneous) {}
void gr_stub_get_region(int front, int w, int h, ubyte *data) {}
void gr_stub_save_mouse_area(int x, int y, int w, int h) {}
int gr_stub_save_screen() {return 1;}
void gr_stub_restore_screen(int id) {}
void gr_stub_free_screen(int id) {}
void gr_stub_dump_frame_start(int first_frame, int frames_between_dumps) {}
void gr_stub_dump_frame_stop() {}
void gr_stub_dump_frame() {}
uint gr_stub_lock(){return 1;}
void gr_stub_unlock() {}
void gr_stub_zbias_stub(int bias) {}
void gr_set_fill_mode_stub(int mode) {}
void stub_zbias(int bias) {}
void gr_stub_bitmap_internal(int x,int y,int w,int h,int sx,int sy) {}
void gr_stub_bitmap_ex(int x,int y,int w,int h,int sx,int sy) {}
void gr_stub_bitmap(int x, int y) {}
void gr_stub_push_texture_matrix(int unit) {}
void gr_stub_pop_texture_matrix(int unit) {}
void gr_stub_translate_texture_matrix(int unit, vector *shift) {}
void stub_init_vertex_buffers() {}
int stub_find_first_free_buffer() {return 0;}
int stub_check_for_errors() {return 0;}
int stub_mod_depth() {return 0;}
uint stub_create_vbo(uint size, void** data) {return 0;}
int gr_stub_make_buffer(poly_list *list) {return 0;}
void gr_stub_destroy_buffer(int idx) {}
void gr_stub_render_buffer(int start, int n_prim, short* index_list) {}
void gr_stub_set_buffer(int idx) {}
void gr_stub_start_instance_matrix(vector *offset, matrix* rotation) {}
void gr_stub_start_instance_angles(vector *pos, angles* rotation) {}
void gr_stub_end_instance_matrix() {}
void gr_stub_set_projection_matrix(float fov, float aspect, float z_near, float z_far) {}
void gr_stub_end_projection_matrix() {}
void gr_stub_set_view_matrix(vector *pos, matrix* orient) {}
void gr_stub_end_view_matrix() {}
void gr_stub_push_scale_matrix(vector *scale_factor) {}
void gr_stub_pop_scale_matrix() {}
void gr_stub_end_clip_plane() {}
void gr_stub_start_clip_plane() {}
void gr_stub_center_alpha( int type) {}
void gr_stub_set_texture_addressing(int mode) {}

void stub_set_font(int fontnum) {}
void gr_stub_set_shader( shader * shade ) {}

// Bitmaps
void bm_stub_init(){}
void bm_stub_close(){}
int bm_stub_get_cache_slot( int bitmap_id, int separate_ani_frames ){return 0;}
int bm_stub_get_next_handle(){return 0;}
int bm_stub_load(char * filename){return 0;}
int bm_stub_load_duplicate(char *filename){return 0;}
int bm_stub_create( int bpp, int w, int h, void * data, int flags = 0){return 0;}
int bm_stub_unload( int n ){return 0;}
void bm_stub_release(int n){}
int bm_stub_load_animation( char * filename, int * nframes, int *fps = NULL, int can_drop_frames = 0, int dir_type = CF_TYPE_ANY ){return 0;}
bitmap * bm_stub_lock( int bitmapnum, ubyte bpp, ubyte flags ){return 0;}
void bm_stub_unlock( int bitmapnum ){}
void bm_stub_get_info( int bitmapnum, int *w=NULL, int * h=NULL, ubyte * flags=NULL, int *nframes=NULL, int *fps=NULL, bitmap_section_info **sections = NULL ){}
void bm_stub_load_all(){}
void bm_stub_unload_all(){}
void bm_stub_get_palette(int n, ubyte *pal, char *name){}
void bm_stub_get_pixel( int bitmap, float u, float v, ubyte *r, ubyte *g, ubyte *b ){}
void bm_stub_get_frame_usage(int *ntotal, int *nnew){}
void bm_stub_page_in_start(){}
void bm_stub_page_in_stop(){}
void bm_stub_page_in_texture( int bitmapnum, int num_frames=1 ){}
void bm_stub_page_in_nondarkening_texture( int bitmap, int num_frames=1 ){}
void bm_stub_page_in_xparent_texture( int bitmapnum, int num_frames=1 ){}
void bm_stub_page_in_aabitmap( int bitmapnum, int num_frames=1 ){}
void bm_stub_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a){}
void bm_stub_get_section_size(int bitmapnum, int sx, int sy, int *w, int *h){}

void gr_stub_init() 
{
	gr_screen.gf_flip = gr_stub_flip;
	gr_screen.gf_flip_window = gr_stub_flip_window;
	gr_screen.gf_set_clip = gr_stub_set_clip;
	gr_screen.gf_reset_clip = gr_stub_reset_clip;
	gr_screen.gf_set_font = stub_set_font;
	
	gr_screen.gf_set_color = gr_stub_set_color;
	gr_screen.gf_set_bitmap = gr_stub_set_bitmap;
	gr_screen.gf_create_shader = gr_stub_create_shader;
	gr_screen.gf_set_shader = gr_stub_set_shader;
	gr_screen.gf_clear = gr_stub_clear;
	gr_screen.gf_aabitmap = gr_stub_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_stub_aabitmap_ex;
	
	gr_screen.gf_rect = gr_stub_rect;
	gr_screen.gf_shade = gr_stub_shade;
	gr_screen.gf_string = gr_stub_string;
	gr_screen.gf_circle = gr_stub_circle;

	gr_screen.gf_line = gr_stub_line;
	gr_screen.gf_aaline = gr_stub_aaline;
	gr_screen.gf_pixel = gr_stub_pixel;
	gr_screen.gf_scaler = gr_stub_scaler;
	gr_screen.gf_tmapper = gr_stub_tmapper;
	gr_screen.gf_tmapper_batch_3d_unlit = gr_stub_tmapper_batch_3d_unlit;

	gr_screen.gf_gradient = gr_stub_gradient;

	gr_screen.gf_set_palette = gr_stub_set_palette;
	gr_screen.gf_get_color = gr_stub_get_color;
	gr_screen.gf_init_color = gr_stub_init_color;
	gr_screen.gf_init_alphacolor = gr_stub_init_alphacolor;
	gr_screen.gf_set_color_fast = gr_stub_set_color_fast;
	gr_screen.gf_print_screen = gr_stub_print_screen;

	gr_screen.gf_fade_in = gr_stub_fade_in;
	gr_screen.gf_fade_out = gr_stub_fade_out;
	gr_screen.gf_flash = gr_stub_flash;
	
	gr_screen.gf_zbuffer_get = gr_stub_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_stub_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_stub_zbuffer_clear;
	
	gr_screen.gf_save_screen = gr_stub_save_screen;
	gr_screen.gf_restore_screen = gr_stub_restore_screen;
	gr_screen.gf_free_screen = gr_stub_free_screen;
	
	gr_screen.gf_dump_frame_start = gr_stub_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_stub_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_stub_dump_frame;
	
	gr_screen.gf_set_gamma = gr_stub_set_gamma;
	
	gr_screen.gf_lock = gr_stub_lock;
	gr_screen.gf_unlock = gr_stub_unlock;
	
	gr_screen.gf_fog_set = gr_stub_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region = gr_stub_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_get_next_handle         = bm_stub_get_next_handle;         
	gr_screen.gf_bm_close                   = bm_stub_close;                   
	gr_screen.gf_bm_init                    = bm_stub_init;                    
	gr_screen.gf_bm_get_frame_usage         = bm_stub_get_frame_usage;         
	gr_screen.gf_bm_create                  = bm_stub_create;                  
	gr_screen.gf_bm_load                    = bm_stub_load;                   
	gr_screen.gf_bm_load_duplicate          = bm_stub_load_duplicate;          
	gr_screen.gf_bm_load_animation          = bm_stub_load_animation;          
	gr_screen.gf_bm_get_info                = bm_stub_get_info;                
	gr_screen.gf_bm_lock                    = bm_stub_lock;                    
	gr_screen.gf_bm_unlock                  = bm_stub_unlock;                  
	gr_screen.gf_bm_get_palette             = bm_stub_get_palette;             
	gr_screen.gf_bm_release                 = bm_stub_release;                 
	gr_screen.gf_bm_unload                  = bm_stub_unload;                  
	gr_screen.gf_bm_unload_all              = bm_stub_unload_all;              
	gr_screen.gf_bm_page_in_texture         = bm_stub_page_in_texture;         
	gr_screen.gf_bm_page_in_start           = bm_stub_page_in_start;           
	gr_screen.gf_bm_page_in_stop            = bm_stub_page_in_stop;            
	gr_screen.gf_bm_get_cache_slot          = bm_stub_get_cache_slot;          
	gr_screen.gf_bm_get_components          = bm_stub_get_components;          
	gr_screen.gf_bm_get_section_size        = bm_stub_get_section_size;

	gr_screen.gf_bm_page_in_nondarkening_texture = bm_stub_page_in_nondarkening_texture; 
	gr_screen.gf_bm_page_in_xparent_texture		 = bm_stub_page_in_xparent_texture;		 
	gr_screen.gf_bm_page_in_aabitmap			 = bm_stub_page_in_aabitmap;

	gr_screen.gf_get_pixel = gr_stub_get_pixel;

	gr_screen.gf_set_cull = gr_stub_set_cull;

	gr_screen.gf_cross_fade = gr_stub_cross_fade;

	gr_screen.gf_filter_set = gr_stub_filter_set;

	gr_screen.gf_tcache_set = gr_stub_tcache_set;

	gr_screen.gf_set_clear_color = gr_stub_set_clear_color;

	gr_screen.gf_preload = gr_stub_preload;

	gr_screen.gf_push_texture_matrix = gr_stub_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_stub_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_stub_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing = gr_stub_set_texture_addressing;
	gr_screen.gf_zbias = gr_stub_zbias_stub;
	gr_screen.gf_set_fill_mode = gr_set_fill_mode_stub;

	{
		gr_screen.gf_make_buffer = gr_stub_make_buffer;
		gr_screen.gf_destroy_buffer = gr_stub_destroy_buffer;
		gr_screen.gf_render_buffer = gr_stub_render_buffer;
		gr_screen.gf_set_buffer = gr_stub_set_buffer;

		gr_screen.gf_start_instance_matrix = gr_stub_start_instance_matrix;
		gr_screen.gf_end_instance_matrix = gr_stub_end_instance_matrix;
		gr_screen.gf_start_angles_instance_matrix = gr_stub_start_instance_angles;


		gr_screen.gf_make_light = gr_stub_make_light;
		gr_screen.gf_modify_light = gr_stub_modify_light;
		gr_screen.gf_destroy_light = gr_stub_destroy_light;
		gr_screen.gf_set_light = gr_stub_set_light;
		gr_screen.gf_reset_lighting = gr_stub_reset_lighting;

		gr_screen.gf_start_clip_plane = gr_stub_start_clip_plane;
		gr_screen.gf_end_clip_plane = gr_stub_end_clip_plane;

		gr_screen.gf_lighting = gr_stub_set_lighting;

		gr_screen.gf_set_proj_matrix=gr_stub_set_projection_matrix;
		gr_screen.gf_end_proj_matrix=gr_stub_end_projection_matrix;

		gr_screen.gf_set_view_matrix=gr_stub_set_view_matrix;
		gr_screen.gf_end_view_matrix=gr_stub_end_view_matrix;

		gr_screen.gf_push_scale_matrix = gr_stub_push_scale_matrix;
		gr_screen.gf_pop_scale_matrix = gr_stub_pop_scale_matrix;
		gr_screen.gf_center_alpha = gr_stub_center_alpha;
	}
	{
		gr_screen.gf_make_light = gr_stub_make_light;
		gr_screen.gf_modify_light = gr_stub_modify_light;
		gr_screen.gf_destroy_light = gr_stub_destroy_light;
		gr_screen.gf_set_light = gr_stub_set_light;
		gr_screen.gf_reset_lighting = gr_stub_reset_lighting;


		gr_screen.gf_lighting = gr_stub_set_lighting;

	}
}

