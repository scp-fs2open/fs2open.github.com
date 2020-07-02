#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "lab/renderer/lab_renderer.h"
#include "lab/dialogs/ship_classes.h"
#include "lab/dialogs/weapon_classes.h"
#include "lab/dialogs/render_options.h"
#include "lab/dialogs/material_overrides.h"
#include "lab/dialogs/backgrounds.h"
#include "io/key.h"

LabManager::LabManager() {
	Screen = GUI_system.PushScreen(new GUIScreen("Lab"));
	Toolbar = (Window*)Screen->Add(new Window("Toolbar", gr_screen.center_offset_x, gr_screen.center_offset_y,
		-1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));

	Renderer = new LabRenderer(new OrbitCamera());

	Dialogs.push_back(new ShipClasses());
	Dialogs.push_back(new WeaponClasses());
	Dialogs.push_back(new Backgrounds());
	Dialogs.push_back(new RenderOptions());
	Dialogs.push_back(new MaterialOverrides());

	int x = 0;
	for (auto dialog : Dialogs) {
		auto cbp = Toolbar->AddChild(new DialogOpener(dialog, x, 0));
		x += cbp->GetWidth() + 10;
	}
}

void LabManager::onFrame(float frametime) {
	
	Renderer->onFrame(frametime);

	bool keyPressed = (GUI_system.OnFrame(frametime, true, false) == GSOF_NOTHINGPRESSED);

	if (keyPressed) {
		int key = GUI_system.GetKeyPressed();
		int status = GUI_system.GetStatus();

		//// set trackball modes
		//if (status & GST_MOUSE_LEFT_BUTTON) {
		//	Trackball_active = 1;
		//	Trackball_mode = 1; // rotate viewed object
		//
		//	if (key_get_shift_status() & KEY_SHIFTED) {
		//		Trackball_mode = 2; // zoom
		//	}
		//}
		//else if (status & GST_MOUSE_RIGHT_BUTTON) {
		//	Trackball_active = 1;
		//	Trackball_mode = 3; // rotate camera
		//}
		//else if (!mouse_down(MOUSE_LEFT_BUTTON | MOUSE_RIGHT_BUTTON)) {
		//	// reset trackball modes
		//	Trackball_active = 0;
		//	Trackball_mode = 0;
		//}

		int dx, dy;
		mouse_get_delta(&dx, &dy);
		Renderer->getCurrentCamera()->handleMouseInput(dx, dy, mouse_down(MOUSE_LEFT_BUTTON), mouse_down(MOUSE_RIGHT_BUTTON), key_get_shift_status());

		// handle any key presses
		switch (key) {
		// Adjust AA presets
		case KEY_0:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::FXAA_Low);
			break;
		case KEY_1:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::FXAA_Medium);
			break;
		case KEY_2:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::FXAA_High);
			break;
		case KEY_3:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::SMAA_Low);
			break;
		case KEY_4:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::SMAA_Medium);
			break;
		case KEY_5:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::SMAA_High);
			break;
		case KEY_6:
			if (!PostProcessing_override)
				Renderer->setAAMode(AntiAliasMode::SMAA_Ultra);
			break;

		case KEY_T:
			Renderer->useNextTeamColorPreset();
			break;

		case KEY_Y:
			Renderer->usePreviousTeamColorPreset();
			break;

		case KEY_V:
			Renderer->resetView();
			break;

		case KEY_UP:
			break;

		case KEY_DOWN:
			break;

		case KEY_R:
			switch (RotationMode) {
			case LabRotationMode::Both:
				RotationMode = LabRotationMode::Yaw;
				break;
			case LabRotationMode::Yaw:
				RotationMode = LabRotationMode::Pitch;
				break;
			case LabRotationMode::Pitch:
				RotationMode = LabRotationMode::Roll;
				break;
			case LabRotationMode::Roll:
				RotationMode = LabRotationMode::Both;
				break;
			}

			break;
		case KEY_S:
			RotationSpeedDivisor *= 10.f;
			if (RotationSpeedDivisor > 10000.f)
				RotationSpeedDivisor = 100.f;
			break;

			// bail...
		case KEY_ESC:
			close();
			lab_close();
			break;

		default: 
			// check for game-specific controls
			if (CurrentMode == LabMode::Ship) {
				if (check_control(PLUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len += 0.05f;
				else if (check_control(MINUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len -= 0.05f;

				CLAMP(Lab_thrust_len, 0.0f, 1.0f);

				if (check_control(AFTERBURNER, key))
					Lab_thrust_afterburn = !Lab_thrust_afterburn;
			}
			break;
		}
	}

	gr_flip();
}

void LabManager::changeDisplayedObject(LabMode mode, int info_index) {
	if (mode == CurrentMode && info_index == CurrentClass)
		return;

	CurrentMode = mode;
	CurrentClass = info_index;

	// TODO: Actually do things
	
	for (auto const& dialog : Dialogs) {
		dialog->update(CurrentMode, CurrentClass);
	}
}