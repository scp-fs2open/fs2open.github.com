/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ui/UI.H $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:02 $
 * $Author: penguin $
 *
 * Include file for our user interface.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 20    8/16/99 9:45a Jefff
 * changes to cursor management to allow a 2nd temporary cursor
 * 
 * 19    8/11/99 3:21p Jefff
 * set_bmaps clarification by daveb
 * 
 * 18    8/11/99 12:18p Jefff
 * added option to slider2 class to not force slider reset on
 * set_numberItems
 * 
 * 17    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 16    8/05/99 2:44p Jefff
 * added disabled callback to UI_BUTTON
 * 
 * 15    6/25/99 11:59a Dave
 * Multi options screen.
 * 
 * 14    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 13    5/21/99 6:45p Dave
 * Sped up ui loading a bit. Sped up localization disk access stuff. Multi
 * start game screen, multi password, and multi pxo-help screen.
 * 
 * 12    5/04/99 5:20p Dave
 * Fixed up multiplayer join screen and host options screen. Should both
 * be at 100% now.
 * 
 * 11    5/03/99 8:33p Dave
 * New version of multi host options screen.
 * 
 * 10    4/29/99 2:15p Neilk
 * fixed slider so there is an extra callback for mouse locks
 * 
 * 9     4/16/99 5:22p Neilk
 * Added UI_SLIDER2 class
 * 
 * 8     2/21/99 6:02p Dave
 * Fixed standalone WSS packets. 
 * 
 * 7     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 6     2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
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
 * 71    5/12/98 11:59p Dave
 * Put in some more functionality for Parallax Online.
 * 
 * 70    5/11/98 5:29p Hoffoss
 * Added mouse button mapped to joystick button support.
 * 
 * 69    5/05/98 1:49a Lawrance
 * Add member function to disable processing for all gadgets in a window
 * 
 * 68    5/03/98 1:55a Lawrance
 * Add function call that forces button code to skip first callback for
 * highlighing
 * 
 * 67    4/17/98 12:22a Dave
 * Bumped up MAX_TOOLTIPS from 350 to 500 so Freespace will start.
 * 
 * 66    4/14/98 5:07p Dave
 * Don't load or send invalid pilot pics. Fixed chatbox graphic errors.
 * Made chatbox display team icons in a team vs. team game. Fixed up pause
 * and endgame sequencing issues.
 * 
 * 65    4/14/98 4:27p Hoffoss
 * Changed the way tooltips render as requested.
 * 
 * 64    4/13/98 4:30p Hoffoss
 * Fixed a bunch of stupid little bugs in options screen.  Also changed
 * forced_draw() to work like it used to.
 * 
 * 63    4/12/98 2:09p Dave
 * Make main hall door text less stupid. Make sure inputbox focus in the
 * multi host options screen is managed more intelligently.
 * 
 * 62    4/10/98 5:36p Dave
 * Put in user notification of illegal values in multi host options
 * screen. Fixed server respawn ship class problem.
 * 
 * 61    4/10/98 4:51p Hoffoss
 * Made several changes related to tooltips.
 * 
 * 60    4/09/98 7:14p Hoffoss
 * Did some cool changes for tooltips.
 * 
 * 59    4/09/98 5:57p Hoffoss
 * Added custom tooltip handler functionality for callback.
 * 
 * 58    4/09/98 12:12p Mike
 * Separate versioning for demo and full versions.
 * Fix inputbox bugs.
 * 
 * 57    3/30/98 6:24p Hoffoss
 * Added the tooltip (foreign language translation of text) system.
 * 
 * 56    3/23/98 5:48p Hoffoss
 * Improved listbox handling.  Most notibly the scrollbar arrows work now.
 * 
 * 55    3/22/98 10:50p Lawrance
 * Allow sliders to not have end-buttons.
 * 
 * 54    3/10/98 4:06p Hoffoss
 * Removed unused variables.
 * 
 * 53    3/09/98 5:55p Dave
 * Fixed stats to take asteroid hits into account. Polished up UI stuff in
 * team select. Finished up pilot info popup. Tracked down and fixed
 * double click bug.
 * 
 * 52    3/02/98 3:54p Lawrance
 * make button draw function public
 * 
 * 51    2/26/98 4:21p Dave
 * More robust multiplayer voice.
 * 
 * 50    2/11/98 6:24p Hoffoss
 * Fixed bug where disabled and hidden buttons give failed sound when
 * pressed.  Shouldn't happen when they are hidden.
 * 
 * 49    2/09/98 10:03a Hoffoss
 * Made first_time variable public so I can clear the stupid thing in code
 * that I want it to be cleared in.
 * 
 * 48    1/26/98 6:28p Lawrance
 * Add ability to for a button press event externally.
 * 
 * 47    1/23/98 5:43p Dave
 * Finished bringing standalone up to speed. Coded in new host options
 * screen.
 * 
 * 46    1/16/98 7:57p Lawrance
 * support animating input box cursor
 * 
 * 45    1/15/98 5:10p Allender
 * ton of interface changes.  chatbox in multiplayer now behaves
 * differently than before.  It's always active in any screen that uses
 * it.  Only non-printatble characters will get passed back out from
 * chatbox
 * 
 * 44    1/14/98 6:44p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
 * 
 * 43    1/02/98 9:11p Lawrance
 * Add button_hot() function
 * 
 * 42    12/22/97 5:08p Hoffoss
 * Changed inputbox class to be able to accept only certain keys, changed
 * pilot screens to utilize this feature.  Added to assert with pilot file
 * saving.
 * 
 * 41    12/11/97 8:15p Dave
 * Put in network options screen. Xed out olf protocol selection screen.
 * 
 * 40    12/10/97 3:14p Dave
 * Added an overloaded set_mask_bmap(int) function for the UI_WINDOW
 * 
 * 39    12/08/97 6:22p Lawrance
 * blink cursor on inputbox
 * 
 * 38    12/06/97 4:27p Dave
 * Another load of interface and multiplayer bug fixes.
 * 
 * 37    11/25/97 3:51p Hoffoss
 * Changed edit background rect position slightly.
 * 
 * 36    11/19/97 8:32p Hoffoss
 * Changed UI buttons so they go back to unpressed when they are disabled.
 * 
 * 35    10/29/97 7:25p Hoffoss
 * Added crude support for UI button double click checking.
 * 
 * 34    10/24/97 10:58p Hoffoss
 * Made some changes to the UI code to do some things I need it to do.
 * 
 * 33    10/01/97 4:40p Lawrance
 * allow process() to have key input
 * 
 * 32    9/18/97 10:31p Lawrance
 * allow listbox to change text
 * 
 * 31    9/09/97 4:32p Dave
 * Added sel_changed() function to UI_LISTBOX to check is the selection
 * has changed.
 * 
 * 30    9/07/97 10:05p Lawrance
 * add icon class
 * 
 * 29    8/30/97 12:23p Lawrance
 * add button function to reset the status of a button
 * 
 * 28    8/29/97 7:33p Lawrance
 * move cleanup code from destructor to destroy() method
 * 
 * 27    8/24/97 5:25p Lawrance
 * improve drawing of buttons 
 * 
 * 26    8/21/97 12:13p Dave
 * Made it possible for input box to ignore esc to lose focus.
 * 
 * 25    8/19/97 1:27p Dave
 * Modified input box to allow character limitation by pixel width.
 * Changed list box so that you can create an empty box and add items as
 * you need to.
 * 
 * 24    8/18/97 5:28p Lawrance
 * integrating sounds for when mouse goes over an active control
 * 
 * 23    8/17/97 2:42p Lawrance
 * add code to have selected bitmap for buttons linger for a certain time
 * 
 * 22    8/15/97 8:21p Dave
 * Modified UI_INPUTBOX so that it is possible to draw it invisibly. That
 * is, only the text is displayed.
 * 
 * 21    8/14/97 5:23p Dave
 * Added clear_all_items() to the UI_LISTBOX
 * 
 * 20    6/13/97 5:51p Lawrance
 * add in support for repeating buttons
 * 
 * 19    6/11/97 1:32p Allender
 * externed sort_filelist
 * 
 * 18    5/26/97 10:26a Lawrance
 * get slider control working 100%
 * 
 * 17    5/22/97 5:36p Lawrance
 * allowing custom art for scrollbars
 * 
 * 16    5/21/97 11:07a Lawrance
 * integrate masks and custom bitmaps
 * 
 * 15    4/28/97 2:19p Lawrance
 * added clear_focus() function
 * 
 * 14    4/22/97 10:11a John
 * Added checkbox lists to listboxes
 * 
 * 13    4/15/97 3:47p Allender
 * moved type selection of list box items into actual UI code.  Made it
 * behave more like windows listboxes do
 * 
 * 12    1/28/97 4:58p Lawrance
 * allowing hidden UI components
 * 
 * 11    12/23/96 2:42p Lawrance
 * allowing keys to select list box items in the mission load screen
 * 
 * 10    12/04/96 3:00p John
 * Added code to allow adjusting of HUD colors and saved it to the player
 * config file.
 * 
 * 9     12/03/96 3:46p Lawrance
 * added ability to set contents of input box
 * 
 * 8     12/03/96 11:29a John
 * Made scroll buttons on listbox scroll once, then delay, then repeat
 * when the buttons are held down.
 * 
 * 7     12/02/96 2:17p John
 * Made right button drag UI gadgets around and
 * Ctrl+Shift+Alt+F12 dumps out where they are.
 * 
 * 6     12/01/96 3:48a Lawrance
 * added function set_current to UI_LISTBOX
 * 
 * 5     11/29/96 6:08p Lawrance
 * enabled check-boxes to be set to a specific value outside of the
 * create() function
 * 
 * 4     11/21/96 10:58a John
 * Started adding code to drag buttons.
 * 
 * 3     11/18/96 4:28p Jasen
 * making class member process return an int
 * 
 * 2     11/15/96 11:42a John
 * 
 * 1     11/14/96 6:55p John
 *
 * $NoKeywords: $
 */

