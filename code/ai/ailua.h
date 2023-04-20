#pragma once

#include "ai/ai.h"
#include "ai/aigoals.h"
#include "mission/missionmessage.h"
#include "ship/ship.h"
#include "parse/sexp.h"
#include "globalincs/globals.h"

namespace sexp { class LuaAISEXP; }

struct ai_mode_lua {
	const sexp::LuaAISEXP& sexp;
	bool needsTarget;
	const char* hudText;
};

struct player_order_lua {
	int ai_message = MESSAGE_YESSIR;
	SCP_string parseText = "";
	SCP_string displayText = "";
	enum class target_restrictions : int { TARGET_ALLIES, TARGET_ALL, TARGET_OWN, TARGET_ENEMIES, TARGET_SAME_WING, TARGET_PLAYER_WING, TARGET_ALL_CAPS, TARGET_ALLIED_CAPS, TARGET_ENEMY_CAPS, TARGET_NOT_SELF } targetRestrictions = target_restrictions::TARGET_ALL;
	enum class ship_restrictions : int { ANY, WING, IN_PLAYER_WING, PLAYER_WING } shipRestrictions = ship_restrictions::ANY;
};


void ai_lua_add_mode(int sexp_op, const ai_mode_lua& mode);
bool ai_lua_add_order(int sexp_op, player_order_lua order);
bool ai_lua_has_mode(int sexp_op);
const ai_mode_lua* ai_lua_find_mode(int sexp_op);
const player_order_lua* ai_lua_find_player_order(int sexp_op);
void ai_lua(ai_info* aip);
void ai_lua_start(ai_goal* aigp, object* objp);
bool ai_lua_is_valid_target(int sexp_op, int target_objnum, ship* self, size_t order);
bool ai_lua_is_valid_ship(int sexp_op, bool isWing, ship* self);
ai_achievability ai_lua_is_achievable(const ai_goal* aigp, int objnum);