#include "ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57.h"
#include "ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9.h"
#include "ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28.h"
#include "ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626.h"

typedef union ffxm_fsr2_lock_pass_16bit_PermutationKey {
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
} ffxm_fsr2_lock_pass_16bit_PermutationKey;

typedef struct ffxm_fsr2_lock_pass_16bit_PermutationInfo {
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
} ffxm_fsr2_lock_pass_16bit_PermutationInfo;

static const uint32_t g_ffxm_fsr2_lock_pass_16bit_IndirectionTable[] = {
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
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
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
};

static const ffxm_fsr2_lock_pass_16bit_PermutationInfo g_ffxm_fsr2_lock_pass_16bit_PermutationInfo[] = {
    { g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_size, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_data, 1, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_CBVResourceNames, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_CBVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_CBVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_CBVResourceSpaces, 1, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureSRVResourceNames, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureSRVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureSRVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureSRVResourceSpaces, 2, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureUAVResourceNames, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureUAVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureUAVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_SamplerResourceNames, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_SamplerResourceBindings, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_SamplerResourceCounts, g_ffxm_fsr2_lock_pass_16bit_5b6370049fc779ba79a7ab9e010e1e57_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_size, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_data, 1, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_CBVResourceNames, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_CBVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_CBVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_CBVResourceSpaces, 1, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureSRVResourceNames, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureSRVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureSRVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureSRVResourceSpaces, 2, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureUAVResourceNames, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureUAVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureUAVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_SamplerResourceNames, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_SamplerResourceBindings, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_SamplerResourceCounts, g_ffxm_fsr2_lock_pass_16bit_2f1c18c8732678ef5cb40542fbe62af9_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_size, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_data, 1, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_CBVResourceNames, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_CBVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_CBVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_CBVResourceSpaces, 1, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureSRVResourceNames, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureSRVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureSRVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureSRVResourceSpaces, 2, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureUAVResourceNames, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureUAVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureUAVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_SamplerResourceNames, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_SamplerResourceBindings, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_SamplerResourceCounts, g_ffxm_fsr2_lock_pass_16bit_dcad9a9382192898ccc8f761178f4f28_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_size, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_data, 1, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_CBVResourceNames, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_CBVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_CBVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_CBVResourceSpaces, 1, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureSRVResourceNames, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureSRVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureSRVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureSRVResourceSpaces, 2, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureUAVResourceNames, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureUAVResourceBindings, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureUAVResourceCounts, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_SamplerResourceNames, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_SamplerResourceBindings, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_SamplerResourceCounts, g_ffxm_fsr2_lock_pass_16bit_66bbd510c70b8cbcbe01ff81d6720626_SamplerResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

