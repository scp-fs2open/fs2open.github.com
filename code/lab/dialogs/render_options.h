#pragma once

#include "lab/dialogs/lab_dialog.h"


class RenderOptions : public LabDialog {
	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override { /* Do nothing */ };

	void close() override { 
		dialogWindow->DeleteChildren(); 
		dialogWindow = nullptr; 
	}

	// Returns the string to use for the top nav bar
	SCP_string getTitle() { return "Render Options"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode /*labMode*/) { return true; }

private:
	DialogWindow* dialogWindow = nullptr;
};