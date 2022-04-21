//
//
#include "LuaAISEXP.h"

#include "localization/localize.h"
#include "parse/parselo.h"
#include "parse/sexp/LuaSEXP.h"

using namespace luacpp;

namespace sexp {
static const SCP_unordered_set<int> allowed_oswpt_parameters{ OPF_SHIP, OPF_WING, OPF_SHIP_POINT, OPF_SHIP_WING, OPF_SHIP_WING_WHOLETEAM, OPF_SHIP_WING_SHIPONTEAM_POINT, OPF_SHIP_WING_POINT, OPF_SHIP_WING_POINT_OR_NONE };

LuaAISEXP::LuaAISEXP(const SCP_string& name) : DynamicSEXP(name) {
}

void LuaAISEXP::initialize() {};

int LuaAISEXP::getMinimumArguments() {	return _arg_type == -1 ? 1 : 2;};

int LuaAISEXP::getMaximumArguments() {	return getMinimumArguments();};

int LuaAISEXP::getArgumentType(int argnum) const {	return argnum == 0 && _arg_type != -1 ? _arg_type : OPF_POSITIVE;};

int LuaAISEXP::execute(int node) {
	UNREACHABLE("Tried to execute AI Lua SEXP %s! AI-Goal SEXPs should never be run.", _name.c_str());
	return SEXP_CANT_EVAL;
}

int LuaAISEXP::getReturnType() {	return OPR_AI_GOAL;};

int LuaAISEXP::getSubcategory() {	return getCategory();};

int LuaAISEXP::getCategory() {	return OP_CATEGORY_AI;};

void LuaAISEXP::parseTable() {
	SCP_stringstream help_text;
	help_text << _name << "\r\n";

	required_string("$Description:");

	SCP_string description;
	stuff_string(description, F_NAME);

	help_text << "\t" << description << "\r\n";

	if (optional_string("+HUD String:")) {
		SCP_string hudTextParse;
		stuff_string(hudTextParse, F_NAME);
		hudText = vm_strdup(hudTextParse.c_str());
	}

	int paramNum = 1;
	if (optional_string("$Target Parameter:")) {
		needsTarget = true;
		required_string("+Description:");

		SCP_string param_desc;
		stuff_string(param_desc, F_NAME);

		help_text << "\t1";
		help_text << ": " << param_desc << "\r\n";

		required_string("+Type:");
		SCP_string type_str;
		stuff_string(type_str, F_NAME);

		auto type = LuaSEXP::get_parameter_type(type_str);
		if (type.second < 0 || allowed_oswpt_parameters.count(type.second) <= 0) {
			error_display(0, "Parameter type '%s' is not an valid target type!", type_str.c_str());
			type = LuaSEXP::get_parameter_type("ship");
		}

		_arg_type = type.second;

		paramNum++;
	}

	if (optional_string("$Player Order:")) {
		playerOrder = std::unique_ptr<player_order_lua>(new player_order_lua());
		auto &order = *playerOrder;
		if (_arg_type != OPF_SHIP && _arg_type != -1) {
			error_display(1, "Player orders must have either no target or a ship-type target parameter!");
		}

		required_string("+Display String:");
		stuff_string(order.displayText, F_NAME);

		order.parseText = order.displayText;
		if (optional_string("+Parse String:")) {
			stuff_string(order.parseText, F_NAME);
		}

		if (optional_string("+Target Restrictions:")) {
			int result = optional_string_one_of(8, "All", "Own Team", "Allies", "Hostiles", "Player Wing", "Capitals", "Allied Capitals", "Enemy Capitals");
			if (result == -1) {
				error_display(0, "Unknown target restriction for player order %s. Assuming \"All\".", order.displayText.c_str());
				order.targetRestrictions = player_order_lua::target_restrictions::TARGET_ALL;
			}
			else {
				order.targetRestrictions = static_cast<player_order_lua::target_restrictions>(result);
			}
		}

		//TODO implement proper parsing for message types
		/*if (optional_string("+Acknowledge Message:")) {
			int result = optional_string_one_of(1, "");
			if (result == -1) {
				error_display(0, "Unknown ackn message for player order %s. Assuming \"Yessir\".", order.displayText.c_str());
				order.ai_message = MESSAGE_YESSIR;
			}
			else {
				order.ai_message = result;
			}
		}*/

	}

	help_text << "\t" << paramNum;
	help_text << ": Goal priority (number between 0 and 200. Player orders have a priority of 90-100).\r\n";

	_help_text = help_text.str();
	lcl_replace_stuff(_help_text, true);
}

void LuaAISEXP::setActionEnter(const luacpp::LuaFunction& action) {
	_actionEnter = action;
}
luacpp::LuaFunction LuaAISEXP::getActionEnter() const {
	return _actionEnter;
}

void LuaAISEXP::setActionFrame(const luacpp::LuaFunction& action) {
	_actionFrame = action;
}
luacpp::LuaFunction LuaAISEXP::getActionFrame() const {
	return _actionFrame;
}

void LuaAISEXP::registerAIMode(int sexp_id) const {
	ai_lua_add_mode(sexp_id, { needsTarget, hudText });
}

void LuaAISEXP::maybeRegisterPlayerOrder(int sexp_id) const {
	if (playerOrder == nullptr)
		return;
	ai_lua_add_order(sexp_id, *playerOrder);
}

}
