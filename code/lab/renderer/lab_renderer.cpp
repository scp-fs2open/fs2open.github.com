#include "lab/renderer/lab_renderer.h"
#include "lab/labv2_internal.h"
#include "lab/wmcgui.h"
#include "graphics/2d.h"


SCP_string get_rot_mode_string(Lab_rotation_modes rotmode)
{
	switch (rotmode) {
	case Lab_rotation_modes::Both:
		return "Manual rotation mode: Pitch and Yaw";
	case Lab_rotation_modes::Pitch:
		return "Manual rotation mode: Pitch";
	case Lab_rotation_modes::Yaw:
		return "Manual rotation mode: Yaw";
	case Lab_rotation_modes::Roll:
		return "Manual rotation mode: Roll";
	default:
		return "HOW DID THIS HAPPEN? Ask a coder!";
	}
}

SCP_string get_rot_speed_string(float speed_divisor)
{
	auto exp = std::lroundf(log10f(speed_divisor));

	switch (exp) {
	case 2:
		return "Fast";
	case 3:
		return "Slow";
	case 4:
		return "Slowest";
	default:
		return "HOW DID THIS HAPPEN? Ask a coder!";
	}
}

void LabRenderer::onFrame(float frametime) {
	GR_DEBUG_SCOPE("Lab Frame");

	gr_reset_clip();
	gr_clear();

	if (!renderFlags[LabRenderFlag::TimeStopped])
		Missiontime += Frametime;

	renderHud(frametime);
}

void LabRenderer::renderHud(float frametime) {
	GR_DEBUG_SCOPE("Lab Render");

	int w, h;

	// render our particular thing
	if (LMGR->CurrentObject >= 0) {
		renderModel(frametime);

		// print out the current pof filename, to help with... something
		if (strlen(LMGR->ModelFilename.c_str())) {
			gr_get_string_size(&w, &h, LMGR->ModelFilename.c_str());
			gr_set_color_fast(&Color_white);
			gr_string(gr_screen.center_offset_x + gr_screen.center_w - w,
				gr_screen.center_offset_y + gr_screen.center_h - h, LMGR->ModelFilename.c_str(), GR_RESIZE_NONE);
		}
	}

	// print FPS at bottom left, might be helpful
	extern void game_get_framerate();
	extern float frametotal;
	extern float Framerate;

	game_get_framerate();

	gr_set_color_fast(&Color_white);

	if (frametotal != 0.0f) {
		gr_printf_no_resize(gr_screen.center_offset_x + 2,
			gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(),
			"FPS: %3i Camera Distance: %4f", (int)std::lround(Framerate), cameraDistance);
	}
	else {
		gr_string(gr_screen.center_offset_x + 10, gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(),
			"FPS: ?", GR_RESIZE_NONE);
	}

	//Print FXAA preset
	if ((gr_is_fxaa_mode(Gr_aa_mode) || gr_is_smaa_mode(Gr_aa_mode)) && !PostProcessing_override) {
		const char* aa_mode;
		switch (Gr_aa_mode) {
		case AntiAliasMode::FXAA_Low:
			aa_mode = "FXAA Low";
			break;
		case AntiAliasMode::FXAA_Medium:
			aa_mode = "FXAA Medium";
			break;
		case AntiAliasMode::FXAA_High:
			aa_mode = "FXAA High";
			break;
		case AntiAliasMode::SMAA_Low:
			aa_mode = "SMAA Low";
			break;
		case AntiAliasMode::SMAA_Medium:
			aa_mode = "SMAA Medium";
			break;
		case AntiAliasMode::SMAA_High:
			aa_mode = "SMAA High";
			break;
		case AntiAliasMode::SMAA_Ultra:
			aa_mode = "SMAA Ultra";
			break;
		default:
			aa_mode = "None";
			break;
		}

		gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 2) - 3, "AA Preset: %s", aa_mode);
	}

	//Print current Team Color setting, if any
	if (currentTeamColor != "<none>")
		gr_printf_no_resize(gr_screen.center_offset_x + 2,
			gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 3) - 3,
			"Use T and Y to cycle through available Team Color settings. Current: %s",
			currentTeamColor.c_str());

	// Camera usage info
	gr_printf_no_resize(gr_screen.center_offset_x + 2,
		gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 4) - 3,
		"Hold LMB to rotate the ship or weapon. Hold RMB to rotate the Camera. Hold Shift + LMB to "
		"zoom in or out. Use number keys to switch between AA presets. R to cycle model rotation "
		"modes, S to cycle model rotation speeds, V to reset view.");

	// Rotation mode
	SCP_string text = get_rot_mode_string(Lab_rotation_mode);
	gr_printf_no_resize(gr_screen.center_offset_x + 2,
		gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 5) - 3,
		"%s Rotation speed: %s", get_rot_mode_string(Lab_rotation_mode).c_str(),
		get_rot_speed_string(Lab_manual_rotation_speed_divisor).c_str());
}

void LabRenderer::useBackground(SCP_string mission_name) {}