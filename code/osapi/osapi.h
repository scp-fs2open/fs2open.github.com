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
#include "globalincs/flagset.h"

#include <functional>
#include <memory>

#include <SDL_events.h>

// --------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// set if running under MsDev - done after os_init(...) has returned
extern int Os_debugger_running;

#ifdef SCP_UNIX
const char* os_get_legacy_user_dir();
#endif

// --------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(const char * wclass, const char * title, const char * app_name = nullptr);

// set the main window title
void os_set_title( const char * title );

// call at program end
void os_cleanup();

// window management ---------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground();

// process management --------------------------------------------------------------

/**
 * @brief Defers all pending events to be handled later
 */
void os_defer_events_on_load_screen();

// removes key events that may unintentionally skip the cutscene
void os_remove_deferred_cutscene_key_events();

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
	 * @defgroup os_graphics_api Graphics operations abstraction
	 * @see @ref graphics_api_page
	 * @ingroup osapi
	 */

	/**
	 * @brief Flags for OpenGL context creation
	 * @ingroup os_graphics_api
	 */
	FLAG_LIST(OpenGLContextFlags)
	{
		Debug = 0,
		ForwardCompatible,

		NUM_VALUES
	};

	/**
	 * @brief The required context profile
	 * @ingroup os_graphics_api
	 */
	enum class OpenGLProfile
	{
		Core,
		Compatibility
	};

	/**
	 * @brief Attributes for OpenGL context creation
	 * @ingroup os_graphics_api
	 */
	struct OpenGLContextAttributes
	{
		uint32_t major_version; //!< The major version of the created context
		uint32_t minor_version; //!< The minor version of the created context

		flagset<OpenGLContextFlags> flags; //!< The OpenGL context flags
		OpenGLProfile profile; //!< The desired OpenGL profile
	};

	/**
	 * @brief Pixel format of a viewport
	 * @ingroup os_graphics_api
	 */
	struct ViewportPixelFormat {
		uint32_t red_size;
		uint32_t green_size;
		uint32_t blue_size;
		uint32_t alpha_size;

		uint32_t depth_size;
		uint32_t stencil_size;

		uint32_t multi_samples; //!< The amount of multi-sampling, use 0 for no multi-sampling
	};

	/**
	 * @brief A function pointer for loading an OpenGL function
	 * @ingroup os_graphics_api
	 */
	typedef void* (*OpenGLLoadProc)(const char *name);

	/**
	 * @brief An OpenGL context
	 * Can be deleted which will properly free the resources of the underlying OpenGL context
	 * @ingroup os_graphics_api
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
		 * @brief Sets the swap interval
		 */
		virtual bool setSwapInterval(int status) = 0;
	};

	/**
	 * @brief Flags for viewport creation
	 * @ingroup os_graphics_api
	 */
	FLAG_LIST(ViewPortFlags)
	{
		Fullscreen = 0,
		Borderless,
		Resizeable,

		NUM_VALUES
	};

	/**
	 * @brief Properties of a viewport that should be created
	 * @ingroup os_graphics_api
	 */
	struct ViewPortProperties
	{
		bool enable_opengl = false; //!< Set to true if the viewport should support OpenGL rendering
		OpenGLContextAttributes gl_attributes;

		bool enable_vulkan = false; //!< Set to true if the viewport should support Vulkan rendering

		ViewportPixelFormat pixel_format;

		SCP_string title;

		uint32_t width;
		uint32_t height;

		flagset<ViewPortFlags> flags;

		uint32_t display;
	};

	/**
	 * @brief State of a viewport
	 * @ingroup os_graphics_api
	 */
	enum class ViewportState {
		Windowed = 0,
		Borderless = 1,
		Fullscreen = 2
	};

	/**
	 * @brief A viewport supporting graphics operations
	 *
	 * A viewport is something that supports rendering operations. Typically this is a window but here it's more
	 * abstract.
	 *
	 * @ingroup os_graphics_api
	 */
	class Viewport
	{
	public:
		virtual ~Viewport() {}

		/**
		 * @brief Returns a SDL_Window handle for this viewport
		 *
		 * @note The returned handle is owned by the viewport and may not be destroyed by the caller.
		 *
		 * @return The window handle or @c nullptr if the viewport can't be represented as an SDL_Window
		 */
		virtual SDL_Window* toSDLWindow() = 0;

		/**
		 * @brief Gets the size of this viewport
		 *
		 * @note This is the actual window size. On HiDPI systems the size of the renderable area might be bigger if
		 * the window is created with support for that.
		 *
		 * @return A (width, height) pair
		 */
		virtual std::pair<uint32_t, uint32_t> getSize() = 0;

		/**
		 * @brief Swaps the buffers of this window
		 */
		virtual void swapBuffers() = 0;

		/**
		 * @brief Sets the window state of the viewport
		 *
		 * @note Implementation may ignore invocations of this function
		 *
		 * @param state The desired state
		 */
		virtual void setState(ViewportState state) = 0;

		/**
		 * @brief Minimizes the viewport
		 *
		 * @note Implementation may ignore invocations of this function
		 */
		virtual void minimize() = 0;

		/**
		 * @brief Restores/Maximizes the viewport
		 *
		 * @note Implementation may ignore invocations of this function
		 */
		virtual void restore() = 0;
	};

	/**
	 * @brief Abstraction for various graphics operations
	 * 
	 * This is used for providing platform specific functionality for various graphics operations.
	 *
	 * @ingroup os_graphics_api
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
		 * @warning The viewport must be configured to support OpenGL!
		 *
		 * @param viewport The viewport to create the context for.
		 *
		 * @return A pointer to the OpenGL context or @c nullptr if the creation has failed
		 */
		virtual std::unique_ptr<OpenGLContext> createOpenGLContext(Viewport* viewport,
																   const OpenGLContextAttributes& gl_attrs) = 0;

		/**
		 * @brief Makes an OpenGL context current
		 *
		 * @warning The viewport must be configured to support OpenGL!
		 *
		 * @param view The viewport to make the context current on
		 * @param ctx The OpenGL context to make current, may be @c nullptr
		 */
		virtual void makeOpenGLContextCurrent(Viewport* view, OpenGLContext* ctx) = 0;

		/**
		 * @brief Creates a new viewport
		 *
		 * @note Implementations may choose to dissallow viewport creation after a certain number of viewports are
		 * created. E.g. FRED may not want to create more than one viewport.
		 *
		 * @param props The desired properties of the new viewport.
		 * @return The created viewport, may be @c nullptr if the viewport can't be created
		 */
		virtual std::unique_ptr<Viewport> createViewport(const ViewPortProperties& props) = 0;
	};

	/**
	 * @brief Adds a viewport to the osapi list
	 *
	 * The osapi system will automatically free these viewports when it is shut down.
	 *
	 * @param viewport The viewport to add. The osapi system assumes ownership over the pointer
	 * @return The pointer of the viewport
	 *
	 * @ingroup os_graphics_api
	 */
	Viewport* addViewport(std::unique_ptr<Viewport>&& viewport);

	/**
	 * @brief Sets the main viewport of the application
	 * @param mainView The viewport to set
	 *
	 * @ingroup os_graphics_api
	 */
	void setMainViewPort(Viewport* mainView);

	/**
	 * @brief Gets the main viewport of the application
	 * @return The main viewport, can be @c nullptr if it hasn't been set yet
	 *
	 * @ingroup os_graphics_api
	 */
	Viewport* getMainViewport();

	/**
	 * @brief Gets the SDL handle of the main window
	 * @return The SDL handle or @c nullptr if there is no main window or if the main window can't be represented as
	 * an SDL window
	 *
	 * @ingroup os_graphics_api
	 */
	SDL_Window* getSDLMainWindow();

	void closeAllViewports();

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
 * @subpage graphics_api_page
 */

/**
 * @page eventhandling_page OS Event handling API
 * The OS-API exposed a generic event handler interface to allow user code to consume SDL events. The handler will
 * be passed the SDL_Event structure and should return @c true if it handled the event or @c false if not. If it handled the event then the processing of this event will be stopped and it will not be delivered to subsequent
 * listeners.
 */

/**
 * @page graphics_api_page OS Graphics API
 * Since multiple different applications need to use the FSO rendering engine it's necessary to provide an abstraction
 * of how an OpenGL context is created. Furthermore, since FSO may want to use multiple viewports it's also necessary to
 * provide an abstraction for that. This is done via the @ref GraphicsOperations class which must be implemented by users
 * of the FSO rendering engine. This class allows to create viewports and OpenGL contexts. First, a viewport will be
 * created which can be a standalone window or a part of a GUI. Then an OpenGL context will be created based on that
 * viewport which will be used for further rendering.
 *
 * @see @ref os_graphics_api
 */

#endif // _OSAPI_H
