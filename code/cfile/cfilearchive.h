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
	int header = 0;                     // compression magic, 0 = not compressed / not valid
	size_t compressed_size = 0;         // size of the compressed file (within the VP, excludes lib_offset)
	int block_size = 0;                 // uncompressed bytes per block
	int num_offsets = 0;                // number of entries in the offset table (num_blocks + 1)
	int* offsets = nullptr;             // start position (within the compressed file) of each block
	char* decoder_buffer = nullptr;		// holds the most recently decoded block (block_size bytes)
	int cached_block = -1;              // index of the block currently held in decoder_buffer: -1 = none
	int cached_block_bytes = 0;         // number of decoded bytes in decoder_buffer
	bool read_failed = false;
};

struct CFILE {
	int type = CFILE_BLOCK_UNUSED;                // CFILE_BLOCK_UNUSED, CFILE_BLOCK_USED
	int dir_type;        // directory location
	FILE* fp;                // File pointer if opening an individual file
	const void* data;            // Pointer for memory-mapped file access.  NULL if not mem-mapped.
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

// Detect compression and initialize the decoder. Returns false for invalid compressed files.
bool cf_check_compression(CFILE* cfile);

// Used to clear compression info data and free dynamic memory used by compression
void cf_clear_compression_info(CFILE* cfile);

#endif
