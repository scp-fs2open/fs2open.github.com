/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/SnazzyUI.cpp $
 * $Revision: 2.6 $
 * $Date: 2004-07-26 20:47:37 $
 * $Author: Kazan $
 *
 *  Code to drive the Snazzy User Interface
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2004/07/12 16:32:53  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2004/02/14 00:18:33  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.2  2003/10/27 23:04:22  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 6     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 57    4/09/98 5:51p Lawrance
 * Be able to disable sounds for certain menus
 * 
 * 56    4/08/98 10:18a John
 * Made version and snazzy info all render as white in all screen modes.
 * 
 * 55    3/18/98 12:11p John
 * redid some string externalization
 * 
 * 54    3/05/98 11:15p Hoffoss
 * Changed non-game key checking to use game_check_key() instead of
 * game_poll().
 * 
 * 53    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 52    2/03/98 11:52p Lawrance
 * call snazzy_flush() from game_flush()
 * 
 * 51    1/20/98 2:23p Dave
 * Removed optimized build warnings. 99% done with ingame join fixes.
 * 
 * 50    1/10/98 12:47a Lawrance
 * rip out hud config text drawing
 * 
 * 49    12/23/97 5:28p Hoffoss
 * Made enter key act the same as clicking the mouse button in main hall
 * screen.
 * 
 * 48    11/19/97 8:36p Dave
 * Removed references to MainMenu.h
 * 
 * 47    11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 * 
 * 46    9/07/97 10:06p Lawrance
 * let snazzy code keep track of mouse status
 * 
 * 45    8/11/97 9:48p Lawrance
 * don't poll keyboard if not requested 
 * 
 * 44    6/11/97 1:13p John
 * Started fixing all the text colors in the game.
 * 
 * 43    6/05/97 10:19a Lawrance
 * before playing a sound, ensure it is valid
 * 
 * 42    6/05/97 1:07a Lawrance
 * changes to support sound interface
 * 
 * 41    4/23/97 5:19p Lawrance
 * split up misc sounds into: gamewide, ingame, and interface
 * 
 * 40    4/18/97 2:54p Lawrance
 * sounds now have a default volume, when playing, pass a scaling factor
 * not the actual volume
 * 
 * 39    3/31/97 5:45p Lawrance
 * supporting changes to allow multiple streamed audio files
 * 
 * 38    3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 37    3/01/97 2:13p Lawrance
 * made to work with new cfile changes
 * 
 * 36    2/25/97 11:11a Lawrance
 * adding more functionality needed for ship selection screen
 * 
 * 35    2/05/97 10:35a Lawrance
 * supporting spooled music at menus, briefings, credits etc.
 * 
 * 34    1/22/97 11:01a John
 * Added code to stream wav files in during menus.
 * 
 * 33    1/20/97 7:58p John
 * Fixed some link errors with testcode.
 * 
 * 32    1/13/97 5:36p Lawrance
 * integrating new Game_sounds structure for general game sounds
 * 
 * 31    1/07/97 6:56p Lawrance
 * adding sound hooks
 * 
 * 30    12/10/96 4:18p Lawrance
 * added snazzy_menu_close() call and integrated with existing menus
 * 
 * 29    12/09/96 2:53p Lawrance
 * fixed bug where both the snazzy code and ui code were reading the
 * keyboard, and the keypress from the snazzy code was being lost
 * 
 * 28    12/08/96 1:57a Lawrance
 * integrating hud configuration
 * 
 * 27    11/26/96 10:13a Allender
 * fixed code to properly get text from the region
 * 
 * 26    11/21/96 7:14p Lawrance
 * converted menu code to use a file (menu.tbl) to get the data for the
 * menu
 * 
 * 25    11/15/96 2:14p Lawrance
 * improved comments, removed some unnecssary #includes and code
 * 
 * 24    11/15/96 12:09p John
 * Added new UI code.  Made mouse not return Enter when you click it.
 * Changed the doSnazzyUI function and names to be snazzy_menu_xxx.   
 * 
 * 23    11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 22    11/13/96 10:30a John
 * Added code to call game_poll instead of key_inkey.
 * 
 * 21    11/13/96 8:32a Lawrance
 * streamlined menu code
 * 
 * 20    11/12/96 12:20p John
 * 
 * 19    11/12/96 12:20p John
 * added game pol
 * 
 * 18    11/11/96 4:03p Lawrance
 * 
 * 17    11/08/96 10:00a Lawrance
 * 
 * 16    11/06/96 8:54a Lawrance
 * added revision templates, made more efficient
 *
 * $NoKeywords: $
 *
