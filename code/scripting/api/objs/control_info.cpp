//
//

#include <playerman/player.h>
#include "control_info.h"

namespace {

flag_def_list plr_commands[] = {
	{	"TARGET_NEXT",							TARGET_NEXT,							0	},
	{	"TARGET_PREV",							TARGET_PREV,							0	},
	{	"TARGET_NEXT_CLOSEST_HOSTILE",			TARGET_NEXT_CLOSEST_HOSTILE,			0	},
	{	"TARGET_PREV_CLOSEST_HOSTILE",			TARGET_PREV_CLOSEST_HOSTILE,			0	},
	{	"TOGGLE_AUTO_TARGETING",				TOGGLE_AUTO_TARGETING,					0	},
	{	"TARGET_NEXT_CLOSEST_FRIENDLY",			TARGET_NEXT_CLOSEST_FRIENDLY,			0	},
	{	"TARGET_PREV_CLOSEST_FRIENDLY",			TARGET_PREV_CLOSEST_FRIENDLY,			0	},
	{	"TARGET_SHIP_IN_RETICLE",				TARGET_SHIP_IN_RETICLE,					0	},
	{	"TARGET_CLOSEST_SHIP_ATTACKING_TARGET",	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,	0	},
	{	"TARGET_LAST_TRANMISSION_SENDER",		TARGET_LAST_TRANMISSION_SENDER,			0	},
	{	"STOP_TARGETING_SHIP",					STOP_TARGETING_SHIP,					0	},
	{	"TARGET_SUBOBJECT_IN_RETICLE",			TARGET_SUBOBJECT_IN_RETICLE,			0	},
	{	"TARGET_NEXT_SUBOBJECT",				TARGET_NEXT_SUBOBJECT,					0	},
	{	"TARGET_PREV_SUBOBJECT",				TARGET_PREV_SUBOBJECT,					0	},
	{	"STOP_TARGETING_SUBSYSTEM",				STOP_TARGETING_SUBSYSTEM,				0	},
	{	"MATCH_TARGET_SPEED",					MATCH_TARGET_SPEED,						0	},
	{	"TOGGLE_AUTO_MATCH_TARGET_SPEED",		TOGGLE_AUTO_MATCH_TARGET_SPEED,			0	},
	{	"FIRE_PRIMARY",							FIRE_PRIMARY,							0	},
	{	"FIRE_SECONDARY",						FIRE_SECONDARY,							0	},
	{	"CYCLE_NEXT_PRIMARY",					CYCLE_NEXT_PRIMARY,						0	},
	{	"CYCLE_PREV_PRIMARY",					CYCLE_PREV_PRIMARY,						0	},
	{	"CYCLE_SECONDARY",						CYCLE_SECONDARY,						0	},
	{	"CYCLE_NUM_MISSLES",					CYCLE_NUM_MISSLES,						0	},
	{	"LAUNCH_COUNTERMEASURE",				LAUNCH_COUNTERMEASURE,					0	},
	{	"FORWARD_THRUST",						FORWARD_THRUST,							0	},
	{	"REVERSE_THRUST",						REVERSE_THRUST,							0	},
	{	"BANK_LEFT",							BANK_LEFT,								0	},
	{	"BANK_RIGHT",							BANK_RIGHT,								0	},
	{	"PITCH_FORWARD",						PITCH_FORWARD,							0	},
	{	"PITCH_BACK",							PITCH_BACK,								0	},
	{	"YAW_LEFT",								YAW_LEFT,								0	},
	{	"YAW_RIGHT",							YAW_RIGHT,								0	},
	{	"ZERO_THROTTLE",						ZERO_THROTTLE,							1	},
	{	"MAX_THROTTLE",							MAX_THROTTLE,							1	},
	{	"ONE_THIRD_THROTTLE",					ONE_THIRD_THROTTLE,						1	},
	{	"TWO_THIRDS_THROTTLE",					TWO_THIRDS_THROTTLE,					1	},
	{	"PLUS_5_PERCENT_THROTTLE",				PLUS_5_PERCENT_THROTTLE,				1	},
	{	"MINUS_5_PERCENT_THROTTLE",				MINUS_5_PERCENT_THROTTLE,				1	},
	{	"ATTACK_MESSAGE",						ATTACK_MESSAGE,							1	},
	{	"DISARM_MESSAGE",						DISARM_MESSAGE,							1	},
	{	"DISABLE_MESSAGE",						DISABLE_MESSAGE,						1	},
	{	"ATTACK_SUBSYSTEM_MESSAGE",				ATTACK_SUBSYSTEM_MESSAGE,				1	},
	{	"CAPTURE_MESSAGE",						CAPTURE_MESSAGE,						1	},
	{	"ENGAGE_MESSAGE",						ENGAGE_MESSAGE,							1	},
	{	"FORM_MESSAGE",							FORM_MESSAGE,							1	},
	{	"IGNORE_MESSAGE",						IGNORE_MESSAGE,							1	},
	{	"PROTECT_MESSAGE",						PROTECT_MESSAGE,						1	},
	{	"COVER_MESSAGE",						COVER_MESSAGE,							1	},
	{	"WARP_MESSAGE",							WARP_MESSAGE,							1	},
	{	"REARM_MESSAGE",						REARM_MESSAGE,							1	},
	{	"TARGET_CLOSEST_SHIP_ATTACKING_SELF",	TARGET_CLOSEST_SHIP_ATTACKING_SELF,		1	},
	{	"VIEW_CHASE",							VIEW_CHASE,								1	},
	{	"VIEW_EXTERNAL",						VIEW_EXTERNAL,							1	},
	{	"VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK",		VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,		1	},
	{	"VIEW_SLEW",							VIEW_SLEW,								1	},
	{	"VIEW_OTHER_SHIP",						VIEW_OTHER_SHIP,						1	},
	{	"VIEW_DIST_INCREASE",					VIEW_DIST_INCREASE,						1	},
	{	"VIEW_DIST_DECREASE",					VIEW_DIST_DECREASE,						1	},
	{	"VIEW_CENTER",							VIEW_CENTER,							1	},
	{	"PADLOCK_UP",							PADLOCK_UP,								1	},
	{	"PADLOCK_DOWN",							PADLOCK_DOWN,							1	},
	{	"PADLOCK_LEFT",							PADLOCK_LEFT,							1	},
	{	"PADLOCK_RIGHT",						PADLOCK_RIGHT,							1	},
	{	"RADAR_RANGE_CYCLE",					RADAR_RANGE_CYCLE,						1	},
	{	"SQUADMSG_MENU",						SQUADMSG_MENU,							2	},
	{	"SHOW_GOALS",							SHOW_GOALS,								2	},
	{	"END_MISSION",							END_MISSION,							2	},
	{	"TARGET_TARGETS_TARGET",				TARGET_TARGETS_TARGET,					2	},
	{	"AFTERBURNER",							AFTERBURNER,							2	},
	{	"INCREASE_WEAPON",						INCREASE_WEAPON,						2	},
	{	"DECREASE_WEAPON",						DECREASE_WEAPON,						2	},
	{	"INCREASE_SHIELD",						INCREASE_SHIELD,						2	},
	{	"DECREASE_SHIELD",						DECREASE_SHIELD,						2	},
	{	"INCREASE_ENGINE",						INCREASE_ENGINE,						2	},
	{	"DECREASE_ENGINE",						DECREASE_ENGINE,						2	},
	{	"ETS_EQUALIZE",							ETS_EQUALIZE,							2	},
	{	"SHIELD_EQUALIZE",						SHIELD_EQUALIZE,						2	},
	{	"SHIELD_XFER_TOP",						SHIELD_XFER_TOP,						2	},
	{	"SHIELD_XFER_BOTTOM",					SHIELD_XFER_BOTTOM,						2	},
	{	"SHIELD_XFER_LEFT",						SHIELD_XFER_LEFT,						2	},
	{	"SHIELD_XFER_RIGHT",					SHIELD_XFER_RIGHT,						2	},
	{	"XFER_SHIELD",							XFER_SHIELD,							2	},
	{	"XFER_LASER",							XFER_LASER,								2	},
	{	"GLIDE_WHEN_PRESSED",					GLIDE_WHEN_PRESSED,						2	},
	{	"BANK_WHEN_PRESSED",					BANK_WHEN_PRESSED,						2	},
	{	"SHOW_NAVMAP",							SHOW_NAVMAP,							2	},
	{	"ADD_REMOVE_ESCORT",					ADD_REMOVE_ESCORT,						2	},
	{	"ESCORT_CLEAR",							ESCORT_CLEAR,							2	},
	{	"TARGET_NEXT_ESCORT_SHIP",				TARGET_NEXT_ESCORT_SHIP,				2	},
	{	"TARGET_CLOSEST_REPAIR_SHIP",			TARGET_CLOSEST_REPAIR_SHIP,				2	},
	{	"TARGET_NEXT_UNINSPECTED_CARGO",		TARGET_NEXT_UNINSPECTED_CARGO,			2	},
	{	"TARGET_PREV_UNINSPECTED_CARGO",		TARGET_PREV_UNINSPECTED_CARGO,			2	},
	{	"TARGET_NEWEST_SHIP",					TARGET_NEWEST_SHIP,						2	},
	{	"TARGET_NEXT_LIVE_TURRET",				TARGET_NEXT_LIVE_TURRET,				2	},
	{	"TARGET_PREV_LIVE_TURRET",				TARGET_PREV_LIVE_TURRET,				2	},
	{	"TARGET_NEXT_BOMB",						TARGET_NEXT_BOMB,						2	},
	{	"TARGET_PREV_BOMB",						TARGET_PREV_BOMB,						3	},
	{	"MULTI_MESSAGE_ALL",					MULTI_MESSAGE_ALL,						3	},
	{	"MULTI_MESSAGE_FRIENDLY",				MULTI_MESSAGE_FRIENDLY,					3	},
	{	"MULTI_MESSAGE_HOSTILE",				MULTI_MESSAGE_HOSTILE,					3	},
	{	"MULTI_MESSAGE_TARGET",					MULTI_MESSAGE_TARGET,					3	},
	{	"MULTI_OBSERVER_ZOOM_TO",				MULTI_OBSERVER_ZOOM_TO,					3	},
	{	"TIME_SPEED_UP",						TIME_SPEED_UP,							3	},
	{	"TIME_SLOW_DOWN",						TIME_SLOW_DOWN,							3	},
	{	"TOGGLE_HUD_CONTRAST",					TOGGLE_HUD_CONTRAST,					3	},
	{	"TOGGLE_HUD_SHADOWS",					TOGGLE_HUD_SHADOWS,						3	},
	{	"MULTI_TOGGLE_NETINFO",					MULTI_TOGGLE_NETINFO,					3	},
	{	"MULTI_SELF_DESTRUCT",					MULTI_SELF_DESTRUCT,					3	},
	{	"TOGGLE_HUD",							TOGGLE_HUD,								3	},
	{	"RIGHT_SLIDE_THRUST",					RIGHT_SLIDE_THRUST,						3	},
	{	"LEFT_SLIDE_THRUST",					LEFT_SLIDE_THRUST,						3	},
	{	"UP_SLIDE_THRUST",						UP_SLIDE_THRUST,						3	},
	{	"DOWN_SLIDE_THRUST",					DOWN_SLIDE_THRUST,						3	},
	{	"HUD_TARGETBOX_TOGGLE_WIREFRAME",		HUD_TARGETBOX_TOGGLE_WIREFRAME,			3	},
	{	"VIEW_TOPDOWN",							VIEW_TOPDOWN,							3	},
	{	"VIEW_TRACK_TARGET",					VIEW_TRACK_TARGET,						3	},
	{	"AUTO_PILOT_TOGGLE",					AUTO_PILOT_TOGGLE,						3	},
	{	"NAV_CYCLE",							NAV_CYCLE,								3	},
	{	"TOGGLE_GLIDING",						TOGGLE_GLIDING,							3	},
};

int num_plr_commands = sizeof(plr_commands)/sizeof(flag_def_list);

}

