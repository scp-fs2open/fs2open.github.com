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

#define FSR2_BIND_SRV_LOCK_INPUT_LUMA                       0

#define FSR2_BIND_UAV_NEW_LOCKS                             1
#define FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH      2

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
#include "fsr2/ffxm_fsr2_lock.h"

#ifndef FFXM_FSR2_THREAD_GROUP_WIDTH
#define FFXM_FSR2_THREAD_GROUP_WIDTH 8
#endif // #ifndef FFXM_FSR2_THREAD_GROUP_WIDTH
#ifndef FFXM_FSR2_THREAD_GROUP_HEIGHT
#define FFXM_FSR2_THREAD_GROUP_HEIGHT 8
#endif // #ifndef FFXM_FSR2_THREAD_GROUP_HEIGHT
#ifndef FFXM_FSR2_THREAD_GROUP_DEPTH
#define FFXM_FSR2_THREAD_GROUP_DEPTH 1
#endif // #ifndef FFXM_FSR2_THREAD_GROUP_DEPTH
#ifndef FFXM_FSR2_NUM_THREADS
#define FFXM_FSR2_NUM_THREADS layout (local_size_x = FFXM_FSR2_THREAD_GROUP_WIDTH, local_size_y = FFXM_FSR2_THREAD_GROUP_HEIGHT, local_size_z = FFXM_FSR2_THREAD_GROUP_DEPTH) in;
#endif // #ifndef FFXM_FSR2_NUM_THREADS

FFXM_FSR2_NUM_THREADS
void main()
{
    uvec2 uDispatchThreadId = gl_WorkGroupID.xy * uvec2(FFXM_FSR2_THREAD_GROUP_WIDTH, FFXM_FSR2_THREAD_GROUP_HEIGHT) + gl_LocalInvocationID.xy;

    ComputeLock(ivec2(uDispatchThreadId));
}
