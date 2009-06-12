/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/OsApi/OutWnd.cpp $
 * $Revision: 2.7.2.2 $
 * $Date: 2007-05-11 03:16:01 $
 * $Author: taylor $
 *
 * Routines for debugging output
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.7.2.1  2006/11/15 00:54:33  taylor
 * updated outwnd code for both Windows and non-Windows:
 *  - make print filters dynamic
 *  - fix various little bugs and issues
 *  - cleanup some non-used variables (non-Windows)
 *  - put everything under NDEBUG like it was, or should be
 *  - change to using data/fs2_open.log for log file (Windows)
 *  - change to using data/debug_filter.cfg (Windows)
 *  - make the extra debug window optional, and disabled by default (Windows)
 *  - fix some possible NULL references
 *  - clean up the SAFEPOINT() crap
 *
 * Revision 2.7  2006/04/20 06:32:23  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.6  2005/10/23 20:34:30  taylor
 * some cleanup, fix some general memory leaks, safety stuff and whatever else Valgrind complained about
 *
 * Revision 2.5  2005/01/31 10:34:38  taylor
 * merge with Linux/OSX tree - p0131
 *
 * Revision 2.4  2004/07/26 17:50:02  Goober5000
 * last bit of Unix fixorage
 * --Goober5000
 *
 * Revision 2.3  2003/03/18 10:07:05  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.2.2.1  2002/11/04 21:25:00  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who don't want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.2  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/29 20:12:02  penguin
 * removed bogus #include windows.h
 *
 * Revision 2.0  2002/06/03 04:10:40  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/21 15:46:27  mharris
 * Turned on debug logging to file
 *
 * Revision 1.2  2002/05/17 23:41:41  mharris
 * We can print even if not inited... also flush after print
 *
 * Revision 1.1  2002/05/16 00:47:21  mharris
 * New version of OsApi files for Unix variants.
 *
 * Revision 1.2  2002/05/09 13:52:26  mharris
 * Removed dead test code
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 5     6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 4     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 3     10/08/98 2:38p Dave
 * Cleanup up OsAPI code significantly. Removed old functions, centralized
 * registry functions.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 49    5/15/98 3:36p John
 * Fixed bug with new graphics window code and standalone server.  Made
 * hwndApp not be a global anymore.
 * 
 * 48    5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 47    5/14/98 11:24a Allender
 * throw mprints to outtext regardless of gr mode
 * 
 * 46    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 45    4/16/98 6:31p Dave
 * Doubled MAX_FILTERS to 48
 * 
 * 44    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 43    3/11/98 12:06p John
 * Made it so output window never gets focus upon startup
 * 
 * 42    2/16/98 9:47a John
 * Made so only error, general, and warning are shown if no
 * debug_filter.cfg and  .cfg file isn't saved if so.
 * 
 * 41    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 40    1/15/98 9:14p John
 * Made it so general, error and warning can't be turned off.
 * 
 * 39    9/13/97 10:44a Lawrance
 * allow logging of nprintf output to file
 * 
 * 38    8/23/97 11:32a Dave
 * Made debug window size correctly in standalone mode.
 * 
 * 37    8/22/97 3:42p Hoffoss
 * Lowered the filter limit to 24, and added support for filter recycling,
 * instead of the rather nasty Assert if we should happen to exceed this
 * limit.
 * 
 * 36    8/05/97 4:29p Dave
 * Futzed with window sizes/placements for standalone mode.
 * 
 * 35    8/04/97 3:15p Dave
 * Added an include for systemvars.h
 * 
 * 34    5/20/97 1:13p Allender
 * fixes to make outwnd work better under Win95
 * 
 * 33    5/14/97 11:08a John
 * fixed bug under 95 that occasionally hung program.
 * 
 * 32    5/07/97 3:01p Lawrance
 * check for NT in mono_init
 * 
 * 31    5/07/97 3:06p John
 * disabled new mono direct stuff under NT.
 * 
 * 30    5/07/97 2:59p John
 * Made so mono driver works under '95.
 * 
 * 29    4/22/97 10:56a John
 * fixed some resource leaks.
 *
 * $NoKeywords: $
 */

#ifndef WIN32	// Goober5000

#ifndef NDEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "globalincs/pstypes.h"
#include "osapi/outwnd.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "cfile/cfilesystem.h"
#include "globalincs/globals.h"

#include <vector>


void outwnd_print(char *id = NULL, char *tmp = NULL);

#define MAX_LINE_WIDTH	128

bool outwnd_inited = false;
ubyte Outwnd_no_filter_file = 0;		// 0 = .cfg file found, 1 = not found and warning not printed yet, 2 = not found and warning printed

struct outwnd_filter_struct {
	char name[NAME_LENGTH];
	bool enabled;

	outwnd_filter_struct() { memset(this, 0, sizeof(outwnd_filter_struct)); }
};

std::vector<outwnd_filter_struct> OutwndFilter;

int outwnd_filter_loaded = 0;

