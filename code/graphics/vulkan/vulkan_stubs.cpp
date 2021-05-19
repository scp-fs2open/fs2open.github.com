#include "vulkan_stubs.h"

#include "graphics/2d.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

namespace graphics {
namespace vulkan {

namespace {

gr_buffer_handle stub_create_buffer(BufferType, BufferUsageHint)
{
	return gr_buffer_handle::invalid();
}

void stub_delete_buffer(gr_buffer_handle /*handle*/) {}

int stub_preload(int /*bitmap_num*/, int /*is_aabitmap*/) { return 0; }

int stub_save_screen() { return 1; }

int stub_zbuffer_get() { return 0; }

int stub_zbuffer_set(int /*mode*/) { return 0; }

void gr_set_fill_mode_stub(int /*mode*/) {}

void stub_clear() {}

void stub_free_screen(int /*id*/) {}

void stub_get_region(int /*front*/, int /*w*/, int /*h*/, ubyte* /*data*/) {}

void stub_print_screen(const char* /*filename*/) {}

void stub_reset_clip() {}

void stub_restore_screen(int /*id*/) {}

void stub_update_buffer_data(gr_buffer_handle /*handle*/, size_t /*size*/, const void* /*data*/) {}

void stub_update_buffer_data_offset(gr_buffer_handle /*handle*/,
	size_t /*offset*/,
	size_t /*size*/,
	const void* /*data*/)
{
}

void stub_update_transform_buffer(void* /*data*/, size_t /*size*/) {}

void stub_set_clear_color(int /*r*/, int /*g*/, int /*b*/) {}

void stub_set_clip(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*resize_mode*/) {}

int stub_set_cull(int /*cull*/) { return 0; }

int stub_set_color_buffer(int /*mode*/) { return 0; }

void stub_set_texture_addressing(int /*mode*/) {}

void stub_zbias_stub(int /*bias*/) {}

void stub_zbuffer_clear(int /*mode*/) {}

int stub_stencil_set(int /*mode*/) { return 0; }

void stub_stencil_clear() {}

int stub_alpha_mask_set(int /*mode*/, float /*alpha*/) { return 0; }

void stub_post_process_set_effect(const char* /*name*/, int /*x*/, const vec3d* /*rgb*/) {}

void stub_post_process_set_defaults() {}

void stub_post_process_save_zbuffer() {}

void stub_post_process_begin() {}

void stub_post_process_end() {}

void stub_scene_texture_begin() {}

void stub_scene_texture_end() {}

void stub_copy_effect_texture() {}

void stub_deferred_lighting_begin() {}

void stub_deferred_lighting_end() {}

void stub_deferred_lighting_finish() {}

void stub_set_line_width(float /*width*/) {}

void stub_draw_sphere(material* /*material_def*/, float /*rad*/) {}

void stub_clear_states() {}

void stub_update_texture(int /*bitmap_handle*/, int /*bpp*/, const ubyte* /*data*/, int /*width*/, int /*height*/) {}

void stub_get_bitmap_from_texture(void* /*data_out*/, int /*bitmap_num*/) {}

int stub_bm_make_render_target(int /*n*/, int* /*width*/, int* /*height*/, int* /*bpp*/, int* /*mm_lvl*/, int /*flags*/)
{
	return 0;
}

int stub_bm_set_render_target(int /*n*/, int /*face*/) { return 0; }

void stub_bm_create(bitmap_slot* /*slot*/) {}

void stub_bm_free_data(bitmap_slot* /*slot*/, bool /*release*/) {}

void stub_bm_init(bitmap_slot* /*slot*/) {}

void stub_bm_page_in_start() {}

bool stub_bm_data(int /*n*/, bitmap* /*bm*/) { return true; }

int stub_maybe_create_shader(shader_type /*shader_t*/, unsigned int /*flags*/) { return -1; }

void stub_shadow_map_start(matrix4* /*shadow_view_matrix*/, const matrix* /*light_matrix*/, vec3d* /*eye_pos*/) {}

void stub_shadow_map_end() {}

void stub_render_shield_impact(shield_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	gr_buffer_handle /*buffer_handle*/,
	int /*n_verts*/)
{
}

void stub_render_model(model_material* /*material_info*/,
	indexed_vertex_source* /*vert_source*/,
	vertex_buffer* /*bufferp*/,
	size_t /*texi*/)
{
}

void stub_render_primitives(material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/,
	size_t /*buffer_offset*/)
{
}

void stub_render_primitives_particle(particle_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void stub_render_primitives_distortion(distortion_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}
void stub_render_movie(movie_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer*/,
	size_t /*buffer_offset*/)
{
}

void stub_render_nanovg(nanovg_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void stub_render_primitives_batched(batched_bitmap_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*offset*/,
	int /*n_verts*/,
	gr_buffer_handle /*buffer_handle*/)
{
}

void stub_render_rocket_primitives(interface_material* /*material_info*/,
	primitive_type /*prim_type*/,
	vertex_layout* /*layout*/,
	int /*n_indices*/,
	gr_buffer_handle /*vertex_buffer*/,
	gr_buffer_handle /*index_buffer*/)
{
}

bool stub_is_capable(gr_capability /*capability*/) { return false; }
bool stub_get_property(gr_property p, void* dest)
{
	if (p == gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT) {
		// This is required by the startup code of the uniform buffer manager
		*reinterpret_cast<int*>(dest) = 4;
		return true;
	}
	return false;
};

void stub_push_debug_group(const char*) {}

void stub_pop_debug_group() {}

int stub_create_query_object() { return -1; }

void stub_query_value(int /*obj*/, QueryType /*type*/) {}

bool stub_query_value_available(int /*obj*/) { return false; }

std::uint64_t stub_get_query_value(int /*obj*/) { return 0; }

void stub_delete_query_object(int /*obj*/) {}

} // namespace

void init_stub_pointers()
{
	// function pointers...
	gr_screen.gf_set_clip = stub_set_clip;
	gr_screen.gf_reset_clip = stub_reset_clip;

	gr_screen.gf_clear = stub_clear;

	gr_screen.gf_print_screen = stub_print_screen;

	gr_screen.gf_zbuffer_get = stub_zbuffer_get;
	gr_screen.gf_zbuffer_set = stub_zbuffer_set;
	gr_screen.gf_zbuffer_clear = stub_zbuffer_clear;

	gr_screen.gf_stencil_set = stub_stencil_set;
	gr_screen.gf_stencil_clear = stub_stencil_clear;

	gr_screen.gf_alpha_mask_set = stub_alpha_mask_set;

	gr_screen.gf_save_screen = stub_save_screen;
	gr_screen.gf_restore_screen = stub_restore_screen;
	gr_screen.gf_free_screen = stub_free_screen;

	gr_screen.gf_get_region = stub_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data = stub_bm_free_data;
	gr_screen.gf_bm_create = stub_bm_create;
	gr_screen.gf_bm_init = stub_bm_init;
	gr_screen.gf_bm_page_in_start = stub_bm_page_in_start;
	gr_screen.gf_bm_data = stub_bm_data;
	gr_screen.gf_bm_make_render_target = stub_bm_make_render_target;
	gr_screen.gf_bm_set_render_target = stub_bm_set_render_target;

	gr_screen.gf_set_cull = stub_set_cull;
	gr_screen.gf_set_color_buffer = stub_set_color_buffer;

	gr_screen.gf_set_clear_color = stub_set_clear_color;

	gr_screen.gf_preload = stub_preload;

	gr_screen.gf_set_texture_addressing = stub_set_texture_addressing;
	gr_screen.gf_zbias = stub_zbias_stub;
	gr_screen.gf_set_fill_mode = gr_set_fill_mode_stub;

	gr_screen.gf_create_buffer = stub_create_buffer;
	gr_screen.gf_delete_buffer = stub_delete_buffer;

	gr_screen.gf_update_transform_buffer = stub_update_transform_buffer;
	gr_screen.gf_update_buffer_data = stub_update_buffer_data;
	gr_screen.gf_update_buffer_data_offset = stub_update_buffer_data_offset;
	gr_screen.gf_map_buffer = [](gr_buffer_handle) -> void* { return nullptr; };
	gr_screen.gf_flush_mapped_buffer = [](gr_buffer_handle, size_t, size_t) {};

	gr_screen.gf_post_process_set_effect = stub_post_process_set_effect;
	gr_screen.gf_post_process_set_defaults = stub_post_process_set_defaults;

	gr_screen.gf_post_process_begin = stub_post_process_begin;
	gr_screen.gf_post_process_end = stub_post_process_end;
	gr_screen.gf_post_process_save_zbuffer = stub_post_process_save_zbuffer;
	gr_screen.gf_post_process_restore_zbuffer = []() {};

	gr_screen.gf_scene_texture_begin = stub_scene_texture_begin;
	gr_screen.gf_scene_texture_end = stub_scene_texture_end;
	gr_screen.gf_copy_effect_texture = stub_copy_effect_texture;

	gr_screen.gf_deferred_lighting_begin = stub_deferred_lighting_begin;
	gr_screen.gf_deferred_lighting_end = stub_deferred_lighting_end;
	gr_screen.gf_deferred_lighting_finish = stub_deferred_lighting_finish;

	gr_screen.gf_set_line_width = stub_set_line_width;

	gr_screen.gf_sphere = stub_draw_sphere;

	gr_screen.gf_shadow_map_start = stub_shadow_map_start;
	gr_screen.gf_shadow_map_end = stub_shadow_map_end;

	gr_screen.gf_render_shield_impact = stub_render_shield_impact;

	gr_screen.gf_maybe_create_shader = stub_maybe_create_shader;

	gr_screen.gf_clear_states = stub_clear_states;

	gr_screen.gf_update_texture = stub_update_texture;
	gr_screen.gf_get_bitmap_from_texture = stub_get_bitmap_from_texture;

	gr_screen.gf_render_model = stub_render_model;
	gr_screen.gf_render_primitives = stub_render_primitives;
	gr_screen.gf_render_primitives_particle = stub_render_primitives_particle;
	gr_screen.gf_render_primitives_distortion = stub_render_primitives_distortion;
	gr_screen.gf_render_movie = stub_render_movie;
	gr_screen.gf_render_nanovg = stub_render_nanovg;
	gr_screen.gf_render_primitives_batched = stub_render_primitives_batched;
	gr_screen.gf_render_rocket_primitives = stub_render_rocket_primitives;

	gr_screen.gf_is_capable = stub_is_capable;
	gr_screen.gf_get_property = stub_get_property;

	gr_screen.gf_push_debug_group = stub_push_debug_group;
	gr_screen.gf_pop_debug_group = stub_pop_debug_group;

	gr_screen.gf_create_query_object = stub_create_query_object;
	gr_screen.gf_query_value = stub_query_value;
	gr_screen.gf_query_value_available = stub_query_value_available;
	gr_screen.gf_get_query_value = stub_get_query_value;
	gr_screen.gf_delete_query_object = stub_delete_query_object;

	gr_screen.gf_create_viewport = [](const os::ViewPortProperties&) { return std::unique_ptr<os::Viewport>(); };
	gr_screen.gf_use_viewport = [](os::Viewport*) {};

	gr_screen.gf_bind_uniform_buffer = [](uniform_block_type, size_t, size_t, gr_buffer_handle) {};

	gr_screen.gf_sync_fence = []() -> gr_sync { return nullptr; };
	gr_screen.gf_sync_wait = [](gr_sync /*sync*/, uint64_t /*timeoutns*/) { return true; };
	gr_screen.gf_sync_delete = [](gr_sync /*sync*/) {};

	gr_screen.gf_set_viewport = [](int /*x*/, int /*y*/, int /*width*/, int /*height*/) {};
}

} // namespace vulkan
} // namespace graphics
