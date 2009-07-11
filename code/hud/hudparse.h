



#ifndef _HUDPARSE_H
#define _HUDPARSE_H

//Define or undefine NEW_HUD here so you don't have to recompile everything.
//#define NEW_HUD

#include "globalincs/globals.h"
#include "hud/hudescort.h"
#include "ai/ai.h"

//Teh hud_info struct...maybe class
#define MAX_CUSTOM_HUD_GAUGES 32
#ifdef NEW_HUD
#define MAX_HUD_GAUGES			64
#define MAX_GAUGE_CHILDREN		8
#define MAX_GAUGE_FRAMES		8
#define MAX_GAUGE_SHAPES		8

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

//Shape types
#define HG_SHAPERECTANGLE		0
#define HG_SHAPECIRCLE			1
#define HG_SHAPELINE			2
#define HG_SHAPEAALINE			3
#define HG_SHAPEGRADIENT		4

#define GV_NONE			0
#define GV_ADD			1
#define GV_SUBTRACT		2
#define GV_MULTIPLY		3
#define GV_DIVIDE		4

//**********gauge_var***********
//Assume sizeof(int) is 32
#define GV_NOVAR		0
#define GV_INTVAR		1	//<-- These mean that the function needs
#define GV_CHARVAR		2	//<-- to take on the responsibility of
#define GV_FLOATVAR		3	//<-- deallocating whatever types are set
#define GV_GAUGEVAR		4	//<-- in the union.

#define GV_PTRSTART		100

#define GV_INTPTR		100	
#define GV_CHARPTR		101
#define GV_FLOATPTR		102
#define GV_GAUGEVARPTR	103

class gauge_var
{
	int var_operator;
	int data_type;
	size_t char_size;

	union
	{
		int *int_variable;
		char **char_pointer;	//Necessary for dynamic calculations
		char *char_variable;
		float *float_variable;

		gauge_var *gv_variable;
	};

	int prev_data_type;
	size_t prev_char_size;
	union
	{
		int *prev_int_result;
		char *prev_char_result;
		float *prev_float_result;
	};

	gauge_var *next, *prev;

	void DeallocVars();
	void DeallocPrevVars();
public:
	//Cover all the bases
	void gauge_var::operator=(int value);
	void gauge_var::operator=(char* value);
	void gauge_var::operator=(float value);

	//Pointer to other parts of the code
	void gauge_var::operator=(int* pointer);
	void gauge_var::operator=(char** pointer);
	void gauge_var::operator=(float* pointer);

	int Evaluate(int* result);
	int Evaluate(float* result);
	int Evaluate(char** result);

	int Append();
	gauge_var();
	~gauge_var();
};

//**********gauge_data***********
struct ship;
class gauge_data;

class gauge_data
{
	friend class hud;
private:
	//nonperishable stuff - these are constant.
	char name[NAME_LENGTH];								//Gauge name - nonperishable after setup
	int type;											//Basically, main gauge or child gauge?
	int priority;										//Goes 0 to 100, 50 are retail gauges
	
	//Dynamically changeable gauge variables
	int coords[2];										//Coordinates of the gauge

	//Set by update - not to be touched outside of update()
	int draw_coords[2];									//Coordinates the gauge is drawn at
	int update_num;										//Number of times the gauge has been updated

	//Image
	char image[MAX_FILENAME_LEN];						//Image displayed for the gauge
	int image_id;										//ID of image
	int image_size[2];									//Size of the image
	int num_frames;										//Number of frames the image has
	int frame[MAX_GAUGE_FRAMES];						//Which image frames to show; -1 means empty
	int frame_attrib[MAX_GAUGE_FRAMES][8];				//Info for each frame; 0-top clip, 1-left clip, 2-bottom clip, 3-right clip
	//TODO: attribute 4 should determine style, hud-color or image-pallete

	//Text
	char text[MESSAGE_LENGTH];							//Text for the gauge (Displayed under image)
	int max_text_lines;									//Number of text lines
	int max_text_width;									//How wide the text can be

	//Shape
	int shape_id[MAX_GAUGE_SHAPES];						//Index of shape to draw
	int shape_attrib[MAX_GAUGE_SHAPES][8];				//Attributes of shape

	//Model
	char model[MAX_FILENAME_LEN];						//Name of model to draw
	int model_id;										//Model id
	int model_attrib[8];								//Attributes of model
	color g_color;										//Gauge's default color; no alpha means it uses hud default
	color user_color;									//Default user color; no alpha means no setting

	gauge_data *next, *prev;
	gauge_data *first_child;							//Linked list of children

	//Private functions that help keep drawing nice and neat
	void draw_shape();
	void draw_image();
	void draw_model();
	void draw_text();
public:
	int (*update_func)(gauge_data* cg, ship* gauge_owner);//Function that sets data, maybe performs esoteric display functions

	int data_flags;										//Flags that provide additional info on the data
	int user_flags;										//Flags settable by the user
	int show_flags;										//Flags that set where the gauge is viewable
public:
	gauge_data();										//Creates the gauge
	~gauge_data();
	int copy(gauge_data *dest_gauge);

	gauge_data *get_gauge(char *name);

