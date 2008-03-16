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
 * $Revision: 2.101 $
 * $Date: 2007-01-11 18:46:35 $
 * $Author: bobboau $
 *
 * Code for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.100  2007/01/11 07:07:46  bobboau
 * makeing D3D compatable with 32 bit index buffers, and fixing a minor directx
 * technicality in the texture compression code.
 *
 * Revision 2.99  2006/12/28 00:59:26  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.98  2006/11/06 06:33:48  taylor
 * more cleanup of warp_global crap
 * scale render/detail box limits with detail level setting
 * make sure that we reset culling and zbuffer after each model buffer that gets rendered
 *
 * Revision 2.97  2006/11/06 05:42:44  taylor
 * various bits of cleanup (slight reformatting to help readability, remove old/dead code bits, etc.)
 * deal with a index_buffer memory leak that Valgrind has always complained about
 * make HTL model buffers dynamic (get rid of MAX_BUFFERS_PER_SUBMODEL)
 * get rid of MAX_BUFFERS
 * make D3D vertex buffers dynamic, like OGL has already done
 *
 * Revision 2.96  2006/09/11 06:38:32  taylor
 * crap.  ... again
 *
 * Revision 2.95  2006/05/27 17:07:48  taylor
 * remove grd3dparticle.* and grd3dbatch.*, they are obsolete
 * allow us to build without D3D support under Windows (just define NO_DIRECT3D)
 * clean up TMAP flags
 * fix a couple of minor OpenGL state change issues with spec and env map rendering
 * make sure we build again for OS X (OGL extension functions work a little different there)
 * render targets always need to be power-of-2 to avoid incomplete buffer issues in the code
 * when we disable culling in opengl_3dunlit be sure that we re-enable it on exit of function
 * re-fix screenshots
 * add true alpha blending support (with cmdline for now since the artwork has the catch up)
 * draw lines with float positioning, to be more accurate with resizing on non-standard resolutions
 * don't load cubemaps from file for D3D, not sure how to do it anyway
 * update geometry batcher code, memory fixes, dynamic stuff, basic fixage, etc.
 *
 * Revision 2.94  2006/02/25 21:46:59  Goober5000
 * spelling
 *
 * Revision 2.93  2006/01/30 06:40:49  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 * some basic lighting code cleanup
 *
 * Revision 2.92  2006/01/18 16:14:04  taylor
 * allow gr_render_buffer() to take TMAP flags
 * let gr_render_buffer() render untextured polys (OGL only until some D3D people fix it on their side)
 * add MR_SHOW_OUTLINE_HTL flag so we easily render using HTL mode for wireframe views
 * make Interp_verts/Interp_norms/etc. dynamic and get rid of the extra htl_* versions
 *
 * Revision 2.91  2005/12/08 15:08:39  taylor
 * signed->unsigned compiler warning fix
 *
 * Revision 2.90  2005/12/06 17:53:25  taylor
 * 15 individual commits, ya had know I was going to miss something :)
 *
 * Revision 2.89  2005/12/06 02:53:02  taylor
 * clean up some D3D debug messages to better match new OGL messages (for easier debugging)
 * remove D3D_32bit variable since it's basically useless and the same thing can be done another way
 *
 * Revision 2.88  2005/10/16 11:20:43  taylor
 * use unsigned index buffers
 *
 * Revision 2.87  2005/08/23 17:06:28  matt
 * Changed rect_size_y - 1 to rect_size_y. Not really that important, but much more correct. I'll stop barraging cvs now :)
 *
 * Revision 2.86  2005/08/23 16:57:28  matt
 * Fixed another screen save bug relating to having your window positioned offscreen. Still isn't great for when the window is offscreen top left
 *
 * Revision 2.85  2005/08/23 15:59:51  matt
 * Fixed screen save crash when a popup is called
 *
 * Revision 2.84  2005/06/19 02:31:50  taylor
 * allow screenshots and backsaves in windowed mode
 * account for D3D_textures_in size so that it doesn't hit negative values
 *
 * Revision 2.83  2005/06/03 16:42:30  matt
 * D3D gamma now no longer works backwards --Sticks
 *
 * Revision 2.82  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.81  2005/04/05 05:53:16  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.80  2005/03/07 13:10:20  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.79  2005/03/01 06:55:40  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.78  2005/02/18 09:51:06  wmcoolmon
 * Updates for better nonstandard res support, as well as a fix to the Perseus crash bug I've been experiencing. Bobb, you might want to take a look at my change to grd3d.cpp
 *
 * Revision 2.77  2005/02/18 08:05:16  wmcoolmon
 * If a vertex buffer fails to be made, don't crash.
 *
 * Revision 2.76  2005/02/15 00:06:27  taylor
 * clean up some model related globals
 * code to disable individual thruster glows
 * fix issue where 1 extra OGL light pass didn't render
 *
 * Revision 2.75  2005/02/10 04:01:42  wmcoolmon
 * Low-level code for better hi-res support; better error reporting for vertex errors on model load.
 *
 * Revision 2.74  2005/01/30 03:24:39  wmcoolmon
 * Don't try and create a vertex buffer with no vertices (Seems to cause CTD) and fix to brackets in nonstandard res
 *
 * Revision 2.73  2004/12/20 20:28:15  fryday
 * Fixed a tiny error in bob's code in d3d_init_environment (which seems to be called even
 * if there's no -env command-line) which created the cube-map and didn't check
 * if the creation of it failed. Added the same sanity check to d3d_render_to_env
 *
 * Revision 2.72  2004/10/31 21:38:25  taylor
 * reinit device if lost - one of the D3D people should make sure this isn't stupid
 *
 * Revision 2.71  2004/09/26 16:24:51  taylor
 * handle lost devices better, fix movie crash
 *
 * Revision 2.70  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.69  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.68  2004/07/11 03:22:48  bobboau
 * added the working decal code
 *
 * Revision 2.67  2004/07/05 05:09:19  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.66  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.65  2004/05/25 00:37:26  wmcoolmon
 * Updated function calls for VC7 use
 *
 * Revision 2.64  2004/05/03 08:41:24  randomtiger
 * Fixed D3D fogging on R7000
 *
 * Revision 2.63  2004/04/11 13:56:33  randomtiger
 * Adding batching functions here and there and into gr_screen for use with OGL when its ready.
 *
 * Revision 2.62  2004/04/03 06:22:32  Goober5000
 * fixed some stub functions and a bunch of compile warnings
 * --Goober5000
 *
 * Revision 2.61  2004/03/20 14:47:13  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.60  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.59  2004/02/28 14:14:56  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.58  2004/02/27 04:09:55  bobboau
 * fixed a Z buffer error in HTL submodel rendering,
 * and glow points,
 * and other stuff
 *
 * Revision 2.57  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.56  2004/02/20 04:29:54  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.55  2004/02/16 21:53:40  bobboau
 * fixing lighting bug I accedently introduced
 *
 * Revision 2.54  2004/02/16 11:47:33  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.53  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.52  2004/02/15 03:04:25  bobboau
 * fixed bug involving 3d shockwaves, note I wasn't able to compile the directshow file, so I ifdefed everything to an older version,
 * you shouldn't see anything diferent, as the ifdef should be set to the way it should be, if it isn't you will get a warning mesage during compile telling you how to fix it
 *
 * Revision 2.51  2004/02/14 00:18:31  randomtiger
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
 * Revision 2.50  2004/01/24 14:31:27  randomtiger
 * Added the D3D particle code, its not bugfree but works perfectly on my card and helps with the framerate.
 * Its optional and off by default, use -d3d_particle to activiate.
 * Also bumped up D3D ambient light setting, it was way too dark.
 * Its now set to something similar to the original game.
 *
 * Revision 2.49  2004/01/20 22:37:44  Goober5000
 * First of all, this should be more readable, second of all, old_fog_color wasn't
 * initialized and VC++ is complaining.  Someone want to fix this?
 * --Goober5000
 *
 * Revision 2.48  2003/12/17 23:25:10  phreak
 * added a MAX_BUFFERS_PER_SUBMODEL define so it can be easily changed if we ever want to change the 16 texture limit
 *
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

