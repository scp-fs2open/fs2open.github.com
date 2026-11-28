#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "cmdline/cmdline.h"
#include "globalincs/utility.h"
#include "iff_defs/iff_defs.h"
#include "particle/ParticleManager.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"

namespace
{

// unit tests turn Warning() into an exception; this lets ship_init() continue past a warning it is expected to raise
class ScopedNonfatalWarnings
{
  public:
	ScopedNonfatalWarnings() : _running_unittests(running_unittests), _noninteractive(Cmdline_noninteractive)
	{
		running_unittests = false;
		Cmdline_noninteractive = true;
	}
	~ScopedNonfatalWarnings()
	{
		running_unittests = _running_unittests;
		Cmdline_noninteractive = _noninteractive;
	}

  private:
	bool _running_unittests;
	bool _noninteractive;
};

template <typename NAME_OF>
SCP_vector<SCP_string> member_names(const char* group_name, SCP_vector<int> ai_target_priority::*members, int num_entries, NAME_OF name_of)
{
	SCP_vector<SCP_string> names;

	int group = find_item_with_string(Ai_tp_list, &ai_target_priority::name, group_name);
	if (group < 0)
	{
		ADD_FAILURE() << "no target priority group named '" << group_name << "'";
		return names;
	}

	for (int index : Ai_tp_list[group].*members)
	{
		if (index >= 0 && index < num_entries)
			names.emplace_back(name_of(index));
		else
			names.emplace_back("<invalid index " + std::to_string(index) + ">");
	}
	return names;
}

SCP_vector<SCP_string> ship_type_members(const char* group_name)
{
	return member_names(group_name, &ai_target_priority::ship_type, sz2i(Ship_types.size()), [](int idx) { return SCP_string(Ship_types[idx].name); });
}

SCP_vector<SCP_string> ship_class_members(const char* group_name)
{
	return member_names(group_name, &ai_target_priority::ship_class, ship_info_size(), [](int idx) { return SCP_string(Ship_info[idx].name); });
}

} // namespace

class TargetPriorityGroupsTest : public test::FSTestFixture
{
  public:
	TargetPriorityGroupsTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS)
	{
		pushModDir("ship");
		pushModDir("target_priority_groups");
	}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		// ship classes create their default particle effects on construction
		particle::ParticleManager::init();
		// ship classes look up their species while parsing, and species look up their default IFF
		iff_init();
		species_init();
	}
	void TearDown() override
	{
		ship_close();
		iff_close();
		particle::ParticleManager::shutdown();
		test::FSTestFixture::TearDown();
	}
};

// group memberships are recorded while the objecttypes tables are parsed, but ship_init() erases the "stealth" type afterwards
TEST_F(TargetPriorityGroupsTest, stealth_type_removal)
{
	int warnings = Global_warning_count;
	{
		ScopedNonfatalWarnings nonfatal;
		ship_init();
	}
	EXPECT_EQ(Global_warning_count, warnings + 1) << "expected only the warning about the removed \"stealth\" type";

	EXPECT_EQ(ship_type_members("fighters"), SCP_vector<SCP_string>{"fighter"});
	EXPECT_EQ(ship_type_members("bombers"), SCP_vector<SCP_string>{"bomber"});
}

// group memberships are recorded as each ship class is parsed, but +remove in a later table erases an earlier class
TEST_F(TargetPriorityGroupsTest, remove_ship_class)
{
	int warnings = Global_warning_count;
	ship_init();
	EXPECT_EQ(Global_warning_count, warnings);

	EXPECT_EQ(ship_class_members("group_a"), (SCP_vector<SCP_string>{"Ship B", "Ship C"}));
}

// +Override replaces the entry's own memberships in every group, and leaves the groups' other members alone
TEST_F(TargetPriorityGroupsTest, override_replaces_own_memberships)
{
	int warnings = Global_warning_count;
	ship_init();
	EXPECT_EQ(Global_warning_count, warnings);

	EXPECT_EQ(ship_class_members("group_1"), SCP_vector<SCP_string>{"Ship B"});
	EXPECT_EQ(ship_class_members("group_2"), SCP_vector<SCP_string>{"Ship B"});
	EXPECT_EQ(ship_class_members("group_3"), SCP_vector<SCP_string>{"Ship A"});
}
