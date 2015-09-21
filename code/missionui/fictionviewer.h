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
const char *fiction_background(int res);
int fiction_ui_index();
const char *fiction_ui_name();
int fiction_viewer_ui_name_to_index(char *ui_name);
const char *fiction_file();
const char *fiction_font();
const char *fiction_voice();
void fiction_viewer_reset();
void fiction_viewer_load(const char *background_640, const char *background_1024, int ui_index, const char *filename, const char *font_filename, const char* voice_filename);

void fiction_viewer_pause();
void fiction_viewer_unpause();

#endif
