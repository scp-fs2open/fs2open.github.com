/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Model/ModelInterp.cpp $
 * $Revision: 2.121 $
 * $Date: 2005-06-21 00:25:34 $
 * $Author: taylor $
 *
 *	Rendering models, I think.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.120  2005/06/21 00:20:24  taylor
 * in the model _render functions change "light_ignore_id" to "objnum" since that's what it really is
 *   and this makes it so much easier to realize that
 * properly deal with the fact that objnum can be -1 in  model_really_render()
 * add NULL check to neb2_get_fog_values() so that it can just send back defaults if objp is NULL
 * small compiler warning fix for neb code
 *
 * Revision 2.119  2005/06/19 09:03:05  taylor
 * check_values() shouldn't be inline anymore
 * remove useless neb2_get_fog_intensity() call
 * s/alocate/allocate/g
 *
 * Revision 2.118  2005/06/19 02:42:21  taylor
 * really needed to mention those two things as well given the types of changes those are
 *
 * Revision 2.117  2005/06/19 02:28:55  taylor
 * add a _fast version of bm_unload() to be used in modelinterp and future graphics API code
 * clean up some modelinterp code to not use memcpy() everywhere so it's more platform compatible and matches old code (Jens Granseuer)
 * NaN check to catch shards-of-death and prevent hitting an Assert() (Jens Granseuer)
 * fix jumpnode code to catch model errors and close a memory leak
 * make the call to bm_unload_all() after model_free_all() since we will get bmpman screwups otherwise
 * don't show hardware sound RAM when using OpenAL build, it will always be 0
 * print top-right memory figures in debug builds slighly further over when 1024+ res
 * IBX checks for size and make sure we don't try to use a zero byte file
 * don't replicate base textures to specular textures since it screws up bmpman accounting
 *
 * Revision 2.116  2005/05/12 17:49:14  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.115  2005/04/21 15:49:20  taylor
 * update of bmpman and model bitmap management, well tested but things may get a bit bumpy
 *  - use VM_* macros for bmpman since it didn't seem to register the memory correctly (temporary)
 *  - a little "stupid" fix for dds bitmap reading
 *  - fix it so that memory is released properly on bitmap read errors
 *  - some cleanup to model texture loading
 *  - allow model textures to get released rather than just unloaded, saves bitmap slots
 *  - bump MAX_BITMAPS to 4750, should be able to decrease after public testing of new code
 *
 * Revision 2.114  2005/04/12 05:26:37  taylor
 * many, many compiler warning and header fixes (Jens Granseuer)
 * fix free on possible NULL in modelinterp.cpp (Jens Granseuer)
 *
 * Revision 2.113  2005/04/05 05:53:20  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.112  2005/03/25 06:57:36  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.111  2005/03/25 01:10:51  taylor
 * I think I deserve a medal or something for that one :)
 *
 * Revision 2.110  2005/03/24 23:36:13  taylor
 * fix compiler warnings with mismatched types and unused variables
 * cleanup some debug messages so they can be turned off if needed
 * get rid of extra strstr() check for thrusters since it should never get that far anyway
 * page_in/page_out of model glows should be better now
 * removed a bunch of unneeded casts and get type specific math functions right
 *
 * Revision 2.109  2005/03/20 20:01:15  phreak
 * draw paths and docking points using HTL
 *
 * Revision 2.108  2005/03/16 01:35:58  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.107  2005/03/10 08:00:10  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.106  2005/03/08 03:50:23  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.105  2005/03/07 13:10:22  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.104  2005/03/01 06:55:41  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.103  2005/02/27 23:42:07  wmcoolmon
 * Added a white component to lightning so it isn't just a big blue block
 *
 * Revision 2.102  2005/02/15 00:06:27  taylor
 * clean up some model related globals
 * code to disable individual thruster glows
 * fix issue where 1 extra OGL light pass didn't render
 *
 * Revision 2.101  2005/02/12 12:17:54  taylor
 * fix Error() check so that it doesn't hit on MAX_BUFFERS_PER_SUBMODEL-1
 *
 * Revision 2.100  2005/02/04 23:29:32  taylor
 * merge with Linux/OSX tree - p0204-3
 *
 * Revision 2.99  2005/01/30 10:49:26  Goober5000
 * bah - rolled back the HTL fix that doesn't really work
 * --Goober5000
 *
 * Revision 2.98  2005/01/30 09:27:40  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 2.97  2005/01/30 03:23:22  wmcoolmon
 * Improved user-friendliness - give an Error instead of a CTD
 *
 * Revision 2.96  2005/01/29 15:40:22  phreak
 * fixed error message again :)
 * -p
 *
 * Revision 2.95  2005/01/28 23:45:23  wmcoolmon
 * Added better too-many-textures error message
 *
 * Revision 2.94  2005/01/28 11:39:17  Goober5000
 * cleaned up some build warnings
 * --Goober5000
 *
 * Revision 2.93  2005/01/28 09:56:44  taylor
 * add model_page_out_textures() for use in techroom
 * make model_page_in_textures() load more textures
 *
 * Revision 2.92  2005/01/01 19:45:32  taylor
 * add MR_NO_FOGGING flag to easily render models without fog (warp model, targetbox models)
 *
 * Revision 2.91  2004/12/05 22:01:11  bobboau
 * sevral feature additions that WCS wanted,
 * and the foundations of a submodel animation system,
 * the calls to the animation triggering code (exept the procesing code,
 * wich shouldn't do anything without the triggering code)
 * have been commented out.
 *
 * Revision 2.90  2004/10/31 21:55:00  taylor
 * s/fisrt/first/g
 *
 * Revision 2.89  2004/10/09 17:48:58  taylor
 * da simple IBX code, back to old line rendering for models since it should be faster for OGL now
 *
 * Revision 2.88  2004/09/05 19:23:24  Goober5000
 * fixed a few warnings
 * --Goober5000
 *
 * Revision 2.87  2004/07/26 20:47:41  Kazan
 * remove MCD complete
 *
 * Revision 2.86  2004/07/12 16:32:56  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.85  2004/07/11 03:22:50  bobboau
 * added the working decal code
 *
 * Revision 2.84  2004/07/05 05:09:20  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.83  2004/07/01 01:12:32  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.82  2004/06/28 02:13:08  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.81  2004/05/12 22:50:31  phreak
 * don't fog the warp model if its being rendered
 *
 * Revision 2.80  2004/05/01 17:37:09  Goober5000
 * added an assert
 * --Goober5000
 *
 * Revision 2.79  2004/04/26 12:38:35  taylor
 * use d3_draw_rod() for lightning arcs (for OGL fix), fix debris making HUD in OGL disappear
 *
 * Revision 2.78  2004/04/03 06:22:33  Goober5000
 * fixed some stub functions and a bunch of compile warnings
 * --Goober5000
 *
 * Revision 2.77  2004/03/20 21:17:13  bobboau
 * fixed -spec comand line option,
 * probly some other stuf
 *
 * Revision 2.76  2004/03/17 04:07:30  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.75  2004/03/06 23:28:23  bobboau
 * fixed motion debris
 * animated laser textures
 * and added a new error check called a safepoint, mostly for tracking the 'Y bug'
 *
 * Revision 2.74  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.73  2004/03/05 04:16:20  phreak
 * made OpenGl use the matrix api if called by g3_start_**** or g3_done_instance
 *
 * Revision 2.72  2004/02/27 04:09:55  bobboau
 * fixed a Z buffer error in HTL submodel rendering,
 * and glow points,
 * and other stuff
 *
 * Revision 2.71  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.70  2004/02/15 06:02:32  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.69  2004/02/15 03:04:25  bobboau
 * fixed bug involving 3d shockwaves, note I wasn't able to compile the directshow file, so I ifdefed everything to an older version,
 * you shouldn't see anything diferent, as the ifdef should be set to the way it should be, if it isn't you will get a warning mesage during compile telling you how to fix it
 *
 * Revision 2.68  2004/02/14 00:18:34  randomtiger
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
 * Revision 2.67  2004/02/13 04:17:14  randomtiger
 * Turned off fog in OGL for Fred.
 * Simulated speech doesnt say tags marked by $ now.
 * The following are fixes to issues that came up testing TBP in fs2_open and fred2_open:
 * Changed vm_vec_mag and parse_tmap to fail gracefully on bad data.
 * Error now given on missing briefing icon and bad ship normal data.
 * Solved more species divide by zero error.
 * Fixed neb cube crash.
 *
 * Revision 2.66  2004/02/06 23:00:29  Goober5000
 * commented my tmap_num Asserts
 * --Goober5000
 *
 * Revision 2.65  2004/02/06 05:55:46  matt
 * Fix for bad ambient lighting in non-htl -Sticks
 *
 * Revision 2.64  2004/02/04 10:14:58  Goober5000
 * commented a decal function that someone missed
 * --Goober5000
 *
 * Revision 2.63  2004/01/30 07:39:08  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.62  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.61  2004/01/20 22:39:06  Goober5000
 * commented the variables in a commented function
 * --Goober5000
 *
 * Revision 2.60  2004/01/12 21:12:41  randomtiger
 * Added fix for fogging debris in D3D htl.
 *
 * Revision 2.59  2003/12/04 20:39:10  randomtiger
 * Added DDS image support for D3D
 * Added new command flag '-ship_choice_3d' to activate 3D models instead of ani's in ship choice, feature now off by default
 * Hopefully have fixed D3D text batching bug that caused old values to appear
 * Added Hud_target_object_factor variable to control 3D object sizes of zoom in HUD target
 * Fixed jump nodes not showing
 *
 * Revision 2.58  2003/12/03 19:27:01  randomtiger
 * Changed -t32 flag to -jpgtga
 * Added -query_flag to identify builds with speech not compiled and other problems
 * Now loads up launcher if videocard reg entry not found
 * Now offers to go online to download launcher if its not present
 * Changed target view not to use lower res texture, hi res one is already chached so might as well use it
 *
 * Revision 2.57  2003/11/29 17:13:54  randomtiger
 * Undid my node fix, it introduced a lot of bugs, update again if you have that version.
 *
 * Revision 2.56  2003/11/29 16:11:46  fryday
 * Fixed normal loading in OpenGL HT&L.
 * Fixed lighting in OpenGL HT&L, hopefully for the last time.
 * Added a test if a normal is valid during model load, if not, replaced with face normal
 *
 * Revision 2.55  2003/11/29 14:54:35  randomtiger
 * Rendering jumpnodes in htl via the old system to make them display again, appears to be so cheap it wouldnt be worth the effort to convert it to true htl.
 *
 * Revision 2.54  2003/11/25 15:04:46  fryday
 * Got lasers to work in HT&L OpenGL
 * Messed a bit with opengl_tmapper_internal3d, draw_laser functions, and added draw_laser_htl
 *
 * Revision 2.53  2003/11/17 04:25:57  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.52  2003/11/16 04:09:22  Goober5000
 * language
 *
 * Revision 2.51  2003/11/11 18:12:41  phreak
 * changed g3_done_instance() calls to take a parameter to pop the T&L matrices
 *
 * Revision 2.50  2003/11/11 03:56:12  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.49  2003/11/11 02:15:45  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.48  2003/11/07 18:31:02  randomtiger
 * Fixed a nohtl call to htl funcs (crash with NULL pointer)
 * Fixed a bug with 32bit PCX code.
 * Fixed a bug in the d3d_string batch system that was messing up screen shaking.
 * Added a couple of checks to try and stop timerbar push and pop overloads, check returns missing pops if you use the system.
 * Put in a higher res icon until we get something better sorted out.
 *
 * Revision 2.47  2003/11/06 20:22:11  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 2.46  2003/11/02 05:50:08  bobboau
 * modified trails to render with tristrips now rather than with stinky old trifans,
 * MUCH faster now, at least one order of magnatude.
 *
 * Revision 2.45  2003/11/01 21:59:22  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.44  2003/10/27 23:04:22  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.43  2003/10/26 00:31:59  randomtiger
 * Fixed hulls not drawing (with Phreaks advise).
 * Put my 32bit PCX loading under PCX_32 compile flag until its working.
 * Fixed a bug with res 640x480 I introduced with my non standard mode code.
 * Changed JPG and TGA loading command line param to "-t32"
 *
 * Revision 2.42  2003/10/25 06:56:06  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.41  2003/10/25 03:27:50  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.40  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.39  2003/10/23 13:53:36  fryday
 * Simplified model rendering, and it fixed up a serious bug on my system in which models with submodels didn't get their hull rendered
 *
 * Revision 2.38  2003/10/18 02:43:59  phreak
 * oops!  forgot to bracket something off with an if :rolleyes:
 *
 * Revision 2.37  2003/10/18 01:59:02  phreak
 * switched around positions of gr_start_instance_matrix and gr_end_instance_matrix
 *
 * Revision 2.36  2003/10/14 17:39:15  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.35  2003/10/13 05:57:48  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.34  2003/10/10 03:59:41  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.33  2003/10/04 22:42:22  Kazan
 * fs2netd now TCP
 *
 * Revision 2.32  2003/09/26 14:37:15  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.31  2003/09/14 19:02:06  wmcoolmon
 * Changed "cell" to "Cmdline_cell" -C
 *
 * Revision 2.30  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.29  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.28  2003/08/16 03:52:24  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.27  2003/08/12 03:18:33  bobboau
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
 * Revision 2.26  2003/08/06 17:24:57  phreak
 * optimized the way cloaking is handled
 *
 * Revision 2.25  2003/07/15 16:09:46  phreak
 * lighting in the render functions now work for opengl as they do in d3d
 *
 * Revision 2.24  2003/07/15 02:37:59  phreak
 * models that are fully cloaked draw front-to-back
 *
 * Revision 2.23  2003/07/04 02:28:37  phreak
 * changes for cloaking implemented
 *
 * Revision 2.22  2003/06/08 17:31:37  phreak
 * debris is now properly fogged
 *
 * Revision 2.21  2003/06/04 15:28:24  phreak
 * added some code that took advantage of opengl fogging stuff
 *
 * Revision 2.20  2003/03/18 10:07:04  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.19  2003/03/18 01:44:31  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.18  2003/03/02 05:55:51  penguin
 * Moved Gr_scaler_zbuffering definition to top of file
 *  - penguin
 *
 * Revision 2.17  2003/01/30 23:19:14  phreak
 * enabled RGB lighting for OpenGL
 *
 * Revision 2.16  2003/01/21 17:24:16  Goober5000
 * fixed a few bugs in Bobboau's implementation of the glow sexps; also added
 * help for the sexps in sexp_tree
 * --Goober5000
 *
 * Revision 2.15  2003/01/20 05:40:49  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.14  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.13  2003/01/17 01:48:49  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.12  2003/01/16 06:49:11  Goober5000
 * yay! got texture replacement to work!!!
 * --Goober5000
 *
 * Revision 2.11  2003/01/09 05:55:55  Goober5000
 * fixed the giant thruster glow bug
 * --Goober5000
 *
 * Revision 2.10  2003/01/06 19:33:22  Goober5000
 * cleaned up some stuff with model_set_thrust and a commented Assert that
 * shouldn't have been
 * --Goober5000
 *
 * Revision 2.9  2002/12/07 01:37:42  bobboau
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.8  2002/12/02 23:16:45  Goober5000
 * commented out an unneeded variable (ship *shipp) in model_really_render
 *
 * Revision 2.7  2002/11/14 06:15:03  bobboau
 * added nameplate code
 *
 * Revision 2.6  2002/11/14 04:18:16  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.5  2002/10/22 23:02:40  randomtiger
 * Made Phreaks alternative scanning style optional under the command line tag "-phreak"
 * Fixed bug that changes HUD colour when targetting debris in a full nebula. - RT
 *
 * Revision 2.4  2002/10/19 19:29:27  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.3.2.1  2002/10/30 22:57:21  randomtiger
 *
 * Changed DX8 code to not use set render and texture states if they are already set to that value.
 * Disabled buffer saving and restoring code when windowed to stop DX8 debug runs from crashing. - RT
 *
 * Revision 2.3  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/10 18:42:14  wmcoolmon
 * Added  Bobboau's glow code; all comments include "-Bobboau"
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/09 13:49:30  mharris
 * Added ifndef NO_DIRECT3D
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 37    9/13/99 11:25p Dave
 * Fixed problem with mode-switching and D3D movies.
 * 
 * 36    9/13/99 10:09a Andsager
 * Add debug console commands to lower model render detail and fireball
 * LOD for big ship explosiosns.
 * 
 * 35    9/08/99 12:03a Dave
 * Make squad logos render properly in D3D all the time. Added intel anim
 * directory.
 * 
 * 34    9/01/99 10:09a Dave
 * Pirate bob.
 * 
 * 33    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 32    8/24/99 8:55p Dave
 * Make sure nondimming pixels work properly in tech menu.
 * 
 * 31    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 30    7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 29    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 28    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 27    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 26    7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 25    7/15/99 2:13p Dave
 * Added 32 bit detection.
 * 
 * 24    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 23    6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 22    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 21    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 20    5/12/99 10:05a Johnson
 * DKA:  Fix fred bug from engine wash
 * 
 * 19    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 18    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 17    4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 16    4/20/99 3:30p Andsager
 * Let get_model_closest_box_point_with_delta() take NULL as pointer to
 * is_inside
 * 
 * 15    4/19/99 12:51p Andsager
 * Add function to find the nearest point on extneded bounding box and
 * check if inside bounding box.
 * 
 * 14    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 13    3/24/99 6:14p Dave
 * Added position and orientation checksumming for multiplayer. Fixed LOD
 * rendering bugs for squad insignias
 * 
 * 12    3/23/99 5:17p Dave
 * Changed model file format somewhat to account for LOD's on insignias
 * 
 * 11    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 10    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 9     3/02/99 9:25p Dave
 * Added a bunch of model rendering debug code. Started work on fixing
 * beam weapon wacky firing.
 * 
 * 8     2/19/99 11:42a Dave
 * Put in model rendering autocentering.
 * 
 * 7     1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 6     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 5     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 4     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 3     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 193   8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 192   6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 191   5/24/98 4:53p John
 * changed rounding for model caching to get rid of some empty lines in
 * cached bitmap.
 * 
 * 190   5/16/98 4:47p John
 * Put back in my version 188 changes that someone so rudely (but
 * hopefully unintentionally) deleted.
 * 
 * 189   5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 187   5/12/98 11:32a Mike
 * Support for weapon pof detail levels.
 * 
 * 186   5/08/98 1:32p John
 * Added code for using two layered subspace effects.
 * 
 * 185   4/29/98 11:03a John
 * Added code to show octants.
 * 
 * 184   4/22/98 9:58p John
 * Added code to view invisible faces.
 * 
 * 183   4/22/98 9:43p John
 * Added code to allow checking of invisible faces, flagged by any texture
 * name with invisible in it.
 * 
 * 182   4/20/98 4:44p John
 * Fixed problems with black being xparent on model cache rneders.  Made
 * model cache key off of detail level setting and framerate.
 * 
 * 181   4/18/98 4:00p John
 * Made highest model caching detail level disable model caching.
 * 
 * 180   4/14/98 11:11p John
 * Made ships with < 50% hull left show electrical damage arcs.
 * 
 * 179   4/13/98 4:54p John
 * Made uv rotate independently on subspace effect. Put in DCF function
 * for setting subspace speeds.
 * 
 * 178   4/12/98 5:54p John
 * Made models work with subspace.  Made subspace rotate also.
 * 
 * 177   4/12/98 9:56a John
 * Made lighting detail flags work.   Made explosions cast light on
 * highest.
 * 
 * 176   4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 175   4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 174   4/09/98 11:40p John
 * Fixed some bugs with hardware lighting.
 * 
 * 173   4/09/98 4:38p John
 * Made non-darkening and transparent textures work under Glide.  Fixed
 * bug with Jim's computer not drawing any bitmaps.
 * 
 * $NoKeywords: $
 */

#define MODEL_LIB

#include "model/model.h"
#include "model/modelsinc.h"
#include "graphics/2d.h"
#include "render/3dinternal.h"
#include "math/fvi.h"
#include "lighting/lighting.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "io/timer.h"
#include "graphics/grinternal.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "math/staticrand.h"
#include "particle/particle.h"
#include "ship/ship.h"
#include "cmdline/cmdline.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"



float model_radius = 0;

// Some debug variables used externally for displaying stats
#ifndef NDEBUG
int modelstats_num_polys = 0;
int modelstats_num_polys_drawn = 0;
int modelstats_num_verts = 0;
int modelstats_num_sortnorms = 0;
int modelstats_num_boxes = 0;
#endif

extern int Cmdline_nohtl;

int glow_maps_active = 1;


// a lighting object
typedef struct model_light_object {
	ubyte		r[MAX_POLYGON_NORMS];
	ubyte		g[MAX_POLYGON_NORMS];
	ubyte		b[MAX_POLYGON_NORMS];
	ubyte		spec_r[MAX_POLYGON_NORMS];
	ubyte		spec_g[MAX_POLYGON_NORMS];
	ubyte		spec_b[MAX_POLYGON_NORMS];
	int		objnum;
	int		skip;
	int		skip_max;
} model_light_object;

// -----------------------
// Local variables
//

// Vertices used internally to rotate model points
static vertex Interp_points[MAX_POLYGON_VECS];
static vertex Interp_splode_points[MAX_POLYGON_VECS];
vec3d *Interp_verts[MAX_POLYGON_VECS];
vec3d Interp_splode_verts[MAX_POLYGON_VECS];
static int Interp_num_verts;


// -------------------------------------------------------------------
// lighting save stuff 
//
#define MAX_MODEL_LIGHTING_SAVE			30
int hack_skip_max = 1;
DCF(skip, "")
{
	dc_get_arg(ARG_INT);
	hack_skip_max = Dc_arg_int;
}
// model_light_object Interp_lighting_save[MAX_MODEL_LIGHTING_SAVE];
model_light_object Interp_lighting_temp;
model_light_object *Interp_lighting = &Interp_lighting_temp;
int Interp_use_saved_lighting = 0;
int Interp_saved_lighting_full = 0;
//
// lighting save stuff 
// -------------------------------------------------------------------


static ubyte Interp_light_applied[MAX_POLYGON_NORMS];
static vec3d *Interp_norms[MAX_POLYGON_NORMS];
static int Interp_num_norms = 0;
static ubyte *Interp_lights;

static float Interp_fog_level = 0.0f;

// Stuff to control rendering parameters
static color Interp_outline_color;
static int Interp_detail_level = 0;
static uint Interp_flags = 0;
static uint Interp_tmap_flags = 0;

// If non-zero, then the subobject gets scaled by Interp_thrust_scale.
static int Interp_thrust_scale_subobj=0;
static float Interp_thrust_scale = 0.1f;
static int Interp_thrust_bitmap = -1;
static int Interp_thrust_glow_bitmap = -1;
static int Interp_secondary_thrust_glow_bitmap = -1;
static int Interp_tertiary_thrust_glow_bitmap = -1;
static bool Interp_AB=false;
static float Interp_thrust_glow_rad_factor = 1.0f;
static float Interp_secondary_thrust_glow_rad_factor = 1.0f;
static float Interp_secondary_thrust_glow_len_factor = 1.0f;
static float Interp_tertiary_thrust_glow_rad_factor = 1.0f;
static vec3d controle_rotval = ZERO_VECTOR;
static float Interp_thrust_glow_noise = 1.0f;
static float Interp_thrust_scale_x = 0.0f;//added -bobboau
static float Interp_thrust_scale_y = 0.0f;//added -bobboau
float Model_Interp_scale_x = 1.0f;	//added these three for warpin stuff-Bobbau
float Model_Interp_scale_y = 1.0f;
float Model_Interp_scale_z = 1.0f;
int Warp_Model = -1; //global warp model number
int Warp_Map = -1;	//global map to be used while rendering the warp model
float Warp_Alpha = -1.0f;
static int Interp_objnum = -1;

// if != -1, use this bitmap when rendering ship insignias
static int Interp_insignia_bitmap = -1;

// replacement - Goober5000
static int Interp_replacement_bitmap = -1;
static int *Interp_replacement_textures = NULL;

// if != -1, use this bitmap when rendering with a forced texture
static int Interp_forced_bitmap = -1;

// for rendering with the MR_ALL_XPARENT FLAG SET
static float Interp_xparent_alpha = 1.0f;

float Interp_light = 0.0f;

int Interp_multitex_cloakmap=-1;
int Interp_cloakmap_alpha=255;

int model_current_LOD = 0;

void set_warp_globals(float a, float b, float c, int d, float e){
	Model_Interp_scale_x =a;
	Model_Interp_scale_y=b;
	Model_Interp_scale_z=c;
	Warp_Map=d;
	Warp_Alpha=e;
//	mprintf(("warpmap being set to %d\n",Warp_Map));
}

static int FULLCLOAK=-1;

void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);

