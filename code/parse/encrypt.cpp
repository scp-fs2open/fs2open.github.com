/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define	ENCRYPT_NEW													// new, better encryption scheme

#include "globalincs/pstypes.h"
#include "parse/encrypt.h"



#if BYTE_ORDER == BIG_ENDIAN
const uint Encrypt_new_signature			= 0x551a335c;		// new encrpytion
const uint Encrypt_signature				= 0xefbeadde;		// full encryption
#else
const uint Encrypt_new_signature			= 0x5c331a55;		// new encryption
const uint Encrypt_signature				= 0xdeadbeef;		// full encryption
#endif
const uint Encrypt_signature_8bit		= 0xcacacaca;		// light encryption - doesn't use 7bit chars

int Encrypt_inited = 0;

// new encryption
void encrypt_new(char *text, int text_len, char *scrambled_text, int *scrambled_len);
void unencrypt_new(char *scrambled_text, int scrambled_len, char *text, int *text_len);

// update cur_seed with the chksum of the new_data of size new_data_size
ushort chksum_add_short(ushort seed, char *buffer, int size)
{
	ubyte * ptr = (ubyte *)buffer;
	unsigned int sum1,sum2;

	sum1 = sum2 = (int)(seed);

	while(size--)	{
		sum1 += *ptr++;
		if (sum1 >= 255 ) sum1 -= 255;
		sum2 += sum1;
	}
	sum2 %= 255;
	
	return (unsigned short)((sum1<<8)+ sum2);
}

// scramble text data
//
// input:	text		=>	ascii data to be scrambled
//				text_len	=>	number of bytes of ascii data to scramble
//				scrambled_text	=>	storage for scrambled text (malloc at least text_len)
//				scrambled_len	=>	size of text after getting scrambled
//				use_8bit => flag to indicate that chars are stored using 8 bits (default value is 0)
//				new_encrypt => flag to switch between old and new encryption styles (default = yes)
void encrypt(char *text, int text_len, char *scrambled_text, int *scrambled_len, int use_8bit, bool new_encrypt)
{
#ifdef ENCRYPT_NEW
	// new_encrypt, when set, is for FS2 style encryption
	// when not set it's either of two FS1 styles, 8bit or 7bit
	if (new_encrypt == true) {
		encrypt_new(text, text_len, scrambled_text, scrambled_len);
		return;
	}
#endif

	int i;
	int byte_offset = 0;
	int bit_offset = 0;

	*scrambled_len = 0;

	// Identify encrypted files with a unique signature
	if (use_8bit) {
		memcpy(scrambled_text, &Encrypt_signature_8bit, 4);
	} else {
		memcpy(scrambled_text, &Encrypt_signature, 4);
	}
	byte_offset = 4;

	// First stage: packing chars into 7 bit boundries
	for ( i =0; i < text_len; i++ ) {

		// account for wacky apostrophe that has ascii code 0x92
		if ( (unsigned char)text[i] == 0x92 ) {
			text[i] = 0x27;
		}

		if (use_8bit) {
			scrambled_text[byte_offset++] = text[i];
		} else {
			switch(bit_offset) {
			case 0:
				scrambled_text[byte_offset] = (char)((text[i] << 1) & 0xfe);
				bit_offset = 7;
				break;
			case 1:
				scrambled_text[byte_offset] &= 0x80;						// clear out bottom 7 bits
				scrambled_text[byte_offset] |= (text[i] & 0x7F);	
				byte_offset++;
				bit_offset = 0;
				break;
			case 2:
				scrambled_text[byte_offset] &= 0xc0;										// clear out bottom 6 bits
				scrambled_text[byte_offset] |= ((text[i] >> 1) & 0x3F);				// put in top 6 bits
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 7) & 0x80);		// put in last bit
				bit_offset = 1;
				break;
			case 3:
				scrambled_text[byte_offset] &= 0xe0;										// clear out bottom 5 bits
				scrambled_text[byte_offset] |= ((text[i] >> 2) & 0x1F);				// put in top 5 bits
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 6) & 0xc0);		// put in last two bits
				bit_offset = 2;
				break;
			case 4:
				scrambled_text[byte_offset] &= 0xf0;										// clear out bottom 4 bits
				scrambled_text[byte_offset] |= ((text[i] >> 3) & 0x0F);				// put in top 4 bits
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 5) & 0xe0);		// put in last three bits
				bit_offset = 3;
				break;
			case 5:
				scrambled_text[byte_offset] &= 0xf8;										// clear out bottom 3 bits
				scrambled_text[byte_offset] |= ((text[i] >> 4) & 0x07);				// put in top 3 bits
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 4) & 0xf0);		// put in last four bits
				bit_offset = 4;
				break;
			case 6:
				scrambled_text[byte_offset] &= 0xfc;										// clear out bottom 2 bits
				scrambled_text[byte_offset] |= ((text[i] >> 5) & 0x03);				// put in top 2 bits
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 3) & 0xf8);		// put in last five bits
				bit_offset = 5;
				break;
			case 7:
				scrambled_text[byte_offset] &= 0xfe;										// clear out bottom bit
				scrambled_text[byte_offset] |= ((text[i] >> 6) & 0x01);				// put in top bit
				byte_offset++;
				scrambled_text[byte_offset] = (char)((text[i] << 2) & 0xfc);		// put in last six bits
				bit_offset = 6;
				break;
			default:
				return;
			}
		}
	}

	if ( bit_offset > 0 ) {
		byte_offset++;
	}

	*scrambled_len = byte_offset;

	// Second stage: XOR with offset into file (skip signature)
	scrambled_text += 4;
	int len = *scrambled_len - 4;
	for ( i =0; i < len; i++ ) {
		scrambled_text[i] ^= i;
	}
}

