#pragma once

#include "lab/wmcgui.h"
#include "globalincs/vmallocator.h"
#include "lab/dialogs/lab_dialog.h"
#include "lab/renderer/lab_renderer.h"
#include "lab/labv2_internal.h"
#include <initializer_list>

enum class LabRotationMode { Both, Yaw, Pitch, Roll };

class LabManager {
public:
	LabManager();

	// Do rendering and handle keyboard/mouse events
	void onFrame(float frametime);
	
	// Creates a new object of the passed type, using the respective class definition found at info_index and replaces the currently
	// displayed object
	void changeDisplayedObject(LabMode type, int info_index);

	void close() {
		if (CurrentObject != -1)
			obj_delete(CurrentObject);

		Renderer->close();

		delete Screen;
		Screen = nullptr;

		Game_mode &= ~GM_LAB;
	}

	GUIScreen* Screen;
	Window* Toolbar;
	LabRenderer* Renderer;

	LabMode CurrentMode = LabMode::None;
	int CurrentObject = -1;
	int CurrentClass = -1;

	bool isSafeForShips() {
		return CurrentMode == LabMode::Ship && CurrentObject != -1;
	}

	bool isSafeForWeapons() {
		return CurrentMode == LabMode::Weapon && CurrentObject != -1;
	}

	int FirePrimaries = 0;
	int FireSecondaries = 0;
private:
	SCP_vector<LabDialog*> Dialogs = {};

	int Trackball_active;
	int Trackball_mode;
	LabRotationMode RotationMode = LabRotationMode::Both;
	float RotationSpeedDivisor = 100.0f;
	float Lab_thrust_len = 0.0f;
	bool Lab_thrust_afterburn = false;
};
