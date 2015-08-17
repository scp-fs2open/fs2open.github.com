/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <algorithm>

#include "globalincs/pstypes.h"
#include "io/key.h"
#include "palman/palman.h"
#include "io/mouse.h"
#include "osapi/outwnd.h"
#include "sound/sound.h"
#include "freespace2/freespaceresource.h"
#include "playerman/managepilot.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"
#include "sound/voicerec.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"


#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"

#include "SDL_syswm.h"

#include <SDL_assert.h>


// used to be a THREADED define but only use multiple process threads if this is defined
// NOTE: may hang if set
//#define THREADED_PROCESS

namespace
{
	const char* ORGANIZATION_NAME = "HardLightProductions";
	const char* APPLICATION_NAME = "FreeSpaceOpen";
	
	char* preferencesPath = nullptr;

	bool checkedLegacyMode = false;
	bool legacyMode = false;

	const char* getPreferencesPath()
	{
		// Lazily initialize the preferences path
		if (!preferencesPath) {
			preferencesPath = SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME);
			if (!preferencesPath) {
				mprintf(("Failed to get preferences path from SDL: %s\n", SDL_GetError()));
			}
		}

		if (preferencesPath) {
			return preferencesPath;
		}
		else {
			// No preferences path, try current directory
			return "." DIR_SEPARATOR_STR;
		}

	}
}

// ----------------------------------------------------------------------------------------------------
// PLATFORM SPECIFIC FUNCTION FOLLOWING
//


#ifdef WIN32

// Windows specific includes
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <io.h>
#include <winsock.h>
#include <direct.h>

// For FRED
void os_set_window_from_hwnd(HWND handle)
{
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	if (SDL_GL_LoadLibrary(NULL) < 0)
		Error(LOCATION, "Failed to load OpenGL library: %s!", SDL_GetError());

	SDL_Window* window = SDL_CreateWindowFrom((void*) handle);

	os_set_window(window);
}

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string )
{
	char tmp[128];
	int len;

	len = GetWindowText( hwnd, tmp, 127 );
	 
	if ( len )	{
		if ( strstr( tmp, search_string ))	{
			Os_debugger_running = 1;		// found the search string!
			return FALSE;	// stop enumerating windows
		}
	}

	return TRUE;	// continue enumeration
}

// Fills in the Os_debugger_running with non-zero if debugger detected.
void os_check_debugger()
{
	HMODULE hMod;
	char search_string[256];
	char myname[128];
	int namelen;
	char * p;

	Os_debugger_running = 0;		// Assume its not

	// Find my EXE file name
	hMod = GetModuleHandle(NULL);
	if ( !hMod ) return;
	namelen = GetModuleFileName( hMod, myname, 127 );	
	if ( namelen < 1 ) return;
	
	// Strip off the .EXE
	p = strstr( myname, ".exe" );
	if (!p) return;
	*p = '\0';

	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	
	if ( strlen(p) < 1 ) return;

	// Build what the debugger's window title would be if the debugger is running...
	sprintf( search_string, "[run] - %s -", p );

	// ... and then search for it.
	EnumWindows( (int (__stdcall *)(struct HWND__ *,long))os_enum_windows, (long)&search_string );
}

void os_set_process_affinity()
{
	HANDLE pHandle = GetCurrentProcess();
	DWORD pMaskProcess = 0, pMaskSystem = 0;

	if ( GetProcessAffinityMask(pHandle, &pMaskProcess, &pMaskSystem) ) {
		// only do this if we have at least 2 procs
		if (pMaskProcess >= 3) {
			// prefer running on the second processor by default
			pMaskProcess = os_config_read_uint(NULL, "ProcessorAffinity", 2);

			if (pMaskProcess > 0) {
				SetProcessAffinityMask(pHandle, pMaskProcess);
			}
		}
	}
}

#endif // WIN32


// ----------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

static SDL_Window* main_window = NULL;

