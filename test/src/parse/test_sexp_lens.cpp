#include <gtest/gtest.h>

#include <parse/sexp.h>

#include <graphics/lens_flare.h>
#include <util/FSTestFixture.h>

#include <algorithm>

// The five lens-flare operators are wired up by hand across five separate
// tables in sexp.cpp (the operator list, the argument-type switch, the category
// and subcategory switches, and the help text). Nothing makes those agree, and
// a desync is silent: an argument slot that returns the wrong OPF still parses,
// it just reads the designer's number into the wrong field.
namespace {

struct lens_operator_expectation {
	const char* name;
	int op_const;
	int min_args;
	int max_args;
	int first_arg_type; // OPF for argnum 0
};

const lens_operator_expectation Lens_operators[] = {
	// only set-camera-lens names a lens; the aperture operators restyle whatever
	// lens the mission has mounted, so their first argument is already a value
	{"set-camera-lens", OP_SET_CAMERA_LENS, 1, 1, OPF_LENS_SYSTEM},
	{"set-lens-aperture", OP_SET_LENS_APERTURE, 1, 4, OPF_POSITIVE},
	{"set-lens-grating", OP_SET_LENS_GRATING, 1, 5, OPF_POSITIVE},
	{"set-lens-scratches", OP_SET_LENS_SCRATCHES, 1, 7, OPF_POSITIVE},
	{"set-lens-dust", OP_SET_LENS_DUST, 1, 4, OPF_POSITIVE},
};

const sexp_oper* find_lens_operator(const char* name)
{
	auto it = std::find_if(Operators.begin(), Operators.end(), [name](const sexp_oper& op) {
		return op.text == name;
	});
	return (it == Operators.end()) ? nullptr : &(*it);
}

} // namespace

TEST(SexpLens, OperatorsAreRegisteredWithExpectedArity)
{
	for (const auto& expected : Lens_operators) {
		SCOPED_TRACE(expected.name);
		const sexp_oper* op = find_lens_operator(expected.name);
		ASSERT_NE(op, nullptr) << "operator missing from the Operators table";
		EXPECT_EQ(op->value, expected.op_const);
		EXPECT_EQ(op->min, expected.min_args);
		EXPECT_EQ(op->max, expected.max_args);
	}
}

TEST(SexpLens, EveryArgumentSlotHasAnArgumentType)
{
	// note that query_operator_argument_type() wants the operator's index in the
	// Operators table, not its OP_ constant
	for (const auto& expected : Lens_operators) {
		SCOPED_TRACE(expected.name);
		const int op_index = find_operator_index(expected.op_const);
		ASSERT_GE(op_index, 0);

		EXPECT_EQ(query_operator_argument_type(op_index, 0), expected.first_arg_type);

		// no slot may fall through to the switch default, which would hand the
		// designer an argument the operator never reads
		for (int argnum = 0; argnum < expected.max_args; argnum++) {
			SCOPED_TRACE("argnum " + std::to_string(argnum));
			EXPECT_NE(query_operator_argument_type(op_index, argnum), OPF_NONE)
				<< "argument slot has no declared type";
		}
	}

	// set-lens-aperture's curvature bows the blades inward at negative values,
	// so that slot in particular must not be restricted to positive numbers
	EXPECT_EQ(query_operator_argument_type(find_operator_index(OP_SET_LENS_APERTURE), 2), OPF_NUMBER);
}

TEST(SexpLens, OperatorsAreCategorisedAndDocumented)
{
	for (const auto& expected : Lens_operators) {
		SCOPED_TRACE(expected.name);

		// an uncategorised operator doesn't appear in FRED's menus at all
		EXPECT_EQ(get_category(expected.op_const), OP_CATEGORY_CHANGE);
		EXPECT_EQ(get_subcategory(expected.op_const), CHANGE_SUBCATEGORY_BACKGROUND_AND_NEBULA);

		EXPECT_EQ(query_operator_return_type(expected.op_const), OPR_NULL);

		auto help = std::find_if(Sexp_help.begin(), Sexp_help.end(), [&expected](const sexp_help_struct& h) {
			return h.id == expected.op_const;
		});
		ASSERT_NE(help, Sexp_help.end()) << "operator has no help text";
		EXPECT_NE(help->help.find(expected.name), SCP_string::npos) << "help text doesn't name the operator";
	}
}

// OPF_LENS_SYSTEM arguments are validated at mission load, where a rejection
// aborts the load outright. That is only safe because the built-in lens table
// is an engine default, so these names resolve whatever is installed.
class SexpLensTableTest : public test::FSTestFixture {
  public:
	SexpLensTableTest() : test::FSTestFixture(INIT_CFILE) {}

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		graphics::lens_flare_init();
	}

	void TearDown() override
	{
		graphics::lens_flare_close();
		test::FSTestFixture::TearDown();
	}
};

TEST_F(SexpLensTableTest, ValidatesLensNames)
{
	// every lens the default table ships must be nameable from a mission
	const int count = graphics::lens_flare_num_systems();
	ASSERT_GT(count, 0);
	for (int i = 0; i < count; i++) {
		const char* name = graphics::lens_flare_get_system(i)->name.c_str();
		SCOPED_TRACE(name);
		EXPECT_TRUE(sexp_lens_name_is_valid(name));
	}

	// the set-camera-lens sentinels, case-insensitively
	EXPECT_TRUE(sexp_lens_name_is_valid("<none>"));
	EXPECT_TRUE(sexp_lens_name_is_valid("<default>"));
	EXPECT_TRUE(sexp_lens_name_is_valid("<None>"));
	EXPECT_TRUE(sexp_lens_name_is_valid("<DEFAULT>"));

	// and a name no table defines, which is what stops a typo from silently
	// leaving the flare unchanged at runtime
	EXPECT_FALSE(sexp_lens_name_is_valid("no_such_lens"));
	EXPECT_FALSE(sexp_lens_name_is_valid(""));
	EXPECT_FALSE(sexp_lens_name_is_valid(nullptr));

	// the error code has a message, or FRED and the mission loader print nothing
	EXPECT_STRNE(sexp_error_message(SEXP_CHECK_INVALID_LENS_SYSTEM), nullptr);
	EXPECT_GT(strlen(sexp_error_message(SEXP_CHECK_INVALID_LENS_SYSTEM)), 0u);
}
