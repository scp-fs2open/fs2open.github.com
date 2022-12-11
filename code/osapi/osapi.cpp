/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "freespace.h"

#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"
#include "parse/parselo.h"

#include <fcntl.h>
#include <utf8.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"

#ifdef SCP_UNIX
#include <sys/stat.h>
#elif defined(WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#endif

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
			
			// this section will at least tell the user if something is seriously wrong instead of just crashing without a message or debug log.
			// It may crash later, especially when trying to load sound. But let's let it *try* to run in the current directory at least.
		    if (preferencesPath == nullptr) {
				static bool sdl_is_borked_warning = false;
				if (!sdl_is_borked_warning) {
					ReleaseWarning(LOCATION, "%s\n\nSDL and Windows are unable to get the preferred path for the reason above. "
						"Installing FSO, its executables and DLLs in another non-protected folder may fix the issue.\n\n"
						"You may experience issues if you continue playing, and FSO may crash. Please report this error if it persists.\n\n"
						"Report at www.hard-light.net or the hard-light discord.", SDL_GetError());
					sdl_is_borked_warning = true;
				}
				// No preferences path, try current directory.
				Cmdline_portable_mode = true;
				return "." DIR_SEPARATOR_STR;
		    }
#ifdef WIN32
		    try {
			    auto current           = preferencesPath;
			    const auto prefPathEnd = preferencesPath + strlen(preferencesPath);
			    while (current != prefPathEnd) {
				    const auto cp = utf8::next(current, prefPathEnd);
				    if (cp > 127) {
					    // On Windows, we currently do not support Unicode paths so force portable mode let the user
					    // know
					    const auto invalid_end = current;
						static bool force_portable_warning = false;
						if (!force_portable_warning) {
							utf8::prior(current, preferencesPath);
							ReleaseWarning(LOCATION,
								"Determined the preferences path as \"%s\". That path is not supported since it "
								"contains a Unicode character (%s). Using portable mode. Set -portable_mode in "
								"the commandline to avoid this message in the future.",
								preferencesPath, std::string(current, invalid_end).c_str());
							force_portable_warning = true;
						}
						Cmdline_portable_mode = true;
						return "." DIR_SEPARATOR_STR;
				    }
			    }
		    } catch (const std::exception& e) {
			    Error(LOCATION, "UTF-8 error while checking the preferences path \"%s\": %s", preferencesPath,
			          e.what());
		    }
#endif
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
	
	bool quit_handler(const SDL_Event&  /*e*/) {
		mprintf(("Recevied quit signal\n"));
		gameseq_post_event(GS_EVENT_QUIT_GAME);
		return true;
	}

	const char* mapCategory(int category) {
		switch (category) {
			case SDL_LOG_CATEGORY_APPLICATION:
				return "APPLICATION";
			case SDL_LOG_CATEGORY_ERROR:
				return "ERROR";
			case SDL_LOG_CATEGORY_ASSERT:
				return "ASSERT";
			case SDL_LOG_CATEGORY_SYSTEM:
				return "SYSTEM";
			case SDL_LOG_CATEGORY_AUDIO:
				return "AUDIO";
			case SDL_LOG_CATEGORY_VIDEO:
				return "VIDEO";
			case SDL_LOG_CATEGORY_RENDER:
				return "RENDER";
			case SDL_LOG_CATEGORY_INPUT:
				return "INPUT";
			case SDL_LOG_CATEGORY_TEST:
				return "TEST";

			default:
				return "UNKNOWN";
		}
	}

	const char* mapPriority(SDL_LogPriority prio)
	{
		switch (prio) {
		case SDL_LOG_PRIORITY_VERBOSE:
			return "VRB";
		case SDL_LOG_PRIORITY_DEBUG:
			return "DBG";
		case SDL_LOG_PRIORITY_INFO:
			return "INF";
		case SDL_LOG_PRIORITY_WARN:
			return "WRN";
		case SDL_LOG_PRIORITY_ERROR:
			return "ERR";
		case SDL_LOG_PRIORITY_CRITICAL:
			return "CRI";

		default:
			return "UNK";
		}
	}

	void SDLCALL logHandler(void*, int category, SDL_LogPriority priority, const char* message) {
		if (priority >= SDL_LOG_PRIORITY_INFO) {
			mprintf(("SDL [%s][%s]: %s\n", mapPriority(priority), mapCategory(category), message));
		} else {
			nprintf(("SDL", "SDL [%s][%s]: %s\n", mapPriority(priority), mapCategory(category), message));
		}
	}
}


// ----------------------------------------------------------------------------------------------------
// PLATFORM SPECIFIC FUNCTION FOLLOWING
//

#ifdef WIN32

// Windows specific includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <backends/imgui_impl_sdl.h>

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

static SCP_vector<SDL_Event> deferred_events;

int Os_debugger_running = 0;

#ifdef SCP_UNIX
static bool user_dir_initialized = false;
static SCP_string Os_user_dir_legacy;

