/* hudparse.cpp
*	Contains code to parse hud gauge locations
*/
#include <stddef.h>

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"
#include "ship/ship.h"
#include "hud/hudparse.h"
#include "hud/hud.h"
#include "hud/hudescort.h"



//Global stuffs
hud_info* current_hud;
hud_info default_hud;
hud_info ship_huds[MAX_SHIP_TYPES];
extern int ships_inited; //Need this

//Set coord_x or coord_y to -1 to not change that value
//void resize_coords(int* values, float* factors);
//void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y = -1);

//ADD YOUR VARIABLES HERE
//Gauges MUST come first, and all variables MUST be in the hud struct.

//Use this when setting gauge variables. It gets the OFFSET of the value in the hud_info struct
#define HUD_VAR(a) offsetof(hud_info, a)

gauge_info gauges[MAX_HUD_GAUGE_TYPES] = {
{NULL,			HUD_VAR(Player_shield_coords),	"$Player Shield:",			396, 379, 634, 670,	NULL, NULL, NULL, NULL, NULL},
{NULL,			HUD_VAR(Target_shield_coords),	"$Target Shield:",			142, 379, 292, 670,	NULL, NULL, NULL, NULL, NULL},
{NULL,			HUD_VAR(Shield_mini_coords),	"$Shield Mini:",			305, 291, 497, 470, NULL, HUD_VAR(Shield_mini_fname), NULL, NULL, NULL},
{NULL,			HUD_VAR(Aburn_coords),			"$Afterburner Energy:",		171, 265, 274, 424, HUD_VAR(Aburn_size) ,HUD_VAR(Aburn_fname), NULL, NULL, NULL},
{NULL,			HUD_VAR(Wenergy_coords),		"$Weapons Energy:",			416, 265, 666, 424, HUD_VAR(Wenergy_size) ,HUD_VAR(Wenergy_fname), NULL, NULL, NULL},
{NULL,			HUD_VAR(Escort_coords),			"$Escort List:",			486, 206, 865, 330, NULL, HUD_VAR(Escort_filename[0]), NULL, HUD_VAR(Escort_htext), NULL},
//Mini-gauges
{&gauges[2],	HUD_VAR(Hud_mini_3digit),		"$Text Base:",				5, 7, 5, 7,			NULL},
{&gauges[2],	HUD_VAR(Hud_mini_1digit),		"$Text 1 digit:",			11, 7, 11, 7,		NULL},
{&gauges[2],	HUD_VAR(Hud_mini_2digit),		"$Text 2 digit:",			7, 7, 7, 7,			NULL},
{&gauges[5],	HUD_VAR(Escort_htext_coords),	"$Header Text:",			3, 2, 4, 1,			NULL},
{&gauges[5],	HUD_VAR(Escort_list),			"$List:",					0, 12, 0, 13,		NULL, NULL, NULL, NULL, NULL, HG_NOADD},
{&gauges[5],	HUD_VAR(Escort_entry),			"$Ship:",					0, 11, 0, 11,		NULL, HUD_VAR(Escort_filename[1]), NULL, NULL, NULL, HG_NOADD},
{&gauges[5],	HUD_VAR(Escort_entry_last),		"$Last Ship:",				0, 11, 0, 11,		NULL, HUD_VAR(Escort_filename[2]), NULL, NULL, NULL, HG_NOADD},
{&gauges[5],	HUD_VAR(Escort_name),			"$Ship Name:",				3, 0, 4, 0,			NULL, NULL, NULL, NULL, NULL, HG_NOADD},
{&gauges[5],	HUD_VAR(Escort_integrity),		"$Ship Hull:",				128, 0, 116, 0,		NULL, NULL, NULL, NULL, NULL, HG_NOADD},
{&gauges[5],	HUD_VAR(Escort_status),			"$Ship Status:",			-12, 0, -11, 0,		NULL, NULL, NULL, NULL, NULL, HG_NOADD}
};

