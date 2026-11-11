
#include <cfile/cfile.h>
#include <cfile/cfilesystem.h>
#include <globalincs/systemvars.h>
#include <graphics/font.h>
#include <gtest/gtest.h>
#include <osapi/dialogs.h>
#include <utils/finally.h>

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

// Debug test builds turn Warning() into an exception, Release builds only count it
template <typename Read>
void expect_warning(const char* text, Read read)
{
#ifndef NDEBUG
	try {
		read();
		ADD_FAILURE() << "expected a warning containing '" << text << "'";
	} catch (const os::dialogs::WarningException& e) {
		EXPECT_NE(SCP_string(e.what()).find(text), SCP_string::npos) << e.what();
	}
#else
	int warnings = Global_warning_count;
	read();
	EXPECT_EQ(Global_warning_count, warnings + 1) << "expected one warning containing '" << text << "'";
#endif
}

template <typename Read>
void expect_no_warning(Read read)
{
	int warnings = Global_warning_count;
	EXPECT_NO_THROW(read());
	EXPECT_EQ(Global_warning_count, warnings);
}
} // namespace

TEST_F(CFileTest, string_len_fits)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_one_short_of_the_buffer)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[4];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_empty_inside_max_read_len)
{
	StringLenFile file;
	file.add_int(0).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 8);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cfread_int(fp), 42);
	cfclose(fp);

	fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 4);
	SCP_string str = "old";
	expect_no_warning([&] { str = cfread_string_len(fp); });
	EXPECT_EQ(str, "");
	EXPECT_EQ(cftell(fp), 4);
	cfclose(fp);
}

TEST_F(CFileTest, string_len_clean_end_of_file_is_silent)
{
	StringLenFile file;
	file.add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	ASSERT_EQ(cfread_int(fp), 42);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cftell(fp), 4);

	SCP_string str = "old";
	expect_no_warning([&] { str = cfread_string_len(fp); });
	EXPECT_EQ(str, "");
	EXPECT_EQ(cftell(fp), 4);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_without_a_file_reads_empty)
{
	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), nullptr); });
	EXPECT_STREQ(buf, "");

	SCP_string str = "old";
	expect_no_warning([&] { str = cfread_string_len(nullptr); });
	EXPECT_EQ(str, "");
}

TEST_F(CFileTest, string_len_exact_fit_with_stored_terminator)
{
	StringLenFile file;
	file.add_int(4).add_chars("abc\0", 4).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[4];
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
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
	expect_warning("was cut", [&] { cfread_string_len(buf, 4, fp); });
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
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");

	strcpy_s(buf, "old");
	expect_warning("corrupt string length -5", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_scp_string)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3).add_int(-5).add_int(42).add_int(1000000).add_chars("de", 2);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	SCP_string fits;
	expect_no_warning([&] { fits = cfread_string_len(fp); });
	EXPECT_EQ(fits, "abc");
	// Debug test builds throw at the warning, before the string is returned, so only the position is checked there
	SCP_string negative = "old";
	expect_warning("corrupt string length -5", [&] { negative = cfread_string_len(fp); });
	EXPECT_EQ(cfread_int(fp), 42);
	SCP_string past_end;
	expect_warning("corrupt string length 1000000", [&] { past_end = cfread_string_len(fp); });
	EXPECT_EQ(cftell(fp), cfilelength(fp));
#ifdef NDEBUG
	EXPECT_EQ(negative, "");
	EXPECT_EQ(past_end, "de");
#endif

	cfclose(fp);
}

