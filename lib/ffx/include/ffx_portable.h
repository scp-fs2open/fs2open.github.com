// Additions to AMD's FSR API that are needed on every platform. Clients use AMD's headers and the
// ffx* functions exactly as AMD documents them; this header adds only what AMD's API lacks.
//
// AMD's API has no way to ask which Vulkan extensions and features FSR needs, yet the device must be
// created with them before any FSR context exists. AMD's runtime picks its shaders from what the
// device supports, not from what was enabled, so skipping this can produce invalid Vulkan.
#pragma once

#include <FidelityFX/api/include/vk/ffx_api_vk.h>
#include <FidelityFX/upscalers/include/ffx_upscale.h>

#include <cstdint>
#include <cstring>
#include <vector>

#if defined(FFX_PORTABLE_DIRECT)
// Only with the DIRECT build option. The vendor runtime is linked directly: if the process started, it is available.
inline bool ffxPortableCanLoad() { return true; }
inline const char* ffxPortableLoadError() { return ""; }
#else
// Loads the runtime on first use and caches the result. False means every ffx* call returns
// FFX_API_RETURN_NO_PROVIDER and the application should run without FSR.
extern "C" bool ffxPortableCanLoad();
// Why ffxPortableCanLoad() is false, for logging. Empty when it is true.
extern "C" const char* ffxPortableLoadError();
#endif

struct ffxPortableVkRequirements
{
    uint32_t minApiVersion;  // The instance and device must be created with at least this version.
    bool     fp8Capable;     // The device can run the FP8 model, given these requirements are applied.
    bool     int8Capable;    // The device can run an INT8 model, given these requirements are applied.

    uint32_t    extensionCount;
    const char* extensions[8];

    VkPhysicalDeviceFeatures                core;
    VkPhysicalDeviceShaderFloat16Int8Features float16Int8;
    VkPhysicalDevice16BitStorageFeatures    storage16;
    VkPhysicalDeviceVulkanMemoryModelFeatures memoryModel;
    VkPhysicalDeviceVariablePointersFeatures variablePointers;
    VkPhysicalDeviceShaderIntegerDotProductFeatures dotProduct;
    VkPhysicalDeviceSubgroupSizeControlFeatures subgroupSizeControl;
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing;
#if defined(VK_KHR_cooperative_matrix)
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR coopMatrix;
#endif
#if defined(VK_EXT_shader_float8)
    VkPhysicalDeviceShaderFloat8FeaturesEXT float8;
#endif
#if defined(VK_NV_cooperative_matrix2)
    VkPhysicalDeviceCooperativeMatrix2FeaturesNV coopMatrix2;
#endif
#if defined(VK_KHR_compute_shader_derivatives)
    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR derivatives;
#endif

    // Storage used by ffxPortableVkApplyRequirements; must outlive vkCreateDevice.
    VkPhysicalDeviceFeatures mergedCore;
    const char*              mergedExtensions[128];
};

namespace ffx_portable_detail
{
    template <typename T>
    T make(VkStructureType type)
    {
        T value{};
        value.sType = type;
        return value;
    }

    inline VkBaseOutStructure* find(const void* chain, VkStructureType type)
    {
        for (auto* node = static_cast<const VkBaseOutStructure*>(chain); node; node = node->pNext)
            if (node->sType == type)
                return const_cast<VkBaseOutStructure*>(node);
        return nullptr;
    }

    // Feature structs are a header followed only by VkBool32 members.
    template <typename T>
    void merge(VkDeviceCreateInfo* info, T& ours)
    {
        const size_t count = (sizeof(T) - sizeof(VkBaseOutStructure)) / sizeof(VkBool32);
        auto* source = reinterpret_cast<VkBool32*>(reinterpret_cast<char*>(&ours) + sizeof(VkBaseOutStructure));
        bool any = false;
        for (size_t i = 0; i < count; ++i)
            any = any || source[i];
        if (!any)
            return;

        if (VkBaseOutStructure* existing = find(info->pNext, ours.sType))
        {
            auto* target = reinterpret_cast<VkBool32*>(reinterpret_cast<char*>(existing) + sizeof(VkBaseOutStructure));
            for (size_t i = 0; i < count; ++i)
                target[i] = target[i] || source[i];
        }
        else
        {
            ours.pNext = const_cast<void*>(info->pNext);
            info->pNext = &ours;
        }
    }

