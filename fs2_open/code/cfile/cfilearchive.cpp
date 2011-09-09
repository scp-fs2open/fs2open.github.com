/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#define _CFILE_INTERNAL 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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


#define CHECK_POSITION

// Called once to setup the low-level reading code.

void cf_init_lowlevel_read_code( CFILE * cfile, int lib_offset, int size, int pos )
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
			fseek( cb->fp, cb->lib_offset, SEEK_SET );
		}

		#if defined(CHECK_POSITION) && !defined(NDEBUG)
			int raw_position;
			raw_position = ftell(cb->fp) - cb->lib_offset;
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

	int result;

	result = 0;

	// cfeof() not supported for memory-mapped files
	Assert( !cb->data );

	Assert(cb->fp != NULL);

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
		int raw_position;
		raw_position = ftell(cb->fp) - cb->lib_offset;
		Assert(raw_position == cb->raw_position);
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

	// Doesn't work for memory mapped files
	Assert( !cb->data );

	Assert(cb->fp != NULL);

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
		int raw_position;
		raw_position = ftell(cb->fp) - cb->lib_offset;
		Assert(raw_position == cb->raw_position);
	#endif

	return cb->raw_position;
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
	Assert( !cb->data );
	Assert( cb->fp != NULL );
	
	int goal_position;

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

	int result = fseek(cb->fp, goal_position, SEEK_SET );
	cb->raw_position = goal_position - cb->lib_offset;

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
		int tmp_offset;
		tmp_offset = (int)ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset==cb->raw_position);
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

	int size = elsize*nelem;

	if(buf == NULL || size <= 0)
		return 0;

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	// cfread() not supported for memory-mapped files
	if(cb->data != NULL)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return 0;
	}

	if ( (cb->raw_position+size) > cb->size ) {
		size = cb->size - cb->raw_position;
		if ( size < 1 ) {
			return 0;
		}
		//mprintf(( "CFILE: EOF encountered in file\n" ));
	}

	if (cb->max_read_len) {
		if ( (size_t)(cb->raw_position+size) > cb->max_read_len ) {
			std::ostringstream s_buf;
			s_buf << "Attempted to read " << size << "-byte(s) beyond length limit";

			throw cfile::max_read_length(s_buf.str());
		}
	}

	int bytes_read = fread( buf, 1, size, cb->fp );
	if ( bytes_read > 0 )	{
		cb->raw_position += bytes_read;
	}		

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
		int tmp_offset;
		tmp_offset = ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset==cb->raw_position);
	#endif

	return bytes_read / elsize;

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

	long orig_pos = ftell(cb->fp);
	int items_read = fscanf(cb->fp, LUA_NUMBER_SCAN, buf);
	cb->raw_position += ftell(cb->fp)-orig_pos;		

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
		int tmp_offset;
		tmp_offset = ftell(cb->fp) - cb->lib_offset;
		Assert(tmp_offset==cb->raw_position);
	#endif

	return items_read;

}