int model_should_render_engine_glow(int objnum, int bank_obj);

void model_render_buffers(bsp_info* model, polymodel * pm);
void model_render_childeren_buffers(bsp_info* model, polymodel * pm, int mn, int detail_level);


void (*model_interp_sortnorm)(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check) = model_interp_sortnorm_b2f;

void model_setup_cloak(vec3d *shift, int full_cloak, int alpha)
{
	FULLCLOAK=full_cloak;
	int unit;
	if (full_cloak)
	{
		unit=0;
		Interp_multitex_cloakmap=0;
		model_set_insignia_bitmap(-1);
		model_set_forced_texture(CLOAKMAP);
		gr_set_cull(1);
		model_interp_sortnorm=model_interp_sortnorm_f2b;
	}
	else
	{
		unit=2;
		Interp_multitex_cloakmap=1;
	}

	Interp_cloakmap_alpha=alpha;
	gr_push_texture_matrix(unit);
	gr_translate_texture_matrix(unit,shift);
}

void model_finish_cloak(int full_cloak)
{
	int unit;
	if (full_cloak){		unit=0; gr_set_cull(0);}
	else				unit=2;

	gr_pop_texture_matrix(unit);
	Interp_multitex_cloakmap=-1;
	model_set_forced_texture(-1);
	FULLCLOAK=-1;
}


// call at the beginning of a level. after the level has been loaded
void model_level_post_init()
{
	/*
	int idx;

	// reset lighting stuff	
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		Interp_lighting_save[idx].objnum = -1;
		Interp_lighting_save[idx].skip = 0;
	}

	// saved lighting is not full
	Interp_saved_lighting_full = 0;
	*/
}

// call to select an object for using "saved" lighting
void model_set_saved_lighting(int objnum, int skip_max)
{
	/*
	int idx;

	// always set to not using saved light to start with
	Interp_use_saved_lighting = 0;
	Interp_lighting = &Interp_lighting_temp;

	// if he passed a -1 for either value, no saved lighting
	if((objnum == -1) || (skip_max == -1)){
		return;
	}

	// see if the object is already on the list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// ahha, he is on the list
		if(Interp_lighting_save[idx].objnum == objnum){
			// if he's entered a new skip max
			if(Interp_lighting_save[idx].skip_max != skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_lighting_save[idx].skip_max = skip_max;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			} 
			// if we're reached the "skip" frame, re-render lighting for this guy
			else if(Interp_lighting_save[idx].skip == Interp_lighting_save[idx].skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			}
			// otherwise, use his saved lighting values
			else {
				Interp_lighting_save[idx].skip++;
				Interp_use_saved_lighting = 1;
				Interp_lighting = &Interp_lighting_save[idx];
			}

			// duh
			return;
		}
	}

	// no free saved lighting slots
	if(Interp_saved_lighting_full){
		return;
	}
	
	// find a free slot
	int found = 0;
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// got one
		if(Interp_lighting_save[idx].objnum == -1){
			Interp_lighting_save[idx].objnum = objnum;
			Interp_lighting_save[idx].skip = 0;
			Interp_lighting_save[idx].skip_max = skip_max;

			Interp_use_saved_lighting = 0;
			Interp_lighting = &Interp_lighting_save[idx];

			found = 1;
			break;
		}
	}

	// oops. out of free slots
	if(!found){
		Interp_saved_lighting_full = 1;
	}
	*/
}

// notify the model system that a ship has died
void model_notify_dead_ship(int objnum)
{
	/*
	int idx;

	// see if this guy was on the lighting list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// free him up
		if(objnum == Interp_lighting_save[idx].objnum){
			Interp_lighting_save[idx].objnum = -1;
			Interp_saved_lighting_full = 0;
			return;
		}
	}
	*/
}

void interp_clear_instance()
{
	Interp_thrust_scale = 0.1f;
	Interp_thrust_scale_x = 0.0f;//added-Bobboau
	Interp_thrust_scale_y = 0.0f;//added-Bobboau
	Interp_thrust_bitmap = -1;
	Interp_thrust_glow_bitmap = -1;
	Interp_secondary_thrust_glow_bitmap = -1;
	Interp_tertiary_thrust_glow_bitmap = -1;
	Interp_thrust_glow_noise = 1.0f;
	Interp_insignia_bitmap = -1;
	Interp_thrust_glow_rad_factor = 1.0f;
	Interp_secondary_thrust_glow_rad_factor = 1.0f;
	Interp_secondary_thrust_glow_len_factor = 1.0f;
	Interp_tertiary_thrust_glow_rad_factor = 1.0f;
	Interp_AB=false;
	vm_vec_zero(&controle_rotval);
}

// Scales the engines thrusters by this much
void model_set_thrust( int model_num, vec3d *length /*<-I did that-Bobboau*/, int bitmap, int glow_bitmap, float glow_noise, bool AB, int secondary_bitmap, int tertiary_bitmap, vec3d *rovel, float trf1, float trf2, float trf3, float tlf)
{
	Interp_AB = AB;

	Interp_thrust_glow_rad_factor = trf1;
	Interp_secondary_thrust_glow_rad_factor = trf2;
	Interp_secondary_thrust_glow_len_factor = tlf;
	Interp_tertiary_thrust_glow_rad_factor = trf3;

	Interp_thrust_scale = length->xyz.z;
	Interp_thrust_scale_x = length->xyz.x;
	Interp_thrust_scale_y = length->xyz.y;
	Interp_thrust_bitmap = bitmap;
	Interp_thrust_glow_bitmap = glow_bitmap;
	Interp_thrust_glow_noise = glow_noise;
	Interp_secondary_thrust_glow_bitmap = secondary_bitmap;
	Interp_tertiary_thrust_glow_bitmap = tertiary_bitmap;
	if(rovel != NULL)controle_rotval = *rovel;
	else 	vm_vec_zero(&controle_rotval);


	if ( Interp_thrust_scale < 0.1f ) {
		Interp_thrust_scale = 0.1f;
	} else if ( Interp_thrust_scale > 1.0f ) {
		Interp_thrust_scale = 1.0f;
	}
	//this isn't used
/*
	polymodel * pm = model_get( model_num );
	int i;

	// If thrust is set up, use it.
	for (i=0; i<pm->num_lights; i++ )	{
		if ( pm->lights[i].type == BSP_LIGHT_TYPE_THRUSTER )	{
			float scale = (Interp_thrust_scale-0.1f)*0.5f;

			pm->lights[i].value += (scale+Interp_thrust_glow_noise*0.2f) / 255.0f;
		}
	}
	*/
}

extern int spec;
extern int Cmdline_cell;
extern bool cell_enabled;


bool splodeing = false;
int splodeingtexture = -1;
float splode_level = 0.0f;

float GEOMETRY_NOISE = 0.0f;

// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.
void model_interp_splode_defpoints(ubyte * p, polymodel *pm, bsp_info *sm, float dist)
{
	if(dist==0.0f)return;

	if(dist<0.0f)dist*=-1.0f;

	int n;
	int nverts = w(p+8);	
	int offset = w(p+16);

	ubyte * normcount = p+20;
	vertex *dest = Interp_splode_points;
	vec3d *src = vp(p+offset);

	Assert( nverts < MAX_POLYGON_VECS );

	vec3d dir;

	for (n=0; n<nverts; n++ )	{	
		Interp_splode_verts[n] = *src;		
			
		src++;

		vm_vec_avg_n(&dir, normcount[n], src);
		vm_vec_normalize(&dir);

		for(int i=0; i<normcount[n]; i++)src++;

		vm_vec_scale_add2(&Interp_splode_verts[n], &dir, dist);

		g3_rotate_vertex(dest, &Interp_splode_verts[n]);
		
		dest++;

	}

}

// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.
void model_interp_defpoints(ubyte * p, polymodel *pm, bsp_info *sm)
{
	if(Cmdline_cell)model_interp_splode_defpoints(p, pm, sm, model_radius/100);
	if(splodeing)model_interp_splode_defpoints(p, pm, sm, splode_level*model_radius);

	int i, n;
	int nverts = w(p+8);	
	int offset = w(p+16);
	int next_norm = 0;

	ubyte * normcount = p+20;
	vertex *dest = Interp_points;
	vec3d *src = vp(p+offset);

	// Get pointer to lights
	Interp_lights = p+20+nverts;

	Assert( nverts < MAX_POLYGON_VECS );
	// Assert( nnorms < MAX_POLYGON_NORMS );

	Interp_num_verts = nverts;
	#ifndef NDEBUG
	modelstats_num_verts += nverts;
	#endif

/*
	static int Max_vecs = 0;
	static int Max_norms = 0;

	if ( Max_vecs < nverts )	{
		Max_vecs = nverts;
		mprintf(( "MAX NORMS = %d\n", Max_norms ));
		mprintf(( "MAX VECS = %d\n", Max_vecs ));
	}

	if ( Max_norms < nnorms )	{
		Max_norms = nnorms;
		mprintf(( "MAX NORMS = %d\n", Max_norms ));
		mprintf(( "MAX VECS = %d\n", Max_vecs ));
	}
*/

	if (Interp_thrust_scale_subobj)	{

		// Only scale vertices that aren't on the "base" of 
		// the effect.  Base is something Adam decided to be
		// anything under 1.5 meters, hence the 1.5f.
		float min_thruster_dist = -1.5f;

		if ( Interp_flags & MR_IS_MISSILE )	{
			min_thruster_dist = 0.5f;
		}

		for (n=0; n<nverts; n++ )	{
			vec3d tmp;
			if(nverts >= MAX_POLYGON_VECS)Error( LOCATION, "model has too many points %d needs to be less than %d\n", nverts ,MAX_POLYGON_VECS );

			Interp_verts[n] = src;

			// Only scale vertices that aren't on the "base" of 
			// the effect.  Base is something Adam decided to be
			// anything under 1.5 meters, hence the 1.5f.
			if ( src->xyz.z < min_thruster_dist )	{
				tmp.xyz.x = src->xyz.x * 1.0f;
				tmp.xyz.y = src->xyz.y * 1.0f;
				tmp.xyz.z = src->xyz.z * Interp_thrust_scale;
			} else {
				tmp = *src;
			}
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				if(next_norm >= MAX_POLYGON_NORMS)Error( LOCATION, "model has too many normals %d needs to be less than %d\n", next_norm ,MAX_POLYGON_NORMS );

				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 

	} else if((Model_Interp_scale_x != 1) || (Model_Interp_scale_y != 1) || (Model_Interp_scale_z != 1)) {

		// SHUT UP! -- Kazan -- This is massively slowing debug builds down
		//mprintf(("warp model being scaled by %f %f %f\n",Model_Interp_scale_x ,Model_Interp_scale_y, Model_Interp_scale_z));
		for (n=0; n<nverts; n++ )	{
			vec3d tmp;

			Interp_verts[n] = src;

				tmp.xyz.x = (src->xyz.x) * Model_Interp_scale_x;
				tmp.xyz.y = (src->xyz.y) * Model_Interp_scale_y;
				tmp.xyz.z = (src->xyz.z) * Model_Interp_scale_z;
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 
	}else{


		vec3d point;

		for (n=0; n<nverts; n++ )	{	

			if(!Cmdline_nohtl) {
				if(GEOMETRY_NOISE!=0.0f){
					GEOMETRY_NOISE = model_radius / 50;

					Interp_verts[n] = src;	
					point.xyz.x = src->xyz.x + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
					point.xyz.y = src->xyz.y + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
					point.xyz.z = src->xyz.z + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
							
					g3_rotate_vertex(dest, &point);
				}else{
					Interp_verts[n] = src;	
					g3_rotate_vertex(dest, src);
				}
			}
			else {
	
				Interp_verts[n] = src; 	 
				 /* 	                                 
				 vec3d tmp = *src; 	 
				 // TEST 	                                 ;
				 if(Interp_thrust_twist > 0.0f){ 	                                 
						 float theta; 	                                 
						 float st, ct; 	                                 

						 // determine theta for this vertex 	                                 
						 theta = fl_radian(20.0f + Interp_thrust_twist2); 	                         
						 st = sin(theta); 	                                 
						 ct = cos(theta); 	                                 
  					 }
						 // twist 	 
						 tmp.xyz.z = (src->xyz.z * ct) - (src->xyz.y * st); 	 
						 tmp.xyz.y = (src->xyz.z * st) + (src->xyz.y * ct); 	 

						 // scale the z a bit 	 
						 tmp.xyz.z += Interp_thrust_twist; 	 
				 } 	 

				 g3_rotate_vertex(dest, &tmp); 	 
				 */ 	 

				 g3_rotate_vertex(dest, src);
			}

			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		}
	}

	Interp_num_norms = next_norm;

}

matrix *Interp_orient;
vec3d *Interp_pos;
vec3d Interp_offset;


void interp_compute_environment_mapping( vec3d *nrm, vertex * pnt)
{
	return;
	vec3d R;
	float a;
//	matrix * m = &View_matrix;

	vm_vec_rotate( &R, nrm, &View_matrix );	
	vm_vec_normalize(&R);

	a = 2.0f * R.xyz.z;
	R.xyz.x = a * R.xyz.x;	// reflected R = 2N(N.E) -E;  E = eye
	R.xyz.y = a * R.xyz.y;
	R.xyz.z = a * R.xyz.z;
	vm_vec_normalize(&R);
	a = (float)fl_sqrt( 1.0f - R.xyz.y * R.xyz.y);
	pnt->u2 = (float)atan2( R.xyz.x, -R.xyz.z) / (2.0f * 3.14159f);
	if (pnt->u2 < 0.0) pnt->u2 += 1.0f;
	pnt->v2 = 1.0f - (float)atan2( a, R.xyz.y) / 3.14159f;
}

extern int spec;
extern bool cell_enabled;


// Flat Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
// +32     float       radius
// +36     int         nverts
// +40     byte        red
// +41     byte        green
// +42     byte        blue
// +43     byte        pad
// +44     nverts*short*short  vertlist, smoothlist
void model_interp_flatpoly(ubyte * p,polymodel * pm)
{
	vertex *Interp_list[TMAP_MAX_VERTS];
	int nv = w(p+36);

	if ( nv < 0 )	return;

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if (!g3_check_normal_facing(vp(p+20),vp(p+8)) ) return;
	

	int i;
	short * verts = (short *)(p+44);
	
	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i*2]];

		if ( Interp_flags & MR_NO_LIGHTING )	{
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
		} else if(Cmdline_cell){
			Interp_list[i]->r = 0;
			Interp_list[i]->g = 0;
			Interp_list[i]->b = 0;
		}else{
			int vertnum = verts[i*2+0];
			int norm = verts[i*2+1];
	
			if ( Interp_flags & MR_NO_SMOOTHING )	{
				light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
			} else {
				// if we're not using saved lighting
				if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{
					light_apply_rgb( &Interp_lighting->r[norm], &Interp_lighting->g[norm], &Interp_lighting->b[norm], Interp_verts[vertnum], vp(p+8), Interp_light );
					Interp_light_applied[norm] = 1;
				}

				Interp_list[i]->r = Interp_lighting->r[norm];
				Interp_list[i]->g = Interp_lighting->g[norm];
				Interp_list[i]->b = Interp_lighting->b[norm];
			}
		}
	}

	// HACK!!! FIX ME!!! I'M SLOW!!!!
	if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
		gr_set_color( *(p+40), *(p+41), *(p+42) );
	}

	if ( !(Interp_flags & MR_NO_POLYS))	{
		g3_draw_poly( nv, Interp_list, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB );	
	}

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET))	{
		int i, j;

		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
			j = (i + 1) % nv;
			g3_draw_line(Interp_list[i], Interp_list[j]);
		}
	}
}

extern bool env_enabled;

void model_interp_edge_alpha( ubyte *param_r, ubyte *param_g, ubyte *param_b, vec3d *pnt, vec3d *norm, float alpha, bool invert = false){
	vec3d r;
	vm_vec_sub(&r, &View_position, pnt);
	vm_vec_normalize(&r);
	float d = vm_vec_dot(&r, norm);
	if(d<0.0f)d=-d;

	if(invert)
		*param_r = *param_g = *param_b = ubyte( fl2i((1.0f - d) * 254.0f * alpha));
	else
		*param_r = *param_g = *param_b = ubyte( fl2i(d * 254.0f * alpha) );
}

// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
// +32     float       radius
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
extern int Tmap_show_layers;

int Interp_subspace = 0;
float Interp_subspace_offset_u = 0.0f;
float Interp_subspace_offset_v = 0.0f;
ubyte Interp_subspace_r = 255;
ubyte Interp_subspace_g = 255;
ubyte Interp_subspace_b = 255;

void model_interp_tmappoly(ubyte * p,polymodel * pm)
{
	vertex *Interp_list[TMAP_MAX_VERTS];
	int i;
	int nv;
	model_tmap_vert *verts;

	// Goober5000
	int tmap_num = w(p+40);
	//mprintf(("model_interp_tmappoly tmap_num: %d\n", tmap_num));
	//Assert(tmap_num >= 0 && tmap_num < MAX_MODEL_TEXTURES);

	int is_invisible = 0;

	if((Warp_Map < 0)){
		if ((!Interp_thrust_scale_subobj) && (pm->textures[tmap_num]<0))	{
			// Don't draw invisible polygons.
			if ( !(Interp_flags & MR_SHOW_INVISIBLE_FACES))	{
				return;
			} else {
				is_invisible = 1;
			}
		}
	}


	nv = w(p+36);

//	Tmap_show_layers = 1;

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if ( nv < 0 ) return;

	verts = (model_tmap_vert *)(p+44);
	if(!Cmdline_nohtl) {
		if((Warp_Map < 0)){
			if (!g3_check_normal_facing(vp(p+20),vp(p+8)) && !(Interp_flags & MR_NO_CULL)){
				if(Cmdline_cell){
					for (i=0;i<nv;i++){
						Interp_list[i] = &Interp_splode_points[verts[i].vertnum];
						Interp_list[i]->u = verts[i].u;
						Interp_list[i]->v = verts[i].v;
						Interp_list[i]->r = 250;
						Interp_list[i]->g = 250;
						Interp_list[i]->b = 250;
		
					}
					gr_set_cull(0);
					gr_set_color( 0, 0, 0 );
					g3_draw_poly( nv, Interp_list, 0 );
					gr_set_cull(1);
				}
				if(!splodeing)return;
			}
		}

		if(splodeing){
			float salpha = 1.0f - splode_level;
			for (i=0;i<nv;i++){
				Interp_list[i] = &Interp_splode_points[verts[i].vertnum];
				Interp_list[i]->u = verts[i].u*2;
				Interp_list[i]->v = verts[i].v*2;
				Interp_list[i]->r = (unsigned char)(255*salpha);
				Interp_list[i]->g = (unsigned char)(250*salpha);
				Interp_list[i]->b = (unsigned char)(200*salpha);
				model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], salpha, false);
			}
			gr_set_cull(0);
			gr_set_bitmap( splodeingtexture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, salpha );
		//	gr_set_color( 255, 250, 200 );
		//	g3_draw_poly( nv, Interp_list, 0 );
			g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_GOURAUD);
			gr_set_cull(1);
			return;
		}
	}

	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i].vertnum];

		Interp_list[i]->u = verts[i].u;
		Interp_list[i]->v = verts[i].v;
		
		if ( Interp_subspace )	{
			Interp_list[i]->v += Interp_subspace_offset_u;
			Interp_list[i]->u += Interp_subspace_offset_v;
			Interp_list[i]->r = Interp_subspace_r;
			Interp_list[i]->g = Interp_subspace_g;
			Interp_list[i]->b = Interp_subspace_b;
		} else {

	//		if ( !(pm->flags & PM_FLAG_ALLOW_TILING) )	{
	//			Assert(verts[i].u <= 1.0f );
	//			Assert(verts[i].v <= 1.0f );
	//		}

	//		Assert( verts[i].normnum == verts[i].vertnum );
			if ( (Interp_flags & MR_NO_LIGHTING) || (pm->ambient[tmap_num]))	{	//gets the ambient glow to work
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
				
				Interp_list[i]->spec_r = 0;
				Interp_list[i]->spec_g = 0;
				Interp_list[i]->spec_b = 0;
				
				if((Interp_flags & MR_EDGE_ALPHA))model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], Warp_Alpha, false);
				if((Interp_flags & MR_CENTER_ALPHA))model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], Warp_Alpha, true);
				SPECMAP = -1;
			} else {

				int vertnum = verts[i].vertnum;
				int norm = verts[i].normnum;
		
				if ( Interp_flags & MR_NO_SMOOTHING )	{
					light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
					if((Detail.lighting > 2) && (model_current_LOD < 2) && !Cmdline_cell && !Cmdline_nospec )
						light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], vp(p+8),  &View_position);
					//	interp_compute_environment_mapping(vp(p+8), Interp_list[i]);
				} else {					
					// if we're applying lighting as normal, and not using saved lighting
					if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{

						light_apply_rgb( &Interp_lighting->r[norm], &Interp_lighting->g[norm], &Interp_lighting->b[norm], Interp_verts[vertnum], Interp_norms[norm], Interp_light );
						if((Detail.lighting > 2) && (model_current_LOD < 2) && !Cmdline_cell && !Cmdline_nospec )
							light_apply_specular( &Interp_lighting->spec_r[norm], &Interp_lighting->spec_g[norm], &Interp_lighting->spec_b[norm], Interp_verts[vertnum], Interp_norms[norm],  &View_position);
						//	interp_compute_environment_mapping(Interp_verts[vertnum], Interp_list[i]);

						Interp_light_applied[norm] = 1;
					}else if(Interp_light_applied[norm]){
					//	if((Detail.lighting > 2) && (model_current_LOD < 2))
					//		light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);
					//	interp_compute_environment_mapping(Interp_verts[vertnum], Interp_list[i]);
					}

					Interp_list[i]->spec_r = Interp_lighting->spec_r[norm];
					Interp_list[i]->spec_g = Interp_lighting->spec_g[norm];
					Interp_list[i]->spec_b = Interp_lighting->spec_b[norm];

					Interp_list[i]->r = Interp_lighting->r[norm];
					Interp_list[i]->g = Interp_lighting->g[norm];
					Interp_list[i]->b = Interp_lighting->b[norm];
//					if((Detail.lighting > 2) && (model_current_LOD < 2))
//						light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);
				}
			}
		}

