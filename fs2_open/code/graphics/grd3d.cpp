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
 * $Revision: 2.48 $
 * $Date: 2003-12-17 23:25:10 $
 * $Author: phreak $
 *
 * Code for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.47  2003/12/08 22:30:02  randomtiger
 * Put render state and other direct D3D calls repetition check back in, provides speed boost.
 * Fixed bug that caused fullscreen only crash with DXT textures
 * Put dithering back in for tgas and jpgs
 *
 * Revision 2.46  2003/11/29 10:52:09  randomtiger
 * Turned off D3D file mapping, its using too much memory which may be hurting older systems and doesnt seem to be providing much of a speed benifit.
 * Added stats command for ingame stats on memory usage.
 * Trys to play intro.mve and intro.avi, just to be safe since its not set by table.
 * Added fix for fonts wrapping round in non standard hi res modes.
 * Changed D3D mipmapping to a good value to suit htl mode.
 * Added new fog colour method which makes use of the bitmap, making this htl feature backcompatible again.
 *
 * Revision 2.45  2003/11/19 20:37:24  randomtiger
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
 * Revision 2.44  2003/11/17 06:52:52  bobboau
 * got assert to work again
 *
 * Revision 2.43  2003/11/17 04:25:55  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.42  2003/11/16 04:09:24  Goober5000
 * language
 *
 * Revision 2.41  2003/11/11 03:56:11  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.40  2003/11/11 02:15:44  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.39  2003/11/06 21:10:26  randomtiger
 * Added my batching solution for more efficient d3d_string.
 * Its part of the new grd3dbatch module, most of this isnt in use but it might help out later so I've left it in.
 *
 * Revision 2.38  2003/11/02 05:50:08  bobboau
 * modified trails to render with tristrips now rather than with stinky old trifans,
 * MUCH faster now, at least one order of magnatude.
 *
 * Revision 2.37  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.36  2003/10/30 08:20:36  fryday
 * Added code to handle MR_NO_ZBUFFER in d3d VB rendering code.
 *
 * Revision 2.35  2003/10/29 02:09:17  randomtiger
 * Updated timerbar code to work properly, also added support for it in OGL.
 * In D3D red is general processing (and generic graphics), green is 2D rendering, dark blue is 3D unlit, light blue is HT&L renders and yellow is the presentation of the frame to D3D. OGL is all red for now. Use compile flag TIMERBAR_ON with code lib to activate it.
 * Also updated some more D3D device stuff that might get a bit more speed out of some cards.
 *
 * Revision 2.34  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.33  2003/10/25 06:56:05  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.32  2003/10/25 03:26:39  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.31  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.30  2003/10/18 02:45:39  phreak
 * edited gr_d3d_start_instance_matrix to make it take a vector* and a matrix*, but it doesn't do anything yet
 *
 * Revision 2.29  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.28  2003/10/16 17:36:29  randomtiger
 * D3D now has its own gamma system (stored in GammaD3D reg entry) that effects everything.
 * Put in Bobs specular fog fix.
 *
 * Revision 2.27  2003/10/16 00:17:14  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
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

#include "graphics/2d.h"

#include "globalincs/systemvars.h"
#include "globalincs/alphacolors.h"

#include "graphics/grinternal.h"

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/grd3dbatch.h"
#include "graphics/grd3dlight.h"
#include "graphics/grd3dbmpman.h"

#include "osapi/osapi.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "osapi/osregistry.h"

#include "graphics/line.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"
#include "freespace2/freespaceresource.h"   
#include "model/model.h"
#include "cmdline/cmdline.h"   

enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};
// Structures and enums
struct Vertex_buffer{
	Vertex_buffer(): ocupied(false), n_prim(0){};
	bool ocupied;
	int n_prim;
	vertex_buffer_type type;
	IDirect3DVertexBuffer8 *buffer;
};

enum stage_state{
	NONE = -1, 
	INITAL = 0, 
	DEFUSE = 1, 
	GLOW_MAPPED_DEFUSE = 2, 
	NONMAPPED_SPECULAR = 3, 
	GLOWMAPPED_NONMAPPED_SPECULAR = 4, 
	MAPPED_SPECULAR = 5, CELL = 6, 
	GLOWMAPPED_CELL = 7, 
	ADDITIVE_GLOWMAPPING = 8, 
	SINGLE_PASS_SPECMAPPING = 9, 
	SINGLE_PASS_GLOW_SPEC_MAPPING = 10};


