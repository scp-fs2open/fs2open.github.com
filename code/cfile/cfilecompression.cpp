#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>
#endif

#include "lz4.h"
#include "cfilecompression.h"
#include "cfilearchive.h"

/*INTERNAL FUNCTIONS*/
/*LZ41*/
void lz41_load_offsets(CFILE* cf);
size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length);
void lz41_create_ci(CFILE* cf, int header);
/*MISC*/
int fso_fseek(CFILE* cfile, int offset, int where);
/*END OF INTERNAL FUNCTIONS*/

int comp_check_header(int header)
{
	if (LZ41_FILE_HEADER == header)
		return COMP_HEADER_MATCH;

	return COMP_HEADER_MISMATCH;
}

void comp_create_ci(CFILE* cf, int header)
{
	mprintf(("(CI)Compressed File Opened: %s \n", cf->original_filename.c_str()));

	if (LZ41_FILE_HEADER == header)
		lz41_create_ci(cf, header);

	mprintf(("(CI)Uncompressed FileSize: %d \n", cf->size));
	mprintf(("(CI)Compressed FileSize: %d \n", cf->compression_info.compressed_size));
	mprintf(("(CI)Block Size: %d \n", cf->compression_info.block_size));
}

size_t comp_fread(CFILE* cf, char* buffer, size_t length)
{
	/* Check the request to be at least 1 byte */
	if (length <= 0)
	{
		mprintf(("\n(CI) File: %s . Invalid length requested: %d \n", cf->original_filename.c_str(), length));
		return COMP_INVALID_LENGTH_REQUESTED;
	}
	/* Check that we are not requesting to read beyond end of file */
	if (length + cf->raw_position > cf->size)
	{
		mprintf(("\n(CI) File: %s . Requested to read beyond end of file, Lenght requested: %d, Current Pos: %d, Filesize: d% \n", cf->original_filename.c_str(), length, cf->raw_position,cf->size));
		return COMP_INVALID_LENGTH_REQUESTED;
	}

	if (LZ41_FILE_HEADER == cf->compression_info.header)
		return lz41_stream_random_access(cf, buffer, cf->raw_position, length);

	return 0;
}

size_t comp_ftell(CFILE* cf)
{
	return cf->raw_position;
}

int comp_feof(CFILE* cf)
{
	int result = 0;
	if (cf->raw_position >= cf->size)
		result = 1;
	else
		result = 0;
	return result;
}

size_t comp_fseek(CFILE* cf, int offset, int where)
{
	size_t goal_position;
	switch (where) {
	case SEEK_SET: goal_position = offset; break;
	case SEEK_CUR: goal_position = cf->raw_position + offset; break;
	case SEEK_END: goal_position = cf->raw_position + offset; break;
	default: return 1;
	}
	cf->raw_position = goal_position;
	if (goal_position > cf->size || goal_position<0)
	{
		mprintf(("\n(CI) File: %s . Requested to seek beyond end of compressed file or invalid seek. Goal Pos: %d, Filesize: d% \n", cf->original_filename.c_str(), goal_position, cf->size));
		Assertion(goal_position <= cf->size || goal_position > 0, "Requested to seek beyond end of compressed file or invalid seek.");
	}
	return goal_position;
}

//Special fseek for compressed files handled by FSO, only SEEK_SET and SEEK_END is supported.
int fso_fseek(CFILE* cfile, int offset, int where)
{
	size_t goal_position;

	switch (where) {
	case SEEK_SET: goal_position = offset + cfile->lib_offset; break;
	case SEEK_END: goal_position = cfile->compression_info.compressed_size + offset + cfile->lib_offset; break;
	default: return 0;
	}
	
	// Make sure we don't seek beyond the end of the file
	CAP(goal_position, cfile->lib_offset, cfile->lib_offset + cfile->compression_info.compressed_size);
	
	int result = 0;
	if (cfile->fp)
	{
		result = fseek(cfile->fp, (long)goal_position, SEEK_SET);
		Assertion(goal_position >= cfile->lib_offset, "Invalid offset values detected while seeking! Goal was " SIZE_T_ARG ", lib_offset is " SIZE_T_ARG ".", goal_position, cfile->lib_offset);
	}
	return result;
}

