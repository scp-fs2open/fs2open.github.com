/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi_voice.h"
#include "io/timer.h"
#include "io/key.h"
#include "gamesequence/gamesequence.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_pmsg.h"
#include "gamesnd/gamesnd.h"
#include "sound/rtvoice.h"
#include "menuui/optionsmenumulti.h"
#include "network/multi.h"
#include "playerman/player.h"



// --------------------------------------------------------------------------------------------------
// MULTI VOICE DEFINES/VARS
//

// #define MULTI_VOICE_POST_DECOMPRESS									// when we're _not_ using streaming
#define MULTI_VOICE_PRE_DECOMPRESS										// when we _are_ using streaming

#define MULTI_VOICE_VERBOSE												// keep this defined for verbose debug output

#define MULTI_VOICE_LOCAL_ECHO											// keep this defined for local echo of recorded network voice

// flag indicating the status of the multi voice system
int Multi_voice_inited = 0;
int Multi_voice_can_record = 0;
int Multi_voice_can_play = 0;
int Multi_voice_send_mode = MULTI_MSG_NONE;							// gotten from the multi_msg system when we start recording

// packet code defines
#define MV_CODE_GIVE_TOKEN								0					// received player side - he now has the token to speak
#define MV_CODE_DENY_TOKEN								1					// received player side - server has denied this request
#define MV_CODE_TAKE_TOKEN								2					// received player side - the server is forcibly taking his token
#define MV_CODE_RELEASE_TOKEN							3					// received server side - player is relinquishing token
#define MV_CODE_REQUEST_TOKEN							4					// received server side - player is requesting token
#define MV_CODE_PLAYER_PREFS							5					// received server side - player bitflags for who he'll receive from
#define MV_CODE_DATA										6					// sound data
#define MV_CODE_DATA_DUMMY								7					// in place of a packet which has been deemed too large, so that receivers don't time out early

// default quality of sound
#define MV_DEFAULT_QOS									10					// default quality of sound
int Multi_voice_qos;															// default quality of sound

// sounds added to the front and end of a playing voice stream (set to -1 if none are wanted)
#define MULTI_VOICE_PRE_SOUND							SND_CUE_VOICE
#define MULTI_VOICE_POST_SOUND						SND_END_VOICE
int Multi_voice_pre_sound_size = 0;

// sound data

// NOTE : the following 2 defines should be used for reference only. they represent the worst case situation,
//			 sending voice to a specific target under IPX. you should use multi_voice_max_chunk_size(...) when 
//        determining if a given chunk will fit into an individual freespace packet
// max size of a data packet header (note, this changes as the code itself changes - should probably never use this except for reference)
#define MULTI_VOICE_MAX_HEADER_SIZE					22
// size of an individual chunk (CHUNK == block of data stuck into a packet), in the worst case of header size (see above)
#define MULTI_VOICE_MAX_CHUNK_SIZE					488

// total max size of an incoming or an outgoing uncompressed buffer (note this is probably too big, but we won't worry about that for now)
#define MULTI_VOICE_MAX_BUFFER_SIZE					((1<<16)+(1<<14))		// 80k

// overall size of an total accum buffer for a stream
#define MULTI_VOICE_ACCUM_BUFFER_SIZE				(1<<14)					// 16k

// how many accum buffers need to be in a total accum buffer
// NOTE : we reference MULTI_VOICE_MAX_CHUNK_SIZE here because it is worst case. ie, we'll always have enough
//        accum buffers in anything better than the worst case if we use MULTI_VOICE_MAX_CHUNK_SIZE
#define MULTI_VOICE_ACCUM_BUFFER_COUNT				(MULTI_VOICE_ACCUM_BUFFER_SIZE / MULTI_VOICE_MAX_CHUNK_SIZE)

int Multi_voice_max_time;													// current maximum recording time
char *Multi_voice_record_buffer = NULL;								// buffer for recording back voice
char *Multi_voice_playback_buffer = NULL;								// buffer for processing the accum buffer and playing the result

// DEBUG CODE
#ifdef MULTI_VOICE_POST_DECOMPRESS
	char Multi_voice_unpack_buffer[MULTI_VOICE_MAX_BUFFER_SIZE];
#endif

// the max amount of tokens we want to be floating about (max sound streams)
#define MULTI_VOICE_MAX_STREAMS						1

// voice algorithm stuff
// it would probably be good to base the timeout time on some multiple of our average ping to the server
#define MV_ALG_TIMEOUT	500												// if start get new data for a window then a pause this long, play the window
int Multi_voice_stamps[MULTI_VOICE_MAX_STREAMS];

// NOTE : this should be > then MULTI_VOICE_MAX_TIME + the time for the data to come over a network connection!!
#define MULTI_VOICE_TOKEN_TIMEOUT					7000				// timeout - server will take the token back if he does not hear from the guy in this amount of time

#define MULTI_VOICE_TOKEN_RELEASE_WAIT				(1.0f)			// wait 1 second

// the token index of a voice stream is set to one of these values, or the index of the player who has the token
#define MULTI_VOICE_TOKEN_INDEX_FREE				-1					// the token (and the stream are free)
#define MULTI_VOICE_TOKEN_INDEX_RELEASED			0xDEADBEAD		// the token has been released but the stream is still active

typedef struct voice_stream {		
	int token_status;															// status of the token (player index if a player has it) or one of the above defines
	int token_stamp;															// timestamp for the MULTI_VOICE_TOKEN_TIMEOUT

	short stream_from;														// id of the player the stream is coming from

	ubyte *accum_buffer[MULTI_VOICE_ACCUM_BUFFER_COUNT];			// accum buffer
	ubyte accum_buffer_flags[MULTI_VOICE_ACCUM_BUFFER_COUNT];	// flag indicating the existence of a given accum (sub)buffer
	ushort accum_buffer_usize[MULTI_VOICE_ACCUM_BUFFER_COUNT];	// uncompressed size of the corresponding (sub)buffer
	ushort accum_buffer_csize[MULTI_VOICE_ACCUM_BUFFER_COUNT];	// compressed size of the corresponding (sub)buffer
	double accum_buffer_gain[MULTI_VOICE_ACCUM_BUFFER_COUNT];	// gain of the corresponding (sub)buffer
		
	ubyte stream_id;															// stream id #
	fix stream_last_heard;													// last time we heard from this stream	

	fix stream_start_time;													// time the stream started playing
	int stream_snd_handle;													// sound playing instance handle
	int stream_rtvoice_handle;												// rtvoice buffer handle
} voice_stream;
voice_stream Multi_voice_stream[MULTI_VOICE_MAX_STREAMS];		// voice streams themselves

// player-side data
#define MULTI_VOICE_KEY									KEY_LAPOSTRO	// key used for realtime voice
int Multi_voice_keydown = 0;												// is the record key currently being pressed
int Multi_voice_recording = 0;											// flag indicating if we're currently recording or not
int Multi_voice_token = 0;													// if we currently have a token or not
int Multi_voice_recording_stamp = -1;									// how long we've been recording
ubyte Multi_voice_stream_id = 0;											// stream id for the stream we're currently sending
int Multi_voice_current_stream_index = 0;								// packet index of the currently recodring stream
int Multi_voice_current_stream_sent = -1;								// index of packet we've sent up to

// server-side data
ubyte Multi_voice_next_stream_id = 0;									// kept on the server - given to the next valid token requester
int Multi_voice_player_prefs[MAX_PLAYERS];							// player bitflag preferences

// voice status data - used for determing the result of multi_voice_status
#define MULTI_VOICE_DENIED_TIME						1000				// how long to display the "denied" status
int Multi_voice_denied_stamp = -1;										// timestamp for when we got denied a token

// local muting preferences
int Multi_voice_local_prefs = 0xffffffff;


// --------------------------------------------------------------------------------------------------
// MULTI VOICE FORWARD DECLARATIONS
//

// process voice details as the server
void multi_voice_server_process();

// process voice details as a player (may also be the server)
void multi_voice_player_process();

// determine if the voice key is down this frame
int multi_voice_keydown();

// find the voice stream index by token player index
int multi_voice_find_token(int player_index);

// <server> gives the token to a given player
void multi_voice_give_token(int stream_index,int player_index);

// <server> takes the token from a given stream entry
void multi_voice_take_token(int stream_index);

// <server> tells the client he's been denied on this request
void multi_voice_deny_token(int player_index);

// <player> releases the token back to the server
void multi_voice_release_token();

// <player> requests the token from the server
void multi_voice_request_token();

// <server> process a request for the token
void multi_voice_process_token_request(int player_index);

// free up any memory which may have been malloced
void multi_voice_free_all();

// <player> send the currently recorded sound
void multi_voice_player_send_stream();

// process incoming sound data, return bytes processed
int multi_voice_process_data(ubyte *data, int player_index,int msg_mode,net_player *target);

// <server> increment the current stream id#
void multi_voice_inc_stream_id();

// flush any old sound stream data because we've started to receive data for a new stream
void multi_voice_flush_old_stream(int stream_index);

