/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FRED_XSTR_HASH_TABLE_HEADER_FILE
#define _FRED_XSTR_HASH_TABLE_HEADER_FILE

// -----------------------------------------------------------------------------------------------
// HASH DEFINES/VARS
//


// -----------------------------------------------------------------------------------------------
// HASH FUNCTIONS
//

// initialize the hash table
void fhash_init();

// set the hash table to be active for parsing
void fhash_activate();

// set the hash table to be inactive for parsing
void fhash_deactivate();

// if the hash table is active
int fhash_active();

// flush out the hash table, freeing up everything
void fhash_flush();

// add a string with the given id# to the has table
void fhash_add_str(char *str, int id);

// determine if the passed string exists in the table
// returns : -2 if the string doesn't exit, or >= -1 as the string id # otherwise
int fhash_string_exists(char *str);

#endif
