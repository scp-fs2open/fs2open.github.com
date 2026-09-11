#include <gtest/gtest.h>
#include <math/vecmat.h>
#include <graphics/vulkan/VulkanRaytracing.h>

// Regression test for a bug where toVkTransform() used the FS2Open orientation
// matrix directly (world-to-local convention) instead of its transpose
// (local-to-world), producing acceleration-structure instances rotated
// incorrectly relative to their rasterized counterparts. An identity-oriented
// object can't distinguish the two conventions (M == M^T), so this uses a
// non-identity rotation matrix deliberately.
TEST(VulkanRaytracingTransform, MatchesLocalToWorldConvention)
{
	// rvec=(0,0,1), uvec=(0,1,0), fvec=(-1,0,0) -- an orthonormal, proper
	// (determinant +1) rotation matrix that is not its own transpose.
	matrix orient = {{{{{{0.0f, 0.0f, 1.0f}}}, {{{0.0f, 1.0f, 0.0f}}}, {{{-1.0f, 0.0f, 0.0f}}}}}};
	vec3d pos = {{{10.0f, 20.0f, 30.0f}}};
	vec3d local = {{{1.0f, 2.0f, 3.0f}}};

	// Ground truth: FS2Open's own local-to-world rotation, vm_vec_unrotate,
	// plus the translation.
	vec3d expectedWorld;
	vm_vec_unrotate(&expectedWorld, &local, &orient);
	vm_vec_add2(&expectedWorld, &pos);

	vk::TransformMatrixKHR vkTransform = graphics::vulkan::toVkTransform(orient, pos);

	// Apply the row-major 3x4 VkTransformMatrixKHR the same way the GPU would:
	// world = M*local + T.
	float worldX = vkTransform.matrix[0][0] * local.xyz.x + vkTransform.matrix[0][1] * local.xyz.y +
		vkTransform.matrix[0][2] * local.xyz.z + vkTransform.matrix[0][3];
	float worldY = vkTransform.matrix[1][0] * local.xyz.x + vkTransform.matrix[1][1] * local.xyz.y +
		vkTransform.matrix[1][2] * local.xyz.z + vkTransform.matrix[1][3];
	float worldZ = vkTransform.matrix[2][0] * local.xyz.x + vkTransform.matrix[2][1] * local.xyz.y +
		vkTransform.matrix[2][2] * local.xyz.z + vkTransform.matrix[2][3];

	EXPECT_NEAR(expectedWorld.xyz.x, worldX, 0.0001f);
	EXPECT_NEAR(expectedWorld.xyz.y, worldY, 0.0001f);
	EXPECT_NEAR(expectedWorld.xyz.z, worldZ, 0.0001f);
}
