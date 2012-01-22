/*
 * Copyright (C) 2005  Taylor Richards
 *
*/



#include "cfile/cfile.h"
#include "globalincs/pstypes.h"

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>


// /////////////////////////////////////////////////////////////////////////////
//
//  GLOBAL FUNCTIONS AND VARIABLES
//

// right out of pstypes.h but we don't want to use the INTEL_INT macro here
// since it would require SDL which isn't used on WIN32 platforms
#if BYTE_ORDER == BIG_ENDIAN
#define INT_SWAP(x)	(							\
						(x << 24) |					\
						(((ulong)x) >> 24) |		\
						((x & 0x0000ff00) << 8) |	\
						((x & 0x00ff0000) >> 8)		\
						)
#else
#define INT_SWAP(x) (x)
#endif

FILE *fp_in = NULL;
FILE *fp_out = NULL;

#ifndef MAX_PATH
#define MAX_PATH	255
#endif

char out_dir[MAX_PATH];

#define BLOCK_SIZE (1024*1024)

char tmp_data[BLOCK_SIZE];		// 1 MB

static int have_index = 0;
static int have_header = 0;

#define ERR_NO_FP_IN		0
#define ERR_NO_FP_OUT		1
#define ERR_INVALID_VP		2
#define ERR_NO_INDEX		3
#define ERR_NO_HEADER		4
#define ERR_PATH_TOO_LONG	5

typedef struct vp_header {
	char id[4];  // 'VPVP'
	int version;
	int index_offset;  // offset where the file index starts, also a total file data size count
	int num_files;	// number of files, including each step in the directory structure
} vp_header;

vp_header VP_Header;

typedef struct vp_fileinfo {
	// stuff that is actually written to the VP
	int offset;		// offset of where this file is in the VP
	int file_size;	// size of file
	char file_name[CF_MAX_FILENAME_LENGTH];		// filename
	_fs_time_t write_time;	// date/time in _fs_time_t type (a 32-bit version of time_t)

	// not in the VP
	char file_path[CF_MAX_PATHNAME_LENGTH];	// file path, generated here and not actually in the VP on a per file basis

   vp_fileinfo() {
		offset = 0;
		file_size = 0;
		file_name[0] = '\0';
		file_path[0] = '\0';
		write_time = 0;
	}

   ~vp_fileinfo() {}
} vp_fileinfo;

SCP_vector<vp_fileinfo> VP_FileInfo;

// just like strlwr() but without actually including it here
void lowercase(char *s)
{
	if (s == NULL)
		return;

	while (*s) {
		*s = tolower(*s);
		s++;
	}
}

//
// /////////////////////////////////////////////////////////////////////////////

// /////////////////////////////////////////////////////////////////////////////
//
// The useful stuff...
//

void print_error(int err)
{
	switch (err) {
		case ERR_NO_FP_IN:
			printf("ERROR: The input file (fp_in) is NULL!  Exiting...\n");
			break;
		case ERR_NO_FP_OUT:
			printf("ERROR: The output file (fp_out) is NULL!  Exiting...\n");
			break;
		case ERR_INVALID_VP:
			printf("ERROR: The specified VP file is invalid!  Exiting...\n");
			break;
		case ERR_NO_INDEX:
			printf("ERROR: No file index is available!  Exiting...\n");
			break;
		case ERR_NO_HEADER:
			printf("ERROR: No file header is available!  Exiting...\n");
			break;
		case ERR_PATH_TOO_LONG:
			printf("ERROR: Path to output directory is too long!  Exiting...\n");
			break;
		default:
			break;
	}

	exit(1);
}

void read_header()
{
	fseek(fp_in, 0, SEEK_SET);
	fread(&VP_Header.id, 1, 4, fp_in);
	fread(&VP_Header.version, 1, sizeof(int), fp_in);
	fread(&VP_Header.index_offset, 1, sizeof(int), fp_in);
	fread(&VP_Header.num_files, 1, sizeof(int), fp_in);

	VP_Header.version = INT_SWAP(VP_Header.version);
	VP_Header.index_offset = INT_SWAP(VP_Header.index_offset);
	VP_Header.num_files = INT_SWAP(VP_Header.num_files);

	// check for a valid id (VPVP)
	if ( (VP_Header.id[0] != 'V') && (VP_Header.id[1] != 'P') && (VP_Header.id[2] != 'V') && (VP_Header.id[3] != 'P') )
		print_error(ERR_INVALID_VP);

	// the index_offset needs to be greater than the size of vp_header at the very least and there should be at least one file
	if ( !(VP_Header.index_offset > (int)sizeof(vp_header)) || !(VP_Header.num_files > 1) )
		print_error(ERR_INVALID_VP);

	
	have_header = 1;
}

