#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

/**
 * @brief Lightweight non-owning view over a contiguous array (C++17 substitute for std::span)
 */
template<typename T>
struct ArrayView {
	const T* data = nullptr;
	size_t size = 0;

	constexpr ArrayView() = default;
	constexpr ArrayView(const T* data_, size_t size_) : data(data_), size(size_) {}

	template<size_t N>
	constexpr ArrayView(const T (&arr)[N]) : data(arr), size(N) {}

	template<size_t N>
	ArrayView(const std::array<T, N>& arr) : data(arr.data()), size(N) {}

	const T* begin() const { return data; }
	const T* end() const { return data + size; }
};

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Standard pixel formats for rendering targets
static constexpr vk::Format HDR_COLOR_FORMAT    = vk::Format::eR16G16B16A16Sfloat;
static constexpr vk::Format LDR_COLOR_FORMAT    = vk::Format::eR8G8B8A8Unorm;
static constexpr vk::Format SHADOW_DEPTH_FORMAT = vk::Format::eD32Sfloat;

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