//Number of gauges
int Num_gauge_types = sizeof(gauges)/sizeof(gauge_info);
int Num_custom_gauges = 0;

//Loads defaults for if a hud isn't specified in the table
static void load_hud_defaults(hud_info *hud)
{
	//X values
	if(gr_screen.max_w == 640)
	{
		//Size defaults
		hud->Aburn_size[0] = hud->Wenergy_size[0] = 60;

		//Image defaults
		strcpy(hud->Aburn_fname, "energy2");
		strcpy(hud->Wenergy_fname, "energy2");

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
		strcpy(hud->Aburn_fname, "2_energy2");
		strcpy(hud->Wenergy_fname, "2_energy2");

		/**************************************************/
		//DO NOT CHANGE
		hud->resolution[0] = 1024;
		hud->resolution[1] = 768;
		/**************************************************/
	}

	//Neither
	strcpy(hud->Shield_mini_fname, "targhit1");
	strcpy(hud->Escort_filename[0], "escort1");
	strcpy(hud->Escort_filename[1], "escort2");
	strcpy(hud->Escort_filename[2], "escort3");
	strcpy(hud->Escort_htext, XSTR( "monitoring", 285));

	/**************************************************/
	//NONE OF THIS NEEDS TO BE MODIFIED TO SETUP VARS
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(gr_screen.max_w == 640)
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

//This doesn't belong in parse_lo, it's not really that low.
static int size_temp[2];
static float percentage_temp[2];
int stuff_coords(hud_info* dest_hud, gauge_info* cg, bool required = false)
{
	//Speed up calculations
	static hud_info* factor_for_hud;
	static float resize_factor[2];
	float fl_buffer[2];
	bool size_defined = false;

	if(required)
	{
		required_string(cg->fieldname);
	}
	if(!required && !optional_string(cg->fieldname))
	{
		return 0;
	}

	//stuff_int_list(HUD_INT(dest_hud, i), 2, RAW_INTEGER_TYPE);
	stuff_float_list(fl_buffer, 2);
	if(!cg->parent)
	{
		factor_for_hud = NULL;
		resize_factor[0] = 1;
		resize_factor[1] = 1;
	}
	else if(dest_hud != factor_for_hud)
	{
		resize_factor[0] = (float)gr_screen.max_w / (float)dest_hud->resolution[0];
		resize_factor[1] = (float)gr_screen.max_h / (float)dest_hud->resolution[1];
	}
	//Resize to current res
	HUD_INT(dest_hud, cg->coord_dest)[0] = fl2i(fl_buffer[0] * resize_factor[0]);
	HUD_INT(dest_hud, cg->coord_dest)[1] = fl2i(fl_buffer[1] * resize_factor[1]);

	if(optional_string("+Size:"))
	{
		stuff_int_list(size_temp, 2, RAW_INTEGER_TYPE);
		if(cg->size_dest)
		{
			HUD_INT(dest_hud, cg->size_dest)[0] = size_temp[0];
			HUD_INT(dest_hud, cg->size_dest)[1] = size_temp[1];
		}
		
		//For %
		size_defined = false;
	}

	if(optional_string("+Percentage:"))
	{
		stuff_float_list(percentage_temp, 2);
		percentage_temp[0] *= (gr_screen.max_w / 100.0f);
		percentage_temp[1] *= (gr_screen.max_h / 100.0f);

		//Bool true, size defined
		if(!size_defined)
		{
			if(percentage_temp[0])
			{
				percentage_temp[0] -= size_temp[0] / 2;
			}
			if(percentage_temp[1])
			{
				percentage_temp[1] -= size_temp[1] / 2;
			}
		}
		HUD_INT(dest_hud, cg->coord_dest)[0] += fl2i(percentage_temp[0]);
		HUD_INT(dest_hud, cg->coord_dest)[1] += fl2i(percentage_temp[1]);
	}

	char buffer[32];
	if(optional_string("+Image:"))
	{
		if(cg->image_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->image_dest), F_NAME, NULL);
		}
		else
		{
			stuff_string(buffer, F_NAME, NULL);
		}
	}
	if(optional_string("+Text:"))
	{
		if(cg->text_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->text_dest),  F_NAME, NULL);
		}
		else
		{
			stuff_string(buffer, F_NAME, NULL);
		}
	}
	if(optional_string("+Color:"))
	{
		if(cg->color_dest)
		{
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->red);
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->green);
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->blue);
		}
		else
		{
			ubyte junk_byte;
			stuff_byte(&junk_byte);
			stuff_byte(&junk_byte);
			stuff_byte(&junk_byte);
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
		if(!cg->parent && cg->fieldname)
		{

			stuff_coords(dest_hud, cg);
		}
	}
}

