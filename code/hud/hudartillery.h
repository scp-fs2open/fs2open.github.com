/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HudArtillery.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 * 
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 2     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 1     4/20/99 12:00a Dave
 * 
 * 
 * $NoKeywords: $
 */

#ifndef _FS2_HUD_ARTILLERY_HEADER_FILE
#define _FS2_HUD_ARTILLERY_HEADER_FILE

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY DEFINES/VARS
//


// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY FUNCTIONS
//

// level init
void hud_init_artillery();

// update all hud artillery related stuff
void hud_artillery_update();

// render all hud artillery related stuff
void hud_artillery_render();

#endif
