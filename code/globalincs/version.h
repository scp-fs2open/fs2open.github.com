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

#include "globalincs/vmallocator.h"

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
#define FS_VERSION_BUILD 3					// bugfix release
#define FS_VERSION_REVIS 000000				// SVN revision
//#define FS_VERSION_IDENT NOX("custom")	// special build release identifier, must be a string (don't define unless it's supposed to be used!!)

namespace version
{
	/**
	 * @brief Checks if the current version is at least the given version
	 * 
	 * @param major The major version to check
	 * @param minor The minor version to check
	 * @param build The build version to check
	 * @param revision The revision version to check
	 *
	 * @returns @c true when we are at least the given version, @c false otherwise
	 */
	bool check_at_least(int major, int minor, int build, int revision);
	
	/**
	 * @brief Returns the string representation of the passed version
	 * @param major The major version to format
	 * @param minor The minor version to format
	 * @param build The build version to format
	 * @param revision The revision version to format
	 * @returns A string representation of the version number
	 */
	SCP_string format_version(int major, int minor, int build, int revision);
}

#endif
