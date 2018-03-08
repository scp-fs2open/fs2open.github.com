//
//

#include "DebugWindow.h"

#include "globalincs/alphacolors.h"

namespace {
uint32_t get_debug_display() {
	// If there are two or more monitors then we would like to display the window on the second monitor but it shouldn't
	// be shown on the same monitor that the game is running on
	// If there is only then then we'll just display it on that one
	auto numDisplays = SDL_GetNumVideoDisplays();

	if (numDisplays < 1) {
		// Some kind of error
		return 0;
	}

	if (numDisplays == 1) {
		// We only have one display
		return 0;
	}
	auto mainDisplay = os_config_read_uint("Video", "Display", 0);
	if (mainDisplay == 1) {
		// Game is on the second monitor => use the primary screen
		return 0;
	} else {
		// Use the secondary screen
		return 1;
	}
}
}

namespace osapi {

DebugWindow::DebugWindow() {
	os::ViewPortProperties attrs;
	uint32_t display = get_debug_display();
	attrs.display = display;

	SDL_Rect size;
	SDL_GetDisplayBounds(display, &size);
	// Make the window a bit smaller so that it doesn't take up the whole screen
	attrs.width = (uint32_t) (size.w / 1.3f);
	attrs.height = (uint32_t) (size.h / 1.3f);

	attrs.title = "FreeSpace Open - Debug Window";

	attrs.flags.set(os::ViewPortFlags::Resizeable); // Make this window resizeable

	auto debugView = gr_create_viewport(attrs);
	if (debugView) {
		debug_view = os::addViewport(std::move(debugView));
		debug_sdl_window = debug_view->toSDLWindow();

		if (debug_sdl_window != nullptr) {
			os::events::addEventListener(SDL_KEYUP,
										 os::events::DEFAULT_LISTENER_WEIGHT - 5,
										 [this](const SDL_Event& e) { return this->debug_key_handler(e); });
		}
	}
	if (debug_view->toSDLWindow() != nullptr && os::getSDLMainWindow() != nullptr) {
		SDL_RaiseWindow(os::getSDLMainWindow());
	}
}

DebugWindow::~DebugWindow() {
}

void DebugWindow::doFrame(float) {
	if (!debug_view) {
		// Failed to create debug window, nothing to do here
		return;
	}

	gr_use_viewport(debug_view);

	gr_clear();
	font::set_font(font::FONT1);

	if (max_category_width != 0) {
		// Draw a line to separate the category from the text
		auto x_pos = max_category_width + 14;
		gr_set_color_fast(&Color_grey);
		gr_line(x_pos, 0, x_pos, gr_screen.max_h, GR_RESIZE_NONE);
	}

	// Print the saved debug log lines from the bottom up
	float current_y = i2fl(gr_screen.max_h - 10);

	// If the current view offset is at the end of the list then we also need to display the line that is currently being constructed
	if (log_view_offset == lines.size() - 1) {
		if (!current_line.empty()) {
			LineInfo info;
			info.text = current_line;
			info.category = current_category;

			current_y = print_line(current_y, info);
		}
	}
	// Now print all the stored lines
	// Backwards iteration of the vector comes from here: http://stackoverflow.com/a/4206815
	for (auto i = log_view_offset + 1; i-- > 0;) {
		auto& line = lines[i];

		current_y = print_line(current_y, line);

		if (current_y < 0.f) {
			// End to iteration if the rendered string would be invisible
			break;
		}
	}

	// Don't run scripting here
	gr_flip(false);

	gr_use_viewport(os::getMainViewport());
}

float DebugWindow::print_line(float bottom_y, const LineInfo& line) {
	int category_width;
	gr_get_string_size(&category_width, nullptr, line.category.c_str());

	// Wrap the string to make sure everything can be read
	SCP_vector<const char*> split_lines;
	SCP_vector<int> line_lengths;

	// Substract 40 so that we can have margins on both sides
	// Make sure that the width doesn't go too low or else split_str will not be able to fit enough characters in one line
	auto max_w = std::max(40, gr_screen.max_w - max_category_width - 40);

	split_str(line.text.c_str(), max_w, line_lengths, split_lines);

	auto text_height = split_lines.size() * font::get_current_font()->getHeight();

	float y_pos = bottom_y - text_height;

	float cat_x_pos = max_category_width - category_width + 10.f;
	// Give each category a unique color. We do this by hashing the string and using that to construct the RGB values
	auto hash = std::hash<SCP_string>()(line.category);
	gr_set_color((int) (hash & 0xFF), (int) ((hash & 0xFF00) >> 8), (int) ((hash & 0xFF0000) >> 16));
	gr_string(cat_x_pos, y_pos, line.category.c_str(), GR_RESIZE_NONE);

	gr_set_color_fast(&Color_white);

	for (size_t i = 0; i < split_lines.size(); ++i) {
		gr_string(max_category_width + 18.f, y_pos, split_lines[i], GR_RESIZE_NONE, line_lengths[i]);

		y_pos += font::get_current_font()->getHeight();
	}

	return bottom_y - text_height;
}

bool DebugWindow::debug_key_handler(const SDL_Event& evt) {
	if (!os::events::isWindowEvent(evt, debug_sdl_window)) {
		// Event belongs to another window
		return false;
	}

	ptrdiff_t diff = 0;
	switch(evt.key.keysym.sym) {
		case SDLK_DOWN:
			diff = 1;
			break;
		case SDLK_UP:
			diff = -1;
			break;
		case SDLK_PAGEDOWN:
			diff = 60; // It's not actually a page but this should still work
			break;
		case SDLK_PAGEUP:
			diff = -60;
			break;
		case SDLK_END:
			log_view_offset = lines.size() - 1;
			return true;
		case SDLK_HOME:
			log_view_offset = 0;
			return true;
		default:
			return true;
	}

	auto prev = log_view_offset;

	log_view_offset += diff;

	if (diff < 0 && log_view_offset > prev) {
		// We had an overflow of the counter
		log_view_offset = 0;
	}
	log_view_offset = std::min(log_view_offset, lines.size() - 1);

	return true;
}

void DebugWindow::addDebugMessage(const char* category, const char* text) {
	if (!debug_view) {
		// Failed to create debug window, nothing to do here
		return;
	}

	if (category != current_category) {
		// There is a new category so we need to write the old string to our log

		// Find finished lines and add them to our vector
		split_current_and_add_to_log(category);

		if (!current_line.empty()) {
			// Write the last line to our log
			LineInfo info;
			info.text = current_line;
			info.category = category;
			addToLog(std::move(info));

			current_line.clear();
		}
	}

	current_category = category;
	current_line += text;

	split_current_and_add_to_log(category);
}
void DebugWindow::split_current_and_add_to_log(const SCP_string& category) {
	size_t pos;
	while ((pos = current_line.find('\n')) != SCP_string::npos) {
		LineInfo info;
		info.text = current_line.substr(0, pos);
		info.category = category;
		addToLog(std::move(info));

		// Since pos refers to a valid position in the string, the expression pos + 1 is always valid since substr
		// allows values up to str.size()
		current_line = current_line.substr(pos + 1);
	}
}
void DebugWindow::addToLog(LineInfo&& line) {
	int cat_width;
	gr_get_string_size(&cat_width, nullptr, line.category.c_str());

	max_category_width = std::max(cat_width, max_category_width);

	if (log_view_offset == lines.size() - 1) {
		// We are currently showing the last line so we need to follow new entries
		++log_view_offset;
	}

	lines.push_back(std::move(line));
}
}
