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
	connect(this, &QDialog::accepted, _model.get(), &VariableDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &VariableDialogModel::reject);
	
	connect(ui->variablesTable, 
		QOverload<int, int>::of(&QTableWidget::cellChanged),
		this,
		&VariableDialog::onVariablesTableUpdated);

	connect(ui->variablesTable, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&VariableDialog::onVariablesSelectionChanged);

	connect(ui->containersTable,
		QOverload<int, int>::of(&QTableWidget::cellChanged),
		this,
		&VariableDialog::onContainersTableUpdated);

	connect(ui->containersTable, 
		&QTableWidget::itemSelectionChanged, 
		this, 
		&VariableDialog::onContainersSelectionChanged);

	connect(ui->containerContentsTable,
		QOverload<int, int>::of(&QTableWidget::cellChanged),
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

void VariableDialog::onVariablesTableUpdated() {} // could be new name or new value
void VariableDialog::onVariablesSelectionChanged() {}
void VariableDialog::onContainersTableUpdated() {} // could be new name
void VariableDialog::onContainersSelectionChanged() {}
void VariableDialog::onContainerContentsTableUpdated() {} // could be new key or new value
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
	auto variables = _model->getVariableValues();

	for (int x = 0; x < static_cast<int>(variables.size()); ++x){
		if (ui->variablesTable->item(x, 0)){
			ui->variablesTable->item(x, 0)->setText(variables[x]<0>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(variables[x]<0>.c_str());
			ui->variablesTable->setItem(x, 0, item);
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

	if (_currentVariable.empty()){
		if( ui->VariablesTable->item(0,0) && strlen(ui->VariablesTable->item(0,0)->text())){
			_currentVariable = ui->VariablesTable->item(0,0)->text();
		}
			// TODO! Make new ui function with the following stuff.
			// get type with getVariableType
			// get network status with getVariableNetworkStatus
			// get getVariablesOnMissionCloseOrCompleteFlag
			// getVariableEternalFlag
			// string or number value with getVariableStringValue or getVariableNumberValue
	}

	auto containers = _model->getContainerNames();

	// TODO! Change getContainerNames to a tuple with notes/maybe data key types?
	for (x = 0; x < static_cast<int>(containers.size()); ++x){
		if (ui->containersTable->item(x, 0)){
			ui->containersTable->item(x, 0)->setText(containers[x]<0>.c_str());
		} else {
			QTableWidgetItem* item = new QTableWidgetItem(containers[x]<0>.c_str());
			ui->containersTable->setItem(x, 0, item);
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

};


} // namespace dialogs
} // namespace fred
} // namespace fso