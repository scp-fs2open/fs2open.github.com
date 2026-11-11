#pragma once

#include <gtest/gtest.h>
#include <globalincs/vmallocator.h>
#include <io/timer.h>

#include <cstdint>

namespace test {

enum InitFlags {
	INIT_NONE = 0,
	INIT_CFILE = 1 << 0,
	INIT_GRAPHICS = 1 << 1,
	INIT_SHIPS = 1 << 2,
	INIT_MOD_TABLE = 1 << 3,
	INIT_FONTS = 1 << 4,
};

class FSTestFixture: public ::testing::Test {
 private:
	std::uint64_t _initFlags;

	SCP_vector<SCP_string> _cmdlineArgs;
	SCP_string _currentModDir;

	void init_cmdline();
 protected:
	explicit FSTestFixture(uint64_t init_flags = INIT_CFILE);
	virtual ~FSTestFixture() {};

	void addCommandlineArg(const SCP_string& arg);

	void pushModDir(const SCP_string& mod);

	struct ClockStep {
		std::uint64_t microseconds = 0;
		TIMER_DIRECTION direction = TIMER_DIRECTION::FORWARD;
	};

	static constexpr std::int64_t GAME_CLOCK_START_MS = 10'000;

	static void primeGameClock();

	static ClockStep startGameClock();

	static void undoGameClock(const ClockStep& step);

	virtual void SetUp();

	virtual void TearDown();
};

}

using namespace test;
