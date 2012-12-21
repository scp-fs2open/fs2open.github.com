/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FREESPACE_ACM_H__
#define __FREESPACE_ACM_H__

#ifdef _WIN32
#include "mm/mmreg.h"
#endif

#include "globalincs/pstypes.h"

int	ACM_convert_ADPCM_to_PCM(WAVEFORMATEX *pwfxSrc, ubyte *src, int src_len, ubyte **dest, int max_dest_bytes, int *dest_len, unsigned int *src_bytes_used, int dest_bps=16);

int ACM_stream_open(WAVEFORMATEX *pwfxSrc, WAVEFORMATEX *pwfxDest, void **stream, int dest_bps=16);
int ACM_stream_close(void *stream);
int ACM_query_source_size(void *stream, int dest_len);
int ACM_query_dest_size(void *stream, int src_len);

int ACM_convert(void *stream, ubyte *src, int src_len, ubyte *dest, int max_dest_bytes, unsigned int *dest_len, unsigned int *src_bytes_used);


#endif /* __ACM_H__ */
