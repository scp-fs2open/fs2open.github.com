/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "debugconsole/console.h"
#include "debugconsole/consoleparse.h"
#include "globalincs/alphacolors.h"
#include "globalincs/pstypes.h"
#include "globalincs/version.h"
#include "globalincs/vmallocator.h"
#include "graphics/2d.h"
#include "graphics/font.h"
#include "io/timer.h"
#include "io/key.h"
#include "osapi/osapi.h"

#include <algorithm>
#include <cmath>

// ========================= GLOBALS =========================
bool Dc_debug_on;		//!< Flag used to print console and command debugging strings

// Commands and History
SCP_string dc_command_str;		//!< The entered command line, arguments and all.
								//!< Is progressively culled from the left as commands, arguments are parsed in DCF's

// Misc
bool debug_inited = FALSE;
uint lastline = 0;  // Number of lines written to the console since the last command was processed


// ========================= LOCALS ==========================
// Text Buffer
uint DBROWS = 80;   // # of buffer rows
uint DBCOLS = 80;   // # of buffer columns
uint lastwhite = 0; // Last whitespace character encountered, used by putc for 'true' word wrapping
ubyte DTABS = 4;    //!< Tab size in spaces

/**
 * Human readable versions of the dc_token's. Primarily used in error diagnosis
 */
static
const char *token_str[DCT_MAX_ITEMS] =
{
	"nothing",
	"string",
	"float",
	"integer",
	"unsigned integer",
	"byte",
	"unsigned byte",
	"boolean"
};

SCP_deque<SCP_string> dc_buffer;

// Display Window
uint DROWS = 25;
uint DCOLS = 80;
const uint DROWS_MIN = 25;
const uint DCOLS_MIN = 80;
uint dc_scroll_x;   // X scroll position (Leftmost character)
uint dc_scroll_y;   // Y scroll position (Topmost character)
int row_height;     // Row/Line height, in pixels
int col_width;      // Col/Character width, in pixels
int dc_font = FONT1;

SCP_string dc_title;

#define SCROLL_X_MAX (DBCOLS - DCOLS)
#define SCROLL_Y_MAX (DBROWS - DROWS)

// Commands and History
uint DCMDS = 40;			// Max number of commands to remember
const uint DCMDS_MIN = 3;

SCP_deque<SCP_string> dc_history;
SCP_deque<SCP_string>::iterator last_oldcommand;		// Iterator to the last old command. Is reset to the start every new command push.

const char dc_prompt[]= "> ";	// The prompt c_str
SCP_string dc_command_buf;		// The command line as shown in the console. Essentially an input buffer for dc_command_str

// Local functions
/**
 * @brief Initializes the debug console.
 */
void dc_init(void);

/**
 * @brief Process the entered command string
 */
void dc_do_command(SCP_string *cmd_str);

/**
 * @brief Draws the in-game console.
 */
void dc_draw(bool show_prompt);

/**
 * Draws the cursor
 * @param [in] cmd_string	The formatted command string displayed by dc_draw_window
 * @param [in] x, y			The x and y screen position of the command string
 */
void dc_draw_cursor( SCP_string &cmd_string, int x, int y );

/**
 * Draws the window text
 */
void dc_draw_window(bool show_prompt);

/**
 * @brief   Stuffs the given character into the output buffer.
 * @details Also handles tab alignment, newlines, and maintains the target.
 */
void dc_putc(char c);

/**
 * @brief Predicate function used to sort debug_commands
 */
bool dcmd_less(debug_command *first, debug_command *second);

