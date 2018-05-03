/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#define _CFILE_INTERNAL 

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>		/* needed for memory mapping of file functions */
#endif

#ifdef SCP_UNIX
#include <glob.h>
#include <sys/mman.h>
#endif

#include "cfile/cfile.h"
#include "cfile/cfilearchive.h"
#include "cfile/cfilesystem.h"
#include "osapi/osapi.h"
#include "parse/encrypt.h"

#include <limits>

char Cfile_root_dir[CFILE_ROOT_DIRECTORY_LEN] = "";
char Cfile_user_dir[CFILE_ROOT_DIRECTORY_LEN] = "";
#ifdef SCP_UNIX
char Cfile_user_dir_legacy[CFILE_ROOT_DIRECTORY_LEN] = "";
#endif

// During cfile_init, verify that Pathtypes[n].index == n for each item
// Each path must have a valid parent that can be tracable all the way back to the root 
// so that we can create directories when we need to.
//
// Please make sure extensions are all lower-case, or we'll break unix compatibility
//
cf_pathtype Pathtypes[CF_MAX_PATH_TYPES]  = {
	// What type this is          Path																			Extensions        					Parent type
	{ CF_TYPE_INVALID,				NULL,																		NULL,								CF_TYPE_INVALID },
	// Root must be index 1!!
	{ CF_TYPE_ROOT,					"",																			".mve .ogg",						CF_TYPE_ROOT	},
	{ CF_TYPE_DATA,					"data",																		".cfg .txt",						CF_TYPE_ROOT	},
	{ CF_TYPE_MAPS,					"data" DIR_SEPARATOR_STR "maps",											".pcx .ani .eff .tga .jpg .png .dds",	CF_TYPE_DATA	},
	{ CF_TYPE_TEXT,					"data" DIR_SEPARATOR_STR "text",											".txt .net",						CF_TYPE_DATA	},
	{ CF_TYPE_MODELS,				"data" DIR_SEPARATOR_STR "models",											".pof",								CF_TYPE_DATA	},
	{ CF_TYPE_TABLES,				"data" DIR_SEPARATOR_STR "tables",											".tbl .tbm",						CF_TYPE_DATA	},
	{ CF_TYPE_SOUNDS,				"data" DIR_SEPARATOR_STR "sounds",											".wav .ogg",						CF_TYPE_DATA	},
	{ CF_TYPE_SOUNDS_8B22K,			"data" DIR_SEPARATOR_STR "sounds" DIR_SEPARATOR_STR "8b22k",				".wav .ogg",						CF_TYPE_SOUNDS	},
	{ CF_TYPE_SOUNDS_16B11K,		"data" DIR_SEPARATOR_STR "sounds" DIR_SEPARATOR_STR "16b11k",				".wav .ogg",						CF_TYPE_SOUNDS	},
	{ CF_TYPE_VOICE,				"data" DIR_SEPARATOR_STR "voice",											"",									CF_TYPE_DATA	},
	{ CF_TYPE_VOICE_BRIEFINGS,		"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "briefing",				".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_VOICE_CMD_BRIEF,		"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "command_briefings",		".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_VOICE_DEBRIEFINGS,	"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "debriefing",			".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_VOICE_PERSONAS,		"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "personas",				".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_VOICE_SPECIAL,		"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "special",				".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_VOICE_TRAINING,		"data" DIR_SEPARATOR_STR "voice" DIR_SEPARATOR_STR "training",				".wav .ogg",						CF_TYPE_VOICE	},
	{ CF_TYPE_MUSIC,				"data" DIR_SEPARATOR_STR "music",											".wav .ogg",						CF_TYPE_DATA	},
	{ CF_TYPE_MOVIES,				"data" DIR_SEPARATOR_STR "movies",											".mve .msb .ogg .mp4",				CF_TYPE_DATA	},
	{ CF_TYPE_INTERFACE,			"data" DIR_SEPARATOR_STR "interface",										".pcx .ani .dds .tga .eff .png .jpg",	CF_TYPE_DATA	},
	{ CF_TYPE_FONT,					"data" DIR_SEPARATOR_STR "fonts",											".vf .ttf",							CF_TYPE_DATA	},
	{ CF_TYPE_EFFECTS,				"data" DIR_SEPARATOR_STR "effects",											".ani .eff .pcx .neb .tga .jpg .png .dds .sdr",	CF_TYPE_DATA	},
	{ CF_TYPE_HUD,					"data" DIR_SEPARATOR_STR "hud",												".pcx .ani .eff .tga .jpg .png .dds",	CF_TYPE_DATA	},
	{ CF_TYPE_PLAYERS,				"data" DIR_SEPARATOR_STR "players",											".hcf",								CF_TYPE_DATA	},
	{ CF_TYPE_PLAYER_IMAGES,		"data" DIR_SEPARATOR_STR "players" DIR_SEPARATOR_STR "images",				".pcx .png .dds",						CF_TYPE_PLAYERS	},
	{ CF_TYPE_SQUAD_IMAGES,			"data" DIR_SEPARATOR_STR "players" DIR_SEPARATOR_STR "squads",				".pcx .png .dds",						CF_TYPE_PLAYERS	},
	{ CF_TYPE_SINGLE_PLAYERS,		"data" DIR_SEPARATOR_STR "players" DIR_SEPARATOR_STR "single",				".pl2 .cs2 .plr .csg .css",			CF_TYPE_PLAYERS	},
	{ CF_TYPE_MULTI_PLAYERS,		"data" DIR_SEPARATOR_STR "players" DIR_SEPARATOR_STR "multi",				".plr",								CF_TYPE_PLAYERS	},
	{ CF_TYPE_CACHE,				"data" DIR_SEPARATOR_STR "cache",											".clr .tmp .bx",					CF_TYPE_DATA	}, 	//clr=cached color
	{ CF_TYPE_MULTI_CACHE,			"data" DIR_SEPARATOR_STR "multidata",										".pcx .png .dds .fs2 .txt",				CF_TYPE_DATA	},
	{ CF_TYPE_MISSIONS,				"data" DIR_SEPARATOR_STR "missions",										".fs2 .fc2 .ntl .ssv",				CF_TYPE_DATA	},
	{ CF_TYPE_CONFIG,				"data" DIR_SEPARATOR_STR "config",											".cfg .tbl .tbm .xml .csv",			CF_TYPE_DATA	},
	{ CF_TYPE_DEMOS,				"data" DIR_SEPARATOR_STR "demos",											".fsd",								CF_TYPE_DATA	},
	{ CF_TYPE_CBANIMS,				"data" DIR_SEPARATOR_STR "cbanims",											".pcx .ani .eff .tga .jpg .png .dds",	CF_TYPE_DATA	},
	{ CF_TYPE_INTEL_ANIMS,			"data" DIR_SEPARATOR_STR "intelanims",										".pcx .ani .eff .tga .jpg .png .dds",	CF_TYPE_DATA	},
	{ CF_TYPE_SCRIPTS,				"data" DIR_SEPARATOR_STR "scripts",											".lua .lc",							CF_TYPE_DATA	},
	{ CF_TYPE_FICTION,				"data" DIR_SEPARATOR_STR "fiction",											".txt",								CF_TYPE_DATA	}, 
	{ CF_TYPE_FREDDOCS,				"data" DIR_SEPARATOR_STR "freddocs",										".html",							CF_TYPE_DATA	},
};


