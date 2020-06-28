#pragma once

#include "lab/dialogs/lab_dialog.h"
#include "lab/labv2_internal.h"

class MaterialOverrides : public LabDialog {
	void open(Button* /*caller*/) override;

	void close() override {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;

		LMGR->Renderer->resetTextureOverride();
	}

	void update(LabMode /*newLabMode*/, int /*classIndex*/) override { /* Do Nothing */ }

	SCP_string getTitle() { return "Material Overrides"; }

	bool safeToOpen(LabMode /*labMode*/) { return true; }

private:
	DialogWindow* dialogWindow = nullptr;
};