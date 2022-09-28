#include "shipwepselect.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "missionui/missionweaponchoice.h"

namespace scripting {
namespace api {

ss_wing_info_h::ss_wing_info_h() : ss_wing(-1) {}
ss_wing_info_h::ss_wing_info_h(int l_wing) : ss_wing(l_wing) {}
bool ss_wing_info_h::IsValid() const
{
	return ss_wing >= 0;
}
ss_wing_info* ss_wing_info_h::getWing() const
{
	return &Ss_wings[ss_wing];
}

ss_slot_info_h::ss_slot_info_h() {}
ss_slot_info_h::ss_slot_info_h(ss_slot_info* l_slots, int l_idx) : ss_slots(l_slots), ss_idx(l_idx) {}
bool ss_slot_info_h::IsValid() const
{
	return ss_slots != nullptr;
}
ss_slot_info* ss_slot_info_h::getSlot() const
{
	return &ss_slots[ss_idx];
}

wss_unit_wep_h::wss_unit_wep_h() : ss_unit(-1) {}
wss_unit_wep_h::wss_unit_wep_h(int l_unit) : ss_unit(l_unit) {}
bool wss_unit_wep_h::IsValid() const
{
	return ss_unit >= 0;
	// return ((ss_unit >= 0) && ((ss_bank >= 0) && (ss_bank <= 6)));
}
wss_unit* wss_unit_wep_h::getBank() const
{
	return &Wss_slots[ss_unit];
}

wss_unit_count_h::wss_unit_count_h() : ss_unit(-1) {}
wss_unit_count_h::wss_unit_count_h(int l_unit) : ss_unit(l_unit) {}
bool wss_unit_count_h::IsValid() const
{
	return ss_unit >= 0;
	// return ((ss_unit >= 0) && ((ss_bank >= 0) && (ss_bank <= 6)));
}
wss_unit* wss_unit_count_h::getBank() const
{
	return &Wss_slots[ss_unit];
}

// HERE BE MY NOTES SIR!
//  create_wings() in missionshipchoice.cpp seems to be what we want to finally use on Commit, maybe


//**********HANDLE: loadout wing
ADE_OBJ(l_Loadout_Wing_Slot, ss_slot_info_h, "loadout_wing_slot", "Loadout wing slot handle");

ADE_VIRTVAR(isShipLocked, l_Loadout_Wing_Slot, nullptr, "If the slot's ship is locked", "boolean", "The slot ship status")
{
	ss_slot_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing_Slot.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", (current.getSlot()->status & WING_SLOT_SHIPS_DISABLED) != 0);
}

ADE_VIRTVAR(isWeaponLocked,
	l_Loadout_Wing_Slot,
	nullptr,
	"If the slot's weapons are locked",
	"string",
	"The slot weapon status")
{
	ss_slot_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing_Slot.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", (current.getSlot()->status & WING_SLOT_WEAPONS_DISABLED) != 0);
}

ADE_VIRTVAR(isDisabled, l_Loadout_Wing_Slot, nullptr, "If the slot is disabled", "string", "The slot disabled status")
{
	ss_slot_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing_Slot.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	//Player ships are marked as disabled for some reason, but scpui needs to assume they aren't
	bool result = false;
	if (!((current.getSlot()->status & WING_SLOT_IS_PLAYER) != 0)) {
		result = !((current.getSlot()->status & WING_SLOT_DISABLED) != 0);
	};

	return ade_set_args(L, "b", result);
}

ADE_VIRTVAR(isPlayer, l_Loadout_Wing_Slot, nullptr, "If the slot is a player ship", "string", "The slot player status")
{
	ss_slot_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing_Slot.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", (current.getSlot()->status & WING_SLOT_IS_PLAYER) != 0);
}
// Probably don't need this one!
ADE_VIRTVAR(ShipClassIndex,
	l_Loadout_Wing_Slot,
	nullptr,
	"The index of the ship class assigned to the slot",
	"number",
	"The ship class index")
{
	ss_slot_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing_Slot.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", (current.getSlot()->original_ship_class + 1));
}

//**********HANDLE: loadout amount
ADE_OBJ(l_Loadout_Amount, wss_unit_count_h, "loadout_amount", "Loadout handle");

ADE_INDEXER(l_Loadout_Amount,
	"number bank, number amount",
	"Array of ship bank weapons. 1-3 are Primary weapons. 4-7 are Secondary weapons. Note that banks "
	"that do not exist on the ship class are still valid here as a loadout slot. Also note that "
	"primary banks will hold the value of 1 even if it is ballistic. If the amount to set is greater than "
	"the bank's capacity then it will be set to capacity. Set to -1 to empty the slot. Amounts less than -1 will be set to -1.",
	"number",
	"Amount of the currently loaded weapon, -1 if bank has no weapon, or nil if the ship or index is invalid")
{
	wss_unit_count_h current;
	int idx = -1;
	int amount;
	if (!ade_get_args(L, "oi|i", l_Loadout_Amount.Get(&current), &idx, &amount))
		return ADE_RETURN_NIL;

	if (idx < 1 || idx > MAX_SHIP_WEAPONS) {
		return ADE_RETURN_NIL;
	};

	idx--; // Convert from Lua
	if (ADE_SETTING_VAR) {
		ship_info* sip = &Ship_info[current.getBank()->ship_class];
		weapon_info* wip = &Weapon_info[current.getBank()->wep[idx]];
		int capacity = 0;
		//Calculate max capacity of the weapon in the current bank
		if (wip->subtype == WP_LASER) {
			capacity = 1;
		} else {
			capacity = wl_calc_missile_fit(current.getBank()->wep[idx],
				sip->secondary_bank_ammo_capacity[idx - MAX_SHIP_PRIMARY_BANKS]);
		}
		if (amount > capacity) {
			amount = capacity;
		} 
		if (amount < -1){
			amount = -1;
		}
		current.getBank()->wep_count[idx] = amount;
	}

	return ade_set_args(L, "i", current.getBank()->wep_count[idx]);
}

ADE_FUNC(__len, l_Loadout_Amount, nullptr, "The number of weapon banks in the slot", "number", "The number of banks.")
{
	return ade_set_args(L, "i", MAX_SHIP_WEAPONS);
}

//**********HANDLE: loadout weapon
ADE_OBJ(l_Loadout_Weapon, wss_unit_wep_h, "loadout_weapon", "Loadout handle");

ADE_INDEXER(l_Loadout_Weapon,
	"number bank, number WeaponIndex",
	"Array of ship bank weapons. 1-3 are Primary weapons. 4-7 are Secondary weapons. Note that banks "
	"that do not exist on the ship class are still valid here as a loadout slot. When setting the weapon "
	"it will be checked if it is valid for the ship and bank. If it is not then it will be set to -1 and the "
	"amount will be set to -1. If it is valid for the ship then the amount is set to 0. Use .Amounts to set "
	"the amount afterwards. Set to -1 to empty the slot.",
	"number",
	"index into Weapon Classes, 0 if bank is empty, -1 if the ship cannot carry the weapon, or nil if the ship or index is invalid")
{
	wss_unit_wep_h current;
	int bank = -1;
	int wepIndex;
	if (!ade_get_args(L, "oi|i", l_Loadout_Weapon.Get(&current), &bank, &wepIndex))
		return ADE_RETURN_NIL;

	if (bank < 1 || bank > MAX_SHIP_WEAPONS) {
		return ADE_RETURN_NIL;
	};

	ship_info* sip = &Ship_info[current.getBank()->ship_class];
	int bankType = -1;
	// Decide if we're setting a valid primary or secondary bank on the current ship class
	if (bank <= sip->num_primary_banks) {
		bankType = WP_LASER;
	} else if (bank <= (sip->num_secondary_banks + MAX_SHIP_PRIMARY_BANKS)) {
		bankType = WP_MISSILE;
	}

	wepIndex--; // Convert from Lua
	bank--; //Convert from Lua
	if (ADE_SETTING_VAR) {
		if (bankType == -1 || wepIndex > weapon_info_size()) {
			LuaError(L, "The provided bank or weapon index is invalid!");
		} else {
			bool valid = false;
			//First we check that this is a valid bank for the ship and that the script is not trying to empty the bank
			if (wepIndex > -1) {
				// Next we check that this is a weapon the ship can carry
				if (eval_weapon_flag_for_game_type(sip->restricted_loadout_flag[bank])) {
					if (eval_weapon_flag_for_game_type(sip->allowed_bank_restricted_weapons[bank][wepIndex])) {
						valid = true;
					}
				} else {
					if (eval_weapon_flag_for_game_type(sip->allowed_weapons[wepIndex])) {
						valid = true;
					}
				}
			}
			if (valid) {
				//Now we double check that we're putting a primary in a primary slot or secondary in a secondary slot
				weapon_info* wip = &Weapon_info[wepIndex];
				if (wip->subtype == bankType) {
					current.getBank()->wep[bank] = wepIndex;
				} else {
					LuaError(L, "Cannot put a primary in a secondary bank or a secondary in a primary bank!");
				}
				current.getBank()->wep_count[bank] = 0;
			}
			else {
				//If the ship cannot carry this weapon in this bank then we set it to -1 and the amount to -1
				current.getBank()->wep[bank] = -1;
				current.getBank()->wep_count[bank] = -1;
			}
		}
	}

	return ade_set_args(L, "i", current.getBank()->wep[bank] + 1);
}

ADE_FUNC(__len, l_Loadout_Weapon, nullptr, "The number of weapon banks in the slot", "number", "The number of banks.")
{
	return ade_set_args(L, "i", MAX_SHIP_WEAPONS);
}

//**********HANDLE: loadout ship
ADE_OBJ(l_Loadout_Ship, int, "loadout_ship", "Loadout handle");

ADE_VIRTVAR(ShipClassIndex,
	l_Loadout_Ship,
	"number",
	"The index of the Ship Class. When setting the ship class this will also set the weapons to empty slots. Use "
	".Weapons and .Amounts to set those afterwards. Set to -1 to empty the slot.",
	"number",
	"The index or nil if handle is invalid")
{
	int current;
	int shipIndex;
	if (!ade_get_args(L, "o|i", l_Loadout_Ship.Get(&current), &shipIndex)) {
		return ADE_RETURN_NIL;
	}

	shipIndex--; //Convert from Lua

	if (ADE_SETTING_VAR) {
		if (shipIndex > -1) {
			// If we're not trying to empty the slot then make sure it's a valid ship index
			if (shipIndex > ship_info_size()) {
				LuaError(L, "The provided ship index is invalid!");
			} else {
				Wss_slots[current].ship_class = shipIndex;
				//Reset all currently loaded weapons here
				for (int i = 0; i < MAX_SHIP_WEAPONS; i++) {
					Wss_slots[current].wep[i] = -1;
					Wss_slots[current].wep_count[i] = -1;
				}
			}
		} else {
			Wss_slots[current].ship_class = -1;
		}
	}

	return ade_set_args(L, "i", (Wss_slots[current].ship_class + 1));
}

ADE_VIRTVAR(Weapons,
	l_Loadout_Ship,
	nullptr,
	"Array of weapons in the loadout slot",
	"loadout_weapon",
	"The weapons array or nil if handle is invalid")
{
	int current;
	if (!ade_get_args(L, "o", l_Loadout_Ship.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Loadout_Weapon.Set(wss_unit_wep_h(current)));
}

ADE_VIRTVAR(Amounts,
	l_Loadout_Ship,
	nullptr,
	"Array of weapon amounts in the loadout slot",
	"loadout_amount",
	"The weapon amounts array or nil if handle is invalid")
{
	int current;
	if (!ade_get_args(L, "o", l_Loadout_Ship.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Loadout_Amount.Set(wss_unit_count_h(current)));
}

//**********HANDLE: loadout wing
ADE_OBJ(l_Loadout_Wing, ss_wing_info_h, "loadout_wing", "Loadout handle");

ADE_INDEXER(l_Loadout_Wing,
	"number idx",
	"Array of loadout wing slot data",
	"loadout_wing_slot",
	"loadout slot handle, or invalid handle if index is invalid")
{
	ss_wing_info_h current;
	int idx;
	if (!ade_get_args(L, "oi", l_Loadout_Wing.Get(&current), &idx))
		return ade_set_error(L, "s", "");
	idx--; // Convert to Lua's 1 based index system
	return ade_set_args(L, "o", l_Loadout_Wing_Slot.Set(ss_slot_info_h(current.getWing()->ss_slots, idx)));
}

//This seems superfluous because the loadout wing slots is always max but not only is it something scripters may expect,
//but also if max wings is ever expanded this will automatically reflect that
ADE_FUNC(__len, l_Loadout_Wing, nullptr, "The number of slots in the wing", "number", "The number of slots.")
{
	return ade_set_args(L, "i", MAX_WING_SLOTS);
}

ADE_VIRTVAR(Name, l_Loadout_Wing, nullptr, "The name of the wing", "string", "The wing")
{
	ss_wing_info_h current;
	if (!ade_get_args(L, "o", l_Loadout_Wing.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	wing* wp;
	wp = &Wings[current.getWing()->wingnum];

	return ade_set_args(L, "s", wp->name);
}

} // namespace api
} // namespace scripting