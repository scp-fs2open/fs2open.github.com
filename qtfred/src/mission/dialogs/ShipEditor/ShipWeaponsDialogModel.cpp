#include "ShipWeaponsDialogModel.h"

#include <ship/ship.h>

namespace fso::fred {
namespace {
// Compare one slot of the "tracking" Bank (from the first ship's view) against the corresponding
// slot of a subsequent ship. Mark the bank as CONFLICT (-2) where they disagree, but never
// clobber an already-CONFLICT marker.
void reconcileSlot(Bank* bank, int otherWeaponId, int otherAmmoPct)
{
	if (bank->getWeaponId() == -2) {
		return;
	}
	if (bank->getWeaponId() != otherWeaponId) {
		bank->setWeapon(-2);
		return;
	}
	if (bank->getAmmo() == -2 || bank->getMaxAmmo() <= 0) {
		return;
	}
	const int otherAmmo = fl2ir(otherAmmoPct * bank->getMaxAmmo() / 100.0f);
	if (bank->getAmmo() != otherAmmo) {
		bank->setAmmo(-2);
	}
}

Banks* findBanksByName(const SCP_vector<std::unique_ptr<Banks>>& banks, const SCP_string& name)
{
	for (const auto& b : banks) {
		if (b->getName() == name) {
			return b.get();
		}
	}
	return nullptr;
}

ship_subsys* findTurretByName(int inst, const SCP_string& name)
{
	ship_subsys* ssl = &Ships[inst].subsys_list;
	for (ship_subsys* pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
		if (pss->system_info->type == SUBSYSTEM_TURRET &&
			SCP_string(pss->system_info->subobj_name) == name) {
			return pss;
		}
	}
	return nullptr;
}

void saveSlotsTo(ship_weapon& target, const SCP_vector<Bank*>& bankList, bool isPrimary)
{
	int* weapons = isPrimary ? target.primary_bank_weapons : target.secondary_bank_weapons;
	int* ammo = isPrimary ? target.primary_bank_ammo : target.secondary_bank_ammo;
	for (Bank* bank : bankList) {
		if (bank->getWeaponId() == -2) {
			continue; // weapon CONFLICT — preserve per-ship weapon
		}
		weapons[bank->getBankId()] = bank->getWeaponId();
		if (bank->getAmmo() != -2) {
			ammo[bank->getBankId()] = bank->getMaxAmmo() ? fl2ir(bank->getAmmo() * 100.0f / bank->getMaxAmmo()) : 0;
		}
		// else: ammo CONFLICT — preserve per-ship ammo
	}
}
} // namespace

Banks::Banks(SCP_string _name, int aiIndex, int _ship, int _id, ship_subsys* _subsys)
	: name(std::move(_name)), subsys(_subsys), currentAi(aiIndex), ship(_ship), id(_id)
{
}
Banks::~Banks() = default;
int Banks::getId() const
{
	return id;
}
void Banks::add(std::unique_ptr<Bank> bank)
{
	banks.push_back(std::move(bank));
}
SCP_string Banks::getName() const
{
	return name;
}
int Banks::getShip() const
{
	return ship;
}
ship_subsys* Banks::getSubsys() const
{
	return subsys;
}
bool Banks::empty() const
{
	return banks.empty();
}
SCP_vector<Bank*> Banks::getBanks() const
{
	SCP_vector<Bank*> result;
	result.reserve(banks.size());
	for (const auto& b : banks) {
		result.push_back(b.get());
	}
	return result;
}
int Banks::getAiClass() const
{
	return currentAi;
}
void Banks::setAiClass(int newClass)
{
	currentAi = newClass;
	aiClassDirty = true;
}
void Banks::reconcileAiClass(int otherAi)
{
	if (currentAi == -1) {
		return;
	}
	if (currentAi != otherAi) {
		currentAi = -1;
	}
}
bool Banks::isAiClassDirty() const
{
	return aiClassDirty;
}
Bank::Bank(const int _weaponId, const int _bankId, const int _ammoMax, const int _ammo, Banks* _parent)
{
	this->weaponId = _weaponId;
	this->bankId = _bankId;
	this->ammo = _ammo;
	this->ammoMax = _ammoMax;
	this->parent = _parent;
}
int Bank::getWeaponId() const
{
	return weaponId;
}
int Bank::getAmmo() const
{
	return ammo;
}
int Bank::getBankId() const
{
	return bankId;
}
int Bank::getMaxAmmo() const
{
	return ammoMax;
}
void Bank::setWeapon(const int id)
{
	weaponId = id;
	if (id < 0) {
		// "None" or CONFLICT placeholder... no weapon assigned, no ammo capacity.
		ammoMax = 0;
		ammo = 0;
		return;
	}
	const int shipClass = Ships[parent->getShip()].ship_info_index;
	if (Weapon_info[id].subtype == WP_LASER || Weapon_info[id].subtype == WP_BEAM) {
		if (parent->getName() == "Pilot") {
			ammoMax = get_max_ammo_count_for_primary_bank(shipClass, bankId, id);
		} else {
			ammoMax = get_max_ammo_count_for_primary_turret_bank(&parent->getSubsys()->weapons, bankId, id);
		}
	} else {
		if (parent->getName() == "Pilot") {
			ammoMax = get_max_ammo_count_for_bank(shipClass, bankId, id);
		} else {
			ammoMax = get_max_ammo_count_for_turret_bank(&parent->getSubsys()->weapons, bankId, id);
		}
	}
	ammo = ammoMax;
}
void Bank::setAmmo(const int newAmmo)
{
	this->ammo = newAmmo;
}
namespace dialogs {
ShipWeaponsDialogModel::ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool isMultiEdit)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(isMultiEdit);
}
ShipWeaponsDialogModel::~ShipWeaponsDialogModel() = default;
bool ShipWeaponsDialogModel::selectedShipsShareClass()
{
	int sharedClass = -1;
	for (object* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if ((ptr->type != OBJ_SHIP && ptr->type != OBJ_START) || !ptr->flags[Object::Object_Flags::Marked]) {
			continue;
		}
		const int cls = Ships[ptr->instance].ship_info_index;
		if (sharedClass < 0) {
			sharedClass = cls;
		} else if (cls != sharedClass) {
			return false;
		}
	}
	return true;
}
void ShipWeaponsDialogModel::initializeData(bool isMultiEdit)
{
	m_isMultiEdit = isMultiEdit;
	PrimaryBanks.clear();
	SecondaryBanks.clear();

	m_ship = _editor->cur_ship;
	if (m_ship == -1) {
		Assertion(_editor->currentObject >= 0 && _editor->currentObject < MAX_OBJECTS, // NOLINT(readability-simplify-boolean-expr)
			"ShipWeaponsDialog opened with no valid current ship and an out-of-range currentObject (%d)",
			_editor->currentObject);
		m_ship = Objects[_editor->currentObject].instance;
	}
	Assertion(m_ship >= 0 && m_ship < MAX_SHIPS, // NOLINT(readability-simplify-boolean-expr)
		"ShipWeaponsDialog resolved to invalid ship index %d", m_ship);

	if (m_isMultiEdit) {
		object* ptr = GET_FIRST(&obj_used_list);
		bool first = true;
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				int inst = ptr->instance;
				if (!(Ship_info[Ships[inst].ship_info_index].is_big_or_huge()))
					big = false;
				initPrimary(inst, first);
				initSecondary(inst, first);
				first = false;
			}
			ptr = GET_NEXT(ptr);
		}
	} else {
		if (!(Ship_info[Ships[m_ship].ship_info_index].is_big_or_huge()))
			big = false;
		initPrimary(m_ship, true);
		initSecondary(m_ship, true);
	}
	_modified = false;
}

