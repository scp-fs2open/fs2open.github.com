#include "globalincs/globals.h"

//Teh hud_info struct...maybe class
#define MAX_CUSTOM_HUD_GAUGES 32
#define MAX_HUD_GAUGE_TYPES 64

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
	int custom_gauge_frames[MAX_CUSTOM_HUD_GAUGES];
	char custom_gauge_images[MAX_CUSTOM_HUD_GAUGES][MAX_FILENAME_LEN];
	char custom_gauge_text[MAX_CUSTOM_HUD_GAUGES][NAME_LENGTH];

//	int gauge_text_sexp_vars[MAX_HUD_GAUGE_TYPES];
//	int gauge_frame_sexp_vars[MAX_HUD_GAUGE_TYPES];

	hud_info();
} hud_info;

typedef struct gauge_info
{
	gauge_info* parent;	//Parent, used for mini-gauges, pointer to gauge_info (NULL if main gauge
	size_t coord_dest;	//Offset of coord in hud_info
	char fieldname[NAME_LENGTH];	//TBL entry token
	int defaultx_640;	//Default 640x480 x coord
	int defaulty_480;	//y coord
	int defaultx_1024;	//Default 1024x768 x coord
	int defaulty_768;	//y coord
	size_t size_dest;	//offset of size coord in hud_info; init in load_hud_defaults() (Can be NULL)
	size_t image_dest;	//offset of image string in hud_info; init in load_hud_defaults() (Can be NULL)
	size_t frame_dest;	//Storage spot for frame info
	size_t text_dest;	//Text value
	int show_outside;	//Show outside ship?
} gauge_info;

//Macros to get HUD gauge values
#define HUD_INT(a, b) ((int*)((char*)a + b))
#define HUD_CHAR(a, b) ((char *)((char*)a + b))

//Variables
extern int Num_custom_gauges;

//Functions
int hud_get_gauge_index(char* name);
gauge_info* hud_get_gauge(char* name);
void hud_positions_init();
void set_current_hud(int player_ship_num);