// Defines and constants
#define MAX_SUBOBJECTS 64

#ifdef INF_BUILD
#define MAX_BUFFERS_PER_SUBMODEL 24
#else
#define MAX_BUFFERS_PER_SUBMODEL 16
#endif

#define MAX_BUFFERS MAX_POLYGON_MODELS*MAX_SUBOBJECTS*MAX_BUFFERS_PER_SUBMODEL

// External variables - booo!
extern bool env_enabled;
extern matrix View_matrix;
extern vector View_position;
extern matrix Eye_matrix;
extern vector Eye_position;
extern vector Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;

extern float Model_Interp_scale_x;	//added these three for warpin stuff-Bobbau
extern float Model_Interp_scale_y;
extern float Model_Interp_scale_z;

extern int G3_user_clip;
extern vector G3_user_clip_normal;
extern vector G3_user_clip_point;

static int D3d_dump_frames = 0;
static ubyte *D3d_dump_buffer = NULL;
static int D3d_dump_frame_number = 0;
static int D3d_dump_frame_count = 0;
static int D3d_dump_frame_count_max = 0;
static int D3d_dump_frame_size = 0;

// Variables
stage_state current_render_state = NONE;

int In_frame = 0;
int D3D_32bit = 0;

IDirect3DSurface8 *Gr_saved_surface = NULL;
Vertex_buffer vertex_buffer[MAX_BUFFERS];
extern int n_active_lights;

D3DXPLANE d3d_user_clip_plane;

// Function declarations
void shift_active_lights(int pos);
void pre_render_lights_init();
const char *d3d_error_string(HRESULT error);

void d3d_string_mem_use(int x, int y)
{
	char mem_buffer[50];
	sprintf(mem_buffer,"Texture mem free: %d Meg", GlobalD3DVars::lpD3DDevice->GetAvailableTextureMem()/1024/1024);
	gr_string( x, y, mem_buffer);
}



/**
 * This function is to be called if you wish to scale GR_1024 or GR_640 x and y positions or
 * lengths in order to keep the correctly scaled to nonstandard resolutions
 *
 * @param int *x - x value (width to be scaled), can be NULL
 * @param int *y - y value (height to be scaled), can be NULL
 * @return always true
 */
bool gr_d3d_resize_screen_pos(int *x, int *y)
{
	if(GlobalD3DVars::D3D_custom_size < 0)	return false;

	int div_by_x = (GlobalD3DVars::D3D_custom_size == GR_1024) ? 1024 : 640;
	int div_by_y = (GlobalD3DVars::D3D_custom_size == GR_1024) ?  768 : 480;
			
	if(x) {
		(*x) *= GlobalD3DVars::d3dpp.BackBufferWidth;
		(*x) /= div_by_x;
	}

	if(y) {
		(*y) *= GlobalD3DVars::d3dpp.BackBufferHeight;
		(*y) /= div_by_y;
	}

	return true;
}

/**
 *
 * @param int *x - x value (width to be unsacled), can be NULL
 * @param int *y - y value (height to be unsacled), can be NULL
 * @return always true
 */
bool gr_d3d_unsize_screen_pos(int *x, int *y)
{
	if(GlobalD3DVars::D3D_custom_size < 0)	return false;

	int mult_by_x = (GlobalD3DVars::D3D_custom_size == GR_1024) ? 1024 : 640;
	int mult_by_y = (GlobalD3DVars::D3D_custom_size == GR_1024) ?  768 : 480;
			
	if(x) {
		(*x) *= mult_by_x;
		(*x) /= GlobalD3DVars::d3dpp.BackBufferWidth;
	}

	if(y) {
		(*y) *= mult_by_y;
		(*y) /= GlobalD3DVars::d3dpp.BackBufferHeight;
	}

	return true;
}

/**
 *
 * @param int *x - x value (width to be unsacled), can be NULL
 * @param int *y - y value (height to be unsacled), can be NULL
 * @return always true
 */
