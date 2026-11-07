#include <gtest/gtest.h>

#include <parse/sexp.h>

#include <graphics/lens_flare.h>
#include <util/FSTestFixture.h>

#include <algorithm>

// The six lens-flare operators are wired up by hand across five separate
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
	{"set-lens-flare-strength", OP_SET_LENS_FLARE_STRENGTH, 1, 5, OPF_POSITIVE},
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

// The four iris operators rebuild a 512^2 mask and its Fourier transform, which
// is slow enough to be seen; set-lens-flare-strength deliberately does not. That
// distinction is the whole reason the sixth operator exists, so the help text
// has to actually make it -- a designer who drives the wrong one from a
// repeating event gets a stuttering mission and no clue why.
TEST(SexpLens, IrisOperatorsDocumentTheirCost)
{
	auto help_for = [](int op_const) -> SCP_string {
		auto it = std::find_if(Sexp_help.begin(), Sexp_help.end(), [op_const](const sexp_help_struct& h) {
			return h.id == op_const;
		});
		return (it == Sexp_help.end()) ? SCP_string() : it->help;
	};

	for (int op : {OP_SET_LENS_APERTURE, OP_SET_LENS_GRATING, OP_SET_LENS_SCRATCHES, OP_SET_LENS_DUST}) {
		SCOPED_TRACE(op);
		const SCP_string help = help_for(op);
		ASSERT_FALSE(help.empty());
		EXPECT_NE(help.find("COST:"), SCP_string::npos) << "no cost warning";
		EXPECT_NE(help.find("repeating"), SCP_string::npos) << "doesn't warn against repeating events";
	}

	const SCP_string cheap = help_for(OP_SET_LENS_FLARE_STRENGTH);
	ASSERT_FALSE(cheap.empty());
	EXPECT_NE(cheap.find("cheap"), SCP_string::npos) << "doesn't say it is the cheap one";
	EXPECT_EQ(cheap.find("COST:"), SCP_string::npos) << "warns about a cost it doesn't have";
}

// set-lens-flare-strength must reach every knob it documents. Its arguments are
// written into lens_overrides by hand, so a copy-paste slip would silently make
// one of them do nothing -- and unlike a wrong OPF, nothing would even warn.
TEST_F(SexpLensTableTest, FlareStrengthOperatorTouchesEveryKnobItDocuments)
{
	const int idx = graphics::lens_flare_lookup("angenieux_100mm");
	ASSERT_GE(idx, 0);
	graphics::lens_flare_switch_to("angenieux_100mm");
	ASSERT_EQ(graphics::lens_flare_active_lens(), idx);

	// Halving everything is the interesting case: it exercises the "percentage of
	// whatever is in force" rule rather than landing on a default by accident.
	const graphics::lens_settings before = graphics::lens_flare_effective_settings(idx);

	graphics::lens_overrides& ov = graphics::lens_flare_overrides();
	ov.intensity = before.intensity * 0.5f;
	ov.ghost_brightness = before.ghost_brightness * 0.5f;
	ov.starburst_brightness = before.starburst_brightness * 0.5f;
	ov.starburst_scale = before.starburst_scale * 0.5f;
	ov.max_ghosts = 5;
	graphics::lens_flare_overrides_changed();

	const graphics::lens_settings after = graphics::lens_flare_effective_settings(idx);
	EXPECT_FLOAT_EQ(after.intensity, before.intensity * 0.5f);
	EXPECT_FLOAT_EQ(after.ghost_brightness, before.ghost_brightness * 0.5f);
	EXPECT_FLOAT_EQ(after.starburst_brightness, before.starburst_brightness * 0.5f);
	EXPECT_FLOAT_EQ(after.starburst_scale, before.starburst_scale * 0.5f);
	EXPECT_EQ(after.max_ghosts, 5);

	// None of it touches the iris, which is why this operator is the cheap one:
	// the textures the aperture drives must be untouched.
	EXPECT_EQ(after.aperture, before.aperture);

	graphics::lens_flare_reset_for_level();
}
