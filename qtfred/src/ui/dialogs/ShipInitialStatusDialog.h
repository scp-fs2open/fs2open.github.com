#ifndef SHIPINITIALSTATUSDIALOG_H
#define SHIPINITIALSTATUSDIALOG_H

#include <mission/dialogs/ShipInitialStatusDialogModel.h>
#include "ShipEditorDialog.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipInitialStatusDialog;
}
class ShipEditorDialog;
class ShipInitialStatusDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipInitialStatusDialog(QWidget* parent, EditorViewport* viewport);
	~ShipInitialStatusDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;
	void showEvent(QShowEvent*) override;

  private:
	std::unique_ptr<Ui::ShipInitialStatusDialog> ui;
	std::unique_ptr<ShipInitialStatusDialogModel> _model;
	EditorViewport* _viewport;

	ShipEditorDialog* parentDialog;

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

	void velocityChanged(int);
	void hullChanged(int);
	void hasShieldChanged(int);
	void shieldHullChanged(int);
	// Flags
	void forceShieldChanged(int);
	void shipLockChanged(int);
	void weaponLockChanged(int);
	void primaryLockChanged(int);
	void secondaryLockChanged(int);
	void turretLockChanged(int);
	void afterburnerLockChanged(int);
	// Docking
	void dockChanged(QListWidgetItem*);
	void dockeeComboChanged(int);
	void dockeePointChanged(int);
	// Subsystems
	void subsystemChanged(int);
	void subIntegrityChanged(int);
	void cargoChanged();
	void colourChanged(int);
};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif // !SHIPINITIALSTATUSDIALOG_H
