#pragma once

#include "graphics/2d.h"

// Screen-space ambient occlusion (GTAO, Jimenez et al. 2016, structure from XeGTAO).
// The OpenGL deferred renderer computes it at half resolution from the position and
// normal G-buffers (gtao-f.sdr), and the ambient light pass (deferred-f.sdr, GTAO
// variant) darkens only the ambient term with it. The Vulkan renderer does not have
// this pass; there it has RTAO instead (graphics/rtao.h).

// 0 = Off, 1 = Low, 2 = Medium, 3 = High. Bound to Graphics.SsaoQuality.
extern int Gtao_quality;
// Multiplier on the default sample radius. Bound to Graphics.SsaoRadius.
extern float Gtao_radius_scale;
// Exponent on the visibility, so a larger value gives darker AO. Bound to Graphics.SsaoIntensity.
extern float Gtao_intensity;

// Lab only: draw the blurred AO buffer in place of the lit scene. Set from the lab's
// "Show screen-space AO buffer" checkbox; reset when the lab closes.
extern bool Gtao_show_buffer;

struct gtao_sample_counts {
	int slices;
	int steps; // per side of each slice
};

gtao_sample_counts gtao_samples(int quality);

// The sample radius in full-resolution pixels. See gtao.cpp for why it does not depend on depth.
// proj11 is element [1][1] of the projection matrix (1 / tan(fov_y / 2)).
float gtao_pixel_radius(float radius_scale, float proj11, int screen_height);

// Only the OpenGL renderer has the pass.
bool gtao_supported(GraphicsAPI api);

// Whether a deferred lighting pass runs GTAO. Only the scene pass does: the deferred cockpit
// pass clears the position buffer to 0 instead of the empty-pixel value, and a second run
// would double the cost for a few cockpit pixels.
bool gtao_runs(int quality, GraphicsAPI api, bool cockpit_pass);

// gtao_runs() for the current settings, renderer and lighting pass. Gate all GTAO code on this.
bool gtao_enabled();

// Record that the options need the OpenGL renderer, so that the options menu greys them
// out with the explanation on any other renderer. Call after localization is up.
void gtao_set_option_requirements();
