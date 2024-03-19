#include "LoadoutDialog.h"
#include "ui_LoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>

constexpr int TABLE_MODE = 0;
constexpr int VARIABLE_MODE = 1;

namespace fso {
namespace fred {
namespace dialogs {
	//ui includes:
	// tableVarLabel, switches between "Loadout Editor: Loadout View" and "Loadout Editor: Variable View"
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

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &LoadoutDialog::updateUI);
	connect(this, &QDialog::accepted, _model.get(), &LoadoutDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &LoadoutDialogModel::reject);

	// Ship and Weapon lists, selection changed.
	// TODO: Is there a way to know if we have been selected via the tab button?
	connect(ui->listShipsNotUsed, 
		&QListWidget::itemSelectionChanged,
		this,
		&LoadoutDialog::onPotentialShipListClicked);

	connect(ui->listWeaponsNotUsed,
		&QListWidget::itemSelectionChanged,
		this,
		&LoadoutDialog::onPotentialWeaponListClicked);

	connect(ui->usedShipsList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onUsedShipListClicked);

	connect(ui->usedShipsList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&LoadoutDialog::onUsedWeaponListClicked);

	// And switching views between variable and lists
	connect(ui->switchViewButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onSwitchViewButtonPressed);

	
	// Switch ships and weapons on the list
	connect(ui->addShipButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::addShipButtonClicked);

	connect(ui->addWeaponButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::addWeaponButtonClicked);

	connect(ui->removeShipButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::removeShipButtonClicked);

	connect(ui->removeWeaponButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::removeWeaponButtonClicked);

	// Change item counts
	connect(ui->extraItemSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onextraItemSpinboxUpdated);

	connect(ui->extraItemsViaVariableCombo,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&LoadoutDialog::onExtraItemsViaVariableCombo);

	// Team controls
	connect(ui->currentTeamSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&LoadoutDialog::onCurrentTeamSpinboxUpdated);

	connect(ui->copyLoadoutToOtherTeamsButton,
		&QPushButton::clicked,
		this,
		&LoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed);

	// Miscellaneous controls
	connect(ui->playerDelayDoubleSpinbox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		this,
		&LoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated);


	// things that must be set for everything to work...
	_mode = TABLE_MODE;
	_lastSelectionChanged = NONE;
	
	// UPDATE - Now that things are split, we don't need this anymore.  Keep it around for reference until TODO remove me when done
	// rows will vary but columns need to be 3
	//	ui->shipVarList->setColumnCount(1);
	//	ui->weaponVarList->setColumnCount(1);
	
	// set sizes
	//	ui->shipVarList->setColumnWidth(0,10);
	//	ui->weaponVarList->setColumnWidth(0,10);

	// set headers
	ui->usedShipsList->setColumnCount(2);
	ui->usedWeaponsList->setColumnCount(2);
	ui->usedShipsList->setHorizontalHeaderItem(1, new QTableWidgetItem("In Wings/Extra"));
	ui->usedWeaponsList->setHorizontalHeaderItem(1, new QTableWidgetItem("In Wings/Extra"));

	// quickly enable or disable the team spin box (must not get to multiple teams if in SP!)
	if (The_mission.game_type & MISSION_TYPE_MULTI){
		ui->currentTeamSpinbox->setEnabled(true);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(true);
	}
	else {
		ui->currentTeamSpinbox->setEnabled(false);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(false);
	}

