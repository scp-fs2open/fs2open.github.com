/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CFile/CfileSystem.cpp $
 * $Revision: 2.4 $
 * $Date: 2002-10-30 06:26:11 $
 * $Author: DTP $
 *
 * Functions to keep track of and find files that can exist
 * on the harddrive, cd-rom, or in a pack file on either of those.
 * This keeps a list of all the files in packfiles or on CD-rom
 * and when you need a file you call one function which then searches
 * all those locations, inherently enforcing precedence orders.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 19:52:48  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.6  2002/05/24 16:44:10  mharris
 * Fix unix "find files" operations so they return relative path
 *
 * Revision 1.5  2002/05/21 15:38:00  mharris
 * Unix glob changes -- still needs some work as we are getting full pathnames
 *
 * Revision 1.4  2002/05/17 23:38:33  mharris
 * Fixed silly bug in unix find-files code
 *
 * Revision 1.3  2002/05/16 06:03:29  mharris
 * Unix port changes
 *
 * Revision 1.2  2002/05/07 13:22:52  mharris
 * Added errno.h include
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     9/08/99 10:01p Dave
 * Make sure game won't run in a drive's root directory. Make sure
 * standalone routes suqad war messages properly to the host.
 * 
 * 5     9/03/99 1:31a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 4     2/22/99 10:31p Andsager
 * Get rid of unneeded includes.
 * 
 * 3     10/13/98 9:19a Andsager
 * Add localization support to cfile.  Optional parameter with cfopen that
 * looks for localized files.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 14    8/31/98 2:06p Dave
 * Make cfile sort the ordering or vp files. Added support/checks for
 * recognizing "mission disk" players.
 * 
 * 13    6/23/98 4:18p Hoffoss
 * Fixed some bugs with AC release build.
 * 
 * 12    5/20/98 10:46p John
 * Added code that doesn't include duplicate filenames in any file list
 * functions.
 * 
 * 11    5/14/98 2:14p Lawrance2
 * Use filespec filtering for packfiles
 * 
 * 10    5/03/98 11:53a John
 * Fixed filename case mangling.
 * 
 * 9     5/02/98 11:06p Allender
 * correctly deal with pack pathnames
 * 
 * 8     5/01/98 11:41a Allender
 * Fixed bug with mission saving in Fred.
 * 
 * 7     5/01/98 10:21a John
 * Added code to find all pack files in all trees.   Added code to create
 * any directories that we write to.
 * 
 * 6     4/30/98 10:21p John
 * Added code to cleanup cfilesystem
 * 
 * 5     4/30/98 10:18p John
 * added source safe header
 *
 * $NoKeywords: $
 */

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

#ifdef unix
#include <glob.h>
#include <sys/stat.h>
#endif

#include "Cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "localization/localize.h"

#define CF_ROOTTYPE_PATH 0
#define CF_ROOTTYPE_PACK 1

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
  

// Created by searching all roots in order.   This means Files is then sorted by precedence.
typedef struct cf_file {
	char		name_ext[CF_MAX_FILENAME_LENGTH];		// Filename and extension
	int		root_index;										// Where in Roots this is located
	int		pathtype_index;								// Where in Paths this is located
	time_t	write_time;										// When it was last written
	int		size;												// How big it is in bytes
	int		pack_offset;									// For pack files, where it is at.   0 if not in a pack file.  This can be used to tell if in a pack file.
} cf_file;

#define CF_NUM_FILES_PER_BLOCK   256
#define CF_MAX_FILE_BLOCKS			128						// Can store 256*128 = 32768 files

typedef struct cf_file_block {
	cf_file						files[CF_NUM_FILES_PER_BLOCK];
} cf_file_block;

static int Num_files = 0;
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
	int block = Num_files / CF_NUM_FILES_PER_BLOCK;
	int offset = Num_files % CF_NUM_FILES_PER_BLOCK;
	
	if ( File_blocks[block] == NULL )	{
		File_blocks[block] = (cf_file_block *)malloc( sizeof(cf_file_block) );
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
		Root_blocks[block] = (cf_root_block *)malloc( sizeof(cf_root_block) );
		Assert(Root_blocks[block] != NULL);
	}

	Num_roots++;

	return &Root_blocks[block]->roots[offset];
}

