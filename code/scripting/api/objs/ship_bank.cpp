//
//

#include "ship_bank.h"
#include "weaponclass.h"
#include "weapon/weapon.h"


namespace scripting {
namespace api {

ship_banktype_h::ship_banktype_h() : object_h () {
	sw = NULL;
	type = SWH_NONE;
}
ship_banktype_h::ship_banktype_h(object* objp_in, ship_weapon* wpn, int in_type) : object_h(objp_in) {
	sw = wpn;
	type = in_type;
}
bool ship_banktype_h::IsValid() {
	return object_h::IsValid() && sw != NULL && (type == SWH_PRIMARY || type == SWH_SECONDARY || type == SWH_TERTIARY);
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

	if(!sb->IsValid())
		return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

	switch(sb->type)
	{
		case SWH_PRIMARY:
			if(idx < 1 || idx > sb->sw->num_primary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->IsValid()) {
				sb->sw->primary_bank_weapons[idx] = newbank->sw->primary_bank_weapons[idx];
				sb->sw->next_primary_fire_stamp[idx] = timestamp(0);
				sb->sw->primary_bank_ammo[idx] = newbank->sw->primary_bank_ammo[idx];
				sb->sw->primary_bank_start_ammo[idx] = newbank->sw->primary_bank_start_ammo[idx];
				sb->sw->primary_bank_capacity[idx] = newbank->sw->primary_bank_capacity[idx];
				sb->sw->primary_bank_rearm_time[idx] = timestamp(0);
			}
			break;
		case SWH_SECONDARY:
			if(idx < 1 || idx > sb->sw->num_secondary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->IsValid()) {
				sb->sw->primary_bank_weapons[idx] = newbank->sw->primary_bank_weapons[idx];
				sb->sw->next_primary_fire_stamp[idx] = timestamp(0);
				sb->sw->primary_bank_ammo[idx] = newbank->sw->primary_bank_ammo[idx];
				sb->sw->primary_bank_start_ammo[idx] = newbank->sw->primary_bank_start_ammo[idx];
				sb->sw->primary_bank_capacity[idx] = newbank->sw->primary_bank_capacity[idx];
			}
			break;
		case SWH_TERTIARY:
			if(idx < 1 || idx > sb->sw->num_tertiary_banks)
				return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));

			idx--; //Lua->FS2

			if(ADE_SETTING_VAR && newbank && newbank->IsValid()) {
				Error(LOCATION, "Tertiary bank support is still in progress");
				//WMC: TODO
			}
			break;
		default:
			return ade_set_error(L, "o", l_WeaponBank.Set(ship_bank_h()));	//Invalid type
	}

	return ade_set_args(L, "o", l_WeaponBank.Set(ship_bank_h(sb->objp, sb->sw, sb->type, idx)));
}

ADE_VIRTVAR(Linked, l_WeaponBankType, "boolean", "Whether bank is in linked or unlinked fire mode (Primary-only)", "boolean", "Link status, or false if handle is invalid")
{
	ship_banktype_h *bh;
	bool newlink = false;
	int numargs = ade_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bh), &newlink);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!bh->IsValid())
		return ade_set_error(L, "b", false);

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && numargs > 1) {
				Ships[bh->objp->instance].flags.set(Ship::Ship_Flags::Primary_linked, newlink);
			}

			return ade_set_args(L, "b", (Ships[bh->objp->instance].flags[Ship::Ship_Flags::Primary_linked]));

		case SWH_SECONDARY:
		case SWH_TERTIARY:
			return ADE_RETURN_FALSE;
	}

	return ade_set_error(L, "b", false);
}

ADE_VIRTVAR(DualFire, l_WeaponBankType, "boolean", "Whether bank is in dual fire mode (Secondary-only)", "boolean", "Dual fire status, or false if handle is invalid")
{
	ship_banktype_h *bh;
	bool newfire = false;
	int numargs = ade_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bh), &newfire);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!bh->IsValid())
		return ade_set_error(L, "b", false);

	switch(bh->type)
	{
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && numargs > 1) {
				Ships[bh->objp->instance].flags.set(Ship::Ship_Flags::Secondary_dual_fire, newfire);
			}

			return ade_set_args(L, "b", (Ships[bh->objp->instance].flags[Ship::Ship_Flags::Secondary_dual_fire]));

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

	return ade_set_args(L, "b", sb->IsValid());
}

