/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3DRender.cpp $
 * $Revision: 2.60 $
 * $Date: 2005-02-27 10:38:06 $
 * $Author: wmcoolmon $
 *
 * Code to actually render stuff using Direct3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.59  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 2.58  2005/02/18 09:51:06  wmcoolmon
 * Updates for better nonstandard res support, as well as a fix to the Perseus crash bug I've been experiencing. Bobb, you might want to take a look at my change to grd3d.cpp
 *
 * Revision 2.57  2005/02/10 04:01:42  wmcoolmon
 * Low-level code for better hi-res support; better error reporting for vertex errors on model load.
 *
 * Revision 2.56  2005/01/30 03:24:39  wmcoolmon
 * Don't try and create a vertex buffer with no vertices (Seems to cause CTD) and fix to brackets in nonstandard res
 *
 * Revision 2.55  2005/01/29 08:04:15  wmcoolmon
 * Ahh, the sweet smell of optimized code
 *
 * Revision 2.54  2005/01/14 05:28:57  wmcoolmon
 * gr_curve
 *
 * Revision 2.53  2004/07/29 03:41:46  taylor
 * plug memory leaks
 *
 * Revision 2.52  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.51  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.50  2004/07/05 05:09:19  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.49  2004/05/25 00:37:26  wmcoolmon
 * Updated function calls for VC7 use
 *
 * Revision 2.48  2004/04/11 13:56:33  randomtiger
 * Adding batching functions here and there and into gr_screen for use with OGL when its ready.
 *
 * Revision 2.47  2004/03/20 21:17:12  bobboau
 * fixed -spec comand line option,
 * probly some other stuf
 *
 * Revision 2.46  2004/03/20 14:47:13  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.45  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.44  2004/03/05 09:02:00  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.43  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.42  2004/02/16 11:47:33  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.41  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.40  2003/12/08 22:30:02  randomtiger
 * Put render state and other direct D3D calls repetition check back in, provides speed boost.
 * Fixed bug that caused fullscreen only crash with DXT textures
 * Put dithering back in for tgas and jpgs
 *
 * Revision 2.39  2003/11/29 17:13:53  randomtiger
 * Undid my node fix, it introduced a lot of bugs, update again if you have that version.
 *
 * Revision 2.38  2003/11/29 10:52:09  randomtiger
 * Turned off D3D file mapping, its using too much memory which may be hurting older systems and doesnt seem to be providing much of a speed benifit.
 * Added stats command for ingame stats on memory usage.
 * Trys to play intro.mve and intro.avi, just to be safe since its not set by table.
 * Added fix for fonts wrapping round in non standard hi res modes.
 * Changed D3D mipmapping to a good value to suit htl mode.
 * Added new fog colour method which makes use of the bitmap, making this htl feature backcompatible again.
 *
 * Revision 2.37  2003/11/19 20:37:24  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.36  2003/11/17 04:25:56  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.35  2003/11/11 02:15:44  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.34  2003/11/06 21:10:26  randomtiger
 * Added my batching solution for more efficient d3d_string.
 * Its part of the new grd3dbatch module, most of this isnt in use but it might help out later so I've left it in.
 *
 * Revision 2.33  2003/11/02 05:50:08  bobboau
 * modified trails to render with tristrips now rather than with stinky old trifans,
 * MUCH faster now, at least one order of magnatude.
 *
 * Revision 2.32  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.31  2003/10/29 02:09:18  randomtiger
 * Updated timerbar code to work properly, also added support for it in OGL.
 * In D3D red is general processing (and generic graphics), green is 2D rendering, dark blue is 3D unlit, light blue is HT&L renders and yellow is the presentation of the frame to D3D. OGL is all red for now. Use compile flag TIMERBAR_ON with code lib to activate it.
 * Also updated some more D3D device stuff that might get a bit more speed out of some cards.
 *
 * Revision 2.30  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.29  2003/10/26 00:31:58  randomtiger
 * Fixed hulls not drawing (with Phreaks advise).
 * Put my 32bit PCX loading under PCX_32 compile flag until its working.
 * Fixed a bug with res 640x480 I introduced with my non standard mode code.
 * Changed JPG and TGA loading command line param to "-t32"
 *
 * Revision 2.28  2003/10/24 17:35:05  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.27  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.26  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.25  2003/10/16 17:36:29  randomtiger
 * D3D now has its own gamma system (stored in GammaD3D reg entry) that effects everything.
 * Put in Bobs specular fog fix.
 *
 * Revision 2.24  2003/10/16 00:17:14  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
 * Revision 2.23  2003/10/14 17:39:13  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.22  2003/10/13 05:57:48  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap since we don't give a rats arse
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.21  2003/09/25 21:12:24  Kazan
 * ##Kazan## FS2NetD Completed!  Just needs some thorough bug checking (i don't think there are any serious bugs)
 * Also D3D8 Screenshots work now.
 *
 * Revision 2.20  2003/09/23 02:42:53  Kazan
 * ##KAZAN## - FS2NetD Support! (FS2 Open PXO) -- Game Server Listing, and mission validation completed - stats storing to come - needs fs2open_pxo.cfg file [VP-able]
 *
 * Revision 2.19  2003/09/14 19:01:35  wmcoolmon
 * Changed "cell" to "Cmdline_cell" -C
 *
 * Revision 2.18  2003/09/07 18:14:53  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.17  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.16  2003/08/22 07:35:08  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.15  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.14  2003/08/12 03:18:33  bobboau
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
 * Revision 2.13  2003/08/05 23:45:18  bobboau
 * glow maps, for some reason they wern't in here, they should be now,
 * also there is some debug code for changeing the FOV in game,
 * and I have some changes to the init code to try and get a 32 or 24 bit back buffer
 * if posable, this may cause problems for people
 * the changes were all marked and if needed can be undone
 *
 * Revision 2.12  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.11  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.10  2003/03/02 05:43:48  penguin
 * ANSI C++ - fixed non-compliant casts to unsigned short and unsigned char
 *  - penguin
 *
 * Revision 2.9  2003/02/16 05:14:28  bobboau
 * added glow map nebula bug fix for d3d, someone should add a fix for glide too
 * more importantly I (think I) have fixed all major bugs with fighter beams, and added a bit of new functionality
 *
 * Revision 2.8  2003/01/20 05:40:49  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.7  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.6  2003/01/09 21:20:21  phreak
 * fixed a small error in bob's code
 *
 * Revision 2.5  2003/01/05 23:41:50  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.4  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.3.2.22  2002/11/16 20:09:54  randomtiger
 *
 * Changed my fog hack to be valid code. Put large texture check back in.
 * Added some blending type checks. - RT
 *
 * Revision 2.3.2.21  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.3.2.20  2002/11/10 11:32:29  randomtiger
 *
 * Made D3D8 mipmapping optional on command line flag -d3dmipmip, off by default.
 * When on is now less blury. - RT
 *
 * Revision 2.3.2.19  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.3.2.18  2002/11/05 10:27:39  randomtiger
 *
 * Fixed windowed mode bug I introduced.
 * Added Antialiasing functionality, can only be sure it works on GF4 in 1024 mode. - RT
 *
 * Revision 2.3.2.17  2002/11/02 13:54:26  randomtiger
 *
 * Made a few cunning alterations to get rid of that alpha bug but might have a slight slowdown.
 * Non alpha textures are now alpha textures with (if texture format supported) just one bit for alpha.
 * And non alpha blending is now alpha blending with automatic disregaring of 0 alpha.
 *
 * Revision 2.3.2.16  2002/10/31 15:17:09  randomtiger
 *
 * Added mipmapping, should have done that ages ago! - RT
 *
 * Revision 2.3.2.15  2002/10/26 01:24:22  randomtiger
 * Fixed debug bitmap compiling bug.
 * Fixed tga bug. - RT
 *
 * Revision 2.3.2.14  2002/10/21 16:33:41  randomtiger
 * Added D3D only 32 bit TGA functionality. Will load a texture as big as your graphics card allows. Code not finished yet and will forge the beginnings of the new texture system. - RT
 *
 * Revision 2.3.2.13  2002/10/20 22:21:48  randomtiger
 * Some incomplete code to handle background drawing when message boxes are drawn.
 * It doesnt work but its a good base for someone to start from. - RT
 *
 * Revision 2.3.2.12  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.3.2.11  2002/10/14 21:52:02  randomtiger
 * Finally tracked down and killed off that 8 bit alpha bug.
 * So the font, HUD and 8 bit ani's now work fine. - RT
 *
 * Revision 2.3.2.10  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.3.2.9  2002/10/08 14:33:27  randomtiger
 * OK, I've fixed the z-buffer problem.
 * However I have abandoned using w-buffer for now because of problems.
 * I think I know how to solve it but Im not sure it would make much difference given the way FS2 engine works.
 * I have left code in there of use if anyone wants to turn their hand to it. However for now
 * we just need to crack of the alpha problem then we will have FS2 fully wokring in DX8 on GF4 in 32 bit.
 *
 * Revision 2.3.2.8  2002/10/04 00:48:42  randomtiger
 * Fixed video memory leaks
 * Added code to cope with lost device, not tested
 * Got rid of some DX5 stuff we definately dont need
 * Moved some enum's into internal,h because gr_d3d_set_state should be able to be called from any dx file
 * Cleaned up some stuff - RT
 *
 * Revision 2.3.2.7  2002/10/02 17:52:32  randomtiger
 * Fixed blue lighting bug.
 * Put filtering flag set back in that I accidentally removed
 * Added some new functionality to my debuging system - RT
 *
 * Revision 2.3.2.6  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.3.2.5  2002/09/28 22:13:43  randomtiger
 * Sorted out some bits and pieces. The background nebula blends now which is nice. – RT
 *
 * Revision 2.3.2.4  2002/09/28 12:20:32  randomtiger
 * Just a tiny code change that lets stuff work in 16 bit.
 * For some reason 16 bit code was taking a different code path for displaying textures.
 * So until I unstand why, Im cutting off that codepath because it isnt easy to convert into DX8.
 *
 * Revision 2.3.2.3  2002/09/28 00:18:08  randomtiger
 * Did some work on trying to get textures to load from pcx, define TX_ATTEMPT for access to it.
 * Converted some DX7 blending calls to DX8 which a fair difference to ingame visuals.
 *
 * - RT
 *
 * Revision 2.3.2.2  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.3  2002/08/07 00:45:25  DTP
 * Implented -GF4FIX commandline switch & #include "cmdline/cmdline.h"
 *
 * Revision 2.2  2002/08/03 19:42:17  randomtiger
 * Fixed Geforce 4 bug that caused font and hall video distortion.
 * Very small change in 'gr_d3d_aabitmap_ex_internal'
 *
 * Tested and works on the following systems
 *
 * OUTSIDER Voodoo 3 win98
 * OUTSIDER Geforce 2 win2000
 * Me Geforce 4 PNY 4600 XP
 * JBX-Phoenix Geforce 4 PNY XP
 * Mehrunes GeForce 3 XP
 * WMCoolmon nVidia TNT2 M64 win2000
 * Orange GeForce 4 4200 XP
 * ShadowWolf_IH Monster2 win98
 * ShadowWolf_IH voodoo 2 win98
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 27    9/13/99 11:30a Dave
 * Added checkboxes and functionality for disabling PXO banners as well as
 * disabling d3d zbuffer biasing.
 * 
 * 26    9/08/99 12:03a Dave
 * Make squad logos render properly in D3D all the time. Added intel anim
 * directory.
 * 
 * 25    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 24    7/30/99 4:04p Anoop
 * Fixed D3D shader.
 * 
 * 23    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 22    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 21    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 20    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 19    7/19/99 3:29p Dave
 * Fixed gamma bitmap in the options screen.
 * 
 * 18    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 17    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 16    7/12/99 11:42a Jefff
 * Made rectangle drawing smarter in D3D. Made plines draw properly on Ati
 * Rage Pro.
 * 
 * 15    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 14    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 13    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 12    1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 11    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 10    12/08/98 7:03p Dave
 * Much improved D3D fogging. Also put in vertex fogging for the cheesiest
 * of 3d cards.
 * 
 * 9     12/08/98 2:47p Johnson
 * Made D3D fog use eye-relative fog instead of z depth fog.
 * 
 * 8     12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 7     12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 6     12/07/98 9:00a Dave
 * Fixed d3d rendered. Still don't have fog working.
 * 
 * 5     12/06/98 6:53p Dave
 * 
 * 4     12/01/98 5:54p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 54    5/25/98 10:32a John
 * Took out redundant code for font bitmap offsets that converted to a
 * float, then later on converted back to an integer.  Quite unnecessary.
 * 
 * 53    5/24/98 6:45p John
 * let direct3d do all clipping.
 * 
 * 52    5/24/98 3:42p John
 * Let Direct3D do clipping on any linear textures, like lasers.
 * 
 * 51    5/23/98 7:18p John
 * optimized the uv bashing a bit.
 * 
 * 50    5/22/98 1:11p John
 * Added code to actually detect which offset a line needs
 * 
 * 49    5/22/98 12:54p John
 * added .5 to each pixel of a line.  This seemed to make single pixel
 * lines draw on all cards.
 * 
 * 48    5/22/98 9:00a John
 * Fixed problem of no fading out of additive textures due to Permedia2
 * fix.  Did this by dimming out the vertex RGB values.
 * 
 * 47    5/21/98 9:56p John
 * Made Direct3D work with classic alpha-blending only devices, like the
 * Virge.  Added a texture type XPARENT that fills the alpha in in the
 * bitmap for Virge.   Added support for Permedia by making making
 * additive alphablending be one/one instead of alpha/one, which didn't
 * work, and there is no way to tell this from caps.
 * 
 * 46    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 45    5/20/98 3:10p John
 * Made lines work even if no alphagouraud capabilities on the card.
 * 
 * 44    5/19/98 4:50p Lawrance
 * JAS: Fixed some bugs on Alan's nVidia Riva128 PCI where some
 * unitiallized fields, namely vertex->shw were causing glitches.
 * 
 * 43    5/19/98 1:46p John
 * Fixed Rendition/Riva128 uv problems.
 * 
 * 42    5/19/98 12:34p John
 * added code to fix uv's on rendition.  added code to fix zbuffering
 * problem on rendition.
 * 
 * 41    5/18/98 8:26p John
 * Made scanline be line.   Made lines work if no line alpha blending
 * supported.   Made no alpha mode use alpha off.  
 * 
 * 40    5/17/98 4:13p John
 * Made zbuffer clear only clear current clip region
 * 
 * 39    5/17/98 3:23p John
 * Took out capibility check for additive blending.  Made gr_bitmap_ex
 * clip properly in glide and direct3d.
 * 
 * 38    5/15/98 8:48a John
 * Fixed bug where one-pixel line was getting left on right and bottom.
 * 
 * 37    5/12/98 8:43p John
 * fixed particle zbuffering.
 * 
 * 36    5/12/98 10:34a John
 * Added d3d_shade functionality.  Added d3d_flush function, since the
 * shader seems to get reorganzed behind the overlay text stuff!
 * 
 * 35    5/12/98 10:06a John
 * Made all tmaps "clamp-clip".  This fixed bug with offscreen hud
 * indicators not rendering.
 * 
 * 34    5/12/98 8:18a John
 * Put in code to use a different texture format for alpha textures and
 * normal textures.   Turned off filtering for aabitmaps.  Took out
 * destblend=invsrccolor alpha mode that doesn't work on riva128. 
 * 
 * 33    5/11/98 10:58a John
 * Fixed pilot name cursor bug.  Started adding in code for alphachannel
 * textures.
 * 
 * 32    5/09/98 12:37p John
 * More texture caching
 * 
 * 31    5/09/98 12:16p John
 * Even better texture caching.
 * 
 * 30    5/08/98 10:12a John
 * took out an mprintf
 * 
 * 29    5/07/98 11:31a John
 * Removed DEMO defines
 * 
 * 28    5/07/98 10:28a John
 * Made texture format use 4444.   Made fonts use alpha to render.
 * 
 * 27    5/07/98 10:09a John
 * Fixed some bugs with short lines in D3D.
 * 
 * 26    5/07/98 9:54a John
 * Added in palette flash functionallity.
 * 
 * 25    5/07/98 9:40a John
 * Fixed some bitmap transparency issues with Direct3D.
 * 
 * 24    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 23    5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 22    5/06/98 8:07p John
 * made d3d clear work correctly.
 * 
 * 21    5/06/98 8:00p John
 * Got stars working under D3D.
 * 
 * 20    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 19    5/05/98 10:37p John
 * Added code to optionally use execute buffers.
 * 
 * 18    5/04/98 3:36p John
 * Got zbuffering working with Direct3D.
 * 
 * 17    5/03/98 10:52a John
 * Made D3D sort of work on 3dfx.
 * 
 * 16    5/03/98 10:43a John
 * Working on Direct3D.
 * 
 * 15    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 14    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 13    4/09/98 11:05a John
 * Removed all traces of Direct3D out of the demo version of Freespace and
 * the launcher.
 * 
 * 12    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 11    3/11/98 1:55p John
 * Fixed warnings
 * 
 * 10    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 9     3/08/98 12:33p John
 * Added more lines, tris, and colored flat polys (lasers!) correctly.
 * 
 * 8     3/08/98 10:25a John
 * Added in lines
 * 
 * 7     3/07/98 8:29p John
 * Put in some Direct3D features.  Transparency on bitmaps.  Made fonts &
 * aabitmaps render nice.
 * 
 * 6     3/06/98 5:39p John
 * Started adding in aabitmaps
 * 
 * 5     3/02/98 5:42p John
 * Removed WinAVI stuff from Freespace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 4     2/26/98 3:24p John
 * fixed optimized warning
 * 
 * 3     2/17/98 7:28p John
 * Got fonts and texturing working in Direct3D
 * 
 * 2     2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 1     2/03/98 9:24p John
 *
 * $NoKeywords: $
 */

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/grbatch.h"
#include "graphics/grd3dbatch.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/line.h"
#include "nebula/neb.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"	
#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"

