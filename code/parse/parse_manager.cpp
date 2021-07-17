#pragma

#ifdef WIN32
#include <direct.h>
#endif

#include "globalincs/pstypes.h"
#include "globalincs/version.h"
#include "osapi/osapi.h"
#include "cfile/cfilesystem.h"
#include "parse_manager.h"
#include "asteroid/asteroid_info.h"
#include "ai/ai_class_info.h"
#include "ship/ship_info.h"


void parse_item::reset_fields()
{
	_table_string   = "FIX ME!";
	_description    = "Someone didn't initialize me correctly. Go get a coder.";

	_type   = Parse_Input_Types::USAGE_NOTE;

	_required		= false;
	_enforced_count = NO_ENFORCED_COUNT;
	_major_version_number_implemented = INVALID_VERSION;
	_minor_version_number_implemented = INVALID_VERSION;
	_revision_number_implemented	  = INVALID_VERSION;
	_major_version_number_deprecated  = INVALID_VERSION;
	_minor_version_number_deprecated  = INVALID_VERSION;
	_revision_number_deprecated		  = INVALID_VERSION;
	_deprecation_message			  = "";

}

SCP_string parse_item::get_table_string()
{
	return _table_string;
}

SCP_string parse_item::get_description()
{
	return _description;
}

Parse_Input_Types parse_item::get_type()
{
	return _type;
}

bool parse_item::get_required() 
{
	return _required;
}

int parse_item::get_enforced_count()
{
	return _enforced_count;
}

int parse_item::get_major_version_number_implemented()
{
	return _major_version_number_implemented;
}

int parse_item::get_minor_version_implemented()
{
	return _minor_version_number_implemented;
}

int parse_item::get_revision_number_implemented()
{
	return _revision_number_implemented;
}

int parse_item::get_major_version_deprecated()
{
	return _major_version_number_deprecated;
}

int parse_item::get_minor_version_deprecated()
{
	return _minor_version_number_deprecated;
}

int parse_item::get_revision_number_deprecated()
{
	return _revision_number_deprecated;
}

SCP_string parse_item::get_deprecation_message()
{
	return _deprecation_message;
}

parse_item_builder::parse_item_builder()
{
	_product.reset_fields();
}

void parse_item_builder::finished_product(table_manager* target_table)
{
	target_table->_all_items.push_back(_product);
}

// ====== Factory functions ======
parse_item_builder* parse_item_builder::new_parse_item(SCP_string name)
{
	_product.reset_fields();
	_product._table_string = std::move(name);
	return this;
}

parse_item_builder* parse_item_builder::set_description(SCP_string description)
{
	_product._description = std::move(description);
	return this;
}

parse_item_builder* parse_item_builder::set_type(Parse_Input_Types type)
{
	_product._type = type;
	return this;
}

parse_item_builder* parse_item_builder::set_required(bool required)
{
	_product._required = required;
	return this;
}

parse_item_builder* parse_item_builder::set_enforced_count(int enforced_count)
{
	_product._enforced_count = enforced_count;
	return this;
}

parse_item_builder* parse_item_builder::set_implementation_version(int major_implementation_version, int minor_implementation_version, int revision_implementation_number)
{
	_product._major_version_number_implemented = major_implementation_version;
	_product._minor_version_number_implemented = minor_implementation_version;
	_product._revision_number_implemented = revision_implementation_number;

	return this;
}

parse_item_builder* parse_item_builder::set_deprecation_version(int major_deprecation_version, int minor_deprecation_version, int revision_deprecation_number)
{
	_product._major_version_number_deprecated = major_deprecation_version;
	_product._minor_version_number_deprecated = minor_deprecation_version;
	_product._revision_number_deprecated = revision_deprecation_number;
	return this;
}

parse_item_builder* parse_item_builder::set_deprecation_message(SCP_string deprecation_message)
{
	_product._deprecation_message = std::move(deprecation_message);
	return this;
}

void parse_item_builder::set_table_type(table_manager* target_table, Table_Types type)
{
	target_table->_table_type = type;
}


