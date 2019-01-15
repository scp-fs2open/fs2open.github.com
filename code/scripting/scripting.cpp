#include <cstdio>
#include <cstdarg>

#include "bmpman/bmpman.h"
#include "controlconfig/controlsconfig.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/systemvars.h"
#include "globalincs/version.h"
#include "hud/hud.h"
#include "io/key.h"
#include "mission/missioncampaign.h"
#include "parse/parselo.h"
#include "scripting/scripting.h"
#include "scripting/ade_args.h"
#include "ship/ship.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"
#include "ade.h"

using namespace scripting;

//tehe. Declare the main event
script_state Script_system("FS2_Open Scripting");
bool Output_scripting_meta = false;

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
	{"On Beam Collision",		CHA_COLLIDEBEAM,	0},
	{"On Message Received",		CHA_MSGRECEIVED,	0},
    {"On HUD Message Received", CHA_HUDMSGRECEIVED, 0},
	{ "On Afterburner Engage",	CHA_AFTERBURNSTART, 0 },
	{ "On Afterburner Stop",	CHA_AFTERBURNEND,	0 },
	{ "On Beam Fire",			CHA_BEAMFIRE,		0 },
	{ "On Simulation",			CHA_SIMULATION,		0 },
	{ "On Load Screen",			CHA_LOADSCREEN,		0 },
	{ "On Campaign Mission Accept", 	CHA_CMISSIONACCEPT,	0 },
    { "On Ship Depart",			CHA_ONSHIPDEPART,	0 },
	{ "On Weapon Created",		CHA_ONWEAPONCREATED, 0},
};

int Num_script_actions = sizeof(Script_actions)/sizeof(flag_def_list);
int scripting_state_inited = 0;

//*************************Scripting init and handling*************************

// ditto
bool script_hook_valid(script_hook *hook)
{
	return hook->hook_function.function.isValid();
}

void script_parse_table(const char *filename)
{
	script_state *st = &Script_system;
	
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		if (optional_string("#Global Hooks"))
		{
			//int num = 42;
			//Script_system.SetHookVar("Version", 'i', &num);
			if (optional_string("$Global:")) {
				st->ParseGlobalChunk(CHA_ONFRAME, "Global");
			}

			if (optional_string("$Splash:")) {
				st->ParseGlobalChunk(CHA_SPLASHSCREEN, "Splash");
			}

			if (optional_string("$GameInit:")) {
				st->ParseGlobalChunk(CHA_GAMEINIT, "GameInit");
			}

			if (optional_string("$Simulation:")) {
				st->ParseGlobalChunk(CHA_SIMULATION, "Simulation");
			}

			if (optional_string("$HUD:")) {
				st->ParseGlobalChunk(CHA_HUDDRAW, "HUD");
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
		if (optional_string("#Conditional Hooks"))
		{
			while (st->ParseCondition(filename));
			required_string("#End");
		}

		// add tbl/tbm to multiplayer validation list
		extern void fs2netd_add_table_validation(const char *tblname);
		fs2netd_add_table_validation(filename);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

//Initializes the (global) scripting system, as well as any subsystems.
//script_close is handled by destructors
void script_init()
{
	mprintf(("SCRIPTING: Beginning initialization sequence...\n"));

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
				if(stricmp(GS_state_text[gameseq_get_state(0)], scp->data.name) != 0)
					return false;
				break;
			case CHC_SHIPTYPE:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				sip = &Ship_info[Ships[objp->instance].ship_info_index];
				if(sip->class_type < 0)
					return false;
				if(stricmp(Ship_types[sip->class_type].name, scp->data.name) != 0)
					return false;
				break;
			case CHC_SHIPCLASS:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				if(stricmp(Ship_info[Ships[objp->instance].ship_info_index].name, scp->data.name) != 0)
					return false;
				break;
			case CHC_SHIP:
				if(objp == NULL || objp->type != OBJ_SHIP)
					return false;
				if(stricmp(Ships[objp->instance].ship_name, scp->data.name) != 0)
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
					if(strnicmp(scp->data.name, Mission_filename, len) != 0)
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
					if(strnicmp(scp->data.name, Mission_filename, len) != 0)
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
					} else if (action == CHA_ONWEAPONCREATED) {
						if (objp == NULL || objp->type != OBJ_WEAPON)
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

								if ((shipp->flags[Ship::Ship_Flags::Primary_linked]) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink]))
									return false;
								
								break;
							case CHA_ONWPDESELECTED:
								primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
								prev_primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].name, scp->data.name) == 0;
								secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								prev_secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.previous_secondary_bank]].name, scp->data.name) == 0;

								if ((shipp->flags[Ship::Ship_Flags::Primary_linked]) && prev_primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink]))
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

								if ((shipp->flags[Ship::Ship_Flags::Primary_linked]) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink]))
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
								if (stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) != 0)
									return false;
								break;
							}
							case CHA_SECONDARYFIRE: {
								if (stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) != 0)
									return false;
								break;
							}
							case CHA_BEAMFIRE: {
								if (!(stricmp(Weapon_info[more_data].name, scp->data.name) == 0))
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
				if(stricmp(Object_type_names[objp->type], scp->data.name) != 0)
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
					if(stricmp(textify_scancode(Current_key_down), scp->data.name) != 0)
						return false;
					break;
				}
			case CHC_ACTION:
				{
					if(gameseq_get_depth() < 0)
						return false;

					int action_index = more_data;

					if (action_index <= 0 || stricmp(scp->data.name, Control_config[action_index].text) != 0)
						return false;
					break;
				}
			case CHC_VERSION:
				{
					// Goober5000: I'm going to assume scripting doesn't care about SVN revision
					char buf[32];
					sprintf(buf, "%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
					if(stricmp(buf, scp->data.name) != 0)
					{
						//In case some people are lazy and say "3.7" instead of "3.7.0" or something
						if(FS_VERSION_BUILD == 0)
						{
							sprintf(buf, "%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR);
							if(stricmp(buf, scp->data.name) != 0)
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
						if(stricmp("FRED2_Open", scp->data.name) != 0 && stricmp("FRED2Open", scp->data.name) != 0 && stricmp("FRED 2", scp->data.name) != 0 && stricmp("FRED", scp->data.name) != 0)
							return false;
					}
					else
					{
						if(stricmp("FS2_Open", scp->data.name) != 0 && stricmp("FS2Open", scp->data.name) != 0 && stricmp("Freespace 2", scp->data.name) != 0 && stricmp("Freespace", scp->data.name) != 0)
							return false;
					}
				}
			default:
				break;
		}
	}

	return true;
}

