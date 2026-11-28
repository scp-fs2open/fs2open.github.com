#pragma once

#include "osapi/osapi.h"

namespace graphics::vulkan {

class VulkanRenderer;

void initialize_function_pointers();
bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

// True when this frame needs a ray-tracing TLAS (see shadows_rt_scene_needed()) and no build
// for this frame occurred yet.
bool vulkan_frame_tlas_pending();

// Builds the ray-tracing TLAS for this frame. vulkan_scene_texture_begin() calls this once per
// frame, before the scene render pass starts. Call it only when no render pass is open, because
// acceleration-structure builds are not permitted inside a render pass.
void vulkan_build_frame_tlas();

void cleanup();

struct HardwareQuery {
	bool usable_device = false;
	bool raytraced_shadows = false;
	float max_anisotropy = 0.0f;
};

// -get_options runs without a renderer, so a launcher would otherwise never see the hardware-dependent choices.
// False if Vulkan could not be asked; true with usable_device unset if no card can run the Vulkan renderer.
bool query_hardware_without_window(HardwareQuery& out);

} // namespace graphics::vulkan
