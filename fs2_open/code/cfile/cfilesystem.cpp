/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>		/* needed for memory mapping of file functions */
#endif

#ifdef SCP_UNIX
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "cmdline/cmdline.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "localization/localize.h"



#define CF_ROOTTYPE_PATH 0
#define CF_ROOTTYPE_PACK 1

// for a defined and specifically set location to get/send pilot/campaign files
char *Pilot_file_path = NULL;

//  Created by:
//    specifying hard drive tree
//    searching for pack files on hard drive		// Found by searching all known paths
//    specifying cd-rom tree
//    searching for pack files on CD-rom tree
typedef struct cf_root {
	char				path[CF_MAX_PATHNAME_LENGTH];		// Contains something like c:\projects\freespace or c:\projects\freespace\freespace.vp
	int				roottype;								// CF_ROOTTYPE_PATH  = Path, CF_ROOTTYPE_PACK =Pack file
} cf_root;

// convenient type for sorting (see cf_build_pack_list())
typedef struct cf_root_sort { 
	char				path[CF_MAX_PATHNAME_LENGTH];
	int				roottype;
	int				cf_type;
} cf_root_sort;

#define CF_NUM_ROOTS_PER_BLOCK   32
#define CF_MAX_ROOT_BLOCKS			256				// Can store 32*256 = 8192 Roots
#define CF_MAX_ROOTS					(CF_NUM_ROOTS_PER_BLOCK * CF_MAX_ROOT_BLOCKS)

typedef struct cf_root_block {
	cf_root			roots[CF_NUM_ROOTS_PER_BLOCK];
} cf_root_block;

static int Num_roots = 0;
static cf_root_block  *Root_blocks[CF_MAX_ROOT_BLOCKS];

static int Num_path_roots = 0;

// Created by searching all roots in order.   This means Files is then sorted by precedence.
typedef struct cf_file {
	char		name_ext[CF_MAX_FILENAME_LENGTH];		// Filename and extension
	int		root_index;										// Where in Roots this is located
	int		pathtype_index;								// Where in Paths this is located
	time_t	write_time;										// When it was last written
	int		size;												// How big it is in bytes
	int		pack_offset;									// For pack files, where it is at.   0 if not in a pack file.  This can be used to tell if in a pack file.
} cf_file;

#define CF_NUM_FILES_PER_BLOCK   512
#define CF_MAX_FILE_BLOCKS			128						// Can store 512*128 = 65536 files

typedef struct cf_file_block {
	cf_file						files[CF_NUM_FILES_PER_BLOCK];
} cf_file_block;

static uint Num_files = 0;
static cf_file_block  *File_blocks[CF_MAX_FILE_BLOCKS];


// Return a pointer to to file 'index'.
cf_file *cf_get_file(int index)
{
	int block = index / CF_NUM_FILES_PER_BLOCK;
	int offset = index % CF_NUM_FILES_PER_BLOCK;

	return &File_blocks[block]->files[offset];
}

// Create a new file and return a pointer to it.
cf_file *cf_create_file()
{
	Assertion(Num_files < CF_NUM_FILES_PER_BLOCK * CF_MAX_FILE_BLOCKS, "Too many files found. CFile cannot handle more than %d files.\n", CF_NUM_FILES_PER_BLOCK * CF_MAX_FILE_BLOCKS);

	uint block = Num_files / CF_NUM_FILES_PER_BLOCK;
	uint offset = Num_files % CF_NUM_FILES_PER_BLOCK;
	
	if ( File_blocks[block] == NULL )	{
		File_blocks[block] = (cf_file_block *)vm_malloc( sizeof(cf_file_block) );
		Assert( File_blocks[block] != NULL);
	}

	Num_files++;

	return &File_blocks[block]->files[offset];
}

extern int cfile_inited;

// Create a new root and return a pointer to it.  The structure is assumed unitialized.
cf_root *cf_get_root(int n)
{
	int block = n / CF_NUM_ROOTS_PER_BLOCK;
	int offset = n % CF_NUM_ROOTS_PER_BLOCK;

	if (!cfile_inited)
		return NULL;

	return &Root_blocks[block]->roots[offset];
}


// Create a new root and return a pointer to it.  The structure is assumed unitialized.
cf_root *cf_create_root()
{
	int block = Num_roots / CF_NUM_ROOTS_PER_BLOCK;
	int offset = Num_roots % CF_NUM_ROOTS_PER_BLOCK;
	
	if ( Root_blocks[block] == NULL )	{
		Root_blocks[block] = (cf_root_block *)vm_malloc( sizeof(cf_root_block) );
		Assert(Root_blocks[block] != NULL);
	}

	Num_roots++;

	return &Root_blocks[block]->roots[offset];
}

