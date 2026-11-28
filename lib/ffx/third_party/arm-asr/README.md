# Arm Accuracy Super Resolution

FSO builds the Arm ASR 25.06 Vulkan backend as a static library on platforms without bundled AMD FidelityFX support. The default build uses AMD FidelityFX on x86-64 Windows and x86-64 Linux, and Arm ASR on other targets. This directory contains the C++ sources, headers and shaders from [Arm's generic library at revision `0501f490bd9946a2e1806b5363d7ab8a9a6a5e0a`](https://github.com/arm/accuracy-super-resolution-generic-library/tree/0501f490bd9946a2e1806b5363d7ab8a9a6a5e0a). The build uses the upstream SPIR-V headers. See [the MIT license](LICENSES/arm-asr.txt) for the copyright and license terms.

The ARM ASR sources include some changes that involve:

- Use FSO Vulkan instance and device functions with `VK_NO_PROTOTYPES`.
- Synchronize color attachment writes and resources shared by the fragment and compute stages.
- Preserve 64-bit Vulkan handles in 32-bit processes.
- Allow host-visible device-local memory as a fallback on unified-memory GPUs.
- Account for the resource-view table in scratch memory, check context alignment, and return an error for unknown shader passes.
- Use FP16 instead of FP8 for the input HDR color in Performance profile