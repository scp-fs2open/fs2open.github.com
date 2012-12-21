/***********************************************************
 * Portions of this file are Copyright (c) Ryan C. Gordon
 ***********************************************************/


#ifdef _WIN32
#define NONEWWAVE
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "sound/acm.h"

// we aren't including all of mmreg.h on Windows so this picks up the slack
#ifndef WAVE_FORMAT_ADPCM
#define WAVE_FORMAT_ADPCM	2
#endif


typedef struct adpcmcoef_tag {
	short iCoef1;
	short iCoef2;
} ADPCMCOEFSET;

typedef struct adpcmblockheader_tag {
	ubyte bPredictor;
	ushort iDelta;
	short iSamp1;
	short iSamp2;
} ADPCMBLOCKHEADER;

typedef struct adpcmwaveformat_tag {
	WAVEFORMATEX wav;
	WORD wSamplesPerBlock;
	WORD wNumCoef;
	ADPCMCOEFSET *aCoef;
} ADPCMWAVEFORMAT;

typedef struct ADPCM_FMT_T {
	ADPCMWAVEFORMAT adpcm;
	ADPCMBLOCKHEADER *header;

	int bytes_remaining;
	uint bytes_processed;
	uint buffer_size;
	uint sample_frame_size;
	uint samples_left_in_block;
	int nibble_state;
	ubyte nibble;

	ushort dest_bps;
} adpcm_fmt_t;

typedef struct acm_stream_t {
	adpcm_fmt_t *fmt;
	ushort dest_bps;
	ushort src_bps;
} acm_stream_t;


// similar to BIAL_IF_MACRO in SDL_sound
#define IF_ERR(a, b) if (a) { mprintf(("ACM ERROR, line %d...\n", __LINE__)); return b; }
#define IF_ERR2(a, b) if (a) { mprintf(("ACM ERROR, line %d...\n", __LINE__)); b; }


/*****************************************************************************
 * Begin ADPCM compression handler...                                       */

/*
 * ADPCM decoding routines taken with permission from SDL_sound
 * Copyright (C) 2001  Ryan C. Gordon.
 */

#define FIXED_POINT_COEF_BASE      256
#define FIXED_POINT_ADAPTION_BASE  256
#define SMALLEST_ADPCM_DELTA       16


// utility functions
static int read_ushort(HMMIO rw, ushort *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(ushort) );
	IF_ERR(rc != sizeof(ushort), 0);
	*i = INTEL_SHORT(*i); //-V570
	return 1;
}

static int read_word(HMMIO rw, WORD *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(WORD) );
	IF_ERR(rc != sizeof(WORD), 0);
	return 1;
}

// same as read_word() but swapped
static int read_word_s(HMMIO rw, WORD *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(WORD) );
	IF_ERR(rc != sizeof(WORD), 0);
	*i = INTEL_SHORT(*i); //-V570
	return 1;
}

static int read_short(HMMIO rw, short *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(short) );
	IF_ERR(rc != sizeof(short), 0);
	*i = INTEL_SHORT(*i); //-V570
	return 1;
}

static int read_dword(HMMIO rw, DWORD *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(DWORD) );
	IF_ERR(rc != sizeof(DWORD), 0);
	return 1;
}

static int read_ubyte(HMMIO rw, ubyte *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(ubyte) );
	IF_ERR(rc != sizeof(ubyte), 0);
	return 1;
}

// decoding functions
static int read_adpcm_block_headers(HMMIO rw, adpcm_fmt_t *fmt)
{
	int i;
	int max = fmt->adpcm.wav.nChannels;

	if (fmt->bytes_remaining < fmt->adpcm.wav.nBlockAlign) {
		return 0;
	}

	fmt->bytes_remaining -= fmt->adpcm.wav.nBlockAlign;
	fmt->bytes_processed += fmt->adpcm.wav.nBlockAlign;

	for (i = 0; i < max; i++) {
		IF_ERR(!read_ubyte(rw, &fmt->header[i].bPredictor), 0);
	}

	for (i = 0; i < max; i++) {
		IF_ERR(!read_ushort(rw, &fmt->header[i].iDelta), 0);
	}

	for (i = 0; i < max; i++) {
		IF_ERR(!read_short(rw, &fmt->header[i].iSamp1), 0);
	}

	for (i = 0; i < max; i++) {
		IF_ERR(!read_short(rw, &fmt->header[i].iSamp2), 0);
	}

	fmt->samples_left_in_block = fmt->adpcm.wSamplesPerBlock;
	fmt->nibble_state = 0;

	return 1;
}