#define CFILE_STACK_MAX	8

int cfile_inited = 0;
static int Cfile_stack_pos = 0;

static char Cfile_stack[CFILE_STACK_MAX][CFILE_ROOT_DIRECTORY_LEN];

Cfile_block Cfile_block_list[MAX_CFILE_BLOCKS];
static CFILE Cfile_list[MAX_CFILE_BLOCKS];

static const char *Cfile_cdrom_dir = NULL;

//
// Function prototypes for internally-called functions
//
static int cfget_cfile_block();
static CFILE *cf_open_fill_cfblock(const char* source, int line, FILE * fp, int type);
static CFILE *cf_open_packed_cfblock(const char* source, int line, FILE *fp, int type, size_t offset, size_t size);
static CFILE *cf_open_memory_fill_cfblock(const char* source, int line, const void* data, size_t size, int dir_type);

#if defined _WIN32
static CFILE *cf_open_mapped_fill_cfblock(const char* source, int line, HANDLE hFile, int type);
#elif defined SCP_UNIX
static CFILE *cf_open_mapped_fill_cfblock(const char* source, int line, FILE *fp, int type);
#endif

static void cf_chksum_long_init();

static void dump_opened_files()
{
	for (int i = 0; i < MAX_CFILE_BLOCKS; i++) {
		auto cb = &Cfile_block_list[i];
		if (cb->type != CFILE_BLOCK_UNUSED) {
			mprintf(("    %s:%d\n", cb->source_file, cb->line_num));
		}
	}
}

void cfile_close()
{
	mprintf(("Still opened files:\n"));
	dump_opened_files();

	cf_free_secondary_filelist();

	cfile_inited = 0;
}

#ifdef SCP_UNIX
	#define MIN_NUM_PATH_COMPONENTS 2     /* Directory + file */
#else
	#define MIN_NUM_PATH_COMPONENTS 3     /* Drive + directory + file */
#endif

/**
 * @brief Determine if the given path is in the root directory
 *
 * @param exe_path Path to executable
 *
 * @return true if root directory, false if not
 */
static bool cfile_in_root_dir(const char *exe_path)
{
	int new_token;
	int token_count = 0;
	const char *p = exe_path;

	Assert(exe_path != NULL);

	do {
		new_token = 0;
		while (*p == DIR_SEPARATOR_CHAR) {
			p++;
		}

		while ((*p != '\0') && (*p != DIR_SEPARATOR_CHAR)) {
			new_token = 1;
			p++;
		}
		token_count += new_token;
	} while (*p != '\0');

	return (token_count < MIN_NUM_PATH_COMPONENTS);
}

/**
 * @brief Initialize the cfile system. Called once at application start.
 *
 * @param exe_dir Path to a file (not a directory)
 * @param cdrom_dir Path to a CD drive mount point (may be NULL)
 *
 * @return 0 On success
 * @return 1 On error
 */
int cfile_init(const char *exe_dir, const char *cdrom_dir)
{
	// initialize encryption
	encrypt_init();	

	if (cfile_inited) {
		return 0;
	}

	char buf[CFILE_ROOT_DIRECTORY_LEN];

	strncpy(buf, exe_dir, CFILE_ROOT_DIRECTORY_LEN - 1);
	buf[CFILE_ROOT_DIRECTORY_LEN - 1] = '\0';
	size_t i = strlen(buf);

	// are we in a root directory?		
	if(cfile_in_root_dir(buf)){
		os::dialogs::Message(os::dialogs::MESSAGEBOX_ERROR, "FreeSpace2/Fred2 cannot be run from a drive root directory!");
		return 1;
	}		

	// This needs to be set here because cf_build_secondary_filelist assumes it to be true
	cfile_inited = 1;
	
	/*
	 * Determine the executable's directory.  Note that DIR_SEPARATOR_CHAR
	 * is guaranteed to be found in the string else cfile_in_root_dir()
	 * would have failed.
	 */

	char *p;

	p = strrchr(buf, DIR_SEPARATOR_CHAR);
	*p = '\0';

	cfile_chdir(buf);

	// set root directory
	strncpy(Cfile_root_dir, buf, CFILE_ROOT_DIRECTORY_LEN-1);
	strncpy(Cfile_user_dir, os_get_config_path().c_str(), CFILE_ROOT_DIRECTORY_LEN-1);
	
#ifdef SCP_UNIX
	// Initialize path of old pilot files
	extern const char* Osreg_user_dir_legacy;
	snprintf(Cfile_user_dir_legacy, CFILE_ROOT_DIRECTORY_LEN-1, "%s/%s/", getenv("HOME"), Osreg_user_dir_legacy);
#endif

	for (i = 0; i < MAX_CFILE_BLOCKS; i++) {
		Cfile_block_list[i].type = CFILE_BLOCK_UNUSED;
	}

	// 32 bit CRC table init
	cf_chksum_long_init();

	Cfile_cdrom_dir = cdrom_dir;
	cf_build_secondary_filelist(Cfile_cdrom_dir);

	return 0;
}

// Call this if pack files got added or removed or the
// cdrom changed.  This will refresh the list of filenames 
// stored in packfiles and on the cdrom.
void cfile_refresh()
{
	cf_build_secondary_filelist(Cfile_cdrom_dir);
}



#ifdef _WIN32
// Changes to a drive if valid.. 1=A, 2=B, etc
// If flag, then changes to it.
// Returns 0 if not-valid, 1 if valid.
int cfile_chdrive( int DriveNum, int flag )
{
	int Valid = 0;
	int n, org;

	org = -1;
	if (!flag)
		org = _getdrive();

	_chdrive( DriveNum );
	n = _getdrive();


	if (n == DriveNum )
		Valid = 1;

	if ( (!flag) && (n != org) )
		_chdrive( org );

	return Valid;

}
#endif // _WIN32

/**
 * @brief Common code for changing directory
 *
 * @param new_dir Directory to which to change
 * @param cur_dir Current directory (only used on Windows)
 *
 * @retval 0 Success
 * @retval 1 Failed to change to new directory's drive (Windows only)
 * @retval 2 Failed to change to new directory
 */
static int _cfile_chdir(const char *new_dir, const char *cur_dir __UNUSED)
{
	int status;
	const char *path = NULL;
	const char no_dir[] = "\\.";

#ifdef _WIN32
	const char *colon = strchr(new_dir, ':');

	if (colon) {
		if (!cfile_chdrive(tolower(*(colon - 1)) - 'a' + 1, 1))
			return 1;

		path = colon + 1;
	} else
#endif /* _WIN32 */
	{
		path = new_dir;
	}

	if (*path == '\0') {
		path = no_dir;
	}

	/* This chdir might get a critical error! */
	status = _chdir(path);
	if (status != 0) {
#ifdef _WIN32
		cfile_chdrive(tolower(cur_dir[0]) - 'a' + 1, 1);
#endif /* _WIN32 */
		return 2;
	}

	return 0;
}

/**
 * @brief Push current directory onto a 'stack' and change to a new directory
 *
 * The current directory is pushed onto a 'stack' so that it can be easily
 * restored at a later time. The new directory is derived from @a type.
 *
 * @param type path type (CF_TYPE_xxx)
 *
 * @retval -1 'Stack' is full
 * @retval  0 Success
 * @retval  1 Failed to change to new directory's drive (Windows only)
 * @retval  2 Failed to change to new directory
 */
