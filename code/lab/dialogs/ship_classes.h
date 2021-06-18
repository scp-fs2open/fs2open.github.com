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
		Subdialogs.emplace_back(std::make_shared<Descriptions>());
		Subdialogs.emplace_back(std::make_shared<Options>());
		Subdialogs.emplace_back(std::make_shared<Variables>());

		dialogWindow = nullptr;
		Class_toolbar = nullptr;
	}

	SCP_string getTitle() override { return "Ship Classes"; }

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
	SCP_vector<std::shared_ptr<LabDialog>> Subdialogs;
};