// return the # of packfiles which exist
int cf_get_packfile_count(cf_root *root)
{
	SCP_string filespec;
	int i;
	int packfile_count;

	// count up how many packfiles we're gonna have
	packfile_count = 0;
	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++ )
	{
		filespec = root->path;
		
		if(strlen(Pathtypes[i].path))
		{
			filespec += Pathtypes[ i ].path;
			if ( filespec[ filespec.length( ) - 1 ] != DIR_SEPARATOR_CHAR )
			{
				filespec += DIR_SEPARATOR_STR;
			}
		}

#if defined _WIN32
		filespec += "*.vp";

		int find_handle;
		_finddata_t find;
		
		find_handle = _findfirst( filespec.c_str( ), &find );

 		if (find_handle != -1) {
			do {
				if (!(find.attrib & _A_SUBDIR)) {
					packfile_count++;
				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}	
#elif defined SCP_UNIX
		filespec += "*.[vV][pP]";

		glob_t globinfo;
		memset(&globinfo, 0, sizeof(globinfo));
		int status = glob(filespec.c_str( ), 0, NULL, &globinfo);
		if (status == 0) {
			for (unsigned int j = 0;  j < globinfo.gl_pathc;  j++) {
				// Determine if this is a regular file
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				stat(globinfo.gl_pathv[j], &statbuf);
				if (S_ISREG(statbuf.st_mode)) {
					packfile_count++;
				}
			}
			globfree(&globinfo);
		}
#endif
	}

	return packfile_count;
}

// packfile sort function
int cf_packfile_sort_func(const void *elem1, const void *elem2)
{
	cf_root_sort *r1, *r2;
	r1 = (cf_root_sort*)elem1;
	r2 = (cf_root_sort*)elem2;

	// if the 2 directory types are the same, do a string compare
	if(r1->cf_type == r2->cf_type){
		return stricmp(r1->path, r2->path);
	}

	// otherwise return them in order of CF_TYPE_* precedence
	return (r1->cf_type < r2->cf_type) ? -1 : 1;
}

// Go through a root and look for pack files
void cf_build_pack_list( cf_root *root )
{
	char filespec[MAX_PATH_LEN];
	int i;
	cf_root_sort *temp_roots_sort, *rptr_sort;
	int temp_root_count, root_index;

	// determine how many packfiles there are
	temp_root_count = cf_get_packfile_count(root);

	if (temp_root_count <= 0)
		return;

	// allocate a temporary array of temporary roots so we can easily sort them
	temp_roots_sort = (cf_root_sort*)vm_malloc(sizeof(cf_root_sort) * temp_root_count);

	if (temp_roots_sort == NULL) {
		Int3();
		return;
	}

	// now just setup all the root info
	root_index = 0;
	for (i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++) {
		strcpy_s( filespec, root->path );
		
		if ( strlen(Pathtypes[i].path) ) {
			strcat_s( filespec, Pathtypes[i].path );

			if ( filespec[strlen(filespec)-1] != DIR_SEPARATOR_CHAR )
				strcat_s( filespec, DIR_SEPARATOR_STR );
		}

#if defined _WIN32
		strcat_s( filespec, "*.vp" );

		int find_handle;
		_finddata_t find;
		
		find_handle = _findfirst( filespec, &find );

 		if (find_handle != -1) {
			do {
				// add the new item
				if (!(find.attrib & _A_SUBDIR)) {					
					Assert(root_index < temp_root_count);

					// get a temp pointer
					rptr_sort = &temp_roots_sort[root_index++];

					// fill in all the proper info
					strcpy_s(rptr_sort->path, root->path);
					
					if(strlen(Pathtypes[i].path)) {

						strcat_s(rptr_sort->path, Pathtypes[i].path );
						strcat_s(rptr_sort->path, DIR_SEPARATOR_STR);
					}
					
					strcat_s(rptr_sort->path, find.name );
					rptr_sort->roottype = CF_ROOTTYPE_PACK;
					rptr_sort->cf_type = i;
				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}	
#elif defined SCP_UNIX
		strcat_s( filespec, "*.[vV][pP]" );
		glob_t globinfo;

		memset(&globinfo, 0, sizeof(globinfo));

		int status = glob(filespec, 0, NULL, &globinfo);

		if (status == 0) {
			for (uint j = 0;  j < globinfo.gl_pathc;  j++) {
				// Determine if this is a regular file
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				stat(globinfo.gl_pathv[j], &statbuf);

				if ( S_ISREG(statbuf.st_mode) ) {
					Assert(root_index < temp_root_count);

					// get a temp pointer
					rptr_sort = &temp_roots_sort[root_index++];

					// fill in all the proper info
					strcpy_s(rptr_sort->path, globinfo.gl_pathv[j] );
					rptr_sort->roottype = CF_ROOTTYPE_PACK;
					rptr_sort->cf_type = i;
				}
			}

			globfree(&globinfo);
		}
#endif
	}

	// these should always be the same
	Assert(root_index == temp_root_count);

	// sort the roots
	qsort(temp_roots_sort, temp_root_count, sizeof(cf_root_sort), cf_packfile_sort_func);

	// now insert them all into the real root list properly
	cf_root *new_root;
	for (i = 0; i < temp_root_count; i++) {
		new_root = cf_create_root();
		strcpy_s( new_root->path, root->path );

#ifndef NDEBUG
		uint chksum = 0;
		cf_chksum_pack(temp_roots_sort[i].path, &chksum);
		mprintf(("Found root pack '%s' with a checksum of 0x%08x\n", temp_roots_sort[i].path, chksum));
#endif

		// mwa -- 4/2/98 put in the next 2 lines because the path name needs to be there
		// to find the files.
		strcpy_s(new_root->path, temp_roots_sort[i].path);		
		new_root->roottype = CF_ROOTTYPE_PACK;		
	}

	// free up the temp list
	vm_free(temp_roots_sort);
}


void cf_build_root_list(char *cdrom_dir)
{
	Num_roots = 0;
	Num_path_roots = 0;

	cf_root	*root;
	char str_temp[CF_MAX_PATHNAME_LENGTH], *cur_pos;

#ifdef SCP_UNIX
	// =========================================================================
	// now look for mods under the users HOME directory to use before system ones
	if (Cmdline_mod) {
		for (cur_pos=Cmdline_mod; strlen(cur_pos) != 0; cur_pos+= (strlen(cur_pos)+1))
		{
			memset(str_temp, 0, CF_MAX_PATHNAME_LENGTH);
			strncpy(str_temp, cur_pos, CF_MAX_PATHNAME_LENGTH-1);

			strncat(str_temp, DIR_SEPARATOR_STR, (CF_MAX_PATHNAME_LENGTH - strlen(str_temp) - 1));

			// truncated string check
			if ( (strlen(Cfile_user_dir) + strlen(str_temp) + 1) >= CF_MAX_PATHNAME_LENGTH ) {
				Int3();
			}

			root = cf_create_root();

			strncpy( root->path, Cfile_user_dir, CF_MAX_PATHNAME_LENGTH-1 );

			// do we already have a slash? as in the case of a root directory install
			if ( (strlen(root->path) < (CF_MAX_PATHNAME_LENGTH-1)) && (root->path[strlen(root->path)-1] != DIR_SEPARATOR_CHAR) ) {
				strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
			}

			strncat(root->path, str_temp, (CF_MAX_PATHNAME_LENGTH - strlen(root->path) - 1));

			root->roottype = CF_ROOTTYPE_PATH;
			cf_build_pack_list(root);
		}
	}
	// =========================================================================

	// =========================================================================
	// set users HOME directory as default for loading and saving files
	root = cf_create_root();
	strncpy( root->path, Cfile_user_dir, CF_MAX_PATHNAME_LENGTH-1 );

	// do we already have a slash? as in the case of a root directory install
	if( (strlen(root->path) < (CF_MAX_PATHNAME_LENGTH-1)) && (root->path[strlen(root->path)-1] != DIR_SEPARATOR_CHAR) ) {
		strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
	}
	root->roottype = CF_ROOTTYPE_PATH;

	// set the default player location to here
	if ( Pilot_file_path == NULL )
		Pilot_file_path = root->path;

	// Next, check any VP files under the current directory.
	cf_build_pack_list(root);
	// =========================================================================
#endif

	if(Cmdline_mod) {
		// stackable Mod support -- Kazan
		//This for statement is a work of art :D
		for (cur_pos=Cmdline_mod; strlen(cur_pos) != 0; cur_pos+= (strlen(cur_pos)+1))
		{
			memset(str_temp, 0, CF_MAX_PATHNAME_LENGTH);
			strncpy(str_temp, cur_pos, CF_MAX_PATHNAME_LENGTH-1);

			strncat(str_temp, DIR_SEPARATOR_STR, (CF_MAX_PATHNAME_LENGTH - strlen(str_temp) - 1));
			root = cf_create_root();

			if ( !_getcwd(root->path, CF_MAX_PATHNAME_LENGTH ) ) {
				Error(LOCATION, "Can't get current working directory -- %d", errno );
			}

			// truncated string check
			if ( (strlen(root->path) + strlen(str_temp) + 1) >= CF_MAX_PATHNAME_LENGTH ) {
				Int3();
			}

			// do we already have a slash? as in the case of a root directory install
			if ( (strlen(root->path) < (CF_MAX_PATHNAME_LENGTH-1)) && (root->path[strlen(root->path)-1] != DIR_SEPARATOR_CHAR) ) {
				strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
			}

			strncat(root->path, str_temp, (CF_MAX_PATHNAME_LENGTH - strlen(root->path) - 1));
			root->roottype = CF_ROOTTYPE_PATH;
			cf_build_pack_list(root);
		}
	}

	root = cf_create_root();
	
	if ( !_getcwd(root->path, CF_MAX_PATHNAME_LENGTH ) ) {
		Error(LOCATION, "Can't get current working directory -- %d", errno );
	}

	// do we already have a slash? as in the case of a root directory install
	if ( (strlen(root->path) < (CF_MAX_PATHNAME_LENGTH-1)) && (root->path[strlen(root->path)-1] != DIR_SEPARATOR_CHAR) ) {
		strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
	}

	root->roottype = CF_ROOTTYPE_PATH;

	// set the default path for pilot files
	if ( Pilot_file_path == NULL )
		Pilot_file_path = root->path;

   //======================================================
	// Next, check any VP files under the current directory.
	cf_build_pack_list(root);


   //======================================================
	// Check the real CD if one...
	if ( cdrom_dir && (strlen(cdrom_dir) < CF_MAX_PATHNAME_LENGTH) )	{
		root = cf_create_root();
		strcpy_s( root->path, cdrom_dir );
		root->roottype = CF_ROOTTYPE_PATH;

		//======================================================
		// Next, check any VP files in the CD-ROM directory.
		cf_build_pack_list(root);

	}

}

// Given a lower case list of file extensions 
// separated by spaces, return zero if ext is
// not in the list.
int is_ext_in_list( char *ext_list, char *ext )
{
	char tmp_ext[128];

	strncpy( tmp_ext, ext, 127 );
	strlwr(tmp_ext);
	if ( strstr(ext_list, tmp_ext ))	{
		return 1;
	}	

	return 0;
}

void cf_search_root_path(int root_index)
{
	int i;
	int num_files = 0;

	cf_root *root = cf_get_root(root_index);

	mprintf(( "Searching root '%s' ... ", root->path ));

	char search_path[CF_MAX_PATHNAME_LENGTH];

	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++ )	{

		// we don't want to add player files to the cache - taylor
		if ( (i == CF_TYPE_SINGLE_PLAYERS) || (i == CF_TYPE_MULTI_PLAYERS) ) {
			continue;
		}

		strcpy_s( search_path, root->path );

		if(strlen(Pathtypes[i].path)) {
			strcat_s( search_path, Pathtypes[i].path );
			if ( search_path[strlen(search_path)-1] != DIR_SEPARATOR_CHAR ) {
				strcat_s( search_path, DIR_SEPARATOR_STR );
			}
		} 

#if defined _WIN32
		strcat_s( search_path, "*.*" );

		int find_handle;
		_finddata_t find;
		
		find_handle = _findfirst( search_path, &find );

 		if (find_handle != -1) {
			do {
				if (!(find.attrib & _A_SUBDIR)) {

					char *ext = strrchr( find.name, '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[i].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();

							strcpy_s( file->name_ext, find.name );
							file->root_index = root_index;
							file->pathtype_index = i;
							file->write_time = find.time_write;
							file->size = find.size;
							file->pack_offset = 0;			// Mark as a non-packed file

							num_files++;
							//mprintf(( "Found file '%s'\n", file->name_ext ));
						}
					}

				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}
#elif defined SCP_UNIX
		DIR *dirp;
		struct dirent *dir;

		dirp = opendir (search_path);
		if ( dirp ) {
			while ((dir = readdir (dirp)) != NULL)
			{
				if (!fnmatch ("*.*", dir->d_name, 0))
				{
					char fn[MAX_PATH];
					snprintf(fn, MAX_PATH-1, "%s%s", search_path, dir->d_name);
					fn[MAX_PATH-1] = 0;

					struct stat buf;
					if (stat(fn, &buf) == -1) {
						continue;
					}
					
					if (!S_ISREG(buf.st_mode)) {
						continue;
					}
					
					char *ext = strrchr( dir->d_name, '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[i].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();

							strcpy_s( file->name_ext, dir->d_name );
							file->root_index = root_index;
							file->pathtype_index = i;


							file->write_time = buf.st_mtime;
							file->size = buf.st_size;

							file->pack_offset = 0;			// Mark as a non-packed file

							num_files++;
							//mprintf(( "Found file '%s'\n", file->name_ext ));
						}
					}
				}
			}
			closedir(dirp);
		}
#endif
	}

	mprintf(( "%i files\n", num_files ));
}


typedef struct VP_FILE_HEADER {
	char id[4];
	int version;
	int index_offset;
	int num_files;
} VP_FILE_HEADER;

typedef struct VP_FILE {
	int	offset;
	int	size;
	char	filename[32];
	_fs_time_t write_time;
} VP_FILE;

void cf_search_root_pack(int root_index)
{
	int num_files = 0;
	cf_root *root = cf_get_root(root_index);

	Assert( root != NULL );

	// Open data		
	FILE *fp = fopen( root->path, "rb" );
	// Read the file header
	if (!fp) {
		return;
	}

	if ( filelength(fileno(fp)) < (int)(sizeof(VP_FILE_HEADER) + (sizeof(int) * 3)) ) {
		mprintf(( "Skipping VP file ('%s') of invalid size...\n", root->path ));
		fclose(fp);
		return;
	}

	VP_FILE_HEADER VP_header;

	Assert( sizeof(VP_header) == 16 );
	fread(&VP_header, 1, sizeof(VP_header), fp);

	VP_header.version = INTEL_INT( VP_header.version );
	VP_header.index_offset = INTEL_INT( VP_header.index_offset );
	VP_header.num_files = INTEL_INT( VP_header.num_files );

	mprintf(( "Searching root pack '%s' ... ", root->path ));

	// Read index info
	fseek(fp, VP_header.index_offset, SEEK_SET);

	char search_path[CF_MAX_PATHNAME_LENGTH];

	strcpy_s( search_path, "" );
	
	// Go through all the files
	int i;
	for (i=0; i<VP_header.num_files; i++ )	{
		VP_FILE find;

		fread( &find, sizeof(VP_FILE), 1, fp );

		find.offset = INTEL_INT( find.offset );
		find.size = INTEL_INT( find.size );
		find.write_time = INTEL_INT( find.write_time );

		if ( find.size == 0 )	{
			if ( !stricmp( find.filename, ".." ))	{
				int l = strlen(search_path);
				char *p = &search_path[l-1];
				while( (p > search_path) && (*p != DIR_SEPARATOR_CHAR) )	{
					p--;
				}
				*p = 0;
			} else {
				if ( strlen(search_path) && (search_path[strlen(search_path)-1] != DIR_SEPARATOR_CHAR) ) {
						strcat_s( search_path, DIR_SEPARATOR_STR );
				}
				strcat_s( search_path, find.filename );
			}

			//mprintf(( "Current dir = '%s'\n", search_path ));
		} else {
	
			int j;			
							
			for (j=CF_TYPE_ROOT; j<CF_MAX_PATH_TYPES; j++ )	{
				
				if ( !stricmp( search_path, Pathtypes[j].path ))	{
					char *ext = strrchr( find.filename, '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[j].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();
							strcpy_s( file->name_ext, find.filename );
							file->root_index = root_index;
							file->pathtype_index = j;
							file->write_time = (time_t)find.write_time;
							file->size = find.size;
							file->pack_offset = find.offset;			// Mark as a packed file

							num_files++;
							//mprintf(( "Found pack file '%s'\n", file->name_ext ));
						}
					}
				}
			}
		}
	}

	fclose(fp);

	mprintf(( "%i files\n", num_files ));
}



void cf_build_file_list()
{
	int i;

	Num_files = 0;

	// For each root, find all files...
	for (i=0; i<Num_roots; i++ )	{
		cf_root	*root = cf_get_root(i);
		if ( root->roottype == CF_ROOTTYPE_PATH )	{
			cf_search_root_path(i);
		} else if ( root->roottype == CF_ROOTTYPE_PACK )	{
			cf_search_root_pack(i);
		}
	}

}


void cf_build_secondary_filelist(char *cdrom_dir)
{
	int i;

	// Assume no files
	Num_roots = 0;
	Num_files = 0;

	// Init the root blocks
	for (i=0; i<CF_MAX_ROOT_BLOCKS; i++ )	{
		Root_blocks[i] = NULL;
	}

	// Init the file blocks	
	for (i=0; i<CF_MAX_FILE_BLOCKS; i++ )	{
		File_blocks[i] = NULL;
	}

	mprintf(( "Building file index...\n" ));
	
	// build the list of searchable roots
	cf_build_root_list(cdrom_dir);	

	// build the list of files themselves
	cf_build_file_list();

	mprintf(( "Found %d roots and %d files.\n", Num_roots, Num_files ));
}

void cf_free_secondary_filelist()
{
	int i;

	// Free the root blocks
	for (i=0; i<CF_MAX_ROOT_BLOCKS; i++ )	{
		if ( Root_blocks[i] )	{
			vm_free( Root_blocks[i] );
			Root_blocks[i] = NULL;
		}
	}
	Num_roots = 0;

	// Init the file blocks	
	for (i=0; i<CF_MAX_FILE_BLOCKS; i++ )	{
		if ( File_blocks[i] )	{
			vm_free( File_blocks[i] );
			File_blocks[i] = NULL;
		}
	}
	Num_files = 0;
}

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.
// Input:  filespace   - Filename & extension
//         pathtype    - See CF_TYPE_ defines in CFILE.H
//         max_out     - Maximum string length that should be stuffed into pack_filename
// Output: pack_filename - Absolute path and filename of this file.   Could be a packfile or the actual file.
//         size        - File size
//         offset      - Offset into pack file.  0 if not a packfile.
// Returns: If not found returns 0.
int cf_find_file_location( char *filespec, int pathtype, int max_out, char *pack_filename, int *size, int *offset, bool localize )
{
	int cfs_slow_search = 0;
	char longname[MAX_PATH_LEN];

#if defined WIN32
	long findhandle;
	_finddata_t findstruct;
#endif

	Assert( (filespec != NULL) && (strlen(filespec) > 0) );
	Assert( (pack_filename == NULL) || (max_out > 1) );

	// see if we have something other than just a filename
	// our current rules say that any file that specifies a direct
	// path will try to be opened on that path.  If that open
	// fails, then we will open the file based on the extension
	// of the file

	// NOTE: full path should also include localization, if so desired
#ifdef SCP_UNIX
	if ( strpbrk(filespec, "/") ) {			// do we have a full path already?
#else
	if ( strpbrk(filespec,"/\\:")  ) {		// do we have a full path already?
#endif
		FILE *fp = fopen(filespec, "rb" );
		if (fp)	{
			if ( size ) *size = filelength(fileno(fp));
			if ( offset ) *offset = 0;
			if ( pack_filename ) {
				strncpy( pack_filename, filespec, max_out );
			}				
			fclose(fp);
			return 1;		
		}

		return 0;		// If they give a full path, fail if not found.
	}

	// Search the hard drive for files first.
	int num_search_dirs = 0;
	int search_order[CF_MAX_PATH_TYPES];

	if ( CF_TYPE_SPECIFIED(pathtype) )	{
		search_order[num_search_dirs++] = pathtype;
	} else {
		for (int i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++) {
			if (i != pathtype)
				search_order[num_search_dirs++] = i;
		}
	}

	memset( longname, 0, sizeof(longname) );


	for (int i=0; i<num_search_dirs; i++ )	{
		switch (search_order[i])
		{
			case CF_TYPE_ROOT:
			case CF_TYPE_DATA:
			case CF_TYPE_SINGLE_PLAYERS:
			case CF_TYPE_MULTI_PLAYERS:
			case CF_TYPE_MULTI_CACHE:
			case CF_TYPE_MISSIONS:
			case CF_TYPE_CACHE:
				cfs_slow_search = 1;
				break;
 
			default:
				// always hit the disk if we are looking in only one path
				cfs_slow_search = (num_search_dirs == 1) ? 1 : 0;
				break;
		}
 
		if (cfs_slow_search) {
			cf_create_default_path_string( longname, sizeof(longname)-1, search_order[i], filespec, localize );

#if defined _WIN32
			if (!Cmdline_safeloading) {
				findhandle = _findfirst(longname, &findstruct);
				if (findhandle != -1) {
					if (size)
						*size = findstruct.size;

					_findclose(findhandle);

					if (offset)
						*offset = 0;

					if (pack_filename)
						strncpy( pack_filename, longname, max_out );

					return 1;
				}
			} else
#endif
			{
				FILE *fp = fopen(longname, "rb" );

				if (fp) {
					if (size)
						*size = filelength( fileno(fp) );

					fclose(fp);

					if (offset)
						*offset = 0;

					if (pack_filename)
						strncpy(pack_filename, longname, max_out);

					return 1;
				}
			}
		}
	}

	// Search the pak files and CD-ROM.

	for (uint i = 0; i < Num_files; i++ )	{
		cf_file *f = cf_get_file(i);

		// only search paths we're supposed to...
		if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index) )
			continue;


		if (localize) {
			// create localized filespec
			strncpy(longname, filespec, MAX_PATH_LEN - 1);

			if ( lcl_add_dir_to_path_with_filename(longname, MAX_PATH_LEN - 1) ) {
				if ( !stricmp(longname, f->name_ext) ) {
					if (size)
						*size = f->size;

					if (offset)
						*offset = f->pack_offset;

					if (pack_filename) {
						cf_root *r = cf_get_root(f->root_index);

						strncpy( pack_filename, r->path, max_out );

						if (f->pack_offset < 1) {
							strcat_s( pack_filename, max_out, Pathtypes[f->pathtype_index].path );

							if ( pack_filename[strlen(pack_filename)-1] != DIR_SEPARATOR_CHAR )
								strcat_s( pack_filename, max_out, DIR_SEPARATOR_STR );

							strcat_s( pack_filename, max_out, f->name_ext );
						}
					}

					return 1;
				}
			}
		}

		// file either not localized or localized version not found
		if ( !stricmp(filespec, f->name_ext) ) {
			if (size)
				*size = f->size;

			if (offset)
				*offset = f->pack_offset;

			if (pack_filename) {
				cf_root *r = cf_get_root(f->root_index);

				strcpy( pack_filename, r->path );

				if (f->pack_offset < 1) {
					if ( strlen(Pathtypes[f->pathtype_index].path) ) {
						strcat_s( pack_filename, max_out, Pathtypes[f->pathtype_index].path );

						if ( pack_filename[strlen(pack_filename)-1] != DIR_SEPARATOR_CHAR )
							strcat_s( pack_filename, max_out, DIR_SEPARATOR_STR );
					}

					strcat_s( pack_filename, max_out, f->name_ext );
				}
			}

			return 1;
		}
	}
		
	return 0;
}

