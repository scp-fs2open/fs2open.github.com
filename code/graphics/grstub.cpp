

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

gr_buffer_handle gr_stub_create_buffer(BufferType, BufferUsageHint)
{
	return {};
}

void gr_stub_delete_buffer(gr_buffer_handle /*handle*/) {}

int gr_stub_preload(int  /*bitmap_num*/, int  /*is_aabitmap*/)
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

int gr_stub_zbuffer_set(int  /*mode*/)
{
	return 0;
}

void gr_set_fill_mode_stub(int  /*mode*/)
{
}

void gr_stub_activate(int  /*active*/)
{
}


void gr_stub_cleanup(int  /*minimize*/)
{
}

void gr_stub_clear()
{
}

void gr_stub_flip()
{
}

void gr_stub_fog_set(int  /*fog_mode*/, int  /*r*/, int  /*g*/, int  /*b*/, float  /*fog_near*/, float  /*fog_far*/)
{
}

void gr_stub_free_screen(int  /*id*/)
{
}

void gr_stub_get_region(int  /*front*/, int  /*w*/, int  /*h*/, ubyte * /*data*/)
{
}

void gr_stub_preload_init()
{
}

void gr_stub_print_screen(const char * /*filename*/)
{
}

SCP_string gr_stub_blob_screen()
{
	return "";
}

void gr_stub_rect(int  /*x*/, int  /*y*/, int  /*w*/, int  /*h*/, int  /*resize_mode*/)
{
}

void gr_stub_reset_clip()
{
}

void gr_stub_restore_screen(int  /*id*/)
{
}

void gr_stub_save_mouse_area(int  /*x*/, int  /*y*/, int  /*w*/, int  /*h*/)
{
}

void gr_stub_update_buffer_data(gr_buffer_handle /*handle*/, size_t /*size*/, const void* /*data*/) {}

void gr_stub_update_buffer_data_offset(gr_buffer_handle /*handle*/,
	size_t /*offset*/,
	size_t /*size*/,
	const void* /*data*/)
{
}

void gr_stub_update_transform_buffer(void*  /*data*/, size_t  /*size*/)
{

}

void gr_stub_set_clear_color(int  /*r*/, int  /*g*/, int  /*b*/)
{
}

void gr_stub_set_clip(int  /*x*/, int  /*y*/, int  /*w*/, int  /*h*/, int  /*resize_mode*/)
{
}

int gr_stub_set_cull(int  /*cull*/)
{
	return 0;
}

int gr_stub_set_color_buffer(int  /*mode*/)
{
	return 0;
}

void gr_stub_set_tex_env_scale(float  /*scale*/)
{
}

void gr_stub_set_texture_addressing(int  /*mode*/)
{
}

void gr_stub_stuff_fog_coord(vertex * /*v*/)
{
}

void gr_stub_stuff_secondary_color(vertex * /*v*/, ubyte  /*fr*/, ubyte  /*fg*/, ubyte  /*fb*/)
{
}

void gr_stub_zbias_stub(int  /*bias*/)
{
}

void gr_stub_zbuffer_clear(int  /*mode*/)
{
}

int gr_stub_stencil_set(int  /*mode*/)
{
	return 0;
}

void gr_stub_stencil_clear()
{
}

int gr_stub_alpha_mask_set(int  /*mode*/, float  /*alpha*/)
{
	return 0;
}

/*void gr_stub_shade(int x,int y,int w,int h)
{
}*/

void gr_stub_post_process_set_effect(const char * /*name*/, int  /*x*/, const vec3d * /*rgb*/)
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

void gr_stub_deferred_lighting_begin(bool /*clearNonColorBufs*/)
{
}

void gr_stub_deferred_lighting_end()
{
}

void gr_stub_deferred_lighting_finish()
{
}

void gr_stub_set_line_width(float  /*width*/)
{
}

void gr_stub_draw_sphere(material * /*material_def*/, float  /*rad*/)
{
}

void gr_stub_clear_states()
{
}