bool gr_d3d_unsize_screen_posf(float *x, float *y)
{
	if(GlobalD3DVars::D3D_custom_size < 0)	return false;

	float mult_by_x = (float) ((GlobalD3DVars::D3D_custom_size == GR_1024) ? 1024 : 640);
	float mult_by_y = (float) ((GlobalD3DVars::D3D_custom_size == GR_1024) ?  768 : 480);
			
	if(x) {
		(*x) *= mult_by_x;
		(*x) /= (float) GlobalD3DVars::d3dpp.BackBufferWidth;
	}

	if(y) {
		(*y) *= mult_by_y;
		(*y) /= (float) GlobalD3DVars::d3dpp.BackBufferHeight;
	}

	return true;
}

void d3d_fill_pixel_format(PIXELFORMAT *pixelf, D3DFORMAT tformat);

// Whats all this then? Can we get rid of it? - RT

// LPVOID lpBufStart, lpPointer, lpInsStart;
// int Exb_size;

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

// No objects should be rendered before this frame
void d3d_start_frame()
{
	if(!GlobalD3DVars::D3D_activate) return;

	HRESULT ddrval;



	if (!GlobalD3DVars::D3D_inited) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Start frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 1 ) return;

	In_frame++;


	ddrval = GlobalD3DVars::lpD3DDevice->BeginScene();
	if (ddrval != D3D_OK )	{
		//mprintf(( "Failed to begin scene!\n%s\n", d3d_error_string(ddrval) ));
		return;
	}
}

// No objects should be rendered after this frame
void d3d_stop_frame()
{

	if (!GlobalD3DVars::D3D_inited) return;
	if(!GlobalD3DVars::D3D_activate) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Stop frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 0 ) return;

	In_frame--;
	

	TIMERBAR_END_FRAME();
	if(FAILED(GlobalD3DVars::lpD3DDevice->EndScene()))
	{
		return;
	}
	TIMERBAR_START_FRAME();

	d3d_batch_end_frame();
						 
	TIMERBAR_PUSH(4);
	// Must cope with device being lost
	if(GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL) == D3DERR_DEVICELOST)
	{
		d3d_lost_device();
	}
	TIMERBAR_POP();
}


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
		if(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
			d3d_SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
	}


	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

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
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

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

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = CELL;
		
}

void set_stage_for_cell_glowmapped_shaded(){
	if(current_render_state == GLOWMAPPED_CELL)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

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

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = GLOWMAPPED_CELL;
		
}

void set_stage_for_additive_glowmapped(){
	if(current_render_state == ADDITIVE_GLOWMAPPING)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = ADDITIVE_GLOWMAPPING;
}

void set_stage_for_defuse(){
	if(current_render_state == DEFUSE)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = DEFUSE;
		
}

void set_stage_for_glow_mapped_defuse(){
	if(current_render_state == GLOW_MAPPED_DEFUSE)return;
	if(GLOWMAP < 0){
		set_stage_for_defuse();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

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

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = GLOW_MAPPED_DEFUSE;
}

void set_stage_for_defuse_and_non_mapped_spec(){
	if(current_render_state == NONMAPPED_SPECULAR)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = NONMAPPED_SPECULAR;
}

void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(){
	if(current_render_state == GLOWMAPPED_NONMAPPED_SPECULAR)return;
	if(GLOWMAP < 0){
		set_stage_for_defuse_and_non_mapped_spec();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

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

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);

	current_render_state = GLOWMAPPED_NONMAPPED_SPECULAR;
}

bool set_stage_for_spec_mapped(){
	if(current_render_state == MAPPED_SPECULAR)return true;
	if(SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);


	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
//	d3d_SetTexture(0, SPECMAP);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE);


	current_render_state = MAPPED_SPECULAR;
	return true;
}

//set the base texture for stage 0 and the spec texture for stage 1
void set_stage_for_single_pass_specmapping(int SAME){
	//D3DPMISCCAPS_TSSARGTEMP
	static int same = -1;
//	if((current_render_state == SINGLE_PASS_SPECMAPPING) && (same == SAME))return;
	if(SPECMAP < 0)return;
	if(!SAME){		
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
	
		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_TEMP );
	
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEMP);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_CURRENT);

		same = SAME;
	}else{
	//	d3d_SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255,64,64,64));
		
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE4X);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 4, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 4, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_ADD);

		same = SAME;
	}
	current_render_state = SINGLE_PASS_SPECMAPPING;
}

