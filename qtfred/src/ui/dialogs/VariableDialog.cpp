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
	resize(QDialog::sizeHint()); // The best I can tell without some research, when a dialog doesn't use an underlying grid or layout, it needs to be resized this way before anything will show up 

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	// Here we need to check that there are no issues with variable names or container names, or with maps having duplicate keys.
	connect(ui->OkCancelButtons, &QDialogButtonBox::accepted, this, &VariableDialog::checkValidModel);
	// Reject if the user wants to.
	connect(ui->OkCancelButtons, &QDialogButtonBox::rejected, this, &VariableDialog::preReject);
	connect(this, &QDialog::accepted, _model.get(), &VariableDialogModel::apply);
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
		&QRadioButton::clicked,
		this, 
		&VariableDialog::onSetVariableAsStringRadioSelected);

	connect(ui->setVariableAsNumberRadio,
		&QRadioButton::clicked,
		this, 
		&VariableDialog::onSetVariableAsNumberRadioSelected);

	connect(ui->doNotSaveVariableRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onDoNotSaveVariableRadioSelected);

	connect(ui->saveVariableOnMissionCompletedRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSaveVariableOnMissionCompleteRadioSelected);

	connect(ui->saveVariableOnMissionCloseRadio,
		&QRadioButton::clicked,
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
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSetContainerAsMapRadioSelected);

	connect(ui->setContainerAsListRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSetContainerAsListRadioSelected);

	connect(ui->setContainerAsStringRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSetContainerAsStringRadioSelected);

	connect(ui->setContainerAsNumberRadio,
		&QRadioButton::clicked,
		this, 
		&VariableDialog::onSetContainerAsNumberRadioSelected);

	connect(ui->setContainerKeyAsStringRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSetContainerKeyAsStringRadioSelected);

	connect(ui->setContainerKeyAsNumberRadio,
		&QRadioButton::clicked,
		this, 
		&VariableDialog::onSetContainerKeyAsNumberRadioSelected);

	connect(ui->doNotSaveContainerRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onDoNotSaveContainerRadioSelected);

	connect(ui->saveContainerOnMissionCloseRadio,
		&QRadioButton::clicked,
		this,
		&VariableDialog::onSaveContainerOnMissionCloseRadioSelected);

	connect(ui->saveContainerOnMissionCompletedRadio,
		&QRadioButton::clicked,
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

	connect(ui->shiftItemUpButton,
		&QPushButton::clicked,
		this,
		&VarableDialog::onShiftItemUpButtonPressed);

	connect(ui->shiftItemDownButton,
		&QPushButton::clicked,
		this,
		&VarableDialog::onShiftItemDownButtonPressed);


	ui->variablesTable->setColumnCount(3);
	ui->variablesTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->variablesTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	ui->variablesTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->variablesTable->setColumnWidth(0, 95);
	ui->variablesTable->setColumnWidth(1, 95);
	ui->variablesTable->setColumnWidth(2, 105);

	ui->containersTable->setColumnCount(3);
	ui->containersTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->containersTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Types"));
	ui->containersTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->containersTable->setColumnWidth(0, 95);
	ui->containersTable->setColumnWidth(1, 95);
	ui->containersTable->setColumnWidth(2, 105);

	ui->containerContentsTable->setColumnCount(2);

	// Default to list
	ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
	ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
	ui->containerContentsTable->setColumnWidth(0, 150);
	ui->containerContentsTable->setColumnWidth(1, 150);

	// set radio buttons to manually toggled, as some of these have the same parent widgets and some don't
	// and I don't mind just manually toggling them.
	ui->setVariableAsStringRadio->setAutoExclusive(false);
	ui->setVariableAsNumberRadio->setAutoExclusive(false);
	ui->doNotSaveVariableRadio->setAutoExclusive(false);
	ui->saveVariableOnMissionCompletedRadio->setAutoExclusive(false);
	ui->saveVariableOnMissionCloseRadio->setAutoExclusive(false);

	ui->setContainerAsMapRadio->setAutoExclusive(false);
	ui->setContainerAsListRadio->setAutoExclusive(false);
	ui->setContainerAsStringRadio->setAutoExclusive(false);
	ui->setContainerAsNumberRadio->setAutoExclusive(false);
	ui->saveContainerOnMissionCloseRadio->setAutoExclusive(false);
	ui->saveContainerOnMissionCompletedRadio->setAutoExclusive(false);

	ui->variablesTable->setRowCount(0);
	ui->containersTable->setRowCount(0);
	ui->containerContentsTable->setRowCount(0);
	ui->variablesTable->clearSelection();
	ui->containersTable->clearSelection();
	ui->containerContentsTable->clearSelection();

	applyModel();
}