namespace scripting {
namespace api {

//**********HANDLE: Control Info
ADE_OBJ(l_Control_Info, int, "control_info", "control info handle");

ADE_VIRTVAR(Pitch, l_Control_Info, "number", "Pitch of the player ship", "number", "Pitch")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.pitch = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.pitch);
}

ADE_VIRTVAR(Heading, l_Control_Info, "number", "Heading of the player ship", "number", "Heading")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.heading = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.heading);
}

ADE_VIRTVAR(Bank, l_Control_Info, "number", "Bank of the player ship", "number", "Bank")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.bank = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.bank);
}

ADE_VIRTVAR(Vertical, l_Control_Info, "number", "Vertical control of the player ship", "number", "Vertical control")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.vertical = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.vertical);
}

ADE_VIRTVAR(Sideways, l_Control_Info, "number", "Sideways control of the player ship", "number", "Sideways control")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.sideways = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.sideways);
}

ADE_VIRTVAR(Forward, l_Control_Info, "number", "Forward control of the player ship", "number", "Forward")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.forward = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.forward);
}

ADE_VIRTVAR(ForwardCruise, l_Control_Info, "number", "Forward control of the player ship", "number", "Forward")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci) || Player == nullptr)
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.forward_cruise_percent = new_ci*100.0f;
	}

	return ade_set_args(L, "f", Player->lua_ci.forward_cruise_percent*0.01f);
}

