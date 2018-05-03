/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _FS2_REGISTRY_HEADER_FILE
#define _FS2_REGISTRY_HEADER_FILE


#include <cstdlib>

// ------------------------------------------------------------------------------------------------------------
// REGISTRY DEFINES/VARS
//

// exectuable defines
extern const char *Osreg_company_name;
extern const char *Osreg_class_name;
extern const char *Osreg_app_name;
extern const char *Osreg_title;

extern const char *Osreg_config_file_name;

// ------------------------------------------------------------------------------------------------------------
// REGISTRY FUNCTIONS
//


// initialize the registry. setup default keys to use
void os_init_registry_stuff( const char *company, const char *app);

// Writes a string to the registry
void os_config_write_string( const char *section, const char *name, const char *value );

// Writes an unsigned int to the INI file.  
void os_config_write_uint( const char *section, const char *name, unsigned int value );

// Reads a string from the INI file.  If default is passed,
// and the string isn't found, returns ptr to default otherwise
// returns NULL;    Copy the return value somewhere before
// calling os_read_string again, because it might reuse the
// same buffer.
const char * os_config_read_string( const char *section, const char *name, const char *default_value=0 /*NULL*/ );

// Reads a string from the INI file.  Default_value must 
// be passed, and if 'name' isn't found, then returns default_value
unsigned int  os_config_read_uint( const char *section, const char *name, unsigned int default_value );

#endif
