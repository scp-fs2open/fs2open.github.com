//
//


#include <util/FSTestFixture.h>
#include <mod_table/mod_table.h>

class ModTableTest : public test::FSTestFixture {
 public:
	ModTableTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("mod");
	}
};

TEST_F(ModTableTest, future_targeted_version) {
	ASSERT_THROW(mod_table_init(), os::dialogs::ErrorException);
}

TEST_F(ModTableTest, correct_targeted_version) {
	mod_table_init();
}

TEST_F(ModTableTest, mod_support_test) {
	mod_table_init();

	ASSERT_TRUE(mod_supports_version(3, 7, 0));
	ASSERT_FALSE(mod_supports_version(4, 0, 0));
}
