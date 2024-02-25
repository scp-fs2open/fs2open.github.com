#include "LoadoutDialog.h"
#include "ui_LoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>

constexpr int TABLE_MODE = 0;
constexpr int VARIABLE_MODE = 1;

// header text
constexpr char* SHIPHEADER = "Ship";
constexpr char* WEAPONHEADER = "Weapon";
constexpr char* KEYHEADER = "In wings / Extra / Total";


namespace fso {
namespace fred {
namespace dialogs {
	//ui includes: tableVarLabel, switches between "Loadout Editor: Loadout View" and "Loadout Editor: Variable View"
	//switchViewButton for switching modes
	//startingShipsLabel "Ships Not in Loadout"
	//listShipsNotUsed -- The list of ships that is not present in the loadout.
	//listWeaponsNotUsed -- The list of weapons that is not present in the loadout
	//usedShipsList
	//usedWeaponsList
	//addShipButton
	//addWeaponButton
	//removeShipButton
	//removeWeaponButton
	//copyLoadoutToOtherTeamsButton
	//currentTeamSpinbox
	//playerDelayDoubleSpinbox
	//extraItemSpinbox
	//extraItemsViaVariableCombo

LoadoutDialog::LoadoutDialog(FredView* parent, EditorViewport* viewport) 
	: QDialog(parent), ui(new Ui::LoadoutDialog()), _model(new LoadoutDialogModel(this, viewport)),
		_viewport(viewport)
{
	this->setFocus();
    ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &LoadoutDialog::updateUI);
	connect(this, &QDialog::accepted, _model.get(), &LoadoutDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &LoadoutDialogModel::reject);

	connect(ui->currentTeamSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onCurrentTeamSpinboxUpdated);

	connect(ui->extraShipSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onExtraShipSpinboxUpdated);

	connect(ui->playerDelayDoubleSpinbox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		this,
		&LoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated);

	connect(ui->extraWeaponsViaVarCombo,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&LoadoutDialog::onExtraWeaponComboboxUpdated);

	connect(ui->copyLoadoutToOtherTeamsButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed);

	connect(ui->switchViewButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onSwitchViewButtonPressed);

	connect(ui->shipVarList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onShipListEdited);

	connect(ui->weaponVarList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onWeaponListEdited);


	updateUI();

/*

	void onSwitchViewButtonPressed();
	void onExtraShipComboboxUpdated();
	void onExtraWeaponComboboxUpdated();
	void onPlayerDelayDoubleSpinBoxUpdated();
	void onCurrentTeamSpinboxUpdated();
	void onExtraShipSpinboxUpdated();
	void onExtraWeaponSpinboxUpdated();
	void onCopyLoadoutToOtherTeamsButtonPressed();
	void onShipListEdited();
	void onWeaponListEdited();


	*/

	connect(ui->extraShipSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onExtraShipSpinboxUpdated);

	connect(ui->extraWepSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onExtraWeaponSpinboxUpdated);
	
	connect(ui->playerDelayDoubleSpinbox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		this,
		&LoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated);

	connect(ui->extraShipsViaVarCombo,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&LoadoutDialog::onExtraShipComboboxUpdated);

	connect(ui->extraWeaponsViaVarCombo,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&LoadoutDialog::onExtraWeaponComboboxUpdated);

	connect(ui->copyLoadoutToOtherTeamsButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed);

	connect(ui->switchViewButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onSwitchViewButtonPressed);

	connect(ui->shipVarList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onShipListEdited);

	connect(ui->weaponVarList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onWeaponListEdited);

	// things that must be set for everything to work...
	_mode = TABLE_MODE;
	
	// rows will vary but columns need to be 3
	ui->shipVarList->setColumnCount(3);
	ui->weaponVarList->setColumnCount(3);
	
	// set sizes
	ui->shipVarList->setColumnWidth(0,10);
	ui->weaponVarList->setColumnWidth(0,10);
	ui->shipVarList->setColumnWidth(1,200);
	ui->weaponVarList->setColumnWidth(1,200);
	ui->shipVarList->setColumnWidth(2,200);
	ui->weaponVarList->setColumnWidth(2,200);

