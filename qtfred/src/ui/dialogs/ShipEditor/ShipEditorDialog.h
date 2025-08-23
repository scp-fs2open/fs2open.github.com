#ifndef SHIPDEDITORDIALOG_H
#define SHIPDEDITORDIALOG_H

#include "PlayerOrdersDialog.h"
#include "ShipCustomWarpDialog.h"
#include "ShipFlagsDialog.h"
#include "ShipGoalsDialog.h"
#include "ShipInitialStatusDialog.h"
#include "ShipPathsDialog.h"
#include "ShipSpecialStatsDialog.h"
#include "ShipTBLViewer.h"
#include "ShipTextureReplacementDialog.h"
#include "ShipWeaponsDialog.h"
#include "ShipPathsDialog.h"
#include "ShipCustomWarpDialog.h"
#include "ShipAltShipClass.h"

#include <mission/dialogs/ShipEditor/ShipEditorDialogModel.h>
#include <ui/FredView.h>

#include <QAbstractButton>
#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipEditorDialog;
}

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
	void hideEvent(QHideEvent*) override;
	void showEvent(QShowEvent*) override;
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

	// column one
	void on_shipNameEdit_editingFinished();
	void on_shipDisplayNameEdit_editingFinished();
	void on_shipClassCombo_currentIndexChanged(int);
	void on_AIClassCombo_currentIndexChanged(int);
	void on_teamCombo_currentIndexChanged(int);

	// column two
	void on_hotkeyCombo_currentIndexChanged(int);
	void on_personaCombo_currentIndexChanged(int);
	void on_killScoreEdit_valueChanged(int);
	void on_assistEdit_valueChanged(int);
	void on_playerShipCheckBox_toggled(bool);
	void on_respawnSpinBox_valueChanged(int);

	//arrival
	void on_arrivalLocationCombo_currentIndexChanged(int);
	void on_arrivalTargetCombo_currentIndexChanged(int);
	void on_arrivalDistanceEdit_valueChanged(int);
	void on_arrivalDelaySpinBox_valueChanged(int);
	void on_updateArrivalCueCheckBox_toggled(bool);
	void on_noArrivalWarpCheckBox_toggled(bool);
	void on_arrivalTree_rootNodeFormulaChanged(int, int);
	void on_arrivalTree_helpChanged(const QString&);
	void on_arrivalTree_miniHelpChanged(const QString&);

	//departure
	void on_departureLocationCombo_currentIndexChanged(int);
	void on_departureTargetCombo_currentIndexChanged(int);
	void on_departureDelaySpinBox_valueChanged(int);
	void on_updateDepartureCueCheckBox_toggled(bool);
	void on_departureTree_rootNodeFormulaChanged(int, int);
	void on_departureTree_helpChanged(const QString&);
	void on_departureTree_miniHelpChanged(const QString&);
	void on_noDepartureWarpCheckBox_toggled(bool);
  private:
	std::unique_ptr<Ui::ShipEditorDialog> ui;
	std::unique_ptr<ShipEditorDialogModel> _model;
	EditorViewport* _viewport;

	void update();

	void updateUI(bool overwrite = false);
	void updateColumnOne(bool overwrite = false);
	void updateColumnTwo(bool ovewrite = false);
	void updateArrival(bool overwrite = false);
	void updateDeparture(bool overwrite = false);
	void enableDisable();

	// column one
	void cargoChanged();
	void altNameChanged();
	void callsignChanged();
};
} // namespace fso::fred::dialogs

#endif // SHIPDEDITORDIALOG_H