/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3DTexture.cpp $
 * $Revision: 2.53 $
 * $Date: 2005-11-13 06:44:18 $
 * $Author: taylor $
 *
 * Code to manage loading textures into VRAM for Direct3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.52  2005/08/20 20:34:51  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.51  2005/06/19 09:04:40  taylor
 * make sure to reset size to 0 on texture free
 *
 * Revision 2.50  2005/06/19 02:31:51  taylor
 * allow screenshots and backsaves in windowed mode
 * account for D3D_textures_in size so that it doesn't hit negative values
 *
 * Revision 2.49  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.48  2005/04/24 12:56:42  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 2.47  2005/03/10 08:00:05  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.46  2005/03/07 13:10:21  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.45  2005/02/15 00:03:36  taylor
 * don't try and draw starfield bitmaps if they aren't valid
 * make AB thruster stuff in ship_create() a little less weird
 * replace an Int3() with debug warning and fix crash in docking code
 * make D3D Textures[] allocate on use like OGL does, can only use one anyway
 *
 * Revision 2.44  2005/01/01 11:24:22  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 2.43  2004/10/31 21:40:11  taylor
 * move some otherwise bmpman stuff into grd3dbmpman.cpp
 *
 * Revision 2.42  2004/07/29 03:41:46  taylor
 * plug memory leaks
 *
 * Revision 2.41  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.40  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.39  2004/07/11 03:22:49  bobboau
 * added the working decal code
 *
 * Revision 2.38  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.37  2004/06/06 12:25:20  randomtiger
 * Added new compression option -pcx32dds, build posted in RSB forum.
 * Changed flag because of launcher bug, have fixed launcher bug, will distribute later.
 * Also removed experimental flag from launcher flag list, stupid people were reporting bugs on unfinished code.
 *
 * Revision 2.36  2004/04/03 06:22:32  Goober5000
 * fixed some stub functions and a bunch of compile warnings
 * --Goober5000
 *
 * Revision 2.35  2004/03/19 14:51:55  randomtiger
 * New command line parameter: -d3d_lesstmem causes D3D to bypass V's secondry texture system.
 *
 * Revision 2.34  2004/03/19 12:35:58  randomtiger
 * Further D3D texture system simplification.
 *
 * Revision 2.33  2004/03/19 11:44:04  randomtiger
 * Removed -d3d_notmanaged param.
 * Tided D3D texture code. Merged remaining section code into the rest of the system.
 * Prepared for removal of code causing waste of memory for secondry store of textures.
 *
 * Revision 2.32  2004/02/28 14:14:56  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.31  2004/02/20 04:29:54  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.30  2004/02/16 11:47:33  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.29  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.28  2004/02/14 00:18:32  randomtiger
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
 * Revision 2.27  2004/01/29 01:34:02  randomtiger
 * Added malloc montoring system, use -show_mem_usage, debug exes only to get an ingame list of heap usage.
 * Also added -d3d_notmanaged flag to activate non managed D3D path, in experimental stage.
 *
 * Revision 2.26  2004/01/26 20:03:51  randomtiger
 * Fix to blurring of interface bitmaps from TGA and JPG.
 * Changes to the pointsprite system, better but not perfect yet.
 *
 * Revision 2.25  2004/01/20 23:01:52  Goober5000
 * added some initialization to get rid of warnings
 * --Goober5000
 *
 * Revision 2.24  2003/12/08 22:30:02  randomtiger
 * Put render state and other direct D3D calls repetition check back in, provides speed boost.
 * Fixed bug that caused fullscreen only crash with DXT textures
 * Put dithering back in for tgas and jpgs
 *
 * Revision 2.23  2003/12/05 18:17:06  randomtiger
 * D3D now supports loading for DXT1-5 into the texture itself, defaults to on same as OGL.
 * Fixed bug in old ship choice screen that stopped ani repeating.
 * Changed all builds (demo, OEM) to use retail reg path, this means launcher can set all them up successfully.
 *
 * Revision 2.22  2003/12/04 20:39:09  randomtiger
 * Added DDS image support for D3D
 * Added new command flag '-ship_choice_3d' to activate 3D models instead of ani's in ship choice, feature now off by default
 * Hopefully have fixed D3D text batching bug that caused old values to appear
 * Added Hud_target_object_factor variable to control 3D object sizes of zoom in HUD target
 * Fixed jump nodes not showing
 *
 * Revision 2.21  2003/12/03 19:27:00  randomtiger
 * Changed -t32 flag to -jpgtga
 * Added -query_flag to identify builds with speech not compiled and other problems
 * Now loads up launcher if videocard reg entry not found
 * Now offers to go online to download launcher if its not present
 * Changed target view not to use lower res texture, hi res one is already chached so might as well use it
 *
 * Revision 2.20  2003/11/19 20:37:24  randomtiger
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
 * Revision 2.19  2003/11/17 06:52:52  bobboau
 * got assert to work again
 *
 * Revision 2.18  2003/11/16 04:09:23  Goober5000
 * language
 *
 * Revision 2.17  2003/11/12 00:31:45  Kazan
 * A few multi tweaks - fixed a couple warning-killing collisions between goober and me (kazan)
 *
 * Revision 2.16  2003/11/11 03:56:11  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.15  2003/11/11 02:15:44  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.14  2003/11/07 18:31:02  randomtiger
 * Fixed a nohtl call to htl funcs (crash with NULL pointer)
 * Fixed a bug with 32bit PCX code.
 * Fixed a bug in the d3d_string batch system that was messing up screen shaking.
 * Added a couple of checks to try and stop timerbar push and pop overloads, check returns missing pops if you use the system.
 * Put in a higher res icon until we get something better sorted out.
 *
 * Revision 2.13  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.12  2003/10/26 00:31:58  randomtiger
 * Fixed hulls not drawing (with Phreaks advise).
 * Put my 32bit PCX loading under PCX_32 compile flag until its working.
 * Fixed a bug with res 640x480 I introduced with my non standard mode code.
 * Changed JPG and TGA loading command line param to "-t32"
 *
 * Revision 2.11  2003/10/24 17:35:05  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.10  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.9  2003/10/16 17:36:29  randomtiger
 * D3D now has its own gamma system (stored in GammaD3D reg entry) that effects everything.
 * Put in Bobs specular fog fix.
 *
 * Revision 2.8  2003/08/22 07:35:08  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.7  2003/08/21 20:54:38  randomtiger
 * Fixed switching - RT
 *
 * Revision 2.6  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.5  2003/08/05 23:45:18  bobboau
 * glow maps, for some reason they wern't in here, they should be now,
 * also there is some debug code for changeing the FOV in game,
 * and I have some changes to the init code to try and get a 32 or 24 bit back buffer
 * if posable, this may cause problems for people
 * the changes were all marked and if needed can be undone
 *
 * Revision 2.4  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.3  2003/03/02 05:43:48  penguin
 * ANSI C++ - fixed non-compliant casts to unsigned short and unsigned char
 *  - penguin
 *
 * Revision 2.2  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.1.2.26  2002/11/16 20:09:54  randomtiger
 *
 * Changed my fog hack to be valid code. Put large texture check back in.
 * Added some blending type checks. - RT
 *
 * Revision 2.1.2.25  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.1.2.24  2002/11/10 11:32:29  randomtiger
 *
 * Made D3D8 mipmapping optional on command line flag -d3dmipmip, off by default.
 * When on is now less blury. - RT
 *
 * Revision 2.1.2.23  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.1.2.22  2002/11/05 10:27:39  randomtiger
 *
 * Fixed windowed mode bug I introduced.
 * Added Antialiasing functionality, can only be sure it works on GF4 in 1024 mode. - RT
 *
 * Revision 2.1.2.21  2002/11/04 23:53:25  randomtiger
 *
 * Added new command line parameter -d3dlauncher which brings up the launcher.
 * This is needed since FS2 DX8 now stores the last successful details in the registry and
 * uses them to choose the adapter and mode to run in unless its windowed or they are not set.
 * Added some code for Antialiasing but it messes up the font but hopefully that can be fixed later. - RT
 *
 * Revision 2.1.2.20  2002/11/04 21:24:59  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.1.2.19  2002/11/04 03:02:29  randomtiger
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
 * Revision 2.1.2.18  2002/11/02 15:22:34  randomtiger
 *
 * Small addition to my TGA code. Trys to determine if TGA is using an alpha channel, if so an alpha texture format it used.
 *
 * Revision 2.1.2.17  2002/11/02 13:54:26  randomtiger
 *
 * Made a few cunning alterations to get rid of that alpha bug but might have a slight slowdown.
 * Non alpha textures are now alpha textures with (if texture format supported) just one bit for alpha.
 * And non alpha blending is now alpha blending with automatic disregaring of 0 alpha.
 *
 * Revision 2.1.2.16  2002/10/30 22:57:21  randomtiger
 *
 * Changed DX8 code to not use set render and texture states if they are already set to that value.
 * Disabled buffer saving and restoring code when windowed to stop DX8 debug runs from crashing. - RT
 *
 * Revision 2.1.2.15  2002/10/26 01:24:22  randomtiger
 * Fixed debug bitmap compiling bug.
 * Fixed tga bug. - RT
 *
 * Revision 2.1.2.14  2002/10/22 17:46:18  randomtiger
 * Fixed new TGA code texturing bug. - RT
 *
 * Revision 2.1.2.13  2002/10/21 16:33:41  randomtiger
 * Added D3D only 32 bit TGA functionality. Will load a texture as big as your graphics card allows. Code not finished yet and will forge the beginnings of the new texture system. - RT
 *
 * Revision 2.1.2.12  2002/10/19 23:56:40  randomtiger
 * Changed generic bitmap code to allow maximum dimensions to be determined by 3D's engines maximum texture size query.
 * Defaults to 256 as it was before. Also added base code for reworking the texture code to be more efficient. - RT
 *
 * Revision 2.1.2.11  2002/10/14 21:52:02  randomtiger
 * Finally tracked down and killed off that 8 bit alpha bug.
 * So the font, HUD and 8 bit ani's now work fine. - RT
 *
 * Revision 2.1.2.10  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.1.2.9  2002/10/08 14:33:27  randomtiger
 * OK, I've fixed the z-buffer problem.
 * However I have abandoned using w-buffer for now because of problems.
 * I think I know how to solve it but Im not sure it would make much difference given the way FS2 engine works.
 * I have left code in there of use if anyone wants to turn their hand to it. However for now
 * we just need to crack of the alpha problem then we will have FS2 fully wokring in DX8 on GF4 in 32 bit.
 *
 * Revision 2.1.2.8  2002/10/04 00:48:42  randomtiger
 * Fixed video memory leaks
 * Added code to cope with lost device, not tested
 * Got rid of some DX5 stuff we definately dont need
 * Moved some enum's into internal,h because gr_d3d_set_state should be able to be called from any dx file
 * Cleaned up some stuff - RT
 *
 * Revision 2.1.2.7  2002/10/03 08:32:08  unknownplayer
 *
 * Hacked in a windowed mode so we can properly debug this without using
 * monitors (although I drool at the concept of having that!)
 *
 * Revision 2.1.2.6  2002/10/02 17:52:32  randomtiger
 * Fixed blue lighting bug.
 * Put filtering flag set back in that I accidentally removed
 * Added some new functionality to my debuging system - RT
 *
 * Revision 2.1.2.5  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.1.2.4  2002/09/28 22:13:43  randomtiger
 * Sorted out some bits and pieces. The background nebula blends now which is nice. – RT
 *
 * Revision 2.1.2.3  2002/09/28 12:20:32  randomtiger
 * Just a tiny code change that lets stuff work in 16 bit.
 * For some reason 16 bit code was taking a different code path for displaying textures.
 * So until I unstand why, Im cutting off that codepath because it isnt easy to convert into DX8.
 *
 * Revision 2.1.2.2  2002/09/28 00:18:08  randomtiger
 * Did some work on trying to get textures to load from pcx, define TX_ATTEMPT for access to it.
 * Converted some DX7 blending calls to DX8 which a fair difference to ingame visuals.
 *
 * - RT
 *
 * Revision 2.1.2.1  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
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
 * 22    9/09/99 8:53p Dave
 * Fixed multiplayer degenerate orientation case problem. Make sure warp
 * effect never goes lower than LOD 1. 
 * 
 * 21    9/05/99 11:19p Dave
 * Made d3d texture cache much more safe. Fixed training scoring bug where
 * it would backout scores without ever having applied them in the first
 * place.
 * 
 * 20    8/16/99 4:04p Dave
 * Big honking checkin.
 * 
 * 19    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 18    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 17    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 16    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 15    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 14    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 13    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 12    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 11    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 8     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 7     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 6     12/01/98 6:12p Johnson
 * Make sure to page in weapon impact animations as xparent textures.
 * 
 * 5     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 37    6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 36    5/23/98 6:12p John
 * added code to use registry to force preloading of textures or not.
 * 
 * 35    5/23/98 5:17p John
 * added reg key to set texture divider
 * 
 * 34    5/23/98 5:01p John
 * made agp preloading happen if >= 6 MB of VRAM.
 * 
 * 33    5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 32    5/22/98 10:29p John
 * made direct3d textures scale as little as glide.
 * 
 * 31    5/22/98 12:54p John
 * forced all cards to use a max of 256 pixel wide textures, but added a
 * registry setting to disable it.
 * 
 * 30    5/21/98 9:56p John
 * Made Direct3D work with classic alpha-blending only devices, like the
 * Virge.  Added a texture type XPARENT that fills the alpha in in the
 * bitmap for Virge.   Added support for Permedia by making making
 * additive alphablending be one/one instead of alpha/one, which didn't
 * work, and there is no way to tell this from caps.
 * 
 * 29    5/20/98 10:23a John
 * put in code to fix an optimized build problem.
 * 
 * 28    5/18/98 8:26p John
 * Made cards with only 1bpp alpha fonts work.
 * 
 * 27    5/12/98 7:53p John
 * Fixed some 3dfx d3d bugs on allenders, jasen and johnson's computers
 * caused by 8:3:3:2 format being used, but not liked by the card.
 * 
 * 26    5/12/98 8:18a John
 * Put in code to use a different texture format for alpha textures and
 * normal textures.   Turned off filtering for aabitmaps.  Took out
 * destblend=invsrccolor alpha mode that doesn't work on riva128. 
 * 
 * 25    5/11/98 10:58a John
 * Fixed pilot name cursor bug.  Started adding in code for alphachannel
 * textures.
 * 
 * 24    5/09/98 12:37p John
 * More texture caching
 * 
 * 23    5/09/98 12:16p John
 * Even better texture caching.
 * 
 * 22    5/09/98 11:07a John
 * Better Direct3D texture caching
 * 
 * 21    5/08/98 5:41p John
 * 
 * 20    5/08/98 5:36p John
 * MAde texturing blink white but not crash
 * 
 * 19    5/07/98 3:02p John
 * Mpre texture cleanup.   You can now reinit d3d without a crash.
 * 
 * 18    5/07/98 11:31a John
 * Removed DEMO defines
 * 
 * 17    5/07/98 10:28a John
 * Made texture format use 4444.   Made fonts use alpha to render.
 * 
 * 16    5/07/98 9:40a John
 * Fixed some bitmap transparency issues with Direct3D.
 * 
 * 15    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 14    5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 13    5/03/98 10:52a John
 * Made D3D sort of work on 3dfx.
 * 
 * 12    5/03/98 10:43a John
 * Working on Direct3D.
 * 
 * 11    4/09/98 11:05a John
 * Removed all traces of Direct3D out of the demo version of Freespace and
 * the launcher.
 * 
 * 10    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 9     3/08/98 10:25a John
 * Made textures in VRAM reload if they changed
 * 
 * 8     3/07/98 8:29p John
 * Put in some Direct3D features.  Transparency on bitmaps.  Made fonts &
 * aabitmaps render nice.
 * 
 * 7     3/06/98 5:39p John
 * Started adding in aabitmaps
 * 
 * 6     3/02/98 6:00p John
 * Moved MAX_BITMAPS into BmpMan.h so the stuff in the graphics code that
 * is dependent on it won't break if it changes.   Made ModelCache slots
 * be equal to MAX_OBJECTS which is what it is.
 * 
 * 5     2/17/98 7:46p John
 * Took out debug code
 * 
 * 4     2/17/98 7:28p John
 * Got fonts and texturing working in Direct3D
 * 
 * 3     2/06/98 4:56p John
 * Turned off texturing
 * 
 * 2     2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 1     2/03/98 9:24p John
 *
 * $NoKeywords: $
 */