// TODO! make sure that when a variable is added that the whole model is reloaded.  
// TODO! Fix me.  This function does not work as intended because it must process both, not just one.
void VariableDialog::onVariablesTableUpdated() 
{
	if (_applyingModel){
		return;
	}

	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	
	
	auto item = ui->variablesTable->item(currentRow, 0);
	bool apply = false;

	// so if the user just removed the name, mark it as deleted *before changing the name*
	if (_currentVariable != "" && !strlen(item->text().toStdString().c_str())) {
		if (!_model->removeVariable(item->row())) {
			// marking a variable as deleted failed, resync UI
			apply = true;
		} else {
			updateVariableOptions();
		}
	} else {
		
		auto ret = _model->changeVariableName(item->row(), item->text().toStdString());

		// we put something in the cell, but the model couldn't process it.
		if (strlen(item->text().toStdString().c_str()) && ret == ""){
			// update of variable name failed, resync UI
			apply = true;

		// we had a successful rename.  So update the variable we reference.
		} else if (ret != "") {
			item->setText(ret.c_str());
			_currentVariable = ret;
		}
	}
	// empty return and cell was handled earlier.


	item = ui->variablesTable->item(currentRow, 1);

	// check if data column was altered
	// TODO!  Set up comparison between last and current value
	if (item->column() == 1) {

		// Variable is a string
		if (_model->getVariableType(item->row())){
			SCP_string temp = item->text().toStdString().c_str();
			temp = temp.substr(0, NAME_LENGTH - 1);

			SCP_string ret = _model->setVariableStringValue(item->row(), temp);
			if (ret == ""){
				apply = true;
			} else {
				item->setText(ret.c_str());
			}			
		} else {
			SCP_string source = item->text().toStdString();
			SCP_string temp = _model->trimIntegerString(source);

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


void VariableDialog::onVariablesSelectionChanged() 
{
	if (_applyingModel){
		return;
	}

	int row = getCurrentVariableRow();

	if (row < 0){
		updateVariableOptions();
		return;
	}

	SCP_string newVariableName;

	auto item = ui->variablesTable->item(row, 0); 

	if (item){
		newVariableName = item->text().toStdString();
	}

	item = ui->variablesTable->item(row, 1);

	if (item){
		_currentVariableData = item->text().toStdString();
	}

	if (newVariableName != _currentVariable){
		_currentVariable = newVariableName;
	}

	applyModel();
}


void VariableDialog::onContainersTableUpdated() 
{
	if (_applyingModel){
		return;
	}

	int row = getCurrentContainerRow();

	// just in case something is goofy, return
	if (row < 0){
		return;
	}

	// Are they adding a new container?
	if (row == ui->containersTable->rowCount() - 1){
		if (ui->containersTable->item(row, 0)) {
			SCP_string newString = ui->containersTable->item(row, 0)->text().toStdString();
			if (!newString.empty() && newString != "Add Container ..."){
				_model->addContainer(newString);
				_currentContainer = newString;
				applyModel();
			}
		}

	// are they editing an existing container name?
	} else if (ui->containersTable->item(row, 0)){
		_currentContainer = ui->containersTable->item(row,0)->text().toStdString();
		
		if (_currentContainer != _model->changeContainerName(row, ui->containersTable->item(row,0)->text().toStdString())){
			applyModel();
		}	
	}
}

void VariableDialog::onContainersSelectionChanged() 
{
	if (_applyingModel){
		return;
	}

	int row = getCurrentContainerRow();

	if (row < 0) {
		updateContainerOptions();
		return;
	}

	// guaranteed not to be null, since getCurrentContainerRow already checked.
	SCP_string newContainerName = ui->containersTable->item(row, 0)->text().toStdString();

	if (newContainerName != _currentContainer){
		_currentContainer = newContainerName;
	}

	applyModel(); // Seems to be buggy unless I have this outside the if.
}

// TODO, finish this function
void VariableDialog::onContainerContentsTableUpdated() 
{
	if (_applyingModel){
		return;
	}


} // could be new key or new value

void VariableDialog::onContainerContentsSelectionChanged() 
{
	if (_applyingModel){
		return;
	}

	int row = getCurrentContainerItemRow();

	if (row < 0){
		return;
	}

	auto item = ui->containerContentsTable->item(row, 0);
	SCP_string newContainerItemName;

	if (!item){
		return;
	}

	newContainerItemName = item->text().toStdString();
	item = ui->containerContentsTable->item(row, 1);	
	SCP_string newContainerDataText = (item) ? item->text().toStdString() : "";

	if (newContainerItemName != _currentContainerItem || _currentContainerItemData != newContainerDataText){
		_currentContainerItem = newContainerItemName;
		_currentContainerItemData = newContainerDataText;
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

	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	
	
	auto ret = _model->copyVariable(currentRow);
	_currentVariable = ret;
	applyModel();
}

void VariableDialog::onDeleteVariableButtonPressed() 
{
	if (_currentVariable.empty()){
		return;
	}

	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	

	// Because of the text update we'll need, this needs an applyModel, whether it fails or not.
	if (ui->deleteVariableButton->item(currentRow, 2) && ui->deleteVariableButton->item(currentRow, 2)->text().toStdString() == "Flagged For Deletion"){
		_model->removeVariable(currentRow, false);
		applyModel();
	} else {
		_model->removeVariable(currentRow, true);
		applyModel();
	}

}

void VariableDialog::onSetVariableAsStringRadioSelected() 
{
	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return true then it failed since this is the string radio
	if(!_model->setVariableType(currentRow, true)){
		applyModel();
	} else {
		ui->setVariableAsStringRadio->setChecked(true);
		ui->setVariableAsNumberRadio->setChecked(false);
	}
}

void VariableDialog::onSetVariableAsNumberRadioSelected() 
{
	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return false then it failed since this is the number radio
	if (!_model->setVariableType(currentRow, false)) {
		applyModel();
	}
	else {
		ui->setVariableAsStringRadio->setChecked(false);
		ui->setVariableAsNumberRadio->setChecked(true);
	}
}

void VariableDialog::onDoNotSaveVariableRadioSelected()
{
	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		return;
	}	

	int ret = _model->setVariableOnMissionCloseOrCompleteFlag(currentRow, 0);

	if (ret != 0){
		applyModel();
	} else {
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);
	}
}



void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() 
{
	int row = getCurrentVariableRow();

	if (row < 0){
		return;
	}
	
	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(row, 1);

	if (ret != 1){
		applyModel();
	} else {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);
	}
}

void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() 
{
	int row = getCurrentVariableRow();

	if (row < 0){
		return;
	}

	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(row, 2);

	// out of sync because we did not get the expected return value.
	if (ret != 2){
		applyModel();
	} else {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);
	}
}

