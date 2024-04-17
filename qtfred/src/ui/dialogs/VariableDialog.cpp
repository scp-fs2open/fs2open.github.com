#include "VariableDialog.h"
#include "ui_VariableDialog.h"

#include <tuple>
#include <qlist.h>
#include <qtablewidget.h>

#include <QListWidget>
#include <QMessageBox>
//#include <QtWidgets/QMenuBar>

namespace fso {
namespace fred {
namespace dialogs {

VariableDialog::VariableDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::VariableEditorDialog()), _model(new VariableDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);
	resize(QDialog::sizeHint()); // The best I can tell without some research, when a dialog doesn't use an underling grid or layout, it needs to be resized this way before anything will show up 

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	connect(this, &QDialog::accepted, _model.get(), &VariableDialogModel::checkValidModel);
	connect(this, &QDialog::rejected, _model.get(), &VariableDialogModel::reject);
	
	connect(ui->variablesTable, 
		&QTableWidget::itemChanged,
		this,
		&VariableDialog::onVariablesTableUpdated);

	connect(ui->variablesTable, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&VariableDialog::onVariablesSelectionChanged);

	connect(ui->containersTable,
		&QTableWidget::itemChanged,
		this,
		&VariableDialog::onContainersTableUpdated);

	connect(ui->containersTable, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&VariableDialog::onContainersSelectionChanged);

	connect(ui->containerContentsTable,
		&QTableWidget::itemChanged,
		this,
		&VariableDialog::onContainerContentsTableUpdated);

	connect(ui->containerContentsTable, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&VariableDialog::onContainerContentsSelectionChanged);

	connect(ui->addVariableButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onAddVariableButtonPressed);

	connect(ui->copyVariableButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onCopyVariableButtonPressed);

	connect(ui->deleteVariableButton,
		&QPushButton::clicked,
		this, 
		&VariableDialog::onDeleteVariableButtonPressed);

	connect(ui->setVariableAsStringRadio,
		&QRadioButton::toggled,
		this, 
		&VariableDialog::onSetVariableAsStringRadioSelected);

	connect(ui->setVariableAsNumberRadio,
		&QRadioButton::toggled,
		this, 
		&VariableDialog::onSetVariableAsNumberRadioSelected);

	connect(ui->saveContainerOnMissionCompletedRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSaveVariableOnMissionCompleteRadioSelected);

	connect(ui->saveVariableOnMissionCloseRadio,
		&QRadioButton::toggled,
		this, 
		&VariableDialog::onSaveVariableOnMissionCloseRadioSelected);

	connect(ui->networkVariableCheckbox,
		&QCheckBox::clicked,
		this,
		&VariableDialog::onNetworkVariableCheckboxClicked);

	connect(ui->setVariableAsEternalcheckbox,
		&QCheckBox::clicked,
		this,
		&VariableDialog::onSaveVariableAsEternalCheckboxClicked);

	connect(ui->addContainerButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onAddContainerButtonPressed);

	connect(ui->copyContainerButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onCopyContainerButtonPressed);

	connect(ui->deleteContainerButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onDeleteContainerButtonPressed);

	connect(ui->setContainerAsMapRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSetContainerAsMapRadioSelected);

	connect(ui->setContainerAsListRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSetContainerAsListRadioSelected);

	connect(ui->setContainerAsStringRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSetContainerAsStringRadioSelected);

	connect(ui->setContainerAsNumberRadio,
		&QRadioButton::toggled,
		this, 
		&VariableDialog::onSetContainerAsNumberRadioSelected);

	connect(ui->setContainerKeyAsStringRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSetContainerKeyAsStringRadioSelected);

	connect(ui->setContainerKeyAsNumberRadio,
		&QRadioButton::toggled,
		this, 
		&VariableDialog::onSetContainerKeyAsNumberRadioSelected);

	connect(ui->saveContainerOnMissionCloseRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSaveContainerOnMissionClosedRadioSelected);

	connect(ui->saveContainerOnMissionCompletedRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSaveContainerOnMissionCompletedRadioSelected);

