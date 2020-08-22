#include "ShipEditorDialog.h"

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"

#include <globalincs\linklist.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

ShipEditorDialog::ShipEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(), ui(new Ui::ShipEditorDialog()),
	  _model(new ShipEditorDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipEditorDialog::updateUI);
	connect(this, &QDialog::accepted, _model.get(), &ShipEditorDialogModel::apply);

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

	// Column Two
	connect(ui->hotkeyCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::hotkeyChanged);

	connect(ui->personaCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::personaChanged);

	connect(ui->killScoreEdit,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipEditorDialog::scoreChanged);

	connect(ui->assistEdit,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipEditorDialog::assistChanged);
	connect(ui->playerShipCheckBox, &QCheckBox::toggled, this, &ShipEditorDialog::playerChanged);

	// Arival Box
	connect(ui->arrivalLocationCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::arrivalLocationChanged);
	connect(ui->arrivalTargetCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::arrivalTargetChanged);
	connect(ui->arrivalDistanceEdit,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipEditorDialog::arrivalDistanceChanged);
	connect(ui->arrivalDelaySpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipEditorDialog::arrivalDelayChanged);

	connect(ui->arrivalTree, &sexp_tree::rootNodeFormulaChanged, this, [this](int old, int node) {
		_model->setArrivalFormula(old, node);
	});
	connect(ui->arrivalTree, &sexp_tree::helpChanged, this, [this](const QString& help) {
		ui->helpText->setPlainText(help);
	});
	connect(ui->arrivalTree, &sexp_tree::miniHelpChanged, this, [this](const QString& help) {
		ui->HelpTitle->setText(help);
	});

	connect(ui->noArrivalWarpCheckBox, &QCheckBox::toggled, this, &ShipEditorDialog::arrivalWarpChanged);

	// Departure Box
	connect(ui->departureLocationCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::departureLocationChanged);
	connect(ui->departureTargetCombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipEditorDialog::departureTargetChanged);
	connect(ui->departureDelaySpinBox,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		this,
		&ShipEditorDialog::departureDelayChanged);

	connect(ui->departureTree, &sexp_tree::rootNodeFormulaChanged, this, [this](int old, int node) {
		_model->setDepartureFormula(old, node);
	});
	connect(ui->departureTree, &sexp_tree::helpChanged, this, [this](const QString& help) {
		ui->helpText->setPlainText(help);
	});
	connect(ui->departureTree, &sexp_tree::miniHelpChanged, this, [this](const QString& help) {
		ui->HelpTitle->setText(help);
	});
	connect(ui->noDepartureWarpCheckBox, &QCheckBox::toggled, this, &ShipEditorDialog::departureWarpChanged);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipEditorDialog::~ShipEditorDialog() {}

void ShipEditorDialog::closeEvent(QCloseEvent* event)
{
	_model->apply();
	QDialog::closeEvent(event);
}
void ShipEditorDialog::focusOutEvent(QFocusEvent* event)
{
	QDialog::focusOutEvent(event);
	_model->apply();
}

