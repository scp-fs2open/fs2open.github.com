/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/MissionUI/FictionViewer.h $
 * $Revision: 1.2 $
 * $Date: 2007-11-21 07:28:38 $
 * $Author: Goober5000 $
 *
 * Fiction Viewer briefing screen
 *
 * $Log: not supported by cvs2svn $
 *
 * $NoKeywords: $
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
void fiction_viewer_reset();
void fiction_viewer_load(char *filename);

#endif
