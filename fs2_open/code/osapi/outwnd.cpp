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
 * $Revision: 2.16.2.5 $
 * $Date: 2007-11-24 04:33:50 $
 * $Author: wmcoolmon $
 *
 * Routines for debugging output
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.16.2.4  2007/07/23 16:08:29  Kazan
 * Autopilot updates, minor misc fixes, working MSVC2005 project files
 *
 * Revision 2.16.2.3  2007/05/11 03:16:01  taylor
 * add session open/close times to debug log
 *
 * Revision 2.16.2.2  2007/02/11 09:39:09  taylor
 * some minor performance improvements
 * remove NO_SOUND
 *
 * Revision 2.16.2.1  2006/11/15 00:54:33  taylor
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
 * Revision 2.16  2006/04/20 06:32:23  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.15  2006/01/26 03:23:30  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 2.14  2005/04/20 04:34:42  phreak
 * NMONO should not interfere with the debug window
 *
 * Revision 2.13  2005/03/03 06:05:30  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.12  2004/07/26 20:47:46  Kazan
 * remove MCD complete
 *
 * Revision 2.11  2004/07/12 16:33:01  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.10  2004/03/31 05:42:28  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 2.9  2004/03/06 23:28:23  bobboau
 * fixed motion debris
 * animated laser textures
 * and added a new error check called a safepoint, mostly for tracking the 'Y bug'
 *
 * Revision 2.8  2004/03/05 09:02:08  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.7  2004/02/14 00:18:35  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.6  2004/02/04 09:02:44  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 2.5  2003/03/18 10:07:05  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.4  2003/03/02 06:04:00  penguin
 * Added versions of _outp() and _inp() for gcc.
 * Revision 2.1.2.3  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.1.2.2  2002/11/04 21:25:00  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.1.2.1  2002/10/08 14:33:27  randomtiger
 * OK, I've fixed the z-buffer problem.
 * However I have abandoned using w-buffer for now because of problems.
 * I think I know how to solve it but Im not sure it would make much difference given the way FS2 engine works.
 * I have left code in there of use if anyone wants to turn their hand to it. However for now
 * we just need to crack of the alpha problem then we will have FS2 fully wokring in DX8 on GF4 in 32 bit.
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.3  2002/11/14 04:18:17  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * 
 * 6     6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 5     6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 4      12/14/98 12:13p Dave
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

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// to disable otherwise well-lodged compiler warning
#pragma warning(disable: 4201)	// nameless struct/union

#include <winioctl.h>
#include <conio.h>

#include "osapi/osapi.h"
#include "osapi/outwnd.h"
#include "osapi/monopub.h"
#include "graphics/2d.h"
#include "freespace2/freespaceresource.h"
#include "globalincs/systemvars.h"
#include "cfile/cfilesystem.h"
#include "globalincs/globals.h"

#include <vector>

extern int Cmdline_debug_window;


#define SCROLL_BUFFER_SIZE	512
#define MAX_LINE_WIDTH	128
#define TASKBAR_HEIGHT 30

#define ID_COPY 32000
#define ID_FIND 32010
#define ID_FILTER 32100
#define TIMER1 1
#define UP_FAST 65537
#define DOWN_FAST 65538

HANDLE hOutputThread=NULL;
DWORD OutputThreadID;

HWND hOutputWnd;
char szOutputClass[] = "OutputWindow";
char spaces[MAX_LINE_WIDTH + 1];
ubyte outwnd_filter_loaded = 0;
char find_text[85];

struct outwnd_filter_struct {
	char name[NAME_LENGTH];
	bool enabled;

	outwnd_filter_struct() { memset(this, 0, sizeof(outwnd_filter_struct)); }
};

std::vector<outwnd_filter_struct> OutwndFilter;

int mprintf_last_line = -1;
char outtext[SCROLL_BUFFER_SIZE][MAX_LINE_WIDTH];
int old_scroll_pos = -32768;
int nTextHeight=0, nTextWidth=0, nCharRows=0;
bool outwnd_inited = false;
bool outwnd_disabled = true;
int max_scroll_pos = SCROLL_BUFFER_SIZE - 1;
static int marked = 0;
int marked_top, marked_bottom, marked_left, marked_right;
int old_marked = 0, old_marked_top, old_marked_bottom, old_marked_left, old_marked_right;
int marking_started_x, marking_started_y, client_top_yoffset, cur_line_index, marking_active = 0;
int scroll_wait = 1;
BOOL OutputActive = FALSE;
LPARAM last_mousemove;
int Outwnd_changed = 0;
int find_line = -1, find_pos;

// monochrome screen info
#define NMONO
HANDLE  mono_driver=NULL;				// handle to the monochrome driver