ADE_VIRTVAR(PrimaryCount, l_Control_Info, "number", "Number of primary weapons that will fire", "number", "Number of weapons to fire, or 0 if handle is invalid")
{
	int idx;
	int new_pri = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_pri) || Player == nullptr)
		return ade_set_error(L, "i", new_pri);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_primary_count = new_pri;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_primary_count);
}

ADE_VIRTVAR(SecondaryCount, l_Control_Info, "number", "Number of secondary weapons that will fire", "number", "Number of weapons to fire, or 0 if handle is invalid")
{
	int idx;
	int new_sec = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_sec) || Player == nullptr)
		return ade_set_error(L, "i", new_sec);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_secondary_count = new_sec;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_secondary_count);
}

ADE_VIRTVAR(CountermeasureCount, l_Control_Info, "number", "Number of countermeasures that will launch", "number", "Number of countermeasures to launch, or 0 if handle is invalid")
{
	int idx;
	int new_cm = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_cm) || Player == nullptr)
		return ade_set_error(L, "i", new_cm);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_countermeasure_count = new_cm;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_countermeasure_count);
}

ADE_FUNC(clearLuaButtonInfo, l_Control_Info, NULL, "Clears the lua button control info", NULL, NULL)
{
	SCP_UNUSED(L); // unused parameter

	if (Player != nullptr) {
		button_info_clear(&Player->lua_bi);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(getButtonInfo,
	l_Control_Info,
	nullptr,
	"Access the four bitfields containing the button info",
	"number, number, number, number",
	"Four bitfields")
{
	int i;
	int bi_status[4];

	if (Player == nullptr) {
		return ade_set_error(L, "iiii", 0, 0, 0, 0);
	}

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}

ADE_FUNC(accessButtonInfo,
	l_Control_Info,
	"number, number, number, number",
	"Access the four bitfields containing the button info",
	"number, number, number, number",
	"Four bitfields")
{
	int i;
	int bi_status[4];

	for(i=0;i<4;i++)
		bi_status[i] = 0;

	if(!ade_get_args(L, "|iiii", &bi_status[0], &bi_status[1], &bi_status[2], &bi_status[3]) || Player == nullptr)
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR) {
		for(i=0;i<4;i++)
			Player->lua_bi.status[i] = bi_status[i];
	}

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}

ADE_FUNC(useButtonControl, l_Control_Info, "number, string", "Adds the defined button control to lua button control data, if number is -1 it tries to use the string", NULL, NULL)
{
	int index;
	const char* buf = nullptr;

	if(!ade_get_args(L, "i|s", &index, &buf) || Player == nullptr)
		return ADE_RETURN_NIL;

	if(index != -1) {
		// Process the number
		if (index > (4 * 32))
			return ADE_RETURN_NIL;

		int a, b;
		a = index / 32;
		b = index % 32;

		// now add the processed bit
		Player->lua_bi.status[a] |= (1<<b);
	} else if (buf != NULL) {
		int i;
		for(i=0; i<num_plr_commands; i++) {
			if(!(strcmp(buf, plr_commands[i].name))) {
				int a;
				a = plr_commands[i].def % 32;
				Player->lua_bi.status[plr_commands[i].var] |= (1<<a);
				break;
			}
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(getButtonControlName, l_Control_Info, "number", "Gives the name of the command corresponding with the given number", "string", "Name of the command")
{
	int index;

	if(!ade_get_args(L, "i", &index))
		return ade_set_error(L, "s", "");

	if((index < 0) || (index > num_plr_commands))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr_commands[index].name);
}

ADE_FUNC(getButtonControlNumber, l_Control_Info, "string", "Gives the number of the command corresponding with the given string", "number", "Number of the command")
{
	int i;
	const char* buf;

	if(!ade_get_args(L, "s", &buf))
		return ade_set_error(L, "i", -1);

	for(i = 0; i < num_plr_commands; i++) {
		if (!(strcmp(buf, plr_commands[i].name))) {
			return ade_set_args(L, "i", plr_commands[i].def);
		}
	}

	return ade_set_error(L, "i", -1);
}

ADE_VIRTVAR(AllButtonPolling, l_Control_Info, "boolean", "Toggles the all button polling for lua", "boolean", "If the all button polling is enabled or not")
{
	bool p;
	int idx;

	if(!ade_get_args(L, "o|b", l_Control_Info.Get(&idx), &p))
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		if (p)
			lua_game_control |= LGC_B_POLL_ALL;
		else
			lua_game_control &= ~LGC_B_POLL_ALL;
	}

	if (lua_game_control & LGC_B_POLL_ALL)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(pollAllButtons,
	l_Control_Info,
	nullptr,
	"Access the four bitfields containing the button info",
	"number, number, number, number",
	"Four bitfields")
{
	int i;
	int bi_status[4];

	if(!(lua_game_control & LGC_B_POLL_ALL) || Player == nullptr)
		return ADE_RETURN_NIL;

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi_full.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}


}
}

