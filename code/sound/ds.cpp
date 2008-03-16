/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/ds.cpp $
 * $Revision: 2.46.2.11 $
 * $Date: 2007-10-29 15:32:19 $
 * $Author: taylor $
 *
 * C file for interface to DirectSound
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.46.2.10  2007/02/11 09:56:25  taylor
 * support for new finding/loading of sound files
 * add support for automatically figuring out samples-per-measure based on Goober's explanation in the Wiki (not sure if it's actually right though)
 * remove NO_SOUND
 *
 * Revision 2.46.2.9  2006/12/26 05:31:22  taylor
 * fix some sound state stuff that was leading to crashes in OS X and undefined behavior in Windows (this isn't all that pretty, but will handled properly in post-3.6.9 revised code)
 *
 * Revision 2.46.2.8  2006/12/07 18:24:43  taylor
 * various sound fixes/changes, mainly for OS X fixage
 *
 * Revision 2.46.2.7  2006/10/06 04:58:25  wmcoolmon
 * Fix OGG bug where sound files were always allocated 16 seconds of wavedata
 *
 * Revision 2.46.2.6  2006/08/19 04:31:24  taylor
 * cleanup
 * bugfixes
 * error-handling
 * (lots of crap got fixed to work better, and lets leave it at that, the commit log would be too long otherwise :))
 *
 * Revision 2.46.2.5  2006/07/08 18:07:31  taylor
 * require OpenAL 1.1 under Windows, I think it's causing problems for us to mix users with 1.0 and builds for 1.1
 *   (having 1.0 is fatal to sound, but the game should still work, a corresponding Launcher change goes with this too)
 * clean up some of the error messages to not say "DirectSound" is doing this or that, just say "Audio" instead
 *
 * Revision 2.46.2.4  2006/07/06 21:56:18  taylor
 * some better handling of OpenAL errors during init
 *
 * Revision 2.46.2.3  2006/06/22 14:59:45  taylor
 * fix various things that Valgrind has been complaining about
 *
 * Revision 2.46.2.2  2006/06/18 16:52:04  taylor
 * make sure we can report both AL and ALC errors
 * fix for ds_get_free_channel(), it shouldn't return -1 on an AL error
 *
 * Revision 2.46.2.1  2006/06/12 03:40:26  taylor
 * sync up current OpenAL changes
 *  - "SoundDeviceOAL" reg option for user specified sound device (used instead of "Soundcard" for OpenAL)
 *  - reset current context when we are leaving, may be leaving drivers in a bad state, and it hasn't hung up in quite a while
 *  - if sound card (which DS or OAL) is set to "no sound" then be sure to disable both sound and music
 *
 * Revision 2.46  2006/06/01 07:33:00  taylor
 * make sure to use software device rather than hardware since we don't currently handle source rotation/reuse properly between ds and audiostr code
 *
 * Revision 2.45  2006/06/01 04:46:18  taylor
 * put back listener setup for each sound, doesn't make a difference under Linux but it may be hurting Windows for some reason
 *
 * Revision 2.44  2006/05/31 04:01:13  taylor
 * bah!  this thing is crashing.  disable for now until I can figure out what's going on
 *
 * Revision 2.43  2006/05/31 03:27:19  Goober5000
 * whoops, didn't mean to commit that
 *
 * Revision 2.41  2006/05/27 16:39:40  taylor
 * remove non-Windows timeSetEvent() functions
 * make PDWORD and LPDWORD int instead of long (64-bit issue) (thanks Spike)
 * fix 64-bit ptr issue with OpenAL audio streaming code (thanks Spike)
 * add start of device enumberation for OpenAL 1.1 (it just lists available devices now)
 * make sure we open the default OpenAL 1.1 device under Windows
 * comment out blank listener stuff, this may have been crashing under OS X
 * add support for AL_BYTE_OFFSET in OpenAL 1.1 for buffer positioning under Windows
 *
 * Revision 2.40  2006/04/20 06:32:30  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.39  2006/04/14 18:39:38  taylor
 * something that Valgrind complained about
 *
 * Revision 2.38  2006/03/15 17:30:47  taylor
 * remove alut headers, since we don't use any alut functions anyway (didn't I already do this a couple of months ago??)
 *
 * Revision 2.37  2006/01/30 07:14:32  taylor
 * make sure to init dest and dest_size for any calling functions that don't
 *
 * Revision 2.36  2005/12/06 03:17:48  taylor
 * cleanup some debug log messages:
 *   note that a nprintf() with "Warning" or "General" is basically the same thing as mprintf()
 *   make sure that OpenAL init failures always get to the debug log
 *
 * Revision 2.35  2005/11/14 05:12:01  taylor
 * now able to build with OpenAL 1.0 and 1.1 thanks to newer OpenAL CVS changes (patch from icculus.org FS2 mailing list)
 *
 * Revision 2.34  2005/10/17 02:09:29  wmcoolmon
 * Fix redmenace's compile problem
 *
 * Revision 2.33  2005/10/17 00:02:46  wmcoolmon
 * Change this back for MSVC.NET?
 *
 * Revision 2.32  2005/09/20 02:48:37  taylor
 * fix a couple of things that Valgrind complained about
 *
 * Revision 2.31  2005/07/31 01:35:43  taylor
 * we only care about fmt and data tags so once we have them bail out, fixes some really bad seek errors in crappy wav files
 *
 * Revision 2.30  2005/06/19 02:45:55  taylor
 * OGG streaming fixes to get data reading right and avoid skipping
 * properly handle seeking in OGG streams
 * compiler warning fix in OpenAL builds
 *
 * Revision 2.29  2005/06/02 01:56:32  wmcoolmon
 * Cast to quiet error
 *
 * Revision 2.28  2005/06/01 09:41:14  taylor
 * bit of cleanup for audiostr-openal and fix a Windows-only enum error
 * bunch of OGG related fixes for Linux and Windows (DirectSound and OpenAL), fixes audio related TBP 3.2 crashes
 * gracefully handle OGG logical bitstream changes, shouldn't even load if there is more than 1
 *
 * Revision 2.27  2005/05/26 04:32:31  taylor
 * the Loki extensions are Linux only so don't even bother checking under Windows, the
 *   new spec has a cross-platform way of getting the same thing so this isn't a big deal
 *
 * Revision 2.26  2005/05/12 17:47:57  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 * fix a few streaming errors in OpenAL code (Jens Granseuer)
 * temporary change to help deal with missing music in OpenAL Windows builds
 * don't assert when si->data is NULL unless we really need to check (OpenAL only)
 *
 * Revision 2.25  2005/04/28 05:12:26  wmcoolmon
 * Cleared up some ambiguity that made MSVC 2003 go on strike
 *
 * Revision 2.24  2005/04/24 12:45:14  taylor
 * possible dev/0 bug fix (Jens Granseuer)
 *
 * Revision 2.23  2005/04/18 03:30:05  taylor
 * more accurate play position for OpenAL when AL_BYTE_LOKI is used (Jens Granseuer)
 *
 * Revision 2.22  2005/04/13 22:31:53  Goober5000
 * fixed the compile errors (thanks Phreak)
 * --Goober5000
 *
 * Revision 2.21  2005/04/05 11:48:22  taylor
 * remove acm-unix.cpp, replaced by acm-openal.cpp since it's properly cross-platform now
 * better error handling for OpenAL functions
 * Windows can now build properly with OpenAL
 * extra check to make sure we don't try and use too many hardware bases sources
 * fix memory error from OpenAL extension list in certain instances
 *
 * Revision 2.20  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.19  2005/04/01 07:33:08  taylor
 * fix hanging on exit with OpenAL
 * some better error handling on OpenAL init and make it more Windows friendly too
 * basic 3d sound stuff for OpenAL, not working right yet
 *
 * Revision 2.18  2005/03/28 00:40:09  taylor
 * fix to snd_time_remaining() to make sure we are getting the correct index into Sounds[]
 *
 * Revision 2.17  2005/03/27 06:16:46  taylor
 * don't use a platform specific ifdef since the OpenAL code may work under Windows now too
 *
 * Revision 2.16  2005/03/24 23:27:25  taylor
 * make sounds.tbl dynamic
 * have snd_time_remaining() be less stupid
 * some OpenAL error fixerage
 * be able to turn off some typically useless debug messages
 *
 * Revision 2.15  2005/02/02 10:36:23  taylor
 * merge with Linux/OSX tree - p0202
 *
 * Revision 2.14  2005/01/18 01:14:17  wmcoolmon
 * OGG fixes, ship selection fixes
 *
 * Revision 2.13  2005/01/08 09:59:10  wmcoolmon
 * Sound quality in FreeSpace 2 is now controlled by SoundSampleBits, and SoundSampleRate. Also, some sounds will use hardware rather than software buffers if available.
 *
 * Revision 2.12  2004/12/25 17:45:38  wmcoolmon
 * Quick update to tempfix possible bug.
 *
 * Revision 2.11  2004/12/25 00:23:46  wmcoolmon
 * Ogg support for WIN32
 *
 * Revision 2.10  2004/09/05 19:23:24  Goober5000
 * fixed a few warnings
 * --Goober5000
 *
 * Revision 2.9  2004/08/23 07:48:08  Goober5000
 * fix0red some warnings
 * --Goober5000
 *
 * Revision 2.8  2004/07/26 20:47:52  Kazan
 * remove MCD complete
 *
 * Revision 2.7  2004/07/12 16:33:06  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.6  2004/06/22 23:14:10  wmcoolmon
 * Nonworking OGG support for sound (not music) added, disabled load-only-used-weapons code, modification to sound system registry code.
 * OGG code has been commented out, so you don't need the SDK yet.
 *
 * Revision 2.5  2004/06/18 04:59:55  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.4  2003/11/16 09:42:37  Goober5000
 * clarified and pruned debug spew messages
 * --Goober5000
 *
 * Revision 2.3  2003/03/02 06:37:24  penguin
 * Use multimedia headers in local dir, not system's (headers are not present in MinGW distribution)
 *  - penguin
 *
 * Revision 2.2  2003/01/19 22:45:35  Goober5000
 * cleaned up build output a bit
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 18    10/25/99 5:56p Jefff
 * increase num software channels to the number the users hardware can
 * handle.  not less than 16, tho.
 * 
 * 17    9/08/99 3:22p Dave
 * Updated builtin mission list.
 * 
 * 16    8/27/99 6:38p Alanl
 * crush the blasted repeating messages bug
 * 
 * 15    8/23/99 11:16p Danw
 * Allow stereo waves
 * 
 * 14    8/22/99 11:06p Alanl
 * fix small bug in ds_close_channel
 * 
 * 13    8/19/99 11:25a Alanl
 * change format of secondary buffer from 44100 to 22050
 * 
 * 12    8/17/99 4:11p Danw
 * AL: temp fix for solving A3D crash
 * 
 * 11    8/06/99 2:20p Jasonh
 * AL: free 3D portion of buffer first
 * 
 * 10    8/04/99 9:48p Alanl
 * fix bug with setting 3D properties on a 2D sound buffer
 * 
 * 9     8/04/99 11:42a Danw
 * tone down EAX reverb
 * 
 * 8     8/01/99 2:06p Alanl
 * increase the rolloff for A3D
 * 
 * 7     7/20/99 5:28p Dave
 * Fixed debug build error.
 * 
 * 6     7/20/99 1:49p Dave
 * Peter Drake build. Fixed some release build warnings.
 * 
 * 5     7/14/99 11:32a Danw
 * AL: add some debug code to catch nefarious A3D problem
 * 
 * 4     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 3     10/08/98 4:29p Dave
 * Removed reference to osdefs.h
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 72    6/28/98 6:34p Lawrance
 * add sanity check in while() loop for releasing channels
 * 
 * 71    6/13/98 1:45p Sandeep
 * 
 * 70    6/10/98 2:29p Lawrance
 * don't use COM for initializing DirectSound... appears some machines
 * don't like it
 * 
 * 69    5/26/98 2:10a Lawrance
 * make sure DirectSound pointer gets freed if Aureal resource manager
 * fails
 * 
 * 68    5/21/98 9:14p Lawrance
 * remove obsolete registry setting
 * 
 * 67    5/20/98 4:28p Allender
 * upped sound buffers as per alan's request
 * 
 * 66    5/15/98 3:36p John
 * Fixed bug with new graphics window code and standalone server.  Made
 * hwndApp not be a global anymore.
 * 
 * 65    5/06/98 3:37p Lawrance
 * allow panned sounds geesh
 * 
 * 64    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 63    4/20/98 11:17p Lawrance
 * fix bug with releasing channels
 * 
 * 62    4/20/98 7:34p Lawrance
 * take out obsolete directsound3d debug command
 * 
 * 61    4/20/98 11:10a Lawrance
 * put correct flags when creating sound buffer
 * 
 * 60    4/20/98 12:03a Lawrance
 * Allow prioritizing of CTRL3D buffers
 * 
 * 59    4/19/98 9:31p Lawrance
 * Use Aureal_enabled flag
 * 
 * 58    4/19/98 9:39a Lawrance
 * use DYNAMIC_LOOPERS for Aureal resource manager
 * 
 * 57    4/19/98 4:13a Lawrance
 * Improve how dsound is initialized
 * 
 * 56    4/18/98 9:13p Lawrance
 * Added Aureal support.
 * 
 * 55    4/13/98 5:04p Lawrance
 * Write functions to determine how many milliseconds are left in a sound
 * 
 * 54    4/09/98 5:53p Lawrance
 * Make DirectSound init more robust
 * 
 * 53    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 52    3/31/98 5:19p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 51    3/29/98 12:56a Lawrance
 * preload the warp in and explosions sounds before a mission.
 * 
 * 50    3/25/98 6:10p Lawrance
 * Work on DirectSound3D
 * 
 * 49    3/24/98 4:28p Lawrance
 * Make DirectSound3D support more robust
 * 
 * 48    3/24/98 11:49a Dave
 * AL: Change way buffer gets locked.
 * 
 * 47    3/24/98 11:27a Lawrance
 * Use buffer_size for memcpy when locking buffer
 * 
 * 46    3/23/98 10:32a Lawrance
 * Add functions for extracting raw sound data
 * 
 * 45    3/19/98 5:36p Lawrance
 * Add some sound debug functions to see how many sounds are playing, and
 * to start/stop random looping sounds.
 * 
 * 44    3/07/98 3:35p Dave
 * AL: check for ds being initialized in ds_create_buffer()
 * 
 * 43    2/18/98 5:49p Lawrance
 * Even if the ADPCM codec is unavailable, allow game to continue.
 * 
 * 42    2/16/98 7:31p Lawrance
 * get compression/decompression of voice working
 * 
 * 41    2/15/98 11:10p Lawrance
 * more work on real-time voice system
 * 
 * 40    2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 39    2/06/98 7:30p John
 * Added code to monitor the number of channels of sound actually playing.
 * 
 * 38    2/06/98 8:56a Allender
 * fixed calling convention problem with DLL handles
 * 
 * 37    2/04/98 6:08p Lawrance
 * Read function pointers from dsound.dll, further work on
 * DirectSoundCapture.
 * 
 * 36    2/03/98 11:53p Lawrance
 * Adding support for DirectSoundCapture
 * 
 * 35    1/31/98 5:48p Lawrance
 * Start on real-time voice recording
 * 
 * 34    1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 33    12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 32    12/08/97 12:24a Lawrance
 * Allow duplicate sounds to be stopped if less than OR equal to new sound
 * volume.
 * 
 * 31    12/05/97 5:19p Lawrance
 * re-do sound priorities to make more general and extensible
 * 
 * 30    11/28/97 2:09p Lawrance
 * Overhaul how ADPCM conversion works... use much less memory... safer
 * too.
 * 
 * 29    11/22/97 11:32p Lawrance
 * decompress ADPCM data into 8 bit (not 16bit) for regular sounds (ie not
 * music)
 * 
 * 28    11/20/97 5:36p Dave
 * Hooked in a bunch of main hall changes (including sound). Made it
 * possible to reposition (rewind/ffwd) 
 * sound buffer pointers. Fixed animation direction change framerate
 * problem.
 * 
 * 27    10/13/97 7:41p Lawrance
 * store duration of sound
 * 
 * 26    10/11/97 6:39p Lawrance
 * start playing primary buffer, to reduce latency on sounds starting
 * 
 * 25    10/08/97 5:09p Lawrance
 * limit player impact sounds so only one plays at a time
 * 
 * 24    9/26/97 5:43p Lawrance
 * fix a bug that was freeing memory early when playing compressed sound
 * data
 * 
 * 23    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 22    8/16/97 4:05p Lawrance
 * don't load sounds into hardware if running Lean_and_mean
 * 
 * 21    8/05/97 1:39p Lawrance
 * support compressed stereo playback
 * 
 * 20    7/31/97 10:38a Lawrance
 * return old debug function for toggling DirectSound3D
 * 
 * 19    7/29/97 3:27p Lawrance
 * make console toggle for directsound3d work right
 * 
 * 18    7/28/97 11:39a Lawrance
 * allow individual volume scaling on 3D buffers
 * 
 * 17    7/18/97 8:18p Lawrance
 * fix bug in ds_get_free_channel() that caused sounds to not play when
 * they should have
 * 
 * 16    7/17/97 8:04p Lawrance
 * allow priority sounds to play if free channel, otherwise stop lowest
 * volume priority sound of same type
 * 
 * 15    7/17/97 5:57p John
 * made directsound3d config value work
 * 
 * 14    7/17/97 5:43p John
 * added new config stuff
 * 
 * 13    7/17/97 4:25p John
 * First, broken, stage of changing config stuff
 * 
 * 12    7/15/97 12:13p Lawrance
 * don't stop sounds that have highest priority
 * 
 * 11    7/15/97 11:15a Lawrance
 * limit the max instances of simultaneous sound effects, implement
 * priorities to force critical sounds
 * 
 * 10    6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 9     6/08/97 5:59p Lawrance
 * integrate DirectSound3D into sound system
 * 
 * 8     6/04/97 1:19p Lawrance
 * made hardware mixing robust
 * 
 * 7     6/03/97 1:56p Hoffoss
 * Return correct error code when direct sound init fails.
 * 
 * 6     6/03/97 12:07p Lawrance
 * don't enable 3D sounds in Primary buffer
 * 
 * 5     6/02/97 3:45p Dan
 * temp disable of hardware mixing until problem solved with
 * CreateBuffer() failing
 * 
 * 4     6/02/97 1:45p Lawrance
 * implementing hardware mixing
 * 
 * 3     5/29/97 4:01p Lawrance
 * let snd_init() have final say on initialization
 * 
 * 2     5/29/97 12:04p Lawrance
 * creation of file to hold DirectSound specific portions
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
//#include <initguid.h>
#include "mm/mmreg.h"
#define STUB_FUNCTION
#endif
#include "cfile/cfile.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/acm.h"
#include "osapi/osapi.h"
#include "sound/dscap.h"

