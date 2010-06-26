/*
 * Created by WMCoolmon for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

 

#include <cstddef>

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"
#include "hud/hud.h"
#include "mission/missiontraining.h" //for Max_directives
#include "hud/hudescort.h"
//#include "weapon/emp.h"
#include "hud/hudparse.h" //Duh.
#include "ship/ship.h" //for ship struct
#include "graphics/font.h" //for gr_force_fit_string
#include "hud/hudtargetbox.h" 


//Global stuffs
hud_info *current_hud = NULL; //If not set, it's NULL. This should always be null outside of a mission.
bool Custom_gauge_images_loaded = false;
hud_info default_hud;
hud_info ship_huds[MAX_SHIP_CLASSES];
extern int ships_inited; //Need this
extern char Hud_Gauge_Names[NUM_HUD_GAUGES][NAME_LENGTH];

float Hud_unit_multiplier = 1.0f;	//Backslash
bool Hud_lead_alternate = false;

// Goober5000
int Hud_reticle_style = HUD_RETICLE_STYLE_FS2;

//Set coord_x or coord_y to -1 to not change that value
//void resize_coords(int* values, float* factors);
//void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y = -1);

//ADD YOUR VARIABLES HERE
//Gauges MUST come first, and all variables MUST be in the hud struct.

//Use this when setting gauge variables. It gets the OFFSET of the value in the hud_info struct
#define HUD_VAR(a) offsetof(hud_info, a)

gauge_info gauges[MAX_HUD_GAUGE_TYPES] = {
	{ NULL,			HUD_VAR(Player_shield_coords),	"Player Shield",		396, 379, 634, 670,	0, 0, 0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Target_shield_coords),	"Target Shield",		142, 379, 292, 670,	0, 0, 0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Shield_mini_coords),	"Shield Mini",			305, 291, 497, 470, 0, HUD_VAR(Shield_mini_fname), 0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Aburn_coords),			"Afterburner Energy",	171, 265, 274, 424, HUD_VAR(Aburn_size), HUD_VAR(Aburn_fname), 0, 0, 0, 0, HUD_VAR(Aburn_move_flag), -1, -1 },
	{ NULL,			HUD_VAR(Wenergy_coords),		"Weapons Energy",		416, 265, 666, 424, HUD_VAR(Wenergy_size), HUD_VAR(Wenergy_fname), 0, 0, 0, 0, HUD_VAR(Wenergy_move_flag), -1, -1 },
	{ NULL,			HUD_VAR(Wenergy_text_coords),	"Weapons Energy Text",	439, 318, 708, 509, 0, 0, 0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Escort_coords),			"Escort List",			486, 206, 865, 330, 0, HUD_VAR(Escort_filename[0]), 0, HUD_VAR(Escort_htext), 0, 0, 0, -1, -1 },

	//Mini-gauges
	{ &gauges[2],	HUD_VAR(Hud_mini_3digit),		"Text Base",			310, 298, 502, 477,	0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[2],	HUD_VAR(Hud_mini_1digit),		"Text 1 digit",			316, 298, 511, 477,	0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
//	{ &gauges[2],	HUD_VAR(Hud_mini_2digit),		"Text 2 digit",			213, 298, 346, 477,	0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[2],	HUD_VAR(Hud_mini_2digit),		"Text 2 digit",			313, 298, 506, 477,	0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_htext_coords),	"Header Text",			489, 208, 869, 331,	0, 0, 0, 0, 0, 0, 0, -1, -1 },
	{ &gauges[6],	HUD_VAR(Escort_list),			"List",					0, 12, 0, 13,		0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_entry),			"Ship",					0, 11, 0, 11,		0, HUD_VAR(Escort_filename[1]), 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_entry_last),		"Last Ship",			0, 11, 0, 11,		0, HUD_VAR(Escort_filename[2]), 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_name),			"Ship Name",			3, 0, 4, 0,			0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_integrity),		"Ship Hull",			128, 0, 116, 0,		0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[6],	HUD_VAR(Escort_status),			"Ship Status",			-12, 0, -11, 0,		0, 0, 0, 0, 0, 0, 0, HG_NOADD, -1 }
};

//Number of gauges
int Num_gauge_types = 17;
int Num_custom_gauges = 0;

static void load_hud_defaults(hud_info *hud)
{
	//X values
	if(gr_screen.res == GR_640)
	{
		//Size defaults
		hud->Aburn_size[0] = hud->Wenergy_size[0] = 60;

		//Image defaults
		strcpy_s(hud->Aburn_fname, "energy2");
		strcpy_s(hud->Wenergy_fname, "energy2");

		/**************************************************/
		//DO NOT CHANGE
		hud->resolution[0] = 640;
		hud->resolution[1] = 480;
		/**************************************************/
	}
	else
	{
		//Size defaults
		hud->Aburn_size[0] = hud->Wenergy_size[0] = 96;

		//Image defaults
		strcpy_s(hud->Aburn_fname, "2_energy2");
		strcpy_s(hud->Wenergy_fname, "2_energy2");

		/**************************************************/
		//DO NOT CHANGE
		hud->resolution[0] = 1024;
		hud->resolution[1] = 768;
		/**************************************************/
	}

	// Pan View Move Flag defaults
	hud->Aburn_move_flag = true;
	hud->Wenergy_move_flag = true;

	//Neither
	strcpy_s(hud->Shield_mini_fname, "targhit1");
	strcpy_s(hud->Escort_filename[0], "escort1");
	strcpy_s(hud->Escort_filename[1], "escort2");
	strcpy_s(hud->Escort_filename[2], "escort3");
	strcpy_s(hud->Escort_htext, XSTR( "monitoring", 285));

	/**************************************************/
	//NONE OF THIS NEEDS TO BE MODIFIED TO SETUP VARS
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(gr_screen.res == GR_640)
		{
			HUD_INT(hud, cg->coord_dest)[0] = cg->defaultx_640;
			HUD_INT(hud, cg->coord_dest)[1] = cg->defaulty_480;
		}
		else
		{
			HUD_INT(hud, cg->coord_dest)[0] = cg->defaultx_1024;
			HUD_INT(hud, cg->coord_dest)[1] = cg->defaulty_768;
		}
	}
}