// used for file logging
int Log_debug_output_to_file = 1;
FILE *Log_fp = NULL;
char *FreeSpace_logfilename = "fs2_open.log";

char safe_string[512] = { 0 };


void load_filter_info(void)
{
	FILE *fp = NULL;
	char pathname[MAX_PATH_LEN];
	char inbuf[NAME_LENGTH+4];
	outwnd_filter_struct new_filter;
	int z;

	outwnd_filter_loaded = 1;

	snprintf( pathname, MAX_PATH_LEN, "%s/%s/%s/%s", detect_home(), Osreg_user_dir, Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

	fp = fopen(pathname, "rt");

	if (!fp) {
		Outwnd_no_filter_file = 1;

		memset( &new_filter, 0, sizeof(outwnd_filter_struct) );
		strcpy( new_filter.name, "error" );
		new_filter.enabled = true;

		OutwndFilter.push_back( new_filter );

		memset( &new_filter, 0, sizeof(outwnd_filter_struct) );
		strcpy( new_filter.name, "general" );
		new_filter.enabled = true;

		OutwndFilter.push_back( new_filter );

		memset( &new_filter, 0, sizeof(outwnd_filter_struct) );
		strcpy( new_filter.name, "warning" );
		new_filter.enabled = true;

		OutwndFilter.push_back( new_filter );

		return;
	}

	Outwnd_no_filter_file = 0;

	while ( fgets(inbuf, NAME_LENGTH+3, fp) ) {
		memset( &new_filter, 0, sizeof(outwnd_filter_struct) );

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
		strcpy(new_filter.name, inbuf + 1);

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


	snprintf( pathname, MAX_PATH_LEN, "%s/%s/%s/%s", detect_home(), Osreg_user_dir, Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

	fp = fopen(pathname, "wt");

	if (fp) {
		for (uint i = 0; i < OutwndFilter.size(); i++)
			fprintf(fp, "%c%s\n", OutwndFilter[i].enabled ? '+' : '-', OutwndFilter[i].name);

		fclose(fp);
	}
}

void outwnd_printf2(char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4];
	va_list args;

	if (format == NULL)
		return;

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	outwnd_print("General", tmp);
}

void outwnd_printf(char *id, char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4];
	va_list args;

	if ( (id == NULL) || (format == NULL) )
		return;

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	outwnd_print(id, tmp);
}

void outwnd_print(char *id, char *tmp)
{
	uint i;

	if ( (id == NULL) || (tmp == NULL) )
		return;

  	if ( !outwnd_inited ) {
  		fputs("outwnd not initialized yet...  ", stdout);
		fputs(tmp, stdout);
		fflush(stdout);

  		return;
	}

	if (Outwnd_no_filter_file == 1) {
		Outwnd_no_filter_file = 2;

		outwnd_print( "general", "==========================================================================\n" );
		outwnd_print( "general", "DEBUG SPEW: No debug_filter.cfg found, so only general, error, and warning\n" );
		outwnd_print( "general", "categories can be shown and no debug_filter.cfg info will be saved.\n" );
		outwnd_print( "general", "==========================================================================\n" );
	}

	if ( !id )
		id = "General";

	for (i = 0; i < OutwndFilter.size(); i++) {
		if ( !stricmp(id, OutwndFilter[i].name) )
			break;
	}

	// id found that isn't in the filter list yet
	if ( i == OutwndFilter.size() ) {
		// Only create new filters if there was a filter file
		if (Outwnd_no_filter_file)
			return;

		Assert( strlen(id)+1 < NAME_LENGTH );
		outwnd_filter_struct new_filter;

		strcpy(new_filter.name, id);
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
	} else {
		fputs(tmp, stdout);
		fflush(stdout);
	}
}


void outwnd_init(int display_under_freespace_window)
{
	outwnd_inited = true;

	char pathname[MAX_PATH_LEN];

	snprintf(pathname, MAX_PATH_LEN, "%s/%s/%s/%s", detect_home(), Osreg_user_dir, Pathtypes[CF_TYPE_DATA].path, FreeSpace_logfilename);

	if (Log_fp == NULL) {
		Log_fp = fopen(pathname, "wb");

		if (Log_fp == NULL) {
			outwnd_printf("Error", "Error opening %s\n", pathname);
		} else {
			time_t timedate = time(NULL);
			char datestr[50];

			memset( datestr, 0, sizeof(datestr) );
			strftime( datestr, sizeof(datestr)-1, "%a %b %d %H:%M:%S %Y", localtime(&timedate) );

			printf("Future debug output directed to: %s\n", pathname);
			outwnd_printf("General", "Opened log '%s', %s ...\n", pathname, datestr);
		}
	}
}

void outwnd_close()
{
	if (Log_fp != NULL) {
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

void safe_point_print(char *format, ...)
{
	char tmp[512];
	va_list args;
	
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	strcpy(safe_string, tmp);
}

void safe_point(char *file, int line, char *format, ...)
{
	safe_point_print("last safepoint: %s, %d; [%s]", file, line, format);
}

#endif // NDEBUG

#endif		// Goober5000 - #ifndef WIN32
