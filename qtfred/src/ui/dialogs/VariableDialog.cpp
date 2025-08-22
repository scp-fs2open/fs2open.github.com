#include "VariableDialog.h"
#include "ui_VariableDialog.h"

#include <tuple>
#include <qlist.h>
#include <qtablewidget.h>

#include <QListWidget>
#include <QMessageBox>
//#include <QtWidgets/QMenuBar>

namespace fso::fred::dialogs {

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
	connect(ui->OkCancelButtons, &QDialogButtonBox::rejected, this, &VariableDialog::reject);
	connect(this, &QDialog::accepted, _model.get(), &VariableDialogModel::apply);
	
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
		&VariableDialog::onShiftItemUpButtonPressed);

	connect(ui->shiftItemDownButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onShiftItemDownButtonPressed);

	connect(ui->swapKeysAndValuesButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onSwapKeysAndValuesButtonPressed);

	connect(ui->selectFormatCombobox,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&VariableDialog::onSelectFormatComboboxSelectionChanged);

	ui->variablesTable->setColumnCount(3);
	ui->variablesTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->variablesTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	ui->variablesTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->variablesTable->setColumnWidth(0, 200);
	ui->variablesTable->setColumnWidth(1, 200);
	ui->variablesTable->setColumnWidth(2, 130);

	ui->containersTable->setColumnCount(3);
	ui->containersTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->containersTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Types"));
	ui->containersTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->containersTable->setColumnWidth(0, 190);
	ui->containersTable->setColumnWidth(1, 220);
	ui->containersTable->setColumnWidth(2, 120);

	ui->containerContentsTable->setColumnCount(2);

	// Default to list
	ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
	ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
	ui->containerContentsTable->setColumnWidth(0, 245);
	ui->containerContentsTable->setColumnWidth(1, 245);

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

	ui->selectFormatCombobox->addItem("Verbose");
	ui->selectFormatCombobox->addItem("Simplified");
	ui->selectFormatCombobox->addItem("Type and ()");
	ui->selectFormatCombobox->addItem("Type and <>");
	ui->selectFormatCombobox->addItem("Only ()");
	ui->selectFormatCombobox->addItem("Only <>");
	ui->selectFormatCombobox->addItem("No extra Marks");

	applyModel();
}

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
	SCP_string itemText = item->text().toUtf8().constData();
	bool apply = false;

	// This will only be true if the user is trying to add a new variable.
	if (currentRow == ui->variablesTable->rowCount() - 1) {

		// make sure the item exists before we dereference
		if (ui->variablesTable->item(currentRow, 0)) {

			// Add the new container.
			if (!itemText.empty() && itemText != "Add Variable ...") {
				_model->addNewVariable(itemText);
				_currentVariable = itemText;
				_currentVariableData = "";
				applyModel();
			}

		} else {
			// reapply the model if the item is null.
			applyModel();
		}

		// we're done here because we cannot edit the data column on the add variable row
		return;
	}

	// so if the user just removed the name, mark it as deleted
	if (itemText.empty() && !_currentVariable.empty()) {

		_model->removeVariable(item->row(), true);
		// these things need to be done whether the deletion failed or not.
		_currentVariable = _model->changeVariableName(item->row(), itemText);
		apply = true;

	// if the user is restoring a deleted variable by inserting a name....
	} else if (!itemText.empty() && _currentVariable.empty()){

		_model->removeVariable(item->row(), false);
		// these things need to be done whether the restoration failed or not.
		_currentVariable =_model->changeVariableName(item->row(), itemText);
		apply = true;
		
	} else if (itemText != _currentVariable){

		auto ret = _model->changeVariableName(item->row(), itemText);

		// we put something in the cell, but the model couldn't process it.
		if (strlen(item->text().toUtf8().constData()) && ret.empty()) {
			// update of variable name failed, resync UI
			apply = true;

		// we had a successful rename.  So update the variable we reference.
		} else if (!ret.empty()) {
			item->setText(ret.c_str());
			_currentVariable = ret;
		}
	}// No action needed if the first cell was not changed.


	// now work on the variable data cell
	item = ui->variablesTable->item(currentRow, 1);
	itemText = item->text().toUtf8().constData();

	// check if data column was altered
	if (itemText != _currentVariableData) {
		// Variable is a string
		if (_model->getVariableType(item->row())){
			SCP_string temp = itemText;
			temp = temp.substr(0, NAME_LENGTH - 1);

			SCP_string ret = _model->setVariableStringValue(item->row(), temp);
			if (ret.empty()){
				apply = true;
			} else {
				item->setText(ret.c_str());
				_currentVariableData = ret;
			}
		
		// Variable is a number
		} else {
			SCP_string source = item->text().toUtf8().constData();
			SCP_string temp = _model->trimIntegerString(source);

			try {
				int ret = _model->setVariableNumberValue(item->row(), std::stoi(temp));
				temp = "";
				sprintf(temp, "%i", ret);
				item->setText(temp.c_str());
			}
			catch (...) {
				// that's not good....
				apply = true;
			}

			// best we can do is to set this to temp, whether conversion fails or not.
			_currentContainerItemCol2 = temp;
		}
	}

	if (apply) {
		applyModel();
	}
}