// route sound data through the server to all appropriate players
void multi_voice_route_data(ubyte *data, int packet_size,int player_index,int mode,net_player *target);

// find the stream to apply incoming sound data to, freeing up old ones as necessary
int multi_voice_get_stream(int stream_id);

// NOTE : these 4 functions can be arbitrarily written to perform in any way necessary. This way the algorithm is
//        completely seperate from the transport and token layers
// initialize the smart algorithm
void multi_voice_alg_init();

// process incoming sound data in whatever way necessary (this function should take care of playing data when necessary)
void multi_voice_alg_process_data(int player_index,int stream_index,ushort chunk_index,ushort chunk_size);		

// process existing streams
void multi_voice_alg_process_streams();

// we are going to flush the current stream because we have started to receive data for a new one. do something first
void multi_voice_alg_flush_old_stream(int stream_index);

// is the given sound stream playing (compares uncompressed sound size with current playback position)
int multi_voice_stream_playing(int stream_index);

// tack on a post voice sound (pass -1 for none)
// return final buffer size
int multi_voice_mix(int post_sound,char *data,int cur_size,int max_size);

// send a dummy packet in the place of a too-large data packet
void multi_voice_send_dummy_packet();

// process a dummy data packet
int multi_voice_process_data_dummy(ubyte *data);

// max size of a sound chunk which we can fit into a packet
int multi_voice_max_chunk_size(int msg_mode);

// process a player preferences packet, return bytes processed
int multi_voice_process_player_prefs(ubyte *data,int player_index);

// process and play the current window of sound stream data we have. reset the window for the next incoming data as well
void multi_voice_alg_play_window(int stream_index);

// send all pending voice packets
void multi_voice_client_send_pending();


// --------------------------------------------------------------------------------------------------
// MULTI VOICE FUNCTIONS
//

// initialize the multiplayer voice system
void multi_voice_init()
{
	int idx,s_idx,pre_size,pre_sound;

	// if the voice system is already initialized, just reset some stuff
	if(Multi_voice_inited){
		multi_voice_reset();
		return;
	}

	// set the default quality of sound
	Multi_voice_qos = MV_DEFAULT_QOS;

	// if we're the standalone server, we can't record _or_ playback, but we can still route data and manage tokens
	if(Game_mode & GM_STANDALONE_SERVER){
		Multi_voice_can_record = 0;
		Multi_voice_can_play = 0;
	} else {
		// initialize the realtime voice module
		if(rtvoice_init_recording(Multi_voice_qos)){
			nprintf(("Network","MULTI VOICE : Error initializing rtvoice - recording will not be possible\n"));
			Multi_voice_can_record = 0;
		} else {
			Multi_voice_can_record = 1;
		}		

		if(rtvoice_init_playback()){		
			nprintf(("Network","MULTI VOICE : Error initializing rtvoice - playback will not be possible\n"));
			Multi_voice_can_play = 0;
		} else {
			Multi_voice_can_play = 1;
		}

		// _always_ set the quality of server
		multi_voice_set_vars(MV_DEFAULT_QOS,MULTI_VOICE_MAX_TIME);
	}

	// initialize player-side data
	Multi_voice_token = 0;
	Multi_voice_keydown = 0;
	Multi_voice_recording = 0;	
	Multi_voice_stream_id = 0;
	Multi_voice_recording_stamp = -1;
	Multi_voice_current_stream_index = 0;
	Multi_voice_current_stream_sent = -1;

	// initialize server-side data
	memset(Multi_voice_player_prefs,0xff,sizeof(int)*MAX_PLAYERS);				
	Multi_voice_next_stream_id = 0;

	Multi_voice_local_prefs = 0xffffffff;

	// initialize the sound buffers
	Multi_voice_record_buffer = NULL;	

	Multi_voice_playback_buffer = NULL;
	Multi_voice_pre_sound_size = 0;
	if(Multi_voice_can_play){
		// attempt to allocate the buffer
		Multi_voice_playback_buffer = (char*)vm_malloc(MULTI_VOICE_MAX_BUFFER_SIZE);
		if(Multi_voice_playback_buffer == NULL){
			nprintf(("Network","MULTI VOICE : Error allocating playback buffer - playback will not be possible\n"));		
			Multi_voice_can_play = 0;		
		} 

		// attempt to copy in the "pre" voice sound
		pre_sound = snd_load(&Snds[MULTI_VOICE_PRE_SOUND], 0);
		if(pre_sound != -1){
			// get the pre-sound size
			if((snd_size(pre_sound,&pre_size) != -1) && (pre_size < MULTI_VOICE_MAX_BUFFER_SIZE)){
				snd_get_data(pre_sound,Multi_voice_playback_buffer);
				Multi_voice_pre_sound_size = pre_size;
			} else {
				Multi_voice_pre_sound_size = 0;
			}
		} else {
			Multi_voice_pre_sound_size = 0;
		}
	}

	// initialize the streams	
	memset(Multi_voice_stream,0,sizeof(voice_stream) * MULTI_VOICE_MAX_STREAMS);	
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		Multi_voice_stream[idx].token_status = MULTI_VOICE_TOKEN_INDEX_FREE;
		Multi_voice_stream[idx].token_stamp = -1;		
		Multi_voice_stream[idx].stream_snd_handle = -1;

		// get a playback buffer handle
		if(Multi_voice_can_play){
			Multi_voice_stream[idx].stream_rtvoice_handle = -1;
			Multi_voice_stream[idx].stream_rtvoice_handle = rtvoice_create_playback_buffer();
			if(Multi_voice_stream[idx].stream_rtvoice_handle == -1){
				nprintf(("Network","MULTI VOICE : Error getting rtvoice buffer handle - playback will not be possible!\n"));
				multi_voice_free_all();	

				Multi_voice_can_play = 0;
			}
					
			// allocate the accum buffer
			for(s_idx=0;s_idx<MULTI_VOICE_ACCUM_BUFFER_COUNT;s_idx++){
				Multi_voice_stream[idx].accum_buffer[s_idx] = NULL;
				Multi_voice_stream[idx].accum_buffer[s_idx] = (ubyte*)vm_malloc(MULTI_VOICE_ACCUM_BUFFER_SIZE);
				if(Multi_voice_stream[idx].accum_buffer[s_idx] == NULL){
					nprintf(("Network","MULTI VOICE : Error allocating accum buffer - playback will not be possible\n"));
					multi_voice_free_all();
					
					Multi_voice_can_play = 0;
				}
			}
		}
	}	

	// initialize the default max time
	Multi_voice_max_time = MULTI_VOICE_MAX_TIME;

	// initialize voice status data
	Multi_voice_denied_stamp = -1;
	
	// initialize the smart algorithm
	multi_voice_alg_init();	
	
	Multi_voice_inited = 1;
}

// shutdown the multiplayer voice system
void multi_voice_close()
{
	int idx;
	
	// if the voice system isn't already initialized, don't do anything
	if(!Multi_voice_inited){
		return;
	}

	// free up buffers
	multi_voice_free_all();

	// release all the rtvoice buffers
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(Multi_voice_stream[idx].stream_rtvoice_handle != -1){
			rtvoice_free_playback_buffer(Multi_voice_stream[idx].stream_rtvoice_handle);
			Multi_voice_stream[idx].stream_rtvoice_handle = -1;
			Multi_voice_stream[idx].stream_snd_handle = -1;
		}
	}

	// close the realtime voice module
	rtvoice_close_recording();
	rtvoice_close_playback();

	Multi_voice_inited = 0;
}

// reset between levels
void multi_voice_reset()
{
	int idx;

#ifdef MULTI_VOICE_VERBOSE
	nprintf(("Network","MULTI VOICE : Resetting\n"));
#endif

	Assert(Multi_voice_inited);	

	// if we're the standalone server, we can't record _or_ playback, but we can still route data and manage tokens
	if(Game_mode & GM_STANDALONE_SERVER){
		Multi_voice_can_record = 0;
		Multi_voice_can_play = 0;
	} 

	// initialize player-side data
	Multi_voice_token = 0;
	Multi_voice_keydown = 0;
	Multi_voice_recording = 0;	
	Multi_voice_stream_id = 0;
	Multi_voice_recording_stamp = -1;

	// initialize server-side data
	memset(Multi_voice_player_prefs,0xff,sizeof(int)*MAX_PLAYERS);				
	Multi_voice_local_prefs = 0xffffffff;
	Multi_voice_next_stream_id = 0;

	// initialize the sound buffers
	Multi_voice_record_buffer = NULL;	
	
	// initialize the streams		
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		Multi_voice_stream[idx].token_status = MULTI_VOICE_TOKEN_INDEX_FREE;
		Multi_voice_stream[idx].token_stamp = -1;							
	}	
	
	// initialize the smart algorithm
	multi_voice_alg_init();	
}

