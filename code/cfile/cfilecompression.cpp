#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <thread>
#include <atomic>
#include <algorithm>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>
#endif

#include "lz4.h"
#include "cfilecompression.h"
#include "cfilearchive.h"
#include "utils/threading.h"


constexpr int LZ41_MT_MIN_BLOCKS = 3; // Minimum number of consecutive blocks to trigger MT
constexpr int LZ41_MAX_BLOCK_SIZE = 4194304; // 4 MB
constexpr int LZ41_MIN_BLOCK_SIZE = 16;

/*INTERNAL FUNCTIONS*/
/*LZ41*/
static bool   lz41_create_ci(CFILE* cf, int header);
static bool   lz41_load_and_validate_offsets(CFILE* cf);
static int    lz41_block_compressed_size(const CFILE* cf, int block);
static int    lz41_decode_one_block(CFILE* cf, int block);
static size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length);
static size_t lz41_random_access_mt(CFILE* cf, char* bytes_out, size_t offset, size_t length, size_t current_block, size_t end_block, int max_threads);

/*MISC*/
static int fso_fseek(CFILE* cfile, int offset, int where);
static int max_threads();
/*END OF INTERNAL FUNCTIONS*/

int comp_check_header(int header)
{
	if (LZ41_FILE_HEADER == header)
		return COMP_HEADER_MATCH;

	return COMP_HEADER_MISMATCH;
}