	// need to completely rebuild the lists here.
//	resetLists();
	updateUI();
}

// a result of competing CI requirements
LoadoutDialog::~LoadoutDialog(){} // NOLINT

void LoadoutDialog::onSwitchViewButtonPressed()
{
	// Change important lables.
	if (_mode == TABLE_MODE) {
		ui->tableVarLabel->setText("Loadout Editor: Variable View");
		// TODO! FIXME! Some of the labels are missing from this function.  Please check QT Creator
		ui->startingShipsLabel->setText("Potential Variables - Ships");
		ui->startingWeaponsLabel->setText("Potential Variables - Weapons");
		_mode = VARIABLE_MODE;
	}
	else {
		ui->tableVarLabel->setText("Loadout Editor: Loadout View");
		// TODO! FIXME! Some of the labels are missing from this function.  Please check QT Creator
		ui->startingShipsLabel->setText("Ships Not in Loadout");
		ui->startingWeaponsLabel->setText("Starting Weapons");
		_mode = TABLE_MODE;
	}

	// model does not keep track of whether the UI is editing the table values or the vars
	// so, just reset the lists and then update the UI
//	resetLists();
	updateUI();
}

void LoadoutDialog::addShipButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->listShipsNotUsed->selectedItems()){
		list.push_back(item->text().toStdString());
	}

	_model->setShipEnabled(list, true);
	updateUI();
}

void LoadoutDialog::addWeaponButtonClicked()
{
	SCP_vector<SCP_string> list;
	
	for (const auto& item: ui->listWeaponsNotUsed->selectedItems()){
		list.push_back(item->text().toStdString());
	}

	_model->setWeaponEnabled(list, true);
	updateUI();
}

void LoadoutDialog::removeShipButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->usedShipsList->selectedItems()){
		list.push_back(item->text().toStdString());
	}

	_model->setShipEnabled(list, false);
	updateUI();
}

void LoadoutDialog::removeWeaponButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->usedWeaponsList->selectedItems()){
		list.push_back(item->text().toStdString());
	}

	_model->setWeaponEnabled(list, false);
	updateUI();
}

void LoadoutDialog::onextraItemSpinboxUpdated()
{
	SCP_vector<SCP_string> list;

	if (_lastSelectionChanged == USED_SHIPS){
		for (const auto& item : ui->usedShipsList->selectedItems()){
			list.emplace_back(item->text().toStdString());
		}

		_model->setExtraAllocatedShipCount(list, ui->extraItemSpinbox->value());
	} else if (_lastSelectionChanged == USED_WEAPONS){
		for (const auto& item : ui->usedWeaponsList->selectedItems()){
			list.emplace_back(item->text().toStdString());
		}

		if (ui->extraItemSpinbox->value() < 0){
			ui->extraItemSpinbox->setValue(0.0f);
		}
		
		_model->setExtraAllocatedWeaponCount(list, ui->extraItemSpinbox->value());
	}
}

void LoadoutDialog::onExtraItemsViaVariableCombo()
{
	// TODO!  Figure out if this actually makes sense
	// if the variable is replacing the amount, get rid of the amount in the spinbox.
	if (!ui->extraItemsViaVariableCombo->currentText().isEmpty() && ui->extraItemSpinbox->value() > 0) {
		ui->extraItemSpinbox->setValue(0);
	} 

	// TODO, make a function that updates the model with the new combobox choice

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
	updateUI();
}

void LoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed()
{
	// TODO! Add confirmation button
	_model->copyToOtherTeam();
}

