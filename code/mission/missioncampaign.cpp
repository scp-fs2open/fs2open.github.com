/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

#ifdef SCP_UNIX
#include <sys/stat.h>
#include <glob.h>
#endif

#include "mission/missioncampaign.h"
#include "ui/ui.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "parse/sexp.h"
#include "playerman/player.h"
#include "mission/missiongoals.h"
#include "cutscene/movie.h"
#include "gamesnd/eventmusic.h"
#include "localization/localize.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "cfile/cfile.h"
#include "starfield/supernova.h"
#include "cutscene/cutscenes.h"
#include "menuui/techmenu.h"
#include "missionui/missionscreencommon.h"
#include "missionui/redalert.h"
#include "pilotfile/pilotfile.h"
#include "popup/popup.h"


// campaign wasn't ended
int Campaign_ending_via_supernova = 0;

// stuff for selecting campaigns.  We need to keep both arrays around since we display the
// list of campaigns by name, but must load campaigns by filename
char *Campaign_names[MAX_CAMPAIGNS] = { NULL };
char *Campaign_file_names[MAX_CAMPAIGNS] = { NULL };
char *Campaign_descs[MAX_CAMPAIGNS] = { NULL };
int	Num_campaigns;
int Campaign_file_missing;
int Campaign_load_failure = 0;
int Campaign_names_inited = 0;
SCP_vector<SCP_string> Ignored_campaigns;

char Default_campaign_file_name[MAX_FILENAME_LEN - 4]  = { 0 };

// stuff used for campaign list building
static bool MC_desc = false;
static bool MC_multiplayer = false;

char *campaign_types[MAX_CAMPAIGN_TYPES] = 
{
//XSTR:OFF
	"single",
	"multi coop",
	"multi teams"
//XSTR:ON
};

// modules local variables to deal with getting new ships/weapons available to the player
int Num_granted_ships, Num_granted_weapons;		// per mission counts of new ships and weapons
int Granted_ships[MAX_SHIP_CLASSES];
int Granted_weapons[MAX_WEAPON_TYPES];

// variables to control the UI stuff for loading campaigns
LOCAL UI_WINDOW Campaign_window;
LOCAL UI_LISTBOX Campaign_listbox;
LOCAL UI_BUTTON Campaign_okb, Campaign_cancelb;

// the campaign!!!!!
campaign Campaign;


bool campaign_is_ignored(const char *filename);

/**
 * Returns a string (which is malloced in this routine) of the name of the given freespace campaign file.  
 * In the type field, we return if the campaign is a single player or multiplayer campaign.  
 * The type field will only be valid if the name returned is non-NULL
 */
int mission_campaign_get_info(const char *filename, char *name, int *type, int *max_players, char **desc)
{
	int rval, i, success = 0;
	char campaign_type[NAME_LENGTH], fname[MAX_FILENAME_LEN];

	Assert( name != NULL );
	Assert( type != NULL );

	strncpy(fname, filename, MAX_FILENAME_LEN - 1);
	int fname_len = strlen(fname);
	if ((fname_len < 4) || stricmp(fname + fname_len - 4, FS_CAMPAIGN_FILE_EXT)){
		strcat_s(fname, FS_CAMPAIGN_FILE_EXT);
		fname_len += 4;
	}
	Assert(fname_len < MAX_FILENAME_LEN);

	// open localization
	lcl_ext_open();

	*type = -1;
	do {
		if ((rval = setjmp(parse_abort)) != 0) {
			mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error code = %i.\n", fname, rval));
			break;
		}

		read_file_text(fname);
		reset_parse();

		required_string("$Name:");
		stuff_string( name, F_NAME, NAME_LENGTH );
		if (name == NULL) {
			nprintf(("Warning", "No name found for campaign file %s\n", filename));
			break;
		}

		required_string("$Type:");
		stuff_string( campaign_type, F_NAME, NAME_LENGTH );

		for (i=0; i<MAX_CAMPAIGN_TYPES; i++) {
			if ( !stricmp(campaign_type, campaign_types[i]) ) {
				*type = i;
			}
		}

		if (name == NULL) {
			Warning(LOCATION, "Invalid campaign type \"%s\"\n", campaign_type);
			break;
		}

		if (desc) {
			if (optional_string("+Description:")) {
				*desc = stuff_and_malloc_string(F_MULTITEXT, NULL);
			} else {
				*desc = NULL;
			}
		}

		// if this is a multiplayer campaign, get the max players
		if ((*type) != CAMPAIGN_TYPE_SINGLE) {
			skip_to_string("+Num Players:");
			stuff_int(max_players);
		}

		// if we found a valid campaign type
		if ((*type) >= 0) {
			success = 1;
		}
	} while (0);

	// close localization
	lcl_ext_close();

	Assert(success);
	return success;
}

/**
 * Parses campaign and returns a list of missions in it.  
 * @return Number of missions added to the 'list', and up to 'max' missions may be added to 'list'.  
 * @return Negative on error.
 */
int mission_campaign_get_mission_list(const char *filename, char **list, int max)
{
	int rval, i, num = 0;
	char name[MAX_FILENAME_LEN];

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	// read the campaign file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error code = %i.\n", filename, rval));

		// since we can't return count of allocated elements, free them instead
		for (i=0; i<num; i++)
			vm_free(list[i]);

		num = -1;

	} else {
		read_file_text(filename);
		reset_parse();

		while (skip_to_string("$Mission:") > 0) {
			stuff_string(name, F_NAME, MAX_FILENAME_LEN);
			if (num < max)
				list[num++] = vm_strdup(name);
			else
				Warning(LOCATION, "Maximum number of missions exceeded (%d)!", max);
		}
	}

	return num;
}

void mission_campaign_free_list()
{
	int i;

	if ( !Campaign_names_inited )
		return;

	for (i = 0; i < Num_campaigns; i++) {
		if (Campaign_names[i] != NULL) {
			vm_free(Campaign_names[i]);
			Campaign_names[i] = NULL;
		}

		if (Campaign_file_names[i] != NULL) {
			vm_free(Campaign_file_names[i]);
			Campaign_file_names[i] = NULL;
		}

		if (Campaign_descs[i] != NULL) {
			vm_free(Campaign_descs[i]);
			Campaign_descs[i] = NULL;
		}
	}

	Num_campaigns = 0;
	Campaign_names_inited = 0;
}

int mission_campaign_maybe_add(const char *filename)
{
	char name[NAME_LENGTH];
	char *desc = NULL;
	int type, max_players;

	// don't add ignored campaigns
	if (campaign_is_ignored(filename)) {
		return 0;
	}

	if ( mission_campaign_get_info( filename, name, &type, &max_players, &desc) ) {
		if ( !MC_multiplayer && (type == CAMPAIGN_TYPE_SINGLE) ) {
			Campaign_names[Num_campaigns] = vm_strdup(name);

			if (MC_desc)
				Campaign_descs[Num_campaigns] = desc;

			Num_campaigns++;

			return 1;
		}
	}

	if (desc != NULL)
		vm_free(desc);
 
	return 0;
}

/**
 * Builds up the list of campaigns that the user might be able to pick from.
 * It uses the multiplayer flag to tell if we should display a list of single or multiplayer campaigns.
 * This routine sets the Num_campaigns and Campaign_names global variables
 */