#include <D3dx8tex.h>



// Viewport used to change render between full screen and sub sections like the pilot animations
D3DVIEWPORT8 viewport;

int D3d_last_state = -1;


/**
 * This function should be used to control blending texture, the z buffer and how the textures
 * are filtered
 *
 * @param gr_texture_source ts
 * @param gr_alpha_blend ab
 * @param gr_zbuffer_type zt
 *
 * @return void
 */
void gr_d3d_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt )
{
	int current_state = 0;

	current_state = current_state | (ts<<0);
	current_state = current_state | (ab<<5);
	current_state = current_state | (zt<<10);

	if ( current_state == D3d_last_state ) {
	//	return;
	}

	D3d_last_state = current_state;

	switch( ts )	{
	case TEXTURE_SOURCE_NONE:
		// Let the texture cache system know whe set the handle to NULL
  	   	d3d_SetTexture(0, NULL);
   	 	gr_tcache_set(-1, -1, NULL, NULL );

		break;
	case TEXTURE_SOURCE_DECAL:
		d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		if( Cmdline_d3dmipmap ) {
			d3d_SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
		  	const float f_bias = -2.0f;
		 	d3d_SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&f_bias)));
		}

		// RT This code seems to render inactive text
		d3d_SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);  
		break;

	case TEXTURE_SOURCE_NO_FILTERING:

		if(gr_screen.custom_size < 0) {
			d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT );
			d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		} else {
			// If we are using a non standard mode we will need this because textures are being stretched
			d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
			d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );
		}

		d3d_SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);

		// RT This code seems to render inactive text
		d3d_SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);  
		break;

	default:
		Int3();
	}

	switch( ab )	{
	case ALPHA_BLEND_NONE:							// 1*SrcPixel + 0*DestPixel	(not true anymore)
		d3d_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		d3d_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		d3d_SetRenderState( D3DRS_ALPHAREF, 0x00000000); 
		d3d_SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER); 
		d3d_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3d_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

   		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:				// Alpha*SrcPixel + 1*DestPixel
	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
		if( GlobalD3DVars::d3d_caps.SrcBlendCaps & D3DPBLENDCAPS_ONE &&
			GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE) {
			
			d3d_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			// Must use ONE:ONE as the Permedia2 can't do SRCALPHA:ONE.
			// But I lower RGB values so we don't loose anything.
			d3d_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE );
			d3d_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
			break;
		}
		// Fall through to normal alpha blending mode...

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:			// Alpha*SrcPixel + (1-Alpha)*DestPixel
			
		d3d_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		if( GlobalD3DVars::d3d_caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA &&
			GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) {
			d3d_SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			d3d_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		} else {
			d3d_SetRenderState( D3DRS_SRCBLEND, D3DBLEND_BOTHSRCALPHA );
		}
		break;

	default:
		Int3();
	}

	// RT - we should get the wbuffer back in at some point, I will attend to this at some point
	// Changing this value to -1 is not enough
	static int use_wbuffer = false;

	// Determine if device can use wbuffer 
	if(!Cmdline_nohtl && use_wbuffer == -1)
	{
		use_wbuffer = (GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_WBUFFER) ? 1 : 0;
	}

	switch( zt )	{

	case ZBUFFER_TYPE_NONE:
		d3d_SetRenderState( D3DRS_ALPHATESTENABLE, false );
		d3d_SetRenderState(D3DRS_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;
	case ZBUFFER_TYPE_READ:
		d3d_SetRenderState( D3DRS_ALPHATESTENABLE, false );
		d3d_SetRenderState(D3DRS_ZENABLE, use_wbuffer ? D3DZB_USEW : TRUE);
		d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;
	case ZBUFFER_TYPE_WRITE:
		d3d_SetRenderState(D3DRS_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;
	case ZBUFFER_TYPE_FULL:
		d3d_SetRenderState(D3DRS_ZENABLE, use_wbuffer ? D3DZB_USEW : TRUE);
		d3d_SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;

	// This is the state that the zbuffer will be put into if the device is reset
	case ZBUFFER_TYPE_DEFAULT:
		d3d_SetRenderState(D3DRS_ZENABLE, TRUE);
		d3d_SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;

	default:
		DBUGFILE_OUTPUT_0("Invalid Z buffer option");
		Int3();
	}

}

/**
 * @param int bias
 *
 * @return void
 */
void gr_d3d_zbias(int bias)
{
	if(GlobalD3DVars::D3D_zbias)
		d3d_SetRenderState(D3DRS_ZBIAS, bias);
}

/**
 * If mode is FALSE, turn zbuffer off the entire frame,
 * no matter what people pass to gr_zbuffer_set.
 *
 * @param int mode
 *
 * @return void
 */
void gr_d3d_zbuffer_clear(int mode)
{
	if ( mode )	{
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		// Make sure zbuffering is on
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );

		if(FAILED(GlobalD3DVars::lpD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0x00000000, 1.0, 0))) {
			mprintf(( "Failed to clear zbuffer!\n" ));
			return;
		}

	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

/**
 * internal d3d rect function
 *
 * @param int x 
 * @param int y 
 * @param int w 
 * @param int h 
 * @param int r 
 * @param int g 
 * @param int b 
 * @param int a
 *
 * @return void
 */
void gr_d3d_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int saved_zbuf;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	saved_zbuf = gr_zbuffer_get();
	
	// start the frame, no zbuffering, no culling
	g3_start_frame(1);	
	gr_zbuffer_set(GR_ZBUFF_NONE);		
	gr_set_cull(0);		

	// stuff coords		
	v[0].sx = i2fl(x);
	v[0].sy = i2fl(y);
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)r;
	v[0].g = (ubyte)g;
	v[0].b = (ubyte)b;
	v[0].a = (ubyte)a;

	v[1].sx = i2fl(x + w);
	v[1].sy = i2fl(y);	
	v[1].sw = 0.0f;
	v[1].u = 0.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)r;
	v[1].g = (ubyte)g;
	v[1].b = (ubyte)b;
	v[1].a = (ubyte)a;

	v[2].sx = i2fl(x + w);
	v[2].sy = i2fl(y + h);
	v[2].sw = 0.0f;
	v[2].u = 0.0f;
	v[2].v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)r;
	v[2].g = (ubyte)g;
	v[2].b = (ubyte)b;
	v[2].a = (ubyte)a;

	v[3].sx = i2fl(x);
	v[3].sy = i2fl(y + h);
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;				
	v[3].r = (ubyte)r;
	v[3].g = (ubyte)g;
	v[3].b = (ubyte)b;
	v[3].a = (ubyte)a;

	// draw the polys
	g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA | TMAP_HTL_2D, 0.1f);		

	g3_end_frame();

	// restore zbuffer and culling
	gr_zbuffer_set(saved_zbuf);
	gr_set_cull(1);
}

