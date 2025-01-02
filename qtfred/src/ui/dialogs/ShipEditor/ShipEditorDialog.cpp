#include "ShipEditorDialog.h"

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "mission/object.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>

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
	connect(viewport->editor, &Editor::currentObjectChanged, this, &ShipEditorDialog::update);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &ShipEditorDialog::update);

	// Column One
	connect(ui->shipNameEdit, (&QLineEdit::editingFinished), this, &ShipEditorDialog::shipNameChanged);

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
	connect(ui->altNameCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::altNameChanged);
	connect(ui->callsignCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::callsignChanged);

	// ui->cargoCombo->installEventFilter(this);

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
void ShipEditorDialog::showEvent(QShowEvent* e) {
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
	auto dialog = new dialogs::ShipGoalsDialog(this, _viewport, getIfMultipleShips(), Ships[getSingleShip()].objnum, -1);
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
	}
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
	util::SignalBlockers blockers(this);
	ui->shipNameEdit->setText(_model->getShipName().c_str());
	size_t i;
	auto idx = _model->getShipClass();
	ui->shipClassCombo->clear();
	for (i = 0; i < Ship_info.size(); i++) {
		ui->shipClassCombo->addItem(Ship_info[i].name, QVariant(static_cast<int>(i)));
	}
	ui->shipClassCombo->setCurrentIndex(ui->shipClassCombo->findData(idx));

	auto ai = _model->getAIClass();
	ui->AIClassCombo->clear();
	for (int j = 0; j < Num_ai_classes; j++) {
		ui->AIClassCombo->addItem(Ai_class_names[j], QVariant(j));  
	}
	ui->AIClassCombo->setCurrentIndex(ui->AIClassCombo->findData(ai));

	if (_model->getNumSelectedPlayers()) {
		if (_model->getTeam() != -1) {
			ui->teamCombo->setEnabled(true);
		} else {
			ui->teamCombo->setEnabled(false);
		}

		ui->teamCombo->clear();
		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(static_cast<int>(i)));
		}
	} else {
		idx = _model->getTeam();
		ui->teamCombo->setEnabled(_model->getUIEnable());
		ui->teamCombo->clear();
		for (i = 0; i < Iff_info.size(); i++) {
			ui->teamCombo->addItem(Iff_info[i].iff_name, QVariant(static_cast<int>(i)));
		}
		ui->teamCombo->setCurrentIndex(ui->teamCombo->findData(idx));
	}
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
	ui->altNameCombo->clear();
	if (_model->getNumSelectedObjects()) {
		if (_model->getIfMultipleShips()) {
			ui->altNameCombo->setEnabled(false);
		} else {
			auto altname = _model->getAltName();
			ui->altNameCombo->setEnabled(true);
			ui->altNameCombo->addItem("<none>");
			for (j = 0; j < Mission_alt_type_count; j++) {
				ui->altNameCombo->addItem(Mission_alt_types[j]);
			}
			if (ui->altNameCombo->findText(QString(altname.c_str()))) {
				ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText(QString(altname.c_str())));
			} else {
				ui->altNameCombo->setCurrentIndex(ui->altNameCombo->findText("<none>"));
			}
		}
	}
	ui->callsignCombo->clear();
	if (_model->getNumSelectedObjects()) {
		if (_model->getIfMultipleShips()) {
			ui->callsignCombo->setEnabled(false);
		} else {
			auto callsign = _model->getCallsign();
			ui->callsignCombo->addItem("<none>");
			ui->callsignCombo->setEnabled(true);
			for (j = 0; j < Mission_callsign_count; j++) {
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
void ShipEditorDialog::updateColumnTwo()
{
	util::SignalBlockers blockers(this);
	ui->wing->setText(_model->getWing().c_str());

	ui->personaCombo->clear();
	ui->personaCombo->addItem("<none>", QVariant(-1));
	for (size_t i = 0; i < Personas.size(); i++) {
		if (Personas[i].flags & PERSONA_FLAG_WINGMAN) {
			SCP_string persona_name = Personas[i].name;

			// see if the bitfield matches one and only one species
			int species = -1;
			for (size_t j = 0; j < 32 && j < Species_info.size(); j++) {
				if (Personas[i].species_bitfield == (1 << j)) {
					species = static_cast<int>(j);
					break;
				}
			}

			// if it is an exact species that isn't the first
			if (species > 0) {
				persona_name += "-";

				auto species_name = Species_info[species].species_name;
				size_t len = strlen(species_name);
				for (size_t j = 0; j < 3 && j < len; j++)
					persona_name += species_name[j];
			}

			ui->personaCombo->addItem(persona_name.c_str(), QVariant(static_cast<int>(i)));
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
	util::SignalBlockers blockers(this);
	auto idx = _model->getArrivalLocationIndex();
	int i;
	ui->arrivalLocationCombo->clear();
	for (i = 0; i < MAX_ARRIVAL_NAMES; i++) {
		ui->arrivalLocationCombo->addItem(Arrival_location_names[i], QVariant(i));
	}
	ui->arrivalLocationCombo->setCurrentIndex(ui->arrivalLocationCombo->findData(idx));

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
void ShipEditorDialog::updateDeparture()
{
	util::SignalBlockers blockers(this);
	auto idx = _model->getDepartureLocationIndex();
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

/*---------------------------------------------------------
					WARNING
Do not try to optimise string entries this convoluted method is necessary to avoid fata errors caused by QT
-----------------------------------------------------------*/
void ShipEditorDialog::shipNameChanged()
{
	const QString entry = ui->shipNameEdit->text();
	if (!entry.isEmpty() && entry != _model->getShipName().c_str()) {
		const auto textBytes = entry.toUtf8();
		const std::string NewShipName = textBytes.toStdString();
		_model->setShipName(NewShipName);
	}
}
void ShipEditorDialog::shipClassChanged(const int index)
{
	auto shipClassIdx = ui->shipClassCombo->itemData(index).value<int>();
	_model->setShipClass(shipClassIdx);
}
void ShipEditorDialog::aiClassChanged(const int index)
{
	auto aiClassIdx = ui->AIClassCombo->itemData(index).value<int>();
	_model->setAIClass(aiClassIdx);
}
void ShipEditorDialog::teamChanged(const int index)
{
	auto teamIdx = ui->teamCombo->itemData(index).value<int>();
	_model->setTeam(teamIdx);
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
void ShipEditorDialog::hotkeyChanged(const int index)
{
	auto hotkeyIdx = ui->hotkeyCombo->itemData(index).value<int>();
	_model->setHotkey(hotkeyIdx);
}
void ShipEditorDialog::personaChanged(const int index)
{
	auto personaIdx = ui->personaCombo->itemData(index).value<int>();
	_model->setPersona(personaIdx);
}
void ShipEditorDialog::scoreChanged(const int value)
{
	_model->setScore(value);
}
void ShipEditorDialog::assistChanged(const int value)
{
	_model->setAssist(value);
}
void ShipEditorDialog::playerChanged(const bool enabled)
{
	_model->setPlayer(enabled);
}

void ShipEditorDialog::arrivalLocationChanged(const int index)
{
	auto arrivalLocationIdx = ui->arrivalLocationCombo->itemData(index).value<int>();
	_model->setArrivalLocationIndex(arrivalLocationIdx);
}

void ShipEditorDialog::arrivalTargetChanged(const int index)
{
	auto arrivalLocationIdx = ui->arrivalTargetCombo->itemData(index).value<int>();
	_model->setArrivalTarget(arrivalLocationIdx);
}

void ShipEditorDialog::arrivalDistanceChanged(const int value)
{
	_model->setArrivalDistance(value);
}

void ShipEditorDialog::arrivalDelayChanged(const int value)
{
	_model->setArrivalDelay(value);
}

void ShipEditorDialog::arrivalWarpChanged(const bool enable)
{
	_model->setNoArrivalWarp(enable);
}

void ShipEditorDialog::ArrivalCueChanged(const bool value)
{
	_model->setArrivalCue(value);
}

void ShipEditorDialog::departureLocationChanged(const int index)
{
	auto depLocationIdx = ui->departureLocationCombo->itemData(index).value<int>();
	_model->setDepartureLocationIndex(depLocationIdx);
}

void ShipEditorDialog::departureTargetChanged(const int index)
{
	auto depLocationIdx = ui->departureTargetCombo->itemData(index).value<int>();
	_model->setDepartureTarget(depLocationIdx);
}

void ShipEditorDialog::departureDelayChanged(const int value)
{
	_model->setDepartureDelay(value);
}

void ShipEditorDialog::departureWarpChanged(const bool value)
{
	_model->setNoDepartureWarp(value);
}

void ShipEditorDialog::DepartureCueChanged(const bool value)
{
	_model->setDepartureCue(value);
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
	// TODO: altshipclassui
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
		//TODO
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
} // namespace dialogs
} // namespace fred
} // namespace fso