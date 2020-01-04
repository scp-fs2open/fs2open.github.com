
#include "gr_vulkan.h"

#include "VulkanRenderer.h"

#include "mod_table/mod_table.h"

namespace graphics {
namespace vulkan {

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps)
{
	VulkanRenderer renderer(std::move(graphicsOps));
	if (!renderer.initialize()) {
		return false;
	}

	// Nothing else is finished so always fail here
	mprintf(("Vulkan support is not finished yet so graphics initialization will always fail...\n"));
	return false;
}

} // namespace vulkan
} // namespace graphics
