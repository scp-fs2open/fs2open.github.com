#include "globalincs/globals.h"

//Teh hud_info struct...maybe class
#define MAX_CUSTOM_HUD_GAUGES 32

typedef struct hud_info
{
	bool loaded;
	int resolution[2];

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
	int Aburn_size[2];
	int Wenergy_size[2];
	char Aburn_fname[MAX_FILENAME_LEN];
	char Wenergy_fname[MAX_FILENAME_LEN];

	//*Gauges*
	//Shield mini text
	int Hud_mini_3digit[2];
	int Hud_mini_2digit[2];
	int Hud_mini_1digit[2];

	int custom_gauge_coords[MAX_CUSTOM_HUD_GAUGES][2];
	int custom_gauge_sizes[MAX_CUSTOM_HUD_GAUGES][2];
	char custom_gauge_images[MAX_CUSTOM_HUD_GAUGES][MAX_FILENAME_LEN];
	char custom_gauge_text[MAX_CUSTOM_HUD_GAUGES][NAME_LENGTH];

	hud_info();
} hud_info;

//Variables
extern int Num_custom_gauges;

//Functions
void hud_positions_init();
void set_current_hud(int player_ship_num);