int cfile_push_chdir(int type)
{
	char dir[CFILE_ROOT_DIRECTORY_LEN];
	char OriginalDirectory[CFILE_ROOT_DIRECTORY_LEN];

	_getcwd(OriginalDirectory, CFILE_ROOT_DIRECTORY_LEN - 1);

	Assert(Cfile_stack_pos < CFILE_STACK_MAX);

	if (Cfile_stack_pos >= CFILE_STACK_MAX) {
		return -1;
	}

	strncpy(Cfile_stack[Cfile_stack_pos++], OriginalDirectory,
	        CFILE_ROOT_DIRECTORY_LEN - 1);

	cf_create_default_path_string(dir, sizeof(dir) - 1, type, NULL);

	return _cfile_chdir(dir, OriginalDirectory);
}

/**
 * @brief Change to the specified directory
 *
 * @param dir Directory
 *
 * @retval  0 Success
 * @retval  1 Failed to change to new directory's drive (Windows only)
 * @retval  2 Failed to change to new directory
 */
int cfile_chdir(const char *dir)
{
	char OriginalDirectory[CFILE_ROOT_DIRECTORY_LEN];

	_getcwd(OriginalDirectory, CFILE_ROOT_DIRECTORY_LEN - 1);

	return _cfile_chdir(dir, OriginalDirectory);
}

int cfile_pop_dir()
{
	Assert(Cfile_stack_pos);

	if ( !Cfile_stack_pos )
		return -1;

	Cfile_stack_pos--;
	return cfile_chdir(Cfile_stack[Cfile_stack_pos]);
}

// flush (delete all files in) the passed directory (by type), return the # of files deleted
// NOTE : WILL NOT DELETE READ-ONLY FILES
int cfile_flush_dir(int dir_type)
{
	int del_count;

	Assert( CF_TYPE_SPECIFIED(dir_type) );

	// attempt to change the directory to the passed type
	if(cfile_push_chdir(dir_type)){
		return 0;
	}

	// proceed to delete the files
	del_count = 0;
#if defined _WIN32
	intptr_t find_handle;
	_finddata_t find;
	find_handle = _findfirst( "*", &find );
	if (find_handle != -1) {
		do {			
			if (!(find.attrib & _A_SUBDIR) && !(find.attrib & _A_RDONLY)) {
				// delete the file
				cf_delete(find.name,dir_type);				

				// increment the deleted count
				del_count++;
			}
		} while (!_findnext(find_handle, &find));
		_findclose( find_handle );
	}
#elif defined SCP_UNIX
	glob_t globinfo;
	memset(&globinfo, 0, sizeof(globinfo));
	int status = glob("*", 0, NULL, &globinfo);
	if (status == 0) {
		for (unsigned int i = 0;  i < globinfo.gl_pathc;  i++) {
			// Determine if this is a regular file
			struct stat statbuf;

			stat(globinfo.gl_pathv[i], &statbuf);
			if (S_ISREG(statbuf.st_mode)) {
				// delete the file
				cf_delete(globinfo.gl_pathv[i], dir_type);				

				// increment the deleted count
				del_count++;				
			}
		}
		globfree(&globinfo);
	}
#endif

	// pop the directory back
	cfile_pop_dir();

	// return the # of files deleted
	return del_count;
}


// add the given extention to a filename (or filepath) if it doesn't already have this
// extension.
//    filename = name of filename or filepath to process
//    ext = extension to add.  Must start with the period
//    Returns: new filename or filepath with extension.
char *cf_add_ext(const char *filename, const char *ext)
{
	static char path[MAX_PATH_LEN];

	size_t flen = strlen(filename);
	size_t elen = strlen(ext);
	Assert(flen < MAX_PATH_LEN);
	strcpy_s(path, filename);
	if ((flen < 4) || stricmp(path + flen - elen, ext) != 0) {
		Assert(flen + elen < MAX_PATH_LEN);
		strcat_s(path, ext);
	}

	return path;
}

/**
 * @brief Delete the specified file
 *
 * @param filename Name of file to delete
 * @param path_type Path type (CF_TYPE_xxx)
 *
 * @return 0 on failure, 1 on success
 */
int cf_delete(const char *filename, int path_type)
{
	char longname[MAX_PATH_LEN];

	Assert(CF_TYPE_SPECIFIED(path_type));

	cf_create_default_path_string(longname, sizeof(longname) - 1,
	                              path_type, filename);

	return (_unlink(longname) != -1);
}


// Same as _access function to read a file's access bits
int cf_access(const char *filename, int dir_type, int mode)
{
	char longname[MAX_PATH_LEN];

	Assert( CF_TYPE_SPECIFIED(dir_type) );

	cf_create_default_path_string( longname, sizeof(longname)-1, dir_type, filename );

	return access(longname,mode);
}


// Returns 1 if the file exists, 0 if not.
// Checks only the file system.
// cf_find_file_location checks the filesystem before VPs
// If offset is 0, it was found in the filesystem, so offset is boolean false
// If offset equates to boolean true, it was found in a VP and the logic will negate the function return
int cf_exists(const char *filename, int dir_type)
{
	size_t offset = 1;

	if ( (filename == NULL) || !strlen(filename) )
		return 0;

	return (cf_find_file_location(filename, dir_type, 0, NULL, NULL, &offset) && !offset);
}

// Goober5000
// Returns !0 if the file exists, 0 if not.
// Checks both the file system and the VPs.
int cf_exists_full(const char *filename, int dir_type)
{
	if ( (filename == NULL) || !strlen(filename) )
		return 0;

	return cf_find_file_location(filename, dir_type, 0, NULL, NULL, NULL);
}

// same as the above, but with extension check
int cf_exists_full_ext(const char *filename, int dir_type, const int num_ext, const char **ext_list)
{
	if ( (filename == NULL) || !strlen(filename) )
		return 0;

	if ( (num_ext <= 0) || (ext_list == NULL) )
		return 0;

	return (cf_find_file_location_ext(filename, num_ext, ext_list, dir_type, 0, NULL, NULL, NULL) != -1);
}

#ifdef _WIN32
void cf_attrib(const char *filename, int set, int clear, int dir_type)
{
	char longname[MAX_PATH_LEN];

	Assert( CF_TYPE_SPECIFIED(dir_type) );

	cf_create_default_path_string( longname, sizeof(longname)-1, dir_type, filename );

	FILE *fp = fopen(longname, "rb");
	if (fp) {
		fclose(fp);

		DWORD z = GetFileAttributes(longname);
		SetFileAttributes(longname, z | (set & ~clear));
	}

}
#endif

int cf_rename(const char *old_name, const char *name, int dir_type)
{
	Assert( CF_TYPE_SPECIFIED(dir_type) );

	int ret_code;
	char old_longname[_MAX_PATH];
	char new_longname[_MAX_PATH];
	
	cf_create_default_path_string( old_longname, sizeof(old_longname)-1, dir_type, old_name );
	cf_create_default_path_string( new_longname, sizeof(old_longname)-1, dir_type, name );

	ret_code = rename(old_longname, new_longname );		
	if(ret_code != 0){
		switch(errno){
		case EACCES :
			return CF_RENAME_FAIL_ACCESS;
		case ENOENT :
		default:
			return CF_RENAME_FAIL_EXIST;
		}
	}

	return CF_RENAME_SUCCESS;
	

}


