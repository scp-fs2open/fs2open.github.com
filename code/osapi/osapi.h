/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _OSAPI_H
#define _OSAPI_H

// --------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// set if running under MsDev - done after os_init(...) has returned
extern int Os_debugger_running;

// game-wide
// #define THREADED

#ifdef THREADED
	#define ENTER_CRITICAL_SECTION(csc)		do { EnterCriticalSection(csc); } while(0);
	#define LEAVE_CRITICAL_SECTION(csc)		do { LeaveCriticalSection(csc); } while(0);
#else
	#define ENTER_CRITICAL_SECTION(csc)		do { } while(0);
	#define LEAVE_CRITICAL_SECTION(csc)		do { } while(0);
#endif

// --------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(char * wclass, char * title, char *app_name=NULL, char *version_string=NULL );

// set the main window title
void os_set_title( char * title );

// call at program end
void os_cleanup();


// window management ---------------------------------------------------------------

// toggle window size between full screen and windowed
void os_toggle_fullscreen();

// Returns 1 if app is not the foreground app.
int os_foreground();

// Returns the handle to the main window
uint os_get_window();


// process management --------------------------------------------------------------

// call to process windows messages. only does something in non THREADED mode
void os_poll();

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms);

// Used to stop message processing
void os_suspend();

// resume message processing
void os_resume();

#endif