#ifdef USE_OPENAL
	#include <vector>

#if !(defined(__APPLE__) || defined(_WIN32))
	#include <AL/al.h>
	#include <AL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif // !__APPLE__ && !_WIN32
#else
#include "sound/channel.h"
#endif // USE_OPENAL

extern unsigned short UserSampleRate, UserSampleBits; //in sound.h


#ifndef USE_OPENAL
// Pointers to functions contained in DSOUND.dll
HRESULT (__stdcall *pfn_DirectSoundCreate)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, IUnknown FAR *pUnkOuter) = NULL;
HRESULT (__stdcall *pfn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter) = NULL;
int Ds_dll_loaded=0;
HINSTANCE Ds_dll_handle=NULL;

LPDIRECTSOUND				pDirectSound = NULL;
LPDIRECTSOUNDBUFFER		pPrimaryBuffer = NULL;

static LPKSPROPERTYSET			pPropertySet;		// pointer to sound card property set
static LPDIRECTSOUNDBUFFER		Ds_property_set_pdsb = NULL;
static LPDIRECTSOUND3DBUFFER	Ds_property_set_pds3db = NULL;

static int Ds_must_call_couninitialize = 0;

channel* Channels;		//[MAX_CHANNELS];
static int channel_next_sig = 1;

#define MAX_DS_SOFTWARE_BUFFERS	256
typedef struct ds_sound_buffer
{
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBUFFERDESC			desc;
	WAVEFORMATEX			wfx;

} ds_sound_buffer;

ds_sound_buffer ds_software_buffers[MAX_DS_SOFTWARE_BUFFERS];

#define MAX_DS_HARDWARE_BUFFERS	32
ds_sound_buffer ds_hardware_buffers[MAX_DS_HARDWARE_BUFFERS];

static DSCAPS Soundcard_caps;					// current soundcard capabilities

extern int Snd_sram;					// mem (in bytes) used up by storing sounds in system memory
extern int Snd_hram;					// mem (in bytes) used up by storing sounds in soundcard memory

static int Ds_use_ds3d = 0;
static int Ds_use_eax = 0;

static bool Stop_logging_sounds = false;

static int MAX_CHANNELS = 0;		// initialized properly in ds_init_channels()

///////////////////////////
//
// EAX
//
///////////////////////////

// presets
//#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.5F,1.493F,0.5F
#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.2F,0.2F,1.0F
#define EAX_PRESET_PADDEDCELL      EAX_ENVIRONMENT_PADDEDCELL,0.25F,0.1F,0.0F
#define EAX_PRESET_ROOM            EAX_ENVIRONMENT_ROOM,0.417F,0.4F,0.666F
#define EAX_PRESET_BATHROOM        EAX_ENVIRONMENT_BATHROOM,0.653F,1.499F,0.166F
#define EAX_PRESET_LIVINGROOM      EAX_ENVIRONMENT_LIVINGROOM,0.208F,0.478F,0.0F
#define EAX_PRESET_STONEROOM       EAX_ENVIRONMENT_STONEROOM,0.5F,2.309F,0.888F
#define EAX_PRESET_AUDITORIUM      EAX_ENVIRONMENT_AUDITORIUM,0.403F,4.279F,0.5F
#define EAX_PRESET_CONCERTHALL     EAX_ENVIRONMENT_CONCERTHALL,0.5F,3.961F,0.5F
#define EAX_PRESET_CAVE            EAX_ENVIRONMENT_CAVE,0.5F,2.886F,1.304F
#define EAX_PRESET_ARENA           EAX_ENVIRONMENT_ARENA,0.361F,7.284F,0.332F
#define EAX_PRESET_HANGAR          EAX_ENVIRONMENT_HANGAR,0.5F,10.0F,0.3F
#define EAX_PRESET_CARPETEDHALLWAY EAX_ENVIRONMENT_CARPETEDHALLWAY,0.153F,0.259F,2.0F
#define EAX_PRESET_HALLWAY         EAX_ENVIRONMENT_HALLWAY,0.361F,1.493F,0.0F
#define EAX_PRESET_STONECORRIDOR   EAX_ENVIRONMENT_STONECORRIDOR,0.444F,2.697F,0.638F
#define EAX_PRESET_ALLEY           EAX_ENVIRONMENT_ALLEY,0.25F,1.752F,0.776F
#define EAX_PRESET_FOREST          EAX_ENVIRONMENT_FOREST,0.111F,3.145F,0.472F
#define EAX_PRESET_CITY            EAX_ENVIRONMENT_CITY,0.111F,2.767F,0.224F
#define EAX_PRESET_MOUNTAINS       EAX_ENVIRONMENT_MOUNTAINS,0.194F,7.841F,0.472F
#define EAX_PRESET_QUARRY          EAX_ENVIRONMENT_QUARRY,1.0F,1.499F,0.5F
#define EAX_PRESET_PLAIN           EAX_ENVIRONMENT_PLAIN,0.097F,2.767F,0.224F
#define EAX_PRESET_PARKINGLOT      EAX_ENVIRONMENT_PARKINGLOT,0.208F,1.652F,1.5F
#define EAX_PRESET_SEWERPIPE       EAX_ENVIRONMENT_SEWERPIPE,0.652F,2.886F,0.25F
#define EAX_PRESET_UNDERWATER      EAX_ENVIRONMENT_UNDERWATER,1.0F,1.499F,0.0F
#define EAX_PRESET_DRUGGED         EAX_ENVIRONMENT_DRUGGED,0.875F,8.392F,1.388F
#define EAX_PRESET_DIZZY           EAX_ENVIRONMENT_DIZZY,0.139F,17.234F,0.666F
#define EAX_PRESET_PSYCHOTIC       EAX_ENVIRONMENT_PSYCHOTIC,0.486F,7.563F,0.806F

static LPKSPROPERTYSET Ds_eax_reverb = NULL;

static int Ds_eax_inited = 0;

EAX_REVERBPROPERTIES Ds_eax_presets[] = 
{
	{EAX_PRESET_GENERIC},
	{EAX_PRESET_PADDEDCELL},
	{EAX_PRESET_ROOM},
	{EAX_PRESET_BATHROOM},
	{EAX_PRESET_LIVINGROOM},
	{EAX_PRESET_STONEROOM},
	{EAX_PRESET_AUDITORIUM},
	{EAX_PRESET_CONCERTHALL},
	{EAX_PRESET_CAVE},
	{EAX_PRESET_ARENA},
	{EAX_PRESET_HANGAR},
	{EAX_PRESET_CARPETEDHALLWAY},
	{EAX_PRESET_HALLWAY},
	{EAX_PRESET_STONECORRIDOR},
	{EAX_PRESET_ALLEY},
	{EAX_PRESET_FOREST},
	{EAX_PRESET_CITY},
	{EAX_PRESET_MOUNTAINS},
	{EAX_PRESET_QUARRY},
	{EAX_PRESET_PLAIN},
	{EAX_PRESET_PARKINGLOT},
	{EAX_PRESET_SEWERPIPE},
	{EAX_PRESET_UNDERWATER},
	{EAX_PRESET_DRUGGED},
	{EAX_PRESET_DIZZY},
	{EAX_PRESET_PSYCHOTIC},
};

