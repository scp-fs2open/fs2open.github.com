/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
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
#include "hud/hudparse.h"
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
		strcpy_s( tmp[i], Recent_missions[i] );
	}

	// get a pointer to just the basename of the filename (including extension)
	p = strrchr(filename, DIR_SEPARATOR_CHAR);
	if ( p == NULL ) {
		p = filename;
	} else {
		p++;
	}

	Assert(strlen(p) < MAX_FILENAME_LEN);
	strcpy_s( Recent_missions[0], p );

	j = 1;
	for ( i = 0; i < Num_recent_missions; i++ ) {
		if ( stricmp(Recent_missions[0], tmp[i]) ) {
			strcpy_s(Recent_missions[j++], tmp[i]);
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
	ext = strrchr(filename, '.');
	if (ext) {
		mprintf(( "Hmmm... Extension passed to mission_load...\n" ));
		*ext = 0;				// remove any extension!
	}

	strcat_s(filename, FS_MISSION_FILE_EXT);

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
	init_hud();
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
	strcpy_s(wild_card, NOX("*"));
	strcat_s(wild_card, FS_MISSION_FILE_EXT);
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
		strcpy_s(Campaign_name_list[i+1], Campaign_names[i]);
	}
	strcpy_s(Campaign_name_list[0], NOX("All campaigns"));
	strcpy_s(Campaign_name_list[1], NOX("Player Missions"));

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
				strcpy_s(mission_name, mlm_missions[selected]);
			} else if (Campaign_filter_index == 1 )	{
				strcpy_s( mission_name, jtmp_missions[selected]);
			} else {
				strcpy_s(mission_name, Campaign_missions[selected]);
			}
			strncpy( mission_name_final, mission_name, MAX_FILENAME_LEN );
		}

		// go
		strcpy_s(Game_current_mission_filename, mission_name_final);
		mprintf(( "Selected '%s'\n", Game_current_mission_filename ));
		gameseq_post_event(GS_EVENT_START_GAME);			
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

