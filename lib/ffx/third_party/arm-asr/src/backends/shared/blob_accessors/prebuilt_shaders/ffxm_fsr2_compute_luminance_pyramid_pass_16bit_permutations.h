#include "ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb.h"
#include "ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53.h"

typedef union ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationKey {
    struct {
        uint32_t FFXM_FSR2_OPTION_HDR_COLOR_INPUT : 1;
        uint32_t FFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS : 1;
        uint32_t FFXM_FSR2_OPTION_JITTERED_MOTION_VECTORS : 1;
        uint32_t FFXM_FSR2_OPTION_INVERTED_DEPTH : 1;
        uint32_t FFXM_FSR2_OPTION_APPLY_SHARPENING : 1;
        uint32_t FFXM_FSR2_OPTION_SHADER_OPT_BALANCED : 1;
        uint32_t FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE : 1;
        uint32_t FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE : 1;
    };
    uint32_t index;
} ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationKey;

typedef struct ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationInfo {
    const uint32_t       blobSize;
    const unsigned char* blobData;


    const uint32_t  numConstantBuffers;
    const char**    constantBufferNames;
    const uint32_t* constantBufferBindings;
    const uint32_t* constantBufferCounts;
    const uint32_t* constantBufferSpaces;

    const uint32_t  numSRVTextures;
    const char**    srvTextureNames;
    const uint32_t* srvTextureBindings;
    const uint32_t* srvTextureCounts;
    const uint32_t* srvTextureSpaces;

    const uint32_t  numUAVTextures;
    const char**    uavTextureNames;
    const uint32_t* uavTextureBindings;
    const uint32_t* uavTextureCounts;
    const uint32_t* uavTextureSpaces;

    const uint32_t  numSRVBuffers;
    const char**    srvBufferNames;
    const uint32_t* srvBufferBindings;
    const uint32_t* srvBufferCounts;
    const uint32_t* srvBufferSpaces;

    const uint32_t  numUAVBuffers;
    const char**    uavBufferNames;
    const uint32_t* uavBufferBindings;
    const uint32_t* uavBufferCounts;
    const uint32_t* uavBufferSpaces;

    const uint32_t  numSamplers;
    const char**    samplerNames;
    const uint32_t* samplerBindings;
    const uint32_t* samplerCounts;
    const uint32_t* samplerSpaces;

    const uint32_t  numRTAccelerationStructures;
    const char**    rtAccelerationStructureNames;
    const uint32_t* rtAccelerationStructureBindings;
    const uint32_t* rtAccelerationStructureCounts;
    const uint32_t* rtAccelerationStructureSpaces;

    const uint32_t  numRTTextures;
    const char**    rtTextureNames;
    const uint32_t* rtTextureBindings;
    const uint32_t* rtTextureCounts;
    const uint32_t* rtTextureSpaces;
} ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationInfo;

static const uint32_t g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_IndirectionTable[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
};

static const ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationInfo g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_PermutationInfo[] = {
    { g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_size, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_data, 2, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_CBVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_CBVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_CBVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_CBVResourceSpaces, 1, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureSRVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureSRVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureSRVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureSRVResourceSpaces, 4, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureUAVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureUAVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureUAVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_SamplerResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_SamplerResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_SamplerResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_96064b9d98373791c5adbcff87982deb_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_size, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_data, 2, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_CBVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_CBVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_CBVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_CBVResourceSpaces, 1, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureSRVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureSRVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureSRVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureSRVResourceSpaces, 4, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureUAVResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureUAVResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureUAVResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_SamplerResourceNames, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_SamplerResourceBindings, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_SamplerResourceCounts, g_ffxm_fsr2_compute_luminance_pyramid_pass_16bit_fc667413de4bc14a1646afdc6ebfea53_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

