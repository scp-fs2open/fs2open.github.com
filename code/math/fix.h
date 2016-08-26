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

#define F1_0 65536
#define f1_0 65536

fix fixmul(fix a, fix b);
fix fixdiv(fix a, fix b);
fix fixmuldiv(fix a, fix b, fix c);

// Needed for compatibility with MSVC 2013
#if SCP_COMPILER_CXX_CONSTEXPR
#define CONST_EXPR constexpr
#else
#define CONST_EXPR inline
#endif

CONST_EXPR int f2i(fix a) {
	return static_cast<int>(a >> 16);
}

CONST_EXPR fix i2f(int a) {
	return static_cast<fix>(a << 16);
}

#undef CONST_EXPR

#endif
