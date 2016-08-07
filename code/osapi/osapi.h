/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/** @defgroup osapi The OS API
 *  Provides a number of function to interact with the operating system. The API is the same for every platform.
 */

/** @file
 *  @ingroup osapi
 */


#ifndef _OSAPI_H
#define _OSAPI_H

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "osapi/dialogs.h"

#include <functional>
#include <memory>

#include <SDL_events.h>

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

enum class WindowState {
	Windowed,
	Borderless,
	Fullscreen
};

void os_set_window_state(WindowState state);

// Returns 1 if app is not the foreground app.
int os_foreground();

// Returns the handle to the main window
SDL_Window* os_get_window();

void os_set_window(SDL_Window* new_handle);

// process management --------------------------------------------------------------

/**
 * @brief Removes all pending events and ignores them
 */
void os_ignore_events();

// call to process windows messages. only does something in non THREADED mode
void os_poll();

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(uint ms);

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

namespace os
{
	/**
	 * @brief Flags for OpenGL context creation
	 */
	enum OpenGLContextFlags
	{
		OGL_NONE = 0,
		OGL_DEBUG = 1 << 0,
		OGL_FORWARD_COMPATIBLE = 1 << 1
	};

	/**
	 * @brief The required context profile
	 */
	enum class OpenGLProfile
	{
		Core,
		Compatibility
	};

	/**
	 * @brief Attributes for OpenGL context creation
	 */
	struct OpenGLContextAttributes
	{
		uint32_t red_size;
		uint32_t green_size;
		uint32_t blue_size;
		uint32_t alpha_size;

		uint32_t depth_size;
		uint32_t stencil_size;

		uint32_t multi_samples;

		uint32_t major_version;
		uint32_t minor_version;

		uint32_t flags;
		OpenGLProfile profile;
	};

	/**
	 * @brief A function pointer for loading an OpenGL function
	 */
	typedef void* (*OpenGLLoadProc)(const char *name);

	/**
	 * @brief An OpenGL context
	 * Can be deleted which will properly free the resources of the underlying OpenGL context
	 */
	class OpenGLContext
	{
	public:
		virtual ~OpenGLContext() {}

		/**
		 * @brief Gets an OpenGL loader function
		 */
		virtual OpenGLLoadProc getLoaderFunction() = 0;

		/**
		 * @brief Swaps the buffers of this context
		 */
		virtual void swapBuffers() = 0;

		/**
		 * @brief Sets the swap interval
		 */
		virtual void setSwapInterval(int status) = 0;
	};

	/**
	 * @brief Abstraction for various graphics operations
	 * 
	 * This is used for providing platform specific functionality for various graphics operations.
	 */
	class GraphicsOperations {
	public:
		virtual ~GraphicsOperations() {}

		/**
		 * @brief Creates an OpenGL contex
		 *
		 * Uses the specified attributes and creates an OpenGL context. The width and height
		 * values may be used for creating the main window.
		 *
		 * @param attrs The desired Context attributes
		 * @param width The width of the main window
		 * @param height The height of the main window
		 *
		 * @return A pointer to the OpenGL context or @c nullptr if the creation has failed
		 */
		virtual std::unique_ptr<OpenGLContext> createOpenGLContext(const OpenGLContextAttributes& attrs,
												  uint32_t width, uint32_t height) = 0;

		/**
		 * @brief Makes an OpenGL context current
		 * @param ctx The OpenGL context to make current, may be @c nullptr
		 */
		virtual void makeOpenGLContextCurrent(OpenGLContext* ctx) = 0;
	};

	/**
	 * @defgroup eventhandling API for consuming OS events
	 * @ingroup osapi
	 * See \ref eventhandling_page for more information.
	 *  @{
	 */
	namespace events
	{
		/**
		 * @brief The default weight of a listener.
		 * The default event handler use this weight so other event handlers should have a weight that is less than this.
		 */
		const int DEFAULT_LISTENER_WEIGHT = 0;

		/**
		 * @brief An event handler
		 * Gets the generated sdl event and must return if it handled the event or not.
		 */
		typedef std::function<bool(const SDL_Event&)> Listener;

		/**
		 * @brief An identification for a listener
		 */
		typedef size_t ListenerIdentifier;

		/**
		 * @brief Adds a new event handler
		 * @param type The type of events to handle
		 * @param weigth The weight of this handler, used for determining in which order handlers are called.
		 * @param listener The listener that will be called
		 * @return An idfentification which can be used to remove this handler again
		 * @see removeEventListener()
		 */
		ListenerIdentifier addEventListener(SDL_EventType type, int weigth, const Listener& listener);

		/**
		 * @brief Removes the event handler with the given identifier
		 * @param identifier The identifier of the event handler that should be removed
		 * @return @c true if the event handler was removed, @c false otherwise
		 */
		bool removeEventListener(ListenerIdentifier identifier);

		/**
		* @brief Checks if the event belongs to a window
		* This can be used to handle multiple windows correctly and only handle the events
		* that belong to a specific window.
		*
		* @param e The event that should be checked
		* @param window The window the event should belong to
		* @return @c true if the event belongs to the window, @c false otherwise
		*/
		bool isWindowEvent(const SDL_Event& e, SDL_Window* window);
	}
	/** @} */ // end of OsAPI
}

// Documentation pages
/**
 * @page osapi_page The OS API
 * @subpage eventhandling_page
 */

/**
 * @page eventhandling_page OS Event handling API
 * The OS-API exposed a generic event handler interface to allow user code to consume SDL events. The handler will
 * be passed the SDL_Event structure and should return @c true if it handled the event or @c false if not. If it handled the event then the processing of this event will be stopped and it will not be delivered to subsequent
 * listeners.
 */

#endif // _OSAPI_H
