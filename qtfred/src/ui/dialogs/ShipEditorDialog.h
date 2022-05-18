#ifndef SHIPDEDITORDIALOG_H
#define SHIPDEDITORDIALOG_H

#include <QtWidgets/QDialog>
#include <mission/dialogs/ShipEditorDialogModel.h>
#include <ui/FredView.h>
#include "ShipGoalsDialog.h"
#include "ShipInitialStatusDialog.h"
#include "ShipFlagsDialog.h"
#include "PlayerOrdersDialog.h"
#include "ShipSpecialStatsDialog.h"
#include "ShipTextureReplacementDialog.h"
#include "ShipTBLViewer.h"

#include <QAbstractButton>
#include <QtWidgets/QDialog>


namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipEditorDialog;
}

class ShipTBLViewer;
class ShipGoalsDialog;
class ShipInitialStatusDialog;
class ShipFlagsDialog;
class ShipTextureReplacementDialog;
class PlayerOrdersDialog;

/**
* @brief QTFred's Ship Editor
*/
class ShipEditorDialog : public QDialog, public SexpTreeEditorInterface {

	Q_OBJECT

  public:
	/**
	 * @brief Constructor
	 * @param parent The main fred window. Needed for triggering window updates.
	 * @param viewport The viewport this dialog is attacted to.
	 */
	explicit ShipEditorDialog(FredView* parent, EditorViewport* viewport);
	~ShipEditorDialog() override;

	/**
	 * @brief Allows subdialogs to get the ships class
	 * @return Returns the ship_info_index of the current ship or -1 if multiple ships selected.
	 */
	int getShipClass() const;

	/**
	 * @brief Allows subdialogs to get the ship the editor is currently working on.
	 * @return Returns the index in Ships if working on one or -1 if working on multiple.
	 */
	int getSingleShip() const;

	/**
	 * @brief Allows subdialogs to know if we are working on multiple ships.
	 * @return true if multiple ships are selected.
	 */
	bool getIfMultipleShips() const;

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
	void on_specialStatsButton_clicked();
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
	void shipNameChanged();
	void shipClassChanged(const int);
	void aiClassChanged(const int);
	void teamChanged(const int);
	void cargoChanged();
	void altNameChanged();
	void altNameChanged(const QString&);
	void callsignChanged();
	void callsignChanged(const QString&);

	//column two
	void hotkeyChanged(const int);
	void personaChanged(const int);
	void scoreChanged(const int);
	void assistChanged(const int);
	void playerChanged(const bool);

	//arrival
	void arrivalLocationChanged(const int);
	void arrivalTargetChanged(const int);
	void arrivalDistanceChanged(const int);
	void arrivalDelayChanged(const int);
	void arrivalWarpChanged(const bool);
	void ArrivalCueChanged(const bool);

	//departure
	void departureLocationChanged(const int);
	void departureTargetChanged(const int);
	void departureDelayChanged(const int);
	void departureWarpChanged(const bool);
	void DepartureCueChanged(const bool);

	std::unique_ptr<ShipGoalsDialog> GoalsDialog = nullptr;
	std::unique_ptr<ShipInitialStatusDialog> initialStatusDialog = nullptr;
	std::unique_ptr<ShipFlagsDialog> flagsDialog = nullptr;
	std::unique_ptr<ShipTextureReplacementDialog> TextureReplacementDialog = nullptr;
	std::unique_ptr<PlayerOrdersDialog> playerOrdersDialog = nullptr;
	std::unique_ptr<ShipSpecialStatsDialog> specialStatsDialog = nullptr;
	std::unique_ptr<ShipTBLViewer> TBLViewer = nullptr;
};
} // namespace dialogs
} // namespace fred
} // namespace fso

#endif // SHIPDEDITORDIALOG_H