GUID DSPROPSETID_EAX_ReverbProperties_Def = {0x4a4e6fc1, 0xc341, 0x11d1, {0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
GUID DSPROPSETID_EAXBUFFER_ReverbProperties_Def = {0x4a4e6fc0, 0xc341, 0x11d1, {0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

//----------------------------------------------------------------
// prototypes 
void ds_get_soundcard_caps(DSCAPS *dscaps);

#elif defined(USE_OPENAL)

typedef struct sound_buffer
{
	ALuint buf_id;		// OpenAL buffer id
	int source_id;		// source index this buffer is currently bound to

	int frequency;
	int bits_per_sample;
	int nchannels;
	int nseconds;
	int nbytes;

	sound_buffer(): buf_id(0), source_id(-1), frequency(0), bits_per_sample(0), nchannels(0), nseconds(0), nbytes(0) {}
} sound_buffer;


static int MAX_CHANNELS = 32;		// initialized properly in ds_init_channels()
channel *Channels = NULL;
static int channel_next_sig = 1;

const int BUFFER_BUMP = 50;
std::vector<sound_buffer> sound_buffers;

extern int Snd_sram;					// mem (in bytes) used up by storing sounds in system memory

static int Ds_use_ds3d = 0;
static int Ds_use_a3d = 0;
static int Ds_use_eax = 0;

static int AL_play_position = 0;

#ifndef AL_BYTE_LOKI
// in case it's not defined by older/other drivers
#define AL_BYTE_LOKI	0x100C
#endif

ALCdevice *ds_sound_device = NULL;
ALCcontext *ds_sound_context = NULL;

ALCint AL_minor_version = 0;

//--------------------------------------------------------------------------
// openal_error_string()
//
// Returns the human readable error string if there is an error or NULL if not
//
const char* openal_error_string(int get_alc)
{
	int i;

	if (get_alc) {
		// Apple implementation requires a valid device to give a valid error msg
		i = alcGetError(ds_sound_device);

		if ( i != ALC_NO_ERROR )
			return (const char*) alcGetString(NULL, i);
	}
	else {
		i = alGetError();

		if ( i != AL_NO_ERROR )
			return (const char*)alGetString(i);
	}

	return NULL;
}

#endif // defined(USE_OPENAL)

int ds_vol_lookup[101];						// lookup table for direct sound volumes
int ds_initialized = FALSE;

//--------------------------------------------------------------------------
// ds_is_3d_buffer()
//
// Determine if a secondary buffer is a 3d secondary buffer.
//
#ifndef USE_OPENAL
int ds_is_3d_buffer(LPDIRECTSOUNDBUFFER pdsb)
{
	DSBCAPS			dsbc;
	HRESULT			hr;

	dsbc.dwSize = sizeof(dsbc);
	hr = pdsb->GetCaps(&dsbc);
	if ( hr == DS_OK && dsbc.dwFlags & DSBCAPS_CTRL3D ) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}
#endif
//--------------------------------------------------------------------------
// ds_is_3d_buffer()
//
// Determine if a secondary buffer is a 3d secondary buffer.
//
int ds_is_3d_buffer(int sid)
{
#ifndef USE_OPENAL
	if ( sid >= 0 ) {
		return ds_is_3d_buffer(ds_software_buffers[sid].pdsb);
	}
#else
	// they are all 3d
	if ( sid >= 0 ) {
		return 1;
	}
#endif
	return 0;
}

//--------------------------------------------------------------------------
//  ds_build_vol_lookup()
//
//  Fills up the ds_vol_lookup[] tables that converts from a volume in the form
//  0.0 -> 1.0 to -10000 -> 0 (this is the DirectSound method, where units are
//  hundredths of decibls)
//
void ds_build_vol_lookup()
{
	int	i;
	float	vol;

	ds_vol_lookup[0] = -10000;
	for ( i = 1; i <= 100; i++ ) {
		vol = i / 100.0f;
		ds_vol_lookup[i] = fl2i( (log(vol) / log(2.0f)) * 1000.0f);
	}
}


//--------------------------------------------------------------------------
// ds_convert_volume()
//
// Takes volume between 0.0f and 1.0f and converts into
// DirectSound style volumes between -10000 and 0.
int ds_convert_volume(float volume)
{
	int index;

	index = fl2i(volume * 100.0f);
	if ( index > 100 )
		index = 100;
	if ( index < 0 )
		index = 0;

	return ds_vol_lookup[index];
}

//--------------------------------------------------------------------------
// ds_get_percentage_vol()
//
// Converts -10000 -> 0 range volume to 0 -> 1
float ds_get_percentage_vol(int ds_vol)
{
	double vol;
	vol = pow(2.0, ds_vol/1000.0);
	return (float)vol;
}

// ---------------------------------------------------------------------------------------
// ds_parse_sound() 
//
// Parse a wave file.
//
// parameters:		filename			=> file of sound to parse
//						dest				=> address of pointer of where to store raw sound data (output parm)
//						dest_size		=> number of bytes of sound data stored (output parm)
//						header			=> address of pointer to a WAVEFORMATEX struct (output parm)
//						ovf				=> pointer to a OggVorbis_File struct, OGG vorbis only (output parm)
//
// returns:			0					=> wave file successfully parsed
//						-1					=> error
//
//	NOTE: memory is malloced for the header and dest (if not OGG) in this function.  It is the responsibility
//			of the caller to free this memory later.
//
int ds_parse_sound(CFILE* fp, ubyte **dest, uint *dest_size, WAVEFORMATEX **header, bool ogg, OggVorbis_File *ovf)
{
	PCMWAVEFORMAT	PCM_header;
	ushort			cbExtra = 0;
	unsigned int	tag, size, next_chunk;
	bool			got_fmt = false, got_data = false;

	// some preinit stuff, could be done from calling function but this should guarantee it's right
	*dest = NULL;
	*dest_size = 0;

	if (fp == NULL)
		return -1;


	// if we should have a Vorbis file then try for it
	if (ogg) {
		if (ovf == NULL) {
			Int3();
			return -1;
		}

		// Check for OGG Vorbis first
		if ( !ov_open_callbacks(fp, ovf, NULL, 0, cfile_callbacks) ) {
			// got one, now read all of the needed header info
			ov_info(ovf, -1);

			// we only support one logical bitstream
			if ( ov_streams(ovf) != 1 ) {
				nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
				return -1;
			}

			if ( (*header = (WAVEFORMATEX *) vm_malloc ( sizeof(WAVEFORMATEX) )) != NULL ) {
				(*header)->wFormatTag = OGG_FORMAT_VORBIS;
				(*header)->nChannels = (ushort)ovf->vi->channels;
				(*header)->nSamplesPerSec = ovf->vi->rate;
				(*header)->wBitsPerSample = 16;								//OGGs always decoded at 16 bits here
				(*header)->nBlockAlign = (ushort)(ovf->vi->channels * 2);
				(*header)->nAvgBytesPerSec =  ovf->vi->rate * ovf->vi->channels * 2;

				//WMC - Total samples * channels * bits/sample
				*dest_size = (uint)(ov_pcm_total(ovf, -1) * ovf->vi->channels * 2);
			} else {
				Assert( 0 );
				return -1;
			}

			// we're all good, can leave now
			return 0;
		}
	}
	// otherwise we assime Wave format
	else {
		// Skip the "RIFF" tag and file size (8 bytes)
		// Skip the "WAVE" tag (4 bytes)
		// IMPORTANT!! Look at snd_load before even THINKING about changing this.
		cfseek( fp, 12, CF_SEEK_SET );

		// Now read RIFF tags until the end of file

		while (1) {
			if ( cfread( &tag, sizeof(uint), 1, fp ) != 1 )
				break;

			tag = INTEL_INT( tag );

			if ( cfread( &size, sizeof(uint), 1, fp ) != 1 )
				break;

			size = INTEL_INT( size );

			next_chunk = cftell(fp) + size;

			switch (tag)
			{
				case 0x20746d66:		// The 'fmt ' tag
				{
					//nprintf(("Sound", "SOUND => size of fmt block: %d\n", size));
					PCM_header.wf.wFormatTag		= cfread_ushort(fp);
					PCM_header.wf.nChannels			= cfread_ushort(fp);
					PCM_header.wf.nSamplesPerSec	= cfread_uint(fp);
					PCM_header.wf.nAvgBytesPerSec	= cfread_uint(fp);
					PCM_header.wf.nBlockAlign		= cfread_ushort(fp);
					PCM_header.wBitsPerSample		= cfread_ushort(fp);

					if (PCM_header.wf.wFormatTag != WAVE_FORMAT_PCM)
						cbExtra = cfread_ushort(fp);

					// Allocate memory for WAVEFORMATEX structure + extra bytes
					if ( (*header = (WAVEFORMATEX *) vm_malloc ( sizeof(WAVEFORMATEX)+cbExtra )) != NULL ) {
						// Copy bytes from temporary format structure
						memcpy (*header, &PCM_header, sizeof(PCM_header));
						(*header)->cbSize = cbExtra;

						// Read those extra bytes, append to WAVEFORMATEX structure
						if (cbExtra != 0)
							cfread( ((ubyte *)(*header) + sizeof(WAVEFORMATEX)), cbExtra, 1, fp);
					} else {
						Assert(0);		// malloc failed
					}

					got_fmt = true;
	
					break;
				}

				case 0x61746164:		// the 'data' tag
				{
					*dest_size = size;

					(*dest) = (ubyte *)vm_malloc(size);
					Assert( *dest != NULL );

					cfread( *dest, size, 1, fp );

					got_data = true;

					break;
				}

				default:	// unknown, skip it
					break;
			}

			// This is here so that we can avoid reading data that we don't understand or properly handle.
			// We could do this just as well by checking the RIFF size, but this is easier - taylor
			if (got_fmt && got_data)
				break;

			cfseek( fp, next_chunk, CF_SEEK_SET );
		}

		// we're all good, can leave now
		return 0;
	}

	return -1;
}

// ---------------------------------------------------------------------------------------
// ds_parse_sound_info() 
//
// Parse a a sound file, any format, and store the info in "s_info".
//
int ds_parse_sound_info(char *real_filename, sound_info *s_info)
{
	PCMWAVEFORMAT	PCM_header;
	uint			tag, size, next_chunk;
	bool			got_fmt = false, got_data = false;
	OggVorbis_File	ovf;
	int				rc, FileSize, FileOffset;
	char			fullpath[MAX_PATH];
	char			filename[MAX_FILENAME_LEN];
	const int		NUM_EXT = 2;
	const char		*audio_ext[NUM_EXT] = { ".ogg", ".wav" };


	if ( (real_filename == NULL) || (s_info == NULL) )
		return -1;


	// remove extension
	strcpy( filename, real_filename );
	char *p = strchr(filename, '.');
	if ( p ) *p = 0;

	rc = cf_find_file_location_ext(filename, NUM_EXT, audio_ext, CF_TYPE_ANY, sizeof(fullpath) - 1, fullpath, &FileSize, &FileOffset);

	if (rc < 0)
		return -1;

	// open the file
	CFILE *fp = cfopen_special(fullpath, "rb", FileSize, FileOffset);

	if (fp == NULL)
		return -1;


	// Ogg Vorbis
	if (rc == 0) {
		if ( !ov_open_callbacks(fp, &ovf, NULL, 0, cfile_callbacks) ) {
			// got one, now read all of the needed header info
			ov_info(&ovf, -1);

			// we only support one logical bitstream
			if ( ov_streams(&ovf) != 1 ) {
				nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
				return -1;
			}

			s_info->format = OGG_FORMAT_VORBIS;
			s_info->n_channels = (ushort)ovf.vi->channels;
			s_info->sample_rate = ovf.vi->rate;
			s_info->bits = 16;								//OGGs always decoded at 16 bits here
			s_info->n_block_align = (ushort)(ovf.vi->channels * 2);
			s_info->avg_bytes_per_sec = ovf.vi->rate * ovf.vi->channels * 2;

			s_info->size = (uint)(ov_pcm_total(&ovf, -1) * ovf.vi->channels * 2);

			ov_clear(&ovf);
	
			// we're all good, can leave now
			goto Done;
		}
	}
	// PCM Wave
	else if (rc == 1) {
		// Skip the "RIFF" tag and file size (8 bytes)
		// Skip the "WAVE" tag (4 bytes)
		// IMPORTANT!! Look at snd_load before even THINKING about changing this.
		cfseek( fp, 12, CF_SEEK_SET );

		// Now read RIFF tags until the end of file

		while (1) {
			if ( cfread( &tag, sizeof(uint), 1, fp ) != 1 )
				break;

			tag = INTEL_INT( tag );

			if ( cfread( &size, sizeof(uint), 1, fp ) != 1 )
				break;

			size = INTEL_INT( size );

			next_chunk = cftell(fp) + size;

			switch (tag)
			{
				case 0x20746d66:		// The 'fmt ' tag
					PCM_header.wf.wFormatTag		= cfread_ushort(fp);
					PCM_header.wf.nChannels			= cfread_ushort(fp);
					PCM_header.wf.nSamplesPerSec	= cfread_uint(fp);
					PCM_header.wf.nAvgBytesPerSec	= cfread_uint(fp);
					PCM_header.wf.nBlockAlign		= cfread_ushort(fp);
					PCM_header.wBitsPerSample		= cfread_ushort(fp);

					s_info->format = PCM_header.wf.wFormatTag;
					s_info->n_channels = PCM_header.wf.nChannels;
					s_info->sample_rate = PCM_header.wf.nSamplesPerSec;
					s_info->bits = PCM_header.wBitsPerSample;
					s_info->n_block_align = PCM_header.wf.nBlockAlign;
					s_info->avg_bytes_per_sec =  PCM_header.wf.nAvgBytesPerSec;

					got_fmt = true;
	
					break;

				case 0x61746164:		// the 'data' tag
					s_info->size = size;
					got_data = true;

					break;

				default:
					break;
			}

			if (got_fmt && got_data)
				goto Done;

			cfseek( fp, next_chunk, CF_SEEK_SET );
		}
	}

	return -1;

Done:
	cfclose(fp);
	return 0;
}

// ---------------------------------------------------------------------------------------
// ds_get_sid()
//
//	
int ds_get_sid()
{
#ifdef USE_OPENAL
	sound_buffer new_buffer;
	uint i;

	for (i = 0; i < sound_buffers.size(); i++) {
		if (sound_buffers[i].buf_id == 0) {
			return (int)i;
		}
	}

	// if we need to, bump the reserve limit (helps prevent memory fragmentation)
	if ( sound_buffers.size() == sound_buffers.capacity() ) {
		sound_buffers.reserve( sound_buffers.size() + BUFFER_BUMP );
	}

	sound_buffers.push_back( new_buffer );

	return (int)(sound_buffers.size() - 1);
#else
	int i;

	for ( i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++ ) {
		if ( ds_software_buffers[i].pdsb == NULL )
		break;
	}

	if ( i == MAX_DS_SOFTWARE_BUFFERS )	{
		return -1;
	}

	return i;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_get_hid()
//
//	
int ds_get_hid()
{
#ifdef USE_OPENAL
	return -1;
#else
	int i;

	for ( i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++ ) {
		if ( ds_hardware_buffers[i].pdsb == NULL )
		break;
	}

	if ( i == MAX_DS_HARDWARE_BUFFERS )	{
		return -1;
	}

	return i;
#endif
}

// ---------------------------------------------------------------------------------------
// Load a DirectSound secondary buffer with sound data.  The sounds data for
// game sounds are stored in the DirectSound secondary buffers, and are 
// duplicated as needed and placed in the Channels[] array to be played.
// 
//
// parameters:  
//					 sid				  => pointer to software id for sound ( output parm)
//					 hid				  => pointer to hardware id for sound ( output parm)
//					 final_size		  => pointer to storage to receive uncompressed sound size (output parm)
//              header          => pointer to a WAVEFORMATEX structure
//					 si				  => sound_info structure, contains details on the sound format
//					 flags			  => buffer properties ( DS_HARDWARE , DS_3D )
//
// returns:     -1           => sound effect could not loaded into a secondary buffer
//               0           => sound effect successfully loaded into a secondary buffer
//
//
// NOTE: this function is slow, especially when sounds are loaded into hardware.  Don't call this
// function from within gameplay.
//
int ds_load_buffer(int *sid, int *hid, int *final_size, void *header, sound_info *si, int flags)
{
#ifdef USE_OPENAL
	Assert( final_size != NULL );
	Assert( header != NULL );
	Assert( si != NULL );

	// All sounds are required to have a software buffer

	*sid = ds_get_sid();
	if ( *sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more sound buffers available\n"));
		return -1;
	}

	ALuint pi;
	OpenAL_ErrorCheck( alGenBuffers (1, &pi), return -1 );

	ALenum format;
	ALsizei size;
	ALint bits, bps;
	ALuint frequency;
	ALvoid *data = NULL;
	int byte_order = 0, section, last_section = -1;

	// the below two covnert_ variables are only used when the wav format is not
	// PCM.  DirectSound only takes PCM sound data, so we must convert to PCM if required
	ubyte *convert_buffer = NULL;		// storage for converted wav file
	int convert_len;					// num bytes of converted wav file
	uint src_bytes_used;				// number of source bytes actually converted (should always be equal to original size)
	int rc;
	WAVEFORMATEX *pwfx = (WAVEFORMATEX *)header;


	switch (si->format) {
		case WAVE_FORMAT_PCM:
			Assert( si->data != NULL );
			bits = si->bits;
			bps  = si->avg_bytes_per_sec;
			size = si->size;
#if BYTE_ORDER == BIG_ENDIAN
			// swap 16-bit sound data
			if (bits == 16) {
				ushort *swap_tmp;

				for (uint i=0; i<size; i=i+2) {
					swap_tmp = (ushort*)(si->data + i);
					*swap_tmp = INTEL_SHORT(*swap_tmp);
				}
			}
#endif
			data = si->data;
			break;

		case WAVE_FORMAT_ADPCM:
			Assert( si->data != NULL );
			// this ADPCM decoder decodes to 16-bit only so keep that in mind
			nprintf(( "Sound", "SOUND ==> converting sound from ADPCM to PCM\n" ));
			rc = ACM_convert_ADPCM_to_PCM(pwfx, si->data, si->size, &convert_buffer, 0, &convert_len, &src_bytes_used, 16);

			if ( rc == -1 ) {
				return -1;
			}

			if (src_bytes_used != si->size) {
				return -1;	// ACM conversion failed?
			}

			bits = 16;
			bps  = (((si->n_channels * bits) / 8) * si->sample_rate);
			size = convert_len;
			data = convert_buffer;

			nprintf(( "Sound", "SOUND ==> Coverted sound from ADPCM to PCM successfully\n" ));
			break;

		case OGG_FORMAT_VORBIS:
			nprintf(( "Sound", "SOUND ==> converting sound from OGG to PCM\n" ));

#if BYTE_ORDER == BIG_ENDIAN
			byte_order = 1;
#endif
			src_bytes_used = 0;
			convert_buffer = (ubyte*)vm_malloc(si->size);
			Assert(convert_buffer != NULL);

			if (convert_buffer == NULL)
				return -1;

			while (src_bytes_used < si->size) {
				rc = ov_read(&si->ogg_info, (char *) convert_buffer + src_bytes_used, si->size - src_bytes_used, byte_order, si->bits / 8, 1, &section);

				// fail if the bitstream changes, shouldn't get this far if that's the case though
				if ((last_section != -1) && (last_section != section)) {
					nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
					vm_free(convert_buffer);
					convert_buffer = NULL;
					return -1;
				}

				if (rc == OV_EBADLINK) {
					vm_free(convert_buffer);
					convert_buffer = NULL;
					return -1;
				} else if (rc == 0) {
					break;
				} else if (rc > 0) {
					last_section = section;
					src_bytes_used += rc;
				}
			}

			bits = si->bits;
			bps = (((si->n_channels * bits) / 8) * si->sample_rate);
			size = (int)src_bytes_used;
			data = convert_buffer;

			// we're done with ogg stuff so clean it up
			ov_clear(&si->ogg_info);

			nprintf(( "Sound", "SOUND ==> Coverted sound from OGG to PCM successfully\n" ));
			break;

		default:
			STUB_FUNCTION;
			return -1;
	}

	/* format is now in pcm */
	frequency = si->sample_rate;

	if (bits == 16) {
		if (si->n_channels == 2) {
			format = AL_FORMAT_STEREO16;
		} else if (si->n_channels == 1) {
			format = AL_FORMAT_MONO16;
		} else {
			return -1;
		}
	} else if (bits == 8) {
		if (si->n_channels == 2) {
			format = AL_FORMAT_STEREO8;
		} else if (si->n_channels == 1) {
			format = AL_FORMAT_MONO8;
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	Snd_sram += size;
	*final_size = size;

	OpenAL_ErrorCheck( alBufferData(pi, format, data, size, frequency), return -1 );

	sound_buffers[*sid].buf_id = pi;
	sound_buffers[*sid].source_id = -1;
	sound_buffers[*sid].frequency = frequency;
	sound_buffers[*sid].bits_per_sample = bits;
	sound_buffers[*sid].nchannels = si->n_channels;
	sound_buffers[*sid].nseconds = size / bps;
	sound_buffers[*sid].nbytes = size;

	if ( convert_buffer )
		vm_free( convert_buffer );

	return 0;
#else
	Assert( final_size != NULL );
	Assert( si != NULL );
	Assert( si->size > 0 );
	Assert( si->sample_rate > 0);
	Assert( si->bits > 0 );
	Assert( si->n_channels > 0 );
	Assert( si->n_block_align >= 0 );
	Assert( si->avg_bytes_per_sec > 0 );

	WAVEFORMATEX	*pwfx = (WAVEFORMATEX *)header;
	DSBUFFERDESC	BufferDesc;
	WAVEFORMATEX	WaveFormat;
	HRESULT			DSReturn;
	int				rc = 1, final_sound_size, DSOUND_load_buffer_result = 0, byte_order = 0, section, last_section = -1;
	BYTE				*pData, *pData2;
	DWORD				DataSize, DataSize2;

	// the below two covnert_ variables are only used when the wav format is not 
	// PCM.  DirectSound only takes PCM sound data, so we must convert to PCM if required
	ubyte *convert_buffer = NULL;		// storage for converted wav file 
	int	convert_len;					// num bytes of converted wav file
	uint	src_bytes_used;				// number of source bytes actually converted (should always be equal to original size)

	// Ensure DirectSound initialized
	if (!ds_initialized) {
		DSOUND_load_buffer_result = -1;
		goto DSOUND_load_buffer_done;
	}

	// Set up buffer information
	WaveFormat.wFormatTag		= (unsigned short)si->format;
	WaveFormat.nChannels		= (unsigned short)si->n_channels;
	WaveFormat.nSamplesPerSec	= si->sample_rate;
	WaveFormat.wBitsPerSample	= (unsigned short)si->bits;
	WaveFormat.cbSize			= 0;
	WaveFormat.nBlockAlign		= (unsigned short)si->n_block_align;
	WaveFormat.nAvgBytesPerSec = si->avg_bytes_per_sec;

	final_sound_size = si->size;	// assume this format will be used, may be over-ridded by convert_len

//	Assert(WaveFormat.nChannels == 1);

	switch ( si->format ) {
		case WAVE_FORMAT_PCM:
			break;

		case WAVE_FORMAT_ADPCM:
			Assert( pwfx != NULL );
			Assert( si->data != NULL );
			nprintf(( "Sound", "SOUND ==> converting sound from ADPCM to PCM\n" ));

			//Do ADPCM conversion at what bitrate the user wants.
			if(UserSampleBits == 16 || UserSampleBits == 8)
				WaveFormat.wBitsPerSample = UserSampleBits;
			else if(UserSampleBits > 16)
				WaveFormat.wBitsPerSample = 16;
			else
				WaveFormat.wBitsPerSample = 8;

			rc = ACM_convert_ADPCM_to_PCM(pwfx, si->data, si->size, &convert_buffer, 0, &convert_len, &src_bytes_used, WaveFormat.wBitsPerSample);
			if ( rc == -1 ) {
				DSOUND_load_buffer_result = -1;
				goto DSOUND_load_buffer_done;
			}

			if (src_bytes_used != si->size) {
				Int3();	// ACM conversion failed?
				DSOUND_load_buffer_result = -1;
				goto DSOUND_load_buffer_done;
			}

			final_sound_size = convert_len;

			// Set up the WAVEFORMATEX structure to have the right PCM characteristics
			WaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
			WaveFormat.nChannels		= (unsigned short)si->n_channels;
			WaveFormat.nSamplesPerSec	= si->sample_rate;
			WaveFormat.cbSize				= 0;
			WaveFormat.nBlockAlign		= (unsigned short)(( WaveFormat.nChannels * WaveFormat.wBitsPerSample ) / 8);
			WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign * WaveFormat.nSamplesPerSec;

			nprintf(( "Sound", "SOUND ==> Coverted sound from ADPCM to PCM successfully\n" ));
			break;	

		case OGG_FORMAT_VORBIS:
			nprintf(( "Sound", "SOUND ==> converting sound from OGG to PCM\n" ));

#if BYTE_ORDER == BIG_ENDIAN
			byte_order = 1;
#endif
			src_bytes_used = 0;
			convert_buffer = (byte*) vm_malloc(si->size);
			Assert(convert_buffer != NULL);

			while (src_bytes_used < si->size) {
				rc = ov_read(&si->ogg_info, (char *) convert_buffer + src_bytes_used, si->size - src_bytes_used, byte_order, si->bits / 8, 1, &section);

				// fail if the bitstream changes, shouldn't get this far if that's the case though
				if ((last_section != -1) && (last_section != section)) {
					nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
					vm_free(convert_buffer);
					convert_buffer = NULL;
					DSOUND_load_buffer_result = -1;
					goto DSOUND_load_buffer_done;
				}

				if (rc == OV_EBADLINK) {
					vm_free(convert_buffer);
					convert_buffer = NULL;
					DSOUND_load_buffer_result = -1;
					goto DSOUND_load_buffer_done;
				} else if (rc == 0) {
					break;
				} else if (rc > 0) {
					last_section = section;
					src_bytes_used += rc;
				}
			}

			final_sound_size = (int)src_bytes_used;

			WaveFormat.wFormatTag		= (unsigned short) WAVE_FORMAT_PCM;
			WaveFormat.nChannels		= (unsigned short) si->n_channels;
			WaveFormat.nSamplesPerSec	= si->sample_rate;
			WaveFormat.wBitsPerSample	= (unsigned short) si->bits;
			WaveFormat.cbSize			= 0;
			WaveFormat.nBlockAlign		= (unsigned short) si->n_block_align;
			WaveFormat.nAvgBytesPerSec	= si->avg_bytes_per_sec;

			//The ogg handle isn't needed no more
			ov_clear(&si->ogg_info);

			nprintf(( "Sound", "SOUND ==> Coverted sound from OGG to PCM successfully\n" ));
			break;
		default:
			nprintf(( "Sound", "Unsupported sound encoding\n" ));
			DSOUND_load_buffer_result = -1;
			goto DSOUND_load_buffer_done;
			break;
	}

	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;		// DirectSound only used PCM wave files

	// Set up a DirectSound buffer
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.dwSize = sizeof(BufferDesc);
	BufferDesc.dwBufferBytes = final_sound_size;
	BufferDesc.lpwfxFormat = &WaveFormat;

	// check if DirectSound3D is enabled and the sound is flagged for 3D
	if ((ds_using_ds3d()) && (flags & DS_USE_DS3D)) {
//	if (ds_using_ds3d()) {
		BufferDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	} else {
		BufferDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME;
	}

	// Create a new software buffer using the settings for this wave
	// All sounds are required to have a software buffer
	*sid = ds_get_sid();
	if ( *sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more software secondary buffers available\n"));
		return -1;
	}
	DSReturn = pDirectSound->CreateSoundBuffer(&BufferDesc, &ds_software_buffers[*sid].pdsb, NULL );

	if ( DSReturn == DS_OK && ds_software_buffers[*sid].pdsb != NULL )	{

		ds_software_buffers[*sid].desc = BufferDesc;
		ds_software_buffers[*sid].wfx =	*BufferDesc.lpwfxFormat;

		// Lock the buffer and copy in the data
		if ((ds_software_buffers[*sid].pdsb)->Lock(0, final_sound_size, (void**)(&pData), &DataSize, (void**)(&pData2), &DataSize2, 0) == DS_OK)	{

			if ( convert_buffer )
				memcpy(pData, convert_buffer, final_sound_size); // use converted data (PCM format)
			else
				memcpy(pData, si->data, final_sound_size);

			(ds_software_buffers[*sid].pdsb)->Unlock(pData, DataSize, 0, 0);
		}
		DSOUND_load_buffer_result = 0;

		// update ram used for sound
		Snd_sram += final_sound_size;
		*final_size = final_sound_size;
	}
	else {
		nprintf(("Sound","SOUND => fatal error in DSOUND_load_buffer\n"));
		*sid = -1;
		DSOUND_load_buffer_result = -1;
	}

	DSOUND_load_buffer_done:
	if ( convert_buffer )
		vm_free( convert_buffer );
	return DSOUND_load_buffer_result;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_init_channels()
//
// init the Channels[] array
//
void ds_init_channels()
{
#ifdef USE_OPENAL
	int i, n;
	ALuint *sids;
	sids = new ALuint[MAX_CHANNELS];

	// -------------------------------------------------------------------------
	// Begin crazy little error check...  This will try and generate up to MAX_CHANNELS worth
	// of sources and if there is an error break off and reset MAX_CHANNELS to that number.  This
	// makes sure that we don't try to make more sources than the implementation can handle.
	//
	// NOTE: This doesn't protect against bad drivers so it's possible to generate more than
	//       we could actually use.  In that case it will likely segfault.

	// clear the current error buffer before doing anything else
	n = alGetError();
	while ( n != AL_NO_ERROR ) {
		n = alGetError();
	}

	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		alGenSources( 1, &sids[i] );
		n = alGetError();
		if ( n != AL_NO_ERROR )
			break;
	}

	// if we didn't make all of them then reset the max and give a Warning message
	if ( i != MAX_CHANNELS ) {
		nprintf(("Warning", "OpenAL: Restricting MAX_CHANNELS to %i (default: %i)\n", i, MAX_CHANNELS));
		MAX_CHANNELS = i;
	}

	// now we have to delete them of course so that the game can make the real ones
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( (sids[i] != 0) && alIsSource(sids[i]) )
			OpenAL_ErrorPrint( alDeleteSources(1, &sids[i]) );
	}

	// cleanup
	delete[] sids;

	// ... End crazy little error check
	// -------------------------------------------------------------------------

	Channels = (channel*) vm_malloc(sizeof(channel) * MAX_CHANNELS);
	if (Channels == NULL) {
		Error(LOCATION, "Unable to allocate %d bytes for %d audio channels.", sizeof(channel) * MAX_CHANNELS, MAX_CHANNELS);
	}

	memset( Channels, 0, sizeof(channel) * MAX_CHANNELS );

	// init the channels
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		Channels[i].source_id = 0;
		Channels[i].buf_id = -1;
		Channels[i].sig = -1;
		Channels[i].snd_id = -1;
	}
#else
	int i;

	// detect how many channels we can support
	DSCAPS caps;
	ds_get_soundcard_caps(&caps);

//	caps.dwSize = sizeof(DSCAPS);
//	pDirectSound->GetCaps(&caps);

	// minimum 16 channels
	MAX_CHANNELS = caps.dwMaxHwMixingStaticBuffers;
#ifndef NDEBUG
	int dbg_channels = MAX_CHANNELS;
#endif
	if (MAX_CHANNELS < 16) {
		MAX_CHANNELS = 16;
	}

	// allocate the channels array
	Channels = (channel*) vm_malloc(sizeof(channel) * MAX_CHANNELS);
	if (Channels == NULL) {
		Error(LOCATION, "Unable to allocate %d bytes for %d audio channels.", sizeof(channel) * MAX_CHANNELS, MAX_CHANNELS);
	}

	// init the channels
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		Channels[i].pdsb = NULL;
		Channels[i].pds3db = NULL;
		Channels[i].vol = 0;
	}
#ifndef NDEBUG
	mprintf(("** MAX_CHANNELS set to %d.  DS reported %d.\n", MAX_CHANNELS, dbg_channels));
#endif
#endif
}

// ---------------------------------------------------------------------------------------
// ds_init_software_buffers()
//
// init the software buffers
//
void ds_init_software_buffers()
{
#ifdef USE_OPENAL
	sound_buffers.clear();

	// pre-allocate for at least BUFFER_BUMP buffers
	sound_buffers.reserve( BUFFER_BUMP );
#else
	int i;

	for ( i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++ ) {
		ds_software_buffers[i].pdsb = NULL;
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_init_hardware_buffers()
//
// init the hardware buffers
//
void ds_init_hardware_buffers()
{
#ifdef USE_OPENAL
	//	STUB_FUNCTION;	// not needed with openal (CM)
	return;
#else
	int i;

	for ( i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++ ) {
		ds_hardware_buffers[i].pdsb = NULL;
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_init_buffers()
//
// init the both the software and hardware buffers
//
void ds_init_buffers()
{
	ds_init_software_buffers();
	ds_init_hardware_buffers();
}

// Get the current soundcard capabilities
#ifndef USE_OPENAL
void ds_get_soundcard_caps(DSCAPS *dscaps)
{
	HRESULT	hr;
	int		n_hbuffers, hram;

	dscaps->dwSize = sizeof(DSCAPS);

	hr = pDirectSound->GetCaps(dscaps); 
	if (hr != DS_OK )	{
		nprintf(("Sound","SOUND ==> DirectSound GetCaps() failed with code %s\n.",get_DSERR_text(hr) ));
		return;
	}
	
	n_hbuffers = dscaps->dwMaxHwMixingStaticBuffers;
	hram = dscaps->dwTotalHwMemBytes;
	
	if ( !(dscaps->dwFlags & DSCAPS_CERTIFIED) ) {
		nprintf(("Sound","SOUND ==> Warning: audio driver is not Microsoft certified.\n"));
	}
}

// ---------------------------------------------------------------------------------------
// ds_show_caps()
//
// init the both the software and hardware buffers
//
void ds_show_caps(DSCAPS *dscaps)
{
	nprintf(("Sound", "SOUND => Soundcard Capabilities:\n"));
	nprintf(("Sound", "================================\n"));
	nprintf(("Sound", "Number of primary buffers: %d\n", dscaps->dwPrimaryBuffers ));
	nprintf(("Sound", "Number of total hw mixing buffers: %d\n", dscaps->dwMaxHwMixingAllBuffers ));
	nprintf(("Sound", "Number of total hw mixing static buffers: %d\n", dscaps->dwMaxHwMixingStaticBuffers ));
	nprintf(("Sound", "Number of total hw mixing streaming buffers: %d\n", dscaps->dwMaxHwMixingStreamingBuffers ));
	nprintf(("Sound", "Number of free hw mixing buffers: %d\n", dscaps->dwFreeHwMixingAllBuffers ));
	nprintf(("Sound", "Number of free hw mixing static buffers: %d\n", dscaps->dwFreeHwMixingStaticBuffers ));
	nprintf(("Sound", "Number of free hw mixing streaming buffers: %d\n", dscaps->dwFreeHwMixingStreamingBuffers ));
	nprintf(("Sound", "Number of hw 3D buffers: %d\n", dscaps->dwMaxHw3DAllBuffers ));
	nprintf(("Sound", "Number of hw 3D static buffers: %d\n", dscaps->dwMaxHw3DStaticBuffers ));
	nprintf(("Sound", "Number of hw 3D streaming buffers: %d\n", dscaps->dwMaxHw3DStreamingBuffers ));
	nprintf(("Sound", "Number of free hw 3D buffers: %d\n", dscaps->dwFreeHw3DAllBuffers ));
	nprintf(("Sound", "Number of free hw static 3D buffers: %d\n", dscaps->dwFreeHw3DStaticBuffers ));
	nprintf(("Sound", "Number of free hw streaming 3D buffers: %d\n", dscaps->dwFreeHw3DStreamingBuffers ));
	nprintf(("Sound", "Number of total hw bytes: %d\n", dscaps->dwTotalHwMemBytes ));
	nprintf(("Sound", "Number of free hw bytes: %d\n", dscaps->dwFreeHwMemBytes ));
	nprintf(("Sound", "================================\n"));
}
#endif

// Fill in the waveformat struct with the primary buffer characteristics.
#ifndef USE_OPENAL
void ds_get_primary_format(WAVEFORMATEX *wfx, DWORD sample_rate, WORD sample_bits)
{
	// Set 16 bit / 22KHz / mono
	wfx->wFormatTag = WAVE_FORMAT_PCM;
	wfx->nChannels = 2;
	wfx->nSamplesPerSec = sample_rate;
	wfx->wBitsPerSample = sample_bits;
	wfx->cbSize = 0;
	wfx->nBlockAlign = (unsigned short)(wfx->nChannels * (wfx->wBitsPerSample / 8));
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}
//XSTR:OFF
// obtain the function pointers from the dsound.dll
void ds_dll_get_functions()
{
	pfn_DirectSoundCreate = (HRESULT(__stdcall *)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, IUnknown FAR *pUnkOuter))GetProcAddress(Ds_dll_handle,"DirectSoundCreate");
	pfn_DirectSoundCaptureCreate = (HRESULT(__stdcall *)(LPGUID lpGuid, LPDIRECTSOUNDCAPTURE *lplpDSC, IUnknown FAR *pUnkOuter))GetProcAddress(Ds_dll_handle,"DirectSoundCaptureCreate");
}
#endif

// Load the dsound.dll, and get funtion pointers
// exit:	0	->	dll loaded successfully
//			!0	->	dll could not be loaded
int ds_dll_load()
{
#ifndef USE_OPENAL
	if ( !Ds_dll_loaded ) {
		Ds_dll_handle = LoadLibrary("dsound.dll");
		if ( !Ds_dll_handle ) {
			return -1;
		}
		ds_dll_get_functions();
		Ds_dll_loaded=1;
	}
#endif
	return 0;
}


// Initialize the property set interface.
//
// returns: 0 if successful, otherwise -1.  If successful, the global pPropertySet will
//          set to a non-NULL value.
//
int ds_init_property_set(DWORD sample_rate, WORD sample_bits)
{
#ifndef USE_OPENAL
	HRESULT hr;

	// Create the secondary buffer required for EAX initialization
	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.nSamplesPerSec = sample_rate;
	wf.wBitsPerSample = sample_bits;
	wf.cbSize = 0;
	wf.nBlockAlign = (unsigned short)(wf.nChannels * (wf.wBitsPerSample / 8));
	wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;

	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	dsbd.dwBufferBytes = 3 * wf.nAvgBytesPerSec;
	dsbd.lpwfxFormat = &wf;

	// Create a new buffer using the settings for this wave
	hr = pDirectSound->CreateSoundBuffer(&dsbd, &Ds_property_set_pdsb, NULL);
	if (FAILED(hr)) {
		pPropertySet = NULL;
		return -1;
	}

	// Get the 3D interface from the secondary buffer, which is used to query the EAX interface
	hr = Ds_property_set_pdsb->QueryInterface(IID_IDirectSound3DBuffer, (void**)&Ds_property_set_pds3db);
	if (FAILED(hr)) {
		Ds_property_set_pds3db = NULL;
		return -1;
	}

	Assert(Ds_property_set_pds3db != NULL);
	hr = Ds_property_set_pds3db->QueryInterface(IID_IKsPropertySet, (void**)&pPropertySet);
	if ((FAILED(hr)) || (pPropertySet == NULL)) {
		return -1;
	}
#endif
	return 0;
}

#ifdef USE_OPENAL
const char *openal_get_best_device(int report = 1)
{
	int ext_length = 0;
	// FIXME: this is crashing for some reason, a NULL device should be legal here, but who knows
	const char *my_default_device = (const char*) alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
	const char *my_devices = (const char*) alcGetString( NULL, ALC_DEVICE_SPECIFIER );

	if (report) {
		mprintf(("  Default OpenAL device: %s\n", (my_default_device != NULL) ? my_default_device : NOX("<none>")));

		char *str_list = (char*)my_devices;

		mprintf(("  Available OpenAL devices:\n"));

		if ( (str_list != NULL) && ((ext_length = strlen(str_list)) > 0) ) {
			while (ext_length) {
				mprintf(("    %s\n", str_list));
				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		} else {
			mprintf(("    <none>\n"));
		}

		mprintf(("\n"));
	}

	return my_default_device;
}
#endif

// ---------------------------------------------------------------------------------------
// ds_init()
//
// returns:     -1           => init failed
//               0           => init success
int ds_init(int use_a3d, int use_eax, unsigned int sample_rate, unsigned short sample_bits)
{
#ifdef USE_OPENAL
//	NOTE: A3D and EAX are unused in OpenAL
	int attr[] = { ALC_FREQUENCY, sample_rate, ALC_SYNC, AL_FALSE, 0 };
	ALfloat list_orien[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };

	Ds_use_a3d = 0;
	Ds_use_eax = 0;
//	Ds_use_ds3d = 1;
	Ds_use_ds3d = 0;

	mprintf(("Initializing OpenAL...\n"));

	// FIXME: see function for problem!
//	openal_get_best_device();

	// version check (for 1.0 or 1.1)
	alcGetIntegerv(NULL, ALC_MINOR_VERSION, sizeof(ALCint), &AL_minor_version);

	// we need to clear out all errors before moving on
	alcGetError(NULL);
	alGetError();

	// load OpenAL
#ifdef _WIN32
	// we require OpenAL 1.1 on Windows, so version check it
	if (!AL_minor_version) {
		MessageBox(NULL, "OpenAL 1.1 or newer is required for proper operation.  Please upgrade your OpenAL drivers, which\nare available at http://www.openal.org/downloads.html, and try running the game again.", NULL, MB_OK);
		return -2;
	}

	// restrict to software rather than hardware (the default) devices here by default since
	// we may have 'too many hardware sources' type problems otherwise - taylor
	char *device_spec = os_config_read_string( NULL, "SoundDeviceOAL", "Generic Software" );
	mprintf(("  Using '%s' as OpenAL sound device...\n", device_spec));

	ds_sound_device = alcOpenDevice( (const ALCchar *) device_spec );
#else
	ds_sound_device = alcOpenDevice( NULL );
#endif

	if ( !ds_sound_device )
		goto AL_InitError;

	// Create Sound Device
	OpenAL_C_ErrorCheck( { ds_sound_context = alcCreateContext( ds_sound_device, attr ); }, goto AL_InitError );

	// set the new context as current
	OpenAL_C_ErrorCheck( alcMakeContextCurrent( ds_sound_context ), goto AL_InitError );

	mprintf(( "  OpenAL Vendor     : %s\n", alGetString( AL_VENDOR ) ));
	mprintf(( "  OpenAL Renderer   : %s\n", alGetString( AL_RENDERER ) ));
	mprintf(( "  OpenAL Version    : %s\n", alGetString( AL_VERSION ) ));
	mprintf(( "\n" ));

	// make sure we can actually use AL_BYTE_LOKI (Mac/Win OpenAL doesn't have it)
	AL_play_position = alIsExtensionPresent( "AL_LOKI_play_position" );

	if (AL_play_position)
		mprintf(( "  Using extension \"AL_LOKI_play_position\".\n" ));

	// not a big deal here, but for consitancy sake
	if (Ds_use_ds3d && ds3d_init(0) != 0)
		Ds_use_ds3d = 0;

	// setup default listener position/orientation
	// this is needed for 2D pan
	OpenAL_ErrorPrint( alListener3f(AL_POSITION, 0.0, 0.0, 0.0) );
	OpenAL_ErrorPrint( alListenerfv(AL_ORIENTATION, list_orien) );

	ds_build_vol_lookup();
	ds_init_channels();
	ds_init_buffers();

	// we need to clear out all errors before moving on
	alcGetError(NULL);
	alGetError();

	mprintf(("... OpenAL successfully initialized!\n"));

	return 0;


AL_InitError:
	alcMakeContextCurrent(NULL);

	if (ds_sound_context != NULL) {
		alcDestroyContext(ds_sound_context);
		ds_sound_context = NULL;
	}

	if (ds_sound_device != NULL) {
		alcCloseDevice(ds_sound_device);
		ds_sound_device = NULL;
	}

	return -1;
#else
	HRESULT			hr;
	HWND				hwnd;
	WAVEFORMATEX	wave_format;
	DSBUFFERDESC	BufferDesc;

	nprintf(( "Sound", "SOUND ==> Initializing DirectSound...\n" ));

	hwnd = (HWND)os_get_window();
	if ( hwnd == NULL )	{
		nprintf(( "Sound", "SOUND ==> No window handle, so no sound...\n" ));
		return -1;
	}

	if ( ds_dll_load() == -1 ) {
		return -1;
	}

	pDirectSound = NULL;

	Ds_use_eax = use_eax;

	if (Ds_use_eax) {
		Ds_use_ds3d = 1;
	}

	/*
	if (Ds_use_eax) {
		Ds_use_eax = 0;
	}
	*/	
	
	if (!pfn_DirectSoundCreate) {
		nprintf(( "Sound", "SOUND ==> Could not get DirectSoundCreate function pointer\n" ));
		return -1;
	}

	hr = pfn_DirectSoundCreate(NULL, &pDirectSound, NULL);
	if (FAILED(hr)) {
		return -1;
	}

	// Set up DirectSound for exclusive mode, so we can change the primary buffer if we want to.	
	hr = pDirectSound->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> DirectSound pDirectSound->SetCooperativeLevel failed with code %s\n.",get_DSERR_text(hr) ));
		pDirectSound = NULL;	
		return -1;
	}

	// Create the primary buffer
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.dwSize = sizeof(BufferDesc);

	ds_get_soundcard_caps(&Soundcard_caps);

	if (Ds_use_ds3d) {
		BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

		hr = pDirectSound->CreateSoundBuffer(&BufferDesc, &pPrimaryBuffer, 0);
		if (hr != DS_OK) {
			nprintf(("Sound","SOUND ==> Primary Buffer create failed with DSBCAPS_CTRL3D property... disabling DirectSound3D\n"));
			Ds_use_ds3d = 0;
			Ds_use_eax = 0;			
		} else {
			nprintf(("Sound","SOUND ==> Primary Buffer created with DirectSound3D enabled\n"));
		}
	}

	// If not using DirectSound3D, then create a normal primary buffer
	if (Ds_use_ds3d == 0) {
		BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		hr = pDirectSound->CreateSoundBuffer(&BufferDesc, &pPrimaryBuffer, 0);
		if (hr != DS_OK) {
			nprintf(("Sound","SOUND ==> Primary Buffer create failed with error: %s\n",get_DSERR_text(hr) ));
			pDirectSound = NULL;	
			return -1;
		}
		else {
			nprintf(("Sound","SOUND ==> Primary Buffer created with without DirectSound3D enabled\n"));
		}
	}

	// Get the primary buffer format
	ds_get_primary_format(&wave_format, sample_rate, sample_bits);

	hr = pPrimaryBuffer->SetFormat(&wave_format);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> pPrimaryBuffer->SetFormat() failed with code %s\n",get_DSERR_text(hr) ));
	}

	pPrimaryBuffer->GetFormat(&wave_format, sizeof(wave_format), NULL);
	nprintf(("Sound","SOUND ==> Primary Buffer forced to: rate: %d Hz bits: %d n_channels: %d\n",
			wave_format.nSamplesPerSec, wave_format.wBitsPerSample, wave_format.nChannels));

	// start the primary buffer playing.  This will reduce sound latency when playing a sound
	// if no other sounds are playing.
	hr = pPrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> pPrimaryBuffer->Play() failed with code %s\n",get_DSERR_text(hr) ));
	}

	// Initialize DirectSound3D.  Since software performance of DirectSound3D is unacceptably
	// slow, we require the voice manger (a DirectSound extension) to be present.  The 
	// exception is when A3D is being used, since A3D has a resource manager built in.
	if (Ds_use_ds3d) {
		int vm_required = 1;	// voice manager		

		if (ds3d_init(vm_required) != 0) {
			Ds_use_ds3d = 0;
			//Ds_use_eax = 0;
		}
	}

	if (Ds_use_eax == 1) {
		ds_init_property_set(sample_rate, sample_bits);
		if (ds_eax_init() != 0) {
			Ds_use_eax = 0;
		}
	}

	ds_build_vol_lookup();
	ds_init_channels();
	ds_init_buffers();

	ds_show_caps(&Soundcard_caps);

	return 0;
#endif
}

// ---------------------------------------------------------------------------------------
// get_DSERR_text()
//
// returns the text equivalent for the a DirectSound DSERR_ code
//
char *get_DSERR_text(int DSResult)
{
#ifdef USE_OPENAL
	STUB_FUNCTION;

	return "unknown";
#else
	switch( DSResult ) {

		case DS_OK:
			return "DS_OK";
			break;

		case DSERR_ALLOCATED:
			return "DSERR_ALLOCATED";
			break;

		case DSERR_ALREADYINITIALIZED:
			return "DSERR_ALREADYINITIALIZED";
			break;

		case DSERR_BADFORMAT:
			return "DSERR_BADFORMAT";
			break;

		case DSERR_BUFFERLOST:
			return "DSERR_BUFFERLOST";
			break;

		case DSERR_CONTROLUNAVAIL:
			return "DSERR_CONTROLUNAVAIL";
			break;

		case DSERR_GENERIC:
			return "DSERR_GENERIC";
			break;

		case DSERR_INVALIDCALL:
			return "DSERR_INVALIDCALL";
			break;

		case DSERR_INVALIDPARAM:
			return "DSERR_INVALIDPARAM";
			break;

		case DSERR_NOAGGREGATION:
			return "DSERR_NOAGGREGATION";
			break;

		case DSERR_NODRIVER:
			return "DSERR_NODRIVER";
			break;

		case DSERR_OUTOFMEMORY:
			return "DSERR_OUTOFMEMORY";
			break;

		case DSERR_OTHERAPPHASPRIO:
			return "DSERR_OTHERAPPHASPRIO";
			break;

		case DSERR_PRIOLEVELNEEDED:
			return "DSERR_PRIOLEVELNEEDED";
			break;

		case DSERR_UNINITIALIZED:
			return "DSERR_UNINITIALIZED";
			break;

		case DSERR_UNSUPPORTED:
			return "DSERR_UNSUPPORTED";
			break;

		default:
			return "unknown";
			break;
	}
#endif
}


// ---------------------------------------------------------------------------------------
// ds_close_channel()
//
// Free a single channel
//
void ds_close_channel(int i)
{
#ifdef USE_OPENAL
	if ( (Channels[i].source_id != 0) && alIsSource(Channels[i].source_id) ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[i].source_id) );

		OpenAL_ErrorPrint( alDeleteSources(1, &Channels[i].source_id) );

		Channels[i].source_id = 0;
		Channels[i].buf_id = -1;
		Channels[i].sig = -1;
		Channels[i].snd_id = -1;
	}

	return;
#else
	HRESULT	hr;

	// If a 3D interface exists, free it
	if ( Channels[i].pds3db != NULL ) {

		{
			int attempts = 0;
			while(++attempts < 10) {
				hr = Channels[i].pds3db->Release();
				if ( hr == DS_OK ) {
					break;
				} else {
					// nprintf(("Sound", "SOUND ==> Channels[channel].pds3db->Release() failed with return value %s\n", get_DSERR_text(second_hr) ));
				}
			}

			Channels[i].pds3db = NULL;
		}
	}

	if ( Channels[i].pdsb != NULL ) {
		// If a 2D interface exists, free it
		if ( Channels[i].pdsb != NULL ) {
			int attempts = 0;
			while(++attempts < 10) {
				hr = Channels[i].pdsb->Release();
				if ( hr == DS_OK ) {
					break;
				} else {
					nprintf(("Sound", "SOUND ==> Channels[channel].pdsb->Release() failed with return value %s\n", get_DSERR_text(hr) ));
				}
			}
		}

		Channels[i].pdsb = NULL;
	}
#endif
}



// ---------------------------------------------------------------------------------------
// ds_close_all_channels()
//
// Free all the channel buffers
//
void ds_close_all_channels()
{
	int		i;

	for (i = 0; i < MAX_CHANNELS; i++)	{
		ds_close_channel(i);
	}
}

// ---------------------------------------------------------------------------------------
// ds_unload_buffer()
//
//
void ds_unload_buffer(int sid, int hid)
{
#ifdef USE_OPENAL
	if (sid != -1) {
		ALuint buf_id = sound_buffers[sid].buf_id;
		int channel_idx = sound_buffers[sid].source_id;

		if (channel_idx != -1)
			ds_close_channel(channel_idx);

		if ( (buf_id != 0) && alIsBuffer(buf_id) )
			OpenAL_ErrorPrint( alDeleteBuffers(1, &buf_id) );

		sound_buffers[sid].buf_id = 0;
		sound_buffers[sid].source_id = -1;
	}

	/* hid unused */

	return;
#else
	HRESULT	hr;

	if ( sid != -1 ) {
		if ( ds_software_buffers[sid].pdsb != NULL ) {
			hr = ds_software_buffers[sid].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_software_buffers[sid]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_software_buffers[sid].pdsb = NULL;
		}
	}

	if ( hid != -1 ) {
		if ( ds_hardware_buffers[hid].pdsb != NULL ) {
			hr = ds_hardware_buffers[hid].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_hardware_buffers[hid]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_hardware_buffers[hid].pdsb = NULL;
		}
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_close_software_buffers()
//
//
void ds_close_software_buffers()
{
#ifdef USE_OPENAL
	uint i;

	for (i = 0; i < sound_buffers.size(); i++) {
		ALuint buf_id = sound_buffers[i].buf_id;

		if ( (buf_id != 0) && alIsBuffer(buf_id) ) {
			OpenAL_ErrorPrint( alDeleteBuffers(1, &buf_id) );
		}
	}

	sound_buffers.clear();
#else
	int		i;
	HRESULT	hr;

	for (i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++)	{
		if ( ds_software_buffers[i].pdsb != NULL ) {
			hr = ds_software_buffers[i].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_software_buffers[i]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_software_buffers[i].pdsb = NULL;
		}
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_close_hardware_buffers()
//
//
void ds_close_hardware_buffers()
{
#ifndef USE_OPENAL
	int		i;
	HRESULT	hr;

	for (i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++)	{
		if ( ds_hardware_buffers[i].pdsb != NULL ) {
			hr = ds_hardware_buffers[i].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_hardware_buffers[i]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_hardware_buffers[i].pdsb = NULL;
		}
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_close_buffers()
//
// Free the channel buffers
//
void ds_close_buffers()
{
	ds_close_software_buffers();
	ds_close_hardware_buffers();
}

// ---------------------------------------------------------------------------------------
// ds_close()
//
// Close the DirectSound system
//
void ds_close()
{
#ifndef USE_OPENAL
	ds_close_all_channels();
	ds_close_buffers();
#else
	ds_close_buffers();
	ds_close_all_channels();
#endif

#ifndef USE_OPENAL
	if (pPropertySet != NULL) {
		pPropertySet->Release();
		pPropertySet = NULL;
	}

	if (Ds_property_set_pdsb != NULL) {
		Ds_property_set_pdsb->Release();
		Ds_property_set_pdsb = NULL;
	}

	if (Ds_property_set_pds3db != NULL) {
		Ds_property_set_pds3db->Release();
		Ds_property_set_pds3db = NULL;
	}

	if (pPrimaryBuffer)	{
		pPrimaryBuffer->Release();
		pPrimaryBuffer = NULL;
	}

	if (pDirectSound)	{
		pDirectSound->Release();
		pDirectSound = NULL;
	}

	if ( Ds_dll_loaded ) {
		FreeLibrary(Ds_dll_handle);
		Ds_dll_loaded=0;
	}

	if (Ds_must_call_couninitialize == 1) {
		CoUninitialize();
	}
#endif
	// free the Channels[] array, since it was dynamically allocated
	vm_free(Channels);
	Channels = NULL;

#ifdef USE_OPENAL
	alcMakeContextCurrent(NULL);	// hangs on me for some reason

	if (ds_sound_context != NULL)
		alcDestroyContext(ds_sound_context);

	if (ds_sound_device != NULL)
		alcCloseDevice(ds_sound_device);
#endif
}

// ---------------------------------------------------------------------------------------
// ds_get_3d_interface()
// 
// Get the 3d interface for a secondary buffer. 
//
// If the secondary buffer wasn't created with a DSBCAPS_CTRL3D flag, then no 3d interface
// exists
//
#ifndef USE_OPENAL
void ds_get_3d_interface(LPDIRECTSOUNDBUFFER pdsb, LPDIRECTSOUND3DBUFFER *ppds3db)
{
	DSBCAPS			dsbc;
	HRESULT			DSResult;

	dsbc.dwSize = sizeof(dsbc);
	DSResult = pdsb->GetCaps(&dsbc);
	if ( DSResult == DS_OK && dsbc.dwFlags & DSBCAPS_CTRL3D ) {
		DSResult = pdsb->QueryInterface( IID_IDirectSound3DBuffer, (void**)ppds3db );
		if ( DSResult != DS_OK ) {
			nprintf(("SOUND","Could not obtain 3D interface for hardware buffer: %s\n", get_DSERR_text(DSResult) ));
		}
	}
}
#endif


// ---------------------------------------------------------------------------------------
// ds_get_free_channel()
// 
// Find a free channel to play a sound on.  If no free channels exists, free up one based
// on volume levels.
//
//	input:		new_volume	=>		volume in DS units for sound to play at
//					snd_id		=>		which kind of sound to play
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//
//	returns:		channel number to play sound on
//					-1 if no channel could be found
//
// NOTE:	snd_id is needed since we limit the number of concurrent samples
//
//
#define DS_MAX_SOUND_INSTANCES 2

int ds_get_free_channel(int new_volume, int snd_id, int priority)
{
#ifdef USE_OPENAL
	int				i, first_free_channel, limit;
	int				lowest_vol = 0, lowest_vol_index = -1;
	int				instance_count;	// number of instances of sound already playing
	int				lowest_instance_vol, lowest_instance_vol_index;
	channel			*chp;
	int status;

	instance_count = 0;
	lowest_instance_vol = 99;
	lowest_instance_vol_index = -1;
	first_free_channel = -1;

	// Look for a channel to use to play this sample
	for ( i = 0; i < MAX_CHANNELS; i++ )	{
		chp = &Channels[i];

		if ( chp->source_id == 0 ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;

			continue;
		}

		OpenAL_ErrorCheck( alGetSourcei(chp->source_id, AL_SOURCE_STATE, &status), continue );

		if ( status != AL_PLAYING ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;

			ds_close_channel(i);

			continue;
		}
		else {
			if ( chp->snd_id == snd_id ) {
				instance_count++;
				if ( chp->vol < lowest_instance_vol && chp->looping == FALSE ) {
					lowest_instance_vol = chp->vol;
					lowest_instance_vol_index = i;
				}
			}

			if ( chp->vol < lowest_vol && chp->looping == FALSE ) {
				lowest_vol_index = i;
				lowest_vol = chp->vol;
			}
		}
	}

	// determine the limit of concurrent instances of this sound
	switch(priority) {
		case DS_MUST_PLAY:
			limit = 100;
			break;
		case DS_LIMIT_ONE:
			limit = 1;
			break;
		case DS_LIMIT_TWO:
			limit = 2;
			break;
		case DS_LIMIT_THREE:
			limit = 3;
			break;
		default:
			Int3();			// get Alan
			limit = 100;
			break;
	}


	// If we've exceeded the limit, then maybe stop the duplicate if it is lower volume
	if ( instance_count >= limit ) {
		// If there is a lower volume duplicate, stop it.... otherwise, don't play the sound
		if ( lowest_instance_vol_index >= 0 && (Channels[lowest_instance_vol_index].vol <= new_volume) ) {
			ds_close_channel(lowest_instance_vol_index);
			first_free_channel = lowest_instance_vol_index;
		} else {
			first_free_channel = -1;
		}
	} else {
		// there is no limit barrier to play the sound, so see if we've ran out of channels
		if ( first_free_channel == -1 ) {
			// stop the lowest volume instance to play our sound if priority demands it
			if ( lowest_vol_index != -1 && priority == DS_MUST_PLAY ) {
				// Check if the lowest volume playing is less than the volume of the requested sound.
				// If so, then we are going to trash the lowest volume sound.
				if ( Channels[lowest_vol_index].vol <= new_volume ) {
					ds_close_channel(lowest_vol_index);
					first_free_channel = lowest_vol_index;
				}
			}
		}
	}

	if ( (first_free_channel >= 0) && (Channels[first_free_channel].source_id == 0) )
		OpenAL_ErrorCheck( alGenSources(1, &Channels[first_free_channel].source_id), return -1 );

	return first_free_channel;

#else

	int				i, first_free_channel, limit;
	int				lowest_vol = 0, lowest_vol_index = -1;
	int				instance_count;	// number of instances of sound already playing
	int				lowest_instance_vol, lowest_instance_vol_index;
	unsigned long	status;
	HRESULT			hr;
	channel			*chp;

	instance_count = 0;
	lowest_instance_vol = 99;
	lowest_instance_vol_index = -1;
	first_free_channel = -1;

	// Look for a channel to use to play this sample
	for ( i = 0; i < MAX_CHANNELS; i++ )	{
		chp = &Channels[i];
		if ( chp->pdsb == NULL ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;
			continue;
		}

		hr = chp->pdsb->GetStatus(&status);
		if ( hr != DS_OK ) {
			nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
			return -1;
		}
		if ( !(status & DSBSTATUS_PLAYING) ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;
			ds_close_channel(i);
			continue;
		}
		else {
			if ( chp->snd_id == snd_id ) {
				instance_count++;
				if ( chp->vol < lowest_instance_vol && chp->looping == FALSE ) {
					lowest_instance_vol = chp->vol;
					lowest_instance_vol_index = i;
				}
			}

			if ( chp->vol < lowest_vol && chp->looping == FALSE ) {
				lowest_vol_index = i;
				lowest_vol = chp->vol;
			}
		}
	}

	// determine the limit of concurrent instances of this sound
	switch(priority) {
		case DS_MUST_PLAY:
			limit = 100;
			break;
		case DS_LIMIT_ONE:
			limit = 1;
			break;
		case DS_LIMIT_TWO:
			limit = 2;
			break;
		case DS_LIMIT_THREE:
			limit = 3;
			break;
		default:
			Int3();			// get Alan
			limit = 100;
			break;
	}


	// If we've exceeded the limit, then maybe stop the duplicate if it is lower volume
	if ( instance_count >= limit ) {
		// If there is a lower volume duplicate, stop it.... otherwise, don't play the sound
		if ( lowest_instance_vol_index >= 0 && (Channels[lowest_instance_vol_index].vol <= new_volume) ) {
			ds_close_channel(lowest_instance_vol_index);
			first_free_channel = lowest_instance_vol_index;
		} else {
			first_free_channel = -1;
		}
	} else {
		// there is no limit barrier to play the sound, so see if we've ran out of channels
		if ( first_free_channel == -1 ) {
			// stop the lowest volume instance to play our sound if priority demands it
			if ( lowest_vol_index != -1 && priority == DS_MUST_PLAY ) {
				// Check if the lowest volume playing is less than the volume of the requested sound.
				// If so, then we are going to trash the lowest volume sound.
				if ( Channels[lowest_vol_index].vol <= new_volume ) {
					ds_close_channel(lowest_vol_index);
					first_free_channel = lowest_vol_index;
				}
			}
		}
	}

	return first_free_channel;
#endif
}


// ---------------------------------------------------------------------------------------
// ds_channel_dup()
// 
// Find a free channel to play a sound on.  If no free channels exists, free up one based
// on volume levels.
//
// returns:		0		=>		dup was successful
//					-1		=>		dup failed (Channels[channel].pdsb will be NULL)
//
#ifndef USE_OPENAL
int ds_channel_dup(LPDIRECTSOUNDBUFFER pdsb, int channel, int use_ds3d)
{
	HRESULT DSResult;

	// Duplicate the master buffer into a channel buffer.
	DSResult = pDirectSound->DuplicateSoundBuffer(pdsb, &Channels[channel].pdsb );
	if ( DSResult != DS_OK ) {
		nprintf(("Sound", "SOUND ==> DuplicateSoundBuffer failed with return value %s\n", get_DSERR_text(DSResult) ));
		Channels[channel].pdsb = NULL;
		return -1;
	}

	// get the 3d interface for the buffer if it exists
	if ( use_ds3d ) {
		if (Channels[channel].pds3db == NULL) {
			ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
		}
	}
	
	return 0;
}


// ---------------------------------------------------------------------------------------
// ds_restore_buffer()
// 
//
void ds_restore_buffer(LPDIRECTSOUNDBUFFER pdsb)
{
	HRESULT hr;
	
	Int3();	// get Alan, he wants to see this
	hr = pdsb->Restore();
	if ( hr != DS_OK ) {
		nprintf(("Sound", "Sound ==> Lost a buffer, tried restoring but got %s\n", get_DSERR_text(hr) ));
	}
}
#endif

// Create a direct sound buffer in software, without locking any data in
int ds_create_buffer(int frequency, int bits_per_sample, int nchannels, int nseconds)
{
#ifdef USE_OPENAL
	ALuint i;
	int sid;

	if (!ds_initialized) {
		return -1;
	}

	sid = ds_get_sid();
	if ( sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more OpenAL buffers available\n"));
		return -1;
	}

	OpenAL_ErrorCheck( alGenBuffers(1, &i), return -1 );
	
	sound_buffers[sid].buf_id = i;
	sound_buffers[sid].source_id = -1;
	sound_buffers[sid].frequency = frequency;
	sound_buffers[sid].bits_per_sample = bits_per_sample;
	sound_buffers[sid].nchannels = nchannels;
	sound_buffers[sid].nseconds = nseconds;
	sound_buffers[sid].nbytes = nseconds * (bits_per_sample / 8) * nchannels * frequency;

	return sid;
#else
	HRESULT			dsrval;
	DSBUFFERDESC	dsbd;
	WAVEFORMATEX	wfx;
	int				sid;

	if (!ds_initialized) {
		return -1;
	}

	sid = ds_get_sid();
	if ( sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more software secondary buffers available\n"));
		return -1;
	}

	// Set up buffer format
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (unsigned short)nchannels;
	wfx.nSamplesPerSec = frequency;
	wfx.wBitsPerSample = (unsigned short)bits_per_sample;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (unsigned short)(wfx.nChannels * (wfx.wBitsPerSample / 8));
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwBufferBytes = wfx.nAvgBytesPerSec * nseconds;
	dsbd.lpwfxFormat = &wfx;
	dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLDEFAULT;

	dsrval = pDirectSound->CreateSoundBuffer(&dsbd, &ds_software_buffers[sid].pdsb, NULL);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	ds_software_buffers[sid].desc = dsbd;
	return sid;
#endif
}

// Lock data into an existing buffer
int ds_lock_data(int sid, unsigned char *data, int size)
{
#ifdef USE_OPENAL
	STUB_FUNCTION;
/*
	Assert(sid >= 0);

	ALuint buf_id = sound_buffers[sid].buf_id;
	ALenum format;

	if (sound_buffers[sid].bits_per_sample == 16) {
		if (sound_buffers[sid].nchannels == 2) {
			format = AL_FORMAT_STEREO16;
		} else if (sound_buffers[sid].nchannels == 1) {
			format = AL_FORMAT_MONO16;
		} else {
			return -1;
		}
	} else if (sound_buffers[sid].bits_per_sample == 8) {
		if (sound_buffers[sid].nchannels == 2) {
			format = AL_FORMAT_STEREO8;
		} else if (sound_buffers[sid].nchannels == 1) {
			format = AL_FORMAT_MONO8;
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	sound_buffers[sid].nbytes = size;

	OpenAL_ErrorCheck( alBufferData(buf_id, format, data, size, sound_buffers[sid].frequency), return -1 );
*/
	return 0;
#else
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;
	void						*buffer_data, *buffer_data2;
	DWORD						buffer_size, buffer_size2;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	pdsb->SetCurrentPosition(0);

	// lock the entire buffer
	dsrval = pdsb->Lock(0, caps.dwBufferBytes, &buffer_data, &buffer_size, &buffer_data2, &buffer_size2, 0 );
	if ( dsrval != DS_OK ) {
		return -1;
	}

	// first clear it out with silence
	memset(buffer_data, 0x80, buffer_size);
	memcpy(buffer_data, data, size);

	dsrval = pdsb->Unlock(buffer_data, buffer_size, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
#endif
}

// Stop a buffer from playing directly
void ds_stop_easy(int sid)
{
#ifdef USE_OPENAL
	Assert(sid >= 0);

	int cid = sound_buffers[sid].source_id;

	if (cid != -1) {
		ALuint source_id = Channels[cid].source_id;

		OpenAL_ErrorPrint( alSourceStop(source_id) );
	}
#else
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;
	dsrval = pdsb->Stop();
#endif
}

//	Play a sound without the usual baggage (used for playing back real-time voice)
//
// parameters:  
//					sid			=> software id of sound
//					volume      => volume of sound effect in DirectSound units
int ds_play_easy(int sid, int volume)
{
#ifdef USE_OPENAL
	if (!ds_initialized)
		return -1;

	int ch_idx = ds_get_free_channel(volume, -1, DS_MUST_PLAY);

	if (ch_idx < 0)
		return -1;

	ALuint source_id = Channels[ch_idx].source_id;

	OpenAL_ErrorPrint( alSourceStop(source_id) );

	if (Channels[ch_idx].buf_id != sid) {
		ALuint buffer_id = sound_buffers[sid].buf_id;

		OpenAL_ErrorCheck( alSourcei(source_id, AL_BUFFER, buffer_id), return -1 );
	}

	Channels[ch_idx].buf_id = sid;

	ALfloat alvol = (volume != -10000) ? powf(10.0f, (float)volume / (-600.0f / log10f(.5f))): 0.0f;

	OpenAL_ErrorPrint( alSourcef(source_id, AL_GAIN, alvol) );

	OpenAL_ErrorPrint( alSourcei(source_id, AL_LOOPING, AL_FALSE) );

	OpenAL_ErrorPrint( alSourcePlay(source_id) );

	return 0;

#else

	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	pdsb->SetVolume(volume);
	dsrval=pdsb->Play(0, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
#endif
}

//extern void HUD_add_to_scrollback(char *text, int source);
//extern void HUD_printf(char *format, ...);

// ---------------------------------------------------------------------------------------
// Play a DirectSound secondary buffer.  
// 
//
// parameters:  
//					sid			=> software id of sound
//					hid			=> hardware id of sound ( -1 if not in hardware )
//					snd_id		=>	what kind of sound this is
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//					volume      => volume of sound effect in DirectSound units
//					pan         => pan of sound in DirectSound units
//             looping     => whether the sound effect is looping or not
//
// returns:    -1          => sound effect could not be started
//              >=0        => sig for sound effect successfully started
//
int ds_play(int sid, int hid, int snd_id, int priority, int volume, int pan, int looping, bool is_voice_msg)
{
#ifdef USE_OPENAL
	int ch_idx;

	if (!ds_initialized)
		return -1;

	ch_idx = ds_get_free_channel(volume, snd_id, priority);

	if (ch_idx < 0) {
//		nprintf(( "Sound", "SOUND ==> Not playing sound requested at volume %.2f\n", ds_get_percentage_vol(volume) ));
		return -1;
	}

	if (Channels[ch_idx].source_id == 0)
		return -1;

	if ( ds_using_ds3d() ) { }

	// set new position for pan or zero out if none
	ALfloat alpan = (float)pan / MAX_PAN;

	if ( alpan ) {
		OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, alpan, 0.0, 1.0) );
	} else {
		OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, 0.0, 0.0, 0.0) );
	}

	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_VELOCITY, 0.0, 0.0, 0.0) );

	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_PITCH, 1.0) );

	ALfloat alvol = (volume != -10000) ? powf(10.0f, (float)volume / (-600.0f / log10f(.5f))): 0.0f;
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_GAIN, alvol) );

	ALint status;
	OpenAL_ErrorCheck( alGetSourcei(Channels[ch_idx].source_id, AL_SOURCE_STATE, &status), return -1 );
		
	if (status == AL_PLAYING)
		OpenAL_ErrorPrint( alSourceStop(Channels[ch_idx].source_id) );


	OpenAL_ErrorCheck( alSourcei(Channels[ch_idx].source_id, AL_BUFFER, sound_buffers[sid].buf_id), return -1 );

	// setup default listener position/orientation
	// this is needed for 2D pan
	OpenAL_ErrorPrint( alListener3f(AL_POSITION, 0.0, 0.0, 0.0) );

	ALfloat list_orien[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	OpenAL_ErrorPrint( alListenerfv(AL_ORIENTATION, list_orien) );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_SOURCE_RELATIVE, AL_FALSE) );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	OpenAL_ErrorPrint( alSourcePlay(Channels[ch_idx].source_id) );

	sound_buffers[sid].source_id = ch_idx;

	Channels[ch_idx].buf_id = sid;
	Channels[ch_idx].snd_id = snd_id;
	Channels[ch_idx].sig = channel_next_sig++;
	Channels[ch_idx].last_position = 0;
	Channels[ch_idx].is_voice_msg = is_voice_msg;
	Channels[ch_idx].vol = volume;
	Channels[ch_idx].looping = looping;
	Channels[ch_idx].priority = priority;

	if (channel_next_sig < 0)
		channel_next_sig = 1;

	return Channels[ch_idx].sig;

#else

	int				channel;
	HRESULT			DSResult;

	if (!ds_initialized)
		return -1;

	channel = ds_get_free_channel(volume, snd_id, priority);

	if (channel > -1)	{
		if ( Channels[channel].pdsb != NULL ) {
			return -1;
		}

		// First check if the sound is in hardware, and try to duplicate from there
		if ( hid != -1 ) {
			Int3();
			if ( ds_channel_dup(ds_hardware_buffers[hid].pdsb, channel, 0) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound in hardware..\n"));
			}
		}

		// Channel will be NULL if hardware dup failed, or there was no hardware dup attempted
		if ( Channels[channel].pdsb == NULL ) {
			if ( ds_channel_dup(ds_software_buffers[sid].pdsb, channel, 0) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound in software..\n"));
			}
		}
	
		if ( Channels[channel].pdsb == NULL ) {
			return -1;
		}

		if ( ds_using_ds3d() ) {
			if ( ds_is_3d_buffer(Channels[channel].pdsb) ) {
				if (Channels[channel].pds3db == NULL) {
					ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
				}
				if ( Channels[channel].pds3db ) {
					Channels[channel].pds3db->SetMode(DS3DMODE_DISABLE,DS3D_IMMEDIATE);
				}
			}
		}

		// Actually play it
		Channels[channel].vol = volume;
		Channels[channel].looping = looping;
		Channels[channel].priority = priority;
    	Channels[channel].pdsb->SetPan(pan);
		Channels[channel].pdsb->SetVolume(volume);
		Channels[channel].is_voice_msg = is_voice_msg;

		int ds_flags = 0;
		if ( looping )
			ds_flags |= DSBPLAY_LOOPING;
		
		DSResult = Channels[channel].pdsb->Play(0, 0, ds_flags );

		/*
		if (Stop_logging_sounds == false) {
			char buf[256];
			sprintf(buf, "channel %d, address: %x, ds_flags: %d", channel, Channels[channel].pdsb, ds_flags);
			HUD_add_to_scrollback(buf, 3);
		}
		*/

		if ( DSResult == DSERR_BUFFERLOST ) {
			ds_restore_buffer(Channels[channel].pdsb);
			DSResult = Channels[channel].pdsb->Play(0, 0, ds_flags );
		}

		if ( DSResult != DS_OK ) {
			nprintf(("Sound", "Sound ==> Play failed with return value %s\n", get_DSERR_text(DSResult) ));
			return -1;
		}
	}
	else {
//		nprintf(( "Sound", "SOUND ==> Not playing sound requested at volume %.2f\n", ds_get_percentage_vol(volume) ));
		return -1;
	}

	Channels[channel].snd_id = snd_id;
	Channels[channel].sig = channel_next_sig++;
	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}

	/*
	if (Stop_logging_sounds == false) {
		if (is_voice_msg) {
			char buf[256];
			sprintf(buf, "VOICE sig: %d, sid: %d, snd_id: %d, ch: %d", Channels[channel].sig, sid, snd_id, channel);
			HUD_add_to_scrollback(buf, 3);
		}
	}
	*/

	Channels[channel].last_position = 0;

	// make sure there aren't any looping voice messages
	for (int i=0; i<MAX_CHANNELS; i++) {
		if (Channels[i].is_voice_msg == true) {
			if (Channels[i].pdsb == NULL) {
				continue;
			}

			DWORD current_position = ds_get_play_position(i);
			if (current_position != 0) {
				if (current_position < Channels[i].last_position) {
					ds_close_channel(i);
				} else {
					Channels[i].last_position = current_position;
				}
			}
		}
	}

	return Channels[channel].sig;
