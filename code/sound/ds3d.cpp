/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/ds3d.cpp $
 * $Revision: 2.3 $
 * $Date: 2003-03-02 06:37:24 $
 * $Author: penguin $
 *
 * C file for interface to DirectSound3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:56:00  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    8/05/99 4:45p Alanl
 * the FINAL tweak to rolloffs!
 * 
 * 11    8/05/99 4:34p Alanl
 * change rolloff factors again
 * 
 * 10    8/05/99 4:27p Danw
 * would you believe we're still tweaking the EAX?? :)
 * 
 * 9     8/05/99 4:04p Danw
 * tweak rolloffs for EAX
 * 8     8/05/99 2:54p Danw
 * tweak rolloffs for A3D and EAX
 * 7     8/05/99 10:54a Alanl
 * change EAX rolloff to 3.0
 * 
 * 6     8/04/99 11:51a Danw
 * tweak rolloffs for A3D and EAX
 * 5     8/04/99 11:42a Danw
 * tweak rolloffs for A3D and EAX
 * 
 * 4     8/01/99 2:06p Alanl
 * increase the rolloff for A3D
 * 
 * 3     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 15    5/06/98 2:16p Dan
 * 
 * 14    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 13    4/19/98 9:30p Lawrance
 * Use Aureal_enabled flag
 * 
 * 12    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 11    8/01/97 10:40a Lawrance
 * decrease rolloff for DirectSound3D sounds
 * 
 * 10    7/29/97 2:54p Lawrance
 * 
 * 9     7/28/97 11:39a Lawrance
 * allow individual volume scaling on 3D buffers
 * 
 * 8     7/17/97 9:32a John
 * made all directX header files name start with a v
 * 
 * 7     6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 6     6/09/97 8:53a Lawrance
 * remove warning
 * 
 * 5     6/08/97 5:59p Lawrance
 * integrate DirectSound3D into sound system
 * 
 * 4     6/02/97 1:45p Lawrance
 * implementing hardware mixing
 * 
 * 3     5/29/97 4:02p Lawrance
 * listener interface in place
 * 
 * 2     5/29/97 12:03p Lawrance
 * creation of file to hold DirectSound3D specific code
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include <windows.h>
#include "directx/vdsound.h"

#include "sound/ds3d.h"
#include "sound/ds.h"
#include "sound/channel.h"
#include "sound/sound.h"
#include "object/object.h"

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

int DS3D_inited = FALSE;

LPDIRECTSOUND3DLISTENER	pDS3D_listener = NULL;

GUID DSPROPSETID_VoiceManager_Def = {0x62a69bae, 0xdf9d, 0x11d1, {0x99, 0xa6, 0x0, 0xc0, 0x4f, 0xc9, 0x9d, 0x46}};

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
int ds3d_update_buffer(int channel, float min, float max, vector *pos, vector *vel)
{
	HRESULT						hr;
	LPDIRECTSOUND3DBUFFER	pds3db;
	float							max_dist, min_dist;

	if (DS3D_inited == FALSE)
		return 0;

	if ( channel == -1 )
		return 0;

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

	return 0;
}


// ---------------------------------------------------------------------------------------
// ds3d_update_listener()
//
//	returns:		0		=>		success
//					-1		=>		failure
//
int ds3d_update_listener(vector *pos, vector *vel, matrix *orient)
{
	HRESULT			hr;

	if (DS3D_inited == FALSE)
		return 0;

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
	HRESULT			hr;

	if ( pDS3D_listener != NULL )
		return 0;

	hr = pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void**)&pDS3D_listener);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND => Fatal error calling pPrimaryBuffer->QueryInterface(): %s\n", get_DSERR_text(hr) ));
		return -1;
	}

	return 0;		
}

// ---------------------------------------------------------------------------------------
// ds3d_close_listener()
//
//
void ds3d_close_listener()
{
	if ( pDS3D_listener != NULL ) {
		pDS3D_listener->Release();
		pDS3D_listener = NULL;
	}
}


// ---------------------------------------------------------------------------------------
// ds3d_init()
//
// Initialize the DirectSound3D system.  Call the initalization for the pDS3D_listener
// 
// returns:     -1	=> init failed
//              0		=> success
int ds3d_init(int voice_manager_required)
{
	if ( DS3D_inited == TRUE )
		return 0;

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

