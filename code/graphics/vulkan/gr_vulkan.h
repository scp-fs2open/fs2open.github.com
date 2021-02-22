#pragma once

#include "osapi/osapi.h"

namespace graphics {
namespace vulkan {

class VulkanRenderer;

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

void cleanup();

} // namespace vulkan
} // namespace graphics