#endif
}


// ---------------------------------------------------------------------------------------
// ds_get_channel()
//
// Return the channel number that is playing the sound identified by sig.  If that sound is
// not playing, return -1.
//
int ds_get_channel(int sig)
{
#ifdef USE_OPENAL
	int i;

	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].source_id && (Channels[i].sig == sig) ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				return i;
			}
		}
	}

	return -1;
#else
	int i;

	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].pdsb && Channels[i].sig == sig ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				return i;
			}
		}
	}
	return -1;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_is_channel_playing()
//
//
int ds_is_channel_playing(int channel)
{
#ifdef USE_OPENAL
	if ( Channels[channel].source_id != 0 ) {
		ALint status;

		OpenAL_ErrorPrint( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status) );

		return (status == AL_PLAYING);
	}

	return 0;
#else
	HRESULT			hr;
	unsigned long	status;		

	if ( !Channels[channel].pdsb ) {
		return 0;
	}

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return 0;
	}

	if ( status & DSBSTATUS_PLAYING )
		return TRUE;
	else
		return FALSE;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_stop_channel()
//
//
void ds_stop_channel(int channel)
{
#ifdef USE_OPENAL
	if ( Channels[channel].source_id != 0 ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[channel].source_id) );
	}
#else
	ds_close_channel(channel);
