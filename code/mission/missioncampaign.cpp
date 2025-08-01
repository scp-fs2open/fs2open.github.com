/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <cstdio>
#include <cstring>
#include <csetjmp>
#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

#ifdef SCP_UNIX
#include <sys/stat.h>
#include <glob.h>
#endif

#include "freespace.h"
#include "missioncampaign.h"
#include "cfile/cfile.h"
#include "cutscene/cutscenes.h"
#include "cutscene/movie.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "localization/localize.h"
#include "menuui/mainhallmenu.h"
#include "menuui/techmenu.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "missionui/missionscreencommon.h"
#include "missionui/redalert.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "scripting/global_hooks.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "starfield/supernova.h"
#include "ui/ui.h"
#include "utils/string_utils.h"
#include "weapon/weapon.h"

// campaign wasn't ended
int Campaign_ending_via_supernova = 0;

// stuff for selecting campaigns.  We need to keep both arrays around since we display the
// list of campaigns by name, but must load campaigns by filename
char *Campaign_names[MAX_CAMPAIGNS] = { NULL };
char *Campaign_file_names[MAX_CAMPAIGNS] = { NULL };
char *Campaign_descs[MAX_CAMPAIGNS] = { NULL };
int	Num_campaigns;
bool Campaign_file_missing = false;
int Campaign_load_failure = 0;
int Campaign_names_inited = 0;
SCP_vector<SCP_string> Ignored_campaigns;

char Default_campaign_file_name[MAX_FILENAME_LEN - 4]  = { 0 };

// stuff used for campaign list building
static bool MC_desc = false;
static bool MC_multiplayer = false;

const char *campaign_types[MAX_CAMPAIGN_TYPES] =
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


/**
 * Returns a string (which is malloced in this routine) of the name of the given freespace campaign file.  
 * In the type field, we return if the campaign is a single player or multiplayer campaign.  
 * The type field will only be valid if the name returned is non-NULL
 */
bool mission_campaign_get_info(const char *filename, SCP_string &name, int *type, int *max_players, char **desc, char **first_mission)
{
	int i, success = false;
	SCP_string campaign_type;
	char fname[MAX_FILENAME_LEN];

	Assert( type != NULL );

	// make sure outputs always have sane values
	name.clear();
	*type = -1;

	if (max_players) {
		*max_players = 0;
	}

	if (desc) {
		*desc = nullptr;
	}

	if (first_mission) {
		*first_mission = nullptr;
	}

	strncpy(fname, filename, MAX_FILENAME_LEN - 1);
	auto fname_len = strlen(fname);
	if ((fname_len < 4) || stricmp(fname + fname_len - 4, FS_CAMPAIGN_FILE_EXT) != 0){
		strcat_s(fname, FS_CAMPAIGN_FILE_EXT);
		fname_len += 4;
	}
	Assert(fname_len < MAX_FILENAME_LEN);

	do {
		try
		{
			read_file_text(fname);
			reset_parse();

			required_string("$Name:");
			stuff_string(name, F_NAME);
			if (name.empty()) {
				nprintf(("Warning", "No name found for campaign file %s\n", filename));
				break;
			}

			required_string("$Type:");
			stuff_string(campaign_type, F_NAME);

			for (i = 0; i < MAX_CAMPAIGN_TYPES; i++) {
				if (!stricmp(campaign_type.c_str(), campaign_types[i])) {
					*type = i;
				}
			}

			if (*type < 0) {
				Warning(LOCATION, "Invalid campaign type \"%s\"\n", campaign_type.c_str());
				break;
			}

			if (desc) {
				if (optional_string("+Description:")) {
					*desc = stuff_and_malloc_string(F_MULTITEXT, NULL);
				}
				else {
					*desc = NULL;
				}
			}

			// if this is a multiplayer campaign, get the max players
			if ((*type) != CAMPAIGN_TYPE_SINGLE) {
				skip_to_string("+Num Players:");
				stuff_int(max_players);
				// Cyborg17 - and the first mission name if we want it, too
				if (first_mission) {
					skip_to_string("$Mission:");
					*first_mission = stuff_and_malloc_string(F_NAME, nullptr);
				}
			}

			// if we found a valid campaign type
			if ((*type) >= 0) {
				success = true;
			}
		}
		catch (const parse::ParseException& e)
		{
			mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", fname, e.what()));
			break;
		}
	} while (0);

	return success;
}

/**
 * Parses campaign and returns a list of missions in it.  
 * @return Number of missions added to the 'list', and up to 'max' missions may be added to 'list'.  
 * @return Negative on error.
 */
