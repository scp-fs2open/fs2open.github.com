/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include <string.h>
#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"

// NEVER ADD ANY OTHER INCLUDES HERE - YOU'LL LIKELY BREAK THE LAUNCHER!!!!!!!!!!!!!

// ------------------------------------------------------------------------------------------------------------
// REGISTRY DEFINES/VARS
//

static char			szCompanyName[128];
static char			szAppName[128];
static char			szAppVersion[128];

char *Osreg_company_name = "Volition";
char *Osreg_class_name = "Freespace2Class";
#if defined(FS2_DEMO)
char *Osreg_app_name = "FreeSpace2Demo";
char *Osreg_title = "Freespace 2 Demo";
#elif defined(OEM_BUILD)
char *Osreg_app_name = "FreeSpace2OEM";
char *Osreg_title = "Freespace 2 OEM";
#else
char *Osreg_app_name = "FreeSpace2";
char *Osreg_title = "Freespace 2";
#endif

int Os_reg_inited = 0;


// ------------------------------------------------------------------------------------------------------------
// REGISTRY FUNCTIONS
//

// os registry functions -------------------------------------------------------------

// initialize the registry. setup default keys to use
void os_init_registry_stuff(char *company, char *app, char *version)
{
	if(company){
		strcpy( szCompanyName, company );	
	} else {
		strcpy( szCompanyName, Osreg_company_name);
	}

	if(app){
		strcpy( szAppName, app );	
	} else {
		strcpy( szAppName, Osreg_app_name);
	}

	if(version){
		strcpy( szAppVersion, version);	
	} else {
		strcpy( szAppVersion, "1.0");
	}

	Os_reg_inited = 1;
}

// Removes a value from to the INI file.  Passing
// name=NULL will delete the section.
void os_config_remove( char *section, char *name )
{
	// mharris TODO
}

// Writes a string to the INI file.  If value is NULL,
// removes the string. Writing a NULL value to a NULL name will delete
// the section.
void os_config_write_string( char *section, char *name, char *value )
{
	// mharris TODO
}

// same as previous function except we don't use the application name to build up the keyname
void os_config_write_string2( char *section, char *name, char *value )
{
	// mharris TODO
}

// Writes an unsigned int to the INI file.  
void os_config_write_uint( char *section, char *name, uint value )
{
	// mharris TODO
}


// Reads a string from the INI file.  If default is passed,
// and the string isn't found, returns ptr to default otherwise
// returns NULL;    Copy the return value somewhere before
// calling os_read_string again, because it might reuse the
// same buffer.
//static char tmp_string_data[1024];
char * os_config_read_string( char *section, char *name, char *default_value )
{
	// mharris TODO
	return default_value;
}

// same as previous function except we don't use the application name to build up the keyname
char * os_config_read_string2( char *section, char *name, char *default_value )
{
	// mharris TODO
	return default_value;
}

// Reads a string from the INI file.  Default_value must 
// be passed, and if 'name' isn't found, then returns default_value
uint  os_config_read_uint( char *section, char *name, uint default_value )
{
	// mharris TODO
	return default_value;
}

// uses Ex versions of Windows registry functions
//static char tmp_string_data_ex[1024];
char * os_config_read_string_ex( char *keyname, char *name, char *default_value )
{
	// mharris TODO
	return default_value;
}
