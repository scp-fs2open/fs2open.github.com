#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "cfile/cfile.h"
#include "checkpoint/store.h"
#include "mission/missiongoals.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"

extern char Game_current_mission_filename[];

class CheckpointStoreTest : public test::FSTestFixture {
  public:
	CheckpointStoreTest() : test::FSTestFixture(INIT_CFILE) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		strcpy_s(Game_current_mission_filename, MAX_FILENAME_LEN, "checkpoint_store_test.fs2");
		checkpoint::store_clear(-1);
	}

	void TearDown() override
	{
		checkpoint::store_clear(-1);
		test::FSTestFixture::TearDown();
	}
};

TEST_F(CheckpointStoreTest, slots_are_kept_per_index)
{
	EXPECT_FALSE(checkpoint::store_has(-1));
	EXPECT_EQ(checkpoint::store_next_free_index(), 1);

	checkpoint::Buffer first = {1, 2, 3};
	checkpoint::Buffer third = {4, 5};
	ASSERT_TRUE(checkpoint::store_save(1, first));
	ASSERT_TRUE(checkpoint::store_save(3, third));

	EXPECT_TRUE(checkpoint::store_has(-1));
	EXPECT_TRUE(checkpoint::store_has(1));
	EXPECT_FALSE(checkpoint::store_has(2));
	EXPECT_TRUE(checkpoint::store_has(3, "CHECKPOINT_STORE_TEST"));	// with another case and without extension
	EXPECT_EQ(checkpoint::store_next_free_index(), 2);

	checkpoint::Buffer loaded;
	ASSERT_TRUE(checkpoint::store_load(3, loaded));
	EXPECT_EQ(loaded, third);
	EXPECT_FALSE(checkpoint::store_load(2, loaded));

	// replacing one slot keeps the others
	checkpoint::Buffer replaced = {9};
	ASSERT_TRUE(checkpoint::store_save(1, replaced));
	ASSERT_TRUE(checkpoint::store_load(1, loaded));
	EXPECT_EQ(loaded, replaced);
	ASSERT_TRUE(checkpoint::store_load(3, loaded));
	EXPECT_EQ(loaded, third);
}

TEST_F(CheckpointStoreTest, clearing_one_or_all)
{
	ASSERT_TRUE(checkpoint::store_save(1, {1}));
	ASSERT_TRUE(checkpoint::store_save(2, {2}));

	checkpoint::store_clear(1);
	EXPECT_FALSE(checkpoint::store_has(1));
	EXPECT_TRUE(checkpoint::store_has(2));
	EXPECT_EQ(checkpoint::store_next_free_index(), 1);

	checkpoint::store_clear(-1);
	EXPECT_FALSE(checkpoint::store_has(-1));
}

TEST_F(CheckpointStoreTest, damaged_file_has_no_slots)
{
	ASSERT_TRUE(checkpoint::store_save(1, {1, 2, 3, 4}));

	// cut the file short
	SCP_string name = ".checkpoint_store_test.chk";
	auto fp = cfopen(name.c_str(), "wb", CF_TYPE_PLAYERS, false, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	ASSERT_NE(fp, nullptr);
	const char junk[] = "FSCK";
	cfwrite(junk, 1, 4, fp);
	cfclose(fp);

	EXPECT_FALSE(checkpoint::store_has(-1));
	checkpoint::Buffer loaded;
	EXPECT_FALSE(checkpoint::store_load(1, loaded));
}

TEST(CheckpointEventFlag, is_saved_with_the_mission_by_name)
{
	bool found = false;
	for (int i = 0; i < Num_mission_event_flags; ++i) {
		if (!stricmp(Mission_event_flags[i].name, "not checkpointed")) {
			EXPECT_EQ(Mission_event_flags[i].def, MEF_NOT_CHECKPOINTED);
			found = true;
		}
	}
	EXPECT_TRUE(found);
}
