/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/debugconsole/timerbar.h $
 * $Revision: 1.5 $
 * $Date: 2003-11-09 04:09:17 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2003/11/08 22:25:47  Kazan
 * Timerbar was enabled in both release and debug - so i made it a command line option "-timerbar"
 * DONT MESS WITH OTHER PEOPLES INCLUDE PATHS
 * DONT MESS WITH EXEC NAMES (leave it fs2_open_r and fs2_open_d) or paths!
 *
 *
 * $NoKeywords: $
 */

#ifndef _TIMERBAR_HEADER_
#define _TIMERBAR_HEADER_

const int MAX_NUM_TIMERBARS = 20;

#include "../cmdline/cmdline.h"

// These functions should never be used directly, always use macros below
void timerbar_start_frame();
void timerbar_end_frame();
void timerbar_set_draw_func(void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h));

void timerbar_push(int value);
void timerbar_pop();

// This function shouldnt not be used any more or it will break push and pop calls
void timerbar_switch_type(int num);

// Only show timer bars if in debug mode in windows unless TIMERBAR_ON compile flag is set
#if defined(TIMERBAR_ON)

#define TIMERBAR_SET_DRAW_FUNC(f) if (Cmdline_timerbar) timerbar_set_draw_func(f);
#define TIMERBAR_START_FRAME()    if (Cmdline_timerbar) timerbar_start_frame();
#define TIMERBAR_END_FRAME()      if (Cmdline_timerbar) timerbar_end_frame();
#define TIMERBAR_SWITCH_TYPE(n)   if (Cmdline_timerbar) timerbar_switch_type(n);

#define TIMERBAR_PUSH(v) timerbar_push(v);
#define TIMERBAR_POP()   timerbar_pop();

#else

#define TIMERBAR_SET_DRAW_FUNC(f) ;
#define TIMERBAR_START_FRAME()   ;
#define TIMERBAR_END_FRAME()     ;
#define TIMERBAR_SWITCH_TYPE(n)  ;

#define TIMERBAR_PUSH(v) ;
#define TIMERBAR_POP()   ;

#endif

#endif

