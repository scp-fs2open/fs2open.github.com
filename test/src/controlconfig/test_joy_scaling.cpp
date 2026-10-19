#include "cmdline/cmdline.h"
#include "controlconfig/controlsconfig.h"
#include "io/joy.h"
#include "options/Option.h"
#include "options/OptionsManager.h"

#include <gtest/gtest.h>

// not in a header; defined in controlsconfig.cpp
void scale_invert(const CC_bind& bind,
	int action,
	CC_type type,
	float frame_time,
	int (&axis_in)[CID_JOY_MAX + 1][JOY_NUM_AXES],
	int* axis_out);

class JoyScalingTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		saved_dead_zone = Joy_dead_zone_size;
		saved_sensitivity = Joy_sensitivity;
		saved_cmdline_dead_zone = Cmdline_deadzone;
		for (int i = 0; i < Action::NUM_VALUES; ++i) {
			saved_axis_dead_zone[i] = Joy_axis_dead_zone_size[i];
			saved_axis_sensitivity[i] = Joy_axis_sensitivity[i];
			Joy_axis_dead_zone_size[i] = -1;
			Joy_axis_sensitivity[i] = -1;
		}
		Joy_dead_zone_size = 10;
		Joy_sensitivity = 9;
		Cmdline_deadzone = -1;
	}

	void TearDown() override
	{
		// also here, so a failed assertion can't leave an override behind for later tests
		options::OptionsManager::instance()->clearOverrides();
		Joy_dead_zone_size = saved_dead_zone;
		Joy_sensitivity = saved_sensitivity;
		Cmdline_deadzone = saved_cmdline_dead_zone;
		for (int i = 0; i < Action::NUM_VALUES; ++i) {
			Joy_axis_dead_zone_size[i] = saved_axis_dead_zone[i];
			Joy_axis_sensitivity[i] = saved_axis_sensitivity[i];
		}
	}

	int saved_dead_zone = 0;
	int saved_sensitivity = 0;
	int saved_cmdline_dead_zone = 0;
	int saved_axis_dead_zone[Action::NUM_VALUES] = {};
	int saved_axis_sensitivity[Action::NUM_VALUES] = {};
};

// 10% of the axis range is 6553 units, so 5000 off center sits inside the global deadzone and 5% (3276) is not.
TEST_F(JoyScalingTest, AxisDeadZoneOverridesGlobal)
{
	const int raw = JOY_AXIS_CENTER + 5000;
	ASSERT_EQ(joy_get_scaled_reading(raw), 0);
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::BANK), 0);

	Joy_axis_dead_zone_size[Action::BANK] = 5;
	ASSERT_GT(joy_get_scaled_reading(raw, Action::BANK), 0);
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::HEADING), 0);
	ASSERT_EQ(joy_get_scaled_reading(raw), 0);

	Joy_axis_dead_zone_size[Action::HEADING] = 0;
	ASSERT_GT(joy_get_scaled_reading(JOY_AXIS_CENTER + 1, Action::HEADING), 0);
}

TEST_F(JoyScalingTest, AxisDeadZoneWinsOverCommandLine)
{
	const int raw = JOY_AXIS_CENTER + 5000;
	Cmdline_deadzone = 0;
	ASSERT_GT(joy_get_scaled_reading(raw, Action::PITCH), 0);

	Joy_axis_dead_zone_size[Action::PITCH] = 20;
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::PITCH), 0);
}

TEST_F(JoyScalingTest, AxisSensitivityOverridesGlobal)
{
	const int raw = JOY_AXIS_CENTER + JOY_AXIS_RANGE / 2;
	const int global = joy_get_scaled_reading(raw, Action::PITCH);

	Joy_axis_sensitivity[Action::PITCH] = 0;
	const int low = joy_get_scaled_reading(raw, Action::PITCH);
	ASSERT_LT(low, global);
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::HEADING), global);

	Joy_sensitivity = 0;
	ASSERT_EQ(joy_get_scaled_reading(raw), low);
	Joy_axis_sensitivity[Action::PITCH] = 9;
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::PITCH), global);
}

