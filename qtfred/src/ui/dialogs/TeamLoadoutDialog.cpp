#include "TeamLoadoutDialog.h"
#include "ui_TeamLoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>
#include <QMessageBox>
#include <mission/util.h>

constexpr int TABLE_MODE = 0;
constexpr int VARIABLE_MODE = 1;

namespace fso::fred::dialogs {

TeamLoadoutDialog::TeamLoadoutDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::TeamLoadoutDialog()), _model(new TeamLoadoutDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &TeamLoadoutDialog::updateUi);

	// Ship and Weapon lists, selection changed.
	connect(ui->listShipsNotUsed, 
		&QListWidget::itemClicked, 
		this, 
		&TeamLoadoutDialog::onPotentialShipListClicked);

	// We need a second trigger for each list when multiple items are selected because itemClicked doesn't trigger on
	// those, and itemSelectionChanged doesn't really trigger if you select the same item that's already selected (I
	// think) so keep both
	connect(ui->listShipsNotUsed, 
		&QListWidget::itemSelectionChanged,
		this, 
		&TeamLoadoutDialog::onPotentialShipListClicked);

	connect(ui->listWeaponsNotUsed,
		&QListWidget::itemClicked,
		this, 
		&TeamLoadoutDialog::onPotentialWeaponListClicked);

	connect(ui->listWeaponsNotUsed,
		&QListWidget::itemSelectionChanged,
		this,
		&TeamLoadoutDialog::onPotentialWeaponListClicked);

	connect(ui->usedShipsList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&TeamLoadoutDialog::onUsedShipListClicked);

	connect(ui->usedShipsList, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&TeamLoadoutDialog::onUsedShipListClicked);

	connect(ui->usedWeaponsList,
		static_cast<void (QTableWidget::*)(QTableWidgetItem*)>(&QTableWidget::itemClicked),
		this,
		&TeamLoadoutDialog::onUsedWeaponListClicked);

	connect(ui->usedWeaponsList, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&TeamLoadoutDialog::onUsedWeaponListClicked);

	// Selection convenience buttons
	connect(ui->selectAllUnusedShipsButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::onSelectAllUnusedShipsPressed);

	connect(ui->clearUnusedShipSelectionButton,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onClearAllUnusedShipsPressed);

	connect(ui->selectAllUnusedWeaponsButton,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onSelectAllUnusedWeaponsPressed);

	connect(ui->clearUnusedWeaponSelectionButton,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onClearAllUnusedWeaponsPressed);

	connect(ui->selectAllUsedShipsButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::onSelectAllUsedShipsPressed);

	connect(ui->clearUsedShipSelectionButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::onClearAllUsedShipsPressed);

	connect(ui->selectAllUsedWeaponsButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::onSelectAllUsedWeaponsPressed);

	connect(ui->clearUsedWeaponSelectionButton,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onClearAllUsedWeaponsPressed);

	// And switching views between variable and lists
	connect(ui->switchViewButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::onSwitchViewButtonPressed);

	// Switch ships and weapons on the list
	connect(ui->addShipButton, 
		&QPushButton::clicked,
		this, 
		&TeamLoadoutDialog::addShipButtonClicked);

	connect(ui->addWeaponButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::addWeaponButtonClicked);

	connect(ui->removeShipButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::removeShipButtonClicked);

	connect(ui->removeWeaponButton, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::removeWeaponButtonClicked);

	// Change item counts
	connect(ui->extraItemSpinbox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&TeamLoadoutDialog::onExtraItemSpinboxUpdated);

	connect(ui->extraItemsViaVariableCombo,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&TeamLoadoutDialog::onExtraItemsViaVariableCombo);

	// Team controls
	connect(ui->copyLoadoutToOtherTeamsButton,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed);

	// Miscellaneous controls
	connect(ui->playerDelayDoubleSpinbox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		this,
		&TeamLoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated);

	connect(ui->editVariables, 
		&QPushButton::clicked, 
		this, 
		&TeamLoadoutDialog::openEditVariablePressed);

	connect(ui->requiredWeaponCheckBox,
		&QPushButton::clicked,
		this,
		&TeamLoadoutDialog::onSelectionRequiredCheckbox);

	connect(ui->weaponValidationCheckbox,
		&QCheckBox::clicked,
		this,
		&TeamLoadoutDialog::onWeaponValidationCheckboxClicked);

	// things that must be set for everything to work...
	_mode = TABLE_MODE;
	_lastSelectionChanged = NONE;

	// set headers
	ui->usedShipsList->setColumnCount(2);
	ui->usedWeaponsList->setColumnCount(3);
	ui->usedShipsList->setHorizontalHeaderItem(0, new QTableWidgetItem("Ship Name"));
	ui->usedShipsList->setHorizontalHeaderItem(1, new QTableWidgetItem("In Wings/Extra"));
	ui->usedWeaponsList->setHorizontalHeaderItem(0, new QTableWidgetItem("Weapon Name"));
	ui->usedWeaponsList->setHorizontalHeaderItem(1, new QTableWidgetItem("In Wings/Extra"));
	ui->usedWeaponsList->setHorizontalHeaderItem(2, new QTableWidgetItem("Required"));

	ui->usedShipsList->setColumnWidth(0, 220);
	ui->usedShipsList->setColumnWidth(0, 175);
	ui->usedWeaponsList->setColumnWidth(0, 130);
	ui->usedWeaponsList->setColumnWidth(1, 122);
	ui->usedWeaponsList->setColumnWidth(2, 65);

	// Populate the variable combobox
	ui->extraItemsViaVariableCombo->clear();
	ui->extraItemsViaVariableCombo->addItem("");
	ui->extraItemsViaVariableCombo->addItem("<none>");

	SCP_vector<SCP_string> numberVarList = _model->getNumberVarList();

	for (const auto& item : numberVarList) {
		ui->extraItemsViaVariableCombo->addItem(item.c_str());
	}

	// quickly enable or disable the team spin box (must not get to multiple teams if in SP!)
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		ui->currentTeamComboBox->setEnabled(true); // TODO make an enable/disable function for all the controls
		ui->copyLoadoutToOtherTeamsButton->setEnabled(true);
	} else {
		ui->currentTeamComboBox->setEnabled(false);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(false);
	}

	auto delay = _model->getPlayerEntryDelay();
	if (delay >= 0.0f) {
		ui->playerDelayDoubleSpinbox->setValue(static_cast<double>(delay));
	} else {
		ui->playerDelayDoubleSpinbox->setValue(0.0f);
	}

	ui->weaponValidationCheckbox->setChecked(_model->getSkipValidation());

	updateUi();
	initializeUi();
}

