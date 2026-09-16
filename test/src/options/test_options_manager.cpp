#include "options/OptionsManager.h"
#include "options/Option.h"
#include "graphics/2d.h"
#include "graphics/shadows.h"
#include "localization/localize.h"
#include "math/floating.h"

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

// Graphics.Anisotropy is only registered in builds with the OpenGL backend compiled in
// (its Option<float> lives in gropengltexture.cpp, which the CMake build excludes entirely
// when FSO_BUILD_WITH_OPENGL is off) -- so, like Graphics.RenderAPI above, round-trip
// coverage needs a build-config split rather than a single unconditional test.
#ifdef WITH_OPENGL

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

#else

TEST_F(OptionsManagerOverride, AnisotropyIsAbsentWithoutOpenGLSupport)
{
	ASSERT_EQ(OptionsManager::instance()->getOptionByKey("Graphics.Anisotropy"), nullptr);
}

#endif

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
