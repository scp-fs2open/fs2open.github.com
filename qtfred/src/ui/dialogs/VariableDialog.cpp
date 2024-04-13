#include "VariableDialog.h"
#include "ui_VariableDialog.h"

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

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &VariableDialog::updateUI);
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
		&VariableDialog::onSetContainerAsNumberRadio);

	connect(ui->saveContainerOnMissionCloseRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSaveContainerOnMissionClosedRadioSelected);

	connect(ui->saveContainerOnMissionCompletedRadio,
		&QRadioButton::toggled,
		this,
		&VariableDialog::onSaveContainerOnMissionCompletedRadioSelected);

	connect(ui->addContainerItemButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onAddContainerItemButtonPressed);

	connect(ui->deleteContainerItemButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onDeleteContainerItemButtonPressed);

	connect(ui->setContainerAsEternalCheckbox, 
		&QCheckBox::clicked, 
		this, 
		&VariableDialog::onSetContainerAsEternalCheckboxClicked);

	resize(QDialog::sizeHint());

	ui->variablesTable->setColumnCount(3);
	ui->variablesTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->variablesTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	ui->variablesTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->variablesTable->setColumnWidth(0, 200);
	ui->variablesTable->setColumnWidth(1, 200);
	ui->variablesTable->setColumnWidth(2, 200);

	ui->containersTable->setColumnCount(3);
	ui->containersTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
	ui->containersTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Types"));
	ui->containersTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Notes"));
	ui->containersTable->setColumnWidth(0, 200);
	ui->containersTable->setColumnWidth(1, 200);
	ui->containersTable->setColumnWidth(2, 200);
	
	ui->containerContentsTable->setColumnCount(2);

	ui->containerContentsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
	ui->containerContentsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
	ui->containerContentsTable->setColumnWidth(0, 200);
	ui->containerContentsTable->setColumnWidth(1, 200);

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

	// yes, selected items returns a list, but we really should only have one item because multiselect will be off.
	for(const auto& item : items) {
		if (item->column() == 0){

			// so if the user just removed the name, mark it as deleted *before changing the name*
			if (_currentVariable != "" && !strlen(item->text.c_str())){
				if (!_model->removeVariable(item->row())) {
					// marking a variable as deleted failed, resync UI
					applyModel();
					return;
				}
			}

			auto ret = _model->changeVariableName(item->row(), item->text().toStdString());

			// we put something in the cell, but the model couldn't process it.
			if (strlen(item->text()) && ret == ""){
				// update of variable name failed, resync UI
				applyModel();

			// we had a successful rename.  So update the variable we reference.
			} else if (ret != "") {
				item->setText(ret.c_str());
				_currentVariable = ret;
			}

			// empty return and cell was handled earlier.
		
		// data cell was altered
		} else if (item->column() == 1) {

			// Variable is a string
			if (_model->getVariableType(int->row())){
				SCP_string temp = item->text()->toStdString().c_str();
				temp = temp.substr(0, NAME_LENGTH - 1);

				SCP_string ret = _model->setVariableStringValue(int->row(), temp);
				if (ret == ""){
					applyModel();
					return;
				}
				
				item->setText(ret.c_str());
			} else {
				SCP_string temp;
				SCP_string source = item->text().toStdString();

				SCP_string temp = trimNumberString();

				if (temp != source){
					item->setText(temp.c_str());
				}

				try {
					int ret = _model->setVariableNumberValue(item->row(), std::stoi(temp));
					temp = "";
					sprintf(temp, "%i", ret);
					item->setText(temp);
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

}

void VariableDialog::onAddVariableButtonPressed() 
{
	auto ret = _model->addNewVriable();
	_currentVariable = ret;
	applyModel();
}

void VariableDialog::onCopyVariableButtonPressed()
{
	if (_currentVariable.empty()){
		return;
	}

	auto ret = _model->copyVariable(_currentVariable);
	_currentVariable = ret;
	applyModel();
}

void VariableDialog::onDeleteVariableButtonPressed() 
{
	if (_currentVariable.empty()){
		return;
	}

	// Because of the text update we'll need, this needs an applyModel, whether it fails or not.
	_model->removeVariable(_currentVariable);
	applyModel();
}

void VariableDialog::onSetVariableAsStringRadioSelected() 
{
	if (_currentVariable.empty() || ui->setVariableAsStringRadio->isChecked()){
		return;
	}

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return true then it failed since this is the string radio
	if(!_model->setVariableType(_currentVariable, true)){
		applyModel();
	} else {
		ui->setVariableAsStringRadio->setChecked(true);
		ui->setVariableAsNumberRadio->setChecked(false);
	}
}

void VariableDialog::onSetVariableAsNumberRadioSelected() 
{
	if (_currentVariable.empty() || ui->setVariableAsNumberRadio->isChecked()){
		return;
	}

	// this doesn't return succeed or fail directly, 
	// but if it doesn't return false then it failed since this is the number radio
	if(!_model->setVariableType(_currentVariable, false)){
		applyModel();
	} else {
		ui->setVariableAsStringRadio->setChecked(false);
		ui->setVariableAsNumberRadio->setChecked(true);
	}
}


void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() 
{
	if (_currentVariable.empty() || ui->saveContainerOnMissionCompletedRadio->isChecked()){
		return;
	}

	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(_currentVariable, 1);

	if (ret != 1){
		applyModel();
	} else {
		// TODO!  Need "no persistence" options and functions!
		ui->saveContainerOnMissionCompletedRadio->setChecked(true);
		ui->saveVariableOnMissionCloseRadio->setChecked(false);
		//ui->saveContainerOnMissionCompletedRadio->setChecked(true);
	}
}

void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() 
{
	if (_currentVariable.empty() || ui->saveContainerOnMissionCompletedRadio->isChecked()){
		return;
	}

	auto ret = _model->setVariableOnMissionCloseOrCompleteFlag(_currentVariable, 2);

	if (ret != 2){
		applyModel();
	} else {
		// TODO!  Need "no persistence" options.
		ui->saveContainerOnMissionCompletedRadio->setChecked(false);
		ui->saveVariableOnMissionCloseRadio->setChecked(true);
		//ui->saveContainerOnMissionCompletedRadio->setChecked(false);
	}
}

void VariableDialog::onSaveVariableAsEternalCheckboxClicked() 
{
	if (_currentVariable.empty()){
		return;
	}

	// If the model returns the old status, then the change failed and we're out of sync.	
	if (ui->setVariableAsEternalcheckbox->isChecked() == _model->setVariableEternalFlag(_currentVariable, !ui->setVariableAsEternalcheckbox->isChecked())){
		applyModel();
	} else {
		_ui->setVariableAsEternalcheckbox->setChecked(!ui->setVariableAsEternalcheckbox->isChecked());
	}
}

void VariableDialog::onNetworkVariableCheckboxClicked()
{
	if (_currentVariable.empty()){
		return;
	}

	// If the model returns the old status, then the change failed and we're out of sync.	
	if (ui->setVariableNetworkStatus->isChecked() == _model->setVariableNetworkStatus(_currentVariable, !ui->setVariableNetworkStatus->isChecked())){
		applyModel();
	} else {
		_ui->setVariableNetworkStatus->setChecked(!ui->setVariableNetworkStatus->isChecked());
	}
}

void VariableDialog::onAddContainerButtonPressed() {}
void VariableDialog::onCopyContainerButtonPressed() {}
void VariableDialog::onDeleteContainerButtonPressed() {}
void VariableDialog::onSetContainerAsMapRadioSelected() {}
void VariableDialog::onSetContainerAsListRadioSelected() {}
void VariableDialog::onSetContainerAsStringRadioSelected() {}
void VariableDialog::onSetContainerAsNumberRadio() {}
void VariableDialog::onSaveContainerOnMissionClosedRadioSelected() {}
void VariableDialog::onSaveContainerOnMissionCompletedRadioSelected() {}
void VariableDialog::onNetworkContainerCheckboxClicked() {}
void VariableDialog::onSetContainerAsEternalCheckboxClicked() {}
void VariableDialog::onAddContainerItemButtonPressed() {}
void VariableDialog::onDeleteContainerItemButtonPressed() {}


VariableDialog::~VariableDialog(){}; // NOLINT

void VariableDialog::applyModel()
{
	_applyingModel = true;

	auto variables = _model->getVariableValues();
	int x, selectedRow = -1;

	for (x = 0; x < static_cast<int>(variables.size()); ++x){
		if (ui->variablesTable->item(x, 0)){
			ui->variablesTable->item(x, 0)->setText(variables[x]<0>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x]<0>.c_str());
			ui->variablesTable->setItem(x, 0, item);
		}

		// check if this is the current variable.
		if (!_currentVariable.empty() && variables[x]<0> == _currentVariable){
			selectedRow = x
		}

		if (ui->variablesTable->item(x, 1)){
			ui->variablesTable->item(x, 1)->setText(variables[x]<1>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x]<1>.c_str());
			ui->variablesTable->setItem(x, 1, nameItem);
		}

		if (ui->variablesTable->item(x, 2)){
			ui->variablesTable->item(x, 2)->setText(variables[x]<2>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x]<2>.c_str());
			ui->variablesTable->setItem(x, 2, item);
		}
	}

	// TODO, try setting row count?
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

	if (_currentVariable.empty() || selectedRow < 0){
		if (ui->variablesTable->item(0,0) && strlen(ui->variablesTable->item(0,0)->text())){
			_currentVariable = ui->variablesTable->item(0,0)->text();
		}
	}

	// TODO! Make new ui function with the following stuff.
	// get type with getVariableType
	// get network status with getVariableNetworkStatus
	// get getVariablesOnMissionCloseOrCompleteFlag
	// getVariableEternalFlag
	// string or number value with getVariableStringValue or getVariableNumberValue
	updateVariableOptions();


	auto containers = _model->getContainerNames();
	selectedRow = -1;

	// TODO! Change getContainerNames to a tuple with notes/maybe data key types?
	for (x = 0; x < static_cast<int>(containers.size()); ++x){
		if (ui->containersTable->item(x, 0)){
			ui->containersTable->item(x, 0)->setText(containers[x]<0>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x]<0>.c_str());
			ui->containersTable->setItem(x, 0, item);
		}

		// check if this is the current variable.
		if (!_currentVariable.empty() && containers[x]<0> == _currentVariable){
			selectedRow = x;
		}


		if (ui->containersTable->item(x, 1)){
			ui->containersTable->item(x, 1)->setText(containers[x]<1>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x]<1>.c_str());
			ui->containersTable->setItem(x, 1, nameItem);
		}

		if (ui->containersTable->item(x, 2)){
			ui->containersTable->item(x, 2)->setText(containers[x]<2>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x]<2>.c_str());
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
		if (ui->containersTable->item(0,0) && strlen(ui->containersTable->item(0,0)->text())){
			_currentContainer = ui->containersTable->item(0,0)->text();
		}
	}

	// this will update the list/map items.
	updateContainerOptions();

	_applyingModel = false;
};