	// set headers
	ui->shipVarList->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
	ui->weaponVarList->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
	ui->shipVarList->setHorizontalHeaderItem(1, new QTableWidgetItem(SHIPHEADER));
	ui->weaponVarList->setHorizontalHeaderItem(1, new QTableWidgetItem(WEAPONHEADER));
	ui->shipVarList->setHorizontalHeaderItem(2, new QTableWidgetItem(KEYHEADER));
	ui->weaponVarList->setHorizontalHeaderItem(2, new QTableWidgetItem(KEYHEADER));

	// quickly enable or diable the team spin box (must not get to multiple teams if in SP!)
	if (The_mission.game_type & MISSION_TYPE_MULTI){
		ui->currentTeamSpinbox->setEnabled(true);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(true);
	}
	else {
		ui->currentTeamSpinbox->setEnabled(false);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(false);
	}

	// need to completely rebuild the lists here.
	resetLists();
	updateUI();
}

// a product of competing CI requirements
LoadoutDialog::~LoadoutDialog(){} // NOLINT

void LoadoutDialog::onSwitchViewButtonPressed()
{
	// Change important lables.
	if (_mode == TABLE_MODE) {
		ui->tableVarLabel->setText("Enable Via Variable View");
		ui->startingShipsLabel->setText("Ship-Enabling Variables");
		ui->startingWeaponsLabel->setText("Weapon-Enabling Variables");
		_mode = VARIABLE_MODE;
	}
	else {
		ui->tableVarLabel->setText("Table Entry View");
		ui->startingShipsLabel->setText("Starting Ships");
		ui->startingWeaponsLabel->setText("Starting Weapons");
		_mode = TABLE_MODE;
	}

	// model does not keep track of whether the UI is editing the table values or the vars
	// so, just reset the lists and then update the UI
	resetLists();
	updateUI();
}

void LoadoutDialog::onShipListEdited()
{
	SCP_vector<bool> newSelectedStatus;
	SCP_vector<bool> newEnabledStatus;
	bool newStatus, checksChanged = false, selectionChanged = false;

	// we need the index, so iterate through just the first column.
	for (int i = 0; i < ui->shipVarList->rowCount(); i++) {
		bool checkState = (ui->shipVarList->item(i,0)->checkState() == Qt::Checked);
		newEnabledStatus.push_back(checkState);
		
		// compare the old to the new to see if there was a change in check marks. This method would break if the amount of
		// ships or weapons ever became dynamic (dynamic with respect to this dialog) and we would then have to rely on names.
		if (!_lastEnabledShips.empty() && (_lastEnabledShips[i] != newEnabledStatus[i])) {
			checksChanged = true;
			newStatus = checkState;
		}

		bool selectState = (ui->shipVarList->item(i, 0)->isSelected());
		newSelectedStatus.push_back(selectState);

		if (!_lastSelectedShips.empty() && (_lastSelectedShips[i] != selectState)){
			selectionChanged = true;
		}

	}

	// If we are changing the status of multiple items, apply to all.
	if (checksChanged && !selectionChanged) {
		// go through the selected cells and check/uncheck the ones in the first column.
		for (size_t i = 0; i < newEnabledStatus.size(); ++i) {
			ui->shipVarList->item(static_cast<int>(i),0)->setCheckState((newEnabledStatus[i]) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		}

	// undo check changes if the selection changed to cut down on accidental clicks.
	} else if (checksChanged && selectionChanged) {
		for (size_t i = 0; i < _lastEnabledShips.size(); ++i) {
			ui->shipVarList->item(static_cast<int>(i),0)->setCheckState((_lastEnabledShips[i]) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		}
	}

	// always save the new status.
	_lastEnabledShips = std::move(newEnabledStatus);
	_lastSelectedShips = std::move(newSelectedStatus);

	sendEditedShips(); 
}

void LoadoutDialog::onWeaponListEdited()
{
	SCP_vector<bool> newEnabledStatus;
	bool newStatus, checksChanged = false;

	// we need the index anyway, and more efficient to iterate through just the first column.
	for (int i = 0; i < ui->weaponVarList->rowCount(); i++) {
		bool checkState = (ui->weaponVarList->item(i,0)->checkState() == Qt::Checked);
		newEnabledStatus.push_back(checkState);

		// compare the old to the new to see if there was a change in check marks.
		// this would break if the amount of ships or weapons ever became dynamic
		if (!checksChanged && !_lastEnabledWeapons.empty() && (_lastEnabledWeapons[i] != newEnabledStatus[i])) {
			newStatus = newEnabledStatus[i];
			checksChanged = true;
		}
		else if (_lastEnabledWeapons.empty()) {
			newStatus = newEnabledStatus[i];
			checksChanged = true;
		}
	}

	if (checksChanged) {
		// go through the selected cells and check/uncheck the ones in the first column.
		for (auto& item : ui->weaponVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState((newStatus) ? Qt::Checked : Qt::Unchecked);
			}
		}

		// redo the _lastEnabledWeapons vector to reflect the change.
		_lastEnabledWeapons.clear();
		for (int i = 0; i < ui->weaponVarList->rowCount(); i++) {
			_lastEnabledWeapons.push_back(ui->weaponVarList->item(i, 0)->checkState() == Qt::Checked);
		}
	} // if we ended up here, basically something was probably selected or unselected, but there's a chance that it is an incompatible selection.
	// because if just one is enabled or disabled, then there is no way to automatically reconcile the info.
	// IOW, I can't automatically check it for you, FREDer
	else { 
		_lastEnabledWeapons = newEnabledStatus;

		bool newSelectedCheckStatus = (ui->weaponVarList->currentItem()->checkState() == Qt::Checked);

		for (auto& item : ui->weaponVarList->selectedItems()) {
			if (item->column() == 0) {
				bool oldSelectedCheckStatus = (item->checkState() == Qt::Checked);

				// so the newly selected item does not match the rest, and is incompatible. So clear our old selection.
				if (newSelectedCheckStatus != oldSelectedCheckStatus) {
					ui->weaponVarList->clearSelection();
					// and reselect our new one!
					ui->weaponVarList->selectRow(ui->weaponVarList->currentItem()->row());
					// nothing actually changed, so return!
					break;
				}
			}
		}
	}

	sendEditedWeapons();
}

void LoadoutDialog::onExtraShipSpinboxUpdated()
{
	// if there are no extra allocated here, we need to uncheck the selected items.
	if (ui->extraShipSpinbox->value() <= 0) {
		ui->extraShipSpinbox->setValue(0);
		if (ui->extraShipsViaVarCombo->currentText().isEmpty()) {
			for (auto& item : ui->shipVarList->selectedItems()) {
				if (item->column() == 0) {
					item->setCheckState(Qt::Unchecked);
				}
			}
		}
	}
	else {
		// if we are using the spinbox, the variable needs to be disabled. And everything needs to be enabled
		ui->extraShipsViaVarCombo->setCurrentText("");
		for (auto& item : ui->shipVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Checked);
			}
		}
	}