void ShipWeaponsDialogModel::initPrimary(int inst, bool first)
{
	if (first) {
		int id = 0;
		auto pilotBank = std::make_unique<Banks>("Pilot", Ships[inst].weapons.ai_class, inst, id);
		id++;
		auto pilot = Ships[inst].weapons;
		const int shipClass = Ships[inst].ship_info_index;
		const int numPilotBanks = Ship_info[shipClass].num_primary_banks;
		for (int i = 0; i < numPilotBanks; i++) {
			const int weaponId = pilot.primary_bank_weapons[i];
			int maxAmmo = 0;
			int ammo = 0;
			if (weaponId >= 0) {
				maxAmmo = get_max_ammo_count_for_primary_bank(shipClass, i, weaponId);
				ammo = fl2ir(pilot.primary_bank_ammo[i] * maxAmmo / 100.0f);
			}
			pilotBank->add(std::make_unique<Bank>(weaponId, i, maxAmmo, ammo, pilotBank.get()));
		}
		if (!pilotBank->empty()) {
			PrimaryBanks.push_back(std::move(pilotBank));
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		for (ship_subsys* pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = std::make_unique<Banks>(psub->subobj_name, pss->weapons.ai_class, inst, id, pss);
				const int numTurretBanks = pss->weapons.num_primary_banks;
				for (int i = 0; i < numTurretBanks; i++) {
					const int weaponId = pss->weapons.primary_bank_weapons[i];
					int maxAmmo = 0;
					int ammo = 0;
					if (weaponId >= 0) {
						maxAmmo = get_max_ammo_count_for_primary_turret_bank(&pss->weapons, i, weaponId);
						ammo = fl2ir(pss->weapons.primary_bank_ammo[i] * maxAmmo / 100.0f);
					}
					turretBank->add(std::make_unique<Bank>(weaponId, i, maxAmmo, ammo, turretBank.get()));
				}
				if (!turretBank->empty()) {
					PrimaryBanks.push_back(std::move(turretBank));
					id++;
				}
			}
		}
	} else {
		// Subsequent ship: reconcile each slot against the tracking Banks built from the first ship.
		if (Banks* tracking = findBanksByName(PrimaryBanks, "Pilot")) {
			tracking->reconcileAiClass(Ships[inst].weapons.ai_class);
			const auto bankList = tracking->getBanks();
			for (size_t i = 0; i < bankList.size(); i++) {
				reconcileSlot(bankList[i], Ships[inst].weapons.primary_bank_weapons[i],
					Ships[inst].weapons.primary_bank_ammo[i]);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		for (ship_subsys* pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			if (pss->system_info->type != SUBSYSTEM_TURRET) {
				continue;
			}
			Banks* tracking = findBanksByName(PrimaryBanks, pss->system_info->subobj_name);
			if (tracking == nullptr) {
				continue;
			}
			tracking->reconcileAiClass(pss->weapons.ai_class);
			const auto bankList = tracking->getBanks();
			for (size_t i = 0; i < bankList.size(); i++) {
				reconcileSlot(bankList[i], pss->weapons.primary_bank_weapons[i],
					pss->weapons.primary_bank_ammo[i]);
			}
		}
	}
}

void ShipWeaponsDialogModel::initSecondary(int inst, bool first)
{
	if (first) {
		int id = 0;
		auto pilotBank = std::make_unique<Banks>("Pilot", Ships[inst].weapons.ai_class, inst, id);
		id++;
		auto pilot = Ships[inst].weapons;
		const int shipClass = Ships[inst].ship_info_index;
		const int numPilotBanks = Ship_info[shipClass].num_secondary_banks;
		for (int i = 0; i < numPilotBanks; i++) {
			const int weaponId = pilot.secondary_bank_weapons[i];
			int maxAmmo = 0;
			int ammo = 0;
			if (weaponId >= 0) {
				maxAmmo = get_max_ammo_count_for_bank(shipClass, i, weaponId);
				ammo = fl2ir(pilot.secondary_bank_ammo[i] * maxAmmo / 100.0f);
			}
			pilotBank->add(std::make_unique<Bank>(weaponId, i, maxAmmo, ammo, pilotBank.get()));
		}
		if (!pilotBank->empty()) {
			SecondaryBanks.push_back(std::move(pilotBank));
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		for (ship_subsys* pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = std::make_unique<Banks>(psub->subobj_name, pss->weapons.ai_class, inst, id, pss);
				const int numTurretBanks = pss->weapons.num_secondary_banks;
				for (int i = 0; i < numTurretBanks; i++) {
					const int weaponId = pss->weapons.secondary_bank_weapons[i];
					int maxAmmo = 0;
					int ammo = 0;
					if (weaponId >= 0) {
						maxAmmo = get_max_ammo_count_for_turret_bank(&pss->weapons, i, weaponId);
						ammo = fl2ir(pss->weapons.secondary_bank_ammo[i] * maxAmmo / 100.0f);
					}
					turretBank->add(std::make_unique<Bank>(weaponId, i, maxAmmo, ammo, turretBank.get()));
				}
				if (!turretBank->empty()) {
					SecondaryBanks.push_back(std::move(turretBank));
					id++;
				}
			}
		}
	} else {
		if (Banks* tracking = findBanksByName(SecondaryBanks, "Pilot")) {
			tracking->reconcileAiClass(Ships[inst].weapons.ai_class);
			const auto bankList = tracking->getBanks();
			for (size_t i = 0; i < bankList.size(); i++) {
				reconcileSlot(bankList[i], Ships[inst].weapons.secondary_bank_weapons[i],
					Ships[inst].weapons.secondary_bank_ammo[i]);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		for (ship_subsys* pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			if (pss->system_info->type != SUBSYSTEM_TURRET) {
				continue;
			}
			Banks* tracking = findBanksByName(SecondaryBanks, pss->system_info->subobj_name);
			if (tracking == nullptr) {
				continue;
			}
			tracking->reconcileAiClass(pss->weapons.ai_class);
			const auto bankList = tracking->getBanks();
			for (size_t i = 0; i < bankList.size(); i++) {
				reconcileSlot(bankList[i], pss->weapons.secondary_bank_weapons[i],
					pss->weapons.secondary_bank_ammo[i]);
			}
		}
	}
}
void ShipWeaponsDialogModel::saveShip(int inst)
{
	auto saveBank = [&](Banks* turret, bool isPrimary) {
		ship_weapon* target = nullptr;
		if (turret->getName() == "Pilot") {
			target = &Ships[inst].weapons;
		} else if (ship_subsys* pss = findTurretByName(inst, turret->getName())) {
			target = &pss->weapons;
		}
		if (target == nullptr) {
			return;
		}
		saveSlotsTo(*target, turret->getBanks(), isPrimary);
		if (turret->isAiClassDirty()) {
			target->ai_class = turret->getAiClass();
		}
	};
	for (const auto& turret : PrimaryBanks) {
		saveBank(turret.get(), true);
	}
	for (const auto& turret : SecondaryBanks) {
		saveBank(turret.get(), false);
	}
}
bool ShipWeaponsDialogModel::apply()
{
	if (m_isMultiEdit) {
		object* ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				int inst = ptr->instance;
				saveShip(inst);
			}
			ptr = GET_NEXT(ptr);
		}
	} else {
		saveShip(m_ship);
	}
	_editor->missionChanged();
	return true;
}
void ShipWeaponsDialogModel::reject()
{
	// Weapons, ammo, and AI class are all buffered in the Banks/Bank model and only written
	// to Ships[] in apply(). Cancel/close is therefore a true no-op as far as mission data
	// is concerned: the next dialog open re-reads ship state from scratch.
}
SCP_vector<Banks*> ShipWeaponsDialogModel::getPrimaryBanks() const
{
	SCP_vector<Banks*> result;
	result.reserve(PrimaryBanks.size());
	for (const auto& b : PrimaryBanks) {
		result.push_back(b.get());
	}
	return result;
}
SCP_vector<Banks*> ShipWeaponsDialogModel::getSecondaryBanks() const
{
	SCP_vector<Banks*> result;
	result.reserve(SecondaryBanks.size());
	for (const auto& b : SecondaryBanks) {
		result.push_back(b.get());
	}
	return result;
}
SCP_vector<WeaponItem> ShipWeaponsDialogModel::getAvailableWeapons(WeaponListType type) const
{
	SCP_vector<WeaponItem> result;
	result.push_back(WeaponItem{-1, "None", true});

	const int shipClass = getShipClass();
	const bool haveShipInfo = shipClass >= 0 && shipClass < ship_info_size();
	// allowed_weapons is a player-loadout concept and only meaningful for fighters/bombers.
	// On other ship classes (capships, support, etc.) every weapon is rendered as normal.
	const bool applyAllowedTint = haveShipInfo && Ship_info[shipClass].is_fighter_bomber();

	const bool isPrimary = (type == WeaponListType::Primary);
	const int wantedSubtype = isPrimary ? WP_LASER : WP_MISSILE;
	const bool acceptBeams = isPrimary;

	for (int i = 0; i < static_cast<int>(Weapon_info.size()); i++) {
		const auto& w = Weapon_info[i];
		if (w.wi_flags[Weapon::Info_Flags::No_fred]) {
			continue;
		}
		if (w.wi_flags[Weapon::Info_Flags::Child]) {
			continue;
		}
		const bool subtypeMatches = (w.subtype == wantedSubtype) || (acceptBeams && w.subtype == WP_BEAM);
		if (!subtypeMatches) {
			continue;
		}
		if (!big && w.wi_flags[Weapon::Info_Flags::Big_only]) {
			continue;
		}
		const bool allowed = !applyAllowedTint || Ship_info[shipClass].allowed_weapons[i] != 0;
		result.push_back(WeaponItem{i, w.name, allowed});
	}
	return result;
}
SCP_string ShipWeaponsDialogModel::getWeaponName(int weaponId)
{
	if (weaponId == -2) {
		return "CONFLICT";
	}
	if (weaponId < 0 || weaponId >= static_cast<int>(Weapon_info.size())) {
		return "None";
	}
	return Weapon_info[weaponId].name;
}
SCP_vector<SCP_string> ShipWeaponsDialogModel::getAiClassNames()
{
	SCP_vector<SCP_string> result;
	result.reserve(Num_ai_classes);
	for (int i = 0; i < Num_ai_classes; i++) {
		result.emplace_back(Ai_class_names[i]);
	}
	return result;
}
SCP_string ShipWeaponsDialogModel::getAiClassName(int aiClass)
{
	if (aiClass < 0 || aiClass >= Num_ai_classes) {
		return "";
	}
	return Ai_class_names[aiClass];
}
int ShipWeaponsDialogModel::getShipClass() const
{
	return Ships[m_ship].ship_info_index;
}
bool ShipWeaponsDialogModel::isBigShip() const
{
	return big;
}
void ShipWeaponsDialogModel::notifyChanged()
{
	set_modified();
	modelChanged();
}
} // namespace dialogs
} // namespace fso::fred
