#include <stdio.h>
#include <stdarg.h>
#include "parse/scripting.h"
#include "parse/lua.h"
#include "parse/parselo.h"
#include "globalincs/version.h"
#include "gamesequence/gamesequence.h"
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "hud/hud.h"
#include "ship/ship.h"
#include "mission/missioncampaign.h"
#include "weapon/weapon.h"
#include "io/key.h"
#include "controlconfig/controlsconfig.h"
#include "freespace2/freespace.h"
#include "weapon/beam.h"

//tehe. Declare the main event
script_state Script_system("FS2_Open Scripting");
bool Output_scripting_meta = false;

//*************************Scripting hook globals*************************
script_hook Script_splashhook;
script_hook Script_simulationhook;
script_hook Script_hudhook;
script_hook Script_globalhook;
script_hook Script_gameinithook;

flag_def_list Script_conditions[] = 
{
	{"State",		CHC_STATE,			0},
	{"Campaign",	CHC_CAMPAIGN,		0},
	{"Mission",		CHC_MISSION,		0},
	{"Object Type", CHC_OBJECTTYPE,		0},
	{"Ship",		CHC_SHIP,			0},
	{"Ship class",	CHC_SHIPCLASS,		0},
	{"Ship type",	CHC_SHIPTYPE,		0},
	{"Weapon class",CHC_WEAPONCLASS,	0},
	{"KeyPress",	CHC_KEYPRESS,		0},
	{"Action",		CHC_ACTION,			0},
	{"Version",		CHC_VERSION,		0},
	{"Application",	CHC_APPLICATION,	0}
};

int Num_script_conditions = sizeof(Script_conditions)/sizeof(flag_def_list);

flag_def_list Script_actions[] = 
{
	{"On Game Init",			CHA_GAMEINIT,		0},
	{"On Splash Screen",		CHA_SPLASHSCREEN,	0},
	{"On State Start",			CHA_ONSTATESTART,	0},
	{"On Frame",				CHA_ONFRAME,		0},
	{"On Action",				CHA_ONACTION,		0},
	{"On Action Stopped",		CHA_ONACTIONSTOPPED,0},
	{"On Key Pressed",			CHA_KEYPRESSED,		0},
	{"On Key Released",			CHA_KEYRELEASED,	0},
	{"On Mouse Moved",			CHA_MOUSEMOVED,		0},
	{"On Mouse Pressed",		CHA_MOUSEPRESSED,	0},
	{"On Mouse Released",		CHA_MOUSERELEASED,	0},
	{"On State End",			CHA_ONSTATEEND,		0},
	{"On Mission Start",		CHA_MISSIONSTART,	0},
	{"On HUD Draw",				CHA_HUDDRAW,		0},
	{"On Ship Collision",		CHA_COLLIDESHIP,	0},
	{"On Weapon Collision",		CHA_COLLIDEWEAPON,	0},
	{"On Debris Collision",		CHA_COLLIDEDEBRIS,	0},
	{"On Asteroid Collision",	CHA_COLLIDEASTEROID,0},
	{"On Object Render",		CHA_OBJECTRENDER,	0},
	{"On Warp In",				CHA_WARPIN,			0},
	{"On Warp Out",				CHA_WARPOUT,		0},
	{"On Death",				CHA_DEATH,			0},
	{"On Mission End",			CHA_MISSIONEND,		0},
	{"On Weapon Delete",		CHA_ONWEAPONDELETE,	0},
	{"On Weapon Equipped",		CHA_ONWPEQUIPPED,	0},
	{"On Weapon Fired",			CHA_ONWPFIRED,		0},
	{"On Weapon Selected",		CHA_ONWPSELECTED,	0},
	{"On Weapon Deselected",	CHA_ONWPDESELECTED,	0},
	{"On Gameplay Start",		CHA_GAMEPLAYSTART,	0},
	{"On Turret Fired",			CHA_ONTURRETFIRED,	0},
	{"On Primary Fire",			CHA_PRIMARYFIRE,	0},
	{"On Secondary Fire",		CHA_SECONDARYFIRE,	0},
	{"On Ship Arrive",			CHA_ONSHIPARRIVE,	0},
	{"On Beam Collision",		CHA_COLLIDEBEAM,	0}
};

int Num_script_actions = sizeof(Script_actions)/sizeof(flag_def_list);
int scripting_state_inited = 0;

//*************************Scripting init and handling*************************

// the prototype for this is in pstypes.h, below the script_hook struct
void script_hook_init(script_hook *hook)
{
	hook->o_language = 0;
	hook->h_language = 0;

	hook->o_index = -1;
	hook->h_index = -1;
}

// ditto
bool script_hook_valid(script_hook *hook)
{
	return hook->h_index >= 0;
}

void script_parse_table(const char *filename)
{
	script_state *st = &Script_system;
	int rval;
	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		return;
	}

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	if(optional_string("#Global Hooks"))
	{
		//int num = 42;
		//Script_system.SetHookVar("Version", 'i', &num);
		if(optional_string("$Global:")) {
			st->ParseChunk(&Script_globalhook, "Global");
		}

		if(optional_string("$Splash:")) {
			st->ParseChunk(&Script_splashhook, "Splash");
		}

		if(optional_string("$GameInit:")) {
			st->ParseChunk(&Script_gameinithook, "GameInit");
		}

		if(optional_string("$Simulation:")) {
			st->ParseChunk(&Script_simulationhook, "Simulation");
		}

		if(optional_string("$HUD:")) {
			st->ParseChunk(&Script_hudhook, "HUD");
		}

		required_string("#End");
	}