	sendEditedShips();
}

void LoadoutDialog::onExtraWeaponSpinboxUpdated()
{
	// if there are no extra allocated here, we need to uncheck the selected items.
	if (ui->extraWepSpinbox->value() <= 0) {
		ui->extraWepSpinbox->setValue(0);
		if (ui->extraWeaponsViaVarCombo->currentText().isEmpty()) {
			for (auto& item : ui->weaponVarList->selectedItems()) {
				if (item->column() == 0) {
					item->setCheckState(Qt::Unchecked);
				}
			}
		}
	}
	else {
		// if we are using the spinbox, the variable needs to be disabled. And checks needs to be enabled
		ui->extraWeaponsViaVarCombo->setCurrentText("");
		for (auto& item : ui->weaponVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Checked);
			}
		}
	}

	sendEditedWeapons();
}

void LoadoutDialog::onExtraShipComboboxUpdated()
{
	// if the variable is replacing the amount, get rid of the amount in the spinbox.
	if (!ui->extraWeaponsViaVarCombo->currentText().isEmpty() && ui->extraWepSpinbox->value() > 0) {
		ui->extraWepSpinbox->setValue(0);
	} // if there are no ships allocated, uncheck the ship
	else if (ui->extraWeaponsViaVarCombo->currentText().isEmpty() && ui->extraWepSpinbox->value() == 0) {
		for (auto& item : ui->weaponVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Unchecked);
			}
		}
	} // if we just picked a variable, check mark the ship
	else if (!ui->extraWeaponsViaVarCombo->currentText().isEmpty()) {
		for (auto& item : ui->weaponVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Checked);
			}
		}
	}

	sendEditedShips();
}

