//
//

#include "ship_bank.h"
#include "weaponclass.h"
#include "weapon/weapon.h"


namespace scripting {
namespace api {

ship_banktype_h::ship_banktype_h() : objh(), sw(nullptr), type(SWH_NONE) {}
ship_banktype_h::ship_banktype_h(object* objp_in, ship_weapon* wpn, int in_type) : objh(objp_in), sw(wpn), type(in_type) {}

bool ship_banktype_h::isValid() const
{
	return objh.isValid() && sw != nullptr && (type == SWH_PRIMARY || type == SWH_SECONDARY || type == SWH_TERTIARY);
}

//**********HANDLE: Weaponbanktype
ADE_OBJ_NO_MULTI(l_WeaponBankType, ship_banktype_h, "weaponbanktype", "Ship/subsystem weapons bank type handle");

ADE_INDEXER(l_WeaponBankType, "number Index", "Array of weapon banks", "weaponbank", "Weapon bank, or invalid handle on failure")
{
	ship_banktype_h *sb=NULL;
	int idx = -1;
	ship_bank_h *newbank = nullptr;
	if(!ade_get_args(L, "oi|o", l_WeaponBankType.GetPtr(&sb), &idx, l_WeaponBank.GetPtr(&newbank)))
		return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

	if(!sb->isValid())
		return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

	switch(sb->type)
	{
		case SWH_PRIMARY:
			if(idx < 1 || idx > sb->sw->num_primary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->isValid()) {
				sb->sw->primary_bank_weapons[idx] = newbank->typeh.sw->primary_bank_weapons[idx];
				sb->sw->next_primary_fire_stamp[idx] = timestamp(0);
				sb->sw->primary_bank_ammo[idx] = newbank->typeh.sw->primary_bank_ammo[idx];
				sb->sw->primary_bank_start_ammo[idx] = newbank->typeh.sw->primary_bank_start_ammo[idx];
				sb->sw->primary_bank_capacity[idx] = newbank->typeh.sw->primary_bank_capacity[idx];
				sb->sw->primary_bank_rearm_time[idx] = timestamp(0);
			}
			break;
		case SWH_SECONDARY:
			if(idx < 1 || idx > sb->sw->num_secondary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->isValid()) {
				sb->sw->primary_bank_weapons[idx] = newbank->typeh.sw->primary_bank_weapons[idx];
				sb->sw->next_primary_fire_stamp[idx] = timestamp(0);
				sb->sw->primary_bank_ammo[idx] = newbank->typeh.sw->primary_bank_ammo[idx];
				sb->sw->primary_bank_start_ammo[idx] = newbank->typeh.sw->primary_bank_start_ammo[idx];
				sb->sw->primary_bank_capacity[idx] = newbank->typeh.sw->primary_bank_capacity[idx];
			}
			break;
		case SWH_TERTIARY:
			if(idx < 1 || idx > sb->sw->num_tertiary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->isValid()) {
				Error(LOCATION, "Tertiary bank support is still in progress");
				//WMC: TODO
			}
			break;
		default:
			return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));	//Invalid type
	}

	return ade_set_args(L, "o", l_WeaponBank.Set(ship_bank_h(sb->objh.objp(), sb->sw, sb->type, idx)));
}

ADE_VIRTVAR(Linked, l_WeaponBankType, "boolean", "Whether bank is in linked or unlinked fire mode (Primary-only)", "boolean", "Link status, or false if handle is invalid")
{
	ship_banktype_h *bth;
	bool newlink = false;
	int numargs = ade_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bth), &newlink);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!bth->isValid())
		return ade_set_error(L, "b", false);

	switch(bth->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && numargs > 1) {
				Ships[bth->objh.objp()->instance].flags.set(Ship::Ship_Flags::Primary_linked, newlink);
			}

			return ade_set_args(L, "b", (Ships[bth->objh.objp()->instance].flags[Ship::Ship_Flags::Primary_linked]));

		case SWH_SECONDARY:
		case SWH_TERTIARY:
			return ADE_RETURN_FALSE;
	}

	return ade_set_error(L, "b", false);
}

