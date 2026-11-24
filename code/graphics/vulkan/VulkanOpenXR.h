#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::vulkan {

/**
 * @brief OpenXR support for the Vulkan backend
 *
 * Every line of Vulkan code that knows about OpenXR lives in VulkanOpenXR.cpp.
 * The rest of the backend only sees the four entry points below, none of which
 * mention OpenXR in their signatures.
 *
 * The three creation helpers exist because XR_KHR_vulkan_enable2 does not let
 * the application create its own instance and device: the runtime has to create
 * them (xrCreateVulkanInstanceKHR / xrCreateVulkanDeviceKHR) so it can inject
 * the extensions it needs, and it dictates which physical device to use. That
 * happens during VulkanRenderer::initialize(), long before openxr_init() runs,
 * so the renderer's normal creation calls are routed through here instead of
 * being duplicated behind an "is VR on?" branch at each site.
 *
 * When VR was not requested -- or the build has no OpenXR support at all --
 * each helper performs exactly the plain Vulkan call it replaced.
 */

/**
 * @brief Create the Vulkan instance, via OpenXR when a VR session is coming
 */
vk::UniqueInstance vulkan_openxr_create_instance(const vk::InstanceCreateInfo& createInfo);

/**
 * @brief The physical device a VR session requires, or a null handle if any device will do
 */
vk::PhysicalDevice vulkan_openxr_required_physical_device(vk::Instance instance);

/**
 * @brief Create the logical device, via OpenXR when a VR session is coming
 */
vk::UniqueDevice vulkan_openxr_create_device(vk::PhysicalDevice physicalDevice, const vk::DeviceCreateInfo& createInfo);

SCP_vector<const char*> vulkan_openxr_get_extensions();

bool vulkan_openxr_test_capabilities();

bool vulkan_openxr_create_session();

int64_t vulkan_openxr_get_swapchain_format(const SCP_vector<int64_t>& allowed);

bool vulkan_openxr_acquire_swapchain_buffers();

bool vulkan_openxr_flip();

} // namespace graphics::vulkan
