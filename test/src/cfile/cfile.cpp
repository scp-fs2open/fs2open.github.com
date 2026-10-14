
#include <cfile/cfile.h>
#include <cfile/cfilesystem.h>
#include <graphics/font.h>
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

class CFileInitTest : public test::FSTestFixture {
 public:
	CFileInitTest() : test::FSTestFixture(INIT_NONE) {
		pushModDir("cfile");
	}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
		test::FSTestFixture::TearDown();

		cfile_close();
	}
};

TEST_F(CFileInitTest, wrong_data_case) {
	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	ASSERT_FALSE(cfile_init(cfile_dir.c_str()));

	ASSERT_TRUE(cf_exists("ships.tbl", CF_TYPE_TABLES));
}

TEST_F(CFileInitTest, right_data_case) {
	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	ASSERT_FALSE(cfile_init(cfile_dir.c_str()));

	ASSERT_TRUE(cf_exists("ships.tbl", CF_TYPE_TABLES));
}

class CFileTest : public test::FSTestFixture {
 public:
	CFileTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("cfile");
	}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
		extern bool Skip_memory_files;
		Skip_memory_files = false;

		test::FSTestFixture::TearDown();

		cfile_close();
	}
};

TEST_F(CFileTest, list_files_in_vps_and_dirs) {
	SCP_vector<SCP_string> table_files;
	extern bool Skip_memory_files;

	// For this test we need to skip the memory files to keep the results consistent
	Skip_memory_files = true;
	ASSERT_EQ(4, cf_get_file_list(table_files, CF_TYPE_TABLES, "*", CF_SORT_NAME));

	ASSERT_EQ((size_t)4, table_files.size());

	ASSERT_STREQ("dir", table_files[0].c_str());
	ASSERT_STREQ("dir2", table_files[1].c_str());

	ASSERT_STREQ("test", table_files[2].c_str());
	ASSERT_STREQ("test2", table_files[3].c_str());

	table_files.clear();
	extern int Skip_packfile_search;
	Skip_packfile_search = 1;
	ASSERT_EQ(2, cf_get_file_list(table_files, CF_TYPE_TABLES, "*", CF_SORT_NAME));
	Skip_packfile_search = 0;

	ASSERT_EQ((size_t)2, table_files.size());

	ASSERT_STREQ("dir", table_files[0].c_str());
	ASSERT_STREQ("dir2", table_files[1].c_str());
}

TEST_F(CFileTest, access_default_file) {
	// We use the controlconfig file since that should stay relatively stable
	ASSERT_TRUE(cf_exists("controlconfigdefaults.tbl", CF_TYPE_TABLES));

	auto fp = cfopen("controlconfigdefaults.tbl", "rb", CF_TYPE_TABLES);
	ASSERT_TRUE(fp != nullptr);

	ASSERT_EQ(28, cfilelength(fp));

	cfclose(fp);
}

TEST_F(CFileTest, override_default_file) {
	// We use the controlconfig file since that should stay relatively stable
	ASSERT_TRUE(cf_exists("controlconfigdefaults.tbl", CF_TYPE_TABLES));

	auto fp = cfopen("controlconfigdefaults.tbl", "rb", CF_TYPE_TABLES);
	ASSERT_TRUE(fp != nullptr);

	ASSERT_EQ(66, cfilelength(fp));

	cfclose(fp);
}

TEST(CFileStandalone, test_check_location_flags) {
	ASSERT_FALSE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT, CF_LOCATION_ROOT_USER));
	ASSERT_TRUE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT, CF_LOCATION_ROOT_GAME));
	ASSERT_TRUE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT, CF_LOCATION_TYPE_ROOT));
	ASSERT_FALSE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT, CF_LOCATION_TYPE_PRIMARY_MOD));
	ASSERT_FALSE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT,
	                                     CF_LOCATION_TYPE_SECONDARY_MODS));
	ASSERT_FALSE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT,
	                                     CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_PRIMARY_MOD));
	ASSERT_TRUE(cf_check_location_flags(CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT,
	                                    CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT));
}
TEST_F(CFileTest, test_get_path_type)
{
	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	ASSERT_FALSE(cfile_init(cfile_dir.c_str()));

	// Test simple case
	ASSERT_EQ(CF_TYPE_DATA, cfile_get_path_type("data"));
	ASSERT_EQ(CF_TYPE_DATA, cfile_get_path_type("///data"));
	ASSERT_EQ(CF_TYPE_DATA, cfile_get_path_type("data\\\\\\"));
	ASSERT_EQ(CF_TYPE_DATA, cfile_get_path_type("////data\\\\\\"));

	// Test single subdirectory case
	ASSERT_EQ(CF_TYPE_VOICE, cfile_get_path_type("data/voice"));
	ASSERT_EQ(CF_TYPE_VOICE, cfile_get_path_type("///data/voice"));
	ASSERT_EQ(CF_TYPE_VOICE, cfile_get_path_type("data/voice\\\\\\"));
	ASSERT_EQ(CF_TYPE_VOICE, cfile_get_path_type("////data/voice\\\\\\"));

	// Test nested subdirectory case
	ASSERT_EQ(CF_TYPE_VOICE_SPECIAL, cfile_get_path_type("data/voice/special"));
	ASSERT_EQ(CF_TYPE_VOICE_SPECIAL, cfile_get_path_type("///data/voice/special"));
	ASSERT_EQ(CF_TYPE_VOICE_SPECIAL, cfile_get_path_type("data/voice/special\\\\\\"));
	ASSERT_EQ(CF_TYPE_VOICE_SPECIAL, cfile_get_path_type("////data/voice/special\\\\\\"));
}

