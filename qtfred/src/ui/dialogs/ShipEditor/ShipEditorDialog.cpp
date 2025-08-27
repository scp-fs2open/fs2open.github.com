#include "ShipEditorDialog.h"

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "mission/object.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {

ShipEditorDialog::ShipEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipEditorDialog()), _model(new ShipEditorDialogModel(this, viewport)),
	  _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this] { updateUI(false); });
	connect(this, &QDialog::accepted, _model.get(), &ShipEditorDialogModel::apply);
	connect(viewport->editor, &Editor::currentObjectChanged, this, &ShipEditorDialog::update);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &ShipEditorDialog::update);

	// Column One

	connect(ui->cargoCombo->lineEdit(), (&QLineEdit::editingFinished), this, &ShipEditorDialog::cargoChanged);
	connect(ui->altNameCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::altNameChanged);
	connect(ui->callsignCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::callsignChanged);

	// ui->cargoCombo->installEventFilter(this);

	updateUI(true);

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipEditorDialog::~ShipEditorDialog() = default;

int ShipEditorDialog::getShipClass() const
{
	return _model->getShipClass();
}

int ShipEditorDialog::getSingleShip() const
{
	return _model->getSingleShip();
}

bool ShipEditorDialog::getIfMultipleShips() const
{
	return _model->getIfMultipleShips();
}

void ShipEditorDialog::closeEvent(QCloseEvent* e)
{
	util::SignalBlockers blockers(this);
	_model->apply();
	QDialog::closeEvent(e);
}

void ShipEditorDialog::hideEvent(QHideEvent* e)
{
	QDialog::hideEvent(e);
}
void ShipEditorDialog::showEvent(QShowEvent* e)
{
	_model->initializeData();
	QDialog::showEvent(e);
}