void mission_campaign_build_list(bool desc, bool sort, bool multiplayer)
{
	char wild_card[10];
	int i, j, incr = 0;
	char *t = NULL;
	int rc = 0;

	if (Campaign_names_inited)
		return;

	MC_desc = desc;
	MC_multiplayer = multiplayer;

	memset(wild_card, 0, sizeof(wild_card));
	strcpy_s(wild_card, NOX("*"));
	strcat_s(wild_card, FS_CAMPAIGN_FILE_EXT);

	// if we have already been loaded then free everything and reload
	if (Num_campaigns != 0)
		mission_campaign_free_list();

	// set filter for cf_get_file_list() if there isn't one set already (the simroom has a special one)
	if (Get_file_list_filter == NULL)
		Get_file_list_filter = mission_campaign_maybe_add;

	// now get the list of all mission names
	// NOTE: we don't do sorting here, but we assume CF_SORT_NAME, and do it manually below
	rc = cf_get_file_list(MAX_CAMPAIGNS, Campaign_file_names, CF_TYPE_MISSIONS, wild_card, CF_SORT_NONE);
	Assert( rc == Num_campaigns );

	// now sort everything, if we are supposed to
	if (sort) {
		incr = Num_campaigns / 2;

		while (incr > 0) {
			for (i = incr; i < Num_campaigns; i++) {
				j = i - incr;
	
				while (j >= 0) {
					char *name1 = Campaign_names[j];
					char *name2 = Campaign_names[j + incr];

					// if we hit this then a coder probably did something dumb (like not needing to sort)
					if ( (name1 == NULL) || (name2 == NULL) ) {
						Int3();
						break;
					}

					if ( !strnicmp(name1, "the ", 4) )
						name1 += 4;

					if ( !strnicmp(name2, "the ", 4) )
						name2 += 4;

					if (stricmp(name1, name2) > 0) {
						// first, do filenames
						t = Campaign_file_names[j];
						Campaign_file_names[j] = Campaign_file_names[j + incr];
						Campaign_file_names[j + incr] = t;

						// next, actual names
						t = Campaign_names[j];
						Campaign_names[j] = Campaign_names[j + incr];
						Campaign_names[j + incr] = t;

						// finally, do descriptions
						if (desc) {
							t = Campaign_descs[j];
							Campaign_descs[j] = Campaign_descs[j + incr];
							Campaign_descs[j + incr] = t;
						}

						j -= incr;
					} else {
						break;
					}
				}
			}

			incr /= 2;
		}
	}

	// Done!
	Campaign_names_inited = 1;
}


/**
 * Gets optional ship/weapon information
 */
void mission_campaign_get_sw_info()
{
	int i, count, ship_list[MAX_SHIP_CLASSES], weapon_list[MAX_WEAPON_TYPES];

	// set allowable ships to the SIF_PLAYER_SHIPs
	memset( Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed) );
	for (i = 0; i < Num_ship_classes; i++ ) {
		if ( Ship_info[i].flags & SIF_PLAYER_SHIP )
			Campaign.ships_allowed[i] = 1;
	}

	for (i = 0; i < MAX_WEAPON_TYPES; i++ )
		Campaign.weapons_allowed[i] = 1;

	if ( optional_string("+Starting Ships:") ) {
		for (i = 0; i < Num_ship_classes; i++ )
			Campaign.ships_allowed[i] = 0;

		count = stuff_int_list(ship_list, MAX_SHIP_CLASSES, SHIP_INFO_TYPE);

		// now set the array elements stating which ships we are allowed
		for (i = 0; i < count; i++ ) {
			if ( Ship_info[ship_list[i]].flags & SIF_PLAYER_SHIP )
				Campaign.ships_allowed[ship_list[i]] = 1;
		}
	}

	if ( optional_string("+Starting Weapons:") ) {
		for (i = 0; i < MAX_WEAPON_TYPES; i++ )
			Campaign.weapons_allowed[i] = 0;

		count = stuff_int_list(weapon_list, MAX_WEAPON_TYPES, WEAPON_POOL_TYPE);

		// now set the array elements stating which ships we are allowed
		for (i = 0; i < count; i++ )
			Campaign.weapons_allowed[weapon_list[i]] = 1;
	}
}

/**
 * Starts a new campaign.  It reads in the mission information in the campaign file
 * It also sets up all variables needed inside of the game to deal with starting mission numbers, etc
 *
 * Note: Due to difficulties in generalizing this function, parts of it are duplicated throughout
 * this file.  If you change the format of the campaign file, you should be sure these related
 * functions work properly and update them if it breaks them.
 */
