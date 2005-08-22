/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Source: /cvs/cvsroot/fs2open/fs2_open/code/parse/parselo.h,v $
 * $Revision: 2.29 $
 * $Author: Goober5000 $
 * $Date: 2005-08-22 22:24:21 $
 * 
 * Header for parselo.c
 * 20-07-02 21:20 DTP
 * Bumped MISSION_TEXT_SIZE from 390000 to 1000000
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.28  2005/07/23 21:47:46  Goober5000
 * bah - fixed subsystem comparison
 * --Goober5000
 *
 * Revision 2.27  2005/07/13 03:35:31  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.26  2005/05/08 20:23:28  wmcoolmon
 * "backspace" function
 *
 * Revision 2.25  2005/04/28 01:36:46  wmcoolmon
 * More parsing flexibility
 *
 * Revision 2.24  2005/04/28 01:12:19  wmcoolmon
 * Added stuff_bool_list; Internationalized stuff_boolean.
 *
 * Revision 2.23  2005/04/05 05:53:22  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.22  2005/03/30 02:32:40  wmcoolmon
 * Made it so *Snd fields in ships.tbl and weapons.tbl take the sound name
 * as well as its index (ie "L_sidearm.wav" instead of "76")
 *
 * Revision 2.21  2005/03/01 06:55:43  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.20  2005/02/04 20:06:06  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.19  2005/01/25 22:47:37  Goober5000
 * more cool parsing stuff
 * --Goober5000
 *
 * Revision 2.18  2005/01/25 22:21:45  Goober5000
 * separated one parsing function into two
 * --Goober5000
 *
 * Revision 2.17  2004/12/25 09:25:18  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD
 *
 * Revision 2.16  2004/08/11 05:06:31  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.15  2004/05/31 08:32:25  wmcoolmon
 * Custom HUD support, better loading, etc etc.
 *
 * Revision 2.14  2004/05/29 02:52:17  wmcoolmon
 * Added stuff_float_list
 *
 * Revision 2.13  2004/05/26 03:52:07  wmcoolmon
 * Ship & weapon modular table files
 *
 * Revision 2.12  2004/05/11 02:52:11  Goober5000
 * completed the FRED import conversion stuff that I started ages ago
 * --Goober5000
 *
 * Revision 2.11  2004/03/05 09:02:08  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.10  2004/02/07 00:48:52  Goober5000
 * made FS2 able to account for subsystem mismatches between ships.tbl and the
 * model file - e.g. communication vs. communications
 * --Goober5000
 *
 * Revision 2.9  2003/10/12 03:46:23  Kazan
 * #Kazan# FS2NetD client code gone multithreaded, some Fred2 Open -mod stuff [obvious code.lib] including a change in cmdline.cpp, changed Stick's "-nohtl" to "-htl" - HTL is _OFF_ by default here (Bobboau and I decided this was a better idea for now)
 *
 * Revision 2.8  2003/09/30 04:05:09  Goober5000
 * updated FRED to import FS1 default weapons loadouts as well as missions
 * --Goober5000
 *
 * Revision 2.7  2003/09/28 21:22:58  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 2.6  2003/08/25 04:45:57  Goober5000
 * added replacement of $rank with the player's rank in any string that appears
 * in-game (same as with $callsign); also bumped the limit on the length of text
 * allowed per entry in species.tbl
 * --Goober5000
 *
 * Revision 2.5  2003/08/22 07:01:57  Goober5000
 * implemented $callsign to add the player callsign in a briefing, message, or whatever
 * --Goober5000
 *
 * Revision 2.4  2003/01/17 07:59:08  Goober5000
 * fixed some really strange behavior with strings not being truncated at the
 * # symbol
 * --Goober5000
 *
 * Revision 2.3  2003/01/05 23:41:51  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.2  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/20 19:21:13  DTP
 * bumped MAX_MISSION_TEXT to 1000000 in code/parse/parselo.h
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin		//DTP; 2003 :). should be 2002
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    9/13/99 10:40a Mikeb
 * Bumped up MISSION_TEXT_SIZE from 380000 to 390000 for ships.tbl ship
 * descriptions.
 * 
 * 13    8/02/99 4:19p Andsager
 * Bump up MISSION_TEXT_SIZE to 380000 for ships.tbl
 * 
 * 12    5/03/99 10:10a Davidg
 * DA:  Bump Mission_text_size to handle large ship.tbl
 * 
 * 11    2/19/99 11:10a Jasen
 * Upped MISSION_TEXT_SIZE to 300000 (for souper cap)
 * 
 * 10    2/03/99 6:06p Dave
 * Groundwork for FS2 PXO usertracker support.  Gametracker support next.
 * 
 * 9     1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 8     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 7     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 6     10/22/98 6:14p Dave
 * Optimized some #includes in Anim folder. Put in the beginnings of
 * parse/localization support for externalized strings and tstrings.tbl
 * 
 * 5     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 4     10/14/98 1:15p Andsager
 * Fix fred
 * 
 * 3     10/14/98 12:25p Andsager
 * Cleaned up and removed unnecessary include files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 54    6/10/98 6:47p Lawrance
 * Increase allowed mission length to 512, to accommodate longer French
 * text
 * 
 * 53    5/20/98 2:27a Sandeep
 * 
 * 52    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 51    4/03/98 10:31a John
 * Made briefing and debriefing arrays be malloc'd
 * 
 * 50    3/30/98 6:22p Hoffoss
 * Added a new parsing function.
 * 
 * 49    3/29/98 12:55a Lawrance
 * Get demo build working with limited set of data.
 * 
 * 48    2/02/98 4:59p Hoffoss
 * Added a promotion text field to rank.tbl and code to support it in
 * FreeSpace.
 * 
 * 47    11/03/97 10:11p Hoffoss
 * Added new split_str_once() function.  Works similar to split_str(),
 * only it only splits a string into 2 lines.
 * 
 * 46    10/29/97 9:02a Jasen
 * Raised MISSION_TEXT_SIZE limit to 128000.
 * 
 * 45    10/17/97 3:12p Hoffoss
 * Added a whitespace elimination function, and utilized it in briefing
 * icon label saving.
 * 
 * 44    10/15/97 4:46p Lawrance
 * add drop_leading_whitespace() function
 * 
 * 43    10/08/97 4:48p Dave
 * Moved file id function into missionparse (from parselo). Finished
 * tracker logging in and out of games. Changed how file checksumming
 * works. 
 * 
 * 42    10/07/97 5:08p Dave
 * Added file versioning/checksumming for multiplayer situations. Began
 * master tracker changes for finalized version.
 * 
 * 41    10/01/97 5:07p Hoffoss
 * Weapon loadout load and save from fsm files added.
 * 
 * 40    9/22/97 5:47p Lawrance
 * re-write split_str() to allow special control words that don't get
 * considered as part of printable string
 * 
 * 39    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 38    8/17/97 12:47p Hoffoss
 * Changed code so I can force missions to load from the missions
 * directory regardless of it's extension.
 * 
 * 37    8/13/97 10:54a Hoffoss
 * Added function to help extract only parts of a mission file.
 * 
 * 36    7/22/97 5:38p Jasen
 * Fixed bug with sexp error checking code.  Wasn't accounting for player
 * names in ship name lookup.
 * 
 * 35    6/23/97 12:02p Lawrance
 * move split_str() here, support \n's.
 * 
 * 34    6/11/97 6:14p Hoffoss
 * Made messages able to be longer.
 * 
 * 33    4/28/97 3:30p Hoffoss
 * Changed mission saving to not rely on required strings being present in
 * mission files being overwritten.
 * 
 * 32    4/22/97 4:50p Hoffoss
 * Added some flexibility to stuff_string().
 * 
 * 31    4/21/97 5:02p Hoffoss
 * Player/player status editing supported, and both saved and loaded from
 * Mission files.
 * 
 * 30    3/09/97 2:23p Allender
 * Major changes to player messaging system.  Added messages.tbl.  Made
 * all currently player messages go through new system.  Not done yet.
 * 
 * 29    3/04/97 11:19a Lawrance
 * added stuff_booleanI() that recognizes 1/0 and YES/NO
 * 
 * 28    2/18/97 9:52a Adam
 * Raised parsing text max size limit (Jason)
 * 
 * 27    1/31/97 12:07p John
 * Add turret gun type on end of $subsystem line.  Changed match_and_find
 * to return index rather than stuffing it.
 * 
 * 26    1/30/97 5:15p Mike
 * Skill levels.
 * Better named AI classes.
 * Optional override of AI class in fsm file.
 * 
 * 25    1/29/97 2:59p Mike
 * Table-driven support for John's laser rendering.
 * Null hook for AVI weapons.
 * 
 * 24    1/17/97 3:50p Hoffoss
 * Fred bug fixes and slight improvements in order to be able to create
 * mission 5.
 * 
 * 23    12/12/96 3:06p Mike
 * Fix bug in parsing code created by adding support for optional
 * terminators in required_string.
 * 
 * 22    12/12/96 2:35p Mike
 * Subsystem support
 * 
 * 21    12/11/96 3:31p Hoffoss
 * Added support for more flexible ship cargos.
 * 
 * 20    12/09/96 9:47a Hoffoss
 * Fixed silly error.  #define was in wrong file, so I moved it.
 * 
 * 19    12/03/96 2:44p Mike
 * Support for ship class and ship specific AI abilities: evasion,
 * courage, patience, accuracy
 * 
 * 18    11/15/96 1:43p Hoffoss
 * Improvements to the Ship Dialog editor window.  It is now an
 * independant window that updates data correctly.
 * 
 * 17    11/07/96 1:56p Allender
 * externed function for use in sexp.cpp
 * 
 * 16    10/30/96 9:05a Hoffoss
 * Expanded mission file max size to 32k
 * 
 * 15    10/29/96 3:28p Allender
 * added stuff_string_list function.
 * 
 * 14    10/28/96 12:18p Allender
 * added a couple of functions to deal with getting and moving past
 * strings on a whitespace only basis (instead ot to EOL)
 * 
 * 13    10/23/96 11:06a Mike
 * New weapon system.
 * Revision 1.4  1996/09/24  13:50:41  mike
 * Clean up some low level code in parselo.c.
 * Add support for Player info in mission.txt.
 * 
 * Revision 1.3  1996/09/23  20:42:03  allender
 * added filename as parameter to read_mission_text
 * 
 * Revision 1.2  1996/09/23  17:18:39  mike
 * Split parse.h into parse.h and parselo.h
 * 
 * Revision 1.1  1996/09/23  17:11:36  mike
 * Initial revision
 * 
 * 
 */