ADE_VIRTVAR(DualFire, l_WeaponBankType, "boolean", "Whether bank is in dual fire mode (Secondary-only)", "boolean", "Dual fire status, or false if handle is invalid")
{
	ship_banktype_h *bth;
	bool newfire = false;
	int numargs = ade_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bth), &newfire);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!bth->isValid())
		return ade_set_error(L, "b", false);

	switch(bth->type)
	{
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && numargs > 1) {
				Ships[bth->objh.objp()->instance].flags.set(Ship::Ship_Flags::Secondary_dual_fire, newfire);
			}

			return ade_set_args(L, "b", (Ships[bth->objh.objp()->instance].flags[Ship::Ship_Flags::Secondary_dual_fire]));

		case SWH_PRIMARY:
		case SWH_TERTIARY:
			return ADE_RETURN_FALSE;
	}

	return ade_set_error(L, "b", false);
}

ADE_FUNC(isValid, l_WeaponBankType, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	ship_banktype_h *sb;
	if(!ade_get_args(L, "o", l_WeaponBankType.GetPtr(&sb)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sb->isValid());
}

ADE_FUNC(__len, l_WeaponBankType, NULL, "Number of weapons in the mounted bank", "number", "Number of bank weapons, or 0 if handle is invalid")
{
	ship_banktype_h *sb=NULL;
	if(!ade_get_args(L, "o", l_WeaponBankType.GetPtr(&sb)))
		return ade_set_error(L, "i", 0);

	if(!sb->isValid())
		return ade_set_error(L, "i", 0);

	switch(sb->type)
	{
		case SWH_PRIMARY:
			return ade_set_args(L, "i", sb->sw->num_primary_banks);
		case SWH_SECONDARY:
			return ade_set_args(L, "i", sb->sw->num_secondary_banks);
		case SWH_TERTIARY:
			return ade_set_args(L, "i", sb->sw->num_tertiary_banks);
		default:
			return ade_set_error(L, "i", 0);	//Invalid type
	}
}


//**********HANDLE: Weaponbank
ADE_OBJ_NO_MULTI(l_WeaponBank, ship_bank_h, "weaponbank", "Ship/subystem weapons bank handle");

ship_bank_h::ship_bank_h() : typeh(), bank(-1) {}
ship_bank_h::ship_bank_h(object* objp_in, ship_weapon* wpn, int in_type, int in_bank) : typeh(objp_in, wpn, in_type), bank(in_bank) {}
bool ship_bank_h::isValid() const
{
	if(!typeh.isValid())
		return false;

	if(bank < 0)
		return false;

	if(typeh.type == SWH_PRIMARY && bank >= typeh.sw->num_primary_banks)
		return false;
	if(typeh.type == SWH_SECONDARY && bank >= typeh.sw->num_secondary_banks)
		return false;
	if(typeh.type == SWH_TERTIARY && bank >= typeh.sw->num_tertiary_banks)
		return false;

	return true;
}

ADE_VIRTVAR(WeaponClass, l_WeaponBank, "weaponclass", "Class of weapon mounted in the bank. As of FSO 21.0, also changes the maximum ammo to its proper value, which is what the support ship will rearm the ship to.", "weaponclass", "Weapon class, or an invalid weaponclass handle if bank handle is invalid")
{
	ship_bank_h *bh = NULL;
	int weaponclass=-1;
	if(!ade_get_args(L, "o|o", l_WeaponBank.GetPtr(&bh), l_Weaponclass.Get(&weaponclass)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!bh->isValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				bh->typeh.sw->primary_bank_weapons[bh->bank] = weaponclass;
				if (Weapon_info[weaponclass].wi_flags[Weapon::Info_Flags::Ballistic]) {
					bh->typeh.sw->primary_bank_start_ammo[bh->bank] = (int)std::lround(bh->typeh.sw->primary_bank_capacity[bh->bank] / Weapon_info[weaponclass].cargo_size);
				}
			}

			return ade_set_args(L, "o", l_Weaponclass.Set(bh->typeh.sw->primary_bank_weapons[bh->bank]));
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				bh->typeh.sw->secondary_bank_weapons[bh->bank] = weaponclass;
				bh->typeh.sw->secondary_bank_start_ammo[bh->bank] = (int)std::lround(bh->typeh.sw->secondary_bank_capacity[bh->bank] / Weapon_info[weaponclass].cargo_size);
			}

			return ade_set_args(L, "o", l_Weaponclass.Set(bh->typeh.sw->secondary_bank_weapons[bh->bank]));
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				//bh->typeh.sw->tertiary_bank_weapons[bh->bank] = weaponclass;
			}

			// return ade_set_args(L, "o", l_Weaponclass.Set(bh->typeh.sw->tertiary_bank_weapons[bh->bank]));
			// Error(LOCATION, "Tertiary bank support is still in progress");
			// WMC: TODO
			return ADE_RETURN_FALSE;
	}

	return ade_set_error(L, "o", l_Weaponclass.Set(-1));
}

