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

	void on_currentTeamComboBox_currentIndexChanged(int index);
	void on_copyLoadoutToOtherTeamsButton_clicked();
	void on_weaponValidationCheckbox_toggled(bool checked);

	void on_shipsFilterLineEdit_textChanged(const QString& arg1);
	void on_weaponsFilterLineEdit_textChanged(const QString& arg1);
	void on_shipsVarFilterLineEdit_textChanged(const QString& arg1);
	void on_weaponsVarFilterLineEdit_textChanged(const QString& arg1);

	void on_clearShipsListButton_clicked();
	void on_clearWeaponsListButton_clicked();
	void on_clearShipsVarListButton_clicked();
	void on_clearWeaponsVarListButton_clicked();

	void on_selectAllShipsListButton_clicked();
	void on_selectAllWeaponsListButton_clicked();
	void on_selectAllShipsVarListButton_clicked();
	void on_selectAllWeaponsVarListButton_clicked();

	void on_shipsMultiSelectCheckBox_toggled(bool checked);
	void on_weaponsMultiSelectCheckBox_toggled(bool checked);
	void on_shipsVarMultiSelectCheckBox_toggled(bool checked);
	void on_weaponsVarMultiSelectCheckBox_toggled(bool checked);

	void on_shipsList_itemChanged(QTableWidgetItem*);
	void on_weaponsList_itemChanged(QTableWidgetItem*);
	void on_shipVarsList_itemChanged(QTableWidgetItem*);
	void on_weaponVarsList_itemChanged(QTableWidgetItem*);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::TeamLoadoutDialog> ui;
	std::unique_ptr<TeamLoadoutDialogModel> _model;
	EditorViewport* _viewport;

	void initializeUi();
	void updateUi();

	void populateShipsList();
	void populateWeaponsList();
	void populateShipVarsList();
	void populateWeaponVarsList();

	static QList<int> selectedRowNumbers(QTableWidget* tbl);

	void rebuildShipRowFromModel(int row);
	void refreshShipRows(const QList<int>& tableRows);
	void rebuildWeaponRowFromModel(int row);
	void refreshWeaponRows(const QList<int>& tableRows);
	void rebuildShipVarRowFromModel(int row);
	void refreshShipVarRows(const QList<int>& tableRows);
	void rebuildWeaponVarRowFromModel(int row);
	void refreshWeaponVarRows(const QList<int>& tableRows);

	// Methods to create and set up widgets for a new row
	static void setupEnabledCell(QTableWidget* tbl, int row, int classIndex);
	static void setupNameCell(QTableWidget* tbl, int row, int classIndex, const QString& name);
	static void setupWingOrVarValueCell(QTableWidget* tbl, int row, int classIndex, const QString& value);
	void setupExtraCountCell(QTableWidget* tbl, int row, int classIndex);
	void setupVarCountCell(QTableWidget* tbl, int row, int classIndex);
	static void setupRequiredCell(QTableWidget* tbl, int row, int classIndex);

	// Methods to refresh the state of widgets from the model
	static void refreshEnabledCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem);
	static void refreshExtraCountCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem);
	static void refreshVarCountCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem);
	static void refreshRequiredCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem);
};

} // namespace fso::fred::dialogs
