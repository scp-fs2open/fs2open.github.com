/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _UI_H
#define _UI_H

#include "graphics/2d.h"
#include "graphics/font.h"

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
class UI_SLIDER2;
class UI_DOT_SLIDER;
class UI_DOT_SLIDER_NEW;

class UI_GADGET
{
	friend class UI_WINDOW;
	friend class UI_BUTTON;
	friend class UI_KEYTRAP;
	friend class UI_CHECKBOX;
	friend class UI_RADIO;
	friend class UI_SCROLLBAR;
	friend class UI_LISTBOX;
	friend class UI_INPUTBOX;
	friend class UI_SLIDER2;
	friend class UI_DOT_SLIDER;	
	friend class UI_DOT_SLIDER_NEW;

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
		virtual ~UI_GADGET();	// destructor

		void base_create( UI_WINDOW *wnd, int kind, int x, int y, int w, int h );
		virtual void draw();
		void set_focus();
		void clear_focus();
		int has_focus();
		void set_hotkey(int keycode);
		void set_callback(void (*_user_function)(void));
		void disable();
		void enable(int n = 1);
		void capture_mouse();
		int mouse_captured(UI_GADGET *gadget = NULL);
		int disabled();
		int enabled();
		virtual void hide(int n);
		virtual void hide();
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
		virtual int is_hidden();
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
	friend class UI_SCROLLBAR;
	friend class UI_SLIDER2;
	friend class UI_DOT_SLIDER;
	friend class UI_DOT_SLIDER_NEW;

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
		void set_highlight_action( void (*_user_function)(void) );
		void set_disabled_action( void (*_user_function)(void) );
		void draw_forced(int frame_num);
		void reset_status();
		void reset_timestamps();
		void skip_first_highlight_callback();
		void repeatable(int yes);
		void set_custom_cursor_bmap(int bmap_id) { custom_cursor_bmap = bmap_id; }
};

class UI_KEYTRAP : public UI_GADGET
{
		int pressed_down;
		virtual void draw();
		virtual void process(int focus = 0);

	public:
		int pressed();
		void create(UI_WINDOW *wnd, int hotkey, void (*_user_function)(void) );
};

/** TODO
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
 */

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
		void create(UI_WINDOW *wnd, int _x, int _y, int _w, int _textlen, char *text, int _flags = 0, int pixel_lim = -1, color *clr = NULL);
		void set_valid_chars(char *vchars);
		void set_invalid_chars(char *ichars);
		int changed();
		int pressed();
		void get_text(char *out);
		void set_text(const char *in);
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
	friend class UI_LISTBOX;
	friend class UI_BUTTON;
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
		void link_hotspot(int up_button_num, int down_button_num);
		int set_bmaps(char *up_button_fname, char *down_button_fname, char *line_fname);
};

class UI_SLIDER2 : public UI_GADGET
{
	friend class UI_BUTTON;
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
		void set_numberItems(int _numberItems, int _reset = 1);		// change range. maybe reset back to position 0
		void set_currentItem(int _currentItem);		// force slider to new position manually
		void force_currentItem(int _currentItem);		// force slider to new position manually, _not_ calling any callbacks

		// force down
		void forceDown();

		// force up
		void forceUp();
};

// to be phased out eventually in FS2
class UI_DOT_SLIDER : public UI_GADGET
{
	friend class UI_BUTTON;

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
	friend class UI_BUTTON;
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

		// kazan
		int draw_frame;

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

		// kazan
		void set_drawframe(int mode);
		int CurSize();
		int MaxSize();
		void RemoveFirstItem();
		void ScrollEnd();

		int set_bmaps(char *lbox_fname, char *b_up_fname, char *b_down_fname, char *sb_fname);
		void link_hotspot(int up_button_num, int down_button_num);
};

#define WIN_BORDER 1
#define WIN_FILLED 2
#define WIN_SAVE_BG 4
#define WIN_DIALOG (4+2+1)

class UI_WINDOW
{
	friend class UI_GADGET;
	friend class UI_BUTTON;
	friend class UI_KEYTRAP;
	friend class UI_CHECKBOX;
	friend class UI_RADIO;
	friend class UI_SCROLLBAR;
	friend class UI_LISTBOX;
	friend class UI_INPUTBOX;
	friend class UI_SLIDER2;
	friend class UI_DOT_SLIDER;
	friend class UI_DOT_SLIDER_NEW;
	friend class UI_ICON;

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
	ubyte *mask_data;					// points to raw mask bitmap data
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
	void create( int x, int y, int w, int h, int flags, int f_id = -1 );
	int process( int key_in = -1,int process_mouse = 1);
	void draw();
	void draw_tooltip();
	void draw_XSTR_forced(UI_GADGET *owner, int frame);
	int get_current_hotspot();
	void destroy();
	ubyte *get_mask_data(int *w_md, int *h_md) { *w_md = mask_w; *h_md = mask_h; return mask_data; }
	void render_tooltip(char *str);
	void set_ignore_gadgets(int state);
	void add_XSTR(char *string, int xstr_id, int x, int y, UI_GADGET *assoc, int color_type, int font_id = -1);
	void add_XSTR(UI_XSTR *xstr);

	const char *(*tooltip_handler)(const char *text);
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


int ui_getfilelist( int MaxNum, char **list, char *filespec );
void ui_sort_filenames( int n, char **list );

#endif
