/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/dscap.cpp $
 * $Revision: 2.6.2.1 $
 * $Date: 2007-10-29 15:32:19 $
 * $Author: taylor $
 *
 * C module for DirectSoundCapture code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2006/04/20 06:32:30  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.5  2005/04/05 11:48:23  taylor
 * remove acm-unix.cpp, replaced by acm-openal.cpp since it's properly cross-platform now
 * better error handling for OpenAL functions
 * Windows can now build properly with OpenAL
 * extra check to make sure we don't try and use too many hardware bases sources
 * fix memory error from OpenAL extension list in certain instances
 *
 * Revision 2.4  2005/01/08 09:59:10  wmcoolmon
 * Sound quality in FreeSpace 2 is now controlled by SoundSampleBits, and SoundSampleRate. Also, some sounds will use hardware rather than software buffers if available.
 *
 * Revision 2.3  2004/07/26 20:47:52  Kazan
 * remove MCD complete
 *
 * Revision 2.2  2004/07/12 16:33:06  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
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
 * 11    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 10    4/24/98 2:17a Lawrance
 * Clear out record buffer when recording begins
 * 
 * 9     3/24/98 9:37a Lawrance
 * Only read up to a safe offset in the capture buffer.
 * 
 * 8     3/22/98 7:13p Lawrance
 * Get streaming of recording voice working
 * 
 * 7     2/26/98 2:54p Lawrance
 * Don't recreate capture buffer each time recording starts... just use
 * one.
 * 
 * 6     2/15/98 11:10p Lawrance
 * more work on real-time voice system
 * 
 * 5     2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 4     2/09/98 8:07p Lawrance
 * get buffer create working
 * 
 * 3     2/04/98 6:08p Lawrance
 * Read function pointers from dsound.dll, further work on
 * DirectSoundCapture.
 * 
 * 2     2/03/98 11:53p Lawrance
 * Adding support for DirectSoundCapture
 * 
 * 1     2/03/98 4:48p Lawrance
 *
 * $NoKeywords: $
 */


#ifndef SCP_UNIX

#ifndef USE_OPENAL
#error This code has been modified to work only with OpenAL builds!
#endif

#include "globalincs/pstypes.h"
#include "sound/ds.h"
#include "sound/dscap.h"
#include "directx/vdsound.h"



int dscap_inited = 0;						// flag to indicate that DirectSoundCapture inited ok
int dscap_recording;						// flag to indicate that sound is being recorded

static LPDIRECTSOUNDCAPTURE			pDSC;		// global capture interface
static LPDIRECTSOUNDCAPTUREBUFFER	pDSCB;	// global capture buffer

static WAVEFORMATEX	Dscap_wfx;

static unsigned long Dscap_last_capture_offset;	

HINSTANCE Dscap_dll_handle = NULL;

HRESULT (__stdcall *pfn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter);

// init the DirectSoundCapture system
// exit:	0	->		success
//			!0	->		failure
int dscap_init()
{
	HRESULT	dsrval;

	if ( dscap_inited ) {
		return 0;
	}

	if (Dscap_dll_handle == NULL) {
		Dscap_dll_handle = LoadLibrary("dsound.dll");

		if ( !Dscap_dll_handle ) {
			return -1;
		}
	}

	pfn_DirectSoundCaptureCreate = (HRESULT(__stdcall *)(LPGUID lpGuid, LPDIRECTSOUNDCAPTURE *lplpDSC, IUnknown FAR *pUnkOuter))GetProcAddress(Dscap_dll_handle, "DirectSoundCaptureCreate");

	if ( !pfn_DirectSoundCaptureCreate ) {
		nprintf(( "Sound", "SOUND ==> Could not get DirectSoundCaptureCreate function pointer\n" ));
		return -1;
	}

	dsrval = pfn_DirectSoundCaptureCreate(NULL, &pDSC, NULL);

	if ( dsrval != DS_OK ) {
		nprintf(( "Sound", "SOUND ==> Error '%s' initializing DirectSoundCapture.\n", get_DSERR_text(dsrval)  ));
		return -1;
	}		

	pDSCB=NULL;

	dscap_recording=0;
	dscap_inited=1;

	return 0;
}

void dscap_release_buffer()
{
	if ( !pDSCB ) {
		return;
	}

	pDSCB->Release();
	pDSCB = NULL;
}

