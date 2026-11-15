#include "options/OptionsManager.h"
#include "options/Option.h"
#include "graphics/2d.h"
#include "graphics/grstub.h"
#include "graphics/shadows.h"
#include "cmdline/cmdline.h"
#include "hud/hud.h"
#include "localization/localize.h"
#include "mod_table/mod_table.h"
#include "parse/parselo.h"
#include "math/floating.h"
#include "osapi/osregistry.h"

#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

using namespace options;

// OptionsManager is a process-wide singleton, so overrides and pending edits set by one test would
// otherwise stay visible to every test that runs after it in the same binary. Clear both after each test.
class OptionsManagerOverride : public ::testing::Test {
  protected:
	void TearDown() override
	{
		OptionsManager::instance()->clearOverrides();
		OptionsManager::instance()->discardChanges();
		OptionsManager::instance()->clearRequirements();
	}
};

// Probe: real Option<T> instances are defined via static initializers scattered across many translation
// units in libcode.a. Confirm the linker actually pulled at least one of them into the unittests binary
// (rather than static init being stripped as unreferenced) before trusting any test that exercises a real
// option's deserializer.
TEST_F(OptionsManagerOverride, RealOptionsAreRegistered)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr) << "Graphics.VSync was not registered -- static Option<T> initializers are not "
	                           "linked into this test binary, so override round-trip tests against real "
	                           "options are not possible here.";
}

// Regression coverage for the cmdline-priority-rework: OptionsManager::getValueFromConfig() must resolve a
// command-line override ahead of both a persisted ini value and any in-session edit, and setConfigValue()
// must refuse to record an edit for an overridden key at all. The options UI relies on this: it disables
// the control for an overridden option, so an accepted edit could only ever be a value the game ignores.

TEST_F(OptionsManagerOverride, OverrideWinsOverSessionEdit)
{
	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Test.OverridePriority.Edited", "42", "-test_flag");
	mgr->setConfigValue("Test.OverridePriority.Edited", std::unique_ptr<json_t>(json_integer(7)));

	auto result = mgr->getValueFromConfig("Test.OverridePriority.Edited");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 42);
}

TEST_F(OptionsManagerOverride, OverrideDropsAnAlreadyPendingEdit)
{
	auto* mgr = OptionsManager::instance();

	// An edit recorded before the key was overridden must not survive the override either, so that
	// persistChanges() cannot later write a value the game will never use.
	mgr->setConfigValue("Test.OverridePriority.PreEdited", std::unique_ptr<json_t>(json_integer(7)));
	mgr->setOverride("Test.OverridePriority.PreEdited", "42", "-test_flag");

	auto result = mgr->getValueFromConfig("Test.OverridePriority.PreEdited");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 42);
}

TEST_F(OptionsManagerOverride, OverrideReasonIsQueryable)
{
	auto* mgr = OptionsManager::instance();

	ASSERT_FALSE(mgr->isOverridden("Test.OverridePriority.NeverSet"));
	ASSERT_FALSE(mgr->getOverrideReason("Test.OverridePriority.NeverSet").has_value());

	mgr->setOverride("Test.OverridePriority.Reason", "1", "-my_flag");

	ASSERT_TRUE(mgr->isOverridden("Test.OverridePriority.Reason"));
	auto reason = mgr->getOverrideReason("Test.OverridePriority.Reason");
	ASSERT_TRUE(reason.has_value());
	ASSERT_EQ(*reason, "-my_flag");
}

TEST_F(OptionsManagerOverride, LaterOverrideReplacesEarlierOne)
{
	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Test.OverridePriority.Replaced", "1", "-first_flag");
	mgr->setOverride("Test.OverridePriority.Replaced", "2", "-second_flag");

	auto result = mgr->getValueFromConfig("Test.OverridePriority.Replaced");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 2);

	auto reason = mgr->getOverrideReason("Test.OverridePriority.Replaced");
	ASSERT_TRUE(reason.has_value());
	ASSERT_EQ(*reason, "-second_flag");
}

// A script can persist one option while another still has an edit pending, so a listener that reads another
// option has to see only what was persisted.
TEST_F(OptionsManagerOverride, CommittedValueIgnoresPendingEdit)
{
	auto* mgr = OptionsManager::instance();

	mgr->setConfigValue("Test.Committed.Pending", std::unique_ptr<json_t>(json_integer(7)));

	auto pending = mgr->getValueFromConfig("Test.Committed.Pending");
	ASSERT_TRUE(pending.has_value());
	ASSERT_EQ(json_integer_value(pending->get()), 7);

	ASSERT_FALSE(mgr->getValueFromConfig("Test.Committed.Pending", PendingEdits::Ignore).has_value());
}

TEST_F(OptionsManagerOverride, CommittedValueStillHonorsOverride)
{
	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Test.Committed.Override", "42", "-test_flag");

	auto result = mgr->getValueFromConfig("Test.Committed.Override", PendingEdits::Ignore);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 42);
}

TEST_F(OptionsManagerOverride, WindowModeCommittedValueIgnoresPendingEdit)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.WindowMode");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<os::ViewportState>*>(opt);

	class WindowStateGuard {
	  public:
		WindowStateGuard() : _original(Gr_configured_window_state) {}
		~WindowStateGuard() { Gr_configured_window_state = _original; }

	  private:
		os::ViewportState _original;
	};
	WindowStateGuard guard;

	// No override and no persisted ini value, so the committed value is the default
	Gr_configured_window_state = os::ViewportState::Fullscreen;
	OptionsManager::instance()->setConfigValue("Graphics.WindowMode",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(os::ViewportState::Windowed))));

	ASSERT_EQ(typedOpt->getValue(), os::ViewportState::Windowed);
	ASSERT_EQ(typedOpt->getCommittedValue(), os::ViewportState::Fullscreen);
}