#ifndef _UI_H
#define _UI_H

#include "2d.h"

#define UI_KIND_BUTTON				1
#define UI_KIND_KEYTRAP				2
#define UI_KIND_CHECKBOX			3
#define UI_KIND_RADIO				4
#define UI_KIND_SCROLLBAR			5
#define UI_KIND_LISTBOX				6
#define UI_KIND_INPUTBOX			7
#define UI_KIND_SLIDER				8
#define UI_KIND_ICON					9
#define UI_KIND_DOT_SLIDER			10
#define UI_KIND_SLIDER2				11
#define UI_KIND_DOT_SLIDER_NEW	12

#define MAX_KEY_BUFFER				32		// for listboxes

#define MAX_BMAPS_PER_GADGET		15

#define UI_INPUTBOX_FLAG_INVIS			(1 << 0)		// don't draw the input box boarders
#define UI_INPUTBOX_FLAG_KEYTHRU			(1 << 1)		// pass all keypresses through to parent
#define UI_INPUTBOX_FLAG_ESC_CLR			(1 << 2)		// allow escape key to clear input box
#define UI_INPUTBOX_FLAG_ESC_FOC			(1 << 3)		// escape loses focus for the input box
#define UI_INPUTBOX_FLAG_PASSWD			(1 << 4)		// display all characters as special "password" characters
#define UI_INPUTBOX_FLAG_EAT_USED		(1 << 5)		// don't return any characters actually used by inputbox
#define UI_INPUTBOX_FLAG_LETTER_FIRST	(1 << 6)		// require input string to begin with a letter.
#define UI_INPUTBOX_FLAG_NO_LETTERS		(1 << 7)		// don't allow [a-z,A-Z] at all, no matter what
#define UI_INPUTBOX_FLAG_NO_NUMERALS	(1 << 8)		// don't allow [0-9] at all, no matter what
#define UI_INPUTBOX_FLAG_TEXT_CEN		(1 << 9)		// always draw text centered in the inputbox
#define UI_INPUTBOX_FLAG_NO_BACK			(1 << 10)	// don't draw a black background rectangle