/**
 *
 *
 * @return int
 */
int gr_d3d_zbuffer_get()
{
	if ( !gr_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

/**
 * @param int mode
 *
 * @return int
 */
int gr_d3d_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if ( gr_zbuffering_mode == GR_ZBUFF_NONE )	{
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}

float D3D_line_offset = 0.0f;

/**
 * @param D3DTLVERTEX *a 
 * @param D3DTLVERTEX *b 
 * @param int x1 
 * @param int y1 
 * @param int x2 
 * @param int y2
 *
 * @return void
 */
void d3d_make_rect( D3DVERTEX2D *a, D3DVERTEX2D *b, int x1, int y1, int x2, int y2 )
{
	// Alan's nvidia riva128 PCI screws up targetting brackets if rhw are uninitialized.
	a->rhw = 1.0f;
	b->rhw = 1.0f;

	a->sz = 0.99f;
	b->sz = 0.99f;

	a->sx = i2fl(x1 + gr_screen.offset_x)+D3D_line_offset;
	a->sy = i2fl(y1 + gr_screen.offset_y)+D3D_line_offset;

	b->sx = i2fl(x2 + gr_screen.offset_x)+D3D_line_offset;
	b->sy = i2fl(y2 + gr_screen.offset_y)+D3D_line_offset;

	if ( x1 == x2 )	{
		// Verticle line
		if ( a->sy < b->sy )	{
			b->sy += 0.5f;
		} else {
			a->sy += 0.5f;
		}
	} else if ( y1 == y2 )	{
		// Horizontal line
		if ( a->sx < b->sx )	{
			b->sx += 0.5f;
		} else {
			a->sx += 0.5f;
		}
	}
}

/**
 * basically just fills in the alpha component of the specular color. Hardware does the rest
 * when rendering the poly
 *
 * @param float z
 * @param D3DCOLOR *spec
 *
 * @return void
 */
void gr_d3d_stuff_fog_value(float z, D3DCOLOR *spec)
{
	float f_float;	
	*spec = 0;

	// linear fog formula
	f_float = (gr_screen.fog_far - z) / (gr_screen.fog_far - gr_screen.fog_near);
	if(f_float < 0.0f){
		f_float = 0.0f;
	} else if(f_float > 1.0f){
		f_float = 1.0f;
	}

	*spec = D3DCOLOR_RGBA(0,0,0, (int)(f_float * 255.0f));
}

float z_mult = 30000.0f;
DCF(zmult, "")
{
  //	dc_get_arg(ARG_FLOAT);
  //	z_mult = Dc_arg_float;
}

/**
 * Caps a floating point value between minx and maxx
 *
 * @param float x 
 * @param  float minx 
 * @param  float maxx
 *
 * @return float
 */
float flCAP( float x, float minx, float maxx)
{
	if ( x < minx )	{
		return minx;
	} else if ( x > maxx )	{
		return maxx;
	}
	return x;
}

static float Interp_fog_level;
int w_factor = 256;

#define MAX_INTERNAL_POLY_VERTS 1024

/**
 * This will be used to render the 3D parts the of FS2 engine
 *
 * @param int nverts
 * @param  vertex **verts
 * @param  uint flags
 * @param  int is_scaler
 *
 * @return void
 */
void gr_d3d_tmapper_internal_batch_3d_unlit( int nverts, vertex *verts, uint flags)	
{
	// Some checks to make sure this function isnt used when it shouldnt be
	Assert(flags & TMAP_HTL_3D_UNLIT);

	float u_scale = 1.0f, v_scale = 1.0f;
	int bw = 1, bh = 1;		

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;

	if ( gr_zbuffering )	{
		zbuffer_type = ZBUFFER_TYPE_FULL;
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	// want to be in here!
	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		if (GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE )	{
			tmap_type   = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = 255;
	}

	Assert(!(flags & TMAP_FLAG_BITMAP_SECTION));

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale))	{
//			mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		texture_source = TEXTURE_SOURCE_DECAL;
	}
	
 //	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	
	BatchInfo batch_info;
	batch_info.alpha_blend_type = alpha_blend;
	batch_info.bitmap_type      = tmap_type;
	batch_info.filter_type      = texture_source;
	batch_info.is_set           = true;
	batch_info.state_set_func   = NULL;
	batch_info.texture_id       = gr_screen.current_bitmap;
	batch_info.zbuffer_type     = zbuffer_type;

	D3DLVERTEX *d3d_verts	= (D3DLVERTEX *) d3d_batch_lock_vbuffer(GlobalD3DVars::unlit_3D_batch, nverts, batch_info);
	D3DLVERTEX *src_v		= d3d_verts;

	float uoffset = 0.0f;
	float voffset = 0.0f;

	float minu=0.0f, minv=0.0f, maxu=1.0f, maxv=1.0f;

	if ( flags & TMAP_FLAG_TEXTURED )	{								
		if ( GlobalD3DVars::D3D_rendition_uvs )	{				
			bm_get_info(gr_screen.current_bitmap, &bw, &bh);			
				
			uoffset = 2.0f/i2fl(bw);
			voffset = 2.0f/i2fl(bh);

			minu = uoffset;
			minv = voffset;

			maxu = 1.0f - uoffset;
			maxv = 1.0f - voffset;
		}				
	}	

	for (int i=0; i<nverts; i++ )	{
		vertex * va = &verts[i];		
			  
		int a      = ( flags & TMAP_FLAG_ALPHA )   ? verts[i].a : alpha;
	
		if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = verts[i].r;
			g = verts[i].g;
			b = verts[i].b;
		} else {
			// use constant RGB values...
		}
		
		src_v->color = D3DCOLOR_ARGB(a, r, g, b);
		src_v->specular = D3DCOLOR_ARGB(a, r, g, b);

		src_v->sx = va->x; 
		src_v->sy = va->y; 
		src_v->sz = va->z;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			// argh. rendition
			if ( GlobalD3DVars::D3D_rendition_uvs ){				
				// tiled texture (ships, etc), bitmap sections
				if(flags & TMAP_FLAG_TILED){					
					src_v->tu = va->u*u_scale;
					src_v->tv = va->v*v_scale;
				}
				// sectioned
				else if(flags & TMAP_FLAG_BITMAP_SECTION){
					int sw, sh;
					bm_get_info(gr_screen.current_bitmap, &sw, &sh, NULL, NULL, NULL);


				 //	DBUGFILE_OUTPUT_4("%f %f %d %d",va->u,va->v,sw,sh);
					src_v->tu = (va->u + (0.5f / i2fl(sw))) * u_scale;
					src_v->tv = (va->v + (0.5f / i2fl(sh))) * v_scale;
				}										   

				// all else.
				else {				
					src_v->tu = flCAP(va->u, minu, maxu);
					src_v->tv = flCAP(va->v, minv, maxv);
				}				
			}
			// yay. non-rendition
			else {
				src_v->tu = va->u*u_scale;
				src_v->tv = va->v*v_scale;
			}							
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}
		src_v++;
	}

	// None of these objects are set to be fogged, but perhaps they should be
	if(flags & TMAP_FLAG_PIXEL_FOG) {
		Assert(0); // Shouldnt be here
	//  	gr_fog_set(GR_FOGMODE_FOG, 255, 0, 0, 1.0f, 750.0f);
	} else {
		gr_fog_set(GR_FOGMODE_NONE,0,0,0);
	}

	d3d_batch_unlock_vbuffer(GlobalD3DVars::unlit_3D_batch);
	d3d_batch_draw_vbuffer(GlobalD3DVars::unlit_3D_batch);

