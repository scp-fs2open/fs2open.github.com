
#include "TestUtil.h"

#include "scripting/lua/LuaValue.h"

using namespace luacpp;

class LuaValueTest: public LuaStateTest {};

TEST_F(LuaValueTest, SetReference) {
	ScopedLuaStackTest stackTest(L);

	LuaValue val = LuaValue::createValue(L, "TestTest");

	lua_pushnumber(L, 42.0);

	val.setReference(UniqueLuaReference::create(L));

	lua_pop(L, 1);

	ASSERT_EQ(ValueType::NUMBER, val.getValueType());
	ASSERT_DOUBLE_EQ(42.0, val.getValue<double>());
}