void VariableDialog::onSaveVariableAsEternalCheckboxClicked() 
{
	int row = getCurrentVariableRow();

	if (row < 0){
		return;
	}

	// If the model returns the old status, then the change failed and we're out of sync.	
	if (ui->setVariableAsEternalcheckbox->isChecked() == _model->setVariableEternalFlag(row, ui->setVariableAsEternalcheckbox->isChecked())) {
		applyModel();
	} else {
		ui->setVariableAsEternalcheckbox->setChecked(!ui->setVariableAsEternalcheckbox->isChecked());
	}
}

void VariableDialog::onNetworkVariableCheckboxClicked()
{
	int row = getCurrentVariableRow();

	if (row < 0){
		return;
	}

	// If the model returns the old status, then the change failed and we're out of sync.	
	if (ui->networkVariableCheckbox->isChecked() == _model->setVariableNetworkStatus(row, ui->networkVariableCheckbox->isChecked())) {
		applyModel();
	} else {
		ui->networkVariableCheckbox->setChecked(!ui->networkVariableCheckbox->isChecked());
	}

}

void VariableDialog::onAddContainerButtonPressed() 
{
	auto result = (_model->addContainer());

	if (result.empty()) {
			QMessageBox msgBox;
		msgBox.setText("Adding a container failed because the code is out of automatic names.  Try adding a container directly in the table.");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
	}
	
	applyModel();

}

