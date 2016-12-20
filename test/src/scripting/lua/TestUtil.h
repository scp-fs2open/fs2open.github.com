
#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#pragma once

#include "scripting/lua/LuaHeaders.h"

#include <gtest/gtest.h>

class LuaStateTest: public testing::Test {
 public:
	lua_State* L;

	virtual void SetUp() override {
		L = luaL_newstate();
		luaL_openlibs(L);
	}

	virtual void TearDown() override {
		lua_close(L);
	}
};

class ScopedLuaStackTest {
 private:
	lua_State* L;
	int top;

 public:
	ScopedLuaStackTest(lua_State* state) : L(state), top(-1) {
		top = lua_gettop(L);
	}

	void checkStack() {
		ASSERT_EQ(top, lua_gettop(L));
	}

	~ScopedLuaStackTest() {
		checkStack();
	}
};

#endif // TEST_UTIL_H
