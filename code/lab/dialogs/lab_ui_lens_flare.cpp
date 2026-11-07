#include "lab_ui.h"

#include "graphics/2d.h"
#include "graphics/lens_flare.h"
#include "lab/labv2_internal.h"
#include "object/object.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "weapon/beam.h"

using namespace ImGui;

namespace {

// A thruster draw names the ship by objnum, and the pass that produced it ran a
// frame ago -- so the object may already be gone by the time the panel reads it.
const char* flare_source_ship_name(int objnum)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS || Objects[objnum].type != OBJ_SHIP) {
		return "<gone>";
	}
	return Ships[Objects[objnum].instance].ship_name;
}

// A beam draw names the beam object itself, which isn't a ship -- what the
// panel actually wants to show is who is firing it.
const char* flare_source_beam_shooter_name(int beam_objnum)
{
	if (beam_objnum < 0 || beam_objnum >= MAX_OBJECTS || Objects[beam_objnum].type != OBJ_BEAM) {
		return "<gone>";
	}
	const int bm_idx = Objects[beam_objnum].instance;
	if (bm_idx < 0 || bm_idx >= MAX_BEAMS) {
		return "<gone>";
	}
	const beam& bm = Beams[bm_idx];
	if (bm.objp == nullptr || bm.objp->type != OBJ_SHIP) {
		return "<gone>";
	}
	return Ships[bm.objp->instance].ship_name;
}

} // namespace

// The lab's "Lens flare options" panel: the camera lens the scene is shot
// through, live iris editing, the global brightness calibration, and what the
// last flare pass actually did. Split out of lab_ui.cpp, which has no room for
// another feature panel.

// Live iris controls. One aperture drives both the ghosts and the starburst
// (the starburst is the Fraunhofer transform of this mask), so every slider
// here changes both at once. Edits are coalesced by lens_flare.cpp -- the mask
// and its FFT are far too expensive to rebuild on every frame of a drag.
void LabUi::build_lens_aperture_options(graphics::lens_aperture& ap)
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
		graphics::lens_flare_overrides().aperture = ap;
		graphics::lens_flare_overrides_changed();
	}
}

void LabUi::build_lens_flare_options()
{
	// The camera's own settings, resolved once: the controls below start from
	// whatever is currently in force -- a lens's tabled values, or whatever this
	// panel or a mission has already overridden them with -- so nothing here has
	// to know which of the two it is looking at.
	const int active_lens = graphics::lens_flare_active_lens();
	graphics::lens_settings settings = graphics::lens_flare_effective_settings(active_lens);
	auto& overrides = graphics::lens_flare_overrides();
	bool settings_changed = false;

	// A control writes back *only its own* override, and only when it actually
	// moved. Writing the whole set on any change would freeze the mounted lens's
	// entire tabled look into the overrides the moment one slider was nudged --
	// after which switching lenses in the combo below would keep showing the old
	// lens's intensity, starburst and squeeze, since an override quite correctly
	// beats whatever the new lens tables.
	auto edited = [&settings_changed](bool moved, auto& slot, const auto& value) {
		if (moved) {
			slot = value;
			settings_changed = true;
		}
		return moved;
	};

	// Not per-camera and so not overridable: this one describes the display.
	auto& tuning = graphics::lens_flare_get_tuning();
	edited(SliderFloat("Ghost brightness", &settings.ghost_brightness, 0.0f, 500.0f, "%.1f",
			   ImGuiSliderFlags_Logarithmic),
		overrides.ghost_brightness, settings.ghost_brightness);
	edited(SliderFloat("Starburst brightness", &settings.starburst_brightness, 0.0f, 10.0f),
		overrides.starburst_brightness, settings.starburst_brightness);
	SliderFloat("HDR headroom (x paper white)", &tuning.hdr_headroom, 0.0f, 8.0f);
	if (Gr_hdr_output_active) {
		TextDisabled("HDR output active: flare auto-scaled to ~%.1fx paper white", tuning.hdr_headroom);
	} else {
		TextDisabled("SDR output active: HDR headroom has no effect right now");
	}

	// The camera lens: one for the whole scene, so every sun flares through it
	Separator();
	const auto lab_lens = graphics::lens_flare_get_lab_lens();

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

	if (const auto* lens = graphics::lens_flare_get_system(active_lens)) {
		edited(SliderFloat("Lens intensity", &settings.intensity, 0.0f, 10.0f), overrides.intensity,
			settings.intensity);
		edited(Checkbox("Starburst", &settings.starburst), overrides.starburst, settings.starburst);
		if (settings.starburst) {
			edited(SliderFloat("Starburst scale", &settings.starburst_scale, 0.0f, 4.0f, "%.2fx"),
				overrides.starburst_scale, settings.starburst_scale);
		}
		edited(SliderInt("Max ghosts", &settings.max_ghosts, 0, graphics::MAX_LENS_FLARE_GHOSTS),
			overrides.max_ghosts, settings.max_ghosts);

		// The squeeze and the streak are one artifact and are overridden together,
		// so unlike the knobs above they share a slot -- any of the five moving
		// writes the whole lens_anamorphic. All of them cost nothing to change:
		// they are applied when the quads are drawn, with no texture to rebuild.
		graphics::lens_streak& streak = settings.anamorphic.streak;
		bool anamorphic_moved = SliderFloat("Anamorphic squeeze", &settings.anamorphic.squeeze, 1.0f, 3.0f, "%.2fx");
		with_TreeNode("Anamorphic streak")
		{
			TextDisabled("Stays horizontal wherever the sun is");
			anamorphic_moved |= SliderFloat("Streak strength", &streak.strength, 0.0f, 2.0f);
			if (streak.strength > 0.0f) {
				anamorphic_moved |= SliderFloat("Streak length", &streak.length, 0.0f, 4.0f);
				anamorphic_moved |= SliderFloat("Streak thickness", &streak.thickness, 0.001f, 0.2f, "%.3f");
				anamorphic_moved |= ColorEdit3("Streak tint", streak.tint);
			}
		}
		edited(anamorphic_moved, overrides.anamorphic, settings.anamorphic);

		Text("%d of %d ghosts | EFL %.1f mm | f/%.1f | %s",
			MIN(static_cast<int>(lens->ghosts.size()), MAX(settings.max_ghosts, 0)),
			static_cast<int>(lens->ghosts.size()),
			lens->efl,
			lens->efl / (2.0f * lens->entrance_radius),
			settings.starburst ? "starburst" : "no starburst");

		build_lens_aperture_options(settings.aperture);
	} else {
		TextDisabled("No lens mounted: this background renders no physically-based flares");
	}

	// The overrides themselves were written above, by whichever control moved.
	// This only publishes the fact that something did -- the iris sliders do it
	// for themselves, since theirs is the edit that costs a texture rebuild.
	if (settings_changed) {
		graphics::lens_flare_overrides_changed();
	}

	Separator();
	build_thruster_flare_options();

	// live pass state, refreshed every frame by lens_flare_frame_update():
	// one entry per light source that got a draw
	Separator();
	build_lens_flare_pass_report();
}