#ifndef NO_DIRECT3D

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
#include "cmdline/cmdline.h"

#include <vector>


enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};

// Structures and enums
struct Vertex_buffer {
	Vertex_buffer() { memset(this, 0, sizeof(Vertex_buffer)); };
	int n_prim;
	int n_verts;
	vertex_buffer_type type;
	uint FVF;
	int size;
	IDirect3DVertexBuffer8 *buffer;
};

enum stage_state{
	NONE = -1, 
	INITIAL = 0, 
	DEFUSE = 1, 
	GLOW_MAPPED_DEFUSE = 2, 
	NONMAPPED_SPECULAR = 3, 
	GLOWMAPPED_NONMAPPED_SPECULAR = 4, 
	MAPPED_SPECULAR = 5, CELL = 6, 
	GLOWMAPPED_CELL = 7, 
	ADDITIVE_GLOWMAPPING = 8, 
	SINGLE_PASS_SPECMAPPING = 9, 
	SINGLE_PASS_GLOW_SPEC_MAPPING = 10,
	BACKGROUND_FOG = 11,
	ENV = 12
};

//LPDIRECT3DCUBETEXTURE8 cube_map;


// External variables - booo!
extern bool env_enabled;
extern matrix View_matrix;
extern vec3d View_position;
extern matrix Eye_matrix;
extern vec3d Eye_position;
extern vec3d Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;

extern int G3_user_clip;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

static int D3d_dump_frames = 0;
static ubyte *D3d_dump_buffer = NULL;
static int D3d_dump_frame_number = 0;
static int D3d_dump_frame_count = 0;
static int D3d_dump_frame_count_max = 0;
static int D3d_dump_frame_size = 0;

// Variables
stage_state current_render_state = NONE;

int In_frame = 0;

IDirect3DSurface8 *Gr_saved_surface = NULL;
std::vector<Vertex_buffer> D3D_vertex_buffers;
static int D3D_vertex_buffers_in_use = 0;
extern int n_active_lights;

D3DXPLANE d3d_user_clip_plane;

IDirect3DSurface8 *old_render_target = NULL;
//IDirect3DTexture8 *background_render_target = NULL;

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

	if(GlobalD3DVars::D3D_activate)TIMERBAR_END_FRAME();
	if(FAILED(GlobalD3DVars::lpD3DDevice->EndScene()))
	{
		return;
	}
	TIMERBAR_START_FRAME();
						 
	TIMERBAR_PUSH(1);
	// Must cope with device being lost
	if(GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL) == D3DERR_DEVICELOST)
	{
		d3d_lost_device();
	}
	TIMERBAR_POP();
}



