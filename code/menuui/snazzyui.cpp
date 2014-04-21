/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
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
	ubyte pixel_value = 0;

	Assert(data != NULL);
	Assert(num_regions > 0);
	Assert(regions != NULL);
	
	gr_reset_clip();  // don't remove
	mouse_get_pos_unscaled( &x, &y ); 

	// boundary conditions
	if((y < 0) || (y > mask_h - 1) || (x < 0) || (x > mask_w - 1)){
		pixel_value = 255;
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

	for (i=0; i<num_regions; i++) {
		if (pixel_value == regions[i].mask) {
			mouse_on_choice = regions[i].mask;	
			break;
		}
	}	// end for

	gr_set_color_fast(&Color_white);
	gr_set_font( FONT1 );

	if ((mouse_on_choice >= 0) && (mouse_on_choice <= (num_regions)) && (i >=0)) {
		gr_printf_menu( 0x8000, 450, regions[i].text );
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
	strcpy_s(region->text, text);
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
				strcpy_s(regions[*num_regions].text,++p1);
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