#define UI_GF_MOUSE_CAPTURED				(1 << 31)  // gadget has all rights to the mouse

class UI_WINDOW;
class UI_BUTTON;
class UI_KEYTRAP;
class UI_CHECKBOX;
class UI_RADIO;
class UI_SCROLLBAR;
class UI_LISTBOX;
class UI_INPUTBOX;
// class UI_SLIDER;
class UI_DOT_SLIDER;
class UI_DOT_SLIDER_NEW;

class UI_GADGET
{
	friend UI_WINDOW;
	friend UI_BUTTON;
	friend UI_KEYTRAP;
	friend UI_CHECKBOX;
	friend UI_RADIO;
	friend UI_SCROLLBAR;
	friend UI_LISTBOX;
	friend UI_INPUTBOX;
	// friend UI_SLIDER;
	friend UI_DOT_SLIDER;	
	friend UI_DOT_SLIDER_NEW;

	protected:
		char *bm_filename;
		int kind;
		int hotkey;
		int x, y, w, h;
		int m_flags;
		void (*user_function)(void);
		int disabled_flag;
		int base_dragging;
		int base_drag_x, base_drag_y;
		int base_start_x, base_start_y;
		int hidden;

		// Data for supporting linking controls to hotspots
		int linked_to_hotspot;
		int hotspot_num;

