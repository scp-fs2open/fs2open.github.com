/* hudparse.cpp
*	Contains code to parse hud gauge locations
*/
#include <stddef.h>

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"
#include "ship/ship.h"
#include "hud/hudparse.h"

//Global stuffs
hud_info* current_hud;
hud_info default_hud;
hud_info ship_huds[MAX_SHIP_TYPES];
extern int ships_inited; //Need this

//Set coord_x or coord_y to -1 to not change that value
void resize_coords(int* values, float* factors);
void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y = -1);
int stuff_coords(char* pstr, int* i, char* image = NULL, int* size_x = NULL, int* size_y = NULL, bool required = false);

//Set up coord info array
typedef struct coord_info
{
	coord_info* parent;	//Parent, used for mini-gauges, pointer to coord_info (NULL if main gauge
	size_t variable;	//Offset of coord in hud_info
	char* fieldname;	//TBL entry token
	int defaultx_640;	//Default 640x480 x coord
	int defaulty_480;	//y coord
	int defaultx_1024;	//Default 1024x768 x coord
	int defaulty_768;	//y coord
	size_t size_dest;	//offset of size coord in hud_info; init in load_hud_defaults() (Can be NULL)
	size_t image_dest;	//offset of image string in hud_info; init in load_hud_defaults() (Can be NULL)
	bool addparent;		//For mini gauges, add parent coordinates in calculations?
} coord_info;

//ADD YOUR VARIABLES HERE
//Gauges MUST come first, and all variables MUST be in the hud struct.
#define MAX_COORD_TYPES 8

#define HUD_VAR(a) offsetof(hud_info, a)
coord_info gauges[MAX_COORD_TYPES] = {
{NULL,			HUD_VAR(Player_shield_coords),	"$Player Shield:",			396, 379, 634, 670,	NULL, NULL, false},
{NULL,			HUD_VAR(Target_shield_coords),	"$Target Shield:",			142, 379, 292, 670,	NULL, NULL, false},
{NULL,			HUD_VAR(Shield_mini_coords),		"$Shield Mini:",			305, 291, 497, 470, NULL, HUD_VAR(Shield_mini_fname), false},
{NULL,			HUD_VAR(Aburn_coords),			"$Afterburner Energy:",		171, 265, 274, 424, HUD_VAR(Aburn_size) ,HUD_VAR(Aburn_fname), false},
{NULL,			HUD_VAR(Wenergy_coords),			"$Weapons Energy:",			416, 265, 666, 424, HUD_VAR(Wenergy_size) ,HUD_VAR(Wenergy_fname), false},
//Mini-gauges
{&gauges[2],	HUD_VAR(Hud_mini_3digit),		"$Text Base:",				310, 298, 502, 477,	NULL, NULL, false},
{&gauges[2],	HUD_VAR(Hud_mini_1digit),		"$Text 1 digit:",			6, 0, 6, 0,	NULL, NULL, true},
{&gauges[2],	HUD_VAR(Hud_mini_2digit),		"$Text 2 digit:",			2, 0, 2, 0,	NULL, NULL, true}};
//#define HUD_INT(a, b) ((int *)((int*)a + b))
#define HUD_CHAR(a, b) ((char *)((char*)a + b))

int* HUD_INT(hud_info* a, size_t b)
{
	return (int*)((char*)a + b);
}

