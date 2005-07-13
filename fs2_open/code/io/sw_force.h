/****************************************************************************

    MODULE:     	SW_Force.H
	Tab settings: 	5 9

	Copyright 1995, 1996, Microsoft Corporation, 	All Rights Reserved.

	You have a royalty-free right to use, modify, reproduce and 
	distribute this header Files (and/or any modified version) in 
	any way you find useful, provided that you agree that 
	Microsoft has no warranty obligations or liability for any 
	Application Files which are created using the header Files. 

    PURPOSE:    	Header for SideWinder Force Feedback Joystick
    				and interface to DirectInput Force Feedback API
    

	Author(s):	Name:
	----------	----------------


	Revision History:
	-----------------
	Version Date            Author  Comments
   	1.0  	24-Mar-97       MEA     original
        
****************************************************************************/

#ifndef _SW_Force_SEEN
#define _SW_Force_SEEN
#include <winerror.h>
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#define INC_OLE2
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>
#undef INITGUIDS

#include "sw_guid.hpp"
#include "directx/vdinput.h"
#include "sw_error.hpp"

/*
#include "sw_guid.h"
#include "directx/vdinput.h"
#include "sw_error.h"
*/


//
// --- Defines and macros for making DirectInput FF a little easier to work 
//     with.
//
#define SINE			1
#define COSINE			2
#define SQUARE_HIGH		3
#define SQUARE_LOW		4
#define TRIANGLE_UP		5
#define TRIANGLE_DOWN	6
#define SAWTOOTH_UP		7
#define SAWTOOTH_DOWN	8
#define RAMP_UP			9
#define RAMP_DOWN		10
#define SPRING			11
#define INERTIA			12
#define DAMPER			13
#define FRICTION		14
#define WALL			15

#define HZ_TO_uS(HZ) ((int)(1000000.0/(double)(HZ) + 0.5))
#define uS_TO_HZ(uS) (max(1,(int)((double)(uS)/1000000.0 + 0.5)))

#ifndef X_AXIS
#define X_AXIS	1
#endif

#ifndef Y_AXIS
#define Y_AXIS	2
#endif

//---------------------------------------------------------------------------
// Function prototype declarations C-callable
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

HRESULT SWFF_OpenDefaultFFJoystick(
	IN HWND hWnd,
	OUT LPDIRECTINPUT* ppDI, 
	OUT LPDIRECTINPUTDEVICE2* ppDIDevice2);

HRESULT SWFF_OpenDefaultFFJoystickEx(
	IN HWND hWnd,
	IN HINSTANCE hInstance,
	OUT LPDIRECTINPUT* ppDI, 
	OUT LPDIRECTINPUTDEVICE2* ppDIDevice,
	DWORD dwFlags);

HRESULT SWFF_DestroyEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN LPDIRECTINPUTEFFECT pDIEffect);

HRESULT SWFF_DestroyAllEffects(
	IN LPDIRECTINPUTDEVICE2 pDIDevice);

HRESULT SWFF_SetGain(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwGain);

HRESULT SWFF_SetDirection(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwAngle);

HRESULT SWFF_SetDuration(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwDuration);

HRESULT SWFF_SetDirectionGain(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN DWORD dwAngle,
	IN DWORD dwMag);

HRESULT SWFF_PutRawForce(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwDirection);

HRESULT SWFF_PutRawAxisForce(
	IN LPDIRECTINPUTEFFECT pDIEffect,
	IN LONG lMagnitude);

HRESULT SWFF_CreateRawForceEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwDirection);

HRESULT SWFF_CreateRawAxisForceEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN LONG lMagnitude,
	IN DWORD dwAxis);

HRESULT SWFF_CreateROMEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT * ppDIEffect,
	IN REFGUID refGUID,
	IN DWORD dwDuration,		
	IN DWORD dwGain,
	IN DWORD dwDirection,		
	IN LONG lButton);

HRESULT SWFF_CreatePeriodicEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
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
	IN LONG lButton);			

HRESULT SWFF_CreateSpringEffect(	
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,		
	IN LONG lKx,				
	IN LONG lCenterX,			
	IN LONG lKy,				
	IN LONG lCenterY,			
	IN LONG lButton);
	
