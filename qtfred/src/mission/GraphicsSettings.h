#pragma once

#include <graphics/2d.h>
#include <graphics/shadows.h>

namespace fso::fred {

/**
 * @brief qtFRED's Preferences > Graphics settings.
 *
 * Single owner of the *rules*: the QSettings keys, the defaults, and when each value can be applied
 * all live here. Two very different callers need them -- management.cpp before gr_init(), and
 * EditorViewport once the editor is up -- and previously each read QSettings with its own copy of
 * the key strings and default values.
 *
 * The values themselves are ordinary fields and have more than one writer. Preferences edits a copy
 * and writes the whole struct back on apply (PreferencesDialogModel); View > Enable Post Processing
 * flips enablePostProcessing in place (FredView::syncViewOptions). Those cannot race -- the
 * Preferences dialog is modal, and the menu action re-reads the field on every viewIdle so its
 * check state follows an edit made in the dialog. A third writer, or a modeless Preferences, would
 * break that; route it through here rather than adding another direct one.
 *
 * The settings split by *when* they can take effect, which is what the two apply functions below
 * encode:
 *
 *  - applyLive() settings can be changed at any time and are re-applied whenever the user hits
 *    Apply in Preferences.
 *  - applyBeforeGrInit() settings are baked into GPU resources during gr_init() and cannot be
 *    changed afterwards, so they are read straight out of QSettings before the editor exists.
 *    Shadow quality is the sharpest example: shadow_cascade_params_init() only runs during
 *    gr_init(), and only if Shadow_quality is already non-Disabled, so setting it later leaves the
 *    cascade buffers unsized and crashes the next frame that renders shadows.
 */
struct GraphicsSettings {
	// Texture filtering and anisotropy reach the engine through the options system, whose defaults
	// are better informed than anything qtFRED could hardcode -- the user's existing config for
	// one, the hardware maximum for the other. The backend has a comparable existing default: the
	// VideocardFs2open config entry gr_init() already reads. All three therefore carry a sentinel
	// meaning "the user has not chosen one", and applyBeforeGrInit() overrides only once there is a
	// real choice.
	static constexpr int NO_TEXTURE_FILTER_CHOICE = -1;
	static constexpr float NO_ANISOTROPY_CHOICE = 0.0f;

	/**
	 * @brief Run the viewport through the game's HDR scene-texture + post-processing pipeline.
	 *
	 * Off by default so missions keep looking exactly as they do today unless a user opts in.
	 * Everything else on this struct is only visible in the viewport while this is on.
	 */
	bool enablePostProcessing = false;

	/**
	 * @brief The renderer backend, applied before gr_init() the same way Shadow_quality is.
	 *
	 * GraphicsAPI::Default means "the user has not chosen one in Preferences" -- gr_init() already
	 * has a fallback for that (the VideocardFs2open config entry, then OpenGL), and qtFRED must not
	 * get in front of it: a user relying on that entry to launch qtFRED into Vulkan without passing
	 * -vulkan every time would otherwise lose that the moment they saved any other preference, since
	 * save() would start persisting an explicit OpenGL nobody chose. So this is only ever written or
	 * applied once Preferences actually sets it to OpenGL or Vulkan -- see save() and
	 * applyBeforeGrInit(). `-vulkan`/`-opengl` on the command line still win over an explicit choice
	 * here too: applyBeforeGrInit() only sets Cmdline_graphics_api from this when parse_cmdline() left
	 * it at GraphicsAPI::Default, matching the "commandline always wins" rule gr_init() enforces.
	 * load() also refuses to hand back GraphicsAPI::Vulkan on a build without WITH_VULKAN, so a
	 * settings file left over from a Vulkan-enabled build can't make a later build without it hit the
	 * Error() in gr_init_function_pointers().
	 */
	GraphicsAPI backend = GraphicsAPI::Default;

	ShadowQuality shadowQuality = ShadowQuality::Disabled;

	/**
	 * @brief Whether shadows are cast via cascaded shadow maps or hardware raytracing.
	 *
	 * A live-apply setting: unlike ShadowQuality, the engine's own Graphics.ShadowRenderMethod
	 * option just binds straight to Shadow_render_method with no initial-only change_listener,
	 * so applyLive() can set it directly, same as aaMode.
	 *
	 * Raytraced only actually renders that way when shadows_use_raytracing() agrees, which
	 * requires shadows_raytracing_supported() (Vulkan plus hardware ray-query support) --
	 * exactly the same check Graphics.ShadowRenderMethod's own enumerator uses to decide
	 * whether to offer the value at all. So this field can hold Raytraced on a build/session
	 * that can't honour it with no effect on rendering, same as the engine option it mirrors.
	 * Preferences populates its combo from that option (see PreferencesDialog::initializeUi()),
	 * so it reads as greyed out whenever the choice would be inert, rather than qtFRED needing
	 * its own Vulkan/hardware check.
	 *
	 * Graphics.RTShadowQuality (Low vs. High local-light coverage) is deliberately not exposed
	 * here -- not useful enough in the editor to be worth a second control.
	 */
	ShadowRenderMethod shadowMethod = ShadowRenderMethod::ShadowMap;

	AntiAliasMode aaMode = AntiAliasMode::None;

	int msaaSamples = 0; //!< 0 (off), 4, or 8; see validMsaaSampleCounts()

	/**
	 * @brief Viewport brightness, as the exponent in pow(colour, 1/gamma).
	 *
	 * 1.0 (identity, and the engine's own default) is what qtFRED has always *rendered* with,
	 * whatever this said. It used to default to 3.0, copied verbatim from retail FRED2
	 * (fred2/management.cpp still has the same gr_set_gamma(3.0f)), where it has never had any
	 * effect: OpenGL only applies Gr_gamma inside the `if (Cmdline_window_res)` branch of
	 * gr_opengl_flip(), and gr_init() deliberately does not set Cmdline_window_res for FRED.
	 *
	 * The Vulkan backend has no such branch -- encodeToSwapChain() always runs the gamma encode --
	 * so a 3.0 default made the Vulkan viewport render every frame through pow(colour, 1/3) while
	 * OpenGL rendered it untouched. Keeping the identity here is what makes the two backends agree.
	 *
	 * Note the control is still inert under OpenGL in the editor; see qtfred/README.md.
	 */
	float gamma = 1.0f;

	int textureFilter = NO_TEXTURE_FILTER_CHOICE; //!< 0 = bilinear, 1 = trilinear
	float anisotropy = NO_ANISOTROPY_CHOICE;      //!< 1.0 = off, otherwise a power of two

	bool operator==(const GraphicsSettings& rhs) const;
	bool operator!=(const GraphicsSettings& rhs) const { return !(*this == rhs); }

	/**
	 * @brief The MSAA sample counts the Preferences combo offers, in combo order.
	 */
	static SCP_vector<int> validMsaaSampleCounts();

	static GraphicsSettings load();
	void save() const;

	/**
	 * @brief Push the settings that can be changed without a restart into the engine.
	 */
	void applyLive() const;

	/**
	 * @brief Load and apply the settings that must be in place before gr_init() runs.
	 *
	 * The engine reads texture filtering and anisotropy through the options system, so those are
	 * pushed as in-memory-only overrides rather than written out -- qtFRED must never rewrite the
	 * config file the game reads its own graphics settings from.
	 *
	 * @return the settings that were loaded, so the caller can applyLive() them once gr_init() has
	 *         returned and there is a renderer to apply them to.
	 */
	static GraphicsSettings applyBeforeGrInit();
};

} // namespace fso::fred
