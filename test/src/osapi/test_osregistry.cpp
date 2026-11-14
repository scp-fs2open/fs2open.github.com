#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class OsRegistryNoProfile : public test::FSTestFixture {
  public:
	OsRegistryNoProfile() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("osapi");
	}

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		_save_found = Ingame_options_save_found;

		// Start like a first launch: with the flag false the mod settings load from fs2_open.ini, which must be absent
		FILE* ini = fopen(os_get_config_path("fs2_open.ini").c_str(), "r");
		const bool ini_found = ini != nullptr;
		if (ini_found) {
			fclose(ini);
		}
		ASSERT_FALSE(ini_found) << "test_data holds an fs2_open.ini, which hides the no-profile case";

		os_deinit_registry_stuff();
		Ingame_options_save_found = false;
		os_init_registry_stuff(nullptr, "Test");
	}
	void TearDown() override
	{
		Ingame_options_save_found = _save_found;

		test::FSTestFixture::TearDown();
	}

	static void reload_from_saved_file()
	{
		os_deinit_registry_stuff();
		Ingame_options_save_found = true;
		os_init_registry_stuff(nullptr, "Test");
	}

	bool _save_found = true;
};

TEST_F(OsRegistryNoProfile, FirstStringWriteKeepsItsProfile)
{
	os_config_write_string("Test", "First", "one", true);
	os_config_write_uint("Test", "Second", 2, true);

	reload_from_saved_file();

	EXPECT_STREQ(os_config_read_string("Test", "First", "missing", true), "one");
	EXPECT_EQ(os_config_read_uint("Test", "Second", 0, true), 2u);
}

TEST_F(OsRegistryNoProfile, FirstUintWriteKeepsItsProfile)
{
	os_config_write_uint("Test", "First", 1, true);
	os_config_write_string("Test", "Second", "two", true);

	reload_from_saved_file();

	EXPECT_EQ(os_config_read_uint("Test", "First", 0, true), 1u);
	EXPECT_STREQ(os_config_read_string("Test", "Second", "missing", true), "two");
}
