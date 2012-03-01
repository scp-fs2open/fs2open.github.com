/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _FIX_H
#define _FIX_H

//#include "pstypes.h"
#ifdef _MSC_VER
	#include "globalincs/msvc/stdint.h"
#else
	#include <stdint.h>
#endif

#define F1_0 65536
#define f1_0 65536

fix fixmul(fix a, fix b);
fix fixdiv(fix a, fix b);
fix fixmuldiv(fix a, fix b, fix c);

static inline uint32_t fixTimeToMs(const fix time) {
	uint64_t temp = time;
	temp = temp * 1000;
	temp = temp >> 16;
	return (uint32_t) temp;
}

static inline fix fixTimeFromMs(const uint32_t ms) {
	uint64_t temp = ms;
	temp = temp << 16;
	temp = temp / 1000;
	return (fix) temp;
}

#define f2i(a) ((int)((a)>>16))
#define i2f(a) ((fix)((a)<<16))

#endif
