/****************************************************************************

Module name:	SWFF_LIB.cpp

	(C) Copyright Microsoft Corp. 1993.  All rights reserved.

	You have a royalty-free right to use, modify, reproduce and 
	distribute the Library Files (and/or any modified version) in 
	any way you find useful, provided that you agree that 
	Microsoft has no warranty obligations or liability for any 
	Sample Application Files which are modified. 


Purpose:	This module provides routines to simplify creating Effects
			using the DirectInput Force Feedback subsystem.
					
Algorithm:	

Version	Date		Author	Comments
-------	---------	------	--------
 1.1	01-Apr-97	MEA/DJS
		15-Apr-97	MEA		Moved prototype SWFF_SetDuration to sw_force.h
		16-Apr-97	DMS		Added SWFF_CreateEffectFromVFXEx
		14-May-97	DMS		Added SWFF_PutRawAxisForce 
							  and SWFF_CreateRawAxisForceEffect
  		22-May-97	DMS		Added SWFF_CreateEffectFromVFX2
							  and SWFF_CreateEffectFromVFX2Ex
							  and SWFF_CreateEffectFromVFXBuffer


****************************************************************************/


#include "globalincs/pstypes.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmsystem.h"
#include "io/sw_force.h"		// SideWinder Force Feedback Header File

#define INITGUIDS       // Make GUID available
#include "sw_guid.hpp"
#include "directx/vdinput.h"
#undef INITGUIDS


BOOL CALLBACK DIEnumAndDestroyCreatedEffectsProc(LPDIRECTINPUTEFFECT pDIEffect, LPVOID lpvRef);
BOOL CALLBACK DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi, LPVOID lpvContext);


// ----------------------------------------------------------------------------
//
// ***** FUNCTIONS ************************************************************
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Function: 	SWFF_OpenDefaultFFJoystick
// Parameters:	HWND hWnd						  - Client Window Handle 
//				LPDIRECTINPUT* ppDI				  - Pointer to DIRECTINPUT
//				LPDIRECTINPUTDEVICE2* ppDIDevice) - Pointer to IDIRECTINPUTDEVICE2
//
// Returns:
// Algorithm:
// Comments:
// ----------------------------------------------------------------------------
HRESULT SWFF_OpenDefaultFFJoystick(
	IN HWND hWnd,
	LPDIRECTINPUT* ppDI, 
	LPDIRECTINPUTDEVICE2* ppDIDevice)
{
	HRESULT hResult;
	if(hWnd == NULL || ppDI == NULL || ppDIDevice == NULL)
	{
		return SFERR_INVALID_PARAM;
	}

	// create the DirectInput object
	hResult = DirectInputCreate(GetModuleHandle(NULL), DIRECTINPUT_VERSION, ppDI, NULL);
	if(FAILED(hResult))
		return hResult;
	
	// enumerate the first attached joystick
	// instance goes in pDIDeviceInstance
	DIDEVICEINSTANCE DIDeviceInstance;
	DIDeviceInstance.dwDevType = 0;
	hResult = (*ppDI)->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumDevicesProc, &DIDeviceInstance, DIEDFL_FORCEFEEDBACK);
	if(FAILED(hResult))
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	if(DIDeviceInstance.dwDevType == 0)
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return DIERR_DEVICENOTREG;
	}

	// create the DirectInput Device object
	LPDIRECTINPUTDEVICE pDIDevice = NULL;
	hResult = (*ppDI)->CreateDevice(DIDeviceInstance.guidInstance, &pDIDevice, NULL);
	if(FAILED(hResult))
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// get a pointer to its DirectInputDevice2 interface
	hResult = pDIDevice->QueryInterface(IID_IDirectInputDevice2, (void**)ppDIDevice);
	if(FAILED(hResult))
	{
		pDIDevice->Release();
		pDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	pDIDevice->Release();
	pDIDevice = NULL;

	// set the data format to the pre-defined DirectInput joystick format
	hResult = (*ppDIDevice)->SetDataFormat(&c_dfDIJoystick);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// set the cooperative level
	hResult = (*ppDIDevice)->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// turn auto-center off
/*	DIPROPDWORD DIPropAutoCenter;
	DIPropAutoCenter.diph.dwSize = sizeof(DIPropAutoCenter);
	DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIPropAutoCenter.diph.dwObj = 0;
	DIPropAutoCenter.diph.dwHow = DIPH_DEVICE;
	DIPropAutoCenter.dwData = 0;

	hResult = (*ppDIDevice)->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
*/
	// acquire the joystick
	hResult = (*ppDIDevice)->Acquire();
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	return SUCCESS;
} 


// ----------------------------------------------------------------------------
// Function: 	SWFF_OpenDefaultFFJoystickEx
// Parameters:	HWND hWnd						  - Client Window Handle 
//				LPDIRECTINPUT* ppDI				  - Pointer to IDIRECTINPUT
//				HINSTANCE hInstance				  - object instance handle
//				LPDIRECTINPUTDEVICE2* ppDIDevice) - Pointer to IDIRECTINPUTDEVICE2
//				DWORD dwFlags					  - DISCL_EXCLUSIVE | DISCL_FOREGROUND
//
// Returns:
// Algorithm:
// Comments:
// ----------------------------------------------------------------------------
HRESULT SWFF_OpenDefaultFFJoystickEx(
	IN HWND hWnd,
	IN HINSTANCE hInstance,
	OUT LPDIRECTINPUT* ppDI, 
	OUT LPDIRECTINPUTDEVICE2* ppDIDevice,
	IN DWORD dwFlags)
{
	HRESULT hResult;
	if(hWnd == NULL || hInstance == NULL || ppDI == NULL || ppDIDevice == NULL)
	{
		return SFERR_INVALID_PARAM;
	}

	// create the DirectInput object
	hResult = DirectInputCreate(hInstance, DIRECTINPUT_VERSION, ppDI, NULL);
	if(FAILED(hResult))
		return hResult;
	
	// enumerate the first attached joystick
	// instance goes in pDIDeviceInstance
	DIDEVICEINSTANCE DIDeviceInstance;
	DIDeviceInstance.dwDevType = 0;
	hResult = (*ppDI)->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumDevicesProc, &DIDeviceInstance, DIEDFL_FORCEFEEDBACK);
	if(FAILED(hResult))
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	if(DIDeviceInstance.dwDevType == 0)
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return DIERR_DEVICENOTREG;
	}

	// create the DirectInput Device object
	LPDIRECTINPUTDEVICE pDIDevice = NULL;
	hResult = (*ppDI)->CreateDevice(DIDeviceInstance.guidInstance, &pDIDevice, NULL);
	if(FAILED(hResult))
	{
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// get a pointer to its DirectInputDevice2 interface
	hResult = pDIDevice->QueryInterface(IID_IDirectInputDevice2, (void**)ppDIDevice);
	if(FAILED(hResult))
	{
		pDIDevice->Release();
		pDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	pDIDevice->Release();
	pDIDevice = NULL;

	// set the data format to the pre-defined DirectInput joystick format
	hResult = (*ppDIDevice)->SetDataFormat(&c_dfDIJoystick);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// set the cooperative level
	hResult = (*ppDIDevice)->SetCooperativeLevel(hWnd, dwFlags);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// turn auto-center off
	DIPROPDWORD DIPropAutoCenter;
	DIPropAutoCenter.diph.dwSize = sizeof(DIPropAutoCenter);
	DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIPropAutoCenter.diph.dwObj = 0;
	DIPropAutoCenter.diph.dwHow = DIPH_DEVICE;
	DIPropAutoCenter.dwData = 0;

	hResult = (*ppDIDevice)->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}

	// acquire the joystick
	hResult = (*ppDIDevice)->Acquire();
	if(FAILED(hResult))
	{
		(*ppDIDevice)->Release();
		*ppDIDevice = NULL;
		(*ppDI)->Release();
		*ppDI = NULL;
		return hResult;
	}
	return SUCCESS;
} 


