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
#include "lz4/lz4.h"

struct compression_info {
	char header[4] = { 'N','O','C','P' };
	int isCompressed = 0;
	size_t uncompressed_position = 0;
	size_t uncompressed_size = 0;
	int maxBlocks = 0;
	int numOffsets = 0;
	int* offsets = nullptr;
};


// The following Cfile_block data is private to cfile.cpp
// DO NOT MOVE the Cfile_block* information to cfile.h / do not extern this data
//
#define CFILE_BLOCK_UNUSED		0
#define CFILE_BLOCK_USED		1

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
	compression_info compression_info;
};

#define MAX_CFILE_BLOCKS	64
extern std::array<CFILE, MAX_CFILE_BLOCKS> Cfile_block_list;

// Called once to setup the low-level reading code.
void cf_init_lowlevel_read_code(CFILE* cfile, size_t lib_offset, size_t size, size_t pos);

/*LZ41*/
#define LZ41_FILE_HEADER "LZ41"
#define LZ41_BLOCK_BYTES 8192
#define LZ41_DECOMPRESSION_ERROR -1
#define LZ41_MAX_BLOCKS_OVERFLOW -2
#define LZ41_HEADER_MISMATCH -3
#define LZ41_OFFSETS_MISMATCH -4
/******/

#define COMP_HEADER_MATCH 1
#define COMP_HEADER_MISMATCH 0

/*
	Returns COMP_HEADER_MATCH if header is a valid compressed file header,
	returns COMP_HEADER_MISMATCH if it dosent.
*/
int comp_check_header(char* header);

/*
	This is called to generate the correct compression_info data
	after the file has been indentified as a compressed file.
	This must be done before calling any other function.
*/
void comp_create_ci(CFILE* cf, char* header);

/*
	Read X bytes from the uncompressed file starting from X offset.
	Returns the amount of bytes read, and 0 or lower to indicate errors.
*/
int comp_fread(CFILE* cf, char* buffer, int offset, int length);

/*
	Returns the current uncompressed file position.
*/
int comp_ftell(CFILE* cf);

/*
	Returns 1 of the uncompressed file has been completely read, otherwise it returns a 0.
*/
int comp_feof(CFILE* cf);

/*
	Used to move the uncompressed file current position.
*/
int comp_fseek(CFILE* cf, int offset, int where);

#endif
