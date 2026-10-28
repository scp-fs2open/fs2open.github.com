#include "graphics/2d.h"
#include "graphics/vulkan/VulkanConvert.h"

#include <gtest/gtest.h>

using graphics::vulkan::convertDepthMode;

TEST(VulkanDepthMode, ReversedDepthComparesWithGreater)
{
	vk::CompareOp op = vk::CompareOp::eAlways;
	bool write = false;

	convertDepthMode(ZBUFFER_TYPE_FULL, op, write);
	EXPECT_EQ(op, vk::CompareOp::eLess);
	convertDepthMode(ZBUFFER_TYPE_READ, op, write);
	EXPECT_EQ(op, vk::CompareOp::eLess);

	Gr_reversed_depth_active = true;
	convertDepthMode(ZBUFFER_TYPE_FULL, op, write);
	const auto full = op;
	convertDepthMode(ZBUFFER_TYPE_READ, op, write);
	const auto read = op;
	Gr_reversed_depth_active = false;
	EXPECT_EQ(full, vk::CompareOp::eGreater);
	EXPECT_EQ(read, vk::CompareOp::eGreater);
}
