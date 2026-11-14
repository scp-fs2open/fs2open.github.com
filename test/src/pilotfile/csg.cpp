#include "cfile/cfile.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class PilotCampaignFileTest : public test::FSTestFixture {
  public:
	PilotCampaignFileTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("pilotfile");
	}

  protected:
	void TearDown() override
	{
		cf_delete("short.stub.csg",
			CF_TYPE_PLAYERS,
			CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
		Num_recent_missions = 0;

		FSTestFixture::TearDown();
	}

	static bool load_short_save(player* plr = nullptr)
	{
		player local;
		if (plr == nullptr) {
			plr = &local;
		}
		plr->reset();
		strcpy_s(plr->callsign, "short");

		pilotfile loader;
		return loader.load_savefile(plr, "stub");
	}
};

namespace {
CFILE* start_save()
{
	auto fp = cfopen("short.stub.csg",
		"wb",
		CF_TYPE_PLAYERS,
		false,
		CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	if (fp != nullptr) {
		cfwrite_uint(CSG_FILE_ID, fp);
		cfwrite_ubyte(CSG_VERSION, fp);
	}
	return fp;
}

// Writes a save whose only section ends right after its count, with none of the data the count promises
void write_save(Section id, int count)
{
	auto fp = start_save();
	ASSERT_NE(fp, nullptr);
	cfwrite_ushort(static_cast<ushort>(id), fp);
	cfwrite_uint(4, fp);
	cfwrite_int(count, fp);
	cfclose(fp);
}

// Flags, then Info with empty lists
void write_flags_and_info(CFILE* fp, int last_ship, int missions_completed)
{
	cfwrite_ushort(static_cast<ushort>(Section::Flags), fp);
	cfwrite_uint(5, fp);
	cfwrite_ubyte(0, fp);
	cfwrite_int(0, fp);

	cfwrite_ushort(static_cast<ushort>(Section::Info), fp);
	cfwrite_uint(48, fp);
	for (int i = 0; i < 4; ++i) {
		cfwrite_int(0, fp);
	}
	for (int value : {last_ship, -1, 0, -1, 0, missions_completed}) {
		cfwrite_int(value, fp);
	}
	cfwrite_string_len("", fp);
	cfwrite_string_len("", fp);
}
} // namespace

TEST_F(PilotCampaignFileTest, recent_missions_without_names_are_not_counted)
{
	write_save(Section::LastMissions, 10);
	Num_recent_missions = 3;

	ASSERT_TRUE(load_short_save());

	EXPECT_EQ(Num_recent_missions, 0);
}

TEST_F(PilotCampaignFileTest, variables_without_data_stay_empty)
{
	write_save(Section::Variables, 1000);

	ASSERT_TRUE(load_short_save());

	EXPECT_TRUE(Campaign.persistent_variables.empty());
}

TEST_F(PilotCampaignFileTest, missions_completed_counts_only_the_missions_read)
{
	auto fp = start_save();
	ASSERT_NE(fp, nullptr);
	write_flags_and_info(fp, -1, 1000);

	// Missions as the last section, holding the record of mission 0 only
	cfwrite_ushort(static_cast<ushort>(Section::Missions), fp);
	cfwrite_uint(68, fp);
	for (int i = 0; i < 17; ++i) {
		cfwrite_int(0, fp);
	}
	cfclose(fp);

	ASSERT_TRUE(load_short_save());

	EXPECT_EQ(Campaign.num_missions_completed, 1);
}

TEST_F(PilotCampaignFileTest, last_ship_outside_the_ship_list_is_cleared)
{
	auto fp = start_save();
	ASSERT_NE(fp, nullptr);
	write_flags_and_info(fp, 0, 0);
	cfclose(fp);

	player plr;
	ASSERT_TRUE(load_short_save(&plr));

	EXPECT_EQ(plr.last_ship_flown_si_index, -1);
}
