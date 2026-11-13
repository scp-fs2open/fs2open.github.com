#include "parse/sexp.h"
#include "parse/sexp/sexp_lookup.h"

#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

// the checksum table is filled when the file system starts
class SexpFingerprint : public test::FSTestFixture {
  public:
	SexpFingerprint() : test::FSTestFixture(INIT_CFILE) {}
};

TEST_F(SexpFingerprint, builtinChecksumChangesWithTheList)
{
	const uint original = sexp::builtin_operator_fingerprint();
	EXPECT_EQ(original, sexp::builtin_operator_fingerprint());

	Operators.push_back({"test-only-operator", OP_NOT, 0, 0, sexp_oper_type::ACTION});
	const uint added = sexp::builtin_operator_fingerprint();
	Operators.back().text = "test-only-renamed";
	const uint renamed = sexp::builtin_operator_fingerprint();
	Operators.pop_back();

	EXPECT_NE(original, added);
	EXPECT_NE(added, renamed);
	EXPECT_EQ(original, sexp::builtin_operator_fingerprint());
}

TEST_F(SexpFingerprint, renumberingChangesTheBuiltinChecksum)
{
	const uint original = sexp::builtin_operator_fingerprint();
	std::swap(Operators[0].value, Operators[1].value);
	const uint swapped = sexp::builtin_operator_fingerprint();
	std::swap(Operators[0].value, Operators[1].value);

	EXPECT_NE(original, swapped);
}
