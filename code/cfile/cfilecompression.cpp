#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <atomic>
#include <limits>
#include <memory>
#include <new>
#include <system_error>
#include <thread>

#include "lz4.h"
#include "cfilecompression.h"
#include "cfilearchive.h"
#include "cmdline/cmdline.h"
#include "utils/threading.h"


constexpr size_t LZ41_MT_MIN_BLOCKS = 5;
constexpr int LZ41_MAX_BLOCK_SIZE = 4194304; // 4 MB
constexpr int LZ41_MIN_BLOCK_SIZE = 16;

/*INTERNAL FUNCTIONS*/
/*LZ41*/
static bool   lz41_create_ci(CFILE* cf, int header);
static bool   lz41_load_and_validate_offsets(CFILE* cf);
static int    lz41_decode_one_block(CFILE* cf, int block);
static size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length);
static size_t lz41_random_access_mt(CFILE* cf, char* bytes_out, size_t offset, size_t length, size_t current_block, size_t end_block, int max_threads);

/*MISC*/
static int fso_fseek(const CFILE* cf, size_t offset);
static int max_threads();
/*END OF INTERNAL FUNCTIONS*/

bool comp_create_ci(CFILE* cf, int header)
{
	if (lz41_create_ci(cf, header))
		return true;

	cf_clear_compression_info(cf);
	mprintf(("CFILE: '%s' is not a valid LZ41 file.\n", cf->original_filename.c_str()));
	return false;
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
	return cf->compression_info.read_failed || cf->raw_position >= cf->size;
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
	cf->compression_info.read_failed = false;

	return 0;
}

static int max_threads()
{
	if (Cmdline_multithreading != 0)
		return Cmdline_multithreading;

	static const auto auto_threads = static_cast<int>(threading::get_num_auto_threads());
	return auto_threads;
}

// Seek within the stored file, which may be a member of a VP.
static int fso_fseek(const CFILE* cf, size_t offset)
{
	const auto max_position = static_cast<size_t>(std::numeric_limits<long>::max());
	if (offset > cf->compression_info.compressed_size || cf->lib_offset > max_position ||
		offset > max_position - cf->lib_offset)
		return -1;

	return fseek(cf->fp, static_cast<long>(cf->lib_offset + offset), SEEK_SET);
}

// Read and validate the LZ41 footer + offset table. Returns true only if the metadata is consistent
static bool lz41_create_ci(CFILE* cf, int header)
{
	COMPRESSION_INFO* ci = &cf->compression_info;
	ci->header = header;
	ci->compressed_size = cf->size;

	// Minimum layout: header, at least one compressed byte, two offsets, footer.
	if (cf->size < 25 || cf->size > static_cast<size_t>(std::numeric_limits<int>::max()))
		return false;

	int footer[3];
	if (fso_fseek(cf, cf->size - sizeof(footer)) != 0 ||
		fread(footer, sizeof(footer), 1, cf->fp) != 1)
		return false;

	const int num_offsets = INTEL_INT(footer[0]);
	const int orig_size = INTEL_INT(footer[1]);
	const int block_size = INTEL_INT(footer[2]);
	if (block_size < LZ41_MIN_BLOCK_SIZE || block_size > LZ41_MAX_BLOCK_SIZE || orig_size <= 0)
		return false;

	const size_t max_offsets = (cf->size - sizeof(footer) - sizeof(int) - 1) / sizeof(int);
	if (num_offsets < 2 || static_cast<size_t>(num_offsets) > max_offsets ||
		num_offsets - 1 != 1 + (orig_size - 1) / block_size)
		return false;

	ci->num_offsets = num_offsets;
	ci->block_size = block_size;
	if (!lz41_load_and_validate_offsets(cf))
		return false;

	ci->decoder_buffer = static_cast<char*>(malloc(static_cast<size_t>(block_size)));
	if (ci->decoder_buffer == nullptr)
		return false;

	cf->size = static_cast<size_t>(orig_size);
	return true;
}

static bool lz41_load_and_validate_offsets(CFILE* cf)
{
	auto& ci = cf->compression_info;
	const auto table_size = ci.num_offsets * sizeof(int);
	const auto table_start = static_cast<int>(ci.compressed_size - 3 * sizeof(int) - table_size);

	ci.offsets = static_cast<int*>(malloc(table_size));
	if (ci.offsets == nullptr || fso_fseek(cf, table_start) != 0 ||
		fread(ci.offsets, table_size, 1, cf->fp) != 1)
		return false;

	ci.offsets[0] = INTEL_INT(ci.offsets[0]);
	if (ci.offsets[0] != static_cast<int>(sizeof(int)))
		return false;

	const int max_block_bytes = LZ4_compressBound(ci.block_size);
	for (int i = 1; i < ci.num_offsets; ++i) {
		const int offset = INTEL_INT(ci.offsets[i]);
		const int prev = ci.offsets[i - 1];
		if (offset <= prev || offset > table_start || offset - prev > max_block_bytes)
			return false;

		ci.offsets[i] = offset;
	}

	return ci.offsets[ci.num_offsets - 1] == table_start;
}

