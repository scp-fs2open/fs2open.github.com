/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3D.cpp $
 * $Revision: 2.27 $
 * $Date: 2003-10-16 00:17:14 $
 * $Author: randomtiger $
 *
 * Code for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.26  2003/10/14 17:39:13  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.25  2003/10/13 19:39:19  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.24  2003/10/10 03:59:40  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.23  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.22  2003/09/23 02:42:53  Kazan
 * ##KAZAN## - FS2NetD Support! (FS2 Open PXO) -- Game Server Listing, and mission validation completed - stats storing to come - needs fs2open_pxo.cfg file [VP-able]
 *
 * Revision 2.21  2003/09/07 18:14:53  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.20  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.19  2003/08/22 07:35:08  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.18  2003/08/21 20:54:38  randomtiger
 * Fixed switching - RT
 *
 * Revision 2.17  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.16  2003/08/12 03:18:33  bobboau
 * Specular 'shine' mapping;
 * useing a phong lighting model I have made specular highlights
 * that are mapped to the model,
 * rendering them is still slow, but they look real purdy
 *
 * also 4 new (untested) comand lines, the XX is a floating point value
 * -spec_exp XX
 * the n value, you can set this from 0 to 200 (actualy more than that, but this is the recomended range), it will make the highlights bigger or smaller, defalt is 16.0 so start playing around there
 * -spec_point XX
 * -spec_static XX
 * -spec_tube XX
 * these are factors for the three diferent types of lights that FS uses, defalt is 1.0,
 * static is the local stars,
 * point is weapons/explosions/warp in/outs,
 * tube is beam weapons,
 * for thouse of you who think any of these lights are too bright you can configure them you're self for personal satisfaction
 *
 * Revision 2.15  2003/08/09 06:07:24  bobboau
 * slightly better implementation of the new zbuffer thing, it now checks only three diferent formats defalting to the 16 bit if neither the 24 or 32 bit versions are suported
 *
 * Revision 2.14  2003/08/05 23:45:18  bobboau
 * glow maps, for some reason they wern't in here, they should be now,
 * also there is some debug code for changeing the FOV in game,
 * and I have some changes to the init code to try and get a 32 or 24 bit back buffer
 * if posable, this may cause problems for people
 * the changes were all marked and if needed can be undone
 *
 * Revision 2.13  2003/07/06 00:19:25  randomtiger
 * Random Tiger 6/7/2003
 *
 * fs2_open now uses the registry entry 'VideocardFs2open' instead of 'Videocard' to store its video settings. To run fs2_open now you MUST use the launcher I have provided.
 *
 * Launcher binary:      http://mysite.freeserve.com/thomaswhittaker/c_code/freespace/Launcher.rar
 * Launcher source code: http://mysite.freeserve.com/thomaswhittaker/c_code/freespace/Launcher_code.rar
 *
 * I have also taken the opertunity to fix a few bugs in the launcher and add a new feature to make selecting mods a bit easier.
 *
 * The launcher now uses some files in the freespace project so it should be put into CVS with the rest of the code inside the 'code' directory (still in its 'Launcher' dir of course). Currently the launcher wont compile since speech.cpp and speech.h arent in cvs yet. But once Roee has checked in that will be sorted.
 *
 * I have also removed the internal launcher from the D3D8 module.
 * Please contact me if you have any problems.
 *
 * When trying to run the exe after updating I get an error parsing 'rank.tbl' but im fairly sure thats nothing to do with me so I'll just have to leave it for now because I'm still using a 56K modem and cant afford to find out.
 *
 * Revision 2.12  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.11  2003/03/19 23:06:39  Goober5000
 * bit o' housecleaning
 * --Goober5000
 *
 * Revision 2.10  2003/03/19 12:29:02  unknownplayer
 * Woohoo! Killed two birds with one stone!
 * Fixed the 'black screen around dialog boxes' problem and also the much more serious freezing problem experienced by Goober5000. It wasn't a crash, just an infinite loop. DX8 merge is GO! once again :)
 *
 * Revision 2.9  2003/03/19 09:05:26  Goober5000
 * more housecleaning, this time for debug warnings
 * --Goober5000
 *
 * Revision 2.8  2003/03/19 07:17:05  unknownplayer
 * Updated the DX8 code to allow it to be run in 640x480 windowed mode. The game will only procede to do so if your registry settings are set to 640x480 to begin with, but this is accomplished simply enough.
 *
 * Revision 2.7  2003/03/19 06:22:58  Goober5000
 * added typecasting to get rid of some build warnings
 * --Goober5000
 *
 * Revision 2.6  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.5  2003/03/02 05:43:48  penguin
 * ANSI C++ - fixed non-compliant casts to unsigned short and unsigned char
 *  - penguin
 *
 * Revision 2.4  2003/01/14 05:53:58  Goober5000
 * commented out some mprintfs that were clogging up the debug spew
 * --Goober5000
 *
 * Revision 2.3  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.2.2.28  2002/11/16 20:09:54  randomtiger
 *
 * Changed my fog hack to be valid code. Put large texture check back in.
 * Added some blending type checks. - RT
 *
 * Revision 2.2.2.27  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.2.2.26  2002/11/10 02:44:43  randomtiger
 *
 * Have put in a hack to get fog working in D3D8 but the method V has used is frowned
 * on in D3D8 and will likely be a lot slower than the same thing in D3D5. - RT
 *
 * Revision 2.2.2.25  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.2.2.24  2002/11/05 10:27:38  randomtiger
 *
 * Fixed windowed mode bug I introduced.
 * Added Antialiasing functionality, can only be sure it works on GF4 in 1024 mode. - RT
 *
 * Revision 2.2.2.23  2002/11/04 23:53:25  randomtiger
 *
 * Added new command line parameter -d3dlauncher which brings up the launcher.
 * This is needed since FS2 DX8 now stores the last successful details in the registry and
 * uses them to choose the adapter and mode to run in unless its windowed or they are not set.
 * Added some code for Antialiasing but it messes up the font but hopefully that can be fixed later. - RT
 *
 * Revision 2.2.2.22  2002/11/04 21:24:59  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.2.2.21  2002/11/04 16:04:20  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.2.2.20  2002/11/04 03:02:28  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.2.2.19  2002/11/02 13:54:26  randomtiger
 *
 * Made a few cunning alterations to get rid of that alpha bug but might have a slight slowdown.
 * Non alpha textures are now alpha textures with (if texture format supported) just one bit for alpha.
 * And non alpha blending is now alpha blending with automatic disregaring of 0 alpha.
 *
 * Revision 2.2.2.18  2002/10/30 22:57:21  randomtiger
 *
 * Changed DX8 code to not use set render and texture states if they are already set to that value.
 * Disabled buffer saving and restoring code when windowed to stop DX8 debug runs from crashing. - RT
 *
 * Revision 2.2.2.17  2002/10/28 00:40:41  randomtiger
 * Implemented screen saving code for restoring when drawing popups, a bit slow but works. - RT
 *
 * Revision 2.2.2.16  2002/10/26 01:24:22  randomtiger
 * Fixed debug bitmap compiling bug.
 * Fixed tga bug. - RT
 *
 * Revision 2.2.2.15  2002/10/21 16:33:41  randomtiger
 * Added D3D only 32 bit TGA functionality. Will load a texture as big as your graphics card allows. Code not finished yet and will forge the beginnings of the new texture system. - RT
 *
 * Revision 2.2.2.14  2002/10/20 22:21:48  randomtiger
 * Some incomplete code to handle background drawing when message boxes are drawn.
 * It doesnt work but its a good base for someone to start from. - RT
 *
 * Revision 2.2.2.13  2002/10/19 23:56:40  randomtiger
 * Changed generic bitmap code to allow maximum dimensions to be determined by 3D's engines maximum texture size query.
 * Defaults to 256 as it was before. Also added base code for reworking the texture code to be more efficient. - RT
 *
 * Revision 2.2.2.12  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.2.2.11  2002/10/14 21:52:01  randomtiger
 * Finally tracked down and killed off that 8 bit alpha bug.
 * So the font, HUD and 8 bit ani's now work fine. - RT
 *
 * Revision 2.2.2.10  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.2.2.9  2002/10/08 14:33:27  randomtiger
 * OK, I've fixed the z-buffer problem.
 * However I have abandoned using w-buffer for now because of problems.
 * I think I know how to solve it but Im not sure it would make much difference given the way FS2 engine works.
 * I have left code in there of use if anyone wants to turn their hand to it. However for now
 * we just need to crack of the alpha problem then we will have FS2 fully wokring in DX8 on GF4 in 32 bit.
 *
 * Revision 2.2.2.8  2002/10/04 00:48:42  randomtiger
 * Fixed video memory leaks
 * Added code to cope with lost device, not tested
 * Got rid of some DX5 stuff we definately dont need
 * Moved some enum's into internal,h because gr_d3d_set_state should be able to be called from any dx file
 * Cleaned up some stuff - RT
 *
 * Revision 2.2.2.7  2002/10/03 08:32:08  unknownplayer
 *
 * Hacked in a windowed mode so we can properly debug this without using
 * monitors (although I drool at the concept of having that!)
 *
 * Revision 2.2.2.6  2002/10/02 17:52:32  randomtiger
 * Fixed blue lighting bug.
 * Put filtering flag set back in that I accidentally removed
 * Added some new functionality to my debuging system - RT
 *
 * Revision 2.2.2.5  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.2.2.4  2002/09/28 22:13:42  randomtiger
 * Sorted out some bits and pieces. The background nebula blends now which is nice. – RT
 *
 * Revision 2.2.2.3  2002/09/28 12:20:32  randomtiger
 * Just a tiny code change that lets stuff work in 16 bit.
 * For some reason 16 bit code was taking a different code path for displaying textures.
 * So until I unstand why, Im cutting off that codepath because it isnt easy to convert into DX8.
 *
 * Revision 2.2.2.2  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.2  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/30 14:29:15  unknownplayer
 *
 * Started work on DX8.1 implementation. Updated the project files to encompass
 * the new files. Disable the compiler tag to use old DX code (THERE IS NO
 * NEW CODE YET!)
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 42    10/13/99 3:49p Jefff
 * fixed unnumbered XSTRs
 * 
 * 41    9/13/99 11:25p Dave
 * Fixed problem with mode-switching and D3D movies.
 * 
 * 40    9/13/99 11:30a Dave
 * Added checkboxes and functionality for disabling PXO banners as well as
 * disabling d3d zbuffer biasing.
 * 
 * 39    9/10/99 11:53a Dave
 * Shutdown graphics before sound to eliminate apparent lockups when
 * Directsound decides to be lame. Fix TOPMOST problem with D3D windows.
 * 
 * 38    9/04/99 8:00p Dave
 * Fixed up 1024 and 32 bit movie support.
 * 
 * 37    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 36    8/20/99 2:09p Dave
 * PXO banner cycling.
 * 
 * 35    8/18/99 9:35a Dave
 * Made d3d shutdown more stable.
 * 
 * 34    8/11/99 3:30p Dave
 * Fixed window focus problems.
 * 
 * 33    8/04/99 5:36p Dave
 * Make glide and D3D switch out properly.
 * 
 * 32    8/02/99 6:25p Dave
 * Fixed d3d screen save/popup problem.
 * 
 * 31    7/30/99 7:01p Dave
 * Dogfight escort gauge. Fixed up laser rendering in Glide.
 * 
 * 30    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 29    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 28    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 27    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 26    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 25    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 24    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 23    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 22    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 21    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 20    1/24/99 11:36p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 19    1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 18    1/11/99 6:21p Neilk
 * Fixed broken D3D card fog-capability check.
 * 
 * 17    1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 16    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 15    12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 14    12/08/98 7:30p Dave
 * Fixed broken compile.
 * 
 * 13    12/08/98 7:03p Dave
 * Much improved D3D fogging. Also put in vertex fogging for the cheesiest
 * of 3d cards.
 * 
 * 12    12/08/98 2:47p Johnson
 * Made D3D fogging use eye-relative depth instead of z-depth. Works like
 * Glide w-buffer now.
 * 
 * 11    12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 10    12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 9     12/06/98 6:53p Dave
 * 
 * 8     12/06/98 3:08p Dave
 * Fixed grx_tmapper to handle pixel fog flag. First run fog support for
 * D3D.
 * 
 * 7     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 6     12/01/98 10:25a Johnson
 * Fixed direct3d texture coord/font problems.
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 110   6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 109   6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 108   5/24/98 9:41p John
 * changed allender's previous fix to actually not draw the lines on
 * NDEBUG.
 * 
 * 107   5/24/98 9:16p Allender
 * put in previously non-NDEBUG code to draw bogus cursor when Gr_cursor
 * wasn't defined.  Caused d3d to crash before playing movies
 * 
 * 106   5/22/98 10:29p John
 * fixed some mode switching and line offset detection bugs.
 * 
 * 105   5/22/98 1:11p John
 * Added code to actually detect which offset a line needs
 * 
 *
 * $NoKeywords: $
 */

