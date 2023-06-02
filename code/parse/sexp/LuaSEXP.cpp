//
//
#include "LuaSEXP.h"

#include "iff_defs/iff_defs.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "object/waypoint.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp/sexp_lookup.h"
#include "scripting/api/objs/hudgauge.h"
#include "scripting/api/objs/message.h"
#include "scripting/api/objs/model.h"
#include "scripting/api/objs/event.h"
#include "scripting/api/objs/oswpt.h"
#include "scripting/api/objs/sexpvar.h"
#include "scripting/api/objs/ship.h"
#include "scripting/api/objs/shipclass.h"
#include "scripting/api/objs/sound.h"
#include "scripting/api/objs/subsystem.h"
#include "scripting/api/objs/team.h"
#include "scripting/api/objs/waypoint.h"
#include "scripting/api/objs/weaponclass.h"
#include "scripting/api/objs/wing.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

using namespace luacpp;

namespace sexp {

static SCP_unordered_map<SCP_string, int> parameter_type_mapping{{ "boolean",      OPF_BOOL },
														  { "number",       OPF_NUMBER },
														  { "ship",         OPF_SHIP },
														  { "shipname",     OPF_SHIP },
														  { "string",       OPF_STRING },
														  { "team",         OPF_IFF },
														  { "waypointpath", OPF_WAYPOINT_PATH },
														  { "variable",     OPF_VARIABLE_NAME },
														  { "message",      OPF_MESSAGE },
														  { "wing",         OPF_WING },
														  { "shipclass",    OPF_SHIP_CLASS_NAME },
														  { "weaponclass",  OPF_WEAPON_NAME },
														  { "soundentry",   OPF_GAME_SND }, 
														  { "ship+waypoint",   OPF_SHIP_POINT },
														  { "ship+wing",   OPF_SHIP_WING },
														  { "ship+wing+team",   OPF_SHIP_WING_WHOLETEAM },
														  { "ship+wing+ship_on_team+waypoint",   OPF_SHIP_WING_SHIPONTEAM_POINT },
														  { "ship+wing+waypoint",   OPF_SHIP_WING_POINT },
														  { "ship+wing+waypoint+none",   OPF_SHIP_WING_POINT_OR_NONE },
														  { "subsystem",   OPF_SUBSYSTEM },
														  { "dockpoint",   OPF_DOCKER_POINT },
														  { "hudgauge",   OPF_ANY_HUD_GAUGE },
														  { "event",   OPF_EVENT_NAME },
														  { "enum",   First_available_opf_id } };

// If a parameter requires a parent parameter then it must be listed here!
static SCP_vector<SCP_string> parent_parameter_required{"subsystem", "dockpoint"};

std::pair<SCP_string, int> LuaSEXP::get_parameter_type(const SCP_string& name)
{
	SCP_string copy = name;
	SCP_tolower(copy);

	auto iter = parameter_type_mapping.find(copy);
	if (iter == parameter_type_mapping.end()) {
		return std::pair<SCP_string, int>(copy, -1);
	} else {
		return std::pair<SCP_string, int>(copy, iter->second);
	}
}

static SCP_unordered_map<SCP_string, int> return_type_mapping{{ "number",  OPR_NUMBER },
													   { "boolean", OPR_BOOL },
													   { "nothing", OPR_NULL }, };
int LuaSEXP::get_return_type(const SCP_string& name)
{
	SCP_string copy = name;
	SCP_tolower(copy);

	auto iter = return_type_mapping.find(copy);
	if (iter == return_type_mapping.end()) {
		return -1;
	} else {
		return iter->second;
	}
}

LuaSEXP::LuaSEXP(const SCP_string& name) : DynamicSEXP(name) {
}
void LuaSEXP::initialize() {
	// Nothing to do for this type
}
int LuaSEXP::getMinimumArguments() const {
	return _min_args;
}
int LuaSEXP::getMaximumArguments() const {
	return _max_args;
}
std::pair<SCP_string, int> LuaSEXP::getArgumentInternalType(int argnum) const {
	if (argnum < 0) {
		return std::pair<SCP_string, int>(SEXP_NONE_STRING, OPF_NONE);
	}

	if (argnum < (int) _argument_types.size()) {
		// Normal, non variable argument types
		return _argument_types[argnum];
	}

	// sanity check in case of bad table data
	if (_varargs_type_pattern.empty()) {
		Warning(LOCATION, "Not enough parameters specified for Lua SEXP %s!", getName().c_str());
		return std::pair<SCP_string, int>(SEXP_NONE_STRING, OPF_NONE);
	}

	// We are in the variable argument types region
	// First, adjust the argnum base index so that our variable index starts at 0
	auto varargs_index = argnum - _argument_types.size();

	// Then use modulo magic to bring the argument number into the right range
	varargs_index = varargs_index % _varargs_type_pattern.size();

	// And then use that to get the parameter type
	return _varargs_type_pattern[varargs_index];
}
int LuaSEXP::getArgumentType(int argnum) const {
	return getArgumentInternalType(argnum).second;
}
luacpp::LuaValue LuaSEXP::sexpToLua(int node, int argnum, int parent_node) const {
	using namespace scripting::api;
	auto argtype = getArgumentInternalType(argnum);

	switch (argtype.second) {
	case OPF_BOOL: {
		auto value = is_sexp_true(node) != 0;
		return LuaValue::createValue(_action.getLuaState(), value);
	}
	case OPF_NUMBER: {
		bool is_nan, is_nan_forever;
		auto res = eval_num(node, is_nan, is_nan_forever);
		float value;
		if (is_nan || is_nan_forever) {
			value = std::numeric_limits<float>::quiet_NaN();
		} else {
			value = (float) res;
		}
		return LuaValue::createValue(_action.getLuaState(), value);
	}
	case OPF_VARIABLE_NAME: {
		// Variable names work by getting the variable index from the text node
		auto sexp_variable_index = sexp_get_variable_index(node);

		// Add the variable to the parameter list as a SEXPVariable object handle
		return LuaValue::createValue(_action.getLuaState(), l_SEXPVariable.Set(sexpvar_h(sexp_variable_index)));
	}
	case OPF_IFF: {
		auto team_idx = iff_lookup(CTEXT(node));

		return LuaValue::createValue(_action.getLuaState(), l_Team.Set(team_idx));
	}
	case OPF_WAYPOINT_PATH: {
		waypoint_list *wp_list = find_matching_waypoint_list(CTEXT(node));

		return LuaValue::createValue(_action.getLuaState(), l_WaypointList.Set(waypointlist_h(wp_list)));
	}
		// The following argument types are all strings
	case OPF_SHIP: {
		auto ship_entry = eval_ship(node);

		// if this is a shipname type, we want the name of a valid ship but not the ship itself
		// (if the ship is not valid, return an empty string)
		if (argtype.first == "shipname") {
			return LuaValue::createValue(_action.getLuaState(), ship_entry ? ship_entry->name : "");
		}

		if (!ship_entry || ship_entry->status != ShipStatus::PRESENT) {
			// Name is invalid
			return LuaValue::createValue(_action.getLuaState(), l_Ship.Set(object_h()));
		}

		auto objp = ship_entry->objp;

		// The other SEXP code does not validate the object type so this should be safe
		Assertion(objp->type == OBJ_SHIP,
				  "Ship '%s' was found in the Ships array but has a different object type in the Objects array. Get a coder!",
				  CTEXT(node));

		return LuaValue::createValue(_action.getLuaState(), l_Ship.Set(object_h(objp)));
	}
	case OPF_MESSAGE: {
		auto name = CTEXT(node);

		auto idx = -1;
		for (int i = Num_builtin_messages; i < (int) Messages.size(); i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}

		return LuaValue::createValue(_action.getLuaState(), l_Message.Set(idx));
	}
	case OPF_WING: {
		auto wingp = eval_wing(node);
		int wingnum = static_cast<int>(wingp - Wings);

		return LuaValue::createValue(_action.getLuaState(), l_Wing.Set(wingnum));
	}
	case OPF_SHIP_CLASS_NAME: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_Shipclass.Set(ship_info_lookup(name)));
	}
	case OPF_WEAPON_NAME: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_Weaponclass.Set(weapon_info_lookup(name)));
	}
	case OPF_GAME_SND: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_SoundEntry.Set(sound_entry_h(gamesnd_get_by_name(name))));
	}
	case OPF_STRING: {
		auto text = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), text);
	}
	case OPF_SHIP_POINT:
	case OPF_SHIP_WING:
	case OPF_SHIP_WING_WHOLETEAM:
	case OPF_SHIP_WING_SHIPONTEAM_POINT:
	case OPF_SHIP_WING_POINT:
	case OPF_SHIP_WING_POINT_OR_NONE: {
		object_ship_wing_point_team oswpt;
		eval_object_ship_wing_point_team(&oswpt, node);
		return LuaValue::createValue(_action.getLuaState(), l_OSWPT.Set(oswpt));
	}
	case OPF_SUBSYSTEM: {
		auto name = CTEXT(node);

		//Use the parent node to get the index of the parameter we should
		//look at to find the parent object
		int index = get_dynamic_parameter_index(_name, argnum);

		if (index < 0)
			error_display(1, "Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
				argnum,
				_name.c_str());

		int this_node = parent_node;
		for (int i = 0; i <= index; i++) {
			this_node = CDR(this_node);
		}

		auto ship_entry = eval_ship(this_node);

		if (!ship_entry || ship_entry->status != ShipStatus::PRESENT) {
			// Name is invalid
			return LuaValue::createValue(_action.getLuaState(), l_Ship.Set(object_h()));
		}

		ship_subsys* ss = ship_get_subsys(ship_entry->shipp, name);
		
		return LuaValue::createValue(_action.getLuaState(), l_Subsystem.Set(ship_subsys_h(ship_entry->objp, ss)));
	}
	case OPF_DOCKER_POINT: {
		auto name = CTEXT(node);

		// Use the parent node to get the index of the parameter we should
		// look at to find the parent object
		int index = get_dynamic_parameter_index(_name, argnum);

		if (index < 0)
			error_display(1,
				"Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
				argnum,
				_name.c_str());

		int this_node = parent_node;
		for (int i = 0; i <= index; i++) {
			this_node = CDR(this_node);
		}

		auto ship_entry = eval_ship(this_node);
		if (!ship_entry || ship_entry->status != ShipStatus::PRESENT) {
			// Name is invalid
			return LuaValue::createValue(_action.getLuaState(), l_Ship.Set(object_h()));
		}

		auto docker_pm = model_get(Ship_info[ship_entry->shipp->ship_info_index].model_num);
		auto dockindex = model_find_dock_name_index(Ship_info[ship_entry->shipp->ship_info_index].model_num, name);

		return LuaValue::createValue(_action.getLuaState(), l_Dockingbay.Set(dockingbay_h(docker_pm, dockindex)));
  }
	case OPF_ANY_HUD_GAUGE: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_HudGauge.Set(hud_get_gauge(name)));
	}
	case OPF_EVENT_NAME: {
		auto name = CTEXT(node);

		int i;
		for (i = 0; i < (int)Mission_events.size(); i++) {
			if (!stricmp(Mission_events[i].name.c_str(), name))
				break;
			}
		return LuaValue::createValue(_action.getLuaState(), l_Event.Set(i));
	}
	default:
		if ((strcmp(argtype.first.c_str(), "enum")) == 0) {
			auto text = CTEXT(node);
			return LuaValue::createValue(_action.getLuaState(), text);
		} else {
			UNREACHABLE(
				"Unhandled argument type! Someone added an argument type but didn't add handling code to execute().");
			return LuaValue::createNil(_action.getLuaState());
		}
	}
}
int LuaSEXP::getSexpReturnValue(const LuaValueList& retVals) const {
	switch (_return_type) {
	case OPR_NUMBER:
		if (retVals.size() != 1) {
			Warning(LOCATION,
					"Wrong number of return values for Lua SEXP '%s'! Expected 1, got " SIZE_T_ARG ".",
					_name.c_str(),
					retVals.size());
			return 0;
		} else if (retVals[0].getValueType() != ValueType::NUMBER) {
			Warning(LOCATION, "Wrong return type detected for Lua SEXP '%s', expected a number.", _name.c_str());
			return 0;
		} else {
			return retVals[0].getValue<int>();
		}
	case OPR_BOOL:
		if (retVals.size() != 1) {
			Warning(LOCATION,
					"Wrong number of return values for Lua SEXP '%s'! Expected 1, got " SIZE_T_ARG ".",
					_name.c_str(),
					retVals.size());
			return SEXP_FALSE;
		} else if (retVals[0].getValueType() != ValueType::BOOLEAN) {
			Warning(LOCATION, "Wrong return type detected for Lua SEXP '%s', expected a boolean.", _name.c_str());
			return SEXP_FALSE;
		} else {
			return retVals[0].getValue<bool>() ? SEXP_TRUE : SEXP_FALSE;
		}
	case OPR_NULL:
		if (retVals.size() != 0) {
			Warning(LOCATION,
					"Wrong number of return values for Lua SEXP '%s'! Expected 0, got " SIZE_T_ARG ".",
					_name.c_str(),
					retVals.size());
		}
		return SEXP_TRUE;
	default:
		return SEXP_TRUE;
	}
}

