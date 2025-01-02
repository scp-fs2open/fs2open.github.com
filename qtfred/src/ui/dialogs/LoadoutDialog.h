#ifndef TEAMLOADOUTDIALOG_H
#define TEAMLOADOUTDIALOG_H

#include <QDialog>
#include <QtWidgets/QDialog>
#include <mission/dialogs/LoadoutEditorDialogModel.h>
#include <ui/FredView.h>


namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class LoadoutDialog;
}

constexpr int NONE = -1;
constexpr int POTENTIAL_SHIPS = 0;
constexpr int POTENTIAL_WEAPONS = 1;
constexpr int USED_SHIPS = 2;
constexpr int USED_WEAPONS = 3;

class LoadoutDialog : public QDialog
{
    Q_OBJECT

public:
	explicit LoadoutDialog(FredView* parent, EditorViewport* viewport);
	~LoadoutDialog() override;

  protected:
	/**
	 * @brief Overides the Dialogs Close event to add a confermation dialog
	 * @param [in] *e The event.
	 */
	void closeEvent(QCloseEvent*) override;
	void rejectHandler();

  private:
	std::unique_ptr<Ui::LoadoutDialog> ui;
	std::unique_ptr<LoadoutDialogModel> _model;
	EditorViewport* _viewport;

	void onSwitchViewButtonPressed();
	void onExtraItemSpinboxUpdated();
	void onExtraItemsViaVariableCombo();
	void onPlayerDelayDoubleSpinBoxUpdated();
	void onCurrentTeamSpinboxUpdated();
	void onCopyLoadoutToOtherTeamsButtonPressed();
	void addShipButtonClicked();
	void addWeaponButtonClicked();
	void removeShipButtonClicked();
	void removeWeaponButtonClicked();
	void onPotentialShipListClicked()
	{
		_lastSelectionChanged = POTENTIAL_SHIPS;
		updateUI();
	}
	void onPotentialWeaponListClicked(){ 
		_lastSelectionChanged = POTENTIAL_WEAPONS;
		updateUI();
	}
	void onUsedShipListClicked(){ 
		_lastSelectionChanged = USED_SHIPS;
		updateUI();
	}
	void onUsedWeaponListClicked(){ 
		_lastSelectionChanged = USED_WEAPONS;
		updateUI();
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
	void onSelectionRequiredPressed();
	void onSelectionNotRequiredPressed();
	void onWeaponValidationCheckboxClicked();

	SCP_vector<SCP_string> getSelectedShips(); 
	SCP_vector<SCP_string> getSelectedWeapons(); 

	void updateUI();

	int _mode;
	int _lastSelectionChanged;
};

}
}
}

#endif // TEAMLOADOUTDIALOG_H
