/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionCampaign.cpp $
 * $Revision: 2.4 $
 * $Date: 2002-08-18 19:48:29 $
 * $Author: randomtiger $
 *
 * source for dealing with campaigns
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 20:12:31  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/17 06:45:53  mharris
 * More porting tweaks.  It links!  but segfaults...
 *
 * Revision 1.3  2002/05/17 03:06:13  mharris
 * more porting tweaks
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 23    9/14/99 4:35a Dave
 * Argh. Added all kinds of code to handle potential crashes in debriefing
 * code.
 * 
 * 22    9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 21    9/09/99 9:34a Jefff
 * fixed a potential exit-loop bug
 * 
 * 20    9/07/99 6:55p Jefff
 * functionality to break out of a loop.  hacked functionality to jump to
 * a specific mission in a campaign -- doesnt grant ships/weapons from
 * skipped missions tho.
 * 
 * 19    9/07/99 2:19p Jefff
 * clear skip mission player vars on mission skip
 * 
 * 18    9/06/99 9:45p Jefff
 * break out of loop and skip mission support
 * 
 * 17    9/06/99 6:38p Dave
 * Improved CD detection code.
 * 
 * 16    9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 15    8/27/99 12:04a Dave
 * Campaign loop screen.
 * 
 * 14    8/04/99 5:36p Andsager
 * Show upsell screens at end of demo campaign before returning to main
 * hall.
 * 
 * 13    2/05/99 3:50p Anoop
 * Removed dumb campaign mission stats saving from multiplayer.
 * 
 * 12    12/17/98 2:43p Andsager
 * Modify fred campaign save file to include optional mission loops
 * 
 * 11    12/12/98 3:17p Andsager
 * Clean up mission eval, goal, event and mission scoring.
 * 
 * 10    12/10/98 9:59a Andsager
 * Fix some bugs with mission loops
 * 
 * 9     12/09/98 1:56p Andsager
 * Initial checkin of mission loop
 * 
 * 8     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 7     11/03/98 4:48p Johnson
 * Fixed campaign file versioning bug left over from Silent Threat port.
 * 
 * 6     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 5     10/13/98 2:47p Andsager
 * Remove reference to Tech_shivan_species_avail
 * 
 * 4     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 95    9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and Freespace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 94    9/01/98 4:25p Dave
 * Put in total (I think) backwards compatibility between mission disk
 * freespace and non mission disk freespace, including pilot files and
 * campaign savefiles.
 * 
 * 93    7/06/98 4:10p Hoffoss
 * Fixed some bugs that presented themselves when trying to use a pilot
 * that has a no longer existent campaign active.  Also expanded the
 * campaign load code to actually return a proper error code, instead of
 * always trapping errors internally and crashing, and always returning 0.
 * 
 * 92    6/17/98 9:30a Allender
 * fixed red alert replay stats clearing problem
 * 
 * 91    6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 90    5/25/98 1:29p Allender
 * end mission sequencing
 * 
 * 89    5/21/98 9:25p Allender
 * endgame movie always viewable at end of campaign
 * 
 * 88    5/13/98 5:14p Allender
 * red alert support to go back to previous mission
 * 
 * 87    5/12/98 4:16p Hoffoss
 * Fixed bug where not all missions in all campaigns were being filtered
 * out of stand alone mission listing in simulator room.
 * 
 * 86    5/05/98 3:29p Hoffoss
 * Changed code so description is BEFORE num players in campaign file.
 * Other code is relying on this ordering.
 * 
 * 85    5/05/98 12:19p Dave
 * campaign description goes *after* num players
 * 
 * 84    5/04/98 5:52p Comet
 * Fixed bug with Galatea/Bastion selection when finishing missions.
 * 
 * 83    5/01/98 2:46p Duncan
 * fix a cfile problem with campaigns related to the new cfile stuff
 * 
 * 82    5/01/98 12:34p John
 * Added code to force FreeSpace to run in the same dir as exe and made
 * all the parse error messages a little nicer.
 * 
 * 81    4/30/98 7:01p Duncan
 * AL: don't allow deletion of campaign files in multiplayer
 * 
 * 80    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 *
 * $NoKeywords: $
 */

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

#if defined unix
#include <sys/stat.h>
#include <glob.h>
#endif

#include "io/key.h"
#include "ui/ui.h"
#include "mission/missioncampaign.h"
#include "gamesequence/gamesequence.h"
#include "graphics/2d.h"
#include "parse/parselo.h"
#include "mission/missionload.h"
#include "freespace2/freespace.h"
#include "parse/sexp.h"
#include "cfile/cfile.h"
#include "playerman/player.h"
#include "mission/missiongoals.h"
#include "cutscene/movie.h"
#include "network/multi.h"
#include "menuui/techmenu.h"
#include "gamesnd/eventmusic.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "starfield/supernova.h"

// mission disk stuff
#define CAMPAIGN_SAVEFILE_MAX_SHIPS_OLD						75
#define CAMPAIGN_SAVEFILE_MAX_WEAPONS_OLD						44

#define CAMPAIGN_INITIAL_RELEASE_FILE_VERSION				6

// campaign wasn't ended
int Campaign_ended_in_mission = 0;

// stuff for selecting campaigns.  We need to keep both arrays around since we display the
// list of campaigns by name, but must load campaigns by filename
char *Campaign_names[MAX_CAMPAIGNS];
char *Campaign_file_names[MAX_CAMPAIGNS];
int	Num_campaigns;

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
int Granted_ships[MAX_SHIP_TYPES];
int Granted_weapons[MAX_WEAPON_TYPES];

// variables to control the UI stuff for loading campaigns
LOCAL int Campaign_ui_active = 0;
LOCAL UI_WINDOW Campaign_window;
LOCAL UI_LISTBOX Campaign_listbox;
LOCAL UI_BUTTON Campaign_okb, Campaign_cancelb;

// the campaign!!!!!
campaign Campaign;

// variables with deal with the campaign save file
#define CAMPAIGN_FILE_VERSION							12
//#define CAMPAIGN_FILE_COMPATIBLE_VERSION		CAMPAIGN_INITIAL_RELEASE_FILE_VERSION
#define CAMPAIGN_FILE_COMPATIBLE_VERSION			CAMPAIGN_FILE_VERSION
#define CAMPAIGN_FILE_ID								0xbeefcafe

// variables with deal with the campaign stats save file
#define CAMPAIGN_STATS_FILE_VERSION					1
#define CAMPAIGN_STATS_FILE_COMPATIBLE_VERSION	1
#define CAMPAIGN_STATS_FILE_ID						0xabbadaad

