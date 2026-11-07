#pragma once

#include <QtWidgets/QDialog>

#include "mission/dialogs/BackgroundEditorDialogModel.h"

class QCheckBox;
class QLabel;
class QSlider;

namespace fso::fred::dialogs {

namespace Ui {
class LensApertureDialog;
}

// Live editor for how the mission restyles the camera lens: the iris (shape plus
// grating/scratches/dust), the anamorphic look, and how strongly the flare draws
// -- everything about the camera except the lens prescription itself, which is
// what naming a lens in the Background Editor picks. See graphics/lens_flare.h's
// lens_settings for the whole set and the set-lens-* sexps this mirrors.
//
// Opened from the Background Editor's "Lens Aperture..." button; every control
// applies straight to BackgroundEditorDialogModel as it is dragged, the same way
// that dialog's own ambient light sliders do, so there is no separate Apply/OK
// step.
//
// The controls are not laid out in the .ui file. They are built from the field
// table at the top of LensApertureDialog.cpp, which is also what reads and writes
// them, so a knob added to lens_settings needs one line there rather than a
// widget, a build entry, a load line and a save line that can all drift apart.
class LensApertureDialog : public QDialog {
	Q_OBJECT

  public:
	explicit LensApertureDialog(QWidget* parent, BackgroundEditorDialogModel* model);
	~LensApertureDialog() override;

  private:
	// The widgets built for one field: a slider plus its value readout, or a
	// checkbox. Owned by the layout, like everything else Qt parents.
	struct row {
		QSlider* slider = nullptr;
		QLabel* value = nullptr;
		QCheckBox* check = nullptr;
	};

	void buildFields();
	void showValue(int index, float value);
	void updateUi();
	void applyFromControls();

	std::unique_ptr<Ui::LensApertureDialog> ui;
	BackgroundEditorDialogModel* _model;
	SCP_vector<row> _rows; // parallel to the field table
};

} // namespace fso::fred::dialogs
