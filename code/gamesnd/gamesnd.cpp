/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <sstream>

#include "gamesnd/gamesnd.h"
#include "localization/localize.h"
#include "species_defs/species_defs.h"
#include "parse/parselo.h"
#include "sound/ds.h"
#include <limits.h>

SCP_vector<game_snd>	Snds;
SCP_vector<game_snd>	Snds_iface;
SCP_vector<int>			Snds_iface_handle;

void gamesnd_play_iface(int n)
{
	if (Snds_iface_handle[n] >= 0)
		snd_stop(Snds_iface_handle[n]);

	Snds_iface_handle[n] = snd_play(&Snds_iface[n]);
}

/**
 * Function to search for a given game_snd with the specified name
 * in the passed vector
 * 
 * @param name Name to search for
 * @param sounds Vector to seach in
 *
 */
int gamesnd_lookup_name(const char* name, const SCP_vector<game_snd>& sounds)
{
	// if we get passed -1, don't bother trying to look it up.
	if (name == NULL || *name == 0 || !strcmp(name, "-1"))
	{
		return -1;
	}

	Assert( sounds.size() <= INT_MAX );

	int i = 0;

	for(SCP_vector<game_snd>::const_iterator snd = sounds.begin(); snd != sounds.end(); ++snd)
	{
		if (!snd->name.compare(name))
		{
			return i;
		}
		i++;
	}

	return -1;
}

// WMC - now ignores file extension.
int gamesnd_get_by_name(const char* name)
{
	Assert( Snds.size() <= INT_MAX );
	
	int index = gamesnd_lookup_name(name, Snds);

	if (index < 0)
	{
		int i = 0;
		for(SCP_vector<game_snd>::iterator snd = Snds.begin(); snd != Snds.end(); ++snd)
		{
			char *p = strrchr( snd->filename, '.' );
			if(p == NULL)
			{
				if(!stricmp(snd->filename, name))
				{
					index = i;
					break;
				}
			}
			else if(!strnicmp(snd->filename, name, p-snd->filename))
			{
				index = i;
				break;
			}

			i++;
		}
	}

	return index;
}

int gamesnd_get_by_iface_name(const char* name)
{
	Assert( Snds_iface.size() <= INT_MAX );
	Assert( Snds_iface.size() == Snds_iface_handle.size() );
	
	int index = gamesnd_lookup_name(name, Snds_iface);

	if (index < 0)
	{
		int i = 0;
		for(SCP_vector<game_snd>::iterator snd = Snds_iface.begin(); snd != Snds_iface.end(); ++snd)
		{
			char *p = strrchr( snd->filename, '.' );
			if(p == NULL)
			{
				if(!stricmp(snd->filename, name))
				{
					index = i;
					break;
				}
			}
			else if(!strnicmp(snd->filename, name, p-snd->filename))
			{
				index = i;
				break;
			}

			i++;
		}
	}

	return index;
}

int gamesnd_get_by_tbl_index(int index)
{
	char temp[11];
	sprintf(temp, "%i", index);

	return gamesnd_lookup_name(temp, Snds);
}

int gamesnd_get_by_iface_tbl_index(int index)
{
	Assert( Snds_iface.size() == Snds_iface_handle.size() );

	char temp[11];
	sprintf(temp, "%i", index);

	return gamesnd_lookup_name(temp, Snds_iface);
}

/**
 * Helper function for parse_sound and parse_sound_list. Do not use directly.
 * 
 * @param tag Tag 
 * @param idx_dest Sound index destination
 * @param object_name Object name being parsed
 * @param buf Buffer holding string to be parsed
 * @param flags See the parse_sound_flags enum
 *
 */
void parse_sound_core(const char* tag, int *idx_dest, const char* object_name, const char* buf, parse_sound_flags flags)
{
	int idx;

	if(flags & PARSE_SOUND_INTERFACE_SOUND)
		idx = gamesnd_get_by_iface_name(buf);
	else
		idx = gamesnd_get_by_name(buf);

	if(idx != -1)
	{
		(*idx_dest) = idx;
	}

	size_t size_to_check = 0;
	
	if(flags & PARSE_SOUND_INTERFACE_SOUND)
	{
		size_to_check = Snds_iface.size();
		Assert( Snds_iface.size() == Snds_iface_handle.size() );
	}
	else
	{
		size_to_check = Snds.size();
	}

	Assert( size_to_check <= INT_MAX );

	//Ensure sound is in range
	if((*idx_dest) < -1 || (*idx_dest) >= (int)size_to_check)
	{
		(*idx_dest) = -1;
		Warning(LOCATION, "%s sound index out of range on '%s'. Must be between 0 and %d. Forcing to -1 (Nonexistent sound).\n", tag, object_name, size_to_check);
	}
}