// This takes a path (e.g. "C:\Games\FreeSpace2\Lots\More\Directories") and creates it in its entirety.
// Do note that this requires the path to have normalized directory separators as defined by DIR_SEPARATOR_CHAR
static void mkdir_recursive(const char *path) {
    size_t pre = 0, pos;
    SCP_string tmp(path);
    SCP_string dir;

    if (tmp[tmp.size() - 1] != DIR_SEPARATOR_CHAR) {
        // force trailing / so we can handle everything in loop
        tmp += DIR_SEPARATOR_CHAR;
    }

    while ((pos = tmp.find_first_of(DIR_SEPARATOR_CHAR, pre)) != std::string::npos) {
        dir = tmp.substr(0, pos++);
        pre = pos;
        if (dir.empty()) continue; // if leading / first time is 0 length
        
        _mkdir(dir.c_str());
    }
}

// Creates the directory path if it doesn't exist. Even creates all its
// parent paths.
void cf_create_directory( int dir_type )
{
	int num_dirs = 0;
	int dir_tree[CF_MAX_PATH_TYPES];
	char longname[MAX_PATH_LEN];
	struct stat statbuf;

	Assertion( CF_TYPE_SPECIFIED(dir_type), "Invalid dir_type passed to cf_create_directory." );

	int current_dir = dir_type;

	do {
		Assert( num_dirs < CF_MAX_PATH_TYPES );		// Invalid Pathtypes data?

		dir_tree[num_dirs++] = current_dir;
		current_dir = Pathtypes[current_dir].parent_index;

	} while( current_dir != CF_TYPE_ROOT );

	int i;

	for (i=num_dirs-1; i>=0; i-- )	{
		cf_create_default_path_string( longname, sizeof(longname)-1, dir_tree[i], NULL );
		if (stat(longname, &statbuf) != 0) {
			mprintf(( "CFILE: Creating new directory '%s'\n", longname ));
			mkdir_recursive(longname);
		}
	}
}


extern int game_cd_changed();

// cfopen()
//
// parameters:  *filepath ==> name of file to open (may be path+name)
//              *mode     ==> specifies how file should be opened (eg "rb" for read binary)
//                            passing NULL to mode deletes the file if it exists and returns NULL
//               type     ==> one of:    CFILE_NORMAL
//                                       CFILE_MEMORY_MAPPED
//					  dir_type	=>	override extension check, value is one of CF_TYPE* #defines
//
//               NOTE: type parameter is an optional parameter.  The default value is CFILE_NORMAL
//
//
// returns:		success ==> address of CFILE structure
//					error   ==> NULL
//

