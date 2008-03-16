/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionLoad.cpp $
 * $Revision: 2.11.2.1 $
 * $Date: 2006-06-22 14:59:45 $
 * $Author: taylor $
 *
 * C source module for mission loading
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2006/04/20 06:32:07  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.10  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.9  2005/05/12 17:49:13  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.8  2005/04/01 07:31:10  taylor
 * *that blasted Enter key*, just fixing the log... nothing to see here...
 *
 * Revision 2.7  2005/04/01 07:27:32  taylor
 * some minor Linux fixage
 *
 * Revision 2.6  2004/07/26 20:47:37  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:32:54  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.2  2002/12/02 23:53:49  Goober5000
 * fixed misspelling
 *
 * Revision 2.1.2.1  2002/09/24 18:56:43  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     7/20/99 1:49p Dave
 * Peter Drake build. Fixed some release build warnings.
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
 * 102   5/19/98 1:19p Allender
 * new low level reliable socket reading code.  Make all missions/campaign
 * load/save to data missions folder (i.e. we are rid of the player
 * missions folder)
 * 
 * 101   5/10/98 10:05p Allender
 * only show cutscenes which have been seen before.  Made Fred able to
 * write missions anywhere, defaulting to player misison folder, not data
 * mission folder.  Fix FreeSpace code to properly read missions from
 * correct locations
 * 
 * 100   4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 99    2/23/98 6:55p Lawrance
 * Rip out obsolete code.
 * 
 * 98    2/23/98 8:53a John
 * String externalization
 * 
 * 97    1/19/98 9:37p Allender
 * Great Compiler Warning Purge of Jan, 1998.  Used pragma's in a couple
 * of places since I was unsure of what to do with code.
 * 
 * 96    1/17/98 8:49p Hoffoss
 * Fixed mission_load() calls to handle failure correctly.
 * 
 * 95    12/28/97 1:34p John
 * Fixed yet another mission filename bug
 * 
 * 94    12/28/97 12:42p John
 * Put in support for reading archive files; Made missionload use the
 * cf_get_file_list function.   Moved demos directory out of data tree.
 * 
 * 93    12/27/97 2:39p John
 * Took out the outdated ui_getfilelist functions.  Made the mission load
 * screen use cf_get_filelist instead.  Fixed a bug in mission load that
 * crashed the program if there are no missions available.
 * 
 * 92    12/23/97 12:00p Allender
 * change write_pilot_file to *not* take is_single as a default parameter.
 * causing multiplayer pilots to get written to the single player folder
 * 
 * 91    11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 * 
 * 90    10/31/97 11:27a John
 * appended path for j:\tmp missions
 * 
 * 89    10/31/97 11:19a John
 * added filter category for j:\tmp\*.fsm
 * 
 * 88    10/12/97 5:22p Lawrance
 * have ESC back out of mission load screen
 * 
 * 87    9/18/97 10:19p Lawrance
 * Add a mission campaign filter to the load screen
 * 
 * 86    9/16/97 2:41p Allender
 * beginning of code to change way player starts are handled.  Reordered
 * some code when missions loading since player ship is now created at
 * mission load time instead of before misison load
 * 
 * 85    8/25/97 5:47p Mike
 * Increase number of missions supported in mission load list (outside
 * campaign) to 256 and Assert() if there are more than 256.
 * 
 * 84    8/20/97 5:19p Hoffoss
 * Fixed bug where creating a new pilot causes the mission load mission
 * list box to be empty.
 * 
 * 83    7/28/97 10:53a Lawrance
 * initialize a timestamp
 * 
 * 82    7/17/97 4:25p John
 * First, broken, stage of changing config stuff
 * 
 * 81    7/05/97 1:47p Lawrance
 * write pilot file when a mission is loaded
 * 
 * 80    6/26/97 5:53p Lawrance
 * save recently played missions, allow player to choose from list
 * 
 * 79    6/12/97 12:39p John
 * made ui use freespace colors
 * 
 * 78    6/12/97 11:35a John
 * more menu backgrounds
 * 
 * 77    5/14/97 1:33p Allender
 * remmoved extern declaration
 * 
 * 76    5/12/97 4:59p Allender
 * move the rest of the mission initialization functions into
 * game_level_init().  All mission loading now going through this
 * fucntion.
 * 
 * 75    5/12/97 3:21p Allender
 * re-ordered mission load code into single function in FreeSpace.
 * Simulation part now runs as seperate thread
 * 
 * 74    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 73    4/28/97 5:43p Lawrance
 * allow hotkey assignment screen to work from ship selection
 * 
 * 72    4/25/97 11:31a Allender
 * Campaign state now saved in campaign save file in player directory.
 * Made some global variables follow naming convention.  Solidified
 * continuing campaigns based on new structure
 * 
 * 71    4/23/97 4:46p Allender
 * remove unused code
 * 
 * 70    4/23/97 3:21p Allender
 * more campaign stuff -- mission branching through campaign file now
 * works!!!!
 * 
 * 69    4/22/97 10:44a Allender
 * more campaign stuff.  Info about multiple campaigns now stored in
 * player file -- not saving some player information in save games.
 * 
 * 68    4/18/97 9:59a Allender
 * more campaign stuff.  All campaign related varaibles now stored in
 * campaign structure
 * 
 * 67    4/17/97 9:02p Allender
 * new campaign stuff.  all campaign related material stored in external
 * file.  Continuing campaign won't work at this time
 * 
 * 66    4/15/97 4:37p Lawrance
 * removed unused variables
 *