// dumps the documentation to a file. (code shamelessly ripped from outwnd.cpp and sexp.cpp)
void table_manager::dump_to_file()
{

	// Make sure the directory is created.
	_mkdir(os_get_config_path(Pathtypes[CF_TYPE_DATA].path).c_str());

	SCP_string pre_filename = get_table_filename_from_type(_table_type);

	SCP_string filename = pre_filename + ".html";

	char pathname[MAX_PATH_LEN];
	memset(pathname, 0, sizeof(pathname));
	snprintf(pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, filename.c_str());

	SCP_string fullpath = os_get_config_path(pathname);

	FILE* file_out = fopen(fullpath.c_str(), "wb");
	
	if (file_out == nullptr) {
		Warning(LOCATION, "Unable to open the file location in order to print %s documenation", pre_filename.c_str());
		return;
	}

	//Header
	fprintf(file_out, "<html>\n<head>\n\t<title>%s Output - FSO v%s</title>\n</head>\n", pre_filename.c_str(), FS_VERSION_FULL);
	fputs("<body>", file_out);
	fprintf(file_out,"\t<h1>%s Documentation - FSO v%s</h1>\n", pre_filename.c_str(), FS_VERSION_FULL);

	//dl dt dd
	for (auto& current_item : _all_items)
	{
		fprintf(file_out, "<p>\t\t<dl> <dt> Item: </dt> <dd>%s</dd>\n", current_item.get_table_string().c_str());
		fprintf(file_out, "\t\t<dt> Description: </dt> <dd>%s</dd>\n", current_item.get_description().c_str());
		if (current_item.get_type() != Parse_Input_Types::USAGE_NOTE) {
			fprintf(file_out, "\t\t<dt> Type: </dt> <dd>%s</dd>\n", get_parse_item_type_description_from_type(current_item.get_type()).c_str());
		}

		if (current_item.get_enforced_count() > NO_ENFORCED_COUNT) {
			fprintf(file_out, "\t\t<dt> Required number of entires: </dt> <dd>%d</dd>\n", current_item.get_enforced_count());
		}

		if (current_item.get_major_version_number_implemented() > INVALID_VERSION) {
			fprintf(file_out, "\t\t<dt> Version Implemented: </dt><dd>%d.%d.%d</dd>\n", current_item.get_major_version_number_implemented(), current_item.get_minor_version_implemented(), current_item.get_revision_number_implemented());
		}

		if (current_item.get_major_version_deprecated() > INVALID_VERSION) {
			fprintf(file_out, "\t\t<dt> Version Deprecated: </dt> <dd>%d.%d.%d</dd>\n", current_item.get_major_version_deprecated(), current_item.get_minor_version_deprecated(), current_item.get_revision_number_deprecated());
			fprintf(file_out, "\t\t<dt> Deprecation Reason: </dt> <dd>%s</dd>\n", current_item.get_deprecation_message().c_str());
		}
		fputs("<dl></p>", file_out);
	}

	fputs("</body>\n</html>\n", file_out);

	fclose(file_out);
}

SCP_string get_parse_item_type_description_from_type(Parse_Input_Types type_in) {
	switch (type_in) {
		case Parse_Input_Types::USAGE_NOTE:
			return "Help item that only exists in documentation.";
			break;
		case Parse_Input_Types::STRING:
			return "Regular string";
			break;
		case Parse_Input_Types::XSTR:
			return "Translateable \"XSTR\" string";
			break;
		case Parse_Input_Types::INT:
			return "Integer";
			break;
		case Parse_Input_Types::INT_LIST:
			return "List of Integers";
			break;
		case Parse_Input_Types::LONG:
			return "Long";
			break;
		case Parse_Input_Types::FLOAT:
			return "Float";
			break;
		case Parse_Input_Types::FLOAT_LIST:
			return "List of floats";
			break;
		case Parse_Input_Types::DOUBLE:
			return "Double";
			break;
		case Parse_Input_Types::VEC2:
			return "2 Dimensional Vector";
			break;
		case Parse_Input_Types::VEC3:
			return "3 Dimensional Vector";
			break;
		case Parse_Input_Types::BOOL:
			return "Boolean";
			break;
		case Parse_Input_Types::FLAGS:
			return "List of Flags";
			break;
		case Parse_Input_Types::MARKER:
			return "Only marks a new section or provides instructions for the parser.";
			break;
		case Parse_Input_Types::MODEL_FILENAME:
			return "Model filname";
			break;
		case Parse_Input_Types::TEXTURE_FILENAME:
			return "Texture filename";
			break;
		case Parse_Input_Types::SOUND_FILENAME:
			return "Sound filename";
			break;
		default:
			UNREACHABLE("A coder passed an inavlid type value to get_parse_item_type_description_from_type() of %d. Please report!", (int)type_in);
			return "";
	}
}