// -- from parselo.cpp --
extern char *stristr(const char *str, const char *substr);

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.  Searches all locations in order for first filename using filter list.
// Input:  filename    - Filename & extension
//         ext_num     - number of extensions to look for
//         ext_list    - extension filter list
//         pathtype    - See CF_TYPE_ defines in CFILE.H
//         max_out     - Maximum string length that should be stuffed into pack_filename
// Output: pack_filename - Absolute path and filename of this file.   Could be a packfile or the actual file.
//         size        - File size
//         offset      - Offset into pack file.  0 if not a packfile.
// Returns: If not found returns -1, else returns offset into ext_list.
// (NOTE: This function is exponentially slow, so don't use it unless truely needed!!)
int cf_find_file_location_ext( char *filename, const int ext_num, const char **ext_list, int pathtype, int max_out, char *pack_filename, int *size, int *offset, bool localize )
{
	int cur_ext;
	int cfs_slow_search = 0;
	char longname[MAX_PATH_LEN];
	char filespec[MAX_FILENAME_LEN];
	char *p = NULL;

#if defined WIN32
	long findhandle;
	_finddata_t findstruct;
#endif

	Assert( (filename != NULL) && (strlen(filename) < MAX_FILENAME_LEN) );
	Assert( (ext_list != NULL) && (ext_num > 1) );	// if we are searching for just one ext
													// then this is the wrong function to use
	Assert( (pack_filename == NULL) || (max_out > 1) );


	// if we have a full path already then fail.  this function if for searching via filter only!
#ifdef SCP_UNIX
	if ( strpbrk(filename, "/") ) {			// do we have a full path already?
#else
	if ( strpbrk(filename,"/\\:")  ) {		// do we have a full path already?
#endif
		Int3();
		return 0;
	}

	// Search the hard drive for files first.
	int num_search_dirs = 0;
	int search_order[CF_MAX_PATH_TYPES];

	if ( CF_TYPE_SPECIFIED(pathtype) )	{
		search_order[num_search_dirs++] = pathtype;
	} else {
		for (int i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++)
			search_order[num_search_dirs++] = i;
	}

	memset( longname, 0, sizeof(longname) );
	memset( filespec, 0, sizeof(filespec) );

	// strip any existing extension
	strncpy(filespec, filename, MAX_FILENAME_LEN-1);

	for (int i = 0; i < num_search_dirs; i++) {
		// always hit the disk if we are looking in only one path
		if (num_search_dirs == 1) {
			cfs_slow_search = 1;
		}
		// otherwise hit based on a directory type
		else {
			switch (search_order[i])
			{
				case CF_TYPE_ROOT:
				case CF_TYPE_DATA:
				case CF_TYPE_SINGLE_PLAYERS:
				case CF_TYPE_MULTI_PLAYERS:
				case CF_TYPE_MULTI_CACHE:
				case CF_TYPE_MISSIONS:
				case CF_TYPE_CACHE:
					cfs_slow_search = 1;
					break;
			}
		}

		if ( !cfs_slow_search )
			continue;

		for (cur_ext = 0; cur_ext < ext_num; cur_ext++) {
			// strip any extension and add the one we want to check for
			// (NOTE: to be fully retail compatible, we need to support multiple periods for something like *_1.5.wav,
			//        which means that we need to strip a length of >2 only, assuming that all valid ext are at least 2 chars)
			p = strrchr(filespec, '.');
			if ( p && (strlen(p) > 2) )
				(*p) = 0;

			strcat_s( filespec, ext_list[cur_ext] );
 
			cf_create_default_path_string( longname, sizeof(longname)-1, search_order[i], filespec, localize );

#if defined _WIN32
			if (!Cmdline_safeloading) {
				findhandle = _findfirst(longname, &findstruct);
				if (findhandle != -1) {
					if (size)
						*size = findstruct.size;

					_findclose(findhandle);

					if (offset)
						*offset = 0;

					if (pack_filename)
						strncpy( pack_filename, longname, max_out );

					return cur_ext;
				}
			} else
#endif
			{
				FILE *fp = fopen(longname, "rb" );

				if (fp) {
					if (size)
						*size = filelength( fileno(fp) );

					fclose(fp);

					if (offset)
						*offset = 0;

					if (pack_filename)
						strncpy(pack_filename, longname, max_out);

					return cur_ext;
				}
			}
		}
	}

	// Search the pak files and CD-ROM.

	// first off, make sure that we don't have an extension
	// (NOTE: to be fully retail compatible, we need to support multiple periods for something like *_1.5.wav,
	//        which means that we need to strip a length of >2 only, assuming that all valid ext are at least 2 chars)
	p = strrchr(filespec, '.');
	if ( p && (strlen(p) > 2) )
		(*p) = 0;

	// go ahead and get our length, which is used to test with later
	int filespec_len = strlen(filespec);

	// get total legnth, with extension, which is iused to test with later
	// (FIXME: this assumes that everything in ext_list[] is the same length!)
	uint filespec_len_big = filespec_len + strlen(ext_list[0]);

	SCP_vector< cf_file* > file_list_index;
	int last_root_index = -1;
	int last_path_index = -1;

	file_list_index.reserve( MIN(ext_num * 4, (int)Num_files) );

	// next, run though and pick out base matches
	for (uint i = 0; i < Num_files; i++) {
		cf_file *f = cf_get_file(i);

		// ... only search paths that we're supposed to
		if ( (num_search_dirs == 1) && (pathtype != f->pathtype_index) )
			continue;

		// ... check that our names are the same length (accounting for the missing extension on our own name)
		if ( strlen(f->name_ext) != filespec_len_big )
			continue;

		// ... check that we match the base filename
		if ( strnicmp(f->name_ext, filespec, filespec_len) )
			continue;

		// ... make sure that it's one of our supported types
		bool found_one = false;
		for (cur_ext = 0; cur_ext < ext_num; cur_ext++) {
			if ( stristr(f->name_ext, ext_list[cur_ext]) ) {
				found_one = true;
				break;
			}
		}

		if ( !found_one )
			continue;

		// ... we check based on location, so if location changes after the first find then bail
		if (last_root_index == -1) {
			last_root_index = f->root_index;
			last_path_index = f->pathtype_index;
		} else {
			if (f->root_index != last_root_index)
				break;

			if (f->pathtype_index != last_path_index)
				break;
		}

		// ok, we have a good base match, so add it to our cache
		file_list_index.push_back( f );
	}

	size_t file_list_size = file_list_index.size();

	// quick exit test
	if (file_list_size < 1)
		goto Bail;


	// now try and find our preferred match
	for (cur_ext = 0; cur_ext < ext_num; cur_ext++) {
		for (size_t i = 0; i < file_list_size; i++) {
			cf_file *f = file_list_index[i];
	
			strcat_s( filespec, ext_list[cur_ext] );

			if (localize) {
				// create localized filespec
				strncpy(longname, filespec, MAX_PATH_LEN - 1);

				if ( lcl_add_dir_to_path_with_filename(longname, MAX_PATH_LEN - 1) ) {
					if ( !stricmp(longname, f->name_ext) ) {
						if (size)
							*size = f->size;

						if (offset)
							*offset = f->pack_offset;

						if (pack_filename) {
							cf_root *r = cf_get_root(f->root_index);

							strncpy( pack_filename, r->path, max_out );

							if (f->pack_offset < 1) {
								strcat_s( pack_filename, max_out, Pathtypes[f->pathtype_index].path );

								if ( pack_filename[strlen(pack_filename)-1] != DIR_SEPARATOR_CHAR )
									strcat_s( pack_filename, max_out, DIR_SEPARATOR_STR );

								strcat_s( pack_filename, max_out, f->name_ext );
							}
						}

						// found it, so cleanup and return
						file_list_index.clear();

						return cur_ext;
					}
				}
			}

			// file either not localized or localized version not found
			if ( !stricmp(filespec, f->name_ext) ) {
				if (size)
					*size = f->size;

				if (offset)
					*offset = f->pack_offset;

				if (pack_filename) {
					cf_root *r = cf_get_root(f->root_index);

					strcpy( pack_filename, r->path );

					if (f->pack_offset < 1) {

						if ( strlen(Pathtypes[f->pathtype_index].path) ) {
							strcat_s( pack_filename, max_out, Pathtypes[f->pathtype_index].path );

							if ( pack_filename[strlen(pack_filename)-1] != DIR_SEPARATOR_CHAR )
								strcat_s( pack_filename, max_out, DIR_SEPARATOR_STR );
						}

						strcat_s( pack_filename, max_out, f->name_ext );
					}
				}

				// found it, so cleanup and return
				file_list_index.clear();

				return cur_ext;
			}

			// ok, we're still here, so strip off the extension again in order to
			// prepare for the next run
			p = strrchr(filespec, '.');
			if ( p && (strlen(p) > 2) )
				(*p) = 0;
		}
	}

Bail:
	// didn't find anything, bail...
	file_list_index.clear();

	return -1;
}


// Returns true if filename matches filespec, else zero if not
int cf_matches_spec(char *filespec, char *filename)
{
	char *src_ext, *dst_ext;

	src_ext = strrchr(filespec, '*');
	if(!src_ext)
	{
		src_ext = strrchr(filespec, '.');
		if (!src_ext)
			return 1;
	}
	else
	{
		src_ext++;
	}

	if(strlen(filespec) > strlen(filename))
	{
		return 0;
	}

	dst_ext = filename + strlen(filename) - ((filespec + strlen(filespec)) - src_ext);
	if (!dst_ext)
		return 1;
	
	if(src_ext == filespec)
	{
		return !stricmp(dst_ext, src_ext);
	}
	else
	{
		return (!stricmp(dst_ext, src_ext) && !strnicmp(dst_ext, src_ext, src_ext - filespec));
	}
}

int (*Get_file_list_filter)(char *filename) = NULL;
int Skip_packfile_search = 0;

int cf_file_already_in_list( int num_files, char **list, char *filename )
{
	int i;

	char name_no_extension[MAX_PATH_LEN];

	strcpy_s(name_no_extension, filename );
	char *p = strrchr( name_no_extension, '.' );
	if ( p ) *p = 0;

	for (i=0; i<num_files; i++ )	{
		if ( !stricmp(list[i], name_no_extension ) )	{
			// Match found!
			return 1;
		}
	}
	// Not found
	return 0;
}

// An alternative cf_get_file_list(), dynamic list version.
// This one has a 'type', which is a CF_TYPE_* value.  Because this specifies the directory
// location, 'filter' only needs to be the filter itself, with no path information.
// See above descriptions of cf_get_file_list() for more information about how it all works.
int cf_get_file_list( int max, char **list, int pathtype, char *filter, int sort, file_list_info *info )
{
	char *ptr;
	uint i;
	int l, num_files = 0, own_flag = 0;

	if (max < 1) {
		Get_file_list_filter = NULL;

		return 0;
	}

	Assert(list);

	if (!info && (sort == CF_SORT_TIME)) {
		info = (file_list_info *) vm_malloc(sizeof(file_list_info) * max);
		own_flag = 1;
	}

	char filespec[MAX_PATH_LEN];

#if defined _WIN32
	cf_create_default_path_string( filespec, sizeof(filespec)-1, pathtype, filter );

	_finddata_t find;
	int find_handle;

	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
			if (num_files >= max)
				break;

            if (strcmp(strstr(filter, "."), strstr(find.name,".")) != 0)
                continue;

			if ( strlen(find.name) >= MAX_FILENAME_LEN )
				continue;

			if (!(find.attrib & _A_SUBDIR)) {
				if ( !Get_file_list_filter || (*Get_file_list_filter)(find.name) ) {
					ptr = strrchr(find.name, '.');
					if (ptr)
						l = ptr - find.name;
					else
						l = strlen(find.name);

					list[num_files] = (char *)vm_malloc(l + 1);
					strncpy(list[num_files], find.name, l);
					list[num_files][l] = 0;
					if (info)
						info[num_files].write_time = find.time_write;

					num_files++;
				}
			}

		} while (!_findnext(find_handle, &find));

		_findclose( find_handle );
	}

