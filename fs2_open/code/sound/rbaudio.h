/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/RBAudio.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:35:29 $
 * $Author: Goober5000 $
 *
 * header for redbook audio playback
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
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
 * 1     4/28/97 4:45p John
 * Initial version of ripping sound & movie out of OsAPI.
 * 
 * 2     1/30/97 9:57a Allender
 * basic Redbook audio implemented.
 * 
 * 1     1/28/97 9:54a Allender
 *
 * $NoKeywords: $
*/

#ifndef _RBAUDIO_H
#define _RBAUDIO_H

#define RBA_MEDIA_CHANGED	-1

typedef struct _RBACHANNELCTL {
	unsigned int out0in, out0vol;
	unsigned int out1in, out1vol;
	unsigned int out2in, out2vol;
	unsigned int out3in, out3vol;
} RBACHANNELCTL;


// mwa ??#if defined(__NT__) 
extern void RBAInit(void);	//drive a == 0, drive b == 1
// mwa ??#else
// mwa ??   extern void RBAInit(ubyte cd_drive_num);	//drive a == 0, drive b == 1
// mwa ??#endif
extern void RBClose(void);
extern void RBARegisterCD(void);
extern long RBAGetDeviceStatus(void);
extern int RBAPlayTrack(int track);
extern int RBAPlayTracks(int first, int last);	//plays tracks first through last, inclusive
extern int RBACheckMediaChange();
extern long	RBAGetHeadLoc(int *min, int *sec, int *frame);
extern int	RBAPeekPlayStatus(void);
extern void RBAStop(void);
extern void RBASetStereoAudio(RBACHANNELCTL *channels);
extern void RBASetQuadAudio(RBACHANNELCTL *channels);
extern void RBAGetAudioInfo(RBACHANNELCTL *channels);
extern void RBASetChannelVolume(int channel, int volume);
extern void RBASetVolume(int volume);
extern int	RBAEnabled(void);
extern void RBADisable(void);
extern void RBAEnable(void);
extern int	RBAGetNumberOfTracks(void);
extern void	RBAPause();
extern int	RBAResume();

//return the track number currently playing.  Useful if RBAPlayTracks() 
//is called.  Returns 0 if no track playing, else track number
int RBAGetTrackNum();

#endif

