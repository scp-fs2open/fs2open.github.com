#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"

struct matrix;
struct matrix4;
struct vec3d;

namespace graphics {
namespace vulkan {

// Deferred lighting pipeline entry points (gr_screen.gf_* implementations)
void vulkan_deferred_lighting_begin(bool clearNonColorBufs);
void vulkan_deferred_lighting_msaa();
void vulkan_deferred_lighting_end();
void vulkan_deferred_lighting_finish();

// Fog control
void vulkan_override_fog(bool set_override);

// Shadow map rendering
void vulkan_shadow_map_start(matrix4* shadow_view_matrix, const matrix* light_matrix, vec3d* eye_pos);
void vulkan_shadow_map_end();

// Decal pass
void vulkan_start_decal_pass();
void vulkan_stop_decal_pass();
void vulkan_render_decals(decal_material* material_info,
                          primitive_type prim_type,
                          vertex_layout* layout,
                          int num_elements,
                          const indexed_vertex_source& buffers,
                          const gr_buffer_handle& instance_buffer,
                          int num_instances);

} // namespace vulkan
} // namespace graphics