//Loads defaults for if a hud isn't specified in the table
static void load_hud_defaults()
{
	coord_info* cg;
	for(int i = 0; i < MAX_COORD_TYPES; i++)
	{
		cg = &gauges[i];
		if(cg->parent && cg->fieldname)
		{
			HUD_INT(&default_hud, cg->variable)[0] = -1;
			HUD_INT(&default_hud, cg->variable)[1] = -1;
		}
		else if(gr_screen.max_w == 640)
		{
			HUD_INT(&default_hud, cg->variable)[0] = cg->defaultx_640;
			HUD_INT(&default_hud, cg->variable)[1] = cg->defaulty_480;
		}
		else
		{
			HUD_INT(&default_hud, cg->variable)[0] = cg->defaultx_1024;
			HUD_INT(&default_hud, cg->variable)[1] = cg->defaulty_768;
		}
	}
	//X values
	if(gr_screen.max_w == 640)
	{
		//Size defaults
		default_hud.Aburn_size[0] = default_hud.Wenergy_size[0] = 60;

		//Image defaults
		strcpy(default_hud.Aburn_fname, "energy2");
		strcpy(default_hud.Wenergy_fname, "energy2");

		//Don't change
		default_hud.resolution[0] = 640;
		default_hud.resolution[0] = 480;
	}
	else
	{
		//Size defaults
		default_hud.Aburn_size[0] = default_hud.Wenergy_size[0] = 96;

		//Image defaults
		strcpy(default_hud.Aburn_fname, "2_energy2");
		strcpy(default_hud.Wenergy_fname, "2_energy2");

		//Don't change
		default_hud.resolution[0] = 1024;
		default_hud.resolution[1] = 768;
	}

	//Neither
	strcpy(default_hud.Shield_mini_fname, "targhit1");
}

/* You shouldn't have to modify anything past here to add gauges */

static void parse_resolution(hud_info* dest_hud)
{
	//Parse it
	coord_info* cg;
	for(int i = 0; i < MAX_COORD_TYPES; i++)
	{
		cg = &gauges[i];
		if(!cg->parent && cg->fieldname)
		{
			stuff_coords(cg->fieldname, HUD_INT(dest_hud, cg->variable), HUD_CHAR(dest_hud, cg->image_dest), HUD_INT(dest_hud, cg->size_dest));
		}
	}
}

static void parse_resolution_gauges(hud_info* dest_hud)
{
	char gaugename[32];
	coord_info *cg, *parent;
	while(!required_string_3("$Gauge:","$Resolution:","#End"))
	{
		required_string("$Gauge:");
		stuff_string(gaugename, F_NAME, NULL);

		parent = NULL;

		for(int i = 0; i < MAX_COORD_TYPES; i++)
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
				stuff_coords(cg->fieldname, 
					HUD_INT(dest_hud, cg->variable), 
					HUD_CHAR(dest_hud, cg->image_dest), 
					HUD_INT(dest_hud, cg->image_dest));
			}
		}
	}

	dest_hud->loaded = true;
}

static void calculate_gauges(hud_info* dest_hud)
{
	//Resizing section
	float resize[2];
	if(gr_screen.max_w != dest_hud->resolution[1] || gr_screen.max_w != dest_hud->resolution[0])
	{
		//Don't touch
		resize[0] = ((float)gr_screen.max_w / (float)dest_hud->resolution[0]);
		resize[1] = ((float)gr_screen.max_h / (float)dest_hud->resolution[1]);

		int resize_x, resize_y;
		coord_info* cg;
		for(int i = 0; i < MAX_COORD_TYPES; i++)
		{
			cg = &gauges[i];
			if(!cg->parent)
			{
				//resize_coords(HUD_INT(dest_hud, cg->variable), resize);
			}
			else
			{
				if(cg->addparent)
				{
					resize_x = HUD_INT(dest_hud, cg->variable)[0] + HUD_INT(dest_hud, cg->parent->variable)[0];
					resize_y = HUD_INT(dest_hud, cg->variable)[1] + HUD_INT(dest_hud, cg->parent->variable)[1];
				}
				else
				{
					resize_x = HUD_INT(dest_hud, cg->variable)[0];
					resize_y = HUD_INT(dest_hud, cg->variable)[1];
				}

				set_coords_if_clear(HUD_INT(dest_hud, cg->variable), resize_x, resize_y);
			}
		}
	}
}

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
}