luacpp::LuaValueList LuaSEXP::getSEXPArgumentList(int node, int parent_node) const {
	LuaValueList luaParameters;

	// We need to adapt how we handle parameters based on their type. We use this variable to keep track of which parameter
	// we are currently looking at
	int argnum = 0;
	while (node != -1) {
		if (argnum < (int)_argument_types.size()) {
			// This is a parameter in the normal list so we add it to the normal parameter list
			luaParameters.push_back(sexpToLua(node, argnum, parent_node));

			node = CDR(node);
			++argnum;
		}
		else {
			// The varargs part is handled in chunks so that scripts can use the data more easily
			// Every repeat pattern instance is put into its own table
			LuaTable varargs_part = LuaTable::create(_action.getLuaState());

			// Iterate over all parameters in this varargs pattern and put them all in the table
			// If we reach the end of the parameter list inside this for loop we just exit since all parameters are optional
			for (auto i = 0; i < (int)_varargs_type_pattern.size() && node != -1; ++i) {
				// i + 1 since lua arrays are 1-indexed
				varargs_part.addValue(i + 1, sexpToLua(node, argnum, parent_node));

				node = CDR(node);
				++argnum;
			}

			luaParameters.push_back(varargs_part);
		}
	}

	return luaParameters;
}

int LuaSEXP::execute(int node, int parent_node) {
	if (!_action.isValid()) {
		
		Error(LOCATION,
			  "Lua SEXP %s called without a valid action function! A script probably failed to set the action for some reason.", _name.c_str());
		return SEXP_CANT_EVAL;
	}

	LuaValueList luaParameters = getSEXPArgumentList(node, parent_node);

	// All parameters are now in LuaValues, time to call our function
	try {
		auto retVals = _action.call(Script_system.GetLuaSession(), luaParameters);

		return getSexpReturnValue(retVals);
	} catch(const LuaException&) {
		// This is only caused by an error in the function but those are already handled by our error function
		// These values are close to what the existing SEXP code does if errors occur while processing a SEXP
		switch(_return_type) {
		case OPR_NUMBER:
		case OPR_BOOL:
			return SEXP_NAN;
		case OPR_NULL:
			return SEXP_TRUE;
		default:
			return SEXP_NAN;
		}
	}
}
int LuaSEXP::getReturnType() {
	return _return_type;
}

