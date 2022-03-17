#pragma once

#include "ai/ai.h"
#include "ship/ship.h"

struct ai_mode_lua {
	bool needsTarget;
};

struct player_order_lua {
	int ai_submode;
};

void ai_lua_add_mode(int sexp_op, ai_mode_lua mode);
bool ai_lua_has_mode(int sexp_op);
const ai_mode_lua* ai_lua_find_mode(int sexp_op);
void ai_lua(ship* shipp);
void ai_lua_start(ai_goal* aip, object* objp);