void LoadoutDialog::onExtraWeaponComboboxUpdated()
{

	// if the variable is replacing the amount, get rid of the amount in the spinbox.
	if (!ui->extraShipsViaVarCombo->currentText().isEmpty() && ui->extraShipSpinbox->value() > 0) {
		ui->extraShipSpinbox->setValue(0);
	} // if there are no ships allocated, uncheck the ship
	else if (ui->extraShipsViaVarCombo->currentText().isEmpty() && ui->extraShipSpinbox->value() == 0) {
		for (auto& item : ui->shipVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Unchecked);
			}
		}
	} // if we just picked a variable, check mark the ship
	else if (!ui->extraShipsViaVarCombo->currentText().isEmpty()) {
		for (auto& item : ui->shipVarList->selectedItems()) {
			if (item->column() == 0) {
				item->setCheckState(Qt::Checked);
			}
		}
	}

	sendEditedWeapons();
}

void LoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated()
{
	if (ui->playerDelayDoubleSpinbox->value() < 0) {
		ui->playerDelayDoubleSpinbox->setValue(0.0f);
	}

	_model->setPlayerEntryDelay(static_cast<float>(ui->playerDelayDoubleSpinbox->value()));
}

void LoadoutDialog::onCurrentTeamSpinboxUpdated()
{
	_model->switchTeam(ui->currentTeamSpinbox->value());
}

void LoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed()
{
	_model->copyToOtherTeam();
}

void LoadoutDialog::sendEditedShips()
{
	SCP_vector<SCP_string> namesOut;
	bool enabled = false;

	for (auto& item : ui->shipVarList->selectedItems()) {
		if (item->column() == 0) {
			enabled = (item->checkState() == Qt::Checked);
		}
		else if (item->column() == 1) {
			namesOut.push_back(item->text().toStdString());
		}
			
	}

	if (_mode == TABLE_MODE) {
		// why did I do it this way?  I don't know. Sorry. I wrote the model first.
		for (auto& nameOut : namesOut){
			_model->setShipInfo(nameOut, enabled, ui->extraShipSpinbox->value(), ui->extraShipsViaVarCombo->currentText().toStdString());
		}
	}
	else {
		_model->setShipEnablerVariables(namesOut, enabled, ui->extraShipSpinbox->value(), ui->extraShipsViaVarCombo->currentText().toStdString());
	}

	updateUI(); // Better to call it here, than over and over with a modelChanged
}

void LoadoutDialog::sendEditedWeapons()
{
	SCP_vector<SCP_string> namesOut;
	bool enabled = false;

	for (auto& item : ui->weaponVarList->selectedItems()) {
		if (item->column() == 0) {
			enabled = (item->checkState() == Qt::Checked);
		} else if (item->column() == 1){
			namesOut.push_back(ui->weaponVarList->itemAt(item->row(), 0)->text().toStdString());
		}

	}

	if (_mode == TABLE_MODE) {
		// why did I do it this way?  Bad assumption, sorry. I wrote the model first.
		for (auto& nameOut : namesOut){
			_model->setWeaponInfo(nameOut, enabled, ui->extraWepSpinbox->value(), ui->extraWeaponsViaVarCombo->currentText().toStdString());
		}
	}
	else {
		_model->setWeaponEnablerVariables(namesOut, enabled, ui->extraWepSpinbox->value(), ui->extraWeaponsViaVarCombo->currentText().toStdString());
	}

	updateUI(); // Better to call it here, than over and over with a modelChanged
}

