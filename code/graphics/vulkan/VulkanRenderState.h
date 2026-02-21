#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Convert FSO alpha blend mode to Vulkan blend factors
 * @param mode FSO blend mode
 * @param srcFactor Output source blend factor
 * @param dstFactor Output destination blend factor
 */
void convertBlendMode(gr_alpha_blend mode, vk::BlendFactor& srcFactor, vk::BlendFactor& dstFactor);

/**
 * @brief Convert FSO depth buffer type to Vulkan compare op and write mask
 * @param type FSO zbuffer type
 * @param compareOp Output compare operation
 * @param writeEnable Output depth write enable
 */
void convertDepthMode(gr_zbuffer_type type, vk::CompareOp& compareOp, bool& writeEnable);

/**
 * @brief Convert FSO stencil comparison function to Vulkan compare op
 * @param func FSO comparison function
 * @return Vulkan compare operation
 */
vk::CompareOp convertStencilCompare(ComparisionFunction func);

/**
 * @brief Convert FSO stencil operation to Vulkan stencil op
 * @param op FSO stencil operation
 * @return Vulkan stencil operation
 */
vk::StencilOp convertStencilOp(StencilOperation op);

/**
 * @brief Convert FSO primitive type to Vulkan topology
 * @param type FSO primitive type
 * @return Vulkan primitive topology
 */
vk::PrimitiveTopology convertPrimitiveType(primitive_type type);

/**
 * @brief Convert FSO cull mode to Vulkan cull mode
 * @param cullEnabled Whether culling is enabled
 * @return Vulkan cull mode flags
 */
vk::CullModeFlags convertCullMode(bool cullEnabled);

/**
 * @brief Check if a blend mode requires blending to be enabled
 * @param mode FSO blend mode
 * @return true if blending should be enabled
 */
bool isBlendingEnabled(gr_alpha_blend mode);

/**
 * @brief Create a complete color blend attachment state
 * @param mode FSO blend mode
 * @return Vulkan color blend attachment state
 */
vk::PipelineColorBlendAttachmentState createColorBlendAttachment(gr_alpha_blend mode,
	const bvec4& colorWriteMask = {true, true, true, true});

/**
 * @brief Create depth stencil state create info
 * @param depthMode FSO depth buffer mode
 * @param stencilEnabled Whether stencil testing is enabled
 * @param stencilFunc Stencil comparison function
 * @param frontOp Front face stencil operations
 * @param backOp Back face stencil operations
 * @param stencilMask Stencil write mask
 * @return Vulkan depth stencil state create info
 */
vk::PipelineDepthStencilStateCreateInfo createDepthStencilState(
	gr_zbuffer_type depthMode,
	bool stencilEnabled = false,
	ComparisionFunction stencilFunc = ComparisionFunction::Always,
	const material::StencilOp* frontOp = nullptr,
	const material::StencilOp* backOp = nullptr,
	uint32_t stencilMask = 0xFF);

/**
 * @brief Create rasterization state create info
 * @param cullEnabled Whether back-face culling is enabled
 * @param fillMode Polygon fill mode (0 = fill, 1 = line, 2 = point)
 * @param frontFace Front face winding (true = CW, false = CCW)
 * @return Vulkan rasterization state create info
 */
vk::PipelineRasterizationStateCreateInfo createRasterizationState(
	bool cullEnabled = true,
	int fillMode = 0,
	bool frontFaceCW = true,
	bool depthBiasEnabled = false);

} // namespace vulkan
} // namespace graphics
