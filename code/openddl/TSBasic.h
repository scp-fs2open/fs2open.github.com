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


#ifndef TSBasic_h
#define TSBasic_h


#include "TSPlatform.h"


#if defined(_MSC_VER)

	extern "C"
	{
		unsigned char _BitScanReverse(unsigned long *, unsigned long);
		#pragma intrinsic(_BitScanReverse)
	}

#endif


namespace Terathon
{
	inline int32 Abs(int32 x)
	{
		int32 a = x >> 31;
		return ((x ^ a) - a);
	}

	inline int64 Abs64(int64 x)
	{
		int64 a = x >> 63;
		return ((x ^ a) - a);
	}


	inline int32 Sgn(int32 x)
	{
		return ((x >> 31) - (-x >> 31));
	}

	inline int64 Sgn64(int64 x)
	{
		return ((x >> 63) - (-x >> 63));
	}


	inline int32 Min(int32 x, int32 y)
	{
		return ((x < y) ? x : y);
	}

	inline int64 Min64(int64 x, int64 y)
	{
		return ((x < y) ? x : y);
	}

	inline int32 Max(int32 x, int32 y)
	{
		return ((x > y) ? x : y);
	}

	inline int64 Max64(int64 x, int64 y)
	{
		return ((x > y) ? x : y);
	}

	inline int32 MinZero(int32 x)
	{
		return (x & (x >> 31));
	}

	inline int64 MinZero64(int64 x)
	{
		return (x & (x >> 63));
	}

	inline int32 MaxZero(int32 x)
	{
		return (x & ~(x >> 31));
	}

	inline int64 MaxZero64(int64 x)
	{
		return (x & ~(x >> 63));
	}


	template <int32 mod>
	inline int32 IncMod(int32 x)
	{
		return ((x + 1) & ((x - (mod - 1)) >> 31));
	}

	template <int32 mod>
	inline int32 DecMod(int32 x)
	{
		x--;
		return (x + ((x >> 31) & mod));
	}

	inline int32 OverflowZero(int32 x, int32 y)
	{
		return (x & ((x - y) >> 31));
	}


	template <typename type>
	inline void Exchange(type& x, type& y)
	{
		x ^= y;
		y ^= x;
		x ^= y;
	}

	template <>
	inline void Exchange<float>(float& x, float& y)
	{
		float f = x;
		x = y;
		y = f;
	}


	inline int32 Cntlz(uint32 n)
	{
		#if defined(_MSC_VER)

			unsigned long	x;

			if (_BitScanReverse(&x, n) == 0)
			{
				return (32);
			}

			return (31 - x);

		#else

			return ((n != 0) ? __builtin_clz(n) : 32);

		#endif
	}


	inline int32 IntLog2(uint32 n)
	{
		return (31 - Cntlz(n));
	}

	inline uint32 Pow2Floor(uint32 n)
	{
		return (0x80000000U >> Cntlz(n));
	}

	inline uint32 Pow2Ceil(uint32 n)
	{
		return (uint32(1 << (32 - Cntlz(n - 1))));
	}
}


#endif