CFILE *_cfopen(const char* source, int line, const char *file_path, const char *mode, int type, int dir_type, bool localize)
{
	/* Bobboau, what is this doing here? 31 is way too short... - Goober5000
	if( strlen(file_path) > 31 )
		Error(LOCATION, "file name %s too long, \nmust be less than 31 charicters", file_path);*/

	char longname[_MAX_PATH];

	if ( !cfile_inited ) {
		Int3();
		return NULL;
	}

	//================================================
	// Check that all the parameters make sense
	Assert(file_path && strlen(file_path));
	Assert( mode != NULL );
	
	// Can only open read-only binary files in memory mapped mode.
	if ( (type & CFILE_MEMORY_MAPPED) && strcmp(mode,"rb") != 0 ) {
		Int3();				
		return NULL;
	}

	//===========================================================
	// If in write mode, just try to open the file straight off
	// the harddisk.  No fancy packfile stuff here!
	
	if ( strchr(mode,'w') || strchr(mode,'+') || strchr(mode,'a') )	{
		// For write-only files, require a full path or a path type
#ifdef SCP_UNIX
		if ( strpbrk(file_path, "/") ) {
#else
		if ( strpbrk(file_path,"/\\:")  ) {
#endif
			// Full path given?
			strcpy_s(longname, file_path );
		} else {
			// Path type given?
			Assert( dir_type != CF_TYPE_ANY );

			// Create the directory if necessary
			cf_create_directory( dir_type );

			cf_create_default_path_string( longname, sizeof(longname)-1, dir_type, file_path );
		}
		Assert( !(type & CFILE_MEMORY_MAPPED) );

		// JOHN: TODO, you should create the path if it doesn't exist.
		
		//WMC - For some godawful reason, fread does not return the correct number of bytes read
		//in text mode, which messes up FS2_Open's raw_position indicator in fgets. As a consequence, you
		//_must_ open files that are gonna be read in binary mode.

		char happy_mode[8];
		if(strcspn(mode, "ra+") != strlen(mode) && (strchr(mode, 't') || !strchr(mode, 'b')))
		{
			//*****BEGIN PROCESSING OF MODE*****
			//Copies all 'mode' characters over, except for t, and adds b if needed.
			unsigned int max = sizeof(happy_mode) - 2;	//space for null and 'b'
			bool need_b = true;
			unsigned int i;
			for( i = 0; i < strlen(mode); i++)
			{
				if(i > max)
					break;

				if(mode[i] != 't')
					happy_mode[i] = mode[i];

				if(mode[i] == 'b')
					need_b = false;
			}
			happy_mode[i] = '\0';
			if(need_b)
				strcat_s(happy_mode, "b");
			//*****END PROCESSING OF MODE*****
		}
		else
		{
			strcpy_s(happy_mode, mode);
		}

		FILE *fp = fopen(longname, happy_mode);
		if (fp)	{
			return cf_open_fill_cfblock(source, line, fp, dir_type);
 		}
		return NULL;
	} 


	//================================================
	// Search for file on disk, on cdrom, or in a packfile

	size_t offset, size;
	char copy_file_path[MAX_PATH_LEN];  // FIX change in memory from cf_find_file_location
	strcpy_s(copy_file_path, file_path);

	const void* file_data = nullptr;
	if ( cf_find_file_location( copy_file_path, dir_type, sizeof(longname) - 1, longname, &size, &offset, localize, &file_data ) )	{

		// Fount it, now create a cfile out of it
		nprintf(("CFileDebug", "Requested file %s found at: %s\n", file_path, longname));
		
		if ( type & CFILE_MEMORY_MAPPED ) {
		
			// Can't open memory mapped files out of pack or memory files
			if ( offset == 0 && file_data != nullptr )	{
#if defined _WIN32
				HANDLE hFile;

				hFile = CreateFile(longname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)	{
					return cf_open_mapped_fill_cfblock(source, line, hFile, dir_type);
				}
#elif defined SCP_UNIX
				FILE *fp = fopen( longname, "rb" );
				if (fp) {
					return cf_open_mapped_fill_cfblock(source, line, fp, dir_type);
				}
#endif
			} 

		} else {
			// since cfopen_special already has all the code to handle the opening we can just use that here
			return _cfopen_special(source, line, longname, mode, size, offset, file_data, dir_type);
		}

	}

	return NULL;
}

// cfopen_ext()
//
// parameters:	*filepath	==> name of file to open (may be path+name)
//				*mode		==> specifies how file should be opened (eg "rb" for read binary)
//								passing NULL to mode deletes the file if it exists and returns NULL

//				dir_type	=>	override extension check, value is one of CF_TYPE* #defines
//
// returns:		success	==> address of CFILE structure
//				error	==> NULL
//
CFILE *_cfopen_special(const char* source, int line, const char *file_path, const char *mode, const size_t size, const size_t offset, const void* data, int dir_type)
{
	if ( !cfile_inited) {
		Int3();
		return NULL;
	}

	Assert( file_path && strlen(file_path) );
	Assert( mode != NULL );

	// cfopen_special() only supports reading files, not creating them
	if ( strchr(mode, 'w') ) {
		Int3();
		return NULL;
	}

	// In-Memory files are a bit different from normal files so we need to handle them separately
	if (data != nullptr) {
		return cf_open_memory_fill_cfblock(source, line, data, size, dir_type);
	}
	else {
		// "file_path" should already be a fully qualified path, so just try to open it
		FILE *fp = fopen(file_path, "rb");

		if (fp) {
			if (offset) {
				// Found it in a pack file
				return cf_open_packed_cfblock(source, line, fp, dir_type, offset, size);
			}
			else {
				// Found it in a normal file
				return cf_open_fill_cfblock(source, line, fp, dir_type);
			}
		}
	}

	return NULL;
}


// ------------------------------------------------------------------------
// ctmpfile() 
//
// Open up a temporary file.  A unique name is automatically generated.  The
// file will be automatically deleted when file is closed.
//
// return:		NULL					=>		tmp file could not be opened
//					pointer to CFILE	=>		tmp file successfully opened
//
CFILE *ctmpfile()
{
	FILE	*fp;
	fp = tmpfile();
	if ( fp )
		return cf_open_fill_cfblock(LOCATION, fp, 0);
	else
		return NULL;
}



// cfget_cfile_block() will try to find an empty Cfile_block structure in the
//	Cfile_block_list[] array and return the index.
//
// returns:   success ==> index in Cfile_block_list[] array
//            failure ==> -1
//
static int cfget_cfile_block()
{	
	int i;
	Cfile_block *cb;

	for ( i = 0; i < MAX_CFILE_BLOCKS; i++ ) {
		cb = &Cfile_block_list[i];
		if ( cb->type == CFILE_BLOCK_UNUSED ) {
			cb->data = NULL;
			cb->fp = NULL;
			cb->type = CFILE_BLOCK_USED;
			return i;
		}
	}

	// If we've reached this point, a free Cfile_block could not be found
	nprintf(("Warning","A free Cfile_block could not be found.\n"));

	// Dump a list of all opened files
	mprintf(("Out of cfile blocks! Currently opened files:\n"));
	dump_opened_files();

	Assertion(false, "There are no more free cfile blocks. This means that there are too many files opened by FSO.\n"
		"This is probably caused by a programming or scripting error where a file does not get closed."); // out of free cfile blocks
	return -1;			
}


// cfclose() closes the file
//
// returns:   success ==> 0
//				  failure ==> EOF
//
int cfclose( CFILE * cfile )
{
	int result;

	Assert(cfile != NULL);
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	result = 0;
	if ( cb->data && cb->mem_mapped ) {
		// close memory mapped file
#if defined _WIN32
		result = UnmapViewOfFile((void*)cb->data);
		Assert(result);
		result = CloseHandle(cb->hInFile);		
		Assert(result);	// Ensure file handle is closed properly
		result = CloseHandle(cb->hMapFile);		
		Assert(result);	// Ensure file handle is closed properly
		result = 0;
#elif defined SCP_UNIX
		// FIXME: result is wrong after munmap() but it is successful
		//result = munmap(cb->data, cb->data_length);
		//Assert(result);
		// This const_cast is safe since the pointer returned by mmap was also non-const
		munmap(const_cast<void*>(cb->data), cb->data_length);
		if ( cb->fp != NULL)
			result = fclose(cb->fp);
#endif

	} else if ( cb->fp != NULL )	{
		Assert(cb->fp != NULL);
		result = fclose(cb->fp);
	} else {
		// VP  do nothing
	}

	cb->type = CFILE_BLOCK_UNUSED;
	return result;
}

int cf_is_valid(CFILE *cfile)
{
	//Was a valid pointer passed?
	if(cfile == NULL)
		return 0;

	//Does it have a valid ID?
	if(cfile->id < 0 || cfile->id >= MAX_CFILE_BLOCKS)
		return 0;

	//Is it used?
	Cfile_block *cb = &Cfile_block_list[cfile->id];	
	if(cb->type != CFILE_BLOCK_USED && (cb->fp != NULL || cb->data != NULL))
		return 0;

	//It's good, as near as we can tell.
	return 1;
}




// cf_open_fill_cfblock() will fill up a Cfile_block element in the Cfile_block_list[] array
// for the case of a file being opened by cf_open();
//
// returns:   success ==> ptr to CFILE structure.  
//            error   ==> NULL
//
static CFILE *cf_open_fill_cfblock(const char* source, int line, FILE *fp, int type)
{
	int cfile_block_index;

	cfile_block_index = cfget_cfile_block();
	if ( cfile_block_index == -1 ) {
		fclose(fp);
		return NULL;
	} else {
		CFILE *cfp;
		Cfile_block *cfbp;
		cfbp = &Cfile_block_list[cfile_block_index];
		cfp = &Cfile_list[cfile_block_index];
		cfp->id = cfile_block_index;
		cfp->version = 0;
		cfbp->data = NULL;
		cfbp->mem_mapped = false;
		cfbp->fp = fp;
		cfbp->dir_type = type;
		cfbp->max_read_len = 0;

		cfbp->source_file = source;
		cfbp->line_num = line;
		
		int pos = ftell(fp);
		if(pos == -1L)
			pos = 0;
		cf_init_lowlevel_read_code(cfp,0,filelength(fileno(fp)), 0 );

		return cfp;
	}
}


// cf_open_packed_cfblock() will fill up a Cfile_block element in the Cfile_block_list[] array
// for the case of a file being opened by cf_open();
//
// returns:   success ==> ptr to CFILE structure.  
//            error   ==> NULL
//
static CFILE *cf_open_packed_cfblock(const char* source, int line, FILE *fp, int type, size_t offset, size_t size)
{
	// Found it in a pack file
	int cfile_block_index;
	
	cfile_block_index = cfget_cfile_block();
	if ( cfile_block_index == -1 ) {
		fclose(fp);
		return NULL;
	} else {
		CFILE *cfp;
		Cfile_block *cfbp;
		cfbp = &Cfile_block_list[cfile_block_index];
	
		cfp = &Cfile_list[cfile_block_index];
		cfp->id = cfile_block_index;
		cfp->version = 0;
		cfbp->data = NULL;
		cfbp->fp = fp;
		cfbp->mem_mapped = false;
		cfbp->dir_type = type;
		cfbp->max_read_len = 0;

		cfbp->source_file = source;
		cfbp->line_num = line;

		cf_init_lowlevel_read_code(cfp,offset, size, 0 );

		return cfp;
	}

}



// cf_open_mapped_fill_cfblock() will fill up a Cfile_block element in the Cfile_block_list[] array
// for the case of a file being opened by cf_open_mapped();
//
// returns:   ptr CFILE structure.  
//
#if defined _WIN32
static CFILE *cf_open_mapped_fill_cfblock(const char* source, int line, HANDLE hFile, int type)
#elif defined SCP_UNIX
static CFILE *cf_open_mapped_fill_cfblock(const char* source, int line, FILE *fp, int type)
#endif
{
	int cfile_block_index;

	cfile_block_index = cfget_cfile_block();
	if ( cfile_block_index == -1 ) {
#ifdef SCP_UNIX
		fclose(fp);
#endif
		return NULL;
	}
	else {
		CFILE *cfp;
		Cfile_block *cfbp;
		cfbp = &Cfile_block_list[cfile_block_index];

		cfp = &Cfile_list[cfile_block_index];
		cfp->id = cfile_block_index;
		cfp->version = 0;
		cfbp->max_read_len = 0;
		cfbp->fp = NULL;
		cfbp->mem_mapped = true;
#if defined _WIN32
		cfbp->hInFile = hFile;
#endif
		cfbp->dir_type = type;

		cfbp->source_file = source;
		cfbp->line_num = line;

		cf_init_lowlevel_read_code(cfp, 0, 0, 0 );
#if defined _WIN32
		cfbp->hMapFile = CreateFileMapping(cfbp->hInFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (cfbp->hMapFile == NULL) { 
			nprintf(("Error", "Could not create file-mapping object.\n")); 
			return NULL;
		} 
	
		cfbp->data = (ubyte*)MapViewOfFile(cfbp->hMapFile, FILE_MAP_READ, 0, 0, 0);
		Assert( cfbp->data != NULL );		
#elif defined SCP_UNIX
		cfbp->fp = fp;
		cfbp->data_length = filelength( fileno(fp) );
		cfbp->data = mmap(NULL,						// start
								cfbp->data_length,	// length
								PROT_READ,				// prot
								MAP_SHARED,				// flags
								fileno(fp),				// fd
								0);						// offset
		Assert( cfbp->data != NULL );		
#endif

		return cfp;
	}
}

static CFILE *cf_open_memory_fill_cfblock(const char* source, int line, const void* data, size_t size, int dir_type)
{
	int cfile_block_index;

	cfile_block_index = cfget_cfile_block();
	if ( cfile_block_index == -1 ) {
		return NULL;
	}
	else {
		CFILE *cfp;
		Cfile_block *cfbp;
		cfbp = &Cfile_block_list[cfile_block_index];

		cfp = &Cfile_list[cfile_block_index];
		cfp->id = cfile_block_index;
		cfp->version = 0;
		cfbp->max_read_len = 0;
		cfbp->fp = NULL;
		cfbp->mem_mapped = false;
		cfbp->dir_type = dir_type;

		cfbp->source_file = source;
		cfbp->line_num = line;

		cf_init_lowlevel_read_code(cfp, 0, size, 0 );
		cfbp->data = data;

		return cfp;
	}
}

int cf_get_dir_type(CFILE *cfile)
{
	return Cfile_block_list[cfile->id].dir_type;
}

// cf_returndata() returns the data pointer for a memory-mapped file that is associated
// with the CFILE structure passed as a parameter
//
// 

const void *cf_returndata(CFILE *cfile)
{
	Assert(cfile != NULL);
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	
	Assert(cb->data != NULL);
	return cb->data;
}



// version number of opened file.  Will be 0 unless you put something else here after you
// open a file.  Once set, you can use minimum version numbers with the read functions.
void cf_set_version( CFILE * cfile, int version )
{
	Assert(cfile != NULL);

	cfile->version = version;
}

// cutoff point where cfread() will throw an error when it hits this limit
// if 'len' is 0 then this check will be disabled
void cf_set_max_read_len( CFILE * cfile, size_t len )
{
	Assert( cfile != NULL );
	Assert( (cfile->id >= 0) && (cfile->id < MAX_CFILE_BLOCKS) );

	Cfile_block *cb = &Cfile_block_list[cfile->id];

	if (len) {
		cb->max_read_len = cb->raw_position + len;
	} else {
		cb->max_read_len = 0;
	}
}

// routines to read basic data types from CFILE's.  Put here to
// simplify mac/pc reading from cfiles.

float cfread_float(CFILE *file, int ver, float deflt)
{
	float f;

	if (file->version < ver)
		return deflt;

	if (cfread( &f, sizeof(f), 1, file) != 1)
		return deflt;

	f = INTEL_FLOAT(&f);
	return f;
}

int cfread_int(CFILE *file, int ver, int deflt)
{
	int i;

	if (file->version < ver)
		return deflt;

	if (cfread( &i, sizeof(i), 1, file) != 1)
		return deflt;

	i = INTEL_INT(i);
	return i;
}

uint cfread_uint(CFILE *file, int ver, uint deflt)
{
	uint i;

	if (file->version < ver)
		return deflt;

	if (cfread( &i, sizeof(i), 1, file) != 1)
		return deflt;

	i = INTEL_INT(i);
	return i;
}

short cfread_short(CFILE *file, int ver, short deflt)
{
	short s;

	if (file->version < ver)
		return deflt;

	if (cfread( &s, sizeof(s), 1, file) != 1)
		return deflt;

	s = INTEL_SHORT(s);
	return s;
}

ushort cfread_ushort(CFILE *file, int ver, ushort deflt)
{
	ushort s;

	if (file->version < ver)
		return deflt;

	if (cfread( &s, sizeof(s), 1, file) != 1)
		return deflt;

	s = INTEL_SHORT(s);
	return s;
}

ubyte cfread_ubyte(CFILE *file, int ver, ubyte deflt)
{
	ubyte b;

	if (file->version < ver)
		return deflt;

	if (cfread( &b, sizeof(b), 1, file) != 1)
		return deflt;

	return b;
}

void cfread_vector(vec3d *vec, CFILE *file, int ver, vec3d *deflt)
{
	if (file->version < ver) {
		if (deflt)
			*vec = *deflt;
		else
			vec->xyz.x = vec->xyz.y = vec->xyz.z = 0.0f;

		return;
	}

	vec->xyz.x = cfread_float(file, ver, deflt ? deflt->xyz.x : 0.0f);
	vec->xyz.y = cfread_float(file, ver, deflt ? deflt->xyz.y : 0.0f);
	vec->xyz.z = cfread_float(file, ver, deflt ? deflt->xyz.z : 0.0f);
}
	
void cfread_angles(angles *ang, CFILE *file, int ver, angles *deflt)
{
	if (file->version < ver) {
		if (deflt)
			*ang = *deflt;
		else
			ang->p = ang->b = ang->h = 0.0f;

		return;
	}

	ang->p = cfread_float(file, ver, deflt ? deflt->p : 0.0f);
	ang->b = cfread_float(file, ver, deflt ? deflt->b : 0.0f);
	ang->h = cfread_float(file, ver, deflt ? deflt->h : 0.0f);
}

char cfread_char(CFILE *file, int ver, char deflt)
{
	char b;

	if (file->version < ver)
		return deflt;

	if (cfread( &b, sizeof(b), 1, file) != 1)
		return deflt;

	return b;
}

void cfread_string(char *buf, int n, CFILE *file)
{
	char c;

	do {
		c = cfread_char(file);
		if ( n > 0 )	{
			*buf++ = c;
			n--;
		}
	} while (c != 0 );
}

void cfread_string_len(char *buf,int n, CFILE *file)
{
	int len;
	len = cfread_int(file);
	Assertion( (len < n), "len: %i, n: %i", len, n );
	if (len)
		cfread(buf, len, 1, file);

	buf[len] = 0;
}

// equivalent write functions of above read functions follow

int cfwrite_float(float f, CFILE *file)
{
	f = INTEL_FLOAT(&f);
	return cfwrite(&f, sizeof(f), 1, file);
}

int cfwrite_int(int i, CFILE *file)
{
	i = INTEL_INT(i);
	return cfwrite(&i, sizeof(i), 1, file);
}

int cfwrite_uint(uint i, CFILE *file)
{
	i = INTEL_INT(i);
	return cfwrite(&i, sizeof(i), 1, file);
}

int cfwrite_short(short s, CFILE *file)
{
	s = INTEL_SHORT(s);
	return cfwrite(&s, sizeof(s), 1, file);
}

int cfwrite_ushort(ushort s, CFILE *file)
{
	s = INTEL_SHORT(s);
	return cfwrite(&s, sizeof(s), 1, file);
}

int cfwrite_ubyte(ubyte b, CFILE *file)
{
	return cfwrite(&b, sizeof(b), 1, file);
}

int cfwrite_vector(vec3d *vec, CFILE *file)
{
	if(!cfwrite_float(vec->xyz.x, file)){
		return 0;
	}
	if(!cfwrite_float(vec->xyz.y, file)){
		return 0;
	}
	return cfwrite_float(vec->xyz.z, file);
}

int cfwrite_angles(angles *ang, CFILE *file)
{
	if(!cfwrite_float(ang->p, file)){
		return 0;
	}
	if(!cfwrite_float(ang->b, file)){
		return 0;
	}
	return cfwrite_float(ang->h, file);
}

int cfwrite_char(char b, CFILE *file)
{
	return cfwrite( &b, sizeof(b), 1, file);
}

int cfwrite_string(const char *buf, CFILE *file)
{
	if ( (!buf) || (buf && !buf[0]) ) {
		return cfwrite_char(0, file);
	} 
	int len = (int)strlen(buf);
	if(!cfwrite(buf, len, 1, file)){
		return 0;
	}
	return cfwrite_char(0, file);			// write out NULL termination			
}

int cfwrite_string_len(const char *buf, CFILE *file)
{
	int len = (int)strlen(buf);

	if(!cfwrite_int(len, file)){
		return 0;
	}
	if (len){
		return cfwrite(buf,len,1,file);
	} 

	return 1;
}

// Get the filelength
int cfilelength( CFILE * cfile )
{
	Assert(cfile != NULL);
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	// TODO: return length of memory mapped file
	Assert( !cb->mem_mapped );

	// cb->size gets set at cfopen
	
	// The rest of the code still uses ints, do an overflow check to detect cases where this fails
	Assertion(cb->size <= static_cast<size_t>(std::numeric_limits<int>::max()),
		"Integer overflow in cfilelength! A file is too large (but I don't know which...).");
	return (int) cb->size;
}

// cfwrite() writes to the file
//
// returns:   number of full elements actually written
//            
//
int cfwrite(const void *buf, int elsize, int nelem, CFILE *cfile)
{
	if(!cf_is_valid(cfile))
		return 0;

	if(buf == NULL || elsize == 0 || nelem == 0)
		return 0;

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	if(cb->lib_offset != 0)
	{
		Error(LOCATION, "Attempt to write to a VP file (unsupported)");
		return 0;
	}

	if(cb->data != NULL)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return EOF;
	}

	size_t bytes_written = 0;
	size_t size = elsize * nelem;

	bytes_written = fwrite(buf, 1, size, cb->fp);

	//WMC - update filesize and position
	if (bytes_written > 0) {
		cb->size += bytes_written;
		cb->raw_position += bytes_written;
	}

#if defined(CHECK_SIZE) && !defined(NDEBUG)
	Assert( cb->size == filelength(fileno(cb->fp)) );
#endif

	return (int)(bytes_written / elsize);
}


// cfputc() writes a character to a file
//
// returns:   success ==> returns character written
//				  error   ==> EOF
//
int cfputc(int c, CFILE *cfile)
{
	if(!cf_is_valid(cfile))
		return EOF;

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	if(cb->lib_offset != 0)
	{
		Error(LOCATION, "Attempt to write character to a VP file (unsupported)");
		return EOF;
	}

	if(cb->data != NULL)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return EOF;
	}

	// writing not supported for memory-mapped files
	Assert( !cb->data );

	int result = fputc(c, cb->fp);

	//WMC - update filesize and position
	if(result != EOF)
	{
		cb->size += 1;
		cb->raw_position += 1;
	}

#if defined(CHECK_SIZE) && !defined(NDEBUG)
	Assert( cb->size == filelength(fileno(cb->fp)) );
#endif

	return result;	
}

