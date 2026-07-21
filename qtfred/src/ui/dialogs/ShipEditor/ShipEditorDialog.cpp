#include "ShipEditorDialog.h"
#include <ui/util/DialogUndo.h>

#include "ui_ShipEditorDialog.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "missioneditor/common.h"
#include "mission/object.h"
#include "mission/missionparse.h"

#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <object/objectdock.h>
#include <model/model.h>
#include <ui/util/SignalBlockers.h>
#include <ui/util/menu.h>
#include <mission/commands/FredCommands.h>

#include <QCloseEvent>
#include <ui/dialogs/General/CheckBoxListDialog.h>
#include <QVariant>

namespace fso::fred::dialogs {

namespace {

struct ShipDisplayNameState {
	SCP_string displayName;
	bool hasFlag;
	bool operator==(const ShipDisplayNameState& o) const {
		return displayName == o.displayName && hasFlag == o.hasFlag;
	}
};

// Iterate marked ships and call fn(signature, instance) for each
template<typename Fn>
void forEachMarkedShip(Fn fn) {
	for (auto* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
		if ((p->type == OBJ_SHIP || p->type == OBJ_START) && p->flags[Object::Object_Flags::Marked]) {
			fn(p->signature, p->instance);
		}
	}
}

// Merge-identity key for FieldEditCommand: the current marked-ship signatures.
// Consecutive edits of the same field merge only while the selection is
// unchanged; switching ships starts a new undo step.
SCP_string markedShipsKey() {
	SCP_string key;
	forEachMarkedShip([&key](int sig, int /*inst*/) {
		key += std::to_string(sig);
		key += ',';
	});
	return key;
}

// --- per-subdialog snapshot types ---

struct ShipFlagsSnapshot {
	int sig;
	SCP_string shipName;
	flagset<Ship::Ship_Flags> shipFlags;
	flagset<AI::AI_Flags> aiFlags;
	flagset<Object::Object_Flags> objFlags;
	int kamikazeDamage;
	int escortPriority;
	int finalDeathTime;
	bool operator==(const ShipFlagsSnapshot& o) const {
		return sig == o.sig && shipFlags == o.shipFlags && aiFlags == o.aiFlags &&
		       objFlags == o.objFlags && kamikazeDamage == o.kamikazeDamage &&
		       escortPriority == o.escortPriority && finalDeathTime == o.finalDeathTime;
	}
};

// Editor-visible equality for one initial-orders slot. Strings are compared by
// content (an apply can re-point target_name without changing it), and the
// dock point unions are compared per their *_index_valid flags.
bool aiGoalEditorEqual(const ai_goal& a, const ai_goal& b)
{
	auto strEq = [](const char* x, const char* y) {
		if (x == y) return true;
		if (!x || !y) return false;
		return stricmp(x, y) == 0;
	};

	if (a.ai_mode != b.ai_mode)
		return false;
	if (a.ai_mode == AI_GOAL_NONE)
		return true; // empty slots; remaining fields are meaningless
	if (a.ai_submode != b.ai_submode || a.priority != b.priority)
		return false;
	if (!strEq(a.target_name, b.target_name))
		return false;

	const bool aDockerIdx = a.flags[AI::Goal_Flags::Docker_index_valid];
	if (aDockerIdx != b.flags[AI::Goal_Flags::Docker_index_valid])
		return false;
	if (aDockerIdx ? (a.docker.index != b.docker.index) : !strEq(a.docker.name, b.docker.name))
		return false;

	const bool aDockeeIdx = a.flags[AI::Goal_Flags::Dockee_index_valid];
	if (aDockeeIdx != b.flags[AI::Goal_Flags::Dockee_index_valid])
		return false;
	if (aDockeeIdx ? (a.dockee.index != b.dockee.index) : !strEq(a.dockee.name, b.dockee.name))
		return false;

	return true;
}

struct ShipGoalsSnapshot {
	int sig;
	std::array<ai_goal, MAX_AI_GOALS> goals;
	bool operator==(const ShipGoalsSnapshot& o) const {
		if (sig != o.sig) return false;
		for (int i = 0; i < MAX_AI_GOALS; i++) {
			if (!aiGoalEditorEqual(goals[i], o.goals[i]))
				return false;
		}
		return true;
	}
};

struct ShipInitialStatusSnapshot {
	int sig;
	vec3d pos;
	matrix orient;
	float speed;
	float shieldQuadrant0;
	float hullStrength;
	int guardianThreshold;
	flagset<Ship::Ship_Flags> shipFlags;
	flagset<Object::Object_Flags> objFlags;
	SCP_string teamName;
	int arrivalCue;
	struct SubsysState {
		float currentHits;
		int subsysCargoName;
		char subsysCargoTitle[NAME_LENGTH];
	};
	SCP_vector<SubsysState> subsystems;
	struct DockLink {
		int thisDockpoint;
		int otherSig;
		int otherDockpoint;
		// Position of the other ship at snapshot time so dock-related moves
		// (ships moved apart on undock) are fully reversed.
		vec3d otherPos;
		matrix otherOrient;
	};
	SCP_vector<DockLink> dockLinks;
	// Positions of ships that were moved by a newly-added dock connection.
	// Populated from ShipInitialStatusDialog::preApplyDockeePositions() via
	// the accepted() signal — captured before apply() physically moves them.
	struct ExtraPosition { int sig; vec3d pos; matrix orient; };
	SCP_vector<ExtraPosition> extraPositions;
	// pos/orient are excluded from equality: they change as a consequence of
	// dock changes, which are already compared via dockLinks.
	bool operator==(const ShipInitialStatusSnapshot& o) const {
		if (sig != o.sig || speed != o.speed || shieldQuadrant0 != o.shieldQuadrant0 ||
		    hullStrength != o.hullStrength || guardianThreshold != o.guardianThreshold ||
		    shipFlags != o.shipFlags || objFlags != o.objFlags ||
		    teamName != o.teamName || arrivalCue != o.arrivalCue)
			return false;
		if (subsystems.size() != o.subsystems.size()) return false;
		for (size_t i = 0; i < subsystems.size(); i++) {
			if (subsystems[i].currentHits != o.subsystems[i].currentHits ||
			    subsystems[i].subsysCargoName != o.subsystems[i].subsysCargoName ||
			    strcmp(subsystems[i].subsysCargoTitle, o.subsystems[i].subsysCargoTitle) != 0)
				return false;
		}
		if (dockLinks.size() != o.dockLinks.size()) return false;
		for (size_t i = 0; i < dockLinks.size(); i++) {
			if (dockLinks[i].thisDockpoint != o.dockLinks[i].thisDockpoint ||
			    dockLinks[i].otherSig != o.dockLinks[i].otherSig ||
			    dockLinks[i].otherDockpoint != o.dockLinks[i].otherDockpoint)
				return false;
		}
		return true;
	}
};

struct WeaponBankState {
	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];
	int primary_bank_ammo[MAX_SHIP_PRIMARY_BANKS];
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];
	int secondary_bank_ammo[MAX_SHIP_SECONDARY_BANKS];
	int ai_class;
};

bool weaponBankStateEqual(const WeaponBankState& a, const WeaponBankState& b)
{
	if (a.ai_class != b.ai_class) return false;
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
		if (a.primary_bank_weapons[i] != b.primary_bank_weapons[i]) return false;
		if (a.primary_bank_ammo[i]    != b.primary_bank_ammo[i])    return false;
	}
	for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		if (a.secondary_bank_weapons[i] != b.secondary_bank_weapons[i]) return false;
		if (a.secondary_bank_ammo[i]    != b.secondary_bank_ammo[i])    return false;
	}
	return true;
}

struct ShipWeaponsSnapshot {
	int sig;
	WeaponBankState pilotWeapons;
	SCP_vector<WeaponBankState> turretWeapons;
	bool operator==(const ShipWeaponsSnapshot& o) const {
		if (sig != o.sig) return false;
		if (!weaponBankStateEqual(pilotWeapons, o.pilotWeapons)) return false;
		if (turretWeapons.size() != o.turretWeapons.size()) return false;
		for (size_t i = 0; i < turretWeapons.size(); i++) {
			if (!weaponBankStateEqual(turretWeapons[i], o.turretWeapons[i])) return false;
		}
		return true;
	}
};

struct ShipAltClassSnapshot {
	int sig;
	SCP_vector<alt_class> altClasses;
	bool operator==(const ShipAltClassSnapshot& o) const {
		return sig == o.sig && altClasses == o.altClasses;
	}
};

struct ShipSpecialStatsSnapshot {
	int sig;
	int specialHitpoints;
	int specialShield;
	bool useSpecialExplosion;
	int specialExpInner, specialExpOuter, specialExpDamage, specialExpBlast;
	bool useShockwave;
	int specialExpShockwaveSpeed;
	int specialExpDeathrollTime;
	int kamikazeDamage;
	bool operator==(const ShipSpecialStatsSnapshot& o) const {
		return sig == o.sig && specialHitpoints == o.specialHitpoints &&
		       specialShield == o.specialShield && useSpecialExplosion == o.useSpecialExplosion &&
		       specialExpInner == o.specialExpInner && specialExpOuter == o.specialExpOuter &&
		       specialExpDamage == o.specialExpDamage && specialExpBlast == o.specialExpBlast &&
		       useShockwave == o.useShockwave && specialExpShockwaveSpeed == o.specialExpShockwaveSpeed &&
		       specialExpDeathrollTime == o.specialExpDeathrollTime && kamikazeDamage == o.kamikazeDamage;
	}
};

