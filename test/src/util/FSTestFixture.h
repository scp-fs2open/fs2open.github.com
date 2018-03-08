#pragma once

#include <gtest/gtest.h>
#include <globalincs/vmallocator.h>

#include <cstdint>

namespace test {

enum InitFlags {
	INIT_NONE = 0,
	INIT_CFILE = 1 << 0,
	INIT_GRAPHICS = 1 << 1,
	INIT_SHIPS = 1 << 2,
	INIT_MOD_TABLE = 1 << 3,
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

	virtual void SetUp();

	virtual void TearDown();
};

}

using namespace test;
