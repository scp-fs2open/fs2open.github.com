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
	bool IsValid() const;
	ss_wing_info* getWing() const;
};

struct ss_slot_info_h {
	ss_slot_info* ss_slots;
	int ss_idx;
	ss_slot_info_h();
	explicit ss_slot_info_h(ss_slot_info* l_slots, int l_idx);
	bool IsValid() const;
	ss_slot_info* getSlot() const;
};

struct wss_unit_wep_h {
	int ss_unit;
	wss_unit_wep_h();
	explicit wss_unit_wep_h(int l_unit);
	bool IsValid() const;
	wss_unit* getBank() const;
};

struct wss_unit_count_h {
	int ss_unit;
	wss_unit_count_h();
	explicit wss_unit_count_h(int l_unit);
	bool IsValid() const;
	wss_unit* getBank() const;
};

DECLARE_ADE_OBJ(l_Loadout_Wing, ss_wing_info_h);
DECLARE_ADE_OBJ(l_Loadout_Wing_Slot, ss_slot_info_h);

DECLARE_ADE_OBJ(l_Loadout_Ship, int);
DECLARE_ADE_OBJ(l_Loadout_Weapon, wss_unit_wep_h);
DECLARE_ADE_OBJ(l_Loadout_Amount, wss_unit_count_h);

} // namespace api
} // namespace scripting