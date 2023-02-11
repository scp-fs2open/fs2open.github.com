#include "scripting/scripting.h"

#include "globalincs/systemvars.h"
#include "globalincs/version.h"

#include "ade.h"
#include "ade_args.h"
#include "freespace.h"
#include "hook_api.h"

#include "bmpman/bmpman.h"
#include "controlconfig/controlsconfig.h"
#include "gamesequence/gamesequence.h"
#include "hud/hud.h"
#include "io/key.h"
#include "mission/missioncampaign.h"
#include "network/multi.h"
#include "parse/parselo.h"
#include "scripting/doc_html.h"
#include "scripting/doc_json.h"
#include "scripting/scripting_doc.h"
#include "ship/ship.h"
#include "tracing/tracing.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

using namespace scripting;

// tehe. Declare the main event
script_state Script_system("FS2_Open Scripting");
bool Output_scripting_meta = false;
bool Output_scripting_json = false;

flag_def_list Script_conditions[] = 
{
	{"State",		CHC_STATE,			0},
	{"Campaign", CHC_CAMPAIGN, 0},
	{"Mission", CHC_MISSION, 0},
	{"KeyPress", CHC_KEYPRESS, 0},
	{"Version", CHC_VERSION, 0},
	{"Application", CHC_APPLICATION, 0},
	{"Multi type", CHC_MULTI_SERVER, 0},
};

int Num_script_conditions = sizeof(Script_conditions) / sizeof(flag_def_list);

class BuiltinHook : public scripting::HookBase {
  public:
	BuiltinHook(SCP_string hookName, int32_t hookId)
		: HookBase(std::move(hookName), SCP_string(), SCP_vector<HookVariableDocumentation>(), SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>{}, hookId)
	{
	}
	~BuiltinHook() override = default;

	bool isActive() const override
	{
		return Script_system.IsActiveAction(_hookId);
	}

	bool isOverridable() const override { return true; }
};

// clang-format off
static HookVariableDocumentation GlobalVariables[] =
{
	{
		"Player",
		"object",
		"The player object in a mission. Does not need to be a ship (e.g. in multiplayer). Not "
		"present if not in a game play state."
	},
};
// clang-format on

int scripting_state_inited = 0;

//*************************Scripting init and handling*************************

// ditto
bool script_hook_valid(script_hook* hook) { return hook->hook_function.function.isValid(); }

void script_parse_table(const char* filename)
{
	script_state* st = &Script_system;

	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		if (optional_string("#Global Hooks")) {
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

		if (optional_string("#Conditional Hooks"))
		{
			while (st->ParseCondition(filename));
			required_string("#End");
		}

		st->ProcessAddedHooks();
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}
void script_parse_lua_script(const char *filename) {
	using namespace luacpp;

	CFILE *cfp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_TABLES);
	if(cfp == nullptr)
	{
		Warning(LOCATION, "Could not open lua script file '%s'", filename);
		return;
	}

	int len = cfilelength(cfp);

	SCP_string source;
	source.resize((size_t) len);
	cfread(&source[0], len, 1, cfp);
	cfclose(cfp);

	try {
		auto function = LuaFunction::createFromCode(Script_system.GetLuaSession(), source, filename);
		function.setErrorFunction(LuaFunction::createFromCFunction(Script_system.GetLuaSession(), ade_friendly_error));

		script_function func;
		func.language = SC_LUA;
		func.function = function;

		Script_system.AddGameInitFunction(func);
	} catch (const LuaException& e) {
		LuaError(Script_system.GetLuaSession(), "Failed to parse %s: %s", filename, e.what());
	}
}

