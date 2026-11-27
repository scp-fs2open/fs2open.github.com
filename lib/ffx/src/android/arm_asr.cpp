#include "ffx_portable.h"

#include <host/backends/vk/ffxm_vk.h>
#include <host/ffxm_fsr2.h>

#include <memory>
#include <new>

namespace {

constexpr uint64_t ProviderId = 1;
constexpr char ProviderName[] = "Arm ASR 25.06";
constexpr size_t MaxViews = 2;

VkInstance Instance;
VkPhysicalDevice PhysicalDevice;
PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
const char* UnavailableReason = "the Vulkan device has not been checked";

struct Backend {
    arm::FfxmInterface interface{};
    std::unique_ptr<std::byte[]> scratch;
    size_t views = 0;
};

// The standalone Arm backend uses one shared interface for its effect contexts.
Backend SharedBackend;
ffxApiMessage ClientMessage;

struct Context {
    arm::FfxmFsr2Context asr;
    arm::FfxmDimensions2D outputSize;
};

ffxReturnCode_t translateResult(arm::FfxmErrorCode result)
{
    if (result == arm::FFXM_OK)
        return FFX_API_RETURN_OK;
    if (result == arm::FFXM_ERROR_OUT_OF_MEMORY || result == arm::FFXM_ERROR_INSUFFICIENT_MEMORY)
        return FFX_API_RETURN_ERROR_MEMORY;
    return FFX_API_RETURN_ERROR_RUNTIME_ERROR;
}

void logAsr(arm::FfxmMsgType type, const wchar_t* message)
{
    if (ClientMessage) {
        ClientMessage(type == arm::FFXM_MESSAGE_TYPE_ERROR ? FFX_API_MESSAGE_TYPE_ERROR : FFX_API_MESSAGE_TYPE_WARNING,
            message);
    }
}

ffxReturnCode_t initBackend(const VkDeviceContext& vk)
{
    if (SharedBackend.views)
        return FFX_API_RETURN_OK;

    arm::VkDeviceContext device{vk.vkDevice, vk.vkPhysicalDevice, vk.vkDeviceProcAddr, Instance, GetInstanceProcAddr};
    const auto armDevice = arm::ffxmGetDeviceVK(&device);
    const auto size = arm::ffxmGetScratchMemorySizeVK(vk.vkPhysicalDevice, MaxViews);
    SharedBackend.scratch.reset(new (std::nothrow) std::byte[size]);
    if (!SharedBackend.scratch)
        return FFX_API_RETURN_ERROR_MEMORY;

    const auto result = arm::ffxmGetInterfaceVK(&SharedBackend.interface, armDevice, SharedBackend.scratch.get(),
        size, MaxViews);
    if (result != arm::FFXM_OK)
        SharedBackend = {};
    return translateResult(result);
}

arm::FfxmSurfaceFormat surfaceFormat(uint32_t format)
{
    switch (format) {
    case FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT: return arm::FFXM_SURFACE_FORMAT_R16G16B16A16_FLOAT;
    case FFX_API_SURFACE_FORMAT_R16G16_FLOAT: return arm::FFXM_SURFACE_FORMAT_R16G16_FLOAT;
    case FFX_API_SURFACE_FORMAT_R32_FLOAT: return arm::FFXM_SURFACE_FORMAT_R32_FLOAT;
    case FFX_API_SURFACE_FORMAT_R16_FLOAT: return arm::FFXM_SURFACE_FORMAT_R16_FLOAT;
    case FFX_API_SURFACE_FORMAT_R8_UNORM: return arm::FFXM_SURFACE_FORMAT_R8_UNORM;
    default: return arm::FFXM_SURFACE_FORMAT_UNKNOWN;
    }
}

arm::FfxmResource image(const FfxApiResource& resource)
{
    if (!resource.resource)
        return {};

    arm::FfxmResourceDescription desc{};
    desc.type = arm::FFXM_RESOURCE_TYPE_TEXTURE2D;
    desc.format = surfaceFormat(resource.description.format);
    desc.width = resource.description.width;
    desc.height = resource.description.height;
    desc.depth = resource.description.depth;
    desc.mipCount = resource.description.mipCount;
    const bool output = resource.state == FFX_API_RESOURCE_STATE_RENDER_TARGET;
    desc.usage = output ? arm::FFXM_RESOURCE_USAGE_RENDERTARGET : arm::FFXM_RESOURCE_USAGE_READ_ONLY;
    return arm::ffxmGetResourceVK(resource.resource, desc, nullptr,
        output ? arm::FFXM_RESOURCE_STATE_PIXEL_WRITE : arm::FFXM_RESOURCE_STATE_PIXEL_COMPUTE_READ);
}

} // namespace

