#pragma once

#include "lab/dialogs/lab_dialog.h"

class Variables : public LabDialog {
public:
	~Variables() override = default;

private:
	void open(Button* caller) override;

	void close() override {
		if (dialogWindow != nullptr) {
			dialogWindow->DeleteChildren();
			dialogWindow = nullptr;
		}
	}
	void update(LabMode newLabMode, int classIndex) override;

	SCP_string getTitle() override { return "Class Variables"; }

	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship || labMode == LabMode::Weapon; }

private:
	DialogWindow* dialogWindow;

	template<typename T>
	void addVariable(int* Y, const char* var_name, T& value);

	Text* addHeader(int& y, const SCP_string& text);
};