#pragma once

#include "lab/wmcgui.h"
#include "lab/labv2.h"

class DialogOpener;

class LabDialog {
public:
	virtual ~LabDialog() = default;

	// Called when this dialog is opened via the top toolbar
	virtual void open(Button* caller) = 0;

	// Called when the dialog's window is closed
	virtual void close() = 0;

	// Called when the global state changes (e.g. other ship/weapon being selected)
	virtual void update(LabMode newLabMode, int classIndex) = 0;

	// Returns the string to use for the top nav bar
	virtual SCP_string getTitle() = 0;

	// Returns true if it is safe to open this dialog
	virtual bool safeToOpen(LabMode labMode) = 0;

	void setOpener(const DialogOpener* opener) { Opener = opener; }
protected:
	const DialogOpener *Opener = nullptr;
};

// This is a base class for Windows opened by the various dialogs. Since dialogs exist even when their Windows are closed,
// this introduces the concept of an "Owner", who gets notified when the DialogWindow is closed.
class DialogWindow : public Window {
public:
	DialogWindow(const SCP_string& in_caption, int x_coord, int y_coord, int x_width = -1, int y_height = -1, int in_style = 0) :
		Window(in_caption, x_coord, y_coord, x_width, y_height, in_style) {
	}

	~DialogWindow() override {
		if (Owner != nullptr)
			Owner->close();
	}

	void SetOwner(std::shared_ptr<LabDialog> owner)
	{
		Owner = std::move(owner);
	}

private:
	std::shared_ptr<LabDialog> Owner;
};