	connect(ui->setContainerAsEternalCheckbox, 
		&QCheckBox::clicked, 
		this, 
		&VariableDialog::onSetContainerAsEternalCheckboxClicked);

	connect(ui->networkContainerCheckbox, 
		&QCheckBox::clicked, 
		this, 
		&VariableDialog::onNetworkContainerCheckboxClicked);

	connect(ui->addContainerItemButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onAddContainerItemButtonPressed);

	connect(ui->copyContainerItemButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onCopyContainerItemButtonPressed);

	connect(ui->deleteContainerItemButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onDeleteContainerItemButtonPressed);


	ui->variablesTable->setColumnCount(3);
	ui->variablesTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->variablesTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	ui->variablesTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->variablesTable->setColumnWidth(0, 70);
	ui->variablesTable->setColumnWidth(1, 70);
	ui->variablesTable->setColumnWidth(2, 70);

	ui->containersTable->setColumnCount(3);
	ui->containersTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->containersTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Types"));
	ui->containersTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->containersTable->setColumnWidth(0, 80);
	ui->containersTable->setColumnWidth(1, 80);
	ui->containersTable->setColumnWidth(2, 75);
	
	ui->containerContentsTable->setColumnCount(2);

	// Default to list
	ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
	ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
	ui->containerContentsTable->setColumnWidth(0, 105);
	ui->containerContentsTable->setColumnWidth(1, 105);

	// set radio buttons to manually toggled, as some of these have the same parent widgets and some don't
	ui->setVariableAsStringRadio->setAutoExclusive(false);
	ui->setVariableAsNumberRadio->setAutoExclusive(false);
	ui->saveContainerOnMissionCompletedRadio->setAutoExclusive(false);
	ui->saveVariableOnMissionCloseRadio->setAutoExclusive(false);
	ui->setContainerAsMapRadio->setAutoExclusive(false);
	ui->setContainerAsListRadio->setAutoExclusive(false);
	ui->setContainerAsStringRadio->setAutoExclusive(false);
	ui->setContainerAsNumberRadio->setAutoExclusive(false);
	ui->saveContainerOnMissionCloseRadio->setAutoExclusive(false);
	ui->saveContainerOnMissionCompletedRadio->setAutoExclusive(false);

	applyModel();
}

// TODO! make sure that when a variable is added that the whole model is reloaded.  
void VariableDialog::onVariablesTableUpdated() 
{
	if (_applyingModel){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one row of items because multiselect will be off.
	for(const auto& item : items) {
		if (item->column() == 0){

			// so if the user just removed the name, mark it as deleted *before changing the name*
			if (_currentVariable != "" && !strlen(item->text().toStdString().c_str())) {
				if (!_model->removeVariable(item->row())) {
					// marking a variable as deleted failed, resync UI
					applyModel();
					return;
				} else {
					updateVariableOptions();
				}
			} else {
				
				auto ret = _model->changeVariableName(item->row(), item->text().toStdString());

				// we put something in the cell, but the model couldn't process it.
				if (strlen(item->text().toStdString().c_str()) && ret == ""){
					// update of variable name failed, resync UI
					applyModel();

				// we had a successful rename.  So update the variable we reference.
				} else if (ret != "") {
					item->setText(ret.c_str());
					_currentVariable = ret;
				}
			}
			// empty return and cell was handled earlier.
		
		// data cell was altered
		} else if (item->column() == 1) {

			// Variable is a string
			if (_model->getVariableType(item->row())){
				SCP_string temp = item->text().toStdString().c_str();
				temp = temp.substr(0, NAME_LENGTH - 1);

				SCP_string ret = _model->setVariableStringValue(item->row(), temp);
				if (ret == ""){
					applyModel();
					return;
				}
				
				item->setText(ret.c_str());
			} else {
				SCP_string source = item->text().toStdString();
				SCP_string temp = trimNumberString(source);

				if (temp != source){
					item->setText(temp.c_str());
				}

				try {
					int ret = _model->setVariableNumberValue(item->row(), std::stoi(temp));
					temp = "";
					sprintf(temp, "%i", ret);
					item->setText(temp.c_str());
				}
				catch (...) {
					applyModel();
				}
			}

		// if the user somehow edited the info that should only come from the model and should not be editable, reload everything.
		} else {
			applyModel();
		}
	}
}


void VariableDialog::onVariablesSelectionChanged() 
{
	if (_applyingModel){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	SCP_string newVariableName = "";

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		if (item->column() == 0){
			newVariableName = item->text().toStdString();
			break;
		}
	}

	if (newVariableName != _currentVariable){
		_currentVariable = newVariableName;
		applyModel();
	}
}


void VariableDialog::onContainersTableUpdated() 
{
	if (_applyingModel){
		return;
	}


} // could be new name

void VariableDialog::onContainersSelectionChanged() 
{
	if (_applyingModel){
		return;
	}

	auto items = ui->containersTable->selectedItems();

	SCP_string newContainerName = "";

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		if (item->column() == 0){
			newContainerName = item->text().toStdString();
			break;
		}
	}

	if (newContainerName != _currentContainer){
		_currentContainer = newContainerName;
		applyModel();
	}
}

void VariableDialog::onContainerContentsTableUpdated() 
{
	if (_applyingModel){
		return;
	}


} // could be new key or new value

void VariableDialog::onContainerContentsSelectionChanged() {
	if (_applyingModel){
		return;
	}

	auto items = ui->containerContentsTable->selectedItems();

	SCP_string newContainerItemName = "";

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		if (item->column() == 0){
			newContainerItemName = item->text().toStdString();
			break;
		}
	}