TeamLoadoutDialog::~TeamLoadoutDialog() = default;

void TeamLoadoutDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void TeamLoadoutDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void TeamLoadoutDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void TeamLoadoutDialog::rejectHandler()
{
	this->close();
}

void TeamLoadoutDialog::onSwitchViewButtonPressed()
{
	// Change important lables.
	if (_mode == TABLE_MODE) {
		ui->tableVarLabel->setText("Variable View");
		
		ui->listShipsNotUsedLabel->setText("Potential Variables To Enable Ships");
		ui->listWeaponsNotUsedLabel->setText("Potential Variables To Enable Weapons");
		ui->startingShipsLabel->setText("Variables Used for Enabling Ships");
		ui->startingWeaponsLabel->setText("Variables Used for Enabling Weapons");

		_mode = VARIABLE_MODE;
	}
	else {
		ui->tableVarLabel->setText("Loadout View");
		ui->listShipsNotUsedLabel->setText("Ships Not in Loadout");
		ui->listWeaponsNotUsedLabel->setText("Weapons Not In Loadout");
		ui->startingShipsLabel->setText("Ships in Loadout");
		ui->startingWeaponsLabel->setText("Weapons in Loadout");

		_mode = TABLE_MODE;
	}

	for (int x = 0; x < ui->listShipsNotUsed->count(); ++x) {
		if (ui->listShipsNotUsed->item(x)) {
			ui->listShipsNotUsed->item(x)->setText("");
		}
	}

	for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x) {
		if (ui->listWeaponsNotUsed->item(x)) {
			ui->listWeaponsNotUsed->item(x)->setText("");
		}
	}

	for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
		if (ui->usedShipsList->item(x, 0)) {
			ui->usedShipsList->item(x, 0)->setText("");
		}
		if (ui->usedShipsList->item(x, 1)) {
			ui->usedShipsList->item(x, 1)->setText("");
		}
	}

	for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
		if (ui->usedWeaponsList->item(x, 0)) {
			ui->usedWeaponsList->item(x, 0)->setText("");
		}
		if (ui->usedWeaponsList->item(x, 1)) {
			ui->usedWeaponsList->item(x, 1)->setText("");
		}
	}

	// Since we're changing modes, clear what we selected previously.
	ui->listShipsNotUsed->clearSelection();
	ui->listWeaponsNotUsed->clearSelection();
	ui->usedShipsList->clearSelection();
	ui->usedWeaponsList->clearSelection();

	// model does not keep track of whether the UI is editing the table values or the vars
	// so, just update the UI
	updateUi();
}

