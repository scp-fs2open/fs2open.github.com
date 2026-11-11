#include "cfile/cfile.h"
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

		FSTestFixture::TearDown();
	}
};

TEST_F(PilotCampaignFileTest, recent_missions_without_names_are_not_counted)
{
	auto fp = cfopen("short.stub.csg",
		"wb",
		CF_TYPE_PLAYERS,
		false,
		CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	ASSERT_NE(fp, nullptr);
	cfwrite_uint(CSG_FILE_ID, fp);
	cfwrite_ubyte(CSG_VERSION, fp);
	// LastMissions as the last section, holding a count of 10 and no names
	cfwrite_ushort(static_cast<ushort>(Section::LastMissions), fp);
	cfwrite_uint(4, fp);
	cfwrite_int(10, fp);
	cfclose(fp);

	player plr;
	plr.reset();
	strcpy_s(plr.callsign, "short");
	Num_recent_missions = 3;

	pilotfile loader;
	ASSERT_TRUE(loader.load_savefile(&plr, "stub"));

	EXPECT_EQ(Num_recent_missions, 0);
}