// The display and window mode listeners read each other's committed value, which needs a real settings profile to
// write to and read back from. FSTestFixture runs in portable mode, so the file is test_data/data/options_settings.ini.
class OptionsManagerPersist : public test::FSTestFixture {
  public:
	OptionsManagerPersist() : test::FSTestFixture(INIT_CFILE) { pushModDir("options"); }

  protected:
	void SetUp() override
	{
		_save_found = Ingame_options_save_found;
		test::FSTestFixture::SetUp();

		// On a fresh checkout the first init found no settings file and loaded no profile; it created the file, so
		// load again from it
		Ingame_options_save_found = true;
		os_deinit_registry_stuff();
		os_init_registry_stuff(nullptr, "Test");

		_first = makeOption("Test.Persist.First", &_second_seen_by_first, "Test.Persist.Second");
		_second = makeOption("Test.Persist.Second", &_first_seen_by_second, "Test.Persist.First");

		// Start from known committed values; the file keeps whatever an earlier run wrote
		OptionsManager::instance()->discardChanges();
		setBoth(1);
		ASSERT_TRUE(OptionsManager::instance()->persistChanges().empty());
		ASSERT_EQ(_first->getCommittedValue(), 1);
		ASSERT_EQ(_second->getCommittedValue(), 1);
		_first_seen_by_second = 0;
		_second_seen_by_first = 0;
	}
	void TearDown() override
	{
		OptionsManager::instance()->discardChanges();
		OptionsManager::instance()->removeOption(_first);
		OptionsManager::instance()->removeOption(_second);
		Ingame_options_save_found = _save_found;

		test::FSTestFixture::TearDown();
	}

	static std::shared_ptr<const Option<int>> makeOption(const char* key, int* seen, const char* other_key)
	{
		return OptionBuilder<int>(key, SCP_string(key), SCP_string(key))
			.default_val(0)
			.change_listener([seen, other_key](const int&, bool) {
				auto* other = static_cast<const Option<int>*>(OptionsManager::instance()->getOptionByKey(other_key));
				*seen = other->getCommittedValue();
				return true;
			})
			.finish();
	}
	static void setBoth(int value)
	{
		OptionsManager::instance()->setConfigValue("Test.Persist.First", std::unique_ptr<json_t>(json_integer(value)));
		OptionsManager::instance()->setConfigValue("Test.Persist.Second", std::unique_ptr<json_t>(json_integer(value)));
	}

	std::shared_ptr<const Option<int>> _first;
	std::shared_ptr<const Option<int>> _second;
	int _first_seen_by_second = 0;
	int _second_seen_by_first = 0;
	bool _save_found = true;
};

// One Apply of several options: persistChanges() writes every value before it notifies any listener, so either
// listener, whichever runs first, sees the other option's new value
TEST_F(OptionsManagerPersist, PersistChangesShowsEveryNewValueToEveryListener)
{
	setBoth(2);
	ASSERT_TRUE(OptionsManager::instance()->persistChanges().empty());

	ASSERT_EQ(_first_seen_by_second, 2);
	ASSERT_EQ(_second_seen_by_first, 2);
}

// A script can persist one option alone; another option's edit stays pending and its committed value stays old
TEST_F(OptionsManagerPersist, PersistOneOptionKeepsTheOtherCommittedValueOld)
{
	setBoth(2);
	ASSERT_TRUE(OptionsManager::instance()->persistOptionChanges(_second.get()));

	ASSERT_EQ(_first_seen_by_second, 1);
	ASSERT_EQ(_first->getValue(), 2);
	ASSERT_EQ(_first->getCommittedValue(), 1);
}

// The tests above exercise getValueFromConfig() directly with synthetic keys and integer payloads. The
// tests below instead go through real, statically-registered Option<T> instances (confirmed live by the
// probe test above), each with the same JSON value cmdline.cpp builds for that option, so a malformed value
// shows up as a deserialization failure here instead of silently falling back to that option's default_func
// and masking the bug.
//
// These call the typed Option<T>::getValue() (via a static_cast down to the option's real, statically-known
// type) rather than getCurrentValueDescription(), because several options' display functions call XSTR() for
// localized display strings (e.g. "On"/"Off", the shadow quality tiers). XSTR() hits an Int3() unless
// lcl_xstr_init() ran first, and the OptionsManagerOverride fixture never runs it. Only FSTestFixture does,
// and Xstr_inited stays set for the rest of the process afterwards, so under --gtest_shuffle a display call
// from this fixture crashes or passes purely by test order. getValue() only exercises the deserializer, which
// is exactly the contract these tests are checking anyway. The one test that must go through a display
// function uses the OptionsManagerOverrideLocalized fixture below instead.

TEST_F(OptionsManagerOverride, BoolOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<bool>*>(opt);

	auto* mgr = OptionsManager::instance();

	// Exactly the values built in cmdline.cpp's -no_vsync handling.
	mgr->setOverride("Graphics.VSync", std::unique_ptr<json_t>(json_false()), "-no_vsync");
	ASSERT_FALSE(typedOpt->getValue());

	mgr->setOverride("Graphics.VSync", std::unique_ptr<json_t>(json_true()), "-vsync");
	ASSERT_TRUE(typedOpt->getValue());
}

TEST_F(OptionsManagerOverride, ResolutionObjectOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Resolution");
	ASSERT_NE(opt, nullptr);

	// ResolutionInfo is a file-local type in 2d.cpp, not reachable from here, so this test goes through
	// getCurrentValueDescription() instead -- safe because resolution_display() is a plain sprintf with no
	// XSTR() call.
	// Exactly the value built in cmdline.cpp's -render_res handling.
	OptionsManager::instance()->setOverride("Graphics.Resolution",
		std::unique_ptr<json_t>(json_pack("{s:i, s:i}", "width", 1920, "height", 1080)), "-render_res");

	ASSERT_EQ(opt->getCurrentValueDescription().display, "1920x1080");
}

