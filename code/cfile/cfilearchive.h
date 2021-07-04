/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _CFILEARCHIVE_H
#define _CFILEARCHIVE_H

#ifndef _CFILE_INTERNAL 
#pragma message("This file should only be included internally in CFILE!!")
#endif

#include "globalincs/pstypes.h"

// The following Cfile_block data is private to cfile.cpp
// DO NOT MOVE the Cfile_block* information to cfile.h / do not extern this data
//
#define CFILE_BLOCK_UNUSED		0
#define CFILE_BLOCK_USED		1

struct COMPRESSION_INFO {
	int header = 0;
	size_t compressed_size = 0;
	int block_size = 0;
	int num_offsets = 0;
	int* offsets = nullptr;
	char* decoder_buffer = nullptr;
	int last_decoded_block_pos = 0;
	int last_decoded_block_bytes = 0;
};

struct CFILE {
	int type = CFILE_BLOCK_UNUSED;                // CFILE_BLOCK_UNUSED, CFILE_BLOCK_USED
	int dir_type;        // directory location
	FILE* fp;                // File pointer if opening an individual file
	const void* data;            // Pointer for memory-mapped file access.  NULL if not mem-mapped.
	bool mem_mapped; // Flag for memory mapped files (if data is not null and this is false it means that it's an embedded file)
#ifdef _WIN32
	HANDLE	hInFile;			// Handle from CreateFile()
	HANDLE	hMapFile;		// Handle from CreateFileMapping()
#else
	size_t data_length;    // length of data for mmap
#endif
	size_t lib_offset;
	size_t raw_position;
	size_t size;                // for packed files

	size_t max_read_len;    // max read offset, for special error handling

	SCP_string original_filename;
	const char* source_file;
	int line_num;
	COMPRESSION_INFO compression_info;
};

#define MAX_CFILE_BLOCKS	64
extern std::array<CFILE, MAX_CFILE_BLOCKS> Cfile_block_list;

// Called once to setup the low-level reading code.
void cf_init_lowlevel_read_code(CFILE* cfile, size_t lib_offset, size_t size, size_t pos);

// This checks if the file is compressed or not, and creates the proper compression info if so.
void cf_check_compression(CFILE* cfile);

// Used to clear compression info data and free dynamic memory used by compression
void cf_clear_compression_info(CFILE* cfile);

#endif
