//
//
#include "LuaSEXP.h"

#include "parse/parselo.h"
#include "parse/sexp/sexp_lookup.h"

#include "object/waypoint.h"
#include "iff_defs/iff_defs.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "mission/missionmessage.h"

#include "scripting/api/objs/sexpvar.h"
#include "scripting/api/objs/team.h"
#include "scripting/api/objs/waypoint.h"
#include "scripting/api/objs/ship.h"
#include "scripting/api/objs/shipclass.h"
#include "scripting/api/objs/weaponclass.h"
#include "scripting/api/objs/wing.h"
#include "scripting/api/objs/message.h"

using namespace luacpp;

namespace {

SCP_unordered_map<SCP_string, int> parameter_type_mapping{{ "boolean",      OPF_BOOL },
														  { "number",       OPF_NUMBER },
														  { "ship",         OPF_SHIP },
														  { "string",       OPF_STRING },
														  { "team",         OPF_IFF },
														  { "waypointpath", OPF_WAYPOINT_PATH },
														  { "variable",     OPF_VARIABLE_NAME },
														  { "message",      OPF_MESSAGE },
														  { "wing",         OPF_WING },
														  { "shipclass",    OPF_SHIP_CLASS_NAME },
														  { "weaponclass",  OPF_WEAPON_NAME }, };
int get_parameter_type(const SCP_string& name) {
	SCP_string copy = name;
	std::transform(copy.begin(), copy.end(), copy.begin(), [](char c) { return (char)::tolower(c); });

	auto iter = parameter_type_mapping.find(copy);
	if (iter == parameter_type_mapping.end()) {
		return -1;
	} else {
		return iter->second;
	}
}

SCP_unordered_map<SCP_string, int> return_type_mapping{{ "number",  OPR_NUMBER },
													   { "boolean", OPR_BOOL },
													   { "nothing", OPR_NULL }, };
int get_return_type(const SCP_string& name) {
	SCP_string copy = name;
	std::transform(copy.begin(), copy.end(), copy.begin(), [](char c) { return (char)::tolower(c); });

	auto iter = return_type_mapping.find(copy);
	if (iter == return_type_mapping.end()) {
		return -1;
	} else {
		return iter->second;
	}
}

int get_category(const SCP_string& name) {
	for (auto& subcat : op_menu) {
		if (subcat.name == name) {
			return subcat.id;
		}
	}

	return -1;
}

int get_subcategory(const SCP_string& name) {
	for (auto& subcat : op_submenu) {
		if (subcat.name == name) {
			return subcat.id;
		}
	}

	return -1;
}

}