void read_index(int lc = 0)
{
	if (fp_in == NULL)
		print_error(ERR_NO_FP_IN);
	else if (!have_header)
		print_error(ERR_NO_HEADER);

	int i;
	char path[CF_MAX_PATHNAME_LENGTH];

	fseek(fp_in, VP_Header.index_offset, SEEK_SET);

	memset(path, 0, CF_MAX_PATHNAME_LENGTH);

	for ( i = 0; i < VP_Header.num_files; i++) {
		vp_fileinfo vpinfo;

		fread(&vpinfo.offset, 1, sizeof(int), fp_in);
		fread(&vpinfo.file_size, 1, sizeof(int), fp_in);
		fread(&vpinfo.file_name, 1, CF_MAX_FILENAME_LENGTH, fp_in);
		fread(&vpinfo.write_time, 1, sizeof(_fs_time_t), fp_in);

		vpinfo.offset = INT_SWAP(vpinfo.offset);
		vpinfo.file_size = INT_SWAP(vpinfo.file_size);
		vpinfo.write_time = INT_SWAP(vpinfo.write_time);

		// check if it's a directory and if so then create a path to use for files
		if (vpinfo.file_size == 0) {
			// if we get a ".." then drop down in the path
			if (!strcmp(vpinfo.file_name, "..")) {
				char *s = strrchr(path, DIR_SEPARATOR_CHAR);

				if (s)
					*s = '\0';

				// skip, we don't want to add the ".." to the path
				continue;
			}

			// don't add the very first directory separator character, easier to clean later
			if (path[0] != 0)
				strcat( path, DIR_SEPARATOR_STR );

			// always make directory names lower case for ease of use
			lowercase( vpinfo.file_name );

			strcat( path, vpinfo.file_name );
		} // it's a file then
		else {
			// lowercase the filename if wanted
			if (lc == 1)
				lowercase( vpinfo.file_name );

			strcpy_s( vpinfo.file_path, path );
			VP_FileInfo.push_back(vpinfo);
		}
	}

	have_index = 1;
}

void extract_all_files(char *file)
{
	if (fp_in == NULL)
		print_error(ERR_NO_FP_IN);
	else if (!have_header)
		print_error(ERR_NO_HEADER);
	else if (!have_index)
		print_error(ERR_NO_INDEX);

	int status, m_error, nbytes, nbytes_remaining;
	char path[CF_MAX_PATHNAME_LENGTH+CF_MAX_FILENAME_LENGTH+1]; // path length + filename length + extra NULL
	char path2[CF_MAX_PATHNAME_LENGTH+CF_MAX_FILENAME_LENGTH+1]; // path length + filename length + extra NULL
	char *c;
	ubyte have_outdir = 0;

	int out_len = strlen(out_dir);

	if (out_len > 0)
		have_outdir = 1;

	printf("VP file extractor - version 0.6\n");
	printf("\n");

	if (have_outdir) {
		printf("Output directory: \"%s\"\n", out_dir);
	}

	printf("Extracting: %s...\n", file);

	for (uint i = 0; i < VP_FileInfo.size(); i++) {
		// save the file path to a temp location and recursively make the needed directories
		if (have_outdir) {
			if ( (out_len + 1 + strlen(path)) > MAX_PATH-1 )
				print_error(ERR_PATH_TOO_LONG);

			sprintf(path, "%s%s%s%s", out_dir, DIR_SEPARATOR_STR, VP_FileInfo[i].file_path, DIR_SEPARATOR_STR);
		} else {
			sprintf(path, "%s%s", VP_FileInfo[i].file_path, DIR_SEPARATOR_STR);
		}

		c = &path[0];

		while (c++) {
			c = strchr(c, DIR_SEPARATOR_CHAR);

			if (c) {
				*c = '\0';	// NULL at DIR_SEP char

#ifdef _WIN32
				status = _mkdir(path);
#else
				status = mkdir(path, 0777);
#endif

				m_error = errno;

				if (status && (m_error != EEXIST) ) {
					printf("Cannot mkdir %s: %s\n", VP_FileInfo[i].file_path, strerror(m_error));
					continue;
				}

				*c = DIR_SEPARATOR_CHAR;  // replace DIR_SEP char
			}
		}

		memset( path, 0, CF_MAX_PATHNAME_LENGTH+CF_MAX_FILENAME_LENGTH+1);

		// start writing out the file
		fseek(fp_in, VP_FileInfo[i].offset, SEEK_SET);

		sprintf(path, "%s%s%s", VP_FileInfo[i].file_path, DIR_SEPARATOR_STR, VP_FileInfo[i].file_name);

		// this is cheap, I know.
		if (have_outdir) {
			if ( (out_len + 1 + strlen(path)) > MAX_PATH-1 )
				print_error(ERR_PATH_TOO_LONG);

			sprintf(path2, "%s%s%s", out_dir, DIR_SEPARATOR_STR, path);
		} else {
			strcpy_s(path2, path);
		}

		fp_out = fopen(path2, "wb");

		printf("  %s ... ", path);

		if (fp_out == NULL) {
			printf("can't create file!\n");
			continue;
		}

		nbytes_remaining = VP_FileInfo[i].file_size;

		while ( nbytes_remaining > 0 ) {
			nbytes = fread( tmp_data, 1, MIN(BLOCK_SIZE, nbytes_remaining), fp_in );

			if (nbytes > 0) {
				fwrite( tmp_data, 1, nbytes, fp_out );
				nbytes_remaining -= nbytes;
			}
		}

		printf("done!\n");

		fclose(fp_out);
		fp_out = NULL;
	}

	printf("\n");
}