void VariableDialog::onVariablesSelectionChanged() 
{
	if (_applyingModel){
		applyModel();
		return;
	}

	int row = getCurrentVariableRow();

	if (row < 0){
		updateVariableOptions(false);
		return;
	}

	SCP_string newVariableName;

	auto item = ui->variablesTable->item(row, 0); 

	if (item){
		newVariableName = item->text().toUtf8().constData();
	}

	item = ui->variablesTable->item(row, 1);

	if (item){
		_currentVariableData = item->text().toUtf8().constData();
	}

	if (newVariableName != _currentVariable){
		_currentVariable = newVariableName;
	}

	applyModel();
}


void VariableDialog::onContainersTableUpdated() 
{
	if (_applyingModel){
		applyModel();
		return;
	}

	int row = getCurrentContainerRow();

	// just in case something is goofy, return
	if (row < 0){
		applyModel();
		return;
	}

	// Are they adding a new container?
	if (row == ui->containersTable->rowCount() - 1){
		if (ui->containersTable->item(row, 0)) {
			SCP_string newString = ui->containersTable->item(row, 0)->text().toUtf8().constData();
			if (!newString.empty() && newString != "Add Container ..."){
				_model->addContainer(newString);
				_currentContainer = newString;
				applyModel();
			}
		}
		else {
			applyModel();
		}

		return;

	// are they editing an existing container name?
	} else if (ui->containersTable->item(row, 0)){
		SCP_string newName = ui->containersTable->item(row,0)->text().toUtf8().constData();

		// Restoring a deleted container?
		if (_currentContainer.empty()){
			_model->removeContainer(row, false);
		// Removing a container?
		} else if (newName.empty()) {
			_model->removeContainer(row, true);		
		}

		_currentContainer = _model->changeContainerName(row, newName);
		applyModel();
	}
}

void VariableDialog::onContainersSelectionChanged() 
{
	if (_applyingModel){
		applyModel();
		return;
	}

	int row = getCurrentContainerRow();

	if (row < 0) {
		updateContainerOptions(false);
		return;
	}

	// guaranteed not to be null, since getCurrentContainerRow already checked.
	_currentContainer = ui->containersTable->item(row, 0)->text().toUtf8().constData();
	applyModel();
}

void VariableDialog::onContainerContentsTableUpdated() 
{
	if (_applyingModel){
		applyModel();
		return;
	}

	int containerRow = getCurrentContainerRow();
	int row = getCurrentContainerItemRow();



	// just in case something is goofy, return
	if (row < 0 || containerRow < 0){
		applyModel();
		return;
	}

	// Are they adding a new item?
	if (row == ui->containerContentsTable->rowCount() - 1){
	
		SCP_string newString;

		if (ui->containerContentsTable->item(row, 0)) {
			newString = ui->containerContentsTable->item(row, 0)->text().toUtf8().constData();
			
			if (!newString.empty() && newString != "Add item ..."){
				
				if (_model->getContainerListOrMap(containerRow)) {
					_model->addListItem(containerRow, newString);
				} else {
					_model->addMapItem(containerRow, newString, "");
				}
				
				_currentContainerItemCol1 = newString;
				_currentContainerItemCol2 = "";

				applyModel();
				return;
			}
		
		} 
		
		if (ui->containerContentsTable->item(row, 1)) {
			newString = ui->containerContentsTable->item(row, 1)->text().toUtf8().constData();
			
			if (!newString.empty() && newString != "Add item ..."){
				
				// This should not be a list container.
				if (_model->getContainerListOrMap(containerRow)) {
					applyModel();
					return;
				}

				auto ret = _model->addMapItem(containerRow, "", newString);
				
				_currentContainerItemCol1 = ret.first;
				_currentContainerItemCol2 = ret.second;

				applyModel();
				return;
			}
		}

	// are they editing an existing container item column 1?
	} else if (ui->containerContentsTable->item(row, 0)){
		SCP_string newText = ui->containerContentsTable->item(row, 0)->text().toUtf8().constData();
			
		if (_model->getContainerListOrMap(containerRow)){

			if (newText != _currentContainerItemCol1){

				// Trim the string if necessary
				if (!_model->getContainerValueType(containerRow)){
					newText = _model->trimIntegerString(newText);		
				}
				
				// Finally change the list item
				_currentContainerItemCol1 = _model->changeListItem(containerRow, row, newText);
				applyModel();
				return;
			} 	
				
		} else if (newText != _currentContainerItemCol1){
			_model->changeMapItemKey(containerRow, row, newText);
			applyModel();
			return;
		}
	}

	// if we're here, nothing has changed so far.  So let's attempt column 2
	if (ui->containerContentsTable->item(row, 1) && !_model->getContainerListOrMap(containerRow)){
		
		SCP_string newText = ui->containerContentsTable->item(row, 1)->text().toUtf8().constData();
		
		if(newText != _currentContainerItemCol2){
			
			if (_model->getContainerValueType(containerRow)){
				_currentContainerItemCol2 = _model->changeMapItemStringValue(containerRow, row, newText);
			
			} else {	
				try{
					_currentContainerItemCol2 = _model->changeMapItemNumberValue(containerRow, row, std::stoi(_model->trimIntegerString(newText)));
				}
				catch(...) {
					_currentContainerItemCol2 = _model->changeMapItemNumberValue(containerRow, row, 0);					
				}				
			}

			applyModel();
		}
	}
} 

