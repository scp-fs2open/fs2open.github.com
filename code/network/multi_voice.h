/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_voice.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:26 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 12    4/25/98 2:02p Dave
 * Put in multiplayer context help screens. Reworked ingame join ship
 * select screen. Fixed places where network timestamps get hosed.
 * 
 * 11    4/21/98 4:44p Dave
 * Implement Vasudan ships in multiplayer. Added a debug function to bash
 * player rank. Fixed a few rtvoice buffer overrun problems. Fixed ui
 * problem in options screen. 
 * 
 * 10    4/17/98 5:27p Dave
 * More work on the multi options screen. Fixed many minor ui todo bugs.
 * 
 * 9     4/09/98 11:01p Dave
 * Put in new multi host options screen. Tweaked multiplayer options a
 * bit.
 * 
 * 8     4/07/98 5:42p Dave
 * Put in support for ui display of voice system status (recording,
 * playing back, etc). Make sure main hall music is stopped before
 * entering a multiplayer game via ingame join.
 * 
 * 7     3/30/98 6:27p Dave
 * Put in a more official set of multiplayer options, including a system
 * for distributing netplayer and netgame settings.
 * 
 * 6     3/23/98 12:59a Lawrance
 * remove obsolete parameters from multi_voice_process_next_chunk()
 * 
 * 5     3/18/98 3:32p Dave
 * Put in a hook for streamed rtvoice data from the rtvoice system.
 * 
 * 4     3/17/98 12:30a Dave
 * Put in hud support for rtvoice. Several ui interface changes.
 * 
 * 3     3/16/98 2:35p Dave
 * Numerous bug fixes. Made the "cue sound" sound play before incoming
 * voice. 
 * 
 * 2     2/26/98 4:21p Dave
 * More robust multiplayer voice.
 * 
 * 1     2/24/98 10:12p Dave
 * Initial pass at multiplayer voice streaming.
 *  
 * $NoKeywords: $
 */

#ifndef _MULTIPLAYER_VOICE_STREAMING_HEADER_FILE
#define _MULTIPLAYER_VOICE_STREAMING_HEADER_FILE

// --------------------------------------------------------------------------------------------------
// MULTI VOICE DEFINES/VARS
//

struct header;

// voice system status defines
#define MULTI_VOICE_STATUS_IDLE					0			// nothing's happening, do nothing
#define MULTI_VOICE_STATUS_DENIED				1			// have been denied the token (show a red icon or something)
#define MULTI_VOICE_STATUS_RECORDING			2			// am currently recording (show a green icon or something)
#define MULTI_VOICE_STATUS_PLAYING				3			// playing back a stream (show another icon)

// max recording time for one stream
#define MULTI_VOICE_MAX_TIME						5000

// capabilities of this machine (make sure multi_voice_init() is called before referencing these)
extern int Multi_voice_can_record;
extern int Multi_voice_can_play;

// local muting preferences
extern int Multi_voice_local_prefs;


// --------------------------------------------------------------------------------------------------
// MULTI VOICE FUNCTIONS
//

// initialize the multiplayer voice system
void multi_voice_init();

// shutdown the multiplayer voice system
void multi_voice_close();

// reset between levels
void multi_voice_reset();

// process all voice details
void multi_voice_process();

// set the default voice quality and duration (if server passes -1, he just broadcasts the qos to all clients)
void multi_voice_set_vars(int qos,int duration);

// voice settings debug console function
void multi_voice_dcf();

// update the qos and/or duration of recording if the current setting is different from the passed in value
void multi_voice_maybe_update_vars(int new_qos,int new_duration);

// the status of the voice system - use this to determine what bitmaps to display, etc see above MULTI_VOICE_STATUS_* defines
int multi_voice_status();

// <player> sends hit bitflag settings (who he'll receive sound from, etc)
void multi_voice_set_prefs(int pref_flags);


// --------------------------------------------------------------------------------------------------
// MULTI VOICE / RTVOICE INTERFACE
//

// process the "next" chunk of standalone valid sound data from the rtvoice system
void multi_voice_process_next_chunk();


// --------------------------------------------------------------------------------------------------
// MULTI VOICE PACKET HANDLERS
//

// process an incoming voice packet of some kind or another
void multi_voice_process_packet(unsigned char *data, header *hinfo);


// --------------------------------------------------------------------------------------------------
// MULTI VOICE TESTING FUNCTIONS
//

// start recording voice locally for playback testing
void multi_voice_test_record_start();

// return if the test recording is going on
int multi_voice_test_recording();

// call this function if multi_voice_test_recording() is true to process various odds and ends of the test recording
void multi_voice_test_process();

// force stop any recording voice test
void multi_voice_test_record_stop();

// get a playback buffer handle (return -1 if none exist - bad)
int multi_voice_test_get_playback_buffer();

// return whether the last sampled chunk would have been too large to fit in a packet
int multi_voice_test_packet_tossed();

#endif
