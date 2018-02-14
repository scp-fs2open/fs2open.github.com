/*
 * Def_Files.h
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */




#ifndef __DEF_FILES_H_
#define __DEF_FILES_H_

#include "globalincs/pstypes.h"

struct default_file
{
	const char* path_type;
	const char* filename;
	const void* data;
	size_t size;
};

//Used to retrieve pointer to file data from def_files.cpp
default_file defaults_get_file(const char *filename);

SCP_vector<default_file> defaults_get_all();

#endif