TEST_F(CFileTest, subfolders)
{
	// base check for all files
	ASSERT_TRUE(cf_exists("file.tbl", CF_TYPE_TABLES));
	ASSERT_TRUE(cf_exists("file2.tbl", CF_TYPE_TABLES));
	ASSERT_TRUE(cf_exists("file3.tbl", CF_TYPE_TABLES));

	// good direct subfolder check
	ASSERT_TRUE(cf_exists("sub/file2.tbl", CF_TYPE_TABLES));

	// bad direct subfolder check
	ASSERT_FALSE(cf_exists("sub/file3.tbl", CF_TYPE_TABLES));

	// sub-subfolder check
	ASSERT_TRUE(cf_exists("sub/folder/file3.tbl", CF_TYPE_TABLES));

	// check cf_find_file_location() with and without subfolders
	auto loc = cf_find_file_location("file2.tbl", CF_TYPE_TABLES);
	ASSERT_TRUE(loc.found);

	loc = cf_find_file_location("sub/file2.tbl", CF_TYPE_ANY);
	ASSERT_TRUE(loc.found);

	// check that cf_find_file_location_ext() works with subfolders
	const char *exts[] = { ".tbl", ".cfg" };
	loc = cf_find_file_location_ext("sub/folder/file3.tbl", 2, exts, CF_TYPE_TABLES);
	ASSERT_TRUE(loc.found);
}

TEST_F(CFileTest, subfolder_list)
{
	SCP_vector<SCP_string> table_files;
	extern bool Skip_memory_files;

	// For this test we need to skip the memory files to keep the results consistent
	Skip_memory_files = true;

	// look for all tables (one result, shadowing second file)
	ASSERT_EQ(1, cf_get_file_list(table_files, CF_TYPE_TABLES, "*.tbl", CF_SORT_NAME));

	// look for file in subfolder only, with unix and windows directory separators
	table_files.clear();
	ASSERT_EQ(1, cf_get_file_list(table_files, CF_TYPE_TABLES, "folder/*.tbl", CF_SORT_NAME));
	ASSERT_TRUE(table_files.front().substr(0, 6) == "folder");

	table_files.clear();
	ASSERT_EQ(1, cf_get_file_list(table_files, CF_TYPE_TABLES, "folder\\*.tbl", CF_SORT_NAME));
	ASSERT_TRUE(table_files.front().substr(0, 6) == "folder");

	// subfolder glob, with unix and windows directory separators
	// should return two files, one with a sub path
	table_files.clear();
	ASSERT_EQ(2, cf_get_file_list(table_files, CF_TYPE_TABLES, "*/*.tbl", CF_SORT_NAME));
	ASSERT_TRUE(table_files.back().substr(0, 6) == "folder");

	table_files.clear();
	ASSERT_EQ(2, cf_get_file_list(table_files, CF_TYPE_TABLES, "*\\*.tbl", CF_SORT_NAME));
	ASSERT_TRUE(table_files.back().substr(0, 6) == "folder");
}

namespace {
// Builds the bytes cfwrite_string_len writes (a little-endian length, then the characters) as an in-memory file.
// The file reads from this object's buffer, so the object has to outlive the CFILE.
class StringLenFile {
  public:
	StringLenFile& add_int(int value)
	{
		for (int i = 0; i < 4; ++i) {
			_bytes.push_back(static_cast<char>((value >> (8 * i)) & 0xFF));
		}
		return *this;
	}

	StringLenFile& add_chars(const char* chars, size_t count)
	{
		_bytes.insert(_bytes.end(), chars, chars + count);
		return *this;
	}

	CFILE* open() const
	{
		CFileLocation res(true);
		res.name_ext = "string_len.bin";
		res.full_name = "string_len.bin";
		res.data_ptr = _bytes.data();
		res.size = _bytes.size();
		return cfopen_special(res, "rb");
	}

  private:
	SCP_vector<char> _bytes;
};
} // namespace

TEST_F(CFileTest, string_len_fits)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8];
	cfread_string_len(buf, sizeof(buf), fp);
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_exact_fit_with_stored_terminator)
{
	StringLenFile file;
	file.add_int(4).add_chars("abc\0", 4).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[4];
	cfread_string_len(buf, sizeof(buf), fp);
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_longer_than_buffer)
{
	StringLenFile file;
	file.add_int(10).add_chars("0123456789", 10).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	cfread_string_len(buf, 4, fp);
	EXPECT_STREQ(buf, "012");
	for (int i = 4; i < 8; ++i) {
		EXPECT_EQ(buf[i], 'X') << "wrote past the buffer at " << i;
	}
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_zero_and_negative)
{
	StringLenFile file;
	file.add_int(0).add_int(-5).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8] = "old";
	cfread_string_len(buf, sizeof(buf), fp);
	EXPECT_STREQ(buf, "");

	strcpy_s(buf, "old");
	cfread_string_len(buf, sizeof(buf), fp);
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_scp_string)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3).add_int(-5).add_int(1000000).add_chars("de", 2);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	EXPECT_EQ(cfread_string_len(fp), "abc");
	EXPECT_EQ(cfread_string_len(fp), "");
	EXPECT_EQ(cfread_string_len(fp), "de");

	cfclose(fp);
}
