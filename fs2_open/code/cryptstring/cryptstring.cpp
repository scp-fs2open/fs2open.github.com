/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Cryptstring/cryptstring.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:04 $
 * $Author: penguin $
 *
 * Applet for crypting strings.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/23/98 6:21p Dave
 *
 * $NoKeywords: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalincs/crypt.h"

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
