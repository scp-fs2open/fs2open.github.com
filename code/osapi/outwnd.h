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


#ifndef NDEBUG
void load_filter_info(void);
void outwnd_init(int display_under_freespace_window = 0);
void outwnd_close();
void outwnd_printf(const char *id = NULL, const char *format = NULL, ...);
void outwnd_printf2(const char *format = NULL, ...);

extern int Log_debug_output_to_file;

void safe_point(const char *file, int line, const char *format, ...);
#define SAFEPOINT(s) safe_point(__FILE__,__LINE__,(s))

#else

#define SAFEPOINT(s)

#endif	// NDEBUG

#endif	// _OUTWND_H