static void calculate_gauges(hud_info* dest_hud)
{
	//Put any post-loading calculation code after the beep. *BEEP*

	// ok -- G5K
	if (Hud_reticle_style == HUD_RETICLE_STYLE_FS1)
	{
		if(gr_screen.res == GR_640)
		{
			//Image defaults
			strcpy_s(dest_hud->Aburn_fname, "energy2_fs1");
			strcpy_s(dest_hud->Wenergy_fname, "energy2_fs1");
		}
		else
		{
			//Image defaults
			strcpy_s(dest_hud->Aburn_fname, "2_energy2_fs1");
			strcpy_s(dest_hud->Wenergy_fname, "2_energy2_fs1");
		}
	}


	/**************************************************/
	//DO NOT MODIFY this stuff, unless you're changing the loading system.
	//Calculate parent gauge info
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(cg->parent && !(cg->placement_flags & HG_NOADD))
		{
			HUD_INT(dest_hud, cg->coord_dest)[0] = HUD_INT(dest_hud, cg->coord_dest)[0] + HUD_INT(dest_hud, cg->parent->coord_dest)[0];
			HUD_INT(dest_hud, cg->coord_dest)[1] = HUD_INT(dest_hud, cg->coord_dest)[1] + HUD_INT(dest_hud, cg->parent->coord_dest)[1];
		}
	}
}