/*
	if(optional_string("#State Hooks"))
	{
		while(optional_string("$State:")) {
			char buf[NAME_LENGTH];
			int idx;
			stuff_string(buf, F_NAME, sizeof(buf));

			idx = gameseq_get_state_idx(buf);

			if(optional_string("$Hook:"))
			{
				if(idx > -1) {
					GS_state_hooks[idx] = st->ParseChunk(buf);
				} else {
					st->ParseChunk(buf);
				}
			}
		}
		required_string("#End");
	}
*/
	if(optional_string("#Conditional Hooks"))
	{
		while(st->ParseCondition(filename));
		required_string("#End");
	}

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(const char *tblname);
	fs2netd_add_table_validation(filename);
}

//Initializes the (global) scripting system, as well as any subsystems.
//script_close is handled by destructors
void script_init()
{
	mprintf(("SCRIPTING: Beginning initialization sequence...\n"));

	// first things first: init all script hooks, since they are PODs now, not classes...
	script_hook_init(&Script_splashhook);
	script_hook_init(&Script_simulationhook);
	script_hook_init(&Script_hudhook);
	script_hook_init(&Script_globalhook);
	script_hook_init(&Script_gameinithook);

	mprintf(("SCRIPTING: Beginning Lua initialization...\n"));
	Script_system.CreateLuaState();

	if(Output_scripting_meta)
	{
		mprintf(("SCRIPTING: Outputting scripting metadata...\n"));
		Script_system.OutputMeta("scripting.html");
	}
	mprintf(("SCRIPTING: Beginning main hook parse sequence....\n"));
	script_parse_table("scripting.tbl");
	parse_modular_table(NOX("*-sct.tbm"), script_parse_table);
	mprintf(("SCRIPTING: Inititialization complete.\n"));
}
/*
//WMC - Doesn't work as debug console interferes with any non-alphabetic chars.
DCF(script, "Evaluates a line of scripting")
{
	if(Dc_command)
	{
		dc_get_arg(ARG_STRING);
		Script_system.EvalString(Dc_arg);
	}

	if(Dc_help)
	{
		dc_printf("Usage: script <script\n");
		dc_printf("<script> --  Scripting to evaluate.\n");
	}
}
*/

//*************************CLASS: ConditionedScript*************************
extern char Game_current_mission_filename[];
bool ConditionedHook::AddCondition(script_condition *sc)
{
	for(int i = 0; i < MAX_HOOK_CONDITIONS; i++)
	{
		if(Conditions[i].condition_type == CHC_NONE)
		{
			Conditions[i] = *sc;
			return true;
		}
	}

	return false;
}

bool ConditionedHook::AddAction(script_action *sa)
{
	if(!script_hook_valid(&sa->hook))
		return false;

	Actions.push_back(*sa);

	return true;
}