//glow texture stage 3
void set_stage_for_single_pass_glow_specmapping(int SAME){
	//D3DPMISCCAPS_TSSARGTEMP
	static int same = -1;
//	if((current_render_state == SINGLE_PASS_GLOW_SPEC_MAPPING) && (same == SAME))return;
	if(SPECMAP < 0)return;
	if(GLOWMAP < 0){
		set_stage_for_single_pass_specmapping(SAME);
		return;
	}
	float u_scale = 1.0f, v_scale = 1.0f;
	if(!SAME){
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
	
		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_TEMP );
	
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_TEMP);
		d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_CURRENT);

		same = SAME;
	}else{
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
 		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);

		gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 5);

		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE4X);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 4, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 4, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 4, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 5, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 5, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 5, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 5, D3DTSS_COLOROP, D3DTOP_ADD);

		same = SAME;
	}
	current_render_state = SINGLE_PASS_GLOW_SPEC_MAPPING;
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


void gr_d3d_flip()
{
	if(!GlobalD3DVars::D3D_activate) return;
	int mx, my;	
	
	// Attempt to allow D3D8 to recover from task switching
	if(GlobalD3DVars::lpD3DDevice->TestCooperativeLevel() != D3D_OK) {
		d3d_lost_device();
	}

	gr_reset_clip();	

	mouse_eval_deltas();

	if ( mouse_is_visible() )	{				
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		
		if ( Gr_cursor != -1 )	{
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my, false);
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

void gr_d3d_fade_in(int instantaneous)
{
}

void gr_d3d_fade_out(int instantaneous)
{
}

void d3d_setup_format_components(PIXELFORMAT *surface, color_gun *r_gun, color_gun *g_gun, color_gun *b_gun, color_gun *a_gun);

int gr_d3d_save_screen()
{
	if(!GlobalD3DVars::D3D_activate) return -1;
	gr_reset_clip();

	// Lets not bother with this if its a window or it causes a debug DX8 error
	if(GlobalD3DVars::D3D_window == true) {
		return 0;
	}

	if ( Gr_saved_surface )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	IDirect3DSurface8 *front_buffer_a8r8g8b8 = NULL;

	// Problem that we can only get front buffer in A8R8G8B8
	mprintf(("Creating surface for front buffer of size: %d %d",gr_screen.max_w, gr_screen.max_h));
	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, D3DFMT_A8R8G8B8, &front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		return -1;
	}

	if(FAILED(GlobalD3DVars::lpD3DDevice->GetFrontBuffer(front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to get front buffer");
		goto Failed;
	}

	// Get the back buffer format
	PIXELFORMAT dest_format;
	color_gun r_gun, g_gun, b_gun, a_gun;

	d3d_fill_pixel_format( &dest_format, GlobalD3DVars::d3dpp.BackBufferFormat);
	d3d_setup_format_components(&dest_format, &r_gun, &g_gun, &b_gun, &a_gun);


	// Create a surface of a compatable type
	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, 
		GlobalD3DVars::d3dpp.BackBufferFormat, &Gr_saved_surface))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		goto Failed;
	}

	// Make a copy of the thing
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
		
	if(FAILED(GlobalD3DVars::lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &dest_buffer)))
	{
		DBUGFILE_OUTPUT_0("FAiled");
		return;
	}

	if(FAILED(GlobalD3DVars::lpD3DDevice->CopyRects(Gr_saved_surface, NULL, 0, dest_buffer, NULL)))
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
 * Support function for gr_d3d_set_gamma 
 *
 */
inline ushort d3d_ramp_val(UINT i, float recip_gamma, int scale = 65535)
{
    return static_cast<ushort>(scale*pow(i/255.f, recip_gamma));
}

/**
 * Set the gamma, or brightness
 *
 * @param float gamma
 * @return void
 */
void gr_d3d_set_gamma(float gamma)
{
	if(Cmdline_no_set_gamma) return;

	Gr_gamma = gamma;
	D3DGAMMARAMP gramp;

	// Create the Gamma lookup table
	for (int i = 0; i < 256; i++ ) {
	  	gramp.red[i] = gramp.green[i] = gramp.blue[i] = d3d_ramp_val(i, gamma);
	}

   	GlobalD3DVars::lpD3DDevice->SetGammaRamp(D3DSGR_CALIBRATE, &gramp);
}