// process all voice details
void multi_voice_process()
{
	int idx;
	
	// don't do anything if the voice module is not initialized
	if((!Multi_voice_inited) || !(Net_player->flags & NETINFO_FLAG_CONNECTED)){
		return;
	}		

	// send all pending voice packets
	multi_voice_client_send_pending();

	// find any playing sound streams which have finished and unmark them
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if((Multi_voice_stream[idx].stream_snd_handle != -1) && !multi_voice_stream_playing(idx)){
			Multi_voice_stream[idx].stream_snd_handle = -1;
		}
	}

	// process seperately as player or server
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_voice_server_process();
	}

	// all "players" do this, except the standalone who isn't a real player by definition
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		multi_voice_player_process();	
	}

	// everyont calls the general algorithm process function
	multi_voice_alg_process_streams();
}

// voice settings debug console function
void multi_voice_dcf()
{
	dc_get_arg(ARG_STRING);

	// set the quality of sound
	if (strcmp(Dc_arg, NOX("qos")) == 0) {
		dc_get_arg(ARG_INT);
		if(Dc_arg_type & ARG_INT){
			if((Dc_arg_int >= 1) && (Dc_arg_int <= 10) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				multi_voice_set_vars(Dc_arg_int,-1);
				dc_printf("Quality of sound : %d\n",Dc_arg_int);
			}
		}
	}
}

// the status of the voice system - use this to determine what bitmaps to display, etc see above MULTI_VOICE_STATUS_* defines
int multi_voice_status()
{
	int idx;
	int earliest;
	fix earliest_time;
	
	// if the "denied" timestamp is set, return that as the status
	if(Multi_voice_denied_stamp != -1){
		return MULTI_VOICE_STATUS_DENIED;
	}

	// if we're currently recording (has precedence over playing back a sound from somebody)
	if(Multi_voice_recording){
		return MULTI_VOICE_STATUS_RECORDING;
	}
	
	// find the stream which started playing the farthest back (if any)
	earliest = -1;
	earliest_time = -1;
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		// if we found a playing stream
		if(Multi_voice_stream[idx].stream_snd_handle != -1){
			if((earliest == -1) || (Multi_voice_stream[idx].stream_start_time < earliest_time)){
				earliest = idx;
				earliest_time = Multi_voice_stream[idx].stream_start_time;
			}
		}
	}
	// if we found a stream
	if(earliest != -1){
		return MULTI_VOICE_STATUS_PLAYING;
	}

	// system is idle
	return MULTI_VOICE_STATUS_IDLE;
}

// update the qos if the current setting is different from the passed in value
void multi_voice_maybe_update_vars(int new_qos,int new_duration)
{
	// if the current qos is different from the passed qos, set it
	if((new_qos != Multi_voice_qos) || (new_duration != Multi_voice_max_time)){
		multi_voice_set_vars(new_qos,new_duration);
	}
}


// --------------------------------------------------------------------------------------------------
// MULTI VOICE FORWARD DECLARATIONS
//

// process voice details as the server
void multi_voice_server_process()
{
	int idx;

	// process all the tokens for all the available streams
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		switch(Multi_voice_stream[idx].token_status){
		// if the token is free, so is the stream - don't do anything
		case MULTI_VOICE_TOKEN_INDEX_FREE:
			break;

		// if the token has been released - check to see if the stream is "done" (ie, can be marked as FREE once again)
		case MULTI_VOICE_TOKEN_INDEX_RELEASED:
			// if the stream_last_heard var is -1, it means we never got sound from this guy so free the token up immediately
			if(Multi_voice_stream[idx].stream_last_heard == -1){
				Multi_voice_stream[idx].token_status = MULTI_VOICE_TOKEN_INDEX_FREE;				

#ifdef MULTI_VOICE_VERBOSE
				nprintf(("Network","MULTI VOICE : freeing released token (no packets)\n"));
#endif
			} 
			// if a sufficiently long amount of time has elapsed since he released the token, free it up
			else {
				float t1,t2;
				t1 = f2fl(Multi_voice_stream[idx].stream_last_heard);
				t2 = f2fl(timer_get_fixed_seconds());
				if((t2 - t1) >= MULTI_VOICE_TOKEN_RELEASE_WAIT){
					Multi_voice_stream[idx].token_status = MULTI_VOICE_TOKEN_INDEX_FREE;

#ifdef MULTI_VOICE_VERBOSE
					nprintf(("Network","MULTI VOICE : freeing released token (time elapsed)\n"));
#endif
				}
			}
			break;

		// if the token is still being held by a player
		default :
			// if the token timestamp has elapsed, take the token back
			if((Multi_voice_stream[idx].token_stamp != -1) && timestamp_elapsed(Multi_voice_stream[idx].token_stamp)){
				Assert(Multi_voice_stream[idx].token_status != MULTI_VOICE_TOKEN_INDEX_FREE);
				multi_voice_take_token(idx);
			}				
			break;
		}
	}

	// for each netplayer, if his token wait timestamp is running, see if it has popped yet
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].s_info.voice_token_timestamp != -1) && timestamp_elapsed(Net_players[idx].s_info.voice_token_timestamp)){
			// unset it so that he can have the token again
			Net_players[idx].s_info.voice_token_timestamp = -1;
		}
	}
}

// process voice details as a player (may also be the server)
void multi_voice_player_process()
{
	// if the voice key is down for the first time this frame, send a request for the token
	if(!Multi_voice_keydown && multi_voice_keydown() && Multi_voice_can_record && !(Netgame.options.flags & MSO_FLAG_NO_VOICE)){
		// mark the key as being down
		Multi_voice_keydown = 1;

		// send a request for a token
		multi_voice_request_token();

#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : Request\n"));
#endif
	}	
	
	// if the key is still being pressed
	if(Multi_voice_keydown && multi_voice_keydown() && Multi_voice_can_record){
		// if we have the token
		if(Multi_voice_token){			
			// if we're not already recording, start recording
			if(!Multi_voice_recording){
#ifdef MULTI_VOICE_VERBOSE
				nprintf(("Network","MULTI VOICE : RECORD %d\n",(int)Multi_voice_stream_id));
#endif	
				// flush the old stream
				multi_voice_flush_old_stream(0);

				// start the recording process with the appropriate callback function
				if(rtvoice_start_recording(multi_voice_process_next_chunk, 175)){
					nprintf(("Network","MULTI VOICE : Error initializing recording!\n"));					
					return;
				}
				
				// set myself to be recording
				Multi_voice_recording = 1;

				// set the time when I started recording
				Multi_voice_recording_stamp = timestamp(Multi_voice_max_time);

				// set the current packet/chunk index to 0
				Multi_voice_current_stream_index = 0;
				Multi_voice_current_stream_sent = 0;
				
				// get the proper messaging mode
				if(Game_mode & GM_IN_MISSION){
					// in mission, paused
					if(gameseq_get_state() == GS_STATE_MULTI_PAUSED){
						Multi_voice_send_mode = MULTI_MSG_ALL;
					} 
					// in mission, unpaused
					else {
						Multi_voice_send_mode = multi_msg_mode();
					}
				} else {
					Multi_voice_send_mode = MULTI_MSG_ALL;
				}
			}

			// if we've recorded the max time allowed, send the data
			if((Multi_voice_recording_stamp != -1) && timestamp_elapsed(Multi_voice_recording_stamp)){
#ifdef MULTI_VOICE_VERBOSE
				nprintf(("Network","MULTI VOICE : timestamp popped"));
#endif
				// mark me as no longer recording
				Multi_voice_recording = 0;			
				Multi_voice_current_stream_sent = -1;

				// stop the recording process
				rtvoice_stop_recording();				
				
#ifdef MULTI_VOICE_POST_DECOMPRESS
				multi_voice_player_send_stream();
#endif

				// play my sound locally as well
#ifdef MULTI_VOICE_LOCAL_ECHO	
				multi_voice_alg_play_window(0);
#endif
				// release the token back to the server
				multi_voice_release_token();
			}
		}
	}
	// if the key has been released
	else if(Multi_voice_keydown && !multi_voice_keydown() && Multi_voice_can_record){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : Release\n"));
#endif

		// mark the kay as not being down
		Multi_voice_keydown = 0;
	
		// if we were recording, send the data
		if(Multi_voice_recording){		
			// mark me as no longer recording
			Multi_voice_recording = 0;

			Multi_voice_current_stream_sent = -1;

			// stop the recording process
			rtvoice_stop_recording();			
		
#ifdef MULTI_VOICE_POST_DECOMPRESS
			multi_voice_player_send_stream();			
#endif

			// play my sound locally as well
#ifdef MULTI_VOICE_LOCAL_ECHO	
			multi_voice_alg_play_window(0);
#endif

			// release the token back to the server
			multi_voice_release_token();
		}		
	}	

	// if the "denied" timestamp is set, but has elapsed or the user has let up on the key, set it to -1
	if((Multi_voice_denied_stamp != -1) && (timestamp_elapsed(Multi_voice_denied_stamp) || !multi_voice_keydown())){
		Multi_voice_denied_stamp = -1;
	}
}