void VariableDialog::onContainerContentsSelectionChanged() 
{
	if (_applyingModel){
		applyModel();
		return;
	}

	int row = getCurrentContainerItemRow();

	if (row < 0){
		applyModel();
		return;
	}

	auto item = ui->containerContentsTable->item(row, 0);
	SCP_string newContainerItemName;

	if (!item){
		applyModel();
		return;
	}

	newContainerItemName = item->text().toUtf8().constData();
	item = ui->containerContentsTable->item(row, 1);	
	SCP_string newContainerDataText = (item) ? item->text().toUtf8().constData() : "";

	if (newContainerItemName != _currentContainerItemCol1 || _currentContainerItemCol2 != newContainerDataText){
		_currentContainerItemCol1 = newContainerItemName;
		_currentContainerItemCol2 = newContainerDataText;
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
		applyModel();
		return;
	}

	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		applyModel();
		return;
	}	
	
	auto ret = _model->copyVariable(currentRow);
	_currentVariable = ret;
	applyModel();
}

void VariableDialog::onDeleteVariableButtonPressed() 
{
	if (_currentVariable.empty()){
		applyModel();
		return;
	}

	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		applyModel();
		return;
	}	

	// Because of the text update we'll need, this needs an applyModel, whether it fails or not.
	SCP_string btn_text = ui->deleteVariableButton->text().toUtf8().constData();
	if (btn_text == "Restore") {
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
		applyModel();
		return;
	}	

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return true then it failed since this is the string radio
	_model->setVariableType(currentRow, true);
	applyModel();
}

void VariableDialog::onSetVariableAsNumberRadioSelected() 
{
	int currentRow = getCurrentVariableRow();

	if (currentRow < 0){
		applyModel();
		return;
	}	

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return false then it failed since this is the number radio
	_model->setVariableType(currentRow, false);
	applyModel();
}

void VariableDialog::onDoNotSaveVariableRadioSelected()
{
	int currentRow = getCurrentVariableRow();

	if (currentRow < 0 || !ui->doNotSaveVariableRadio->isChecked()){
		applyModel();
		return;
	}	

	int ret = _model->setVariableOnMissionCloseOrCompleteFlag(currentRow, 0);

	if (ret != 0){
		applyModel();
	} else {
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);

		ui->setVariableAsEternalcheckbox->setChecked(false);
		ui->setVariableAsEternalcheckbox->setEnabled(false);
	}
}

void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() 
{
	int row = getCurrentVariableRow();

	if (row < 0 || !ui->saveVariableOnMissionCompletedRadio->isChecked()){
		applyModel();
		return;
	}
	
	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(row, 1);

	if (ret != 1){
		applyModel();
	} else {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);

		ui->setVariableAsEternalcheckbox->setEnabled(true);
		ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));
	}
}

void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() 
{
	int row = getCurrentVariableRow();

	if (row < 0 || !ui->saveVariableOnMissionCloseRadio->isChecked()){
		applyModel();
		return;
	}

	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(row, 2);

	// out of sync because we did not get the expected return value.
	if (ret != 2){
		applyModel();
	} else {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);

		ui->setVariableAsEternalcheckbox->setEnabled(true);
		ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));		
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
		applyModel();
		return;
	}

	// Because of the text update we'll need, this needs an applyModel, whether it fails or not.
	SCP_string btn_text = ui->deleteContainerButton->text().toUtf8().constData();
	if (btn_text == "Restore"){
		_model->removeContainer(row, false);
	} else {
		_model->removeContainer(row, true);
	}

	applyModel();
}

void VariableDialog::onSetContainerAsMapRadioSelected() 
{
	// to avoid visual weirdness, make it false.
	ui->setContainerAsListRadio->setChecked(false);
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerListOrMap(row, false);
	applyModel();
}

void VariableDialog::onSetContainerAsListRadioSelected() 
{
	// to avoid visual weirdness, make it false.
	ui->setContainerAsMapRadio->setChecked(false);
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerListOrMap(row, true);
	applyModel();
}