	if (newContainerItemName != _currentContainerItem){
		_currentContainerItem = newContainerItemName;
		applyModel();
	}
}

void VariableDialog::onAddVariableButtonPressed() 
{
	auto ret = _model->addNewVariable();
	_currentVariable = ret;
	applyModel();
}

void VariableDialog::onCopyVariableButtonPressed()
{
	if (_currentVariable.empty()){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		auto ret = _model->copyVariable(item->row());
		_currentVariable = ret;
		applyModel();
		break;
	}
}

void VariableDialog::onDeleteVariableButtonPressed() 
{
	if (_currentVariable.empty()){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		// Because of the text update we'll need, this needs an applyModel, whether it fails or not.
		auto ret = _model->removeVariable(item->row());
		applyModel();
		break;
	}
}

void VariableDialog::onSetVariableAsStringRadioSelected() 
{
	if (_currentVariable.empty() || ui->setVariableAsStringRadio->isChecked()){
		return;
	}


	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		// this doesn't return succeed or fail directly, 
		// but if it doesn't return true then it failed since this is the string radio
		if(!_model->setVariableType(item->row(), true)){
			applyModel();
		} else {
			ui->setVariableAsStringRadio->setChecked(true);
			ui->setVariableAsNumberRadio->setChecked(false);
		}

		break;
	}

}

void VariableDialog::onSetVariableAsNumberRadioSelected() 
{
	if (_currentVariable.empty() || ui->setVariableAsNumberRadio->isChecked()){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {

		// this doesn't return succeed or fail directly, 
		// but if it doesn't return false then it failed since this is the number radio
		if (!_model->setVariableType(item->row(), false)) {
			applyModel();
		}
		else {
			ui->setVariableAsStringRadio->setChecked(false);
			ui->setVariableAsNumberRadio->setChecked(true);
		}
	}
}


void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() 
{
	if (_currentVariable.empty() || ui->saveContainerOnMissionCompletedRadio->isChecked()){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(item->row(), 1);

		if (ret != 1){
			applyModel();
		} else {
			// TODO!  Need "no persistence" options and functions!
			ui->saveContainerOnMissionCompletedRadio->setChecked(true);
			ui->saveVariableOnMissionCloseRadio->setChecked(false);
			//ui->saveContainerOnMissionCompletedRadio->setChecked(true);
		}
	}
}

void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() 
{
	if (_currentVariable.empty() || ui->saveContainerOnMissionCompletedRadio->isChecked()){
		return;
	}


	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {

		auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(item->row(), 2);

		if (ret != 2){
			applyModel();
		} else {
			// TODO!  Need "no persistence" options.
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveVariableOnMissionCloseRadio->setChecked(true);
			//ui->saveContainerOnMissionCompletedRadio->setChecked(false);
		}
	}
}

void VariableDialog::onSaveVariableAsEternalCheckboxClicked() 
{
	if (_currentVariable.empty()){
		return;
	}


	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		// If the model returns the old status, then the change failed and we're out of sync.	
		if (ui->setVariableAsEternalcheckbox->isChecked() == _model->setVariableEternalFlag(item->row(), !ui->setVariableAsEternalcheckbox->isChecked())) {
			applyModel();
		} else {
			ui->setVariableAsEternalcheckbox->setChecked(!ui->setVariableAsEternalcheckbox->isChecked());
		}
	}
}

