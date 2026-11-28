//
//


#include <util/FSTestFixture.h>
#include <hud/hudsquadmsg.h>
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

TEST_F(ModTableTest, squad_orders_later_list_replaces) {
	mod_table_init();

	ASSERT_EQ(Parsed_comm_orders.size(), 2u);
	ASSERT_EQ(Parsed_comm_orders[0].first, CommOrderType::MSG_ALL_BOMBERS);
	ASSERT_EQ(Parsed_comm_orders[0].second, "Bombers");
	ASSERT_EQ(Parsed_comm_orders[1].first, CommOrderType::ABORT_REARM);
	ASSERT_EQ(Parsed_comm_orders[1].second, "Stop Rearm");
}

TEST_F(ModTableTest, squad_order_text_too_long) {
#ifndef NDEBUG
	ASSERT_THROW(mod_table_init(), os::dialogs::WarningException);
#else
	mod_table_init();

	ASSERT_EQ(Parsed_comm_orders.size(), 1u);
	ASSERT_EQ(Parsed_comm_orders[0].second, SCP_string(MAX_MENU_ITEM_TEXT_SIZE - 1, 'x'));
#endif
}

TEST_F(ModTableTest, squad_order_xstr_kept_untranslated) {
	mod_table_init();

	ASSERT_EQ(Parsed_comm_orders.size(), 1u);
	ASSERT_EQ(Parsed_comm_orders[0].second, "XSTR(\"Bombers\", 9999)");
}

TEST_F(ModTableTest, squad_orders_reset) {
	mod_table_init();
	ASSERT_EQ(Parsed_comm_orders.size(), 1u);

	mod_table_reset();

	ASSERT_TRUE(Parsed_comm_orders.empty());
}