static void do_adpcm_nibble(ubyte nib, ADPCMBLOCKHEADER *header, int lPredSamp)
{
	static const short max_audioval = ((1<<(16-1))-1);
	static const short min_audioval = -(1<<(16-1));
	static const ushort AdaptionTable[] = {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};

	int lNewSamp;
	ushort delta;

	if (nib & 0x08) {
		lNewSamp = lPredSamp + (header->iDelta * (nib - 0x10));
	} else {
		lNewSamp = lPredSamp + (header->iDelta * nib);
	}

	// clamp value...
	if (lNewSamp < min_audioval) {
		lNewSamp = min_audioval;
	} else if (lNewSamp > max_audioval) {
		lNewSamp = max_audioval;
	}

	delta = (header->iDelta * AdaptionTable[nib]) / FIXED_POINT_ADAPTION_BASE;

	if (delta < SMALLEST_ADPCM_DELTA) {
		delta = SMALLEST_ADPCM_DELTA;
	}

	header->iDelta = delta;
	header->iSamp2 = header->iSamp1;
	header->iSamp1 = (short)lNewSamp;
}

static int decode_adpcm_sample_frame(HMMIO rw, adpcm_fmt_t *fmt)
{
	int i;
	int max = fmt->adpcm.wav.nChannels;
	ubyte nib = fmt->nibble;
	short iCoef1, iCoef2;
	int lPredSamp;

	for (i = 0; i < max; i++) {
		iCoef1 = fmt->adpcm.aCoef[fmt->header[i].bPredictor].iCoef1;
		iCoef2 = fmt->adpcm.aCoef[fmt->header[i].bPredictor].iCoef2;
		lPredSamp = ((fmt->header[i].iSamp1 * iCoef1) + (fmt->header[i].iSamp2 * iCoef2)) / FIXED_POINT_COEF_BASE;

		if (fmt->nibble_state == 0) {
			IF_ERR(!read_ubyte(rw, &nib), 0);
			fmt->nibble_state = 1;
			do_adpcm_nibble(nib >> 4, &fmt->header[i], lPredSamp);
		} else {
			fmt->nibble_state = 0;
			do_adpcm_nibble(nib & 0x0F, &fmt->header[i], lPredSamp);
		}
	}

	fmt->nibble = nib;

	return 1;
}

static void put_adpcm_sample_frame1(ubyte *_buf, adpcm_fmt_t *fmt)
{
	int i;

	if (fmt->dest_bps == 16) {
		short *buf = (short *)_buf;

		for (i = 0; i < fmt->adpcm.wav.nChannels; i++) {
			*buf++ = fmt->header[i].iSamp1;
		}
	} else {
		Assert( fmt->dest_bps == 8 );

		for (i = 0; i < fmt->adpcm.wav.nChannels; i++) {
			int i_val = (int)(fmt->header[i].iSamp1 / 255) + 128;
			CLAMP(i_val, 0, 255);

			*_buf++ = (ubyte)i_val;
		}
	}
}

static void put_adpcm_sample_frame2(ubyte *_buf, adpcm_fmt_t *fmt)
{
	int i;

	if (fmt->dest_bps == 16) {
		short *buf = (short *)_buf;

		for (i = 0; i < fmt->adpcm.wav.nChannels; i++) {
			*buf++ = fmt->header[i].iSamp2;
		}
	} else {
		Assert( fmt->dest_bps == 8 );

		for (i = 0; i < fmt->adpcm.wav.nChannels; i++) {
			int i_val = (int)(fmt->header[i].iSamp2 / 255) + 128;
			CLAMP(i_val, 0, 255);

			*_buf++ = (ubyte)i_val;
		}
	}
}

static uint read_sample_fmt_adpcm(ubyte *data, HMMIO rw, adpcm_fmt_t *fmt)
{
	uint bw = 0;

	while (bw < fmt->buffer_size) {
		// write ongoing sample frame before reading more data...
		switch (fmt->samples_left_in_block) {
			case 0:  // need to read a new block...

				if (!read_adpcm_block_headers(rw, fmt)) {
					return(bw);		// EOF
				}

				// only write first sample frame for now.
				put_adpcm_sample_frame2(data + bw, fmt);
				fmt->samples_left_in_block--;
				bw += fmt->sample_frame_size;
			break;

			case 1:  // output last sample frame of block...
				put_adpcm_sample_frame1(data + bw, fmt);
				fmt->samples_left_in_block--;
				bw += fmt->sample_frame_size;
			break;

			default: // output latest sample frame and read a new one...
				put_adpcm_sample_frame1(data + bw, fmt);
				fmt->samples_left_in_block--;
				bw += fmt->sample_frame_size;

				if (!decode_adpcm_sample_frame(rw, fmt)) {
					return(bw);
				}
		}
	}

	return(bw);
}

