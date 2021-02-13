//
// This file is part of the Terathon Common Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#include "TSTools.h"


using namespace Terathon;


#ifdef TERATHON_DEBUG

	#if defined(_MSC_VER) || defined(__ORBIS__) || defined(__PROSPERO__)

		void Terathon::Fatal(const char *message)
		{
			__debugbreak();
		}

	#elif defined(__GNUC__)

		extern "C"
		{
			int raise(int);
		}

		void Terathon::Fatal(const char *message)
		{
			raise(5);	// SIGTRAP
		}

	#endif

	void Terathon::Assert(bool condition, const char *message)
	{
		if (!condition)
		{
			Fatal(message);
		}
	}

#endif
