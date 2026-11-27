#include "ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906.h"
#include "ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49.h"

typedef union ffxm_fsr2_rcas_pass_fs_16bit_PermutationKey {
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
} ffxm_fsr2_rcas_pass_fs_16bit_PermutationKey;

typedef struct ffxm_fsr2_rcas_pass_fs_16bit_PermutationInfo {
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
} ffxm_fsr2_rcas_pass_fs_16bit_PermutationInfo;

static const uint32_t g_ffxm_fsr2_rcas_pass_fs_16bit_IndirectionTable[] = {
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
};

static const ffxm_fsr2_rcas_pass_fs_16bit_PermutationInfo g_ffxm_fsr2_rcas_pass_fs_16bit_PermutationInfo[] = {
    { g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_size, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_data, 1, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_CBVResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_CBVResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_CBVResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_CBVResourceSpaces, 1, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureSRVResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureSRVResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureSRVResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureRTResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureRTResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureRTResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_989ca3aa2569246549715b86945d0906_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_size, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_data, 2, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_CBVResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_CBVResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_CBVResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_CBVResourceSpaces, 2, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureSRVResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureSRVResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureSRVResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureRTResourceNames, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureRTResourceBindings, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureRTResourceCounts, g_ffxm_fsr2_rcas_pass_fs_16bit_94718e98433509ab5cf3548f460bee49_TextureRTResourceSpaces, },
};