bool ConditionedHook::ConditionsValid(int action, object *objp, int more_data)
{
	uint i;

	//Return false if any conditions are not met
	script_condition *scp;
	ship_info *sip;
	for(i = 0; i < MAX_HOOK_CONDITIONS; i++)
	{
		scp = &Conditions[i];
		switch(scp->condition_type)
		{
			case CHC_STATE:
				if(gameseq_get_depth() < 0)
					return false;
				if(stricmp(GS_state_text[gameseq_get_state(0)], scp->data.name))
					return false;
				break;
			case CHC_SHIPTYPE:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				sip = &Ship_info[Ships[objp->instance].ship_info_index];
				if(sip->class_type < 0)
					return false;
				if(stricmp(Ship_types[sip->class_type].name, scp->data.name))
					return false;
			case CHC_SHIPCLASS:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				if(stricmp(Ship_info[Ships[objp->instance].ship_info_index].name, scp->data.name))
					return false;
				break;
			case CHC_SHIP:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				if(stricmp(Ships[objp->instance].ship_name, scp->data.name))
					return false;
				break;
			case CHC_MISSION:
				{
					//WMC - Get mission filename with Mission_filename
					//I don't use Game_current_mission_filename, because
					//Mission_filename is valid in both fs2_open and FRED
					size_t len = strlen(Mission_filename);
					if(!len)
						return false;
					if(len > 4 && !stricmp(&Mission_filename[len-4], ".fs2"))
						len -= 4;
					if(strnicmp(scp->data.name, Mission_filename, len))
						return false;
					break;
				}
			case CHC_CAMPAIGN:
				{
					size_t len = strlen(Campaign.filename);
					if(!len)
						return false;
					if(len > 4 && !stricmp(&Mission_filename[len-4], ".fc2"))
						len -= 4;
					if(strnicmp(scp->data.name, Mission_filename, len))
						return false;
					break;
				}
			case CHC_WEAPONCLASS:
				{
					if (action == CHA_COLLIDEWEAPON) {
						if (stricmp(Weapon_info[more_data].name, scp->data.name) != 0)
							return false;
					} else if (!(action == CHA_ONWPSELECTED || action == CHA_ONWPDESELECTED || action == CHA_ONWPEQUIPPED || action == CHA_ONWPFIRED || action == CHA_ONTURRETFIRED )) {
						if(objp == NULL || (objp->type != OBJ_WEAPON && objp->type != OBJ_BEAM))
							return false;
						else if (( objp->type == OBJ_WEAPON) && (stricmp(Weapon_info[Weapons[objp->instance].weapon_info_index].name, scp->data.name) != 0 ))
							return false;
						else if (( objp->type == OBJ_BEAM) && (stricmp(Weapon_info[Beams[objp->instance].weapon_info_index].name, scp->data.name) != 0 ))
							return false;
					} else if(objp == NULL || objp->type != OBJ_SHIP) {
						return false;
					} else {

						// Okay, if we're still here, then objp is both valid and a ship
						ship* shipp = &Ships[objp->instance];
						bool primary = false, secondary = false, prev_primary = false, prev_secondary = false;
						switch (action) {
							case CHA_ONWPSELECTED:
								primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
								secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								
								if (!(primary || secondary))
									return false;

								if ((shipp->flags & SF_PRIMARY_LINKED) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags3 & WIF3_NOLINK))
									return false;
								
								break;
							case CHA_ONWPDESELECTED:
								primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
								prev_primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].name, scp->data.name) == 0;
								secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								prev_secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.previous_secondary_bank]].name, scp->data.name) == 0;

								if ((shipp->flags & SF_PRIMARY_LINKED) && prev_primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].wi_flags3 & WIF3_NOLINK))
									return true;

								if ( !prev_secondary && ! secondary && !prev_primary && !primary )
									return false;

								if ( (!prev_secondary && !secondary) && (prev_primary && primary) ) 
									return false;

								if ( (!prev_secondary && !secondary) && (!prev_primary && primary) ) 
									return false;

								if ( (!prev_primary && !primary) && (prev_secondary && secondary) )
									return false;

								if ( (!prev_primary && !primary) && (!prev_secondary && secondary) )
									return false;

								break;
							case CHA_ONWPEQUIPPED: {
								bool equipped = false;
								for(int j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
									if (!equipped && (shipp->weapons.primary_bank_weapons[j] >= 0) && (shipp->weapons.primary_bank_weapons[j] < MAX_WEAPON_TYPES) ) {
										if ( !stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[j]].name, scp->data.name) ) {
											equipped = true;
											break;
										}
									}
								}
							
								if (!equipped) {
									for(int j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++) {
										if (!equipped && (shipp->weapons.secondary_bank_weapons[j] >= 0) && (shipp->weapons.secondary_bank_weapons[j] < MAX_WEAPON_TYPES) ) {
											if ( !stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[j]].name, scp->data.name) ) {
												equipped = true;
												break;
											}
										}
									}
								}

								if (!equipped)
									return false;
							
								break;
							}
							case CHA_ONWPFIRED: {
								if (more_data == 1) {
									primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
									secondary = false;
								} else {
									primary = false;
									secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								}

								if ((shipp->flags & SF_PRIMARY_LINKED) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags3 & WIF3_NOLINK))
								 	return false;

								return more_data == 1 ? primary : secondary;

								break;
							}
							case CHA_ONTURRETFIRED: {
								if (! (stricmp(Weapon_info[shipp->last_fired_turret->last_fired_weapon_info_index].name, scp->data.name) == 0))
									return false;
								break;
							}
							case CHA_PRIMARYFIRE: {
								if (stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name))
									return false;
								break;
							}
							case CHA_SECONDARYFIRE: {
								if (stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name))
									return false;
								break;
							}

						}
					} // case CHC_WEAPONCLASS
					break;
				}
			case CHC_OBJECTTYPE:
				if(objp == NULL)
					return false;
				if(stricmp(Object_type_names[objp->type], scp->data.name))
					return false;
				break;
			case CHC_KEYPRESS:
				{
					extern int Current_key_down;
					if(gameseq_get_depth() < 0)
						return false;
					if(Current_key_down == 0)
						return false;
					//WMC - could be more efficient, but whatever.
					if(stricmp(textify_scancode(Current_key_down), scp->data.name))
						return false;
					break;
				}
			case CHC_ACTION:
				{
					if(gameseq_get_depth() < 0)
						return false;

					int action_index = more_data;

					if (action_index <= 0 || stricmp(scp->data.name, Control_config[action_index].text))
						return false;
					break;
				}
			case CHC_VERSION:
				{
					// Goober5000: I'm going to assume scripting doesn't care about SVN revision
					char buf[32];
					sprintf(buf, "%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
					if(stricmp(buf, scp->data.name))
					{
						//In case some people are lazy and say "3.7" instead of "3.7.0" or something
						if(FS_VERSION_BUILD == 0)
						{
							sprintf(buf, "%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR);
							if(stricmp(buf, scp->data.name))
								return false;
						}
						else
						{
							return false;
						}
					}
					break;
				}
			case CHC_APPLICATION:
				{
					if(Fred_running)
					{
						if(stricmp("FRED2_Open", scp->data.name) && stricmp("FRED2Open", scp->data.name) && stricmp("FRED 2", scp->data.name) && stricmp("FRED", scp->data.name))
							return false;
					}
					else
					{
						if(stricmp("FS2_Open", scp->data.name) && stricmp("FS2Open", scp->data.name) && stricmp("Freespace 2", scp->data.name) && stricmp("Freespace", scp->data.name))
							return false;
					}
				}
			default:
				break;
		}
	}

	return true;
}

