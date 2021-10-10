#ifndef SHIPDEDITORDIALOG_H
#define SHIPDEDITORDIALOG_H

#include <QtWidgets/QDialog>
#include <mission/dialogs/ShipEditorDialogModel.h>
#include <ui/FredView.h>
#include "ShipGoalsDialog.h"
#include <QAbstractButton>
#include <QtWidgets/QDialog>


namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipEditorDialog;
}

class ShipEditorDialog : public QDialog, public SexpTreeEditorInterface {

	Q_OBJECT

  public:
	explicit ShipEditorDialog(FredView* parent, EditorViewport* viewport);
	~ShipEditorDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;
  private slots:

	void on_textureReplacementButton_clicked();
	void on_miscButton_clicked();
	void on_initialStatusButton_clicked();
	void on_initialOrdersButton_clicked();
	void on_tblInfoButton_clicked();
	void on_playerShipButton_clicked();
	void on_altShipClassButton_clicked();
	void on_prevButton_clicked();
	void on_nextButton_clicked();
	void on_deleteButton_clicked();
	void on_resetButton_clicked();
	void on_weaponsButton_clicked();
	void on_playerOrdersButton_clicked();
	void on_specialExpButton_clicked();
	void on_specialHitsButton_clicked();
	void on_hideCuesButton_clicked();
	void on_restrictArrivalPathsButton_clicked();
	void on_customWarpinButton_clicked();
	void on_restrictDeparturePathsButton_clicked();
	void on_customWarpoutButton_clicked();

  private:
	std::unique_ptr<Ui::ShipEditorDialog> ui;
	std::unique_ptr<ShipEditorDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void updateColumnOne();
	void updateColumnTwo();
	void updateArrival();
	void updateDeparture();
	void enableDisable();

	//column one
	void shipNameChanged(const QString&);
	void shipClassChanged(int);
	void aiClassChanged(int);
	void teamChanged(int);
	void cargoChanged();
	void altNameChanged(const QString&);
	void callsignChanged(const QString&);

	//column two
	void hotkeyChanged(int);
	void personaChanged(int);
	void scoreChanged(int);
	void assistChanged(int);
	void playerChanged(bool);

	//arrival
	void arrivalLocationChanged(int);
	void arrivalTargetChanged(int);
	void arrivalDistanceChanged(int);
	void arrivalDelayChanged(int);
	void arrivalWarpChanged(bool);
	void ArrivalCueChanged(bool);

	//departure
	void departureLocationChanged(int);
	void departureTargetChanged(int);
	void departureDelayChanged(int);
	void departureWarpChanged(bool);
	void DepartureCueChanged(bool);

};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif // SHIPDEDITORDIALOG_H