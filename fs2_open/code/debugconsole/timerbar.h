/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/debugconsole/timerbar.h $
 * $Revision: 1.10 $
 * $Date: 2005-07-13 02:50:50 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2004/08/11 05:06:20  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.8  2004/02/16 11:47:31  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 1.7  2003/11/19 20:37:23  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 1.6  2003/11/09 06:31:39  Kazan
 * a couple of htl functions being called in nonhtl (ie NULL functions) problems fixed
 * conflicts in cmdline and timerbar.h log entries
 * cvs stopped acting like it was on crack obviously
 *
 * Revision 1.5  2003/11/09 04:09:17  Goober5000
 * edited for language
 * --Goober5000
 *
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