bool ConditionedHook::Run(script_state *sys, int action, char format, void *data)
{
	Assert(sys != NULL);

	//Do the actions
	for(SCP_vector<script_action>::iterator sap = Actions.begin(); sap != Actions.end(); ++sap)
	{
		if(sap->action_type == action)
			sys->RunBytecode(sap->hook, format, data);
	}

	return true;
}

bool ConditionedHook::IsOverride(script_state *sys, int action)
{
	Assert(sys != NULL);
	//bool b = false;

	//Do the actions
	for(SCP_vector<script_action>::iterator sap = Actions.begin(); sap != Actions.end(); ++sap)
	{
		if(sap->action_type == action)
		{
			if(sys->IsOverride(sap->hook))
				return true;
		}
	}

	return false;
}

//*************************CLASS: script_state*************************
//Most of the icky stuff is here. Lots of #ifdefs

//WMC - defined in parse/lua.h
int ade_set_object_with_breed(lua_State *L, int obj_idx);
void script_state::SetHookObject(char *name, object *objp)
{
	SetHookObjects(1, name, objp);
}

void script_state::SetHookObjects(int num, ...)
{
	va_list vl;
	va_start(vl, num);
	if(this->OpenHookVarTable())
	{
		int amt_ldx = lua_gettop(LuaState);
		for(int i = 0; i < num; i++)
		{
			char *name = va_arg(vl, char*);
			object *objp = va_arg(vl, object*);
			
			ade_set_object_with_breed(LuaState, OBJ_INDEX(objp));
			int data_ldx = lua_gettop(LuaState);

			lua_pushstring(LuaState, name);
			lua_pushvalue(LuaState, data_ldx);
			lua_rawset(LuaState, amt_ldx);

			lua_pop(LuaState, 1);	//data_ldx
		}
		this->CloseHookVarTable();
	}
	else
	{
		LuaError(LuaState, "Could not get HookVariable library to add hook variables - get a coder");
	}
	va_end(vl);
}

//This pair of abstraction functions handles
//getting the table of ade member functions
//for the hook library.
//Call CloseHookVarTable() only if OpenHookVarTable()
//returns true. (see below)
static int ohvt_poststack = 0;		//Items on the stack prior to OHVT
static int ohvt_isopen = 0;			//Items OHVT puts on the stack
bool script_state::OpenHookVarTable()
{
	if(ohvt_isopen)
		Error(LOCATION, "OpenHookVarTable was called twice with no call to CloseHookVarTable - missing call ahoy!");

	lua_pushstring(LuaState, "hv");
	lua_gettable(LuaState, LUA_GLOBALSINDEX);
	int sv_ldx = lua_gettop(LuaState);
	if(lua_isuserdata(LuaState, sv_ldx))
	{
		//Get ScriptVar metatable
		lua_getmetatable(LuaState, sv_ldx);
		int mtb_ldx = lua_gettop(LuaState);
		if(lua_istable(LuaState, mtb_ldx))
		{
			//Get ScriptVar/metatable/__ademembers
			lua_pushstring(LuaState, "__ademembers");
			lua_rawget(LuaState, mtb_ldx);
			int amt_ldx = lua_gettop(LuaState);
			if(lua_istable(LuaState, amt_ldx))
			{
				ohvt_isopen = 3;
				ohvt_poststack = amt_ldx;
				return true;
			}
			lua_pop(LuaState, 1);	//amt
		}
		lua_pop(LuaState, 1);	//metatable
	}
	lua_pop(LuaState, 1);	//Library
	
	return false;
}

//Call when you are done with CloseHookVarTable,
//and you have removed all other objects 'above'
//the stack objects generated by the call to OpenHookVarTable.
bool script_state::CloseHookVarTable()
{
	if(!ohvt_isopen)
	{
		Error(LOCATION, "CloseHookVarTable was called with no associated call to OpenHookVarTable");
	}
	int top_ldx = lua_gettop(LuaState);
	if(top_ldx >= ohvt_poststack)
	{
		lua_pop(LuaState, ohvt_isopen);
		ohvt_isopen = 0;
		return true;
	}
	else
	{
		Error(LOCATION, "CloseHookVarTable() was called with too few objects on the stack; get a coder. (Stack: %d OHVT post: %d OHVT num: %d", top_ldx, ohvt_poststack, ohvt_isopen);
		return false;
	}
}