void outwnd_print(char *id, char *tmp);
BOOL CALLBACK find_dlg_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam);
void find_text_in_outwindow(int n, int p);
void outwnd_copy_marked_selection(HWND hwnd);
void outwnd_update_marking(LPARAM l_parm, HWND hwnd);
void outwnd_paint(HWND hwnd);

ubyte Outwnd_no_filter_file = 0;		// 0 = .cfg file found, 1 = not found and warning not printed yet, 2 = not found and warning printed

// used for file logging
int Log_debug_output_to_file = 1;
FILE *Log_fp = NULL;
char *FreeSpace_logfilename = "fs2_open.log";

char safe_string[512] = { 0 };



#ifndef _MSC_VER
inline void _outp(unsigned short port, unsigned char value)
{
	asm(
		 "mov %0,%%al;"
		 "mov %1,%%dx;"
		 "out %%al,%%dx;"
		 : : "g" (value), "g" (port) : "al", "dx");
}

inline unsigned char _inp(unsigned short port)
{
	unsigned char value;
	asm(
		 "mov %1,%%dx;"
		 "in  %%dx,%%al;"
		 "mov %%al,%0"
		 : "=g"(value) : "g" (port) : "al", "dx");
	return value;
}
#endif


inline void text_hilight(HDC &hdc)
{
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));
}

inline void text_normal(HDC &hdc)
{
	SetBkColor(hdc, RGB(255, 255, 255));
	SetTextColor(hdc, RGB(0, 0, 0));
}

inline void fix_marking_coords(int &x, int &y, LPARAM l_parm)
{
	x = (signed short) LOWORD(l_parm) / nTextWidth;
	y = ((signed short) HIWORD(l_parm) - client_top_yoffset) / nTextHeight + cur_line_index;
	
	if (y < 0)
		y = x = 0;

	if (y > SCROLL_BUFFER_SIZE)
	{
		y = SCROLL_BUFFER_SIZE;
		x = 0;
	}

	if (x < 0)
		x = -1;

	if (x > MAX_LINE_WIDTH)
	{
		y++;
		x = 0;  // marks to end of line
	}
}


//			InvalidateRect(hOutputWnd,NULL,0);

void load_filter_info(void)
{
	FILE *fp = NULL;
	char pathname[MAX_PATH_LEN];
	char inbuf[NAME_LENGTH+4];
	outwnd_filter_struct new_filter;
	int z;

	outwnd_filter_loaded = 1;

	memset( pathname, 0, sizeof(pathname) );
	snprintf( pathname, MAX_PATH_LEN, "%s\\%s\\%s", detect_home(), Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

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


	memset( pathname, 0, sizeof(pathname) );
	snprintf( pathname, MAX_PATH_LEN, "%s\\%s\\%s", detect_home(), Pathtypes[CF_TYPE_DATA].path, NOX("debug_filter.cfg") );

	fp = fopen(pathname, "wt");

	if (fp) {
		for (uint i = 0; i < OutwndFilter.size(); i++)
			fprintf(fp, "%c%s\n", OutwndFilter[i].enabled ? '+' : '-', OutwndFilter[i].name);

		fclose(fp);
	}
}

void outwnd_printf2(char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4] = {'\0'};
	va_list args;

	if (format == NULL)
		return;

	va_start(args, format);
	//WMC - Switched from vsprintf to _vsnprintf to prevent overflow of tmp
	//Try vsnprintf under Linux or OS X if this breaks compilation there.
	_vsnprintf(tmp, sizeof(tmp)-1,format, args);
	va_end(args);

	outwnd_print("General", tmp);
}

#ifndef NMONO
char mono_ram[80*25*2];
int mono_x, mono_y;
int mono_found = 0;

void mono_flush()
{
	if ( !mono_found ) return;
	memcpy( (void *)0xb0000, mono_ram, 80*25*2 );
}

void mono_init()
{
	int i;

	OSVERSIONINFO ver;
	
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	if ( ver.dwPlatformId == VER_PLATFORM_WIN32_NT )	{
		mono_found = 0;
		return;
	}

	_outp( 0x3b4, 0x0f );
	_outp( 0x3b4+1, 0x55 );

	if ( _inp( 0x3b4+1 ) == 0x55 )	{
		mono_found = 1;
		_outp( 0x3b4+1, 0 );
	} else {
		mono_found = 0;
		return;
	}


	for (i=0; i<80*25; i++ )	{
		mono_ram[i*2+0] = ' ';
		mono_ram[i*2+1] = 0x07;
	}
	mono_flush();
	mono_x = mono_y = 0;
}


