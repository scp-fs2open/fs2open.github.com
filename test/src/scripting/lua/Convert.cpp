
#include "TestUtil.h"

#include "scripting/lua/LuaConvert.h"
#include "scripting/lua/LuaTable.h"
#include "scripting/lua/LuaFunction.h"

using namespace luacpp;
using namespace convert;

namespace {
int testCFunction(lua_State*  /*state*/) {
	return 0;
}
}

class LuaConvertTest: public LuaStateTest {
};

TEST_F(LuaConvertTest, PushDouble) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, 1.0);

	ASSERT_TRUE(lua_isnumber(L, -1) == 1);
	ASSERT_DOUBLE_EQ(1.0, lua_tonumber(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushFloat) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, 1.0f);

	ASSERT_TRUE(lua_isnumber(L, -1) == 1);
	ASSERT_DOUBLE_EQ(1.0, lua_tonumber(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushInt) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, 1);

	ASSERT_TRUE(lua_isnumber(L, -1) == 1);
	ASSERT_DOUBLE_EQ(1.0, lua_tonumber(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushSizeT) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, static_cast<size_t>(1));

	ASSERT_TRUE(lua_isnumber(L, -1) == 1);
	ASSERT_DOUBLE_EQ(1.0, lua_tonumber(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushStdString) {
	ScopedLuaStackTest stackTest(L);

	std::string testStr("TestTest");
	pushValue(L, testStr);

	ASSERT_TRUE(lua_isstring(L, -1) == 1);
	ASSERT_STREQ("TestTest", lua_tostring(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushString) {
	ScopedLuaStackTest stackTest(L);

	const char* string = "TestTest";
	pushValue(L, string);

	ASSERT_TRUE(lua_isstring(L, -1) == 1);
	ASSERT_STREQ(string, lua_tostring(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushBool) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, true);

	ASSERT_TRUE(lua_isboolean(L, -1) == 1);
	ASSERT_TRUE(lua_toboolean(L, -1) == 1);

	lua_pop(L, 1);

	pushValue(L, false);

	ASSERT_TRUE(lua_isboolean(L, -1) == 1);
	ASSERT_FALSE(lua_toboolean(L, -1) == 1);

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushCFunction) {
	ScopedLuaStackTest stackTest(L);

	pushValue(L, &testCFunction);

	ASSERT_TRUE(lua_iscfunction(L, -1) == 1);

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushValue) {
	ScopedLuaStackTest stackTest(L);

	LuaTable table = LuaTable::create(L);

	pushValue(L, table);

	ASSERT_TRUE(lua_istable(L, -1) == 1);

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushFunction) {
	ScopedLuaStackTest stackTest(L);

	LuaFunction func = LuaFunction::createFromCode(L, "return 0");

	pushValue(L, func);

	ASSERT_TRUE(lua_isfunction(L, -1) == 1 && !lua_iscfunction(L, -1));

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PopDouble) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		double target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_DOUBLE_EQ(1.0, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		double target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_DOUBLE_EQ(1.0, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		double target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		double target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_DOUBLE_EQ(1.0, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		double target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_DOUBLE_EQ(1.0, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		double target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopFloat) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		float target = -1.f;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_FLOAT_EQ(1.0f, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		float target = -1.f;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_FLOAT_EQ(1.0f, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		float target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopInt) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		int target = -1;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(1, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		int target = -1;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(1, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		int target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopSizeT) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		size_t target = SIZE_MAX;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(1, (int)target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		size_t target = SIZE_MAX;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(1, (int)target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		size_t target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopStdString) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		std::string target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_STREQ("TestTest", target.c_str());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		std::string target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_STREQ("TestTest", target.c_str());
		ASSERT_TRUE(lua_isstring(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		std::string target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopBool) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		bool target = false;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_TRUE(target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		bool target = false;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_TRUE(target);
		ASSERT_TRUE(lua_isboolean(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		bool target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopCFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushcfunction(L, &testCFunction);

		lua_CFunction target = nullptr;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(target, &testCFunction);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushcfunction(L, &testCFunction);

		lua_CFunction target = nullptr;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(target, &testCFunction);
		ASSERT_TRUE(lua_iscfunction(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		lua_CFunction target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaTable) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_newtable(L);

		LuaTable target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(ValueType::TABLE, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_newtable(L);

		LuaTable target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(ValueType::TABLE, target.getValueType());
		ASSERT_TRUE(lua_istable(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaTable target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		// Put the print function onto the stack
		lua_getglobal(L, "print");

		LuaFunction target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(ValueType::FUNCTION, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_getglobal(L, "print");

		LuaFunction target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(ValueType::FUNCTION, target.getValueType());
		ASSERT_TRUE(lua_isfunction(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaFunction target;
		ASSERT_FALSE(popValue(L, target));

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaValue) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaValue target;
		ASSERT_TRUE(popValue(L, target));

		ASSERT_EQ(ValueType::NUMBER, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaValue target;
		ASSERT_TRUE(popValue(L, target, -1, false));

		ASSERT_EQ(ValueType::NUMBER, target.getValueType());
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
}