		// Data for supporting bitmaps associated with different states of the control
		int uses_bmaps;
		int m_num_frames;
//		ubyte		*bmap_storage[MAX_BMAPS_PER_GADGET];

		void drag_with_children( int dx, int dy );
		void start_drag_with_children();
		void stop_drag_with_children();

		UI_GADGET *parent;
		UI_GADGET *children;
		UI_GADGET *prev;
		UI_GADGET *next;

		int is_mouse_on_children();
		void remove_from_family();
		void set_parent(UI_GADGET *_parent);

		UI_GADGET *get_next();
		UI_GADGET *get_prev();
		UI_WINDOW *my_wnd;
		
		virtual void process(int focus = 0);
		virtual void destroy();
		int check_move();

	public:
		int bmap_ids[MAX_BMAPS_PER_GADGET];

		UI_GADGET();	// constructor
		~UI_GADGET();	// destructor

		void base_create( UI_WINDOW *wnd, int kind, int x, int y, int w, int h );
		virtual void draw();
		void set_focus();
		void clear_focus();
		int has_focus();
		void set_hotkey(int keycode);
		void set_callback(void (*user_function)(void));
		void disable();
		void enable(int n = 1);
		void capture_mouse();
		int mouse_captured(UI_GADGET *gadget = NULL);
		int disabled();
		int enabled();
		virtual void hide(int n = 1);
		virtual void unhide();
		void update_dimensions(int x, int y, int w, int h);
		void get_dimensions(int *x, int *y, int *w, int *h);
		int is_mouse_on();
		void get_mouse_pos(int *x, int *y);

		void link_hotspot(int num);
		int get_hotspot();
		int bmaps_used() { return uses_bmaps; }

		// loads nframes bitmaps, starting at index start_frame.
		// anything < start_frame will not be loaded.
		// this keeps the loading code from trying to load bitmaps which don't exist
		// and taking an unnecessary disk hit.		
		int set_bmaps(char *ani_filename, int nframes = 3, int start_frame = 1);		// extracts MAX_BMAPS_PER_GADGET from .ani file		

		void reset();  // zero out m_flags
		int is_hidden() { return hidden; }
};

// xstrings for a window
#define UI_NUM_XSTR_COLORS			2
#define UI_XSTR_COLOR_GREEN		0			// shades of green/gray
#define UI_XSTR_COLOR_PINK			1			// pinkish hue
typedef struct UI_XSTR {
	char *xstr;										// base string
	int xstr_id;									// xstring id	
	int x, y;										// coords of the string
	int clr;											// color to use
	int font_id;									// font id		
	UI_GADGET *assoc;								// the associated gadget
} UI_XSTR;

#define MAX_UI_XSTRS					100

// Button terminology:
//   Up = button is in up state (also called pressed)
//   Down = button is in down state (also called released)
//   Just pressed = button has just gone from up to down state
//   Just released = button has just gone from down to up state
//   Clicked = a trigger type effect caused by 'just pressed' event or repeat while 'down'
//   Double clicked = 2 'just pressed' events occuring within a short amount of time

// Button flags
#define BF_UP							(1<<0)
#define BF_DOWN						(1<<1)
#define BF_JUST_PRESSED				(1<<2)
#define BF_JUST_RELEASED			(1<<3)
#define BF_CLICKED					(1<<4)
#define BF_DOUBLE_CLICKED			(1<<5)
#define BF_HIGHLIGHTED				(1<<6)  // button is not highlighted (ie mouse is not over)
#define BF_JUST_HIGHLIGHTED		(1<<7)  // button has just been highlighted, true for 1 frame
#define BF_IGNORE_FOCUS				(1<<8)  // button should not use focus to accept space/enter keypresses
#define BF_HOTKEY_JUST_PRESSED	(1<<9)  // button hotkey was just pressed
#define BF_REPEATS					(1<<10) // if held down, generates repeating presses
#define BF_SKIP_FIRST_HIGHLIGHT_CALLBACK (1<<11)	// skip first callback for mouse over event

