#include "graphics/texture_material.h"

#include "cfile/cfile.h"
#include "lighting/lighting_profiles.h"
#include "parse/parselo.h"

#include <cmath>

namespace {

// Misses are stored as nullopt so each name is read once; node-based, so bitmap entries can keep pointers into it
SCP_unordered_map<SCP_string, std::optional<texture_material>, SCP_string_lcase_hash, SCP_string_lcase_equal_to>
	Texture_materials;

constexpr int MAX_MAT_FILE_LEN = 1024 * 1024;

void mat_warning(const SCP_string& text)
{
	Warning(LOCATION, "%s", text.c_str());
}

bool at_line_end()
{
	ignore_gray_space();
	return *Mp == '\0' || *Mp == EOLN || *Mp == CARRIAGE_RETURN || *Mp == ';';
}

std::optional<float> parse_glow_value(const char* filename, const char* key, SCP_vector<SCP_string>& warnings)
{
	ignore_gray_space();
	const char* start = Mp;
	while (*Mp != '\0' && !is_white_space(*Mp) && *Mp != ';')
		Mp++;
	const SCP_string text(start, Mp - start);

	char* end = nullptr;
	const float value = strtof(text.c_str(), &end);
	if (text.empty() || *end != '\0' || !std::isfinite(value)) {
		warnings.push_back(SCP_string(filename) + ": " + key + " needs a number, not '" + text + "'.  Ignoring it.");
		return std::nullopt;
	}
	if (!at_line_end())
		warnings.push_back(SCP_string(filename) + ": extra text after " + key + " " + text + ".  Ignoring the extra text.");

	if (value < 0.0f) {
		warnings.push_back(SCP_string(filename) + ": " + key + " is negative; using 0.");
		return 0.0f;
	}
	if (value > lighting_profiles::MAX_GLOW_MAP_VALUE) {
		warnings.push_back(SCP_string(filename) + ": " + key + " is above " +
					std::to_string(static_cast<int>(lighting_profiles::MAX_GLOW_MAP_VALUE)) + "; using that.");
		return lighting_profiles::MAX_GLOW_MAP_VALUE;
	}
	return value;
}

void parse_texture_material(const char* filename, char* text, texture_material& mat, SCP_vector<SCP_string>& warnings)
{
	reset_parse(text);

	int unknown = 0;
	// each pass consumes at least one character, so the file length bounds the loop
	for (int pass = 0; !check_for_eof(); pass++) {
		if (pass > MAX_MAT_FILE_LEN) {
			warnings.push_back(SCP_string(filename) + ": stopped reading after " + std::to_string(pass) + " entries.");
			break;
		}

		if (*Mp == ';') {
			advance_to_eoln(nullptr);
			continue;
		}

		if (optional_string("$Glow Map Intensity:")) {
			mat.glow.intensity = parse_glow_value(filename, "$Glow Map Intensity:", warnings);
		} else if (optional_string("$Glow Map HDR Multiplier:")) {
			mat.glow.hdr_multiplier = parse_glow_value(filename, "$Glow Map HDR Multiplier:", warnings);
		} else if (unknown++ == 0) {
			warnings.push_back(SCP_string(filename) + ": unrecognized entry [" + next_tokens() + "].  Ignoring it.");
		}
		advance_to_eoln(nullptr);
	}

	if (unknown > 1)
		warnings.push_back(SCP_string(filename) + ": " + std::to_string(unknown - 1) + " more unrecognized entries ignored.");
}

// Read like an .eff file, so .mat files follow the same encoding rules
bool read_texture_material(const char* filename, int dir_type, texture_material& mat)
{
	CFILE* cfp = cfopen(filename, "rb", dir_type);
	if (cfp == nullptr)
		return false;
	const int file_len = cfilelength(cfp);
	cfclose(cfp);

	if (file_len <= 0 || file_len > MAX_MAT_FILE_LEN) {
		mat_warning(SCP_string(filename) + " is empty or larger than " + std::to_string(MAX_MAT_FILE_LEN) +
					" bytes.  Ignoring it.");
		return false;
	}

	// encrypted text unpacks 7 bytes into 8, and processing can turn one character into two
	const size_t raw_len = static_cast<size_t>(file_len) + file_len / 7 + 16;
	SCP_vector<char> text_raw(raw_len, '\0');
	SCP_vector<char> text(raw_len * 2, '\0');

	// our own warnings wait until the outer parse is restored, since a debug build can exit from the warning dialog
	SCP_vector<SCP_string> warnings;
	bool ok = true;
	pause_parse();
	try {
		read_file_text(filename, dir_type, text.data(), text_raw.data());
		parse_texture_material(filename, text.data(), mat, warnings);
	} catch (const parse::ParseException& e) {
		mprintf(("TEXTURE MATERIAL: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		ok = false;
	} catch (...) {
		unpause_parse();
		throw;
	}
	unpause_parse();

	for (const auto& warning : warnings)
		mat_warning(warning);

	return ok;
}

} // namespace

const texture_material* texture_material_find(const char* texture_filename, int dir_type)
{
	if (texture_filename == nullptr || *texture_filename == '\0')
		return nullptr;

	SCP_string name = texture_filename;
	const auto dot = name.rfind('.');
	if (dot != SCP_string::npos)
		name.erase(dot);
	name += ".mat";

	const auto key = name + '|' + std::to_string(dir_type);
	const auto found = Texture_materials.find(key);
	if (found != Texture_materials.end())
		return found->second ? &*found->second : nullptr;

	auto& entry = Texture_materials[key];
	if (!cf_exists_full(name.c_str(), dir_type))
		return nullptr;

	texture_material mat;
	if (!read_texture_material(name.c_str(), dir_type, mat))
		return nullptr;

	mprintf(("TEXTURE MATERIAL: Using '%s'\n", name.c_str()));
	entry = mat;
	return &*entry;
}

void texture_material_reset()
{
	Texture_materials.clear();
}

float texture_glow_intensity(const texture_glow_settings& glow)
{
	return glow.intensity.value_or(lighting_profiles::current_glow_map_intensity());
}

float texture_glow_hdr_multiplier(const texture_glow_settings& glow)
{
	return glow.hdr_multiplier.value_or(
		lighting_profiles::current_glow_map_hdr_multiplier(texture_glow_intensity(glow)));
}