TEST_F(JoyScalingTest, NegativeDeflectionIsSymmetric)
{
	Joy_axis_dead_zone_size[Action::BANK] = 5;
	Joy_axis_sensitivity[Action::BANK] = 4;
	const int offset = JOY_AXIS_RANGE / 3;
	ASSERT_EQ(joy_get_scaled_reading(JOY_AXIS_CENTER - offset, Action::BANK),
		-joy_get_scaled_reading(JOY_AXIS_CENTER + offset, Action::BANK));
}

TEST_F(JoyScalingTest, AxisDeadZoneUsesTheGlobalScale)
{
	for (int dz : {0, 10, 20, 45}) {
		for (int offset : {1000, 5000, 9000, 15000, 30000}) {
			Joy_dead_zone_size = dz;
			const int global = joy_get_scaled_reading(JOY_AXIS_CENTER + offset);
			Joy_dead_zone_size = 10;
			Joy_axis_dead_zone_size[Action::HEADING] = dz;
			ASSERT_EQ(joy_get_scaled_reading(JOY_AXIS_CENTER + offset, Action::HEADING), global)
				<< "deadzone " << dz << ", offset " << offset;
			Joy_axis_dead_zone_size[Action::HEADING] = -1;
		}
	}
}

TEST_F(JoyScalingTest, ActionsOutsideTheRangeUseTheGlobalSettings)
{
	for (int i = 0; i < Action::NUM_VALUES; ++i) {
		Joy_axis_dead_zone_size[i] = 0;
		Joy_axis_sensitivity[i] = 0;
	}
	const int raw = JOY_AXIS_CENTER + 5000;
	ASSERT_EQ(joy_get_scaled_reading(raw, -1), 0);
	ASSERT_EQ(joy_get_scaled_reading(raw, Action::NUM_VALUES), 0);
	ASSERT_GT(joy_get_scaled_reading(raw, Action::NUM_VALUES - 1), 0);
}

TEST_F(JoyScalingTest, BoundAxisUsesItsActionSettings)
{
	int axis_in[CID_JOY_MAX + 1][JOY_NUM_AXES] = {};
	int axis_out[Action::NUM_VALUES] = {};
	axis_in[CID_JOY1][JOY_X_AXIS] = JOY_AXIS_CENTER + 5000;
	const CC_bind bind(CID_JOY1, JOY_X_AXIS);

	scale_invert(bind, Action::BANK, CC_TYPE_AXIS_REL, 0.016f, axis_in, axis_out);
	ASSERT_EQ(axis_out[Action::BANK], 0);

	Joy_axis_dead_zone_size[Action::BANK] = 5;
	scale_invert(bind, Action::BANK, CC_TYPE_AXIS_REL, 0.016f, axis_in, axis_out);
	ASSERT_GT(axis_out[Action::BANK], 0);
}

TEST_F(JoyScalingTest, OptionsBindToTheirOwnAction)
{
	const struct {
		const char* key;
		int* dest;
	} options_to_check[] = {
		{"Input.HeadingDeadZone", &Joy_axis_dead_zone_size[Action::HEADING]},
		{"Input.PitchDeadZone", &Joy_axis_dead_zone_size[Action::PITCH]},
		{"Input.BankDeadZone", &Joy_axis_dead_zone_size[Action::BANK]},
		{"Input.RelThrottleDeadZone", &Joy_axis_dead_zone_size[Action::REL_THROTTLE]},
		{"Input.HeadingSensitivity", &Joy_axis_sensitivity[Action::HEADING]},
		{"Input.PitchSensitivity", &Joy_axis_sensitivity[Action::PITCH]},
		{"Input.BankSensitivity", &Joy_axis_sensitivity[Action::BANK]},
		{"Input.RelThrottleSensitivity", &Joy_axis_sensitivity[Action::REL_THROTTLE]},
	};
	auto* mgr = options::OptionsManager::instance();
	for (const auto& entry : options_to_check) {
		auto* opt = mgr->getOptionByKey(entry.key);
		ASSERT_NE(opt, nullptr) << entry.key;
		ASSERT_TRUE(opt->getFlags()[options::OptionFlags::RangeTypeInteger]) << entry.key;

		mgr->setOverride(entry.key, "7", "test");
		opt->loadInitial();
		for (const auto& other : options_to_check) {
			ASSERT_EQ(*other.dest, other.dest == entry.dest ? 7 : -1) << entry.key << " set " << other.key;
		}
		mgr->clearOverrides();
		*entry.dest = -1;
	}
}
