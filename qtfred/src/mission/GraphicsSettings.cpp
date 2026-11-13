#include "mission/GraphicsSettings.h"

#include <cmdline/cmdline.h>
#include <mod_table/mod_table.h>
#include <options/OptionsManager.h>

#include <QSettings>

namespace fso::fred {

namespace {

const char* SETTINGS_GROUP = "Preferences";

const char* KEY_POST_PROCESSING = "view_enable_post_processing";
const char* KEY_SHADOW_QUALITY = "view_graphics_shadow_quality";
const char* KEY_SHADOW_CONTACT_HARDENING = "view_graphics_shadow_contact_hardening";
const char* KEY_AA_MODE = "view_graphics_aa_mode";
const char* KEY_TEXTURE_FILTER = "view_graphics_texture_filter";
const char* KEY_ANISOTROPY = "view_graphics_anisotropy";
const char* KEY_GAMMA = "view_graphics_gamma";

// A settings file can be hand-edited or left over from a build with a different set of values, so
// anything that ends up in an enum or a fixed value list gets checked rather than cast blindly.
template <typename T>
T readEnum(const QSettings& settings, const char* key, T fallback, T highest)
{
	const int raw = settings.value(key, static_cast<int>(fallback)).toInt();

	if (raw < 0 || raw > static_cast<int>(highest)) {
		return fallback;
	}

	return static_cast<T>(raw);
}

} // namespace

bool GraphicsSettings::operator==(const GraphicsSettings& rhs) const
{
	return enablePostProcessing == rhs.enablePostProcessing && shadowQuality == rhs.shadowQuality &&
	       shadowContactHardening == rhs.shadowContactHardening &&
	       aaMode == rhs.aaMode && gamma == rhs.gamma &&
	       textureFilter == rhs.textureFilter && anisotropy == rhs.anisotropy;
}

GraphicsSettings GraphicsSettings::load()
{
	GraphicsSettings out;

	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);

	out.enablePostProcessing = settings.value(KEY_POST_PROCESSING, out.enablePostProcessing).toBool();
	out.shadowQuality = readEnum(settings, KEY_SHADOW_QUALITY, out.shadowQuality, ShadowQuality::Ultra);
	out.shadowContactHardening =
		settings.value(KEY_SHADOW_CONTACT_HARDENING, out.shadowContactHardening).toBool();
	out.aaMode = readEnum(settings, KEY_AA_MODE, out.aaMode, AntiAliasMode::SMAA_Ultra);
	out.gamma = settings.value(KEY_GAMMA, out.gamma).toFloat();

	if (settings.contains(KEY_TEXTURE_FILTER)) {
		out.textureFilter = settings.value(KEY_TEXTURE_FILTER).toInt() != 0 ? 1 : 0;
	}

	if (settings.contains(KEY_ANISOTROPY)) {
		out.anisotropy = settings.value(KEY_ANISOTROPY).toFloat();
	}

	settings.endGroup();

	return out;
}

void GraphicsSettings::save() const
{
	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);

	settings.setValue(KEY_POST_PROCESSING, enablePostProcessing);
	settings.setValue(KEY_SHADOW_QUALITY, static_cast<int>(shadowQuality));
	settings.setValue(KEY_SHADOW_CONTACT_HARDENING, shadowContactHardening);
	settings.setValue(KEY_AA_MODE, static_cast<int>(aaMode));
	settings.setValue(KEY_GAMMA, gamma);

	// Don't write the sentinels back out -- an absent key is what keeps the engine default in play.
	if (textureFilter != NO_TEXTURE_FILTER_CHOICE) {
		settings.setValue(KEY_TEXTURE_FILTER, textureFilter);
	}

	if (anisotropy != NO_ANISOTROPY_CHOICE) {
		settings.setValue(KEY_ANISOTROPY, anisotropy);
	}

	settings.endGroup();
}

void GraphicsSettings::applyLive() const
{
	Gr_aa_mode = aaMode;

	// qtFRED does not load the engine options, so nothing else writes this global. The shadow
	// code reads it on each frame, thus the change applies immediately. If the GPU cannot do
	// contact hardening, shadow_contact_hardening_enabled() ignores this value.
	Shadow_contact_hardening_enabled = shadowContactHardening;
	gr_set_gamma(gamma);
}

GraphicsSettings GraphicsSettings::applyBeforeGrInit()
{
	const GraphicsSettings settings = load();

	Shadow_quality = settings.shadowQuality;

	auto* options = options::OptionsManager::instance();

	// Overrides are in-memory only, so they steer this session's gr_init() without touching the
	// config file the game reads its own graphics settings from.
	if (settings.textureFilter != NO_TEXTURE_FILTER_CHOICE) {
		options->setOverride("Graphics.TextureFilter", std::to_string(settings.textureFilter), "qtFRED Preferences");

		// When a mod turns off the in-game options, the engine does not read the override. It
		// reads Gr_texture_filter_override, so set that value also. It is in memory only.
		if (!Using_in_game_options) {
			Gr_texture_filter_override = settings.textureFilter;
		}
	}

	if (settings.anisotropy != NO_ANISOTROPY_CHOICE) {
		options->setOverride("Graphics.Anisotropy", std::to_string(settings.anisotropy), "qtFRED Preferences");

		// When a mod turns off the in-game options, the engine does not read the override. It
		// reads the command-line level, so set that value also. It is in memory only.
		if (!Using_in_game_options) {
			Cmdline_aniso_level = static_cast<int>(settings.anisotropy);
		}
	}

	return settings;
}

} // namespace fso::fred
