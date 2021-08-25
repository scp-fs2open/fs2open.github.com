#pragma once

#include "lab/dialogs/lab_dialog.h"


class BackgroundDialog : public LabDialog {
public:
	~BackgroundDialog() override = default;

private:
	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode /*newLabMode*/, int /*classIndex*/) override { /*do nothing*/ }

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Backgrounds"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode /*labMode*/) override { return true; }

private:
	DialogWindow* dialogWindow = nullptr;
};