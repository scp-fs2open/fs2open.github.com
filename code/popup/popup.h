/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Popup/Popup.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:01 $
 * $Author: penguin $
 *
 * Code for displaying pop-up dialog boxes
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     8/16/99 9:50a Jefff
 * added mouseover webcursor options to user-defined popup buttons.
 * 
 * 6     8/04/99 10:53a Dave
 * Added title to the user tips popup.
 * 
 * 5     8/02/99 9:13p Dave
 * Added popup tips.
 * 
 * 4     6/01/99 4:03p Dave
 * Changed some popup flag #defines. Upped string count to 1336
 * 
 * 3     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 17    6/12/98 4:48p Hoffoss
 * Externalized default popup choices.
 * 
 * 16    2/05/98 11:09a Dave
 * Fixed an ingame join bug. Fixed a read-only file problem with
 * multiplauer file xfer.
 * 
 * 15    2/03/98 8:18p Dave
 * More MT stats transfer stuff
 * 
 * 14    1/28/98 6:24p Dave
 * Made standalone use ~8 megs less memory. Fixed multiplayer submenu
 * sequencing problem.
 * 
 * 13    1/27/98 5:52p Lawrance
 * support new "tiny" popups
 * 
 * 12    1/27/98 3:25p Hoffoss
 * Made popups use the correct button icons for default positive and
 * negative buttons.
 * 
 * 11    1/26/98 6:28p Lawrance
 * Use '&' meta char for underlining, change how keyboard usage works so
 * fits better with mouse usage.
 * 
 * 10    1/17/98 10:04p Lawrance
 * fix errors in popup comments
 * 
 * 9     1/14/98 6:55p Dave
 * Fixed a slew of multiplayer bugs. Made certain important popups ignore
 * the escape character.
 * 
 * 8     1/13/98 5:37p Dave
 * Reworked a lot of standalone interface code. Put in single and
 * multiplayer popups for death sequence. Solidified multiplayer kick
 * code.
 * 
 * 7     1/13/98 4:06p Lawrance
 * Add underline char for shortcuts.
 * 
 * 6     1/02/98 9:08p Lawrance
 * Integrated art for popups, expanded options.
 * 
 * 5     12/30/97 4:30p Sandeep
 * Added conditional popups
 * 
 * 4     12/26/97 10:01p Lawrance
 * Allow keyboard shortcuts for popup buttons
 * 
 * 3     12/24/97 9:49p Lawrance
 * ensure mouse gets drawn when popup menu is up
 * 
 * 2     12/24/97 8:54p Lawrance
 * Integrating new popup code
 * 
 * $NoKeywords: $
 */

#ifndef __POPUP_H__
#define __POPUP_H__

// standardized text used for common buttons
// Special note (JH): The leading '&' is expected for these 4 defines in the code
#define POPUP_OK						XSTR("&Ok", 503)
#define POPUP_CANCEL					XSTR("&Cancel", 504)
#define POPUP_YES						XSTR("&Yes", 505)
#define POPUP_NO						XSTR("&No", 506)

///////////////////////////////////////////////////
// flags
///////////////////////////////////////////////////

// NEVER ADD FLAGS LESS THAN 10 here. there are some internal flags which use those

// font size
#define PF_TITLE			(1<<10)	// Draw title centered in regular font (title is first line)
#define PF_TITLE_BIG		(1<<11)	// Draw title centered in large font (title is first line)
#define PF_BODY_BIG		(1<<12)	// Draw message body in large font

// color
#define PF_TITLE_RED		(1<<13)	// Color to draw title, if different from default
#define PF_TITLE_GREEN	(1<<14)
#define PF_TITLE_BLUE	(1<<15)
#define PF_TITLE_WHITE	(1<<16)
#define PF_BODY_RED		(1<<17)	// Color to draw body, if different from title
#define PF_BODY_GREEN	(1<<18)
#define PF_BODY_BLUE		(1<<19)

// icon choices
#define PF_USE_NEGATIVE_ICON		(1<<20)	// Always drawn as first icon if set
#define PF_USE_AFFIRMATIVE_ICON	(1<<21)	// Always drawn as second icon if two choices (if 1 choice, it is the only icon)

// misc
#define PF_RUN_STATE					(1<<22)	// call the do frame of the current state underneath the popup
#define PF_IGNORE_ESC				(1<<23)	// ignore the escape character
#define PF_ALLOW_DEAD_KEYS			(1<<24)	// Allow player to use keyset that exists when player dies
#define PF_NO_NETWORKING			(1<<25)	// don't do any networking

// no special buttons
#define PF_NO_SPECIAL_BUTTONS		(1<<26)

// special web mouseover cursor flags
#define PF_WEB_CURSOR_1				(1<<27)		// button 1 will get web cursor
#define PF_WEB_CURSOR_2				(1<<28)		// button 2 will get web cursor

// input:	flags			=>		formatting specificatons (PF_... shown above)
//				nchoices		=>		number of choices popup has
//				text_1		=>		text for first button
//				...			=>		
//				text_n		=>		text for last button
//				msg text		=>		text msg for popup (can be of form "%s",pl->text)
//
// exit: choice selected (0..nchoices-1)
//			will return -1 if there was an error or popup was aborted
//
// typical usage:
//
//	rval = popup(0, 2, POPUP_YES, POPUP_NO, "Hey %s, do you want to quit", pl->callsign);
int popup(int flags, int nchoices, ... );

// popup with cancel button and conditional funcrion.
// input:   condition   =>   function to call every frame, if condition() returns FALSE, the popup
//                          continues waiting.  If condition() returns anything else, the popup will
//                          return that value.
//          text_1      => text for cancel button
// 			msg text		=>	text msg for popup (can be of form "%s",pl->text)
//
// exit: condition occured (return value of condition)
//       will return 0 if cancel was pressed or popup was aborted
//       will return -1 if there was an error
//
// typical usage:
//
// int condition_function() {
// if (blah) return 1;
// return 0;
// }
// .
// .
// .
//	rval = popup_till_condition( condition_function, "Cancel", "Checking to see if %s is an idiot.", pl->callsign);
int popup_till_condition( int(*condition)() , ...);

// popup to return the value from an input box
char *popup_input(int flags, char *caption, int max_output_len = -1);

int popup_active();

int popup_running_state();

// kill any active popup, forcing it to return -1 (similar to ESC)
void popup_kill_any_active();

// change the text inside of the popup 
void popup_change_text(char *new_text);

#endif
