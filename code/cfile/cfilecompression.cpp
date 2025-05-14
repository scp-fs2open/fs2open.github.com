#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <thread>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>
#endif

#include "lz4.h"
#include "cfilecompression.h"
#include "cfilearchive.h"
#include "liblzma/api/lzma.h"

/*INTERNAL FUNCTIONS*/
/*LZ41*/
void lz41_load_offsets(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size);
size_t lz41_stream_random_access(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length);
void lz41_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size);
/*XZ*/
void xz_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size);
// Called by xz_stream_random_access(). Do not use directly!
// Block decoder intended to be used for multithreaded cases
int xz_block_decoder(COMPRESSION_INFO *ci, size_t uncompressed_size, size_t input_size, uint8_t* input_buffer, char* bytes_out, size_t offset, size_t copyLength, bool save_to_cache);
// Called by xz_stream_random_access(). Do not use directly!
// Single block decoding version for when we are sure we only need to decode one block, use the minimum amount of code as possible
size_t xz_stream_random_access_single_block_optimised(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length); 
size_t xz_stream_random_access(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length);
/*MISC*/
int fso_fseek(CFILE* cfile, int offset, int where);
int fso_fseek(FILE* fp, size_t compressed_size, size_t lib_offset, int offset, int where);
/*END OF INTERNAL FUNCTIONS*/

int comp_cfile_uses_compression(CFILE* cf)
{
	if (cf->compression_info.header != COMP_HEADER_IS_UNKNOWN || cf->pack_ci_ptr != nullptr)
		return 1;
	return 0;
}

int comp_get_header(char* header)
{
	//XZ
	if (strncmp(header, XZ_HEADER_MAGIC, sizeof(XZ_HEADER_MAGIC)) == 0)
		return COMP_HEADER_IS_XZ;

	//LZ41
	if (strncmp(header, LZ41_HEADER_MAGIC, sizeof(LZ41_HEADER_MAGIC)) == 0)
		return COMP_HEADER_IS_LZ41;

	return COMP_HEADER_IS_UNKNOWN;
}

void comp_create_ci(CFILE* cf, int header_id)
{
	if (header_id == COMP_HEADER_IS_UNKNOWN)
		return;
	//mprintf(("(CI)Compressed File Opened: %s \n", cf->original_filename.c_str()));

	comp_create_ci(&cf->compression_info, cf->fp, cf->size, cf->lib_offset, header_id);
	cf->size = cf->compression_info.uncompressed_size;

	//mprintf(("(CI)Uncompressed FileSize: %d \n", cf->compression_info.uncompressed_size));
	//mprintf(("(CI)Compressed FileSize: %d \n", cf->compression_info.compressed_size));
	//mprintf(("(CI)Block Size: %d \n", cf->compression_info.block_size));
}

void comp_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t file_size, size_t lib_offset, int header_id)
{
	if (COMP_HEADER_IS_XZ == header_id) {
		xz_create_ci(ci, fp, lib_offset, file_size);
	}

	if (COMP_HEADER_IS_LZ41 == header_id) {
		lz41_create_ci(ci, fp, lib_offset, file_size);
	}
}

size_t comp_fread(CFILE* cf, char* buffer, size_t length)
{
	//We need to adapt to the file being a compressed single file, a compressed file inside a uncompressed pack (VPC) or part of a compressed pack
	COMPRESSION_INFO* ci = cf->pack_ci_ptr != nullptr ? cf->pack_ci_ptr : &cf->compression_info;
	
	//lib_offset is the offset of this CFILE in the pack, if it is part of a pack
	//This is needed to fseek to were the file starts inside of it, in a VPC for example
	//But for single files and compressed packs this is always 0, we dont need to seek to were the compressed file start,
	//thats the start of the file itself
	size_t lib_offset = cf->pack_ci_ptr != nullptr ? 0 : cf->lib_offset;
	
	//The offset is like a virtual FILE pointer for compressed files, it helps to track in what position we are in a compressed file
	//Just like the FILE pointer would move after a fread or fseek, the raw_position moves too
	//In a compressed pack we need to include the lib_offset of the CFILE during readings 
	//Because the requested CFILE starts at lib_offset position of the uncompresed data of the compressed pack file
	size_t offset = cf->pack_ci_ptr != nullptr ? cf->lib_offset + cf->raw_position : cf->raw_position;

	return comp_fread(ci, cf->fp, lib_offset, buffer, offset, length);
}

