/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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

