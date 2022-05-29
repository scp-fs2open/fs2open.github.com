#pragma once

#include "ai/ailua.h"

#include "parse/sexp/DynamicSEXP.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"

#include "parse/sexp.h"

namespace sexp {

class LuaAISEXP : public DynamicSEXP {
	
	int _arg_type = -1;
	bool needsTarget = false;
	const char* hudText = nullptr;

	luacpp::LuaFunction _actionEnter;
	luacpp::LuaFunction _actionFrame;
	luacpp::LuaFunction _achievable;
	luacpp::LuaFunction _targetRestrict;

	std::unique_ptr<player_order_lua> playerOrder = nullptr;

	// just a helper for parseTable
	static bool parseCheckEndOfDescription();

 public:
	explicit LuaAISEXP(const SCP_string& name);

	void initialize() override;
	int getMinimumArguments() override;
	int getMaximumArguments() override;
	int getArgumentType(int argnum) const override;
	int execute(int node) override;
	int getReturnType() override;
	int getSubcategory() override;
	int getCategory() override;

	void parseTable();

	void setActionEnter(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getActionEnter() const;

	void setActionFrame(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getActionFrame() const;

	void setAchievable(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getAchievable() const;

	void setTargetRestrict(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getTargetRestrict() const;


	void registerAIMode(int sexp_id) const;
	void maybeRegisterPlayerOrder(int sexp_id) const;
};

}