void ShipEditorDialog::on_miscButton_clicked()
{
	auto dialog = new dialogs::ShipFlagsDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::on_initialStatusButton_clicked()
{
	auto dialog = new dialogs::ShipInitialStatusDialog(this, _viewport, getIfMultipleShips());
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::on_initialOrdersButton_clicked()
{
	auto dialog =
		new dialogs::ShipGoalsDialog(this, _viewport, getIfMultipleShips(), Ships[getSingleShip()].objnum, -1);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::on_tblInfoButton_clicked()
{
	auto dialog = new dialogs::ShipTBLViewer(this, _viewport, getShipClass());
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::update()
{
	if (this->isVisible()) {
		if (_model->getNumSelectedObjects() && _model->query_modified()) {
			_model->apply();
		}
		_model->initializeData();
		updateUI(true);
	}
}

void ShipEditorDialog::updateUI(bool overwrite)
{
	util::SignalBlockers blockers(this);
	enableDisable();
	updateColumnOne(overwrite);
	updateColumnTwo(overwrite);
	updateArrival(overwrite);
	updateDeparture(overwrite);
}
void ShipEditorDialog::updateColumnOne(bool overwrite)
{
	util::SignalBlockers blockers(this);
	int idx;
	if (overwrite) {
		ui->shipNameEdit->setText(_model->getShipName().c_str());
		ui->shipDisplayNameEdit->setText(_model->getShipDisplayName().c_str());
		idx = _model->getShipClass();
		ui->shipClassCombo->clear();
		for (size_t i = 0; i < Ship_info.size(); i++) {
			ui->shipClassCombo->addItem(Ship_info[i].name, QVariant(static_cast<int>(i)));
		}
		ui->shipClassCombo->setCurrentIndex(ui->shipClassCombo->findData(idx));

		auto ai = _model->getAIClass();
		ui->AIClassCombo->clear();
		for (auto j = 0; j < Num_ai_classes; j++) {
			ui->AIClassCombo->addItem(Ai_class_names[j], QVariant(j));
		}
		ui->AIClassCombo->setCurrentIndex(ui->AIClassCombo->findData(ai));
	}
	if (_model->getNumSelectedPlayers()) {
		if (_model->getTeam() != -1) {
			ui->teamCombo->setEnabled(true);
		} else {
			ui->teamCombo->setEnabled(false);
		}
		if (overwrite) {
			ui->teamCombo->clear();
			for (auto i = 0; i < MAX_TVT_TEAMS; i++) {
				ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(static_cast<int>(i)));
			}
		}
	} else {
		ui->teamCombo->setEnabled(_model->getUIEnable());
		if (overwrite) {
			idx = _model->getTeam();
			ui->teamCombo->clear();
			for (size_t i = 0; i < Iff_info.size(); i++) {
				ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(static_cast<int>(i)));
			}
			ui->teamCombo->setCurrentIndex(ui->teamCombo->findData(idx));
		}
	}
	if (overwrite) {
		auto cargo = _model->getCargo();
		ui->cargoCombo->clear();
		int j;
		for (j = 0; j < Num_cargo; j++) {
			ui->cargoCombo->addItem(Cargo_names[j]);
		}
		if (ui->cargoCombo->findText(QString(cargo.c_str()))) {
			ui->cargoCombo->setCurrentIndex(ui->cargoCombo->findText(QString(cargo.c_str())));
		} else {
			ui->cargoCombo->addItem(cargo.c_str());

			ui->cargoCombo->setCurrentIndex(ui->cargoCombo->findText(QString(cargo.c_str())));
		}
	}
	if (_model->getNumSelectedObjects()) {
		if (_model->getIfMultipleShips()) {
			ui->altNameCombo->setEnabled(false);
		} else {
			auto altname = _model->getAltName();
			ui->altNameCombo->setEnabled(true);
			if (overwrite) {
				ui->altNameCombo->clear();
				ui->altNameCombo->addItem("<none>");
				for (auto j = 0; j < Mission_alt_type_count; j++) {
					ui->altNameCombo->addItem(Mission_alt_types[j]);
				}
				if (ui->altNameCombo->findText(QString(altname.c_str()))) {
					ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText(QString(altname.c_str())));
				} else {
					ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText("<none>"));
				}
			}
		}
	}
	if (_model->getNumSelectedObjects()) {
		if (_model->getIfMultipleShips()) {
			ui->callsignCombo->setEnabled(false);
		} else {
			ui->callsignCombo->clear();
			auto callsign = _model->getCallsign();
			ui->callsignCombo->setEnabled(true);
			if (overwrite) {
				ui->callsignCombo->addItem("<none>");
				for (auto j = 0; j < Mission_callsign_count; j++) {
					ui->callsignCombo->addItem(Mission_callsigns[j], QVariant(Mission_callsigns[j]));
				}

				if (ui->callsignCombo->findText(QString(callsign.c_str()))) {
					ui->callsignCombo->setCurrentIndex(ui->callsignCombo->findText(QString(callsign.c_str())));
				} else {
					ui->altNameCombo->setCurrentIndex(ui->callsignCombo->findText("<none>"));
				}
			}
		}
	}
}
void ShipEditorDialog::updateColumnTwo(bool overwrite)
{
	util::SignalBlockers blockers(this);
	if (overwrite) {
		ui->wing->setText(_model->getWing().c_str());

		auto idx = _model->getPersona();
		ui->personaCombo->setCurrentIndex(ui->personaCombo->findData(idx));

		ui->killScoreEdit->setValue(_model->getScore());

		ui->assistEdit->setValue(_model->getAssist());

		ui->playerShipCheckBox->setChecked(_model->getPlayer());
		ui->respawnSpinBox->setValue(_model->getRespawn());
	}
}
void ShipEditorDialog::updateArrival(bool overwrite)
{
	util::SignalBlockers blockers(this);
	if (overwrite) {
		auto idx = _model->getArrivalLocationIndex();
		int i;
		ui->arrivalLocationCombo->clear();
		for (i = 0; i < MAX_ARRIVAL_NAMES; i++) {
			ui->arrivalLocationCombo->addItem(Arrival_location_names[i], QVariant(i));
		}
		ui->arrivalLocationCombo->setCurrentIndex(ui->arrivalLocationCombo->findData(idx));
	}
	object* objp;
	int restrict_to_players;
	ui->arrivalTargetCombo->clear();
	if (_model->getArrivalLocation() != ArrivalLocation::FROM_DOCK_BAY) {
		// Add Special Arrivals
		for (restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++) {
			for (size_t j = 0; j < Iff_info.size(); j++) {
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, static_cast<int>(j), restrict_to_players, 0);

				ui->arrivalTargetCombo->addItem(tmp, QVariant(get_special_anchor(tmp)));
			}
		}
		// Add All Ships
		for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				!(objp->flags[Object::Object_Flags::Marked])) {
				auto ship = get_ship_from_obj(objp);
				ui->arrivalTargetCombo->addItem(Ships[ship].ship_name, QVariant(ship));
			}
		}
	} else {
		for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				!(objp->flags[Object::Object_Flags::Marked])) {
				polymodel* pm;

				// determine if this ship has a docking bay
				pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
				Assert(pm);
				if (pm->ship_bay && (pm->ship_bay->num_paths > 0)) {
					auto ship = get_ship_from_obj(objp);
					ui->arrivalTargetCombo->addItem(Ships[ship].ship_name, QVariant(ship));
				}
			}
		}
	}
	ui->arrivalTargetCombo->setCurrentIndex(ui->arrivalTargetCombo->findData(_model->getArrivalTarget()));
	if (overwrite) {
		ui->arrivalDistanceEdit->clear();
		ui->arrivalDistanceEdit->setValue(_model->getArrivalDistance());
		ui->arrivalDelaySpinBox->setValue(_model->getArrivalDelay());

		ui->updateArrivalCueCheckBox->setChecked(_model->getArrivalCue());

		ui->arrivalTree->initializeEditor(_viewport->editor, this);
		if (_model->getNumSelectedShips()) {

			if (_model->getIfMultipleShips()) {
				ui->arrivalTree->clear_tree("");
			}
			if (_model->getUseCue()) {
				ui->arrivalTree->load_tree(_model->getArrivalFormula());
			} else {
				ui->arrivalTree->clear_tree("");
			}
			if (!_model->getIfMultipleShips()) {
				int j = ui->arrivalTree->select_sexp_node;
				if (j != -1) {
					ui->arrivalTree->hilite_item(j);
				}
			}
		} else {
			ui->arrivalTree->clear_tree("");
		}

		ui->noArrivalWarpCheckBox->setChecked(_model->getNoArrivalWarp());
	}
}
void ShipEditorDialog::updateDeparture(bool overwrite)
{
	util::SignalBlockers blockers(this);
	if (overwrite) {
		auto idx = _model->getDepartureLocationIndex();
		int i;
		ui->departureLocationCombo->clear();
		for (i = 0; i < MAX_DEPARTURE_NAMES; i++) {
			ui->departureLocationCombo->addItem(Departure_location_names[i], QVariant(i));
		}
		ui->departureLocationCombo->setCurrentIndex(ui->departureLocationCombo->findData(idx));
	}
	object* objp;

	ui->departureTargetCombo->clear();
	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && !(objp->flags[Object::Object_Flags::Marked])) {
			polymodel* pm;

			// determine if this ship has a docking bay
			pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
			Assert(pm);
			if (pm->ship_bay && (pm->ship_bay->num_paths > 0)) {
				auto ship = get_ship_from_obj(objp);
				ui->departureTargetCombo->addItem(Ships[ship].ship_name, QVariant(ship));
			}
		}
	}
	ui->departureTargetCombo->setCurrentIndex(ui->departureTargetCombo->findData(_model->getDepartureTarget()));
	if (overwrite) {
		ui->departureDelaySpinBox->setValue(_model->getDepartureDelay());

		ui->departureTree->initializeEditor(_viewport->editor, this);
		if (_model->getNumSelectedShips()) {

			if (_model->getIfMultipleShips()) {
				ui->departureTree->clear_tree("");
			}
			if (_model->getUseCue()) {
				ui->departureTree->load_tree(_model->getDepartureFormula(), "false");
			} else {
				ui->departureTree->clear_tree("");
			}
			if (!_model->getIfMultipleShips()) {
				auto i = ui->arrivalTree->select_sexp_node;
				if (i != -1) {
					i = ui->departureTree->select_sexp_node;
					ui->departureTree->hilite_item(i);
				}
			}
		} else {
			ui->departureTree->clear_tree("");
		}

		ui->noDepartureWarpCheckBox->setChecked(_model->getNoDepartureWarp());

		ui->updateDepartureCueCheckBox->setChecked(_model->getDepartureCue());
	}
}
// Enables disbales controls based on what is selected
void ShipEditorDialog::enableDisable()
{
	if (!_model->getUseCue()) {
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
		ui->arrivalLocationCombo->setEnabled(_model->getUIEnable());
		if (_model->getArrivalLocationIndex()) {
			ui->arrivalDistanceEdit->setEnabled(_model->getUIEnable());
			ui->arrivalTargetCombo->setEnabled(_model->getUIEnable());
		} else {
			ui->arrivalDistanceEdit->setEnabled(false);
			ui->arrivalTargetCombo->setEnabled(false);
		}
		if (_model->getArrivalLocation() == ArrivalLocation::FROM_DOCK_BAY) {
			if (_model->getArrivalTarget() >= 0) {
				ui->restrictArrivalPathsButton->setEnabled(_model->getUIEnable());
			} else {
				ui->restrictArrivalPathsButton->setEnabled(false);
			}
			ui->customWarpinButton->setEnabled(false);
		} else {
			ui->restrictArrivalPathsButton->setEnabled(false);
			ui->customWarpinButton->setEnabled(_model->getUIEnable());
		}

		ui->departureLocationCombo->setEnabled(_model->getUIEnable());
		if (_model->getDepartureLocationIndex()) {
			ui->departureTargetCombo->setEnabled(_model->getUIEnable());
		} else {
			ui->departureTargetCombo->setEnabled(false);
		}
		if (_model->getDepartureLocation() == DepartureLocation::TO_DOCK_BAY) {
			if (_model->getDepartureTarget() >= 0) {
				ui->restrictDeparturePathsButton->setEnabled(_model->getUIEnable());
			} else {
				ui->restrictDeparturePathsButton->setEnabled(false);
			}
			ui->customWarpoutButton->setEnabled(false);
		} else {
			ui->restrictDeparturePathsButton->setEnabled(false);
			ui->customWarpoutButton->setEnabled(_model->getUIEnable());
		}

		ui->arrivalDelaySpinBox->setEnabled(_model->getUIEnable());
		ui->arrivalTree->setEnabled(_model->getUIEnable());
		ui->departureDelaySpinBox->setEnabled(_model->getUIEnable());
		ui->departureTree->setEnabled(_model->getUIEnable());
		ui->noArrivalWarpCheckBox->setEnabled(_model->getUIEnable());
		ui->noDepartureWarpCheckBox->setEnabled(_model->getUIEnable());
	}

	if (_model->getNumSelectedObjects()) {
		ui->shipNameEdit->setEnabled(!_model->getIfMultipleShips());
		ui->shipDisplayNameEdit->setEnabled(!_model->getIfMultipleShips());
		ui->shipClassCombo->setEnabled(true);
		ui->altNameCombo->setEnabled(true);
		ui->initialStatusButton->setEnabled(true);
		ui->weaponsButton->setEnabled(_model->getShipClass() >= 0);
		ui->miscButton->setEnabled(true);
		ui->textureReplacementButton->setEnabled(true);
		ui->altShipClassButton->setEnabled(true);
		ui->specialStatsButton->setEnabled(true);
	} else {
		ui->shipNameEdit->setEnabled(false);
		ui->shipDisplayNameEdit->setEnabled(false);
		ui->shipClassCombo->setEnabled(false);
		ui->altNameCombo->setEnabled(false);
		ui->initialStatusButton->setEnabled(false);
		ui->weaponsButton->setEnabled(false);
		ui->miscButton->setEnabled(false);
		ui->textureReplacementButton->setEnabled(false);
		ui->altShipClassButton->setEnabled(false);
		ui->specialStatsButton->setEnabled(false);
	}

	// disable textures for multiple ships
	ui->textureReplacementButton->setEnabled(_model->getTexEditEnable());

	ui->AIClassCombo->setEnabled(_model->getUIEnable());
	ui->cargoCombo->setEnabled(_model->getUIEnable());
	ui->hotkeyCombo->setEnabled(_model->getUIEnable());
	if ((_model->getShipClass() >= 0) && !(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::Cargo]) &&
		!(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::No_ship_type]))
		ui->initialOrdersButton->setEnabled(_model->getUIEnable());
	else if (_model->getIfMultipleShips())
		ui->initialOrdersButton->setEnabled(_model->getUIEnable());
	else
		ui->initialOrdersButton->setEnabled(false);

	// !pship_count used because if allowed to clear, we would have no player starts
	// mission_type 0 = multi, 1 = single
	if (!(The_mission.game_type & MISSION_TYPE_MULTI) || !_model->getNumUnmarkedPlayers() ||
		(_model->getNumUnmarkedPlayers() + _model->getNumSelectedObjects() > MAX_PLAYERS) ||
		(_model->getNumValidPlayers() < _model->getNumSelectedObjects()))
		ui->playerShipCheckBox->setEnabled(false);
	else
		ui->playerShipCheckBox->setEnabled(true);
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		ui->respawnSpinBox->setEnabled(_model->getUIEnable());
	} else {
		ui->respawnSpinBox->setEnabled(false);
	}

	// show the "set player" button only if single player
	if (!(The_mission.game_type & MISSION_TYPE_MULTI))
		ui->playerShipButton->setVisible(true);
	else
		ui->playerShipButton->setVisible(false);

	// enable the "set player" button only if single player, single edit, and ship is in player wing
	{
		int marked_ship = (_model->getIfPlayerShip() >= 0) ? _model->getIfPlayerShip() : _model->getSingleShip();
		const bool isPlayerWing = _model->wing_is_player_wing(Ships[marked_ship].wingnum);
		if (!(The_mission.game_type & MISSION_TYPE_MULTI) && (_model->getNumSelectedObjects() > 0) &&
			(_model->getIfMultipleShips() != true) && (isPlayerWing == true))
			ui->playerShipButton->setEnabled(true);
		else
			ui->playerShipButton->setEnabled(false);
	}

	ui->deleteButton->setEnabled(_model->getUIEnable());
	ui->resetButton->setEnabled(_model->getUIEnable());
	ui->killScoreEdit->setEnabled(_model->getUIEnable());
	ui->assistEdit->setEnabled(_model->getUIEnable());

	ui->tblInfoButton->setEnabled(_model->getShipClass() >= 0);

	if (_model->getUseCue() > 1) {
		ui->updateArrivalCueCheckBox->setVisible(true);
		ui->updateDepartureCueCheckBox->setVisible(true);
	} else {
		ui->updateArrivalCueCheckBox->setVisible(false);
		ui->updateDepartureCueCheckBox->setVisible(false);
	}

	if (_model->getIfMultipleShips() || (_model->getNumSelectedObjects() > 1)) {
		// we will allow the ignore orders dialog to be multi edit if all selected
		// ships are the same type.  the ship_type variable holds the ship types
		// for all ships.  Determine how may bits set and enable/diable window
		// as appropriate
		if (_model->getShipOrders().find(std::numeric_limits<size_t>::max()) != _model->getShipOrders().end()) {
			ui->playerOrdersButton->setEnabled(false);
		} else {
			ui->playerOrdersButton->setEnabled(true);
		}
	} else {
		// always enabled when one ship is selected
		ui->playerOrdersButton->setEnabled(_model->getUIEnable());
	}

	// always enabled if >= 1 ship selected
	ui->personaCombo->setEnabled(_model->getUIEnable());

	if (_model->getIfMultipleShips()) {
		this->setWindowTitle("Edit Marked Ships");
	} else if (_model->getNumSelectedPlayers()) {
		this->setWindowTitle("Edit Player Ship");
	} else {
		this->setWindowTitle("Edit Ship");
	}
}
void ShipEditorDialog::cargoChanged()
{
	const QString entry = ui->cargoCombo->lineEdit()->text();
	if (!entry.isEmpty() && entry != _model->getCargo().c_str()) {
		const auto textBytes = entry.toUtf8();
		const SCP_string NewCargo = textBytes.toStdString();
		_model->setCargo(NewCargo);
	}
}
void ShipEditorDialog::altNameChanged()
{
	const QString entry = ui->altNameCombo->lineEdit()->text();
	if (!entry.isEmpty() && entry != _model->getAltName().c_str()) {
		const auto textBytes = entry.toUtf8();
		const SCP_string NewAltname = textBytes.toStdString();
		_model->setAltName(NewAltname);
	}
}
void ShipEditorDialog::callsignChanged()
{
	const QString entry = ui->callsignCombo->lineEdit()->text();
	if (!entry.isEmpty() && entry != _model->getCallsign().c_str()) {
		const auto textBytes = entry.toUtf8();
		const SCP_string newCallsign = textBytes.toStdString();
		_model->setCallsign(newCallsign);
	}
}

