/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "gamesequence/gamesequence.h"
#include "menuui/trainingmenu.h"
#include "graphics/2d.h"
#include "menuui/snazzyui.h"



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
		bm_release(trainingMenuBitmap);
		bm_release(trainingMenuMask);

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
		gr_bitmap(0,0,GR_RESIZE_MENU);
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