int mission_campaign_load( char *filename, player *pl, int load_savefile )
{
	int len, rval, i;
	char name[NAME_LENGTH], type[NAME_LENGTH], temp[NAME_LENGTH];

	if (campaign_is_ignored(filename)) {
		Campaign_file_missing = 1;
		return CAMPAIGN_ERROR_IGNORED;
	}

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	// open localization
	lcl_ext_open();	

	if ( pl == NULL )
		pl = Player;

	if ( !Fred_running && load_savefile && (pl == NULL) ) {
		Int3();
		load_savefile = 0;
	}

	// read the mission file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("Error parsing '%s'\r\nError code = %i.\r\n", filename, rval));

		// close localization
		lcl_ext_close();

		Campaign.filename[0] = 0;
		Campaign.num_missions = 0;

		if ( !Fred_running && !(Game_mode & GM_MULTIPLAYER) ) {
			Campaign_file_missing = 1;
			Campaign_load_failure = CAMPAIGN_ERROR_MISSING;
			return CAMPAIGN_ERROR_MISSING;
		}

		return CAMPAIGN_ERROR_CORRUPT;

	} else {
		// be sure to remove all old malloced strings of Mission_names
		// we must also free any goal stuff that was from a previous campaign
		// this also frees sexpressions so the next call to init_sexp will be able to reclaim
		// nodes previously used by another campaign.
		mission_campaign_clear();

		strcpy_s( Campaign.filename, filename );

		// only initialize the sexpression stuff when Fred isn't running.  It'll screw things up major
		// if it does
		if ( !Fred_running ){
			init_sexp();		// must initialize the sexpression stuff
		}

		read_file_text( filename );
		reset_parse();

		// copy filename to campaign structure minus the extension
		len = strlen(filename) - 4;
		Assert(len < MAX_FILENAME_LEN);
		strncpy(Campaign.filename, filename, len);
		Campaign.filename[len] = 0;

		required_string("$Name:");
		stuff_string( name, F_NAME, NAME_LENGTH );
		
		//Store campaign name in the global struct
		strcpy_s( Campaign.name, name );

		required_string( "$Type:" );
		stuff_string( type, F_NAME, NAME_LENGTH );

		for (i = 0; i < MAX_CAMPAIGN_TYPES; i++ ) {
			if ( !stricmp(type, campaign_types[i]) ) {
				Campaign.type = i;
				break;
			}
		}

		if ( i == MAX_CAMPAIGN_TYPES )
			Error(LOCATION, "Unknown campaign type %s!", type);

		Campaign.desc = NULL;
		if (optional_string("+Description:"))
			Campaign.desc = stuff_and_malloc_string(F_MULTITEXT, NULL);

		// if the type is multiplayer -- get the number of players
		Campaign.num_players = 0;
		if ( Campaign.type != CAMPAIGN_TYPE_SINGLE) {
			required_string("+Num players:");
			stuff_int( &(Campaign.num_players) );
		}		

		// parse flags - Goober5000
		Campaign.flags = CF_DEFAULT_VALUE;
		if (optional_string("$Flags:"))
		{
			stuff_int( &(Campaign.flags) );
		}

		// parse the optional ship/weapon information
		mission_campaign_get_sw_info();

		// parse the mission file and actually read in the mission stuff
		Campaign.num_missions = 0;
		while ( required_string_either("#End", "$Mission:") ) {
			cmission *cm;

			required_string("$Mission:");
			stuff_string(name, F_NAME, NAME_LENGTH);
			cm = &Campaign.missions[Campaign.num_missions];
			cm->name = vm_strdup(name);

			cm->notes = NULL;

			cm->briefing_cutscene[0] = 0;
			if ( optional_string("+Briefing Cutscene:") )
				stuff_string( cm->briefing_cutscene, F_NAME, NAME_LENGTH );

			cm->flags = 0;
			if (optional_string("+Flags:"))
				stuff_int(&cm->flags);

			cm->main_hall = "0";
			// deal with previous campaign versions
			if (cm->flags & CMISSION_FLAG_BASTION) {
				cm->main_hall = "1";
			}

			// Goober5000 - new main hall stuff!
			// Updated by CommanderDJ
			if (optional_string("+Main Hall:")) {
				stuff_string(temp, F_RAW, 32);
				cm->main_hall = temp;
			}

			// Goober5000 - new debriefing persona stuff!
			cm->debrief_persona_index = 0;
			if (optional_string("+Debriefing Persona Index:"))
				stuff_ubyte(&cm->debrief_persona_index);

			cm->formula = -1;
			if ( optional_string("+Formula:") ) {
				cm->formula = get_sexp_main();
				if ( !Fred_running ) {
					Assert ( cm->formula != -1 );
					sexp_mark_persistent( cm->formula );

				} else {
					if ( cm->formula == -1 ){
						// close localization
						lcl_ext_close();

						Campaign_load_failure = CAMPAIGN_ERROR_SEXP_EXHAUSTED;
						return CAMPAIGN_ERROR_SEXP_EXHAUSTED;
					}
				}
			}

			// Do mission branching stuff
			if ( optional_string("+Mission Loop:") ) {
				cm->flags |= CMISSION_FLAG_HAS_LOOP;
			} else if ( optional_string("+Mission Fork:") ) {
				cm->flags |= CMISSION_FLAG_HAS_FORK;
			}

			cm->mission_branch_desc = NULL;
			if ( optional_string("+Mission Loop Text:") || optional_string("+Mission Fork Text:") ) {
				cm->mission_branch_desc = stuff_and_malloc_string(F_MULTITEXT, NULL);
			}

			cm->mission_branch_brief_anim = NULL;
			if ( optional_string("+Mission Loop Brief Anim:") || optional_string("+Mission Fork Brief Anim:") ) {
				cm->mission_branch_brief_anim = stuff_and_malloc_string(F_MULTITEXT, NULL);
			}

			cm->mission_branch_brief_sound = NULL;
			if ( optional_string("+Mission Loop Brief Sound:") || optional_string("+Mission Fork Brief Sound:") ) {
				cm->mission_branch_brief_sound = stuff_and_malloc_string(F_MULTITEXT, NULL);
			}

			cm->mission_loop_formula = -1;
			if ( optional_string("+Formula:") ) {
				cm->mission_loop_formula = get_sexp_main();
				if ( !Fred_running ) {
					Assert ( cm->mission_loop_formula != -1 );
					sexp_mark_persistent( cm->mission_loop_formula );

				} else {
					if ( cm->mission_loop_formula == -1 ){
						// close localization
						lcl_ext_close();

						Campaign_load_failure = CAMPAIGN_ERROR_SEXP_EXHAUSTED;
						return CAMPAIGN_ERROR_SEXP_EXHAUSTED;
					}
				}
			}

			cm->level = 0;
			if (optional_string("+Level:")) {
				stuff_int( &cm->level );
				if ( cm->level == 0 )  // check if the top (root) of the whole tree
					Campaign.next_mission = Campaign.num_missions;

			} else
				Campaign.realign_required = 1;

			cm->pos = 0;
			if (optional_string("+Position:"))
				stuff_int( &cm->pos );
			else
				Campaign.realign_required = 1;

			if (Fred_running) {
				cm->num_goals = -1;
				cm->num_events = -1;
				cm->num_variables = -1;

			} else {
				cm->num_goals = 0;
				cm->num_events = 0;
				cm->num_variables = 0;
			}

			cm->goals = NULL;
			cm->events = NULL;
			cm->variables = NULL;

			Campaign.num_missions++;
		}
	}

	// close localization
	lcl_ext_close();

	// set up the other variables for the campaign stuff.  After initializing, we must try and load
	// the campaign save file for this player.  Since all campaign loads go through this routine, I
	// think this place should be the only necessary place to load the campaign save stuff.  The campaign
	// save file will get written when a mission has ended by player choice.
	Campaign.next_mission = 0;
	Campaign.prev_mission = -1;
	Campaign.current_mission = -1;
	Campaign.loop_mission = CAMPAIGN_LOOP_MISSION_UNINITIALIZED;
	Campaign.num_missions_completed = 0;

	// loading the campaign will get us to the current and next mission that the player must fly
	// plus load all of the old goals that future missions might rely on.
	if (!Fred_running && load_savefile && (Campaign.type == CAMPAIGN_TYPE_SINGLE)) {
		// savefile can fail to load for numerous otherwise non-fatal reasons
		// if it doesn't load in that case then it will be (re)created at save
		if ( !Pilot.load_savefile(Campaign.filename) ) {
			// but if the data is invalid for the savefile then it is fatal
			if ( Pilot.is_invalid() ) {
				Campaign.filename[0] = 0;
				Campaign.num_missions = 0;
				Campaign_load_failure = CAMPAIGN_ERROR_SAVEFILE;
				return CAMPAIGN_ERROR_SAVEFILE;
			} else {
				Pilot.save_savefile();
			}
		}
	}

	// all is good here, move along
	Campaign_file_missing = 0;

	return 0;
}

/**
 * Loads up a freespace campaign given the filename.  
 * This routine is used to load up campaigns when a pilot file is loaded.  
 * Generally, the filename will probably be the freespace campaign file, but not necessarily.
 */
int mission_campaign_load_by_name( char *filename )
{
	char name[NAME_LENGTH],test[5];
	int type,max_players;

	// make sure to tack on .fsc on the end if its not there already
	if(filename[0] != '\0'){
		if(strlen(filename) > 4){
			strcpy_s(test,filename+(strlen(filename)-4));
			if(strcmp(test, FS_CAMPAIGN_FILE_EXT)!=0){
				strcat(filename, FS_CAMPAIGN_FILE_EXT);
			}
		} else {
			strcat(filename, FS_CAMPAIGN_FILE_EXT);
		}
	} else {
		Error(LOCATION,"Tried to load campaign file with illegal length/extension!");
	}

	if (!mission_campaign_get_info(filename, name, &type, &max_players)){
		return -1;	
	}

	Num_campaigns = 0;
	Campaign_file_names[Num_campaigns] = vm_strdup(filename);
	Campaign_names[Num_campaigns] = vm_strdup(name);
	Num_campaigns++;
	mission_campaign_load(filename);		
	return 0;
}

int mission_campaign_load_by_name_csfe( char *filename, char *callsign )
{
	Game_mode |= GM_NORMAL;
	strcpy_s(Player->callsign, callsign);
	return mission_campaign_load_by_name( filename);
}

