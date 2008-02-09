/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/MainHallTemp.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:59 $
 * $Author: penguin $
 *
 * Header file for main-hall menu code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     6/03/99 10:15p Dave
 * Put in temporary main hall screen.
 * 
 * $NoKeywords: $
 *
 */

#ifndef __TEMP_MAIN_HALL_MENU_HEADER_FILE
#define __TEMP_MAIN_HALL_MENU_HEADER_FILE

// ------------------------------------------------------------------------------------------------------------------------
// TEMP MAIN HALL DEFINES/VARS
//


// ------------------------------------------------------------------------------------------------------------------------
// TEMP MAIN HALL FUNCTIONS
//

// initialize the temporary main hall
void mht_init();

// do a frame for the main hall
void mht_do();

// close the temporary main hall
void mht_close();

#endif
