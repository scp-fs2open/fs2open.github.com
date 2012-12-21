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
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"

// ---------------------------------------------------------------------------------------
// input:
//			do_repeat		=>		property of button, set to 1 to allow pressed events if mouse
//										pointer is held over button with left mouse button down,
//										otherwise 0 (useful for buttons that scroll items)
//			ignore_focus	=>		whether to allow Enter/Spacebar to affect pressed state when
//										control has focus
//
void UI_BUTTON::create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _w, int _h, int do_repeat, int ignore_focus)
{
	text = NULL;

	if (_text) {
		if ( _text[0] != '\0' ) {
			text = vm_strdup(_text);
		}
	}

	// register gadget with UI window
	base_create( wnd, UI_KIND_BUTTON, _x, _y, _w, _h );

	// initialize variables
	m_flags = 0;
	next_repeat = 0;
	m_just_highlighted_function = NULL;		// assume there is no callback
	m_disabled_function = NULL;				// ditto
	if (do_repeat) {
		m_flags |= BF_REPEATS;
		next_repeat = 1;
	}

	m_press_linger = 1;
	first_callback = 1;

	hotkey_if_focus = KEY_SPACEBAR;

	if (ignore_focus){
		m_flags |= BF_IGNORE_FOCUS;
	}

	custom_cursor_bmap = -1;
	previous_cursor_bmap = -1;
}

void UI_BUTTON::destroy()
{
	if (text) {
		vm_free(text);
		text = NULL;
	}

	UI_GADGET::destroy();  // call base as well
}

// sets a hotkey for button that works only when it had focus (or derived focus)
void UI_BUTTON::set_hotkey_if_focus(int key)
{
	hotkey_if_focus = key;
}

void UI_BUTTON::reset_status()
{
	m_flags &= ~BF_HIGHLIGHTED;
	m_flags &= ~BF_HOTKEY_JUST_PRESSED;
	m_flags &= ~BF_DOWN;
	m_flags &= ~BF_DOUBLE_CLICKED;
	m_flags &= ~BF_JUST_HIGHLIGHTED;
	m_flags &= ~BF_CLICKED;
}

// reset anything that needs to be at the start of a new frame before processing
void UI_BUTTON::frame_reset()
{
	m_flags &= ~BF_HIGHLIGHTED;
	m_flags &= ~BF_HOTKEY_JUST_PRESSED;
	m_flags &= ~BF_DOWN;
	m_flags &= ~BF_JUST_PRESSED;
	m_flags &= ~BF_JUST_RELEASED;
	m_flags &= ~BF_CLICKED;
	m_flags &= ~BF_DOUBLE_CLICKED;
	m_flags &= ~BF_JUST_HIGHLIGHTED;

	restore_previous_cursor();
}

// Force button to draw a specified frame
void UI_BUTTON::draw_forced(int frame_num)
{
	if (uses_bmaps) {
		if (bmap_ids[frame_num] >= 0) {
			gr_set_bitmap(bmap_ids[frame_num]);
			gr_bitmap(x, y);
			
			// my_wnd->draw_tooltip();

			// redraw any associated xstr
			my_wnd->draw_XSTR_forced(this, frame_num);
		}
	}
}

// Render button.  How it draws exactly depends on it's current state.
void UI_BUTTON::draw()
{
	int offset, frame_num = -1;

	if (uses_bmaps) {
		gr_reset_clip();
		// if button is down, draw it that way
		if (button_down()) {
			if (bmap_ids[B_PRESSED] >= 0){
				frame_num = B_PRESSED;
			}
		// otherwise if button is disabled, draw it that way
		} else if (disabled_flag) {
			if (bmap_ids[B_DISABLED] >= 0){
				frame_num = B_DISABLED;
			}
		// otherwise, if button is highlighted (mouse is over it, but mouse buttons not down) draw it that way
		} else if (m_flags & BF_HIGHLIGHTED) {
			if (bmap_ids[B_HIGHLIGHT] >= 0){
				frame_num = B_HIGHLIGHT;
			}
		// otherwise, just draw it normally
		} else {
			if (bmap_ids[B_NORMAL] >= 0){
				frame_num = B_NORMAL;
			}
		}

		if (frame_num >= 0) {
			gr_set_bitmap(bmap_ids[frame_num]);
			gr_bitmap(x, y);
		}
	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h );

		// draw the button's box
		if (button_down()) {
			ui_draw_box_in( 0, 0, w-1, h-1 );
			offset = 1;

		} else {
			ui_draw_box_out( 0, 0, w-1, h-1 );
			offset = 0;
		}

		// now draw the button's text
		if (disabled_flag){
			gr_set_color_fast(&CDARK_GRAY);
		} else if (my_wnd->selected_gadget == this){
			gr_set_color_fast(&CBRIGHT_GREEN);
		} else {
			gr_set_color_fast(&CBLACK);
		}

		if (text){
			ui_string_centered( Middle(w) + offset, Middle(h) + offset, text );
		}

		gr_reset_clip();
	}
}

