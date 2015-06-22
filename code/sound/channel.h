/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "sound/openal.h"

typedef struct channel
{
	int			sig;			// uniquely identifies the sound playing on the channel
	int			snd_id;		// identifies which kind of sound is playing
	ALuint		source_id;	// OpenAL source id
	int			sid;		// currently bound sound buffer index (-1 if none)
	int			looping;		// flag to indicate that the sound is looping
	float		vol;			// in linear scale
	int			priority;	// implementation dependant priority
	unsigned int		last_position;
	bool		is_voice_msg;
	bool		is_ambient;

	channel() :
		sig(-1), snd_id(-1), source_id(0), sid(-1), looping(0), vol(1.0f),
		priority(0), last_position(0), is_voice_msg(false), is_ambient(false)
	{
	}
} channel;


// #define	MAX_CHANNELS  16
extern	channel* Channels;    //[MAX_CHANNELS];

#endif /* __CHANNEL_H__ */
