/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "sound/sound.h"
#include "sound/ds.h"
#include "sound/dscap.h"
#include "sound/rtvoice.h"


typedef struct rtv_format
{
	int nchannels;
	int bits_per_sample;
	int frequency;
} rtv_format;


#define MAX_RTV_FORMATS	5
static rtv_format Rtv_formats[MAX_RTV_FORMATS] = 
{
	{1,	8,		11025},
	{1,	16,	11025},
	{1,	8,		22050},
	{1,	16,	22050},
	{1,	8,		44100},
};

static int Rtv_recording_format;					// recording format, index into Rtv_formats[]
static int Rtv_playback_format;					// playback format, index into Rtv_formats[]

#define RTV_BUFFER_TIME		8						// length of buffer in seconds	

static int Rtv_recording_inited=0;				// The input stream has been inited
static int Rtv_playback_inited=0;				// The output stream has been inited

static int Rtv_recording=0;						// Voice is currently being recorded

#define MAX_RTV_OUT_BUFFERS	1
#define RTV_OUT_FLAG_USED		(1<<0)
typedef struct rtv_out_buffer
{
	int ds_handle;		// handle to directsound buffer
	int flags;			// see RTV_OUT_FLAG_ #defines above
} rtv_out_buffer;
static rtv_out_buffer Rtv_output_buffers[MAX_RTV_OUT_BUFFERS];		// data for output buffers

//static struct	t_CodeInfo Rtv_code_info;		// Parms will need to be transmitted with packets

// recording timer data
#ifdef _WIN32
static int Rtv_record_timer_id;		// unique id for callback timer
#else
static SDL_TimerID Rtv_record_timer_id;
#endif
static int Rtv_callback_time;			// callback time in ms

void (*Rtv_callback)();

// recording/encoding buffers
static unsigned char *Rtv_capture_raw_buffer;
static unsigned char *Rtv_capture_compressed_buffer;
//static int Rtv_capture_compressed_buffer_size;
static int Rtv_capture_raw_buffer_size;

// playback/decoding buffers
static unsigned char *Rtv_playback_uncompressed_buffer;
static int Rtv_playback_uncompressed_buffer_size;

/////////////////////////////////////////////////////////////////////////////////////////////////
// RECORD/ENCODE
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void CALLBACK TimeProc(unsigned int id, unsigned int msg, unsigned long userdata, unsigned long dw1, unsigned long dw2) 
{
	if ( !Rtv_callback ) {
		return;
	}

	nprintf(("Alan","In callback\n"));
	Rtv_callback();
}
#else
Uint32 CALLBACK TimeProc(Uint32 interval, void *param)
{
	if ( !Rtv_callback ) {
		SDL_RemoveTimer(Rtv_record_timer_id);
		Rtv_record_timer_id = NULL;

		return 0;
	}

	mprintf(("In callback\n"));
	Rtv_callback();

	if (Rtv_callback_time) {
		return interval;
	} else {
		SDL_RemoveTimer(Rtv_record_timer_id);
		Rtv_record_timer_id = NULL;

		return 0;
	}
}
#endif

// Try to pick the most appropriate recording format
//
// exit:	0	=>		success
//			!0	=>		failure
int rtvoice_pick_record_format()
{
	int i;

	for (i=0; i<MAX_RTV_FORMATS; i++) {
		if ( dscap_create_buffer(Rtv_formats[i].frequency, Rtv_formats[i].bits_per_sample, 1, RTV_BUFFER_TIME) == 0 ) {
			dscap_release_buffer();
			Rtv_recording_format=i;
			break;
		}
	}

	if ( i == MAX_RTV_FORMATS ) {
		return -1;
	}

	return 0;
}

// input:	qos => new quality of service (1..10)
void rtvoice_set_qos(int qos)
{
	// TODO:  Speex stuff
}

