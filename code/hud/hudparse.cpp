/* hudparse.cpp
*	Contains code to parse hud gauge locations
*/

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"

//ADDING A HUD GAUGE
//1) Add the X/Y variable as an int array in "Int Row"
//2) Add it to parse_resolution() with a stuff_coords()
//3) Add the default values to load_hud_defaults()
//4) Add any calculated values to the end of hud_positions_init
//	 If you want these values to be settable, init the array to {-1, -1} and use set_coords_if_clear
//5) Set any functions to use the array

//"Int Row"
//*Main Gauges*
//Hudshield
int Player_shield_coords[2];
int Target_shield_coords[2];
int Shield_mini_coords[2];
char Shield_mini_fname[MAX_FILENAME_LEN];
//Hudtarget
int Aburn_coords[2];
int Wenergy_coords[2];
int Aburn_width;
int Wenergy_width;
char Aburn_fname[MAX_FILENAME_LEN];
char Wenergy_fname[MAX_FILENAME_LEN];

//*Gauges*
//Shield mini text
int Hud_mini_3digit[2];
int Hud_mini_2digit[2] = {-1, -1};
int Hud_mini_1digit[2] = {-1, -1};

void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y = -1)
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

//This doesn't belong in parse_lo, it's not really that low.
static int size_temp[2];
static float percentage_temp[2];
int stuff_coords(char* pstr, int* i, char* image = NULL, int* size_x = NULL, int* size_y = NULL, bool required = false)
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
		percentage_temp[0] *= gr_screen.max_w / 100;
		percentage_temp[1] *= gr_screen.max_h / 100;

		//Our, uh, bool is true; size is loaded
		if(!pstr)
		{
			percentage_temp[0] -= size_temp[0] / 2;
			percentage_temp[1] -= size_temp[1] / 2;
		}
		i[0] += fl2i(percentage_temp[0]);
		i[1] += fl2i(percentage_temp[1]);
	}

	if(optional_string("+Image:") && image)
	{
		stuff_string(image, F_NAME, NULL);
	}
	return 1;
}

void parse_resolution()
{
	stuff_coords("$Player Shield:", Player_shield_coords);
	stuff_coords("$Target Shield:", Target_shield_coords);
	stuff_coords("$Shield Mini:", Shield_mini_coords);
	stuff_coords("$Afterburner Energy:", Aburn_coords, Aburn_fname, &Aburn_width);
	stuff_coords("$Weapons Energy:", Wenergy_coords, Wenergy_fname, &Wenergy_width);
}
void parse_resolution_gauges()
{
	char gaugename[32];
	while(!required_string_3("$Gauge:","$Resolution:","#End"))
	{
		required_string("$Gauge:");
		stuff_string(gaugename, F_NAME, NULL);
		if(!stricmp(gaugename, "Shield Mini"))
		{
			if(stuff_coords("$Text Base:", Hud_mini_3digit))
			{
				nprintf(("WARNING", "I READ A TEXT BASE OMG WTF LOL"));
			}
			stuff_coords("$Text 1 digit:", Hud_mini_1digit);
			stuff_coords("$Text 2 digit:", Hud_mini_2digit);
		}
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

	//It's parsing time everehbody!
	int buffer[2];
	//Parse main gauges
	if(optional_string("#Main Gauges"))
	{
		while (required_string_either("#End","$Resolution:"))
		{
			required_string("$Resolution:");
			stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);
			if((buffer[0] == gr_screen.max_w && buffer[1] == gr_screen.max_h) || (buffer[0] == 0 && buffer[1] == 0))
			{
				parse_resolution();
			}
			else
			{
				skip_to_string("$Resolution:", "#End");
			}
		}
		required_string("#End");
	}

	//Parse individual extra gauge info
	if(optional_string("#Gauges"))
	{
		while(required_string_either("#End","$Resolution:"))
		{
			required_string("$Resolution:");
			stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);
			if((buffer[0] == gr_screen.max_w && buffer[1] == gr_screen.max_h) || (buffer[0] == 0 && buffer[1] == 0))
			{
				parse_resolution_gauges();
			}
			else
			{
				skip_to_string("$Resolution:", "#End");
			}
		}
		required_string("#End");
	}
	lcl_ext_close();

	return 1;
}

static void load_hud_defaults()
{
	//X values
	if(gr_screen.max_w == 640)
	{
		Player_shield_coords[0] = 396;
		Target_shield_coords[0] = 142;
		Shield_mini_coords[0] = 305;
		Hud_mini_3digit[0] = 310;
		Aburn_coords[0] = 171;
		Wenergy_coords[0] = 416;

		Aburn_width = Wenergy_width = 60;

		strcpy(Aburn_fname, "energy2");
		strcpy(Wenergy_fname, "energy2");
	}
	else
	{
		int resize = gr_screen.max_w / 1024;
		Player_shield_coords[0] = 634 * resize;
		Target_shield_coords[0] = 292 * resize;
		Shield_mini_coords[0] = 497 * resize;
		Hud_mini_3digit[0] = 502 * resize;
		Aburn_coords[0] = 274 * resize;
		Wenergy_coords[0] = 666 * resize;

		Aburn_width = Wenergy_width = 96;

		strcpy(Aburn_fname, "2_energy2");
		strcpy(Wenergy_fname, "2_energy2");
	}
	

	//Y values
	if(gr_screen.max_h == 480)
	{
		Player_shield_coords[1] = 379;
		Target_shield_coords[1] = 379;
		Shield_mini_coords[1] = 291;
		Hud_mini_3digit[1] = 298;
		Aburn_coords[1] = Wenergy_coords[1] = 265;
	}
	else
	{
		int resize = gr_screen.max_h / 768;
		Player_shield_coords[1] = 670 * resize;
		Target_shield_coords[1] = 670 * resize;
		Shield_mini_coords[1] = 470 * resize;
		Hud_mini_3digit[1] = 477 * resize;
		Aburn_coords[1] = Wenergy_coords[1] = 424 * resize;
	}

	//Neither
	strcpy(Shield_mini_fname, "targhit1");
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

	//Calculate HUD gauges info
	//Shield mini text
	set_coords_if_clear(Hud_mini_2digit, Hud_mini_3digit[0] + 2, Hud_mini_3digit[1]);
	set_coords_if_clear(Hud_mini_1digit, Hud_mini_3digit[0] + 6, Hud_mini_3digit[1]);
}