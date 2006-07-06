/*
 * Def_Files.h
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/globalincs/def_files.h $
 * $Revision: 2.4 $
 * $Date: 2006-07-06 21:23:20 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 */


#ifndef __DEF_FILES_H_
#define __DEF_FILES_H_

//Used to retrieve pointer to file data from def_files.cpp
char *defaults_get_file(char *filename);

//WMC - 
//There are three parts to adding a file
//:PART 1: Add variable declaration for new file
//:PART 2: Add filename of default file to Default_files[] array, along with content variable
//:PART 3: Define the content using the variable declared in part 1.
//Do a search in def_files.cpp for the individual part labels for examples and locations.

#endif
