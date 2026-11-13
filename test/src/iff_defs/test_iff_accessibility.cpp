#include "iff_defs/iff_defs.h"
#include "options/Option.h"
#include "options/OptionsManager.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

// defined in iff_defs.cpp, not in its header
extern SCP_vector<std::pair<color, color>> Iff_colors;

class IffAccessibilityTest : public test::FSTestFixture {
  public:
	IffAccessibilityTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("iff/tbm_adds_iff");
	}
};

namespace {
::testing::AssertionResult has_rgb(int color_index, int r, int g, int b)
{
	if (color_index < 0 || color_index >= static_cast<int>(Iff_colors.size())) {
		return ::testing::AssertionFailure() << "no color at index " << color_index;
	}
	const color& c = Iff_colors[color_index].first;
	if (c.red == r && c.green == g && c.blue == b) {
		return ::testing::AssertionSuccess();
	}
	return ::testing::AssertionFailure() << "color is (" << static_cast<int>(c.red) << ", " << static_cast<int>(c.green)
										 << ", " << static_cast<int>(c.blue) << ")";
}
} // namespace

// one test: nothing clears Iff_info, so a second iff_init() in the process fails on duplicate IFF names
TEST_F(IffAccessibilityTest, TbmIffsKeepTheirColorsAndBuiltInsUseAccessibilityColors)
{
	iff_init();

	const int pirate = iff_lookup("Pirate");
	const int friendly = iff_lookup("Friendly");
	ASSERT_GE(pirate, 0);
	ASSERT_GE(friendly, 0);

	EXPECT_TRUE(has_rgb(Iff_info[pirate].accessibility_color_index, 200, 0, 200));
	const auto& seen = Iff_info[pirate].accessibility_observed_color_map;
	ASSERT_EQ(seen.count(friendly), 1u);
	EXPECT_TRUE(has_rgb(seen.at(friendly), 250, 250, 0));

	const int hostile = iff_lookup("Hostile");
	const int unknown = iff_lookup("Unknown");
	ASSERT_GE(hostile, 0);
	ASSERT_GE(unknown, 0);

	EXPECT_TRUE(has_rgb(Iff_info[friendly].accessibility_color_index, 86, 180, 233));
	EXPECT_TRUE(has_rgb(Iff_info[hostile].accessibility_color_index, 230, 159, 0));
	EXPECT_TRUE(has_rgb(Iff_info[hostile].accessibility_observed_color_map.at(friendly), 230, 159, 0));
	EXPECT_TRUE(has_rgb(Iff_info[hostile].accessibility_observed_color_map.at(hostile), 86, 180, 233));

	// recolored by the tbm, but the built-in accessibility color stays
	EXPECT_TRUE(has_rgb(Iff_info[unknown].color_index, 0, 0, 255));
	EXPECT_TRUE(has_rgb(Iff_info[unknown].accessibility_color_index, 204, 121, 167));
	EXPECT_TRUE(has_rgb(Iff_info[iff_lookup("Neutral")].accessibility_color_index, 230, 159, 0));
	EXPECT_TRUE(has_rgb(Iff_info[iff_lookup("Traitor")].accessibility_color_index, 230, 159, 0));

	// every built-in +Sees line has an accessibility twin: green becomes sky blue, red becomes orange
	for (const char* name : {"Friendly", "Hostile", "Neutral", "Unknown", "Traitor"}) {
		const auto& info = Iff_info[iff_lookup(name)];
		ASSERT_EQ(info.accessibility_observed_color_map.size(), info.observed_color_map.size()) << name;
		for (const auto& seen_pair : info.observed_color_map) {
			ASSERT_EQ(info.accessibility_observed_color_map.count(seen_pair.first), 1u) << name;
			const color& normal = Iff_colors[seen_pair.second].first;
			const int accessible = info.accessibility_observed_color_map.at(seen_pair.first);
			if (normal.green == 255) {
				EXPECT_TRUE(has_rgb(accessible, 86, 180, 233))
					<< name << " sees " << Iff_info[seen_pair.first].iff_name;
			} else {
				EXPECT_TRUE(has_rgb(accessible, 230, 159, 0)) << name << " sees " << Iff_info[seen_pair.first].iff_name;
			}
		}
	}

	// the color lookup, with the option on
	auto* mgr = options::OptionsManager::instance();
	mgr->setOverride("Game.IffAccessibility", "true", "test");
	mgr->getOptionByKey("Game.IffAccessibility")->loadInitial();
	mgr->clearOverrides();
	const color* own = iff_get_color_by_team(friendly, -1, 0);
	EXPECT_EQ(own->red, 86);
	EXPECT_EQ(own->blue, 233);
	const color* seen_by_hostile = iff_get_color_by_team(friendly, hostile, 0);
	EXPECT_EQ(seen_by_hostile->red, 230);
	EXPECT_EQ(seen_by_hostile->green, 159);
	const color* seen_by_pirate = iff_get_color_by_team(friendly, pirate, 0);
	EXPECT_EQ(seen_by_pirate->red, 250);
	EXPECT_EQ(seen_by_pirate->blue, 0);

	// the option binds once, so turn it off explicitly for any later test
	mgr->setOverride("Game.IffAccessibility", "false", "test");
	mgr->getOptionByKey("Game.IffAccessibility")->loadInitial();
	mgr->clearOverrides();
	EXPECT_EQ(iff_get_color_by_team(friendly, -1, 0)->red, 0);
}