struct ShipResetSnapshot {
	int sig;
	char cargo1;
	int team;
	int aiClass; // Ships[inst].weapons.ai_class
	std::array<ai_goal, MAX_AI_GOALS> goals;
	float speed;
	float shieldQuadrant0;
	float hullStrength;
	int primaryBankWeapons[MAX_SHIP_PRIMARY_BANKS];
	int secondaryBankWeapons[MAX_SHIP_SECONDARY_BANKS];
	int secondaryBankCapacity[MAX_SHIP_SECONDARY_BANKS];
	struct TurretState {
		float currentHits;
		int numPrimaryBanks;
		int primaryBankWeapons[MAX_SHIP_PRIMARY_BANKS];
		int numSecondaryBanks;
		int secondaryBankWeapons[MAX_SHIP_SECONDARY_BANKS];
		int secondaryBankCapacity[MAX_SHIP_SECONDARY_BANKS];
		int secondaryBankAmmo[MAX_SHIP_SECONDARY_BANKS];
	};
	SCP_vector<float> subsysHits; // current_hits for every subsystem in list order
	SCP_vector<TurretState> turrets; // one per SUBSYSTEM_TURRET in list order
	bool operator==(const ShipResetSnapshot&) const { return false; } // always push
};

ShipResetSnapshot captureShipResetSnapshot(int sig, int inst)
{
	ShipResetSnapshot s;
	s.sig = sig;
	s.cargo1 = Ships[inst].cargo1;
	s.team = Ships[inst].team;
	s.aiClass = Ships[inst].weapons.ai_class;
	const int ai_idx = Ships[inst].ai_index;
	for (int i = 0; i < MAX_AI_GOALS; i++)
		s.goals[i] = Ai_info[ai_idx].goals[i];
	const object& obj = Objects[Ships[inst].objnum];
	s.speed = obj.phys_info.speed;
	s.shieldQuadrant0 = obj.shield_quadrant.empty() ? 0.0f : obj.shield_quadrant[0];
	s.hullStrength = obj.hull_strength;
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
		s.primaryBankWeapons[i] = Ships[inst].weapons.primary_bank_weapons[i];
	for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		s.secondaryBankWeapons[i] = Ships[inst].weapons.secondary_bank_weapons[i];
		s.secondaryBankCapacity[i] = Ships[inst].weapons.secondary_bank_capacity[i];
	}
	for (auto* ss = GET_FIRST(&Ships[inst].subsys_list); ss != END_OF_LIST(&Ships[inst].subsys_list); ss = GET_NEXT(ss)) {
		s.subsysHits.push_back(ss->current_hits);
		if (ss->system_info && ss->system_info->type == SUBSYSTEM_TURRET) {
			ShipResetSnapshot::TurretState ts;
			ts.currentHits = ss->current_hits;
			ts.numPrimaryBanks = ss->weapons.num_primary_banks;
			for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
				ts.primaryBankWeapons[i] = ss->weapons.primary_bank_weapons[i];
			ts.numSecondaryBanks = ss->weapons.num_secondary_banks;
			for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
				ts.secondaryBankWeapons[i] = ss->weapons.secondary_bank_weapons[i];
				ts.secondaryBankCapacity[i] = ss->weapons.secondary_bank_capacity[i];
				ts.secondaryBankAmmo[i] = ss->weapons.secondary_bank_ammo[i];
			}
			s.turrets.push_back(ts);
		}
	}
	return s;
}

// Capture ShipFlagsSnapshot for one ship
ShipFlagsSnapshot captureShipFlagsSnapshot(int sig, int inst)
{
	ShipFlagsSnapshot s;
	s.sig = sig;
	s.shipName = Ships[inst].ship_name;
	s.shipFlags = Ships[inst].flags;
	s.aiFlags = Ai_info[Ships[inst].ai_index].ai_flags;
	s.objFlags = Objects[Ships[inst].objnum].flags;
	s.kamikazeDamage = Ai_info[Ships[inst].ai_index].kamikaze_damage;
	s.escortPriority = Ships[inst].escort_priority;
	s.finalDeathTime = Ships[inst].final_death_time;
	return s;
}

// Capture ShipInitialStatusSnapshot for one ship
ShipInitialStatusSnapshot captureShipInitialStatusSnapshot(int sig, int inst)
{
	ShipInitialStatusSnapshot s;
	s.sig = sig;
	const object& obj = Objects[Ships[inst].objnum];
	s.pos = obj.pos;
	s.orient = obj.orient;
	s.speed = obj.phys_info.speed;
	s.shieldQuadrant0 = obj.shield_quadrant.empty() ? 0.0f : obj.shield_quadrant[0];
	s.hullStrength = obj.hull_strength;
	s.guardianThreshold = Ships[inst].ship_guardian_threshold;
	s.shipFlags = Ships[inst].flags;
	s.objFlags = obj.flags;
	s.teamName = Ships[inst].team_name;
	s.arrivalCue = Ships[inst].arrival_cue;
	// subsystems
	for (auto* ss = GET_FIRST(&Ships[inst].subsys_list); ss != END_OF_LIST(&Ships[inst].subsys_list); ss = GET_NEXT(ss)) {
		ShipInitialStatusSnapshot::SubsysState ss_state;
		ss_state.currentHits = ss->current_hits;
		ss_state.subsysCargoName = ss->subsys_cargo_name;
		strcpy_s(ss_state.subsysCargoTitle, ss->subsys_cargo_title);
		s.subsystems.push_back(ss_state);
	}
	// dock links — also capture the other ship's position so dock-related
	// moves can be reversed on undo/redo
	for (auto* dl = obj.dock_list; dl != nullptr; dl = dl->next) {
		ShipInitialStatusSnapshot::DockLink link;
		link.thisDockpoint = dl->dockpoint_used;
		link.otherSig = dl->docked_objp->signature;
		link.otherDockpoint = dock_find_dockpoint_used_by_object(dl->docked_objp, &Objects[Ships[inst].objnum]);
		link.otherPos = dl->docked_objp->pos;
		link.otherOrient = dl->docked_objp->orient;
		s.dockLinks.push_back(link);
	}
	return s;
}

// Capture WeaponBankState from a ship_weapon struct
WeaponBankState captureWeaponBankState(const ship_weapon& w)
{
	WeaponBankState s;
	s.ai_class = w.ai_class;
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
		s.primary_bank_weapons[i] = w.primary_bank_weapons[i];
		s.primary_bank_ammo[i]    = w.primary_bank_ammo[i];
	}
	for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		s.secondary_bank_weapons[i] = w.secondary_bank_weapons[i];
		s.secondary_bank_ammo[i]    = w.secondary_bank_ammo[i];
	}
	return s;
}

// Restore a WeaponBankState into a ship_weapon struct
void restoreWeaponBankState(ship_weapon& w, const WeaponBankState& s)
{
	w.ai_class = s.ai_class;
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
		w.primary_bank_weapons[i] = s.primary_bank_weapons[i];
		w.primary_bank_ammo[i]    = s.primary_bank_ammo[i];
	}
	for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
		w.secondary_bank_weapons[i] = s.secondary_bank_weapons[i];
		w.secondary_bank_ammo[i]    = s.secondary_bank_ammo[i];
	}
}

// Capture ShipWeaponsSnapshot for one ship
ShipWeaponsSnapshot captureShipWeaponsSnapshot(int sig, int inst)
{
	ShipWeaponsSnapshot s;
	s.sig = sig;
	s.pilotWeapons = captureWeaponBankState(Ships[inst].weapons);
	for (auto* ss = GET_FIRST(&Ships[inst].subsys_list); ss != END_OF_LIST(&Ships[inst].subsys_list); ss = GET_NEXT(ss)) {
		if (ss->system_info && ss->system_info->type == SUBSYSTEM_TURRET)
			s.turretWeapons.push_back(captureWeaponBankState(ss->weapons));
	}
	return s;
}

} // anonymous namespace

ShipEditorDialog::ShipEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()),
	  ui(new Ui::ShipEditorDialog()), _model(new ShipEditorDialogModel(this, viewport)),
	  _viewport(viewport), _fredView(parent)
{
	this->setFocus();

	ui->setupUi(this);
	util::installMainStackUndoShortcuts(this, _fredView->mainUndoStack());
	ui->HelpTitle->setVisible(viewport->Show_sexp_help_ship_editor);
	ui->helpText->setVisible(viewport->Show_sexp_help_ship_editor);

	ui->shipNameEdit->setMaxLength(NAME_LENGTH - 1);
	ui->shipDisplayNameEdit->setMaxLength(NAME_LENGTH - 1);
	ui->altNameCombo->lineEdit()->setMaxLength(NAME_LENGTH - 1);
	ui->callsignCombo->lineEdit()->setMaxLength(CALLSIGN_LEN);
	ui->cargoTitleEdit->setMaxLength(NAME_LENGTH - 1);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this] { updateUi(false); });
	connect(viewport->editor, &Editor::currentObjectChanged, this, &ShipEditorDialog::update);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &ShipEditorDialog::update);
	connect(viewport->editor, &Editor::missionChanged, this, &ShipEditorDialog::update);

	// The on_arrivalTree_*/on_departureTree_* slots are auto-connected by
	// setupUi's connectSlotsByName; connecting them here again would run each
	// handler twice per signal (and push duplicate undo commands).

	// Column One
	connect(ui->cargoCombo->lineEdit(), (&QLineEdit::editingFinished), this, &ShipEditorDialog::cargoChanged);
	connect(ui->cargoTitleEdit, (&QLineEdit::editingFinished), this, &ShipEditorDialog::cargoTitleChanged);
	connect(ui->altNameCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::altNameChanged);
	connect(ui->callsignCombo->lineEdit(), (&QLineEdit::textEdited), this, &ShipEditorDialog::callsignChanged);

	// ui->cargoCombo->installEventFilter(this);

	// "Select Ship" menu: jump the editor to any ship (or player) in the mission.
	Editor* editor = viewport->editor;
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
			for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
					entries.push_back({QString::fromUtf8(Ships[ptr->instance].ship_name), OBJ_INDEX(ptr)});
				}
			}
			return entries;
		},
		[this, editor]() { return _model->getIfMultipleShips() ? -1 : editor->currentObject; },
		[editor](int objnum) {
			editor->unmark_all();
			editor->selectObject(objnum);
		},
		tr("&Select Ship"));

	updateUi(true);

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
	QDialog::closeEvent(e);
}

