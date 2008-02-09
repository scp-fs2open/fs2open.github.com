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
 * $Revision: 2.3 $
 * $Date: 2003-03-18 10:07:05 $
 * $Author: unknownplayer $
 *
 * Routines for debugging output
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2.2.1  2002/11/04 21:25:00  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
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

#ifndef NDEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "globalincs/pstypes.h"
#include "osapi/outwnd.h"

void outwnd_print(char *id, char *tmp);

#define MAX_FILTERS 48
#define MAX_LINE_WIDTH	128

bool outwnd_inited = false;
bool outwnd_disabled = true;
bool OutputActive = false;
int Outwnd_no_filter_file = 0;		// 0 = .cfg file found, 1 = not found and warning not printed yet, 2 = not found and warning printed

struct outwnd_filter_struct {
	char name[FILTER_NAME_LENGTH];
	int state;
} *outwnd_filter[MAX_FILTERS], real_outwnd_filter[MAX_FILTERS];


int outwnd_filter_count = 0;
int outwnd_filter_loaded = 0;

// used for file logging
#ifndef NDEBUG
	int Log_debug_output_to_file = 1;
	FILE *Log_fp;
	char *Freespace_logfilename = "fs.log";
#endif

void load_filter_info(void)
{
	FILE *fp;
	char pathname[256], inbuf[FILTER_NAME_LENGTH+4];
	int z;

	outwnd_filter_loaded = 1;
	outwnd_filter_count = 0;
	strcpy(pathname, "Debug_filter.cfg" );
	fp = fopen(pathname, "rt");
	if (!fp)	{
		Outwnd_no_filter_file = 1;

		outwnd_filter[outwnd_filter_count] = &real_outwnd_filter[outwnd_filter_count];
		strcpy( outwnd_filter[outwnd_filter_count]->name, "error" );
		outwnd_filter[outwnd_filter_count]->state = 1;
		outwnd_filter_count++;

		outwnd_filter[outwnd_filter_count] = &real_outwnd_filter[outwnd_filter_count];
		strcpy( outwnd_filter[outwnd_filter_count]->name, "general" );
		outwnd_filter[outwnd_filter_count]->state = 1;
		outwnd_filter_count++;

		outwnd_filter[outwnd_filter_count] = &real_outwnd_filter[outwnd_filter_count];
		strcpy( outwnd_filter[outwnd_filter_count]->name, "warning" );
		outwnd_filter[outwnd_filter_count]->state = 1;
		outwnd_filter_count++;

		return;
	}

	Outwnd_no_filter_file = 0;

	while (fgets(inbuf, FILTER_NAME_LENGTH+3, fp))
	{
		if (outwnd_filter_count == MAX_FILTERS)
			break;

		outwnd_filter[outwnd_filter_count] = &real_outwnd_filter[outwnd_filter_count];
		if (*inbuf == '+')
			outwnd_filter[outwnd_filter_count]->state = 1;
		else if (*inbuf == '-')
			outwnd_filter[outwnd_filter_count]->state = 0;
		else continue;  // skip everything else

		z = strlen(inbuf) - 1;
		if (inbuf[z] == '\n')
			inbuf[z] = 0;

		Assert(strlen(inbuf+1) < FILTER_NAME_LENGTH);
		strcpy(outwnd_filter[outwnd_filter_count]->name, inbuf + 1);

		if ( !stricmp( outwnd_filter[outwnd_filter_count]->name, "error" ) )	{
			outwnd_filter[outwnd_filter_count]->state = 1;
		} else if ( !stricmp( outwnd_filter[outwnd_filter_count]->name, "general" ) )	{
			outwnd_filter[outwnd_filter_count]->state = 1;
		} else if ( !stricmp( outwnd_filter[outwnd_filter_count]->name, "warning" ) )	{
			outwnd_filter[outwnd_filter_count]->state = 1;
		}

		outwnd_filter_count++;
	}

	if (ferror(fp) && !feof(fp))
		nprintf(("Error", "Error reading \"%s\"\n", pathname));

	fclose(fp);
}

void save_filter_info(void)
{
	FILE *fp;
	int i;
	char pathname[256];

	if (!outwnd_filter_loaded)
		return;

	if ( Outwnd_no_filter_file )	{
		return;	// No file, don't save
	}

	strcpy(pathname, "Debug_filter.cfg" );
	fp = fopen(pathname, "wt");
	if (fp)
	{
		for (i=0; i<outwnd_filter_count; i++)
			fprintf(fp, "%c%s\n", outwnd_filter[i]->state ? '+' : '-', outwnd_filter[i]->name);

		fclose(fp);
	}
}

void outwnd_printf2(char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4];
	va_list args;
	
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	outwnd_print("General", tmp);
}


//  char mono_ram[80*25*2];
//  int mono_x, mono_y;
//  int mono_found = 0;

//  void mono_flush()
//  {
//  	if ( !mono_found ) return;
//  	memcpy( (void *)0xb0000, mono_ram, 80*25*2 );
//  }

//  void mono_init()
//  {
//  	int i;

//  	OSVERSIONINFO ver;
	
//  	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//  	GetVersionEx(&ver);
//  	if ( ver.dwPlatformId == VER_PLATFORM_WIN32_NT )	{
//  		mono_found = 0;
//  		return;
//  	}