// determine if the voice key is down this frame
int multi_voice_keydown()
{
	// if we're in the options screen, we should never allow the button to be pressed
	if(gameseq_get_state() == GS_STATE_OPTIONS_MENU){
		return 0;
	}

	// if we're pre-game, we should just be checking the keyboard bitflags
	if(!(Game_mode & GM_IN_MISSION)){	
		return (keyd_pressed[MULTI_VOICE_KEY] && !(keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT])) ? 1 : 0;
	} 

	// in-mission, paused - treat just like any other "chattable" screen.
	if(gameseq_get_state() == GS_STATE_MULTI_PAUSED){
		return (keyd_pressed[MULTI_VOICE_KEY] && !(keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT])) ? 1 : 0;
	}

	// ingame, unpaused, rely on the multi-messaging system (ingame)
	return multi_msg_voice_record();
}

// find the voice stream index by token player index
int multi_voice_find_token(int player_index)
{
	int idx;

	// look through all the existing streams
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(Multi_voice_stream[idx].token_status == player_index){
			return idx;
		}
	}

	// couldn't find it
	return -1;
}

// <server> gives the token to a given player
void multi_voice_give_token(int stream_index,int player_index)
{
	ubyte data[10],code;
	int packet_size = 0;
	
	// only the server should ever be here
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);

	// set this player as having the token	
	Multi_voice_stream[stream_index].token_status = player_index;
	
	// set the token timeout
	Multi_voice_stream[stream_index].token_stamp = timestamp(MULTI_VOICE_TOKEN_TIMEOUT);

	// set the stream id and increment the count
	Multi_voice_stream[stream_index].stream_id = Multi_voice_next_stream_id;
	multi_voice_inc_stream_id();

	// set the last heard from time to -1 to indicate we've heard no sound from this guy
	Multi_voice_stream[stream_index].stream_last_heard = -1;

#ifdef MULTI_VOICE_VERBOSE
	nprintf(("Network","MULTI VOICE : GIVE TOKEN %d\n",(int)Multi_voice_next_stream_id));	
#endif

	// if we're giving to ourself, don't send any data
	if(Net_player == &Net_players[player_index]){
		Multi_voice_token = 1;

		Multi_voice_stream_id = Multi_voice_stream[stream_index].stream_id;
	} else {
		// send the "give" packet to the guy
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_GIVE_TOKEN;
		ADD_DATA(code);

		// add the current stream id#
		ADD_DATA(Multi_voice_stream[stream_index].stream_id);

		// send reliably		
		multi_io_send_reliable(&Net_players[player_index], data, packet_size);
	}	
}

// <server> takes the token from a given player
void multi_voice_take_token(int stream_index)
{
	ubyte data[10],code;
	int packet_size = 0;

	// only the server should ever be here
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);	

	// if the index is -1, the token has probably been released to us "officially" already
	if((Multi_voice_stream[stream_index].token_status == (int)MULTI_VOICE_TOKEN_INDEX_FREE) || (Multi_voice_stream[stream_index].token_status == (int)MULTI_VOICE_TOKEN_INDEX_RELEASED)){
		Multi_voice_stream[stream_index].token_stamp = -1;
		return;
	}

	// if i'm taking from myself, don't send any data
	if(Net_player == &Net_players[Multi_voice_stream[stream_index].token_status]){
		Multi_voice_token = 0;

		// timestamp this guy so that he can't get the token back immediately
		Net_player->s_info.voice_token_timestamp = timestamp(Netgame.options.voice_token_wait);
	} else {
		// send the "take" packet to the guy
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_TAKE_TOKEN;
		ADD_DATA(code);

		// send reliably		
		multi_io_send_reliable(&Net_players[Multi_voice_stream[stream_index].token_status], data, packet_size);

		// timestamp this guy so that he can't get the token back immediately
		Net_players[Multi_voice_stream[stream_index].token_status].s_info.voice_token_timestamp = timestamp(Netgame.options.voice_token_wait);
	}

	// take the token back from the dude
	Multi_voice_stream[stream_index].token_status = MULTI_VOICE_TOKEN_INDEX_RELEASED;	
	Multi_voice_stream[stream_index].token_stamp = -1;
}

// <server> tells the client he's been denied on this request
void multi_voice_deny_token(int player_index)
{
	ubyte data[10],code;
	int packet_size = 0;

	// only the server should ever be here
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);	
	

	// if i'm denying myself, set the denied timestamp
	if(Net_player == &Net_players[player_index]){	
		Multi_voice_denied_stamp = timestamp(MULTI_VOICE_DENIED_TIME);		
	} else {
		// send the "deny" packet to the guy
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_DENY_TOKEN;
		ADD_DATA(code);

		// send reliably		
		multi_io_send_reliable(&Net_players[player_index], data, packet_size);
	}
}

// <player> releases the token back to the server
void multi_voice_release_token()
{
	ubyte data[10],code;
	int packet_size = 0;

	// I don't have the token anymore
	Multi_voice_token = 0;

	// if i'm the server, don't send any data
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// mark the token as being released
		int stream_index = multi_voice_find_token(MY_NET_PLAYER_NUM);
		Multi_voice_stream[stream_index].token_status = MULTI_VOICE_TOKEN_INDEX_RELEASED;
				
		// timestamp this guy so that he can't get the token back immediately
		Net_player->s_info.voice_token_timestamp = timestamp(Netgame.options.voice_token_wait);
	} else {
		// send the "release" packet to the server
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_RELEASE_TOKEN;
		ADD_DATA(code);

		// send reliably		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// <player> requests the token from the server
void multi_voice_request_token()
{
	ubyte data[10],code;
	int packet_size = 0;

	// if i'm the server, process the request right now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_voice_process_token_request(MY_NET_PLAYER_NUM);
	} else {
		// send the "request" packet to the server
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_REQUEST_TOKEN;
		ADD_DATA(code);

		// send reliably		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// <player> sends hit bitflag settings (who he'll receive sound from, etc)
void multi_voice_set_prefs(int pref_flags)
{
	ubyte data[MAX_PACKET_SIZE],code;
	int idx;
	int packet_size = 0;

	// set the local flags
	Multi_voice_local_prefs = pref_flags;

	// if i'm the server, set the sound prefs right now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		Multi_voice_player_prefs[MY_NET_PLAYER_NUM] = pref_flags;
	} else {
		// send the prefs to the server
		BUILD_HEADER(VOICE_PACKET);
		code = MV_CODE_PLAYER_PREFS;
		ADD_DATA(code);

		// add the address of all players being ignored
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(!(pref_flags & (1<<idx))){
				code = 0x0;
				ADD_DATA(code);

				// add the player's id
				ADD_SHORT(Net_players[idx].player_id);
			}
		}
		// add final stop byte
		code = 0xff;
		ADD_DATA(code);

		// send reliably		
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// set the default voice quality and duration (if server passes -1, he just broadcasts the qos to all clients)
void multi_voice_set_vars(int qos,int duration)
{					
	int need_update = 0;
	
	// make sure its in the right range
	if((qos > 0) && (qos <= 10)){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : SETTING QOS %d\n",qos));
#endif 

		// set the default value
		Multi_voice_qos = qos;		

		// set the value in the rtvoice module
		rtvoice_set_qos(Multi_voice_qos);

		// update the netgame settings
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){						
			Netgame.options.voice_qos = (ubyte)Multi_voice_qos;
			need_update = 1;			
		}
	}

	// set the maximum duration
	if((duration > 0) && (duration <= MULTI_VOICE_MAX_TIME)){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : SETTING MAX RECORD TIME %d\n",duration));
#endif
		// set the default value
		Multi_voice_max_time = duration;

		// update the netgame settings
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			Netgame.options.voice_record_time = duration;
			need_update = 1;
		}
	}	

	// send an options update if necessary
	if(need_update && !(Game_mode & GM_STANDALONE_SERVER)){
		multi_options_update_netgame();
	}
}

// <server> process a request for the token
void multi_voice_process_token_request(int player_index)
{
	int stream_index,idx;
	
	// if we're not doing voice on this server, return now
	if(Netgame.options.flags & MSO_FLAG_NO_VOICE){
		return;
	}

	// if the player's token timestamp is not -1, can't give him the token
	if(Net_players[player_index].s_info.voice_token_timestamp != -1){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : Not giving token because player %s's timestamp hasn't elapsed yet!\n",Net_players[player_index].m_player->callsign));
		nprintf(("Network","MULTI VOICE : token status %d\n",Multi_voice_stream[0].token_status));
#endif
		// deny the guy
		multi_voice_deny_token(player_index);
		return;
	}

	// attempt to find a free token token
	stream_index = -1;
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(Multi_voice_stream[idx].token_status == MULTI_VOICE_TOKEN_INDEX_FREE){
			multi_voice_give_token(idx,player_index);
			return;
		}
	}	
}

