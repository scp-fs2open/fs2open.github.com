/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/parse/Encrypt.h $
 * $Revision: 2.3 $
 * $Date: 2005-04-15 11:35:18 $
 * $Author: taylor $
 *
 * Header for encryption code common to FreeSpace and related tools
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/08/11 05:06:31  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2003/09/28 21:22:58  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     3/25/99 11:26a Dave
 * Beefed up encryption scheme so that even someone viewing the
 * disassembly would have a hard time cracking it.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 4     8/09/98 4:44p Lawrance
 * support alternate encryption scheme (doesn't pack chars into 7 bits)
 * 
 * 3     3/31/98 4:57p Lawrance
 * Add signature at the beginning of encrypted files
 * 
 * 2     3/31/98 1:14a Lawrance
 * Get .tbl and mission file encryption working.
 * 
 * 1     3/30/98 11:02p Lawrance
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__

// initialize encryption
void encrypt_init();

// Return 1 if the file is encrypted, otherwise return 0
int is_encrypted(char *scrambled_text);

// Returns 1 if the data uses one of the FS1 style encryptions, 0 if FS2 style
int is_old_encrypt(char *scrambled_text);

// return text description of the encrypted text type
const char *encrypt_type(char *scrambled_text);

// Encrypt text data
void encrypt(char *text, int text_len, char *scrambled_text, int *scrambled_len, int use_8bit, bool new_encrypt = true);

// Decrypt scrambled_text
void unencrypt(char *scrambled_text, int scrambled_len, char *text, int *text_len);

#endif

