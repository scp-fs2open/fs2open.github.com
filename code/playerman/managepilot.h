/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"

class player;

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

void pilot_set_start_campaign(player* p);