// This function calls these render state one when the device is initialised and when the device is lost.
void d3d_set_initial_render_state(bool set)
{
	if(current_render_state == INITIAL)return;

	if(current_render_state == NONE){//this only needs to be done the first time-Bobboau
		d3d_SetRenderState(D3DRS_DITHERENABLE, TRUE );
		d3d_SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		d3d_SetRenderState(D3DRS_SPECULARENABLE, FALSE ); 

		// Turn lighting off here, its on by default!
		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	}


	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE), set, set;
	d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTexture(1, NULL);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(initial_state_block);
	current_render_state = INITIAL;
}

//BACKGROUND_FOG
void set_stage_for_background_fog(bool set){
	if(current_render_state == BACKGROUND_FOG)return;
/*
	if(!set){
		//d3d_proj_fov
		//d3d_proj_ratio;
		float fov = (1.0/d3d_proj_fov) * 0.65; // I don't know whay this needs to be 0.65 ???
//		int timestamp();
//		float distort = sin(float(timestamp()) / 1000.0);
		float distort = 0.0;

		D3DXMATRIX world;
		D3DXMATRIX cloak(
			fov,		0,						0,		0,
			0,			-fov * d3d_proj_ratio,	0,		0,
			0.5,		0.5,					1,		0,
			0,			0,						0,		1);
		D3DXMATRIX corection(
			1,	0,	0,	0,
			0,	1,	0,	0,
			0,	0,	1,	0,
			0,	0,	-1,	1);
		D3DXMatrixMultiply(&world, &corection, &cloak);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE0, &world);
		
	}

	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTexture(0, background_render_target);
*/	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(background_fog_state_block);
	current_render_state = BACKGROUND_FOG;
}

extern bool env_enabled;

void set_stage_for_cell_shaded(bool set){
	if(current_render_state == CELL)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT , set, set);
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT , set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(cell_state_block);
	current_render_state = CELL;
		
}

void set_stage_for_cell_glowmapped_shaded(bool set){
	if(current_render_state == GLOWMAPPED_CELL)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT, set, set );
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT, set, set );

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_cell_state_block);
	current_render_state = GLOWMAPPED_CELL;
		
}

void set_stage_for_additive_glowmapped(bool set){
	if(current_render_state == ADDITIVE_GLOWMAPPING)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(additive_glow_mapping_state_block);
	current_render_state = ADDITIVE_GLOWMAPPING;
}

void set_stage_for_defuse(bool set){
	if(current_render_state == DEFUSE)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(defuse_state_block);
	current_render_state = DEFUSE;
		
}

void set_stage_for_glow_mapped_defuse(bool set){
	if(current_render_state == GLOW_MAPPED_DEFUSE)return;
	if(!set && GLOWMAP < 0){
		set_stage_for_defuse();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

		d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);
//	d3d_SetTexture(1, GLOWMAP);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_defuse_state_block);
	current_render_state = GLOW_MAPPED_DEFUSE;
}

void set_stage_for_defuse_and_non_mapped_spec(bool set){
	if(current_render_state == NONMAPPED_SPECULAR)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(nonmapped_specular_state_block);
	current_render_state = NONMAPPED_SPECULAR;
}

void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(bool set){
	if(current_render_state == GLOWMAPPED_NONMAPPED_SPECULAR)return;
	if(!set && GLOWMAP < 0){
		set_stage_for_defuse_and_non_mapped_spec();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);
//	d3d_SetTexture(1, GLOWMAP);

	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_nonmapped_specular_state_block);
	current_render_state = GLOWMAPPED_NONMAPPED_SPECULAR;
}

bool set_stage_for_spec_mapped(bool set){
	if(current_render_state == MAPPED_SPECULAR)return true;
	if(!set && SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);


	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X, set, set);
//	d3d_SetTexture(0, SPECMAP);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);


	current_render_state = MAPPED_SPECULAR;
	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(mapped_specular_state_block);
	return true;
}

extern bool texture_has_alpha(int t_idx);

bool set_stage_for_env_mapped(bool set){
	if(current_render_state == ENV)return true;
	if(!set && SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}

//	D3DXMATRIX world;

	if(!set){
		D3DXMATRIX world(
			Eye_matrix.vec.rvec.xyz.x, Eye_matrix.vec.rvec.xyz.y, Eye_matrix.vec.rvec.xyz.z, 0,
			Eye_matrix.vec.uvec.xyz.x, Eye_matrix.vec.uvec.xyz.y, Eye_matrix.vec.uvec.xyz.z, 0,
			Eye_matrix.vec.fvec.xyz.x, Eye_matrix.vec.fvec.xyz.y, Eye_matrix.vec.fvec.xyz.z, 0,
			0, 0, 0, 1);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE1, &world);
	}

	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);


//	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_SPECULAR);
//	if(texture_has_alpha(SPECMAP))
	if(Cmdline_alpha_env)
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE, set, set);
	else
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);
//	d3d_SetTexture(0, SPECMAP);

//	if(!set)d3d_SetTexture(1, cube_map);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);
//	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_PREMODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
//D3DTOP_BLENDTEXTUREALPHA

	current_render_state = ENV;

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(env_state_block);
	return true;
}


//glow texture stage 3
void set_stage_for_single_pass_glow_specmapping(int SAME){
	//D3DPMISCCAPS_TSSARGTEMP
	static int same = -1;
//	if((current_render_state == SINGLE_PASS_GLOW_SPEC_MAPPING) && (same == SAME))return;
	if(SPECMAP < 0)return;
	if(GLOWMAP < 0){
	//	set_stage_for_single_pass_specmapping(SAME);
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
 		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);

		gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 5);

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

	state = INITIAL;*/
}

