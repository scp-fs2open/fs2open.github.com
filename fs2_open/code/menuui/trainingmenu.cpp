/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/TrainingMenu.cpp $
 * $Revision: 2.4 $
 * $Date: 2004-07-12 16:32:53 $
 * $Author: Kazan $
 *
 * C module that contains functions to drive the Training user interface
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/09/24 18:56:43  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 5     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 4     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 22    2/23/98 6:55p Lawrance
 * Rip out obsolete code.
 * 
 * 21    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 20    12/06/97 1:11a Lawrance
 * make a general interface for help overlays
 * 
 * 19    12/05/97 2:39p Lawrance
 * added some different sounds to main hall, add support for looping
 * ambient sounds
 * 
 * 18    11/19/97 8:36p Dave
 * Removed references to MainMenu.h
 * 
 * 17    9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 16    8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 15    6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 14    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 13    2/25/97 11:11a Lawrance
 * adding more functionality needed for ship selection screen
 * 
 * 12    12/10/96 4:18p Lawrance
 * added snazzy_menu_close() call and integrated with existing menus
 * 
 * 11    11/29/96 11:47a Lawrance
 * removed out-dated comments
 * 
 * 10    11/21/96 7:14p Lawrance
 * converted menu code to use a file (menu.tbl) to get the data for the
 * menu
 * 
 * 9     11/20/96 12:33p Lawrance
 * fixed problem when index for adding regions was not being initalized to
 * 0 in the menu init() function
 * 
 * 8     11/19/96 5:20p Lawrance
 * fixed state problem that came up as a result of menu code re-work
 * 
 * 7     11/15/96 12:09p John
 * Added new UI code.  Made mouse not return Enter when you click it.
 * Changed the doSnazzyUI function and names to be snazzy_menu_xxx.   
 * 
 * 6     11/13/96 5:08p Lawrance
 * fixed up close functions to only free stuff if it was actually
 * allocated  duh
 * 
 * 5     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 4     11/13/96 8:32a Lawrance
 * streamlined menu code
 * 
 * 3     11/08/96 4:57p Lawrance
 * integrated playing previous mission code with latest goal code
 * 
 * 2     11/05/96 1:54p Lawrance
 * 
 * 1     11/05/96 1:11p Lawrance
 * files for different game menus
 *
 * $NoKeywords: $
 *
*/

#include "gamesequence/gamesequence.h"
#include "menuui/trainingmenu.h"
#include "graphics/2d.h"
#include "menuui/snazzyui.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

// global to this file
static int trainingMenuBitmap;
static int trainingMenuMask;
static bitmap* trainingMenuMaskPtr;
static ubyte* mask_data;
static int Training_mask_w, Training_mask_h;
static MENU_REGION region[TRAINING_MENU_MAX_CHOICES];
static int num_training;

static int training_menu_inited=0;

void training_menu_init()
{
	char background_img_filename[MAX_FILENAME_LEN];
	char background_mask_filename[MAX_FILENAME_LEN];	

	snazzy_menu_init();

	read_menu_tbl(NOX("TRAINING MENU"), background_img_filename, background_mask_filename, region, &num_training);

	// load in the background bitmap (filenames are hard-coded temporarily)
	trainingMenuBitmap = bm_load(background_img_filename);
	if (trainingMenuBitmap < 0) {
		Error(LOCATION,"Could not load in %s!",background_img_filename);
	}

	trainingMenuMask = bm_load(background_mask_filename);
	Training_mask_w = -1;
	Training_mask_h = -1;

	if (trainingMenuMask < 0) {
		Error(LOCATION,"Could not load in %s!",background_mask_filename);
	}
	else {
		// get a pointer to bitmap by using bm_lock()
		trainingMenuMaskPtr = bm_lock(trainingMenuMask, 8, BMP_AABITMAP);
		mask_data = (ubyte*)trainingMenuMaskPtr->data;		
		bm_get_info(trainingMenuMask, &Training_mask_w, &Training_mask_h);
	}
}

void training_menu_close()
{
	if (training_menu_inited) {
		// done with the bitmap, so unlock it
		bm_unlock(trainingMenuMask);

		// unload the bitmaps
		bm_unload(trainingMenuBitmap);
		bm_unload(trainingMenuMask);

		training_menu_inited = 0;
		snazzy_menu_close();
	}
}

void training_menu_do_frame(float frametime)
{
	int training_menu_choice;	

	if (!training_menu_inited) {
		training_menu_init();
		training_menu_inited=1;
	}

	gr_reset_clip();
	gr_set_color(0,0,0);
	GR_MAYBE_CLEAR_RES(trainingMenuBitmap);	
	// set the background
	if(trainingMenuBitmap != -1){
		gr_set_bitmap(trainingMenuBitmap);
		gr_bitmap(0,0);
	}

	int snazzy_action = -1;
	training_menu_choice = snazzy_menu_do(mask_data, Training_mask_w, Training_mask_h, num_training, region, &snazzy_action);
	if ( snazzy_action != SNAZZY_CLICKED ){
		training_menu_choice = -1;
	}

	switch (training_menu_choice) {

		case TRAINING_MENU_TRAINING_MISSIONS_MASK:
			break;
		case TRAINING_MENU_REPLAY_MISSIONS_MASK:
			// TODO: load the mission and start the briefing
			break;
		case TRAINING_MENU_RETURN_MASK:
		case ESC_PRESSED:
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;
		case -1:
			// nothing selected
			break;
		default:
			Error(LOCATION, "Unknown option %d in training menu screen", training_menu_choice );
			break;

	} // end switch

	gr_flip();
}
