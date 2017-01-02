
#include "TestUtil.h"

#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaException.h"
#include "scripting/lua/LuaConvert.h"

using namespace luacpp;

namespace {
int testCFunction(lua_State* L) {
	lua_pushnumber(L, 42.0);
	return 1;
}

int testErrorFunction(lua_State* L) {
	lua_pushliteral(L, "TestError");
	return 1;
}

int testErrorFunctionTwoRetVals(lua_State* L) {
	lua_pushliteral(L, "TestError1");
	lua_pushnumber(L, 0.0);
	return 2;
}

int testErrorFunctionNoRetVals(lua_State* L) {
	return 0;
}

int upvalueTest(lua_State* L) {
	auto upval = lua_upvalueindex(1);

	if (!lua_isstring(L, upval)) {
		lua_pushboolean(L, 0);
		return 1;
	}

	auto val = lua_tostring(L, upval);
	if (strcmp(val, "UpvalueTest")) {
		lua_pushboolean(L, 0);
		return 1;
	}

	// Value is valid
	lua_pushboolean(L, 1);
	return 1;
}
}

class LuaFunctionTest: public LuaStateTest {
};

TEST_F(LuaFunctionTest, CreateFromCode) {
	{
		ScopedLuaStackTest stackTest(L);

		ASSERT_THROW(LuaFunction::createFromCode(L, "/\\invalid"), luacpp::LuaException);
	}
	{
		ScopedLuaStackTest stackTest(L);

		ASSERT_NO_THROW(LuaFunction::createFromCode(L, "return 1 + 1"));
	}
}

TEST_F(LuaFunctionTest, Call) {
	{
		ScopedLuaStackTest stackTest(L);

		// Test execution with failure
		LuaFunction function = LuaFunction::createFromCode(L, "invalid()");

		ASSERT_THROW(function.call(), luacpp::LuaException);
		ASSERT_THROW(function(), luacpp::LuaException);
	}
	{
		ScopedLuaStackTest stackTest(L);

		// Test execution without failure
		LuaFunction function = LuaFunction::createFromCode(L, "local a = 1");

		ASSERT_NO_THROW(function.call());
		ASSERT_NO_THROW(function());
	}
	{
		ScopedLuaStackTest stackTest(L);

		// Test execution without failure
		LuaFunction function = LuaFunction::createFromCode(L, "return 'abc', 5");


		LuaValueList returnValues = function();

		ASSERT_EQ(luacpp::ValueType::STRING, returnValues[0].getValueType());
		ASSERT_EQ(luacpp::ValueType::NUMBER, returnValues[1].getValueType());

		ASSERT_STREQ("abc", returnValues[0].getValue<std::string>().c_str());
		ASSERT_EQ(5, returnValues[1].getValue<int>());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_getglobal(L, "type");

		LuaFunction func;
		ASSERT_TRUE(convert::popValue(L, func));

		LuaValue arg = LuaValue::createValue(L, "testString");
		LuaValueList returnVals = func({ arg });

		ASSERT_EQ(1, (int)returnVals.size());
		ASSERT_EQ(ValueType::STRING, returnVals[0].getValueType());
		ASSERT_STREQ("string", returnVals[0].getValue<std::string>().c_str());
	}
}

TEST_F(LuaFunctionTest, SetEnvironment) {
	{
		ScopedLuaStackTest stackTest(L);

		// Setup environment table
		LuaTable envionment = LuaTable::create(L);
		envionment.addValue("key", "Test");

		LuaFunction func = LuaFunction::createFromCode(L, "return key");

		func.setEnvironment(envionment);

		LuaValueList returnVals = func();

		ASSERT_EQ(1, (int)returnVals.size());
		ASSERT_EQ(ValueType::STRING, returnVals[0].getValueType());

		ASSERT_STREQ("Test", returnVals[0].getValue<std::string>().c_str());
	}
}

TEST_F(LuaFunctionTest, SetCFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		LuaFunction func = LuaFunction::createFromCFunction(L, testCFunction);

		LuaValueList retVals = func();

		ASSERT_EQ(1, (int)retVals.size());
		ASSERT_EQ(ValueType::NUMBER, retVals[0].getValueType());
		ASSERT_DOUBLE_EQ(42.0, retVals[0].getValue<double>());
	}
}

TEST_F(LuaFunctionTest, SetErrorFunction) {
	{
		ScopedLuaStackTest stackTest(L);

		LuaFunction func = LuaFunction::createFromCode(L, "invalid()");
		func.setErrorFunction(LuaFunction::createFromCFunction(L, &testErrorFunction));

		try {
			func();
			FAIL();
		}
		catch (const LuaException& err) {
			ASSERT_STREQ("TestError", err.what());
		}
	}
}

TEST_F(LuaFunctionTest, SetReference) {
	{
		ScopedLuaStackTest stackTest(L);

		LuaFunction func = LuaFunction::createFromCode(L, "invalid()");

		lua_pushliteral(L, "abc");
		ASSERT_THROW(func.setReference(UniqueLuaReference::create(L)), LuaException);

		lua_pop(L, 1);

		lua_getglobal(L, "type");
		ASSERT_NO_THROW(func.setReference(UniqueLuaReference::create(L)));

		lua_pop(L, 1);
	}
}

TEST_F(LuaFunctionTest, ErrorFunctionMultipleReturnValues) {
	ScopedLuaStackTest stackTest(L);

	LuaFunction func = LuaFunction::createFromCode(L, "invalid()");
	func.setErrorFunction(LuaFunction::createFromCFunction(L, &testErrorFunctionTwoRetVals));

	try {
		func();
		FAIL();
	}
	catch (const LuaException& err) {
		ASSERT_STREQ("TestError1", err.what());
	}
}

TEST_F(LuaFunctionTest, ErrorFunctionNoReturnValues) {
	ScopedLuaStackTest stackTest(L);

	LuaFunction func = LuaFunction::createFromCode(L, "invalid()");
	func.setErrorFunction(LuaFunction::createFromCFunction(L, &testErrorFunctionNoRetVals));

	try {
		func();
		FAIL();
	}
	catch (const LuaException& err) {
		ASSERT_STREQ("Invalid lua value on stack!", err.what());
	}
}

TEST_F(LuaFunctionTest, Upvalues) {
	ScopedLuaStackTest stackTest(L);

	LuaValue upval = LuaValue::createValue(L, "UpvalueTest");

	LuaFunction func = LuaFunction::createFromCFunction(L, upvalueTest, { upval });

	auto ret = func();

	ASSERT_EQ(1, (int)ret.size());
	ASSERT_TRUE(ret.front().is(ValueType::BOOLEAN));
	ASSERT_TRUE(ret.front().getValue<bool>());
}
