/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Gamesnd/GameSnd.cpp $
 * $Revision: 2.13 $
 * $Date: 2005-06-01 09:35:42 $
 * $Author: taylor $
 *
 * Routines to keep track of which sound files go where
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.12  2005/05/12 17:49:11  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.11  2005/04/25 00:22:34  wmcoolmon
 * Added parse_sound; replaced Assert() with an if() (The latter may not be a good idea, but it keeps missions from being un-debuggable)
 *
 * Revision 2.10  2005/04/21 15:58:08  taylor
 * initial changes to mission loading and status in debug builds
 *  - move bmpman page in init to an earlier stage to avoid unloading sexp loaded images
 *  - small changes to progress reports in debug builds so that it's easier to tell what's slow
 *  - initialize the loading screen before mission_parse() so that we'll be about to get a more accurate load time
 * fix memory leak in gamesnd (yes, I made a mistake ;))
 * make sure we unload models on game shutdown too
 *
 * Revision 2.9  2005/03/30 02:32:40  wmcoolmon
 * Made it so *Snd fields in ships.tbl and weapons.tbl take the sound name
 * as well as its index (ie "L_sidearm.wav" instead of "76")
 *
 * Revision 2.8  2005/03/24 23:31:46  taylor
 * make sounds.tbl dynamic
 * "filename" will never be larger than 33 chars so having it 260 is a waste (freespace.cpp)
 *
 * Revision 2.7  2004/07/26 20:47:30  Kazan
 * remove MCD complete
 *
 * Revision 2.6  2004/07/12 16:32:47  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.5  2004/04/26 00:25:08  taylor
 * don't preload every sound (fixes some slowdowns), snd_preload option support
 *
 * Revision 2.4  2004/04/03 02:55:49  bobboau
 * commiting recent minor bug fixes
 *
 * Revision 2.3  2004/03/05 09:02:00  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/10/15 22:03:24  Kazan
 * Da Species Update :D
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 5     6/25/99 3:08p Dave
 * Multiple flyby sounds.
 * 
 * 4     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 3     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 39    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 38    4/25/98 1:25p Lawrance
 * Make function for playing generic error beep
 * 
 * 37    4/18/98 9:12p Lawrance
 * Added Aureal support.
 * 
 * 36    3/29/98 12:56a Lawrance
 * preload the warp in and explosions sounds before a mission.
 * 
 * 35    3/25/98 6:10p Lawrance
 * Work on DirectSound3D
 * 
 * 34    2/22/98 2:48p John
 * More String Externalization Classification
 * 
 * 33    1/17/98 12:33p John
 * Made the game_busy function be called a constant amount of times per
 * level load, making the bar prediction easier.
 * 
 * 32    1/17/98 12:14p John
 * Added loading... bar to freespace.
 * 
 * 31    1/11/98 11:14p Lawrance
 * Preload sounds that we expect will get played.
 * 
 * 30    12/24/97 8:54p Lawrance
 * Integrating new popup code
 * 
 * 29    12/19/97 3:44p Mike
 * Fix parse code.  Would improperly read a number through a comma.  Lots
 * of ships.tbl problems.
 * 
 * 28    12/01/97 5:25p Hoffoss
 * Routed interface sound playing through a special function that will
 * only allow one instance of the sound to play at a time, avoiding
 * over-mixing problems.
 * 
 * 27    11/20/97 1:06a Lawrance
 * Add Master_voice_volume, make voices play back at correctly scaled
 * volumes
 * 
 * 26    10/17/97 1:36p Lawrance
 * load/unload interface sounds
 * 
 * 25    10/14/97 11:35p Lawrance
 * change snd_load parameters
 * 
 * 24    7/05/97 1:46p Lawrance
 * improve robustness of gameplay and interface sound loading/unloading
 * 
 * 23    6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 22    6/08/97 5:59p Lawrance
 * flag sounds as 3D
 * 
 * 21    6/05/97 11:25a Lawrance
 * use sound signatures to ensure correct sound is loaded
 * 
 * 20    6/05/97 1:07a Lawrance
 * changes to support sound interface
 * 
 * 19    6/04/97 1:18p Lawrance
 * added hooks for shield impacts
 * 
 * 18    6/02/97 1:50p Lawrance
 * supporting new format of sounds in table
 * 
 * 17    5/14/97 9:54a Lawrance
 * supporting mission-specific briefing music
 * 
 * 16    5/08/97 1:56p Lawrance
 * supporting ship-specific engine sounds
 * 
 * 15    5/06/97 9:36a Lawrance
 * added support for min and max distances for 3d sounds
 * 
 * 14    4/23/97 5:19p Lawrance
 * split up misc sounds into: gamewide, ingame, and interface
 * 
 * 13    4/20/97 11:48a Lawrance
 * added array of filenames for misc sounds.  Will be useful if we want to
 * unload then re-load sounds
 * 
 * 12    4/20/97 11:19a Lawrance
 * sndman_ interface obsolete.  Using snd_ functions to load, play, and
 * manage static sound fx
 * 
 * 11    4/18/97 4:31p Mike
 * Add support for default volume levels.
 * 
 * 10    3/20/97 11:04a Lawrance
 * using incorrect constant when loading music filenames
 * 
 * 9     3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 8     3/17/97 3:47p Mike
 * Homing missile lock sound.
 * More on AI ships firing missiles.
 * 
 * 7     3/10/97 8:54a Lawrance
 * added gamesnd_init_looping_sounds()
 * 
 * 6     2/28/97 8:41a Lawrance
 * added afterburner engage and burn sounds
 * 
 * 5     2/14/97 12:37a Lawrance
 * added hooks to play docking/undocking sounds
 * 
 * 4     2/13/97 12:03p Lawrance
 * hooked in throttle sounds
 * 
 * 3     2/05/97 10:35a Lawrance
 * supporting spooled music at menus, briefings, credits etc.
 * 
 * 2     1/20/97 7:58p John
 * Fixed some link errors with testcode.
 * 
 * 1     1/20/97 7:08p John
 *
 * $NoKeywords: $
 */

