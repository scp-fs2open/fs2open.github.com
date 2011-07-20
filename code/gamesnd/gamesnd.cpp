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
#include "sound/ds.h"
#include <limits.h>


SCP_vector<game_snd> Snds;

SCP_vector<game_snd> Snds_iface;
SCP_vector<int> Snds_iface_handle;

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
	Assert( Snds.size() <= INT_MAX );
	int i = 0;
	for(SCP_vector<game_snd>::iterator snd = Snds.begin(); snd != Snds.end(); snd++)
	{
		char *p = strrchr( snd->filename, '.' );
		if(p == NULL)
		{
			if(!stricmp(snd->filename, name))
			{
				return i;
			}
		}
		else if(!strnicmp(snd->filename, name, p-snd->filename))
		{
			return i;
		}
		i++;
	}
	return -1;
}

int gamesnd_get_by_tbl_index(int index)
{
	//if we get passed -1, don't bother trying to look it up.
	if (index == -1)
		return -1;
	Assert( Snds.size() <= INT_MAX );
	int i = 0;
	for(SCP_vector<game_snd>::iterator snd = Snds.begin(); snd != Snds.end(); snd++) {
		if ( snd->sig == index )
		{
			return i;
		}
		i++;
	}
	return -1;
}

int gamesnd_get_by_iface_tbl_index(int index)
{
	Assert( Snds_iface.size() <= INT_MAX );
	Assert( Snds_iface.size() == Snds_iface_handle.size() );
	int i = 0;
	for(SCP_vector<game_snd>::iterator snd = Snds.begin(); snd != Snds.end(); snd++) {
		if ( snd->sig == index )
		{
			return i;
		}
		i++;
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
		{
			idx = gamesnd_get_by_tbl_index(atoi(buf));
			if (idx != -1)
				(*idx_dest) = idx;
		}

		Assert( Snds.size() <= INT_MAX );
		//Ensure sound is in range
		if((*idx_dest) < -1 || (*idx_dest) >= (int)Snds.size())
		{
			(*idx_dest) = -1;
			Warning(LOCATION, "%s sound index out of range on '%s'. Must be between 0 and %d. Forcing to -1 (Nonexistant sound).\n", tag, object_name, Snds.size());
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
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); gs++) {
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( gs->preload ) {
				game_busy( NOX("** preloading common game sounds **") );	// Animate loading cursor... does nothing if loading screen not active.
				gs->id = snd_load(&(*gs));
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
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); gs++) {
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( !gs->preload ) { // don't try to load anything that's already preloaded
				game_busy( NOX("** preloading gameplay sounds **") );		// Animate loading cursor... does nothing if loading screen not active.
				gs->id = snd_load(&(*gs));
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
	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); gs++) {
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
	if ( !Sound_enabled )
		return;

	Assert( Snds_iface.size() < INT_MAX );
	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); si++) {
		if ( si->filename[0] != 0 && strnicmp(si->filename, NOX("none.wav"), 4) ) {
			si->id = snd_load(&(*si));
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
	Assert( Snds_iface.size() < INT_MAX );
	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); si++) {
		if ( si->id != -1 ) {
			snd_unload( si->id );
			si->id = -1;
			si->id_sig = -1;
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
		Assert( Snds.size() < INT_MAX );
		Assert( num_game_sounds < (int)Snds.size() );
		gamesnd_parse_line( &Snds[num_game_sounds], "$Name:" );
		num_game_sounds++;
		gamesnd_add_sound_slot( GAME_SND, num_game_sounds );
	}
	required_string("#Game Sounds End");

	// Parse the interface sounds section
	required_string("#Interface Sounds Start");
	while (required_string_either("#Interface Sounds End","$Name:")) {
		Assert( Snds_iface_handle.size() < INT_MAX );
		Assert( num_iface_sounds < (int)Snds_iface.size() );
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

	if ( optional_string("#Sound Environments Start") ) {
		char name[65] = { '\0' };
		char template_name[65] = { '\0' };
		EFXREVERBPROPERTIES *props;

		while ( required_string_either("#Sound Environments End", "$Name:") ) {
			required_string("$Name:");
			stuff_string(name, F_NAME, sizeof(name)-1);

			if ( optional_string("$Template:") ) {
				stuff_string(template_name, F_NAME, sizeof(template_name)-1);
			} else {
				template_name[0] = '\0';
			}

			ds_eax_get_prop(&props, name, template_name);

			if ( optional_string("+Density:") ) {
				stuff_float(&props->flDensity);
				CLAMP(props->flDensity, 0.0f, 1.0f);
			}

			if ( optional_string("+Diffusion:") ) {
				stuff_float(&props->flDiffusion);
				CLAMP(props->flDiffusion, 0.0f, 1.0f);
			}

			if ( optional_string("+Gain:") ) {
				stuff_float(&props->flGain);
				CLAMP(props->flGain, 0.0f, 1.0f);
			}

			if ( optional_string("+Gain HF:") ) {
				stuff_float(&props->flGainHF);
				CLAMP(props->flGainHF, 0.0f, 1.0f);
			}

			if ( optional_string("+Gain LF:") ) {
				stuff_float(&props->flGainLF);
				CLAMP(props->flGainLF, 0.0f, 1.0f);
			}

			if ( optional_string("+Decay Time:") ) {
				stuff_float(&props->flDecayTime);
				CLAMP(props->flDecayTime, 0.01f, 20.0f);
			}

			if ( optional_string("+Decay HF Ratio:") ) {
				stuff_float(&props->flDecayHFRatio);
				CLAMP(props->flDecayHFRatio, 0.1f, 20.0f);
			}

			if ( optional_string("+Decay LF Ratio:") ) {
				stuff_float(&props->flDecayLFRatio);
				CLAMP(props->flDecayLFRatio, 0.1f, 20.0f);
			}

			if ( optional_string("+Reflections Gain:") ) {
				stuff_float(&props->flReflectionsGain);
				CLAMP(props->flReflectionsGain, 0.0f, 3.16f);
			}

			if ( optional_string("+Reflections Delay:") ) {
				stuff_float(&props->flReflectionsDelay);
				CLAMP(props->flReflectionsDelay, 0.0f, 0.3f);
			}

			if ( optional_string("+Reflections Pan:") ) {
				stuff_float_list(props->flReflectionsPan, 3);
				CLAMP(props->flReflectionsPan[0], 0.0f, 1.0f);
				CLAMP(props->flReflectionsPan[1], 0.0f, 1.0f);
				CLAMP(props->flReflectionsPan[2], 0.0f, 1.0f);
			}

			if ( optional_string("+Late Reverb Gain:") ) {
				stuff_float(&props->flLateReverbGain);
				CLAMP(props->flLateReverbGain, 0.0f, 10.0f);
			}

			if ( optional_string("+Late Reverb Delay:") ) {
				stuff_float(&props->flLateReverbDelay);
				CLAMP(props->flLateReverbDelay, 0.0f, 0.1f);
			}

			if ( optional_string("+Late Reverb Pan:") ) {
				stuff_float_list(props->flLateReverbPan, 3);
				CLAMP(props->flLateReverbPan[0], 0.0f, 1.0f);
				CLAMP(props->flLateReverbPan[1], 0.0f, 1.0f);
				CLAMP(props->flLateReverbPan[2], 0.0f, 1.0f);
			}

			if ( optional_string("+Echo Time:") ) {
				stuff_float(&props->flEchoTime);
				CLAMP(props->flEchoTime, 0.075f, 0.25f);
			}

			if ( optional_string("+Echo Depth:") ) {
				stuff_float(&props->flEchoDepth);
				CLAMP(props->flEchoDepth, 0.0f, 1.0f);
			}

			if ( optional_string("+Modulation Time:") ) {
				stuff_float(&props->flModulationTime);
				CLAMP(props->flModulationTime, 0.004f, 4.0f);
			}

			if ( optional_string("+Modulation Depth:") ) {
				stuff_float(&props->flModulationDepth);
				CLAMP(props->flModulationDepth, 0.0f, 1.0f);
			}

			if ( optional_string("+HF Reference:") ) {
				stuff_float(&props->flHFReference);
				CLAMP(props->flHFReference, 1000.0f, 20000.0f);
			}

			if ( optional_string("+LF Reference:") ) {
				stuff_float(&props->flLFReference);
				CLAMP(props->flLFReference, 20.0f, 1000.0f);
			}

			if ( optional_string("+Room Rolloff Factor:") ) {
				stuff_float(&props->flRoomRolloffFactor);
				CLAMP(props->flRoomRolloffFactor, 0.0f, 10.0f);
			}

			if ( optional_string("+Air Absorption Gain HF:") ) {
				stuff_float(&props->flAirAbsorptionGainHF);
				CLAMP(props->flAirAbsorptionGainHF, 0.892f, 1.0f);
			}

			if ( optional_string("+Decay HF Limit:") ) {
				stuff_int(&props->iDecayHFLimit);
				CLAMP(props->iDecayHFLimit, 0, 1);
			}
		}

		required_string("#Sound Environments End");
	}

	// close localization
	lcl_ext_close();
}


// -------------------------------------------------------------------------------------------------
// gamesnd_init_struct()
//
void gamesnd_init_struct(game_snd *gs)
{
	gs->sig = -1;
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
	int		i = 0;

	Snds.clear();
	Snds.resize(MIN_GAME_SOUNDS);

	Assert( Snds.size() > 0 );

	Assert( Snds.size() <= INT_MAX );
	// init the gameplay sounds
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); gs++) {
		gamesnd_init_struct(&(*gs));
	}

	Snds_iface.clear();
	Snds_iface.resize(MIN_INTERFACE_SOUNDS);
	Snds_iface_handle.resize(MIN_INTERFACE_SOUNDS);
	
	Assert( Snds_iface.size() > 0 );

	Assert( Snds_iface.size() < INT_MAX );
	// init the interface sounds

	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); si++) {
		gamesnd_init_struct(&(*si));
		Snds_iface_handle[i] = -1;
		i++;
	}
}

// close out gamesnd,  ONLY CALL FROM game_shutdown()!!!!
void gamesnd_close()
{
	Snds.clear();
	Snds_iface.clear();
	Snds_iface_handle.clear();
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
			Assert( Snds.size() <= INT_MAX );
			Assert( num < ((int)Snds.size() + increase_by) );

			if (num >= (int)Snds.size()) {
				Snds.resize(Snds.size() + increase_by);

				// default all new entries
				for (i = ((int)Snds.size() - increase_by); i < (int)Snds.size(); i++) {
					gamesnd_init_struct(&Snds[i]);
				}
			}
		}
		break;

		case IFACE_SND:
		{
			Assert( Snds_iface.size() < INT_MAX );
			Assert( num < ((int)Snds_iface.size() + increase_by) );

			if (num >= (int)Snds_iface.size()) {
				Snds_iface.resize(Snds_iface.size() + increase_by);

				Snds_iface_handle.resize(Snds_iface.size());
				Assert( Snds_iface.size() < INT_MAX );

				// default all new entries
				for (i = ((int)Snds_iface_handle.size() - increase_by); i < (int)Snds_iface_handle.size(); i++) {
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