#include <D3dx8tex.h>

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "osapi/osregistry.h"
#include "debugconsole/dbugfile.h"
#include "graphics/grd3dbmpman.h"
#include "ddsutils/ddsutils.h"


extern int Cmdline_mipmap;


static tcache_slot_d3d *Textures = NULL;

int D3D_frame_count = 0;
int D3D_min_texture_width = 0;
int D3D_max_texture_width = 0;
int D3D_min_texture_height = 0;
int D3D_max_texture_height = 0;
int D3D_square_textures = 0;
int D3D_pow2_textures = 0;
int D3D_textures_in = 0;
int D3D_textures_in_frame = 0;
int D3D_last_bitmap_id = -1;
int D3D_last_detail = -1;
int D3D_last_bitmap_type = -1;

/**
 *
 * @return bool 
 */
bool d3d_free_texture( tcache_slot_d3d *t )
{
	// Bitmap changed!!	
	if ( t->bitmap_id > -1 )	{				
		// if I have been used this frame, bail		
		if(t->used_this_frame){			
			return false;
		}		

		if(t->d3d8_thandle)
		{
			DBUGFILE_DEC_COUNTER(0);
			int m = 1;

			while ( m > 0 ) {
				m = t->d3d8_thandle->Release();
			}

			t->d3d8_thandle = NULL;
		}

		if ( D3D_last_bitmap_id == t->bitmap_id )	{
			D3D_last_bitmap_id = -1;
		}
			
		t->bitmap_id = -1;
		t->used_this_frame = 0;
		D3D_textures_in -= t->size;
		t->size = 0;
	}

	return true;
}

