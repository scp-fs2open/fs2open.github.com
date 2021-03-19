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

/*LZ41*/
#define LZ41_FILE_HEADER 0x31345A4C // "14ZL" -> "LZ41", inverted
#define LZ41_DECOMPRESSION_ERROR -1
#define LZ41_MAX_BLOCKS_OVERFLOW -2
#define LZ41_HEADER_MISMATCH -3
#define LZ41_OFFSETS_MISMATCH -4
/******/

#define COMP_HEADER_MATCH 1
#define COMP_HEADER_MISMATCH 0

/*
	Returns COMP_HEADER_MATCH if header is a valid compressed file header,
	returns COMP_HEADER_MISMATCH if it dosent.
*/
int comp_check_header(int header);

/*
	This is called to generate the correct compression_info data
	after the file has been indentified as a compressed file.
	This must be done before calling any other function.
*/
void comp_create_ci(CFILE* cf, int header);

/*
	Read X bytes from the uncompressed file starting from X offset.
	Returns the amount of bytes read, and 0 or lower to indicate errors.
*/
size_t comp_fread(CFILE* cf, char* buffer, size_t length);

/*
	Returns the current uncompressed file position.
*/
size_t comp_ftell(CFILE* cf);

/*
	Returns 1 of the uncompressed file has been completely read, otherwise it returns a 0.
*/
int comp_feof(CFILE* cf);

/*
	Used to move the uncompressed file current position.
*/
int comp_fseek(CFILE* cf, int offset, int where);

#endif