size_t comp_fread(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, char* buffer, size_t offset, size_t length)
{
	/* Check the request to be at least 1 byte */
	Assertion(length > 0, "Invalid length requested.");

	/* Check that we are not requesting to read beyond end of file */
	Assertion(offset + length <= ci->uncompressed_size, "Invalid length requested.");

	if (COMP_HEADER_IS_XZ == ci->header)
		return xz_stream_random_access(ci, fp, lib_offset, buffer, offset, length);

	if (COMP_HEADER_IS_LZ41 == ci->header)
		return lz41_stream_random_access(ci, fp, lib_offset, buffer, offset, length);

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
int fso_fseek(FILE *fp, size_t compressed_size, size_t lib_offset, int offset, int where)
{
	size_t goal_position;

	switch (where) {
	case SEEK_SET: goal_position = offset + lib_offset; break;
	case SEEK_END: goal_position = compressed_size + offset + lib_offset; break;
	default: return 0;
	}
	
	// Make sure we don't seek beyond the end of the file
	CAP(goal_position, lib_offset, lib_offset + compressed_size);
	
	int result = 0;
	if (fp)
	{
		result = fseek(fp, (long)goal_position, SEEK_SET);
		Assertion(goal_position >= lib_offset,
			"Invalid offset values detected while seeking! Goal was " SIZE_T_ARG ", lib_offset is " SIZE_T_ARG ".",
			goal_position,
			lib_offset);
	}
	return result;
}

// Special fseek for compressed files handled by FSO, only SEEK_SET and SEEK_END is supported.
int fso_fseek(CFILE* cfile, int offset, int where)
{
	return fso_fseek(cfile->fp, cfile->compression_info.compressed_size, cfile->lib_offset, offset, where);
}

void lz41_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size)
{
	ci->header = COMP_HEADER_IS_LZ41;
	ci->compressed_size = file_size;
	fso_fseek(fp, file_size, lib_offset, (int)sizeof(int)*-3, SEEK_END);
	auto fNumoffsets = fread(&ci->num_offsets, sizeof(int), 1, fp);
	auto fSize = fread(&ci->uncompressed_size, sizeof(int), 1, fp);
	auto fBsize = fread(&ci->block_size, sizeof(int), 1, fp);

	Assertion(ci->num_offsets > 0, "Invalid number of offsets, compressed file is possibly in the wrong format or corrupted.");
	#if !defined(NDEBUG)
	Assertion(ci->uncompressed_size > 4, "Invalid filesize, compressed file is possibly in the wrong format or corrupted.");
	Assertion(ci->block_size > 16, "Invalid block size, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fNumoffsets == 1, "Error while reading the number of offsets, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fSize == 1, "Error while reading original filesize, compressed file is possibly in the wrong format or corrupted.");
	Assertion(fBsize == 1, "Error while reading block size, compressed file is possibly in the wrong format or corrupted.");
	#endif

	ci->decoder_buffer = (char*)malloc(ci->block_size);
	ci->last_decoded_block_pos = 0;
	ci->last_decoded_block_bytes = 0;
	
	lz41_load_offsets(ci, fp, lib_offset, file_size);
}

void lz41_load_offsets(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size)
{
	ci->offsets = (int*)malloc(ci->num_offsets * sizeof(int));
	int block;
	int* offsets_ptr = ci->offsets;

	/* Seek to the first offset position, remember to consider the trailing ints */
	fso_fseek(fp, file_size, lib_offset, ((sizeof(int) * ci->num_offsets) * -1) - (sizeof(int) * 3), SEEK_END);
	for (block = 0; block < ci->num_offsets; ++block)
	{
		auto bytes_read = fread(offsets_ptr++, sizeof(int), 1, fp);
		Assertion(bytes_read == 1, "Error reading offset list.");
	}
}

