
#include <cfile/cfilesystem.h>
#include <graphics/font.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>

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

namespace {

class Lz41Test : public CFileTest {
protected:
	void SetUp() override
	{
		CFileTest::SetUp();
		const auto name = "fso-lz41-" + std::to_string(std::random_device{}());
		const auto temp_dir = std::filesystem::temp_directory_path() / name;
		ASSERT_TRUE(std::filesystem::create_directory(temp_dir));
		_temp_dir = temp_dir;
	}

	void TearDown() override
	{
		if (!_temp_dir.empty()) {
			std::error_code error;
			std::filesystem::remove(_temp_dir / "test.lz41", error);
			std::filesystem::remove(_temp_dir, error);
		}
		CFileTest::TearDown();
	}

	CFileLocation write_lz41(const std::string& bytes, bool packed)
	{
		const auto path = _temp_dir / "test.lz41";
		std::ofstream stream(path, std::ios::binary);
		const std::string padding(packed ? 32 : 0, '\0');
		stream << padding << bytes << padding;
		stream.close();
		EXPECT_TRUE(stream);

		CFileLocation location(true);
		location.full_name = path.string();
		location.name_ext = "test-wep.tbm";
		location.offset = padding.size();
		location.size = bytes.size();
		return location;
	}

	static void append_words(std::string& bytes, std::initializer_list<uint32_t> words)
	{
		for (const auto word : words) {
			for (unsigned shift = 0; shift < 32; shift += 8)
				bytes.push_back(static_cast<char>((word >> shift) & 0xff));
		}
	}

private:
	std::filesystem::path _temp_dir;
};

TEST_F(Lz41Test, rejects_oversized_compressed_block)
{
	std::string bytes = "LZ41";
	bytes.append(1000, '\0');
	append_words(bytes, {4, 1004, 2, 64, 64});

	for (bool packed : {false, true}) {
		const auto location = write_lz41(bytes, packed);
		const std::unique_ptr<CFILE> file(cfopen_special(location, "rb", CF_TYPE_TABLES));
		EXPECT_EQ(nullptr, file.get()) << "packed = " << packed;
	}
}

TEST_F(Lz41Test, reports_short_decoded_block_as_read_failure)
{
	constexpr int block_size = 1024 * 1024;
	// An independent LZ4 block containing just one literal byte.
	std::string bytes = "LZ41\x10x";
	append_words(bytes, {4, 6, 2, block_size, block_size});
	const auto location = write_lz41(bytes, true);
	const std::unique_ptr<CFILE> file(cfopen_special(location, "rb", CF_TYPE_TABLES));
	ASSERT_NE(nullptr, file.get());

	char buffer = '?';
	EXPECT_EQ(0, cfread(&buffer, 1, 1, file.get()));
	EXPECT_EQ(0, cftell(file.get()));
	EXPECT_TRUE(cfeof(file.get()));
}

} // namespace

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
