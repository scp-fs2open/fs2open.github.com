#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "lab/renderer/lab_renderer.h"

LabManager::LabManager() {
	Screen = GUI_system.PushScreen(new GUIScreen("Lab"));
	Toolbar = (Window*)Screen->Add(new Window("Toolbar", gr_screen.center_offset_x, gr_screen.center_offset_y,
		-1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));
	Renderer = new LabRenderer();

	int x = 0;
	for (auto dialog : Dialogs) {
		auto cbp = Toolbar->AddChild(new DialogOpener(dialog, x, 0));
		x += cbp->GetWidth() + 10;
	}
}

void LabManager::onFrame(float frametime) {
	Renderer->onFrame(frametime);
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