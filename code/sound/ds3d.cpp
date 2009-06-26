/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"

#ifndef USE_OPENAL
#include <windows.h>
#include "directx/vdsound.h"
#include "sound/channel.h"
#else
#if !(defined(__APPLE__) || defined(_WIN32))
	#include <AL/al.h>
	#include <AL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif // !__APPLE__ && !_WIN32
#endif // USE_OPENAL

#include "sound/ds3d.h"
#include "sound/ds.h"
#include "sound/sound.h"
#include "object/object.h"



#ifndef USE_OPENAL
typedef enum 
{
	DSPROPERTY_VMANAGER_MODE = 0,
	DSPROPERTY_VMANAGER_PRIORITY,
	DSPROPERTY_VMANAGER_STATE
} DSPROPERTY_VMANAGER;


typedef enum 
{
	DSPROPERTY_VMANAGER_MODE_DEFAULT = 0,
	DSPROPERTY_VMANAGER_MODE_AUTO,
	DSPROPERTY_VMANAGER_MODE_REPORT,
	DSPROPERTY_VMANAGER_MODE_USER
} VmMode;


typedef enum 
{
	DSPROPERTY_VMANAGER_STATE_PLAYING3DHW = 0,
	DSPROPERTY_VMANAGER_STATE_SILENT,
	DSPROPERTY_VMANAGER_STATE_BUMPED,
	DSPROPERTY_VMANAGER_STATE_PLAYFAILED
} VmState;


extern LPDIRECTSOUND pDirectSound;

LPDIRECTSOUND3DLISTENER	pDS3D_listener = NULL;

GUID DSPROPSETID_VoiceManager_Def = {0x62a69bae, 0xdf9d, 0x11d1, {0x99, 0xa6, 0x0, 0xc0, 0x4f, 0xc9, 0x9d, 0x46}};
#endif	// !USE_OPENAL


int DS3D_inited = FALSE;


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
	if (DS3D_inited == FALSE)
		return 0;

	if ( channel == -1 )
		return 0;

#ifdef USE_OPENAL
	// as used by DS3D
//	OpenAL_ErrorPrint( alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED) );

	// set the min distance
	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_REFERENCE_DISTANCE, min) );

	// set the max distance
//	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_MAX_DISTANCE, max) );
	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_MAX_DISTANCE, 40000.0f) );
	
	// set rolloff factor
	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_ROLLOFF_FACTOR, 1.0f) );
		
	// set doppler
	OpenAL_ErrorPrint( alDopplerVelocity(10000.0f) );
	OpenAL_ErrorPrint( alDopplerFactor(0.0f) );  // TODO: figure out why using a value of 1 sounds bad

	// set the buffer position
	if ( pos != NULL ) {
		ALfloat alpos[] = { pos->xyz.x, pos->xyz.y, pos->xyz.z };
		OpenAL_ErrorPrint( alSourcefv(Channels[channel].source_id, AL_POSITION, alpos) );
	}

	// set the buffer velocity
	if ( vel != NULL ) {
		ALfloat alvel[] = { vel->xyz.x, vel->xyz.y, vel->xyz.z };
		OpenAL_ErrorPrint( alSourcefv(Channels[channel].source_id, AL_VELOCITY, alvel) );
	} else {
		ALfloat alvel[] = { 0.0f, 0.0f, 0.0f };
		OpenAL_ErrorPrint( alSourcefv(Channels[channel].source_id, AL_VELOCITY, alvel) );
	}

#else

	HRESULT						hr;
	LPDIRECTSOUND3DBUFFER	pds3db;
	float							max_dist, min_dist;

	pds3db = Channels[channel].pds3db;
	Assert( pds3db != NULL);

	// set the buffer position
	if ( pos != NULL ) {
		hr = pds3db->SetPosition(pos->xyz.x, pos->xyz.y, pos->xyz.z, DS3D_DEFERRED);
	}

	// set the buffer veclocity
	if ( vel != NULL ) {
		hr = pds3db->SetVelocity(vel->xyz.x, vel->xyz.y, vel->xyz.z, DS3D_DEFERRED);
	}
	else {
		hr = pds3db->SetVelocity(0.0f, 0.0f, 0.0f, DS3D_DEFERRED);
	}

	// set the min distance
	hr = pds3db->GetMinDistance(&min_dist);
	hr = pds3db->SetMinDistance( min, DS3D_DEFERRED );
	// set the max distance
	hr = pds3db->GetMaxDistance(&max_dist);
//	hr = pds3db->SetMaxDistance( max, DS3D_DEFERRED );
	hr = pds3db->SetMaxDistance( 100000.0f, DS3D_DEFERRED );
#endif

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
	if (DS3D_inited == FALSE)
		return 0;

