
#include <cmath>
#include <algorithm>

#include "cmdline/cmdline.h"
#include "bmpman/bmpman.h"
#include "io/cursor.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "popup/popup.h"
#include "popup/popupdead.h"

#include <utility>
#include <globalincs/systemvars.h>

namespace
{
	SDL_Cursor* bitmapToCursor(int bitmapNum)
	{
		Assertion(bm_is_valid(bitmapNum), "%d is no valid bitmap handle!", bitmapNum);

		int w;
		int h;

		bm_get_info(bitmapNum, &w, &h, nullptr, nullptr);
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

	void setRelativeMouseMode(bool grab) {
		if (Cmdline_nograb) {
			// Never grab the mouse if this is enabled
			SDL_SetRelativeMouseMode(SDL_FALSE);
		} else {
			SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
		}
	}
	
	void changeMouseStatus(bool show, bool grab)
	{
		if (show)
		{
			// If shown don't grab the mouse
			setRelativeMouseMode(false);
			SDL_ShowCursor(1);
		}
		else
		{
			if (grab)
			{
				setRelativeMouseMode(true);
			}
			else
			{
				setRelativeMouseMode(false);
			}

			SDL_ShowCursor(0);
		}
	}
}

namespace io
{
	namespace mouse
	{

		Cursor::Cursor(Cursor&& other)
		{
			*this = std::move(other);
		}
		
		Cursor& Cursor::operator=(Cursor&& other)
		{
			std::swap(this->mAnimationFrames, other.mAnimationFrames);
			
			this->mBitmapHandle = other.mBitmapHandle;
			this->mBeginTimeStamp = other.mBeginTimeStamp;
			this->mLastFrame = other.mLastFrame;
			
			other.mBitmapHandle = -1;
			other.mBeginTimeStamp= -1;
			other.mLastFrame = static_cast<size_t>(-1);
			
			return *this;
		}

		Cursor::~Cursor()
		{
			SCP_vector<SDL_Cursor*>::iterator iter;

			for (iter = mAnimationFrames.begin(); iter != mAnimationFrames.end(); ++iter)
			{
				SDL_FreeCursor(*iter);
			}

			mAnimationFrames.clear();
			
			// Free cursor
			bm_release(mBitmapHandle);
			mBitmapHandle = -1;
		}
			
		void Cursor::addFrame(SDL_Cursor* frame)
		{
			mAnimationFrames.push_back(frame);
		}

		void Cursor::enable()
		{
			if (mAnimationFrames.size() > 1)
			{
				// Animated, set the begin and do everything else in setCurrentFrame()
				mBeginTimeStamp = timestamp();
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

				// Use the bmpman function for this. That also ensures that APNG cursors work correctly 
				auto frameIndex = static_cast<size_t>(bm_get_anim_frame(mBitmapHandle, diffSeconds, 0.0f, true));

				Assert(frameIndex < mAnimationFrames.size());

				if (mLastFrame != frameIndex)
				{
					SDL_SetCursor(mAnimationFrames[frameIndex]);
					mLastFrame = frameIndex;
				}
			}
		}

		CursorManager* CursorManager::mSingleton = nullptr;

		CursorManager::CursorManager() : mCurrentCursor(nullptr)
		{
			mStatusStack.push_back(std::make_pair(true, false));
		}

		CursorManager::~CursorManager()
		{
		}

		Cursor* CursorManager::loadCursor(const char* fileName, bool animated)
		{
			Assertion(!Is_standalone, "Cursors can't be loaded in standalone mode!");

			int handle;

			if (animated)
			{
				handle = bm_load_animation(fileName, nullptr, nullptr);
			}
			else
			{
				handle = bm_load(fileName);
			}

			if (handle < 0)
			{
				mprintf(("Failed to load cursor bitmap %s!", fileName));
				return nullptr;
			}

			Cursor* cursor = this->loadFromBitmap(handle);

			return cursor;
		}

		Cursor* CursorManager::loadFromBitmap(int bitmapHandle)
		{
			Assertion(bm_is_valid(bitmapHandle), "%d is no valid bitmap handle!", bitmapHandle);

			int nframes;
			int fps;

			bm_get_info(bitmapHandle, nullptr, nullptr, nullptr, &nframes, &fps);

			std::unique_ptr<Cursor> cursor(new Cursor(bitmapHandle));

			for (int i = 0; i < nframes; ++i)
			{
				cursor->addFrame(bitmapToCursor(bitmapHandle + i));
			}

			mLoadedCursors.push_back(std::move(cursor));

			return mLoadedCursors.back().get();
		}

		void CursorManager::setCurrentCursor(Cursor* cursor)
		{
			Assertion(cursor, "Invalid cursor pointer passed!");

			mCurrentCursor = cursor;
			mCurrentCursor->enable();
		}

		void CursorManager::showCursor(bool show, bool grab)
		{
			auto current = mStatusStack.back();
			if (show == current.first && grab == current.second)
			{
				// Don't bother calling anithing if it's not going to change anything
				return;
			}

			changeMouseStatus(show, grab);

			mStatusStack.back() = std::make_pair(show, grab);
		}
			
		void CursorManager::pushStatus()
		{
			// Copy the last status into the top
			mStatusStack.push_back(mStatusStack.back());
		}
		
		std::pair<bool, bool> CursorManager::popStatus()
		{
			Assertion(mStatusStack.size() > 1, "Can't pop the last status!");
			
			auto current = mStatusStack.back();
			
			mStatusStack.pop_back();
			
			auto newState = mStatusStack.back();
			changeMouseStatus(newState.first, newState.second);
			
			return current;
		}

		void CursorManager::init()
		{
			mSingleton = new CursorManager();

			// Hide the cursor initially
			mSingleton->showCursor(false);
		}

		void CursorManager::doFrame()
		{
			CursorManager* manager = get();

			if (manager->mCurrentCursor != nullptr && manager->isCursorShown())
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