extern bool d3d_init_device();
void gr_d3d_flip()
{
	if(!GlobalD3DVars::D3D_activate) return;
	int mx, my;

	// Attempt to allow D3D8 to recover from task switching
	// this can't work if it's NULL, check reversed - taylor
	if ( GlobalD3DVars::lpD3DDevice == NULL ) {
		// we really lost the device, try a reinit
		if ( !d3d_init_device() )
			Assert( 0 );
	}

	// if the device is not going to be available then just loop around until we get it back
	// Returns:
	//   TRUE  = the device is lost and cannot be recovered yet
	//   FALSE = the device is fine or has been successfully reset
	if ( d3d_lost_device() )
		return;

	gr_reset_clip();	

	mouse_eval_deltas();

	if ( mouse_is_visible() )	{				
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		
		if ( Gr_cursor != -1 )	{
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my, false);
		}
		//was crashing for some reason
/*		else
		{
			//WMC - Backup cheapo cursor
			gr_set_color(0, 255, 0);
			gr_line(mx, my, mx+8, my+8);
			gr_line(mx, my, mx, my+16);
			gr_line(mx, my+16, mx+8, my+8);
		}
*/	} 	

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

	int rect_size_x;
	int rect_size_y;

	if(!GlobalD3DVars::D3D_activate) return -1;
	gr_reset_clip();

	if (GlobalD3DVars::lpD3D == NULL)
		return -1;

	if ( Gr_saved_surface )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	IDirect3DSurface8 *front_buffer_a8r8g8b8 = NULL;
	D3DDISPLAYMODE mode;
	mode.Width = mode.Height = 0;

	// although this doesn't really matter in fullscreen mode it doesn't hurt either
	if (FAILED(GlobalD3DVars::lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
	{
		DBUGFILE_OUTPUT_0("Could not get adapter display mode");
		return -1;
	}

	// we get the full mode size which for windowed mode is the entire screen and not just the window
	// Problem that we can only get front buffer in A8R8G8B8
	mprintf(("Creating surface for front buffer of size: %d %d", mode.Width, mode.Height));
	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		mode.Width, mode.Height, D3DFMT_A8R8G8B8, &front_buffer_a8r8g8b8))) {

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

	RECT rct;

	if (GlobalD3DVars::D3D_window) {
		POINT pnt;
		pnt.x = pnt.y = 0;

		HWND wnd = (HWND)os_get_window();
		ClientToScreen(wnd, &pnt);

		rct.left = pnt.x;
		rct.top = pnt.y;

		if(pnt.x < 0) {
			rct.left = 0;
		}

		if(pnt.y < 0) {
			rct.top = 0;
		}

		//We can't write to anything larger than the desktop resolution, so check against it
		if((UINT)(pnt.x + gr_screen.max_w) > mode.Width) {
			rct.right = mode.Width;
		}
		else {
			rct.right = pnt.x + gr_screen.max_w;
		}

		//And again
		if((UINT)(pnt.y + gr_screen.max_h) > mode.Height) {
			rct.bottom = mode.Height;
		}
		else{
			rct.bottom = pnt.y + gr_screen.max_h;
		}

	} else {
		rct.left = rct.top = 0;
		rct.right = gr_screen.max_w;
		rct.bottom = gr_screen.max_h;
	}

	//Find the total size of our rectangle
	rect_size_x = rct.right - rct.left;
	rect_size_y = rct.bottom - rct.top;


	if(FAILED(front_buffer_a8r8g8b8->LockRect(&src_rect, &rct, D3DLOCK_READONLY))) {

		DBUGFILE_OUTPUT_0("Failed to lock front buffer");
		goto Failed;

	}

	typedef struct { unsigned char b,g,r,a; } TmpC;

	if(gr_screen.bits_per_pixel == 32) {
		for(int j = 0; j < (rect_size_y); j++) {
		
			TmpC *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			uint *dst = (uint *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < (rect_size_x); i++) {
			 	dst[i] = 0;
				dst[i] |= (uint)(( (int) src[i].r / r_gun.scale ) << r_gun.shift);
				dst[i] |= (uint)(( (int) src[i].g / g_gun.scale ) << g_gun.shift);
				dst[i] |= (uint)(( (int) src[i].b / b_gun.scale ) << b_gun.shift);
			}
		}
	} else {
		for(int j = 0; j < (rect_size_y); j++) {
		
			TmpC   *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			ushort *dst = (ushort *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < (rect_size_x); i++) {
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
		D3d_dump_buffer = (ubyte *)vm_malloc(size);
		if ( !D3d_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

void gr_d3d_flush_frame_dump()
{
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
			/*
			ubyte *src_ptr = D3d_dump_buffer+(i*D3d_dump_frame_size)+(j*w*2);

			int len = tga_compress( (char *)outrow, (char *)src_ptr, w*sizeof(short) );
			*/
			int len = 0;

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
		vm_free(D3d_dump_buffer);
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

			if(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)
			{
				d3d_SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
			}
			else 
			{
				d3d_SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
				d3d_SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR);

		  		if(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
		  	  		d3d_SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
			}
		}

		gr_screen.current_fog_mode = fog_mode;	
	}	

	// is color changing?
	if( (gr_screen.current_fog_color.red != r) || (gr_screen.current_fog_color.green != g) || (gr_screen.current_fog_color.blue != b) ){
		// store the values
		gr_init_color( &gr_screen.current_fog_color, r, g, b );

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
    return static_cast<ushort>(scale*pow(i/255.f, 1.0f/recip_gamma));
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

void gr_d3d_set_fill_mode(int mode)
{
	if(mode == GR_FILL_MODE_SOLID){
		d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		return;
	}
	if(mode == GR_FILL_MODE_WIRE){
		d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		return;
	}
	//defalt value
	d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

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
int vertex_size(uint flags){
	int size = 0;
	Assert(! ((flags & VERTEX_FLAG_RHW) && (flags & VERTEX_FLAG_NORMAL)));
	if(flags & VERTEX_FLAG_UV1)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV2)Assert(! ((flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV3)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV4)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV1)));

	if(flags & VERTEX_FLAG_POSITION)size	+= sizeof(vec3d);
	if(flags & VERTEX_FLAG_RHW)size			+= sizeof(float);
	if(flags & VERTEX_FLAG_NORMAL)size		+= sizeof(vec3d);
	if(flags & VERTEX_FLAG_DIFUSE)size		+= sizeof(DWORD);
	if(flags & VERTEX_FLAG_SPECULAR)size	+= sizeof(DWORD);
	if(flags & VERTEX_FLAG_UV1)size			+= sizeof(float)*2;
	else if(flags & VERTEX_FLAG_UV2)size	+= sizeof(float)*4;
	else if(flags & VERTEX_FLAG_UV3)size	+= sizeof(float)*6;
	else if(flags & VERTEX_FLAG_UV4)size	+= sizeof(float)*8;

	return size;
}

//D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1
int convert_to_fvf(uint flags){
	int fvf = 0;
	Assert(! ((flags & VERTEX_FLAG_RHW) && (flags & VERTEX_FLAG_NORMAL)));
	if(flags & VERTEX_FLAG_UV1)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV2)Assert(! ((flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV3)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV4)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV1)));

	if(flags & VERTEX_FLAG_POSITION)	fvf |= D3DFVF_XYZ;
	if(flags & VERTEX_FLAG_RHW)			fvf |= D3DFVF_XYZRHW;
	if(flags & VERTEX_FLAG_NORMAL)		fvf |= D3DFVF_NORMAL;
	if(flags & VERTEX_FLAG_DIFUSE)		fvf |= D3DFVF_DIFFUSE;
	if(flags & VERTEX_FLAG_SPECULAR)	fvf |= D3DFVF_SPECULAR;
	if(flags & VERTEX_FLAG_UV1)			fvf |= D3DFVF_TEX1;
	else if(flags & VERTEX_FLAG_UV2)	fvf |= D3DFVF_TEX2;
	else if(flags & VERTEX_FLAG_UV3)	fvf |= D3DFVF_TEX3;
	else if(flags & VERTEX_FLAG_UV4)	fvf |= D3DFVF_TEX4;
	return fvf;
}