#include <math.h>
#include <d3d8.h>
#include <D3dx8tex.h>
#include <Dxerr8.h>

// Only needed to check for hi res vp file
#include <direct.h>
#include <io.h>

#include "graphics/grd3dinternal.h"

#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "osapi/osregistry.h"
#include "graphics/grd3d.h"
#include "graphics/line.h"
#include "graphics/font.h"
#include "graphics/grinternal.h"
#include "io/mouse.h"
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"
#include "graphics/grd3dbmpman.h"
#include "freespace2/freespaceresource.h"   
#include "model/model.h"   

// The main direct3D interface that devices are create from
IDirect3D8 *lpD3D = NULL;
// The direct3D device, if pofview is ever to use this code this may have to become an array
IDirect3DDevice8 *lpD3DDevice = NULL;
// direct3D capabilites, this is filled after the device is created
D3DCAPS8 d3d_caps;
// Present parameters, these are filled before device create to determine a number of display options
D3DPRESENT_PARAMETERS d3dpp; 

bool D3D_Antialiasing =  0;
int	 D3D_custom_size  = -1;

// DirectDraw structures used without DD
// To contain shift and scale variables to convert graphics files to textures 
DDPIXELFORMAT			AlphaTextureFormat;
DDPIXELFORMAT			NonAlphaTextureFormat;

D3DFORMAT default_non_alpha_tformat = D3DFMT_UNKNOWN;
D3DFORMAT default_alpha_tformat		= D3DFMT_UNKNOWN;

D3DFORMAT default_32_non_alpha_tformat;
D3DFORMAT default_32_alpha_tformat;

// Viewport used to change render between full screen and sub sections like the pilot animations
D3DVIEWPORT8 viewport;

RECT D3D_cursor_clip_rect;
int  D3D_texture_divider = 1;
int  D3D_window = 0;
char Device_init_error[512] = "";
extern int Cmdline_nohtl;

const int multisample_max = 16;
const D3DMULTISAMPLE_TYPE multisample_types[multisample_max] =
{
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_2_SAMPLES,
	D3DMULTISAMPLE_3_SAMPLES,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_5_SAMPLES,
	D3DMULTISAMPLE_6_SAMPLES,
	D3DMULTISAMPLE_7_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_9_SAMPLES,
	D3DMULTISAMPLE_10_SAMPLES,
	D3DMULTISAMPLE_11_SAMPLES,
	D3DMULTISAMPLE_12_SAMPLES,
	D3DMULTISAMPLE_13_SAMPLES,
	D3DMULTISAMPLE_14_SAMPLES,
	D3DMULTISAMPLE_15_SAMPLES,
	D3DMULTISAMPLE_16_SAMPLES
};

int D3D_activate = 0;

// -1 == no fog, bad bad bad
// 0 == vertex fog
// 1 == table fog
int D3D_fog_mode = -1;

int In_frame = 0;
int D3D_inited = 0;
//int DrawPrim = 0;
int D3d_rendition_uvs = 0;	
int D3D_32bit = 0;

int D3D_zbias = 1;
DCF(zbias, "")
{
	D3D_zbias = !D3D_zbias;
}

D3DLIGHT8 empty_light;

LPVOID lpBufStart, lpPointer, lpInsStart;
int Exb_size;

bool gr_d3d_resize_screen_pos(int *x, int *y)
{
	if(D3D_custom_size < 0)	return false;

	int div_by_x = (D3D_custom_size == GR_1024) ? 1024 : 640;
	int div_by_y = (D3D_custom_size == GR_1024) ?  768 : 480;
			
	if(x) {
		(*x) *= d3dpp.BackBufferWidth;
		(*x) /= div_by_x;
	}

	if(y) {
		(*y) *= d3dpp.BackBufferHeight;
		(*y) /= div_by_y;
	}

	return true;
}

void d3d_fill_pixel_format(DDPIXELFORMAT *pixelf, D3DFORMAT tformat);

// This doesnt do much now but will be very important in making FS2 use hardware T&L
HRESULT set_wbuffer_planes(float dvWNear, float dvWFar)
{
		
	HRESULT res;
	D3DMATRIX matWorld;
	D3DMATRIX matView;
	D3DMATRIX matProj;
	
	memset(&matWorld, 0, sizeof(matWorld));
	memset(&matView, 0, sizeof(matWorld));
	memset(&matProj, 0, sizeof(matWorld));
	matWorld._11 = 1; matWorld._22 = 1; matWorld._33 = 1; matWorld._44 = 1;
	matView._11 = 1; matView._22 = 1; matView._33 = 1; matView._44 = 1;
	matProj._11 = 1; matProj._22 = 1; matProj._33 = 1; matProj._44 = 1;
	
	res = lpD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
	
	if (FAILED(res)) 
	{
		DBUGFILE_OUTPUT_0("Failed set world");
		return res;
	}

	res = lpD3DDevice->SetTransform( D3DTS_VIEW, &matView );
	
	if (FAILED(res)) 
	{
		DBUGFILE_OUTPUT_0("Failed set view");
		return res;
	}
	
	matProj._43 = 0;
	matProj._34 = 1;
	matProj._44 = dvWNear; // not used
	matProj._33 = dvWNear / (dvWFar - dvWNear) + 1;  
	
	extern float z_mult;

//	RT Have to use this wbuffer compatiable matrix to use wbuffer
//	D3DXMatrixPerspectiveLH((D3DXMATRIX *) &matProj, 4, 3, 0.5, z_mult);
//	D3DXMatrixPerspectiveFovLH((D3DXMATRIX *) &matProj, 3.14 / 2, 3/4, 0.5, z_mult); 
//	DBUGFILE_OUTPUT_MATRIX_4X4((float *) &matProj, "Projection");
	
	res = lpD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	return res;
}

DCF(wplanes, "")
{
	dc_get_arg(ARG_FLOAT);
	float n = Dc_arg_float;
	dc_get_arg(ARG_FLOAT);
	float f = Dc_arg_float;
	set_wbuffer_planes(n, f);
}

void gr_d3d_exb_flush(int end_of_frame)
{
	/*
	HRESULT ddrval;
	D3DEXECUTEBUFFERDESC debDesc;
	D3DEXECUTEDATA d3dExData;

	if ( DrawPrim ) {
		return;
	}

	if (!lpExBuf) return;

	OP_EXIT( D3D_ex_ptr );

	lpPointer = lpInsStart;
	OP_PROCESS_VERTICES( 1, lpPointer );
	PROCESSVERTICES_DATA( D3DPROCESSVERTICES_COPY, 0,  D3D_num_verts, lpPointer );

	ddrval = lpExBuf->Unlock();
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to unlock the execute buffer!\n" ));
		goto D3DError;
	}

	memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
	d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
	d3dExData.dwVertexCount = D3D_num_verts;
	d3dExData.dwInstructionOffset = (ULONG)((char*)lpInsStart - (char*)lpBufStart);
	d3dExData.dwInstructionLength = (ULONG)((char*)D3D_ex_ptr - (char*)lpInsStart);

//	if (end_of_frame==0)	{
//		mprintf(( "Flushing execute buffer in frame, %d verts, %d data size!\n", D3D_num_verts, d3dExData.dwInstructionLength ));
//	} else if (end_of_frame==2)	{ 
//		mprintf(( "Flushing execute buffer in frame, because of VRAM flush!\n" ));
//	}

	ddrval = lpExBuf->SetExecuteData(&d3dExData);
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to SetExecuteData!\n" ));
		goto D3DError;
	}

	ddrval = lpD3DDeviceEB->Execute( lpExBuf, lpViewport, D3DEXECUTE_UNCLIPPED );
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to Execute! nverts=%d\n", D3D_num_verts));
		mprintf(( "(%s)\n", d3d_error_string(ddrval) ));
		goto D3DError;
	}


	memset( &debDesc, 0, sizeof( debDesc ) );
	debDesc.dwSize       = sizeof( debDesc );
	ddrval = lpExBuf->Lock( &debDesc );
	if ( ddrval != DD_OK )	{
		mprintf(( "Failed to lock the execute buffer!\n" ));
		goto D3DError;
	}

	lpPointer = lpBufStart = lpInsStart = debDesc.lpData;

	lpPointer = (void *)((uint)lpPointer+sizeof(D3DTLVERTEX)*D3D_MAX_VERTS);
	lpInsStart = lpPointer;

	OP_PROCESS_VERTICES( 1, lpPointer );
	PROCESSVERTICES_DATA( D3DPROCESSVERTICES_COPY, 0,  1, lpPointer );

	D3D_num_verts = 0;
	D3D_ex_ptr = lpPointer;
	D3D_ex_end = (void *)((uint)lpBufStart + Exb_size - 1024);
	D3D_vertices = (D3DTLVERTEX *)lpBufStart;
	return;


D3DError:
	// Reset everything

	if ( lpExBuf )	{
		lpExBuf->Release();
		lpExBuf = NULL;
	}
//	gr_d3d_cleanup();
//	exit(1);
*/
}