int mission_campaign_get_mission_list(const char *filename, char **list, int max)
{
	int i, num = 0;
	char name[MAX_FILENAME_LEN];

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	// read the campaign file and get the list of mission filenames
	try
	{
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
	catch (const parse::ParseException& e)
	{
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));

		// since we can't return count of allocated elements, free them instead
		for (i = 0; i<num; i++)
			vm_free(list[i]);

		num = -1;
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
	SCP_string name;
	char *desc = NULL;
	int type, max_players;

	// don't add ignored campaigns
	if (campaign_is_ignored(filename)) {
		return 0;
	}

	if ( mission_campaign_get_info( filename, name, &type, &max_players, &desc) ) {
		if ( !MC_multiplayer && (type == CAMPAIGN_TYPE_SINGLE) ) {
			Campaign_names[Num_campaigns] = vm_strdup(name.c_str());

			if (MC_desc)
				Campaign_descs[Num_campaigns] = desc;

			Num_campaigns++;

			// Note that we're not freeing desc here because the pointer is getting copied to Campaign_descs which is freed later.

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

    if (optional_string("+Starting Ships:")) {
        count = (int)stuff_int_list(ship_list, MAX_SHIP_CLASSES, SHIP_INFO_TYPE);

        // now set the array elements stating which ships we are allowed
        for (i = 0; i < count; i++) {
            if (Ship_info[ship_list[i]].flags[Ship::Info_Flags::Player_ship])
                Campaign.ships_allowed[ship_list[i]] = 1;
        }
	}
	else {
		// set allowable ships to the SIF_PLAYER_SHIPs
		for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
			if (it->flags[Ship::Info_Flags::Player_ship])
				Campaign.ships_allowed[std::distance(Ship_info.cbegin(), it)] = 1;
		}
	}

    if (optional_string("+Starting Weapons:")) {
        count = (int)stuff_int_list(weapon_list, MAX_WEAPON_TYPES, WEAPON_POOL_TYPE);

        // now set the array elements stating which ships we are allowed
		for (i = 0; i < count; i++) {
			if (Weapon_info[weapon_list[i]].wi_flags[Weapon::Info_Flags::Player_allowed])
				Campaign.weapons_allowed[weapon_list[i]] = 1;
		}
    }
	else {
		// set allowable weapons to the player-allowed ones
		for (auto it = Weapon_info.cbegin(); it != Weapon_info.cend(); ++it) {
			if (it->wi_flags[Weapon::Info_Flags::Player_allowed])
				Campaign.weapons_allowed[std::distance(Weapon_info.cbegin(), it)] = 1;
		}
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
int mission_campaign_load(const char* filename, const char* full_path, player* pl, int load_savefile)
{
	int i;
	char name[NAME_LENGTH], type[NAME_LENGTH], temp[NAME_LENGTH];

	if (campaign_is_ignored(filename)) {
		Campaign_file_missing = true;
		Campaign_load_failure = CAMPAIGN_ERROR_IGNORED;
		return CAMPAIGN_ERROR_IGNORED;
	}

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	if ( pl == NULL )
		pl = Player;

	if ( !Fred_running && load_savefile && (pl == NULL) ) {
		Int3();
		load_savefile = 0;
	}

	// be sure to remove all old malloced strings of Mission_names
	// we must also free any goal stuff that was from a previous campaign
	// this also frees sexpressions so the next call to init_sexp will be able to reclaim
	// nodes previously used by another campaign.
	mission_campaign_clear();

	// read the mission file and get the list of mission filenames
	try
	{
		strcpy_s( Campaign.filename, filename );

		// only initialize the sexpression stuff when Fred isn't running.  It'll screw things up major
		// if it does
		if ( !Fred_running ){
			init_sexp();		// must initialize the sexpression stuff
		}

		read_file_text( full_path ? full_path : filename );
		reset_parse();

		// copy filename to campaign structure minus the extension
		auto len = strlen(filename) - 4;
		Assert(len + 1 < CF_MAX_PATHNAME_LENGTH);
		strncpy(Campaign.filename, filename, len);
		Campaign.filename[len] = '\0';

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

		if (optional_string("+Description:"))
			Campaign.desc = stuff_and_malloc_string(F_MULTITEXT, NULL);

		// if the type is multiplayer -- get the number of players
		if ( Campaign.type != CAMPAIGN_TYPE_SINGLE) {
			required_string("+Num players:");
			stuff_int( &(Campaign.num_players) );
		}		

		// parse flags - Goober5000
		if (optional_string("$Flags:"))
		{
			stuff_int( &(Campaign.flags) );
		}

		// parse the optional ship/weapon information
		mission_campaign_get_sw_info();

		// parse the mission file and actually read in the mission stuff
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

			// Goober5000 - substitute main hall (like substitute music)
			cm->substitute_main_hall = "";
			if (optional_string("+Substitute Main Hall:")) {
				stuff_string(temp, F_RAW, 32);
				cm->substitute_main_hall = temp;

				// if we're running FRED, keep the halls separate (so we can save the campaign file),
				// but if we're running FS, replace the main hall with the substitute right now
				if (!Fred_running) {
					// see if this main hall exists
					main_hall_defines* mhd = main_hall_get_pointer(temp);
					if (mhd != nullptr) {
						cm->main_hall = temp;
					} else {
						mprintf(("Substitute main hall '%s' not found\n", temp));
					}
				}
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

			cm->goals.clear();
			cm->events.clear();
			cm->variables.clear();
			cm->flags |= CMISSION_FLAG_FRED_LOAD_PENDING;

			Campaign.num_missions++;
		}

		if ((Game_mode & GM_MULTIPLAYER) && Campaign.num_missions > UINT8_MAX)
			throw parse::ParseException("Number of campaign missions is too high and breaks multi!");
	}
	catch (const parse::FileOpenException& foe)
	{
		mprintf(("Error opening '%s'\r\nError message = %s.\r\n", filename, foe.what()));

		Campaign.filename[0] = 0;
		Campaign.num_missions = 0;

		Campaign_file_missing = true;
		Campaign_load_failure = CAMPAIGN_ERROR_MISSING;
		return CAMPAIGN_ERROR_MISSING;
	}
	catch (const parse::ParseException& pe)
	{
		mprintf(("Error parsing '%s'\r\nError message = %s.\r\n", filename, pe.what()));

		Campaign.filename[0] = 0;
		Campaign.num_missions = 0;

		Campaign_file_missing = true;
		Campaign_load_failure = CAMPAIGN_ERROR_CORRUPT;
		return CAMPAIGN_ERROR_CORRUPT;
	}

	// set up the other variables for the campaign stuff.  After initializing, we must try and load
	// the campaign save file for this player.  Since all campaign loads go through this routine, I
	// think this place should be the only necessary place to load the campaign save stuff.  The campaign
	// save file will get written when a mission has ended by player choice.

	// loading the campaign will get us to the current and next mission that the player must fly
	// plus load all of the old goals that future missions might rely on.
	if (!Fred_running && load_savefile && (Campaign.type == CAMPAIGN_TYPE_SINGLE)) {
		// savefile can fail to load for numerous otherwise non-fatal reasons
		// if it doesn't load in that case then it will be (re)created at save
		if ( !Pilot.load_savefile(pl, Campaign.filename) ) {
			// but if the data is invalid for the savefile then it is fatal
			if ( Pilot.is_invalid() ) {
				Campaign.filename[0] = 0;
				Campaign.num_missions = 0;
				Campaign_load_failure = CAMPAIGN_ERROR_SAVEFILE;
				return CAMPAIGN_ERROR_SAVEFILE;
			}
			// start with fresh new campaign data
			else {
				Pilot.clear_savefile(false);	// don't reset ships and weapons because they are currently the campaign's starting ones
				Campaign.next_mission = 0;
				Pilot.save_savefile();
			}
		}
	}

	// all is good here, move along
	Campaign_file_missing = false;

	return 0;
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

	Campaign_file_missing = false;

	player_loadout_init();
}

// the below two functions is internal to this module.  It is here so that I can group the save/load
// functions together.
//

/**
 * Deletes any save file in the players directory for the given
 * campaign filename
 */
void mission_campaign_savefile_delete(const char* cfilename)
{
	char filename[_MAX_FNAME];

	if ( Player->flags & PLAYER_FLAGS_IS_MULTI ) {
		return;	// no such thing as a multiplayer campaign savefile
	}

	auto base = util::get_file_part(cfilename);
	// do a sanity check, but don't arbitrarily drop any extension in case the filename contains a period
	Assertion(!stristr(base, FS_CAMPAIGN_FILE_EXT), "The campaign should not have an extension at this point!");

	// only support the new filename here - taylor
	sprintf_safe( filename, NOX("%s.%s.csg"), Player->callsign, base );

	cf_delete(filename, CF_TYPE_PLAYERS, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
}

/**
 * Deletes all the save files for this particular pilot.
 * Just call cfile function which will delete multiple files
 *
 * @param pilot_name Name of pilot
 */
void mission_campaign_delete_all_savefiles(char *pilot_name)
{
	int dir_type, num_files, i;
	char file_spec[MAX_FILENAME_LEN + 2];
	char filename[1024];
	int(*filter_save)(const char *filename);
	SCP_vector<SCP_string> names;

	auto ext = NOX(".csg");
	dir_type = CF_TYPE_PLAYERS;

	sprintf(file_spec, NOX("%s.*%s"), pilot_name, ext);

	// HACK HACK HACK HACK!!!!  cf_get_file_list is not reentrant.  Pretty dumb because it should
	// be.  I have to save any file filters
	filter_save = Get_file_list_filter;
	Get_file_list_filter = NULL;
	num_files            = cf_get_file_list(names, dir_type, file_spec, CF_SORT_NONE, nullptr,
                                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	Get_file_list_filter = filter_save;

	for (i = 0; i < num_files; i++) {
		strcpy_s(filename, names[i].c_str());
		strcat_s(filename, ext);
		cf_delete(filename, dir_type, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	}
}

/**
 * Sets up the internal veriables of the campaign structure so the player can play the next mission.
 * If there are no more missions available in the campaign, this function returns -1, else 0 if the mission was
 * set successfully
 */
int mission_campaign_next_mission()
{
	if ((Campaign.next_mission == -1) || (Campaign.name[0] == '\0')) // will be set to -1 when there is no next mission
		return -1;

	if (Campaign.num_missions < 1)
		return -2;

	Campaign.current_mission = Campaign.next_mission;
	strcpy_s(Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name);

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
	if (!(Game_mode & GM_CAMPAIGN_MODE))
		return 0;

	if (Campaign.prev_mission == -1)
		return 0;

	Campaign.current_mission = Campaign.prev_mission;
	Campaign.prev_mission = -1;
	Campaign.next_mission = Campaign.current_mission;
	Campaign.num_missions_completed--;
	Campaign.missions[Campaign.next_mission].completed = 0;

	// copy backed up variables over  
	for (auto& ra_variable : Campaign.red_alert_variables) {
		Campaign.persistent_variables.push_back(ra_variable);
	}

	// copy backed up containers over
	for (const auto& ra_container : Campaign.red_alert_containers) {
		Campaign.persistent_containers.emplace_back(ra_container);
	}
	
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
		int saved_next_mission = Campaign.next_mission;
		// Set temporarily to -1 so we know if loop formula fails to assign
		Campaign.next_mission = -1;
		if (Campaign.missions[cur].mission_loop_formula != -1) {
			flush_sexp_tree(Campaign.missions[cur].mission_loop_formula);  // force formula to be re-evaluated
			eval_sexp(Campaign.missions[cur].mission_loop_formula);  // this should set Campaign.next_mission to the loop mission
		}

		Campaign.loop_mission = Campaign.next_mission;
		Campaign.next_mission = saved_next_mission;

		// If the loop mission and the next mission are the same, then don't do the loop.  This could be the case if the campaign
		// only allows us to proceed to the loop mission if certain conditions are met.
		if (Campaign.loop_mission == Campaign.next_mission) {
			Campaign.loop_mission = -1;
		}
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
void mission_campaign_store_goals_and_events()
{
	int cur;
	cmission *mission_obj;

	if (!(Game_mode & GM_CAMPAIGN_MODE) || (Campaign.current_mission < 0))
		return;

	cur = Campaign.current_mission;

	mission_obj = &Campaign.missions[cur];

	// first we must save the status of the current missions goals in the campaign mission structure.
	// After that, we can determine which mission is tagged as the next mission.  Finally, we
	// can save the campaign save file
	// we might have goal and event status if the player replayed a mission
	mission_obj->goals.clear();

	// copy the needed info from the Mission_goal struct to our internal structure
	for (const auto& goal: Mission_goals) {
		mission_obj->goals.emplace_back();
		auto& stored_goal = mission_obj->goals.back();

		if (goal.name.empty())
			sprintf(stored_goal.name, NOX("Goal #" SIZE_T_ARG), &goal - &Mission_goals[0] + 1);
		else
			strncpy_s(stored_goal.name, goal.name.c_str(), NAME_LENGTH - 1);

		Assert ( goal.satisfied != GOAL_INCOMPLETE );		// should be true or false at this point!!!
		stored_goal.status = (char)goal.satisfied;
	}

	// do the same thing for events as we did for goals
	// we might have goal and event status if the player replayed a mission
	mission_obj->events.clear();

	// copy the needed info from the Mission_goal struct to our internal structure
	for (const auto& event: Mission_events) {
		mission_obj->events.emplace_back();
		auto& stored_event = mission_obj->events.back();

		if (event.name.empty()) {
			sprintf(stored_event.name, NOX("Event #" SIZE_T_ARG), &event - &Mission_events[0] + 1);
			nprintf(("Warning", "Mission event in mission %s must have a +Name field! using %s for campaign save file\n", mission_obj->name, stored_event.name));
		} else
			strncpy_s(stored_event.name, event.name.c_str(), NAME_LENGTH - 1);

		// Old method:
		// getting status for the events is a little different.  If the formula value for the event entry
		// is -1, then we know the value of the result field will never change.  If the formula is
		// not -1 (i.e. still being evaluated at mission end time), we will write "incomplete" for the
		// event evaluation
		// New method: check a flag.  Also, even with the old method, events are always
		// forced satisfied or failed at the end of a mission
		if (event.flags & MEF_EVENT_IS_DONE) {
			if ( event.result )
				stored_event.status = static_cast<int>(EventStatus::SATISFIED);
			else
				stored_event.status = static_cast<int>(EventStatus::FAILED);
		} else
			UNREACHABLE("Mission event formula should be marked MEF_EVENT_IS_DONE at end-of-mission");
	}
}

void mission_campaign_store_variables(int persistence_type, bool store_red_alert)
{
	int cur, i, j;
	cmission *mission_obj;

	if (!(Game_mode & GM_CAMPAIGN_MODE) || (Campaign.current_mission < 0))
		return;

	cur = Campaign.current_mission;
	mission_obj = &Campaign.missions[cur];

	// handle variables that are saved on mission victory -------------------------------------
	mission_obj->variables.clear();

	int num_mission_variables = sexp_campaign_file_variable_count();

	if (num_mission_variables > 0) {
		
		if (store_red_alert) {
			for (auto& current_rav : Campaign.red_alert_variables) {
				Campaign.persistent_variables.push_back(current_rav);
			}
		}

		for (i = 0; i < sexp_variable_count(); i++) {
			if (!(Sexp_variables[i].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
				if (Sexp_variables[i].type & persistence_type) {
					bool add_it = true;

					// see if we already have a variable with this name
					for (j = 0; j < (int)Campaign.persistent_variables.size(); j++) {
						if (!(stricmp(Sexp_variables[i].variable_name, Campaign.persistent_variables[j].variable_name))) {
							add_it = false;
							Campaign.persistent_variables[j].type = Sexp_variables[i].type;
							strcpy_s(Campaign.persistent_variables[j].text, Sexp_variables[i].text);
							break;
						}
					}

					// new variable
					if (add_it) {
						Campaign.persistent_variables.push_back(Sexp_variables[i]);
					}
				}
			}
			// we might need to save some eternal variables
			else if ((persistence_type & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS) && (Sexp_variables[i].type & persistence_type) && (Sexp_variables[i].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
				bool add_it = true;

				for (j = 0; j < (int)Player->variables.size(); j++) {
					if (!(stricmp(Sexp_variables[i].variable_name, Player->variables[j].variable_name))) {
						Player->variables[j] = Sexp_variables[i];

						add_it = false;
						break;
					}
				}

				// if not found then add new entry
				if (add_it) {
					Player->variables.push_back(Sexp_variables[i]);
				}
			}
		}
	}
}

// jg18 - adapted from mission_campaign_store_variables()
void mission_campaign_store_containers(ContainerType persistence_type, bool store_red_alert)
{
	if (!(Game_mode & GM_CAMPAIGN_MODE) || (Campaign.current_mission < 0))
		return;

	if (!sexp_container_has_persistent_non_eternal_containers()) {
		// nothing to do
		return;
	}

	if (store_red_alert) {
		for (const auto& current_con : Campaign.red_alert_containers) {
			Campaign.persistent_containers.emplace_back(current_con);
		}
	}

	for (const auto &container : get_all_sexp_containers()) {
		if (!container.is_eternal()) {
			if (any(container.type & persistence_type)) {
				// see if we already have a container with this name
				auto cpc_it = std::find_if(Campaign.persistent_containers.begin(),
					Campaign.persistent_containers.end(),
					[container](const sexp_container &cpc) {
						return cpc.name_matches(container);
					});

				if (cpc_it != Campaign.persistent_containers.end()) {
					*cpc_it = container;
				} else {
					// new container
					Campaign.persistent_containers.emplace_back(container);
				}
			}
		} else if (any(persistence_type & ContainerType::SAVE_ON_MISSION_PROGRESS) &&
				   any(container.type & persistence_type) && container.is_eternal()) {
			// we might need to save some eternal player-persistent containers
			auto ppc_it = std::find_if(Player->containers.begin(),
				Player->containers.end(),
				[container](const sexp_container &ppc) {
					return ppc.name_matches(container);
				});

			if (ppc_it != Player->containers.end()) {
				*ppc_it = container;
			} else {
				// new player-persistent container
				Player->containers.emplace_back(container);
			}
		}
	}
}

void mission_campaign_store_goals_and_events_and_variables()
{
	mission_campaign_store_goals_and_events();
	mission_campaign_store_variables(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS);
	mission_campaign_store_containers(ContainerType::SAVE_ON_MISSION_PROGRESS);
}

/**
 * Called when the player's mission is over.  It updates the internal store of goals
 * and their status then saves the state of the campaign in the campaign file.  
 * This gets called after player accepts mission results in debriefing.
 */
void mission_campaign_mission_over(bool do_next_mission)
{
	int mission_num, i;
	cmission *mission_obj;

	// I don't think that we should have a record for these -- maybe we might??????  If we do,
	// then we should free them
	if ( !(Game_mode & GM_CAMPAIGN_MODE) ){
		return;
	}

	mission_num = Campaign.current_mission;
	Assert( mission_num != -1 );
	mission_obj = &Campaign.missions[mission_num];

	// determine if any ships/weapons were granted this mission
	for ( i=0; i<Num_granted_ships; i++ ){
		Campaign.ships_allowed[Granted_ships[i]] = 1;
	}

	for ( i=0; i<Num_granted_weapons; i++ ){
		Campaign.weapons_allowed[Granted_weapons[i]] = 1;	
	}

	// Goober5000 - player-persistent variables are handled when the mission is
	// over, not necessarily when the mission is accepted

	// update campaign.mission stats (used to allow backout inRedAlert)
	// .. but we don't do this if we are inside of the prev/current loop hack
	if ( Campaign.prev_mission != Campaign.current_mission ) {
		mission_obj->stats.assign( Player->stats );
		if(!(Game_mode & GM_MULTIPLAYER)){
			scoring_backout_accept( &mission_obj->stats );
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

		// runs the new scripting conditional hook, "On Campaign Mission Accept" --wookieejedi
		scripting::hooks::OnCampaignMissionAccept->run(
			scripting::hook_param_list(scripting::hook_param("Mission", 's', mission_obj->name)
		));
		
	} else {
		// free up the goals and events which were just malloced.  It's kind of like erasing any fact
		// that the player played this mission in the campaign.
		mission_obj->goals.clear();
		mission_obj->events.clear();
		mission_obj->variables.clear();

		Sexp_nodes[mission_obj->formula].value = SEXP_UNKNOWN;
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

		Campaign.missions[i].goals.clear();
		Campaign.missions[i].events.clear();
		Campaign.missions[i].variables.clear();

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
		Campaign.missions[i].mission_loop_formula = 0;
		Campaign.missions[i].level = 0;
		Campaign.missions[i].pos = 0;
		Campaign.missions[i].flags = 0;
		Campaign.missions[i].main_hall = "";
		Campaign.missions[i].substitute_main_hall = "";
		Campaign.missions[i].debrief_persona_index = 0;

		Campaign.missions[i].stats.init();
	}

	memset(Campaign.name, 0, NAME_LENGTH);
	memset(Campaign.filename, 0, MAX_FILENAME_LEN);
	Campaign.type = 0;
	Campaign.flags = CF_DEFAULT_VALUE;
	Campaign.num_missions = 0;
	Campaign.num_missions_completed = 0;
	Campaign.current_mission = -1;
	Campaign.next_mission = 0;
	Campaign.prev_mission = -1;
	Campaign.loop_enabled = 0;
	Campaign.loop_mission = CAMPAIGN_LOOP_MISSION_UNINITIALIZED;
	Campaign.loop_reentry = 0;
	Campaign.realign_required = 0;
	Campaign.num_players = 0;
	memset( Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed) );
	memset( Campaign.weapons_allowed, 0, sizeof(Campaign.weapons_allowed) );
	Campaign.persistent_variables.clear(); 
	Campaign.red_alert_variables.clear();
	Campaign.persistent_containers.clear();
	Campaign.red_alert_containers.clear();
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
	// read the mission file and get the list of mission filenames
	try
	{
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
	catch (const parse::ParseException& e)
	{
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return 1;
	}

	return 0;
}

SCP_string mission_campaign_get_name(const char* filename)
{
	// read the mission file and only read the name entry
	SCP_string filename_str = filename;
	filename_str += FS_CAMPAIGN_FILE_EXT;
	try {
		Assertion(filename_str.size() < MAX_FILENAME_LEN,
		          "Filename (%s) is too long. Is " SIZE_T_ARG " bytes long but maximum is %d.", filename_str.c_str(),
		          filename_str.size(), MAX_FILENAME_LEN); // make sure no overflow
		read_file_text(filename_str.c_str());
		reset_parse();

		required_string("$Name:");

		SCP_string res;
		stuff_string(res, F_NAME);

		return res;
	} catch (const parse::ParseException& e) {
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", filename_str.c_str(), e.what()));
		return SCP_string();
	}
}

/**
 * Read the goals and events from a mission in a campaign file and store that information
 * in the campaign structure for use in the campaign editor, error checking, etc
 */
void read_mission_goal_list(int num)
{
	char *filename, notes[NOTES_LENGTH];
	int z;

	Assertion(num >= 0 && num < Campaign.num_missions, "mission number out of range!");
	filename = Campaign.missions[num].name;

	if (Campaign.missions[num].notes) {
		vm_free(Campaign.missions[num].notes);
		Campaign.missions[num].notes = nullptr;
	}
	Campaign.missions[num].events.clear();
	Campaign.missions[num].goals.clear();
	Campaign.missions[num].variables.clear();

	try
	{
		read_file_text(filename);
		reset_parse();
		init_sexp();

		// first, read the mission notes for this mission.  Used in campaign editor
		if (skip_to_string("#Mission Info")) {
			if (skip_to_string("$Notes:")) {
				stuff_string(notes, F_NOTES, NOTES_LENGTH);
				Campaign.missions[num].notes = (char *)vm_malloc(strlen(notes) + 1);
				strcpy(Campaign.missions[num].notes, notes);
			}
		}

		// skip to events section in the mission file.  Events come before goals, so we process them first
		if (skip_to_string("#Events")) {
			while (true) {
				if (skip_to_string("$Formula:", "#Goals") != 1){
					break;
				}

				z = skip_to_string("+Name:", "$Formula:");
				if (!z){
					break;
				}

				Campaign.missions[num].events.emplace_back();
				auto& stored_event = Campaign.missions[num].events.back();

				if (z == 1)
					stuff_string(stored_event.name, F_NAME, NAME_LENGTH);
				else
					sprintf(stored_event.name, NOX("Event #" SIZE_T_ARG), &stored_event - &Campaign.missions[num].events[0] + 1);
			}
		}

		if (skip_to_string("#Goals")) {
			while (true) {
				if (skip_to_string("$Type:", "#End") != 1){
					break;
				}

				z = skip_to_string("+Name:", "$Type:");
				if (!z){
					break;
				}

				Campaign.missions[num].goals.emplace_back();
				auto& stored_goal = Campaign.missions[num].goals.back();

				if (z == 1)
					stuff_string(stored_goal.name, F_NAME, NAME_LENGTH);
				else
					sprintf(stored_goal.name, NOX("Goal #" SIZE_T_ARG), &stored_goal - &Campaign.missions[num].goals[0] + 1);
			}
		}

		// Goober5000 - variables do not need to be read here
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
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
int mission_campaign_find_mission( const char *name )
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
	int mission_idx;
	char *filename;

	// only support pre mission movies for now.
	Assert ( type == CAMPAIGN_MOVIE_PRE_MISSION );

	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	mission_idx = Campaign.current_mission;
	Assert( mission_idx != -1 );

	// get a possible filename for a movie to play.
	filename = NULL;
	switch( type ) {
	case CAMPAIGN_MOVIE_PRE_MISSION:
		if ( strlen(Campaign.missions[mission_idx].briefing_cutscene) )
			filename = Campaign.missions[mission_idx].briefing_cutscene;
		break;

	default:
		Int3();
		break;
	}

	// no filename, no movie!
	if ( !filename )
		return;

	movie::play(filename);	//Play the movie!
}

/**
 * Return the type of campaign of the passed filename
 */
int mission_campaign_parse_is_multi(const char *filename, char *name)
{	
	int i;
	char temp[NAME_LENGTH];
	
	try
	{
		read_file_text(filename);
		reset_parse();

		required_string("$Name:");
		stuff_string(temp, F_NAME, NAME_LENGTH);
		if (name)
			strcpy(name, temp);

		required_string("$Type:");
		stuff_string(temp, F_NAME, NAME_LENGTH);

		for (i = 0; i < MAX_CAMPAIGN_TYPES; i++) {
			if (!stricmp(temp, campaign_types[i])) {
				return i;
			}
		}

		Error(LOCATION, "Unknown campaign type %s", temp);
		return -1;
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("MISSIONCAMPAIGN: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return -1;
	}
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
	SCP_tolower(filename_no_ext);

	for (auto &ii: Ignored_campaigns) {
		if (ii == filename_no_ext) {
			return true;
		}
	}
	
	return false;
}

static bool campaign_reportable_error(int val)
{
	return val != 0 && val != CAMPAIGN_ERROR_MISSING && val != CAMPAIGN_ERROR_IGNORED;
}

// returns 0: loaded, !0: error
int mission_load_up_campaign(bool fall_back_from_current)
{
	int rc = -1, idx;
	auto pl = Player;

	// find best campaign to use:
	//   1) cmdline
	//   2) last used
	//   3) builtin
	//   4) anything else
	// Note that in each step we only fall back when the error is benign, e.g. ignored or missing;
	// if there's some other real error with the campaign file, we report it.
	// Also note that if we *have* a current campaign, we shouldn't fall back *at all*, lest we repeatedly
	// reset what the current campaign is, *unless* we are starting a brand new session or loading a new pilot.

	// cmdline...
	if ( Cmdline_campaign != nullptr && strlen(Cmdline_campaign) ) {
		char* campaign = Cmdline_campaign;

		// Clear cmdline value
		// * Only set campaign once from cmdline.
		// * Prevent subsequent load failures.
		// * On success, campaign becomes "last used".
		Cmdline_campaign = nullptr;

		bool has_last_used_campaign = strlen(pl->current_campaign) > 0;
		bool campaign_already_set = has_last_used_campaign
			&& (stricmp(campaign, pl->current_campaign) == 0);

		if (has_last_used_campaign) {
			mprintf(("Current campaign is '%s'\n", pl->current_campaign));
		}

		if (!campaign_already_set) {
			rc = mission_campaign_load(campaign, nullptr, pl);

			if (rc == 0) {
				// update pilot with the new current campaign (becomes "last used")
				strcpy_s(pl->current_campaign, Campaign.filename);
				mprintf(("Set current campaign to '%s'\n", campaign));
				return rc;
			} else {
				mprintf(("Failed to set current campaign to '%s'\n", campaign));
			}
		}
	}

	// last used...
	if ( strlen(pl->current_campaign) ) {
		rc = mission_campaign_load(pl->current_campaign, nullptr, pl);
		if (rc == 0 || !fall_back_from_current || campaign_reportable_error(rc)) {
			return rc;
		}
	}

	// builtin...
	rc = mission_campaign_load(Default_campaign_file_name, nullptr, pl);

	// everything else...
	if (rc < 0 && !campaign_reportable_error(rc)) {
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
			rc = mission_campaign_load(Campaign_file_names[idx], nullptr, pl);
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
		if (supernova_stage() >= SUPERNOVA_STAGE::HIT) {
			movie::play_two("endpart1.mve", "endprt2b.mve");			// bad ending
		} else {
			movie::play_two("endpart1.mve", "endprt2a.mve");			// good ending
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
 * this also posts the state change
 */
void mission_campaign_skip_to_next()
{
	// mark mission as skipped
	Campaign.missions[Campaign.current_mission].flags |= CMISSION_FLAG_SKIPPED;

	// mark all goals/events complete
	// these do not really matter, since is-previous-event-* and is-previous-goal-* sexps check
	// to see if the mission was skipped, and use defaults accordingly.
	mission_goal_mark_objectives_complete();
	mission_goal_mark_events_complete();

	// store
	mission_campaign_store_goals_and_events_and_variables();

	// now set the next mission
	mission_campaign_eval_next_mission();

	// because all goals/events are marked true, it's possible for the campaign condition to unexpectedly *not* evaluate to the next mission
	// (e.g. if is-previous-goal-false or is-previous-event-false is used without the optional argument), so this is a failsafe
	if (Campaign.next_mission == Campaign.current_mission) {
		Campaign.next_mission++;
		if (Campaign.next_mission < Campaign.num_missions) {
			Warning(LOCATION, "mission_campaign_skip_to_next() could not determine the next mission!  Choosing the next-in-sequence mission as a failsafe...");
		} else {
			Warning(LOCATION, "mission_campaign_skip_to_next() could not determine the next mission!");
			Campaign.next_mission = -1;
		}
	}

	// clear out relevant player vars
	Player->failures_this_session = 0;
	Player->show_skip_popup = 1;

	// proceed to next mission or main hall
	if ((Campaign.missions[Campaign.current_mission].flags & CMISSION_FLAG_HAS_LOOP) && (Campaign.loop_mission != -1)) {
		// go to loop solicitation
		gameseq_post_event(GS_EVENT_LOOP_BRIEF);
	} else {
		// closes out mission stuff, sets up next one
		mission_campaign_mission_over();

		if ( Campaign.next_mission == -1 || (The_mission.flags[Mission::Mission_Flags::End_to_mainhall]) ) {
			// go to main hall, either the campaign is over or the FREDer requested it.
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		} else {
			// go to next mission
			gameseq_post_event(GS_EVENT_START_GAME);
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

	if ( Campaign.next_mission == -1 || (The_mission.flags[Mission::Mission_Flags::End_to_mainhall]) ) {
		gameseq_post_event( GS_EVENT_MAIN_MENU );
	} else {
		gameseq_post_event( GS_EVENT_START_GAME );
	}
}


/**
 * Used for jumping to a particular campaign mission
 * all previous missions marked skipped
 * this relies on correct mission ordering in the campaign file
 */
bool mission_campaign_jump_to_mission(const char* filename, bool no_skip)
{
	int i = 0, mission_num = -1;
	constexpr size_t dest_filename_size = 64;
	char dest_filename[dest_filename_size], *p;

	// load in the campaign junk
	mission_load_up_campaign();

	// tack the .fs2 onto the input name if necessary
	strcpy_s(dest_filename, filename);
	drop_white_space(dest_filename);
	p = strchr(dest_filename, '.');
	if ((p == nullptr || stricmp(p, ".fs2") != 0) && strlen(dest_filename) <= dest_filename_size - 4 - 1)
		strcat_s(dest_filename, ".fs2");

	// search for our mission
	for (i = 0; i < Campaign.num_missions; i++) {
		if ((Campaign.missions[i].name != nullptr) && !stricmp(Campaign.missions[i].name, dest_filename)) {
			mission_num = i;
			break;
		} else if (!no_skip) {
			Campaign.missions[i].flags |= CMISSION_FLAG_SKIPPED;
			Campaign.missions[i].completed = 1;
			Campaign.num_missions_completed = i + 1;
		}
	}

	if (mission_num < 0) {
		// if we got here, no match was found
		// based on player feedback, let's NOT restart the campaign but rather fail gracefully
		return false;
	} else {
		for (SCP_vector<ship_info>::iterator it = Ship_info.begin(); it != Ship_info.end(); it++) {
			i = static_cast<int>(std::distance(Ship_info.begin(), it));
			Campaign.ships_allowed[i] = 1;
		}
		for (i = 0; i < weapon_info_size(); i++) {
			Campaign.weapons_allowed[i] = 1;
		}

		Campaign.next_mission = mission_num;
		Campaign.prev_mission = mission_num - 1;
		mission_campaign_next_mission();
		Game_mode |= GM_CAMPAIGN_MODE;

		gameseq_post_event(GS_EVENT_START_GAME);
		return true;
	}
}

// Goober5000
void mission_campaign_save_on_close_variables()
{
	int i;

	// make sure we are actually playing a single-player campaign
	if (!(Game_mode & GM_CAMPAIGN_MODE) || (Campaign.type != CAMPAIGN_TYPE_SINGLE) || (Campaign.current_mission < 0))
		return;

	// now save variables
	for (i = 0; i < sexp_variable_count(); i++) {
		// we only want the on mission close type. On campaign progress type are dealt with elsewhere
		if ( !(Sexp_variables[i].type & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE) ) {
			continue;
		}

		bool found = false;

		// deal with eternals 
		if ((Sexp_variables[i].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
			// check if variable already exists and updated it
			for (auto& current_variable : Player->variables) {
				if (!(stricmp(Sexp_variables[i].variable_name, current_variable.variable_name))) {
					current_variable = Sexp_variables[i];

					found = true;
					break;
				}
			}

			// if not found then add new entry
			if (!found) {
				Player->variables.push_back(Sexp_variables[i]);
			}
		}

	}

	// store any non-eternal on mission close variables
	mission_campaign_store_variables(SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE, false);
}

// jg18 - adapted from mission_campaign_save_on_close_variables()
void mission_campaign_save_on_close_containers()
{
	// make sure we are actually playing a single-player campaign
	if (!(Game_mode & GM_CAMPAIGN_MODE) || (Campaign.type != CAMPAIGN_TYPE_SINGLE) || (Campaign.current_mission < 0))
		return;

	// now save containers
	for (const auto &container : get_all_sexp_containers()) {
		// we only want the on mission close type. On campaign progress type are dealt with elsewhere
		if (none(container.type & ContainerType::SAVE_ON_MISSION_CLOSE)) {
			continue;
		}

		// deal with eternals
		if (container.is_eternal()) {
			// check if container already exists and update it
			auto ppc_it = std::find_if(Player->containers.begin(),
				Player->containers.end(),
				[container](const sexp_container &ppc) {
					return ppc.name_matches(container);
				});

			if (ppc_it != Player->containers.end()) {
				*ppc_it = container;
			} else {
				// if not found then add new entry
				Player->containers.emplace_back(container);
			}
		}
	}

	// store any non-eternal on mission close containers
	mission_campaign_store_containers(ContainerType::SAVE_ON_MISSION_CLOSE, false);
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
