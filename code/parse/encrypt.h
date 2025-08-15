/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include <cstdint>
#include "globalincs/vmallocator.h"

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

//A fast platform/std-implementation stable hashing algorithm. Implements the FNV-1a hash algorithm
constexpr uint32_t hash_fnv1a(const uint8_t* bytes, size_t length) {
	const uint32_t fnv1a_magic_prime = 16777619;
	uint32_t hash = 2166136261;

	for (size_t cnt = 0; cnt < length; cnt++) {
		hash = (hash ^ bytes[cnt]) * fnv1a_magic_prime;
	}

	return hash;
}

uint32_t hash_fnv1a(const SCP_string& string);
uint32_t hash_fnv1a(const void* data, size_t length);
constexpr uint32_t hash_fnv1a(uint32_t value) {
	uint8_t bytes[4] = {static_cast<uint8_t>((value) & 0xff),
						static_cast<uint8_t>((value >> 8) & 0xff),
						static_cast<uint8_t>((value >> 16) & 0xff),
						static_cast<uint8_t>((value >> 24) & 0xff)};
	return hash_fnv1a(bytes, sizeof(uint32_t));
}

#endif