/*
 * initialise Player_loadout with default values
 */
void player_loadout_init()
{
	int i = 0, j = 0;

	memset(Player_loadout.filename, 0, sizeof(Player_loadout.filename));
	memset(Player_loadout.last_modified, 0, sizeof(Player_loadout.last_modified));

	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		Player_loadout.ship_pool[i] = 0;
	}

	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Player_loadout.weapon_pool[i] = 0;
	}

	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		Player_loadout.unit_data[i].ship_class = -1;

		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			Player_loadout.unit_data[i].wep[j] = 0;
			Player_loadout.unit_data[i].wep_count[j] = 0;
		}
	}

}

/**
 * Initializes some variables then loads the default FreeSpace single player campaign.
 */
void mission_campaign_init()
{
	mission_campaign_clear();

	Campaign_file_missing = 0;

	player_loadout_init();
}

/**
 * Fill in the root of the campaign save filename
 */
void mission_campaign_savefile_generate_root(char *filename, player *pl)
{
	char base[_MAX_FNAME];

	Assert ( strlen(Campaign.filename) != 0 ); //-V805

	if (pl == NULL) {
		Assert((Player_num >= 0) && (Player_num < MAX_PLAYERS));
		pl = &Players[Player_num];
	}

	Assert( pl != NULL );

	// build up the filename for the save file.  There could be a problem with filename length,
	// but this problem can get fixed in several ways -- ignore the problem for now though.
	_splitpath( Campaign.filename, NULL, NULL, base, NULL );

	Assert ( (strlen(base) + strlen(pl->callsign) + 1) < _MAX_FNAME );

	sprintf( filename, NOX("%s.%s."), pl->callsign, base );
}

/**
 * The following function always only ever ever ever called by CSFE!!!!!
 */
int campaign_savefile_save(char *pname)
{
	if (Campaign.type == CAMPAIGN_TYPE_SINGLE)
		Game_mode &= ~GM_MULTIPLAYER;
	else
		Game_mode |= GM_MULTIPLAYER;

	strcpy_s(Player->callsign, pname);
	
	return (int)Pilot.save_savefile();
}


// the below two functions is internal to this module.  It is here so that I can group the save/load
// functions together.
//

/**
 * Deletes any save file in the players directory for the given
 * campaign filename
 */
void mission_campaign_savefile_delete( char *cfilename )
{
	char filename[_MAX_FNAME], base[_MAX_FNAME];

	_splitpath( cfilename, NULL, NULL, base, NULL );

	if ( Player->flags & PLAYER_FLAGS_IS_MULTI ) {
		return;	// no such thing as a multiplayer campaign savefile
	}

	sprintf( filename, NOX("%s.%s.csg"), Player->callsign, base ); // only support the new filename here - taylor

	cf_delete( filename, CF_TYPE_PLAYERS );
}

void campaign_delete_save( char *cfn, char *pname)
{
	strcpy_s(Player->callsign, pname);
	mission_campaign_savefile_delete(cfn);
}

/**
 * Deletes all the save files for this particular pilot.  
 * Just call cfile function which will delete multiple files
 *
 * @param pilot_name Name of pilot
 */
void mission_campaign_delete_all_savefiles( char *pilot_name )
{
	int dir_type, num_files, i;
	char file_spec[MAX_FILENAME_LEN + 2], *ext;
	char filename[1024];
	int (*filter_save)(const char *filename);
	SCP_vector<SCP_string> names;

	ext = NOX(".csg");
	dir_type = CF_TYPE_PLAYERS;

	sprintf(file_spec, NOX("%s.*%s"), pilot_name, ext);

	// HACK HACK HACK HACK!!!!  cf_get_file_list is not reentrant.  Pretty dumb because it should
	// be.  I have to save any file filters
	filter_save = Get_file_list_filter;
	Get_file_list_filter = NULL;
	num_files = cf_get_file_list(names, dir_type, const_cast<char *>(file_spec));
	Get_file_list_filter = filter_save;

	for (i=0; i<num_files; i++) {
		strcpy_s(filename, names[i].c_str());
		strcat_s(filename, ext);
		cf_delete(filename, dir_type);
	}
}

/**
 * The following code only ever called by CSFE!!!!
 */
void campaign_savefile_load(char *fname, char *pname)
{
	if (Campaign.type==CAMPAIGN_TYPE_SINGLE) {
		Game_mode &= ~GM_MULTIPLAYER;
		Game_mode &= GM_NORMAL;
	}
	else
		Game_mode |= GM_MULTIPLAYER;

	strcpy_s(Player->callsign, pname);

	Pilot.load_savefile(fname);
}

/**
 * Sets up the internal veriables of the campaign structure so the player can play the next mission.
 * If there are no more missions available in the campaign, this function returns -1, else 0 if the mission was
 * set successfully
 */
int mission_campaign_next_mission()
{
	if ( (Campaign.next_mission == -1) || (Campaign.name[0] == '\0')) // will be set to -1 when there is no next mission
		return -1;

	if(Campaign.num_missions < 1)
		return -2;

	Campaign.current_mission = Campaign.next_mission;	
	strcpy_s( Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name );

	// check for end of loop.
	if (Campaign.current_mission == Campaign.loop_reentry) {
		Campaign.loop_enabled = 0;
	}

	// reset the number of persistent ships and weapons for the next campaign mission
	Num_granted_ships = 0;
	Num_granted_weapons = 0;
	return 0;
}

/**
 * Called to go to the previous mission in the campaign.  Used only for Red Alert missions
 */
int mission_campaign_previous_mission()
{
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return 0;

	if ( Campaign.prev_mission == -1 )
		return 0;

	Campaign.current_mission = Campaign.prev_mission;
	Campaign.next_mission = Campaign.current_mission;
	Campaign.num_missions_completed--;
	Campaign.missions[Campaign.next_mission].completed = 0;

	Pilot.save_savefile();

	// reset the player stats to be the stats from this level
	Player->stats.assign( Campaign.missions[Campaign.current_mission].stats );

	strcpy_s( Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name );
	Num_granted_ships = 0;
	Num_granted_weapons = 0;

	return 1;
}

/**
 * Evaluate next campaign mission - set as Campaign.next_mission.  Also set Campaign.loop_mission
 */
void mission_campaign_eval_next_mission()
{
	Campaign.next_mission = -1;
	int cur = Campaign.current_mission;

	// evaluate next mission (straight path)
	if (Campaign.missions[cur].formula != -1) {
		flush_sexp_tree(Campaign.missions[cur].formula);  // force formula to be re-evaluated
		eval_sexp(Campaign.missions[cur].formula);  // this should reset Campaign.next_mission to proper value
	}

	// evaluate mission loop mission (if any) so it can be used if chosen
	if ( Campaign.missions[cur].flags & CMISSION_FLAG_HAS_LOOP ) {
		int copy_next_mission = Campaign.next_mission;
		// Set temporarily to -1 so we know if loop formula fails to assign
		Campaign.next_mission = -1;
		if (Campaign.missions[cur].mission_loop_formula != -1) {
			flush_sexp_tree(Campaign.missions[cur].mission_loop_formula);  // force formula to be re-evaluated
			eval_sexp(Campaign.missions[cur].mission_loop_formula);  // this should reset Campaign.next_mission to proper value
		}

		Campaign.loop_mission = Campaign.next_mission;
		Campaign.next_mission = copy_next_mission;
	}

	if (Campaign.next_mission == -1) {
		nprintf(("allender", "No next mission to proceed to.\n"));
	} else {
		nprintf(("allender", "Next mission is number %d [%s]\n", Campaign.next_mission, Campaign.missions[Campaign.next_mission].name));
	}

}

