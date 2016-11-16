
#include "lua/TestUtil.h"

#include "scripting/ade_args.h"

#include "scripting/lua/LuaFunction.h"

using namespace luacpp;
using namespace scripting;

class AdeArgsTest : public LuaStateTest {
};

TEST_F(AdeArgsTest, GetFunctionArg) {
	auto stack_func = LuaFunction::createFromCode(L, "return 1");
	stack_func.pushValue();

	Ade_get_args_lfunction = true;
	LuaFunction func;
	ASSERT_EQ(1, ade_get_args(L, "u", &func));
	Ade_get_args_lfunction = false;
}

TEST_F(AdeArgsTest, GetTableArg) {
	auto stack_table = LuaTable::create(L);
	stack_table.pushValue();

	Ade_get_args_lfunction = true;
	LuaTable table;
	ASSERT_EQ(1, ade_get_args(L, "t", &table));
	Ade_get_args_lfunction = false;
}

TEST_F(AdeArgsTest, SetFunctionArg) {
	auto stack_func = LuaFunction::createFromCode(L, "return 1");
	ASSERT_EQ(1, ade_set_args(L, "u", &stack_func));

	ASSERT_TRUE(lua_isfunction(L, -1));
}

TEST_F(AdeArgsTest, SetTableArg) {
	auto stack_table = LuaTable::create(L);
	ASSERT_EQ(1, ade_set_args(L, "t", &stack_table));

	ASSERT_TRUE(lua_istable(L, -1));
}
