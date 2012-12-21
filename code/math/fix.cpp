/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#ifdef _WIN32
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "math/fix.h"



fix fixmul(fix a, fix b)
{
	longlong tmp;
	tmp = (longlong)a * (longlong)b;
	return (fix)(tmp>>16);
}

fix fixdiv(fix a, fix b)
{
	return MulDiv(a,65536,b);
}

fix fixmuldiv(fix a, fix b,fix c)
{
	return MulDiv(a,b,c);
}
