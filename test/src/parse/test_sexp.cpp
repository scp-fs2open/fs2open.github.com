
#include <gtest/gtest.h>

#include <parse/sexp.h>

// Tests for SEXP metadata functions
// These tests don't require game initialization because the Operators vector
// is populated via static initialization from sexp_get_operator_descriptors()

class SexpMetadataTest : public ::testing::Test {
protected:
	void SetUp() override {
		// Operators vector is already populated via static initializer
	}

	// Helper to find operator index by value
	int find_operator_index_by_value(int op_value) {
		for (size_t i = 0; i < Operators.size(); i++) {
			if (Operators[i].value == op_value) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}
};

// Basic sanity check - verify Operators vector is populated
TEST_F(SexpMetadataTest, operators_populated) {
	ASSERT_FALSE(Operators.empty()) << "Operators vector should be populated at startup";
	EXPECT_GT(Operators.size(), 100u) << "Should have at least 100 operators";
}

// Test that all operators have valid return types
TEST_F(SexpMetadataTest, all_operators_have_valid_return_type) {
	for (size_t i = 0; i < Operators.size(); i++) {
		int op_value = Operators[i].value;
		int return_type = query_operator_return_type(op_value);

		// Return type should be one of the valid OPR_* values
		EXPECT_TRUE(return_type >= OPR_NONE && return_type <= OPR_FLEXIBLE_ARGUMENT)
			<< "Operator '" << Operators[i].text << "' (value=" << op_value
			<< ") has invalid return type: " << return_type;
	}
}

// Test that all operators have valid categories
TEST_F(SexpMetadataTest, all_operators_have_valid_category) {
	for (size_t i = 0; i < Operators.size(); i++) {
		int op_value = Operators[i].value;
		int category = get_category(op_value);

		// Category should be one of the valid OP_CATEGORY_* values
		EXPECT_TRUE(category >= OP_CATEGORY_NONE && category < First_available_category_id)
			<< "Operator '" << Operators[i].text << "' (value=" << op_value
			<< ") has invalid category: " << category;
	}
}

// Test specific well-known operators have expected return types
TEST_F(SexpMetadataTest, known_operators_return_types) {
	// Boolean operators should return OPR_BOOL
	EXPECT_EQ(query_operator_return_type(OP_TRUE), OPR_BOOL);
	EXPECT_EQ(query_operator_return_type(OP_FALSE), OPR_BOOL);
	EXPECT_EQ(query_operator_return_type(OP_AND), OPR_BOOL);
	EXPECT_EQ(query_operator_return_type(OP_OR), OPR_BOOL);
	EXPECT_EQ(query_operator_return_type(OP_NOT), OPR_BOOL);

	// Arithmetic operators return OPR_NUMBER or OPR_POSITIVE
	int plus_ret = query_operator_return_type(OP_PLUS);
	EXPECT_TRUE(plus_ret == OPR_NUMBER || plus_ret == OPR_POSITIVE);

	int minus_ret = query_operator_return_type(OP_MINUS);
	EXPECT_TRUE(minus_ret == OPR_NUMBER || minus_ret == OPR_POSITIVE);

	// abs returns OPR_POSITIVE (non-negative number)
	EXPECT_EQ(query_operator_return_type(OP_ABS), OPR_POSITIVE);
}

// Test specific well-known operators have expected categories
TEST_F(SexpMetadataTest, known_operators_categories) {
	// Arithmetic operators
	EXPECT_EQ(get_category(OP_PLUS), OP_CATEGORY_ARITHMETIC);
	EXPECT_EQ(get_category(OP_MINUS), OP_CATEGORY_ARITHMETIC);
	EXPECT_EQ(get_category(OP_MUL), OP_CATEGORY_ARITHMETIC);

	// Logical operators
	EXPECT_EQ(get_category(OP_AND), OP_CATEGORY_LOGICAL);
	EXPECT_EQ(get_category(OP_OR), OP_CATEGORY_LOGICAL);
	EXPECT_EQ(get_category(OP_TRUE), OP_CATEGORY_LOGICAL);
	EXPECT_EQ(get_category(OP_FALSE), OP_CATEGORY_LOGICAL);
}

// Test get_operator_index(const char*) finds operators by name
TEST_F(SexpMetadataTest, get_operator_index_by_name) {
	int plus_idx = get_operator_index("+");
	ASSERT_GE(plus_idx, 0) << "Should find + operator by name";
	EXPECT_EQ(Operators[plus_idx].value, OP_PLUS);

	int and_idx = get_operator_index("and");
	ASSERT_GE(and_idx, 0) << "Should find and operator by name";
	EXPECT_EQ(Operators[and_idx].value, OP_AND);

	int true_idx = get_operator_index("true");
	ASSERT_GE(true_idx, 0) << "Should find true operator by name";
	EXPECT_EQ(Operators[true_idx].value, OP_TRUE);
}

// Test get_operator_index returns -1 for unknown operators
TEST_F(SexpMetadataTest, get_operator_index_unknown_returns_negative) {
	int invalid_idx = get_operator_index("this_is_not_an_operator");
	EXPECT_LT(invalid_idx, 0) << "Should return negative for unknown operator name";
}

// Test that we can find operators in Operators[] by value
TEST_F(SexpMetadataTest, find_operator_by_value) {
	int plus_idx = find_operator_index_by_value(OP_PLUS);
	ASSERT_GE(plus_idx, 0) << "Should find OP_PLUS in Operators";
	EXPECT_STREQ(Operators[plus_idx].text.c_str(), "+");

	int minus_idx = find_operator_index_by_value(OP_MINUS);
	ASSERT_GE(minus_idx, 0) << "Should find OP_MINUS in Operators";
	EXPECT_STREQ(Operators[minus_idx].text.c_str(), "-");
}

// Test that operator min/max argument counts are reasonable
TEST_F(SexpMetadataTest, operator_argument_counts_reasonable) {
	for (size_t i = 0; i < Operators.size(); i++) {
		EXPECT_GE(Operators[i].min, 0)
			<< "Operator '" << Operators[i].text << "' has negative min args";
		EXPECT_GE(Operators[i].max, Operators[i].min)
			<< "Operator '" << Operators[i].text << "' has max < min";
	}
}

