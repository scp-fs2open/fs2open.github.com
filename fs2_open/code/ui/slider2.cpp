/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ui/uidefs.h"
#include "ui/ui.h"
#include "freespace2/freespace.h"
#include "bmpman/bmpman.h"
#include "io/timer.h"



// captureCallback is called when an item is "selected" by mouse release. That is, the user has clicked, dragged and _released_. 
// the callback is called when the scrollbar has been released
void UI_SLIDER2::create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, int _numberItems, char *_bitmapSliderControl, void (* _upCallback)(), void (*_downCallback)(),
				void (* _captureCallback)()) {

	int buttonHeight, buttonWidth;

	base_create( wnd, UI_KIND_SLIDER2, _x, _y, _w, _h );

	Assert(_upCallback != NULL);
	Assert(_downCallback != NULL);

	upCallback = _upCallback;
	downCallback = _downCallback;

	captureCallback = _captureCallback;	

	Assert(_bitmapSliderControl > 0);

	last_scrolled = 0;

	// set bitmap
	set_bmaps(_bitmapSliderControl, 3, 0);
	
	// determine possible positions
	bm_get_info(bmap_ids[S2_NORMAL],&buttonWidth, &buttonHeight, NULL, NULL, NULL);
	slider_w = buttonWidth;
	slider_h = buttonHeight;
	Assert(buttonHeight > 5);
	slider_half_h = (int)(buttonHeight / 2);
	numberPositions = _h - buttonHeight;
	
	Assert(numberPositions >= 0);
	currentItem = 0;
	currentPosition = 0;

	numberItems = _numberItems;
	if (numberItems <= 0) {
		disabled_flag = 1;
	}

	slider_mode = S2M_DEFAULT;
}

void UI_SLIDER2::draw() {
	Assert((currentPosition >= 0) && (currentPosition <= numberPositions));
	if (uses_bmaps && !disabled_flag) {
		gr_reset_clip();
		switch (slider_mode) {
		case S2M_ON_ME:
			gr_set_bitmap(bmap_ids[S2_HIGHLIGHT]);  // draw slider level
			break;
		case S2M_MOVING:
			gr_set_bitmap(bmap_ids[S2_PRESSED]);
			break;
		case S2M_DEFAULT:
		default:
			gr_set_bitmap(bmap_ids[S2_NORMAL]);  // draw slider level
			break;
		}
		gr_bitmap(x, y+currentPosition);
	}
}

void UI_SLIDER2::process(int focus)
{
	int OnMe, mouse_lock_move;	

	if (disabled_flag) {
		return;
	}

	OnMe = is_mouse_on();
	if ( OnMe ) {
		// are we on the button?
		if ( (ui_mouse.y >= (y+currentPosition)) && (ui_mouse.y <= (y+currentPosition+slider_h)) ) {
			slider_mode = S2M_ON_ME;
			if ( B1_PRESSED ) {
				mouse_locked = 1;
			}
		}
	} else
		slider_mode = S2M_DEFAULT;

	if ( !B1_PRESSED) {
		if (mouse_locked == 1)
			if (captureCallback != NULL) {
				captureCallback();
				mprintf(("Called captureCallback()!\n"));
			}
		mouse_locked = 0;
	}			
	
	if (!OnMe && !mouse_locked)
		return;
	
	// could we possibly be moving up?
	if ((OnMe && B1_PRESSED && ui_mouse.y < (currentPosition+y+slider_half_h-1)) || (mouse_locked && (ui_mouse.y < (currentPosition+y+slider_half_h-1))) ) {		
		// make sure we wait at least 50 ms between events unless mouse locked 
		if ( (timer_get_milliseconds() > last_scrolled+50) || B1_JUST_PRESSED || mouse_locked ) {
			last_scrolled = timer_get_milliseconds();
			if (!mouse_locked) {
				if (currentItem > 0) {
					currentItem--;
					if (upCallback != NULL) {
						upCallback();
						if (captureCallback != NULL)
							captureCallback();
					}
				}
				currentPosition = fl2i((((float)currentItem/(float)numberItems) * (float)numberPositions)-.49);
			} else {
				mouse_lock_move = fl2i( ((((float)ui_mouse.y - (float)y - (float)slider_half_h)/(float)numberPositions) * (float)numberItems) -.49);				
				mouse_lock_move = currentItem - mouse_lock_move;
				if (mouse_lock_move > 0) {
					while (mouse_lock_move >  0) {						
						if (currentItem > 0) {
							currentItem--;
							if (upCallback != NULL)
								upCallback();
						}
						mouse_lock_move--;
					}
				}
				// currentPosition = ui_mouse.y - y - slider_half_h;
				currentPosition = fl2i((((float)currentItem/(float)numberItems) * (float)numberPositions)-.49);
			}
			if (currentPosition < 0)
				currentPosition = 0;
			if (currentPosition > numberPositions)
			currentPosition = numberPositions;
			slider_mode = S2M_MOVING;	
		}
	}

	if ( ( OnMe && B1_PRESSED && ui_mouse.y > (currentPosition+y+slider_half_h+1)) || (mouse_locked && (ui_mouse.y > (currentPosition+y+slider_half_h+1)))  ) {		
		// make sure we wait at least 50 ms between events unless mouse locked 
		if ( (timer_get_milliseconds() > last_scrolled+50) || B1_JUST_PRESSED || mouse_locked ) {
			last_scrolled = timer_get_milliseconds();
			if (!mouse_locked) {
				if (currentItem < numberItems) {
					currentItem++;
					if (downCallback != NULL) {
						downCallback();
						if (captureCallback != NULL)
							captureCallback();
					}
				}
				currentPosition = fl2i((((float)currentItem/(float)numberItems) * (float)numberPositions)-.49);
			} else {
				mouse_lock_move = fl2i( ((((float)ui_mouse.y - (float)y - (float)slider_half_h)/(float)numberPositions) * (float)numberItems) -.49);				
				mouse_lock_move -= currentItem;
				if (mouse_lock_move > 0) {
					while (mouse_lock_move > 0) {						
						if  (currentItem < numberItems) {
							currentItem++;
							if (downCallback != NULL)
								downCallback();
						}
						mouse_lock_move--;
					}
				}
				// currentPosition = ui_mouse.y - y - slider_half_h;
				currentPosition = fl2i((((float)currentItem/(float)numberItems) * (float)numberPositions)-.49);
			}	
			if (currentPosition < 0){
				currentPosition = 0;
			}
			if (currentPosition > numberPositions){
				currentPosition = numberPositions;
			}
			slider_mode = S2M_MOVING;
		} 
	} 

	// if we are centerd on the bitmap and still in mouse lock mode, we need to make sure the MOVING bitmap is still shown
	// or if mouse is on us and we are pressing the mouse button
	if (mouse_locked || (OnMe && B1_PRESSED)){
		slider_mode = S2M_MOVING;
	}
}

