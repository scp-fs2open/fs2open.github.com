#include <QCloseEvent>
#include <QKeyEvent>
#include "ShieldSystemDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_ShieldSystemDialog.h"


namespace fso {
namespace fred {
namespace dialogs {

ShieldSystemDialog::ShieldSystemDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::ShieldSystemDialog()),
	_model(new ShieldSystemDialogModel(this, viewport)) {
    ui->setupUi(this);
	
	connect(this, &QDialog::accepted, _model.get(), &ShieldSystemDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &ShieldSystemDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShieldSystemDialog::updateUI);

	connect(ui->shipTeamCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShieldSystemDialog::teamSelectionChanged);
	connect(ui->shipTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShieldSystemDialog::typeSelectionChanged);

	connect(ui->teamHasShieldRadio, &QRadioButton::toggled, this,
		[this](bool param) { _model->setCurrentTeamShieldSys(ui->teamHasShieldRadio->isChecked() ? 0 : 1); });
	connect(ui->teamNoShieldRadio, &QRadioButton::toggled, this,
		[this](bool param) { _model->setCurrentTeamShieldSys(ui->teamNoShieldRadio->isChecked() ? 1 : 0); });
	connect(ui->typeHasShieldRadio, &QRadioButton::toggled, this,
		[this](bool param) { _model->setCurrentTypeShieldSys(ui->typeHasShieldRadio->isChecked() ? 0 : 1); });
	connect(ui->typeNoShieldRadio, &QRadioButton::toggled, this,
		[this](bool param) { _model->setCurrentTypeShieldSys(ui->typeNoShieldRadio->isChecked() ? 1 : 0); });

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ShieldSystemDialog::~ShieldSystemDialog() {
}

void ShieldSystemDialog::updateUI() {
	util::SignalBlockers blockers(this);

	updateTeam();
	updateType();
}

void ShieldSystemDialog::updateTeam() {
	if (ui->shipTeamCombo->count() == 0) {
		for (const auto& teamName : _model->getTeamOptions()) {
			ui->shipTeamCombo->addItem(QString::fromStdString(teamName));
		}
	}

	ui->shipTeamCombo->setCurrentIndex(_model->getCurrentTeam());

	const int status = _model->getCurrentTeamShieldSys();

	ui->teamHasShieldRadio->setChecked(false);
	ui->teamNoShieldRadio->setChecked(false);

	if (status == 0) {
		ui->teamHasShieldRadio->setChecked(true);
	} else if (status == 1) {
		ui->teamNoShieldRadio->setChecked(true);
	}
}

void ShieldSystemDialog::updateType() {
	if (ui->shipTypeCombo->count() == 0) {
		for (const auto& typeName : _model->getShipTypeOptions()) {
			ui->shipTypeCombo->addItem(QString::fromStdString(typeName));
		}
	}

	ui->shipTypeCombo->setCurrentIndex(_model->getCurrentShipType());

	const int status = _model->getCurrentTypeShieldSys();

	ui->typeHasShieldRadio->setChecked(false);
	ui->typeNoShieldRadio->setChecked(false);

	if (status == 0) {
		ui->typeHasShieldRadio->setChecked(true);
	} else if (status == 1) {
		ui->typeNoShieldRadio->setChecked(true);
	}
}

void ShieldSystemDialog::teamSelectionChanged(int index) {
	if (index >= 0) {
		_model->setCurrentTeam(index);
	}
}

void ShieldSystemDialog::typeSelectionChanged(int index) {
	if (index >= 0) {
		_model->setCurrentShipType(index);
	}
}

void ShieldSystemDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		// Instead of calling reject when we close a dialog it should try to close the window which will will allow the
		// user to save unsaved changes
		event->ignore();
		this->close();
		return;
	}
	QDialog::keyPressEvent(event);
}

void ShieldSystemDialog::closeEvent(QCloseEvent* event) {
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question, "Changes detected", "Do you want to keep your changes?",
			{ DialogButton::Yes, DialogButton::No, DialogButton::Cancel });

		if (button == DialogButton::Cancel) {
			event->ignore();
			return;
		}

		if (button == DialogButton::Yes) {
			accept();
			return;
		}
	}

	QDialog::closeEvent(event);
}

}
}
}