void TeamLoadoutDialog::addShipButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->listShipsNotUsed->selectedItems()){
		SCP_string shipName = item->text().toUtf8().constData();
		list.emplace_back(shipName);
	}

	if (_mode == TABLE_MODE) {
		_model->setShipEnabled(list, true);	
	} else {
		_model->setShipVariableEnabled(list, true);
	}

	updateUi();
}

void TeamLoadoutDialog::addWeaponButtonClicked()
{
	SCP_vector<SCP_string> list;
	
	for (const auto& item: ui->listWeaponsNotUsed->selectedItems()){
		SCP_string weaponName = item->text().toUtf8().constData();
		list.emplace_back(weaponName);
	}

	if (_mode == TABLE_MODE) {
		_model->setWeaponEnabled(list, true);
	} else {
		_model->setWeaponVariableEnabled(list, true);
	}

	updateUi();
}

void TeamLoadoutDialog::removeShipButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->usedShipsList->selectedItems()){
		SCP_string shipName = item->text().toUtf8().constData();
		list.emplace_back(shipName);
	}

	if (_mode == TABLE_MODE) {
		_model->setShipEnabled(list, false);
	} else {
		_model->setShipVariableEnabled(list, false);
	}

	updateUi();
}

void TeamLoadoutDialog::removeWeaponButtonClicked()
{
	SCP_vector<SCP_string> list;

	for (const auto& item : ui->usedWeaponsList->selectedItems()){
		SCP_string weaponName = item->text().toUtf8().constData();
		list.emplace_back(weaponName);
	}

	if (_mode == TABLE_MODE) {
		_model->setWeaponEnabled(list, false);	
	} else {
		_model->setWeaponVariableEnabled(list, false);
	}

	updateUi();
}

void TeamLoadoutDialog::onExtraItemSpinboxUpdated()
{
	SCP_vector<SCP_string> list;

	if (_mode == TABLE_MODE) {
		if (_lastSelectionChanged == USED_SHIPS) {
			list = getSelectedShips();

			if (ui->extraItemSpinbox->value() < 0) {
				ui->extraItemSpinbox->setValue(0.0f);
			}

			_model->setExtraAllocatedShipCount(list, ui->extraItemSpinbox->value());
		} else if (_lastSelectionChanged == USED_WEAPONS) {
			list = getSelectedWeapons();

			if (ui->extraItemSpinbox->value() < 0) {
				ui->extraItemSpinbox->setValue(0.0f);
			}

			_model->setExtraAllocatedWeaponCount(list, ui->extraItemSpinbox->value());
		}
	} else {
		if (_lastSelectionChanged == USED_SHIPS) {
			list = getSelectedShips();

			if (ui->extraItemSpinbox->value() < 0) {
				ui->extraItemSpinbox->setValue(0.0f);
			}

			_model->setExtraAllocatedForShipVariablesCount(list, ui->extraItemSpinbox->value());
		} else if (_lastSelectionChanged == USED_WEAPONS) {
			list = getSelectedWeapons();

			if (ui->extraItemSpinbox->value() < 0) {
				ui->extraItemSpinbox->setValue(0.0f);
			}

			_model->setExtraAllocatedForWeaponVariablesCount(list, ui->extraItemSpinbox->value());
		}
	}

	updateUi();
}

void TeamLoadoutDialog::onExtraItemsViaVariableCombo()
{
	if (_lastSelectionChanged == POTENTIAL_SHIPS || _lastSelectionChanged == POTENTIAL_WEAPONS || _lastSelectionChanged == NONE) {
		return;
	}

	SCP_vector<SCP_string> list = (_lastSelectionChanged == USED_SHIPS) ? getSelectedShips() : getSelectedWeapons();
	SCP_string chosenVariable = ui->extraItemsViaVariableCombo->currentText().toUtf8().constData();

	_model->setExtraAllocatedViaVariable(list, chosenVariable, _lastSelectionChanged == USED_SHIPS, _mode == VARIABLE_MODE);
	updateUi();
}

