#include "CustomWingNamesDialog.h"

#include "ui_CustomWingNamesDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

CustomWingNamesDialog::CustomWingNamesDialog(QWidget* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::CustomWingNamesDialog()), _model(new CustomWingNamesDialogModel(this, viewport)),
	_viewport(viewport) {
    ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &CustomWingNamesDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &CustomWingNamesDialogModel::reject);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &CustomWingNamesDialog::updateUI);
	
	// Starting wings
	connect(ui->startingWing_1, &QLineEdit::textChanged, this, [this](const QString & param) { startingWingChanged(param, 0); });
	connect(ui->startingWing_2, &QLineEdit::textChanged, this, [this](const QString & param) { startingWingChanged(param, 1); });
	connect(ui->startingWing_3, &QLineEdit::textChanged, this, [this](const QString & param) { startingWingChanged(param, 2); });

	// Squadron wings
	connect(ui->squadronWing_1, &QLineEdit::textChanged, this, [this](const QString & param) { squadronWingChanged(param, 0); });
	connect(ui->squadronWing_2, &QLineEdit::textChanged, this, [this](const QString & param) { squadronWingChanged(param, 1); });
	connect(ui->squadronWing_3, &QLineEdit::textChanged, this, [this](const QString & param) { squadronWingChanged(param, 2); });
	connect(ui->squadronWing_4, &QLineEdit::textChanged, this, [this](const QString & param) { squadronWingChanged(param, 3); });
	connect(ui->squadronWing_5, &QLineEdit::textChanged, this, [this](const QString & param) { squadronWingChanged(param, 4); });

	// Dogfight wings
	connect(ui->dogfightWing_1, &QLineEdit::textChanged, this, [this](const QString & param) { dogfightWingChanged(param, 0); });
	connect(ui->dogfightWing_2, &QLineEdit::textChanged, this, [this](const QString & param) { dogfightWingChanged(param, 1); });

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

CustomWingNamesDialog::~CustomWingNamesDialog() {
}

void CustomWingNamesDialog::closeEvent(QCloseEvent * event) {
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

void CustomWingNamesDialog::updateUI() {
	// Update starting wings
	ui->startingWing_1->setText(_model->getStartingWing(0).c_str());
	ui->startingWing_2->setText(_model->getStartingWing(1).c_str());
	ui->startingWing_3->setText(_model->getStartingWing(2).c_str());

	// Update squadron wings
	ui->squadronWing_1->setText(_model->getSquadronWing(0).c_str());
	ui->squadronWing_2->setText(_model->getSquadronWing(1).c_str());
	ui->squadronWing_3->setText(_model->getSquadronWing(2).c_str());
	ui->squadronWing_4->setText(_model->getSquadronWing(3).c_str());
	ui->squadronWing_5->setText(_model->getSquadronWing(4).c_str());

	// Update dogfight wings
	ui->dogfightWing_1->setText(_model->getTvTWing(0).c_str());
	ui->dogfightWing_2->setText(_model->getTvTWing(1).c_str());
}

void CustomWingNamesDialog::startingWingChanged(const QString & str, int index) {
	_model->setStartingWing(str.toStdString(), index);
}

void CustomWingNamesDialog::squadronWingChanged(const QString & str, int index) {
	_model->setSquadronWing(str.toStdString(), index);
}

void CustomWingNamesDialog::dogfightWingChanged(const QString & str, int index) {
	_model->setTvTWing(str.toStdString(), index);
}

}
}
}
