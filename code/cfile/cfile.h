/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __CFILE_H__
#define __CFILE_H__


#include "globalincs/pstypes.h"

#include <ctime>
#include <stdexcept>
#include <memory>
#include <utility>


#define CF_EOF (-1)

#define CF_SEEK_SET (0)
#define CF_SEEK_CUR (1)
#define CF_SEEK_END (2)

// Opaque file handle
struct CFILE;

// extra info that can be returned when getting a file listing
typedef struct {
	time_t write_time;
} file_list_info;


#define CF_MAX_FILENAME_LENGTH 	32		// Includes null terminater, so real length is 31
#define CF_MAX_PATHNAME_LENGTH 	256	// Includes null terminater, so real length is 255

#define CF_TYPE_ANY						-1		// Used to check in any directory

#define CF_TYPE_INVALID				0
#define CF_TYPE_ROOT				1			// Root must be 1!!
#define CF_TYPE_DATA				2
#define CF_TYPE_MAPS				3
#define CF_TYPE_TEXT				4
#define CF_TYPE_MODELS				5
#define CF_TYPE_TABLES				6
#define CF_TYPE_SOUNDS				7
#define CF_TYPE_SOUNDS_8B22K		8
#define CF_TYPE_SOUNDS_16B11K		9
#define CF_TYPE_VOICE				10
#define CF_TYPE_VOICE_BRIEFINGS		11
#define CF_TYPE_VOICE_CMD_BRIEF		12
#define CF_TYPE_VOICE_DEBRIEFINGS	13
#define CF_TYPE_VOICE_PERSONAS		14
#define CF_TYPE_VOICE_SPECIAL		15
#define CF_TYPE_VOICE_TRAINING		16
#define CF_TYPE_MUSIC				17
#define CF_TYPE_MOVIES				18
#define CF_TYPE_INTERFACE			19
#define CF_TYPE_FONT				20
#define CF_TYPE_EFFECTS				21
#define CF_TYPE_HUD					22
#define CF_TYPE_PLAYERS				23
#define CF_TYPE_PLAYER_IMAGES		24
#define CF_TYPE_SQUAD_IMAGES		25
#define CF_TYPE_SINGLE_PLAYERS		26
#define CF_TYPE_MULTI_PLAYERS		27
#define CF_TYPE_CACHE				28
#define CF_TYPE_MULTI_CACHE			29
#define CF_TYPE_MISSIONS			30
#define CF_TYPE_CONFIG				31
#define CF_TYPE_DEMOS				32
#define CF_TYPE_CBANIMS				33
#define CF_TYPE_INTEL_ANIMS			34
#define CF_TYPE_SCRIPTS				35
#define CF_TYPE_FICTION				36
#define CF_TYPE_FREDDOCS			37
#define CF_TYPE_INTERFACE_MARKUP 38
#define CF_TYPE_INTERFACE_CSS 39

#define CF_MAX_PATH_TYPES                                                                                              \
	40 // Can be as high as you'd like //DTP; yeah but beware alot of things uses CF_MAX_PATH_TYPES

// TRUE if type is specified and valid
#define CF_TYPE_SPECIFIED(path_type) (((path_type)>CF_TYPE_INVALID) && ((path_type)<CF_MAX_PATH_TYPES))

// #define's for the type parameter in cfopen.  
#define CFILE_NORMAL				0			// open file normally
#define CFILE_MEMORY_MAPPED	(1<<0)	//	open file as a memory-mapped file

#define CF_SORT_NONE	0
#define CF_SORT_NAME 1
#define CF_SORT_TIME 2
#define CF_SORT_REVERSE 3

/**
 * @brief Contains a collection of flags for specifying from where a CFILE should be opened
 *
 * There are two types of flags. CF_LOCATION_GAME_ROOT and CF_LOCATION_USER_ROOT specify in which basic root directory
 * CFile should search for files.
 *
 * The remaining flags control what type of location may be used for the operation. It's possible to restrict the
 * location to the primary mod, the remaining mods or the top-level root directory.
 */
enum CFileLocationFlags {
	CF_LOCATION_ROOT_GAME = 1 << 0, //!< The game root location. This is the location of the game data (e.g. FS2 retail)
	/**
	 * @brief The user directories.
	 *
	 * If the engine is in portable mode then using this flag exclusively will result in a failure to open files!
	 */
	CF_LOCATION_ROOT_USER = 1 << 1,