/* End ADPCM Compression Handler                                              *
 *****************************************************************************/

static void adpcm_memory_free(adpcm_fmt_t *fmt)
{
	if (fmt->adpcm.aCoef != NULL) {
		vm_free(fmt->adpcm.aCoef);
		fmt->adpcm.aCoef = NULL;
	}

	if (fmt->header != NULL) {
		vm_free(fmt->header);
		fmt->header = NULL;
	}

	if (fmt != NULL) {
		vm_free(fmt);
		fmt = NULL;
	}
}

// =============================================================================
// ACM_convert_ADPCM_to_PCM()
//
// Convert an ADPCM wave file to a PCM wave file using the Audio Compression Manager
//
// parameters:	*pwfxSrc			=> address of WAVEFORMATEX structure describing the source wave
//				*src				=> pointer to raw source wave data
//				src_len				=> num bytes of source wave data
//				**dest				=> pointer to pointer to dest buffer for wave data
//										(mem is allocated in this function if *dest is NULL)
//				max_dest_bytes		=> Maximum memory allocated to dest
//				*dest_len			=> returns num bytes of wave data in converted form (OUTPUT PARAMETER)
//				*src_bytes_used		=> returns num bytes of src actually used in the conversion
//				dest_bps			=> bits per sample that data should be uncompressed to
//
// returns:		0 => success
//				1 => could not convert wav file
//
// NOTES:
// 1. Storage for the decompressed audio will be allocated in this function if *dest in NULL.
//    The caller is responsible for freeing this memory later.
//
int ACM_convert_ADPCM_to_PCM(WAVEFORMATEX *pwfxSrc, ubyte *src, int src_len, ubyte **dest, int max_dest_bytes, int *dest_len, unsigned int *src_bytes_used, int dest_bps)
{
	Assert( src != NULL );
	Assert( src_len > 0 );
	Assert( dest_len != NULL );

	if (pwfxSrc == NULL) {
		Int3();
		return -1;
	}

	Assert( pwfxSrc->wFormatTag == WAVE_FORMAT_ADPCM );

	MMIOINFO IOhdr, IOrw;

	memset( &IOhdr, 0, sizeof(MMIOINFO) );
	memset( &IOrw, 0, sizeof(MMIOINFO) );

	IOhdr.pchBuffer = (char *)pwfxSrc;
	IOhdr.fccIOProc = FOURCC_MEM;
	IOhdr.cchBuffer = (sizeof(WAVEFORMATEX) + pwfxSrc->cbSize);

	IOrw.pchBuffer = (char *)src;
	IOrw.fccIOProc = FOURCC_MEM;
	IOrw.cchBuffer = src_len;

	HMMIO hdr = mmioOpen( NULL, &IOhdr, MMIO_READ );
	HMMIO rw = mmioOpen( NULL, &IOrw, MMIO_READ );
	uint rc;
	uint new_size = 0;
	int left_over = 0;
	int i;

	adpcm_fmt_t *fmt = (adpcm_fmt_t *)vm_malloc(sizeof(adpcm_fmt_t));
	IF_ERR(fmt == NULL, -1);
	memset(fmt, '\0', sizeof(adpcm_fmt_t));

	// wav header info (WAVEFORMATEX)
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.wFormatTag), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.nChannels), goto Error);
	IF_ERR2(!read_dword(hdr, &fmt->adpcm.wav.nSamplesPerSec), goto Error);
	IF_ERR2(!read_dword(hdr, &fmt->adpcm.wav.nAvgBytesPerSec), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.nBlockAlign), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.wBitsPerSample), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.cbSize), goto Error);
	// adpcm specific header info
	IF_ERR2(!read_word_s(hdr, &fmt->adpcm.wSamplesPerBlock), goto Error);
	IF_ERR2(!read_word_s(hdr, &fmt->adpcm.wNumCoef), goto Error);

	// allocate memory for COEF struct and fill it
	fmt->adpcm.aCoef = (ADPCMCOEFSET *)vm_malloc(sizeof(ADPCMCOEFSET) * fmt->adpcm.wNumCoef);
	IF_ERR2(fmt->adpcm.aCoef == NULL, goto Error);

	for (i = 0; i < fmt->adpcm.wNumCoef; i++) {
		IF_ERR2(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef1), goto Error);
		IF_ERR2(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef2), goto Error);
	}

	// allocate memory for the ADPCM block header that's to be filled later
	fmt->header = (ADPCMBLOCKHEADER *)vm_malloc(sizeof(ADPCMBLOCKHEADER) * fmt->adpcm.wav.nChannels);
	IF_ERR2(fmt->header == NULL, goto Error);

	// estimate size of uncompressed data
	// uncompressed data has: channels=pfwxScr->nChannels, bitPerSample=destbits
	// compressed data has:   channels=pfwxScr->nChannels, bitPerSample=pwfxSrc->wBitsPerSample
	new_size = (src_len / fmt->adpcm.wav.nBlockAlign) * fmt->adpcm.wSamplesPerBlock * (dest_bps / 8);

	// DO NOT free() here, *estimated size*
	if ( *dest == NULL ) {
		*dest = (ubyte *)vm_malloc(new_size);

		if (*dest == NULL) {
			goto Error;
		}

		// silence
		if (dest_bps == 8) {
			memset(*dest, 0x80, new_size);
		} else {
			memset(*dest, 0x00, new_size);
		}
	}

	// buffer to estimated size since we have to process the whole thing at once
	fmt->buffer_size = new_size;
	fmt->bytes_remaining = src_len;
	fmt->bytes_processed = 0;

	// really fix and REMOVE
	if (fmt->adpcm.wav.wBitsPerSample != 4) {
		fmt->adpcm.wav.wBitsPerSample = INTEL_SHORT(fmt->adpcm.wav.wBitsPerSample); //-V570
	}

	// sanity check, should always be 4
	if (fmt->adpcm.wav.wBitsPerSample != 4) {
		goto Error;
	}

	fmt->sample_frame_size = ((dest_bps / 8) * pwfxSrc->nChannels);
	fmt->dest_bps = (ushort)dest_bps;

	if ( !max_dest_bytes ) {
		max_dest_bytes = new_size;
	}

	// convert to PCM
	rc = read_sample_fmt_adpcm(*dest, rw, fmt);

	left_over = (src_len - fmt->bytes_processed);

	if ( (left_over > 0) && (left_over < fmt->adpcm.wav.nBlockAlign) ) {
		// hmm, we have some left over, probably a crappy file.  just add in the
		// remainder since we don't have enough frame size left over for a decode
		// but we should have decoded most of the data already
		mprintf(("ACM ERROR: Have leftover data after decode!!\n"));
		fmt->bytes_processed += left_over;
	}

	// send back actual sizes
	if (dest_len) {
		*dest_len = rc;
	}

	if (src_bytes_used) {
		*src_bytes_used = fmt->bytes_processed;
	}

	// cleanup
	adpcm_memory_free(fmt);

	// cleanup mmio stuff
	mmioClose( rw, 0 );
	mmioClose( hdr, 0 );

	return 0;