//		Assert(verts[i].u >= 0.0f );
//		Assert(verts[i].v >= 0.0f );

	}

	#ifndef NDEBUG
	modelstats_num_polys_drawn++;
	#endif

	if (!(Interp_flags & MR_NO_POLYS) )		{
		if ( is_invisible )	{
			gr_set_color( 0, 255, 0 );
			g3_draw_poly( nv, Interp_list, 0 );		
		} else if (Interp_thrust_scale_subobj)	{
			if ((Interp_thrust_bitmap >= 0)	&& (Interp_thrust_scale > 0.0f) && !Pofview_running) {
				gr_set_bitmap( Interp_thrust_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );
				g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );		
			} else if(!Pofview_running){
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 255 );
				}
				uint tflags = Interp_tmap_flags;
				tflags &=  (~(TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT));
				g3_draw_poly( nv, Interp_list, tflags );		
			}
		} else if(Warp_Map >= 0){	//warpin effect-Bobboau
			gr_set_bitmap( Warp_Map, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Warp_Alpha );
			g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );
		}else{
			env_enabled = true;
			cell_enabled = true;
			// all textured polys go through here
			if ( Interp_tmap_flags & TMAP_FLAG_TEXTURED )	{
				// subspace special case
				if ( Interp_subspace && (D3D_enabled || OGL_enabled) )	{										
					gr_set_bitmap( pm->textures[tmap_num], GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );					
				}
				// all other textures
				else {					
					int texture;

					// if we're rendering a nebula background pof, maybe select a custom texture
					if((Interp_flags & MR_FORCE_TEXTURE) && (Interp_forced_bitmap >= 0)){
						texture = Interp_forced_bitmap;
					}else if(Interp_replacement_bitmap >= 0){
						texture = Interp_replacement_bitmap;
					} else {
						if (pm->is_ani[tmap_num]){
							texture = pm->textures[tmap_num] + ((timestamp() / (int)(pm->fps[tmap_num])) % pm->num_frames[tmap_num]);//here is were it picks the texture to render for ani-Bobboau
						}else{
							texture = pm->textures[tmap_num];//here is were it picks the texture to render for normal-Bobboau
						}

						if((Detail.lighting > 2)  && (model_current_LOD < 1))SPECMAP = pm->specular_textures[tmap_num];

						if(glow_maps_active)
						{
							if (pm->glow_is_ani[tmap_num])
							{
								GLOWMAP = pm->glow_textures[tmap_num] + ((timestamp() / (int)(pm->glow_fps[tmap_num])) % pm->glow_numframes[tmap_num]);
							}
							else
							{
								GLOWMAP = pm->glow_textures[tmap_num];
							}
						}
					}

					// muzzle flashes draw xparent
					if(Interp_flags & MR_ALL_XPARENT){
						gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_xparent_alpha );
					} else {
						if(pm->transparent[tmap_num]){	//trying to get transperent textures-Bobboau
							gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f );
						}else{
							gr_set_bitmap( texture );
						}
					}
				}
			} else {
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 128 );
				}
			}

			if ( Interp_subspace )	{
				g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT );		
			} else {				
				if(Interp_flags & MR_ALL_XPARENT){
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags );
				} else {
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags|TMAP_FLAG_NONDARKENING );		
				}
			}

//			env_enabled = false;
//			GLOWMAP = -1;
//			SPECMAP = -1;
		}
	}
	env_enabled = false;
	cell_enabled = false;
	GLOWMAP = -1;
	SPECMAP = -1;

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET) )	{
	
		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
			int j = (i + 1) % nv;
	   		g3_draw_line(Interp_list[i], Interp_list[j]);
		}
	}

}

void interp_draw_box( vec3d *min, vec3d *max )
{
/*
	int i;
	vec3d bounding_box[8];
	vertex pts[8];
	vertex *l[4];

	model_calc_bound_box(bounding_box,min,max);

	for (i=0; i<8; i++ )	{
		g3_rotate_vertex( &pts[i], &bounding_box[i] );
	}

	gr_set_color(128,0,0);

	Tmap_show_layers = 1;

	l[3] = &pts[0];  l[2] = &pts[1]; l[1] = &pts[2];  l[0] = &pts[3];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[3];  l[2] = &pts[2]; l[1] = &pts[6];  l[0] = &pts[7];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[2];  l[2] = &pts[1]; l[1] = &pts[5];  l[0] = &pts[6];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[1];  l[2] = &pts[0]; l[1] = &pts[4];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[0];  l[2] = &pts[3]; l[1] = &pts[7];  l[0] = &pts[4];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[4];  l[2] = &pts[7]; l[1] = &pts[6];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );
*/
}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);
	}

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		// 

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist
}




void model_draw_debug_points( polymodel *pm, bsp_info * submodel )
{
	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	// Draw the brown "core" mass
//	if ( submodel && (submodel->parent==-1) )	{
//		gr_set_color(128,128,0);
//		g3_draw_sphere_ez( &vmd_zero_vector, pm->core_radius );
//	}

	// Draw a red pivot point
	gr_set_color(128,0,0);
	g3_draw_sphere_ez(&vmd_zero_vector, 2.0f );

	// Draw a green center of mass when drawing the hull
	if ( submodel && (submodel->parent==-1) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( &pm->center_of_mass, 1.0f );
	}

	if ( submodel )	{
		// Draw a blue center point
		gr_set_color(0,0,128);
		g3_draw_sphere_ez( &submodel->geometric_center, 0.9f );
	}
	
	// Draw the bounding box
	int i;
	vertex pts[8];

	if ( submodel )	{
		for (i=0; i<8; i++ )	{
			g3_rotate_vertex( &pts[i], &submodel->bounding_box[i] );
		}
		gr_set_color(128,128,128);
		g3_draw_line( &pts[0], &pts[1] );
		g3_draw_line( &pts[1], &pts[2] );
		g3_draw_line( &pts[2], &pts[3] );
		g3_draw_line( &pts[3], &pts[0] );

		g3_draw_line( &pts[4], &pts[5] );
		g3_draw_line( &pts[5], &pts[6] );
		g3_draw_line( &pts[6], &pts[7] );
		g3_draw_line( &pts[7], &pts[4] );

		g3_draw_line( &pts[0], &pts[4] );
		g3_draw_line( &pts[1], &pts[5] );
		g3_draw_line( &pts[2], &pts[6] );
		g3_draw_line( &pts[3], &pts[7] );
	} else {
		//for (i=0; i<8; i++ )	{
		//	g3_rotate_vertex( &pts[i], &pm->bounding_box[i] );
		//}
		gr_set_color(0,255,0);

		int j;
		for (j=0; j<8; j++ )	{

			vec3d	bounding_box[8];		// caclulated fron min/max
			model_calc_bound_box(bounding_box,&pm->octants[j].min,&pm->octants[j].max);

			for (i=0; i<8; i++ )	{
				g3_rotate_vertex( &pts[i], &bounding_box[i] );
			}
			gr_set_color(128,0,0);
			g3_draw_line( &pts[0], &pts[1] );
			g3_draw_line( &pts[1], &pts[2] );
			g3_draw_line( &pts[2], &pts[3] );
			g3_draw_line( &pts[3], &pts[0] );

			g3_draw_line( &pts[4], &pts[5] );
			g3_draw_line( &pts[5], &pts[6] );
			g3_draw_line( &pts[6], &pts[7] );
			g3_draw_line( &pts[7], &pts[4] );

			g3_draw_line( &pts[0], &pts[4] );
			g3_draw_line( &pts[1], &pts[5] );
			g3_draw_line( &pts[2], &pts[6] );
			g3_draw_line( &pts[3], &pts[7] );			
		}		
	}
}

// Debug code to show all the paths of a model
void model_draw_paths( int model_num )
{
	int i,j;
	vec3d pnt;
	polymodel * pm;	

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	pm = model_get(model_num);

	if (pm->n_paths<1){
		return;
	}	

	for (i=0; i<pm->n_paths; i++ )	{
		vertex prev_pnt;

		for (j=0; j<pm->paths[i].nverts; j++ )	{
			// Rotate point into world coordinates			
			pnt = pm->paths[i].verts[j].pos;

			// Pnt is now the x,y,z world coordinates of this vert.
			// For this example, I am just drawing a sphere at that
			// point.
			{
				vertex tmp;
				g3_rotate_vertex(&tmp,&pnt);

				if ( pm->paths[i].verts[j].nturrets > 0 ){
					gr_set_color( 0, 0, 255 );						// draw points covered by turrets in blue
				} else {
					gr_set_color( 255, 0, 0 );
				}

				g3_draw_sphere( &tmp, 0.5f );

				if (j){
					g3_draw_line(&prev_pnt, &tmp);
				}

				prev_pnt = tmp;
			}
		}
	}
}

// Debug code to show all the paths of a model
void model_draw_paths_htl( int model_num )
{
	int i,j;
	vec3d pnt;
	vec3d prev_pnt;
	polymodel * pm;	

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	pm = model_get(model_num);

	if (pm->n_paths<1){
		return;
	}	

	gr_set_cull(0);
	for (i=0; i<pm->n_paths; i++ )	{
		for (j=0; j<pm->paths[i].nverts; j++ )
		{
			// Rotate point into world coordinates			
			pnt = pm->paths[i].verts[j].pos;

			// Pnt is now the x,y,z world coordinates of this vert.
			// For this example, I am just drawing a sphere at that
			// point.

			vertex tmp;
			g3_rotate_vertex(&tmp,&pnt);

			if ( pm->paths[i].verts[j].nturrets > 0 ){
				gr_set_color( 0, 0, 255 );						// draw points covered by turrets in blue
			} else {
				gr_set_color( 255, 0, 0 );
			}

			g3_draw_htl_sphere(&pnt, 0.5f);
			
			if (j)
			{
				g3_draw_htl_line(&prev_pnt, &pnt);
			}

			prev_pnt = pnt;
		}
	}

	gr_set_cull(1);
}

// docking bay and fighter bay paths
void model_draw_bay_paths_htl(int model_num)
{
	int idx, s_idx;
	vec3d v1, v2;

	polymodel *pm = model_get(model_num);
	if(pm == NULL){
		return;
	}

	gr_set_cull(0);
	// render docking bay normals
	gr_set_color(0, 255, 0);
	for(idx=0; idx<pm->n_docks; idx++){
		for(s_idx=0; s_idx<pm->docking_bays[idx].num_slots; s_idx++){
			v1 = pm->docking_bays[idx].pnt[s_idx];
			vm_vec_scale_add(&v2, &v1, &pm->docking_bays[idx].norm[s_idx], 10.0f);

			// draw the point and normal
			g3_draw_htl_sphere(&v1, 2.0);
			g3_draw_htl_line(&v1, &v2);
		}
	}

	// render figher bay paths
	gr_set_color(0, 255, 255);
		
	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	for (idx = 0; idx<pm->n_paths; idx++) {
		if ( !strnicmp(pm->paths[idx].name, NOX("$bay"), 4) ) {						
			for(s_idx=0; s_idx<pm->paths[idx].nverts-1; s_idx++){
				v1 = pm->paths[idx].verts[s_idx].pos;
				v2 = pm->paths[idx].verts[s_idx+1].pos;

				g3_draw_htl_line(&v1, &v2);
			}
		}
	}	

	gr_set_cull(1);
}


// docking bay and fighter bay paths
void model_draw_bay_paths(int model_num)
{
	int idx, s_idx;
	vec3d v1, v2;
	vertex l1, l2;	

	polymodel *pm = model_get(model_num);
	if(pm == NULL){
		return;
	}

	// render docking bay normals
	gr_set_color(0, 255, 0);
	for(idx=0; idx<pm->n_docks; idx++){
		for(s_idx=0; s_idx<pm->docking_bays[idx].num_slots; s_idx++){
			v1 = pm->docking_bays[idx].pnt[s_idx];
			vm_vec_scale_add(&v2, &v1, &pm->docking_bays[idx].norm[s_idx], 10.0f);

			// rotate the points
			g3_rotate_vertex(&l1, &v1);
			g3_rotate_vertex(&l2, &v2);

			// draw the point and normal
			g3_draw_sphere(&l1, 2.0);
			g3_draw_line(&l1, &l2);
		}
	}

	// render figher bay paths
	gr_set_color(0, 255, 255);
		
	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	for (idx = 0; idx<pm->n_paths; idx++) {
		if ( !strnicmp(pm->paths[idx].name, NOX("$bay"), 4) ) {						
			for(s_idx=0; s_idx<pm->paths[idx].nverts-1; s_idx++){
				v1 = pm->paths[idx].verts[s_idx].pos;
				v2 = pm->paths[idx].verts[s_idx+1].pos;

				// rotate and draw
				g3_rotate_vertex(&l1, &v1);
				g3_rotate_vertex(&l2, &v2);
				g3_draw_line(&l1, &l2);
			}
		}
	}	
}	
/*
// struct that holds the indicies into path information associated with a fighter bay on a capital ship
// NOTE: Fighter bay paths are identified by the path_name $bayN (where N is numbered from 1).
//			Capital ships only have ONE fighter bay on the entire ship
#define MAX_SHIP_BAY_PATHS		10
typedef struct ship_bay {
	int	num_paths;							// how many paths are associated with the model's fighter bay
	int	paths[MAX_SHIP_BAY_PATHS];		// index into polymodel->paths[] array
	int	arrive_flags;	// bitfield, set to 1 when that path number is reserved for an arrival
	int	depart_flags;	// bitfield, set to 1 when that path number is reserved for a departure
} ship_bay;

  typedef struct mp_vert {
	vec3d		pos;				// xyz coordinates of vertex in object's frame of reference
	int			nturrets;		// number of turrets guarding this vertex
	int			*turret_ids;	// array of indices into ship_subsys linked list (can't index using [] though)
	float			radius;			// How far the closest obstruction is from this vertex
} mp_vert;

typedef struct model_path {
	char			name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
	char			parent_name[MAX_NAME_LEN];			// parent name of submodel that path is linked to in POF
	int			parent_submodel;
	int			nverts;
	mp_vert		*verts;
	int			goal;			// Which of the verts is the one closest to the goal of this path
	int			type;			// What this path takes you to... See MP_TYPE_??? defines above for details
	int			value;		// This depends on the type.
									// For MP_TYPE_UNUSED, this means nothing.
									// For MP_TYPE_SUBSYS, this is the subsystem number this path takes you to.
} model_path;
*/

void interp_render_arc_segment( vec3d *v1, vec3d *v2, int depth, color *outside_color, bool init_halfway )
{
	static color halfway_color;
	float d = vm_vec_dist_quick( v1, v2 );

	if(init_halfway)
	{
		gr_init_alphacolor(&halfway_color,
							outside_color->red + (255 - outside_color->red)/2,
							outside_color->green + (255-outside_color->green)/2,
							outside_color->blue + (255-outside_color->blue)/2,
							255);
	}

	if ( d < 0.30f || (depth>4) )	{
		vertex p1, p2;
		g3_rotate_vertex( &p1, v1 );
		g3_rotate_vertex( &p2, v2 );

		//g3_draw_rod( v1, 0.2f, v2, 0.2f, NULL, 0);
		gr_set_color_fast(outside_color);
		g3_draw_rod( v1, 0.6f, v2, 0.6f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
		gr_set_color_fast(&halfway_color);
		g3_draw_rod( v1, 0.45f, v2, 0.45f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
		gr_set_color(255,255,255);
		g3_draw_rod( v1, 0.3f, v2, 0.3f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
	//	g3_draw_line( &p1, &p2 );
	} else {
		// divide in half
		vec3d tmp;
		vm_vec_avg( &tmp, v1, v2 );
	
		float scaler = 0.30f;
		tmp.xyz.x += (frand()-0.5f)*d*scaler;
		tmp.xyz.y += (frand()-0.5f)*d*scaler;
		tmp.xyz.z += (frand()-0.5f)*d*scaler;
		
		interp_render_arc_segment( v1, &tmp, depth+1, outside_color, false );
		interp_render_arc_segment( &tmp, v2, depth+1, outside_color, false );
	}
}

int Interp_lightning = 1;
DCF_BOOL( Arcs, Interp_lightning )

int AR = 64;
int AG = 64;
int AB = 5;
int AR2 = 128;
int AG2 = 128;
int AB2 = 10;
void interp_render_lightning( polymodel *pm, bsp_info * sm )
{
	Assert( sm->num_arcs > 0 );

	int i;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	if (!Interp_lightning) return;

	color outside_arc_color;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;
//	if ( rad < 3.0f ) return;	
	
	for (i=0; i<sm->num_arcs; i++ )	{
		// pick a color based upon arc type
		switch(sm->arc_type[i]){
		// "normal", Freespace 1 style arcs
		case MARC_TYPE_NORMAL:
			if ( (rand()>>4) & 1 )	{
				gr_init_alphacolor(&outside_arc_color, 64, 64, 255, 255 );
			} else {
				gr_init_alphacolor(&outside_arc_color, 128, 128, 255, 255 );
			}
			break;

		// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( (rand()>>4) & 1 )	{
				gr_init_alphacolor(&outside_arc_color, AR, AG, AB, 255 );
			} else {
				gr_init_alphacolor(&outside_arc_color, AR2, AG2, AB2, 255 );
			}
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		interp_render_arc_segment( &sm->arc_pts[i][0], &sm->arc_pts[i][1], 0, &outside_arc_color, true );
	}
}

void model_interp_subcall(polymodel * pm, int mn, int detail_level)
{
	int i;
	int zbuf_mode = gr_zbuffering_mode;

	if ( (mn < 0) || (mn>=pm->n_models) )
		return;

	Assert( mn >= 0 );
	Assert( mn < pm->n_models );

//	mprintf(( "Name = '%s'\n", pm->submodel[mn].name ));
//	char * p = pm->submodel[mn].name;



//	mprintf(("model =%s, submodel=%s, interp offset=%f %f %f\n",pm->filename, pm->submodel[mn].name,Interp_offset.xyz.x,
//															Interp_offset.xyz.y,Interp_offset.xyz.z));
	
	if (pm->submodel[mn].blown_off){
		return;
	}

	if (pm->submodel[mn].is_thruster )	{
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) ){
			return;
		}
		Interp_thrust_scale_subobj=1;
	} else {
		Interp_thrust_scale_subobj=0;
	}

	vm_vec_add2(&Interp_offset,&pm->submodel[mn].offset);

	g3_start_instance_angles(&pm->submodel[mn].offset, &pm->submodel[mn].angs);
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_interp_sub( pm->submodel[mn].bsp_data, pm, &pm->submodel[mn], 0 );

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn] );

	if ( pm->submodel[mn].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[mn]);
	}

	i = pm->submodel[mn].first_child;
	while( i >= 0 )	{
		if (!pm->submodel[i].is_thruster )	{
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			} else {
				zbuf_mode = GR_ZBUFF_FULL;		// read only
			}

			gr_zbuffer_set(zbuf_mode);

			model_interp_subcall( pm, i, detail_level );
		}
		i = pm->submodel[i].next_sibling;
	}

	vm_vec_sub2(&Interp_offset,&pm->submodel[mn].offset);


	g3_done_instance(true);
}

// Returns one of the following
#define IBOX_ALL_OFF 0
#define IBOX_ALL_ON 1
#define IBOX_SOME_ON_SOME_OFF 2

int interp_box_offscreen( vec3d *min, vec3d *max )
{
	if ( keyd_pressed[KEY_LSHIFT] )	{
		return IBOX_ALL_ON;
	}

	vec3d v[8];
	v[0].xyz.x = min->xyz.x; v[0].xyz.y = min->xyz.y; v[0].xyz.z = min->xyz.z;
	v[1].xyz.x = max->xyz.x; v[1].xyz.y = min->xyz.y; v[1].xyz.z = min->xyz.z;
	v[2].xyz.x = max->xyz.x; v[2].xyz.y = max->xyz.y; v[2].xyz.z = min->xyz.z;
	v[3].xyz.x = min->xyz.x; v[3].xyz.y = max->xyz.y; v[3].xyz.z = min->xyz.z;

	v[4].xyz.x = min->xyz.x; v[4].xyz.y = min->xyz.y; v[4].xyz.z = max->xyz.z;
	v[5].xyz.x = max->xyz.x; v[5].xyz.y = min->xyz.y; v[5].xyz.z = max->xyz.z;
	v[6].xyz.x = max->xyz.x; v[6].xyz.y = max->xyz.y; v[6].xyz.z = max->xyz.z;
	v[7].xyz.x = min->xyz.x; v[7].xyz.y = max->xyz.y; v[7].xyz.z = max->xyz.z;

	ubyte and_codes = 0xff;
	ubyte or_codes = 0xff;
	int i;

	for (i=0; i<8; i++ )	{
		vertex tmp;
		ubyte codes=g3_rotate_vertex( &tmp, &v[i] );
// Early out which we cannot do because we want to differentiate btwn
// IBOX_SOME_ON_SOME_OFF and IBOX_ALL_ON
//		if ( !codes )	{
//			//mprintf(( "A point is inside, so render it.\n" ));
//			return 0;		// this point is in, so return 0
//		}
		or_codes |= codes;
		and_codes &= codes;
	}

	// If and_codes is set this means that all points lie off to the
	// same side of the screen.
	if (and_codes)	{
		//mprintf(( "All points offscreen, so don't render it.\n" ));
		return IBOX_ALL_OFF;	//all points off screen
	}

	// If this is set it means at least one of the points is offscreen,
	// but they aren't all off to the same side.
	if (or_codes)	{
		return IBOX_SOME_ON_SOME_OFF;
	}

	// They are all onscreen.
	return IBOX_ALL_ON;	
}


