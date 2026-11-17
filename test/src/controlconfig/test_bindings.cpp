#include "cfile/cfile.h"
#include "controlconfig/controlsconfig.h"
#include "controlconfig/presets.h"
#include "io/joy.h"
#include "io/key.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>
#include <jansson.h>

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

TEST(ControlBindings, ReleasedKeyOnAnAxis)
{
	int axes[CID_JOY_MAX + 1][JOY_NUM_AXES] = {{0}};
	int abs_plain = 0;
	int abs_inverted = 0;
	int abs_inverted_keys_ignored = 0;
	int rel_inverted = 0;
	const CC_bind key(CID_KEYBOARD, KEY_A);
	const CC_bind inverted_key(CID_KEYBOARD, KEY_A, CCF_INVERTED);
	scale_invert(key, 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &abs_plain, true);
	scale_invert(inverted_key, 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &abs_inverted, true);
	scale_invert(inverted_key, 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &abs_inverted_keys_ignored, false);
	scale_invert(inverted_key, 0, CC_TYPE_AXIS_REL, 0.0f, axes, &rel_inverted, true);

	EXPECT_EQ(abs_plain, 0);
	EXPECT_EQ(abs_inverted, F1_0);
	EXPECT_EQ(abs_inverted_keys_ignored, F1_0);
	EXPECT_EQ(rel_inverted, 0);

	// two binds on one control add up, as two analog binds always have
	int abs_pair = 0;
	scale_invert(inverted_key, 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &abs_pair, true);
	scale_invert(key, 0, CC_TYPE_AXIS_ABS, 0.0f, axes, &abs_pair, true);
	EXPECT_EQ(abs_pair, F1_0);
}

class ControlPresetFile : public test::FSTestFixture {
  public:
	ControlPresetFile() : test::FSTestFixture(INIT_CFILE) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		control_config_common_init();
	}

	void TearDown() override
	{
		cf_delete("xtypetest.json", CF_TYPE_PLAYER_BINDS, PRESET_LOCATION);
		control_config_common_close();
		test::FSTestFixture::TearDown();
	}

	static const uint32_t PRESET_LOCATION = CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT;
};

static json_t* preset_slot(json_t* root, const char* action, const char* slot)
{
	json_t* actions = json_object_get(root, "actions");
	size_t index;
	json_t* entry;
	json_array_foreach(actions, index, entry) {
		const char* name = json_string_value(json_object_get(entry, "bind"));
		if ((name != nullptr) && (strcmp(name, action) == 0)) {
			return json_object_get(entry, slot);
		}
	}
	return nullptr;
}

static const char* slot_field(json_t* slot, const char* field)
{
	const char* value = json_string_value(json_object_get(slot, field));
	return (value != nullptr) ? value : "(missing)";
}

// Builds up to 26.2 read only a slot's outer fields and take a bind's meaning from the control's type, so a bind
// they would misread has to say unbound there
TEST_F(ControlPresetFile, CrossTypeBindsStayOutOfTheFieldsOlderBuildsRead)
{
	CC_preset preset;
	preset.name = "xtypetest";
	preset.type = Preset_t::pst;
	std::copy(Control_config.begin(), Control_config.end(), std::back_inserter(preset.bindings));
	preset.bindings[JOY_HEADING_AXIS].first = CC_bind(CID_KEYBOARD, KEY_D);
	preset.bindings[JOY_HEADING_AXIS].second = CC_bind(CID_KEYBOARD, KEY_J, CCF_INVERTED);
	preset.bindings[JOY_PITCH_AXIS].first = CC_bind(CID_JOY0, iHAT_UP, CCF_BUTTON);
	preset.bindings[JOY_BANK_AXIS].first = CC_bind(CID_JOY0, JOY_RX_AXIS, CCF_AXIS);
	preset.bindings[RIGHT_SLIDE_THRUST].first = CC_bind(CID_KEYBOARD, KEY_E);
	preset.bindings[RIGHT_SLIDE_THRUST].second = CC_bind(CID_JOY0, JOY_X_AXIS, CCF_AXIS_BTN);
	ASSERT_TRUE(save_preset_file(preset, true));

	CFILE* fp = cfopen("xtypetest.json", "rb", CF_TYPE_PLAYER_BINDS, false, PRESET_LOCATION);
	ASSERT_NE(fp, nullptr);
	SCP_string text(static_cast<size_t>(cfilelength(fp)), '\0');
	ASSERT_EQ(cfread(&text[0], 1, static_cast<int>(text.size()), fp), static_cast<int>(text.size()));
	cfclose(fp);

	json_error_t err;
	json_t* root = json_loads(text.c_str(), 0, &err);
	ASSERT_NE(root, nullptr) << err.text;

	const std::pair<const char*, const char*> hidden[] = {{"JOY_HEADING_AXIS", "primary"},
		{"JOY_HEADING_AXIS", "secondary"},
		{"JOY_PITCH_AXIS", "primary"},
		{"RIGHT_SLIDE_THRUST", "secondary"}};
	for (const auto& where : hidden) {
		json_t* slot = preset_slot(root, where.first, where.second);
		ASSERT_NE(slot, nullptr) << where.first << " " << where.second;
		EXPECT_STREQ(slot_field(slot, "cid"), "CID_NONE") << where.first << " " << where.second;
		EXPECT_STREQ(slot_field(slot, "flags"), "NONE") << where.first << " " << where.second;
		EXPECT_STREQ(slot_field(slot, "input"), "NONE") << where.first << " " << where.second;
		EXPECT_TRUE(json_is_object(json_object_get(slot, "v1"))) << where.first << " " << where.second;
	}

	const std::pair<const char*, const char*> plain[] = {{"JOY_BANK_AXIS", "primary"}, {"RIGHT_SLIDE_THRUST", "primary"}};
	for (const auto& where : plain) {
		json_t* slot = preset_slot(root, where.first, where.second);
		ASSERT_NE(slot, nullptr) << where.first << " " << where.second;
		EXPECT_STRNE(slot_field(slot, "cid"), "CID_NONE") << where.first << " " << where.second;
		EXPECT_EQ(json_object_get(slot, "v1"), nullptr) << where.first << " " << where.second;
	}
	json_decref(root);

	Control_config_presets.resize(1);
	load_preset_files();
	auto loaded = std::find_if(Control_config_presets.begin(), Control_config_presets.end(), [](const CC_preset& p) {
		return p.name == "xtypetest";
	});
	ASSERT_NE(loaded, Control_config_presets.end());
	for (const IoActionId id : {JOY_HEADING_AXIS, JOY_PITCH_AXIS, JOY_BANK_AXIS, RIGHT_SLIDE_THRUST}) {
		EXPECT_TRUE(loaded->bindings[id].first == preset.bindings[id].first) << "action " << id << " first";
		EXPECT_TRUE(loaded->bindings[id].second == preset.bindings[id].second) << "action " << id << " second";
	}
}
