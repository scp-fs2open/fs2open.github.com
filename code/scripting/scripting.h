#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#include "graphics/2d.h"
#include "scripting/ade_args.h"
#include "scripting/lua/LuaFunction.h"
#include "utils/event.h"

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

namespace scripting {
struct ScriptingDocumentation;
}

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

// Conditionals
enum ConditionalType {
	CHC_NONE        = -1,
	CHC_MISSION     = 0,
	CHC_SHIP        = 1,
	CHC_SHIPCLASS   = 2,
	CHC_SHIPTYPE    = 3,
	CHC_STATE       = 4,
	CHC_CAMPAIGN    = 5,
	CHC_WEAPONCLASS = 6,
	CHC_OBJECTTYPE  = 7,
	CHC_KEYPRESS    = 8,
	CHC_ACTION      = 9,
	CHC_VERSION     = 10,
	CHC_APPLICATION = 11,
};

//Actions
enum ConditionalActions : int32_t {
	CHA_NONE    = -1,
	CHA_DEATH,
	CHA_ONFRAME,
	CHA_COLLIDESHIP,
	CHA_COLLIDEWEAPON,
	CHA_COLLIDEDEBRIS,
	CHA_COLLIDEASTEROID,
	CHA_HUDDRAW,
	CHA_OBJECTRENDER,
	CHA_SPLASHSCREEN,
	CHA_GAMEINIT,
	CHA_MISSIONSTART,
	CHA_MISSIONEND,
	CHA_MOUSEMOVED,
	CHA_MOUSEPRESSED,
	CHA_MOUSERELEASED,
	CHA_KEYPRESSED,
	CHA_KEYRELEASED,
	CHA_ONSTATESTART,
	CHA_ONSTATEEND,
	CHA_ONWEAPONDELETE,
	CHA_ONWPEQUIPPED,
	CHA_ONWPFIRED,
	CHA_ONWPSELECTED,
	CHA_ONWPDESELECTED,
	CHA_GAMEPLAYSTART,
	CHA_ONTURRETFIRED,
	CHA_PRIMARYFIRE,
	CHA_SECONDARYFIRE,
	CHA_ONSHIPARRIVE,
	CHA_COLLIDEBEAM,
	CHA_ONACTION,
	CHA_ONACTIONSTOPPED,
	CHA_MSGRECEIVED,
	CHA_HUDMSGRECEIVED,
	CHA_AFTERBURNSTART,
	CHA_AFTERBURNEND,
	CHA_BEAMFIRE,
	CHA_SIMULATION,
	CHA_LOADSCREEN,
	CHA_CMISSIONACCEPT,
	CHA_ONSHIPDEPART,
	CHA_ONWEAPONCREATED,
	CHA_ONWAYPOINTSDONE,
	CHA_ONSUBSYSDEATH,
	CHA_ONGOALSCLEARED,
	CHA_ONBRIEFSTAGE,
	CHA_ONDEBRISCREATED,

	CHA_LAST = CHA_ONDEBRISCREATED,
};

// management stuff
void scripting_state_init();
void scripting_state_close();
void scripting_state_do_frame(float frametime);

int32_t scripting_string_to_action(const char* action);
ConditionalType scripting_string_to_condition(const char* condition);

struct script_condition
{
	ConditionalType condition_type = CHC_NONE;
	SCP_string condition_string;
};

struct script_action
{
	int32_t action_type {CHA_NONE};
	script_hook hook;
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

	SCP_vector<script_function> GameInitFunctions;

private:

	void ParseChunkSub(script_function& out_func, const char* debug_str=NULL);

	void SetLuaSession(struct lua_State *L);

	void OutputLuaDocumentation(scripting::ScriptingDocumentation& doc);

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
	scripting::ScriptingDocumentation OutputDocumentation();

	//***Moves data
	//void MoveData(script_state &in);

	template<typename T>
	void SetHookVar(const char *name, char format, T&& value);
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
	void ParseGlobalChunk(ConditionalActions hookType, const char* debug_str=nullptr);
	bool ParseCondition(const char *filename="<Unknown>");
	void AddConditionedHook(ConditionedHook hook);

	void AddGameInitFunction(script_function func);

	//***Hook running functions
	template <typename T>
	int RunBytecode(script_function& hd, char format = '\0', T* data = nullptr);
	int RunBytecode(script_function& hd);
	bool IsOverride(script_hook &hd);
	int RunCondition(int condition, object* objp = nullptr, int more_data = 0);
	bool IsConditionOverride(int action, object *objp=NULL);

	void RunInitFunctions();

	//*****Other functions
	void EndFrame();

	util::event<void, lua_State*> OnStateDestroy;
};

template<typename T>
void script_state::SetHookVar(const char *name, char format, T&& value)
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
			scripting::ade_set_args(LuaState, fmt, std::forward<T>(value));
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
			auto ret = function.call(LuaState);

			if (rtn != nullptr && ret.size() >= 1) {
				auto stack_start = lua_gettop(LuaState);

				auto val = ret.front();
				val.pushValue(LuaState);

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
		auto ret = hd.function.call(LuaState);

		if (data != nullptr && ret.size() >= 1) {
			auto stack_start = lua_gettop(LuaState);

			auto val = ret.front();
			val.pushValue(LuaState);

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
extern bool Output_scripting_json;

//*************************Conditional scripting*************************

#endif //_SCRIPTING_H
