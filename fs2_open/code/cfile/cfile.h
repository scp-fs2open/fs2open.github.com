/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CFile/cfile.h $
 * $Revision: 2.3 $
 * $Date: 2002-10-27 23:59:28 $
 * $Author: DTP $
 *
 * <insert description of file here>
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/16 06:03:29  mharris
 * Unix port changes
 *
 * Revision 1.2  2002/05/09 22:58:08  mharris
 * Added ifdef WIN32 around change-drive functions
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 11    9/08/99 12:03a Dave
 * Make squad logos render properly in D3D all the time. Added intel anim
 * directory.
 * 
 * 10    8/31/99 9:46a Dave
 * Support for new cfile cbanims directory.
 * 
 * 9     5/19/99 4:07p Dave
 * Moved versioning code into a nice isolated common place. Fixed up
 * updating code on the pxo screen. Fixed several stub problems.
 * 
 * 8     3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 7     3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 6     1/12/99 3:15a Dave
 * Barracks screen support for selecting squad logos. We need real artwork
 * :)
 * 
 * 5     10/29/98 10:41a Dave
 * Change the way cfile initializes exe directory.
 * 
 * 4     10/13/98 9:19a Andsager
 * Add localization support to cfile.  Optional parameter with cfopen that
 * looks for localized files.
 * 
 * 3     10/12/98 9:54a Dave
 * Fixed a few file organization things.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 84    9/09/98 5:53p Dave
 * Put in new tracker packets in API. Change cfile to be able to checksum
 * portions of a file.
 * 
 * 83    8/12/98 4:53p Dave
 * Put in 32 bit checksumming for PXO missions. No validation on the
 * actual tracker yet, though.
 * 
 * 82    5/19/98 1:19p Allender
 * new low level reliable socket reading code.  Make all missions/campaign
 * load/save to data missions folder (i.e. we are rid of the player
 * missions folder)
 * 
 * 81    5/13/98 10:22p John
 * Added cfile functions to read/write rle compressed blocks of data.
 * Made palman use it for .clr files.  Made alphacolors calculate on the
 * fly rather than caching to/from disk.
 * 
 * 80    5/01/98 10:21a John
 * Added code to find all pack files in all trees.   Added code to create
 * any directories that we write to.
 * 
 * 79    4/30/98 10:29p John
 * Added code to refresh filelist if cd-rom changed or packfiles created
 * or deleted.
 * 
 * 78    4/30/98 10:06p John
 * Started adding code for splitting the maps data tree for hardware
 * textures.
 * 
 * 77    4/30/98 9:43p John
 * Restructured some stuff.
 * 
 * 76    4/30/98 8:23p John
 * Fixed some bugs with Fred caused by my new cfile code.
 * 
 * 75    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 74    4/20/98 6:04p Dave
 * Implement multidata cache flushing and xferring mission files to
 * multidata. Make sure observers can't change hud config. Fix pilot image
 * viewing in popup. Put in game status field. Tweaked multi options. 
 * 
 * 73    4/01/98 6:06p Hoffoss
 * Added a command briefing directory for those voice files.
 * 
 * 72    3/31/98 4:51p Dave
 * Removed medals screen and multiplayer buttons from demo version. Put in
 * new pilot popup screen. Make ships in mp team vs. team have proper team
 * ids. Make mp respawns a permanent option saved in the player file.
 * 
 * 71    3/26/98 6:01p Dave
 * Put in file checksumming routine in cfile. Made pilot pic xferring more
 * robust. Cut header size of voice data packets in half. Put in
 * restricted game host query system.
 * 
 * 70    3/10/98 2:27p Hoffoss
 * Added change directory with history stack functions, so you can switch
 * to a new directory, do something, and restore to what it was.  useful
 * for Fred.
 * 
 * 69    3/07/98 3:48p Lawrance
 * get save game working, allow restore from main menu
 * 
 * 68    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 67    2/20/98 4:43p Dave
 * Finished support for multiplayer player data files. Split off
 * multiplayer campaign functionality.
 * 
 * 66    2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 65    2/06/98 3:47p Allender
 * subtitling for movies
 * 
 * 64    2/05/98 10:15p Lawrance
 * Add support for .svg filenames.
 * 
 * 63    1/12/98 10:07p Hoffoss
 * Made tab cycle properly in debriefing screen and put in aux. pilot
 * image directory.
 * 
 * 62    1/11/98 2:45p John
 * Changed .lst to .clt
 * 
 * 61    1/11/98 2:14p John
 * Changed a lot of stuff that had to do with bitmap loading.   Made cfile
 * not do callbacks, I put that in global code.   Made only bitmaps that
 * need to load for a level load.
 * 
 * 60    1/02/98 4:41p Allender
 * added new mission folder for "player" missions
 * 
 * 59    12/28/97 12:42p John
 * Put in support for reading archive files; Made missionload use the
 * cf_get_file_list function.   Moved demos directory out of data tree.
 * 
 * 58    12/17/97 10:16p Allender
 * implemented a "no callback" flag to tell the cfile code not to use the
 * cf_callback on files which specify this flag
 * 
 * 57    12/08/97 6:23p Lawrance
 * add cflush()
 * 
 * 56    12/07/97 4:30p John
 * Fixed bug with cfile versioning if two files use it at once.   Added
 * code to cfile so I could display a loading box while loading.
 * 
 * 55    11/24/97 9:28a Hoffoss
 * Moved define of CF_TYPE_MULTI_PLAYERS so it's in order.  I started to
 * add it because I couldn't find it, so that's indication enough it
 * needed it.
 * 
 * 54    11/20/97 1:07a Lawrance
 * add support for voice/debriefings directory
 * 
 * 53    11/19/97 7:27p Hoffoss
 * Added version checking read functions.
 * 
 * 52    11/18/97 10:53a Hoffoss
 * 
 * 51    11/17/97 6:07p Hoffoss
 * Make get_file_list() allow filtering via a callback function.
 * 
 * 50    11/16/97 2:29p John
 * added versioning to nebulas; put nebula code into freespace.
 * 
 * 49    11/15/97 6:10p Lawrance
 * add in support for new voice directories
 * 
 * 48    11/11/97 4:54p Dave
 * Put in support for single vs. multiplayer pilots. Put in initial player
 * selection screen (no command line option yet). Started work on
 * multiplayer campaign file save gaming.
 * 
 * 47    11/07/97 4:00p Hoffoss
 * Capitalized directory names, changed the player/image directory to work
 * like the rest, and forced the player/images directory to be created.
 * 
 * 46    11/06/97 5:38p Hoffoss
 * Added a new player image directory and added support for it.
 * 
 * 45    11/04/97 7:46p Lawrance
 * Add support for data\interface\HUD directory
 * 
 * 44    10/29/97 6:22p Hoffoss
 * Added some new file listing functions.
 * 
 * 43    10/28/97 10:54a Lawrance
 * support for 8b22k and 16b11k directories under the sound directory
 * 
 * 42    10/14/97 11:34p Lawrance
 * add function to get full path for a given filename
 * 
 * 41    9/24/97 5:30p Lawrance
 * add directory for voices
 * 
 * 40    9/20/97 8:16a John
 * Made .clr files go into the Cache directory. Replaced cfopen(name,NULL)
 * to delete a file with cf_delete.
 * 
 * 39    9/09/97 6:50p Hoffoss
 * Fixed bug with mission saving.
 * 
 * 38    9/05/97 4:52p Lawrance
 * fix prototype for cfread_angles() and cfwrite_angles()
 * 
 * 37    8/29/97 4:47p Dave
 * Added an extension for state transfer status files.
 * 
 * 36    8/21/97 12:14p Dave
 * Changed demo file extension from .keg to .fsd
 * 
 * 35    8/19/97 5:51p Hoffoss
 * Fixes to cfopen to not check the default directory first, but check it
 * last.  Also tracks info as to where a file is located when opened.
 * 
 * 34    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 33    8/17/97 12:47p Hoffoss
 * Changed code so I can force missions to load from the missions
 * directory regardless of it's extension.
 * 
 * 32    8/13/97 1:39p Adam
 * make .ani files available in the data/effects directory
 * 
 * 31    8/13/97 12:24p Lawrance
 * Add support for effects directory
 * 
 * 30    7/30/97 5:23p Dave
 * Added file extensions for demo stuff
 * 
 * 29    7/28/97 10:42p Lawrance
 * added ctmpfile(), analog to tmpfile()
 * 
 * 28    7/16/97 5:29p John
 * added palette table caching and made scaler and liner no light tmapper
 * do alpha blending in 8 bpp mode.
 * 
 * 27    6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 26    6/05/97 1:22p Allender
 * added .ntl as an extension for nettest program
 * 
 * 25    4/25/97 11:31a Allender
 * Campaign state now saved in campaign save file in player directory.
 * Made some global variables follow naming convention.  Solidified
 * continuing campaigns based on new structure
 * 
 * 24    4/17/97 9:01p Allender
 * start of campaign stuff.  Campaigns now stored in external file (no
 * filenames in code).  Continuing campaign won't work at this point
 * 
 * 23    4/03/97 4:26p Lawrance
 * adding .wav search to music directory
 * 
 * 22    4/01/97 9:26a Allender
 * added support for descent style fonts although they are not used in the
 * game yet
 * 
 * 21    3/04/97 8:17a John
 * Fixed movie code to not require a file handle.  Used CFILE instead.
 * Took cfile_get_handle or whatever out.
 * 
 * 20    3/03/97 8:57a Lawrance
 * took out cf_returnfp()
 * 
 * 19    3/01/97 2:09p Lawrance
 * supporting memory mapped files, moved cfile implementation details into
 * .cpp file
 * 
 * 18    2/17/97 3:00p Lawrance
 * added .ani type to MAPS_EXT and INTERFACE_EXT
 * 
 * 17    2/07/97 9:00a Lawrance
 * added cfread_uint() and cwrite_uint()
 * 
 * 16    2/04/97 9:29a Allender
 * added cfwrite* functions
 * 
 * 15    1/22/97 10:48a Lawrance
 * supporting AVI playback
 * 
 * 14    12/23/96 10:56a John
 * Totally restructured the POF stuff to support multiple 
 * detail levels in one POF file.
 *  
 * 
 * 13    11/20/96 10:00a Hoffoss
 * Added cfile_chdir() function.
 * 
 * 12    11/13/96 10:14a Allender
 * added small routines to read basic data types.  Also changed code to
 * try to find extensions in more than 1 directory.
 * 
 * 11    11/11/96 3:21p Allender
 * added extension for movies
 *
 * $NoKeywords: $
 */

