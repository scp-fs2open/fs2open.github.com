#pragma once

#include <QDialog>
#include <QtWidgets/QDialog>
#include <mission/dialogs/TeamLoadoutDialogModel.h>
#include <ui/FredView.h>


namespace fso::fred::dialogs {

namespace Ui {
class TeamLoadoutDialog;
}

constexpr int NONE = -1;
constexpr int POTENTIAL_SHIPS = 0;
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
	void rejectHandler();

  private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_currentTeamComboBox_currentIndexChanged(int index);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::TeamLoadoutDialog> ui;
	std::unique_ptr<TeamLoadoutDialogModel> _model;
	EditorViewport* _viewport;

	void onSwitchViewButtonPressed();
	void onExtraItemSpinboxUpdated();
	void onExtraItemsViaVariableCombo();
	void onPlayerDelayDoubleSpinBoxUpdated();
	void onCopyLoadoutToOtherTeamsButtonPressed();
	void addShipButtonClicked();
	void addWeaponButtonClicked();
	void removeShipButtonClicked();
	void removeWeaponButtonClicked();
	void onPotentialShipListClicked()
	{
		_lastSelectionChanged = POTENTIAL_SHIPS;
		updateUi();
	}
	void onPotentialWeaponListClicked(){ 
		_lastSelectionChanged = POTENTIAL_WEAPONS;
		updateUi();
	}
	void onUsedShipListClicked(){ 
		_lastSelectionChanged = USED_SHIPS;
		updateUi();
	}
	void onUsedWeaponListClicked(){ 
		_lastSelectionChanged = USED_WEAPONS;
		updateUi();
	}

	void onSelectAllUnusedShipsPressed();
	void onClearAllUnusedShipsPressed();
	void onSelectAllUnusedWeaponsPressed();
	void onClearAllUnusedWeaponsPressed();
	void onSelectAllUsedShipsPressed();
	void onClearAllUsedShipsPressed();
	void onSelectAllUsedWeaponsPressed();
	void onClearAllUsedWeaponsPressed();
	void openEditVariablePressed();
	void onSelectionRequiredCheckbox();
	void onSelectionRequiredPressed();
	void onSelectionNotRequiredPressed();
	void onWeaponValidationCheckboxClicked();

	SCP_vector<SCP_string> getSelectedShips(); 
	SCP_vector<SCP_string> getSelectedWeapons(); 

	void updateUi();
	void initializeUi();

	int _mode;
	int _lastSelectionChanged;
};

} // namespace fso::fred::dialogs