BOOL CALLBACK DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi, LPVOID lpvContext)
{
	LPDIDEVICEINSTANCE pDIDeviceInstance = (LPDIDEVICEINSTANCE)lpvContext;
	if(pDIDeviceInstance == NULL)
	{
		return DIENUM_STOP;
	}

	if(GET_DIDEVICE_TYPE(lpddi->dwDevType) == DIDEVTYPE_JOYSTICK)
	{
		memcpy((LPVOID)pDIDeviceInstance, (LPVOID)lpddi, sizeof(*pDIDeviceInstance));

		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_DestroyEffect
// Purpose:		Destroys one effect or all effects
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT pDIEffect	- Effect to destroy
//												- NULL destroys all effects
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_DestroyEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN LPDIRECTINPUTEFFECT pDIEffect)
{
	HRESULT hResult = SUCCESS;
	if(pDIEffect != NULL)
	{
		pDIEffect->Release();
	}
	else
		hResult = SWFF_DestroyAllEffects(pDIDevice);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_DestroyAllEffects
// Purpose:		Destroys all created effects
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_DestroyAllEffects(
	IN LPDIRECTINPUTDEVICE2 pDIDevice)
{
	HRESULT hResult;

	if(pDIDevice == NULL)
		return SFERR_INVALID_PARAM;

	hResult = pDIDevice->EnumCreatedEffectObjects(DIEnumAndDestroyCreatedEffectsProc, NULL, 0);

	return hResult;
}

#pragma warning(disable:4100)	// unreferenced formal parameter
BOOL CALLBACK DIEnumAndDestroyCreatedEffectsProc(LPDIRECTINPUTEFFECT pDIEffect, LPVOID lpvRef)
{
	pDIEffect->Release();

	return DIENUM_CONTINUE;
}
#pragma warning(default:4100)	// unreferenced formal parameter

// ----------------------------------------------------------------------------
// Function: 	SWFF_SetGain
// Purpose:		Sets Gain for the Effect
// Parameters:	LPDIRECTINPUTEFFECT	pDIEffect	- Pointer to effect to set the gain for
//				DWORD dwGain					- Gain in 1 to 10000
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_SetGain(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwGain)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	DIEFFECT thisEffect = { sizeof(DIEFFECT) };
	thisEffect.dwGain = dwGain;
	return pDIEffect->SetParameters(&thisEffect, DIEP_GAIN);
}


// ----------------------------------------------------------------------------
// Function: 	SWFF_SetDirection
// Purpose:		Sets 2D Angle Direction for the Effect
// Parameters:	LPDIRECTINPUTEFFECT pDIEffect	- Pointer to effect to set the direction for
//				DWORD dwAngle					- Direction in 0 to 35999
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_SetDirection(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwAngle)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	// set up a DIEFFECT structure so we can change direction
	LONG rglDirection[2];
	DIEFFECT thisEffect = {sizeof(DIEFFECT)};
	thisEffect.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	thisEffect.cAxes = 2;
	thisEffect.rgdwAxes = NULL;
	thisEffect.rglDirection = rglDirection;
	thisEffect.rglDirection[0] = dwAngle;
	return pDIEffect->SetParameters(&thisEffect, DIEP_DIRECTION);
}

HRESULT SWFF_SetDirectionGain(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwAngle,
	IN DWORD dwGain)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	// set up a DIEFFECT structure so we can change direction
	LONG rglDirection[2];
	DIEFFECT thisEffect = {sizeof(DIEFFECT)};

	thisEffect.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	thisEffect.cAxes = 2;
	thisEffect.rgdwAxes = NULL;
	thisEffect.rglDirection = rglDirection;
	thisEffect.rglDirection[0] = dwAngle;
	thisEffect.dwGain = dwGain;
	return pDIEffect->SetParameters(&thisEffect, DIEP_DIRECTION | DIEP_GAIN);
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_SetDuration
// Purpose:		Sets Duration for the Effect
// Parameters:	LPDIRECTINPUTEFFECT pDIEffect	- Pointer to effect to set the duration for
//				DWORD dwDuration				- In uSecs, INFINITE is FOREVER
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_SetDuration(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwDuration)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	DIEFFECT thisEffect = { sizeof(DIEFFECT) };
	thisEffect.dwDuration = dwDuration;
	return pDIEffect->SetParameters(&thisEffect, DIEP_DURATION);
}


// ----------------------------------------------------------------------------
// Function: 	SWFF_PutRawForce
// Purpose:		Sends Force Value,Direction to ff Device
// Parameters:	LPDIRECTINPUTEFFECT pDIEffect	- Pointer to a two axis raw force object
//				LONG lMagnitude					- -10000 to +10000 force value
//				DWORD dwDirection				- 0 to 35999
// Returns:
// Algorithm:	
// Comments:	To use this, you need to create the effect using
//				SWFF_CreateRawForceEffect() first
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_PutRawForce(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwDirection)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	DICONSTANTFORCE DIConstantForceStruct;
	DIConstantForceStruct.lMagnitude = lMagnitude;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT thisEffect = { sizeof(DIEFFECT) };
	thisEffect.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	thisEffect.cAxes		   = 2;
	thisEffect.rgdwAxes		   = rgdwAxes;
	thisEffect.rglDirection	   = rglDirection;

	thisEffect.cbTypeSpecificParams	= sizeof(DICONSTANTFORCE);
	thisEffect.lpvTypeSpecificParams	= &DIConstantForceStruct;
	return pDIEffect->SetParameters(&thisEffect, DIEP_DIRECTION|DIEP_TYPESPECIFICPARAMS);
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_PutRawAxisForce
// Purpose:		Sends Force Value,Direction to ff Device
// Parameters:	LPDIRECTINPUTEFFECT pDIEffect	- Pointer to a one axis raw force object
//				LONG lMagnitude					- -10000 to +10000 force value
// Returns:
// Algorithm:	
// Comments:	To use this, you need to create the effect using
//				SWFF_CreateRawAxisForceEffect() first
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_PutRawAxisForce(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN LONG lMagnitude)
{
	if(pDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	DICONSTANTFORCE DIConstantForceStruct;
	DIConstantForceStruct.lMagnitude = lMagnitude;

	DIEFFECT thisEffect = { sizeof(DIEFFECT) };

	thisEffect.cbTypeSpecificParams	= sizeof(DICONSTANTFORCE);
	thisEffect.lpvTypeSpecificParams	= &DIConstantForceStruct;
	return pDIEffect->SetParameters(&thisEffect, DIEP_TYPESPECIFICPARAMS);
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateRawForceEffect
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				LONG lMagnitude					- -10000 to 10000
//				DWORD dwDirection				- 0 to 35999
//
//				
// Returns:		
// Algorithm:
// Comments:	Create this Effect once, then use SetParameter(...) to play the
//				force value
//
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateRawForceEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwDirection)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pRawForce;

	DICONSTANTFORCE DIConstantForceStruct;
	DIConstantForceStruct.lMagnitude = lMagnitude;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffect;
	DIEffect.dwSize					= sizeof(DIEFFECT);
	DIEffect.dwFlags				= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffect.dwDuration				= INFINITE;
	DIEffect.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffect.dwGain					= 10000;
	DIEffect.dwTriggerButton		= DIEB_NOTRIGGER;
	DIEffect.dwTriggerRepeatInterval= 0;
	DIEffect.cAxes					= 2;
	DIEffect.rgdwAxes				= rgdwAxes;
	DIEffect.rglDirection			= rglDirection;
	DIEffect.lpEnvelope				= NULL;
	DIEffect.cbTypeSpecificParams	= sizeof(DICONSTANTFORCE);
	DIEffect.lpvTypeSpecificParams	= &DIConstantForceStruct;

	HRESULT hRet;
	hRet = pDIDevice->CreateEffect(GUID_RawForce, &DIEffect, &pRawForce, NULL);
	if(FAILED(hRet)) return hRet;

	*ppDIEffect = pRawForce;
	return SUCCESS;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateRawAxisForceEffect
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice		- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect		- Receives pointer to created effect
//				LONG lMagnitude						- -10000 to 10000
//				DWORD dwAxis						- Either X_AXIS or Y_AXIS
//
//				
// Returns:		
// Algorithm:
// Comments:	Create this Effect once, then use SetParameter(...) to play the
//				force value
//
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateRawAxisForceEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwAxis)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	if(!(dwAxis == X_AXIS || dwAxis == Y_AXIS))
		return SFERR_INVALID_PARAM;

	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pRawForce;

	DICONSTANTFORCE DIConstantForceStruct;
	DIConstantForceStruct.lMagnitude = lMagnitude;

	DWORD rgdwAxes[1];
	if(dwAxis == X_AXIS)
		rgdwAxes[0] = DIJOFS_X;
	else
		rgdwAxes[0] = DIJOFS_Y;

	LONG rglDirection[1];
	rglDirection[0] = 0;

	DIEFFECT DIEffect;
	DIEffect.dwSize					= sizeof(DIEFFECT);
	DIEffect.dwFlags				= DIEFF_OBJECTOFFSETS | DIEFF_CARTESIAN;
	DIEffect.dwDuration				= INFINITE;
	DIEffect.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffect.dwGain					= 10000;
	DIEffect.dwTriggerButton		= DIEB_NOTRIGGER;
	DIEffect.dwTriggerRepeatInterval= 0;
	DIEffect.cAxes					= 1;
	DIEffect.rgdwAxes				= rgdwAxes;
	DIEffect.rglDirection			= rglDirection;
	DIEffect.lpEnvelope				= NULL;
	DIEffect.cbTypeSpecificParams	= sizeof(DICONSTANTFORCE);
	DIEffect.lpvTypeSpecificParams	= &DIConstantForceStruct;

	HRESULT hRet;
	hRet = pDIDevice->CreateEffect(GUID_RawForce, &DIEffect, &pRawForce, NULL);
	if(FAILED(hRet)) return hRet;

	*ppDIEffect = pRawForce;
	return SUCCESS;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateROMEffect
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				REFGUID refGUID					- GUID for ROM Effect
//				DWORD dwDuration				- uS
//				DWORD dwGain					- 1 to 10000
//				DWORD dwDirection				- 0 to 35999
//				LONG lButton					- Index of playback button, -1 for none
//
// Returns:		
// Algorithm:
// Comments:	Assumes valid GUID for the ROM Effect
// Note:		If unmodified ROM Effect, user has to pass
//				   DEFAULT_ROM_EFFECT_DURATION,  DEFAULT_ROM_EFFECT_GAIN
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateROMEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN REFGUID refGUID,
	IN DWORD dwDuration,		
	IN DWORD dwGain,
	IN DWORD dwDirection,		
	IN LONG lButton)			
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pROMEffect = NULL;

	// Default NO Envelope
	DIENVELOPE DIEnvelopeStruct;				
	DIEnvelopeStruct.dwSize = sizeof(DIENVELOPE);
	DIEnvelopeStruct.dwAttackTime = 0;
	DIEnvelopeStruct.dwAttackLevel = 10000;
	DIEnvelopeStruct.dwFadeTime = 0;
	DIEnvelopeStruct.dwFadeLevel = 10000;

	// 2DOF
	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffect;
	DIEffect.dwSize					= sizeof(DIEFFECT);
	DIEffect.dwFlags				= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	// Set Duration and Gain to use Default ROM Effect params unless overridden
	DIEffect.dwDuration				= dwDuration;	// can be DEFAULT_ROM_EFFECT_DURATION
	DIEffect.dwSamplePeriod			= DEFAULT_ROM_EFFECT_OUTPUTRATE;
	DIEffect.dwGain					= dwGain;		// can be DEFAULT_ROM_EFFECT_GAIN;	
	//
	DIEffect.dwTriggerButton		= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffect.dwTriggerRepeatInterval= 0;
	DIEffect.cAxes					= 2;
	DIEffect.rgdwAxes				= rgdwAxes;
	DIEffect.rglDirection			= rglDirection;
	DIEffect.lpEnvelope				= &DIEnvelopeStruct;
	DIEffect.cbTypeSpecificParams	= 0;
	DIEffect.lpvTypeSpecificParams	= NULL;

	HRESULT hRet = pDIDevice->CreateEffect(refGUID, &DIEffect, &pROMEffect, NULL);
	if(FAILED(hRet)) return hRet;

	*ppDIEffect = pROMEffect;
	return SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_WriteRegString