#ifndef __CFILE_H__
#define __CFILE_H__

#include <time.h>
#include "globalincs/pstypes.h"

#if defined _WIN32
  #define DIR_SEPARATOR_CHAR '\\'
  #define DIR_SEPARATOR_STR  "\\"
#elif defined unix
  #define DIR_SEPARATOR_CHAR '/'
  #define DIR_SEPARATOR_STR  "/"
#else
  #error unknown OS
#endif

#define CF_EOF (-1)

#define CF_SEEK_SET (0)
#define CF_SEEK_CUR (1)
#define CF_SEEK_END (2)

typedef struct CFILE {
	int		id;			// Index into cfile.cpp specific structure
	int		version;		// version of this file
} CFILE;

// extra info that can be returned when getting a file listing
typedef struct {
	time_t write_time;
} file_list_info;


#define CF_MAX_FILENAME_LENGTH 	32		// Includes null terminater, so real length is 31
#define CF_MAX_PATHNAME_LENGTH 	256	// Includes null terminater, so real length is 255

#define CF_TYPE_ANY						-1		// Used to check in any directory

#define CF_TYPE_INVALID					0
#define CF_TYPE_ROOT						1			// Root must be 1!!
#define CF_TYPE_DATA						2
#define CF_TYPE_MAPS						3
#define CF_TYPE_TEXT						4
#define CF_TYPE_MISSIONS				5
#define CF_TYPE_MODELS					6
#define CF_TYPE_TABLES					7
#define CF_TYPE_SOUNDS					8
#define CF_TYPE_SOUNDS_8B22K			9
#define CF_TYPE_SOUNDS_16B11K			10
#define CF_TYPE_VOICE					11
#define CF_TYPE_VOICE_BRIEFINGS		12
#define CF_TYPE_VOICE_CMD_BRIEF		13
#define CF_TYPE_VOICE_DEBRIEFINGS	14
#define CF_TYPE_VOICE_PERSONAS		15
#define CF_TYPE_VOICE_SPECIAL			16
#define CF_TYPE_VOICE_TRAINING		17
#define CF_TYPE_MUSIC					18
#define CF_TYPE_MOVIES					19
#define CF_TYPE_INTERFACE				20
#define CF_TYPE_FONT						21
#define CF_TYPE_EFFECTS					22
#define CF_TYPE_HUD						23
#define CF_TYPE_PLAYER_MAIN			24
#define CF_TYPE_PLAYER_IMAGES_MAIN	25
#define CF_TYPE_CACHE					26
#define CF_TYPE_PLAYERS					27
#define CF_TYPE_SINGLE_PLAYERS		28
#define CF_TYPE_MULTI_PLAYERS			29
#define CF_TYPE_MULTI_CACHE			30
#define CF_TYPE_CONFIG					31
#define CF_TYPE_SQUAD_IMAGES_MAIN	32
#define CF_TYPE_DEMOS					33
#define CF_TYPE_CBANIMS					34
#define CF_TYPE_INTEL_ANIMS			35

