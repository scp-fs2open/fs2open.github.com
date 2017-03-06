

#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "jpgutils/jpgutils.h"
#include "model/model.h"
#include "graphics/material.h"
#include "pcxutils/pcxutils.h"
#include "pngutils/pngutils.h"
#include "tgautils/tgautils.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"


uint gr_stub_lock()
{
	return 1;
}

int gr_stub_create_vertex_buffer(bool static_buffer)
{
	return -1;
}

int gr_stub_create_index_buffer(bool static_buffer)
{
	return -1;
}

void gr_stub_delete_buffer(int handle)
{

}

int gr_stub_preload(int bitmap_num, int is_aabitmap)
{
	return 0;
}

int gr_stub_save_screen()
{
	return 1;
}

int gr_stub_zbuffer_get()
{
	return 0;
}

int gr_stub_zbuffer_set(int mode)
{
	return 0;
}

void gr_set_fill_mode_stub(int mode)
{
}

void gr_stub_activate(int active)
{
}


void gr_stub_cleanup(int minimize)
{
}

void gr_stub_clear()
{
}

void gr_stub_end_clip_plane()
{
}

void gr_stub_end_instance_matrix()
{
}

void gr_stub_end_projection_matrix()
{
}

void gr_stub_end_view_matrix()
{
}

void gr_stub_flip()
{
}

void gr_stub_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
}

void gr_stub_free_screen(int id)
{
}

void gr_stub_get_region(int front, int w, int h, ubyte *data)
{
}

void gr_stub_pop_scale_matrix()
{
}

void gr_stub_pop_texture_matrix(int unit)
{
}

void gr_stub_preload_init()
{
}

void gr_stub_print_screen(const char *filename)
{
}

void gr_stub_push_scale_matrix(const vec3d *scale_factor)
{
}

void gr_stub_push_texture_matrix(int unit)
{
}

void gr_stub_rect(int x, int y, int w, int h, int resize_mode)
{
}

void gr_stub_reset_clip()
{
}

void gr_stub_reset_lighting()
{
}

void gr_stub_restore_screen(int id)
{
}

void gr_stub_save_mouse_area(int x, int y, int w, int h)
{
}

void gr_stub_update_buffer_data(int handle, size_t size, void* data)
{

}

void gr_stub_update_transform_buffer(void* data, size_t size)
{

}

void gr_stub_set_transform_buffer_offset(size_t offset)
{

}

void gr_stub_render_stream_buffer(int buffer_handle, size_t offset, size_t n_verts, int flags)
{
}

void gr_stub_set_clear_color(int r, int g, int b)
{
}

void gr_stub_set_clip(int x, int y, int w, int h, int resize_mode)
{
}

int gr_stub_set_cull(int cull)
{
	return 0;
}

int gr_stub_set_color_buffer(int mode)
{
	return 0;
}

void gr_stub_set_gamma(float gamma)
{
}

void gr_stub_set_lighting(bool set, bool state)
{
}

void gr_stub_set_light(light *light)
{
}

void gr_stub_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
}

void gr_stub_set_tex_env_scale(float scale)
{
}

void gr_stub_set_texture_addressing(int mode)
{
}

void gr_stub_set_view_matrix(const vec3d *pos, const matrix* orient)
{
}

void gr_stub_start_clip_plane()
{
}

void gr_stub_start_instance_angles(const vec3d *pos, const angles *rotation)
{
}

void gr_stub_start_instance_matrix(const vec3d *offset, const matrix *rotation)
{
}

void gr_stub_stuff_fog_coord(vertex *v)
{
}

void gr_stub_stuff_secondary_color(vertex *v, ubyte fr, ubyte fg, ubyte fb)
{
}

void gr_stub_translate_texture_matrix(int unit, const vec3d *shift)
{
}

void gr_stub_zbias_stub(int bias)
{
}

void gr_stub_zbuffer_clear(int mode)
{
}

int gr_stub_stencil_set(int mode)
{
	return 0;
}

void gr_stub_stencil_clear()
{
}

int gr_stub_alpha_mask_set(int mode, float alpha)
{
	return 0;
}