TEST_F(OptionsManagerOverride, EnumIntOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Shadows");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<ShadowQuality>*>(opt);

	// Exactly the value built in cmdline.cpp's -shadow_quality handling.
	OptionsManager::instance()->setOverride("Graphics.Shadows",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(ShadowQuality::Medium))), "-shadow_quality");

	ASSERT_EQ(typedOpt->getValue(), ShadowQuality::Medium);
}

// Regression coverage for the -enable_shadows path: Graphics.Shadows' default_func must read the
// Shadow_quality global rather than return a constant. -enable_shadows and game_settings.tbl's
// "$Shadow Quality Default:" both write that global before the options manager loads its initial values,
// so a constant default_func would discard whatever they set.
TEST_F(OptionsManagerOverride, ShadowQualityDefaultFollowsTheGlobal)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Shadows");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<ShadowQuality>*>(opt);

	// Shadow_quality is a shared global, so restore it from a destructor: a failing assertion below must
	// not leave a later test looking at this test's value.
	class ShadowQualityGuard {
	  public:
		ShadowQualityGuard() : _original(Shadow_quality) {}
		~ShadowQualityGuard() { Shadow_quality = _original; }

	  private:
		ShadowQuality _original;
	};
	ShadowQualityGuard guard;

	// No override and no persisted ini value, so getValue() has to fall back to default_func().
	Shadow_quality = ShadowQuality::Medium; // what -enable_shadows sets
	ASSERT_EQ(typedOpt->getValue(), ShadowQuality::Medium);

	Shadow_quality = ShadowQuality::Ultra; // what a mod table can set afterwards
	ASSERT_EQ(typedOpt->getValue(), ShadowQuality::Ultra);

	// An override still beats the default, which is what -shadow_quality relies on.
	OptionsManager::instance()->setOverride("Graphics.Shadows",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(ShadowQuality::Low))), "-shadow_quality");
	ASSERT_EQ(typedOpt->getValue(), ShadowQuality::Low);
}

// Same defect, same fix, one option over: Graphics.ShadowRenderMethod has a parser too, so a constant
// default_func would discard the mod table's "$Shadow Render Method:" value. bind_to() writes the option
// value back to the global on the initial load, which is what made the loss silent.
TEST_F(OptionsManagerOverride, ShadowRenderMethodDefaultFollowsTheGlobal)
{
	// gr_init() keeps this option also when the renderer cannot raytrace: shadows_mark_unsupported_options() only
	// records its requirement. So the Stub renderer of FSTestFixture leaves it in place.
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.ShadowRenderMethod");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<ShadowRenderMethod>*>(opt);

	class ShadowRenderMethodGuard {
	  public:
		ShadowRenderMethodGuard() : _original(Shadow_render_method) {}
		~ShadowRenderMethodGuard() { Shadow_render_method = _original; }

	  private:
		ShadowRenderMethod _original;
	};
	ShadowRenderMethodGuard guard;

	// No override and no persisted ini value, so getValue() has to fall back to default_func().
	Shadow_render_method = ShadowRenderMethod::Raytraced; // what a mod table can set
	ASSERT_EQ(typedOpt->getValue(), ShadowRenderMethod::Raytraced);

	Shadow_render_method = ShadowRenderMethod::ShadowMap;
	ASSERT_EQ(typedOpt->getValue(), ShadowRenderMethod::ShadowMap);

	// An override still beats the default, which is what -rt_shadows relies on.
	OptionsManager::instance()->setOverride("Graphics.ShadowRenderMethod",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(ShadowRenderMethod::Raytraced))), "-rt_shadows");
	ASSERT_EQ(typedOpt->getValue(), ShadowRenderMethod::Raytraced);
}

// Graphics.MSAASamples uses a MapValueDisplay, so the check below cannot avoid XSTR(): Option<T> builds the
// display string for every value inside getValidValues(), and toDescription() catches MapValueDisplay's throw
// for an unmapped value and falls back to the serialized JSON, so an XSTR-free reformulation could no longer
// tell a mapped value from an unmapped one. This fixture therefore runs the full FSTestFixture setup, which
// calls lcl_init() and lcl_xstr_init(), instead of relying on some earlier test to have done it.
class OptionsManagerOverrideLocalized : public test::FSTestFixture {
  public:
	OptionsManagerOverrideLocalized() : test::FSTestFixture(INIT_CFILE) { pushModDir("options"); }

  protected:
	void TearDown() override
	{
		OptionsManager::instance()->clearOverrides();
		OptionsManager::instance()->discardChanges();

		test::FSTestFixture::TearDown();
	}
};

TEST_F(OptionsManagerOverrideLocalized, MsaaOverrideAcceptsEverySampleCountTheFlagAllows)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.MSAASamples");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<int>*>(opt);

	// -msaa accepts 0, 4, 8 and 16, so every one of them must also be a valid value of the option.
	// Otherwise MapValueDisplay throws for the sample count and printValues() reports the override as
	// invalid at startup.
	auto values = opt->getValidValues();
	for (int samples : {0, 4, 8, 16}) {
		OptionsManager::instance()->setOverride("Graphics.MSAASamples",
			std::unique_ptr<json_t>(json_integer(samples)), "-msaa");

		ASSERT_EQ(typedOpt->getValue(), samples);

		SCP_string serialized = std::to_string(samples);
		ASSERT_NE(std::find_if(values.begin(), values.end(),
					  [&serialized](const ValueDescription& val) { return val.serialized == serialized; }),
			values.end())
			<< "MSAA sample count " << samples << " is accepted by -msaa but is not a valid option value.";
	}
}

// Graphics.RenderAPI only exists in builds compiled with Vulkan support (code/graphics/2d.cpp gates the whole
// option definition on #ifdef WITH_VULKAN) -- in an OpenGL-only build there's nothing to choose between, so
// gr_get_configured_render_api() just returns GraphicsAPI::Default and no option is registered at all.
#ifdef WITH_VULKAN