	/**
	 * @brief The memory root
	 *
	 * This contains all the built-in files available to the engine.
	 */
	CF_LOCATION_ROOT_MEMORY = 1 << 2,

	/**
	 * @brief Mask for extracting root type from a location bit field
	 */
	CF_LOCATION_ROOT_MASK = 0xFFFF,

	CF_LOCATION_TYPE_ROOT = 1 << 16, //!< The basic, top-level root location
	/**
	 * @brief The primary mod location.
	 *
	 * This is the mod that appears first on the command line. If there are no mods then the root location is considered
	 * to be the primary mod
	 */
	CF_LOCATION_TYPE_PRIMARY_MOD = 1 << 17,
	/**
	 * @brief The remaining mods.
	 *
	 * If there are no mods on the command line then this might not include any valid location.
	 */
	CF_LOCATION_TYPE_SECONDARY_MODS = 1 << 18,

	/**
	 * @brief Bitmask for extracting the type from a location bit field
	 */
	CF_LOCATION_TYPE_MASK = 0xFFFF0000,

	/**
	 * @brief A combination of all flags
	 *
	 * This should be used when the default behavior without any location filtering is desired.
	 */
	CF_LOCATION_ALL = CF_LOCATION_ROOT_MASK | CF_LOCATION_TYPE_MASK
};

// callback function used for get_file_list() to filter files to be added to list.  Return 1
// to add file to list, or 0 to not add it.
extern int (*Get_file_list_filter)(const char *filename);

// extra check for child directory under CF_TYPE_*
// NOTE: if specified cf_get_file_list() will not search pack files!
// NOTE: specified string must not contain ':' or spaces or begin with DIR_SEPARATOR!
extern const char *Get_file_list_child;

// cfile directory. valid after cfile_init() returns successfully
#define CFILE_ROOT_DIRECTORY_LEN			256
extern char Cfile_root_dir[CFILE_ROOT_DIRECTORY_LEN];
extern char Cfile_user_dir[CFILE_ROOT_DIRECTORY_LEN];

//================= LOW-LEVEL FUNCTIONS ==================
int cfile_init(const char *exe_dir, const char *cdrom_dir=NULL);
void cfile_close();

// add an extension to a filename if it doesn't already have it
char *cf_add_ext(const char *filename, const char *ext);

// return filename of a CFILE you called cfopen() successfully on.
const char *cf_get_filename(const CFILE *cfile);

// return CF_TYPE (directory location type) of a CFILE you called cfopen() successfully on.
int cf_get_dir_type(const CFILE *cfile);

// Opens the file.  If no path is given, use the extension to look into the
// default path.  If mode is NULL, delete the file.
CFILE* _cfopen(const char* source_file, int line, const char* filename, const char* mode, int type = CFILE_NORMAL,
               int dir_type = CF_TYPE_ANY, bool localize = false, uint32_t location_flags = CF_LOCATION_ALL);
#define cfopen(...) _cfopen(LOCATION, __VA_ARGS__) // Pass source location to the function

struct CFileLocation;

// like cfopen(), but it accepts a fully qualified path only (ie, the result of a cf_find_file_location() call)
// NOTE: only supports reading files!!
CFILE *_cfopen_special(const char* source_file, int line, const CFileLocation &res, const char* mode,
	                   int dir_type = CF_TYPE_ANY);
#define cfopen_special(...) _cfopen_special(LOCATION, __VA_ARGS__) // Pass source location to the function

// Flush the open file buffer
int cflush(CFILE *cfile);

// will throw an error if cfread*() functions read past this mark
// converted to raw offsets when used, but gets passed actual length from current position
// setting 'len' to zero will disable the check
void cf_set_max_read_len(CFILE *cfile, size_t len);

// Deletes a file. Returns 0 on error, 1 if successful
int cf_delete(const char *filename, int dir_type, uint32_t location_flags = CF_LOCATION_ALL);

// Same as _access function to read a file's access bits
int cf_access(const char *filename, int dir_type, int mode);

// Returns 1 if the file exists, 0 if not.
// Checks only the file system.
int cf_exists(const char *filename, int dir_type);

// Goober5000
// Returns 1 if the file exists, 0 if not.
// Checks both the file system and the VPs.
int cf_exists_full(const char *filename, int dir_type);
// check num_ext worth of ext_list extensions
int cf_exists_full_ext(const char *filename, int dir_type, const int num_ext, const char **ext_list);

