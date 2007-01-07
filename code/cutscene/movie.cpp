/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/cutscene/movie.cpp $
 * $Revision: 2.31.2.10 $
 * $Date: 2007-01-07 12:10:18 $
 * $Author: taylor $
 *
 * movie player code
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.31.2.9  2006/12/26 05:13:29  taylor
 * add support for Theora movies (the default format)
 * some little bits of cleanup
 * add second gr_clear() so that double-buffer'd visuals will work properly (ie, not flicker when playing MVE/OGG)
 *
 * Revision 2.31.2.8  2006/12/07 17:57:25  taylor
 * cleanup for movie init stuff (this will be even cleaner once the rest of the Theora code finally gets in)
 * get rid of the clear screen hack for Windows, we can handle that better now
 *
 * Revision 2.31.2.7  2006/11/15 00:24:47  taylor
 * clean up AVI movie stuff a bit:
 *  - use the default black brush for clearing the screen, it's a little less stupid this way
 *  - have the AVI player send messages back to the game rather than trying to poll for that extra crap
 *  - remove the only DivX6 fix, there is better addressed by newer window handling code
 *
 * Revision 2.31.2.6  2006/09/09 21:27:50  taylor
 * be sure to reset color and clear color before MVE playback (fix for Mantis bug #1041)
 *
 * Revision 2.31.2.5  2006/08/28 17:14:52  taylor
 * stupid, stupid, stupid...
 *  - fix AVI/MPG movie playback
 *  - fix missing campaign craziness
 *
 * Revision 2.31.2.4  2006/08/27 18:02:26  taylor
 * switch to using cf_find_file_location_ext() when looking for movies
 *
 * Revision 2.31.2.3  2006/08/19 04:14:57  taylor
 * add decoder for 8-bit MVEs
 * a basic fix for finding AVIs over MVEs, for mod dir stuff (this needs some CFILE support added to be a true fix, it's on the TODO list)
 * little bits of cleanup for old/unused code
 * make sure MVE filenames are correct in mvelib
 *
 * Revision 2.31.2.2  2006/07/13 22:06:38  taylor
 * handle non-MVE movies a bit better in OpenGL (don't get freaky with the window, don't lose input, etc.)
 * some cleanup to OpenGL window handling, to fix min/max/full issues, and try to make shutdown a little nicer
 *
 * Revision 2.31.2.1  2006/06/18 16:54:36  taylor
 * various compiler warning fixes
 *
 * Revision 2.31  2006/05/27 17:13:22  taylor
 * add NO_DIRECT3D support
 *
 * Revision 2.30  2006/05/13 06:59:48  taylor
 * MVE player (audio only works with OpenAL builds!)
 *
 * Revision 2.29  2005/12/28 22:17:01  taylor
 * deal with cf_find_file_location() changes
 * add a central parse_modular_table() function which anything can use
 * fix up weapon_expl so that it can properly handle modular tables and LOD count changes
 * add support for for a fireball TBM (handled a little different than a normal TBM is since it only changes rather than adds)
 *
 * Revision 2.28  2005/11/13 06:46:10  taylor
 * some minor movie cleanup, not what I would like but enough for now
 *
 * Revision 2.27  2005/10/16 23:15:46  wmcoolmon
 * Hardened cfile against array overflows
 *
 * Revision 2.26  2005/05/12 17:37:48  taylor
 * add an extra gr_flip() after gr_clear() to make sure the change is made active
 *
 * Revision 2.25  2005/04/24 06:52:23  wmcoolmon
 * Enabled movies in windowed mode; if this causes problems for people, they can always use -dnoshowvid
 *
 * Revision 2.24  2005/02/05 00:30:49  taylor
 * fix a few things post merge
 *
 * Revision 2.23  2004/11/27 10:42:18  taylor
 * this should get movies working again in D3D and help OGL too
 *
 * Revision 2.22  2004/07/26 20:47:26  Kazan
 * remove MCD complete
 *
 * Revision 2.21  2004/07/12 16:32:43  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.20  2004/04/26 13:09:20  taylor
 * cvs log header, mark cutscenes viewable for later, gr_flip() to fix OGL white flash
 *
 * 
 * 
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include "directx/dx8show.h"
#include "graphics/grd3dinternal.h"
#endif

#include "cutscene/movie.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "debugconsole/dbugfile.h" 
#include "cfile/cfile.h"
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "freespace2/freespace.h" // for Game_mode, etc.
#include "cutscene/mvelib.h"
#include "cutscene/oggplayer.h"
#include "menuui/mainhallmenu.h"

// to know if we are in a movie, needed for non-MVE only
bool Playing_movie = false;

#define MOVIE_NONE	-1
#define MOVIE_OGG	0
#define MOVIE_MVE	1
#define MOVIE_AVI	2
#define MOVIE_MPG	3

// This module links freespace movie calls to the actual API calls the play the movie.
// This module handles all the different requires of OS and gfx API and finding the file to play

void process_messages()
{
#ifdef _WIN32
	MSG msg;
   	while(PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE))
   	{
   		TranslateMessage(&msg);
   		DispatchMessage(&msg);
   	}
#endif
}


// filename		- file to search for
// out_name		- output, full path to file
// returns non-zero if file is found
int movie_find(char *filename, char *out_name)
{
	char full_path[MAX_PATH];
	char tmp_name[MAX_PATH];
	int size, offset = 0;
	const int NUM_EXT = 4;
	const char *movie_ext[NUM_EXT] = { ".ogg", ".mve", ".avi", ".mpg" };

	if (out_name == NULL)
		return MOVIE_NONE;


	memset( full_path, 0, sizeof(full_path) );
	memset( tmp_name, 0, sizeof(tmp_name) );

	// remove extension
	strcpy( tmp_name, filename );
	char *p = strchr(tmp_name, '.');
	if ( p ) *p = 0;

    int rc = cf_find_file_location_ext(tmp_name, NUM_EXT, movie_ext, CF_TYPE_ANY, sizeof(full_path) - 1, full_path, &size, &offset, 0);

	if (rc == MOVIE_NONE)
		return MOVIE_NONE;

	// if it's not in a packfile then we're done
	// NOTE: MVEs use CFILE internally for the player, so they can work out of VPs
	if ( ((rc == MOVIE_AVI) || (rc == MOVIE_MPG)) && (offset != 0) ) {
		mprintf(("MOVIE ERROR:  Found '%s' in a VP archive, but it can't be used from there!\n", full_path));
		rc = MOVIE_NONE;
	} else {
		strcpy( out_name, full_path );
	}


	return rc;
}

// Play one movie
bool movie_play(char *name)
{
	// mark the movie as viewable to the player when in a campaign
	// do this before anything else so that we're sure the movie is available
	// to the player even if it's not going to play right now
	if (Game_mode & GM_CAMPAIGN_MODE) {
		cutscene_mark_viewable(name);
	}

	extern int Mouse_hidden;
	extern int Is_standalone;

	if (Cmdline_nomovies || Is_standalone)
		return false;


	char full_name[MAX_PATH];
	int rc = 0;

	rc = movie_find(name, full_name);

	if (rc == MOVIE_NONE) {
		mprintf(("MOVIE ERROR: Unable to open movie file '%s' in any supported format.\n", name));
		return false;
	} else {
		DBUGFILE_OUTPUT_1("About to play: %s", full_name);
	}

#ifndef NO_DIRECT3D
	// no Theora movies in D3D mode, they don't display properly
	if ( (rc == MOVIE_OGG) && (gr_screen.mode == GR_DIRECT3D) ) {
		mprintf(("MOVIE ERROR: Theora movies are not currently supported in Direct3D mode!\n"));
		return false;
	}
#endif

	// MVE checks first since they use a different player
	if (rc == MOVIE_MVE || rc == MOVIE_OGG) {
		MVESTREAM *movie_mve = NULL;
		THEORAFILE *movie_ogg = NULL;

		// NOTE the MVE/Theora code uses CFILE, so only pass the base name and it will load up the movie properly
		if (rc == MOVIE_MVE)
			movie_mve = mve_open(name);
		else
			movie_ogg = theora_open(name);

		if (movie_mve || movie_ogg) {
			// kill all background sounds
			main_hall_pause();

			// clear the screen and hide the mouse cursor
			Mouse_hidden++;
			gr_reset_clip();
			gr_set_color(255, 255, 255);
			gr_set_clear_color(0, 0, 0);
			gr_zbuffer_clear(0);
			// clear first buffer
			gr_clear();
			gr_flip();
			// clear second buffer (may not be one, but that's ok)
			gr_clear();
			gr_flip();
			// clear third buffer (may not be one, but that's ok)
			gr_clear();

			// ready to play...
			if (movie_mve) {
				mve_init(movie_mve);
				mve_play(movie_mve);
			} else if (movie_ogg) {
				theora_play(movie_ogg);
			}

			// ... done playing, close the movie and show the cursor again
			if (movie_mve) {
				mve_shutdown();
				mve_close(movie_mve);
			} else if (movie_ogg) {
				theora_close(movie_ogg);
			}

			Mouse_hidden--;

			main_hall_unpause();

			return true;
		} else {
			// uh-oh, movie is invalid... Abory, Retry, Fail?
			mprintf(("MOVIE ERROR: Found invalid movie! (%s)\n", full_name));
			return false;
		}
	}

	// no MVE version so move on to AVI/MPG specific player code
#ifdef _WIN32
	process_messages();

#ifndef NO_DIRECT3D
	// This is a bit of a hack but it works nicely
 	if(gr_screen.mode == GR_DIRECT3D)
	{
   		GlobalD3DVars::D3D_activate = 0;
		GlobalD3DVars::lpD3DDevice->EndScene();
	  	GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL);
		d3d_lost_device();
	}
#endif

	Playing_movie = true;

	// kill all background sounds
	main_hall_pause();
	// clear the screen and hide the mouse cursor
	Mouse_hidden++;
	gr_reset_clip();
	gr_set_color(255, 255, 255);
	gr_set_clear_color(0, 0, 0);
	gr_zbuffer_clear(0);
	gr_clear();
	gr_flip();
	gr_clear(); // fullscreen under Windows appears to need this extra one

	HWND hWnd = (HWND)os_get_window();

  	process_messages();

	if (OpenClip(hWnd, full_name)) {
		do {
			// Give system threads time to run (and don't sample user input madly)
			Sleep(100);

			MSG msg;

			while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
 				TranslateMessage(&msg);

				if (msg.message == WM_ERASEBKGND)
					continue;

			//	PassMsgToVideoWindow(hWnd, msg.message, msg.wParam, msg.lParam);

				if (msg.message == WM_KEYDOWN) {
					switch (msg.wParam)
					{
						// Quits movie if escape, space or return is pressed
						case VK_ESCAPE:
						case VK_SPACE:
						case VK_RETURN:
						{
							goto Done;
						}
					}
				} else {
			   	  	DispatchMessage(&msg);
				}
			}
		}
		// Stream bit of the movie then handle windows messages
		while (dx8show_stream_movie() == false);
	}

Done:
	Mouse_hidden--;

	main_hall_unpause();

	// We finished playing the movie
	CloseClip( hWnd );

	// reset window settings (works the same for fullscreen too)
	SetForegroundWindow( hWnd );
	SetActiveWindow( hWnd );

#ifndef NO_DIRECT3D
	if (gr_screen.mode == GR_DIRECT3D) {
	 	GlobalD3DVars::D3D_activate = 1;
	}
#endif

	Playing_movie = false;

#else

	STUB_FUNCTION;

#endif

	return true;
}

bool movie_play_two(char *name1, char *name2)
{
	bool result1 = movie_play(name1);
	process_messages();
	bool result2 = movie_play(name2);

	return result1 && result2;
}