void VariableDialog::updateVariableOptions()
{
	if (_currentVariable.empty()){
		ui->copyVariableButton.setEnabled(false);
		ui->deleteVariableButton.setEnabled(false);
		ui->setVariableAsStringRadio.setEnabled(false);
		ui->setVariableAsNumberRadio.setEnabled(false);
		ui->saveContainerOnMissionCompletedRadio.setEnabled(false);
		ui->saveVariableOnMissionCloseRadio.setEnabled(false);
		ui->setVariableAsEternalcheckbox.setEnabled(false);

		return;
	}

	ui->copyVariableButton.setEnabled(true);
	ui->deleteVariableButton.setEnabled(true);
	ui->setVariableAsStringRadio.setEnabled(true);
	ui->setVariableAsNumberRadio.setEnabled(true);
	ui->saveContainerOnMissionCompletedRadio.setEnabled(true);
	ui->saveVariableOnMissionCloseRadio.setEnabled(true);
	ui->setVariableAsEternalcheckbox.setEnabled(true);

	// start populating values
	bool string = _model->getVariableType(_currentVariable);
	ui->setVariableAsStringRadio.setChecked(string);
	ui->setVariableAsNumberRadio.setChecked(!string);
	ui->setVariableAsEternalcheckbox.setChecked();

	int ret = _model->getVariableOnMissionCloseOrCompleteFlag(_currentVariable);

	if (ret == 0){
		// TODO ADD NO PERSISTENCE
	} else if (ret == 1) {
		ui->saveContainerOnMissionCompletedRadio.setChecked(true);
		ui->saveVariableOnMissionCloseRadio.setChecked(false);		
	} else {
		ui->saveContainerOnMissionCompletedRadio.setChecked(false);
		ui->saveVariableOnMissionCloseRadio.setChecked(true);
	}

	ui->networkVariableCheckbox.setChecked(_model->getVariableNetworkStatus(_currentVariable));
	ui->setVariableAsEternalcheckbox.setChecked(_model->getVariableEternalFlag(_currentVariable));

}

void VariableDialog::updateContainerOptions()
{

}

SCP_string VariableDialog::trimNumberString(SCP_string source) 
{
	SCP_string ret;

	// account for a lead negative sign.
	if (source[0] == "-") {
		ret = "-";
	}

	// filter out non-numeric digits
	std::copy_if(s1.begin(), s1.end(), std::back_inserter(ret),
		[](char c){ 
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
					return true;
					break;
				default:
					return false;
					break;
			}
		}
	);

	return ret;
}


} // namespace dialogs
} // namespace fred
} // namespace fso