// os-wide globals
static bool			fAppActive = false;
static char			szWinTitle[128];
static char			szWinClass[128];
static int			Os_inited = 0;

static SDL_mutex* Os_lock;

int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit();

// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(const char * wclass, const char * title, const char *app_name, const char *version_string )
{
	os_init_registry_stuff(Osreg_company_name, title, version_string);

	strcpy_s( szWinTitle, title );
	strcpy_s( szWinClass, wclass );	

	Os_lock = SDL_CreateMutex();

	mprintf(("  Initializing SDL...\n"));

	if (SDL_Init(SDL_INIT_EVENTS) < 0)
	{
		fprintf(stderr, "Couldn't init SDL: %s", SDL_GetError());
		mprintf(("Couldn't init SDL: %s", SDL_GetError()));
		
		exit(1);
		return;
	}

#ifdef FS2_VOICER
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE); // We currently only need this for voice recognition
#endif

	// initialized
	Os_inited = 1;

#ifdef WIN32
	// check to see if we're running under msdev
	os_check_debugger();

	if (Cmdline_set_cpu_affinity)
	{
		// deal with processor affinity
		os_set_process_affinity();
	}
#endif // WIN32

	atexit(os_deinit);
}

// set the main window title
void os_set_title( const char * title )
{
	strcpy_s( szWinTitle, title );

	SDL_SetWindowTitle(main_window, szWinTitle);
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
SDL_Window* os_get_window()
{
	return main_window;
}

// Returns the handle to the main window
void os_set_window(SDL_Window* new_handle)
{
	main_window = new_handle;
	fAppActive = true;
}

// process management -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(uint ms)
{
#ifdef __APPLE__
	// ewwww, I hate this!!  SDL_Delay() is causing issues for us though and this
	// basically matches Apple examples of the same thing.  Same as SDL_Delay() but
	// we aren't hitting up the system for anything during the process
	uint then = SDL_GetTicks() + ms;

	while (then > SDL_GetTicks());
#else
	SDL_Delay(ms);
#endif
}

// Used to stop message processing
void os_suspend()
{
	SDL_LockMutex( Os_lock );	
}

// resume message processing
void os_resume()
{
	SDL_UnlockMutex( Os_lock );	
}

bool os_is_legacy_mode()
{
	// Make this check a little faster by caching the result
	if (checkedLegacyMode)
	{
		return legacyMode;
	}

	if (Cmdline_portable_mode) {
		// When the portable mode option is given, non-legacy is implied
		legacyMode = false;
		checkedLegacyMode = true;
	}
	else {
		SCP_stringstream path_stream;
		path_stream << getPreferencesPath() << DIR_SEPARATOR_CHAR << Osreg_config_file_name;

		// Use the existance of the fs2_open.ini file for determining if the launcher supports the new mode
		auto file = fopen(path_stream.str().c_str(), "r");

		if (file == nullptr)
		{
			legacyMode = true;
		}
		else
		{
			fclose(file);
			legacyMode = false;
		}
	}

	if (legacyMode) {
		// Print a message for the people running it from the terminal
		fprintf(stdout, "FSO is running in legacy config mode. Please either update your launcher or"
			" copy the configuration and pilot files to '%s' for better future compatibility.", getPreferencesPath());
	}

	checkedLegacyMode = true;
	return legacyMode;
}

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	if (preferencesPath) {
		SDL_free(preferencesPath);
		preferencesPath = nullptr;
	}
	
	SDL_DestroyMutex(Os_lock);

	SDL_Quit();
}

extern SCP_map<int, int> SDLtoFS2;
extern void joy_set_button_state(int button, int state);
extern void joy_set_hat_state(int position);

