#pragma once

#include "lab/wmcgui.h"
#include "globalincs/vmallocator.h"
#include "lab/dialogs/lab_dialog.h"
#include "lab/renderer/lab_renderer.h"
#include "lab/labv2_internal.h"
#include <initializer_list>

class LabManager {
public:
	LabManager();
	void onFrame(float frametime);
	
	// Creates a new object of the passed type, using the respective class definition found at info_index and replaces the currently
	// displayed object
	void changeDisplayedObject(LabMode type, int info_index);

	GUIScreen* Screen;
	Window* Toolbar;

	LabMode CurrentMode = LabMode::None;
	int CurrentObject = -1;
	int CurrentClass = -1;
private:
	SCP_vector<LabDialog*> Dialogs = {};
	LabRenderer* Renderer;
};