/*void gr_stub_shade(int x,int y,int w,int h)
{
}*/

void gr_stub_post_process_set_effect(const char *name, int x, const vec3d *rgb)
{
}

void gr_stub_post_process_set_defaults()
{
}

void gr_stub_post_process_save_zbuffer()
{
}

void gr_stub_post_process_blur_shadow_map()
{
}

void gr_stub_post_process_begin()
{
}

void gr_stub_post_process_end()
{
}

void gr_stub_scene_texture_begin()
{
}

void gr_stub_scene_texture_end()
{
}

void gr_stub_copy_effect_texture()
{
}

void gr_stub_deferred_lighting_begin()
{
}

void gr_stub_deferred_lighting_end()
{
}

void gr_stub_deferred_lighting_finish()
{
}

void gr_stub_set_ambient_light(int red, int green, int blue)
{
}

void gr_stub_set_texture_panning(float u, float v, bool enable)
{
}

void gr_stub_set_line_width(float width)
{
}

void gr_stub_draw_sphere(material *material_def, float rad)
{
}

void gr_stub_clear_states()
{
}

void gr_stub_update_texture(int bitmap_handle, int bpp, const ubyte *data, int width, int height)
{
}

void gr_stub_get_bitmap_from_texture(void* data_out, int bitmap_num)
{

}

int gr_stub_bm_make_render_target(int n, int *width, int *height, int *bpp, int *mm_lvl, int flags)
{
	return 0;
}

int gr_stub_bm_set_render_target(int n, int face)
{
	return 0;
}

void gr_stub_bm_create(int n)
{
}

void gr_stub_bm_free_data(int n, bool release)
{
}

void gr_stub_bm_init(int n)
{
}

void gr_stub_bm_page_in_start()
{
}

bool gr_stub_bm_data(int n, bitmap* bm)
{
	return true;
}

int gr_stub_maybe_create_shader(shader_type shader_t, unsigned int flags) {
	return -1;
}

void gr_stub_shadow_map_start(matrix4 *shadow_view_matrix, const matrix* light_matrix)
{
}

void gr_stub_shadow_map_end()
{
}

void gr_stub_render_shield_impact(shield_material *material_info, primitive_type prim_type, vertex_layout *layout, int buffer_handle, int n_verts)
{

}

void gr_stub_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi)
{

}

void gr_stub_render_primitives(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle)
{

}

void gr_stub_render_primitives_immediate(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size)
{

}

void gr_stub_render_primitives_2d(material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle)
{

}

void gr_stub_render_primitives_2d_immediate(material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size)
{

}

void gr_stub_render_primitives_particle(particle_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle)
{

}

void gr_stub_render_primitives_distortion(distortion_material* material_info, primitive_type prim_type, vertex_layout* layout, int offset, int n_verts, int buffer_handle)
{

}
void gr_stub_render_movie(movie_material* material_info, primitive_type prim_type, vertex_layout* layout, int n_verts, int buffer)
{
}

bool gr_stub_is_capable(gr_capability capability)
{
	return false;
}

void gr_stub_push_debug_group(const char*){
}

void gr_stub_pop_debug_group(){
}

int gr_stub_create_query_object()
{
	return -1;
}

void gr_stub_query_value(int obj, QueryType type)
{
}

bool gr_stub_query_value_available(int obj)
{
	return false;
}

std::uint64_t gr_stub_get_query_value(int obj)
{
	return 0;
}

void gr_stub_delete_query_object(int obj)
{
}

