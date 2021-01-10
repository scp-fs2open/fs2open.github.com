
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

	const auto expression = parser.parse({});

	if (!parser.getErrorText().empty()) {
		std::cout << parser.getErrorText() << std::endl;
	}

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), expectedReturnType);

	const auto resultVal = expression->execute({});

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

INSTANTIATE_TEST_SUITE_P(ImplicitConversionTest,
	ExpressionRandomRangeTest,
	testing::Values(RandomRangeTestValues("~(5 8) + 3.0", ValueType::Float, 8.0f, 12.0f)));

using ExecutionTestValues = std::tuple<SCP_string, Value>;

class ExpressionExecutionTest : public testing::TestWithParam<ExecutionTestValues> {
};

TEST_P(ExpressionExecutionTest, operator_execution)
{
	SCP_string expressionText;
	Value expectedReturnValue;

	std::tie(expressionText, expectedReturnValue) = GetParam();

	ParseContext context;
	auto& testScope = context.variables.addScope("test");

	testScope.addMember("test1", actions::expression::ValueType::Vector);
	testScope.addMember("test2", actions::expression::ValueType::Float);

	ProgramVariables variables;

	variables.setValue({"test", "test1"}, Value(vm_vec_new(3.f, 4.f, 5.f)));
	variables.setValue({"test", "test2"}, Value(8.f));

	ExpressionParser parser(expressionText);

	const auto expression = parser.parse(context);

	if (!parser.getErrorText().empty()) {
		std::cout << parser.getErrorText() << std::endl;
	}

	ASSERT_NE(expression.get(), nullptr);

	ASSERT_EQ(expression->getExpressionType(), expectedReturnValue.getType());

	const auto resultVal = expression->execute(variables);

	if (expectedReturnValue.getType() == actions::expression::ValueType::Float) {
		ASSERT_FLOAT_EQ(resultVal.getFloat(), expectedReturnValue.getFloat());
	} else {
		ASSERT_EQ(resultVal, expectedReturnValue);
	}
}

INSTANTIATE_TEST_SUITE_P(LiteralTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("5", Value(5)),
		ExecutionTestValues("8.0", Value(8.0f)),
		ExecutionTestValues("\"This is an identifier \"", Value("This is an identifier "))));

INSTANTIATE_TEST_SUITE_P(VectorConstructorTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("(3.3 13.8 (-3.9))", Value(vm_vec_new(3.3f, 13.8f, -3.9f)))));

INSTANTIATE_TEST_SUITE_P(IntegerOperatorTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("5 + -8", Value(-3)), ExecutionTestValues("5 - -8", Value(13))));

INSTANTIATE_TEST_SUITE_P(FloatOperatorTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("5.0 + -8.0", Value(-3.0f)), ExecutionTestValues("5.0 - -8.0", Value(13.0f))));

INSTANTIATE_TEST_SUITE_P(VectorOperatorTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("(1.0 2.0 3.0) + (3.0 2.0 1.0)", Value(vm_vec_new(4.0, 4.0f, 4.0f))),
		ExecutionTestValues("(4.0 4.0 4.0) - (3.0 2.0 1.0)", Value(vm_vec_new(1.0, 2.0f, 3.0f)))));

INSTANTIATE_TEST_SUITE_P(ImplicitOperatorConversionTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("4 + 5.0", Value(9.0f)),
		ExecutionTestValues("5.0 + 4", Value(9.0f)),
		ExecutionTestValues("4 - 5.0", Value(-1.0f)),
		ExecutionTestValues("5.0 - 4", Value(1.0f))));

INSTANTIATE_TEST_SUITE_P(ImplicitVectorConstructorConversionTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("(1 2 3)", Value(vm_vec_new(1.0f, 2.0f, 3.0f))),
		ExecutionTestValues("(1.0 3.0 2)", Value(vm_vec_new(1.0f, 3.0f, 2.0f))),
		ExecutionTestValues("(1.0 2 + 3 3.0)", Value(vm_vec_new(1.0f, 5.0f, 3.0f)))));

INSTANTIATE_TEST_SUITE_P(VariableTests,
	ExpressionExecutionTest,
	testing::Values(ExecutionTestValues("(1 2 3) + test.test1", Value(vm_vec_new(4.f, 6.f, 8.f))),
		ExecutionTestValues("test.test2", Value(8.0f))));

class ExpressionParserFailureTest : public testing::TestWithParam<SCP_string> {
};

TEST_P(ExpressionParserFailureTest, test_parsing)
{
	const auto expressionText = GetParam();

	ExpressionParser parser(expressionText);

	ParseContext context;
	auto& testScope = context.variables.addScope("test");

	testScope.addMember("test1", actions::expression::ValueType::Vector);

	const auto expression = parser.parse(context);

	ASSERT_EQ(expression.get(), nullptr);
	ASSERT_FALSE(parser.getErrorText().empty());
}

INSTANTIATE_TEST_SUITE_P(SyntaxError,
	ExpressionParserFailureTest,
	testing::Values("(1 2)", "5. 0", "(1 + 2", "1 + 2)"));
INSTANTIATE_TEST_SUITE_P(OperatorUnknown, ExpressionParserFailureTest, testing::Values("\"Test\" - \"asdf\""));
INSTANTIATE_TEST_SUITE_P(VectorConstructorTest,
	ExpressionParserFailureTest,
	testing::Values("(\"Test\" 1.0 2.0)", "((1.0 1.0 1.0) 1.0 2.0)"));
INSTANTIATE_TEST_SUITE_P(VariableTest,
	ExpressionParserFailureTest,
	testing::Values("notTest", "test.invalid", "test.test1.wrong"));
