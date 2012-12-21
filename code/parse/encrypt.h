/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



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