TEST_F(CFileTest, string_len_exact_fit_without_terminator)
{
	StringLenFile file;
	file.add_int(4).add_chars("abcd", 4).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[4];
	expect_warning("was cut", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_past_end_of_file)
{
	StringLenFile file;
	file.add_int(20).add_chars("hello", 5);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[32];
	memset(buf, 'X', sizeof(buf));
	expect_warning("corrupt string length 20", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "hello");
	EXPECT_EQ(cftell(fp), cfilelength(fp));

	cfclose(fp);
}

TEST_F(CFileTest, string_len_respects_max_read_len)
{
	StringLenFile file;
	file.add_int(12).add_chars("0123456789ab", 12).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, 10);
	char buf[4];
	EXPECT_THROW(cfread_string_len(buf, sizeof(buf), fp), cfile::max_read_length);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_inside_max_read_len)
{
	StringLenFile file;
	file.add_int(6).add_chars("abcdef", 6).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 10);

	char buf[8];
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abcdef");
	EXPECT_EQ(cftell(fp), 10);
	EXPECT_THROW(cfread_int(fp), cfile::max_read_length);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_cut_at_max_read_len)
{
	StringLenFile file;
	file.add_int(6).add_chars("abcdef", 6).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 10);

	char buf[4];
	expect_warning("was cut", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cftell(fp), 10);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_negative_inside_max_read_len)
{
	StringLenFile file;
	file.add_int(-5).add_chars("abcdefgh", 8).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 8);

	char buf[8];
	EXPECT_THROW(cfread_string_len(buf, sizeof(buf), fp), cfile::max_read_length);
	cfclose(fp);

	auto fp2 = file.open();
	ASSERT_NE(fp2, nullptr);
	cf_set_max_read_len(fp2, 8);
	EXPECT_THROW(cfread_string_len(fp2), cfile::max_read_length);
	cfclose(fp2);
}

TEST_F(CFileTest, string_len_scp_string_respects_max_read_len)
{
	StringLenFile file;
	file.add_int(12).add_chars("0123456789ab", 12).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, 10);
	EXPECT_THROW(cfread_string_len(fp), cfile::max_read_length);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_nul_inside_buffer_is_not_cut)
{
	StringLenFile file;
	file.add_int(10).add_chars("ab\0defghij", 10).add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, 4, fp); });
	EXPECT_STREQ(buf, "ab");
	EXPECT_EQ(buf[3], 0);
	for (int i = 4; i < 8; ++i) {
		EXPECT_EQ(buf[i], 'X') << "wrote past the buffer at " << i;
	}
	EXPECT_EQ(cfread_int(fp), 42);

	cfclose(fp);
}

TEST_F(CFileTest, string_len_corrupt_and_cut_warns_once)
{
	StringLenFile file;
	file.add_int(20).add_chars("hello", 5);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[4];
	memset(buf, 'X', sizeof(buf));
	expect_warning("corrupt string length 20", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "hel");
	EXPECT_EQ(cftell(fp), cfilelength(fp));

	cfclose(fp);
}

namespace {
// A file on disk that holds fewer bytes than its recorded size, as when it shrinks after it was opened, so reads come
// up short. The one-byte offset opens it the way a VP entry is opened, which takes the size from the location.
CFILE* open_short_file(const SCP_string& path, const SCP_vector<char>& bytes, size_t recorded_size)
{
	FILE* fp = fopen(path.c_str(), "wb");
	if (fp == nullptr) {
		return nullptr;
	}
	bool written = (fputc(0, fp) == 0) && (fwrite(bytes.data(), 1, bytes.size(), fp) == bytes.size());
	if ((fclose(fp) != 0) || !written) {
		return nullptr;
	}

	CFileLocation res(true);
	res.name_ext = "short.bin";
	res.full_name = path;
	res.offset = 1;
	res.size = recorded_size;
	return cfopen_special(res, "rb");
}

SCP_vector<char> length_then_chars(int len, const char* chars)
{
	SCP_vector<char> bytes;
	for (int i = 0; i < 4; ++i) {
		bytes.push_back(static_cast<char>((len >> (8 * i)) & 0xFF));
	}
	bytes.insert(bytes.end(), chars, chars + strlen(chars));
	return bytes;
}
} // namespace