/*	Obsolete as per merge
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState )
{
	if ( DrawPrim )	{
		return lpD3DDevice->SetRenderState(dwRenderStateType, dwRenderState );
	} else {
		if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  {
			gr_d3d_exb_flush(0);
		}
		OP_STATE_RENDER(1, D3D_ex_ptr);
		STATE_DATA(dwRenderStateType, dwRenderState, D3D_ex_ptr);
		return DD_OK;
	}

}

HRESULT d3d_DrawPrimitive( D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags )
{
	if ( DrawPrim )	{
		return lpD3DDevice->DrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags );
	} else {	

		switch( dptPrimitiveType )	{

		case D3DPT_TRIANGLEFAN:	
			if ( dvtVertexType == D3DVT_TLVERTEX )	{

				D3DTLVERTEX *Verts = (D3DTLVERTEX *)lpvVertices;
				
				if ( D3D_num_verts + dwVertexCount > D3D_MAX_VERTS ) gr_d3d_exb_flush(0);
				if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  gr_d3d_exb_flush(0);

				D3DTLVERTEX *src_v = &D3D_vertices[D3D_num_verts];

				int i;
				for (i=0; i<(int)dwVertexCount; i++ )	{
					*src_v++ = Verts[i];
				}

				// triangle data must be QWORD aligned, so we need to make sure
				// that the OP_TRIANGLE_LIST is unaligned!  Note that you MUST have
				// the braces {} around the OP_NOP since the macro in D3DMACS.H will
				// fail if you remove them.

				if ( QWORD_ALIGNED( D3D_ex_ptr ) ) {
					OP_NOP( D3D_ex_ptr );
				}

				OP_TRIANGLE_LIST( (unsigned short)(dwVertexCount-2), D3D_ex_ptr );

				LPD3DTRIANGLE tri = ( LPD3DTRIANGLE )D3D_ex_ptr;

				for (i=0; i<(int)dwVertexCount-2; i++ )	{
					tri->v1 = (unsigned short)(D3D_num_verts+0);
					tri->v2 = (unsigned short)(D3D_num_verts+i+1);
					tri->v3 = (unsigned short)(D3D_num_verts+i+2);
					tri->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
					tri++;
				
				}
				D3D_ex_ptr = ( LPVOID )tri;
				D3D_num_verts += (int)dwVertexCount;
			}
			break;

		case D3DPT_LINELIST:
			if ( dvtVertexType == D3DVT_TLVERTEX )	{

				D3DTLVERTEX *Verts = (D3DTLVERTEX *)lpvVertices;
				
				if ( D3D_num_verts + dwVertexCount > D3D_MAX_VERTS ) gr_d3d_exb_flush(0);
				if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  gr_d3d_exb_flush(0);

				D3DTLVERTEX *src_v = &D3D_vertices[D3D_num_verts];

				int i;
				for (i=0; i<(int)dwVertexCount; i++ )	{
					*src_v++ = Verts[i];
				}

				// triangle data must be QWORD aligned, so we need to make sure
				// that the OP_TRIANGLE_LIST is unaligned!  Note that you MUST have
				// the braces {} around the OP_NOP since the macro in D3DMACS.H will
				// fail if you remove them.

				if ( QWORD_ALIGNED( D3D_ex_ptr ) ) {
					OP_NOP( D3D_ex_ptr );
				}

				ushort nlines = ushort(dwVertexCount/2);

				OP_LINE_LIST( nlines, D3D_ex_ptr );
	
				for (i=0; i<(int)nlines; i++ )	{
					LPD3DLINE line = (LPD3DLINE )D3D_ex_ptr;
					line->v1 = (unsigned short)(D3D_num_verts);
					line->v2 = (unsigned short)(D3D_num_verts+1);
					D3D_ex_ptr = (void *)(((LPD3DLINE)D3D_ex_ptr) + 1);
				}	

				D3D_num_verts += (int)dwVertexCount;
			}
			break;
		}
		return DD_OK;
	}
}
*/
void gr_d3d_activate(int active)
{
	mprintf(( "Direct3D activate: %d\n", active ));

	HWND hwnd = (HWND)os_get_window();
	
	if ( active  )	{
		D3D_activate = 1;

		if ( hwnd )	{
			ClipCursor(&D3D_cursor_clip_rect);
			ShowWindow(hwnd,SW_RESTORE);
		}

	} else {

		D3D_activate = 0;

		if ( hwnd )	{
			ClipCursor(NULL);
			ShowWindow(hwnd,SW_MINIMIZE);
		}
	}
}

// No objects should be rendered before this frame
void d3d_start_frame()
{
	if(!D3D_activate) return;

	HRESULT ddrval;

	if (!D3D_inited) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Start frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 1 ) return;

	In_frame++;


	ddrval = lpD3DDevice->BeginScene();
	if (ddrval != D3D_OK )	{
		//mprintf(( "Failed to begin scene!\n%s\n", d3d_error_string(ddrval) ));
		return;
	}
}

// No objects should be rendered after this frame
void d3d_stop_frame()
{
	if (!D3D_inited) return;
	if(!D3D_activate) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Stop frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 0 ) return;

	In_frame--;
	

	TIMERBAR_END_FRAME();
	if(FAILED(lpD3DDevice->EndScene()))
	{
		return;
	}
	TIMERBAR_START_FRAME();

	// Must cope with device being lost
	if(lpD3DDevice->Present(NULL,NULL,NULL,NULL) == D3DERR_DEVICELOST)
	{
		d3d_lost_device();
	}
}

// Outputs a format to debug console
void d3d_dump_format(DDPIXELFORMAT *pf)
{
	unsigned long m;
	int r, g, b, a;
	for (r = 0, m = pf->dwRBitMask; !(m & 1); r++, m >>= 1);
	for (r = 0; m & 1; r++, m >>= 1);
	for (g = 0, m = pf->dwGBitMask; !(m & 1); g++, m >>= 1);
	for (g = 0; m & 1; g++, m >>= 1); 
	for (b = 0, m = pf->dwBBitMask; !(m & 1); b++, m >>= 1);
	for (b = 0; m & 1; b++, m >>= 1);
	if ( pf->dwFlags & DDPF_ALPHAPIXELS ) {
		for (a = 0, m = pf->dwRGBAlphaBitMask; !(m & 1); a++, m >>= 1);
		for (a = 0; m & 1; a++, m >>= 1);
		mprintf(( "ARGB, %d:%d:%d:%d\n", a, r, g, b ));
	} else {
		a = 0;
		mprintf(( "RGB, %d:%d:%d\n", r, g, b ));
	}
}

void d3d_clip_cursor(int active)
{
	ClipCursor(active ? &D3D_cursor_clip_rect : NULL);
}

// This destroys the direct3D device
void d3d_release_rendering_objects()
{
	if ( lpD3DDevice )	{
		lpD3DDevice->Release();
		lpD3DDevice = NULL;
	}

	DBUGFILE_OUTPUT_COUNTER(0, "Number of unfreed textures");
}

enum stage_state{NONE = -1, INITAL = 0, DEFUSE = 1, GLOW_MAPPED_DEFUSE = 2, NONMAPPED_SPECULAR = 3, GLOWMAPPED_NONMAPPED_SPECULAR = 4, MAPPED_SPECULAR = 5, CELL = 6, GLOWMAPPED_CELL = 7, ADDITIVE_GLOWMAPPING = 8, GLOW_MAPPED_SPECULAR_MAPPED = 9, GLOW_SPEC_NO_SPEC = 10, GLOW_SPEC_NO_GLOW = 11};
stage_state current_render_state = NONE;

// This function calls these render state one when the device is initialised and when the device is lost.
void d3d_set_initial_render_state()
{
	if(current_render_state == INITAL)return;

	if(current_render_state == NONE){//this only needs to be done the first time-Bobboau
		d3d_SetRenderState(D3DRS_DITHERENABLE, TRUE );
		d3d_SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		d3d_SetRenderState(D3DRS_SPECULARENABLE, FALSE ); 

		// Turn lighting off here, its on by default!
		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	}


	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	d3d_SetTexture(1, NULL);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = INITAL;
}

extern bool env_enabled;

void set_stage_for_cell_shaded(){
	if(current_render_state == CELL)return;
	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT );
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT );

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = CELL;
		
}

void set_stage_for_cell_glowmapped_shaded(){
	if(current_render_state == GLOWMAPPED_CELL)return;
	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT );
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT );

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD);

	current_render_state = GLOWMAPPED_CELL;
		
}

void set_stage_for_additive_glowmapped(){
	if(current_render_state == ADDITIVE_GLOWMAPPING)return;

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	
	current_render_state = ADDITIVE_GLOWMAPPING;
}

void set_stage_for_defuse(){
	if(current_render_state == DEFUSE)return;

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = DEFUSE;
		
}

void set_stage_for_glow_mapped_defuse(){
	if(current_render_state == GLOW_MAPPED_DEFUSE)return;
	if(GLOWMAP < 0){
		set_stage_for_defuse();
		return;
	}
		d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
//	d3d_SetTexture(1, GLOWMAP);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = GLOW_MAPPED_DEFUSE;
}

void set_stage_for_defuse_and_non_mapped_spec(){
	if(current_render_state == NONMAPPED_SPECULAR)return;
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = NONMAPPED_SPECULAR;
}

void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(){
	if(current_render_state == GLOWMAPPED_NONMAPPED_SPECULAR)return;
	if(GLOWMAP < 0){
		set_stage_for_defuse_and_non_mapped_spec();
		return;
	}
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
//	d3d_SetTexture(1, GLOWMAP);

	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD);

	current_render_state = GLOWMAPPED_NONMAPPED_SPECULAR;
}

bool set_stage_for_spec_mapped(){
	if(current_render_state == MAPPED_SPECULAR)return true;
	if(SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
//	d3d_SetTexture(0, SPECMAP);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);


	current_render_state = MAPPED_SPECULAR;
	return true;
}


bool set_stage_for_spec_glow_mapped(){
	if(current_render_state == GLOW_MAPPED_SPECULAR_MAPPED)return true;
	if((SPECMAP < 0)&&(GLOWMAP < 0)){
		return false;
	}

	if(SPECMAP < 0){
		if(current_render_state == GLOW_SPEC_NO_SPEC)return true;
		d3d_SetTexture(0, NULL);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		current_render_state = GLOW_SPEC_NO_SPEC;
	}else{
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
//		d3d_SetTexture(0, SPECMAP);
	}

	if(GLOWMAP < 0){
		if(current_render_state == GLOW_SPEC_NO_GLOW)return true;
		d3d_SetTexture(1, NULL);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		current_render_state = GLOW_SPEC_NO_GLOW;
	}else{
		d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);
	}


	if((SPECMAP > 0)&&(GLOWMAP > 0))
		current_render_state = GLOW_MAPPED_SPECULAR_MAPPED;
	return true;
}

void set_stage_for_mapped_environment_mapping(){
/*	if(ENVMAP > 0 && env_enabled){
		d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1);
	
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_ADD);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}else{
		d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	state = INITAL;*/
}

void d3d_setup_format_components(
	DDPIXELFORMAT *surface, color_gun *r_gun, color_gun *g_gun, color_gun *b_gun, color_gun *a_gun)
{
	int s;
	unsigned long m;		

	for (s = 0, m = surface->dwRBitMask; !(m & 1); s++, m >>= 1);
	r_gun->mask = surface->dwRBitMask;
	r_gun->shift = s;
	r_gun->scale = 255 / (surface->dwRBitMask >> s);

	for (s = 0, m = surface->dwGBitMask; !(m & 1); s++, m >>= 1);
	g_gun->mask = surface->dwGBitMask;
	g_gun->shift = s;
	g_gun->scale = 255 / (surface->dwGBitMask >> s);

	for (s = 0, m = surface->dwBBitMask; !(m & 1); s++, m >>= 1);
	b_gun->mask = surface->dwBBitMask;
	b_gun->shift = s;
	b_gun->scale = 255 / (surface->dwBBitMask >> s);

	a_gun->mask = surface->dwRGBAlphaBitMask;

	// UP: Filter out cases which cause infinite loops
	if ((surface->dwFlags & DDPF_ALPHAPIXELS) && (surface->dwRGBAlphaBitMask != 0) ) 
	{	
		// UP: This is the exact line causing problems - it forms an infinite loop
		// under the right conditions so the above if statement has been modified to filter
		// out these conditions.
		for (s = 0, m = surface->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1); 

		// Friendly debugging loop
//		for (s = 0, m = surface->dwRGBAlphaBitMask; !(m & 1); s++)
//		{
//			m >>= 1;
//		}

		a_gun->shift = s;
		a_gun->scale = 255 / (surface->dwRGBAlphaBitMask >> s);			
	} 
	else 
	{
		a_gun->shift = 0;
		a_gun->scale = 256;
	}
}

D3DMATERIAL8 material;

