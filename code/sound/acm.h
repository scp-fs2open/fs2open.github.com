/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/acm.h $
 * $Revision: 2.4 $
 * $Date: 2005-01-31 10:34:39 $
 * $Author: taylor $
 *
 * Header file for interface to Audio Compression Manager functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2003/03/02 06:37:24  penguin
 * Use multimedia headers in local dir, not system's (headers are not present in MinGW distribution)
 *  - penguin
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
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 5     2/18/98 5:49p Lawrance
 * Even if the ADPCM codec is unavailable, allow game to continue.
 * 
 * 4     11/28/97 2:09p Lawrance
 * Overhaul how ADPCM conversion works... use much less memory... safer
 * too.
 * 
 * 3     11/22/97 11:32p Lawrance
 * decompress ADPCM data into 8 bit (not 16bit) for regular sounds (ie not
 * music)
 * 
 * 2     5/29/97 12:03p Lawrance
 * creation of file to hold AudioCompressionManager specific code
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __FREESPACE_ACM_H__
#define __FREESPACE_ACM_H__

#ifdef _WIN32
#include "mm/mmreg.h"
#include "mm/msacm.h"
#endif
#include "globalincs/pstypes.h"

int	ACM_convert_ADPCM_to_PCM(WAVEFORMATEX *pwfxSrc, ubyte *src, int src_len, ubyte **dest, int max_dest_bytes, int *dest_len, unsigned int *src_bytes_used, unsigned short dest_bps=16);
int	ACM_init();
void	ACM_close();
int	ACM_is_inited();


int ACM_stream_open(WAVEFORMATEX *pwfxSrc, WAVEFORMATEX *pwfxDest, void **stream, int dest_bps=16);
int ACM_stream_close(void *stream);
int ACM_query_source_size(void *stream, int dest_len);
int ACM_query_dest_size(void *stream, int src_len);

int ACM_convert(void *stream, ubyte *src, int src_len, ubyte *dest, int max_dest_bytes, unsigned int *dest_len, unsigned int *src_bytes_used);


#endif /* __ACM_H__ */