// ============================== IMPLEMENTATIONS =============================
void dc_do_command(SCP_string *cmd_str)
{
	/**
	 * Grab the first word from the cmd_str
	 *  If it is not a literal, ignore it "Invalid keyword: %s"
	 *  Search for the command...
	 *      Compare the word against valid commands
	 *      If command not found, ignore it "Invalid or unknown command: %s"\
	 *  Process the command...
	 *      Call the function to process the command (the rest of the command line is in the parser)
	 *          Function takes care of long_help and status depending on the mode.
	 */
	int i;
	SCP_string command;
	extern debug_command* dc_commands[];	// z64: I don't like this extern here, at all. Nope nope nope.

	if (cmd_str->empty()) {
		return;
	}

	dc_parse_init(*cmd_str);

	dc_stuff_string_white(command);		// Grab the first token, presumably this is a command

	for (i = 0; i < dc_commands_size; ++i) {

		if (stricmp(dc_commands[i]->name, command.c_str()) == 0) {
			break;
		} // Else, continue
	}

	if (i == dc_commands_size) {
		dc_printf("Command not found: '%s'\n", command.c_str());
		return;
	} // Else, we found our command

	try {
		dc_commands[i]->func();	// Run the command!
	
	} catch (errParseString err) {
		dc_printf("Require string(s) not found: \n");
		for (uint i = 0; i < err.expected_tokens.size(); ++i) {
			dc_printf("%i: %s\n", err.expected_tokens[i].c_str());
		}

		dc_printf("Found '%s' instead\n", err.found_token.c_str());
	
	} catch (errParse err) {
		dc_printf("Invalid argument. Expected %s, found '%s'\n", token_str[err.expected_type], err.found_token.c_str());

	}

	// dc_maybe_stuff_string is vulnerable to overflow. Once the errParseOverflow throw class (or w/e) gets
	// implemented, this last command should be put into its own try{} catch{} block.
	if (dc_maybe_stuff_string(command)) {
		dc_printf( "Ignoring the unused command line tail '%s'\n", command.c_str() );
	}
}

void dc_draw(bool show_prompt = FALSE)
{
	gr_clear();
	gr_set_font(dc_font);
	gr_set_color_fast( &Color_bright );
	gr_string( 0x8000, 3, dc_title.c_str(), GR_RESIZE_NONE );

	gr_set_color_fast( &Color_normal );

	dc_draw_window(show_prompt);

	gr_flip();
}

void dc_draw_cursor( SCP_string &cmd_string, int x, int y )
{
	int t;
	int w, h;	// gr_string width and height

	t = timer_get_fixed_seconds() / (F1_0/3);
	if ( t & 1 ) {
		gr_get_string_size( &w, &h, cmd_string.c_str() );

		w %= (DCOLS * Current_font->w);
		//gr_string( w, debug_y*16, "_" );
		gr_rect((x + (w + 1)), (y + (h + 1)), 2, Current_font->h, GR_RESIZE_NONE);
	}
}

void dc_draw_window(bool show_prompt)
{
	uint cmd_lines;                 // Number of lines for the command string
	uint buffer_lines;              // Number of lines from the buffer to draw
	uint i;                         // The current row we're drawing
	uint j;                         // The current row of the command string we're drawing
	SCP_string out_str;             // The command string + prompt character
	SCP_string::iterator str_it;    // Iterator to out_str

	out_str = dc_prompt + dc_command_buf;
	cmd_lines = (out_str.size() / DCOLS) + 1;
	if (show_prompt) {
		buffer_lines = DROWS - cmd_lines;
	} else {
		buffer_lines = DROWS;
	}

	// Ensure the window is not bigger than the buffer
	CLAMP(DROWS, DROWS_MIN, DBROWS);
	CLAMP(DCOLS, DCOLS_MIN, DBCOLS);

	// Ensure we don't scroll too far
	dc_scroll_x = MIN(dc_scroll_x, (DBCOLS - DCOLS));
	if (dc_buffer.size() >= buffer_lines) {
		dc_scroll_y = MIN(dc_scroll_y, (dc_buffer.size() - buffer_lines));
	} else {
		dc_scroll_y = 0;	// Disallow vscroll until the buffer is larger than the window
	}

	// Draw the buffer strings
	for (i = 0; i < buffer_lines; ++i) {
		if ((i + dc_scroll_y) < dc_buffer.size()) {
			gr_string(0, ((i * row_height) + row_height), dc_buffer[i + dc_scroll_y].substr(dc_scroll_x).c_str(), GR_RESIZE_NONE);
		}
	}

	// Draw the command string w/ padding only if the prompt is active.
	if (show_prompt) {
		i += 1;		// 1 line between the output and the input text
		j = 0;
		gr_set_color_fast(&Color_bright);
		for (str_it = out_str.begin(); str_it < out_str.end(); ++str_it) {
			if (j == (DCOLS - 1)) {
				// Insert a newline char at every place the string needs to return the 'carriage'
				str_it = out_str.insert(str_it, '\n');
				j = 0;
			} else {
				++j;
			}
		}
		gr_string(0, ((i * row_height) + row_height), out_str.c_str(), GR_RESIZE_NONE);

		dc_draw_cursor(out_str, 0, ((i * row_height)));
		gr_set_color_fast(&Color_normal);
	}
}

