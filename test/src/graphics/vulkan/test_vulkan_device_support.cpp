#include <gtest/gtest.h>
#include <graphics/vulkan/VulkanRenderer.h>

#include <cstring>

using graphics::vulkan::PhysicalDeviceValues;
using graphics::vulkan::raytracedShadowsSupported;

namespace {
PhysicalDeviceValues deviceWithEverything()
{
	PhysicalDeviceValues values;
	for (const char* name : {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			 VK_KHR_RAY_QUERY_EXTENSION_NAME,
			 VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME}) {
		vk::ExtensionProperties ext;
		strncpy(ext.extensionName.data(), name, VK_MAX_EXTENSION_NAME_SIZE - 1);
		values.extensions.push_back(ext);
	}
	values.accelerationStructureFeatureSupported = true;
	values.rayQueryFeatureSupported = true;
	values.bufferDeviceAddressFeatureSupported = true;
	return values;
}

void removeExtension(PhysicalDeviceValues& values, const char* name)
{
	for (auto it = values.extensions.begin(); it != values.extensions.end(); ++it) {
		if (strcmp(it->extensionName, name) == 0) {
			values.extensions.erase(it);
			return;
		}
	}
}
} // namespace

// the renderer and the launcher query both decide ray-traced shadow support with this, so every part must count
TEST(VulkanDeviceSupport, RaytracingNeedsEveryExtensionAndFeature)
{
	ASSERT_TRUE(raytracedShadowsSupported(deviceWithEverything()));

	for (const char* name : {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			 VK_KHR_RAY_QUERY_EXTENSION_NAME,
			 VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME}) {
		auto values = deviceWithEverything();
		removeExtension(values, name);
		EXPECT_FALSE(raytracedShadowsSupported(values)) << "missing " << name;
	}

	auto noAccel = deviceWithEverything();
	noAccel.accelerationStructureFeatureSupported = false;
	EXPECT_FALSE(raytracedShadowsSupported(noAccel));

	auto noRayQuery = deviceWithEverything();
	noRayQuery.rayQueryFeatureSupported = false;
	EXPECT_FALSE(raytracedShadowsSupported(noRayQuery));

	auto noAddress = deviceWithEverything();
	noAddress.bufferDeviceAddressFeatureSupported = false;
	EXPECT_FALSE(raytracedShadowsSupported(noAddress));
}
