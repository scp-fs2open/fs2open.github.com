/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/crypt.cpp $
 * $Revision: 2.2 $
 * $Date: 2004-07-12 16:32:47 $
 * $Author: Kazan $
 *
 * Files for cypting stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 2     3/30/98 9:33p Allender
 * string encryption stuff for cheat codes
 *
 * $NoKeywords: $
 */

#include <string.h>
#include "globalincs/crypt.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

char *jcrypt (char *plainstring)
{
	int i,t,len;
	static char cryptstring[CRYPT_STRING_LENGTH + 1];

	len=strlen (plainstring);
	if (len > CRYPT_STRING_LENGTH)
		len = CRYPT_STRING_LENGTH;
   
	for (i = 0;i < len; i++) {
		cryptstring[i]=0; 

		for (t = 0; t < len; t++) {
			cryptstring[i]^=(plainstring[t] ^ plainstring[i%(t+1)]);
			cryptstring[i]%=90;
			cryptstring[i]+=33;
		}
	}

	cryptstring[i]=0;
	return ((char *)cryptstring);
}