*/

#include "mission/missionload.h"
#include "mission/missionparse.h"    
#include "missionui/missionshipchoice.h"
#include "mission/missioncampaign.h"
#include "playerman/managepilot.h"
#include "freespace2/freespace.h"
#include "io/key.h"
#include "gamesequence/gamesequence.h"
#include "ui/ui.h"
#include "globalincs/alphacolors.h"
#include "cfile/cfilesystem.h"



extern mission The_mission;  // need to send this info to the briefing
extern int shifted_ascii_table[];
extern int ascii_table[];

// -----------------------------------------------
// For recording most recent missions played
// -----------------------------------------------
char	Recent_missions[MAX_RECENT_MISSIONS][MAX_FILENAME_LEN];
int	Num_recent_missions;


// -----------------------------------------------------
// ml_update_recent_missions()
//
//	Update the Recent_missions[][] array
//
void ml_update_recent_missions(char *filename)
{
	char	tmp[MAX_RECENT_MISSIONS][MAX_FILENAME_LEN], *p;
	int	i,j;
	

	for ( i = 0; i < Num_recent_missions; i++ ) {
		strcpy( tmp[i], Recent_missions[i] );
	}

	// get a pointer to just the basename of the filename (including extension)
	p = strrchr(filename, DIR_SEPARATOR_CHAR);
	if ( p == NULL ) {
		p = filename;
	} else {
		p++;
	}

	Assert(strlen(p) < MAX_FILENAME_LEN);
	strcpy( Recent_missions[0], p );

	j = 1;
	for ( i = 0; i < Num_recent_missions; i++ ) {
		if ( stricmp(Recent_missions[0], tmp[i]) ) {
			strcpy(Recent_missions[j++], tmp[i]);
			if ( j >= MAX_RECENT_MISSIONS ) {
				break;
			}
		}
	}

	Num_recent_missions = j;
	Assert(Num_recent_missions <= MAX_RECENT_MISSIONS);
}

// Mission_load takes no parameters.
// It sets the following global variables
//   Game_current_mission_filename

// returns -1 if failed, 0 if successful
int mission_load(char *filename_ext)
{
	char filename[128], *ext;

	if ( (filename_ext != NULL) && (Game_current_mission_filename != filename_ext) )
		strncpy(Game_current_mission_filename, filename_ext, MAX_FILENAME_LEN-1);

	mprintf(("MISSION LOAD: '%s'\n", filename_ext));

	strncpy(filename, filename_ext, 127);
	ext = strchr(filename, '.');
	if (ext) {
		mprintf(( "Hmmm... Extension passed to mission_load...\n" ));
		*ext = 0;				// remove any extension!
	}

	strcat(filename, FS_MISSION_FILE_EXT);

	// does the magical mission parsing
	// creates all objects, except for the player object
	// save the player object later since the player may get
	// to choose the type of ship that he is to fly
	// return value of 0 indicates success, other is failure.

	if ( parse_main(filename) )
		return -1;

	if (Select_default_ship) {
		int ret;
		ret = create_default_player_ship();
		Assert(!ret);
	}

	ml_update_recent_missions(filename_ext);  // update recently played missions list
	write_pilot_file();
	return 0;
}