/**
 * Toggle polygon culling mode Counter-clockwise or none
 *
 * @param int cull 
 * @return void
 */
void gr_d3d_set_cull(int cull)
{
	d3d_SetRenderState( D3DRS_CULLMODE, cull ? D3DCULL_CCW : D3DCULL_NONE );				
}

/**
 * Cross fade, actually works now
 *
 * @param int bmap1 
 * @param int bmap2 
 * @param int x1 
 * @param int y1 
 * @param int x2 
 * @param int y2 
 * @param float pct	- Fade value, between 0.0 - 1.0
 * @return void
 */
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
   	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f - pct );
	gr_bitmap(x1, y1);

  	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct );
	gr_bitmap(x2, y2);
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

	hr = GlobalD3DVars::lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
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

//*******Vertex buffer stuff*******//

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
		for(int k = 0; k<list->n_poly*3; k++){
				V = &v[k];
				L = &list->vert[k];
				N = &list->norm[k];

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

		vertex_buffer[idx].buffer->Unlock();

		vertex_buffer[idx].ocupied = true;
		vertex_buffer[idx].n_prim  = list->n_poly;
		vertex_buffer[idx].type = TRILIST_;
	}
	return idx;
}
//makes the vertex buffer, returns an index to it
int gr_d3d_make_flat_buffer(poly_list *list){
	int idx = find_first_empty_buffer();

	if(idx > -1){
		IDirect3DVertexBuffer8 **buffer = &vertex_buffer[idx].buffer;

		d3d_CreateVertexBuffer(D3DVT_LVERTEX, (list->n_poly*3), NULL, (void**)buffer);

		D3DLVERTEX *v, *V;
		vertex *L;
//		vector *N;

		vertex_buffer[idx].buffer->Lock(0, 0, (BYTE **)&v, NULL);
		for(int k = 0; k<list->n_poly*3; k++){
				V = &v[k];
				L = &list->vert[k];
			//	N = &list->norm[k]; //these don't have normals :\

				V->sx = L->x;
				V->sy = L->y;
				V->sz = L->z;

				V->tu = L->u;
				V->tv = L->v;

				V->color = D3DCOLOR_ARGB(255,L->r,L->g,L->b);
		}

		vertex_buffer[idx].buffer->Unlock();

		vertex_buffer[idx].ocupied = true;
		vertex_buffer[idx].n_prim  = list->n_poly;
		vertex_buffer[idx].type = FLAT_;
	}
	return idx;
}


//makes line buffer, returns index
int gr_d3d_make_line_buffer(line_list *list){
/*
	int idx = find_first_empty_buffer();

	if(idx > -1){
		IDirect3DVertexBuffer8 **buffer = &vertex_buffer[idx].buffer;

		d3d_CreateVertexBuffer(D3DVT_LVERTEX, (list->n_line*2), NULL, (void**)buffer);

		D3DLVERTEX *v, *V;
		vertex *L;
//		vector *N;
		int c = 0;

		vertex_buffer[idx].buffer->Lock(0, 0, (BYTE **)&v, NULL);
		for(int k = 0; k<list->n_line; k++){
			for(int j = 0; j < 2; j++){
				V = &v[(k*2)+j];
				L = &list->vert[k][j];

				V->sx = L->x;
				V->sy = L->y;
				V->sz = L->z;

				V->tu = L->u;
				V->tv = L->v;

				V->color = D3DCOLOR_ARGB(255,L->r,L->g,L->b);
				c++;
			}
		}

		vertex_buffer[idx].buffer->Unlock();

		vertex_buffer[idx].ocupied = true;
		vertex_buffer[idx].n_prim  = list->n_line;
		vertex_buffer[idx].type = LINELIST_;
	}
//	return-1;
	return idx;
	*/
	return-1;

}
	
//kills buffers dead!
void gr_d3d_destroy_buffer(int idx){
	vertex_buffer[idx].buffer->Release();
	vertex_buffer[idx].ocupied = false;
}

//enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};

