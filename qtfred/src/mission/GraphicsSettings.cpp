#include "mission/GraphicsSettings.h"

#include <cmdline/cmdline.h>
#include <options/OptionsManager.h>

#include <QSettings>

namespace fso::fred {

namespace {

const char* SETTINGS_GROUP = "Preferences";

const char* KEY_BACKEND = "view_graphics_backend";
const char* KEY_POST_PROCESSING = "view_enable_post_processing";
const char* KEY_SHADOW_QUALITY = "view_graphics_shadow_quality";
const char* KEY_SHADOW_METHOD = "view_graphics_shadow_method";
const char* KEY_AA_MODE = "view_graphics_aa_mode";
const char* KEY_MSAA_SAMPLES = "view_graphics_msaa_samples";
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
	return backend == rhs.backend && enablePostProcessing == rhs.enablePostProcessing &&
	       shadowQuality == rhs.shadowQuality && shadowMethod == rhs.shadowMethod && aaMode == rhs.aaMode &&
	       msaaSamples == rhs.msaaSamples && gamma == rhs.gamma && textureFilter == rhs.textureFilter &&
	       anisotropy == rhs.anisotropy;
}

SCP_vector<int> GraphicsSettings::validMsaaSampleCounts()
{
	return {0, 4, 8};
}

GraphicsSettings GraphicsSettings::load()
{
	GraphicsSettings out;

	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);

	if (settings.contains(KEY_BACKEND)) {
		// Only OpenGL and, on a build that has it, Vulkan are valid explicit choices. Stub, a Vulkan
		// choice this build can't honour, or a garbage value from a hand-edited file all keep
		// GraphicsAPI::Default, same as an absent key -- see the field comment for why that matters.
		const auto raw = static_cast<GraphicsAPI>(settings.value(KEY_BACKEND).toInt());
		if (raw == GraphicsAPI::OpenGL) {
			out.backend = GraphicsAPI::OpenGL;
		}
#ifdef WITH_VULKAN
		else if (raw == GraphicsAPI::Vulkan) {
			out.backend = GraphicsAPI::Vulkan;
		}
#endif
	}

	out.enablePostProcessing = settings.value(KEY_POST_PROCESSING, out.enablePostProcessing).toBool();
	out.shadowQuality = readEnum(settings, KEY_SHADOW_QUALITY, out.shadowQuality, ShadowQuality::Ultra);
	out.shadowMethod =
		readEnum(settings, KEY_SHADOW_METHOD, out.shadowMethod, ShadowRenderMethod::Raytraced);
	out.aaMode = readEnum(settings, KEY_AA_MODE, out.aaMode, AntiAliasMode::SMAA_Ultra);
	out.gamma = settings.value(KEY_GAMMA, out.gamma).toFloat();

	if (settings.contains(KEY_TEXTURE_FILTER)) {
		out.textureFilter = settings.value(KEY_TEXTURE_FILTER).toInt() != 0 ? 1 : 0;
	}

	if (settings.contains(KEY_ANISOTROPY)) {
		out.anisotropy = settings.value(KEY_ANISOTROPY).toFloat();
	}

	const int samples = settings.value(KEY_MSAA_SAMPLES, out.msaaSamples).toInt();
	const auto validSamples = validMsaaSampleCounts();
	if (std::find(validSamples.begin(), validSamples.end(), samples) != validSamples.end()) {
		out.msaaSamples = samples;
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
	settings.setValue(KEY_SHADOW_METHOD, static_cast<int>(shadowMethod));
	settings.setValue(KEY_AA_MODE, static_cast<int>(aaMode));
	settings.setValue(KEY_MSAA_SAMPLES, msaaSamples);
	settings.setValue(KEY_GAMMA, gamma);

	// Don't write the sentinels back out -- an absent key is what keeps the engine default in play.
	if (backend != GraphicsAPI::Default) {
		settings.setValue(KEY_BACKEND, static_cast<int>(backend));
	}

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
	gr_set_gamma(gamma);

	// Same as Gr_aa_mode: this is what Graphics.ShadowRenderMethod's own .bind_to() does, and it is
	// harmless to set even when raytraced shadows are not actually available this session --
	// shadows_use_raytracing() re-checks shadows_raytracing_supported() before anything reads it.
	Shadow_render_method = shadowMethod;
}

GraphicsSettings GraphicsSettings::applyBeforeGrInit()
{
	const GraphicsSettings settings = load();

	// Commandline always wins (see gr_init()'s own "Commandline ALWAYS wins" comment), so only fill
	// in the backend here if -vulkan/-opengl didn't already set it. If the user has never chosen one
	// in Preferences either, settings.backend is itself GraphicsAPI::Default, and this is a no-op --
	// gr_init() falls through to its own VideocardFs2open/OpenGL fallback exactly as it always has.
	if (Cmdline_graphics_api == GraphicsAPI::Default) {
		Cmdline_graphics_api = settings.backend;
	}

	Cmdline_msaa_enabled = settings.msaaSamples;
	Shadow_quality = settings.shadowQuality;

	auto* options = options::OptionsManager::instance();

	// Overrides are in-memory only, so they steer this session's gr_init() without touching the
	// config file the game reads its own graphics settings from.
	if (settings.textureFilter != NO_TEXTURE_FILTER_CHOICE) {
		options->setOverride("Graphics.TextureFilter", std::to_string(settings.textureFilter));
	}

	if (settings.anisotropy != NO_ANISOTROPY_CHOICE) {
		options->setOverride("Graphics.Anisotropy", std::to_string(settings.anisotropy));
	}

	return settings;
}

} // namespace fso::fred
