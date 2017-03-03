#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "playerman/player.h"
#include "pilotfile/pilotfile.h"

class PilotPlayerFileTest : public test::FSTestFixture {
 public:
	PilotPlayerFileTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_FONTS) {
		pushModDir("pilotfile");
	}

 protected:
	void TearDown() override {
		// Delete the converted files
		cf_delete("asdf.json", CF_TYPE_PLAYERS);

		FSTestFixture::TearDown();
	}
};

TEST_F(PilotPlayerFileTest, binaryToJSONConversion) {
	// Call the conversion function
	convert_pilot_files();

	// Now check if the pilot files before and after are the same
	pilotfile loader;

	// First read the binary file
	player binary_plr;
	binary_plr.reset();
	ASSERT_TRUE(loader.load_player("asdf", &binary_plr, true));

	// Then read the json file
	player json_plr;
	json_plr.reset();
	ASSERT_TRUE(loader.load_player("asdf", &json_plr, false));

	ASSERT_EQ(binary_plr, json_plr);
}