void ShipEditorDialog::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && isActiveWindow())
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	QDialog::changeEvent(e);
}

void ShipEditorDialog::hideEvent(QHideEvent* e)
{
	QDialog::hideEvent(e);
}
void ShipEditorDialog::showEvent(QShowEvent* e)
{
	_model->initializeData();
	updateUi(true);
	QDialog::showEvent(e);
}

void ShipEditorDialog::on_miscButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipFlagsSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.push_back(captureShipFlagsSnapshot(sig, inst));
	});

	auto* dlg = new dialogs::ShipFlagsDialog(this, _viewport);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() mutable {
		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o >= 0) after.push_back(captureShipFlagsSnapshot(b.sig, Objects[o].instance));
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_Flags, _viewport->editor, tr("Edit Ship Flags"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [editor = _viewport->editor](const SnapshotVec& v) {
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				int inst = Objects[o].instance;
				Ships[inst].flags = s.shipFlags;
				Ai_info[Ships[inst].ai_index].ai_flags = s.aiFlags;
				Objects[o].flags = s.objFlags;
				Ai_info[Ships[inst].ai_index].kamikaze_damage = s.kamikazeDamage;
				Ships[inst].escort_priority = s.escortPriority;
				Ships[inst].final_death_time = s.finalDeathTime;
				// sync Reinforcements[] vector via editor helper
				editor->set_reinforcement(s.shipName.c_str(),
					s.shipFlags[Ship::Ship_Flags::Reinforcement] ? 1 : 0);
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}

void ShipEditorDialog::on_initialStatusButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipInitialStatusSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.push_back(captureShipInitialStatusSnapshot(sig, inst));
	});

	auto* dlg = new dialogs::ShipInitialStatusDialog(this, _viewport, getIfMultipleShips());
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	// accepted() fires after apply() but before destroyed().  Retrieve the
	// pre-apply dockee positions the dialog captured and stash them so the
	// destroyed() handler can store them in before.extraPositions.
	using ExtraPos = ShipInitialStatusSnapshot::ExtraPosition;
	auto preApply = std::make_shared<SCP_vector<ExtraPos>>();
	connect(dlg, &QDialog::accepted, this, [dlg, preApply]() {
		for (const auto& p : dlg->preApplyDockeePositions())
			preApply->push_back({ p.sig, p.pos, p.orient });
	}, Qt::DirectConnection);

	connect(dlg, &QObject::destroyed, this,
		[this, before = std::move(before), preApply]() mutable {

		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o >= 0) after.push_back(captureShipInitialStatusSnapshot(b.sig, Objects[o].instance));
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		// For each ship that is newly docked in 'after' (not in 'before'),
		// store its pre-apply position in before.extraPositions so undo can
		// move it back.  preApply was populated in the accepted() handler
		// before apply() physically moved the ships.
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			for (const auto& aDL : after[i].dockLinks) {
				bool wasDocked = false;
				for (const auto& bDL : before[i].dockLinks)
					if (bDL.otherSig == aDL.otherSig) { wasDocked = true; break; }
				if (!wasDocked) {
					auto it = std::find_if(preApply->begin(), preApply->end(),
						[&](const ExtraPos& ep) { return ep.sig == aDL.otherSig; });
					if (it != preApply->end())
						before[i].extraPositions.push_back(*it);
				}
			}
		}

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_InitialStatus, _viewport->editor, tr("Edit Ship Initial Status"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const SnapshotVec& v) {
			// Undock all ships in the snapshot first
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o >= 0) dock_undock_all(&Objects[o]);
			}
			// Restore per-ship state including position, so dock-related moves
			// (dockee repositioned by ai_dock_with_object / ships moved apart on
			// undock) are properly reversed
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				int inst = Objects[o].instance;
				Objects[o].pos = s.pos;
				Objects[o].orient = s.orient;
				Objects[o].phys_info.speed = s.speed;
				if (!Objects[o].shield_quadrant.empty())
					Objects[o].shield_quadrant[0] = s.shieldQuadrant0;
				Objects[o].hull_strength = s.hullStrength;
				Ships[inst].ship_guardian_threshold = s.guardianThreshold;
				Ships[inst].flags = s.shipFlags;
				Objects[o].flags = s.objFlags;
				Ships[inst].team_name = s.teamName;
				Ships[inst].arrival_cue = s.arrivalCue;
				int ss_idx = 0;
				for (auto* ss = GET_FIRST(&Ships[inst].subsys_list);
				     ss != END_OF_LIST(&Ships[inst].subsys_list) && ss_idx < static_cast<int>(s.subsystems.size());
				     ss = GET_NEXT(ss), ss_idx++) {
					ss->current_hits = s.subsystems[ss_idx].currentHits;
					ss->subsys_cargo_name = s.subsystems[ss_idx].subsysCargoName;
					strcpy_s(ss->subsys_cargo_title, s.subsystems[ss_idx].subsysCargoTitle);
				}
				// Restore dockee positions (ships already in dockLinks) and any
				// newly-docked ships whose pre-docking position was captured via
				// the dialog's pre-apply hook.  Both must be set before
				// dock_dock_objects, which only sets the list without moving ships.
				for (const auto& dl : s.dockLinks) {
					int other_o = obj_get_by_signature(dl.otherSig);
					if (other_o < 0) continue;
					Objects[other_o].pos = dl.otherPos;
					Objects[other_o].orient = dl.otherOrient;
				}
				for (const auto& ep : s.extraPositions) {
					int ep_o = obj_get_by_signature(ep.sig);
					if (ep_o >= 0) {
						Objects[ep_o].pos = ep.pos;
						Objects[ep_o].orient = ep.orient;
					}
				}
			}
			// Re-establish dock links. Each link is established at most once:
			// both dockpoints must still be free, so when both partners are in
			// the snapshot the second one skips the already-made link. A
			// partner that is NOT in the snapshot (unmarked ship) is docked
			// here too — its side never runs, so no ordering test is allowed.
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				for (const auto& dl : s.dockLinks) {
					int other_o = obj_get_by_signature(dl.otherSig);
					if (other_o < 0) continue;
					if (dock_find_object_at_dockpoint(&Objects[o], dl.thisDockpoint) == nullptr &&
					    dock_find_object_at_dockpoint(&Objects[other_o], dl.otherDockpoint) == nullptr) {
						dock_dock_objects(&Objects[o], dl.thisDockpoint,
						                  &Objects[other_o], dl.otherDockpoint);
					}
				}
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}

void ShipEditorDialog::on_initialOrdersButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipGoalsSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		ShipGoalsSnapshot snap;
		snap.sig = sig;
		int ai_idx = Ships[inst].ai_index;
		for (int i = 0; i < MAX_AI_GOALS; i++)
			snap.goals[i] = Ai_info[ai_idx].goals[i];
		before.push_back(snap);
	});

	// Pass ship index directly (getSingleShip() returns Ships[] index, -1 for multi-edit)
	auto* dlg = new dialogs::ShipGoalsDialog(this, _viewport, getIfMultipleShips(), getSingleShip(), -1);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() mutable {
		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o < 0) continue;
			int inst = Objects[o].instance;
			ShipGoalsSnapshot snap;
			snap.sig = b.sig;
			int ai_idx = Ships[inst].ai_index;
			for (int i = 0; i < MAX_AI_GOALS; i++)
				snap.goals[i] = Ai_info[ai_idx].goals[i];
			after.push_back(snap);
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_InitialOrders, _viewport->editor, tr("Edit Ship Initial Orders"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const SnapshotVec& v) {
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				int inst = Objects[o].instance;
				int ai_idx = Ships[inst].ai_index;
				for (int i = 0; i < MAX_AI_GOALS; i++)
					Ai_info[ai_idx].goals[i] = s.goals[i];
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}