TEST_F(OptionsManagerOverride, RenderAPIOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.RenderAPI");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<GraphicsAPI>*>(opt);

	// Exactly the value built in cmdline.cpp's -vulkan handling.
	OptionsManager::instance()->setOverride("Graphics.RenderAPI",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(GraphicsAPI::Vulkan))), "-vulkan");

	ASSERT_EQ(typedOpt->getValue(), GraphicsAPI::Vulkan);
	ASSERT_EQ(gr_get_configured_render_api(), GraphicsAPI::Vulkan);
}

TEST_F(OptionsManagerOverride, RenderAPIMenuInitPathDoesNotThrow)
{
	// Unlike VSync/Shadows/etc, RenderAPI's display function is plain text (no XSTR()), so -- unlike those --
	// it's safe to exercise the exact calls ingame_options_init() makes when building the options menu
	// (getValidValues() for the dropdown entries, getCurrentValueDescription() for the current selection),
	// giving at least some coverage of the menu-construction path itself, not just the deserializer.
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.RenderAPI");
	ASSERT_NE(opt, nullptr);
	ASSERT_EQ(opt->getType(), OptionType::Selection);
	ASSERT_EQ(opt->getValidValues().size(), 2u);

	OptionsManager::instance()->setOverride("Graphics.RenderAPI",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(GraphicsAPI::OpenGL))), "-opengl");
	ASSERT_EQ(opt->getCurrentValueDescription().display, "OpenGL");

	OptionsManager::instance()->setOverride("Graphics.RenderAPI",
		std::unique_ptr<json_t>(json_integer(static_cast<int>(GraphicsAPI::Vulkan))), "-vulkan");
	ASSERT_EQ(opt->getCurrentValueDescription().display, "Vulkan");
}

#else

TEST_F(OptionsManagerOverride, RenderAPIIsAbsentWithoutVulkanSupport)
{
	ASSERT_EQ(OptionsManager::instance()->getOptionByKey("Graphics.RenderAPI"), nullptr);
	ASSERT_EQ(gr_get_configured_render_api(), GraphicsAPI::Default);
}

#endif

TEST_F(OptionsManagerOverride, FlagsetOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.FramebufferEffects");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<flagset<FramebufferEffects>>*>(opt);

	// Exactly the value built in cmdline.cpp's -fb_explosions/-fb_thrusters handling (flagset::to_u64()).
	flagset<FramebufferEffects> effects;
	effects.set(FramebufferEffects::Thrusters, true);
	effects.set(FramebufferEffects::Shockwaves, true);

	OptionsManager::instance()->setOverride("Graphics.FramebufferEffects",
		std::unique_ptr<json_t>(json_integer(static_cast<json_int_t>(effects.to_u64()))),
		"-fb_explosions -fb_thrusters");

	auto value = typedOpt->getValue();
	ASSERT_TRUE(value[FramebufferEffects::Thrusters]);
	ASSERT_TRUE(value[FramebufferEffects::Shockwaves]);
}

TEST_F(OptionsManagerOverride, FloatOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Anisotropy");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<float>*>(opt);

	// Exactly the value built in cmdline.cpp's -anisotropic_filter handling. json_real matters here: the
	// option's deserializer wants a JSON real, and a whole number written as a bare integer would not be one.
	OptionsManager::instance()->setOverride("Graphics.Anisotropy",
		std::unique_ptr<json_t>(json_real(16.0)), "-anisotropic_filter");

	ASSERT_FLOAT_EQ(typedOpt->getValue(), 16.0f);
}

// these globals belong to the whole process and CI shuffles test order, so put them back after each test
class AnisotropySetting : public OptionsManagerOverride {
  protected:
	void SetUp() override
	{
		_saved_property = gr_screen.gf_get_property;
		_saved_ingame = Using_in_game_options;
		_saved_level = Cmdline_aniso_level;
		gr_screen.gf_get_property = [](gr_property p, void* dest) {
			if (p != gr_property::MAX_ANISOTROPY) {
				return false;
			}
			*static_cast<float*>(dest) = 16.0f;
			return true;
		};
	}
	void TearDown() override
	{
		gr_screen.gf_get_property = _saved_property;
		Using_in_game_options = _saved_ingame;
		Cmdline_aniso_level = _saved_level;
		OptionsManagerOverride::TearDown();
	}

  private:
	decltype(gr_screen.gf_get_property) _saved_property;
	bool _saved_ingame = true;
	int _saved_level = 0;
};

TEST_F(AnisotropySetting, InGameOptionsUseTheOption)
{
	Using_in_game_options = true;
	Cmdline_aniso_level = 0;
	OptionsManager::instance()->setOverride("Graphics.Anisotropy", std::unique_ptr<json_t>(json_real(4.0)), "test");
	ASSERT_FLOAT_EQ(gr_anisotropy_setting(), 4.0f);
}

TEST_F(AnisotropySetting, WithoutInGameOptionsTheFlagWins)
{
	Using_in_game_options = false;
	Cmdline_aniso_level = 8;
	ASSERT_FLOAT_EQ(gr_anisotropy_setting(), 8.0f);
}

TEST_F(AnisotropySetting, WithNothingChosenItIsTheCardMaximum)
{
	Using_in_game_options = false;
	Cmdline_aniso_level = 0;
	ASSERT_FLOAT_EQ(gr_anisotropy_setting(), 16.0f);
}

TEST_F(AnisotropySetting, WithNoRendererAnswerItIsOff)
{
	Using_in_game_options = false;
	Cmdline_aniso_level = 0;
	gr_screen.gf_get_property = [](gr_property, void*) { return false; };
	ASSERT_FLOAT_EQ(gr_anisotropy_setting(), 1.0f);
}