// Init the recording portion of the real-time voice system
// input:	qos	=> quality of service (1..10) 1 is highest compression, 10 is highest quality
//	exit:	0	=>	success
//			!0	=>	failure, recording not possible
int rtvoice_init_recording(int qos)
{
#if 0
	if ( !Rtv_recording_inited ) {
		if ( rtvoice_pick_record_format() ) {
			return -1;
		}

		Rtv_capture_raw_buffer_size = Rtv_formats[Rtv_recording_format].frequency * (RTV_BUFFER_TIME) * fl2i(Rtv_formats[Rtv_recording_format].bits_per_sample/8.0f);

		if ( dscap_create_buffer(Rtv_formats[Rtv_recording_format].frequency, Rtv_formats[Rtv_recording_format].bits_per_sample, 1, RTV_BUFFER_TIME) ) {
			return -1;
		}

		// malloc out the voice data buffer for raw (uncompressed) recorded sound
		if ( Rtv_capture_raw_buffer ) {
			vm_free(Rtv_capture_raw_buffer);
			Rtv_capture_raw_buffer=NULL;
		}
		Rtv_capture_raw_buffer = (unsigned char*)vm_malloc(Rtv_capture_raw_buffer_size);

		// malloc out voice data buffer for compressed recorded sound
		if ( Rtv_capture_compressed_buffer ) {
			vm_free(Rtv_capture_compressed_buffer);
			Rtv_capture_compressed_buffer=NULL;
		}
		Rtv_capture_compressed_buffer_size=Rtv_capture_raw_buffer_size;	// be safe and allocate same as uncompressed
		Rtv_capture_compressed_buffer = (unsigned char*)vm_malloc(Rtv_capture_compressed_buffer_size);

		Rtv_recording_inited=1;
	}
	return 0;
#else
	return -1;
#endif
}

// Stop a stream from recording
void rtvoice_stop_recording()
{
	if ( !Rtv_recording ) {
		return;
	}

	dscap_stop_record();

	if ( Rtv_record_timer_id ) {
#ifndef _WIN32
		SDL_RemoveTimer(Rtv_record_timer_id);
		Rtv_record_timer_id = NULL;
#else
		timeKillEvent(Rtv_record_timer_id);
		Rtv_record_timer_id = 0;
#endif
	}

	Rtv_recording=0;
}

// Close down the real-time voice recording system
void rtvoice_close_recording()
{
	if ( Rtv_recording ) {
		rtvoice_stop_recording();
	}

	if ( Rtv_capture_raw_buffer ) {
		vm_free(Rtv_capture_raw_buffer);
		Rtv_capture_raw_buffer=NULL;
	}

	if ( Rtv_capture_compressed_buffer ) {
		vm_free(Rtv_capture_compressed_buffer);
		Rtv_capture_compressed_buffer=NULL;
	}

	dscap_release_buffer();

	Rtv_recording_inited=0;
}

// Open a stream for recording (recording begins immediately)
// exit:	0	=>	success
//			!0	=>	failure
int rtvoice_start_recording( void (*user_callback)(), int callback_time ) 
{
	if ( !dscap_supported() ) {
		return -1;
	}

	Assert(Rtv_recording_inited);

	if ( Rtv_recording ) {
		return -1;
	}

	if ( dscap_start_record() ) {
		return -1;
	}

	if ( user_callback ) {
#ifndef _WIN32
		Rtv_record_timer_id = SDL_AddTimer(callback_time, TimeProc, NULL);
#else
		Rtv_record_timer_id = timeSetEvent(callback_time, callback_time, TimeProc, 0, TIME_PERIODIC);
#endif
		if ( !Rtv_record_timer_id ) {
			dscap_stop_record();
			return -1;
		}
		Rtv_callback = user_callback;
		Rtv_callback_time = callback_time;
	} else {
		Rtv_callback = NULL;
		Rtv_record_timer_id = 0;
	}

	Rtv_recording=1;
	return 0;
}

