/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/scramble/scramble.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:28 $
 * $Author: penguin $
 *
 * Module for file scrambler
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     3/25/99 11:26a Dave
 * Beefed up encryption scheme so that even someone viewing the
 * disassembly would have a hard time cracking it.
 * 
 * 2     10/24/98 11:41p Dave
 * 
 * 1     10/24/98 11:31p Dave
 * 
 * 7     8/09/98 4:44p Lawrance
 * support alternate encryption scheme (doesn't pack chars into 7 bits)
 * 
 * 6     4/14/98 4:14p Lawrance
 * fix bug with ships and weapons tbl preprocessing
 * 
 * 5     4/14/98 1:39p Lawrance
 * Add command line switches to preprocess ship and weapon tables
 * 
 * 4     3/31/98 1:14a Lawrance
 * Get .tbl and mission file encryption working.
 * 
 * 3     3/30/98 5:57p Lawrance
 * add some comments
 * 
 * 2     3/30/98 5:51p Lawrance
 * file encryption and decryption
 *
 * $NoKeywords: $
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>

#include "encrypt.h"
#include "scramble.h"

#define MAX_LINE_LEN	512

static int Use_8bit = 0;	// set to 1 to disable 7bit character packing

// strip out any ships tbl data not used in demo (ie entries without @ preceding name)
void scramble_read_ships_tbl(char **text, int *text_len, FILE *fp)
{
	char	line[MAX_LINE_LEN+1];
	char	token_line[MAX_LINE_LEN+1];
	char	*dest;
	int	line_len, discard_line = 1, keep_all_lines = 0, post_discard = 0;
	char	seps[]   = " ,\t\n";
	char	*token;

	*text_len = _filelength(fileno(fp));
	*text = (char*)malloc(*text_len+1);

	dest = *text;

	while ( fgets(line, MAX_LINE_LEN, fp) != NULL ) {

		line_len = strlen(line);
		memcpy(token_line, line, line_len+1);

		if ( !keep_all_lines ) {
			token = strtok( token_line, seps );

			if ( token ) {
				if ( !strnicmp("#End", token, 4) ) {
					keep_all_lines = 1;
				} else if ( !strnicmp("#Ship", token, 5) ) {
					discard_line = 0;
					post_discard = 1;
				} else if ( !strnicmp("$Name:", token, 6) ) {
					token = strtok( NULL, seps );
					if ( token ) {
						if ( token[0] == '@' ) {
							discard_line = 0;
						} else {
							discard_line = 1;
						}
					}
				}
			}
		}

		if ( !discard_line || keep_all_lines ) {
			memcpy(dest, line, line_len);
			dest += line_len;
		}

		if ( post_discard ) {
			discard_line = 1;
			post_discard = 0;
		} 
	}

	*text_len = dest - *text;
}

// strip out any weapons tbl data not used in demo (ie entries without @ preceding name)
void scramble_read_weapons_tbl(char **text, int *text_len, FILE *fp)
{
	char	line[MAX_LINE_LEN+1];
	char	token_line[MAX_LINE_LEN+1];
	char	*dest;
	int	line_len, discard_line = 1, keep_all_lines = 0, post_discard = 0;
	char	seps[]   = " ,\t\n";
	char	*token = NULL;

	*text_len = _filelength(fileno(fp));
	*text = (char*)malloc(*text_len+1);

	dest = *text;

	while ( fgets(line, MAX_LINE_LEN, fp) != NULL ) {

		line_len = strlen(line);
		memcpy(token_line, line, line_len+1);

		if ( !keep_all_lines ) {
			token = strtok( token_line, seps );

			if ( token ) {
				if ( !strnicmp("#Countermeasures", token, 16) ) {
					keep_all_lines = 1;
				} else if ( !strnicmp("#End", token, 4) || !strnicmp("#Beam", token, 5) || !strnicmp("#Primary", token, 8) || !strnicmp("#Secondary", token, 10) ) {
					discard_line = 0;
					post_discard = 1;
				} else if ( !strnicmp("$Name:", token, 6) ) {
					discard_line = 1;
					token = strtok( NULL, seps );
					if ( token ) {
						if ( token[0] == '@' ) {
							discard_line = 0;
						}
					}
				}
			}
		}

		if ( (token[0] != ';') && (!discard_line || keep_all_lines) ) {
			memcpy(dest, line, line_len);
			dest += line_len;
		}

		if ( post_discard ) {
			discard_line = 1;
			post_discard = 0;
		} 
	}

	*text_len = dest - *text;
}

void scramble_read_default(char **text, int *text_len, FILE *fp)
{
	*text_len = _filelength(fileno(fp));
	*text = (char*)malloc(*text_len+1);
	fread( *text, *text_len, 1, fp );
}

// scramble a file
//
// input:	src_filename	=>	filename of text to scramble
//				dest_filename	=>	optional, this is the filename scrambled data will get stored to
void scramble_file(char *src_filename, char *dest_filename, int preprocess)
{
	FILE	*fp;
	int	text_len, scramble_len;
	char	*text, *scramble_text;

	fp = fopen(src_filename, "rb");
	if ( !fp ) {
		return;
	}

	// read in data, maybe preprocess
	switch(preprocess) {
	case PREPROCESS_SHIPS_TBL:
		scramble_read_ships_tbl(&text, &text_len, fp);
		break;
	case PREPROCESS_WEAPONS_TBL:
		scramble_read_weapons_tbl(&text, &text_len, fp);
		break;
	default:
		// read in the raw data
		scramble_read_default(&text, &text_len, fp);
		break;
	}

	fclose(fp);

	// open up file for writing scrambled text
	if ( dest_filename ) {
		fp = fopen(dest_filename, "wb");
	} else {
		fp = fopen(src_filename, "wb");
	}

	if ( !fp ) {
		return;
	}

	scramble_text = (char*)malloc(text_len+32);

	encrypt(text, text_len, scramble_text, &scramble_len, Use_8bit);
	
	// write out scrambled data
	fwrite( scramble_text, scramble_len, 1, fp );

	free(text);
	free(scramble_text);
	fclose(fp);
}

// unscramble a file
//
// input:	src_filename	=>	filename of scrambled text
//				dest_filename	=>	optional, this is the filename unscrambled text data will get stored to
void unscramble_file(char *src_filename, char *dest_filename)
{
	FILE	*fp;
	int	scramble_len, text_len;
	char	*text, *scramble_text;

	fp = fopen(src_filename, "rb");
	if ( !fp ) {
		return;
	}

	// read in the scrambled data
	scramble_len = _filelength(fileno(fp));
	scramble_text = (char*)malloc(scramble_len+1);
	fread( scramble_text, scramble_len, 1, fp );
	fclose(fp);

	// open up file for writing unscrambled text
	if ( dest_filename ) {
		fp = fopen(dest_filename, "wb");
	} else {
		fp = fopen(src_filename, "wb");
	}
	if ( !fp ) {
		return;
	}

	// assume original text no larger than double scrambled size
	text = (char*)malloc(scramble_len*2);

	unencrypt(scramble_text, scramble_len, text, &text_len);

	// write out unscrambled data
	fwrite( text, text_len, 1, fp );

	free(text);
	free(scramble_text);
	fclose(fp);
}

void print_instructions()
{
	printf("Encrypt: scramble [-st] [-wt] <filename_in> [filename_out] \n");
	printf("Decrypt: scramble -u <filename_in> [filename_out] \n");
}

void main(int argc, char *argv[])
{
	switch (argc) {
	case 2:
		encrypt_init();
		scramble_file(argv[1]);
		break;
	case 3:
		encrypt_init();
		if ( !stricmp("-u", argv[1]) ) {
			unscramble_file(argv[2]);
		} else if ( !stricmp("-st", argv[1]) ) {
			scramble_file(argv[2], argv[2], PREPROCESS_SHIPS_TBL);
		} else if ( !stricmp("-wt", argv[1]) ) {
			scramble_file(argv[2], argv[2], PREPROCESS_WEAPONS_TBL);
		} else {
			scramble_file(argv[1], argv[2]);
		}
		break;
	case 4:
		encrypt_init();
		if ( !stricmp("-u", argv[1]) ) {
			unscramble_file(argv[2], argv[3]);
		} else if ( !stricmp("-st", argv[1]) ) {
			scramble_file(argv[2], argv[3], PREPROCESS_SHIPS_TBL);
		} else if ( !stricmp("-wt", argv[1]) ) {
			scramble_file(argv[2], argv[3], PREPROCESS_WEAPONS_TBL);
		} else {
			print_instructions();
		}
		break;
	default:
		print_instructions();
		return;
	}
}