// create a capture buffer with the specified format
// exit:	0	->		buffer created successfully
//			!0	->		error creating the buffer
int dscap_create_buffer(int freq, int bits_per_sample, int nchannels, int nseconds)
{
	HRESULT			dsrval;
	DSCBUFFERDESC	dscbd;
	WAVEFORMATEX	wfx;

	if ( !dscap_inited ) {
		dscap_init();
	}

	if ( !dscap_inited ) {
		return -1;
	}

	// Set up recording format
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (unsigned short)nchannels;
	wfx.nSamplesPerSec = freq;
	wfx.wBitsPerSample = (unsigned short)bits_per_sample;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (unsigned short)(wfx.nChannels * (wfx.wBitsPerSample / 8));
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	Dscap_wfx = wfx;	// save the recording format

	if ( pDSCB ) {
		dscap_release_buffer();
	}

	memset(&dscbd, 0, sizeof(DSCBUFFERDESC));
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwBufferBytes = wfx.nAvgBytesPerSec * nseconds;
	dscbd.lpwfxFormat = &wfx;

	dsrval = pDSC->CreateCaptureBuffer(&dscbd, &pDSCB, NULL);
	if ( dsrval != DS_OK ) {
		nprintf(( "Sound", "SOUND ==> Error '%s' creating a DirectSoundCapture buffer.\n", get_DSERR_text(dsrval)  ));
		return -1;
	}

	Dscap_last_capture_offset=0;
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

// fill up the capture buffer with silence
int dscap_fill_buffer_with_silence()
{
	HRESULT			dsrval;
	unsigned long	buffer_len, size1, size2;
	void				*data1=NULL, *data2=NULL;

	buffer_len = dscap_max_buffersize();

	Assert(pDSCB);

	dsrval = pDSCB->Lock(0, buffer_len, &data1, &size1, &data2, &size2, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	unsigned char silence_byte;

	switch(Dscap_wfx.wBitsPerSample) {
	case 8:
		silence_byte = 0x80;
		break;
	case 16:
		silence_byte = 0x00;
		break;
	default:
		silence_byte = 0x00;
		break;
	}

	if ( (data1) && (size1 > 0) ) {
		memset(data1, silence_byte, size1);
	}

	if ( (data2) && (size2 > 0) ) {
		memset(data2, silence_byte, size2);
	}

	dsrval = pDSCB->Unlock(data1, size1, data2, size2);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
}

// start recording into the buffer
int dscap_start_record()
{
	HRESULT	dsrval;

	if ( !dscap_inited ) {
		dscap_init();
	}

	if ( !dscap_inited ) {
		return -1;
	}

	if ( dscap_recording ) {
		return -1;
	}

	Assert(pDSCB);

	dscap_fill_buffer_with_silence();

	dsrval = pDSCB->Start(DSCBSTART_LOOPING);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	dscap_recording=1;
//	nprintf(("Alan","RTVOICE => start record\n"));
	return 0;
}

// stop recording into the buffer
int dscap_stop_record()
{
	HRESULT	dsrval;

	if ( !dscap_inited ) {
		return -1;
	}

	if ( !dscap_recording ) {
		return -1;
	}

	Assert(pDSCB);
	dsrval = pDSCB->Stop();
	if ( dsrval != DS_OK ) {
		return -1;
	}

	dscap_recording=0;
//	nprintf(("Alan","RTVOICE => stop record\n"));
	return 0;
}

// close the DirectSoundCapture system
void dscap_close()
{
	dscap_stop_record();
	dscap_release_buffer();

	if ( pDSC ) {
		pDSC->Release();
		pDSC=NULL;
	}

	if (Dscap_dll_handle != NULL) {
		FreeLibrary(Dscap_dll_handle);
		Dscap_dll_handle = NULL;
	}
}

// return the max buffer size
int dscap_max_buffersize()
{
	DSCBCAPS	caps;

	if ( !dscap_inited ) {
		dscap_init();
	}

	if ( !dscap_inited ) {
		return -1;
	}

	if (!pDSCB) {
		return 0;
	}

	caps.dwSize = sizeof(DSCBCAPS);
	caps.dwFlags = 0;

	pDSCB->GetCaps(&caps);

	return caps.dwBufferBytes;
}

// retreive the recorded voice data
int dscap_get_raw_data(unsigned char *outbuf, unsigned int max_size)
{
	HRESULT			dsrval;
	unsigned long	capture_offset, read_offset, num_bytes_captured, size1, size2;
	void				*data1=NULL, *data2=NULL;

	if ( !dscap_inited ) {
		dscap_init();
	}

	if ( !dscap_inited ) {
		return -1;
	}

	if ( !pDSCB ) {
		return -1;
	}

	dsrval = pDSCB->GetCurrentPosition(&capture_offset, &read_offset);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	if ( read_offset >= Dscap_last_capture_offset ) {
		num_bytes_captured = read_offset-Dscap_last_capture_offset;
	} else {
		unsigned long max_size = dscap_max_buffersize();
		num_bytes_captured = max_size - Dscap_last_capture_offset + read_offset;
	}

	if ( num_bytes_captured <= 0 ) {
		return -1;
	}	

	dsrval = pDSCB->Lock(Dscap_last_capture_offset, num_bytes_captured, &data1, &size1, &data2, &size2, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	if ( max_size < (size1+size2) ) {
		return -1;
	}

	if ( (data1) && (size1 > 0) ) {
		memcpy(outbuf, data1, size1);
	}

	if ( (data2) && (size2 > 0) ) {
		memcpy(outbuf+size1, data2, size2);
	}

	dsrval = pDSCB->Unlock(data1, size1, data2, size2);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	Dscap_last_capture_offset = read_offset;
	return (size1+size2);
}

#endif	// !SCP_UNIX