#elif defined SCP_UNIX
	cf_create_default_path_string( filespec, sizeof(filespec)-1, pathtype, NULL );

	DIR *dirp;
	struct dirent *dir;

	dirp = opendir (filespec);
	if ( dirp ) {
		while ((dir = readdir (dirp)) != NULL)
		{
			if (num_files >= max)
				break;

			if ( strlen(dir->d_name) >= MAX_FILENAME_LEN ) {
				continue;
			}

			if (fnmatch(filter, dir->d_name, 0) != 0)
				continue;

			char fn[MAX_PATH];
			snprintf(fn, MAX_PATH-1, "%s/%s", filespec, dir->d_name);
			fn[MAX_PATH-1] = 0;

			struct stat buf;
			if (stat(fn, &buf) == -1) {
				continue;
			}

			if (!S_ISREG(buf.st_mode)) {
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(dir->d_name) ) {
				ptr = strrchr(dir->d_name, '.');
				if (ptr)
					l = ptr - dir->d_name;
				else
					l = strlen(dir->d_name);

				list[num_files] = (char *)vm_malloc(l + 1);
				strncpy(list[num_files], dir->d_name, l);
				list[num_files][l] = 0;
				if (info)
					info[num_files].write_time = buf.st_mtime;

				num_files++;
			}
		}

		closedir(dirp);
	}