//This doesn't belong in parse_lo, it's not really that low.
static int size_temp[2];
static float percentage_temp[2];
int stuff_coords(char* pstr, int* i, char* image, int* size_x, int* size_y, bool required)
{
	if(required)
	{
		required_string(pstr);
	}
	if(!required && !optional_string(pstr))
	{
		return 0;
	}

	stuff_int_list(i, 2, RAW_INTEGER_TYPE);

	if(optional_string("+Size:"))
	{
		stuff_int_list(size_temp, 2, RAW_INTEGER_TYPE);
		if(size_x)
		{
			*size_x = size_temp[0];
		}
		if(size_y)
		{
			*size_y = size_temp[1];
		}
		
		//Look! Now it's a bool!
		pstr = NULL;
	}

	if(optional_string("+Percentage:"))
	{
		stuff_float_list(percentage_temp, 2);
		percentage_temp[0] *= (gr_screen.max_w / 100.0f);
		percentage_temp[1] *= (gr_screen.max_h / 100.0f);

		//Our, uh, bool is true; size is loaded
		if(!pstr)
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
		i[0] += fl2i(percentage_temp[0]);
		i[1] += fl2i(percentage_temp[1]);
	}

	if(optional_string("+Image:"))
	{
		if(image)
		{
			stuff_string(image, F_NAME, NULL);
		}
		else
		{
			char buffer[32];
			stuff_string(buffer, F_NAME, NULL);
		}
	}
	return 1;
}

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

	//It's parsing time everehbody!
	hud_info* dest_hud = &default_hud;

	if(optional_string("#Main Gauges"))
	{
		rval = required_string_3("#End", "$Default:", "$Resolution:");
		while(rval)
		{
			dest_hud = parse_resolution_start(dest_hud, rval);
			if(dest_hud)
			{
				parse_resolution(dest_hud);
			}
			else
			{
				if(skip_to_string("$Resolution:", "#End") != 1)
				{
					skip_to_string("$Default:", "#End");
				}
			}
		}
		required_string("#End");
	}
	
	if(optional_string("#Gauges"))
	{
		rval = required_string_3("#End", "$Default:", "$Resolution:");
		while(rval)
		{
			dest_hud = parse_resolution_start(dest_hud, rval);
			if(dest_hud)
			{
				parse_resolution_gauges(dest_hud);
			}
			else
			{
				if(skip_to_string("$Resolution:", "#End") != 1)
				{
					skip_to_string("$Default:", "#End");
				}
			}
		}
		required_string("#End");
	}

	calculate_gauges(&default_hud);

	if(ships_inited)
	{
		//Parse main gauges
		if(optional_string("#Ship Main Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				dest_hud = parse_ship_start();
				rval = required_string_3("#End", "$Default:", "$Resolution:");
				while(rval)
				{
					dest_hud = parse_resolution_start(dest_hud, rval);
					if(dest_hud)
					{
						parse_resolution(dest_hud);
					}
					else
					{
						if(skip_to_string("$Resolution:", "$Ship:") != 1)
						{
							skip_to_string("$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		//Parse individual extra gauge info
		if(optional_string("#Ship Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				dest_hud = parse_ship_start();
				rval = required_string_3("#End", "$Default:", "$Resolution:");
				while(rval)
				{
					dest_hud = parse_resolution_start(dest_hud, rval);
					if(dest_hud)
					{
						parse_resolution_gauges(dest_hud);
					}
					else
					{
						if(skip_to_string("$Resolution:", "$Ship:") != 1)
						{
							skip_to_string("$Default:", "$Ship:");
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
	load_hud_defaults();

	parse_hud_gauges_tbl("hud_gauges.tbl");
	char tbl_files[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_files, NULL, CF_TYPE_TABLES, "*-hdg.tbm", CF_SORT_REVERSE);
	for(int i = 0; i < num_files; i++)
	{
		//HACK HACK HACK
		strcat(tbl_files[i], ".tbm");
		parse_hud_gauges_tbl(tbl_files[i]);
	}

	set_current_hud(-1);
}

void set_current_hud(int player_ship_num)
{
	if(player_ship_num >= 0 && player_ship_num < MAX_SHIP_TYPES && ship_huds[player_ship_num].loaded)
	{
		current_hud = &ship_huds[player_ship_num];
	}
	else
	{
		current_hud = &default_hud;
	}
}

hud_info::hud_info()
{
	loaded = 0;
}