//  	_outp( 0x3b4, 0x0f );
//  	_outp( 0x3b4+1, 0x55 );

//  	if ( _inp( 0x3b4+1 ) == 0x55 )	{
//  		mono_found = 1;
//  		_outp( 0x3b4+1, 0 );
//  	} else {
//  		mono_found = 0;
//  		return;
//  	}


//  	for (i=0; i<80*25; i++ )	{
//  		mono_ram[i*2+0] = ' ';
//  		mono_ram[i*2+1] = 0x07;
//  	}
//  	mono_flush();
//  	mono_x = mono_y = 0;
//  }


//  void mono_print( char * text, int len )
//  {
//  	int i, j;

//  	if ( !mono_found ) return;

//  	for (i=0; i<len; i++ )	{
//  		int scroll_it = 0;

//  		switch( text[i] )	{
//  		case '\n':
//  			scroll_it = 1;
//  			break;
//  		default:
//  			mono_ram[mono_y*160+mono_x*2] = text[i];
//  			if ( mono_x < 79 )	{
//  				mono_x++;
//  			} else {
//  				scroll_it = 1;
//  			}
//  			break;
//  		}

//  		if ( scroll_it )	{
//  			if ( mono_y < 24 )	{
//  				mono_y++;
//  				mono_x = 0;
//  			} else {
//  				memmove( mono_ram, mono_ram+160, 80*24*2 );
//  				for (j=0; j<80; j++ )	{
//  					mono_ram[24*160+j*2] = ' ';
//  				}
//  				mono_y = 24;
//  				mono_x = 0;
//  			}
//  		}
//  	}
//  	mono_flush();
//  }

void outwnd_printf(char *id, char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4];
	va_list args;
	
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	outwnd_print(id, tmp);
}

void outwnd_print(char *id, char *tmp)
{
//  	char *sptr;
//  	char *dptr;
//	int i, nrows, ccol;
	int i;
	outwnd_filter_struct *temp;

  	if (!outwnd_inited) {
  		fputs("outwnd not initialized yet...  ", stdout);
		fputs(tmp, stdout);
		fflush(stdout);
  		return;
	}

	if ( Outwnd_no_filter_file == 1 )	{
		Outwnd_no_filter_file = 2;

		outwnd_print( "general", "==========================================================================\n" );
		outwnd_print( "general", "DEBUG SPEW: No debug_filter.cfg found, so only general, error, and warning\n" );
		outwnd_print( "general", "categories can be shown and no debug_filter.cfg info will be saved.\n" );
		outwnd_print( "general", "==========================================================================\n" );
	}

	if (!id)
		id = "General";

	for (i=0; i<outwnd_filter_count; i++)
		if (!stricmp(id, outwnd_filter[i]->name))
			break;


	if (i == outwnd_filter_count)  // new id found that's not yet in filter list
	{
		// Only create new filters if there was a filter file
		if ( Outwnd_no_filter_file )	{
			return;
		}

		if (outwnd_filter_count >= MAX_FILTERS) {
			Assert(outwnd_filter_count == MAX_FILTERS);  // how did it get over the max?  Very bad..
			outwnd_printf("General", "Outwnd filter limit reached.  Recycling \"%s\" to add \"%s\"",
				outwnd_filter[MAX_FILTERS - 1]->name, id);

			i--;  // overwrite the last element (oldest used filter in the list)
		}

		Assert(strlen(id) < FILTER_NAME_LENGTH);
		outwnd_filter[i] = &real_outwnd_filter[i];  // note: this assumes the list doesn't have gaps (from deleting an element for example)
		strcpy(outwnd_filter[i]->name, id);
		outwnd_filter[i]->state = 1;
		outwnd_filter_count = i + 1;
		save_filter_info();
	}

	// sort the filters from most recently used to oldest, so oldest ones will get recycled first
	temp = outwnd_filter[i];
	while (i--)
		outwnd_filter[i + 1] = outwnd_filter[i];

	i++;
	outwnd_filter[i] = temp;

	if (!outwnd_filter[i]->state)
		return;

//  	sptr = tmp;
//  	ccol = strlen(outtext[mprintf_last_line] );
//  	dptr = &outtext[mprintf_last_line][ccol];
//  	nrows = 0;

#ifndef NDEBUG

	if ( Log_debug_output_to_file ) {
		if ( Log_fp != NULL ) {
			fputs(tmp, Log_fp);	
			fflush(Log_fp);
		}
	}

#endif

	fputs(tmp, stdout);
	fflush(stdout);
}


void outwnd_init(int display_under_freespace_window)
{
	outwnd_inited = TRUE;

#ifndef NDEBUG
	if ( Log_fp == NULL ) {
		Log_fp = fopen(Freespace_logfilename, "wb");
		if ( Log_fp == NULL )
			outwnd_printf("Error", "Error opening %s\n", Freespace_logfilename);
		else
			outwnd_printf("General", "Opened %s OK\n", Freespace_logfilename);
	}
#endif 
}

void outwnd_close()
{
#ifndef NDEBUG
	if ( Log_fp != NULL ) {
		fclose(Log_fp);
		Log_fp = NULL;
	}
#endif

}


#endif //NDEBUG



