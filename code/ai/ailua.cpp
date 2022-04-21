#include "ai/ailua.h"

#include "ai/aigoals.h"
#include "hud/hudsquadmsg.h"
#include "parse/sexp/sexp_lookup.h"
#include "parse/sexp/LuaAISEXP.h"
#include "scripting/api/objs/oswpt.h"
#include "scripting/scripting.h"

static std::unordered_map<int, ai_mode_lua> Lua_ai_modes;
static std::unordered_map<int, player_order_lua> Lua_player_orders;

void ai_lua_add_mode(int sexp_op, ai_mode_lua mode) {
	Lua_ai_modes.emplace(sexp_op, std::move(mode));
}

bool ai_lua_add_order(int sexp_op, player_order_lua order) {
	if (current_highest_player_order_type == max_highest_player_order_type)
		return false;

	current_highest_player_order_type = current_highest_player_order_type << 1;
	Player_orders.emplace_back(vm_strdup(order.parseText.c_str()), vm_strdup(order.displayText.c_str()), -1, current_highest_player_order_type, sexp_op);
	Player_orders.back().localize();

	Lua_player_orders.emplace(sexp_op, std::move(order));

	return true;
}

bool ai_lua_has_mode(int sexp_op){
	return Lua_ai_modes.count(sexp_op) > 0;
}

const ai_mode_lua* ai_lua_find_mode(int sexp_op){
	auto aiLuaMode = Lua_ai_modes.find(sexp_op);
	
	if(aiLuaMode == Lua_ai_modes.end())
		return nullptr;
	else
		return &aiLuaMode->second;
}

const player_order_lua* ai_lua_find_player_order(int sexp_op) {
	auto aiLuaOrder = Lua_player_orders.find(sexp_op);

	if (aiLuaOrder == Lua_player_orders.end())
		return nullptr;
	else
		return &aiLuaOrder->second;
}

void run_ai_lua_action(const luacpp::LuaFunction& action, const ai_mode_lua& lua_ai, ai_info* aip) {
	if (!action.isValid()) {
		Error(LOCATION,
			"Lua AI SEXP called without a valid action function! A script probably failed to set the action for some reason.");
		return;
	}

	luacpp::LuaValueList luaParameters;
	if (lua_ai.needsTarget) {
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(aip->lua_ai_target)));
	}

	auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

	if (retVals.size() > 0 && retVals[0].getValueType() == luacpp::ValueType::BOOLEAN) {
		if (retVals[0].getValue<bool>())
			ai_mission_goal_complete(aip);
	}
}

void ai_lua(ship* shipp){
	
	ai_info	*aip = &Ai_info[shipp->ai_index];

	Assertion(aip->mode == AIM_LUA, "Tried to invoke LuaAI on a ship not in LuaAI-Mode");

	const auto& lua_ai = Lua_ai_modes.at(aip->submode);
	
	auto dynamicSEXP = sexp::get_dynamic_sexp(aip->submode);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getActionFrame();

		run_ai_lua_action(action, lua_ai, aip);
	}
}

void ai_lua_start(ai_goal* aip, object* objp){

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	ai_info *aiip = &Ai_info[Ships[objp->instance].ai_index];

	aiip->mode = AIM_LUA;
	aiip->submode = aip->ai_submode;
	aiip->lua_ai_target = aip->lua_ai_target;
	aiip->submode_start_time = Missiontime;
	
	const auto& lua_ai = Lua_ai_modes.at(aip->ai_submode);

	auto dynamicSEXP = sexp::get_dynamic_sexp(aip->ai_submode);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getActionEnter();

		run_ai_lua_action(action, lua_ai, aiip);
	}
	
}

bool ai_lua_is_valid_target(int sexp_op, int target_objnum) {
	const ai_mode_lua& mode = *ai_lua_find_mode(sexp_op);

	//All targetless AI modes are fine
	if (!mode.needsTarget)
		return true;

	//No target is then not valid
	if (target_objnum == -1)
		return false;

	//As of now, only accept ships
	if (Objects[target_objnum].type != OBJ_SHIP)
		return false;

	const player_order_lua& order = *ai_lua_find_player_order(sexp_op);



	return true;
}