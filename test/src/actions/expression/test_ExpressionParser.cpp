
#include "actions/expression/ExpressionParser.h"

#include <gtest/gtest.h>

using namespace actions::expression;

TEST(ExpressionParser, int_value_expression)
{
	ExpressionParser parser("5");

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), ValueType::Integer);

	// Check the value of the expression
	ASSERT_EQ(expression->execute(), Value(5));
}

TEST(ExpressionParser, float_value_expression)
{
	ExpressionParser parser("8.0");

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), ValueType::Float);

	// Check the value of the expression
	ASSERT_EQ(expression->execute(), Value(8.0f));
}

TEST(ExpressionParser, identifier_value_expression)
{
	ExpressionParser parser("This is an identifier ");

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), ValueType::Identifier);

	// Check the value of the expression
	ASSERT_EQ(expression->execute(), Value("This is an identifier "));
}

TEST(ExpressionParser, vector_constructor)
{
	ExpressionParser parser("(3.3 13.8 (-3.9))");

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), ValueType::Vector);

	const auto result = expression->execute().getVector();

	ASSERT_FLOAT_EQ(result.xyz.x, 3.3f);
	ASSERT_FLOAT_EQ(result.xyz.y, 13.8f);
	ASSERT_FLOAT_EQ(result.xyz.z, -3.9f);
}

using RandomRangeTestValues = std::tuple<SCP_string, ValueType, float /* lower limit */, float /* upper limit */>;

class ExpressionRandomRangeTest : public testing::TestWithParam<RandomRangeTestValues> {
};

TEST_P(ExpressionRandomRangeTest, random_range_exec)
{
	SCP_string expressionText;
	ValueType expectedReturnType;
	float lowerBound;
	float upperBound;

	std::tie(expressionText, expectedReturnType, lowerBound, upperBound) = GetParam();

	ExpressionParser parser(expressionText);

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), expectedReturnType);

	const auto resultVal = expression->execute();

	float result;
	if (expectedReturnType == ValueType::Integer) {
		result = i2fl(resultVal.getInteger());
	} else {
		result = resultVal.getFloat();
	}

	ASSERT_GE(result, lowerBound);
	ASSERT_LE(result, upperBound);
}

INSTANTIATE_TEST_SUITE_P(IntegerTests,
	ExpressionRandomRangeTest,
	testing::Values(RandomRangeTestValues("~(5 8)", ValueType::Integer, 5.0f, 8.0f),
		RandomRangeTestValues("~(5 5)", ValueType::Integer, 5.0f, 5.0f),
		RandomRangeTestValues("~(8 5)", ValueType::Integer, 5.0f, 8.0f),
		RandomRangeTestValues("~(-8 -5)", ValueType::Integer, -8.0f, -5.0f)));

INSTANTIATE_TEST_SUITE_P(FloatTests,
	ExpressionRandomRangeTest,
	testing::Values(RandomRangeTestValues("~(5.0 8.0)", ValueType::Float, 5.0f, 8.0f),
		RandomRangeTestValues("~(5.3 5.3)", ValueType::Float, 5.3f, 5.3f),
		RandomRangeTestValues("~(8.7 5.2)", ValueType::Float, 5.2f, 8.7f),
		RandomRangeTestValues("~(-8.0 -5.0)", ValueType::Float, -8.0f, -5.0f)));

using OperatorTestValues = std::tuple<SCP_string, ValueType, float>;

class ExpressionOperatorTest : public testing::TestWithParam<OperatorTestValues> {
};

TEST_P(ExpressionOperatorTest, operator_execution)
{
	SCP_string expressionText;
	ValueType expectedReturnType;
	float expectedReturnVal;

	std::tie(expressionText, expectedReturnType, expectedReturnVal) = GetParam();

	ExpressionParser parser(expressionText);

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), expectedReturnType);

	float result;
	if (expectedReturnType == ValueType::Integer) {
		result = i2fl(expression->execute().getInteger());
	} else {
		result = expression->execute().getFloat();
	}

	ASSERT_FLOAT_EQ(result, expectedReturnVal);
}

INSTANTIATE_TEST_SUITE_P(IntegerTests,
	ExpressionOperatorTest,
	testing::Values(OperatorTestValues("5 + -8", ValueType::Integer, -3.0f),
		OperatorTestValues("5 - -8", ValueType::Integer, 13.0f)));

INSTANTIATE_TEST_SUITE_P(FloatTests,
	ExpressionOperatorTest,
	testing::Values(OperatorTestValues("5.0 + -8.0", ValueType::Float, -3.0f),
		OperatorTestValues("5.0 - -8.0", ValueType::Float, 13.0f)));

using ParseFailureTestValues = std::tuple<SCP_string, ValueType, float>;

class ExpressionParserFailureTest : public testing::TestWithParam<SCP_string> {
};

TEST_P(ExpressionParserFailureTest, test_parsing)
{
	const auto expressionText = GetParam();

	ExpressionParser parser(expressionText);

	const auto expression = parser.parse();

	ASSERT_EQ(expression.get(), nullptr);
	ASSERT_FALSE(parser.getErrorText().empty());

	std::cout << parser.getErrorText() << std::endl;
}

INSTANTIATE_TEST_SUITE_P(SyntaxError,
	ExpressionParserFailureTest,
	testing::Values("(1 2)", "5. 0", "(1 + 2", "1 + 2)"));
INSTANTIATE_TEST_SUITE_P(OperatorUnknown,
	ExpressionParserFailureTest,
	testing::Values("Test - asdf", "(1 2 3) + (4 5 6)"));
INSTANTIATE_TEST_SUITE_P(OperatorInconsistentType,
	ExpressionParserFailureTest,
	testing::Values("4 + 5.0", "5.0 + 4", "4 - 5.0", "5.0 - 4"));
INSTANTIATE_TEST_SUITE_P(VectorConstructorTest,
	ExpressionParserFailureTest,
	testing::Values("(1 2 3)",
		"(1.0 3.0 2)",
		"(~(1.0 2.0) ~(1 2) 3.0)",
		"(1.0 2 + 3 3.0)",
		"(Test 1.0 2.0)",
		"((1.0 1.0 1.0) 1.0 2.0)"));