ADE_FUNC(__len, l_WeaponBankType, NULL, "Number of weapons in the mounted bank", "number", "Number of bank weapons, or 0 if handle is invalid")
{
	ship_banktype_h *sb=NULL;
	if(!ade_get_args(L, "o", l_WeaponBankType.GetPtr(&sb)))
		return ade_set_error(L, "i", 0);

	if(!sb->IsValid())
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

ship_bank_h::ship_bank_h() : ship_banktype_h() {
	bank = -1;
}
ship_bank_h::ship_bank_h(object* objp_in, ship_weapon* wpn, int in_type, int in_bank) : ship_banktype_h(objp_in, wpn, in_type) {
	bank = in_bank;
}
bool ship_bank_h::IsValid() {
	if(!ship_banktype_h::IsValid())
		return false;

	if(bank < 0)
		return false;

	if(type == SWH_PRIMARY && bank >= sw->num_primary_banks)
		return false;
	if(type == SWH_SECONDARY && bank >= sw->num_secondary_banks)
		return false;
	if(type == SWH_TERTIARY && bank >= sw->num_tertiary_banks)
		return false;

	return true;
}

ADE_VIRTVAR(WeaponClass, l_WeaponBank, "weaponclass", "Class of weapon mounted in the bank. As of FSO 21.0, also changes the maximum ammo to its proper value, which is what the support ship will rearm the ship to.", "weaponclass", "Weapon class, or an invalid weaponclass handle if bank handle is invalid")
{
	ship_bank_h *bh = NULL;
	int weaponclass=-1;
	if(!ade_get_args(L, "o|o", l_WeaponBank.GetPtr(&bh), l_Weaponclass.Get(&weaponclass)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!bh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				bh->sw->primary_bank_weapons[bh->bank] = weaponclass;
				if (Weapon_info[weaponclass].wi_flags[Weapon::Info_Flags::Ballistic]) {
					bh->sw->primary_bank_start_ammo[bh->bank] = (int)std::lround(bh->sw->primary_bank_capacity[bh->bank] / Weapon_info[weaponclass].cargo_size);
				}
			}

			return ade_set_args(L, "o", l_Weaponclass.Set(bh->sw->primary_bank_weapons[bh->bank]));
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				bh->sw->secondary_bank_weapons[bh->bank] = weaponclass;
				bh->sw->secondary_bank_start_ammo[bh->bank] = (int)std::lround(bh->sw->secondary_bank_capacity[bh->bank] / Weapon_info[weaponclass].cargo_size);
			}

			return ade_set_args(L, "o", l_Weaponclass.Set(bh->sw->secondary_bank_weapons[bh->bank]));
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && weaponclass > -1) {
				//bh->sw->tertiary_bank_weapons[bh->bank] = weaponclass;
			}

			// return ade_set_args(L, "o", l_Weaponclass.Set(bh->sw->tertiary_bank_weapons[bh->bank]));
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

	if(!bh->IsValid())
		return ade_set_error(L, "i", 0);

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->sw->primary_bank_ammo[bh->bank] = ammo;
			}

			return ade_set_args(L, "i", bh->sw->primary_bank_ammo[bh->bank]);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->sw->secondary_bank_ammo[bh->bank] = ammo;
			}

			return ade_set_args(L, "i", bh->sw->secondary_bank_ammo[bh->bank]);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && ammo > -1) {
				bh->sw->tertiary_bank_ammo = ammo;
			}
			return ade_set_args(L, "i", bh->sw->tertiary_bank_ammo);
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

	if(!bh->IsValid())
		return ade_set_error(L, "i", 0);

	switch(bh->type)
	{
		case SWH_PRIMARY:
		{
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->sw->primary_bank_capacity[bh->bank] = ammomax;
			}

			int weapon_class = bh->sw->primary_bank_weapons[bh->bank];

			Assert(bh->objp->type == OBJ_SHIP);

			return ade_set_args(L, "i", get_max_ammo_count_for_primary_bank(Ships[bh->objp->instance].ship_info_index, bh->bank, weapon_class));
		}
		case SWH_SECONDARY:
		{
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->sw->secondary_bank_capacity[bh->bank] = ammomax;
			}

			int weapon_class = bh->sw->secondary_bank_weapons[bh->bank];

			Assert(bh->objp->type == OBJ_SHIP);

			return ade_set_args(L, "i", get_max_ammo_count_for_bank(Ships[bh->objp->instance].ship_info_index, bh->bank, weapon_class));
		}
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && ammomax > -1) {
				bh->sw->tertiary_bank_capacity = ammomax;
			}

			return ade_set_args(L, "i", bh->sw->tertiary_bank_capacity);
	}

	return ade_set_error(L, "i", 0);
}

