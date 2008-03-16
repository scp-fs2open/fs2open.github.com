/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Localization/fhash.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:15:51 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 *  
 * $NoKeywords: $
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
