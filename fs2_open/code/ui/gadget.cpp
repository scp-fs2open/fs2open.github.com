/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ui/GADGET.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:10 $
 * $Author: penguin $
 *
 * Functions for the base gadget class
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 18    8/11/99 3:21p Jefff
 * set_bmaps clarification by daveb
 * 
 * 17    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 16    7/16/99 1:50p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 15    6/25/99 11:59a Dave
 * Multi options screen.
 * 
 * 14    5/21/99 6:45p Dave
 * Sped up ui loading a bit. Sped up localization disk access stuff. Multi
 * start game screen, multi password, and multi pxo-help screen.
 * 
 * 13    5/03/99 8:33p Dave
 * New version of multi host options screen.
 * 
 * 12    4/16/99 5:23p Neilk
 * Removed bitmap mprintf's to make debugging faster
 * 
 * 11    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 10    2/05/99 4:09p Anoop
 * Hopefully the last fix for this file :)
 * 
 * 9     2/05/99 3:26p Enricco
 * Oops. Some .ani's can have 4 frames.
 * 
 * 8     2/05/99 3:07p Davidg
 * Fixed braindead bmap setting code.
 * 
 * 7     2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
 * 
 * 6     12/21/98 9:05a Dave
 * Changed UI code so it doesn't require 3 bitmaps for a 3 state button.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 22    5/11/98 5:29p Hoffoss
 * Added mouse button mapped to joystick button support.
 * 
 * 21    4/12/98 2:09p Dave
 * Make main hall door text less stupid. Make sure inputbox focus in the
 * multi host options screen is managed more intelligently.
 * 
 * 20    4/09/98 7:14p Hoffoss
 * Did some cool changes for tooltips.
 * 
 * 19    2/11/98 6:24p Hoffoss
 * Fixed bug where disabled and hidden buttons give failed sound when
 * pressed.  Shouldn't happen when they are hidden.
 * 
 * 18    1/15/98 11:54a Hoffoss
 * Embelished file with nifty comments.
 * 
 * 17    1/14/98 6:43p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
 * 
 * 16    12/30/97 4:26p Lawrance
 * Disable right-click movement of ui controls
 * 
 * 15    10/24/97 10:58p Hoffoss
 * Made some changes to the UI code to do some things I need it to do.
 * 
 * 14    10/12/97 3:45p Lawrance
 * only allow movement of UI controls in debug
 * 
 * 13    10/09/97 4:56p Lawrance
 * init GADGET stuff in base_create(), not the constructor
 * 
 * 12    9/07/97 10:05p Lawrance
 * don't set hotspot_num, done in gadget constructor
 * 
 * 11    8/29/97 7:34p Lawrance
 * use bmpman for storing bitmaps for ui controls
 * 
 * 10    8/18/97 5:28p Lawrance
 * integrating sounds for when mouse goes over an active control
 * 
 * 9     7/20/97 7:00p Lawrance
 * changed name of some anim_ functions to be more consistent
 * 
 * 8     5/26/97 10:26a Lawrance
 * get slider control working 100%
 * 
 * 7     5/22/97 5:36p Lawrance
 * allowing custom art for scrollbars
 * 
 * 6     5/21/97 11:07a Lawrance
 * integrate masks and custom bitmaps
 * 
 * 5     4/28/97 2:19p Lawrance
 * added clear_focus() function
 * 
 * 4     1/28/97 4:58p Lawrance
 * allowing hidden UI components
 * 
 * 3     12/02/96 2:17p John
 * Made right button drag UI gadgets around and
 * Ctrl+Shift+Alt+F12 dumps out where they are.
 * 
 * 2     11/21/96 10:58a John
 * Started adding code to drag buttons.
 * 
 * 1     11/14/96 6:55p John
 *
 * $NoKeywords: $
 */

#include "ui/uidefs.h"
#include "ui/ui.h"
#include "bmpman/bmpman.h"
#include "anim/animplay.h"

// constructor
UI_GADGET::UI_GADGET()
{
	bm_filename = NULL;
}

// destructor
UI_GADGET::~UI_GADGET()
{
}

int UI_GADGET::get_hotspot()
{
	if (!linked_to_hotspot)
		return -1;

	return hotspot_num;
}

void UI_GADGET::reset()
{
	m_flags = 0;
}

// --------------------------------------------------------------------
// Links a hotspot (palette index in mask) to the given gadget.
//
void UI_GADGET::link_hotspot(int num)
{
	hotspot_num = num;
	linked_to_hotspot = 1;
}

// --------------------------------------------------------------------
// Extract MAX_BMAPS_PER_CONTROL bitmaps for the different states of the control.
//
// The bitmap ids are stored in the bmap_ids[] array.  If you don't want to store
// from the zero index, fill in the offset parameter.  offset is a default parameter
// with a default value of zero.
//
// NOTE:  The bitmaps stored in a .ani file.  What each frame is used for
//			 is left up to the component to decide.
//