//calls the object interpreter to render an object.  
//returns true if drew
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check )
{
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;
	int pushed = 0;

	chunk_type = w(p);
	chunk_size = w(p+4);

	// Goober5000
	int tmap_num = w(p+40);
	//mprintf(("model_interp_sub tmap_num: %d\n", tmap_num));
	//Assert(tmap_num >= 0 && tmap_num < MAX_MODEL_TEXTURES);
	
	while ( chunk_type != OP_EOF )	{

//		mprintf(( "Processing chunk type %d, len=%d\n", chunk_type, chunk_size ));

		switch (chunk_type) {
		case OP_EOF: return 1;
		case OP_DEFPOINTS:		model_interp_defpoints(p,pm,sm); break;
		case OP_FLATPOLY:		model_interp_flatpoly(p,pm); break;
		case OP_TMAPPOLY:
#ifndef FRED

			// possible texture replacements - Goober5000
			if (Interp_replacement_textures)
			{
				model_set_replacement_bitmap(Interp_replacement_textures[tmap_num]);
			}
			else
			{
				model_set_replacement_bitmap(-1);
			}
#endif
			
			model_interp_tmappoly(p,pm);
			break;
		case OP_SORTNORM:		model_interp_sortnorm(p,pm,sm,do_box_check); break;
	
		case OP_BOUNDBOX:		

			if ( do_box_check )	{
				int retval = interp_box_offscreen( vp(p+8), vp(p+20) );
				switch( retval )	{
				case IBOX_ALL_OFF:
					goto DoneWithThis;	// Don't need to draw any more polys from this box
					break;

				case IBOX_ALL_ON:
					do_box_check = 0;		// Don't need to check boxes any more
					break;

				case IBOX_SOME_ON_SOME_OFF:
					// continue like we were
					break;
				default:
					Int3();
				}
			}


			if (Interp_flags & MR_SHOW_PIVOTS )	{
				#ifndef NDEBUG
				modelstats_num_boxes++;
				#endif
				interp_draw_box( vp(p+8), vp(p+20) );
			}

			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if ( pushed )	{
					light_filter_pop();
					pushed = 0;

				}
				light_filter_push_box( vp(p+8), vp(p+20) );
				pushed = 1;
			}
			break;

		default:
			mprintf(( "Bad chunk type %d, len=%d in model_interp_sub\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}

DoneWithThis:

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		if ( pushed )	{
			light_filter_pop();
			pushed = 0;
		}
	}

	return 1;
}


void model_render_shields( polymodel * pm )
{
	model_set_outline_color(255,255,255);
	int i, j;
	shield_tri *tri;
	vertex pnt0, tmp, prev_pnt;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	gr_set_color(0, 0, 200 );

	//	Scan all the triangles in the mesh.
	for (i=0; i<pm->shield.ntris; i++ )	{

		tri = &pm->shield.tris[i];

		if (g3_check_normal_facing(&pm->shield.verts[tri->verts[0]].pos,&tri->norm ) ) {

			//	Process the vertices.
			//	Note this rotates each vertex each time it's needed, very dumb.
			for (j=0; j<3; j++ )	{

				g3_rotate_vertex(&tmp, &pm->shield.verts[tri->verts[j]].pos );

				if (j)
					g3_draw_line(&prev_pnt, &tmp);
				else
					pnt0 = tmp;
				prev_pnt = tmp;
			}

			g3_draw_line(&pnt0, &prev_pnt);
		}
	}
}

void model_render_insignias(polymodel *pm, int detail_level)
{
	int idx, s_idx;
	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	vec3d t1, t2, t3, x;
	int i1, i2, i3;

	x.xyz.x=0;
	x.xyz.y=0;
	x.xyz.z=0;
	// if the model has no insignias we're done
	if(pm->num_ins <= 0){
		return;
	}

	// set the proper texture
	if(Interp_insignia_bitmap >= 0){		
		gr_set_bitmap(Interp_insignia_bitmap);
	}
	// otherwise don't even bother rendering
	else {
		return;
	}

	// otherwise render them	
	for(idx=0; idx<pm->num_ins; idx++){	
		// skip insignias not on our detail level
		if(pm->ins[idx].detail_level != detail_level){
			continue;
		}

		for(s_idx=0; s_idx<pm->ins[idx].num_faces; s_idx++){
			// get vertex indices
			i1 = pm->ins[idx].faces[s_idx][0];
			i2 = pm->ins[idx].faces[s_idx][1];
			i3 = pm->ins[idx].faces[s_idx][2];

			// transform vecs and setup vertices
			vm_vec_add(&t1, &pm->ins[idx].vecs[i1], &pm->ins[idx].offset);
			vm_vec_add(&t2, &pm->ins[idx].vecs[i2], &pm->ins[idx].offset);
			vm_vec_add(&t3, &pm->ins[idx].vecs[i3], &pm->ins[idx].offset);

			if(Cmdline_nohtl){
				g3_rotate_vertex(&vecs[0], &t1);
				g3_rotate_vertex(&vecs[1], &t2);
				g3_rotate_vertex(&vecs[2], &t3);
			}else{
				g3_transfer_vertex(&vecs[0], &t1);
				g3_transfer_vertex(&vecs[1], &t2);
				g3_transfer_vertex(&vecs[2], &t3);
			}

			// setup texture coords
			vecs[0].u = pm->ins[idx].u[s_idx][0];  vecs[0].v = pm->ins[idx].v[s_idx][0];
			vecs[1].u = pm->ins[idx].u[s_idx][1];  vecs[1].v = pm->ins[idx].v[s_idx][1];
			vecs[2].u = pm->ins[idx].u[s_idx][2];  vecs[2].v = pm->ins[idx].v[s_idx][2];

			// vm_vec_avg3(&x, 
/*
			for(int k =0; k < 3; k++){
				if ( D3D_enabled )	{
					light_apply_rgb( &vecs[k].r, &vecs[k].g, &vecs[k].b, &pm->ins[idx].vecs[k], &pm->ins[idx].norm[k], 0.8f );
				} else {
					vecs[k].b = light_apply( &x, &pm->ins[idx].norm[k], 0.8f );
				}
			}
			vecs[k].r = (ubyte)255;*/
//			gr_printf((0), (0), "r %d, g %d, b %d", (int)vecs[k].r, (int)vecs[k].g, (int)vecs[k].b);
			// draw the polygon
			g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
		//	g3_draw_poly(3, vlist, 0);
		}
	}
}

int Model_texturing = 1;
int Model_polys = 1;

DCF_BOOL( model_texturing, Model_texturing )
DCF_BOOL( model_polys, Model_polys )

MONITOR( NumModelsRend );	
MONITOR( NumHiModelsRend );	
MONITOR( NumMedModelsRend );	
MONITOR( NumLowModelsRend );	


typedef struct model_cache {
	int		model_num;
	//matrix	orient;
	vec3d	pos;
	int		num_lights;

	float		last_dot;

	float		cr;

	int		w, h;
	ubyte		*data;
	int		cached_valid;
	int		bitmap_id;

	angles	angs;

	// thrust stuff
	float		thrust_scale;
	int		thrust_bitmap;
	int		thrust_glow_bitmap;
	float		thrust_glow_noise;

	int		last_frame_rendered;		//	last frame in which this model was rendered not from the cache
} model_cache;

#define MAX_MODEL_CACHE MAX_OBJECTS
model_cache Model_cache[MAX_MODEL_CACHE];		// Indexed by objnum
int Model_cache_inited = 0;



// Returns 0 if not valid points
int model_cache_calc_coords(vec3d *pnt,float rad, float *cx, float *cy, float *cr)
{
	vertex pt;
	ubyte flags;

	flags = g3_rotate_vertex(&pt,pnt);

	if (flags == 0) {

		g3_project_vertex(&pt);

		if (!(pt.flags & (PF_OVERFLOW|CC_BEHIND)))	{

			*cx = pt.sx;
			*cy = pt.sy;
			*cr = rad*Matrix_scale.xyz.x*Canv_w2/pt.z;

			if ( *cr < 1.0f )	{
				*cr = 1.0f;
			}

			int x1, x2, y1, y2;

			x1 = fl2i(*cx-*cr); 
			if ( x1 < gr_screen.clip_left ) return 0;
			x2 = fl2i(*cx+*cr);
			if ( x2 > gr_screen.clip_right ) return 0;
			y1 = fl2i(*cy-*cr);
			if ( y1 < gr_screen.clip_top ) return 0;
			y2 = fl2i(*cy+*cr);
			if ( y2 > gr_screen.clip_bottom ) return 0;

			return 1;
		}
	}
	return 0;
}


//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if not
int model_get_rotated_bitmap_points(vertex *pnt,float angle, float rad, vertex *v)
{
	float sa, ca;
	int i;

	Assert( G3_count == 1 );



//	angle = 0.0f;
		
	sa = (float)sin(angle);
	ca = (float)cos(angle);

	float width, height;

	width = height = rad;

	v[0].x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[0].y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[0].z = pnt->z;
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;

	v[1].x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[1].y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[1].z = pnt->z;
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;

	v[2].x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[2].y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[2].z = pnt->z;
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;

	v[3].x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[3].y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[3].z = pnt->z;
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		g3_project_vertex(&v[i]);
		v[i].sw = sw;
	}

	if (codes_and)
		return 1;		//1 means off screen

	return 0;
}


int Model_caching = 1;
DCF_BOOL( model_caching, Model_caching );

extern int Tmap_scan_read;		// 0 = normal mapper, 1=read, 2=write

#define MODEL_MAX_BITMAP_SIZE 128
ubyte tmp_bitmap[MODEL_MAX_BITMAP_SIZE*MODEL_MAX_BITMAP_SIZE];

void mc_get_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*sptr++ = *dptr;
			*dptr++ = 255;				// XPARENT!
		}
	}

	gr_unlock();
}

void mc_put_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*dptr++ = *sptr++;
		}
	}

	gr_unlock();
}

float Interp_depth_scale = 1500.0f;

DCF(model_darkening,"Makes models darker with distance")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT);
		Interp_depth_scale = Dc_arg_float;
	}

	if ( Dc_help )	{
		dc_printf( "Usage: model_darkening float\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "model_darkening = %.1f\n", Interp_depth_scale );
	}
}


		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 4.0f
		
		//float cmp_val = 999.75f;		// old

// Return a number from 'min' to 'max' where it is
// 'v' is between v1 and v2.
float scale_it( float min, float max, float v, float v1, float v2 )
{
	if ( v < v1 ) return min;
	if ( v > v2 ) return max;

	v = (v - v1)/(v2-v1);
	v = v*(max-min)+min;

	return v;
}

void model_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int lighting_skip, int *replacement_textures)
{
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel *pm = model_get(model_num);

	int time = timestamp();
	for (int i = 0; i < pm->n_glows; i++ ) { //glow point blink code -Bobboau
		glow_bank *bank = &pm->glows[i];
		if (bank->glow_timestamp == 0)
			bank->glow_timestamp=time;
		if(bank->off_time){
		//	time += bank->disp_time;
			if(bank->is_on){
				if( (bank->on_time) > ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=0;
				}
			}else{
				if( (bank->off_time) < ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=1;
				}
			}
		}
	}


	// maybe turn off (hardware) culling
	if(flags & MR_NO_CULL){
		gr_set_cull(0);
	}

	Interp_objnum = objnum;


/*
	vec3d thruster_pnt=vmd_zero_vector;
	vec3d rotated_thruster;
	//maybe add lights by engine glows
	if (Interp_flags & MR_SHOW_THRUSTERS) 
	{
		for (int thruster_num=0; thruster_num < pm->n_thrusters; thruster_num++)
		{
			for (int slot_num=0; slot_num < pm->thrusters[thruster_num].num_slots; slot_num++)
			{
				thruster_pnt=pm->thrusters[thruster_num].pnt[slot_num];
				//vm_vec_rotate(&rotated_thruster,&thruster_pnt,orient);
				
				light_add_point(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*5,pm->thrusters[thruster_num].radius[slot_num]*10,1.0f,0.0f,0.0f,1.0f,-1);

		//		gr_set_color(255,255,255);
			//	g3_draw_sphere_ez(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*15);
			}
		}
	}*/

	if ( flags & MR_NO_LIGHTING )	{
		Interp_light = 1.0f;

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
	} else if ( flags & MR_IS_ASTEROID ) {
		// Dim it based on distance
		float depth = vm_vec_dist_quick( pos, &Eye_position );
		if ( depth > Interp_depth_scale )	{
			Interp_light = Interp_depth_scale/depth;
			// If it is too far, exit
			if ( Interp_light < (1.0f/32.0f) ) {
				Interp_light = 0.0f;
				return;
			} else if ( Interp_light > 1.0f )	{
				Interp_light = 1.0f;
			}
		} else {
			Interp_light = 1.0f;
		}

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
	} else {
		Interp_light = 1.0f;

		// maybe use saved lighting
		model_set_saved_lighting(objnum, hack_skip_max);
	}

	int num_lights = 0;

	if ( !(flags & MR_NO_LIGHTING ) )	{
		num_lights = light_filter_push( objnum, pos, pm->rad );
	}

	model_try_cache_render(model_num, orient, pos, flags, objnum, num_lights );
	
	if ( !(flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();
	}

	// maybe turn culling back on
	if(flags & MR_NO_CULL){
		gr_set_cull(1);
	}

	// turn off fog after each model renders
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}
}


void model_cache_init()
{
	if ( !Model_cache_inited )	{
		int i;

		Model_cache_inited = 1;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			Model_cache[i].data = NULL;
			Model_cache[i].bitmap_id = -1;
			Model_cache[i].last_frame_rendered = -1;
		}
	}
}

void model_cache_reset()
{
	if ( Model_cache_inited )	{
		int i;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			if ( Model_cache[i].data )	{
				vm_free(Model_cache[i].data);
				Model_cache[i].data = NULL;
			}
			if ( Model_cache[i].bitmap_id != -1 )	{
				bm_release(Model_cache[i].bitmap_id);
				Model_cache[i].bitmap_id = -1;
			}
		}
	}
}

// tmp_detail_level
// 0 - Max
// 1
// 2
// 3
// 4 - None

#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in ModelInterp.cpp
#endif

// Given detail level, what is the threshold for how far viewer
// can move in the object's frame of reference before a redraw.
float Mc_viewer_pos_factor[MAX_DETAIL_LEVEL+1] = {  0.080f, 0.040f, 0.020f, 0.010f, 0.0f };
float Mc_size_factor[MAX_DETAIL_LEVEL+1] = {  1.40f, 1.30f, 1.20f, 1.10f, 0.0f };

int Model_object_caching_tmp = MAX_DETAIL_LEVEL;

// When framerate goes below this, knock it down a notch.
float Mc_framerate_lo[MAX_DETAIL_LEVEL+1] = { 0.0f, 10.0f, 15.0f, 20.0f, 25.0f };
// When it goes above this, knock it up a notch.
float Mc_framerate_hi[MAX_DETAIL_LEVEL+1] = { 15.0f, 20.0f, 25.0f, 30.0f, 100000.0f };

int Mc_detail_add[MAX_DETAIL_LEVEL+1] = { -2, -1, +1, +2, +4 };

extern float flFrametime;

void model_try_cache_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int num_lights)
{
	model_really_render(model_num, orient, pos, flags, objnum);
	/*
	int i;

	model_cache *mc = NULL;
	
	if ( (objnum >= 0) && (objnum<MAX_MODEL_CACHE) )	{
		mc = &Model_cache[objnum];
	}
	
	if ( (!mc) || (!Model_caching) || (D3D_enabled) || (!Model_cache_inited) || (flags & MR_ALWAYS_REDRAW) || (Detail.object_caching > 3) )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	Assert( mc != NULL );

	// Fake the detail level based on framerate.
	if ( 1.0f / flFrametime < Mc_framerate_lo[Model_object_caching_tmp] )	{
		Model_object_caching_tmp--;
		//	mprintf(( "Model cache level bumped down to %d\n", Model_object_caching ));
	} else if ( 1.0f / flFrametime > Mc_framerate_hi[Model_object_caching_tmp] )	{
		Model_object_caching_tmp++;
		//	mprintf(( "Model cache level bumped up to %d\n", Model_object_caching ));
	}

	int tmp_detail_level = Model_object_caching_tmp + Mc_detail_add[Detail.object_caching];

	if ( tmp_detail_level < 0 )	{
		tmp_detail_level = 0;
	} else if (tmp_detail_level > MAX_DETAIL_LEVEL )  {
		tmp_detail_level = MAX_DETAIL_LEVEL;
	}

	if ( tmp_detail_level > 3 )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	
//	static int last_one = -1;
//	if ( last_one != tmp_detail_level )	{
//		last_one = tmp_detail_level;
//		mprintf(( "Detail level %d\n", tmp_detail_level ));
//	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		mc->cached_valid = 0;
//		model_really_render(model_num, orient, pos, flags, objnum );
//		return;
//	}


//	mprintf(( "Rendering cache model\n" ));

	polymodel *pm = model_get(model_num);
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };
	float cx, cy, cr;
	vertex pt;
	ubyte ccflags;

	matrix tempm, tempm2;
	angles new_angles;

	vm_copy_transpose_matrix(&tempm2,orient);
	vm_matrix_x_matrix(&tempm,&tempm2,&Eye_matrix);
	vm_extract_angles_matrix(&new_angles, &tempm );
	
	if ( !model_cache_calc_coords(pos,pm->rad, &cx, &cy, &cr) )	{
		// Not onscreen, do a real render and exit
		mc->cached_valid = 0;
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	//================================================================
	// A bunch of checks to see if we need to redraw the model or not

	
	vec3d ship_to_eye;

	vm_vec_sub( &ship_to_eye, &Eye_position, pos );
	vm_vec_normalize_safe(&ship_to_eye);
	float this_dot = vm_vec_dot( &ship_to_eye, &orient->fvec );
	this_dot += vm_vec_dot( &ship_to_eye, &orient->rvec );

	float diff = 0.0f;
	
	if ( !mc->cached_valid )	{
		// Nothing cached
		goto RedrawIt;
	}

	Assert( mc->data != NULL );

	if (Framecount - mc->last_frame_rendered > 1 + 2*(MAX_DETAIL_LEVEL - Detail.object_caching - 1)) {
		goto RedrawIt;
	}

	diff = fl_abs( this_dot - mc->last_dot );

	if ( diff > Mc_viewer_pos_factor[tmp_detail_level] )	{
//		mprintf(( "Redraw!!! %.4f\n", diff ));
		goto RedrawIt;
	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		goto RedrawIt;
//	}

	if (tmp_detail_level > 2)	{
		if ( mc->thrust_glow_bitmap != Interp_thrust_glow_bitmap )	{
			// Engline glow bitmap changed
			//	mprintf(( "MC: Glow bitmap changed! %d -> %d\n", mc->thrust_glow_bitmap, Interp_thrust_glow_bitmap ));
			goto RedrawIt;
		}
	}

	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f ) {
			float diff = fl_abs( mc->thrust_scale - Interp_thrust_scale );

			if ( diff > 0.1f )	{
				// Thruster size has changed
				//mprintf(( "MC: Thruster size changed! %.2f -> %.2f\n", mc->thrust_scale, Interp_thrust_scale ));
				goto RedrawIt;
			}
		}
	}

//		if (0) {
//			float diff = fl_abs( mc->thrust_glow_noise - Interp_thrust_glow_noise );

//			if ( diff > 0.1f )	{
				// Glow noise has changed
				//mprintf(( "MC: Thruster glow changed! %.2f -> %.2f\n", mc->thrust_glow_noise, Interp_thrust_glow_noise ));
//				goto RedrawIt;
//			}
//		}


	if ( mc->model_num != model_num )	{
		// Model changed
		goto RedrawIt;
	}

	if ( cr>mc->cr*Mc_size_factor[tmp_detail_level] )	{
		// Scaling up too far
		goto RedrawIt;
	}
		
	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f )	{
			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if (mc->num_lights != num_lights)	{
					// Lighting changed
					goto RedrawIt;
				}
			}
		}
	}

		// This method is correct, but rotating ship makes things redraw which is too slow.
	#if 0
		if ( cr > 4.0f )	{
			// Check orientation
			float angle_error = MAX( fl_abs( mc->angs.p-new_angles.p ),fl_abs( mc->angs.h-new_angles.h ));

			// Exact
			//if ( angle_error > 0.075f  )	{	

			// Rough
			if ( angle_error > 0.40f  )	{	
				// Ship/view turned too much
				//mprintf(( "Ship/view turned too much %.4f\n", angle_error ));

				goto RedrawIt;
			}
		}
	#endif


//		mprintf(( "Dot = %.5f\n", dot ));

#if 0
	if (0) {
		float dx, dy, dz;

		dx = vm_vec_dot( &orient->rvec, &mc->orient.rvec )+1.0f;
		dy = vm_vec_dot( &orient->uvec, &mc->orient.uvec )+1.0f;
		dz = vm_vec_dot( &orient->fvec, &mc->orient.fvec )+1.0f;
			
		float angle_error = (dx+dy+dz)*1000.0f/6.0f;		

		//mprintf(( "Angle_error = %.4f\n", angle_error ));

		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 0.0f
		
		float cmp_val = 999.75f;		// old
//			if ( is_asteroid )	{
//				cmp_val = scale_it( 0.0f, 999.75f, cr, 0.0f, 64.0f );
//			}
											
		if ( angle_error < cmp_val ) {
			// Ship turned too much
			goto RedrawIt;
		}
	}	
#endif


	// Have a valid cache entry, mc
	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags )	{
		// offscreen		
		goto RedrawIt;
	}

	if ( model_get_rotated_bitmap_points(&pt,mc->angs.b - new_angles.b, pm->rad, v ))	{
		// offscreen		
		goto RedrawIt;
	}


	gr_set_bitmap( mc->bitmap_id );

	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	//	if ( keyd_pressed[KEY_LSHIFT] )	{
	//	gr_set_color( 255, 0, 0 );
	//	gr_pixel( fl2i(v[0].sx), fl2i(v[0].sy) );
	//	}

	//if ( keyd_pressed[KEY_RSHIFT] )	{
	//	gr_line( fl2i(v[0].sx), fl2i(v[0].sy), fl2i(v[1].sx), fl2i(v[1].sy) );
	//	gr_line( fl2i(v[1].sx), fl2i(v[1].sy), fl2i(v[2].sx), fl2i(v[2].sy) );
	//	gr_line( fl2i(v[2].sx), fl2i(v[2].sy), fl2i(v[3].sx), fl2i(v[3].sy) );
	//	gr_line( fl2i(v[3].sx), fl2i(v[3].sy), fl2i(v[0].sx), fl2i(v[0].sy) );
	//}


	return;


	//==========================================================
	// Cache is bad for model, so draw it and save it
RedrawIt:


//	if ( mc->data != NULL )	{
//		vm_free(mc->data);
//		mc->data = NULL;
//	}

	if ( mc->bitmap_id != -1 )	{
		bm_release(mc->bitmap_id);
		mc->bitmap_id = -1;
	}

	mc->cached_valid = 0;
	mc->model_num = model_num;
	mc->pos = *pos;
	//mc->orient = *orient;
	mc->cr = cr;
	mc->angs = new_angles;	//-Physics_viewer_bank;

	mc->thrust_scale = Interp_thrust_scale;
	mc->thrust_bitmap = Interp_thrust_bitmap;
	mc->thrust_glow_bitmap = Interp_thrust_glow_bitmap;
	mc->thrust_glow_noise = Interp_thrust_glow_noise;

	mc->last_dot = this_dot;

	if ( cr > MODEL_MAX_BITMAP_SIZE/2-1 )	
		goto JustDrawIt;

	//Physics_viewer_bank

	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags ) {
		goto JustDrawIt;
	}

	model_get_rotated_bitmap_points(&pt,0.0f, pm->rad, v );
				
	int x1, y1, x2, y2, w, h;

	x1 = fl_round_2048( v[0].sx );
	y1 = fl_round_2048( v[0].sy );

	x2 = fl_round_2048( v[2].sx );	//+0.5f );
	y2 = fl_round_2048( v[2].sy );	//+0.5f );

	if ( x1 < gr_screen.clip_left)	
		goto JustDrawIt;
	
	if ( y1 < gr_screen.clip_top )
		goto JustDrawIt;
	
	if ( x2 > gr_screen.clip_right)
		goto JustDrawIt;

	if ( y2 > gr_screen.clip_bottom) 
		goto JustDrawIt;

	w = x2 - x1 + 1;	
	if ( w < 0 )
		Int3();

	if ( w < 2 ) 
		w = 2;

	h = y2 - y1 + 1;	

	if ( h < 0 )
		Int3();

	if ( h < 2 ) 
		h = 2;

	if ( w > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;
		
	if ( h > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;

	mc->w = w;
	mc->h = h;

//	mprintf(( "Mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		mc->data = (ubyte *)vm_malloc( MODEL_MAX_BITMAP_SIZE * MODEL_MAX_BITMAP_SIZE );
	}

//	mprintf(( "Done mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		goto JustDrawIt;
	}
	for (i = 0; i < w*h; i++)	{
		mc->data[i] = 255;
	}


	mc->bitmap_id = bm_create( 8, mc->w, mc->h, mc->data, 0 );

	if ( mc->bitmap_id < 0 )	{
		goto JustDrawIt;
	}

	// Save stars and stuff on screen
	mc_get_bmp( tmp_bitmap, x1, y1, w, h );

	mc->num_lights = num_lights;

	// Didn't render a cached one... so render it and then save it in the cache

	// Turn on stippling
	model_really_render(model_num, orient, pos, flags, objnum );

	// Save screen to bitmap 
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 1;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	// Restore stars and stuff to screen
	mc_put_bmp( tmp_bitmap, x1, y1, w, h );

	// Draw the model
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	mc->cached_valid = 1;
	mc->last_frame_rendered = Framecount;
	return;
	
JustDrawIt:

	// Too big to save
	model_really_render(model_num, orient, pos, flags, objnum );
	*/
}