int LuaSEXP::getSubcategory() {
	return _subcategory;
}
int LuaSEXP::getCategory() {
	return _category;
}
bool LuaSEXP::parseCheckEndOfDescription() {
	// Since we're stuffing strings, this is the best way to "unstuff" a string
	// if we determine we're finished with the description - and we also want
	// to preserve whitespace (which the parser eats) while building the description
	pause_parse();

	// look for any token that can follow $Description
	auto possible_tokens =
	{
		"$Operator:",
		"#End",
		"$Repeat",
		"$Parameter:"
	};

	bool found = false;
	for (auto token : possible_tokens)
	{
		if (optional_string(token))
		{
			found = true;
			break;
		}
	}

	unpause_parse();
	return found;
}
void LuaSEXP::parseTable() {
	required_string("$Category:");
	SCP_string category;
	stuff_string(category, F_NAME);

	_category = get_category(category);
	if (_category == OP_CATEGORY_NONE) {
		// Unknown category so we need to add this one
		_category = sexp::add_category(category);
	}

	required_string("$Subcategory:");
	SCP_string subcategory;
	stuff_string(subcategory, F_NAME);

	_subcategory = get_subcategory(subcategory, _category);
	if (_subcategory == OP_SUBCATEGORY_NONE) {
		// Unknown subcategory so we need to add this one
		_subcategory = sexp::add_subcategory(_category, subcategory);
	} 

	required_string("$Minimum Arguments:");

	stuff_int(&_min_args);

	if (_min_args < 0) {
		error_display(0, "Minimum argument number must be at least 0! Got %d.", _min_args);
		_min_args = 0;
	}

	if (optional_string("$Maximum Arguments:")) {
		stuff_int(&_max_args);

		if (_max_args < 0) {
			error_display(0, "Maximum argument number must be at least 0! Got %d.", _max_args);
			_max_args = 0;
		}
	} else {
		_max_args = INT_MAX;
	}

	if (_max_args < _min_args) {
		error_display(0, "Maximum argument number must be greater or equal to minimum number of arguments!");
		std::swap(_min_args, _max_args);
	}

	if (optional_string("$Return Type:")) {
		SCP_string type;
		stuff_string(type, F_NAME);

		_return_type = get_return_type(type);

		if (_return_type < 0) {
			error_display(0, "Unknown return type '%s'!", type.c_str());
			_return_type = OPR_NULL;
		}
	} else {
		_return_type = OPR_NULL;
	}

	SCP_stringstream help_text;
	help_text << _name << "\r\n";

	required_string("$Description:");

	SCP_string description, extra;
	stuff_string(description, F_NAME);
	while (!parseCheckEndOfDescription()) {
		while (skip_eoln()) {
			description += "\r\n";
		}
		stuff_string(extra, F_NAME);
		description += extra;
	}

	help_text << "\t" << description << "\r\n\r\n";

	// argument string
	help_text << "Takes ";
	if (_max_args == INT_MAX) {
		help_text << _min_args << " or more arguments:";
	} else {
		if (_max_args == 0) {
			help_text << "no arguments.";
		} else if (_min_args == _max_args) {
			help_text << _min_args << " argument";
			if (_min_args > 1) {
				help_text << "s";
			}
			help_text << ":";
		} else {
			help_text << _min_args << " to " << _max_args << " arguments:";
		}
	}
	help_text << "\r\n";

	bool variable_arg_part = false;
	if (optional_string("$Repeat")) {
		help_text << "Rest: (The following pattern repeats)\r\n";
		variable_arg_part = true;
	}
	int param_index = 0;
	while (optional_string("$Parameter:")) {

		required_string("+Description:");

		SCP_string param_desc;
		stuff_string(param_desc, F_NAME);

		if (variable_arg_part) {
			help_text << "\t" << _varargs_type_pattern.size() + 1;
		} else {
			help_text << "\t" << _argument_types.size() + 1;
		}
		help_text << ": " << param_desc << "\r\n";

		required_string("+Type:");
		SCP_string type_str;
		stuff_string(type_str, F_NAME);

		auto type = get_parameter_type(type_str);
		if (type.second < 0) {
			error_display(0, "Unknown parameter type '%s'!", type_str.c_str());
			type = get_parameter_type("string");
		}

		if ((strcmp(type.first.c_str(), "enum")) == 0) {
			
			required_string("+Enum Name:");

			SCP_string enum_name;
			stuff_string(enum_name, F_NAME);

			dynamic_sexp_enum_list thisList;
			int list_position = get_dynamic_enum_position(enum_name);
			bool new_enum = false;

			if (list_position >= 0) {
				type.second = list_position + (int)First_available_opf_id;

				//Do this just in case we're going to use this list as a template
				thisList.name = Dynamic_enums[list_position].name; 
				for (const SCP_string& enum_item : Dynamic_enums[list_position].list) {
					thisList.list.push_back(enum_item);
				}
			} else {
				new_enum = true;
				thisList.name = enum_name;
			}

			while (optional_string("+Enum:")) {
				new_enum = true;

				//If we're making a new Enum based off another one, let's give it a unique name
				if (list_position >= 0) {
					SCP_string newName;
					newName = enum_name;
					newName.append(std::to_string(list_position));
					thisList.name = newName;
				}

				SCP_string item;
				stuff_string(item, F_NAME);

				// These characters may not appear in an Enum item
				constexpr const char* ENUM_INVALID_CHARS = "()\"'\\/";
				if (std::strpbrk(item.c_str(), ENUM_INVALID_CHARS) != nullptr) {
					error_display(0, "ENUM item '%s' cannot include these characters [(,),\",',\\,/]. Skipping!\n", item.c_str());

					// Skip the invalid entry
					continue;

				}

				if (item.length() >= NAME_LENGTH) {
					error_display(0, "Enum item '%s' is longer than %i characters. Truncating!\n", item.c_str(), NAME_LENGTH);
					item.resize(NAME_LENGTH - 1);
				}

				bool skip = false;
				// Case insensitive check if the item already exists in the list
				for (int i = 0; i < (int)thisList.list.size(); i++) {
					if (lcase_equal(item, thisList.list[i])) {
						error_display(0, "Enum item '%s' already exists in list %s. Skipping!\n", item.c_str(), thisList.name.c_str());
						skip = true;
						break;
					}
				}

				if (skip)
					continue;

				thisList.list.push_back(item);
			}

			if (thisList.list.size() == 0) {
				error_display(0, "Parsed empty enum list '%s'\n", thisList.name.c_str());
				thisList.list.push_back("<none>");
			}

			if (new_enum) {
				type.second = increment_enum_list_id();
				Dynamic_enums.push_back(thisList);
			} else {
				// Not an error but large mods may lose track of their enum names and I thought this would be helpful -Mjn
				mprintf(("Found previously existing Lua Enum '%s'. Using that for sexp '%s'!\n",
					enum_name.c_str(),
					_name.c_str()));
			}

		}
		bool parent_required = false;
		for (int i = 0; i < (int)parent_parameter_required.size(); i++) {
			if (type.first == parent_parameter_required[i]) {
				parent_required = true;
				break;
			}

		}
		if (parent_required)
		{
			int this_index = -1;
			
			if (optional_string("+Parent Parameter Index:")) {
				stuff_int(&this_index);
				this_index--;
			}

			if (this_index >= (param_index + 1)) {
				error_display(1,
					"Ship Parameter Index '%i' cannot be greater or equal to %i (the current parameter index)!\n",
					this_index,
					param_index + 1);
			}
			
			std::pair<int, int> param_map(param_index, this_index);

			// check if this operator already has an entry for dynamic parameters
			int dyn_index = -1;
			for (int i = 0; i < (int)Dynamic_parameters.size(); i++) {
				if (lcase_equal(Dynamic_parameters[i].operator_name, _name)) {
					dyn_index = i;
				}
			}

			if (dyn_index >= 0) {
				Dynamic_parameters[dyn_index].parameter_map.push_back(param_map);
			} else {
				dynamic_sexp_parameter_list dyn_param;
				dyn_param.operator_name = _name;
				dyn_param.parameter_map.push_back(param_map);

				Dynamic_parameters.push_back(dyn_param);
			}

		}

		if (variable_arg_part) {
			_varargs_type_pattern.push_back(type);
		} else {
			_argument_types.push_back(type);
		}

		if (optional_string("$Repeat")) {
			if (!variable_arg_part) {
				help_text << "Rest: (The following pattern repeats)\r\n";
				variable_arg_part = true;
			} else {
				error_display(0, "A second $Repeat has been encountered! Only one may appear in the parameter list.");
			}
		}
		param_index++;
	}

	_help_text = help_text.str();
	lcl_replace_stuff(_help_text, true);
}
void LuaSEXP::setAction(const luacpp::LuaFunction& action) {
	Assertion(action.isValid(), "Invalid function handle supplied!");
	_action = action;
}

luacpp::LuaFunction LuaSEXP::getAction() const {
	return _action;
}
}