    inline void mergeCore(VkPhysicalDeviceFeatures& target, const VkPhysicalDeviceFeatures& source)
    {
        auto* out = reinterpret_cast<VkBool32*>(&target);
        auto* in = reinterpret_cast<const VkBool32*>(&source);
        for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++i)
            out[i] = out[i] || in[i];
    }
}  // namespace ffx_portable_detail

// Fills `out` with what FSR needs and this device supports. Call before vkCreateDevice. Takes the
// client's loader entry point so that VK_NO_PROTOTYPES clients work.
inline void ffxPortableVkGetDeviceRequirements(VkInstance instance, VkPhysicalDevice gpu, PFN_vkGetInstanceProcAddr getInstanceProcAddr,
                                               ffxPortableVkRequirements* out)
{
    using ffx_portable_detail::make;

    *out = {};
    out->minApiVersion = VK_API_VERSION_1_2;

    auto enumerate = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(getInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties"));
    auto getFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2"));
    auto getProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
    if (!enumerate || !getFeatures2 || !getProperties)
        return;

    VkPhysicalDeviceProperties properties{};
    getProperties(gpu, &properties);
    if (properties.apiVersion < out->minApiVersion)
        return;

    uint32_t count = 0;
    enumerate(gpu, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    enumerate(gpu, nullptr, &count, available.data());
    auto has = [&](const char* name) {
        for (const auto& extension : available)
            if (!std::strcmp(extension.extensionName, name))
                return true;
        return false;
    };

    auto float16Int8 = make<VkPhysicalDeviceShaderFloat16Int8Features>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES);
    auto storage16 = make<VkPhysicalDevice16BitStorageFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
    auto memoryModel = make<VkPhysicalDeviceVulkanMemoryModelFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES);
    auto variablePointers = make<VkPhysicalDeviceVariablePointersFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES);
    auto dotProduct = make<VkPhysicalDeviceShaderIntegerDotProductFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES);
    auto subgroupSizeControl = make<VkPhysicalDeviceSubgroupSizeControlFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES);
    auto descriptorIndexing = make<VkPhysicalDeviceDescriptorIndexingFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES);
    auto features = make<VkPhysicalDeviceFeatures2>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);

    float16Int8.pNext = &storage16;
    storage16.pNext = &memoryModel;
    memoryModel.pNext = &variablePointers;
    variablePointers.pNext = &descriptorIndexing;
    void* tail = &descriptorIndexing;
    auto link = [&](auto& feature) {
        static_cast<VkBaseOutStructure*>(tail)->pNext = reinterpret_cast<VkBaseOutStructure*>(&feature);
        tail = &feature;
    };

    const bool hasDotProduct = properties.apiVersion >= VK_API_VERSION_1_3 || has(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME);
    if (hasDotProduct)
        link(dotProduct);
    const bool hasSubgroupSizeControl = properties.apiVersion >= VK_API_VERSION_1_3 || has(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    if (hasSubgroupSizeControl)
        link(subgroupSizeControl);

#if defined(VK_KHR_cooperative_matrix)
    auto coopMatrix = make<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR);
    const bool hasCoopMatrix = has(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    if (hasCoopMatrix)
        link(coopMatrix);
#else
    const bool hasCoopMatrix = false;
#endif
#if defined(VK_EXT_shader_float8)
    auto float8 = make<VkPhysicalDeviceShaderFloat8FeaturesEXT>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT);
    const bool hasFloat8 = has(VK_EXT_SHADER_FLOAT8_EXTENSION_NAME);
    if (hasFloat8)
        link(float8);
#else
    const bool hasFloat8 = false;
