#include "lab_ui.h"

#include "graphics/2d.h"
#include "graphics/lens_flare.h"
#include "lab/labv2_internal.h"
#include "starfield/starfield.h"

using namespace ImGui;

// The lab's "Lens flare options" panel: the camera lens the scene is shot
// through, live iris editing, the global brightness calibration, and what the
// last flare pass actually did. Split out of lab_ui.cpp, which has no room for
// another feature panel.

// Live iris controls. One aperture drives both the ghosts and the starburst
// (the starburst is the Fraunhofer transform of this mask), so every slider
// here changes both at once. Edits are coalesced by lens_flare.cpp -- the mask
// and its FFT are far too expensive to rebuild on every frame of a drag.
void LabUi::build_lens_aperture_options(int lens_idx, graphics::lens_aperture& ap)
{
	bool changed = false;

	with_TreeNode("Aperture")
	{
		TextDisabled("Shared by ghosts and starburst");

		changed |= SliderInt("Blades", &ap.blades, 2, 16);
		changed |= SliderFloat("Blade rotation", &ap.rotation, 0.0f, 180.0f, "%.1f deg");
		changed |= SliderFloat("Blade curvature", &ap.curvature, -1.0f, 1.0f);
		changed |= SliderFloat("Edge softness", &ap.softness, 0.0f, 0.5f);

		Separator();
		TextDisabled("Rim diffraction grating");
		changed |= SliderFloat("Grating strength", &ap.grating.strength, 0.0f, 1.0f);
		if (ap.grating.strength > 0.0f) {
			changed |= SliderFloat("Grating density", &ap.grating.density, 0.0f, 1.0f);
			changed |= SliderFloat("Grating length", &ap.grating.length, 0.0f, 1.0f);
			changed |= SliderFloat("Grating width", &ap.grating.width, 0.0f, 1.0f);
			changed |= SliderFloat("Grating softness", &ap.grating.softness, 0.0f, 0.5f);
		}

		Separator();
		TextDisabled("Scratches");
		changed |= SliderFloat("Scratch strength", &ap.scratches.strength, 0.0f, 1.0f);
		if (ap.scratches.strength > 0.0f) {
			changed |= SliderFloat("Scratch density", &ap.scratches.density, 0.0f, 1.0f);
			changed |= SliderFloat("Scratch length", &ap.scratches.length, 0.0f, 1.0f);
			changed |= SliderFloat("Scratch width", &ap.scratches.width, 0.0f, 1.0f);
			changed |= SliderFloat("Scratch rotation", &ap.scratches.rotation, 0.0f, 180.0f, "%.1f deg");
			changed |= SliderFloat("Scratch rot variation", &ap.scratches.rotation_variation, 0.0f, 1.0f);
			changed |= SliderFloat("Scratch softness", &ap.scratches.softness, 0.0f, 0.5f);
		}

		Separator();
		TextDisabled("Dust");
		changed |= SliderFloat("Dust strength", &ap.dust.strength, 0.0f, 1.0f);
		if (ap.dust.strength > 0.0f) {
			changed |= SliderFloat("Dust density", &ap.dust.density, 0.0f, 1.0f);
			changed |= SliderFloat("Dust radius", &ap.dust.radius, 0.0f, 1.0f);
			changed |= SliderFloat("Dust softness", &ap.dust.softness, 0.0f, 0.5f);
		}

		if (graphics::lens_flare_aperture_edit_pending()) {
			TextDisabled("Rebuilding aperture + starburst...");
		}
		TextDisabled("Grating/scratches/dust add off-axis energy, which the");
		TextDisabled("starburst normalizes against -- expect the core to dim");
		TextDisabled("as they come up. Changes are undone on table reload and");
		TextDisabled("whenever the background changes (same as set-lens-* sexps).");
	}

	if (changed) {
		graphics::lens_flare_aperture_changed(lens_idx);
	}
}

