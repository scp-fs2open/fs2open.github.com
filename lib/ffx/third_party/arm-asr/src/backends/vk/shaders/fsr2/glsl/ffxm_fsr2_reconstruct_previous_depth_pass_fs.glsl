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

#define FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  0
#define FSR2_BIND_SRV_INPUT_DEPTH                           1
#define FSR2_BIND_SRV_INPUT_COLOR                           2
#define FSR2_BIND_SRV_INPUT_EXPOSURE                        3

#define FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH      4

#define FSR2_BIND_CB_FSR2                                   0

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
#include "fsr2/ffxm_fsr2_reconstruct_dilated_velocity_and_previous_depth.h"

layout (location = 0) out FfxFloat32 rw_dilatedDepth;
layout (location = 1) out FfxFloat32x2 rw_dilated_motion_vectors;
layout (location = 2) out FfxFloat32 rw_lock_input_luma;

void main()
{
    FfxInt32x2 uPixelCoord = FfxInt32x2(gl_FragCoord.xy);
    ReconstructPrevDepthOutputs result = ReconstructAndDilate(uPixelCoord);
    rw_dilatedDepth = result.fDepth;
    rw_dilated_motion_vectors = result.fMotionVector;
    rw_lock_input_luma = result.fLuma;
}
