#pragma once

#include "mission/dialogs/BackgroundEditorDialogModel.h"
#include <QtWidgets/QDialog>

#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class BackgroundEditor;
}

class BackgroundEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackgroundEditorDialog(FredView* parent, EditorViewport* viewport);
	~BackgroundEditorDialog() override;

private slots:

	// Bitmaps
	void on_bitmapListWidget_currentRowChanged(int row);
	void on_bitmapTypeCombo_currentIndexChanged(int index);
	void on_bitmapPitchSpin_valueChanged(int arg1);
	void on_bitmapBankSpin_valueChanged(int arg1);
	void on_bitmapHeadingSpin_valueChanged(int arg1);
	void on_bitmapScaleXDoubleSpinBox_valueChanged(double arg1);
	void on_bitmapScaleYDoubleSpinBox_valueChanged(double arg1);
	void on_bitmapDivXSpinBox_valueChanged(int arg1);
	void on_bitmapDivYSpinBox_valueChanged(int arg1);
	void on_addBitmapButton_clicked();
	void on_changeBitmapButton_clicked();
	void on_deleteBitmapButton_clicked();

	// Suns
	void on_sunListWidget_currentRowChanged(int row);
	void on_sunSelectionCombo_currentIndexChanged(int index);
	void on_sunPitchSpin_valueChanged(int arg1);
	void on_sunHeadingSpin_valueChanged(int arg1);
	void on_sunScaleDoubleSpinBox_valueChanged(double arg1);
	void on_addSunButton_clicked();
	void on_changeSunButton_clicked();
	void on_deleteSunButton_clicked();

	// Nebula
	void on_fullNebulaCheckBox_toggled(bool checked);
	void on_rangeSpinBox_valueChanged(int arg1);
	void on_nebulaPatternCombo_currentIndexChanged(int index);
	void on_nebulaLightningCombo_currentIndexChanged(int index);
	void on_poofsListWidget_itemSelectionChanged();
	void on_shipTrailsCheckBox_toggled(bool checked);
	void on_fogNearDoubleSpinBox_valueChanged(double arg1);
	void on_fogFarDoubleSpinBox_valueChanged(double arg1);
	void on_displayBgsInNebulaCheckbox_toggled(bool checked);
	void on_overrideFogPaletteCheckBox_toggled(bool checked);
	void on_fogOverrideRedSpinBox_valueChanged(int arg1);
	void on_fogOverrideGreenSpinBox_valueChanged(int arg1);
	void on_fogOverrideBlueSpinBox_valueChanged(int arg1);

	// Old Nebula
	void on_oldNebulaPatternCombo_currentIndexChanged(int index);
	void on_oldNebulaColorCombo_currentIndexChanged(int index);
	void on_oldNebulaPitchSpinBox_valueChanged(int arg1);
	void on_oldNebulaBankSpinBox_valueChanged(int arg1);
	void on_oldNebulaHeadingSpinBox_valueChanged(int arg1);

private: // NOLINT(readability-redundant-access-specifiers)
    std::unique_ptr<Ui::BackgroundEditor> ui;
	std::unique_ptr<BackgroundEditorDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void refreshBitmapList();
	void updateBitmapControls();
	void refreshSunList();
	void updateSunControls();
	void updateNebulaControls();
	void updateOldNebulaControls();
};

} // namespace fso::fred::dialogs
