#include "ai/ailua.h"

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
	Player_orders.emplace_back(order.parseText, order.displayText, -1, sexp_op);
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

int ai_lua_get_num_general_orders() {
	int count = 0;
	
	for (const auto& order : Lua_player_orders) {
		if (order.second.generalOrder) {
			count++;
		}
	}

	return count;
}

SCP_vector<SCP_string> ai_lua_get_general_order_categories(bool enabled_only)
{
	SCP_vector<SCP_string> list;

	for (const auto& order : Lua_player_orders) {
		const SCP_string &cat = order.second.category;

		// If it's not a general order then move on
		if (!order.second.generalOrder) {
			continue;
		}

		// If the order is not enabled then move on
		if (enabled_only && !order.second.cur_enabled) {
			continue;
		}

		Assertion((cat.length() > 0), "Lua general orders category name must be longer than 0 characters!");

		for (size_t i = 0; i <= list.size(); i++) {

			// reached the end of the list, so add it
			if (i == list.size()) {
				list.push_back(cat);
				break;
			}

			// if it's already in the list then don't add it
			if (cat == list[i]) {
				break;
			}
		}
	}

	return list;
}

// Provides a list of general orders for mission saving. You can only get one kind
// of list at a time. If onlyEnabled and onlyValid are both true, for example, you will get
// a list of onlyEnabled orders. If you need a list of enabled orders that are also valid, you'll
// need to get both lists separately and compare.
SCP_vector<SCP_string> ai_lua_get_general_orders(bool onlyEnabled, bool onlyValid, const SCP_string& category)
{
	SCP_vector<SCP_string> list;

	for (const auto& order : Lua_player_orders) {
		if (order.second.generalOrder) {
			bool add = false;

			// All general orders
			if (!onlyEnabled && !onlyValid && category.empty()) {
				add = true;
			} else {

				// Only enabled orders
				if (onlyEnabled && order.second.cur_enabled) {
					add = true;

					// Only valid orders
				} else if (onlyValid && order.second.cur_valid) {
					add = true;

					// Only orders that match the category requested
				} else if ((!category.empty()) && (order.second.category == category)) {
					add = true;
				}
			}

			if (add) {
				list.push_back(order.second.parseText);
			}
		}
	}

	return list;
}

int ai_lua_find_general_order_id(const SCP_string& name) {
	for (const auto& order : Lua_player_orders) {
		if (order.second.parseText == name) {
			return order.first;
		}
	}

	return -1;
}

void ai_lua_enable_general_order(int sexp_op, bool enable) {
	auto aiLuaOrder = Lua_player_orders.find(sexp_op);

	if (aiLuaOrder == Lua_player_orders.end())
		return;
	else
		aiLuaOrder->second.cur_enabled = enable;
}

void ai_lua_validate_general_order(int sexp_op, bool validity)
{
	auto aiLuaOrder = Lua_player_orders.find(sexp_op);

	if (aiLuaOrder == Lua_player_orders.end())
		return;
	else
		aiLuaOrder->second.cur_valid = validity;
}

void ai_lua_reset_general_orders() {
	for (const auto& order : Lua_player_orders) {
		ai_lua_enable_general_order(order.first, false);
		ai_lua_validate_general_order(order.first, false);
	}
}

bool ai_lua_is_general_order(int sexp_op) {
	auto aiLuaOrder = Lua_player_orders.find(sexp_op);

	if (aiLuaOrder == Lua_player_orders.end())
		return false;

	return aiLuaOrder->second.generalOrder;
}

void run_ai_lua_action(const luacpp::LuaFunction& action, const ai_mode_lua& lua_ai, ai_info* aip) {
	if (!action.isValid()) {
		Error(LOCATION,
			"Lua AI SEXP called without a valid action function! A script probably failed to set the action for some reason.");
		return;
	}

	luacpp::LuaValueList luaParameters;
	if (aip->shipnum >= 0){
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_AI_Helper.Set(object_h(&Objects[Ships[aip->shipnum].objnum]))));
	} else {
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_AI_Helper.Set(object_h())));
	}
	if (lua_ai.needsTarget) {
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(aip->lua_ai_target.target)));
	}
	for (const auto& additionalParam : aip->lua_ai_target.arguments) {
		luaParameters.push_back(additionalParam);
	}

	auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

	if (!retVals.empty() && retVals[0].getValueType() == luacpp::ValueType::BOOLEAN) {
		if (retVals[0].getValue<bool>()) {

			// If we don't have a ship then it's a general order and we have nothing else to do
			if (aip->shipnum >= 0) {
				ai_mission_goal_complete(aip);
			}
		}
	}
}

