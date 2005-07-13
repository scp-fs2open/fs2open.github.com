/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/
 
/*
 * $Logfile: /Freespace2/code/MissionUI/MissionLoopBrief.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:25:58 $
 * $Author: Goober5000 $
 *
 * Campaign Loop briefing screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     8/27/99 12:04a Dave
 * Campaign loop screen.
 *  
 *
 * $NoKeywords: $
 */

#ifndef __FS2_CAMPAIGN_LOOP_BRIEF_HEADER_FILE
#define __FS2_CAMPAIGN_LOOP_BRIEF_HEADER_FILE

// ---------------------------------------------------------------------------------------------------------------------------------------
// MISSION LOOP BRIEF DEFINES/VARS
//


// ---------------------------------------------------------------------------------------------------------------------------------------
// MISSION LOOP BRIEF FUNCTIONS
//

// init
void loop_brief_init();

// do
void loop_brief_do();

// close
void loop_brief_close();

#endif