TEST_F(CFileTest, string_len_failed_read_skips_the_string)
{
	SCP_string path = testing::TempDir() + "cfile_short_read.bin";
	// The guard closes the file before removing it: Windows cannot delete an open file, and a failed ASSERT returns
	// with the file still open.
	CFILE* fp = nullptr;
	auto remove_file = util::finally([&] {
		if (fp != nullptr) {
			cfclose(fp);
		}
		remove(path.c_str());
	});
	fp = open_short_file(path, length_then_chars(10, "hel"), 18);
	ASSERT_NE(fp, nullptr);

	char buf[16] = "old";
	expect_warning("could not read", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cftell(fp), 14);
	cfclose(fp);
	fp = nullptr;

	fp = open_short_file(path, length_then_chars(10, "hel"), 18);
	ASSERT_NE(fp, nullptr);
	SCP_string str = "old";
	expect_warning("could not read", [&] { str = cfread_string_len(fp); });
#ifdef NDEBUG
	EXPECT_EQ(str, "");
#endif
	EXPECT_EQ(cftell(fp), 14);
}

#ifdef NDEBUG
TEST_F(CFileTest, string_len_zero_buffer_warns_and_writes_nothing)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[2] = {'X', 'X'};
	int warnings = Global_warning_count;
	cfread_string_len(buf + 1, 0, fp);
	EXPECT_EQ(buf[0], 'X');
	EXPECT_EQ(buf[1], 'X');
	EXPECT_EQ(Global_warning_count, warnings + 1);
	EXPECT_EQ(cftell(fp), cfilelength(fp));

	cfclose(fp);
}
#endif

TEST_F(CFileTest, string_len_negative_in_last_section_throws)
{
	StringLenFile file;
	file.add_int(-5);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, cfilelength(fp));
	char buf[8];
	EXPECT_THROW(cfread_string_len(buf, sizeof(buf), fp), cfile::max_read_length);
	cfclose(fp);

	fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, cfilelength(fp));
	EXPECT_THROW(cfread_string_len(fp), cfile::max_read_length);
	cfclose(fp);
}

TEST_F(CFileTest, string_len_past_end_in_last_section_warns)
{
	StringLenFile file;
	file.add_int(1000000).add_chars("hi", 2);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, cfilelength(fp));
	SCP_string str;
	expect_warning("corrupt string length 1000000", [&] { str = cfread_string_len(fp); });
#ifdef NDEBUG
	EXPECT_EQ(str, "hi");
#endif
	EXPECT_EQ(cftell(fp), cfilelength(fp));
	cfclose(fp);
}

TEST_F(CFileTest, string_len_truncated_section_reads_what_is_left)
{
	StringLenFile file;
	file.add_int(20).add_chars("abc", 3);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	// the section claims more bytes than the file still has, as in a truncated pilot file
	cf_set_max_read_len(fp, 10);
	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_warning("corrupt string length 20", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "abc");
	EXPECT_EQ(cftell(fp), cfilelength(fp));
	cfclose(fp);
}

TEST_F(CFileTest, string_len_cut_off_length_field_warns)
{
	StringLenFile file;
	file.add_chars("\x05\x00", 2);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[8] = "old";
	expect_warning("length field", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cftell(fp), cfilelength(fp));
	cfclose(fp);

	fp = file.open();
	ASSERT_NE(fp, nullptr);
	SCP_string str = "old";
	expect_warning("length field", [&] { str = cfread_string_len(fp); });
#ifdef NDEBUG
	EXPECT_EQ(str, "");
#endif
	EXPECT_EQ(cftell(fp), cfilelength(fp));
	cfclose(fp);
}

#ifdef NDEBUG
TEST_F(CFileTest, string_len_zero_buffer_still_warns_on_corrupt_length)
{
	StringLenFile file;
	file.add_int(-5);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[2] = {'X', 'X'};
	expect_warning("corrupt string length -5", [&] { cfread_string_len(buf + 1, 0, fp); });
	EXPECT_EQ(buf[0], 'X');
	EXPECT_EQ(buf[1], 'X');
	cfclose(fp);
}
#endif

TEST_F(CFileTest, string_len_cut_off_length_field_inside_max_read_len_warns)
{
	StringLenFile file;
	file.add_chars("\x05\x00", 2);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, cfilelength(fp));
	char buf[8] = "old";
	expect_warning("length field", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cftell(fp), cfilelength(fp));
	cfclose(fp);
}

