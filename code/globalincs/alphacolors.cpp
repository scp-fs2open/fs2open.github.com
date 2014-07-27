/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "graphics/2d.h"
#include "parse/parselo.h"
#include "globalincs/def_files.h"
#include "globalincs/alphacolors.h"

SCP_map<SCP_string, team_color> Team_Colors;
SCP_vector<SCP_string> Team_Names;

SCP_map<char, color*> Tagged_Colors;
SCP_map<char, color> Custom_Colors;
SCP_vector<char> Color_Tags;

// -----------------------------------------------------------------------------------
// ALPHA DEFINES/VARS
//

color Color_text_normal, Color_text_subselected, Color_text_selected;
color Color_text_error, Color_text_error_hi, Color_text_active, Color_text_active_hi;
color Color_text_heading, Color_more_indicator, Color_more_bright, Color_bright, Color_normal;

color Color_blue, Color_bright_blue, Color_green, Color_bright_green;
color Color_black, Color_grey, Color_silver, Color_white, Color_bright_white;
color Color_violet_gray, Color_violet, Color_pink, Color_light_pink;
color Color_dim_red, Color_red, Color_bright_red, Color_yellow, Color_bright_yellow;
color Color_ui_light_green, Color_ui_green;
color Color_ui_light_pink, Color_ui_pink;

color Brief_color_red, Brief_color_green, Brief_color_legacy_neutral;

char default_fiction_viewer_color = 'w';
char default_command_briefing_color = 'w';
char default_briefing_color = 'w';
char default_redalert_briefing_color = 'w';
char default_debriefing_color = 'w';
char default_recommendation_color = 'r';
char default_loop_briefing_color = 'w';

color *COLOR_LIST[TOTAL_COLORS] = {
	&Color_blue,
	&Color_bright_blue,
	&Color_green,
	&Color_bright_green,
	&Color_black,
	&Color_grey,
	&Color_silver,
	&Color_white,
	&Color_bright_white,
	&Color_violet_gray,
	&Color_violet,
	&Color_dim_red,
	&Color_red,
	&Color_bright_red,
	&Color_pink,
	&Color_light_pink,
	&Color_yellow,
	&Color_bright_yellow,
	&Color_ui_light_green,
	&Color_ui_green,
	&Color_ui_light_pink,
	&Color_ui_pink,
};

const char *COLOR_NAMES[TOTAL_COLORS] = {
	"Blue",
	"Bright Blue",
	"Green",
	"Bright Green",
	"Black",
	"Grey",
	"Silver",
	"White",
	"Bright White",
	"Violet Gray",
	"Violet",
	"Dim Red",
	"Red",
	"Bright Red",
	"Pink",
	"Light Pink",
	"Yellow",
	"Bright Yellow",
	"UI Light Green",
	"UI Green",
	"UI Light Pink",
	"UI Pink"
};

const int rgba_defaults[TOTAL_COLORS][4] = {
	{93, 93, 128, 255},
	{185, 185, 255, 255},
	{0, 120, 0, 255},
	{50, 190, 50, 255},
	{0, 0, 0, 255},
	{65, 65, 65, 255},
	{160, 160, 160, 255},
	{105, 105, 105, 255},
	{255, 255, 255 ,255},
	{160, 144, 160, 255},
	{192, 104, 192, 255},
	{80, 6, 6, 255},
	{126, 6, 6, 255},
	{200, 0, 0, 255},
	{185, 150, 150, 255},
	{230, 190, 190, 255},
	{255, 255, 122, 255},
	{255, 255, 0, 255},
	{161, 184, 161, 255},
	{190, 228, 190, 255},
	{184, 161, 161, 255},
	{228, 190, 190, 255}
};

color *interface_colors[INTERFACE_COLORS] = {
	&Color_text_normal,
	&Color_text_subselected,
	&Color_text_selected,
	&Color_text_error,
	&Color_text_error_hi,
	&Color_text_active,
	&Color_text_active_hi,
	&Color_text_heading,
	&Color_more_indicator,
	&Color_more_bright,
	&Color_bright,
	&Color_normal,
};