/**
 * Store mission's goals and events in Campaign struct
 */
void mission_campaign_store_goals_and_events_and_variables()
{
	char *name;
	int cur, i, j;
	cmission *mission;

	cur = Campaign.current_mission;
	name = Campaign.missions[cur].name;

	mission = &Campaign.missions[cur];

	// first we must save the status of the current missions goals in the campaign mission structure.
	// After that, we can determine which mission is tagged as the next mission.  Finally, we
	// can save the campaign save file
	// we might have goal and event status if the player replayed a mission
	if ( mission->goals != NULL ) {
		vm_free( mission->goals );
		mission->goals = NULL;
	}

	mission->num_goals = Num_goals;
	if ( mission->num_goals > 0 ) {
		mission->goals = (mgoal *)vm_malloc( sizeof(mgoal) * Num_goals );
		Assert( mission->goals != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_goals; i++ ) {
		if (Mission_goals[i].name[0] == '\0') {
			char goal_name[NAME_LENGTH];

			sprintf(goal_name, NOX("Goal #%d"), i);
			strcpy_s( mission->goals[i].name, goal_name);
		} else
			strcpy_s( mission->goals[i].name, Mission_goals[i].name );
		Assert ( Mission_goals[i].satisfied != GOAL_INCOMPLETE );		// should be true or false at this point!!!
		mission->goals[i].status = (char)Mission_goals[i].satisfied;
	}

	// do the same thing for events as we did for goals
	// we might have goal and event status if the player replayed a mission
	if ( mission->events != NULL ) {
		vm_free( mission->events );
		mission->events = NULL;
	}

	mission->num_events = Num_mission_events;
	if ( mission->num_events > 0 ) {
		mission->events = (mevent *)vm_malloc( sizeof(mevent) * Num_mission_events );
		Assert( mission->events != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_mission_events; i++ ) {
 		if (Mission_events[i].name[0] == '\0') {
			char event_name[NAME_LENGTH];

			sprintf(event_name, NOX("Event #%d"), i);
			nprintf(("Warning", "Mission goal in mission %s must have a +Name field! using %s for campaign save file\n", mission->name, name));
			strcpy_s( mission->events[i].name, event_name);
		} else
			strcpy_s( mission->events[i].name, Mission_events[i].name );

		// getting status for the events is a little different.  If the formula value for the event entry
		// is -1, then we know the value of the result field will never change.  If the formula is
		// not -1 (i.e. still being evaluated at mission end time), we will write "incomplete" for the
		// event evaluation
		if ( Mission_events[i].formula == -1 ) {
			if ( Mission_events[i].result )
				mission->events[i].status = EVENT_SATISFIED;
			else
				mission->events[i].status = EVENT_FAILED;
		} else
			Int3();
	}

	// Goober5000 - handle campaign-persistent variables -------------------------------------
	if (mission->variables != NULL) {
		vm_free( mission->variables );
		mission->variables = NULL;
	}
/*
	mission->num_variables = sexp_campaign_persistent_variable_count();
	if ( mission->num_variables > 0) {
		mission->variables = (sexp_variable *)vm_malloc( sizeof(sexp_variable) * mission->num_variables);
		Assert( mission->variables != NULL );
	}

	// copy the needed variable info
	j=0;
	for (i = 0; i < sexp_variable_count(); i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT)
		{
			mission->variables[j].type = Sexp_variables[i].type;
			strcpy_s(mission->variables[j].text, Sexp_variables[i].text);
			strcpy_s(mission->variables[j].variable_name, Sexp_variables[i].variable_name);
			j++;
		}
	}
*/
	int num_mission_variables = sexp_campaign_persistent_variable_count();

	if (num_mission_variables > 0) {
		int variable_count = 0;
		int total_variables = Campaign.num_variables;
		int matching_variables = 0;
		int persistent_variables_in_mission = 0;

		// get count of new variables
		for (i = 0; i < sexp_variable_count(); i++) {
			if ( Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT ) {
				persistent_variables_in_mission++;

				// see if we already have a variable with this name
				for (j = 0; j < Campaign.num_variables; j++) {
					if (!(stricmp(Sexp_variables[i].variable_name, Campaign.variables[j].variable_name) )) {
						matching_variables++;
						break;
					}
				}
			}
		}
		
		Assert(persistent_variables_in_mission >= matching_variables); 
		total_variables += (persistent_variables_in_mission - matching_variables);

		// allocate new storage
		sexp_variable *n_variables = (sexp_variable *) vm_malloc(total_variables * sizeof(sexp_variable));
		Assert( n_variables );

		// copy existing variables over
		if (Campaign.num_variables > 0) {
			Assert( Campaign.variables );
			memcpy(n_variables, Campaign.variables, Campaign.num_variables * sizeof(sexp_variable));

			variable_count = Campaign.num_variables;

			vm_free(Campaign.variables);
			Campaign.variables = NULL;
		}

		// update/add variables
		for (i = 0; i < sexp_variable_count(); i++) {
			if ( !(Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) ) {
				continue;
			}

			bool add_it = true;

			// maybe update...
			for (j = 0; j < Campaign.num_variables; j++) {
				if ( !stricmp(Sexp_variables[i].variable_name, n_variables[j].variable_name) ) {
					n_variables[j].type = Sexp_variables[i].type;
					strcpy_s(n_variables[j].text, Sexp_variables[i].text);
					add_it = false;
					break;
				}
			}

			// otherwise add...
			if (add_it) {
				n_variables[variable_count].type = Sexp_variables[i].type;
				strcpy_s(n_variables[variable_count].text, Sexp_variables[i].text);
				strcpy_s(n_variables[variable_count].variable_name, Sexp_variables[i].variable_name);
				variable_count++;
			}
		}

		Assert( variable_count == total_variables );
		Assert( Campaign.variables == NULL );

		// update with new data/count
		Campaign.variables = n_variables;
		Campaign.num_variables = total_variables;
	}
	// --------------------------------------------------------------------------
}

/**
 * Called when the player's mission is over.  It updates the internal store of goals
 * and their status then saves the state of the campaign in the campaign file.  
 * This gets called after player accepts mission results in debriefing.
 */
void mission_campaign_mission_over(bool do_next_mission)
{
	int mission_num, i;
	cmission *mission;

	// I don't think that we should have a record for these -- maybe we might??????  If we do,
	// then we should free them
	if ( !(Game_mode & GM_CAMPAIGN_MODE) ){
		return;
	}

	mission_num = Campaign.current_mission;
	Assert( mission_num != -1 );
	mission = &Campaign.missions[mission_num];

	// determine if any ships/weapons were granted this mission
	for ( i=0; i<Num_granted_ships; i++ ){
		Campaign.ships_allowed[Granted_ships[i]] = 1;
	}

	for ( i=0; i<Num_granted_weapons; i++ ){
		Campaign.weapons_allowed[Granted_weapons[i]] = 1;	
	}

	// Goober5000 - player-persistent variables are handled when the mission is
	// over, not necessarily when the mission is accepted
	Player->failures_this_session = 0;

	// update campaign.mission stats (used to allow backout inRedAlert)
	// .. but we don't do this if we are inside of the prev/current loop hack
	if ( Campaign.prev_mission != Campaign.current_mission ) {
		mission->stats.assign( Player->stats );
		if(!(Game_mode & GM_MULTIPLAYER)){
			scoring_backout_accept( &mission->stats );
		}
	}

	// if we are moving to a new mission, then change our data.  If we are staying on the same mission,
	// then we don't want to do anything.  Remove information about goals/events
	if ( Campaign.next_mission != mission_num ) {
		Campaign.prev_mission = mission_num;
		Campaign.current_mission = -1;

		// very minor support for non-linear campaigns - taylor
		if (Campaign.missions[mission_num].completed != 1) {
			Campaign.num_missions_completed++;
			Campaign.missions[mission_num].completed = 1;
		}

		// save the scoring values from the previous mission at the start of this mission -- for red alert

		// save the state of the campaign in the campaign save file and move to the end_game state
		if (Campaign.type == CAMPAIGN_TYPE_SINGLE) {
			Pilot.save_savefile();
		}

	} else {
		// free up the goals and events which were just malloced.  It's kind of like erasing any fact
		// that the player played this mission in the campaign.
		if (mission->goals != NULL) {
			vm_free( mission->goals );
			mission->goals = NULL;
		}
		mission->num_goals = 0;

		if (mission->events != NULL) {
			vm_free( mission->events );
			mission->events = NULL;
		}
		mission->num_events = 0;

		// Goober5000
		if (mission->variables != NULL) {
			vm_free( mission->variables );
			mission->variables = NULL;
		}
		mission->num_variables = 0;

		Sexp_nodes[mission->formula].value = SEXP_UNKNOWN;
	}

	if (do_next_mission)
		mission_campaign_next_mission();			// sets up whatever needs to be set to actually play next mission
}

/**
 * Called when the game closes -- to get rid of memory errors for Bounds checker
 * also called at campaign init and campaign load
 */
void mission_campaign_clear()
{
	int i;

	if (Campaign.desc != NULL) {
		vm_free(Campaign.desc);
		Campaign.desc = NULL;
	}

	// be sure to remove all old malloced strings of Mission_names
	// we must also free any goal stuff that was from a previous campaign
	for ( i=0; i<Campaign.num_missions; i++ ) {
		if ( Campaign.missions[i].name != NULL ) {
			vm_free(Campaign.missions[i].name);
			Campaign.missions[i].name = NULL;
		}

		if (Campaign.missions[i].notes != NULL) {
			vm_free(Campaign.missions[i].notes);
			Campaign.missions[i].notes = NULL;
		}

		if ( Campaign.missions[i].goals != NULL ) {
			vm_free ( Campaign.missions[i].goals );
			Campaign.missions[i].goals = NULL;
		}

		if ( Campaign.missions[i].events != NULL ) {
			vm_free ( Campaign.missions[i].events );
			Campaign.missions[i].events = NULL;
		}

		// Goober5000
		if ( Campaign.missions[i].variables != NULL ) {
			vm_free ( Campaign.missions[i].variables );
			Campaign.missions[i].variables = NULL;
		}

		// the next three are strdup'd return values from parselo.cpp - taylor
		if (Campaign.missions[i].mission_branch_desc != NULL) {
			vm_free(Campaign.missions[i].mission_branch_desc);
			Campaign.missions[i].mission_branch_desc = NULL;
		}

		if (Campaign.missions[i].mission_branch_brief_anim != NULL) {
			vm_free(Campaign.missions[i].mission_branch_brief_anim);
			Campaign.missions[i].mission_branch_brief_anim = NULL;
		}

		if (Campaign.missions[i].mission_branch_brief_sound != NULL) {
			vm_free(Campaign.missions[i].mission_branch_brief_sound);
			Campaign.missions[i].mission_branch_brief_sound = NULL;
 		}

		if ( !Fred_running ){
			sexp_unmark_persistent(Campaign.missions[i].formula);		// free any sexpression nodes used by campaign.
		}

		memset(Campaign.missions[i].briefing_cutscene, 0, NAME_LENGTH);
		Campaign.missions[i].formula = 0;
		Campaign.missions[i].completed = 0;
		Campaign.missions[i].num_goals = 0;
		Campaign.missions[i].num_events = 0;
		Campaign.missions[i].num_variables = 0;	// Goober5000
		Campaign.missions[i].mission_loop_formula = 0;
		Campaign.missions[i].level = 0;
		Campaign.missions[i].pos = 0;
		Campaign.missions[i].flags = 0;
		Campaign.missions[i].main_hall = "";
		Campaign.missions[i].debrief_persona_index = 0;

		Campaign.missions[i].stats.init();
	}

	memset(Campaign.name, 0, NAME_LENGTH);
	memset(Campaign.filename, 0, MAX_FILENAME_LEN);
	Campaign.type = 0;
	Campaign.flags = 0;
	Campaign.num_missions = 0;
	Campaign.num_missions_completed = 0;
	Campaign.current_mission = -1;
	Campaign.next_mission = -1;
	Campaign.prev_mission = -1;
	Campaign.loop_enabled = 0;
	Campaign.loop_mission = CAMPAIGN_LOOP_MISSION_UNINITIALIZED;
	Campaign.loop_reentry = 0;
	Campaign.realign_required = 0;
	Campaign.num_players = 0;
	memset( Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed) );
	memset( Campaign.weapons_allowed, 0, sizeof(Campaign.weapons_allowed) );
	Campaign.num_variables = 0;
	if (Campaign.variables != NULL) {
		vm_free(Campaign.variables);
		Campaign.variables = NULL;
	}
}

/**
 * Extract the mission filenames for a campaign.  
 *
 * @param filename	Name of campaign file
 * @param dest		Storage for the mission filename, must be already allocated
 * @param num		Output parameter for the number of mission filenames in the campaign
 *
 * note that dest should allocate at least dest[MAX_CAMPAIGN_MISSIONS][NAME_LENGTH]
 */
int mission_campaign_get_filenames(char *filename, char dest[][NAME_LENGTH], int *num)
{
	int	rval;

	// read the mission file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		return rval;

	} else {
		Assert( strlen(filename) < MAX_FILENAME_LEN );  // make sure no overflow
		read_file_text(filename);
		reset_parse();

		required_string("$Name:");
		advance_to_eoln(NULL);

		required_string( "$Type:" );
		advance_to_eoln(NULL);

		// parse the mission file and actually read in the mission stuff
		*num = 0;
		while ( skip_to_string("$Mission:") == 1 ) {
			stuff_string(dest[*num], F_NAME, NAME_LENGTH);
			(*num)++;
		}
	}

	return 0;
}