// cfputs() writes a string to a file
//
// returns:   success ==> non-negative value
//				  error   ==> EOF
//
int cfputs(const char *str, CFILE *cfile)
{
	if(!cf_is_valid(cfile))
		return EOF;

	if(str == NULL)
		return EOF;

	Cfile_block *cb = &Cfile_block_list[cfile->id];	

	if(cb->lib_offset != 0)
	{
		Error(LOCATION, "Attempt to write character to a VP file (unsupported)");
		return EOF;
	}

	if(cb->data != NULL)
	{
		Warning(LOCATION, "Writing is not supported for mem-mapped files");
		return EOF;
	}

	int result = fputs(str, cb->fp);

	//WMC - update filesize and position
	if(result != EOF)
	{
		cb->size += strlen(str);
		cb->raw_position += strlen(str);
	}

#if defined(CHECK_SIZE) && !defined(NDEBUG)
	Assert( cb->size == filelength(fileno(cb->fp)) );
#endif

	return result;	
}


// cfgetc() reads a character from a file
//
// returns:   success ==> returns character read
//				  error   ==> EOF
//
int cfgetc(CFILE *cfile)
{
	char tmp;

	int result = cfread(&tmp, 1, 1, cfile );
	if ( result == 1 )	{
		result = char(tmp);
	} else {
		result = CF_EOF;
	}

	return result;	
}





