/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "sound/ds3d.h"
#include "sound/ds.h"
#include "sound/channel.h"
#include "sound/sound.h"
#include "object/object.h"
#include "cmdline/cmdline.h"


// ---------------------------------------------------------------------------------------
// ds3d_update_buffer()
//
//	parameters:		channel	=> identifies the 3D sound to update
//						min		=>	the distance at which sound doesn't get any louder
//						max		=>	the distance at which sound doesn't attenuate any further
//						pos		=> world position of sound
//						vel		=> velocity of the objects producing the sound
//
//	returns:		0		=>		success
//					-1		=>		failure
//
//
int ds3d_update_buffer(int channel, float min, float max, vec3d *pos, vec3d *vel)
{
	if (Cmdline_no_3d_sound) {
		nprintf(("Sound", "Aborting ds3d_update_buffer due to Cmdline_no_3d_sound..."));
		return -1;
	}

	if (channel < 0) {
		return 0;
	}

	ALuint source_id = Channels[channel].source_id;
	ALfloat rolloff = 1.0f;

	if (pos) {
		OpenAL_ErrorPrint( alSource3f(source_id, AL_POSITION, pos->xyz.x, pos->xyz.y, -pos->xyz.z) );
	}

	if (vel) {
		OpenAL_ErrorPrint( alSource3f(source_id, AL_VELOCITY, vel->xyz.x, vel->xyz.y, vel->xyz.z) );
	//	OpenAL_ErrorPrint( alSourcef(source_id, AL_DOPPLER_FACTOR, 1.0f) );
	} else {
		OpenAL_ErrorPrint( alSource3f(source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f) );
		OpenAL_ErrorPrint( alSourcef(source_id, AL_DOPPLER_FACTOR, 0.0f) );
	}

	if (max <= min) {
		rolloff = 0.0f;
	} else {
		#define MIN_GAIN	0.05f

		rolloff = (min / (min + (max - min))) / MIN_GAIN;

		if (rolloff < 0.0f) {
			rolloff = 0.0f;
		}
	}

	OpenAL_ErrorPrint( alSourcef(source_id, AL_ROLLOFF_FACTOR, rolloff) );

	OpenAL_ErrorPrint( alSourcef(source_id, AL_REFERENCE_DISTANCE, min) );
	OpenAL_ErrorPrint( alSourcef(source_id, AL_MAX_DISTANCE, max) );

	return 0;
}


// ---------------------------------------------------------------------------------------
// ds3d_update_listener()
//
//	returns:		0		=>		success
//					-1		=>		failure
//
int ds3d_update_listener(vec3d *pos, vec3d *vel, matrix *orient)
{
	if (Cmdline_no_3d_sound) {
		nprintf(("Sound", "Aborting ds3d_update_buffer due to Cmdline_no_3d_sound..."));
		return -1;
	}

	if (pos) {
		OpenAL_ErrorPrint( alListener3f(AL_POSITION, pos->xyz.x, pos->xyz.y, -pos->xyz.z) );
	}

	if (vel) {
		OpenAL_ErrorPrint( alListener3f(AL_VELOCITY, vel->xyz.x, vel->xyz.y, vel->xyz.z) );
	}

	if (orient) {
		ALfloat alOrient[6];

		alOrient[0] =  orient->vec.fvec.xyz.x;
		alOrient[1] =  orient->vec.fvec.xyz.y;
		alOrient[2] = -orient->vec.fvec.xyz.z;

		alOrient[3] =  orient->vec.uvec.xyz.x;
		alOrient[4] =  orient->vec.uvec.xyz.y;
		alOrient[5] = -orient->vec.uvec.xyz.z;

		OpenAL_ErrorPrint( alListenerfv(AL_ORIENTATION, alOrient) );
	}

	return 0;
}

