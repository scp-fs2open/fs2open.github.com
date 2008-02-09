/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/crypt.h $
 * $Revision: 2.1 $
 * $Date: 2004-08-11 05:06:23 $
 * $Author: Kazan $
 *
 * Header for crypt stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     8/05/99 2:05a Dave
 * Fixes. Optimized detail level stuff.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 3     5/19/98 12:19p Mike
 * Cheat codes!
 * 
 * 2     3/30/98 9:33p Allender
 * string encryption stuff for cheat codes
 * 
 * 1     3/30/98 3:20p Allender
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _CRYPT_H
#define _CRYPT_H

// define for the length of a crypted string
#define CRYPT_STRING_LENGTH		17

char *jcrypt (char *plainstring);

#endif