//	input:	scrambled_text	=>	scrambled text
//				scrambled_len	=>	number of bytes of scrambled text
//				text				=>	storage for unscrambled ascii data
//				text_len			=>	actual number of bytes of unscrambled data
void unencrypt(char *scrambled_text, int scrambled_len, char *text, int *text_len)
{
#ifdef ENCRYPT_NEW
	// check if we are an old FS1 style encryption before moving to new type
	if ( !is_old_encrypt(scrambled_text) ) {
		unencrypt_new(scrambled_text, scrambled_len, text, text_len);
		return;
	}
#endif

	int i, num_runs;
	int scramble_offset = 0;
	int byte_offset = 0;
	char maybe_last = 0;

	uint encrypt_id;

	// Only decrypt files that start with unique signature
	memcpy(&encrypt_id, scrambled_text, 4);

	if ( (encrypt_id != Encrypt_signature) && (encrypt_id !=  Encrypt_signature_8bit) ) {
		memcpy(text, scrambled_text, scrambled_len);
		*text_len = scrambled_len;
		return;
	}	

	scrambled_text += 4;
	scrambled_len -= 4;

	// First decrypt stage: undo XOR operation
	for ( i =0; i < scrambled_len; i++ ) {
		scrambled_text[i] ^= i;
	}

	if (encrypt_id == Encrypt_signature_8bit) {
		memcpy(text, scrambled_text, scrambled_len);
		*text_len = scrambled_len;
		return;
	}

	// Second decrypt stage: remove chars from 7 bit packing to 8 bit boundries
	num_runs = (int) (scrambled_len / 7.0f );
	if ( scrambled_len % 7 ) {
		num_runs++;
	}

	for ( i =0; i < num_runs; i++ ) {
		// a run consists of 8 chars packed into 56 bits (instead of 64)

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 1) & 0x7f);
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 2) & 0x3f);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 6) & 0x40 );
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 3) & 0x1f);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 5) & 0x60 );
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 4) & 0x0f);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 4) & 0x70 );
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 5) & 0x07);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 3) & 0x78 );
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 6) & 0x03);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 2) & 0x7c );
		byte_offset++;
		scramble_offset++;

		if ( scramble_offset >= scrambled_len ) {
			break;
		}

		text[byte_offset] = (char)((scrambled_text[scramble_offset] >> 7) & 0x01);
		text[byte_offset] |= ( (scrambled_text[scramble_offset-1] << 1) & 0x7e );
		byte_offset++;

		maybe_last = (char)(scrambled_text[scramble_offset] & 0x7f);
		if ( maybe_last > 0 ) {
			text[byte_offset] = maybe_last;
			byte_offset++;
			scramble_offset++;
		}
	}

	*text_len = byte_offset;
}

#define NUM_LVL1_KEYS					11
ushort Lvl1_keys[NUM_LVL1_KEYS] = {
	0xa820, 0x71f0,
	0x88da, 0x1fff,
	0x2718, 0xe6a1,
	0x42b8, 0x0ce9,
	0x10ec, 0xd77d,
	0x3fa9
};

// scramble text data
//
// input:	text		=>	ascii data to be scrambled
//				text_len	=>	number of bytes of ascii data to scramble
//				scrambled_text	=>	storage for scrambled text (malloc at least text_len)
//				scrambled_len	=>	size of text after getting scrambled
void encrypt_new(char *text, int text_len, char *scrambled_text, int *scrambled_len)
{
	ushort lvl1_block[NUM_LVL1_KEYS * 2];	
	ushort block_checksum;
	int block_size, idx;

	// add the encryption signature
	memcpy(scrambled_text, &Encrypt_new_signature, 4);	
		
	// go through and read in chunks of NUM_LVL1_KEYS * 2 bytes	
	block_checksum = 0xffff;
	*scrambled_len = 0;	
	while(*scrambled_len < text_len){
		// if we have less than one block left
		if((text_len - *scrambled_len) < (NUM_LVL1_KEYS * 2)){
			memcpy(lvl1_block, text + *scrambled_len, text_len - *scrambled_len);
			block_size = text_len - *scrambled_len;			
		}
		// if we have at least one full block left
		else {
			memcpy(lvl1_block, text + *scrambled_len, NUM_LVL1_KEYS * 2); //-V512
			block_size = NUM_LVL1_KEYS * 2;			
		}

		// run the lvl1 over the block
		for(idx=0; idx<block_size/2; idx++){
			// the base key with the running checksum from the _last_ block
			lvl1_block[idx] ^= (Lvl1_keys[idx] ^ block_checksum);
		}

		// the running checksum
		block_checksum = chksum_add_short(block_checksum, (char*)lvl1_block, block_size);		
		
		// copy into the outgoing buffer
		memcpy(scrambled_text + *scrambled_len + 4, lvl1_block, block_size);
		*scrambled_len += block_size;
	}

	// add the 4 bytes for the header
	*scrambled_len += 4;

}