//====================================
// Mission Load Menu stuff
#define MLM_MAX_MISSIONS 256
int mlm_active=0;
UI_WINDOW mlm_window;
UI_LISTBOX mlm_mission_list;
UI_LISTBOX recent_mission_list;
UI_LISTBOX campaign_filter;

UI_BUTTON mlm_ok, mlm_cancel;
char * mlm_missions[MLM_MAX_MISSIONS];
char * recent_missions[MAX_RECENT_MISSIONS];
char * campaign_names[MAX_CAMPAIGNS+2];
char * campaign_missions[MAX_CAMPAIGN_MISSIONS];
int mlm_nfiles = 0;
static int	last_recent_current = -1;
static int	last_mlm_current = -1;
static int	Campaign_filter_index;

char * jtmp_missions[MLM_MAX_MISSIONS];
int jtmp_nfiles = 0;


void ml_change_listbox()
{
	if ( !Num_recent_missions 	|| !mlm_nfiles )
		return;

	if ( mlm_mission_list.current() != -1 ) {
		mlm_mission_list.set_current(-1);
		last_mlm_current = -1;
		recent_mission_list.set_focus();
		recent_mission_list.set_current(0);
		return;
	}

	if ( recent_mission_list.current() != -1 ) {
		recent_mission_list.set_current(-1);
		last_recent_current = -1;
		mlm_mission_list.set_focus();
		mlm_mission_list.set_current(0);
		return;
	}
}

static char Campaign_missions[MAX_CAMPAIGN_MISSIONS][NAME_LENGTH];
static char Campaign_name_list[MAX_CAMPAIGNS+2][NAME_LENGTH];
static int	Num_campaign_missions;

// get the mission filenames that make up a campaign
extern int mission_campaign_get_filenames(char *filename, char dest[][NAME_LENGTH], int *num);

void mission_load_menu_init()
{
	int i;
	char wild_card[256];
	Assert( mlm_active == 0 );
	mlm_active = 1;

	memset(wild_card, 0, 256);
	strcpy(wild_card, NOX("*"));
	strcat(wild_card, FS_MISSION_FILE_EXT);
	mlm_nfiles = cf_get_file_list( MLM_MAX_MISSIONS, mlm_missions, CF_TYPE_MISSIONS, wild_card, CF_SORT_NAME );
	jtmp_nfiles = 0;	
		
	Assert(mlm_nfiles <= MLM_MAX_MISSIONS);

	mlm_window.create( 100,100,500,300, 0 );	//WIN_DIALOG

	mlm_ok.create( &mlm_window, NOX("Ok"), 125, 420, 80, 40 );
	mlm_cancel.create( &mlm_window, NOX("Cancel"), 250, 420, 80, 40 );
	mlm_cancel.set_hotkey( KEY_ESC );

	mlm_mission_list.create( &mlm_window, 450, 150, 150, 200, mlm_nfiles, mlm_missions );

	for ( i = 0; i < Num_recent_missions; i++ ) {
		recent_missions[i] = Recent_missions[i];
	}
	recent_mission_list.create( &mlm_window, 250, 150, 150, 200, Num_recent_missions, recent_missions );

	mlm_mission_list.set_focus();	
	mlm_mission_list.set_current(0);


	mission_campaign_build_list(0);
	for ( i = 0; i < Num_campaigns; i++ ) {
		strcpy(Campaign_name_list[i+1], Campaign_names[i]);
	}
	strcpy(Campaign_name_list[0], NOX("All campaigns"));
	strcpy(Campaign_name_list[1], NOX("Player Missions"));

	for ( i = 0; i < Num_campaigns+2; i++ ) {
		campaign_names[i] = Campaign_name_list[i];
	}

	campaign_filter.create( &mlm_window, 50, 150, 150, 200, Num_campaigns+2, campaign_names );
	Campaign_filter_index = 0;
	campaign_filter.set_current(Campaign_filter_index);
}

