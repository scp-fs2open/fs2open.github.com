#pragma once

#include "lab/dialogs/lab_dialog.h"
#include "lab/dialogs/class_descriptions.h"
#include "lab/dialogs/class_options.h"
#include "lab/dialogs/class_variables.h"

class WeaponClasses : public LabDialog {
public:
	WeaponClasses() {
		Subdialogs.push_back(new Descriptions());
		Subdialogs.push_back(new Options());
		Subdialogs.push_back(new Variables());

		dialogWindow = nullptr;
		Class_toolbar = nullptr;
	}

	SCP_string getTitle() override { return "Weapon Classes"; }

	bool safeToOpen(LabMode /*labMode*/) override { return true; }

	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}

		if (Class_toolbar != nullptr) {
			Class_toolbar->DeleteChildren();
			Class_toolbar = nullptr;
		}
	}

	void open(Button* /*caller*/) override;

	void update(LabMode newLabMode, int classIndex) override;

private:
	DialogWindow* dialogWindow = nullptr;
	DialogWindow* Class_toolbar = nullptr;
	SCP_vector<LabDialog*> Subdialogs;
};