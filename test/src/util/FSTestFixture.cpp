//
//

#include "FSTestFixture.h"

#include <osapi/outwnd.h>
#include <cmdline/cmdline.h>
#include <graphics/2d.h>
#include <ship/ship.h>
#include <io/timer.h>
#include <localization/localize.h>

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

#ifndef NDEBUG
	outwnd_init();
	mprintf(("TEST: Setting up test '%s.%s'\n", currentTest->test_case_name(), currentTest->name()));
#endif

	os_init("Test", "Test");

	SCP_string cfile_dir(TEST_DATA_PATH);
	cfile_dir += DIR_SEPARATOR_CHAR;
	cfile_dir += "test"; // Cfile expects something after the path

	if ( cfile_init(cfile_dir.c_str()) ) {
		FAIL() << "Cfile init failed!";
	}

	lcl_init(-1);
	lcl_xstr_init();

	if (_initFlags & INIT_GRAPHICS) {
		if (!gr_init(nullptr, GR_STUB, 1024, 768)) {
			FAIL() << "Graphics init failed!";
		}
	}

	if (_initFlags & INIT_SHIPS) {
		ship_init();
	}
}
void test::FSTestFixture::TearDown() {
	if (_initFlags & INIT_SHIPS) {
		ship_close();
	}

	if (_initFlags & INIT_GRAPHICS) {
		io::mouse::CursorManager::shutdown();

		bm_unload_all();

		gr_close();
	}

	cfile_close();

	timer_close();

	lcl_close();

	os_cleanup();

#ifndef NDEBUG
	outwnd_close();
#endif

	// although the comment in cmdline.cpp said this isn't needed,
	// Valgrind disagrees (quite possibly incorrectly), but this is just cleaner
	if (Cmdline_mod != NULL) {
		delete[] Cmdline_mod;
		Cmdline_mod = NULL;
	}
}
void test::FSTestFixture::addCommandlineArg(const SCP_string& arg) {
	_cmdlineArgs.push_back(arg);
}
void test::FSTestFixture::init_cmdline() {
	std::unique_ptr<char*[]> parts(new char*[_cmdlineArgs.size()]);

	for (size_t i = 0; i < _cmdlineArgs.size(); ++i) {
		parts[i] = const_cast<char*>(_cmdlineArgs[i].c_str());
	}

	parse_cmdline((int) _cmdlineArgs.size(), parts.get());
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
