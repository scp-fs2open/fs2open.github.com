/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <algorithm>

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
#include <libgen.h>
#endif

#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "def_files/def_files.h"
#include "localization/localize.h"
#include "osapi/osapi.h"
#include "parse/parselo.h"

enum CfileRootType {
	CF_ROOTTYPE_PATH = 0,
	CF_ROOTTYPE_PACK = 1,
	CF_ROOTTYPE_MEMORY = 2,
};

//  Created by:
//    specifying hard drive tree
//    searching for pack files on hard drive		// Found by searching all known paths
//    specifying cd-rom tree
//    searching for pack files on CD-rom tree
typedef struct cf_root {
	char	path[CF_MAX_PATHNAME_LENGTH];	// Contains something like c:\projects\freespace or c:\projects\freespace\freespace.vp
	int		roottype;						// CF_ROOTTYPE_PATH  = Path, CF_ROOTTYPE_PACK =Pack file, CF_ROOTTYPE_MEMORY=In memory
	uint32_t location_flags;
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
	char		name_ext[CF_MAX_FILENAME_LENGTH];	// Filename and extension
	int			root_index;							// Where in Roots this is located
	int			pathtype_index;						// Where in Paths this is located
	time_t		write_time;							// When it was last written
	int			size;								// How big it is in bytes
	int			pack_offset;						// For pack files, where it is at.   0 if not in a pack file.  This can be used to tell if in a pack file.
	char*		real_name;							// For real files, the full path
	const void*	data;								// For in-memory files, the data pointer
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
		memset(File_blocks[block], 0, sizeof(cf_file_block));
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

		intptr_t find_handle;
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
bool cf_packfile_sort_func(const cf_root_sort &r1, const cf_root_sort &r2)
{
	// if the 2 directory types are the same, do a string compare
	if (r1.cf_type == r2.cf_type) {
		return (stricmp(r1.path, r2.path) < 0);
	}

	// otherwise return them in order of CF_TYPE_* precedence
	return (r1.cf_type < r2.cf_type);
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

		intptr_t find_handle;
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
	std::sort(temp_roots_sort, temp_roots_sort + temp_root_count, cf_packfile_sort_func);

	// now insert them all into the real root list properly
	for (i = 0; i < temp_root_count; i++) {
		auto new_root            = cf_create_root();
		new_root->location_flags = root->location_flags;
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

static char normalize_directory_separator(char in)
{
	if (in == '/')
	{
		return DIR_SEPARATOR_CHAR;
	}

	return in;
}

static void cf_add_mod_roots(const char* rootDirectory, uint32_t basic_location)
{
	if (Cmdline_mod)
	{
		bool primary = true;
		for (const char* cur_pos=Cmdline_mod; strlen(cur_pos) != 0; cur_pos+= (strlen(cur_pos)+1))
		{
			SCP_stringstream ss;
			ss << rootDirectory;

			if (rootDirectory[strlen(rootDirectory) - 1] != DIR_SEPARATOR_CHAR)
			{
				ss << DIR_SEPARATOR_CHAR;
			}

			ss << cur_pos << DIR_SEPARATOR_STR;

			SCP_string rootPath = ss.str();
			if (rootPath.size() + 1 >= CF_MAX_PATHNAME_LENGTH) {
				Error(LOCATION, "The length of mod directory path '%s' exceeds the maximum of %d!\n", rootPath.c_str(), CF_MAX_PATHNAME_LENGTH);
			}

			// normalize the path to the native path format
			std::transform(rootPath.begin(), rootPath.end(), rootPath.begin(), normalize_directory_separator);

			cf_root* root = cf_create_root();

			strncpy(root->path, rootPath.c_str(),  CF_MAX_PATHNAME_LENGTH-1);
			if (primary) {
				root->location_flags = basic_location | CF_LOCATION_TYPE_PRIMARY_MOD;
			} else {
				root->location_flags = basic_location | CF_LOCATION_TYPE_SECONDARY_MODS;
			}

			root->roottype = CF_ROOTTYPE_PATH;
			cf_build_pack_list(root);

			primary = false;
		}
	}
}

void cf_build_root_list(const char *cdrom_dir)
{
	Num_roots = 0;
	Num_path_roots = 0;

	cf_root	*root = nullptr;

	if (os_is_legacy_mode())
	{
		// =========================================================================
#ifdef WIN32
		// Nothing to do here, Windows uses the current directory as the base
#else
		cf_add_mod_roots(os_get_legacy_user_dir(), CF_LOCATION_ROOT_USER);

		root = cf_create_root();
		strncpy(root->path, os_get_legacy_user_dir(), CF_MAX_PATHNAME_LENGTH - 1);

		root->location_flags |= CF_LOCATION_ROOT_USER | CF_LOCATION_TYPE_ROOT;
		if (Cmdline_mod == nullptr || strlen(Cmdline_mod) <= 0) {
			// If there are no mods then the root is the primary mod
			root->location_flags |= CF_LOCATION_TYPE_PRIMARY_MOD;
		}

		// do we already have a slash? as in the case of a root directory install
		if ((strlen(root->path) < (CF_MAX_PATHNAME_LENGTH - 1)) && (root->path[strlen(root->path) - 1] != DIR_SEPARATOR_CHAR)) {
			strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
		}
		root->roottype = CF_ROOTTYPE_PATH;

		// Next, check any VP files under the current directory.
		cf_build_pack_list(root);
#endif
		// =========================================================================
	}
	else if (!Cmdline_portable_mode)
	{
		// =========================================================================
		// now look for mods under the users HOME directory to use before system ones
		cf_add_mod_roots(Cfile_user_dir, CF_LOCATION_ROOT_USER);
		// =========================================================================

		// =========================================================================
		// set users HOME directory as default for loading and saving files
		root = cf_create_root();
		strcpy_s(root->path, Cfile_user_dir);

		root->location_flags |= CF_LOCATION_ROOT_USER | CF_LOCATION_TYPE_ROOT;
		if (Cmdline_mod == nullptr || strlen(Cmdline_mod) <= 0) {
			// If there are no mods then the root is the primary mod
			root->location_flags |= CF_LOCATION_TYPE_PRIMARY_MOD;
		}

		// do we already have a slash? as in the case of a root directory install
		if ((strlen(root->path) < (CF_MAX_PATHNAME_LENGTH - 1)) && (root->path[strlen(root->path) - 1] != DIR_SEPARATOR_CHAR)) {
			strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
		}
		root->roottype = CF_ROOTTYPE_PATH;

		// Next, check any VP files under the current directory.
		cf_build_pack_list(root);
		// =========================================================================
	}

	char working_directory[CF_MAX_PATHNAME_LENGTH];
	
	if ( !_getcwd(working_directory, CF_MAX_PATHNAME_LENGTH ) ) {
		Error(LOCATION, "Can't get current working directory -- %d", errno );
	}

	cf_add_mod_roots(working_directory, CF_LOCATION_ROOT_GAME);

	root = cf_create_root();

	root->location_flags |= CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT;
	if (Cmdline_mod == nullptr || strlen(Cmdline_mod) <= 0) {
		// If there are no mods then the root is the primary mod
		root->location_flags |= CF_LOCATION_TYPE_PRIMARY_MOD;
	}

	strcpy_s(root->path, working_directory);

	size_t path_len = strlen(root->path);

	// do we already have a slash? as in the case of a root directory install
	if ( (path_len < (CF_MAX_PATHNAME_LENGTH-1)) && (root->path[path_len-1] != DIR_SEPARATOR_CHAR) ) {
		strcat_s(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
	}

	root->roottype = CF_ROOTTYPE_PATH;

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

	// The final root is the in-memory root
	root = cf_create_root();
	root->location_flags = CF_LOCATION_ROOT_MEMORY | CF_LOCATION_TYPE_ROOT;
	memset(root->path, 0, sizeof(root->path));
	root->roottype = CF_ROOTTYPE_MEMORY;

}

// Given a lower case list of file extensions 
// separated by spaces, return zero if ext is
// not in the list.
int is_ext_in_list( const char *ext_list, const char *ext )
{
	char tmp_ext[128];

	strcpy_s( tmp_ext, ext);
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

	cf_root* root = cf_get_root(root_index);

	mprintf(("Searching root '%s' ... ", root->path));

#ifndef WIN32
	try {
		auto current           = root->path;
		const auto prefPathEnd = root->path + strlen(root->path);
		while (current != prefPathEnd) {
			const auto cp = utf8::next(current, prefPathEnd);
			if (cp > 127) {
				// On Windows, we currently do not support Unicode paths so catch this early and let the user
				// know
				const auto invalid_end = current;
				utf8::prior(current, root->path);
				Error(LOCATION,
				      "Trying to use path \"%s\" as a data root. That path is not supported since it "
				      "contains a Unicode character (%s). If possible, change this path to something that only uses "
				      "ASCII characters.",
				      root->path, std::string(current, invalid_end).c_str());
			}
		}
	} catch (const std::exception& e) {
		Error(LOCATION, "UTF-8 error while checking the root path \"%s\": %s", root->path, e.what());
	}
#endif

	char search_path[CF_MAX_PATHNAME_LENGTH];
#ifdef SCP_UNIX
	// This map stores the mapping between a specific path type and the actual path that we use for it
	SCP_unordered_map<int, SCP_string> pathTypeToRealPath;
#endif

	for (i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++) {

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
		SCP_string search_directory = search_path;
		strcat_s( search_path, "*.*" );

		intptr_t find_handle;
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

							SCP_string file_name;
							sprintf(file_name, "%s%s%s", search_directory.c_str(), DIR_SEPARATOR_STR, find.name);

							file->real_name = vm_strdup(file_name.c_str());

							num_files++;
							//mprintf(( "Found file '%s'\n", file->name_ext ));
						}
					}

				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}
#elif defined SCP_UNIX
		DIR *dirp = nullptr;
		SCP_string search_dir;
		{
			if (i == CF_TYPE_ROOT) {
				// Don't search for the same name for the root case since we would be searching in other mod directories in that case
				dirp = opendir (search_path);
				search_dir.assign(search_path);
			} else {
				// On Unix we can have a different case for the search paths so we also need to account for that
				// We do that by looking at the parent of search_path and enumerating all directories and the check if any of
				// them are a case-insensitive match
				SCP_string directory_name;

				auto parentPathIter = pathTypeToRealPath.find(Pathtypes[i].parent_index);

				if (parentPathIter == pathTypeToRealPath.end()) {
					// No parent known yet, use the standard dirname
					char dirname_copy[CF_MAX_PATHNAME_LENGTH];
					memcpy(dirname_copy, search_path, sizeof(search_path));
					// According to the documentation of directory_name and basename, the return value does not need to be freed
					directory_name.assign(dirname(dirname_copy));
				} else {
					// we have a valid parent path -> use that
					directory_name = parentPathIter->second;
				}

				char basename_copy[CF_MAX_PATHNAME_LENGTH];
				memcpy(basename_copy, search_path, sizeof(search_path));
				// According to the documentation of dirname and basename, the return value does not need to be freed
				auto search_name = basename(basename_copy);

				auto parentDirP = opendir(directory_name.c_str());

				if (parentDirP) {
					struct dirent *dir = nullptr;
					while ((dir = readdir (parentDirP)) != nullptr) {

						if (stricmp(search_name, dir->d_name) != 0) {
							continue;
						}

						SCP_string fn;
						sprintf(fn, "%s/%s", directory_name.c_str(), dir->d_name);

						struct stat buf;
						if (stat(fn.c_str(), &buf) == -1) {
							continue;
						}

						if (S_ISDIR(buf.st_mode)) {
							// Found a case insensitive match
							dirp = opendir(fn.c_str());
							search_dir = fn;
							// We also need to store this in our mapping since we may need it in the future
							pathTypeToRealPath.insert(std::make_pair(i, fn));
							break;
						}
					}
					closedir(parentDirP);
				}
			}
		}

		if ( dirp ) {
			struct dirent *dir = nullptr;
			while ((dir = readdir (dirp)) != NULL)
			{
				if (!fnmatch ("*.*", dir->d_name, 0))
				{
					SCP_string fn;
					sprintf(fn, "%s/%s", search_dir.c_str(), dir->d_name);

					struct stat buf;
					if (stat(fn.c_str(), &buf) == -1) {
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

							file->real_name = vm_strdup(fn.c_str());

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
	if (fread(&VP_header, sizeof(VP_header), 1, fp) != 1) {
		mprintf(("Skipping VP file ('%s') because the header could not be read...\n", root->path));
		fclose(fp);
		return;
	}

	VP_header.version = INTEL_INT( VP_header.version ); //-V570
	VP_header.index_offset = INTEL_INT( VP_header.index_offset ); //-V570
	VP_header.num_files = INTEL_INT( VP_header.num_files ); //-V570

	mprintf(( "Searching root pack '%s' ... ", root->path ));

	// Read index info
	fseek(fp, VP_header.index_offset, SEEK_SET);

	char search_path[CF_MAX_PATHNAME_LENGTH];

	strcpy_s( search_path, "" );
	
	// Go through all the files
	int i;
	for (i=0; i<VP_header.num_files; i++ )	{
		VP_FILE find;

		if (fread( &find, sizeof(VP_FILE), 1, fp ) != 1) {
			mprintf(("Failed to read file entry (currently in directory %s)!\n", search_path));
			break;
		}

		find.offset = INTEL_INT( find.offset ); //-V570
		find.size = INTEL_INT( find.size ); //-V570
		find.write_time = INTEL_INT( find.write_time ); //-V570
		find.filename[sizeof(find.filename)-1] = '\0';

		if ( find.size == 0 )	{
			size_t search_path_len = strlen(search_path);
			if ( !stricmp( find.filename, ".." ))	{
				char *p = &search_path[search_path_len-1];
				while( (p > search_path) && (*p != DIR_SEPARATOR_CHAR) )	{
					p--;
				}
				*p = 0;
			} else {
				if ( search_path_len && (search_path[search_path_len-1] != DIR_SEPARATOR_CHAR) ) {
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

void cf_search_memory_root(int root_index) {
	int num_files = 0;
	mprintf(( "Searching memory root ... " ));

	auto default_files = defaults_get_all();
	for (auto& default_file : default_files) {
		// Pure built in files have an empty path_type string
		if (strlen(default_file.path_type) <= 0) {
			// Ignore pure builtin files. They should only be accessible with defaults_get_file
			continue;
		}

		int pathtype = -1;
		for (int i = 0; i < CF_MAX_PATH_TYPES; ++i) {
			if (Pathtypes[i].path == nullptr) {
				continue;
			}

			if (!strcmp(Pathtypes[i].path, default_file.path_type)) {
				pathtype = i;
				break;
			}
		}
		Assertion(pathtype != -1, "Default file '%s%s%s' does not use a valid path type!",
				  default_file.path_type,
				  DIR_SEPARATOR_STR,
				  default_file.filename);

		cf_file *file = cf_create_file();

		strcpy_s( file->name_ext, default_file.filename );
		file->root_index = root_index;
		file->pathtype_index = pathtype;
		file->write_time = time(nullptr); // Just assume that memory files were last written to just now
		file->size = (int)default_file.size;
		file->data = default_file.data;

		num_files++;
	}

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
		} else if (root->roottype == CF_ROOTTYPE_MEMORY) {
			cf_search_memory_root(i);
		}
	}

}


void cf_build_secondary_filelist(const char *cdrom_dir)
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
			// Free file paths
			for (auto& f : File_blocks[i]->files) {
				if (f.real_name) {
					vm_free(f.real_name);
					f.real_name = nullptr;
				}
			}

			vm_free( File_blocks[i] );
			File_blocks[i] = NULL;
		}
	}
	Num_files = 0;
}

/**
 * Searches for a file.
 *
 * @note Follows all rules and precedence and searches CD's and pack files.
 *
 * @param filespec      Filename & extension
 * @param pathtype      See CF_TYPE_ defines in CFILE.H
 * @param localize      Undertake localization
 * @param location_flags Specifies where to search for the specified flag
 *
 * @return A structure which describes the found file
 */
CFileLocation cf_find_file_location(const char* filespec, int pathtype, bool localize, uint32_t location_flags)
{
	int i;
    uint ui;
	int cfs_slow_search = 0;
	char longname[MAX_PATH_LEN];

	Assert( (filespec != NULL) && (strlen(filespec) > 0) ); //-V805

	// see if we have something other than just a filename
	// our current rules say that any file that specifies a direct
	// path will try to be opened on that path.  If that open
	// fails, then we will open the file based on the extension
	// of the file

	// NOTE: full path should also include localization, if so desired
	auto last_separator = strrchr(filespec, DIR_SEPARATOR_CHAR);

	// do we have a full path already?
	if ( last_separator ) {
		FILE *fp = fopen(filespec, "rb" );
		if (fp)	{
			CFileLocation res(true);
			res.size = static_cast<size_t>(filelength(fileno(fp)));

			fclose(fp);

			res.offset = 0;
			res.full_name = filespec;
			res.name_ext = last_separator + 1;

			return res;
		}

		return CFileLocation();		// If they give a full path, fail if not found.
	}

	// Search the hard drive for files first.
	uint num_search_dirs = 0;
	int search_order[CF_MAX_PATH_TYPES];

	if ( CF_TYPE_SPECIFIED(pathtype) )	{
		search_order[num_search_dirs++] = pathtype;
	} else {
		for (i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++) {
			if (i != pathtype)
				search_order[num_search_dirs++] = i;
		}
	}

	memset( longname, 0, sizeof(longname) );


	for (ui=0; ui<num_search_dirs; ui++ )	{
		switch (search_order[ui])
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
			cf_create_default_path_string(longname, sizeof(longname) - 1, search_order[ui], filespec, localize,
			                              location_flags);

#if defined _WIN32
			_finddata_t findstruct;

			intptr_t findhandle = _findfirst(longname, &findstruct);
			if (findhandle != -1) {
				CFileLocation res;
				res.found = true;
				res.size = static_cast<size_t>(findstruct.size);

				_findclose(findhandle);

				res.offset = 0;
				res.full_name = longname;
				res.name_ext = filespec;

				return res;
			}
#endif
			{
				FILE *fp = fopen(longname, "rb" );

				if (fp) {
					CFileLocation res(true);
					res.size = static_cast<size_t>(filelength( fileno(fp) ));

					fclose(fp);

					res.offset = 0;
					res.full_name = longname;
					res.name_ext = filespec;

					return res;
				}
			}
		}
	}

	// Search the pak files and CD-ROM.
	for (ui = 0; ui < Num_files; ui++ )	{
		cf_file *f = cf_get_file(ui);

		// only search paths we're supposed to...
		if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index) )
			continue;

		if (location_flags != CF_LOCATION_ALL) {
			// If a location flag was specified we need to check if the root of this file satisfies the request
			auto root = cf_get_root(f->root_index);

			if (!cf_check_location_flags(root->location_flags, location_flags)) {
				// Root does not satisfy location flags
				continue;
			}
		}

		if (localize) {
			// create localized filespec
			strncpy(longname, filespec, MAX_PATH_LEN - 1);

			if ( lcl_add_dir_to_path_with_filename(longname, MAX_PATH_LEN - 1) ) {
				if ( !stricmp(longname, f->name_ext) ) {
					CFileLocation res(true);
					res.size = static_cast<size_t>(f->size);
					res.offset = (size_t)f->pack_offset;
					res.data_ptr = f->data;
					res.name_ext = f->name_ext;

					if (f->data != nullptr) {
						// This is an in-memory file so we just copy the pathtype name + file name
						res.full_name = Pathtypes[f->pathtype_index].path;
						res.full_name += DIR_SEPARATOR_STR;
						res.full_name += f->name_ext;
					} else if (f->pack_offset < 1) {
						// This is a real file, return the actual file path
						res.full_name = f->real_name;
					} else {
						// File is in a pack file
						cf_root *r = cf_get_root(f->root_index);

						res.full_name = r->path;
					}

					return res;
				}
			}
		}

		// file either not localized or localized version not found
		if ( !stricmp(filespec, f->name_ext) ) {
			CFileLocation res(true);
			res.size = static_cast<size_t>(f->size);
			res.offset = (size_t)f->pack_offset;
			res.data_ptr = f->data;
			res.name_ext = f->name_ext;

			if (f->data != nullptr) {
				// This is an in-memory file so we just copy the pathtype name + file name
				res.full_name = Pathtypes[f->pathtype_index].path;
				res.full_name += DIR_SEPARATOR_STR;
				res.full_name += f->name_ext;
			} else if (f->pack_offset < 1) {
				// This is a real file, return the actual file path
				res.full_name = f->real_name;
			} else {
				// File is in a pack file
				cf_root *r = cf_get_root(f->root_index);

				res.full_name = r->path;
			}

			return res;
		}
	}
		
	return CFileLocation();
}

// -- from parselo.cpp --
extern char *stristr(char *str, const char *substr);

/**
 * Searches for a file.
 *
 * @note Follows all rules and precedence and searches CD's and pack files. Searches all locations in order for first filename using filter list.
 * @note This function is exponentially slow, so don't use it unless truely needed
 *
 * @param filename      Filename & extension
 * @param ext_num       Number of extensions to look for
 * @param ext_list      Extension filter list
 * @param pathtype      See CF_TYPE_ defines in CFILE.H
 * @param max_out       Maximum string length that should be stuffed into pack_filename
 * @param localize      Undertake localization
 *
 * @return A structure containing information about the found file
 */
CFileLocationExt cf_find_file_location_ext( const char *filename, const int ext_num, const char **ext_list, int pathtype, bool localize)
{
	int cur_ext, i;
    uint ui;
	int cfs_slow_search = 0;
	char longname[MAX_PATH_LEN];
	char filespec[MAX_FILENAME_LEN];
	char *p = NULL;
	
	Assert( (filename != NULL) && (strlen(filename) < MAX_FILENAME_LEN) );
	Assert( (ext_list != NULL) && (ext_num > 1) );	// if we are searching for just one ext
													// then this is the wrong function to use

	// if we have a full path already then fail.  this function if for searching via filter only!
	if ( strchr(filename, DIR_SEPARATOR_CHAR) ) {		// do we have a full path already?
		Int3();
		return CFileLocationExt();
	}

	// Search the hard drive for files first.
	uint num_search_dirs = 0;
	int search_order[CF_MAX_PATH_TYPES];

	if ( CF_TYPE_SPECIFIED(pathtype) )	{
		search_order[num_search_dirs++] = pathtype;
	} else {
		for (i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++)
			search_order[num_search_dirs++] = i;
	}

	memset( longname, 0, sizeof(longname) );
	memset( filespec, 0, sizeof(filespec) );

	// strip any existing extension
	strncpy(filespec, filename, MAX_FILENAME_LEN-1);

	for (ui = 0; ui < num_search_dirs; ui++) {
		// always hit the disk if we are looking in only one path
		if (num_search_dirs == 1) {
			cfs_slow_search = 1;
		}
		// otherwise hit based on a directory type
		else {
			switch (search_order[ui])
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
 
			cf_create_default_path_string( longname, sizeof(longname)-1, search_order[ui], filespec, localize );

#if defined _WIN32
			_finddata_t findstruct;

			intptr_t findhandle = _findfirst(longname, &findstruct);
			if (findhandle != -1) {
				CFileLocationExt res(cur_ext);
				res.found = true;
				res.size = static_cast<size_t>(findstruct.size);

				_findclose(findhandle);

				res.offset = 0;
				res.full_name = longname;
				res.name_ext = filespec;

				return res;
			}
#endif
			{
				FILE *fp = fopen(longname, "rb" );

				if (fp) {
					CFileLocationExt res(cur_ext);
					res.found = true;
					res.size = static_cast<size_t>(filelength( fileno(fp) ));

					fclose(fp);

					res.offset = 0;
					res.full_name = longname;
					res.name_ext = filespec;

					return res;
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
	size_t filespec_len = strlen(filespec);

	// get total legnth, with extension, which is iused to test with later
	// (FIXME: this assumes that everything in ext_list[] is the same length!)
	size_t filespec_len_big = filespec_len + strlen(ext_list[0]);

	SCP_vector< cf_file* > file_list_index;
	int last_root_index = -1;
	int last_path_index = -1;

	file_list_index.reserve( MIN(ext_num * 4, (int)Num_files) );

	// next, run though and pick out base matches
	for (ui = 0; ui < Num_files; ui++) {
		cf_file *f = cf_get_file(ui);

		// ... only search paths that we're supposed to
		if ( (num_search_dirs == 1) && (pathtype != f->pathtype_index) )
			continue;

		// ... check that our names are the same length (accounting for the missing extension on our own name)
		if ( strlen(f->name_ext) != filespec_len_big )
			continue;

		// ... check that we match the base filename
		if ( strnicmp(f->name_ext, filespec, filespec_len) != 0 )
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

	// now try and find our preferred match
	for (cur_ext = 0; cur_ext < ext_num; cur_ext++) {
		for (SCP_vector<cf_file*>::iterator fli = file_list_index.begin(); fli != file_list_index.end(); ++fli) {
			cf_file *f = *fli;
	
			strcat_s( filespec, ext_list[cur_ext] );

			if (localize) {
				// create localized filespec
				strncpy(longname, filespec, MAX_PATH_LEN - 1);

				if ( lcl_add_dir_to_path_with_filename(longname, MAX_PATH_LEN - 1) ) {
					if ( !stricmp(longname, f->name_ext) ) {
						CFileLocationExt res(cur_ext);
						res.found = true;
						res.size = static_cast<size_t>(f->size);
						res.offset = (size_t)f->pack_offset;
						res.data_ptr = f->data;
						res.name_ext = f->name_ext;

						if (f->data != nullptr) {
							// This is an in-memory file so we just copy the pathtype name + file name
							res.full_name = Pathtypes[f->pathtype_index].path;
							res.full_name += DIR_SEPARATOR_STR;
							res.full_name += f->name_ext;
						} else if (f->pack_offset < 1) {
							// This is a real file, return the actual file path
							res.full_name = f->real_name;
						} else {
							// File is in a pack file
							cf_root *r = cf_get_root(f->root_index);

							res.full_name = r->path;
						}

						// found it, so cleanup and return
						file_list_index.clear();

						return res;
					}
				}
			}

			// file either not localized or localized version not found
			if ( !stricmp(filespec, f->name_ext) ) {
				CFileLocationExt res(cur_ext);
				res.found = true;
				res.size = static_cast<size_t>(f->size);
				res.offset = (size_t)f->pack_offset;
				res.data_ptr = f->data;
				res.name_ext = f->name_ext;

				if (f->data != nullptr) {
					// This is an in-memory file so we just copy the pathtype name + file name
					res.full_name = Pathtypes[f->pathtype_index].path;
					res.full_name += DIR_SEPARATOR_STR;
					res.full_name += f->name_ext;
				} else if (f->pack_offset < 1) {
					// This is a real file, return the actual file path
					res.full_name = f->real_name;
				} else {
					// File is in a pack file
					cf_root *r = cf_get_root(f->root_index);

					res.full_name = r->path;
				}

				// found it, so cleanup and return
				file_list_index.clear();

				return res;
			}

			// ok, we're still here, so strip off the extension again in order to
			// prepare for the next run
			p = strrchr(filespec, '.');
			if ( p && (strlen(p) > 2) )
				(*p) = 0;
		}
	}

	return CFileLocationExt();
}


// Returns true if filename matches filespec, else zero if not
int cf_matches_spec(const char *filespec, const char *filename)
{
	const char *src_ext;
	const char *dst_ext;

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

int (*Get_file_list_filter)(const char *filename) = NULL;
const char *Get_file_list_child = NULL;
int Skip_packfile_search = 0;
bool Skip_memory_files = false;

static bool verify_file_list_child()
{
	if (Get_file_list_child == NULL) {
		return false;
	}

	// empty or too long
	size_t len = strlen(Get_file_list_child);
	if ( !len || (len > MAX_FILENAME_LEN) ) {
		return false;
	}

	// can not being with directory separator
	if (Get_file_list_child[0] == DIR_SEPARATOR_CHAR) {
		return false;
	}

	// no ':' or spaces
	if ( strchr(Get_file_list_child, ':') || strchr(Get_file_list_child, ' ') ) {
		return false;
	}

	return true;
}

static int cf_file_already_in_list( SCP_vector<SCP_string> &list, const char *filename )
{
	char name_no_extension[MAX_PATH_LEN];
	size_t i, size = list.size();

	if (size == 0) {
		return 0;
	}

	strcpy_s(name_no_extension, filename );
	char *p = strrchr( name_no_extension, '.' );
	if ( p ) *p = 0;

	for (i = 0; i < size; i++) {
		if ( !stricmp(list[i].c_str(), name_no_extension ) ) {
			// Match found!
			return 1;
		}
	}

	// Not found
	return 0;
}

// An alternative cf_get_file_list*(), true dynamic list version.
// This one has a 'type', which is a CF_TYPE_* value.  Because this specifies the directory
// location, 'filter' only needs to be the filter itself, with no path information.
// See above descriptions of cf_get_file_list() for more information about how it all works.
// Note that filesystem listing is always sorted by name *before* sorting by the
// provided sort order. This isn't strictly needed on NTFS, which always provides the list
// sorted by name. But on mac/linux it's always needed.
int cf_get_file_list(SCP_vector<SCP_string>& list, int pathtype, const char* filter, int sort,
                     SCP_vector<file_list_info>* info, uint32_t location_flags)
{
	char *ptr;
	uint i;
	int own_flag = 0;
	size_t l;
	SCP_vector<file_list_info> my_info;
	file_list_info tinfo;

	if ( !info && (sort == CF_SORT_TIME) ) {
		info = &my_info;
		own_flag = 1;
	}

	char filespec[MAX_PATH_LEN];

	bool check_duplicates = !list.empty();

	if ( check_duplicates && (sort != CF_SORT_NONE) ) {
		Int3();
		sort = CF_SORT_NONE;
	}

	if (Get_file_list_child && !verify_file_list_child() ) {
		Get_file_list_child = NULL;
	}

#if defined _WIN32
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, (char*)Get_file_list_child, false,
	                              location_flags);
	strcat_s(filespec, DIR_SEPARATOR_STR);
	strcat_s(filespec, filter);

	_finddata_t find;
	intptr_t find_handle;

	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
            if (strcmp(strrchr(filter, '.'), strrchr(find.name,'.')) != 0)
                continue;

			if (!(find.attrib & _A_SUBDIR)) {
				if ( !Get_file_list_filter || (*Get_file_list_filter)(find.name) ) {
					if ( check_duplicates && cf_file_already_in_list(list, find.name) ) {
						continue;
					}

					ptr = strrchr(find.name, '.');
					if (ptr)
						l = (size_t)(ptr - find.name);
					else
						l = strlen(find.name);

					list.push_back( SCP_string(find.name, l) );

					if (info) {
						tinfo.write_time = find.time_write;
						info->push_back( tinfo );
					}
				}
			}

		} while (!_findnext(find_handle, &find));

		_findclose( find_handle );
	}

#elif defined SCP_UNIX
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, (char*)Get_file_list_child, false,
	                              location_flags);

	DIR *dirp;
	struct dirent *dir;

	dirp = opendir (filespec);
	if ( dirp ) {
		while ((dir = readdir (dirp)) != NULL) {

			if (fnmatch(filter, dir->d_name, 0) != 0)
				continue;

			char fn[MAX_PATH];
			if (snprintf(fn, MAX_PATH, "%s/%s", filespec, dir->d_name) >= MAX_PATH) {
				// Make sure the string is null terminated
				fn[MAX_PATH-1] = 0;
			}

			struct stat buf;
			if (stat(fn, &buf) == -1) {
				continue;
			}

			if (!S_ISREG(buf.st_mode)) {
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(dir->d_name) ) {
				if ( check_duplicates && cf_file_already_in_list(list, dir->d_name) ) {
					continue;
				}

				ptr = strrchr(dir->d_name, '.');
				if (ptr)
					l = (size_t)(ptr - dir->d_name);
				else
					l = strlen(dir->d_name);

				list.push_back( SCP_string(dir->d_name, l) );

				if (info) {
					tinfo.write_time = buf.st_mtime;
					info->push_back( tinfo );
				}
			}
		}

		closedir(dirp);
	}
#endif

	// tcrayford: sort the filesystem listing by name by default to ensure
	// a stable order across filesystems
	cf_sort_filenames( list, CF_SORT_NAME, info );

	bool skip_packfiles = false;
	if ( (pathtype == CF_TYPE_PLAYERS) || (pathtype == CF_TYPE_SINGLE_PLAYERS) || (pathtype == CF_TYPE_MULTI_PLAYERS) ) {
		skip_packfiles = true;
	} else if (Get_file_list_child != NULL) {
		skip_packfiles = true;
	}

	// Search all the packfiles and CD.
	if ( !skip_packfiles )	{
		for (i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (location_flags != CF_LOCATION_ALL) {
				// If a location flag was specified we need to check if the root of this file satisfies the request
				auto root = cf_get_root(f->root_index);

				if (!cf_check_location_flags(root->location_flags, location_flags)) {
					// Root does not satisfy location flags
					continue;
				}
			}

			if ( !cf_matches_spec( filter,f->name_ext))	{
				continue;
			}

			if ( cf_file_already_in_list(list, f->name_ext))	{
				continue;
			}

			if (Skip_packfile_search && f->pack_offset != 0) {
				// If the packfile skip flag is set we skip files in VPs but still search in directories
				continue;
			}

			if (Skip_memory_files && f->data != nullptr) {
				// If we want to skip memory files and this is a memory file then ignore it
				continue;
			}

			if ( !Get_file_list_filter || (*Get_file_list_filter)(f->name_ext) ) {
				//mprintf(( "Found '%s' in root %d path %d\n", f->name_ext, f->root_index, f->pathtype_index ));

				ptr = strrchr(f->name_ext, '.');
				if (ptr)
					l = (size_t)(ptr - f->name_ext);
				else
					l = strlen(f->name_ext);

				list.push_back( SCP_string(f->name_ext, l) );

				if (info) {
					tinfo.write_time = f->write_time;
					info->push_back( tinfo );
				}
			}

		}
	}

	if (sort != CF_SORT_NONE)	{
		cf_sort_filenames( list, sort, info );
	}

	if (own_flag)	{
		my_info.clear();
	}

	Get_file_list_filter = NULL;
	Get_file_list_child = NULL;

	return (int)list.size();
}

int cf_file_already_in_list( int num_files, char **list, const char *filename )
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
int cf_get_file_list(int max, char** list, int pathtype, const char* filter, int sort, file_list_info* info,
                     uint32_t location_flags)
{
	char *ptr;
	uint i;
	int num_files = 0, own_flag = 0;
	size_t l;

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
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, filter, false, location_flags);

	_finddata_t find;
	intptr_t find_handle;

	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
			if (num_files >= max)
				break;

            if (strcmp(strrchr(filter, '.'), strrchr(find.name,'.')) != 0)
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
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, nullptr, false, location_flags);

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
			if (snprintf(fn, MAX_PATH, "%s/%s", filespec, dir->d_name) >= MAX_PATH) {
				// Make sure the string is null terminated
				fn[MAX_PATH-1] = 0;
			}

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

	bool skip_packfiles = false;
	if ((pathtype == CF_TYPE_PLAYERS) || (pathtype == CF_TYPE_SINGLE_PLAYERS) || (pathtype == CF_TYPE_MULTI_PLAYERS)) {
		skip_packfiles = true;
	}
	else if (Get_file_list_child != NULL) {
		skip_packfiles = true;
	}

	// Search all the packfiles and CD.
	if ( !skip_packfiles)	{
		for (i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (location_flags != CF_LOCATION_ALL) {
				// If a location flag was specified we need to check if the root of this file satisfies the request
				auto root = cf_get_root(f->root_index);

				if (!cf_check_location_flags(root->location_flags, location_flags)) {
					// Root does not satisfy location flags
					continue;
				}
			}

			if (num_files >= max)
				break;
			
			if ( !cf_matches_spec( filter,f->name_ext))	{
				continue;
			}

			if ( cf_file_already_in_list(num_files,list,f->name_ext))	{
				continue;
			}

			if (Skip_packfile_search && f->pack_offset != 0) {
				// If the packfile skip flag is set we skip files in VPs but still search in directories
				continue;
			}

			if (Skip_memory_files && f->data != nullptr) {
				// If we want to skip memory files and this is a memory file then ignore it
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

int cf_file_already_in_list_preallocated( int num_files, char arr[][MAX_FILENAME_LEN], const char *filename )
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
int cf_get_file_list_preallocated(int max, char arr[][MAX_FILENAME_LEN], char** list, int pathtype, const char* filter,
                                  int sort, file_list_info* info, uint32_t location_flags)
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
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, filter, false, location_flags);

	intptr_t find_handle;
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
	cf_create_default_path_string(filespec, sizeof(filespec) - 1, pathtype, nullptr, false, location_flags);

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
			if (snprintf(fn, MAX_PATH, "%s/%s", filespec, dir->d_name) >= MAX_PATH) {
				// Make sure the string is null terminated
				fn[MAX_PATH-1] = 0;
			}

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

				strcpy_s(arr[num_files], dir->d_name );
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

	bool skip_packfiles = false;
	if ((pathtype == CF_TYPE_PLAYERS) || (pathtype == CF_TYPE_SINGLE_PLAYERS) || (pathtype == CF_TYPE_MULTI_PLAYERS)) {
		skip_packfiles = true;
	}
	else if (Get_file_list_child != NULL) {
		skip_packfiles = true;
	}

	// Search all the packfiles and CD.
	if (!skip_packfiles) {
		for (uint i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (location_flags != CF_LOCATION_ALL) {
				// If a location flag was specified we need to check if the root of this file satisfies the request
				auto root = cf_get_root(f->root_index);

				if (!cf_check_location_flags(root->location_flags, location_flags)) {
					// Root does not satisfy location flags
					continue;
				}
			}

			if (num_files >= max)
						
				break;

			if ( !cf_matches_spec( filter,f->name_ext))	{
				continue;
			}

			if ( cf_file_already_in_list_preallocated( num_files, arr, f->name_ext ))	{
				continue;
			}

			if (Skip_packfile_search && f->pack_offset != 0) {
				// If the packfile skip flag is set we skip files in VPs but still search in directories
				continue;
			}

			if (Skip_memory_files && f->data != nullptr) {
				// If we want to skip memory files and this is a memory file then ignore it
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
int cf_create_default_path_string(char* path, uint path_max, int pathtype, const char* filename, bool localize,
                                  uint32_t location_flags)
{
	if ( filename && strchr(filename, DIR_SEPARATOR_CHAR) ) {
		// Already has full path
		strncpy( path, filename, path_max );

	} else {
		cf_root* root = nullptr;

		for (auto i = 0; i < Num_roots; ++i) {
			auto current_root = cf_get_root(i);

			if (current_root->roottype != CF_ROOTTYPE_PATH) {
				// We want a "real" path here so only path roots are valid
				continue;
			}

			if (cf_check_location_flags(current_root->location_flags, location_flags)) {
				// We found a valid root
				root = current_root;
				break;
			}
		}

		if (!root) {
			Assert( filename != NULL );
			strncpy(path, filename, path_max);
			return 1;
		}

		Assert(CF_TYPE_SPECIFIED(pathtype));

		strncpy(path, root->path, path_max);

		strcat_s(path, path_max, Pathtypes[pathtype].path);

		// Don't add slash for root directory
		if (Pathtypes[pathtype].path[0] != '\0') {
			if ( path[strlen(path)-1] != DIR_SEPARATOR_CHAR ) {
				strcat_s(path, path_max, DIR_SEPARATOR_STR);
			}
		}

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

// Returns the default storage path for files given a 
// particular pathtype.   In other words, the path to 
// the unpacked, non-cd'd, stored on hard drive path.
// If filename isn't null it will also tack the filename
// on the end, creating a completely valid filename.
// Input:   pathtype  - CF_TYPE_??
//          filename  - optional, if set, tacks the filename onto end of path.
// Output:  path      - Fully qualified pathname.
//Returns 0 if the result would be too long (invalid result)
int cf_create_default_path_string(SCP_string& path, int pathtype, const char* filename, bool /*localize*/,
                                  uint32_t location_flags)
{
	if ( filename && strchr(filename, DIR_SEPARATOR_CHAR) ) {
		// Already has full path
		path.assign(filename);

	} else {
		cf_root* root = nullptr;

		for (auto i = 0; i < Num_roots; ++i) {
			auto current_root = cf_get_root(i);

			if (current_root->roottype != CF_ROOTTYPE_PATH) {
				// We want a "real" path here so only path roots are valid
				continue;
			}

			if (cf_check_location_flags(current_root->location_flags, location_flags)) {
				// We found a valid root
				root = current_root;
				break;
			}
		}

		if (!root) {
			Assert( filename != NULL );
			path.assign(filename);
			return 1;
		}

		Assert(CF_TYPE_SPECIFIED(pathtype));
		std::ostringstream s_path;

		s_path << root->path;

		s_path << Pathtypes[pathtype].path;

		// Don't add slash for root directory
		if (Pathtypes[pathtype].path[0] != '\0') {
			if ( *(s_path.str().rbegin()) != DIR_SEPARATOR_CHAR ) {
				s_path << DIR_SEPARATOR_STR;
			}
		//	if ( path[strlen(path)-1] != DIR_SEPARATOR_CHAR ) {
		//		strcat_s(path, path_max, DIR_SEPARATOR_STR);
		//	}
		}

		// add filename
		if (filename) {
			s_path << filename;
		}

		path = s_path.str().c_str();
	}

	return 1;
}

void cfile_spew_pack_file_crcs()
{
	int i;
	char datetime[45];
	uint chksum = 0;
	time_t my_time;
	
	FILE *out = fopen(os_get_config_path("vp_crcs.txt").c_str(), "w");

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

bool cf_check_location_flags(uint32_t check_flags, uint32_t desired_flags)
{
	Assertion((check_flags & CF_LOCATION_ROOT_MASK) != 0, "check_flags must have a valid root value");
	Assertion((check_flags & CF_LOCATION_TYPE_MASK) != 0, "check_flags must have a valid type value");

	auto check_root         = check_flags & CF_LOCATION_ROOT_MASK;
	auto desired_root_flags = desired_flags & CF_LOCATION_ROOT_MASK;

	// If the root part is not set then assume that every root matches
	if (desired_root_flags != 0 && (check_root & desired_root_flags) == 0) {
		return false;
	}

	auto check_type         = check_flags & CF_LOCATION_TYPE_MASK;
	auto desired_type_flags = desired_flags & CF_LOCATION_TYPE_MASK;

	if (desired_type_flags != 0 && (check_type & desired_type_flags) == 0) {
		return false;
	}

	return true;
}
