/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/rtvoice.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:34 $
 * $Author: Kazan $
 *
 * Header file for real-time voice code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/22 01:37:24  penguin
 * Stub defines for NO_SOUND
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
 * 12    4/21/98 4:44p Dave
 * Implement Vasudan ships in multiplayer. Added a debug function to bash
 * player rank. Fixed a few rtvoice buffer overrun problems. Fixed ui
 * problem in options screen. 
 * 
 * 11    4/17/98 5:27p Dave
 * More work on the multi options screen. Fixed many minor ui todo bugs.
 * 
 * 10    3/25/98 9:56a Dave
 * Increase buffer size to handle 8 seconds of voice data.
 * 
 * 9     3/22/98 7:13p Lawrance
 * Get streaming of recording voice working
 * 
 * 8     2/24/98 11:56p Lawrance
 * Change real-time voice code to provide the uncompressed size on decode.
 * 
 * 7     2/24/98 10:13p Dave
 * Put in initial support for multiplayer voice streaming.
 * 
 * 6     2/23/98 6:54p Lawrance
 * Make interface to real-time voice more generic and useful.
 * 
 * 5     2/16/98 7:31p Lawrance
 * get compression/decompression of voice working
 * 
 * 4     2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 3     2/03/98 11:53p Lawrance
 * Adding support for DirectSoundCapture
 * 
 * 2     1/31/98 5:48p Lawrance
 * Start on real-time voice recording
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __RTVOICE_H__
#define __RTVOICE_H__

#ifndef NO_SOUND
// general
void  rtvoice_set_qos(int qos);

// recording
int	rtvoice_init_recording(int qos);
void	rtvoice_close_recording();
int	rtvoice_start_recording( void (*user_callback)() = NULL, int callback_time = 175 );
void	rtvoice_stop_recording();
void	rtvoice_get_data(unsigned char **outbuf, int *compressed_size, int *uncompressed_size, double *gain, unsigned char **outbuf_raw = NULL, int *outbuf_size_raw = NULL);

// playback
int	rtvoice_init_playback();
void	rtvoice_close_playback();
int	rtvoice_get_decode_buffer_size();

int	rtvoice_create_playback_buffer();
void	rtvoice_free_playback_buffer(int index);

void	rtvoice_uncompress(unsigned char *data_in, int size_in, double gain, unsigned char *data_out, int size_out);

// return a sound handle, _NOT_ a buffer handle
int	rtvoice_play_compressed(int handle, unsigned char *data, int compressed_size, int uncompressed_size, double gain);
int	rtvoice_play_uncompressed(int handle, unsigned char *data, int size);

// pass in buffer handle returned from rtvoice_create_playback_buffer(), kills the _sound_ only
void	rtvoice_stop_playback(int handle);
void	rtvoice_stop_playback_all();

#else  // NO_SOUND

#define  rtvoice_set_qos(qos)								((void)(qos))
#define	rtvoice_init_recording(qos)					((qos), -1)
#define	rtvoice_close_recording()
#define	rtvoice_start_recording(user_callback, callback_time)	((user_callback), (callback_time), -1)
#define	rtvoice_stop_recording()
#define	rtvoice_get_data(outbuf, compressed_size, uncompressed_size, gain, outbuf_raw, outbuf_size_raw) \
	do {	*(outbuf)=NULL; *(compressed_size)=0; *(uncompressed_size)=0; \
			(void)(gain); (void)(outbuf_raw); (void)(outbuf_size_raw); \
	} while (0)
#define	rtvoice_init_playback()							(-1)
#define	rtvoice_close_playback()
#define	rtvoice_get_decode_buffer_size()				(0)
#define	rtvoice_create_playback_buffer()				(-1)
#define	rtvoice_free_playback_buffer(index)			((void)(index))
#define	rtvoice_uncompress(data_in, size_in, gain, data_out, size_out) \
	((void)((data_in), (size_in), (gain), (data_out), (size_out)))
#define	rtvoice_play_compressed(handle, data, compressed_size, uncompressed_size, gain) \
	((handle), (data), (compressed_size), (uncompressed_size), (gain), -1)
#define	rtvoice_play_uncompressed(handle, data, size) \
	((handle), (data), (size), -1)
#define	rtvoice_stop_playback(handle)					((void)(handle))
#define	rtvoice_stop_playback_all()

#endif // #ifndef NO_SOUND

#endif
