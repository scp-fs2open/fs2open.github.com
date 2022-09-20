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
	int ss_idx;
	ss_slot_info* ss_slots;
	ss_slot_info_h();
	explicit ss_slot_info_h(ss_slot_info* l_slots, int l_idx);
	bool IsValid() const;
	ss_slot_info* getSlot() const;
};

//DECLARE_ADE_OBJ(l_LoadoutWingSlot, ss_slot_info_h);

//DECLARE_ADE_OBJ(l_LoadoutWing, ss_wing_info_h);

DECLARE_ADE_OBJ(l_Loadout_Wing, ss_wing_info_h);
DECLARE_ADE_OBJ(l_Loadout_Wing_Slot, ss_slot_info_h);

//DECLARE_ADE_OBJ(l_Goals, int);

} // namespace api
} // namespace scripting