#define fill_v(V,v) {(*((float *) (V))) = v; V = ((byte*)(V)) + sizeof(float);}

vec3d *check_vec1, *check_vec2;
void fill_vert(void *V, vertex *L, vec3d* N, uint flags){
				if(flags & VERTEX_FLAG_RHW){
					fill_v(V, L->sx);
					fill_v(V, L->sy);
					fill_v(V, L->sw);
					fill_v(V, L->sw);
				}else
				if(flags & VERTEX_FLAG_POSITION){
					check_vec1 = (vec3d*)V;
					fill_v(V, L->x);
					fill_v(V, L->y);
					fill_v(V, L->z);
				}
				if(flags & VERTEX_FLAG_NORMAL){
					check_vec2 = (vec3d*)V;
					fill_v(V, N->xyz.x);
					fill_v(V, N->xyz.y);
					fill_v(V, N->xyz.z);
				}
				if(flags & VERTEX_FLAG_DIFUSE){
					*(byte*)(V = ((byte*)V)+1 ) = L->a;
					*(byte*)(V = ((byte*)V)+1 ) = L->r;
					*(byte*)(V = ((byte*)V)+1 ) = L->g;
					*(byte*)(V = ((byte*)V)+1 ) = L->b;
				}
				if(flags & VERTEX_FLAG_SPECULAR){
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_a;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_r;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_g;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_b;
				}
				if(flags & VERTEX_FLAG_UV1){
					fill_v(V, L->u);
					fill_v(V, L->v);
				}else
				if(flags & VERTEX_FLAG_UV2){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
				}else
				if(flags & VERTEX_FLAG_UV3){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
					fill_v(V, L->u3);
					fill_v(V, L->v3);
				}else
				if(flags & VERTEX_FLAG_UV4){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
					fill_v(V, L->u3);
					fill_v(V, L->v3);
					fill_v(V, L->u4);
					fill_v(V, L->v4);
				}
}

