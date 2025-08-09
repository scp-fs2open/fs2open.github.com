#include "ShipWeaponsDialogModel.h"
namespace fso::fred {
Banks::Banks(SCP_string _name, int aiIndex, int _ship, int multiedit, ship_subsys* _subsys)
	: m_isMultiEdit(multiedit), name(std::move(_name)), subsys(_subsys), initalAI(aiIndex), ship(_ship)
{
	aiClass = aiIndex;
}
void Banks::add(Bank* bank)
{
	banks.push_back(bank);
}
Bank* Banks::getByBankId(const int id)
{
	for (auto bank : banks) {
		if (id == bank->getWeaponId())
			return bank;
	}
	return nullptr;
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
	return banks;
}
int Banks::getAiClass() const
{
	if (name == "Pilot") {
		return Ships[ship].weapons.ai_class;
	} else {
		return subsys->weapons.ai_class;
	}
}
void Banks::setAiClass(int newClass)
{
	if (m_isMultiEdit) {
		object* ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				int inst = ptr->instance;
				if (name == "Pilot") {
					Ships[inst].ai_index = newClass;
				} else {
					subsys->weapons.ai_class = newClass;
				}
			}
			ptr = GET_NEXT(ptr);
		}
	} else {
		if (name == "Pilot") {
			Ships[ship].weapons.ai_class = newClass;
		} else {
			subsys->weapons.ai_class = newClass;
		}
	}
}
int Banks::getInitalAI() const
{
	return initalAI;
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
	if (Weapon_info[id].subtype == WP_LASER || Weapon_info[id].subtype == WP_BEAM) {
		if (parent->getName() == "Pilot") {
			ammoMax = get_max_ammo_count_for_primary_bank(parent->getShip(), bankId, id);
		} else {
			ammoMax = get_max_ammo_count_for_primary_turret_bank(&parent->getSubsys()->weapons, bankId, id);
		}
	} else {
		if (parent->getName() == "Pilot") {
			ammoMax = get_max_ammo_count_for_bank(parent->getShip(), bankId, id);
		} else {
			ammoMax = get_max_ammo_count_for_turret_bank(&parent->getSubsys()->weapons, bankId, id);
		}
	}
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
void ShipWeaponsDialogModel::initializeData(bool isMultiEdit)
{
	m_isMultiEdit = isMultiEdit;
	PrimaryBanks.clear();
	SecondaryBanks.clear();

	m_ship = _editor->cur_ship;
	if (m_ship == -1)
		m_ship = Objects[_editor->currentObject].instance;

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
				// initTertiary(inst, first);
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
}

void ShipWeaponsDialogModel::initPrimary(int inst, bool first)
{
	auto pilotBank = new Banks("Pilot", Ships[inst].weapons.ai_class, inst, m_isMultiEdit);
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (pilot.primary_bank_weapons[i] >= 0) {
				const int maxAmmo =
					get_max_ammo_count_for_primary_bank(Ships[inst].ship_info_index, i, pilot.primary_bank_weapons[i]);
				const int ammo = fl2ir(pilot.primary_bank_ammo[i] * maxAmmo / 100.0f);
				pilotBank->add(new Bank(pilot.primary_bank_weapons[i], i, maxAmmo, ammo, pilotBank));
			}
		}
		PrimaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss->weapons.ai_class, inst, m_isMultiEdit, pss);
				for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
					if (pss->weapons.primary_bank_weapons[i] >= 0) {
						const int maxAmmo = get_max_ammo_count_for_primary_turret_bank(&pss->weapons,
							i,
							pss->weapons.primary_bank_weapons[i]);
						const int ammo = fl2ir(pss->weapons.primary_bank_ammo[i] * maxAmmo / 100.0f);
						turretBank->add(new Bank(pss->weapons.primary_bank_weapons[i], i, maxAmmo, ammo, turretBank));
					}
				}
				if (!turretBank->empty()) {
					PrimaryBanks.push_back(turretBank);
				} else {
					delete turretBank;
				}
			}
		}
	} else {
		for (int i = 0; i < static_cast<int>(PrimaryBanks[0]->getBanks().size()); i++) {
			if (PrimaryBanks[0]->getBanks()[i]->getWeaponId() != Ships[inst].weapons.primary_bank_weapons[i]) {
				PrimaryBanks[0]->getBanks()[i]->setWeapon(-2);
			}
			if (PrimaryBanks[0]->getBanks()[i]->getAmmo() != Ships[inst].weapons.primary_bank_ammo[i]) {
				PrimaryBanks[0]->getBanks()[i]->setAmmo(-2);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				for (auto banks : PrimaryBanks) {
					if (banks->getSubsys() == pss) {
						for (int i = 0; i < static_cast<int>(banks->getBanks().size()); i++) {
							if (banks->getBanks()[i]->getWeaponId() != pss->weapons.primary_bank_weapons[i]) {
								banks->getBanks()[i]->setWeapon(-2);
							}
							if (banks->getBanks()[i]->getAmmo() != pss->weapons.primary_bank_ammo[i]) {
								banks->getBanks()[i]->setAmmo(-2);
							}
						}
					}
				}
			}
		}
	}
}