#ifdef USE_OPENAL
	// set the listener position
	if ( pos != NULL ) {
		OpenAL_ErrorPrint( alListener3f(AL_POSITION, pos->xyz.x, pos->xyz.y, pos->xyz.z) );
	}

	// set the listener velocity
	if ( vel != NULL ) {
		OpenAL_ErrorPrint( alListener3f(AL_VELOCITY, vel->xyz.x, vel->xyz.y, vel->xyz.z) );
	}

	// set the listener orientation
	if ( orient != NULL ) {
		// uvec is up/top vector, fvec is at/front vector
		ALfloat list_orien[] = { orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z,
									orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z };
		OpenAL_ErrorPrint( alListenerfv(AL_ORIENTATION, list_orien) );
	}

#else

	HRESULT			hr;

	if ( pDS3D_listener == NULL )
		return -1;
	
	// set the listener position
	if ( pos != NULL ) {
		hr = pDS3D_listener->SetPosition(pos->xyz.x, pos->xyz.y, pos->xyz.z, DS3D_DEFERRED); 
	}

	// set the listener veclocity
	if ( vel != NULL ) {
		hr = pDS3D_listener->SetVelocity(vel->xyz.x, vel->xyz.y, vel->xyz.z, DS3D_DEFERRED); 
	}

	if ( orient != NULL ) {
		hr = pDS3D_listener->SetOrientation(	orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z,
															orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z,
															DS3D_DEFERRED );
	}

	float rolloff_factor = 1.0f;
	if (ds_using_a3d() == true) {
		rolloff_factor = 3.0f;		// A3D rolloff
	} else {
		rolloff_factor = 3.0f;		// EAX rolloff
	}

	hr = pDS3D_listener->SetRolloffFactor( rolloff_factor, DS3D_DEFERRED );
	hr = pDS3D_listener->SetDopplerFactor( 1.0f, DS3D_DEFERRED );
	
	hr = pDS3D_listener->CommitDeferredSettings();
	if ( hr != DS_OK ) {
		nprintf(("SOUND","Error in pDS3D_listener->CommitDeferredSettings(): %s\n", get_DSERR_text(hr) ));
		return -1;
	}
#endif

	return 0;
}

// ---------------------------------------------------------------------------------------
// ds3d_init_listener()
//
//
//	returns:		0		=>		success
//					-1		=>		failure
//
int ds3d_init_listener()
{
#ifdef USE_OPENAL
	// this is already setup in ds_init()
#else
	HRESULT			hr;

	if ( pDS3D_listener != NULL )
		return 0;

	hr = pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void**)&pDS3D_listener);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND => Fatal error calling pPrimaryBuffer->QueryInterface(): %s\n", get_DSERR_text(hr) ));
		return -1;
	}
#endif

	return 0;		
}

// ---------------------------------------------------------------------------------------
// ds3d_close_listener()
//
//
void ds3d_close_listener()
{
#ifndef USE_OPENAL
	if ( pDS3D_listener != NULL ) {
		pDS3D_listener->Release();
		pDS3D_listener = NULL;
	}
#endif
}


// ---------------------------------------------------------------------------------------
// ds3d_init()
//
// Initialize the DirectSound3D system.  Call the initialization for the pDS3D_listener
// 
// returns:     -1	=> init failed
//              0		=> success
int ds3d_init(int voice_manager_required)
{
	if ( DS3D_inited == TRUE )
		return 0;

#ifndef USE_OPENAL
	if (voice_manager_required == 1) {
		LPKSPROPERTYSET pset;
		pset = (LPKSPROPERTYSET)ds_get_property_set_interface();

		if (pset == NULL) {
			nprintf(("Sound", "Disabling DirectSound3D since unable to get property set interface\n"));
			return -1;
		}

		HRESULT hr;
		unsigned long driver_support = 0;

		hr = pset->QuerySupport(DSPROPSETID_VoiceManager_Def, DSPROPERTY_VMANAGER_MODE, &driver_support);
		if (FAILED(hr)) {
			nprintf(("Sound", "Driver does not support Voice Manager extension, so abort DirectSound3D initialization\n"));
			return -1;
		}

		if ((driver_support & KSPROPERTY_SUPPORT_SET|KSPROPERTY_SUPPORT_GET) != (KSPROPERTY_SUPPORT_SET|KSPROPERTY_SUPPORT_GET)) {
			nprintf(("Sound", "Driver does not support Voice Manager extension, so abort DirectSound3D initialization\n"));
			return -1;
		}

		VmMode vmode = DSPROPERTY_VMANAGER_MODE_AUTO;
		hr = pset->Set(DSPROPSETID_VoiceManager_Def, DSPROPERTY_VMANAGER_MODE, NULL, 0, &vmode, sizeof(float));
		if (FAILED(hr)) {
			nprintf(("Sound", "Driver does not support Voice Manager extension, so abort DirectSound3D initialization\n"));
			return -1;
		}
	}
#endif

	if (ds3d_init_listener() != 0) {
		return -1;
	}
	
	DS3D_inited = TRUE;
	return 0;
}


// ---------------------------------------------------------------------------------------
// ds3d_close()
//
// De-initialize the DirectSound3D system
// 
void ds3d_close()
{
	if ( DS3D_inited == FALSE )
		return;

	ds3d_close_listener();
	DS3D_inited = FALSE;
}