HRESULT SWFF_CreateDamperEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,		
	IN LONG lBx,				
	IN LONG lV0x,				
	IN LONG lBy,				
	IN LONG lV0y,				
	IN LONG lButton);

HRESULT SWFF_CreateInertiaEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,		
	IN LONG lMx,				
	IN LONG lA0x,				
	IN LONG lMy,				
	IN LONG lA0y,				
	IN LONG lButton);

HRESULT SWFF_CreateFrictionEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,		
	IN LONG lFx,				
	IN LONG lFy,				
	IN LONG lButton);

HRESULT SWFF_CreateConditionEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwType,
	IN DWORD dwDuration,		
	IN LONG lXCoefficient,		
	IN LONG lXOffset,			
	IN LONG lYCoefficient,		
	IN LONG lYOffset,			
	IN LONG lButton);			

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
	IN LONG lButtonMask);

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
	IN LONG lButton);

HRESULT SWFF_CreateWallEffect(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN DWORD dwDuration,		
	IN DWORD dwDirection,		
	IN DWORD dwDistance,		
	IN BOOL bInner,				
	IN LONG lWallCoefficient,	
	IN LONG lButton);			

HRESULT SWFF_CreateVFXEffectFromFile(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName);

HRESULT SWFF_CreateVFXEffectFromFileEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName,
	IN DWORD dwDuration,
	IN DWORD dwGain,
	IN DWORD dwDirection);

HRESULT SWFF_CreateVFXEffectFromBuffer(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize);

HRESULT SWFF_CreateVFXEffectFromBufferEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize,
	IN DWORD dwDuration,
	IN DWORD dwGain,
	IN DWORD dwDirection);

HRESULT SWFF_CreateDIEffectFromFile(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const TCHAR *pszFileName);

HRESULT SWFF_CreateDIEffectFromFileEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT** pppDIEffect,
	IN OUT PDWORD pdwEffectCount,
	IN const TCHAR *pszFileName,
	IN OUT void** ppUDBuffer,
	IN OUT PDWORD pdwOutFlags);

HRESULT SWFF_CreateDIEffectFromBuffer(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT* ppDIEffect,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize);

HRESULT SWFF_CreateDIEffectFromBufferEx(
	IN LPDIRECTINPUTDEVICE2 pDIDevice, 
	IN OUT LPDIRECTINPUTEFFECT** pppDIEffect,
	IN OUT PDWORD pdwEffectCount,
	IN const LPVOID pBuffer,
	IN DWORD dwBufferSize,
	IN OUT void** ppUDBuffer,
	IN OUT PDWORD pdwOutFlags);

BOOL SWFF_RegisterVFXObject(LPCTSTR pszVFXPath);

BOOL SWFF_GetJoyData(
	IN int nJoyID, 
	IN OUT JOYINFOEX * pjix, 
	OUT char *pszErr);
/*
HRESULT SWFF_GetJoyData2(
	IN LPDIRECTINPUTDEVICE2 pDIDevice,
	IN OUT LPDIJOYSTATE pjs);
*/
void SWFF_ErrorCodeToString(
	IN HRESULT hResult, 
	OUT TCHAR * pszCodeString);


#ifdef __cplusplus
}
#endif

//
// --- IVFX Interface prototypes
//
#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif  //PPVOID

typedef struct IVFX  			*PVFX;
typedef struct IVFX				**PPVFX;

#define VFXCE_CREATE_SINGLE		0x00001
#define VFXCE_CREATE_MULTIPLE	0x00002
#define VFXCE_CALC_BUFFER_SIZE	0x00004
#define VFXCE_CALC_EFFECT_COUNT	0x00008
#define VFXCE_CONCATENATE		0x00010
#define VFXCE_SUPERIMPOSE		0x00020