void LoadoutDialog::updateUI()
{
	// repopulate with the correct lists from the model.
	SCP_vector<std::pair<SCP_string, bool>> newShipList;
	SCP_vector<std::pair<SCP_string, bool>> newWeaponList;

	if (_mode == TABLE_MODE) {

		newShipList = _model->getShipList();
		newWeaponList = _model->getWeaponList();
	}
	else {
		newShipList = _model->getShipEnablerVariables();
		newWeaponList = _model->getWeaponEnablerVariables();
	}

	// build the ship lists...
	for (const auto& newShip : newShipList) {		
		// need to split the incoming string into the different parts.
		size_t divider = newShip.first.rfind(" ");
		SCP_string shipName = newShip.first.substr(0, divider);

		if (newShip.second) {
			bool found = false;
			
			for (int x = 0; x < ui->usedShipsList->rowCount(); ++x){
				if (ui->usedShipsList->item(x,0) && !stricmp(ui->usedShipsList->item(x, 0)->text().toStdString().c_str(), shipName.c_str())) {
					found = true;
					// only need to update the quantities here.
					ui->usedShipsList->item(x, 1)->setText(newShip.first.substr(divider + 1).c_str());
					break;
				}
			}

			if (!found){
				ui->usedShipsList->insertRow(ui->usedShipsList->rowCount());

				QTableWidgetItem* nameItem = new QTableWidgetItem(shipName.c_str());
				QTableWidgetItem* countItem = new QTableWidgetItem(newShip.first.substr(divider + 1).c_str());
	
				ui->usedShipsList->setItem(ui->usedShipsList->rowCount() - 1, 0, nameItem);
				ui->usedShipsList->setItem(ui->usedShipsList->rowCount() - 1, 1, countItem);

				//remove from the unused list
				for (int x = 0; x < ui->listShipsNotUsed->count(); ++x){
					if (!stricmp(ui->listShipsNotUsed->item(x)->text().toStdString().c_str(), shipName.c_str())) {
						delete ui->listShipsNotUsed->takeItem(x);
						break;
					}
				}
			}

		} else {
			bool found = false;

			for (int x = 0; x < ui->listShipsNotUsed->count(); ++x){
				if (!stricmp(ui->listShipsNotUsed->item(x)->text().toStdString().c_str(), shipName.c_str())) {
					found = true;
					break;
				}
			}

			if (!found){
				ui->listShipsNotUsed->addItem(shipName.c_str());

				//remove from the used list
				for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
					if (!stricmp(ui->usedShipsList->item(x, 0)->text().toStdString().c_str(), shipName.c_str())) {
						delete ui->usedShipsList->takeItem(x, 0);
						delete ui->usedShipsList->takeItem(x, 1);
						break;
					}
				}
			}
		}
	}

	for (auto& newWeapon : newWeaponList) {		
		// need to split the incoming string into the different parts.
		size_t divider = newWeapon.first.rfind(" ");
		SCP_string weaponName = newWeapon.first.substr(0, divider);
		if (newWeapon.second) {
			bool found = false;
			
			// Add or update in the used list
			for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
				if (ui->usedWeaponsList->item(x,0) && !stricmp(ui->usedWeaponsList->item(x, 0)->text().toStdString().c_str(), weaponName.c_str())) {
					found = true;
					// only need to update the quantities here.
					ui->usedWeaponsList->item(x, 1)->setText(newWeapon.first.substr(divider + 1).c_str());
					break;
				}
			}

			if (!found){
				QTableWidgetItem* nameItem = new QTableWidgetItem(weaponName.c_str());
				QTableWidgetItem* countItem = new QTableWidgetItem(newWeapon.first.substr(divider + 1).c_str());

				ui->usedWeaponsList->insertRow(ui->usedWeaponsList->rowCount());
				ui->usedWeaponsList->setItem(ui->usedWeaponsList->rowCount() - 1, 0, nameItem);
				ui->usedWeaponsList->setItem(ui->usedWeaponsList->rowCount() - 1, 1, countItem);

				//remove from the unused list
				for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x){
					if (!stricmp(ui->listWeaponsNotUsed->item(x)->text().toStdString().c_str(), weaponName.c_str())) {
						delete ui->listWeaponsNotUsed->takeItem(x);
						break;
					}
				}
			}
			
		} else {
			bool found = false;

			for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x){
				if (!stricmp(ui->listWeaponsNotUsed->item(x)->text().toStdString().c_str(), weaponName.c_str())) {
					found = true;
					break;
				}
			}

			if (!found){
				ui->listWeaponsNotUsed->addItem(weaponName.c_str());

				//remove from the used list
				for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x){
					if (!stricmp(ui->usedWeaponsList->item(x, 0)->text().toStdString().c_str(), weaponName.c_str())) {
						delete ui->usedWeaponsList->takeItem(x, 0);
						delete ui->usedWeaponsList->takeItem(x, 1);
						break;
					}
				}
			}
		}
	}

	SCP_vector<SCP_string> namesOut;
	bool requestSpinComboUpdate = false;

	// Do some basic enabling and disabling
	if (_mode == TABLE_MODE && _lastSelectionChanged == USED_SHIPS) {
		ui->extraItemSpinbox->setEnabled(true);
		ui->extraItemsViaVariableCombo->setEnabled(true);

		for (const auto& item : ui->usedShipsList->selectedItems()) {
			namesOut.emplace_back(item->text().toStdString().c_str());
			requestSpinComboUpdate = true;
		}
	} else if (_mode == TABLE_MODE && _lastSelectionChanged == USED_WEAPONS){
		ui->extraItemSpinbox->setEnabled(true);
		ui->extraItemsViaVariableCombo->setEnabled(true);

		for (const auto& item : ui->usedWeaponsList->selectedItems()){
			namesOut.emplace_back(item->text().toStdString().c_str());
			requestSpinComboUpdate = true;
		}
	//} else if (mode == VARIABLE_MODE) { TODO FINISH ME!

	} else {
		ui->extraItemSpinbox->setEnabled(false);
		ui->extraItemsViaVariableCombo->setEnabled(false);
	}

	if (requestSpinComboUpdate || _model->spinBoxUpdateRequired()) {
		ui->extraItemsViaVariableCombo->setCurrentText(_model->getCountVarShips(namesOut).c_str());
		
		int temp;

		if (_lastSelectionChanged == USED_SHIPS){
			temp = _model->getExtraAllocatedShipEnabler(namesOut);		
		} else {
			temp = _model->getExtraAllocatedWeapons(namesOut);
		}

		if (temp > -1) {
			ui->extraItemSpinbox->setValue(temp);
		}
		else {
			ui->extraItemSpinbox->clear();
		}
	}


}