ADE_VIRTVAR(Armed, l_WeaponBank, "boolean", "Weapon armed status. Does not take linking into account.", "boolean", "True if armed, false if unarmed or handle is invalid")
{
	ship_bank_h *bh = NULL;
	bool armthis=false;
	if(!ade_get_args(L, "o|b", l_WeaponBank.GetPtr(&bh), &armthis))
		return ade_set_error(L, "b", false);

	if(!bh->IsValid())
		return ade_set_error(L, "b", false);

	int new_armed_bank = -1;
	if(armthis)
		new_armed_bank = bh->bank;

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR) {
				bh->sw->current_primary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->sw->current_primary_bank == bh->bank);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR) {
				bh->sw->current_secondary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->sw->current_secondary_bank == bh->bank);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR) {
				bh->sw->current_tertiary_bank = new_armed_bank;
			}
			return ade_set_args(L, "b", bh->sw->current_tertiary_bank == bh->bank);
	}

	return ade_set_error(L, "b", false);
}

ADE_VIRTVAR(Capacity, l_WeaponBank, "number", "The actual capacity of a weapon bank as specified in the table", "number", "The capacity or -1 if handle is invalid")
{
	ship_bank_h *bh = NULL;
	int newCapacity = -1;
	if(!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &newCapacity))
		return ade_set_error(L, "i", -1);

	if(!bh->IsValid())
		return ade_set_error(L, "i", -1);

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->sw->primary_bank_capacity[bh->bank] = newCapacity;
			}
			return ade_set_args(L, "i", bh->sw->primary_bank_capacity[bh->bank]);
		case SWH_SECONDARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->sw->secondary_bank_capacity[bh->bank] = newCapacity;
			}
			return ade_set_args(L, "i", bh->sw->secondary_bank_capacity[bh->bank]);
		case SWH_TERTIARY:
			if(ADE_SETTING_VAR && newCapacity > 0) {
				bh->sw->tertiary_bank_capacity = newCapacity;
			}
			return ade_set_args(L, "i", bh->sw->tertiary_bank_capacity);
	}

	return ade_set_error(L, "i", -1);
}

ADE_VIRTVAR(FOFCooldown, l_WeaponBank, "number", "The FOF cooldown value. A value of 0 means the default weapon FOF is used. A value of 1 means that the max FOF will be used", "number", "The cooldown value or -1 if invalid")
{
	ship_bank_h *bh = NULL;
	float newValue = -1.f;
	if(!ade_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &newValue))
		return ade_set_error(L, "f", -1.f);

	if(!bh->IsValid())
		return ade_set_error(L, "f", -1.f);

	if (ADE_SETTING_VAR) {
		LuaError(L, "This function does not support setting values yet!");
		return ade_set_error(L, "f", -1.f);
	}

	switch(bh->type)
	{
		case SWH_PRIMARY:
		{
			auto wif = &Weapon_info[bh->sw->primary_bank_weapons[bh->bank]];
			float reset_amount = (timestamp_until(bh->sw->last_primary_fire_stamp[bh->bank]) / 1000.0f) * wif->fof_reset_rate;
			auto val = bh->sw->primary_bank_fof_cooldown[bh->bank] + reset_amount;
			CLAMP(val, 0.0f, 1.0f);
			return ade_set_args(L, "f", val);
		}
		case SWH_SECONDARY:
		case SWH_TERTIARY:
			LuaError(L, "FOF cooldown is not valid for secondary or tertiary banks!");
			return ade_set_args(L, "f", -1.f);
	}

	return ade_set_error(L, "f", -1.f);
}

ADE_FUNC(isValid, l_WeaponBank, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	ship_bank_h *bh;
	if(!ade_get_args(L, "o", l_WeaponBank.GetPtr(&bh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", bh->IsValid());
}


}
}

