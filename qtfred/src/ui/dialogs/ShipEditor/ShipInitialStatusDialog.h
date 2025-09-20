#ifndef SHIPINITIALSTATUSDIALOG_H
#define SHIPINITIALSTATUSDIALOG_H

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
	void on_colourComboBox_currentIndexChanged(int);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipInitialStatusDialog> ui;
	std::unique_ptr<ShipInitialStatusDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void updateFlags();
	void updateDocks();
	void updateDockee();
	void list_dockees(int);
	void list_dockee_points(int);
	void updateSubsystems();

	int cur_docker_point = -1;
	int cur_dockee = -1;
	int cur_dockee_point = -1;
};
} // namespace fso::fred::dialogs

#endif // !SHIPINITIALSTATUSDIALOG_H
