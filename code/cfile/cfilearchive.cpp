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


/*INTERNAL FUNCTIONS*/
/*LZ41*/
void lz41_load_offsets(CFILE* cf, compression_info* ci);
int lz41_stream_random_access(CFILE* cf, char* bytes_out, int offset, int length);
compression_info lz41_create_ci(CFILE* cf, char* header);
int fso_fseek(CFILE* cfile, int offset, int where);
/*END OF INTERNAL FUNCTIONS*/


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



// cfeof() Tests for end-of-file on a stream
//
// returns a nonzero value after the first read operation that attempts to read
// past the end of the file. It returns 0 if the current position is not end of file.
// There is no error return.

int cfeof(CFILE *cfile)
{
	Assert(cfile != NULL);

	if (cfile->compression_info.isCompressed == 1)
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

	if (cfile->compression_info.isCompressed == 1)
		return comp_ftell(cfile);

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

	if (cfile->compression_info.isCompressed == 1)
		return comp_fseek(cfile,offset,where);

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
	}else if (cfile->compression_info.isCompressed==1){
		bytes_read = size;
		int result = comp_fread(cfile, reinterpret_cast<char*>(buf),cfile->compression_info.uncompressed_position,size);
		mprintf(("Continuing reading CP file: %s \n", cfile->original_filename.c_str()));
		mprintf(("Size requested: %d \n", size));
		mprintf(("(CI)Result: %d \n", result));
		mprintf(("(CI)CP post: %d \n", cfile->compression_info.uncompressed_position));
	}else{
		if (cfile->raw_position == 0)
		{
			bytes_read = fread(buf, 1, size, cfile->fp);
			if (comp_check_header((char*)buf) == COMP_HEADER_MATCH)
			{
				mprintf(("Compressed File requested: %s \n", cfile->original_filename.c_str()));
				mprintf(("Size requested: %d \n", size));
				comp_create_ci(cfile,(char*)buf);
				int result = comp_fread(cfile, (char*)buf, cfile->compression_info.uncompressed_position, size);
				mprintf(("(CI)Result: %d \n", result));
				mprintf(("(CI)NumOffsets: %d \n", cfile->compression_info.numOffsets));
				mprintf(("(CI)FileSize: %d \n", cfile->compression_info.uncompressed_size));
				mprintf(("(CI)Buffer read: %s \n", buf));
			}
		}
		else
		{
			bytes_read = fread(buf, 1, size, cfile->fp);
		}
	}
	if ( bytes_read > 0 )	{
		cfile->raw_position += bytes_read;
		Assertion(cfile->raw_position <= cfile->size, "Invalid raw_position value detected!");
	}		

	#if defined(CHECK_POSITION) && !defined(NDEBUG)
    if (cfile->fp) {
		auto tmp_offset = ftell(cfile->fp) - cfile->lib_offset;
		//Assert(tmp_offset==cfile->raw_position);
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

int comp_check_header(char* header)
{
	if (memcmp(LZ41_FILE_HEADER, header, 4) == 0)
		return COMP_HEADER_MATCH;

	return COMP_HEADER_MISMATCH;
}

void comp_create_ci(CFILE* cf, char* header)
{
	if (memcmp(LZ41_FILE_HEADER, header, 4) == 0)
		cf->compression_info = lz41_create_ci(cf, header);
}

int comp_fread(CFILE* cf, char* buffer, int offset, int length)
{
	if (memcmp(LZ41_FILE_HEADER, cf->compression_info.header, 4) == 0)
		return lz41_stream_random_access(cf, buffer, offset, length);

	return 0;
}

int comp_ftell(CFILE* cf)
{
	mprintf(("FTELL HERE\n"));
	if (cf->compression_info.isCompressed == 1)
		return (int)cf->compression_info.uncompressed_position;
	else
		return (int)cf->raw_position;
}

int comp_feof(CFILE* cf)
{
	int result = 0;
	mprintf(("FEOF HERE\n"));
	if (cf->compression_info.isCompressed == 1)
	{
		if (cf->compression_info.uncompressed_position >= cf->compression_info.uncompressed_size)
			result = 1;
		else
			result = 0;
	}
	else
	{
		if (cf->raw_position >= cf->size)
			result = 1;
		else
			result = 0;
	}
	return result;
}

int comp_fseek(CFILE* cf, int offset, int where)
{
	mprintf(("FSEEK HERE\n"));
	if (cf->compression_info.isCompressed == 1)
	{
		size_t goal_position;
		switch (where) {
		case SEEK_SET: goal_position = offset; break;
		case SEEK_CUR: goal_position = cf->compression_info.uncompressed_position + offset; break;
		case SEEK_END: goal_position = cf->compression_info.uncompressed_size + offset; break;
		default: return 1;
		}
		cf->compression_info.uncompressed_position = goal_position;
		return goal_position;
	}
	else
	{
		return fso_fseek(cf, offset, where);
	}
}

int fso_fseek(CFILE* cfile, int offset, int where)
{
	size_t goal_position;

	switch (where) {
	case SEEK_SET: goal_position = offset + cfile->lib_offset; break;
	case SEEK_CUR: goal_position = cfile->raw_position + offset + cfile->lib_offset; break;
	case SEEK_END: goal_position = cfile->size + offset + cfile->lib_offset; break;
	default: return 1;
	}

	int result = 0;

	if (cfile->fp)
		result = fseek(cfile->fp, (long)goal_position, SEEK_SET);

	cfile->raw_position = goal_position - cfile->lib_offset;

	return result;
}

compression_info lz41_create_ci(CFILE* cf, char* header)
{
	compression_info ci;
	cf->raw_position = 4;
	ci.isCompressed = 1;
	memcpy(ci.header, header, 4);
	ci.uncompressed_position = 0;
	ci.maxBlocks = cf->size + 8 / LZ41_BLOCK_BYTES;
	fso_fseek(cf, -8, SEEK_END);
	fread(&ci.numOffsets, sizeof(int), 1, cf->fp);
	fread(&ci.uncompressed_size, sizeof(int), 1, cf->fp);
	lz41_load_offsets(cf, &ci);
	return ci;
}

void lz41_load_offsets(CFILE* cf, compression_info* ci)
{
	ci->offsets = (int*)malloc(ci->maxBlocks);
	int block;
	int* offsetsPtr = ci->offsets;

	fso_fseek(cf, (-4 * (ci->numOffsets + 1)) - sizeof(int), SEEK_END);
	for (block = 0; block <= ci->numOffsets; ++block)
		fread(offsetsPtr++, sizeof(int), 1, cf->fp);
}

int lz41_stream_random_access(CFILE* cf, char* bytes_out, int offset, int length)
{
	if (length == 0)
		return 0;

	if (offset >= cf->compression_info.uncompressed_size)
		return 0;

	LZ4_streamDecode_t lz4StreamDecode_body;
	LZ4_streamDecode_t* lz4StreamDecode = &lz4StreamDecode_body;
	/* The blocks [currentBlock, endBlock) contain the data we want */
	int currentBlock = offset / LZ41_BLOCK_BYTES;
	int endBlock = ((offset + length - 1) / LZ41_BLOCK_BYTES) + 1;
	int* offsets = cf->compression_info.offsets, * offsetsPtr = offsets;
	int block = 0, written_bytes = 0;
	char decBuf[LZ41_BLOCK_BYTES];

	if (cf->compression_info.numOffsets <= endBlock)
		return LZ41_OFFSETS_MISMATCH;

	/* Seek to the first block to read */
	fso_fseek(cf, offsets[currentBlock], SEEK_SET);
	offset = offset % LZ41_BLOCK_BYTES;
	cf->raw_position = offsets[currentBlock];

	/* Start decoding */
	for (; currentBlock < endBlock; ++currentBlock)
	{
		char cmpBuf[LZ4_COMPRESSBOUND(LZ41_BLOCK_BYTES)];
		/* The difference in offsets is the size of the block */
		int cmpBytes = offsets[currentBlock + 1] - offsets[currentBlock];
		fread(cmpBuf, cmpBytes, 1, cf->fp);
		cf->raw_position += (size_t)cmpBytes;

		const int decBytes = LZ4_decompress_safe_continue(lz4StreamDecode, cmpBuf, decBuf, cmpBytes, LZ41_BLOCK_BYTES);
		if (decBytes <= 0)
			return LZ41_DECOMPRESSION_ERROR;

		/* Write out the part of the data we care about */
		int blockLength = ((length) < ((decBytes - offset)) ? (length) : ((decBytes - offset)));
		memcpy(bytes_out + written_bytes, decBuf + offset, blockLength);
		written_bytes += blockLength;
		offset = 0;
		length -= blockLength;
	}

	cf->compression_info.uncompressed_position += written_bytes;

	if (cf->compression_info.uncompressed_position == cf->compression_info.uncompressed_size)
		cf->raw_position = cf->size;

	return written_bytes;
}
