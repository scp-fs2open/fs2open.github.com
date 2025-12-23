
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
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
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

TEST_F(FontTest, force_fit)
{
	font::init();

	{
		char str[] = "";
		int w = font::force_fit_string(str, std::string::npos, 80);
		ASSERT_EQ(w, 0);
		ASSERT_EQ(strlen(str), 0);
	}

	{
		char str[] = "abcdefghijklmnopqrstuvwxyz";
		int w = font::force_fit_string(str, std::string::npos, 0);
		ASSERT_EQ(w, 0);
		ASSERT_EQ(strlen(str), 0);
	}

	{
		char str[] = "abcdefghijklmnopqrstuvwxyz";
		int w = font::force_fit_string(str, std::string::npos, 80);
		ASSERT_LE(w, 80);
		ASSERT_EQ(strlen(str), 12);
	}

	{
		char str[] = "abcdefghijklmnopqrstuvwxyz";
		int w = font::force_fit_string(str, 5, 300);
		ASSERT_LE(w, 300);
		ASSERT_EQ(strlen(str), 5);
	}

	{
		char str[] = "abcdefghijklmnopqrstuvwxyz";
		int w = font::force_fit_string(str, std::string::npos, 300);
		ASSERT_LE(w, 300);
		ASSERT_EQ(strlen(str), 26);
	}

	font::close();
}
