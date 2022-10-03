//
//

#ifndef FS2_OPEN_ENUMS_H
#define FS2_OPEN_ENUMS_H

#include "globalincs/pstypes.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

const int32_t LE_ALPHABLEND_FILTER           = 14;
const int32_t LE_ALPHABLEND_NONE             = 27;
const int32_t LE_CFILE_TYPE_NORMAL           = 20;
const int32_t LE_CFILE_TYPE_MEMORY_MAPPED    = 21;
const int32_t LE_MOUSE_LEFT_BUTTON           = 1;
const int32_t LE_MOUSE_RIGHT_BUTTON          = 2;
const int32_t LE_MOUSE_MIDDLE_BUTTON         = 3;
const int32_t LE_ORDER_ATTACK                = 28;
const int32_t LE_ORDER_ATTACK_ANY            = 29;
const int32_t LE_ORDER_DEPART                = 30;
const int32_t LE_ORDER_DISABLE               = 31;
const int32_t LE_ORDER_DISARM                = 32;
const int32_t LE_ORDER_DOCK                  = 33;
const int32_t LE_ORDER_EVADE                 = 34;
const int32_t LE_ORDER_FLY_TO                = 35;
const int32_t LE_ORDER_FORM_ON_WING          = 36;
const int32_t LE_ORDER_GUARD                 = 37;
const int32_t LE_ORDER_IGNORE                = 38;
const int32_t LE_ORDER_KEEP_SAFE_DISTANCE    = 39;
const int32_t LE_ORDER_PLAY_DEAD             = 40;
const int32_t LE_ORDER_PLAY_DEAD_PERSISTENT  = 77;
const int32_t LE_ORDER_REARM                 = 41;
const int32_t LE_ORDER_STAY_NEAR             = 42;
const int32_t LE_ORDER_STAY_STILL            = 43;
const int32_t LE_ORDER_UNDOCK                = 44;
const int32_t LE_ORDER_WAYPOINTS             = 45;
const int32_t LE_ORDER_WAYPOINTS_ONCE        = 46;
const int32_t LE_ORDER_ATTACK_WING           = 69;
const int32_t LE_ORDER_GUARD_WING            = 70;
const int32_t LE_ORDER_ATTACK_SHIP_CLASS     = 74;
const int32_t LE_PARTICLE_DEBUG              = 4;
const int32_t LE_PARTICLE_BITMAP             = 5;
const int32_t LE_PARTICLE_FIRE               = 6;
const int32_t LE_PARTICLE_SMOKE              = 7;
const int32_t LE_PARTICLE_SMOKE2             = 8;
const int32_t LE_PARTICLE_PERSISTENT_BITMAP  = 9;
const int32_t LE_SEXPVAR_CAMPAIGN_PERSISTENT = 22;
const int32_t LE_SEXPVAR_NOT_PERSISTENT      = 23;
const int32_t LE_SEXPVAR_PLAYER_PERSISTENT   = 24;
const int32_t LE_SEXPVAR_TYPE_NUMBER         = 25;
const int32_t LE_SEXPVAR_TYPE_STRING         = 26;
const int32_t LE_TEXTURE_STATIC              = 10;
const int32_t LE_TEXTURE_DYNAMIC             = 11;
const int32_t LE_LOCK                        = 12;
const int32_t LE_UNLOCK                      = 13;
const int32_t LE_NONE                        = 15;
const int32_t LE_SHIELD_FRONT                = 16;
const int32_t LE_SHIELD_LEFT                 = 17;
const int32_t LE_SHIELD_RIGHT                = 18;
const int32_t LE_SHIELD_BACK                 = 19;
const int32_t LE_MISSION_REPEAT              = 47;
const int32_t LE_NORMAL_CONTROLS             = 48;
const int32_t LE_LUA_STEERING_CONTROLS       = 49;
const int32_t LE_LUA_FULL_CONTROLS           = 50;
const int32_t LE_NORMAL_BUTTON_CONTROLS      = 51;
const int32_t LE_LUA_ADDITIVE_BUTTON_CONTROL = 52;
const int32_t LE_LUA_OVERRIDE_BUTTON_CONTROL = 53;
const int32_t LE_VM_INTERNAL                 = 54;
const int32_t LE_VM_EXTERNAL                 = 55;
const int32_t LE_VM_TRACK                    = 56;
const int32_t LE_VM_DEAD_VIEW                = 57;
const int32_t LE_VM_CHASE                    = 58;
const int32_t LE_VM_OTHER_SHIP               = 59;
const int32_t LE_VM_EXTERNAL_CAMERA_LOCKED   = 60;
const int32_t LE_VM_CAMERA_LOCKED            = 75;
const int32_t LE_VM_WARP_CHASE               = 61;
const int32_t LE_VM_PADLOCK_UP               = 62;
const int32_t LE_VM_PADLOCK_REAR             = 63;
const int32_t LE_VM_PADLOCK_LEFT             = 64;
const int32_t LE_VM_PADLOCK_RIGHT            = 65;
const int32_t LE_VM_WARPIN_ANCHOR            = 66;
const int32_t LE_VM_TOPDOWN                  = 67;
const int32_t LE_VM_FREECAMERA               = 68;
const int32_t LE_VM_CENTERING                = 76;
const int32_t LE_MESSAGE_PRIORITY_LOW        = 71;
const int32_t LE_MESSAGE_PRIORITY_NORMAL     = 72;
const int32_t LE_MESSAGE_PRIORITY_HIGH       = 73;
const int32_t LE_OPTION_TYPE_SELECTION       = 78;
const int32_t LE_OPTION_TYPE_RANGE           = 79;
const int32_t LE_ASF_EVENTMUSIC              = 80;
const int32_t LE_ASF_MENUMUSIC               = 81;
const int32_t LE_ASF_VOICE                   = 82;
const int32_t LE_CONTEXT_VALID               = 83;
const int32_t LE_CONTEXT_SUSPENDED           = 84;
const int32_t LE_CONTEXT_INVALID             = 85;
const int32_t LE_FIREBALL_MEDIUM_EXPLOSION   = 86;
const int32_t LE_FIREBALL_LARGE_EXPLOSION    = 87;
const int32_t LE_FIREBALL_WARP_EFFECT        = 88;
const int32_t LE_GR_RESIZE_NONE              = 89; // the sequence and offsets of the LE_GR_* #defines should correspond to the GR_* #defines
const int32_t LE_GR_RESIZE_FULL              = 90;
const int32_t LE_GR_RESIZE_FULL_CENTER       = 91;
const int32_t LE_GR_RESIZE_MENU              = 92;
const int32_t LE_GR_RESIZE_MENU_ZOOMED       = 93;
const int32_t LE_GR_RESIZE_MENU_NO_OFFSET    = 94;
const int32_t LE_TBOX_FLASH_NAME             = 95;
const int32_t LE_TBOX_FLASH_CARGO            = 96;
const int32_t LE_TBOX_FLASH_HULL             = 97;
const int32_t LE_TBOX_FLASH_STATUS           = 98;
const int32_t LE_TBOX_FLASH_SUBSYS           = 99;
const int32_t LE_LUAAI_ACHIEVABLE			 = 100;
const int32_t LE_LUAAI_NOT_YET_ACHIEVABLE	 = 101;
const int32_t LE_LUAAI_UNACHIEVABLE			 = 102;
const int32_t LE_MOVIE_PRE_FICTION           = 103; // the sequence and offsets of the LE_MOVIE_* #defines should correspond to the MOVIE_* #defines
const int32_t LE_MOVIE_PRE_CMD_BRIEF         = 104;
const int32_t LE_MOVIE_PRE_BRIEF             = 105;
const int32_t LE_MOVIE_PRE_GAME              = 106;
const int32_t LE_MOVIE_PRE_DEBRIEF           = 107;
const int32_t LE_MOVIE_POST_DEBRIEF          = 108;
const int32_t LE_MOVIE_END_CAMPAIGN          = 109;
const int32_t LE_SCORE_BRIEFING              = 110; // the sequence and offsets of the LE_SCORE_* #defines should correspond to the SCORE_* #defines
const int32_t LE_SCORE_DEBRIEFING_SUCCESS    = 111;
const int32_t LE_SCORE_DEBRIEFING_AVERAGE    = 112;
const int32_t LE_SCORE_DEBRIEFING_FAILURE    = 113;
const int32_t LE_SCORE_FICTION_VIEWER        = 114;

const int ENUM_NEXT_INDEX = 115; // <<<<<<<<<<<<<<<<<<<<<<
extern flag_def_list Enumerations[];
extern size_t Num_enumerations;

struct enum_h {
	int index;
	bool is_constant;

	enum_h();

	explicit enum_h(int n_index);

	SCP_string getName() const;

	bool IsValid() const;
};

DECLARE_ADE_OBJ(l_Enum, enum_h);

}
}

#endif //FS2_OPEN_ENUMS_H