class UI_BUTTON : public UI_GADGET
{
	friend UI_SCROLLBAR;
	// friend UI_SLIDER;
	friend UI_DOT_SLIDER;
	friend UI_DOT_SLIDER_NEW;

		char *text;
		int position;			// indicates position of button (0 - up, 1 - down by mouse click 2 - down by keypress
		int next_repeat;		// timestamp for next repeat if held down
		int m_press_linger;	// timestamp for hold a pressed state animation
		int hotkey_if_focus;	// hotkey for button that only works if it has focus
		int force_draw_frame;	// frame number to draw next time (override default)
		int first_callback;		// true until first time callback function is called for button highlight

		// Used to index into bmap_ids[] array to locate right bitmap for button
		enum { B_NORMAL = 0 };
		enum { B_HIGHLIGHT = 1 };
		enum { B_PRESSED = 2 };
		enum { B_DISABLED = 3 };
		enum { B_REPEAT_TIME = 100 };  // ms
			
		void (*m_just_highlighted_function)(void);	// call-back that gets called when button gets highlighted
		void (*m_disabled_function)(void);				// callback that gets called when disabled button gets pressed (sound, popup, etc)

		void frame_reset();
		virtual void process(int focus = 0);
		virtual void destroy();

		int custom_cursor_bmap;					// bmap handle of special cursor used on mouseovers
		int previous_cursor_bmap;				// store old cursor
		void maybe_show_custom_cursor();		// call this in process() 
		void restore_previous_cursor();		// called in frame_reset()

	public:
		virtual void draw();
		void set_hotkey_if_focus(int key);
		int pressed();				// has it been selected (ie clicked on)
		int double_clicked();	// button was double clicked on
		int just_pressed();		// button has just been selected
		int just_highlighted();	// button has just had mouse go over it
		int button_down();		// is the button depressed?
		int button_hilighted();	// is the mouse over this button?
		void set_button_hilighted();	// force button to be highlighted
		void press_button();		// force button to get pressed
		void create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _w, int _h, int do_repeat=0, int ignore_focus = 0);
		void set_highlight_action( void (*user_function)(void) );
		void set_disabled_action( void (*user_function)(void) );
		void draw_forced(int frame_num);
		void reset_status();
		void reset_timestamps();
		void skip_first_highlight_callback();
		void repeatable(int yes);
		void set_custom_cursor_bmap(int bmap_id) { custom_cursor_bmap = bmap_id; };
};

class UI_KEYTRAP : public UI_GADGET
{
		int pressed_down;
		virtual void draw();
		virtual void process(int focus = 0);

	public:
		int pressed();
		void create(UI_WINDOW *wnd, int hotkey, void (*user_function)(void) );
};

class UI_USERBOX : public UI_GADGET
{
		int b1_held_down;
		int b1_clicked;
		int b1_double_clicked;
		int b1_dragging;
		int b1_drag_x1, b1_drag_y1;
		int b1_drag_x2, b1_drag_y2;
		int b1_done_dragging;
		int keypress;
		int mouse_onme;
		int mouse_x, mouse_y;
		int bitmap_number;
};

class UI_INPUTBOX : public UI_GADGET
{
		char *text;
		char *passwd_text;
		int length;
		int position;
		int oldposition;
		int pressed_down;
		int changed_flag;
		int flags;
		int pixel_limit;    // base max characters on how wide the string is (-1 to ignore) in pixels
		int locked;
//		int should_reset;
		int ignore_escape;
		color *text_color;
		char *valid_chars;
		char *invalid_chars;

		// cursor drawing
		int cursor_first_frame;
		int cursor_nframes;
		int cursor_fps;
		int cursor_current_frame;
		int cursor_elapsed_time;

		int	validate_input(int chr);
		void	init_cursor();

		virtual void draw();
		virtual void process(int focus = 0);
		virtual void destroy();

	public:
//		int first_time;

		void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _textlen, char *text, int _flags = 0, int pixel_lim = -1, color *clr = NULL);
		void set_valid_chars(char *vchars);
		void set_invalid_chars(char *ichars);
		int changed();
		int pressed();
		void get_text(char *out);
		void set_text(char *in);
};

