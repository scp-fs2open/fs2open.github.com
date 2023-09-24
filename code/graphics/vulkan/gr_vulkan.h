#pragma once

#include "osapi/osapi.h"

namespace graphics {
namespace vulkan {

class VulkanRenderer;

void initialize_function_pointers();
bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

void cleanup();

} // namespace vulkan
} // namespace graphics