void mono_print( char * text, int len )
{
	int i, j;

	if ( !mono_found )
		return;


	for (i=0; i<len; i++ )	{
		int scroll_it = 0;

		switch( text[i] )	{
		case '\n':
			scroll_it = 1;
			break;
		default:
			mono_ram[mono_y*160+mono_x*2] = text[i];
			if ( mono_x < 79 )	{
				mono_x++;
			} else {
				scroll_it = 1;
			}
			break;
		}

		if ( scroll_it )	{
			if ( mono_y < 24 )	{
				mono_y++;
				mono_x = 0;
			} else {
				memmove( mono_ram, mono_ram+160, 80*24*2 );
				for (j=0; j<80; j++ )	{
					mono_ram[24*160+j*2] = ' ';
				}
				mono_y = 24;
				mono_x = 0;
			}
		}
	}
	mono_flush();
}
#endif // NMONO


void outwnd_printf(char *id, char *format, ...)
{
	char tmp[MAX_LINE_WIDTH*4] = {'\0'};
	va_list args;

	if ( (id == NULL) || (format == NULL) )
		return;

	va_start(args, format);
	//WMC - Switched from vsprintf to _vsnprintf to prevent overflow of tmp
	//Try vsnprintf under Linux or OS X if this breaks compilation there.
	_vsnprintf(tmp, sizeof(tmp)-1, format, args);
	va_end(args);

	outwnd_print(id, tmp);
}

void outwnd_print(char *id, char *tmp)
{
	char *sptr;
	char *dptr;
	int nrows, ccol;
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

	if ( !id )
		id = "General";

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
	}

	if ( !Cmdline_debug_window )
		return;

	// everything after this point relates to printing in the debug window

	if (mprintf_last_line == -1 )	{
		for (i=0; i<SCROLL_BUFFER_SIZE;i++)	{
			outtext[i][0] = 0;
		}

		mprintf_last_line = 0;
	}

	// printf out to the monochrome screen first
	if ( mono_driver != ((HANDLE)-1) ) {
		DWORD   cbReturned;

		DeviceIoControl (mono_driver, (DWORD)IOCTL_MONO_PRINT, tmp, strlen(tmp), NULL, 0, &cbReturned, 0 );
#ifndef NMONO
	} else {
		mono_print(tmp, strlen(tmp) );
#endif
	}

	sptr = tmp;
	ccol = strlen(outtext[mprintf_last_line] );
	dptr = &outtext[mprintf_last_line][ccol];
	nrows = 0;

	while(*sptr) {
		if ( (*sptr == '\n') || (ccol >= MAX_LINE_WIDTH-1 ) )	{
			nrows++;
			mprintf_last_line++;
			if (mprintf_last_line >= SCROLL_BUFFER_SIZE )
				mprintf_last_line = 0;
			ccol = 0;
			if ( *sptr != '\n' )	{
				outtext[mprintf_last_line][ccol]	= *sptr;
				ccol++;
			}
			outtext[mprintf_last_line][ccol] = '\0';
			dptr = &outtext[mprintf_last_line][ccol];
		} else {
			*dptr++ = *sptr;
			*dptr = '\0';
			ccol++;
		}
		sptr++;
	} 

	if (outwnd_disabled)
		return;

	if ( !OutputActive ) {
		int oldpos = GetScrollPos( hOutputWnd, SB_VERT );
		if ( oldpos != max_scroll_pos )	{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			si.nPos = max_scroll_pos;
			SetScrollInfo(hOutputWnd, SB_VERT, &si, 1 );
			InvalidateRect(hOutputWnd,NULL,0);
			UpdateWindow(hOutputWnd);
		} else {
			if ( nrows )	{
				RECT client;
				ScrollWindow(hOutputWnd,0,-nTextHeight*nrows,NULL,NULL);
				GetClientRect(hOutputWnd, &client);
				client.top = client.bottom - nTextHeight*(nrows+1);
				InvalidateRect(hOutputWnd,&client,0);

				UpdateWindow(hOutputWnd);
			} else {
				Outwnd_changed++;
			}
		}
	}
}

