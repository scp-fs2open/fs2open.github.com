#include "options/OptionsManager.h"
#include "options/Option.h"
#include "graphics/2d.h"
#include "graphics/shadows.h"
#include "cmdline/cmdline.h"
#include "hud/hud.h"
#include "localization/localize.h"
#include "mod_table/mod_table.h"
#include "parse/parselo.h"
#include "math/floating.h"
#include "osapi/osregistry.h"
#include "utils/finally.h"

#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <type_traits>

using namespace options;

// OptionsManager is a process-wide singleton, so overrides and pending edits set by one test would
// otherwise stay visible to every test that runs after it in the same binary. Clear both after each test.
class OptionsManagerOverride : public ::testing::Test {
  protected:
	void TearDown() override
	{
		OptionsManager::instance()->clearOverrides();
		OptionsManager::instance()->discardChanges();
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
// and its teardown resets Xstr_inited, so a display call from this fixture would hit the Int3() regardless
// of test order. getValue() only exercises the deserializer, which is exactly the contract these tests are
// checking anyway. The one test that must go through a display function uses the
// OptionsManagerOverrideLocalized fixture below instead.

// The display and window mode apply in a hook after the persist; what it could not apply must reach the restart prompt
// like an option whose listener returned false. Nothing is pending, so the test writes no settings.
TEST_F(OptionsManagerOverride, PostPersistHookReportsWhatItCouldNotApply)
{
	auto* mgr = OptionsManager::instance();
	auto* opt = mgr->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr);

	const OptionBase* unapplied_by_hook = opt;
	int calls = 0;
	const int hook = mgr->addPostPersistHook([&](const SCP_vector<const OptionBase*>& changed) {
		++calls;
		EXPECT_TRUE(changed.empty());
		SCP_vector<const OptionBase*> unapplied;
		if (unapplied_by_hook != nullptr) {
			unapplied.push_back(unapplied_by_hook);
		}
		return unapplied;
	});
	auto remove_hook = util::finally([mgr, hook]() { mgr->removePostPersistHook(hook); });

	mgr->discardChanges();
	const auto unchanged = mgr->persistChanges();
	ASSERT_EQ(unchanged.size(), 1u);
	EXPECT_EQ(unchanged[0], opt);

	unapplied_by_hook = nullptr;
	EXPECT_TRUE(mgr->persistChanges().empty());
	EXPECT_EQ(calls, 2);
}

TEST_F(OptionsManagerOverride, RemovedPostPersistHookNoLongerRuns)
{
	auto* mgr = OptionsManager::instance();
	auto* opt = mgr->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr);

	const int hook = mgr->addPostPersistHook(
		[opt](const SCP_vector<const OptionBase*>&) { return SCP_vector<const OptionBase*>{opt}; });
	mgr->removePostPersistHook(hook);

	mgr->discardChanges();
	EXPECT_TRUE(mgr->persistChanges().empty());
}

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
	// This option is not always present: gr_init() calls shadows_remove_unsupported_options(), which removes
	// it when the renderer cannot raytrace. Every FSTestFixture test in this binary runs gr_init() with the
	// Stub renderer, so by the time this test runs the option is usually already gone.
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.ShadowRenderMethod");
	if (opt == nullptr) {
		GTEST_SKIP() << "Graphics.ShadowRenderMethod was removed by shadows_remove_unsupported_options(), so "
		                "this binary cannot exercise its default_func.";
	}
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

// Persisting writes the settings file, so this needs FSTestFixture's portable profile
// (test_data/data/options_settings.ini, ignored by git); the value written is the same on every run.
class OptionsManagerPersistHook : public test::FSTestFixture {
  public:
	OptionsManagerPersistHook() : test::FSTestFixture(INIT_CFILE) { pushModDir("options"); }

  protected:
	void SetUp() override
	{
		test::FSTestFixture::SetUp();

		// Once any os_init finds no settings file the flag stays false with no profile, so each persist would leak one
		_save_found = Ingame_options_save_found;
		os_deinit_registry_stuff();
		Ingame_options_save_found = true;
		os_init_registry_stuff(nullptr, "Test");

		_option =
			OptionBuilder<int>("Test.Hook.Changed", SCP_string("Test.Hook.Changed"), SCP_string("Test.Hook.Changed"))
				.default_val(0)
				.change_listener([](const int&, bool) { return true; })
				.finish();
	}
	void TearDown() override
	{
		OptionsManager::instance()->discardChanges();
		OptionsManager::instance()->removeOption(_option);
		Ingame_options_save_found = _save_found;

		test::FSTestFixture::TearDown();
	}

