#pragma once

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

namespace osapi {

/**
 * @brief A debug window
 *
 * A debug window is a separate os::Viewport which displays additional debug information at runtime.
 */
class DebugWindow {
	/**
	 * A single line in the debug log can have multiple properties which are stored in this structure
	 */
	struct LineInfo {
		SCP_string category;
		SCP_string text;
	};

	os::Viewport* debug_view = nullptr; //!< The viewport in which the debug output is shown
	SDL_Window* debug_sdl_window = nullptr; //!< The SDL window handle of the debug view

	SCP_vector<LineInfo> lines; //!< Finished lines

	SCP_string current_category; //!< The category of the current line
	SCP_string current_line; //!< Line that is currently being constructed, may be empty

	int max_category_width = 0; //!< The maximum width of any category string encountered thus far

	size_t log_view_offset = 0; //!< The offset into the @c lines array of the last message to display

	/**
	 * @brief Adds the specified line to the log
	 * @param line The line
	 */
	void addToLog(LineInfo&& line);

	/**
	 * @brief Prints a line of the log to the screen
	 * @param bottom_y The bottom position of where the text should be drawn
	 * @param line The line to draw
	 * @return The new bottom position for the next line
	 */
	float print_line(float bottom_y, const LineInfo& line);

	/**
	 * @brief Splits current_line into individual strings and adds them to the log
	 *
	 * Since debug output can contain multiple newlines in one string we need to split them before storing them in the vector
	 *
	 * @param category The category to use for these lines
	 */
	void split_current_and_add_to_log(const SCP_string& category);

	/**
	 * @brief Event handler for window events
	 *
	 * @details This handles the keyboard events for the debug window
	 *
	 * @param evt The keyboard event
	 * @return @c true if the event was handled, @c false otherwise
	 */
	bool debug_key_handler(const SDL_Event& evt);
 public:
	/**
	 * @brief Initializes the instance
	 */
	DebugWindow();

	~DebugWindow();

	/**
	 * @brief Renders the contents of the debug log
	 *
	 * This does all the viewport switching and page flipping automatically
	 *
	 * @param frametime The time the last frame took to process
	 */
	void doFrame(float frametime);

	/**
	 * @brief Adds a debug log line to the buffer
	 *
	 * @param category The category of the specified text
	 * @param text The text to print to the debug log
	 */
	void addDebugMessage(const char* category, const char* text);
};

}