void VariableDialog::onSetContainerAsStringRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerValueType(row, true);
	applyModel();
}

void VariableDialog::onSetContainerAsNumberRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerValueType(row, false);
	applyModel();
}

void VariableDialog::onSetContainerKeyAsStringRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerKeyType(row, true);
	applyModel();
}


void VariableDialog::onSetContainerKeyAsNumberRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	_model->setContainerKeyType(row, false);
	applyModel();
}

void VariableDialog::onDoNotSaveContainerRadioSelected()
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 0) != 0){
		applyModel();
	} else {
		ui->doNotSaveContainerRadio->setChecked(true);
		ui->saveContainerOnMissionCloseRadio->setChecked(false);
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);

		ui->setContainerAsEternalCheckbox->setChecked(false);
		ui->setContainerAsEternalCheckbox->setEnabled(false);
	}
}

void VariableDialog::onSaveContainerOnMissionCompletedRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 1) != 1)	
		applyModel();
	else {
		ui->doNotSaveContainerRadio->setChecked(false);
		ui->saveContainerOnMissionCloseRadio->setChecked(false);
		ui->saveContainerOnMissionCompletedRadio->setChecked(true);

		ui->setContainerAsEternalCheckbox->setEnabled(true);
		ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerEternalFlag(row));
	}
}

void VariableDialog::onSaveContainerOnMissionCloseRadioSelected() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
		return;
	}

	if (_model->setContainerOnMissionCloseOrCompleteFlag(row, 2) != 2)
		applyModel();
	else {
		ui->doNotSaveContainerRadio->setChecked(false);
		ui->saveContainerOnMissionCloseRadio->setChecked(true);
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);

		ui->setContainerAsEternalCheckbox->setEnabled(true);
		ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerEternalFlag(row));
	}
}

void VariableDialog::onNetworkContainerCheckboxClicked() 
{
	int row = getCurrentContainerRow();

	if (row < 0){
		applyModel();
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
		applyModel();
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
		applyModel();
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
		applyModel();
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		applyModel();
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
		applyModel();
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		applyModel();
		return;
	}

	if (_model->getContainerListOrMap(containerRow)) {
		_model->removeListItem(containerRow, itemRow);
	} else {
		_model->removeMapItem(containerRow, itemRow);
	}

	applyModel();
}

void VariableDialog::onShiftItemUpButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		applyModel();
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	// item row being 0 is bad here since we're shifting up.
	if (itemRow < 1){
		applyModel();
		return;
	}

	_model->shiftListItemUp(containerRow, itemRow);
	applyModel();
}

void VariableDialog::onShiftItemDownButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		applyModel();
		return;
	}

	int itemRow = getCurrentContainerItemRow();

	if (itemRow < 0){
		applyModel();
		return;
	}

	_model->shiftListItemDown(containerRow, itemRow);
	applyModel();
}

void VariableDialog::onSwapKeysAndValuesButtonPressed()
{
	int containerRow = getCurrentContainerRow();
	
	if (containerRow < 0){
		applyModel();
		return;
	}

	_model->swapKeyAndValues(containerRow);
	applyModel();
}

void VariableDialog::onSelectFormatComboboxSelectionChanged()
{
	_model->setTextMode(ui->selectFormatCombobox->currentIndex());
	applyModel();
}

VariableDialog::~VariableDialog(){}; // NOLINT


