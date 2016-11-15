
#include "TestUtil.h"

#include "scripting/lua/LuaConvert.h"
#include "scripting/lua/LuaTable.h"
#include "scripting/lua/LuaFunction.h"

using namespace luacpp;
using namespace convert;

namespace {
int testCFunction(lua_State* state) {
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

	pushValue<LuaValue>(L, table);

	ASSERT_TRUE(lua_istable(L, -1) == 1);

	lua_pop(L, 1);
}

TEST_F(LuaConvertTest, PushFunction) {
	ScopedLuaStackTest stackTest(L);

	LuaFunction func = LuaFunction::createFromCode(L, "return 0");

	pushValue<LuaValue>(L, func);

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

		double target = popValue<double>(L);

		ASSERT_DOUBLE_EQ(1.0, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		double target = popValue<double>(L, -1, false);

		ASSERT_DOUBLE_EQ(1.0, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		ASSERT_THROW(popValue<double>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopFloat) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		float target;
		ASSERT_NO_THROW(target = popValue<float>(L));

		ASSERT_FLOAT_EQ(1.0f, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		float target;
		ASSERT_NO_THROW(target = popValue<float>(L, -1, false));

		ASSERT_FLOAT_EQ(1.0f, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		ASSERT_THROW(popValue<float>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopInt) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		int target;
		ASSERT_NO_THROW(target = popValue<int>(L));

		ASSERT_EQ(1, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		int target;
		ASSERT_NO_THROW(target = popValue<int>(L, -1, false));

		ASSERT_EQ(1, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		ASSERT_THROW(popValue<int>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopSizeT) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		size_t target;
		ASSERT_NO_THROW(target = popValue<size_t>(L));

		ASSERT_EQ(1, target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		size_t target;
		ASSERT_NO_THROW(target = popValue<size_t>(L, -1, false));

		ASSERT_EQ(1, target);
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		ASSERT_THROW(popValue<size_t>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopStdString) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		std::string target;
		ASSERT_NO_THROW(target = popValue<std::string>(L));

		ASSERT_STREQ("TestTest", target.c_str());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushliteral(L, "TestTest");

		std::string target;
		ASSERT_NO_THROW(target = popValue<std::string>(L, -1, false));

		ASSERT_STREQ("TestTest", target.c_str());
		ASSERT_TRUE(lua_isstring(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		ASSERT_THROW(popValue<std::string>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopBool) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		bool target;
		ASSERT_NO_THROW(target = popValue<bool>(L));

		ASSERT_TRUE(target);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		bool target;
		ASSERT_NO_THROW(target = popValue<bool>(L, -1, false));

		ASSERT_TRUE(target);
		ASSERT_TRUE(lua_isboolean(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		ASSERT_THROW(popValue<bool>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopCFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushcfunction(L, &testCFunction);

		lua_CFunction target;
		ASSERT_NO_THROW(target = popValue<lua_CFunction>(L));

		ASSERT_EQ(target, &testCFunction);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushcfunction(L, &testCFunction);

		lua_CFunction target;
		ASSERT_NO_THROW(target = popValue<lua_CFunction>(L, -1, false));

		ASSERT_EQ(target, &testCFunction);
		ASSERT_TRUE(lua_iscfunction(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		ASSERT_THROW(popValue<lua_CFunction>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaTable) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_newtable(L);

		LuaTable target;
		ASSERT_NO_THROW(target = popValue<LuaTable>(L));

		ASSERT_EQ(ValueType::TABLE, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_newtable(L);

		LuaTable target;
		ASSERT_NO_THROW(target = popValue<LuaTable>(L, -1, false));

		ASSERT_EQ(ValueType::TABLE, target.getValueType());
		ASSERT_TRUE(lua_istable(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		ASSERT_THROW(popValue<LuaTable>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		// Put the print function onto the stack
		lua_getglobal(L, "print");

		LuaFunction target;
		ASSERT_NO_THROW(target = popValue<LuaFunction>(L));

		ASSERT_EQ(ValueType::FUNCTION, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_getglobal(L, "print");

		LuaFunction target;
		ASSERT_NO_THROW(target = popValue<LuaFunction>(L, -1, false));

		ASSERT_EQ(ValueType::FUNCTION, target.getValueType());
		ASSERT_TRUE(lua_isfunction(L, -1) == 1);

		lua_pop(L, 1);
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		ASSERT_THROW(popValue<LuaFunction>(L), LuaException);

		lua_pop(L, 1);
	}
}

TEST_F(LuaConvertTest, PopLuaValue) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaValue target;
		ASSERT_NO_THROW(target = popValue<LuaValue>(L));

		ASSERT_EQ(ValueType::NUMBER, target.getValueType());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushnumber(L, 1.0);

		LuaValue target;
		ASSERT_NO_THROW(target = popValue<LuaValue>(L, -1, false));

		ASSERT_EQ(ValueType::NUMBER, target.getValueType());
		ASSERT_TRUE(lua_isnumber(L, -1) == 1);

		lua_pop(L, 1);
	}
}
