/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FS2_VERSIONING_HEADER_FILE
#define __FS2_VERSIONING_HEADER_FILE

// ----------------------------------------------------------------------------------------------------------------
// VERSION DEFINES/VARS
//

// Here are the version defines.  
//	Gets displayed as MAJOR.MINOR, or 1.21 if MAJOR = 1, MINOR = 21.
//	Prior to release, MAJOR should be zero.  After release, it should be 1.  Probably never increase to 2 as that could
//	cause confusion with a sequel.
//	MINOR should increase by 1 for each minor upgrade and by 10 for major upgrades.
//	With each rev we send, we should increase MINOR.
// Version history (full version):
//		1.0	Initial US/UK release
//		1.01	Patch for Win95 volume label bug
//		1.20	German release version


// fs2_open version numbers:
//   the first version is 3.0 :-)
//   Major.Minor.Bugfix

#define FS_VERSION_MAJOR 3					// major version
#define FS_VERSION_MINOR 7					// increase by 1 for minor revs
#define FS_VERSION_BUILD 1					// bugfix release
#define FS_VERSION_REVIS 0000				// SVN revision

#define VERSION_LOC_FNAME			"version.nfo"
#define MOTD_LOC_FNAME				"motd.txt"

#define MOTD_URL						"http://www.pxo.net/files/fs2/motd.txt"
#define VERSION_URL					"http://www.pxo.net/files/fs2/version.nfo"

#define VERSION_FILE_COMMENT_CHAR ';'
#define NO_VERSION_IN_REGISTRY		-2

// ----------------------------------------------------------------------------------------------------------------
// VERSION FUNCTIONS
//

// compare version against the passed version file
// returns -1 on error 
// 0 if we are an earlier version
// 1 if same version
// 2 if higher version
// fills in user version and latest version values if non-NULL
int version_compare(char *filename, int *u_major, int *u_minor, int *u_build, int *l_major, int *l_minor, int *l_build);

#endif