void gr_d3d_render_line_buffer(int idx){
	if(idx<0)return;
	if(!vertex_buffer[idx].ocupied)return;
	d3d_SetVertexShader(D3DVT_LVERTEX);

	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, vertex_buffer[idx].buffer, sizeof(D3DLVERTEX));
	
	gr_d3d_fog_set(GR_FOGMODE_NONE, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);		//it's a HUD item, should never be fogged

	gr_d3d_set_cull(0);
	d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_LINELIST , 0, vertex_buffer[idx].n_prim);
	d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
	gr_d3d_set_cull(1);
}

bool the_lights_are_on;
extern bool lighting_enabled;
void gr_d3d_render_buffer(int idx)
{
	TIMERBAR_PUSH(3);
//	GlobalD3DVars::d3d_caps.MaxActiveLights = 1;

//	if(!the_lights_are_on){
//		d3d_SetVertexShader(D3DVT_VERTEX);
//		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
//		the_lights_are_on = true;
//	}												  

	extern D3DMATERIAL8 material;
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
	if(!lighting_enabled){
		int l = int(255.0f*gr_screen.current_alpha);
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(l,l,l,l));
	}else{
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
	}
	GlobalD3DVars::lpD3DDevice->SetMaterial(&material);

	color old_fog_color;

	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)//when fogging don't fog unlit things, but rather fade them in a fog like manner -Bobboau
		if(!lighting_enabled){
			old_fog_color = gr_screen.current_fog_color;
			gr_d3d_fog_set(gr_screen.current_fog_mode, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);
		}


		if(!vertex_buffer[idx].ocupied) {
			TIMERBAR_POP();
			return;
		}
	if(vertex_buffer[idx].type == LINELIST_) {
		gr_d3d_render_line_buffer(idx); 
		TIMERBAR_POP(); 
		return;
	}

	float u_scale = 1.0f, v_scale = 1.0f;

	gr_alpha_blend ab = ALPHA_BLEND_NONE;
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	
		ab = ALPHA_BLEND_ALPHA_ADDITIVE;

//	int same = (gr_screen.current_bitmap != SPECMAP)?0:1;
//	if(!same)d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0);
	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0);
//	if(!gr_zbuffering_mode)
//		gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_NONE);
	if(gr_zbuffering_mode == GR_ZBUFF_READ){
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_READ);
			d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}else
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);

	pre_render_lights_init();
	if(lighting_enabled){
		shift_active_lights(0);
	}

//	bool single_pass_spec = false;

	if(GLOWMAP > -1 && !Cmdline_noglow){
		//glowmapped
			gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		 	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 1);
			
	 		
			set_stage_for_glow_mapped_defuse();
	}else{
		//non glowmapped
			set_stage_for_defuse();
	}

	int passes = (n_active_lights / GlobalD3DVars::d3d_caps.MaxActiveLights);
	d3d_SetVertexShader(D3DVT_VERTEX);

	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, vertex_buffer[idx].buffer, sizeof(D3DVERTEX));

//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);



	if(!lighting_enabled)
	{
		TIMERBAR_POP();
		return;
	}
	//single pass specmap rendering ends here
/*	if(single_pass_spec){
		return;
	}*/
	if (gr_zbuffering_mode)
	{
		gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
	}
	else
		gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
		

	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE){
		old_fog_color = gr_screen.current_fog_color;
		gr_d3d_fog_set(gr_screen.current_fog_mode, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);
	}

//	if(single_pass_spec)set_stage_for_single_pass_specmapping(same);
//	else set_stage_for_defuse();
	set_stage_for_defuse();


	for(int i = 1; i<passes; i++){
		shift_active_lights(i);
		GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
	}

	pre_render_lights_init();
	shift_active_lights(0);

	//spec mapping
	if(SPECMAP > -1){
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
				TIMERBAR_POP();
				return;
			}

		if(set_stage_for_spec_mapped()){
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
			GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
			for(int i = 1; i<passes; i++){
				shift_active_lights(i);
				GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , 0, vertex_buffer[idx].n_prim);
			}
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		}
	}

	// Revert back to old fog state
	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)gr_d3d_fog_set(gr_screen.current_fog_mode, old_fog_color.red,old_fog_color.green,old_fog_color.blue, gr_screen.fog_near, gr_screen.fog_far);

	TIMERBAR_POP();
}

//*******matrix stuff*******//

