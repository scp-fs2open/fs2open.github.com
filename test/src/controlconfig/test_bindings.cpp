#include "controlconfig/controlsconfig.h"
#include "io/joy.h"
#include "io/key.h"

#include <gtest/gtest.h>

TEST(ControlBindings, FlagNamesRoundTrip)
{
	const char all[] = {CCF_AXIS_BTN, CCF_RELATIVE, CCF_INVERTED, CCF_AXIS, CCF_HAT, CCF_BALL, CCF_BUTTON};

	for (int mask = 0; mask < (1 << 7); ++mask) {
		char flags = 0;
		for (int i = 0; i < 7; ++i) {
			if (mask & (1 << i)) {
				flags |= all[i];
			}
		}

		EXPECT_EQ(CCFToVal(ValToCCF(flags).c_str()), flags) << "flags " << static_cast<int>(flags);
	}
}

TEST(ControlBindings, AxisButtonIsNotAnAxis)
{
	EXPECT_EQ(CCFToVal("AXIS_BTN"), CCF_AXIS_BTN);
	EXPECT_EQ(CCFToVal("AXIS_BTN, INVERTED"), CCF_AXIS_BTN | CCF_INVERTED);
	EXPECT_EQ(CCFToVal("AXIS"), CCF_AXIS);
	EXPECT_EQ(CCFToVal("BUTTON"), CCF_BUTTON);
}

TEST(ControlBindings, KeyboardKeepsOnlyInverted)
{
	EXPECT_EQ(CC_bind(CID_KEYBOARD, KEY_A, CCF_INVERTED | CCF_AXIS | CCF_RELATIVE).get_flags(), CCF_INVERTED);
	EXPECT_EQ(CC_bind(CID_KEYBOARD, KEY_A, CCF_AXIS).get_flags(), 0);
}

TEST(ControlBindings, InvertedKeyStillConflicts)
{
	const CC_bind plain(CID_KEYBOARD, KEY_A);
	const CC_bind inverted(CID_KEYBOARD, KEY_A, CCF_INVERTED);

	EXPECT_TRUE(plain.conflicts_with(inverted));
	EXPECT_TRUE(inverted.conflicts_with(plain));
}

TEST(ControlBindings, AxisButtonSides)
{
	const CC_bind pos(CID_JOY0, JOY_X_AXIS, CCF_AXIS_BTN);
	const CC_bind neg(CID_JOY0, JOY_X_AXIS, CCF_AXIS_BTN | CCF_INVERTED);
	const CC_bind axis(CID_JOY0, JOY_X_AXIS, CCF_AXIS);
	const CC_bind other(CID_JOY0, JOY_Y_AXIS, CCF_AXIS_BTN);

	EXPECT_FALSE(pos.conflicts_with(neg));
	EXPECT_TRUE(pos.conflicts_with(pos));
	EXPECT_TRUE(neg.conflicts_with(axis));
	EXPECT_FALSE(pos.conflicts_with(other));
}

TEST(ControlBindings, AxisControlKeepsTwoKeys)
{
	CCB item;
	item.take(CC_bind(CID_KEYBOARD, KEY_D), 0, true);
	item.take(CC_bind(CID_KEYBOARD, KEY_A, CCF_INVERTED), 1, true);

	EXPECT_EQ(item.first.get_btn(), KEY_D);
	EXPECT_EQ(item.second.get_btn(), KEY_A);
	EXPECT_TRUE(item.second.is_inverted());

	item.take(CC_bind(CID_KEYBOARD, KEY_W), 0, true);
	EXPECT_EQ(item.first.get_btn(), KEY_W);
	EXPECT_EQ(item.second.get_btn(), KEY_A);

	CCB digital;
	digital.take(CC_bind(CID_KEYBOARD, KEY_D), 0);
	digital.take(CC_bind(CID_KEYBOARD, KEY_A), 1);

	EXPECT_TRUE(digital.first.empty());
	EXPECT_EQ(digital.second.get_btn(), KEY_A);
}

