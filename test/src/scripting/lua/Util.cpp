
#include <vector>

#include "TestUtil.h"

#include "scripting/lua/LuaUtil.h"

using namespace luacpp;

class LuaUtilTest: public LuaStateTest {};

TEST_F(LuaUtilTest, TableListPairs) {
	ScopedLuaStackTest stackTest(L);

	LuaTable table = LuaTable::create(L);
	table.addValue(1, "test1");
	table.addValue(2, "test2");
	table.addValue("key", "test3");

	std::vector<std::pair<std::string, std::string>> pairList;
	util::tableListPairs(table, pairList);

	ASSERT_EQ(3, (int)pairList.size());
	ASSERT_TRUE(std::find(pairList.begin(), pairList.end(), std::make_pair<std::string, std::string>("1", "test1"))
					!= pairList.end());
	ASSERT_TRUE(std::find(pairList.begin(), pairList.end(), std::make_pair<std::string, std::string>("2", "test2"))
					!= pairList.end());
	ASSERT_TRUE(std::find(pairList.begin(), pairList.end(), std::make_pair<std::string, std::string>("key", "test3"))
					!= pairList.end());
}

TEST_F(LuaUtilTest, TableToList) {
	ScopedLuaStackTest stackTest(L);

	LuaTable table = LuaTable::create(L);
	table.addValue(1, "test1");
	table.addValue(2, "test2");
	table.addValue("key", "test3");

	std::vector<std::string> valueList;
	util::tableToList(table, valueList);

	ASSERT_EQ(2, (int)valueList.size());
	ASSERT_TRUE(valueList[0] == "test1");
	ASSERT_TRUE(valueList[1] == "test2");
}

TEST_F(LuaUtilTest, GetValueName) {
	ScopedLuaStackTest stackTest(L);

	ASSERT_STREQ("none", luacpp::util::getValueName(ValueType::NONE));
	ASSERT_STREQ("nil", luacpp::util::getValueName(ValueType::NIL));
	ASSERT_STREQ("boolean", luacpp::util::getValueName(ValueType::BOOLEAN));
	ASSERT_STREQ("light userdata", luacpp::util::getValueName(ValueType::LIGHTUSERDATA));
	ASSERT_STREQ("string", luacpp::util::getValueName(ValueType::STRING));
	ASSERT_STREQ("number", luacpp::util::getValueName(ValueType::NUMBER));
	ASSERT_STREQ("table", luacpp::util::getValueName(ValueType::TABLE));
	ASSERT_STREQ("function", luacpp::util::getValueName(ValueType::FUNCTION));
	ASSERT_STREQ("userdata", luacpp::util::getValueName(ValueType::USERDATA));
	ASSERT_STREQ("thread", luacpp::util::getValueName(ValueType::THREAD));
}