void os_poll()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_WINDOWEVENT: {
			if (event.window.windowID == SDL_GetWindowID(os_get_window())) {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					{
						if (fAppActive) {
							if (!Cmdline_no_unfocus_pause) {
								game_pause();
							}
							
							fAppActive = false;
						}
						break;
					}
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						if (!fAppActive) {
							if (!Cmdline_no_unfocus_pause) {
								game_unpause();
							}
							
							fAppActive = true;
						}
						break;
					}
					case SDL_WINDOWEVENT_CLOSE:
						gameseq_post_event(GS_EVENT_QUIT_GAME);
						break;
				}
			}
			
			gr_activate(fAppActive);
			
			break;
		}

		case SDL_SYSWMEVENT:
#ifdef WIN32
#ifdef FS2_VOICER
			switch(event.syswm.msg->msg.win.msg)
			{
			case WM_RECOEVENT:
				if ( Game_mode & GM_IN_MISSION && Cmdline_voice_recognition)
				{
					VOICEREC_process_event( event.syswm.msg->msg.win.hwnd );
				}
				break;
			default:
				break;
			}
#endif // FS2_VOICER
#endif // WIN32
			break;

		case SDL_KEYDOWN:
			if (SDLtoFS2[event.key.keysym.scancode]) {
				key_mark(SDLtoFS2[event.key.keysym.scancode], 1, 0);
			}
			break;

		case SDL_KEYUP:
			if (SDLtoFS2[event.key.keysym.scancode]) {
				key_mark(SDLtoFS2[event.key.keysym.scancode], 0, 0);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
				mouse_mark_button(MOUSE_LEFT_BUTTON, event.button.state);
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				mouse_mark_button(MOUSE_MIDDLE_BUTTON, event.button.state);
			else if (event.button.button == SDL_BUTTON_RIGHT)
				mouse_mark_button(MOUSE_RIGHT_BUTTON, event.button.state);

			break;

		case SDL_JOYAXISMOTION:
			joy_event(event.jaxis.which, event.jaxis.axis, event.jaxis.value);
			break;

		case SDL_JOYHATMOTION:
			joy_set_hat_state(event.jhat.value);
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (event.jbutton.button < JOY_NUM_BUTTONS) {
				joy_set_button_state(event.jbutton.button, event.jbutton.state);
			}
			break;
		
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			joy_device_changed(event.jdevice.type, event.jdevice.which);
			break;
		case SDL_MOUSEMOTION:
			mouse_event(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			break;

		case SDL_MOUSEWHEEL:
			mousewheel_motion(event.wheel.x, event.wheel.y);
			break;
		}
	}
}

SCP_string os_get_config_path(const SCP_string& subpath)
{
	// Make path platform compatible
	SCP_string compatiblePath(subpath);
	std::replace(compatiblePath.begin(), compatiblePath.end(), '/', DIR_SEPARATOR_CHAR);

	SCP_stringstream ss;

	if (Cmdline_portable_mode) {
		// Use the current directory
		ss << "." << DIR_SEPARATOR_CHAR << compatiblePath;
		return ss.str();
	}

	// Avoid infinite recursion when checking legacy mode
	if (os_is_legacy_mode()) {
#ifdef WIN32
		// Use the current directory
		ss << ".";
#else
		extern const char* Osreg_user_dir_legacy;
		// Use the home directory
		ss << getenv("HOME") << DIR_SEPARATOR_CHAR << Osreg_user_dir_legacy;
#endif

		ss << DIR_SEPARATOR_CHAR << compatiblePath;
		return ss.str();
	}
	
	ss << getPreferencesPath() << compatiblePath;
	
	return ss.str();
}

void debug_int3(char *file, int line)
{
	mprintf(("Int3(): From %s at line %d\n", file, line));

	gr_activate(0);

	mprintf(("%s\n", dump_stacktrace().c_str()));

#ifndef NDEBUG
	SDL_TriggerBreakpoint();
#endif

	gr_activate(1);
	// we have to call os_deinit() before abort() so we make sure that SDL gets
	// closed out and we don't lose video/input control
	os_deinit();

	abort();
}