// the table parser writes a process-wide default, so put trilinear back after each test
class TextureFilterSetting : public OptionsManagerOverride {
  protected:
	static void parseDefault(const char* value)
	{
		SCP_string text = SCP_string(value) + "\n";
		reset_parse(text.data());
		OptionsManager::instance()->getOptionByKey("Graphics.TextureFilter")->parseDefaultSetting();
		reset_parse(nullptr);
	}
	void SetUp() override { _saved_ingame = Using_in_game_options; }
	void TearDown() override
	{
		parseDefault("Trilinear");
		Using_in_game_options = _saved_ingame;
		OptionsManagerOverride::TearDown();
	}

  private:
	bool _saved_ingame = true;
};

TEST_F(TextureFilterSetting, InGameDefaultIsTrilinear)
{
	Using_in_game_options = true;
	ASSERT_EQ(gr_texture_filter_setting(), 1);
}

TEST_F(TextureFilterSetting, ModTableDefaultIsUsed)
{
	Using_in_game_options = true;
	parseDefault("Bilinear");
	ASSERT_EQ(gr_texture_filter_setting(), 0);
}

TEST_F(TextureFilterSetting, BilinearOverrideBeatsTrilinearDefault)
{
	Using_in_game_options = true;
	OptionsManager::instance()->setOverride("Graphics.TextureFilter", std::unique_ptr<json_t>(json_integer(0)), "test");
	ASSERT_EQ(gr_texture_filter_setting(), 0);
}

// qtFRED sets an override and never loads initial values
TEST_F(TextureFilterSetting, OverrideIsReadWithoutALoad)
{
	Using_in_game_options = true;
	parseDefault("Bilinear");
	OptionsManager::instance()->setOverride("Graphics.TextureFilter", std::unique_ptr<json_t>(json_integer(1)), "test");
	ASSERT_EQ(gr_texture_filter_setting(), 1);
}

TEST_F(TextureFilterSetting, WithoutInGameOptionsTheLauncherDefaultIsTrilinear)
{
	Using_in_game_options = false;
	parseDefault("Bilinear");
	OptionsManager::instance()->setOverride("Graphics.TextureFilter", std::unique_ptr<json_t>(json_integer(0)), "test");
	ASSERT_EQ(gr_texture_filter_setting(), 1);
}

// texture filtering only trades quality for speed, so a mod's +Enforce does not outrank the player's choice
TEST_F(TextureFilterSetting, OverrideBeatsAnEnforcedModValue)
{
	Using_in_game_options = true;
	parseDefault("Bilinear");
	OptionsManager::instance()->enforceOption("Graphics.TextureFilter");
	OptionsManager::instance()->setOverride("Graphics.TextureFilter", std::unique_ptr<json_t>(json_integer(1)), "test");
	const int filter = gr_texture_filter_setting();
	OptionsManager::instance()->unenforceOption("Graphics.TextureFilter");
	ASSERT_EQ(filter, 1);
}

TEST_F(OptionsManagerOverride, LanguageObjectOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Game.Language");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<int>*>(opt);

	// Game.Language's real deserializer resolves a {"name","ext"} pair against Lcl_languages via
	// lcl_find_lang_index_by_name(), rather than storing a plain index -- so unlike the other round-trip
	// tests, this one needs at least one entry in that table to resolve against. A full lcl_init() is too
	// heavy for this test binary, so a single throwaway entry is pushed directly and restored afterwards,
	// since Lcl_languages is a shared global other tests may also touch. The restore runs from a destructor
	// so that a failing assertion below cannot leak the throwaway entry into later tests.
	class LanguagesGuard {
	  public:
		LanguagesGuard() : _original(Lcl_languages) {}
		~LanguagesGuard() { Lcl_languages = _original; }

	  private:
		SCP_vector<lang_info> _original;
	};
	LanguagesGuard guard;

	lang_info test_lang{};
	strcpy_s(test_lang.lang_name, "OverrideRoundTripTestLang");
	strcpy_s(test_lang.lang_ext, "orttl");
	Lcl_languages.push_back(test_lang);
	int expected_idx = sz2i(Lcl_languages.size()) - 1;

	// Exactly the value built in localize.cpp's lcl_init() (language_serializer()'s {"name":..,"ext":..}).
	// language_serializer() itself is not exported, so this test rebuilds the same object.
	OptionsManager::instance()->setOverride("Game.Language",
		std::unique_ptr<json_t>(
			json_pack("{ssss}", "name", test_lang.lang_name, "ext", test_lang.lang_ext)),
		"-lang");

	ASSERT_EQ(typedOpt->getValue(), expected_idx);
}

// Restores the HUD globals these options write, so a failing assertion can't leak a value into later tests.
class HudOptionGlobalsGuard {
  public:
	HudOptionGlobalsGuard()
		: _orb(Cmdline_orb_radar), _scan(Cmdline_dualscanlines), _rearm(Cmdline_rearm_timer),
		  _ballistic(Cmdline_ballistic_gauge), _targetInfo(Extra_target_info)
	{
	}
	~HudOptionGlobalsGuard()
	{
		Cmdline_orb_radar = _orb;
		Cmdline_dualscanlines = _scan;
		Cmdline_rearm_timer = _rearm;
		Cmdline_ballistic_gauge = _ballistic;
		Extra_target_info = _targetInfo;
	}

  private:
	int _orb;
	int _scan;
	int _rearm;
	int _ballistic;
	bool _targetInfo;
};

// cmdline.cpp overrides these keys by name; a typo there would leave an override no option ever reads.
TEST_F(OptionsManagerOverride, HudFlagOptionsAreRegistered)
{
	for (const char* key :
		{"HUD.OrbRadar", "HUD.DualScanLines", "HUD.RearmTimer", "HUD.TargetInfo", "HUD.BallisticGauge"}) {
		ASSERT_NE(OptionsManager::instance()->getOptionByKey(key), nullptr) << key;
	}
}

