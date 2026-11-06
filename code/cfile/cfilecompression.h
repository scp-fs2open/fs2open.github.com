/*
ShivanSpS - Compressed files support for FSO. Many thanks to ngld, taylor and everyone else who helped me in getting this done.

LZ41 FORMAT
--------------------------------------------------------------------------------------
                                  char[4]      (n compressed blocks)   (num_offsets ints)      (int)        (int)              (int)
  COMPRESSED FILE DATA STRUCTURE: HEADER     | BLOCK_0 .. BLOCK_{n-1} | OFFSET_TABLE      | NUM_OFFSETS | ORIGINAL_FILESIZE | BLOCK_SIZE

  * HEADER is a 4-char version tag ("LZ41"). It lets FSO pick the right decoder and lets us add
    future revisions/formats without breaking compatibility. Always 4 chars to keep alignment.
  * Each BLOCK is an *independent* LZ4 block: the compressor resets the HC stream before every
    block, so no block references any other. That is what allows both random access
    and parallel decoding possible.
  * OFFSET_TABLE holds NUM_OFFSETS little-endian ints. offsets[i] is the start of block i within
    the compressed file; offsets[i+1] is its end. The first entry is always 4 (just past the
    header) and the last entry marks the start of the offset table itself. So
    NUM_OFFSETS = (number of blocks) + 1.
  * The 12-byte footer (NUM_OFFSETS, ORIGINAL_FILESIZE, BLOCK_SIZE) is written by the compressor.

  A decoder cache holds the last block decoded by the single-threaded reader.
*/

#ifndef _CFILECOMPRESSION_H
#define _CFILECOMPRESSION_H
struct CFILE;

#define LZ41_FILE_HEADER 0x31345A4C // "14ZL" -> "LZ41", inverted

// Initialize compression info after detecting LZ41. Returns false if the file is invalid.
bool comp_create_ci(CFILE* cf, int header);

/*
	Read X bytes from the uncompressed file starting from the current position.
	Returns the requested length on success or zero on error.
*/
size_t comp_fread(CFILE* cf, char* buffer, size_t length);

/*
	Returns the current uncompressed file position.
*/
size_t comp_ftell(CFILE* cf);

/*
	Returns 1 if the uncompressed file has been completely read, otherwise it returns a 0.
*/
int comp_feof(CFILE* cf);

/*
	Used to move the uncompressed file current position.
*/
int comp_fseek(CFILE* cf, int offset, int where);

#endif
