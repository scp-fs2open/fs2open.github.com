#pragma once

#include "osapi/osapi.h"

namespace graphics::vulkan {

class VulkanRenderer;

void initialize_function_pointers();
bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

// Build (or refresh) this frame's raytraced-shadow/RTAO TLAS. Safe to call more
// than once per frame (VulkanRaytracingManager::buildTlas() is frame-guarded),
// but acceleration-structure builds must be recorded outside a render pass, so
// this ends the state tracker's current render pass (and clears it in the
// tracker) if one is active.
void vulkan_build_shadow_tlas();

void cleanup();

} // namespace graphics::vulkan
