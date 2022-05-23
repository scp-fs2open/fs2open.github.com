#include "ai/ailua.h"

#include "ai/aigoals.h"
#include "hud/hudsquadmsg.h"
#include "iff_defs/iff_defs.h"
#include "parse/sexp/sexp_lookup.h"
#include "parse/sexp/LuaAISEXP.h"
#include "scripting/api/objs/ai_helper.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/oswpt.h"
#include "scripting/api/objs/ship.h"
#include "scripting/scripting.h"

static std::unordered_map<int, ai_mode_lua> Lua_ai_modes;
static std::unordered_map<int, player_order_lua> Lua_player_orders;

void ai_lua_add_mode(int sexp_op, const ai_mode_lua& mode) {
	Lua_ai_modes.emplace(sexp_op, mode);
}

bool ai_lua_add_order(int sexp_op, player_order_lua order) {
	if (current_new_player_order_type == MAX_ENGINE_PLAYER_ORDER)
		return false;

	current_new_player_order_type = current_new_player_order_type >> 1;
	Player_orders.emplace_back(vm_strdup(order.parseText.c_str()), vm_strdup(order.displayText.c_str()), -1, current_new_player_order_type, sexp_op);
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
	luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_AI_Helper.Set(object_h(&Objects[Ships[aip->shipnum].objnum]))));
	if (lua_ai.needsTarget) {
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(aip->lua_ai_target)));
	}

	auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

	if (!retVals.empty() && retVals[0].getValueType() == luacpp::ValueType::BOOLEAN) {
		if (retVals[0].getValue<bool>())
			ai_mission_goal_complete(aip);
	}
}

void ai_lua(ai_info* aip){
	Assertion(aip->mode == AIM_LUA, "Tried to invoke LuaAI on a ship not in LuaAI-Mode");

	const auto& lua_ai = Lua_ai_modes.at(aip->submode);
	
	auto dynamicSEXP = sexp::get_dynamic_sexp(aip->submode);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getActionFrame();

		run_ai_lua_action(action, lua_ai, aip);
	}
}

void ai_lua_start(ai_goal* aigp, object* objp){

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	ai_info *aip = &Ai_info[Ships[objp->instance].ai_index];

	aip->mode = AIM_LUA;
	aip->submode = aigp->ai_submode;
	aip->lua_ai_target = aigp->lua_ai_target;
	aip->submode_start_time = Missiontime;
	
	const auto& lua_ai = Lua_ai_modes.at(aigp->ai_submode);

	auto dynamicSEXP = sexp::get_dynamic_sexp(aigp->ai_submode);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getActionEnter();

		run_ai_lua_action(action, lua_ai, aip);
	}
	
}

bool ai_lua_is_valid_target_intrinsic(int sexp_op, int target_objnum, ship* self) {
	ship* target = &Ships[Objects[target_objnum].instance];

	const player_order_lua& order = *ai_lua_find_player_order(sexp_op);
	switch (order.targetRestrictions) {
	case player_order_lua::target_restrictions::TARGET_ALL:
		return true;
	case player_order_lua::target_restrictions::TARGET_OWN:
		return target->team == self->team;
	case player_order_lua::target_restrictions::TARGET_ALLIES:
		return !iff_x_attacks_y(self->team, target->team);
	case player_order_lua::target_restrictions::TARGET_ENEMIES:
		return iff_x_attacks_y(self->team, target->team);
	case player_order_lua::target_restrictions::TARGET_SAME_WING:
		return target->wingnum != -1 && self->wingnum == target->wingnum;
	case player_order_lua::target_restrictions::TARGET_PLAYER_WING:
		return target->wingnum != -1 && Ships[Player_obj->instance].wingnum == target->wingnum;
	case player_order_lua::target_restrictions::TARGET_ALL_CAPS:
		return Ship_info[target->ship_info_index].is_big_ship();
	case player_order_lua::target_restrictions::TARGET_ALLIED_CAPS:
		return !iff_x_attacks_y(self->team, target->team) && Ship_info[target->ship_info_index].is_big_ship();
	case player_order_lua::target_restrictions::TARGET_ENEMY_CAPS:
		return iff_x_attacks_y(self->team, target->team) && Ship_info[target->ship_info_index].is_big_ship();
	case player_order_lua::target_restrictions::TARGET_NOT_SELF:
		return self->objnum != target_objnum;
	}

	return false;
}