// cfgets() reads a string from a file
//
// returns:   success ==> returns pointer to string
//				  error   ==> NULL
//
char *cfgets(char *buf, int n, CFILE *cfile)
{
	Assert(cfile != NULL);
	Assert(buf != NULL);
	Assert(n > 0 );

	char * t = buf;
	int i, c;

	for (i=0; i<n-1; i++ )	{
		do {
			char tmp_c;

			int ret = cfread( &tmp_c, 1, 1, cfile );
			if ( ret != 1 )	{
				*buf = 0;
				if ( buf > t )	{
					return t;
				} else {
					return NULL;
				}
			}
			c = int(tmp_c);
		} while ( c == 13 );
		*buf++ = char(c);
		if ( c=='\n' ) break;
	}
	*buf++ = 0;

	return  t;
}


// 16 and 32 bit checksum stuff ----------------------------------------------------------

// CRC code for mission validation.  given to us by Kevin Bentley on 7/20/98.   Some sort of
// checksumming code that he wrote a while ago.  
#define CRC32_POLYNOMIAL					0xEDB88320
static uint CRCTable[256];

#define CF_CHKSUM_SAMPLE_SIZE				512

// update cur_chksum with the chksum of the new_data of size new_data_size
ushort cf_add_chksum_short(ushort seed, ubyte *buffer, int size)
{
	ubyte *ptr = buffer;
	uint sum1, sum2;

	sum1 = sum2 = (int)(seed);

	while(size--)	{
		sum1 += *ptr++;
		if (sum1 >= 255 ) sum1 -= 255;
		sum2 += sum1;
	}
	sum2 %= 255;
	
	return (ushort)((sum1 << 8) + sum2);
}

