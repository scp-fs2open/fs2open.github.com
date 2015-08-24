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

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "osapi/dialogs.h"

// --------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// set if running under MsDev - done after os_init(...) has returned
extern int Os_debugger_running;

// --------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// detect the home/base/writable directory to use
extern const char *detect_home(void);

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(const char * wclass, const char * title, const char *app_name=NULL, const char *version_string=NULL );

// set the main window title
void os_set_title( const char * title );

// call at program end
void os_cleanup();

// window management ---------------------------------------------------------------

// toggle window size between full screen and windowed
void os_toggle_fullscreen();

// Returns 1 if app is not the foreground app.
int os_foreground();

// Returns the handle to the main window
SDL_Window* os_get_window();

void os_set_window(SDL_Window* new_handle);	 

// process management --------------------------------------------------------------

// call to process windows messages. only does something in non THREADED mode
void os_poll();

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(uint ms);

// Used to stop message processing
void os_suspend();

// resume message processing
void os_resume(); 

// These map onto the SDL ShowSimpleMessageBox flags
enum MessageBoxType
{
	MESSAGEBOX_ERROR,
	MESSAGEBOX_WARNING,
	MESSAGEBOX_INFORMATION,
};

// Display a simple messagebox through SDL
void SCP_Messagebox(MessageBoxType type, const char* message, const char* title = NULL);

#endif // _OSAPI_H