// This is what remains of the old d3d5 init function
bool d3d_init_device(int screen_width, int screen_height)
{
	HWND hwnd = (HWND)os_get_window();

	if ( !hwnd )	{
		strcpy(Device_init_error, "Could not get application window handle");
		return false;
	}

	// windowed
	if(D3D_window)
	{
		SetWindowPos(hwnd, HWND_TOP, 0, 0, gr_screen.max_w, gr_screen.max_h, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);	
		
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w-1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h-1;
	} else {
		// Prepare the window to go full screen
	#ifndef NDEBUG
		mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		SetWindowPos( hwnd, HWND_TOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		D3D_cursor_clip_rect.left = work_rect.left;
		D3D_cursor_clip_rect.top = work_rect.top;
		D3D_cursor_clip_rect.right = work_rect.left + gr_screen.max_w - 1;
		D3D_cursor_clip_rect.bottom = work_rect.top + gr_screen.max_h - 1;
	#else
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		// SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ), 0 );	
		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w - 1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h - 1;
	#endif
	}

	d3d_clip_cursor(1);

	d3d_setup_format_components(&NonAlphaTextureFormat, &Gr_t_red, &Gr_t_green, &Gr_t_blue, &Gr_t_alpha);
	d3d_setup_format_components(&AlphaTextureFormat, &Gr_ta_red, &Gr_ta_green, &Gr_ta_blue, &Gr_ta_alpha);

	mprintf(( "Alpha texture format = " ));
	d3d_dump_format(&AlphaTextureFormat);

	mprintf(( "Non-alpha texture format = " ));
	d3d_dump_format(&NonAlphaTextureFormat);

#if 0
	mprintf(( "Screen format = " ));
	d3d_dump_format(&ScreenFormat);
#endif

	{
		int not_good = 0;

		char missing_features[128*1024];

		strcpy( missing_features, XSTR("Your video card is missing the following features required by FreeSpace:\r\n\r\n",623) );

		// fog
		if ( !(d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX) && !(d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)){
			strcat( missing_features, XSTR("Vertex fog or Table fog\r\n", 1499) );
			not_good++;			
		}		
				
		// Texture blending values
		if ( !(d3d_caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE  ))	{
			strcat( missing_features, XSTR("Texture blending mode = Modulate\r\n", 624) );
			not_good++;
		}

		if ( !(d3d_caps.SrcBlendCaps & (D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_BOTHSRCALPHA)) )	{
			strcat( missing_features, XSTR("Source blending mode = SRCALPHA or BOTHSRCALPHA\r\n", 625) );
			not_good++;
		}

		// Dest blending values
		if ( !(d3d_caps.DestBlendCaps & (D3DPBLENDCAPS_INVSRCALPHA|D3DPBLENDCAPS_BOTHINVSRCALPHA)) )	{
			strcat( missing_features, XSTR("Destination blending mode = INVSRCALPHA or BOTHINVSRCALPHA\r\n",626) );
			not_good++;
		}
	
#if 0
		// If card is Mystique 220, turn off modulaalpha since it doesn't work...
		if ( Largest_alpha < 4 )	{
			lpDevDesc->dpcTriCaps.dwTextureBlendCaps &= (~D3DPTBLENDCAPS_MODULATEALPHA);
		}
#endif

		if ( not_good )	{
			gr_d3d_cleanup();
			MessageBox( NULL, missing_features, XSTR("Missing Features",621), MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
			exit(1);
		}	
	}

	// fog info - for now we'll prefer table fog over vertex fog
	D3D_fog_mode = 1; 

	// if the user wants to force w-fog, maybe do it	
	if(os_config_read_uint(NULL, "ForceWFOG", 0) && (d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE))
	{
		D3D_fog_mode = 2;
	}	
	// if the card does not have vertex fog, but has table fog, let it go
	if(!(d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX) && (d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE))
	{
		D3D_fog_mode = 2;
	}

	DBUGFILE_OUTPUT_1("D3D_fog_mode %d",D3D_fog_mode);

	// setup proper render state
	d3d_set_initial_render_state();	

	if(!Cmdline_nohtl) {

		ZeroMemory(&material,sizeof(D3DMATERIAL8));
		material.Diffuse.r = 1.0f;
		material.Diffuse.g = 1.0f;
		material.Diffuse.b = 1.0f;
		material.Diffuse.a = 1.0f;

		material.Ambient.r = 1.0f;
		material.Ambient.g = 1.0f;
		material.Ambient.b = 1.0f;
		material.Ambient.a = 1.0f;

		material.Emissive.r = 0.0f;
		material.Emissive.g = 0.0f;
		material.Emissive.b = 0.0f;
		material.Emissive.a = 0.0f;

 		material.Specular.r = 1.0f;
		material.Specular.g = 1.0f;
		material.Specular.b = 1.0f;
		material.Specular.a = 1.0f;

		material.Power = 16.0f;

		lpD3DDevice->SetMaterial(&material);

	}

	mprintf(( "Direct3D Initialized OK!\n" ));

	D3D_inited = 1;	
	return true;
}

void gr_d3d_flip()
{
	if(!D3D_activate) return;
	int mx, my;	
	
	// Attempt to allow D3D8 to recover from task switching
	if(lpD3DDevice->TestCooperativeLevel() != D3D_OK) {
		d3d_lost_device();
	}

	gr_reset_clip();	

	mouse_eval_deltas();

	if ( mouse_is_visible() )	{				
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		
		if ( Gr_cursor == -1 )	{
			#ifndef NDEBUG
				gr_set_color(255,255,255);
				gr_line( mx, my, mx+7, my + 7 );
				gr_line( mx, my, mx+5, my );
				gr_line( mx, my, mx, my+5 );
			#endif
		} else {	
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my );
		}		
	} 	

	d3d_stop_frame();

	d3d_tcache_frame();

	d3d_start_frame();

}

void gr_d3d_flip_cleanup()
{
	d3d_stop_frame();
}


void gr_d3d_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_d3d_cleanup()
{
	if (!D3D_inited) return;

	d3d_tcache_cleanup();	

	// release surfaces
	d3d_release_rendering_objects();

	if ( lpD3D ) {
		lpD3D->Release();
		lpD3D = NULL; 
	}

	// restore windows clipping rectangle
 	ClipCursor(NULL);
	D3D_inited = 0;
}


void gr_d3d_fade_in(int instantaneous)
{
}

void gr_d3d_fade_out(int instantaneous)
{
}

IDirect3DSurface8 *Gr_saved_surface = NULL;

// This needs to be updated to DX8
int gr_d3d_save_screen()
{
	if(!D3D_activate) return -1;
	gr_reset_clip();

	// Lets not bother with this if its a window or it causes a debug DX8 error
	if(D3D_window == 1) {
		return 0;
	}

	if ( Gr_saved_surface )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	IDirect3DSurface8 *front_buffer_a8r8g8b8 = NULL;

	// Problem that we can only get front buffer in A8R8G8B8
	mprintf(("Creating surface for front buffer of size: %d %d",gr_screen.max_w, gr_screen.max_h));
	if(FAILED(lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, D3DFMT_A8R8G8B8, &front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		return -1;
	}

	if(FAILED(lpD3DDevice->GetFrontBuffer(front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to get front buffer");
		goto Failed;
	}

	// Get the back buffer format
	DDPIXELFORMAT dest_format;
	color_gun r_gun, g_gun, b_gun, a_gun;

	d3d_fill_pixel_format( &dest_format, d3dpp.BackBufferFormat);
	d3d_setup_format_components(&dest_format, &r_gun, &g_gun, &b_gun, &a_gun);


	// Create a surface of a compatable type
	if(FAILED(lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, d3dpp.BackBufferFormat, &Gr_saved_surface))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		goto Failed;
	}

	// Make a copy of the damn thing
	D3DLOCKED_RECT src_rect;
	D3DLOCKED_RECT dst_rect;
	
	if(FAILED(Gr_saved_surface->LockRect(&dst_rect, NULL, 0))) { 

		DBUGFILE_OUTPUT_0("Failed to lock save buffer");
		goto Failed;

	}

	if(FAILED(front_buffer_a8r8g8b8->LockRect(&src_rect, NULL, D3DLOCK_READONLY))) {

		DBUGFILE_OUTPUT_0("Failed to lock front buffer");
		goto Failed;

	}

	typedef struct { unsigned char b,g,r,a; } TmpC;

	if(D3D_32bit) {
		for(int j = 0; j < gr_screen.max_h; j++) {
		
			TmpC *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			uint *dst = (uint *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < gr_screen.max_w; i++) {
			 	dst[i] = 0;
				dst[i] |= (uint)(( (int) src[i].r / r_gun.scale ) << r_gun.shift);
				dst[i] |= (uint)(( (int) src[i].g / g_gun.scale ) << g_gun.shift);
				dst[i] |= (uint)(( (int) src[i].b / b_gun.scale ) << b_gun.shift);
			}
		}
	} else {
		for(int j = 0; j < gr_screen.max_h; j++) {
		
			TmpC   *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			ushort *dst = (ushort *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < gr_screen.max_w; i++) {
			 	dst[i] = 0;
				dst[i] |= (ushort)(( (int) src[i].r / r_gun.scale ) << r_gun.shift);
				dst[i] |= (ushort)(( (int) src[i].g / g_gun.scale ) << g_gun.shift);
				dst[i] |= (ushort)(( (int) src[i].b / b_gun.scale ) << b_gun.shift);
			}
		}
	}

	front_buffer_a8r8g8b8->UnlockRect();
	Gr_saved_surface->UnlockRect();

	if(front_buffer_a8r8g8b8) {
		front_buffer_a8r8g8b8->Release();
	}

	return 0;

Failed:

	if(front_buffer_a8r8g8b8) {
		front_buffer_a8r8g8b8->Release();
	}

	extern void gr_d3d_free_screen(int id);
	gr_d3d_free_screen(0);
	return -1;
}

void gr_d3d_restore_screen(int id)
{
	gr_reset_clip();

	if ( !Gr_saved_surface )	{
		gr_clear();
		return;
	}

	// attempt to replace DX5 code with DX8 
	IDirect3DSurface8 *dest_buffer = NULL;
		
	if(FAILED(lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &dest_buffer)))
	{
		DBUGFILE_OUTPUT_0("FAiled");
		return;
	}

	if(FAILED(lpD3DDevice->CopyRects(Gr_saved_surface, NULL, 0, dest_buffer, NULL)))
	{
		DBUGFILE_OUTPUT_0("FAiled");
	}

	dest_buffer->Release();
}

void gr_d3d_free_screen(int id)
{
	if ( Gr_saved_surface )	{
		Gr_saved_surface->Release();
		Gr_saved_surface = NULL;
	}
}

static int D3d_dump_frames = 0;
static ubyte *D3d_dump_buffer = NULL;
static int D3d_dump_frame_number = 0;
static int D3d_dump_frame_count = 0;
static int D3d_dump_frame_count_max = 0;
static int D3d_dump_frame_size = 0;

void gr_d3d_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if ( D3d_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}	
	D3d_dump_frames = 1;
	D3d_dump_frame_number = first_frame;
	D3d_dump_frame_count = 0;
	D3d_dump_frame_count_max = frames_between_dumps;
	D3d_dump_frame_size = gr_screen.max_w * gr_screen.max_h * 2;
	
	if ( !D3d_dump_buffer ) {
		int size = D3d_dump_frame_count_max * D3d_dump_frame_size;
		D3d_dump_buffer = (ubyte *)malloc(size);
		if ( !D3d_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

void gr_d3d_flush_frame_dump()
{
	extern int tga_compress(char *out, char *in, int bytecount);

	int i,j;
	char filename[MAX_PATH_LEN], *movie_path = ".\\";
	ubyte outrow[1024*3*4];

	if ( gr_screen.max_w > 1024)	{
		mprintf(( "Screen too wide for frame_dump\n" ));
		return;
	}

	for (i = 0; i < D3d_dump_frame_count; i++) {

		int w = gr_screen.max_w;
		int h = gr_screen.max_h;

		sprintf(filename, NOX("%sfrm%04d.tga"), movie_path, D3d_dump_frame_number );
		D3d_dump_frame_number++;

		CFILE *f = cfopen(filename, "wb");

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 10, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)w, f );	//	Width;
		cfwrite_ushort( (ushort)h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		// Go through and write our pixels
		for (j=0;j<h;j++)	{
			ubyte *src_ptr = D3d_dump_buffer+(i*D3d_dump_frame_size)+(j*w*2);

			int len = tga_compress( (char *)outrow, (char *)src_ptr, w*sizeof(short) );

			cfwrite(outrow,len,1,f);
		}

		cfclose(f);

	}

	D3d_dump_frame_count = 0;
}

void gr_d3d_dump_frame_stop()
{

	if ( !D3d_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	gr_d3d_flush_frame_dump();
	
	D3d_dump_frames = 0;
	if ( D3d_dump_buffer )	{
		free(D3d_dump_buffer);
		D3d_dump_buffer = NULL;
	}
}

void gr_d3d_dump_frame()
{
	D3d_dump_frame_count++;

	if ( D3d_dump_frame_count == D3d_dump_frame_count_max ) {
		gr_d3d_flush_frame_dump();
	}
}	

/**
 * Empty function
 *
 * @return uint, always 1
 */
uint gr_d3d_lock()
{
	return 1;
}

/**
 * Empty function
 *
 * @return void
 */
void gr_d3d_unlock()
{
}

/**
 * Set fog
 *
 * @param int fog_mode 
 * @param int r 
 * @param int g 
 * @param int b 
 * @param float fog_near 
 * @param float fog_far
 * @return void
 */
void gr_d3d_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
	D3DCOLOR color = 0;	

	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));	

	// turning fog off
	if(fog_mode == GR_FOGMODE_NONE){
		// only change state if we need to
		if(gr_screen.current_fog_mode != fog_mode){
			d3d_SetRenderState(D3DRS_FOGENABLE, FALSE );		
		}
		gr_screen.current_fog_mode = fog_mode;

		// to prevent further state changes
		return;
	}

	// maybe switch fogging on
	if(gr_screen.current_fog_mode != fog_mode){		
		d3d_SetRenderState(D3DRS_FOGENABLE, TRUE);	

		// if we're using table fog, enable table fogging
		if(!Cmdline_nohtl){
			d3d_SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );			
		}

		gr_screen.current_fog_mode = fog_mode;	
	}	

	// is color changing?
	if( (gr_screen.current_fog_color.red != r) || (gr_screen.current_fog_color.green != g) || (gr_screen.current_fog_color.blue != b) ){
		// store the values
		gr_d3d_init_color( &gr_screen.current_fog_color, r, g, b );

		color = D3DCOLOR_XRGB(r, g, b);
		d3d_SetRenderState(D3DRS_FOGCOLOR, color);	
	}		

	// planes changing?
	if( (fog_near >= 0.0f) && (fog_far >= 0.0f) && ((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)) ){		
		gr_screen.fog_near = fog_near;		
		gr_screen.fog_far = fog_far;					

		// only generate a new fog table if we have to (wfog/table fog mode)
		if(!Cmdline_nohtl){
			d3d_SetRenderState( D3DRS_FOGSTART, *((DWORD *)(&fog_near)));		
			d3d_SetRenderState( D3DRS_FOGEND, *((DWORD *)(&fog_far)));
		}				
	}  
}

