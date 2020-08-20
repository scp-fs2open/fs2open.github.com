#include "ShipEditorDialog.h"

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

ShipEditorDialog::ShipEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipEditorDialog()), _model(new ShipEditorDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipEditorDialog::updateUI);

	// Column One
	connect(ui->shipNameEdit,
		static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
		this,
		&ShipEditorDialog::shipNameChanged);

	connect(ui->shipClassCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::shipClassChanged);
	connect(ui->AIClassCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::aiClassChanged);
	connect(ui->teamCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::teamChanged);
	connect(ui->cargoCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::cargoChanged);
	connect(ui->altNameCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::altNameChanged);
	connect(ui->callsignCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::callsignChanged);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipEditorDialog::~ShipEditorDialog() {}

void ShipEditorDialog::closeEvent(QCloseEvent* event) { QDialog::closeEvent(event); }

void ShipEditorDialog::updateUI()
{
	util::SignalBlockers blockers(this);

	updateColumnOne();
}
void ShipEditorDialog::updateColumnOne()
{
	ui->shipNameEdit->setText(_model->getShipName().c_str());
	int i;
	auto idx = _model->getShipClass();
	ui->shipClassCombo->clear();
	for (i = 0; i < Ship_info.size(); i++) {
		ui->shipClassCombo->addItem(Ship_info[i].name, QVariant(i));
	}
	ui->shipClassCombo->setCurrentIndex(ui->shipClassCombo->findData(idx));

	auto ai = _model->getAIClass();
	ui->AIClassCombo->clear();
	for (i = 0; i < Num_ai_classes; i++) {
		ui->AIClassCombo->addItem(Ai_class_names[i], QVariant(i));
	}
	ui->AIClassCombo->setCurrentIndex(ui->AIClassCombo->findData(ai));

	if (_model->player_count) {
		if (_model->mission_type) {
			ui->teamCombo->setEnabled(true);
		} else {
			ui->teamCombo->setEnabled(false);
			_model->setTeam(-1);
		}

		ui->teamCombo->clear();
		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(i));
		}
	} else {
		auto idx = _model->getTeam();
		ui->teamCombo->setEnabled(true);
		ui->teamCombo->clear();
		for (i = 0; i < Num_iffs; i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(i));
		}
		ui->teamCombo->setCurrentIndex(ui->teamCombo->findData(idx));
	}

	auto cargo = _model->getCargo();
	ui->cargoCombo->clear();
	for (i = 0; i < Num_cargo; i++) {
		ui->cargoCombo->addItem(Cargo_names[i], QVariant(i));
	}
	ui->cargoCombo->setCurrentIndex(ui->cargoCombo->findData(QString(cargo.c_str())));

	if (_model->multi_edit) {
		ui->altNameCombo->setEnabled(false);
	} else {
		ui->altNameCombo->setEnabled(true);
		ui->altNameCombo->addItem("<none>");
		int sel_idx = (_model->base_ship < 0 || !strlen(Fred_alt_names[_model->base_ship])) ? -1 : -2;
		for (i = 0; i < Mission_alt_type_count; i++) {
			ui->altNameCombo->addItem(Mission_alt_types[i], QVariant(Mission_alt_types[i]));
			if (sel_idx == -2 && !strcmp(Mission_alt_types[i], Fred_alt_names[_model->base_ship])) {
				sel_idx = i;
			}
			sel_idx = i;
		}
		Assertion(sel_idx >= -1, "Alt name exists but can't be found in Mission_alt_types; get a coder!\n");

		sel_idx += 1;
		ui->altNameCombo->setCurrentIndex(sel_idx);
	}

	if (_model->multi_edit) {
		ui->callsignCombo->setEnabled(false);
	} else {
		ui->callsignCombo->setEnabled(true);
		ui->callsignCombo->addItem("<none>");
		int sel_idx = (_model->base_ship < 0 || !strlen(Fred_callsigns[_model->base_ship])) ? -1 : -2;
		for (i = 0; i < Mission_callsign_count; i++) {
			ui->callsignCombo->addItem(Mission_callsigns[i], QVariant(Mission_callsigns[i]));
			if (sel_idx == -2 && !strcmp(Mission_callsigns[i], Fred_callsigns[_model->base_ship])) {
				sel_idx = i;
			}
			sel_idx = i;
		}
		Assertion(sel_idx >= -1, "Callsign exists but can't be found in Mission_callsigns; get a coder!\n");

		sel_idx += 1;
		ui->callsignCombo->setCurrentIndex(sel_idx);
	}
}
void ShipEditorDialog::shipNameChanged(const QString& string) { _model->setShipName(string.toStdString()); }
void ShipEditorDialog::shipClassChanged(int index)
{
	auto shipClassIdx = ui->shipClassCombo->itemData(index).value<int>();
	_model->setShipClass(shipClassIdx);
}
void ShipEditorDialog::aiClassChanged(int index)
{
	auto aiClassIdx = ui->AIClassCombo->itemData(index).value<int>();
	_model->setAIClass(aiClassIdx);
}
void ShipEditorDialog::teamChanged(int index)
{
	auto teamIdx = ui->teamCombo->itemData(index).value<int>();
	_model->setTeam(teamIdx);
}
void ShipEditorDialog::cargoChanged(int index)
{
	auto cargoIdx = ui->cargoCombo->itemData(index).value<QString>();
	_model->setCargo(cargoIdx.toStdString());
}
void ShipEditorDialog::altNameChanged(int index)
{
	auto altNameIdx = ui->altNameCombo->itemData(index).value<QString>();
	_model->setAltName(altNameIdx.toStdString());
}
void ShipEditorDialog::callsignChanged(int index)
{
	auto callsignIdx = ui->callsignCombo->itemData(index).value<QString>();
	_model->setCallsign(callsignIdx.toStdString());
}
} // namespace dialogs
} // namespace fred
} // namespace fso