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

#define FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH      0
#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                1
#define FSR2_BIND_SRV_DILATED_DEPTH                         2
#define FSR2_BIND_SRV_REACTIVE_MASK                         3
#define FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     4
#define FSR2_BIND_SRV_PREVIOUS_DILATED_MOTION_VECTORS       5
#define FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  6
#define FSR2_BIND_SRV_INPUT_COLOR                           7
#define FSR2_BIND_SRV_INPUT_DEPTH                           8
#define FSR2_BIND_SRV_INPUT_EXPOSURE                        9

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
#include "fsr2/ffxm_fsr2_depth_clip.h"

layout (location = 0) out FfxFloat32x2 rw_dilated_reactive_masks;
layout (location = 1) out FfxFloat32x4 rw_prepared_input_color;

void main()
{
    FfxInt32x2 uPixelCoord = FfxInt32x2(gl_FragCoord.xy);

	DepthClipOutputs result = DepthClip(uPixelCoord);

    rw_dilated_reactive_masks = result.fDilatedReactiveMasks;
    rw_prepared_input_color = result.fTonemapped;
}