bool ai_lua_is_valid_target_lua(const ai_mode_lua& mode, int sexp_op, int target_objnum, ship* self) {
	auto dynamicSEXP = sexp::get_dynamic_sexp(sexp_op);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getTargetRestrict();

		if (!action.isValid())
			return true;

		luacpp::LuaValueList luaParameters;
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_Ship.Set(object_h(&Objects[self->objnum]))));
		if (mode.needsTarget) {
			luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(object_ship_wing_point_team(&Ships[Objects[target_objnum].instance]))));
		}

		auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

		if (!retVals.empty() && retVals[0].getValueType() == luacpp::ValueType::BOOLEAN) {
			return retVals[0].getValue<bool>();
		}
		else {
			Error(LOCATION, "LuaAI target restriction function did not return a valid boolean!");
		}
	}

	return true;
}

bool ai_lua_is_valid_target(int sexp_op, int target_objnum, ship* self) {
	const ai_mode_lua& mode = *ai_lua_find_mode(sexp_op);

	//All targetless AI modes are fine
	if (mode.needsTarget) {

		//No target is then not valid
		if (target_objnum == -1)
			return false;

		//As of now, only accept ships
		if (Objects[target_objnum].type != OBJ_SHIP)
			return false;

		if (!ai_lua_is_valid_target_intrinsic(sexp_op, target_objnum, self))
			return false;
	}

	//If we haven't bailed yet, query the custom callback
	return ai_lua_is_valid_target_lua(mode, sexp_op, target_objnum, self);
}

int ai_lua_is_achievable(const ai_goal* aigp, int objnum){
	const auto& lua_ai = Lua_ai_modes.at(aigp->ai_submode);

	auto dynamicSEXP = sexp::get_dynamic_sexp(aigp->ai_submode);

	if (dynamicSEXP != nullptr && typeid(*dynamicSEXP) == typeid(sexp::LuaAISEXP)) {
		auto lua_ai_sexp = static_cast<sexp::LuaAISEXP*>(dynamicSEXP);
		const auto& action = lua_ai_sexp->getAchievable();

		if (!action.isValid())
			return AI_GOAL_ACHIEVABLE;

		luacpp::LuaValueList luaParameters;
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_Ship.Set(object_h(&Objects[objnum]))));
		if (lua_ai.needsTarget) {
			luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(aigp->lua_ai_target)));
		}

		auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

		if (!retVals.empty() && retVals[0].getValueType() == luacpp::ValueType::USERDATA) {
			scripting::api::enum_h enumData;
			retVals[0].getValue(scripting::api::l_Enum.Get(&enumData));
			if (!enumData.IsValid()) {
				Error(LOCATION, "LuaAI SEXP Availability hook returned an invalid avilability enum.");
				return AI_GOAL_ACHIEVABLE;
			}

			switch (enumData.index) {
			case scripting::api::LE_LUAAI_ACHIEVABLE:
				return AI_GOAL_ACHIEVABLE;
			case scripting::api::LE_LUAAI_NOT_YET_ACHIEVABLE:
				return AI_GOAL_NOT_KNOWN;
			case scripting::api::LE_LUAAI_UNACHIEVABLE:
				return AI_GOAL_NOT_ACHIEVABLE;
			default:
				Error(LOCATION, "LuaAI SEXP Availability hook returned an invalid avilability enum.");
				return AI_GOAL_ACHIEVABLE;
			}
		}
		else {
			Error(LOCATION, "LuaAI SEXP Availability hook did not return any avilability enum!");
		}
	}

	return AI_GOAL_ACHIEVABLE;
}