// Find the distance from p0 to the closest point on a box.
// The box's dimensions from 'min' to 'max'.
float interp_closest_dist_to_box( vec3d *hitpt, vec3d *p0, vec3d *min, vec3d *max )
{
	float *origin = (float *)&p0->xyz.x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	float *coord = (float *)&hitpt->xyz.x;
	int inside = 1;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] )	{
			coord[i] = minB[i];
			inside = 0;
		} else if (origin[i] > maxB[i] )	{
			coord[i] = maxB[i];
			inside = 0;
		} else {
			coord[i] = origin[i];
		}
	}
	
	if ( inside )	{
		return 0.0f;
	}

	return vm_vec_dist(hitpt,p0);
}


// Finds the closest point on a model to a point in space.  Actually only finds a point
// on the bounding box of the model.    
// Given:
//   model_num      Which model
//   orient         Orientation of the model
//   pos            Position of the model
//   eye_pos        Point that you want to find the closest point to
// Returns:
//   distance from eye_pos to closest_point.  0 means eye_pos is 
//   on or inside the bounding box.
//   Also fills in outpnt with the actual closest point.
float model_find_closest_point( vec3d *outpnt, int model_num, int submodel_num, matrix *orient, vec3d * pos, vec3d *eye_pos )
{
	vec3d closest_pos, tempv, eye_rel_pos;
	
	polymodel *pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		 submodel_num = pm->detail[0];
	}

	// Rotate eye pos into object coordinates
	vm_vec_sub(&tempv,pos,eye_pos );
	vm_vec_rotate(&eye_rel_pos,&tempv,orient );

	return interp_closest_dist_to_box( &closest_pos, &eye_rel_pos, &pm->submodel[submodel_num].min, &pm->submodel[submodel_num].max );
}

int tiling = 1;
DCF(tiling, "")
{
	tiling = !tiling;
	if(tiling){
		dc_printf("Tiled textures\n");
	} else {
		dc_printf("Non-tiled textures\n");
	}
}

void moldel_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add, vec3d *Eyeposition )
{
	vec3d uvec, rvec;
	vec3d temp;

	temp = *pos;

//	vm_vec_sub( &rvec, &View, &temp );
	vm_vec_sub( &rvec, Eyeposition, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	

//	Int3();
}

geometry_batcher primary_thruster_batcher, secondary_thruster_batcher, tertiary_thruster_batcher;
 
void light_set_all_relevent();

#define mSTUFF_VERTICES()	do { verts[0]->u = 0.0f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.0f; verts[3]->v = 1.0f; } while(0);
#define mR_VERTICES()		do { g3_rotate_vertex(verts[0], &bottom1); g3_rotate_vertex(verts[1], &bottom2);	g3_rotate_vertex(verts[2], &top2); g3_rotate_vertex(verts[3], &top1); } while(0);
#define mP_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);

extern int Warp_model;

void model_really_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum )
{
	int i, detail_level;
	polymodel * pm;
	glow_maps_active = 1;

	uint save_gr_zbuffering_mode;
	int zbuf_mode;
	ship *shipp = NULL;
	int is_ship = 0;
	object *objp = NULL;

	if (objnum >= 0) {
		objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];
			is_ship = 1;
			glow_maps_active = shipp->glowmaps_active;
		}
	}
	
	if (FULLCLOAK != 1)
		model_interp_sortnorm = model_interp_sortnorm_b2f;


	MONITOR_INC( NumModelsRend, 1 );	

	Interp_orient = orient;
	Interp_pos = pos;
	memset(&Interp_offset,0,sizeof(vec3d));

	int tmp_detail_level = Game_detail_level;
	
//	if ( D3D_enabled )	{
//		tmp_detail_level = -1;		// Force no hires models for Direct3D
//	}

	//	Tmap_show_layers = 1;
//	model_set_detail_level(0);
//	flags |= MR_LOCK_DETAIL|MR_NO_TEXTURING|MR_NO_LIGHTING;		//MR_LOCK_DETAIL |	|MR_NO_LIGHTING|MR_NO_SMOOTHINGMR_NO_TEXTURING | 

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	if ( !Model_polys )	{
		flags |= MR_NO_POLYS;
	}

	Interp_flags = flags;			 

	pm = model_get(model_num);	

	// Set the flags we will pass to the tmapper
	Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode, fog everything except for the warp holes and other non-fogged models
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !(flags & MR_NO_FOGGING)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}


	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	save_gr_zbuffering_mode = gr_zbuffering_mode;
	zbuf_mode = gr_zbuffering_mode;

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( pos, pm->rad );
	//	if(!Cmdline_nohtl)gr_set_lighting(false,false);
		return;
	}

	bool is_outlines_only = !Cmdline_nohtl && (flags & MR_NO_POLYS) && ((flags & MR_SHOW_OUTLINE_PRESET) || (flags & MR_SHOW_OUTLINE));  
	bool use_api = (!is_outlines_only || (gr_screen.mode == GR_DIRECT3D)) || (gr_screen.mode == GR_OPENGL);


	g3_start_instance_matrix(pos,orient, use_api);

	if ( Interp_flags & MR_SHOW_RADIUS )	{
		if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
			gr_set_color(0,64,0);
			g3_draw_sphere_ez(&vmd_zero_vector,pm->rad);
		}
	}

	Assert( pm->n_detail_levels < MAX_MODEL_DETAIL_LEVELS );

	vec3d closest_pos;
	float depth = model_find_closest_point( &closest_pos, model_num, -1, orient, pos, &Eye_position );
	if ( pm->n_detail_levels > 1 )	{

		if ( Interp_flags & MR_LOCK_DETAIL )	{
			// RT, why use lower texture for just one ship, means we have to cache in another texture.
			// If the ship is in target the model and its texture are already loaded and cached
			i = Interp_detail_level;//+1;
		} else {

			//gr_set_color(0,128,0);
			//g3_draw_sphere_ez( &closest_pos, 2.0f );

			#if MAX_DETAIL_LEVEL != 4
			#error Code in modelInterp.cpp assumes MAX_DETAIL_LEVEL == 4
			#endif

			switch( Detail.detail_distance )	{
			case 0:		// lowest
				depth *= 8.0f;
				break;
			case 1:		// lower than normal
				depth *= 4.0f; 
				break;
			case 2:		// default  (leave the same)
				break;
			case 3:		// above normal
				depth /= 4.0f;
				break;
			case 4:		// even more normal
				depth /= 8.0f;
				break;
			}

			// nebula ?
			if(The_mission.flags & MISSION_FLAG_FULLNEB){
				depth *= neb2_get_lod_scale(Interp_objnum);
			}

			for ( i=0; i<pm->n_detail_levels; i++ )	{
				if ( depth<=pm->detail_depth[i] ){
					break;
				}
			}

			// If no valid detail depths specified, use highest.
			if ( (i > 1) && (pm->detail_depth[i-1] < 1.0f))	{
				i = 1;
			}
		}


		// maybe force lower detail
		if (Interp_flags & MR_FORCE_LOWER_DETAIL) {
			i++;
		}

		//detail_level = fl2i(depth/10.0f);		
		//detail_level = 0;
		model_current_LOD = detail_level = i-1-tmp_detail_level;

		if ( detail_level < 0 ) 
			detail_level = 0;
		else if (detail_level >= pm->n_detail_levels ) 
			detail_level = pm->n_detail_levels-1;

		//mprintf(( "Depth = %.2f, detail = %d\n", depth, detail_level ));

	} else {
		model_current_LOD = detail_level = 0;
	}

#ifndef NDEBUG
	if ( detail_level==0 )	{
		MONITOR_INC( NumHiModelsRend, 1 );	
	} else if ( detail_level ==pm->n_detail_levels-1 )	{
		MONITOR_INC( NumLowModelsRend, 1 );	
	}  else {
		MONITOR_INC( NumMedModelsRend, 1 );	
	}
#endif	

	if((Interp_flags & MR_AUTOCENTER) && (pm->flags & PM_FLAG_AUTOCEN)){
		vec3d auto_back = pm->autocenter;				
		vm_vec_scale(&auto_back, -1.0f);		
		g3_start_instance_matrix(&auto_back, NULL, true);		
	}	

	gr_zbias(1);

	if(!Cmdline_nohtl) {
		light_set_all_relevent();
	}

	if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
	{
		float fog_near = 10.0f, fog_far = 1000.0f;
		neb2_get_fog_values(&fog_near, &fog_far, objp);
		unsigned char r, g, b;
		neb2_get_fog_colour(&r, &g, &b);
		gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
	}

	// Draw the subobjects	
	i = pm->submodel[pm->detail[detail_level]].first_child;

	//gr_set_fill_mode((is_outlines_only == true)?GR_FILL_MODE_WIRE:GR_FILL_MODE_SOLID);
	while( i >= 0 )	{
		if (!pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_FULL;


			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}
			gr_zbuffer_set(zbuf_mode);
			// When in htl mode render with htl method unless its a jump node
			if(!Cmdline_nohtl && !is_outlines_only ){
				model_render_childeren_buffers(&pm->submodel[i], pm, i, detail_level);
			}
			else {
				model_interp_subcall( pm, i, detail_level );
			}
		} 
		i = pm->submodel[i].next_sibling;
	}	

	// rotate lights for the hull
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}else {
		// This NULL func ptr was causing a crash in non htl
		if(!Cmdline_nohtl) {
			gr_reset_lighting();
		}
	}

	if ( pm->submodel[pm->detail[detail_level]].num_children > 0 ){
//		zbuf_mode |= GR_ZBUFF_WRITE;		// write only
		zbuf_mode = GR_ZBUFF_FULL;
	}

	// no zbuffering
	if(Interp_flags & MR_NO_ZBUFFER){
		zbuf_mode = GR_ZBUFF_NONE;
	}
	
	gr_zbuffer_set(zbuf_mode);
	gr_zbias(0);	

	model_radius = pm->submodel[pm->detail[detail_level]].rad;

	//*************************** draw the hull of the ship *********************************************

	// When in htl mode render with htl method unless its a jump node
	if(!Cmdline_nohtl && !is_outlines_only){
		model_render_buffers(&pm->submodel[pm->detail[detail_level]], pm);
//		model_render_childeren_buffers(&pm->submodel[pm->detail[detail_level]], pm, pm->detail[detail_level], detail_level);
	}
	else {
		model_interp_subcall(pm,pm->detail[detail_level],detail_level);
	}
	gr_set_fill_mode(GR_FILL_MODE_SOLID);


	if(!Cmdline_nohtl)gr_set_lighting(true,true);

	if (objp != NULL) {
		vec3d decal_z_corection = View_position;
	//	vm_vec_sub(&decal_z_corection, &Eye_position, pos);
		if(decal_z_corection.xyz.x != 0 && decal_z_corection.xyz.y != 0 && decal_z_corection.xyz.z != 0)
			vm_vec_normalize(&decal_z_corection);
		float corection = 0.2f;
		vm_vec_scale(&decal_z_corection, corection);
		g3_start_instance_matrix(&decal_z_corection, NULL, use_api);		
		decal_render_all(objp);
		g3_done_instance(use_api);
	}


	if(!Cmdline_nohtl){
		gr_reset_lighting();
		gr_set_lighting(false,false);
	}

	if (Interp_flags & MR_SHOW_PIVOTS )	{
		model_draw_debug_points( pm, NULL );
		model_draw_debug_points( pm, &pm->submodel[pm->detail[detail_level]] );

		if(pm->flags & PM_FLAG_AUTOCEN){
			gr_set_color(255, 255, 255);
			g3_draw_sphere_ez(&pm->autocenter, pm->rad / 4.5f);
		}
	}

	model_radius = 0.0f;

	// render model insignias
	gr_zbias(1);

	if (!Cmdline_nohtl)	gr_set_texture_panning(0.0, 0.0, false);

	gr_zbuffer_set(GR_ZBUFF_READ);
	model_render_insignias(pm, detail_level);	

	gr_zbias(0);  

	gr_set_lighting(false,false);

	if (FULLCLOAK != -1)	model_finish_cloak(FULLCLOAK);

	if ( pm->submodel[pm->detail[0]].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[pm->detail[0]]);
	}	

	if ( Interp_flags & MR_SHOW_SHIELDS )	{
		model_render_shields(pm);
	}	
			
//start rendering glow points -Bobboau

		if ( (pm->n_glows) /*&& (Interp_flags & MR_SHOW_THRUSTERS) && (Detail.engine_glows)*/ )	{



		for (i = 0; i < pm->n_glows; i++ ) {
			glow_bank *bank = &pm->glows[i];
			int j;
			if(bank->is_on)
			{
				if(is_ship && i<32)
					if(!(shipp->glows_active & (1 << i)))
						continue;

				for ( j=0; j<bank->num_slots; j++ )	{
					
					int flick;
						if(pm->submodel[pm->detail[0]].num_arcs){
						flick = static_rand(timestamp()%20)%(pm->submodel[pm->detail[0]].num_arcs + j); //the more damage, the more arcs, the more likely the lights will fail
						}else{
						flick = 1;
						}
 
					if(flick == 1){
						glow_point *gpt = &bank->point[j];
						vec3d pnt = gpt->pnt;
						vec3d norm = gpt->norm;
					
						if(bank->submodel_parent > 0){//this is were it rotates for the submodel parent-Bobboau
							matrix m;
							if(pm->submodel[bank->submodel_parent].blown_off)continue;

							angles angs = pm->submodel[bank->submodel_parent].angs;
							angs.b = PI2 - angs.b;
							angs.p = PI2 - angs.p;
							angs.h = PI2 - angs.h;

//								gr_set_color_fast(&Color_bright_blue);
//								gr_printf(0, 10, "b %0.2f, p %0.2f, h %0.2f", angs.b, angs.p, angs.h);

							vm_angles_2_matrix(&m, &angs);

							vec3d offset = pm->submodel[bank->submodel_parent].offset;
							vm_vec_sub(&pnt, &pnt, &offset);
							vec3d p = pnt;
							vec3d n = norm;
							vm_vec_rotate(&pnt, &p, &m);
							vm_vec_rotate(&norm, &n, &m);
							vm_vec_add2(&pnt, &offset);
						}
							vec3d tempv;
						switch(bank->type){
						case 0:
							float d;
							if((bank->point[j].norm.xyz.x == 0.0f) && (bank->point[j].norm.xyz.z == 0.0f) && (bank->point[j].norm.xyz.z == 0.0f)){
								d=1.0f;	//if given a nul vector then always show it
							}else{
								vm_vec_sub(&tempv,&View_position,&pnt);
								vm_vec_normalize(&tempv);

								d = vm_vec_dot(&tempv,&norm);
								d -= 0.25;	
							}
						
							if ( d > 0.0f)	{
								vertex p;					
		
								d *= 3.0f;
								if ( d > 1.0f ) d = 1.0f;
	
								float w = bank->point[j].radius;
		
								// fade them in the nebula as well
								if(The_mission.flags & MISSION_FLAG_FULLNEB){
									vec3d npnt;
									vm_vec_add(&npnt, &pnt, pos);
									d *= (1.0f - neb2_get_fog_intensity(&npnt));
									w *= 1.5;	//make it bigger in a nebula
								}
					
								// disable fogging
								//if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)gr_fog_set(GR_FOGMODE_FOG, 0, 0, 0);
								if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
		
								vertex pt;
								g3_rotate_vertex( &p, &pnt );
								if(!Cmdline_nohtl) g3_transfer_vertex(&pt, &pnt);
								else pt = p;
								
	
							//	if(bank->submodel_parent)
//								{
//								}

								gr_set_bitmap( bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d );
							//	mprintf(( "rendering glow with texture %d\n", bank->glow_bitmap ));
								if(Cmdline_nohtl)g3_draw_bitmap(&pt,0,w*0.5f, TMAP_FLAG_TEXTURED );
								else g3_draw_bitmap(&pt,0,w*0.5f, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT, w );
								//g3_draw_rotated_bitmap(&p,0.0f,w,w, TMAP_FLAG_TEXTURED );

							}//d>0
							break;
						case 1:
								vertex h1[4];				// halves of a beam section	
								vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };	
								vec3d fvec, top1, bottom1, top2, bottom2, start, end;

								vm_vec_add2(&norm, &pnt);

								vm_vec_rotate(&start, &pnt, orient);
								vm_vec_rotate(&end, &norm, orient);
								vm_vec_sub(&fvec, &end, &start);

								vm_vec_normalize(&fvec);

								moldel_calc_facing_pts(&top1, &bottom1, &fvec, &pnt, bank->point[j].radius, 1.0f, &View_position);	
								moldel_calc_facing_pts(&top2, &bottom2, &fvec, &norm, bank->point[j].radius, 1.0f, &View_position);	

								int idx = 0;
 
/*R_VERTICES()*/		if(Cmdline_nohtl){ 
							g3_rotate_vertex(verts[0], &bottom1); 
							g3_rotate_vertex(verts[1], &bottom2);	
							g3_rotate_vertex(verts[2], &top2); 
							g3_rotate_vertex(verts[3], &top1); 
						}else{
							g3_transfer_vertex(verts[0], &bottom1); 
							g3_transfer_vertex(verts[1], &bottom2);	
							g3_transfer_vertex(verts[2], &top2); 
							g3_transfer_vertex(verts[3], &top1);
						}
/*P_VERTICES()*/		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);
						verts[0]->u = 0.0f; verts[0]->v = 0.0f;	
						verts[1]->u = 1.0f; verts[1]->v = 0.0f; 
						verts[2]->u = 1.0f;	verts[2]->v = 1.0f; 
						verts[3]->u = 0.0f; verts[3]->v = 1.0f; 
//								mR_VERTICES();																// rotate and project the vertices
//								mP_VERTICES();						
//								mSTUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

								vm_vec_sub(&tempv,&View_position,&pnt);
								vm_vec_normalize(&tempv);
								

							gr_set_cull(0);
							if(The_mission.flags & MISSION_FLAG_FULLNEB){
								gr_set_bitmap(bank->glow_neb_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							}else{
								gr_set_bitmap(bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							}
								g3_draw_poly( 4, verts, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT); // added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau
							gr_set_cull(1);
						}
					}//flick
				}//for slot
			}//bank is on
		}//for bank

	}//any glow banks
				
//	gr_printf((200), (20), "x %0.2f, y %0.2f, z %0.2f", Eye_position.xyz.x, Eye_position.xyz.y, Eye_position.xyz.z);