bool ConditionedHook::IsOverride(script_state* sys, int action)
{
	Assert(sys != NULL);
	// bool b = false;

	//Do the actions
	for(SCP_vector<script_action>::iterator sap = Actions.begin(); sap != Actions.end(); ++sap)
	{
		if (sap->action_type == action) {
			if (sys->IsOverride(sap->hook))
				return true;
		}
	}

	return false;
}

bool ConditionedHook::Run(class script_state* sys, int action)
{
	Assert(sys != NULL);

	// Do the actions
	for (auto & Action : Actions) {
		if (Action.action_type == action) {
			sys->RunBytecode(Action.hook.hook_function);
		}
	}

	return true;
}

//*************************CLASS: script_state*************************
//Most of the icky stuff is here. Lots of #ifdefs

//WMC - defined in parse/scripting.h
void script_state::SetHookObject(const char *name, object *objp)
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

void script_state::RemHookVar(const char *name)
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

int script_state::LoadBm(const char* name)
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
		bm_release(ScriptImages[i].handle);
	}

	ScriptImages.clear();
}

int script_state::RunCondition(int action, object* objp, int more_data)
{
	int num = 0;
	for(SCP_vector<ConditionedHook>::iterator chp = ConditionalHooks.begin(); chp != ConditionalHooks.end(); ++chp) 
	{
		if(chp->ConditionsValid(action, objp, more_data))
		{
			chp->Run(this, action);
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
	// Free all lua value references
	ConditionalHooks.clear();

	if(LuaState != NULL) {
		lua_close(LuaState);
	}

	StateName[0] = '\0';
	Langs = 0;

	//Don't close this yet
	LuaState = NULL;
	LuaLibs = NULL;
}

script_state::script_state(const char *name)
{
	strncpy(StateName, name, sizeof(StateName)-1);

	Langs = 0;

	LuaState = NULL;
	LuaLibs = NULL;
}

script_state::~script_state()
{
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

int script_state::OutputMeta(const char *filename)
{
	FILE *fp = fopen(filename,"w");
	int i;

	if(fp == NULL)
	{
		return 0; 
	}

	fprintf(fp, "<html>\n<head>\n\t<title>Script Output - FSO v%s (%s)</title>\n</head>\n", FS_VERSION_FULL, StateName);
	fputs("<body>", fp);
	fprintf(fp,"\t<h1>Script Output - FSO v%s (%s)</h1>\n", FS_VERSION_FULL, StateName);
		
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

void script_state::ParseChunkSub(script_function& script_func, const char* debug_str)
{
	using namespace luacpp;

	Assert(debug_str != NULL);

	//Lua
	script_func.language = SC_LUA;

	std::string source;
	std::string function_name(debug_str);

	if(check_for_string("[["))
	{
		//Lua from file

		char *filename = alloc_block("[[", "]]");

		//Load from file
		CFILE *cfp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SCRIPTS );

		//WMC - use filename instead of debug_str so that the filename gets passed.
		function_name = filename;
		vm_free(filename);

		if(cfp == NULL)
		{
			Warning(LOCATION, "Could not load lua script file '%s'", function_name.c_str());
			return;
		}
		else
		{
			int len = cfilelength(cfp);

			source.resize((size_t) len);
			cfread(&source[0], len, 1, cfp);
			cfclose(cfp);
		}
	}
	else if(check_for_string("["))
	{
		//Lua string

		// Determine the current line in the file so that the Lua source can begin at the same line as in the table
		// This will make sure that the line in the error message matches the line number in the table.
		auto line = get_line_num();

		//Allocate raw script
		char* raw_lua = alloc_block("[", "]", 1);
		//WMC - minor hack to make sure that the last line gets
		//executed properly. In testing, I couldn't reproduce Nuke's
		//crash, so this is here just to be on the safe side.
		strcat(raw_lua, "\n");

		for (auto i = 0; i <= line; ++i) {
			source += "\n";
		}
		source += raw_lua;
		vm_free(raw_lua);
	}
	else
	{
		std::string buf;

		//Stuff it
		stuff_string(buf, F_RAW);

		source = "return ";
		source += buf;
	}

	try {
		auto function = LuaFunction::createFromCode(LuaState, source, function_name);
		function.setErrorFunction(LuaFunction::createFromCFunction(LuaState, ade_friendly_error));

		script_func.function = function;
	} catch (const LuaException& e) {
		LuaError(GetLuaSession(), "%s", e.what());
	}
}

void script_state::ParseChunk(script_hook *dest, const char *debug_str)
{
	static int total_parse_calls = 0;
	char debug_buf[128];

	total_parse_calls++;

	//DANGER! This code means the debug_str must be used only before parsing
	if(debug_str == NULL)
	{
		sprintf(debug_buf, "script_parse() count %d", total_parse_calls);
		debug_str = debug_buf;
	}

	ParseChunkSub(dest->hook_function, debug_str);

	if(optional_string("+Override:"))
	{
		size_t bufSize = strlen(debug_str) + 10;
		char *debug_str_over = (char*)vm_malloc(bufSize);
		strcpy_s(debug_str_over, bufSize, debug_str);
		strcat_s(debug_str_over, bufSize, " override");
		ParseChunkSub(dest->override_function, debug_str_over);
		vm_free(debug_str_over);
	}
}

bool script_state::EvalString(const char* string, const char* debug_str)
{
	using namespace luacpp;

	size_t string_size = strlen(string);
	char lastchar      = string[string_size - 1];

	if (string[0] == '{') {
		return false;
	}

	if (string[0] == '[' && lastchar != ']') {
		return false;
	}

	size_t s_bufSize = string_size + 8;
	std::string s;
	s.reserve(s_bufSize);
	if (string[0] != '[') {
		s += string;
	} else {
		s.assign(string + 1, string + string_size);
	}

	SCP_string debug_name;
	if (debug_str == nullptr) {
		debug_name = "String: ";
		debug_name += s;
	} else {
		debug_name = debug_str;
	}

	try {
		auto function = LuaFunction::createFromCode(LuaState, s, debug_name);
		function.setErrorFunction(LuaFunction::createFromCFunction(LuaState, scripting::ade_friendly_error));

		try {
			function.call();
		} catch (const LuaException&) {
			return false;
		}
	} catch (const LuaException& e) {
		LuaError(GetLuaSession(), "%s", e.what());

		return false;
	}

	return true;
}
int script_state::RunBytecode(script_function& hd)
{
	using namespace luacpp;

	if (!hd.function.isValid()) {
		return 1;
	}

	GR_DEBUG_SCOPE("Lua code");

	try {
		hd.function.call();
	} catch (const LuaException&) {
		return 0;
	}

	return 1;
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
void script_state::ParseGlobalChunk(int hookType, const char* debug_str) {
	ConditionedHook hook;

	script_action sat;
	sat.action_type = hookType;

	ParseChunk(&sat.hook, debug_str);

	//Add the action
	hook.AddAction(&sat);

	ConditionalHooks.push_back(hook);
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
	if(!hd.hook_function.function.isValid())
		return false;

	bool b=false;
	RunBytecode(hd.override_function, 'b', &b);

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

void scripting_state_do_frame(float  /*frametime*/)
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