void VariableDialog::onCopyContainerButtonPressed() 
{
	int row = getCurrentContainerRow();

	if (row < 0 ){
		return;		
	}

	// This will always need an apply model update, whether it succeeds or fails.
	_model->copyContainer(row);
	applyModel();
}

void VariableDialog::onDeleteContainerButtonPressed() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->removeContainer(row);
	applyModel();
}

void VariableDialog::onSetContainerAsMapRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerListOrMap(row, false);
	applyModel();
}

void VariableDialog::onSetContainerAsListRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerListOrMap(row, true);
	applyModel();
}


void VariableDialog::onSetContainerAsStringRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerValueType(row, true);
	applyModel();
}

void VariableDialog::onSetContainerAsNumberRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerValueType(row, false);
	applyModel();
}

void VariableDialog::onSetContainerKeyAsStringRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerKeyType(row, true);
	applyModel();
}


void VariableDialog::onSetContainerKeyAsNumberRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	_model->setContainerKeyType(row, false);
	applyModel();
}

void VariableDialog::onDoNotSaveContainerRadioSelected()
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 0) != 0){
		applyModel();
	} else {
		ui->doNotSaveContainerRadio->setChecked(true);
		ui->saveContainerOnMissionCloseRadio->setChecked(false);
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);
	}
}
void VariableDialog::onSaveContainerOnMissionCloseRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 2) != 2)
		applyModel();
	else {
		ui->doNotSaveContainerRadio->setChecked(false);
		ui->saveContainerOnMissionCloseRadio->setChecked(true);
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);
	}
}

void VariableDialog::onSaveContainerOnMissionCompletedRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 1) != 1)	
		applyModel();
	else {
		ui->doNotSaveContainerRadio->setChecked(false);
		ui->saveContainerOnMissionCloseRadio->setChecked(false);
		ui->saveContainerOnMissionCompletedRadio->setChecked(true);
	}
}

void VariableDialog::onNetworkContainerCheckboxClicked() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	if (ui->networkContainerCheckbox->isChecked() != _model->setContainerNetworkStatus(row, ui->networkContainerCheckbox->isChecked())){
		applyModel();
	}
}

void VariableDialog::onSetContainerAsEternalCheckboxClicked() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		return;
	}

	if (ui->setContainerAsEternalCheckbox->isChecked() != _model->setContainerEternalFlag(row, ui->setContainerAsEternalCheckbox->isChecked())){
		applyModel();
	} 
}

void VariableDialog::onAddContainerItemButtonPressed() 
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		return;
	}

	if (_model->getContainerListOrMap(containerRow)) {
		_model->addListItem(containerRow);
	} else {
		_model->addMapItem(containerRow);
	}

	applyModel();
}

void VariableDialog::onCopyContainerItemButtonPressed() 
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		return;
	}

	if (_model->getContainerListOrMap(containerRow)) {
		_model->copyListItem(containerRow, itemRow);
	} else {
		_model->copyMapItem(containerRow, itemRow);
	}

	applyModel();
}

void VariableDialog::onDeleteContainerItemButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		return;
	}

	if (_model->getContainerListOrMap(containerRow)) {
		_model->removeListItem(containerRow, itemRow);
	} else {
		_model->removeMapItem(containerRow, itemRow);
	}

	applyModel();
}

VariableDialog::onShiftItemUpButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		return;
	}

	_model->shiftListItemUp(containerRow, itemRow);
	applyModel();
}

VariableDialog::onShiftItemDownButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		return;
	}

	_model->shiftListItemUp(containerRow, itemRow);
	applyModel();
}


VariableDialog::~VariableDialog(){}; // NOLINT