#ifndef NDEBUG
int Show_uploads = 0;
DCF_BOOL( show_uploads, Show_uploads )
#endif

// get the final texture size (the one which will get allocated as a surface)
void d3d_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	if ( D3D_pow2_textures )	{
		int i;
		for (i=0; i<16; i++ )	{
			if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )	{
				tex_w = 1 << (i+1);
				break;
			}
		}

		for (i=0; i<16; i++ )	{
			if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )	{
				tex_h = 1 << (i+1);
				break;
			}
		}
	}

	if ( tex_w < D3D_min_texture_width ) {
		tex_w = D3D_min_texture_width;
	} else if ( tex_w > D3D_max_texture_width )	{
		tex_w = D3D_max_texture_width;
	}

	if ( tex_h < D3D_min_texture_height ) {
		tex_h = D3D_min_texture_height;
	} else if ( tex_h > D3D_max_texture_height )	{
		tex_h = D3D_max_texture_height;
	}

	if ( D3D_square_textures )	{
		int new_size;
		// Make the both be equal to larger of the two
		new_size = MAX(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}	

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

void *d3d_vimage_to_texture(int bitmap_type, int bpp, void *thandle, ushort *data, int src_w, int src_h, int *tex_w, int *tex_h, float *u_scale, float *v_scale, int reload)
{
	// Dont prepare
	bool use_mipmapping = (bitmap_type != TCACHE_TYPE_INTERFACE);

	if(Cmdline_mipmap == 0) {
		use_mipmapping = 0;
	}

	int i,j;	

	PIXELFORMAT *surface_desc = NULL;
	D3DFORMAT d3d8_format;

	switch( bitmap_type ) {
		case TCACHE_TYPE_AABITMAP:		
			surface_desc = (bpp == 32) ? NULL : &AlphaTextureFormat;
			d3d8_format  = (bpp == 32) ? D3DFMT_A8R8G8B8 : default_alpha_tformat;
			break;

//		case TCACHE_TYPE_XPARENT:
//			Int3();

		default:
			surface_desc = (bpp == 32) ? NULL : &NonAlphaTextureFormat;
			d3d8_format  = (bpp == 32) ? D3DFMT_A8R8G8B8 : default_non_alpha_tformat;
			break;
	}	

	// get final texture size
	d3d_tcache_get_adjusted_texture_size(*tex_w, *tex_h, tex_w, tex_h);

	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) ) 
	{
		*u_scale = (float)src_w / (float)*tex_w;
		*v_scale = (float)src_h / (float)*tex_h;
	} else {
		*u_scale = 1.0f;
		*v_scale = 1.0f;
	}

	IDirect3DTexture8 *texture_handle = (IDirect3DTexture8 *) thandle; 
	if(!reload) {

		DBUGFILE_INC_COUNTER(0);
		if(FAILED(GlobalD3DVars::lpD3DDevice->CreateTexture(
			*tex_w, *tex_h,
			use_mipmapping ? 0 : 1, 
			0,
			d3d8_format, 
			D3DPOOL_MANAGED, 
			&texture_handle)))
		{
			Assert(0);
			return NULL;
		}
	}

	IDirect3DSurface8 *d3d_surface = NULL; 

	texture_handle->GetSurfaceLevel(0, &d3d_surface);   

	// lock texture here
	D3DLOCKED_RECT locked_rect;
	if(FAILED(d3d_surface->LockRect(&locked_rect, NULL, 0)))
	{
		Assert(0);
		return NULL;
	}

	ubyte *bmp_data_byte = (ubyte*)data;
	
	// If 16 bit 
	if( surface_desc)
	{
		ushort *bmp_data = (ushort *)data;
		ushort *lpSP;	

		int pitch = locked_rect.Pitch / sizeof(ushort); 
		ushort *dest_data_start = (ushort *) locked_rect.pBits;

		ushort xlat[256];
		int r, g, b, a;

		switch( bitmap_type ) {		
			case TCACHE_TYPE_AABITMAP:			
				// setup convenient translation table
				for (i=0; i<16; i++ ) {
					r = 255;
					g = 255;
					b = 255;
					a = (i*255)/15;
					r /= Gr_ta_red.scale;
					g /= Gr_ta_green.scale;
					b /= Gr_ta_blue.scale;
					a /= Gr_ta_alpha.scale;
					xlat[i] = unsigned short(((a<<Gr_ta_alpha.shift) | (r << Gr_ta_red.shift) | (g << Gr_ta_green.shift) | (b << Gr_ta_blue.shift)));
				}			
				
				xlat[15] = xlat[1];			
				for ( ; i<256; i++ ) {
					xlat[i] = xlat[0];						
				}			
				
				for (j = 0; j < *tex_h; j++) {				
				  	lpSP = dest_data_start + pitch * j;
		
					for (i = 0; i < *tex_w; i++) {
						if ( (i < src_w) && (j<src_h) ) {						
							*lpSP++ = xlat[(ubyte)bmp_data_byte[j*src_w+i]];
						} else {
							*lpSP++ = 0;
						}
					}
				}
			break;

		case TCACHE_TYPE_INTERFACE:  
			for (j = 0; j < src_h; j++) {
				// the proper line in the temp ram
  				lpSP = dest_data_start + (pitch * j);
		
				// nice and clean
				for (i = 0; i < src_w; i++) {												
					// stuff the texture into vram
				  	*lpSP++ = bmp_data[(j * src_w) + i];
				}			
			}
			break;

		// Stretches bitmaps to 2 power of n format
		default: {	// normal:		
				fix u, utmp, v, du, dv;
		
				u = v = 0;
		
				du = ( (src_w-1)*F1_0 ) / *tex_w;
				dv = ( (src_h-1)*F1_0 ) / *tex_h;
												
				for (j = 0; j < *tex_h; j++) {
					lpSP = dest_data_start + pitch * j;
		
					utmp = u;				
					
					for (i = 0; i < *tex_w; i++) {
				 		*lpSP++ = bmp_data[f2i(v)*src_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
			break;
		}
	}
	// Its 32 bit
	else
	{
		D3DCOLOR *bmp_data = (D3DCOLOR *)data;
		D3DCOLOR *lpSP;	

		int pitch = locked_rect.Pitch / sizeof(D3DCOLOR); 
		D3DCOLOR *dest_data_start = (D3DCOLOR *) locked_rect.pBits;

		for (j = 0; j < *tex_h; j++)  
			for (i = 0; i < *tex_w; i++) 
				  	((D3DCOLOR*)locked_rect.pBits)[(pitch * j) + i]	= 0xff0000ff;

		switch( bitmap_type ) {

			case TCACHE_TYPE_AABITMAP: Assert(0); break; 

			case TCACHE_TYPE_INTERFACE: 
			{
				for (j = 0; j < src_h; j++) {
					// the proper line in the temp ram
  					lpSP = dest_data_start + (pitch * j);
				
					// nice and clean
					for (i = 0; i < src_w; i++) {												
						// stuff the texture into vram
					  	*lpSP++ = bmp_data[(j * src_w) + i];
					}			
				}
				break; 
			}

			default: {
				fix u, utmp, v, du, dv;
		
				u = v = 0;
		
				du = ( (src_w-1)*F1_0 ) / *tex_w;
				dv = ( (src_h-1)*F1_0 ) / *tex_h;
												
				for (j = 0; j < *tex_h; j++) {
					lpSP = dest_data_start + pitch * j;
		
					utmp = u;				
					
					for (i = 0; i < *tex_w; i++) {
				 		*lpSP++ = bmp_data[f2i(v)*src_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}	
				break;
			}
		}
	}

	// Unlock the texture 
	if(FAILED(d3d_surface->UnlockRect()))
	{
		Assert(0);
		return NULL;
	}

	if( use_mipmapping ) {
		D3DXFilterTexture( texture_handle, NULL, 0, D3DX_DEFAULT);
	}

	return texture_handle;
}

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int d3d_create_texture_sub(int bitmap_type, int texture_handle, int bpp, ushort *data, int src_w, int src_h, int tex_w, int tex_h, tcache_slot_d3d *t, int reload, int fail_on_full)
{
	Assert(*data != 0xdeadbeef);
	if(t == NULL)
	{
		return 0;
	}

	if(!reload) {
		d3d_free_texture(t);
	}

#if 0
	if ( reload )	{
  		mprintf( ("Reloading '%s'\n", bm_get_filename(texture_handle)) );
	} else {
		mprintf( ("Uploading '%s'\n", bm_get_filename(texture_handle)) );
	}
#endif

	t->d3d8_thandle = (IDirect3DTexture8 *) d3d_vimage_to_texture(
		bitmap_type, bpp, t->d3d8_thandle, data, 
		src_w, src_h, 
		&tex_w, &tex_h,
		&t->u_scale, &t->v_scale,
		reload);
	
	if(t->d3d8_thandle == NULL)
	{
		return 0;
	}

	t->bitmap_id = texture_handle;
	t->time_created = D3D_frame_count;
	t->used_this_frame = 0;	
	t->size = tex_w * tex_h * bpp / 8;	
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	D3D_textures_in_frame += t->size;
	if (!reload) {
		D3D_textures_in += t->size;
	}

	return 1;
}

int d3d_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0;

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
			break;
		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;				
			break;
		case TCACHE_TYPE_NONDARKENING:		
			Int3();
			flags |= BMP_TEX_NONDARK;
			break;
		case TCACHE_TYPE_COMPRESSED:
			switch (bm_is_compressed(bitmap_handle)) {
				case DDS_DXT1:				//dxt1
					bpp = 24;
					flags |= BMP_TEX_DXT1;
					break;
				case DDS_DXT3:				//dxt3
					bpp = 32;
					flags |= BMP_TEX_DXT3;
					break;
				case DDS_DXT5:				//dxt5
					bpp = 32;
					flags |= BMP_TEX_DXT5;
					break;
				default:
					Assert( 0 );
					break;
			}
			break;
	}
	
	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h; 

	// if we ended up locking a texture that wasn't originally compressed then this should catch it
	if ( bm_is_compressed(bitmap_handle) ) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	if ( (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_INTERFACE) && (bitmap_type != TCACHE_TYPE_COMPRESSED) )	{
		// Detail.debris_culling goes from 0 to 4.
		max_w /= 16 >> Detail.hardware_textures;
		max_h /= 16 >> Detail.hardware_textures;
	}

	// get final texture size as it will be allocated as a DD surface
	d3d_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if(tslot->bitmap_id != bitmap_handle) {
		reload = (final_w == tslot->w) && (final_h == tslot->h);
	}

 	int ret_val = d3d_create_texture_sub(bitmap_type, bitmap_handle, bmp->bpp, (ushort*)bmp->data, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

bool d3d_preload_texture_func(int bitmap_id)
{
	return 1;
}

bool d3d_lock_and_set_internal_texture(int stage, int handle, ubyte bpp, int bitmap_type, float *u_scale, float *v_scale );

int d3d_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int sx, int sy, int force, int stage )
{
	//bitmap *bmp = NULL;

	if ( bitmap_id < 0 )	{
		D3D_last_bitmap_id  = -1;
		return 0;
	}

	if(d3d_lock_and_set_internal_texture(stage, bitmap_id, (ubyte) 16, bitmap_type, u_scale, v_scale) == true) 
	{
	 	D3D_last_bitmap_id = -1;
	 	return 1;
	}

	int n = bm_get_cache_slot( bitmap_id, 1 );

	if ( D3D_last_detail != Detail.hardware_textures )	{
		D3D_last_detail = Detail.hardware_textures;
		d3d_tcache_flush();
	}
	
	tcache_slot_d3d * t = &Textures[n];	
	
	if(!bm_is_render_target(bitmap_id)){
		// If rendering exactly the same texture section as before
		if ( (D3D_last_bitmap_id == bitmap_id) && (D3D_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id))	{
			t->used_this_frame++;
		
			*u_scale = t->u_scale;
			*v_scale = t->v_scale;
			return 1;
		}	

		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id))
		{	
			if(d3d_create_texture( bitmap_id, bitmap_type, t, fail_on_full ) == 0) 
			{
	 			d3d_free_texture(t);
	 			return 0;
			}
		}
		
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		d3d_SetTexture(stage, t->d3d8_thandle);
	}else{
		d3d_SetTexture(stage, get_render_target_texture(bitmap_id));
	}
	
	D3D_last_bitmap_id = t->bitmap_id;
	D3D_last_bitmap_type = bitmap_type;

	t->used_this_frame++;		
	return 1;
}

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int sx, int sy, int force, int stage)
{
	return d3d_tcache_set_internal(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, 0, 0, force, stage );
}

void d3d_tcache_init()
{
	int i; 	
  	D3D_min_texture_width  = 16;
	D3D_min_texture_height = 16;
	D3D_max_texture_width  = GlobalD3DVars::d3d_caps.MaxTextureWidth;
	D3D_max_texture_height = GlobalD3DVars::d3d_caps.MaxTextureHeight;

	D3D_square_textures = (GlobalD3DVars::d3d_caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) ? 1 : 0; 
	D3D_pow2_textures   = (GlobalD3DVars::d3d_caps.TextureCaps & D3DPTEXTURECAPS_POW2)       ? 1 : 0; 


	// RT I dont think wide surfaces are supported in D3D8 so we need this code
	if(0)
	{
		if ( D3D_max_texture_width > gr_screen.max_w ) {
			D3D_max_texture_width = gr_screen.max_w;

			if ( D3D_pow2_textures ) {	
				for (i=0; i<16; i++ ) {	
					if ( (D3D_max_texture_width>= (1<<i)) && (D3D_max_texture_width < (1<<(i+1))) )	{
						D3D_max_texture_width = 1 << i;
						break;
					}
				}
			}

			if ( D3D_max_texture_height > D3D_max_texture_width ) {
				D3D_max_texture_height = D3D_max_texture_width;
			}

			mprintf(( "Doesn't support wide surfaces. Bashing max down to %d\n", D3D_max_texture_width ));
		}
	}

	{
		if(	!os_config_read_uint( NULL, NOX("D3DUseLargeTextures"), 0 )) {
			if ( D3D_max_texture_width > 1024 )	{
				D3D_max_texture_width = 1024;
			}

			if ( D3D_max_texture_height > 1024 )	{
				D3D_max_texture_height = 1024;
			}	
		} 
	}

	DBUGFILE_OUTPUT_2("Max textures: %d %d",D3D_max_texture_width,D3D_max_texture_height);
	
	Textures = (tcache_slot_d3d *) vm_malloc(MAX_BITMAPS * sizeof(tcache_slot_d3d));

	if ( !Textures ) {
		DBUGFILE_OUTPUT_0("exit");
		exit(1);
	}

	memset( Textures, 0, MAX_BITMAPS * sizeof(tcache_slot_d3d) );

	// Init the texture structures
	for( i=0; i<MAX_BITMAPS; i++ )	{
	//	Textures[i].d3d8_thandle = NULL;

		Textures[i].bitmap_id = -1;
	//	Textures[i].size = 0;
	//	Textures[i].used_this_frame = 0; 

	//	Textures[i].parent = NULL;
	}

	D3D_last_detail = Detail.hardware_textures;
	D3D_last_bitmap_id = -1;
	D3D_last_bitmap_type = -1;

	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;
}

void d3d_tcache_flush()
{
	int i; 

	for( i=0; i<MAX_BITMAPS; i++ )	{
		d3d_free_texture( &Textures[i] );		
	}
	if ( D3D_textures_in != 0 )	{
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", D3D_textures_in ));
		D3D_textures_in = 0;
	}

	D3D_last_bitmap_id = -1;
}

void d3d_tcache_cleanup()
{
	d3d_tcache_flush();
	
	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

	if ( Textures )	{
		vm_free(Textures);
		Textures = NULL;
	}
}

void d3d_tcache_frame()
{
	D3D_last_bitmap_id = -1;
	D3D_textures_in_frame = 0;

	D3D_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].used_this_frame = 0; 
	}
}


