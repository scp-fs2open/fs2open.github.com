/*
ShivanSpS - Compressed files support for FSO. Many thanks to ngld, taylor and everyone elsewho helped me in getting this done.

-Audio, movies and ANIs cant be used this way. DO NOT COMPRESS.
-Mission files and tables should stay in a readeable format, you can compress then, but i think it is better this way.
-The minimum size is a optimization, there is no need to compress very small files.
-The file header is a version, this is used to tell FSO how to decompress that file, always use 4 chars to mantain alignment, it is stored at the start of the file. "LZ41" for this implementation.
-The block size is used to tell how much information is compressed into a block, each block adds overhead, so a larger the block bytes result in smaller file size.
................................char[4]..........(n ints)...(int)..........(int)..........(int)
-COMPRESSED FILE DATA STUCTURE: HEADER|N BLOCKS|N OFFSETS|NUM_OFFSETS|ORIGINAL_FILESIZE|BLOCK_SIZE
*/

#ifndef _CFILECOMPRESSION_H
#define _CFILECOMPRESSION_H
struct CFILE;

struct compression_info {
	int header=0;
	int isCompressed = 0;
	size_t compressed_size = 0;
	int block_size = 0;
	int numOffsets = 0;
	int* offsets = nullptr;
	char* decoderBuffer = nullptr;
	int lastDecBlockNum = 0;
	int lastDecBytes = 0;
};


/*LZ41*/
#define LZ41_FILE_HEADER 0x31345A4C //"LZ41", inverted
#define LZ41_DECOMPRESSION_ERROR -1
#define LZ41_MAX_BLOCKS_OVERFLOW -2
#define LZ41_HEADER_MISMATCH -3
#define LZ41_OFFSETS_MISMATCH -4
/******/

#define COMP_HEADER_MATCH 1
#define COMP_HEADER_MISMATCH 0
#define COMP_INVALID_LENGTH_REQUESTED -5

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
size_t comp_fseek(CFILE* cf, int offset, int where);

#endif