#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::vulkan {

/**
 * @brief Adds what FSR needs to the device about to be created, if FSR can run on it
 *
 * Must run after the create info's pNext chain is final, and the create info must not outlive the process-lifetime
 * storage this keeps for it. Failure only means FSR stays unavailable; the renderer carries on without it.
 */
void vulkan_upscaler_prepare_device(vk::Instance instance,
	vk::PhysicalDevice gpu,
	uint32_t graphicsQueueFamily,
	const vk::PhysicalDeviceFeatures& supportedFeatures,
	vk::DeviceCreateInfo& createInfo);

// Looks up which FSR version this device gets, for display.
void vulkan_upscaler_device_created(vk::Device device);

bool vulkan_upscaler_available();

} // namespace graphics::vulkan
