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
	enableDisable();
	updateColumnOne();
	updateColumnTwo();
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
		if (The_mission.game_type & MISSION_TYPE_MULTI) {
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
void ShipEditorDialog::updateColumnTwo() { ui->wing->SetText(_model->getWing()); }
// Enables disbales controls based on what is selected
void ShipEditorDialog::enableDisable()
{
	if (!_model->cue_init) {
		ui->arrivalLocationCombo->setEnabled(false);
		ui->arrivalDelaySpinBox->setEnabled(false);
		ui->arrivalDistanceEdit->setEnabled(false);
		ui->arrivalTargetCombo->setEnabled(false);
		ui->arrivalTree->setEnabled(false);

		ui->departureLocationCombo->setEnabled(false);
		ui->departureDelaySpinBox->setEnabled(false);
		ui->departureTargetCombo->setEnabled(false);
		ui->departureTree->setEnabled(false);

		ui->noArrivalWarpCheckBox->setEnabled(false);
		ui->noDepartureWarpCheckBox->setEnabled(false);

		ui->restrictArrivalPathsButton->setEnabled(false);
		ui->restrictDeparturePathsButton->setEnabled(false);
	} else {
		ui->arrivalLocationCombo->setEnabled(true);
		if (_model->getArrivalLocation()) {
			ui->arrivalDistanceEdit->setEnabled(true);
			ui->arrivalTargetCombo->setEnabled(true);
		} else {
			ui->arrivalDistanceEdit->setEnabled(false);
			ui->arrivalTargetCombo->setEnabled(false);
		}
		if (_model->getArrivalLocation() == ARRIVE_FROM_DOCK_BAY) {
			ui->restrictArrivalPathsButton->setEnabled(true);
			ui->customWarpinButton->setEnabled(false);
		} else {
			ui->restrictArrivalPathsButton->setEnabled(false);
			ui->customWarpinButton->setEnabled(true);
		}

		ui->departureLocationCombo->setEnabled(true);
		if (_model->getDepartureLocation()) {
			ui->departureTargetCombo->setEnabled(true);
		} else {
			ui->departureTargetCombo->setEnabled(false);
		}
		if (_model->getDepartureLocation() == DEPART_AT_DOCK_BAY) {
			ui->restrictDeparturePathsButton->setEnabled(true);
			ui->customWarpoutButton->setEnabled(false);
		} else {
			ui->restrictDeparturePathsButton->setEnabled(false);
			ui->customWarpoutButton->setEnabled(true);
		}

		ui->arrivalDelaySpinBox->setEnabled(true);
		ui->arrivalTree->setEnabled(true);
		ui->departureDelaySpinBox->setEnabled(true);
		ui->departureTree->setEnabled(true);
		ui->noArrivalWarpCheckBox->setEnabled(true);
		ui->noDepartureWarpCheckBox->setEnabled(true);
	}

	if (_model->total_count) {
		ui->shipNameEdit->setEnabled(!_model->multi_edit);
		ui->shipClassCombo->setEnabled(true);
		ui->altNameCombo->setEnabled(true);
		ui->initialStatusButton->setEnabled(true);
		ui->weaponsButton->setEnabled(_model->getShipClass() >= 0);
		ui->miscButton->setEnabled(true);
		ui->textureReplacementButton->setEnabled(true);
		ui->altShipClassButton->setEnabled(true);
		ui->specialExpButton->setEnabled(true);
		ui->specialHitsButton->setEnabled(true);
	} else {
		ui->shipNameEdit->setEnabled(false);
		ui->shipClassCombo->setEnabled(false);
		ui->altNameCombo->setEnabled(false);
		ui->initialStatusButton->setEnabled(false);
		ui->weaponsButton->setEnabled(false);
		ui->miscButton->setEnabled(false);
		ui->textureReplacementButton->setEnabled(false);
		ui->altShipClassButton->setEnabled(false);
		ui->specialExpButton->setEnabled(false);
		ui->specialHitsButton->setEnabled(false);
	}

	// disable textures for multiple ships
	if (_model->multi_edit) {
		ui->textureReplacementButton->setEnabled(false);
	}

	ui->AIClassCombo->setEnabled(true);
	ui->cargoCombo->setEnabled(true);
	ui->hotkeyCombo->setEnabled(true);
	if ((_model->getShipClass() >= 0) && !(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::Cargo]) &&
		!(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::No_ship_type]))
		ui->initialOrdersButton->setEnabled(true);
	else if (_model->multi_edit)
		ui->initialOrdersButton->setEnabled(true);
	else
		ui->initialOrdersButton->setEnabled(false);

	// !pship_count used because if allowed to clear, we would have no player starts
	// mission_type 0 = multi, 1 = single
	if (!(The_mission.game_type & MISSION_TYPE_MULTI) || !_model->pship_count ||
		(_model->pship_count + _model->total_count > MAX_PLAYERS) || (_model->pvalid_count < _model->total_count))
		ui->playerShipCheckBox->setEnabled(false);
	else
		ui->playerShipCheckBox->setEnabled(true);

	// show the "set player" button only if single player
	if (!(The_mission.game_type & MISSION_TYPE_MULTI))
		ui->playerShipButton->setVisible(true);
	else
		ui->playerShipButton->setVisible(false);

	// enable the "set player" button only if single player, single edit, and ship is in player wing
	{
		int marked_ship = (_model->player_ship >= 0) ? _model->player_ship : _model->single_ship;

		if (!(The_mission.game_type & MISSION_TYPE_MULTI) && _model->total_count && !_model->multi_edit &&
			_model->wing_is_player_wing(Ships[marked_ship].wingnum))
			ui->playerShipButton->setEnabled(true);
		else
			ui->playerShipButton->setEnabled(false);
	}

	ui->deleteButton->setEnabled(true);
	ui->resetButton->setEnabled(false);
	ui->killScoreEdit->setEnabled(true);
	ui->assistEdit->setEnabled(true);

	ui->tblInfoButton->setEnabled(_model->getShipClass() >= 0);

	if (_model->cue_init > 1) {
		ui->updateArrivalCueCheckBox->setVisible(true);
		ui->updateDepartureCueCheckBox->setVisible(true);
	} else {
		ui->updateArrivalCueCheckBox->setVisible(false);
		ui->updateDepartureCueCheckBox->setVisible(false);
	}

	if (_model->multi_edit || (_model->total_count > 1)) {
		// we will allow the ignore orders dialog to be multi edit if all selected
		// ships are the same type.  the ship_type variable holds the ship types
		// for all ships.  Determine how may bits set and enable/diable window
		// as appropriate
		if (_model->ship_orders == -1) {
			ui->playerOrdersButton->setEnabled(false);
		} else {
			ui->playerOrdersButton->setEnabled(true);
		}
	} else
		// always enabled when one ship is selected
		ui->playerOrdersButton->setEnabled(true);

	// always enabled if >= 1 ship selected
	ui->personaCombo->setEnabled(true);

	if (_model->multi_edit) {
		this->setWindowTitle("Edit Marked Ships");
	} else if (_model->player_count) {
		this->setWindowTitle("Edit Player Ship");
	} else {
		this->setWindowTitle("Edit Ship");
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