static void parse_resolution_gauges(hud_info* dest_hud)
{
	char gaugename[32];
	gauge_info *cg, *parent;
	while(!required_string_3("$Gauge:","$Resolution:","#End"))
	{
		required_string("$Gauge:");
		stuff_string(gaugename, F_NAME, NULL);

		parent = NULL;

		for(int i = 0; i < Num_gauge_types; i++)
		{
			cg = &gauges[i];
			if(!parent)
			{
				if(!strnicmp(cg->fieldname + sizeof(char), gaugename, strlen(cg->fieldname) - 2))
				{
					parent = cg;
				}
			}
			else if(parent == cg->parent)
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
	hud_info* dest_hud;

	required_string("$Ship:");
	char shipname[NAME_LENGTH];
	int ship_index;
	stuff_string(shipname, F_NAME, NULL);
	ship_index = ship_info_lookup(shipname);

	if(ship_index == -1)
	{
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
		if(!dest_hud->loaded)
		{
			stuff_int_list(dest_hud->resolution, 2, RAW_INTEGER_TYPE);
			if(dest_hud->resolution[0] == 0 || dest_hud->resolution == 0)
			{
				dest_hud->resolution[0] = gr_screen.max_w;
				dest_hud->resolution[1] = gr_screen.max_h;
			}
			return dest_hud;
		}
	}
	else
	{
		required_string("$Resolution:");
		stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);

		if(buffer[0] == gr_screen.max_w && buffer[1] == gr_screen.max_h)
		{
			//Get the ship HUD ready w/ defaults
			if(default_hud.loaded)
			{
				memcpy(dest_hud, &default_hud, sizeof(hud_info));
				//It's not really loaded
				dest_hud->loaded = false;
			}
			else
			{
				//Set the resolution
				memcpy(dest_hud->resolution, buffer, sizeof(buffer));
			}

			return dest_hud;
		}
	}

	return 0;
}

void parse_custom_gauge()
{
	if(Num_gauge_types < MAX_HUD_GAUGE_TYPES)
	{
		char buffer[32];

		gauge_info* cg = &gauges[Num_gauge_types];
		memset(cg, 0, sizeof(gauge_info));
		//Set all the ptrs
		cg->coord_dest = HUD_VAR(custom_gauge_coords[Num_custom_gauges]);
		cg->size_dest = HUD_VAR(custom_gauge_sizes[Num_custom_gauges]);
		cg->image_dest = HUD_VAR(custom_gauge_images[Num_custom_gauges]);
		cg->frame_dest = HUD_VAR(custom_gauge_frames[Num_custom_gauges]);
		cg->text_dest = HUD_VAR(custom_gauge_text[Num_custom_gauges]);
		cg->color_dest = HUD_VAR(custom_gauge_colors[Num_custom_gauges]);

		required_string("$Name:");
		//Gotta make this a token
		cg->fieldname[0] = '$';
		stuff_string(cg->fieldname + 1, F_NAME, NULL);
		strcat(cg->fieldname, ":");

		if(optional_string("$Default640X:"))
		{
			stuff_int(&cg->defaultx_640);
		}
		if(optional_string("$Default640Y:"))
		{
			stuff_int(&cg->defaulty_480);
		}
		if(optional_string("$Default1024X:"))
		{
			stuff_int(&cg->defaultx_1024);
		}
		if(optional_string("$Default1024Y:"))
		{
			stuff_int(&cg->defaulty_768);
		}
		if(optional_string("$Parent:"))
		{
			stuff_string(buffer, F_NAME, NULL);
			cg->parent = hud_get_gauge(buffer);
		}

		Num_gauge_types++;
		Num_custom_gauges++;
	}
}

int parse_hud_gauges_tbl(char* longname)
{
	int rval;
	lcl_ext_open();
	if ((rval = setjmp(parse_abort)) != 0)
	{
		nprintf(("Warning", "Unable to parse %s!  Code = %i.\n", longname, rval));
		lcl_ext_close();
		return 0;
	}
	else
	{	
		read_file_text(longname);
		reset_parse();
	}

	read_file_text(longname);

	if(optional_string("$Max Escort Ships:"))
	{
		stuff_int(&Max_escort_ships);
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
				skip_to_start_of_strings("$Resolution:", "$Default:", "#End");
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
				skip_to_start_of_strings("$Resolution:", "$Default:", "#End");
			}
		}
		required_string("#End");
	}

	calculate_gauges(&default_hud);

	if(ships_inited)
	{
		//Parse main ship gauges
		if(optional_string("#Ship Main Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				if(dest_hud = parse_ship_start(), dest_hud)
				{
					while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
					{
						if(parse_resolution_start(dest_hud, rval))
						{
							parse_resolution(dest_hud);
						}
						else
						{
							skip_to_start_of_strings("$Resolution:", "$Default:", "$Ship:");
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
					while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
					{
						if(parse_resolution_start(dest_hud, rval))
						{
							parse_resolution_gauges(dest_hud);
						}
						else
						{
							skip_to_start_of_strings("$Resolution:", "$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		for(int i = 0; i < MAX_SHIP_TYPES; i++)
		{
			if(ship_huds[i].loaded)
			{
				calculate_gauges(&ship_huds[i]);
			}
		}
	}
	lcl_ext_close();

	return 1;
}

void hud_positions_init()
{
	load_hud_defaults(&default_hud);

	if(!parse_hud_gauges_tbl("hud_gauges.tbl"))
	{
		calculate_gauges(&default_hud);
	}

	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];

	int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, "*-hdg.tbm", CF_SORT_REVERSE);
	for(int i = 0; i < num_files; i++)
	{
		//HACK HACK HACK
		strcat(tbl_file_names[i], ".tbm");
		parse_hud_gauges_tbl(tbl_file_names[i]);
	}

	set_current_hud(-1);
}

//Depending on the ship number specified, this copies either the default hud or a ship hud
//to a temporary hud_info struct and sets the current_hud pointer to it.
//This enables ship-specific huds.
//Note that this creates a temporary current hud, so changes aren't permanently saved.
void set_current_hud(int player_ship_num)
{
	static hud_info real_current_hud;

	if(player_ship_num >= 0 && player_ship_num < MAX_SHIP_TYPES && ship_huds[player_ship_num].loaded)
	{
		memcpy(&real_current_hud, &ship_huds[player_ship_num], sizeof(hud_info));
	}
	else
	{
		memcpy(&real_current_hud, &default_hud, sizeof(hud_info));
	}

	current_hud = &real_current_hud;
}

hud_info::hud_info()
{
	//loaded = false;
	memset(this, 0, sizeof(hud_info));
}

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
		if(!strnicmp(gauges[i].fieldname + sizeof(char), name, strlen(gauges[i].fieldname) - 2))
		{
			return i;
		}
	}

	return -1;
}