#endif
}

// ---------------------------------------------------------------------------------------
// ds_stop_channel_all()
//
//	
void ds_stop_channel_all()
{
#ifdef USE_OPENAL
	int i;

	for ( i=0; i<MAX_CHANNELS; i++ )	{
		if ( Channels[i].source_id != 0 ) {
			OpenAL_ErrorPrint( alSourceStop(Channels[i].source_id) );
		}
	}
#else
	int i;

	for ( i=0; i<MAX_CHANNELS; i++ )	{
		if ( Channels[i].pdsb != NULL ) {
			ds_stop_channel(i);
		}
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_set_volume()
//
//	Set the volume for a channel.  The volume is expected to be in DirectSound units
//
//	If the sound is a 3D sound buffer, this is like re-establishing the maximum 
// volume.
//
void ds_set_volume( int channel, int vol )
{
#ifdef USE_OPENAL
	ALuint source_id = Channels[channel].source_id;

	if (source_id != 0) {
		ALfloat alvol = (vol != -10000) ? powf(10.0f, (float)vol / (-600.0f / log10f(.5f))): 0.0f;

		OpenAL_ErrorPrint( alSourcef(source_id, AL_GAIN, alvol) );
	}
#else
	HRESULT			hr;
	unsigned long	status;		

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( status & DSBSTATUS_PLAYING ) {
		Channels[channel].pdsb->SetVolume(vol);
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_set_pan()
//
//	Set the pan for a channel.  The pan is expected to be in DirectSound units
//
void ds_set_pan( int channel, int pan )
{
#ifdef USE_OPENAL
	ALint state;

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &state), return );

	if (state == AL_PLAYING) {
		ALfloat alpan = (pan != 0) ? ((float)pan / MAX_PAN) : 0.0f;
		OpenAL_ErrorPrint( alSource3f(Channels[channel].source_id, AL_POSITION, alpan, 0.0, 1.0) );
	}
#else
	HRESULT			hr;
	unsigned long	status;		

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( status & DSBSTATUS_PLAYING ) {
		Channels[channel].pdsb->SetPan(pan);
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_get_pitch()
//
//	Get the pitch of a channel
//
int ds_get_pitch(int channel)
{
#ifdef USE_OPENAL
	ALint status;
	ALfloat alpitch = 0;
	int pitch;

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status), return -1 );

	if (status == AL_PLAYING)
		OpenAL_ErrorPrint( alGetSourcef(Channels[channel].source_id, AL_PITCH, &alpitch) );

	// convert OpenAL values to DirectSound values and return
	pitch = fl2i( pow(10.0, (alpitch + 2.0)) );

	return pitch;
#else
	unsigned long	status, pitch = 0;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);

	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return -1;
	}

	if ( status & DSBSTATUS_PLAYING )	{
		hr = Channels[channel].pdsb->GetFrequency(&pitch);
		if ( hr != DS_OK ) {
			nprintf(("Sound", "SOUND ==> GetFrequency failed with return value %s\n", get_DSERR_text(hr) ));
			return -1;
		}
	}

	return (int)pitch;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_set_pitch()
//
//	Set the pitch of a channel
//
void ds_set_pitch(int channel, int pitch)
{
#ifdef USE_OPENAL
	ALint status;

	if ( pitch < MIN_PITCH )
		pitch = MIN_PITCH;

	if ( pitch > MAX_PITCH )
		pitch = MAX_PITCH;

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status), return );

	if (status == AL_PLAYING) {
		ALfloat alpitch = log10f((float)pitch) - 2.0f;
		OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_PITCH, alpitch) );
	}
