/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalincs/crypt.h"


// we end up #include'ing SDL.h which on Windows and Mac will redfine main() which is something
// that we don't want since we don't actually link against SDL, this solves the problem...
#ifdef main
#undef main
#endif

int main(int argc, char **argv)
{
	int i;
	char *crypt_string;

	if ( argc == 1 ) {
		printf("Usage: cryptstring <string1> <string2> ...\n");
		printf("Output will be <crypt1> <crypt2>\n");
		exit(1);
	}

	for ( i = 1; i < argc; i++ ) {
		char *s;

		s = argv[i];
		// if the length of the string is greater than the number of crypted symbols we
		// return, then pass only the maximum length
		if ( strlen(s) > CRYPT_STRING_LENGTH )
			s += (strlen(s) - CRYPT_STRING_LENGTH);

		crypt_string = jcrypt(s);
		printf("%s\n", crypt_string);
	}

	return 0;
}

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
