#pragma once

#include <optional>

struct texture_glow_settings {
	std::optional<float> intensity;
	std::optional<float> hdr_multiplier;
};

struct texture_material {
	texture_glow_settings glow;
};

/**
 * @brief Finds and parses the .mat file that belongs to a texture
 *
 * Each name is read once per session, so edits to a .mat file take effect after a restart.
 *
 * @param texture_filename The texture's filename; any extension is ignored
 * @param dir_type The CF_TYPE_ the texture was requested from
 *
 * @returns The parsed settings, valid until texture_material_reset(), or nullptr if the texture has no usable .mat file
 */
const texture_material* texture_material_find(const char* texture_filename, int dir_type);

/**
 * @brief Forgets every .mat file read so far
 *
 * Only for when no bitmap is loaded, such as between unit tests, since loaded bitmaps point into this storage.
 */
void texture_material_reset();

// A texture's glow values, falling back to the current lighting profile for any it doesn't set
float texture_glow_intensity(const texture_glow_settings& glow);
float texture_glow_hdr_multiplier(const texture_glow_settings& glow);
