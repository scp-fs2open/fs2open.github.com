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
void outwnd_init();
void outwnd_close();
void outwnd_printf(const char *id, const char *format, ...);
void outwnd_printf2(const char *format, ...);

extern int Log_debug_output_to_file;

#endif	// NDEBUG

#endif	// _OUTWND_H