// free up any memory which may have been malloced
void multi_voice_free_all()
{
	int idx,s_idx;

	// free up the playback buffer
	if(Multi_voice_playback_buffer != NULL){
		vm_free(Multi_voice_playback_buffer);
		Multi_voice_playback_buffer = NULL;
	}

	// free up the accum buffers
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		for(s_idx=0;s_idx<MULTI_VOICE_ACCUM_BUFFER_COUNT;s_idx++){
			if(Multi_voice_stream[idx].accum_buffer[s_idx] != NULL){
				vm_free(Multi_voice_stream[idx].accum_buffer[s_idx]);
				Multi_voice_stream[idx].accum_buffer[s_idx] = NULL;
			}
		}
	}	
}

// <player> send the currently recorded sound
void multi_voice_player_send_stream()
{
	ubyte data[MAX_PACKET_SIZE],code,*rbuf,msg_mode,chunk_index;
	ushort chunk_size,uc_size;
	int packet_size = 0;
	int sound_size,size_sent,target_index,max_chunk_size;
	float gain;
	double d_gain;

	// we'd better not ever get here as we can't record voice
	Assert(Multi_voice_can_record);

	// get the data	
	rtvoice_get_data((unsigned char**)&Multi_voice_record_buffer, &sound_size, &d_gain);
	gain = (float)d_gain;

	msg_mode = (ubyte)Multi_voice_send_mode;
	// get the specific target if we're in MSG_TARGET mode
	target_index = -1;
	if(msg_mode == MULTI_MSG_TARGET){
		if(Player_ai->target_objnum != -1){
			target_index = multi_find_player_by_object(&Objects[Player_ai->target_objnum]);
			if(target_index == -1){
				return;
			}
		} else {
			return;
		}
	}

	// get the max chunk size
	max_chunk_size = multi_voice_max_chunk_size(Multi_voice_send_mode);

	// go through the data and send all of it
	code = MV_CODE_DATA;
	chunk_index = 0;
	size_sent = 0;
	rbuf = (unsigned char*)Multi_voice_record_buffer;	
	while(size_sent < sound_size){
		// build the header and add the opcode
		BUILD_HEADER(VOICE_PACKET);

		// add the packet code type
		ADD_DATA(code);

		// add the routing data and any necessary targeting information
		ADD_DATA(msg_mode);
		if(msg_mode == MULTI_MSG_TARGET){
			ADD_DATA(Objects[Net_players[target_index].m_player->objnum].net_signature);
		}

		// add my id#
		ADD_SHORT(Net_player->player_id);

		// add the current stream id#
		ADD_DATA(Multi_voice_stream_id);

		Assert(sound_size < MULTI_VOICE_MAX_BUFFER_SIZE);
		uc_size = (ushort)sound_size;
		ADD_USHORT(uc_size);

		// add the chunk index
		ADD_DATA(chunk_index);

		// determine how much we are going to send in this packet
		if((sound_size - size_sent) >= max_chunk_size){
			chunk_size = (ushort)max_chunk_size;
		} else {
			chunk_size = (ushort)(sound_size - size_sent);
		}
		ADD_USHORT(chunk_size);

		// add the gain
		ADD_FLOAT(gain);

		// add the chunk of data		
		memcpy(data+packet_size, rbuf,chunk_size);		
		packet_size += chunk_size;

		// send to the server or rebroadcast if I _am_ the server
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			multi_voice_route_data(data,packet_size,MY_NET_PLAYER_NUM,(int)msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);
		} else {			
			multi_io_send(Net_player, data, packet_size);
		}

		// increment the chunk_index
		chunk_index++;

		// increment bytes sent and the buffer
		size_sent += (int)chunk_size;
		rbuf += chunk_size;
	}	
}

// process incoming sound data, return bytes processed
int multi_voice_process_data(ubyte *data, int player_index,int msg_mode,net_player *target)
{
	ubyte stream_id,chunk_index;
	ushort chunk_size,uc_size;	
	short who_from;
	int stream_index;
	float gain;
	int offset = 0;

	// read in all packet data except for the sound chunk itself
	GET_SHORT(who_from);
	GET_DATA(stream_id);
	GET_USHORT(uc_size);
	GET_DATA(chunk_index);
	GET_USHORT(chunk_size);
	GET_FLOAT(gain);				

	// if our netgame options are currently set for no voice, ignore the packet
	if((Netgame.options.flags & MSO_FLAG_NO_VOICE) || !Multi_options_g.std_voice){
		offset += chunk_size;
		return offset;
	}

	// get a handle to a valid stream to be using, freeing old streams as necessary
	stream_index = multi_voice_get_stream((int)stream_id);	

	// if this index is too high, flush the stream
	if(chunk_index >= MULTI_VOICE_ACCUM_BUFFER_COUNT){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : flushing stream because packet index is too high!!\n"));
#endif
		
		// flush the stream
		multi_voice_flush_old_stream(stream_index);

		// return bytes processed
		offset += chunk_size;
		return offset;
	}

	// if we found a stream to work with
	if(stream_index != -1){
		// set the id of where it came from
		Multi_voice_stream[stream_index].stream_from = who_from;		

		// set the stream id#
		Multi_voice_stream[stream_index].stream_id = stream_id;

		// set the gain
		Multi_voice_stream[stream_index].accum_buffer_gain[chunk_index] = (double)gain;			

		// set the stream uncompressed size size
		Multi_voice_stream[stream_index].accum_buffer_usize[chunk_index] = uc_size;			

		// set the token timestamp
		Multi_voice_stream[stream_index].token_stamp = timestamp(MULTI_VOICE_TOKEN_TIMEOUT);

		// set the last heard time
		Multi_voice_stream[stream_index].stream_last_heard = timer_get_fixed_seconds();
	
		// copy the data and setup any other accum buffer data necessary
		// ignore data if we can't play sounds
		if(Multi_voice_can_play){
			memcpy(Multi_voice_stream[stream_index].accum_buffer[chunk_index],data+offset,(int)chunk_size);
		}

		Multi_voice_stream[stream_index].accum_buffer_flags[chunk_index] = 1;
		Multi_voice_stream[stream_index].accum_buffer_csize[chunk_index] = chunk_size;			
			
		// pass the data into the smart voice algorithm
		if(player_index != -1){
			multi_voice_alg_process_data(player_index,stream_index,chunk_index,chunk_size);		
		}
	}

	// increment the offset
	offset += (int)chunk_size;					

	return offset;
}

// <server> increment the current stream id#
void multi_voice_inc_stream_id()
{
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);
	
	if(Multi_voice_next_stream_id == 0xff){
		Multi_voice_next_stream_id = 0;
	} else {
		Multi_voice_next_stream_id++;
	}
}

// flush any old sound stream data because we've started to receive data for a new stream
void multi_voice_flush_old_stream(int stream_index)
{		
#ifdef MULTI_VOICE_VERBOSE
	nprintf(("Network","MULTI VOICE : old stream flush\n"));		
#endif

	// call the smart algorithm for flushing streams
	multi_voice_alg_flush_old_stream(stream_index);
	
	// clear all the accum buffer flags
	memset(Multi_voice_stream[stream_index].accum_buffer_flags,0,MULTI_VOICE_ACCUM_BUFFER_COUNT);

	// clear the token 
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_voice_take_token(stream_index);
	}

	Multi_voice_stream[stream_index].token_stamp = -1;
	Multi_voice_stream[stream_index].token_status = MULTI_VOICE_TOKEN_INDEX_FREE;

	// timestamp the player
	Net_player->s_info.voice_token_timestamp = timestamp(Netgame.options.voice_token_wait);
}

// route sound data through the server to all appropriate players
void multi_voice_route_data(ubyte *data, int packet_size,int player_index,int mode,net_player *target)
{
	int idx;

	// route the data to all other players
	switch(mode){
	case MULTI_MSG_ALL:
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED( Net_players[idx] ) &&													// player is connected
			  ( &Net_players[idx] != &Net_players[player_index] ) &&								// not the sending player
			  ( Net_player != &Net_players[idx] ) &&													// not me
			  ( Multi_voice_player_prefs[idx] & (1 << player_index) ) &&						// is accepting sound from this player
			  !( Net_players[idx].p_info.options.flags & MLO_FLAG_NO_VOICE ) ){				// is accepting sound periods
							
				multi_io_send(&Net_players[idx], data, packet_size);
			}
		}
		break;
	
	case MULTI_MSG_FRIENDLY:
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED( Net_players[idx] ) &&													// player is connected
			  ( &Net_players[idx] != &Net_players[player_index] ) &&								// not the sending player
			  ( Net_player != &Net_players[idx] ) &&													// not me
			  ( Net_players[idx].p_info.team == Net_players[player_index].p_info.team ) &&// on the same team
			  ( Multi_voice_player_prefs[idx] & (1 << player_index) ) &&						// is accepting sound from the sender
			  !( Net_players[idx].p_info.options.flags & MLO_FLAG_NO_VOICE) ){				// is accepting sound periods
						
				multi_io_send(&Net_players[idx], data, packet_size);
			}
		}
		break;
	case MULTI_MSG_HOSTILE:
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED( Net_players[idx] ) &&													// player is connected
			  ( &Net_players[idx] != &Net_players[player_index] ) &&								// not the sending player	
			  ( Net_player != &Net_players[idx] ) &&													// not me
			  ( Net_players[idx].p_info.team != Net_players[player_index].p_info.team ) &&// on the opposite team
			  ( Multi_voice_player_prefs[idx] & (1 << player_index) ) &&						// is accepting sound from the sender
			  !( Net_players[idx].p_info.options.flags & MLO_FLAG_NO_VOICE ) ){				// is accepting sound periods
							
				multi_io_send(&Net_players[idx], data, packet_size);
			}
		}
		break;
	
	case MULTI_MSG_TARGET:
		Assert(target != NULL);
		if(!(target->p_info.options.flags & MLO_FLAG_NO_VOICE)){					
			multi_io_send(target, data, packet_size);
		}
		break;
	}
}

