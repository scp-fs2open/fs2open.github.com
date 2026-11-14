#include <gtest/gtest.h>

#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "localization/localize.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "options/OptionsManager.h"

#include "util/FSTestFixture.h"

// Test data (test_data/voice_language):
//   data/tables/zh-lcl.tbm            adds the language "Chinese" with the extension "tw"
//   data/voice/briefing/              foo.ogg, foo-tw.ogg, bar.ogg (no bar-tw.ogg), none-tw.ogg (no none.wav),
//                                     dot_1.5.ogg, dot_1.5-tw.ogg and the decoys dot_1.ogg, dot_1-tw.ogg,
//                                     sub/foo.ogg, sub/foo-tw.ogg, sub.d/foo-tw.ogg (no sub.d/foo.ogg),
//                                     long_name_1234567890abcde-tw.ogg (MAX_FILENAME_LEN characters)
//   data/voice/debriefing/            9_onlytw-tw.ogg (no 9_onlytw.ogg)
//   data/voice/special/               command_onlytw-tw.ogg (no command_onlytw.ogg)
//   ModDefault/data/tables/           game_settings.tbl with "$Voice language: Chinese"; the other tests of the
//                                     mod default set Default_voice_language in the test
class VoiceLanguageTest : public test::FSTestFixture {
  public:
	VoiceLanguageTest() : test::FSTestFixture(INIT_MOD_TABLE | INIT_CFILE) { pushModDir("voice_language"); }

  protected:
	// lcl_close() does not clear Lcl_languages, so the languages of an earlier test (and one more copy of English)
	// would stay in the list. Start from an empty list so that this test does not depend on the tests that ran
	// before it, and put the list back afterwards: lcl_init() adds the built-in languages only to a list with one
	// entry, and a later test (IntelParseTest) relies on German being at index LCL_GERMAN.
	SCP_vector<lang_info> _saved_languages;
	int _saved_current_lang = 0;

	void SetUp() override
	{
		_saved_languages = Lcl_languages;
		_saved_current_lang = Lcl_current_lang;
		Lcl_languages.clear();
		test::FSTestFixture::SetUp();
	}

	void TearDown() override
	{
		// The voice language comes from three process-wide places. Clear all of them, so that nothing leaks
		// into the next test. mod_table_reset() in the base class clears the mod default.
		Cmdline_voice_lang.clear();
		options::OptionsManager::instance()->clearOverrides();
		options::OptionsManager::instance()->discardChanges();
		test::FSTestFixture::TearDown();
		Lcl_languages = _saved_languages;
		Lcl_current_lang = _saved_current_lang;
	}

	static int chinese()
	{
		int lang = lcl_find_lang_index_by_name("Chinese");
		EXPECT_GE(lang, 0) << "zh-lcl.tbm was not read";
		return lang;
	}

	// the text language is English after SetUp(); this makes it Chinese for the rest of the test
	static void set_text_language_chinese() { lcl_set_language(chinese()); }

	static void set_player_voice_language(const char* name, const char* ext)
	{
		options::OptionsManager::instance()->setConfigValue("Audio.VoiceLanguage",
			std::unique_ptr<json_t>(json_pack("{ssss}", "name", name, "ext", ext)));
	}

	// "original" or "same-as-text", the two stored values that are not a language
	static void set_player_voice_choice(const char* value)
	{
		options::OptionsManager::instance()->setConfigValue("Audio.VoiceLanguage",
			std::unique_ptr<json_t>(json_string(value)));
	}

	static SCP_string option_display()
	{
		auto* opt = options::OptionsManager::instance()->getOptionByKey("Audio.VoiceLanguage");
		EXPECT_NE(opt, nullptr);
		return opt ? opt->getCurrentValueDescription().display : "";
	}
};

// The -voice_language flag is read when the command line is parsed, so it must be in place before SetUp().
class VoiceLanguageCmdlineTest : public VoiceLanguageTest {
  public:
	VoiceLanguageCmdlineTest()
	{
		addCommandlineArg("-voice_language");
		addCommandlineArg("Chinese");
	}
};

