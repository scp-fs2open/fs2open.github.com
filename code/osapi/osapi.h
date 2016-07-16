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

/**
 * @brief Determines if FSO is running in legacy config mode
 */
bool os_is_legacy_mode();

/**
 * @brief Gets a path to a configuration file
 * A relative path to the wanted file or directory. Directories should be separated with '/'
 * @param subpath The path of the wanted file or directory, may be emtpy to get the config directory
 * 
 * @returns The path to the specified config path
 */
SCP_string os_get_config_path(const SCP_string& subpath = "");

#endif // _OSAPI_H
