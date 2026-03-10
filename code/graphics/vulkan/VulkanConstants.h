#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Return the correct image aspect flags for a given format:
//   depth-only  → eDepth
//   depth+stencil → eDepth | eStencil
//   everything else → eColor
inline vk::ImageAspectFlags imageAspectFromFormat(vk::Format format)
{
	switch (format) {
		case vk::Format::eD16Unorm:
		case vk::Format::eD32Sfloat:
			return vk::ImageAspectFlagBits::eDepth;
		case vk::Format::eD16UnormS8Uint:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32SfloatS8Uint:
			return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		default:
			return vk::ImageAspectFlagBits::eColor;
	}
}

} // namespace graphics::vulkan