// Icon flags
#define	ICON_NOT_HIGHLIGHTED		(1<<0)	// icon is not highlighted (ie mouse is not over)
#define	ICON_JUST_HIGHLIGHTED	(1<<1)	// icon has just been highlighted, true for 1 frame

class UI_ICON : public UI_GADGET
{
		char *text;

		// Used to index into bmap_ids[] array to locate right bitmap for button
		enum { ICON_NORMAL = 0 };
		enum { ICON_HIGHLIGHT = 1 };
		enum { ICON_SELECTED = 2 };
		enum { ICON_DISABLED = 3 };
			
		virtual void draw();
		virtual void process(int focus = 0);
		virtual void destroy();

	public:
		void create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _w, int _h);
};

class UI_CHECKBOX : public UI_GADGET
{
		char *text;
		int position;
		int pressed_down;
		int flag;
		virtual void draw();
		virtual void process(int focus = 0);
		virtual void destroy();

		// Used to index into bmap_ids[] array to locate right bitmap for checkbox
		enum { CBOX_UP_CLEAR = 0 };
		enum { CBOX_DOWN_CLEAR = 1 };
		enum { CBOX_UP_MARKED = 2 };
		enum { CBOX_DOWN_MARKED = 3 };
		enum { CBOX_DISABLED_CLEAR = 4 };
		enum { CBOX_DISABLED_MARKED = 5 };

	public:
		int changed();
		int checked();
		void create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _state );
		void set_state(int _state);
};

class UI_RADIO : public UI_GADGET
{
		char *text;
		int position;
		int pressed_down;
		int flag;
		int group;
		virtual void draw();
		virtual void process(int focus = 0);
		virtual void destroy();

		// Used to index into bmap_ids[] array to locate right bitmap for radio button
		enum { RADIO_UP_CLEAR = 0 };
		enum { RADIO_DOWN_CLEAR = 1 };
		enum { RADIO_UP_MARKED = 2 };
		enum { RADIO_DOWN_MARKED = 3 };
		enum { RADIO_DISABLED_CLEAR = 4 };
		enum { RADIO_DISABLED_MARKED = 5 };

	public:
		int changed();
		int checked();
		void create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _state, int _group );
};

class UI_SCROLLBAR : public UI_GADGET
{
	friend UI_LISTBOX;
	friend UI_BUTTON;
		int horz;
		int start;
		int stop;
		int position;
		int window_size;
		int bar_length;
		int bar_position;
		int bar_size;
		UI_BUTTON up_button;
		UI_BUTTON down_button;
		int last_scrolled;
		int drag_x, drag_y;
		int drag_starting;
		int dragging;
		int moved;

		virtual void draw();
		virtual void process(int focus = 0);

		// Used to index into bmap_ids[] array to locate right bitmap for scrollbar
		enum { SB_NORMAL = 0 };
		enum { SB_DISABLED = 1 };

	public:
		void create(UI_WINDOW *wnd, int _x, int _y, int _h,int _start, int _stop, int _position, int _window_size  );
		int getpos();
		int changed();
		void hide();
		void unhide();
		int get_hidden();
		void link_hotspot(int up_button_num, int down_button_num);
		int set_bmaps(char *up_button_fname, char *down_button_fname, char *line_fname);
};

class UI_SLIDER2 : public UI_GADGET
{
	friend UI_BUTTON;
	private:
		int numberItems;				// total range
		int numberPositions;			// total positions (height - bitmapbuttonheight)
		int currentItem;				// current item we are on
		int currentPosition;			// current position
		int last_scrolled;
		int mouse_locked;
		int slider_mode;				// 		
		UI_BUTTON sliderBackground;// invisible button to detect clicks
		int bitmapSliderControl;	// this is the bitmap of the slider button itself
		void (*upCallback)();
		void (*downCallback)();
		void (*captureCallback)();	// this is called when the mouse is released
		UI_BUTTON *upButton, *downButton;
		int slider_w, slider_h, slider_half_h;		// this is the width and height and half height of the bitmap used for the slider
		virtual void draw();
		virtual void process(int focus = 0);

		// Used to index into bmap_ids[] array to locate right bitmap for slider
		enum { S2_NORMAL = 0 };
		enum { S2_HIGHLIGHT = 1 };
		enum { S2_PRESSED = 2 };
		enum { S2_DISABLED = 3 };

		// Used for slider mode
		enum { S2M_ON_ME = 0 };
		enum { S2M_MOVING = 1 };
		enum { S2M_DEFAULT = 2 };

