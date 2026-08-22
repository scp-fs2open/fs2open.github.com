
#include "options/OptionsManager.h"
#include "options/Option.h"
#include "graphics/2d.h"
#include "graphics/shadows.h"
#include "localization/localize.h"

#include <gtest/gtest.h>

using namespace options;

// Probe: real Option<T> instances are defined via static initializers scattered across many translation
// units in libcode.a. Confirm the linker actually pulled at least one of them into the unittests binary
// (rather than static init being stripped as unreferenced) before trusting any test that exercises a real
// option's deserializer.
TEST(OptionsManagerOverride, RealOptionsAreRegistered)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr) << "Graphics.VSync was not registered -- static Option<T> initializers are not "
	                           "linked into this test binary, so override round-trip tests against real "
	                           "options are not possible here.";
}

// Regression coverage for the cmdline-priority-rework: OptionsManager::getValueFromConfig() must resolve
// a command-line override ahead of a persisted ini value, but a live, unsaved in-session edit (_changed_values)
// must win over that override so an overridden control in the options UI stays meaningfully editable.

TEST(OptionsManagerOverride, OverrideIsReturnedWhenNoEditIsPending)
{
	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Test.OverridePriority.Fresh", "42", "-test_flag");

	auto result = mgr->getValueFromConfig("Test.OverridePriority.Fresh");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 42);
}

TEST(OptionsManagerOverride, SessionEditWinsOverOverride)
{
	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Test.OverridePriority.Edited", "42", "-test_flag");
	mgr->setConfigValue("Test.OverridePriority.Edited", std::unique_ptr<json_t>(json_integer(7)));

	auto result = mgr->getValueFromConfig("Test.OverridePriority.Edited");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 7);

	// The override should resume winning once the pending edit is discarded (e.g. the player closed the
	// options menu without saving), so it survives for the rest of the session.
	mgr->discardChanges();

	result = mgr->getValueFromConfig("Test.OverridePriority.Edited");
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(json_integer_value(result->get()), 42);
}

TEST(OptionsManagerOverride, OverrideReasonIsQueryable)
{
	auto* mgr = OptionsManager::instance();

	ASSERT_FALSE(mgr->getOverrideReason("Test.OverridePriority.NeverSet").has_value());

	mgr->setOverride("Test.OverridePriority.Reason", "1", "-my_flag");

	auto reason = mgr->getOverrideReason("Test.OverridePriority.Reason");
	ASSERT_TRUE(reason.has_value());
	ASSERT_EQ(*reason, "-my_flag");
}

// The tests above exercise getValueFromConfig() directly with synthetic integer payloads. The tests below
// instead go through real, statically-registered Option<T> instances (confirmed live by the probe test
// above) using the exact JSON payload shapes cmdline.cpp constructs for each type, so a malformed payload
// shows up as a deserialization failure here instead of silently falling back to that option's default_func
// and masking the bug.
//
// These call the typed Option<T>::getValue() (via a static_cast down to the option's real, statically-known
// type) rather than getCurrentValueDescription(), because several options' display functions call XSTR() for
// localized display strings (e.g. "On"/"Off", the shadow quality tiers), and this test binary never runs
// lcl_init() to set up the localization tables XSTR needs. getValue() only exercises the deserializer, which
// is exactly the contract these tests are checking anyway.

TEST(OptionsManagerOverride, BoolOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.VSync");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<bool>*>(opt);

	auto* mgr = OptionsManager::instance();

	mgr->setOverride("Graphics.VSync", "false", "-no_vsync");
	ASSERT_FALSE(typedOpt->getValue());

	mgr->setOverride("Graphics.VSync", "true", "-no_vsync");
	ASSERT_TRUE(typedOpt->getValue());
}

TEST(OptionsManagerOverride, ResolutionObjectOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Resolution");
	ASSERT_NE(opt, nullptr);

	// ResolutionInfo is a file-local type in 2d.cpp, not reachable from here, so this test goes through
	// getCurrentValueDescription() instead -- safe because resolution_display() is a plain sprintf with no
	// XSTR() call.
	SCP_string override;
	sprintf(override, "{\"width\":%d,\"height\":%d}", 1920, 1080);
	OptionsManager::instance()->setOverride("Graphics.Resolution", override, "-res");

	ASSERT_EQ(opt->getCurrentValueDescription().display, "1920x1080");
}

TEST(OptionsManagerOverride, EnumIntOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Shadows");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<ShadowQuality>*>(opt);

	// Exactly the payload shape built in cmdline.cpp's -shadow_quality handling (static_cast<int>(enum) as a
	// plain decimal string).
	OptionsManager::instance()->setOverride("Graphics.Shadows", "2", "-shadow_quality");

	ASSERT_EQ(typedOpt->getValue(), ShadowQuality::Medium);
}

// Graphics.RenderAPI only exists in builds compiled with Vulkan support (code/graphics/2d.cpp gates the whole
// option definition on #ifdef WITH_VULKAN) -- in an OpenGL-only build there's nothing to choose between, so
// gr_get_configured_render_api() just returns GraphicsAPI::Default and no option is registered at all.
#ifdef WITH_VULKAN

