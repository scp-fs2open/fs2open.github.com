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

#define FILTER_NAME_LENGTH 30

void load_filter_info(void);
void outwnd_init(int display_under_freespace_window=0);
void outwnd_close();
void outwnd_printf(char *id, char *format, ...);
void outwnd_printf2(char *format, ...);

#ifndef NDEBUG
	extern int Log_debug_output_to_file;
#endif

#endif