	std::shared_ptr<const Option<int>> _option;
	bool _save_found = true;
};

TEST_F(OptionsManagerPersistHook, HookReceivesTheOptionsThisPersistNotified)
{
	auto* mgr = OptionsManager::instance();
	SCP_vector<const OptionBase*> received;
	const int hook = mgr->addPostPersistHook([&received](const SCP_vector<const OptionBase*>& changed) {
		received = changed;
		return SCP_vector<const OptionBase*>();
	});
	auto remove_hook = util::finally([mgr, hook]() { mgr->removePostPersistHook(hook); });

	mgr->setConfigValue("Test.Hook.Changed", std::unique_ptr<json_t>(json_integer(1)));
	EXPECT_TRUE(mgr->persistChanges().empty());
	ASSERT_EQ(received.size(), 1u);
	EXPECT_EQ(received[0], _option.get());

	received.clear();
	mgr->setConfigValue("Test.Hook.Changed", std::unique_ptr<json_t>(json_integer(1)));
	EXPECT_TRUE(mgr->persistOptionChanges(_option.get()));
	ASSERT_EQ(received.size(), 1u);
	EXPECT_EQ(received[0], _option.get());
}

// persistOptionChanges() returns one bool for its own option, so another option a hook reports only reaches the log
TEST_F(OptionsManagerPersistHook, PersistOneOptionReportsOnlyThatOption)
{
	auto* mgr = OptionsManager::instance();
	auto* other = mgr->getOptionByKey("Graphics.VSync");
	ASSERT_NE(other, nullptr);

	const OptionBase* reported = _option.get();
	const int hook = mgr->addPostPersistHook(
		[&reported](const SCP_vector<const OptionBase*>&) { return SCP_vector<const OptionBase*>{reported}; });
	auto remove_hook = util::finally([mgr, hook]() { mgr->removePostPersistHook(hook); });

	mgr->setConfigValue("Test.Hook.Changed", std::unique_ptr<json_t>(json_integer(1)));
	EXPECT_FALSE(mgr->persistOptionChanges(_option.get()));

	reported = other;
	mgr->setConfigValue("Test.Hook.Changed", std::unique_ptr<json_t>(json_integer(1)));
	EXPECT_TRUE(mgr->persistOptionChanges(_option.get()));
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

// valueChanged() writes the bound variable, which clearOverrides() does not touch; put back the configured value.
// getValue(), not getCurrentValueDescription(): the display functions call XSTR(), which this fixture never initializes.
template <typename T>
static void restore_bound_value(const char* key)
{
	auto* opt = static_cast<const Option<T>*>(OptionsManager::instance()->getOptionByKey(key));
	ASSERT_NE(opt, nullptr) << key;
	const T configured = opt->getValue();
	std::unique_ptr<json_t> value(std::is_same<T, bool>::value ? json_boolean(configured) : json_integer(configured));
	EXPECT_TRUE(opt->valueChanged(value.get())) << key << " was not restored";
}

TEST_F(OptionsManagerOverride, UpscaleFilterAppliesWithoutARestart)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.UpscaleFilter");
	ASSERT_NE(opt, nullptr);
	const bool saved = Using_in_game_options;
	Using_in_game_options = true;

	std::unique_ptr<json_t> nearest(json_integer(1));
	std::unique_ptr<json_t> linear(json_integer(0));
	const bool applied_nearest = opt->valueChanged(nearest.get());
	const bool is_nearest = gr_upscale_nearest();
	Using_in_game_options = false;
	const bool without_ingame = gr_upscale_nearest();
	Using_in_game_options = true;
	const bool saved_native_res_hud = Gr_native_res_hud;
	Gr_native_res_hud = true;
	const bool with_native_res_hud = gr_upscale_nearest();
	Gr_native_res_hud = saved_native_res_hud;
	const bool applied_linear = opt->valueChanged(linear.get());
	const bool is_linear = !gr_upscale_nearest();
	Using_in_game_options = saved;
	restore_bound_value<int>("Graphics.UpscaleFilter");

	EXPECT_TRUE(applied_nearest);
	EXPECT_TRUE(is_nearest);
	EXPECT_FALSE(without_ingame);
	EXPECT_FALSE(with_native_res_hud);
	EXPECT_TRUE(applied_linear);
	EXPECT_TRUE(is_linear);
}

