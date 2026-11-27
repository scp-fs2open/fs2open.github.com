// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
// Needed for rw_upscaled_output declaration
#extension GL_EXT_shader_image_load_formatted : require

#define FSR2_BIND_SRV_INPUT_EXPOSURE                         0
#define FSR2_BIND_SRV_DILATED_REACTIVE_MASKS                 1
#if FFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                 2
#else
#define FSR2_BIND_SRV_INPUT_MOTION_VECTORS                   2
#endif
#define FSR2_BIND_SRV_INTERNAL_UPSCALED                      3
#define FSR2_BIND_SRV_LOCK_STATUS                            4
#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR                   5
#define FSR2_BIND_SRV_LANCZOS_LUT                            6
#define FSR2_BIND_SRV_UPSCALE_MAXIMUM_BIAS_LUT               7
#define FSR2_BIND_SRV_SCENE_LUMINANCE_MIPS                   8
#define FSR2_BIND_SRV_AUTO_EXPOSURE                          9
#define FSR2_BIND_SRV_LUMA_HISTORY                           10

#define FSR2_BIND_SRV_TEMPORAL_REACTIVE                      11
#define FSR2_BIND_UAV_NEW_LOCKS                              12

#define FSR2_BIND_CB_FSR2                                    0

// Global mandatory defines
#if !defined(FFXM_HALF)
#define FFXM_HALF 1
#endif
#if !defined(FFXM_GPU)
#define FFXM_GPU 1
#endif
#if !defined(FFXM_GLSL)
#define FFXM_GLSL 1
#endif

#include "fsr2/ffxm_fsr2_callbacks_glsl.h"
#include "fsr2/ffxm_fsr2_common.h"
#include "fsr2/ffxm_fsr2_sample.h"
#include "fsr2/ffxm_fsr2_upsample.h"
#include "fsr2/ffxm_fsr2_postprocess_lock_status.h"
#include "fsr2/ffxm_fsr2_reproject.h"
#include "fsr2/ffxm_fsr2_accumulate.h"

#if !FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
layout (location = 0) out FfxFloat32x4 rw_internal_upscaled_color;
layout (location = 1) out FfxFloat32x2 rw_lock_status;
layout (location = 2) out FfxFloat32x4 rw_luma_history;
#if FFXM_FSR2_OPTION_APPLY_SHARPENING == 0
layout (location = 3) out FfxFloat32x3 rw_upscaled_output;
#endif
#else // FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
layout (location = 0) out FfxFloat32x3 rw_internal_upscaled_color;
layout (location = 1) out FfxFloat32 rw_internal_temporal_reactive;
layout (location = 2) out FfxFloat32x2 rw_lock_status;
#if FFXM_FSR2_OPTION_APPLY_SHARPENING == 0
layout (location = 3) out FfxFloat32x3 rw_upscaled_output;
#endif
#endif

void main()
{
    FfxInt32x2 uPixelCoord = FfxInt32x2(gl_FragCoord.xy);
    AccumulateOutputs result = Accumulate(uPixelCoord);

#if !FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
    rw_internal_upscaled_color = result.fColorAndWeight;
    rw_luma_history = result.fLumaHistory;
#else
    rw_internal_upscaled_color = result.fUpscaledColor;
    rw_internal_temporal_reactive = result.fTemporalReactive;
#endif
    rw_lock_status = result.fLockStatus;
#if FFXM_FSR2_OPTION_APPLY_SHARPENING == 0
    rw_upscaled_output = result.fColor;
#endif
}
