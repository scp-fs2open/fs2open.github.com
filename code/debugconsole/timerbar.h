/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 



#ifndef _TIMERBAR_HEADER_
#define _TIMERBAR_HEADER_

const int MAX_NUM_TIMERBARS = 20;

#include "cmdline/cmdline.h"

// These functions should never be used directly, always use macros below
void timerbar_start_frame();
void timerbar_end_frame();
void timerbar_set_draw_func(void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h));

void timerbar_push(int value);
void timerbar_pop();

// This function shouldnt not be used any more or it will break push and pop calls
void timerbar_switch_type(int num);

#endif