#endif
#if defined(VK_NV_cooperative_matrix2)
    auto coopMatrix2 = make<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV);
    const bool hasCoopMatrix2 = has(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    if (hasCoopMatrix2)
        link(coopMatrix2);
#endif
#if defined(VK_KHR_compute_shader_derivatives)
    auto derivatives = make<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR);
    const bool hasDerivatives = has(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    if (hasDerivatives)
        link(derivatives);
#else
    const bool hasDerivatives = false;
#endif

    features.pNext = &float16Int8;
    getFeatures2(gpu, &features);

    // Every family of AMD's shaders uses these.
    const VkPhysicalDeviceFeatures& core = features.features;
    if (!core.shaderStorageImageExtendedFormats || !core.shaderStorageImageWriteWithoutFormat)
        return;
    out->core.shaderStorageImageExtendedFormats = VK_TRUE;
    out->core.shaderStorageImageWriteWithoutFormat = VK_TRUE;
    out->core.shaderInt16 = core.shaderInt16;

    out->float16Int8 = float16Int8;
    out->float16Int8.pNext = nullptr;

    auto addExtension = [&](const char* name) { out->extensions[out->extensionCount++] = name; };

    // These two aren't visible in the shaders: AMD's host code asks for them when creating pipelines and
    // descriptor layouts, which validation reports if they're missing.
    out->subgroupSizeControl = make<VkPhysicalDeviceSubgroupSizeControlFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES);
    const bool sizeControl = hasSubgroupSizeControl && subgroupSizeControl.subgroupSizeControl && subgroupSizeControl.computeFullSubgroups;
    if (sizeControl)
    {
        out->subgroupSizeControl.subgroupSizeControl = VK_TRUE;
        out->subgroupSizeControl.computeFullSubgroups = VK_TRUE;
        if (has(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME))
            addExtension(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    }
    out->descriptorIndexing = make<VkPhysicalDeviceDescriptorIndexingFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES);
    out->descriptorIndexing.descriptorBindingPartiallyBound = descriptorIndexing.descriptorBindingPartiallyBound;

    const bool baseMl = float16Int8.shaderFloat16 && float16Int8.shaderInt8 && sizeControl && descriptorIndexing.descriptorBindingPartiallyBound;

#if defined(VK_KHR_compute_shader_derivatives)
    out->derivatives = make<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR);
    const bool linearDerivatives = hasDerivatives && derivatives.computeDerivativeGroupLinear;
    if (linearDerivatives)
    {
        out->derivatives.computeDerivativeGroupLinear = VK_TRUE;
        addExtension(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    }
#else
    const bool linearDerivatives = false;
#endif

    out->dotProduct = make<VkPhysicalDeviceShaderIntegerDotProductFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES);
    if (hasDotProduct && dotProduct.shaderIntegerDotProduct)
    {
        out->dotProduct.shaderIntegerDotProduct = VK_TRUE;
        if (has(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME))
            addExtension(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME);
        out->int8Capable = baseMl && linearDerivatives;
    }

    out->storage16 = make<VkPhysicalDevice16BitStorageFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
    out->memoryModel = make<VkPhysicalDeviceVulkanMemoryModelFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES);
    out->variablePointers = make<VkPhysicalDeviceVariablePointersFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES);
#if defined(VK_KHR_cooperative_matrix) && defined(VK_EXT_shader_float8)
    out->coopMatrix = make<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR);
    out->float8 = make<VkPhysicalDeviceShaderFloat8FeaturesEXT>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT);
    if (hasCoopMatrix && hasFloat8 && coopMatrix.cooperativeMatrix && float8.shaderFloat8 && float8.shaderFloat8CooperativeMatrix && baseMl &&
        core.shaderInt16 && storage16.storageBuffer16BitAccess && memoryModel.vulkanMemoryModel && memoryModel.vulkanMemoryModelDeviceScope &&
        variablePointers.variablePointers && variablePointers.variablePointersStorageBuffer)
    {
        out->fp8Capable = true;
        out->coopMatrix.cooperativeMatrix = VK_TRUE;
        out->float8.shaderFloat8 = VK_TRUE;
        out->float8.shaderFloat8CooperativeMatrix = VK_TRUE;
        out->storage16.storageBuffer16BitAccess = VK_TRUE;
        out->memoryModel.vulkanMemoryModel = VK_TRUE;
        out->memoryModel.vulkanMemoryModelDeviceScope = VK_TRUE;
        out->variablePointers.variablePointers = VK_TRUE;
        out->variablePointers.variablePointersStorageBuffer = VK_TRUE;
        addExtension(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
        addExtension(VK_EXT_SHADER_FLOAT8_EXTENSION_NAME);
#if defined(VK_NV_cooperative_matrix2)
        // AMD's FP8 shaders declare this, though RADV hides it by default and compiles them regardless.
        out->coopMatrix2 = make<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV);
        if (hasCoopMatrix2 && coopMatrix2.cooperativeMatrixConversions)
        {
            out->coopMatrix2.cooperativeMatrixConversions = VK_TRUE;
            addExtension(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
        }
#endif
    }
#endif
}