// find the stream to apply incoming sound data to, freeing up old ones as necessary
int multi_voice_get_stream(int stream_id)
{
	int idx,max_diff_index;
	fix cur_time,max_diff;

	// first check to see if this stream exists
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(Multi_voice_stream[idx].stream_id == (ubyte)stream_id){
			return idx;
		}
	}

	// if we got to this point, we didn't find the matching stream, so we should try and find an empty stream
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(Multi_voice_stream[idx].token_stamp == -1){
			return idx;
		}
	}

#ifdef MULTI_VOICE_VERBOSE
	nprintf(("Network","MULTI VOICE : going to blast old voice stream while looking for a free one - beware!!\n"));
#endif

	// if we got to this point, we should free up the oldest stream we have
	cur_time = timer_get_fixed_seconds();
	max_diff_index = -1;
	max_diff = -1;
	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		if(((max_diff_index == -1) || ((cur_time - Multi_voice_stream[idx].stream_last_heard) > max_diff)) && (Multi_voice_stream[idx].token_stamp != -1)){
			max_diff_index = idx;
			max_diff = cur_time - Multi_voice_stream[idx].stream_last_heard;			
		}
	}

	// if we found the oldest 
	if(max_diff_index != -1){
		// flush the old stream
		multi_voice_flush_old_stream(max_diff_index);		
		
		return max_diff_index;
	}

	// some other fail condition
	return -1;
}

// is the given sound stream playing (compares uncompressed sound size with current playback position)
int multi_voice_stream_playing(int stream_index)
{
	// if the handle is invalid, it can't be playing
	/*
	if(Multi_voice_stream[stream_index].stream_snd_handle < 0){
		return 0;
	}

	// if the sound is playing and the buffer is past the uncompressed size, its effectively done	
	if(ds_get_play_position(ds_get_channel(Multi_voice_stream[stream_index].stream_snd_handle)) >= (DWORD)Multi_voice_stream[stream_index].stream_uc_size){
		return 1;
	}
	*/

	// not done yet
	return 0;
}

// tack on pre and post sounds to a sound stream (pass -1 for either if no sound is wanted)
// return final buffer size
int multi_voice_mix(int post_sound,char *data,int cur_size,int max_size)
{
	int post_size;
	
	// if the user passed -1 for both pre and post sounds, don't do a thing
	if(post_sound == -1){
		return cur_size;
	}

	// get the sizes of the additional sounds
	
	// post sound
	if(post_sound >= 0){
		post_sound = snd_load(&Snds[post_sound], 0);
		if(post_sound >= 0){
			if(snd_size(post_sound,&post_size) == -1){
				post_size = 0;
			}
		} else {
			post_size = 0;
		}
	} else {
		post_size = 0;
	}
			
	// if we have a "post" sound to add
	if(post_size > 0){
		if((max_size - cur_size) > post_size){
			// copy in the sound
			snd_get_data(post_sound,data + cur_size);

			// increment the cur_size
			cur_size += post_size;
		}
	}

	// return the size of the new buffer
	return cur_size;
}

// max size of a sound chunk which we can fit into a packet
int multi_voice_max_chunk_size(int msg_mode)
{
	int header_size;

	// all headers contain the following data
	header_size =	1 +									// messaging mode
						1 +									// stream id #
						2 +									// packet uncompressed size
						2 +									// compressed size
						4;										// gain 

	// if we're targeting a specific player
	if(msg_mode == MULTI_MSG_TARGET){
		header_size += 2;									// targeted player's object net_signature
	}
	
	// if we're in IPX mode
	if(Psnet_my_addr.type == NET_IPX){
		header_size += 10;								// my address (10 bytes in IPX)		
	}
	// if we're in TCP mode
	else {
		header_size +=	4;									// my address (4 bytes in TCP)
	}

	// calculate max chunk size
	return (MAX_PACKET_SIZE -							// max freespace packet size
			  1					-							// packet type 
			  1					-							// voice packet code subtype
			  header_size);								// calculated header size
}

// --------------------------------------------------------------------------------------------------
// MULTI VOICE / RTVOICE INTERFACE
//

// process the "next" chunk of standalone valid sound data from the rtvoice system
void multi_voice_process_next_chunk()
{			
	int sound_size;
	float gain;
	double d_gain;
	voice_stream *str;

	// we'd better not ever get here is we can't record voice
	Assert(Multi_voice_can_record);

	// get the data	
	rtvoice_get_data((unsigned char**)&Multi_voice_record_buffer, &sound_size, &d_gain);		
	gain = (float)d_gain;

	// if we've reached the max # of packets for this stream, bail
	if(Multi_voice_current_stream_index >= (MULTI_VOICE_ACCUM_BUFFER_COUNT - 1)){
		nprintf(("Network","MULTI VOICE : Forcing stream to stop on the record size!!!\n"));

		// mark me as no longer recording
		Multi_voice_recording = 0;			

		Multi_voice_current_stream_sent = -1;
				
		// stop the recording process
		rtvoice_stop_recording();				
				
#ifdef MULTI_VOICE_POST_DECOMPRESS
		multi_voice_player_send_stream();
#endif

		// play my sound locally as well
#ifdef MULTI_VOICE_LOCAL_ECHO	
		multi_voice_alg_play_window(0);
#endif
		// release the token back to the server
		multi_voice_release_token();

		// unset the timestamp so we don't still think we're still recording
		Multi_voice_recording_stamp = -1;

		return;
	}

	// pack the data locally as well (so I can hear myself)
	str = &Multi_voice_stream[0];
	memcpy(str->accum_buffer[Multi_voice_current_stream_index],Multi_voice_record_buffer,sound_size);
	str->stream_from = Net_player->player_id;	
	str->accum_buffer_flags[Multi_voice_current_stream_index] = 1;
	str->accum_buffer_usize[Multi_voice_current_stream_index] = (ushort)sound_size;
	str->accum_buffer_csize[Multi_voice_current_stream_index] = (ushort)sound_size;
	str->accum_buffer_gain[Multi_voice_current_stream_index] = d_gain;	

	// increment the stream index
	Multi_voice_current_stream_index++;
}


// --------------------------------------------------------------------------------------------------
// MULTI VOICE PACKET HANDLERS
//

// send a dummy packet in the place of a too-large data packet
void multi_voice_send_dummy_packet()
{
	ubyte data[10],code,msg_mode;
	int packet_size,target_index;	

	// build the header and add the opcode
	BUILD_HEADER(VOICE_PACKET);
	code = (ubyte)MV_CODE_DATA_DUMMY;
	ADD_DATA(code);

	msg_mode = (ubyte)Multi_voice_send_mode;
	// get the specific target if we're in MSG_TARGET mode
	target_index = -1;
	if(msg_mode == MULTI_MSG_TARGET){
		if(Player_ai->target_objnum != -1){
			target_index = multi_find_player_by_object(&Objects[Player_ai->target_objnum]);
			if(target_index == -1){
				return;
			}
		} else {
			return;
		}
	}
	ADD_DATA(msg_mode);
	if(msg_mode == MULTI_MSG_TARGET){
		ADD_USHORT(Objects[Net_players[target_index].m_player->objnum].net_signature);
	}

	// add the voice stream id
	ADD_DATA(Multi_voice_stream_id);

	// send to the server or rebroadcast if I _am_ the server
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_voice_route_data(data,packet_size,MY_NET_PLAYER_NUM,(int)msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);
	} else {		
		multi_io_send(Net_player, data, packet_size);
	}	
}

// process a dummy data packet
int multi_voice_process_data_dummy(ubyte *data)
{
	int offset = 0;
	int stream_index;
	ubyte stream_id;

	// get the stream id
	GET_DATA(stream_id);

	// get the proper stream index
	stream_index = multi_voice_get_stream((int)stream_id);

	// set the token timestamp
	Multi_voice_stream[stream_index].token_stamp = timestamp(MULTI_VOICE_TOKEN_TIMEOUT);

	// set the last heard time
	Multi_voice_stream[stream_index].stream_last_heard = timer_get_fixed_seconds();

	// set the timeout timestamp
	Multi_voice_stamps[stream_index] = timestamp(MV_ALG_TIMEOUT);	

	// return bytes processed
	return offset;
}

