/*
 * $Logfile: /Freespace2/src/cutscene/oggplayer.h $
 * $Revision: 1.1.2.1 $
 * $Date: 2006-12-25 21:44:11 $
 * $Author: taylor $
 *
 * main lib for using Theora movies
 *
 * $Log: not supported by cvs2svn $
 *
 * $NoKeywords: $
 *
 */

#ifndef _OGGPLAYER_H
#define _OGGPLAYER_H

#include "globalincs/pstypes.h"

#include "theora/theora.h"
#include "vorbis/codec.h"

struct CFILE;

// structure for maintaining info on a THEORAFILE stream
typedef struct THEORAFILE
{
	CFILE	*cfp;
	ubyte	theora_p;
	ubyte	vorbis_p;

	ogg_sync_state		osyncstate;
	ogg_page			opage;
	ogg_packet			opacket;
	ogg_stream_state 	v_osstate;
	ogg_stream_state	t_osstate;

	theora_info			tinfo;
	theora_comment		tcomment;
	theora_state		tstate;

	vorbis_info			vinfo;
	vorbis_dsp_state	vstate;
	vorbis_block		vblock;
	vorbis_comment		vcomment;
} THEORAFILE;

void theora_play(THEORAFILE *movie);
THEORAFILE *theora_open(char *filename);
void theora_close(THEORAFILE *movie);

#endif