Error:
	// cleanup
	if (fmt) {
		adpcm_memory_free(fmt);
	}

	mmioClose( rw, 0 );
	mmioClose( hdr, 0 );

	return -1;
}

int ACM_stream_open(WAVEFORMATEX *pwfxSrc, WAVEFORMATEX *pwfxDest, void **stream, int dest_bps)
{
	Assert( stream != NULL );

	if (pwfxSrc == NULL) {
		Int3();
		return -1;
	}

	Assert( pwfxSrc->wFormatTag == WAVE_FORMAT_ADPCM );

	int i;
	acm_stream_t *str = NULL;
	MMIOINFO IOhdr;

	memset( &IOhdr, 0, sizeof(MMIOINFO) );

	IOhdr.pchBuffer = (char *)pwfxSrc;
	IOhdr.fccIOProc = FOURCC_MEM;
	IOhdr.cchBuffer = (sizeof(WAVEFORMATEX) + pwfxSrc->cbSize);

	HMMIO hdr = mmioOpen( NULL, &IOhdr, MMIO_READ );

	adpcm_fmt_t *fmt = (adpcm_fmt_t *)vm_malloc(sizeof(adpcm_fmt_t));
	IF_ERR2(fmt == NULL, goto Error);
	memset(fmt, '\0', sizeof(adpcm_fmt_t));

	// wav header info (WAVEFORMATEX)
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.wFormatTag), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.nChannels), goto Error);
	IF_ERR2(!read_dword(hdr, &fmt->adpcm.wav.nSamplesPerSec), goto Error);
	IF_ERR2(!read_dword(hdr, &fmt->adpcm.wav.nAvgBytesPerSec), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.nBlockAlign), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.wBitsPerSample), goto Error);
	IF_ERR2(!read_word(hdr, &fmt->adpcm.wav.cbSize), goto Error);
	// adpcm specific header info
	IF_ERR2(!read_word_s(hdr, &fmt->adpcm.wSamplesPerBlock), goto Error);
	IF_ERR2(!read_word_s(hdr, &fmt->adpcm.wNumCoef), goto Error);

	// allocate memory for COEF struct and fill it
	fmt->adpcm.aCoef = (ADPCMCOEFSET *)vm_malloc(sizeof(ADPCMCOEFSET) * fmt->adpcm.wNumCoef);
	IF_ERR2(fmt->adpcm.aCoef == NULL, goto Error);

	for (i = 0; i < fmt->adpcm.wNumCoef; i++) {
		IF_ERR2(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef1), goto Error);
		IF_ERR2(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef2), goto Error);
	}

	// allocate memory for the ADPCM block header that's to be filled later
	fmt->header = (ADPCMBLOCKHEADER *)vm_malloc(sizeof(ADPCMBLOCKHEADER) * fmt->adpcm.wav.nChannels);
	IF_ERR2(fmt->header == NULL, goto Error);

	// sanity check, should always be 4
	if (fmt->adpcm.wav.wBitsPerSample != 4) {
		adpcm_memory_free(fmt);
		mmioClose( hdr, 0 );

		return -1;
	}

	fmt->sample_frame_size = dest_bps/8*pwfxSrc->nChannels;
	fmt->dest_bps = (ushort)dest_bps;

	str = (acm_stream_t *)vm_malloc(sizeof(acm_stream_t));
	IF_ERR2(str == NULL, goto Error);
	str->fmt = fmt;
	str->dest_bps = (ushort)dest_bps;
	str->src_bps = pwfxSrc->wBitsPerSample;

	if (stream) {
		*stream = str;
	}

	// close the io stream
	mmioClose( hdr, 0 );

	return 0;