const char* os_get_legacy_user_dir() {
	if (user_dir_initialized) {
		return Os_user_dir_legacy.c_str();
	}

	extern const char* Osreg_user_dir_legacy;
	sprintf(Os_user_dir_legacy, "%s/%s", getenv("HOME"), Osreg_user_dir_legacy);
	user_dir_initialized = true;

	return Os_user_dir_legacy.c_str();
}
#endif

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

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	mprintf(("  Initializing SDL %d.%d.%d (compiled with %d.%d.%d)...\n", linked.major, linked.minor, linked.patch,
	         compiled.major, compiled.minor, compiled.patch));

	if (LoggingEnabled) {
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
		SDL_LogSetOutputFunction(&logHandler, nullptr);
	}

	if (SDL_Init(SDL_INIT_EVENTS) < 0)
	{
		fprintf(stderr, "Couldn't init SDL: %s", SDL_GetError());
		mprintf(("Couldn't init SDL: %s\n", SDL_GetError()));

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
	os_deinit_registry_stuff();

	if (LoggingEnabled) {
		outwnd_close();
	}

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

static time_t get_file_modification_time(const SCP_string& path) {
#ifdef SCP_UNIX
	struct stat file_stats{};
	if(stat(path.c_str(), &file_stats) < 0) {
		return 0;
	}

	return file_stats.st_mtime;
#elif defined(WIN32)
	struct _stat buf{};
	if (_stat(path.c_str(), &buf) != 0) {
		return 0;
	}
	return buf.st_mtime;
#else
#error Unsupported platform!
#endif
}

const char* Osapi_legacy_mode_reason = nullptr;

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

		Osapi_legacy_mode_reason = "Legacy mode disabled since portable mode was enabled.";
	}
	else {
		SCP_stringstream path_stream;
		path_stream << getPreferencesPath() << Osreg_config_file_name;

		auto new_config_exists = file_exists(path_stream.str());
		time_t new_config_time = 0;
		if (new_config_exists) {
			new_config_time = get_file_modification_time(path_stream.str());
		}

		// Also check the modification times of the command line files in case the launcher did not change the settings
		// file
		path_stream.str("");
		path_stream << getPreferencesPath() << "data" << DIR_SEPARATOR_CHAR << "cmdline_fso.cfg";
		new_config_time = std::max(new_config_time, get_file_modification_time(path_stream.str()));
#ifdef SCP_UNIX
        path_stream.str("");
		path_stream << os_get_legacy_user_dir() << DIR_SEPARATOR_CHAR << Osreg_config_file_name;

		auto old_config_exists = file_exists(path_stream.str());
		time_t old_config_time = 0;
		if (old_config_exists) {
			old_config_time = get_file_modification_time(path_stream.str());
		}

		path_stream.str("");
		path_stream << os_get_legacy_user_dir() << DIR_SEPARATOR_CHAR << "data" << DIR_SEPARATOR_CHAR
					<< "cmdline_fso.cfg";
		old_config_time = std::max(old_config_time, get_file_modification_time(path_stream.str()));
#else
		// At this point we can't determine if the old config exists so just assume that it does
		auto old_config_exists = true;
		time_t old_config_time = os_registry_get_last_modification_time();

		// On Windows the cmdline_fso file was stored in the game root directory which should be in the current directory
		path_stream.str("");
		path_stream << "." << DIR_SEPARATOR_CHAR << "data" << DIR_SEPARATOR_CHAR << "cmdline_fso.cfg";
		old_config_time = std::max(old_config_time, get_file_modification_time(path_stream.str()));
#endif

		if (new_config_exists && old_config_exists) {
			// Both config files exists so we need to decide which to use based on their last modification times
			// if the old config was modified more recently than the new config then we use the legacy mode since the
			// user probably used an outdated launcher after using a more recent one
			legacyMode = old_config_time > new_config_time;

			if (legacyMode) {
				Osapi_legacy_mode_reason = "Legacy mode enabled since the old config location was used more recently than the new location.";
			} else {
				Osapi_legacy_mode_reason = "Legacy mode disabled since the new config location was used more recently than the old location.";
			}
		} else if (new_config_exists) {
			// If the new config exists and the old one doesn't then we can safely disable the legacy mode
			legacyMode = false;

			Osapi_legacy_mode_reason = "Legacy mode disabled since the old config does not exist while the new config exists.";
		} else if (old_config_exists) {
			// Old config exists but new doesn't -> use legacy mode
			legacyMode = true;

			Osapi_legacy_mode_reason = "Legacy mode enabled since the old config exists while the new config does not exist.";
		} else {
			// Neither old nor new config exists -> this is a new install
			legacyMode = false;

			Osapi_legacy_mode_reason = "Legacy mode disabled since no existing config was detected.";
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
	os::closeAllViewports();

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
	void closeAllViewports() {
		viewports.clear();
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
	
void os_defer_events_on_load_screen() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Add event to be handled later
		deferred_events.push_back(event);
	}
}

static void handle_sdl_event(const SDL_Event& event) {
	using namespace os::events;

	bool imgui_processed_this = false;
	if (gameseq_get_state() == GS_STATE_LAB && (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)) {
		imgui_processed_this = ImGui_ImplSDL2_ProcessEvent(&event);
	}

	if (!imgui_processed_this) {
		EventListenerData data;
		data.type = event.type;

		auto iter = std::lower_bound(eventListeners.begin(), eventListeners.end(), data, compare_type);

		if (iter != eventListeners.end()) {
			// The vector contains all event listeners, the listeners are sorted for type and weight
			// -> iterating through all listeners will yield them in increasing weight order
			// but we can only do this until we have reached the end of the vector or the type has changed
			for (; iter != eventListeners.end() && iter->type == event.type; ++iter) {
				if (iter->listener(event)) {
					// Listener has handled the event
					break;
				}
			}
		}
	}
}

void os_remove_deferred_cutscene_key_events() {
	deferred_events.erase(
		std::remove_if(deferred_events.begin(), deferred_events.end(), [](const SDL_Event &event)
		{
			return (event.type == SDL_KEYUP) && (
				event.key.keysym.sym == SDLK_KP_ENTER ||
				event.key.keysym.sym == SDLK_RETURN ||
				event.key.keysym.sym == SDLK_SPACE
			);
		}),
		deferred_events.end()
	);
}

void os_poll()
{
	// Replay the deferred events
	auto end = deferred_events.end();
	for (auto it = deferred_events.begin(); it != end; ++it) {
		handle_sdl_event(*it);
	}
	deferred_events.clear();

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

