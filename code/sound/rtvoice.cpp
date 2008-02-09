/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/rtvoice.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:10 $
 * $Author: penguin $
 *
 * C module file for real-time voice
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 24    4/24/98 2:17a Lawrance
 * Clear out record buffer when recording begins
 * 
 * 23    4/21/98 4:44p Dave
 * Implement Vasudan ships in multiplayer. Added a debug function to bash
 * player rank. Fixed a few rtvoice buffer overrun problems. Fixed ui
 * problem in options screen. 
 * 
 * 22    4/21/98 10:30a Dave
 * allocate 8 second buffers for rtvoice
 * 
 * 21    4/17/98 5:27p Dave
 * More work on the multi options screen. Fixed many minor ui todo bugs.
 * 
 * 20    4/17/98 10:38a Lawrance
 * reduce num output streams to 1
 * 
 * 19    3/25/98 9:56a Dave
 * Increase buffer size to handle 8 seconds of voice data.
 * 
 * 18    3/22/98 7:13p Lawrance
 * Get streaming of recording voice working
 * 
 * 17    3/09/98 5:22p Dave
 * Fixed a rtvoice bug which caused bogus output when given size 0 input.
 * 
 * 16    2/26/98 2:54p Lawrance
 * Don't recreate capture buffer each time recording starts... just use
 * one.
 * 
 * 15    2/24/98 11:56p Lawrance
 * Change real-time voice code to provide the uncompressed size on decode.
 * 
 * 14    2/24/98 10:13p Dave
 * Put in initial support for multiplayer voice streaming.
 * 
 * 13    2/24/98 10:47a Lawrance
 * Play voice through normal channels
 * 
 * 12    2/23/98 6:54p Lawrance
 * Make interface to real-time voice more generic and useful.
 * 
 * 11    2/19/98 12:47a Lawrance
 * Use a global code_info
 * 
 * 10    2/16/98 7:31p Lawrance
 * get compression/decompression of voice working
 * 
 * 9     2/15/98 11:59p Lawrance
 * Change the order of some code when opening a stream
 * 
 * 8     2/15/98 11:10p Lawrance
 * more work on real-time voice system
 * 
 * 7     2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 6     2/09/98 8:07p Lawrance
 * get buffer create working
 * 
 * 5     2/04/98 6:08p Lawrance
 * Read function pointers from dsound.dll, further work on
 * DirectSoundCapture.
 * 
 * 4     2/03/98 11:53p Lawrance
 * Adding support for DirectSoundCapture
 * 
 * 3     2/03/98 4:07p Lawrance
 * check return codes from waveIn calls
 * 
 * 2     1/31/98 5:48p Lawrance
 * Start on real-time voice recording
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "sound/sound.h"
#include "sound/ds.h"
#include "sound/dscap.h"
#include "vcodec/codec1.h"
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

static int Rtv_do_compression=1;					// flag to indicate whether compression should be done		
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

static struct	t_CodeInfo Rtv_code_info;		// Parms will need to be transmitted with packets

// recording timer data
static int Rtv_record_timer_id;		// unique id for callback timer
static int Rtv_callback_time;			// callback time in ms

void (*Rtv_callback)();

// recording/encoding buffers
static unsigned char *Rtv_capture_raw_buffer;
static unsigned char *Rtv_capture_compressed_buffer;
static int Rtv_capture_compressed_buffer_size;
static int Rtv_capture_raw_buffer_size;

static unsigned char	*Encode_buffer1=NULL;
static unsigned char	*Encode_buffer2=NULL;

// playback/decoding buffers
static unsigned char *Rtv_playback_uncompressed_buffer;
static int Rtv_playback_uncompressed_buffer_size;

static unsigned char *Decode_buffer=NULL;
static int Decode_buffer_size;

/////////////////////////////////////////////////////////////////////////////////////////////////
// RECORD/ENCODE
/////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimeProc(unsigned int id, unsigned int msg, unsigned long userdata, unsigned long dw1, unsigned long dw2)
{
	if ( !Rtv_callback ) {
		return;
	}

	nprintf(("Alan","In callback\n"));
	Rtv_callback();
}

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
	InitEncoder(e_cCodec1, qos, Encode_buffer1, Encode_buffer2);
}

