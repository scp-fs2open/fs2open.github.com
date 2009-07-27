/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef WIN32	// Goober5000

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>

#include "globalincs/pstypes.h"
#include "io/key.h"
#include "io/mouse.h"
#include "osapi/outwnd.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "osapi/osregistry.h"
#include "graphics/2d.h"
#include "freespace2/freespace.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"

// ----------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// os-wide globals
static int			fAppActive = 1;
static char			szWinTitle[128];
static char			szWinClass[128];
//static int			WinX, WinY, WinW, WinH;
static int			Os_inited = 0;

static CRITICAL_SECTION Os_lock;

int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

DWORD unix_process(DWORD lparam);

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit();

// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// detect users home directory
const char *detect_home(void)
{
	return (getenv("HOME"));
}

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(char * wclass, char * title, char *app_name, char *version_string )
{
#ifndef NDEBUG
	outwnd_init(1);
#endif	

	// create default ini entries for the user
	if (os_config_read_string(NULL, NOX("VideocardFs2open"), NULL) == NULL)
		os_config_write_string(NULL, NOX("VideocardFs2open"), NOX("OGL -(640x480)x16 bit"));

	os_init_registry_stuff(Osreg_company_name, title, version_string);
	
	strcpy_s( szWinTitle, title );
	strcpy_s( szWinClass, wclass );	

	INITIALIZE_CRITICAL_SECTION( Os_lock );

	unix_process(0);

	// initialized
	Os_inited = 1;

	atexit(os_deinit);
}

// set the main window title
void os_set_title( char *title )
{
	strcpy_s( szWinTitle, title );

	SDL_WM_SetCaption( szWinTitle, NULL );
}

extern void gr_opengl_shutdown();

// call at program end
void os_cleanup()
{
	gr_opengl_shutdown();

#ifndef NDEBUG
	outwnd_close();
#endif
}


// window management -----------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground()
{
	return fAppActive;
}

// Returns the handle to the main window
/*uint os_get_window()
{
	// not used
	return 0;
}*/


// process management -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms)
{
	Sleep(ms);
}

// Used to stop message processing
void os_suspend()
{
	ENTER_CRITICAL_SECTION( Os_lock );	
}

// resume message processing
void os_resume()
{
	LEAVE_CRITICAL_SECTION( Os_lock );	
}


// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

extern int SDLtoFS2[SDLK_LAST];
extern void joy_set_button_state(int button, int state);
extern void joy_set_hat_state(int position);

DWORD unix_process(DWORD lparam)
{
	SDL_Event event;

	while( SDL_PollEvent(&event) ) {
		switch(event.type) {
			case SDL_ACTIVEEVENT:
				if( (event.active.state & SDL_APPACTIVE) || (event.active.state & SDL_APPINPUTFOCUS) ) {
					if (fAppActive != event.active.gain) {
						if (fAppActive)
							game_pause();
						else
							game_unpause();
					}
					fAppActive = event.active.gain;
					gr_activate(fAppActive);
				}
				break;

			case SDL_KEYDOWN:
				/*if( (event.key.keysym.mod & KMOD_ALT) && (event.key.keysym.sym == SDLK_RETURN) ) {
					Gr_screen_mode_switch = 1;
					gr_activate(1);
					break;
				}*/

				if( SDLtoFS2[event.key.keysym.sym] ) {
					key_mark( SDLtoFS2[event.key.keysym.sym], 1, 0 );
				}
				break;

			case SDL_KEYUP:
				/*if( (event.key.keysym.mod & KMOD_ALT) && (event.key.keysym.sym == SDLK_RETURN) ) {
					Gr_screen_mode_switch = 0;
					break;
				}*/

				if (SDLtoFS2[event.key.keysym.sym]) {
					key_mark( SDLtoFS2[event.key.keysym.sym], 0, 0 );
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					mouse_mark_button( MOUSE_LEFT_BUTTON, event.button.state );
				else if (event.button.button == SDL_BUTTON_MIDDLE)
					mouse_mark_button( MOUSE_MIDDLE_BUTTON, event.button.state );
				else if (event.button.button == SDL_BUTTON_RIGHT)
					mouse_mark_button( MOUSE_RIGHT_BUTTON, event.button.state );

				break;

			case SDL_JOYHATMOTION:
				joy_set_hat_state( event.jhat.value );
				break;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if (event.jbutton.button < JOY_NUM_BUTTONS) {
					joy_set_button_state( event.jbutton.button, event.jbutton.state );
				}
				break;
		}
	}

	return 0;
}


// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	DELETE_CRITICAL_SECTION( Os_lock );

	SDL_Quit();
}

void os_poll()
{
	unix_process(0);
}

void debug_int3(char *file, int line)
{
	mprintf(("Int3(): From %s at line %d\n", file, line));

	// we have to call os_deinit() before abort() so we make sure that SDL gets
	// closed out and we don't lose video/input control
	os_deinit();

	abort();
}

#endif		// Goober5000 - #ifndef WIN32