// process a player preferences packet, return bytes processed
int multi_voice_process_player_prefs(ubyte *data,int player_index)
{
	ubyte val;
	int mute_index;
	short mute_id;
	int offset = 0;

	// set all channels active
	Multi_voice_player_prefs[player_index] = 0xffffffff;

	// get all muted players
	GET_DATA(val);
	while(val != 0xff){
		GET_SHORT(mute_id);

		// get the player to mute
		mute_index = find_player_id(mute_id);
		if(mute_index != -1){
#ifdef MULTI_VOICE_VERBOSE
			nprintf(("Network","Player %s muting player %s\n",Net_players[player_index].m_player->callsign,Net_players[mute_index].m_player->callsign));
#endif
			// mute the guy
			Multi_voice_player_prefs[player_index] &= ~(1<<mute_index);
		}

		// get the next stop value
		GET_DATA(val);
	}

	// return bytes processed
	return offset;
}

// process an incoming voice packet of some kind or another
void multi_voice_process_packet(ubyte *data, header *hinfo)
{
	ubyte code,msg_mode;
	ushort target_sig;	
	int player_index,stream_index,target_index;	
	int offset = HEADER_LENGTH;	

	// find out who is sending this data	
	player_index = find_player_id(hinfo->id);		

	// get the opcode
	GET_DATA(code);

	// process the packet
	switch(code){
	// I don't have the token anymore
	case MV_CODE_TAKE_TOKEN:
		// we should never have the token if we cannot record
		if(!Multi_voice_can_record){
			Int3();
		}

		Multi_voice_token = 0;
		break;

	// I have been denied the token
	case MV_CODE_DENY_TOKEN:
		// set the "denied" timestamp
		Multi_voice_denied_stamp = timestamp(MULTI_VOICE_DENIED_TIME);
		break;
	
	// I now have the token
	case MV_CODE_GIVE_TOKEN:		
		GET_DATA(Multi_voice_stream_id);

		// we should never get the token if we cannot record
		if(!Multi_voice_can_record){
			Int3();
		}

		// if we no longer have the keydown, automatically release the token
		if(!Multi_voice_keydown){
			multi_voice_release_token();
		} else {
			Multi_voice_token = 1;
		}
		break;

	// a request for the token from a player
	case MV_CODE_REQUEST_TOKEN:
		if(player_index >= 0){
			multi_voice_process_token_request(player_index);
		}
		break;
	
	// a player gave up the token
	case MV_CODE_RELEASE_TOKEN:		
		if(player_index >= 0){
			stream_index = multi_voice_find_token(player_index);
		} else {
			break;
		}
		
		if(stream_index >= 0){
			// set the token as having been released		
			Multi_voice_stream[stream_index].token_status = MULTI_VOICE_TOKEN_INDEX_RELEASED;		

			// timestamp this guy so that he can't get the token back immediately
			Net_players[player_index].s_info.voice_token_timestamp = timestamp(Netgame.options.voice_token_wait);
		} 
		break;

	// a player has set prefs for himself
	case MV_CODE_PLAYER_PREFS:
		Assert(player_index != -1);		
		offset += multi_voice_process_player_prefs(data+offset,player_index);
		break;

	// a data packet
	case MV_CODE_DATA:
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","VOICE : PROC DATA\n"));
#endif
		// get routing information
		target_index = -1;
		GET_DATA(msg_mode);
		if(msg_mode == MULTI_MSG_TARGET){
			GET_USHORT(target_sig);
			target_index = multi_find_player_by_net_signature(target_sig);
			Assert(target_index != -1);
		}

		offset += multi_voice_process_data(data+offset,player_index,msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);

		// if we're the server of the game, we should also route this data to all other players
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			multi_voice_route_data(data,offset,player_index,msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);
		}
		break;	

	// a data dummy packet
	case MV_CODE_DATA_DUMMY:
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","VOICE : PROC DATA DUMMY\n"));
#endif
		// get routing information
		target_index = -1;
		GET_DATA(msg_mode);
		if(msg_mode == MULTI_MSG_TARGET){
			GET_USHORT(target_sig);
			target_index = multi_find_player_by_net_signature(target_sig);
			Assert(target_index != -1);
		}

		offset += multi_voice_process_data_dummy(data+offset);

		// if we're the server of the game, we should also route this data to all other players
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			multi_voice_route_data(data,offset,player_index,msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);
		}
		break;
	}
	PACKET_SET_SIZE();
}

// send all pending voice packets
void multi_voice_client_send_pending()
{
	ubyte data[MAX_PACKET_SIZE],code;
	ubyte msg_mode,chunk_index;
	ushort uc_size,chunk_size;
	int max_chunk_size,sent,target_index;
	int packet_size;
	float gain;
	voice_stream *str;
	
	// if we're not recording
	if(!Multi_voice_recording || (Multi_voice_current_stream_sent < 0) || (Multi_voice_current_stream_sent > Multi_voice_current_stream_index)){
		return;
	}

	// stream all buffered up packets
	str = &Multi_voice_stream[0];
	while(Multi_voice_current_stream_sent < Multi_voice_current_stream_index){		
		sent = Multi_voice_current_stream_sent++;

		// get the current messaging mode
		msg_mode = (ubyte)Multi_voice_send_mode;		

		// if the size of this voice chunk will fit in the packet
		max_chunk_size = multi_voice_max_chunk_size(Multi_voice_send_mode);
		if(str->accum_buffer_csize[sent] > max_chunk_size){
#ifdef MULTI_VOICE_VERBOSE
			nprintf(("Network","MULTI VOICE : streamed packet size too large!!\n"));
#endif

			Multi_voice_current_stream_sent++;

			// send a dummy data packet instead
			multi_voice_send_dummy_packet();
			
			continue;
		}

#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : PACKET %d %d\n",(int)str->accum_buffer_csize[sent],(int)str->accum_buffer_usize[sent]));
#endif
	
		// get the specific target if we're in MSG_TARGET mode
		target_index = -1;
		if(msg_mode == MULTI_MSG_TARGET){
			if(Player_ai->target_objnum != -1){
				target_index = multi_find_player_by_object(&Objects[Player_ai->target_objnum]);
				if(target_index == -1){
					return;
				}
			} else {
				return;
			}
		}

		// go through the data and send all of it
		code = MV_CODE_DATA;
		chunk_index = 0;		
	
		// if this packet is small enough to fit within a psnet data packet		
		BUILD_HEADER(VOICE_PACKET);

		// add the packet code type
		ADD_DATA(code);

		// add the routing data and any necessary targeting information
		ADD_DATA(msg_mode);
		if(msg_mode == MULTI_MSG_TARGET){
			Assert(Game_mode & GM_IN_MISSION);
			ADD_USHORT(Objects[Net_players[target_index].m_player->objnum].net_signature);
		}

		// add my address 
		ADD_SHORT(Net_player->player_id);

		// add the current stream id#
		ADD_DATA(Multi_voice_stream_id);

		Assert(str->accum_buffer_usize[sent] < MULTI_VOICE_MAX_BUFFER_SIZE);
		uc_size = (ushort)str->accum_buffer_usize[sent];
		ADD_USHORT(uc_size);

		// add the chunk index
		chunk_index = (ubyte)sent;
		ADD_DATA(chunk_index);

		// size of the sound data
		chunk_size = (ushort)str->accum_buffer_csize[sent];		
		ADD_USHORT(chunk_size);

		// add the gain
		gain = (float)str->accum_buffer_gain[sent];
		ADD_FLOAT(gain);

		// add the chunk of data		
		memcpy(data+packet_size, str->accum_buffer[sent],chunk_size);		
		packet_size += chunk_size;

		// send to the server or rebroadcast if I _am_ the server
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
			multi_voice_route_data(data,packet_size,MY_NET_PLAYER_NUM,(int)msg_mode,(target_index == -1) ? NULL : &Net_players[target_index]);
		} else {			
			multi_io_send(Net_player, data, packet_size);
		}	
	}
}


// --------------------------------------------------------------------------------------------------
// MULTI VOICE ALGORITHM stuff
//