// Adds the requirements to a device create info, merging with what the client already requests.
// Handles both pEnabledFeatures and VkPhysicalDeviceFeatures2, and existing feature structs including
// the VulkanXYFeatures aggregates. `requirements` must outlive vkCreateDevice and be applied only once.
// False means the combined extension list did not fit, and nothing about extensions was changed.
inline bool ffxPortableVkApplyRequirements(ffxPortableVkRequirements* requirements, VkDeviceCreateInfo* info)
{
    using namespace ffx_portable_detail;

    const uint32_t capacity = sizeof(requirements->mergedExtensions) / sizeof(requirements->mergedExtensions[0]);
    if (info->enabledExtensionCount + requirements->extensionCount > capacity)
        return false;

    uint32_t merged = 0;
    for (uint32_t i = 0; i < info->enabledExtensionCount; ++i)
        requirements->mergedExtensions[merged++] = info->ppEnabledExtensionNames[i];
    for (uint32_t i = 0; i < requirements->extensionCount; ++i)
    {
        bool present = false;
        for (uint32_t k = 0; k < info->enabledExtensionCount; ++k)
            present = present || !std::strcmp(info->ppEnabledExtensionNames[k], requirements->extensions[i]);
        if (!present)
            requirements->mergedExtensions[merged++] = requirements->extensions[i];
    }
    info->enabledExtensionCount = merged;
    info->ppEnabledExtensionNames = requirements->mergedExtensions;

    if (auto* features2 = reinterpret_cast<VkPhysicalDeviceFeatures2*>(find(info->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2)))
    {
        mergeCore(features2->features, requirements->core);
    }
    else
    {
        requirements->mergedCore = info->pEnabledFeatures ? *info->pEnabledFeatures : VkPhysicalDeviceFeatures{};
        mergeCore(requirements->mergedCore, requirements->core);
        info->pEnabledFeatures = &requirements->mergedCore;
    }

    // The spec forbids chaining a VulkanXYFeatures aggregate alongside the structs it covers.
    if (auto* vk11 = reinterpret_cast<VkPhysicalDeviceVulkan11Features*>(find(info->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES)))
    {
        vk11->storageBuffer16BitAccess = vk11->storageBuffer16BitAccess || requirements->storage16.storageBuffer16BitAccess;
        vk11->variablePointers = vk11->variablePointers || requirements->variablePointers.variablePointers;
        vk11->variablePointersStorageBuffer = vk11->variablePointersStorageBuffer || requirements->variablePointers.variablePointersStorageBuffer;
    }
    else
    {
        merge(info, requirements->storage16);
        merge(info, requirements->variablePointers);
    }

    if (auto* vk12 = reinterpret_cast<VkPhysicalDeviceVulkan12Features*>(find(info->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES)))
    {
        vk12->shaderFloat16 = vk12->shaderFloat16 || requirements->float16Int8.shaderFloat16;
        vk12->shaderInt8 = vk12->shaderInt8 || requirements->float16Int8.shaderInt8;
        vk12->vulkanMemoryModel = vk12->vulkanMemoryModel || requirements->memoryModel.vulkanMemoryModel;
        vk12->vulkanMemoryModelDeviceScope = vk12->vulkanMemoryModelDeviceScope || requirements->memoryModel.vulkanMemoryModelDeviceScope;
        vk12->descriptorBindingPartiallyBound = vk12->descriptorBindingPartiallyBound || requirements->descriptorIndexing.descriptorBindingPartiallyBound;
    }
    else
    {
        merge(info, requirements->float16Int8);
        merge(info, requirements->memoryModel);
        merge(info, requirements->descriptorIndexing);
    }

    if (auto* vk13 = reinterpret_cast<VkPhysicalDeviceVulkan13Features*>(find(info->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES)))
    {
        vk13->shaderIntegerDotProduct = vk13->shaderIntegerDotProduct || requirements->dotProduct.shaderIntegerDotProduct;
        vk13->subgroupSizeControl = vk13->subgroupSizeControl || requirements->subgroupSizeControl.subgroupSizeControl;
        vk13->computeFullSubgroups = vk13->computeFullSubgroups || requirements->subgroupSizeControl.computeFullSubgroups;
    }
    else
    {
        merge(info, requirements->dotProduct);
        merge(info, requirements->subgroupSizeControl);
    }

#if defined(VK_KHR_cooperative_matrix)
    merge(info, requirements->coopMatrix);
#endif
#if defined(VK_EXT_shader_float8)
    merge(info, requirements->float8);
#endif
#if defined(VK_NV_cooperative_matrix2)
    merge(info, requirements->coopMatrix2);
#endif
#if defined(VK_KHR_compute_shader_derivatives)
    merge(info, requirements->derivatives);
#endif
    return true;
}
