#pragma once

#include <gtest/gtest.h>
#include <globalincs/vmallocator.h>

namespace test {

enum InitFlags {
	INIT_NONE = 0,
	INIT_GRAPHICS = 1 << 0,
	INIT_SHIPS = 1 << 1,
};

class FSTestFixture: public ::testing::Test {
 private:
	uint64_t _initFlags;

	SCP_vector<SCP_string> _cmdlineArgs;
	SCP_string _currentModDir;

	void init_cmdline();
 protected:
	explicit FSTestFixture(uint64_t init_flags);
	virtual ~FSTestFixture() {};

	void addCommandlineArg(const SCP_string& arg);

	void pushModDir(const SCP_string& mod);

	virtual void SetUp();

	virtual void TearDown();
};

}

using namespace test;
