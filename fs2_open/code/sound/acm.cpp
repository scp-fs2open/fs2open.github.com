/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/acm.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:29 $
 * $Author: penguin $
 *
 * C file for interface to Audio Compression Manager functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 8     6/13/98 1:45p Sandeep
 * 
 * 7     2/18/98 5:49p Lawrance
 * Even if the ADPCM codec is unavailable, allow game to continue.
 * 
 * 6     11/28/97 2:09p Lawrance
 * Overhaul how ADPCM conversion works... use much less memory... safer
 * too.
 * 
 * 5     11/22/97 11:32p Lawrance
 * decompress ADPCM data into 8 bit (not 16bit) for regular sounds (ie not
 * music)
 * 
 * 4     9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 3     8/05/97 1:39p Lawrance
 * support compressed stereo playback
 * 
 * 2     5/29/97 12:03p Lawrance
 * creation of file to hold AudioCompressionManager specific code
 *
 * $NoKeywords: $
 *
 */

#include "pstypes.h"
#include <windows.h>
#include <mmreg.h>
#include "acm.h"


// variables global to file for Audio Compression Manager (ACM) conversion
static HACMDRIVERID	ghadid = NULL;
static HINSTANCE		ghinstAcm;
static HACMDRIVER		ghacmdriver;
static int				ACM_inited = 0;

//--------------------------------------------------------------------------;
//  
//  int ACM_enum_callback()
//  
//  This function is called by acmDriverEnum() to go through the installed
//  audio codecs.  This function will locate the Microsoft ADPCM codec and
//  set the global value ghadid (type HACMDRIVERID), which we need when 
//  converting audio from ADPCM to PCM format.
//
//  Arguments:
//      HACMDRIVERID hadid:
//      DWORD dwInstance:
//      DWORD fdwSupport:
//  
//--------------------------------------------------------------------------;

int CALLBACK ACM_enum_callback(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
    static TCHAR    szBogus[]       = TEXT("????");

    MMRESULT            mmr;
    HWND                hlb;
    ACMDRIVERDETAILS    add;
    BOOL                fDisabled;
    DWORD               dwPriority;

    hlb = (HWND)(UINT)dwInstance;

    add.cbStruct = sizeof(add);
    mmr = acmDriverDetails(hadid, &add, 0L);
    if (MMSYSERR_NOERROR != mmr)
    {
        lstrcpy(add.szShortName, szBogus);
        lstrcpy(add.szLongName,  szBogus);
    }

    dwPriority = (DWORD)-1L;
    acmMetrics((HACMOBJ)hadid, ACM_METRIC_DRIVER_PRIORITY, &dwPriority);

    fDisabled = (0 != (ACMDRIVERDETAILS_SUPPORTF_DISABLED & fdwSupport));

	
	 if ( stricmp(NOX("MS-ADPCM"),add.szShortName) == 0 ) {
		if ( fDisabled != 0 ) {
			nprintf(("Sound", "SOUND => The Microsoft ADPCM driver is disabled, unable to convert ADPCM to PCM\n"));
			ghadid = NULL;
		}
		else {
			ghadid = hadid;
		}
		return (FALSE);	// stop enumerating devices, we've found what we need
	 }
    //
    //  return TRUE to continue with enumeration (FALSE will stop the
    //  enumerator)
    //
    return (TRUE);
} 

int ACM_stream_open(WAVEFORMATEX *pwfxSrc, WAVEFORMATEX *pwfxDest, void **stream, int dest_bps) 
{
	Assert( pwfxSrc != NULL );
	Assert( pwfxDest != NULL );
	Assert( stream != NULL);

	int rc;

	if ( ACM_inited == 0 ) {
		rc = ACM_init();
		if ( rc != 0 )
			return -1;
	}
		
	pwfxDest->wFormatTag			= WAVE_FORMAT_PCM;
	pwfxDest->nChannels			= pwfxSrc->nChannels;
	pwfxDest->nSamplesPerSec	= pwfxSrc->nSamplesPerSec;
	pwfxDest->wBitsPerSample	= (unsigned short)dest_bps;
	pwfxDest->cbSize				= 0;
	pwfxDest->nBlockAlign		= (unsigned short)(( pwfxDest->nChannels * pwfxDest->wBitsPerSample ) / 8);
	pwfxDest->nAvgBytesPerSec	= pwfxDest->nBlockAlign * pwfxDest->nSamplesPerSec;

	rc = acmStreamOpen((HACMSTREAM*)stream, ghacmdriver, pwfxSrc, pwfxDest, NULL, 0L, 0L, ACM_STREAMOPENF_NONREALTIME);
	if ( rc != 0 ) return -1;

	return 0;
}

