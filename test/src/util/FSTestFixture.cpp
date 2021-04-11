//
//

#include "FSTestFixture.h"

#include <cmdline/cmdline.h>
#include <cfile/cfilemod.h>
#include <graphics/2d.h>
#include <io/timer.h>
#include <localization/localize.h>
#include <mod_table/mod_table.h>
#include <osapi/outwnd.h>
#include <ship/ship.h>

test::FSTestFixture::FSTestFixture(uint64_t init_flags) : testing::Test(), _initFlags(init_flags) {
	addCommandlineArg("-parse_cmdline_only");
	addCommandlineArg("-standalone");
	addCommandlineArg("-portable_mode");
}
void test::FSTestFixture::SetUp() {
	auto currentTest = ::testing::UnitTest::GetInstance()->current_test_info();
	pushModDir(currentTest->name());

	init_cmdline();

	timer_init();

	if (LoggingEnabled) {
		outwnd_init();
		mprintf(("TEST: Setting up test '%s.%s'\n", currentTest->test_case_name(), currentTest->name()));
	}

	os_init("Test", "Test");

	if (_initFlags & INIT_CFILE) {
		SCP_string cfile_dir(TEST_DATA_PATH);

		if (cfile_init(Cmdline_mod)) {
			FAIL() << "Cfile init failed!";
		}

		if (_initFlags & INIT_MOD_TABLE) {
			mod_table_init();        // load in all the mod dependent settings
		}

		lcl_init(-1);
		lcl_xstr_init();

		if (_initFlags & INIT_MOD_TABLE) {
			mod_table_init(); // load in all the mod dependent settings
		}

		if (_initFlags & INIT_GRAPHICS) {
			if (!gr_init(nullptr, GR_STUB, 1024, 768)) {
				FAIL() << "Graphics init failed!";
			}

			if (_initFlags & INIT_FONTS) {
				font::init();
			}
		}

		if (_initFlags & INIT_SHIPS) {
			ship_init();
		}
	}
}
void test::FSTestFixture::TearDown() {
	if (_initFlags & INIT_CFILE) {
		if (_initFlags & INIT_SHIPS) {
			ship_close();
		}

		if (_initFlags & INIT_GRAPHICS) {
			if (_initFlags & INIT_FONTS) {
				font::close();
			}

			io::mouse::CursorManager::shutdown();

			bm_unload_all();

			gr_close();
		}

		if (_initFlags & INIT_MOD_TABLE) {
			// Reset mod settings again so that subsequent tests don't get broken
			mod_table_reset();
		}

		cfile_close();
	}

	timer_close();

	lcl_close();

	os_cleanup();

	if (LoggingEnabled) {
		outwnd_close();
	}
}
void test::FSTestFixture::addCommandlineArg(const SCP_string& arg) {
	_cmdlineArgs.push_back(arg);
}
void test::FSTestFixture::init_cmdline() {
	std::unique_ptr<char* []> parts(new char* [_cmdlineArgs.size()]);

	for (size_t i = 0; i < _cmdlineArgs.size(); ++i) {
		parts[i] = const_cast<char*>(_cmdlineArgs[i].c_str());
	}

	parse_cmdline(TEST_DATA_PATH, (int) _cmdlineArgs.size(), parts.get());
}
void test::FSTestFixture::pushModDir(const SCP_string& mod) {
	if (!_currentModDir.empty()) {
		// Don't prepend a / if we are at the root or else it would be an absolute path
		_currentModDir += DIR_SEPARATOR_CHAR;
	}
	_currentModDir += mod;

	addCommandlineArg("-mod");
	addCommandlineArg(_currentModDir);
}