void ShipEditorDialog::updateUI()
{
	util::SignalBlockers blockers(this);
	enableDisable();
	updateColumnOne();
	updateColumnTwo();
	updateArrival();
	updateDeparture();
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
		if (_model->getTeam() != -1) {
			ui->teamCombo->setEnabled(true);
		} else {
			ui->teamCombo->setEnabled(false);
		}

		ui->teamCombo->clear();
		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(i));
		}
	} else {
		idx = _model->getTeam();
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
		ui->altNameCombo->clear();
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
		ui->callsignCombo->clear();
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
void ShipEditorDialog::updateColumnTwo()
{
	int i;
	ui->wing->setText(_model->getWing().c_str());

	ui->personaCombo->clear();
	ui->personaCombo->addItem("<none>", QVariant(-1));
	for (i = 0; i < Num_personas; i++) {
		if (Personas[i].flags & PERSONA_FLAG_WINGMAN) {

			SCP_string persona_name = Personas[i].name;
			if (Personas[i].flags & PERSONA_FLAG_VASUDAN) {
				persona_name += " -Vas";
			}

			ui->personaCombo->addItem(persona_name.c_str(), QVariant(i));
		}
	}
	auto idx = _model->getPersona();
	ui->personaCombo->setCurrentIndex(ui->personaCombo->findData(idx));

	ui->killScoreEdit->setValue(_model->getScore());

	ui->assistEdit->setValue(_model->getAssist());

	ui->playerShipCheckBox->setChecked(_model->getPlayer());
}
void ShipEditorDialog::updateArrival()
{
	auto idx = _model->getArrivalLocation();
	int i;
	ui->arrivalLocationCombo->clear();
	for (i = 0; i < MAX_ARRIVAL_NAMES; i++) {
		ui->arrivalLocationCombo->addItem(Arrival_location_names[i], QVariant(i));
	}
	ui->arrivalLocationCombo->setCurrentIndex(ui->arrivalLocationCombo->findData(idx));

	object* objp;
	int restrict_to_players;
	ui->arrivalTargetCombo->clear();
	if (_model->getArrivalLocation() != ARRIVE_FROM_DOCK_BAY) {
		// Add Special Arrivals
		for (restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++) {
			for (i = 0; i < Num_iffs; i++) {
				char tmp[NAME_LENGTH + 15];
				// char *buf, int iff_index, int restrict_to_players, //int retail_format
				char* iff_name = Iff_info[i].iff_name;


				if (restrict_to_players)
					sprintf(tmp, "<any %s player>", iff_name);
				else
					sprintf(tmp, "<any %s>", iff_name);

				strlwr(tmp);

				ui->arrivalTargetCombo->addItem(tmp, QVariant(get_special_anchor(tmp)));
			}
		}
		// Add All Ships
		for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				!(objp->flags[Object::Object_Flags::Marked])) {
				ui->arrivalTargetCombo->addItem(Ships[_model->get_ship_from_obj(objp)].ship_name,
					QVariant(_model->get_ship_from_obj(objp)));
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
					ui->arrivalTargetCombo->addItem(Ships[_model->get_ship_from_obj(objp)].ship_name,
						QVariant(_model->get_ship_from_obj(objp)));
				}
			}
		}
	}
	ui->arrivalTargetCombo->setCurrentIndex(ui->arrivalTargetCombo->findData(_model->getArrivalTarget()));

	ui->arrivalDistanceEdit->clear();
	ui->arrivalDistanceEdit->setValue(_model->getArrivalDistance());
	ui->arrivalDelaySpinBox->setValue(_model->getArrivalDelay());

	ui->arrivalTree->initializeEditor(_viewport->editor, this);
	if (_model->ship_count) {

		if (_model->multi_edit) {
			ui->arrivalTree->clear_tree("");
		}
		if (_model->cue_init) {
			ui->arrivalTree->load_tree(_model->getArrivalFormula());
		} else {
			ui->arrivalTree->clear_tree("");
		}
		if (!_model->multi_edit) {
			i = ui->arrivalTree->select_sexp_node;
			if (i != -1) {
				ui->arrivalTree->hilite_item(i);
			}
		}
	} else {
		ui->arrivalTree->clear_tree("");
	}

	ui->noArrivalWarpCheckBox->setChecked(_model->getNoArrivalWarp());
}
void ShipEditorDialog::updateDeparture()
{
	auto idx = _model->getDepartureLocation();
	int i;
	ui->departureLocationCombo->clear();
	for (i = 0; i < MAX_DEPARTURE_NAMES; i++) {
		ui->departureLocationCombo->addItem(Departure_location_names[i], QVariant(i));
	}
	ui->departureLocationCombo->setCurrentIndex(ui->departureLocationCombo->findData(idx));
	object* objp;

	ui->departureTargetCombo->clear();
	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && !(objp->flags[Object::Object_Flags::Marked])) {
			polymodel* pm;

			// determine if this ship has a docking bay
			pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
			Assert(pm);
			if (pm->ship_bay && (pm->ship_bay->num_paths > 0)) {
				ui->departureTargetCombo->addItem(Ships[_model->get_ship_from_obj(objp)].ship_name,
					QVariant(_model->get_ship_from_obj(objp)));
			}
		}
	}
	ui->departureTargetCombo->setCurrentIndex(ui->departureTargetCombo->findData(_model->getDepartureTarget()));

	ui->departureDelaySpinBox->setValue(_model->getDepartureDelay());

	ui->departureTree->initializeEditor(_viewport->editor, this);
	if (_model->ship_count) {

		if (_model->multi_edit) {
			ui->departureTree->clear_tree("");
		}
		if (_model->cue_init) {
			ui->departureTree->load_tree(_model->getDepartureFormula(), "false");
		} else {
			ui->departureTree->clear_tree("");
		}
		if (!_model->multi_edit) {
			i = ui->arrivalTree->select_sexp_node;
			if (i != -1) {
				i = ui->departureTree->select_sexp_node;
				ui->departureTree->hilite_item(i);
			}
		}
	} else {
		ui->departureTree->clear_tree("");
	}

	ui->noDepartureWarpCheckBox->setChecked(_model->getNoDepartureWarp());
}
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
void ShipEditorDialog::hotkeyChanged(int index)
{
	auto hotkeyIdx = ui->hotkeyCombo->itemData(index).value<int>();
	_model->setHotkey(hotkeyIdx);
}
void ShipEditorDialog::personaChanged(int index)
{
	auto personaIdx = ui->personaCombo->itemData(index).value<int>();
	_model->setPersona(personaIdx);
}
void ShipEditorDialog::scoreChanged(int value) { _model->setScore(value); }
void ShipEditorDialog::assistChanged(int value) { _model->setAssist(value); }
void ShipEditorDialog::playerChanged(bool enabled) { _model->setPlayer(enabled); }

