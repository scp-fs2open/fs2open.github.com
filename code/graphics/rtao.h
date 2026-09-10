#pragma once

// Raytraced ambient occlusion (RTAO): short hemisphere occlusion rays traced via
// inline ray query against the raytraced-shadow TLAS, inside the deferred ambient
// light pass (LT_AMBIENT branch of deferred-f.sdr, RTAO shader variant). Composes
// multiplicatively with baked AO maps through the shared G-buffer `ao` term, so
// env-map/IBL ambient darkens consistently for free. The AO radius and strength
// are content-scale-dependent and therefore mod-owned -- see $RTAO Radius: /
// $RTAO Strength: in lighting_profiles.h.

// Rays traced per pixel for ambient occlusion. 0 (the default) disables RTAO
// entirely; nonzero values only take effect when rtao_supported(). Clamped
// in-shader to RT_SHADOW_MAX_SAMPLES (16). See RtaoQualityOption in rtao.cpp.
extern int Rtao_samples;

// Whether the hardware/renderer can trace AO rays at all -- the same requirement
// set as raytraced shadows (Vulkan + VK_KHR_acceleration_structure +
// VK_KHR_ray_query), so this simply forwards to that capability.
bool rtao_supported();

// Whether the deferred ambient pass should trace AO this frame, i.e. the user has
// enabled it AND the hardware supports it. This is the single source of truth --
// gate any RTAO shader-flag, TLAS-build, or uniform code on this, not on
// Rtao_samples/rtao_supported() separately.
bool rtao_enabled();
