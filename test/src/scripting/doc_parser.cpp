
#include "scripting/doc_parser.h"

#include <gtest/gtest.h>

using namespace scripting;

struct ArgumentListParserTest : public testing::Test {
	argument_list_parser parser;

	ArgumentListParserTest() : parser({"number", "string", "object"}) {}
};

TEST_F(ArgumentListParserTest, parseEmpty) {
	ASSERT_TRUE(parser.parse(""));
}

TEST_F(ArgumentListParserTest, singleTypeParam) {
	ASSERT_TRUE(parser.parse("number"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));
	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
}

TEST_F(ArgumentListParserTest, alternativeTypeParam) {
	ASSERT_TRUE(parser.parse("number|string"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));
	ASSERT_EQ("", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);

	ASSERT_EQ(ade_type_info_type::Alternative, arglist[0].type.getType());
	const auto& elemTypes = arglist[0].type.elements();
	ASSERT_EQ(2, static_cast<int>(elemTypes.size()));
	ASSERT_STREQ("number", elemTypes[0].getIdentifier());
	ASSERT_STREQ("string", elemTypes[1].getIdentifier());
}

TEST_F(ArgumentListParserTest, alternativeTypeParamWith3) {
	ASSERT_TRUE(parser.parse("number|string|object"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));
	ASSERT_EQ("", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);

	ASSERT_EQ(ade_type_info_type::Alternative, arglist[0].type.getType());
	const auto& elemTypes = arglist[0].type.elements();
	ASSERT_EQ(3, static_cast<int>(elemTypes.size()));
	ASSERT_STREQ("number", elemTypes[0].getIdentifier());
	ASSERT_STREQ("string", elemTypes[1].getIdentifier());
	ASSERT_STREQ("object", elemTypes[2].getIdentifier());
}

TEST_F(ArgumentListParserTest, paramWithTypeAndName) {
	ASSERT_TRUE(parser.parse("number test"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));
	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
}

TEST_F(ArgumentListParserTest, paramWithTypeAndNameAndDefaultVal) {
	ASSERT_TRUE(parser.parse("number test = \"tset\""));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));
	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test", arglist[0].name);
	ASSERT_EQ("\"tset\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);
}

