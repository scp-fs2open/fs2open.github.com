/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_update.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:08 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 9     10/14/99 3:35p Jefff
 * new xstrs
 * 
 * 8     8/24/99 1:50a Dave
 * Fixed client-side afterburner stuttering. Added checkbox for no version
 * checking on PXO join. Made button info passing more friendly between
 * client and server.
 * 
 * 7     8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 6     5/19/99 4:07p Dave
 * Moved versioning code into a nice isolated common place. Fixed up
 * updating code on the pxo screen. Fixed several stub problems.
 * 
 * 5     2/04/99 6:29p Dave
 * First full working rev of FS2 PXO support.  Fixed Glide lighting
 * problems.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 15    9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and Freespace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 14    9/09/98 5:53p Dave
 * Put in new tracker packets in API. Change cfile to be able to checksum
 * portions of a file.
 * 
 * 13    7/24/98 11:13a Allender
 * change way that the version.nfo file is parsed so that bogus returns
 * from the HTTP server are accurately caught
 * 
 * 12    7/20/98 12:51p Allender
 * don't remove the version.nfo file if we are exiting to launcher for
 * update.  The launcher asssumes that the file is already present.
 * 
 * 11    7/15/98 10:50a Allender
 * changed web site to get version.nfo from
 * 
 * 10    7/15/98 10:17a Dave
 * Made the website address lookup more robust.
 * 
 * 9     7/15/98 10:04a Dave
 * Change connect() procedure so that it uses non blocking sockets and
 * freespace doesn't "hang"
 * 
 * 8     7/13/98 10:30a Lawrance
 * add index numbers for newly localized strings
 * 
 * 7     7/13/98 10:12a Dave
 * Remove improperly localized strings.
 * 
 * 6     7/10/98 5:04p Dave
 * Fix connection speed bug on standalone server.
 * 
 * 5     7/10/98 11:33a Dave
 * Give the user extra options when the need for updating is detected on
 * PXO screen. Send special command line arg to fslauncher.
 * 
 * 4     7/09/98 6:07p Dave
 * Format the text in the error dialog better.
 * 
 * 3     7/09/98 6:01p Dave
 * Firsts full version of PXO updater. Put in stub for displaying
 * connection status.
 * 
 * 2     7/09/98 4:51p Dave
 * First revision of PXO autoupdate check system.
 * 
 * 1     7/09/98 2:09p Dave
 * 
 *
 * $NoKeywords: $
 */

#include <winsock.h>
#include "network/multi_update.h"
#include "popup/popup.h"
#include "gamesequence/gamesequence.h"
#include "osapi/osregistry.h"
#include "io/timer.h"
#include "globalincs/version.h"
#include "inetfile/inetgetfile.h"
#include "cfile/cfile.h"

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE DEFINES/VARS
//

// file get object
InetGetFile *Multi_update_get = NULL;

// set this to be true so that game_shutdown() will fire up the launcher, then post a quit game event
int Multi_update_fireup_launcher_on_exit = 0;

// error code string for convenient reporting
char Multi_update_error_string[512];

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE FUNCTIONS
//

// initialize the http xfer of the version info file, return 1 on success
int multi_update_http_init()
{
	char url_file[512] = "";
	char local_file[512] = "";

	// url
	strcpy(url_file, VERSION_URL);

	// local file
	strcpy(local_file, Cfile_root_dir);
	strcat(local_file, "\\");
	strcat(local_file, VERSION_LOC_FNAME);

	// new file	
	Multi_update_get = new InetGetFile(url_file, local_file);
	if(Multi_update_get == NULL){
		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));

		return 0;
	}

	return 1;
}