#else
	unsigned long	status;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( pitch < MIN_PITCH )
		pitch = MIN_PITCH;

	if ( pitch > MAX_PITCH )
		pitch = MAX_PITCH;

	if ( status & DSBSTATUS_PLAYING )	{
		Channels[channel].pdsb->SetFrequency((unsigned long)pitch);
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds_chg_loop_status()
//
//	
void ds_chg_loop_status(int channel, int loop)
{
#ifdef USE_OPENAL
	ALuint source_id = Channels[channel].source_id;

	OpenAL_ErrorPrint( alSourcei(source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE) );
#else
	unsigned long	status;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}
	
	if ( !(status & DSBSTATUS_PLAYING) )
		return;		// sound is not playing anymore

	if ( status & DSBSTATUS_LOOPING ) {
		if ( loop )
			return;	// we are already looping
		else {
			// stop the sound from looping
			hr = Channels[channel].pdsb->Play(0,0,0);
		}
	}
	else {
		if ( !loop )
			return;	// the sound is already not looping
		else {
			// start the sound looping
			hr = Channels[channel].pdsb->Play(0,0,DSBPLAY_LOOPING);
		}
	}
#endif
}

// ---------------------------------------------------------------------------------------
// ds3d_play()
//
// Starts a ds3d sound playing
// 
//	input:
//
//					sid				=>	software id for sound to play
//					hid				=>	hardware id for sound to play (-1 if not in hardware)
//					snd_id			=> identifies what type of sound is playing
//					pos				=>	world pos of sound
//					vel				=>	velocity of object emitting sound
//					min				=>	distance at which sound doesn't get any louder
//					max				=>	distance at which sound becomes inaudible
//					looping			=>	boolean, whether to loop the sound or not
//					max_volume		=>	volume (-10000 to 0) for 3d sound at maximum
//					estimated_vol	=>	manual estimated volume
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//
//	returns:			0				=> sound started successfully
//						-1				=> sound could not be played
//
int ds3d_play(int sid, int hid, int snd_id, vec3d *pos, vec3d *vel, int min, int max, int looping, int max_volume, int estimated_vol, int priority )
{
#ifdef USE_OPENAL
	int ch_idx;
	ALfloat alvol = 1.0f, max_vol = 1.0f;
	ALint status;
	
	if (!ds_initialized)
		return -1;

	ch_idx = ds_get_free_channel(estimated_vol, snd_id, priority);

	if (ch_idx < 0) {
	//	nprintf(( "Sound", "SOUND ==> Not playing sound requested at volume %.2f\n", ds_get_percentage_vol(volume) ));
		return -1;
	}


	if (Channels[ch_idx].source_id == 0)
		return -1;
		
//	alDistanceModel(AL_INVERSE_DISTANCE);
		
	// reset pitch value since it could have been changed for this source
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_PITCH, 1.0) );

	// set up 3D sound data here
	ds3d_update_buffer(ch_idx, i2fl(min), i2fl(max), pos, vel);
		
	// Actually play it
	Channels[ch_idx].vol = estimated_vol;
	Channels[ch_idx].looping = looping;
	Channels[ch_idx].priority = priority;

	// set volume
	alvol = (estimated_vol != -10000) ? powf(10.0f, (float)estimated_vol / (-600.0f / log10f(.5f))): 0.0f;
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_GAIN, alvol) );

	// set maximum "inner cone" volume
	max_vol = (max_volume != -10000) ? powf(10.0f, (float)max_volume / (-600.0f / log10f(.5f))): 0.0f;
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_MAX_GAIN, max_vol) );	

	OpenAL_ErrorCheck( alGetSourcei(Channels[ch_idx].source_id, AL_SOURCE_STATE, &status), return -1 );

	if (status == AL_PLAYING)
		OpenAL_ErrorPrint( alSourceStop(Channels[ch_idx].source_id) );

	OpenAL_ErrorCheck( alSourcei(Channels[ch_idx].source_id, AL_BUFFER, sound_buffers[sid].buf_id), return -1 );
	
	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_SOURCE_RELATIVE, AL_TRUE) );

	OpenAL_ErrorPrint( alSourcePlay(Channels[ch_idx].source_id) );

	sound_buffers[sid].source_id = ch_idx;

	Channels[ch_idx].buf_id = sid;
	Channels[ch_idx].snd_id = snd_id;
	Channels[ch_idx].sig = channel_next_sig++;
	Channels[ch_idx].last_position = 0;

	if (channel_next_sig < 0)
		channel_next_sig = 1;

	return Channels[ch_idx].sig;