// return the # of packfiles which exist
int cf_get_packfile_count(cf_root *root)
{
	char filespec[MAX_PATH_LEN];
	int i;
	int packfile_count;

	// count up how many packfiles we're gonna have
	packfile_count = 0;
	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++ )	{
		strcpy( filespec, root->path );
		
		if((!(Cmdline_mod)) && (i != CF_TYPE_MOD_ROOT)) {
			if(strlen(Pathtypes[i].path)) { //DTP dont add slash for roots
				strcat( filespec, Pathtypes[i].path );
				strcat( filespec, DIR_SEPARATOR_STR );
			}
		}

#if defined _WIN32
		strcat( filespec, "*.vp" );

		int find_handle;
		_finddata_t find;
		
		find_handle = _findfirst( filespec, &find );

 		if (find_handle != -1) {
			do {
				if (!(find.attrib & _A_SUBDIR)) {
					packfile_count++;
				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}	
#elif defined unix
		strcat( filespec, "*.[vV][pP]" );

		glob_t globinfo;
		memset(&globinfo, 0, sizeof(globinfo));
		int status = glob(filespec, 0, NULL, &globinfo);
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
	if(temp_root_count <= 0){
		return;
	}

	// allocate a temporary array of temporary roots so we can easily sort them
	temp_roots_sort = (cf_root_sort*)malloc(sizeof(cf_root_sort) * temp_root_count);
	if(temp_roots_sort == NULL){
		Int3();
		return;
	}

	// now just setup all the root info
	root_index = 0;
	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++ )	{
		strcpy( filespec, root->path );
		
		if((i != CF_TYPE_ROOT) && (i != CF_TYPE_MOD_ROOT))	{ //DTP dont add slash for roots
			//strlen(Pathtypes[i].path)) {
			strcat( filespec, Pathtypes[i].path );
			strcat( filespec, DIR_SEPARATOR_STR );
		}
		


#if defined _WIN32
		strcat( filespec, "*.vp" );

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
					strcpy(rptr_sort->path, root->path);
					if((i != CF_TYPE_ROOT) && (i != CF_TYPE_MOD_ROOT))	{//DTP dont add slash for roots
						//				if(strlen(Pathtypes[i].path)) {
						strcat(rptr_sort->path, Pathtypes[i].path );
						strcat(rptr_sort->path, DIR_SEPARATOR_STR);
					}
					
					strcat(rptr_sort->path, find.name );
					rptr_sort->roottype = CF_ROOTTYPE_PACK;
					rptr_sort->cf_type = i;
				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}	
#elif defined unix
		strcat( filespec, "*.[vV][pP]" );
		glob_t globinfo;
		memset(&globinfo, 0, sizeof(globinfo));
		int status = glob(filespec, 0, NULL, &globinfo);
		if (status == 0) {
			for (unsigned int j = 0;  j < globinfo.gl_pathc;  j++) {
				// Determine if this is a regular file
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				stat(globinfo.gl_pathv[j], &statbuf);
				if (S_ISREG(statbuf.st_mode)) {
					Assert(root_index < temp_root_count);

					// get a temp pointer
					rptr_sort = &temp_roots_sort[root_index++];

					// fill in all the proper info
					strcpy(rptr_sort->path, globinfo.gl_pathv[j] );
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

	// sort tht roots
	qsort(temp_roots_sort,  temp_root_count, sizeof(cf_root_sort), cf_packfile_sort_func);

	// now insert them all into the real root list properly
	cf_root *new_root;
	for(i=0; i<temp_root_count; i++){		
		new_root = cf_create_root();
		strcpy( new_root->path, root->path );

		// mwa -- 4/2/98 put in the next 2 lines because the path name needs to be there
		// to find the files.
		strcpy(new_root->path, temp_roots_sort[i].path);		
		new_root->roottype = CF_ROOTTYPE_PACK;		
	}

	// free up the temp list
	free(temp_roots_sort);
}


void cf_build_root_list(char *cdrom_dir)
{
	Num_roots = 0;

	cf_root	*root;

	root = cf_create_root();
	
	

	if ( !_getcwd(root->path, CF_MAX_PATHNAME_LENGTH ) ) {
		Error(LOCATION, "Can't get current working directory -- %d", errno );
	}

	// do we already have a slash? as in the case of a root directory install
	if(strlen(root->path) && (root->path[strlen(root->path)-1] != DIR_SEPARATOR_CHAR)){
		strcat(root->path, DIR_SEPARATOR_STR);		// put trailing backslash on for easier path construction
	}

			
	if(Cmdline_mod) { //add root here since it will then use files inside this pack before it uses
					  //matching files Vp files found in ../freespace2/
		cf_root *modroot;
		modroot = cf_create_root();
		
		strcpy(modroot->path,cf_add_modname(modroot->path,Pathtypes[CF_TYPE_MOD_ROOT].path));
		cf_build_pack_list(modroot);
		modroot->roottype = CF_ROOTTYPE_PATH;
	}
	
		
   
	root->roottype = CF_ROOTTYPE_PATH;

   //======================================================
	// Next, check any VP files under the current directory.
	cf_build_pack_list(root);


   //======================================================
	// Check the real CD if one...
	if ( cdrom_dir && strlen(cdrom_dir) )	{
		root = cf_create_root();
		strcpy( root->path, cdrom_dir );
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

	cf_root *root = cf_get_root(root_index);

	mprintf(( "Searching root '%s'\n", root->path ));

	char search_path[CF_MAX_PATHNAME_LENGTH];

	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++ )	{

		strcpy( search_path, root->path );

		if((i != CF_TYPE_ROOT) && (i != CF_TYPE_MOD_ROOT))	{ //DTP dont add slash for roots
		//if(strlen(Pathtypes[i].path) {
			strcat( search_path, Pathtypes[i].path );
			strcat( search_path, DIR_SEPARATOR_STR );
		} 

#if defined _WIN32
		strcat( search_path, "*.*" );

		int find_handle;
		_finddata_t find;
		
		find_handle = _findfirst( search_path, &find );

 		if (find_handle != -1) {
			do {
				if (!(find.attrib & _A_SUBDIR)) {

					char *ext = strchr( find.name, '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[i].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();

							strcpy( file->name_ext, find.name );
							file->root_index = root_index;
							file->pathtype_index = i;
							file->write_time = find.time_write;
							file->size = find.size;
							file->pack_offset = 0;			// Mark as a non-packed file

							//mprintf(( "Found file '%s'\n", file->name_ext ));

						}
					}

				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}
#elif defined unix
		strcat( search_path, "*" );
		glob_t globinfo;
		memset(&globinfo, 0, sizeof(globinfo));
		int status = glob(search_path, 0, NULL, &globinfo);
		if (status == 0) {
			for (unsigned int j = 0;  j < globinfo.gl_pathc;  j++) {
				// Determine if this is a regular file
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				stat(globinfo.gl_pathv[j], &statbuf);
				if (S_ISREG(statbuf.st_mode)) {
					char *ext = strchr(globinfo.gl_pathv[j], '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[i].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();
							
							strcpy( file->name_ext, globinfo.gl_pathv[j] );
							file->root_index = root_index;
							file->pathtype_index = i;
							file->write_time = statbuf.st_mtime;
							file->size = statbuf.st_size;
							file->pack_offset = 0;			// Mark as a non-packed file
							
							//mprintf(( "Found file '%s'\n", file->name_ext ));
						}
					}
				}
			}
			globfree(&globinfo);
		}
#endif
	}
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
	time_t write_time;
} VP_FILE;

void cf_search_root_pack(int root_index)
{	
	cf_root *root = cf_get_root(root_index);

	//mprintf(( "Searching root pack '%s'\n", root->path ));

	// Open data
		
	FILE *fp = fopen( root->path, "rb" );
	// Read the file header
	if (!fp) {
		return;
	}

	VP_FILE_HEADER VP_header;

	Assert( sizeof(VP_header) == 16 );
	fread(&VP_header, 1, sizeof(VP_header), fp);

	// Read index info
	fseek(fp, VP_header.index_offset, SEEK_SET);

	char search_path[CF_MAX_PATHNAME_LENGTH];

	strcpy( search_path, "" );
	
	// Go through all the files
	int i;
	for (i=0; i<VP_header.num_files; i++ )	{
		VP_FILE find;

		fread( &find, sizeof(VP_FILE), 1, fp );

		if ( find.size == 0 )	{
			if ( !stricmp( find.filename, ".." ))	{
				int l = strlen(search_path);
				char *p = &search_path[l-1];
				while( (p > search_path) && (*p != DIR_SEPARATOR_CHAR) )	{
					p--;
				}
				*p = 0;
			} else {
				if ( strlen(search_path)	)	{
					strcat( search_path,	DIR_SEPARATOR_STR );
				}
				strcat( search_path, find.filename );
			}

			//mprintf(( "Current dir = '%s'\n", search_path ));
		} else {
	
			int j;			
							
			for (j=CF_TYPE_ROOT; j<CF_MAX_PATH_TYPES; j++ )	{
				
				if ( !stricmp( search_path, Pathtypes[j].path ))	{
					char *ext = strchr( find.filename, '.' );
					if ( ext )	{
						if ( is_ext_in_list( Pathtypes[j].extensions, ext ) )	{
							// Found a file!!!!
							cf_file *file = cf_create_file();
							strcpy( file->name_ext, find.filename );
							file->root_index = root_index;
							file->pathtype_index = j;
							file->write_time = find.write_time;
							file->size = find.size;
							file->pack_offset = find.offset;			// Mark as a packed file
							//mprintf(( "Found pack file '%s'\n", file->name_ext ));
						}
					}
				}
			}
		}
	}
	fclose(fp);
	
}



void cf_build_file_list()
{
	int i;

	Num_files = 0;

	// For each root, find all files...
	for (i=1; i<Num_roots; i++ )	{
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

	// Init the path types
	for (i=0; i<CF_MAX_PATH_TYPES; i++ )	{
		//Assert( Pathtypes[i].index == i );
		// [mharris]  can't modify constant strings, why do this anyhow???
		// just need to make sure exts are all lc in the definition...
//  		if ( Pathtypes[i].extensions )	{
//  			strlwr(Pathtypes[i].extensions);
//  		}
	}
	
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
			free( Root_blocks[i] );
			Root_blocks[i] = NULL;
		}
	}
	Num_roots = 0;

	// Init the file blocks	
	for (i=0; i<CF_MAX_FILE_BLOCKS; i++ )	{
		if ( File_blocks[i] )	{
			free( File_blocks[i] );
			File_blocks[i] = NULL;
		}
	}
	Num_files = 0;
}

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.
// Input:  filespace   - Filename & extension
//         pathtype    - See CF_TYPE_ defines in CFILE.H
// Output: pack_filename - Absolute path and filename of this file.   Could be a packfile or the actual file.
//         size        - File size
//         offset      - Offset into pack file.  0 if not a packfile.
// Returns: If not found returns 0.
int cf_find_file_location( char *filespec, int pathtype, char *pack_filename, int *size, int *offset, bool localize )
{
	int i;

	Assert(filespec && strlen(filespec));

	// see if we have something other than just a filename
	// our current rules say that any file that specifies a direct
	// path will try to be opened on that path.  If that open
	// fails, then we will open the file based on the extension
	// of the file

	// NOTE: full path should also include localization, if so desired
	if ( strpbrk(filespec,"/\\:")  ) {		// do we have a full path already?
		FILE *fp = fopen(filespec, "rb" );
		if (fp)	{
			if ( size ) {
#if defined _WIN32
				*size = filelength(fileno(fp));
#elif defined unix
				struct stat statbuf;
				fstat(fileno(fp), &statbuf);
				*size = statbuf.st_size;
#endif
			}
			if ( offset ) *offset = 0;
			if ( pack_filename ) {
				strcpy( pack_filename, filespec );
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
	}

	for (i=CF_TYPE_ROOT; i<CF_MAX_PATH_TYPES; i++)	{
		if ( i != pathtype )	{
			search_order[num_search_dirs++] = i;
		}
	}

	for (i=0; i<num_search_dirs; i++ )	{
		char longname[MAX_PATH_LEN];

		cf_create_default_path_string( longname, search_order[i], filespec, localize );

		FILE *fp = fopen(longname, "rb" );
		if (fp)	{
			if ( size ) {
#if defined _WIN32
				*size = filelength(fileno(fp));
#elif defined unix
				struct stat statbuf;
				fstat(fileno(fp), &statbuf);
				*size = statbuf.st_size;
#endif
			}
			if ( offset ) *offset = 0;
			if ( pack_filename ) {
				strcpy( pack_filename, longname );
			}				
			fclose(fp);
			return 1;		
		}
	} 

	// Search the pak files and CD-ROM.

		for (i=0; i<Num_files; i++ )	{
			cf_file * f = cf_get_file(i);

			// only search paths we're supposed to...
			if ( (pathtype != CF_TYPE_ANY) && (pathtype != f->pathtype_index)  )	{
				continue;
			}

			if (localize) {
				// create localized filespec
				char temp[MAX_PATH_LEN];
				strcpy(temp, filespec);
				lcl_add_dir_to_path_with_filename(filespec);
			
				if ( !stricmp(filespec, f->name_ext) )	{
					if ( size ) *size = f->size;
					if ( offset ) *offset = f->pack_offset;
					if ( pack_filename ) {
						cf_root * r = cf_get_root(f->root_index);

						strcpy( pack_filename, r->path );
						if ( f->pack_offset < 1 )	{
							strcat( pack_filename, Pathtypes[f->pathtype_index].path );
							strcat( pack_filename, DIR_SEPARATOR_STR );
							strcat( pack_filename, f->name_ext );
						}
					}
					return 1;
				}
				// restore original filespec
				strcpy(filespec, temp);
			}

			// file either not localized or localized version not found
			if ( !stricmp(filespec, f->name_ext) )	{
				if ( size ) *size = f->size;
				if ( offset ) *offset = f->pack_offset;
				if ( pack_filename ) {
					cf_root * r = cf_get_root(f->root_index);

					strcpy( pack_filename, r->path );
					if ( f->pack_offset < 1 )	{

						if(strlen(Pathtypes[f->pathtype_index].path)){
							strcat( pack_filename, Pathtypes[f->pathtype_index].path );
							strcat( pack_filename, DIR_SEPARATOR_STR );
						}

						strcat( pack_filename, f->name_ext );
					}
				}
				
				return 1;
			}
		}
		
		return 0;
}


// Returns true if filename matches filespec, else zero if not
int cf_matches_spec(char *filespec, char *filename)
{
	char *src_ext, *dst_ext;

	src_ext = strchr(filespec, '.');
	if (!src_ext)
		return 1;
	if (*src_ext == '*')
		return 1;

	dst_ext = strchr(filename, '.');
	if (!dst_ext)
		return 1;
	
	return !stricmp(dst_ext, src_ext);
}

int (*Get_file_list_filter)(char *filename) = NULL;
int Skip_packfile_search = 0;

int cf_file_already_in_list( int num_files, char **list, char *filename )
{
	int i;

	char name_no_extension[MAX_PATH_LEN];

	strcpy(name_no_extension, filename );
	char *p = strchr( name_no_extension, '.' );
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
	int i, l, find_handle, num_files = 0, own_flag = 0;

	if (max < 1) {
		Get_file_list_filter = NULL;

		return 0;
	}

	Assert(list);

	if (!info && (sort == CF_SORT_TIME)) {
		info = (file_list_info *) malloc(sizeof(file_list_info) * max);
		own_flag = 1;
	}

	char filespec[MAX_PATH_LEN];

	cf_create_default_path_string( filespec, pathtype, filter );

#if defined _WIN32
	_finddata_t find;
	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
			if (num_files >= max)

				break;

			if (!(find.attrib & _A_SUBDIR)) {
				if ( !Get_file_list_filter || (*Get_file_list_filter)(find.name) ) {
					ptr = strrchr(find.name, '.');
					if (ptr)
						l = ptr - find.name;
					else
						l = strlen(find.name);

					list[num_files] = (char *)malloc(l + 1);
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

#elif defined unix
	char base_filespec[MAX_PATH_LEN];
	cf_create_default_path_string( base_filespec, pathtype );
	int base_filespec_len = strlen(base_filespec);
	glob_t globinfo;
	memset(&globinfo, 0, sizeof(globinfo));
	int status = glob(filespec, 0, NULL, &globinfo);
	if (status == 0) {
		for (unsigned int i = 0;  i < globinfo.gl_pathc;  i++) {
			// Determine if this is a regular file
			struct stat statbuf;
			memset(&statbuf, 0, sizeof(statbuf));
			stat(globinfo.gl_pathv[i], &statbuf);
			if (S_ISREG(statbuf.st_mode)) {
				if ( !Get_file_list_filter || (*Get_file_list_filter)(globinfo.gl_pathv[i]) ) {
					char *found_file = globinfo.gl_pathv[i];
					Assert(strncmp(found_file, base_filespec, base_filespec_len) == 0);
					found_file += base_filespec_len;
					ptr = strrchr(found_file, '.');
					if (ptr)
						l = ptr - found_file;
					else
						l = strlen(found_file);
					
					list[num_files] = (char *)malloc(l + 1);
					strncpy(list[num_files], found_file, l);
					list[num_files][l] = 0;
					if (info)
						info[num_files].write_time = statbuf.st_mtime;
					
					num_files++;
				}
			}
			globfree(&globinfo);
		}
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

					list[num_files] = (char *)malloc(l + 1);
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
		free(info);
	}

	Get_file_list_filter = NULL;

	return num_files;
}

int cf_file_already_in_list_preallocated( int num_files, char arr[][MAX_FILENAME_LEN], char *filename )
{
	int i;

	char name_no_extension[MAX_PATH_LEN];

	strcpy(name_no_extension, filename );
	char *p = strchr( name_no_extension, '.' );
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
	int i, num_files = 0, own_flag = 0;

	if (max < 1) {
		Get_file_list_filter = NULL;

		return 0;
	}

	if (list) {
		for (i=0; i<max; i++)	{
			list[i] = arr[i];
		}
	} else {
		sort = CF_SORT_NONE;  // sorting of array directly not supported.  Sorting done on list only
	}

	if (!info && (sort == CF_SORT_TIME)) {
		info = (file_list_info *) malloc(sizeof(file_list_info) * max);
		own_flag = 1;
	}

	char filespec[MAX_PATH_LEN];

	cf_create_default_path_string( filespec, pathtype, filter );
	mprintf(("cf_get_file_list_preallocated looking for type=%d, filter=\"%s\"\n",
				pathtype, filter));

	// Search the default directories
#if defined _WIN32
	int find_handle;
	_finddata_t find;
	
	find_handle = _findfirst( filespec, &find );
	if (find_handle != -1) {
		do {
			if (num_files >= max)			
				break;

			if (!(find.attrib & _A_SUBDIR)) {

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

#elif defined unix
	char base_filespec[MAX_PATH_LEN];
	cf_create_default_path_string( base_filespec, pathtype );
	int base_filespec_len = strlen(base_filespec);
	glob_t globinfo;
	memset(&globinfo, 0, sizeof(globinfo));
	int status = glob(filespec, 0, NULL, &globinfo);
	if (status == 0) {
		for (unsigned int i = 0;  i < globinfo.gl_pathc;  i++) {
			// Determine if this is a regular file
			struct stat statbuf;
			memset(&statbuf, 0, sizeof(statbuf));
			stat(globinfo.gl_pathv[i], &statbuf);
			if (S_ISREG(statbuf.st_mode)) {
				if ( !Get_file_list_filter || (*Get_file_list_filter)(globinfo.gl_pathv[i]) ) {
					
					char *found_file = globinfo.gl_pathv[i];
					Assert(strncmp(found_file, base_filespec, base_filespec_len) == 0);
					found_file += base_filespec_len;

					strncpy(arr[num_files], found_file, MAX_FILENAME_LEN - 1 );
					char *ptr = strrchr(arr[num_files], '.');
					if ( ptr ) {
						*ptr = 0;
					}

					if (info)	{
						info[num_files].write_time = statbuf.st_mtime;
					}
					
					num_files++;
				}
			}
		}
		globfree(&globinfo);
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
		free(info);
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
//          filename  - optional, if set, tacks the filename onto end of path.
// Output:  path      - Fully qualified pathname.
void cf_create_default_path_string( char *path, int pathtype, char *filename, bool localize )
{
	if ( filename && strpbrk(filename,"/\\:")  ) {  
		// Already has full path
		strcpy( path, filename );

	} else {
		cf_root *root = cf_get_root(0);

		if (!root) {
			strcpy(path, filename);
			return;
		}

		Assert(CF_TYPE_SPECIFIED(pathtype));

		strcpy(path, root->path);
		strcat(path, Pathtypes[pathtype].path);

		// Don't add slash for root directory
		if (Pathtypes[pathtype].path[0] != '\0') {
		strcat(path, DIR_SEPARATOR_STR);
		}

		// add filename
		if (filename) {
			strcat(path, filename);

			// localize filename
			if (localize) {
				// create copy of path
				char temp_path[MAX_PATH_LEN];
				strcpy(temp_path, path);

				// localize the path
				lcl_add_dir_to_path_with_filename(path);

				// verify localized path
				FILE *fp = fopen(path, "rb");
				if (fp) {
					fclose(fp);
				} else {
					strcpy(path, temp_path);
				}
			}
		}
	}
}

