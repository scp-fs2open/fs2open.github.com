
#include "VulkanRenderer.h"


#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"

namespace graphics::vulkan {


void VulkanRenderer::createImGuiDescriptorPool()
{
	vk::DescriptorPoolSize poolSize;
	poolSize.type = vk::DescriptorType::eCombinedImageSampler;
	poolSize.descriptorCount = 100;

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	poolInfo.maxSets = 100;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	m_imguiDescriptorPool = m_device->createDescriptorPoolUnique(poolInfo);
}

void VulkanRenderer::initImGui()
{
	createImGuiDescriptorPool();

	// Load Vulkan function pointers for imgui (required with VK_NO_PROTOTYPES)
	auto vkInstance = static_cast<VkInstance>(*m_vkInstance);
	ImGui_ImplVulkan_LoadFunctions(0, [](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
		return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
			static_cast<VkInstance>(user_data), function_name);
	}, vkInstance);

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = static_cast<VkInstance>(*m_vkInstance);
	initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(m_physicalDevice);
	initInfo.Device = static_cast<VkDevice>(*m_device);
	initInfo.QueueFamily = m_graphicsQueueFamilyIndex;
	initInfo.Queue = static_cast<VkQueue>(m_graphicsQueue);
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = static_cast<VkDescriptorPool>(*m_imguiDescriptorPool);
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(m_swapChainImages.size());
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;
	initInfo.PipelineInfoMain.Subpass = 0;
	initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.PipelineInfoMain.RenderPass = static_cast<VkRenderPass>(*m_renderPass);

	ImGui_ImplVulkan_Init(&initInfo);

	nprintf(("vulkan", "Vulkan: ImGui backend initialized successfully\n"));
}

void VulkanRenderer::shutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	m_imguiDescriptorPool.reset();
	nprintf(("vulkan", "Vulkan: ImGui backend shut down\n"));
}

} // namespace graphics::vulkan