// The menu sort is not stable, so the two options need their own importance, after the window mode
TEST_F(OptionsManagerOverride, UpscaleOptionsFollowTheWindowMode)
{
	SCP_vector<SCP_string> keys;
	for (const auto& opt : OptionsManager::instance()->getOptions()) {
		keys.push_back(opt->getConfigKey());
	}
	auto at = [&keys](const char* key) { return std::find(keys.begin(), keys.end(), key) - keys.begin(); };
	EXPECT_LT(at("Graphics.Resolution"), at("Graphics.WindowMode"));
	EXPECT_EQ(at("Graphics.WindowMode") + 1, at("Graphics.UpscaleFilter"));
	EXPECT_EQ(at("Graphics.UpscaleFilter") + 1, at("Graphics.KeepAspectRatio"));
}

// 640x480 on a 1920x1080 OpenGL window: the frame fills 1440x1080 in the middle
class KeepAspectRatio : public OptionsManagerOverride {
  protected:
	void SetUp() override
	{
		saved_window_res = Cmdline_window_res;
		saved_mode = gr_screen.mode;
		saved_w = gr_screen.max_w;
		saved_h = gr_screen.max_h;
		saved_ingame = Using_in_game_options;
		saved_vr = Cmdline_enable_vr;
		saved_fred = Fred_running;
		saved_native_res_hud = Gr_native_res_hud;
		saved_nograb = Cmdline_nograb;

		Cmdline_window_res.emplace(static_cast<uint16_t>(1920), static_cast<uint16_t>(1080));
		gr_screen.mode = GraphicsAPI::OpenGL;
		gr_screen.max_w = 640;
		gr_screen.max_h = 480;
		Using_in_game_options = true;
	}

	void TearDown() override
	{
		restore_bound_value<bool>("Graphics.KeepAspectRatio");
		Cmdline_window_res = saved_window_res;
		gr_screen.mode = saved_mode;
		gr_screen.max_w = saved_w;
		gr_screen.max_h = saved_h;
		Using_in_game_options = saved_ingame;
		Cmdline_enable_vr = saved_vr;
		Fred_running = saved_fred;
		Gr_native_res_hud = saved_native_res_hud;
		Cmdline_nograb = saved_nograb;
		OptionsManagerOverride::TearDown();
	}

	static void setKeep(bool keep)
	{
		auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.KeepAspectRatio");
		ASSERT_NE(opt, nullptr);
		std::unique_ptr<json_t> value(keep ? json_true() : json_false());
		EXPECT_TRUE(opt->valueChanged(value.get()));
	}

	static std::array<int, 4> rect()
	{
		std::array<int, 4> r{};
		gr_present_rect(r[0], r[1], r[2], r[3]);
		return r;
	}

	std::optional<std::pair<uint16_t, uint16_t>> saved_window_res;
	GraphicsAPI saved_mode{};
	int saved_w = 0, saved_h = 0;
	bool saved_ingame = false;
	bool saved_vr = false;
	int saved_fred = 0;
	bool saved_native_res_hud = false;
	bool saved_nograb = false;
};

TEST_F(KeepAspectRatio, StretchesWhenOff)
{
	setKeep(false);
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
}

TEST_F(KeepAspectRatio, StretchesWithoutTheInGameOptions)
{
	setKeep(true);
	Using_in_game_options = false;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
}

TEST_F(KeepAspectRatio, ConfinesThePointerOnlyInFullscreenWithBars)
{
	setKeep(true);
	std::array<int, 4> r{};
	EXPECT_TRUE(gr_pointer_confine_rect(os::ViewportState::Fullscreen, r[0], r[1], r[2], r[3]));
	EXPECT_EQ(r, (std::array<int, 4>{240, 0, 1440, 1080}));
	EXPECT_FALSE(gr_pointer_confine_rect(os::ViewportState::Windowed, r[0], r[1], r[2], r[3]));
	EXPECT_FALSE(gr_pointer_confine_rect(os::ViewportState::Borderless, r[0], r[1], r[2], r[3]));
	Cmdline_nograb = true;
	EXPECT_FALSE(gr_pointer_confine_rect(os::ViewportState::Fullscreen, r[0], r[1], r[2], r[3]));
	Cmdline_nograb = false;
	setKeep(false);
	EXPECT_FALSE(gr_pointer_confine_rect(os::ViewportState::Fullscreen, r[0], r[1], r[2], r[3]));
}

TEST_F(KeepAspectRatio, PillarboxesAndMapsTheMouse)
{
	setKeep(true);
	EXPECT_EQ(rect(), (std::array<int, 4>{240, 0, 1440, 1080}));

	float x = 240.0f + 720.0f, y = 540.0f;
	gr_window_to_render_pos(x, y);
	EXPECT_FLOAT_EQ(x, 320.0f);
	EXPECT_FLOAT_EQ(y, 240.0f);
	gr_render_to_window_pos(x, y);
	EXPECT_FLOAT_EQ(x, 960.0f);
	EXPECT_FLOAT_EQ(y, 540.0f);

	float dx = 225.0f, dy = 225.0f;
	gr_window_to_render_delta(dx, dy);
	EXPECT_FLOAT_EQ(dx, 100.0f);
	EXPECT_FLOAT_EQ(dy, 100.0f);
}