// process and play the current window of sound stream data we have. reset the window for the next incoming data as well
void multi_voice_alg_play_window(int stream_index)
{
	int idx,buffer_offset;	
	voice_stream *st;

#ifdef MULTI_VOICE_VERBOSE
	nprintf(("Network","MULTI VOICE : PLAYING STREAM %d\n",stream_index));
#endif

	// get a pointer to the stream
	st = &Multi_voice_stream[stream_index];

	// don't play anything back if we can't hear sound
	if(Multi_voice_can_play){	
		// first, pack all the accum buffers into the playback buffer
#ifdef MULTI_VOICE_PRE_DECOMPRESS
		buffer_offset = Multi_voice_pre_sound_size;
		nprintf(("Network","VOICE : pre sound size %d\n",Multi_voice_pre_sound_size));
		for(idx=0;idx<MULTI_VOICE_ACCUM_BUFFER_COUNT;idx++){
			// if the flag is set, uncompress the data into the playback buffer
			if(st->accum_buffer_flags[idx]){
				// first, uncompress the data
				rtvoice_uncompress(st->accum_buffer[idx],(int)st->accum_buffer_csize[idx],st->accum_buffer_gain[idx],(ubyte*)Multi_voice_playback_buffer+buffer_offset,st->accum_buffer_usize[idx]);
			
				// increment the buffer offset
				buffer_offset += st->accum_buffer_usize[idx];
			}
		}				
#endif
#ifdef MULTI_VOICE_POST_DECOMPRESS
		buffer_offset = 0;
		for(idx=0;idx<MULTI_VOICE_ACCUM_BUFFER_COUNT;idx++){
			// if the flag is set, copy the data
			if(st->accum_buffer_flags[idx]){
				memcpy(Multi_voice_unpack_buffer+buffer_offset,st->accum_buffer[idx],st->accum_buffer_csize[idx]);
				buffer_offset += st->accum_buffer_csize[idx];
			}
		}	

		// decompress the whole shebang
		rtvoice_uncompress((ubyte*)Multi_voice_unpack_buffer,buffer_offset,st->accum_buffer_gain[0],(ubyte*)Multi_voice_playback_buffer,st->accum_buffer_usize[0]);
		buffer_offset = st->accum_buffer_usize[0];
#endif		

		// mix in the SND_CUE_VOICE and the SND_END_VOICE game sounds
		buffer_offset = multi_voice_mix(MULTI_VOICE_POST_SOUND,Multi_voice_playback_buffer,buffer_offset,MULTI_VOICE_MAX_BUFFER_SIZE);
			
		Assert(Multi_voice_stream[stream_index].stream_rtvoice_handle != -1);

		// kill any previously playing sounds
		rtvoice_stop_playback(Multi_voice_stream[stream_index].stream_rtvoice_handle);	
		Multi_voice_stream[stream_index].stream_snd_handle = -1;

		// if we can play sound and we know who this is from, display it
		if(Multi_voice_can_play){
			char voice_msg[256];
			int player_index = find_player_id(Multi_voice_stream[stream_index].stream_from);

			if(player_index != -1){
				memset(voice_msg,0,256);
				sprintf(voice_msg,XSTR("<%s is speaking>",712),Net_players[player_index].m_player->callsign);

				// display a chat message (write to the correct spot - hud, standalone gui, chatbox, etc)
				multi_display_chat_msg(voice_msg,player_index,0);
			}
		}
	
		// now call the rtvoice playback functions		
		Multi_voice_stream[stream_index].stream_snd_handle = rtvoice_play(Multi_voice_stream[stream_index].stream_rtvoice_handle,(unsigned char*)Multi_voice_playback_buffer,buffer_offset);	
		Multi_voice_stream[stream_index].stream_start_time = timer_get_fixed_seconds();
	}
	
	// unset the stamp so that its not "free"
	Multi_voice_stamps[stream_index] = -1;

	// flush the stream (will also grab the token back, if the server)
	multi_voice_flush_old_stream(stream_index);
}

// decision function which decides if we should play the current block of sound we have
int multi_voice_alg_should_play(int stream_index)
{
	// if the timestamp has expired, play the sound
	if((Multi_voice_stamps[stream_index] != -1) && timestamp_elapsed(Multi_voice_stamps[stream_index])){
#ifdef MULTI_VOICE_VERBOSE
		nprintf(("Network","MULTI VOICE : DECIDE, TIMEOUT\n"));		
#endif
		return 1;
	}
			
	return 0;
}

// process incoming sound data in whatever way necessary (this function should take care of playing data when necessary)
void multi_voice_alg_process_data(int player_index,int stream_index,ushort chunk_index,ushort chunk_size)
{
	// do this so we don't get compiler warnings
	chunk_index = 0;
	chunk_size = 0;
	player_index = 0;

	// update the timestamp for this window
	Multi_voice_stamps[stream_index] = timestamp(MV_ALG_TIMEOUT);	
}

// process existing streams
void multi_voice_alg_process_streams()
{
	int idx;
	int player_index;

	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){			
		// determine if we should play this window of data
		if((Multi_voice_stamps[idx] != -1) && multi_voice_alg_should_play(idx)){
			// determine who this stream came from
			player_index = find_player_id(Multi_voice_stream[idx].stream_from);			

			// server should check his own settings here
			if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && ((Net_player->p_info.options.flags & MLO_FLAG_NO_VOICE) || (player_index == -1) || !(Multi_voice_player_prefs[MY_NET_PLAYER_NUM] & (1<<player_index))) ){
				// unset the stamp so that its not "free"
				Multi_voice_stamps[idx] = -1;

				// flush the stream (will also grab the token back, if the server)
				multi_voice_flush_old_stream(idx);

				nprintf(("Network","Server not playing sound because of set options!\n"));
			}
			// play the current sound
			else {				
				multi_voice_alg_play_window(idx);			
			}
		}
	}
}

// we are going to flush the current stream because we have started to receive data for a new one. do something first
void multi_voice_alg_flush_old_stream(int stream_index)
{
	// just unset the heard from timestamp for now
	Multi_voice_stamps[stream_index] = -1;
}

// initialize the smart algorithm
void multi_voice_alg_init()
{
	int idx;

	for(idx=0;idx<MULTI_VOICE_MAX_STREAMS;idx++){
		Multi_voice_stamps[idx] = -1;
	}
}


// --------------------------------------------------------------------------------------------------
// MULTI VOICE TESTING FUNCTIONS
//

#define MV_TEST_RECORD_TIME				3000					// recording time in ms for testing voice
int Multi_voice_test_record_stamp = -1;
int Multi_voice_test_packet_tossed = 0;

// process the next chunk of voice data
void multi_voice_test_process_next_chunk()
{
	unsigned char *outbuf;
	int size;
	double gain;
	
	// if the test recording stamp is -1, we should stop
	if(Multi_voice_test_record_stamp == -1){
		rtvoice_stop_recording();
		return;
	}

	// if the recording timestamp has elapsed, stop the whole thing
	if(timestamp_elapsed(Multi_voice_test_record_stamp)){
		nprintf(("Network","Stopping voice test recording\n"));

		rtvoice_stop_recording();

		Multi_voice_test_record_stamp = -1;
		Multi_voice_test_packet_tossed = 0;
		return;
	}

	// otherwise get the compressed and uncompressed data and do something interesting with it
	rtvoice_get_data(&outbuf, &size, &gain);	

	// determine whether the packet would have been dropped
	if (size > multi_voice_max_chunk_size(MULTI_MSG_ALL)) {
		Multi_voice_test_packet_tossed = 1;
	} else {
		Multi_voice_test_packet_tossed = 0;
	}

	// send the raw output buffer to the voice options screen
	options_multi_set_voice_data(outbuf, size, gain);
}

// start recording voice locally for playback testing
void multi_voice_test_record_start()
{
	// if there is test recording going on already, don't do anything
	if(Multi_voice_test_record_stamp != -1){
		return;
	}

	// stop any playback which may be occuring
	rtvoice_stop_playback_all();

	// stop any recording which may be occuring
	rtvoice_stop_recording();

	// set the timestamp
	Multi_voice_test_record_stamp = timestamp(MV_TEST_RECORD_TIME);

	// start the recording of voice
	rtvoice_start_recording(multi_voice_test_process_next_chunk, 175);
}

// force stop any recording voice test
void multi_voice_test_record_stop()
{
	Multi_voice_test_record_stamp = -1;
	Multi_voice_test_packet_tossed = 0;
	rtvoice_stop_recording();
}

// return if the test recording is going on
int multi_voice_test_recording()
{
	return (Multi_voice_test_record_stamp == -1) ? 0 : 1;
}

// call this function if multi_voice_test_recording() is true to process various odds and ends of the test recording
void multi_voice_test_process()
{
	// if we're not recording, do nothing
	if(Multi_voice_test_record_stamp == -1){
		return;
	}

	// check to see if the timestamp has elapsed
	if(timestamp_elapsed(Multi_voice_test_record_stamp)){
		Multi_voice_test_record_stamp = -1;
		Multi_voice_test_packet_tossed = 0;
	}
}

// get a playback buffer handle (return -1 if none exist - bad)
int multi_voice_test_get_playback_buffer()
{
	// return voice stream 0
	Assert(Multi_voice_stream[0].stream_snd_handle == -1);
	Assert(Multi_voice_stream[0].stream_rtvoice_handle != -1);

	return Multi_voice_stream[0].stream_rtvoice_handle;
}

// return whether the last sampled chunk would have been too large to fit in a packet
int multi_voice_test_packet_tossed()
{
	return Multi_voice_test_packet_tossed;
}
