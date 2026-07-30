#pragma once

#include "globalincs/pstypes.h"

#include <type_traits>

namespace os {

class Viewport;

/**
 * @brief The windowing-system half of Vulkan initialization
 * @ingroup os_graphics_api
 *
 * Three things the Vulkan renderer cannot work out for itself, because all three depend on which
 * windowing toolkit created the window: where the Vulkan loader is, which instance extensions that
 * toolkit's surfaces need, and how to turn one of its windows into a VkSurfaceKHR.
 *
 * Handles are passed as @c void* (VkInstance, always a pointer) and @c uint64_t (VkSurfaceKHR, a
 * pointer on 64-bit targets but a plain integer on 32-bit ones) so this header stays free of the
 * Vulkan headers -- it has to compile in the @c FSO_BUILD_WITH_VULKAN=OFF configuration too. Use
 * vulkan_handle_cast() / vulkan_handle_value() to convert at the ends.
 */
class VulkanSurfaceProvider {
  public:
	virtual ~VulkanSurfaceProvider() = default;

	/**
	 * @brief Loads the Vulkan loader and returns @c vkGetInstanceProcAddr
	 *
	 * @return The function pointer, or @c nullptr if the loader is unavailable
	 */
	virtual void* getVulkanProcAddr() = 0;

	/**
	 * @brief The instance extensions this windowing system's surfaces require
	 *
	 * These are merged into the extension list the renderer builds; the renderer still adds its own
	 * (debug utils, swap chain color space, ...) on top.
	 *
	 * @param[out] extensions Receives the extension names
	 * @return @c true on success
	 */
	virtual bool getVulkanInstanceExtensions(SCP_vector<SCP_string>& extensions) = 0;

	/**
	 * @brief Creates a Vulkan surface for a viewport
	 *
	 * @param view      The viewport to create the surface for
	 * @param vkInstance The @c VkInstance the surface belongs to
	 * @return The @c VkSurfaceKHR handle, or 0 on failure
	 */
	virtual uint64_t createVulkanSurface(Viewport* view, void* vkInstance) = 0;

	/**
	 * @brief Destroys a surface previously returned by createVulkanSurface()
	 *
	 * @note The renderer must always go through this rather than calling @c vkDestroySurfaceKHR
	 * itself: an implementation may not own the surface it handed out.
	 */
	virtual void destroyVulkanSurface(void* vkInstance, uint64_t surface) = 0;
};

/**
 * @brief Converts a surface handle from its transport type back to the Vulkan handle type
 * @ingroup os_graphics_api
 */
template <typename HandleType>
inline HandleType vulkan_handle_cast(uint64_t handle)
{
	if constexpr (std::is_pointer<HandleType>::value) {
		return reinterpret_cast<HandleType>(static_cast<uintptr_t>(handle));
	} else {
		return static_cast<HandleType>(handle);
	}
}

/**
 * @brief Converts a Vulkan handle to the transport type used by VulkanSurfaceProvider
 * @ingroup os_graphics_api
 */
template <typename HandleType>
inline uint64_t vulkan_handle_value(HandleType handle)
{
	if constexpr (std::is_pointer<HandleType>::value) {
		return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(handle));
	} else {
		return static_cast<uint64_t>(handle);
	}
}

} // namespace os