void VariableDialog::onNetworkVariableCheckboxClicked()
{
	if (_currentVariable.empty()){
		return;
	}

	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {

		// If the model returns the old status, then the change failed and we're out of sync.	
		if (ui->networkVariableCheckbox->isChecked() == _model->setVariableNetworkStatus(item->row(), !ui->networkVariableCheckbox->isChecked())) {
			applyModel();
		} else {
			ui->networkVariableCheckbox->setChecked(!ui->networkVariableCheckbox->isChecked());
		}
	}
}

void VariableDialog::onAddContainerButtonPressed() {}
void VariableDialog::onCopyContainerButtonPressed() {}
void VariableDialog::onDeleteContainerButtonPressed() {}
void VariableDialog::onSetContainerAsMapRadioSelected() {}
void VariableDialog::onSetContainerAsListRadioSelected() {}
void VariableDialog::onSetContainerAsStringRadioSelected() {}
void VariableDialog::onSetContainerAsNumberRadioSelected() {}
void VariableDialog::onSetContainerKeyAsStringRadioSelected() {}
void VariableDialog::onSetContainerKeyAsNumberRadioSelected() {}
void VariableDialog::onSaveContainerOnMissionClosedRadioSelected() {}
void VariableDialog::onSaveContainerOnMissionCompletedRadioSelected() {}
void VariableDialog::onNetworkContainerCheckboxClicked() {}
void VariableDialog::onSetContainerAsEternalCheckboxClicked() {}
void VariableDialog::onAddContainerItemButtonPressed() {}
void VariableDialog::onCopyContainerItemButtonPressed() {}
void VariableDialog::onDeleteContainerItemButtonPressed() {}


VariableDialog::~VariableDialog(){}; // NOLINT

