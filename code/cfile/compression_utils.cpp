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

#include "lz4/lz4.h"
#include "compression_utils.h"
#include "cfilearchive.h"

/*INTERNAL FUNCTIONS*/
/*LZ41*/
void lz41_load_offsets(CFILE* cf);
size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length);
void lz41_create_ci(CFILE* cf, char* header);
/*MISC*/
int fso_fseek(CFILE* cfile, int offset, int where);
/*END OF INTERNAL FUNCTIONS*/

void comp_create_ci(CFILE* cf, char* header)
{
	if (memcmp(LZ41_FILE_HEADER, header, 4) == 0)
		lz41_create_ci(cf, header);
}

size_t comp_fread(CFILE* cf, char* buffer, size_t length)
{
	if (memcmp(LZ41_FILE_HEADER, cf->compression_info.header, 4) == 0)
		return lz41_stream_random_access(cf, buffer, cf->raw_position, length);

	return 0;
}

size_t comp_ftell(CFILE* cf)
{;
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
	int result = 0;

	if (cfile->fp)
		result = fseek(cfile->fp, (long)goal_position, SEEK_SET);

	return result;
}

void lz41_create_ci(CFILE* cf, char* header)
{
	cf->compression_info.isCompressed = 1;
	memcpy(cf->compression_info.header, header, 4);
	cf->compression_info.compressed_size = cf->size;
	fso_fseek(cf, -12, SEEK_END);
	fread(&cf->compression_info.numOffsets, 4, 1, cf->fp);
	fread(&cf->size, 4, 1, cf->fp);
	fread(&cf->compression_info.block_size,4, 1, cf->fp);
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
	if (length == 0)
		return 0;

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
	if (cf->compression_info.lastDecBlockNum == cf->compression_info.offsets[currentBlock])
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

int comp_check_header(char* header)
{
	if (memcmp(LZ41_FILE_HEADER, header, 4) == 0)
		return COMP_HEADER_MATCH;

	return COMP_HEADER_MISMATCH;
}
