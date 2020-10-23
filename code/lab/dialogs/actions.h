#pragma once

#include "lab/dialogs/lab_dialog.h"
#include "model/modelanim.h"


class DestroySubsystems : public LabDialog {

	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Destroy Subsystems"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship; }

private:
	DialogWindow* dialogWindow = nullptr;
};

class ChangeLoadout : public LabDialog {

	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Change Loadout"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship; }

private:
	DialogWindow* dialogWindow = nullptr;
};

class WeaponFire : public LabDialog {

	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Fire Weapons"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship; }

private:
	DialogWindow* dialogWindow = nullptr;
};

class AnimationTrigger : public LabDialog {
	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override;

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Trigger Animations"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode labMode) override { return labMode == LabMode::Ship; }

private:
	DialogWindow* dialogWindow = nullptr;
};

class Actions : public LabDialog {
public:
	Actions() {
		subDialogs.push_back(new DestroySubsystems());
		subDialogs.push_back(new ChangeLoadout());
		subDialogs.push_back(new WeaponFire());
		subDialogs.push_back(new AnimationTrigger());
	}

	// Called when this dialog is opened via the top toolbar
	void open(Button* /*caller*/) override;

	// Called when the dialog is closed
	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}

	// Called when the global state changes (e.g. other ship/weapon being selected)
	void update(LabMode newLabMode, int classIndex) override;

	// Returns the string to use for the top nav bar
	SCP_string getTitle() override { return "Actions"; }

	// Returns true if it is safe to open this dialog
	bool safeToOpen(LabMode /*labMode*/) override { return true; }

private:
	DialogWindow* dialogWindow = nullptr;
	SCP_vector<LabDialog*> subDialogs;
};