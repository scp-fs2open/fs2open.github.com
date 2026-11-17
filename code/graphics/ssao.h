#pragma once

// Screen-space ambient occlusion (SSAO) with CORTAO (https://github.com/theagentd/CORTAO).
//
// The CORTAO passes read the G-buffer position and normal targets after the G-buffer pass. They
// write an occlusion target, and the deferred ambient light pass (SSAO shader variant of
// deferred-f.sdr) multiplies it into the same `ao` term as the baked AO maps.
//
// The Vulkan and the OpenGL backends have it. The radius and the intensity are
// content-scale-dependent and therefore mod-owned: see $SSAO Max Radius:, $SSAO Screen Radius:
// and $SSAO Intensity: in lighting_profiles.h. See also documentation/plans/cortao-integration-plan.md.

// CORTAO samples per pixel. 0 (the default) disables SSAO. The shader has variants for 4, 8 and 16.
// This is the "Screen-Space Ambient Occlusion" option.
extern int Ssao_samples;

// How the CORTAO filter pass runs. Auto selects from whether a temporal filter (FSR) runs: the
// 5-tap filter with FSR, the 13-tap filter without it.
enum class SsaoFilterMode { Auto, Off, Small, Large };

// The noise pattern of CORTAO. Auto selects from whether a temporal filter (FSR) runs: animated
// with FSR, which removes the noise, and constant without it, which flickers less.
enum class SsaoNoiseMode { Auto, Constant, Animated };

// Session settings for tests and debugging. Only the F3 lab changes them.
struct ssao_settings {
	SsaoFilterMode filter = SsaoFilterMode::Auto;
	SsaoNoiseMode noise = SsaoNoiseMode::Auto;
	// Use the CORTAO bent normals for the irradiance map lookup of the ambient light. See
	// ssao_bent_normals_active().
	bool bent_normals = true;
	// Debug view: the ambient light shows only the SSAO term, and the other lights add nothing.
	// Set the lab emissive factor to 0 to see the occlusion alone.
	bool debug_view = false;
};

extern ssao_settings Ssao_settings;

// Whether the renderer can run the CORTAO passes. See CAPABILITY_SCREEN_SPACE_AO.
bool ssao_supported();

// Whether the deferred ambient pass applies SSAO this frame. Gate all SSAO work on this function.
bool ssao_enabled();

// The sample count for the CORTAO main pass: 4, 8 or 16. Only valid when ssao_enabled().
int ssao_sample_count();

// The filter pass size for this frame, with Auto resolved. temporalFilterActive is true when a
// temporal filter (FSR) runs after the scene. Never returns Auto.
SsaoFilterMode ssao_filter_mode(bool temporalFilterActive);

// Whether the noise pattern changes each frame, with Auto resolved as for ssao_filter_mode().
bool ssao_animate_noise(bool temporalFilterActive);

// Whether the CORTAO passes write bent normals this frame. Only the irradiance map lookup of the
// ambient light uses them, so they are only active when an environment map is present.
// Only valid when ssao_enabled().
bool ssao_bent_normals_active();

// Remove the SSAO option from the options list when the renderer cannot run the CORTAO passes.
void ssao_remove_unsupported_options();
