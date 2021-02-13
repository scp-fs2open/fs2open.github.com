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


#include "TSHalf.h"


using namespace Terathon;


namespace
{
	// Indexed by the high 6 bits of a 16-bit float, this table contains the sign and exponent
	// for the corresponding 32-bit float as well as a mantissa mask that flushes denorms to
	// zero and clears the mantissa bits for any infinities.

	alignas(128) const uint32 halfTable[64] =
	{
		0x00000000, 0x38FFFFFF, 0x397FFFFF, 0x39FFFFFF, 0x3A7FFFFF, 0x3AFFFFFF, 0x3B7FFFFF, 0x3BFFFFFF, 0x3C7FFFFF, 0x3CFFFFFF, 0x3D7FFFFF, 0x3DFFFFFF, 0x3E7FFFFF, 0x3EFFFFFF, 0x3F7FFFFF, 0x3FFFFFFF,
		0x407FFFFF, 0x40FFFFFF, 0x417FFFFF, 0x41FFFFFF, 0x427FFFFF, 0x42FFFFFF, 0x437FFFFF, 0x43FFFFFF, 0x447FFFFF, 0x44FFFFFF, 0x457FFFFF, 0x45FFFFFF, 0x467FFFFF, 0x46FFFFFF, 0x477FFFFF, 0x7F800000,
		0x80000000, 0xB8FFFFFF, 0xB97FFFFF, 0xB9FFFFFF, 0xBA7FFFFF, 0xBAFFFFFF, 0xBB7FFFFF, 0xBBFFFFFF, 0xBC7FFFFF, 0xBCFFFFFF, 0xBD7FFFFF, 0xBDFFFFFF, 0xBE7FFFFF, 0xBEFFFFFF, 0xBF7FFFFF, 0xBFFFFFFF,
		0xC07FFFFF, 0xC0FFFFFF, 0xC17FFFFF, 0xC1FFFFFF, 0xC27FFFFF, 0xC2FFFFFF, 0xC37FFFFF, 0xC3FFFFFF, 0xC47FFFFF, 0xC4FFFFFF, 0xC57FFFFF, 0xC5FFFFFF, 0xC67FFFFF, 0xC6FFFFFF, 0xC77FFFFF, 0xFF800000
	};
}


float Half::GetFloat(void) const
{
	uint32 h = value;
	uint32 i = halfTable[h >> 10];
	uint32 f = ((h << 13) | 0xFF800000) & i;
	return (asfloat(f));
}

void Half::SetFloat(float v) volatile
{
	uint32 f = asuint(v);
	uint32 s = (f >> 16) & 0x8000;
	int32 e = int32((f >> 23) & 0xFF) - 127;

	if (e >= -14)
	{
		// Add one to the 11th bit in the mantissa to perform proper rounding.

		uint32 m = (((f >> 12) & 0x07FF) + 1) >> 1;
		e += m >> 10;

		if (e <= 15)
		{
			value = uint16(s | ((e + 15) << 10) | (m & 0x03FF));
		}
		else
		{
			value = uint16(s | 0x7C00);
		}
	}
	else
	{
		value = uint16(s);
	}
}
