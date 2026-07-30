#pragma once

#include <graphics/2d.h>
#include <graphics/shadows.h>

namespace fso::fred {

/**
 * @brief qtFRED's Preferences > Graphics settings.
 *
 * Single owner of these values: the QSettings keys, their defaults, and the rules for when each
 * one can be applied all live here. Two very different callers need them -- management.cpp before
 * gr_init(), and EditorViewport once the editor is up -- and previously each read QSettings with
 * its own copy of the key strings and default values.
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
	// one, the hardware maximum for the other. Both therefore carry a sentinel meaning "the user
	// has not chosen one", and applyBeforeGrInit() overrides only once there is a real choice.
	static constexpr int NO_TEXTURE_FILTER_CHOICE = -1;
	static constexpr float NO_ANISOTROPY_CHOICE = 0.0f;

	/**
	 * @brief Run the viewport through the game's HDR scene-texture + post-processing pipeline.
	 *
	 * Off by default so missions keep looking exactly as they do today unless a user opts in.
	 * Everything else on this struct is only visible in the viewport while this is on.
	 */
	bool enablePostProcessing = false;

	ShadowQuality shadowQuality = ShadowQuality::Disabled;
	AntiAliasMode aaMode = AntiAliasMode::None;

	int msaaSamples = 0; //!< 0 (off), 4, or 8; see validMsaaSampleCounts()
	float gamma = 3.0f;  //!< matches the brightness qtFRED has always launched with

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