void comp_create_ci(CFILE* cf, int header)
{
	bool ok = false;

	if (LZ41_FILE_HEADER == header)
		ok = lz41_create_ci(cf, header);

	// If validation fails drop back to "not compressed" so the file is handled as a normal
	// (raw) file by the rest of CFILE. This keeps every later read bounds-safe instead of
	// trusting a corrupt footer. The data will be wrong, but no decoder crash can occur.
	if (!ok) {
		cf_clear_compression_info(cf);
		mprintf(("CFILE: '%s' looks compressed but its header/footer failed validation; "
		         "treating it as a normal file.\n", cf->original_filename.c_str()));
	}
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

extern int Cmdline_multithreading;
static int allowed_threads = Cmdline_multithreading;
//Get number of max threads we are allowed to use
static int max_threads()
{
	if (allowed_threads == 0) {
		allowed_threads = static_cast<int>(threading::get_num_physical_cores());
	}
	return allowed_threads;
}

//Special fseek for compressed files handled by FSO, only SEEK_SET and SEEK_END is supported.
static int fso_fseek(CFILE* cfile, int offset, int where)
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

// Read and validate the LZ41 footer + offset table. Returns true only if the metadata is consistent
static bool lz41_create_ci(CFILE* cf, int header)
{
	COMPRESSION_INFO* ci = &cf->compression_info;
	ci->header = header;
	ci->compressed_size = cf->size;

	// Smallest possible valid file: 4 (header) + 1 (>=1 compressed byte) + 8 (2 offsets) + 12 (footer).
	if (cf->size < 25) {
		mprintf(("LZ41: '%s' is too small to be a valid compressed file (" SIZE_T_ARG " bytes).\n",
		         cf->original_filename.c_str(), cf->size));
		return false;
	}

	// Read the footer (NUM_OFFSETS, ORIGINAL_FILESIZE, BLOCK_SIZE)
	int num_offsets = 0, orig_size = 0, block_size = 0;
	if (fso_fseek(cf, (int)sizeof(int) * -3, SEEK_END) != 0) return false;
	if (fread(&num_offsets, sizeof(int), 1, cf->fp) != 1) return false;
	if (fread(&orig_size,   sizeof(int), 1, cf->fp) != 1) return false;
	if (fread(&block_size,  sizeof(int), 1, cf->fp) != 1) return false;

	// Validate BLOCK_SIZE
	if (block_size < LZ41_MIN_BLOCK_SIZE || block_size > LZ41_MAX_BLOCK_SIZE) {
		mprintf(("LZ41: '%s' has an invalid block size (%d).\n", cf->original_filename.c_str(), block_size));
		return false;
	}

	// Validate NUM_OFFSETS
	// There must be at least 2 offsets (one block), and the table has to physically fit before the 12-byte footer.
	const long payload = (long)cf->size;
	const long max_offsets_by_size = (payload - 12) / (long)sizeof(int);
	if (num_offsets < 2 || (long)num_offsets > max_offsets_by_size) {
		mprintf(("LZ41: '%s' has an invalid offset count (%d).\n", cf->original_filename.c_str(), num_offsets));
		return false;
	}

	// Validate ORIGINAL_FILESIZE
	// num_blocks == num_offsets - 1, and the uncompressed size must round up to exactly that many blocks of block_size.
	const long num_blocks = (long)num_offsets - 1;
	if (orig_size <= 0) {
		mprintf(("LZ41: '%s' has an invalid uncompressed size (%d).\n", cf->original_filename.c_str(), orig_size));
		return false;
	}
	const long expected_blocks = ((long)orig_size + block_size - 1) / block_size;
	if (expected_blocks != num_blocks) {
		mprintf(("LZ41: '%s' footer is inconsistent (uncompressed size %d implies %ld blocks, table has %ld).\n",
		         cf->original_filename.c_str(), orig_size, expected_blocks, num_blocks));
		return false;
	}

	ci->num_offsets = num_offsets;
	ci->block_size  = block_size;
	cf->size        = (size_t)orig_size; // from here on cf->size is the uncompressed size

	// Allocate the decoder_buffer, holds one decoded block.
	ci->decoder_buffer = (char*)malloc((size_t)block_size);
	if (ci->decoder_buffer == nullptr) {
		mprintf(("LZ41: '%s' could not allocate decode buffer.\n", cf->original_filename.c_str()));
		return false;
	}

	ci->cached_block = -1;
	ci->cached_block_bytes = 0;

	return lz41_load_and_validate_offsets(cf);
}

// Load the offset table and validate it. The table must:
//   * start at offset 4 (immediately after the header),
//   * be strictly monotonically increasing,
//   * stay within [4, start-of-offset-table].
static bool lz41_load_and_validate_offsets(CFILE* cf)
{
	COMPRESSION_INFO* ci = &cf->compression_info;

	ci->offsets = (int*)malloc((size_t)ci->num_offsets * sizeof(int));
	if (ci->offsets == nullptr) {
		mprintf(("LZ41: '%s' could not allocate the offset table.\n", cf->original_filename.c_str()));
		return false;
	}

	// The offset table sits before the 12-byte footer.
	const int table_start = (int)ci->compressed_size - 12 - (int)(ci->num_offsets * sizeof(int));
	if (table_start < 4) {
		mprintf(("LZ41: '%s' offset table does not fit in the file.\n", cf->original_filename.c_str()));
		return false;
	}

	if (fso_fseek(cf, table_start, SEEK_SET) != 0)
		return false;

	int prev = 3; // first real offset must be >= 4, so seed below it
	for (int i = 0; i < ci->num_offsets; ++i) {
		int off = 0;
		if (fread(&off, sizeof(int), 1, cf->fp) != 1) {
			mprintf(("LZ41: '%s' error reading offset %d.\n", cf->original_filename.c_str(), i));
			return false;
		}
		if (i == 0 && off != 4) {
			mprintf(("LZ41: '%s' first offset is %d (expected 4).\n", cf->original_filename.c_str(), off));
			return false;
		}
		if (off <= prev || off > table_start) {
			mprintf(("LZ41: '%s' offset %d out of range or not increasing (%d).\n",
			         cf->original_filename.c_str(), i, off));
			return false;
		}
		ci->offsets[i] = off;
		prev = off;
	}

	// The final offset must mark exactly the start of the offset table.
	if (ci->offsets[ci->num_offsets - 1] != table_start) {
		mprintf(("LZ41: '%s' last offset (%d) does not match the offset table start (%d).\n",
		         cf->original_filename.c_str(), ci->offsets[ci->num_offsets - 1], table_start));
		return false;
	}

	return true;
}

// Compressed byte length of a single block, or -1 if the offsets produce an impossible size
static int lz41_block_compressed_size(const CFILE* cf, int block)
{
	const COMPRESSION_INFO* ci = &cf->compression_info;
	if (block < 0 || block + 1 >= ci->num_offsets)
		return -1;

	const int len = ci->offsets[block + 1] - ci->offsets[block];
	if (len <= 0 || len > LZ4_compressBound(ci->block_size))
		return -1;

	return len;
}

// Decode a single block into the shared decoder_buffer. Returns the number of decoded bytes, or a
// negative value on error.
static int lz41_decode_one_block(CFILE* cf, int block)
{
	COMPRESSION_INFO* ci = &cf->compression_info;

	if (ci->cached_block == block)
		return ci->cached_block_bytes;

	const int cmp_bytes = lz41_block_compressed_size(cf, block);
	if (cmp_bytes < 0)
		return LZ41_OFFSETS_MISMATCH;

	if (fso_fseek(cf, ci->offsets[block], SEEK_SET) != 0)
		return LZ41_DECOMPRESSION_ERROR;

	auto cmp_buffer = (char*)malloc((size_t)LZ4_compressBound(ci->block_size));
	if (cmp_buffer == nullptr)
		return LZ41_DECOMPRESSION_ERROR;

	if (fread(cmp_buffer, 1, (size_t)cmp_bytes, cf->fp) != (size_t)cmp_bytes)
		return LZ41_DECOMPRESSION_ERROR;

	const int decoded = LZ4_decompress_safe(cmp_buffer, ci->decoder_buffer, cmp_bytes, ci->block_size);
	free(cmp_buffer);

	if (decoded <= 0) {
		ci->cached_block = -1;
		return LZ41_DECOMPRESSION_ERROR;
	}

	ci->cached_block = block;
	ci->cached_block_bytes = decoded;
	return decoded;
}

// Random-access read of length uncompressed bytes starting at `offset`.
// The read maps onto a contiguous run of blocks [current_block, end_block). Single or few-block reads
// go through the cached single-threaded path. Reads that span more than LZ41_MT_MIN_BLOCKS blocks are decoded in parallel. 
static size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length)
{
	COMPRESSION_INFO* ci = &cf->compression_info;

	if (ci->offsets == nullptr || ci->num_offsets < 2)
		return (size_t)LZ41_OFFSETS_MISMATCH;

	const auto block_size = static_cast<size_t>(ci->block_size);

	// The blocks [current_block, end_block) contain the data we want.
	const size_t current_block = offset / block_size;
	const size_t end_block     = ((offset + length - 1) / block_size) + 1;

	// We index offsets[current_block .. end_block].
	if ((size_t)ci->num_offsets <= end_block)
		return (size_t)LZ41_OFFSETS_MISMATCH;

	const size_t block_count = end_block - current_block;

	if (block_count > LZ41_MT_MIN_BLOCKS) {
		const auto threads = max_threads();
		if(threads > 1)
			return lz41_random_access_mt(cf, bytes_out, offset, length, current_block, end_block, threads); // MT path
	}

	// ST path
	size_t in_block_offset = offset % block_size;
	size_t written_bytes = 0;

	for (size_t block = current_block; block < end_block; ++block) {
		const int decoded = lz41_decode_one_block(cf, (int)block);
		if (decoded < 0)
			return (size_t)decoded;

		if ((size_t)decoded <= in_block_offset)
			return (size_t)LZ41_DECOMPRESSION_ERROR; // block shorter than expected -> corrupt

		const size_t available = (size_t)decoded - in_block_offset;
		const size_t to_copy   = (length < available) ? length : available;

		memcpy(bytes_out + written_bytes, ci->decoder_buffer + in_block_offset, to_copy);
		written_bytes += to_copy;
		length        -= to_copy;
		in_block_offset = 0;
	}

	return written_bytes;
}

