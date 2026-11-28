#pragma once

#include "camera/camera.h"

// In VR, the HUD is drawn once per frame, as seen from the ship's camera without the head tracking, into an offscreen
// layer. Each eye then shows that layer as a panel fixed to the ship, far ahead like a collimated HUD. Unlike drawing
// the HUD over each eye's image, this keeps it steady when the head moves or rolls, and both eyes see it in the same
// place.

enum class VrHudPass {
	Direct,	// Draw the HUD over the view as usual (not VR)
	Panel,	// Draw the HUD now, it goes into the panel
	Reuse,	// Skip the HUD, the panel was already drawn this frame
};

// Sets up the panel, if VR is running. Call once the graphics are initialized.
void hud_vr_init();

// Decides how the HUD is drawn for the eye being rendered, once its view is set up. Panel redirects drawing into the
// panel until hud_vr_end(). The camera is where the panel hangs off: the view's camera without the head tracking.
VrHudPass hud_vr_begin_maybe(const vec3d* camera_pos, const matrix* camera_orient);
void hud_vr_end();

// Whether HUD drawing currently goes into the panel
bool hud_vr_drawing_panel();

// Sets the g3 view to the panel's camera. Must be called within a g3 frame.
void hud_vr_set_view();

// Shows the panel to the eye being rendered and gives the g3 view back to that eye
void hud_vr_draw_panel(const fov_t& eye_fov);
