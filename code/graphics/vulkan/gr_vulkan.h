#pragma once

#include "osapi/osapi.h"

namespace graphics::vulkan {

class VulkanRenderer;

void initialize_function_pointers();
bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);

VulkanRenderer* getRendererInstance();

void cleanup();

struct HardwareQuery {
	bool raytraced_shadows = false;
	float max_anisotropy = 0.0f;
};

// -get_options runs without a renderer, so a launcher would otherwise never see the hardware-dependent choices
bool query_hardware_without_window(HardwareQuery& out);

} // namespace graphics::vulkan
