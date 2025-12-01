#pragma once

#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "graphics/material.h"

namespace graphics {
namespace vulkan {

class VulkanRenderer;

void initialize_function_pointers();
bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

void cleanup();

// gr_screen function implementations
void gr_vulkan_setup_frame();
void gr_vulkan_scene_texture_begin();
void gr_vulkan_scene_texture_end();

// Render functions
void gr_vulkan_render_primitives(material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle,
    size_t buffer_offset);

void gr_vulkan_render_primitives_particle(particle_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle);

void gr_vulkan_render_primitives_batched(batched_bitmap_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle);

void gr_vulkan_render_primitives_distortion(distortion_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle);

void gr_vulkan_render_movie(movie_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int n_verts,
    gr_buffer_handle buffer,
    size_t buffer_offset);

void gr_vulkan_render_nanovg(nanovg_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int offset,
    int n_verts,
    gr_buffer_handle buffer_handle);

void gr_vulkan_render_decals(decal_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int num_elements,
    const indexed_vertex_source& binding,
    const gr_buffer_handle& instance_buffer,
    int num_instances);

void gr_vulkan_render_rocket_primitives(interface_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    int n_indices,
    gr_buffer_handle vertex_buffer,
    gr_buffer_handle index_buffer);

void gr_vulkan_render_model(model_material* material_info,
    indexed_vertex_source* vert_source,
    vertex_buffer* bufferp,
    size_t texi);

void gr_vulkan_render_shield_impact(shield_material* material_info,
    primitive_type prim_type,
    vertex_layout* layout,
    gr_buffer_handle buffer_handle,
    int n_verts);

// Capability and property queries
bool gr_vulkan_is_capable(gr_capability capability);
bool gr_vulkan_get_property(gr_property prop, void* dest);

// Shader management
int gr_vulkan_maybe_create_shader(shader_type type, unsigned int flags);

// Z-buffer control
int gr_vulkan_zbuffer_get();
int gr_vulkan_zbuffer_set(int mode);
void gr_vulkan_zbuffer_clear(int mode);

// Stencil control
int gr_vulkan_stencil_set(int mode);
void gr_vulkan_stencil_clear();

// Clipping
void gr_vulkan_set_clip(int x, int y, int w, int h, int resize_mode);
void gr_vulkan_reset_clip();

// Screen clearing
void gr_vulkan_clear();
void gr_vulkan_set_clear_color(int r, int g, int b);

// Culling
int gr_vulkan_set_cull(int cull);

// Color buffer
int gr_vulkan_set_color_buffer(int mode);

} // namespace vulkan
} // namespace graphics
