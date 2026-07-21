#pragma once

#include <mission/dialogs/ShipEditor/ShipInitialStatusDialogModel.h>

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>

namespace fso::fred::dialogs {

namespace Ui {
class ShipInitialStatusDialog;
}
class ShipInitialStatusDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipInitialStatusDialog(QDialog* parent, EditorViewport* viewport, bool editMultiple);
	~ShipInitialStatusDialog() override;

	// Positions of ships that will be moved by docking when accept() is called,
	// captured before apply() runs.  Retrieved by ShipEditorDialog via accepted().
	struct PreApplyDockeePos { int sig; vec3d pos; matrix orient; };
	const SCP_vector<PreApplyDockeePos>& preApplyDockeePositions() const { return _preApplyDockeePositions; }

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	void on_okPushButton_clicked();
	void on_cancelPushButton_clicked();
	void on_guardianSpinBox_valueChanged(int);
	void on_velocitySpinBox_valueChanged(int);
	void on_dockpointList_currentItemChanged(QListWidgetItem*);
	void on_dockeeComboBox_currentIndexChanged(int);
	void on_dockeePointComboBox_currentIndexChanged(int);
	void on_hullSpinBox_valueChanged(int);
	void on_hasShieldCheckBox_stateChanged(int);
	void on_shieldHullSpinBox_valueChanged(int);
	void on_forceShieldsCheckBox_stateChanged(int);
	void on_shipLockCheckBox_stateChanged(int);
	void on_weaponLockCheckBox_stateChanged(int);
	void on_primaryLockCheckBox_stateChanged(int);
	void on_secondaryLockCheckBox_stateChanged(int);
	void on_turretLockCheckBox_stateChanged(int);
	void on_afterburnerLockCheckBox_stateChanged(int);
	void on_subsystemList_currentRowChanged(int);
	void on_subIntegritySpinBox_valueChanged(int);
	void on_cargoEdit_editingFinished();
	void on_cargoTitleEdit_editingFinished();
	void on_colourComboBox_currentIndexChanged(int);
	void on_moveShipsCheckBox_toggled(bool);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipInitialStatusDialog> ui;
	std::unique_ptr<ShipInitialStatusDialogModel> _model;
	EditorViewport* _viewport;

	void updateUi();
	void updateFlags();
	void updateDocks();
	void updateDockee();
	void listDockees(int);
	void listDockeePoints(int);
	void updateSubsystems();

	int _curDockerPoint = -1;
	int _curDockee = -1;
	int _curDockeePoint = -1;

	SCP_vector<PreApplyDockeePos> _preApplyDockeePositions;
};
} // namespace fso::fred::dialogs