namespace sexp {

LuaSEXP::LuaSEXP(const SCP_string& name) : DynamicSEXP(name) {
}
int LuaSEXP::getMinimumArguments() {
	return _min_args;
}
int LuaSEXP::getMaximumArguments() {
	return _max_args;
}
int LuaSEXP::getArgumentType(int argnum) const {
	if (argnum < 0) {
		return OPF_NONE;
	}

	if (argnum < (int) _argument_types.size()) {
		// Normal, non variable argument types
		return _argument_types[argnum];
	}

	// We are in the variable argument types region
	// First, adjust the argnum base index so that our variable index starts at 0
	auto varargs_index = argnum - _argument_types.size();

	// Then use modulo magic to bring the argument number into the right range
	varargs_index = varargs_index % _varargs_type_pattern.size();

	// And then use that to get the parameter type
	return _varargs_type_pattern[varargs_index];
}
luacpp::LuaValue LuaSEXP::sexpToLua(int node, int argnum) const {
	using namespace scripting::api;
	auto argtype = getArgumentType(argnum);

	switch (argtype) {
	case OPF_BOOL: {
		auto value = is_sexp_true(node) != 0;
		return LuaValue::createValue(_action.getLuaState(), value);
	}
	case OPF_NUMBER: {
		auto res = eval_num(node);
		float value;
		if (res == SEXP_NAN || res == SEXP_NAN_FOREVER) {
			value = std::numeric_limits<float>::quiet_NaN();
		} else {
			value = (float) res;
		}
		return LuaValue::createValue(_action.getLuaState(), value);
	}
	case OPF_VARIABLE_NAME: {
		// Variable names work by getting the variable index from the text node
		auto sexp_variable_index = atoi(Sexp_nodes[node].text);

		// verify variable set
		Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

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
		auto ship_id = ship_name_lookup(CTEXT(node));

		if (ship_id < 0) {
			// Name is invalid
			return LuaValue::createValue(_action.getLuaState(), l_Ship.Set(object_h()));
		}

		auto objp = &Objects[Ships[ship_id].objnum];

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
		auto name = CTEXT(node);

		return LuaValue::createValue(_action.getLuaState(), l_Wing.Set(wing_name_lookup(name)));
	}
	case OPF_SHIP_CLASS_NAME: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_Shipclass.Set(ship_info_lookup(name)));
	}
	case OPF_WEAPON_NAME: {
		auto name = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), l_Weaponclass.Set(weapon_info_lookup(name)));
	}
	case OPF_STRING: {
		auto text = CTEXT(node);
		return LuaValue::createValue(_action.getLuaState(), text);
	}
	default:
		UNREACHABLE("Unhandled argument type! Someone added an argument type but didn't add handling code to execute().");
		return LuaValue::createNil(_action.getLuaState());
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
int LuaSEXP::execute(int node) {
	if (!_action.isValid()) {
		Error(LOCATION,
			  "Lua SEXP called without a valid action function! A script probably failed to set the action for some reason.");
		return SEXP_CANT_EVAL;
	}

	LuaValueList luaParameters;

	// We need to adapt how we handle parameters based on their type. We use this variable to keep track of which parameter
	// we are currently looking at
	int argnum = 0;
	while (node != -1) {
		if (argnum < (int) _argument_types.size()) {
			// This is a parameter in the normal list so we add it to the normal parameter list
			luaParameters.push_back(sexpToLua(node, argnum));

			node = CDR(node);
			++argnum;
		} else {
			// The varargs part is handled in chunks so that scripts can use the data more easily
			// Every repeat pattern instance is put into its own table
			LuaTable varargs_part = LuaTable::create(_action.getLuaState());

			// Iterate over all parameters in this varargs pattern and put them all in the table
			// If we reach the end of the parameter list inside this for loop we just exit since all parameters are optional
			for (auto i = 0; i < (int)_varargs_type_pattern.size() && node != -1; ++i) {
				// i + 1 since lua arrays are 1-indexed
				varargs_part.addValue(i + 1, sexpToLua(node, argnum));

				node = CDR(node);
				++argnum;
			}

			luaParameters.push_back(varargs_part);
		}
	}

	// All parameters are now in LuaValues, time to call our function
	try {
		auto retVals = _action.call(luaParameters);

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
	if (_category == OP_CATEGORY_CHANGE) {
		// "Change" is a special case since we can't add new SEXPs to the primary category
		return OP_CATEGORY_CHANGE2;
	}
	return _category;
}
void LuaSEXP::parseTable() {
	required_string("$Category:");
	SCP_string category;
	stuff_string(category, F_NAME);

	_category = get_category(category);
	if (_category < 0) {
		error_display(0, "Invalid category '%s' found. New main categories can't be added!", category.c_str());
		_category = OP_CATEGORY_CHANGE; // Default to change2 so we have a valid value later on
	}

	required_string("$Subcategory:");
	SCP_string subcategory;
	stuff_string(subcategory, F_NAME);

	_subcategory = get_subcategory(subcategory);
	if (_subcategory < 0) {
		// Unknown subcategory so we need to add this one
		_subcategory = sexp::add_subcategory(_category, subcategory);
		if (_subcategory < 0) {
			// Couldn't add subcategory!
			error_display(0,
						  "No more space for subcategory '%s' free in category '%s'!",
						  subcategory.c_str(),
						  category.c_str());

			// Default to the first subcategory in our category, hopefully it will exist...
			_subcategory = 0x0000 | _category;
		}
	} else if ((_subcategory & OP_CATEGORY_MASK) != _category) {
		error_display(0,
					  "Subcategory '%s' is in a different category than the specified category! Subcategory names must be unique.",
					  subcategory.c_str());

		// Default to the first subcategory in our category, hopefully it will exist...
		_subcategory = 0x0000 | _category;
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

	SCP_string description;
	stuff_string(description, F_NAME);

	help_text << "\t" << description << "\r\n";

	bool variable_arg_part = false;
	if (optional_string("$Repeat")) {
		help_text << "Rest: (The following pattern repeats)\r\n";
		variable_arg_part = true;
	}

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
		if (type < 0) {
			error_display(0, "Unknown parameter type '%s'!", type_str.c_str());
			type = OPF_STRING;
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
	}

	_help_text = help_text.str();
}
void LuaSEXP::setAction(const luacpp::LuaFunction& action) {
	Assertion(action.isValid(), "Invalid function handle supplied!");
	_action = action;
}

luacpp::LuaFunction LuaSEXP::getAction() const {
	return _action;
}
}
