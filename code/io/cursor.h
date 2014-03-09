
#ifndef CURSOR_H
#define CURSOR_H
#pragma once

#include <SDL_mouse.h>

#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"

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
			SCP_vector<SDL_Cursor*> mAnimationFrames; //! The individual frames

			float mFps; //! The FPS of the animation, unused if only one frame

			int mBeginTimeStamp; //! The timestamp when the animation was started, unused when not animated
			float mAnimationLength; //! The length (in seconds) of the animation
			size_t mLastFrame; //! The last frame which was set
		public:
			/**
			 * @brief Default constructor
			 */
			Cursor() : mFps(-1.0f), mBeginTimeStamp(-1), mLastFrame(static_cast<size_t>(-1)) {}

			/**
			 * @brief Adds an animation frame
			 *
			 * @param frame The cursor frame to add
			 */
			void addFrame(SDL_Cursor* frame);

			/**
			 * @brief Sets the FPS of the animation
			 * Make sure you set this before enabling the cursor so the values are correctly initialized
			 *
			 * @param fps The frames per second
			 */
			void setFPS(float fps);

			/**
			 * @brief Called to enable the cursor
			 * Either this function sets the cursor directly, or initializes values to #setCurrentFrame() works correctly
			 */
			void enable();

			/**
			 * @brief Called to set the correct frame
			 */
			void setCurrentFrame();

			/**
			 * Releases all SDL handles
			 */
			void releaseResources();
		};

		/**
		 * @brief Manages the cursor state of the game
		 */
		class CursorManager
		{
		private:
			static CursorManager* mSingleton; //! The singleton manager

			SCP_vector<Cursor*> mLoadedCursors; //! A list of loaded cursors

			Cursor* mCurrentCursor; //! The current cursor

			bool mCursorShown; //! @c true of cursor is shown or @c false if not
			bool mMouseGrabbed; //! @c true if the mouse is grabbed, @c false if not

			/**
			 * @brief Default constructor
			 *
			 * This class should not be instantiated outside from this module
			 */
			CursorManager();
		public:

			/**
			 * Releases the cursor resources
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
			bool isCursorShown() { return mCursorShown; }

			/**
			 * @brief Gets the current cursor
			 * @return The current cursor instance
			 */
			Cursor* getCurrentCursor() { return mCurrentCursor; }

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