#endif

	// Search all the packfiles and CD.
	if ( !Skip_packfile_search )	{
		for (i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (num_files >= max)
				break;
			
			if ( !cf_matches_spec( filter,f->name_ext))	{
				continue;
			}

			if ( cf_file_already_in_list(num_files,list,f->name_ext))	{
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(f->name_ext) ) {

				//mprintf(( "Found '%s' in root %d path %d\n", f->name_ext, f->root_index, f->pathtype_index ));

					ptr = strrchr(f->name_ext, '.');
					if (ptr)
						l = ptr - f->name_ext;
					else
						l = strlen(f->name_ext);

					list[num_files] = (char *)vm_malloc(l + 1);
					strncpy(list[num_files], f->name_ext, l);
					list[num_files][l] = 0;

				if (info)	{
					info[num_files].write_time = f->write_time;
				}

				num_files++;
			}

		}
	}


	if (sort != CF_SORT_NONE)	{
		cf_sort_filenames( num_files, list, sort, info );
	}

	if (own_flag)	{
		vm_free(info);
	}

	Get_file_list_filter = NULL;

	return num_files;
}

int cf_file_already_in_list_preallocated( int num_files, char arr[][MAX_FILENAME_LEN], char *filename )
{
	int i;

	char name_no_extension[MAX_PATH_LEN];

	strcpy_s(name_no_extension, filename );
	char *p = strrchr( name_no_extension, '.' );
	if ( p ) *p = 0;

	for (i=0; i<num_files; i++ )	{
		if ( !stricmp(arr[i], name_no_extension ) )	{
			// Match found!
			return 1;
		}
	}
	// Not found
	return 0;
}

