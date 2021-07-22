#pragma once

#include "lab/wmcgui.h"
#include "globalincs/vmallocator.h"
#include "lab/dialogs/lab_dialog.h"
#include "lab/renderer/lab_renderer.h"
#include <initializer_list>
#include <gamesequence/gamesequence.h>

enum class LabRotationMode { Both, Yaw, Pitch, Roll };

FLAG_LIST(ManagerFlags) {
	ModelRotationEnabled,

	NUM_VALUES
};

class LabManager {
public:
	LabManager();
	~LabManager();

	// Do rendering and handle keyboard/mouse events
	void onFrame(float frametime);
	
	// Creates a new object of the passed type, using the respective class definition found at info_index and replaces the currently
	// displayed object
	void changeDisplayedObject(LabMode type, int info_index);

	void close() {
		animation::ModelAnimation::clearAnimations();
  
		LabRenderer::close();

		Game_mode &= ~GM_LAB;

		ai_paused = 0;

		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	std::unique_ptr<GUIScreen> Screen;
	// points to an object inside Screen, so we can't use smart pointer
	Window* Toolbar;
	std::unique_ptr<LabRenderer> Renderer;

	LabMode CurrentMode = LabMode::None;
	int CurrentObject = -1;
	int CurrentClass = -1;
	vec3d CurrentPosition = vmd_zero_vector;
	matrix CurrentOrientation = vmd_identity_matrix;
	SCP_string ModelFilename = "";

	bool isSafeForShips() {
		return CurrentMode == LabMode::Ship && CurrentObject != -1;
	}

	bool isSafeForWeapons() {
		return CurrentMode == LabMode::Weapon && CurrentObject != -1;
	}

	void loadWeapons() {
		if (!Weapons_loaded) {
			extern void weapons_page_in();
			weapons_page_in();

			Weapons_loaded = true;
		}
	}

	// Call this function from outside LabManager::onFrame to signal that the lab should close.
	void notify_close() {
		CloseThis = true;
	}

	int FirePrimaries = 0;
	int FireSecondaries = 0;

	LabRotationMode RotationMode = LabRotationMode::Both;
	float RotationSpeedDivisor = 100.0f;

	flagset<ManagerFlags> Flags;
private:
	SCP_vector<std::shared_ptr<LabDialog>> Dialogs;

	float Lab_thrust_len = 0.0f;
	bool Lab_thrust_afterburn = false;
	bool Weapons_loaded = false;
	bool CloseThis = false;

	void changeShipInternal();
};