Error:
	if (fmt) {
		adpcm_memory_free(fmt);
	}

	if (str) {
		vm_free(str);
	}

	mmioClose( hdr, 0 );

	return -1;
}

int ACM_stream_close(void *stream)
{
	if (stream == NULL) {
		Int3();
		return 0;
	}

	acm_stream_t *str = (acm_stream_t *)stream;
	adpcm_memory_free(str->fmt);
	vm_free(str);

	return 0;
}

int ACM_query_source_size(void *stream, int dest_len)
{
	if (stream == NULL) {
		Int3();
		return 0;
	}

	acm_stream_t *str = (acm_stream_t *)stream;

	// estimate size of compressed data
	// uncompressed data has: channels=pfwxScr->nChannels, bitPerSample=destbits
	// compressed data has:   channels=pfwxScr->nChannels, bitPerSample=pwfxSrc->wBitsPerSample
	return (dest_len * str->src_bps) / str->dest_bps;
}

int ACM_query_dest_size(void *stream, int src_len)
{
	if (stream == NULL) {
		Int3();
		return 0;
	}

	acm_stream_t *str = (acm_stream_t *)stream;

	// estimate size of uncompressed data
	// uncompressed data has: channels=pfwxScr->nChannels, bitPerSample=destbits
	// compressed data has:   channels=pfwxScr->nChannels, bitPerSample=pwfxSrc->wBitsPerSample
	return ( src_len * str->dest_bps ) / str->src_bps;
}

int ACM_convert(void *stream, ubyte *src, int src_len, ubyte *dest, int max_dest_bytes, unsigned int *dest_len, unsigned int *src_bytes_used)
{
	Assert( src != NULL );
	Assert( src_len > 0 );
	Assert( dest_len != NULL );

	if (stream == NULL) {
		Int3();
		return 0;
	}

	acm_stream_t *str = (acm_stream_t *)stream;
	uint rc;
	MMIOINFO IOrw;

	memset( &IOrw, 0, sizeof(MMIOINFO) );

	IOrw.pchBuffer = (char *)src;
	IOrw.fccIOProc = FOURCC_MEM;
	IOrw.cchBuffer = src_len;

	HMMIO rw = mmioOpen( NULL, &IOrw, MMIO_READ );

	// buffer to estimated size since we have to process the whole thing at once
	str->fmt->buffer_size = max_dest_bytes;
	str->fmt->bytes_remaining = src_len;
	str->fmt->bytes_processed = 0;

	// convert to PCM
	rc = read_sample_fmt_adpcm(dest, rw, str->fmt);

	// send back actual sizes
	if (dest_len) {
		*dest_len = rc;
	}

	if (src_bytes_used) {
		*src_bytes_used = str->fmt->bytes_processed;
	}

	mmioClose( rw, 0 );

	return 0;
}
