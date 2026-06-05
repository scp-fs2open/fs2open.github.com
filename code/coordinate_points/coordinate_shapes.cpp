#include "coordinate_points/coordinate_shapes.h"

#include "cfile/cfile.h"
#include "def_files/def_files.h"
#include "parse/parselo.h"

SCP_vector<coordinate_shape_def> Coordinate_shapes;

namespace {

constexpr const char* DEFAULT_TABLE_NAME = "coordinate_points.tbl";
constexpr const char* MODULAR_TABLE_GLOB = "*-cps.tbm";

// Reserved built-in shape kind keywords. Tabled entries cannot use these names.
bool is_reserved_shape_name(const char* name)
{
	return !stricmp(name, "NGon") || !stricmp(name, "Star");
}

bool shape_name_exists(const char* name)
{
	for (const auto& s : Coordinate_shapes) {
		if (!stricmp(s.name.c_str(), name)) {
			return true;
		}
	}
	return false;
}

void parse_coordinate_points_table(const char* filename)
{
	try {
		if (filename != nullptr) {
			read_file_text(filename, CF_TYPE_TABLES);
		} else {
			read_file_text_from_default(defaults_get_file(DEFAULT_TABLE_NAME));
		}
		reset_parse();

		required_string("#Coordinate Shapes");
		while (optional_string("$Name:")) {
			// optional_string consumed the keyword; read the name directly here rather than
			// rewinding for a helper.
			SCP_string name;
			stuff_string(name, F_NAME);

			bool skip = false;
			if (is_reserved_shape_name(name.c_str())) {
				Warning(LOCATION,
					"Coordinate shape name '%s' is reserved (built-in kind); entry skipped.",
					name.c_str());
				skip = true;
			} else if (shape_name_exists(name.c_str())) {
				Warning(LOCATION,
					"Coordinate shape '%s' is already defined; duplicate entry skipped.",
					name.c_str());
				skip = true;
			}

			SCP_vector<float> raw_verts;
			SCP_vector<int>   raw_tris;

			required_string("$Vertices:");
			stuff_float_list(raw_verts);

			required_string("$Triangles:");
			stuff_int_list(raw_tris);

			if (skip) {
				continue;
			}

			if (raw_verts.empty() || (raw_verts.size() % 2) != 0) {
				Warning(LOCATION,
					"Coordinate shape '%s' has %d vertex floats; must be a non-zero even count (x y pairs). Entry skipped.",
					name.c_str(), static_cast<int>(raw_verts.size()));
				continue;
			}
			if (raw_tris.empty() || (raw_tris.size() % 3) != 0) {
				Warning(LOCATION,
					"Coordinate shape '%s' has %d triangle indices; must be a non-zero multiple of 3. Entry skipped.",
					name.c_str(), static_cast<int>(raw_tris.size()));
				continue;
			}

			const int vert_count = static_cast<int>(raw_verts.size() / 2);
			bool indices_valid = true;
			for (int idx : raw_tris) {
				if (idx < 0 || idx >= vert_count) {
					Warning(LOCATION,
						"Coordinate shape '%s' triangle index %d is out of range [0, %d). Entry skipped.",
						name.c_str(), idx, vert_count);
					indices_valid = false;
					break;
				}
			}
			if (!indices_valid) {
				continue;
			}

			coordinate_shape_def def;
			def.name = std::move(name);
			def.verts.reserve(vert_count);
			for (size_t i = 0; i < raw_verts.size(); i += 2) {
				def.verts.push_back({raw_verts[i], raw_verts[i + 1]});
			}
			def.tri_indices = std::move(raw_tris);

			Coordinate_shapes.push_back(std::move(def));
		}
		required_string("#End");
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'! Error message = %s.\n",
			filename ? filename : DEFAULT_TABLE_NAME, e.what()));
	}
}

} // anonymous namespace

void coordinate_shapes_init()
{
	Coordinate_shapes.clear();

	// Default table first so its names take precedence over any colliding mod entries.
	if (cf_exists_full(DEFAULT_TABLE_NAME, CF_TYPE_TABLES)) {
		parse_coordinate_points_table(DEFAULT_TABLE_NAME);
	} else {
		parse_coordinate_points_table(nullptr);  // compiled-in default
	}

	parse_modular_table(MODULAR_TABLE_GLOB, parse_coordinate_points_table);
}

int find_coordinate_shape_index_by_name(const char* name)
{
	if (name == nullptr) {
		return -1;
	}
	for (size_t i = 0; i < Coordinate_shapes.size(); ++i) {
		if (!stricmp(Coordinate_shapes[i].name.c_str(), name)) {
			return static_cast<int>(i);
		}
	}
	return -1;
}
