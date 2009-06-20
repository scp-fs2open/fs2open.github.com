/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _CFILESYSTEM_H
#define _CFILESYSTEM_H

#include "cfile/cfile.h"

// Builds a list of all the files
void cf_build_secondary_filelist( char *cdrom_path );
void cf_free_secondary_filelist();

// Internal stuff
typedef struct cf_pathtype {
	int		index;					// To verify that the CF_TYPE define is correctly indexed into this array
	char		*path;					// Path relative to FreeSpace root, has ending backslash.
	char		*extensions;			// Extensions used in this pathtype, separated by spaces
	int		parent_index;			// Index of this directory's parent.  Used for creating directories when writing.
} cf_pathtype;

// During cfile_init, verify that Pathtypes[n].index == n for each item
extern cf_pathtype Pathtypes[CF_MAX_PATH_TYPES];

// Returns the default storage path for files given a 
// particular pathtype.   In other words, the path to 
// the unpacked, non-cd'd, stored on hard drive path.
// If filename isn't null it will also tack the filename
// on the end, creating a completely valid filename.
// Input:   pathtype  - CF_TYPE_??
//			path_max  - Maximum characters in the path
//          filename  - optional, if set, tacks the filename onto end of path.
// Output:  path      - Fully qualified pathname.
//Returns 0 if result would be too long (invalid result)
int cf_create_default_path_string( char *path, uint path_max, int pathtype, char *filename=NULL, bool localize = false);


#endif	//_CFILESYSTEM_H
