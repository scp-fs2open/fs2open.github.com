/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/Fix.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-29 20:12:31 $
 * $Author: penguin $
 *
 * Code to deal with 16.16 fixed point numbers.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 2     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */


#ifdef _WIN32
#include <windows.h>
#endif

#include "pstypes.h"
#include "fix.h"

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