// do frame for the popup. returns 0 if not done yet, 1 if succeeded, 2 on error
int multi_update_http_do()
{	
	// sanity
	if(Multi_update_get == NULL){
		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));

		return 2;
	}

	// error
	if(Multi_update_get->IsFileError()){			
		delete Multi_update_get;
		Multi_update_get = NULL;

		// error string
		strcpy(Multi_update_error_string, XSTR("Could not get data from website", 977));
		
		return 2;
	} 	

	// done!
	if(Multi_update_get->IsFileReceived()){
		delete Multi_update_get;
		Multi_update_get = NULL;
		return 1;
	}
	
	// connecting, receiving
	if(Multi_update_get->IsConnecting() || Multi_update_get->IsReceiving()){
		return 0;
	}

	return 0;
}

// close down the http xfer
void multi_update_http_close()
{	
}

// error verifying, prompt the user for some action
int multi_update_error_verifying()
{
	char out_str[512];

	memset(out_str, 0, 512);
	strcpy(out_str, "(");
	strcat(out_str, Multi_update_error_string);
	strcat(out_str, ")\n\n");
	strcat(out_str, XSTR("There was an error verifying your version of Freespace, if you continue, you will not necessarily be up to date", 978));	

	switch(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Go back", 1524), XSTR("&Continue", 1525), out_str)){
	// continue on in freespace like nothing happened
	case 1:
	case -1:
		return MULTI_UPDATE_CONTINUE;

	// go back to the main menu
	default:
		return MULTI_UPDATE_MAIN_MENU;
	}

	// should never happen, but if so, move back to the main menu
	return MULTI_UPDATE_MAIN_MENU;
}

// check to see if the version of FS on this machine is not recent. run in a popup
// if the versions don't check out, bail to the launcher
// returns 0 if freespace should continue, 1 if freespace should go back to the main menu,
// and 2 if the "shutdown" event was posted
int multi_update_gobaby()
{			
	char msg[512] = "";
	int ret_code;
	int my_code = MULTI_UPDATE_MAIN_MENU;

	// maybe skip
	if(os_config_read_uint(NULL, "SkipVerify", 0)){
		return MULTI_UPDATE_CONTINUE;
	}

	memset(Multi_update_error_string, 0, 512);	

	// initialize the http connection
	if(!multi_update_http_init()){		
		return multi_update_error_verifying();
	}

	// run the popup
	extern char Multi_options_proxy[512];
	extern ushort Multi_options_proxy_port;
	if(strlen(Multi_options_proxy) > 0){
		sprintf(msg, "%s (%s : %d)", XSTR("Verifying Freespace Version",981), Multi_options_proxy, Multi_options_proxy_port);
	} else {
		strcpy(msg, XSTR("Verifying Freespace Version",981));
	}
	ret_code = popup_till_condition(multi_update_http_do, XSTR("Cancel",948), msg);		

	// determine what to do now
	switch(ret_code){
	// success
	case 1 : 
		// compare the versions
		switch(version_compare(VERSION_LOC_FNAME, NULL, NULL, NULL, NULL, NULL, NULL)){
		// error
		case -1:
			my_code = multi_update_error_verifying();
			break;

		// earlier version - need to update
		case 0:			
			switch(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, "&Update Later", "&Yes", XSTR("A new version of Freespace is available. You must update to the new version to play on PXO\n\nAuto update now?", 980))){
			// update later (go back to main hall for now
			case 0 :
			case -1:
				my_code = MULTI_UPDATE_MAIN_MENU;			
				break;

			default:
				// set things up so that the launcher is launched when Freespace is done closing
				Multi_update_fireup_launcher_on_exit = 1;
				gameseq_post_event(GS_EVENT_QUIT_GAME);

				my_code = MULTI_UPDATE_SHUTTING_DOWN;			
			}				
			break;

		// same version or higher
		case 1:
		case 2:
			my_code = MULTI_UPDATE_CONTINUE;
			break;
		}
		break;

	// error of some kind (should probably notify the user or something)
	case 2:	
		my_code = multi_update_error_verifying();		
		break;

	// this should never happen, but if it does, go back to the main menu
	default :
		my_code = multi_update_error_verifying();		
		break;
	}	

	// close down
	multi_update_http_close();
	
	return my_code;		
}