void VariableDialog::applyModel()
{
	// TODO! We need an undelete action. Best way is to change the text on the button if the notes say "Deleted"
	_applyingModel = true;

	auto variables = _model->getVariableValues();
	int x = 0, selectedRow = -1;

	ui->variablesTable->setRowCount(static_cast<int>(variables.size()) + 1);

	for (x = 0; x < static_cast<int>(variables.size()); ++x){
		if (ui->variablesTable->item(x, 0)){
			ui->variablesTable->item(x, 0)->setText(variables[x][0].c_str());
			ui->variablesTable->item(x, 0)->setFlags(ui->variablesTable->item(x, 0)->flags() | Qt::ItemIsEditable);
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][0].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->variablesTable->setItem(x, 0, item);
		}

		// check if this is the current variable.  This keeps us selecting the correct variable even when
		// there's a deletion.
		if (!_currentVariable.empty() && variables[x][0] == _currentVariable){
			selectedRow = x;
		}

		if (ui->variablesTable->item(x, 1)){
			ui->variablesTable->item(x, 1)->setText(variables[x][1].c_str());
			ui->variablesTable->item(x, 1)->setFlags(ui->variablesTable->item(x, 1)->flags() | Qt::ItemIsEditable);
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][1].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->variablesTable->setItem(x, 1, item);
		}

		if (ui->variablesTable->item(x, 2)){
			ui->variablesTable->item(x, 2)->setText(variables[x][2].c_str());
			ui->variablesTable->item(x, 2)->setFlags(ui->variablesTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x][2].c_str());
			ui->variablesTable->setItem(x, 2, item);
			ui->variablesTable->item(x, 2)->setFlags(item->flags() & ~Qt::ItemIsEditable);
		}
	}

	// set the Add variable row
	if (ui->variablesTable->item(x, 0)){
		ui->variablesTable->item(x, 0)->setText("Add Variable ...");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("Add Variable ...");
		ui->variablesTable->setItem(x, 0, item);
	}

	if (ui->variablesTable->item(x, 1)){
		ui->variablesTable->item(x, 1)->setFlags(ui->variablesTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->item(x, 1)->setText("");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->setItem(x, 1, item);
	}

	if (ui->variablesTable->item(x, 2)){
		ui->variablesTable->item(x, 2)->setFlags(ui->variablesTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->item(x, 2)->setText("");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->setItem(x, 2, item);
	}

	if (_currentVariable.empty() || selectedRow < 0){
		if (ui->variablesTable->item(0, 0) && !ui->variablesTable->item(0, 0)->text().toStdString().empty()){
			_currentVariable = ui->variablesTable->item(0, 0)->text().toStdString();	
		}

		if (ui->variablesTable->item(0, 1)) {
			_currentVariableData = ui->variablesTable->item(0, 1)->text().toStdString();
		}
	}

	updateVariableOptions();

	auto containers = _model->getContainerNames();
	ui->containersTable->setRowCount(static_cast<int>(containers.size() + 1));
	selectedRow = -1;

	for (x = 0; x < static_cast<int>(containers.size()); ++x){
		if (ui->containersTable->item(x, 0)){
			ui->containersTable->item(x, 0)->setText(containers[x][0].c_str());
			ui->containersTable->item(x, 0)->setFlags(ui->containersTable->item(x, 0)->flags() | Qt::ItemIsEditable);
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x][0].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
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

	// do we need to switch the delete button to a restore button?
	if (ui->containersTable->item(row, 2) && ui->containersTable->item(row, 2)->text().toStdString() == "Flagged for Deletion"){
		ui->deleteContainerButton->setText("Restore");
	} else {
		ui->deleteContainerButton->setText("Delete");
	}

	// set the Add container row
	if (ui->containersTable->item(x, 0)){
		ui->containersTable->item(x, 0)->setText("Add Container ...");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("Add Container ...");
		ui->containersTable->setItem(x, 0, item);
	}

	if (ui->containersTable->item(x, 1)){
		ui->containersTable->item(x, 1)->setFlags(ui->containersTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->item(x, 1)->setText("");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->setItem(x, 1, item);
	}

	if (ui->containersTable->item(x, 2)){
		ui->containersTable->item(x, 2)->setFlags(ui->containersTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->item(x, 2)->setText("");
	} else {
		QTableWidgetItem* item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->setItem(x, 2, item);
	}

	if (selectedRow < 0 && ui->containersTable->rowCount() > 1) {
		if (ui->containersTable->item(0, 0)){
			_currentContainer = ui->containersTable->item(0, 0)->text().toStdString();
			ui->containersTable->clearSelection();
			ui->containersTable->item(0, 0)->setSelected(true);
			
		}
	}

	// this will update the list/map items.
	updateContainerOptions();

	_applyingModel = false;
};

void VariableDialog::updateVariableOptions()
{
	int row = getCurrentVariableRow();

	if (row < 0){
		ui->copyVariableButton->setEnabled(false);
		ui->deleteVariableButton->setEnabled(false);
		ui->deleteVariableButton->setText("Delete");
		ui->setVariableAsStringRadio->setEnabled(false);
		ui->setVariableAsNumberRadio->setEnabled(false);
		ui->doNotSaveVariableRadio->setEnabled(false);
		ui->saveVariableOnMissionCompletedRadio->setEnabled(false);
		ui->saveVariableOnMissionCloseRadio->setEnabled(false);
		ui->setVariableAsEternalcheckbox->setEnabled(false);
		ui->networkVariableCheckbox->setEnabled(false);
		ui->onShiftItemUpButton->setEnabled(false);
		ui->onShiftItemDownButton->setEnabled(false);
		return;
	}

	ui->copyVariableButton->setEnabled(true);
	ui->deleteVariableButton->setEnabled(true);
	ui->setVariableAsStringRadio->setEnabled(true);
	ui->setVariableAsNumberRadio->setEnabled(true);
	ui->doNotSaveVariableRadio->setEnabled(true);
	ui->saveVariableOnMissionCompletedRadio->setEnabled(true);
	ui->saveVariableOnMissionCloseRadio->setEnabled(true);
	ui->setVariableAsEternalcheckbox->setEnabled(true);
	ui->networkVariableCheckbox->setEnabled(true);

	// if nothing is selected, but something could be selected, make it so.
	if (row < 0 && ui->variablesTable->rowCount() > 1) {
		row = 0;
		ui->variablesTable->item(row, 0)->setSelected(true);
		_currentVariable = ui->variablesTable->item(row, 0)->text().toStdString();
	}

	// start populating values
	bool string = _model->getVariableType(row);
	ui->setVariableAsStringRadio->setChecked(string);
	ui->setVariableAsNumberRadio->setChecked(!string);

	// do we need to switch the delete button to a restore button?
	if (ui->variablesTable->item(row, 2) && ui->variablesTable->item(row, 2)->text().toStdString() == "Flagged for Deletion"){
		ui->deleteVariableButton->setText("Restore");
	} else {
		ui->deleteVariableButton->setText("Delete");
	}

	int ret = _model->getVariableOnMissionCloseOrCompleteFlag(row);

	if (ret == 0){
		ui->doNotSaveVariableRadio->setChecked(true);
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);		
	} else if (ret == 1) {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCompletedRadio->setChecked(true);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);		
	} else {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(true);
	}

	ui->networkVariableCheckbox->setChecked(_model->getVariableNetworkStatus(row));
	ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));

}