//end rendering glow points

	// Draw the thruster glow
	if ( (Interp_thrust_glow_bitmap != -1) && (Interp_flags & MR_SHOW_THRUSTERS) /*&& (Detail.engine_glows)*/ )	{
		int i;
		int n_q = 0;;
		for (i = 0; i < pm->n_thrusters; i++ ) {
			thruster_bank *bank = &pm->thrusters[i];
		//	for ( int j=0; j<bank->num_slots; j++ )
				n_q+=bank->num_slots;
		}
		primary_thruster_batcher.allocate(n_q);
		secondary_thruster_batcher.allocate(n_q);
		tertiary_thruster_batcher.allocate(n_q);
		//this is used for the secondary thruster glows 
		//it only needs to be calculated once so I'm doing it here -Bobboau
				vec3d norm /*= bank->norm[j]*/;
				norm.xyz.z = -1.0f;
				norm.xyz.x = 1.0f;
				norm.xyz.y = -1.0f;

				norm.xyz.x *= controle_rotval.xyz.y/2;
				norm.xyz.y *= controle_rotval.xyz.x/2;

				vm_vec_normalize(&norm);

		for (i = 0; i < pm->n_thrusters; i++ ) {
			thruster_bank *bank = &pm->thrusters[i];
			int j;

			// don't draw this thruster if the engine is destroyed or just not on
			if ((bank->obj_num > -1) && !(model_should_render_engine_glow(objnum, bank->obj_num)))
				continue;

			for ( j=0; j<bank->num_slots; j++ )	{
				float d, D;
				vec3d tempv;
				vm_vec_sub(&tempv,&View_position,&bank->point[j].pnt);
				vm_vec_normalize(&tempv);

				D = d = vm_vec_dot(&tempv,&bank->point[j].norm);

					//ADAM: Min throttle draws rad*MIN_SCALE, max uses max.
					#define NOISE_SCALE 0.5f
					#define MIN_SCALE 3.4f
					#define MAX_SCALE 4.7f
					float scale = MIN_SCALE;
						
// the following replaces Bobboau's code, commented out below - Goober5000
					float magnitude;
					vec3d scale_vec;

					// normalize banks, in case of incredibly big normals
					vm_vec_copy_normalize(&scale_vec, &bank->point[j].norm);

					// adjust for thrust
					(scale_vec.xyz.x *= Interp_thrust_scale_x) -= 0.1f;
					(scale_vec.xyz.y *= Interp_thrust_scale_y) -= 0.1f;
					(scale_vec.xyz.z *= Interp_thrust_scale) -= 0.1f;

					// get magnitude, which we will use as the scaling reference
					magnitude = vm_vec_normalize(&scale_vec);

					// get absolute value
					if (magnitude < 0.0f)
						magnitude *= -1.0f;

					scale = magnitude*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;


				vertex p;					
				if ( d > 0.0f)	{

					// Make glow bitmap fade in/out quicker from sides.
					d *= 3.0f;
					if ( d > 1.0f ) d = 1.0f;
				}
				vec3d npnt;
				float fog_int = 1.0;
				// fade them in the nebula as well
				if(The_mission.flags & MISSION_FLAG_FULLNEB){
				//	vec3d npnt;
					vm_vec_rotate(&npnt, &bank->point[j].pnt, orient);
					vm_vec_add2(&npnt, pos);
					fog_int = (1.0f - (neb2_get_fog_intensity(&npnt)));
				//	fog_int /=10;.0f;
					d *= fog_int;
				}

					
				// disable fogging
				//if(The_mission.flags & MISSION_FLAG_FULLNEB)gr_fog_set(GR_FOGMODE_FOG, 0, 0, 0);
				if(The_mission.flags & MISSION_FLAG_FULLNEB)gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

// this is the original scaling code - Goober5000
//					scale = (Interp_thrust_scale-0.1f)*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;

					float w = bank->point[j].radius*(scale+Interp_thrust_glow_noise*NOISE_SCALE );

					vertex pt;
					g3_rotate_vertex( &p, &bank->point[j].pnt );
					if(!Cmdline_nohtl) g3_transfer_vertex(&pt, &bank->point[j].pnt);
					else pt = p;
					//these two lines are used by the tertiary glows, thus we will need to project this all of the time
				if ( d > 0.0f){
				//	gr_set_bitmap( Interp_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d );
					{
						//primary thruster glows, sort of a lens flare/engine wash thing 

//						g3_draw_bitmap(&pt,0,w*0.5f*Interp_thrust_glow_rad_factor, TMAP_FLAG_TEXTURED );
				//		if(Cmdline_nohtl)g3_draw_bitmap(&pt,0,w*0.5f*Interp_thrust_glow_rad_factor, TMAP_FLAG_TEXTURED );
				//		else g3_draw_bitmap(&pt,0,w*0.5f*Interp_thrust_glow_rad_factor, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT, w*0.325f );

						pt.r = (ubyte)(255.0f * d);
						pt.g = (ubyte)(255.0f * d);
						pt.b = (ubyte)(255.0f * d);
						pt.a = (ubyte)(255.0f * d);
						primary_thruster_batcher.draw_bitmap(&pt,w*0.5f*Interp_thrust_glow_rad_factor, w*0.325f);

						//g3_draw_rotated_bitmap(&p,0.0f,w,w, TMAP_FLAG_TEXTURED );
					}
				}//d>0

				if(Interp_tertiary_thrust_glow_bitmap > -1){
					//tertiary thruster glows, suposet to be a complement to the secondary thruster glows, it simulates the effect of an ion wake or something, 
					//thus is mostly for haveing a glow that is visable from the front
				//	gr_set_bitmap( Interp_tertiary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, fog_int );
					
					p.sw -= w;
					//g3_draw_bitmap(&p,0,w, TMAP_FLAG_TEXTURED );
				//	if(Cmdline_nohtl)g3_draw_rotated_bitmap(&pt,magnitude*4*Interp_tertiary_thrust_glow_rad_factor,w*0.6f, TMAP_FLAG_TEXTURED); 
				//	else g3_draw_rotated_bitmap(&pt,magnitude*4*Interp_tertiary_thrust_glow_rad_factor,w*0.6f, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT, -(D>0)?D:-D);

					pt.r = (ubyte)(255.0f * fog_int);
					pt.g = (ubyte)(255.0f * fog_int);
					pt.b = (ubyte)(255.0f * fog_int);
					pt.a = (ubyte)(255.0f * fog_int);
					tertiary_thruster_batcher.draw_bitmap(&pt, w*0.6f, magnitude*4*Interp_tertiary_thrust_glow_rad_factor, -(D>0)?D:-D);
				}
				

/*begin secondary glows*/
				//secondary thruster glows, they are based on the beam rendering code
				//they are suposed to simulate... an ion wake... or... something
				//ok, how's this there suposed to look cool! hows that, 
				//it that scientific enough for you!! you anti-asthetic basturds!!!
				///AAAHHhhhh!!!!
				vec3d pnt = bank->point[j].pnt;

				scale = magnitude*(MAX_SCALE-(MIN_SCALE/2))+(MIN_SCALE/2);
																				    
			//	vertex h1[4];				// halves of a beam section	
			//	vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };	
				vec3d fvec, norm2;
			//	vec3d top1, bottom1, top2, bottom2;

				d = vm_vec_dot(&tempv,&norm);
				d += 0.75f;
				d *=3;
				if(d> 1.0f)d = 1.0f;
				if(d > 0)
				{
					vm_vec_add(&norm2, &norm, &pnt);

			//	vm_vec_rotate(&start, &pnt, orient);
			//	vm_vec_rotate(&end, &norm, orient);
			//	vm_vec_sub(&fvec, &end, &start);
					vm_vec_sub(&fvec, &norm2, &pnt);

					vm_vec_normalize(&fvec);

					float w = bank->point[j].radius*scale*2;

			//		vm_vec_scale_add(&pnt, &pnt, &fvec, -0.25f * w*2*);
			//	vm_vec_scale_add(&norm, &pnt, &fvec, 0.75f);

					vm_vec_scale_add(&norm2, &pnt, &fvec, w*2*Interp_secondary_thrust_glow_len_factor);

/*				vec3d eyepos;
				eyepos = Eye_position;
				vm_vec_add2(&eyepos, pos);
*/
/*					moldel_calc_facing_pts(&top1, &bottom1, &fvec, &pnt, w*Interp_secondary_thrust_glow_rad_factor, 1.0f, &View_position);	
					moldel_calc_facing_pts(&top2, &bottom2, &fvec, &norm2, w*Interp_secondary_thrust_glow_rad_factor, 1.0f, &View_position);	
	
					int idx = 0;



					if(Cmdline_nohtl) {
						g3_rotate_vertex(verts[0], &bottom1); 
						g3_rotate_vertex(verts[1], &bottom2);	
						g3_rotate_vertex(verts[2], &top2); 
						g3_rotate_vertex(verts[3], &top1); 	  
					}else{
						g3_transfer_vertex(verts[0], &bottom1); 
						g3_transfer_vertex(verts[1], &bottom2);
						g3_transfer_vertex(verts[2], &top2);    
						g3_transfer_vertex(verts[3], &top1); 	  
					}

					for(idx=0; idx<4; idx++) g3_project_vertex(verts[idx]);
					verts[0]->u = 0.0f; verts[0]->v = 0.0f;	
					verts[1]->u = 1.0f; verts[1]->v = 0.0f; 
					verts[2]->u = 1.0f;	verts[2]->v = 1.0f; 
					verts[3]->u = 0.0f; verts[3]->v = 1.0f; 

					vm_vec_sub(&tempv,&View_position,&pnt);
					vm_vec_normalize(&tempv);

*/					if(The_mission.flags & MISSION_FLAG_FULLNEB){
						vec3d npnt;
						vm_vec_add(&npnt, &pnt, pos);
						d *= fog_int;
					}

/*					gr_set_cull(0);
					gr_set_bitmap(Interp_secondary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d);		
					g3_draw_poly( 4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
					gr_set_cull(1);
				*/
					secondary_thruster_batcher.draw_beam(&pnt, &norm2, w*Interp_secondary_thrust_glow_rad_factor*0.5f, d);
				}

//end secondary glows
//begin particles
				if(is_ship){
					ship_info* sip = &Ship_info[shipp->ship_info_index];
/*					particle_emitter pe;
					float v = vm_vec_mag_quick(&Objects[shipp->objnum].phys_info.desired_vel);
					pe.max_life = v;
					pe.max_rad = bank->radius[j]*0.75f;
					pe.max_vel = v;
					pe.min_life = v*0.5f;
					pe.min_rad = bank->radius[j]*0.5f;
					pe.min_vel = v*0.75f;
					pe.normal = bank->norm[j];
					pe.normal_variance = 0.2f;
					pe.num_high = 5;
					pe.num_low = 1;
						vec3d npnt;
						vm_vec_add(&npnt, &pnt, pos);
					pe.pos = npnt;
					pe.vel = Objects[shipp->objnum].phys_info.desired_vel;
					particle_emit( &pe, PARTICLE_BITMAP, Interp_thruster_particle_bitmap );
					*/
					particle_emitter	pe;
					thruster_particles* tp;

					for(int P = 0; P < sip->n_thruster_particles; P++){
						if(Interp_AB)
							tp = &sip->afterburner_thruster_particles[P];
						else
							tp = &sip->normal_thruster_particles[P];

						float v = vm_vec_mag_quick(&Objects[shipp->objnum].phys_info.desired_vel);
						vec3d npnt = pnt;
						vm_vec_unrotate(&npnt, &pnt, orient);
						vm_vec_add(&npnt, &npnt, pos);

						pe.pos = npnt;				// Where the particles emit from
						pe.vel = Objects[shipp->objnum].phys_info.desired_vel;	// Initial velocity of all the particles
	
						vec3d nn = orient->vec.fvec;
						vm_vec_negate(&nn);
					//	vm_vec_unrotate(&nn, &bank->norm[j], orient);

						pe.normal = nn;	// What normal the particle emit around
						pe.min_vel = v*0.75f;
						pe.max_vel =  v*1.25f;

						pe.num_low = tp->n_low;					// Lowest number of particles to create
						pe.num_high = tp->n_high;				// Highest number of particles to create
						pe.min_rad = bank->point[j].radius*tp->min_rad;	// * objp->radius;
						pe.max_rad = bank->point[j].radius*tp->max_rad; // * objp->radius;
						pe.normal_variance = tp->variance;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree

						particle_emit( &pe, PARTICLE_BITMAP, tp->thruster_particle_bitmap01);
					}

					// do sound - maybe start a random sound, if it has played far enough.
				}
			}
		}
		gr_set_bitmap( Interp_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
		primary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
		gr_set_bitmap(Interp_secondary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
		secondary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
		gr_set_bitmap( Interp_tertiary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
		tertiary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	}
/*
	Interp_thrust_glow_rad_factor = trf1;
	Interp_secondary_thrust_glow_rad_factor = trf2;
	Interp_secondary_thrust_glow_len_factor = tlf;
	Interp_tertiary_thrust_glow_rad_factor = trf3;
*/
//	Interp_thrust_glow_bitmap = -1;	
//	Interp_secondary_thrust_glow_bitmap = -1;
//	Interp_tertiary_thrust_glow_bitmap = -1;
	vm_vec_zero(&controle_rotval);

	gr_set_cull(0);	

	// Draw the thruster subobjects	
	gr_set_fill_mode((is_outlines_only == true)?GR_FILL_MODE_WIRE:GR_FILL_MODE_SOLID);
	i = pm->submodel[pm->detail[detail_level]].first_child;
	while( i >= 0 )	{
		if (pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_READ;

			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}

			gr_zbuffer_set(zbuf_mode);

			if(!Cmdline_nohtl) {
				model_render_childeren_buffers(&pm->submodel[i], pm, i, detail_level);
			} else {
				model_interp_subcall( pm, i, detail_level );
			}
		}
		i = pm->submodel[i].next_sibling;
	}	
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_set_cull(1);	

	if ( Interp_flags & MR_SHOW_PATHS ){
		if (Cmdline_nohtl) model_draw_paths(model_num);
		else model_draw_paths_htl(model_num);
	}

	if (Interp_flags & MR_BAY_PATHS ){
		if (Cmdline_nohtl) model_draw_bay_paths(model_num);
		else model_draw_bay_paths_htl(model_num);
	}

	if((Interp_flags & MR_AUTOCENTER) && (pm->flags & PM_FLAG_AUTOCEN)){
		g3_done_instance(use_api);
	}

//	if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

	g3_done_instance(use_api);
	
	gr_zbuffer_set(save_gr_zbuffering_mode);
	
	glow_maps_active = 1;
	memset(&Interp_offset,0,sizeof(vec3d));

}


void submodel_render(int model_num, int submodel_num, matrix *orient, vec3d * pos, uint flags, int objnum, int *replacement_textures)
{
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel * pm;

	MONITOR_INC( NumModelsRend, 1 );	

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	return;

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	Interp_flags = flags;
	Interp_pos=pos;
	
	pm = model_get(model_num);

	// Set the flags we will pass to the tmapper
	Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}

	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling )
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_filter_push( -1, pos, pm->submodel[submodel_num].rad );
	}

	//set to true since D3d and OGL need the api matrices set
	g3_start_instance_matrix(pos,orient, true);

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	// fixes disappearing HUD in OGL - taylor
	gr_set_cull(1);

	if(!Cmdline_nohtl) {

		// RT - Put this here to fog debris
		if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
		{
			float fog_near, fog_far;
			object *obj = NULL;
			
			if (objnum >= 0)
				obj = &Objects[objnum];

			neb2_get_fog_values(&fog_near, &fog_far, obj);
			unsigned char r, g, b;
			neb2_get_fog_colour(&r, &g, &b);
			gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
		}

		model_render_buffers(&pm->submodel[submodel_num], pm);
			//	if(!Cmdline_nohtl)gr_set_lighting(false,false);
	} else {
		model_interp_sub( pm->submodel[submodel_num].bsp_data, pm, &pm->submodel[submodel_num], 0 );
	}

	gr_set_cull(0);

	if ( pm->submodel[submodel_num].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[submodel_num]);
	}

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[submodel_num] );

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();	
	}
	g3_done_instance(true);


	// turn off fog after each model renders, RT This fixes HUD being fogged when debris is in target box
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

//	if(!Cmdline_nohtl)gr_set_lighting(false,false);

}



// Fills in an array with points from a model.
// Only gets up to max_num verts;
// Returns number of verts found;
static int submodel_get_points_internal(int model_num, int submodel_num, int max_num, vec3d **pnts, vec3d **norms )
{
	polymodel * pm;

	pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		submodel_num = pm->detail[0];
	}

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF: return 1;
		case OP_DEFPOINTS:	{
				int n;
				int nverts = w(p+8);				
				int offset = w(p+16);				

				ubyte * normcount = p+20;
				vec3d *src = vp(p+offset);

				if ( nverts > max_num )
					nverts = max_num; 

				for (n=0; n<nverts; n++ )	{
					*pnts++ = src;
					*norms++ = src + 1;		// first normal associated with the point

					src += normcount[n]+1;
				} 
				return nverts;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_BOUNDBOX:		break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_points\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 0;		// Couldn't find 'em
}

// Gets two random points on a model
void submodel_get_two_random_points(int model_num, int submodel_num, vec3d *v1, vec3d *v2, vec3d *n1, vec3d *n2 )
{
	int nv = submodel_get_points_internal(model_num, submodel_num, MAX_POLYGON_VECS, Interp_verts, Interp_norms );

	Assert(nv > 0);	// Goober5000 - to avoid div-0 error
	int vn1 = (myrand()>>5) % nv;
	int vn2 = (myrand()>>5) % nv;

	*v1 = *Interp_verts[vn1];
	*v2 = *Interp_verts[vn2];

	if(n1 != NULL){
		*n1 = *Interp_norms[vn1];
	}
	if(n2 != NULL){
		*n2 = *Interp_norms[vn2];
	}
}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b )
{
	gr_init_color( &Interp_outline_color, r, g, b );

}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color_fast(void *outline_color)
{
	Interp_outline_color = *((color*)(outline_color));
}

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n)
{
	Interp_detail_level = n;
}


// Returns number of verts in a submodel;
int submodel_get_num_verts(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF: return 0;
		case OP_DEFPOINTS:	{
				int n=w(p+8);
				return n;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_verts\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 0;		// Couldn't find 'em
}

// Returns number of tmaps & flat polys in a submodel;
int submodel_get_num_polys_sub( ubyte *p )
{
	int chunk_type = w(p);
	int chunk_size = w(p+4);
	int n = 0;
	
	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF:			return n;
		case OP_DEFPOINTS:	break;
		case OP_FLATPOLY:		n++; break;
		case OP_TMAPPOLY:		n++; break;
		case OP_SORTNORM:		{
			int frontlist = w(p+36);
			int backlist = w(p+40);
			int prelist = w(p+44);
			int postlist = w(p+48);
			int onlist = w(p+52);
			n += submodel_get_num_polys_sub(p+frontlist);
			n += submodel_get_num_polys_sub(p+backlist);
			n += submodel_get_num_polys_sub(p+prelist);
			n += submodel_get_num_polys_sub(p+postlist );
			n += submodel_get_num_polys_sub(p+onlist );
			}
			break;
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_polys\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return n;		
}

// Returns number of tmaps & flat polys in a submodel;
int submodel_get_num_polys(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	return submodel_get_num_polys_sub( pm->submodel[submodel_num].bsp_data );

}

// Sets the submodel instance data in a submodel
// If show_damaged is true it shows only damaged submodels.
// If it is false it shows only undamaged submodels.
void model_show_damaged(int model_num, int show_damaged )
{
	polymodel * pm;
	int i;

	pm = model_get(model_num);

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		sm->blown_off = 0;
	}

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		if ( show_damaged )	{
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 0;
				sm->blown_off = 1;
			}
		} else {
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 1;
				sm->blown_off = 0;
			}
		}
	}
}

// set the insignia bitmap to be used when rendering a ship with an insignia (-1 switches it off altogether)
void model_set_insignia_bitmap(int bmap)
{
	Interp_insignia_bitmap = bmap;
}

void model_set_replacement_bitmap(int bmap)
{
	Interp_replacement_bitmap = bmap;
}

void model_set_replacement_textures(int *replacement_textures)
{
	Interp_replacement_textures = replacement_textures;	//replacement_textures;
}

// set the forces bitmap
void model_set_forced_texture(int bmap)
{
	Interp_forced_bitmap = bmap;
}

// set model transparency for use with MR_ALL_XPARENT
void model_set_alpha(float alpha)
{
	Interp_xparent_alpha = alpha;
}