void UI_SLIDER2::hide()
{
	hidden = 1;
}

void UI_SLIDER2::unhide()
{
	hidden = 0;
}

int UI_SLIDER2::get_hidden()
{
	return hidden;
}


// return number of itmes
int UI_SLIDER2::get_numberItems() {
	return numberItems;
}

// return current position
int UI_SLIDER2::get_currentPosition() {
	return currentPosition;
}

// return current item
int UI_SLIDER2::get_currentItem() {
	return currentItem;
}

// change range. reset back to position 0
void UI_SLIDER2::set_numberItems(int _numberItems, int _reset) {
	numberItems = _numberItems;

	if (_reset) {
		currentItem = 0;
		currentPosition = 0;
	} else {
		// recalcluate current position
		currentPosition = fl2i((((float)currentItem/(float)numberItems) * (float)numberPositions)-.49);
		if (currentPosition < 0){
			currentPosition = 0;
		}
		if (currentPosition > numberPositions){
			currentPosition = numberPositions;
		}
	}
	if (numberItems <= 0){
		disabled_flag = 1;
	} else {
		disabled_flag = 0;
	}
}

// force slider to new position manually
void UI_SLIDER2::set_currentItem(int _currentItem) {
	if (_currentItem > numberItems) 
		goto cpSafety;

	if (_currentItem == currentItem)
		goto cpSafety;

	if (_currentItem < 0)
		goto cpSafety;

	if (_currentItem > currentItem) {
		while (currentItem != _currentItem) {
			currentItem++;
			if (downCallback != NULL)
				downCallback();
		}
	} else if (_currentItem < currentItem) {
		while (currentItem != _currentItem) {
			currentItem--;
			if (upCallback != NULL)
				upCallback();
		}
	}	
	
	currentPosition = fl2i(((float)currentItem/(float)numberItems) * (float)numberPositions);	

cpSafety: // helps fix math problem on x86_64
	if (currentPosition > numberItems)
		currentPosition = numberItems;

	if (currentPosition < 0)
		currentPosition = 0;
}

void UI_SLIDER2::force_currentItem(int _currentItem) {	
	if (_currentItem > numberItems)
		goto cpSafety;

	if (_currentItem == currentItem)
		goto cpSafety;

	currentItem = _currentItem;	

	if(currentItem < 0){
		currentItem = 0;
	};

	currentPosition = fl2i(((float)currentItem/(float)numberItems) * (float)numberPositions);	

cpSafety: // helps fix math problem on x86_64
	if (currentPosition > numberItems)
		currentPosition = numberItems;

	if (currentPosition < 0)
		currentPosition = 0;
}

void UI_SLIDER2::forceDown() {
	if (currentItem < numberItems) {
		currentItem++;
		currentPosition = fl2i(((float)currentItem/(float)numberItems) * (float)numberPositions);
	}
}

void UI_SLIDER2::forceUp() {
	if (currentItem > 0) {
		currentItem--;
		currentPosition = fl2i(((float)currentItem/(float)numberItems) * (float)numberPositions);

		if (currentPosition < 0)
			currentPosition = 0;
	}
}