//	input:	scrambled_text	=>	scrambled text
//				scrambled_len	=>	number of bytes of scrambled text
//				text				=>	storage for unscrambled ascii data
//				text_len			=>	actual number of bytes of unscrambled data
void unencrypt_new(char *scrambled_text, int scrambled_len, char *text, int *text_len)
{
	ushort lvl1_block[NUM_LVL1_KEYS * 2];
	ushort lvl1_block_copy[NUM_LVL1_KEYS * 2];
	ushort block_checksum;
	int block_size, idx;	
	uint encrypt_id;

	// Only decrypt files that start with unique signature
	memcpy(&encrypt_id, scrambled_text, 4);
	if (encrypt_id != Encrypt_new_signature) {
		memcpy(text, scrambled_text, scrambled_len);
		*text_len = scrambled_len;
		return;
	}	
	
	// go through and read in chunks of NUM_LVL1_KEYS * 2 bytes
	*text_len = 0;	
	scrambled_text += 4;
	scrambled_len -= 4;
	block_checksum = 0xffff;
	while(*text_len < scrambled_len){
		// if we have less than one block left
		if((scrambled_len - *text_len) < (NUM_LVL1_KEYS * 2)){
			memcpy(lvl1_block, scrambled_text + *text_len, scrambled_len - *text_len);
			block_size = scrambled_len - *text_len;			
		}
		// if we have at least one full block left
		else {
			memcpy(lvl1_block, scrambled_text + *text_len, NUM_LVL1_KEYS * 2); //-V512
			block_size = NUM_LVL1_KEYS * 2;			
		}		
		
		// copy the block so that we can properly calculate the next running checksum
		memcpy(lvl1_block_copy, lvl1_block, block_size);
		// run the lvl1 over the block
		for(idx=0; idx<block_size/2; idx++){
			lvl1_block[idx] ^= (Lvl1_keys[idx] ^ block_checksum);
		}

		// the running checksum
		block_checksum = chksum_add_short(block_checksum, (char*)lvl1_block_copy, block_size);		

		// copy into the outgoing buffer
		memcpy(text + *text_len, lvl1_block, block_size);
		*text_len += block_size;
	}
}

// Return 1 if the data is encrypted, otherwise return 0
int is_encrypted(char *scrambled_text)
{
	uint	encrypt_id;

	memcpy(&encrypt_id, scrambled_text, 4);

	if ( (encrypt_id == Encrypt_signature) || (encrypt_id == Encrypt_signature_8bit) || (encrypt_id == Encrypt_new_signature)) {
		return 1;
	}

	return 0;
}

// Returns 1 if the data uses a FS1 style encryption, 0 if FS2 style
int is_old_encrypt(char *scrambled_text)
{
	uint	encrypt_id;

	memcpy(&encrypt_id, scrambled_text, 4);

	if ( (encrypt_id == Encrypt_signature) || (encrypt_id == Encrypt_signature_8bit) ) {
		return 1;
	}

	return 0;
}

// return text description of the encrypted text type
const char *encrypt_type(char *scrambled_text)
{
	uint encrypt_id;

	memcpy(&encrypt_id, scrambled_text, 4);

	if (encrypt_id == Encrypt_signature) {
		return "FreeSpace 1 type encryption, 7-bit";
	} else if (encrypt_id == Encrypt_signature_8bit) {
		return "FreeSpace 1 type encryption, 8-bit";
	} else if (encrypt_id == Encrypt_new_signature) {
		return "FreeSpace 2 type encryption";
	} else {
		return "Not encrypted or unknown encryption type";
	}
}

// initialize encryption
void encrypt_init()
{	
	int idx;
	ushort temp_var = 0xe2A8;

	if(Encrypt_inited){
		return;
	}

	// meddle with the key table so someone reading the disassembly won't be able to get key values unless they're _REALLY_ careful
	for(idx=0; idx<NUM_LVL1_KEYS; idx++){
		Lvl1_keys[idx] ^= (temp_var >> 2);
	}

	Encrypt_inited = 1;
}
