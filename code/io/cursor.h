
#ifndef CURSOR_H
#define CURSOR_H
#pragma once

#include <SDL_mouse.h>

#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "io/timer.h"

#include <memory>

namespace io
{
	namespace mouse
	{
		/**
		 * @brief A mouse cursor
		 *
		 * This class holds the SDL_Cursor objects which represent the individual animation frames
		 */
		class Cursor
		{
		private:
			SCP_vector<SDL_Cursor*> mAnimationFrames; //!< The individual frames

			int mBitmapHandle; //!< The bitmap of this cursor

			UI_TIMESTAMP mBeginTimeStamp; //!< The UI timestamp when the animation was started, unused when not animated
			size_t mLastFrame; //!< The last frame which was set
			
			Cursor(const Cursor&); // Not implemented
			Cursor& operator=(const Cursor&); // Not implemented
		public:
			/**
			 * @brief Default constructor
			 * @param bitmap The bitmap handle of the cursor. The cursor takes ownership over this handle
			 */
			explicit Cursor(int bitmap) : mBitmapHandle(bitmap), mBeginTimeStamp(), mLastFrame(static_cast<size_t>(-1)) {}

			/**
			 * @brief Move constructor
			 *
			 * Transfers SDL resources to this newly constructed object
			 */
			Cursor(Cursor&& other) noexcept;
			
			/**
			 * @brief Move operator
			 *
			 * Transfers SDL resources to this object
			 */
			Cursor& operator=(Cursor&& other) noexcept;

			/**
			 * @brief Cursor destructor
			 *
			 * Frees the allocated SDL cursors
			 */
			~Cursor();
			
			/**
			 * @brief Adds an animation frame
			 *
			 * @param frame The cursor frame to add
			 */
			void addFrame(SDL_Cursor* frame);

			/**
			 * @brief Called to enable the cursor
			 * Either this function sets the cursor directly, or initializes values to #setCurrentFrame() works correctly
			 */
			void enable();

			/**
			 * @brief Called to set the correct frame
			 */
			void setCurrentFrame();
		};

		/**
		 * @brief Manages the cursor state of the game
		 */
		class CursorManager
		{
		private:
			static CursorManager* mSingleton; //!< The singleton manager

			SCP_vector<std::unique_ptr<Cursor>> mLoadedCursors; //!< A list of loaded cursors

			Cursor* mCurrentCursor; //!< The current cursor

			SCP_vector<std::pair<bool, bool>> mStatusStack; //!< Contains the stack of saved mouse statuses

			/**
			 * @brief Default constructor
			 *
			 * @note This class should not be instantiated outside from this module
			 */
			CursorManager();
		public:
			/**
			 * @brief Releases the cursor resources
			 */
			~CursorManager();

			/**
			 * @brief Loads a cursor
			 *
			 * Loads the specified file name as a cursor, by default this will try to load an animated cursor
			 *
			 * @param fileName The file name
			 * @param animated @c true to also load animated images
			 * @return The cursor or @c NULL if the process failed
			 */
			Cursor* loadCursor(const char* fileName, bool animated = true);

			/**
			 * @brief Loads a cursor from a bitmap handle
			 * The returned cursor takes ownership over the passed handle.
			 *
			 * @param bitmapHandle The bitmap handle, must be a valid handle
			 * @return The new cursor
			 */
			Cursor* loadFromBitmap(int bitmapHandle);

			/**
			 * @brief Sets the current cursor
			 * @param cursor The cursor instance to be set, may not be @c NULL
			 */
			void setCurrentCursor(Cursor* cursor);

			/**
			 * @brief Show or hide the cursor.
			 *
			 * Optionally @c grab may be specified to grab or not grab the mouse when the cursor is hidden
			 *
			 * @param show @c true to show @c false to hide
			 * @param grab @c true to grab the mouse @c false to disable grabbing
			 */
			void showCursor(bool show, bool grab = false);

			/**
			 * @brief Specifies if the cursor is shown
			 * @return @c true if shown ,@c false otherwise
			 */
			bool isCursorShown() { return mStatusStack.back().first; }

			/**
			 * @brief Gets the current cursor
			 * @return The current cursor instance
			 */
			Cursor* getCurrentCursor() { return mCurrentCursor; }
			
			/**
			 * @brief Pushes the current status onto the stack as a new entry
			 */
			void pushStatus();
			
			/**
			 * @brief Removes the top status from the stack and restores the previous
			 * @returns The removed state
			 */
			std::pair<bool, bool> popStatus();

		public:
			/**
			 * @brief Gets the global CursorManager
			 * @return The CursorManager
			 */
			static CursorManager* get() { return mSingleton; }

			/**
			 * @brief Initializes the Cursor system
			 */
			static void init();

			/**
			 * @brief Do a cursor frame
			 * This is needed for animated cursors
			 */
			static void doFrame();

			/**
			 * @brief Releases cursor resources
			 */
			static void shutdown();
		};
	}
}

#endif