void LoadoutDialog::updateUI()
{
	SCP_vector<std::pair<SCP_string, bool>> newShipList;
	SCP_vector<std::pair<SCP_string, bool>> newWeaponList;

	// repopulate with the correct lists from the model.
	if (_mode == TABLE_MODE) {
		newShipList = _model->getShipList();
		newWeaponList = _model->getWeaponList();
	}
	else {
		newShipList = _model->getShipEnablerVariables();
		newWeaponList = _model->getWeaponEnablerVariables();
	}

	int currentRow = 0;


	// build the ship list...
	for (auto& newShip : newShipList) {
		// need to split the incoming string into the different parts.
		size_t divider = newShip.first.find_last_of(" ");

		// Overwrite the old number text.
		ui->shipVarList->item(currentRow, 2)->setText(newShip.first.substr(divider + 1).c_str());

		// enable the check box, if necessary - UPDATE!  I am not sure this is actually a good idea.
		// This implies that the model is always right, even when something has just been checked.
		(newShip.second) ? ui->shipVarList->item(currentRow, 0)->setCheckState(Qt::Checked) : ui->shipVarList->item(currentRow, 0)->setCheckState(Qt::Unchecked);

		currentRow++;
	}

	currentRow = 0;

	for (auto& newWeapon : newWeaponList) {
		// need to split the incoming string into the different parts.
		size_t divider = newWeapon.first.find_last_of(" ");

		// Overwrite the old number text.
		ui->weaponVarList->item(currentRow, 2)->setText(newWeapon.first.substr(divider + 1).c_str());


		// enable the check box, if necessary - UPDATE!  I am not sure this is actually a good idea.
		// This implies that the model is always right, even when something has just been checked.
		(newWeapon.second) ? ui->weaponVarList->item(currentRow, 0)->setCheckState(Qt::Checked) : ui->weaponVarList->item(currentRow, 0)->setCheckState(Qt::Unchecked);

		currentRow++;
	}

	SCP_vector<SCP_string> namesOut;
	// get all selected ship items to send to the model
	for (auto& item : ui->shipVarList->selectedItems()) {
		namesOut.push_back(ui->shipVarList->itemAt(item->row(), 1)->text().toStdString());
	}

	int temp;
	bool spinboxUpdate = _model->spinBoxUpdateRequired();

	// request info for combo and extra ship box.
	if (_mode == TABLE_MODE && (spinboxUpdate || !ui->extraShipSpinbox->isEnabled())) {
		ui->extraShipsViaVarCombo->setCurrentText(_model->getCountVarShips(namesOut).c_str()); // TODO in the future, loop through current choices.

		temp = _model->getExtraAllocatedShips(namesOut);

		if (temp > -1){
			ui->extraShipSpinbox->setValue(temp);
		}
		else {
			ui->extraShipSpinbox->clear();
		}
	}
	else if (spinboxUpdate || !ui->extraShipSpinbox->isEnabled()){
		ui->extraShipsViaVarCombo->setCurrentText(_model->getCountVarShipEnabler(namesOut).c_str());

		temp = _model->getExtraAllocatedShipEnabler(namesOut);

		if (temp > -1) {
			ui->extraShipSpinbox->setValue(temp);
		}
		else {
			ui->extraShipSpinbox->clear();
		}

	}

	namesOut.clear();

	for (auto& item : ui->weaponVarList->selectedItems()) {
		namesOut.push_back(ui->weaponVarList->itemAt(item->row(), 0)->text().toStdString());
	}

	if (_mode == TABLE_MODE && (spinboxUpdate || !ui->extraWepSpinbox->isEnabled())) {
		ui->extraWeaponsViaVarCombo->setCurrentText(_model->getCountVarWeapons(namesOut).c_str()); // TODO in the future, loop through current choices.
		ui->extraWepSpinbox->setValue(_model->getExtraAllocatedWeapons(namesOut));

		temp = _model->getExtraAllocatedWeapons(namesOut);

		if (temp > -1) {
			ui->extraWepSpinbox->setValue(temp);
		} 
		else {
			ui->extraWepSpinbox->clear();
		}
	}
	else if (spinboxUpdate || !ui->extraWepSpinbox->isEnabled()) {
		ui->extraWeaponsViaVarCombo->setCurrentText(_model->getCountVarWeaponEnabler(namesOut).c_str());
		ui->extraWepSpinbox->setValue(_model->getExtraAllocatedWeaponEnabler(namesOut));

		if (temp > -1) {
			ui->extraWepSpinbox->setValue(temp);
		} 
		else {
			ui->extraWepSpinbox->clear();
		}
	}

	if (ui->shipVarList->selectedItems().isEmpty()) {
		ui->extraShipsViaVarCombo->setEnabled(false);
		ui->extraShipSpinbox->setEnabled(false);
	}
	else {
		ui->extraShipsViaVarCombo->setEnabled(true);
		ui->extraShipSpinbox->setEnabled(true);
	}

	if (ui->weaponVarList->selectedItems().isEmpty()) {
		ui->extraWeaponsViaVarCombo->setEnabled(false);
		ui->extraWepSpinbox->setEnabled(false);
	}
	else {
		ui->extraWeaponsViaVarCombo->setEnabled(true);
		ui->extraWepSpinbox->setEnabled(true);
	}

}