/**
 * Set the gamma, or brightness
 *
 * @param float gamma
 * @return void
 */
void gr_d3d_set_gamma(float gamma)
{
	Gr_gamma = gamma;
	Gr_gamma_int = int(Gr_gamma*100);

	// Create the Gamma lookup table
	for (int i = 0; i < 256; i++ )	{
		int v = fl2i( pow(i2fl(i) / 255.0f, 1.0f/ Gr_gamma) * 255.0f);

		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )	{
			v = 0;
		}

		Gr_gamma_lookup[i] = v;
	}

	// Flush any existing textures
	d3d_tcache_flush();
}

/**
 * Empty function
 *
 * @param int x
 * @param int y
 * @param ubyte *pixel
 * @return void
 */
void d3d_get_pixel(int x, int y, ubyte *pixel)
{
}

/**
 * Empty function
 *
 * @param int x
 * @param int y
 * @param int *r
 * @param int *g
 * @param int *b
 * @return void
 */
void gr_d3d_get_pixel(int x, int y, int *r, int *g, int *b)
{
}

/**
 * Toggle polygon culling mode Counter-clockwise or none
 *
 * @param int cull 
 * @return void
 */
void gr_d3d_set_cull(int cull)
{
	// switch culling on or off
	if(cull){
		d3d_SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );		
	} else {
		d3d_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );				
	}
}

/**
 * Cross fade
 *
 * @param int bmap1 
 * @param int bmap2 
 * @param int x1 
 * @param int y1 
 * @param int x2 
 * @param int y2 
 * @param float pct
 * @return void
 */
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	if ( pct <= 50 )	{
		gr_set_bitmap(bmap1);
		gr_bitmap(x1, y1);
	} else {
		gr_set_bitmap(bmap2);
		gr_bitmap(x2, y2);
	}	
}

/**
 * Empty function
 *
 * @param int filter
 * @return void
 */
void gr_d3d_filter_set(int filter)
{
}

/**
 * Set clear color
 *
 * @param int r
 * @param int g
 * @param int b
 * @return void
 */
void gr_d3d_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

// JAS: Need to turn optimizations off or Alan's machine, with 3dfx direct3d hangs...
//#pragma optimize("",off)		

/**
 * Determines value of D3d_rendition_uvs
 *
 * @return void
 */
void d3d_detect_texture_origin_32()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	uint pix1a(0), pix2a(0);
	uint pix1b(0), pix2b(0);

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	D3d_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3d_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		D3d_rendition_uvs = 1;
	} else {
		D3d_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", D3d_rendition_uvs ));
}
	
/**
 * Determines value of D3d_rendition_uvs
 *
 * @return void
 */
void d3d_detect_texture_origin_16()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	ushort pix1a(0), pix2a(0);
	ushort pix1b(0), pix2b(0);

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	D3d_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3d_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		D3d_rendition_uvs = 1;
	} else {
		D3d_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", D3d_rendition_uvs ));
}

// Not sure if we need this any more
void gr_d3d_get_region(int front, int w, int h, ubyte *data)
{	
	HRESULT hr;

	// No support for getting the front buffer
	if(front) {
		mprintf(("No support for front buffer"));
		return;
	}

	IDirect3DSurface8 *back_buffer = NULL;

	hr = lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	if( FAILED(hr))
	{
		mprintf(("Unsuccessful GetBackBuffer",d3d_error_string(hr)));
		return;
	}

	D3DLOCKED_RECT buffer_desc;
	hr = back_buffer->LockRect(&buffer_desc, NULL, D3DLOCK_READONLY );
	if( FAILED(hr))
	{
		mprintf(("Unsuccessful buffer lock",d3d_error_string(hr)));
		return;
	}

	ubyte *dptr = data;	
	ubyte *rptr = (ubyte*) buffer_desc.pBits;  
	int pitch   = buffer_desc.Pitch;// / gr_screen.bytes_per_pixel;   

	for (int i=0; i<h; i++ )	{
		ubyte *sptr = (ubyte*)&rptr[ i * pitch ];

		// don't think we need to swizzle here ...
		for(int j=0; j<w; j++ )	{
		  	memcpy(dptr, sptr, gr_screen.bytes_per_pixel);
			dptr += gr_screen.bytes_per_pixel;
			sptr += gr_screen.bytes_per_pixel;

		}
	}	

	back_buffer->UnlockRect();
	back_buffer->Release();
}

/**
 * Determines value of D3D_line_offset
 *
 * @return void
 */
