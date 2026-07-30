#include "lens_flare.h"
#include "lens_flare_internal.h"

#include "cfile/cfile.h"
#include "def_files/def_files.h"
#include "parse/parselo.h"

#include <utility>

// lens_flares.tbl / *-lens.tbm parsing. Owns no state: the systems it reads are
// appended to the vector the caller hands over, and each is precomputed on the
// way in so an unusable prescription never reaches the renderer.

namespace graphics {
namespace {

// Where the parsed lenses go for the duration of one parse run. File-scope
// pointers rather than parameters because parse_modular_table() takes a plain
// function pointer, so the per-file callback cannot capture anything. Set and
// cleared by lens_flare_parse_tables(), which is the only entry point.
SCP_vector<lens_system>* Parse_systems = nullptr;
SCP_string* Parse_default_name = nullptr;

int find_system(const SCP_vector<lens_system>& systems, const char* name)
{
	for (int i = 0; i < static_cast<int>(systems.size()); i++) {
		if (!stricmp(systems[i].name.c_str(), name)) {
			return i;
		}
	}
	return -1;
}

// ---- table parsing ----

void parse_lens_table_core()
{
	Assertion(Parse_systems != nullptr && Parse_default_name != nullptr,
		"Lens tables parsed outside lens_flare_parse_tables()!");
	auto& systems = *Parse_systems;
	auto& default_name = *Parse_default_name;

	reset_parse();

	required_string("#Lens Systems");

	// The lens a mission gets when it doesn't name one itself. Left empty by the
	// shipped table, so content that never asks for a lens keeps the retail look;
	// a mod opts its whole campaign in with one line in a *-lens.tbm.
	if (optional_string("$Default Lens:")) {
		stuff_string(default_name, F_NAME);
	}

	while (optional_string("$Name:")) {
		lens_system ls;
		stuff_string(ls.name, F_NAME);

		if (optional_string("$Entrance Pupil Radius:")) {
			stuff_float(&ls.entrance_radius);
		}
		if (optional_string("$Aperture Radius:")) {
			stuff_float(&ls.aperture_radius);
		}
		if (optional_string("$Sensor Width:")) {
			stuff_float(&ls.sensor_width);
		}
		if (optional_string("$Anamorphic Squeeze:")) {
			stuff_float(&ls.anamorphic_squeeze);
		}
		if (optional_string("$Anamorphic Streak:")) {
			stuff_float(&ls.streak.strength);
			if (optional_string("+Length:")) {
				stuff_float(&ls.streak.length);
			}
			if (optional_string("+Thickness:")) {
				stuff_float(&ls.streak.thickness);
			}
			if (optional_string("+Tint:")) {
				float rgb[3] = {1.0f, 1.0f, 1.0f};
				size_t count = stuff_float_list(rgb, 3);
				if (count != 3) {
					error_display(0, "Lens system '%s': +Tint: needs ( r, g, b )", ls.name.c_str());
				}
				ls.streak.tint[0] = rgb[0];
				ls.streak.tint[1] = rgb[1];
				ls.streak.tint[2] = rgb[2];
			}
		}
		if (optional_string("$Coating Wavelength:")) {
			stuff_float(&ls.coating_wavelength);
		}
		if (optional_string("$Aperture Blades:")) {
			stuff_int(&ls.aperture.blades);
		}
		if (optional_string("+Blade Rotation:")) {
			stuff_float(&ls.aperture.rotation);
		}
		if (optional_string("+Blade Curvature:")) {
			stuff_float(&ls.aperture.curvature);
		}
		if (optional_string("+Edge Softness:")) {
			stuff_float(&ls.aperture.softness);
		}
		if (optional_string("$Aperture Grating:")) {
			stuff_float(&ls.aperture.grating.strength);
			if (optional_string("+Density:")) {
				stuff_float(&ls.aperture.grating.density);
			}
			if (optional_string("+Length:")) {
				stuff_float(&ls.aperture.grating.length);
			}
			if (optional_string("+Width:")) {
				stuff_float(&ls.aperture.grating.width);
			}
			if (optional_string("+Softness:")) {
				stuff_float(&ls.aperture.grating.softness);
			}
		}
		if (optional_string("$Aperture Scratches:")) {
			stuff_float(&ls.aperture.scratches.strength);
			if (optional_string("+Density:")) {
				stuff_float(&ls.aperture.scratches.density);
			}
			if (optional_string("+Length:")) {
				stuff_float(&ls.aperture.scratches.length);
			}
			if (optional_string("+Width:")) {
				stuff_float(&ls.aperture.scratches.width);
			}
			if (optional_string("+Rotation:")) {
				stuff_float(&ls.aperture.scratches.rotation);
			}
			if (optional_string("+Rotation Variation:")) {
				stuff_float(&ls.aperture.scratches.rotation_variation);
			}
			if (optional_string("+Softness:")) {
				stuff_float(&ls.aperture.scratches.softness);
			}
		}
		if (optional_string("$Aperture Dust:")) {
			stuff_float(&ls.aperture.dust.strength);
			if (optional_string("+Density:")) {
				stuff_float(&ls.aperture.dust.density);
			}
			if (optional_string("+Radius:")) {
				stuff_float(&ls.aperture.dust.radius);
			}
			if (optional_string("+Softness:")) {
				stuff_float(&ls.aperture.dust.softness);
			}
		}
		if (optional_string("$Starburst:")) {
			stuff_boolean(&ls.starburst);
		}
		if (optional_string("+Starburst Scale:")) {
			stuff_float(&ls.starburst_scale);
		}
		if (optional_string("$Intensity:")) {
			stuff_float(&ls.intensity);
		}
		if (optional_string("$Max Ghosts:")) {
			stuff_int(&ls.max_ghosts);
		}

		while (true) {
			if (optional_string("$Surface:")) {
				float vals[3] = {0.0f, 0.0f, 1.0f};
				size_t count = stuff_float_list(vals, 3);
				if (count != 3) {
					error_display(0, "Lens system '%s': $Surface: needs ( radius, thickness, index )", ls.name.c_str());
				}
				lens_surface s;
				s.radius = vals[0];
				s.thickness = vals[1];
				s.n = vals[2];
				if (optional_string("+Abbe:")) {
					stuff_float(&s.abbe);
				}
				if (optional_string("+Coating Wavelength:")) {
					stuff_float(&s.coating_wavelength);
				}
				ls.surfaces.push_back(s);
			} else if (optional_string("$Stop:")) {
				float d = 0.0f;
				size_t count = stuff_float_list(&d, 1);
				if (count != 1) {
					error_display(0, "Lens system '%s': $Stop: needs ( thickness )", ls.name.c_str());
				}
				lens_surface s;
				s.thickness = d;
				s.is_stop = true;
				ls.surfaces.push_back(s);
			} else {
				break;
			}
		}

		if (!lens_flare_precompute(ls)) {
			error_display(0, "Lens system '%s' has an unusable prescription and will be ignored", ls.name.c_str());
			continue;
		}

		// what a mission's sexps (or the lab) will be reset back to
		ls.tabled_aperture = ls.aperture;

		// a later table may redefine a lens the engine (or an earlier tbm) shipped
		int existing = find_system(systems, ls.name.c_str());
		if (existing >= 0) {
			systems[existing] = std::move(ls);
		} else {
			systems.push_back(std::move(ls));
		}
	}

	required_string("#End");
}

void parse_lens_table_file(const char* filename)
{
	try {
		if (filename == nullptr) {
			read_file_text_from_default(defaults_get_file("lens_flares.tbl"));
		} else {
			read_file_text(filename, CF_TYPE_TABLES);
		}
		parse_lens_table_core();
	} catch (const parse::ParseException& e) {
		mprintf(("Unable to parse '%s'!  Error message = %s.\n", (filename != nullptr) ? filename : "<default lens_flares.tbl>", e.what()));
	}
}

} // namespace

void lens_flare_parse_tables(SCP_vector<lens_system>& systems, SCP_string& default_lens_name)
{
	Parse_systems = &systems;
	Parse_default_name = &default_lens_name;

	if (cf_exists_full("lens_flares.tbl", CF_TYPE_TABLES)) {
		parse_lens_table_file("lens_flares.tbl");
	} else {
		parse_lens_table_file(nullptr);
	}

	parse_modular_table("*-lens.tbm", [](const char* filename) { parse_lens_table_file(filename); });

	Parse_systems = nullptr;
	Parse_default_name = nullptr;
}

} // namespace graphics