void script_state::SetHookVar(char *name, char format, void *data)
{
	if(format == '\0')
		return;

	if(LuaState != NULL)
	{
		char fmt[2] = {format, '\0'};
		int data_ldx = 0;
		if(data == NULL)
			data_ldx = lua_gettop(LuaState);

		if(data_ldx < 1 && data == NULL)
			return;

		//Get ScriptVar table
		if(this->OpenHookVarTable())
		{
			int amt_ldx = lua_gettop(LuaState);
			lua_pushstring(LuaState, name);
			//ERRORS? LOOK HERE!!!
			//--------------------
			//WMC - Now THIS has to be the nastiest hack I've made
			//Basically, I tell it to copy over enough stack
			//for a ade_odata object. If you pass
			//_anything_ larger as a stack object, this will not work.
			//You'll get memory corruption
			if(data == NULL)
			{
				lua_pushvalue(LuaState, data_ldx);
			}
			else
			{
				if(format == 's')
					ade_set_args(LuaState, fmt, data);
				else
					ade_set_args(LuaState, fmt, *(ade_odata*)data);
			}
			//--------------------
			//WMC - This was a separate function
			//lua_set_arg(LuaState, format, data);
			//WMC - switch to the lua library
			//lua_setglobal(LuaState, name);
			lua_rawset(LuaState, amt_ldx);
			
			if(data_ldx)
				lua_pop(LuaState, 1);
			//Close hook var table
			this->CloseHookVarTable();
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to set hook variable '%s' - get a coder", name);
			if(data_ldx)
				lua_pop(LuaState, 1);
		}
	}
}

//WMC - data can be NULL, if we just want to know if it exists
bool script_state::GetHookVar(char *name, char format, void *data)
{
	bool got_global = false;
	if(LuaState != NULL)
	{
		//Construct format string
		char fmt[3] = {'|', format, '\0'};

		//WMC - Quick and clean. :)
		//WMC - *sigh* nostalgia
		//Get ScriptVar table
		if(this->OpenHookVarTable())
		{
			int amt_ldx = lua_gettop(LuaState);

			lua_pushstring(LuaState, name);
			lua_rawget(LuaState, amt_ldx);
			if(!lua_isnil(LuaState, -1))
			{
				if(data != NULL) {
					ade_get_args(LuaState, fmt, data);
				}
				got_global = true;
			}
			lua_pop(LuaState, 1);	//Remove data

			this->CloseHookVarTable();
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to get hook variable '%' - get a coder", name);
		}
	}

	return got_global;
}

void script_state::RemHookVar(char *name)
{
	this->RemHookVars(1, name);
}

void script_state::RemHookVars(unsigned int num, ...)
{
	if(LuaState != NULL)
	{
		//WMC - Quick and clean. :)
		//WMC - *sigh* nostalgia
		//Get ScriptVar table
		if(this->OpenHookVarTable())
		{
			int amt_ldx = lua_gettop(LuaState);
			va_list vl;
			va_start(vl, num);
			for(unsigned int i = 0; i < num; i++)
			{
				char *name = va_arg(vl, char*);
				lua_pushstring(LuaState, name);
				lua_pushnil(LuaState);
				lua_rawset(LuaState, amt_ldx);
			}
			va_end(vl);

			this->CloseHookVarTable();
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to remove hook variables - get a coder");
		}
	}
}

//WMC - data can be NULL, if we just want to know if it exists
bool script_state::GetGlobal(char *name, char format, void *data)
{
	bool got_global = false;
	if(LuaState != NULL)
	{
		//Construct format string
		char fmt[3] = {'|', format, '\0'};

		lua_getglobal(LuaState, name);
		//Does global exist?
		if(!lua_isnil(LuaState, -1))
		{
			if(data != NULL) {
				ade_get_args(LuaState, fmt, data);
			}
			got_global = true;
		}
		lua_pop(LuaState, 1);	//Remove data
	}

	return got_global;
}

void script_state::RemGlobal(char *name)
{
	if(LuaState != NULL)
	{
		//WMC - Quick and clean. :)
		lua_pushnil(LuaState);
		lua_setglobal(LuaState, name);
	}
}

int script_state::LoadBm(char *name)
{
	for(int i = 0; i < (int)ScriptImages.size(); i++)
	{
		if(!stricmp(name, ScriptImages[i].fname))
			return ScriptImages[i].handle;
	}

	image_desc id;
	int idx = bm_load(name);

	if(idx > -1) {
		id.handle = idx;
		strcpy_s(id.fname, name);
		ScriptImages.push_back(id);
	}

	return idx;
}

void script_state::UnloadImages()
{
	for(int i = 0; i < (int)ScriptImages.size(); i++)
	{
		bm_unload(ScriptImages[i].handle);
	}

	ScriptImages.clear();
}

