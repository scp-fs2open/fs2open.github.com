/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <string.h>
#include "globalincs/crypt.h"



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