void ffxPortableVkGetDeviceRequirements(VkInstance instance, VkPhysicalDevice gpu,
                                        PFN_vkGetInstanceProcAddr get, ffxPortableVkRequirements* out)
{
    *out = {};
    out->supported = false;
    out->minApiVersion = VK_API_VERSION_1_2;
    UnavailableReason = "missing Vulkan instance functions";
    const auto getFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(get(instance, "vkGetPhysicalDeviceFeatures2"));
    const auto getProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(get(instance, "vkGetPhysicalDeviceProperties2"));
    if (!getFeatures || !getProperties || !get(instance, "vkGetPhysicalDeviceMemoryProperties")
        || !get(instance, "vkEnumerateDeviceExtensionProperties"))
        return;

    using ffx_portable_detail::make;
    auto float16 = make<VkPhysicalDeviceShaderFloat16Int8Features>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES);
    auto features = make<VkPhysicalDeviceFeatures2>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
    features.pNext = &float16;
    getFeatures(gpu, &features);

    // The bundled SPIR-V uses Float16, Int16, storage image extended formats,
    // subgroup quad operations in compute, and storage image atomics in fragment.
    UnavailableReason = "GPU lacks Arm ASR shader features";
    if (!float16.shaderFloat16 || !features.features.shaderInt16 || !features.features.fragmentStoresAndAtomics
        || !features.features.shaderStorageImageExtendedFormats)
        return;

    auto subgroup = make<VkPhysicalDeviceSubgroupProperties>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES);
    auto properties = make<VkPhysicalDeviceProperties2>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2);
    properties.pNext = &subgroup;
    getProperties(gpu, &properties);
    UnavailableReason = "Arm ASR needs Vulkan 1.2 and compute subgroup quad operations";
    if (properties.properties.apiVersion < out->minApiVersion
        || !(subgroup.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT)
        || !(subgroup.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT))
        return;

    out->float16Int8 = float16;
    out->float16Int8.shaderInt8 = VK_FALSE;
    out->core.shaderInt16 = VK_TRUE;
    out->core.fragmentStoresAndAtomics = VK_TRUE;
    out->core.shaderStorageImageExtendedFormats = VK_TRUE;
    out->supported = true;
    Instance = instance;
    PhysicalDevice = gpu;
    GetInstanceProcAddr = get;
    UnavailableReason = "";
}

