/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE
#define __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE

#include "globalincs/pstypes.h"
#include "graphics/font.h"

// ------------------------------------------------------------------------------------------------------------
// LOCALIZE DEFINES/VARS
//

// language defines
#define LCL_ENGLISH						0
#define LCL_GERMAN						1
#define LCL_FRENCH						2
#define LCL_POLISH						3

#define FS2_OPEN_DEFAULT_LANGUAGE		0

// for language name strings
#define LCL_LANG_NAME_LEN				32

// language info table
typedef struct lang_info {
	char lang_name[LCL_LANG_NAME_LEN + 1];				// literal name of the language
	char lang_ext[LCL_LANG_NAME_LEN + 1];				// the extension used for adding to names on disk access
	ubyte special_char_indexes[MAX_FONTS];				// where in the font do we have the special characters for this language
														// note: treats 0 as "none" since a zero offset in a font makes no sense
														// i.e. all the normal chars start at zero
	int checksum;										// used for language auto-detection
} lang_info;

extern SCP_vector<lang_info> Lcl_languages; 

// following is the offset where special characters start in our font
extern int Lcl_special_chars;

// use these to replace *_BUILD values
// only 1 will be active at a time
extern int Lcl_fr;
extern int Lcl_gr;
extern int Lcl_pl;
extern int Lcl_english;


// ------------------------------------------------------------------------------------------------------------
// LOCALIZE FUNCTIONS
//

// initialize localization, if no language is passed - use the language specified in the registry
void lcl_init(int lang = -1);

// shutdown localization
void lcl_close();

// initialize the xstr table
void lcl_xstr_init();

// free the xstr table
void lcl_xstr_close();

// determine what language we're running in, see LCL_* defines above
int lcl_get_language();

// returns the current language character string
void lcl_get_language_name(char *lang_name);

// set our current language
void lcl_set_language(int lang);

// get a fonts special characters index
ubyte lcl_get_font_index(int font_num);

// NOTE : generally you should only care about the above functions. Below are very low level functions
//        which should already be well entrenched in FreeSpace. If you think you need to use one of the below
//        functions - ask first :)
// externalization of table/mission files (only parse routines ever need to deal with these functions) ----------------------- 

// maybe add on an appropriate subdirectory when opening a localized file
void lcl_add_dir(char *current_path);

// maybe add localized directory to full path with file name when opening a localized file
int lcl_add_dir_to_path_with_filename(char *current_path, size_t path_max);

// open the externalization file for use during parsing (call before parsing a given file)
void lcl_ext_open();

// close the externalization file (call after parsing a given file)
void lcl_ext_close();

// Goober5000
void lcl_replace_stuff(char *text, size_t max_len);
void lcl_replace_stuff(SCP_string &text);

// Karajorma
void lcl_fred_replace_stuff(char *text, size_t max_len);
void lcl_fred_replace_stuff(SCP_string &text);

// get the localized version of the string. if none exists, return the original string
// valid input to this function includes :
// "this is some text"
// XSTR("wheeee", -1)
// XSTR("whee", 20)
// and these should cover all the externalized string cases
// fills in id if non-NULL. a value of -2 indicates it is not an external string
void lcl_ext_localize(const char *in, char *out, size_t max_len, int *id = NULL);
void lcl_ext_localize(const SCP_string &in, SCP_string &out, int *id = NULL);

// translate the specified string based upon the current language
const char *XSTR(const char *str, int index);
int lcl_get_xstr_offset(int index, int res);

// translate umlauted chars from ascii to ansi codes
// used in launcher
#define LCL_TO_ANSI	0
#define LCL_TO_ASCII	1
char* lcl_fix_umlauts(char *str, int which_way);

// covert some polish characters
void lcl_fix_polish(char *str);
void lcl_fix_polish(SCP_string &str);

// macro for launcher xstrs
#if defined(GERMAN_BUILD)
#define LXSTR(str, i)		(lcl_fix_umlauts(XSTR(str, i), LCL_TO_ANSI))
#else
#define LXSTR(str, i)		(XSTR(str, i))
#endif	// defined(GERMAN_BUILD)

void lcl_translate_wep_name_gr(char *name);
void lcl_translate_ship_name_gr(char *name);
void lcl_translate_brief_icon_name_gr(char *name);
void lcl_translate_brief_icon_name_pl(char *name);
void lcl_translate_targetbox_name_gr(char *name);
void lcl_translate_targetbox_name_pl(char *name);
void lcl_translate_medal_name_gr(char *name);

#endif	// defined __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE
