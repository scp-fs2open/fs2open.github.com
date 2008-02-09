/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/version.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:57 $
 * $Author: penguin $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     8/09/99 2:21p Andsager
 * Fix patching from multiplayer direct to launcher update tab.
 * 
 * 3     8/06/99 3:32p Andsager
 * Handle update when no registry is set
 * 
 * 2     5/19/99 4:07p Dave
 * Moved versioning code into a nice isolated common place. Fixed up
 * updating code on the pxo screen. Fixed several stub problems.
 * 
 * 1     5/18/99 4:28p Dave
 * 
 * $NoKeywords: $
 */

#include <stdio.h>
#include <string.h>
#include "version.h"
#include "osregistry.h"

// ----------------------------------------------------------------------------------------------------------------
// VERSION DEFINES/VARS
//

// Defines
#define VER(major, minor, build) (100*100*major+100*minor+build)
#define MAX_LINE_LENGTH 512


// ----------------------------------------------------------------------------------------------------------------
// VERSION FUNCTIONS
//

// compare version against the passed version file
// returns -1 on error 
// 0 if we are an earlier version
// 1 if same version
// 2 if higher version
// fills in user version and latest version values if non-NULL
int version_compare(char *filename, int *u_major, int *u_minor, int *u_build, int *l_major, int *l_minor, int *l_build)
{	
	int usr_major, usr_minor, usr_build;
	int latest_major, latest_minor, latest_build;

	// open file and try backup, if needed
	FILE *f = fopen(filename, "rt");
	if (f == NULL) {
		return -1;		
	}

	// grab the last line in file which isn't empty and isn't a comment
	char buffer[MAX_LINE_LENGTH+1], verbuffer[MAX_LINE_LENGTH+1];

	strcpy(verbuffer,"");
	strcpy(buffer,"");
	while ( !feof(f) ) {
		// Read the line into a temporary buffer
		fgets(buffer, MAX_LINE_LENGTH, f);

		// take the \n off the end of it
		if (strlen(buffer)>0 && buffer[strlen(buffer) - 1] == '\n')
			buffer[strlen(buffer) - 1] = 0;

		// If the line is empty, go get another one
		if (strlen(buffer) == 0) continue;

		// If the line is a comment, go get another one
		if (buffer[0] == VERSION_FILE_COMMENT_CHAR) continue;

		// Line is a good one, so save it...
		strcpy(verbuffer, buffer);
	}
	fclose(f);

	// Make sure a version line was found
	if (strlen(verbuffer) == 0) {
		// MessageBox(XSTR("Couldn't parse Version file!", 1205), XSTR("Error!", 1185), MB_OK|MB_ICONERROR);
		return -1;
	}

	// Get the most up to date Version number
	latest_major = 0;
	latest_minor = 0;
	latest_build = 0;

	if (sscanf(verbuffer, "%i %i %i", &latest_major, &latest_minor, &latest_build) != 3) {
		// MessageBox(XSTR("Couldn't parse Version file!", 1205), XSTR("Error!", 1185), MB_OK|MB_ICONERROR);
		return -1;
	}

	// retrieve the user's current version
	usr_major = os_config_read_uint("Version", "Major", 0);
	usr_minor = os_config_read_uint("Version", "Minor", 0);
	usr_build = os_config_read_uint("Version", "Build", 0);
	
	// Make sure the user's Version was found!
	if ( VER(usr_major, usr_minor, usr_build) == 0 ) {
		// MessageBox(XSTR("The Freespace 2 Auto-Update program could not find your current game Version in the system registry.\n\nThis should be corrected by starting up the game, exiting the game, and then running the Auto-Update program.", 1206), XSTR("Unable to Determine User's Version", 1207), MB_OK|MB_ICONERROR);
		return NO_VERSION_IN_REGISTRY;
	}	

	// stuff outgoing values
	if(u_major != NULL){
		*u_major = usr_major;
	}
	if(u_minor != NULL){
		*u_minor = usr_minor;
	}
	if(u_build != NULL){
		*u_build = usr_build;
	}
	if(l_major != NULL){
		*l_major = latest_major;
	}
	if(l_minor != NULL){
		*l_minor = latest_minor;
	}
	if(l_build != NULL){
		*l_build = latest_build;
	}

	// check to see if the user's version is up to date
	if (VER(usr_major, usr_minor, usr_build) < VER(latest_major, latest_minor, latest_build)) {		
		return 0;
	}

	// same version
	return 1;
}