	public:
		// create the slider
		void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, int _numberItems, char *_bitmapSliderControl,
						void (*_upCallback)(), void (*_downCallback)(), void (*_captureCallback)());
		
		// range management
		int get_numberItems();		// return number of itmes
		int get_currentPosition();	// return current position
		int get_currentItem();		// return current item
		void set_numberItems(int _numberItems, int reset = 1);		// change range. maybe reset back to position 0
		void set_currentItem(int _currentItem);		// force slider to new position manually
		void force_currentItem(int _currentItem);		// force slider to new position manually, _not_ calling any callbacks

		// force down
		void forceDown();

		// force up
		void forceUp();

		// general ui commands
		void hide();
		void unhide();
		int get_hidden();
};

// to be phased out eventually in FS2
class UI_DOT_SLIDER : public UI_GADGET
{
	friend UI_BUTTON;

		UI_BUTTON button;
		UI_BUTTON up_button;
		UI_BUTTON down_button;
		int first_frame, total_frames;
		int has_end_buttons;
		int num_pos;

	public:
		int pos;  // 0 thru 10

		void create(UI_WINDOW *wnd, int _x, int _y, char *bm, int id, int end_buttons = 1, int num_pos = 10);		
		virtual void draw();
		virtual void process(int focus = 0);
		virtual void destroy();
};

class UI_DOT_SLIDER_NEW : public UI_GADGET
{
	friend UI_BUTTON;
		UI_BUTTON button;
		UI_BUTTON up_button;
		UI_BUTTON down_button;		
		int has_end_buttons;
		int num_pos;
		int dot_width;

	public:
		int pos;  // 0 thru 10

		void create(UI_WINDOW *wnd, int _x, int _y, int num_pos, char *bm_slider, int slider_mask,
																					char *bm_left = NULL, int left_mask = -1, int left_x = -1, int left_y = -1,
																					char *bm_right = NULL, int right_mask = -1, int right_x = -1, int right_y = -1,
																					int dot_width = 19);
		virtual void draw();
		virtual void process(int focus = 0);		
};

class UI_LISTBOX : public UI_GADGET
{
	private:
		char **list;
		char *check_list;
		int max_items;
		int num_items;
		int num_items_displayed;
		int first_item;
		int old_first_item;
		int current_item;
		int selected_item;
		int toggled_item;
		int old_current_item;
		int last_scrolled;
		int dragging;
		int textheight;
		int has_scrollbar;
		char key_buffer[MAX_KEY_BUFFER];
		int key_buffer_count;
		int last_typed;
		UI_SCROLLBAR scrollbar;

		virtual void draw();
		virtual void process(int focus = 0);

		// Used to index into bmap_ids[] array to locate right bitmap for listbox
		enum { LBOX_NORMAL = 0 };
		enum { LBOX_DISABLED = 1 };

	public:
		void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, int _numitem, char **_list, char *_check_list = NULL, int _max_items = -1);
		int selected();	// selected: Returns >= 0 if an item was selected
		int current();	// current:  Returns which item listbox bar is currently on. This could be -1, if none selected!		
		int toggled();	// toggled:  Returns which item was toggled with spacebr or mouse click of check_list is not NULL		
		void set_current(int _index);	// sets the current item in the list box
		void set_first_item(int _index);
		char *get_string(int _index);
		void clear_all_items();       // deletes all the items in the list (makes them empty strings)
		int add_string(char *str);
		int sel_changed();           // returns > 0 if the selected item has changed
		void set_new_list(int _numitems, char **_list);

		int set_bmaps(char *lbox_fname, char *b_up_fname, char *b_down_fname, char *sb_fname);
		void link_hotspot(int up_button_num, int down_button_num);
};

#define WIN_BORDER 1
#define WIN_FILLED 2
#define WIN_SAVE_BG 4
#define WIN_DIALOG (4+2+1)

class UI_WINDOW
{
	friend UI_GADGET;
	friend UI_BUTTON;
	friend UI_KEYTRAP;
	friend UI_CHECKBOX;
	friend UI_RADIO;
	friend UI_SCROLLBAR;
	friend UI_LISTBOX;
	friend UI_INPUTBOX;
	// friend UI_SLIDER;
	friend UI_SLIDER2;
	friend UI_DOT_SLIDER;
	friend UI_DOT_SLIDER_NEW;
	friend UI_ICON;

protected:
	int flags;
	int x, y;
	int w, h;
	int f_id;								// font id
	int last_tooltip_hotspot;
	uint last_tooltip_time;
	int tt_group;  // which tooltip group this window uses, or -1 if none
	int ignore_gadgets;