/**
 * Read the goals and events from a mission in a campaign file and store that information
 * in the campaign structure for use in the campaign editor, error checking, etc
 */
void read_mission_goal_list(int num)
{
	char *filename, notes[NOTES_LENGTH], goals[MAX_GOALS][NAME_LENGTH];
	char events[MAX_MISSION_EVENTS][NAME_LENGTH];
	int i, z, rval, event_count, count = 0;

	filename = Campaign.missions[num].name;

	// open localization
	lcl_ext_open();	
	
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename);
	reset_parse();
	init_sexp();

	// first, read the mission notes for this mission.  Used in campaign editor
	if (skip_to_string("#Mission Info")) {
		if (skip_to_string("$Notes:")) {
			stuff_string(notes, F_NOTES, NOTES_LENGTH);
			if (Campaign.missions[num].notes){
				vm_free(Campaign.missions[num].notes);
			}

			Campaign.missions[num].notes = (char *) vm_malloc(strlen(notes) + 1);
			strcpy(Campaign.missions[num].notes, notes);
		}
	}

	event_count = 0;
	// skip to events section in the mission file.  Events come before goals, so we process them first
	if ( skip_to_string("#Events") ) {
		while (1) {
			if (skip_to_string("$Formula:", "#Goals") != 1){
				break;
			}

			z = skip_to_string("+Name:", "$Formula:");
			if (!z){
				break;
			}

			if (z == 1){
				stuff_string(events[event_count], F_NAME, NAME_LENGTH);
			} else {
				sprintf(events[event_count], NOX("Event #%d"), event_count + 1);
			}

			event_count++;
			if (event_count > MAX_MISSION_EVENTS) {
				Warning(LOCATION, "Maximum number of events exceeded (%d)!", MAX_MISSION_EVENTS);
				event_count = MAX_MISSION_EVENTS;
				break;
			}
		}
	}

	count = 0;
	if (skip_to_string("#Goals")) {
		while (1) {
			if (skip_to_string("$Type:", "#End") != 1){
				break;
			}

			z = skip_to_string("+Name:", "$Type:");
			if (!z){
				break;
			}

			if (z == 1){
				stuff_string(goals[count], F_NAME, NAME_LENGTH);
			} else {
				sprintf(goals[count], NOX("Goal #%d"), count + 1);
			}

			count++;
			if (count > MAX_GOALS) {
				Warning(LOCATION, "Maximum number of goals exceeded (%d)!", MAX_GOALS);
				count = MAX_GOALS;
				break;
			}
		}
	}

	Campaign.missions[num].num_goals = count;
	if (count) {
		Campaign.missions[num].goals = (mgoal *) vm_malloc(count * sizeof(mgoal));
		Assert(Campaign.missions[num].goals);  // make sure we got the memory
		memset(Campaign.missions[num].goals, 0, count * sizeof(mgoal));

		for (i=0; i<count; i++){
			strcpy_s(Campaign.missions[num].goals[i].name, goals[i]);
		}
	}
		// copy the events
	Campaign.missions[num].num_events = event_count;
	if (event_count) {
		Campaign.missions[num].events = (mevent *)vm_malloc(event_count * sizeof(mevent));
		Assert ( Campaign.missions[num].events );
		memset(Campaign.missions[num].events, 0, event_count * sizeof(mevent));

		for (i = 0; i < event_count; i++ ){
			strcpy_s(Campaign.missions[num].events[i].name, events[i]);
		}
	}

	// Goober5000 - variables do not need to be read here

	// close localization
	lcl_ext_close();
}