#undef INTERFACE
#define INTERFACE IVFX
DECLARE_INTERFACE_(IVFX, IUnknown)
{
    //IUnknown members
	STDMETHOD(QueryInterface) (THIS_ REFIID, PPVOID) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    //IVFX Interface members
	// General Methods
 	STDMETHOD_(HRESULT, CreateEffectFromFile) (
		THIS_ 	
		LPDIRECTINPUTDEVICE2,
		LPDIRECTINPUTEFFECT*,
		LPDWORD,
		const TCHAR*,
		LPVOID,
		LPDWORD,
		DWORD,
		LPDWORD) PURE;

	STDMETHOD_(HRESULT, CreateEffectFromBuffer) (
		THIS_ 
		LPDIRECTINPUTDEVICE2,
		LPDIRECTINPUTEFFECT*,
		LPDWORD,
		PVOID,
		DWORD,
		LPVOID,
		LPDWORD,
		DWORD,
		LPDWORD) PURE;
};

//
// --- SideWinder specific 
//
#define DEFAULT_ROM_EFFECT_GAIN		100		// Set dwGain to this for Default
											// ROM Effect gain
#define DEFAULT_ROM_EFFECT_DURATION	1000	// Set dwDuration to this for Default
											// ROM Effect Duration
#define DEFAULT_ROM_EFFECT_OUTPUTRATE	1000	// Set dwSampleRate to this for 
												// Default ROM Effect output rate

#define MIN_ANGLE			0
#define MAX_ANGLE			36000
#define MIN_FORCEOUTPUTRATE 1
#define MIN_GAIN			1
#define MAX_GAIN			10000
#define MAX_FORCE			10000
#define MIN_FORCE			-10000
#define MIN_TIME_PERIOD		1
#define MAX_TIME_PERIOD		4294967296L	// 4096 * 10^^6 usecs

#define SCALE_GAIN			100		// DX is +/- 10000
#define SCALE_TIME			1000	// DX is in microseconds
#define	SCALE_POSITION		100		// DX is +/- 10000
#define	SCALE_CONSTANTS		100		// DX is +/- 10000
#define SCALE_DIRECTION		100		// DX is 0 to 35900

// 
// --- Default Values
//
#define	DEFAULT_OFFSET			0
#define DEFAULT_ATTACK_LEVEL	0
#define DEFAULT_ATTACK_TIME		0
#define DEFAULT_SUSTAIN_LEVEL	10000
#define DEFAULT_FADE_LEVEL		0
#define DEFAULT_FADE_TIME		0


//
// The following are Type Specific parameters structures for SideWinder 
//

//
// --- WALL Effect
//
#define WALL_INNER			0	// Wall material:from center to Wall Distance
#define WALL_OUTER			1	// Wall material:greater than Wall Distance

typedef struct _BE_WALL_PARAM {
	ULONG	m_Bytes;			// Size of this structure
	ULONG 	m_WallType;			// WALL_INNER or WALL_OUTER
	LONG	m_WallConstant;		// in +/- 10000%
	ULONG	m_WallAngle;		// 0, 9000, 18000, 27000
	ULONG	m_WallDistance;		// Distance from Wall face normal to center. 0 to 10000
} BE_WALL_PARAM, *PBE_WALL_PARAM;

//
// ---	EF_VFX_EFFECT = { FRC file effects }
//
// Subtypes:  none

#define VFX_FILENAME	0L
#define VFX_BUFFER		1L

#define DEFAULT_VFX_EFFECT_GAIN			10000	// set dwGain to this for default gain
#define DEFAULT_VFX_EFFECT_DIRECTION	0		// set polar direction to this for default direction
#define DEFAULT_VFX_EFFECT_DURATION		1000	// set dwDuration to this for default duration

typedef struct _VFX_PARAM
{
	ULONG	m_Bytes;				// Size of this structure
	ULONG	m_PointerType;			// VFX_FILENAME or VFX_BUFFER
	ULONG	m_BufferSize;			// number of bytes in buffer (if VFX_BUFFER)
	PVOID	m_pFileNameOrBuffer;	// file name to open
} VFX_PARAM, *PVFX_PARAM;

typedef struct {
	DIEFFECT DIEffectStruct;
	DICONDITION DIConditionStruct[2];
	LONG rglDirection[2];
} di_condition_effect_struct;

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
	IN LONG lButton);

#endif // of ifdef _SW_Force_SEEN