// ctmpfile() opens a temporary file stream.  File is deleted automatically when closed
CFILE *ctmpfile();

// Closes the file
int cfclose(CFILE *cfile);

//Checks if the given handle is valid
int cf_is_valid(CFILE *cfile);

// Returns size of file...
int cfilelength(CFILE *fp);

// Reads data
int cfread(void *buf, int elsize, int nelem, CFILE *fp);

// cfwrite() writes to the file
int cfwrite(const void *buf, int elsize, int nelem, CFILE *cfile);

// Moves the file pointer
int cfseek(CFILE *fp, int offset, int where);

// Returns current position of file.
int cftell(CFILE *fp);

// cfputc() writes a character to a file
int cfputc(int c, CFILE *cfile);

// cfputs() writes a string to a file
int cfputs(const char *str, CFILE *cfile);

// cfgetc() reads a character to a file
int cfgetc(CFILE *cfile);

// cfgets() reads a string from a file
char *cfgets(char *buf, int n, CFILE *cfile);

// cfeof() Tests for end-of-file on a stream
int cfeof(CFILE *cfile);

// Return the data pointer associated with the CFILE structure (for memory mapped files)
const void *cf_returndata(CFILE *cfile);

// get the 2 byte checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_short(const char *filename, ushort *chksum, int max_size = -1, int cf_type = CF_TYPE_ANY );

// get the 2 byte checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_short(CFILE *file, ushort *chksum, int max_size = -1);

// get the 32 bit CRC checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_long(const char *filename, uint *chksum, int max_size = -1, int cf_type = CF_TYPE_ANY );

// get the 32 bit CRC checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_long(CFILE *file, uint *chksum, int max_size = -1);

int cf_chksum_pack(const char *filename, uint *chk_long, bool full = false);

// convenient for misc checksumming purposes ------------------------------------------

// update cur_chksum with the chksum of the new_data of size new_data_size
ushort cf_add_chksum_short(ushort seed, ubyte *buffer, int size);

// update cur_chksum with the chksum of the new_data of size new_data_size
uint cf_add_chksum_long(uint seed, ubyte *buffer, size_t size);

// convenient for misc checksumming purposes ------------------------------------------

//================= HIGH LEVEL FUNCTIONS ==================

// rename a file, utilizing the extension to determine where file is.
#define CF_RENAME_SUCCESS				0					// successfully renamed the file
#define CF_RENAME_FAIL_ACCESS			1					// new name could not be created
#define CF_RENAME_FAIL_EXIST			2					// old name does not exist
int cf_rename(const char *old_name, const char *name, int type = CF_TYPE_ANY );

// Creates the directory path if it doesn't exist. Even creates all its
// parent paths.
void cf_create_directory(int dir_type, uint32_t location_flags = CF_LOCATION_ALL);

// changes the attributes of a file
void cf_attrib(const char *name, int set, int clear, int type);

// flush (delete all files in) the passed directory (by type), return the # of files deleted
// NOTE : WILL NOT DELETE READ-ONLY FILES
int cfile_flush_dir(int type);

// functions for reading from cfile
// These are all high level, built up from
// cfread.
char *cfgets(char *buf, size_t n, CFILE *fp);
char cfread_char(CFILE* file, char deflt = 0);
ubyte cfread_ubyte(CFILE* file, ubyte deflt = 0);
short cfread_short(CFILE* file, short deflt = 0);
ushort cfread_ushort(CFILE* file, ushort deflt = 0);
int cfread_int(CFILE* file, int deflt = 0);
uint cfread_uint(CFILE* file, uint deflt = 0);
float cfread_float(CFILE* file, float deflt = 0.0f);
void cfread_vector(vec3d* vec, CFILE* file, vec3d* deflt = nullptr);

// Reads variable length, null-termined string.   Will only read up
// to n characters.
void cfread_string(char *buf,int n, CFILE *file);
/**
 * @brief Read a fixed length string that is not null-terminated, with the length stored in file
 *
 * @param buf Pre-allocated array to store string
 * @param n Size of pre-allocated array
 * @param file File to read from
 *
 * @note Appends NULL character to string (buf)
 */
void cfread_string_len(char *buf,int n, CFILE *file);

/**
 * @brief Read a string from the file where the length is stored in the file
 * @param file The file to read the string from
 * @return The string that was read
 */
SCP_string cfread_string_len(CFILE *file);