// Retrieve the recorded voice data
// input:	outbuf					=>		output parameter, recorded voice stored here
//				compressed_size		=>		output parameter, size in bytes of recorded voice after compression
//				uncompressed_size		=>		output parameter, size in bytes of recorded voice before compression
//				gain						=>		output parameter, gain value which must be passed to decoder
//				outbuf_raw				=>		output optional parameter, pointer to the raw sound data making up the compressed chunk
//				outbuf_size_raw		=>		output optional parameter, size of the outbuf_raw buffer
//
// NOTE: function converts voice data into compressed format
void rtvoice_get_data(unsigned char **outbuf, int *size, double *gain)
{
	int raw_size;

	*outbuf=NULL;

	raw_size = dscap_get_raw_data(Rtv_capture_raw_buffer, Rtv_capture_raw_buffer_size);

	// TODO: compress voice data

//	*gain = Rtv_code_info.Gain;
	*size = raw_size;
	*outbuf = Rtv_capture_raw_buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DECODE/PLAYBACK
/////////////////////////////////////////////////////////////////////////////////////////////////

// uncompress the data into PCM format
void rtvoice_uncompress(unsigned char *data_in, int size_in, double gain, unsigned char *data_out, int size_out)
{
	if ( (data_in == NULL) || (size_in <= 0) ) {
		return;
	}
}

// Close down the real-time voice playback system
void rtvoice_close_playback()
{
	if ( Rtv_playback_uncompressed_buffer ) {
		vm_free(Rtv_playback_uncompressed_buffer);
		Rtv_playback_uncompressed_buffer=NULL;
	}

	Rtv_playback_inited=0;
}

// Clear out the Rtv_output_buffers[] array
void rtvoice_reset_out_buffers()
{
	int i;
	
	for ( i=0; i<MAX_RTV_OUT_BUFFERS; i++ ) {
		Rtv_output_buffers[i].flags=0;
		Rtv_output_buffers[i].ds_handle=-1;
	}
}

// Init the playback portion of the real-time voice system
//	exit:	0	=>	success
//			!0	=>	failure, playback not possible
int rtvoice_init_playback()
{
	rtv_format	*rtvf=NULL;

	if ( !Rtv_playback_inited ) {

		rtvoice_reset_out_buffers();

		Rtv_playback_format=0;
		rtvf = &Rtv_formats[Rtv_playback_format];

		if ( Rtv_playback_uncompressed_buffer ) {
			vm_free(Rtv_playback_uncompressed_buffer);
			Rtv_playback_uncompressed_buffer=NULL;
		}

		Rtv_playback_uncompressed_buffer_size = rtvf->frequency * (RTV_BUFFER_TIME) * fl2i(rtvf->bits_per_sample/8.0f);
		Rtv_playback_uncompressed_buffer = (unsigned char*)vm_malloc(Rtv_playback_uncompressed_buffer_size);
		Assert(Rtv_playback_uncompressed_buffer);

		Rtv_playback_inited=1;
	}

	return 0;
}

int rtvoice_find_free_output_buffer()
{
	int i;

	for ( i=0; i<MAX_RTV_OUT_BUFFERS; i++ ) {
		if ( !(Rtv_output_buffers[i].flags & RTV_OUT_FLAG_USED) ) {
			break;
		}
	}

	if ( i == MAX_RTV_OUT_BUFFERS ) {
		return -1;
	}

	Rtv_output_buffers[i].flags |= RTV_OUT_FLAG_USED;
	return i;
	 
}

// Open a stream for real-time voice output
int rtvoice_create_playback_buffer()
{
	int			index;
	rtv_format	*rtvf=NULL;

	rtvf = &Rtv_formats[Rtv_playback_format];
	index = rtvoice_find_free_output_buffer();

	if ( index == -1 ) {
		Int3();
		return -1;
	}
	
	Rtv_output_buffers[index].ds_handle = ds_create_buffer(rtvf->frequency, rtvf->bits_per_sample, 1, RTV_BUFFER_TIME);
	if ( Rtv_output_buffers[index].ds_handle == -1 ) {
		return -1;
	}

	return index;
}

void rtvoice_stop_playback(int index)
{
	Assert(index >=0 && index < MAX_RTV_OUT_BUFFERS);

	if ( Rtv_output_buffers[index].flags & RTV_OUT_FLAG_USED ) {
		if ( Rtv_output_buffers[index].ds_handle != -1 ) {
			ds_stop_easy(Rtv_output_buffers[index].ds_handle);
		}
	}
}

void rtvoice_stop_playback_all()
{
	int i;

	for ( i = 0; i < MAX_RTV_OUT_BUFFERS; i++ ) {
		rtvoice_stop_playback(i);
	}
}

// Close a stream that was opened for real-time voice output
void rtvoice_free_playback_buffer(int index)
{
	Assert(index >=0 && index < MAX_RTV_OUT_BUFFERS);

	if ( Rtv_output_buffers[index].flags & RTV_OUT_FLAG_USED ) {
		Rtv_output_buffers[index].flags=0;
		if ( Rtv_output_buffers[index].ds_handle != -1 ) {
			ds_stop_easy(Rtv_output_buffers[index].ds_handle);
			ds_unload_buffer(Rtv_output_buffers[index].ds_handle);
		}
		Rtv_output_buffers[index].ds_handle=-1;
	}
}

// Play sound data
// exit:	>=0	=>	handle to playing sound
//			-1		=>	error, voice not played
int rtvoice_play(int index, unsigned char *data, int size)
{
	int ds_handle, rval;

	ds_handle = Rtv_output_buffers[index].ds_handle;

	// Stop any currently playing voice output
	ds_stop_easy(ds_handle);

	// TODO: uncompress the data into PCM format

	// lock the data in
	if ( ds_lock_data(ds_handle, data, size) ) {
		return -1;
	}

	// play the voice
	rval = ds_play(ds_handle, -100, DS_MUST_PLAY, Master_voice_volume, 0, 0);
	return rval;
}