const int interface_defaults[INTERFACE_COLORS] = {
	7,	//"White"
	0,	//"Blue"
	1,	//"Bright Blue"
	12,	//"Red"
	13,	//"Bright Red"
	8,	//"Bright White"
	8,	//"Bright White"
	9,	//"Violet Gray"
	12,	//"Red"
	13,	//"Bright Red"
	1,	//"Bright Blue"
	7,	//"White"
};

#define DEFAULT_TAGS	20

const char DEFAULT_TAG_LIST[DEFAULT_TAGS] = {
	'w',
	'W',
	'r',
	'g',
	'y',
	'b',
	'f',
	'h',
	'n',
	'B',
	'G',
	'R',
	'Y',
	'k',
	'e',
	'E',
	'v',
	'V',
	'p',
	'P',
};

color* DEFAULT_TAG_COLORS[DEFAULT_TAGS] = {
	&Color_white,
	&Color_bright_white,
	&Color_red,
	&Color_green,
	&Color_yellow,
	&Color_blue,
	&Brief_color_green,
	&Brief_color_red,
	&Brief_color_legacy_neutral,
	&Color_bright_blue,
	&Color_bright_green,
	&Color_bright_red,
	&Color_bright_yellow,
	&Color_black,
	&Color_grey,
	&Color_silver,
	&Color_violet_gray,
	&Color_violet,
	&Color_pink,
	&Color_light_pink,
};


// netplayer colors
color *Color_netplayer[NETPLAYER_COLORS] = {

	&Color_blue,
	&Color_bright_blue,
	&Color_green,
	&Color_bright_green,
	&Color_pink,
	&Color_grey,
	&Color_silver,
	&Color_white,
	&Color_bright_white,
	&Color_violet_gray,
	&Color_violet,
	&Color_dim_red,
	&Color_red,
	&Color_bright_red,
	&Color_yellow,
	&Color_bright_yellow,
	&Color_ui_light_green,
	&Color_ui_green,
	&Color_ui_light_pink,
	&Color_ui_pink,
};


void parse_colors(const char *filename);
void parse_everything_else(const char *filename);	// needs a better name

// -----------------------------------------------------------------------------------
// ALPHA FUNCTIONS
//

/**
* CommanderDJ: initialise alpha colors based on colors.tbl
* Made modular and given a wider range of features by MageKing17
*/
void alpha_colors_init()
{
	// Set our default colors.
	int i;
	for (i = 0; i < TOTAL_COLORS; i++) {
		gr_init_alphacolor(COLOR_LIST[i], rgba_defaults[i][0], rgba_defaults[i][1], rgba_defaults[i][2], rgba_defaults[i][3]);
	}

	if (cf_exists_full("colors.tbl", CF_TYPE_TABLES)) {
		mprintf(("TABLES => Starting parse of 'colors.tbl' (checking '#Start Colors' section only)...\n"));
		parse_colors("colors.tbl");
	}
	parse_modular_table(NOX("*-clr.tbm"), parse_colors);

	// Set defaults for interface colors and color tags (must be done after the above because they're generally just copies of above-defined colors).
	for (i = 0; i < INTERFACE_COLORS; i++) {
		memcpy(interface_colors[i], COLOR_LIST[interface_defaults[i]], sizeof(color));
	}

	for (i = 0; i < DEFAULT_TAGS; i++) {
		Tagged_Colors[DEFAULT_TAG_LIST[i]] = DEFAULT_TAG_COLORS[i];
		Color_Tags.push_back(DEFAULT_TAG_LIST[i]);
	}

	if (cf_exists_full("colors.tbl", CF_TYPE_TABLES)) {
		mprintf(("TABLES => Starting parse of 'colors.tbl' (skipping '#Start Colors' section)...\n"));
		parse_everything_else("colors.tbl");
	}
	parse_modular_table(NOX("*-clr.tbm"), parse_everything_else);
}