// Init the recording portion of the real-time voice system
// input:	qos	=> quality of service (1..10) 1 is highest compression, 10 is highest quality
//	exit:	0	=>	success
//			!0	=>	failure, recording not possible
int rtvoice_init_recording(int qos)
{
	if ( !Rtv_recording_inited ) {
		if ( rtvoice_pick_record_format() ) {
			return -1;
		}

		Rtv_capture_raw_buffer_size = Rtv_formats[Rtv_recording_format].frequency * (RTV_BUFFER_TIME) * fl2i(Rtv_formats[Rtv_recording_format].bits_per_sample/8.0f);

		if ( Encode_buffer1 ) {
			free(Encode_buffer1);
			Encode_buffer1=NULL;
		}

		if ( dscap_create_buffer(Rtv_formats[Rtv_recording_format].frequency, Rtv_formats[Rtv_recording_format].bits_per_sample, 1, RTV_BUFFER_TIME) ) {
			return -1;
		}

		Encode_buffer1 = (unsigned char*)malloc(Rtv_capture_raw_buffer_size);
		Assert(Encode_buffer1);

		if ( Encode_buffer2 ) {
			free(Encode_buffer2);
			Encode_buffer2=NULL;
		}
		Encode_buffer2 = (unsigned char*)malloc(Rtv_capture_raw_buffer_size);
		Assert(Encode_buffer2);

		// malloc out the voice data buffer for raw (uncompressed) recorded sound
		if ( Rtv_capture_raw_buffer ) {
			free(Rtv_capture_raw_buffer);
			Rtv_capture_raw_buffer=NULL;
		}
		Rtv_capture_raw_buffer = (unsigned char*)malloc(Rtv_capture_raw_buffer_size);

		// malloc out voice data buffer for compressed recorded sound
		if ( Rtv_capture_compressed_buffer ) {
			free(Rtv_capture_compressed_buffer);
			Rtv_capture_compressed_buffer=NULL;
		}
		Rtv_capture_compressed_buffer_size=Rtv_capture_raw_buffer_size;	// be safe and allocate same as uncompressed
		Rtv_capture_compressed_buffer = (unsigned char*)malloc(Rtv_capture_compressed_buffer_size);

		InitEncoder(e_cCodec1, qos, Encode_buffer1, Encode_buffer2);

		Rtv_recording_inited=1;
	}
	return 0;
}

// Stop a stream from recording
void rtvoice_stop_recording()
{
	if ( !Rtv_recording ) {
		return;
	}

	dscap_stop_record();

	if ( Rtv_record_timer_id ) {
		timeKillEvent(Rtv_record_timer_id);
		Rtv_record_timer_id = 0;
	}

	Rtv_recording=0;
}

