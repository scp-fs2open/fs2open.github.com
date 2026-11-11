#include "graphics/vulkan/VulkanUpscaler.h"

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/vulkan/VulkanBarrier.h"
#include "graphics/vulkan/VulkanMemory.h"
#include "graphics/vulkan/VulkanPostProcessing.h"
#include "render/3d.h"

#ifdef WITH_FFX
#include <ffx_portable.h>
#endif

#include <optional>

extern float flRealframetime;

namespace graphics::vulkan {

#ifdef WITH_FFX

namespace {

// vkCreateDevice reads through pointers into this, so it has to outlive the caller's scope.
ffxPortableVkRequirements Device_requirements;
bool Upscaler_available = false;
vk::PhysicalDevice Upscaler_gpu;

// The mode and version FSR failed with. Choosing anything else gives it another try.
struct failed_setting {
	FsrMode mode;
	int majorVersion;
};
std::optional<failed_setting> Failed_setting;

bool failed_for_current_setting()
{
	if (Failed_setting && (Failed_setting->mode != Gr_fsr_mode || Failed_setting->majorVersion != Gr_fsr_major_version)) {
		Failed_setting.reset();
	}
	return Failed_setting.has_value();
}

void mark_failed()
{
	Failed_setting = failed_setting{Gr_fsr_mode, Gr_fsr_major_version};
}

struct fsr_version {
	uint64_t id = 0;
	SCP_string name;
};

// AMD lists the versions this device can run, the one it selects by default first.
SCP_vector<fsr_version> query_fsr_versions(vk::Device device)
{
	VkDeviceContext context{static_cast<VkDevice>(device), static_cast<VkPhysicalDevice>(Upscaler_gpu),
		VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr};

	uint64_t count = 0;
	ffxQueryDescGetVersions query{};
	query.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
	query.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
	query.device = &context;
	query.outputCount = &count;
	if (ffxQuery(nullptr, &query.header) != FFX_API_RETURN_OK || count == 0) {
		return {};
	}

	const auto n = static_cast<size_t>(count);
	SCP_vector<uint64_t> ids(n);
	SCP_vector<const char*> names(n);
	query.versionIds = ids.data();
	query.versionNames = names.data();
	if (ffxQuery(nullptr, &query.header) != FFX_API_RETURN_OK) {
		return {};
	}

	SCP_vector<fsr_version> versions;
	for (size_t i = 0; i < n; ++i) {
		versions.push_back({ids[i], names[i] ? names[i] : ""});
	}
	return versions;
}

int major_version(const SCP_string& name)
{
	return std::atoi(name.c_str());
}

bool is_offered(int major)
{
	return major == 4 || major == 3;
}

// The version the user chose, falling back to the newest offered one
const fsr_version* find_chosen_version(const SCP_vector<fsr_version>& versions)
{
	const int wanted = Gr_fsr_major_version != 0 || Gr_fsr_versions.empty() ? Gr_fsr_major_version
		: major_version(Gr_fsr_versions.front());
	for (const auto& version : versions) {
		if (major_version(version.name) == wanted) {
			return &version;
		}
	}
	mprintf(("Vulkan: FSR %d isn't available, leaving the version to AMD's selection\n", wanted));
	return nullptr;
}

void log_ffx_message(uint32_t type, const wchar_t* message)
{
	mprintf(("FSR %s: %ls\n", type == FFX_API_MESSAGE_TYPE_ERROR ? "error" : "warning", message));
}

FfxApiResource ffx_image(const RenderTarget& target, FfxApiResourceState state, uint32_t usage = FFX_API_RESOURCE_USAGE_READ_ONLY)
{
	FfxApiResourceDescription description{};
	description.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
	description.format = ffxApiGetSurfaceFormatVK(static_cast<VkFormat>(target.format));
	description.width = target.width;
	description.height = target.height;
	description.depth = 1;
	description.mipCount = 1;
	description.usage = usage;
	return ffxApiGetResourceVK(static_cast<VkImage>(target.image), description, state);
}

} // namespace

void vulkan_upscaler_prepare_device(vk::Instance instance,
	vk::PhysicalDevice gpu,
	uint32_t graphicsQueueFamily,
	const vk::PhysicalDeviceFeatures& supportedFeatures,
	vk::DeviceCreateInfo& createInfo)
{
	Upscaler_available = false;

	if (!ffxPortableCanLoad()) {
		mprintf(("Vulkan: FSR unavailable: %s\n", ffxPortableLoadError()));
		return;
	}

	const auto queueFamilies = gpu.getQueueFamilyProperties();
	if (graphicsQueueFamily >= queueFamilies.size() ||
		!(queueFamilies[graphicsQueueFamily].queueFlags & vk::QueueFlagBits::eCompute)) {
		mprintf(("Vulkan: FSR unavailable: the graphics queue has no compute support\n"));
		return;
	}
	if (!supportedFeatures.independentBlend) {
		mprintf(("Vulkan: FSR unavailable: independentBlend is not supported\n"));
		return;
	}

	ffxPortableVkGetDeviceRequirements(static_cast<VkInstance>(instance),
		static_cast<VkPhysicalDevice>(gpu),
		VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr,
		&Device_requirements);

	if (VulkanApiVersion < Device_requirements.minApiVersion) {
		mprintf(("Vulkan: FSR unavailable: needs a newer Vulkan API version than the renderer uses\n"));
		return;
	}

	if (!ffxPortableVkApplyRequirements(&Device_requirements, reinterpret_cast<VkDeviceCreateInfo*>(&createInfo))) {
		mprintf(("Vulkan: FSR unavailable: too many device extensions\n"));
		return;
	}

	mprintf(("Vulkan: FSR device requirements applied (FP8 capable: %s, INT8 capable: %s)\n",
		Device_requirements.fp8Capable ? "yes" : "no",
		Device_requirements.int8Capable ? "yes" : "no"));
	Upscaler_gpu = gpu;
	Upscaler_available = true;
}

void vulkan_upscaler_device_created(vk::Device device)
{
	if (!Upscaler_available) {
		return;
	}

	Gr_fsr_max_output_size = static_cast<int>(Upscaler_gpu.getProperties().limits.maxImageDimension2D);
	Gr_fsr_versions.clear();
	const auto versions = query_fsr_versions(device);
	for (const auto& version : versions) {
		mprintf(("Vulkan: FSR version available: %s\n", version.name.c_str()));
		if (is_offered(major_version(version.name))) {
			Gr_fsr_versions.push_back(version.name);
		}
	}
	// FSR 2 is only the fallback for hardware where neither of the newer versions is available
	if (Gr_fsr_versions.empty()) {
		for (const auto& version : versions) {
			if (major_version(version.name) == 2) {
				Gr_fsr_versions.push_back(version.name);
			}
		}
	}
	std::sort(Gr_fsr_versions.begin(), Gr_fsr_versions.end(),
		[](const SCP_string& a, const SCP_string& b) { return major_version(a) > major_version(b); });
}

bool vulkan_upscaler_available()
{
	return Upscaler_available;
}

bool vulkan_upscaler_usable()
{
	return Upscaler_available && !failed_for_current_setting();
}

bool VulkanUpscaler::wanted()
{
	return vulkan_upscaler_usable() && Gr_fsr_mode != FsrMode::Off;
}

bool VulkanUpscaler::createResources()
{
	if (m_context) {
		return true;
	}

	const uint32_t renderW = m_ctx->sceneExtent.width;
	const uint32_t renderH = m_ctx->sceneExtent.height;
	const uint32_t w = m_ctx->outputExtent.width;
	const uint32_t h = m_ctx->outputExtent.height;

	m_output.format = HDR_COLOR_FORMAT;
	m_output.width = w;
	m_output.height = h;
	if (!m_ctx->createImage(w, h, HDR_COLOR_FORMAT,
	                        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
	                        vk::ImageAspectFlagBits::eColor,
	                        m_output.image, m_output.view, m_output.allocation)) {
		mprintf(("Vulkan: FSR disabled: could not create its output image\n"));
		mark_failed();
		return false;
	}

	ffxCreateBackendVKDesc backend{};
	backend.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
	backend.deviceContext.vkDevice = static_cast<VkDevice>(m_ctx->device);
	backend.deviceContext.vkPhysicalDevice = static_cast<VkPhysicalDevice>(Upscaler_gpu);
	backend.deviceContext.vkDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

	ffxCreateContextDescUpscaleVersion version{};
	version.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE_VERSION;
	version.header.pNext = &backend.header;
	version.version = FFX_UPSCALER_VERSION;

	ffxCreateContextDescUpscale desc{};
	desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
	desc.header.pNext = &version.header;
	desc.flags = FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE | FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
	desc.maxRenderSize = {renderW, renderH};
	desc.maxUpscaleSize = {w, h};
	desc.fpMessage = log_ffx_message;

	ffxOverrideVersion chosenVersion{};
	const auto versions = query_fsr_versions(m_ctx->device);
	if (const auto* chosen = find_chosen_version(versions)) {
		chosenVersion.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
		chosenVersion.header.pNext = desc.header.pNext;
		chosenVersion.versionId = chosen->id;
		desc.header.pNext = &chosenVersion.header;
	}
	m_contextMajorVersion = Gr_fsr_major_version;

	const auto result = ffxCreateContext(&m_context, &desc.header, nullptr);
	if (result != FFX_API_RETURN_OK) {
		mprintf(("Vulkan: FSR disabled: creating its context failed with code %u\n", result));
		m_context = nullptr;
		m_ctx->destroyTarget(m_output);
		mark_failed();
		return false;
	}

	ffxQueryGetProviderVersion provider{};
	provider.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
	const bool versionKnown = ffxQuery(&m_context, &provider.header) == FFX_API_RETURN_OK && provider.versionName;
	mprintf(("Vulkan: FSR context created for %ux%u to %ux%u, loaded FSR version: %s\n", renderW, renderH, w, h,
		versionKnown ? provider.versionName : "unknown"));

	ffxQueryDescUpscaleGetJitterPhaseCount phaseCount{};
	phaseCount.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTERPHASECOUNT;
	phaseCount.renderWidth = renderW;
	phaseCount.displayWidth = w;
	phaseCount.pOutPhaseCount = &m_jitterPhaseCount;
	if (ffxQuery(&m_context, &phaseCount.header) != FFX_API_RETURN_OK || m_jitterPhaseCount < 1) {
		m_jitterPhaseCount = 1;
	}
	m_jitterIndex = 0;

	return true;
}

void VulkanUpscaler::destroyTargets()
{
	if (!m_ctx) {
		return;
	}

	if (m_context) {
		ffxDestroyContext(&m_context, nullptr);
		m_context = nullptr;
	}
	m_ctx->destroyTarget(m_output);
}

void VulkanUpscaler::beginFrame(float& imageOffsetX, float& imageOffsetY, float& lodBias)
{
	m_jitterIndex = (m_jitterIndex + 1) % m_jitterPhaseCount;

	ffxQueryDescUpscaleGetJitterOffset query{};
	query.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTEROFFSET;
	query.index = m_jitterIndex;
	query.phaseCount = m_jitterPhaseCount;
	query.pOutX = &m_jitterX;
	query.pOutY = &m_jitterY;
	if (ffxQuery(&m_context, &query.header) != FFX_API_RETURN_OK) {
		m_jitterX = 0.0f;
		m_jitterY = 0.0f;
	}

	// FSR takes a pixel to show the content at its position minus the jitter, so the image moves by the jitter, in
	// image space with Y down.
	imageOffsetX = m_jitterX;
	imageOffsetY = m_jitterY;
	lodBias = std::log2(static_cast<float>(m_ctx->sceneExtent.width) / static_cast<float>(m_output.width)) - 1.0f;
}

bool VulkanUpscaler::dispatch(vk::CommandBuffer cmd, const RenderTarget& color, const RenderTarget& depth, const RenderTarget& motion)
{
	GR_DEBUG_SCOPE("FSR");

	cmdMemoryBarrier(cmd,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderRead);

	ImageBarrier2 barrier;
	barrier.image = m_output.image;
	barrier.levelCount = 1;
	barrier.layerCount = 1;
	barrier.oldLayout = vk::ImageLayout::eUndefined;
	barrier.newLayout = vk::ImageLayout::eGeneral;
	// Last frame's bloom composited into the output, and tonemapping read it
	barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eFragmentShader;
	barrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
	barrier.dstStage = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.dstAccess = vk::AccessFlagBits2::eShaderWrite;
	cmdImageBarrier(cmd, barrier);

	ffxDispatchDescUpscale desc{};
	desc.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;
	desc.commandList = static_cast<VkCommandBuffer>(cmd);
	desc.color = ffx_image(color, FFX_API_RESOURCE_STATE_COMPUTE_READ);
	desc.depth = ffx_image(depth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
	desc.motionVectors = ffx_image(motion, FFX_API_RESOURCE_STATE_COMPUTE_READ);
	desc.output = ffx_image(m_output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS, FFX_API_RESOURCE_USAGE_UAV);
	desc.jitterOffset = {m_jitterX, m_jitterY};
	desc.renderSize = {color.width, color.height};
	desc.upscaleSize = {m_output.width, m_output.height};
	// Motion vectors are in UV units
	desc.motionVectorScale = {static_cast<float>(color.width), static_cast<float>(color.height)};
	desc.frameTimeDelta = flRealframetime * 1000.0f;
	desc.preExposure = 1.0f;
	desc.reset = gr_consume_temporal_reset();
	// The motion vector pass puts all depth on the cockpit's projection
	desc.cameraNear = Min_draw_distance_cockpit;
	desc.cameraFar = Max_draw_distance;
	desc.cameraFovAngleVertical = std::holds_alternative<float>(Proj_fov) ? std::get<float>(Proj_fov) : 1.0f;
	desc.viewSpaceToMetersFactor = 1.0f;

	const auto result = ffxDispatch(&m_context, &desc.header);
	const bool dispatched = result == FFX_API_RETURN_OK;
	if (!dispatched) {
		mprintf(("Vulkan: FSR disabled: dispatch failed with code %u\n", result));
		mark_failed();
	}

	barrier.oldLayout = vk::ImageLayout::eGeneral;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcStage = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.srcAccess = vk::AccessFlagBits2::eShaderWrite;
	barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
	barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;
	cmdImageBarrier(cmd, barrier);

	return dispatched;
}

#else // WITH_FFX

void vulkan_upscaler_prepare_device(vk::Instance /*instance*/,
	vk::PhysicalDevice /*gpu*/,
	uint32_t /*graphicsQueueFamily*/,
	const vk::PhysicalDeviceFeatures& /*supportedFeatures*/,
	vk::DeviceCreateInfo& /*createInfo*/)
{
}

void vulkan_upscaler_device_created(vk::Device /*device*/)
{
}

bool vulkan_upscaler_available()
{
	return false;
}

bool vulkan_upscaler_usable()
{
	return false;
}

bool VulkanUpscaler::wanted()
{
	return false;
}

bool VulkanUpscaler::createResources()
{
	return false;
}

void VulkanUpscaler::destroyTargets()
{
}

void VulkanUpscaler::beginFrame(float& /*imageOffsetX*/, float& /*imageOffsetY*/, float& /*lodBias*/)
{
}

bool VulkanUpscaler::dispatch(vk::CommandBuffer /*cmd*/, const RenderTarget& /*color*/, const RenderTarget& /*depth*/, const RenderTarget& /*motion*/)
{
	return false;
}

#endif // WITH_FFX

} // namespace graphics::vulkan
