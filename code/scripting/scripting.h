#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "scripting/ade_args.h"
#include "scripting/lua/LuaFunction.h"

#include <cstdio>

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

struct image_desc
{
	char fname[MAX_FILENAME_LEN];
	int handle;
};

struct script_function {
	int language = 0;
	luacpp::LuaFunction function;
};

//-WMC
struct script_hook
{
	//Override
	script_function override_function;

	//Actual hook
	script_function hook_function;
};

extern bool script_hook_valid(script_hook *hook);

//**********Main Conditional Hook stuff

#define MAX_HOOK_CONDITIONS	8

//Conditionals
#define CHC_NONE			-1
#define CHC_MISSION			0
#define CHC_SHIP			1
#define CHC_SHIPCLASS		2
#define CHC_SHIPTYPE		3
#define CHC_STATE			4
#define CHC_CAMPAIGN		5
#define CHC_WEAPONCLASS		6
#define CHC_OBJECTTYPE		7
#define CHC_KEYPRESS		8
#define CHC_ACTION			9
#define CHC_VERSION			10
#define CHC_APPLICATION		11

//Actions
#define CHA_NONE			-1
#define CHA_WARPOUT			0
#define CHA_WARPIN			1
#define CHA_DEATH			2
#define CHA_ONFRAME			3
#define CHA_COLLIDESHIP		4
#define CHA_COLLIDEWEAPON	5
#define CHA_COLLIDEDEBRIS	6
#define CHA_COLLIDEASTEROID	7
#define CHA_HUDDRAW			8
#define CHA_OBJECTRENDER	9
#define CHA_SPLASHSCREEN	10
#define CHA_GAMEINIT		11
#define CHA_MISSIONSTART	12
#define CHA_MISSIONEND		13
#define CHA_MOUSEMOVED		14
#define CHA_MOUSEPRESSED	15
#define CHA_MOUSERELEASED	16
#define CHA_KEYPRESSED		17
#define CHA_KEYRELEASED		18
#define CHA_ONSTATESTART	19
#define CHA_ONSTATEEND		20
#define CHA_ONWEAPONDELETE	21
#define CHA_ONWPEQUIPPED	22
#define CHA_ONWPFIRED		23
#define CHA_ONWPSELECTED	24
#define CHA_ONWPDESELECTED	25
#define CHA_GAMEPLAYSTART	26
#define CHA_ONTURRETFIRED	27
#define CHA_PRIMARYFIRE		28
#define CHA_SECONDARYFIRE	29
#define CHA_ONSHIPARRIVE	30
#define CHA_COLLIDEBEAM		31
#define CHA_ONACTION		32
#define CHA_ONACTIONSTOPPED	33
#define CHA_MSGRECEIVED		34
#define CHA_HUDMSGRECEIVED	35
#define CHA_AFTERBURNSTART	36
#define CHA_AFTERBURNEND    37
#define CHA_BEAMFIRE        38
#define CHA_SIMULATION      39
#define CHA_LOADSCREEN      40
#define CHA_CMISSIONACCEPT  41
#define CHA_ONSHIPDEPART	42

// management stuff
void scripting_state_init();
void scripting_state_close();
void scripting_state_do_frame(float frametime);

class script_condition
{
public:
	int condition_type;
	union
	{
		char name[CONDITION_LENGTH];
	} data;

	script_condition()
		: condition_type(CHC_NONE)
	{
		memset(data.name, 0, sizeof(data.name));
	}
};

class script_action
{
public:
	int action_type;
	script_hook hook;

	script_action()
		: action_type(CHA_NONE)
	{
	}
};

class ConditionedHook
{
private:
	SCP_vector<script_action> Actions;
	script_condition Conditions[MAX_HOOK_CONDITIONS];
public:
	bool AddCondition(script_condition *sc);
	bool AddAction(script_action *sa);

	bool ConditionsValid(int action, class object *objp=NULL, int more_data = 0);
	bool IsOverride(class script_state *sys, int action);
	bool Run(class script_state* sys, int action);
};

//**********Main script_state function
class script_state
{
private:
	char StateName[32];

	int Langs;
	struct lua_State *LuaState;
	const struct script_lua_lib_list *LuaLibs;

	//Utility variables
	SCP_vector<image_desc> ScriptImages;
	SCP_vector<ConditionedHook> ConditionalHooks;

private:

	void ParseChunkSub(script_function& out_func, const char* debug_str=NULL);

	void SetLuaSession(struct lua_State *L);

	void OutputLuaMeta(FILE *fp);
	
	//Lua private helper functions
	bool OpenHookVarTable();
	bool CloseHookVarTable();

