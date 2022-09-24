#pragma once

#include "lab/dialogs/lab_dialog.h"

class Descriptions : public LabDialog {
public:
	~Descriptions() override = default;

private:
	void open() override;

	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}
	}

	void update(LabMode newLabMode, int classIndex) override;

	SCP_string getTitle() override { return "Class Description"; }

	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship || labMode == LabMode::Weapon; }

private:
	DialogWindow* dialogWindow = nullptr;
	Text* descriptionText = nullptr;
};