// DTP; MOD_ROOT must be 36!!
// if you want to add more directories for some reason
// the mod defines identified by CF_TYPE_MOD_XXX must be changed accordingly
// since we are duplicating the path of the existing directories 1 time and then modifying
// these to our mod`s path. to add to this, you must create an additional "duplicate" directory
// path for each new directory you want to add.

#define CF_TYPE_MOD_ROOT						36			
#define CF_TYPE_MOD_DATA						37
#define CF_TYPE_MOD_MAPS						38
#define CF_TYPE_MOD_TEXT						39
#define CF_TYPE_MOD_MISSIONS				40
#define CF_TYPE_MOD_MODELS					41
#define CF_TYPE_MOD_TABLES					42
#define CF_TYPE_MOD_SOUNDS					43
#define CF_TYPE_MOD_SOUNDS_8B22K			44
#define CF_TYPE_MOD_SOUNDS_16B11K			45
#define CF_TYPE_MOD_VOICE					46
#define CF_TYPE_MOD_VOICE_BRIEFINGS		47
#define CF_TYPE_MOD_VOICE_CMD_BRIEF		48
#define CF_TYPE_MOD_VOICE_DEBRIEFINGS	49
#define CF_TYPE_MOD_VOICE_PERSONAS		50
#define CF_TYPE_MOD_VOICE_SPECIAL			51
#define CF_TYPE_MOD_VOICE_TRAINING		52
#define CF_TYPE_MOD_MUSIC					53
#define CF_TYPE_MOD_MOVIES					54
#define CF_TYPE_MOD_INTERFACE				55
#define CF_TYPE_MOD_FONT						56
#define CF_TYPE_MOD_EFFECTS					57
#define CF_TYPE_MOD_HUD						58
#define CF_TYPE_MOD_PLAYER_MAIN			59
#define CF_TYPE_MOD_PLAYER_IMAGES_MAIN	60
#define CF_TYPE_MOD_CACHE					61
#define CF_TYPE_MOD_PLAYERS					62
#define CF_TYPE_MOD_SINGLE_PLAYERS		63
#define CF_TYPE_MOD_MULTI_PLAYERS			65
#define CF_TYPE_MOD_MULTI_CACHE			66
#define CF_TYPE_MOD_CONFIG					66
#define CF_TYPE_MOD_SQUAD_IMAGES_MAIN	67
#define CF_TYPE_MOD_DEMOS					68
#define CF_TYPE_MOD_CBANIMS					69
#define CF_TYPE_MOD_INTEL_ANIMS			70