// TODO: the formatting of this is a bit loco but I don't care enough to make it better...
void list_all_files(char *file)
{
	char out_time[20];
	float one_k = 1024.0f;
	float one_m = 1048576.0f;
	char m;
	float div_by;
	time_t plat_time;

	// if we don't have an index then bail
	if (!have_index)
		print_error(ERR_NO_INDEX);

	
	printf("VP file extractor - version 0.6\n");
	printf("\n");
	printf("%s:\n", file);
	printf("\n");

	printf("  Name                          Size     Offset      Date/Time         Path\n");
	printf("-------------------------------------------------------------------------------\n");
	for (uint i = 0; i < VP_FileInfo.size(); i++) {
		plat_time = VP_FileInfo[i].write_time;  // gets rid of some platform strangeness this way
		strftime(out_time, 32, "%F %H:%M", localtime(&plat_time)); // YYYY-mm-dd HH:mm  (ISO 8601 date format, 24-hr time)

		if (VP_FileInfo[i].file_size > (int)one_m) {
			m = 'M';
			div_by = one_m;
		} else {
			m = 'K';
			div_by = one_k;
		}

		int len = strlen( VP_FileInfo[i].file_name );
		printf("%s  %*.1f%c %10i %18s %3s%s\n", VP_FileInfo[i].file_name, 33 - len, (float)VP_FileInfo[i].file_size/div_by, m, VP_FileInfo[i].offset, out_time, DIR_SEPARATOR_STR, VP_FileInfo[i].file_path);
	}

	printf("\n");

	// we use the vector size here since VP_Header.num_files would include each entry
	// in the directory tree as well as individual files and that artificially inflates
	// the number of files that we show or would extract
	printf("Total files: %i\n", (int)VP_FileInfo.size());

	// yeah, I'm just that cheap
	if (VP_Header.index_offset > (int)one_m) {
		m = 'M';
		div_by = one_m;
	} else {
		m = 'K';
		div_by = one_k;
	}

	printf("Total size:  %.1f%c\n", (float)VP_Header.index_offset/div_by, m);

	printf("\n");
}

void help()
{
	printf("VP file extractor - version 0.6\n");
	printf("\n");
	printf("Usage:  cfileextractor [-x | -l] [-L] [-o <dir>] <vp_filename>\n");
	printf("\n");
	printf(" Commands (only one at the time):\n");
	printf("  -x | --extract        Extract all files into current directory.\n");
	printf("  -l | --list           List all files in VP archive.\n");
	printf("  -h | --help           Show this help text.\n");
	printf("\n");
	printf(" Options:\n");
	printf("  -L | --lowercase      Force all filenames to be lower case.\n");
	printf("  -o <dir>              Extract files to <dir> instead of current directory.\n");
	printf("                        NOTE: No spaces allowed, enclose path in \" \" if needed.\n");
	printf("\n");
	printf("  (No command specified will list all files in the VP archive.)\n");
	printf("\n");
}

// we end up #include'ing SDL.h which on Windows and Mac will redefine main() which is something
// that we don't want since we don't actually link against SDL, this solves the problem...
#ifdef main
#undef main
#endif

int main(int argc, char *argv[])
{
	int extract = 0, lc = 0, list = 0;

	if (argc < 2) {
		help();
		return 0;
	}

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		help();
		return 0;
	}

	// the last option should always be the VP file that we want
	fp_in = fopen(argv[argc-1], "rb");

	if (fp_in == NULL)
		print_error(ERR_NO_FP_IN);

	// if the header is invalid then read_header() should exit us out
	read_header();

	memset(out_dir, 0, MAX_PATH);

	// TODO: add filter based extraction
	for (int i = 1; i < argc-1; i++) {
		if (!strcmp(argv[i], "-x") || !strcmp(argv[i], "--extract")) {
			if (list) {
				help();
				exit(0);
			}
			extract = 1;
		} else if (!strcmp(argv[i], "-L") || !strcmp(argv[i], "--lowercase")) {
			lc = 1;
		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")) {
			if (extract) {
				help();
				exit(0);
			}
			list = 1;
		} else if ( !strcmp(argv[i], "-o") && (i+1 < argc) && (argv[i+1][0] != '-') ) {
			strncpy(out_dir, argv[i+1], MAX_PATH-1);
			i++;  // have to increment "i" past the output directory
		} else {
			help();
			return 0;
		}
	}

	// read the file index, make all filenames lowercase if wanted
	read_index( lc );

	if (extract) {
		extract_all_files( argv[argc-1] );
		return 0;
	} else if (list) {
		list_all_files( argv[argc-1] );
		return 0;
	} else {
		// if only -L is specified or no command is specified then just list files
		list_all_files( argv[argc-1] );
		return 0;
	}

	return 0;
}
