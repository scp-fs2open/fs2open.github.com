#include "cutscene/cutscenes.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

// Defined in cutscenes.cpp for shutdown; reuse it to free test descriptions.
void cutscene_close();

class CutsceneParseTest : public test::FSTestFixture {
  public:
	CutsceneParseTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("cutscene");
	}

  protected:
	void TearDown() override
	{
		cutscene_close();
		Cutscenes.clear();
		test::FSTestFixture::TearDown();
	}
};

TEST_F(CutsceneParseTest, ModularNocreateAppliesOnlyToItsEntry)
{
	cutscene_init();

	EXPECT_GE(get_cutscene_index_by_name("Intro Override"), 0);
	EXPECT_GE(get_cutscene_index_by_name("New Cutscene"), 0);
	EXPECT_LT(get_cutscene_index_by_name("Missing Cutscene"), 0);
}