// 	d3d_DrawPrimitive(D3DVT_LVERTEX, D3DPT_TRIANGLELIST, (LPVOID)d3d_verts, nverts);
}

/**
 * This will be used to render the 3D parts the of FS2 engine
 *
 * @param int nverts
 * @param  vertex **verts
 * @param  uint flags
 * @param  int is_scaler
 *
 * @return void
 */
//this is all RT's stuff
bool warn___ = true;
void gr_d3d_tmapper_internal_3d_unlit( int nverts, vertex **verts, uint flags, int is_scaler )	
{
	// Some checks to make sure this function isnt used when it shouldnt be
	Assert(flags & TMAP_HTL_3D_UNLIT);

	float u_scale = 1.0f, v_scale = 1.0f;
	int bw = 1, bh = 1;		

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;

	if ( gr_zbuffering )	{
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	)	{
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	// want to be in here!
	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		if (GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE )	{
			tmap_type   = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = 255;
	}

	Assert(!(flags & TMAP_FLAG_BITMAP_SECTION));

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale))	{
//			mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		texture_source = TEXTURE_SOURCE_DECAL;
	}

//	if((u_scale != 1.0f || v_scale != 1.0f) && warn___)Warning(LOCATION, "UV scale diferent");
	
	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	
	Assert(nverts < MAX_INTERNAL_POLY_VERTS);

	static D3DLVERTEX d3d_verts[MAX_INTERNAL_POLY_VERTS];	//static so it doesn't have to reallocate it every time
	D3DLVERTEX *src_v = d3d_verts;

	float uoffset = 0.0f;
	float voffset = 0.0f;

	float minu=0.0f, minv=0.0f, maxu=1.0f, maxv=1.0f;

	if ( flags & TMAP_FLAG_TEXTURED )	{								
		if ( GlobalD3DVars::D3D_rendition_uvs )	{				
			bm_get_info(gr_screen.current_bitmap, &bw, &bh);			
				
			uoffset = 2.0f/i2fl(bw);
			voffset = 2.0f/i2fl(bh);

			minu = uoffset;
			minv = voffset;

			maxu = 1.0f - uoffset;
			maxv = 1.0f - voffset;
		}				
	}	

	Assert(nverts < MAX_INTERNAL_POLY_VERTS);
	if(nverts > MAX_INTERNAL_POLY_VERTS-1)Error( LOCATION, "too many verts in gr_d3d_tmapper_internal_3d_unlit\n" );
	if(nverts < 3)Error( LOCATION, "too few verts in gr_d3d_tmapper_internal_3d_unlit\n" );

	for (int i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
			  
		int a      = ( flags & TMAP_FLAG_ALPHA )   ? verts[i]->a : alpha;
	
		if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = verts[i]->r;
			g = verts[i]->g;
			b = verts[i]->b;
		} else {
			// use constant RGB values...
		}
		
		src_v->color = D3DCOLOR_ARGB(a, r, g, b);
		src_v->specular = D3DCOLOR_ARGB(a, r, g, b);

		src_v->sx = va->x; 
		src_v->sy = va->y; 
		src_v->sz = va->z;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			// argh. rendition
			if ( GlobalD3DVars::D3D_rendition_uvs ){				
				// tiled texture (ships, etc), bitmap sections
				if(flags & TMAP_FLAG_TILED){					
					src_v->tu = va->u*u_scale;
					src_v->tv = va->v*v_scale;
				}
				// sectioned
				else if(flags & TMAP_FLAG_BITMAP_SECTION){
					int sw, sh;
					bm_get_info(gr_screen.current_bitmap, &sw, &sh, NULL, NULL, NULL);


				 //	DBUGFILE_OUTPUT_4("%f %f %d %d",va->u,va->v,sw,sh);
					src_v->tu = (va->u + (0.5f / i2fl(sw))) * u_scale;
					src_v->tv = (va->v + (0.5f / i2fl(sh))) * v_scale;
				}										   

				// all else.
				else {				
					src_v->tu = flCAP(va->u, minu, maxu);
					src_v->tv = flCAP(va->v, minv, maxv);
				}				
			}
			// yay. non-rendition
			else {
				src_v->tu = va->u*u_scale;
				src_v->tv = va->v*v_scale;
			}							
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}
		src_v++;
	}

	// None of these objects are set to be fogged, but perhaps they should be
	if(flags & TMAP_FLAG_PIXEL_FOG) {
		Assert(0); // Shouldnt be here
	//  	gr_fog_set(GR_FOGMODE_FOG, 255, 0, 0, 1.0f, 750.0f);
	} else {
		gr_fog_set(GR_FOGMODE_NONE,0,0,0);
	}

	TIMERBAR_PUSH(2);
 	d3d_DrawPrimitive(D3DVT_LVERTEX, (flags & TMAP_FLAG_TRISTRIP)?D3DPT_TRIANGLESTRIP :D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);
	TIMERBAR_POP();
}

void gr_d3d_tmapper_internal_2d( int nverts, vertex **verts, uint flags, int is_scaler )	
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;
	int bw = 1, bh = 1;		

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;


	if ( gr_zbuffering )	{
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	)	{
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	// want to be in here!
	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		if (GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			tmap_type = TCACHE_TYPE_NORMAL;
			////////////////////////////////
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_NONE;
		alpha = 255;
	}

	if(flags & TMAP_FLAG_BITMAP_SECTION){
		tmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))	{
			// SHUT UP! -- Kazan -- This is massively slowing debug builds down
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		if(flags & TMAP_FLAG_BITMAP_SECTION) {
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}
	
	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	
	Assert(nverts < 32);

	D3DVERTEX2D d3d_verts[32];
	D3DVERTEX2D *src_v = d3d_verts;

	int x1, y1, x2, y2;
	x1 = gr_screen.clip_left*16;
	x2 = gr_screen.clip_right*16+15;
	y1 = gr_screen.clip_top*16;
	y2 = gr_screen.clip_bottom*16+15;

	float uoffset = 0.0f;
	float voffset = 0.0f;

	float minu=0.0f, minv=0.0f, maxu=1.0f, maxv=1.0f;

	if ( flags & TMAP_FLAG_TEXTURED )	{								
		if ( GlobalD3DVars::D3D_rendition_uvs )	{				
			bm_get_info(gr_screen.current_bitmap, &bw, &bh);			
				
			uoffset = 2.0f/i2fl(bw);
			voffset = 2.0f/i2fl(bh);

			minu = uoffset;
			minv = voffset;

			maxu = 1.0f - uoffset;
			maxv = 1.0f - voffset;
		}				
	}	

	for (i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
			  
		src_v->sz = 0.99f;

		// For texture correction 	
	  	src_v->rhw = ( flags & TMAP_FLAG_CORRECT ) ? va->sw : 1.0f;
		int a      = ( flags & TMAP_FLAG_ALPHA )   ? verts[i]->a : alpha;

		if ( flags & TMAP_FLAG_NEBULA )	{
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )	{
			r = verts[i]->b;
			g = verts[i]->b;
			b = verts[i]->b;
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = verts[i]->r;
			g = verts[i]->g;
			b = verts[i]->b;
		} else {
			// use constant RGB values...
		}

		src_v->color = D3DCOLOR_ARGB(a, r, g, b);

		int x, y;
		x = fl2i(va->sx*16.0f);
		y = fl2i(va->sy*16.0f);

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		src_v->sx = i2fl(x) / 16.0f;
		src_v->sy = i2fl(y) / 16.0f;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			// argh. rendition
			if ( GlobalD3DVars::D3D_rendition_uvs ){				
				// tiled texture (ships, etc), bitmap sections
				if(flags & TMAP_FLAG_TILED){					
				  	src_v->tu = va->u*u_scale;
				  	src_v->tv = va->v*v_scale;
				}
				// sectioned
				else if(flags & TMAP_FLAG_BITMAP_SECTION){
					int sw, sh;
					bm_get_section_size(gr_screen.current_bitmap, 
						gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, &sw, &sh);

				 //	DBUGFILE_OUTPUT_4("%f %f %d %d",va->u,va->v,sw,sh);
				 	src_v->tu = (va->u + (0.5f / i2fl(sw))) * u_scale;
				 	src_v->tv = (va->v + (0.5f / i2fl(sh))) * v_scale;
				}
				// all else.
				else 
				{				
			   		src_v->tu = flCAP(va->u, minu, maxu);
			   		src_v->tv = flCAP(va->v, minv, maxv);
				}				
			}
			// yay. non-rendition
			else {
			  	src_v->tu = va->u*u_scale;
			  	src_v->tv = va->v*v_scale;
			} 							
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}

		src_v++;
	}

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	set_stage_for_defuse();
	TIMERBAR_PUSH(2);
	d3d_DrawPrimitive(D3DVT_VERTEX2D, (flags & TMAP_FLAG_TRISTRIP)?D3DPT_TRIANGLESTRIP :D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);
	TIMERBAR_POP();
}

/**
 * This is used to render the 2D parts the of FS2 engine
 *
 * @param int nverts
 * @param  vertex **verts
 * @param  uint flags
 * @param  int is_scaler
 *
 * @return void
 */

