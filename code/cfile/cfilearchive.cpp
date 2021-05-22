/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>		/* needed for memory mapping of file functions */
#endif

#include "cfile/cfile.h"
#include "cfile/cfilearchive.h"
#include "cfile/cfilecompression.h"
#include "luaconf.h"

#include <sstream>
#include <limits>


#define CHECK_POSITION

// Called once to setup the low-level reading code.
void cf_init_lowlevel_read_code( CFILE * cfile, size_t lib_offset, size_t size, size_t pos )
{
	Assert(cfile != nullptr);

	cfile->lib_offset = lib_offset;
	cfile->raw_position = pos;
	cfile->size = size;

	if ( cfile->fp )	{
		if ( cfile->lib_offset )	{
			fseek( cfile->fp, (long)cfile->lib_offset, SEEK_SET );
		}

		#if defined(CHECK_POSITION) && !defined(NDEBUG)
		auto raw_position = ftell(cfile->fp) - cfile->lib_offset;
		Assert(raw_position == cfile->raw_position);
		#endif
	}
}

//This function checks the file header to see if the file is compressed, if so it fills the correct compression info.
void cf_check_compression(CFILE* cfile)
{
	if (cfile->size <= 16)
		return;
	int header=cfread_int(cfile);
	cfseek(cfile, 0, SEEK_SET);
	if (comp_check_header(header) == COMP_HEADER_MATCH)
		comp_create_ci(cfile, header);
}

//This function is called to cleanup compression info when the cfile handle is reused.
void cf_clear_compression_info(CFILE* cfile)
{
	if (cfile->compression_info.header != 0)
	{
		free(cfile->compression_info.offsets);
		free(cfile->compression_info.decoder_buffer);
		cfile->compression_info.offsets = nullptr;
		cfile->compression_info.decoder_buffer = nullptr;
		cfile->compression_info.header = 0;
		cfile->compression_info.block_size = 0;
		cfile->compression_info.last_decoded_block_pos = 0;
		cfile->compression_info.last_decoded_block_bytes = 0;
		cfile->compression_info.num_offsets = 0;
	}
}


// cfeof() Tests for end-of-file on a stream
//
// returns a nonzero value after the first read operation that attempts to read
// past the end of the file. It returns 0 if the current position is not end of file.
// There is no error return.
int cfeof(CFILE *cfile)
{
	Assert(cfile != NULL);
	if (cfile->compression_info.header != 0)
		return comp_feof(cfile);
	int result = 0;

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cfile->fp) {
		auto raw_position = ftell(cfile->fp) - cfile->lib_offset;
		Assert(raw_position == cfile->raw_position);
	}
	#endif
		
	if (cfile->raw_position >= cfile->size ) {
		result = 1;
	} else {
		result = 0;
	}

	return result;	
}

// cftell() returns offset into file
//
// returns:  success ==> offset into the file
//           error   ==> -1
//
int cftell( CFILE * cfile )
{
	Assert(cfile != NULL);
	if (cfile->compression_info.header != 0)
		return (int)comp_ftell(cfile);

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cfile->fp) {
		auto raw_position = ftell(cfile->fp) - cfile->lib_offset;
		Assert(raw_position == cfile->raw_position);
	}
	#endif

	// The rest of the code still uses ints, do an overflow check to detect cases where this fails
	Assertion(cfile->raw_position <= static_cast<size_t>(std::numeric_limits<int>::max()),
		"Integer overflow in cftell, a file is probably too large (but I don't know which one).");
	return (int) cfile->raw_position;
}


