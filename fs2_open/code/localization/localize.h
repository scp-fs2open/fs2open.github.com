/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Localization/localize.h $
 * $Revision: 2.4 $
 * $Date: 2004-08-11 05:06:27 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/03/05 09:02:05  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/09/28 21:22:59  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 2.1  2003/08/22 07:01:57  Goober5000
 * implemented $callsign to add the player callsign in a briefing, message, or whatever
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 18    11/02/99 3:24p Jefff
 * added translation functions for a few key instances where english was
 * showing up
 * 
 * 17    10/25/99 5:46p Jefff
 * Many localization fixes/changes for German builds
 * 
 * 16    10/14/99 2:52p Jefff
 * localization fixes.  added support for hi-res specific xstr offsets
 * 
 * 15    7/13/99 6:07p Jefff
 * Added support for localization string offsets.
 * 
 * 14    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 13    2/23/99 11:18a Andsager
 * Localize launcher using strings.tbl
 * 
 * 12    2/22/99 9:35p Andsager
 * Add lcl_get_language_name() returns string with current lang.  Added
 * localization for launcher.
 * 
 * 11    12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 *  
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE
#define __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE

#include "globalincs/pstypes.h"

// ------------------------------------------------------------------------------------------------------------
// LOCALIZE DEFINES/VARS
//

// language defines
#define LCL_NUM_LANGUAGES				3							// keep this up to date
#define LCL_ENGLISH						0
#define LCL_GERMAN						1
#define LCL_FRENCH						2

#define LCL_DEFAULT_LANGUAGE			LCL_ENGLISH

// following is the offset where special characters start in our font
extern int Lcl_special_chars;

// for language name strings
#define LCL_LANG_NAME_LEN				32

// use these to replace *_BUILD values
// only 1 will be active at a time
extern int Lcl_fr;
extern int Lcl_gr;
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


// NOTE : generally you should only care about the above functions. Below are very low level functions
//        which should already be well entrenched in Freespace. If you think you need to use one of the below
//        functions - ask first :)
// externalization of table/mission files (only parse routines ever need to deal with these functions) ----------------------- 

// maybe add on an appropriate subdirectory when opening a localized file
void lcl_add_dir(char *current_path);

// maybe add localized directory to full path with file name when opening a localized file
void lcl_add_dir_to_path_with_filename(char *current_path);

// open the externalization file for use during parsing (call before parsing a given file)
void lcl_ext_open();

// close the externalization file (call after parsing a given file)
void lcl_ext_close();

// Goober5000
void lcl_replace_stuff(char *text, unsigned int max_len);

// get the localized version of the string. if none exists, return the original string
// valid input to this function includes :
// "this is some text"
// XSTR("wheeee", -1)
// XSTR("whee", 20)
// and these should cover all the externalized string cases
// fills in id if non-NULL. a value of -2 indicates it is not an external string
void lcl_ext_localize(char *in, char *out, int max_len, int *id = NULL);

// translate the specified string based upon the current language
char *XSTR(char *str, int index);
int lcl_get_xstr_offset(int index, int res);

// translate umlauted chars from ascii to ansi codes
// used in launcher
#define LCL_TO_ANSI	0
#define LCL_TO_ASCII	1
char* lcl_fix_umlauts(char *str, int which_way);

// macro for launcher xstrs
#if defined(GERMAN_BUILD)
#define LXSTR(str, i)		(lcl_fix_umlauts(XSTR(str, i), LCL_TO_ANSI))
#else
#define LXSTR(str, i)		(XSTR(str, i))
#endif	// defined(GERMAN_BUILD)

void lcl_translate_wep_name(char *name);
void lcl_translate_ship_name(char *name);
void lcl_translate_brief_icon_name(char *name);
void lcl_translate_targetbox_name(char *name);

#endif	// defined __FREESPACE2_LOCALIZATION_UTILITIES_HEADER_FILE
