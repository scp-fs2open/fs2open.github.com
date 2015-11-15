
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
	
	void changeMouseStatus(bool show, bool grab)
	{
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
			
			this->mFps = other.mFps;
			this->mBeginTimeStamp = other.mBeginTimeStamp;
			this->mAnimationLength = other.mAnimationLength;
			this->mLastFrame = other.mLastFrame;
			
			other.mFps = -1.0f;
			other.mBeginTimeStamp= -1;
			other.mAnimationLength = -1.f;
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
		}
			
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
				return nullptr;
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

			bm_get_info(bitmapHandle, nullptr, nullptr, nullptr, &nframes, &fps);

			std::unique_ptr<Cursor> cursor(new Cursor());
			cursor->setFPS(i2fl(fps));

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