size_t lz41_stream_random_access(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length)
{
	LZ4_streamDecode_t lz4_stream_decode_body;
	LZ4_streamDecode_t* lz4_stream_decode = &lz4_stream_decode_body;
	/* The blocks (current_block to end_block) contain the data we want */
	size_t current_block = offset / ci->block_size;
	size_t end_block = ((offset + length - 1) / ci->block_size) + 1;
	size_t written_bytes = 0;

	if (ci->num_offsets <= (int)end_block)
		return (size_t)LZ41_OFFSETS_MISMATCH;

	/* Seek to the first block to read, if it matches the cached block, search the next one instead */
	if (ci->last_decoded_block_pos == ci->offsets[current_block] && current_block + 1 <= end_block)
		fso_fseek(fp, ci->compressed_size, lib_offset, ci->offsets[current_block+1], SEEK_SET);
	else
		fso_fseek(fp, ci->compressed_size, lib_offset, ci->offsets[current_block], SEEK_SET);
	
	offset = offset % ci->block_size;
	char* cmp_buf = (char*)malloc(LZ4_compressBound(ci->block_size));

	/* Start decoding */
	for (; current_block < end_block; ++current_block)
	{
		/* Only read and decode if the requested block is not the cached one */
		if (ci->last_decoded_block_pos != ci->offsets[current_block])
		{
			ci->last_decoded_block_pos = ci->offsets[current_block];

			/* The difference in offsets is the size of the block */
			int cmp_bytes = ci->offsets[current_block + 1] - ci->offsets[current_block];
			auto bytes_read = fread(cmp_buf, cmp_bytes, 1, fp);
			Assertion(bytes_read == 1, "Error reading from compressed file.");

			ci->last_decoded_block_bytes = LZ4_decompress_safe_continue(lz4_stream_decode, cmp_buf, ci->decoder_buffer, cmp_bytes, ci->block_size);
			if (ci->last_decoded_block_bytes <= 0)
			{
				free(cmp_buf);
				return (size_t)LZ41_DECOMPRESSION_ERROR;
			}
		}

		/* Write out the part of the data we care about from buffer */
		size_t block_length = (length < (ci->last_decoded_block_bytes - offset) ? length : (ci->last_decoded_block_bytes - offset));
		memcpy(bytes_out + written_bytes, ci->decoder_buffer + offset, (size_t)block_length);
		written_bytes += block_length;
		offset = 0;
		length -= block_length;
	}

	free(cmp_buf);
	return written_bytes;
}

/***************************************************************/

void xz_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t lib_offset, size_t file_size)
{
	ci->header = COMP_HEADER_IS_XZ;
	ci->compressed_size = file_size;

	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_ret ret = lzma_file_info_decoder(&strm, &ci->xz_block_index, UINT64_MAX, file_size);
	if (ret != LZMA_OK) {
		//TODO
	}
	
	strm.avail_in = 0;
	uint8_t in[LZMA_BLOCK_HEADER_SIZE_MAX];

	// Decode file info
	while (1) {
		if (strm.avail_in == 0) {
			strm.next_in = in;
			strm.avail_in = fread(in, 1, sizeof(in), fp);
		}

		ret = lzma_code(&strm, LZMA_RUN);

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END)
				break;

			if (ret == LZMA_SEEK_NEEDED) {
				strm.avail_in = 0;
				fso_fseek(fp, file_size, lib_offset, (long)strm.seek_pos, SEEK_SET);
			} else {
				//TODO: Info decoding failed
			}
		}
	}

	lzma_end(&strm);

	// Set index iterator
	ci->xz_index_iter = (lzma_index_iter*)malloc(sizeof(lzma_index_iter));
	lzma_index_iter_init(ci->xz_index_iter, ci->xz_block_index);

	//Find the first block, this is needed so it loads data into the iterator
	lzma_index_iter_locate(ci->xz_index_iter, 0);

	ci->uncompressed_size = ci->xz_index_iter->stream.uncompressed_size;
	ci->block_size = (int)ci->xz_index_iter->block.uncompressed_size;
	ci->decoder_buffer = (char*)malloc(ci->block_size);
	ci->last_decoded_block_pos = 0;
	ci->last_decoded_block_bytes = 0;
}