void LoadoutDialog::resetLists() {
	// clear the lists
	ui->shipVarList->clearContents();
	ui->weaponVarList->clearContents();

	SCP_vector<std::pair<SCP_string, bool>> newShipList;
	SCP_vector<std::pair<SCP_string, bool>> newWeaponList;

	// repopulate with the correct lists from the model.
	if (_mode == TABLE_MODE) {
		newShipList = _model->getShipList();
		newWeaponList = _model->getWeaponList();
	}
	else {
		newShipList = _model->getShipEnablerVariables();
		newWeaponList = _model->getWeaponEnablerVariables();
	}

	ui->shipVarList->setRowCount(static_cast<int>(newShipList.size()));
	ui->weaponVarList->setRowCount(static_cast<int>(newWeaponList.size()));

	int currentRow = 0;

	// build the ship list...
	for (auto& newShip : newShipList) {
		// need to split the incoming string into the different parts.
		size_t divider = newShip.first.find_last_of(" ");

		// add text to the items
		QTableWidgetItem* checkItem = new QTableWidgetItem();
		QTableWidgetItem* nameItem = new QTableWidgetItem(newShip.first.substr(0, divider).c_str());
		QTableWidgetItem* countItem = new QTableWidgetItem(newShip.first.substr(divider + 2).c_str());

		checkItem->setFlags(checkItem->flags() & ~Qt::ItemIsEditable);
		nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
		countItem->setFlags(countItem->flags() &  ~Qt::ItemIsEditable);

		// enable the check box, if necessary
		(newShip.second) ? checkItem->setCheckState(Qt::Checked) : checkItem->setCheckState(Qt::Unchecked);

		// overwrite the entry in the table.
		ui->shipVarList->setItem(currentRow, 0, checkItem);
		ui->shipVarList->setItem(currentRow, 1, nameItem);
		ui->shipVarList->setItem(currentRow, 2, countItem);

		currentRow++;
	}

	currentRow = 0;

	for (auto& newWeapon : newWeaponList) {
		// need to split the incoming string into the different parts.
		size_t divider = newWeapon.first.find_last_of(" ");

		// add text to the items
		QTableWidgetItem* checkItem = new QTableWidgetItem();
		QTableWidgetItem* nameItem = new QTableWidgetItem(newWeapon.first.substr(0, divider).c_str());
		QTableWidgetItem* countItem = new QTableWidgetItem(newWeapon.first.substr(divider + 2).c_str());

		nameItem->setFlags(nameItem->flags() &  ~Qt::ItemIsEditable);
		countItem->setFlags(countItem->flags() &  ~Qt::ItemIsEditable);

		// enable the check box, if necessary
		(newWeapon.second) ? checkItem->setCheckState(Qt::Checked) : checkItem->setCheckState(Qt::Unchecked);

		// overwrite the entry in the table.
		ui->weaponVarList->setItem(currentRow, 0, checkItem);
		ui->weaponVarList->setItem(currentRow, 1, nameItem);
		ui->weaponVarList->setItem(currentRow, 2, countItem);

		currentRow++;
	}
}

}
}
}