void TeamLoadoutDialog::onPlayerDelayDoubleSpinBoxUpdated()
{
	if (ui->playerDelayDoubleSpinbox->value() < 0) {
		ui->playerDelayDoubleSpinbox->setValue(0.0f);
	}

	_model->setPlayerEntryDelay(static_cast<float>(ui->playerDelayDoubleSpinbox->value()));
}

void TeamLoadoutDialog::onCopyLoadoutToOtherTeamsButtonPressed()
{
	QMessageBox msgBox;
	msgBox.setText("Are you sure that you want to overwrite the other team's loadout with this team's loadout?");
	msgBox.setInformativeText("This can't be undone.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();

switch (ret) {
	case QMessageBox::Yes:
		_model->copyToOtherTeam();
		break;
	case QMessageBox::Cancel:
		break;
	default:
		UNREACHABLE("Bad return value from confirmation message box in the Loadout dialog editor.");
		break;
	}
}


void TeamLoadoutDialog::onSelectAllUnusedShipsPressed() 
{
	ui->listShipsNotUsed->selectAll();
	_lastSelectionChanged = POTENTIAL_SHIPS;
}

void TeamLoadoutDialog::onClearAllUnusedShipsPressed() 
{
	ui->listShipsNotUsed->clearSelection();
	_lastSelectionChanged = POTENTIAL_SHIPS;
}

void TeamLoadoutDialog::onSelectAllUnusedWeaponsPressed()
{
	ui->listWeaponsNotUsed->selectAll();
	_lastSelectionChanged = POTENTIAL_WEAPONS;
}

void TeamLoadoutDialog::onClearAllUnusedWeaponsPressed() 
{
	ui->listWeaponsNotUsed->clearSelection();
	_lastSelectionChanged = POTENTIAL_WEAPONS;
}

void TeamLoadoutDialog::onSelectAllUsedShipsPressed() 
{
	ui->usedShipsList->selectAll();
	_lastSelectionChanged = USED_SHIPS;
}

void TeamLoadoutDialog::onClearAllUsedShipsPressed() 
{
	ui->usedShipsList->clearSelection();
	_lastSelectionChanged = USED_SHIPS;
}

void TeamLoadoutDialog::onSelectAllUsedWeaponsPressed() 
{
	ui->usedWeaponsList->selectAll();
	_lastSelectionChanged = USED_WEAPONS;
}

void TeamLoadoutDialog::onClearAllUsedWeaponsPressed() 
{
	ui->usedWeaponsList->clearSelection();
	_lastSelectionChanged = USED_WEAPONS;
}

void TeamLoadoutDialog::openEditVariablePressed() 
{
	reinterpret_cast<FredView*>(parent())->on_actionVariables_triggered(true);
}

void TeamLoadoutDialog::onSelectionRequiredCheckbox()
{
	if (ui->weaponValidationCheckbox->isChecked()) {
		onSelectionRequiredPressed();
	} else {
		onSelectionNotRequiredPressed();
	}
}

void TeamLoadoutDialog::onSelectionRequiredPressed() 
{
	SCP_vector<SCP_string> namesOut = getSelectedWeapons();
	_model->setRequiredWeapon(namesOut, true);
	updateUi();
}

void TeamLoadoutDialog::onSelectionNotRequiredPressed() 
{
	SCP_vector<SCP_string> namesOut = getSelectedWeapons();
	_model->setRequiredWeapon(namesOut, false);
	updateUi();
}

void TeamLoadoutDialog::onWeaponValidationCheckboxClicked() 
{
	_model->setSkipValidation(ui->weaponValidationCheckbox->isChecked());
}


void TeamLoadoutDialog::updateUi()
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
				SCP_string usedShipName = ui->usedShipsList->item(x, 0)->text().toUtf8().constData();
				if (ui->usedShipsList->item(x,0) && lcase_equal(usedShipName, shipName)) {
					found = true;
					// update the quantities here, and make sure it's visible
					ui->usedShipsList->item(x, 1)->setText(newShip.first.substr(divider + 1).c_str());
					ui->usedShipsList->setRowHidden(x, false);
					break;
				}
			}

			// This should only happen at init
			if (!found){
				ui->usedShipsList->insertRow(ui->usedShipsList->rowCount());

				QTableWidgetItem* nameItem = new QTableWidgetItem(shipName.c_str());
				QTableWidgetItem* countItem = new QTableWidgetItem(newShip.first.substr(divider + 1).c_str());

				ui->usedShipsList->setItem(ui->usedShipsList->rowCount() - 1, 0, nameItem);
				ui->usedShipsList->setItem(ui->usedShipsList->rowCount() - 1, 1, countItem);
			}

			// remove from the unused list
			for (int x = 0; x < ui->listShipsNotUsed->count(); ++x) {
				SCP_string usedShipName = ui->listShipsNotUsed->item(x)->text().toUtf8().constData();
				if (lcase_equal(usedShipName, shipName)) {
					ui->listShipsNotUsed->setRowHidden(x, true);
					break;
				}
			}

		} else {
			bool found = false;

			for (int x = 0; x < ui->listShipsNotUsed->count(); ++x){
				SCP_string usedShipName = ui->listShipsNotUsed->item(x)->text().toUtf8().constData();
				if (lcase_equal(usedShipName, shipName)) {
					found = true;
					ui->listShipsNotUsed->setRowHidden(x, false); 
					break;
				}
			}

			if (!found){
				ui->listShipsNotUsed->addItem(shipName.c_str());
			}

				// remove from the used list
			for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
				SCP_string usedShipName = ui->usedShipsList->item(x, 0)->text().toUtf8().constData();
				if (ui->usedShipsList->item(x, 0) &&
					lcase_equal(usedShipName, shipName)) {
					ui->usedShipsList->setRowHidden(x, true);
					break;
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
				SCP_string usedWepName = ui->usedWeaponsList->item(x, 0)->text().toUtf8().constData();
				if (ui->usedWeaponsList->item(x,0) && lcase_equal(usedWepName, weaponName)) {
					found = true;
					// only need to update the quantities here.
					ui->usedWeaponsList->item(x, 1)->setText(newWeapon.first.substr(divider + 1).c_str());
					ui->usedWeaponsList->setRowHidden(x, false);
					break;
				}
			}

			if (!found){
				ui->usedWeaponsList->insertRow(ui->usedWeaponsList->rowCount());

				QTableWidgetItem* nameItem = new QTableWidgetItem(weaponName.c_str());
				QTableWidgetItem* countItem = new QTableWidgetItem(newWeapon.first.substr(divider + 1).c_str());
				QTableWidgetItem* requiredItem = new QTableWidgetItem("");

				ui->usedWeaponsList->setItem(ui->usedWeaponsList->rowCount() - 1, 0, nameItem);
				ui->usedWeaponsList->setItem(ui->usedWeaponsList->rowCount() - 1, 1, countItem);
				ui->usedWeaponsList->setItem(ui->usedWeaponsList->rowCount() - 1, 2, requiredItem);
			}

			// remove from the unused list
			for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x) {
				SCP_string usedWepName = ui->listWeaponsNotUsed->item(x)->text().toUtf8().constData();
				if (lcase_equal(usedWepName, weaponName)) {
					ui->listWeaponsNotUsed->setRowHidden(x, true);
					break;
				}
			}

		} else {
			bool found = false;

			for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x){
				SCP_string usedWepName = ui->listWeaponsNotUsed->item(x)->text().toUtf8().constData();
				if (ui->listWeaponsNotUsed->item(x) && lcase_equal(usedWepName, weaponName)) {
					found = true;
					ui->listWeaponsNotUsed->setRowHidden(x, false);
					break;
				}
			}

			if (!found){
				ui->listWeaponsNotUsed->addItem(weaponName.c_str());
			}
			
			// remove from the used list
			for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
				SCP_string usedWepName = ui->usedWeaponsList->item(x, 0)->text().toUtf8().constData();
				if (ui->usedWeaponsList->item(x, 0) &&
					lcase_equal(usedWepName, weaponName)) {
					ui->usedWeaponsList->setRowHidden(x, true);
					break;
				}
			}
		}
	}

	
	// Go through the lists and make sure that we don't have random empty entries
	for (int x = 0; x < ui->listShipsNotUsed->count(); ++x) {
		if (ui->listShipsNotUsed->item(x) && ui->listShipsNotUsed->item(x)->text().isEmpty()) {
			for (int y = x + 1; y < ui->listShipsNotUsed->count(); ++y) {
				if (ui->listShipsNotUsed->item(y) && !ui->listShipsNotUsed->item(y)->text().isEmpty()) {
					ui->listShipsNotUsed->item(x)->setText(ui->listShipsNotUsed->item(y)->text());
					ui->listShipsNotUsed->item(y)->setText("");
					break;
				}
			}
		}
	}


	for (int x = 0; x < ui->listWeaponsNotUsed->count(); ++x) {
		if (ui->listWeaponsNotUsed->item(x) && ui->listWeaponsNotUsed->item(x)->text().isEmpty()) {
			for (int y = x + 1; y < ui->listWeaponsNotUsed->count(); ++y) {
				if (ui->listWeaponsNotUsed->item(y) && !ui->listWeaponsNotUsed->item(y)->text().isEmpty()) {
					ui->listWeaponsNotUsed->item(x)->setText(ui->listWeaponsNotUsed->item(y)->text());
					ui->listWeaponsNotUsed->item(y)->setText("");
					break;
				}
			}
		}
	}

	for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
		if (ui->usedShipsList->item(x, 0) && ui->usedShipsList->item(x, 0)->text().isEmpty() && ui->usedShipsList->item(x, 1)) {
			for (int y = x + 1; y < ui->usedShipsList->rowCount(); ++y) {
				if (ui->usedShipsList->item(y, 0) && !ui->usedShipsList->item(y, 0)->text().isEmpty()
				&& ui->usedShipsList->item(y, 1) && !ui->usedShipsList->item(y, 1)->text().isEmpty()) {

					ui->usedShipsList->item(x, 0)->setText(ui->usedShipsList->item(y, 0)->text());
					ui->usedShipsList->item(y, 0)->setText("");

					ui->usedShipsList->item(x, 1)->setText(ui->usedShipsList->item(y, 1)->text());
					ui->usedShipsList->item(y, 1)->setText("");
					break;
				}
			}
		}
	}

	for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
		if (ui->usedWeaponsList->item(x, 0) && ui->usedWeaponsList->item(x, 0)->text().isEmpty() &&
			ui->usedWeaponsList->item(x, 1)) {
			for (int y = x + 1; y < ui->usedWeaponsList->rowCount(); ++y) {
				if (ui->usedWeaponsList->item(y, 0) && !ui->usedWeaponsList->item(y, 0)->text().isEmpty()
				&& ui->usedWeaponsList->item(y, 1) && !ui->usedWeaponsList->item(y, 1)->text().isEmpty()) {

					ui->usedWeaponsList->item(x, 0)->setText(ui->usedWeaponsList->item(y, 0)->text());
					ui->usedWeaponsList->item(y, 0)->setText("");

					ui->usedWeaponsList->item(x, 1)->setText(ui->usedWeaponsList->item(y, 1)->text());
					ui->usedWeaponsList->item(y, 1)->setText("");					
					break;
				}
			}
		}
	}
	

	SCP_vector<SCP_string> namesOut;
	bool requestSpinComboUpdate = false;

	// Do some basic enabling and disabling
	if (_lastSelectionChanged == USED_SHIPS) {
		ui->extraItemSpinbox->setEnabled(true);

		namesOut = getSelectedShips();
		requestSpinComboUpdate = true;
		
	} else if (_lastSelectionChanged == USED_WEAPONS){
		ui->extraItemSpinbox->setEnabled(true);

		namesOut = getSelectedWeapons();
		requestSpinComboUpdate = true;

	} else {
		ui->extraItemSpinbox->setEnabled(false);
	}

	if (requestSpinComboUpdate || _model->spinBoxUpdateRequired()) {
		int temp;

		if (_mode == TABLE_MODE && _lastSelectionChanged == USED_SHIPS){
			temp = _model->getExtraAllocatedShips(namesOut);		
		} else if (_mode == TABLE_MODE && _lastSelectionChanged == USED_WEAPONS){
			temp = _model->getExtraAllocatedWeapons(namesOut);
		} else if (_lastSelectionChanged == USED_SHIPS) {
			temp = _model->getExtraAllocatedShipEnabler(namesOut);
		} else {
			temp = _model->getExtraAllocatedWeaponEnabler(namesOut);
		}

		if (temp > -1) {
			ui->extraItemSpinbox->setValue(temp);
		}
		else {
			ui->extraItemSpinbox->clear();
		}

		ui->currentTeamComboBox->setCurrentIndex(ui->currentTeamComboBox->findData(_model->getCurrentTeam()));
	}

	// Only enable set required if we are working with ships and weapons that have already been enabled, and not variables.
	if (_mode == TABLE_MODE && _lastSelectionChanged == USED_WEAPONS) {
		ui->requiredWeaponCheckBox->setEnabled(true);
		ui->toggleRequiredLabel->setEnabled(true); // TODO does this do anything? Maybe just set the text color
	} else {
		ui->requiredWeaponCheckBox->setEnabled(false);
		ui->toggleRequiredLabel->setEnabled(false);
	}


	// Finally, populate and select the correct variable for the extra ships combobox
	if (_lastSelectionChanged == USED_SHIPS || _lastSelectionChanged == USED_WEAPONS) {
		ui->extraItemsViaVariableCombo->setEnabled(true);

		SCP_string currentVariable = "";

		if (_mode == TABLE_MODE && _lastSelectionChanged == USED_SHIPS) {
			currentVariable = _model->getCountVarShips(namesOut);			
		} else if (_mode == TABLE_MODE && _lastSelectionChanged == USED_WEAPONS) {
			currentVariable = _model->getCountVarWeapons(namesOut);
		} else if (_mode == VARIABLE_MODE && _lastSelectionChanged == USED_SHIPS) {
			currentVariable = _model->getCountVarShipEnabler(namesOut);
		} else if (_mode == VARIABLE_MODE && _lastSelectionChanged == USED_WEAPONS) {
			currentVariable = _model->getCountVarWeaponEnabler(namesOut);
		}

		if (currentVariable == "") {
			ui->extraItemsViaVariableCombo->setCurrentIndex(0);
		} else {
			for (int x = 0; x < ui->extraItemsViaVariableCombo->count(); ++x) {
				SCP_string variableName = ui->extraItemsViaVariableCombo->itemText(x).toUtf8().constData();
				if (lcase_equal(variableName, currentVariable)) {
					ui->extraItemsViaVariableCombo->setCurrentIndex(x);
					break;
				}
			}
		}

	} else {
		ui->extraItemsViaVariableCombo->setEnabled(false);
	}

	if (_mode == TABLE_MODE) {
		const auto requiredWeapons = _model->getRequiredWeapons();
		for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
			bool found = false;

			for (const auto& weapon : requiredWeapons) {
				SCP_string usedWepName = ui->usedWeaponsList->item(x, 0)->text().toUtf8().constData();
				if (ui->usedWeaponsList->item(x, 0) && ui->usedWeaponsList->item(x,2) && lcase_equal(usedWepName, weapon)) {
					found = true;
					ui->usedWeaponsList->item(x, 2)->setText("Yes");
					break;
				}
			}

			if (!found && ui->usedWeaponsList->item(x, 2)) {
				ui->usedWeaponsList->item(x, 2)->setText("");
			}
		}
	}
}