class VoiceLanguageCmdlineSameAsTextTest : public VoiceLanguageTest {
  public:
	VoiceLanguageCmdlineSameAsTextTest()
	{
		addCommandlineArg("-voice_language");
		addCommandlineArg("same-as-text");
	}
};

TEST_F(VoiceLanguageTest, NothingSetUsesTheOriginalVoice)
{
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
	EXPECT_EQ(lcl_get_voice_filename("foo", CF_TYPE_ANY), "foo");
}

TEST_F(VoiceLanguageTest, NothingSetKeepsTheOriginalVoiceForChineseText)
{
	// the text language alone does not switch the voice: foo-tw.ogg exists and must not be picked
	set_text_language_chinese();

	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
}

TEST_F(VoiceLanguageTest, OptionUnsetShowsTheOriginalVoice)
{
	auto* opt = options::OptionsManager::instance()->getOptionByKey("Audio.VoiceLanguage");
	ASSERT_NE(opt, nullptr);
	EXPECT_EQ(opt->getCurrentValueDescription().display, "Original voice");

	// the two values that are not a language, then every language
	auto values = opt->getValidValues();
	ASSERT_EQ(values.size(), Lcl_languages.size() + 2);
	EXPECT_EQ(values[0].display, "Original voice");
	EXPECT_EQ(values[1].display, "Same as text language");
	EXPECT_EQ(values.back().display, "Chinese");
}

TEST_F(VoiceLanguageTest, PlayerSettingPicksTheSuffixedFile)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("foo", CF_TYPE_ANY), "foo-tw.ogg");
	// the extension follows the file that exists, as the audio loader does
	EXPECT_EQ(lcl_get_voice_filename("foo.wav", CF_TYPE_ANY), "foo-tw.ogg");
}

TEST_F(VoiceLanguageTest, PlayerSameAsTextFollowsTheTextLanguage)
{
	Using_in_game_options = true;
	set_player_voice_choice("same-as-text");

	EXPECT_EQ(option_display(), "Same as text language");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");

	set_text_language_chinese();
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");
}

TEST_F(VoiceLanguageTest, PlayerOriginalVoiceIgnoresTheTextLanguage)
{
	Using_in_game_options = true;
	set_player_voice_choice("original");
	set_text_language_chinese();

	EXPECT_EQ(option_display(), "Original voice");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
}

TEST_F(VoiceLanguageTest, MissingSuffixedFileFallsBack)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	EXPECT_EQ(lcl_get_voice_filename("bar.ogg", CF_TYPE_ANY), "bar.ogg");
}

TEST_F(VoiceLanguageTest, DottedNameKeepsAShortTail)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	// cfile reads a dot as the start of an extension only when at least two characters follow it, so that
	// retail names like "x_1.5" keep their ".5". dot_1.ogg and dot_1-tw.ogg are decoys for a wrong strip.
	EXPECT_EQ(lcl_get_voice_filename("dot_1.5", CF_TYPE_ANY), "dot_1.5-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("dot_1.5.ogg", CF_TYPE_ANY), "dot_1.5-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("dot_1.5.wav", CF_TYPE_ANY), "dot_1.5-tw.ogg");
}

TEST_F(VoiceLanguageTest, DirectoryPartIsKept)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	// the caller's directory stays in the name; foo-tw.ogg in the parent folder must not be picked instead
	EXPECT_EQ(lcl_get_voice_filename("sub/foo.ogg", CF_TYPE_ANY), "sub/foo-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("sub/foo", CF_TYPE_ANY), "sub/foo-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("sub/bar.ogg", CF_TYPE_ANY), "sub/bar.ogg");
}

