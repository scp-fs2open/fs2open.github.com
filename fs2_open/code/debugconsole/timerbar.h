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

#define TIMERBAR_SET_DRAW_FUNC(f) if (Cmdline_timerbar) timerbar_set_draw_func(f);
#define TIMERBAR_START_FRAME()    if (Cmdline_timerbar) timerbar_start_frame();
#define TIMERBAR_END_FRAME()      if (Cmdline_timerbar) timerbar_end_frame();
#define TIMERBAR_SWITCH_TYPE(n)   if (Cmdline_timerbar) timerbar_switch_type(n);

#define TIMERBAR_PUSH(v) if (Cmdline_timerbar) timerbar_push(v);
#define TIMERBAR_POP()   if (Cmdline_timerbar) timerbar_pop();

#endif