// These four are read once when the HUD is built, so a change after startup must ask for a restart.
TEST_F(OptionsManagerOverride, HudStartupOptionsApplyOnlyOnInitialLoad)
{
	HudOptionGlobalsGuard guard;
	Cmdline_orb_radar = 0;
	Cmdline_dualscanlines = 0;
	Cmdline_rearm_timer = 0;
	Extra_target_info = false;

	for (const char* key : {"HUD.OrbRadar", "HUD.DualScanLines", "HUD.RearmTimer", "HUD.TargetInfo"}) {
		auto* opt = OptionsManager::instance()->getOptionByKey(key);
		ASSERT_NE(opt, nullptr) << key;
		OptionsManager::instance()->setOverride(key, std::unique_ptr<json_t>(json_true()), "-test_flag");
		opt->loadInitial();
		ASSERT_FALSE(opt->valueChanged(json_false())) << key;
	}

	ASSERT_EQ(Cmdline_orb_radar, 1);
	ASSERT_EQ(Cmdline_dualscanlines, 1);
	ASSERT_EQ(Cmdline_rearm_timer, 1);
	ASSERT_TRUE(Extra_target_info);
}

TEST_F(OptionsManagerOverride, BallisticGaugeOptionAppliesLive)
{
	HudOptionGlobalsGuard guard;
	auto* opt = OptionsManager::instance()->getOptionByKey("HUD.BallisticGauge");
	ASSERT_NE(opt, nullptr);

	Cmdline_ballistic_gauge = 0;
	ASSERT_TRUE(opt->valueChanged(json_true()));
	ASSERT_EQ(Cmdline_ballistic_gauge, 1);
	ASSERT_TRUE(opt->valueChanged(json_false()));
	ASSERT_EQ(Cmdline_ballistic_gauge, 0);
}

// The options UI words the tooltip by source, so a mod's override must not read as a command line flag
TEST_F(OptionsManagerOverride, OverrideRemembersItsSource)
{
	auto* mgr = OptionsManager::instance();
	mgr->setOverride("Test.OverrideSource.Flag", std::unique_ptr<json_t>(json_true()), "-test_flag");
	mgr->setOverride("Test.OverrideSource.Mod",
		std::unique_ptr<json_t>(json_true()),
		"an active mod",
		OverrideSource::Mod);

	ASSERT_EQ(mgr->getOverrideSource("Test.OverrideSource.Flag"), OverrideSource::CommandLine);
	ASSERT_EQ(mgr->getOverrideSource("Test.OverrideSource.Mod"), OverrideSource::Mod);
	ASSERT_FALSE(mgr->getOverrideSource("Test.OverrideSource.Missing").has_value());
}

// Flags are read before the HUD tables, so a mod's override replaces a flag's and must carry the mod's source
TEST_F(OptionsManagerOverride, ModOverrideReplacesFlagOverride)
{
	auto* mgr = OptionsManager::instance();
	mgr->setOverride("Test.OverrideSource.Replaced", std::unique_ptr<json_t>(json_true()), "-test_flag");
	mgr->setOverride("Test.OverrideSource.Replaced",
		std::unique_ptr<json_t>(json_false()),
		"an active mod",
		OverrideSource::Mod);

	ASSERT_EQ(mgr->getOverrideSource("Test.OverrideSource.Replaced"), OverrideSource::Mod);
	ASSERT_EQ(mgr->getOverrideReason("Test.OverrideSource.Replaced"), SCP_string("an active mod"));
}

TEST_F(OptionsManagerOverride, StringOverrideKeepsItsSource)
{
	auto* mgr = OptionsManager::instance();
	mgr->setOverride("Test.OverrideSource.String", "true", "an active mod", OverrideSource::Mod);

	ASSERT_EQ(mgr->getOverrideSource("Test.OverrideSource.String"), OverrideSource::Mod);
}

namespace {
const json_t* described(const json_t* list, const char* key)
{
	for (size_t i = 0; i < json_array_size(list); ++i) {
		const json_t* entry = json_array_get(list, i);
		const char* entry_key = json_string_value(json_object_get(entry, "key"));
		if (entry_key != nullptr && SCP_string(entry_key) == key) {
			return entry;
		}
	}
	return nullptr;
}

// some value lists ask the renderer what it can do; -get_options runs on the stub renderer, so these tests do too
class StubRenderer {
  public:
	StubRenderer() : _saved(gr_screen)
	{
		gr_stub_init_function_pointers();
	}
	~StubRenderer()
	{
		gr_stub_set_queried_hardware(false, 0.0f);
		gr_screen = _saved;
	}

  private:
	screen _saved;
};
} // namespace

// a launcher shows "needs restart" from this, and only options bound to apply at once may say no
TEST_F(OptionsManagerOverrideLocalized, DescribedOptionsSayWhichNeedARestart)
{
	StubRenderer renderer;
	std::unique_ptr<json_t> list(OptionsManager::instance()->describeOptions());
	const json_t* live = described(list.get(), "Input.JoystickDeadZone");
	const json_t* custom = described(list.get(), "HUD.BallisticGauge");
	const json_t* once = described(list.get(), "HUD.OrbRadar");
	ASSERT_NE(live, nullptr);
	ASSERT_NE(custom, nullptr);
	ASSERT_NE(once, nullptr);
	ASSERT_TRUE(json_is_false(json_object_get(live, "restart_needed")));
	// a listener of its own may or may not apply at once, so it is reported as needing a restart
	ASSERT_TRUE(json_is_true(json_object_get(custom, "restart_needed")));
	ASSERT_TRUE(json_is_true(json_object_get(once, "restart_needed")));
	// bind_to_once, and bind_to followed by a listener of its own (3D shockwaves)
	for (const char* key : {"HUD.TargetInfo", "Graphics.3DShockwaves"}) {
		const json_t* entry = described(list.get(), key);
		ASSERT_NE(entry, nullptr) << key;
		ASSERT_TRUE(json_is_true(json_object_get(entry, "restart_needed"))) << key;
	}
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(once, "title"))), "3D Radar");
	const json_t* off = json_array_get(json_object_get(once, "values"), 0);
	ASSERT_TRUE(json_is_false(json_object_get(off, "value")));
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(off, "display"))), "Off");
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(once, "type"))), "selection");
	ASSERT_EQ(json_array_size(json_object_get(once, "values")), 2u);
	ASSERT_TRUE(json_is_false(json_object_get(once, "default")));
}

