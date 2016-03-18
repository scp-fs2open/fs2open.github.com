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

#include "globalincs/pstypes.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"


#include <SDL_assert.h>

#include <algorithm>

namespace
{
	bool fAppActive = false;

	bool window_event_handler(const SDL_Event& e)
	{
		if (e.window.windowID == SDL_GetWindowID(os_get_window())) {
			switch (e.window.event) {
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

			gr_activate(fAppActive);

			return true;
		}

		return false;
	}
}

#ifdef WIN32

// Windows specific includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

// detect home/base directory  (placeholder for possible future Win32 userdir support, just returns current directory for now)
char Cur_path[MAX_PATH_LEN];
const char *detect_home(void)
{
#ifdef WIN32
	if ( strlen(Cfile_root_dir) )
		return Cfile_root_dir;

	memset( Cur_path, 0, MAX_PATH_LEN );
	GetCurrentDirectory( MAX_PATH_LEN-1, Cur_path );

	return Cur_path;
#else
	return (getenv("HOME"));
#endif
}

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

	os::events::addEventListener(SDL_WINDOWEVENT, os::events::DEFAULT_LISTENER_WEIGHT, window_event_handler);

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

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	SDL_DestroyMutex(Os_lock);

	SDL_Quit();
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

namespace os
{
	namespace events
	{
		namespace
		{
			ListenerIdentifier nextListenerIdentifier;

			struct EventListenerData
			{
				ListenerIdentifier identifier;
				Listener listener;
				
				uint32_t type;
				int weight;

				bool operator<(const EventListenerData& other) const
				{
					if (type < other.type)
					{
						return true;
					}
					if (type > other.type)
					{
						return false;
					}
					
					// Type is the same
					return weight < other.weight;
				}
			};
			
			bool compare_type(const EventListenerData& left, const EventListenerData& right)
			{
				return left.type < right.type;
			}
			
			SCP_vector<EventListenerData> eventListeners;
		}

		ListenerIdentifier addEventListener(SDL_EventType type, int weight, const Listener& listener)
		{
			Assertion(listener, "Invalid event handler passed!");

			EventListenerData data;
			data.identifier = ++nextListenerIdentifier;
			data.listener = listener;
			
			data.weight = weight;
			data.type = static_cast<uint32_t>(type);

			eventListeners.push_back(data);
			// This is suboptimal for runtime but we will iterate that vector often so cache hits are more important
			std::sort(eventListeners.begin(), eventListeners.end());

			return data.identifier;
		}

		bool removeEventListener(ListenerIdentifier identifier)
		{
			auto endIter = end(eventListeners);
			for (auto iter = begin(eventListeners); iter != endIter; ++iter)
			{
				if (iter->identifier == identifier)
				{
					eventListeners.erase(iter);
					return true; // Identifiers are unique
				}
			}

			return false;
		}
	}
}

void os_poll()
{
	using namespace os::events;

	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		EventListenerData data;
		data.type = event.type;
			
		auto iter = std::lower_bound(eventListeners.begin(), eventListeners.end(), data, compare_type);

		if (iter != eventListeners.end())
		{
			// The vector contains all event listeners, the listeners are sorted for type and weight
			// -> iterating through all listeners will yield them in increasing weight order
			// but we can only do this until we have reached the end of the vector or the type has changed
			for(; iter != eventListeners.end() && iter->type == event.type; ++iter)
			{
				if (iter->listener(event))
				{
					// Listener has handled the event
					break;
				}
			}
		}
	}
}