// mission_campaign_get_name returns a string (which is malloced in this routine) of the name
// of the given freespace campaign file.  In the type field, we return if the campaign is a single
// player or multiplayer campaign.  The type field will only be valid if the name returned is non-NULL
int mission_campaign_get_info(char *filename, char *name, int *type, int *max_players, char **desc)
{
	int rval, i;
	char campaign_type[NAME_LENGTH], fname[MAX_FILENAME_LEN];

	Assert( name != NULL );
	Assert( type != NULL );

	// open localization
	lcl_ext_open();

	strcpy(fname, filename);
	if ((strlen(fname) < 4) || stricmp(fname + strlen(fname) - 4, FS_CAMPAIGN_FILE_EXT)){
		strcat(fname, FS_CAMPAIGN_FILE_EXT);
	}

	Assert(strlen(fname) < MAX_FILENAME_LEN);

	if ((rval = setjmp(parse_abort)) != 0) {
		if (rval == 5){
			// close localization
			lcl_ext_close();

			return 0;
		}

		Error(LOCATION, "Error parsing '%s'\r\nError code = %i.\r\n", fname, rval);

	} else {
		read_file_text( fname );
		reset_parse();
		required_string("$Name:");

		stuff_string( name, F_NAME, NULL );
		if ( name == NULL ) {
			Int3();
			nprintf(("Warning", "No name found for campaign file %s\n", filename));

			// close localization
			lcl_ext_close();

			return 0;
		}

		required_string( "$Type:" );
		stuff_string( campaign_type, F_NAME, NULL );

		*type = -1;
		for (i=0; i<MAX_CAMPAIGN_TYPES; i++) {
			if ( !stricmp(campaign_type, campaign_types[i]) ) {
				*type = i;
			}
		}

		if (desc) {
			*desc = NULL;
			if (optional_string("+Description:")) {
				*desc = stuff_and_malloc_string(F_MULTITEXT, NULL, MISSION_DESC_LENGTH);
			}
		}

		// if this is a multiplayer campaign, get the max players
		if ((*type) > 0) {
			skip_to_string("+Num Players:");
			stuff_int(max_players);
		}		

		// if we found a valid campaign type
		if ((*type) >= 0) {
			// close localization
			lcl_ext_close();

			return 1;
		}
	}

	Int3();		// get Allender -- incorrect type found

	// close localization
	lcl_ext_close();

	return 0;
}

// parses campaign and returns a list of missions in it.  Returns number of missions added to
// the 'list', and up to 'max' missions may be added to 'list'.  Returns negative on error.
//
int mission_campaign_get_mission_list(char *filename, char **list, int max)
{
	int rval, i, num = 0;
	char name[NAME_LENGTH];

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	// read the mission file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		// since we can't return count of allocated elements, free them instead
		for (i=0; i<num; i++)
			free(list[i]);

		return -1;

	} else {
		read_file_text(filename);
		reset_parse();

		while (skip_to_string("$Mission:") > 0) {
			stuff_string(name, F_NAME, NULL);
			if (num < max)
				list[num++] = strdup(name);
		}
	}

	return num;
}

void mission_campaign_maybe_add( char *filename, int multiplayer )
{
	char name[NAME_LENGTH];
	int type,max_players;

	if ( mission_campaign_get_info( filename, name, &type, &max_players) ) {
		if ( !multiplayer && ( type == CAMPAIGN_TYPE_SINGLE) ) {
			Campaign_names[Num_campaigns] = strdup(name);
			Campaign_file_names[Num_campaigns] = strdup(filename);
			Num_campaigns++;
		}
	}
}

// mission_campaign_build_list() builds up the list of campaigns that the user might
// be able to pick from.  It uses the multiplayer flag to tell if we should display a list
// of single or multiplayer campaigns.  This routine sets the Num_campaigns and Campaign_names
// global variables
void mission_campaign_build_list( int multiplayer )
{
	Num_campaigns = 0;
	mission_campaign_maybe_add( BUILTIN_CAMPAIGN, multiplayer);

	char wild_card[256];
	memset(wild_card, 0, 256);
	strcpy(wild_card, NOX("data" DIR_SEPARATOR_STR "missions" DIR_SEPARATOR_STR "*"));
	strcat(wild_card, FS_CAMPAIGN_FILE_EXT);

#if defined _WIN32
	int find_handle;
	_finddata_t find;
	find_handle = _findfirst( wild_card, &find );
	if( find_handle != -1 )	{
		if ( !(find.attrib & _A_SUBDIR) && stricmp(find.name, BUILTIN_CAMPAIGN) ){
			mission_campaign_maybe_add( find.name, multiplayer);
		}

		while( !_findnext( find_handle, &find ) )	{
			if ( !(find.attrib & _A_SUBDIR) && stricmp(find.name, BUILTIN_CAMPAIGN) )	{
				if ( Num_campaigns >= MAX_CAMPAIGNS ){
					//MessageBox( -2,-2, 1, "Only the first 300 files will be displayed.", "Ok" );
					break;
				} else {
					mission_campaign_maybe_add( find.name, multiplayer);
				}
			}
		}
	}
#elif defined unix
	glob_t globinfo;
	memset(&globinfo, 0, sizeof(globinfo));
	int status = glob(wild_card, 0, NULL, &globinfo);
	if (status == 0) {
		for (unsigned int i = 0;  i < globinfo.gl_pathc;  i++) {
			// Determine if this is a regular file
			struct stat statbuf;
			memset(&statbuf, 0, sizeof(statbuf));
			stat(globinfo.gl_pathv[i], &statbuf);
			if (S_ISREG(statbuf.st_mode) &&
				 (strcasecmp(globinfo.gl_pathv[i], BUILTIN_CAMPAIGN) != 0))
			{
				if ( Num_campaigns >= MAX_CAMPAIGNS ){
					break;
				} else {
					mission_campaign_maybe_add(globinfo.gl_pathv[i], multiplayer);
				}
			}
		}
		globfree(&globinfo);
	}
#endif
}