void VariableDialog::updateContainerOptions()
{
	int row = getCurrentContainerRow();

	if (row < 0){
		ui->copyContainerButton->setEnabled(false);
		ui->deleteContainerButton->setEnabled(false);
		ui->deleteContainerButton->setText("Delete");
		ui->setContainerAsStringRadio->setEnabled(false);
		ui->setContainerAsNumberRadio->setEnabled(false);
		ui->setContainerKeyAsStringRadio->setEnabled(false);
		ui->setContainerKeyAsNumberRadio->setEnabled(false);
		ui->doNotSaveContainerRadio->setEnabled(false);
		ui->saveContainerOnMissionCompletedRadio->setEnabled(false);
		ui->saveContainerOnMissionCloseRadio->setEnabled(false);
		ui->setContainerAsEternalCheckbox->setEnabled(false);
		ui->setContainerAsMapRadio->setEnabled(false);
		ui->setContainerAsListRadio->setEnabled(false);
		ui->networkContainerCheckbox->setEnabled(false);
		ui->onShiftItemUpButton->setEnabled(false);
		ui->onShiftItemDownButton->setEnabled(false);

		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
		ui->containerContentsTable->setRowCount(0);

		// if there's no container, there's no container items
		ui->addContainerItemButton->setEnabled(false);
		ui->copyContainerItemButton->setEnabled(false);
		ui->deleteContainerItemButton->setEnabled(false);
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
		ui->containerContentsTable->setRowCount(0);
		ui->shiftItemDownButton->setEnabled(false);
		ui->shiftItemUpButton->setEnabled(false);

	} else {
		auto items = ui->containersTable->selectedItems();

		ui->copyContainerButton->setEnabled(true);
		ui->deleteContainerButton->setEnabled(true);
		ui->setContainerAsStringRadio->setEnabled(true);
		ui->setContainerAsNumberRadio->setEnabled(true);
		ui->doNotSaveContainerRadio->setEnabled(true);
		ui->saveContainerOnMissionCompletedRadio->setEnabled(true);
		ui->saveContainerOnMissionCloseRadio->setEnabled(true);
		ui->setContainerAsEternalCheckbox->setEnabled(true);
		ui->setContainerAsMapRadio->setEnabled(true);
		ui->setContainerAsListRadio->setEnabled(true);
		ui->networkContainerCheckbox->setEnabled(true);

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

			// Enable Key Controls
			ui->setContainerKeyAsStringRadio->setEnabled(true);
			ui->setContainerKeyAsNumberRadio->setEnabled(true);

			// string keys
			if (_model->getContainerKeyType(row)){
				ui->setContainerKeyAsStringRadio->setChecked(true);
				ui->setContainerKeyAsNumberRadio->setChecked(false);

			// number keys
			} else {
				ui->setContainerKeyAsStringRadio->setChecked(false);
				ui->setContainerKeyAsNumberRadio->setChecked(true);
			}

			// Don't forget to change headings
			ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
			ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
			updateContainerDataOptions(false);
		}

		ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerEternalFlag(row));
		ui->networkContainerCheckbox->setChecked(_model->getContainerNetworkStatus(row));

		int ret = _model->getContainerOnMissionCloseOrCompleteFlag(row);		

		if (ret == 0){
			ui->doNotSaveContainerRadio->setChecked(true);
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveContainerOnMissionCloseRadio->setChecked(false);		
		} else if (ret == 1) {
			ui->doNotSaveContainerRadio->setChecked(false);
			ui->saveContainerOnMissionCompletedRadio->setChecked(true);
			ui->saveContainerOnMissionCloseRadio->setChecked(false);		
		} else {
			ui->doNotSaveContainerRadio->setChecked(false);
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveContainerOnMissionCloseRadio->setChecked(true);
		}

	}
}