#include "gamesnd/gamesnd.h"
#include "localization/localize.h"
#include "species_defs/species_defs.h"
#include "parse/parselo.h"


/* -- dynamic now - taylor
// Global array that holds data about the gameplay sound effects.
game_snd Snds[MAX_GAME_SOUNDS];

// Global array that holds data about the interface sound effects.
game_snd Snds_iface[MAX_INTERFACE_SOUNDS];
int Snds_iface_handle[MAX_INTERFACE_SOUNDS]; */

int Num_game_sounds = 0;
game_snd *Snds = NULL;

int Num_iface_sounds = 0;
game_snd *Snds_iface = NULL;
int *Snds_iface_handle = NULL;

#define GAME_SND	0
#define IFACE_SND	1

void gamesnd_add_sound_slot(int type, int num);

// flyby sounds - 2 for each species (fighter and bomber flybys)
game_snd Snds_flyby[MAX_SPECIES_NAMES][2];


void gamesnd_play_iface(int n)
{
	if (Snds_iface_handle[n] >= 0)
		snd_stop(Snds_iface_handle[n]);

	Snds_iface_handle[n] = snd_play(&Snds_iface[n]);
}

int gamesnd_get_by_name(char* name)
{
	for(int i = 0; i < Num_game_sounds; i++)
	{
		if(!stricmp(Snds[i].filename, name))
		{
			return i;
		}
	}
	return -1;
}

//Takes a tag, a sound index destination, and the object name being parsed
//tag and object_name are mostly so that we can debug stuff
//This also means you shouldn't use optional_string or required_string,
//just make sure the destination sound index can handle -1 if things
//don't work out.
void parse_sound(char* tag, int *idx_dest, char* object_name)
{
	char buf[256];
	int idx;

	(*idx_dest) = -1;
	if(optional_string(tag))
	{
		stuff_string(buf, F_NAME, NULL, sizeof(buf)/sizeof(char));
		idx = gamesnd_get_by_name(buf);
		if(idx != -1)
			(*idx_dest) = idx;
		else
			(*idx_dest) = atoi(buf);

		//Ensure sound is in range
		if((*idx_dest) < -1 || (*idx_dest) >= Num_game_sounds)
		{
			Warning(LOCATION, "%s sound index out of range on '%s'. Must be between 0 and %d. Forcing to -1 (Nonexistant sound).\n", tag, object_name, Num_game_sounds);
		}
	}
}

// load in sounds that we expect will get played
//
// The method currently used is to load all those sounds that have the hardware flag
// set.  This works well since we don't want to try and load hardware sounds in on the
// fly (too slow).
void gamesnd_preload_common_sounds()
{
	int		i;
	game_snd	*gs;

	for ( i = 0; i < Num_game_sounds; i++ ) {
		gs = &Snds[i];
		if ( gs->filename[0] != 0 && stricmp(gs->filename, NOX("none.wav")) ) {
			if ( gs->preload ) {
				gs->id = snd_load(gs);
			}
		}
		game_busy();		// Animate loading cursor... does nothing if loading screen not active.
	}
}