void lz41_create_ci(CFILE* cf, int header)
{
	cf->compression_info.header = header;
	cf->compression_info.compressed_size = cf->size;
	fso_fseek(cf, -12, SEEK_END);
	fread(&cf->compression_info.numOffsets, 4, 1, cf->fp);
	fread(&cf->size, 4, 1, cf->fp);
	fread(&cf->compression_info.block_size,4, 1, cf->fp);

	#if !defined(NDEBUG)
	if (cf->compression_info.numOffsets <= 0 || cf->size <= 0 || cf->compression_info.block_size <= 0)
	{
		mprintf(("\n(CI) File: %s . Something went wrong reading the setup data, file is possibly in the wrong format. Num Offsets: %d, Block Size: %d, Filesize: d% \n", cf->original_filename.c_str(), cf->compression_info.numOffsets, cf->compression_info.block_size, cf->size));
		Assertion(cf->compression_info.numOffsets > 0, "Invalid number of offsets, compressed file is possibly in the wrong format.");
		Assertion(cf->size > 4, "Invalid filesize, compressed file is possibly in the wrong format.");
		Assertion(cf->compression_info.block_size, "Invalid block size, compressed file is possibly in the wrong format.");
	}
	#endif
	cf->compression_info.decoderBuffer = (char*)malloc(LZ4_compressBound(cf->compression_info.block_size));
	cf->compression_info.lastDecBlockNum = 0;
	cf->compression_info.lastDecBytes = 0;
	lz41_load_offsets(cf);
}

void lz41_load_offsets(CFILE* cf)
{
	int maxBlocks = (int)cf->size + 8 / cf->compression_info.block_size;
	cf->compression_info.offsets = (int*)malloc(maxBlocks);
	int block;
	int* offsetsPtr = cf->compression_info.offsets;

	fso_fseek(cf, (-4 * (cf->compression_info.numOffsets + 1)) - (sizeof(int)*2), SEEK_END);
	for (block = 0; block <= cf->compression_info.numOffsets; ++block)
		fread(offsetsPtr++, sizeof(int), 1, cf->fp);
}

size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length)
{
	LZ4_streamDecode_t lz4StreamDecode_body;
	LZ4_streamDecode_t* lz4StreamDecode = &lz4StreamDecode_body;
	/* The blocks (currentBlock to endBlock) contain the data we want */
	size_t currentBlock = offset / cf->compression_info.block_size;
	size_t endBlock = ((offset + length - 1) / cf->compression_info.block_size) + 1;
	int* offsetsPtr = cf->compression_info.offsets;
	int block = 0;
	size_t written_bytes = 0;

	if (cf->compression_info.numOffsets <= endBlock)
		return LZ41_OFFSETS_MISMATCH;

	/* Seek to the first block to read, if it matches the cached block, search the next one instead */
	if (cf->compression_info.lastDecBlockNum == cf->compression_info.offsets[currentBlock] && currentBlock + 1 <= endBlock)
		fso_fseek(cf, cf->compression_info.offsets[currentBlock+1], SEEK_SET);
	else
		fso_fseek(cf, cf->compression_info.offsets[currentBlock], SEEK_SET);
	
	offset = offset % cf->compression_info.block_size;

	/* Start decoding */
	for (; currentBlock < endBlock; ++currentBlock)
	{
		if (cf->compression_info.lastDecBlockNum != cf->compression_info.offsets[currentBlock])
		{
			char* cmpBuf = (char*)malloc(LZ4_compressBound(cf->compression_info.block_size));
			/* The difference in offsets is the size of the block */
			int cmpBytes = cf->compression_info.offsets[currentBlock + 1] - cf->compression_info.offsets[currentBlock];
			fread(cmpBuf, cmpBytes, 1, cf->fp);

			const int decBytes = LZ4_decompress_safe_continue(lz4StreamDecode, cmpBuf, cf->compression_info.decoderBuffer, cmpBytes, cf->compression_info.block_size);
			if (decBytes <= 0)
				return LZ41_DECOMPRESSION_ERROR;

			/* Write out the part of the data we care about */
			size_t blockLength = ((length) < ((decBytes - offset)) ? (length) : ((decBytes - offset)));
			memcpy(bytes_out + written_bytes, cf->compression_info.decoderBuffer + offset, blockLength);
			written_bytes += blockLength;
			offset = 0;
			length -= blockLength;
			free(cmpBuf);
			cf->compression_info.lastDecBlockNum = cf->compression_info.offsets[currentBlock];
			cf->compression_info.lastDecBytes = decBytes;
		}
		else
		{
			/* Write out the part of the data we care about from buffer*/
			size_t blockLength = ((length) < ((cf->compression_info.lastDecBytes - offset)) ? (length) : ((cf->compression_info.lastDecBytes - offset)));
			memcpy(bytes_out + written_bytes, cf->compression_info.decoderBuffer + offset, (size_t)blockLength);
			written_bytes += blockLength;
			offset = 0;
			length -= blockLength;
		}
	}
	return written_bytes;
}