//makes the vertex buffer, returns an index to it
int gr_d3d_make_buffer(poly_list *list, uint flags)
{
	int k;
	byte *v;
	Vertex_buffer new_buffer;

	new_buffer.size = vertex_size(flags);
	new_buffer.FVF = convert_to_fvf(flags);

//	d3d_CreateVertexBuffer(D3DVT_VERTEX, (list->n_verts), NULL, (void**)buffer);
		
	k = GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(	
			new_buffer.size * (list->n_verts ? list->n_verts : 1), 
			D3DUSAGE_WRITEONLY, 
			new_buffer.FVF,
			D3DPOOL_MANAGED,
			&new_buffer.buffer);

	switch (k)
	{
		case D3DERR_INVALIDCALL:
			Error(LOCATION, "CreateVertexBuffer returned D3DERR_INVALIDCALL.");
			break;

		case D3DERR_OUTOFVIDEOMEMORY:
			Error(LOCATION, "CreateVertexBuffer returned D3DERR_OUTOFVIDEOMEMORY");
			break;

		case E_OUTOFMEMORY:
			Error(LOCATION, "CreateVertexBuffer returned E_OUTOFMEMORY");
			break;

		default:
			break;
	}

	if (new_buffer.buffer == NULL)
		return -1;

	new_buffer.buffer->Lock(0, 0, &v, 0);

	for(k = 0; k<list->n_verts; k++){
			fill_vert(&v[k * new_buffer.size],  &list->vert[k], &list->norm[k], flags);
			new_buffer.n_verts++;
	}

	new_buffer.buffer->Unlock();

	new_buffer.n_verts  = list->n_verts;
	new_buffer.n_prim  = list->n_verts;
	new_buffer.type = TRILIST_;

	D3D_vertex_buffers.push_back( new_buffer );
	D3D_vertex_buffers_in_use++;

	return (int)(D3D_vertex_buffers.size() - 1);
}

//makes the vertex buffer, returns an index to it
int gr_d3d_make_flat_buffer(poly_list *list)
{
	int k;
	D3DLVERTEX *v, *V;
	vertex *L;
//	vec3d *N;
	Vertex_buffer new_buffer;

	IDirect3DVertexBuffer8 **buffer = &new_buffer.buffer;

	d3d_CreateVertexBuffer(D3DVT_LVERTEX, (list->n_verts), NULL, (void**)buffer);

	new_buffer.buffer->Lock(0, 0, (BYTE **)&v, NULL);

	for (k = 0; k < list->n_verts; k++) {
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

	new_buffer.buffer->Unlock();

	new_buffer.n_prim  = list->n_verts/3;
	new_buffer.type = FLAT_;

	D3D_vertex_buffers.push_back( new_buffer );
	D3D_vertex_buffers_in_use++;

	return (int)(D3D_vertex_buffers.size() - 1);
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
//		vec3d *N;
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
void gr_d3d_destroy_buffer(int idx)
{
	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	Vertex_buffer *vbp = &D3D_vertex_buffers[idx];

	if (vbp->buffer != NULL)
		vbp->buffer->Release();

	memset( vbp, 0, sizeof(Vertex_buffer) );

	// we try to take advantage of the fact that there shouldn't be a lot of buffer
	// deletions/additions going on all of the time, so a model_unload_all() and/or
	// game_level_close() should pretty much keep everything cleared out on a
	// regular basis
	if (--D3D_vertex_buffers_in_use <= 0)
		D3D_vertex_buffers.clear();
}

//enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};

void gr_d3d_render_line_buffer(int idx)
{
	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	if (D3D_vertex_buffers[idx].buffer == NULL)
		return;

	d3d_SetVertexShader(vertex_types[D3DVT_LVERTEX].fvf);

	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, D3D_vertex_buffers[idx].buffer, sizeof(D3DLVERTEX));
	
	gr_d3d_fog_set(GR_FOGMODE_NONE, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);		//it's a HUD item, should never be fogged

	gr_d3d_set_cull(0);
	d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_LINELIST , 0, D3D_vertex_buffers[idx].n_prim);
	d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
	gr_d3d_set_cull(1);
}

extern int GR_center_alpha;
bool the_lights_are_on;
extern bool lighting_enabled;
void gr_d3d_center_alpha_int(int type);
Vertex_buffer *set_buffer;

void gr_d3d_set_buffer(int idx)
{
	set_buffer = NULL;

	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	set_buffer = &D3D_vertex_buffers[idx];

	d3d_SetVertexShader(set_buffer->FVF);
	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, set_buffer->buffer, set_buffer->size);
}

IDirect3DIndexBuffer8 *global_index_buffer = NULL;
int index_buffer_size = 0;
IDirect3DIndexBuffer8 *global_index_buffer32 = NULL;
int index_buffer_size32 = 0;