int xz_block_decoder(COMPRESSION_INFO *ci, size_t block_uncompressed_size, size_t input_size, uint8_t* input_buffer, char* bytes_out, size_t offset, size_t copy_length, bool save_to_cache)
{
	auto entire_block = copy_length == block_uncompressed_size;
	/*
	* If we are saving to cache always use the cache as the output buffer and then memcpy the results to bytes_out.
	* If not, then check if we need to write the entire block, if so use the bytes_out as the output buffer directly.
	* Only use local buffer in case of partial block reads that we are not saving to cache.
	*/
	auto outbuf = save_to_cache ? (uint8_t*)ci->decoder_buffer : 
				  entire_block  ? (uint8_t*)bytes_out : (uint8_t*)malloc(block_uncompressed_size);

	//Set block
	lzma_block block;
	block.check = ci->xz_index_iter->stream.flags->check;
	lzma_filter block_filters[LZMA_FILTERS_MAX + 1];
	block_filters[LZMA_FILTERS_MAX].id = LZMA_VLI_UNKNOWN;
	block.filters = block_filters;
	block.header_size = lzma_block_header_size_decode(input_buffer[0]);
	//TODO: Check header size min/max in debug

	//Decode block Header
	auto ret_hd = lzma_block_header_decode(&block, nullptr, input_buffer);
	//TODO Check return in debug

	//Set Decoding stream, we must skip the block header
	lzma_stream stream = LZMA_STREAM_INIT;
	stream.next_in = input_buffer + block.header_size;
	stream.next_out = outbuf;
	stream.avail_out = block_uncompressed_size;
	stream.avail_in = input_size - block.header_size;
	
	//Set block decoder
	auto ret_bd = lzma_block_decoder(&stream, &block);
	//TODO: Check return in debug

	//Decode block data
	auto ret_code = lzma_code(&stream, LZMA_RUN);

	//Copy decoded data only when saving to cache and partial reads
	if (save_to_cache || !entire_block)
		memcpy(bytes_out, outbuf + offset, copy_length);

	//Cleanup
	lzma_filters_free(block_filters, nullptr);
	free(input_buffer);
	lzma_end(&stream);
	if (!save_to_cache && !entire_block)
		free(outbuf);

	return 0;
}

size_t xz_stream_random_access_single_block_optimised(COMPRESSION_INFO *ci, FILE *fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length)
{
	// Get the first block we need. Do this before changing the offset
	auto ret = lzma_index_iter_locate(ci->xz_index_iter, offset);

	// Determines the offset of where the data requested starts at the output the first block to decode
	if (offset != 0) {
		offset = offset % ci->block_size;
	}

	// Determine how much data we need to copy from this block to bytes_out
	size_t copy_length = (length < (ci->xz_index_iter->block.uncompressed_size - offset) ? length : ci->xz_index_iter->block.uncompressed_size - offset);

	// If the requested block is the same as the last decoded block use the cache, otherwise find and decode the block
	if (ci->last_decoded_block_pos != ci->xz_index_iter->block.compressed_file_offset) {
		// Seek to the block to read
		fso_fseek(fp, ci->compressed_size, lib_offset, ci->xz_index_iter->block.compressed_file_offset, SEEK_SET);

		uint8_t* inbuf = (uint8_t*)malloc(ci->xz_index_iter->block.total_size);

		// Read block into memory
		fread(inbuf, 1, ci->xz_index_iter->block.total_size, fp);

		// Set block
		lzma_block block;
		block.check = ci->xz_index_iter->stream.flags->check;
		lzma_filter block_filters[LZMA_FILTERS_MAX + 1];
		block_filters[LZMA_FILTERS_MAX].id = LZMA_VLI_UNKNOWN;
		block.filters = block_filters;
		block.header_size = lzma_block_header_size_decode(inbuf[0]);
		// TODO: Check header size min/max in debug

		// Decode block Header
		auto ret_hd = lzma_block_header_decode(&block, nullptr, inbuf);
		// TODO Check return in debug

		// Set Decoding stream, we must skip the block header
		lzma_stream stream = LZMA_STREAM_INIT;
		stream.next_in = inbuf + block.header_size;
		stream.next_out = (uint8_t*)ci->decoder_buffer;
		stream.avail_out = ci->xz_index_iter->block.uncompressed_size;
		stream.avail_in = ci->xz_index_iter->block.total_size - block.header_size;

		// Set block decoder
		auto ret_bd = lzma_block_decoder(&stream, &block);
		// TODO: Check return in debug

		// Decode block data
		auto ret_code = lzma_code(&stream, LZMA_RUN);
		
		// Save data for cache to use later
		ci->last_decoded_block_bytes = ci->xz_index_iter->block.uncompressed_size;
		ci->last_decoded_block_pos = ci->xz_index_iter->block.compressed_file_offset;

		//Cleanup
		free(inbuf);
		lzma_end(&stream);
		lzma_filters_free(block_filters, nullptr);
	}

	// Copy requested data
	memcpy(bytes_out, ci->decoder_buffer + offset, copy_length);

	return copy_length;
}