#else

	int				channel;
	HRESULT			hr;

	if (!ds_initialized)
		return -1;

	channel = ds_get_free_channel(estimated_vol, snd_id, priority);

	if (channel > -1)	{
		Assert(Channels[channel].pdsb == NULL);

		// First check if the sound is in hardware, and try to duplicate from there
		if ( hid != -1 ) {
			Int3();
			if ( ds_is_3d_buffer(ds_hardware_buffers[hid].pdsb) == FALSE ) {
				nprintf(("Sound", "SOUND ==> Tried to play non-3d buffer in ds3d_play()..\n"));
				return -1;
			}

			if ( ds_channel_dup(ds_hardware_buffers[hid].pdsb, channel, 1) == 0 ) {
				nprintf(("Sound", "SOUND ==> Played sound using DirectSound3D in hardware..\n"));
			}
		}

		// Channel will be NULL if hardware dup failed, or there was no hardware dup attempted
		if ( Channels[channel].pdsb == NULL ) {

/*
			if ( ds_is_3d_buffer(ds_software_buffers[sid].pdsb) == FALSE ) {
				nprintf(("Sound", "SOUND ==> Tried to play non-3d buffer in ds3d_play()..\n"));
				return -1;
			}
*/

			if ( ds_channel_dup(ds_software_buffers[sid].pdsb, channel, 1) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound using DirectSound3D \n"));
			}
		}

		if ( Channels[channel].pdsb == NULL ) {
			return -1;
/*
			DSBUFFERDESC desc;

			desc = ds_software_buffers[sid].desc;
			desc.lpwfxFormat = &ds_software_buffers[sid].wfx;

			// duplicate buffer failed, so call CreateBuffer instead

			hr = pDirectSound->CreateSoundBuffer(&desc, &Channels[channel].pdsb, NULL );
			// lock the data in
			if ( (hr == DS_OK) && (Channels[channel].pdsb) ) {
				BYTE	*pdest, *pdest2;
				BYTE	*psrc, *psrc2;
				DWORD	src_ds_size, dest_ds_size, not_used;
				int	src_size;
			
				if ( ds_get_size(sid, &src_size) != 0 ) {
					Int3();
					Channels[channel].pdsb->Release();
					return -1;
				}

				// lock the src buffer
				hr = ds_software_buffers[sid].pdsb->Lock(0, src_size, (void**)&psrc, &src_ds_size, (void**)&psrc2, &not_used, 0);
				if ( hr != DS_OK ) {
					mprintf(("err: %s\n", get_DSERR_text(hr)));
					Int3();
					Channels[channel].pdsb->Release();
					return -1;
				}

				if ( Channels[channel].pdsb->Lock(0, src_ds_size, (void**)(&pdest), &dest_ds_size, (void**)&pdest2, &not_used, 0) == DS_OK)	{
					memcpy(pdest, psrc, src_ds_size);
					Channels[channel].pdsb->Unlock(pdest, dest_ds_size, 0, 0);
					ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
				} else {
					Channels[channel].pdsb->Release();
					return -1;
				}
			}
*/
		}

		Assert(Channels[channel].pds3db );
		Channels[channel].pds3db->SetMode(DS3DMODE_NORMAL,DS3D_IMMEDIATE);

		// set up 3D sound data here
		ds3d_update_buffer(channel, i2fl(min), i2fl(max), pos, vel);

		Channels[channel].vol = estimated_vol;
		Channels[channel].looping = looping;

		// sets the maximum "inner cone" volume
		Channels[channel].pdsb->SetVolume(max_volume);

		int ds_flags = 0;
		if ( looping )
			ds_flags |= DSBPLAY_LOOPING;

		// Actually play it
		hr = Channels[channel].pdsb->Play(0, 0, ds_flags );

		if ( hr == DSERR_BUFFERLOST ) {
			ds_restore_buffer(Channels[channel].pdsb);
			hr = Channels[channel].pdsb->Play(0, 0, ds_flags );
		}

		if ( hr != DS_OK ) {
			nprintf(("Sound", "Sound ==> Play failed with return value %s\n", get_DSERR_text(hr) ));
			if ( Channels[channel].pdsb ) {
				int attempts = 0;
				while(++attempts < 10) {
					hr = Channels[channel].pdsb->Release();
					if ( hr == DS_OK ) {
						break;
					} else {
						nprintf(("Sound","SOUND ==> DirectSound Release() failed with code %s\n.",get_DSERR_text(hr) ));
						continue;
					}
				}
				Channels[channel].pdsb = NULL;
			}
			return -1;
		}
	}
	else {
		nprintf(( "Sound", "SOUND ==> Not playing requested 3D sound\n"));
		return -1;
	}

	Channels[channel].snd_id = snd_id;
	Channels[channel].sig = channel_next_sig++;
	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}
	return Channels[channel].sig;