extern "C" {

bool ffxPortableCanLoad() { return true; }
const char* ffxPortableLoadError() { return UnavailableReason; }

ffxReturnCode_t ffxCreateContext(ffxContext* context, ffxCreateContextDescHeader* header,
                                const ffxAllocationCallbacks* memCb)
{
    if (!context || !header || memCb)
        return FFX_API_RETURN_ERROR_PARAMETER;
    if (header->type != FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE)
        return FFX_API_RETURN_NO_PROVIDER;

    const ffxCreateBackendVKDesc* backend = nullptr;
    for (auto* node = header->pNext; node; node = node->pNext) {
        if (node->type == FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK)
            backend = reinterpret_cast<const ffxCreateBackendVKDesc*>(node);
    }
    if (!backend || backend->deviceContext.vkPhysicalDevice != PhysicalDevice
        || !backend->deviceContext.vkDevice || !backend->deviceContext.vkDeviceProcAddr)
        return FFX_API_RETURN_ERROR_PARAMETER;
    if (SharedBackend.views >= MaxViews)
        return FFX_API_RETURN_ERROR_MEMORY;

    std::unique_ptr<Context> instance(new (std::nothrow) Context);
    if (!instance)
        return FFX_API_RETURN_ERROR_MEMORY;
    const auto init = initBackend(backend->deviceContext);
    if (init != FFX_API_RETURN_OK)
        return init;

    const auto& desc = *reinterpret_cast<const ffxCreateContextDescUpscale*>(header);
    arm::FfxmFsr2ContextDescription asr{};
    asr.backendInterface = SharedBackend.interface;
    asr.qualityMode = arm::FFXM_FSR2_SHADER_QUALITY_MODE_PERFORMANCE;
    // These are the AMD flags whose bit positions match Arm's API.
    asr.flags = desc.flags & (FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE | FFX_UPSCALE_ENABLE_AUTO_EXPOSURE
        | FFX_UPSCALE_ENABLE_DEPTH_INVERTED | FFX_UPSCALE_ENABLE_DEPTH_INFINITE
        | FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS | FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION
        | FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION);
    if (desc.flags & FFX_UPSCALE_ENABLE_DEBUG_CHECKING)
        asr.flags |= arm::FFXM_FSR2_ENABLE_DEBUG_CHECKING;
    asr.maxRenderSize = {desc.maxRenderSize.width, desc.maxRenderSize.height};
    asr.displaySize = {desc.maxUpscaleSize.width, desc.maxUpscaleSize.height};
    ClientMessage = desc.fpMessage;
    asr.fpMessage = logAsr;

    const auto result = arm::ffxmFsr2ContextCreate(&instance->asr, &asr);
    if (result != arm::FFXM_OK) {
        if (!SharedBackend.views)
            SharedBackend = {};
        return translateResult(result);
    }
    instance->outputSize = asr.displaySize;
    ++SharedBackend.views;
    *context = instance.release();
    return FFX_API_RETURN_OK;
}

ffxReturnCode_t ffxDestroyContext(ffxContext* context, const ffxAllocationCallbacks*)
{
    if (!context || !*context)
        return FFX_API_RETURN_ERROR_PARAMETER;
    std::unique_ptr<Context> instance(static_cast<Context*>(*context));
    const auto result = arm::ffxmFsr2ContextDestroy(&instance->asr);
    *context = nullptr;
    if (--SharedBackend.views == 0)
        SharedBackend = {};
    return translateResult(result);
}

ffxReturnCode_t ffxConfigure(ffxContext*, const ffxConfigureDescHeader*)
{
    return FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE;
}

ffxReturnCode_t ffxQuery(ffxContext* context, ffxQueryDescHeader* header)
{
    if (!header)
        return FFX_API_RETURN_ERROR_PARAMETER;
    switch (header->type) {
    case FFX_API_QUERY_DESC_TYPE_GET_VERSIONS: {
        auto& query = *reinterpret_cast<ffxQueryDescGetVersions*>(header);
        const auto* vk = static_cast<const VkDeviceContext*>(query.device);
        if (query.createDescType != FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE || !vk
            || vk->vkPhysicalDevice != PhysicalDevice)
            return FFX_API_RETURN_NO_PROVIDER;
        if (!query.outputCount)
            return FFX_API_RETURN_ERROR_PARAMETER;
        if (*query.outputCount > 0) {
            if (query.versionIds)
                query.versionIds[0] = ProviderId;
            if (query.versionNames)
                query.versionNames[0] = ProviderName;
        }
        *query.outputCount = 1;
        return FFX_API_RETURN_OK;
    }
    case FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION: {
        if (!context || !*context)
            return FFX_API_RETURN_ERROR_PARAMETER;
        auto& query = *reinterpret_cast<ffxQueryGetProviderVersion*>(header);
        query.versionId = ProviderId;
        query.versionName = ProviderName;
        return FFX_API_RETURN_OK;
    }
    case FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTERPHASECOUNT: {
        const auto& query = *reinterpret_cast<const ffxQueryDescUpscaleGetJitterPhaseCount*>(header);
        if (!query.pOutPhaseCount || !query.renderWidth || !query.displayWidth)
            return FFX_API_RETURN_ERROR_PARAMETER;
        *query.pOutPhaseCount = arm::ffxmFsr2GetJitterPhaseCount(query.renderWidth, query.displayWidth);
        return FFX_API_RETURN_OK;
    }
    case FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTEROFFSET: {
        const auto& query = *reinterpret_cast<const ffxQueryDescUpscaleGetJitterOffset*>(header);
        if (!query.pOutX || !query.pOutY || query.phaseCount <= 0)
            return FFX_API_RETURN_ERROR_PARAMETER;
        return translateResult(arm::ffxmFsr2GetJitterOffset(query.pOutX, query.pOutY, query.index, query.phaseCount));
    }
    default:
        return FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE;
    }
}

ffxReturnCode_t ffxDispatch(ffxContext* context, const ffxDispatchDescHeader* header)
{
    if (!context || !*context || !header)
        return FFX_API_RETURN_ERROR_PARAMETER;
    if (header->type != FFX_API_DISPATCH_DESC_TYPE_UPSCALE)
        return FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE;

    auto& instance = *static_cast<Context*>(*context);
    const auto& desc = *reinterpret_cast<const ffxDispatchDescUpscale*>(header);
    if (!desc.commandList || desc.flags || desc.output.state != FFX_API_RESOURCE_STATE_RENDER_TARGET
        || desc.output.description.width != instance.outputSize.width
        || desc.output.description.height != instance.outputSize.height)
        return FFX_API_RETURN_ERROR_PARAMETER;

    arm::FfxmFsr2DispatchDescription asr{};
    asr.commandList = arm::ffxmGetCommandListVK(static_cast<VkCommandBuffer>(desc.commandList));
    asr.color = image(desc.color);
    asr.depth = image(desc.depth);
    asr.motionVectors = image(desc.motionVectors);
    asr.exposure = image(desc.exposure);
    asr.reactive = image(desc.reactive);
    asr.transparencyAndComposition = image(desc.transparencyAndComposition);
    asr.output = image(desc.output);
    asr.jitterOffset = {desc.jitterOffset.x, desc.jitterOffset.y};
    asr.motionVectorScale = {desc.motionVectorScale.x, desc.motionVectorScale.y};
    asr.renderSize = {desc.renderSize.width, desc.renderSize.height};
    asr.enableSharpening = desc.enableSharpening;
    asr.sharpness = desc.sharpness;
    asr.frameTimeDelta = desc.frameTimeDelta;
    asr.preExposure = desc.preExposure;
    asr.reset = desc.reset;
    asr.cameraNear = desc.cameraNear;
    asr.cameraFar = desc.cameraFar;
    asr.cameraFovAngleVertical = desc.cameraFovAngleVertical;
    asr.viewSpaceToMetersFactor = desc.viewSpaceToMetersFactor;
    return translateResult(arm::ffxmFsr2ContextDispatch(&instance.asr, &asr));
}

} // extern "C"