/**
 * Parse a sound. When using this function for a table entry, 
 * required_string and optional_string aren't needed, as this function deals with 
 * that as its tag parameter, just make sure that the destination sound index can 
 * handle -1 if things don't work out.
 *
 * @param tag Tag 
 * @param idx_dest Sound index destination
 * @param object_name Object name being parsed
 * @param flags See the parse_sound_flags enum
 *
 */
void parse_sound(const char* tag, int *idx_dest, const char* object_name, parse_sound_flags flags)
{
	if(optional_string(tag))
	{
		char buf[MAX_FILENAME_LEN];
		stuff_string(buf, F_NAME, MAX_FILENAME_LEN);

		parse_sound_core(tag, idx_dest, object_name, buf, flags);
	}
}

/**
 * CommanderDJ - Parse a list of sounds. When using this function for a table entry, 
 * required_string and optional_string aren't needed, as this function deals with 
 * that as its tag parameter, just make sure that the destination sound index(es) can 
 * handle -1 if things don't work out.
 *
 * @param destination Vector where sound indexes are to be stored
 * @param tag Tag 
 * @param object_name Name of object being parsed
 * @param flags See the parse_sound_flags enum
 *
 */
void parse_sound_list(const char* tag, SCP_vector<int>& destination, const char* object_name, parse_sound_flags flags)
{
	if(optional_string(tag))
	{
		int check=0;

		//if we're using the old format, parse the first entry separately
		if(!(flags & PARSE_SOUND_SCP_SOUND_LIST))
		{
			stuff_int(&check);
		}

		//now read the rest of the entries on the line
		for(size_t i=0; !check_for_eoln(); i++)
		{
			char buf[MAX_FILENAME_LEN];
			stuff_string_white(buf, MAX_FILENAME_LEN);

			//we do this conditionally to avoid adding needless entries when reparsing
			if(destination.size() <= i)
			{
				destination.push_back(-1);
			}

			parse_sound_core(tag, &destination.at(i), object_name, buf, flags);
		}

		//if we're using the old format, double check the size)
		if(!(flags & PARSE_SOUND_SCP_SOUND_LIST) && (destination.size() != (unsigned)check))
		{
			mprintf(("%s in '%s' has %i entries. This does not match entered size of %i.", tag, object_name, destination.size(), check));
		}
	}
}

/**
 * Load in sounds that we expect will get played
 *
 * The method currently used is to load all those sounds that have the hardware flag
 * set.  This works well since we don't want to try and load hardware sounds in on the
 * fly (too slow).
 */
void gamesnd_preload_common_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs) {
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( gs->preload ) {
				game_busy( NOX("** preloading common game sounds **") );	// Animate loading cursor... does nothing if loading screen not active.
				gs->id = snd_load(&(*gs));
			}
		}
	}
}

/**
 * Load the ingame sounds into memory
 */
void gamesnd_load_gameplay_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs) {
		if ( gs->filename[0] != 0 && strnicmp(gs->filename, NOX("none.wav"), 4) ) {
			if ( !gs->preload ) { // don't try to load anything that's already preloaded
				game_busy( NOX("** preloading gameplay sounds **") );		// Animate loading cursor... does nothing if loading screen not active.
				gs->id = snd_load(&(*gs));
			}
		}
	}
}

/**
 * Unload the ingame sounds from memory
 */
void gamesnd_unload_gameplay_sounds()
{
	Assert( Snds.size() <= INT_MAX );
	for (SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs) {
		if ( gs->id != -1 ) {
			snd_unload( gs->id );
			gs->id = -1;
		}
	}	
}

/**
 * Load the interface sounds into memory
 */
void gamesnd_load_interface_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds_iface.size() < INT_MAX );
	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); ++si) {
		if ( si->filename[0] != 0 && strnicmp(si->filename, NOX("none.wav"), 4) ) {
			si->id = snd_load(&(*si));
		}
	}
}

/**
 * Unload the interface sounds from memory
 */
