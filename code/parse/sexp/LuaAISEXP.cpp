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
bool LuaAISEXP::hasTarget() const {
	return needsTarget;
}

}
