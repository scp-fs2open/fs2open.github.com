#pragma once

#include "mission/missionbriefcommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct ss_wing_info_h {
	int ss_wing;
	ss_wing_info_h();
	explicit ss_wing_info_h(int l_wing);
	bool isValid() const;
	ss_wing_info* getWing() const;
	int getWingIndex() const;
};

struct ss_slot_info_h {
	ss_slot_info* ss_slots;
	int ss_idx;
	int ss_wing; //pass in the wing index so we can get the callsign later. I hate you Volition.
	ss_slot_info_h();
	explicit ss_slot_info_h(ss_slot_info* l_slots, int l_idx, int l_wing);
	bool isValid() const;
	ss_slot_info* getSlot() const;
	int getSlotIndex() const;
	int getWingIndex() const;
};

struct wss_unit_wep_h {
	int ss_unit;
	wss_unit_wep_h();
	explicit wss_unit_wep_h(int l_unit);
	bool isValid() const;
	wss_unit* getBank() const;
};

struct wss_unit_count_h {
	int ss_unit;
	wss_unit_count_h();
	explicit wss_unit_count_h(int l_unit);
	bool isValid() const;
	wss_unit* getBank() const;
};

DECLARE_ADE_OBJ(l_Loadout_Wing, ss_wing_info_h);
DECLARE_ADE_OBJ(l_Loadout_Wing_Slot, ss_slot_info_h);

DECLARE_ADE_OBJ(l_Loadout_Ship, int);
DECLARE_ADE_OBJ(l_Loadout_Weapon, wss_unit_wep_h);
DECLARE_ADE_OBJ(l_Loadout_Amount, wss_unit_count_h);

} // namespace api
} // namespace scripting