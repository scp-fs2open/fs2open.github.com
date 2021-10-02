#pragma once

#include "lab/dialogs/lab_dialog.h"
#include "lab/dialogs/class_descriptions.h"
#include "lab/dialogs/class_options.h"
#include "lab/dialogs/class_variables.h"

class WeaponClasses : public LabDialog {
public:
	WeaponClasses() {
		Subdialogs.emplace_back(std::make_shared<Descriptions>());
		Subdialogs.emplace_back(std::make_shared<Options>());
		Subdialogs.emplace_back(std::make_shared<Variables>());

		dialogWindow = nullptr;
		Class_toolbar = nullptr;
	}

	~WeaponClasses() override = default;

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
	SCP_vector<std::shared_ptr<LabDialog>> Subdialogs;
};