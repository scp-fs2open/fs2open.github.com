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
	{"Object Type", CHC_OBJECTTYPE, 0},
	{"Ship", CHC_SHIP, 0},
	{"Ship class", CHC_SHIPCLASS, 0},
	{"Ship type", CHC_SHIPTYPE, 0},
	{"Weapon class", CHC_WEAPONCLASS, 0},
	{"KeyPress", CHC_KEYPRESS, 0},
	{"Action", CHC_ACTION, 0},
	{"Version", CHC_VERSION, 0},
	{"Application", CHC_APPLICATION, 0},
};

int Num_script_conditions = sizeof(Script_conditions) / sizeof(flag_def_list);

class BuiltinHook : public scripting::HookBase {
  public:
	BuiltinHook(SCP_string hookName, int32_t hookId)
		: HookBase(std::move(hookName), SCP_string(), SCP_vector<HookVariableDocumentation>(), hookId)
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
static USED_VARIABLE SCP_vector<std::shared_ptr<BuiltinHook>> Script_actions
{
	std::make_shared<BuiltinHook>("On Splash Screen",		CHA_SPLASHSCREEN ),
	std::make_shared<BuiltinHook>("On State Start",			CHA_ONSTATESTART ),
	std::make_shared<BuiltinHook>("On Action",				CHA_ONACTION ),
	std::make_shared<BuiltinHook>("On Action Stopped",		CHA_ONACTIONSTOPPED ),
	std::make_shared<BuiltinHook>("On Key Pressed",			CHA_KEYPRESSED ),
	std::make_shared<BuiltinHook>("On Key Released",		CHA_KEYRELEASED ),
	std::make_shared<BuiltinHook>("On Mouse Moved",			CHA_MOUSEMOVED ),
	std::make_shared<BuiltinHook>("On Mouse Pressed",		CHA_MOUSEPRESSED ),
	std::make_shared<BuiltinHook>("On Mouse Released",		CHA_MOUSERELEASED ),
	std::make_shared<BuiltinHook>("On Mission Start",		CHA_MISSIONSTART ),
	std::make_shared<BuiltinHook>("On HUD Draw",			CHA_HUDDRAW ),
	std::make_shared<BuiltinHook>("On Weapon Collision",	CHA_COLLIDEWEAPON ),
	std::make_shared<BuiltinHook>("On Debris Collision",	CHA_COLLIDEDEBRIS ),
	std::make_shared<BuiltinHook>("On Asteroid Collision",	CHA_COLLIDEASTEROID ),
	std::make_shared<BuiltinHook>("On Object Render",		CHA_OBJECTRENDER ),
	std::make_shared<BuiltinHook>("On Weapon Delete",		CHA_ONWEAPONDELETE ),
	std::make_shared<BuiltinHook>("On Weapon Equipped",		CHA_ONWPEQUIPPED ),
	std::make_shared<BuiltinHook>("On Weapon Fired",		CHA_ONWPFIRED ),
	std::make_shared<BuiltinHook>("On Weapon Selected",		CHA_ONWPSELECTED ),
	std::make_shared<BuiltinHook>("On Weapon Deselected",	CHA_ONWPDESELECTED ),
	std::make_shared<BuiltinHook>("On Gameplay Start",		CHA_GAMEPLAYSTART ),
	std::make_shared<BuiltinHook>("On Turret Fired",		CHA_ONTURRETFIRED ),
	std::make_shared<BuiltinHook>("On Primary Fire",		CHA_PRIMARYFIRE ),
	std::make_shared<BuiltinHook>("On Secondary Fire",		CHA_SECONDARYFIRE ),
	std::make_shared<BuiltinHook>("On Ship Arrive",			CHA_ONSHIPARRIVE ),
	std::make_shared<BuiltinHook>("On Beam Collision",		CHA_COLLIDEBEAM ),
	std::make_shared<BuiltinHook>("On Afterburner Engage",	CHA_AFTERBURNSTART ),
	std::make_shared<BuiltinHook>("On Afterburner Stop",	CHA_AFTERBURNEND ),
	std::make_shared<BuiltinHook>("On Beam Fire",			CHA_BEAMFIRE ),
	std::make_shared<BuiltinHook>("On Simulation",			CHA_SIMULATION ),
	std::make_shared<BuiltinHook>("On Load Screen",			CHA_LOADSCREEN ),
	std::make_shared<BuiltinHook>("On Campaign Mission Accept", CHA_CMISSIONACCEPT ),
	std::make_shared<BuiltinHook>("On Ship Depart",			CHA_ONSHIPDEPART ),
	std::make_shared<BuiltinHook>("On Weapon Created",		CHA_ONWEAPONCREATED ),
	std::make_shared<BuiltinHook>("On Waypoints Done",		CHA_ONWAYPOINTSDONE ),
	std::make_shared<BuiltinHook>("On Subsystem Destroyed",	CHA_ONSUBSYSDEATH ),
	std::make_shared<BuiltinHook>("On Goals Cleared",		CHA_ONGOALSCLEARED ),
	std::make_shared<BuiltinHook>("On Briefing Stage",		CHA_ONBRIEFSTAGE ),
	// DO NOT ADD NEW HOOKS HERE, see scripting.h for a more in-depth explanation
};

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

