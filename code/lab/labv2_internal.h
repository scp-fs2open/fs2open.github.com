#pragma once
#include "lab/wmcgui.h"
#include "lab/manager/lab_manager.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"


const std::unique_ptr<LabManager> &getLabManager();

class DialogOpener : public Button {
public:
	DialogOpener(std::shared_ptr<LabDialog> dialog, int x_coord, int y_coord, int x_width = -1, int y_height = -1, int in_style = 0) :
		Button(dialog->getTitle(), x_coord, y_coord, nullptr, x_width, y_height, in_style) {
		Dialog = std::move(dialog);
	}

	int DoMouseUp(float frametime) override {
		if (Dialog != nullptr && Dialog->safeToOpen(getLabManager()->CurrentMode))
			Dialog->open();

		return Button::DoMouseUp(frametime);
	}

	std::shared_ptr<LabDialog> getDialog() const { return Dialog; }
private:
	std::shared_ptr<LabDialog> Dialog;
};
