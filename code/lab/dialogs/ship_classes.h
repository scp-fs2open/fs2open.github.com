#pragma once

#include "lab/dialogs/lab_dialog.h"
#include "lab/dialogs/class_descriptions.h"
#include "lab/dialogs/class_options.h"
#include "lab/dialogs/class_variables.h"

#include "species_defs/species_defs.h"
#include "ship/ship.h"


class ShipClasses : public LabDialog {
public:
	ShipClasses() {
		Subdialogs.push_back(new Descriptions());
		Subdialogs.push_back(new Options());
		Subdialogs.push_back(new Variables());

		List_window = nullptr;
		Class_toolbar = nullptr;
	}

	SCP_string getTitle() override { return "Ship Classes"; }

	bool safeToOpen(LabMode /*labMode*/) override { return true; }

	void close() override {
		if (List_window != nullptr) {
			List_window->DeleteChildren();
			List_window->Delete();
			List_window = nullptr;
		}

		if (Class_toolbar != nullptr) {
			Class_toolbar->DeleteChildren();
			Class_toolbar->Delete();
			Class_toolbar = nullptr;
		}
	}

	void open(Button* /*caller*/) override;

	void update(LabMode newLabMode, int classIndex) override;

private:
	Window* List_window = nullptr;
	Window* Class_toolbar = nullptr;
	SCP_vector<LabDialog*> Subdialogs;
};