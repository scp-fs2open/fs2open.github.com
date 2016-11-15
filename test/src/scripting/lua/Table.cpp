
#include "TestUtil.h"

#include "scripting/lua/LuaTable.h"

using namespace luacpp;

class LuaTableTest: public LuaStateTest {
};


TEST_F(LuaTableTest, AddValue) {
	ScopedLuaStackTest stackTest(L);

	auto top = lua_gettop(L);
	LuaTable table = LuaTable::create(L);
	table.addValue("key", "value");

	ASSERT_TRUE(table.pushValue());

	lua_getfield(L, -1, "key");

	ASSERT_TRUE(lua_isstring(L, -1) == 1);
	ASSERT_STREQ("value", lua_tostring(L, -1));

	lua_pop(L, 2); // Pop value and table
}

TEST_F(LuaTableTest, SetReference) {
	ScopedLuaStackTest stackTest(L);

	LuaTable table = LuaTable::create(L);

	lua_pushliteral(L, "abc");
	ASSERT_THROW(table.setReference(UniqueLuaReference::create(L)), LuaException);

	lua_pop(L, 1);

	lua_newtable(L);
	ASSERT_NO_THROW(table.setReference(UniqueLuaReference::create(L)));

	lua_pop(L, 1);
}

TEST_F(LuaTableTest, GetValue) {
	ScopedLuaStackTest stackTest(L);

	lua_newtable(L);
	lua_pushstring(L, "value");
	lua_setfield(L, -2, "key");

	LuaTable table;

	EXPECT_TRUE(convert::popValue(L, table));

	std::string val;
	ASSERT_TRUE(table.getValue("key", val));
	ASSERT_STREQ("value", val.c_str());
}

TEST_F(LuaTableTest, GetLength) {
	ScopedLuaStackTest stackTest(L);

	lua_newtable(L);

	lua_pushnumber(L, 1);
	lua_pushstring(L, "value1");
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushstring(L, "value2");
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushstring(L, "value3");
	lua_settable(L, -3);

	LuaTable table;

	EXPECT_TRUE(convert::popValue(L, table));

	ASSERT_EQ(3, table.getLength());
}

TEST_F(LuaTableTest, Iterator) {
	ScopedLuaStackTest stackTest(L);

	lua_newtable(L);

	lua_pushnumber(L, 1);
	lua_pushstring(L, "value1");
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushstring(L, "value2");
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushstring(L, "value3");
	lua_settable(L, -3);

	lua_pushstring(L, "value4");
	lua_setfield(L, -2, "key");

	LuaTable table;

	EXPECT_TRUE(convert::popValue(L, table));

	auto iter = table.iterator();

	int i = 0;
	while (iter.toNext()) {
		switch (i) {
			case 0: {
				ASSERT_STREQ("value1", lua_tostring(L, -1));
				lua_pop(L, 1);

				ASSERT_DOUBLE_EQ(1.0, lua_tonumber(L, -1));
				break;
			}
			case 1: {
				ASSERT_STREQ("value2", lua_tostring(L, -1));
				lua_pop(L, 1);

				ASSERT_DOUBLE_EQ(2.0, lua_tonumber(L, -1));
				break;
			}
			case 2: {
				ASSERT_STREQ("value3", lua_tostring(L, -1));
				lua_pop(L, 1);

				ASSERT_DOUBLE_EQ(3.0, lua_tonumber(L, -1));
				break;
			}
			case 3: {
				ASSERT_STREQ("value4", lua_tostring(L, -1));
				lua_pop(L, 1);

				ASSERT_STREQ("key", lua_tostring(L, -1));
				break;
			}
			default:
				FAIL();
				break;
		}
		++i;
	}
}