void LabUi::build_lens_flare_options()
{
	// global calibration multipliers (see graphics/lens_flare.cpp)
	float ghost_brightness = graphics::lens_flare_get_ghost_brightness();
	if (SliderFloat("Ghost brightness", &ghost_brightness, 0.0f, 500.0f, "%.1f", ImGuiSliderFlags_Logarithmic)) {
		graphics::lens_flare_set_ghost_brightness(ghost_brightness);
	}

	float starburst_brightness = graphics::lens_flare_get_starburst_brightness();
	if (SliderFloat("Starburst brightness", &starburst_brightness, 0.0f, 10.0f)) {
		graphics::lens_flare_set_starburst_brightness(starburst_brightness);
	}

	float hdr_headroom = graphics::lens_flare_get_hdr_headroom();
	if (SliderFloat("HDR headroom (x paper white)", &hdr_headroom, 0.0f, 8.0f)) {
		graphics::lens_flare_set_hdr_headroom(hdr_headroom);
	}
	if (Gr_hdr_output_active) {
		TextDisabled("HDR output active: flare auto-scaled to ~%.1fx paper white", hdr_headroom);
	} else {
		TextDisabled("SDR output active: HDR headroom has no effect right now");
	}

	// The camera lens: one for the whole scene, so every sun flares through it
	Separator();
	const auto lab_lens = graphics::lens_flare_get_lab_lens();
	const int active_lens = graphics::lens_flare_active_lens();

	const char* mission_lens_name = graphics::lens_flare_mission_lens_name();
	SCP_string mission_label = "Mission default (";
	mission_label += (*mission_lens_name != '\0') ? mission_lens_name : "none";
	mission_label += ")";

	const auto* active_system = graphics::lens_flare_get_system(active_lens);
	const char* preview = mission_label.c_str();
	if (lab_lens) {
		preview = (active_system != nullptr) ? active_system->name.c_str() : "None";
	}

	with_Combo("Camera lens", preview)
	{
		if (Selectable(mission_label.c_str(), !lab_lens)) {
			graphics::lens_flare_clear_lab_lens();
		}
		if (Selectable("None", lab_lens && *lab_lens < 0)) {
			graphics::lens_flare_set_lab_lens(-1);
		}
		for (int lens_idx = 0; lens_idx < graphics::lens_flare_num_systems(); lens_idx++) {
			bool is_selected = (lab_lens == lens_idx);

			if (Selectable(graphics::lens_flare_get_system(lens_idx)->name.c_str(), is_selected)) {
				graphics::lens_flare_set_lab_lens(lens_idx);
			}

			if (is_selected)
				SetItemDefaultFocus();
		}
	}

	if (auto* lens = graphics::lens_flare_get_system_mutable(active_lens)) {
		SliderFloat("Lens intensity", &lens->intensity, 0.0f, 10.0f);
		// Costs nothing to change: the squeeze is applied when the quads are
		// drawn, so unlike the aperture sliders it needs no texture rebuild
		SliderFloat("Anamorphic squeeze", &lens->anamorphic_squeeze, 1.0f, 3.0f, "%.2fx");

		with_TreeNode("Anamorphic streak")
		{
			TextDisabled("Stays horizontal wherever the sun is");
			SliderFloat("Streak strength", &lens->streak.strength, 0.0f, 2.0f);
			if (lens->streak.strength > 0.0f) {
				SliderFloat("Streak length", &lens->streak.length, 0.0f, 4.0f);
				SliderFloat("Streak thickness", &lens->streak.thickness, 0.001f, 0.2f, "%.3f");
				ColorEdit3("Streak tint", lens->streak.tint);
			}
		}
		Text("%d ghosts | EFL %.1f mm | f/%.1f | %s",
			static_cast<int>(lens->ghosts.size()),
			lens->efl,
			lens->efl / (2.0f * lens->entrance_radius),
			lens->starburst ? "starburst" : "no starburst");

		build_lens_aperture_options(active_lens, lens->aperture);
	} else {
		TextDisabled("No lens mounted: this background renders no physically-based flares");
	}

	// live pass state, refreshed every frame by lens_flare_frame_update():
	// one entry per sun that got a draw
	Separator();
	const auto& draws = graphics::lens_flare_get_frame_draws();
	if (draws.empty()) {
		TextUnformatted("Last pass: inactive (no visible sun, or no lens mounted)");
	} else {
		Text("Last pass: %d sun(s) drawn", static_cast<int>(draws.size()));
		for (const auto& draw : draws) {
			Text("  Sun %d (%s): %d instances, visibility %.2f, %.1f deg off-axis, output scale %.3f",
				draw.sun_index,
				stars_get_sun_name(draw.sun_index),
				draw.instances,
				draw.visibility,
				draw.off_axis_deg,
				draw.output_scale);
		}
	}
}
