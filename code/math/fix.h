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

#include "globalincs/pstypes.h"

#define F1_0 65536
#define f1_0 65536

fix fixmul(fix a, fix b);
fix fixdiv(fix a, fix b);
fix fixmuldiv(fix a, fix b, fix c);

constexpr int f2i(fix a) {
	return static_cast<int>(a >> 16);
}

constexpr fix i2f(int a) {
	return static_cast<fix>(a << 16);
}

#endif
