/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"

struct CFILE;
struct player;

#define VALID_PILOT_CHARS	" _-"

#define MAX_PILOTS			20
#define MAX_PILOT_IMAGES	64

// pilot pic image list stuff ( call pilot_load_pic_list() to make these valid )
extern char Pilot_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
extern char *Pilot_image_names[MAX_PILOT_IMAGES];
extern int Num_pilot_images;

// squad logo list stuff (call pilot_load_squad_pic_list() to make these valid )
extern char Pilot_squad_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
extern char *Pilot_squad_image_names[MAX_PILOT_IMAGES];
extern int Num_pilot_squad_images;

// low-level read/writes to files
int read_int(CFILE *file);
short read_short(CFILE *file);
ubyte read_byte(CFILE *file);
void write_int(int i, CFILE *file);
void write_short(short s, CFILE *file);
void write_byte(ubyte i, CFILE *file);

void read_string(char *s, CFILE *f);
void write_string(char *s, CFILE *f);

// two ways of determining if a given pilot is multiplayer
// note, that the first version of this function can possibly return -1 if the file is invalid, etc.
int is_pilot_multi(CFILE *fp);	// pass a newly opened (at the beginning) file pointer to the pilot file itself
int is_pilot_multi(player *p);	// pass a pointer to a player struct

int verify_pilot_file(char *filename, int single = 1, int *rank = NULL);
int pilot_file_upgrade_check(char *callsign, int single = 1);
int read_pilot_file(char* callsign, int single = 1, player *p = NULL);
int write_pilot_file(player *p = NULL);

// function to get default pilot callsign for game
void choose_pilot();

void init_new_pilot(player *p, int reset = 1);

// load up the list of pilot image filenames (do this at game startup as well as barracks startup)
void pilot_load_pic_list();

// load up the list of pilot squad filenames
void pilot_load_squad_pic_list();

// set the truncated version of the callsign in the player struct
void pilot_set_short_callsign(player *p, int max_width);

// pick a random image for the passed player
void pilot_set_random_pic(player *p);

// pick a random squad logo for the passed player
void pilot_set_random_squad_pic(player *p);

// format a pilot's callsign into a "personal" form - ie, adding a 's or just an ' as appropriate
void pilot_format_callsign_personal(char *in_callsign,char *out_callsign);

// throw up a popup asking the user to verify the overwrite of an existing pilot name
// 1 == ok to overwrite, 0 == not ok
int pilot_verify_overwrite();

// functions that update player information that is stored in PLR file
void update_missions_played(int mission_number);
void clear_missions_played();