void gamesnd_unload_interface_sounds()
{
	Assert( Snds_iface.size() < INT_MAX );
	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); ++si) {
		if ( si->id != -1 ) {
			snd_unload( si->id );
			si->id = -1;
			si->id_sig = -1;
		}
	}
}

void parse_gamesnd_old(game_snd* gs)
{
	int is_3d;
	int temp;

	stuff_string(gs->filename, F_NAME, MAX_FILENAME_LEN, ",");

	if (!stricmp(gs->filename, NOX("empty")))
	{
		gs->filename[0] = 0;
		advance_to_eoln(NULL);
		return;
	}
	Mp++;

	stuff_int(&temp);

	if (temp > 0)
	{
		gs->preload = true;
	}

	stuff_float(&gs->default_volume);

	stuff_int(&is_3d);

	if (is_3d)
	{
		gs->flags |= GAME_SND_USE_DS3D;
		stuff_int(&gs->min);
		stuff_int(&gs->max);
	}
	else
	{
		// silly retail, not abiding by its own format...
		if (!stricmp(gs->filename, "l_hit.wav") || !stricmp(gs->filename, "m_hit.wav"))
		{
			ignore_gray_space();
			if (stuff_int_optional(&temp, true) == 2)
			{
				mprintf(("Dutifully ignoring the extra sound values for retail sound %s, '%s'...\n", gs->name.c_str(), gs->filename));
				ignore_gray_space();
				stuff_int_optional(&temp, true);
			}
		}

		gs->min = 0;
		gs->max = 0;
	}

	// check for extra values per Mantis #2408
	ignore_gray_space();
	if (stuff_int_optional(&temp, true) == 2)
	{
		Warning(LOCATION, "Unexpected extra value %d found for sound '%s' (filename '%s')!  Check the format of the sounds.tbl (or .tbm) entry.", temp, gs->name.c_str(), gs->filename);
	}

	advance_to_eoln(NULL);
}

void parse_gamesnd_new(game_snd* gs)
{
	// New extended format found
	stuff_string(gs->filename, F_NAME, MAX_FILENAME_LEN);
		
	if (!stricmp(gs->filename, NOX("empty")))
	{
		gs->filename[0] = 0;
		return;
	}

	required_string("+Preload:");

	stuff_boolean(&gs->preload);

	required_string("+Volume:");

	stuff_float(&gs->default_volume);

	if (optional_string("+3D Sound:"))
	{
		gs->flags |= GAME_SND_USE_DS3D;
		required_string("+Attenuation start:");
		
		stuff_int(&gs->min);
		
		required_string("+Attenuation end:");

		stuff_int(&gs->max);
	}
	else
	{
		gs->min = 0;
		gs->max = 0;
	}
}

void gamesnd_parse_entry(game_snd *gs, bool no_create, SCP_vector<game_snd> *lookupVector)
{
	SCP_string name;

	stuff_string(name, F_NAME, "\t \n");

	if (!no_create)
	{
		if (lookupVector != NULL)
		{
			if (gamesnd_lookup_name(name.c_str(), *lookupVector) >= 0)
			{
				Warning(LOCATION, "Duplicate sound name \"%s\" found!", name.c_str());
			}
		}

		gs->name = name;
	}
	else
	{
		int vectorIndex = gamesnd_lookup_name(name.c_str(), *lookupVector);

		if (vectorIndex < 0)
		{
			Warning(LOCATION, "No existing sound entry with name \"%s\" found!", name.c_str());
			no_create = false;
			gs->name = name;
		}
		else
		{
			gs = &lookupVector->at(vectorIndex);
		}
	}

	if (optional_string("+Filename:"))
	{
		parse_gamesnd_new(gs);
	}
	else
	{
		parse_gamesnd_old(gs);
	}
}

/**
 * Parse a sound effect entry by requiring the given tag at the beginning.
 * 
 * @param gs The game_snd instance to fill in
 * @param tag The tag that's required before an entry
 * @param lookupVector If non-NULL used to look up @c +nocreate entries
 * 
 * @return @c true when a new entry has been parsed and should be added to the list of known
 *			entries. @c false otherwise, for example in case of @c +nocreate
 */
