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
#include "libs/jansson.h"
#include "mission/missioncampaign.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

#include <cstdarg>
#include <cstdio>

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

	bool isOverridable() const override { return true; }
};

// clang-format off
static USED_VARIABLE SCP_vector<std::shared_ptr<BuiltinHook>> Script_actions
{
	std::make_shared<BuiltinHook>("On Game Init",			CHA_GAMEINIT ),
	std::make_shared<BuiltinHook>("On Splash Screen",		CHA_SPLASHSCREEN ),
	std::make_shared<BuiltinHook>("On State Start",			CHA_ONSTATESTART ),
	std::make_shared<BuiltinHook>("On Action",				CHA_ONACTION ),
	std::make_shared<BuiltinHook>("On Action Stopped",		CHA_ONACTIONSTOPPED ),
	std::make_shared<BuiltinHook>("On Key Pressed",			CHA_KEYPRESSED ),
	std::make_shared<BuiltinHook>("On Key Released",		CHA_KEYRELEASED ),
	std::make_shared<BuiltinHook>("On Mouse Moved",			CHA_MOUSEMOVED ),
	std::make_shared<BuiltinHook>("On Mouse Pressed",		CHA_MOUSEPRESSED ),
	std::make_shared<BuiltinHook>("On Mouse Released",		CHA_MOUSERELEASED ),
	std::make_shared<BuiltinHook>("On State End",			CHA_ONSTATEEND ),
	std::make_shared<BuiltinHook>("On Mission Start",		CHA_MISSIONSTART ),
	std::make_shared<BuiltinHook>("On HUD Draw",			CHA_HUDDRAW ),
	std::make_shared<BuiltinHook>("On Ship Collision",		CHA_COLLIDESHIP ),
	std::make_shared<BuiltinHook>("On Weapon Collision",	CHA_COLLIDEWEAPON ),
	std::make_shared<BuiltinHook>("On Debris Collision",	CHA_COLLIDEDEBRIS ),
	std::make_shared<BuiltinHook>("On Asteroid Collision",	CHA_COLLIDEASTEROID ),
	std::make_shared<BuiltinHook>("On Object Render",		CHA_OBJECTRENDER ),
	std::make_shared<BuiltinHook>("On Death",				CHA_DEATH ),
	std::make_shared<BuiltinHook>("On Mission End",			CHA_MISSIONEND ),
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
	std::make_shared<BuiltinHook>("On Pain Flash",			CHA_PAINFLASH ),
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

HookVariableDocumentation::HookVariableDocumentation(const char* name_, ade_type_info type_, const char* description_)
	: name(name_), type(std::move(type_)), description(description_)
{
}

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
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

static void json_doc_generate_class(json_t* elObj, const DocumentationElementClass* lib)
{
	json_object_set_new(elObj, "superClass", json_string(lib->superClass.c_str()));
}
static json_t* json_doc_generate_return_type(const scripting::ade_type_info& type_info)
{
	switch (type_info.getType()) {
	case ade_type_info_type::Empty:
		return json_string("void");
	case ade_type_info_type::Simple:
		return json_string(type_info.getSimpleName());
	case ade_type_info_type::Tuple: {
		json_t* tupleTypes = json_array();

		for (const auto& type : type_info.elements()) {
			json_array_append_new(tupleTypes, json_doc_generate_return_type(type));
		}

		return json_pack("{ssso}", "type", "tuple", "elements", tupleTypes);
	}
	case ade_type_info_type::Array: {
		return json_pack("{ssso}",
		                 "type",
		                 "list",
		                 "element",
		                 json_doc_generate_return_type(type_info.elements().front()));
	}
	default:
		UNREACHABLE("Unknown type type!");
		return nullptr;
	}

}
static void json_doc_generate_function(json_t* elObj, const DocumentationElementFunction* lib)
{
	json_object_set_new(elObj, "returnType", json_doc_generate_return_type(lib->returnType));
	json_object_set_new(elObj, "parameters", json_string(lib->parameters.c_str()));
	json_object_set_new(elObj, "returnDocumentation", json_string(lib->returnDocumentation.c_str()));
}
static void json_doc_generate_property(json_t* elObj, const DocumentationElementProperty* lib)
{
	json_object_set_new(elObj, "getterType", json_doc_generate_return_type(lib->getterType));
	json_object_set_new(elObj, "setterType", json_string(lib->setterType.c_str()));
	json_object_set_new(elObj, "returnDocumentation", json_string(lib->returnDocumentation.c_str()));
}
static json_t* json_doc_generate_elements(const SCP_vector<std::unique_ptr<DocumentationElement>>& elements);
static json_t* json_doc_generate_element(const std::unique_ptr<DocumentationElement>& element)
{
	json_t* elementsObj = json_object();

	json_object_set_new(elementsObj, "name", json_string(element->name.c_str()));
	json_object_set_new(elementsObj, "shortName", json_string(element->shortName.c_str()));
	json_object_set_new(elementsObj, "description", json_string(element->description.c_str()));

	switch (element->type) {
	case ElementType::Library:
		json_object_set_new(elementsObj, "type", json_string("library"));
		break;
	case ElementType::Class:
		json_object_set_new(elementsObj, "type", json_string("class"));
		json_doc_generate_class(elementsObj, static_cast<DocumentationElementClass*>(element.get()));
		break;
	case ElementType::Function:
		json_object_set_new(elementsObj, "type", json_string("function"));
		json_doc_generate_function(elementsObj, static_cast<DocumentationElementFunction*>(element.get()));
		break;
	case ElementType::Operator:
		json_object_set_new(elementsObj, "type", json_string("operator"));
		json_doc_generate_function(elementsObj, static_cast<DocumentationElementFunction*>(element.get()));
		break;
	case ElementType::Property:
		json_object_set_new(elementsObj, "type", json_string("property"));
		json_doc_generate_property(elementsObj, static_cast<DocumentationElementProperty*>(element.get()));
		break;
	default:
		json_object_set_new(elementsObj, "type", json_string("unknown"));
		break;
	}

	json_object_set_new(elementsObj, "children", json_doc_generate_elements(element->children));

	return elementsObj;
}

static json_t* json_doc_generate_elements(const SCP_vector<std::unique_ptr<DocumentationElement>>& elements)
{
	json_t* elementsArray = json_array();

	for (const auto& el : elements) {
		json_array_append_new(elementsArray, json_doc_generate_element(el));
	}

	return elementsArray;
}

static json_t* json_doc_generate_param_element(const HookVariableDocumentation& param)
{
	return json_pack("{s:s,s:s,s:o}",
					 "name",
					 param.name,
					 "description",
					 param.description,
					 "type",
					 json_doc_generate_return_type(param.type));
}

static json_t* json_doc_generate_action_element(const DocumentationAction& action)
{
	json_t* paramArray = json_array();

	for (const auto& param : action.parameters) {
		json_array_append_new(paramArray, json_doc_generate_param_element(param));
	}

	return json_pack("{s:s,s:s,s:o,s:b}",
					 "name",
					 action.name.c_str(),
					 "description",
					 action.description.c_str(),
					 "hookVars",
					 paramArray,
					 "overridable",
					 action.overridable);
}

static void documentation_to_json(const ScriptingDocumentation& doc)
{
	std::unique_ptr<json_t> root(json_object());
	// Should be incremented every time an incompatible change is made
	json_object_set_new(root.get(), "version", json_integer(2));

	{
		json_t* actionArray = json_array();

		for (const auto& action : doc.actions) {
			json_array_append_new(actionArray, json_doc_generate_action_element(action));
		}

		json_object_set_new(root.get(), "actions", actionArray);
	}
	{
		json_t* conditionArray = json_array();

		for (const auto& cond : doc.conditions) {
			json_array_append_new(conditionArray, json_string(cond.c_str()));
		}

		json_object_set_new(root.get(), "conditions", conditionArray);
	}
	{
		json_t* enumObject = json_object();

		for (const auto& enumVal : doc.enumerations) {
			json_object_set_new(enumObject, enumVal.name.c_str(), json_integer(enumVal.value));
		}

		json_object_set_new(root.get(), "enums", enumObject);
	}
	{
		json_t* globalVariables = json_array();

		for (const auto& param : doc.globalVariables) {
			json_array_append_new(globalVariables, json_doc_generate_param_element(param));
		}

		json_object_set_new(root.get(), "globalVars", globalVariables);
	}
	json_object_set_new(root.get(), "elements", json_doc_generate_elements(doc.elements));

	const auto jsonStr = json_dump_string(root.get(), JSON_INDENT(2) | JSON_SORT_KEYS);

	std::ofstream outStr("scripting.json");
	outStr << jsonStr;
}

// Initializes the (global) scripting system, as well as any subsystems.
// script_close is handled by destructors
void script_init()
{
	mprintf(("SCRIPTING: Beginning initialization sequence...\n"));

	mprintf(("SCRIPTING: Beginning Lua initialization...\n"));
	Script_system.CreateLuaState();

	if (Output_scripting_meta || Output_scripting_json) {
		const auto doc = Script_system.OutputDocumentation();

		if (Output_scripting_meta) {
			mprintf(("SCRIPTING: Outputting scripting metadata...\n"));
			Script_system.OutputMeta("scripting.html");
		}
		if (Output_scripting_json) {
			mprintf(("SCRIPTING: Outputting scripting metadata in JSON format...\n"));
			documentation_to_json(doc);
		}
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
						if (objp == nullptr || objp->type != OBJ_WEAPON)
							return false;
					} else {

						// Okay, if we're still here, then objp is both valid and a ship
						ship* shipp = &Ships[objp->instance];
						bool primary = false, secondary = false, prev_primary = false, prev_secondary = false;
						switch (action) {
							case CHA_ONWPSELECTED:
								if (shipp->weapons.current_primary_bank >= 0)
									primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
								if (shipp->weapons.current_secondary_bank >= 0)
									secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								
								if (!(primary || secondary))
									return false;

								if ((shipp->flags[Ship::Ship_Flags::Primary_linked]) && primary && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags[Weapon::Info_Flags::Nolink]))
									return false;
								
								break;
							case CHA_ONWPDESELECTED:
								if (shipp->weapons.current_primary_bank >= 0)
									primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].name, scp->data.name) == 0;
								if (shipp->weapons.previous_primary_bank >= 0)
									prev_primary = stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.previous_primary_bank]].name, scp->data.name) == 0;
								if (shipp->weapons.current_secondary_bank >= 0)
									secondary = stricmp(Weapon_info[shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank]].name, scp->data.name) == 0;
								if (shipp->weapons.previous_secondary_bank >= 0)
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
									if (!equipped && (shipp->weapons.primary_bank_weapons[j] >= 0) && (shipp->weapons.primary_bank_weapons[j] < weapon_info_size()) ) {
										if ( !stricmp(Weapon_info[shipp->weapons.primary_bank_weapons[j]].name, scp->data.name) ) {
											equipped = true;
											break;
										}
									}
								}
							
								if (!equipped) {
									for(int j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++) {
										if (!equipped && (shipp->weapons.secondary_bank_weapons[j] >= 0) && (shipp->weapons.secondary_bank_weapons[j] < weapon_info_size()) ) {
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
							default:
								break;
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
	if (LuaState == nullptr) {
		return;
	}

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
	if (LuaState != nullptr)
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

	if (LuaState == nullptr) {
		return num;
	}

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

ScriptingDocumentation script_state::OutputDocumentation()
{
	ScriptingDocumentation doc;

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

	if (Langs & SC_LUA) {
		OutputLuaDocumentation(doc);
	}

	return doc;
}

template <typename Container>
static void output_hook_variable_list(FILE* fp, const Container& vars)
{
	fputs("<dl>", fp);
	for (const auto& param : vars) {
		fputs("<dt>", fp);
		ade_output_type_link(fp, param.type);
		fprintf(fp, " <i>%s</i></dt>", param.name);
		fprintf(fp, "<dd><b>Description:</b> %s</dt>", param.description);
	}
	fputs("</dl>", fp);
}

int script_state::OutputMeta(const char* filename)
{
	FILE* fp = fopen(filename, "w");
	int i;

	if (fp == nullptr) {
		return 0;
	}

	fprintf(fp, "<html>\n<head>\n\t<title>Script Output - FSO v%s (%s)</title>\n</head>\n", FS_VERSION_FULL, StateName);
	fputs("<body>", fp);
	fprintf(fp, "\t<h1>Script Output - FSO v%s (%s)</h1>\n", FS_VERSION_FULL, StateName);

	// Scripting languages links
	fputs("<dl>", fp);

	//***Hooks
	fputs("<dt><h2>Conditional Hooks</h2></dt>", fp);
	fputs("<dd><dl>", fp);

	// Conditions
	fputs("<dt><b>Conditions</b></dt>", fp);
	for (i = 0; i < Num_script_conditions; i++) {
		fprintf(fp, "<dd>%s</dd>", Script_conditions[i].name);
	}

	// Actions
	fputs("<dt><b>Actions</b></dt>", fp);
	fputs("<dd><dl>", fp);
	auto sortedHooks = scripting::getHooks();
	std::sort(sortedHooks.begin(),
			  sortedHooks.end(),
			  [](const scripting::HookBase* left, const scripting::HookBase* right) {
				  return left->getHookName() < right->getHookName();
			  });
	for (const auto& hook : sortedHooks) {
		fprintf(fp, "<dt><b>%s</b></dt>", hook->getHookName().c_str());
		if (!hook->getDescription().empty()) {
			fprintf(fp, "<dd><b>Description:</b> %s", hook->getDescription().c_str());
			// Only write this for hooks with descriptions since this information is useless for legacy hooks
			if (hook->isOverridable()) {
				fputs("<br><i>This hook is overridable</i>", fp);
			} else {
				fputs("<br><i>This hook is <b>not</b> overridable</i>", fp);
			}
			if (!hook->getParameters().empty()) {
				fputs("<br><b>Hook Variables:</b>", fp);
				output_hook_variable_list(fp, hook->getParameters());
			}
			fputs("</dd>", fp);
		}
	}
	fputs("</dl></dd>", fp);

	fputs("<dt><b>Global Hook Variables:</b> (set globally independent of a specific hook)</dt>", fp);
	output_hook_variable_list(fp, GlobalVariables);
	fputs("</dl></dd><br />", fp);

	//***Scripting langs
	fputs("<dt><h2>Scripting languages</h2></dt>", fp);
	if (Langs & SC_LUA) {
		fputs("<dd><a href=\"#Lua\">Lua</a></dd>", fp);
	}
	fputs("</dl>", fp);

	// Languages
	fputs("<dl>", fp);
	if (Langs & SC_LUA) {
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
	for (int i = 0; i < Num_script_conditions; i++) {
		sprintf(buf, "$%s:", Script_conditions[i].name);
		if (optional_string(buf))
			return Script_conditions[i].def;
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
		char* buf = (char*)vm_malloc(strlen(filename) + action->getHookName().size() + 4);
		sprintf(buf, "%s - %s", filename, action->getHookName().c_str());

		ParseChunk(&sat.hook, buf);

		// Free error string
		vm_free(buf);

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