// Parameters:	LPCTSTR		pszKey		-	The key under HKCR to place the value
//				LPCTSTR		pszValue	-	The string value for pszKey
//				
// Returns:		TRUE if the registry entry was successfully made
// Algorithm:
// Comments:	Helper function for SWFF_RegisterVFXObject to write registry entries
// Note:
// ----------------------------------------------------------------------------
BOOL SWFF_WriteRegString(
	IN LPCTSTR pszKey,
	IN LPCTSTR pszValue)
{
	HKEY hKey;
	LONG lRet;
	int nLen;

	if(pszKey == NULL || pszValue == NULL)
		return FALSE;

	// create it
	hKey = HKEY_CLASSES_ROOT;
	lRet = RegCreateKey(hKey, pszKey, &hKey);
	if(lRet != ERROR_SUCCESS)
		return FALSE;

	// save the value into the key
	nLen = strlen(pszValue);
	lRet = RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)pszValue, nLen + 1);
	if(lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return FALSE;
	}

	// close the key
	RegCloseKey(hKey);
/*	if(lRet != ERROR_SUCCESS)	// eh, this is unreachable code
		return FALSE;*/

	// if we have reached this point, then it was a success
	return TRUE;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_RegisterVFXObject
// Parameters:	LPCTSTR		- Pointer to the fully-qualified path name of VFX.DLL
//				
// Returns:		TRUE if the object was successfully registered
// Algorithm:
// Comments:	Example of code to register the VFX com object.  
//				You supply lpszVFXPath depending on where you install VFX.DLL
// Note:
// ----------------------------------------------------------------------------
#define GUID_VFX_Object "{04ace0a7-1fa8-11d0-aa22-00a0c911f471}"
BOOL SWFF_RegisterVFXObject(IN LPCTSTR pszVFXPath)
{
	if(pszVFXPath == NULL)
		return FALSE;

	return	SWFF_WriteRegString("\\VFX1.0", "VFX Object")
		&&	SWFF_WriteRegString("\\VFX1.0\\CLSID", GUID_VFX_Object)
		&&	SWFF_WriteRegString("\\VFX", "VFX Object")
		&&	SWFF_WriteRegString("\\VFX\\CurVer", "VFX1.0")
		&&	SWFF_WriteRegString("\\VFX\\CLSID", GUID_VFX_Object)
		&&	SWFF_WriteRegString("\\CLSID\\"GUID_VFX_Object, "VFX Object")
		&&	SWFF_WriteRegString("\\CLSID\\"GUID_VFX_Object"\\VersionIndependentProgID", "VFX")
		&&	SWFF_WriteRegString("\\CLSID\\"GUID_VFX_Object"\\InprocServer32", pszVFXPath)
		&&	SWFF_WriteRegString("\\CLSID\\"GUID_VFX_Object"\\NotInsertable", "");
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateVFXEffectFromFile
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				TCHAR *pszFileName				- Pointer to VFX File name
//				
// Returns:		
// Algorithm:
// Comments:
// Note:
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateVFXEffectFromFile(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName)
{
	if(pDIDevice == NULL || ppDIEffect == NULL || pszFileName == NULL)
		return SFERR_INVALID_PARAM;

	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pVFXEffect;

	VFX_PARAM VFXParam;
	VFXParam.m_Bytes = sizeof(VFX_PARAM);
	VFXParam.m_PointerType = VFX_FILENAME;
	VFXParam.m_BufferSize = 0;
	VFXParam.m_pFileNameOrBuffer = (PVOID) pszFileName;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = DEFAULT_VFX_EFFECT_DIRECTION;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= DEFAULT_VFX_EFFECT_DURATION;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= DEFAULT_VFX_EFFECT_GAIN;
	DIEffectStruct.dwTriggerButton			= DIEB_NOTRIGGER;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(VFX_PARAM);
	DIEffectStruct.lpvTypeSpecificParams	= &VFXParam;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_VFXEffect, &DIEffectStruct, &pVFXEffect, NULL);
	if(FAILED(hResult)) return hResult;

	*ppDIEffect = pVFXEffect;
	return hResult;
} 
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateVFXEffectFromFileEx
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				TCHAR *pszFileName				- Pointer to VFX File
//				DWORD dwDuration				- INFINITE or default
//				DWORD dwGain					- 1 to 10000
//				DWORD dwDirection				- 0 to 35999
//				
// Returns:		
// Algorithm:
// Comments:
// Note:
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateVFXEffectFromFileEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName,
	IN DWORD dwDuration,
	IN DWORD dwGain,
	IN DWORD dwDirection)
{
	if(pDIDevice == NULL || ppDIEffect == NULL || pszFileName == NULL)
		return SFERR_INVALID_PARAM;
	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pVFXEffect;

	VFX_PARAM VFXParam;
	VFXParam.m_Bytes = sizeof(VFX_PARAM);
	VFXParam.m_PointerType = VFX_FILENAME;
	VFXParam.m_BufferSize = 0;
	VFXParam.m_pFileNameOrBuffer = (PVOID) pszFileName;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= dwGain;
	DIEffectStruct.dwTriggerButton			= DIEB_NOTRIGGER;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(VFX_PARAM);
	DIEffectStruct.lpvTypeSpecificParams	= &VFXParam;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_VFXEffect, &DIEffectStruct, &pVFXEffect, NULL);
	if(FAILED(hResult)) return hResult;

	*ppDIEffect = pVFXEffect;
	return hResult;
} 
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateVFXEffectFromBuffer
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				LPVOID pBuffer					- Pointer to VFX Buffer
//				DWORD dwBufferSize				- Buffer size in bytes
//				
// Returns:		
// Algorithm:
// Comments:
// Note:
//		If you are compiling the FRC files as resources in your executable
//		by putting #include "script.vfx" in your .rc file, you would use
//		code similar to the following to create the effect (no error checking).
//	
//			HRSRC hResInfo = FindResource(NULL, "IDF_FOO", "FORCE");
//			DWORD dwBytes = SizeofResource(NULL, hResInfo);
//			HGLOBAL hRsrc = LoadResource(NULL, hResInfo);
//			PVOID pBuffer = LockResource(hRsrc);
//			SWFF_CreateEffectFromVFXBuffer(pDIDevice, pBuffer, dwBytes, &pDIEffect);
//
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateVFXEffectFromBuffer(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize)
{
	if(pDIDevice == NULL || ppDIEffect == NULL || pBuffer == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pVFXEffect;

	VFX_PARAM VFXParam;
	VFXParam.m_Bytes = sizeof(VFX_PARAM);
	VFXParam.m_PointerType = VFX_BUFFER;
	VFXParam.m_BufferSize = dwBufferSize;
	VFXParam.m_pFileNameOrBuffer = pBuffer;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = DEFAULT_VFX_EFFECT_DIRECTION;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= DEFAULT_VFX_EFFECT_DURATION;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= DEFAULT_VFX_EFFECT_GAIN;
	DIEffectStruct.dwTriggerButton			= DIEB_NOTRIGGER;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(VFX_PARAM);
	DIEffectStruct.lpvTypeSpecificParams	= &VFXParam;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_VFXEffect, &DIEffectStruct, &pVFXEffect, NULL);
	if(FAILED(hResult)) return hResult;

	*ppDIEffect = pVFXEffect;
	return hResult;
} 
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateVFXEffectFromBufferEx
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				LPVOID pBuffer					- Pointer to VFX Buffer
//				DWORD dwBufferSize				- Buffer size in bytes
//				DWORD dwDuration				- INFINITE or default
//				DWORD dwGain					- 1 to 10000
//				DWORD dwDirection				- 0 to 35999
//				
// Returns:		
// Algorithm:
// Comments:
// Note:
//		See note for SWFF_CreateVFXEffectFromBuffer(...)
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateVFXEffectFromBufferEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize,
	IN DWORD dwDuration,
	IN DWORD dwGain,
	IN DWORD dwDirection)
{
	if(pDIDevice == NULL || ppDIEffect == NULL || pBuffer == NULL)
		return SFERR_INVALID_PARAM;
	// Always clear return IPtr
	*ppDIEffect = NULL;

	LPDIRECTINPUTEFFECT  pVFXEffect;

	VFX_PARAM VFXParam;
	VFXParam.m_Bytes = sizeof(VFX_PARAM);
	VFXParam.m_PointerType = VFX_BUFFER;
	VFXParam.m_BufferSize = dwBufferSize;
	VFXParam.m_pFileNameOrBuffer = pBuffer;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= dwGain;
	DIEffectStruct.dwTriggerButton			= DIEB_NOTRIGGER;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(VFX_PARAM);
	DIEffectStruct.lpvTypeSpecificParams	= &VFXParam;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_VFXEffect, &DIEffectStruct, &pVFXEffect, NULL);
	if(FAILED(hResult)) return hResult;

	*ppDIEffect = pVFXEffect;
	return hResult;
} 
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateDIEffectFromFile
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				TCHAR *pszFileName				- Pointer to VFX File name
//				
// Returns:		
// Algorithm:
// Comments:
// Note:		If the file contains multiple effects or a custom effect this
//				function will fail. Use SWFF_CreateDIEffectFromFileEx.
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateDIEffectFromFile(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName)
{
	HRESULT hResult;
	PVFX pIVFX;

	if(pDIDevice == NULL || pszFileName == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	*ppDIEffect = NULL;

	hResult = CoInitialize(NULL);
	if(FAILED(hResult))
		return hResult;

	hResult = CoCreateInstance(CLSID_VFX, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				IID_IVFX,
				(void**)&pIVFX);
	if(pIVFX == NULL)
	{
		CoUninitialize();
		return SFERR_SYSTEM_INIT;
	}
	
	if(FAILED(hResult))
	{
		CoUninitialize();
		return hResult;
	}

	// Create the Effect from a *.frc file
	DWORD dwInFlags = VFXCE_CREATE_SINGLE;
	hResult = pIVFX->CreateEffectFromFile(pDIDevice, ppDIEffect,
				0, pszFileName, NULL, NULL, dwInFlags, NULL);

	// clean up
	pIVFX->Release();
	CoUninitialize();

	if(FAILED(hResult)) return hResult;

	return hResult;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateDIEffectFromFileEx
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT** pppDIEffect	- Pointer to an array of 
//														LPDIRECTINPUTEFFECT's.
//														This array is allocated
//														by the function.  Caller is
//														responsible for deleting.
//				PDWORD pdwEffectCount			- Gets the number of effects in array
//				TCHAR *pszFileName				- Pointer to VFX File name
//				PPVOID ppUDBuffer				- Gets an array containing custom force 
//													samples.  This array is allocated by
//													the function.  Caller is 
//													responsible for deleting.
//				PDWORD pdwOutFlags				- Receives 0 if the file contains
//													a single effect.  Otherwise
//													it receives VFXCE_CONCATENATE
//													or VFXCE_SUPERIMPOSE.
//				
// Returns:		
// Algorithm:
// Comments:
// Note:	call delete [] pppDIEffect and delete [] ppUDBuffer after
//			releasing the effects
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateDIEffectFromFileEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT** pppDIEffect,
	IN OUT PDWORD pdwEffectCount,
	IN const TCHAR *pszFileName,
	IN OUT void** ppUDBuffer,
	IN OUT PDWORD pdwOutFlags)
{
	// parameter check
	if(pDIDevice == NULL || pszFileName == NULL || pppDIEffect == NULL || 
		pdwEffectCount == NULL || ppUDBuffer == NULL || pdwOutFlags == NULL)
	{
		return SFERR_INVALID_PARAM;
	}

	// zero out the return values
	*pppDIEffect = NULL;
	*pdwEffectCount = 0;
	*ppUDBuffer = NULL;
	*pdwOutFlags = 0;

	HRESULT hResult;
	PVFX pIVFX;

	hResult = CoInitialize(NULL);
	if(FAILED(hResult))
		return hResult;

	hResult = CoCreateInstance(CLSID_VFX, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				IID_IVFX,
				(void**)&pIVFX);
	if(pIVFX == NULL)
	{
		CoUninitialize();
		return SFERR_SYSTEM_INIT;
	}
	
	if(FAILED(hResult))
	{
		CoUninitialize();
		return hResult;
	}

	// see how big a DIEffect array we have to allocate, and see how much memory, if any, 
	// we need to allocate for UD sample caching
	DWORD dwInFlags = VFXCE_CALC_BUFFER_SIZE | VFXCE_CALC_EFFECT_COUNT;
	DWORD dwEffectCount;
	DWORD dwBufferSize;
	hResult = pIVFX->CreateEffectFromFile(NULL, NULL,
				&dwEffectCount, pszFileName, NULL, &dwBufferSize, dwInFlags, NULL);
	if(FAILED(hResult))
	{
		pIVFX->Release();
		CoUninitialize();
		return hResult;
	}

	// allocate memory for the effects
	LPDIRECTINPUTEFFECT* ppDIEffect = new LPDIRECTINPUTEFFECT[dwEffectCount];
	if(ppDIEffect == NULL)
	{
		pIVFX->Release();
		CoUninitialize();
		return DIERR_OUTOFMEMORY;
	}

	// allocate memory for the custom force samples
	PVOID pUDBuffer = NULL;
	if(dwBufferSize > 0)
	{
		pUDBuffer = new BYTE[dwBufferSize];
		if(pUDBuffer == NULL)
		{
			delete [] ppDIEffect;
			ppDIEffect = NULL;
			pIVFX->Release();
			CoUninitialize();
			return DIERR_OUTOFMEMORY;
		}
	}

	// Create the Effect from a *.frc file
	DWORD dwOutFlags;
	dwInFlags = VFXCE_CREATE_MULTIPLE;
	hResult = pIVFX->CreateEffectFromFile(pDIDevice, ppDIEffect,
				&dwEffectCount, pszFileName, pUDBuffer, &dwBufferSize, dwInFlags, &dwOutFlags);

	if(FAILED(hResult))
	{
		delete [] ppDIEffect;
		ppDIEffect = NULL;
		delete [] pUDBuffer;
		pUDBuffer = NULL;
		pIVFX->Release();
		CoUninitialize();
		return hResult;
	}

	// clean up
	pIVFX->Release();
	CoUninitialize();

	if(FAILED(hResult)) return hResult;

	// assign the results
	*pppDIEffect = ppDIEffect;
	*pdwEffectCount = dwEffectCount;
	*ppUDBuffer = pUDBuffer;
	*pdwOutFlags = dwOutFlags;

	return hResult;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateDIEffectFromBuffer
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				LPVOID pBuffer					- Pointer to VFX Buffer
//				DWORD dwBufferSize				- Buffer size in bytes
//				
// Returns:		
// Algorithm:
// Comments:
// Note:		If the file contains multiple effects or a custom effect this
//				function will fail. Use SWFF_CreateDIEffectFromBufferEx.
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateDIEffectFromBuffer(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize)
{
	HRESULT hResult;
	PVFX pIVFX;

	if(pDIDevice == NULL || pBuffer == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	*ppDIEffect = NULL;

	hResult = CoInitialize(NULL);
	if(FAILED(hResult))
		return hResult;

	hResult = CoCreateInstance(CLSID_VFX, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				IID_IVFX,
				(void**)&pIVFX);
	if(pIVFX == NULL)
	{
		CoUninitialize();
		return SFERR_SYSTEM_INIT;
	}
	
	if(FAILED(hResult))
	{
		CoUninitialize();
		return hResult;
	}

	// Create the Effect from a *.frc file
	DWORD dwInFlags = VFXCE_CREATE_SINGLE;
	hResult = pIVFX->CreateEffectFromBuffer(pDIDevice, ppDIEffect,
				0, pBuffer, dwBufferSize, NULL, NULL, dwInFlags, NULL);

	// clean up
	pIVFX->Release();
	CoUninitialize();

	if(FAILED(hResult)) return hResult;

	return hResult;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateDIEffectFromBufferEx
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- Pointer to DirectInputDevice
//				LPDIRECTINPUTEFFECT** pppDIEffect	- Pointer to an array of 
//														LPDIRECTINPUTEFFECT's.
//														This array is allocated
//														by the function.  Caller is
//														responsible for deleting.
//				PDWORD pdwEffectCount			- Gets the number of effects in array
//				LPVOID pBuffer					- Pointer to VFX Buffer
//				DWORD dwBufferSize				- Buffer size in bytes
//				PPVOID ppUDBuffer				- Gets an array containing custom force 
//													samples.  This array is allocated by
//													the function.  Caller is 
//													responsible for deleting.
//				PDWORD pdwOutFlags				- Receives 0 if the file contains
//													a single effect.  Otherwise
//													it receives VFXCE_CONCATENATE
//													or VFXCE_SUPERIMPOSE.
//				
// Returns:		
// Algorithm:
// Comments:
// Note:	call delete [] pppDIEffect and delete [] ppUDBuffer after
//			releasing the effects
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateDIEffectFromBufferEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT** pppDIEffect,
	IN OUT PDWORD pdwEffectCount,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize,
	IN OUT void** ppUDBuffer,
	IN OUT PDWORD pdwOutFlags)
{
	// parameter check
	if(pDIDevice == NULL || pBuffer == NULL || pppDIEffect == NULL|| 
		pdwEffectCount == NULL || ppUDBuffer == NULL || pdwOutFlags == NULL)
	{
		return SFERR_INVALID_PARAM;
	}

	// zero out the return values
	*pppDIEffect = NULL;
	*pdwEffectCount = 0;
	*ppUDBuffer = NULL;
	*pdwOutFlags = 0;

	HRESULT hResult;
	PVFX pIVFX;



	hResult = CoInitialize(NULL);
	if(FAILED(hResult))
		return hResult;

	hResult = CoCreateInstance(CLSID_VFX, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				IID_IVFX,
				(void**)&pIVFX);
	if(pIVFX == NULL)
	{
		CoUninitialize();
		return SFERR_SYSTEM_INIT;
	}
	
	if(FAILED(hResult))
	{
		CoUninitialize();
		return hResult;
	}

	// see how big a DIEffect array we have to allocate, and see how much memory, if any, 
	// we need to allocate for UD sample caching
	DWORD dwInFlags = VFXCE_CALC_BUFFER_SIZE | VFXCE_CALC_EFFECT_COUNT;
	DWORD dwEffectCount;
	DWORD dwUDBufferSize;
	hResult = pIVFX->CreateEffectFromBuffer(NULL, NULL,
				&dwEffectCount, pBuffer, dwBufferSize, NULL, &dwUDBufferSize, dwInFlags, NULL);
	if(FAILED(hResult))
	{
		pIVFX->Release();
		CoUninitialize();
		return  hResult;
	}

	// allocate memory for the effects
	LPDIRECTINPUTEFFECT* ppDIEffect = new LPDIRECTINPUTEFFECT[dwEffectCount];
	if(ppDIEffect == NULL)
	{
		pIVFX->Release();
		CoUninitialize();
		return DIERR_OUTOFMEMORY;
	}

	// allocate memory for the custom force samples
	PVOID pUDBuffer = NULL;
	if(dwUDBufferSize > 0)
	{
		pUDBuffer = new BYTE[dwUDBufferSize];
		if(pUDBuffer == NULL)
		{
			delete [] ppDIEffect;
			ppDIEffect = NULL;
			pIVFX->Release();
			CoUninitialize();
			return DIERR_OUTOFMEMORY;
		}
	}

	// Create the Effect from a *.frc file
	DWORD dwOutFlags;
	dwInFlags = VFXCE_CREATE_MULTIPLE;
	hResult = pIVFX->CreateEffectFromBuffer(pDIDevice, ppDIEffect,
				&dwEffectCount, pBuffer, dwBufferSize, pUDBuffer, &dwUDBufferSize, dwInFlags, &dwOutFlags);

	if(FAILED(hResult))
	{
		delete [] ppDIEffect;
		ppDIEffect = NULL;
		delete [] pUDBuffer;
		pUDBuffer = NULL;
		pIVFX->Release();
		CoUninitialize();
		return hResult;
	}

	// clean up
	pIVFX->Release();
	CoUninitialize();

	if(FAILED(hResult)) return hResult;

	// assign the results
	*pppDIEffect = ppDIEffect;
	*pdwEffectCount = dwEffectCount;
	*ppUDBuffer = pUDBuffer;
	*pdwOutFlags = dwOutFlags;

	return hResult;
}
#endif

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreatePeriodicEffect
// Purpose:		Creates a Periodic type Effect with specified params
// Parameters:  LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwType			- Type of PERIODIC Effect (SINE | COSINE | ...)
//				DWORD dwDuration		- uS
//				DWORD dwPeriod			- uS
//				DWORD dwDirection		- 0 to 35999
//				DWORD dwMagnitude		- 0 to 10000
//				LONG lOffset			- Offset in -10000 to 10000
//				DWORD dwAttackTime		- Envelope Attack Time in uS
//				DWORD dwAttackLevel		- Envelope Attack Level in 0 to 10000
//				DWORD dwFadeTime		- Envelope Fade time in uS
//				DWORD dwFadeLevel		- Envelope Fade Level
//				LONG lButton			- Index of playback button, -1 for none
//
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreatePeriodicEffect(	IN LPDIRECTINPUTDEVICE2 pDIDevice,
								IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
								IN DWORD dwType,
								IN DWORD dwDuration,		
								IN DWORD dwPeriod,			
								IN DWORD dwDirection,		
								IN DWORD dwMagnitude,		
								IN LONG lOffset,			
								IN DWORD dwAttackTime,		
								IN DWORD dwAttackLevel,		
								IN DWORD dwFadeTime,		
								IN DWORD dwFadeLevel,		
								IN LONG lButton)			
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	// Always clear return IPtr
	*ppDIEffect = NULL;

	// type-specific stuff
	DWORD dwPhase = 0;
	GUID guid;
	switch(dwType)
	{
		case SINE:
			guid = GUID_Sine;
			break;
		case COSINE:
			guid = GUID_Sine;
			dwPhase = 9000;
			break;
		case SQUARE_HIGH:
			guid = GUID_Square;
			break;
		case SQUARE_LOW:
			guid = GUID_Square;
			dwPhase = 18000;
			break;
		case TRIANGLE_UP:
			guid = GUID_Triangle;
			break;
		case TRIANGLE_DOWN:
			guid = GUID_Triangle;
			dwPhase = 18000;
			break;
		case SAWTOOTH_UP:
			guid = GUID_SawtoothUp;
			break;
		case SAWTOOTH_DOWN:
			guid = GUID_SawtoothDown;
			break;
		default:
			// illegal
			break;
	}

	DIPERIODIC DIPeriodicStruct;
	DIPeriodicStruct.dwMagnitude = dwMagnitude;
	DIPeriodicStruct.lOffset = lOffset;
	DIPeriodicStruct.dwPhase = dwPhase;
	DIPeriodicStruct.dwPeriod = dwPeriod;

	DIENVELOPE DIEnvelopeStruct;
	DIEnvelopeStruct.dwSize = sizeof(DIENVELOPE);
	DIEnvelopeStruct.dwAttackTime = dwAttackTime;
	DIEnvelopeStruct.dwAttackLevel = dwAttackLevel;
	DIEnvelopeStruct.dwFadeTime = dwFadeTime;
	DIEnvelopeStruct.dwFadeLevel = dwFadeLevel;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= 10000;
	DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= &DIEnvelopeStruct;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(DIPeriodicStruct);
	DIEffectStruct.lpvTypeSpecificParams	= &DIPeriodicStruct;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(guid, &DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateSpringEffect
// Purpose:		Creates a Spring type Effect with specified params
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration				- Duration in uS
//				LONG lKx						- X-Axis K Coefficient in -10000 to 10000
//				LONG lCenterx					- X-Axis Center in -10000 to 10000
//				LONG lKy						- Y-Axis K Coefficient in -10000 to 10000
//				LONG lCentery					- Y-Axis Center in -10000 to 10000
//				LONG lButton					- Index of playback button, -1 for none
//
// Returns:
// Algorithm:	
// Comments:
//			To create a 1D spring, set the lKx or lKy parameter to 0
//			To create a 2D spring, set both lKx and lKy parameter to non-zero
//				or set both lFx and lFy to zero
//  
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateSpringEffect(	IN LPDIRECTINPUTDEVICE2 pDIDevice,
							IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
							IN DWORD dwDuration,
							IN LONG lKx,
							IN LONG lCenterx,
							IN LONG lKy,
							IN LONG lCentery,
							IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	HRESULT hResult = SWFF_CreateConditionEffect(pDIDevice,
											ppDIEffect,
											SPRING,
											dwDuration,
											lKx, lCenterx,
											lKy, lCentery,
											lButton);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateDamperEffect
// Purpose:		Creates a Damper type Effect with specified params
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration				- Duration in uS
//				LONG lBx						- X-Axis B Coefficient +/-10000
//				LONG lV0x						- X-Axis Initial Velocity +/-10000
//				LONG lBy						- Y-Axis B Coefficient +/-10000
//				LONG lV0y						- Y-Axis Initial Velocity +/-10000
//				LONG lButton					- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
//			To create a 1D Damper, set the lBx or lBy parameter to 0
//			To create a 2D Damper, set both lBx and lBy parameter to non-zero
//				or set both lFx and lFy to zero
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateDamperEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN LONG lBx,
	IN LONG lV0x,
	IN LONG lBy,
	IN LONG lV0y,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	HRESULT hResult = SWFF_CreateConditionEffect(pDIDevice,
											ppDIEffect,
											DAMPER,
											dwDuration,
											lBx, lV0x,
											lBy, lV0y,
											lButton);

	return hResult;
}


// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateInertiaEffect
// Purpose:		Creates an Inertia type Effect with specified params
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration				- Duration in uS
//				LONG lMx						- X-Axis M Coefficient +/-10000
//				LONG lA0x						- X-Axis Initial Acceleration +/-10000
//				LONG lMy						- Y-Axis N Coefficient +/-10000
//				LONG lA0y						- Y-Axis Initial Acceleration +/-10000
//				LONG lButton					- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
//			To create a 1D Inertia, set the lMx or lMy parameter to 0
//			To create a 2D Inertia, set both lMx and lMy parameter to non-zero
//				or set both lFx and lFy to zero
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateInertiaEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN LONG lMx,
	IN LONG lA0x,
	IN LONG lMy,
	IN LONG lA0y,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	HRESULT hResult = SWFF_CreateConditionEffect(pDIDevice,
											ppDIEffect,
											INERTIA,
											dwDuration,
											lMx, lA0x,
											lMy, lA0y,
											lButton);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateFrictionEffect
// Purpose:		Creates a Friction type Effect with specified params
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration				- Duration in uS
//				LONG lFx						- X-Axis F Coefficient +/-10000
//				LONG lFy						- Y-Axis F Coefficient +/-10000
//				LONG lButton					- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
//			To create a 1D Friction, set the lFx or lFy parameter to 0
//			To create a 2D Friction, set both lFx and lFy parameter to non-zero
//				or set both lFx and lFy to zero
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateFrictionEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN LONG lFx,
	IN LONG lFy,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	HRESULT hResult = SWFF_CreateConditionEffect(pDIDevice,
											ppDIEffect,
											FRICTION,
											dwDuration,
											lFx, 0,
											lFy, 0,
											lButton);

	return hResult;
}

HRESULT SWFF_CreateConditionEffectStruct(
	di_condition_effect_struct *ptr,
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwType,
	IN DWORD dwDuration,
	IN LONG lXCoefficient,
	IN LONG lXOffset,
	IN LONG lYCoefficient,
	IN LONG lYOffset,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	// Always clear return IPtr
	*ppDIEffect = NULL;

	GUID guid;
	switch(dwType)
	{
		case SPRING:
			guid = GUID_Spring;
			break;
		case INERTIA:
			guid = GUID_Inertia;
			break;
		case DAMPER:
			guid = GUID_Damper;
			break;
		case FRICTION:
			guid = GUID_Friction;
			break;
		default:
			break;
	}

	ptr->DIConditionStruct[0].lOffset				= lXOffset;
	ptr->DIConditionStruct[0].lPositiveCoefficient	= lXCoefficient;
	ptr->DIConditionStruct[0].lNegativeCoefficient	= lXCoefficient;
	ptr->DIConditionStruct[0].dwPositiveSaturation	= 10000;
	ptr->DIConditionStruct[0].dwNegativeSaturation	= 10000;
	ptr->DIConditionStruct[0].lDeadBand				= 0;
	ptr->DIConditionStruct[1].lOffset				= lYOffset;
	ptr->DIConditionStruct[1].lPositiveCoefficient	= lYCoefficient;
	ptr->DIConditionStruct[1].lNegativeCoefficient	= lYCoefficient;
	ptr->DIConditionStruct[1].dwPositiveSaturation	= 10000;
	ptr->DIConditionStruct[1].dwNegativeSaturation	= 10000;
	ptr->DIConditionStruct[1].lDeadBand				= 0;

	DWORD rgdwAxes[2];
	int nAxisCount = 0;
	if(lXCoefficient != 0)
	{
		rgdwAxes[nAxisCount] = DIJOFS_X;
		nAxisCount++;
	}

	if(lYCoefficient != 0)
	{
		rgdwAxes[nAxisCount] = DIJOFS_Y;
		nAxisCount++;
	}

	if(lXCoefficient == 0 && lYCoefficient == 0)
	{
		nAxisCount = 2;
		rgdwAxes[0] = DIJOFS_X;
		rgdwAxes[1] = DIJOFS_Y;
	}

	DWORD cbTypeSpecificParams;
	PVOID pvTypeSpecificParams;

	if (nAxisCount == 1) {
		cbTypeSpecificParams = sizeof(DICONDITION[1]);
		if (lXCoefficient)
			pvTypeSpecificParams = &ptr->DIConditionStruct[0];
		else
			pvTypeSpecificParams = &ptr->DIConditionStruct[1];

	} else {
		cbTypeSpecificParams = sizeof(DICONDITION[2]);
		pvTypeSpecificParams = &ptr->DIConditionStruct[0];
	}

	ptr->rglDirection[0] = 0;
	ptr->rglDirection[1] = 0;

	ptr->DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	ptr->DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_CARTESIAN;
	ptr->DIEffectStruct.dwDuration				= dwDuration;
	ptr->DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	ptr->DIEffectStruct.dwGain					= 10000;
	ptr->DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	ptr->DIEffectStruct.dwTriggerRepeatInterval	= 0;
	ptr->DIEffectStruct.cAxes					= nAxisCount;
	ptr->DIEffectStruct.rgdwAxes					= rgdwAxes;
	ptr->DIEffectStruct.rglDirection				= ptr->rglDirection;
	ptr->DIEffectStruct.lpEnvelope				= NULL;
	ptr->DIEffectStruct.cbTypeSpecificParams		= cbTypeSpecificParams;
	ptr->DIEffectStruct.lpvTypeSpecificParams	= pvTypeSpecificParams;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(guid, &ptr->DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateConditionEffect
// Purpose:		Creates a Condition type Effect with specified params
// Parameters:	LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwType					- SPRING | INERTIA | DAMPER | FRICTION
//				DWORD dwDuration				- Duration in uS
//				LONG lXCoefficient				- Coefficient in -10000 to 10000
//				LONG lXOffset					- Offset in -10000 to 10000
//				LONG lYCoefficient				- Coefficient in -10000 to 10000
//				LONG lYOffset					- Offset in -10000 to 10000
//				LONG lButton					- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
//			To create a 1D Friction, set the lFx or lFy parameter to 0
//			To create a 2D Friction, set both lFx and lFy parameter to non-zero
//				or set both lFx and lFy to zero
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateConditionEffect(	
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwType,
	IN DWORD dwDuration,
	IN LONG lXCoefficient,
	IN LONG lXOffset,
	IN LONG lYCoefficient,
	IN LONG lYOffset,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	// Always clear return IPtr
	*ppDIEffect = NULL;

	GUID guid;
	switch(dwType)
	{
		case SPRING:
			guid = GUID_Spring;
			break;
		case INERTIA:
			guid = GUID_Inertia;
			break;
		case DAMPER:
			guid = GUID_Damper;
			break;
		case FRICTION:
			guid = GUID_Friction;
			break;
		default:
			break;
	}

	DICONDITION DIConditionStruct[2];
	DIConditionStruct[0].lOffset				= lXOffset;
	DIConditionStruct[0].lPositiveCoefficient	= lXCoefficient;
	DIConditionStruct[0].lNegativeCoefficient	= lXCoefficient;
	DIConditionStruct[0].dwPositiveSaturation	= 10000;
	DIConditionStruct[0].dwNegativeSaturation	= 10000;
	DIConditionStruct[0].lDeadBand				= 0;
	DIConditionStruct[1].lOffset				= lYOffset;
	DIConditionStruct[1].lPositiveCoefficient	= lYCoefficient;
	DIConditionStruct[1].lNegativeCoefficient	= lYCoefficient;
	DIConditionStruct[1].dwPositiveSaturation	= 10000;
	DIConditionStruct[1].dwNegativeSaturation	= 10000;
	DIConditionStruct[1].lDeadBand				= 0;

	DWORD rgdwAxes[2];
	int nAxisCount = 0;
	if(lXCoefficient != 0)
	{
		rgdwAxes[nAxisCount] = DIJOFS_X;
		nAxisCount++;
	}
	if(lYCoefficient != 0)
	{
		rgdwAxes[nAxisCount] = DIJOFS_Y;
		nAxisCount++;
	}
	if(lXCoefficient == 0 && lYCoefficient == 0)
	{
		nAxisCount = 2;
		rgdwAxes[0] = DIJOFS_X;
		rgdwAxes[1] = DIJOFS_Y;
	}

	DWORD cbTypeSpecificParams;
	PVOID pvTypeSpecificParams;
	if(nAxisCount == 1)
	{
		cbTypeSpecificParams = sizeof(DICONDITION[1]);
		if(lXCoefficient != 0)
			pvTypeSpecificParams = &DIConditionStruct[0];
		else
			pvTypeSpecificParams = &DIConditionStruct[1];
	}
	else
	{
		cbTypeSpecificParams = sizeof(DICONDITION[2]);
		pvTypeSpecificParams = &DIConditionStruct[0];
	}

	LONG rglDirection[2];
	rglDirection[0] = 0;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_CARTESIAN;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= 10000;
	DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= nAxisCount;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= cbTypeSpecificParams;
	DIEffectStruct.lpvTypeSpecificParams	= pvTypeSpecificParams;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(guid, &DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateRampEffect
// Purpose:		Creates a Ramp type Effect with specified params
// Parameters:  LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration		- uS
//				DWORD dwDirection		- 0 to 35999
//				LONG lStart				- -10000 to 10000
//				LONG lEnd				- -10000 to 10000
//				DWORD dwAttackTime		- Envelope Attack Time in uS
//				DWORD dwAttackLevel		- Envelope Attack Level in 0 to 10000
//				DWORD dwFadeTime		- Envelope Fade time in uS
//				DWORD dwFadeLevel		- Envelope Fade Level
//				LONG lButton			- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateRampEffect(	
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN DWORD dwDirection,
	IN LONG lStart,
	IN LONG lEnd,
	IN DWORD dwAttackTime,
	IN DWORD dwAttackLevel,
	IN DWORD dwFadeTime,
	IN DWORD dwFadeLevel,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	DIRAMPFORCE DIRampStruct;
	DIRampStruct.lStart = lStart;
	DIRampStruct.lEnd = lEnd;

	DIENVELOPE DIEnvelopeStruct;
	DIEnvelopeStruct.dwSize = sizeof(DIENVELOPE);
	DIEnvelopeStruct.dwAttackTime = dwAttackTime;
	DIEnvelopeStruct.dwAttackLevel = dwAttackLevel;
	DIEnvelopeStruct.dwFadeTime = dwFadeTime;
	DIEnvelopeStruct.dwFadeLevel = dwFadeLevel;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= 10000;
	DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= &DIEnvelopeStruct;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(DIRampStruct);
	DIEffectStruct.lpvTypeSpecificParams	= &DIRampStruct;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_RampForce, &DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}

// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateConstantForceEffect
// Purpose:		Creates a ConstantForce type Effect with specified params
// Parameters:  LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration		- in uS
//				DWORD dwDirection		- in 0 to 35999
//				LONG lMagnitude			- in -10000 to 10000
//				DWORD dwAttackTime		- Envelope Attack Time in uS
//				DWORD dwAttackLevel		- Envelope Attack Level in 0 to 10000
//				DWORD dwFadeTime		- Envelope Fade time in uS
//				DWORD dwFadeLevel		- Envelope Fade Level
//				LONG lButton			- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
HRESULT SWFF_CreateConstantForceEffect(	
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN DWORD dwDirection,
	IN LONG lMagnitude,
	IN DWORD dwAttackTime,
	IN DWORD dwAttackLevel,
	IN DWORD dwFadeTime,
	IN DWORD dwFadeLevel,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;
	
	// Always clear return IPtr
	*ppDIEffect = NULL;

	DICONSTANTFORCE DIConstantForceStruct;
	DIConstantForceStruct.lMagnitude = lMagnitude;

	DIENVELOPE DIEnvelopeStruct;
	DIEnvelopeStruct.dwSize = sizeof(DIENVELOPE);
	DIEnvelopeStruct.dwAttackTime = dwAttackTime;
	DIEnvelopeStruct.dwAttackLevel = dwAttackLevel;
	DIEnvelopeStruct.dwFadeTime = dwFadeTime;
	DIEnvelopeStruct.dwFadeLevel = dwFadeLevel;

	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = dwDirection;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= 10000;
	DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= &DIEnvelopeStruct;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(DICONSTANTFORCE);
	DIEffectStruct.lpvTypeSpecificParams	= &DIConstantForceStruct;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_ConstantForce, &DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}


// ----------------------------------------------------------------------------
// Function: 	SWFF_CreateWallEffect
// Purpose:		Creates a Wall Effect
// Parameters:  LPDIRECTINPUTDEVICE2 pDIDevice	- IDIRECTINPUTDEVICE2 interface
//				LPDIRECTINPUTEFFECT* ppDIEffect	- Receives pointer to created effect
//				DWORD dwDuration		- in uS
//				DWORD dwDirection		- 0 | 9000 | 18000 | 27000
//				DWORD dwDistance		- Distance from centerin 0 to 10000
//				BOOL bInner				- T/F = Inner/Outer
//				LONG lCoefficient		- Wall Constant in 0 to 10000
//				LONG lButton			- Index of playback button, -1 for none
// Returns:
// Algorithm:	
// Comments:
// 
// ----------------------------------------------------------------------------
#if 0
HRESULT SWFF_CreateWallEffect(	
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,
	IN DWORD dwDirection,
	IN DWORD dwDistance,
	IN BOOL bInner,
	IN LONG lWallCoefficient,
	IN LONG lButton)
{
	if(pDIDevice == NULL || ppDIEffect == NULL)
		return SFERR_INVALID_PARAM;

	// Always clear return IPtr
	*ppDIEffect = NULL;

	BE_WALL_PARAM WallStruct;
	WallStruct.m_Bytes = sizeof(WallStruct);
	WallStruct.m_WallType = bInner ? WALL_INNER : WALL_OUTER;
	WallStruct.m_WallConstant = lWallCoefficient;
	WallStruct.m_WallAngle = dwDirection;
	WallStruct.m_WallDistance = dwDistance;


	DWORD rgdwAxes[2];
	rgdwAxes[0] = DIJOFS_X;
	rgdwAxes[1] = DIJOFS_Y;

	LONG rglDirection[2];
	rglDirection[0] = 0;
	rglDirection[1] = 0;

	DIEFFECT DIEffectStruct;
	DIEffectStruct.dwSize					= sizeof(DIEFFECT);
	DIEffectStruct.dwFlags					= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	DIEffectStruct.dwDuration				= dwDuration;
	DIEffectStruct.dwSamplePeriod			= HZ_TO_uS(100);
	DIEffectStruct.dwGain					= 10000;
	DIEffectStruct.dwTriggerButton			= lButton == -1 ? DIEB_NOTRIGGER : FIELD_OFFSET(DIJOYSTATE, rgbButtons) + lButton;
	DIEffectStruct.dwTriggerRepeatInterval	= 0;
	DIEffectStruct.cAxes					= 2;
	DIEffectStruct.rgdwAxes					= rgdwAxes;
	DIEffectStruct.rglDirection				= rglDirection;
	DIEffectStruct.lpEnvelope				= NULL;
	DIEffectStruct.cbTypeSpecificParams		= sizeof(WallStruct);
	DIEffectStruct.lpvTypeSpecificParams	= &WallStruct;

	HRESULT hResult;
	hResult = pDIDevice->CreateEffect(GUID_Wall, &DIEffectStruct, ppDIEffect, NULL);

	return hResult;
}
#endif


// ----------------------------------------------------------------------------
// FUNCTION:	SWFF_GetJoyData
// PURPOSE:     Retrieves Joystick data
// PARAMETERS:	int nJoyID		- JOYSTICKID1-16
// 				JOYINFOEX *pjix - PTR to a JOYYINFOEX structure
// 				char *pszErr	- Ptr to Error code string
// RETURNS:		JOYINFOEX filled in
//
//				The axis and buttons info:
//				All axis are in the range 0 to 65535.
//					jix.dwXpos - X position.
//					jix.dwYpos - Y position.
//					jix.dwZpos - Throttle slider control
//					jix.dwRpos - Z Rotation position.
//
//				To see if button 1 is pressed: 
//				   (jix.dwButtons & JOY_BUTTON1) ? PRESSED : NOT_PRESSED;
//				likewise for the other buttons JOY_BUTTON2, JOY_BUTTON3 … 
//				JOY_BUTTON8
//
//				Hat Switch (POV) is in jix.dwPOV
//				   The range is 0 to 35900 and the value is -1 if the 
//				Hat Switch is not pressed.
//
//				TRUE if successful, else FALSE
// COMMENTS:    
// ----------------------------------------------------------------------------
BOOL SWFF_GetJoyData(int nJoyID, JOYINFOEX * pjix, char *pszErr)
{
	if(pjix == NULL || pszErr == NULL)
		return FALSE;

	memset(pjix, 0x00, sizeof(JOYINFOEX));	// for good measure
	pjix->dwSize = sizeof(JOYINFOEX);

// NOTE:  With SideWinder Digital OverDrive, it takes no more time to return all 
// information from the joystick than it does to just get 
// the button states or axis.
//
	pjix->dwFlags = JOY_RETURNALL;


// joyGetPoxEx will fill in the joyinfoex struct with all the
// joystick information 
//
 
	switch(joyGetPosEx(nJoyID, pjix))
	{
		case JOYERR_NOERROR: // no problem
			strcpy(pszErr,"SUCCESS");
			break;
		 
		case MMSYSERR_NODRIVER:
			strcpy(pszErr,"The joystick driver is not present.");
			return FALSE;
		 
		case MMSYSERR_INVALPARAM:
			strcpy(pszErr,"An invalid parameter was passed.");
			return FALSE;
		 
		case MMSYSERR_BADDEVICEID:        
			strcpy(pszErr,"The specified joystick identifier is invalid.");
			return FALSE;
		 
		case JOYERR_UNPLUGGED:        
			strcpy(pszErr,"Your joystick is unplugged.");
			return FALSE;
		 
		default:
			strcpy(pszErr,"Unknown joystick error.");
			return FALSE;
	 
	 }   // end of switch
  	return TRUE; 
 } // GetJoyData()


// ----------------------------------------------------------------------------
// FUNCTION:	SWFF_GetJoyData2
// PURPOSE:     Retrieves Joystick data using DInput calls  
// PARAMETERS:	LPDIRECTINPUTDEVICE2 pDIDevice	- Pointer to DirectInputDevice
//   			LPDIJOYSTATE pjs				- PTR to a DIJOYSTATE structure
// RETURNS:		DIJOYSTATE filled in
//
//				The axis info:
//					all axes have range from 0 to 65535
//					pjs->lX - X position
//					pjs->lY - Y position
//					pjs->lZ - Throttle position
//
//				To see if button 0 is pressed: 
//				   (pjs->rgbButtons[0] & 0x80) ? PRESSED : NOT_PRESSED;
//					likewise for the other buttons pjs->rgbButtons[1],
//					pjs->rgbButtons[2]
//
//				Hat Switch (POV) is in pjs->rgdwPov[0]
//				   The range is 0 to 35999 and the value is -1 if the 
//					Hat Switch is not pressed.
//
// COMMENTS:    
// ----------------------------------------------------------------------------
/*HRESULT SWFF_GetJoyData2(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIJOYSTATE pjs)
{
	HRESULT hResult;

	if(pDIDevice == NULL || pjs == NULL)
		return SFERR_INVALID_PARAM;

	memset(pjs, 0x00, sizeof(DIJOYSTATE));	// for good measure

	// NOTE:  With SideWinder Digital OverDrive, it takes no more time to return all 
	// information from the joystick than it does to just get 
	// the button states or axis.
	//
	
	// must poll before using GetDeviceState(...)
	hResult = pDIDevice->Poll();
	if(FAILED(hResult))
		return hResult;

	// retrieve the values cached during Poll()
	hResult = pDIDevice->GetDeviceState(sizeof(DIJOYSTATE), pjs);

	return hResult;
 } // GetJoyData2()
*/

// ----------------------------------------------------------------------------
// Function: 	SWFF_ErrorCodeToString
// Parameters:	HRESULT hResult		- Error Code
//				TCHAR * pszString	- Ptr to string to fill with code
// Returns:
// Algorithm:
// Comments:
// 
// ----------------------------------------------------------------------------
void SWFF_ErrorCodeToString(HRESULT hResult, TCHAR * pszCodeString)
{       	
	if(pszCodeString == NULL)
		return;

//XSTR:OFF

	switch(hResult)
	{
		case S_FALSE: strcpy(pszCodeString, "S_FALSE"); break;
		case DI_POLLEDDEVICE: strcpy(pszCodeString, "DI_POLLEDDEVICE"); break;
//		case DI_DOWNLOADSKIPPED: strcpy(pszCodeString, "DI_DOWNLOADSKIPPED"); break;
//		case DI_EFFECTRESTARTED: strcpy(pszCodeString, "DI_EFFECTRESTARTED"); break;
		case DIERR_OLDDIRECTINPUTVERSION: strcpy(pszCodeString, "DIERR_OLDDIRECTINPUTVERSION" ); break;
		case DIERR_BETADIRECTINPUTVERSION: strcpy(pszCodeString, "DIERR_BETADIRECTINPUTVERSION" ); break;
		case DIERR_BADDRIVERVER: strcpy(pszCodeString, "DIERR_BADDRIVERVER" ); break;
		case DIERR_DEVICENOTREG: strcpy(pszCodeString, "DIERR_DEVICENOTREG" ); break;
		case DIERR_NOTFOUND: strcpy(pszCodeString, "DIERR_NOTFOUND" ); break;
		case DIERR_INVALIDPARAM: strcpy(pszCodeString, "DIERR_INVALIDPARAM" ); break;
		case DIERR_NOINTERFACE: strcpy(pszCodeString, "DIERR_NOINTERFACE" ); break;
		case DIERR_GENERIC: strcpy(pszCodeString, "DIERR_GENERIC" ); break;
		case DIERR_OUTOFMEMORY: strcpy(pszCodeString, "DIERR_OUTOFMEMORY" ); break;
		case DIERR_UNSUPPORTED: strcpy(pszCodeString, "DIERR_UNSUPPORTED" ); break;
		case DIERR_NOTINITIALIZED: strcpy(pszCodeString, "DIERR_NOTINITIALIZED" ); break;
		case DIERR_ALREADYINITIALIZED: strcpy(pszCodeString, "DIERR_ALREADYINITIALIZED" ); break;
		case DIERR_NOAGGREGATION: strcpy(pszCodeString, "DIERR_NOAGGREGATION" ); break;
		case DIERR_INPUTLOST: strcpy(pszCodeString, "DIERR_INPUTLOST" ); break;
		case DIERR_ACQUIRED: strcpy(pszCodeString, "DIERR_ACQUIRED" ); break;
		case DIERR_NOTACQUIRED: strcpy(pszCodeString, "DIERR_NOTACQUIRED" ); break;
		case E_ACCESSDENIED: strcpy(pszCodeString, "E_ACCESSDENIED: DIERR_OTHERAPPHASPRIO, DIERR_READONLY, DIERR_HANDLEEXISTS"); break;
		case E_PENDING: strcpy(pszCodeString, "E_PENDING" ); break;
		case DIERR_INSUFFICIENTPRIVS: strcpy(pszCodeString, "DIERR_INSUFFICIENTPRIVS" ); break;
		case DIERR_DEVICEFULL: strcpy(pszCodeString, "DIERR_DEVICEFULL" ); break;
		case DIERR_MOREDATA: strcpy(pszCodeString, "DIERR_MOREDATA" ); break;
		case DIERR_NOTDOWNLOADED: strcpy(pszCodeString, "DIERR_NOTDOWNLOADED" ); break;
		case DIERR_HASEFFECTS: strcpy(pszCodeString, "DIERR_HASEFFECTS" ); break;
		case DIERR_NOTEXCLUSIVEACQUIRED: strcpy(pszCodeString, "DIERR_NOTEXCLUSIVEACQUIRED"); break;
		case DIERR_INCOMPLETEEFFECT: strcpy(pszCodeString, "DIERR_INCOMPLETEEFFECT" ); break;
		case DIERR_NOTBUFFERED: strcpy(pszCodeString, "DIERR_NOTBUFFERED" ); break;
		case DIERR_EFFECTPLAYING: strcpy(pszCodeString, "DIERR_EFFECTPLAYING"); break;
		case SFERR_INVALID_OBJECT: strcpy(pszCodeString, "SFERR_INVALID_OBJECT" ); break;
		case SFERR_END_OF_LIST: strcpy(pszCodeString, "SFERR_END_OF_LIST" ); break;
		case SFERR_DEVICE_NACK: strcpy(pszCodeString, "SFERR_DEVICE_NACK" ); break;
		case SFERR_RAW_OUT_DATAEVENT_CREATION: strcpy(pszCodeString, "SFERR_RAW_OUT_DATAEVENT_CREATION" ); break;
		case SFERR_RAW_OUT_THREAD_CREATION: strcpy(pszCodeString, "SFERR_RAW_OUT_THREAD_CREATION" ); break;
		case SFERR_SYSTEM_INIT: strcpy(pszCodeString, "SFERR_SYSTEM_INIT" ); break;
		case SFERR_DRIVER_ERROR: strcpy(pszCodeString, "SFERR_DRIVER_ERROR" ); break;
		case SFERR_NON_FF_DEVICE: strcpy(pszCodeString, "SFERR_NON_FF_DEVICE" ); break;
		case SFERR_INVALID_HAL_OBJECT: strcpy(pszCodeString, "SFERR_INVALID_HAL_OBJECT" ); break;
//		case VFX_ERR_FILE_NOT_FOUND: strcpy(pszCodeString, "VFX_ERR_FILE_NOT_FOUND" ); break;
//		case VFX_ERR_FILE_CANNOT_OPEN: strcpy(pszCodeString, "VFX_ERR_FILE_CANNOT_OPEN" ); break;
//		case VFX_ERR_FILE_CANNOT_CLOSE: strcpy(pszCodeString, "VFX_ERR_FILE_CANNOT_CLOSE" ); break;
//		case VFX_ERR_FILE_CANNOT_READ: strcpy(pszCodeString, "VFX_ERR_FILE_CANNOT_READ" ); break;
//		case VFX_ERR_FILE_CANNOT_WRITE: strcpy(pszCodeString, "VFX_ERR_FILE_CANNOT_WRITE" ); break;
//		case VFX_ERR_FILE_CANNOT_SEEK: strcpy(pszCodeString, "VFX_ERR_FILE_CANNOT_SEEK" ); break;
		case VFX_ERR_FILE_UNKNOWN_ERROR: strcpy(pszCodeString, "VFX_ERR_FILE_UNKNOWN_ERROR" ); break;
		case VFX_ERR_FILE_BAD_FORMAT: strcpy(pszCodeString, "VFX_ERR_FILE_BAD_FORMAT" ); break;
//		case VFX_ERR_FILE_ACCESS_DENIED: strcpy(pszCodeString, "VFX_ERR_FILE_ACCESS_DENIED" ); break;
//		case VFX_ERR_FILE_SHARING_VIOLATION: strcpy(pszCodeString, "VFX_ERR_FILE_SHARING_VIOLATION" ); break;
//		case VFX_ERR_FILE_NETWORK_ERROR: strcpy(pszCodeString, "VFX_ERR_FILE_NETWORK_ERROR" ); break;
//		case VFX_ERR_FILE_TOO_MANY_OPEN_FILES: strcpy(pszCodeString, "VFX_ERR_FILE_TOO_MANY_OPEN_FILES" ); break;
//		case VFX_ERR_FILE_INVALID: strcpy(pszCodeString, "VFX_ERR_FILE_INVALID" ); break;
		case VFX_ERR_FILE_END_OF_FILE: strcpy(pszCodeString, "VFX_ERR_FILE_END_OF_FILE" ); break;
		case SWDEV_ERR_INVALID_ID : strcpy(pszCodeString, "SWDEV_ERR_INVALID_ID" ); break;
		case SWDEV_ERR_INVALID_PARAM : strcpy(pszCodeString, "SWDEV_ERR_INVALID_PARAM" ); break;
		case SWDEV_ERR_CHECKSUM : strcpy(pszCodeString, "SWDEV_ERR_CHECKSUM" ); break;
		case SWDEV_ERR_TYPE_FULL : strcpy(pszCodeString, "SWDEV_ERR_TYPE_FULL" ); break;
		case SWDEV_ERR_UNKNOWN_CMD : strcpy(pszCodeString, "SWDEV_ERR_UNKNOWN_CMD" ); break;
		case SWDEV_ERR_PLAYLIST_FULL : strcpy(pszCodeString, "SWDEV_ERR_PLAYLIST_FULL" ); break;
		case SWDEV_ERR_PROCESSLIST_FULL : strcpy(pszCodeString, "SWDEV_ERR_PROCESSLIST_FULL" ); break;
		default: sprintf(pszCodeString, "%x", hResult); break;
	}

//XSTR:ON

} 