// Parallel decode for reads that span many blocks.
// The whole compressed bytes of the involved blocks is read in a single fread.
// Decoding is then done across multiple threads.
static size_t lz41_random_access_mt(CFILE* cf, char* bytes_out, size_t offset, size_t length, size_t current_block, size_t end_block, int max_threads)
{
	COMPRESSION_INFO* ci = &cf->compression_info;
	const auto block_size = static_cast<size_t>(ci->block_size);
	const size_t block_count = end_block - current_block;

	// Compressed span: from the start of the first block to the start of the block past the last.
	const int span_start = ci->offsets[current_block];
	const int span_end   = ci->offsets[end_block];
	const int span_bytes = span_end - span_start;
	if (span_bytes <= 0)
		return (size_t)LZ41_OFFSETS_MISMATCH;

	SCP_vector<char> span;
	try {
		span.resize((size_t)span_bytes);
	} catch (const std::bad_alloc&) {
		return (size_t)LZ41_DECOMPRESSION_ERROR;
	}

	if (fso_fseek(cf, span_start, SEEK_SET) != 0)
		return (size_t)LZ41_DECOMPRESSION_ERROR;
	if (fread(span.data(), 1, (size_t)span_bytes, cf->fp) != (size_t)span_bytes)
		return (size_t)LZ41_DECOMPRESSION_ERROR;

	// The absolute uncompressed range. Each block writes only its overlap with this range.
	const size_t read_start = offset;
	const size_t read_end   = offset + length;

	std::atomic<bool> failed(false);

	// Set workers to decode blocks [lo, hi) into bytes_out at their destinations.
	auto worker = [&](size_t lo, size_t hi) {
		SCP_vector<char> local;
		try {
			local.resize(block_size);
		} catch (const std::bad_alloc&) {
			failed.store(true);
			return;
		}

		for (size_t block = lo; block < hi && !failed.load(); ++block) {
			const int cmp_bytes = lz41_block_compressed_size(cf, (int)block);
			if (cmp_bytes < 0) { 
				failed.store(true); 
				return; 
			}

			const char* src = span.data() + (ci->offsets[block] - span_start);
			const int decoded = LZ4_decompress_safe(src, local.data(), cmp_bytes, (int)block_size);
			if (decoded <= 0) { 
				failed.store(true); 
				return; 
			}

			const size_t block_uncomp_start = block * block_size;
			const size_t block_uncomp_end   = block_uncomp_start + (size_t)decoded;

			// Overlap of [block_uncomp_start, block_uncomp_end) with [read_start, read_end).
			const size_t copy_start = std::max(block_uncomp_start, read_start);
			const size_t copy_end   = std::min(block_uncomp_end,   read_end);
			if (copy_start >= copy_end)
				continue; // nothing of this block is inside the requested range

			memcpy(bytes_out + (copy_start - read_start),
			       local.data() + (copy_start - block_uncomp_start),
			       copy_end - copy_start);
		}
	};

	// Decide worker count and split the block range into contiguous chunks.
	size_t workers = std::min<size_t>(block_count, max_threads);
	if (workers < 1) workers = 1;

	const size_t per = (block_count + workers - 1) / workers;
	SCP_vector<std::thread> threads;
	threads.reserve(workers);

	for (size_t w = 0; w < workers; ++w) {
		const size_t lo = current_block + w * per;
		if (lo >= end_block) break;

		const size_t hi = std::min(lo + per, end_block);
		if (w + 1 == workers || hi == end_block) {
			worker(lo, hi);
			break;
		}
		threads.emplace_back(worker, lo, hi);
	}

	for (auto& t : threads)
		t.join();

	if (failed.load())
		return (size_t)LZ41_DECOMPRESSION_ERROR;

	return length;
}