*/


#include "menuui/snazzyui.h"
#include "io/key.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "gamesnd/gamesnd.h"
#include "freespace2/freespace.h"
#include "globalincs/alphacolors.h"
#include "cfile/cfile.h"
#include "localization/localize.h"


extern int ascii_table[];
extern int shifted_ascii_table[];

static int Snazzy_mouse_left_was_down;

void snazzy_flush()
{
	Snazzy_mouse_left_was_down = 0;
}

void snazzy_menu_init()
{
	game_flush();
}

// snazzy_menu_do()
//
// This function will return an identifier that matches the region of the
// image the mouse pointer is currently over.  The function works by working
// with two images
//
// 1. An image that is displayed to the player
// 2. A second image, not seen, which has masks for certain areas of image 1
//
// The idea is to read the mouse, and determine if the mouse pointer is currently
// over one of these regions.  Since these regions may be many different colors,
// the mask is checked instead, since the regions are always a uniform color
//
//	The action parameter is used to return whether the region is clicked on or simply
// has the mouse over it.  The #defines SNAZZY_OVER and SNAZZY_CLICKED are used.
//
// The purpose of the key_in parameter is to allow the caller to determine if any
// keys are pressed while within the snazzy_menu_do().  It is an optional parameter.
// 

int snazzy_menu_do(ubyte *data, int mask_w, int mask_h, int num_regions, MENU_REGION *regions, int *action, int poll_key, int *key)
{
	int i, k, x, y, offset;
	int choice = -1, mouse_on_choice = -1;
	ubyte pixel_value;

	Assert(data != NULL);
	Assert(num_regions > 0);
	Assert(regions != NULL);
	
	gr_reset_clip();  // don't remove
	mouse_get_pos( &x, &y ); 

	// This keeps mouse position detection correct for non standard modes
	gr_unsize_screen_pos(&x, &y);

	// boundary conditions
	if((y > mask_h - 1) || (x > mask_w - 1)){
		pixel_value = 0;
	} else {
		// check the pixel value under the mouse
		offset = y * mask_w + x;
		pixel_value = *(data + (offset));
	}

	*action = -1;

	k = 0;
	if ( poll_key ) {
		k = game_check_key();
		if (key)
			*key = k;  // pass keypress back to caller
	}

//	if (mouse_down_count(MOUSE_LEFT_BUTTON) )	{
	if ( !mouse_down(MOUSE_LEFT_BUTTON) && Snazzy_mouse_left_was_down ) {
		if (pixel_value >= 0)
			//nprintf(("Alan", "pixel val: %d\n", pixel_value));
			for (i=0; i < num_regions; i++) {
				if (pixel_value == regions[i].mask) {
					choice = regions[i].mask;
					if ( regions[i].click_sound != -1 ) {
						snd_play( &Snds_iface[regions[i].click_sound], 0.0f );
					}
				}
			}	// end for
	}

	switch ( k ) {
		case KEY_ESC:
			choice = ESC_PRESSED;
			break;

		default:
			if ( k )
				for (i=0; i<num_regions; i++) {
					if ( !regions[i].key )
						continue;
					if (ascii_table[k] == regions[i].key || shifted_ascii_table[k] == regions[i].key) {
						choice = regions[i].mask;
						if ( regions[i].click_sound != -1 ) {
							snd_play( &Snds_iface[regions[i].click_sound], 0.0f );
						}
					}
			}	// end for

			break;

	} // end switch

	i = -1;
	if (pixel_value >= 0) {
		for (i=0; i<num_regions; i++) {
			if (pixel_value == regions[i].mask) {
				mouse_on_choice = regions[i].mask;	
				break;
			}
		}	// end for
	}

	gr_set_color_fast(&Color_white);
	gr_set_font( FONT1 );

	if ((mouse_on_choice >= 0) && (mouse_on_choice <= (num_regions)) && (i >=0)) {
		gr_printf( 0x8000, 450, regions[i].text );
	}

	if ( mouse_down(MOUSE_LEFT_BUTTON) ){
		Snazzy_mouse_left_was_down = 1;
	} else {
		Snazzy_mouse_left_was_down = 0;
	}
	
	if ( choice > -1 || choice == ESC_PRESSED ) {
		*action = SNAZZY_CLICKED;
		return choice;
	}

	if ( mouse_on_choice > -1 ) {
		*action = SNAZZY_OVER;
		return mouse_on_choice;
	}

	return -1;
}

