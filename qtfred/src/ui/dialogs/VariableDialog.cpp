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


void VariableDialog::onVariablesSelectionChanged() {}
void VariableDialog::onContainersTableUpdated() 
{
	if (_applyingModel){
		return;
	}

} // could be new name
void VariableDialog::onContainersSelectionChanged() {}
void VariableDialog::onContainerContentsTableUpdated() 
{
	if (_applyingModel){
		return;
	}


} // could be new key or new value
void VariableDialog::onContainerContentsSelectionChanged() {}

void VariableDialog::onAddVariableButtonPressed() {}
void VariableDialog::onCopyVariableButtonPressed(){}
void VariableDialog::onDeleteVariableButtonPressed() {}
void VariableDialog::onSetVariableAsStringRadioSelected() {}
void VariableDialog::onSetVariableAsNumberRadioSelected() {}
void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() {}
void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() {}
void VariableDialog::onSaveVariableAsEternalCheckboxClicked() {}

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