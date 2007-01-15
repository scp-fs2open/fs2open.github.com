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
	{"State",		CHC_STATE},
	{"Campaign",	CHC_CAMPAIGN},
	{"Mission",		CHC_MISSION},
	{"Object Type", CHC_OBJECTTYPE},
	{"Ship",		CHC_SHIP},
	{"Ship class",	CHC_SHIPCLASS},
	{"Ship type",	CHC_SHIPTYPE},
	{"Weapon class",CHC_WEAPONCLASS},
	{"KeyPress",	CHC_KEYPRESS},
};

int Num_script_conditions = sizeof(Script_conditions)/sizeof(flag_def_list);

flag_def_list Script_actions[] = 
{
	{"On Frame",				CHA_ONFRAME},
	{"On HUD Draw",				CHA_HUDDRAW},
	{"On Ship Collision",		CHA_COLLIDESHIP},
	{"On Weapon Collision",		CHA_COLLIDEWEAPON},
	{"On Debris Collision",		CHA_COLLIDEDEBRIS},
	{"On Asteroid Collision",	CHA_COLLIDEASTEROID},
	{"On Object Render",		CHA_OBJECTRENDER},
	{"On Warpin",				CHA_WARPIN},
	{"On Warpout",				CHA_WARPOUT},
	{"On Death",				CHA_DEATH},
};

int Num_script_actions = sizeof(Script_actions)/sizeof(flag_def_list);

//*************************Scripting init and handling*************************

void script_parse_table(char *filename)
{
	script_state *st = &Script_system;
	int rval;
	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'.  Code = %i.\n", filename, rval));
		return;
	}
	else
	{	
		read_file_text(filename);
		reset_parse();
	}

	if(optional_string("#Global Hooks"))
	{
		int num = 42;
		Script_system.SetHookVar("Version", 'i', &num);
		if(optional_string("$Global:")) {
			Script_globalhook = st->ParseChunk("Global");
		}

		if(optional_string("$Splash:")) {
			Script_splashhook = st->ParseChunk("Splash");
		}

		if(optional_string("$GameInit:")) {
			Script_gameinithook = st->ParseChunk("GameInit");
		}

		if(optional_string("$Simulation:")) {
			Script_simulationhook = st->ParseChunk("Simulation");
		}

		if(optional_string("$HUD:")) {
			Script_hudhook = st->ParseChunk("HUD");
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
}

//Initializes the (global) scripting system, as well as any subsystems.
//script_close is handled by destructors
void script_init (void)
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

//*************************CLASS: ConditionedScript*************************
extern char Game_current_mission_filename[];
bool ConditionedHook::AddCondition(script_condition sc)
{
	for(int i = 0; i < MAX_HOOK_CONDITIONS; i++)
	{
		if(Conditions[i].condition_type == CHC_NONE)
		{
			Conditions[i] = sc;
			return true;
		}
	}

	return false;
}

bool ConditionedHook::AddAction(script_action sa)
{
	if(!sa.hook.IsValid())
		return false;

	Actions.push_back(sa);

	return true;
}

bool ConditionedHook::ConditionsValid(int action, object *objp)
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
				if(objp == NULL || objp->type != OBJ_WEAPON)
					return false;
				if(stricmp(Weapon_info[Weapons[objp->instance].weapon_info_index].name, scp->data.name))
					return false;
				break;
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
					if(Current_key_down == translate_key_to_index(scp->data.name))
						return false;
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
	script_action *sap;
	for(uint i = 0; i < Actions.size(); i++)
	{
		sap = &Actions[i];
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
	script_action *sap;
	for(uint i = 0; i < Actions.size(); i++)
	{
		sap = &Actions[i];
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
#ifndef USE_PYTHON
#pragma message("NOTE: Python is not compiled in")
#endif
 
//WMC - defined in parse/lua.h
int ade_set_object_with_breed(lua_State *L, int obj_idx);
void script_state::SetHookObject(char *name, int obj_idx)
{
	ade_set_object_with_breed(LuaState, obj_idx);
	SetHookVar(name, 'o');
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
		lua_pushliteral(LuaState, "hv");
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
					lua_pushstring(LuaState, name);
					//ERRORS? LOOK HERE!!!
					//--------------------
					//WMC - Now THIS has to be the nastiest hack I've made
					//Basically, I tell it to copy over enough stack
					//for a ade_odata object. If you pass
					//_anything_ larger as a stack object, this will not work.
					//You'll get memory corruption
					if(data == NULL)
						lua_pushvalue(LuaState, data_ldx);
					else
						ade_set_args(LuaState, fmt, *(ade_odata*)data);
					//--------------------
					//WMC - This was a separate function
					//lua_set_arg(LuaState, format, data);
					//WMC - switch to the lua library
					//lua_setglobal(LuaState, name);
					lua_rawset(LuaState, amt_ldx);
				}
				lua_pop(LuaState, 1); //amt
			}
			lua_pop(LuaState, 1);	//Metatable
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to set hook variable '%s' - get a coder", name);
		}
		lua_pop(LuaState, 1);	//Library

		if(data_ldx)
			lua_pop(LuaState, 1);
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
		lua_pushliteral(LuaState, "hv");
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
					lua_pushstring(LuaState, name);
					lua_rawget(LuaState, amt_ldx);
					if(!lua_isnil(LuaState, -1))
					{
						if(data != NULL) {
							ade_get_args(LuaState, fmt, data);
						}
						got_global = true;
					}
					lua_pushstring(LuaState, name);
				}
				lua_pop(LuaState, 1); //amt
			}
			lua_pop(LuaState, 1);	//Metatable
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to get hook variable '%' - get a coder", name);
		}
		lua_pop(LuaState, 1);	//Library
	}

	return got_global;
}

