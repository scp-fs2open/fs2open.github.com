//
//

#ifndef FS2_OPEN_ENUMS_H
#define FS2_OPEN_ENUMS_H

#include "globalincs/pstypes.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

const int32_t LE_ALPHABLEND_FILTER				= 14;
const int32_t LE_ALPHABLEND_NONE				= 27;
const int32_t LE_CFILE_TYPE_NORMAL				= 20;
const int32_t LE_CFILE_TYPE_MEMORY_MAPPED		= 21;
const int32_t LE_MOUSE_LEFT_BUTTON				= 1;
const int32_t LE_MOUSE_RIGHT_BUTTON				= 2;
const int32_t LE_MOUSE_MIDDLE_BUTTON			= 3;
const int32_t LE_ORDER_ATTACK					= 28;
const int32_t LE_ORDER_ATTACK_ANY				= 29;
const int32_t LE_ORDER_DEPART					= 30;
const int32_t LE_ORDER_DISABLE					= 31;
const int32_t LE_ORDER_DISARM					= 32;
const int32_t LE_ORDER_DOCK						= 33;
const int32_t LE_ORDER_EVADE					= 34;
const int32_t LE_ORDER_FLY_TO					= 35;
const int32_t LE_ORDER_FORM_ON_WING				= 36;
const int32_t LE_ORDER_GUARD					= 37;
const int32_t LE_ORDER_IGNORE					= 38;
const int32_t LE_ORDER_KEEP_SAFE_DISTANCE		= 39;
const int32_t LE_ORDER_PLAY_DEAD				= 40;
const int32_t LE_ORDER_PLAY_DEAD_PERSISTENT		= 77;
const int32_t LE_ORDER_REARM					= 41;
const int32_t LE_ORDER_STAY_NEAR				= 42;
const int32_t LE_ORDER_STAY_STILL				= 43;
const int32_t LE_ORDER_UNDOCK					= 44;
const int32_t LE_ORDER_WAYPOINTS				= 45;
const int32_t LE_ORDER_WAYPOINTS_ONCE			= 46;
const int32_t LE_ORDER_ATTACK_WING				= 69;
const int32_t LE_ORDER_GUARD_WING				= 70;
const int32_t LE_ORDER_ATTACK_SHIP_CLASS		= 74;
const int32_t LE_PARTICLE_DEBUG					= 4;
const int32_t LE_PARTICLE_BITMAP				= 5;
const int32_t LE_PARTICLE_FIRE					= 6;
const int32_t LE_PARTICLE_SMOKE					= 7;
const int32_t LE_PARTICLE_SMOKE2				= 8;
const int32_t LE_PARTICLE_PERSISTENT_BITMAP		= 9;
const int32_t LE_SEXPVAR_CAMPAIGN_PERSISTENT	= 22;
const int32_t LE_SEXPVAR_NOT_PERSISTENT			= 23;
const int32_t LE_SEXPVAR_PLAYER_PERSISTENT		= 24;
const int32_t LE_SEXPVAR_TYPE_NUMBER			= 25;
const int32_t LE_SEXPVAR_TYPE_STRING			= 26;
const int32_t LE_TEXTURE_STATIC					= 10;
const int32_t LE_TEXTURE_DYNAMIC				= 11;
const int32_t LE_LOCK							= 12;
const int32_t LE_UNLOCK							= 13;
const int32_t LE_NONE							= 15;
const int32_t LE_SHIELD_FRONT					= 16;
const int32_t LE_SHIELD_LEFT					= 17;
const int32_t LE_SHIELD_RIGHT					= 18;
const int32_t LE_SHIELD_BACK					= 19;
const int32_t LE_MISSION_REPEAT					= 47;
const int32_t LE_NORMAL_CONTROLS				= 48;
const int32_t LE_LUA_STEERING_CONTROLS			= 49;
const int32_t LE_LUA_FULL_CONTROLS				= 50;
const int32_t LE_NORMAL_BUTTON_CONTROLS			= 51;
const int32_t LE_LUA_ADDITIVE_BUTTON_CONTROL	= 52;
const int32_t LE_LUA_OVERRIDE_BUTTON_CONTROL	= 53;
const int32_t LE_VM_INTERNAL					= 54;
const int32_t LE_VM_EXTERNAL					= 55;
const int32_t LE_VM_TRACK						= 56;
const int32_t LE_VM_DEAD_VIEW					= 57;
const int32_t LE_VM_CHASE						= 58;
const int32_t LE_VM_OTHER_SHIP					= 59;
const int32_t LE_VM_EXTERNAL_CAMERA_LOCKED		= 60;
const int32_t LE_VM_CAMERA_LOCKED				= 75;
const int32_t LE_VM_WARP_CHASE					= 61;
const int32_t LE_VM_PADLOCK_UP					= 62;
const int32_t LE_VM_PADLOCK_REAR				= 63;
const int32_t LE_VM_PADLOCK_LEFT				= 64;
const int32_t LE_VM_PADLOCK_RIGHT				= 65;
const int32_t LE_VM_WARPIN_ANCHOR				= 66;
const int32_t LE_VM_TOPDOWN						= 67;
const int32_t LE_VM_FREECAMERA					= 68;
const int32_t LE_VM_CENTERING					= 76;
const int32_t LE_MESSAGE_PRIORITY_LOW			= 71;
const int32_t LE_MESSAGE_PRIORITY_NORMAL		= 72;
const int32_t LE_MESSAGE_PRIORITY_HIGH			= 73;

const int ENUM_NEXT_INDEX = 78; // <<<<<<<<<<<<<<<<<<<<<<
extern flag_def_list Enumerations[];
extern size_t Num_enumerations;

struct enum_h {
	int index;
	bool is_constant;

	enum_h();

	explicit enum_h(int n_index);

	bool IsValid() const;
};

DECLARE_ADE_OBJ(l_Enum, enum_h);

}
}

#endif //FS2_OPEN_ENUMS_H