// An alternative cf_get_file_list(), fixed array version.
// This one has a 'type', which is a CF_TYPE_* value.  Because this specifies the directory
// location, 'filter' only needs to be the filter itself, with no path information.
// See above descriptions of cf_get_file_list() for more information about how it all works.
int cf_get_file_list_preallocated( int max, char arr[][MAX_FILENAME_LEN], char **list, int pathtype, char *filter, int sort, file_list_info *info )
{
	int num_files = 0, own_flag = 0;

	if (max < 1) {
		Get_file_list_filter = NULL;

		return 0;
	}

	if (list) {
		for (int i=0; i<max; i++)	{
			list[i] = arr[i];
		}
	} else {
		sort = CF_SORT_NONE;  // sorting of array directly not supported.  Sorting done on list only
	}

	if (!info && (sort == CF_SORT_TIME)) {
		info = (file_list_info *) vm_malloc(sizeof(file_list_info) * max);

		if ( info )
			own_flag = 1;
	}

	char filespec[MAX_PATH_LEN];

	// Search the default directories
#if defined _WIN32
	cf_create_default_path_string( filespec, sizeof(filespec)-1, pathtype, filter );

	int find_handle;
	_finddata_t find;
	
	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
			if (num_files >= max)			
				break;

			if (!(find.attrib & _A_SUBDIR)) {
            
                if (strcmp(strstr(filter, "."), strstr(find.name,".")) != 0)
                    continue;

				if ( strlen(find.name) >= MAX_FILENAME_LEN )
					continue;

				if ( !Get_file_list_filter || (*Get_file_list_filter)(find.name) ) {

					strncpy(arr[num_files], find.name, MAX_FILENAME_LEN - 1 );
					char *ptr = strrchr(arr[num_files], '.');
					if ( ptr ) {
						*ptr = 0;
					}

					if (info)	{
						info[num_files].write_time = find.time_write;
					}

					num_files++;
				}
			}

		} while (!_findnext(find_handle, &find));

		_findclose( find_handle );
	}