void ai_lua(ai_info* aip){
	Assertion(aip->mode == AIM_LUA, "Tried to invoke LuaAI on a ship not in LuaAI-Mode");

	const auto& lua_ai = Lua_ai_modes.at(aip->submode);
	
	const auto& lua_ai_sexp = lua_ai.sexp;
	const auto& action = lua_ai_sexp.getAction();

	run_ai_lua_action(action, lua_ai, aip);
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

	const auto& lua_ai_sexp = lua_ai.sexp;
	const auto& action = lua_ai_sexp.getActionEnter();

	run_ai_lua_action(action, lua_ai, aip);
}

// For sending general orders that don't run on a ship.
// Essentially just runs arbitrary lua script when the command is called
void ai_lua_start_general(int lua_sexp_id, int target_objnum)
{
	//Create a dummy aip to pass the target data
	ai_info aip;
	aip.shipnum = -1;

	if (target_objnum >= 0 && Objects[target_objnum].type == OBJ_SHIP) {
		aip.lua_ai_target = {object_ship_wing_point_team(&Ships[Objects[target_objnum].instance]), {}};
	}
	
	const auto& lua_ai = ai_lua_find_mode(lua_sexp_id);

	const auto& lua_ai_sexp = lua_ai->sexp;
	const auto& action = lua_ai_sexp.getActionEnter();

	run_ai_lua_action(action, *lua_ai, &aip);
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

bool ai_lua_is_valid_target_lua(const ai_mode_lua& mode, int target_objnum, ship* self) {

	const auto& action = mode.sexp.getTargetRestrict();

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

	return true;
}

bool ai_lua_is_valid_target(int sexp_op, int target_objnum, ship* self, size_t order) {
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

		// check if this order can be issued against the target
		ship *shipp = &Ships[Objects[target_objnum].instance];
		if (shipp->orders_allowed_against.find(order) == shipp->orders_allowed_against.end()) {
			return false;
		}
	}

	//If we haven't bailed yet, query the custom callback
	return ai_lua_is_valid_target_lua(mode, target_objnum, self);
}

bool ai_lua_is_valid_ship(int sexp_op, bool isWing, ship* self)
{
	const player_order_lua& order = *ai_lua_find_player_order(sexp_op);

	switch (order.shipRestrictions) {
	case player_order_lua::ship_restrictions::ANY:
		return true;
	case player_order_lua::ship_restrictions::WING:
		return isWing;
	case player_order_lua::ship_restrictions::IN_PLAYER_WING:
		return self->wingnum != -1 && Ships[Player_obj->instance].wingnum == self->wingnum;
	case player_order_lua::ship_restrictions::PLAYER_WING:
		return isWing && self->wingnum != -1 && Ships[Player_obj->instance].wingnum == self->wingnum;
	}

	return false;
}

ai_achievability ai_lua_is_achievable(const ai_goal* aigp, int objnum){
	const auto& lua_ai = Lua_ai_modes.at(aigp->ai_submode);

	const auto& action = lua_ai.sexp.getAchievable();

	if (!action.isValid())
		return ai_achievability::ACHIEVABLE;

	luacpp::LuaValueList luaParameters;
	luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_Ship.Set(object_h(&Objects[objnum]))));
	if (lua_ai.needsTarget) {
		luaParameters.push_back(luacpp::LuaValue::createValue(action.getLuaState(), scripting::api::l_OSWPT.Set(aigp->lua_ai_target.target)));
	}
	for (const auto& additionalParam : aigp->lua_ai_target.arguments) {
		luaParameters.push_back(additionalParam);
	}

	auto retVals = action.call(Script_system.GetLuaSession(), luaParameters);

	if (!retVals.empty() && retVals[0].getValueType() == luacpp::ValueType::USERDATA) {
		scripting::api::enum_h enumData;
		retVals[0].getValue(scripting::api::l_Enum.Get(&enumData));
		if (!enumData.isValid()) {
			Error(LOCATION, "LuaAI SEXP achievability hook returned an invalid avilability enum.");
			return ai_achievability::ACHIEVABLE;
		}

		switch (enumData.index) {
		case scripting::api::LE_LUAAI_ACHIEVABLE:
			return ai_achievability::ACHIEVABLE;
		case scripting::api::LE_LUAAI_NOT_YET_ACHIEVABLE:
			return ai_achievability::NOT_KNOWN;
		case scripting::api::LE_LUAAI_UNACHIEVABLE:
			return ai_achievability::NOT_ACHIEVABLE;
		default:
			Error(LOCATION, "LuaAI SEXP achievability hook returned an invalid avilability enum.");
			return ai_achievability::ACHIEVABLE;
		}
	}
	else {
		Error(LOCATION, "LuaAI SEXP achievability hook did not return any avilability enum!");
	}

	return ai_achievability::ACHIEVABLE;
}