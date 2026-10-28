#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

#include <cstdio>

class OsRegistryTest : public test::FSTestFixture {
  public:
	OsRegistryTest() : test::FSTestFixture(INIT_NONE) {}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		_saveFoundBefore = Ingame_options_save_found;
		// os_init already read the settings files; start again with none
		ASSERT_NO_FATAL_FAILURE(startWithNoFiles());
	}

	static void startWithNoFiles()
	{
		os_deinit_registry_stuff();
		removeSettingsFiles();
		// A file that survived removal would hand the writes an existing profile and hide the bug
		ASSERT_FALSE(fileExists(modFilePath()));
		ASSERT_FALSE(fileExists(mainFilePath()));
		os_init_registry_stuff(Osreg_company_name, "Test");
	}

	void TearDown() override
	{
		test::FSTestFixture::TearDown();
		removeSettingsFiles();
		// Nothing resets this flag, and CI shuffles test order
		Ingame_options_save_found = _saveFoundBefore;
	}

	// Startup marks the mod file as missing and nothing sets that back, so a reload has to be told the file is there
	static void reloadFromDisk()
	{
		os_deinit_registry_stuff();
		Ingame_options_save_found = true;
		os_init_registry_stuff(Osreg_company_name, "Test");
	}

  private:
	bool _saveFoundBefore = true;

	static SCP_string modFilePath()
	{
		auto test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
		return os_get_config_path(SCP_string("data/") + test_name + "_settings.ini");
	}

	static SCP_string mainFilePath()
	{
		return os_get_config_path(Osreg_config_file_name);
	}

	static bool fileExists(const SCP_string& path)
	{
		auto fp = std::fopen(path.c_str(), "r");
		if (fp == nullptr) {
			return false;
		}
		std::fclose(fp);
		return true;
	}

	static void removeSettingsFiles()
	{
		std::remove(modFilePath().c_str());
		std::remove(mainFilePath().c_str());
	}
};

// Only the first write starts from no profile, so each writer goes first once
TEST_F(OsRegistryTest, ModFileKeepsEveryWriteWithoutASettingsFile)
{
	os_config_write_uint("Test", "Second", 2, true);
	os_config_write_string("Test", "First", "one", true);

	ASSERT_STREQ(os_config_read_string("Test", "First", "missing", true), "one");
	ASSERT_EQ(os_config_read_uint("Test", "Second", 0, true), 2u);

	reloadFromDisk();

	ASSERT_STREQ(os_config_read_string("Test", "First", "missing", true), "one");
	ASSERT_EQ(os_config_read_uint("Test", "Second", 0, true), 2u);

	ASSERT_NO_FATAL_FAILURE(startWithNoFiles());
	os_config_write_string("Test", "Third", "three", true);
	os_config_write_uint("Test", "Fourth", 4, true);

	ASSERT_STREQ(os_config_read_string("Test", "Third", "missing", true), "three");
	ASSERT_EQ(os_config_read_uint("Test", "Fourth", 0, true), 4u);
}

// Windows sends these writes to the registry when there is no fs2_open.ini
#ifndef WIN32
TEST_F(OsRegistryTest, MainFileKeepsEveryWriteWithoutASettingsFile)
{
	os_config_write_uint("Test", "Second", 2);
	os_config_write_string("Test", "First", "one");

	ASSERT_STREQ(os_config_read_string("Test", "First", "missing"), "one");
	ASSERT_EQ(os_config_read_uint("Test", "Second", 0), 2u);

	reloadFromDisk();

	ASSERT_STREQ(os_config_read_string("Test", "First", "missing"), "one");
	ASSERT_EQ(os_config_read_uint("Test", "Second", 0), 2u);

	ASSERT_NO_FATAL_FAILURE(startWithNoFiles());
	os_config_write_string("Test", "Third", "three");
	os_config_write_uint("Test", "Fourth", 4);

	ASSERT_STREQ(os_config_read_string("Test", "Third", "missing"), "three");
	ASSERT_EQ(os_config_read_uint("Test", "Fourth", 0), 4u);
}
#endif