#ifndef _PARSELO_H
#define _PARSELO_H

#include <csetjmp>
#include <cstdio>

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

#define	MISSION_TEXT_SIZE	1000000

extern char	Mission_text[MISSION_TEXT_SIZE];
extern char	Mission_text_raw[MISSION_TEXT_SIZE];
extern char	*Mp;
extern char	*token_found;
extern int fred_parse_flag;
extern int Token_found_flag;
extern jmp_buf parse_abort;


#define	COMMENT_CHAR	(char)';'
#define	EOF_CHAR			(char)-128
#define	EOLN				(char)0x0a

#define	F_NAME					1
#define	F_DATE					2
#define	F_NOTES					3
#define	F_FILESPEC				4
#define	F_MULTITEXTOLD			5	// needed for backwards compatability with old briefing format
#define	F_SEXP					6
#define	F_PATHNAME				7
#define	F_SHIPCHOICE			8
#define	F_MESSAGE				9	// this is now obsolete for mission messages - all messages in missions should now use $MessageNew and stuff strings as F_MULTITEXT
#define	F_MULTITEXT				10
#define F_RAW					11	// for any internal parsing use. Just strips whitespace and copies the text.

#define PARSE_BUF_SIZE			4096

//For modular TBL files -C
#define MAX_TBL_PARTS 32
extern bool modular_tables_loaded;

