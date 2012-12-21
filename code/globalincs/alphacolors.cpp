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

// -----------------------------------------------------------------------------------
// ALPHA DEFINES/VARS
//

color Color_blue, Color_bright_blue, Color_green, Color_bright_green;
color Color_black, Color_grey, Color_silver, Color_white, Color_bright_white;
color Color_violet_gray, Color_violet, Color_pink, Color_light_pink;
color Color_dim_red, Color_red, Color_bright_red, Color_yellow, Color_bright_yellow;
color Color_ui_light_green, Color_ui_green;
color Color_ui_light_pink, Color_ui_pink;

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


// -----------------------------------------------------------------------------------
// ALPHA FUNCTIONS
//

/**
* CommanderDJ: initialise alpha colors based on colors.tbl
* \return 1 if successful, 0 if init fails for any reason
*/
int new_alpha_colors_init()
{
	int err_code;
	if ((err_code = setjmp(parse_abort)) != 0)
	{
		mprintf(("Unable to parse 'colors.tbl'!  Error code = %d.\n", err_code));
		return 0;
	}

	if (cf_exists_full("colors.tbl", CF_TYPE_TABLES))
	{
		read_file_text("colors.tbl", CF_TYPE_TABLES);
		mprintf(("TABLES => Starting parse of 'colors.tbl'...\n"));
	}
	else
	{
		//colors.tbl doesn't exist
		mprintf(("TABLES => Unable to find 'colors.tbl'. Initialising colors with default values.\n"));
		return 0;
	}

	reset_parse();

	// we search for the colors based on their order of definition above
	// if the color isn't found, we just use default values
	if (required_string("#Start Colors"))
	{
		// vars for holding rgba values
		int rgba[4] = {0,0,0,0};

		color *colors[TOTAL_COLORS] = {
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

		int rgba_defaults[TOTAL_COLORS][4] = {
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

		// now for each color, check if it's corresponding string is there
		for (int i=0; i<TOTAL_COLORS; i++)
		{
			if (optional_string(color_names[i]))
			{
				// if so, get its rgba values and initialise it using them
				mprintf(("'%s' has been redefined.\n", color_names[i]));
				stuff_int_list(rgba, 4, RAW_INTEGER_TYPE);
				for (int j=0; j<4; j++)
				{
					if (rgba[j] < 0)
					{
						Warning(LOCATION, "RGBA value for '%s' in colors.tbl too low (%d), capping to 0.", color_names[i], rgba[j]);
						rgba[j] = 0;
					}
					else if (rgba[j] > 255)
					{
						Warning(LOCATION, "RGBA value for '%s' in colors.tbl too high (%d), capping to 255.", color_names[i], rgba[j]);
						rgba[j] = 255;
					}
				}
				gr_init_alphacolor(colors[i], rgba[0], rgba[1], rgba[2], rgba[3]);
			}
			// otherwise use its defaults
			else
				gr_init_alphacolor(colors[i], rgba_defaults[i][0], rgba_defaults[i][1], rgba_defaults[i][2], rgba_defaults[i][3]);
		}
		required_string("#End");
	}

	//Team coloring
	if (optional_string("#Team Colors")) {

		while (required_string_either("#End", "$Team Name:")) {
			required_string("$Team Name:"); // required to move the parse pointer forward
			char temp[NAME_LENGTH];
			SCP_string temp2;
			team_color temp_color;

			stuff_string(temp, F_NAME, NAME_LENGTH);
			temp2 = SCP_string(temp);

			if (required_string("$Team Stripe Color:")) {
				int rgb[3];
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				for (int i = 0; i < 3; i++) {
					CLAMP(rgb[i], 0, 255);
				}
				
				temp_color.stripe.r = rgb[0] / 255.0f;
				temp_color.stripe.g = rgb[1] / 255.0f;
				temp_color.stripe.b = rgb[2] / 255.0f;
			}

			if (required_string("$Team Base Color:")) {
				int rgb[3];
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				for (int i = 0; i < 3; i++) {
					CLAMP(rgb[i], 0, 255);
				}

				temp_color.base.r = rgb[0] / 255.0f;
				temp_color.base.g = rgb[1] / 255.0f;
				temp_color.base.b = rgb[2] / 255.0f;
			}

			Team_Colors[temp2] = temp_color;
			Team_Names.push_back(temp2);
		}
	}

	return 1;
}

// initialize all alpha colors (call at startup)
void old_alpha_colors_init()
{
	// See the variable declarations above for color usage
	gr_init_alphacolor( &Color_blue, 93, 93, 128, 255 );
	gr_init_alphacolor( &Color_bright_blue, 185, 185, 255, 255 );

	gr_init_alphacolor( &Color_green, 0, 120, 0, 255 );
	gr_init_alphacolor( &Color_bright_green, 50, 190, 50, 255 );

	gr_init_alphacolor( &Color_black, 0, 0, 0, 255 );
	gr_init_alphacolor( &Color_grey, 65, 65, 65, 255 );
	gr_init_alphacolor( &Color_silver, 160, 160, 160, 255 );
	gr_init_alphacolor( &Color_white, 105, 105, 105, 255 );
	gr_init_alphacolor( &Color_bright_white, 255, 255, 255, 255 );

	gr_init_alphacolor( &Color_violet_gray, 160, 144, 160, 255 );
	gr_init_alphacolor( &Color_violet, 192, 104, 192, 255 );

	gr_init_alphacolor( &Color_dim_red, 80, 6, 6, 255 );
	gr_init_alphacolor( &Color_red, 126, 6, 6, 255 );
	gr_init_alphacolor( &Color_bright_red, 200, 0, 0, 255 );

	gr_init_alphacolor( &Color_pink, 185, 150, 150, 255 );
	gr_init_alphacolor( &Color_light_pink, 230, 190, 190, 255 );

	gr_init_alphacolor( &Color_yellow, 255, 255, 122, 255 );
	gr_init_alphacolor( &Color_bright_yellow, 255, 255, 0, 255 );

	gr_init_alphacolor( &Color_ui_light_green, 161, 184, 161, 255 );
	gr_init_alphacolor( &Color_ui_green, 190, 228, 190, 255 );

	gr_init_alphacolor( &Color_ui_light_pink, 184, 161, 161, 255 );
	gr_init_alphacolor( &Color_ui_pink, 228, 190, 190, 255 );
}