#define CF_MAX_PATH_TYPES				71			// Can be as high as you'd like //DTP; yeah but beware alot of things uses CF_MAX_PATH_TYPES


// TRUE if type is specified and valid
#define CF_TYPE_SPECIFIED(path_type) (((path_type)>CF_TYPE_INVALID) && ((path_type)<CF_MAX_PATH_TYPES))

// #define's for the type parameter in cfopen.  
#define CFILE_NORMAL				0			// open file normally
#define CFILE_MEMORY_MAPPED	(1<<0)	//	open file as a memory-mapped file

#define CF_SORT_NONE	0
#define CF_SORT_NAME 1
#define CF_SORT_TIME 2

#define cfread_fix(file) (fix)cfread_int(file)
#define cfwrite_fix(i,file) cfwrite_int(i,file)

// callback function used for get_file_list() to filter files to be added to list.  Return 1
// to add file to list, or 0 to not add it.
extern int (*Get_file_list_filter)(char *filename);

// cfile directory. valid after cfile_init() returns successfully
#define CFILE_ROOT_DIRECTORY_LEN			256
extern char Cfile_root_dir[CFILE_ROOT_DIRECTORY_LEN];

//================= LOW-LEVEL FUNCTIONS ==================
// Call this once at the beginning of the program
int cfile_init(char *exe_dir,char *cdrom_dir=NULL);