void ShipEditorDialog::on_textureReplacementButton_clicked()
{
	auto dialog = new dialogs::ShipTextureReplacementDialog(this, _viewport, getIfMultipleShips());
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::on_playerShipButton_clicked()
{
	_model->setPlayer(true);
}
void ShipEditorDialog::on_altShipClassButton_clicked()
{
	auto dialog = new dialogs::ShipAltShipClass(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_prevButton_clicked()
{
	_model->OnPrevious();
}
void ShipEditorDialog::on_nextButton_clicked()
{
	_model->OnNext();
}
void ShipEditorDialog::on_resetButton_clicked()
{
	_model->OnShipReset();
}
void ShipEditorDialog::on_deleteButton_clicked()
{
	_model->OnDeleteShip();
}
void ShipEditorDialog::on_weaponsButton_clicked()
{
	auto dialog = new dialogs::ShipWeaponsDialog(this, _viewport, getIfMultipleShips());
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_playerOrdersButton_clicked()
{
	auto dialog = new dialogs::PlayerOrdersDialog(this, _viewport, getIfMultipleShips());
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_specialStatsButton_clicked()
{
	auto dialog = new dialogs::ShipSpecialStatsDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_hideCuesButton_clicked()
{
	if (ui->hideCuesButton->isChecked()) {
		ui->arrivalGroupBox->setVisible(false);
		ui->departureGroupBox->setVisible(false);
		ui->HelpTitle->setVisible(false);
		ui->helpText->setVisible(false);
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		resize(sizeHint());
	} else {
		ui->arrivalGroupBox->setVisible(true);
		ui->departureGroupBox->setVisible(true);
		ui->HelpTitle->setVisible(true);
		ui->helpText->setVisible(true);
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		resize(sizeHint());
	}
}
void ShipEditorDialog::on_restrictArrivalPathsButton_clicked()
{
	int target_class = Ships[_model->getArrivalTarget()].ship_info_index;
	auto dialog = new dialogs::ShipPathsDialog(this, _viewport, _model->getSingleShip(), target_class, false);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_customWarpinButton_clicked()
{
	auto dialog = new dialogs::ShipCustomWarpDialog(this, _viewport, false);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_restrictDeparturePathsButton_clicked()
{
	int target_class = Ships[_model->getDepartureTarget()].ship_info_index;
	auto dialog = new dialogs::ShipPathsDialog(this, _viewport, _model->getSingleShip(), target_class, true);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void ShipEditorDialog::on_customWarpoutButton_clicked()
{
	auto dialog = new dialogs::ShipCustomWarpDialog(this, _viewport, true);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

/*---------------------------------------------------------
					WARNING
Do not try to optimise string entries; this convoluted method is necessary to avoid fatal errors caused by QT
-----------------------------------------------------------*/
void ShipEditorDialog::on_shipNameEdit_editingFinished()
{
	const QString entry = ui->shipNameEdit->text();
	if (!entry.isEmpty() && entry != _model->getShipName().c_str()) {
		const auto textBytes = entry.toUtf8();
		const std::string NewShipName = textBytes.toStdString();
		_model->setShipName(NewShipName);
	}

	// automatically determine or reset the display name
	_model->setShipDisplayName(Editor::get_display_name_for_text_box(_model->getShipName()));

	// sync the variable to the edit box
	ui->shipDisplayNameEdit->setText(_model->getShipDisplayName().c_str());
}
void ShipEditorDialog::on_shipDisplayNameEdit_editingFinished()
{
	const QString entry = ui->shipDisplayNameEdit->text();
	if (entry != _model->getShipDisplayName().c_str()) {
		const auto textBytes = entry.toUtf8();
		const std::string NewShipDisplayName = textBytes.toStdString();
		_model->setShipDisplayName(NewShipDisplayName);
	}
}
void ShipEditorDialog::on_shipClassCombo_currentIndexChanged(int index)
{
	auto shipClassIdx = ui->shipClassCombo->itemData(index).toInt();
	_model->setShipClass(shipClassIdx);
}
void ShipEditorDialog::on_AIClassCombo_currentIndexChanged(int index)
{
	auto aiClassIdx = ui->AIClassCombo->itemData(index).toInt();
	_model->setAIClass(aiClassIdx);
}
void ShipEditorDialog::on_teamCombo_currentIndexChanged(int index)
{
	auto teamIdx = ui->teamCombo->itemData(index).toInt();
	_model->setTeam(teamIdx);
}
void ShipEditorDialog::on_hotkeyCombo_currentIndexChanged(int index)
{
	auto hotkeyIdx = ui->hotkeyCombo->itemData(index).toInt();
	_model->setHotkey(hotkeyIdx);
}
void ShipEditorDialog::on_personaCombo_currentIndexChanged(int index)
{
	auto personaIdx = ui->personaCombo->itemData(index).toInt();
	_model->setPersona(personaIdx);
}
void ShipEditorDialog::on_killScoreEdit_valueChanged(int value)
{
	_model->setScore(value);
}
void ShipEditorDialog::on_assistEdit_valueChanged(int value)
{
	_model->setAssist(value);
}
void ShipEditorDialog::on_playerShipCheckBox_toggled(bool value)
{
	_model->setPlayer(value);
}
void ShipEditorDialog::on_respawnSpinBox_valueChanged(int value) {
	_model->setRespawn(value);
}
void ShipEditorDialog::on_arrivalLocationCombo_currentIndexChanged(int index)
{
	auto arrivalLocationIdx = ui->arrivalLocationCombo->itemData(index).toInt();
	_model->setArrivalLocationIndex(arrivalLocationIdx);
}
void ShipEditorDialog::on_arrivalTargetCombo_currentIndexChanged(int index)
{
	auto arrivalLocationIdx = ui->arrivalTargetCombo->itemData(index).toInt();
	_model->setArrivalTarget(arrivalLocationIdx);
}
void ShipEditorDialog::on_arrivalDistanceEdit_valueChanged(int value)
{
	_model->setArrivalDistance(value);
}
void ShipEditorDialog::on_arrivalDelaySpinBox_valueChanged(int value)
{
	_model->setArrivalDelay(value);
}
void ShipEditorDialog::on_updateArrivalCueCheckBox_toggled(bool value)
{
	_model->setArrivalCue(value);
}
void ShipEditorDialog::on_noArrivalWarpCheckBox_toggled(bool value)
{
	_model->setNoArrivalWarp(value);
}
void ShipEditorDialog::on_arrivalTree_rootNodeFormulaChanged(int old, int node)
{
	_model->setArrivalFormula(old, node);
}
void ShipEditorDialog::on_arrivalTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}
void ShipEditorDialog::on_arrivalTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}
void ShipEditorDialog::on_departureLocationCombo_currentIndexChanged(int index)
{
	auto depLocationIdx = ui->departureLocationCombo->itemData(index).toInt();
	_model->setDepartureLocationIndex(depLocationIdx);
}
void fred::dialogs::ShipEditorDialog::on_departureTargetCombo_currentIndexChanged(int index)
{
	auto depLocationIdx = ui->departureTargetCombo->itemData(index).toInt();
	_model->setDepartureTarget(depLocationIdx);
}
void fred::dialogs::ShipEditorDialog::on_departureDelaySpinBox_valueChanged(int value)
{
	_model->setDepartureDelay(value);
}
void fred::dialogs::ShipEditorDialog::on_updateDepartureCueCheckBox_toggled(bool value)
{
	_model->setDepartureCue(value);
}
void fred::dialogs::ShipEditorDialog::on_departureTree_rootNodeFormulaChanged(int old, int node)
{
	_model->setDepartureFormula(old, node);
}
void fred::dialogs::ShipEditorDialog::on_departureTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}
void fred::dialogs::ShipEditorDialog::on_departureTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}
void fred::dialogs::ShipEditorDialog::on_noDepartureWarpCheckBox_toggled(bool value)
{
	_model->setNoDepartureWarp(value);
}
} // namespace fso::fred::dialogs