void d3d_detect_line_offset_32()
{
	extern float D3D_line_offset;

	color ac;
	uint pix1a(0), pix2a(0);
	uint pix1b(0), pix2b(0);

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

/**
 * Determines value of D3D_line_offset
 *
 * @return void
 */
void d3d_detect_line_offset_16()
{
	extern float D3D_line_offset;

	color ac;
	ushort pix1a(0), pix2a(0);
	ushort pix1b(0), pix2b(0);

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

//#pragma optimize("",on)	

/**
 * D3D8 Launcher func: Return bit type for modes, those not listed or simply not valid for FS2
 *
 * @return int, 32, 16 or 0 if not valid 
 * @param D3DFORMAT type 
 */
int d3d_get_mode_bit(D3DFORMAT type)
{
	switch(type)
	{
		case D3DFMT_X8R8G8B8: 
		case D3DFMT_A8R8G8B8:		
		//case D3DFMT_A2B10G10R10:	
			return 32;
			
		case D3DFMT_R8G8B8:
		case D3DFMT_R5G6B5:   
		case D3DFMT_X1R5G5B5: 
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A1R5G5B5:		
		case D3DFMT_A4R4G4B4:		
		case D3DFMT_A8R3G3B2:		
			return 16;
	}

	return 0;
}

/**
 * This checks that the given texture format is supported in the chosen adapter and mode
 *
 * @return bool
 * @param D3DFORMAT tformat
 */
bool d3d_texture_format_is_supported(D3DFORMAT tformat, int adapter, D3DDISPLAYMODE *mode)
{
	HRESULT hr;

	hr = lpD3D->CheckDeviceFormat(
			adapter,
			D3DDEVTYPE_HAL,
			mode->Format,
			0,
			D3DRTYPE_TEXTURE,
			tformat);

	return SUCCEEDED(hr); 
}

/**
 * Fills the old style direct draw DDPIXELFORMAT with details needed for later
 * We are not using direct draw, just making use of one of its structures 
 * The shift values are used to convert textures from load set to correct texture format
 *
 * @return void
 * @param DDPIXELFORMAT *pixelf
 * @param D3DFORMAT tformat
 */
void d3d_fill_pixel_format(DDPIXELFORMAT *pixelf, D3DFORMAT tformat)
{
	switch(tformat)
	{
		case D3DFMT_X8R8G8B8:
			pixelf->dwRGBBitCount      = 32;
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;       
			pixelf->dwFlags			   = 0;
			pixelf->dwRGBAlphaBitMask  = 0;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_X8R8G8B8");
			break;
		case D3DFMT_R8G8B8:
			pixelf->dwRGBBitCount      = 24;   
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;           
			pixelf->dwFlags			   = 0;
			pixelf->dwRGBAlphaBitMask  = 0;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_R8G8B8");
			break;
		case D3DFMT_X1R5G5B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0x7c00;      
			pixelf->dwGBitMask         = 0x3e0;      
			pixelf->dwBBitMask         = 0x1f;      
			pixelf->dwFlags			   = 0;
			pixelf->dwRGBAlphaBitMask  = 0;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_X1R5G5B5");
			break;
		case D3DFMT_R5G6B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf800;      
			pixelf->dwGBitMask         = 0x7e0;      
			pixelf->dwBBitMask         = 0x1f;      
			pixelf->dwFlags			   = 0;
			pixelf->dwRGBAlphaBitMask  = 0;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_R5G6B5");
			break;
		case D3DFMT_X4R4G4B4:	 
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf00;      
			pixelf->dwGBitMask         = 0xf0;      
			pixelf->dwBBitMask         = 0xf;
			pixelf->dwFlags			   = 0;
			pixelf->dwRGBAlphaBitMask  = 0;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_X4R4G4B4");
			break;
		case D3DFMT_A8R8G8B8:		
			pixelf->dwRGBBitCount      = 32;   
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;           
			pixelf->dwRGBAlphaBitMask  = 0xff000000;  
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_A8R8G8B8");
			break;
		case D3DFMT_A1R5G5B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0x7c00;      
			pixelf->dwGBitMask         = 0x3e0;       
			pixelf->dwBBitMask         = 0x1f;        
			pixelf->dwRGBAlphaBitMask  = 0x8000;
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_A1R5G5B5");
			break;
		case D3DFMT_A4R4G4B4:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf00;      
			pixelf->dwGBitMask         = 0xf0;       
			pixelf->dwBBitMask         = 0xf;        
			pixelf->dwRGBAlphaBitMask  = 0xf000;;    
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_A4R4G4B4");
			break;
		case D3DFMT_A8R3G3B2:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xe0;       
			pixelf->dwGBitMask         = 0x1c;      
			pixelf->dwBBitMask         = 0x3;      
			pixelf->dwRGBAlphaBitMask  = 0xff00;
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_A8R3G3B2");
			break;
		/*case D3DFMT_A2B10G10R10:
			pixelf->dwRGBBitCount      = 32;   
			pixelf->dwRBitMask         = 0x3ff00000;      
			pixelf->dwGBitMask         = 0xffc00;      
			pixelf->dwBBitMask         = 0x3ff;      
			pixelf->dwRGBAlphaBitMask  = 0xc0000000;
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			DBUGFILE_OUTPUT_0("Using: D3DFMT_A2B10G10R10");
			break;*/
	}
}

/**
 * Works out the best texture formats to use in the chosen mode
 *
 * @return void
 */
void d3d_determine_texture_formats(int adapter, D3DDISPLAYMODE *mode)
{
	const int num_non_alpha = 3;
	const int num_alpha     = 4;

	default_32_non_alpha_tformat = D3DFMT_UNKNOWN;
	default_non_alpha_tformat	 = D3DFMT_UNKNOWN;
	default_32_alpha_tformat	 = D3DFMT_UNKNOWN;
	default_alpha_tformat		 = D3DFMT_UNKNOWN;

	// Non alpha (listed from best to worst)
	D3DFORMAT non_alpha_list[num_non_alpha] =
	{
		D3DFMT_A1R5G5B5,		
		D3DFMT_A4R4G4B4,		
		D3DFMT_A8R3G3B2,
	};

	// Alpha (listed from best to worst)
	D3DFORMAT alpha_list[num_alpha] =
	{
		D3DFMT_A4R4G4B4,		
		D3DFMT_A1R5G5B5,		
		D3DFMT_A8R3G3B2,		
	};

	// Go through the alpha list and find a texture format of a valid depth
	// and is supported in this adapter mode
	for(int i = 0; i < num_alpha; i++) {
		if(d3d_get_mode_bit(alpha_list[i]) != 16) {
			continue;
		}

		if(d3d_texture_format_is_supported(alpha_list[i], adapter, mode) == true) {
			default_alpha_tformat = alpha_list[i];
			break;
		}
	}

	// If this is unknown this has failed
	if(default_alpha_tformat == D3DFMT_UNKNOWN) {
		DBUGFILE_OUTPUT_0("alpha texture format not selected");
	} else {
		d3d_fill_pixel_format(&AlphaTextureFormat, default_alpha_tformat);
	}

	// Try to get 32 bit texture formats
	if(D3D_32bit) {
		if(d3d_texture_format_is_supported(D3DFMT_X8R8G8B8, adapter, mode)) {
			default_32_non_alpha_tformat = D3DFMT_X8R8G8B8;
		}

		if(d3d_texture_format_is_supported(D3DFMT_A8R8G8B8, adapter, mode)) {
			default_32_alpha_tformat = D3DFMT_A8R8G8B8;
		}
	}

		// Go through the non alpha list and find a texture format of a valid depth
	// and is supported in this adapter mode
	for(i = 0; i < num_non_alpha; i++)
	{
		if(d3d_get_mode_bit(non_alpha_list[i]) != 16)
		{
			continue;
		}

		if(d3d_texture_format_is_supported(non_alpha_list[i], adapter, mode) == true)
		{
			default_non_alpha_tformat = non_alpha_list[i];
			break;
		}
	}

	// If this is unknown this has failed
	if(default_non_alpha_tformat == D3DFMT_UNKNOWN)
	{
		DBUGFILE_OUTPUT_0("non alpha texture format not selected");
	}
	else
	{
		// Um hack here, forget about non alpha formats!
		d3d_fill_pixel_format(&NonAlphaTextureFormat, default_non_alpha_tformat);
	}


	// If in 16 bit or 32 attempt failed fall back to 16 bit
	if(default_32_non_alpha_tformat == D3DFMT_UNKNOWN) {
		default_32_non_alpha_tformat = default_non_alpha_tformat;
	}
	
	if(default_32_alpha_tformat == D3DFMT_UNKNOWN) {
		default_32_alpha_tformat = default_alpha_tformat;    
	}

	DBUGFILE_OUTPUT_1("default_32_alpha_tformat %d",default_32_alpha_tformat);
}

//*******Vertex buffer stuff*******//
//-Bobboau
struct Vertex_buffer{
	Vertex_buffer(): ocupied(false), n_prim(0){};
	bool ocupied;
	short int n_prim;
	IDirect3DVertexBuffer8 *buffer;
};

#define MAX_SUBOBJECTS 64
#define MAX_BUFFERS MAX_POLYGON_MODELS*MAX_SUBOBJECTS*(MAX_MODEL_TEXTURES/4)
void shift_active_lights(int pos);
void pre_render_lights_init();

Vertex_buffer vertex_buffer[MAX_BUFFERS];
extern matrix View_matrix;
extern vector View_position;
extern matrix Eye_matrix;
extern vector Eye_position;
extern vector Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;
int n_active_lights = 0;

//finds the first unocupyed buffer
int find_first_empty_buffer(){
	for(int i = 0; i<MAX_BUFFERS; i++)if(!vertex_buffer[i].ocupied)return i;
	return -1;
}

//makes the vertex buffer, returns an index to it
int gr_d3d_make_buffer(poly_list *list){
	int idx = find_first_empty_buffer();

	if(idx > -1){
		IDirect3DVertexBuffer8 **buffer = &vertex_buffer[idx].buffer;

		d3d_CreateVertexBuffer(D3DVT_VERTEX, (list->n_poly*3), NULL, (void**)buffer);

		D3DVERTEX *v, *V;
		vertex *L;
		vector *N;

		vertex_buffer[idx].buffer->Lock(0, 0, (BYTE **)&v, NULL);
		for(int k = 0; k<list->n_poly; k++){
			for(int j = 0; j < 3; j++){
				V = &v[(k*3)+j];
				L = &list->vert[k][j];
				N = &list->norm[k][j];

				V->sx = L->x;
				V->sy = L->y;
				V->sz = L->z;

				V->tu = L->u;
				V->tv = L->v;
				V->tu2 = L->u;
				V->tv2 = L->v;
	
				V->nx = N->xyz.x;
				V->ny = N->xyz.y;
				V->nz = N->xyz.z;
			}
		}

		vertex_buffer[idx].buffer->Unlock();

		vertex_buffer[idx].ocupied = true;
		vertex_buffer[idx].n_prim = list->n_poly;
	}
	return idx;
}
	
//kills buffers dead!
void gr_d3d_destroy_buffer(int idx){
	vertex_buffer[idx].buffer->Release();
	vertex_buffer[idx].ocupied = false;
}

//renders a buffer	
/*void gr_d3d_render_buffer(int idx){

	if(!vertex_buffer[idx].ocupied)return;
	float u_scale = 1.0f, v_scale = 1.0f;

	gr_alpha_blend ab;
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_NONE)	ab = ALPHA_BLEND_NONE;
	else if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	ab = ALPHA_BLEND_ALPHA_ADDITIVE;

	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0);
	gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);


//	set_stage_for_defuse();
	if(GLOWMAP > -1){
		//glowmapped
		gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);
		set_stage_for_glow_mapped_defuse();
	}else{
		//non glowmapped
		d3d_SetTexture(1, NULL);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		set_stage_for_defuse();
	}

	lpD3DDevice->SetStreamSource(0, vertex_buffer[idx].buffer, sizeof(D3DVERTEX));

	lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);

	//spec mapping
	if(SPECMAP > 0){
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
	//			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);
				return;
			}

		if(set_stage_for_spec_mapped()){
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
//			lpD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE );
			lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		}
	}

//	gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);

}*/
//set_stage_for_spec_glow_mapped
void gr_d3d_render_buffer(int idx)
{
	// Sets the current alpha of the object
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER){
		material.Ambient.a = gr_screen.current_alpha;
		material.Diffuse.a = gr_screen.current_alpha;
		material.Specular.a = gr_screen.current_alpha;
		material.Emissive.a = gr_screen.current_alpha;
	}else{
		material.Ambient.a = 1.0;
		material.Diffuse.a = 1.0;
		material.Specular.a = 1.0;
		material.Emissive.a = 1.0f;
	}
	lpD3DDevice->SetMaterial(&material);


	if(!vertex_buffer[idx].ocupied)return;
	float u_scale = 1.0f, v_scale = 1.0f;

	gr_alpha_blend ab = ALPHA_BLEND_NONE;
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	
		ab = ALPHA_BLEND_ALPHA_ADDITIVE;

	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0);
	gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);

	pre_render_lights_init();
	shift_active_lights(0);

//	set_stage_for_defuse();
	if(GLOWMAP > -1){
		//glowmapped
	 	gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
	 	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);
	 	set_stage_for_glow_mapped_defuse();
	}else{
		//non glowmapped
		d3d_SetTexture(1, NULL);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		set_stage_for_defuse();
	}

	int passes = 1;//(n_active_lights/d3d_caps.MaxActiveLights);
	d3d_SetVertexShader(D3DVT_VERTEX);

	lpD3DDevice->SetStreamSource(0, vertex_buffer[idx].buffer, sizeof(D3DVERTEX));

	lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
	gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );

	set_stage_for_defuse();
	for(int i = 1; i<passes+1; i++){
		shift_active_lights(i);
		lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
	}

	pre_render_lights_init();
	shift_active_lights(0);

	// Bob, put a return here and look how well the fog works

	//spec mapping
	if(SPECMAP > 0){
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
	//			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);
				return;
			}

		if(set_stage_for_spec_mapped()){
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
//			lpD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE );
			lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
			for(int i = 1; i<passes+1; i++){
				shift_active_lights(i);
				lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
			}
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		}
	}

//	gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);

}

//D3DLIGHT8 I_light;
extern float Model_Interp_scale_x;	//added these three for warpin stuff-Bobbau
extern float Model_Interp_scale_y;
extern float Model_Interp_scale_z;

void gr_d3d_start_instance_matrix(){
	D3DXMATRIX mat, scale;

/*	lpD3DDevice->GetTransform(D3DTS_PROJECTION, &old_p);
	lpD3DDevice->GetTransform(D3DTS_WORLD, &old_w);
	lpD3DDevice->GetTransform(D3DTS_VIEW, &old_v);
*/
	//hmm... seems I don't need these
	D3DXMatrixPerspectiveFovLH(&mat, (4.0f/9.0f)*(D3DX_PI)*View_zoom, Canv_w2/Canv_h2, 0.2f, 30000.0f);
	lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);

	D3DXMATRIX world(
		Object_matrix.vec.rvec.xyz.x, Object_matrix.vec.rvec.xyz.y, Object_matrix.vec.rvec.xyz.z, 0,
		Object_matrix.vec.uvec.xyz.x, Object_matrix.vec.uvec.xyz.y, Object_matrix.vec.uvec.xyz.z, 0,
		Object_matrix.vec.fvec.xyz.x, Object_matrix.vec.fvec.xyz.y, Object_matrix.vec.fvec.xyz.z, 0,
		Object_position.xyz.x, Object_position.xyz.y, Object_position.xyz.z, 1);
	D3DXMatrixScaling(&scale, Model_Interp_scale_x, Model_Interp_scale_y, Model_Interp_scale_z);
	D3DXMatrixMultiply(&mat, &scale, &world);
	lpD3DDevice->SetTransform(D3DTS_WORLD, &mat);

	D3DXMATRIX view(
		Eye_matrix.vec.rvec.xyz.x, Eye_matrix.vec.rvec.xyz.y, Eye_matrix.vec.rvec.xyz.z, 0,
		Eye_matrix.vec.uvec.xyz.x, Eye_matrix.vec.uvec.xyz.y, Eye_matrix.vec.uvec.xyz.z, 0,
		Eye_matrix.vec.fvec.xyz.x, Eye_matrix.vec.fvec.xyz.y, Eye_matrix.vec.fvec.xyz.z, 0,
		Eye_position.xyz.x, Eye_position.xyz.y, Eye_position.xyz.z, 1);


	D3DXMatrixIdentity(&mat);
	D3DXMatrixInverse(&mat, NULL, &view);
	lpD3DDevice->SetTransform(D3DTS_VIEW, &mat);
/*
	ZeroMemory(&I_light, sizeof(D3DLIGHT8));
	I_light.Type = D3DLIGHT_DIRECTIONAL;
	I_light.Diffuse.r = 0.5f;
	I_light.Diffuse.g = 0.5f;
	I_light.Diffuse.b = 0.5f;
	I_light.Diffuse.a = 1.0f;
	I_light.Ambient.r = 0.25f;
	I_light.Ambient.g = 0.25f;
	I_light.Ambient.b = 0.25f;
	I_light.Ambient.a = 1.0f;
	I_light.Specular.r = 0.5f;
	I_light.Specular.g = 0.5f;
	I_light.Specular.b = 0.5f;
	I_light.Specular.a = 1.0f;
	I_light.Position = D3DXVECTOR3(0,1,0);
	I_light.Direction = D3DXVECTOR3(0,-1,0);
	I_light.Range = 1000.0f;
*/
//	material.Power = 16;

