# Arm Accuracy Super Resolution

FSO builds the Arm ASR 25.06 Vulkan backend as a static library for Android. This directory contains the C++ sources, headers and shaders from [Arm's generic library at revision `0501f490bd9946a2e1806b5363d7ab8a9a6a5e0a`](https://github.com/arm/accuracy-super-resolution-generic-library/tree/0501f490bd9946a2e1806b5363d7ab8a9a6a5e0a). The build uses the upstream SPIR-V headers. See [the MIT license](LICENSES/arm-asr.txt) for the copyright and license terms.

The sources include the [Vulkan changes previously applied when building the Android prebuilts](https://github.com/Shivansps/Fso-Android-Prebuilts/blob/d95df76a79b56c2a3b5ae5c1d38dd96856cde9e7/arm-asr/portable-vulkan.patch).

- Use FSO's Vulkan instance and device functions with `VK_NO_PROTOTYPES`.
- Synchronize color attachment writes and resources shared by the fragment and compute stages.
- Preserve 64-bit Vulkan handles on `armeabi-v7a` and `x86`.
- Account for the resource-view table in scratch memory, check context alignment, and return an error for unknown shader passes.
