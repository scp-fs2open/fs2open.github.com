#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#include "graphics/2d.h"
#include "scripting/ade.h"
#include "scripting/ade_args.h"
#include "scripting/hook_conditions.h"
#include "scripting/lua/LuaFunction.h"
#include "utils/event.h"

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

namespace scripting {
struct ScriptingDocumentation;
class HookBase;
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
	//These conditionals must ONLY contain global conditions.
	//That is, conditions that do not depend on anything specific per-hook (such as a ship firing a weapon),
	//but only conditions that can evaluate exclusively on a global state, identically for all actions. 
	//Per-Hook specific conditionals are added through hook_conditions.h/.cpp and the respective template argument of the hook.
	CHC_NONE        = -1,
	CHC_MISSION,     
	CHC_STATE,       
	CHC_CAMPAIGN,    
	CHC_KEYPRESS,    
	CHC_VERSION,    
	CHC_APPLICATION, 
	CHC_MULTI_SERVER
};

//Actions
enum ConditionalActions : int32_t {
	CHA_NONE    = -1,
	CHA_ONFRAME,
	CHA_SPLASHSCREEN,
	CHA_HUDDRAW,
	CHA_GAMEINIT,
	CHA_SIMULATION,

	// DO NOT ADD NEW HOOKS HERE. THESE HOOKS ARE EXCLUSIVELY FOR COMPATIBILITY WITH NON-CONDITIONAL GLOBAL HOOKS
	// There is a new Lua Hook API, see hook_api.h
	// There you use either scripting::Hook for non-overridable or scripting::OverridableHook for overridable hooks
	// while also having the option to document when the hook is called and what hook variables are set for it.

	CHA_LAST = CHA_SIMULATION
};

// management stuff
void scripting_state_init();
void scripting_state_close();
void scripting_state_do_frame(float frametime);

const scripting::HookBase* scripting_string_to_action(const char* action);
ConditionalType scripting_string_to_condition(const char* condition);

struct script_condition
{
	ConditionalType condition_type = CHC_NONE;
	SCP_string condition_string;
	// stores values evaluated at hook load to optimize later condition checking.
	// currently mostly olg done for the highest impact condition types, according to performance profiling
	// the exact type of information stored varries between hook types.
	// CHC_STATE, CHC_OBJECTTYPE - stores the value of enum matching the name requested by the condition string.
	// CHC_SHIPCLASS, CHC_WEAPONCLASS - stores the index of the info object requested by the condition
	// CHC_VERSION, CHC_APPLICATION - stores validity of the check in 1 for true or 0 for false, as the condition will not change after load.
	// see ConditionedHook::AddCondition for exact implimentation
	int condition_cached_value = -1;
};

class script_action {
public:
	SCP_vector<script_condition> global_conditions;
	SCP_vector<std::unique_ptr<scripting::EvaluatableCondition>> local_conditions;

	script_hook hook;

	bool ConditionsValid(const linb::any& local_condition_data) const;
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
	SCP_unordered_map<int, SCP_vector<script_action>> ConditionalHooks;
	// Scripts can add new hooks at runtime; we collect all hooks to be added here and add them at the end of the current
	// frame to avoid corrupting any iterators that the script system may be using.
	SCP_unordered_map<int, SCP_vector<script_action>> AddedHooks;

	SCP_vector<script_function> GameInitFunctions;

	// Stores references to the Lua values for the hook variables. Uses a raw reference since we do not need the more
	// advanced features of LuaValue
	// values are a vector to provide a stack of values. This is necessary to ensure consistent behavior if a scripting
	// hook is called from within another script (e.g. calls to createShip)
	SCP_unordered_map<SCP_string, SCP_vector<luacpp::LuaReference>> HookVariableValues;

	// ActiveActions lets code that might run scripting hooks know whether any scripts are even registered for it.
	// AssayActions is responsible for keeping it up to date.
	SCP_unordered_map<int, bool> ActiveActions;

	void ParseChunkSub(script_function& out_func, const char* debug_str=NULL);

	void SetLuaSession(struct lua_State *L);

	static void OutputLuaDocumentation(scripting::ScriptingDocumentation& doc,
		const scripting::DocumentationErrorReporter& errorReporter);

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
	scripting::ScriptingDocumentation OutputDocumentation(const scripting::DocumentationErrorReporter& errorReporter);

	//***Moves data
	//void MoveData(script_state &in);

	template<typename T>
	void SetHookVar(const char *name, char format, T&& value);
	void SetHookObject(const char *name, object *objp);
	void SetHookObjects(int num, ...);
	void RemHookVar(const char *name);
	void RemHookVars(std::initializer_list<SCP_string> names);

	const SCP_unordered_map<SCP_string, SCP_vector<luacpp::LuaReference>>& GetHookVariableReferences();

	//***Hook creation functions
	template <typename T>
	bool EvalStringWithReturn(const char* string, const char* format = nullptr, T* rtn = NULL,
	                          const char* debug_str = nullptr);
	bool EvalString(const char* string, const char* debug_str = nullptr);
	void ParseChunk(script_hook *dest, const char* debug_str=NULL);
	void ParseGlobalChunk(ConditionalActions hookType, const char* debug_str=nullptr);
	bool ParseCondition(const char *filename="<Unknown>");
	void AddConditionedHook(int action_id, script_action hook);
	void AssayActions();
	bool IsActiveAction(int hookId);

	void AddGameInitFunction(script_function func);

	//***Hook running functions
	template <typename T>
	int RunBytecode(const script_function& hd, char format = '\0', T* data = nullptr);
	int RunBytecode(const script_function& hd);
	bool IsOverride(const script_hook &hd);
	int RunCondition(int action_type, linb::any local_condition_data);
	bool IsConditionOverride(int action_type, linb::any local_condition_data);

	void RunInitFunctions();

	void ProcessAddedHooks();

	//*****Other functions
	static script_state* GetScriptState(lua_State* L);
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
		::scripting::ade_set_args(LuaState, fmt, std::forward<T>(value));
		auto reference = luacpp::UniqueLuaReference::create(LuaState);
		lua_pop(LuaState, 1); // Remove object value from the stack

		HookVariableValues[name].push_back(std::move(reference));
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
			}
		} catch (const LuaException&) {
			lua_pop(LuaState, 1);

			return false;
		}
	} catch (const LuaException& e) {
		LuaError(GetLuaSession(), "%s", e.what());

		lua_pop(LuaState, 1);

		return false;
	}

	lua_pop(LuaState, 1);

	return true;
}

template <typename T>
int script_state::RunBytecode(const script_function& hd, char format, T* data)
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

			// Reset stack again
			lua_settop(LuaState, stack_start);
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
