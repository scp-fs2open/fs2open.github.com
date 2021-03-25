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
	//mprintf(("(CI)Compressed File Opened: %s \n", cf->original_filename.c_str()));

	if (LZ41_FILE_HEADER == header)
		lz41_create_ci(cf, header);

	//mprintf(("(CI)Uncompressed FileSize: %d \n", cf->size));
	//mprintf(("(CI)Compressed FileSize: %d \n", cf->compression_info.compressed_size));
	//mprintf(("(CI)Block Size: %d \n", cf->compression_info.block_size));
}

size_t comp_fread(CFILE* cf, char* buffer, size_t length)
{
	/* Check the request to be at least 1 byte */
	Assertion(length > 0, "Invalid length requested.");

	/* Check that we are not requesting to read beyond end of file */
	Assertion(cf->raw_position + length <= cf->size, "Invalid length requested.");

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
	if (cf->raw_position >= cf->size)
		return 1;
	else
		return 0;
}

int comp_fseek(CFILE* cf, int offset, int where)
{
	size_t goal_position;
	switch (where) {
	case SEEK_SET: goal_position = offset; break;
	case SEEK_CUR: goal_position = cf->raw_position + offset; break;
	case SEEK_END: goal_position = cf->size + offset; break;
	default: return 1;
	}

	// Make sure we don't seek beyond the end of the file
	CAP(goal_position,(size_t)0, cf->size);

	cf->raw_position = goal_position;

	return 0;
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
	fso_fseek(cf, (int)sizeof(int)*-3, SEEK_END);
	auto fNumoffsets = fread(&cf->compression_info.num_offsets, sizeof(int), 1, cf->fp);
	auto fSize = fread(&cf->size, sizeof(int), 1, cf->fp);
	auto fBsize = fread(&cf->compression_info.block_size, sizeof(int), 1, cf->fp);

	Assertion(cf->compression_info.num_offsets > 0, "Invalid number of offsets, compressed file is possibly in the wrong format or corrupted.");
	#if !defined(NDEBUG)
	Assertion(cf->size > 4, "Invalid filesize, compressed file is possibly in the wrong format or corrupted.");
	Assertion(cf->compression_info.block_size > 16, "Invalid block size, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fNumoffsets == 1, "Error while reading the number of offsets, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fSize == 1, "Error while reading original filesize, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fBsize == 1, "Error while reading block size, compressed file is possibly in the wrong format or corrupted.");
	#endif

	cf->compression_info.decoder_buffer = (char*)malloc(LZ4_compressBound(cf->compression_info.block_size));
	cf->compression_info.last_decoded_block_pos = 0;
	cf->compression_info.last_decoded_block_bytes = 0;
	lz41_load_offsets(cf);
}

void lz41_load_offsets(CFILE* cf)
{
	cf->compression_info.offsets = (int*)malloc(cf->compression_info.num_offsets * sizeof(int));
	int block;
	int* offsets_ptr = cf->compression_info.offsets;

	/* Seek to the first offset position, remember to consider the trailing ints */
	fso_fseek(cf, ( ( sizeof(int) * cf->compression_info.num_offsets ) * -1 ) - (sizeof(int)*3 ), SEEK_END);
	for (block = 0; block < cf->compression_info.num_offsets; ++block)
	{
		auto bytes_read = fread(offsets_ptr++, sizeof(int), 1, cf->fp);
		Assertion(bytes_read == 1, "Error reading offset list.");
	}
}

size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length)
{
	LZ4_streamDecode_t lz4_stream_decode_body;
	LZ4_streamDecode_t* lz4_stream_decode = &lz4_stream_decode_body;
	/* The blocks (current_block to end_block) contain the data we want */
	size_t current_block = offset / cf->compression_info.block_size;
	size_t end_block = ((offset + length - 1) / cf->compression_info.block_size) + 1;
	size_t written_bytes = 0;

	if (cf->compression_info.num_offsets <= (int)end_block)
		return (size_t)LZ41_OFFSETS_MISMATCH;

	/* Seek to the first block to read, if it matches the cached block, search the next one instead */
	if (cf->compression_info.last_decoded_block_pos == cf->compression_info.offsets[current_block] && current_block + 1 <= end_block)
		fso_fseek(cf, cf->compression_info.offsets[current_block+1], SEEK_SET);
	else
		fso_fseek(cf, cf->compression_info.offsets[current_block], SEEK_SET);
	
	offset = offset % cf->compression_info.block_size;
	char* cmp_buf = (char*)malloc(LZ4_compressBound(cf->compression_info.block_size));

	/* Start decoding */
	for (; current_block < end_block; ++current_block)
	{
		/* Only read and decode if the requested block is not the cached one */
		if (cf->compression_info.last_decoded_block_pos != cf->compression_info.offsets[current_block])
		{
			cf->compression_info.last_decoded_block_pos = cf->compression_info.offsets[current_block];

			/* The difference in offsets is the size of the block */
			int cmp_bytes = cf->compression_info.offsets[current_block + 1] - cf->compression_info.offsets[current_block];
			auto bytes_read = fread(cmp_buf, cmp_bytes, 1, cf->fp);
			Assertion(bytes_read == 1, "Error reading from compressed file.");

			cf->compression_info.last_decoded_block_bytes = LZ4_decompress_safe_continue(lz4_stream_decode, cmp_buf, cf->compression_info.decoder_buffer, cmp_bytes, cf->compression_info.block_size);
			if (cf->compression_info.last_decoded_block_bytes <= 0)
			{
				free(cmp_buf);
				return (size_t)LZ41_DECOMPRESSION_ERROR;
			}
		}

		/* Write out the part of the data we care about from buffer */
		size_t block_length = (length < (cf->compression_info.last_decoded_block_bytes - offset) ? length : (cf->compression_info.last_decoded_block_bytes - offset));
		memcpy(bytes_out + written_bytes, cf->compression_info.decoder_buffer + offset, (size_t)block_length);
		written_bytes += block_length;
		offset = 0;
		length -= block_length;
	}

	free(cmp_buf);
	return written_bytes;
}