// add_region() will set up a menu region
//
//
//

void snazzy_menu_add_region(MENU_REGION* region, char* text, int mask, int key, int click_sound)
{
	region->mask = mask;
	region->key = key;
	strcpy(region->text, text);
	region->click_sound = click_sound;
}
 


// read_menu_tbl() will parse through menu.tbl and store the different menu region data
//
//
//

void read_menu_tbl(char* menu_name, char* bkg_filename, char* mask_filename, MENU_REGION* regions, int* num_regions, int play_sound)
{
	CFILE* fp;
	int state=0;
	char* p1, *p2, *p3;
	//char music_filename[128];

	char seps[]   = NOX(" ,\t");
	char *token;
	char tmp_line[132];

	*num_regions=0;

	// open localization
	lcl_ext_open();

	fp = cfopen( NOX("menu.tbl"), "rt" );
	if (fp == NULL) {
		Error(LOCATION, "menu.tbl could not be opened\n");

		// close localization
		lcl_ext_close();

		return;
	}


	while (cfgets(tmp_line, 132, fp)) {
		p1 = strchr(tmp_line,'\n'); if (p1) *p1 = '\0';
		p1 = strchr(tmp_line,';'); if (p1) *p1 = '\0';
		p1 = p3 = strchr( tmp_line, '[' );

		if (p3 && state == 1) {	
			// close localization
			lcl_ext_close();

			cfclose(fp);
			return;
		}
		
		if ( p1 || p3)	{
			if (!state)	{
				p2 = strchr( tmp_line, ']' );
				if (p2) *p2 = 0;
				if (!stricmp( ++p1, menu_name )) state = 1;
			} else {
				cfclose(fp);
				break;
			}
		} else if (state) {
			
		
			// parse a region line
			p1 = strchr( tmp_line, '\"' );
			if (p1) {
				p2 = strchr( tmp_line+1, '\"' );
				if (!p2) {
					nprintf(("Warning","Error parsing menu file\n"));

					// close localization
					lcl_ext_close();

					return;
				}
				*p2 = 0;
				strcpy(regions[*num_regions].text,++p1);
				p2++;

				// get the tokens mask number
				token = strtok( p2, seps );
				regions[*num_regions].mask = atoi(token);
				
				// get the hot key character
				token = strtok( NULL, seps );
				regions[*num_regions].key = token[0];

				// stuff default click sound (not in menu.tbl)
				if ( play_sound ) {
					regions[*num_regions].click_sound = SND_IFACE_MOUSE_CLICK;
				} else {
					regions[*num_regions].click_sound = -1;
				}

				*num_regions = *num_regions + 1;

			}
				else {
				// get the menu filenames

				// Establish string and get the first token
				token = strtok( tmp_line, seps );
				if ( token != NULL )
				{
					// store the background filename
					strcpy(bkg_filename, token);

					// get the mask filename
					token = strtok( NULL, seps );
					strcpy(mask_filename, token);
				}
			}
		}
	}	
	cfclose(fp);
	
	// close localization
	lcl_ext_close();
}

// snazzy_menu_close() is called when the menu using a snazzy interface is exited
//
//

void snazzy_menu_close()
{
	game_flush();
}