ADE_VIRTVAR(AmmoLeft, l_WeaponBank, "number", "Ammo left for the current bank", "number", "Ammo left, or 0 if handle is invalid")
{
	ship_bank_h *bh = NULL;
	int ammo;
	if(!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &ammo))
		return ade_set_error(L, "i", 0);

	if(!bh->isValid())
		return ade_set_error(L, "i", 0);

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->typeh.sw->primary_bank_ammo[bh->bank] = ammo;
			}

			return ade_set_args(L, "i", bh->typeh.sw->primary_bank_ammo[bh->bank]);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->typeh.sw->secondary_bank_ammo[bh->bank] = ammo;
			}

			return ade_set_args(L, "i", bh->typeh.sw->secondary_bank_ammo[bh->bank]);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->typeh.sw->tertiary_bank_ammo = ammo;
			}
			return ade_set_args(L, "i", bh->typeh.sw->tertiary_bank_ammo);
	}

	return ade_set_error(L, "i", 0);
}

ADE_VIRTVAR(AmmoMax, l_WeaponBank, "number", "Maximum ammo for the current bank<br>"
	"<b>Note:</b> Setting this value actually sets the <i>capacity</i> of the weapon bank. To set the actual maximum ammunition use <tt>AmmoMax = <amount> * class.CargoSize</tt>", "number", "Ammo capacity, or 0 if handle is invalid")
{
	ship_bank_h *bh = NULL;
	int ammomax;
	if(!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &ammomax))
		return ade_set_error(L, "i", 0);

	if(!bh->isValid())
		return ade_set_error(L, "i", 0);

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
		{
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->typeh.sw->primary_bank_capacity[bh->bank] = ammomax;
			}

			int weapon_class = bh->typeh.sw->primary_bank_weapons[bh->bank];

			Assert(bh->typeh.objh.objp()->type == OBJ_SHIP);

			return ade_set_args(L, "i", get_max_ammo_count_for_primary_bank(Ships[bh->typeh.objh.objp()->instance].ship_info_index, bh->bank, weapon_class));
		}
		case SWH_SECONDARY:
		{
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->typeh.sw->secondary_bank_capacity[bh->bank] = ammomax;
			}

			int weapon_class = bh->typeh.sw->secondary_bank_weapons[bh->bank];

			Assert(bh->typeh.objh.objp()->type == OBJ_SHIP);

			return ade_set_args(L, "i", get_max_ammo_count_for_bank(Ships[bh->typeh.objh.objp()->instance].ship_info_index, bh->bank, weapon_class));
		}
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->typeh.sw->tertiary_bank_capacity = ammomax;
			}

			return ade_set_args(L, "i", bh->typeh.sw->tertiary_bank_capacity);
	}

	return ade_set_error(L, "i", 0);
}

ADE_VIRTVAR(Armed, l_WeaponBank, "boolean", "Weapon armed status. Does not take linking into account.", "boolean", "True if armed, false if unarmed or handle is invalid")
{
	ship_bank_h *bh = NULL;
	bool armthis=false;
	if(!ade_get_args(L, "o|b", l_WeaponBank.GetPtr(&bh), &armthis))
		return ade_set_error(L, "b", false);

	if(!bh->isValid())
		return ade_set_error(L, "b", false);

	int new_armed_bank = -1;
	if(armthis)
		new_armed_bank = bh->bank;

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR) {
				bh->typeh.sw->current_primary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->typeh.sw->current_primary_bank == bh->bank);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR) {
				bh->typeh.sw->current_secondary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->typeh.sw->current_secondary_bank == bh->bank);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR) {
				bh->typeh.sw->current_tertiary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->typeh.sw->current_tertiary_bank == bh->bank);
	}

	return ade_set_error(L, "b", false);
}

ADE_VIRTVAR(Capacity, l_WeaponBank, "number", "The actual capacity of a weapon bank as specified in the table", "number", "The capacity or -1 if handle is invalid")
{
	ship_bank_h *bh = NULL;
	int newCapacity = -1;
	if(!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &newCapacity))
		return ade_set_error(L, "i", -1);

	if(!bh->isValid())
		return ade_set_error(L, "i", -1);

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->typeh.sw->primary_bank_capacity[bh->bank] = newCapacity;
			}
			return ade_set_args(L, "i", bh->typeh.sw->primary_bank_capacity[bh->bank]);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->typeh.sw->secondary_bank_capacity[bh->bank] = newCapacity;
			}
			return ade_set_args(L, "i", bh->typeh.sw->secondary_bank_capacity[bh->bank]);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->typeh.sw->tertiary_bank_capacity = newCapacity;
			}
			return ade_set_args(L, "i", bh->typeh.sw->tertiary_bank_capacity);
	}

	return ade_set_error(L, "i", -1);
}

