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
	: QDialog(parent), ui(new Ui::VariableDialog()), _model(new VariableDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();

	// Major Changes, like Applying the model, rejecting changes and updating the UI.
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &VariableDialog::updateUI);
	connect(this, &QDialog::accepted, _model.get(), &VariableDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &VariableDialogModel::reject);
	
	connect(ui->variablesTable, 
		QOverload<int, int>::of(&QTableWidget::cellChanged),
		this,
		&VariableDialog::onVariablesTableUpdated);

	connect(ui->variablesTable,
		QOverload<int, int>::of(&QTableWidget::cellChanged),
		this,
		&VariableDialog::onContainersTableUpdated);

	connect(ui->variablesTable,
		QOverload<int, int>::of(&QTableWidget::cellChanged),
		this,
		&VariableDialog::onContainerContentsTableUpdated);

	connect(ui->addVariableButton,
		&QPushButton::clicked,
		this,
		&VariableDialog::onAddVariableButtonPressed);

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
}

	void VariableDialog::onVariablesTableUpdated() {}
	void VariableDialog::onContainersTableUpdated() {}
	void VariableDialog::onContainerContentsTableUpdated() {}
	void VariableDialog::onAddVariableButtonPressed() {}
	void VariableDialog::onDeleteVariableButtonPressed() {}
	void VariableDialog::onSetVariableAsStringRadioSelected() {}
	void VariableDialog::onSetVariableAsNumberRadioSelected() {}
	void VariableDialog::onSaveVariableOnMissionCompleteRadioSelected() {}
	void VariableDialog::onSaveVariableOnMissionCloseRadioSelected() {}
	void VariableDialog::onSaveVariableAsEternalCheckboxClicked() {}

	void VariableDialog::onAddContainerButtonPressed() {}
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

/*
containersTable
containerContentsTable
addVariableButton
deleteVariableButton
setVariableAsStringRadio
setVariableAsNumberRadio
networkVariableCheckbox
saveVariableOnMissionCompletedRaio
saveVariableOnMissionCloseRadio
setVariableAsEternalcheckbox

addContainerButton
deleteContainerButton
setContainerAsMapRadio
setContainerAsListRadio
setContainerAsStringRadio
setContainerAsNumberRadio
saveContainerOnMissionCloseRadio
saveContainerOnMissionCompletedRadio
networkContainerCheckbox
setContainerAsEternalCheckbox
	*/



VariableDialog::~VariableDialog(){}; // NOLINT

void VariableDialog::updateUI(){};


} // namespace dialogs
} // namespace fred
} // namespace fso