TEST(OptionsManagerOverride, RenderAPIOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.RenderAPI");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<GraphicsAPI>*>(opt);

	// Exactly the payload shape built in cmdline.cpp's -vulkan handling.
	SCP_string override;
	sprintf(override, "%d", static_cast<int>(GraphicsAPI::Vulkan));
	OptionsManager::instance()->setOverride("Graphics.RenderAPI", override, "-vulkan");

	ASSERT_EQ(typedOpt->getValue(), GraphicsAPI::Vulkan);
	ASSERT_EQ(gr_get_configured_render_api(), GraphicsAPI::Vulkan);
}

TEST(OptionsManagerOverride, RenderAPIMenuInitPathDoesNotThrow)
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
		std::to_string(static_cast<int>(GraphicsAPI::OpenGL)), "-test");
	ASSERT_EQ(opt->getCurrentValueDescription().display, "OpenGL");

	OptionsManager::instance()->setOverride("Graphics.RenderAPI",
		std::to_string(static_cast<int>(GraphicsAPI::Vulkan)), "-vulkan");
	ASSERT_EQ(opt->getCurrentValueDescription().display, "Vulkan");
}

#else

TEST(OptionsManagerOverride, RenderAPIIsAbsentWithoutVulkanSupport)
{
	ASSERT_EQ(OptionsManager::instance()->getOptionByKey("Graphics.RenderAPI"), nullptr);
	ASSERT_EQ(gr_get_configured_render_api(), GraphicsAPI::Default);
}

#endif

TEST(OptionsManagerOverride, FlagsetOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.FramebufferEffects");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<flagset<FramebufferEffects>>*>(opt);

	// Exactly the payload shape built in cmdline.cpp's -fb_explosions/-fb_thrusters handling
	// (flagset::to_u64() as a plain decimal string). Bits 0 (Thrusters) and 1 (Shockwaves) both set == 3.
	OptionsManager::instance()->setOverride("Graphics.FramebufferEffects", "3", "-fb_explosions -fb_thrusters");

	auto value = typedOpt->getValue();
	ASSERT_TRUE(value[FramebufferEffects::Thrusters]);
	ASSERT_TRUE(value[FramebufferEffects::Shockwaves]);
}

// Graphics.Anisotropy is only registered in builds with the OpenGL backend compiled in
// (its Option<float> lives in gropengltexture.cpp, which the CMake build excludes entirely
// when FSO_BUILD_WITH_OPENGL is off) -- so, like Graphics.RenderAPI above, round-trip
// coverage needs a build-config split rather than a single unconditional test.
#ifdef WITH_OPENGL

TEST(OptionsManagerOverride, FloatOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Graphics.Anisotropy");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<float>*>(opt);

	// Exactly the payload shape built in cmdline.cpp's -anisotropic_filter handling.
	OptionsManager::instance()->setOverride("Graphics.Anisotropy", "16.0", "-anisotropic_filter");

	ASSERT_FLOAT_EQ(typedOpt->getValue(), 16.0f);
}

#else

TEST(OptionsManagerOverride, AnisotropyIsAbsentWithoutOpenGLSupport)
{
	ASSERT_EQ(OptionsManager::instance()->getOptionByKey("Graphics.Anisotropy"), nullptr);
}

#endif

TEST(OptionsManagerOverride, LanguageObjectOverrideRoundTrips)
{
	auto* opt = OptionsManager::instance()->getOptionByKey("Game.Language");
	ASSERT_NE(opt, nullptr);
	auto* typedOpt = static_cast<const Option<int>*>(opt);

	// Game.Language's real deserializer resolves a {"name","ext"} pair against Lcl_languages via
	// lcl_find_lang_index_by_name(), rather than storing a plain index -- so unlike the other round-trip
	// tests, this one needs at least one entry in that table to resolve against. A full lcl_init() is too
	// heavy for this test binary, so a single throwaway entry is pushed directly and restored afterwards,
	// since Lcl_languages is a shared global other tests may also touch.
	auto original_languages = Lcl_languages;

	lang_info test_lang{};
	strcpy_s(test_lang.lang_name, "OverrideRoundTripTestLang");
	strcpy_s(test_lang.lang_ext, "orttl");
	Lcl_languages.push_back(test_lang);
	int expected_idx = static_cast<int>(Lcl_languages.size()) - 1;

	// Exactly the payload shape built in localize.cpp's detect_lang() (matching language_serializer()'s
	// {"name":..,"ext":..} object).
	SCP_string override;
	sprintf(override, "{\"name\":\"%s\",\"ext\":\"%s\"}", test_lang.lang_name, test_lang.lang_ext);
	OptionsManager::instance()->setOverride("Game.Language", override, "-lang");

	ASSERT_EQ(typedOpt->getValue(), expected_idx);

	Lcl_languages = original_languages;
}

TEST(OptionsManagerOverride, LaterOverrideReplacesEarlierOne)
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