// update cur_chksum with the chksum of the new_data of size new_data_size
uint cf_add_chksum_long(uint seed, ubyte *buffer, size_t size)
{
	uint crc;
	ubyte *p;

	p = buffer;
	crc = seed;	

	while (size--)
		crc = (crc >> 8) ^ CRCTable[(crc ^ *p++) & 0xff];

	return crc;
}

static void cf_chksum_long_init()
{
	int i, j;
	uint crc;	

	for (i = 0; i < 256; i++) {
		crc = i;

		for (j = 8; j > 0; j--) {
			if (crc & 1)
				crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
			else
				crc >>= 1;
		}

		CRCTable[i] = crc;
	}
}

// single function convenient to use for both short and long checksums
// NOTE : only one of chk_short or chk_long must be non-NULL (indicating which checksum to perform)
static int cf_chksum_do(CFILE *cfile, ushort *chk_short, uint *chk_long, int max_size)
{
	ubyte cf_buffer[CF_CHKSUM_SAMPLE_SIZE];
	int is_long;
	int cf_len = 0;
	int cf_total;
	int read_size;

	// determine whether we're doing a short or long checksum
	is_long = 0;
	if(chk_short){
		Assert(!chk_long);		
		*chk_short = 0;
	} else {
		Assert(chk_long);
		is_long = 1;
		*chk_long = 0;
	}

	// if max_size is -1, set it to be the size of the file
	if(max_size < 0){
		cfseek(cfile, 0, SEEK_SET);
		max_size = cfilelength(cfile);
	}
	
	cf_total = 0;
	do {
		// determine how much we want to read
		if((max_size - cf_total) >= CF_CHKSUM_SAMPLE_SIZE){
			read_size = CF_CHKSUM_SAMPLE_SIZE;
		} else {
			read_size = max_size - cf_total;
		}

		// read in some buffer
		cf_len = cfread(cf_buffer, 1, read_size, cfile);

		// total we've read so far
		cf_total += cf_len;

		// add the checksum
		if(cf_len > 0){
			// do the proper short or long checksum
			if(is_long){
				*chk_long = cf_add_chksum_long(*chk_long, cf_buffer, cf_len);
			} else {
				*chk_short = cf_add_chksum_short(*chk_short, cf_buffer, cf_len);
			}
		}
	} while((cf_len > 0) && (cf_total < max_size));

	return 1;
}

// get the chksum of a pack file (VP)
int cf_chksum_pack(const char *filename, uint *chk_long, bool full)
{
	const size_t safe_size = 2097152; // 2 Meg
	const int header_offset = 32;  // skip 32bytes for header (header is currently smaller than this though)

	ubyte cf_buffer[CF_CHKSUM_SAMPLE_SIZE];
	size_t read_size;
	size_t max_size;

	if (chk_long == NULL) {
		Int3();
		return 0;
	}

	FILE *fp = fopen(filename, "rb");

	if (fp == NULL) {
		*chk_long = 0;
		return 0;
	}

	*chk_long = 0;

	// get the max size
	fseek(fp, 0, SEEK_END);
	max_size = (size_t)ftell(fp);

	// maybe do a chksum of the entire file
	if (full) {
		fseek(fp, 0, SEEK_SET);
	}
	// othewise it's only a partial check
	else {
		if (max_size > safe_size) {
			max_size = safe_size;
		}

		Assertion(max_size > header_offset,
			"max_size (" SIZE_T_ARG ") > header_offset in packfile %s", max_size, filename);
		max_size -= header_offset;

		fseek(fp, -((long)max_size), SEEK_END);
	}

	size_t cf_total = 0;
	size_t cf_len = 0;
	do {
		// determine how much we want to read
		if ( (max_size - cf_total) >= CF_CHKSUM_SAMPLE_SIZE )
			read_size = CF_CHKSUM_SAMPLE_SIZE;
		else
			read_size = max_size - cf_total;

		// read in some buffer
		cf_len = fread(cf_buffer, 1, read_size, fp);

		// total we've read so far
		cf_total += cf_len;

		// add the checksum
		if (cf_len > 0)
			*chk_long = cf_add_chksum_long((*chk_long), cf_buffer, cf_len);
	} while ( (cf_len > 0) && (cf_total < max_size) );

	fclose(fp);

	return 1;
}
// get the 2 byte checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_short(const char *filename, ushort *chksum, int max_size, int cf_type)
{
	int ret_val;
	CFILE *cfile = NULL;		
	
	// zero the checksum
	*chksum = 0;

	// attempt to open the file
	cfile = cfopen(filename,"rt",CFILE_NORMAL,cf_type);
	if(cfile == NULL){		
		return 0;
	}
	
	// call the overloaded cf_chksum function()
	ret_val = cf_chksum_do(cfile, chksum, NULL, max_size);

	// close the file down
	cfclose(cfile);
	cfile = NULL;

	// return the result
	return ret_val;
}

// get the 2 byte checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_short(CFILE *file, ushort *chksum, int max_size)
{
	int ret_code;
	int start_pos;
	
	// Returns current position of file.
	start_pos = cftell(file);
	if(start_pos == -1){
		return 0;
	}
	
	// move to the beginning of the file
	if(cfseek(file, 0, CF_SEEK_SET)){
		return 0;
	}
	ret_code = cf_chksum_do(file, chksum, NULL, max_size);
	// move back to the start position
	cfseek(file, start_pos, CF_SEEK_SET);

	return ret_code;
}

// get the 32 bit CRC checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_long(const char *filename, uint *chksum, int max_size, int cf_type)
{
	int ret_val;
	CFILE *cfile = NULL;		
	
	// zero the checksum
	*chksum = 0;

	// attempt to open the file
	cfile = cfopen(filename,"rt",CFILE_NORMAL,cf_type);
	if(cfile == NULL){		
		return 0;
	}
	
	// call the overloaded cf_chksum function()
	ret_val = cf_chksum_do(cfile, NULL, chksum, max_size);

	// close the file down
	cfclose(cfile);
	cfile = NULL;

	// return the result
	return ret_val;	
}

// get the 32 bit CRC checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_long(CFILE *file, uint *chksum, int max_size)
{
	int ret_code;
	int start_pos;
	
	// Returns current position of file.
	start_pos = cftell(file);
	if(start_pos == -1){
		return 0;
	}
	
	// move to the beginning of the file
	if(cfseek(file, 0, CF_SEEK_SET)){
		return 0;
	}
	ret_code = cf_chksum_do(file, NULL, chksum, max_size);
	// move back to the start position
	cfseek(file, start_pos, CF_SEEK_SET);

	return ret_code;
}


// Flush the open file buffer
//
// exit: 0 - success
//			1 - failure
int cflush(CFILE *cfile)
{
	Assert(cfile != NULL);
	Cfile_block *cb;
	Assert(cfile->id >= 0 && cfile->id < MAX_CFILE_BLOCKS);
	cb = &Cfile_block_list[cfile->id];	

	// not supported for memory mapped files
	Assert( !cb->data );

	Assert(cb->fp != NULL);

	int result = fflush(cb->fp);

	//WMC - update filesize
	cb->size = filelength(fileno(cb->fp));

	return result;
}