// functions for writing cfiles
int cfwrite_char(char c, CFILE *file);
int cfwrite_float(float f, CFILE *file);
int cfwrite_int(int i, CFILE *file);
int cfwrite_uint(uint i, CFILE *file);
int cfwrite_short(short s, CFILE *file);
int cfwrite_ushort(ushort s, CFILE *file);
int cfwrite_ubyte(ubyte u, CFILE *file);

// writes variable length, null-termined string.
int cfwrite_string(const char *buf, CFILE *file);

/**
 * @brief Write a fixed length string (not including its null terminator), with the length stored in file
 *
 * @param buf String to write to file
 * @param file File to write to
 */
int cfwrite_string_len(const char *buf, CFILE *file);

int cf_get_file_list(SCP_vector<SCP_string>& list, int pathtype, const char* filter, int sort = CF_SORT_NONE,
                     SCP_vector<file_list_info>* info = nullptr, uint32_t location_flags = CF_LOCATION_ALL);
int cf_get_file_list(int max, char** list, int type, const char* filter, int sort = CF_SORT_NONE,
                     file_list_info* info = nullptr, uint32_t location_flags = CF_LOCATION_ALL);
int cf_get_file_list_preallocated(int max, char arr[][MAX_FILENAME_LEN], char** list, int type, const char* filter,
                                  int sort = CF_SORT_NONE, file_list_info* info = nullptr,
                                  uint32_t location_flags = CF_LOCATION_ALL);
void cf_sort_filenames( int n, char **list, int sort, file_list_info *info = NULL );
void cf_sort_filenames( SCP_vector<SCP_string> &list, int sort, SCP_vector<file_list_info> *info = NULL );

struct cf_file;

struct CFileLocation {
	bool found = false;
	SCP_string name_ext;
	SCP_string full_name;
	size_t size          = 0;
	size_t offset        = 0;
	const void* data_ptr = nullptr;

	explicit CFileLocation(bool found_in = false) : found(found_in) {}
	explicit CFileLocation(const cf_file& file);
};

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.
// Input:  filespace   - Filename & extension
//         pathtype    - See CF_TYPE_ defines in CFILE.H
//         max_out     - Maximum string size to be stuffed into pack_filename
// Output: pack_filename - Absolute path and filename of this file.   Could be a packfile or the actual file.
//         size        - File size
//         offset      - Offset into pack file.  0 if not a packfile.
// Returns: If not found returns 0.
CFileLocation cf_find_file_location(const char* filespec, int pathtype, bool localize = false,
                                    uint32_t location_flags = CF_LOCATION_ALL);

struct CFileLocationExt : public CFileLocation {
	int extension_index = -1;

	explicit CFileLocationExt(int extension_index_in = -1)
	    : CFileLocation(extension_index_in >= 0), extension_index(extension_index_in)
	{
	}
};

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.  Searches all locations in order for first filename using ext filter list.
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
CFileLocationExt cf_find_file_location_ext(const char* filename, const int ext_num, const char** ext_list, int pathtype,
                                           bool localize = false);

// Functions to change directories
int cfile_chdir(const char *dir);

#ifdef _WIN32
int cfile_chdrive(int DriveNum, int flag);
#endif

// push current directory on a 'stack' (so we can restore it) and change the directory
int cfile_push_chdir(int type);

// restore directory on top of the stack
int cfile_pop_dir();

int cfile_get_path_type(const SCP_string& dir);

namespace cfile
{
	// exceptions and other errors
	class cfile_error : public std::exception
	{
		public:
			cfile_error() : m_excuse("CFILE Exception")
			{
			}

			explicit cfile_error(std::string excuse) : m_excuse(std::move(excuse))
			{
			}

			~cfile_error() noexcept override = default;

			const char *what() const noexcept override {
				return m_excuse.c_str();
			}

		private:
			std::string m_excuse;
	};

	class max_read_length : public cfile_error
	{
		public:
		explicit max_read_length(const std::string &excuse) : cfile_error(excuse)
			{
			}

			max_read_length() : cfile_error("Attempted to read beyond length limit")
			{
			}
	};

}

// This allows to use std::unique_ptr with CFILE
namespace std {
template <>
struct default_delete<CFILE> {
	void operator()(CFILE* ptr)
	{
		if (cf_is_valid(ptr)) {
			cfclose(ptr);
		}
	}
};
} // namespace std

#endif	/* __CFILE_H__ */