extern int spec;
bool env_enabled = false;
extern int cell_shaded_lightmap;
bool cell_enabled = false;
extern bool rendering_shockwave;
void gr_d3d_tmapper_internal( int nverts, vertex **verts, uint flags, int is_scaler )	
{
//	d3d_set_initial_render_state();

	if(flags & TMAP_HTL_2D)
	{
		gr_d3d_tmapper_internal_2d(nverts, verts, flags, is_scaler);
		return;
	}

	if(!Cmdline_nohtl && (flags & TMAP_HTL_3D_UNLIT)) {
		gr_d3d_tmapper_internal_3d_unlit(nverts, verts, flags, is_scaler);
		return;
	}

	int i;
	float u_scale = 1.0f, v_scale = 1.0f;
	int bw = 1, bh = 1;		

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3();
	}

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;


	if ( gr_zbuffering )	{
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	)	{
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	// want to be in here!
	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		if (GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			tmap_type = TCACHE_TYPE_NORMAL;
			////////////////////////////////
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_NONE;
		alpha = 255;
	}

	if(flags & TMAP_FLAG_BITMAP_SECTION){
		tmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0))	{
			// SHUT UP! -- Kazan -- This is massively slowing debug builds down
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		if(flags & TMAP_FLAG_BITMAP_SECTION) {
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}
	
	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	
	Assert(nverts < MAX_INTERNAL_POLY_VERTS);

	static D3DTLVERTEX d3d_verts[MAX_INTERNAL_POLY_VERTS];		//static so it doesn't have to reallocate it every time
	D3DTLVERTEX *src_v = d3d_verts;

	int x1, y1, x2, y2;
	x1 = gr_screen.clip_left*16;
	x2 = gr_screen.clip_right*16+15;
	y1 = gr_screen.clip_top*16;
	y2 = gr_screen.clip_bottom*16+15;

	float uoffset = 0.0f;
	float voffset = 0.0f;

	float minu=0.0f, minv=0.0f, maxu=1.0f, maxv=1.0f;

	if ( flags & TMAP_FLAG_TEXTURED )	{								
		if ( GlobalD3DVars::D3D_rendition_uvs )	{				
			bm_get_info(gr_screen.current_bitmap, &bw, &bh);			
				
			uoffset = 2.0f/i2fl(bw);
			voffset = 2.0f/i2fl(bh);

			minu = uoffset;
			minv = voffset;

			maxu = 1.0f - uoffset;
			maxv = 1.0f - voffset;
		}				
	}	

	if(nverts > MAX_INTERNAL_POLY_VERTS-1)Error( LOCATION, "too many verts in gr_d3d_tmapper_internal\n" );
	if(nverts < 3)Error( LOCATION, "too few verts in gr_d3d_tmapper_internal\n" );

	for (i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
			  
		// store in case we're doing vertex fog.		
		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )	{
			src_v->sz = va->z / z_mult;	// For zbuffering and fogging
			if ( src_v->sz > 0.98f )	{
				src_v->sz = 0.98f;
			}		
		} else {
			src_v->sz = 0.99f;
		}			

		// For texture correction 	
	  	src_v->rhw = ( flags & TMAP_FLAG_CORRECT ) ? va->sw : 1.0f;
		int a      = ( flags & TMAP_FLAG_ALPHA )   ? verts[i]->a : alpha;

		if ( flags & TMAP_FLAG_NEBULA )	{
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )	{
			r = verts[i]->b;
			g = verts[i]->b;
			b = verts[i]->b;
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = verts[i]->r;
			g = verts[i]->g;
			b = verts[i]->b;
		} else {
			// use constant RGB values...
		}

		src_v->color = D3DCOLOR_ARGB(a, r, g, b);

		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE)){// && Cmdline_nohtl) {
			gr_d3d_stuff_fog_value(va->z, &src_v->specular);
		} else {
			src_v->specular = 0;
		}


		int x, y;
		x = fl2i(va->sx*16.0f);
		y = fl2i(va->sy*16.0f);

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		src_v->sx = i2fl(x) / 16.0f;
		src_v->sy = i2fl(y) / 16.0f;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			// argh. rendition
			if ( GlobalD3DVars::D3D_rendition_uvs ){				
				// tiled texture (ships, etc), bitmap sections
				if(flags & TMAP_FLAG_TILED){					
				  	src_v->tu = va->u*u_scale;
				  	src_v->tv = va->v*v_scale;
				}
				// sectioned
				else if(flags & TMAP_FLAG_BITMAP_SECTION){
					int sw, sh;
					bm_get_section_size(gr_screen.current_bitmap, 
						gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, &sw, &sh);

				 //	DBUGFILE_OUTPUT_4("%f %f %d %d",va->u,va->v,sw,sh);
				 	src_v->tu = (va->u + (0.5f / i2fl(sw))) * u_scale;
				 	src_v->tv = (va->v + (0.5f / i2fl(sh))) * v_scale;
				}
				// all else.
				else 
				{				
			   		src_v->tu = flCAP(va->u, minu, maxu);
			   		src_v->tv = flCAP(va->v, minv, maxv);
				}				
			}
			// yay. non-rendition
			else {
			  	src_v->tu = va->u*u_scale;
			  	src_v->tv = va->v*v_scale;
			} 							
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}

		src_v++;
	}

	int ra = 0, ga = 0, ba = 0;	

	float f_float;	

	// if we're rendering against a fullneb background
	if(flags & TMAP_FLAG_PIXEL_FOG && Cmdline_nohtl){	
		int r, g, b;
//		int ra, ga, ba;		
		ra = ga = ba = 0;		

		// get the average pixel color behind the vertices
		for(i=0; i<nverts; i++){			
			neb2_get_pixel((int)d3d_verts[i].sx, (int)d3d_verts[i].sy, &r, &g, &b);
			ra += r;
			ga += g;
			ba += b;
		}				
		ra /= nverts;
		ga /= nverts;
		ba /= nverts;

		// set fog
		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}					

	//BEGIN FINAL SETTINGS
	if(Cmdline_cell && cell_enabled){
	
		if(GLOWMAP < 0 || Cmdline_noglow){
			d3d_SetTexture(2, NULL);
			d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	
			set_stage_for_cell_shaded();
		}else{
			gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0, -1, -1);
			d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 2);

			if(GlobalD3DVars::d3d_caps.MaxSimultaneousTextures > 2)
				set_stage_for_cell_glowmapped_shaded();
			else
				set_stage_for_cell_shaded();

		}

		gr_screen.gf_set_bitmap(cell_shaded_lightmap, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0, -1, -1);
		d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);

		for (i=0; i<nverts; i++ )	{
			//d3d_verts[i].color = D3DCOLOR_ARGB(255,255,255,255);
			d3d_verts[i].env_u = ((verts[i]->r + verts[i]->g + verts[i]->b)/3)/255.0f;
			d3d_verts[i].env_v = 0.0f;
		}

		d3d_DrawPrimitive(D3DVT_TLVERTEX, D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);

		if(GlobalD3DVars::d3d_caps.MaxSimultaneousTextures < 3){
			gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0, -1, -1);
			if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
				return;
			}
			set_stage_for_additive_glowmapped();
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
			d3d_DrawPrimitive(D3DVT_TLVERTEX, D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);
			gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
		}

		return;
	}

	//a bit of optomiseation, if there is no specular highlights don't bother waisting the recorses on trying to render them
	bool has_spec = false;
	for (i=0; i<nverts; i++ )	{
		if((verts[i]->spec_r > 0) || (verts[i]->spec_g > 0) || (verts[i]->spec_b > 0)){
			has_spec = true;
			break;
		}
	}

	if(GLOWMAP < 0 || Cmdline_noglow){
	//	d3d_SetTexture(1, NULL);
	//	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}else{
		gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0, -1, -1);
		d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);
	}

	if(has_spec){
		if(SPECMAP < 0){
			//nonmapped specular
			if(GLOWMAP > -1){
				//glow mapped
				set_stage_for_glow_mapped_defuse_and_non_mapped_spec();
			}else{
				//non glowmapped
				set_stage_for_defuse_and_non_mapped_spec();
			}
		}else{
			//mapped specular
			if(GLOWMAP > -1){
				//glowmapped
				set_stage_for_glow_mapped_defuse();
			}else{
				//non glowmapped
				set_stage_for_defuse();
			}
		}
	}else{//has_spec
		//defuse only
		if(GLOWMAP > -1){
			//glowmapped
			set_stage_for_glow_mapped_defuse();
		}else{
			//non glowmapped
			set_stage_for_defuse();
		}
	}

	if(rendering_shockwave && (flags & TMAP_FLAG_PIXEL_FOG)){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);//don't fog shockwaves
		for (i=0; i<nverts; i++ )	{
			f_float = (gr_screen.fog_far - verts[i]->z) / (gr_screen.fog_far - gr_screen.fog_near);
			if(f_float < 0.0f)f_float = 0.0f;

			d3d_verts[i].color = D3DCOLOR_RGBA((ubyte)((int)verts[i]->r * f_float), (ubyte)((int)verts[i]->g * f_float), (ubyte)((int)verts[i]->b * f_float), verts[i]->a);
		}
	}

	// Draws just about everything except stars and lines

	TIMERBAR_PUSH(3);
	d3d_DrawPrimitive(D3DVT_TLVERTEX, (flags & TMAP_FLAG_TRISTRIP)?D3DPT_TRIANGLESTRIP :D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);
	TIMERBAR_POP();

	//spec mapping
	if(has_spec && (SPECMAP > 0)){
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0, -1, -1);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
				return;
			}
		for (i=0; i<nverts; i++ )	{
			if(flags & TMAP_FLAG_PIXEL_FOG){
				// linear fog formula
				f_float = (gr_screen.fog_far - verts[i]->z) / (gr_screen.fog_far - gr_screen.fog_near);
				if(f_float < 0.0f){
					f_float = 0.0f;
				} else if(f_float > 1.0f){
					f_float = 1.0f;
				}

				d3d_verts[i].specular = D3DCOLOR_RGBA((ubyte)((int)verts[i]->spec_r * f_float), (ubyte)((int)verts[i]->spec_g * f_float), (ubyte)((int)verts[i]->spec_b * f_float), *(((ubyte*)&d3d_verts[i].specular)+3));
			}else
				d3d_verts[i].specular = D3DCOLOR_RGBA(verts[i]->spec_r, verts[i]->spec_g, verts[i]->spec_b, *(((ubyte*)&d3d_verts[i].specular)+3));
		}

		if(set_stage_for_spec_mapped()){
			//spec mapping is always done on a second pass
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
			if(flags & TMAP_FLAG_PIXEL_FOG) gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
			d3d_DrawPrimitive(D3DVT_TLVERTEX, (flags & TMAP_FLAG_TRISTRIP)?D3DPT_TRIANGLESTRIP :D3DPT_TRIANGLEFAN, (LPVOID)d3d_verts, nverts);
			if(flags & TMAP_FLAG_PIXEL_FOG) gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
			gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
		}
	}

}