void script_state::RemHookVar(char *name)
{
	if(LuaState != NULL)
	{
		//WMC - Quick and clean. :)
		//WMC - *sigh* nostalgia
		//Get ScriptVar table
		lua_pushliteral(LuaState, "hv");
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
					lua_pushstring(LuaState, name);
					lua_pushnil(LuaState);
					lua_rawset(LuaState, amt_ldx);
				}
				lua_pop(LuaState, 1); //amt
			}
			lua_pop(LuaState, 1);	//Metatable
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to remove hook variable '%s' - get a coder", name);
		}
		lua_pop(LuaState, 1);	//Library
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
		strcpy(id.fname, name);
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
			int args_start=0;
			if(data != NULL) {
				args_start = lua_gettop(LuaState);
			}
			lua_pushcfunction(GetLuaSession(), ade_friendly_error);
			lua_getref(GetLuaSession(), in_idx);
			if(lua_isfunction(GetLuaSession(), -1))
			{
				if(lua_pcall(GetLuaSession(), 0, format!='\0' ? 1 : 0, -2) != 0)
				{
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
				args_start = lua_gettop(LuaState) - args_start;
				for(; args_start > 0; args_start--) lua_pop(LuaState, 1);
			}
			else
			{
				Warning(LOCATION, "RunBytecodeSub tried to call a non-function value!");
			}
		}
		else if(in_lang == SC_PYTHON)
		{
#ifdef USE_PYTHON
			PyObject *chk = PyBytecodeLib.Get(in_idx);
			if(chk != NULL)
			{
				PyEval_EvalCode(PyBytecodeLib.Get(in_idx), GetPyGlobals(), GetPyLocals());
				return 1;
			}
#endif
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

int script_state::RunCondition(int action, char format, void *data, object *objp)
{
	ConditionedHook *chp = NULL;
	int num = 0;
	for(uint i= 0; i < ConditionalHooks.size(); i++)
	{
		chp = &ConditionalHooks[i];
		if(chp->ConditionsValid(action, objp))
		{
			chp->Run(this, action, format, data);
			num++;
		}
	}
	return num;
}

bool script_state::IsConditionOverride(int action, object *objp)
{
	ConditionedHook *chp = NULL;
	//bool b = false;
	for(uint i= 0; i < ConditionalHooks.size(); i++)
	{
		chp = &ConditionalHooks[i];
		if(chp->ConditionsValid(action, objp))
		{
			if(chp->IsOverride(this, action))
				return true;
		}
	}
	return false;
}

void script_state::Clear()
{
	StateName[0] = '\0';
	Langs = 0;

	//Don't close this yet
	LuaState = NULL;
	LuaLibs = NULL;
#ifdef USE_PYTHON
	Py_XDECREF(PyGlb);
	PyGlb = NULL;
	Py_XDECREF(PyLoc);
	PyLoc = NULL;
#endif
}

script_state::script_state(char *name)
{
	strncpy(StateName, name, sizeof(StateName)-1);

	Langs = 0;

	LuaState = NULL;
	LuaLibs = NULL;

	PyGlb = NULL;
	PyLoc = NULL;
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

void script_state::SetPySession(PyObject *loc, PyObject *glb)
{
#ifdef USE_PYTHON
	//set variables
	if(PyGlb != NULL)
	{
		Py_XDECREF(PyGlb);
		PyGlb = glb;
	}

	if(PyLoc != NULL)
	{
		Py_XDECREF(PyLoc);
		PyLoc = loc;
	}

	//Add or remove python note
	if(PyGlb != NULL || PyLoc != NULL)
	{
		Langs |= SC_PYTHON;
	}
	else if(Langs & SC_PYTHON)
	{
		Langs &= ~SC_PYTHON;
	}
#endif
}

int script_state::OutputMeta(char *filename)
{
	FILE *fp = fopen(filename,"w");
	int i;

	if(fp == NULL)
	{
		return 0; 
	}

	fprintf(fp, "<html>\n<head>\n\t<title>Script output: %d.%d.%d (%s)</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
	fputs("<body>", fp);
	fprintf(fp,"\t<h1>Script Output - Build %d.%d.%d (%s)</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
	
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
	if(Langs & SC_PYTHON) {
		fputs("<dd><a href=\"#Python\">Python</a></dd>", fp);
	}
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
	/*
	if(Langs & SC_PYTHON) {
		OutputPyMeta(fp);
	}*/
	fputs("</dl></body></html>", fp);

	fclose(fp);

	return 1;
}

bool script_state::EvalString(char* string, char *format, void *rtn, char *debug_str)
{
	char *lcp = &string[strlen(string)-1];
	char lastchar = *lcp;

	if(string[0] == '{')
	{
#ifdef USE_PYTHON
		if(lastchar == '}')
		{
			*lcp = '\0';
		}
		else
		{
			return false;
		}

		//WMC - Whoever comes next to Python...needs to fix this.
		Py_RunString(string, Py_file_input, PyGlb, PyLoc);

		*lcp = lastchar;
		return true;
#else
		return false;
#endif
	}

	if(string[0] == '[' && lastchar != ']')
	{
		return false;
	}

	char *s = new char[strlen(string) + 8];
	if(string[0] != '[' && rtn != NULL)
	{
		strcpy(s, "return ");
		strcat(s, string);
	}
	else if(string[0] != '[')
	{
		strcpy(s, string);
	}
	else if(string[0] == '[')
	{
		strcpy(s, string+1);
	}

	s[strlen(s)-1] = '\0';

	//WMC - So we can pop extra return values
	int args_start = lua_gettop(LuaState);

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

		//Only get args if we can put them someplace
		if(rtn != NULL)
		{
			ade_get_args(LuaState, format, *(ade_odata*)rtn);
		}

		//WMC - Pop anything leftover from the function from the stack
		args_start = lua_gettop(LuaState) - args_start;
		for(; args_start > 0; args_start--) lua_pop(LuaState, 1);

		if(lastchar == ']')
			*lcp = lastchar;
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

			if(!luaL_loadbuffer(GetLuaSession(), raw_lua, len, debug_str))
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
			vm_free(raw_lua);
		}
		//dealloc
		//WMC - For some reason these cause crashes
		//vm_free(filename);
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
		
		//Load it into a buffer & parse it
		//WMC - This is causing an access violation error. Sigh.
		if(!luaL_loadbuffer(GetLuaSession(), raw_lua, strlen(raw_lua), debug_str))
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
		//vm_free(raw_lua);
	}
	else if(check_for_string("{"))
	{
		//Python string

		//Assume python
		*out_lang = SC_PYTHON;

		//Get the block
		char* raw_python = alloc_block("{","}");
#ifdef USE_PYTHON
		//Add it to the lib
		*out_index = PyBytecodeLib.Add(PyBytecode(Py_CompileString(raw_python, debug_str, Py_file_input)));
#endif
		vm_free(raw_python);
	}
	else
	{
		char buf[PARSE_BUF_SIZE];

		//Assume lua
		*out_lang = SC_LUA;

		strcpy(buf, "return ");

		//Stuff it
		stuff_string(buf+strlen(buf), F_RAW, sizeof(buf) - strlen(buf));

		//Add ending
		strcat(buf, "\n");

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

script_hook script_state::ParseChunk(char* debug_str)
{
	static int total_parse_calls = 0;
	char debug_buf[128];
	script_hook rval;

	total_parse_calls++;

	//DANGER! This code means the debug_str must be used only before parsing
	if(debug_str == NULL)
	{
		debug_str = debug_buf;
		sprintf(debug_str, "script_parse() count %d", total_parse_calls);
	}

	ParseChunkSub(&rval.h_language, &rval.h_index, debug_str);

	if(optional_string("+Override:"))
	{
		char *debug_str_over = (char*)vm_malloc(strlen(debug_str) + 10);
		strcpy(debug_str_over, debug_str);
		strcat(debug_str_over, " override");
		ParseChunkSub(&rval.o_language, &rval.o_index, debug_str_over);
		vm_free(debug_str_over);
	}

	return rval;
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
bool script_state::ParseCondition(char *filename)
{
	ConditionedHook *chp = NULL;

	script_condition sct;
	int condition;
	for(condition = script_parse_condition(); condition != CHC_NONE; condition = script_parse_condition())
	{
		//Clear it
		sct = script_condition();
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
			default:
				stuff_string(sct.data.name, F_NAME, NAME_LENGTH);
				break;
		}

		if(chp == NULL)
		{
			ConditionalHooks.push_back(ConditionedHook());
			chp = &ConditionalHooks[ConditionalHooks.size()-1];
		}

		if(!chp->AddCondition(sct))
		{
			Warning(LOCATION, "Could not add condition to conditional hook in file '%s'; you may have more than %d", filename, MAX_HOOK_CONDITIONS);
		}
	}

	if(chp == NULL)
	{
		return false;
	}

	script_action sat;
	flag_def_list *action;
	bool actions_added = false;
	for(action = script_parse_action(); action != NULL; action = script_parse_action())
	{
		sat = script_action();
		sat.action_type = action->def;

		//WMC - build error string
		char *buf = (char *)vm_malloc(strlen(filename) + strlen(action->name) + 4);
		sprintf(buf, "%s - %s", filename, action->name);

		sat.hook = ParseChunk(buf);
		
		//Free error string
		vm_free(buf);

		//Add the action
		if(chp->AddAction(sat))
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

//*************************CLASS: script_hook*************************
bool script_state::IsOverride(script_hook &hd)
{
	if(hd.h_index < 0)
		return false;

	bool b=false;
	RunBytecodeSub(hd.o_language, hd.o_index, 'b', &b);

	return b;
}