TEST(ControlBindings, AxisControlStillReplacesAxesAndRepeats)
{
	const CC_bind x_axis(CID_JOY0, JOY_X_AXIS, CCF_AXIS);
	const CC_bind z_axis(CID_JOY0, JOY_Z_AXIS, CCF_AXIS);
	const CC_bind hat(CID_JOY0, 128, CCF_BUTTON);
	const CC_bind d_key(CID_KEYBOARD, KEY_D);
	const CC_bind d_key_inverted(CID_KEYBOARD, KEY_D, CCF_INVERTED);

	const struct {
		CC_bind kept;
		CC_bind added;
	} pairs[] = {{x_axis, z_axis}, {x_axis, x_axis}, {x_axis, hat}, {hat, x_axis}, {d_key, d_key_inverted}};

	for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
		const auto& pair = pairs[i];
		for (int order = 0; order < 2; ++order) {
			CCB item;
			item.take(pair.kept, order, true);
			item.take(pair.added, 1 - order, true);

			const CC_bind& kept_slot = (order == 0) ? item.first : item.second;
			const CC_bind& added_slot = (order == 0) ? item.second : item.first;
			EXPECT_TRUE(kept_slot.empty()) << "pair " << i << ", order " << order;
			EXPECT_TRUE(added_slot == pair.added) << "pair " << i << ", order " << order;
		}
	}
}

TEST(ControlBindings, WholeRowBindReplacesTheMatchingKey)
{
	CCB item;
	item.take(CC_bind(CID_KEYBOARD, KEY_D), 0, true);
	item.take(CC_bind(CID_KEYBOARD, KEY_A, CCF_INVERTED), 1, true);
	item.take(CC_bind(CID_KEYBOARD, KEY_A), -1, true);

	EXPECT_EQ(item.first.get_btn(), KEY_A);
	EXPECT_FALSE(item.first.is_inverted());
	EXPECT_TRUE(item.second.empty());
}

TEST(ControlBindings, StickButtonConflictsWithPlainButton)
{
	EXPECT_TRUE(CC_bind(CID_JOY0, 0, CCF_BUTTON).conflicts_with(CC_bind(CID_JOY0, 0)));
	EXPECT_TRUE(CC_bind(CID_JOY0, 128, CCF_BUTTON).conflicts_with(CC_bind(CID_JOY0, 128)));
}

// Not in a header; defined in controlsconfig.cpp
void scale_invert(const CC_bind& bind,
	int action,
	CC_type type,
	float frame_time,
	int (&axis_in)[CID_JOY_MAX + 1][JOY_NUM_AXES],
	int* axis_out,
	bool buttons_live);

TEST(ControlBindings, FlaglessStickBindOnAxisIsStillAnAxis)
{
	int axes[CID_JOY_MAX + 1][JOY_NUM_AXES] = {{0}};
	axes[CID_JOY0][JOY_X_AXIS] = JOY_AXIS_MAX - 1;

	int flagged = 0;
	int flagless = 0;
	scale_invert(CC_bind(CID_JOY0, JOY_X_AXIS, CCF_AXIS), 0, CC_TYPE_AXIS_REL, 0.0f, axes, &flagged, true);
	scale_invert(CC_bind(CID_JOY0, JOY_X_AXIS), 0, CC_TYPE_AXIS_REL, 0.0f, axes, &flagless, true);

	EXPECT_NE(flagged, 0);
	EXPECT_EQ(flagless, flagged);
}

TEST(ControlBindings, OutOfRangeAxisIndexIsIgnored)
{
	int axes[CID_JOY_MAX + 1][JOY_NUM_AXES] = {{0}};
	int out = 0;
	scale_invert(CC_bind(CID_JOY0, 128, CCF_AXIS), 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &out, true);
	scale_invert(CC_bind(CID_JOY0, JOY_NUM_AXES, CCF_AXIS), 0, CC_TYPE_AXIS_REL, 0.0f, axes, &out, true);

	EXPECT_EQ(out, 0);
}
