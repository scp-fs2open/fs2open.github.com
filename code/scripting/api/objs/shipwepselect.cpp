#include "shipwepselect.h"
#include "ship/ship.h"

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
	"number idx",
	"Array of ship bank weapons. 1-3 are Primary weapons. 4-7 are Secondary weapons. Note that banks "
	"that do not exist on the ship class are still valid here but should hold nil data. Also note that "
	"primary banks will also hold the value of 1 even if it is ballistic.",
	"number",
	"Amount of the currently loaded weapon, -1 if bank has no weapon, or nil if the ship or index is invalid")
{
	wss_unit_count_h current;
	int idx = -1;
	if (!ade_get_args(L, "oi", l_Loadout_Amount.Get(&current), &idx))
		return ADE_RETURN_NIL;

	// Will eventually be writable so that Lua can manipulate the player loadout!
	// Note may need to do some checking that the bank is valid for the current ship
	// and that the amount-to-write is the correct size for the weapon/ship cargo size
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (idx < 1 || idx > 7) {
		return ADE_RETURN_NIL;
	};
	idx--; // Convert to Lua's 1 based index system

	return ade_set_args(L, "i", current.getBank()->wep_count[idx]);
}

ADE_FUNC(__len, l_Loadout_Amount, nullptr, "The number of weapon banks in the slot", "number", "The number of banks.")
{
	return ade_set_args(L, "i", MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS);
}

//**********HANDLE: loadout weapon
ADE_OBJ(l_Loadout_Weapon, wss_unit_wep_h, "loadout_weapon", "Loadout handle");

ADE_INDEXER(l_Loadout_Weapon,
	"number idx",
	"Array of ship bank weapons. 1-3 are Primary weapons. 4-7 are Secondary weapons. Note that banks "
	"that do not exist on the ship class are still valid here as a loadout slot.",
	"number",
	"index into Weapon Classes, 0 if bank is empty, or nil if the ship or index is invalid")
{
	wss_unit_wep_h current;
	int idx = -1;
	if (!ade_get_args(L, "oi", l_Loadout_Weapon.Get(&current), &idx))
		return ADE_RETURN_NIL;

	// Will eventually be writable so that Lua can manipulate the player loadout!
	// Note may need to do some checking that the bank is valid for the current ship
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (idx < 1 || idx > 7) {
		return ADE_RETURN_NIL;
	};
	idx--; // Convert to Lua's 1 based index system

	return ade_set_args(L, "i", current.getBank()->wep[idx] + 1);
}

ADE_FUNC(__len, l_Loadout_Weapon, nullptr, "The number of weapon banks in the slot", "number", "The number of banks.")
{
	return ade_set_args(L, "i", MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS);
}

//**********HANDLE: loadout ship
ADE_OBJ(l_Loadout_Ship, int, "loadout_ship", "Loadout handle");

ADE_VIRTVAR(ShipClassIndex,
	l_Loadout_Ship,
	nullptr,
	"The index of the Ship Class",
	"number",
	"The index or nil if handle is invalid")
{
	int current;
	if (!ade_get_args(L, "o", l_Loadout_Ship.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	// Will eventually be writable so that Lua can manipulate the player loadout!
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
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