ADE_VIRTVAR(FOFCooldown, l_WeaponBank, "number", "The FOF cooldown value. A value of 0 means the default weapon FOF is used. A value of 1 means that the max FOF will be used", "number", "The cooldown value or -1 if invalid")
{
	ship_bank_h *bh = NULL;
	float newValue = -1.f;
	if(!ade_get_args(L, "o|f", l_WeaponBank.GetPtr(&bh), &newValue))
		return ade_set_error(L, "f", -1.f);

	if(!bh->isValid())
		return ade_set_error(L, "f", -1.f);

	switch(bh->typeh.type)
	{
		case SWH_PRIMARY:
		{
			if (ADE_SETTING_VAR) {
				CLAMP(newValue, 0.0f, 1.0f);
				bh->typeh.sw->primary_bank_fof_cooldown[bh->bank] = newValue;
			}

			return ade_set_args(L, "f", bh->typeh.sw->primary_bank_fof_cooldown[bh->bank]);
		}
		case SWH_SECONDARY:
		case SWH_TERTIARY:
			LuaError(L, "FOF cooldown is not valid for secondary or tertiary banks!");
			return ade_set_args(L, "f", -1.f);
	}

	return ade_set_error(L, "f", -1.f);
}

ADE_VIRTVAR(BurstCounter, l_WeaponBank, "number", "The burst counter for this bank. Starts at 1, counting every shot up to and including the weapon class's burst shots value before resetting to 1.", "number", "The counter or -1 if handle is invalid")
{
	ship_bank_h* bh = NULL;
	int newCounter = -1;
	if (!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &newCounter))
		return ade_set_error(L, "i", -1);

	if (!bh->isValid())
		return ade_set_error(L, "i", -1);

	switch (bh->typeh.type)
	{
	case SWH_PRIMARY:
		if (ADE_SETTING_VAR) {
			bh->typeh.sw->burst_counter[bh->bank] = newCounter - 1;
		}
		return ade_set_args(L, "i", bh->typeh.sw->burst_counter[bh->bank] + 1);
	case SWH_SECONDARY:
		if (ADE_SETTING_VAR) {
			bh->typeh.sw->burst_counter[MAX_SHIP_PRIMARY_BANKS + bh->bank] = newCounter - 1;
		}
		return ade_set_args(L, "i", bh->typeh.sw->burst_counter[MAX_SHIP_PRIMARY_BANKS + bh->bank] + 1);
	case SWH_TERTIARY:
		LuaError(L, "Burst counter is not valid for tertiary banks!");
		return ade_set_error(L, "i", -1);
	}

	return ade_set_error(L, "i", -1);
}

ADE_VIRTVAR(BurstSeed, l_WeaponBank, "number", "A random seed associated to the current burst. Changes only when a new burst starts.", "number", "The seed or -1 if handle is invalid")
{
	ship_bank_h* bh = NULL;
	int newSeed = -1;
	if (!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &newSeed))
		return ade_set_error(L, "i", -1);

	if (!bh->isValid())
		return ade_set_error(L, "i", -1);

	switch (bh->typeh.type)
	{
	case SWH_PRIMARY:
		if (ADE_SETTING_VAR) {
			bh->typeh.sw->burst_seed[bh->bank] = newSeed;
		}
		return ade_set_args(L, "i", bh->typeh.sw->burst_seed[bh->bank]);
	case SWH_SECONDARY:
		if (ADE_SETTING_VAR) {
			bh->typeh.sw->burst_seed[MAX_SHIP_PRIMARY_BANKS + bh->bank] = newSeed;
		}
		return ade_set_args(L, "i", bh->typeh.sw->burst_seed[MAX_SHIP_PRIMARY_BANKS + bh->bank]);
	case SWH_TERTIARY:
		LuaError(L, "Burst seed is not valid for tertiary banks!");
		return ade_set_error(L, "i", -1);
	}

	return ade_set_error(L, "i", -1);
}

ADE_FUNC(isValid, l_WeaponBank, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	ship_bank_h *bh;
	if(!ade_get_args(L, "o", l_WeaponBank.GetPtr(&bh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", bh->isValid());
}


}
}

