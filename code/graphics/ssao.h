#pragma once

// Screen-space ambient occlusion (SSAO) with CORTAO (https://github.com/theagentd/CORTAO).
//
// The CORTAO passes read the G-buffer position and normal targets after the G-buffer pass. They
// write an occlusion target, and the deferred ambient light pass (SSAO shader variant of
// deferred-f.sdr) multiplies it into the same `ao` term as the baked AO maps and RTAO.
//
// This is a prototype. Only the Vulkan backend has it, and it has one quality level, no filter
// pass and no temporal filter. See documentation/plans/cortao-integration-plan.md.

// 0 disables SSAO. Other values enable it. The value is the "Screen-Space Ambient Occlusion" option.
extern int Ssao_quality;

// The tuning values of the prototype. The F3 lab can change them for the current session.
struct ssao_settings {
	// The maximum occlusion radius in world units. The large default lets screen_radius control
	// the radius, because ship sizes go from a few meters to kilometers.
	float max_radius = 1.0e6f;
	// The maximum occlusion radius as a fraction of the screen height (CORTAO
	// maxScreenspaceRadiusHeightRatio). CORTAO recommends 0.1 to 0.25.
	float screen_radius = 0.2f;
	// The occlusion is raised to this power. 1 is the physically correct value.
	float intensity = 1.0f;
	// Change the noise pattern each frame. This is only correct when a temporal filter (FSR)
	// removes the noise. Without one, a constant pattern flickers less.
	bool animate_noise = false;
	// Debug view: the ambient light shows only the SSAO term, and the other lights add nothing.
	// Set the lab emissive factor to 0 to see the occlusion alone.
	bool debug_view = false;
};

extern ssao_settings Ssao_settings;

// Whether the renderer can run the CORTAO passes. Only the Vulkan backend can do this now.
bool ssao_supported();

// Whether the deferred ambient pass applies SSAO this frame. RTAO has priority, because the two
// write the same term. Gate all SSAO work on this function.
bool ssao_enabled();

// Remove the SSAO option from the options list when the renderer cannot run the CORTAO passes.
void ssao_remove_unsupported_options();