void mission_load_menu_do()
{
	int	selected, key_in, recent_current, mlm_current, use_recent_flag, i;
	

	Assert( mlm_active == 1 );

	key_in = mlm_window.process();

	if ( key_in ) {

		switch ( key_in & KEY_MASK ) {

		case KEY_UP:
		case KEY_DOWN:
		case KEY_HOME:
		case KEY_END:
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
		case KEY_ENTER:
			break;

		case KEY_RIGHT:
		case KEY_LEFT:
			ml_change_listbox();
			break;

		case KEY_ESC:
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;

		default:
			break;

		} // end switch

	}

	if ( campaign_filter.current() != Campaign_filter_index ) {
		Campaign_filter_index = campaign_filter.current();

		if ( Campaign_filter_index > 1 ) {
			mission_campaign_get_filenames(Campaign_file_names[Campaign_filter_index-2], Campaign_missions, &Num_campaign_missions);

 			for ( i = 0; i < Num_campaign_missions; i++ ) {
				campaign_missions[i] = Campaign_missions[i];
			}
			mlm_mission_list.set_new_list(Num_campaign_missions, campaign_missions);
		} else if ( Campaign_filter_index == 0 ) {
			mlm_mission_list.set_new_list(mlm_nfiles, mlm_missions);
		} else if ( Campaign_filter_index == 1 ) {
			mlm_mission_list.set_new_list(jtmp_nfiles, jtmp_missions); 
		}
		mlm_current = 0;
	}

	mlm_current = mlm_mission_list.current();
	recent_current = recent_mission_list.current();

	if ( mlm_current != last_mlm_current ) {
		recent_mission_list.set_current(-1);
		last_recent_current = -1;
	}
	last_mlm_current = mlm_current;

	if ( recent_current != last_recent_current ) {
		mlm_mission_list.set_current(-1);
		last_mlm_current = -1;
	}
	last_recent_current = recent_current;

	if (mlm_cancel.pressed())
		gameseq_post_event(GS_EVENT_MAIN_MENU);

	// Check if they hit OK, if so, use the current listbox
	// selection.
	selected = -1;
	use_recent_flag = 0;
	if (mlm_ok.pressed())	{
		selected = mlm_mission_list.current();
		if ( selected == -1 ) {
			selected = recent_mission_list.current();
			use_recent_flag = 1;
		}
	} else	{
		// If they didn't hit OK, then check for a double-click on
		// a list box item.
		selected = mlm_mission_list.selected();
		if ( selected == -1 ) {
			selected = recent_mission_list.selected();
			use_recent_flag = 1;
		}
	}

	char mission_name_final[512] = "";

	if ( selected > -1  )	{
		Campaign.current_mission = -1;
		if ( use_recent_flag ) {
			strncpy( mission_name_final, recent_missions[selected], MAX_FILENAME_LEN );
		} else {
			char mission_name[NAME_LENGTH];
			if ( Campaign_filter_index == 0 )	{
				strcpy(mission_name, mlm_missions[selected]);
			} else if (Campaign_filter_index == 1 )	{
				strcpy( mission_name, jtmp_missions[selected]);
			} else {
				strcpy(mission_name, Campaign_missions[selected]);
			}
			strncpy( mission_name_final, mission_name, MAX_FILENAME_LEN );
		}

		// go
#ifdef PD_BUILD
		// if this valid
		if((game_find_builtin_mission(mission_name_final) != NULL) || strstr(mission_name_final, "peterdrake")){
			strcpy(Game_current_mission_filename, mission_name_final);
			mprintf(( "Selected '%s'\n", Game_current_mission_filename ));
			gameseq_post_event(GS_EVENT_START_GAME);			
		}
#else
		strcpy(Game_current_mission_filename, mission_name_final);
		mprintf(( "Selected '%s'\n", Game_current_mission_filename ));
		gameseq_post_event(GS_EVENT_START_GAME);			
#endif
	}

	gr_clear();
	gr_set_color_fast( &Color_bright );
	gr_printf( 0x8000, 10, NOX("Select Mission") );

	gr_printf( 50, 135, NOX("Campaign Filter"));
	gr_printf( 250, 135, NOX("Recently Played"));
	gr_printf( 450, 135, NOX("Mission List"));
	mlm_window.draw();

	gr_flip();
}

void mission_load_menu_close()
{
	int i;

	Assert( mlm_active == 1 );
	mlm_active = 0;

	for (i=0; i<mlm_nfiles; i++ )	{
		if (mlm_missions[i] )	{
			vm_free(mlm_missions[i]);
			mlm_missions[i] = NULL;
		}
	}

	for (i=0; i<jtmp_nfiles; i++ )	{
		if (jtmp_missions[i] )	{
			vm_free(jtmp_missions[i]);
			jtmp_missions[i] = NULL;
		}
	}


	mlm_window.destroy();

}

