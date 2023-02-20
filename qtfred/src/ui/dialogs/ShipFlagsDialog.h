#ifndef SHIPFLAGDIALOG_H
#define SHIPFLAGDIALOG_H

#include <mission/dialogs/ShipFlagsDialogModel.h>
#include <ui/FredView.h>
#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipFlagsDialog;
}
class ShipFlagsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipFlagsDialog(QWidget* parent, EditorViewport* viewport);
	~ShipFlagsDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;
	void showEvent(QShowEvent*) override;

  private:
	std::unique_ptr<Ui::ShipFlagsDialog> ui;
	std::unique_ptr<ShipFlagsDialogModel> _model;
	EditorViewport* _viewport;
	void updateUI();

	void destroyBeforeMissionChanged(int);
	void destroyBeforeMissionSecondsChanged(int);
	void scannableChanged(int);
	void cargoChanged(int);
	void subsytemScanningChanged(int);
	void reinforcementChanged(int);
	void protectShipChanged(int);
	void beamProtectChanged(int);
	void flakProtectChanged(int);
	void laserProtectChanged(int);
	void missileProtectChanged(int);
	void ignoreForGoalsChanged(int);
	void escortChanged(int);
	void escortValueChanged(int);
	void noArrivalMusicChanged(int);
	void invulnerableChanged(int);
	void guardianedChanged(int);
	void primitiveChanged(int);
	void noSubspaceChanged(int);
	void hiddenChanged(int);
	void stealthChanged(int);
	void friendlyStealthChanged(int);
	void kamikazeChanged(int);
	void kamikazeDamageChanged(int);
	void immobileChanged(int);
	void noDynamicGoalsChanged(int);
	void redAlertChanged(int);
	void gravityChanged(int);
	void warpinChanged(int);
	void targetableAsBombChanged(int);
	void disableBuiltInMessagesChanged(int);
	void neverScreamChanged(int);
	void alwaysScreamChanged(int);
	void vaporizeChanged(int);
	void respawnPriorityChanged(int);
	void autoCarryChanged(int);
	void autoLinkChanged(int);
	void hideShipNameChanged(int);
	void classDynamicChanged(int);
	void disableETSChanged(int);
	void cloakChanged(int);
	void scrambleMessagesChanged(int);
	void noCollideChanged(int);
	void noSelfDestructChanged(int);
	};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif // !SHIPFLAGDIALOG_H