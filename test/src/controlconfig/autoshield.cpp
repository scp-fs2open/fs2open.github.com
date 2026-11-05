#include "controlconfig/controlsconfig.h"
#include "io/key.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class AutoShieldControlTest : public test::FSTestFixture {
  public:
	AutoShieldControlTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("controlconfig");
	}
};

// The table's "Classic" preset gives Alt+Q to Toggle Auto Pilot; the default preset leaves Alt+Q free
TEST_F(AutoShieldControlTest, toggleGivesUpAltQOnlyWhereATableUsesIt)
{
	control_config_common_init();

	const short alt_q = KEY_ALTED | KEY_Q;
	const auto& defaults = Control_config_presets[0].bindings;
	EXPECT_EQ(defaults[TOGGLE_AUTO_SHIELD_EQUALIZE].get_btn(CID_KEYBOARD), alt_q);
	EXPECT_EQ(Control_config[TOGGLE_AUTO_SHIELD_EQUALIZE].get_btn(CID_KEYBOARD), alt_q);

	const CC_preset* classic = nullptr;
	for (const auto& preset : Control_config_presets) {
		if (preset.name == "Classic") {
			classic = &preset;
		}
	}
	ASSERT_NE(classic, nullptr);
	EXPECT_EQ(classic->bindings[AUTO_PILOT_TOGGLE].get_btn(CID_KEYBOARD), alt_q);
	EXPECT_TRUE(classic->bindings[TOGGLE_AUTO_SHIELD_EQUALIZE].empty());
}

// The option is off unless the player or a mod turns it on, so the toggle starts off the controls screen
TEST_F(AutoShieldControlTest, toggleIsHiddenWhileTheOptionIsOff)
{
	control_config_common_init();

	EXPECT_FALSE(Control_config[TOGGLE_AUTO_SHIELD_EQUALIZE].disabled);
	EXPECT_TRUE(control_config_is_hidden(Control_config[TOGGLE_AUTO_SHIELD_EQUALIZE]));
	EXPECT_FALSE(control_config_is_hidden(Control_config[SHIELD_EQUALIZE]));
}