void gr_d3d_preload_init()
{
	d3d_tcache_flush();
}

int gr_d3d_preload(int bitmap_num, int is_aabitmap)
{
	float u_scale, v_scale;
	int retval;
	
	if ( is_aabitmap )	{
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale, 1 );
	} else {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 1 );
	}

	if ( !retval )	{
		mprintf(("Texture upload failed!\n" ));
	}

	return retval;
}

int d3d_get_valid_texture_size(int value, bool width)
{
	int min = width ? D3D_min_texture_width : D3D_min_texture_height;

	if(value < min)
		return min;

	int max = width ? D3D_max_texture_width : D3D_max_texture_height;

	for(int v = min; v <= max; v <<= 1) {
		if(value <= v)
		{
			return v;
		}
	}

	// value is too big
	return -1;
}

//added this to fix AA lines, but then found I didn't need to, 
//leaving it as it will be very useful later, and a pain to remove
void gr_d3d_set_texture_addressing(int address){
	if(address == TMAP_ADDRESS_WRAP){
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP  );
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP  );
	}else if(address == TMAP_ADDRESS_MIRROR){
 	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR  );
 	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR  );
	}else if(address == TMAP_ADDRESS_CLAMP){
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP  );
	d3d_SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP  );
	}
}


void d3d_set_texture_panning(float u, float v, bool enable){

	if(enable){
		d3d_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		D3DXMATRIX world(
			1, 0, 0, 0,
			0, 1, 0, 0,
			u, v, 0, 0,
			0, 0, 0, 1);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE0, &world);
	}else{
		d3d_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
}