// 1K on the stack? seems to work...
// JH: 1k isn't enough!  Command briefs can be 16k max, so changed this.
#define MAX_TMP_STRING_LENGTH 16384


#define	SHIP_TYPE			0	// used to identify which kind of array to do a search for a name in
#define	SHIP_INFO_TYPE		1
#define	WEAPON_LIST_TYPE	2	//	to parse an int_list of weapons
#define	RAW_INTEGER_TYPE	3	//	to parse a list of integers
#define	WEAPON_POOL_TYPE	4

#define SEXP_SAVE_MODE				1
#define SEXP_ERROR_CHECK_MODE		2

// Goober5000 - this seems to be a pretty universal function
void end_string_at_first_hash_symbol(char *src);
char *get_pointer_to_first_hash_symbol(char *src);

// Goober5000
int subsystem_stricmp(const char *str1, const char *str2);

// white space
extern int is_white_space(char ch);
extern void ignore_white_space();
extern void drop_trailing_white_space(char *str);
extern void drop_leading_white_space(char *str);
extern char *drop_white_space(char *str);

// gray space
void ignore_gray_space();

// error
extern int get_line_num();
extern char *next_tokens();
extern void diag_printf(char *format, ...);
extern void error_display(int error_level, char *format, ...);

// skip
extern int skip_to_string(char *pstr, char *end = NULL);
extern int skip_to_start_of_strings(char *pstr1, char *pstr2, char *end = NULL);
extern void advance_to_eoln(char *terminators);
extern void skip_token();

