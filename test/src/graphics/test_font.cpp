
#include <gtest/gtest.h>
#include <graphics/font.h>

#include "util/FSTestFixture.h"

class FontTest : public test::FSTestFixture {
 public:
	FontTest() : test::FSTestFixture(INIT_SHIPS | INIT_GRAPHICS | INIT_CFILE) {
		pushModDir("graphics");
		pushModDir("fonts");
	}

 protected:
	virtual void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	virtual void TearDown() override {
		test::FSTestFixture::TearDown();
	}
};

TEST_F(FontTest, default_table) {
	font::init();

	ASSERT_EQ(3, font::FontManager::numberOfFonts());

	font::close();
}

TEST_F(FontTest, additional_font_ttf) {
	font::init();

	ASSERT_EQ(4, font::FontManager::numberOfFonts());

	auto fnt = font::FontManager::getFont("Test Font");
	ASSERT_NE(nullptr, fnt);
	ASSERT_STREQ("Test Font", fnt->getName().c_str());

	font::close();
}
