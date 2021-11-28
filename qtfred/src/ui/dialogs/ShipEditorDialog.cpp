#include "ShipEditorDialog.h"

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>

#include "mission/object.h"

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {

ShipEditorDialog::ShipEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipEditorDialog()), _model(new ShipEditorDialogModel(this, viewport)),
	  _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipEditorDialog::updateUI);
	connect(this, &QDialog::accepted, _model.get(), &ShipEditorDialogModel::apply);
	connect(viewport->editor, &Editor::currentObjectChanged, _model.get(), &ShipEditorDialogModel::apply);
	connect(viewport->editor, &Editor::objectMarkingChanged, _model.get(), &ShipEditorDialogModel::apply);

	// Column One
	connect(ui->shipNameEdit,(&QLineEdit::editingFinished),
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

	connect(ui->cargoCombo->lineEdit(), (&QLineEdit::editingFinished), this, &ShipEditorDialog::cargoChanged);

	// ui->cargoCombo->installEventFilter(this);
	connect(ui->altNameCombo->lineEdit(), SIGNAL(editingFinished()),
		this,
		SLOT(altNameChanged()));
	connect(ui->altNameCombo,
		SIGNAL(currentIndexChanged(const QString&)),
		this,
		SLOT(altNameChanged(const QString&)));
	connect(ui->callsignCombo,
		SIGNAL(currentIndexChanged(const QString&)),
		this,
		SLOT(callsignChanged(const QString&)));
	connect(ui->callsignCombo->lineEdit(), SIGNAL(editingFinished()),
		this,
		SLOT(callsignChanged()));

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

	connect(ui->updateArrivalCueCheckBox, &QCheckBox::toggled, this, &ShipEditorDialog::ArrivalCueChanged);

	connect(ui->arrivalTree, &sexp_tree::rootNodeFormulaChanged, this, [this](int old, int node) {
		// use this otherwise linux complains

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

	connect(ui->updateDepartureCueCheckBox, &QCheckBox::toggled, this, &ShipEditorDialog::DepartureCueChanged);

	connect(ui->departureTree, &sexp_tree::rootNodeFormulaChanged, this, [this](int old, int node) {
		// use this otherwise linux complains
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

ShipEditorDialog::~ShipEditorDialog() = default;

void ShipEditorDialog::closeEvent(QCloseEvent* event)
{
	_model->apply();
	QDialog::closeEvent(event);
}

void ShipEditorDialog::on_miscButton_clicked()
{
	auto flagsDialog = new dialogs::ShipFlagsDialog(this, _viewport);
	flagsDialog->show();
}

void ShipEditorDialog::on_initialStatusButton_clicked()
{
	auto initialStatusDialog = new dialogs::ShipInitialStatusDialog(this, _viewport, _model->multi_edit);
	initialStatusDialog->show();
}

void ShipEditorDialog::on_initialOrdersButton_clicked()
{
	auto GoalsDialog = new dialogs::ShipGoalsDialog(this, _viewport, _model->multi_edit, Ships[_model->single_ship].objnum, -1);
	GoalsDialog->show();

}

void ShipEditorDialog::on_tblInfoButton_clicked()
{
	// TODO:: TBL Dialog
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
	for (i = 0; i < int(Ship_info.size()); i++) {
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
		ui->teamCombo->setEnabled(_model->enable);
		ui->teamCombo->clear();
		for (i = 0; i < (int)Iff_info.size(); i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(i));
		}
		ui->teamCombo->setCurrentIndex(ui->teamCombo->findData(idx));
	}
	auto cargo = _model->getCargo();
	ui->cargoCombo->clear();
	for (i = 0; i < Num_cargo; i++) {
		ui->cargoCombo->addItem(Cargo_names[i]);
	}
	if (ui->cargoCombo->findText(QString(cargo.c_str()))) {
		ui->cargoCombo->setCurrentIndex(ui->cargoCombo->findText(QString(cargo.c_str())));
	} else {
		ui->cargoCombo->addItem(cargo.c_str());

		ui->cargoCombo->setCurrentIndex(ui->cargoCombo->findText(QString(cargo.c_str())));
	}
	ui->altNameCombo->clear();
	if (_model->multi_edit) {
		ui->altNameCombo->setEnabled(false);
	} else {
		auto altname = _model->getAltName();
		ui->altNameCombo->setEnabled(true);
		ui->altNameCombo->addItem("<none>");
		for (i = 0; i < Mission_alt_type_count; i++) {
			ui->altNameCombo->addItem(Mission_alt_types[i], QVariant(Mission_alt_types[i]));
		}
		if (ui->altNameCombo->findText(QString(altname.c_str()))) {
			ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText(QString(altname.c_str())));
		} else {
			ui->altNameCombo->addItem(altname.c_str(), QVariant(altname.c_str()));
			ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText(QString(altname.c_str())));
		}
	}
	ui->callsignCombo->clear();
	if (_model->multi_edit) {
		ui->callsignCombo->setEnabled(false);
	} else {
		auto callsign = _model->getCallsign();
		ui->callsignCombo->addItem("<none>");
		ui->callsignCombo->setEnabled(true);
		for (i = 0; i < Mission_callsign_count; i++) {
			ui->callsignCombo->addItem(Mission_callsigns[i], QVariant(Mission_callsigns[i]));
		}

		if (ui->callsignCombo->findText(QString(callsign.c_str()))) {
			ui->callsignCombo->setCurrentIndex(ui->callsignCombo->findText(QString(callsign.c_str())));
		} else {
			ui->callsignCombo->addItem(callsign.c_str(), QVariant(callsign.c_str()));
			ui->callsignCombo->setCurrentIndex(ui->callsignCombo->findText(QString(callsign.c_str())));
		}
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
			for (i = 0; i < (int)Iff_info.size(); i++) {
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, i, restrict_to_players, 0);

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

	ui->arrivalDistanceEdit->clear();
	ui->arrivalDistanceEdit->setValue(_model->getArrivalDistance());
	ui->arrivalDelaySpinBox->setValue(_model->getArrivalDelay());

	ui->updateArrivalCueCheckBox->setChecked(_model->getArrivalCue());

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
				auto ship = get_ship_from_obj(objp);
				ui->departureTargetCombo->addItem(Ships[ship].ship_name, QVariant(ship));
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

	ui->updateDepartureCueCheckBox->setChecked(_model->getDepartureCue());
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
		ui->arrivalLocationCombo->setEnabled(_model->enable);
		if (_model->getArrivalLocation()) {
			ui->arrivalDistanceEdit->setEnabled(_model->enable);
			ui->arrivalTargetCombo->setEnabled(_model->enable);
		} else {
			ui->arrivalDistanceEdit->setEnabled(false);
			ui->arrivalTargetCombo->setEnabled(false);
		}
		if (_model->getArrivalLocation() == ARRIVE_FROM_DOCK_BAY) {
			ui->restrictArrivalPathsButton->setEnabled(_model->enable);
			ui->customWarpinButton->setEnabled(false);
		} else {
			ui->restrictArrivalPathsButton->setEnabled(false);
			ui->customWarpinButton->setEnabled(_model->enable);
		}

		ui->departureLocationCombo->setEnabled(_model->enable);
		if (_model->getDepartureLocation()) {
			ui->departureTargetCombo->setEnabled(_model->enable);
		} else {
			ui->departureTargetCombo->setEnabled(false);
		}
		if (_model->getDepartureLocation() == DEPART_AT_DOCK_BAY) {
			ui->restrictDeparturePathsButton->setEnabled(_model->enable);
			ui->customWarpoutButton->setEnabled(false);
		} else {
			ui->restrictDeparturePathsButton->setEnabled(false);
			ui->customWarpoutButton->setEnabled(_model->enable);
		}

		ui->arrivalDelaySpinBox->setEnabled(_model->enable);
		ui->arrivalTree->setEnabled(_model->enable);
		ui->departureDelaySpinBox->setEnabled(_model->enable);
		ui->departureTree->setEnabled(_model->enable);
		ui->noArrivalWarpCheckBox->setEnabled(_model->enable);
		ui->noDepartureWarpCheckBox->setEnabled(_model->enable);
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

	ui->AIClassCombo->setEnabled(_model->enable);
	ui->cargoCombo->setEnabled(_model->enable);
	ui->hotkeyCombo->setEnabled(_model->enable);
	if ((_model->getShipClass() >= 0) && !(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::Cargo]) &&
		!(Ship_info[_model->getShipClass()].flags[Ship::Info_Flags::No_ship_type]))
		ui->initialOrdersButton->setEnabled(_model->enable);
	else if (_model->multi_edit)
		ui->initialOrdersButton->setEnabled(_model->enable);
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

	ui->deleteButton->setEnabled(_model->enable);
	ui->resetButton->setEnabled(_model->enable);
	ui->killScoreEdit->setEnabled(_model->enable);
	ui->assistEdit->setEnabled(_model->enable);

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
	}
	else {
		// always enabled when one ship is selected
		ui->playerOrdersButton->setEnabled(_model->enable);
	}

	// always enabled if >= 1 ship selected
	ui->personaCombo->setEnabled(_model->enable);

	if (_model->multi_edit) {
		this->setWindowTitle("Edit Marked Ships");
	} else if (_model->player_count) {
		this->setWindowTitle("Edit Player Ship");
	} else {
		this->setWindowTitle("Edit Ship");
	}
}

void ShipEditorDialog::shipNameChanged() { _model->setShipName(ui->shipNameEdit->text().toStdString()); }
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
void ShipEditorDialog::cargoChanged() { _model->setCargo(ui->cargoCombo->currentText().toStdString()); }
void ShipEditorDialog::altNameChanged()
{
	_model->setAltName(ui->altNameCombo->lineEdit()->text().toStdString());
}
void ShipEditorDialog::altNameChanged(const QString& value) { _model->setAltName(value.toStdString()); }
void ShipEditorDialog::callsignChanged()
{
	_model->setCallsign(ui->callsignCombo->lineEdit()->text().toStdString());
}
void ShipEditorDialog::callsignChanged(const QString& value) { _model->setCallsign(value.toStdString()); }
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

void ShipEditorDialog::ArrivalCueChanged(bool value) { _model->setArrivalCue(value); }

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

void ShipEditorDialog::DepartureCueChanged(bool value) { _model->setDepartureCue(value); }

void ShipEditorDialog::on_textureReplacementButton_clicked()
{
	// TODO:: Texture Replacement Dialog
}

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
	auto playerOrdersDialog = new dialogs::PlayerOrdersDialog(this, _viewport, _model->multi_edit);
	playerOrdersDialog->show();
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
void ShipEditorDialog::on_restrictArrivalPathsButton_clicked()
{
	// TODO:: Restrict Paths Dialog
}
void ShipEditorDialog::on_customWarpinButton_clicked()
{
	// TODO:: Custom warp Dialog
}
void ShipEditorDialog::on_restrictDeparturePathsButton_clicked()
{
	// TODO:: Restrict Paths Dialog
}
void ShipEditorDialog::on_customWarpoutButton_clicked()
{ // TODO:: Custom warp Dialog
}
} // namespace dialogs
} // namespace fred
} // namespace fso