#elif defined SCP_UNIX
	cf_create_default_path_string( filespec, sizeof(filespec)-1, pathtype, NULL );

	DIR *dirp;
	struct dirent *dir;

	dirp = opendir (filespec);
	if ( dirp ) {
		while ((dir = readdir (dirp)) != NULL)
		{
			if (num_files >= max)
				break;

			if (fnmatch(filter, dir->d_name, 0) != 0)
				continue;

			char fn[MAX_PATH];
			snprintf(fn, MAX_PATH-1, "%s/%s", filespec, dir->d_name);
			fn[MAX_PATH-1] = 0;

			struct stat buf;
			if (stat(fn, &buf) == -1) {
				continue;
			}

			if (!S_ISREG(buf.st_mode)) {
				continue;
			}

			if ( strlen(dir->d_name) >= MAX_FILENAME_LEN ) {
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(dir->d_name) ) {

				strncpy(arr[num_files], dir->d_name, MAX_FILENAME_LEN - 1 );
				char *ptr = strrchr(arr[num_files], '.');
				if ( ptr ) {
					*ptr = 0;
				}

				if (info)	{
					info[num_files].write_time = buf.st_mtime;
				}

				num_files++;
			}
		}
		closedir(dirp);
	}
#endif

	// Search all the packfiles and CD.
	if ( !Skip_packfile_search )	{
		for (uint i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (num_files >= max)
						
				break;

			if ( !cf_matches_spec( filter,f->name_ext))	{
				continue;
			}

			if ( cf_file_already_in_list_preallocated( num_files, arr, f->name_ext ))	{
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(f->name_ext) ) {

				//mprintf(( "Found '%s' in root %d path %d\n", f->name_ext, f->root_index, f->pathtype_index ));

				strncpy(arr[num_files], f->name_ext, MAX_FILENAME_LEN - 1 );
				char *ptr = strrchr(arr[num_files], '.');
				if ( ptr ) {
					*ptr = 0;
				}

				if (info)	{
					info[num_files].write_time = f->write_time;
				}

				num_files++;
			}

		}
	}

	if (sort != CF_SORT_NONE) {
		Assert(list);
		cf_sort_filenames( num_files, list, sort, info );
	}

	if (own_flag)	{
		vm_free(info);
	}

	Get_file_list_filter = NULL;

	return num_files;
}

