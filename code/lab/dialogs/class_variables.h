#pragma once

#include "lab/dialogs/lab_dialog.h"

class Variables : public LabDialog {
	void open(Button* caller) override;

	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}
	void update(LabMode newLabMode, int classIndex) override;

	SCP_string getTitle() override { return "Class Variables"; }

	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship || labMode == LabMode::Weapon; }

private:
	DialogWindow* dialogWindow;

	template<typename T>
	void addVariable(int* Y, const char* var_name, T& value);

	Text* addHeader(int& y, SCP_string text);
};