void VariableDialog::applyModel()
{
	if (_applyingModel) {
		return;
	}

	_applyingModel = true;

	auto variables = _model->getVariableValues();
	int x = 0, selectedRow = -1;

	ui->variablesTable->setRowCount(static_cast<int>(variables.size()) + 1);
	bool safeToAlter = false;

	for (x = 0; x < static_cast<int>(variables.size()); ++x){
		if (ui->variablesTable->item(x, 0)){
			ui->variablesTable->item(x, 0)->setText(variables[x][0].c_str());
			ui->variablesTable->item(x, 0)->setFlags(ui->variablesTable->item(x, 0)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem(variables[x][0].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->variablesTable->setItem(x, 0, item);
		}

		// check if this is the current variable.  This keeps us selecting the correct variable even when
		// there's a deletion.
		if (selectedRow < 0 && !_currentVariable.empty() && variables[x][0] == _currentVariable){
			selectedRow = x;

		    if (_model->safeToAlterVariable(selectedRow)){
				safeToAlter = true;
   			}
		}

		if (ui->variablesTable->item(x, 1)){
			ui->variablesTable->item(x, 1)->setText(variables[x][1].c_str());
			ui->variablesTable->item(x, 1)->setFlags(ui->variablesTable->item(x, 1)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem(variables[x][1].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->variablesTable->setItem(x, 1, item);
		}

		if (ui->variablesTable->item(x, 2)){
			ui->variablesTable->item(x, 2)->setText(variables[x][2].c_str());
			ui->variablesTable->item(x, 2)->setFlags(ui->variablesTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem(variables[x][2].c_str());
			ui->variablesTable->setItem(x, 2, item);
			ui->variablesTable->item(x, 2)->setFlags(item->flags() & ~Qt::ItemIsEditable);
		}
	}

	// set the Add variable row
	if (ui->variablesTable->item(x, 0)){
		ui->variablesTable->item(x, 0)->setText("Add Variable ...");
	} else {
		auto item = new QTableWidgetItem("Add Variable ...");
		ui->variablesTable->setItem(x, 0, item);
	}

	if (ui->variablesTable->item(x, 1)){
		ui->variablesTable->item(x, 1)->setFlags(ui->variablesTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->item(x, 1)->setText("");
	} else {
		auto item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->setItem(x, 1, item);
	}

	if (ui->variablesTable->item(x, 2)){
		ui->variablesTable->item(x, 2)->setFlags(ui->variablesTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->item(x, 2)->setText("");
	} else {
		auto item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->variablesTable->setItem(x, 2, item);
	}

	if (_currentVariable.empty() || selectedRow < 0){
		SCP_string text = ui->variablesTable->item(0, 0)->text().toUtf8().constData();
		if (ui->variablesTable->item(0, 0) && !text.empty()) {
			_currentVariable = text;	
		}

		if (ui->variablesTable->item(0, 1)) {
			_currentVariableData = ui->variablesTable->item(0, 1)->text().toUtf8().constData();
		}
	}

	updateVariableOptions(safeToAlter);

	auto containers = _model->getContainerNames();
	ui->containersTable->setRowCount(static_cast<int>(containers.size() + 1));
	selectedRow = -1;

	for (x = 0; x < static_cast<int>(containers.size()); ++x){
		if (ui->containersTable->item(x, 0)){
			ui->containersTable->item(x, 0)->setText(containers[x][0].c_str());
			ui->containersTable->item(x, 0)->setFlags(ui->containersTable->item(x, 0)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem(containers[x][0].c_str());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->containersTable->setItem(x, 0, item);
		}

		// check if this is the current variable.
		if (selectedRow < 0 && containers[x][0] == _currentContainer){
			selectedRow = x;
		}

		if (ui->containersTable->item(x, 1)){
			ui->containersTable->item(x, 1)->setText(containers[x][1].c_str());
		} else {
			auto item = new QTableWidgetItem(containers[x][1].c_str());
			ui->containersTable->setItem(x, 1, item);
		}

		if (ui->containersTable->item(x, 2)){
			ui->containersTable->item(x, 2)->setText(containers[x][2].c_str());
		} else {
			auto item = new QTableWidgetItem(containers[x][2].c_str());
			ui->containersTable->setItem(x, 2, item);
		}
	}

	// do we need to switch the delete button to a restore button?
	SCP_string var = selectedRow > -1 ? ui->containersTable->item(selectedRow, 2)->text().toUtf8().constData() : "";
	if (selectedRow > -1 && ui->containersTable->item(selectedRow, 2) && var == "To Be Deleted") {
		ui->deleteContainerButton->setText("Restore");
		
		// We can't restore empty container names.
		SCP_string text = ui->containersTable->item(selectedRow, 0)->text().toUtf8().constData();
		if (ui->containersTable->item(selectedRow, 0) && text.empty()){
			ui->deleteContainerButton->setEnabled(false);
		} else {
			ui->deleteContainerButton->setEnabled(true);		
		}

	} else {
		ui->deleteContainerButton->setText("Delete");
	}

	// set the Add container row
	if (ui->containersTable->item(x, 0)){
		ui->containersTable->item(x, 0)->setText("Add Container ...");
	} else {
		auto item = new QTableWidgetItem("Add Container ...");
		ui->containersTable->setItem(x, 0, item);
	}

	if (ui->containersTable->item(x, 1)){
		ui->containersTable->item(x, 1)->setFlags(ui->containersTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->item(x, 1)->setText("");
	} else {
		auto item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->setItem(x, 1, item);
	}

	if (ui->containersTable->item(x, 2)){
		ui->containersTable->item(x, 2)->setFlags(ui->containersTable->item(x, 2)->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->item(x, 2)->setText("");
	} else {
		auto item = new QTableWidgetItem("");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->setItem(x, 2, item);
	}

	bool safeToAlterContainer = false;

	if (selectedRow < 0 && ui->containersTable->rowCount() > 1) {
		if (ui->containersTable->item(0, 0)){
			_currentContainer = ui->containersTable->item(0, 0)->text().toUtf8().constData();
			ui->containersTable->clearSelection();
			ui->containersTable->item(0, 0)->setSelected(true);
		}
	} else if (selectedRow > -1){
		safeToAlterContainer = _model->safeToAlterContainer(selectedRow);
	}

	// this will update the list/map items.
	updateContainerOptions(safeToAlterContainer);

	_applyingModel = false;
};

void VariableDialog::updateVariableOptions(bool safeToAlter)
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
		ui->shiftItemUpButton->setEnabled(false);
		ui->shiftItemDownButton->setEnabled(false);
		return;
	}

	// options that are always safe
	ui->copyVariableButton->setEnabled(true);
	ui->doNotSaveVariableRadio->setEnabled(true);
	ui->saveVariableOnMissionCompletedRadio->setEnabled(true);
	ui->saveVariableOnMissionCloseRadio->setEnabled(true);
	ui->networkVariableCheckbox->setEnabled(true);

	// options that are only safe if there are no references
	if (safeToAlter){
		ui->deleteVariableButton->setEnabled(true);
		ui->setVariableAsStringRadio->setEnabled(true);
		ui->setVariableAsNumberRadio->setEnabled(true);
	} else {
		ui->deleteVariableButton->setEnabled(false);
		ui->setVariableAsStringRadio->setEnabled(false);
		ui->setVariableAsNumberRadio->setEnabled(false);
	}

	// start populating values
	bool string = _model->getVariableType(row);
	ui->setVariableAsStringRadio->setChecked(string);
	ui->setVariableAsNumberRadio->setChecked(!string);

	// do we need to switch the delete button to a restore button?
	SCP_string var = ui->variablesTable->item(row, 2) ? ui->variablesTable->item(row, 2)->text().toUtf8().constData() : "";
	if (ui->variablesTable->item(row, 2) && var == "To Be Deleted"){
		ui->deleteVariableButton->setText("Restore");		

		// We can't restore empty variable names.
		SCP_string text = ui->variablesTable->item(row, 0)->text().toUtf8().constData();
		if (ui->variablesTable->item(row, 0) && text.empty()){
			ui->deleteVariableButton->setEnabled(false);
		} else {
			ui->deleteVariableButton->setEnabled(true);		
		}

	} else {
		ui->deleteVariableButton->setText("Delete");
	}

	int ret = _model->getVariableOnMissionCloseOrCompleteFlag(row);

	if (ret == 0){
		ui->doNotSaveVariableRadio->setChecked(true);
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);

		ui->setVariableAsEternalcheckbox->setChecked(false);
		ui->setVariableAsEternalcheckbox->setEnabled(false);
	} else if (ret == 1) {
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCompletedRadio->setChecked(true);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);		

		ui->setVariableAsEternalcheckbox->setEnabled(true);
		ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));
	} else {
		ui->setVariableAsEternalcheckbox->setEnabled(true);
		ui->doNotSaveVariableRadio->setChecked(false);
		ui->saveVariableOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(true);

		ui->setVariableAsEternalcheckbox->setEnabled(true);
		ui->setVariableAsEternalcheckbox->setChecked(_model->getVariableEternalFlag(row));
	}

	ui->networkVariableCheckbox->setChecked(_model->getVariableNetworkStatus(row));
}

void VariableDialog::updateContainerOptions(bool safeToAlter)
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
		ui->shiftItemUpButton->setEnabled(false);
		ui->shiftItemDownButton->setEnabled(false);
		ui->swapKeysAndValuesButton->setEnabled(false);

		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));


		// if there's no container, there's no container items
		ui->addContainerItemButton->setEnabled(false);
		ui->copyContainerItemButton->setEnabled(false);
		ui->deleteContainerItemButton->setEnabled(false);
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
		ui->shiftItemDownButton->setEnabled(false);
		ui->shiftItemUpButton->setEnabled(false);
		ui->containerContentsTable->clearSelection();
		ui->containerContentsTable->setRowCount(0);


	} else {
		// options that should always be turned on
		ui->copyContainerButton->setEnabled(true);
		ui->doNotSaveContainerRadio->setEnabled(true);
		ui->saveContainerOnMissionCompletedRadio->setEnabled(true);
		ui->saveContainerOnMissionCloseRadio->setEnabled(true);
		ui->networkContainerCheckbox->setEnabled(true);

		// options that require it be safe to alter because the container is not referenced
		ui->deleteContainerButton->setEnabled(safeToAlter);
		ui->setContainerAsStringRadio->setEnabled(safeToAlter);
		ui->setContainerAsNumberRadio->setEnabled(safeToAlter);
		ui->setContainerAsMapRadio->setEnabled(safeToAlter);
		ui->setContainerAsListRadio->setEnabled(safeToAlter);

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

			updateContainerDataOptions(true, safeToAlter);

		} else {
			ui->setContainerAsListRadio->setChecked(false);
			ui->setContainerAsMapRadio->setChecked(true);

			// Enable Key Controls
			ui->setContainerKeyAsStringRadio->setEnabled(safeToAlter);
			ui->setContainerKeyAsNumberRadio->setEnabled(safeToAlter);

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
			updateContainerDataOptions(false, safeToAlter);
		}

		ui->networkContainerCheckbox->setChecked(_model->getContainerNetworkStatus(row));

		int ret = _model->getContainerOnMissionCloseOrCompleteFlag(row);		

		if (ret == 0){
			ui->doNotSaveContainerRadio->setChecked(true);
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveContainerOnMissionCloseRadio->setChecked(false);

			ui->setContainerAsEternalCheckbox->setChecked(false);
			ui->setContainerAsEternalCheckbox->setEnabled(false);
					
		} else if (ret == 1) {
			ui->doNotSaveContainerRadio->setChecked(false);
			ui->saveContainerOnMissionCompletedRadio->setChecked(true);
			ui->saveContainerOnMissionCloseRadio->setChecked(false);		

			ui->setContainerAsEternalCheckbox->setEnabled(true);
			ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerEternalFlag(row));
		} else {
			ui->doNotSaveContainerRadio->setChecked(false);
			ui->saveContainerOnMissionCompletedRadio->setChecked(false);
			ui->saveContainerOnMissionCloseRadio->setChecked(true);

			ui->setContainerAsEternalCheckbox->setEnabled(true);
			ui->setContainerAsEternalCheckbox->setChecked(_model->getContainerEternalFlag(row));
		}

	}
}