/****************************************************************************************************/
/* You shouldn't have to modify anything past here to add gauges */
/****************************************************************************************************/
int stuff_coords(hud_info* dest_hud, gauge_info* cg, bool required = false)
{
	// Speed up calculations
	float percentage_temp[2];
	int size_temp[2];
	bool size_defined = false;
	char token_name[NAME_LENGTH + 2] = "$";

	strcat_s(token_name, cg->fieldname);
	strcat_s(token_name, ":");

	if(required)
	{
		required_string(token_name);
	}
	if(!required && !optional_string(token_name))
	{
		return 0;
	}

	stuff_int_list(HUD_INT(dest_hud, cg->coord_dest), 2, RAW_INTEGER_TYPE);

	if(optional_string("+Size:"))
	{
		stuff_int_list(size_temp, 2, RAW_INTEGER_TYPE);
		
		if(cg->size_dest)
		{
			HUD_INT(dest_hud, cg->size_dest)[0] = size_temp[0];
			HUD_INT(dest_hud, cg->size_dest)[1] = size_temp[1];
			
			size_defined = true;
		}
	}

	if(optional_string("+Percentage:"))
	{
		stuff_float_list(percentage_temp, 2);
		percentage_temp[0] *= (dest_hud->resolution[0] / 100.0f);
		percentage_temp[1] *= (dest_hud->resolution[1] / 100.0f);

		if(size_defined)
		{
			if(percentage_temp[0])
			{
				percentage_temp[0] -= fl2i(size_temp[0] / 2.0f);
			}
			if(percentage_temp[1])
			{
				percentage_temp[1] -= fl2i(size_temp[1] / 2.0f);
			}
		}

		HUD_INT(dest_hud, cg->coord_dest)[0] += fl2i(percentage_temp[0]);
		HUD_INT(dest_hud, cg->coord_dest)[1] += fl2i(percentage_temp[1]);
	}

	char buffer[MAX_FILENAME_LEN];
	if(optional_string("+Image:"))
	{
		if(cg->image_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->image_dest), F_NAME, MAX_FILENAME_LEN);
		}
		else
		{
			stuff_string(buffer, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("+Text:"))
	{
		if(cg->text_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->text_dest), F_NAME, NAME_LENGTH);
		}
		else
		{
			stuff_string(buffer, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("+Color:"))
	{
		if(cg->color_dest)
		{
			stuff_ubyte(&HUD_COLOR(dest_hud, cg->color_dest)->red);
			stuff_ubyte(&HUD_COLOR(dest_hud, cg->color_dest)->green);
			stuff_ubyte(&HUD_COLOR(dest_hud, cg->color_dest)->blue);
		}
		else
		{
			ubyte junk_byte;
			stuff_ubyte(&junk_byte);
			stuff_ubyte(&junk_byte);
			stuff_ubyte(&junk_byte);
		}
	}
	if(optional_string("+Inherit Color from:"))
	{
		*HUD_INT(dest_hud, cg->color_parent_dest) = -1;
		stuff_string(buffer, F_NAME, NAME_LENGTH);
		
		if(cg->color_parent_dest)
		{
			for (int idx = 0; idx < NUM_HUD_GAUGES; idx++)
			{
				if (stricmp(buffer, Hud_Gauge_Names[idx]) == 0)
				{
					*HUD_INT(dest_hud, cg->color_parent_dest) = idx;
					break;
				}
			}
		}
	}
	if(optional_string("+Move in Pan View:"))
	{
		if(cg->moveflag_dest)
		{
			stuff_boolean(HUD_BOOL(dest_hud, cg->moveflag_dest));
		}
		else
		{
			bool junk_bool;
			stuff_boolean(&junk_bool);
		}
	}
	return 1;
}


static void parse_resolution(hud_info* dest_hud)
{
	//Parse it
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(cg->parent == NULL && strlen(cg->fieldname))
		{
			stuff_coords(dest_hud, cg);
		}
	}

	dest_hud->loaded = true;
}

static void parse_resolution_gauges(hud_info* dest_hud)
{
	char gaugename[NAME_LENGTH];
	gauge_info *cg, *parent;
	while(!required_string_4("$Gauge:", "$Default:", "$Resolution:", "#End"))
	{
		required_string("$Gauge:");
		stuff_string(gaugename, F_NAME, NAME_LENGTH);

		parent = NULL;
		int i = 0;

		for(i = 0; i < Num_gauge_types; i++)
		{
			cg = &gauges[i];
			
			if(!parent && !stricmp(cg->fieldname, gaugename))
			{
				parent = cg;
				break;
			}
		}

		for(i = 0; i < Num_gauge_types; i++)
		{
			cg = &gauges[i];
			
			if(parent == cg->parent)
			{
				stuff_coords(dest_hud, cg);
			}
		}
	}

	dest_hud->loaded = true;
}


/*
void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y)
{
	if(dest_coords[0] == -1 && coord_x != -1)
	{
		dest_coords[0] = coord_x;
	}
	if(dest_coords[1] == -1 && coord_y != -1)
	{
		dest_coords[1] = coord_y;
	}
}


void resize_coords(int* values, float* factors)
{
	values[0] = fl2i(values[0] * factors[0]);
	values[1] = fl2i(values[1] * factors[1]);
}*/

hud_info* parse_ship_start()
{
	hud_info* dest_hud = NULL;

	required_string("$Ship:");
	char shipname[NAME_LENGTH];
	int ship_index;
	stuff_string(shipname, F_NAME, NAME_LENGTH);
	ship_index = ship_info_lookup(shipname);

	if(ship_index == -1)
	{
		WarningEx(LOCATION, "\"$Ship:\" name \"%s\" not found.", shipname);
		return NULL;
	}

	dest_hud = &ship_huds[ship_index];
	return dest_hud;
}

