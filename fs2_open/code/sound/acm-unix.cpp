/***********************************************************
 * Portions of this file are Copyright (c) Ryan C. Gordon
 ***********************************************************/

#ifndef WIN32	// Goober5000

#include "globalincs/pstypes.h"
#include "sound/acm.h"


typedef struct adpcmcoef_tag{
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
} adpcm_fmt_t;


// similar to BIAL_IF_MACRO in SDL_sound
#define IF_ERR(a, b) if (a) { printf("ACM ERROR, function: %s, line %d...\n", __FUNCTION__, __LINE__); return b; }

static int ACM_inited = 0;


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
static int read_ushort(SDL_RWops *rw, ushort *i)
{
	int rc = SDL_RWread(rw, i, sizeof(ushort), 1);
	IF_ERR(rc != 1, 0);
	*i = INTEL_SHORT(*i);
	return 1;
}

static int read_word(SDL_RWops *rw, WORD *i)
{
	int rc = SDL_RWread(rw, i, sizeof(WORD), 1);
	IF_ERR(rc != 1, 0);
	return 1;
}

// same as read_word() but swapped
static int read_word_s(SDL_RWops *rw, WORD *i)
{
	int rc = SDL_RWread(rw, i, sizeof(WORD), 1);
	IF_ERR(rc != 1, 0);
	*i = INTEL_SHORT(*i);
	return 1;
}

static int read_short(SDL_RWops *rw, short *i)
{
	int rc = SDL_RWread(rw, i, sizeof(short), 1);
	IF_ERR(rc != 1, 0);
	*i = INTEL_SHORT(*i);
	return 1;
}

static int read_dword(SDL_RWops *rw, DWORD *i)
{
	int rc = SDL_RWread(rw, i, sizeof(DWORD), 1);
	IF_ERR(rc != 1, 0);
	return 1;
}

static int read_ubyte(SDL_RWops *rw, ubyte *i)
{
	int rc = SDL_RWread(rw, i, sizeof(ubyte), 1);
	IF_ERR(rc != 1, 0);
	return 1;
}

// decoding functions
static int read_adpcm_block_headers(SDL_RWops *rw, adpcm_fmt_t *fmt)
{
	int i;
	int max = fmt->adpcm.wav.nChannels;

	if (fmt->bytes_remaining < fmt->adpcm.wav.nBlockAlign) {
		return 0;
	}

	fmt->bytes_remaining -= fmt->adpcm.wav.nBlockAlign;
	fmt->bytes_processed += fmt->adpcm.wav.nBlockAlign;

	for (i = 0; i < max; i++)
		IF_ERR(!read_ubyte(rw, &fmt->header[i].bPredictor), 0);

	for (i = 0; i < max; i++)
		IF_ERR(!read_ushort(rw, &fmt->header[i].iDelta), 0);

	for (i = 0; i < max; i++)
		IF_ERR(!read_short(rw, &fmt->header[i].iSamp1), 0);

	for (i = 0; i < max; i++)
		IF_ERR(!read_short(rw, &fmt->header[i].iSamp2), 0);
	
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

	if (delta < SMALLEST_ADPCM_DELTA)
		delta = SMALLEST_ADPCM_DELTA;

	header->iDelta = delta;
	header->iSamp2 = header->iSamp1;
	header->iSamp1 = lNewSamp;
}

static int decode_adpcm_sample_frame(SDL_RWops *rw, adpcm_fmt_t *fmt)
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
	short *buf = (short *)_buf;
	int i;
	
	for (i = 0; i < fmt->adpcm.wav.nChannels; i++)
		*buf++ = fmt->header[i].iSamp1;
}

static void put_adpcm_sample_frame2(ubyte *_buf, adpcm_fmt_t *fmt)
{
	short *buf = (short *)_buf;
	int i;

	for (i = 0; i < fmt->adpcm.wav.nChannels; i++)
		*buf++ = fmt->header[i].iSamp2;
}

static uint read_sample_fmt_adpcm(ubyte *data, SDL_RWops *rw, adpcm_fmt_t *fmt)
{
	uint bw = 0;

	while (bw < fmt->buffer_size) {
		// write ongoing sample frame before reading more data...
		switch (fmt->samples_left_in_block) {
			case 0:  // need to read a new block...
				if (!read_adpcm_block_headers(rw, fmt))
					return(bw);		// EOF

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

				if (!decode_adpcm_sample_frame(rw, fmt))
					return(bw);
		}
	}

	return(bw);
}

/* End ADPCM Compression Handler                                              *
 *****************************************************************************/

static void adpcm_memory_free(adpcm_fmt_t *fmt)
{
	if (fmt->adpcm.aCoef != NULL) {
		free(fmt->adpcm.aCoef);
		fmt->adpcm.aCoef = NULL;
	}
	
	if (fmt->header != NULL) {
		free(fmt->header);
		fmt->header = NULL;
	}
	
	if (fmt != NULL) {
		free(fmt);
		fmt = NULL;
	}
}

