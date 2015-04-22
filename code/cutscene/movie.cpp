/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifdef _WIN32
#include <windows.h>
#endif

#include "graphics/2d.h"
#include "globalincs/systemvars.h"
#include "cutscene/movie.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "cfile/cfile.h"
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "cutscene/mvelib.h"
#include "cutscene/oggplayer.h"

extern int Game_mode;

const char *movie_ext_list[] = { ".ogg", ".mve" };
const int NUM_MOVIE_EXT = sizeof(movie_ext_list) / sizeof(char*);


#define MOVIE_NONE	-1
#define MOVIE_OGG	0
#define MOVIE_MVE	1

// This module links freespace movie calls to the actual API calls the play the movie.
// This module handles all the different requires of OS and gfx API and finding the file to play


// filename		- file to search for
// out_name		- output, full path to file
// returns non-zero if file is found
int movie_find(char *filename, char *out_name)
{
	char full_path[MAX_PATH];
	char tmp_name[MAX_PATH];
	int size, offset = 0;

	if (out_name == NULL)
		return MOVIE_NONE;


	memset( full_path, 0, sizeof(full_path) );
	memset( tmp_name, 0, sizeof(tmp_name) );

	// remove extension
	strcpy_s( tmp_name, filename );
	char *p = strrchr(tmp_name, '.');
	if ( p ) *p = 0;

    int rc = cf_find_file_location_ext(tmp_name, NUM_MOVIE_EXT, movie_ext_list, CF_TYPE_ANY, sizeof(full_path) - 1, full_path, &size, &offset, 0);

	if (rc == MOVIE_NONE)
		return MOVIE_NONE;

	strcpy( out_name, full_path );

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

	memset(full_name, 0, sizeof(full_name));

	rc = movie_find(name, full_name);

	if (rc == MOVIE_NONE) {
		strcpy_s(full_name, name);
		char *p = strrchr(full_name, '.');
		if ( p ) *p = 0;

		mprintf(("Movie Error:  Unable to open '%s' movie in any supported format.\n", full_name));
		return false;
	}

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

	if (rc == MOVIE_OGG) {
		THEORAFILE *movie_ogg = theora_open(name);

		if (movie_ogg) {
			// start playing ...
			theora_play(movie_ogg);

			// ... done playing, close the movie
			theora_close(movie_ogg);
		} else {
			// uh-oh, movie is invalid... Abory, Retry, Fail?
			mprintf(("MOVIE ERROR: Found invalid movie! (%s)\n", name));
			Mouse_hidden--;	// show the mouse cursor!
			return false;
		}
	} else if (rc == MOVIE_MVE) {
		MVESTREAM *movie_mve = mve_open(name);

		if (movie_mve) {
			// start playing ...
			mve_init(movie_mve);
			mve_play(movie_mve);

			// ... done playing, close the movie
			mve_shutdown();
			mve_close(movie_mve);
		} else {
			// uh-oh, movie is invalid... Abory, Retry, Fail?
			mprintf(("MOVIE ERROR: Found invalid movie! (%s)\n", name));
			Mouse_hidden--;	// show the mouse cursor!
			return false;
		}
	}

	// show the mouse cursor again
	Mouse_hidden--;

	return true;
}

void movie_play_two(char *name1, char *name2)
{
	if ( movie_play(name1) )
		movie_play(name2);
}
