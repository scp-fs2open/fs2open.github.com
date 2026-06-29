
#include "VulkanRenderer.h"


#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#endif


extern float flFrametime;

namespace graphics {
namespace vulkan {


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
	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
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
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32_t>(m_swapChainImages.size());
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo, static_cast<VkRenderPass>(*m_renderPass));

	// Upload font textures via one-time command buffer
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = m_graphicsCommandPool.get();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		auto cmdBuffers = m_device->allocateCommandBuffers(allocInfo);
		auto cmd = cmdBuffers.front();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cmd.begin(beginInfo);

		ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(cmd));

		cmd.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		m_graphicsQueue.submit(submitInfo, nullptr);
		m_graphicsQueue.waitIdle();

		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	mprintf(("Vulkan: ImGui backend initialized successfully\n"));
}

void VulkanRenderer::shutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	m_imguiDescriptorPool.reset();
	mprintf(("Vulkan: ImGui backend shut down\n"));
}

} // namespace vulkan
} // namespace graphics