// Call this if pack files got added or removed or the
// cdrom changed.  This will refresh the list of filenames 
// stored in packfiles and on the cdrom.
void cfile_refresh();

// add an extension to a filename if it doesn't already have it
int checkout_mod(char *checkmod);
char *cf_add_modname(char *filename, char *ext);
char *cf_add_ext(char *filename, char *ext);

// return CF_TYPE (directory location type) of a CFILE you called cfopen() successfully on.
int cf_get_dir_type(CFILE *cfile);

// Opens the file.  If no path is given, use the extension to look into the
// default path.  If mode is NULL, delete the file.  
CFILE *cfopen(char *filename, char *mode, int type = CFILE_NORMAL, int dir_type = CF_TYPE_ANY, bool localize = false);

// Flush the open file buffer
int cflush(CFILE *cfile);

// version number of opened file.  Will be 0 unless you put something else here after you
// open a file.  Once set, you can use minimum version numbers with the read functions.
void cf_set_version( CFILE * cfile, int version );

// Deletes a file.
void cf_delete( char *filename, int dir_type );

// Same as _access function to read a file's access bits
int cf_access( char *filename, int dir_type, int mode );

// Returns 1 if file exists, 0 if not.
int cf_exist( char *filename, int dir_type );

// ctmpfile() opens a temporary file stream.  File is deleted automatically when closed
CFILE *ctmpfile();

// Closes the file
int cfclose(CFILE *cfile);

// Returns size of file...
int cfilelength(CFILE *fp);

// Reads data
int cfread(void *buf, int elsize, int nelem, CFILE *fp);

// cfwrite() writes to the file
int cfwrite(void *buf, int elsize, int nelem, CFILE *cfile);

// Reads/writes RLE compressed data.
int cfread_compressed(void *buf, int elsize, int nelem, CFILE *cfile);
int cfwrite_compressed(void *param_buf, int param_elsize, int param_nelem, CFILE *cfile);

// Moves the file pointer
int cfseek(CFILE *fp, int offset, int where);

// Returns current position of file.
int cftell(CFILE *fp);

// cfputc() writes a character to a file
int cfputc(int c, CFILE *cfile);

// cfputs() writes a string to a file
int cfputs(char *str, CFILE *cfile);

// cfgetc() reads a character to a file
int cfgetc(CFILE *cfile);

// cfgets() reads a string from a file
char *cfgets(char *buf, int n, CFILE *cfile);

// cfeof() Tests for end-of-file on a stream
int cfeof(CFILE *cfile);

// Return the data pointer associated with the CFILE structure (for memory mapped files)
void *cf_returndata(CFILE *cfile);

// get the 2 byte checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_short(char *filename, ushort *chksum, int max_size = -1, int cf_type = CF_TYPE_ANY );

// get the 2 byte checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_short(CFILE *file, ushort *chksum, int max_size = -1);

// get the 32 bit CRC checksum of the passed filename - return 0 if operation failed, 1 if succeeded
int cf_chksum_long(char *filename, uint *chksum, int max_size = -1, int cf_type = CF_TYPE_ANY );

// get the 32 bit CRC checksum of the passed file - return 0 if operation failed, 1 if succeeded
// NOTE : preserves current file position
int cf_chksum_long(CFILE *file, uint *chksum, int max_size = -1);

// convenient for misc checksumming purposes ------------------------------------------

// update cur_chksum with the chksum of the new_data of size new_data_size
ushort cf_add_chksum_short(ushort seed, char *buffer, int size);