/*
			if(env_enabled){
				gr_screen.gf_set_bitmap(ENVMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
				if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1))	{
						mprintf(( "Not rendering environment texture because it didn't fit in VRAM!\n" ));
						return;
				}
				for (i=0; i<nverts; i++ )	{
					d3d_verts[i].env_u = verts[i]->env_u;
					d3d_verts[i].env_v = verts[i]->env_v;
				}
			}
*/
/**
 * This is just a wrapper for gr_d3d_tmapper_internal function, this calls it with a final parameter
 * zero while another D3D functions calls it internally with one.
 * 
 * @param int nverts 
 * @param vertex **verts 
 * @param uint flags
 *
 * @return void
 */
void gr_d3d_tmapper( int nverts, vertex **verts, uint flags )	
{
	gr_d3d_tmapper_internal( nverts, verts, flags, 0 );
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

/**
 * @param vertex *va
 * @param vertex *vb
 *
 * @return void
 */
void gr_d3d_scaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}
	// Clip the bottom, moving v1 up as necessary
	
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================

	vertex v[4];
	vertex *vl[4];

	vl[0] = &v[0];	
	v->sx = clipped_x0;
	v->sy = clipped_y0;
	v->sw = va->sw;
	v->z = va->z;
	v->u = clipped_u0;
	v->v = clipped_v0;

	vl[1] = &v[1];	
	v[1].sx = clipped_x1;
	v[1].sy = clipped_y0;
	v[1].sw = va->sw;
	v[1].z = va->z;
	v[1].u = clipped_u1;
	v[1].v = clipped_v0;

	vl[2] = &v[2];	
	v[2].sx = clipped_x1;
	v[2].sy = clipped_y1;
	v[2].sw = va->sw;
	v[2].z = va->z;
	v[2].u = clipped_u1;
	v[2].v = clipped_v1;

	vl[3] = &v[3];	
	v[3].sx = clipped_x0;
	v[3].sy = clipped_y1;
	v[3].sw = va->sw;
	v[3].z = va->z;
	v[3].u = clipped_u0;
	v[3].v = clipped_v1;

	d3d_set_initial_render_state();

	gr_d3d_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}

/**
 * Empty function
 *
 * @return void
 */
void gr_d3d_aascaler(vertex *va, vertex *vb )
{
}

/**
 * Wrapper for gr_line
 *
 * @param int x
 * @param int y
 *
 * @return void
 */
void gr_d3d_pixel(int x, int y)
{
	gr_line(x,y,x,y);
}

/**
 * Clear the screen with the current colour
 *
 * @return void
 */
void gr_d3d_clear()
{
	// Turn off zbuffering so this doesn't clear the zbuffer
	gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

	D3DCOLOR color = D3DCOLOR_XRGB(
						gr_screen.current_clear_color.red, 
						gr_screen.current_clear_color.green, 
						gr_screen.current_clear_color.blue);

	GlobalD3DVars::lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 0,0);
}

/**
 * sets the clipping region & offset
 *
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 *
 * @return void
 */
void gr_d3d_set_clip(int x,int y,int w,int h, bool resize)
{
	if(resize)
	{
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;

	// check for sanity of parameters
	if ( gr_screen.clip_left+x < 0 ) {
		gr_screen.clip_left = -x;
	} else if ( gr_screen.clip_left+x > gr_screen.max_w-1 )	{
		gr_screen.clip_left = gr_screen.max_w-1-x;
	}
	if ( gr_screen.clip_right+x < 0 ) {
		gr_screen.clip_right = -x;
	} else if ( gr_screen.clip_right+x >= gr_screen.max_w-1 )	{
		gr_screen.clip_right = gr_screen.max_w-1-x;
	}

	if ( gr_screen.clip_top+y < 0 ) {
		gr_screen.clip_top = -y;
	} else if ( gr_screen.clip_top+y > gr_screen.max_h-1 )	{
		gr_screen.clip_top = gr_screen.max_h-1-y;
	}

	if ( gr_screen.clip_bottom+y < 0 ) {
		gr_screen.clip_bottom = -y;
	} else if ( gr_screen.clip_bottom+y > gr_screen.max_h-1 )	{
		gr_screen.clip_bottom = gr_screen.max_h-1-y;
	}

	gr_screen.clip_width = gr_screen.clip_right - gr_screen.clip_left + 1;
	gr_screen.clip_height = gr_screen.clip_bottom - gr_screen.clip_top + 1;

	// Setup the viewport for a reasonable viewing area
	viewport.X = gr_screen.clip_left + x;
	viewport.Y = gr_screen.clip_top  + y;
	viewport.Width  = gr_screen.clip_width;
	viewport.Height = gr_screen.clip_height;
	viewport.MinZ = 0.0F;
	viewport.MaxZ = 1.0F; // choose something appropriate here!

	// Typically this is used for in game ani / video playing 
	if(FAILED(GlobalD3DVars::lpD3DDevice->SetViewport(&viewport)))
	{
  		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}
}

/**
 *
 *
 * @return void
 */
void gr_d3d_reset_clip()
{
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top  = 0;
	gr_screen.clip_right  = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width  = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;

	// Setup the viewport for a reasonable viewing area
	viewport.X = gr_screen.clip_left;
	viewport.Y = gr_screen.clip_top;
	viewport.Width  = gr_screen.clip_width;
	viewport.Height = gr_screen.clip_height;
	viewport.MinZ = 0.0F;
	viewport.MaxZ = 1.0F; // choose something appropriate here!

	if(FAILED(GlobalD3DVars::lpD3DDevice->SetViewport(&viewport)))
	{
  		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}
}

/**
 * @param color *c
 * @param int r
 * @param int g
 * @param int b
 *
 * @return void
 */
void gr_d3d_init_color(color *c, int r, int g, int b)
{
	c->screen_sig = gr_screen.signature;
	c->red   = unsigned char(r);
	c->green = unsigned char(g);
	c->blue  = unsigned char(b);
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

/**
 * @param color *clr
 * @param int r
 * @param int g
 * @param int b
 * @param int alpha
 * @param int type
 *
 * @return void
 */
void gr_d3d_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_d3d_init_color( clr, r, g, b );

	clr->alpha = (unsigned char) alpha;
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

/**
 * Wrapper for gr_d3d_init_color
 *
 * @param int r
 * @param int g
 * @param int b
 *
 * @return void
 */
void gr_d3d_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_d3d_init_color( &gr_screen.current_color, r, g, b );
}

/**
 * @param int *r
 * @param int *g
 * @param int *b
 *
 * @return void
 */
void gr_d3d_get_color( int *r, int *g, int *b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

/**
 * @param color *dst
 *
 * @return void
 */
void gr_d3d_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			gr_d3d_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_d3d_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

/**
 * @param int bitmap_num
 * @param int alphablend_mode
 * @param int bitblt_mode
 * @param float alpha
 * @param int sx
 * @param int sy
 *
 * @return void
 */
void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

/**
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 * @param int sx
 * @param int sy
 *
 * @return void
 */
void gr_d3d_aabitmap_ex_internal(int x,int y,int w,int h,int sx,int sy,bool resize)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	float u_scale, v_scale;

	gr_d3d_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		return;
	}

	D3DVERTEX2D *src_v;
	D3DVERTEX2D d3d_verts[4];

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	float fbw = i2fl(bw);
	float fbh = i2fl(bh);

	// Rendition 
	if(GlobalD3DVars::D3D_Antialiasing) {
		u0 = u_scale*(i2fl(sx)-0.5f) / fbw;
		v0 = v_scale*(i2fl(sy)+0.05f) / fbh;
		u1 = u_scale*(i2fl(sx+w)-0.5f) / fbw;
		v1 = v_scale*(i2fl(sy+h)-0.5f) / fbh;
	} else if (GlobalD3DVars::D3D_rendition_uvs )	{
		u0 = u_scale*(i2fl(sx)+0.5f) / fbw;
		v0 = v_scale*(i2fl(sy)+0.5f) / fbh;

		u1 = u_scale*(i2fl(sx+w)+0.5f) / fbw;
		v1 = v_scale*(i2fl(sy+h)+0.5f) / fbh;
	} else {
		u0 = u_scale*i2fl(sx)/ fbw;
		v0 = v_scale*i2fl(sy)/ fbh;
		u1 = u_scale*i2fl(sx+w)/ fbw;
		v1 = v_scale*i2fl(sy+h)/ fbh;
	} 

	if(gr_screen.custom_size == -1)
	{
		x1 = i2fl(x+gr_screen.offset_x);
		y1 = i2fl(y+gr_screen.offset_y);
		x2 = i2fl(x+w+gr_screen.offset_x);
		y2 = i2fl(y+h+gr_screen.offset_y);

	} else {

		int nx = x+gr_screen.offset_x;
		int ny = y+gr_screen.offset_y;
		int nw = x+w+gr_screen.offset_x;
		int nh = y+h+gr_screen.offset_y;

		if(resize)
		{
			gr_resize_screen_pos(&nx, &ny);
			gr_resize_screen_pos(&nw, &nh);
		}

		x1 = i2fl(nx);
		y1 = i2fl(ny);
		x2 = i2fl(nw);
		y2 = i2fl(nh);
	}

	src_v = d3d_verts;

	uint color;

	if ( gr_screen.current_color.is_alphacolor )	{
		if( GlobalD3DVars::d3d_caps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR) {
			color = D3DCOLOR_ARGB(
				gr_screen.current_color.alpha,
				gr_screen.current_color.red, 
				gr_screen.current_color.green, 
				gr_screen.current_color.blue);
		} else {
			int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
			int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
			int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
			color = D3DCOLOR_ARGB(255, r,g,b);
		}
	} else {
		color = D3DCOLOR_XRGB(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->sx = x2;
	src_v->sy = y1;
	src_v->tu = u1;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->sx = x1;
	src_v->sy = y2;
	src_v->tu = u0;
	src_v->tv = v1;

	d3d_set_initial_render_state();

	TIMERBAR_PUSH(4);
  	d3d_DrawPrimitive(D3DVT_VERTEX2D, D3DPT_TRIANGLEFAN,(LPVOID)d3d_verts,4);
	TIMERBAR_POP();
}			 
/**
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 * @param int sx
 * @param int sy
 *
 * @return void
 */
void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy,bool resize)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	d3d_set_initial_render_state();

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	gr_d3d_aabitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy,resize);
}

/**
 * @param int x
 * @param int y
 *
 * @return void
 */
void gr_d3d_aabitmap(int x, int y, bool resize)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	d3d_set_initial_render_state();

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy,resize);
}

/**
 * @param int sx
 * @param int sy
 * @param char *s
 *
 * @return void
 */