//	lpD3DDevice->SetLight(0,&I_light);
//	lpD3DDevice->LightEnable(0,TRUE);

//	lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(16,16,16));

	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER){
		material.Ambient.a = gr_screen.current_alpha;
		material.Diffuse.a = gr_screen.current_alpha;
		material.Specular.a = gr_screen.current_alpha;
		material.Emissive.a = gr_screen.current_alpha;
	}else{
		material.Ambient.a = 1.0;
		material.Diffuse.a = 1.0;
		material.Specular.a = 1.0;
		material.Emissive.a = 1.0f;
	}
	lpD3DDevice->SetMaterial(&material);
	d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
}

void gr_d3d_end_instance_matrix(){
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	lpD3DDevice->SetTransform(D3DTS_VIEW, &mat);
	lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
	lpD3DDevice->SetTransform(D3DTS_WORLD, &mat);

	d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_set_initial_render_state();
}

//*****Lighting Stuff*****
//-Bobboau
#define MAX_LIGHTS 256
int hardware_slot[8];
bool lighting_enabled = true;
int HWLightSlot = 0;

#define LT_DIRECTIONAL	0		// A light like a sun
#define LT_POINT		1		// A point light, like an explosion
#define LT_TUBE			2		// A tube light, like a fluorescent light

void FSLight2DXLight(D3DLIGHT8 *DXLight,light_data *FSLight) {

	//Copy the vars into a dx compatible struct
	DXLight->Diffuse.r = FSLight->r * FSLight->intensity;
	DXLight->Diffuse.g = FSLight->g * FSLight->intensity;
	DXLight->Diffuse.b = FSLight->b * FSLight->intensity;
	DXLight->Specular.r = FSLight->spec_r * FSLight->intensity;
	DXLight->Specular.g = FSLight->spec_g * FSLight->intensity;
	DXLight->Specular.b = FSLight->spec_b * FSLight->intensity;
	DXLight->Diffuse.a = 1.0f;
	DXLight->Specular.a = 1.0f;
	DXLight->Ambient.r = 0.0f;
	DXLight->Ambient.g = 0.0f;
	DXLight->Ambient.b = 0.0f;
	DXLight->Ambient.a = 1.0f;


	//If the light is a directional light
	if(FSLight->type == LT_DIRECTIONAL) {
		DXLight->Type = D3DLIGHT_DIRECTIONAL;
		DXLight->Position.x = 0.0f;
		DXLight->Position.y = 0.0f;
		DXLight->Position.z = 0.0f;

		DXLight->Direction.x = FSLight->vec.xyz.x;
		DXLight->Direction.y = FSLight->vec.xyz.y;
		DXLight->Direction.z = FSLight->vec.xyz.z;
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {
		DXLight->Type = D3DLIGHT_POINT;
		DXLight->Position.x = FSLight->vec.xyz.x;
		DXLight->Position.y = FSLight->vec.xyz.y;
		DXLight->Position.z = FSLight->vec.xyz.z;
		
		//Increase the brightness of point and beam lights, as they seem to be too dark
		DXLight->Diffuse.r *= 2;
		DXLight->Diffuse.g *= 2;
		DXLight->Diffuse.b *= 2;
		DXLight->Specular.r *= 2;
		DXLight->Specular.g *= 2;
		DXLight->Specular.b *= 2;

		//They also have almost no radius...
		DXLight->Range = FSLight->rada * 64;
		DXLight->Attenuation0 = 0.0f;
		DXLight->Attenuation1 = 1.0f;
		DXLight->Attenuation2 = 0.0f;
	}

}



struct d3d_light{
	d3d_light():occupied(false), priority(1){};
	D3DLIGHT8 light;
	bool occupied;
	int priority;
};

d3d_light d3d_lights[MAX_LIGHTS];
int total_lights = 0;
bool active_list[MAX_LIGHTS];
//finds the first unocupyed light
int find_first_empty_light(){
	for(int i = 0; i<MAX_LIGHTS; i++)if(!d3d_lights[i].occupied)return i;
	return -1;
}

int currently_enabled[8] = {-1};

void pre_render_lights_init(){
	if(lighting_enabled)	lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
	else 	lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(byte(255.0f * gr_screen.current_alpha),byte(255.0f * gr_screen.current_alpha),byte(255.0f * gr_screen.current_alpha),byte(255.0f * gr_screen.current_alpha)));
	for(int i = 0; i<8; i++){
		if(currently_enabled[i] > -1)lpD3DDevice->LightEnable(currently_enabled[i],false);
		currently_enabled[i] = -1;
	}
}

void shift_active_lights(int pos){
//Stub
}

int find_first_empty_hardware_slot(){
	int l;
	for(unsigned int i = 0; i<d3d_caps.MaxActiveLights; i++){
		lpD3DDevice->GetLightEnable(i,&l);
		if(!l)return i;
	}
	return -1;
}


int	 gr_d3d_make_light(light_data* light, int idx, int priority){
//Stub
	return idx;
}

void gr_d3d_modify_light(light_data* light, int idx, int priority){
//Stub
}

void gr_d3d_destroy_light(int idx){
//Stub
}

void gr_d3d_set_light(light_data *light){

	//Init the light
	D3DLIGHT8 DXLight;
	FSLight2DXLight(&DXLight,light);

	//Increment the hw light and set it up in d3d
	HWLightSlot++;
	if(HWLightSlot <= d3d_caps.MaxActiveLights) {

		lpD3DDevice->SetLight(HWLightSlot,&DXLight);
		lpD3DDevice->LightEnable(HWLightSlot,TRUE);
	
	}
}

void gr_d3d_reset_lighting(){
	//Reset the light counter
	HWLightSlot = 0;

	//Disable all the HW lights
	for(unsigned int i = 0; i<d3d_caps.MaxActiveLights; i++){
		lpD3DDevice->LightEnable(i,FALSE);
	}
}

void gr_d3d_lighting(bool set){
	lighting_enabled = set;
//	d3d_SetRenderState(D3DRS_LIGHTING , set);
}

//**********clip plane**********/
extern int G3_user_clip;
extern vector G3_user_clip_normal;
extern vector G3_user_clip_point;

D3DXPLANE d3d_user_clip_plane;

#define VEC2DVEC(v) D3DXVECTOR3(v.xyz.x,v.xyz.y,v.xyz.z)

void d3d_start_clip(){

	// Lets be safe instead, see 'Compiler Warning (level 4) C4238'- RT 
	// D3DXPlaneFromPointNormal(&d3d_user_clip_plane, &VEC2DVEC(G3_user_clip_point), &VEC2DVEC(G3_user_clip_normal));

	D3DXVECTOR3 point(G3_user_clip_point.xyz.x,G3_user_clip_point.xyz.y,G3_user_clip_point.xyz.z); 
	D3DXVECTOR3	normal(G3_user_clip_normal.xyz.x,G3_user_clip_normal.xyz.y,G3_user_clip_normal.xyz.z);

	D3DXPlaneFromPointNormal(&d3d_user_clip_plane, &point, &normal);

	lpD3DDevice->SetClipPlane(0, d3d_user_clip_plane);
	d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , D3DCLIPPLANE0);
}

void d3d_end_clip(){
	d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , FALSE);
}

/**
 * Sets up all the graphics function pointers to the relevent d3d functions
 *
 * @return void
 */
void d3d_setup_function_pointers()
{
		// Set all the pointer to functions to the correct D3D functions
	gr_screen.gf_flip = gr_d3d_flip;
	gr_screen.gf_flip_window = gr_d3d_flip_window;
	gr_screen.gf_set_clip = gr_d3d_set_clip;
	gr_screen.gf_reset_clip = gr_d3d_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_get_color = gr_d3d_get_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_set_color_fast = gr_d3d_set_color_fast;
	gr_screen.gf_set_color = gr_d3d_set_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_init_alphacolor = gr_d3d_init_alphacolor;

	gr_screen.gf_set_bitmap = gr_d3d_set_bitmap;
	gr_screen.gf_create_shader = gr_d3d_create_shader;
	gr_screen.gf_set_shader = gr_d3d_set_shader;
	gr_screen.gf_clear = gr_d3d_clear;
	gr_screen.gf_aabitmap = gr_d3d_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d_aabitmap_ex;

	gr_screen.gf_rect = gr_d3d_rect;
	gr_screen.gf_shade = gr_d3d_shade;
	gr_screen.gf_string = gr_d3d_string;
	gr_screen.gf_circle = gr_d3d_circle;

	gr_screen.gf_line = gr_d3d_line;
	gr_screen.gf_aaline = gr_d3d_aaline;
	gr_screen.gf_pixel = gr_d3d_pixel;
	gr_screen.gf_scaler = gr_d3d_scaler;
	gr_screen.gf_aascaler = gr_d3d_aascaler;
	gr_screen.gf_tmapper = gr_d3d_tmapper;

	gr_screen.gf_gradient = gr_d3d_gradient;

	gr_screen.gf_set_palette = gr_d3d_set_palette;
	gr_screen.gf_print_screen = gr_d3d_print_screen;

	gr_screen.gf_fade_in = gr_d3d_fade_in;
	gr_screen.gf_fade_out = gr_d3d_fade_out;
	gr_screen.gf_flash = gr_d3d_flash;

	gr_screen.gf_zbuffer_get = gr_d3d_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_d3d_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_d3d_zbuffer_clear;

	gr_screen.gf_save_screen = gr_d3d_save_screen;
	gr_screen.gf_restore_screen = gr_d3d_restore_screen;
	gr_screen.gf_free_screen = gr_d3d_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_d3d_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_d3d_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_d3d_dump_frame;

	gr_screen.gf_set_gamma = gr_d3d_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_d3d_lock;
	gr_screen.gf_unlock = gr_d3d_unlock;

	// screen region
	gr_screen.gf_get_region = gr_d3d_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_d3d_fog_set;

	// pixel get
	gr_screen.gf_get_pixel = gr_d3d_get_pixel;

	// poly culling
	gr_screen.gf_set_cull = gr_d3d_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_d3d_cross_fade;

	// filtering
	gr_screen.gf_filter_set = gr_d3d_filter_set;

	// texture cache
	gr_screen.gf_tcache_set = d3d_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_d3d_set_clear_color;

	// now for the bitmap functions
	gr_screen.gf_bm_set_max_bitmap_size     = bm_d3d_set_max_bitmap_size;     
	gr_screen.gf_bm_get_next_handle         = bm_d3d_get_next_handle;         
	gr_screen.gf_bm_close                   = bm_d3d_close;                   
	gr_screen.gf_bm_init                    = bm_d3d_init;                    
	gr_screen.gf_bm_get_frame_usage         = bm_d3d_get_frame_usage;         
	gr_screen.gf_bm_create                  = bm_d3d_create;                  
	gr_screen.gf_bm_load                    = bm_d3d_load;                   
	gr_screen.gf_bm_load_duplicate          = bm_d3d_load_duplicate;          
	gr_screen.gf_bm_load_animation          = bm_d3d_load_animation;          
	gr_screen.gf_bm_get_info                = bm_d3d_get_info;                
	gr_screen.gf_bm_lock                    = bm_d3d_lock;                    
	gr_screen.gf_bm_unlock                  = bm_d3d_unlock;                  
	gr_screen.gf_bm_get_palette             = bm_d3d_get_palette;             
	gr_screen.gf_bm_release                 = bm_d3d_release;                 
	gr_screen.gf_bm_unload                  = bm_d3d_unload;                  
	gr_screen.gf_bm_unload_all              = bm_d3d_unload_all;              
	gr_screen.gf_bm_page_in_texture         = bm_d3d_page_in_texture;         
	gr_screen.gf_bm_page_in_start           = bm_d3d_page_in_start;           
	gr_screen.gf_bm_page_in_stop            = bm_d3d_page_in_stop;            
	gr_screen.gf_bm_get_cache_slot          = bm_d3d_get_cache_slot;          
	gr_screen.gf_bm_24_to_16                = bm_d3d_24_to_16;                
	gr_screen.gf_bm_get_components          = bm_d3d_get_components;          
	gr_screen.gf_bm_get_section_size        = bm_d3d_get_section_size;      
	
	gr_screen.gf_bm_page_in_nondarkening_texture = bm_d3d_page_in_nondarkening_texture; 
	gr_screen.gf_bm_page_in_xparent_texture		 = bm_d3d_page_in_xparent_texture;		 
	gr_screen.gf_bm_page_in_aabitmap			 = bm_d3d_page_in_aabitmap;	 
	
	gr_screen.gf_push_texture_matrix = gr_d3d_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_d3d_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_d3d_translate_texture_matrix;

	if(!Cmdline_nohtl) {
		gr_screen.gf_make_buffer = gr_d3d_make_buffer;
		gr_screen.gf_destroy_buffer = gr_d3d_destroy_buffer;
		gr_screen.gf_render_buffer = gr_d3d_render_buffer;

		gr_screen.gf_start_instance_matrix = gr_d3d_start_instance_matrix;
		gr_screen.gf_end_instance_matrix = gr_d3d_end_instance_matrix;

		gr_screen.gf_make_light = gr_d3d_make_light;
		gr_screen.gf_modify_light = gr_d3d_modify_light;
		gr_screen.gf_destroy_light = gr_d3d_destroy_light;
		gr_screen.gf_set_light = gr_d3d_set_light;
		gr_screen.gf_reset_lighting = gr_d3d_reset_lighting;

		gr_screen.gf_lighting = gr_d3d_lighting;

		gr_screen.start_clip_plane = d3d_start_clip;
		gr_screen.end_clip_plane = d3d_end_clip;
	}

}