		st->AssayActions();
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
bool ConditionedHook::AddCondition(script_condition *sc)
{
	//Since string comparisons are expensive and these hooks have to be checked very frequently
	//where possible whatever string comparison is done here and the outcome stored for later
	//nature of value stored depends on condition type.
	switch (sc->condition_type)
	{
	case CHC_SHIPCLASS:
		sc->condition_cached_value = ship_info_lookup(sc->condition_string.c_str());
		break;
	case CHC_STATE:
		sc->condition_cached_value = gameseq_get_state_idx(sc->condition_string.c_str());
		break;
	case CHC_WEAPONCLASS:
		sc->condition_cached_value = weapon_info_lookup(sc->condition_string.c_str());
		break;
	case CHC_OBJECTTYPE:
		for (int i = 0; i < MAX_OBJECT_TYPES; i++) {
			if (stricmp(Object_type_names[i], sc->condition_string.c_str()) == 0) {
				sc->condition_cached_value = i;
				break;
			}
		}
		break;
	case CHC_VERSION:
	{
		char buf[32];
		sc->condition_cached_value = 0;
		sprintf(buf, "%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
		if (stricmp(buf, sc->condition_string.c_str()) == 0)
		{
			sc->condition_cached_value = 1;
		}
		else if (FS_VERSION_BUILD == 0) //In case some people are lazy and say "3.7" instead of "3.7.0" or something
		{
			sprintf(buf, "%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR);
			if (stricmp(buf, sc->condition_string.c_str()) == 0) {
				sc->condition_cached_value = 1;
			}
		}
		break;
	}
	case CHC_APPLICATION:
	{
		sc->condition_cached_value = 1;
		if (Fred_running)
		{
			if (stricmp("FRED2_Open", sc->condition_string.c_str()) != 0 && stricmp("FRED2Open", sc->condition_string.c_str()) != 0 && stricmp("FRED 2", sc->condition_string.c_str()) != 0 && stricmp("FRED", sc->condition_string.c_str()) != 0)
				sc->condition_cached_value = 0;
		}
		else
		{
			if (stricmp("FS2_Open", sc->condition_string.c_str()) != 0 && stricmp("FS2Open", sc->condition_string.c_str()) != 0 && stricmp("Freespace 2", sc->condition_string.c_str()) != 0 && stricmp("Freespace", sc->condition_string.c_str()) != 0)
				sc->condition_cached_value = 0;
		}
		break;
	}
	default:
		break;
	}
	Conditions.push_back(*sc);

	return true;
}

bool ConditionedHook::AddAction(script_action *sa)
{
	if(!script_hook_valid(&sa->hook))
		return false;

	Actions.push_back(*sa);

	return true;
}

bool ConditionedHook::ConditionsValid(int action, object *objp1, object *objp2, int more_data)
{
	object *objp_array[2];
	objp_array[0] = objp1;
	objp_array[1] = objp2;

	//Return false if any conditions are not met
	//Never return true inside the loop, or you will be potentially skipping other conditions on the hook.
	for (auto & scp : Conditions)
	{
		switch(scp.condition_type)
		{
			case CHC_STATE:
				if(gameseq_get_depth() < 0)
					return false;
				if(gameseq_get_state() != scp.condition_cached_value)
					return false;
				break;

			case CHC_SHIPTYPE:
			{
				for (auto objp : objp_array)
				{
					if (objp != nullptr && objp->type == OBJ_SHIP)
					{
						auto sip = &Ship_info[Ships[objp->instance].ship_info_index];
						if (sip->class_type >= 0)
						{
							if (stricmp(Ship_types[sip->class_type].name, scp.condition_string.c_str()) != 0)
								return false;
						}
						break;
					}
				}
				break;
			}

			case CHC_SHIPCLASS:
			{
				for (auto objp : objp_array)
				{
					if (objp != nullptr && objp->type == OBJ_SHIP)
					{
						// scp.condition_cached_value holds the ship_info_index of the requested ship class
						if (Ships[objp->instance].ship_info_index != scp.condition_cached_value)
							return false;
						break;
					}
				}
				break;
			}

			case CHC_SHIP:
			{
				for (auto objp : objp_array)
				{
					if (objp != nullptr && objp->type == OBJ_SHIP)
					{
						if (stricmp(Ships[objp->instance].ship_name, scp.condition_string.c_str()) != 0)
							return false;
						break;
					}
				}
				break;
			}

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
				if(strnicmp(scp.condition_string.c_str(), Mission_filename, len) != 0)
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
				if(strnicmp(scp.condition_string.c_str(), Mission_filename, len) != 0)
					return false;
				break;
			}

			case CHC_WEAPONCLASS:
			{
				for (auto objp : objp_array)
				{
					if (objp == nullptr)
						continue;

					if (objp->type == OBJ_WEAPON)
					{
						// scp.condition_cached_value holds the weapon_info_index of the requested weapon class
						if (Weapons[objp->instance].weapon_info_index != scp.condition_cached_value)
							return false;
						break;
					}
					else if (objp->type == OBJ_BEAM)
					{
						// scp.condition_cached_value holds the weapon_info_index of the requested weapon class
						if (Beams[objp->instance].weapon_info_index != scp.condition_cached_value)
							return false;
						break;
					}
				}

				if (objp1 == nullptr || objp1->type != OBJ_SHIP)
					break;
				auto shipp = &Ships[objp1->instance];
				bool primary = false, secondary = false, prev_primary = false, prev_secondary = false;

				switch (action)
				{
					case CHA_ONWPSELECTED:
					{
						if (shipp->weapons.current_primary_bank >= 0) {
							primary = shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] == scp.condition_cached_value;
						}
						if (shipp->weapons.current_secondary_bank >= 0) {
							secondary = shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank] == scp.condition_cached_value;
						}

						if (!(primary || secondary)) {
							return false;
						}

						if ((shipp->flags[Ship::Ship_Flags::Primary_linked]) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink])) {
							return false;
						}

						break;
					}

					case CHA_ONWPDESELECTED:
					{
						if (shipp->weapons.current_primary_bank >= 0) {
							primary = shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] == scp.condition_cached_value;
						}
						if (shipp->weapons.previous_primary_bank >= 0) {
							prev_primary = shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank] == scp.condition_cached_value;
						}
						if (shipp->weapons.current_secondary_bank >= 0) {
							secondary = shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank] == scp.condition_cached_value;
						}
						if (shipp->weapons.previous_secondary_bank >= 0) {
							prev_secondary = shipp->weapons.secondary_bank_weapons[shipp->weapons.previous_secondary_bank] == scp.condition_cached_value;
						}

