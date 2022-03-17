#pragma once

#include "parse/sexp/DynamicSEXP.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"

#include "parse/sexp.h"

namespace sexp {

class LuaAISEXP : public DynamicSEXP {
	
	int _arg_type = -1;
	bool needsTarget = false;

	luacpp::LuaFunction _actionEnter;
	luacpp::LuaFunction _actionFrame;
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
	bool hasTarget() const;

	void parseTable();

	void setActionEnter(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getActionEnter() const;

	void setActionFrame(const luacpp::LuaFunction& action);
	luacpp::LuaFunction getActionFrame() const;
};

}