/**
 * Return index into Campaign's list of missions of the mission with the given
 * filename.  
 *
 * This function tried to be a little smart about filename looking for the .fsm
 * extension since filenames are stored with the extension in the campaign file.
 *
 * @return index of mission in campaign structure.  -1 if mission name not found.
 */
int mission_campaign_find_mission( char *name )
{
	int i;
	char realname[_MAX_PATH];

	if (name == NULL)
		return -1;

	// look for an extension on the file.  If no extension, add default ".fsm" onto the
	// end of the filename
	strcpy_s(realname, name );
	if ( strchr(name, '.') == NULL ){
		sprintf(realname, NOX("%s%s"), name, FS_MISSION_FILE_EXT );
	}

	for (i = 0; i < Campaign.num_missions; i++ ) {
		if ( !stricmp(realname, Campaign.missions[i].name) ){
			return i;
		}
	}

	return -1;
}

void mission_campaign_maybe_play_movie(int type)
{
	int mission;
	char *filename;

	// only support pre mission movies for now.
	Assert ( type == CAMPAIGN_MOVIE_PRE_MISSION );

	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	mission = Campaign.current_mission;
	Assert( mission != -1 );

	// get a possible filename for a movie to play.
	filename = NULL;
	switch( type ) {
	case CAMPAIGN_MOVIE_PRE_MISSION:
		if ( strlen(Campaign.missions[mission].briefing_cutscene) )
			filename = Campaign.missions[mission].briefing_cutscene;
		break;

	default:
		Int3();
		break;
	}

	// no filename, no movie!
	if ( !filename )
		return;

	movie_play( filename );	//Play the movie!
	cutscene_mark_viewable( filename );
}

/**
 * Return the type of campaign of the passed filename
 */
int mission_campaign_parse_is_multi(char *filename, char *name)
{	
	int i, rval;
	char temp[NAME_LENGTH];
	
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		return -1;
	}

	read_file_text( filename );
	reset_parse();
	
	required_string("$Name:");
	stuff_string( temp, F_NAME, NAME_LENGTH );	
	if ( name )
		strcpy( name, temp );

	required_string( "$Type:" );
	stuff_string( temp, F_NAME, NAME_LENGTH );

	for (i = 0; i < MAX_CAMPAIGN_TYPES; i++ ) {
		if ( !stricmp(temp, campaign_types[i]) ) {
			return i;
		}
	}

	Error(LOCATION, "Unknown campaign type %s", temp );
	return -1;
}

/** 
 * Save persistent information during a mission -- which then might or might not get
 * saved out when the mission is over depending on whether player replays mission or commits.
 */
void mission_campaign_save_persistent( int type, int sindex )
{
	// based on the type of information, save it off for possible saving into the campsign
	// savefile when the mission is over
	if ( type == CAMPAIGN_PERSISTENT_SHIP ) {
		Assert( Num_granted_ships < MAX_SHIP_CLASSES );
		Granted_ships[Num_granted_ships] = sindex;
		Num_granted_ships++;
	} else if ( type == CAMPAIGN_PERSISTENT_WEAPON ) {
		Assert( Num_granted_weapons < MAX_WEAPON_TYPES );
		Granted_weapons[Num_granted_weapons] = sindex;
		Num_granted_weapons++;
	} else
		Int3();
}

bool campaign_is_ignored(const char *filename)
{
	SCP_string filename_no_ext = filename;
	drop_extension(filename_no_ext);

	for (SCP_vector<SCP_string>::iterator ii = Ignored_campaigns.begin(); ii != Ignored_campaigns.end(); ++ii) {
		if (ii->compare(filename_no_ext) == 0) {
			return true;
		}
	}
	
	return false;
}

// returns 0: loaded, !0: error
int mission_load_up_campaign( player *pl )
{
	int rc = -1, idx;

	if ( pl == NULL )
		pl = Player;

	// find best campaign to use:
	//   1) last used
	//   2) builtin
	//   3) anything else

	// last used...
	if ( strlen(pl->current_campaign) ) {
		if (!campaign_is_ignored(pl->current_campaign)) {
			return mission_campaign_load(pl->current_campaign, pl);
		}
		else {
			Campaign_file_missing = 1;
		}
	}

	rc = mission_campaign_load(Default_campaign_file_name, pl);

	// builtin...
	if (rc < 0) {
		rc = mission_campaign_load(Default_campaign_file_name, pl);
	}

	// everything else...
	if (rc < 0) {
		// no descriptions, no sorting
		mission_campaign_build_list(false, false);

		for (idx = 0; (idx < Num_campaigns) && (rc < 0); idx++) {
			if ( (Campaign_file_names[idx] == NULL) || !strlen(Campaign_file_names[idx]) ) {
				continue;
			}

			// skip current and builtin since they already didn't work
			if ( !stricmp(Campaign_file_names[idx], pl->current_campaign) ) {
				continue;
			}

			if ( !stricmp(Campaign_file_names[idx], BUILTIN_CAMPAIGN) ) {
				continue;
			}

			// try to load it, whatever "it" is
			rc = mission_campaign_load(Campaign_file_names[idx], pl);
		}

		mission_campaign_free_list();
	}
	
	// update pilot with the new current campaign
	if (rc == 0) {
		strcpy_s(pl->current_campaign, Campaign.filename);
	}

	return rc;
}