#endif
}

void ds_set_position(int channel, DWORD offset)
{
#ifdef USE_OPENAL

#ifdef AL_VERSION_1_1
	OpenAL_ErrorPrint( alSourcei(Channels[channel].source_id, AL_BYTE_OFFSET, offset) );
#endif

//	STUB_FUNCTION;

#else
	// set the position of the sound buffer
	Channels[channel].pdsb->SetCurrentPosition(offset);
#endif
}

DWORD ds_get_play_position(int channel)
{
#ifdef USE_OPENAL
	ALint pos = 0;
	int buf_id;

	buf_id = Channels[channel].buf_id;

	if (buf_id == -1)
		return 0;

	if (AL_play_position) {
		OpenAL_ErrorCheck( alGetSourcei( Channels[channel].source_id, AL_BYTE_LOKI, &pos), return 0 );

		if ( pos < 0 ) {
			pos = 0;
		} else if ( pos > 0 ) {
			// AL_BYTE_LOKI returns position in canon format which may differ
			// from our sample, so we may have to scale it
			ALuint buf = sound_buffers[buf_id].buf_id;
			ALint size;

			OpenAL_ErrorCheck( alGetBufferi(buf, AL_SIZE, &size), return 0 );

			pos = (ALint)(pos * ((float)sound_buffers[buf_id].nbytes / size));
		}
	}
#ifdef AL_VERSION_1_1
	// AL_play_position should only be available under Linux, but OpenAL 1.1 provides a standard way now (except under Linux :()
	else {
		OpenAL_ErrorCheck( alGetSourcei( Channels[channel].source_id, AL_BYTE_OFFSET, &pos), return 0 );

		if ( pos < 0 )
			pos = 0;
	}
#endif

	return pos;
#else
	DWORD play,write;
	if ( Channels[channel].pdsb ) {
		Channels[channel].pdsb->GetCurrentPosition((LPDWORD)&play,(LPDWORD)&write);
	} else {
		play = 0;
	}

	return play;
#endif
}

DWORD ds_get_write_position(int channel)
{
#ifdef USE_OPENAL
//	STUB_FUNCTION;

	return 0;
#else
	DWORD play,write;	
	if ( Channels[channel].pdsb ) {
		Channels[channel].pdsb->GetCurrentPosition((LPDWORD)&play,(LPDWORD)&write);
	} else {
		write = 0;
	}

	return write;
#endif
}

int ds_get_channel_size(int channel)
{
#ifdef USE_OPENAL
	int buf_id = Channels[channel].buf_id;

	if (buf_id != -1) {
		return sound_buffers[buf_id].nbytes;
	}

	return 0;
#else
	int		size;
	DSBCAPS	caps;
	HRESULT	dsrval;

	if ( Channels[channel].pdsb ) {
		memset(&caps, 0, sizeof(DSBCAPS));
		caps.dwSize = sizeof(DSBCAPS);
		dsrval = Channels[channel].pdsb->GetCaps(&caps);
		if ( dsrval != DS_OK ) {
			return 0;
		}
		size = caps.dwBufferBytes;
	} else {
		size = 0;
	}

	return size;
#endif
}

// Returns the number of channels that are actually playing
int ds_get_number_channels()
{
#ifdef USE_OPENAL
	int i,n;

	if (!ds_initialized) {
		return 0;
	}

	n = 0;
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].source_id ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				n++;
			}
		}
	}

	return n;
#else
	int i,n;

	n = 0;
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].pdsb ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				n++;
			}
		}
	}

	return n;
#endif
}

// retreive raw data from a sound buffer
int ds_get_data(int sid, char *data)
{
#ifdef USE_OPENAL
	STUB_FUNCTION;

	return -1;
#else
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;
	void						*buffer_data;
	DWORD						buffer_size;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	// lock the entire buffer
	dsrval = pdsb->Lock(0, caps.dwBufferBytes, &buffer_data, &buffer_size, 0, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	memcpy(data, buffer_data, buffer_size);

	dsrval = pdsb->Unlock(buffer_data, buffer_size, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
#endif
}

// return the size of the raw sound data
int ds_get_size(int sid, int *size)
{
#ifdef USE_OPENAL
	Assert(sid >= 0);

	STUB_FUNCTION;

	return -1;
#else
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	*size = caps.dwBufferBytes;
	return 0;
#endif
}

int ds_using_ds3d()
{
	return Ds_use_ds3d;
}

// Return the primary buffer interface.  Note that we cast to a uint to avoid
// having to include dsound.h (and thus windows.h) in ds.h.
//
uint ds_get_primary_buffer_interface()
{
#ifdef USE_OPENAL
	// unused
	return 0;
#else
	return (uint)pPrimaryBuffer;
#endif
}

// Return the DirectSound Interface.
//
uint ds_get_dsound_interface()
{
#ifdef USE_OPENAL
	// unused
	return 0;
#else
	return (uint)pDirectSound;
#endif
}

uint ds_get_property_set_interface()
{
#ifdef USE_OPENAL
	return 0;
#else
	return (uint)pPropertySet;
#endif
}

// --------------------
//
// EAX Functions below
//
// --------------------

// Set the master volume for the reverb added to all sound sources.
//
// volume: volume, range from 0 to 1.0
//
// returns: 0 if the volume is set successfully, otherwise return -1
//
int ds_eax_set_volume(float volume)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(volume, 0.0f, 1.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_VOLUME, NULL, 0, &volume, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Set the decay time for the EAX environment (ie all sound sources)
//
// seconds: decay time in seconds
//
// returns: 0 if decay time is successfully set, otherwise return -1
//
int ds_eax_set_decay_time(float seconds)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(seconds, 0.1f, 20.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_DECAYTIME, NULL, 0, &seconds, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Set the damping value for the EAX environment (ie all sound sources)
//
// damp: damp value from 0 to 2.0
//
// returns: 0 if the damp value is successfully set, otherwise return -1
//
int ds_eax_set_damping(float damp)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(damp, 0.0f, 2.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_DAMPING, NULL, 0, &damp, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Set up the environment type for all sound sources.
//
// envid: value from the EAX_ENVIRONMENT_* enumeration in ds_eax.h
//
// returns: 0 if the environment is set successfully, otherwise return -1
//
int ds_eax_set_environment(unsigned long envid)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ENVIRONMENT, NULL, 0, &envid, sizeof(unsigned long));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Set up a predefined environment for EAX
//
// envid: value from teh EAX_ENVIRONMENT_* enumeration
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_set_preset(unsigned long envid)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);
	Assert(envid < EAX_ENVIRONMENT_COUNT);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, &Ds_eax_presets[envid], sizeof(EAX_REVERBPROPERTIES));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}


// Set up all the parameters for an environment
//
// id: value from teh EAX_ENVIRONMENT_* enumeration
// volume: volume for the environment (0 to 1.0)
// damping: damp value for the environment (0 to 2.0)
// decay: decay time in seconds (0.1 to 20.0)
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_set_all(unsigned long id, float vol, float damping, float decay)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);
	Assert(id < EAX_ENVIRONMENT_COUNT);

	EAX_REVERBPROPERTIES er;

	er.environment = id;
	er.fVolume = vol;
	er.fDecayTime_sec = decay;
	er.fDamping = damping;

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, &er, sizeof(EAX_REVERBPROPERTIES));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Get up the parameters for the current environment
//
// er: (output) hold environment parameters
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_get_all(EAX_REVERBPROPERTIES *er)
{
#ifdef USE_OPENAL
	return -1;
#else
	HRESULT hr;
	unsigned long outsize;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	hr = Ds_eax_reverb->Get(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, er, sizeof(EAX_REVERBPROPERTIES), &outsize);
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

// Close down EAX, freeing any allocated resources
//
void ds_eax_close()
{
#ifndef USE_OPENAL
	if (Ds_eax_inited == 0) {
		return;
	}

	Ds_eax_inited = 0;
#endif
}

// Initialize EAX
//
// returns: 0 if initialization is successful, otherwise return -1
//
int ds_eax_init()
{
#ifndef USE_OPENAL
	HRESULT hr;
	unsigned long driver_support = 0;

	if (Ds_eax_inited) {
		return 0;
	}

	Assert(Ds_eax_reverb == NULL);

	Ds_eax_reverb = (LPKSPROPERTYSET)ds_get_property_set_interface();
	if (Ds_eax_reverb == NULL) {
		return -1;
	}

	// check if the listener property is supported by the audio driver
	hr = Ds_eax_reverb->QuerySupport(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, &driver_support);
	if (FAILED(hr)) {
		nprintf(("Sound", "QuerySupport for the EAX Listener property set failed.. disabling EAX\n"));
		goto ds_eax_init_failed;
	}

	if ((driver_support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) {
		goto ds_eax_init_failed;
	}

	ds_eax_set_all(EAX_ENVIRONMENT_GENERIC, 0.0f, 0.0f, 0.0f);

	Ds_eax_inited = 1;
	return 0;

ds_eax_init_failed:
	if (Ds_eax_reverb != NULL) {
		Ds_eax_reverb->Release();
		Ds_eax_reverb = NULL;
	}

	Ds_eax_inited = 0;
#endif
	return -1;
}

int ds_eax_is_inited()
{
#ifdef USE_OPENAL
	return 0;
#else
	return Ds_eax_inited;
#endif
}

bool ds_using_a3d()
{
	return false;
}

// Called once per game frame to make sure voice messages aren't looping
//
void ds_do_frame()
{
	if (!ds_initialized)
		return;

	int i;
	channel *cp = NULL;

	for (i = 0; i < MAX_CHANNELS; i++) {
		cp = &Channels[i];
		Assert( cp != NULL );

		if (cp->is_voice_msg == true) {
#ifdef USE_OPENAL
			if( cp->source_id == 0 ) {
#else
			if (cp->pdsb == NULL) {
#endif
				continue;
			}

			DWORD current_position = ds_get_play_position(i);
			if (current_position != 0) {
				if (current_position < cp->last_position) {
					ds_close_channel(i);
				} else {
					cp->last_position = current_position;
				}
			}
		}
	}
}

// given a valid channel return the sound id
int ds_get_sound_id(int channel)
{
	Assert( channel >= 0 );

	return Channels[channel].snd_id;
}


#ifdef SCP_UNIX
void dscap_close()
{
	STUB_FUNCTION;
}

int dscap_create_buffer(int freq, int bits_per_sample, int nchannels, int nseconds)
{
//	STUB_FUNCTION;

	return -1;
}

int dscap_get_raw_data(unsigned char *outbuf, unsigned int max_size)
{
//	STUB_FUNCTION;

	return -1;
}

int dscap_max_buffersize()
{
//	STUB_FUNCTION;
	
	return -1;
}

void dscap_release_buffer()
{
//	STUB_FUNCTION;
}

int dscap_start_record()
{
//	STUB_FUNCTION;

	return -1;
}

int dscap_stop_record()
{
//	STUB_FUNCTION;

	return -1;
}

int dscap_supported()
{
//	STUB_FUNCTION;

	return 0;
}
#endif
