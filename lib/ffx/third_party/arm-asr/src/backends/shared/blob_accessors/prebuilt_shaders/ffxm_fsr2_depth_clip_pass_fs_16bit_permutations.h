#include "ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67.h"
#include "ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d.h"

typedef union ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationKey {
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
} ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationKey;

typedef struct ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationInfo {
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
} ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationInfo;

static const uint32_t g_ffxm_fsr2_depth_clip_pass_fs_16bit_IndirectionTable[] = {
    22,
    22,
    11,
    11,
    16,
    16,
    4,
    4,
    19,
    19,
    7,
    7,
    14,
    14,
    5,
    5,
    22,
    22,
    11,
    11,
    16,
    16,
    4,
    4,
    19,
    19,
    7,
    7,
    14,
    14,
    5,
    5,
    22,
    22,
    11,
    11,
    16,
    16,
    4,
    4,
    19,
    19,
    7,
    7,
    14,
    14,
    5,
    5,
    22,
    22,
    11,
    11,
    16,
    16,
    4,
    4,
    19,
    19,
    7,
    7,
    14,
    14,
    5,
    5,
    23,
    23,
    10,
    10,
    17,
    17,
    3,
    3,
    20,
    20,
    8,
    8,
    13,
    13,
    2,
    2,
    23,
    23,
    10,
    10,
    17,
    17,
    3,
    3,
    20,
    20,
    8,
    8,
    13,
    13,
    2,
    2,
    23,
    23,
    10,
    10,
    17,
    17,
    3,
    3,
    20,
    20,
    8,
    8,
    13,
    13,
    2,
    2,
    23,
    23,
    10,
    10,
    17,
    17,
    3,
    3,
    20,
    20,
    8,
    8,
    13,
    13,
    2,
    2,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
    21,
    21,
    9,
    9,
    15,
    15,
    1,
    1,
    18,
    18,
    6,
    6,
    12,
    12,
    0,
    0,
};

static const ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationInfo g_ffxm_fsr2_depth_clip_pass_fs_16bit_PermutationInfo[] = {
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4e67facb7312f373b77ae1a15c481466_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_ce2fa7e1491fe9d456825de08b7ee3f9_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d6ef4fd7c631cec964f63e18999b3526_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_6af55c993723c6b96b2ac020c46c5d71_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_3cedab3890a89d7dab93c4f1db7cd22c_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_353badb555d12e0c0534117c690ba52e_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_fb482d1c41d9a6f77e56b73ef0895592_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53e144afd2d5dbc708d68b618587cefb_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_1422720861b2aed9bb111c79c08a9829_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_5439079d496ad9e361695fec7d616f5b_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_174658b04216488b25c5b100be7b3fe6_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_8b67bea8b0538d0b1e00394a01f8511e_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d711b31a9c8e36001adffd17ff5bf273_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_41c1b4fae01bc3b7bb17a9acac6f4a5b_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_92c7b3e1533069b06d3f0e7c4f135f72_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e6151bd64cb5eb98b4720ea6f9dfbb2c_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_d3bda3bd8cca7de331372b8a7ca2305d_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_302a42a7c8a652d8dfcba532a55e4239_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_4dc302c7bb2dcf710be905249f04650e_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_423aa1b67dfb2ce8674149c8856c4e38_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_70a19c0a9c4bb9f5a41cc5f80673b652_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_CBVResourceSpaces, 4, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_SamplerResourceSpaces, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_2cd72fe071c7bd9ee6fdbad15827505e_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_53b502a69ee0286d50c6c4256b69ec67_TextureRTResourceSpaces, },
    { g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_size, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_data, 1, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_CBVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_CBVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_CBVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_CBVResourceSpaces, 9, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureSRVResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureSRVResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureSRVResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureSRVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_SamplerResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_SamplerResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_SamplerResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_SamplerResourceSpaces, 0, 0, 0, 0, 0, 2, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureRTResourceNames, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureRTResourceBindings, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureRTResourceCounts, g_ffxm_fsr2_depth_clip_pass_fs_16bit_e083d8480424193a0d804a77ac1ecd6d_TextureRTResourceSpaces, },
};