int d3d_match_mode(int adapter)
{
	char *ptr = os_config_read_string(NULL, NOX("videocardFs2open"), NULL);	
	uint width, height;
	int cdepth;

	if(ptr == NULL)
	{
		strcpy(Device_init_error, "Cant get 'videocardFs2open' reg entry");
		return -1;
	}

	if(sscanf(ptr, "D3D8-(%dx%d)x%d bit", &width, &height, &cdepth)  != 3) {
		strcpy(Device_init_error, "Cant understand 'videocardFs2open' reg entry");
		return -1;
	}

	int num_modes = lpD3D->GetAdapterModeCount(adapter);

	if(num_modes == 0) {
		strcpy(Device_init_error, "No modes for this adapter");
		return -1;
	}

	for(int i = 0; i < num_modes; i++)
	{
		D3DDISPLAYMODE mode;
		lpD3D->EnumAdapterModes(adapter, i, &mode); 

		// ignore invalid modes
		if(cdepth != d3d_get_mode_bit(mode.Format)) continue; 
		if(width  != mode.Width)  continue; 
		if(height != mode.Height) continue; 

		// This is the mode we want
		return i;
	}

	strcpy(Device_init_error, "No suitable mode found");
	return -1;
}

/**
 * This is the new D3D8 initialise function
 *
 * @return bool
 */

//trying to use a higher bit depth in the back buffer, the deepest one posale -Bobboau
#define N_FORMATS 3
enum _D3DFORMAT format_type[N_FORMATS] = {D3DFMT_D24X8, D3DFMT_D32, D3DFMT_D16};
//enum _D3DFORMAT format_type[N_FORMATS] = {D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D16};

bool gr_d3d_init()
{
	int adapter_choice = D3DADAPTER_DEFAULT;
	D3DDISPLAYMODE mode;

	DBUGFILE_OUTPUT_0("gr_d3d_init start");

	lpD3D = Direct3DCreate8( D3D_SDK_VERSION );

	if( lpD3D == NULL ) {
		MessageBox(NULL, "Please make sure you have DX8.1b installed", "RandomTiger", MB_OK);
		return false;
	}

	ShowCursor(false);
	// End of choose gfx mode here

	// Set up the common device	parameters
	ZeroMemory( &d3dpp, sizeof(d3dpp) );

	d3dpp.BackBufferCount		 = 1;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.EnableAutoDepthStencil = TRUE;
	//this right here used to be just D3DFMT_D16, but it's now part of the format_type array -Bobboau
    d3dpp.AutoDepthStencilFormat = format_type[0];

	if(Cmdline_nohtl) {
		// Only need this for software fog
		d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}
    
	D3D_window = Cmdline_window;

	if (D3D_window) {	
		// If we go windowed, then we need to adjust some other present parameters		
		if (FAILED(lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode )))
		{
			strcpy(Device_init_error, "Could not get adapter display mode");
			return false;
		}

		d3dpp.MultiSampleType  = D3DMULTISAMPLE_NONE;
		d3dpp.BackBufferWidth  = 1024;
		d3dpp.BackBufferHeight = 768;

		d3dpp.FullScreen_RefreshRateInHz      = 0;
		d3dpp.FullScreen_PresentationInterval = 0;

		d3dpp.Windowed		   = TRUE;
		d3dpp.BackBufferFormat = mode.Format;
	} else {

		// Attempt to get options from the registry
		adapter_choice = os_config_read_uint( NULL, "D3D8_Adapter", 0xffff);
		int aatype_choice  = os_config_read_uint( NULL, "D3D8_AAType", 0xffff);
		int mode_choice	   = d3d_match_mode(adapter_choice);
		
		if(mode_choice == -1)
		{
			strcpy(Device_init_error, "Couldnt match mode");
			return false;
		}

		// Should only activate if a value in the registry is not set or its going to run in a window or
		// a optional parameter forces it to run. Otherwise the mode values are taken from reg value
		if( adapter_choice == 0xffff || 
			aatype_choice  == 0xffff)
		{
			strcpy(Device_init_error, "DX8 options not set, please run launcher");
			return false;
		}

		if(FAILED(lpD3D->EnumAdapterModes(adapter_choice, mode_choice, &mode)))
		{
			sprintf(Device_init_error, "Could not use selected mode: %d", mode_choice);
			return false;
		}

		D3D_Antialiasing = (aatype_choice != 0);

		d3dpp.MultiSampleType  = multisample_types[aatype_choice];
		d3dpp.BackBufferWidth  = mode.Width;
		d3dpp.BackBufferHeight = mode.Height;

		// Determine if we are using a custom size
		if(mode.Width != 1024 && mode.Height != 768) {
			D3D_custom_size = GR_1024;

			// Override these values
			gr_screen.max_w = mode.Width;
			gr_screen.max_h = mode.Height;
			gr_screen.clip_right  = gr_screen.max_w - 1;
			gr_screen.clip_bottom = gr_screen.max_h - 1;
			gr_screen.clip_width  = gr_screen.max_w;
			gr_screen.clip_height = gr_screen.max_h;

		}

		d3dpp.FullScreen_RefreshRateInHz      = D3DPRESENT_RATE_DEFAULT;
		d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		d3dpp.Windowed		   = FALSE;
		d3dpp.BackBufferFormat = mode.Format;
	}
		
	// NOTE from UP: Maybe we should also try for pure devices here?
	// Try to create hardware vertex processing device first

	//it trys to use the highest suported back buffer through trial and error, I wraped the existing code in a for loop to cycle through the diferent formats
	//-Bobboau
	for(int t = 0; t > -1 && t < N_FORMATS; t++){
		d3dpp.AutoDepthStencilFormat = format_type[t];
		if( FAILED( lpD3D->CreateDevice(adapter_choice, D3DDEVTYPE_HAL, 
								(HWND) os_get_window(),
                                D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                &d3dpp, &lpD3DDevice) ) ) {

			DBUGFILE_OUTPUT_0("Failed to create hardware vertex processing device, trying software");

			if( FAILED( lpD3D->CreateDevice(adapter_choice, D3DDEVTYPE_HAL, 
									(HWND) os_get_window(),
					                D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						            &d3dpp, &lpD3DDevice) ) ) {

				DBUGFILE_OUTPUT_0("Failed to create software vertex processing device");
				if(t>N_FORMATS){
					strcpy(Device_init_error, "Failed to create device!");
				}
			} else {
				if(2==N_FORMATS){
					DBUGFILE_OUTPUT_0("useing old stiyle backbuffer, you will have clipping");
				}else{
					DBUGFILE_OUTPUT_0("useing new stiyle backbuffer, you should not have any clipping problems, unless you REALY try");
				}
				t=-2;
			}
		} else {
			if(2==N_FORMATS){
				DBUGFILE_OUTPUT_0("useing old stiyle backbuffer, you will have clipping");
			}else {
				DBUGFILE_OUTPUT_0("useing new stiyle backbuffer, you should not have any clipping problems, unless you REALY try");
			}
			t=-2;
			DBUGFILE_OUTPUT_0("Using hardware vertex processing");
		}
	}

	if (t != -1) 
	{
		sprintf(Device_init_error, "Failed to get depth stencil format");
		return false;
	}

	// determine 32 bit status
	gr_screen.bits_per_pixel  = d3d_get_mode_bit(d3dpp.BackBufferFormat);
	gr_screen.bytes_per_pixel = gr_screen.bits_per_pixel / 8;
	D3D_32bit = gr_screen.bits_per_pixel == 32 ? 1 : 0;

	DBUGFILE_OUTPUT_2("D3D_32bit %d, bits_per_pixel %d",D3D_32bit, gr_screen.bits_per_pixel);

	d3d_determine_texture_formats(adapter_choice, &mode);
											   
	lpD3DDevice->GetDeviceCaps(&d3d_caps);

	// Tell Freespace code that we're using Direct3D.
	D3D_enabled = 1;		

	d3dpp.BackBufferWidth = mode.Width;

	d3d_reset_render_states();
	d3d_reset_texture_stage_states();
	D3D_inited = d3d_init_device(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);	 
	
	// did we initialize properly?
	if(!D3D_inited){
		sprintf(Device_init_error, "Failed to initialise device");
		return false;
	}												    

	// RT - Differences between 32 and 16 bit have been considerably reduced, these variables are
	// Only being kept for the sake of glide.
  	d3d_tcache_init();
  	Gr_bitmap_poly = 1;

	// zbiasing?
	if(os_config_read_uint(NULL, "DisableZbias", 0)){
		D3D_zbias = 0;
	}
	
	d3d_start_frame();
	
	// RT This stuff is needed for software fog
	Gr_current_red =   &Gr_red;
	Gr_current_blue =  &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	{
		DDPIXELFORMAT temp_format;
		d3d_fill_pixel_format(&temp_format, d3dpp.BackBufferFormat);

		d3d_setup_format_components(
			&temp_format, 
			Gr_current_red,
			Gr_current_green,
			Gr_current_blue,
			Gr_current_alpha);
	}

	d3d_setup_function_pointers();
	
	// Not sure now relevent this is now
	uint tmp = os_config_read_uint( NULL, "D3DTextureOrigin", 0xFFFF );

	if ( tmp != 0xFFFF )	{
		if ( tmp )	{
			D3d_rendition_uvs = 1;
		} else {
			D3d_rendition_uvs = 0;
		}
	} else {
		if(D3D_32bit){
			d3d_detect_texture_origin_32();
		} else {
			d3d_detect_texture_origin_16();
		}
	}

	DBUGFILE_OUTPUT_1("D3d_rendition_uvs: %d",D3d_rendition_uvs);

	// Not sure now relevent this is now
	tmp = os_config_read_uint( NULL, "D3DLineOffset", 0xFFFF );

	extern float D3D_line_offset;
	if ( tmp != 0xFFFF )	{
		if ( tmp )	{
			D3D_line_offset = 0.5f;
		} else {
			D3D_line_offset = 0.0f;
		}
	} else {
		if(D3D_32bit){
			d3d_detect_line_offset_32();
		} else {
			d3d_detect_line_offset_16();
		}
	}

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	TIMERBAR_SET_DRAW_FUNC(d3d_render_timer_bar);
	DBUGFILE_OUTPUT_0("gr_d3d_init end");
	gr_d3d_activate(1);

	return true;

}

/**
 * Takes a D3D error and turns it into text, however for DX8 they have really reduced the 
 * number of error codes so the result may be quite general  
 *
 * @param HRESULT error
 * @return const char *
 */
const char *d3d_error_string(HRESULT error)
{
	return DXGetErrorString8(error);
}
