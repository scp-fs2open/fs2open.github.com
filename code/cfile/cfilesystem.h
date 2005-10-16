/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CFile/CfileSystem.h $
 * $Revision: 2.4 $
 * $Date: 2005-10-16 23:15:46 $
 * $Author: wmcoolmon $
 *
 * Functions to keep track of and find files that can exist
 * on the harddrive, cd-rom, or in a pack file on either of those.
 * This keeps a list of all the files in packfiles or on CD-rom
 * and when you need a file you call one function which then searches
 * all those locations, inherently enforcing precedence orders.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.2  2004/08/11 05:06:19  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     10/13/98 9:19a Andsager
 * Add localization support to cfile.  Optional parameter with cfopen that
 * looks for localized files.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 5     5/01/98 10:21a John
 * Added code to find all pack files in all trees.   Added code to create
 * any directories that we write to.
 * 
 * 4     4/30/98 10:21p John
 * Added code to cleanup cfilesystem
 *
 * $NoKeywords: $
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
	char		*path;					// Path relative to Freespace root, has ending backslash.
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