int ACM_stream_close(void *stream)
{
	int rc;

	rc = acmStreamClose((HACMSTREAM)stream, 0);
	if ( rc != 0 )
		return -1;

	return 0;
}


int ACM_convert(void *stream, ubyte *src, int src_len, ubyte *dest, int max_dest_bytes, unsigned int *dest_len, unsigned int *src_bytes_used)
{
	int rc;
	ACMSTREAMHEADER hCvtHdr;

	if ( ACM_inited == 0 ) {
		rc = ACM_init();
		if ( rc != 0 )
			return -1;
	}

	memset(&hCvtHdr, 0, sizeof(hCvtHdr));

	hCvtHdr.cbStruct		= sizeof(hCvtHdr);
	hCvtHdr.pbSrc			= (unsigned char *)src;
	hCvtHdr.cbSrcLength	= src_len;
	hCvtHdr.pbDst			= (unsigned char*)dest;
	hCvtHdr.cbDstLength	= max_dest_bytes;
	
	rc = acmStreamPrepareHeader((HACMSTREAM)stream, &hCvtHdr, 0);
	if ( rc != 0 ) return -1;

	rc = acmStreamConvert((HACMSTREAM)stream, &hCvtHdr, 0);
	if ( rc != 0 ) return -1;

	// Important step, since we need the exact length of the converted data.
	*dest_len = hCvtHdr.cbDstLengthUsed;
	*src_bytes_used = hCvtHdr.cbSrcLengthUsed;

	rc = acmStreamUnprepareHeader((HACMSTREAM)stream, &hCvtHdr, 0);
	if ( rc != 0 )
		return -1;

	return rc;
}

int ACM_query_source_size(void *stream, int dest_len)
{
	int	rc;
	unsigned long	src_size;

	rc = acmStreamSize((HACMSTREAM)stream, dest_len, &src_size, ACM_STREAMSIZEF_DESTINATION);
	return (int)src_size;
}

int ACM_query_dest_size(void *stream, int src_len)
{
	int	rc;
	unsigned long	dest_size;

	rc = acmStreamSize((HACMSTREAM)stream, src_len, &dest_size, ACM_STREAMSIZEF_SOURCE);
	return (int)dest_size;
}