void VariableDialog::updateContainerDataOptions(bool list, bool safeToAlter)
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
		ui->swapKeysAndValuesButton->setEnabled(false);

		return;
	
	// list type container
	} else if (list) {
		// if there's no container, there's no container items
		ui->addContainerItemButton->setEnabled(true);
		ui->copyContainerItemButton->setEnabled(true);
		ui->deleteContainerItemButton->setEnabled(true);
		ui->shiftItemDownButton->setEnabled(true);
		ui->shiftItemUpButton->setEnabled(true);		
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Value"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
		ui->swapKeysAndValuesButton->setEnabled(false);

		ui->containerContentsTable->setRowCount(0);

		int x;

		// with string contents
		if (_model->getContainerValueType(row)){
			auto& strings = _model->getStringValues(row);
			int containerItemsRow = -1;
			ui->containerContentsTable->setRowCount(static_cast<int>(strings.size()) + 1);

			
			for (x = 0; x < static_cast<int>(strings.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(strings[x].c_str());
				} else {
					auto item = new QTableWidgetItem(strings[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				// set selected and enable shifting functions
				if (containerItemsRow < 0 && strings[x] == _currentContainerItemCol1){
					ui->containerContentsTable->clearSelection();
					ui->containerContentsTable->item(x,0)->setSelected(true);

					// more than one item and not already at the top of the list.
					if (x <= 0 || x >= static_cast<int>(strings.size())){
						ui->shiftItemUpButton->setEnabled(false);
					}
					
					if (x <= -1 || x >= static_cast<int>(strings.size()) - 1){
						ui->shiftItemDownButton->setEnabled(false);
					}
				}

				// empty out the second column as it's not needed in list mode
				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText("");
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
				} else {
					auto item = new QTableWidgetItem("");
					item->setFlags(item->flags() & ~Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}

		// list with number contents
		} else {
			auto& numbers = _model->getNumberValues(row);
			int containerItemsRow = -1;
			ui->containerContentsTable->setRowCount(static_cast<int>(numbers.size()) + 1);

			for (x = 0; x < static_cast<int>(numbers.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(std::to_string(numbers[x]).c_str());
				} else {
					auto item = new QTableWidgetItem(std::to_string(numbers[x]).c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				// set selected and enable shifting functions
				if (containerItemsRow < 0 ){

					SCP_string temp;

					if (numbers[x] == 0){
						temp = "0";
					} else {
						sprintf(temp, "%i", numbers[x]);
					}
					
					if (temp == _currentContainerItemCol1){
						ui->containerContentsTable->clearSelection();
						ui->containerContentsTable->item(x,0)->setSelected(true);

						// more than one item and not already at the top of the list.
						if (x <= 0 || x >= static_cast<int>(numbers.size())){
							ui->shiftItemUpButton->setEnabled(false);
						}
					
						if (x <= -1 || x >= static_cast<int>(numbers.size()) - 1){
							ui->shiftItemDownButton->setEnabled(false);
						}
					}
				}

				// empty out the second column as it's not needed in list mode
				if (ui->containerContentsTable->item(x, 1)){
					ui->containerContentsTable->item(x, 1)->setText("");
					ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
				} else {
					auto item = new QTableWidgetItem("");
					item->setFlags(item->flags() & ~Qt::ItemIsEditable);
					ui->containerContentsTable->setItem(x, 1, item);
				}
			}
		}

		if (ui->containerContentsTable->item(x, 0)){
			ui->containerContentsTable->item(x, 0)->setText("Add item ...");
			ui->containerContentsTable->item(x, 0)->setFlags(ui->containerContentsTable->item(x, 0)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem("Add item ...");
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->containerContentsTable->setItem(x, 0, item);
		}

		if (ui->containerContentsTable->item(x, 1)){
			ui->containerContentsTable->item(x, 1)->setText("");
			ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() & ~Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem("");
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			ui->containerContentsTable->setItem(x, 1, item);
		}

	// or it could be a map container
	} else {
		ui->addContainerItemButton->setEnabled(true);
		ui->copyContainerItemButton->setEnabled(true);
		ui->deleteContainerItemButton->setEnabled(true);
		ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
		ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));

		// Enable shift up and down buttons are off in Map mode, because order makes no difference
		ui->shiftItemUpButton->setEnabled(false);
		ui->shiftItemDownButton->setEnabled(false);

		// we can swap if it's safe or if the data types match.  If the data types *don't* match, then we run into reference issues.
		ui->swapKeysAndValuesButton->setEnabled(safeToAlter || _model->getContainerKeyType(row) == _model->getContainerValueType(row));

		// keys I didn't bother to make separate.  Should have done the same with values, ah regrets.
		auto& keys = _model->getMapKeys(row);
		
		int x;
		// string valued map.
		if (_model->getContainerValueType(row)){
			auto& strings = _model->getStringValues(row);

			// use the map as the size because map containers are only as good as their keys anyway.
			ui->containerContentsTable->setRowCount(static_cast<int>(keys.size()) + 1);

			for (x = 0; x < static_cast<int>(keys.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(keys[x].c_str());
				} else {
					auto item = new QTableWidgetItem(keys[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				if (x < static_cast<int>(strings.size())){
					if (ui->containerContentsTable->item(x, 1)){
						ui->containerContentsTable->item(x, 1)->setText(strings[x].c_str());
						ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
					} else {
						auto item = new QTableWidgetItem(strings[x].c_str());
						item->setFlags(item->flags() | Qt::ItemIsEditable);
						ui->containerContentsTable->setItem(x, 1, item);
					}				
				}
			}

		// number valued map
		} else {
			auto& numbers = _model->getNumberValues(row);
			ui->containerContentsTable->setRowCount(static_cast<int>(keys.size()) + 1);

			for (x = 0; x < static_cast<int>(keys.size()); ++x){
				if (ui->containerContentsTable->item(x, 0)){
					ui->containerContentsTable->item(x, 0)->setText(keys[x].c_str());
				} else {
					auto item = new QTableWidgetItem(keys[x].c_str());
					ui->containerContentsTable->setItem(x, 0, item);
				}

				if (x < static_cast<int>(numbers.size())){
					if (ui->containerContentsTable->item(x, 1)){
						ui->containerContentsTable->item(x, 1)->setText(std::to_string(numbers[x]).c_str());
						ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
					} else {
						auto item = new QTableWidgetItem(std::to_string(numbers[x]).c_str());
						item->setFlags(item->flags() | Qt::ItemIsEditable);
						ui->containerContentsTable->setItem(x, 1, item);
					}				
				} else {
					if (ui->containerContentsTable->item(x, 1)){
						ui->containerContentsTable->item(x, 1)->setText("");
						ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
					} else {
						auto item = new QTableWidgetItem("");
						item->setFlags(item->flags() | Qt::ItemIsEditable);
						ui->containerContentsTable->setItem(x, 1, item);
					}
				}
			}
		}

		if (ui->containerContentsTable->item(x, 0)){
			ui->containerContentsTable->item(x, 0)->setText("Add item ...");
			ui->containerContentsTable->item(x, 0)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem("Add item ...");
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->containerContentsTable->setItem(x, 0, item);
		}

		if (ui->containerContentsTable->item(x, 1)){
			ui->containerContentsTable->item(x, 1)->setText("Add item ...");
			ui->containerContentsTable->item(x, 1)->setFlags(ui->containerContentsTable->item(x, 1)->flags() | Qt::ItemIsEditable);
		} else {
			auto item = new QTableWidgetItem("Add item ...");
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui->containerContentsTable->setItem(x, 1, item);
		}
	}
}

int VariableDialog::getCurrentVariableRow()
{
	auto items = ui->variablesTable->selectedItems();

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for (const auto& item : items) {
		SCP_string var = item->text().toUtf8().constData();
		if (item && item->column() == 0 && var != "Add Variable ...") {
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
		SCP_string var = item->text().toUtf8().constData();
		if (item && item->column() == 0 && var != "Add Container ...") {
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
		SCP_string var = item->text().toUtf8().constData();
		if (item && ((item->column() == 0 && var != "Add item ...") || (item->column() == 1 && var != "Add item ..."))) {
			return item->row();
		}
	}

	return -1;
}

void VariableDialog::checkValidModel()
{
	if (ui->OkCancelButtons->button(QDialogButtonBox::Ok)->hasFocus() && _model->checkValidModel()){		
		accept();
	}
}

} // namespace dialogs