void gr_d3d_string( int sx, int sy, char *s, bool resize)
{

//	mprintf(("<%s>\n", s));

	if ( !Current_font )	{
		return;
	}

	if ( !gr_screen.current_color.is_alphacolor ) return;

	gr_set_bitmap(Current_font->bitmap_id);

	// Get this now rather than inside the loop
	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh );
	gr_d3d_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	float u_scale, v_scale;
	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		return;
	}

	uint color;

	if ( gr_screen.current_color.is_alphacolor )	{
		if( GlobalD3DVars::d3d_caps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR) {
			color = D3DCOLOR_ARGB(
				gr_screen.current_color.alpha,
				gr_screen.current_color.red, 
				gr_screen.current_color.green, 
				gr_screen.current_color.blue);
		} else {
			int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
			int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
			int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
			color = D3DCOLOR_ARGB(255, r,g,b);
		}
	} else {
		color = D3DCOLOR_XRGB(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}


	d3d_set_initial_render_state();
  	d3d_batch_string(sx, sy, s, bw, bh, u_scale, v_scale, color, resize);
}

/**
 * Wrapper for gr_d3d_rect_internal
 *
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 *
 * @return void
 */
void gr_d3d_rect(int x,int y,int w,int h,bool resize)
{
	gr_d3d_rect_internal(x, y, w, h, gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);	
}

/**
 * @param int r
 * @param int g
 * @param int b
 *
 * @return void
 */
void gr_d3d_flash(int r, int g, int b)
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);

	if ( r || g || b )	{
		uint color;
		if (GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
			color = D3DCOLOR_ARGB(255, r, g, b);
		} else {
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
			int a = (r+g+b)/3;
			color = D3DCOLOR_ARGB(a,r,g,b);
		}
	
		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
	
		D3DVERTEX2D *src_v;
		D3DVERTEX2D d3d_verts[4];

		src_v = d3d_verts;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->sx = x1;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->sx = x2;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->sx = x2;
		src_v->sy = y2;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->sx = x1;
		src_v->sy = y2;

		d3d_set_initial_render_state();

		TIMERBAR_PUSH(5);
		d3d_DrawPrimitive(D3DVT_VERTEX2D, D3DPT_TRIANGLEFAN,(LPVOID)d3d_verts,4);
		TIMERBAR_POP();
	}
}

/**
 * @param shader * shade
 * @param float r
 * @param float g
 * @param float b
 * @param float c
 *
 * @return void
 */
void gr_d3d_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

/**
 * @param shader *shade
 *
 * @return void
 */
void gr_d3d_set_shader( shader *shade )
{	
	if ( shade )	{
		if (shade->screen_sig != gr_screen.signature)	{
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}

/**
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 *
 * @return void
 */
void gr_d3d_shade(int x,int y,int w,int h)
{	
	int r,g,b,a;
/*
	float shade1 = 1.0f;
	float shade2 = 6.0f;

	r = fl2i(gr_screen.current_shader.r*255.0f*shade1);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g*255.0f*shade1);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b*255.0f*shade1);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c*255.0f*shade2);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;*/
	r = fl2i(gr_screen.current_shader.r);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;

	gr_d3d_rect_internal(x, y, w, h, r, g, b, a);	
}

/**
 * @param int xc
 * @param int yc
 * @param int d
 *
 * @return void
 */
void gr_d3d_circle( int xc, int yc, int d, bool resize)
{
	int p,x, y, r;

	if(resize)
		gr_resize_screen_pos(&xc, &yc);

	r = d/2;
	p=3-d;
	x=0;
	y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		gr_d3d_line( xc-y, yc-x, xc+y, yc-x );
		gr_d3d_line( xc-y, yc+x, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
			gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
		gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
	}
}

//xc - x coordinate
//yc - y coordinate
//r - radius of curve
//direction:
//	/0 1\
//	\2 3/
void gr_d3d_curve( int xc, int yc, int r, int direction)
{
	int a,b,p;
	gr_resize_screen_pos(&xc, &yc);

	p=3-(2*r);
	a=0;
	b=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;

	switch(direction)
	{
		case 0:
			yc += r;
			xc += r;
			while(a<b)
			{
				// Draw the first octant
				gr_d3d_line(xc - b + 1, yc-a, xc - b, yc-a);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_d3d_line(xc-a+1,yc-b,xc-a,yc-b);
					p=p+((a-b)<<2)+10;
					b--;
				}
				a++;
			}
			break;
		case 1:
			yc += r;
			while(a<b)
			{
				// Draw the first octant
				gr_d3d_line(xc + b - 1, yc-a, xc + b, yc-a);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_d3d_line(xc+a-1,yc-b,xc+a,yc-b);
					p=p+((a-b)<<2)+10;
					b--;
				}
				a++;
			}
			break;
		case 2:
			xc += r;
			while(a<b)
			{
				// Draw the first octant
				gr_d3d_line(xc - b + 1, yc+a, xc - b, yc+a);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_d3d_line(xc-a+1,yc+b,xc-a,yc+b);
					p=p+((a-b)<<2)+10;
					b--;
				}
				a++;
			}
			break;
		case 3:
			while(a<b)
			{
				// Draw the first octant
				gr_d3d_line(xc + b - 1, yc+a, xc + b, yc+a);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_d3d_line(xc+a-1,yc+b,xc+a,yc+b);
					p=p+((a-b)<<2)+10;
					b--;
				}
				a++;
			}
			break;
	}
}

/**
 *
 * @param int x1
 * @param int y1
 * @param int x2
 * @param int y2
 *
 * @return void
 */