/**
 * For end of campaign in the single player game.  Called when the end of campaign state is
 * entered, which is triggered when the end-campaign sexpression is hit
 */
void mission_campaign_end_init()
{
	// no need to do any initialization.
}

void mission_campaign_end_do()
{
	// close out the mission
	event_music_level_close();
	mission_goal_fail_incomplete();
	scoring_level_close();
	mission_campaign_mission_over();

	// play the movies
	// eventually we'll want to play one of two options (good ending or bad ending)

	// this is specific to the FreeSpace 2 single-player campaign
	if (!stricmp(Campaign.filename, "freespace2")) {
		// did the supernova blow?
		if (Supernova_status == SUPERNOVA_HIT) {
			movie_play_two("endpart1.mve", "endprt2b.mve");			// bad ending
		} else {
			movie_play_two("endpart1.mve", "endprt2a.mve");			// good ending
		}
	} else {
		common_maybe_play_cutscene(MOVIE_END_CAMPAIGN);
	}

	gameseq_post_event( GS_EVENT_MAIN_MENU );
}

void mission_campaign_end_close()
{
	// no need to do any initialization.
}


/**
 * Skip to the next mission in the campaign
 * this also posts the state change by default.  pass 0 to override that
 */
void mission_campaign_skip_to_next(int start_game)
{
	// mark all goals/events complete
	// these do not really matter, since is-previous-event-* and is-previous-goal-* sexps check
	// to see if the mission was skipped, and use defaults accordingly.
	mission_goal_mark_objectives_complete();
	mission_goal_mark_events_complete();

	// mark mission as skipped
	Campaign.missions[Campaign.current_mission].flags |= CMISSION_FLAG_SKIPPED;

	// store
	mission_campaign_store_goals_and_events_and_variables();

	// now set the next mission
	mission_campaign_eval_next_mission();

	// clear out relevant player vars
	Player->failures_this_session = 0;
	Player->show_skip_popup = 1;

	if (start_game) {
		// proceed to next mission or main hall
		if ((Campaign.missions[Campaign.current_mission].flags & CMISSION_FLAG_HAS_LOOP) && (Campaign.loop_mission != -1)) {
			// go to loop solicitation
			gameseq_post_event(GS_EVENT_LOOP_BRIEF);
		} else {
			// closes out mission stuff, sets up next one
			mission_campaign_mission_over();

			if ( Campaign.next_mission == -1 ) {
				// go to main hall, tha campaign is over!
				gameseq_post_event(GS_EVENT_MAIN_MENU);
			} else {
				// go to next mission
				gameseq_post_event(GS_EVENT_START_GAME);
			}
		}
	}
}

/**
 * Breaks your ass out of the loop
 * this also posts the state change
 */
void mission_campaign_exit_loop()
{
	// set campaign to loop reentry point
	Campaign.next_mission = Campaign.loop_reentry;
	Campaign.current_mission = -1;
	Campaign.loop_enabled = 0;

	// set things up for next mission
	mission_campaign_next_mission();
	gameseq_post_event(GS_EVENT_START_GAME);
}


/**
 * Used for jumping to a particular campaign mission
 * all previous missions marked skipped
 * this relies on correct mission ordering in the campaign file
 */
void mission_campaign_jump_to_mission(char *name)
{
	int i = 0, mission_num = -1;
	char dest_name[64], *p;

	// load in the campaign junk
	mission_load_up_campaign();

	// tack the .fs2 onto the input name
	strcpy_s(dest_name, name);
	p = strchr(dest_name, '.');
	if (p != NULL)
		*p = '\0';
	strcat_s(dest_name, ".fs2");

	// search for our mission
	for (i = 0; i < Campaign.num_missions; i++) {
		if ((Campaign.missions[i].name != NULL) && !stricmp(Campaign.missions[i].name, dest_name)) {
			mission_num = i;
			break;
		} else {
			Campaign.missions[i].flags |= CMISSION_FLAG_SKIPPED;
			Campaign.num_missions_completed = i;
		}
	}

	if (mission_num < 0) {
		// if we got here, no match was found
		// restart the campaign
		mission_campaign_savefile_delete(Campaign.filename);
		mission_campaign_load(Campaign.filename);
	} else {
		for (i = 0; i < MAX_SHIP_CLASSES; i++) {
			Campaign.ships_allowed[i] = 1;
		}
		for (i = 0; i < MAX_WEAPON_TYPES; i++) {
			Campaign.weapons_allowed[i] = 1;
		}

		Campaign.next_mission = mission_num;
		Campaign.prev_mission = mission_num - 1;
		mission_campaign_next_mission();
		Game_mode |= GM_CAMPAIGN_MODE;

		gameseq_post_event(GS_EVENT_START_GAME);
	}
}

// Goober5000
void mission_campaign_save_player_persistent_variables()
{
	int i;

	// make sure we are actually playing a campaign
	if (!(Game_mode & GM_CAMPAIGN_MODE))
		return;

	// make sure this is a single-player campaign
	if (!(Campaign.type == CAMPAIGN_TYPE_SINGLE))
		return;

	// now save variables
	for (i = 0; i < sexp_variable_count(); i++) {
		// we only want the player persistent ones
		if ( !(Sexp_variables[i].type & SEXP_VARIABLE_PLAYER_PERSISTENT) ) {
			continue;
		}

		bool found = false;

		// check if variable already exists and updated it
		for (size_t j = 0; j < Player->variables.size(); j++) {
			if ( !(stricmp(Sexp_variables[i].variable_name, Player->variables[j].variable_name)) ) {
				Player->variables[j] = Sexp_variables[i];

				found = true;
				break;
			}
		}

		// if not found then add new entry
		if ( !found ) {
			Player->variables.push_back( Sexp_variables[i] );
		}
	}
}

void mission_campaign_load_failure_popup()
{
	if (Campaign_load_failure == 0) {
		return;
	}

	if (Campaign_load_failure == CAMPAIGN_ERROR_CORRUPT) {
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR("Error!\n\nRequested campaign is corrupt and cannot be loaded.\n\n"
			"Please select a different campaign in the Campaign Room.", 1614));
	} else if (Campaign_load_failure == CAMPAIGN_ERROR_SEXP_EXHAUSTED) {
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR("Error!\n\nRequested campaign requires too many SEXPs and cannot be loaded.\n\n"
			"Please select a different campaign in the Campaign Room.", 1615));
	} else if (Campaign_load_failure == CAMPAIGN_ERROR_MISSING) {
		// if it's just the campaign missing, there's another popup to deal with that
		;
	} else if (Campaign_load_failure == CAMPAIGN_ERROR_SAVEFILE) {
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR("Error!\n\nThe pilot savefile "
			"for this campaign is invalid for the current mod.\n\nPlease select another campaign or switch to the correct "
			"mod in order to use this campaign.", 1617));
	}

	Campaign_load_failure = 0;
}

