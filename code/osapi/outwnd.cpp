/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef NDEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "osapi/osapi.h"
#include "osapi/outwnd.h"
#include "osapi/monopub.h"
#include "graphics/2d.h"
#include "freespace2/freespaceresource.h"
#include "globalincs/systemvars.h"
#include "cfile/cfilesystem.h"
#include "globalincs/globals.h"
#include "parse/parselo.h"

struct outwnd_filter_struct {
	char name[NAME_LENGTH];
	bool enabled;
};

SCP_vector<outwnd_filter_struct> OutwndFilter;

void outwnd_print(const char *id = NULL, const char *temp = NULL);

ubyte Outwnd_no_filter_file = 0;		// 0 = .cfg file found, 1 = not found and warning not printed yet, 2 = not found and warning printed

ubyte outwnd_filter_loaded = 0;
bool outwnd_inited = false;

// used for file logging
int Log_debug_output_to_file = 1;
FILE *Log_fp = NULL;
char *FreeSpace_logfilename = NULL;

void load_filter_info(void)
{
	FILE *fp = NULL;
	char pathname[MAX_PATH_LEN];
	char inbuf[NAME_LENGTH+4];
	outwnd_filter_struct new_filter;
	int z;

	outwnd_filter_loaded = 1;

	memset( pathname, 0, sizeof(pathname) );
	snprintf( pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

	fp = fopen(os_get_config_path(pathname).c_str(), "rt");

	if (!fp) {
		Outwnd_no_filter_file = 1;

		strcpy_s( new_filter.name, "error" );
		new_filter.enabled = true;
		OutwndFilter.push_back( new_filter );

		strcpy_s( new_filter.name, "general" );
		new_filter.enabled = true;
		OutwndFilter.push_back( new_filter );

		strcpy_s( new_filter.name, "warning" );
		new_filter.enabled = true;
		OutwndFilter.push_back( new_filter );

		return;
	}

	Outwnd_no_filter_file = 0;

	while ( fgets(inbuf, NAME_LENGTH+3, fp) ) {

		if (*inbuf == '+')
			new_filter.enabled = true;
		else if (*inbuf == '-')
			new_filter.enabled = false;
		else
			continue;	// skip everything else

		z = strlen(inbuf) - 1;
		if (inbuf[z] == '\n')
			inbuf[z] = 0;

		Assert( strlen(inbuf+1) < NAME_LENGTH );
		strcpy_s(new_filter.name, inbuf + 1);

		if ( !stricmp(new_filter.name, "error") ) {
			new_filter.enabled = true;
		} else if ( !stricmp(new_filter.name, "general") ) {
			new_filter.enabled = true;
		} else if ( !stricmp(new_filter.name, "warning") ) {
			new_filter.enabled = true;
		}

		OutwndFilter.push_back( new_filter );
	}

	if ( ferror(fp) && !feof(fp) )
		nprintf(("Error", "Error reading \"%s\"\n", pathname));

	fclose(fp);
}

void save_filter_info(void)
{
	FILE *fp = NULL;
	char pathname[MAX_PATH_LEN];

	if ( !outwnd_filter_loaded )
		return;

	if (Outwnd_no_filter_file)
		return;	// No file, don't save


	memset( pathname, 0, sizeof(pathname) );
	snprintf( pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

	fp = fopen(os_get_config_path(pathname).c_str(), "wt");

	if (fp) {
		for (uint i = 0; i < OutwndFilter.size(); i++)
			fprintf(fp, "%c%s\n", OutwndFilter[i].enabled ? '+' : '-', OutwndFilter[i].name);

		fclose(fp);
	}
}

void outwnd_printf2(const char *format, ...)
{
	SCP_string temp;
	va_list args;

	if (format == NULL)
		return;

	va_start(args, format);
	vsprintf(temp, format, args);
	va_end(args);

	outwnd_print("General", temp.c_str());
}

void outwnd_printf(const char *id, const char *format, ...)
{
	SCP_string temp;
	va_list args;

	if ( (id == NULL) || (format == NULL) )
		return;

	va_start(args, format);
	vsprintf(temp, format, args);
	va_end(args);

	outwnd_print(id, temp.c_str());
}

void outwnd_print(const char *id, const char *tmp)
{
	uint i;

	if ( (id == NULL) || (tmp == NULL) )
		return;

  	if ( !outwnd_inited )
  		return;

	if (Outwnd_no_filter_file == 1) {
		Outwnd_no_filter_file = 2;

		outwnd_print( "general", "==========================================================================\n" );
		outwnd_print( "general", "DEBUG SPEW: No debug_filter.cfg found, so only general, error, and warning\n" );
		outwnd_print( "general", "categories can be shown and no debug_filter.cfg info will be saved.\n" );
		outwnd_print( "general", "==========================================================================\n" );
	}

	uint outwnd_size = OutwndFilter.size();

	for (i = 0; i < OutwndFilter.size(); i++) {
		if ( !stricmp(id, OutwndFilter[i].name) )
			break;
	}

	// id found that isn't in the filter list yet
	if ( i == outwnd_size ) {
		// Only create new filters if there was a filter file
		if (Outwnd_no_filter_file)
			return;

		Assert( strlen(id)+1 < NAME_LENGTH );
		outwnd_filter_struct new_filter;

		strcpy_s(new_filter.name, id);
		new_filter.enabled = true;

		OutwndFilter.push_back( new_filter );
		save_filter_info();
	}

	if ( !OutwndFilter[i].enabled )
		return;

	if (Log_debug_output_to_file) {
		if (Log_fp != NULL) {
			fputs(tmp, Log_fp);	
			fflush(Log_fp);
		}
	}
}

void outwnd_init()
{
	if (outwnd_inited)
		return;

	if (Log_fp == NULL) {
		char pathname[MAX_PATH_LEN];

		/* Set where the log file is going to go */
		// Zacam: Set various conditions based on what type of log to generate.
		if (Fred_running) {
			FreeSpace_logfilename = "fred2_open.log";
		} else if (Is_standalone) {
			FreeSpace_logfilename = "fs2_standalone.log";
		} else {
			FreeSpace_logfilename = "fs2_open.log";
		}

		memset( pathname, 0, sizeof(pathname) );
		snprintf( pathname, MAX_PATH_LEN, "%s/%s", Pathtypes[CF_TYPE_DATA].path, FreeSpace_logfilename);

		Log_fp = fopen(os_get_config_path(pathname).c_str(), "wb");

		if (Log_fp == NULL) {
			outwnd_printf("Error", "Error opening %s\n", pathname);
		} else {
			time_t timedate = time(NULL);
			char datestr[50];

			memset( datestr, 0, sizeof(datestr) );
			strftime( datestr, sizeof(datestr)-1, "%a %b %d %H:%M:%S %Y", localtime(&timedate) );

			outwnd_printf("General", "Opened log '%s', %s ...\n", pathname, datestr);
		}
	}

	outwnd_inited = true;
}

void outwnd_close()
{
	if ( Log_fp != NULL ) {
		time_t timedate = time(NULL);
		char datestr[50];

		memset( datestr, 0, sizeof(datestr) );
		strftime( datestr, sizeof(datestr)-1, "%a %b %d %H:%M:%S %Y", localtime(&timedate) );

		outwnd_printf("General", "... Log closed, %s\n", datestr);

		fclose(Log_fp);
		Log_fp = NULL;
	}

	outwnd_inited = false;
}

#endif //NDEBUG
