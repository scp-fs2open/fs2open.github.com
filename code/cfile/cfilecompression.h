/*
ShivanSpS - Compressed files support for FSO. Many thanks to ngld, taylor and everyone elsewho helped me in getting this done.

-The file header is a version, this is used to tell FSO how to decompress that file, always use 4 chars to mantain alignment.
-The header ID can be used to add diferent revisions to LZ41 decompression system or to add other compression format supports whiout breaking compatibility.
-The system uses a offset list to record the position of every block in file, this list, along with the number of offsets, original filesize,
and block size, must be written by the compressor app.
-A decoder cache is used to store the last decoded block, this ensures each block is read and decoded only once in sequential reads. This cache
is a little bit bigger than the block size, a higher block size means less overhead added to the file, but it also means a little more ram will
be used during decompression.
-All this dynamic memory is assigned at cfopen() and it is cleared on cfclose().

................................char[4]..........(n ints)...(int)..........(int)..........(int)
-COMPRESSED FILE DATA STUCTURE: HEADER|N BLOCKS|N OFFSETS|NUM_OFFSETS|ORIGINAL_FILESIZE|BLOCK_SIZE
*/

#ifndef _CFILECOMPRESSION_H
#define _CFILECOMPRESSION_H
struct CFILE;

#include "liblzma/api/lzma.h"

/*LZ41*/
const unsigned char LZ41_HEADER_MAGIC[4] = { 'L', 'Z', '4', '1' };
#define LZ41_DECOMPRESSION_ERROR -1
#define LZ41_MAX_BLOCKS_OVERFLOW -2
#define LZ41_HEADER_MISMATCH -3
#define LZ41_OFFSETS_MISMATCH -4

/* XZ (liblzma) */
const unsigned char XZ_HEADER_MAGIC[6] = { 0xFD, '7', 'z', 'X', 'Z', 0x00 };

/******/

struct COMPRESSION_INFO {
	/* COMMON */
	int header = 0;
	size_t compressed_size = 0;
	size_t uncompressed_size = 0;
	int block_size = 0;
	char* decoder_buffer = nullptr;
	size_t last_decoded_block_pos = 0;
	size_t last_decoded_block_bytes = 0;
	size_t uncompressed_pos = 0;
	/* LZ41 */
	int num_offsets = 0;
	int* offsets = nullptr;
	/* XZ */
	lzma_index* xz_block_index = nullptr;
	lzma_index_iter* xz_index_iter = nullptr;
};

#define COMP_HEADER_IS_XZ 2 // File uses XZ (liblzma) compression type
#define COMP_HEADER_IS_LZ41 1 // File uses LZ41 compression type
#define COMP_HEADER_IS_UNKNOWN 0  // File is not compressed or unknown format
#define COMP_HEADER_MAX_BYTES 8 // Defines the max number of bytes read from the file for header comparison
#define COMP_FILE_MIN_SIZE 24 // The smallest size a compressed file can be, anything smaller than this will be ignored
#define COMP_MAX_DECODER_BUFFER 4194304 //4MB, max size for optimization, anything bigger will result on absurd ram usage

/*
	Check if this CFILE must be handled by cfilecompression "comp_" functions by checking its CI and pack_ci_ptr data
	Returns 1 if it does, 0 otherwise.
*/
int comp_cfile_uses_compression(CFILE* cf);

/*
	Returns the internal header_id if the file has a compressed file header magic,
	returns COMP_HEADER_IS_UNKNOWN if it dosent.
*/
int comp_get_header(char* header);

/*
	This is called to generate the correct CFILE compression_info data
	after the CFILE has been indentified as a compressed file by comp_get_header()
	This must be done before calling any other function.
*/
void comp_create_ci(CFILE* cf, int header_id);

/*
	This is called to generate the correct compression_info data
	after the file has been indentified as a compressed file by comp_get_header()
	This must be done before calling any other function.
*/
void comp_create_ci(COMPRESSION_INFO* ci, FILE* fp, size_t file_size, size_t lib_offset, int header_id, SCP_string file_name);

/*
	Read X bytes from the uncompressed CFILE starting from X offset.
	Returns the amount of bytes read, and 0 or lower to indicate errors.
*/
size_t comp_fread(CFILE* cf, char* buffer, size_t length);

/*
	Returns the current uncompressed CFILE position.
*/
size_t comp_ftell(CFILE* cf);

/*
	Returns 1 if the uncompressed CFILE has been completely read, otherwise it returns a 0.
*/
int comp_feof(CFILE* cf);

/*
	Used to move the uncompressed CFILE current position.
*/
int comp_fseek(CFILE* cf, int offset, int where);

/*
	Use when you dont know if the file is compressed or not
	If it is compressed based on CI data, it will use the comp_ftell function, if not it will use the regular ftell
	function. Returned value is should be the same on both cases.
	Not intended to be used for CFILEs, in that case use they comp_ftell() directly.
*/
size_t comp_compatible_ftell(FILE* fp, COMPRESSION_INFO* ci);

/*
	Use when you dont know if the file is compressed or not
	If it is compressed based on CI data, it will use the comp_fread function, if not it will use the regular fread function.
	Returned value is should be the same on both cases.
	Not intended to be used for CFILEs, in that case use they comp_fread() directly.
*/
size_t comp_compatible_fread(void* dest, size_t elem_size, size_t elem_num, FILE* fp, COMPRESSION_INFO* ci);

/*
	Use when you dont know if the file is compressed or not
	If it is compressed based on CI data, it will use the comp_fseek function, if not it will use the regular fseek
	function. Returned value is should be the same on both cases.
	Not intended to be used for CFILEs, in that case use they comp_fseek() directly.
*/
int comp_compatible_fseek(FILE* fp, long offset, int where, COMPRESSION_INFO* ci);
#endif