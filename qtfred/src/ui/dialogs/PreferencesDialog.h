#pragma once

#include <QDialog>

#include "mission/dialogs/PreferencesDialogModel.h"
#include "ui/FredView.h"

class QKeySequenceEdit;

namespace fso::fred::dialogs {

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
	Q_OBJECT

public:
	explicit PreferencesDialog(FredView* parent, EditorViewport* viewport);
	~PreferencesDialog() override;

	void accept() override;
	void reject() override;

private slots:
	// General
	void on_moveShipsWhenUndocking_toggled(bool checked);
	void on_alwaysSaveDisplayNames_toggled(bool checked);
	void on_errorCheckerChecksForPotentialIssues_toggled(bool checked);
	void on_showSexpHelpMissionEvents_toggled(bool checked);
	void on_showSexpHelpMissionGoals_toggled(bool checked);
	void on_showSexpHelpMissionCutscenes_toggled(bool checked);
	void on_showSexpHelpShipEditor_toggled(bool checked);
	void on_showSexpHelpWingEditor_toggled(bool checked);

	// Grid
	void on_xyPlaneRadio_toggled(bool checked);
	void on_xzPlaneRadio_toggled(bool checked);
	void on_yzPlaneRadio_toggled(bool checked);
	void on_gridCenterX_valueChanged(int value);
	void on_gridCenterY_valueChanged(int value);
	void on_gridCenterZ_valueChanged(int value);

	// Grid
	void on_resetGridButton_clicked();

	// Controls
	void on_resetDefaultsButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	std::unique_ptr<Ui::PreferencesDialog> ui;
	std::unique_ptr<PreferencesDialogModel> _model;
	std::map<ControlAction, QKeySequenceEdit*> _controlEditors;
};

} // namespace fso::fred::dialogs