// see if the given texture is used by the passed model. 0 if not used, 1 if used, -1 on error
int model_find_texture(int model_num, int bitmap)
{
	polymodel * pm;	
	int idx;

	// get a handle to the model
	pm = model_get(model_num);
	if(pm == NULL){
		return -1;
	}

	// find the texture
	for(idx=0; idx<pm->n_textures; idx++){
		if(pm->textures[idx] == bitmap){
			return 1;
		}
	}

	// no texture
	return 0;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_model_closest_box_point_with_delta(vec3d *closest_box_point, vec3d *start_point, int modelnum, int *is_inside, float delta)
{
	int i, idx;
	vec3d box_point, ray_direction, *extremes;
	float dist, best_dist;
	polymodel *pm;
	int inside = 0;
	int masks[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
	int mask_inside = 0x3f;

	best_dist = FLT_MAX;
	pm = model_get(modelnum);

	for (i=0; i<6; i++) {
		idx = i / 2;	// which row vector of Identity matrix

		memcpy(&ray_direction, vmd_identity_matrix.a2d[idx], sizeof(vec3d));

		// do negative, then positive plane for each axis
		if (2 * idx == i) {
			extremes = &pm->mins;
			vm_vec_negate(&ray_direction);
		} else {
			extremes = &pm->maxs;
		}

		// a negative distance means started outside the box
		dist = fvi_ray_plane(&box_point, extremes, &ray_direction, start_point, &ray_direction, 0.0f);
		if (dist > 0) {
			inside |= masks[i];
		}
		if (fabs(dist) < fabs(best_dist)) {
			best_dist = dist;
			if (closest_box_point) {
				vm_vec_scale_add(closest_box_point, &box_point, &ray_direction, delta);
			}
		}
	}

	// is start_point inside the box
	if (is_inside) {
		*is_inside = (inside == mask_inside);
	}

	return -best_dist;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_world_closest_box_point_with_delta(vec3d *closest_box_point, object *box_obj, vec3d *start_point, int *is_inside, float delta)
{
	vec3d temp, box_start;
	float dist;
	int modelnum;

	// get modelnum
	modelnum = Ships[box_obj->instance].modelnum;

	// rotate start_point to box_obj RF
	vm_vec_sub(&temp, start_point, &box_obj->pos);
	vm_vec_rotate(&box_start, &temp, &box_obj->orient);

	dist = get_model_closest_box_point_with_delta(closest_box_point, &box_start, modelnum, is_inside, delta);

	// rotate closest_box_point to world RF
	if (closest_box_point) {
		vm_vec_unrotate(&temp, closest_box_point, &box_obj->orient);
		vm_vec_add(closest_box_point, &temp, &box_obj->pos);
	}

	return dist;
}

void model_set_fog_level(float l)
{
	Interp_fog_level = l;
}

// given a newly loaded model, page in all textures
void model_page_in_textures(int modelnum, int ship_info_index)
{
	int i, idx;
	ship_info *sip;

	polymodel *pm = model_get(modelnum);

	// bogus
	if(pm == NULL){
		return;
	}

	if ((ship_info_index >= 0) && (ship_info_index < Num_ship_types)) {
		sip = &Ship_info[ship_info_index];

		// set nondarkening pixels	
		if(sip->num_nondark_colors){		
			palman_set_nondarkening(sip->nondark_colors, sip->num_nondark_colors);
		}
		// use the colors from the default table
		else {		
			palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
		}
	}

	for (idx=0; idx<pm->n_textures; idx++ ){
		if ( pm->original_textures[idx] > -1 )	{
			// go ahead and set it as a paged in texture so it's not unloaded during bm_page_in_stop()
			bm_page_in_texture(pm->original_textures[idx]);
			bm_lock(pm->original_textures[idx], 16, BMP_TEX_OTHER);
			bm_unlock(pm->original_textures[idx]);
		}

		if ( pm->glow_original_textures[idx] > -1 ) {
			// go ahead and set it as a paged in texture so it's not unloaded during bm_page_in_stop()
			bm_page_in_texture(pm->glow_original_textures[idx]);
			bm_lock(pm->glow_original_textures[idx], 16, BMP_TEX_OTHER);
			bm_unlock(pm->glow_original_textures[idx]);
		}

		if ( pm->specular_original_textures[idx] > -1 ) {
			// go ahead and set it as a paged in texture so it's not unloaded during bm_page_in_stop()
			bm_page_in_texture(pm->specular_original_textures[idx]);
			bm_lock(pm->specular_original_textures[idx], 16, BMP_TEX_OTHER);
			bm_unlock(pm->specular_original_textures[idx]);
		}
	}

	if (pm->n_glows) {
		for (i=0; i<pm->n_glows; i++) {
			glow_bank *bank = &pm->glows[i];

			if (bank->glow_bitmap > -1) {
				// go ahead and set it as a paged in texture so it's not unloaded during bm_page_in_stop()
				bm_page_in_texture(bank->glow_bitmap);
				bm_lock(bank->glow_bitmap, 16, BMP_TEX_OTHER);
				bm_unlock(bank->glow_bitmap);
			}

			if (bank->glow_neb_bitmap > -1) {
				// go ahead and set it as a paged in texture so it's not unloaded during bm_page_in_stop()
				bm_page_in_texture(bank->glow_neb_bitmap);
				bm_lock(bank->glow_neb_bitmap, 16, BMP_TEX_OTHER);
				bm_unlock(bank->glow_neb_bitmap);
			}
		}
	}
}

// unload all textures for a given model
// "release" should only be set if called from model_unload()!!!
void model_page_out_textures(int model_num, bool release)
{
	int i, j;

	if (model_num < 0)
		return;

	polymodel *pm = model_get( model_num );

	if (pm == NULL)
		return;

	if (release && (pm->used_this_mission > 0))
		return;

	for (i=0; i<pm->n_textures; i++) {
		if ( pm->original_textures[i] > -1 ) {
			if (release) {
				bm_release( pm->original_textures[i] );
			} else {
				bm_unload_fast( pm->original_textures[i] );
			}
		}

		if ( pm->glow_original_textures[i] > -1 ) {
			if (release) {
				bm_release( pm->glow_original_textures[i] );
			} else {
				bm_unload_fast( pm->glow_original_textures[i] );
			}
		}

		if ( pm->specular_original_textures[i] > -1 ) {
			if (release) {
				bm_release( pm->specular_original_textures[i] );
			} else {
				bm_unload_fast( pm->specular_original_textures[i] );
			}
		}
	}

	if (pm->n_glows) {
		// NOTE: "release" doesn't work here for some, as of yet unknown, reason - taylor
		for (j=0; j<pm->n_glows; j++) {
			glow_bank *bank = &pm->glows[j];

			if (bank->glow_bitmap > -1) {
			//	if (release) {
			//		bm_release( bank->glow_bitmap );
			//	} else {
					bm_unload( bank->glow_bitmap );
			//	}
			}

			if (bank->glow_neb_bitmap > -1) {
			//	if (release) {
			//		bm_release( bank->glow_neb_bitmap );
			//	} else {
					bm_unload( bank->glow_neb_bitmap );
			//	}
			}
		}
	}
}

// is the given model a pirate ship?
int model_is_pirate_ship(int modelnum)
{
	return 0;
}



//**********verrtex buffer stuff**********//
int tri_count[MAX_MODEL_TEXTURES];
poly_list list[MAX_MODEL_TEXTURES];
poly_list flat_list;
line_list flat_line_list;

vec3d **htl_verts = NULL;
vec3d **htl_norms = NULL;

int htl_nverts = 0;
int htl_nnorms = 0;

#define parseF(dest, f, off)	{memcpy(&dest, &f[off], sizeof(float)); off += sizeof(float);}
#define parseI(dest, f, off)	{memcpy(&dest, &f[off], sizeof(int)); off += sizeof(int);}
#define parseV(dest, f, off)	{parseF(dest.x, f, off); parseF(dest.y, f, off); parseF(dest.z, f, off); }
#define parseS(dest, f, off)	{memset(dest.str, 0, STRLEN); memcpy(&dest.n_char, &f[off], sizeof(int)); off += sizeof(int); memcpy(&dest.str, &f[off], dest.n_char); off += dest.n_char;}


void parse_defpoint(int off, ubyte *bsp_data){

	int i, n;
//	off+=4;
	int nverts = w(off+bsp_data+8);	
	int offset = w(off+bsp_data+16);
	int next_norm = 0;

	ubyte * normcount = off+bsp_data+20;
//	vertex *dest = Interp_points;
	vec3d *src = vp(off+bsp_data+offset);

	// Get pointer to lights
	Interp_lights = off+bsp_data+20+nverts;

//	Assert( nverts < MAX_POLYGON_VECS );
	// Assert( nnorms < MAX_POLYGON_NORMS );

	Interp_num_verts = nverts;
	#ifndef NDEBUG
	modelstats_num_verts += nverts;
	#endif

	{


		for (n=0; n<nverts; n++ )	{	

			htl_verts[n] = src;	
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				htl_norms[next_norm] = src;

				next_norm++;
				src++;
			}
//			dest++;
		}
	}

	Interp_num_norms = next_norm;



}

int check_values(vec3d *N)
{
	// Values equal to -1.#IND0
	if((N->xyz.x * N->xyz.x) < 0 ||
	   (N->xyz.y * N->xyz.y) < 0 ||
	   (N->xyz.z * N->xyz.z) < 0 ||
	   !is_valid_vec(N))
	{
		N->xyz.x = 1;
		N->xyz.y = 0;
		N->xyz.z = 0;
		return 1;
	}

	return 0;
}

int Parse_normal_problem_count = 0;

void parse_tmap(int offset, ubyte *bsp_data){
	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);
	//int n_tri = n_vert - 2;
	ubyte *temp_verts;
	ubyte *p = &bsp_data[offset+8];

	model_tmap_vert *tverts;
	tverts = (model_tmap_vert *)&bsp_data[offset+44];
	temp_verts = &bsp_data[offset+44];

	vertex *V;
	vec3d *v;
	vec3d *N;

	int problem_count = 0;

	for(int i = 1; i<n_vert-1; i++){
		V = &list[pof_tex].vert[(list[pof_tex].n_verts)];
		N = &list[pof_tex].norm[(list[pof_tex].n_verts)];
		v = htl_verts[(int)tverts[0].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[0].u;
		V->v = tverts[0].v;
		*N = *htl_norms[(int)tverts[0].normnum];
		if(IS_VEC_NULL(N))
			*N = *vp(p);

	  	problem_count += check_values(N);
		vm_vec_normalize(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		V = &list[pof_tex].vert[(list[pof_tex].n_verts)+1];
		N = &list[pof_tex].norm[(list[pof_tex].n_verts)+1];
		v = htl_verts[(int)tverts[i].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[i].u;
		V->v = tverts[i].v;
		*N = *htl_norms[(int)tverts[i].normnum];
		if(IS_VEC_NULL(N))
			*N = *vp(p);

	 	problem_count += check_values(N);
		vm_vec_normalize(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		V = &list[pof_tex].vert[(list[pof_tex].n_verts)+2];
		N = &list[pof_tex].norm[(list[pof_tex].n_verts)+2];
		v = htl_verts[(int)tverts[i+1].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[i+1].u;
		V->v = tverts[i+1].v;
		*N = *htl_norms[(int)tverts[i+1].normnum];
		if(IS_VEC_NULL(N))
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		list[pof_tex].n_verts += 3;
		list[pof_tex].n_prim++;
	
	}

	Parse_normal_problem_count += problem_count;
}

// Flat Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
// +32     float       radius
// +36     int         nverts
// +40     byte        red
// +41     byte        green
// +42     byte        blue
// +43     byte        pad
// +44     nverts*short*short  vertlist, smoothlist
void parse_flat_poly(int offset, ubyte *bsp_data)
{
/* Goober5000 - if this function was commented out, the variables should be also
	int nv = w(bsp_data+offset+36);
	short *verts = (short *)(&bsp_data[offset+44]);

	vertex *V;
	vec3d *v;
	vec3d *N;
	int i = 0;

	for( i = 1; i<nv-1; i++){
		V = &flat_list.vert[flat_list.n_poly][0];
		N = &flat_list.norm[flat_list.n_poly][0];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		V = &flat_list.vert[flat_list.n_poly][1];
		N = &flat_list.norm[flat_list.n_poly][1];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		V = &flat_list.vert[flat_list.n_poly][2];
		N = &flat_list.norm[flat_list.n_poly][2];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		flat_list.n_poly++;
	}

	//we don't need this
	for(i = 0; i<nv; i++){
		V = &flat_line_list.vert[flat_line_list.n_line][0];
		v = Interp_verts[verts[(i%nv*2)]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];

		V = &flat_line_list.vert[flat_line_list.n_line][1];
		v = Interp_verts[verts[(((i+1)%nv)*2)]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];

		flat_line_list.n_line++;
	}*/
}
//flat_list

void parse_sortnorm(int offset, ubyte *bsp_data);


void parse_bsp(int offset, ubyte *bsp_data){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS:	parse_defpoint(offset, bsp_data);
			break;
		case OP_SORTNORM:	parse_sortnorm(offset, bsp_data);
			break;
		case OP_FLATPOLY:	parse_flat_poly(offset, bsp_data);
			break;
		case OP_TMAPPOLY:	parse_tmap(offset, bsp_data);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}


void parse_sortnorm(int offset, ubyte *bsp_data){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) parse_bsp(offset+prelist,bsp_data);
	if (backlist) parse_bsp(offset+backlist, bsp_data);
	if (onlist) parse_bsp(offset+onlist, bsp_data);
	if (frontlist) parse_bsp(offset+frontlist, bsp_data);
	if (postlist) parse_bsp(offset+postlist, bsp_data);
}

void find_tri_counts(int offset, ubyte *bsp_data);
/*
int htl_nverts = 0;
int htl_nnorms = 0;
vec3d *htl_verts;
vec3d *htl_norms;
*/

void dealc_model_loadstuf(){
	if(htl_verts)vm_free(htl_verts);
	htl_verts= NULL;
	if(htl_norms)vm_free(htl_norms);
	htl_norms= NULL;
}

 int allocate_poly_list_nvert = 0;
 int allocate_poly_list_nnorm = 0;
 bool allocate_poly_list_a = true;
void allocate_poly_list(){
	for(int i = 0; i<MAX_MODEL_TEXTURES; i++){
		list[i].allocate(tri_count[i]*3);
	}

	if(htl_nverts > allocate_poly_list_nvert){
		if(htl_verts)vm_free(htl_verts);
		htl_verts = (vec3d**)vm_malloc(sizeof(vec3d*)*htl_nverts);
		allocate_poly_list_nvert = htl_nverts;
	}
	if(htl_nnorms > allocate_poly_list_nnorm){
		if(htl_norms)vm_free(htl_norms);
		htl_norms = (vec3d**)vm_malloc(sizeof(vec3d*)*htl_nnorms);
		allocate_poly_list_nnorm = htl_nnorms;
	}

	if(allocate_poly_list_a){
		atexit(dealc_model_loadstuf);
		allocate_poly_list_a = false;
	}
}
int recode_check = 0;

void recode_bsp(int offset, ubyte *bsp_data);
void model_resort_index_buffer(ubyte *bsp_data, bool f2b, int texture, short* index_buffer);
poly_list model_list;
void generate_vertex_buffers(bsp_info* model, polymodel * pm){
	int i;
	for(i =0; i<MAX_MODEL_TEXTURES; i++){
		list[i].n_prim=0;
		list[i].n_verts=0;
		tri_count[i]=0;
	}

//	flat_list.n_poly = 0;
	flat_line_list.n_line = 0;

	find_tri_counts(0, model->bsp_data);
	int total_verts = 0;

	for(i = 0; i<MAX_MODEL_TEXTURES; i++){
		total_verts += tri_count[i];
	}
	if(total_verts < 1){
		model->indexed_vertex_buffer = -1;
		return;
	}

	allocate_poly_list();

	parse_bsp(0, model->bsp_data);
	model->n_buffers = 0;

	total_verts = 0;
	for(i=0; i<MAX_MODEL_TEXTURES; i++){
		total_verts += list[i].n_verts;
	}
	model_list.allocate(total_verts);
	model_list.n_verts = 0;
	model_list.n_prim = 0;

	for(i=0; i<MAX_MODEL_TEXTURES; i++){
		if(!list[i].n_verts)continue;
		memcpy( (model_list.vert) + model_list.n_verts, list[i].vert, sizeof(vertex) * list[i].n_verts);
		memcpy( (model_list.norm) + model_list.n_verts, list[i].norm, sizeof(vec3d) * list[i].n_verts);
		model_list.n_verts += list[i].n_verts;
	}

	// IBX stuff
	extern IBX ibuffer_info;

	// if we have an IBX read file then use it, otherwise generate buffers and save them to file
	if ( ibuffer_info.read != NULL ) {
		int ibx_verts = 0;
		int ibx_size = 0;

		ibx_verts = cfread_int( ibuffer_info.read );
		ibuffer_info.size -= sizeof(int); // subtract

		// figure up how big this section of data is going to be
		ibx_size += ibx_verts * sizeof(float) * 8; // first set of data (here)
		ibx_size += ibx_verts * sizeof(short); // second set of data (next "for" statement)

		// safety check for this section
		// ibuffer_info.size should be greater than or equal to ibx_size at this point
		if ( ibx_size > ibuffer_info.size ) {
			// AAAAAHH! not enough stored data - Abort, Retry, Fail?
			mprintf(("IBX: Safety Check Failure!  The file doesn't contain enough data, deleting '%s'\n", ibuffer_info.name));

			cfclose( ibuffer_info.read );
			ibuffer_info.read = NULL;
			ibuffer_info.size = 0;
			cf_delete( ibuffer_info.name, CF_TYPE_CACHE );

			// force generate
			model_list.make_index_buffer();
		} else {
			// it's all good
			for (i=0; i<ibx_verts; i++) {
				model_list.vert[i].x = cfread_float( ibuffer_info.read );
				model_list.vert[i].y = cfread_float( ibuffer_info.read );
				model_list.vert[i].z = cfread_float( ibuffer_info.read );
				model_list.vert[i].u = cfread_float( ibuffer_info.read );
				model_list.vert[i].v = cfread_float( ibuffer_info.read );
				cfread_vector( &model_list.norm[i], ibuffer_info.read );
			}

			model_list.n_verts = ibx_verts;
		}

		// subtract this block of data from the total size for next check
		// remember that this includes the next set of reads too
		ibuffer_info.size -= ibx_size;
	} else {
		// no read file so we'll have to generate
		model_list.make_index_buffer();

		if ( ibuffer_info.write != NULL ) {
			cfwrite_int( model_list.n_verts, ibuffer_info.write );

			for (i=0; i<model_list.n_verts; i++) {
				cfwrite_float( model_list.vert[i].x, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].y, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].z, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].u, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].v, ibuffer_info.write );
				cfwrite_vector( &model_list.norm[i], ibuffer_info.write );
			}
		}
	}

	model->indexed_vertex_buffer = gr_make_buffer(&model_list, VERTEX_FLAG_POSITION | VERTEX_FLAG_NORMAL | VERTEX_FLAG_UV1);
	if(model->indexed_vertex_buffer == -1)
		Error(LOCATION, "Could not generate model->indexed_vertex_buffer");
	recode_check = 0;
//	recode_bsp(0, model->bsp_data);

	for(i=0; i<MAX_MODEL_TEXTURES; i++){	
		if(!list[i].n_verts)continue;

		if (model->n_buffers >= MAX_BUFFERS_PER_SUBMODEL)
			Error(LOCATION, "Submodel %s on model %s has more than %d textures.\nFind a high-limit build or remap the model.", model->name, pm->filename, MAX_BUFFERS_PER_SUBMODEL);

		model->buffer[model->n_buffers].index_buffer.allocate_index_buffer(list[i].n_verts);
		for(int j = 0; j < list[i].n_verts; j++){
			if ( ibuffer_info.read != NULL ) {
				model->buffer[model->n_buffers].index_buffer.index_buffer[j] = cfread_short( ibuffer_info.read );
			} else {
				model->buffer[model->n_buffers].index_buffer.index_buffer[j] = find_first_index_vb(&list[i], j, &model_list);
				Assert(model->buffer[model->n_buffers].index_buffer.index_buffer[j] != -1);
				Assert(same_vert(&model_list.vert[model->buffer[model->n_buffers].index_buffer.index_buffer[j]], &list[i].vert[j], &model_list.norm[model->buffer[model->n_buffers].index_buffer.index_buffer[j]], &list[i].norm[j]));
			//	Assert(find_first_index_vb(&model_list, j, &list[i]) == j);//there should never ever be any redundant verts

				// try to write out generated index buffer for later use
				if ( ibuffer_info.write != NULL ) {
					cfwrite_short( model->buffer[model->n_buffers].index_buffer.index_buffer[j], ibuffer_info.write );
				}
			}
		}
		model->buffer[model->n_buffers].n_prim = tri_count[i];
		model->buffer[model->n_buffers].texture = i;
		model->n_buffers++;
	}

}


void find_tmap(int offset, ubyte *bsp_data){
	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);

	tri_count[pof_tex] += n_vert-2;	
}

void find_sortnorm(int offset, ubyte *bsp_data){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) find_tri_counts(offset+prelist,bsp_data);
	if (backlist) find_tri_counts(offset+backlist, bsp_data);
	if (onlist) find_tri_counts(offset+onlist, bsp_data);
	if (frontlist) find_tri_counts(offset+frontlist, bsp_data);
	if (postlist) find_tri_counts(offset+postlist, bsp_data);
}
/*
vertex *htl_points;
vec3d *htl_verts;
vec3d *htl_norms;

int htl_nverts = 0;
int htl_nnorms = 0;
*/
void find_defpoint(int off, ubyte *bsp_data){

	int n;
//	off+=4;
	int nverts = w(off+bsp_data+8);	
	int offset = w(off+bsp_data+16);
	//int next_norm = 0;

	ubyte * normcount = off+bsp_data+20;
	vec3d *src = vp(off+bsp_data+offset);

	// Get pointer to lights
	Interp_lights = off+bsp_data+20+nverts;

	// Assert( nnorms < MAX_POLYGON_NORMS );

	Interp_num_verts = nverts;
	#ifndef NDEBUG
	modelstats_num_verts += nverts;
	#endif


	int norm_num = 0;
	for (n=0; n<nverts; n++ )	{	
		
		src++;		// move to normal

		norm_num += normcount[n];
	}

	htl_nverts = nverts;
	htl_nnorms = norm_num;

}


//tri_count
void find_tri_counts(int offset, ubyte *bsp_data){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS:	find_defpoint(offset, bsp_data);
			break;
		case OP_SORTNORM:	find_sortnorm(offset, bsp_data);
			break;
		case OP_FLATPOLY:	//find_flat_poly(offset, bsp_data);
			break;
		case OP_TMAPPOLY:	find_tmap(offset, bsp_data);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}


int in_box(vec3d *min, vec3d *max, vec3d * point){
	if(point->xyz.x >= min->xyz.x && point->xyz.x <= max->xyz.x &&
		point->xyz.y >= min->xyz.y && point->xyz.y <= max->xyz.y &&
		point->xyz.z >= min->xyz.z && point->xyz.z <= max->xyz.z)return 1;
	return -1;
}


void model_render_childeren_buffers(bsp_info* model, polymodel * pm, int mn, int detail_level){

	if(model->use_render_box && -model->use_render_box + in_box(&model->render_box_min, &model->render_box_max, &View_position))return;
	
	int i;
	int zbuf_mode = gr_zbuffering_mode;

	if ( (mn < 0) || (mn>=pm->n_models) )
		return;

	Assert( mn >= 0 );
	Assert( mn < pm->n_models );

	
	if (pm->submodel[mn].blown_off){
		return;
	}

	unsigned int fl = Interp_flags;
	if (pm->submodel[mn].is_thruster )	{
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) ){
			return;
		}
		Interp_flags |= MR_NO_LIGHTING;
		Interp_thrust_scale_subobj=1;
	} else {
		Interp_thrust_scale_subobj=0;
	}

	vm_vec_add2(&Interp_offset,&pm->submodel[mn].offset);

	if(model->gun_rotation){
			if (pm->gun_submodel_rotation > PI2 )
				pm->gun_submodel_rotation -= PI2;
			else if (pm->gun_submodel_rotation < 0.0f )
				pm->gun_submodel_rotation += PI2;
	
		angles ang = pm->submodel[mn].angs;
		ang.b += pm->gun_submodel_rotation;

		g3_start_instance_angles(&pm->submodel[mn].offset, &ang);
	}else{
		g3_start_instance_angles(&pm->submodel[mn].offset, &pm->submodel[mn].angs);
	}
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_render_buffers( model, pm );

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn] );

	if ( pm->submodel[mn].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[mn]);
	}

	i = pm->submodel[mn].first_child;
	while( i >= 0 )	{
		if (!pm->submodel[i].is_thruster )	{
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			} else {
				zbuf_mode = GR_ZBUFF_FULL;		// read only
			}

			gr_zbuffer_set(zbuf_mode);

			model_render_childeren_buffers( &pm->submodel[i], pm, i, detail_level );
		}
		i = pm->submodel[i].next_sibling;
	}

	vm_vec_sub2(&Interp_offset,&pm->submodel[mn].offset);



	Interp_flags = fl;
	g3_done_instance(true);
}