// Returns the default storage path for files given a 
// particular pathtype.   In other words, the path to 
// the unpacked, non-cd'd, stored on hard drive path.
// If filename isn't null it will also tack the filename
// on the end, creating a completely valid filename.
// Input:   pathtype  - CF_TYPE_??
//			path_max  - Maximum characters in the path
//          filename  - optional, if set, tacks the filename onto end of path.
// Output:  path      - Fully qualified pathname.
//Returns 0 if the result would be too long (invalid result)
int cf_create_default_path_string( char *path, uint path_max, int pathtype, char *filename, bool localize )
{
#ifdef SCP_UNIX
	if ( filename && strpbrk(filename,"/")  ) {
#else
	if ( filename && strpbrk(filename,"/\\:")  ) {
#endif
		// Already has full path
		strncpy( path, filename, path_max );

	} else {
		cf_root *root = cf_get_root(0);

		if (!root) {
			Assert( filename != NULL );
			strncpy(path, filename, path_max);
			return 1;
		}

		Assert(CF_TYPE_SPECIFIED(pathtype));

		// force a specific directory to search for player files
		if ( (pathtype == CF_TYPE_PLAYERS) || (pathtype == CF_TYPE_SINGLE_PLAYERS) || (pathtype == CF_TYPE_MULTI_PLAYERS) ) {
			strncpy(path, Pilot_file_path, path_max);
		} else {
			strncpy(path, root->path, path_max);
		}

		strcat_s(path, path_max, Pathtypes[pathtype].path);

		// Don't add slash for root directory
		if (Pathtypes[pathtype].path[0] != '\0') {
			if ( path[strlen(path)-1] != DIR_SEPARATOR_CHAR ) {
				strcat_s(path, path_max, DIR_SEPARATOR_STR);
			}
		}

#ifdef INF_BUILD
		// keep pilot files separated for an Inferno build since they aren't compatible
		if ( pathtype == CF_TYPE_SINGLE_PLAYERS || pathtype == CF_TYPE_MULTI_PLAYERS ) {
			strcat_s(path, path_max, "inferno");
			strcat_s(path, path_max, DIR_SEPARATOR_STR);
		}
#endif

		// add filename
		if (filename) {
			strcat_s(path, path_max, filename);

			// localize filename
			if (localize) {
				// create copy of path
				char path_tmp[MAX_PATH_LEN] = { 0 };
				strncpy( path_tmp, path, MAX_PATH_LEN - 1 );

				// localize the path
				if(lcl_add_dir_to_path_with_filename(path_tmp, MAX_PATH_LEN - 1)) {
					// verify localized path
					FILE *fp = fopen(path, "rb");
					if (fp) {
						fclose(fp);
						return 1;
					}
				}
			}
		}
	}

	return 1;
}

void cfile_spew_pack_file_crcs()
{
	int i;
	char out_path[CFILE_ROOT_DIRECTORY_LEN+MAX_FILENAME_LEN];
	char datetime[45];
	uint chksum = 0;
	time_t my_time;
	
#ifdef SCP_UNIX
	sprintf(out_path, "%s%svp_crcs.txt", Cfile_user_dir, DIR_SEPARATOR_STR);
#else
	sprintf(out_path, "%s%svp_crcs.txt", Cfile_root_dir, DIR_SEPARATOR_STR);
#endif

	FILE *out = fopen(out_path, "w");

	if (out == NULL) {
		Int3();
		return;
	}

	my_time = time(NULL);

	memset( datetime, 0, sizeof(datetime) );
	snprintf(datetime, sizeof(datetime)-1, "%s", ctime(&my_time));
	// ctime() adds a newline char, so we have to strip it off
	datetime[strlen(datetime)-1] = '\0';

	fprintf(out, "Pack file CRC log (%s) ... \n", datetime);
	fprintf(out, "-------------------------------------------------------------------------------\n");

	for (i = 0; i < Num_roots; i++) {
		cf_root *cur_root = cf_get_root(i);

		if (cur_root->roottype != CF_ROOTTYPE_PACK)
			continue;

		chksum = 0;
		cf_chksum_pack(cur_root->path, &chksum, true);

		fprintf(out, "  %s  --  0x%x\n", cur_root->path, chksum);
	}

	fprintf(out, "-------------------------------------------------------------------------------\n");

	fclose(out);
}