void gr_stub_update_texture(int  /*bitmap_handle*/, int  /*bpp*/, const ubyte * /*data*/, int  /*width*/, int  /*height*/)
{
}

void gr_stub_get_bitmap_from_texture(void*  /*data_out*/, int  /*bitmap_num*/)
{

}

int gr_stub_bm_make_render_target(int  /*n*/, int * /*width*/, int * /*height*/, int * /*bpp*/, int * /*mm_lvl*/, int  /*flags*/)
{
	return 0;
}

int gr_stub_bm_set_render_target(int  /*n*/, int  /*face*/)
{
	return 0;
}

void gr_stub_bm_create(bitmap_slot* /*slot*/)
{
}

void gr_stub_bm_free_data(bitmap_slot* /*slot*/, bool /*release*/)
{
}

void gr_stub_bm_init(bitmap_slot* /*slot*/)
{
}

void gr_stub_bm_page_in_start()
{
}

bool gr_stub_bm_data(int  /*n*/, bitmap*  /*bm*/)
{
	return true;
}

int gr_stub_maybe_create_shader(shader_type  /*shader_t*/, unsigned int  /*flags*/) {
	return -1;
}

void gr_stub_shadow_map_start(matrix4 * /*shadow_view_matrix*/, const matrix*  /*light_matrix*/, vec3d* /*eye_pos*/)
{
}

void gr_stub_shadow_map_end()
{
}

void gr_stub_render_shield_impact(shield_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	gr_buffer_handle /*buffer_handle*/,
	int /*n_verts*/)
{
}

void gr_stub_render_model(model_material*  /*material_info*/, indexed_vertex_source * /*vert_source*/, vertex_buffer*  /*bufferp*/, size_t  /*texi*/)
{

}

void gr_stub_render_primitives(material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/,
	size_t /*buffer_offset*/)
{
}

