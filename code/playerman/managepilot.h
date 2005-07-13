/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Playerman/ManagePilot.h $
 * $Revision: 2.5 $
 * $Date: 2005-07-13 03:35:32 $
 * $Author: Goober5000 $
 *
 * ManagePilot.h is a header file for code to load and save pilot files, and
 * to select and manage the pilot
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/12/22 21:49:05  taylor
 * add a popup to make sure people know about pilot upgrade
 *
 * Revision 2.3  2004/08/11 05:06:32  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:02:05  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     1/12/99 3:15a Dave
 * Barracks screen support for selecting squad logos. We need real artwork
 * :)
 * 
 * 4     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 3     10/09/98 5:17p Andsager
 * move barracks screen into barracks.cpp
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 19    4/27/98 4:56p Hoffoss
 * Added 'rank pips' to pilot names in barracks screen.
 * 
 * 18    4/19/98 4:05p Dave
 * Changed main hall region selection. Put an overwrite request in for new
 * pilots with same callsign as existing pilots. Highlight local player by
 * default in multi debriefing. 
 * 
 * 17    4/09/98 5:43p Dave
 * Remove all command line processing from the demo. Began work fixing up
 * the new multi host options screen.
 * 
 * 16    3/25/98 2:16p Dave
 * Select random default image for newly created pilots. Fixed several
 * multi-pause messaging bugs. Begin work on online help for multiplayer
 * keys.
 * 
 * 15    12/23/97 12:00p Allender
 * change write_pilot_file to *not* take is_single as a default parameter.
 * causing multiplayer pilots to get written to the single player folder
 * 
 * 14    12/22/97 5:08p Hoffoss
 * Changed inputbox class to be able to accept only certain keys, changed
 * pilot screens to utilize this feature.  Added to assert with pilot file
 * saving.
 * 
 * 13    11/18/97 10:53a Hoffoss
 * 
 * 12    11/12/97 4:40p Dave
 * Put in multiplayer campaign support parsing, loading and saving. Made
 * command-line variables better named. Changed some things on the initial
 * pilot select screen.
 * 
 * 11    11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 * 
 * 10    10/24/97 10:59p Hoffoss
 * Added in create pilot popup window and barracks screen.
 * 
 * 9     10/21/97 7:18p Hoffoss
 * Overhauled the key/joystick control structure and usage throughout the
 * entire FreeSpace code.  The whole system is very different now.
 * 
 * 8     10/09/97 4:57p Lawrance
 * store short_callsign in the player struct
 * 
 * 7     12/18/96 10:18a Lawrance
 * integrating joystick axis configuration
 * 
 * 6     11/13/96 9:03a Lawrance
 * Capitalized various global varibles
 * 
 * 5     11/13/96 8:40a Lawrance
 * fixed bug when a new campaign would not clear out the missions played
 * from the previous campaign
 * 
 * 4     11/05/96 8:30a Lawrance
 * made backspace work while entering pilots names
 * 
 * 3     11/04/96 2:56p Lawrance
 * changed the way missions completed are read and written from .PLR file.
 * 
 * 2     11/01/96 3:22p Lawrance
 * implemented pilot selection, and pilot file save and restore
 *
 * $NoKeywords: $
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