bool gamesnd_parse_line(game_snd *gs, const char *tag, SCP_vector<game_snd> *lookupVector = NULL)
{
	Assertion(gs != NULL, "Invalid game_snd pointer passed to gamesnd_parse_line!");
	
	required_string(const_cast<char*>(tag));

	bool no_create = false;

	if (lookupVector != NULL)
	{
		if(optional_string("+nocreate"))
		{
			no_create = true;
		}
	}

	gamesnd_parse_entry(gs, no_create, lookupVector);

	return !no_create;
}

void parse_sound_environments()
{
	char name[65] = { '\0' };
	char template_name[65] = { '\0' };
	EFXREVERBPROPERTIES *props;

	while (required_string_either("#Sound Environments End", "$Name:"))
	{
		required_string("$Name:");
		stuff_string(name, F_NAME, sizeof(name)-1);

		if (optional_string("$Template:"))
		{
			stuff_string(template_name, F_NAME, sizeof(template_name)-1);
		}
		else
		{
			template_name[0] = '\0';
		}

		ds_eax_get_prop(&props, name, template_name);

		if (optional_string("+Density:"))
		{
			stuff_float(&props->flDensity);
			CLAMP(props->flDensity, 0.0f, 1.0f);
		}

		if (optional_string("+Diffusion:"))
		{
			stuff_float(&props->flDiffusion);
			CLAMP(props->flDiffusion, 0.0f, 1.0f);
		}

		if (optional_string("+Gain:"))
		{
			stuff_float(&props->flGain);
			CLAMP(props->flGain, 0.0f, 1.0f);
		}

		if (optional_string("+Gain HF:"))
		{
			stuff_float(&props->flGainHF);
			CLAMP(props->flGainHF, 0.0f, 1.0f);
		}

		if (optional_string("+Gain LF:"))
		{
			stuff_float(&props->flGainLF);
			CLAMP(props->flGainLF, 0.0f, 1.0f);
		}

		if (optional_string("+Decay Time:"))
		{
			stuff_float(&props->flDecayTime);
			CLAMP(props->flDecayTime, 0.01f, 20.0f);
		}

		if (optional_string("+Decay HF Ratio:"))
		{
			stuff_float(&props->flDecayHFRatio);
			CLAMP(props->flDecayHFRatio, 0.1f, 20.0f);
		}

		if (optional_string("+Decay LF Ratio:"))
		{
			stuff_float(&props->flDecayLFRatio);
			CLAMP(props->flDecayLFRatio, 0.1f, 20.0f);
		}

		if (optional_string("+Reflections Gain:"))
		{
			stuff_float(&props->flReflectionsGain);
			CLAMP(props->flReflectionsGain, 0.0f, 3.16f);
		}

		if (optional_string("+Reflections Delay:"))
		{
			stuff_float(&props->flReflectionsDelay);
			CLAMP(props->flReflectionsDelay, 0.0f, 0.3f);
		}

		if (optional_string("+Reflections Pan:"))
		{
			stuff_float_list(props->flReflectionsPan, 3);
			CLAMP(props->flReflectionsPan[0], 0.0f, 1.0f);
			CLAMP(props->flReflectionsPan[1], 0.0f, 1.0f);
			CLAMP(props->flReflectionsPan[2], 0.0f, 1.0f);
		}

		if (optional_string("+Late Reverb Gain:"))
		{
			stuff_float(&props->flLateReverbGain);
			CLAMP(props->flLateReverbGain, 0.0f, 10.0f);
		}

		if (optional_string("+Late Reverb Delay:"))
		{
			stuff_float(&props->flLateReverbDelay);
			CLAMP(props->flLateReverbDelay, 0.0f, 0.1f);
		}

		if (optional_string("+Late Reverb Pan:"))
		{
			stuff_float_list(props->flLateReverbPan, 3);
			CLAMP(props->flLateReverbPan[0], 0.0f, 1.0f);
			CLAMP(props->flLateReverbPan[1], 0.0f, 1.0f);
			CLAMP(props->flLateReverbPan[2], 0.0f, 1.0f);
		}

		if (optional_string("+Echo Time:"))
		{
			stuff_float(&props->flEchoTime);
			CLAMP(props->flEchoTime, 0.075f, 0.25f);
		}

		if (optional_string("+Echo Depth:"))
		{
			stuff_float(&props->flEchoDepth);
			CLAMP(props->flEchoDepth, 0.0f, 1.0f);
		}

		if (optional_string("+Modulation Time:"))
		{
			stuff_float(&props->flModulationTime);
			CLAMP(props->flModulationTime, 0.004f, 4.0f);
		}

		if (optional_string("+Modulation Depth:"))
		{
			stuff_float(&props->flModulationDepth);
			CLAMP(props->flModulationDepth, 0.0f, 1.0f);
		}

		if (optional_string("+HF Reference:"))
		{
			stuff_float(&props->flHFReference);
			CLAMP(props->flHFReference, 1000.0f, 20000.0f);
		}

		if (optional_string("+LF Reference:"))
		{
			stuff_float(&props->flLFReference);
			CLAMP(props->flLFReference, 20.0f, 1000.0f);
		}

		if (optional_string("+Room Rolloff Factor:"))
		{
			stuff_float(&props->flRoomRolloffFactor);
			CLAMP(props->flRoomRolloffFactor, 0.0f, 10.0f);
		}

		if (optional_string("+Air Absorption Gain HF:"))
		{
			stuff_float(&props->flAirAbsorptionGainHF);
			CLAMP(props->flAirAbsorptionGainHF, 0.892f, 1.0f);
		}

		if (optional_string("+Decay HF Limit:"))
		{
			stuff_int(&props->iDecayHFLimit);
			CLAMP(props->iDecayHFLimit, 0, 1);
		}
	}

	required_string("#Sound Environments End");
}

