#pragma once

#include "globalincs/vmallocator.h"
#include "lab/labv2.h"
#include "lab/dialogs/lab_ui.h"
#include "lab/renderer/lab_renderer.h"
#include "model/animation/modelanimation.h"

#include <gamesequence/gamesequence.h>
#include "osapi/osapi.h"
#include "asteroid/asteroid.h"
#include "ship/ship.h"
#include "weapon/weapon.h"


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

	// Cleanup all objects and stop firing any weapons
	void cleanup();
	
	// Creates a new object of the passed type, using the respective class definition found at info_index and replaces the currently
	// displayed object
	void changeDisplayedObject(LabMode type, int info_index, int subtype = -1);

	// Deletes the docker object if exists
	void deleteDockerObject();

	// Spawns a docker object to use with dock or undock tests. Deletes the current docker object if it exists
	void spawnDockerObject();

	// Begins the docking test
	void beginDockingTest();

	// Begins the undocking test
	void beginUndockingTest();

	void close() {
		animation::ModelAnimationSet::stopAnimations();

		cleanup();

		LabRenderer::close();

		// Unload any asteroids that were loaded
		asteroid_level_close();

		// Lab can only be entered from the Mainhall so this should be safe
		model_free_all();

		Game_mode &= ~GM_LAB;

		ai_paused = 0;
		Player_ship = nullptr;

		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	std::unique_ptr<LabRenderer> Renderer;

	LabMode CurrentMode = LabMode::None;
	int CurrentObject = -1;
	int CurrentSubtype = -1;
	int CurrentClass = -1;
	int DockerObject = -1;
	int DockerClass = 0;
	SCP_string DockerDockPoint;
	SCP_string DockeeDockPoint;
	vec3d CurrentPosition = vmd_zero_vector;
	matrix CurrentOrientation = vmd_identity_matrix;
	SCP_string ModelFilename;	

	int Saved_cmdline_collisions_value;

	bool isSafeForShips() {
		return CurrentMode == LabMode::Ship && CurrentObject != -1 && Objects[CurrentObject].type == OBJ_SHIP;
	}

	bool isSafeForWeapons() {
		bool valid = CurrentObject != -1 && (Objects[CurrentObject].type == OBJ_WEAPON || Objects[CurrentObject].type == OBJ_BEAM);
		return CurrentMode == LabMode::Weapon && valid;
	}

	bool isSafeForAsteroids() const {
		return CurrentMode == LabMode::Asteroid && CurrentObject != -1 && Objects[CurrentObject].type == OBJ_ASTEROID;
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
	
	void resetGraphicsSettings();
	
	std::array<bool, MAX_SHIP_PRIMARY_BANKS> FirePrimaries = {};
	std::array<bool, MAX_SHIP_SECONDARY_BANKS> FireSecondaries = {};
	SCP_vector<std::tuple<ship_subsys*, LabTurretAimType, bool>> FireTurrets;

	LabRotationMode RotationMode = LabRotationMode::Both;
	float RotationSpeedDivisor = 100.0f;
	bool AllowWeaponDestruction = false;
	bool ShowingTechModel = false;

	flagset<ManagerFlags> Flags;

	gfx_options graphicsSettings;
  private:
	float Lab_thrust_len = 0.0f;
	bool Lab_thrust_afterburn = false;
	bool Weapons_loaded = false;
	bool CloseThis = false;
	LabUi labUi;

	void changeShipInternal();
};