TEST_F(VoiceLanguageTest, SentinelNamesAreNotSpecial)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	// The lookup knows no sentinel: a mod that ships none-tw.ogg gets it. A caller that treats "none" as
	// "no voice" (message_play_training_voice) has to check for it before it asks for the voice file.
	EXPECT_EQ(lcl_get_voice_filename("none.wav", CF_TYPE_ANY), "none-tw.ogg");
}

TEST_F(VoiceLanguageTest, EmptyLanguageListLeavesTheNameAlone)
{
	// before lcl_init(), or after a test that cleared the list; also with "same as text", whose index is the text
	// language's and so not negative, so that a bounds check for a negative index alone is not enough
	Using_in_game_options = true;
	set_player_voice_choice("same-as-text");

	auto languages = Lcl_languages;
	Lcl_languages.clear();
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
	Lcl_languages = languages;
}

TEST_F(VoiceLanguageTest, PlayerSettingIgnoredWithoutInGameOptions)
{
	Using_in_game_options = false;
	set_player_voice_language("Chinese", "tw");

	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
}

TEST_F(VoiceLanguageTest, ModDefault)
{
	ASSERT_EQ(Default_voice_language, "Chinese");
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");

	// also without in-game options
	Using_in_game_options = false;
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
}

TEST_F(VoiceLanguageTest, ModDefaultSameAsText)
{
	Default_voice_language = "same-as-text";
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");

	set_text_language_chinese();
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");

	Using_in_game_options = false;
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
}

TEST_F(VoiceLanguageTest, PlayerChoiceBeatsTheModDefault)
{
	Default_voice_language = "Chinese";
	Using_in_game_options = true;

	// while the player has not chosen, the menu shows the mod default, which is what plays
	EXPECT_EQ(option_display(), "Chinese");
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());

	// each explicit choice wins over the mod default, also "same as text" (English here) and "original"
	set_player_voice_language("English", "");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");

	set_player_voice_choice("same-as-text");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");

	set_player_voice_choice("original");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo.ogg");
}

TEST_F(VoiceLanguageTest, VoiceFileExistsInTheVoiceLanguage)
{
	// as named: only the files without a suffix count, and only in the given folder
	EXPECT_TRUE(lcl_voice_file_exists("foo", CF_TYPE_VOICE_BRIEFINGS));
	EXPECT_FALSE(lcl_voice_file_exists("foo", CF_TYPE_VOICE_DEBRIEFINGS));
	EXPECT_FALSE(lcl_voice_file_exists("9_onlytw", CF_TYPE_VOICE_DEBRIEFINGS));
	EXPECT_FALSE(lcl_voice_file_exists("command_onlytw", CF_TYPE_VOICE_SPECIAL));

	// in Chinese, a name whose only file is the -tw one exists
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");
	EXPECT_TRUE(lcl_voice_file_exists("9_onlytw", CF_TYPE_VOICE_DEBRIEFINGS));
	EXPECT_TRUE(lcl_voice_file_exists("command_onlytw", CF_TYPE_VOICE_SPECIAL));
	EXPECT_TRUE(lcl_voice_file_exists("foo", CF_TYPE_VOICE_BRIEFINGS));
	EXPECT_FALSE(lcl_voice_file_exists("9_missing", CF_TYPE_VOICE_DEBRIEFINGS));
	EXPECT_FALSE(lcl_voice_file_exists("9_onlytw", CF_TYPE_VOICE_BRIEFINGS));

	// the folder also limits the lookup itself
	EXPECT_EQ(lcl_get_voice_filename("9_onlytw", CF_TYPE_VOICE_DEBRIEFINGS), "9_onlytw-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("9_onlytw", CF_TYPE_VOICE_BRIEFINGS), "9_onlytw");
}

TEST_F(VoiceLanguageCmdlineTest, CommandLine)
{
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");

	// the options menu shows the flag as an override of the player setting
	EXPECT_TRUE(options::OptionsManager::instance()->isOverridden("Audio.VoiceLanguage"));
	EXPECT_EQ(option_display(), "Chinese");
}

TEST_F(VoiceLanguageCmdlineTest, CommandLineWithoutInGameOptions)
{
	// With in-game options on, the override registered by lcl_init() makes the option itself return the flag's
	// language, and it also rejects a player value, so a test there cannot tell the flag's own branch apart.
	// Without in-game options the option is not read at all, so only that branch can make the flag count.
	Using_in_game_options = false;

	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");
}

TEST_F(VoiceLanguageCmdlineSameAsTextTest, CommandLineSameAsText)
{
	EXPECT_TRUE(options::OptionsManager::instance()->isOverridden("Audio.VoiceLanguage"));
	EXPECT_EQ(option_display(), "Same as text language");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);

	set_text_language_chinese();
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(lcl_get_voice_filename("foo.ogg", CF_TYPE_ANY), "foo-tw.ogg");

	Using_in_game_options = false;
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
}