	UI_GADGET *first_gadget;
	UI_GADGET *selected_gadget;
	UI_GADGET *mouse_captured_gadget;

	int mask_bmap_id;						// bitmap id of the mask bitmap to define hotspots
	int foreground_bmap_id;				// bitmap id of the foreground bitmap to display
	bitmap *mask_bmap_ptr;				// pointer to bitmap of the mask
	ushort *mask_data;					// points to raw mask bitmap data
	int mask_w, mask_h;					// width and height of the mask

	UI_XSTR	*xstrs[MAX_UI_XSTRS];	// strings for drawing in code instead of in artwork


	int keypress;		// filled in each frame
	void capture_mouse(UI_GADGET *gadget = NULL);
	void release_bitmaps();		// called internally when window destroys gadgets
	void check_focus_switch_keys();
	void do_dump_check();
	void draw_xstrs();			// draw xstrs
	void draw_one_xstr(UI_XSTR *xstr, int frame);

public:
	UI_WINDOW();	// constructor
	~UI_WINDOW();	// destructor
	void set_mask_bmap(char *fname);
	void set_mask_bmap(int bmap, char *name);
	void set_foreground_bmap(char *fname);
	void create( int x, int y, int w, int h, int flags );
	int process( int key_in = -1,int process_mouse = 1);
	void draw();
	void draw_tooltip();
	void draw_XSTR_forced(UI_GADGET *owner, int frame);
	int get_current_hotspot();
	void destroy();
	ushort *get_mask_data(int *w, int *h) { *w = mask_w; *h = mask_h; return mask_data; }
	void render_tooltip(char *str);
	void set_ignore_gadgets(int state);
	void add_XSTR(char *string, int xstr_id, int x, int y, UI_GADGET *assoc, int color_type, int font_id = -1);
	void add_XSTR(UI_XSTR *xstr);

	char *(*tooltip_handler)(char *text);
	int last_keypress;		// filled in each frame
	int ttx, tty;
	int use_hack_to_get_around_stupid_problem_flag;
};

// 2 extremely useful structs
typedef struct ui_button_info {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	ui_button_info(char *name, int x1, int y1, int xt1, int yt1, int h) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h) {}
} ui_button_info;


/*
typedef struct {
	char *mask;
	int start;
	int end;
} tooltip_group;

typedef struct {
	int hotspot;
	char *text;
} tooltip;

#define MAX_TOOLTIP_GROUPS	50
#define MAX_TOOLTIPS			500

extern int Num_tooltip_groups;
extern tooltip_group Tooltip_groups[MAX_TOOLTIP_GROUPS];
extern tooltip Tooltips[MAX_TOOLTIPS];
*/

int ui_getfilelist( int MaxNum, char **list, char *filespec );
void ui_sort_filenames( int n, char **list );

/*
class UI_SLIDER : public UI_GADGET
{
	friend UI_BUTTON;
		int horz;
		int position;
		int window_size;
		int fake_length;
		int fake_position;
		int fake_size;
		UI_BUTTON left_button;
		UI_BUTTON right_button;
		int last_scrolled;
		int drag_x, drag_y;
		int drag_starting;
		int dragging;
		int moved;

		int marker_x, marker_y, marker_w, marker_h;
		int n_positions, pixel_range, increment;
		float start, stop, current;
		int mouse_locked;

		virtual void draw();
		virtual void process(int focus = 0);

		// Used to index into bmap_ids[] array to locate right bitmap for slider
		enum { SLIDER_BAR_NORMAL = 0 };
		enum { SLIDER_BAR_DISABLED = 1 };
		enum { SLIDER_MARKER_NORMAL = 2 };
		enum { SLIDER_MARKER_DISABLED = 3 };

	public:
		void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, float _start, float _stop, float _pos, int n_positions);
		int getpos();
		float getcurrent();
		int changed();
		void hide();
		void unhide();
		int get_hidden();
		void link_hotspot(int up_button_num, int down_button_num);
		int set_bmaps(char *left_button_fname, char *right_button_fname, char *bar_fname, char *marker_fname);
};
*/

#endif
