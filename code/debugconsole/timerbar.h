#ifndef _TIMERBAR_HEADER_
#define _TIMERBAR_HEADER_

enum 
{
	TIMERBAR_COLOUR_RED,
	TIMERBAR_COLOUR_GREEN,
	TIMERBAR_COLOUR_BLUE,
	TIMERBAR_COLOUR_MAX,
};

void timerbar_start_frame();
void timerbar_end_frame();
void timerbar_switch_type(int num);
void timerbar_set_draw_func(void (*new_draw_func_ptr)(int colour, float x, float y, float w, float h));

#ifdef TIMEBAR_ACTIVE 

#define TIMERBAR_SET_DRAW_FUNC(f) timerbar_set_draw_func(f);
#define TIMERBAR_START_FRAME()   timerbar_start_frame();
#define TIMERBAR_END_FRAME()     timerbar_end_frame();
#define TIMERBAR_SWITCH_TYPE(n)  timerbar_switch_type(n);

#else

#define TIMERBAR_SET_DRAW_FUNC(f) ;
#define TIMERBAR_START_FRAME()   ;
#define TIMERBAR_END_FRAME()     ;
#define TIMERBAR_SWITCH_TYPE(n)  ;

#endif

#endif

