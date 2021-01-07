
#include "actions/expression/ExpressionParser.h"
#include "math/vecmat.h"

#include <gtest/gtest.h>

using namespace actions::expression;

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

using OperatorTestValues = std::tuple<SCP_string, Value>;

class ExpressionExecutionTest : public testing::TestWithParam<OperatorTestValues> {
};

TEST_P(ExpressionExecutionTest, operator_execution)
{
	SCP_string expressionText;
	Value expectedReturnValue;

	std::tie(expressionText, expectedReturnValue) = GetParam();

	ExpressionParser parser(expressionText);

	const auto expression = parser.parse();

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), expectedReturnValue.getType());

	const auto resultVal = expression->execute();

	if (expectedReturnValue.getType() == actions::expression::ValueType::Float) {
		ASSERT_FLOAT_EQ(resultVal.getFloat(), expectedReturnValue.getFloat());
	} else {
		ASSERT_EQ(resultVal, expectedReturnValue);
	}
}

INSTANTIATE_TEST_SUITE_P(LiteralTests,
	ExpressionExecutionTest,
	testing::Values(OperatorTestValues("5", Value(5)),
		OperatorTestValues("8.0", Value(8.0f)),
		OperatorTestValues("This is an identifier ", Value("This is an identifier "))));

INSTANTIATE_TEST_SUITE_P(VectorConstructorTests,
	ExpressionExecutionTest,
	testing::Values(OperatorTestValues("(3.3 13.8 (-3.9))", Value(vm_vec_new(3.3f, 13.8f, -3.9f)))));

INSTANTIATE_TEST_SUITE_P(IntegerOperatorTests,
	ExpressionExecutionTest,
	testing::Values(OperatorTestValues("5 + -8", Value(-3)), OperatorTestValues("5 - -8", Value(13))));

INSTANTIATE_TEST_SUITE_P(FloatOperatorTests,
	ExpressionExecutionTest,
	testing::Values(OperatorTestValues("5.0 + -8.0", Value(-3.0f)), OperatorTestValues("5.0 - -8.0", Value(13.0f))));

INSTANTIATE_TEST_SUITE_P(VectorOperatorTests,
	ExpressionExecutionTest,
	testing::Values(OperatorTestValues("(1.0 2.0 3.0) + (3.0 2.0 1.0)", Value(vm_vec_new(4.0, 4.0f, 4.0f))),
		OperatorTestValues("(4.0 4.0 4.0) - (3.0 2.0 1.0)", Value(vm_vec_new(1.0, 2.0f, 3.0f)))));

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