TEST_F(OptionsManagerOverrideLocalized, DescribedOptionsShowAFlagOverride)
{
	auto* mgr = OptionsManager::instance();
	mgr->setOverride("HUD.OrbRadar", std::unique_ptr<json_t>(json_true()), "-orbradar");
	StubRenderer renderer;
	std::unique_ptr<json_t> list(mgr->describeOptions());
	const json_t* entry = described(list.get(), "HUD.OrbRadar");
	ASSERT_NE(entry, nullptr);
	ASSERT_TRUE(json_is_true(json_object_get(entry, "current")));
	ASSERT_TRUE(json_is_false(json_object_get(entry, "default")));
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(entry, "overridden_by"))), "cmdline");
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(entry, "override_reason"))), "-orbradar");
	ASSERT_TRUE(json_is_false(json_object_get(entry, "enforced")));

	const json_t* free_entry = described(list.get(), "HUD.BallisticGauge");
	ASSERT_NE(free_entry, nullptr);
	ASSERT_TRUE(json_is_null(json_object_get(free_entry, "overridden_by")));
}

TEST_F(OptionsManagerOverrideLocalized, DescribedRangeOptionsGiveTheirBounds)
{
	StubRenderer renderer;
	std::unique_ptr<json_t> list(OptionsManager::instance()->describeOptions());
	const json_t* entry = described(list.get(), "Input.JoystickDeadZone");
	ASSERT_NE(entry, nullptr);
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(entry, "type"))), "range");
	const json_t* range = json_object_get(entry, "range");
	ASSERT_EQ(json_number_value(json_object_get(range, "min")), 0.0);
	ASSERT_EQ(json_number_value(json_object_get(range, "max")), 45.0);
	// an int option, so a launcher must not write it a real number
	ASSERT_TRUE(json_is_true(json_object_get(range, "integer")));
	ASSERT_TRUE(json_is_number(json_object_get(entry, "default")));
}

// a listener that writes the global its default reads would make the default look like the saved value, and an
// enforced option leaves the list during loading; both must still be described as they were before loading
TEST_F(OptionsManagerOverrideLocalized, DescribedOptionsKeepWhatTheyWereBeforeLoading)
{
	StubRenderer renderer;
	auto* mgr = OptionsManager::instance();
	auto* lighting = mgr->getOptionByKey("Graphics.Lighting");
	ASSERT_NE(lighting, nullptr);
	SCP_string saved_default = lighting->getDefaultValueDescription().serialized;
	SCP_string other;
	for (const auto& value : lighting->getValidValues()) {
		if (value.serialized != saved_default) {
			other = value.serialized;
		}
	}
	ASSERT_FALSE(other.empty());
	json_error_t err;
	mgr->setConfigValue("Graphics.Lighting", std::unique_ptr<json_t>(json_loads(other.c_str(), JSON_DECODE_ANY, &err)));
	mgr->enforceOption("HUD.BallisticGauge");

	bool saved_flag = Cmdline_get_options;
	Cmdline_get_options = true;
	mgr->loadInitialValues();
	Cmdline_get_options = saved_flag;
	std::unique_ptr<json_t> list(mgr->describeOptions());
	char* dumped = json_dumps(json_object_get(described(list.get(), "Graphics.Lighting"), "default"),
		JSON_ENCODE_ANY | JSON_COMPACT);
	SCP_string described_default = (dumped != nullptr) ? dumped : "";
	free(dumped);
	const json_t* enforced = described(list.get(), "HUD.BallisticGauge");
	bool enforced_is_true = enforced != nullptr && json_is_true(json_object_get(enforced, "enforced"));

	mgr->unenforceOption("HUD.BallisticGauge");
	mgr->forgetLoadedOptions();
	// the listener wrote the saved value into the global the default reads, so load the old default back the same way
	mgr->discardChanges();
	mgr->setConfigValue("Graphics.Lighting",
		std::unique_ptr<json_t>(json_loads(saved_default.c_str(), JSON_DECODE_ANY, &err)));
	lighting->loadInitial();
	mgr->discardChanges();

	ASSERT_EQ(described_default, saved_default);
	ASSERT_TRUE(enforced_is_true);
	ASSERT_NE(mgr->getOptionByKey("HUD.BallisticGauge"), nullptr);
}

TEST_F(OptionsManagerOverrideLocalized, DescribedOptionsShowAModOverride)
{
	StubRenderer renderer;
	auto* mgr = OptionsManager::instance();
	mgr->setOverride("HUD.OrbRadar", std::unique_ptr<json_t>(json_false()), "an active mod", OverrideSource::Mod);
	std::unique_ptr<json_t> list(mgr->describeOptions());
	const json_t* entry = described(list.get(), "HUD.OrbRadar");
	ASSERT_NE(entry, nullptr);
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(entry, "overridden_by"))), "mod");
}

// a title from a legacy-encoded translation must arrive as text, not as a missing key
TEST(OptionsJsonText, TextThatIsNotUtf8IsReadAsLatin1)
{
	std::unique_ptr<json_t> text(options::json_text("caf\xe9"));
	ASSERT_NE(text, nullptr);
	ASSERT_EQ(SCP_string(json_string_value(text.get())), "caf\xc3\xa9");
}

// a launcher greys these and shows the explanation as the reason
TEST_F(OptionsManagerOverrideLocalized, DescribedOptionsCarryWhatTheyNeed)
{
	StubRenderer renderer;
	auto* mgr = OptionsManager::instance();
	mgr->setRequirement("Graphics.Gamma", "vulkan", "Needs the Vulkan renderer");
	std::unique_ptr<json_t> list(mgr->describeOptions());
	mgr->forgetLoadedOptions();
	const json_t* needing = described(list.get(), "Graphics.Gamma");
	const json_t* free_entry = described(list.get(), "Input.JoystickDeadZone");
	ASSERT_NE(needing, nullptr);
	ASSERT_NE(free_entry, nullptr);
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(needing, "requires"))), "vulkan");
	ASSERT_EQ(SCP_string(json_string_value(json_object_get(needing, "requires_text"))), "Needs the Vulkan renderer");
	ASSERT_TRUE(json_is_null(json_object_get(free_entry, "requires")));
}

