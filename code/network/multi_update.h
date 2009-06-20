/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_AUTOUPDATE_THINGIE_HEADER_FILE
#define _FREESPACE_AUTOUPDATE_THINGIE_HEADER_FILE

// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE DEFINES/VARS
//

// operation return codes
#define MULTI_UPDATE_CONTINUE							0				// continue to next screen
#define MULTI_UPDATE_SHUTTING_DOWN					1				// freespace is exiting to the launcher
#define MULTI_UPDATE_MAIN_MENU						2				// caller should move back to the main menu


// -------------------------------------------------------------------------------------------------------------------
// MULTI UPDATE FUNCTIONS
//

// check to see if the version of FS on this machine is not recent. run in a popup
// if the versions don't check out, bail to the launcher
// see MULTI_UPDATE_* return codes, above
int multi_update_gobaby();


#endif
