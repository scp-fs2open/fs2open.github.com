#include <gtest/gtest.h>
#include <bmpman/bmpman.h>
#include <cfile/cfile.h>

#include "util/FSTestFixture.h"

class EffTest : public test::FSTestFixture {
public:
	EffTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("bmpman");
		pushModDir("eff");
	}
};

TEST_F(EffTest, reads_an_eff_longer_than_1024_bytes) {
	int frames = 0, fps = 0, key = 0;
	BM_TYPE type = BM_TYPE_NONE;
	ASSERT_TRUE(bm_load_and_parse_eff("long_eff.eff", CF_TYPE_MAPS, &frames, &fps, &key, &type));
	EXPECT_EQ(frames, 2);
	EXPECT_EQ(fps, 15);
	EXPECT_EQ(key, 1);
	EXPECT_EQ(type, BM_TYPE_PNG);
}