int script_state::RunBytecodeSub(int in_lang, int in_idx, char format, void *data)
{
	if(in_idx >= 0)
	{
		// if this is the hook for the hud, and the hud is disabled or we are in freelook mode, then don't run the script
		// (admittedly this is wrong, but I'm not sure where else to put this check and have it work properly. hopefully
		//  WMCoolmon can get some time to come back over this and fix the issues, or just make it better period. - taylor)
		//WMC - Barf barf icky hack. Maybe later.
		if (in_idx == Script_hudhook.h_index) {
			if ( (Viewer_mode & VM_FREECAMERA) || hud_disabled() ) {
				return 1;
			}
		}

		if(in_lang == SC_LUA)
		{
			lua_pushcfunction(GetLuaSession(), ade_friendly_error);
			int err_ldx = lua_gettop(GetLuaSession());
			if(!lua_iscfunction(GetLuaSession(), err_ldx))
			{
				lua_pop(GetLuaSession(), 1);
				return 0;
			}
			
			int args_start = lua_gettop(LuaState);
			
			lua_getref(GetLuaSession(), in_idx);
			int hook_ldx = lua_gettop(GetLuaSession());

			if(lua_isfunction(GetLuaSession(), hook_ldx))
			{
				if(lua_pcall(GetLuaSession(), 0, format!='\0' ? 1 : 0, err_ldx) != 0)
				{
					//WMC - Pop all extra stuff from ze stack.
					args_start = lua_gettop(GetLuaSession()) - args_start;
					for(; args_start > 0; args_start--) lua_pop(GetLuaSession(), 1);

					lua_pop(GetLuaSession(), 1);
					return 0;
				}

				//WMC - Just allow one argument for now.
				if(data != NULL)
				{
					char fmt[2] = {format, '\0'};
					Ade_get_args_skip = args_start;
					Ade_get_args_lfunction = true;
					ade_get_args(LuaState, fmt, data);
					Ade_get_args_skip = 0;
					Ade_get_args_lfunction = false;
				}

				//WMC - Pop anything leftover from the function from the stack
				args_start = lua_gettop(GetLuaSession()) - args_start;
				for(; args_start > 0; args_start--) lua_pop(GetLuaSession(), 1);
			}
			else
			{
				lua_pop(GetLuaSession(), 1);	//"hook"
				LuaError(GetLuaSession(),"RunBytecodeSub tried to call a non-function value!");
			}
			lua_pop(GetLuaSession(), 1);	//err
			
		}
	}

	return 1;
}

//returns 0 on failure (Parse error), 1 on success
int script_state::RunBytecode(script_hook &hd, char format, void *data)
{
	RunBytecodeSub(hd.h_language, hd.h_index, format, data);
	return 1;
}

int script_state::RunCondition(int action, char format, void *data, object *objp, int more_data)
{
	int num = 0;
	for(SCP_vector<ConditionedHook>::iterator chp = ConditionalHooks.begin(); chp != ConditionalHooks.end(); ++chp) 
	{
		if(chp->ConditionsValid(action, objp, more_data))
		{
			chp->Run(this, action, format, data);
			num++;
		}
	}
	return num;
}

bool script_state::IsConditionOverride(int action, object *objp)
{
	//bool b = false;
	for(SCP_vector<ConditionedHook>::iterator chp = ConditionalHooks.begin(); chp != ConditionalHooks.end(); ++chp)
	{
		if(chp->ConditionsValid(action, objp))
		{
			if(chp->IsOverride(this, action))
				return true;
		}
	}
	return false;
}

void script_state::EndFrame()
{
	EndLuaFrame();
}

void script_state::Clear()
{
	StateName[0] = '\0';
	Langs = 0;

	//Don't close this yet
	LuaState = NULL;
	LuaLibs = NULL;
}

script_state::script_state(char *name)
{
	strncpy(StateName, name, sizeof(StateName)-1);

	Langs = 0;

	LuaState = NULL;
	LuaLibs = NULL;
}

script_state& script_state::operator=(script_state &in)
{
	Error(LOCATION, "SESSION COPY ATTEMPTED");

	return *this;
}

script_state::~script_state()
{
	if(LuaState != NULL) {
		lua_close(LuaState);
	}

	Clear();
}

void script_state::SetLuaSession(lua_State *L)
{
	if(LuaState != NULL)
	{
		lua_close(LuaState);
	}
	LuaState = L;
	if(LuaState != NULL) {
		Langs |= SC_LUA;
	}
	else if(Langs & SC_LUA) {
		Langs &= ~SC_LUA;
	}
}