TEST_F(KeepAspectRatio, LetterboxesATallerScreen)
{
	setKeep(true);
	Cmdline_window_res.emplace(static_cast<uint16_t>(1280), static_cast<uint16_t>(1024));
	gr_screen.max_w = 1920;
	gr_screen.max_h = 1080;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 152, 1280, 720}));

	float x = 640.0f, y = 152.0f + 360.0f;
	gr_window_to_render_pos(x, y);
	EXPECT_FLOAT_EQ(x, 960.0f);
	EXPECT_FLOAT_EQ(y, 540.0f);
	gr_render_to_window_pos(x, y);
	EXPECT_FLOAT_EQ(x, 640.0f);
	EXPECT_FLOAT_EQ(y, 512.0f);
}

// 1720x720 on 1920x1080 leaves 277 rows; the odd row goes to the bottom bar, where the blit puts it
TEST_F(KeepAspectRatio, OddBarTotalPutsTheExtraRowAtTheBottom)
{
	setKeep(true);
	gr_screen.max_w = 1720;
	gr_screen.max_h = 720;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 138, 1920, 803}));
	EXPECT_EQ(gr_present_rect_bottom(138, 803, 1080), 139);

	float x = 0.0f, y = 138.0f;
	gr_window_to_render_pos(x, y);
	EXPECT_FLOAT_EQ(y, 0.0f);
	gr_render_to_window_pos(x, y);
	EXPECT_FLOAT_EQ(y, 138.0f);
}

// 1280x720 fits a 1366x768 window as 1365x768; the one-pixel rest is rounding
TEST_F(KeepAspectRatio, FillsTheWindowWithinOnePixel)
{
	setKeep(true);
	Cmdline_window_res.emplace(static_cast<uint16_t>(1366), static_cast<uint16_t>(768));
	gr_screen.max_w = 1280;
	gr_screen.max_h = 720;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1366, 768}));
	gr_screen.max_w = 1366;
	gr_screen.max_h = 767;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1366, 768}));
}

TEST_F(KeepAspectRatio, StretchesInVulkanVRAndFRED)
{
	setKeep(true);
	gr_screen.mode = GraphicsAPI::Vulkan;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
	gr_screen.mode = GraphicsAPI::OpenGL;
	Cmdline_enable_vr = true;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
	Cmdline_enable_vr = false;
	Fred_running = 1;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
}

TEST_F(KeepAspectRatio, StretchesWithTheHUDAtOutputResolution)
{
	setKeep(true);
	Gr_native_res_hud = true;
	EXPECT_EQ(rect(), (std::array<int, 4>{0, 0, 1920, 1080}));
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

// These three are read once when the HUD is built, so a change after startup must ask for a restart.
TEST_F(OptionsManagerOverride, HudStartupOptionsApplyOnlyOnInitialLoad)
{
	HudOptionGlobalsGuard guard;
	Cmdline_orb_radar = 0;
	Cmdline_dualscanlines = 0;
	Cmdline_rearm_timer = 0;

	for (const char* key : {"HUD.OrbRadar", "HUD.DualScanLines", "HUD.RearmTimer"}) {
		auto* opt = OptionsManager::instance()->getOptionByKey(key);
		ASSERT_NE(opt, nullptr) << key;
		OptionsManager::instance()->setOverride(key, std::unique_ptr<json_t>(json_true()), "-test_flag");
		opt->loadInitial();
		ASSERT_FALSE(opt->valueChanged(json_false())) << key;
		ASSERT_TRUE(opt->getIsOnce()) << key;
	}

	ASSERT_EQ(Cmdline_orb_radar, 1);
	ASSERT_EQ(Cmdline_dualscanlines, 1);
	ASSERT_EQ(Cmdline_rearm_timer, 1);
}

TEST_F(OptionsManagerOverride, TargetInfoOptionAppliesLive)
{
	HudOptionGlobalsGuard guard;
	auto* opt = OptionsManager::instance()->getOptionByKey("HUD.TargetInfo");
	ASSERT_NE(opt, nullptr);

	Extra_target_info = false;
	ASSERT_TRUE(opt->valueChanged(json_true()));
	ASSERT_TRUE(Extra_target_info);
	ASSERT_TRUE(opt->valueChanged(json_false()));
	ASSERT_FALSE(Extra_target_info);
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