// required
extern int required_string(char *pstr);
extern int optional_string(char *pstr);
extern int required_string_either(char *str1, char *str2);
extern int required_string_3(char *str1, char *str2, char *str3);

// stuff
extern void copy_to_eoln(char *outstr, char *more_terminators, char *instr, int max);
extern void copy_text_until(char *outstr, char *instr, char *endstr, int max_chars);
extern void stuff_string_white(char *pstr, int len = 0);
extern void stuff_string(char *pstr, int type, char *terminators, int len = 0);
extern void stuff_string_line(char *pstr, int len);

// Exactly the same as stuff string only Malloc's the buffer. 
//	Supports various FreeSpace primitive types.  If 'len' is supplied, it will override
// the default string length if using the F_NAME case.
char *stuff_and_malloc_string( int type, char *terminators, int len);
extern void stuff_float(float *f);
extern void stuff_int(int *i);
extern void stuff_sound(int *dest);
extern void stuff_ubyte(ubyte *i);
extern int stuff_string_list(char slp[][NAME_LENGTH], int max_strings);
extern int stuff_int_list(int *ilp, int max_ints, int lookup_type);
extern int stuff_float_list(float* flp, int max_floats);
extern int stuff_vector_list(vec3d *vlp, int max_vecs);
extern int stuff_bool_list(bool *blp, int max_bools);
extern void stuff_vector(vec3d *vp);
extern void stuff_matrix(matrix *mp);
extern int string_lookup(char *str1, char *strlist[], int max, char *description = NULL, int say_errors = 0);
extern void find_and_stuff(char *id, int *addr, int f_type, char *strlist[], int max, char *description);
extern int match_and_stuff(int f_type, char *strlist[], int max, char *description);
extern void find_and_stuff_or_add(char *id, int *addr, int f_type, char *strlist[], int *total,
	int max, char *description);
extern int get_string(char *str);
extern void stuff_parenthesized_vector(vec3d *vp);
void stuff_boolean(int *i, bool a_to_eol=true);
void stuff_boolean(bool *b, bool a_to_eol=true);
int check_for_string(char *pstr);
int check_for_string_raw(char *pstr);

// general
extern void reset_parse();
extern void display_parse_diagnostics();

// utility
extern void mark_int_list(int *ilp, int max_ints, int lookup_type);
extern void compact_multitext_string(char *str);
extern void read_file_text(char *filename, int mode = CF_TYPE_ANY, char *processed_text = Mission_text, char *raw_text = Mission_text_raw);
extern void read_file_text_from_array(char *array, char *processed_text = Mission_text, char *raw_text = Mission_text_raw);
extern void read_raw_file_text(char *filename, int mode = CF_TYPE_ANY, char *raw_text = Mission_text_raw);
extern void process_raw_file_text(char *processed_text = Mission_text, char *raw_text = Mission_text_raw);
extern void debug_show_mission_text();
extern void convert_sexp_to_string(int cur_node, char *outstr, int mode);
char *split_str_once(char *src, int max_pixel_w);
int split_str(char *src, int max_pixel_w, int *n_chars, char **p_str, int max_lines, char ignore_char = -1);

// fred
extern int required_string_fred(char *pstr, char *end = NULL);
extern int required_string_either_fred(char *str1, char *str2);
extern int optional_string_fred(char *pstr, char *end = NULL, char *end2 = NULL);

extern char	parse_error_text[64];

// Goober5000 - returns position of replacement or -1 for exceeded length
extern int replace_one(char *str, char *oldstr, char *newstr, unsigned int max_len, int range = 0);

// Goober5000 - returns number of replacements or -1 for exceeded length
extern int replace_all(char *str, char *oldstr, char *newstr, unsigned int max_len, int range = 0);

// Goober5000 (why is this not in the C library?)
extern char *stristr(const char *str, const char *substr);

//WMC - backspaces the first character of given char pointer
void backspace(char *src);

inline void parse_advance(int s){Mp+=s;}
#endif
