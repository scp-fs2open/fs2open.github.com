/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#ifndef __FICTION_VIEWER_H__
#define __FICTION_VIEWER_H__

// management stuff
void fiction_viewer_init();
void fiction_viewer_close();
void fiction_viewer_do_frame(float frametime);

// fiction stuff
int mission_has_fiction();
const char *fiction_file();
const char *fiction_font();
const char *fiction_voice();
void fiction_viewer_reset();
void fiction_viewer_load(const char *filename, const char *font_filename, const char* voice_filename);

void fiction_viewer_pause();
void fiction_viewer_unpause();

#endif