// What the last pass drew. Suns and beams are listed one by one -- neither is
// ever more than a handful -- while nozzles are summarised, because at full
// budget there are dozens of them and a line each would bury everything else
// in this window.
void LabUi::build_lens_flare_pass_report()
{
	const auto& draws = graphics::lens_flare_get_frame_draws();
	if (draws.empty()) {
		TextUnformatted("Last pass: inactive (no visible source, or no lens mounted)");
		return;
	}

	int thruster_draws = 0;
	int thruster_instances = 0;
	float max_off_axis = -1.0f;
	int max_off_axis_obj = -1;

	Text("Last pass: %d source(s) drawn", static_cast<int>(draws.size()));
	for (const auto& draw : draws) {
		if (draw.kind == graphics::flare_source_kind::sun) {
			Text("  Sun (%s): %d instances, visibility %.2f, %.1f deg off-axis, output scale %.3f",
				stars_get_sun_name(draw.source_index),
				draw.instances,
				draw.visibility,
				draw.off_axis_deg,
				draw.output_scale);
			continue;
		}

		if (draw.kind == graphics::flare_source_kind::beam) {
			Text("  Beam (%s): %d instances, visibility %.2f, %.1f deg off-axis, output scale %.3f",
				flare_source_beam_shooter_name(draw.source_index),
				draw.instances,
				draw.visibility,
				draw.off_axis_deg,
				draw.output_scale);
			continue;
		}

		thruster_draws++;
		thruster_instances += draw.instances;
		if (draw.off_axis_deg > max_off_axis) {
			max_off_axis = draw.off_axis_deg;
			max_off_axis_obj = draw.source_index;
		}
	}

	if (thruster_draws > 0) {
		Text("  Thrusters: %d nozzle(s), %d instances total, furthest off-axis %.1f deg on %s",
			thruster_draws,
			thruster_instances,
			max_off_axis,
			flare_source_ship_name(max_off_axis_obj));
	}
}

// Thruster flares are tabled per species, but the lab overrides all species at
// once -- it shows one ship at a time, and a single override leaves every tabled
// value untouched, so nothing has to be restored on the way out (see
// lens_flare.h).
void LabUi::build_thruster_flare_options()
{
	auto& lab_override = graphics::lens_flare_lab_thruster_flare();

	bool overriding = lab_override.has_value();
	if (Checkbox("Override thruster flares", &overriding)) {
		if (overriding) {
			// Start from what the displayed ship's own species tables, so switching
			// the override on changes nothing until a slider is touched
			const int species_idx =
				getLabManager()->isSafeForShips() ? Ship_info[getLabManager()->CurrentClass].species : -1;
			auto tabled = graphics::lens_flare_thruster_settings(species_idx);
			tabled.enabled = true;
			lab_override = tabled;
		} else {
			lab_override.reset();
		}
	}

	// Not part of the override: this is a render policy, not content, so it
	// applies whether or not the tabled values are being overridden
	auto& tuning = graphics::lens_flare_get_tuning();
	Checkbox("Draw ghosts for thruster flares", &tuning.thruster_ghosts);
	TextDisabled("Off by default: every lit nozzle is its own source, so a ghost");
	TextDisabled("train each is both the cost of the pass and, at that count,");
	TextDisabled("noise. Turn it on to see what it buys and what it costs.");

	if (!lab_override) {
		TextDisabled("Using each species' own species_defs.tbl settings");
		return;
	}

	Checkbox("Thruster flares enabled", &lab_override->enabled);
	SliderFloat("Thruster intensity", &lab_override->intensity, 0.0f, 50.0f);
	SliderFloat("Afterburner intensity", &lab_override->afterburner_intensity, 0.0f, 50.0f);
	ColorEdit3("Thruster flare tint", lab_override->color.a1d);
	TextDisabled("1.0 = one nozzle of radius r seen from 32r away. At combat");
	TextDisabled("range a nozzle is a small fraction of that, which is why the");
	TextDisabled("useful values are large. Brightness also follows throttle,");
	TextDisabled("nozzle facing and distance, so it is never constant per ship.");
}
