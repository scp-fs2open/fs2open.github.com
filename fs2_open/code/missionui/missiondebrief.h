/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionDebrief.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:25 $
 * $Author: penguin $
 *
 * Header file for running the debriefing
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/17/98 4:50p Andsager
 * Added debrief_assemble_optional_mission_popup_text() for single and
 * multiplayer
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     5/13/98 5:14p Allender
 * red alert support to go back to previous mission
 * 
 * 6     4/27/98 9:08p Allender
 * fix the debriefing stage problems when clients get to screen long after
 * server
 * 
 * 5     4/25/98 11:24a Allender
 * finsihed multiplayer debriefing stuff.  Work on object updates.
 * External view shoudl work in multiplayer correctly
 * 
 * 4     4/09/98 4:32p Hoffoss
 * Fixed several bugs in debriefing.
 * 
 * 3     12/30/97 6:42p Hoffoss
 * New debriefing screen implemented.
 * 
 * 2     10/24/97 6:19p Dave
 * More standalone testing/fixing. Added reliable endgame sequencing.
 * Added reliable ingame joining. Added reliable stats transfer (endgame).
 * Added support for dropping players in debriefing. Removed a lot of old
 * unused code.
 * 
 * 1     9/30/97 10:16a Lawrance
 * move files from Mission lib to MissionUI lib
 * 
 * 3     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 2     6/13/97 2:30p Lawrance
 * Added debriefings
 * 
 * 1     6/13/97 10:42a Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __MISSIONDEBRIEF_H__
#define __MISSIONDEBRIEF_H__

extern int Debrief_multi_stages_loaded;

void debrief_init();
void debrief_do_frame(float frametime);
void debrief_close();

// useful so that the server can reset the list and ship slots if a player drops
void debrief_rebuild_player_list();
void debrief_handle_player_drop();

void debrief_disable_accept();
void debrief_assemble_optional_mission_popup_text(char *buffer, char *mission_loop_desc);


// multiplayer call to set up the client side debriefings
void debrief_multi_server_stuff();
void debrief_set_multi_clients( int stage_count, int active_stages[] );

#endif /* __MISSIONDEBRIEF_H__ */