LRESULT CALLBACK outwnd_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam)
{
	switch(msg)	{
	case WM_ACTIVATEAPP:
		// The application z-ordering has change
		// foreground application wParm will be
		OutputActive = (BOOL)wParam;
		break;

	case WM_CREATE:	{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_RANGE | SIF_POS;
			si.nMin = 0;
			si.nMax = max_scroll_pos;
			si.nPos = max_scroll_pos;
			SetScrollInfo(hwnd, SB_VERT, &si, 1 );
		}
		break;

	case WM_TIMER:
		if (scroll_wait)
			PostMessage(hwnd, WM_MOUSEMOVE, 0, last_mousemove);

		if ( Outwnd_changed )	{
			RECT client;
			GetClientRect(hOutputWnd, &client);
			client.top = client.bottom - nTextHeight;
			InvalidateRect(hOutputWnd,&client,0);
		}

		scroll_wait = 0;
		break;

	case WM_COMMAND:	{
		int z;

		z = LOWORD(wParam);
		if (z >= ID_FILTER && z < ID_FILTER + (int)OutwndFilter.size())
		{
			z -= ID_FILTER;
			OutwndFilter[z].enabled = !OutwndFilter[z].enabled;

			if ( !stricmp( OutwndFilter[z].name, "error" ) )	{
				OutwndFilter[z].enabled = 1;
			} else if ( !stricmp( OutwndFilter[z].name, "general" ) )	{
				OutwndFilter[z].enabled = 1;
			} else if ( !stricmp( OutwndFilter[z].name, "warning" ) )	{
				OutwndFilter[z].enabled = 1;
			}
			save_filter_info();
			break;
		}

		switch (z)
		{
			case ID_COPY:
				outwnd_copy_marked_selection(hwnd);
				break;

			/*
			case ID_FIND:
				if (DialogBox(GetModuleHandle(NULL), "FIND_DIALOG", hOutputWnd,
					(int (__stdcall *)(void)) find_dlg_handler) == IDOK)
				{
					find_text_in_outwindow(mprintf_last_line, 0);
				}

				break;
				*/
		}
		break;
	}

	case WM_RBUTTONDOWN:	{
			HMENU h_menu = CreatePopupMenu();
			HMENU h_sub_menu = CreatePopupMenu();
			POINT pt;

			for (uint i = 0; i < OutwndFilter.size(); i++) {
				UINT flags = MFT_STRING;	//MF_GRAYED;

				if ( !stricmp( OutwndFilter[i].name, "error" ) )	{
					flags |= MF_GRAYED;
				} else if ( !stricmp( OutwndFilter[i].name, "general" ) )	{
					flags |= MF_GRAYED;
				} else if ( !stricmp( OutwndFilter[i].name, "warning" ) )	{
					flags |= MF_GRAYED;
				}

				if (OutwndFilter[i].enabled)
					AppendMenu(h_sub_menu, flags | MF_CHECKED, ID_FILTER + i, OutwndFilter[i].name);
				else
					AppendMenu(h_sub_menu, flags, ID_FILTER + i, OutwndFilter[i].name);
			}

			AppendMenu(h_menu, MFT_STRING, ID_COPY, "&Copy\tEnter");
			AppendMenu(h_menu, MFT_STRING, ID_FIND, "&Find Text");
			AppendMenu(h_menu, MF_POPUP, (unsigned int) h_sub_menu, "Filter &Messages");
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			ClientToScreen(hwnd, &pt);

			TrackPopupMenu(h_menu, 0, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(h_menu);
			break;
		}
		
	case WM_LBUTTONDOWN:
		fix_marking_coords(marking_started_x, marking_started_y, lParam);
		SetCapture(hwnd);  // monopolize mouse
		marking_active = 1;
		outwnd_update_marking(lParam, hwnd);
		break;

	case WM_MOUSEMOVE:
		last_mousemove = lParam;
		if (marking_active){
			outwnd_update_marking(lParam, hwnd);
		}
		break;

	case WM_LBUTTONUP:
		if (marking_active)
		{
			ReleaseCapture();
			marking_active = 0;
			outwnd_update_marking(lParam, hwnd);
		}
		break;
	
	case WM_VSCROLL:	{
			SCROLLINFO si;
			int vpos = GetScrollPos( hwnd, SB_VERT );
			int old_vpos=vpos;
			switch (LOWORD(wParam))	{
			case SB_LINEDOWN:
				vpos++;
				break;
			case SB_LINEUP:
				vpos--;
				break;
			case SB_THUMBPOSITION:
				vpos = HIWORD(wParam);
				break;
			case SB_THUMBTRACK:
				vpos = HIWORD(wParam);
				break;
			case SB_PAGEDOWN:
				vpos += nCharRows;
				break;
			case SB_PAGEUP:
				vpos -= nCharRows;
				break;
			}
			if ( vpos < 0 ) vpos = 0;
			else if ( vpos > max_scroll_pos ) vpos = max_scroll_pos;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			si.nPos = vpos;
			SetScrollInfo(hwnd, SB_VERT, &si, 1 );
			ScrollWindow(hwnd,0,(old_vpos-vpos)*nTextHeight,NULL,NULL);
			UpdateWindow(hOutputWnd);
			//InvalidateRect(hOutputWnd,NULL,0);
		}
		break;

	case WM_KEYDOWN:	{
			SCROLLINFO si;
			int vpos = GetScrollPos( hwnd, SB_VERT );
			int old_vpos=vpos;
			int nVirtKey = (int) wParam;  // virtual-key code
			switch(nVirtKey)	{
			case VK_DOWN:
			case VK_RIGHT:
				vpos++;
				break;
			case VK_UP:
			case VK_LEFT:
				vpos--;
				break;
			case VK_NEXT:
				vpos += nCharRows;
				break;
			case VK_PRIOR:
				vpos -= nCharRows;
				break;
			case VK_END:
				vpos = 0;
				break;
			case VK_HOME:
				vpos = max_scroll_pos;
				break;
			case VK_RETURN:
				outwnd_copy_marked_selection(hwnd);
				break;
			case UP_FAST:  // special value we define
				vpos -= 5;
				break;
			case DOWN_FAST:  // special value we define
				vpos += 5;
				break;
			}
			
			if ( vpos < 0 ) vpos = 0;
			else if ( vpos > max_scroll_pos ) vpos = max_scroll_pos;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			si.nPos = vpos;
			SetScrollInfo(hwnd, SB_VERT, &si, 1 );
			ScrollWindow(hwnd, 0, (old_vpos-vpos)*nTextHeight, NULL, NULL);
			UpdateWindow(hOutputWnd);
			//InvalidateRect(hOutputWnd,NULL,0);
		}
		break;

	case WM_SIZE:
		InvalidateRect(hOutputWnd,NULL,0);
		break;

	case WM_PAINT:
		outwnd_paint(hwnd);
		break;

	case WM_DESTROY:
		outwnd_disabled = true;
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

void outwnd_copy_marked_selection(HWND hwnd)
{
	HGLOBAL h_text;
	int i, size = 1;
	char *ptr;

	if (marked_top == marked_bottom)
	{
		size += marked_right - marked_left;

	} else {
		i = marked_top;
		size += strlen(outtext[i++]) - marked_left + 2;

		while (i < marked_bottom)
			size += strlen(outtext[i++]) + 2;

		size += marked_right;
	}

	h_text = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, size);
	ptr = (char *) GlobalLock(h_text);

	if (marked_top == marked_bottom)
	{
		i = marked_right - marked_left;
		memcpy(ptr, outtext[marked_top] + marked_left, i);
		ptr[i] = 0;

	} else {
		i = marked_top;
		strcpy(ptr, outtext[i] + marked_left);
		strcat(ptr, "\r\n");
		i++;

		while (i < marked_bottom)
		{
			strcat(ptr, outtext[i++]);
			strcat(ptr, "\r\n");
		}

		ptr[strlen(ptr) + marked_right] = 0;
		strncat(ptr, outtext[i], marked_right);
	}

	GlobalUnlock(h_text);
	OpenClipboard(hwnd);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, h_text);
	CloseClipboard();

	marked = 0;
	InvalidateRect(hwnd, NULL, 0);
}

void outwnd_paint(HWND hwnd)
{
	int i, n, x, y, len;
	int old_nrows, scroll_pos;
	HDC hdc;
	PAINTSTRUCT ps;	
	RECT client;
	TEXTMETRIC tm;
	HFONT newfont, oldfont;
	HBRUSH newbrush, oldbrush;

	Outwnd_changed = 0;

	hdc = BeginPaint(hwnd, &ps);
	GetClientRect(hOutputWnd, &client);
	newfont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	oldfont = (HFONT)SelectObject(hdc,newfont);
	
	GetTextMetrics(hdc, &tm);
	nTextHeight = tm.tmHeight + tm.tmExternalLeading;
	nTextWidth = tm.tmAveCharWidth;
	old_nrows = nCharRows;
	nCharRows = ((client.bottom-client.top)/nTextHeight)+1;

	newbrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	oldbrush = (HBRUSH)SelectObject(hdc,newbrush);
	
	y = client.bottom - nTextHeight * nCharRows;  // starting y position at top
	client_top_yoffset = y - nTextHeight;
	scroll_pos = (max_scroll_pos - GetScrollPos( hOutputWnd, SB_VERT ));
	cur_line_index = x = mprintf_last_line - scroll_pos - nCharRows;
	if (x >= marked_top && x < marked_bottom)  // starting in marked area
		text_hilight(hdc);
	else
		text_normal(hdc);
	if (scroll_pos != old_scroll_pos) {
		if (!scroll_pos)	{
			char tmp[1024];
			sprintf( tmp, "Debug Spew");
			SetWindowText( hOutputWnd, tmp );

		} else {
			char tmp[1024];
			sprintf( tmp, "Debug Spew [Scrolled back %d lines]", scroll_pos );
			SetWindowText( hOutputWnd, tmp );
		}

		old_scroll_pos = scroll_pos;
	}

	i = nCharRows;
	while (i--)
	{
		n = mprintf_last_line - scroll_pos - i;
		if (n < 0)
			n += SCROLL_BUFFER_SIZE;
	
		if (n >= 0 && n < SCROLL_BUFFER_SIZE)
		{
			len = strlen(outtext[n]);
			if (marked)
			{
				if (n == marked_top && n == marked_bottom)  // special 1 line case
				{
					if (marked_left)
						TextOut(hdc, 0, y, outtext[n], marked_left);

					text_hilight(hdc);
					x = marked_left * nTextWidth;
					TextOut(hdc, x, y, outtext[n] + marked_left, marked_right -
						marked_left);

					text_normal(hdc);
					x = marked_right * nTextWidth;
					if (marked_right < len)
						TextOut(hdc, x, y, outtext[n] + marked_right, len - marked_right);

					x = len * nTextWidth;
					TextOut(hdc, x, y, spaces, MAX_LINE_WIDTH - len);

				} else if (n == marked_top)	{  // start marked on this line
					if (marked_left)
						TextOut(hdc, 0, y, outtext[n], marked_left);

					text_hilight(hdc);
					x = marked_left * nTextWidth;

					TextOut(hdc, x, y, outtext[n] + marked_left, len - marked_left);

					x = len * nTextWidth;
					if (marked_left < MAX_LINE_WIDTH)
						TextOut(hdc, x, y, spaces, MAX_LINE_WIDTH - marked_left);

				} else if (n == marked_bottom)	{  // end marked on this line
					if (marked_right)
						TextOut(hdc, 0, y, outtext[n], marked_right);

					text_normal(hdc);
					x = marked_right * nTextWidth;
					if (marked_right < len)
						TextOut(hdc, x, y, outtext[n] + marked_right, len - marked_right);

					x = len * nTextWidth;
					TextOut(hdc, x, y, spaces, MAX_LINE_WIDTH - len);

				} else	{  // whole line marked
					TextOut(hdc, 0, y, outtext[n], len);
					x = len * nTextWidth;
					TextOut(hdc, x, y, spaces, MAX_LINE_WIDTH - len);
				}

			} else {
				TextOut(hdc, 0, y, outtext[n], len);
				x = len * nTextWidth;
				TextOut(hdc, x, y, spaces, MAX_LINE_WIDTH - len);
			}
		} else
			TextOut(hdc, 0, y, spaces, MAX_LINE_WIDTH);

		y += nTextHeight;
	}

	text_normal(hdc);
	SelectObject(hdc, oldfont);
	SelectObject(hdc, oldbrush);
	DeleteObject(newbrush);

	if ( old_nrows != nCharRows )	{
		SCROLLINFO si;
		max_scroll_pos = SCROLL_BUFFER_SIZE-nCharRows - 1;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		si.nMin = 0;
		si.nMax = max_scroll_pos;
		SetScrollInfo(hwnd, SB_VERT, &si, 1 );
	}

	EndPaint(hwnd, &ps);

}

void outwnd_update_marking(LPARAM l_parm, HWND hwnd)
{
	int x, y;
	RECT rect;

	if (!scroll_wait)	{
		y = (signed short) HIWORD(l_parm);
		GetClientRect(hwnd, &rect);
		
		if (y < -150)	{
			SendMessage(hwnd, WM_KEYDOWN, UP_FAST, 0);
			scroll_wait = 1;

		} else if (y < 0)	{
			SendMessage(hwnd, WM_KEYDOWN, VK_UP, 0);
			scroll_wait = 1;
		}

		if (y >= rect.bottom + 150)	{
			SendMessage(hwnd, WM_KEYDOWN, DOWN_FAST, 0);
			scroll_wait = 1;

		} else if (y >= rect.bottom)	{
			SendMessage(hwnd, WM_KEYDOWN, VK_DOWN, 0);
			scroll_wait = 1;
		}
	}

	fix_marking_coords(x, y, l_parm);
	marked = 1;
	marked_top = marked_bottom = marking_started_y;
	marked_left = marking_started_x;
	marked_right = marking_started_x + 1;

	if (y < marked_top)
	{
		marked_top = y;
		marked_left = x;

	} else if (y > marked_bottom)	{
		marked_bottom = y;
		marked_right = x + 1;

	} else {  // must be single line case
		if (x < marked_left)
			marked_left = x;
		if (x >= marked_right)
			marked_right = x + 1;

		if (marked_left >= (signed int) strlen(outtext[y]))
		{
			marked = 0;  // this isn't going to even show up
			return;
		}
	}

	if (marked_left >= (signed int) strlen(outtext[marked_top]))
	{
		marked_top++;
		marked_left = 0;
	}

	if (marked_right > (signed int) strlen(outtext[marked_bottom]))
		marked_right = strlen(outtext[marked_bottom]);

	if (marked && (marked_top > marked_bottom || (marked_top == marked_bottom &&
		marked_left >= marked_right)))
	{
		char msg[1024];

		sprintf(msg, "Marking limits invalid!\n"
			"(%d,%d) to (%d,%d)", marked_left, marked_top, marked_right, marked_bottom);

		MessageBox(hwnd, msg, "Error", MB_OK | MB_ICONERROR);
		marked = 0;
	}

	if (marked != old_marked || marked_top != old_marked_top || marked_bottom !=
		old_marked_bottom || marked_left != old_marked_left || marked_right !=
		old_marked_right)
	{
		old_marked = marked;
		old_marked_left = marked_left;
		old_marked_right = marked_right;
		old_marked_top = marked_top;
		old_marked_bottom = marked_bottom;
		InvalidateRect(hwnd, NULL, 0);
	}
}

BOOL outwnd_create(int display_under_freespace_window)
{
	int x;
	WNDCLASSEX wclass;
	HINSTANCE hInst = GetModuleHandle(NULL);
	RECT rect;
	DWORD style;

	wclass.hInstance 		= hInst;
	wclass.lpszClassName	= szOutputClass;
	wclass.lpfnWndProc		= (WNDPROC) outwnd_handler;
	wclass.style			= CS_BYTEALIGNCLIENT | CS_OWNDC;	//CS_DBLCLKS | CS_PARENTDC| CS_VREDRAW | CS_HREDRAW |;
	wclass.cbSize			= sizeof(WNDCLASSEX);
	wclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wclass.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);
	wclass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wclass.lpszMenuName		= NULL;	//"FreeSpaceMenu";
	wclass.cbClsExtra		= 0;
	wclass.cbWndExtra		= 0;
	wclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//(HBRUSH)NULL;

	if (!RegisterClassEx(&wclass))
		return FALSE;

	if (display_under_freespace_window)	{
		style = WS_OVERLAPPEDWINDOW;

		RECT client_rect;
		client_rect.left = client_rect.top = 0;
		client_rect.right = 640;
		client_rect.bottom = 480;
		AdjustWindowRect(&client_rect,WS_CAPTION | WS_SYSMENU,FALSE);

		int x = (GetSystemMetrics( SM_CXSCREEN )-(client_rect.right - client_rect.left))/2;
		int y = 0;
		if ( x < 0 ) x = 0;

		rect.left = x;
		rect.top = y;
		rect.right = x + client_rect.right - client_rect.left - 1;
		rect.bottom = y + client_rect.bottom - client_rect.top - 1;

		if(!Is_standalone){
			rect.top = rect.bottom;
			rect.bottom = GetSystemMetrics(SM_CYSCREEN) - TASKBAR_HEIGHT - rect.top;
			rect.right -= rect.left;
		} else {
			rect.left += 350;
			rect.right = 550;
			rect.bottom = 400;
		}	
	} else {
		style = WS_OVERLAPPEDWINDOW | WS_MINIMIZE;
		rect.top = rect.bottom = rect.left = rect.right = CW_USEDEFAULT;
	}
	
	//	Create Game Window
	hOutputWnd = CreateWindow(szOutputClass, "Debug Spew", style, rect.left,
		rect.top, rect.right, rect.bottom, NULL, NULL, hInst, NULL);

		// Show it, but don't activate it.  If you activate it, it cause problems
	// with fullscreen startups in main window.
	SetWindowPos( hOutputWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW );

	outwnd_disabled = false;
	SetTimer(hOutputWnd, TIMER1, 50, NULL);
	for (x=0; x<MAX_LINE_WIDTH; x++)
		spaces[x] = ' ';

	spaces[MAX_LINE_WIDTH] = 0;
	return TRUE;
}