// loads nframes bitmaps, starting at index start_frame.
// anything < start_frame will not be loaded.
// this keeps the loading code from trying to load bitmaps which don't exist
// and taking an unnecessary disk hit.		
int UI_GADGET::set_bmaps(char *ani_fname, int nframes, int start_frame)
{
	int first_frame, i;	
	char full_name[MAX_FILENAME_LEN] = "";
	char tmp[10];
	int idx, s_idx;	
	int num_digits;
	int its_all_good = 0;
	
	// clear out all frames
	for(idx=0; idx<MAX_BMAPS_PER_GADGET; idx++){
		bmap_ids[idx] = -1;
	}
	
	// load all the bitmaps
	bm_filename = ani_fname;

	Assert(nframes < MAX_BMAPS_PER_GADGET);		
	m_num_frames = nframes;		
	for(idx=start_frame; idx<nframes; idx++){
		// clear the string
		strcpy(full_name, "");

		// get the # of digits for this index
		num_digits = (idx < 10) ? 1 : (idx < 100) ? 2 : (idx < 1000) ? 3 : 4;

		// build the actual filename
		strcpy(full_name, ani_fname);		
		for(s_idx=0; s_idx<(4-num_digits); s_idx++){
			strcat(full_name, NOX("0"));
		}
		strcat(full_name, itoa(idx, tmp, 10));		

		// try and load the bitmap				
		bmap_ids[idx] = bm_load(full_name);	
		if(bmap_ids[idx] != -1){		
			// bm_lock(bmap_ids[idx], 16, 0);
			// bm_unlock(bmap_ids[idx]);
			
			its_all_good = 1;
		} else {
			// mprintf(("Skipping %s\n", full_name));
			// its_all_good = 0;
		}
	}

	// done
	if(its_all_good){
		uses_bmaps = 1;		
		return 0;
	}

	// no go, so try and load as an ani. try and load as an .ani	
	first_frame = bm_load_animation(ani_fname, &m_num_frames);	
	if((first_frame >= 0) && (m_num_frames <= MAX_BMAPS_PER_GADGET)){					
		// seems pretty stupid that we didn't just use a variable for the first frame and access all
		// other frames offset from it instead of accessing this bmap_ids[] array, but probably too
		// much trouble to go through and change this anymore.  How sad..
		for ( i=0; i<m_num_frames; i++ ) {
			bmap_ids[i] = first_frame + i;
		}	
	}	

	// flag that this control is using bitmaps for art	
	uses_bmaps = 1;
	return 0;
}

// Handle drawing of all children of the gadget.  Since this the base of all other gadgets,
// it doesn't have any look to it (kind of like a soul.  Can you see someone's soul?) and thus
// doesn't actually render itself.
//
void UI_GADGET::draw()
{
	UI_GADGET *cur;

	// if hidden, hide children as well
	if (hidden){
		return;
	}

	if (children) {
		cur = children;
		do {
			cur->draw();
			cur = cur->next;

		} while (cur != children);
	}
}

//	Free up bitmaps used by the gadget, and call children to destroy themselves as well.
//
void UI_GADGET::destroy()
{
	int i;
	UI_GADGET *cur;

	for ( i=0; i<m_num_frames; i++ ) {
		if (bmap_ids[i] != -1) {
			bm_release(bmap_ids[i]);
			bmap_ids[i] = -1;
		}
	}

	if (children) {
		cur = children;
		do {
			cur->destroy();
			cur = cur->next;

		} while (cur != children);
	}
}

// Use this if you need to change the size and position of a gadget after you have created it.
//
void UI_GADGET::update_dimensions(int _x, int _y, int _w, int _h)
{
	if ( _x != -1 ) x = _x;
	if ( _y != -1 ) y = _y;
	if ( _w != -1 ) w = _w;
	if ( _h != -1 ) h = _h;	
}

void UI_GADGET::get_dimensions(int *x_, int *y_, int *w_, int *h_)
{
	*x_ = x;
	*y_ = y;
	*w_ = w;
	*h_ = h;	
}

// Hide (or show) a gadget.
//  n != 0: Hide gadget
//  n == 0: Show gadget
//
void UI_GADGET::hide(int n)
{
	hidden = n ? 1 : 0;
}

void UI_GADGET::unhide()
{
	hidden = 0;
}

// Capture mouse with this gadget, which basically means only this gadget will get process()
// called on it until the mouse button 1 is released.
//
void UI_GADGET::capture_mouse()
{
	my_wnd->capture_mouse(this);
}

