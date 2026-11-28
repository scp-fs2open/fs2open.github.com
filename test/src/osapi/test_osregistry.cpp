#include "cmdline/cmdline.h"
#include "options/OptionsManager.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "util/FSTestFixture.h"

#include <SDL3/SDL_filesystem.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

class OsRegistryTest : public test::FSTestFixture
{
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

// OptionsManager reads persisted options back from the mod settings profile, not from a cache
TEST_F(OsRegistryTest, PersistedOptionsReadBackInTheSameSession)
{
	auto mgr = options::OptionsManager::instance();

	mgr->setConfigValue("Test.First", std::unique_ptr<json_t>(json_integer(1)));
	mgr->setConfigValue("Test.Second", std::unique_ptr<json_t>(json_integer(2)));
	mgr->persistChanges();

	auto first = mgr->getValueFromConfig("Test.First");
	ASSERT_TRUE(first.has_value());
	ASSERT_EQ(json_integer_value(first->get()), 1);

	auto second = mgr->getValueFromConfig("Test.Second");
	ASSERT_TRUE(second.has_value());
	ASSERT_EQ(json_integer_value(second->get()), 2);
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

extern SCP_string Mod_options_file_name;

namespace {
// a fs2_open.ini and a retail profile in a fresh folder, so startup's fall back to fs2_open.ini can be seen
class RetailProfileFolder {
  public:
	RetailProfileFolder()
		: _modBefore(Cmdline_mod), _profileBefore(Mod_options_file_name), _mainBefore(Osreg_config_file_name)
	{
		_ok = SDL_CreateDirectory(os_get_config_path(Folder).c_str());
		std::ofstream main(os_get_config_path(MainFile));
		main << "[Test]\nValue=global\n";
		_ok = _ok && main.good();
		Cmdline_mod = nullptr;
		Osreg_config_file_name = MainFile;
	}
	~RetailProfileFolder()
	{
		os_deinit_registry_stuff();
		SDL_RemovePath(os_get_config_path(ProfileFile).c_str());
		SDL_RemovePath(os_get_config_path(DataFolder).c_str());
		SDL_RemovePath(os_get_config_path(MainFile).c_str());
		SDL_RemovePath(os_get_config_path(Folder).c_str());
		Cmdline_mod = _modBefore;
		Mod_options_file_name = _profileBefore;
		Osreg_config_file_name = _mainBefore;
		Cmdline_get_options = false;
	}

	bool ok() const
	{
		return _ok;
	}

	bool makeDataFolder() const
	{
		return SDL_CreateDirectory(os_get_config_path(DataFolder).c_str());
	}

	static bool profileExists()
	{
		SDL_PathInfo info;
		return SDL_GetPathInfo(os_get_config_path(ProfileFile).c_str(), &info);
	}

	// what a launch with no -mod reads for Test.Value, as a query or as the game
	static SCP_string load(bool query)
	{
		os_deinit_registry_stuff();
		Cmdline_get_options = query;
		Ingame_options_save_found = true;
		Mod_options_file_name = ProfileFile;
		os_init_registry_stuff(Osreg_company_name, "Test");
		return os_config_read_string("Test", "Value", "none", true);
	}

  private:
	static constexpr const char* Folder = "retail_query_test";
	static constexpr const char* DataFolder = "retail_query_test/data";
	static constexpr const char* MainFile = "retail_query_test/fs2_open.ini";
	static constexpr const char* ProfileFile = "retail_query_test/data/retail_fs2_settings.ini";

	char* _modBefore;
	SCP_string _profileBefore;
	const char* _mainBefore;
	bool _ok = false;
};
} // namespace

// a launcher shows what the next launch will use, so the query must pick the profile startup picks, without creating it
TEST_F(OsRegistryTest, RetailQueryLoadsTheProfileStartupWould)
{
	RetailProfileFolder folder;
	ASSERT_TRUE(folder.ok());

	// with no folder for the retail profile, startup cannot create it and falls back to fs2_open.ini
	ASSERT_EQ(RetailProfileFolder::load(true), "global");
	ASSERT_EQ(RetailProfileFolder::load(false), "global");

	// with the folder there, startup creates an empty profile and reads that
	ASSERT_TRUE(folder.makeDataFolder());
	ASSERT_EQ(RetailProfileFolder::load(true), "none");
	ASSERT_FALSE(RetailProfileFolder::profileExists());
	ASSERT_EQ(RetailProfileFolder::load(false), "none");
	ASSERT_TRUE(RetailProfileFolder::profileExists());
}