bool gr_stub_init() 
{
	if (gr_screen.res != GR_640) {
		gr_screen.res = GR_640;
		gr_screen.max_w = 640;
		gr_screen.max_h = 480;
	}

	Gr_red.bits = 8;
	Gr_red.shift = 16;
	Gr_red.scale = 1;
	Gr_red.mask = 0xff0000;
	Gr_t_red = Gr_red;

	Gr_green.bits = 8;
	Gr_green.shift = 8;
	Gr_green.scale = 1;
	Gr_green.mask = 0xff00;
	Gr_t_green = Gr_green;

	Gr_blue.bits = 8;
	Gr_blue.shift = 0;
	Gr_blue.scale = 1;
	Gr_blue.mask = 0xff;
	Gr_t_blue = Gr_blue;

	// function pointers...
	gr_screen.gf_flip				= gr_stub_flip;
	gr_screen.gf_set_clip			= gr_stub_set_clip;
	gr_screen.gf_reset_clip			= gr_stub_reset_clip;
	
	gr_screen.gf_clear				= gr_stub_clear;

//	gr_screen.gf_rect				= gr_stub_rect;
//	gr_screen.gf_shade				= gr_stub_shade;

	gr_screen.gf_print_screen		= gr_stub_print_screen;

	gr_screen.gf_zbuffer_get		= gr_stub_zbuffer_get;
	gr_screen.gf_zbuffer_set		= gr_stub_zbuffer_set;
	gr_screen.gf_zbuffer_clear		= gr_stub_zbuffer_clear;

	gr_screen.gf_stencil_set		= gr_stub_stencil_set;
	gr_screen.gf_stencil_clear		= gr_stub_stencil_clear;

	gr_screen.gf_alpha_mask_set		= gr_stub_alpha_mask_set;
	
	gr_screen.gf_save_screen		= gr_stub_save_screen;
	gr_screen.gf_restore_screen		= gr_stub_restore_screen;
	gr_screen.gf_free_screen		= gr_stub_free_screen;
	
	gr_screen.gf_set_gamma			= gr_stub_set_gamma;

	gr_screen.gf_fog_set			= gr_stub_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region			= gr_stub_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data			= gr_stub_bm_free_data;
	gr_screen.gf_bm_create				= gr_stub_bm_create;
	gr_screen.gf_bm_init				= gr_stub_bm_init;
	gr_screen.gf_bm_page_in_start		= gr_stub_bm_page_in_start;
	gr_screen.gf_bm_data				= gr_stub_bm_data;
	gr_screen.gf_bm_make_render_target	= gr_stub_bm_make_render_target;
	gr_screen.gf_bm_set_render_target	= gr_stub_bm_set_render_target;

	gr_screen.gf_set_cull			= gr_stub_set_cull;
	gr_screen.gf_set_color_buffer	= gr_stub_set_color_buffer;

	gr_screen.gf_set_clear_color	= gr_stub_set_clear_color;

	gr_screen.gf_preload			= gr_stub_preload;

	gr_screen.gf_push_texture_matrix		= gr_stub_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix			= gr_stub_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix	= gr_stub_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing	= gr_stub_set_texture_addressing;
	gr_screen.gf_zbias					= gr_stub_zbias_stub;
	gr_screen.gf_set_fill_mode			= gr_set_fill_mode_stub;
	gr_screen.gf_set_texture_panning	= gr_stub_set_texture_panning;

	gr_screen.gf_create_vertex_buffer	= gr_stub_create_vertex_buffer;
	gr_screen.gf_create_index_buffer	= gr_stub_create_index_buffer;
	gr_screen.gf_delete_buffer		= gr_stub_delete_buffer;

	gr_screen.gf_update_transform_buffer	= gr_stub_update_transform_buffer;
	gr_screen.gf_update_buffer_data		= gr_stub_update_buffer_data;
	gr_screen.gf_set_transform_buffer_offset	= gr_stub_set_transform_buffer_offset;

	gr_screen.gf_render_stream_buffer		= gr_stub_render_stream_buffer;

	gr_screen.gf_start_instance_matrix			= gr_stub_start_instance_matrix;
	gr_screen.gf_end_instance_matrix			= gr_stub_end_instance_matrix;
	gr_screen.gf_start_angles_instance_matrix	= gr_stub_start_instance_angles;

	gr_screen.gf_set_light			= gr_stub_set_light;
	gr_screen.gf_reset_lighting		= gr_stub_reset_lighting;
	gr_screen.gf_set_ambient_light	= gr_stub_set_ambient_light;

	gr_screen.gf_post_process_set_effect	= gr_stub_post_process_set_effect;
	gr_screen.gf_post_process_set_defaults	= gr_stub_post_process_set_defaults;

	gr_screen.gf_post_process_begin		= gr_stub_post_process_begin;
	gr_screen.gf_post_process_end		= gr_stub_post_process_end;
	gr_screen.gf_post_process_save_zbuffer	= gr_stub_post_process_save_zbuffer;
	gr_screen.gf_post_process_restore_zbuffer = [](){};

	gr_screen.gf_scene_texture_begin = gr_stub_scene_texture_begin;
	gr_screen.gf_scene_texture_end = gr_stub_scene_texture_end;
	gr_screen.gf_copy_effect_texture = gr_stub_copy_effect_texture;

	gr_screen.gf_deferred_lighting_begin = gr_stub_deferred_lighting_begin;
	gr_screen.gf_deferred_lighting_end = gr_stub_deferred_lighting_end;
	gr_screen.gf_deferred_lighting_finish = gr_stub_deferred_lighting_finish;

	gr_screen.gf_start_clip_plane	= gr_stub_start_clip_plane;
	gr_screen.gf_end_clip_plane		= gr_stub_end_clip_plane;

	gr_screen.gf_lighting			= gr_stub_set_lighting;

	gr_screen.gf_set_proj_matrix	= gr_stub_set_projection_matrix;
	gr_screen.gf_end_proj_matrix	= gr_stub_end_projection_matrix;

	gr_screen.gf_set_view_matrix	= gr_stub_set_view_matrix;
	gr_screen.gf_end_view_matrix	= gr_stub_end_view_matrix;

	gr_screen.gf_push_scale_matrix	= gr_stub_push_scale_matrix;
	gr_screen.gf_pop_scale_matrix	= gr_stub_pop_scale_matrix;
	
	gr_screen.gf_set_line_width		= gr_stub_set_line_width;

	gr_screen.gf_sphere				= gr_stub_draw_sphere;

	gr_screen.gf_shadow_map_start	= gr_stub_shadow_map_start;
	gr_screen.gf_shadow_map_end		= gr_stub_shadow_map_end;

	gr_screen.gf_render_shield_impact = gr_stub_render_shield_impact;

	gr_screen.gf_maybe_create_shader = gr_stub_maybe_create_shader;
	
	gr_screen.gf_clear_states	= gr_stub_clear_states;
	
	gr_screen.gf_update_texture = gr_stub_update_texture;
	gr_screen.gf_get_bitmap_from_texture = gr_stub_get_bitmap_from_texture;

	gr_screen.gf_render_model = gr_stub_render_model;
	gr_screen.gf_render_primitives	= gr_stub_render_primitives;
	gr_screen.gf_render_primitives_immediate = gr_stub_render_primitives_immediate;
	gr_screen.gf_render_primitives_2d	= gr_stub_render_primitives_2d;
	gr_screen.gf_render_primitives_2d_immediate = gr_stub_render_primitives_2d_immediate;
	gr_screen.gf_render_primitives_particle	= gr_stub_render_primitives_particle;
	gr_screen.gf_render_primitives_distortion = gr_stub_render_primitives_distortion;
	gr_screen.gf_render_movie = gr_stub_render_movie;

	gr_screen.gf_is_capable = gr_stub_is_capable;

	gr_screen.gf_push_debug_group = gr_stub_push_debug_group;
	gr_screen.gf_pop_debug_group = gr_stub_pop_debug_group;

	gr_screen.gf_create_query_object = gr_stub_create_query_object;
	gr_screen.gf_query_value = gr_stub_query_value;
	gr_screen.gf_query_value_available = gr_stub_query_value_available;
	gr_screen.gf_get_query_value = gr_stub_get_query_value;
	gr_screen.gf_delete_query_object = gr_stub_delete_query_object;

	gr_screen.gf_create_viewport = [](const os::ViewPortProperties& props) {
		return std::unique_ptr<os::Viewport>();
	};
	gr_screen.gf_use_viewport = [](os::Viewport*) {
	};

	return true;
}