	//Internal Lua helper functions
	void EndLuaFrame();

public:
	//***Init/Deinit
	script_state(const char *name);

	script_state(const script_state&) = delete;
	script_state& operator=(script_state &i) = delete;

	~script_state();

	/**
	 * @brief Resets the lua state and frees all resources
	 */
	void Clear();

	//***Internal scripting stuff
	int LoadBm(const char* name);
	void UnloadImages();

	lua_State *GetLuaSession(){return LuaState;}

	//***Init functions for langs
	int CreateLuaState();

	//***Get data
	int OutputMeta(const char *filename);

	//***Moves data
	//void MoveData(script_state &in);

	template<typename T>
	void SetHookVar(const char *name, char format, T value);
	void SetHookObject(const char *name, object *objp);
	void SetHookObjects(int num, ...);
	void RemHookVar(const char *name);
	void RemHookVars(unsigned int num, ...);

	//***Hook creation functions
	template <typename T>
	bool EvalStringWithReturn(const char* string, const char* format = nullptr, T* rtn = NULL,
	                          const char* debug_str = nullptr);
	bool EvalString(const char* string, const char* debug_str = nullptr);
	void ParseChunk(script_hook *dest, const char* debug_str=NULL);
	void ParseGlobalChunk(int hookType, const char* debug_str=NULL);
	bool ParseCondition(const char *filename="<Unknown>");

	//***Hook running functions
	template <typename T>
	int RunBytecode(script_function& hd, char format = '\0', T* data = nullptr);
	int RunBytecode(script_function& hd);
	bool IsOverride(script_hook &hd);
	int RunCondition(int condition, object* objp = nullptr, int more_data = 0);
	bool IsConditionOverride(int action, object *objp=NULL);

	//*****Other functions
	void EndFrame();
};

template<typename T>
void script_state::SetHookVar(const char *name, char format, T value)
{
	if(format == '\0')
		return;

	if(LuaState != nullptr)
	{
		char fmt[2] = {format, '\0'};
		//Get ScriptVar table
		if(this->OpenHookVarTable())
		{
			int amt_ldx = lua_gettop(LuaState);
			lua_pushstring(LuaState, name);
			scripting::ade_set_args(LuaState, fmt, value);
			//--------------------
			//WMC - This was a separate function
			//lua_set_arg(LuaState, format, data);
			//WMC - switch to the scripting library
			//lua_setglobal(LuaState, name);
			lua_rawset(LuaState, amt_ldx);

			//Close hook var table
			this->CloseHookVarTable();
		}
		else
		{
			LuaError(LuaState, "Could not get HookVariable library to set hook variable '%s'", name);
		}
	}
}

template <typename T>
bool script_state::EvalStringWithReturn(const char* string, const char* format, T* rtn, const char* debug_str)
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
		if (rtn != nullptr) {
			s = "return ";
		}
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
			auto ret = function.call();

			if (rtn != nullptr && ret.size() >= 1) {
				auto stack_start = lua_gettop(LuaState);

				auto val = ret.front();
				val.pushValue();

				scripting::internal::Ade_get_args_skip      = stack_start;
				scripting::internal::Ade_get_args_lfunction = true;
				scripting::ade_get_args(LuaState, format, rtn);
				scripting::internal::Ade_get_args_skip      = 0;
				scripting::internal::Ade_get_args_lfunction = false;
			}
		} catch (const LuaException&) {
			return false;
		}
	} catch (const LuaException& e) {
		LuaError(GetLuaSession(), "%s", e.what());

		return false;
	}

	return true;
}

template <typename T>
int script_state::RunBytecode(script_function& hd, char format, T* data)
{
	using namespace luacpp;

	if (!hd.function.isValid()) {
		return 1;
	}

	GR_DEBUG_SCOPE("Lua code");

	try {
		auto ret = hd.function.call();

		if (data != nullptr && ret.size() >= 1) {
			auto stack_start = lua_gettop(LuaState);

			auto val = ret.front();
			val.pushValue();

			char fmt[2]                                 = {format, '\0'};
			scripting::internal::Ade_get_args_skip      = stack_start;
			scripting::internal::Ade_get_args_lfunction = true;
			scripting::ade_get_args(LuaState, fmt, data);
			scripting::internal::Ade_get_args_skip      = 0;
			scripting::internal::Ade_get_args_lfunction = false;
		}
	} catch (const LuaException&) {
		return 0;
	}

	return 1;
}

//**********Script registration functions
void script_init();

//**********Script globals
extern class script_state Script_system;
extern bool Output_scripting_meta;

//*************************Conditional scripting*************************

#endif //_SCRIPTING_H
