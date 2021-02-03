
#include "gr_vulkan.h"

#include "VulkanRenderer.h"
#include "vulkan_stubs.h"

#include "mod_table/mod_table.h"

namespace graphics {
namespace vulkan {

namespace {
std::unique_ptr<VulkanRenderer> renderer_instance;
}

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps)
{
	renderer_instance.reset(new VulkanRenderer(std::move(graphicsOps)));
	if (!renderer_instance->initialize()) {
		return false;
	}

	init_stub_pointers();

	gr_screen.gf_flip = []() {
		renderer_instance->flip();
	};

	// Nothing else is finished so always fail here
	mprintf(("Vulkan support is not finished yet so graphics initialization will always fail...\n"));
	return true;
}

VulkanRenderer* getRendererInstance()
{
	return renderer_instance.get();
}

void cleanup()
{
	renderer_instance->shutdown();
	renderer_instance = nullptr;
}

} // namespace vulkan
} // namespace graphics