void VariableDialog::updateContainerDataOptions(bool list)
{
	int row = getCurrentContainerRow();

	// Just in case, No overarching container, no container contents
	if (row < 0){
		ui->addContainerItemButton->setEnabled(false);
		ui->copyContainerItemButton->setEnabled(false);
		ui->deleteContainerItemButton->setEnabled(false);
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
		ui->containerContentsTable->setRowCount(0);
		ui->shiftItemDownButton->setEnabled(false);
		ui->shiftItemUpButton->setEnabled(false);

		return;
	
	// list type container
	} else if (list) {
		// if there's no container, there's no container items
		ui->addContainerItemButton->setEnabled(true);
		ui->copyContainerItemButton->setEnabled(true);
		ui->deleteContainerItemButton->setEnabled(true);
		ui->containerContentsTable->setRowCount(0);
		ui->shiftItemDownButton->setEnabled(true);
		ui->shiftItemUpButton->setEnabled(true);		
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));


		// with string contents
		if (_model->getContainerValueType(row)){
			auto strings = _model->getStringValues(row);
			ui->containerContentsTable->setRowCount(static_cast<int>(strings.size()) + 1);

			int x;
			for (x = 0; x < static_cast<int>(strings.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(strings[x].c_str());
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(strings[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				// set selected and enable shifting functions
				if (strings[x] == _currentContainerItem){
					ui->containerContentsTable->clearSelection();
					ui->containerContentsTable->item(x,0)->setSelected(true);

					// more than one item and not already at the top of the list.
					if (x > 0 && x < static_cast<int>(strings.size())){
						ui->onShiftItemUpButton->setEnabled(false);
					}
					
					if (x > -1 && x < static_Cast<int>(strings.size()) - 1){
						ui->onShiftItemDownButton->setEnabled(false);
					}
				}

				// empty out the second column as it's not needed in list mode
				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText("");
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
				} else {
					QTableWidgetItem* item = new QTableWidgetItem("");
					item->setFlags(item->flags() & ~Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}

			if (ui->containerContentsTable->item(x, 0)){
				ui->containerContentsTable->item(x, 0)->setText("Add item ...");
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("Add item ...");
				ui->containerContentsTable->setItem(x, 0, item);
			}

			if (ui->containerContentsTable->item(x, 1)){
				ui->containerContentsTable->item(x, 1)->setText("");
				ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("");
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
				ui->containerContentsTable->setItem(x, 1, item);
			}
		
		// list with number contents
		} else {
			auto numbers = _model->getNumberValues(row);
			ui->containerContentsTable->setRowCount(static_cast<int>(numbers.size()) + 1);

			int x;
			for (x = 0; x < static_cast<int>(numbers.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(std::to_string(numbers[x]).c_str());
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(std::to_string(numbers[x]).c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				// empty out the second column as it's not needed in list mode
				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText("");
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
				} else {
					QTableWidgetItem* item = new QTableWidgetItem("");
					item->setFlags(item->flags() & ~Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}

			if (ui->containerContentsTable->item(x, 0)){
				ui->containerContentsTable->item(x, 0)->setText("Add item ...");
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("Add item ...");
				ui->containerContentsTable->setItem(x, 0, item);
			}

			if (ui->containerContentsTable->item(x, 1)){
				ui->containerContentsTable->item(x, 1)->setText("");
				ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("");
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
				ui->containerContentsTable->setItem(x, 1, item);
			}

		}

	// or it could be a map container
	} else {
		ui->addContainerItemButton->setEnabled(true);
		ui->copyContainerItemButton->setEnabled(true);
		ui->deleteContainerItemButton->setEnabled(true);
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));

		// Enable shift up and down buttons are off in Map mode.
		ui->onShiftItemUpButton->setEnabled(false);
		ui->onShiftItemDownButton->setEnabled(false);

		// keys I didn't bother to make separate.  Should have done the same with values.
		auto& keys = _model->getMapKeys(row);

		// string valued map.
		if (_model->getContainerValueType(row)){
			auto& strings = _model->getStringValues(row);

			// use the map as the size because map containers are only as good as their keys anyway.
			ui->containerContentsTable->setRowCount(static_cast<int>(keys.size()) + 1);

			int x;
			for (x = 0; x < static_cast<int>(keys.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(keys[x].c_str());
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(keys[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText(strings[x].c_str());
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(strings[x].c_str());
					item->setFlags(item->flags() | Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}

		// number valued map
		} else {
			auto& numbers = _model->getNumberValues(row);
			ui->containerContentsTable->setRowCount(static_cast<int>(keys.size()) + 1);

			int x;
			for (x = 0; x < static_cast<int>(keys.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(keys[x].c_str());
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(keys[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText(std::to_string(numbers[x]).c_str());
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
				} else {
					QTableWidgetItem* item = new QTableWidgetItem(std::to_string(numbers[x]).c_str());
					item->setFlags(item->flags() | Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}

			if (ui->containerContentsTable->item(x, 0)){
				ui->containerContentsTable->item(x, 0)->setText("Add key ...");
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("Add key ...");
				ui->containerContentsTable->setItem(x, 0, item);
			}

			if (ui->containerContentsTable->item(x, 1)){
				ui->containerContentsTable->item(x, 1)->setText("Add Value ...");
				ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
			} else {
				QTableWidgetItem* item = new QTableWidgetItem("Add Value ...");
				item->setFlags(item->flags() | Qt::ItemIsEditable);
				ui->containerContentsTable->setItem(x, 1, item);
			}
		}
	}
}

int VariableDialog::getCurrentVariableRow()
{
	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		if (item && item->column() == 0 && item->text().toStdString() != "Add Variable ...") {
			return item->row();
		}
	}

	return -1;
}

int VariableDialog::getCurrentContainerRow()
{
	auto items = ui->containersTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		if (item && item->column() == 0 && item->text().toStdString() != "Add Container ...") {
			return item->row();
		}
	}

	return -1;
}

int VariableDialog::getCurrentContainerItemRow()
{
	auto items = ui->containerContentsTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		if (item && item->column() == 0 && item->text().toStdString() != "Add Item ...") {
			return item->row();
		}
	}

	return -1;
}

void VariableDialog::preReject()
{
	QMessageBox msgBox;
	msgBox.setText("Are you sure you want to discard your changes?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes) {
		reject();
	}
}

void VariableDialog::checkValidModel()
{
	if (_model->checkValidModel()) {
		accept();
	}
}

} // namespace dialogs
} // namespace fred
} // namespace fso