DWORD outwnd_thread(int display_under_freespace_window)
{
	MSG msg;

	if (!outwnd_create(display_under_freespace_window))
		return 0;

	while (1)	{
		if (WaitMessage())	{
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))	{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (outwnd_disabled) break;
	}
	return 0;
}

void close_mono()
{
//	DeviceIoControl (mono_driver, (DWORD) IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0, &cbReturned, 0);
	if (hOutputThread)	{
		CloseHandle(hOutputThread);
		hOutputThread = NULL;
	}

	if (mono_driver) {
		CloseHandle(mono_driver);
		mono_driver = NULL;
	}
}

void outwnd_init_debug_window(int display_under_freespace_window)
{
	static bool debug_window_inited = false;

	if ( !Cmdline_debug_window || !outwnd_inited || debug_window_inited )
		return;

 	hOutputThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)outwnd_thread, (LPVOID)display_under_freespace_window, 0, &OutputThreadID);
	//SetThreadPriority(hOutputThread, THREAD_PRIORITY_TIME_CRITICAL);

#ifndef NMONO
	// set up the monochrome drivers
    if ( (mono_driver = CreateFile("\\\\.\\MONO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == ((HANDLE)-1))	{
		outwnd_printf2("Cannot get handle to monochrome driver.\n");
		mono_init();
	}

	atexit(close_mono);
#endif

	debug_window_inited = true;
}

void outwnd_init(int display_under_freespace_window)
{
	if (outwnd_inited)
		return;
/*
	if (Cmdline_debug_window) {
 		hOutputThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)outwnd_thread, (LPVOID)display_under_freespace_window, 0, &OutputThreadID);
		//SetThreadPriority(hOutputThread, THREAD_PRIORITY_TIME_CRITICAL);
#ifndef NMONO
		// set up the monochrome drivers
    	if ( (mono_driver = CreateFile("\\\\.\\MONO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == ((HANDLE)-1))	{
			outwnd_printf2("Cannot get handle to monochrome driver.\n");
			mono_init();
		}

		atexit(close_mono);
#endif
	}
*/
	if (Log_fp == NULL) {
		char pathname[MAX_PATH_LEN];

		memset( pathname, 0, sizeof(pathname) );
		snprintf(pathname, MAX_PATH_LEN-1, "%s\\%s\\%s", detect_home(), Pathtypes[CF_TYPE_DATA].path, FreeSpace_logfilename);

		Log_fp = fopen(pathname, "wb");

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

BOOL CALLBACK find_dlg_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hwnd, IDC_TEXT, find_text, 82);  // get the text to find
					EndDialog(hwnd, IDOK);
					return 1;

				case IDCANCEL:
					EndDialog(hwnd, 0);
					return 1;
			}
			break;

		case WM_INITDIALOG:
			SendDlgItemMessage(hwnd, IDC_TEXT, EM_LIMITTEXT, 80, 0);  // limit text to 80 chars
			if (GetDlgCtrlID((HWND) wParam) != IDC_TEXT) {
				SetFocus(GetDlgItem(hwnd, IDC_TEXT));
				return FALSE;
			}

			return TRUE;
	}

	return 0;
}