void gr_d3d_line(int x1,int y1,int x2,int y2, bool resize)
{
	if(resize)
	{
		gr_resize_screen_pos(&x1, &y1);
		gr_resize_screen_pos(&x2, &y2);
	}

	int clipped = 0, swapped=0;
	DWORD color;

	// Set up Render State - flat shading - alpha blending
	if ((GlobalD3DVars::d3d_caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && 
		(GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	{

		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
		color = D3DCOLOR_ARGB(gr_screen.current_color.alpha, gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		float alpha_val = gr_screen.current_color.alpha/255.0f;
		
		color = D3DCOLOR_ARGB(255,
								fl2i(gr_screen.current_color.red*alpha_val),
								fl2i(gr_screen.current_color.green*alpha_val),
								fl2i(gr_screen.current_color.blue*alpha_val));
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	D3DVERTEX2D d3d_verts[2];
	D3DVERTEX2D *a = d3d_verts;
	D3DVERTEX2D *b = d3d_verts+1;

	d3d_make_rect(a,b,x1,y1,x2,y2);

	a->color = color;
	b->color = color;

	d3d_set_initial_render_state();

	TIMERBAR_PUSH(6);
	d3d_DrawPrimitive(D3DVT_VERTEX2D, D3DPT_LINELIST,(LPVOID)d3d_verts,2);
	TIMERBAR_POP();
}

/**
 * Wrapper function for gr_d3d_aaline
 *
 * @param vertex *v1
 * @param vertex *v2
 *
 * @return void
 */
void gr_d3d_aaline(vertex *v1, vertex *v2)
{
	gr_d3d_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy) );
}

/**
 * @param int x1
 * @param int y1
 * @param int x2
 * @param int y2
 *
 * @return void
 */
void gr_d3d_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	//gr_resize_screen_pos(&x1, &y1);
	//gr_resize_screen_pos(&x2, &y2);

	if ( !gr_screen.current_color.is_alphacolor )	{
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	uint color1, color2;

	// Set up Render State - flat shading - alpha blending
	if (	
		(GlobalD3DVars::d3d_caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && 
		(GlobalD3DVars::d3d_caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	
	{
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

		if (GlobalD3DVars::d3d_caps.ShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND )	
		{
			color1 = D3DCOLOR_ARGB(gr_screen.current_color.alpha,gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
			color2 = D3DCOLOR_ARGB(0,gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
		} 
		else if (GlobalD3DVars::d3d_caps.ShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	
		{
			color1 = D3DCOLOR_ARGB(gr_screen.current_color.alpha,gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue);
			color2 = D3DCOLOR_ARGB(gr_screen.current_color.alpha,0,0,0);
		} 
		else 
		{
			color1 = D3DCOLOR_ARGB(gr_screen.current_color.alpha,gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue);
			color2 = D3DCOLOR_ARGB(gr_screen.current_color.alpha,gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue);
		}
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
		int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
		int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;

		color1 = D3DCOLOR_ARGB(255,r,g,b);
		color2 = (GlobalD3DVars::d3d_caps.ShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB ) ?
			D3DCOLOR_ARGB(255,0,0,0) :  color1;
	}

	D3DVERTEX2D d3d_verts[2];
	D3DVERTEX2D *a = d3d_verts;
	D3DVERTEX2D *b = d3d_verts+1;

	d3d_make_rect( a, b, x1, y1, x2, y2 );

	if ( swapped )	{
		b->color = color1;
		a->color = color2;
	} else {
		a->color = color1;
		b->color = color2;
	}

	d3d_set_initial_render_state();

	TIMERBAR_PUSH(6);
	d3d_DrawPrimitive(D3DVT_VERTEX2D, D3DPT_LINELIST,(LPVOID)d3d_verts,2);
	TIMERBAR_POP();
}

/**
 * Empty function
 *
 * @param ubyte *new_palette 
 * @param int restrict_alphacolor
 *
 * @return void
 */
void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor)
{
}

/**
 * copy from one pixel buffer to another
 (
 * @param char *to	 - pointer to source buffer
 * @param char *from - pointer to dest. buffet
 * @param int pixels - number of pixels to copy
 * @param fromsize	 - source pixel size
 * @param int tosize - pixel size
 *
 * @return int 
 */
static int tga_copy_data(char *to, char *from, int pixels, int fromsize, int tosize)
{
	if ( (fromsize == 2) && (tosize == 3) )	{
		ushort *src = (ushort *)from;
		ubyte *dst  = (ubyte *)to;

		int i;
		for (i=0; i<pixels; i++ )	{
			ushort pixel = *src++;

			*dst++ = ubyte(((pixel & Gr_blue.mask)>>Gr_blue.shift)*Gr_blue.scale);
			*dst++ = ubyte(((pixel & Gr_green.mask)>>Gr_green.shift)*Gr_green.scale);
			*dst++ = ubyte(((pixel & Gr_red.mask)>>Gr_red.shift)*Gr_red.scale);
		}
		return tosize*pixels;
	} else if( (fromsize == 4) && (tosize == 3) ){
		uint *src = (uint *)from;
		ubyte *dst  = (ubyte *)to;

		int i;
		for (i=0; i<pixels; i++ )	{
			uint pixel = *src++;

			*dst++ = ubyte(((pixel & Gr_blue.mask)>>Gr_blue.shift)*Gr_blue.scale);
			*dst++ = ubyte(((pixel & Gr_green.mask)>>Gr_green.shift)*Gr_green.scale);
			*dst++ = ubyte(((pixel & Gr_red.mask)>>Gr_red.shift)*Gr_red.scale);
		}
		return tosize*pixels;
	}	else {
		Int3();
		return tosize*pixels;
	}
}

/**
 * Test if two pixels are identical
 * 
 * @param char *pix1
 * @param char *pix2
 * @param int pixbytes
 * @return int - 0: No match, 1: Match 
 */
static int tga_pixels_equal(char *pix1, char *pix2, int pixbytes)
{
	do	{
		if ( *pix1++ != *pix2++ ) {
			return 0;
		}
	} while ( --pixbytes > 0 );

	return 1;
}

/**
 * Do the Run Length Compression to compress a TGA
 *
 *
 * @param char *out - Buffer to write it out to
 * @param char *in  - Buffer to compress       
 * @param int bytecount - Number of bytes input         
 * @param int pixsize   - Number of bytes in input pixel
 * @return int 
 */
int tga_compress(char *out, char *in, int bytecount, int pixsize )
{	
	#define outsize 3

	int pixcount;		// number of pixels in the current packet
	char *inputpixel=NULL;	// current input pixel position
	char *matchpixel=NULL;	// pixel value to match for a run
	char *flagbyte=NULL;		// location of last flag byte to set
	int rlcount;		// current count in r.l. string 
	int rlthresh;		// minimum valid run length
	char *copyloc;		// location to begin copying at

	// set the threshold -- the minimum valid run length

	#if outsize == 1
		rlthresh = 2;					// for 8bpp, require a 2 pixel span before rle'ing
	#else
		rlthresh = 1;			
	#endif

	// set the first pixel up
	flagbyte = out;	// place to put next flag if run
	inputpixel = in;
	pixcount = 1;
	rlcount = 0;
	copyloc = (char *)0;

	// loop till data processing complete
	do	{

		// if we have accumulated a 128-byte packet, process it
		if ( pixcount == 129 )	{
			*flagbyte = 127;

			// set the run flag if this is a run

			if ( rlcount >= rlthresh )	{
					*flagbyte |= 0x80;
					pixcount = 2;
			}

			// copy the data into place
			++flagbyte;
			flagbyte += tga_copy_data(flagbyte, copyloc, pixcount-1, pixsize, outsize);
			pixcount = 1;

			// set up for next packet
			continue;
		}

		// if zeroth byte, handle as special case
		if ( pixcount == 1 )	{
			rlcount = 0;
			copyloc = inputpixel;		/* point to 1st guy in packet */
			matchpixel = inputpixel;	/* set pointer to pix to match */
			pixcount = 2;
			inputpixel += pixsize;
			continue;
		}

		// assembling a packet -- look at next pixel

		// current pixel == match pixel?
		if ( tga_pixels_equal(inputpixel, matchpixel, outsize) )	{

			//	establishing a run of enough length to
			//	save space by doing it
			//		-- write the non-run length packet
			//		-- start run-length packet

			if ( ++rlcount == rlthresh )	{
				
				//	close a non-run packet
				
				if ( pixcount > (rlcount+1) )	{
					// write out length and do not set run flag

					*flagbyte++ = (char)(pixcount - 2 - rlthresh);

					flagbyte += tga_copy_data(flagbyte, copyloc, (pixcount-1-rlcount), pixsize, outsize);

					copyloc = inputpixel;
					pixcount = rlcount + 1;
				}
			}
		} else {

			// no match -- either break a run or continue without one
			//	if a run exists break it:
			//		write the bytes in the string (outsize+1)
			//		start the next string

			if ( rlcount >= rlthresh )	{

				*flagbyte++ = (char)(0x80 | rlcount);
				flagbyte += tga_copy_data(flagbyte, copyloc, 1, pixsize, outsize);
				pixcount = 1;
				continue;
			} else {

				//	not a match and currently not a run
				//		- save the current pixel
				//		- reset the run-length flag
				rlcount = 0;
				matchpixel = inputpixel;
			}
		}
		pixcount++;
		inputpixel += pixsize;
	} while ( inputpixel < (in + bytecount));

	// quit this buffer without loosing any data

	if ( --pixcount >= 1 )	{
		*flagbyte = (char)(pixcount - 1);
		if ( rlcount >= rlthresh )	{
			*flagbyte |= 0x80;
			pixcount = 1;
		}

		// copy the data into place
		++flagbyte;
		flagbyte += tga_copy_data(flagbyte, copyloc, pixcount, pixsize, outsize);
	}
	return(flagbyte-out);
}

/**
 * Used to dump a screenshot at any time using PrtScn key
 * Used to save TGA's, now does BMP using D3DX 
 *
 * @param char *filename - Filename of BMP to save
 * @return void
 */

// From Michael Fötsch.
#ifdef __BORLANDC__
#define BITMAP_FILE_SIGNATURE 'BM'
#else
#define BITMAP_FILE_SIGNATURE 'MB'
#endif


void gr_d3d_print_screen(char *filename)
{
	IDirect3DSurface8 *pDestSurface = NULL;

	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, D3DFMT_A8R8G8B8, &pDestSurface)))
	{
		mprintf(("Failed to create image surface"));
		return;
	}

	
	if(FAILED(GlobalD3DVars::lpD3DDevice->GetFrontBuffer(pDestSurface)))
	{
		pDestSurface->Release();
		mprintf(("Failed to get front buffer"));
		return;
	}

	char pic_name[MAX_PATH];
	strcpy(pic_name, filename);
	strcat(pic_name, ".bmp");
	
	/* --- We need the Direct X 8.1 headers for this function !

	if(FAILED(D3DXSaveSurfaceToFile(pic_name, D3DXIFF_BMP, pDestSurface, NULL, NULL)))
	{
		mprintf(("Failed to save file %s", pic_name));
	}*/


	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// The Following code was borrowed from Michael Fötsch.
	// With slight modification of course
	// http://www.geocities.com/foetsch/d3d8screenshot/Screenshot.cpp.html
	// we can use this until we get the Dx 8.1 headers
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(bmih);
    bmih.biWidth = gr_screen.max_w;
    bmih.biHeight = gr_screen.max_h;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = gr_screen.max_w * gr_screen.max_h * 3;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    // reserve memory for the DIB's bitmap bits
    // (The extra byte is needed because the bitmap is 24-bit but we're
    // going to write 32 bits (a DWORD) at a time. When we write the
    // last pixel, we'll exceed the array's limit if we don't reserve an
    // extra byte.)
    unsigned char *Bits = new unsigned char[bmih.biSizeImage + 1];
    if (!Bits)
    {
        return;
    }

    // lock the surface for reading
    D3DLOCKED_RECT LockedRect;
    if (FAILED(pDestSurface->LockRect(&LockedRect, NULL, D3DLOCK_READONLY)))
    {
		if (Bits)
			delete[] Bits;

        return;
    }

    // flip the bitmap vertically (because that's how DIBs are stored)
    // and convert it from 32-bits to 24-bits (some bitmap viewers can't
    // handle 32-bit bitmaps, although it's a valid format)

    LPDWORD lpSrc;
    LPBYTE lpDest = Bits;

    // read pixels beginning with the bottom scan line
    for (int y = gr_screen.max_h - 1; y >= 0; y--)
    {
        // calculate address of the current source scan line
        lpSrc = reinterpret_cast<LPDWORD>(LockedRect.pBits) + y * gr_screen.max_w;
        for (int x = 0; x < gr_screen.max_w; x++)
        {
            // store the source pixel in the bitmap bits array
            *reinterpret_cast<LPDWORD>(lpDest) = *lpSrc;
            lpSrc++;        // increment source pointer by 1 DWORD
            lpDest += 3;    // increment destination pointer by 3 bytes
        }
    }

    // we can unlock and release the surface
    pDestSurface->UnlockRect();


    // prepare the bitmap file header
    BITMAPFILEHEADER bmfh;
    bmfh.bfType = BITMAP_FILE_SIGNATURE;
    bmfh.bfSize = sizeof(bmfh) + sizeof(bmih) + bmih.biSizeImage;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih);

    // create the BMP file
    FILE *f = fopen(pic_name, "wb");
    if (f)
    {
		// dump the file header
		fwrite(reinterpret_cast<void*>(&bmfh), sizeof(bmfh), 1, f);
		// dump the info header
		fwrite(reinterpret_cast<void*>(&bmih), sizeof(bmih), 1, f);
		// dump the bitmap bits
		fwrite(reinterpret_cast<void*>(Bits), sizeof(char), bmih.biSizeImage, f);

		// close the file
		fclose(f);
	}


    // free the memory for the bitmap bits
    delete[] Bits;

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// End of code borrowed from Michael Fötsch.
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


	pDestSurface->Release();
}


void d3d_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static D3DCOLOR pre_set_colours[MAX_NUM_TIMERBARS] = 
	{
		0xffff0000, // red
		0xff00ff00, // green
		0xff0000ff, // blue

		0xff0ffff0, 
		0xfffff000, 
		0xffff00ff,
		0xffffffff,
		0xffff0f0f,
	};

	D3DVERTEX2D d3d_verts[4];

	static float max_fw = (float) gr_screen.max_w; 
	static float max_fh = (float) gr_screen.max_h; 

	d3d_verts[0].rhw   = 1;
	d3d_verts[0].color = pre_set_colours[colour];
	d3d_verts[0].sx = max_fw * x;
	d3d_verts[0].sy = max_fh * y;

	d3d_verts[1].rhw   = 1;
	d3d_verts[1].color = pre_set_colours[colour];
	d3d_verts[1].sx = max_fw * (x + w);
	d3d_verts[1].sy = max_fh * y;

	d3d_verts[2].rhw   = 1;
	d3d_verts[2].color = pre_set_colours[colour];
	d3d_verts[2].sx = max_fw * (x + w);
	d3d_verts[2].sy = max_fh * (y + h);

	d3d_verts[3].rhw   = 1;
	d3d_verts[3].color = pre_set_colours[colour];
	d3d_verts[3].sx = max_fw * x;
	d3d_verts[3].sy = max_fh * (y + h);

	gr_d3d_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	d3d_set_initial_render_state();

 	d3d_DrawPrimitive(D3DVT_VERTEX2D,D3DPT_TRIANGLEFAN,(LPVOID)d3d_verts,4);

}

void gr_d3d_push_texture_matrix(int unit)
{}
void gr_d3d_pop_texture_matrix(int unit)
{}
void gr_d3d_translate_texture_matrix(int unit, vector *shift)
{}