// with no renderer running, these lists come only from the windowless hardware query
TEST_F(OptionsManagerOverrideLocalized, DescribedHardwareListsFollowTheQuery)
{
	StubRenderer renderer;
	auto* mgr = OptionsManager::instance();
	std::unique_ptr<json_t> before(mgr->describeOptions());
	// the ray-traced options may already be dropped from the shared registry by an earlier test, so ask what their
	// value lists ask
	ASSERT_FALSE(shadows_raytracing_supported());
	gr_stub_set_queried_hardware(true, 8.0f);
	std::unique_ptr<json_t> after(mgr->describeOptions());
	ASSERT_TRUE(shadows_raytracing_supported());
	// the query answers only these two questions; every other capability stays the stub's "no"
	ASSERT_FALSE(gr_is_capable(gr_capability::CAPABILITY_SHADOWS));
	const json_t* aniso_before = described(before.get(), "Graphics.Anisotropy");
	const json_t* aniso_after = described(after.get(), "Graphics.Anisotropy");
	// the anisotropy option lives in the OpenGL texture code, so builds without OpenGL don't have it
	if (aniso_before == nullptr && aniso_after == nullptr) {
		GTEST_SKIP() << "Graphics.Anisotropy is not built without OpenGL";
	}
	ASSERT_NE(aniso_before, nullptr);
	ASSERT_NE(aniso_after, nullptr);
	ASSERT_EQ(json_array_size(json_object_get(aniso_before, "values")), 0u);
	// 1x, 2x, 4x and 8x: the levels stop at the card's maximum
	ASSERT_EQ(json_array_size(json_object_get(aniso_after, "values")), 4u);
	ASSERT_FLOAT_EQ(static_cast<float>(json_number_value(json_object_get(aniso_before, "default"))), 1.0f);
	ASSERT_FLOAT_EQ(static_cast<float>(json_number_value(json_object_get(aniso_after, "default"))), 8.0f);
}

// The in-game options dialog reads this record to grey out an option whose requirement the running renderer
// does not meet. This test checks only the record; the dialog is ImGui code with no test.
TEST_F(OptionsManagerOverride, RequirementIsQueryable)
{
	auto* mgr = OptionsManager::instance();

	ASSERT_FALSE(mgr->getRequirement("Test.Requirement.NeverSet").has_value());

	mgr->setRequirement("Test.Requirement.Set", "opengl", "Needs the OpenGL renderer");
	auto requirement = mgr->getRequirement("Test.Requirement.Set");
	ASSERT_TRUE(requirement.has_value());
	ASSERT_EQ(requirement->first, "opengl");
	ASSERT_EQ(requirement->second, "Needs the OpenGL renderer");

	mgr->clearRequirements();
	ASSERT_FALSE(mgr->getRequirement("Test.Requirement.Set").has_value());
}

TEST(OptionRequirement, MetByTheMatchingRendererOnly)
{
	ASSERT_TRUE(gr_option_requirement_met("opengl", GraphicsAPI::OpenGL, false));
	ASSERT_FALSE(gr_option_requirement_met("opengl", GraphicsAPI::Vulkan, true));
	ASSERT_TRUE(gr_option_requirement_met("vulkan", GraphicsAPI::Vulkan, false));
	ASSERT_FALSE(gr_option_requirement_met("vulkan", GraphicsAPI::OpenGL, false));
	ASSERT_FALSE(gr_option_requirement_met("vulkan", GraphicsAPI::Stub, false));
}

TEST(OptionRequirement, RaytracingNeedsVulkanAndTheHardware)
{
	ASSERT_TRUE(gr_option_requirement_met("raytracing", GraphicsAPI::Vulkan, true));
	ASSERT_FALSE(gr_option_requirement_met("raytracing", GraphicsAPI::Vulkan, false));
	ASSERT_FALSE(gr_option_requirement_met("raytracing", GraphicsAPI::OpenGL, true));
}

// A requirement the UI cannot judge must never grey out a control.
TEST(OptionRequirement, UnknownRequirementCountsAsMet)
{
	ASSERT_TRUE(gr_option_requirement_met("some_future_feature", GraphicsAPI::OpenGL, false));
	ASSERT_TRUE(gr_option_requirement_met("", GraphicsAPI::Vulkan, false));
}

// The HDR and raytraced-shadow options stay in the list on a renderer that cannot use them, with a requirement,
// so the options menu greys them out instead of hiding them. The fixture runs gr_init() with the Stub renderer,
// which has neither, right before the test: other fixtures clear the requirements in their TearDown.
class OptionRequirementAfterGrInit : public test::FSTestFixture {
  public:
	OptionRequirementAfterGrInit() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}

  protected:
	void TearDown() override
	{
		OptionsManager::instance()->clearRequirements();
		test::FSTestFixture::TearDown();
	}
};

TEST_F(OptionRequirementAfterGrInit, UnsupportedGraphicsOptionsStayWithARequirement)
{
	auto* mgr = OptionsManager::instance();
	for (const char* key : {"Graphics.HDR", "Graphics.ShadowRenderMethod"}) {
		if (mgr->getOptionByKey(key) == nullptr) {
			GTEST_SKIP() << key << " is not registered in this build";
		}
		auto requirement = mgr->getRequirement(key);
		ASSERT_TRUE(requirement.has_value()) << key;
		ASSERT_FALSE(gr_option_requirement_met(requirement->first, GraphicsAPI::Stub, false)) << key;
	}
}