void TeamLoadoutDialog::initializeUi()
{
	auto list = _model->getTeamList();

	ui->currentTeamComboBox->clear();

	for (const auto& team : list) {
		ui->currentTeamComboBox->addItem(QString::fromStdString(team.first), team.second);
	}
}

SCP_vector<SCP_string> TeamLoadoutDialog::getSelectedShips() 
{
	SCP_vector<SCP_string> namesOut;

	for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
		if (ui->usedShipsList->item(x, 0) && ui->usedShipsList->item(x,0)->isSelected()) {
			SCP_string shipName = ui->usedShipsList->item(x, 0)->text().toUtf8().constData();
			namesOut.emplace_back(shipName);
		}
	}

	return namesOut;
}

SCP_vector<SCP_string> TeamLoadoutDialog::getSelectedWeapons()
{
	SCP_vector<SCP_string> namesOut;

	for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
		if (ui->usedWeaponsList->item(x, 0) && ui->usedWeaponsList->item(x, 0)->isSelected()) {
			SCP_string weaponName = ui->usedWeaponsList->item(x, 0)->text().toUtf8().constData();
			namesOut.emplace_back(weaponName);
		}
	}

	return namesOut;
}

void TeamLoadoutDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void TeamLoadoutDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void TeamLoadoutDialog::on_currentTeamComboBox_currentIndexChanged(int index)
{
	if (index < 0) {
		return;
	}
	int team = ui->currentTeamComboBox->itemData(index).toInt();
	_model->switchTeam(team);

	updateUi();
}


} // namespace fso::fred::dialogs
