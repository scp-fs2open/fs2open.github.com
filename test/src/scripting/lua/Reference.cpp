
#include "TestUtil.h"

#include "scripting/lua/LuaReference.h"

using namespace luacpp;

class LuaReferenceTest: public LuaStateTest {};

TEST_F(LuaReferenceTest, Create) {
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);
		lua_pushnumber(L, 42.0);

		auto top = lua_gettop(L);

		auto ref = UniqueLuaReference::create(L, -2);

		// This may not modify the stack!
		ASSERT_EQ(top, lua_gettop(L));

		// Remove the two values again
		lua_pop(L, 2);

		ref->pushValue();

		ASSERT_TRUE(lua_isboolean(L, -1) == 1);
		ASSERT_TRUE(lua_toboolean(L, -1) == 1);

		lua_pop(L, 1);
	}
}

TEST_F(LuaReferenceTest, IsValid) {
	{
		ScopedLuaStackTest stackTest(L);

		UniqueLuaReference ref;

		ASSERT_FALSE(ref.isValid());
	}
	{
		ScopedLuaStackTest stackTest(L);

		lua_pushboolean(L, 1);

		LuaReference refPtr = UniqueLuaReference::create(L);

		lua_pop(L, 1);

		ASSERT_TRUE(refPtr->isValid());

		refPtr->removeReference();

		ASSERT_FALSE(refPtr->isValid());
	}
}

TEST_F(LuaReferenceTest, RemoveReference) {
	ScopedLuaStackTest stackTest(L);

	lua_pushboolean(L, 1);

	LuaReference refPtr = UniqueLuaReference::create(L);

	lua_pop(L, 1);

	ASSERT_TRUE(refPtr->removeReference());

	ASSERT_FALSE(refPtr->removeReference());
}

TEST_F(LuaReferenceTest, PushValue) {
	ScopedLuaStackTest stackTest(L);

	lua_pushboolean(L, 1);

	LuaReference refPtr = UniqueLuaReference::create(L);

	lua_pop(L, 1);

	refPtr->pushValue();

	ASSERT_TRUE(lua_isboolean(L, -1) == 1);
	ASSERT_EQ(1, lua_toboolean(L, -1));

	lua_pop(L, 1);
}
