/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/fix.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:24 $
 * $Author: penguin $
 *
 * Routines for fixed point math
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 4     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#ifndef _FIX_H
#define _FIX_H

//#include "pstypes.h"

#define F1_0 65536
#define f1_0 65536

fix fixmul(fix a, fix b);
fix fixdiv(fix a, fix b);
fix fixmuldiv(fix a, fix b, fix c);

#define f2i(a) ((int)((a)>>16))
#define i2f(a) ((fix)((a)<<16))

#endif