void find_text_in_outwindow(int n, int p)
{
	char *ptr, *str;

	str = outtext[n] + p;
	while (1)
	{
		ptr = strstr(str, find_text);
		if (ptr)
		{
			int scroll_pos, pos;

			find_pos = ptr - str;
			find_line = n;
			marked = 1;
			marked_top = marked_bottom = find_line;
			marked_left = find_pos;
			marked_right = find_pos + strlen(find_text);

			scroll_pos = (max_scroll_pos - GetScrollPos(hOutputWnd, SB_VERT));
			pos = mprintf_last_line - scroll_pos;
			if (pos < 0)
				pos += SCROLL_BUFFER_SIZE;

			pos -= n;
			if (pos < 0)
				pos += SCROLL_BUFFER_SIZE;

			Assert(pos >= 0);
			if (pos >= nCharRows - 1)  // outside of window viewport, so scroll
			{
				SCROLLINFO si;

				pos = mprintf_last_line - n - nCharRows / 2;
				if (pos < 0)
					pos += SCROLL_BUFFER_SIZE;

				if (pos > max_scroll_pos)
					pos = max_scroll_pos;

				scroll_pos = max_scroll_pos - GetScrollPos(hOutputWnd, SB_VERT);
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_POS;
				si.nPos = max_scroll_pos - pos;
				SetScrollInfo(hOutputWnd, SB_VERT, &si, 1);
				ScrollWindow(hOutputWnd, 0, (scroll_pos - pos) * nTextHeight, NULL, NULL);
			}

			InvalidateRect(hOutputWnd, NULL, 0);
			UpdateWindow(hOutputWnd);
			return;
		}

		n--;
		if (n < 0)
			n += SCROLL_BUFFER_SIZE;

		if (n == mprintf_last_line)
		{
			MessageBox(hOutputWnd, "Search text not found", "Find Error", MB_OK | MB_ICONERROR);
			find_line = -1;
			return;
		}

		str = outtext[n];
	}
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

#endif //NDEBUG
