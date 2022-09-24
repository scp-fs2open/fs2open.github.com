#pragma once

#include "lab/dialogs/lab_dialog.h"

template <class T>
struct lab_flag {
	Checkbox* cb;
	T flag;
};

class Options : public LabDialog {
public:
	~Options() override = default;

private:
	// Called when this dialog is opened via the top toolbar
	void open() override;

	// Called when the dialog is closed
	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Class Options"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship || labMode == LabMode::Weapon; }

private:
	DialogWindow* dialogWindow;

	SCP_vector<lab_flag<Ship::Info_Flags>> Ship_Class_Flags;
	SCP_vector<lab_flag<Weapon::Info_Flags>> Weapon_Class_Flags;

	template <class T>
	void addFlags(int* const X, int* const Y, const char* flag_name, T flag, SCP_vector<lab_flag<T>>& flag_list);
};