	//Info
	int get_update_num(){return update_num;}			//Current iteration number, so you can output a list of items

	//Level paging
	int page_in();										//Pages in images and stuff for the current hud gauge
	int reset();										//Call before a level to reset everything to defaults (incomplete)

	//In-mission functions
	int show(ship* gauge_owner, gauge_data* cgp = NULL);	//Updates the gauge and all children
	int draw();												//Draws the gauge; shouldn't need to be called outside of update
	int set_up(char* name, int def_x, int def_y, gauge_data* new_parent = NULL, int priority = 100);		//Inits a gauge with data and readies it for use
	int set_image(char* image_name, int newframe = 0, int sizex = 0, int sizey = 0);	//Sets the current image
	int set_model(char* model_name);	//Sets the current model (incomplete)
};

//**********hud***********
#define HI_DIM		0
#define HI_NORMAL	1
#define HI_BRIGHT	2

class hud
{
	friend void set_current_hud(ship* owner);
private:
	bool loaded;
	int resolution[2];

	int num_gauges;
	gauge_data *first_gauge;
	ship* owner;
	color hud_color;
public:
	bool is_loaded() { return loaded; }
	void show();
	ubyte get_preset_alpha(int preset_id);
	gauge_data* add_gauge(char* name, int def_x_coord, int def_y_coord, gauge_data* new_gauge_parent = NULL, int priority = 100);
	gauge_data *get_gauge(char* name);

	int copy(hud* dest_hud);

	hud();
	~hud();
};

#else
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
	int Wenergy_text_coords[2];
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

//	hud_info();  // GCC won't take this with offsetof

	hud_info( )
		: loaded( false )
	{
		memset( resolution, 0, sizeof( resolution ) );
		memset( Player_shield_coords, 0, sizeof( Player_shield_coords ) );
		memset( Target_shield_coords, 0, sizeof( Target_shield_coords ) );
		memset( Shield_mini_coords, 0, sizeof( Shield_mini_coords ) );
		Shield_mini_fname[ 0 ] = NULL;
		memset( Aburn_coords, 0, sizeof( Aburn_coords ) );
		memset( Wenergy_coords, 0, sizeof( Wenergy_coords ) );
		memset( Wenergy_text_coords, 0, sizeof( Wenergy_text_coords ) );
		memset( Aburn_size, 0, sizeof( Aburn_size ) );
		memset( Wenergy_size, 0, sizeof( Wenergy_size ) );
		Aburn_fname[ 0 ] = NULL;
		Wenergy_fname[ 0 ] = NULL;
		memset( Escort_coords, 0, sizeof( Escort_coords ) );
		memset( Hud_mini_3digit, 0, sizeof( Hud_mini_3digit ) );
		memset( Hud_mini_2digit, 0, sizeof( Hud_mini_2digit ) );
		memset( Hud_mini_1digit, 0, sizeof( Hud_mini_1digit ) );
		memset( Escort_htext_coords, 0, sizeof( Escort_htext_coords ) );
		Escort_htext[ 0 ] = NULL;
		memset( Escort_list, 0, sizeof( Escort_list ) );
		memset( Escort_entry, 0, sizeof( Escort_entry ) );
		memset( Escort_entry_last, 0, sizeof( Escort_entry_last ) );
		memset( Escort_name, 0, sizeof( Escort_name ) );
		memset( Escort_integrity, 0, sizeof( Escort_integrity ) );
		memset( Escort_status, 0, sizeof( Escort_status ) );
		memset( Escort_filename, 0, sizeof( Escort_filename ) );
		memset( custom_gauge_coords, 0, sizeof( custom_gauge_coords ) );
		memset( custom_gauge_sizes, 0, sizeof( custom_gauge_sizes ) );
		memset( custom_gauge_images, 0, sizeof( custom_gauge_images ) );
		memset( custom_gauge_frames, 0, sizeof( custom_gauge_frames ) );
		memset( custom_gauge_text, 0, sizeof( custom_gauge_text ) );
		memset( custom_gauge_colors, 0, sizeof( custom_gauge_colors ) );
	}
} hud_info;

#endif

typedef struct gauge_info
{
	gauge_info* parent;	//Parent, used for mini-gauges, pointer to gauge_info (NULL if main gauge
	size_t coord_dest;	//Offset of coord in hud_info
	char fieldname[NAME_LENGTH];	//TBL entry token
	int defaultx_640;	//Default 640x480 x coord
	int defaulty_480;	//Default 640x480 y coord
	int defaultx_1024;	//Default 1024x768 x coord
	int defaulty_768;	//Default 1024x768 y coord
	size_t size_dest;	//offset of size coord in hud_info; init in load_hud_defaults() (Can be 0 (*not* NULL))
	size_t image_dest;	//offset of image string in hud_info; init in load_hud_defaults() (Can be 0 (*not* NULL))
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
extern float Hud_unit_multiplier;


#define NUM_HUD_RETICLE_STYLES	2

#define HUD_RETICLE_STYLE_FS1	0
#define HUD_RETICLE_STYLE_FS2	1

extern int Hud_reticle_style;


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

#endif // _HUDPARSE_H