void VariableDialog::applyModel()
{
	_applyingModel = true;

	auto variables = _model->getVariableValues();
	int x, selectedRow = -1;

	ui->variablesTable->setRowCount(static_cast<int>(variables.size()));

	for (x = 0; x < static_cast<int>(variables.size()); ++x){
		if (ui->variablesTable->item(x, 0)){
			ui->variablesTable->item(x, 0)->setText(variables[x][0].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][0].c_str());
			ui->variablesTable->setItem(x, 0, item);
		}

		// check if this is the current variable.
		if (!_currentVariable.empty() && variables[x][0] == _currentVariable){
			selectedRow = x;
		}

		if (ui->variablesTable->item(x, 1)){
			ui->variablesTable->item(x, 1)->setText(variables[x][1].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][1].c_str());
			ui->variablesTable->setItem(x, 1, item);
		}

		if (ui->variablesTable->item(x, 2)){
			ui->variablesTable->item(x, 2)->setText(variables[x][2].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][2].c_str());
			ui->variablesTable->setItem(x, 2, item);
		}
	}

	/*
	// This empties rows that might have previously had variables
	if (x < ui->variablesTable->rowCount()) {
		++x;
		for (; x < ui->variablesTable->rowCount(); ++x){
			if (ui->variablesTable->item(x, 0)){
				ui->variablesTable->item(x, 0)->setText("");
			}

			if (ui->variablesTable->item(x, 1)){
				ui->variablesTable->item(x, 1)->setText("");
			}

			if (ui->variablesTable->item(x, 2)){
				ui->variablesTable->item(x, 2)->setText("");
			}
		}
	}
	*/

	if (_currentVariable.empty() || selectedRow < 0){
		if (ui->variablesTable->item(0,0) && strlen(ui->variablesTable->item(0,0)->text().toStdString().c_str())){
			_currentVariable = ui->variablesTable->item(0,0)->text().toStdString();
		}
	}

	updateVariableOptions();

	auto containers = _model->getContainerNames();
	ui->containersTable->setRowCount(static_cast<int>(containers.size()));
	selectedRow = -1;

	// TODO! Change getContainerNames to a tuple with notes/maybe data key types?
	for (x = 0; x < static_cast<int>(containers.size()); ++x){
		if (ui->containersTable->item(x, 0)){
			ui->containersTable->item(x, 0)->setText(containers[x][0].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x][0].c_str());
			ui->containersTable->setItem(x, 0, item);
		}

		// check if this is the current variable.
		if (!_currentVariable.empty() && containers[x][0] == _currentVariable){
			selectedRow = x;
		}


		if (ui->containersTable->item(x, 1)){
			ui->containersTable->item(x, 1)->setText(containers[x][1].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x][1].c_str());
			ui->containersTable->setItem(x, 1, item);
		}

		if (ui->containersTable->item(x, 2)){
			ui->containersTable->item(x, 2)->setText(containers[x][2].c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x][2].c_str());
			ui->containersTable->setItem(x, 2, item);
		}
	}

	// This empties rows that might have previously had containers
	if (x < ui->containersTable->rowCount()) {
		++x;
		for (; x < ui->containersTable->rowCount(); ++x){
			if (ui->containersTable->item(x, 0)){
				ui->containersTable->item(x, 0)->setText("");
			}

			if (ui->containersTable->item(x, 1)){
				ui->containersTable->item(x, 1)->setText("");
			}

			if (ui->containersTable->item(x, 2)){
				ui->containersTable->item(x, 2)->setText("");
			}
		}
	}

	if (_currentContainer.empty() || selectedRow < 0){
		if (ui->containersTable->item(0,0) && strlen(ui->containersTable->item(0,0)->text().toStdString().c_str())){
			_currentContainer = ui->containersTable->item(0,0)->text().toStdString();
		}
	}

	// this will update the list/map items.
	updateContainerOptions();

	_applyingModel = false;
};

void VariableDialog::updateVariableOptions()
{
	if (_currentVariable.empty()){
		ui->copyVariableButton->setEnabled(false);
		ui->deleteVariableButton->setEnabled(false);
		ui->setVariableAsStringRadio->setEnabled(false);
		ui->setVariableAsNumberRadio->setEnabled(false);
		ui->saveVariableOnMissionCompletedRadio->setEnabled(false);
		ui->saveVariableOnMissionCloseRadio->setEnabled(false);
		ui->setVariableAsEternalcheckbox->setEnabled(false);

		return;
	}

	ui->copyVariableButton->setEnabled(true);
	ui->deleteVariableButton->setEnabled(true);
	ui->setVariableAsStringRadio->setEnabled(true);
	ui->setVariableAsNumberRadio->setEnabled(true);
	ui->saveVariableOnMissionCompletedRadio->setEnabled(true);
	ui->saveVariableOnMissionCloseRadio->setEnabled(true);
	ui->setVariableAsEternalcheckbox->setEnabled(true);

	auto items = ui->variablesTable->selectedItems();
	int row = -1;

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		row = item->row();
	}

	if (row == -1 && ui->variablesTable->rowCount() > 0) {
		row = 0;
		_currentVariable = ui->variablesTable->item(row, 0)->text().toStdString();
	}


	// start populating values
	bool string = _model->getVariableType(row);
	ui->setVariableAsStringRadio->setChecked(string);
	ui->setVariableAsNumberRadio->setChecked(!string);
	ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));

	int ret = _model->getVariableOnMissionCloseOrCompleteFlag(row);

	if (ret == 0){
		// TODO ADD NO PERSISTENCE
	} else if (ret == 1) {
		ui->saveVariableOnMissionCompletedRadio->setChecked(true);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);		
	} else {
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(true);
	}

	ui->networkVariableCheckbox->setChecked(_model->getVariableNetworkStatus(row));
	ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));

}

