//Define or undefine NEW_HUD here so you don't have to recompile everything.
#define NEW_HUD

#include "PreProcDefines.h"
#pragma once
#include "globalincs/globals.h"
#include "hud/hudescort.h"
#include "ship/ai.h"

//Teh hud_info struct...maybe class
#define MAX_CUSTOM_HUD_GAUGES 32
#ifndef NEW_HUD
#define MAX_HUD_GAUGE_TYPES 64
#else
#define MAX_HUD_GAUGES			64
#define MAX_GAUGE_CHILDREN		8
#define MAX_GAUGE_FRAMES		8
#define MAX_GAUGE_IMAGES		2

//Return values for update funcs
#define HG_RETURNNODRAW			-1
#define HG_RETURNLOOPUPDATE		0
#define HG_RETURNLASTUPDATE		1
#define HG_RETURNREDRAW			2

//Gauge types
#define HG_UNUSED			0
#define HG_MAINGAUGE		1
#define HG_CHILDGAUGE		2

//Data flags
#define HG_NEEDSUPDATE		(1<<0)
#define HG_ALWAYSUPDATE		(1<<1)
#define HG_PRIORITYCOLOR	(1<<2)

struct ship;
class gauge_data;

class gauge_data
{
private:
	//nonperishable stuff
	char name[NAME_LENGTH];								//Gauge name - nonperishable after setup

	//Set by update
	int draw_coords[2];									//Coordinates the gauge is drawn at
	int update_num;										//Number of times the gauge has been updated

	char image[MAX_FILENAME_LEN];						//Image displayed for the gauge
	int image_id;										//ID of image
	int num_frames;										//Number of frames the image has

	int shape_id;										//Index of shape to draw
	int shape_attrib[8];								//Attributes of shape

	char model[MAX_FILENAME_LEN];						//Name of model to draw
	int model_id;										//Model id
public:
	int type;											//Basically, main gauge or child gauge?
	int coords[2];										//Coordinates of the gauge
	gauge_data *children[MAX_GAUGE_CHILDREN];				//Children of the gauge
	int (*update_func)(gauge_data* cg, ship* gauge_owner);//Function that sets data, maybe performs esoteric display functions

	int image_size[2];									//Size of the image

	int frame[MAX_GAUGE_FRAMES];						//Which image frame to use
	int frame_info[MAX_GAUGE_FRAMES][8];				//Info for each frame; 0-top clip, 1-left clip, 2-bottom clip, 3-right clip, 4-alpha

	char text[MESSAGE_LENGTH];							//Text for the gauge (Displayed under image)
//	int max_text_width;									//Maximum length in pixels text can be

	color g_color;										//Gauge's default color; no alpha means it uses hud default
	color user_color;									//Default user color; no alpha means no setting

	int model_attrib[8];								//Attributes of model

	int user_flags;										//Flags settable by the user
	int show_flags;										//Flags that set where the gauge is viewable
	int data_flags;										//Flags that provide additional info on the data
public:
	gauge_data();										//Creates the gauge

	//Info
	int get_update_num(){return update_num;}			//Current iteration number, so you can output a list of items

	//Level paging
	int page_in();										//Pages in images and stuff for the current hud gauge
	int reset();										//Call before a level to reset everything to defaults (incomplete)

	//In-mission functions
	int update(ship* gauge_owner, gauge_data* cgp = NULL);	//Updates the gauge and all children
	int draw();												//Draws the gauge; shouldn't need to be called outside of update
	int set_up(char* name, int def_x, int def_y, gauge_data* new_parent = NULL);		//Inits a gauge with data and readies it for use
	int set_image(char* image_name, int newframe = 0, int sizex = 0, int sizey = 0);	//Sets the current image
	int set_model(char* model_name);	//Sets the current model (incomplete)
};

#define HI_DIM		0
#define HI_NORMAL	1
#define HI_BRIGHT	2

#endif

typedef struct hud_info
{
	bool loaded;
	int resolution[2];
#ifdef NEW_HUD
	int num_gauges;
	gauge_data gauges[MAX_HUD_GAUGES];
	//Who's hud is this??
	ship *owner;
	color hud_color;
#endif
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
	//Hudescort
	int Escort_coords[2];

	//*Gauges*
	//Shield mini text
	int Hud_mini_3digit[2];
	int Hud_mini_2digit[2];
	int Hud_mini_1digit[2];
	//Escort list
	int Escort_htext_coords[2];
	char Escort_htext[NAME_LENGTH];
	int Escort_list[2];
	int Escort_entry[2];
	int Escort_entry_last[2];
	int Escort_name[2];
	int Escort_integrity[2];
	int Escort_status[2];
	char Escort_filename[NUM_ESCORT_FRAMES][MAX_FILENAME_LEN];

	int custom_gauge_coords[MAX_CUSTOM_HUD_GAUGES][2];
	int custom_gauge_sizes[MAX_CUSTOM_HUD_GAUGES][2];
	char custom_gauge_images[MAX_CUSTOM_HUD_GAUGES][MAX_FILENAME_LEN];
	int custom_gauge_frames[MAX_CUSTOM_HUD_GAUGES];
	char custom_gauge_text[MAX_CUSTOM_HUD_GAUGES][NAME_LENGTH];
	color custom_gauge_colors[MAX_CUSTOM_HUD_GAUGES];

//	int gauge_text_sexp_vars[MAX_HUD_GAUGE_TYPES];
//	int gauge_frame_sexp_vars[MAX_HUD_GAUGE_TYPES];

	hud_info();
#ifdef NEW_HUD
	ubyte get_preset_alpha(int preset_id);
	gauge_data* add_gauge(char* name, int def_x_coord, int def_y_coord, gauge_data* new_gauge_parent = NULL);
#endif
} hud_info;

typedef struct gauge_info
{
	gauge_info* parent;	//Parent, used for mini-gauges, pointer to gauge_info (NULL if main gauge
	size_t coord_dest;	//Offset of coord in hud_info
	char fieldname[NAME_LENGTH];	//TBL entry token
	int defaultx_640;	//Default 640x480 x coord
	int defaulty_480;	//Default 640x480 y coord
	int defaultx_1024;	//Default 1024x768 x coord
	int defaulty_768;	//Default 1024x768 y coord
	size_t size_dest;	//offset of size coord in hud_info; init in load_hud_defaults() (Can be NULL)
	size_t image_dest;	//offset of image string in hud_info; init in load_hud_defaults() (Can be NULL)
	size_t frame_dest;	//Storage spot for frame info
	size_t text_dest;	//Storage spot for text value
	size_t color_dest;	//Storage spot for color value
	int placement_flags;
	int show_flags;	//Show outside ship?
	//int (*update_gauge)(gauge_info* cg);	//Function to update the gauge
} gauge_info;

//Flags
#define HG_NOADD	(1<<0)

//Macros to get HUD gauge values
#define HUD_INT(a, b) ((int*)((char*)a + b))
#define HUD_CHAR(a, b) ((char *)((char*)a + b))
#define HUD_COLOR(a, b) ((color *)((char*)a + b))

//Variables
extern int Num_custom_gauges;

//Functions
int hud_get_gauge_index(char* name);
gauge_info* hud_get_gauge(char* name);
void hud_positions_init();
#ifdef NEW_HUD
extern ship* Player_ship;
void set_current_hud(ship* owner = Player_ship);
#else
void set_current_hud(int player_ship_num);
#endif