void parse_colors(const char *filename)
{
	Assertion(filename != NULL, "parse_colors() called on NULL; get a coder!\n");
	read_file_text(filename, CF_TYPE_TABLES);

	int err_code;
	if ((err_code = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %d.\n", filename, err_code));
		return;
	}

	reset_parse();

	// we search for the colors based on their order of definition above
	// if the color isn't found, we just use default values
	if (optional_string("#Start Colors"))
	{
		// vars for holding rgba values
		int rgba[4] = {0,0,0,0};
		int i, j;

		char* color_names[TOTAL_COLORS] = {
			"$Blue:",
			"$Bright Blue:",
			"$Green:",
			"$Bright Green:",
			"$Black:",
			"$Grey:",
			"$Silver:",
			"$White:",
			"$Bright White:",
			"$Violet Gray:",
			"$Violet:",
			"$Dim Red:",
			"$Red:",
			"$Bright Red:",
			"$Pink:",
			"$Light Pink:",
			"$Yellow:",
			"$Bright Yellow:",
			"$UI Light Green:",
			"$UI Green:",
			"$UI Light Pink:",
			"$UI Pink:"
		};

		// now for each color, check if it's corresponding string is there
		for (i = 0; i < TOTAL_COLORS; i++) {
			if (optional_string(color_names[i])) {
				// if so, get its rgba values and initialise it using them
				mprintf(("'%s' has been redefined.\n", color_names[i]));
				//if (check_for_string("(")) {
					stuff_int_list(rgba, 4, RAW_INTEGER_TYPE);
					for (j = 0; j < 4; j++) {
						if (rgba[j] < 0) {
							Warning(LOCATION, "RGBA value for '%s' in %s too low (%d), capping to 0.\n", color_names[i], filename, rgba[j]);
							rgba[j] = 0;
						} else if (rgba[j] > 255) {
							Warning(LOCATION, "RGBA value for '%s' in %s too high (%d), capping to 255.\n", color_names[i], filename, rgba[j]);
							rgba[j] = 255;
						}
					}
				//} else {
				//	stuff_hex_list(rgba, 4);
				//}
				gr_init_alphacolor(COLOR_LIST[i], rgba[0], rgba[1], rgba[2], rgba[3]);
			}
		}
		required_string("#End");
	}
}

void parse_everything_else(const char *filename)
{
	Assertion(filename != NULL, "parse_everything_else() called on NULL; get a coder!\n");
	read_file_text(filename, CF_TYPE_TABLES);

	int err_code;
	if ((err_code = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %d.\n", filename, err_code));
		return;
	}

	reset_parse();

	int rgba[4] = {0,0,0,0};

	// reusable temp vars
	int i, j;
	SCP_string temp;

	if (optional_string("#Start Colors")) {
		// Skip this section; we already parsed it in every file.
		skip_to_string("#End", NULL);
	}

	//Team coloring
	if (optional_string("#Team Colors")) {

		while (required_string_either("#End", "$Team Name:")) {
			required_string("$Team Name:"); // required to move the parse pointer forward
			team_color temp_color;

			char temp2[NAME_LENGTH];
			stuff_string(temp2, F_NAME, NAME_LENGTH);
			temp = temp2;

			if (!stricmp(temp2, "none")) {
				Warning(LOCATION, "Team color in '%s' defined with a name of '%s'; this won't be usable due to 'None' being used for a lack of a team color by the engine.\n", filename, temp2);
			}

			if (required_string("$Team Stripe Color:")) {
				int rgb[3];
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				for (i = 0; i < 3; i++) {
					CLAMP(rgb[i], 0, 255);
				}
				
				temp_color.stripe.r = rgb[0] / 255.0f;
				temp_color.stripe.g = rgb[1] / 255.0f;
				temp_color.stripe.b = rgb[2] / 255.0f;
			}

			if (required_string("$Team Base Color:")) {
				int rgb[3];
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				for (i = 0; i < 3; i++) {
					CLAMP(rgb[i], 0, 255);
				}

				temp_color.base.r = rgb[0] / 255.0f;
				temp_color.base.g = rgb[1] / 255.0f;
				temp_color.base.b = rgb[2] / 255.0f;
			}

			if (Team_Colors.find(temp) == Team_Colors.end()) {	// Only push to the vector if the team isn't already defined.
				Team_Names.push_back(temp);
			}
			Team_Colors[temp] = temp_color;
		}
		required_string("#End");
	}

	// Previously-hardcoded interface colors
	if (optional_string("#Interface Colors")) {
		char *color_names[INTERFACE_COLORS] = {
			"$Text Normal:",
			"$Text Subselected:",
			"$Text Selected:",
			"$Text Error:",
			"$Text Error Highlighted:",
			"$Text Active:",
			"$Text Active Highlighted:",
			"$Text Heading:",
			"$More Indicator:",
			"$Bright More Indicator:",
			"$Bright:",
			"$Normal:",
		};

		// now for each color, check if its corresponding string is there
		for (i = 0; i < INTERFACE_COLORS; i++) {
			if (optional_string(color_names[i])) {
				// if so, get its rgba values and initialise it using them
				mprintf(("'%s' has been redefined.\n", color_names[i]));
				if ( check_for_string("(") ) {
					// If we have a list of integers, use them.
					stuff_int_list(rgba, 4, RAW_INTEGER_TYPE);
					for (j = 0; j < 4; j++) {
						if (rgba[j] < 0)
						{
							Warning(LOCATION, "RGBA value for '%s' in %s too low (%d), capping to 0.\n", color_names[i], filename, rgba[j]);
							rgba[j] = 0;
						}
						else if (rgba[j] > 255)
						{
							Warning(LOCATION, "RGBA value for '%s' in %s too high (%d), capping to 255.\n", color_names[i], filename, rgba[j]);
							rgba[j] = 255;
						}
					}
					gr_init_alphacolor(interface_colors[i], rgba[0], rgba[1], rgba[2], rgba[3]);
				//} else if (check_for_string("#")) {
				//	stuff_hex_list(rgba, 4);
				//	gr_init_alphacolor(interface_colors[i], rgba[0], rgba[1], rgba[2], rgba[3]);
				} else {
					// We have a string; it should be the name of a color to use.
					stuff_string(temp, F_NAME);
					for (j = 0; j < TOTAL_COLORS; j++) {
						if ( !temp.compare(COLOR_NAMES[j]) ) {
							break;
						}
					}
					if ( j == TOTAL_COLORS ) {
						Warning(LOCATION, "Unknown color '%s' in %s, for definition of '%s'; using default ('%s').\n", temp.c_str(), filename, color_names[i], COLOR_NAMES[interface_defaults[i]]);
					} else {
						Assertion(j >= 0 && j < TOTAL_COLORS, "Attempting to copy nonexistant color (%d out of 0-%d)!\n", j, TOTAL_COLORS-1);
						memcpy(interface_colors[i], COLOR_LIST[j], sizeof(color));
					}
				}
			}
		}
		required_string("#End");
	}

	// Text color tags; for briefings, command briefings, debriefings, and the fiction viewer
	if (optional_string("#Color Tags")) {
		while (required_string_either("$Tag:", "#End") < 1) {
			required_string("$Tag:");
			color temp_color;
			char tag;

			stuff_string(temp, F_RAW);
			if (temp[0] == '$') {
				if (temp[1] == '\0') {
					Error(LOCATION, "%s - found a '$Tag:' entry with a solitary '$'.\n", filename);
				}
				tag = temp[1];
				if (temp[2] != '\0') {
					Warning(LOCATION, "%s - tag '$%c' has extra text in its definition.\n", filename, tag);
				}
			} else if (temp[0] == '\0') {
				Error(LOCATION, "%s - found a '$Tag:' entry with no tag.\n", filename);
			} else {
				tag = temp[0];
				if (temp[1] != '\0') {
					Warning(LOCATION, "%s - tag '$%c' has extra text in its definition.\n", filename, tag);
				}
			}

			if (Tagged_Colors.find(tag) == Tagged_Colors.end()) {	// Only push the tag to our list of tags if it's actually new, not just a redefinition.
				Color_Tags.push_back(tag);
			}

			switch(required_string_one_of(4, "+Color:", "+Friendly", "+Hostile", "+Neutral")) {
			case 0:	// +Color
				required_string("+Color:");

				rgba[0] = rgba[1] = rgba[2] = 0;
				rgba[3] = 255;	// Odds are pretty high you want it to have full alpha...

				if ( check_for_string("(") ) {
					stuff_int_list(rgba, 4, RAW_INTEGER_TYPE);
					for (j = 0; j < 4; j++) {
						if (rgba[j] < 0)
						{
							Warning(LOCATION, "RGBA value for '$%c' in %s too low (%d), capping to 0.\n", tag, filename, rgba[j]);
							rgba[j] = 0;
						}
						else if (rgba[j] > 255)
						{
							Warning(LOCATION, "RGBA value for '$%c' in %s too high (%d), capping to 255.\n", tag, filename, rgba[j]);
							rgba[j] = 255;
						}
					}
					gr_init_alphacolor(&temp_color, rgba[0], rgba[1], rgba[2], rgba[3]);
					Custom_Colors[tag] = temp_color;
					Tagged_Colors[tag] = &Custom_Colors[tag];
				//} else if ( check_for_string ("#") ) {
				//	stuff_hex_list(rgba, 4);
				//	gr_init_alphacolor(&temp_color, rgba[0], rgba[1], rgba[2], rgba[3]);
				//	Custom_Colors[tag] = temp_color;
				//	Tagged_Colors[tag] = &Custom_Colors[tag];
				} else {
					// We have a string; it should be the name of a color to use.
					stuff_string(temp, F_NAME);
					for (j = 0; j < TOTAL_COLORS; j++) {
						if ( !temp.compare(COLOR_NAMES[j]) ) {
							break;
						}
					}
					if ( j == TOTAL_COLORS ) {
						Error(LOCATION, "Unknown color '%s' in %s, for definition of tag '$%c'.\n", temp.c_str(), filename, tag);
					}
					Tagged_Colors[tag] = COLOR_LIST[j];
				}
				break;
			case 1:	// +Friendly
				required_string("+Friendly");
				Tagged_Colors[tag] = &Brief_color_green;
				break;
			case 2:	// +Hostile
				required_string("+Hostile");
				Tagged_Colors[tag] = &Brief_color_red;
				break;
			case 3:	// +Neutral
				required_string("+Neutral");
				Tagged_Colors[tag] = &Brief_color_legacy_neutral;
				break;
			case -1:
				// -noparseerrors is set
				if (Tagged_Colors.find(tag) == Tagged_Colors.end()) {	// It was a new color, but since we haven't actually defined it...
					Color_Tags.pop_back();
				}
				break;
			default:
				Assertion(false, "MageKing17 made a coding error somewhere, and you should laugh at him (and report this error).\n");
				break;
			}
		}

		required_string("#End");
	}
	Assertion(Color_Tags.size() == Tagged_Colors.size(), "Color_Tags and Tagged_Colors size mismatch; get a coder!\n");

	if (optional_string("#Default Text Colors")) {

		char* color_names[MAX_DEFAULT_TEXT_COLORS] = {
			"$Fiction Viewer:",
			"$Command Briefing:",
			"$Briefing:",
			"$Redalert Briefing:",
			"$Debriefing:",
			"$Recommendation:",
			"$Loop Briefing:",
		};

		char *color_value[MAX_DEFAULT_TEXT_COLORS] = {
			&default_fiction_viewer_color,
			&default_command_briefing_color,
			&default_briefing_color,
			&default_redalert_briefing_color,
			&default_debriefing_color,
			&default_recommendation_color,
			&default_loop_briefing_color,
		};

		for (i = 0; i < MAX_DEFAULT_TEXT_COLORS; i++) {
			if ( optional_string(color_names[i]) ) {
				stuff_string(temp, F_RAW);
				if (temp[0] == '$') {
					if (temp[1] == '\0') {
						Error(LOCATION, "%s - default text color '%s' entry with a solitary '$'.\n", filename, color_names[i]);
					}
					*color_value[i] = temp[1];
					if (temp[2] != '\0') {
						Warning(LOCATION, "%s - default text color '%s' has extra text after the tag '$%c'.\n", filename, color_names[i], *color_value[i]);
					}
				} else if (temp[0] == '\0') {
					Error(LOCATION, "%s - default text color '%s' entry with no tag.\n", filename, color_names[i]);
				} else {
					*color_value[i] = temp[0];
					if (temp[1] != '\0') {
						Warning(LOCATION, "%s - default text color '%s' has extra text after the tag '$%c'.\n", filename, color_names[i], *color_value[i]);
					}
				}
				if (Tagged_Colors.find(*color_value[i]) == Tagged_Colors.end()) {
					// Just mprintf() this information instead of complaining with a Warning(); the tag might be defined in a later-loading TBM, and if it isn't, nothing too terrible will happen.
					mprintf(("%s - default text color '%s' set to non-existant tag '$%c'.\n", filename, color_names[i], *color_value[i]));
				}
			}
		}
		required_string("#End");
	}
}
