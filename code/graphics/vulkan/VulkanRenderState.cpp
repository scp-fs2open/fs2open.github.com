#include "VulkanRenderState.h"

namespace graphics {
namespace vulkan {

void convertBlendMode(gr_alpha_blend mode, vk::BlendFactor& srcFactor, vk::BlendFactor& dstFactor)
{
	// Based on SetAlphaBlendMode in gropenglstate.cpp
	switch (mode) {
	case ALPHA_BLEND_NONE:
		srcFactor = vk::BlendFactor::eOne;
		dstFactor = vk::BlendFactor::eZero;
		break;
	case ALPHA_BLEND_ALPHA_ADDITIVE:
		srcFactor = vk::BlendFactor::eSrcAlpha;
		dstFactor = vk::BlendFactor::eOne;
		break;
	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
		srcFactor = vk::BlendFactor::eSrcAlpha;
		dstFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		break;
	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
		srcFactor = vk::BlendFactor::eSrcColor;
		dstFactor = vk::BlendFactor::eOneMinusSrcColor;
		break;
	case ALPHA_BLEND_ADDITIVE:
		srcFactor = vk::BlendFactor::eOne;
		dstFactor = vk::BlendFactor::eOne;
		break;
	case ALPHA_BLEND_PREMULTIPLIED:
		srcFactor = vk::BlendFactor::eOne;
		dstFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		break;
	default:
		srcFactor = vk::BlendFactor::eOne;
		dstFactor = vk::BlendFactor::eZero;
		break;
	}
}

void convertDepthMode(gr_zbuffer_type type, vk::CompareOp& compareOp, bool& writeEnable)
{
	// Based on SetZbufferType in gropenglstate.cpp
	switch (type) {
	case ZBUFFER_TYPE_NONE:
		compareOp = vk::CompareOp::eAlways;
		writeEnable = false;
		break;
	case ZBUFFER_TYPE_READ:
		compareOp = vk::CompareOp::eLess;
		writeEnable = false;
		break;
	case ZBUFFER_TYPE_WRITE:
		compareOp = vk::CompareOp::eAlways;
		writeEnable = true;
		break;
	case ZBUFFER_TYPE_FULL:
		compareOp = vk::CompareOp::eLess;
		writeEnable = true;
		break;
	default:
		compareOp = vk::CompareOp::eAlways;
		writeEnable = false;
		break;
	}
}

vk::CompareOp convertStencilCompare(ComparisionFunction func)
{
	switch (func) {
	case ComparisionFunction::Never:
		return vk::CompareOp::eNever;
	case ComparisionFunction::Less:
		return vk::CompareOp::eLess;
	case ComparisionFunction::Equal:
		return vk::CompareOp::eEqual;
	case ComparisionFunction::LessOrEqual:
		return vk::CompareOp::eLessOrEqual;
	case ComparisionFunction::Greater:
		return vk::CompareOp::eGreater;
	case ComparisionFunction::NotEqual:
		return vk::CompareOp::eNotEqual;
	case ComparisionFunction::GreaterOrEqual:
		return vk::CompareOp::eGreaterOrEqual;
	case ComparisionFunction::Always:
	default:
		return vk::CompareOp::eAlways;
	}
}

vk::StencilOp convertStencilOp(StencilOperation op)
{
	switch (op) {
	case StencilOperation::Keep:
		return vk::StencilOp::eKeep;
	case StencilOperation::Zero:
		return vk::StencilOp::eZero;
	case StencilOperation::Replace:
		return vk::StencilOp::eReplace;
	case StencilOperation::Increment:
		return vk::StencilOp::eIncrementAndClamp;
	case StencilOperation::Decrement:
		return vk::StencilOp::eDecrementAndClamp;
	case StencilOperation::Invert:
		return vk::StencilOp::eInvert;
	case StencilOperation::IncrementWrap:
		return vk::StencilOp::eIncrementAndWrap;
	case StencilOperation::DecrementWrap:
		return vk::StencilOp::eDecrementAndWrap;
	default:
		return vk::StencilOp::eKeep;
	}
}

vk::PrimitiveTopology convertPrimitiveType(primitive_type type)
{
	switch (type) {
	case PRIM_TYPE_POINTS:
		return vk::PrimitiveTopology::ePointList;
	case PRIM_TYPE_LINES:
		return vk::PrimitiveTopology::eLineList;
	case PRIM_TYPE_LINESTRIP:
		return vk::PrimitiveTopology::eLineStrip;
	case PRIM_TYPE_TRIS:
		return vk::PrimitiveTopology::eTriangleList;
	case PRIM_TYPE_TRISTRIP:
		return vk::PrimitiveTopology::eTriangleStrip;
	case PRIM_TYPE_TRIFAN:
		return vk::PrimitiveTopology::eTriangleFan;
	default:
		return vk::PrimitiveTopology::eTriangleList;
	}
}

vk::CullModeFlags convertCullMode(bool cullEnabled)
{
	return cullEnabled ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
}

bool isBlendingEnabled(gr_alpha_blend mode)
{
	return mode != ALPHA_BLEND_NONE;
}

vk::PipelineColorBlendAttachmentState createColorBlendAttachment(gr_alpha_blend mode, const bvec4& colorWriteMask)
{
	vk::PipelineColorBlendAttachmentState attachment;

	attachment.blendEnable = isBlendingEnabled(mode) ? VK_TRUE : VK_FALSE;

	vk::BlendFactor srcFactor, dstFactor;
	convertBlendMode(mode, srcFactor, dstFactor);

	attachment.srcColorBlendFactor = srcFactor;
	attachment.dstColorBlendFactor = dstFactor;
	attachment.colorBlendOp = vk::BlendOp::eAdd;

	// Alpha blend - same as color for most modes
	attachment.srcAlphaBlendFactor = srcFactor;
	attachment.dstAlphaBlendFactor = dstFactor;
	attachment.alphaBlendOp = vk::BlendOp::eAdd;

	// Color write mask from material
	vk::ColorComponentFlags writeMask;
	if (colorWriteMask.x) writeMask |= vk::ColorComponentFlagBits::eR;
	if (colorWriteMask.y) writeMask |= vk::ColorComponentFlagBits::eG;
	if (colorWriteMask.z) writeMask |= vk::ColorComponentFlagBits::eB;
	if (colorWriteMask.w) writeMask |= vk::ColorComponentFlagBits::eA;
	attachment.colorWriteMask = writeMask;

	return attachment;
}

vk::PipelineDepthStencilStateCreateInfo createDepthStencilState(
	gr_zbuffer_type depthMode,
	bool stencilEnabled,
	ComparisionFunction stencilFunc,
	const material::StencilOp* frontOp,
	const material::StencilOp* backOp,
	uint32_t stencilMask)
{
	vk::PipelineDepthStencilStateCreateInfo info;

	// Depth settings
	vk::CompareOp depthCompare;
	bool depthWrite;
	convertDepthMode(depthMode, depthCompare, depthWrite);

	info.depthTestEnable = (depthMode != ZBUFFER_TYPE_NONE) ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = depthCompare;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f;
	info.maxDepthBounds = 1.0f;

	// Stencil settings
	info.stencilTestEnable = stencilEnabled ? VK_TRUE : VK_FALSE;

	if (stencilEnabled) {
		// Front face stencil
		info.front.compareOp = convertStencilCompare(stencilFunc);
		info.front.compareMask = 0xFF;
		info.front.writeMask = stencilMask;
		info.front.reference = 0; // Set dynamically

		if (frontOp) {
			info.front.failOp = convertStencilOp(frontOp->stencilFailOperation);
			info.front.depthFailOp = convertStencilOp(frontOp->depthFailOperation);
			info.front.passOp = convertStencilOp(frontOp->successOperation);
		} else {
			info.front.failOp = vk::StencilOp::eKeep;
			info.front.depthFailOp = vk::StencilOp::eKeep;
			info.front.passOp = vk::StencilOp::eKeep;
		}

		// Back face stencil
		info.back.compareOp = convertStencilCompare(stencilFunc);
		info.back.compareMask = 0xFF;
		info.back.writeMask = stencilMask;
		info.back.reference = 0;

		if (backOp) {
			info.back.failOp = convertStencilOp(backOp->stencilFailOperation);
			info.back.depthFailOp = convertStencilOp(backOp->depthFailOperation);
			info.back.passOp = convertStencilOp(backOp->successOperation);
		} else {
			info.back.failOp = vk::StencilOp::eKeep;
			info.back.depthFailOp = vk::StencilOp::eKeep;
			info.back.passOp = vk::StencilOp::eKeep;
		}
	}

	return info;
}

vk::PipelineRasterizationStateCreateInfo createRasterizationState(
	bool cullEnabled,
	int fillMode,
	bool frontFaceCW,
	bool depthBiasEnabled)
{
	vk::PipelineRasterizationStateCreateInfo info;

	info.depthClampEnable = VK_FALSE;
	info.rasterizerDiscardEnable = VK_FALSE;

	// Fill mode
	switch (fillMode) {
	case GR_FILL_MODE_WIRE:
		info.polygonMode = vk::PolygonMode::eLine;
		break;
	case GR_FILL_MODE_SOLID:
	default:
		info.polygonMode = vk::PolygonMode::eFill;
		break;
	}

	info.cullMode = convertCullMode(cullEnabled);
	info.frontFace = frontFaceCW ? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise;

	// Depth bias - actual values set dynamically via vkCmdSetDepthBias
	info.depthBiasEnable = depthBiasEnabled ? VK_TRUE : VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	info.lineWidth = 1.0f;

	return info;
}

} // namespace vulkan
} // namespace graphics
