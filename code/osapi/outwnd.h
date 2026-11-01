/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _OUTWND_H
#define _OUTWND_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

void load_filter_info();
void outwnd_init();
void outwnd_close();
void outwnd_printf(const char *id, SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(2, 3);
void outwnd_printf2(SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(1, 2);

void outwnd_debug_window_init();
void outwnd_debug_window_do_frame(float frametime);
void outwnd_debug_window_deinit();

extern bool Log_debug_output_to_file;

struct outwnd_filter_struct {
	char name[NAME_LENGTH];
	bool enabled;
};

// Returns the known log filter categories. New categories are added to this list
// the first time they are used by an nprintf()/mprintf() call.
const SCP_vector<outwnd_filter_struct>& outwnd_get_filters();
void outwnd_set_filter_enabled(const char* name, bool enabled);

// A single line that was written to the log, kept around so it can be displayed in a UI.
struct OutwndLogEntry {
	SCP_string category;
	SCP_string text;
};

const SCP_deque<OutwndLogEntry>& outwnd_get_log_history();
void outwnd_clear_log_history();

#endif	// _OUTWND_H
