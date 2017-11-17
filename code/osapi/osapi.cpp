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
#include "gamesequence/gamesequence.h"
#include "freespace.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"

#include <fstream>
#include <algorithm>

namespace
{
	const char* ORGANIZATION_NAME = "HardLightProductions";
	const char* APPLICATION_NAME = "FreeSpaceOpen";

	char* preferencesPath = nullptr;

	bool checkedLegacyMode = false;
	bool legacyMode = false;

	SCP_vector<std::unique_ptr<os::Viewport>> viewports;
	os::Viewport* mainViewPort = nullptr;
	SDL_Window* mainSDLWindow = nullptr;

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
	
	bool fAppActive = false;
	bool window_event_handler(const SDL_Event& e)
	{
		Assertion(mainSDLWindow != nullptr, "This function may only be called with a valid SDL Window.");
		if (os::events::isWindowEvent(e, mainSDLWindow)) {
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
	
	bool quit_handler(const SDL_Event& e) {
		gameseq_post_event(GS_EVENT_QUIT_GAME);
		return true;
	}
}


// ----------------------------------------------------------------------------------------------------
// PLATFORM SPECIFIC FUNCTION FOLLOWING
//

#ifdef WIN32

// Windows specific includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, LPARAM param )
{
	const char* search_string = reinterpret_cast<const char*>(param);
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
	EnumWindows(os_enum_windows, reinterpret_cast<LPARAM>(&search_string));
}

void os_set_process_affinity()
{
	HANDLE pHandle = GetCurrentProcess();
	DWORD_PTR pMaskProcess = 0, pMaskSystem = 0;

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

// os-wide globals
static char			szWinTitle[128];
static char			szWinClass[128];
static int			Os_inited = 0;

static SCP_vector<SDL_Event> buffered_events;

int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//
void os_deinit();

// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(const char * wclass, const char * title, const char * app_name)
{
	if (app_name == nullptr || !app_name[0])
	{
		os_init_registry_stuff(Osreg_company_name, title);
	}
	else
	{
		os_init_registry_stuff(Osreg_company_name, app_name);
	}

	strcpy_s( szWinTitle, title );
	strcpy_s( szWinClass, wclass );

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
	os::events::addEventListener(SDL_QUIT, os::events::DEFAULT_LISTENER_WEIGHT, quit_handler);
}

// set the main window title
void os_set_title( const char * title )
{
	Assertion(mainSDLWindow != nullptr, "This function may only be called with a valid SDL Window.");
	strcpy_s( szWinTitle, title );

	SDL_SetWindowTitle(mainSDLWindow, szWinTitle);
}

// call at program end
void os_cleanup()
{
#ifndef NDEBUG
	outwnd_close();
#endif

	os_deinit();
}

// window management -----------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground()
{
	return fAppActive;
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

static bool file_exists(const SCP_string& path) {
	std::ofstream str(path, std::ios::in);
	return str.good();
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
		bool old_config_exists = false;
		bool new_config_exists = false;

		SCP_stringstream path_stream;
		path_stream << getPreferencesPath() << DIR_SEPARATOR_CHAR << Osreg_config_file_name;

		new_config_exists = file_exists(path_stream.str());
#ifdef SCP_UNIX
        path_stream.str("");
		path_stream << Cfile_user_dir_legacy << DIR_SEPARATOR_CHAR << Osreg_config_file_name;

		old_config_exists = file_exists(path_stream.str());
#else
		// At this point we can't determine if the old config exists so just assume that it does
		old_config_exists = true;
#endif

		if (new_config_exists) {
			// If the new config exists then we never use the lagacy mode
			legacyMode = false;
		} else if (old_config_exists) {
			// Old config exists but new doesn't -> use legacy mode
			legacyMode = true;
		} else {
			// Neither old nor new config exists -> this is a new install
			legacyMode = false;
		}
	}

	if (legacyMode) {
		// Print a message for the people running it from the terminal
		fprintf(stdout, "FSO is running in legacy config mode. Please either update your launcher or"
			" copy the configuration and pilot files to '%s' for better future compatibility.\n", getPreferencesPath());
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
	// Free the view ports 
	viewports.clear();

	if (preferencesPath) {
		SDL_free(preferencesPath);
		preferencesPath = nullptr;
	}

	SDL_Quit();
}

void debug_int3(const char *file, int line)
{
	mprintf(("Int3(): From %s at line %d\n", file, line));

	gr_activate(0);

	mprintf(("%s\n", dump_stacktrace().c_str()));

#ifndef NDEBUG
	SDL_TriggerBreakpoint();
#endif

	gr_activate(1);
}

namespace os
{
	Viewport* addViewport(std::unique_ptr<Viewport>&& viewport) {
		auto port = viewport.get();
		viewports.push_back(std::move(viewport));
		return port;
	}
	void setMainViewPort(Viewport* mainView) {
		mainViewPort = mainView;
		mainSDLWindow = mainView->toSDLWindow();
	}
	SDL_Window* getSDLMainWindow() {
		return mainSDLWindow;
	}
	Viewport* getMainViewport() {
		return mainViewPort;
	}

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

		bool isWindowEvent(const SDL_Event& e, SDL_Window* window)
		{
			auto mainId = SDL_GetWindowID(window);
			switch(e.type)
			{
			case SDL_WINDOWEVENT:
				return mainId == e.window.windowID;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				return mainId == e.key.windowID;
			case SDL_TEXTEDITING:
				return mainId == e.edit.windowID;
			case SDL_TEXTINPUT:
				return mainId == e.text.windowID;
			case SDL_MOUSEMOTION:
				return mainId == e.motion.windowID;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				return mainId == e.button.windowID;
			case SDL_MOUSEWHEEL:
				return mainId == e.wheel.windowID;
			default:
				// Event doesn't have a window ID
				return true;
			}
		}
	}
}
	
void os_ignore_events() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Add event to buffer
		buffered_events.push_back(event);
	}
}

static void handle_sdl_event(const SDL_Event& event) {
	using namespace os::events;
	
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

void os_poll()
{
	// Replay the buffered events
	auto end = buffered_events.end();
	for (auto it = buffered_events.begin(); it != end; ++it) {
		handle_sdl_event(*it);
	}
	buffered_events.clear();

	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		handle_sdl_event(event);
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