// Check if (return true if):
//   mouse_captured():	this gadget has the mouse captured.
//   mouse_captured(x):	gadget x has the mouse captured.
//
int UI_GADGET::mouse_captured(UI_GADGET *gadget)
{
	if (!gadget)
		gadget = this;

	return (my_wnd->mouse_captured_gadget == gadget);
}

// Set up the gadget and registers it with the UI window
//
void UI_GADGET::base_create(UI_WINDOW *wnd, int _kind, int _x, int _y, int _w, int _h)
{
	int i;

	// initialize data with passed values
	kind = _kind;
	x = _x;
	y = _y;
	w = _w;
	h = _h;

	// set up reference to UI window and initialize as having no family
	my_wnd = wnd;
	parent = NULL;
	children = NULL;
	next = prev = this;

	// this actually links the gadget into the UI window's top level family (as the youngest gadget)
	set_parent(NULL);

	// initialize variables
	hotkey = -1;
	user_function = NULL;
	disabled_flag = 0;
	base_dragging = 0;
	base_drag_x = base_drag_y = 0;
	hotspot_num = -1;
	hidden = 0;
	linked_to_hotspot = 0;
	uses_bmaps = 0;
	m_num_frames = 0;
	for ( i=0; i<MAX_BMAPS_PER_GADGET; i++ ) {
		bmap_ids[i] = -1;
	}
}

void UI_GADGET::set_hotkey(int key)
{
	hotkey = key;
}

// Far as I can tell, the callback function is handled differently for each gadget type, if
// handled by a given gadget type at all.
//
void UI_GADGET::set_callback(void (*_user_function)(void))
{
	user_function = _user_function;
}

// get the next enabled gadget in sibling list or self if none
//
UI_GADGET *UI_GADGET::get_next()
{
	UI_GADGET *tmp;

	tmp = next;
	while ((tmp != this) && tmp->disabled_flag)
		tmp = tmp->next;

	return tmp;
}

// get the previous enabled gadget in sibling list or self if none
//
UI_GADGET *UI_GADGET::get_prev()
{
	UI_GADGET *tmp;

	tmp = prev;
	while ((tmp != this) && tmp->disabled_flag)
		tmp = tmp->prev;

	return tmp;
}

// Set this gadget as the focus (selected gadget) of the UI window
//
void UI_GADGET::set_focus()
{
	my_wnd->selected_gadget = this;
}

// Make no gadget have focus in the UI window.
//
void UI_GADGET::clear_focus()
{
	my_wnd->selected_gadget = NULL;
}

// Return true or false if this gadget currently has the focus
int UI_GADGET::has_focus()
{
	return my_wnd->selected_gadget == this ? 1 : 0;
}

// get mouse pointer position relative to gadget's origin (UL corner)
//
void UI_GADGET::get_mouse_pos(int *xx, int *yy)
{
	if (xx)
		*xx = ui_mouse.x - x;
	if (yy)
		*yy = ui_mouse.y - y;
}

// Call process() for all children of gadget.  As a base gadget for all other gadget types,
// it doesn't actually do anything for itself.
//
void UI_GADGET::process(int focus)
{
	UI_GADGET *tmp;

	if (disabled_flag)
		return;

	tmp = children;  // process all children of gadget
	if (tmp) {
		do {
			tmp->process();
			tmp = tmp->next;

		} while (tmp != children);
	}
}

// Check if the mouse is over the gadget's area or not,
//
int UI_GADGET::is_mouse_on()
{
	int offset, pixel_val;
	ubyte *mask_data;
	int mask_w, mask_h;

	// if linked to a hotspot, use the mask for determination
	if (linked_to_hotspot) {
		mask_data = (ubyte*)my_wnd->get_mask_data(&mask_w, &mask_h);
		if ( mask_data == NULL ) {
			nprintf(("Warning", "No mask defined, but control is linked to hotspot\n"));
			Int3();
			return 0;
		}

		// if the mouse values are out of range of the bitmap
		// NOTE : this happens when using smaller mask bitmaps than the screen resolution (during development)
		if((ui_mouse.x >= mask_w) || (ui_mouse.y >= mask_h)){
			return 0;
		}

		// check the pixel value under the mouse
		offset = ui_mouse.y * mask_w + ui_mouse.x;
		pixel_val = *(mask_data + offset);
		if (pixel_val == hotspot_num){
			return 1;
		} else {
			return 0;
		}
	// otherwise, we just check the bounding box area
	} else {
		if ((ui_mouse.x >= x) && (ui_mouse.x < x + w) && (ui_mouse.y >= y) && (ui_mouse.y < y + h) ){
			return 1;
		} else {
			return 0;
		}
	}
}

// check if mouse is over any child of this gadget
//
int UI_GADGET::is_mouse_on_children()
{
	UI_GADGET *tmp;
	
	tmp = children;
	if (tmp) {
		do {
			if (tmp->is_mouse_on())
				return 1;
			if (tmp->is_mouse_on_children())
				return 1;

			tmp = tmp->next;

		} while (tmp != children);
	}

	return 0;	
}