void gr_d3d_render_buffer(int start, int n_prim, ushort* index_buffer, uint *ibuf32, int flags)
{
	if(set_buffer == NULL)return;
	if(index_buffer != NULL && ibuf32 !=NULL){
		Error(LOCATION, "gr_d3d_render_buffer was given TWO indext buffers, that's not cool man!\n only useing the 16 bit one");
		ibuf32=NULL;
	}
	if(index_buffer != NULL || ibuf32 !=NULL){
		if(index_buffer){
			if(index_buffer_size < n_prim * 3 || !global_index_buffer){
				if(global_index_buffer)global_index_buffer->Release();
				GlobalD3DVars::lpD3DDevice->CreateIndexBuffer(n_prim * 3 * sizeof(ushort), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, (IDirect3DIndexBuffer8**) &global_index_buffer);
				index_buffer_size = n_prim * 3;
			}
			ushort* i_buffer;
			global_index_buffer->Lock(0, 0, (BYTE **)&i_buffer, D3DLOCK_DISCARD);
			memcpy(i_buffer, index_buffer, n_prim*3*sizeof(ushort));
			global_index_buffer->Unlock();
			GlobalD3DVars::lpD3DDevice->SetIndices(global_index_buffer, 0);
		}
		if(ibuf32) {
			if(index_buffer_size32 < n_prim * 3 || !global_index_buffer32){
				if(global_index_buffer32)global_index_buffer32->Release();
				GlobalD3DVars::lpD3DDevice->CreateIndexBuffer(n_prim * 3 * sizeof(uint), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, (IDirect3DIndexBuffer8**) &global_index_buffer32);
				index_buffer_size32 = n_prim * 3;
			}
			uint* i_buffer32;
			global_index_buffer32->Lock(0, 0, (BYTE **)&i_buffer32, D3DLOCK_DISCARD);
			memcpy(i_buffer32, ibuf32, n_prim*3*sizeof(uint));
			global_index_buffer32->Unlock();
			GlobalD3DVars::lpD3DDevice->SetIndices(global_index_buffer32, 0);
		}
	//	global_index_buffer->Lock(start, n_prim * 3 * sizeof(short), (BYTE **)&index_buffer, D3DLOCK_DISCARD);
	}
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
		d3d_SetRenderState(D3DRS_AMBIENT, ambient_light);
	}
	GlobalD3DVars::lpD3DDevice->SetMaterial(&material);

	color old_fog_color = gr_screen.current_fog_color;

	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)//when fogging don't fog unlit things, but rather fade them in a fog like manner -Bobboau
		if(!lighting_enabled){
			gr_d3d_fog_set(gr_screen.current_fog_mode, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);
		}


		if (set_buffer->buffer == NULL) {
			return;
		}
/*	if(set_buffer->type == LINELIST_) {
		gr_d3d_render_line_buffer(idx); 
		return;
	}
*/
	float u_scale = 1.0f, v_scale = 1.0f;

	gr_alpha_blend ab = ALPHA_BLEND_NONE;
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	
		ab = ALPHA_BLEND_ALPHA_ADDITIVE;

//	int same = (gr_screen.current_bitmap != SPECMAP)?0:1;
//	if(!same)d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0);
	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0);
//	if(!gr_zbuffering_mode)
//		gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_NONE);
	if(gr_zbuffering_mode == GR_ZBUFF_NONE){
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_NONE);
			d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}else if(gr_zbuffering_mode == GR_ZBUFF_READ){
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_READ);
			d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}else{
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);
	}

	pre_render_lights_init();
	if(lighting_enabled){
		shift_active_lights(0);
	}

//	bool single_pass_spec = false;

	if(GLOWMAP > -1 && !Cmdline_noglow){
		//glowmapped
			gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		 	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);
			
	 		
			set_stage_for_glow_mapped_defuse();
	}else{
		//non glowmapped
			set_stage_for_defuse();
	}

	int passes = (n_active_lights / GlobalD3DVars::d3d_caps.MaxActiveLights);
//	d3d_SetVertexShader(D3DVT_VERTEX);

	gr_d3d_center_alpha_int(GR_center_alpha);
//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
	else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);


	if(!lighting_enabled)
	{
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


	d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));
	for(int i = 1; i<passes; i++){
		shift_active_lights(i);
		TIMERBAR_PUSH(7);
		if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
		else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
		TIMERBAR_POP();
	}
	if(!lighting_enabled){
		int l = int(255.0f*gr_screen.current_alpha);
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(l,l,l,l));
	}else{
		d3d_SetRenderState(D3DRS_AMBIENT, ambient_light);
	}
								    
	pre_render_lights_init();
	shift_active_lights(0);

	//spec mapping
	if(SPECMAP > -1){
		gr_zbias(1);
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
				return;
			}

		if(set_stage_for_spec_mapped()){
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
			if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
			else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
			d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));
			for(int i = 1; i<passes; i++){
				shift_active_lights(i);
				if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
				else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
			}
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
			if(Cmdline_env){
				gr_zbias(2);

				extern int Game_subspace_effect;
				gr_screen.gf_set_bitmap(ENVMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
				d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);

				gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
				set_stage_for_env_mapped();
				d3d_SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
				if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
				else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
				d3d_SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			}
		}
		gr_zbias(0);
	}

	// Revert back to old fog state
	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)
		gr_d3d_fog_set(gr_screen.current_fog_mode, old_fog_color.red,old_fog_color.green,old_fog_color.blue, gr_screen.fog_near, gr_screen.fog_far);

}

//*******matrix stuff*******//

/*
	//the projection matrix; fov, aspect ratio, near, far
 	void (*gf_set_proj_matrix)(float, float, float, float);
  	void (*gf_end_proj_matrix)();
	//the view matrix
 	void (*gf_set_view_matrix)(vec3d *, matrix*);
  	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(vec3d *);
 	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(vec3d *, matrix*);
	void (*gf_start_angles_instance_matrix)(vec3d *, angles*);
	void (*gf_end_instance_matrix)();
*/

//fov = (4.0/9.0)*(PI)*View_zoom
//ratio = screen.aspect
//near = 1.0
//far = 30000
float fov_corection = 1.0f;
	//the projection matrix; fov, aspect ratio, near, far
void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f){
//	proj_matrix_stack->Push();
	fov *= fov_corection;
	D3DXMATRIX mat;
	D3DXMatrixPerspectiveFovLH(&mat, fov, ratio, n, f);
//	D3DXMatrixPerspectiveFovLH(&mat, (4.0f/9.0f)*(D3DX_PI)*View_zoom, 1.0f/gr_screen.aspect, 0.2f, 30000);
//	proj_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
}