// This has been deprecated for now.  Possibly only going to use UpdateUI, since it will add and remove items from the lists as needed.
/*
void LoadoutDialog::resetLists() {
	// clear the lists
	ui->usedShipsList->clearContents();
	ui->usedWeaponsList->clearContents();
	ui->listShipsNotUsed->clearContents();
	ui->listWeaponsNotUsed->clearContents();

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

	ui->usedShipsList->setRowCount(static_cast<int>(newShipList.size()));
	ui->usedWeaponsList->setRowCount(static_cast<int>(newWeaponList.size()));

	int currentRow = 0;

	// build the ship list...
	for (auto& newShip : newShipList) {
		// need to split the incoming string into the different parts.
		size_t divider = newShip.first.find_last_of(" ");

		// add text to the items
		QTableWidgetItem nameItem(newShip.first.substr(0, divider).c_str());
		QTableWidgetItem countItem(newShip.first.substr(divider + 1).c_str());

		// TODO, figure out how to make the entire list editable or not editable.
		nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
		countItem->setFlags(countItem->flags() &  ~Qt::ItemIsEditable);

		// enable the check box, if necessary
		if (newShip.second) {
			// overwrite the entry in the table.
			// TODO, check this syntax
			ui->usedShipList->insertItem(ui->usedShipList.size() -1 , 0, nameItem);
			ui->usedShipList->setItem(ui->usedShipList.size() - 1, 1, countItem);
		} //else {
			//ui->
		//}

		currentRow++;
	}

	currentRow = 0;

	for (auto& newWeapon : newWeaponList) {
		// need to split the incoming string into the different parts.
		size_t divider = newWeapon.first.find_last_of(" ");

		// add text to the items
		QTableWidgetItem nameItem(newWeapon.first.substr(0, divider).c_str());
		QTableWidgetItem countItem(newWeapon.first.substr(divider + 2).c_str());

		nameItem->setFlags(nameItem->flags() &  ~Qt::ItemIsEditable);
		countItem->setFlags(countItem->flags() &  ~Qt::ItemIsEditable);

		// enable the check box, if necessary
		(newWeapon.second) ? checkItem->setCheckState(Qt::Checked) : checkItem->setCheckState(Qt::Unchecked);

		// overwrite the entry in the table.
		ui->weaponVarList->setItem(currentRow, 1, nameItem);
		ui->weaponVarList->setItem(currentRow, 2, countItem);

		currentRow++;
	}
}


void LoadoutDialog::getCurrentSelection() {

}
*/

}
}
}