void UI_GADGET::disable()
{
	disabled_flag = 1;
}

// Enables (or possibly disables) the gadget.  n is an optional argument.  If not supplied,
// enables the garget.  If supplied, enables garget if n is true, disables it if n is false.
//
void UI_GADGET::enable(int n)
{
	disabled_flag = n ? 0 : 1;
}

// Check if gadget is disabled
//
int UI_GADGET::disabled()
{
	return disabled_flag;
}

// Check if gadget is enabled
//
int UI_GADGET::enabled()
{
	return !disabled_flag;
}

void UI_GADGET::drag_with_children( int dx, int dy )
{
	UI_GADGET *tmp;

	x = dx + base_start_x;
	y = dy + base_start_y;
	
	tmp = children;
	if (tmp) {
		do {
			tmp->drag_with_children(dx, dy);
			tmp = tmp->next;

		} while (tmp != children);
	}
}

void UI_GADGET::start_drag_with_children()
{
	UI_GADGET *tmp;

	base_dragging = 1;
	base_start_x = x;
	base_start_y = y;
	
	tmp = children;
	if (tmp) {
		do {
			tmp->start_drag_with_children();
			tmp = tmp->next;

		} while (tmp != children);
	}
}

void UI_GADGET::stop_drag_with_children()
{
	UI_GADGET *tmp;

	base_dragging = 0;
	tmp = children;
	if (tmp) {
		do {
			tmp->stop_drag_with_children();
			tmp = tmp->next;

		} while (tmp != children);
	}
}

// Returns 1 if moving
int UI_GADGET::check_move()
{
	#if 0
		if ( parent != NULL ) return base_dragging;

		if ( !base_dragging )	{

			if ( B2_JUST_PRESSED )	{
				if ( is_mouse_on() || is_mouse_on_children() ) {
					start_drag_with_children();
					base_drag_x = ui_mouse.x;
					base_drag_y = ui_mouse.y;
					return 1;
				} else {
					return 0;
				}
			} else 
				return 0;
		} else {
			drag_with_children(ui_mouse.x - base_drag_x,ui_mouse.y - base_drag_y);
			nprintf(( "UI", "UI: X=%d, Y=%d, Delta=(%d,%d)\n", x, y, (ui_mouse.x - base_drag_x), (ui_mouse.y - base_drag_y) ));
			if (B2_RELEASED)	{
				stop_drag_with_children();
			}
			return 1;
		}
	#endif
	return 0;
}

// Take gadget out of family.  Children of this gadget stay with gadget, though.
//
// A family is basically the whole parent/sibling/children hierarchy for gadgets.  Any gadget
// that is linked to another gadget by one of these pointers is said to be in the same family
// as that gadget.  This function, therefore, caused all references to a gadget by it's
// family's gadgets' sibling or children pointers to be broken, and all references to any of them
// by this gadget's parent or sibling pointers to also be broken.  This isolates the gadget and
// it's children into a new family.
//
void UI_GADGET::remove_from_family()
{
	if (parent) {
		if (parent->children == this) {
			if (next == this)  // an only child?
				parent->children = NULL;  // if so, parent now has no children
			else
				parent->children = next;  // next sibling is now the eldest
		}

	} else {
		if (my_wnd->first_gadget == this) {
			if (next == this)  // an only child?
				my_wnd->first_gadget = NULL;  // if so, parent now has no children
			else
				my_wnd->first_gadget = next;  // next sibling is now the eldest
		}
	}

	parent = NULL;
	if (next != this) {  // does gadget have siblings?
		next->prev = prev;
		prev->next = next;
	}

	next = prev = this;
}

// Put gadget into a new family (removing from old one if needed first).
// See remove_from_family() for definition of what a family is.
//
void UI_GADGET::set_parent(UI_GADGET *daddy)
{
	remove_from_family();
	parent = daddy;

	if (!daddy) {
		if (!my_wnd->first_gadget) {
			my_wnd->first_gadget = this;

		} else {
			UI_GADGET *eldest_sibling, *youngest_sibling;

			eldest_sibling = my_wnd->first_gadget;
			youngest_sibling = eldest_sibling->prev;

			next = eldest_sibling;
			prev = youngest_sibling;

			eldest_sibling->prev = this;
			youngest_sibling->next = this;
		}

		return;
	}

	if (!daddy->children) {
		daddy->children = this;

	} else {
		UI_GADGET *eldest_sibling, *youngest_sibling;

		eldest_sibling = daddy->children;
		youngest_sibling = eldest_sibling->prev;

		next = eldest_sibling;
		prev = youngest_sibling->prev;

		eldest_sibling->prev = this;
		youngest_sibling->next = this;
	}
}