// gets optional ship/weapon information
void mission_campaign_get_sw_info()
{
	int i, count, ship_list[MAX_SHIP_TYPES], weapon_list[MAX_WEAPON_TYPES];

	// set allowable ships to the SIF_PLAYER_SHIPs
	memset( Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed) );
	for (i = 0; i < MAX_SHIP_TYPES; i++ ) {
		if ( Ship_info[i].flags & SIF_PLAYER_SHIP )
			Campaign.ships_allowed[i] = 1;
	}

	for (i = 0; i < MAX_WEAPON_TYPES; i++ )
		Campaign.weapons_allowed[i] = 1;

	if ( optional_string("+Starting Ships:") ) {
		for (i = 0; i < MAX_SHIP_TYPES; i++ )
			Campaign.ships_allowed[i] = 0;

		count = stuff_int_list(ship_list, MAX_SHIP_TYPES, SHIP_INFO_TYPE);

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

// mission_campaign_load starts a new campaign.  It reads in the mission information in the campaign file
// It also sets up all variables needed inside of the game to deal with starting mission numbers, etc
//
// Note: Due to difficulties in generalizing this function, parts of it are duplicated throughout
// this file.  If you change the format of the campaign file, you should be sure these related
// functions work properly and update them if it breaks them.
int mission_campaign_load( char *filename, int load_savefile )
{
	int len, rval, i;
	char name[NAME_LENGTH], type[NAME_LENGTH];

	filename = cf_add_ext(filename, FS_CAMPAIGN_FILE_EXT);

	// open localization
	lcl_ext_open();	

	// read the mission file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("Error parsing '%s'\r\nError code = %i.\r\n", filename, rval));

		// close localization
		lcl_ext_close();

		return CAMPAIGN_ERROR_CORRUPT;

	} else {
		// be sure to remove all old malloced strings of Mission_names
		// we must also free any goal stuff that was from a previous campaign
		// this also frees sexpressions so the next call to init_sexp will be able to reclaim
		// nodes previously used by another campaign.
		mission_campaign_close();

		strcpy( Campaign.filename, filename );

		// only initialize the sexpression stuff when Fred isn't running.  It'll screw things up major
		// if it does
		if ( !Fred_running ){
			init_sexp();		// must initialize the sexpression stuff
		}

		read_file_text( filename );
		reset_parse();
		memset( &Campaign, 0, sizeof(Campaign) );

		// copy filename to campaign structure minus the extension
		len = strlen(filename) - 4;
		Assert(len < MAX_FILENAME_LEN);
		strncpy(Campaign.filename, filename, len);
		Campaign.filename[len] = 0;

		required_string("$Name:");
		stuff_string( name, F_NAME, NULL );
		
		//Store campaign name in the global struct
		strcpy( Campaign.name, name );

		required_string( "$Type:" );
		stuff_string( type, F_NAME, NULL );

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
			Campaign.desc = stuff_and_malloc_string(F_MULTITEXT, NULL, MISSION_DESC_LENGTH);

		// if the type is multiplayer -- get the number of players
		Campaign.num_players = 0;
		if ( Campaign.type != CAMPAIGN_TYPE_SINGLE) {
			required_string("+Num players:");
			stuff_int( &(Campaign.num_players) );
		}		

		// parse the optional ship/weapon information
		mission_campaign_get_sw_info();

		// parse the mission file and actually read in the mission stuff
		Campaign.num_missions = 0;
		while ( required_string_either("#End", "$Mission:") ) {
			cmission *cm;

			required_string("$Mission:");
			stuff_string(name, F_NAME, NULL);
			cm = &Campaign.missions[Campaign.num_missions];
			cm->name = strdup(name);

			cm->briefing_cutscene[0] = 0;
			if ( optional_string("+Briefing Cutscene:") )
				stuff_string( cm->briefing_cutscene, F_NAME, NULL );

			cm->flags = 0;
			if (optional_string("+Flags:"))
				stuff_int(&cm->flags);

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

						return CAMPAIGN_ERROR_SEXP_EXHAUSTED;
					}
				}
			}

			// Do misison looping stuff
			cm->has_mission_loop = 0;
			if ( optional_string("+Mission Loop:") ) {
				cm->has_mission_loop = 1;
			}

			cm->mission_loop_desc = NULL;
			if ( optional_string("+Mission Loop Text:")) {
				cm->mission_loop_desc = stuff_and_malloc_string(F_MULTITEXT, NULL, MISSION_DESC_LENGTH);
			}

			cm->mission_loop_brief_anim = NULL;
			if ( optional_string("+Mission Loop Brief Anim:")) {
				cm->mission_loop_brief_anim = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_FILENAME_LEN);
			}

			cm->mission_loop_brief_sound = NULL;
			if ( optional_string("+Mission Loop Brief Sound:")) {
				cm->mission_loop_brief_sound = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_FILENAME_LEN);
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

						return CAMPAIGN_ERROR_SEXP_EXHAUSTED;
					}
				}
			}

			if (optional_string("+Level:")) {
				stuff_int( &cm->level );
				if ( cm->level == 0 )  // check if the top (root) of the whole tree
					Campaign.next_mission = Campaign.num_missions;

			} else
				Campaign.realign_required = 1;

			if (optional_string("+Position:"))
				stuff_int( &cm->pos );
			else
				Campaign.realign_required = 1;

			if (Fred_running) {
				cm->num_goals = -1;
				cm->num_events = -1;
				cm->notes = NULL;

			} else {
				cm->num_goals = 0;
				cm->num_events = 0;
			}

			cm->goals = NULL;
			cm->events = NULL;
			Campaign.num_missions++;
		}
	}

	// set up the other variables for the campaign stuff.  After initializing, we must try and load
	// the campaign save file for this player.  Since all campaign loads go through this routine, I
	// think this place should be the only necessary place to load the campaign save stuff.  The campaign
	// save file will get written when a mission has ended by player choice.
	Campaign.next_mission = 0;
	Campaign.prev_mission = -1;
	Campaign.current_mission = -1;	

	// loading the campaign will get us to the current and next mission that the player must fly
	// plus load all of the old goals that future missions might rely on.
	if (!Fred_running && load_savefile && (Campaign.type == CAMPAIGN_TYPE_SINGLE)) {
		mission_campaign_savefile_load(Campaign.filename);
	}

	// close localization
	lcl_ext_close();

	return 0;
}

