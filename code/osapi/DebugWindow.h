#pragma once

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

namespace osapi {

class DebugWindow {
	struct LineInfo {
		SCP_string category;
		SCP_string text;
	};

	os::Viewport* debug_view = nullptr;

	SCP_vector<LineInfo> lines; //!< Finished lines

	SCP_string current_category; //!< The category of the current line
	SCP_string current_line; //!< Line that is currently being constructed, may be empty

	int max_category_width = 0;

	void addToLog(LineInfo&& line);

	/**
	 * @brief Prints a line of the log to the screen
	 * @param bottom_y The bottom position of where the text should be drawn
	 * @param line The line to draw
	 * @return The new bottom position for the next line
	 */
	float print_line(float bottom_y, const LineInfo& line);

	void split_current_and_add_to_log(const SCP_string& category);
 public:
	DebugWindow();
	~DebugWindow();

	void doFrame(float frametime);

	void addDebugMessage(const char* category, const char* text);
};

}