void gr_stub_render_primitives_particle(particle_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void gr_stub_render_primitives_distortion(distortion_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}
void gr_stub_render_movie(movie_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer*/,
	size_t /*buffer_offset*/)
{
}

void gr_stub_render_nanovg(nanovg_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void gr_stub_render_primitives_batched(batched_bitmap_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void gr_stub_render_rocket_primitives(interface_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*n_indices*/,
	gr_buffer_handle /*vertex_buffer*/,
	gr_buffer_handle /*index_buffer*/)
{
}

bool gr_stub_is_capable(gr_capability  /*capability*/)
{
	return false;
}
bool gr_stub_get_property(gr_property p, void* dest)
{
	if (p == gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT) {
		// This is required by the startup code of the uniform buffer manager
		*reinterpret_cast<int*>(dest) = 4;
		return true;
	}
	return false;
};

void gr_stub_push_debug_group(const char*){
}

void gr_stub_pop_debug_group(){
}

int gr_stub_create_query_object()
{
	return -1;
}

void gr_stub_query_value(int  /*obj*/, QueryType  /*type*/)
{
}

bool gr_stub_query_value_available(int  /*obj*/)
{
	return false;
}

std::uint64_t gr_stub_get_query_value(int  /*obj*/)
{
	return 0;
}

void gr_stub_delete_query_object(int  /*obj*/)
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
	gr_screen.gf_blob_screen		= gr_stub_blob_screen;

	gr_screen.gf_zbuffer_get		= gr_stub_zbuffer_get;
	gr_screen.gf_zbuffer_set		= gr_stub_zbuffer_set;
	gr_screen.gf_zbuffer_clear		= gr_stub_zbuffer_clear;

	gr_screen.gf_stencil_set		= gr_stub_stencil_set;
	gr_screen.gf_stencil_clear		= gr_stub_stencil_clear;

	gr_screen.gf_alpha_mask_set		= gr_stub_alpha_mask_set;
	
	gr_screen.gf_save_screen		= gr_stub_save_screen;
	gr_screen.gf_restore_screen		= gr_stub_restore_screen;
	gr_screen.gf_free_screen		= gr_stub_free_screen;

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

	gr_screen.gf_set_texture_addressing = gr_stub_set_texture_addressing;
	gr_screen.gf_zbias = gr_stub_zbias_stub;
	gr_screen.gf_set_fill_mode = gr_set_fill_mode_stub;

	gr_screen.gf_create_buffer = gr_stub_create_buffer;
	gr_screen.gf_delete_buffer = gr_stub_delete_buffer;

	gr_screen.gf_update_transform_buffer = gr_stub_update_transform_buffer;
	gr_screen.gf_update_buffer_data = gr_stub_update_buffer_data;
	gr_screen.gf_update_buffer_data_offset = gr_stub_update_buffer_data_offset;
	gr_screen.gf_map_buffer = [](gr_buffer_handle) -> void* { return nullptr; };
	gr_screen.gf_flush_mapped_buffer = [](gr_buffer_handle, size_t, size_t) {};

	gr_screen.gf_post_process_set_effect = gr_stub_post_process_set_effect;
	gr_screen.gf_post_process_set_defaults = gr_stub_post_process_set_defaults;

	gr_screen.gf_post_process_begin = gr_stub_post_process_begin;
	gr_screen.gf_post_process_end = gr_stub_post_process_end;
	gr_screen.gf_post_process_save_zbuffer = gr_stub_post_process_save_zbuffer;
	gr_screen.gf_post_process_restore_zbuffer = []() {};

	gr_screen.gf_scene_texture_begin = gr_stub_scene_texture_begin;
	gr_screen.gf_scene_texture_end = gr_stub_scene_texture_end;
	gr_screen.gf_copy_effect_texture = gr_stub_copy_effect_texture;

	gr_screen.gf_deferred_lighting_begin = gr_stub_deferred_lighting_begin;
	gr_screen.gf_deferred_lighting_end = gr_stub_deferred_lighting_end;
	gr_screen.gf_deferred_lighting_finish = gr_stub_deferred_lighting_finish;

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
	gr_screen.gf_render_primitives_particle	= gr_stub_render_primitives_particle;
	gr_screen.gf_render_primitives_distortion = gr_stub_render_primitives_distortion;
	gr_screen.gf_render_movie = gr_stub_render_movie;
	gr_screen.gf_render_nanovg = gr_stub_render_nanovg;
	gr_screen.gf_render_primitives_batched = gr_stub_render_primitives_batched;
	gr_screen.gf_render_rocket_primitives     = gr_stub_render_rocket_primitives;

	gr_screen.gf_is_capable = gr_stub_is_capable;
	gr_screen.gf_get_property = gr_stub_get_property;

	gr_screen.gf_push_debug_group = gr_stub_push_debug_group;
	gr_screen.gf_pop_debug_group = gr_stub_pop_debug_group;

	gr_screen.gf_create_query_object = gr_stub_create_query_object;
	gr_screen.gf_query_value = gr_stub_query_value;
	gr_screen.gf_query_value_available = gr_stub_query_value_available;
	gr_screen.gf_get_query_value = gr_stub_get_query_value;
	gr_screen.gf_delete_query_object = gr_stub_delete_query_object;

	gr_screen.gf_create_viewport = [](const os::ViewPortProperties&) { return std::unique_ptr<os::Viewport>(); };
	gr_screen.gf_use_viewport = [](os::Viewport*) {};

	gr_screen.gf_bind_uniform_buffer = [](uniform_block_type, size_t, size_t, gr_buffer_handle) {};

	gr_screen.gf_sync_fence = []() -> gr_sync { return nullptr; };
	gr_screen.gf_sync_wait = [](gr_sync /*sync*/, uint64_t /*timeoutns*/) { return true; };
	gr_screen.gf_sync_delete = [](gr_sync /*sync*/) {};

	gr_screen.gf_set_viewport = [](int /*x*/, int /*y*/, int /*width*/, int /*height*/) {};

	return true;
}