// Close down the real-time voice recording system
void rtvoice_close_recording()
{
	if ( Rtv_recording ) {
		rtvoice_stop_recording();
	}

	if ( Encode_buffer1 ) {
		free(Encode_buffer1);
		Encode_buffer1=NULL;
	}

	if ( Encode_buffer2 ) {
		free(Encode_buffer2);
		Encode_buffer2=NULL;
	}

	if ( Rtv_capture_raw_buffer ) {
		free(Rtv_capture_raw_buffer);
		Rtv_capture_raw_buffer=NULL;
	}

	if ( Rtv_capture_compressed_buffer ) {
		free(Rtv_capture_compressed_buffer);
		Rtv_capture_compressed_buffer=NULL;
	}

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
		Rtv_record_timer_id = timeSetEvent(callback_time, callback_time, TimeProc, 0, TIME_PERIODIC);
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

// compress voice data using specialized codec
int rtvoice_compress(unsigned char *data_in, int size_in, unsigned char *data_out, int size_out)
{
	int		compressed_size;
	
	Rtv_code_info.Code = e_cCodec1;
	Rtv_code_info.Gain = 0;

	compressed_size = 0;
	if(size_in <= 0){
		nprintf(("Network","RTVOICE => 0 bytes size in !\n"));		
	} else {
		compressed_size = Encode(data_in, data_out, size_in, size_out, &Rtv_code_info);

		nprintf(("SOUND","RTVOICE => Sound compressed to %d bytes (%0.2f percent)\n", compressed_size, (compressed_size*100.0f)/size_in));
	}

	return compressed_size;
}

// For 8-bit formats (unsigned, 0 to 255)
// For 16-bit formats (signed, -32768 to 32767)
int rtvoice_16to8(unsigned char *data, int size)
{
	int i;
	unsigned short	sample16;
	unsigned char	sample8, *dest, *src;

	Assert(size%2 == 0);

	dest = data;
	src = data;

	for (i=0; i<size; i+=2) {
		sample16  = src[0];
		sample16 |= src[1] << 8;

		sample16 += 32768;
		sample8 = (unsigned char)(sample16>>8);

		*dest++ = sample8;
		src += 2;
	}

	return (size>>1);
}

// Convert voice sample from 22KHz to 11KHz
int rtvoice_22to11(unsigned char *data, int size)
{
	int i, new_size=0;
	unsigned char *dest, *src;

	dest=data;
	src=data;

	for (i=0; i<size; i+=2) {
		*(dest+new_size) = *(src+i);
		new_size++;
	}

	return new_size; 
}

// Convert voice data to 8bit, 11KHz if necessary
int rtvoice_maybe_convert_data(unsigned char *data, int size)
{
	int new_size=size;
	switch ( Rtv_recording_format ) {
	case 0:
		// do nothing
		break;
	case 1:
		// convert samples to 8 bit from 16 bit
		new_size = rtvoice_16to8(data,new_size);
		break;
	case 2:
		// convert to 11KHz
		new_size = rtvoice_22to11(data,new_size);
		break;
	case 3:
		// convert to 11Khz, 8 bit
		new_size = rtvoice_16to8(data,new_size);
		new_size = rtvoice_22to11(data,new_size);
		break;
	default:
		Int3();	// should never happen
		break;
	}

	return new_size;
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
void rtvoice_get_data(unsigned char **outbuf, int *compressed_size, int *uncompressed_size, double *gain, unsigned char **outbuf_raw, int *outbuf_size_raw)
{
	int max_size, raw_size, csize;
	max_size = dscap_max_buffersize();

	*compressed_size=0;
	*uncompressed_size=0;
	*outbuf=NULL;

	if ( max_size < 0 ) {
		return;
	}	
	
	raw_size = dscap_get_raw_data(Rtv_capture_raw_buffer, max_size);

	// convert data to 8bit, 11KHz if necessary
	raw_size = rtvoice_maybe_convert_data(Rtv_capture_raw_buffer, raw_size);
	*uncompressed_size=raw_size;

	// compress voice data
	if ( Rtv_do_compression ) {
		csize = rtvoice_compress(Rtv_capture_raw_buffer, raw_size, Rtv_capture_compressed_buffer, Rtv_capture_compressed_buffer_size);
		*gain = Rtv_code_info.Gain;
		*compressed_size = csize;
		*outbuf = Rtv_capture_compressed_buffer;		
	} else {
		*gain = Rtv_code_info.Gain;
		*compressed_size = raw_size;
		*outbuf = Rtv_capture_raw_buffer;
	}

	// NOTE : if we are not doing compression, then the raw buffer and size are going to be the same as the compressed
	//        buffer and size

	// assign the raw buffer and size if necessary
	if(outbuf_raw != NULL){
		*outbuf_raw = Rtv_capture_raw_buffer;
	}
	if(outbuf_size_raw != NULL){
		*outbuf_size_raw = raw_size;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DECODE/PLAYBACK
/////////////////////////////////////////////////////////////////////////////////////////////////

// return the size that the decode buffer should be
int rtvoice_get_decode_buffer_size()
{
	return Decode_buffer_size;
}

// uncompress the data into PCM format
void rtvoice_uncompress(unsigned char *data_in, int size_in, double gain, unsigned char *data_out, int size_out)
{
	Rtv_code_info.Gain = gain;
	Decode(&Rtv_code_info, data_in, data_out, size_in, size_out);
}

// Close down the real-time voice playback system
void rtvoice_close_playback()
{
	if ( Decode_buffer ) {
		free(Decode_buffer);
		Decode_buffer=NULL;
	}

	if ( Rtv_playback_uncompressed_buffer ) {
		free(Rtv_playback_uncompressed_buffer);
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
		Decode_buffer_size = rtvf->frequency * (RTV_BUFFER_TIME) * fl2i(rtvf->bits_per_sample/8.0f);

		if ( Decode_buffer ) {
			free(Decode_buffer);
			Decode_buffer=NULL;
		}

		Decode_buffer = (unsigned char*)malloc(Decode_buffer_size);
		Assert(Decode_buffer);

		if ( Rtv_playback_uncompressed_buffer ) {
			free(Rtv_playback_uncompressed_buffer);
			Rtv_playback_uncompressed_buffer=NULL;
		}

		Rtv_playback_uncompressed_buffer_size=Decode_buffer_size;
		Rtv_playback_uncompressed_buffer = (unsigned char*)malloc(Rtv_playback_uncompressed_buffer_size);
		Assert(Rtv_playback_uncompressed_buffer);

		InitDecoder(1, Decode_buffer); 

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

	return 0;
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
			ds_unload_buffer(Rtv_output_buffers[index].ds_handle, -1);
		}
		Rtv_output_buffers[index].ds_handle=-1;
	}
}

// Play compressed sound data
// exit:	>=0	=>	handle to playing sound
//			-1		=>	error, voice not played
int rtvoice_play_compressed(int index, unsigned char *data, int size, int uncompressed_size, double gain)
{
	int ds_handle, rval;

	ds_handle = Rtv_output_buffers[index].ds_handle;

	// Stop any currently playing voice output
	ds_stop_easy(ds_handle);

	Assert(uncompressed_size <= Rtv_playback_uncompressed_buffer_size);

	// uncompress the data into PCM format
	if ( Rtv_do_compression ) {
		rtvoice_uncompress(data, size, gain, Rtv_playback_uncompressed_buffer, uncompressed_size);
	}

	// lock the data in
	if ( ds_lock_data(ds_handle, Rtv_playback_uncompressed_buffer, uncompressed_size) ) {
		return -1;
	}

	// play the voice
	rval = ds_play(ds_handle, -1, -100, DS_MUST_PLAY, ds_convert_volume(Master_voice_volume), 0, 0);
	return rval;
}

// Play uncompressed (raw) sound data
// exit:	>=0	=>	handle to playing sound
//			-1		=>	error, voice not played
int rtvoice_play_uncompressed(int index, unsigned char *data, int size)
{
	int ds_handle, rval;

	ds_handle = Rtv_output_buffers[index].ds_handle;

	// Stop any currently playing voice output
	ds_stop_easy(ds_handle);

	// lock the data in
	if ( ds_lock_data(ds_handle, data, size) ) {
		return -1;
	}

	// play the voice
	rval = ds_play(ds_handle, -1, -100, DS_MUST_PLAY, ds_convert_volume(Master_voice_volume), 0, 0);
	return rval;
}