int script_state::OutputMeta(char *filename)
{
	FILE *fp = fopen(filename,"w");
	int i;

	if(fp == NULL)
	{
		return 0; 
	}

	if (FS_VERSION_BUILD == 0 && FS_VERSION_REVIS == 0) //-V547
	{
		fprintf(fp, "<html>\n<head>\n\t<title>Script Output - FSO v%i.%i (%s)</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, StateName);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>Script Output - FSO v%i.%i (%s)</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, StateName);
	}
	else if (FS_VERSION_REVIS == 0)
	{
		fprintf(fp, "<html>\n<head>\n\t<title>Script Output - FSO v%i.%i.%i (%s)</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>Script Output - FSO v%i.%i.%i (%s)</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
	}
	else
	{
		fprintf(fp, "<html>\n<head>\n\t<title>Script Output - FSO v%i.%i.%i.%i (%s)</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS, StateName);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>Script Output - FSO v%i.%i.%i.%i (%s)</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS, StateName);
	}
		
	//Scripting languages links
	fputs("<dl>", fp);

	//***Hooks
	fputs("<dt><h2>Conditional Hooks</h2></dt>", fp);
	fputs("<dd><dl>", fp);

	//Conditions
	fputs("<dt><b>Conditions</b></dt>", fp);
	for(i = 0; i < Num_script_conditions; i++)
	{
		fprintf(fp, "<dd>%s</dd>", Script_conditions[i].name);
	}

	//Actions
	fputs("<dt><b>Actions</b></dt>", fp);
	for(i = 0; i < Num_script_actions; i++)
	{
		fprintf(fp, "<dd>%s</dd>", Script_actions[i].name);
	}

	fputs("</dl></dd><br />", fp);

	//***Scripting langs
	fputs("<dt><h2>Scripting languages</h2></dt>", fp);
	if(Langs & SC_LUA) {
		fputs("<dd><a href=\"#Lua\">Lua</a></dd>", fp);
	}
	fputs("</dl>", fp);

	//Languages
	fputs("<dl>", fp);
	if(Langs & SC_LUA) {
		fputs("<dt><H2><a name=\"#Lua\">Lua</a></H2></dt>", fp);

		fputs("<dd>", fp);
		OutputLuaMeta(fp);
		fputs("</dd>", fp);
	}
	fputs("</dl></body></html>", fp);

	fclose(fp);

	return 1;
}

bool script_state::EvalString(char* string, char *format, void *rtn, char *debug_str)
{
	char lastchar = string[strlen(string)-1];

	if(string[0] == '{')
	{
		return false;
	}

	if(string[0] == '[' && lastchar != ']')
	{
		return false;
	}

	size_t s_bufSize = strlen(string) + 8;
	char *s = new char[ s_bufSize ];
	if(string[0] != '[')
	{
		if(rtn != NULL)
		{
			strcpy_s(s, s_bufSize, "return ");
			strcat_s(s, s_bufSize, string);
		}
		else
		{
			strcpy_s(s, s_bufSize, string);
		}
	}
	else
	{
		strcpy_s(s, s_bufSize, string+1);
		s[strlen(s)-1] = '\0';
	}

	//WMC - So we can pop everything we put on the stack
	int stack_start = lua_gettop(LuaState);

	//WMC - Push error handling function
	lua_pushcfunction(LuaState, ade_friendly_error);
	//Parse string
	int rval = luaL_loadbuffer(LuaState, s, strlen(s), debug_str);
	//We don't need s anymore.
	delete[] s;
	//Call function
	if(!rval)
	{
		if(lua_pcall(LuaState, 0, LUA_MULTRET, -2))
		{
			return false;
		}
		int stack_curr = lua_gettop(LuaState);

		//Only get args if we can put them someplace
		//WMC - We must have more than one more arg on the stack than stack_start:
		//(stack_start)
		//ade_friendly_error
		//(additional return values)
		if(rtn != NULL && stack_curr > (stack_start+1))
		{
			Ade_get_args_skip = stack_start+1;
			Ade_get_args_lfunction = true;
			ade_get_args(LuaState, format, rtn);
			Ade_get_args_skip = 0;
			Ade_get_args_lfunction = false;
		}

		//WMC - Pop anything leftover from the function from the stack
		stack_curr = lua_gettop(LuaState) - stack_start;
		for(; stack_curr > 0; stack_curr--) lua_pop(LuaState, 1);
	}
	else
	{
		//Or return error
		if(lua_isstring(GetLuaSession(), -1))
			LuaError(GetLuaSession());
		else
			LuaError(GetLuaSession(), "Error parsing %s", debug_str);
	}

	return true;
}

void script_state::ParseChunkSub(int *out_lang, int *out_index, char* debug_str)
{
	Assert(out_lang != NULL);
	Assert(out_index != NULL);
	Assert(debug_str != NULL);

	if(check_for_string("[["))
	{
		//Lua from file

		//Lua
		*out_lang = SC_LUA;

		char *filename = alloc_block("[[", "]]");

		//Load from file
		CFILE *cfp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SCRIPTS );
		if(cfp == NULL)
		{
			Warning(LOCATION, "Could not load lua script file '%s'", filename);
		}
		else
		{
			int len = cfilelength(cfp);

			char *raw_lua = (char*)vm_malloc(len+1);
			raw_lua[len] = '\0';

			cfread(raw_lua, len, 1, cfp);
			cfclose(cfp);

			//WMC - use filename instead of debug_str so that the filename
			//gets passed.
			if(!luaL_loadbuffer(GetLuaSession(), raw_lua, len, filename))
			{
				//Stick it in the registry
				*out_index = luaL_ref(GetLuaSession(), LUA_REGISTRYINDEX);
			}
			else
			{
				if(lua_isstring(GetLuaSession(), -1))
					LuaError(GetLuaSession());
				else
					LuaError(GetLuaSession(), "Error parsing %s", filename);
				*out_index = -1;
			}
			vm_free(raw_lua);
		}
		//dealloc
		//WMC - For some reason these cause crashes
		vm_free(filename);
	}
	else if(check_for_string("["))
	{
		//Lua string

		//Assume Lua
		*out_lang = SC_LUA;

		//Allocate raw script
		char* raw_lua = alloc_block("[", "]", 1);
		//WMC - minor hack to make sure that the last line gets
		//executed properly. In testing, I couldn't reproduce Nuke's
		//crash, so this is here just to be on the safe side.
		strcat(raw_lua, "\n");

		char *tmp_ptr = raw_lua;
		
		//Load it into a buffer & parse it
		//WMC - This is causing an access violation error. Sigh.
		if(!luaL_loadbuffer(GetLuaSession(), tmp_ptr, strlen(tmp_ptr), debug_str))
		{
			//Stick it in the registry
			*out_index = luaL_ref(GetLuaSession(), LUA_REGISTRYINDEX);
		}
		else
		{
			if(lua_isstring(GetLuaSession(), -1))
				LuaError(GetLuaSession());
			else
				LuaError(GetLuaSession(), "Error parsing %s", debug_str);
			*out_index = -1;
		}

		//free the mem
		//WMC - This makes debug go wonky.
		vm_free(raw_lua);
	}
	else
	{
		char buf[PARSE_BUF_SIZE];

		//Assume lua
		*out_lang = SC_LUA;

		strcpy_s(buf, "return ");

		//Stuff it
		stuff_string(buf+strlen(buf), F_RAW, sizeof(buf) - strlen(buf));

		//Add ending
		strcat_s(buf, "\n");

		int len = strlen(buf);

		//Load it into a buffer & parse it
		if(!luaL_loadbuffer(GetLuaSession(), buf, len, debug_str))
		{
			//Stick it in the registry
			*out_index = luaL_ref(GetLuaSession(), LUA_REGISTRYINDEX);
		}
		else
		{
			if(lua_isstring(GetLuaSession(), -1))
				LuaError(GetLuaSession());
			else
				LuaError(GetLuaSession(), "Error parsing %s", debug_str);
			*out_index = -1;
		}
	}
}

void script_state::ParseChunk(script_hook *dest, char *debug_str)
{
	static int total_parse_calls = 0;
	char debug_buf[128];

	total_parse_calls++;

	//DANGER! This code means the debug_str must be used only before parsing
	if(debug_str == NULL)
	{
		debug_str = debug_buf;
		sprintf(debug_str, "script_parse() count %d", total_parse_calls);
	}

	ParseChunkSub(&dest->h_language, &dest->h_index, debug_str);

	if(optional_string("+Override:"))
	{
		size_t bufSize = strlen(debug_str) + 10;
		char *debug_str_over = (char*)vm_malloc(bufSize);
		strcpy_s(debug_str_over, bufSize, debug_str);
		strcat_s(debug_str_over, bufSize, " override");
		ParseChunkSub(&dest->o_language, &dest->o_index, debug_str_over);
		vm_free(debug_str_over);
	}
}

int script_parse_condition()
{
	char buf[NAME_LENGTH];
	for(int i = 0; i < Num_script_conditions; i++)
	{
		sprintf(buf, "$%s:", Script_conditions[i].name);
		if(optional_string(buf))
			return Script_conditions[i].def;
	}

	return CHC_NONE;
}
flag_def_list* script_parse_action()
{
	char buf[NAME_LENGTH];
	for(int i = 0; i < Num_script_actions; i++)
	{
		sprintf(buf, "$%s:", Script_actions[i].name);
		if(optional_string(buf))
			return &Script_actions[i];
	}

	return NULL;
}
bool script_state::ParseCondition(const char *filename)
{
	ConditionedHook *chp = NULL;
	int condition;

	for(condition = script_parse_condition(); condition != CHC_NONE; condition = script_parse_condition())
	{
		script_condition sct;
		sct.condition_type = condition;

		switch(condition)
		{
			case CHC_STATE:
			case CHC_SHIPCLASS:
			case CHC_SHIPTYPE:
			case CHC_SHIP:
			case CHC_MISSION:
			case CHC_CAMPAIGN:
			case CHC_WEAPONCLASS:
			case CHC_OBJECTTYPE:
			case CHC_VERSION:
			case CHC_APPLICATION:
			default:
				stuff_string(sct.data.name, F_NAME, CONDITION_LENGTH);
				break;
		}

		if(chp == NULL)
		{
			ConditionalHooks.push_back(ConditionedHook());
			chp = &ConditionalHooks[ConditionalHooks.size()-1];
		}

		if(!chp->AddCondition(&sct))
		{
			Warning(LOCATION, "Could not add condition to conditional hook in file '%s'; you may have more than %d", filename, MAX_HOOK_CONDITIONS);
		}
	}

	if(chp == NULL)
	{
		return false;
	}

	flag_def_list *action;
	bool actions_added = false;
	for(action = script_parse_action(); action != NULL; action = script_parse_action())
	{
		script_action sat;
		sat.action_type = action->def;

		//WMC - build error string
		char *buf = (char *)vm_malloc(strlen(filename) + strlen(action->name) + 4);
		sprintf(buf, "%s - %s", filename, action->name);

		ParseChunk(&sat.hook, buf);
		
		//Free error string
		vm_free(buf);

		//Add the action
		if(chp->AddAction(&sat))
			actions_added = true;
	}

	if(!actions_added)
	{
		Warning(LOCATION, "No actions specified for conditional hook in file '%s'", filename);
		ConditionalHooks.pop_back();
		return false;
	}

	return true;
}

//*************************CLASS: script_state*************************
bool script_state::IsOverride(script_hook &hd)
{
	if(hd.h_index < 0)
		return false;

	bool b=false;
	RunBytecodeSub(hd.o_language, hd.o_index, 'b', &b);

	return b;
}

void scripting_state_init()
{
	// nothing to do here
	if (scripting_state_inited)
		return;

	gr_set_clear_color(0, 0, 0);

	scripting_state_inited = 1;
}

void scripting_state_close()
{
	if (!scripting_state_inited)
		return;

	game_flush();

	scripting_state_inited = 0;
}

void scripting_state_do_frame(float frametime)
{
	// just incase something is wrong
	if (!scripting_state_inited)
		return;

	gr_reset_clip();
	gr_clear();
	gr_flip();

	// process keys
	int k = game_check_key() & ~KEY_DEBUGGED;	

	switch (k)
	{
		case KEY_ESC:
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			return;
	}
}