size_t xz_stream_random_access(COMPRESSION_INFO* ci, FILE *fp, size_t lib_offset, char* bytes_out, size_t offset, size_t length)
{
	if (length == 0)
		return 0;

	// Determine the the block number of the first and last block we need to decode
	size_t start_block = offset / ci->block_size;
	size_t end_block = (offset + length - 1) / ci->block_size;

	//If we need to decode only one block use the single-block version as it is more efficient
	if (start_block == end_block) {
		return xz_stream_random_access_single_block_optimised(ci, fp, lib_offset, bytes_out, offset, length);
	}

	// Create the thread pool, one thread per block to decode
	SCP_vector<std::thread> thread_pool;
	size_t written_bytes = 0;

	// Get the first block we need. Do this before changing the offset
	auto ret = lzma_index_iter_locate(ci->xz_index_iter, offset);
	// TODO: Check return in debug

	// Determines the offset of where the data requested starts at the output the first block to decode
	if (offset != 0) {
		offset = offset % ci->block_size;
	}

	while (length > 0) {
		// Determine how much data we need to copy from this block to bytes_out
		size_t copy_length = (length < (ci->xz_index_iter->block.uncompressed_size - offset) ? length : ci->xz_index_iter->block.uncompressed_size - offset);

		// Is this is the last block to decode? Is so we will need to save it to the cache
		auto is_last_block = (length - copy_length) == 0;

		// If the requested block is the same as the last decoded block use the cache, otherwise find and decode the block
		if (ci->last_decoded_block_pos != ci->xz_index_iter->block.compressed_file_offset) {
			// Seek to the block to read
			fso_fseek(fp, ci->compressed_size, lib_offset, ci->xz_index_iter->block.compressed_file_offset, SEEK_SET);

			uint8_t* inbuf = (uint8_t*)malloc(ci->xz_index_iter->block.total_size);

			// Read block into memory
			auto read_bytes = fread(inbuf, 1, ci->xz_index_iter->block.total_size, fp);

			// Decode block in thread pool
			thread_pool.emplace_back(xz_block_decoder,
				ci,
				ci->xz_index_iter->block.uncompressed_size,
				ci->xz_index_iter->block.total_size,
				inbuf,
				bytes_out + written_bytes,
				offset,
				copy_length,
				is_last_block);

			if (is_last_block) {
				// Save data for cache to use later
				ci->last_decoded_block_bytes = ci->xz_index_iter->block.uncompressed_size;
				ci->last_decoded_block_pos = ci->xz_index_iter->block.compressed_file_offset;
			}

		} else {
			// Copy requested data from cache
			memcpy(bytes_out + written_bytes, ci->decoder_buffer + offset, copy_length);
		}

		// Look for the next block if we are not finished
		if (!is_last_block) {
			lzma_index_iter_next(ci->xz_index_iter, LZMA_INDEX_ITER_BLOCK);
		}

		written_bytes += copy_length;
		length -= copy_length;
		offset = 0;
	}

	// Wait for all decode threads to finish
	for (auto &th : thread_pool)
		th.join();

	return written_bytes;
}