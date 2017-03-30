#pragma once

#include "parse/sexp/DynamicSEXP.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"

#include "parse/sexp.h"

namespace sexp {

class LuaSEXP : public DynamicSEXP {
	luacpp::LuaFunction _action;

	int _min_args;
	int _max_args;

	SCP_vector<int> _argument_types; //!< These are the types of the static, non-repeating arguments
	SCP_vector<int> _varargs_type_pattern; //!< This is the pattern for the variable argument part of the SEXP

	int _return_type = OPR_NULL;

	int _category;
	int _subcategory;

	luacpp::LuaValue sexpToLua(int node, int argnum) const;
 public:
	explicit LuaSEXP(const SCP_string& name);

	int getMinimumArguments() override;

	int getMaximumArguments() override;

	int getArgumentType(int argnum) const override;

	int execute(int node) override;

	int getReturnType() override;

	int getSubcategory() override;

	int getCategory() override;

	void parseTable();

	void setAction(const luacpp::LuaFunction& action);

	luacpp::LuaFunction getAction() const;
	int getSexpReturnValue(const luacpp::LuaValueList& retVals) const;
};

}