TEST_F(CFileTest, string_len_clamped_but_still_past_max_read_len_throws)
{
	StringLenFile file;
	file.add_int(20).add_chars("0123456789", 10);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	// a 2-byte buffer keeps the body read inside the limit, so only the header's check can throw
	cf_set_max_read_len(fp, 6);
	char buf[2];
	EXPECT_THROW(cfread_string_len(buf, sizeof(buf), fp), cfile::max_read_length);
	cfclose(fp);
}

TEST_F(CFileTest, string_len_length_field_cut_by_max_read_len_throws)
{
	StringLenFile file;
	file.add_int(5).add_chars("abcde", 5);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, 2);
	char buf[8];
	EXPECT_THROW(cfread_string_len(buf, sizeof(buf), fp), cfile::max_read_length);
	cfclose(fp);
}

TEST_F(CFileTest, cfread_keeps_whole_elements_at_end_of_file)
{
	StringLenFile file;
	file.add_chars("abcdef", 6);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[12];
	EXPECT_EQ(cfread(buf, 4, 3, fp), 1);
	EXPECT_EQ(cftell(fp), 6);
	cfclose(fp);
}

TEST_F(CFileTest, cfread_at_end_of_file_does_not_check_max_read_len)
{
	StringLenFile file;
	file.add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	cf_set_max_read_len(fp, 2);
	cfseek(fp, 0, CF_SEEK_END);
	int value = 0;
	EXPECT_NO_THROW(EXPECT_EQ(cfread(&value, sizeof(value), 1, fp), 0));
	cfclose(fp);
}

TEST_F(CFileTest, string_len_clean_end_of_file_inside_max_read_len_is_silent)
{
	StringLenFile file;
	file.add_int(42);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);
	cf_set_max_read_len(fp, 4);
	ASSERT_EQ(cfread_int(fp), 42);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_no_warning([&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	cfclose(fp);
}

TEST_F(CFileTest, string_len_missing_length_field_before_the_end_warns)
{
	SCP_string path = testing::TempDir() + "cfile_missing_length.bin";
	CFILE* fp = nullptr;
	auto remove_file = util::finally([&] {
		if (fp != nullptr) {
			cfclose(fp);
		}
		remove(path.c_str());
	});
	fp = open_short_file(path, {}, 8);
	ASSERT_NE(fp, nullptr);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_warning("length field", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
}

#ifdef NDEBUG
TEST_F(CFileTest, string_len_zero_buffer_and_empty_string_is_silent)
{
	StringLenFile file;
	file.add_int(0);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[2] = {'X', 'X'};
	expect_no_warning([&] { cfread_string_len(buf + 1, 0, fp); });
	EXPECT_EQ(buf[1], 'X');
	cfclose(fp);
}
#endif

TEST_F(CFileTest, string_len_length_field_cut_short_on_disk_warns)
{
	SCP_string path = testing::TempDir() + "cfile_short_length.bin";
	CFILE* fp = nullptr;
	auto remove_file = util::finally([&] {
		if (fp != nullptr) {
			cfclose(fp);
		}
		remove(path.c_str());
	});
	fp = open_short_file(path, {'\x05', '\x00'}, 18);
	ASSERT_NE(fp, nullptr);

	char buf[8];
	memset(buf, 'X', sizeof(buf));
	expect_warning("length field", [&] { cfread_string_len(buf, sizeof(buf), fp); });
	EXPECT_STREQ(buf, "");
	EXPECT_EQ(cftell(fp), 2);
}

#ifndef NDEBUG
TEST_F(CFileTest, string_len_zero_buffer_asserts)
{
	StringLenFile file;
	file.add_int(3).add_chars("abc", 3);
	auto fp = file.open();
	ASSERT_NE(fp, nullptr);

	char buf[2] = {'X', 'X'};
	EXPECT_THROW(cfread_string_len(buf + 1, 0, fp), os::dialogs::AssertException);
	EXPECT_EQ(buf[0], 'X');
	EXPECT_EQ(buf[1], 'X');
	cfclose(fp);
}
#endif
