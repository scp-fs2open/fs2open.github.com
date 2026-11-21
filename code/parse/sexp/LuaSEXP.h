#pragma once

#include "parse/sexp/DynamicSEXP.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"

#include "parse/sexp.h"

namespace sexp {

class LuaSEXP : public DynamicSEXP {
protected:
	luacpp::LuaFunction _action;

	int _min_args;
	int _max_args;

	SCP_vector<std::pair<SCP_string, int>> _argument_types;			//!< These are the types of the static, non-repeating arguments
	SCP_vector<std::pair<SCP_string, int>> _varargs_type_pattern;	//!< This is the pattern for the variable argument part of the SEXP

	int _return_type = OPR_NULL;

	bool _run_on_clients = false; //!< In multiplayer, the server sends each call to the clients, which run it too
	mutable bool _warned_not_sent = false;

	int _category;
	int _subcategory;

	std::pair<SCP_string, int> getArgumentInternalType(int argnum) const;
	luacpp::LuaValue sexpToLua(int node, int argnum, int parent_node) const;
	bool argumentToText(int node, int argnum, const luacpp::LuaValue& value, SCP_string& text) const;
	bool sendToClients(const SCP_vector<SCP_string>& arg_text) const;
	void warnNotSent(const char* reason) const;

	// just a helper for parseTable
	static bool parseCheckEndOfDescription();

	// another helper
	static bool maybeExtractSexpSpecialRetVal(const luacpp::LuaValue& value, int& sexp_retval);

 public:
	static std::pair<SCP_string, int> get_parameter_type(const SCP_string& name);
	static int get_return_type(const SCP_string& name);

	explicit LuaSEXP(const SCP_string& name);

	void initialize() override;

	int getMinimumArguments() const override;

	int getMaximumArguments() const override;

	int getArgumentType(int argnum) const override;

	int execute(int node, int parent_node = -1) override;

	bool executeMulti() override;

	bool runsOnClients() const override;

	SCP_string getArgumentLayout() const override;

	int getReturnType() override;

	int getSubcategory() override;

	int getCategory() override;

	void parseTable();

	void setAction(const luacpp::LuaFunction& action);

	luacpp::LuaFunction getAction() const;
	int getSexpReturnValue(const luacpp::LuaValueList& retVals) const;

	luacpp::LuaValueList getSEXPArgumentList(int node,
		int parent_node = -1,
		SCP_vector<SCP_string>* arg_text = nullptr) const;
};

/**
 * @brief Writes one text argument the way a client's run_sexp reads it back
 * @return false if the text cannot be sent: it has a quote mark, starts with @, or is TOKEN_LENGTH or longer
 */
bool lua_sexp_quote_text(const char* value, SCP_string& text);

/**
 * @brief Whether argument text a multiplayer server sent for a Lua SEXP holds only plain values
 *
 * Plain values are quoted text in which every ! is doubled (shorter than TOKEN_LENGTH, not starting with @), whole
 * numbers of up to 10 digits, and ( true ) or ( false ), each followed by a space or the end of the text. Anything
 * else could carry other SEXPs into the client.
 * @param[out] value_count if not null, set to the number of values when the text is plain
 */
bool lua_sexp_arguments_are_plain(const SCP_string& text, int* value_count = nullptr);

}