// update cur_chksum with the chksum of the new_data of size new_data_size
unsigned long cf_add_chksum_long(unsigned long seed, char *buffer, int size);

// convenient for misc checksumming purposes ------------------------------------------

//================= HIGH LEVEL FUNCTIONS ==================
int cfexist(char *filename);	// Returns true if file exists on disk (1) or in hog (2).

// rename a file, utilizing the extension to determine where file is.
#define CF_RENAME_SUCCESS				0					// successfully renamed the file
#define CF_RENAME_FAIL_ACCESS			1					// new name could not be created
#define CF_RENAME_FAIL_EXIST			2					// old name does not exist
int cf_rename(char *old_name, char *name, int type = CF_TYPE_ANY );

// changes the attributes of a file
void cf_attrib(char *name, int set, int clear, int type);

// flush (delete all files in) the passed directory (by type), return the # of files deleted
// NOTE : WILL NOT DELETE READ-ONLY FILES
int cfile_flush_dir(int type);

// functions for reading from cfile
// These are all high level, built up from
// cfread.
int cfgetc(CFILE *fp);
char *cfgets(char *buf, size_t n, CFILE *fp);
char cfread_char(CFILE *file, int ver = 0, char deflt = 0);
ubyte cfread_ubyte(CFILE *file, int ver = 0, ubyte deflt = 0);
short cfread_short(CFILE *file, int ver = 0, short deflt = 0);
ushort cfread_ushort(CFILE *file, int ver = 0, ushort deflt = 0);
int cfread_int(CFILE *file, int ver = 0, int deflt = 0);
uint cfread_uint(CFILE *file, int ver = 0, uint deflt = 0);
float cfread_float(CFILE *file, int ver = 0, float deflt = 0.0f);
void cfread_vector(vector *vec, CFILE *file, int ver = 0, vector *deflt = NULL);
void cfread_angles(angles *ang, CFILE *file, int ver = 0, angles *deflt = NULL);

// Reads variable length, null-termined string.   Will only read up
// to n characters.
void cfread_string(char *buf,int n, CFILE *file);
// Read a fixed length that is null-terminatedm, and has the length
// stored in file
void cfread_string_len(char *buf,int n, CFILE *file);

// functions for writing cfiles
int cfwrite_char(char c, CFILE *file);
int cfwrite_float(float f, CFILE *file);
int cfwrite_int(int i, CFILE *file);
int cfwrite_uint(uint i, CFILE *file);
int cfwrite_short(short s, CFILE *file);
int cfwrite_ushort(ushort s, CFILE *file);
int cfwrite_ubyte(ubyte u, CFILE *file);
int cfwrite_vector(vector *vec, CFILE *file);
int cfwrite_angles(angles *ang, CFILE *file);

// writes variable length, null-termined string.
int cfwrite_string(char *buf, CFILE *file);

// write a fixed length that is null-terminatedm, and has the length
// stored in file
int cfwrite_string_len(char *buf, CFILE *file);

int cf_get_file_list( int max, char **list, int type, char *filter, int sort = CF_SORT_NONE, file_list_info *info = NULL );
int cf_get_file_list_preallocated( int max, char arr[][MAX_FILENAME_LEN], char **list, int type, char *filter, int sort = CF_SORT_NONE, file_list_info *info = NULL );
void cf_sort_filenames( int n, char **list, int sort, file_list_info *info = NULL );

// Searches for a file.   Follows all rules and precedence and searches
// CD's and pack files.
// Input:  filespace   - Filename & extension
//         pathtype    - See CF_TYPE_ defines in CFILE.H
// Output: pack_filename - Absolute path and filename of this file.   Could be a packfile or the actual file.
//         size        - File size
//         offset      - Offset into pack file.  0 if not a packfile.
// Returns: If not found returns 0.
int cf_find_file_location( char *filespec, int pathtype, char *pack_filename, int *size, int *offset, bool localize = false);

// Functions to change directories
int cfile_chdir(char *dir);

#ifdef _WIN32
int cfile_chdrive(int DriveNum, int flag);
#endif

// push current directory on a 'stack' (so we can restore it) and change the directory
int cfile_push_chdir(int type);

// restore directory on top of the stack
int cfile_pop_dir();


#endif	/* __CFILE_H__ */