TEST_F(ArgumentListParserTest, twoParametersOneDefault) {
	ASSERT_TRUE(parser.parse("number test1 = \"first\", string test2"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(2, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test1", arglist[0].name);
	ASSERT_EQ("\"first\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("test2", arglist[1].name);
	ASSERT_EQ("", arglist[1].def_val);
	ASSERT_FALSE(arglist[1].optional);
}

TEST_F(ArgumentListParserTest, optionalParameterPackOnSingleParam) {
	ASSERT_TRUE(parser.parse("number test1 = \"first\", [string test2]"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(2, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test1", arglist[0].name);
	ASSERT_EQ("\"first\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("test2", arglist[1].name);
	ASSERT_EQ("", arglist[1].def_val);
	ASSERT_TRUE(arglist[1].optional);
}

TEST_F(ArgumentListParserTest, optionalParameterPackOnMultipleParam) {
	ASSERT_TRUE(parser.parse("number test1 = \"first\", [string test2, object objTest]"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(3, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test1", arglist[0].name);
	ASSERT_EQ("\"first\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("test2", arglist[1].name);
	ASSERT_EQ("", arglist[1].def_val);
	ASSERT_TRUE(arglist[1].optional);

	ASSERT_STREQ("object", arglist[2].type.getIdentifier());
	ASSERT_EQ("objTest", arglist[2].name);
	ASSERT_EQ("", arglist[2].def_val);
	ASSERT_TRUE(arglist[2].optional);
}

TEST_F(ArgumentListParserTest, optionalParameterPackOnMultipleParamWithDefault) {
	ASSERT_TRUE(parser.parse("number test1 = \"first\", [string test2 = \"test\", object objTest]"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(3, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test1", arglist[0].name);
	ASSERT_EQ("\"first\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("test2", arglist[1].name);
	ASSERT_EQ("\"test\"", arglist[1].def_val);
	ASSERT_TRUE(arglist[1].optional);

	ASSERT_STREQ("object", arglist[2].type.getIdentifier());
	ASSERT_EQ("objTest", arglist[2].name);
	ASSERT_EQ("", arglist[2].def_val);
	ASSERT_TRUE(arglist[2].optional);
}

TEST_F(ArgumentListParserTest, entireListOptional) {
	ASSERT_TRUE(parser.parse("[number test1 = \"first\", string test2 = \"test\", object objTest]"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(3, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test1", arglist[0].name);
	ASSERT_EQ("\"first\"", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("test2", arglist[1].name);
	ASSERT_EQ("\"test\"", arglist[1].def_val);
	ASSERT_TRUE(arglist[1].optional);

	ASSERT_STREQ("object", arglist[2].type.getIdentifier());
	ASSERT_EQ("objTest", arglist[2].name);
	ASSERT_EQ("", arglist[2].def_val);
	ASSERT_TRUE(arglist[2].optional);
}

TEST_F(ArgumentListParserTest, CommentOnOnlyType) {
	ASSERT_TRUE(parser.parse("number /* test */"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
	ASSERT_EQ("test", arglist[0].comment);
}

TEST_F(ArgumentListParserTest, CommentOnTypeAndName) {
	ASSERT_TRUE(parser.parse("number test /* test */"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
	ASSERT_EQ("test", arglist[0].comment);
}

TEST_F(ArgumentListParserTest, CommentOnTypeNameAndDefault) {
	ASSERT_TRUE(parser.parse("number test = 0 /* test */"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("test", arglist[0].name);
	ASSERT_EQ("0", arglist[0].def_val);
	ASSERT_TRUE(arglist[0].optional);
	ASSERT_EQ("test", arglist[0].comment);
}

TEST_F(ArgumentListParserTest, CommentOnArgWithFolliwingArg) {
	ASSERT_TRUE(parser.parse("number /* test */, string otherArg"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(2, static_cast<int>(arglist.size()));

	ASSERT_STREQ("number", arglist[0].type.getIdentifier());
	ASSERT_EQ("", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
	ASSERT_EQ("test", arglist[0].comment);

	ASSERT_STREQ("string", arglist[1].type.getIdentifier());
	ASSERT_EQ("otherArg", arglist[1].name);
	ASSERT_EQ("", arglist[1].def_val);
	ASSERT_FALSE(arglist[1].optional);
	ASSERT_EQ("", arglist[1].comment);
}

TEST_F(ArgumentListParserTest, MapType) {
	ASSERT_TRUE(parser.parse("{ string => number ... } mapArg"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));

	const auto mapType = arglist[0].type;
	ASSERT_EQ(ade_type_info_type::Map, mapType.getType());
	ASSERT_EQ(2, static_cast<int>(mapType.elements().size()));
	ASSERT_TRUE(mapType.elements()[0].isSimple());
	ASSERT_STREQ("string", mapType.elements()[0].getIdentifier());
	ASSERT_TRUE(mapType.elements()[1].isSimple());
	ASSERT_STREQ("number", mapType.elements()[1].getIdentifier());

	ASSERT_EQ("mapArg", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
	ASSERT_EQ("", arglist[0].comment);
}

TEST_F(ArgumentListParserTest, IteratorType) {
	ASSERT_TRUE(parser.parse("iterator<string> iteratorArg"));

	const auto arglist = parser.getArgList();
	ASSERT_EQ(1, static_cast<int>(arglist.size()));

	const auto mapType = arglist[0].type;
	ASSERT_EQ(ade_type_info_type::Iterator, mapType.getType());
	ASSERT_EQ(1, static_cast<int>(mapType.elements().size()));
	ASSERT_TRUE(mapType.elements()[0].isSimple());
	ASSERT_STREQ("string", mapType.elements()[0].getIdentifier());

	ASSERT_EQ("iteratorArg", arglist[0].name);
	ASSERT_EQ("", arglist[0].def_val);
	ASSERT_FALSE(arglist[0].optional);
	ASSERT_EQ("", arglist[0].comment);
}