hud_info* parse_resolution_start(hud_info* dest_hud, int str_token)
{
	int buffer[2];

	if(str_token == 1)
	{
		required_string("$Default:");
		stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);

		if(buffer[0] == 0 || buffer == 0)
		{
			buffer[0] = gr_screen.max_w;
			buffer[1] = gr_screen.max_h;
		}
		
		// In case of a second $Default: declaration (in a tbm or a ship gauge perhaps),
		// check if it declares the same resolution as before. If not, reload defaults.
		if(dest_hud->loaded && (buffer[0] != dest_hud->resolution[0]) || (buffer[1] != dest_hud->resolution[1]))
		{
			load_hud_defaults(dest_hud);
		}
		
		//Set the resolution
		memcpy(dest_hud->resolution, buffer, sizeof(buffer));
		dest_hud->loaded = false;
		
		return dest_hud;
	}
	else
	{
		required_string("$Resolution:");
		stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);

		if ( (buffer[0] == gr_screen.max_w_unscaled) && (buffer[1] == gr_screen.max_h_unscaled) )
		{
			if(dest_hud->loaded)
			{
				load_hud_defaults(dest_hud);
			}
			
			//Set the resolution
			memcpy(dest_hud->resolution, buffer, sizeof(buffer));
			dest_hud->loaded = false;

			return dest_hud;
		}
	}

	return 0;
}


void parse_custom_gauge()
{
	if(Num_gauge_types < MAX_HUD_GAUGE_TYPES)
	{
		char buffer[NAME_LENGTH];

		gauge_info* cg = &gauges[Num_gauge_types];
		memset(cg, 0, sizeof(gauge_info));
		//Set all the ptrs (modified to work with GCC 3.4 - taylor)
		cg->coord_dest = HUD_VAR(custom_gauge_coords[0]) + (Num_custom_gauges * (2*sizeof(int)));
		cg->size_dest = HUD_VAR(custom_gauge_sizes[0]) + (Num_custom_gauges * (2*sizeof(int)));
		cg->image_dest = HUD_VAR(custom_gauge_images[0]) + (Num_custom_gauges * (MAX_FILENAME_LEN * sizeof(char)));
		cg->frame_dest = HUD_VAR(custom_gauge_frames[0]) + (Num_custom_gauges * sizeof(int));
		cg->text_dest = HUD_VAR(custom_gauge_text[0]) + (Num_custom_gauges * (NAME_LENGTH * sizeof(char)));
		cg->color_dest = HUD_VAR(custom_gauge_colors[0]) + (Num_custom_gauges * sizeof(color));
		cg->color_parent_dest = HUD_VAR(custom_gauge_color_parents[0]) + (Num_custom_gauges * sizeof(int));
		cg->moveflag_dest = HUD_VAR(custom_gauge_moveflags[0]) + (Num_custom_gauges * sizeof(bool));

		required_string("$Name:");
		stuff_string(cg->fieldname, F_NAME, NAME_LENGTH);

		if(optional_string("+Default640X:"))
		{
			stuff_int(&cg->defaultx_640);
		}
		if(optional_string("+Default640Y:"))
		{
			stuff_int(&cg->defaulty_480);
		}
		if(optional_string("+Default1024X:"))
		{
			stuff_int(&cg->defaultx_1024);
		}
		if(optional_string("+Default1024Y:"))
		{
			stuff_int(&cg->defaulty_768);
		}
		if(optional_string("+Parent:"))
		{
			stuff_string(buffer, F_NAME, NAME_LENGTH);
			cg->parent = hud_get_gauge(buffer);
			if (cg->parent == NULL) {
				WarningEx(LOCATION, "\"+Parent:\" HUD gauge \"%s\" not found!", buffer);
			}
		}

		Num_gauge_types++;
		Num_custom_gauges++;
	}
	else
	{
		skip_to_start_of_string_either("$Name:", "#End");
	}
}

