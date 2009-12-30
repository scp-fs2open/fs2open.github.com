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
char *fiction_file();
char *fiction_font();
void fiction_viewer_reset();
void fiction_viewer_load(char *filename, char *font_filename);

#endif