static SCP_vector<species_info> missingFlybySounds;

void parse_sound_table(const char* filename)
{
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// Parse the gameplay sounds section
		if (optional_string("#Game Sounds Start"))
		{
			while (!check_for_string("#Game Sounds End"))
			{
				game_snd tempSound;
				if (gamesnd_parse_line(&tempSound, "$Name:", &Snds))
				{
					Snds.push_back(game_snd(tempSound));
				}
			}

			required_string("#Game Sounds End");
		}

		// Parse the interface sounds section
		if (optional_string("#Interface Sounds Start"))
		{
			while (!check_for_string("#Interface Sounds End"))
			{
				game_snd tempSound;
				if (gamesnd_parse_line(&tempSound, "$Name:", &Snds_iface))
				{
					Snds_iface.push_back(game_snd(tempSound));
					Snds_iface_handle.push_back(-1);
				}
			}

			required_string("#Interface Sounds End");
		}

		// parse flyby sound section	
		if (optional_string("#Flyby Sounds Start"))
		{
			char species_name_tag[NAME_LENGTH + 2];
			int	sanity_check = 0;
			size_t i;

			while (!check_for_string("#Flyby Sounds End") && (sanity_check <= (int)Species_info.size()))
			{
				for (i = 0; i < Species_info.size(); i++)
				{
					species_info *species = &Species_info[i];

					sprintf(species_name_tag, "$%s:", species->species_name);

					if (check_for_string(species_name_tag))
					{
						gamesnd_parse_line(&species->snd_flyby_fighter, species_name_tag);
						gamesnd_parse_line(&species->snd_flyby_bomber, species_name_tag);
						sanity_check--;
					}
					else
					{
						sanity_check++;
					}
				}
			}

			required_string("#Flyby Sounds End");
		}

		if (optional_string("#Sound Environments Start"))
		{
			parse_sound_environments();
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

/**
 * Parse the sounds.tbl file, and load the specified sounds.
 */
void gamesnd_parse_soundstbl()
{
	parse_sound_table("sounds.tbl");

	parse_modular_table("*-snd.tbm", parse_sound_table);

	// if we are missing any species then report 
	if (missingFlybySounds.size() > 0)
	{
		SCP_string errorString;
		for (size_t i = 0; i < missingFlybySounds.size(); i++)
		{
			errorString.append(missingFlybySounds[i].species_name);
			errorString.append("\n");
		}
		
		Error(LOCATION, "The following species are missing flyby sounds in sounds.tbl:\n%s", errorString.c_str());
	}

	missingFlybySounds.clear();
}

/**
 * Close out gamesnd, ONLY CALL FROM game_shutdown()!!!!
 */
void gamesnd_close()
{
	Snds.clear();
	Snds_iface.clear();
	Snds_iface_handle.clear();
}

/**
 * Callback function for the UI code to call when the mouse first goes over a button.
 */
void common_play_highlight_sound()
{
	gamesnd_play_iface(SND_USER_OVER);
}

/**
 * Callback function for the UI code to call when an error beep needed.
 */
void gamesnd_play_error_beep()
{
	gamesnd_play_iface(SND_GENERAL_FAIL);
}