// -------------------------------------------------------------------------------------------------
// gamesnd_load_gameplay_sounds()
//
// Load the ingame sounds into memory
//
void gamesnd_load_gameplay_sounds()
{
	int		i;
	game_snd	*gs;

	for ( i = 0; i < Num_game_sounds; i++ ) {
		gs = &Snds[i];
		if ( gs->filename[0] != 0 && stricmp(gs->filename, NOX("none.wav")) ) {
			if ( !gs->preload ) { // don't try to load anything that's already preloaded
				gs->id = snd_load(gs);
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------
// gamesnd_unload_gameplay_sounds()
//
// Unload the ingame sounds from memory
//
void gamesnd_unload_gameplay_sounds()
{
	int		i;
	game_snd	*gs;

	for ( i = 0; i < Num_game_sounds; i++ ) {
		gs = &Snds[i];
		if ( gs->id != -1 ) {
			snd_unload( gs->id );
			gs->id = -1;
		}
	}	
}

// -------------------------------------------------------------------------------------------------
// gamesnd_load_interface_sounds()
//
// Load the interface sounds into memory
//
void gamesnd_load_interface_sounds()
{
	int		i;
	game_snd	*gs;

	for ( i = 0; i < Num_iface_sounds; i++ ) {
		gs = &Snds_iface[i];
		if ( gs->filename[0] != 0 && stricmp(gs->filename, NOX("none.wav")) ) {
			gs->id = snd_load(gs);
		}
	}
}

// -------------------------------------------------------------------------------------------------
// gamesnd_unload_interface_sounds()
//
// Unload the interface sounds from memory
//
void gamesnd_unload_interface_sounds()
{
	int		i;
	game_snd	*gs;

	for ( i = 0; i < Num_iface_sounds; i++ ) {
		gs = &Snds_iface[i];
		if ( gs->id != -1 ) {
			snd_unload( gs->id );
			gs->id = -1;
			gs->id_sig = -1;
		}
	}
}

// -------------------------------------------------------------------------------------------------
// gamesnd_parse_line()
//
// Parse a sound effect line
//
void gamesnd_parse_line(game_snd *gs, char *tag)
{
	int is_3d;

	required_string(tag);
	stuff_int(&gs->sig);
	stuff_string(gs->filename, F_NAME, ",");
	if ( !stricmp(gs->filename,NOX("empty")) ) {
		gs->filename[0] = 0;
		advance_to_eoln(NULL);
		return;
	}
	Mp++;
	stuff_int(&gs->preload);
	stuff_float(&gs->default_volume);
	stuff_int(&is_3d);
	if ( is_3d ) {
		gs->flags |= GAME_SND_USE_DS3D;
		stuff_int(&gs->min);
		stuff_int(&gs->max);
	}
	advance_to_eoln(NULL);
}

// -------------------------------------------------------------------------------------------------
// gamesnd_parse_soundstbl() will parse the sounds.tbl file, and load the specified sounds.
//
//
void gamesnd_parse_soundstbl()
{
	int		rval;
	int		num_game_sounds = 0;
	int		num_iface_sounds = 0;

	// open localization
	lcl_ext_open();

	gamesnd_init_sounds();

	if ((rval = setjmp(parse_abort)) != 0) {
		Error(LOCATION, "Unable to parse sounds.tbl!  Code = %i.\n", rval);
	}
	else {
		read_file_text("sounds.tbl");
		reset_parse();		
	}

	// Parse the gameplay sounds section
	required_string("#Game Sounds Start");
	while (required_string_either("#Game Sounds End","$Name:")) {
		Assert( num_game_sounds < Num_game_sounds);
		gamesnd_parse_line( &Snds[num_game_sounds], "$Name:" );
		num_game_sounds++;
		gamesnd_add_sound_slot( GAME_SND, num_game_sounds );
	}
	required_string("#Game Sounds End");

	// Parse the interface sounds section
	required_string("#Interface Sounds Start");
	while (required_string_either("#Interface Sounds End","$Name:")) {
		Assert( num_iface_sounds < Num_iface_sounds);
		gamesnd_parse_line(&Snds_iface[num_iface_sounds], "$Name:");
		num_iface_sounds++;
		gamesnd_add_sound_slot( IFACE_SND, num_iface_sounds );
	}
	required_string("#Interface Sounds End");

	// parse flyby sound section	
	required_string("#Flyby Sounds Start");

#if defined(MORE_SPECIES)
	char cstrtemp[SPECIES_NAME_MAXLEN+3];

	for (int i = 0; i < True_NumSpecies; i++)
	{
		sprintf(cstrtemp, "$%s:", Species_names[i]);
		gamesnd_parse_line(&Snds_flyby[i][0], cstrtemp);
		gamesnd_parse_line(&Snds_flyby[i][1], cstrtemp);	
	}
#else
	// read 2 terran sounds
	gamesnd_parse_line(&Snds_flyby[SPECIES_TERRAN][0], "$Terran:");
	gamesnd_parse_line(&Snds_flyby[SPECIES_TERRAN][1], "$Terran:");

	// 2 vasudan sounds
	gamesnd_parse_line(&Snds_flyby[SPECIES_VASUDAN][0], "$Vasudan:");
	gamesnd_parse_line(&Snds_flyby[SPECIES_VASUDAN][1], "$Vasudan:");

	gamesnd_parse_line(&Snds_flyby[SPECIES_SHIVAN][0], "$Shivan:");
	gamesnd_parse_line(&Snds_flyby[SPECIES_SHIVAN][1], "$Shivan:");
#endif
	
	required_string("#Flyby Sounds End");

	// close localization
	lcl_ext_close();
}


// -------------------------------------------------------------------------------------------------
// gamesnd_init_struct()
//
void gamesnd_init_struct(game_snd *gs)
{
	gs->filename[0] = 0;
	gs->id = -1;
	gs->id_sig = -1;
//	gs->is_3d = 0;
//	gs->use_ds3d = 0;
	gs->flags = 0;
}

// -------------------------------------------------------------------------------------------------
// gamesnd_init_sounds() will initialize the Snds[] and Snds_iface[] arrays
//
void gamesnd_init_sounds()
{
	int		i;

	if (Snds == NULL) {
		Snds = (game_snd *) vm_malloc (sizeof(game_snd) * MIN_GAME_SOUNDS);
		Verify( Snds != NULL );
		Num_game_sounds += MIN_GAME_SOUNDS;
	}

	// init the gameplay sounds
	for ( i = 0; i < Num_game_sounds; i++ ) {
		gamesnd_init_struct(&Snds[i]);
	}

	if (Snds_iface == NULL) {
		Snds_iface = (game_snd *) vm_malloc (sizeof(game_snd) * MIN_INTERFACE_SOUNDS);
		Verify( Snds_iface != NULL );
		Num_iface_sounds += MIN_INTERFACE_SOUNDS;

		Assert( Snds_iface_handle == NULL );
		Snds_iface_handle = (int *) vm_malloc (sizeof(int) * Num_iface_sounds);
		Verify( Snds_iface_handle != NULL );
	}

	// init the interface sounds
	for ( i = 0; i < Num_iface_sounds; i++ ) {
		gamesnd_init_struct(&Snds_iface[i]);
		Snds_iface_handle[i] = -1;
	}
}

// close out gamesnd,  ONLY CALL FROM game_shutdown()!!!!
void gamesnd_close()
{
	if (Snds != NULL) {
		vm_free(Snds);
		Snds = NULL;
	}

	if (Snds_iface != NULL) {
		vm_free(Snds_iface);
		Snds_iface = NULL;
	}

	if (Snds_iface_handle != NULL) {
		vm_free(Snds_iface_handle);
		Snds_iface_handle = NULL;
	}
}

// callback function for the UI code to call when the mouse first goes over a button.
void common_play_highlight_sound()
{
	gamesnd_play_iface(SND_USER_OVER);
}

void gamesnd_play_error_beep()
{
	gamesnd_play_iface(SND_GENERAL_FAIL);
}

void gamesnd_add_sound_slot(int type, int num)
{
	const int increase_by = 5;
	int i;

	switch (type) {
		case GAME_SND:
		{
			Assert( Snds != NULL );
			Assert( num < (Num_game_sounds + increase_by) );

			if (num >= Num_game_sounds) {
				Snds = (game_snd *) vm_realloc (Snds, sizeof(game_snd) * (Num_game_sounds + increase_by));
				Verify( Snds != NULL );
				Num_game_sounds += increase_by;

				// default all new entries
				for (i = (Num_game_sounds - increase_by - 1); i < Num_game_sounds; i++) {
					gamesnd_init_struct(&Snds[i]);
				}
			}
		}
		break;

		case IFACE_SND:
		{
			Assert( Snds_iface != NULL );
			Assert( num < (Num_game_sounds + increase_by) );

			if (num >= Num_iface_sounds) {
				Snds_iface = (game_snd *) vm_realloc (Snds_iface, sizeof(game_snd) * (Num_iface_sounds + increase_by));
				Verify( Snds_iface != NULL );
				Num_iface_sounds += increase_by;

				Assert( Snds_iface_handle != NULL );
				Snds_iface_handle = (int *) vm_realloc (Snds_iface_handle, sizeof(int) * Num_iface_sounds);
				Verify( Snds_iface_handle != NULL );

				// default all new entries
				for (i = (Num_iface_sounds - increase_by - 1); i < Num_iface_sounds; i++) {
					gamesnd_init_struct(&Snds_iface[i]);
					Snds_iface_handle[i] = -1;
				}
			}
		}
		break;

		default:
			Int3();
	}
}
