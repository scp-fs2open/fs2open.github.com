
#include <gtest/gtest.h>
#include <graphics/font.h>

#include "util/FSTestFixture.h"

class CFileTest : public test::FSTestFixture {
 public:
	CFileTest() : test::FSTestFixture(INIT_NONE) {
		pushModDir("cfile");
	}

 protected:
	virtual void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	virtual void TearDown() override {
		test::FSTestFixture::TearDown();

		cfile_close();
	}
};

TEST_F(CFileTest, wrong_data_case) {
	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	ASSERT_FALSE(cfile_init(cfile_dir.c_str()));

	ASSERT_TRUE(cf_exists("ships.tbl", CF_TYPE_TABLES));
}

TEST_F(CFileTest, right_data_case) {
	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	ASSERT_FALSE(cfile_init(cfile_dir.c_str()));

	ASSERT_TRUE(cf_exists("ships.tbl", CF_TYPE_TABLES));
}