// cfseek() moves the file pointer
//
// returns:   success ==> 0
//            error   ==> non-zero
//
int cfseek( CFILE *cfile, int offset, int where )
{

	Assert(cfile != NULL);

	if (cfile->compression_info.header != 0)
		return comp_fseek(cfile, offset, where);

	// TODO: seek to offset in memory mapped file
	Assert( !cfile->mem_mapped );
	
	size_t goal_position;

	switch( where )	{
	case CF_SEEK_SET:
		goal_position = offset+cfile->lib_offset;
		break;
	case CF_SEEK_CUR:	
		{
			goal_position = cfile->raw_position+offset+cfile->lib_offset;
		}
		break;
	case CF_SEEK_END:
		goal_position = cfile->size+offset+cfile->lib_offset;
		break;
	default:
		Int3();
		return 1;
	}

	// Make sure we don't seek beyond the end of the file
	CAP(goal_position, cfile->lib_offset, cfile->lib_offset + cfile->size);

	int result = 0;

	if (cfile->fp) {
		// If we have a file pointer we can also seek in that file
		result = fseek(cfile->fp, (long)goal_position, SEEK_SET );
		Assertion(goal_position >= cfile->lib_offset, "Invalid offset values detected while seeking! Goal was " SIZE_T_ARG ", lib_offset is " SIZE_T_ARG ".", goal_position, cfile->lib_offset);
	}
	// If we only have a data pointer this will do all the work
	cfile->raw_position = goal_position - cfile->lib_offset;
	Assertion(cfile->raw_position <= cfile->size, "Invalid raw_position value detected!");

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
	if (cfile->fp) {
		auto tmp_offset = ftell(cfile->fp) - cfile->lib_offset;
		Assert(tmp_offset == cfile->raw_position);
	}
	#endif

	return result;	
}


// cfread() reads from a file
//
// returns:   returns the number of full elements read
//            
//
int cfread(void *buf, int elsize, int nelem, CFILE *cfile)
{
	if(!cf_is_valid(cfile))
		return 0;

	size_t size = elsize*nelem;

	if(buf == NULL || size <= 0)
		return 0;

	if ( (cfile->raw_position+size) > cfile->size ) {
		Assertion(cfile->raw_position <= cfile->size, "Invalid raw_position value detected!");
		size = cfile->size - cfile->raw_position;
		if ( size < 1 ) {
			return 0;
		}
		//mprintf(( "CFILE: EOF encountered in file\n" ));
	}

	if (cfile->max_read_len) {
		if ( cfile->raw_position+size > cfile->max_read_len ) {
			std::ostringstream s_buf;
			s_buf << "Attempted to read " << size << "-byte(s) beyond length limit";

			throw cfile::max_read_length(s_buf.str());
		}
	}

	size_t bytes_read;
	if (cfile->data != nullptr) {
		// This is a file from memory
		bytes_read = size;
		memcpy(buf, reinterpret_cast<const char*>(cfile->data) + cfile->raw_position, size);
	} else if (cfile->compression_info.header != 0) {
		bytes_read = comp_fread(cfile, reinterpret_cast<char*>(buf),size);
		if (bytes_read != size)
		{
			mprintf(("\nFile: %s decompression error. Result was: %d expected: %d\n", cfile->original_filename.c_str(), (int)bytes_read, (int)size));
			Assertion(bytes_read == size, "File decompression error!");
		}
	} else {
		bytes_read = fread(buf, 1, size, cfile->fp);
	}

	if ( bytes_read > 0 )	{
		cfile->raw_position += bytes_read;
		Assertion(cfile->raw_position <= cfile->size, "Invalid raw_position value detected!");
	}		

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
	//Raw position is not the fp position with compressed files
    if (cfile->fp && cfile->compression_info.header == 0) {
		auto tmp_offset = ftell(cfile->fp) - cfile->lib_offset;
		Assert(tmp_offset == cfile->raw_position);
	}
	#endif

	return (int)(bytes_read / elsize);
}

int cfread_lua_number(double *buf, CFILE *cfile)
{
	if(!cf_is_valid(cfile))
		return 0;

	if(buf == NULL)
		return 0;

	// cfread() not supported for memory-mapped files
	if(cfile->data != nullptr)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return 0;
	}

	size_t advance = 0;
	int items_read;
	if (cfile->fp) {
		long orig_pos = ftell(cfile->fp);
		items_read = fscanf(cfile->fp, LUA_NUMBER_SCAN, buf);
		advance = (size_t) (ftell(cfile->fp)-orig_pos);
	} else {
		int read;
		// %n returns the number of bytes currently read so we append that to the scan format at the end so it will return
		// how many bytes we have consumed
		items_read = sscanf(reinterpret_cast<const char*>(cfile->data), LUA_NUMBER_SCAN "%n", buf, &read);
		if (items_read == 2) {
			// We need to correct the items read counter since we read one additional item
			items_read = 1;
		}
		advance = (size_t) read;
	}
	cfile->raw_position += advance;
	Assertion(cfile->raw_position <= cfile->size, "Invalid raw_position value detected!");

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cfile->fp) {
		auto tmp_offset = ftell(cfile->fp) - cfile->lib_offset;
		Assert(tmp_offset==cfile->raw_position);
	}
	#endif

	return items_read;

}