void ShipEditorDialog::on_tblInfoButton_clicked()
{
	auto dialog = new TableViewerDialog(this, _viewport, "Ship TBL Data",
	                                     "ships.tbl", "*-shp.tbm", Ship_info[getShipClass()].name);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void ShipEditorDialog::update()
{
	if (this->isVisible()) {
		_model->initializeData();
		updateUi(true);
	}
}

void ShipEditorDialog::updateUi(bool overwrite)
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
		ui->cargoTitleEdit->setText(_model->getCargoTitle().c_str());
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
				int altNameIdx = ui->altNameCombo->findText(QString(altname.c_str()));
				if (altNameIdx >= 0) {
					ui->altNameCombo->setCurrentIndex(altNameIdx);
				} else {
					ui->altNameCombo->setEditText("<none>");
				}
			}
		}
	}
	if (_model->getNumSelectedObjects()) {
		if (_model->getIfMultipleShips()) {
			ui->callsignCombo->setEnabled(false);
		} else {
			auto callsign = _model->getCallsign();
			ui->callsignCombo->setEnabled(true);
			if (overwrite) {
				ui->callsignCombo->clear();
				ui->callsignCombo->addItem("<none>");
				for (auto j = 0; j < Mission_callsign_count; j++) {
					SCP_string current = Mission_callsigns[j];
					ui->callsignCombo->addItem(Mission_callsigns[j], current.c_str());
				}
				int callsignIdx = ui->callsignCombo->findText(QString(callsign.c_str()));
				if (callsignIdx >= 0) {
					ui->callsignCombo->setCurrentIndex(callsignIdx);
				} else {
					ui->callsignCombo->setEditText("<none>");
				}
			}
		}
	}

	// Layer combo — always rebuild so it reflects current mission layers
	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}
	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));
	ui->layerCombo->setEnabled(_model->getNumSelectedObjects() > 0);
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
		ui->hotkeyCombo->setCurrentIndex(_model->getHotkey());
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
				stuff_special_arrival_anchor_name(tmp, static_cast<int>(j), restrict_to_players, false);

				ui->arrivalTargetCombo->addItem(tmp, QVariant(get_special_anchor(tmp).value()));
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

		ui->arrivalTree->initializeEditor(_viewport->editor, this, _viewport, _fredView);
		if (_model->getNumSelectedShips()) {

			if (_model->getIfMultipleShips()) {
				ui->arrivalTree->clear_tree("");
			}
			if (_model->getUseCue()) {
				ui->arrivalTree->load_tree(_model->getArrivalFormula());
				ui->arrivalTree->expandAll();
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

		ui->noArrivalWarpCheckBox->setCheckState(Qt::CheckState(_model->getNoArrivalWarp()));
		ui->dockWarpinCheckBox->setCheckState(Qt::CheckState(_model->getDockWarpinChange()));
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

		ui->departureTree->initializeEditor(_viewport->editor, this, _viewport, _fredView);
		if (_model->getNumSelectedShips()) {

			if (_model->getIfMultipleShips()) {
				ui->departureTree->clear_tree("");
			}
			if (_model->getUseCue()) {
				ui->departureTree->load_tree(_model->getDepartureFormula(), "false");
				ui->departureTree->expandAll();
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

		ui->noDepartureWarpCheckBox->setCheckState(Qt::CheckState(_model->getNoDepartureWarp()));
		ui->dockWarpoutCheckBox->setCheckState(Qt::CheckState(_model->getDockWarpoutChange()));

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

		ui->dockWarpinCheckBox->setEnabled(false);
		ui->dockWarpoutCheckBox->setEnabled(false);

		ui->restrictArrivalPathsButton->setEnabled(false);
		ui->restrictDeparturePathsButton->setEnabled(false);
	} else {
		ui->arrivalLocationCombo->setEnabled(_model->getUIEnable());
		ui->arrivalDistanceEdit->setEnabled(_model->getUIEnable() && _model->arrivalNeedsDistance());
		ui->arrivalTargetCombo->setEnabled(_model->getUIEnable() && _model->arrivalNeedsTarget());
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
		ui->noArrivalWarpCheckBox->setEnabled(_model->getUIEnable() && !_model->getPlayer());
		ui->noDepartureWarpCheckBox->setEnabled(_model->getUIEnable());

		ui->dockWarpinCheckBox->setEnabled(_model->getUIEnable() && !_model->getPlayer());
		ui->dockWarpoutCheckBox->setEnabled(_model->getUIEnable());
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

	// disable textures unless exactly one ship/player is selected
	ui->textureReplacementButton->setEnabled(_model->getNumSelectedObjects() == 1);

	ui->AIClassCombo->setEnabled(_model->getUIEnable());
	ui->cargoCombo->setEnabled(_model->getUIEnable());
	ui->cargoTitleEdit->setEnabled(_model->getUIEnable());
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
		const bool isPlayerWing = _model->wingIsPlayerWing(Ships[marked_ship].wingnum);
		if (!(The_mission.game_type & MISSION_TYPE_MULTI) && (_model->getNumSelectedObjects() > 0) &&
			(_model->getIfMultipleShips() != true) && (isPlayerWing == true))
			ui->playerShipButton->setEnabled(true);
		else
			ui->playerShipButton->setEnabled(false);
	}

	const bool noPlayerSelected = (_model->getNumSelectedPlayers() == 0);
	ui->deleteButton->setEnabled(_model->getUIEnable() && noPlayerSelected);
	ui->resetButton->setEnabled(_model->getUIEnable() && noPlayerSelected);
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

	if (_model->getNumSelectedPlayers() > 0) {
		// player ships don't take orders from the player
		ui->playerOrdersButton->setEnabled(false);
	} else if (_model->getIfMultipleShips() || (_model->getNumSelectedObjects() > 1)) {
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

// ---------------------------------------------------------------------------
// Column one private slot helpers
// ---------------------------------------------------------------------------

void ShipEditorDialog::cargoChanged()
{
	const QString entry = ui->cargoCombo->lineEdit()->text();
	if (entry.isEmpty() || entry == _model->getCargo().c_str())
		return;

	const SCP_string newCargo = entry.toUtf8().constData();

	// Capture before (cargo1 index per marked ship)
	SCP_vector<std::pair<int,char>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].cargo1);
	});

	_model->setCargo(newCargo);

	auto* cmd = new FieldEditCommand<char>(FieldId::Ship_Cargo, _viewport->editor, tr("Edit Ship Cargo"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		char after = Ships[Objects[o].instance].cargo1;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const char& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) Ships[Objects[o2].instance].cargo1 = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::cargoTitleChanged()
{
	const QString entry = ui->cargoTitleEdit->text();
	if (entry.isEmpty() || entry == _model->getCargoTitle().c_str())
		return;

	const SCP_string newTitle = entry.toUtf8().constData();

	SCP_vector<std::pair<int,SCP_string>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, SCP_string(Ships[inst].cargo_title));
	});

	_model->setCargoTitle(newTitle);

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Ship_CargoTitle, _viewport->editor, tr("Edit Cargo Title"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		SCP_string after(Ships[Objects[o].instance].cargo_title);
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const SCP_string& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) strcpy_s(Ships[Objects[o2].instance].cargo_title, v.c_str());
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::altNameChanged()
{
	const QString entry = ui->altNameCombo->lineEdit()->text();
	if (entry.isEmpty() || entry == _model->getAltName().c_str())
		return;

	const SCP_string newAltName = entry.toUtf8().constData();
	int si = _model->getSingleShip();
	if (_model->getIfMultipleShips() || si < 0) {
		_model->setAltName(newAltName);
		return;
	}

	int sig = Objects[Ships[si].objnum].signature;
	SCP_string before(Fred_alt_names[si]);

	_model->setAltName(newAltName);

	SCP_string after(Fred_alt_names[si]);
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Ship_AltName, _viewport->editor, tr("Edit Ship Alt Name"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->addEntry(before, after, [sig = sig](const SCP_string& v) {
		int o = obj_get_by_signature(sig);
		if (o >= 0) strcpy_s(Fred_alt_names[Objects[o].instance], v.c_str());
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::callsignChanged()
{
	const QString entry = ui->callsignCombo->lineEdit()->text();
	if (entry.isEmpty() || entry == _model->getCallsign().c_str())
		return;

	const SCP_string newCallsign = entry.toUtf8().constData();
	int si = _model->getSingleShip();
	if (_model->getIfMultipleShips() || si < 0) {
		_model->setCallsign(newCallsign);
		return;
	}

	int sig = Objects[Ships[si].objnum].signature;
	SCP_string before(Fred_callsigns[si]);

	_model->setCallsign(newCallsign);

	SCP_string after(Fred_callsigns[si]);
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Ship_Callsign, _viewport->editor, tr("Edit Ship Callsign"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->addEntry(before, after, [sig = sig](const SCP_string& v) {
		int o = obj_get_by_signature(sig);
		if (o >= 0) strcpy_s(Fred_callsigns[Objects[o].instance], v.c_str());
	});
	_fredView->mainUndoStack()->push(cmd);
}

// ---------------------------------------------------------------------------
// Button slots
// ---------------------------------------------------------------------------

void ShipEditorDialog::on_textureReplacementButton_clicked()
{
	// In multi-edit the texture dialog writes entries for EVERY marked ship,
	// so the undo capture must cover them all — one command per ship, grouped
	// into a single undo step.
	struct PerShip {
		SCP_string name;
		SCP_vector<texture_replace> before;
	};
	auto ships = std::make_shared<SCP_vector<PerShip>>();
	forEachMarkedShip([&](int /*sig*/, int inst) {
		PerShip ps;
		ps.name = Ships[inst].ship_name;
		for (const auto& tr : Fred_texture_replacements)
			if (!tr.from_table && !stricmp(tr.ship_name, ps.name.c_str()))
				ps.before.push_back(tr);
		ships->push_back(std::move(ps));
	});

	auto dialog = new dialogs::ShipTextureReplacementDialog(this, _viewport, getIfMultipleShips());
	dialog->setAttribute(Qt::WA_DeleteOnClose);

	// On accept: capture after-state per ship and push the undo command(s).
	// Nothing is heap-allocated before this point, so reject needs no cleanup.
	connect(dialog, &QDialog::accepted, this, [this, ships]() {
		auto sameEntries = [](const SCP_vector<texture_replace>& a, const SCP_vector<texture_replace>& b) {
			if (a.size() != b.size()) return false;
			for (size_t i = 0; i < a.size(); i++) {
				if (stricmp(a[i].old_texture, b[i].old_texture) != 0 ||
				    stricmp(a[i].new_texture, b[i].new_texture) != 0)
					return false;
			}
			return true;
		};

		const bool grouped = ships->size() > 1;
		bool macroOpen = false;
		for (auto& ps : *ships) {
			SCP_vector<texture_replace> after;
			for (const auto& tr : Fred_texture_replacements)
				if (!tr.from_table && !stricmp(tr.ship_name, ps.name.c_str()))
					after.push_back(tr);
			if (sameEntries(ps.before, after))
				continue; // this ship's entries didn't change
			if (grouped && !macroOpen) {
				_fredView->mainUndoStack()->beginMacro(tr("Texture Replacement"));
				macroOpen = true;
			}
			auto* cmd = new fso::fred::TextureReplacementCommand(ps.name, std::move(ps.before), _viewport->editor);
			cmd->setAfter(std::move(after));
			_fredView->mainUndoStack()->push(cmd);
		}
		if (macroOpen)
			_fredView->mainUndoStack()->endMacro();
	});

	dialog->show();
}

void ShipEditorDialog::on_playerShipButton_clicked()
{
	// "Set as Player Ship" in single-player mode: makeSolePlayerStart() makes
	// this ship the one and only player start, demoting every other start —
	// so capture the player state of ALL ships, not just the marked ones.
	using PlayerStateVec = SCP_vector<std::pair<int,bool>>;
	PlayerStateVec before;
	for (auto* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
		if (p->type == OBJ_SHIP || p->type == OBJ_START)
			before.emplace_back(p->signature, p->type == OBJ_START);
	}

	_model->makeSolePlayerStart();

	PlayerStateVec after;
	for (auto* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
		if (p->type == OBJ_SHIP || p->type == OBJ_START)
			after.emplace_back(p->signature, p->type == OBJ_START);
	}

	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<PlayerStateVec>(FieldId::Ship_Player, _viewport->editor, tr("Set Player Ship"), true);
	cmd->setTargetKey(markedShipsKey());
	// Never merge: the multiplayer checkbox pushes a marked-ships payload under
	// the same fieldId; a merge would apply the wrong-shaped vector.
	cmd->setNoMerge();
	cmd->addEntry(before, after, [](const PlayerStateVec& v) {
		for (const auto& [sig, isPlayer] : v) {
			int o = obj_get_by_signature(sig);
			if (o < 0) continue;
			int inst = Objects[o].instance;
			// Keep the Player_ship object flag in sync with the type, mirroring
			// set_single_player_start().
			if (isPlayer) {
				if (Objects[Ships[inst].objnum].type != OBJ_START) Player_starts++;
				Objects[Ships[inst].objnum].type = OBJ_START;
				Objects[Ships[inst].objnum].flags.set(Object::Object_Flags::Player_ship);
			} else {
				if (Objects[Ships[inst].objnum].type == OBJ_START) Player_starts--;
				Objects[Ships[inst].objnum].type = OBJ_SHIP;
				Objects[Ships[inst].objnum].flags.remove(Object::Object_Flags::Player_ship);
			}
		}
		ensure_valid_player_start_shipnum();
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_altShipClassButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipAltClassSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.push_back({sig, Ships[inst].s_alt_classes});
	});

	auto* dlg = new dialogs::ShipAltShipClass(this, _viewport);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() mutable {
		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o >= 0) after.push_back({b.sig, Ships[Objects[o].instance].s_alt_classes});
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_AltClass, _viewport->editor, tr("Edit Alternate Ship Classes"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const SnapshotVec& v) {
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o >= 0) Ships[Objects[o].instance].s_alt_classes = s.altClasses;
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}
void ShipEditorDialog::on_prevButton_clicked()
{
	_model->onPrevious();
}
void ShipEditorDialog::on_nextButton_clicked()
{
	_model->onNext();
}
void ShipEditorDialog::on_resetButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipResetSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.push_back(captureShipResetSnapshot(sig, inst));
	});
	if (before.empty()) return;

	_model->onShipReset();

	SnapshotVec after;
	forEachMarkedShip([&](int sig, int inst) {
		after.push_back(captureShipResetSnapshot(sig, inst));
	});

	auto* cmd = new FieldEditCommand<SnapshotVec>(
		FieldId::Ship_Reset, _viewport->editor, tr("Reset Ship"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->setNoMerge();
	cmd->addEntry(std::move(before), std::move(after), [](const SnapshotVec& v) {
		for (const auto& s : v) {
			int o = obj_get_by_signature(s.sig);
			if (o < 0) continue;
			int inst = Objects[o].instance;
			Ships[inst].cargo1 = s.cargo1;
			Ships[inst].team = s.team;
			Ships[inst].weapons.ai_class = s.aiClass;
			const int ai_idx = Ships[inst].ai_index;
			for (int i = 0; i < MAX_AI_GOALS; i++)
				Ai_info[ai_idx].goals[i] = s.goals[i];
			Objects[o].phys_info.speed = s.speed;
			if (!Objects[o].shield_quadrant.empty())
				Objects[o].shield_quadrant[0] = s.shieldQuadrant0;
			Objects[o].hull_strength = s.hullStrength;
			for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
				Ships[inst].weapons.primary_bank_weapons[i] = s.primaryBankWeapons[i];
			for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
				Ships[inst].weapons.secondary_bank_weapons[i] = s.secondaryBankWeapons[i];
				Ships[inst].weapons.secondary_bank_capacity[i] = s.secondaryBankCapacity[i];
			}
			int ss_idx = 0, turret_idx = 0;
			for (auto* ss = GET_FIRST(&Ships[inst].subsys_list); ss != END_OF_LIST(&Ships[inst].subsys_list); ss = GET_NEXT(ss), ss_idx++) {
				if (ss_idx < static_cast<int>(s.subsysHits.size()))
					ss->current_hits = s.subsysHits[ss_idx];
				if (ss->system_info && ss->system_info->type == SUBSYSTEM_TURRET &&
				    turret_idx < static_cast<int>(s.turrets.size())) {
					const auto& ts = s.turrets[turret_idx++];
					ss->weapons.num_primary_banks = ts.numPrimaryBanks;
					for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
						ss->weapons.primary_bank_weapons[i] = ts.primaryBankWeapons[i];
					ss->weapons.num_secondary_banks = ts.numSecondaryBanks;
					for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
						ss->weapons.secondary_bank_weapons[i] = ts.secondaryBankWeapons[i];
						ss->weapons.secondary_bank_capacity[i] = ts.secondaryBankCapacity[i];
						ss->weapons.secondary_bank_ammo[i] = ts.secondaryBankAmmo[i];
					}
				}
			}
		}
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_deleteButton_clicked()
{
	if (_viewport->editor->getNumMarked() <= 0) return;
	auto* cmd = new DeleteObjectsCommand(_viewport->editor, _viewport);
	_model->onDeleteShip(); // calls delete_marked() + unmark_all()
	if (!cmd->isEmpty() && _viewport->editor->getNumMarked() == 0) {
		_fredView->mainUndoStack()->push(cmd); // first redo() is a no-op
	} else {
		delete cmd;
	}
}
void ShipEditorDialog::on_weaponsButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipWeaponsSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.push_back(captureShipWeaponsSnapshot(sig, inst));
	});

	auto* dlg = new dialogs::ShipWeaponsDialog(this, _viewport, getIfMultipleShips());
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() mutable {
		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o >= 0) after.push_back(captureShipWeaponsSnapshot(b.sig, Objects[o].instance));
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_Weapons, _viewport->editor, tr("Edit Ship Weapons"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const SnapshotVec& v) {
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				int inst = Objects[o].instance;
				restoreWeaponBankState(Ships[inst].weapons, s.pilotWeapons);
				int turret_idx = 0;
				for (auto* ss = GET_FIRST(&Ships[inst].subsys_list);
				     ss != END_OF_LIST(&Ships[inst].subsys_list) && turret_idx < static_cast<int>(s.turretWeapons.size());
				     ss = GET_NEXT(ss)) {
					if (ss->system_info && ss->system_info->type == SUBSYSTEM_TURRET) {
						restoreWeaponBankState(ss->weapons, s.turretWeapons[turret_idx]);
						turret_idx++;
					}
				}
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}
void ShipEditorDialog::on_playerOrdersButton_clicked()
{
	// Capture orders_accepted set per marked ship before showing dialog
	using OrdersState = SCP_vector<std::pair<int, SCP_set<size_t>>>;
	OrdersState before;
	forEachMarkedShip([&](int sig, int inst) {
		before.emplace_back(sig, Ships[inst].orders_accepted);
	});

	QVector<std::pair<QString, int>> toWidget;
	for (const auto& p : _model->getPlayerOrders())
		toWidget.append({QString::fromUtf8(p.first.c_str()), p.second});

	dialogs::CheckBoxListDialog dlg(this);
	dlg.setCaption(tr("Player Orders Accepted"));
	dlg.setTristate(true);
	dlg.setOptions(toWidget);

	if (dlg.exec() != QDialog::Accepted)
		return;

	SCP_vector<std::pair<SCP_string, int>> orders;
	for (const auto& [name, state] : dlg.getFlags())
		orders.emplace_back(name.toUtf8().constData(), state);
	_model->applyPlayerOrders(orders);

	// Capture after state using same signatures
	OrdersState after;
	for (const auto& [sig, _] : before) {
		int o = obj_get_by_signature(sig);
		if (o >= 0) after.emplace_back(sig, Ships[Objects[o].instance].orders_accepted);
	}

	bool anyChanged = false;
	for (size_t i = 0; i < before.size() && i < after.size(); i++) {
		if (before[i].second != after[i].second) { anyChanged = true; break; }
	}
	if (!anyChanged)
		return;

	auto* cmd = new FieldEditCommand<OrdersState>(FieldId::Ship_PlayerOrders, _viewport->editor, tr("Edit Player Orders"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->setNoMerge();
	cmd->addEntry(before, after, [](const OrdersState& v) {
		for (const auto& [sig, accepted] : v) {
			int o = obj_get_by_signature(sig);
			if (o >= 0) Ships[Objects[o].instance].orders_accepted = accepted;
		}
	});
	_fredView->mainUndoStack()->push(cmd);
}
void ShipEditorDialog::on_specialStatsButton_clicked()
{
	using SnapshotVec = SCP_vector<ShipSpecialStatsSnapshot>;
	SnapshotVec before;
	forEachMarkedShip([&](int sig, int inst) {
		ShipSpecialStatsSnapshot s;
		s.sig = sig;
		s.specialHitpoints = Ships[inst].special_hitpoints;
		s.specialShield    = Ships[inst].special_shield;
		s.useSpecialExplosion     = Ships[inst].use_special_explosion;
		s.specialExpInner         = Ships[inst].special_exp_inner;
		s.specialExpOuter         = Ships[inst].special_exp_outer;
		s.specialExpDamage        = Ships[inst].special_exp_damage;
		s.specialExpBlast         = Ships[inst].special_exp_blast;
		s.useShockwave            = Ships[inst].use_shockwave;
		s.specialExpShockwaveSpeed   = Ships[inst].special_exp_shockwave_speed;
		s.specialExpDeathrollTime    = Ships[inst].special_exp_deathroll_time;
		s.kamikazeDamage          = Ai_info[Ships[inst].ai_index].kamikaze_damage;
		before.push_back(s);
	});

	auto* dlg = new dialogs::ShipSpecialStatsDialog(this, _viewport);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() mutable {
		SnapshotVec after;
		for (const auto& b : before) {
			int o = obj_get_by_signature(b.sig);
			if (o < 0) continue;
			int inst = Objects[o].instance;
			ShipSpecialStatsSnapshot s;
			s.sig = b.sig;
			s.specialHitpoints = Ships[inst].special_hitpoints;
			s.specialShield    = Ships[inst].special_shield;
			s.useSpecialExplosion     = Ships[inst].use_special_explosion;
			s.specialExpInner         = Ships[inst].special_exp_inner;
			s.specialExpOuter         = Ships[inst].special_exp_outer;
			s.specialExpDamage        = Ships[inst].special_exp_damage;
			s.specialExpBlast         = Ships[inst].special_exp_blast;
			s.useShockwave            = Ships[inst].use_shockwave;
			s.specialExpShockwaveSpeed   = Ships[inst].special_exp_shockwave_speed;
			s.specialExpDeathrollTime    = Ships[inst].special_exp_deathroll_time;
			s.kamikazeDamage          = Ai_info[Ships[inst].ai_index].kamikaze_damage;
			after.push_back(s);
		}
		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (!(before[i] == after[i])) { anyChanged = true; break; }
		}
		if (!anyChanged) return;

		auto* cmd = new FieldEditCommand<SnapshotVec>(
			FieldId::Ship_SpecialStats, _viewport->editor, tr("Edit Ship Special Stats"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const SnapshotVec& v) {
			for (const auto& s : v) {
				int o = obj_get_by_signature(s.sig);
				if (o < 0) continue;
				int inst = Objects[o].instance;
				Ships[inst].special_hitpoints        = s.specialHitpoints;
				Ships[inst].special_shield           = s.specialShield;
				Ships[inst].use_special_explosion    = s.useSpecialExplosion;
				Ships[inst].special_exp_inner        = s.specialExpInner;
				Ships[inst].special_exp_outer        = s.specialExpOuter;
				Ships[inst].special_exp_damage       = s.specialExpDamage;
				Ships[inst].special_exp_blast        = s.specialExpBlast;
				Ships[inst].use_shockwave            = s.useShockwave;
				Ships[inst].special_exp_shockwave_speed  = s.specialExpShockwaveSpeed;
				Ships[inst].special_exp_deathroll_time   = s.specialExpDeathrollTime;
				Ai_info[Ships[inst].ai_index].kamikaze_damage = s.kamikazeDamage;
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}
void ShipEditorDialog::on_hideCuesButton_clicked()
{
	const auto showHelp = _viewport->Show_sexp_help_ship_editor;

	_cues_hidden = !_cues_hidden;

	ui->arrivalGroupBox->setVisible(!_cues_hidden);
	ui->departureGroupBox->setVisible(!_cues_hidden);
	ui->HelpTitle->setVisible(!_cues_hidden && showHelp);
	ui->helpText->setVisible(!_cues_hidden && showHelp);
	ui->hideCuesButton->setText(_cues_hidden ? "Show Cues" : "Hide Cues");

	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	resize(sizeHint());
}

// ---------------------------------------------------------------------------
// Arrival / departure path restriction dialogs (modal inline)
// ---------------------------------------------------------------------------

void ShipEditorDialog::on_restrictArrivalPathsButton_clicked()
{
	int si = _model->getSingleShip();
	if (si < 0) return;
	int sig = Objects[Ships[si].objnum].signature;
	int beforeMask = Ships[si].arrival_path_mask;

	CheckBoxListDialog dlg(this);
	dlg.setCaption("Restrict Arrival Paths");
	auto arrivalPaths = _model->getArrivalPaths();

	QVector<std::pair<QString, bool>> checkbox_list;
	for (const auto& path : arrivalPaths) {
		checkbox_list.append({path.first.c_str(), path.second});
	}
	dlg.setOptions(checkbox_list);
	if (dlg.exec() != QDialog::Accepted)
		return;

	auto returned_values = dlg.getCheckedStates();
	SCP_vector<std::pair<SCP_string, bool>> updatedPaths;
	for (int i = 0; i < checkbox_list.size(); ++i) {
		SCP_string name = checkbox_list[i].first.toUtf8().constData();
		updatedPaths.emplace_back(name, returned_values[i]);
	}

	_model->setArrivalPaths(updatedPaths);

	int afterMask = Ships[si].arrival_path_mask;
	if (beforeMask == afterMask)
		return;

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_ArrivalPaths, _viewport->editor, tr("Edit Arrival Paths"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->setNoMerge();
	cmd->addEntry(beforeMask, afterMask, [sig = sig](const int& v) {
		int o = obj_get_by_signature(sig);
		if (o >= 0) Ships[Objects[o].instance].arrival_path_mask = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_customWarpinButton_clicked()
{
	// Capture warpin_params_index per marked ship before opening dialog
	using WarpState = SCP_vector<std::pair<int,int>>;
	WarpState before;
	forEachMarkedShip([&](int sig, int inst) {
		before.emplace_back(sig, Ships[inst].warpin_params_index);
	});

	auto* dlg = new dialogs::ShipCustomWarpDialog(this, _viewport, false);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() {
		// Capture after state using captured signatures
		WarpState after;
		for (const auto& [sig, _] : before) {
			int o = obj_get_by_signature(sig);
			if (o >= 0) after.emplace_back(sig, Ships[Objects[o].instance].warpin_params_index);
		}

		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (before[i].second != after[i].second) { anyChanged = true; break; }
		}
		if (!anyChanged)
			return;

		auto* cmd = new FieldEditCommand<WarpState>(
			FieldId::Ship_WarpinParams, _viewport->editor, tr("Edit Warp-In Parameters"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const WarpState& v) {
			for (const auto& [sig, idx] : v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].warpin_params_index = idx;
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}

void ShipEditorDialog::on_restrictDeparturePathsButton_clicked()
{
	int si = _model->getSingleShip();
	if (si < 0) return;
	int sig = Objects[Ships[si].objnum].signature;
	int beforeMask = Ships[si].departure_path_mask;

	CheckBoxListDialog dlg(this);
	dlg.setCaption("Restrict Departure Paths");
	auto departurePaths = _model->getDeparturePaths();

	QVector<std::pair<QString, bool>> checkbox_list;
	for (const auto& path : departurePaths) {
		checkbox_list.append({path.first.c_str(), path.second});
	}
	dlg.setOptions(checkbox_list);
	if (dlg.exec() != QDialog::Accepted)
		return;

	auto returned_values = dlg.getCheckedStates();
	SCP_vector<std::pair<SCP_string, bool>> updatedPaths;
	for (int i = 0; i < checkbox_list.size(); ++i) {
		SCP_string name = checkbox_list[i].first.toUtf8().constData();
		updatedPaths.emplace_back(name, returned_values[i]);
	}

	_model->setDeparturePaths(updatedPaths);

	int afterMask = Ships[si].departure_path_mask;
	if (beforeMask == afterMask)
		return;

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_DeparturePaths, _viewport->editor, tr("Edit Departure Paths"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->setNoMerge();
	cmd->addEntry(beforeMask, afterMask, [sig = sig](const int& v) {
		int o = obj_get_by_signature(sig);
		if (o >= 0) Ships[Objects[o].instance].departure_path_mask = v;
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_customWarpoutButton_clicked()
{
	// Capture warpout_params_index per marked ship before opening dialog
	using WarpState = SCP_vector<std::pair<int,int>>;
	WarpState before;
	forEachMarkedShip([&](int sig, int inst) {
		before.emplace_back(sig, Ships[inst].warpout_params_index);
	});

	auto* dlg = new dialogs::ShipCustomWarpDialog(this, _viewport, true);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();

	connect(dlg, &QObject::destroyed, this, [this, before = std::move(before)]() {
		// Capture after state using captured signatures
		WarpState after;
		for (const auto& [sig, _] : before) {
			int o = obj_get_by_signature(sig);
			if (o >= 0) after.emplace_back(sig, Ships[Objects[o].instance].warpout_params_index);
		}

		bool anyChanged = false;
		for (size_t i = 0; i < before.size() && i < after.size(); i++) {
			if (before[i].second != after[i].second) { anyChanged = true; break; }
		}
		if (!anyChanged)
			return;

		auto* cmd = new FieldEditCommand<WarpState>(
			FieldId::Ship_WarpoutParams, _viewport->editor, tr("Edit Warp-Out Parameters"), false);
		cmd->setTargetKey(markedShipsKey());
		cmd->setNoMerge();
		cmd->addEntry(before, after, [](const WarpState& v) {
			for (const auto& [sig, idx] : v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].warpout_params_index = idx;
			}
		});
		_fredView->mainUndoStack()->push(cmd);
	});
}

// ---------------------------------------------------------------------------
// Column one combo/edit slots
// ---------------------------------------------------------------------------

/*---------------------------------------------------------
					WARNING
Do not try to optimise string entries; this convoluted method is necessary to avoid fatal errors caused by QT
-----------------------------------------------------------*/
void ShipEditorDialog::on_shipNameEdit_editingFinished()
{
	const QString entry = ui->shipNameEdit->text();
	if (!entry.isEmpty() && entry != _model->getShipName().c_str()) {
		const SCP_string oldName = _model->getShipName();
		const SCP_string newName = entry.toUtf8().constData();

		const bool single = !_model->getIfMultipleShips() && _model->getSingleShip() >= 0;
		const int si = single ? _model->getSingleShip() : -1;
		const int sig = single ? Objects[Ships[si].objnum].signature : -1;
		ShipDisplayNameState dnBefore;
		if (single) {
			dnBefore = {Ships[si].display_name, Ships[si].flags[Ship::Ship_Flags::Has_display_name]};
		}

		_model->setShipName(newName);
		const SCP_string appliedName = _model->getShipName();

		// automatically determine or reset the display name
		_model->setShipDisplayName(Editor::get_display_name_for_text_box(_model->getShipName()));

		// Only push undo if validation passed, name changed, and single-ship mode
		if (appliedName != oldName && single) {
			// If the rename also reset a hash-derived display name, record that
			// write in the same undo step so Ctrl+Z restores both.
			const ShipDisplayNameState dnAfter{Ships[si].display_name,
				Ships[si].flags[Ship::Ship_Flags::Has_display_name]};
			const bool dnChanged = !(dnBefore == dnAfter);

			if (dnChanged)
				_fredView->mainUndoStack()->beginMacro(tr("Rename Ship"));
			_fredView->mainUndoStack()->push(
				new RenameObjectCommand(Ships[si].objnum, oldName, appliedName, _viewport->editor, true));
			if (dnChanged) {
				auto* cmd = new FieldEditCommand<ShipDisplayNameState>(
					FieldId::Ship_DisplayName, _viewport->editor, tr("Edit Ship Display Name"), true);
				cmd->setTargetKey(markedShipsKey());
				cmd->setNoMerge();
				cmd->addEntry(dnBefore, dnAfter, [sig](const ShipDisplayNameState& v) {
					int o = obj_get_by_signature(sig);
					if (o < 0) return;
					int inst = Objects[o].instance;
					Ships[inst].display_name = v.displayName;
					if (v.hasFlag) Ships[inst].flags.set(Ship::Ship_Flags::Has_display_name);
					else Ships[inst].flags.remove(Ship::Ship_Flags::Has_display_name);
				});
				_fredView->mainUndoStack()->push(cmd);
				_fredView->mainUndoStack()->endMacro();
			}
		}
	} else {
		// Name unchanged: keep the pre-existing derive/reset behavior.
		_model->setShipDisplayName(Editor::get_display_name_for_text_box(_model->getShipName()));
	}

	// sync the variable to the edit box
	ui->shipDisplayNameEdit->setText(_model->getShipDisplayName().c_str());
}

void ShipEditorDialog::on_shipDisplayNameEdit_editingFinished()
{
	const QString entry = ui->shipDisplayNameEdit->text();
	if (entry == _model->getShipDisplayName().c_str())
		return;

	int si = _model->getSingleShip();
	if (_model->getIfMultipleShips() || si < 0) {
		_model->setShipDisplayName(entry.toUtf8().constData());
		return;
	}

	int sig = Objects[Ships[si].objnum].signature;
	ShipDisplayNameState before{Ships[si].display_name, Ships[si].flags[Ship::Ship_Flags::Has_display_name]};

	_model->setShipDisplayName(entry.toUtf8().constData());

	ShipDisplayNameState after{Ships[si].display_name, Ships[si].flags[Ship::Ship_Flags::Has_display_name]};
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<ShipDisplayNameState>(
		FieldId::Ship_DisplayName, _viewport->editor, tr("Edit Ship Display Name"), true);
	cmd->setTargetKey(markedShipsKey());
	cmd->addEntry(before, after, [sig = sig](const ShipDisplayNameState& v) {
		int o = obj_get_by_signature(sig);
		if (o < 0) return;
		int inst = Objects[o].instance;
		Ships[inst].display_name = v.displayName;
		if (v.hasFlag) Ships[inst].flags.set(Ship::Ship_Flags::Has_display_name);
		else Ships[inst].flags.remove(Ship::Ship_Flags::Has_display_name);
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_shipClassCombo_currentIndexChanged(int index)
{
	auto newClass = ui->shipClassCombo->itemData(index).toInt();
	if (newClass < 0) return;

	// Capture before (ship_info_index per marked ship)
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].ship_info_index);
	});

	_model->setShipClass(newClass);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Class, _viewport->editor, tr("Change Ship Class"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		int after = Ships[Objects[o].instance].ship_info_index;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const int& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) change_ship_type(Objects[o2].instance, v);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_AIClassCombo_currentIndexChanged(int index)
{
	auto aiClassIdx = ui->AIClassCombo->itemData(index).toInt();

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].weapons.ai_class);
	});

	_model->setAIClass(aiClassIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_AIClass, _viewport->editor, tr("Change AI Class"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != aiClassIdx) {
			anyChanged = true;
			cmd->addEntry(before, aiClassIdx, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].weapons.ai_class = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_teamCombo_currentIndexChanged(int index)
{
	auto teamIdx = ui->teamCombo->itemData(index).toInt();
	if (teamIdx < 0) return;

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].team);
	});

	_model->setTeam(teamIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Team, _viewport->editor, tr("Change Ship Team"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != teamIdx) {
			anyChanged = true;
			cmd->addEntry(before, teamIdx, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].team = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	SCP_string layerName = ui->layerCombo->itemData(index).toString().toUtf8().constData();

	SCP_vector<ObjectLayerChange> changes;
	for (auto* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
		if ((p->type == OBJ_SHIP || p->type == OBJ_START) && p->flags[Object::Object_Flags::Marked]) {
			SCP_string before = _viewport->getObjectLayerName(OBJ_INDEX(p));
			if (before != layerName)
				changes.push_back({p->signature, std::move(before), layerName});
		}
	}

	_model->setLayer(layerName);

	if (!changes.empty())
		_fredView->mainUndoStack()->push(new MoveLayerCommand(std::move(changes), _viewport, _viewport->editor));
}

// ---------------------------------------------------------------------------
// Column two slots
// ---------------------------------------------------------------------------

void ShipEditorDialog::on_hotkeyCombo_currentIndexChanged(int index)
{
	// index = 1-indexed hotkey (0 = none); Ships[].hotkey is 0-indexed (-1 = none)
	int newHotkey = index - 1; // what Ships[].hotkey will be

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].hotkey);
	});

	_model->setHotkey(index);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Hotkey, _viewport->editor, tr("Change Ship Hotkey"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newHotkey) {
			anyChanged = true;
			cmd->addEntry(before, newHotkey, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].hotkey = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_personaCombo_currentIndexChanged(int index)
{
	auto personaIdx = ui->personaCombo->itemData(index).toInt();

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].persona_index);
	});

	_model->setPersona(personaIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Persona, _viewport->editor, tr("Change Ship Persona"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		int after = Ships[Objects[o].instance].persona_index;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const int& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) Ships[Objects[o2].instance].persona_index = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_killScoreEdit_valueChanged(int value)
{
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].score);
	});

	_model->setScore(value);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Score, _viewport->editor, tr("Edit Kill Score"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != value) {
			anyChanged = true;
			cmd->addEntry(before, value, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].score = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_assistEdit_valueChanged(int value)
{
	float newPct = std::clamp(static_cast<float>(value) / 100.0f, 0.0f, 1.0f);

	SCP_vector<std::pair<int,float>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].assist_score_pct);
	});

	_model->setAssist(value);

	auto* cmd = new FieldEditCommand<float>(FieldId::Ship_Assist, _viewport->editor, tr("Edit Assist Score"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newPct) {
			anyChanged = true;
			cmd->addEntry(before, newPct, [sig = sig](const float& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].assist_score_pct = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_playerShipCheckBox_toggled(bool value)
{
	using PlayerStateVec = SCP_vector<std::pair<int,bool>>;
	PlayerStateVec before;
	forEachMarkedShip([&](int sig, int inst) {
		before.emplace_back(sig, Objects[Ships[inst].objnum].type == OBJ_START);
	});

	_model->setPlayer(value);

	PlayerStateVec after;
	forEachMarkedShip([&](int sig, int inst) {
		after.emplace_back(sig, Objects[Ships[inst].objnum].type == OBJ_START);
	});

	bool anyChanged = false;
	for (size_t i = 0; i < before.size() && i < after.size(); i++) {
		if (before[i].second != after[i].second) { anyChanged = true; break; }
	}
	if (!anyChanged)
		return;

	auto* cmd = new FieldEditCommand<PlayerStateVec>(FieldId::Ship_Player, _viewport->editor, tr("Toggle Player Ship"), true);
	cmd->setTargetKey(markedShipsKey());
	// Never merge: the single-player button pushes an all-ships payload under
	// the same fieldId; a merge would apply the wrong-shaped vector.
	cmd->setNoMerge();
	cmd->addEntry(before, after, [](const PlayerStateVec& v) {
		for (const auto& [sig, isPlayer] : v) {
			int o = obj_get_by_signature(sig);
			if (o < 0) continue;
			int inst = Objects[o].instance;
			if (isPlayer) {
				if (Objects[Ships[inst].objnum].type != OBJ_START) Player_starts++;
				Objects[Ships[inst].objnum].type = OBJ_START;
			} else {
				if (Objects[Ships[inst].objnum].type == OBJ_START) Player_starts--;
				Objects[Ships[inst].objnum].type = OBJ_SHIP;
			}
		}
		ensure_valid_player_start_shipnum();
	});
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_respawnSpinBox_valueChanged(int value)
{
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].respawn_priority);
	});

	_model->setRespawn(value);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_Respawn, _viewport->editor, tr("Edit Respawn Priority"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != value) {
			anyChanged = true;
			cmd->addEntry(before, value, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].respawn_priority = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

// ---------------------------------------------------------------------------
// Arrival slots
// ---------------------------------------------------------------------------

void ShipEditorDialog::on_arrivalLocationCombo_currentIndexChanged(int index)
{
	auto newLocation = ui->arrivalLocationCombo->itemData(index).toInt();
	if (newLocation < 0) return;

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, static_cast<int>(Ships[inst].arrival_location));
	});

	_model->setArrivalLocationIndex(newLocation);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_ArrivalLocation, _viewport->editor, tr("Change Arrival Location"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newLocation) {
			anyChanged = true;
			cmd->addEntry(before, newLocation, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].arrival_location = static_cast<ArrivalLocation>(v);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_arrivalTargetCombo_currentIndexChanged(int index)
{
	auto newTarget = ui->arrivalTargetCombo->itemData(index).toInt();

	SCP_vector<std::pair<int, anchor_t>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, Ships[inst].arrival_anchor);
	});

	_model->setArrivalTarget(newTarget);

	auto* cmd = new FieldEditCommand<anchor_t>(FieldId::Ship_ArrivalTarget, _viewport->editor, tr("Change Arrival Target"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		anchor_t after = Ships[Objects[o].instance].arrival_anchor;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const anchor_t& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) Ships[Objects[o2].instance].arrival_anchor = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_arrivalDistanceEdit_valueChanged(int value)
{
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, Ships[inst].arrival_distance);
	});

	_model->setArrivalDistance(value);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_ArrivalDistance, _viewport->editor, tr("Edit Arrival Distance"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		int after = Ships[Objects[o].instance].arrival_distance;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const int& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) Ships[Objects[o2].instance].arrival_distance = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_arrivalDelaySpinBox_valueChanged(int value)
{
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, Ships[inst].arrival_delay);
	});

	_model->setArrivalDelay(value);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_ArrivalDelay, _viewport->editor, tr("Edit Arrival Delay"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != value) {
			anyChanged = true;
			cmd->addEntry(before, value, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].arrival_delay = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_updateArrivalCueCheckBox_toggled(bool value)
{
	// _updateArrival is UI-only state (controls multi-edit sexp tree display), no Ships[] data
	_model->setArrivalCue(value);
}

void ShipEditorDialog::on_noArrivalWarpCheckBox_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
		return;

	bool newValue = (state == Qt::Checked);

	SCP_vector<std::pair<int,bool>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].flags[Ship::Ship_Flags::No_arrival_warp]);
	});

	_model->setNoArrivalWarp(state);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ship_NoArrivalWarp, _viewport->editor, tr("Toggle No Arrival Warp"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newValue) {
			anyChanged = true;
			cmd->addEntry(before, newValue, [sig = sig](const bool& v) {
				int o = obj_get_by_signature(sig);
				if (o < 0) return;
				int inst = Objects[o].instance;
				if (v) Ships[inst].flags.set(Ship::Ship_Flags::No_arrival_warp);
				else Ships[inst].flags.remove(Ship::Ship_Flags::No_arrival_warp);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}
void ShipEditorDialog::on_arrivalTree_modified()
{
	// Multi-edit without "update cue" checked: the model setter is a no-op.
	if (_model->getIfMultipleShips() && !_model->getArrivalCue()) {
		_model->setArrivalTreeDirty(ui->arrivalTree->_model.save_tree());
		return;
	}

	auto* cmd = new SexpCueEditCommand(_viewport->editor, tr("Edit Arrival Cue"), true);
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum >= 0)
			return;
		cmd->addOwner(Ships[inst].arrival_cue,
			[sig]() {
				const int n = obj_get_by_signature(sig);
				return n < 0 ? -1 : Ships[Objects[n].instance].arrival_cue;
			},
			[sig](int formula) {
				const int n = obj_get_by_signature(sig);
				if (n >= 0)
					Ships[Objects[n].instance].arrival_cue = formula;
			});
	});

	const int newFormula = ui->arrivalTree->_model.save_tree();
	_model->setArrivalTreeDirty(newFormula);

	if (cmd->isEmpty()) {
		delete cmd;
		return;
	}
	cmd->captureAfter(newFormula);
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_arrivalTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}

