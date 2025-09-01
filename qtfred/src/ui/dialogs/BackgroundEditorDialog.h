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

	// Backgrounds
	void on_backgroundSelectionCombo_currentIndexChanged(int index);
	void on_addButton_clicked();
	void on_removeButton_clicked();
	void on_importButton_clicked();
	void on_swapWithButton_clicked();
	void on_swapWithCombo_currentIndexChanged(int index);
	void on_useCorrectAngleFormatCheckBox_toggled(bool checked);

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

	// Ambient Light
	void on_ambientLightRedSlider_valueChanged(int value);
	void on_ambientLightGreenSlider_valueChanged(int value);
	void on_ambientLightBlueSlider_valueChanged(int value);

	// Skybox
	void on_skyboxModelButton_clicked();
	void on_skyboxEdit_textChanged(const QString& arg1);
	void on_skyboxPitchSpin_valueChanged(int arg1);
	void on_skyboxBankSpin_valueChanged(int arg1);
	void on_skyboxHeadingSpin_valueChanged(int arg1);
	void on_skyboxNoLightingCheckBox_toggled(bool checked);
	void on_noLightingCheckBox_toggled(bool checked);
	void on_transparentCheckBox_toggled(bool checked);
	void on_forceClampCheckBox_toggled(bool checked);
	void on_noZBufferCheckBox_toggled(bool checked);
	void on_noCullCheckBox_toggled(bool checked);
	void on_noGlowmapsCheckBox_toggled(bool checked);

	// Misc
	void on_numStarsSlider_valueChanged(int value);
	void on_subspaceCheckBox_toggled(bool checked);
	void on_envMapButton_clicked();
	void on_envMapEdit_textChanged(const QString& arg1);
	void on_lightingProfileCombo_currentIndexChanged(int index);

private: // NOLINT(readability-redundant-access-specifiers)
    std::unique_ptr<Ui::BackgroundEditor> ui;
	std::unique_ptr<BackgroundEditorDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();
	void updateBackgroundControls();
	void refreshBitmapList();
	void updateBitmapControls();
	void refreshSunList();
	void updateSunControls();
	void updateNebulaControls();
	void updateFogSwatch();
	void updateOldNebulaControls();
	void updateAmbientLightControls();
	void updateAmbientSwatch();
	void updateSkyboxControls();
	void updateMiscControls();

	static int pickBackgroundIndexDialog(QWidget* parent, int count, int defaultIndex = 0);
};

} // namespace fso::fred::dialogs
