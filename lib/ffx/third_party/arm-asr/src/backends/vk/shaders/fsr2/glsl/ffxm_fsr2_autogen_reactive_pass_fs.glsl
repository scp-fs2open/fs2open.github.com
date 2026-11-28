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

#define FSR2_BIND_SRV_INPUT_OPAQUE_ONLY                     0
#define FSR2_BIND_SRV_INPUT_COLOR                           1

#define FSR2_BIND_CB_FSR2                                   0
#define FSR2_BIND_CB_REACTIVE                               1

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

layout (location = 0) out FfxFloat32 rw_output_autoreactive;

void main()
{
    FfxInt32x2 uPixelCoord = FfxInt32x2(gl_FragCoord.xy);

    FfxFloat32x3 ColorPreAlpha  = LoadOpaqueOnly(FFXM_MIN16_I2(uPixelCoord)).rgb;
    FfxFloat32x3 ColorPostAlpha = LoadInputColor(uPixelCoord).rgb;

    if ((GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_TONEMAP) != 0)
    {
        ColorPreAlpha = Tonemap(ColorPreAlpha);
        ColorPostAlpha = Tonemap(ColorPostAlpha);
    }

    if ((GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_INVERSETONEMAP) != 0)
    {
        ColorPreAlpha = InverseTonemap(ColorPreAlpha);
        ColorPostAlpha = InverseTonemap(ColorPostAlpha);
    }

    FfxFloat32 out_reactive_value = 0.f;
    FfxFloat32x3 delta = abs(ColorPostAlpha - ColorPreAlpha);

    out_reactive_value = ((GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX)!=0) ? max(delta.x, max(delta.y, delta.z)) : length(delta);
    out_reactive_value *= GenReactiveScale();

    out_reactive_value = ((GenReactiveFlags() & FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_THRESHOLD)!=0) ? ((out_reactive_value < GenReactiveThreshold()) ? 0 : GenReactiveBinaryValue()) : out_reactive_value;

    rw_output_autoreactive = out_reactive_value;
}
