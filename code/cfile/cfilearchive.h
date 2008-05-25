/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CFile/CfileArchive.h $
 * $Revision: 2.3 $
 * $Date: 2005-07-13 02:50:49 $
 * $Author: Goober5000 $
 *
 * External def's for CfileArchive.cpp.  This should only be used 
 * internally by cfile stuff.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/08/11 05:06:19  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/17 02:56:19  mharris
 * first crack at unix compatibity
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 4     5/11/98 10:59a John
 * Moved the low-level file reading code into cfilearchive.cpp.
 * 
 * 3     4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 2     12/28/97 12:42p John
 * Put in support for reading archive files; Made missionload use the
 * cf_get_file_list function.   Moved demos directory out of data tree.
 * 
 * 1     12/28/97 11:48a John
 *
 * $NoKeywords: $
 */

#ifndef _CFILEARCHIVE_H
#define _CFILEARCHIVE_H

#ifndef _CFILE_INTERNAL 
#error This file should only be included internally in CFILE!!
#endif

// The following Cfile_block data is private to cfile.cpp
// DO NOT MOVE the Cfile_block* information to cfile.h / do not extern this data
//
#define CFILE_BLOCK_UNUSED		0
#define CFILE_BLOCK_USED		1

typedef struct Cfile_block {
	int		type;				// CFILE_BLOCK_UNUSED, CFILE_BLOCK_USED
	int		dir_type;		// directory location
	FILE		*fp;				// File pointer if opening an individual file
	void		*data;			// Pointer for memory-mapped file access.  NULL if not mem-mapped.
#ifdef _WIN32
	HANDLE	hInFile;			// Handle from CreateFile()
	HANDLE	hMapFile;		// Handle from CreateFileMapping()
#else
//	int		fd;				// file descriptor
	size_t	data_length;	// length of data for mmap
#endif
	int		lib_offset;
	int		raw_position;
	int		size;				// for packed files
	
} Cfile_block;

#define MAX_CFILE_BLOCKS	64
extern Cfile_block Cfile_block_list[MAX_CFILE_BLOCKS];
extern CFILE Cfile_list[MAX_CFILE_BLOCKS];

// Called once to setup the low-level reading code.
void cf_init_lowlevel_read_code( CFILE * cfile, int lib_offset, int size, int pos );

#endif
