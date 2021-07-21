#pragma once

#include "lab/dialogs/lab_dialog.h"


class RenderOptions : public LabDialog {
public:
	~RenderOptions() override = default;

private:
	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode, int) override { /* Do nothing */ };

	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}
	}

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Render Options"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode /*labMode*/) override { return true; }

private:
	DialogWindow* dialogWindow = nullptr;
};