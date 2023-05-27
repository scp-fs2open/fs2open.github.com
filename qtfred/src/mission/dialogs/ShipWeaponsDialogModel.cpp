#include "ShipWeaponsDialogModel.h"
namespace fso {
namespace fred {
namespace dialogs {
ShipWeaponsDialogModel::ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool multi)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(multi);
}
void ShipWeaponsDialogModel::initializeData(bool multi)
{
	m_multi = multi;
	int z, inst;
	bool first = true;
	object* ptr;
	model_subsystem* psub;
	ship_subsys *ssl, *pss;

	m_ship = _editor->cur_ship;
	if (m_ship == -1)
		m_ship = Objects[_editor->currentObject].instance;

	if (m_multi) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				inst = ptr->instance;
				if (!(Ship_info[Ships[inst].ship_info_index].is_big_or_huge()))
					big = 0;
				initPrimary(inst, first);
				initSecondary(inst, first);
				initTertiary(inst, first);
				first = false;
				// set up primarys;
			}
		}
	}
}

void ShipWeaponsDialogModel::initPrimary(int inst, bool first)
{
	auto pilotBank = new Banks("Pilot");
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (pilot.primary_bank_weapons[i]) {
				pilotBank->add(new Bank(pilot.primary_bank_weapons[i], i, pilot.primary_bank_ammo[i]));
			}
		}
		PrimaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss);
				for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
					if (&pss->weapons.primary_bank_weapons[i]) {
						turretBank->add(
							new Bank(pss->weapons.primary_bank_weapons[i], i, pss->weapons.primary_bank_ammo[i]));
					}
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
	auto pilotBank = new Banks("Pilot");
	if (first) {
		auto pilot = Ships[inst].weapons;
		for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (pilot.secondary_bank_weapons[i]) {
				pilotBank->add(new Bank(pilot.secondary_bank_weapons[i], i, pilot.secondary_bank_ammo[i]));
			}
		}
		SecondaryBanks.push_back(pilotBank);
		ship_subsys* ssl = &Ships[inst].subsys_list;
		ship_subsys* pss;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			model_subsystem* psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				auto turretBank = new Banks(psub->subobj_name, pss);
				for (int i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
					if (&pss->weapons.secondary_bank_weapons[i]) {
						turretBank->add(
							new Bank(pss->weapons.secondary_bank_weapons[i], i, pss->weapons.secondary_bank_ammo[i]));
					}
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
void ShipWeaponsDialogModel::initTertiary(int inst, bool first) {

}
Banks::Banks(SCP_string name, ship_subsys* subsys) : subsys(subsys) {
	this->name = std::move(name);
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
} // namespace dialogs
} // namespace fred
} // namespace fso