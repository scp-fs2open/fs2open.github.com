#include "parse/sexp/LuaSEXP.h"

#include <gtest/gtest.h>

using sexp::lua_sexp_arguments_are_plain;

TEST(LuaSexpArguments, plainValuesAreAccepted)
{
	EXPECT_TRUE(lua_sexp_arguments_are_plain(""));
	EXPECT_TRUE(lua_sexp_arguments_are_plain(" 7 \"Alpha 1\" \"hi!! there\""));
	EXPECT_TRUE(lua_sexp_arguments_are_plain(" -3 ( true ) ( false ) \"\""));
}

TEST(LuaSexpArguments, otherSexpsAreRefused)
{
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" ( end-mission )"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a\" ( script-eval \"x\" )"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" ( + 1 2 )"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" ( true)"));
}

TEST(LuaSexpArguments, textThatCouldLeaveItsQuotesIsRefused)
{
	// run_sexp turns a lone ! into a quote mark
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a! ( end-mission ) !\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a!\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a! b\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"unterminated"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a\"b"));
}

TEST(LuaSexpArguments, malformedNumbersAreRefused)
{
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 7x"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" -"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 1.5"));
}

TEST(LuaSexpArguments, quotedTextPassesTheCheck)
{
	for (const char* value : {"Alpha 1", "hi! there", "a!", "!!!", "( end-mission )", "", "1234567890123456789012345678901"}) {
		SCP_string text;
		ASSERT_TRUE(sexp::lua_sexp_quote_text(value, text)) << value;
		EXPECT_TRUE(lua_sexp_arguments_are_plain(" " + text)) << text;
	}
}

TEST(LuaSexpArguments, textThatCannotTravelIsNotQuoted)
{
	SCP_string text;
	EXPECT_FALSE(sexp::lua_sexp_quote_text("say \"hi\"", text));
	EXPECT_FALSE(sexp::lua_sexp_quote_text("@Score", text));
	EXPECT_FALSE(sexp::lua_sexp_quote_text("12345678901234567890123456789012", text));
}

TEST(LuaSexpArguments, limitsMatchWhatTheServerSends)
{
	EXPECT_TRUE(lua_sexp_arguments_are_plain(" -2147483648 2147483647"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 12345678901"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"@Score\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"12345678901234567890123456789012\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"!!!\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 7( end-mission )"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain("\t7"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" +3"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" --3"));
}

TEST(LuaSexpArguments, valuesMustBeSeparatedBySpaces)
{
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 0\" \" \"( + 1 2 )0\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 1111111111-1111111111-1111111111"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" \"a\"\"b\""));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" ( true )7"));
	EXPECT_TRUE(lua_sexp_arguments_are_plain(" 0 \" \" 7"));
}

TEST(LuaSexpArguments, valuesAreCounted)
{
	int count = -1;
	ASSERT_TRUE(lua_sexp_arguments_are_plain(" 7 \"Alpha 1\" ( true ) -3", &count));
	EXPECT_EQ(count, 4);
	ASSERT_TRUE(lua_sexp_arguments_are_plain("", &count));
	EXPECT_EQ(count, 0);
}

TEST(LuaSexpArguments, numbersOutsideTheIntRangeAreRefused)
{
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 9999999999"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" 2147483648"));
	EXPECT_FALSE(lua_sexp_arguments_are_plain(" -2147483649"));
	EXPECT_TRUE(lua_sexp_arguments_are_plain(" 2147483647 -2147483648"));
}
