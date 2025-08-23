#pragma once

#include <ui/FredView.h>
#include <mission/dialogs/WingEditorDialogModel.h>

#include <QDialog>
#include <memory>

namespace fso::fred::dialogs {

namespace Ui {
class WingEditorDialog;
}

class WingEditorDialog : public QDialog, public SexpTreeEditorInterface {
	Q_OBJECT
	public:
		explicit WingEditorDialog(FredView* parent, EditorViewport* viewport);
		~WingEditorDialog() override;

	private slots:
		void on_hideCuesButton_clicked();

		// Top section, first column
		void on_wingNameEdit_editingFinished();
		void on_wingLeaderCombo_currentIndexChanged(int index);
		void on_numberOfWavesSpinBox_valueChanged(int value);
		void on_waveThresholdSpinBox_valueChanged(int value);
		void on_hotkeyCombo_currentIndexChanged(int /*index*/);

		// Top section, second column
		void on_formationCombo_currentIndexChanged(int /*index*/);
		void on_formationScaleSpinBox_valueChanged(double value);
		void on_alignFormationButton_clicked();
		void on_setSquadLogoButton_clicked();

		// Top section, third column
		void on_prevWingButton_clicked();
		void on_nextWingButton_clicked();
		void on_deleteWingButton_clicked();
		void on_disbandWingButton_clicked();
		void on_initialOrdersButton_clicked();
		void on_wingFlagsButton_clicked();

		// Arrival controls
		void on_arrivalLocationCombo_currentIndexChanged(int /*index*/);
		void on_arrivalDelaySpinBox_valueChanged(int value);
		void on_minDelaySpinBox_valueChanged(int value);
		void on_maxDelaySpinBox_valueChanged(int value);
		void on_arrivalTargetCombo_currentIndexChanged(int /*index*/);
		void on_arrivalDistanceSpinBox_valueChanged(int value);
		void on_restrictArrivalPathsButton_clicked();
		void on_customWarpinButton_clicked();
		void on_arrivalTree_nodeChanged(int newTree);
		void on_noArrivalWarpCheckBox_toggled(bool checked);
		void on_noArrivalWarpAdjustCheckbox_toggled(bool checked);

		// Departure controls
		void on_departureLocationCombo_currentIndexChanged(int /*index*/);
		void on_departureDelaySpinBox_valueChanged(int value);
		void on_departureTargetCombo_currentIndexChanged(int /*index*/);
		void on_restrictDeparturePathsButton_clicked();
		void on_customWarpoutButton_clicked();
		void on_departureTree_nodeChanged(int newTree);
		void on_noDepartureWarpCheckBox_toggled(bool checked);
		void on_noDepartureWarpAdjustCheckbox_toggled(bool checked);

		// Sexp help text
		void on_arrivalTree_helpChanged(const QString& help);
		void on_arrivalTree_miniHelpChanged(const QString& help);
		void on_departureTree_helpChanged(const QString& help);
		void on_departureTree_miniHelpChanged(const QString& help);

	private: // NOLINT(readability-redundant-access-specifiers)
		std::unique_ptr<Ui::WingEditorDialog> ui;
		std::unique_ptr<WingEditorDialogModel> _model;
		EditorViewport* _viewport;

		bool _cues_hidden = false;

		void updateUi();
		void enableOrDisableControls();

		void clearArrivalFields();
		void clearDepartureFields();
		void clearGeneralFields();

		void refreshLeaderCombo();
		void refreshHotkeyCombo();
		void refreshFormationCombo();
		void refreshArrivalLocationCombo();
		void refreshDepartureLocationCombo();
		void refreshArrivalTargetCombo();
		void refreshDepartureTargetCombo();
		void refreshAllDynamicCombos();

		void updateLogoPreview();
};

} // namespace fso::fred::dialogs