void parse_hud_gauges_tbl(char *filename)
{
	int rval;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	if(optional_string("$Max Directives:"))
	{
		stuff_int(&Max_directives);
	}

	if(optional_string("$Max Escort Ships:"))
	{
		stuff_int(&Max_escort_ships);
	}

	if(optional_string("$Length Unit Multiplier:"))
	{
		stuff_float(&Hud_unit_multiplier);

		if (Hud_unit_multiplier <= 0.0f)
		{
			Warning(LOCATION, "\"$Length Unit Multiplier:\" value of \"%f\" is invalid!  Resetting to default.", Hud_unit_multiplier);
			Hud_unit_multiplier = 1.0f;
		}
	}

	if (optional_string("$Wireframe Targetbox:")) {
		stuff_int(&Targetbox_wire);
		if ((Targetbox_wire < 0) || (Targetbox_wire > 2)) {
			Targetbox_wire = 0;
		}
	}

	if (optional_string("$Lock Wireframe Mode:")) {
		stuff_boolean(&Lock_targetbox_mode);
	}

	if(optional_string("$Reticle Style:"))
	{
		int temp = required_string_either("FS1", "FS2");

		if (temp < 0)
			Warning(LOCATION, "Undefined reticle style in hud_gauges.tbl!");
		else
			Hud_reticle_style = temp;
	}

	if(optional_string("$Alternate Lead Indicator:"))
	{
		stuff_boolean(&Hud_lead_alternate);
	}

	if(optional_string("#Custom Gauges"))
	{
		while(required_string_either("#End", "$Name:"))
		{
			parse_custom_gauge();
		}
		required_string("#End");
	}

	//It's parsing time everehbody!
	hud_info* dest_hud = &default_hud;

	if(optional_string("#Main Gauges"))
	{
		while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
		{
			if(parse_resolution_start(dest_hud, rval))
			{
				parse_resolution(dest_hud);
			}
			else
			{
				skip_to_start_of_string_either("$Resolution:", "$Default:", "#End");
			}
		}
		required_string("#End");
	}
	
	if(optional_string("#Gauges"))
	{
		while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
		{
			if(parse_resolution_start(dest_hud, rval))
			{
				parse_resolution_gauges(dest_hud);
			}
			else
			{
				skip_to_start_of_string_either("$Resolution:", "$Default:", "#End");
			}
		}
		required_string("#End");
	}

	if(ships_inited)
	{
		//Parse main ship gauges
		if(optional_string("#Ship Main Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				if(dest_hud = parse_ship_start(), dest_hud)
				{
					// Copy defaults
					if(!dest_hud->loaded && default_hud.loaded)
					{
						memcpy(dest_hud, &default_hud, sizeof(hud_info));
					}

					while(rval = required_string_4("#End", "$Ship:", "$Default:", "$Resolution:"), rval > 1)
					{
						if(parse_resolution_start(dest_hud, rval - 1))
						{
							parse_resolution(dest_hud);
						}
						else
						{
							skip_to_start_of_string_either("$Resolution:", "$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		//Parse individual extra ship gauge info
		if(optional_string("#Ship Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				if(dest_hud = parse_ship_start(), dest_hud)
				{
					// Copy defaults
					if(!dest_hud->loaded && default_hud.loaded)
					{
						memcpy(dest_hud, &default_hud, sizeof(hud_info));
					}

					while(rval = required_string_4("#End", "$Ship:", "$Default:", "$Resolution:"), rval > 1)
					{
						if(parse_resolution_start(dest_hud, rval - 1))
						{
							parse_resolution_gauges(dest_hud);
						}
						else
						{
							skip_to_start_of_string_either("$Resolution:", "$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		for(int i = 0; i < MAX_SHIP_CLASSES; i++)
		{
			if(ship_huds[i].loaded)
			{
				calculate_gauges(&ship_huds[i]);
			}
		}
	}

	calculate_gauges(&default_hud);

	// close localization
	lcl_ext_close();
}

extern int Ships_inited;
void hud_positions_init()
{
	if(!ships_inited)
	{
		Error(LOCATION, "Could not initialize hudparse.cpp as ships were not inited first.");
		return;
	}

	load_hud_defaults(&default_hud);

	if (cf_exists_full("hud_gauges.tbl", CF_TYPE_TABLES))
		parse_hud_gauges_tbl("hud_gauges.tbl");
	else
		calculate_gauges(&default_hud);

	parse_modular_table(NOX("*-hdg.tbm"), parse_hud_gauges_tbl);

	set_current_hud(-1);
}

//Depending on the ship number specified, this copies either the default hud or a ship hud
//to a temporary hud_info struct and sets the current_hud pointer to it.
//This enables ship-specific huds.
//Note that this creates a temporary current hud, so changes aren't permanently saved.
void set_current_hud(int player_ship_num)
{
	static hud_info real_current_hud;

	if(player_ship_num >= 0 && player_ship_num < MAX_SHIP_CLASSES && ship_huds[player_ship_num].loaded)
	{
		memcpy(&real_current_hud, &ship_huds[player_ship_num], sizeof(hud_info));
	}
	else
	{
		memcpy(&real_current_hud, &default_hud, sizeof(hud_info));
	}	

	current_hud = &real_current_hud;	
	Custom_gauge_images_loaded = false;
}

/* - not POD so GCC won't take it for offsetof - taylor
hud_info::hud_info()
{
	memset(this, 0, sizeof(hud_info));
}
*/

gauge_info* hud_get_gauge(char* name)
{
	int id = hud_get_gauge_index(name);
	if(id > -1)
	{
		return &gauges[id];
	}

	return NULL;
}

int hud_get_gauge_index(char* name)
{
	for(int i = 0; i < Num_gauge_types; i++)
	{
		if(!stricmp(gauges[i].fieldname, name))
		{
			return i;
		}
	}

	return -1;
}