// mission_campaign_load_by_name() loads up a freespace campaign given the filename.  This routine
// is used to load up campaigns when a pilot file is loaded.  Generally, the
// filename will probably be the freespace campaign file, but not necessarily.
int mission_campaign_load_by_name( char *filename )
{
	char name[NAME_LENGTH],test[5];
	int type,max_players;

	// make sure to tack on .fsc on the end if its not there already
	if(strlen(filename) > 0){
		if(strlen(filename) > 4){
			strcpy(test,filename+(strlen(filename)-4));
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
	Campaign_file_names[Num_campaigns] = filename;
	Campaign_names[Num_campaigns] = name;
	Num_campaigns++;
	mission_campaign_load(filename);		
	return 0;
}

int mission_campaign_load_by_name_csfe( char *filename, char *callsign )
{
	Game_mode |= GM_NORMAL;
	strcpy(Player->callsign, callsign);
	return mission_campaign_load_by_name( filename);
}


// mission_campaign_init initializes some variables then loads the default Freespace single player campaign.
void mission_campaign_init()
{
	memset(&Campaign, 0, sizeof(Campaign) );
}

// Fill in the root of the campaign save filename
void mission_campaign_savefile_generate_root(char *filename)
{
	char base[_MAX_FNAME];

	Assert ( strlen(Campaign.filename) != 0 );

#ifdef _WIN32
	// build up the filename for the save file.  There could be a problem with filename length,
	// but this problem can get fixed in several ways -- ignore the problem for now though.
	_splitpath( Campaign.filename, NULL, NULL, base, NULL );
#else
	// mharris FIXME: this may not work...
	strcpy(base, Campaign.filename);
#endif

	Assert ( (strlen(base) + strlen(Player->callsign) + 1) < _MAX_FNAME );

	sprintf( filename, NOX("%s.%s."), Player->callsign, base );
}


// mission_campaign_savefile_save saves the state of the campaign.  This function will probably always be called
// then the player is done flying a mission in the campaign path.  It will save the missions played, the
// state of the goals, etc.
int mission_campaign_savefile_save()
{
	char filename[_MAX_FNAME];
	CFILE *fp;
	int i,j, mission_count;

	memset(filename, 0, _MAX_FNAME);
	mission_campaign_savefile_generate_root(filename);

	// name the file differently depending on whether we're in single player or multiplayer mode
	// single player : *.csg
	strcat( filename, NOX("csg"));	

	fp = cfopen(filename,"wb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);

	if (!fp)
		return errno;

	// Write out campaign file info
	cfwrite_int( CAMPAIGN_FILE_ID,fp );
	cfwrite_int( CAMPAIGN_FILE_VERSION,fp );

	// put in the file signature (single or multiplayer campaign) - see MissionCampaign.h for the #defines
	cfwrite_int( CAMPAIGN_SINGLE_PLAYER_SIG, fp );

	// do we need to write out the filename of the campaign?
	cfwrite_string_len( Campaign.filename, fp );
	cfwrite_int( Campaign.prev_mission, fp );
	cfwrite_int( Campaign.next_mission, fp );
	cfwrite_int( Campaign.loop_reentry, fp );
	cfwrite_int( Campaign.loop_enabled, fp );

	// write out the information for ships/weapons which this player is allowed to use
	cfwrite_int(Num_ship_types, fp);
	cfwrite_int(Num_weapon_types, fp);
	for ( i = 0; i < Num_ship_types; i++ ){
		cfwrite_char( Campaign.ships_allowed[i], fp );
	}

	for ( i = 0; i < Num_weapon_types; i++ ){
		cfwrite_char( Campaign.weapons_allowed[i], fp );
	}

	// write out the completed mission matrix.  Used to tell which missions the player
	// can replay in the simulator.  Also, each completed mission contains a list of the goals
	// that were in the mission along with the goal completion status.
	cfwrite_int( Campaign.num_missions_completed, fp );
	for (i = 0; i < MAX_CAMPAIGN_MISSIONS; i++ ) {
		if ( Campaign.missions[i].completed ) {
			cfwrite_int( i, fp );
			cfwrite_int( Campaign.missions[i].num_goals, fp );
			for ( j = 0; j < Campaign.missions[i].num_goals; j++ ) {
				cfwrite_string_len( Campaign.missions[i].goals[j].name, fp );
				cfwrite_char( Campaign.missions[i].goals[j].status, fp );
			}
			cfwrite_int( Campaign.missions[i].num_events, fp );
			for ( j = 0; j < Campaign.missions[i].num_events; j++ ) {
				cfwrite_string_len( Campaign.missions[i].events[j].name, fp );
				cfwrite_char( Campaign.missions[i].events[j].status, fp );
			}

			// write flags
			cfwrite_int(Campaign.missions[i].flags, fp);
		}
	}

	cfclose( fp );

	// 6/17/98
	// ugh!  due to horrible bug, the stats saved at the end of every level were not written
	// out to disk.  Write out a seperate file to do this.  We will only read it in if we actually
	// find the file.
	memset(filename, 0, _MAX_FNAME);
	mission_campaign_savefile_generate_root(filename);

	// name the file differently depending on whether we're in single player or multiplayer mode
	// single player : *.csg
	strcat( filename, NOX("css"));

	fp = cfopen(filename,"wb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);

	if (!fp)
		return errno;

	// Write out campaign file info
	cfwrite_int( CAMPAIGN_STATS_FILE_ID,fp );
	cfwrite_int( CAMPAIGN_STATS_FILE_VERSION,fp );

	// determine how many missions we are saving -- I think that this method is safer than the method
	// I used for release
	mission_count = 0;
	for ( i = 0; i < Campaign.num_missions; i++ ) {
		if ( Campaign.missions[i].completed ) {
			mission_count++;
		}
	}

	// write out the stats information to disk.	
	cfwrite_int( mission_count, fp );
	for (i = 0; i < Campaign.num_missions; i++ ) {
		if ( Campaign.missions[i].completed ) {
			cfwrite_int( i, fp );
			cfwrite( &Campaign.missions[i].stats, sizeof(scoring_struct), 1, fp );
		}
	}

	cfclose( fp );
	
	return 0;
}


// The following function always only ever ever ever called by CSFE!!!!!
int campaign_savefile_save(char *pname)
{
	if (Campaign.type == CAMPAIGN_TYPE_SINGLE)
		Game_mode &= ~GM_MULTIPLAYER;
	else
		Game_mode |= GM_MULTIPLAYER;

	strcpy(Player->callsign, pname);
	//memcpy(&Campaign, camp, sizeof(campaign));
	return mission_campaign_savefile_save();
}


// the below two functions is internal to this module.  It is here so that I can group the save/load
// functions together.
//

// mission_campaign_savefile_delete deletes any save file in the players directory for the given
// campaign filename
void mission_campaign_savefile_delete( char *cfilename, int is_multi )
{
	char filename[_MAX_FNAME], base[_MAX_FNAME];

#ifdef _WIN32
	_splitpath( cfilename, NULL, NULL, base, NULL );
#else
	// mharris FIXME: this may not work...
	strcpy(base, cfilename);
#endif

	if ( Player->flags & PLAYER_FLAGS_IS_MULTI ) {
		return;	// no such thing as a multiplayer campaign savefile
	}

	sprintf( filename, NOX("%s.%s.csg"), Player->callsign, base );

	cf_delete( filename, CF_TYPE_SINGLE_PLAYERS );
}


void campaign_delete_save( char *cfn, char *pname)
{
	strcpy(Player->callsign, pname);
	mission_campaign_savefile_delete(cfn);
}

// next function deletes all the save files for this particular pilot.  Just call cfile function
// which will delete multiple files
// Player_select_mode tells us whether we are deleting single or multiplayer files
void mission_campaign_delete_all_savefiles( char *pilot_name, int is_multi )
{
	int dir_type, num_files, i;
	char *names[MAX_CAMPAIGNS], spec[MAX_FILENAME_LEN + 2], *ext;
	char filename[1024];
	int (*filter_save)(char *filename);

	if ( is_multi ) {
		return;				// can't have multiplayer campaign save files
	}

	ext = NOX(".csg");
	dir_type = CF_TYPE_SINGLE_PLAYERS;

	sprintf(spec, NOX("%s.*%s"), pilot_name, ext);

	// HACK HACK HACK HACK!!!!  cf_get_file_list is not reentrant.  Pretty dumb because it should
	// be.  I have to save any file filters
	filter_save = Get_file_list_filter;
	Get_file_list_filter = NULL;
	num_files = cf_get_file_list(MAX_CAMPAIGNS, names, dir_type, spec);
	Get_file_list_filter = filter_save;

	for (i=0; i<num_files; i++) {
		strcpy(filename, names[i]);
		strcat(filename, ext);
		cf_delete(filename, dir_type);
		free(names[i]);
	}
}

// mission_campaign_savefile_load takes a filename of a campaign file as a parameter and loads all
// of the information stored in the campaign file.
void mission_campaign_savefile_load( char *cfilename )
{
	char filename[_MAX_FNAME], base[_MAX_FNAME];
	int id, version, i, num, j, num_stats_blocks;
	int type_sig;
	CFILE *fp;

	Assert ( strlen(cfilename) != 0 );

	// probably only called from single player games anymore!!! should be anyway
	Assert( Game_mode & GM_NORMAL );		// get allender or DaveB.  trying to save campaign in multiplayer

	// build up the filename for the save file.  There could be a problem with filename length,
	// but this problem can get fixed in several ways -- ignore the problem for now though.
#ifdef _WIN32
	_splitpath( cfilename, NULL, NULL, base, NULL );
#else
	// mharris FIXME: this may not work...
	strcpy(base, cfilename);
#endif
	Assert ( (strlen(base) + strlen(Player->callsign) + 1) < _MAX_FNAME );

	if(Game_mode & GM_MULTIPLAYER)
		sprintf( filename, NOX("%s.%s.msg"), Player->callsign, base );
	else
		sprintf( filename, NOX("%s.%s.csg"), Player->callsign, base );

	fp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS );
	if ( !fp )
		return;

	id = cfread_int( fp );
	if ( id != CAMPAIGN_FILE_ID ) {
		Warning(LOCATION, "Campaign save file has invalid signature");
		cfclose( fp );
		return;
	}

	version = cfread_int( fp );
	if ( version < CAMPAIGN_FILE_COMPATIBLE_VERSION ) {
		Warning(LOCATION, "Campaign save file too old -- not compatible.  Deleting file.\nYou can continue from here without trouble\n\n");
		cfclose( fp );
		cf_delete( filename, CF_TYPE_SINGLE_PLAYERS );
		return;
	}

	// verify that we are loading the correct type of campaign file for the mode that we are in.
	if(version >= 3)
		type_sig = cfread_int( fp );
	else
		type_sig = CAMPAIGN_SINGLE_PLAYER_SIG;
	// the actual check
	Assert( ((Game_mode & GM_MULTIPLAYER) && (type_sig==CAMPAIGN_MULTI_PLAYER_SIG)) || (!(Game_mode & GM_MULTIPLAYER) && (type_sig==CAMPAIGN_SINGLE_PLAYER_SIG)) );

	Campaign.type = type_sig == CAMPAIGN_SINGLE_PLAYER_SIG ? CAMPAIGN_TYPE_SINGLE : CAMPAIGN_TYPE_MULTI_COOP;

	// read in the filename of the campaign and compare the filenames to be sure that
	// we are reading data that really belongs to this campaign.  I think that this check
	// is redundant.
	cfread_string_len( filename, _MAX_FNAME, fp );
	/*if ( stricmp( filename, cfilename) ) {	//	Used to be !stricmp.  How did this ever work? --MK, 11/9/97
		Warning(LOCATION, "Campaign save file appears corrupt because of mismatching filenames.");
		cfclose(fp);
		return;
	}*/

	Campaign.prev_mission = cfread_int( fp );
	Campaign.next_mission = cfread_int( fp );
	Campaign.loop_reentry = cfread_int( fp );
	Campaign.loop_enabled = cfread_int( fp );

	//  load information about ships/weapons allowed	
	int ship_count, weapon_count;

	// if earlier than mission disk version, use old MAX_SHIP_TYPES, otherwise read from the file		
	if(version <= CAMPAIGN_INITIAL_RELEASE_FILE_VERSION){
		ship_count = CAMPAIGN_SAVEFILE_MAX_SHIPS_OLD;
		weapon_count = CAMPAIGN_SAVEFILE_MAX_WEAPONS_OLD;
	} else {
		ship_count = cfread_int(fp);
		weapon_count = cfread_int(fp);
	}

	for ( i = 0; i < ship_count; i++ ){
		Campaign.ships_allowed[i] = cfread_ubyte( fp );
	}

	for ( i = 0; i < weapon_count; i++ ){
		Campaign.weapons_allowed[i] = cfread_ubyte( fp );
	}	

	// read in the completed mission matrix.  Used to tell which missions the player
	// can replay in the simulator.  Also, each completed mission contains a list of the goals
	// that were in the mission along with the goal completion status.
	Campaign.num_missions_completed = cfread_int( fp );
	for (i = 0; i < Campaign.num_missions_completed; i++ ) {
		num = cfread_int( fp );
		Campaign.missions[num].completed = 1;
		Campaign.missions[num].num_goals = cfread_int( fp );
		
		// be sure to malloc out space for the goals stuff, then zero the memory!!!  Don't do malloc
		// if there are no goals
		Campaign.missions[num].goals = (mgoal *)malloc( Campaign.missions[num].num_goals * sizeof(mgoal) );
		if ( Campaign.missions[num].num_goals > 0 ) {
			memset( Campaign.missions[num].goals, 0, sizeof(mgoal) * Campaign.missions[num].num_goals );
			Assert( Campaign.missions[num].goals != NULL );
		}

		// now read in the goal information for this mission
		for ( j = 0; j < Campaign.missions[num].num_goals; j++ ) {
			cfread_string_len( Campaign.missions[num].goals[j].name, NAME_LENGTH, fp );
			Campaign.missions[num].goals[j].status = cfread_char( fp );
		}

		// get the events from the savefile
		Campaign.missions[num].num_events = cfread_int( fp );
		
		// be sure to malloc out space for the events stuff, then zero the memory!!!  Don't do malloc
		// if there are no events
//		if (Campaign.missions[num].events < 0)
//			Campaign.missions[num].events = 0;
		Campaign.missions[num].events = (mevent *)malloc( Campaign.missions[num].num_events * sizeof(mevent) );
		if ( Campaign.missions[num].num_events > 0 ) {
			memset( Campaign.missions[num].events, 0, sizeof(mevent) * Campaign.missions[num].num_events );
			Assert( Campaign.missions[num].events != NULL );
		}
		
		// now read in the event information for this mission
		for ( j = 0; j < Campaign.missions[num].num_events; j++ ) {
			cfread_string_len( Campaign.missions[num].events[j].name, NAME_LENGTH, fp );
			Campaign.missions[num].events[j].status = cfread_char( fp );
		}

		// now read flags
		Campaign.missions[num].flags = cfread_int(fp);
	}	

	cfclose( fp );

	// 4/17/98
	// now, try and read in the campaign stats saved information.  This code was added for the 1.03 patch
	// since the stats data was never written out to disk.  We try and open the file, and if we cannot find
	// it, then simply return
	sprintf( filename, NOX("%s.%s.css"), Player->callsign, base );

	fp = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS );
	if ( !fp )
		return;

	id = cfread_int( fp );
	if ( id != CAMPAIGN_STATS_FILE_ID ) {
		Warning(LOCATION, "Campaign stats save file has invalid signature");
		cfclose( fp );
		return;
	}

	version = cfread_int( fp );
	if ( version < CAMPAIGN_STATS_FILE_COMPATIBLE_VERSION ) {
		Warning(LOCATION, "Campaign save file too old -- not compatible.  Deleting file.\nYou can continue from here without trouble\n\n");
		cfclose( fp );
		cf_delete( filename, CF_TYPE_SINGLE_PLAYERS );
		return;
	}

	num_stats_blocks = cfread_int( fp );
	for (i = 0; i < num_stats_blocks; i++ ) {
		num = cfread_int( fp );
		cfread( &Campaign.missions[num].stats, sizeof(scoring_struct), 1, fp );
	}

	cfclose(fp);

}


// the following code only ever called by CSFE!!!!
void campaign_savefile_load(char *fname, char *pname)
{
	if (Campaign.type==CAMPAIGN_TYPE_SINGLE) {
		Game_mode &= ~GM_MULTIPLAYER;
		Game_mode &= GM_NORMAL;
	}
	else
		Game_mode |= GM_MULTIPLAYER;
	strcpy(Player->callsign, pname);
	mission_campaign_savefile_load(fname);
}

// mission_campaign_next_mission sets up the internal veriables of the campaign
// structure so the player can play the next mission.  If there are no more missions
// available in the campaign, this function returns -1, else 0 if the mission was
// set successfully
int mission_campaign_next_mission()
{
	if ( (Campaign.next_mission == -1) || (strlen(Campaign.name) == 0) ) // will be set to -1 when there is no next mission
		return -1;

	Campaign.current_mission = Campaign.next_mission;	
	strncpy( Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name, MAX_FILENAME_LEN );

	// check for end of loop.
	if (Campaign.current_mission == Campaign.loop_reentry) {
		Campaign.loop_enabled = 0;
	}

	// reset the number of persistent ships and weapons for the next campaign mission
	Num_granted_ships = 0;
	Num_granted_weapons = 0;
	return 0;
}

// mission_campaign_previous_mission() gets called to go to the previous mission in
// the campaign.  Used only for Red Alert missions
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
	mission_campaign_savefile_save();

	// reset the player stats to be the stats from this level
	memcpy( &Player->stats, &Campaign.missions[Campaign.current_mission].stats, sizeof(Player->stats) );

	strncpy( Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name, MAX_FILENAME_LEN );
	Num_granted_ships = 0;
	Num_granted_weapons = 0;

	return 1;
}

/*
// determine what the next mission is after the current one.  Because this evaluates an sexp,
// and that could check just about anything, the results are only going to be valid in
// certain places.
// DA 12/09/98 -- To allow for mission loops, need to maintain call with store stats
int mission_campaign_eval_next_mission( int store_stats )
{
	char *name;
	int cur, i;
	cmission *mission;

	Campaign.next_mission = -1;
	cur = Campaign.current_mission;
	name = Campaign.missions[cur].name;

	mission = &Campaign.missions[cur];

	// first we must save the status of the current missions goals in the campaign mission structure.
	// After that, we can determine which mission is tagged as the next mission.  Finally, we
	// can save the campaign save file
	// we might have goal and event status if the player replayed a mission
	if ( mission->num_goals > 0 ) {
		free( mission->goals );
	}

	mission->num_goals = Num_goals;
	if ( mission->num_goals > 0 ) {
		mission->goals = (mgoal *)malloc( sizeof(mgoal) * Num_goals );
		Assert( mission->goals != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_goals; i++ ) {
		if ( strlen(Mission_goals[i].name) == 0 ) {
			char name[NAME_LENGTH];

			sprintf(name, NOX("Goal #%d"), i);
			//Warning(LOCATION, "Mission goal in mission %s must have a +Name field! using %s for campaign save file\n", mission->name, name);
			strcpy( mission->goals[i].name, name);
		} else
			strcpy( mission->goals[i].name, Mission_goals[i].name );
		Assert ( Mission_goals[i].satisfied != GOAL_INCOMPLETE );		// should be true or false at this point!!!
		mission->goals[i].status = (char)Mission_goals[i].satisfied;
	}

	// do the same thing for events as we did for goals
	// we might have goal and event status if the player replayed a mission
	if ( mission->num_events > 0 ) {
		free( mission->events );
	}

	mission->num_events = Num_mission_events;
	if ( mission->num_events > 0 ) {
		mission->events = (mevent *)malloc( sizeof(mevent) * Num_mission_events );
		Assert( mission->events != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_mission_events; i++ ) {
		if ( strlen(Mission_events[i].name) == 0 ) {
			char name[NAME_LENGTH];

			sprintf(name, NOX("Event #%d"), i);
			nprintf(("Warning", "Mission goal in mission %s must have a +Name field! using %s for campaign save file\n", mission->name, name));
			strcpy( mission->events[i].name, name);
		} else
			strcpy( mission->events[i].name, Mission_events[i].name );

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

	// maybe store the alltime stats which would be current at the end of this mission
	if ( store_stats ) {
		memcpy( &mission->stats, &Player->stats, sizeof(Player->stats) );
		scoring_backout_accept( &mission->stats );
	}

	if ( store_stats ) {	// second (last) time through, so use choose loop_mission if chosen
		if ( Campaign.loop_enabled ) {
			Campaign.next_mission = Campaign.loop_mission;
		} else {
			// evaluate next mission (straight path)
			if (Campaign.missions[cur].formula != -1) {
				flush_sexp_tree(Campaign.missions[cur].formula);  // force formula to be re-evaluated
				eval_sexp(Campaign.missions[cur].formula);  // this should reset Campaign.next_mission to proper value
			}
		}
	} else {

		// evaluate next mission (straight path)
		if (Campaign.missions[cur].formula != -1) {
			flush_sexp_tree(Campaign.missions[cur].formula);  // force formula to be re-evaluated
			eval_sexp(Campaign.missions[cur].formula);  // this should reset Campaign.next_mission to proper value
		}

		// evaluate mission loop mission (if any) so it can be used if chosen
		if ( Campaign.missions[cur].has_mission_loop ) {
			int copy_next_mission = Campaign.next_mission;
			// Set temporarily to -1 so we know if loop formula fails to assign
			Campaign.next_mission = -1;  // Cannot exit campaign from loop
			if (Campaign.missions[cur].mission_loop_formula != -1) {
				flush_sexp_tree(Campaign.missions[cur].mission_loop_formula);  // force formula to be re-evaluated
				eval_sexp(Campaign.missions[cur].mission_loop_formula);  // this should reset Campaign.next_mission to proper value
			}

			Campaign.loop_mission = Campaign.next_mission;
			Campaign.next_mission = copy_next_mission;
		}
	}

	if (Campaign.next_mission == -1)
		nprintf(("allender", "No next mission to proceed to.\n"));
	else
		nprintf(("allender", "Next mission is number %d [%s]\n", Campaign.next_mission, Campaign.missions[Campaign.next_mission].name));

	return Campaign.next_mission;
} */

// Evaluate next campaign mission - set as Campaign.next_mission.  Also set Campaign.loop_mission
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
	if ( Campaign.missions[cur].has_mission_loop ) {
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

// Store mission's goals and events in Campaign struct
void mission_campaign_store_goals_and_events()
{
	char *name;
	int cur, i;
	cmission *mission;

	cur = Campaign.current_mission;
	name = Campaign.missions[cur].name;

	mission = &Campaign.missions[cur];

	// first we must save the status of the current missions goals in the campaign mission structure.
	// After that, we can determine which mission is tagged as the next mission.  Finally, we
	// can save the campaign save file
	// we might have goal and event status if the player replayed a mission
	if ( mission->num_goals > 0 ) {
		free( mission->goals );
	}

	mission->num_goals = Num_goals;
	if ( mission->num_goals > 0 ) {
		mission->goals = (mgoal *)malloc( sizeof(mgoal) * Num_goals );
		Assert( mission->goals != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_goals; i++ ) {
		if ( strlen(Mission_goals[i].name) == 0 ) {
			char name[NAME_LENGTH];

			sprintf(name, NOX("Goal #%d"), i);
			//Warning(LOCATION, "Mission goal in mission %s must have a +Name field! using %s for campaign save file\n", mission->name, name);
			strcpy( mission->goals[i].name, name);
		} else
			strcpy( mission->goals[i].name, Mission_goals[i].name );
		Assert ( Mission_goals[i].satisfied != GOAL_INCOMPLETE );		// should be true or false at this point!!!
		mission->goals[i].status = (char)Mission_goals[i].satisfied;
	}

	// do the same thing for events as we did for goals
	// we might have goal and event status if the player replayed a mission
	if ( mission->num_events > 0 ) {
		free( mission->events );
	}

	mission->num_events = Num_mission_events;
	if ( mission->num_events > 0 ) {
		mission->events = (mevent *)malloc( sizeof(mevent) * Num_mission_events );
		Assert( mission->events != NULL );
	}

	// copy the needed info from the Mission_goal struct to our internal structure
	for (i = 0; i < Num_mission_events; i++ ) {
 		if ( strlen(Mission_events[i].name) == 0 ) {
			char name[NAME_LENGTH];

			sprintf(name, NOX("Event #%d"), i);
			nprintf(("Warning", "Mission goal in mission %s must have a +Name field! using %s for campaign save file\n", mission->name, name));
			strcpy( mission->events[i].name, name);
		} else
			strcpy( mission->events[i].name, Mission_events[i].name );

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
}

// this function is called when the player's mission is over.  It updates the internal store of goals
// and their status then saves the state of the campaign in the campaign file.  This gets called
// after player accepts mission results in debriefing.
void mission_campaign_mission_over()
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

	// DKA 12/11/98 - Unneeded already evaluated and stored
	// determine what new mission we are moving to.
	//	mission_campaign_eval_next_mission(1);

	// update campaign.mission stats (used to allow backout inRedAlert)
	memcpy( &mission->stats, &Player->stats, sizeof(Player->stats) );
	if(!(Game_mode & GM_MULTIPLAYER)){
		scoring_backout_accept( &mission->stats );
	}

	// if we are moving to a new mission, then change our data.  If we are staying on the same mission,
	// then we don't want to do anything.  Remove information about goals/events
	if ( Campaign.next_mission != mission_num ) {
		Campaign.prev_mission = mission_num;
		Campaign.current_mission = -1;
		Campaign.num_missions_completed++;
		Campaign.missions[mission_num].completed = 1;

		// save the scoring values from the previous mission at the start of this mission -- for red alert

		// save the state of the campaign in the campaign save file and move to the end_game state
		if (Campaign.type == CAMPAIGN_TYPE_SINGLE) {
			mission_campaign_savefile_save();
		}

	} else {
		// free up the goals and events which were just malloced.  It's kind of like erasing any fact
		// that the player played this mission in the campaign.
		free( mission->goals );
		mission->num_goals = 0;

		free( mission->events );
		mission->num_events = 0;

		Sexp_nodes[mission->formula].value = SEXP_UNKNOWN;
	}

	Assert(Player);
	if (Campaign.missions[Campaign.next_mission].flags & CMISSION_FLAG_BASTION){
		Player->on_bastion = 1;
	} else {
		Player->on_bastion = 0;
	}

	mission_campaign_next_mission();			// sets up whatever needs to be set to actually play next mission
}

// called when the game closes -- to get rid of memory errors for Bounds checker
void mission_campaign_close()
{
	int i;

	if (Campaign.desc)
		free(Campaign.desc);

	// be sure to remove all old malloced strings of Mission_names
	// we must also free any goal stuff that was from a previous campaign
	for ( i=0; i<Campaign.num_missions; i++ ) {
		if ( Campaign.missions[i].name ){
			free(Campaign.missions[i].name);
		}

		if (Campaign.missions[i].notes){
			free(Campaign.missions[i].notes);
		}

		if ( Campaign.missions[i].num_goals > 0 ){
			free ( Campaign.missions[i].goals );
		}

		if ( Campaign.missions[i].num_events > 0 ){
			free ( Campaign.missions[i].events );
		}

		if ( !Fred_running ){
			sexp_unmark_persistent(Campaign.missions[i].formula);		// free any sexpression nodes used by campaign.
		}

		Campaign.missions[i].num_goals = 0;
		Campaign.missions[i].num_events = 0;
	}
}

// extract the mission filenames for a campaign.  
//
// filename	=>	name of campaign file
//	dest		=> storage for the mission filename, must be already allocated
// num		=> output parameter for the number of mission filenames in the campaign
//
// note that dest should allocate at least dest[MAX_CAMPAIGN_MISSIONS][NAME_LENGTH]
int mission_campaign_get_filenames(char *filename, char dest[][NAME_LENGTH], int *num)
{
	int	rval;

	// read the mission file and get the list of mission filenames
	if ((rval = setjmp(parse_abort)) != 0) {
		return rval;

	} else {
		read_file_text(filename);
		Assert(strlen(filename) < MAX_FILENAME_LEN - 1);  // make sure no overflow

		reset_parse();
		required_string("$Name:");
		advance_to_eoln(NULL);

		required_string( "$Type:" );
		advance_to_eoln(NULL);

		// parse the mission file and actually read in the mission stuff
		*num = 0;
		while ( skip_to_string("$Mission:") == 1 ) {
			stuff_string(dest[*num], F_NAME, NULL);
			(*num)++;
		}
	}

	return 0;
}

// function to read the goals and events from a mission in a campaign file and store that information
// in the campaign structure for use in the campaign editor, error checking, etc
void read_mission_goal_list(int num)
{
	char *filename, notes[NOTES_LENGTH], goals[MAX_GOALS][NAME_LENGTH];
	char events[MAX_MISSION_EVENTS][NAME_LENGTH];
	int i, z, r, event_count, count = 0;

	filename = Campaign.missions[num].name;
	if ((r = setjmp(parse_abort))>0) {
		Warning(LOCATION, "Error reading \"%s\" (code = %d)", filename, r);
		return;
	}

	// open localization
	lcl_ext_open();	
	
	read_file_text(filename);
	init_parse();

	// first, read the mission notes for this mission.  Used in campaign editor
	if (skip_to_string("#Mission Info")) {
		if (skip_to_string("$Notes:")) {
			stuff_string(notes, F_NOTES, NULL);
			if (Campaign.missions[num].notes){
				free(Campaign.missions[num].notes);
			}

			Campaign.missions[num].notes = (char *) malloc(strlen(notes) + 1);
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
				stuff_string(events[event_count], F_NAME, NULL);
			} else {
				sprintf(events[event_count], NOX("Event #%d"), event_count + 1);
			}

			event_count++;
			Assert(event_count < MAX_MISSION_EVENTS);
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
				stuff_string(goals[count], F_NAME, NULL);
			} else {
				sprintf(goals[count], NOX("Goal #%d"), count + 1);
			}

			count++;
			Assert(count < MAX_GOALS);
		}
	}

	Campaign.missions[num].num_goals = count;
	if (count) {
		Campaign.missions[num].goals = (mgoal *) malloc(count * sizeof(mgoal));
		Assert(Campaign.missions[num].goals);  // make sure we got the memory
		memset(Campaign.missions[num].goals, 0, count * sizeof(mgoal));

		for (i=0; i<count; i++){
			strcpy(Campaign.missions[num].goals[i].name, goals[i]);
		}
	}
		// copy the events
	Campaign.missions[num].num_events = event_count;
	if (event_count) {
		Campaign.missions[num].events = (mevent *)malloc(event_count * sizeof(mevent));
		Assert ( Campaign.missions[num].events );
		memset(Campaign.missions[num].events, 0, event_count * sizeof(mevent));

		for (i = 0; i < event_count; i++ ){
			strcpy(Campaign.missions[num].events[i].name, events[i]);
		}
	}

	// close localization
	lcl_ext_close();
}

// function to return index into Campaign's list of missions of the mission with the given
// filename.  This function tried to be a little smart about filename looking for the .fsm
// extension since filenames are stored with the extension in the campaign file.  Returns
// index of mission in campaign structure.  -1 if mission name not found.
int mission_campaign_find_mission( char *name )
{
	int i;
	char realname[_MAX_PATH];

	// look for an extension on the file.  If no extension, add default ".fsm" onto the
	// end of the filename
	strcpy(realname, name );
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

	// no soup for you!
	movie_play( filename );
}

// return nonzero if the passed filename is a multiplayer campaign, 0 otherwise
int mission_campaign_parse_is_multi(char *filename, char *name)
{	
	int i;
	char temp[50];
	
	read_file_text( filename );
	reset_parse();
	
	required_string("$Name:");
	stuff_string( temp, F_NAME, NULL );	
	if ( name )
		strcpy( name, temp );

	required_string( "$Type:" );
	stuff_string( temp, F_NAME, NULL );

	for (i = 0; i < MAX_CAMPAIGN_TYPES; i++ ) {
		if ( !stricmp(temp, campaign_types[i]) ) {
			return i;
		}
	}

	Error(LOCATION, "Unknown campaign type %s", temp );
	return -1;
}

// functions to save persistent information during a mission -- which then might or might not get
// saved out when the mission is over depending on whether player replays mission or commits.
void mission_campaign_save_persistent( int type, int sindex )
{
	// based on the type of information, save it off for possible saving into the campsign
	// savefile when the mission is over
	if ( type == CAMPAIGN_PERSISTENT_SHIP ) {
		Assert( Num_granted_ships < MAX_SHIP_TYPES );
		Granted_ships[Num_granted_ships] = sindex;
		Num_granted_ships++;
	} else if ( type == CAMPAIGN_PERSISTENT_WEAPON ) {
		Assert( Num_granted_weapons < MAX_WEAPON_TYPES );
		Granted_weapons[Num_granted_weapons] = sindex;
		Num_granted_weapons++;
	} else
		Int3();
}

// returns 0: loaded, !0: error
int mission_load_up_campaign()
{
	if (strlen(Player->current_campaign))
		return mission_campaign_load(Player->current_campaign);
	else
		return mission_campaign_load(BUILTIN_CAMPAIGN);
}

// for end of campaign in the single player game.  Called when the end of campaign state is
// entered, which is triggered when the end-campaign sexpression is hit

void mission_campaign_end_init()
{
	// no need to do any initialization.
}

void mission_campaign_end_do()
{
	// play the movies
	event_music_level_close();
	mission_goal_fail_incomplete();
	scoring_level_close();
	mission_campaign_mission_over();

	// eventually we'll want to play one of two options (good ending or bad ending)
	// did the supernova blow?
	if(Supernova_status == SUPERNOVA_HIT){
		// no soup for you!
		movie_play_two("endpart1.mve", "endprt2b.mve");			// good ending
	} else {
		// no soup for you!
		movie_play_two("endpart1.mve", "endprt2a.mve");			// good ending
	}	

#ifdef FS2_DEMO
	gameseq_post_event( GS_EVENT_END_DEMO );
#else	
	gameseq_post_event( GS_EVENT_MAIN_MENU );
#endif
}

void mission_campaign_end_close()
{
	// nothing to do here.
}


// skip to the next mission in the campaign
// this also posts the state change by default.  pass 0 to override that
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
	mission_campaign_store_goals_and_events();

	// now set the next mission
	mission_campaign_eval_next_mission();

	// clear out relevant player vars
	Player->failures_this_session = 0;
	Player->show_skip_popup = 1;

	if (start_game) {
		// proceed to next mission or main hall
		if ((Campaign.missions[Campaign.current_mission].has_mission_loop) && (Campaign.loop_mission != -1)) {
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


// breaks your ass out of the loop
// this also posts the state change
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


// used for jumping to a particular campaign mission
// all pvs missions marked skipped
// this relies on correct mission ordering in the campaign file
void mission_campaign_jump_to_mission(char *name)
{
	int i = 0;
	char dest_name[64];

	// load in the campaign junk
	mission_load_up_campaign();

	// tack the .fs2 onto the input name
	strcpy(dest_name, name);
	strcat(name, ".fs2");

	// search for our mission
	for (i=0; i<Campaign.num_missions; i++) {
		if ((Campaign.missions[i].name != NULL) && !stricmp(Campaign.missions[i].name, name) ) {
			Campaign.next_mission = i;
			Campaign.prev_mission = i-1;
			mission_campaign_next_mission();
			Game_mode |= GM_CAMPAIGN_MODE;
			gameseq_post_event(GS_EVENT_START_GAME);
			return;
		} else {
			Campaign.missions[i].flags |= CMISSION_FLAG_SKIPPED;
			Campaign.num_missions_completed = i;
		}
	}

	// if we got here, no match was found
	// restart the campaign
	mission_campaign_savefile_delete(Campaign.filename);
	mission_campaign_load(Campaign.filename);
}
