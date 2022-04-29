#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#include "graphics/2d.h"
#include "scripting/ade.h"
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
	CHA_MOUSEMOVED,
	CHA_MOUSEPRESSED,
	CHA_MOUSERELEASED,
	CHA_KEYPRESSED,
	CHA_KEYRELEASED,
	CHA_ONSTATESTART,
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

	// DO NOT ADD NEW HOOKS HERE
	// There is a new Lua Hook API, see hook_api.h
	// There you use either scripting::Hook for non-overridable or scripting::OverridableHook for overridable hooks
	// while also having the option to document when the hook is called and what hook variables are set for it.

	CHA_LAST = CHA_ONBRIEFSTAGE,
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
	// stores values evaluated at hook load to optimize later condition checking.
	// currently mostly olg done for the highest impact condition types, according to performance profiling
	// the exact type of information stored varries between hook types.
	// CHC_STATE, CHC_OBJECTTYPE - stores the value of enum matching the name requested by the condition string.
	// CHC_SHIPCLASS, CHC_WEAPONCLASS - stores the index of the info object requested by the condition
	// CHC_VERSION, CHC_APPLICATION - stores validity of the check in 1 for true or 0 for false, as the condition will not change after load.
	// see ConditionedHook::AddCondition for exact implimentation
	int condition_cached_value = -1;
};

struct script_action
{
	int32_t action_type {CHA_NONE};
	script_hook hook;
};

class ConditionedHook
{
public:
	SCP_vector<script_action> Actions;
	SCP_vector<script_condition> Conditions;
	bool AddCondition(script_condition *sc);
	bool AddAction(script_action *sa);

	bool ConditionsValid(int action, class object *objp1 = nullptr, class object *objp2 = nullptr, int more_data = -1);
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

	// Stores references to the Lua values for the hook variables. Uses a raw reference since we do not need the more
	// advanced features of LuaValue
	// values are a vector to provide a stack of values. This is necessary to ensure consistent behavior if a scripting
	// hook is called from within another script (e.g. calls to createShip)
	SCP_unordered_map<SCP_string, SCP_vector<luacpp::LuaReference>> HookVariableValues;
	// ActiveActions lets code that might run scripting hooks know whether any scripts are even registered for it.
	// AssayActions is responsible for keeping it up to date.
	bool ActiveActions[ConditionalActions::CHA_LAST+1];

	void ParseChunkSub(script_function& out_func, const char* debug_str=NULL);

	void SetLuaSession(struct lua_State *L);

	static void OutputLuaDocumentation(scripting::ScriptingDocumentation& doc,
		const scripting::DocumentationErrorReporter& errorReporter);

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
	void AddConditionedHook(ConditionedHook hook);
	void AssayActions();
	bool IsActiveAction(ConditionalActions action_id);

	void AddGameInitFunction(script_function func);

	//***Hook running functions
	template <typename T>
	int RunBytecode(script_function& hd, char format = '\0', T* data = nullptr);
	int RunBytecode(script_function& hd);
	bool IsOverride(script_hook &hd);
	int RunCondition(int condition, object *objp1 = nullptr, object *objp2 = nullptr, int more_data = -1);
	bool IsConditionOverride(int action, object *objp1 = nullptr, object *objp2 = nullptr, int more_data = -1);

	void RunInitFunctions();

	//*****Other functions
	void EndFrame();

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