/*
	//the projection matrix; fov, aspect ratio, near, far
 	void (*gf_set_proj_matrix)(float, float, float, float);
  	void (*gf_end_proj_matrix)();
	//the view matrix
 	void (*gf_set_view_matrix)(vector *, matrix*);
  	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(vector *);
 	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(vector *, matrix*);
	void (*gf_start_angles_instance_matrix)(vector *, angles*);
	void (*gf_end_instance_matrix)();
*/

//fov = (4.0/9.0)*(PI)*View_zoom
//ratio = screen.aspect
//near = 1.0
//far = 30000
	//the projection matrix; fov, aspect ratio, near, far
void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f){
//	proj_matrix_stack->Push();
	D3DXMATRIX mat;
	D3DXMatrixPerspectiveFovLH(&mat, fov, ratio, n, f);
//	D3DXMatrixPerspectiveFovLH(&mat, (4.0f/9.0f)*(D3DX_PI)*View_zoom, 1.0f/gr_screen.aspect, 0.2f, 30000);
//	proj_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
}

void gr_d3d_end_proj_matrix(){
//	proj_matrix_stack->Pop();
}

	//the view matrix
void gr_d3d_set_view_matrix(vector* offset, matrix *orient){

//	view_matrix_stack->Push();

	D3DXMATRIX mat, scale_m;

	D3DXMATRIX MAT(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

	D3DXMatrixIdentity(&mat);
	D3DXMatrixInverse(&mat, NULL, &MAT);
//	view_matrix_stack->LoadMatrix(&mat);
//	D3DXMatrixScaling(&scale_m, 3.0f, 3.0f, 3.0f);
//	D3DXMatrixMultiply(&mat, &scale_m, &mat);

	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, &mat);


}

void gr_d3d_end_view_matrix(){
//	view_matrix_stack->Pop();
//	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, view_matrix_stack->GetTop());
}

	//object position and orientation
void gr_d3d_start_instance_matrix(vector* offset, matrix *orient){

	D3DXMATRIX *old_world = world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX world(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

	D3DXMatrixMultiply(&world, old_world, &world);

//	D3DXMatrixScaling(&scale_m, 3.0f, 3.0f, 3.0f);
//	D3DXMatrixMultiply(&world, &scale_m, &world);

	world_matrix_stack->LoadMatrix(&world);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
}

void gr_d3d_start_angles_instance_matrix(vector* offset, angles *orient){

	D3DXMATRIX current = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat,orient->h,orient->p,orient->b);
	D3DXMATRIX world(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);
	D3DXMatrixMultiply(&mat, &mat, &world);
	D3DXMatrixMultiply(&mat, &mat, &current);

//	D3DXMatrixScaling(&scale_m, 3.0f, 3.0f, 3.0f);

	world_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());

}

void gr_d3d_end_instance_matrix()
{
	world_matrix_stack->Pop();
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());

}


	//object scaleing
void gr_d3d_set_scale_matrix(vector* scale){

	D3DXMATRIX mat = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMatrixScaling(&scale_m, scale->xyz.x, scale->xyz.y, scale->xyz.z);
	D3DXMatrixMultiply(&mat, &scale_m, &mat);

	world_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, &mat);

}

void gr_d3d_end_scale_matrix()
{
	world_matrix_stack->Pop();
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());

}




/**
 * Turns on clip plane clip plane
 * Doenst seem to work at the moment
 *
 * @return void
 */
void d3d_start_clip(){

	D3DXVECTOR3 point(G3_user_clip_point.xyz.x,G3_user_clip_point.xyz.y,G3_user_clip_point.xyz.z); 
	D3DXVECTOR3	normal(G3_user_clip_normal.xyz.x,G3_user_clip_normal.xyz.y,G3_user_clip_normal.xyz.z);

	D3DXPlaneFromPointNormal(&d3d_user_clip_plane, &point, &normal);

	HRESULT hr = GlobalD3DVars::lpD3DDevice->SetClipPlane(0, d3d_user_clip_plane);
  //	Assert(SUCCEEDED(hr));
	if(FAILED(hr))
	{
		mprintf(("Failed to set clip plane\n"));
	}

	hr = d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , D3DCLIPPLANE0);
	Assert(SUCCEEDED(hr));
}

/**
 * Turns off clip plane
 *
 * @return void
 */
void d3d_end_clip(){
	d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , FALSE);
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
