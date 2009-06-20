/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "gamesnd/gamesnd.h"
#include "localization/localize.h"
#include "species_defs/species_defs.h"
#include "parse/parselo.h"


int Num_game_sounds = 0;
game_snd *Snds = NULL;

int Num_iface_sounds = 0;
game_snd *Snds_iface = NULL;
int *Snds_iface_handle = NULL;

#define GAME_SND	0
#define IFACE_SND	1

void gamesnd_add_sound_slot(int type, int num);

void gamesnd_play_iface(int n)
{
	if (Snds_iface_handle[n] >= 0)
		snd_stop(Snds_iface_handle[n]);

	Snds_iface_handle[n] = snd_play(&Snds_iface[n]);
}

//WMC - now ignores file extension.
int gamesnd_get_by_name(char* name)
{
	for(int i = 0; i < Num_game_sounds; i++)
	{
		char *p = strrchr( Snds[i].filename, '.' );
		if(p == NULL)
		{
			if(!stricmp(Snds[i].filename, name))
			{
				return i;
			}
		}
		else if(!strnicmp(Snds[i].filename, name, p-Snds[i].filename))
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
	char buf[MAX_FILENAME_LEN];
	int idx;

	if(optional_string(tag))
	{
		stuff_string(buf, F_NAME, MAX_FILENAME_LEN);
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

	if ( !Sound_enabled )
		return;

	for ( i = 0; i < Num_game_sounds; i++ ) {
		gs = &Snds[i];
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( gs->preload ) {
				game_busy( NOX("** preloading common game sounds **") );	// Animate loading cursor... does nothing if loading screen not active.
				gs->id = snd_load(gs);
			}
		}
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

	if ( !Sound_enabled )
		return;

	for ( i = 0; i < Num_game_sounds; i++ ) {
		gs = &Snds[i];
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( !gs->preload ) { // don't try to load anything that's already preloaded
				game_busy( NOX("** preloading gameplay sounds **") );		// Animate loading cursor... does nothing if loading screen not active.
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

	if ( !Sound_enabled )
		return;

	for ( i = 0; i < Num_iface_sounds; i++ ) {
		gs = &Snds_iface[i];
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
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
	stuff_string(gs->filename, F_NAME, MAX_FILENAME_LEN, ",");
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
	} else {
		gs->min = 0;
		gs->max = 0;
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
	int		i;
	char	cstrtemp[NAME_LENGTH+3];
	char	*missing_species_names = NULL;
	ubyte	*missing_species = NULL;
	int		sanity_check = 0;

	gamesnd_init_sounds();

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "sounds.tbl", rval));
		lcl_ext_close();
		return;
	}

	read_file_text("sounds.tbl", CF_TYPE_TABLES);
	reset_parse();		

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

	missing_species_names = new char[Species_info.size() * (NAME_LENGTH+2)];
	missing_species = new ubyte[Species_info.size()];

	memset( missing_species_names, 0, Species_info.size() * (NAME_LENGTH+2) );
	memset( missing_species, 1, Species_info.size() );	// assume they are all missing

	while ( !check_for_string("#Flyby Sounds End") && (sanity_check <= (int)Species_info.size()) )
	{
		for (i = 0; i < (int)Species_info.size(); i++) {
			species_info *species = &Species_info[i];

			sprintf(cstrtemp, "$%s:", species->species_name);

			if ( check_for_string(cstrtemp) ) {
				gamesnd_parse_line(&species->snd_flyby_fighter, cstrtemp);
				gamesnd_parse_line(&species->snd_flyby_bomber, cstrtemp);
				missing_species[i] = 0;
				sanity_check--;
			} else {
				sanity_check++;
			}
		}
	}

	// if we are missing any species then report it
	for (i = 0; i < (int)Species_info.size(); i++) {
		if ( missing_species[i] ) {
			strcat(missing_species_names, Species_info[i].species_name);
			strcat(missing_species_names, "\n");
		}
	}

	if ( strlen(missing_species_names) ) {
		Error( LOCATION, "The following species are missing flyby sounds in sounds.tbl:\n%s", missing_species_names );
	}

	delete[] missing_species_names;
	delete[] missing_species;

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
		Num_game_sounds = MIN_GAME_SOUNDS;
	}

	Assert( Num_game_sounds > 0 );

	// init the gameplay sounds
	for ( i = 0; i < Num_game_sounds; i++ ) {
		gamesnd_init_struct(&Snds[i]);
	}

	if (Snds_iface == NULL) {
		Snds_iface = (game_snd *) vm_malloc (sizeof(game_snd) * MIN_INTERFACE_SOUNDS);
		Verify( Snds_iface != NULL );
		Num_iface_sounds = MIN_INTERFACE_SOUNDS;

		Assert( Snds_iface_handle == NULL );
		Snds_iface_handle = (int *) vm_malloc (sizeof(int) * Num_iface_sounds);
		Verify( Snds_iface_handle != NULL );
	}

	Assert( Num_iface_sounds > 0 );

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
				for (i = (Num_game_sounds - increase_by); i < Num_game_sounds; i++) {
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
				for (i = (Num_iface_sounds - increase_by); i < Num_iface_sounds; i++) {
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