						if (!(
								(shipp->flags[Ship::Ship_Flags::Primary_linked]) && prev_primary && 
								(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink])
							)) {
							return false;
						}
						if (!prev_secondary && !secondary && !prev_primary && !primary)
							return false;

						if ((!prev_secondary && !secondary) && (prev_primary && primary))
							return false;

						if ((!prev_secondary && !secondary) && (!prev_primary && primary))
							return false;

						if ((!prev_primary && !primary) && (prev_secondary && secondary))
							return false;

						if ((!prev_primary && !primary) && (!prev_secondary && secondary))
							return false;

						break;
					}

					case CHA_ONWPEQUIPPED:
					{
						bool equipped = false;
						for (int j = 0; j < MAX_SHIP_PRIMARY_BANKS && !equipped; j++) {
							if (shipp->weapons.primary_bank_weapons[j] > 0 && shipp->weapons.primary_bank_weapons[j] < weapon_info_size()) {
								equipped = shipp->weapons.primary_bank_weapons[j] == scp.condition_cached_value;
							}
						}
						for (int j = 0; j < MAX_SHIP_SECONDARY_BANKS && !equipped; j++) {
							if (shipp->weapons.secondary_bank_weapons[j] >= 0 && (shipp->weapons.secondary_bank_weapons[j] < weapon_info_size()))
							{
								equipped = shipp->weapons.secondary_bank_weapons[j] == scp.condition_cached_value;
							}

						}

						if (!equipped) {
							return false;
						}

						break;
					}

					case CHA_ONWPFIRED:
					{
						if (more_data == 1) {
							primary = shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] == scp.condition_cached_value;
							secondary = false;
						} else {
							primary = false;
							secondary = shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank] == scp.condition_cached_value;
						}

						if (
							(shipp->flags[Ship::Ship_Flags::Primary_linked]) && primary && 
							(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink]))
						{
							return false;
						}

						if (more_data == 1 && !primary) {
							return false;
						}
						else if (more_data != 1 && !secondary) {
							return false;
						}
						break;
					}

					case CHA_ONTURRETFIRED: {
						if(shipp->last_fired_turret->last_fired_weapon_info_index != scp.condition_cached_value)
							return false;
						break;
					}
					case CHA_PRIMARYFIRE: {
						if (shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] != scp.condition_cached_value)
							return false;
						break;
					}
					case CHA_SECONDARYFIRE: {
						if(shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank] != scp.condition_cached_value)
							return false;
						break;
					}
					case CHA_BEAMFIRE: {
						if(more_data != scp.condition_cached_value)
							return false;
						break;
					}
					default:
						break;
				}
				break;
			} // case CHC_WEAPONCLASS

			case CHC_OBJECTTYPE:
				if (objp1 == nullptr || objp1->type != scp.condition_cached_value)
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
				if(stricmp(textify_scancode(Current_key_down), scp.condition_string.c_str()) != 0)
					return false;
				break;
			}

			case CHC_ACTION:
			{
				if(gameseq_get_depth() < 0)
					return false;

				int action_index = more_data;

				if (action_index <= 0 || stricmp(scp.condition_string.c_str(), Control_config[action_index].text.c_str()) != 0)
					return false;
				break;
			}

			case CHC_VERSION:
			{
				//Already evaluated on script load, stored value is 1 if application matches condition, 0 if not.
				if(scp.condition_cached_value == 0)
				{
					return false;
				}
				break;
			}

			case CHC_APPLICATION:
			{
				//Already evaluated on script load, stored value is 1 if application matches condition, 0 if not.
				if(scp.condition_cached_value == 0)
				{
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

int script_state::RunCondition(int action, object *objp1, object *objp2, int more_data)
{
	TRACE_SCOPE(tracing::LuaHooks);
	int num = 0;

	if (LuaState == nullptr) {
		return num;
	}

	for(SCP_vector<ConditionedHook>::iterator chp = ConditionalHooks.begin(); chp != ConditionalHooks.end(); ++chp) 
	{
		if(chp->ConditionsValid(action, objp1, objp2, more_data))
		{
			chp->Run(this, action);
			num++;
		}
	}
	return num;
}

bool script_state::IsConditionOverride(int action, object *objp1, object *objp2, int more_data)
{
	for(SCP_vector<ConditionedHook>::iterator chp = ConditionalHooks.begin(); chp != ConditionalHooks.end(); ++chp)
	{
		if(chp->ConditionsValid(action, objp1, objp2, more_data))
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
			{hook->getHookName(), hook->getDescription(), hook->getParameters(), hook->isOverridable()});
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
int script_state::RunBytecode(script_function& hd)
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

int32_t scripting_string_to_action(const char* action)
{
	for (const auto& hook : scripting::getHooks()) {
		if (hook->getHookName() == action)
			return hook->getHookId();
	}

	return CHA_NONE;
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

void script_state::ParseGlobalChunk(ConditionalActions hookType, const char* debug_str) {
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
	ConditionedHook *chp = nullptr;

	for(auto condition = script_parse_condition(); condition != CHC_NONE; condition = script_parse_condition())
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
				stuff_string(sct.condition_string, F_NAME);
				break;
			}

			if (chp == nullptr) {
				ConditionalHooks.push_back(ConditionedHook());
				chp = &ConditionalHooks[ConditionalHooks.size() - 1];
			}

			if (!chp->AddCondition(&sct)) {
				Warning(LOCATION,
						"Could not add condition to conditional hook in file '%s'; you may have more than %d",
						filename,
						MAX_HOOK_CONDITIONS);
			}
	}

	if (chp == NULL) {
		return false;
	}

	bool actions_added = false;
	for (const auto* action = script_parse_action(); action != nullptr; action = script_parse_action()) {
		script_action sat;
		sat.action_type = action->getHookId();

		// WMC - build error string
		SCP_string buf;
		sprintf(buf, "%s - %s", filename, action->getHookName().c_str());

		ParseChunk(&sat.hook, buf.c_str());

		// Add the action
		if (chp->AddAction(&sat))
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

void script_state::AddConditionedHook(ConditionedHook hook) {
	ConditionalHooks.push_back(std::move(hook));
	AssayActions();
}

void script_state::AddGameInitFunction(script_function func) { GameInitFunctions.push_back(std::move(func)); }

// For each possible script_action this maintains an array that records whether any scripts are actually using this action
// This allows us to avoid significant overhead from checking everything at the potential hook sites, but you must call
// AssayActions() after modifying ConditionalHooks before returning to normal operation of the scripting system!
void script_state::AssayActions() {
	ActiveActions.clear();

	for (const auto &hook : ConditionalHooks) {
		for (const auto &action : hook.Actions) {
			ActiveActions[action.action_type] = true;
		}
	}
}

bool script_state::IsActiveAction(int action_id) {
	auto entry = ActiveActions.find(action_id);
	if (entry != ActiveActions.end())
		return entry->second;
	else
		return false;
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