// =============================================================================
// ACM_convert_ADPCM_to_PCM()
//
// Convert an ADPCM wave file to a PCM wave file using the Audio Compression Manager
//
// parameters:	*pwfxSrc   => address of WAVEFORMATEX structure describing the source wave
//				*src	   => pointer to raw source wave data
//				src_len    => num bytes of source wave data
//				**dest     => pointer to pointer to dest buffer for wave data
//							  (mem is allocated in this function if *dest is NULL)
//				max_dest_bytes   => Maximum memory allocated to dest
//				*dest_len        => returns num bytes of wave data in converted form (OUTPUT PARAMETER)
//				*src_bytes_used  =>	returns num bytes of src actually used in the conversion
//				dest_bps         => bits per sample that data should be uncompressed to
//
// returns:	   0 => success
//			   -1 => could not convert wav file
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

	SDL_RWops *hdr = SDL_RWFromMem(pwfxSrc, sizeof(WAVEFORMATEX) + pwfxSrc->cbSize);
	SDL_RWops *rw = SDL_RWFromMem(src, src_len);
	uint rc;
	uint new_size = 0;

	if ( ACM_inited == 0 ) {
		rc = ACM_init();
		if ( rc != 0 )
			return -1;
	}

	// estimate size of uncompressed data
	new_size = src_len * ( (dest_bps * pwfxSrc->nChannels * pwfxSrc->wBitsPerSample) / 8 );

	// DO NOT free() here, *estimated size*
	if ( *dest == NULL ) {
		*dest = (ubyte *)malloc(new_size);
		
		IF_ERR(*dest == NULL, -1);
		
		memset(*dest, 0x80, new_size);	// silence (for 8 bits/sec)
	}

	adpcm_fmt_t *fmt = (adpcm_fmt_t *)malloc(sizeof(adpcm_fmt_t));
	IF_ERR(fmt == NULL, -1);
	memset(fmt, '\0', sizeof(adpcm_fmt_t));

	// wav header info (WAVEFORMATEX)
	IF_ERR(!read_word(hdr, &fmt->adpcm.wav.wFormatTag), -1);
	IF_ERR(!read_word(hdr, &fmt->adpcm.wav.nChannels), -1);
	IF_ERR(!read_dword(hdr, &fmt->adpcm.wav.nSamplesPerSec), -1);
	IF_ERR(!read_dword(hdr, &fmt->adpcm.wav.nAvgBytesPerSec), -1);
	IF_ERR(!read_word(hdr, &fmt->adpcm.wav.nBlockAlign), -1);
	IF_ERR(!read_word(hdr, &fmt->adpcm.wav.wBitsPerSample), -1);
	IF_ERR(!read_word(hdr, &fmt->adpcm.wav.cbSize), -1);
	// adpcm specific header info
	IF_ERR(!read_word_s(hdr, &fmt->adpcm.wSamplesPerBlock), -1);
	IF_ERR(!read_word_s(hdr, &fmt->adpcm.wNumCoef), -1);

	// allocate memory for COEF struct and fill it
	fmt->adpcm.aCoef = (ADPCMCOEFSET *)malloc(sizeof(ADPCMCOEFSET) * fmt->adpcm.wNumCoef);
	IF_ERR(fmt->adpcm.aCoef == NULL, -1);

	for (int i=0; i<fmt->adpcm.wNumCoef; i++) {
		IF_ERR(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef1), -1);
		IF_ERR(!read_short(hdr, &fmt->adpcm.aCoef[i].iCoef2), -1);
	}

	// allocate memory for the ADPCM block header that's to be filled later
	fmt->header = (ADPCMBLOCKHEADER *)malloc(sizeof(ADPCMBLOCKHEADER) * fmt->adpcm.wav.nChannels);
	IF_ERR(fmt->header == NULL, -1);

	// buffer to estimated size since we have to process the whole thing at once
	fmt->buffer_size = new_size;
	fmt->bytes_remaining = src_len;
	fmt->bytes_processed = 0;

	// really fix and REMOVE
	if (fmt->adpcm.wav.wBitsPerSample != 4)
		fmt->adpcm.wav.wBitsPerSample = INTEL_SHORT(fmt->adpcm.wav.wBitsPerSample);

	// sanity check, should always be 4
	if (fmt->adpcm.wav.wBitsPerSample != 4) {
		adpcm_memory_free(fmt);
		return -1;
	}

	fmt->sample_frame_size = 2;

	if ( !max_dest_bytes ) {
		max_dest_bytes = new_size;
	}

	// convert to PCM
	rc = read_sample_fmt_adpcm(*dest, rw, fmt);

	// send back actual sizes
	*dest_len = rc;
	*src_bytes_used = fmt->bytes_processed;

	// cleanup
	adpcm_memory_free(fmt);

	// cleanup SDL_RWops stuff
	SDL_FreeRW(rw);
	SDL_FreeRW(hdr);

	return 0;
}

int ACM_stream_open(WAVEFORMATEX *pwfxSrc, WAVEFORMATEX *pwfxDest, void **stream, int dest_bps)
{
	return -1;
}

int ACM_stream_close(void *stream)
{
	return -1;
}

int ACM_query_source_size(void *stream, int dest_len)
{
	return -1;
}

int ACM_query_dest_size(void *stream, int src_len)
{
	int new_size = 0;

	// estimate size of uncompressed data
//	new_size = src_len * ( (16 * (WAVEFORMATEX)stream->nChannels * (WAVEFORMATEX)stream->wBitsPerSample) / 8 );
	
	return new_size;
}

int ACM_convert(void *stream, ubyte *src, int src_len, ubyte *dest, int max_dest_bytes, unsigned int *dest_len, unsigned int *src_bytes_used)
{
	return -1;
}

// ACM_init() - decoding should always work
int ACM_init()
{
	if ( ACM_inited == 1 )
		return 0;

	ACM_inited = 1;

	return 0;
}

// close out
void ACM_close()
{
	if ( ACM_inited == 0 )
		return;

	ACM_inited = 0;
}

// Query if the ACM system is initialized
int ACM_is_inited()
{
	return ACM_inited;
}

#endif		// Goober5000 - #ifndef WIN32