void ShipEditorDialog::arrivalLocationChanged(int index)
{
	auto arrivalLocationIdx = ui->arrivalLocationCombo->itemData(index).value<int>();
	_model->setArrivalLocation(arrivalLocationIdx);
}

void ShipEditorDialog::arrivalTargetChanged(int index)
{
	auto arrivalLocationIdx = ui->arrivalTargetCombo->itemData(index).value<int>();
	_model->setArrivalTarget(arrivalLocationIdx);
}

void ShipEditorDialog::arrivalDistanceChanged(int value) { _model->setArrivalDistance(value); }

void ShipEditorDialog::arrivalDelayChanged(int value) { _model->setArrivalDelay(value); }

void ShipEditorDialog::arrivalWarpChanged(bool enable) { _model->setNoArrivalWarp(enable); }

void ShipEditorDialog::departureLocationChanged(int index)
{
	auto depLocationIdx = ui->departureLocationCombo->itemData(index).value<int>();
	_model->setDepartureLocation(depLocationIdx);
}

void ShipEditorDialog::departureTargetChanged(int index)
{
	auto depLocationIdx = ui->departureTargetCombo->itemData(index).value<int>();
	_model->setDepartureTarget(depLocationIdx);
}

void ShipEditorDialog::departureDelayChanged(int value) { _model->setDepartureDelay(value); }

void ShipEditorDialog::departureWarpChanged(bool value) { _model->setNoDepartureWarp(value); }

void ShipEditorDialog::on_playerShipButton_clicked() { _model->setPlayer(true); }
void ShipEditorDialog::on_altShipClassButton_clicked()
{
	// TODO: altshipclassui
}
void ShipEditorDialog::on_prevButton_clicked() { _model->OnPrevious(); }
void ShipEditorDialog::on_nextButton_clicked() { _model->OnNext(); }
void ShipEditorDialog::on_resetButton_clicked() { _model->OnShipReset(); }
void ShipEditorDialog::on_deleteButton_clicked() { _model->OnDeleteShip(); }
void ShipEditorDialog::on_weaponsButton_clicked()
{
	// TODO: weapons dialog
}
void ShipEditorDialog::on_playerOrdersButton_clicked()
{
	// TODO: player orders dialog
}
void ShipEditorDialog::on_specialExpButton_clicked()
{
	// TODO: Special exp dialog
}
void ShipEditorDialog::on_specialHitsButton_clicked()
{
	// TODO: Special hits dialog
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
} // namespace dialogs
} // namespace fred
} // namespace fso