void gr_d3d_end_proj_matrix(){
//	proj_matrix_stack->Pop();
}

//extern float global_scaleing_factor;
	//the view matrix
void gr_d3d_set_view_matrix(vec3d* offset, matrix *orient){

//	view_matrix_stack->Push();

	D3DXMATRIX mat, scale_m;

	D3DXMATRIX MAT(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

	D3DXMatrixIdentity(&mat);

//	D3DXMatrixScaling(&scale_m, 1/global_scaleing_factor, 1/global_scaleing_factor, 1/global_scaleing_factor);//global sacaleing
//	D3DXMatrixMultiply(&MAT, &MAT, &scale_m);

	D3DXMatrixInverse(&mat, NULL, &MAT);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, &mat);
}

void gr_d3d_end_view_matrix(){
//	view_matrix_stack->Pop();
//	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, view_matrix_stack->GetTop());
}
int matr_depth = 0;
	//object position and orientation
void gr_d3d_start_instance_matrix(vec3d* offset, matrix *orient){

	D3DXMATRIX old_world = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX world(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

//	D3DXMatrixScaling(&scale_m, global_scaleing_factor, global_scaleing_factor, global_scaleing_factor);//global sacaleing
//	D3DXMatrixMultiply(&world, &scale_m, &world);

	D3DXMatrixMultiply(&world, &world, &old_world);

	world_matrix_stack->LoadMatrix(&world);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth++;
}

void gr_d3d_start_angles_instance_matrix(vec3d* offset, angles *orient){

	D3DXMATRIX current = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat,orient->h,orient->p,orient->b);
	D3DXMATRIX world(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

///	D3DXMatrixScaling(&scale_m, global_scaleing_factor, global_scaleing_factor, global_scaleing_factor);;//global sacaleing
//	D3DXMatrixMultiply(&world, &scale_m, &world);

	D3DXMatrixMultiply(&mat, &mat, &world);
	D3DXMatrixMultiply(&mat, &mat, &current);

	world_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth++;

}

void gr_d3d_end_instance_matrix()
{
	world_matrix_stack->Pop();
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth--;

}


	//object scaleing
void gr_d3d_set_scale_matrix(vec3d* scale){

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
void gr_d3d_start_clip(){

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
void gr_d3d_end_clip(){
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

//this is an attempt to get something more like the old style fogging that used the lockable back buffer, 
//it will render the model with the background texture then fog with black as the color
void gr_d3d_setup_background_fog(bool set){
	return;
/*	IDirect3DSurface8 *bg_surf;
	if(!background_render_target){
		GlobalD3DVars::lpD3DDevice->CreateTexture(
			1024, 1024,
			0, 
			D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, 
			D3DPOOL_DEFAULT, &background_render_target);
	}
	background_render_target->GetSurfaceLevel(0,&bg_surf);
	if(set){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(bg_surf , 0);
		gr_d3d_clear();
	}else{
	//	GlobalD3DVars::lpD3DDevice->GetRenderTarget(&background_render_target);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target , 0);
	}*/
}


IDirect3DSurface8 *old_render_sten = NULL;
/*
extern D3DFORMAT d3d8_format;
LPDIRECT3DSURFACE8 face;

void d3d_init_environment(){
	if(!cube_map){
		D3DFORMAT use_format;
		use_format =  D3DFMT_X8R8G8B8;
		GlobalD3DVars::lpD3DDevice->CreateCubeTexture(512, 1, D3DUSAGE_RENDERTARGET , use_format, D3DPOOL_DEFAULT, &cube_map);

		// Check if it succeeds in allocating the cube map - it fails on a card that doesn't support
		// cubemaps, silly!
		if (NULL == cube_map) {
			mprintf(("Failed in creating a cube-map in d3d_init_environment!"));
			return;
		}
	}

	if(!old_render_target){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&old_render_sten);
	}
	
	for(int i = 0; i<6; i++){
		cube_map->GetCubeMapSurface(_D3DCUBEMAP_FACES(i), 0, &face);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(face , NULL);
		gr_d3d_clear();
		int r = face->Release();
	}
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target, old_render_sten);

//	old_render_target = NULL;

}

LPDIRECT3DSURFACE8 env_face = NULL;
void d3d_render_to_env(int FACE){
	if(env_face){
		env_face->Release();
		env_face = NULL;
	}
	if(!cube_map){
		D3DFORMAT use_format;
		use_format =  D3DFMT_X8R8G8B8;
		GlobalD3DVars::lpD3DDevice->CreateCubeTexture(512, 1, D3DUSAGE_RENDERTARGET , use_format, D3DPOOL_DEFAULT, &cube_map);

		// Check if it succeeds in allocating the cube map - it fails on a card that doesn't support
		// cubemaps, silly!
		if (NULL == cube_map) {
			mprintf(("Failed in creating a cube-map in d3d_render_to_env!"));
			return;
		}
	}

	if(!old_render_target){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&old_render_sten);
	}
	
	if(FACE > -1){
		cube_map->GetCubeMapSurface(_D3DCUBEMAP_FACES(FACE), 0, &env_face);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(env_face , NULL);
		gr_d3d_clear();
	}else{
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target, old_render_sten);
	}

//	old_render_target = NULL;

}
*/

#endif // !NO_DIRECT3D
