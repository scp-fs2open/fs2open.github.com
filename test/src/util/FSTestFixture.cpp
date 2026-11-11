//
//

#include "FSTestFixture.h"

#include <cmdline/cmdline.h>
#include <graphics/2d.h>
#include <io/timer.h>
#include <localization/localize.h>
#include <mod_table/mod_table.h>
#include <osapi/outwnd.h>
#include <ship/ship.h>

#include <chrono>
#include <thread>

namespace {
constexpr int OFFSET_NUDGE_MS = 3'600'000;
}

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
		cfile_dir += DIR_SEPARATOR_CHAR;
		cfile_dir += "test"; // Cfile expects something after the path

		if (cfile_init(cfile_dir.c_str())) {
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
			if (!gr_init(nullptr, GraphicsAPI::Stub, 1024, 768)) {
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

	// although the comment in cmdline.cpp said this isn't needed,
	// Valgrind disagrees (quite possibly incorrectly), but this is just cleaner
	if (Cmdline_mod != NULL) {
		delete[] Cmdline_mod;
		Cmdline_mod = NULL;
	}
}
void test::FSTestFixture::primeGameClock() {
	// timer_init pauses on the counter's first tick, which can read 0, and adjusting the clock asserts on that 0
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	timestamp_pause(true);
	timestamp_unpause(true);
	// the unpause can land the clock offset on exactly 0, which asserts too; a pretend pause and equal skip clear it
	timestamp_adjust_pause_offset(OFFSET_NUDGE_MS);
	timestamp_adjust_microseconds(static_cast<std::uint64_t>(OFFSET_NUDGE_MS) * MICROSECONDS_PER_MILLISECOND,
		TIMER_DIRECTION::FORWARD);
}
test::FSTestFixture::ClockStep test::FSTestFixture::startGameClock() {
	primeGameClock();
	timer_start_frame();
	const std::int64_t now_ms = _timestamp().value();

	ClockStep step;
	if (now_ms < GAME_CLOCK_START_MS) {
		step.microseconds = static_cast<std::uint64_t>(GAME_CLOCK_START_MS - now_ms) * MICROSECONDS_PER_MILLISECOND;
	} else {
		step.microseconds = static_cast<std::uint64_t>(now_ms - GAME_CLOCK_START_MS) * MICROSECONDS_PER_MILLISECOND;
		step.direction = TIMER_DIRECTION::BACKWARD;
	}
	timestamp_adjust_microseconds(step.microseconds, step.direction);
	timer_start_frame();
	return step;
}
void test::FSTestFixture::undoGameClock(const ClockStep& step) {
	const bool forward = step.direction == TIMER_DIRECTION::FORWARD;
	timestamp_adjust_microseconds(step.microseconds, forward ? TIMER_DIRECTION::BACKWARD : TIMER_DIRECTION::FORWARD);
}
void test::FSTestFixture::addCommandlineArg(const SCP_string& arg) {
	_cmdlineArgs.push_back(arg);
}
void test::FSTestFixture::init_cmdline() {
	std::vector<char*> parts(_cmdlineArgs.size());

	for (size_t i = 0; i < _cmdlineArgs.size(); ++i) {
		parts[i] = _cmdlineArgs[i].data();
	}

	parse_cmdline(sz2i(_cmdlineArgs.size()), parts.data());
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
