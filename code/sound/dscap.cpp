/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "sound/ds.h"
#include "sound/dscap.h"

#include <string>


int dscap_inited = 0;						// flag to indicate that DirectSoundCapture inited ok
int dscap_recording = 0;						// flag to indicate that sound is being recorded

typedef struct ALcapture_t {
	uint samples_per_second;
	uint bits_per_sample;
	uint n_channels;
	uint block_align;

	ALenum format;
	ALsizei buffer_size;
} ALcapture_t;

static ALcapture_t ALCaptureInfo;

static ALCdevice *ds_capture_device = NULL;
static SCP_string capture_dev_name;


// init the capture system
// exit:	0	->		success
//			!0	->		failure
int dscap_init()
{
	if (dscap_inited) {
		return 0;
	}

	bool rval = openal_init_device(NULL, &capture_dev_name);

	if ( !rval || capture_dev_name.empty() ) {
		dscap_inited = 0;

		return -1;
	}

	dscap_inited = 1;

	return 0;
}

void dscap_release_buffer()
{
	if (ds_capture_device != NULL) {
		OpenAL_C_ErrorPrint( alcCaptureCloseDevice(ds_capture_device) );
		ds_capture_device = NULL;
	}
}

// create a capture buffer with the specified format
// exit:	0	->		buffer created successfully
//			!0	->		error creating the buffer
int dscap_create_buffer(int freq, int bits_per_sample, int nchannels, int nseconds)
{
	ALenum al_format = AL_FORMAT_MONO8;
	ALsizei buf_size = freq * nseconds;

	if ( !dscap_inited ) {
		dscap_init();
	}

	//Just in case we couldn't init for whatever reason
	if ( !dscap_inited ) { //-V581
		return -1;
	}

	Assert( (nchannels == 1) || (nchannels == 2) );
	Assert( (bits_per_sample == 8) || (bits_per_sample == 16) );

	if (nchannels == 1) {
		if (bits_per_sample == 8)  {
			al_format = AL_FORMAT_MONO8;
		} else if (bits_per_sample == 16) {
			al_format = AL_FORMAT_MONO16;
		}
	} else if (nchannels == 2) {
		if (bits_per_sample == 8) {
			al_format = AL_FORMAT_STEREO8;
		} else if (bits_per_sample == 16) {
			al_format = AL_FORMAT_STEREO16;
		}
	}

	const ALCchar *dev_name = (const ALCchar*) capture_dev_name.c_str();
	ds_capture_device = alcCaptureOpenDevice(dev_name, freq, al_format, buf_size);

	if (ds_capture_device == NULL) {
		return -1;
	}

	if ( alcGetError(ds_capture_device) != ALC_NO_ERROR ) {
		return -1;
	}

	ALCaptureInfo.format = al_format;
	ALCaptureInfo.bits_per_sample = bits_per_sample;
	ALCaptureInfo.n_channels = nchannels;
	ALCaptureInfo.samples_per_second = freq;
	ALCaptureInfo.block_align = (nchannels * bits_per_sample) / 8;

	return 0;
}

// check if DirectSoundCapture is supported
int dscap_supported()
{
	if ( !dscap_inited ) {
		dscap_init();
	}

	return dscap_inited;
}

// start recording into the buffer
int dscap_start_record()
{
	if ( !dscap_inited ) {
		return -1;
	}

	if (dscap_recording) {
		return -1;
	}

	OpenAL_C_ErrorCheck( alcCaptureStart(ds_capture_device), return -1 );

	dscap_recording = 1;

//	nprintf(("Alan","RTVOICE => start record\n"));

	return 0;
}

// stop recording into the buffer
int dscap_stop_record()
{
	if ( !dscap_inited ) {
		return -1;
	}

	if ( !dscap_recording ) {
		return -1;
	}

	OpenAL_C_ErrorCheck( alcCaptureStop(ds_capture_device), return -1 );

	dscap_recording = 0;

//	nprintf(("Alan","RTVOICE => stop record\n"));

	return 0;
}

// close the DirectSoundCapture system
void dscap_close()
{
	dscap_stop_record();

	if (ds_capture_device != NULL) {
		OpenAL_C_ErrorPrint( alcCaptureCloseDevice(ds_capture_device) );
		ds_capture_device = NULL;
	}
}

// return the max buffer size
int dscap_max_buffersize()
{
	if ( !dscap_inited ) {
		dscap_init();
	}

	//Just in case we're still not initialized
	if ( !dscap_inited ) { //-V581
		return -1;
	}

	ALCsizei num_samples = 0;

	OpenAL_C_ErrorCheck( alcGetIntegerv(ds_capture_device, ALC_CAPTURE_SAMPLES, sizeof(ALCsizei), &num_samples), return -1 );

	return (num_samples * ALCaptureInfo.block_align);
}

// retreive the recorded voice data
int dscap_get_raw_data(unsigned char *outbuf, unsigned int max_size)
{
	if ( !dscap_inited ) {
		dscap_init();
	}

	//Just in case we're still not initialized
	if ( !dscap_inited ) { //-V581
		return 0;
	}

	if (outbuf == NULL) {
		return 0;
	}

	ALCsizei num_samples = 0;

	OpenAL_C_ErrorPrint( alcGetIntegerv(ds_capture_device, ALC_CAPTURE_SAMPLES, sizeof(ALCsizei), &num_samples) );

	if (num_samples <= 0) {
		return 0;
	}

	ALCsizei max_buf_size = MIN(num_samples, ALsizei(max_size / ALCaptureInfo.block_align));

	OpenAL_C_ErrorCheck( alcCaptureSamples(ds_capture_device, outbuf, max_buf_size), return 0 );

	return (int)max_buf_size * ALCaptureInfo.block_align;
}