SCP_string get_table_filename_from_type(Table_Types type_in)
{
	switch (type_in) {
	case Table_Types::AI:
		return "ai.tbl";
		break;
	case Table_Types::AI_PROFILES:
		return "ai_profiles.tbl";
		break;
	case Table_Types::ARMOR:
		return "armor.tbl";
		break;
	case Table_Types::ASTEROID:
		return "asteroid.tbl";
		break;
	case Table_Types::AUTOPILOT:
		return "autopilot.tbl";
		break;
	case Table_Types::COLORS:
		return "colors.tbl";
		break;
	case Table_Types::CONTROL_CONFIG_DEFAULTS:
		return "controlconfigdefaults.tbl";
		break;
	case Table_Types::CREDITS:
		return "credits.tbl";
		break;
	case Table_Types::CUTSCENES:
		return "cutscenes.tbl";
		break;
	case Table_Types::FIREBALL:
		return "fireball.tbl";
		break;
	case Table_Types::FONTS:
		return "fonts.tbl";
		break;
	case Table_Types::GAME_SETTINGS:
		return "game_settings.tbl";
		break;
	case Table_Types::GLOWPOINTS:
		return "glowpoints.tbl";
		break;
	case Table_Types::HELP:
		return "help.tbl";
		break;
	case Table_Types::HUD_GAUGES:
		return "hud_gauges.tbl";
		break;
	case Table_Types::ICONS:
		return "icons.tbl";
		break;
	case Table_Types::IFF_DEFS:
		return "iff_defs.tbl";
		break;
	case Table_Types::LIGHTNING:
		return "lightning.tbl";
		break;
	case Table_Types::MAINHALL:
		return "mainhall.tbl";
		break;
	case Table_Types::MEDALS:
		return "medals.tbl";
		break;
	case Table_Types::MESSAGES:
		return "messages.tbl";
		break;
	case Table_Types::MFLASH:
		return "mflash.tbl";
		break;
	case Table_Types::MUSIC:
		return "music.tbl";
		break;
	case Table_Types::NEBULA:
		return "nebula.tbl";
		break;
	case Table_Types::OBJECT_TYPES:
		return "objecttypes.tbl";
		break;
	case Table_Types::PIXELS:
		return "pixels.tbl";
		break;
	case Table_Types::POST_PROCESSING:
		return "credits.tbl";
		break;
	case Table_Types::RANK:
		return "rank.tbl";
		break;
	case Table_Types::SCRIPTING:
		return "scripting.tbl";
		break;
	case Table_Types::SHIPS:
		return "ships.tbl";
		break;
	case Table_Types::SEXPS:
		return "sexps.tbl";
		break;
	case Table_Types::SOUNDS:
		return "sounds.tbl";
		break;
	case Table_Types::SPECIES_DEFS:
		return "species_defs.tbl";
		break;
	case Table_Types::SPECIES:
		return "species.tbl";
		break;
	case Table_Types::SSM:
		return "ssm.tbl";
		break;
	case Table_Types::STARS:
		return "stars.tbl";
		break;
	case Table_Types::STRINGS:
		return "strings.tbl";
		break;
	case Table_Types::TIPS:
		return "tips.tbl";
		break;
	case Table_Types::TRAITOR:
		return "traitor.tbl";
		break;
	case Table_Types::TSTRINGS:
		return "tstrings.tbl";
		break;
	case Table_Types::WEAPONS_EXPL:
		return "weapon_expl.tbl";
		break;
	case Table_Types::WEAPONS:
		return "weapons.tbl";
		break;
	default:
		UNREACHABLE("A coder made a mistake and passed a bad Table_Type of %d in get_table_filename_from_type, please report!", (int)type_in);
		return "";
		break;
	}
}

void init_parse_tables() 
{

}