extern vec3d Object_position;
extern matrix Object_matrix;

void model_render_buffers(bsp_info* model, polymodel * pm){

	if(model->use_render_box && -model->use_render_box + in_box(&model->render_box_min, &model->render_box_max, &View_position))return;
	
	if(model->indexed_vertex_buffer == -1)return;

	// RT Added second conditional parameter, seems to not distrupt anything in either API
	gr_set_lighting( !(Interp_flags & MR_NO_LIGHTING), !(Interp_flags & MR_NO_LIGHTING) );

	vec3d scale;

	if(Interp_thrust_scale_subobj){
		scale.xyz.x = 1.0f;
		scale.xyz.y = 1.0f;
		scale.xyz.z = Interp_thrust_scale;
	}else{
		scale.xyz.x = Model_Interp_scale_x;
		scale.xyz.y = Model_Interp_scale_y;
		scale.xyz.z = Model_Interp_scale_z;
	}

	if(Interp_flags & MR_NO_CULL){
		gr_set_cull(0);
	}else{
		gr_set_cull(1);
	}

	gr_push_scale_matrix(&scale);

	gr_set_buffer(model->indexed_vertex_buffer);
	for(int i = 0; i<model->n_buffers; i++){
		int texture = -1;

		if((Interp_flags & MR_FORCE_TEXTURE) && (Interp_forced_bitmap >= 0)){
			texture = Interp_forced_bitmap;
		}else if(Warp_Map > -1){
			texture = Warp_Map;
		}else if(Interp_replacement_bitmap >= 0){
			texture = Interp_replacement_bitmap;
		}else if(Interp_thrust_scale_subobj){
			texture = Interp_thrust_bitmap;
		} else {
			if (pm->is_ani[model->buffer[i].texture]){
				texture = pm->textures[model->buffer[i].texture] + ((timestamp() / (int)(pm->fps[model->buffer[i].texture])) % pm->num_frames[model->buffer[i].texture]);//here is were it picks the texture to render for ani-Bobboau
			}else{
				texture = pm->textures[model->buffer[i].texture];//here is were it picks the texture to render for normal-Bobboau
			}

			if((Detail.lighting > 2)  && (model_current_LOD < 2))SPECMAP = pm->specular_textures[model->buffer[i].texture];

			if(glow_maps_active)
			{
				if (pm->glow_is_ani[model->buffer[i].texture])
				{
					GLOWMAP = pm->glow_textures[model->buffer[i].texture] + ((timestamp() / (int)(pm->glow_fps[model->buffer[i].texture])) % pm->glow_numframes[model->buffer[i].texture]);
				}
				else
				{
					GLOWMAP = pm->glow_textures[model->buffer[i].texture];
				}
			}
		}

		if(texture == -1)continue;
//		extern int big_ole_honkin_hack_test;
//		texture = big_ole_honkin_hack_test;

// Goober5000 - this should fix replacement textures under HTL, but it doesn't... more work needed :(
//		if (Interp_replacement_textures != NULL)
//			texture = Interp_replacement_textures[texture];

		if(Interp_thrust_scale_subobj) {

			if((Interp_thrust_bitmap >= 0) && (Interp_thrust_scale > 0.0f)) {
				gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f);
			}
			gr_zbuffer_set(GR_ZBUFF_READ);
			
		}else if(Interp_flags & MR_ALL_XPARENT){
			gr_set_cull(0);
			gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_xparent_alpha );
			gr_zbuffer_set(GR_ZBUFF_NONE);
		}else if(Warp_Map > -1){	//trying to get transperent textures-Bobboau
			gr_set_cull(0);
			gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Warp_Alpha );
			gr_zbuffer_set(GR_ZBUFF_READ);
		} else if(pm->transparent[model->buffer[i].texture] ){	//trying to get transperent textures-Bobboau
			if(Warp_Alpha!=-1.0)gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Warp_Alpha );
			else gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f );
			gr_zbuffer_set(GR_ZBUFF_READ);

		}else{
		//	gr_set_state(TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_DEFAULT);
			gr_set_bitmap( texture, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 1.0 );
	//		gr_zbuffer_set(GR_ZBUFF_FULL);
		}
		if((Interp_flags & MR_EDGE_ALPHA))gr_center_alpha(-1);
		else if((Interp_flags & MR_CENTER_ALPHA))gr_center_alpha(1);
		else gr_center_alpha(0);

	//	model_resort_index_buffer(model->bsp_data, 1, model->buffer[i].texture, model->buffer[i].index_buffer.index_buffer);
		
		gr_render_buffer(0, model->buffer[i].n_prim, model->buffer[i].index_buffer.index_buffer);		
	}
	GLOWMAP = -1;
	SPECMAP = -1;

//	if(model->flat_buffer != -1)gr_render_buffer(model->flat_buffer);
	//we don't need this
//	if(model->flat_line_buffer != -1)gr_render_buffer(model->flat_line_buffer);

	gr_pop_scale_matrix();

	gr_set_lighting(false,false);

}


//int recode_check = 0;
void recode_tmap(int offset, ubyte *bsp_data){

//	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);

//	if(pof_tex == 4){
//		1;
//	}

	//int n_tri = n_vert - 2;
	ubyte *temp_verts;
	//ubyte *p = &bsp_data[offset];

	model_tmap_vert *tverts;
	tverts = (model_tmap_vert *)&bsp_data[offset+44];
	temp_verts = &bsp_data[offset+44];

	//int problem_count = 0;

	for(int i = 0; i<n_vert; i++){	
		vertex vert;
		vm_vec2vert(htl_verts[(int)tverts[i].vertnum], &vert);
		vec3d norm = *htl_norms[(int)tverts[i].normnum];
		vm_vec_normalize(&norm);

		vert.u = tverts[i].u;
		vert.v = tverts[i].v;

		for(int k = 0; k<model_list.n_verts; k++){
			if(same_vert(&model_list.vert[k], &vert, &model_list.norm[k], &norm)){
				tverts[i].normnum = (short)k;
				recode_check++;
				break;
			}
			if(k == model_list.n_verts -1){
				Warning(LOCATION, "recode error");
			//	tverts[i].normnum = (short)0;
			}
		}
	}

}

void recode_sortnorm(int offset, ubyte *bsp_data);

void recode_bsp(int offset, ubyte *bsp_data){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS: parse_defpoint(offset, bsp_data);
			break;
		case OP_SORTNORM:	recode_sortnorm(offset, bsp_data);
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:	recode_tmap(offset, bsp_data);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}

int model_resort_index_buffer_n_verts = 0;
void recode_sortnorm(int offset, ubyte *bsp_data){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) recode_bsp(offset+prelist,bsp_data);
	if (backlist) recode_bsp(offset+backlist, bsp_data);
	if (onlist) recode_bsp(offset+onlist, bsp_data);
	if (frontlist) recode_bsp(offset+frontlist, bsp_data);
	if (postlist) recode_bsp(offset+postlist, bsp_data);
}

/*
buffer[j] = find_first_index_vb(list, j, model_list)
short find_first_index_vb(poly_list *plist, int idx, poly_list *v){
	for(short i = 0; i<v->n_verts; i++){
		if(same_vert(&v->vert[i], &plist->vert[idx], &v->norm[i], &plist->norm[idx])){
			return i;
		}
	}
	return -1;
}
*/
void model_resort_index_buffer_tmap(int offset, ubyte *bsp_data, short* index_buffer, int texture){
	if(texture != w(bsp_data+offset+40))return;

	int n_vert = w(bsp_data+offset+36);
	//int n_tri = n_vert - 2;
	ubyte *temp_verts;
	//ubyte *p = &bsp_data[offset];

	model_tmap_vert *tverts;
	tverts = (model_tmap_vert *)&bsp_data[offset+44];
	temp_verts = &bsp_data[offset+44];
	for(int i = 1; i<n_vert-1; i++){	
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[0].normnum;
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[i].normnum;
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[i+1].normnum;
	}

}

void model_resort_index_buffer_bsp(int offset, ubyte *bsp_data, bool f2b, int texture, short* index_buffer);

void model_resort_index_buffer_sortnorm_nonsorted(int offset, ubyte *bsp_info, int texture, short* index_buffer){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, false, texture, index_buffer);
	if (backlist) model_resort_index_buffer_bsp(offset+backlist, bsp_info, false, texture, index_buffer);
	if (onlist) model_resort_index_buffer_bsp(offset+onlist, bsp_info, false, texture, index_buffer);
	if (frontlist) model_resort_index_buffer_bsp(offset+frontlist, bsp_info, false, texture, index_buffer);
	if (postlist) model_resort_index_buffer_bsp(offset+postlist, bsp_info, false, texture, index_buffer);
}

// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_resort_index_buffer_sortnorm_b2f(int offset, ubyte *bsp_info, int texture, short* index_buffer)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, false, texture, index_buffer);		// prelist

	if (g3_check_normal_facing(vp(bsp_info+offset+20),vp(bsp_info+offset+8))) {		//facing

		//draw back then front

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, false, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, false, texture, index_buffer);			//onlist

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, false, texture, index_buffer);

	}	else {			//not facing.  draw front then back

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, false, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, false, texture, index_buffer);			//onlist

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, false, texture, index_buffer);
	}

	if (postlist) model_resort_index_buffer_bsp(offset+postlist,bsp_info, false, texture, index_buffer);		// postlist

}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_resort_index_buffer_sortnorm_f2b(int offset, ubyte *bsp_info, int texture, short* index_buffer)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (postlist) model_resort_index_buffer_bsp(offset+postlist,bsp_info, true, texture, index_buffer);		// postlist

	if (g3_check_normal_facing(vp(bsp_info+offset+20),vp(bsp_info+offset+8))) {		//facing

		//draw front then back

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, true, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, true, texture, index_buffer);			//onlist

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, true, texture, index_buffer);

	} else {

		//draw back then front

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, true, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, true, texture, index_buffer);			//onlist

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, true, texture, index_buffer);

	}

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, true, texture, index_buffer);		// prelist
}


void model_resort_index_buffer_bsp(int offset, ubyte *bsp_data, bool f2b, int texture, short* index_buffer){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS:
			break;
		case OP_SORTNORM:	if(f2b)model_resort_index_buffer_sortnorm_f2b(offset, bsp_data, texture, index_buffer); else model_resort_index_buffer_sortnorm_b2f(offset, bsp_data, texture, index_buffer);
//			model_resort_index_buffer_sortnorm_nonsorted(offset, bsp_data, texture, index_buffer);
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:	model_resort_index_buffer_tmap(offset, bsp_data, index_buffer, texture);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}


void model_resort_index_buffer(ubyte *bsp_data, bool f2b, int texture, short* index_buffer){
	model_resort_index_buffer_n_verts = 0;
	model_resort_index_buffer_bsp(0, bsp_data, f2b, texture, index_buffer);
}

/*
struct silhouette_index{
	short* silhouette_index;
	int silhouette_n_allocated;
	int n_in_list;
	void allocate(int size){
		if(size <= silhouette_n_allocated)return;
		short* new_buffer = (short*)vm_malloc(sizeof(short)* size);
		if(silhouette_n_allocated){memcpy(new_buffer, silhouette_index, sizeof(short)*silhouette_n_allocated );}
		silhouette_n_allocated = size;
	};
	void add_line(short one, short two){
		for(int i = 0; i<n_in_list; i+=2){
			if(silhouette_index[i] == one && silhouette_index[i+1] = two ||
				silhouette_index[i] == two && silhouette_index[i+1] = one){
				memcpy(&silhouette_index[i+2], &silhouette_index[i], n_in_list - i - 2);
				return;
			}else{
				allocate(n_in_list+2);
				silhouette_index[n_in_list] = one;
				silhouette_index[n_in_list+1] = two;
			}
		}
	};
	silhouette_index(){silhouette_n_allocated=0; n_in_list=0;};
	~silhouette_index(){vm_free(silhouette_index);};
};


void get_silhouette_from_point(vec3d *point_list, ubyte *bsp_data, vec3d* point){
}
*/
//************************************//
//*** triggered submodel animation ***//
//************************************//

char* animation_type_names[MAX_TRIGGER_ANIMATION_TYPES] = {
"\"inital\"",
"\"docking\"",
"\"docked\"",
"\"primary_bank\"",
"\"secondary_bank\"",
"\"door\""
};


//this calculates the angle at wich the rotation should start to slow down
//and basicly fills in a bunch of other crap
//if anyone wants the calculus behind these numbers I'll provide it
void triggered_rotation::start(queued_animation* q){
	start_time = timestamp();
	instance = q->instance;

#ifndef NDEBUG
	mprintf(("animation start at %d\n",timestamp()));
	char ax[]="xyz";
#endif

//	vec3d sp;
//	vm_vec_rotate(&sp,&snd_pnt,&Objects[obj_num].orient);
//	vm_vec_add2(&sp, &Objects[obj_num].pos);
//	if(q->start_sound != -1)current_snd = snd_play_3d(&Snds[q->start_sound], &sp, &View_position, q->snd_rad) ;

	current_snd = -2;
	current_snd_index = start_sound = q->start_sound;
	loop_sound = q->loop_sound;
	end_sound = q->end_sound;
	snd_rad = q->snd_rad;

	for(int axis = 0; axis < 3; axis++){

		direction.a1d[axis] = (end_angle.a1d[axis]+q->angle.a1d[axis])-current_ang.a1d[axis];
		if(direction.a1d[axis])direction.a1d[axis] /= (float)fabs(direction.a1d[axis]);

		if(q->absolute){
			end_angle.a1d[axis] = end_angle.a1d[axis] - (2*PI2*(float)int(end_angle.a1d[axis]/(2*PI2)));
			current_ang.a1d[axis] = current_ang.a1d[axis] - (2*PI2*(float)int(current_ang.a1d[axis]/(2*PI2)));
		}else{
			end_angle.a1d[axis]=q->angle.a1d[axis]+end_angle.a1d[axis];
		}
	
		rot_vel.a1d[axis] = q->vel.a1d[axis]*direction.a1d[axis];
		rot_accel.a1d[axis] = q->accel.a1d[axis]*direction.a1d[axis];
	
		slow_angle.a1d[axis]=end_angle.a1d[axis]-(((q->vel.a1d[axis]*q->vel.a1d[axis])/(2.0f*q->accel.a1d[axis]))*direction.a1d[axis]);

#ifndef NDEBUG
		mprintf(("axis %c: direction=%d, end angle=%f, velocity=%f, acceleration=%f, slow angle=%f\n", ax[axis],(int)direction.a1d[axis],end_angle.a1d[axis],rot_vel.a1d[axis],rot_accel.a1d[axis],slow_angle.a1d[axis]));
#endif
	}
}

void triggered_rotation::set_to_end(queued_animation* q){
	for(int axis = 0; axis < 3; axis++){
		current_ang.a1d[axis] += q->angle.a1d[axis];
	}
}

triggered_rotation::triggered_rotation(){
	vec3d v = ZERO_VECTOR;
	current_ang = v;	
	current_vel = v;	
	rot_accel = v;	
	rot_vel = v;	
	slow_angle = v;	
	end_time = 0;	
	end_angle = v;
	n_queue = 0;
	instance = -1;
}

triggered_rotation::~triggered_rotation(){
	//origonaly the que was a dynamic array
}



void triggered_rotation::add_queue(queued_animation* the_queue, int direction){
	static queued_animation scrap_queue;
	scrap_queue = *the_queue;

	queued_animation* new_queue = &scrap_queue;

	if(direction == -1)new_queue->start = new_queue->reverse_start;

	if(direction == -1){
		scrap_queue.angle.xyz.x *= direction;
		scrap_queue.angle.xyz.y *= direction;
		scrap_queue.angle.xyz.z *= direction;
	}

	queued_animation old[MAX_TRIGGERED_ANIMATIONS];		//all the triggered animations assosiated with this object

	memcpy(old, queue, sizeof(queued_animation)*MAX_TRIGGERED_ANIMATIONS);

	int i;
	if(n_queue > 0){
		//remove any items on the queue that are the opposite of what we are thinking about doing
//		if(direction = -1){
			//if we are reverseing an animation see if the forward animation is on the queue already
			//if it is remove it
			for(i = 0; i < n_queue && i < MAX_TRIGGERED_ANIMATIONS; i++){
				if(new_queue->type == old[i].type && new_queue->subtype == old[i].subtype){
					//same type, if they have the same values (direction reversed) this bitch is gone!
					if(new_queue->instance == old[i].instance){
						//that's it that's everything is the same
						break;
					}
				}
			}
			if(i!=n_queue){
				//we've got to remove item number i
				if(i!=MAX_TRIGGERED_ANIMATIONS-1)
					//if it's not the last item on the list
					//copy everything after it over top of it
					memcpy(&old[i], &old[i+1], sizeof(queued_animation)*(MAX_TRIGGERED_ANIMATIONS-(i+1)));
				n_queue--;
				//ok these two animations basicly caceled each other out, 
				//so he doesn't get on the queue
				return;
			}
	//	}
	}

	if(new_queue->start == 0){
		new_queue->start += timestamp();
		new_queue->end += new_queue->start;
		start(new_queue);	//if there is no delay don't bother with the queue, just start the thing
		return;
	}

	if(new_queue->instance == instance){
		//same animation is playing that we are about to think about playing some point in the future
		if(this->direction.xyz.x * rot_vel.xyz.x == new_queue->vel.xyz.x && this->direction.xyz.y * rot_vel.xyz.y == new_queue->vel.xyz.y && this->direction.xyz.z * rot_vel.xyz.z == new_queue->vel.xyz.z){
			//there going in opposite directions
			//one of them is a reversal!
			//so this means thata there is some sort of delay that's getting fubared becase of other queue items getting removed due to reversal
			//this animation needs to be started now!
			new_queue->start =  start_time + new_queue->real_end_time - timestamp();
			new_queue->end += new_queue->start;
			start(new_queue);	//if there is no delay don't bother with the queue, just start the thing
			return;
		}
	}

	//starts that many seconds from now
	new_queue->start += timestamp();
	//runs for that long
	new_queue->end += new_queue->start;


	if(n_queue > 0){
		int i;
		//if we already have something in the queue find the first item on the 
		//queue that is going to start after the new item, 
		for(i = 0; new_queue->start > old[i].start && i < n_queue && i < MAX_TRIGGERED_ANIMATIONS; i++);
		if(i >= MAX_TRIGGERED_ANIMATIONS)return;
		//then incert the new item before that item
		//from the begining of the queue to the item on the queue that is just before the new item
		if(i)memcpy(queue, old, sizeof(queued_animation)*(i));
		//if there are any items after copy them from the origonal queue
		if(n_queue >= i+1)
			memcpy(&queue[i+1], &old[i], sizeof(queued_animation)*(n_queue - i));
		//add the new item
		queue[i] = *new_queue;
	}else{
		queue[0] = *new_queue;
	}
	n_queue++;
}

//look at the queue and see if any of the items on it need to be started
//remove items from the queue that you just executed
void triggered_rotation::proces_queue(){
	int i;

	if(!n_queue)return;
	//if there is nothing on the queue just quit right now

	//all items on the que are in cronological order (or at least they should be)
	//so execute all items who's starting timestamps are less than the current time
	for(i = 0; queue[i].start<=timestamp() && i<n_queue; i++){
		start(&queue[i]);
	}
	//if no items were procesed quit
	if(!i)return;

	queued_animation old[MAX_TRIGGERED_ANIMATIONS];		//all the triggered animations assosiated with this object

	memcpy(old, queue, sizeof(queued_animation)*MAX_TRIGGERED_ANIMATIONS);

//	if(n_queue > i){

		//if there are more items on the queue than we just executed reallocate the queue 
		//copy all the items after the last one we executed
		memcpy(queue, &old[i], sizeof(queued_animation)*(n_queue-i));
//	}
	//then erase the old queue
	n_queue-=i;
	queue[n_queue].start = -1;
}


queued_animation::queued_animation(float an,float v,float a,int s, int e, int t, int st, int axis)
:start(s),end(e),absolute(false),type(t),subtype(st){
	angle.a1d[axis]=an; 
	angle.a1d[(axis+1)%3]=0; 
	angle.a1d[(axis+2)%3]=0; 
	vel.a1d[axis]=v; 
	vel.a1d[(axis+1)%3]=0; 
	vel.a1d[(axis+2)%3]=0; 
	accel.a1d[axis]=a; 
	accel.a1d[(axis+1)%3]=0; 
	accel.a1d[(axis+2)%3]=0;
}

void queued_animation::corect(){
	for(int i = 0; i<3; i++)
		if((vel.a1d[i]*vel.a1d[i])/accel.a1d[i] > fabs(angle.a1d[i]))
			vel.a1d[i] = (float)sqrt(fabs(accel.a1d[i] * angle.a1d[i]));
}


// returns 1 if the thruster should be drawn
//         0 if it shouldn't
int model_should_render_engine_glow(int objnum, int bank_obj)
{
	if ((bank_obj == -1) || (objnum == -1))
		return 1;

	object *obj = &Objects[objnum];

	if (obj == NULL)
		return 1;

	if (obj->type == OBJ_SHIP) {
		ship_subsys *ssp;
		ship *shipp = &Ships[obj->instance];
		ship_info *si = &Ship_info[shipp->ship_info_index];

		char subname[MAX_NAME_LEN];
		// shipp->subsystems isn't always valid here so don't use it
		strncpy(subname, si->subsystems[bank_obj].subobj_name, MAX_NAME_LEN);

		ssp = GET_FIRST(&shipp->subsys_list);
		while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {
			if ( !strcmp(subname, ssp->system_info->name) ) {
				// this subsystem has 0 or less hits, ie. it's destroyed
				if ( ssp->current_hits <= 0 )
					return 0;

				// TODO: there could also be something here for disabled engine subsystems
				//       if the need arose.

				return 1;
			}
			ssp = GET_NEXT( ssp );
		}

	} else if (obj->type == OBJ_WEAPON) {
		// for weapons, if needed in the future
	}

	// default to render glow
	return 1;
}
