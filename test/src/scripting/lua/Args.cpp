
#include "TestUtil.h"

#include "scripting/lua/LuaArgs.h"

using namespace luacpp;
using namespace luacpp::args;

class LuaArgumentTest: public LuaStateTest {
};

TEST_F(LuaArgumentTest, GetArgs) {
	ScopedLuaStackTest stackTest(L);

	lua_pushliteral(L, "Abcdefg");

	std::string arg1;
	int arg2;

	int n = getArgs(L, arg1, optional, arg2);

	ASSERT_EQ(1, n);
	ASSERT_STREQ("Abcdefg", arg1.c_str());

	ASSERT_EQ(1, lua_gettop(L));

	lua_pop(L, 1);
}
