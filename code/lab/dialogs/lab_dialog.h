#pragma once

#include "lab/wmcgui.h"
#include "lab/labv2.h"

class LabDialog {
public:
	// Called when this dialog is opened via the top toolbar
	virtual void open(Button* caller) = 0;

	// Called when the dialog is closed
	virtual void close() = 0;

	// Called when the global state changes (e.g. other ship/weapon being selected)
	virtual void update(LabMode newLabMode, int classIndex) = 0;

	// Returns the string to use for the top nav bar
	virtual SCP_string getTitle() = 0;

	// Returns true if it is safe to open this dialog
	virtual bool safeToOpen(LabMode labMode) = 0;
};

class DialogWindow : public Window {
public:
	void SetOwner(LabDialog* owner) { Owner = owner; }
	DialogWindow(const SCP_string& in_caption, int x_coord, int y_coord, int x_width = -1, int y_height = -1, int in_style = 0) :
		Window(in_caption, x_coord, y_coord, x_width, y_height, in_style) {
	}

	~DialogWindow() {
		if (Owner != nullptr)
			Owner->close();
	}
private:
	LabDialog* Owner = nullptr;
};