// process() is called to process the button, which amounts to:
//   If mouse is over button, hilight it
//   If highlighted and mouse button down, flag button as down
//   If hotkey pressed, flag button as down
//   If hotkey_if_focus pressed, and button has focus, flag button as down
//   Set various BF_JUST_* flags if events changed from last frame
//
void UI_BUTTON::process(int focus)
{
	int mouse_on_me, old_flags;

	old_flags = m_flags;
	frame_reset();

	// check mouse over control and handle hilighting state
	mouse_on_me = is_mouse_on();

	// if gadget is disabled, force button up and return
	if (disabled_flag) {
		if (old_flags & BF_DOWN){
			m_flags |= BF_JUST_RELEASED;
		}

		if (!hidden && !my_wnd->use_hack_to_get_around_stupid_problem_flag) {
			if (mouse_on_me && B1_JUST_PRESSED){
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}

			if ( (hotkey >= 0) && (my_wnd->keypress == hotkey) ){
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}

		// do callback if the button is disabled
		if (mouse_on_me && B1_JUST_PRESSED){
			if (m_disabled_function != NULL) {
				m_disabled_function();
			}
		}

		return;
	}

	// check focus and derived focus with one variable
	if (my_wnd->selected_gadget == this) {
		focus = 1;
	}

	// show alternate cursor, perhaps?
	maybe_show_custom_cursor();

	if ( !mouse_on_me ) {
		next_repeat = 0;
	} else {
		m_flags |= BF_HIGHLIGHTED;
		if ( !(old_flags & BF_HIGHLIGHTED) ) {
			int do_callback = 1;
			m_flags |= BF_JUST_HIGHLIGHTED;
			// if a callback exists, call it
			if (m_just_highlighted_function) {

				if ( m_flags & BF_SKIP_FIRST_HIGHLIGHT_CALLBACK ) {
					if ( first_callback ) {
						do_callback = 0;
					}
				}

				first_callback = 0;						
				if ( do_callback ) {
					m_just_highlighted_function();
				}
			}
		}
	}

	// check if mouse is pressed
	if ( B1_PRESSED && mouse_on_me )	{
		m_flags |= BF_DOWN;
		capture_mouse();
	}

	// check if hotkey is down or not
	if ( (hotkey >= 0) && (my_wnd->keypress == hotkey) ) {
		m_flags |= BF_DOWN | BF_CLICKED;
	}

	// only check for space/enter keystrokes if we are not ignoring the focus (this is the
	// default behavior)
	if ( !(m_flags & BF_IGNORE_FOCUS) ) {
		if ( focus && (hotkey_if_focus >= 0) ) {
			if (my_wnd->keypress == hotkey_if_focus)
				m_flags |= BF_DOWN | BF_CLICKED;

			if ( (hotkey_if_focus == KEY_SPACEBAR) && (my_wnd->keypress == KEY_ENTER) )
				m_flags |= BF_DOWN | BF_CLICKED;
		}
	}

	// handler for button not down
	if ( !(m_flags & BF_DOWN) ) {
		next_repeat = 0;
		if ( (old_flags & BF_DOWN) && !(old_flags & BF_CLICKED) )  // check for release of mouse, not hotkey
			m_flags |= BF_JUST_RELEASED;

		// non-repeating buttons behave sort of uniquely..  They activate when released over button
		if (!(m_flags & BF_REPEATS)) {
			if ( (m_flags & BF_JUST_RELEASED) && (m_flags & BF_HIGHLIGHTED) )
				m_flags |= BF_CLICKED;
		}

		return;
	}

	// check if button just went down this frame
	if ( !(old_flags & BF_DOWN) ) {
		m_flags |= BF_JUST_PRESSED;
		m_press_linger = timestamp(100);
		if (user_function)
			user_function();

		if (m_flags & BF_REPEATS) {
			next_repeat = timestamp(B_REPEAT_TIME * 3);
			m_flags |= BF_CLICKED;
		}
	}

	// check if a repeat event should occur
	if ( timestamp_elapsed(next_repeat) && (m_flags & BF_REPEATS) ) {
		next_repeat = timestamp(B_REPEAT_TIME);
		m_flags |= BF_CLICKED;
		m_press_linger = timestamp(100);
	}

	// check for double click occurance
	if (B1_DOUBLE_CLICKED && mouse_on_me) {
		m_flags |= BF_DOUBLE_CLICKED;
		m_press_linger = timestamp(100);
	}
}

// Check if button should do it's function in life (trigger the event)
//
int UI_BUTTON::pressed()
{
	if (m_flags & BF_CLICKED)
		return TRUE;

	return FALSE;
}

int UI_BUTTON::double_clicked()
{
	if ( m_flags & BF_DOUBLE_CLICKED )
		return TRUE;
	else
		return FALSE;
}

int UI_BUTTON::just_pressed()
{
	if ( m_flags & BF_JUST_PRESSED )
		return TRUE;
	else
		return FALSE;
}

int UI_BUTTON::just_highlighted()
{
	if ( m_flags & BF_JUST_HIGHLIGHTED )
		return TRUE;
	else
		return FALSE;
}

// ----------------------------------------------------------------------------------
// Checks if button is down (or up).  This checks the button instead, rather than any
// events that may have caused it to be down.  Buttons also stay down for a certain amount
// of time minimum, to make sure it's long enough for the user to see it has went down (since
// one frame is probably far to quick for users to notice it).  Basically, this indicates
// how the button is being drawn, if you want to think of it that way.
int UI_BUTTON::button_down()
{
	if ( (m_flags & BF_DOWN) || !timestamp_elapsed(m_press_linger) )
		return TRUE;
	else
 		return FALSE;
}

// ------------------------------------------------------------
// set the callback function for when the mouse first goes over
// a button
//
void UI_BUTTON::set_highlight_action( void (*_user_function)(void) )
{
	m_just_highlighted_function = _user_function;
}

void UI_BUTTON::set_disabled_action( void (*_user_function)(void) )
{
	m_disabled_function = _user_function;
}


// Is the mouse over this button?
int UI_BUTTON::button_hilighted()
{
	return m_flags & BF_HIGHLIGHTED;
}

// Is the mouse over this button?
void UI_BUTTON::set_button_hilighted()
{
	m_flags |= BF_HIGHLIGHTED;
}

// Force button to get pressed
void UI_BUTTON::press_button()
{
	if ( !disabled_flag ) {
		m_flags |= BF_DOWN | BF_CLICKED;
		//m_flags |= BF_JUST_PRESSED;
	}
}
		
// reset the "pressed" timestamps
void UI_BUTTON::reset_timestamps()
{
	m_press_linger = 1;
	next_repeat = 0;
}

void UI_BUTTON::skip_first_highlight_callback()
{
	m_flags |= BF_SKIP_FIRST_HIGHLIGHT_CALLBACK;
	first_callback = 1;
}

void UI_BUTTON::repeatable(int yes)
{
	if(yes){
		m_flags |= BF_REPEATS;
		next_repeat = 1;
	} else {
		m_flags &= ~(BF_REPEATS);
		next_repeat = 0;
	}
}

void UI_BUTTON::maybe_show_custom_cursor()
{
	if (disabled_flag)
		return;

	// set the mouseover cursor 
	if (is_mouse_on()) {
		if ((custom_cursor_bmap >= 0) && (previous_cursor_bmap < 0)) {
			previous_cursor_bmap = gr_get_cursor_bitmap();
			gr_set_cursor_bitmap(custom_cursor_bmap, GR_CURSOR_LOCK);			// set and lock
		}
	}
}

void UI_BUTTON::restore_previous_cursor()
{
	if (previous_cursor_bmap >= 0) {
		gr_set_cursor_bitmap(previous_cursor_bmap, GR_CURSOR_UNLOCK);		// restore and unlock
		previous_cursor_bmap = -1;
	}
}
