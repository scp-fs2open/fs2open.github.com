#ifndef TEAMLOADOUTDIALOG_H
#define TEAMLOADOUTDIALOG_H

#include <QtWidgets/QDialog>
#include <mission/dialogs/LoadoutEditorDialogModel.h>
#include <ui/FredView.h>


namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class LoadoutDialog;
}

class LoadoutDialog : public QDialog
{
    Q_OBJECT

public:
	explicit LoadoutDialog(FredView* parent, EditorViewport* viewport);
	~LoadoutDialog() override;

private:
	std::unique_ptr<Ui::LoadoutDialog> ui;
	std::unique_ptr<LoadoutDialogModel> _model;
	EditorViewport* _viewport;

	void onSwitchViewButtonPressed();
	void onExtraItemSpinbox();
	void onExtraItemsViaVariableCombo();
	void onPlayerDelayDoubleSpinBoxUpdated();
	void onCurrentTeamSpinboxUpdated();
	void onCopyLoadoutToOtherTeamsButtonPressed();
	void addShipButtonClicked();
	void addWeaponButtonClicked();
	void removeShipButtonClicked();
	void removeWeaponButtonClicked();
	void onPotentialShipListClicked(){ _lastSelectionChanged = POTENTIAL_SHIPS;}
	void onPotentialWeaponListClicked(){ _lastSelectionChanged = POTENTIAL_WEAPONS;}
	void onUsedShipListClicked(){ _lastSelectionChanged = USED_SHIPS;}
	void onPotentialShipListClicked(){ _lastSelectionChanged = USED_WEAPONS;}

	void updateUI();

	void resetLists();

	int _mode;
	int _lastSelectionChanged
};

}
}
}

#endif // TEAMLOADOUTDIALOG_H
