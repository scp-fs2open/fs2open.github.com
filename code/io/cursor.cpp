
#include <cmath>
#include <algorithm>

#include "cmdline/cmdline.h"
#include "bmpman/bmpman.h"
#include "io/cursor.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "popup/popup.h"
#include "popup/popupdead.h"

namespace
{
	SDL_Cursor* bitmapToCursor(int bitmapNum)
	{
		Assertion(bm_is_valid(bitmapNum), "%d is no valid bitmap handle!", bitmapNum);

		int w;
		int h;

		bm_get_info(bitmapNum, &w, &h, NULL, NULL);
		Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0x0000ff00;
		gmask = 0x00ff0000;
		bmask = 0xff000000;
		amask = 0x000000ff;
#else
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;
		amask = 0xff000000;
#endif

		SDL_Surface* bitmapSurface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
		SDL_LockSurface(bitmapSurface);
		bitmap* bmp = bm_lock(bitmapNum, 32, BMP_TEX_XPARENT);

		memcpy(bitmapSurface->pixels, reinterpret_cast<void*>(bmp->data), w * h * 4);

		bm_unlock(bitmapNum);
		SDL_UnlockSurface(bitmapSurface);

		// For now set the hot coordinates to the upper left corner
		SDL_Cursor* cursorHandle = SDL_CreateColorCursor(bitmapSurface, 0, 0);

		SDL_FreeSurface(bitmapSurface);

		return cursorHandle;
	}
}

namespace io
{
	namespace mouse
	{
		void Cursor::addFrame(SDL_Cursor* frame)
		{
			mAnimationFrames.push_back(frame);
		}

		void Cursor::setFPS(float fps)
		{
			this->mFps = fps;
		}

		void Cursor::enable()
		{
			if (mAnimationFrames.size() > 1)
			{
				// Animated, set the begin and do everything else in setCurrentFrame()
				mBeginTimeStamp = timestamp();
				mAnimationLength = static_cast<float>(mAnimationFrames.size()) / mFps;
				mLastFrame = static_cast<size_t>(-1);
			}
			else
			{
				// Not animated, just set the first frame
				SDL_SetCursor(mAnimationFrames.front());
			}
		}

		void Cursor::setCurrentFrame()
		{
			if (mAnimationFrames.size() > 1)
			{
				// We are animated, compute the current frame
				float diffSeconds = i2fl(timestamp() - mBeginTimeStamp) / TIMESTAMP_FREQUENCY;

				float progress = std::fmod(diffSeconds, mAnimationLength) / mAnimationLength;

				// This should round towards zero
				size_t frameIndex = static_cast<size_t>(mAnimationFrames.size() * progress);

				Assert(frameIndex < mAnimationFrames.size());

				if (mLastFrame != frameIndex)
				{
					SDL_SetCursor(mAnimationFrames[frameIndex]);
					mLastFrame = frameIndex;
				}
			}
		}

		void Cursor::releaseResources()
		{
			SCP_vector<SDL_Cursor*>::iterator iter;

			for (iter = mAnimationFrames.begin(); iter != mAnimationFrames.end(); ++iter)
			{
				SDL_FreeCursor(*iter);
			}

			mAnimationFrames.clear();
		}

		CursorManager* CursorManager::mSingleton = NULL;

		CursorManager::CursorManager() : mCursorShown(true), mCurrentCursor(NULL), mLoadedCursors(SCP_vector<Cursor*>())
		{
		}

		CursorManager::~CursorManager()
		{
			SCP_vector<Cursor*>::iterator iter;

			for (iter = mLoadedCursors.begin(); iter != mLoadedCursors.end(); ++iter)
			{
				(*iter)->releaseResources();
				delete *iter;
			}

			mLoadedCursors.clear();
		}

		Cursor* CursorManager::loadCursor(const char* fileName, bool animated)
		{
			int handle;
			int frameNum = 1;
			int fps = 0;

			if (animated)
			{
				handle = bm_load_animation(fileName, &frameNum, &fps);
			}
			else
			{
				handle = bm_load(fileName);
			}

			if (handle < 0)
			{
				mprintf(("Failed to load cursor bitmap %s!", fileName));
				return NULL;
			}

			Cursor* cursor = this->loadFromBitmap(handle);

			bm_release(handle);

			return cursor;
		}

		Cursor* CursorManager::loadFromBitmap(int bitmapHandle)
		{
			Assertion(bm_is_valid(bitmapHandle), "%d is no valid bitmap handle!", bitmapHandle);

			int nframes;
			int fps;

			bm_get_info(bitmapHandle, NULL, NULL, NULL, &nframes, &fps);

			Cursor* cursor = new Cursor();
			cursor->setFPS(i2fl(fps));

			for (int i = 0; i < nframes; ++i)
			{
				cursor->addFrame(bitmapToCursor(bitmapHandle + i));
			}

			mLoadedCursors.push_back(cursor);

			return cursor;
		}

		void CursorManager::setCurrentCursor(Cursor* cursor)
		{
			Assertion(cursor != NULL, "Invalud cursor pointer passed!");
			Assertion(std::find(mLoadedCursors.begin(), mLoadedCursors.end(), cursor) != mLoadedCursors.end(),
				"Cursor pointer is not in the loaded cursors vector!");

			mCurrentCursor = cursor;
			mCurrentCursor->enable();
		}

		void CursorManager::showCursor(bool show, bool grab)
		{
			if (show == mCursorShown && grab == mMouseGrabbed)
			{
				// Don't bother calling anithing if it's not going to change anything
				return;
			}

			if (show)
			{
				// If shown don't grab the mouse
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_ShowCursor(1);
			}
			else
			{
				if (grab)
				{
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				else
				{
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}

				SDL_ShowCursor(0);
			}

			mCursorShown = show;
			mMouseGrabbed = grab;
		}

		void CursorManager::init()
		{
			mSingleton = new CursorManager();

			// Load the default cursor and enable it
			Cursor* cursor = mSingleton->loadCursor("cursor", true);
			mSingleton->setCurrentCursor(cursor);

			// Hide the cursor initially
			mSingleton->showCursor(false);
		}

		void CursorManager::doFrame()
		{
			CursorManager* manager = get();

			int game_state = gameseq_get_state();

			switch (game_state) {
			case GS_STATE_GAME_PAUSED:
				// case GS_STATE_MULTI_PAUSED:
				// Don't grab the mouse when we are pausing, helps with debugging
				manager->showCursor(false);
				break;
			case GS_STATE_GAME_PLAY:
			case GS_STATE_DEATH_DIED:
			case GS_STATE_DEATH_BLEW_UP:
				if (popup_active() || popupdead_is_active())
				{
					manager->showCursor(true);
				}
				else
				{
					// Always grab the mouse when in gameplay
					manager->showCursor(false, true);
				}
				break;

			default:
				manager->showCursor(true);
				break;
			}	// end switch

			if (manager->mCurrentCursor != NULL && manager->mCursorShown)
			{
				manager->mCurrentCursor->setCurrentFrame();
			}
		}

		void CursorManager::shutdown()
		{
			delete mSingleton;
		}
	}
}