TEST_F(VoiceLanguageTest, DottedDirectoryKeepsItsName)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	// the extension dot is looked for after the last directory separator, as cfile does
	EXPECT_EQ(lcl_get_voice_filename("sub.d/foo", CF_TYPE_ANY), "sub.d/foo-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("sub.d/foo.ogg", CF_TYPE_ANY), "sub.d/foo-tw.ogg");
	EXPECT_EQ(lcl_get_voice_filename("sub.d/bar", CF_TYPE_ANY), "sub.d/bar");
}

TEST_F(VoiceLanguageTest, StaleStoredLanguageFallsBackToTheModDefault)
{
	// the mod default is English here, so that a stored "Chinese" with a wrong extension cannot pass by its name
	Default_voice_language = "English";
	Using_in_game_options = true;

	// a language this mod does not have, and a known name with another extension: both count as "not chosen"
	set_player_voice_language("Klingon", "kl");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(option_display(), "English");

	set_player_voice_language("Chinese", "xx");
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_ENGLISH);
	EXPECT_EQ(option_display(), "English");
}

TEST_F(VoiceLanguageTest, BogusStoredStringFallsBackToTheModDefault)
{
	Default_voice_language = "Chinese";
	Using_in_game_options = true;

	// only "original" and "same-as-text" are values; a hand-edited string counts as "not chosen"
	set_player_voice_choice("bogus");
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
	EXPECT_EQ(option_display(), "Chinese");
}

TEST_F(VoiceLanguageCmdlineTest, FlagBeatsModDefault)
{
	Default_voice_language = "English";

	// without in-game options the option's override is not read, so only the order of the branches decides
	Using_in_game_options = false;
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());

	Using_in_game_options = true;
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
}

TEST_F(VoiceLanguageTest, UnknownFlagValueIsIgnored)
{
	// set after SetUp(): for an unknown name lcl_init() gives a Warning, which throws under unittests
	Cmdline_voice_lang = "Klingon";
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);

	// it falls through to the next source, not to "original"
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");
	EXPECT_EQ(lcl_get_voice_lang_index(), chinese());
}

TEST_F(VoiceLanguageTest, ModDefaultUnknown)
{
	Default_voice_language = "Klingon";
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
	EXPECT_EQ(option_display(), "Original voice");

	Using_in_game_options = false;
	EXPECT_EQ(lcl_get_voice_lang_index(), LCL_VOICE_ORIGINAL);
}

TEST_F(VoiceLanguageTest, LongNameIsLeftAlone)
{
	Using_in_game_options = true;
	set_player_voice_language("Chinese", "tw");

	// "long_name_1234567890abcde-tw.ogg" has MAX_FILENAME_LEN characters, one more than the sound code's buffers hold
	EXPECT_EQ(lcl_get_voice_filename("long_name_1234567890abcde", CF_TYPE_ANY), "long_name_1234567890abcde");
	EXPECT_EQ(lcl_get_voice_filename("long_name_1234567890abcde.ogg", CF_TYPE_ANY), "long_name_1234567890abcde.ogg");
}