void dc_init(void)
{
	if (debug_inited) {
		return;
	}

	debug_inited = TRUE;

	// Init window settings
	dc_font = FONT1;
	row_height = ((Current_font->h) * 3) / 2;	// Row/Line height, in pixels
	col_width = Current_font->w;			// Col/Character width, in pixels
	dc_scroll_x = 0;
	dc_scroll_y = 0;
	DCOLS = (gr_screen.max_w / col_width) - 1;	// Subtract as needed. Windowed mode has some quirks with the resolution
	DROWS = (gr_screen.max_h / row_height) - 2;
	DBCOLS = DCOLS;
	DBROWS = 2 * DROWS;

	// Init History
	dc_history.clear();
	dc_history.push_back("");
	last_oldcommand = dc_history.begin();

	// Init buffers
	dc_buffer.clear();
	dc_buffer.push_back("");
	
	dc_command_buf.reserve(MAX_CLI_LEN);
	dc_command_buf.clear();

	sprintf(dc_title, "FreeSpace Open v%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
	dc_printf("Debug console started.\n" );
}

bool dc_pause_output(void)
{
	dc_printf("More to follow. Press any key to continue. ESC halts output...");

	int key;
	bool loop;
	do {
		loop = true;

		os_poll();

		dc_draw(FALSE);

		key = key_inkey();
		switch (key) {
		case KEY_ESC:
			return true;
			break;

		case KEY_PAGEUP:
			if (dc_scroll_y > 1) {
				dc_scroll_y--;
			}
			break;

		case KEY_PAGEDOWN:
			if (dc_scroll_y < DBROWS) {
				dc_scroll_y++;
			} else {
				dc_scroll_y = DBROWS;
			}
			break;

		case KEY_LEFT:
			// TODO: Scroll Left
			break;
		case KEY_RIGHT:
			// TODO: Scroll Right
			break;
		case 0:
			// No key pressed
			break;
		default:
			// Non-control key pressed, break.
			loop = false;
		}
	} while (loop);

	dc_printf("\n");
	return false;
};

void dc_printf(const char *format, ...)
{
	SCP_string tmp;
	va_list args;
	SCP_string::iterator tmp_it;

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	for (tmp_it = tmp.begin(); tmp_it != tmp.end(); ++tmp_it) {
		dc_putc(*tmp_it);
	}
}

void dc_putc(char c)
{
	SCP_string* line_str = &(dc_buffer.back());
	SCP_string temp_str;
	int i;
	int w;

	if (c == ' ') {
		/**
		 * Push c onto the temp_str and get its gr_string width
		 *
		 * If we run out of room on the line, or 
		 * If we run out of room on the screen, change c to a '\n' and let subsequent block handle it,
		 * Else, push the space onto the line and bail
		 */
		temp_str = *line_str;
		temp_str.push_back(c);
		gr_get_string_size(&w, NULL, temp_str.c_str());

		if ((temp_str.size() >= DBCOLS) || (w > gr_screen.max_w)) {
			c = '\n';
		
		} else {
			lastwhite = temp_str.size();
			*line_str = temp_str;
			return;
		}
	}

	if (c == '\t') {
		/**
		 * Calculate how many spaces to put in to align tabs,
		 * Append temp_str with the spaces and get its gr_string width
		 *
		 * If we run out of room on the line, or
		 * If we run out of room on the screen, change c to a '\n' and let subsequent block handle it,
		 * Else, copy temp_str onto the line, update the lastwhite index, and bail
		 */
		i = DTABS - (line_str->size() % DTABS);
		temp_str = *line_str;
		temp_str.append(i, ' ');
		gr_get_string_size(&w, NULL, temp_str.c_str());

		if ((temp_str.size() >= DBCOLS) || (w > gr_screen.max_w)) {
			c = '\n';

		} else {
			lastwhite = temp_str.size();
			*line_str = temp_str;
			return;
		}
	}

	if (c == '\n') {
		/**
		 * Trash whatever char happens to be past (DBCOLS - 1),
		 * Push a blank line onto the dc_buffer from the bottom,
		 * Increment the scroller, if needed,
		 * Trash the topmost line(s) in the buffer,
		 * Reset the lastwhite index,
		 * Increment the lastline counter, and finally
		 * bail
		 */
		if (line_str->size() > DBCOLS) {
			line_str->resize(DBCOLS);
		}
		dc_buffer.push_back("");

		if ((dc_buffer.size() > DROWS) && (dc_scroll_y < SCROLL_Y_MAX)) {
			dc_scroll_y++;
		}

		while (dc_buffer.size() > DBROWS) {
			dc_buffer.pop_front();
		}

		lastwhite = 0;
		lastline++;
		return;
	}

	// By this point, c is probably a writable character
	temp_str = *line_str;
	temp_str.push_back(c);
	gr_get_string_size(&w, NULL, temp_str.c_str());

	if ((temp_str.size() >= DBCOLS) || (w > gr_screen.max_w)) {
		/**
		 * Word wrapping
		 * Save the word, clear the line of the word, push new line with the word on it
		 * Update scroll_y, if needed,
		 * Pop off old lines, and finally
		 * Push new character onto the new line
		 */
		temp_str = line_str->substr(lastwhite);
		line_str->resize(lastwhite);
		dc_buffer.push_back(temp_str);
		line_str = &dc_buffer.back();

		if ((dc_buffer.size() > DROWS) && (dc_scroll_y < SCROLL_Y_MAX)) {
			dc_scroll_y++;
		}

		while (dc_buffer.size() > DBROWS) {
			dc_buffer.pop_front();
		}

		lastwhite = 0;
		lastline++;
		line_str->push_back(c);
		return;
	}

	// Else, just push the char onto the line
	line_str->push_back(c);
}

bool dcmd_less(debug_command *first, debug_command *second)
{
	return (strcmp(first->name, second->name) < 0);
}

void debug_console(void (*_func)(void))
{
	int done = 0;

	while( key_inkey() ) {
		os_poll();
	}

	if ( !debug_inited ) {
		dc_init();
	}

	dc_draw(TRUE);

	while (!done) {
		// poll the os
		os_poll();

		int k = key_inkey();
		switch( k ) {

		case KEY_SHIFTED+KEY_ENTER:
		case KEY_ESC:
			done = TRUE;
			break;

		case KEY_BACKSP:
			if (!dc_command_buf.empty()) {
				dc_command_buf.erase(dc_command_buf.size() - 1);
			}
			break;

		case KEY_F3:
		case KEY_UP:
			if (last_oldcommand < (dc_history.end() - 1)) {
				++last_oldcommand;
			}

			dc_command_buf = *last_oldcommand;
			break;

		case KEY_DOWN:
			if (last_oldcommand > dc_history.begin()) {
				--last_oldcommand;
			}

			dc_command_buf = *last_oldcommand;
			break;

		case KEY_PAGEUP:
			if (dc_scroll_y > 1) {
				dc_scroll_y--;
			}
			break;

		case KEY_PAGEDOWN:
			if (dc_scroll_y < (DBROWS - DROWS)) {
				dc_scroll_y++;
			} else {
				dc_scroll_y = (DBROWS - DROWS);
			}
			break;

		case KEY_ENTER:
			dc_scroll_y = (DBROWS - DROWS);			// Set the scroll to look at the bottom
			last_oldcommand = dc_history.begin();	// Reset the last oldcommand
			lastline = 0;	// Reset the line counter

			// Clear the command line on the window, but don't print the prompt until the command has processed
			// Stuff a copy of the command line onto the history
			// Search for the command
				// If not found:
				//   abort,
				//   dc_printf("Error: Invalid or Missing command %s", cmd.c_str()), and
				//   dc_printf(dc_prompt) when ready for input
			// Call the function for that command, and strip the cmd token from the command line string
			if (dc_command_buf.empty()) {
				dc_printf("No command given.\n");
				break;
			} // Else, continue to process the cmd_line

			// z64: Thread Note: Maybe lock a mutex here to allow a previous DCF to finish/abort before starting a new one
			// z64: We'll just assume we won't be here unless a command has finished...
			dc_history.push_front(dc_command_buf);	// Push the command onto the history queue
			last_oldcommand = dc_history.begin();	// Reset oldcommand

			while (dc_history.size() > DCMDS) {
				dc_history.pop_back();			// Keep the commands less than or equal to DCMDS
			}

			dc_command_str = dc_command_buf;	// Xfer to the command string for processing
			dc_command_buf.resize(0);			// Nullify the buffer
			dc_printf("%s%s\n", dc_prompt, dc_command_str.c_str());	// Print the command w/ prompt.
			dc_draw(FALSE);					// Redraw the console without the command line.
			dc_do_command(&dc_command_str);	// Try to do the command
			break;

		default:
			// Not any of the control key codes, so it's probably a letter or number.
			ubyte c = (ubyte)key_to_ascii(k);
			if ((c != 255) && (dc_command_buf.size() < MAX_CLI_LEN)) {
				dc_command_buf.push_back(c);
			}
		}

		// Do the passed function
		if ( _func ) {
			_func();
		}

		// All done, and ready for new entry
		dc_draw(TRUE);
	}

	while( key_inkey() ) {
		os_poll();
	}
}