void ShipEditorDialog::on_arrivalTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}
void ShipEditorDialog::on_dockWarpinCheckBox_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
		return;

	bool newValue = (state == Qt::Checked);

	SCP_vector<std::pair<int,bool>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].flags[Ship::Ship_Flags::Same_arrival_warp_when_docked]);
	});

	_model->setDockWarpinChange(state);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ship_DockWarpin, _viewport->editor, tr("Toggle Same Arrival Warp When Docked"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newValue) {
			anyChanged = true;
			cmd->addEntry(before, newValue, [sig = sig](const bool& v) {
				int o = obj_get_by_signature(sig);
				if (o < 0) return;
				int inst = Objects[o].instance;
				if (v) Ships[inst].flags.set(Ship::Ship_Flags::Same_arrival_warp_when_docked);
				else Ships[inst].flags.remove(Ship::Ship_Flags::Same_arrival_warp_when_docked);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

// ---------------------------------------------------------------------------
// Departure slots
// ---------------------------------------------------------------------------

void ShipEditorDialog::on_departureLocationCombo_currentIndexChanged(int index)
{
	auto newLocation = ui->departureLocationCombo->itemData(index).toInt();
	if (newLocation < 0) return;

	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, static_cast<int>(Ships[inst].departure_location));
	});

	_model->setDepartureLocationIndex(newLocation);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_DepartureLocation, _viewport->editor, tr("Change Departure Location"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newLocation) {
			anyChanged = true;
			cmd->addEntry(before, newLocation, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].departure_location = static_cast<DepartureLocation>(v);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_departureTargetCombo_currentIndexChanged(int index)
{
	auto newTarget = ui->departureTargetCombo->itemData(index).toInt();

	SCP_vector<std::pair<int, anchor_t>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, Ships[inst].departure_anchor);
	});

	_model->setDepartureTarget(newTarget);

	auto* cmd = new FieldEditCommand<anchor_t>(FieldId::Ship_DepartureTarget, _viewport->editor, tr("Change Departure Target"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		int o = obj_get_by_signature(sig);
		if (o < 0) continue;
		anchor_t after = Ships[Objects[o].instance].departure_anchor;
		if (before != after) {
			anyChanged = true;
			cmd->addEntry(before, after, [sig = sig](const anchor_t& v) {
				int o2 = obj_get_by_signature(sig);
				if (o2 >= 0) Ships[Objects[o2].instance].departure_anchor = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_departureDelaySpinBox_valueChanged(int value)
{
	SCP_vector<std::pair<int,int>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum < 0)
			beforeList.emplace_back(sig, Ships[inst].departure_delay);
	});

	_model->setDepartureDelay(value);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ship_DepartureDelay, _viewport->editor, tr("Edit Departure Delay"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != value) {
			anyChanged = true;
			cmd->addEntry(before, value, [sig = sig](const int& v) {
				int o = obj_get_by_signature(sig);
				if (o >= 0) Ships[Objects[o].instance].departure_delay = v;
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

void ShipEditorDialog::on_updateDepartureCueCheckBox_toggled(bool value)
{
	// _updateDeparture is UI-only state, no Ships[] data
	_model->setDepartureCue(value);
}
void fred::dialogs::ShipEditorDialog::on_departureTree_modified()
{
	// Multi-edit without "update cue" checked: the model setter is a no-op.
	if (_model->getIfMultipleShips() && !_model->getDepartureCue()) {
		_model->setDepartureTreeDirty(ui->departureTree->_model.save_tree());
		return;
	}

	auto* cmd = new SexpCueEditCommand(_viewport->editor, tr("Edit Departure Cue"), true);
	forEachMarkedShip([&](int sig, int inst) {
		if (Ships[inst].wingnum >= 0)
			return;
		cmd->addOwner(Ships[inst].departure_cue,
			[sig]() {
				const int n = obj_get_by_signature(sig);
				return n < 0 ? -1 : Ships[Objects[n].instance].departure_cue;
			},
			[sig](int formula) {
				const int n = obj_get_by_signature(sig);
				if (n >= 0)
					Ships[Objects[n].instance].departure_cue = formula;
			});
	});

	const int newFormula = ui->departureTree->_model.save_tree();
	_model->setDepartureTreeDirty(newFormula);

	if (cmd->isEmpty()) {
		delete cmd;
		return;
	}
	cmd->captureAfter(newFormula);
	_fredView->mainUndoStack()->push(cmd);
}

void ShipEditorDialog::on_departureTree_helpChanged(const QString& help)
{
	ui->helpText->setPlainText(help);
}

void ShipEditorDialog::on_departureTree_miniHelpChanged(const QString& help)
{
	ui->HelpTitle->setText(help);
}

void ShipEditorDialog::on_noDepartureWarpCheckBox_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
		return;

	bool newValue = (state == Qt::Checked);

	SCP_vector<std::pair<int,bool>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].flags[Ship::Ship_Flags::No_departure_warp]);
	});

	_model->setNoDepartureWarp(state);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ship_NoDepartureWarp, _viewport->editor, tr("Toggle No Departure Warp"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newValue) {
			anyChanged = true;
			cmd->addEntry(before, newValue, [sig = sig](const bool& v) {
				int o = obj_get_by_signature(sig);
				if (o < 0) return;
				int inst = Objects[o].instance;
				if (v) Ships[inst].flags.set(Ship::Ship_Flags::No_departure_warp);
				else Ships[inst].flags.remove(Ship::Ship_Flags::No_departure_warp);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}
void ShipEditorDialog::on_dockWarpoutCheckBox_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
		return;

	bool newValue = (state == Qt::Checked);

	SCP_vector<std::pair<int,bool>> beforeList;
	forEachMarkedShip([&](int sig, int inst) {
		beforeList.emplace_back(sig, Ships[inst].flags[Ship::Ship_Flags::Same_departure_warp_when_docked]);
	});

	_model->setDockWarpoutChange(state);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ship_DockWarpout, _viewport->editor, tr("Toggle Same Departure Warp When Docked"), true);
	cmd->setTargetKey(markedShipsKey());
	bool anyChanged = false;
	for (const auto& [sig, before] : beforeList) {
		if (before != newValue) {
			anyChanged = true;
			cmd->addEntry(before, newValue, [sig = sig](const bool& v) {
				int o = obj_get_by_signature(sig);
				if (o < 0) return;
				int inst = Objects[o].instance;
				if (v) Ships[inst].flags.set(Ship::Ship_Flags::Same_departure_warp_when_docked);
				else Ships[inst].flags.remove(Ship::Ship_Flags::Same_departure_warp_when_docked);
			});
		}
	}
	if (anyChanged)
		_fredView->mainUndoStack()->push(cmd);
	else
		delete cmd;
}

} // namespace fso::fred::dialogs
