#include "ShipWeaponsDialogModel.h"
namespace fso {
namespace fred {
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
	int inst;
	bool first = true;
	object* ptr;

	m_ship = _editor->cur_ship;
	if (m_ship == -1)
		m_ship = Objects[_editor->currentObject].instance;

	if (m_isMultiEdit) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				inst = ptr->instance;
				if (!(Ship_info[Ships[inst].ship_info_index].is_big_or_huge()))
					big = 0;
				initPrimary(inst, first);
				initSecondary(inst, first);
				// initTertiary(inst, first);
				first = false;
			}
		}
	} else {
		if (!(Ship_info[Ships[m_ship].ship_info_index].is_big_or_huge()))
			big = 0;
		initPrimary(m_ship, true);
		initSecondary(m_ship, true);
	}
}

void ShipWeaponsDialogModel::initPrimary(int inst, bool first)
{
	auto pilotBank = new Banks("Pilot", Ships[inst].ai_index);
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (pilot.primary_bank_weapons[i] >= 0) {
				int maxAmmo = get_max_ammo_count_for_primary_bank(Ships[inst].ship_info_index,
					i, pilot.primary_bank_weapons[i]);
				int ammo = fl2ir(pilot.primary_bank_ammo[i] * maxAmmo / 100.0f);
				pilotBank->add(new Bank(pilot.primary_bank_weapons[i], i, maxAmmo,ammo));
			}
		}
		PrimaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss->weapons.ai_class, pss);
				for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
					if (pss->weapons.primary_bank_weapons[i] >= 0) {
						int maxAmmo =
							get_max_ammo_count_for_primary_turret_bank(&pss->weapons,
							i,
							pss->weapons.primary_bank_weapons[i]);
						int ammo = fl2ir(pss->weapons.primary_bank_ammo[i] * maxAmmo / 100.0f);
						turretBank->add(new Bank(pss->weapons.primary_bank_weapons[i], i, maxAmmo, ammo));
					}
				}
				if (turretBank->banks.size() > 0) {
					PrimaryBanks.push_back(turretBank);
				}
			}
		}
	} else {
		for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (PrimaryBanks[0]->banks[i]->getWeaponId() != Ships[inst].weapons.primary_bank_weapons[i]) {
				PrimaryBanks[0]->getByBankId(i)->setWeapon(-2);
			}
			if (PrimaryBanks[0]->getByBankId(i)->getAmmo() != Ships[inst].weapons.primary_bank_ammo[i]) {
				PrimaryBanks[0]->getByBankId(i)->setAmmo(-2);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				for (auto banks : PrimaryBanks) {
					if (banks->subsys == pss) {
						for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
							if (banks->getByBankId(i)->getWeaponId() != pss->weapons.primary_bank_weapons[i]) {
								banks->getByBankId(i)->setWeapon(-2);
							}
							if (banks->getByBankId(i)->getAmmo() != pss->weapons.primary_bank_ammo[i]) {
								banks->getByBankId(i)->setAmmo(-2);
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
	auto pilotBank = new Banks("Pilot", Ships[inst].ai_index);
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (pilot.secondary_bank_weapons[i] >= 0) {
				int maxAmmo =
					get_max_ammo_count_for_bank(Ships[inst].ship_info_index, i, pilot.secondary_bank_weapons[i]);
				int ammo = fl2ir(pilot.secondary_bank_ammo[i] * maxAmmo / 100.0f);
				pilotBank->add(new Bank(pilot.secondary_bank_weapons[i], i, maxAmmo, ammo));
			}
		}
		SecondaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss->weapons.ai_class, pss);
				for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
					if (pss->weapons.secondary_bank_weapons[i] >= 0) {
						int maxAmmo = get_max_ammo_count_for_turret_bank(&pss->weapons,
							i,
							pss->weapons.secondary_bank_weapons[i]);
						int ammo = fl2ir(pss->weapons.secondary_bank_ammo[i] * maxAmmo / 100.0f);
						turretBank->add(new Bank(pss->weapons.secondary_bank_weapons[i], i, maxAmmo, ammo));
					}
				}
				if (turretBank->banks.size() > 0) {
					SecondaryBanks.push_back(turretBank);
				}
			}
		}
	} else {
		for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (SecondaryBanks[0]->banks[i]->getWeaponId() != Ships[inst].weapons.secondary_bank_weapons[i]) {
				SecondaryBanks[0]->getByBankId(i)->setWeapon(-2);
			}
			if (SecondaryBanks[0]->getByBankId(i)->getAmmo() != Ships[inst].weapons.secondary_bank_ammo[i]) {
				SecondaryBanks[0]->getByBankId(i)->setAmmo(-2);
			}
		}
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				for (auto banks : SecondaryBanks) {
					if (banks->subsys == pss) {
						for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
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
bool ShipWeaponsDialogModel::apply()
{
	return false;
}
void ShipWeaponsDialogModel::reject() {}
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
Banks::Banks(const SCP_string name, int aiIndex, ship_subsys* subsys) : subsys(subsys)
{
	this->name = std::move(name);
	aiClass = aiIndex;
}
void Banks::add(Bank* bank)
{
	banks.push_back(bank);
}
Bank* Banks::getByBankId(int id)
{
	for (auto bank : banks) {
		if (id == bank->getWeaponId())
			return bank;
	}
	return nullptr;
}
Bank::Bank(const int weaponId, const int bankId, const int ammoMax, const int ammo)
{
	this->weaponId = weaponId;
	this->bankId = bankId;
	this->ammo = ammo;
	this->ammoMax = ammoMax;
}
int Bank::getWeaponId() const
{
	return weaponId;
}
int Bank::getAmmo() const
{
	return ammo;
}
void Bank::setWeapon(const int id)
{
	weaponId = id;
}
void Bank::setAmmo(const int newAmmo)
{
	this->ammo = newAmmo;
}
} // namespace dialogs
} // namespace fred
} // namespace fso