void VariableDialog::updateContainerOptions()
{
	if (_currentContainer.empty()){
		ui->copyContainerButton->setEnabled(false);
		ui->deleteContainerButton->setEnabled(false);
		ui->setContainerAsStringRadio->setEnabled(false);
		ui->setContainerAsNumberRadio->setEnabled(false);
		ui->saveContainerOnMissionCompletedRadio->setEnabled(false);
		ui->saveContainerOnMissionCloseRadio->setEnabled(false);
		ui->setContainerAsEternalCheckbox->setEnabled(false);
		ui->setContainerAsMapRadio->setEnabled(false);
		ui->setContainerAsListRadio->setEnabled(false);

		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));


	} else {
		auto items = ui->containersTable->selectedItems();
		int row = -1;

		// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
		for (const auto& item : items) {
			row = item->row();
		}


		ui->copyContainerButton->setEnabled(false);
		ui->deleteContainerButton->setEnabled(false);
		ui->setContainerAsStringRadio->setEnabled(false);
		ui->setContainerAsNumberRadio->setEnabled(false);
		ui->saveContainerOnMissionCompletedRadio->setEnabled(false);
		ui->saveContainerOnMissionCloseRadio->setEnabled(false);
		ui->setContainerAsEternalCheckbox->setEnabled(false);
		ui->setContainerAsMapRadio->setEnabled(false);
		ui->setContainerAsListRadio->setEnabled(false);

		if (_model->getContainerValueType(row)){
			ui->setContainerAsStringRadio->setChecked(true);
			ui->setContainerAsNumberRadio->setChecked(false);
		} else {
			ui->setContainerAsStringRadio->setChecked(false);
			ui->setContainerAsNumberRadio->setChecked(true);
		}
		
		if (_model->getContainerListOrMap(row)){
			ui->setContainerAsListRadio->setChecked(true);
			ui->setContainerAsMapRadio->setChecked(false);

			// Disable Key Controls			
			ui->setContainerKeyAsStringRadio->setEnabled(false);
			ui->setContainerKeyAsNumberRadio->setEnabled(false);

			// Don't forget to change headings
			ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
			ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
			updateContainerDataOptions(true);

		} else {
			ui->setContainerAsListRadio->setChecked(false);
			ui->setContainerAsMapRadio->setChecked(true);

			// Enabled Key Controls
			ui->setContainerKeyAsStringRadio->setEnabled(true);
			ui->setContainerKeyAsNumberRadio->setEnabled(true);

			// Don't forget to change headings
			ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
			ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
			updateContainerDataOptions(false);
		}

		ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerNetworkStatus(row));
		ui->networkContainerCheckbox->setChecked(_model->getContainerNetworkStatus(row));

		int ret = _model->getContainerOnMissionCloseOrCompleteFlag(row);		

		if (ret == 0){
		// TODO ADD NO PERSISTENCE
		} else if (ret == 1) {
			ui->saveContainerOnMissionCompletedRadio->setChecked(true);
			ui->saveContainerOnMissionCloseRadio->setChecked(false);		
		} else {
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveContainerOnMissionCloseRadio->setChecked(true);
		}

	}
}

void VariableDialog::updateContainerDataOptions(bool list)
{

}

SCP_string VariableDialog::trimNumberString(SCP_string source) 
{
	SCP_string ret;

	// account for a lead negative sign.
	if (source[0] == '-') {
		ret = "-";
	}

	// filter out non-numeric digits
	std::copy_if(source.begin(), source.end(), std::back_inserter(ret),
		[](char c) -> bool { 
			bool result = false;

			switch (c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					result = true;
					break;
				default:
					break;
			}

			return result;
		}
	);

	return ret;
}


} // namespace dialogs
} // namespace fred
} // namespace fso