// =============================================================================
// ACM_convert_ADPCM_to_PCM()
//
// Convert an ADPCM wave file to a PCM wave file using the Audio Compression Manager
//
//	parameters:    *pwfxSrc   => address of WAVEFORMATEX structure describing the source wave
//                *src       => pointer to raw source wave data
//                src_len    => num bytes of source wave data
//                **dest     => pointer to pointer to dest buffer for wave data
//                              (mem is allocated in this function if *dest is NULL)
//						max_dest_bytes		=> Maximum memory allocated to dest
//                *dest_len			=> returns num bytes of wave data in converted form (OUTPUT PARAMETER)
//						*src_bytes_used	=>	returns num bytes of src actually used in the conversion
//						dest_bps				=> bits per sample that data should be uncompressed to
//
// returns:       0 => success
//               -1 => could not convert wav file
//
//
// NOTES:
// 1. Storage for the decompressed audio will be allocated in this function if *dest in NULL.
//    The caller is responsible for freeing this memory later.
//
int ACM_convert_ADPCM_to_PCM(WAVEFORMATEX *pwfxSrc, ubyte *src, int src_len, ubyte **dest, int max_dest_bytes, int *dest_len, unsigned int *src_bytes_used, unsigned short dest_bps)
{
	Assert( pwfxSrc != NULL );
	Assert( pwfxSrc->wFormatTag == WAVE_FORMAT_ADPCM );
	Assert( src != NULL );
	Assert( src_len > 0 );
	Assert( dest_len != NULL );

	WAVEFORMATEX wfxDest;
	HACMSTREAM hStream;
	ACMSTREAMHEADER hCvtHdr;
	int rc;

	if ( ACM_inited == 0 ) {
		rc = ACM_init();
		if ( rc != 0 )
			return -1;
	}
		
	wfxDest.wFormatTag = WAVE_FORMAT_PCM;
	wfxDest.nChannels = pwfxSrc->nChannels;
	wfxDest.nSamplesPerSec = pwfxSrc->nSamplesPerSec;
	wfxDest.wBitsPerSample = dest_bps;
	wfxDest.cbSize = 0;
	wfxDest.nBlockAlign = (unsigned short)(( wfxDest.nChannels * wfxDest.wBitsPerSample ) / 8);
	wfxDest.nAvgBytesPerSec = wfxDest.nBlockAlign * wfxDest.nSamplesPerSec;

	rc = acmStreamOpen(&hStream, ghacmdriver, pwfxSrc, &wfxDest, NULL, 0L, 0L, ACM_STREAMOPENF_NONREALTIME);
	if ( rc != 0 ) return -1;

	rc = acmStreamSize(hStream, src_len, (unsigned long *)dest_len, ACM_STREAMSIZEF_SOURCE);
	if ( rc != 0 ) return -1;

	if ( *dest == NULL ) {
		*dest = (ubyte*)malloc(*dest_len);
		Assert( *dest != NULL );
	}

	if ( !max_dest_bytes ) {
		max_dest_bytes = *dest_len;
	}

	memset(&hCvtHdr, 0, sizeof(hCvtHdr));

	hCvtHdr.cbStruct		= sizeof(hCvtHdr);
	hCvtHdr.pbSrc			= (unsigned char *)src;
	hCvtHdr.cbSrcLength	= src_len;
	hCvtHdr.pbDst			= (unsigned char *)*dest;
	hCvtHdr.cbDstLength	= max_dest_bytes;
	
	rc = acmStreamPrepareHeader(hStream, &hCvtHdr, 0);
	if ( rc != 0 ) return -1;

	rc = acmStreamConvert(hStream, &hCvtHdr, 0);
	if ( rc != 0 ) return -1;

	// Important step, since we need the exact length of the converted data.
	*dest_len = hCvtHdr.cbDstLengthUsed;
	*src_bytes_used = hCvtHdr.cbSrcLengthUsed;

	rc = acmStreamUnprepareHeader(hStream, &hCvtHdr, 0);
	if ( rc != 0 ) return -1;

	rc = acmStreamClose(hStream, 0);
	if ( rc != 0 ) return -1;

	return 0;
}

// =============================================================================
// ACM_init()
//
// Initializes the Audio Compression Manager components used to convert ADPCM to PCM
//
// returns:       0 => success
//               -1 => ACM could not be initialized
//
int ACM_init()
{
	int rc;

	if ( ACM_inited == 1 )
		return 0;

	ghinstAcm = LoadLibrary(NOX("msacm32.dll"));
	if (ghinstAcm == NULL) {
		return -1;
	}
	FreeLibrary(ghinstAcm);

	long dwVersion;
	dwVersion = acmGetVersion();
	nprintf(("Sound", "ACM Version number: %u.%.02u\n", HIWORD(dwVersion) >> 8, HIWORD(dwVersion) & 0x00FF)); 
	// ACM must be version 3.5 or higher
	if ( dwVersion < 0x03320000 )  {
		return -1;
	}

	rc = acmDriverEnum(ACM_enum_callback,	0L, ACM_DRIVERENUMF_DISABLED);
	if ( rc != MMSYSERR_NOERROR ) return -1;

	if ( ghadid == NULL ) {
		nprintf(("Sound", "SOUND => Unable to locate the Microsoft ADPCM driver\n"));
		return -1;
	}

	rc = acmDriverOpen(&ghacmdriver, ghadid, 0L);
	if ( rc != MMSYSERR_NOERROR ) return -1;

	rc = acmDriverPriority(ghadid,0,0);
	if ( rc != MMSYSERR_NOERROR ) return -1;

	ACM_inited = 1;
	return 0;
}

// Closes down the Audio Compression Manager components
void ACM_close()
{
	if ( ACM_inited == 0 )
		return;

	acmDriverClose( ghacmdriver, 0L);
	ACM_inited = 0;
}

// Query if the ACM system is initialized
int ACM_is_inited()
{
	return ACM_inited;
}
