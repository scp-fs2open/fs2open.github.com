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
#include "luaconf.h"

#include <sstream>
#include <limits>


#define CHECK_POSITION

// Called once to setup the low-level reading code.

void cf_init_lowlevel_read_code( CFILE * cfile, size_t lib_offset, size_t size, size_t pos )
{
	Assert(cfile != NULL);

	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	cb->lib_offset = lib_offset;
	cb->raw_position = pos;
	cb->size = size;

	if ( cb->fp )	{
		if ( cb->lib_offset )	{
			fseek( cb->fp, (long)cb->lib_offset, SEEK_SET );
		}

		#if defined(CHECK_POSITION) && !defined(NDEBUG)
		auto raw_position = ftell(cb->fp) - cb->lib_offset;
		Assert(raw_position == cb->raw_position);
		#endif
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

	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	int result = 0;

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cb->fp) {
		auto raw_position = ftell(cb->fp) - cb->lib_offset;
		Assert(raw_position == cb->raw_position);
	}
	#endif
		
	if (cb->raw_position >= cb->size ) {
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
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cb->fp) {
		auto raw_position = ftell(cb->fp) - cb->lib_offset;
		Assert(raw_position == cb->raw_position);
	}
	#endif

	// The rest of the code still uses ints, do an overflow check to detect cases where this fails
	Assertion(cb->raw_position <= static_cast<size_t>(std::numeric_limits<int>::max()),
		"Integer overflow in cftell, a file is probably too large (but I don't know which one).");
	return (int) cb->raw_position;
}


// cfseek() moves the file pointer
//
// returns:   success ==> 0
//            error   ==> non-zero
//
int cfseek( CFILE *cfile, int offset, int where )
{

	Assert(cfile != NULL);
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	


	// TODO: seek to offset in memory mapped file
	Assert( !cb->mem_mapped );
	
	size_t goal_position;

	switch( where )	{
	case CF_SEEK_SET:
		goal_position = offset+cb->lib_offset;
		break;
	case CF_SEEK_CUR:	
		{
			goal_position = cb->raw_position+offset+cb->lib_offset;
		}
		break;
	case CF_SEEK_END:
		goal_position = cb->size+offset+cb->lib_offset;
		break;
	default:
		Int3();
		return 1;
	}

	// Make sure we don't seek beyond the end of the file
	CAP(goal_position, cb->lib_offset, cb->lib_offset + cb->size);

	int result = 0;

	if (cb->fp) {
		// If we have a file pointer we can also seek in that file
		result = fseek(cb->fp, (long)goal_position, SEEK_SET );
		Assertion(goal_position >= cb->lib_offset, "Invalid offset values detected while seeking! Goal was " SIZE_T_ARG ", lib_offset is " SIZE_T_ARG ".", goal_position, cb->lib_offset);
	}
	// If we only have a data pointer this will do all the work
	cb->raw_position = goal_position - cb->lib_offset;
	Assertion(cb->raw_position <= cb->size, "Invalid raw_position value detected!");

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
	if (cb->fp) {
		auto tmp_offset = ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset == cb->raw_position);
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

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	if ( (cb->raw_position+size) > cb->size ) {
		Assertion(cb->raw_position <= cb->size, "Invalid raw_position value detected!");
		size = cb->size - cb->raw_position;
		if ( size < 1 ) {
			return 0;
		}
		//mprintf(( "CFILE: EOF encountered in file\n" ));
	}

	if (cb->max_read_len) {
		if ( cb->raw_position+size > cb->max_read_len ) {
			std::ostringstream s_buf;
			s_buf << "Attempted to read " << size << "-byte(s) beyond length limit";

			throw cfile::max_read_length(s_buf.str());
		}
	}

	size_t bytes_read;
	if (cb->data != nullptr) {
		// This is a file from memory
		bytes_read = size;
		memcpy(buf, reinterpret_cast<const char*>(cb->data) + cb->raw_position, size);
	} else {
		bytes_read = fread( buf, 1, size, cb->fp );
	}
	if ( bytes_read > 0 )	{
		cb->raw_position += bytes_read;
		Assertion(cb->raw_position <= cb->size, "Invalid raw_position value detected!");
	}		

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cb->fp) {
		auto tmp_offset = ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset==cb->raw_position);
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

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	// cfread() not supported for memory-mapped files
	if(cb->data != NULL)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return 0;
	}

	size_t advance = 0;
	int items_read;
	if (cb->fp) {
		long orig_pos = ftell(cb->fp);
		items_read = fscanf(cb->fp, LUA_NUMBER_SCAN, buf);
		advance = (size_t) (ftell(cb->fp)-orig_pos);
	} else {
		int read;
		// %n returns the number of bytes currently read so we append that to the scan format at the end so it will return
		// how many bytes we have consumed
		items_read = sscanf(reinterpret_cast<const char*>(cb->data), LUA_NUMBER_SCAN "%n", buf, &read);
		if (items_read == 2) {
			// We need to correct the items read counter since we read one additional item
			items_read = 1;
		}
		advance = (size_t) read;
	}
	cb->raw_position += advance;
	Assertion(cb->raw_position <= cb->size, "Invalid raw_position value detected!");

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cb->fp) {
		auto tmp_offset = ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset==cb->raw_position);
	}
	#endif

	return items_read;

}

