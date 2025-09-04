#pragma once

#include <QDialog>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QtWidgets/QDialog>
#include <mission/dialogs/TeamLoadoutDialogModel.h>
#include <ui/FredView.h>


namespace fso::fred::dialogs {

namespace Ui {
class TeamLoadoutDialog;
}

constexpr int NONE = -1;
constexpr int POTENTIAL_SHIPS = 0; // TODO make this an enum class
constexpr int POTENTIAL_WEAPONS = 1;
constexpr int USED_SHIPS = 2;
constexpr int USED_WEAPONS = 3;

class TeamLoadoutDialog : public QDialog
{
    Q_OBJECT

public:
	explicit TeamLoadoutDialog(FredView* parent, EditorViewport* viewport);
	~TeamLoadoutDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_shipsList_itemChanged(QTableWidgetItem*);
	void on_weaponsList_itemChanged(QTableWidgetItem*);
	void on_shipVarsList_itemChanged(QTableWidgetItem*);
	void on_weaponVarsList_itemChanged(QTableWidgetItem*);

	//void on_switchViewButton_clicked();
	//void on_editVariables_clicked();

	//void on_currentTeamComboBox_currentIndexChanged(int index);
	//void on_copyLoadoutToOtherTeamsButton_clicked();

	//void on_clearUsedShipSelectionButton_clicked();
	//void on_selectAllUsedShipsButton_clicked();
	//void on_usedShipsList_itemChanged(QTableWidgetItem* /*item*/);
	//void on_usedShipsList_itemSelectionChanged();

	//void on_clearUnusedShipSelectionButton_clicked();
	//void on_selectAllUnusedShipsButton_clicked();
	//void on_listShipsNotUsed_itemChanged(QListWidgetItem* /*item*/);
	//void on_listShipsNotUsed_itemSelectionChanged();

	//void on_addShipButton_clicked();
	//void on_removeShipButton_clicked();

	//void on_clearUsedWeaponSelectionButton_clicked();
	//void on_selectAllUsedWeaponsButton_clicked();
	//void on_usedWeaponsList_itemChanged(QTableWidgetItem* /*item*/);
	//void on_usedWeaponsList_itemSelectionChanged();

	//void on_clearUnusedWeaponSelectionButton_clicked();
	//void on_selectAllUnusedWeaponsButton_clicked();
	//void on_listWeaponsNotUsed_itemChanged(QListWidgetItem* /*item*/);
	//void on_listWeaponsNotUsed_itemSelectionChanged();

	//void on_addWeaponButton_clicked();
	//void on_removeWeaponButton_clicked();

	//void on_extraItemSpinbox_valueChanged(int arg1);
	//void on_extraItemsViaVariableCombo_currentIndexChanged(int /*index*/);
	//void on_requiredWeaponCheckBox_clicked(bool checked);

	//void on_weaponValidationCheckbox_clicked(bool checked);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::TeamLoadoutDialog> ui;
	std::unique_ptr<TeamLoadoutDialogModel> _model;
	EditorViewport* _viewport;

	SCP_vector<SCP_string> getSelectedShips(); 
	SCP_vector<SCP_string> getSelectedWeapons(); 

	void initializeUi();
	void updateUi();
	void populateShipsList();
	void populateWeaponsList();
	void populateShipVarsList();
	void populateWeaponVarsList();

	void rebuildShipRowFromModel(int row);
	void refreshShipsRows(const QList<int>& tableRows);

	static QList<int> selectedRowNumbers(QTableWidget* tbl);

	int _mode;
	int _lastSelectionChanged;
};

} // namespace fso::fred::dialogs
