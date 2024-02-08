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
	void onShipListEdited();
	void onWeaponListEdited();
	void onExtraShipSpinboxUpdated();
	void onExtraWeaponSpinboxUpdated();
	void onExtraShipComboboxUpdated();
	void onExtraWeaponComboboxUpdated();
	void onPlayerDelayDoubleSpinBoxUpdated();
	void onCurrentTeamSpinboxUpdated();
	void onCopyLoadoutToOtherTeamsButtonPressed();

	void updateUI();

	void sendEditedShips();
	void sendEditedWeapons();

	void resetLists();

	int _mode;

	SCP_vector<bool> _lastEnabledShips;
	SCP_vector<bool> _lastSelectedShips;
	SCP_vector<bool> _lastEnabledWeapons;
	SCP_vector<bool> _lastSelectedWeapons;
};

}
}
}

#endif // TEAMLOADOUTDIALOG_H