// A block must fill its entire logical range, including a possibly partial last block.
static int lz41_decode_block(const CFILE* cf, size_t block, const char* src, int src_size, char* dst)
{
	const auto block_size = static_cast<size_t>(cf->compression_info.block_size);
	const int expected = static_cast<int>(std::min(block_size, cf->size - block * block_size));
	return LZ4_decompress_safe(src, dst, src_size, expected) == expected ? expected : 0;
}

// Decode into the shared cache. Returns zero on error.
static int lz41_decode_one_block(CFILE* cf, int block)
{
	auto& ci = cf->compression_info;
	if (ci.cached_block == block)
		return ci.cached_block_bytes;

	const int cmp_bytes = ci.offsets[block + 1] - ci.offsets[block];
	if (fso_fseek(cf, ci.offsets[block]) != 0)
		return 0;

	std::unique_ptr<char[]> cmp_buffer(new (std::nothrow) char[cmp_bytes]);
	if (!cmp_buffer || fread(cmp_buffer.get(), 1, cmp_bytes, cf->fp) != static_cast<size_t>(cmp_bytes))
		return 0;

	const int decoded = lz41_decode_block(cf, block, cmp_buffer.get(), cmp_bytes, ci.decoder_buffer);
	ci.cached_block = decoded == 0 ? -1 : block;
	ci.cached_block_bytes = decoded;
	return decoded;
}

// Random-access read of length uncompressed bytes starting at `offset`.
// The read maps onto a contiguous run of blocks [current_block, end_block). Single or few-block reads
// go through the cached single-threaded path.
static size_t lz41_stream_random_access(CFILE* cf, char* bytes_out, size_t offset, size_t length)
{
	COMPRESSION_INFO* ci = &cf->compression_info;

	if (ci->offsets == nullptr || ci->num_offsets < 2)
		return 0;

	const auto block_size = static_cast<size_t>(ci->block_size);

	// The blocks [current_block, end_block) contain the data we want.
	const size_t current_block = offset / block_size;
	const size_t end_block     = ((offset + length - 1) / block_size) + 1;

	// We index offsets[current_block .. end_block].
	if ((size_t)ci->num_offsets <= end_block)
		return 0;

	const size_t block_count = end_block - current_block;

	if (block_count >= LZ41_MT_MIN_BLOCKS) {
		const auto threads = max_threads();
		if (threads > 1)
			return lz41_random_access_mt(cf, bytes_out, offset, length, current_block, end_block, threads);
	}

	// ST path
	size_t in_block_offset = offset % block_size;
	size_t written_bytes = 0;

	for (size_t block = current_block; block < end_block; ++block) {
		const int decoded = lz41_decode_one_block(cf, (int)block);
		if (decoded == 0)
			return 0;

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
		return 0;

	SCP_vector<char> span;
	try {
		span.resize((size_t)span_bytes);
	} catch (const std::bad_alloc&) {
		return 0;
	}

	if (fso_fseek(cf, span_start) != 0)
		return 0;
	if (fread(span.data(), 1, (size_t)span_bytes, cf->fp) != (size_t)span_bytes)
		return 0;

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
			const int cmp_bytes = ci->offsets[block + 1] - ci->offsets[block];
			const char* src = span.data() + (ci->offsets[block] - span_start);
			const int decoded = lz41_decode_block(cf, block, src, cmp_bytes, local.data());
			if (decoded == 0) {
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

	const size_t workers = std::min<size_t>(block_count, max_threads);
	const size_t per = (block_count + workers - 1) / workers;
	SCP_vector<std::thread> threads;
	size_t inline_start = current_block;

	try {
		threads.reserve(workers - 1);
		for (; end_block - inline_start > per; inline_start += per)
			threads.emplace_back(worker, inline_start, inline_start + per);
	} catch (const std::bad_alloc&) {
		// Finish the unassigned blocks on the calling thread.
	} catch (const std::system_error&) {
		// Finish the unassigned blocks if the system cannot start another worker.
	}

	worker(inline_start, end_block);
	for (auto& t : threads)
		t.join();

	if (failed.load())
		return 0;

	return length;
}
