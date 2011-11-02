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

#define F1_0 65536
#define f1_0 65536

fix fixmul(fix a, fix b);
fix fixdiv(fix a, fix b);
fix fixmuldiv(fix a, fix b, fix c);

static inline int f2i(const fix value) { return (value >> 16); }
static inline fix i2f(const int value) { return (value << 16); }

static inline fix i2f(const unsigned short value, const unsigned short fractionalPart)
{
	return i2f(value) + fractionalPart;
}

#endif
