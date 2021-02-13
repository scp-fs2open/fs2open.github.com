#pragma

#ifndef parse_manager_h
#define parse_manager_h


#include "globalincs/systemvars.h"
#include "globalincs/pstypes.h"

constexpr int NO_ENFORCED_COUNT = -1;
constexpr int INVALID_VERSION = -1;

enum class Parse_Input_Types
{
	USAGE_NOTE,
	STRING,
	XSTR,
	INT,
	INT_LIST,
	LONG,
	FLOAT,
	FLOAT_LIST,
	DOUBLE,
	VEC2,
	VEC3,
	BOOL,
	FLAGS,
	MARKER,
	MODEL_FILENAME,
	TEXTURE_FILENAME,
	SOUND_FILENAME
};

// Allows a Table_manager to choose which filenames to use based on what type of table it is.
enum class Table_Types
{
	AI,
	AI_PROFILES,
	ARMOR,
	ASTEROID,
	AUTOPILOT,
	COLORS,
	CONTROL_CONFIG_DEFAULTS,
	CREDITS,
	CUTSCENES,
	FIREBALL,
	FONTS,
	GAME_SETTINGS,
	GLOWPOINTS,
	HELP,
	HUD_GAUGES,
	ICONS,
	IFF_DEFS,
	LIGHTNING,
	MAINHALL,
	MEDALS,
	MESSAGES,
	MFLASH,
	MUSIC,
	NEBULA,
	OBJECT_TYPES,
	PIXELS,
	POST_PROCESSING,
	RANK,
	SCRIPTING,
	SHIPS,
	SEXPS,
	SOUNDS,
	SPECIES_DEFS,
	SPECIES,
	SSM,
	STARS,
	STRINGS,
	TIPS,
	TRAITOR,
	TSTRINGS,
	WEAPONS_EXPL,
	WEAPONS
};

// forward declarations for the classes defined below
class parse_item_builder;
class table_manager;

class parse_item {
	friend class parse_item_builder;

private:
	// basic description 
	SCP_string	_table_string   = "FIX ME!";
	SCP_string	_description    = "Someone didn't initialize me correctly. Go get a coder.";

	Parse_Input_Types	_type   = Parse_Input_Types::USAGE_NOTE;
	// rules for this element
	bool		_required		= false;
	int			_enforced_count = NO_ENFORCED_COUNT;		// if this is a list that has a specific number of entries, mark it with this variable
	int			_major_version_number_implemented = INVALID_VERSION;
	int			_minor_version_number_implemented = INVALID_VERSION;
	int			_revision_number_implemented	  = INVALID_VERSION;
	int			_major_version_number_deprecated  = INVALID_VERSION;
	int			_minor_version_number_deprecated  = INVALID_VERSION;
	int			_revision_number_deprecated		  = INVALID_VERSION;
	SCP_string	_deprecation_message			  = "";

	void reset_fields();

public:
	parse_item() = default;

	SCP_string get_table_string();
	SCP_string get_description();
	Parse_Input_Types get_type();
	bool get_required();
	int get_enforced_count();
	int get_major_version_number_implemented();
	int get_minor_version_implemented();
	int get_revision_number_implemented();
	int get_major_version_deprecated();
	int get_minor_version_deprecated();
	int get_revision_number_deprecated();
	SCP_string get_deprecation_message();
	
};

class parse_item_builder {
	parse_item _product;
public:
	parse_item_builder();

	parse_item_builder* new_parse_item(SCP_string name);
	parse_item_builder* set_description(SCP_string description);
	parse_item_builder* set_type(Parse_Input_Types type);
	parse_item_builder* set_required(bool required);
	parse_item_builder* set_enforced_count(int enforced_count);
	parse_item_builder* set_implementation_version(int major_implementation_version, int minor_implementation_version, int revision_implementation_number);
	parse_item_builder* set_deprecation_version(int major_deprecation_version, int minor_deprecation_version, int revision_deprecation_number);
	parse_item_builder* set_deprecation_message(SCP_string deprecation_message);
	
	void finished_product(table_manager* target_table);

	static void set_table_type(table_manager* target_table, Table_Types type);
};

class table_manager {
private:
	SCP_vector<parse_item> _all_items;
	Table_Types _table_type;
	friend class parse_item_builder;
public:

	void dump_to_file();
};

SCP_string get_table_filename_from_type(Table_Types type_in);
SCP_string get_parse_item_type_description_from_type(Parse_Input_Types type_in);

#endif