// Initializes the (global) scripting system, as well as any subsystems.
// script_close is handled by destructors
void script_init()
{
	mprintf(("SCRIPTING: Beginning initialization sequence...\n"));

	mprintf(("SCRIPTING: Beginning Lua initialization...\n"));
	Script_system.CreateLuaState();

	if (Output_scripting_meta || Output_scripting_json) {
		const auto doc = Script_system.OutputDocumentation([](const SCP_string& error) {
			mprintf(("Scripting documentation: Error while parsing\n%s(This is only relevant for coders)\n\n",
				error.c_str()));
		});

		if (Output_scripting_meta) {
			mprintf(("SCRIPTING: Outputting scripting metadata...\n"));
			scripting::output_html_doc(doc, "scripting.html");
		}
		if (Output_scripting_json) {
			mprintf(("SCRIPTING: Outputting scripting metadata in JSON format...\n"));
			scripting::output_json_doc(doc, "scripting.json");
		}
	}

	mprintf(("SCRIPTING: Beginning main hook parse sequence....\n"));
	script_parse_table("scripting.tbl");
	parse_modular_table(NOX("*-sct.tbm"), script_parse_table);
	mprintf(("SCRIPTING: Parsing pure Lua scripts\n"));
	parse_modular_table(NOX("*-sct.lua"), script_parse_lua_script);
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

static bool global_condition_valid(const script_condition& condition)
{
	switch (condition.condition_type) {
	case CHC_STATE:
		if (gameseq_get_depth() < 0)
			return false;
		if (gameseq_get_state() != condition.condition_cached_value)
			return false;
		break;

	case CHC_MISSION: {
		// WMC - Get mission filename with Mission_filename
		// I don't use Game_current_mission_filename, because
		// Mission_filename is valid in both fs2_open and FRED
		size_t len = strlen(Mission_filename);
		if (!len)
			return false;
		if (len > 4 && !stricmp(&Mission_filename[len - 4], ".fs2"))
			len -= 4;
		if (strnicmp(condition.condition_string.c_str(), Mission_filename, len) != 0)
			return false;
		break;
	}

	case CHC_CAMPAIGN: {
		size_t len = strlen(Campaign.filename);
		if (!len)
			return false;
		if (len > 4 && !stricmp(&Mission_filename[len - 4], ".fc2"))
			len -= 4;
		if (strnicmp(condition.condition_string.c_str(), Mission_filename, len) != 0)
			return false;
		break;
	}

	case CHC_KEYPRESS: {
		extern int Current_key_down;
		if (gameseq_get_depth() < 0)
			return false;
		if (Current_key_down == 0)
			return false;
		// WMC - could be more efficient, but whatever.
		if (stricmp(textify_scancode_universal(Current_key_down), condition.condition_string.c_str()) != 0)
			return false;
		break;
	}

	case CHC_VERSION: {
		// Already evaluated on script load, stored value is 1 if application matches condition, 0 if not.
		if (condition.condition_cached_value == 0) {
			return false;
		}
		break;
	}

	case CHC_APPLICATION: {
		// Already evaluated on script load, stored value is 1 if application matches condition, 0 if not.
		if (condition.condition_cached_value == 0) {
			return false;
		}
		break;
	}

	case CHC_MULTI_SERVER: {
		// condition_cached_value is 0 if we execute on clients, 1 on servers
		return static_cast<bool>(condition.condition_cached_value == 1) == static_cast<bool>(MULTIPLAYER_MASTER);
	}

	default:
		break;
	}

	return true;
}

int cache_condition(ConditionalType type, const SCP_string& value){
	//Since string comparisons are expensive and these hooks have to be checked very frequently
	//where possible whatever string comparison is done here and the outcome stored for later
	//nature of value stored depends on condition type.
	switch (type)
	{
	case CHC_STATE:
		return gameseq_get_state_idx(value.c_str());
	case CHC_VERSION:
	{
		return gameversion::parse_version_inline() == gameversion::get_executable_version() ? 1 : 0;
	}
	case CHC_APPLICATION:
	{
		if (Fred_running)
		{
			if (stricmp("FRED2_Open", value.c_str()) != 0 && stricmp("FRED2Open", value.c_str()) != 0 && stricmp("FRED 2", value.c_str()) != 0 && stricmp("FRED", value.c_str()) != 0)
				return 0;
			else
				return 1;
		}
		else
		{
			if (stricmp("FS2_Open", value.c_str()) != 0 && stricmp("FS2Open", value.c_str()) != 0 && stricmp("Freespace 2", value.c_str()) != 0 && stricmp("Freespace", value.c_str()) != 0)
				return 0;
			else
				return 1;
		}
	}
	case CHC_MULTI_SERVER:
	{
		if (stricmp("Server", value.c_str()) == 0 || stricmp("Master", value.c_str()) == 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	default:
		return -1;
	}
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
	if (LuaState == nullptr) {
		return;
	}

	va_list vl;
	va_start(vl, num);

	for (int i = 0; i < num; i++) {
		char* name = va_arg(vl, char*);
		object* objp = va_arg(vl, object*);

		ade_set_object_with_breed(LuaState, OBJ_INDEX(objp));
		auto reference = luacpp::UniqueLuaReference::create(LuaState);
		lua_pop(LuaState, 1); // Remove object value from the stack

		HookVariableValues[name].push_back(std::move(reference));
	}

	va_end(vl);
}

void script_state::RemHookVar(const char* name)
{
	this->RemHookVars({name});
}

void script_state::RemHookVars(std::initializer_list<SCP_string> names)
{
	if (LuaState != nullptr) {
		for (const auto& hookVar : names) {
			if (HookVariableValues[hookVar].empty()) {
				// Nothing to do
				continue;
			}
			HookVariableValues[hookVar].pop_back();
		}
	}
}
const SCP_unordered_map<SCP_string, SCP_vector<luacpp::LuaReference>>& script_state::GetHookVariableReferences()
{
	return HookVariableValues;
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

int script_state::RunCondition(int action_type, linb::any local_condition_data)
{
	TRACE_SCOPE(tracing::LuaHooks);
	int num = 0;

	if (LuaState == nullptr) {
		return num;
	}

	auto action_it = ConditionalHooks.find(action_type);
	if (action_it == ConditionalHooks.end())
		return num;

	for(const auto& action : action_it->second) 
	{
		if (action.ConditionsValid(local_condition_data))
		{
			RunBytecode(action.hook.hook_function);
			num++;
		}
	}

	ProcessAddedHooks();
	return num;
}

bool script_state::IsConditionOverride(int action_type, linb::any local_condition_data)
{
	auto action_it = ConditionalHooks.find(action_type);
	if (action_it == ConditionalHooks.end())
		return false;

	for (const auto& action : action_it->second)
	{
		if (action.ConditionsValid(local_condition_data))
		{
			if (IsOverride(action.hook))
				return true;
		}
	}
	return false;
}

void script_state::Clear()
{
	// Free all lua value references
	ConditionalHooks.clear();
	HookVariableValues.clear();

	AssayActions();

	if (LuaState != nullptr) {
		OnStateDestroy(LuaState);

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
	auto len = sizeof(StateName);
	strncpy(StateName, name, len);
	StateName[len - 1] = 0;

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
	if (LuaState != nullptr)
	{
		lua_close(LuaState);
	}
	LuaState = L;
	if (LuaState != nullptr) {
		Langs |= SC_LUA;
	}
	else if(Langs & SC_LUA) {
		Langs &= ~SC_LUA;
	}
}

ScriptingDocumentation script_state::OutputDocumentation(const scripting::DocumentationErrorReporter& errorReporter)
{
	ScriptingDocumentation doc;

	doc.name = StateName;

	// Conditions
	doc.conditions.reserve(static_cast<size_t>(Num_script_conditions));
	for (int32_t i = 0; i < Num_script_conditions; i++) {
		doc.conditions.emplace_back(Script_conditions[i].name);
	}

	// Global variables
	doc.globalVariables.assign(std::begin(GlobalVariables), std::end(GlobalVariables));

	// Actions
	auto sortedHooks = scripting::getHooks();
	std::sort(sortedHooks.begin(),
			  sortedHooks.end(),
			  [](const scripting::HookBase* left, const scripting::HookBase* right) {
				  return left->getHookName() < right->getHookName();
			  });
	for (const auto& hook : sortedHooks) {
		doc.actions.push_back(
			{hook->getHookName(), hook->getDescription(), hook->getParameters(), hook->_conditions, hook->isOverridable()});
	}

	OutputLuaDocumentation(doc, errorReporter);

	return doc;
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

		for (auto i = 1; i <= line; ++i) {
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
			function.call(LuaState);
		} catch (const LuaException&) {
			return false;
		}
	} catch (const LuaException& e) {
		LuaError(GetLuaSession(), "%s", e.what());

		return false;
	}

	return true;
}

int script_state::RunBytecode(const script_function& hd)
{
	using namespace luacpp;

	if (!hd.function.isValid()) {
		return 1;
	}

	GR_DEBUG_SCOPE("Lua code");

	try {
		hd.function.call(LuaState);
	} catch (const LuaException&) {
		return 0;
	}

	return 1;
}

ConditionalType script_parse_condition()
{
	char buf[NAME_LENGTH];
	for (int i = 0; i < Num_script_conditions; i++) {
		sprintf(buf, "$%s:", Script_conditions[i].name);
		if(optional_string(buf))
			return static_cast<ConditionalType>(Script_conditions[i].def);
	}

	return CHC_NONE;
}

const scripting::HookBase* script_parse_action()
{
	for (const auto& action : scripting::getHooks()) {
		SCP_string buf;
		sprintf(buf, "$%s:", action->getHookName().c_str());
		if (optional_string(buf.c_str()))
			return action;
	}

	return nullptr;
}

const HookBase* scripting_string_to_action(const char* action)
{
	for (const auto& hook : scripting::getHooks()) {
		if (hook->getHookName() == action)
			return hook;
	}

	return nullptr;
}

ConditionalType scripting_string_to_condition(const char* condition)
{
	for (int i = 0; i < Num_script_conditions; i++) {
		if (!stricmp(Script_conditions[i].name, condition)) {
			return static_cast<ConditionalType>(Script_conditions[i].def);
		}
	}

	return CHC_NONE;
}

bool script_action::ConditionsValid(const linb::any& local_condition_data) const {
	for (const auto& global_condition : global_conditions) {
		if (!global_condition_valid(global_condition))
			return false;
	}

	for (const auto& local_condition : local_conditions) {
		if (!local_condition->evaluate(local_condition_data))
			return false;
	}

	return true;
};

void script_state::ParseGlobalChunk(ConditionalActions hookType, const char* debug_str) {
	script_action sat;

	ParseChunk(&sat.hook, debug_str);

	ConditionalHooks[hookType].emplace_back(std::move(sat));
}
bool script_state::ParseCondition(const char *filename)
{
	SCP_vector<script_condition> parsed_conditions;
	SCP_vector<SCP_string> conditions;

	//First, is this a script condition?

	const HookBase* currHook = nullptr;

	//As long as we don't get hooks, its gotta be a condition
	while ((currHook = script_parse_action()) == nullptr) {
		auto condition = script_parse_condition();
		if (condition != CHC_NONE) {
			//It's a global condition
			SCP_string condition_string;
			stuff_string(condition_string, F_NAME);
			int cache = cache_condition(condition, condition_string);
			parsed_conditions.emplace_back(script_condition{ condition, std::move(condition_string), cache });
		}
		else {
			if (check_for_string("#End") || check_for_eof()) {
				//There was no action here, but an EOF
				if(!parsed_conditions.empty() || !conditions.empty())
					error_display(1, "No actions specified for conditional hook in file '%s'", filename);
				return false;
			}

			//It's a local condition. Store it once we find hooks.
			SCP_string condition_string;
			stuff_string(condition_string, F_RAW);
			conditions.emplace_back(std::move(condition_string));
		}
	}

	do {
		int hookId = currHook->getHookId();
		script_action sat;

		// WMC - build error string
		SCP_string buf;
		sprintf(buf, "%s - %s", filename, currHook->getHookName().c_str());

		ParseChunk(&sat.hook, buf.c_str());

		sat.global_conditions = parsed_conditions;
		for (const SCP_string& local_condition : conditions) {
			bool found = false;
			pause_parse();
			SCP_vm_unique_ptr<char> parse{ vm_strdup(local_condition.c_str()) };
			reset_parse(parse.get());
			for (const auto& potential_condition : currHook->_conditions) {
				SCP_string bufCond;
				sprintf(bufCond, "$%s:", potential_condition.first.c_str());
				if (optional_string(bufCond.c_str())) {
					SCP_string arg;
					stuff_string(arg, F_NAME);
					sat.local_conditions.emplace_back(potential_condition.second->parse(arg));
					found = true;
					break;
				}
			}
			unpause_parse();
			
			if (!found) {
				error_display(0, "Condition '%s' is not valid for hook '%s'. The hook will not evaluate!", local_condition.c_str(), currHook->getHookName().c_str());
				sat.local_conditions.emplace_back(ParseableCondition().parse(local_condition));
				continue;
			}
		}

		AddConditionedHook(hookId, std::move(sat));

	} while ((currHook = script_parse_action()) != nullptr);

	return true;
}

void script_state::AddConditionedHook(int action_id, script_action hook) {
	AddedHooks[action_id].emplace_back(std::move(hook));
}

void script_state::ProcessAddedHooks() {
	for (auto& hook : AddedHooks) {
		auto& conditionalHooks = ConditionalHooks[hook.first];
		conditionalHooks.insert(conditionalHooks.end(), std::make_move_iterator(hook.second.begin()), std::make_move_iterator(hook.second.end()));
	}
	AddedHooks.clear();
	AssayActions();
}

void script_state::AddGameInitFunction(script_function func) { GameInitFunctions.push_back(std::move(func)); }

// For each possible script_action this maintains an array that records whether any scripts are actually using this action
// This allows us to avoid significant overhead from checking everything at the potential hook sites, but you must call
// AssayActions() after modifying ConditionalHooks before returning to normal operation of the scripting system!
void script_state::AssayActions() {
	ActiveActions.clear();

	for (const auto &hook : ConditionalHooks) {
		ActiveActions[hook.first] = !hook.second.empty();
	}
}

bool script_state::IsActiveAction(int action_id) {
	auto entry = ActiveActions.find(action_id);
	if (entry != ActiveActions.end())
		return entry->second;
	else
		return false;
}

bool script_state::IsOverride(const script_hook &hd)
{
	if(!hd.hook_function.function.isValid())
		return false;

	bool b=false;
	RunBytecode(hd.override_function, 'b', &b);

	return b;
}

void script_state::RunInitFunctions() {
	for (const auto& initFunc : GameInitFunctions) {
		initFunc.function(LuaState);
	}
	// We don't need this anymore so no need to keep references to those functions around anymore
	GameInitFunctions.clear();
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