void ShipWeaponsDialogModel::initSecondary(int inst, bool first)
{
	auto pilotBank = new Banks("Pilot", Ships[inst].weapons.ai_class, inst, m_isMultiEdit);
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (pilot.secondary_bank_weapons[i] >= 0) {
				const int maxAmmo =
					get_max_ammo_count_for_bank(Ships[inst].ship_info_index, i, pilot.secondary_bank_weapons[i]);
				const int ammo = fl2ir(pilot.secondary_bank_ammo[i] * maxAmmo / 100.0f);
				pilotBank->add(new Bank(pilot.secondary_bank_weapons[i], i, maxAmmo, ammo, pilotBank));
			}
		}
		SecondaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss->weapons.ai_class, inst, m_isMultiEdit, pss);
				for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
					if (pss->weapons.secondary_bank_weapons[i] >= 0) {
						const int maxAmmo = get_max_ammo_count_for_turret_bank(&pss->weapons,
							i,
							pss->weapons.secondary_bank_weapons[i]);
						const int ammo = fl2ir(pss->weapons.secondary_bank_ammo[i] * maxAmmo / 100.0f);
						turretBank->add(new Bank(pss->weapons.secondary_bank_weapons[i], i, maxAmmo, ammo, turretBank));
					}
				}
				if (!turretBank->empty()) {
					SecondaryBanks.push_back(turretBank);
				} else {
					delete turretBank;
				}
			}
		}
	} else {
		for (int i = 0; i < static_cast<int>(SecondaryBanks[0]->getBanks().size()); i++) {
			if (SecondaryBanks[0]->getBanks()[i]->getWeaponId() != Ships[inst].weapons.secondary_bank_weapons[i]) {
				SecondaryBanks[0]->getBanks()[i]->setWeapon(-2);
			}
			if (SecondaryBanks[0]->getBanks()[i]->getAmmo() != Ships[inst].weapons.secondary_bank_ammo[i]) {
				SecondaryBanks[0]->getBanks()[i]->setAmmo(-2);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				for (auto banks : SecondaryBanks) {
					if (banks->getSubsys() == pss) {
						for (int i = 0; i < static_cast<int>(banks->getBanks().size()); i++) {
							if (banks->getByBankId(i)->getWeaponId() != pss->weapons.secondary_bank_weapons[i]) {
								banks->getByBankId(i)->setWeapon(-2);
							}
							if (banks->getByBankId(i)->getAmmo() != pss->weapons.secondary_bank_ammo[i]) {
								banks->getByBankId(i)->setAmmo(-2);
							}
						}
					}
				}
			}
		}
	}
}
void ShipWeaponsDialogModel::saveShip(int inst)
{
	for (auto Turret : PrimaryBanks) {
		if (Turret->getName() == "Pilot") {
			for (auto bank : Turret->getBanks()) {
				if (bank->getWeaponId() != -2) {
					Ships[inst].weapons.primary_bank_weapons[bank->getBankId()] = bank->getWeaponId();
					Ships[inst].weapons.primary_bank_ammo[bank->getBankId()] =
						bank->getMaxAmmo() ? fl2ir(bank->getAmmo() * 100.0f / bank->getMaxAmmo()) : 0;
				}
			}
		} else {
			ship_subsys* pss = Turret->getSubsys();
			for (auto bank : Turret->getBanks()) {
				if (bank->getWeaponId() != -2) {
					pss->weapons.primary_bank_weapons[bank->getBankId()] = bank->getWeaponId();
					pss->weapons.primary_bank_ammo[bank->getBankId()] =
						bank->getMaxAmmo() ? fl2ir(bank->getAmmo() * 100.0f / bank->getMaxAmmo()) : 0;
				}
			}
		}
	}
	for (auto Turret : SecondaryBanks) {
		if (Turret->getName() == "Pilot") {
			for (auto bank : Turret->getBanks()) {
				if (bank->getWeaponId() != -2) {
					Ships[inst].weapons.secondary_bank_weapons[bank->getBankId()] = bank->getWeaponId();
					Ships[inst].weapons.secondary_bank_ammo[bank->getBankId()] =
						bank->getMaxAmmo() ? fl2ir(bank->getAmmo() * 100.0f / bank->getMaxAmmo()) : 0;
				}
			}
		} else {
			ship_subsys* pss = Turret->getSubsys();
			for (auto bank : Turret->getBanks()) {
				if (bank->getWeaponId() != -2) {
					pss->weapons.secondary_bank_weapons[bank->getBankId()] = bank->getWeaponId();
					pss->weapons.secondary_bank_ammo[bank->getBankId()] =
						bank->getMaxAmmo() ? fl2ir(bank->getAmmo() * 100.0f / bank->getMaxAmmo()) : 0;
				}
			}
		}
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
	for (auto Turret : PrimaryBanks) {
		Turret->setAiClass(Turret->getInitalAI());
	}
	for (auto Turret : SecondaryBanks) {
		Turret->setAiClass(Turret->getInitalAI());
	}
}
SCP_vector<Banks*> ShipWeaponsDialogModel::getPrimaryBanks() const
{
	return PrimaryBanks;
}
SCP_vector<Banks*> ShipWeaponsDialogModel::getSecondaryBanks() const
{
	return SecondaryBanks;
}
/* void ShipWeaponsDialogModel::initTertiary(int inst, bool first) {

}
*/
} // namespace dialogs
} // namespace fso::fred