/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/2d.h $
 * $Revision: 2.77 $
 * $Date: 2006-04-20 06:32:01 $
 * $Author: Goober5000 $
 *
 * Header file for 2d primitives.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.76  2006/04/15 00:13:22  phreak
 * gr_flash_alpha(), much like gr_flash(), but allows an alpha value to be passed
 *
 * Revision 2.75  2006/01/30 06:40:49  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 * some basic lighting code cleanup
 *
 * Revision 2.74  2006/01/20 17:15:16  taylor
 * gr_*_bitmap_ex() stuff, D3D side is 100% untested to even compile
 * several other very minor changes as well
 *
 * Revision 2.73  2006/01/19 16:00:04  wmcoolmon
 * Lua debugging stuff; gr_bitmap_ex stuff for taylor
 *
 * Revision 2.72  2006/01/19 03:20:49  phreak
 * gr_render_buffer shouldn't be returning values if it's supposed to return void
 *
 * Revision 2.71  2006/01/18 16:14:04  taylor
 * allow gr_render_buffer() to take TMAP flags
 * let gr_render_buffer() render untextured polys (OGL only until some D3D people fix it on their side)
 * add MR_SHOW_OUTLINE_HTL flag so we easily render using HTL mode for wireframe views
 * make Interp_verts/Interp_norms/etc. dynamic and get rid of the extra htl_* versions
 *
 * Revision 2.70  2006/01/12 17:42:56  wmcoolmon
 * Even more scripting stuff.
 *
 * Revision 2.69  2005/12/29 08:08:33  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.68  2005/12/29 00:52:57  phreak
 * changed around aabitmap calls to accept a "mirror" parameter.  defaults to false, and is only true for mirrored briefing icons.
 * If the mirror param is true, then the picture is mirrored about the y-axis so left becomes right and vice versa.
 *
 * Revision 2.67  2005/10/30 06:44:57  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.66  2005/10/22 20:17:18  wmcoolmon
 * mission-set-nebula fixage; remainder of python code
 *
 * Revision 2.65  2005/10/16 11:20:43  taylor
 * use unsigned index buffers
 *
 * Revision 2.64  2005/08/20 20:34:50  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.63  2005/07/18 03:44:00  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.62  2005/07/13 02:50:47  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.61  2005/07/02 19:42:15  taylor
 * ton of non-standard resolution fixes
 *
 * Revision 2.60  2005/06/19 09:00:09  taylor
 * minor sanity checking for geometry_batcher
 * make particle batchers allocate dynamically
 * handle cases where a particle graphic couldn't be loaded
 *
 * Revision 2.59  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.58  2005/04/24 12:56:42  taylor
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
 * Revision 2.57  2005/04/24 03:02:43  wmcoolmon
 * Added resizing option to gr_shade. We can probably merge gr_shade and gr_rect; they do the same thing.
 *
 * Revision 2.56  2005/04/24 02:38:31  wmcoolmon
 * Moved gr_rect and gr_shade to be API-nonspecific as the OGL/D3D functions were virtually identical
 *
 * Revision 2.55  2005/04/12 02:04:56  phreak
 * gr_set_ambient_light() function for the ambient light sliders in FRED
 *
 * Revision 2.54  2005/04/05 05:53:16  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.53  2005/03/20 00:09:07  phreak
 * Added gr_draw_htl_line and gr_draw_htl sphere
 * There still needs to be D3D versions implemented, but OGL is done.
 * Follow that or ask phreak about how its implemented/
 *
 * Revision 2.52  2005/03/19 18:02:33  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.51  2005/03/16 01:35:58  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.50  2005/03/11 01:27:17  wmcoolmon
 * I like comments
 *
 * Revision 2.49  2005/03/10 08:00:04  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.48  2005/03/07 13:10:20  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.47  2005/03/06 11:23:44  wmcoolmon
 * RE-fixed stuff. Ogg support. Briefings.
 *
 * Revision 2.46  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.45  2005/02/27 07:08:22  wmcoolmon
 * More nonstandard res stuff
 *
 * Revision 2.44  2005/02/18 09:51:06  wmcoolmon
 * Updates for better nonstandard res support, as well as a fix to the Perseus crash bug I've been experiencing. Bobb, you might want to take a look at my change to grd3d.cpp
 *
 * Revision 2.43  2005/02/10 04:01:42  wmcoolmon
 * Low-level code for better hi-res support; better error reporting for vertex errors on model load.
 *
 * Revision 2.42  2005/02/04 20:06:04  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.41  2005/01/29 08:04:15  wmcoolmon
 * Ahh, the sweet smell of optimized code
 *
 * Revision 2.40  2005/01/14 05:28:57  wmcoolmon
 * gr_curve
 *
 * Revision 2.39  2005/01/01 11:24:22  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 2.38  2004/11/27 10:48:03  taylor
 * some fixes for position problems on the HUD in non-standard resolutions
 * few compiler warning fixes
 *
 * Revision 2.37  2004/11/23 00:10:06  taylor
 * try and protect the bitmap_entry stuff a bit better
 * fix the transparent support ship, again, but correctly this time
 *
 * Revision 2.36  2004/10/31 21:36:39  taylor
 * s/fisrt/first/g, bmpman merge
 *
 * Revision 2.35  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.34  2004/07/11 03:22:48  bobboau
 * added the working decal code
 *
 * Revision 2.33  2004/07/05 05:09:19  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.32  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.31  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.30  2004/05/25 00:37:26  wmcoolmon
 * Updated function calls for VC7 use
 *
 * Revision 2.28  2004/04/11 13:56:33  randomtiger
 * Adding batching functions here and there and into gr_screen for use with OGL when its ready.
 *
 * Revision 2.27  2004/04/01 15:31:21  taylor
 * don't use interface anis as ship textures
 *
 * Revision 2.26  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.25  2004/03/08 18:36:21  randomtiger
 * Added complete stub system to replace software.
 *
 * Revision 2.24  2004/03/07 23:07:20  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 2.23  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.22  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.21  2004/02/15 03:04:25  bobboau
 * fixed bug involving 3d shockwaves, note I wasn't able to compile the directshow file, so I ifdefed everything to an older version,
 * you shouldn't see anything diferent, as the ifdef should be set to the way it should be, if it isn't you will get a warning mesage during compile telling you how to fix it
 *
 * Revision 2.20  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless .
 _poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.19  2004/01/21 17:33:47  phreak
 * added MAX_DRAW_DISTANCE constant
 *
 * Revision 2.18  2003/12/16 20:42:36  phreak
 * created two constants
 * MAX_DRAW_DISTANCE = 250000
 * MIN_DRAW_DISTANCE = 1
 *
 * Revision 2.17  2003/11/17 04:25:55  bobboau
 * made the poly list dynamicly allocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.16  2003/11/16 04:09:24  Goober5000
 * language
 *
 * Revision 2.15  2003/11/11 03:56:11  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.14  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.13  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.12  2003/10/25 03:26:39  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.11  2003/10/24 17:35:05  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.10  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.9  2003/10/21 18:23:15  phreak
 * added gr_flip_window back in.  its used in FRED
 *
 * Revision 2.8  2003/10/18 02:46:45  phreak
 * changed gr_start_instance_matrix(void) to gr_start_instance_matrix((vector*, matrix*)
 *
 * Revision 2.7  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.6  2003/10/13 19:39:19  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.5  2003/10/10 03:59:40  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.4  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.3  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.2  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.5  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.1.2.4  2002/11/04 16:04:20  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.1.2.3  2002/11/04 03:02:28  randomtiger
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
 * Revision 2.1.2.2  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.1.2.1  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
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
 * 16    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 15    8/16/99 9:45a Jefff
 * changes to cursor management to allow a 2nd temporary cursor
 * 
 * 14    7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 13    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 12    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 11    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 8     1/24/99 11:36p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 7     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 6     12/18/98 1:49a Dave
 * Fixed Fred initialization problem resulting from hi-res mode changes.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 75    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 74    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 73    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 72    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 71    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 70    3/24/98 8:31a John
 * Added function to set gamma
 * 
 * 69    3/17/98 5:55p John
 * Added code to dump Glide frames.   Moved Allender's  "hack" code out of
 * FreeSpace.cpp into the proper place, graphics lib.
 * 
 * 68    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 67    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 66    2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 65    1/08/98 1:54p John
 * Added code to fix palette problems when Alt+Tabbing
 * 
 * 64    12/30/97 6:46p John
 * Added first rev of palette fade in out functions
 * 
 * 63    12/03/97 10:47a John
 * added functions to save/restore entire screens.
 * 
 * 62    12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 61    11/20/97 9:51a John
 * added code to force screen to 16-bit even if rendering 8.
 * 
 * 60    11/03/97 10:08p Hoffoss
 * Changed gr_get_string_size to utilize an optional length specifier, if
 * you want to use non-null terminated strings.
 * 
 * 59    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 58    10/03/97 10:02a John
 * added better comments for lines.
 * 
 * 57    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 56    9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 55    9/07/97 10:01p Lawrance
 * add in support for animating mouse pointer
 * 
 * 54    8/04/97 4:47p John
 * added gr_aascaler.
 * 
 * 53    7/16/97 3:07p John
 * 
 * 52    7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 51    6/25/97 2:35p John
 * added some functions to use the windows font for Fred.
 * 
 * 50    6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 49    6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 48    6/13/97 5:35p John
 * added some antialiased bitmaps and lines
 * 
 * 47    6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 46    6/11/97 4:11p John
 * addec function to get font height
 * 
 * 45    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 44    6/09/97 9:24a John
 * Changed the way fonts are set.
 * 
 * 43    6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 42    6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 41    5/29/97 3:09p John
 * Took out debug menu.  
 * Made software scaler draw larger bitmaps.
 * Optimized Direct3D some.
 * 
 * 40    5/14/97 10:53a John
 * fixed some discrepencies between d3d and software palette setting.
 * 
 * 39    5/12/97 3:09p John
 * fixed a stupid macro bug.
 * 
 * 38    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 37    4/28/97 4:46p John
 * 
 * 36    4/23/97 5:26p John
 * First rev of new debug console stuff.
 * 
 * 35    3/12/97 2:51p John
 * Added some test code for tmapper.  
 * 
 * 34    3/12/97 9:25a John
 * fixed a bug with zbuffering.  Reenabled it by default.
 * 
 * 33    3/04/97 3:36p John
 * took out old debug "h' key.   Made zbuffer flag bit bit field so you
 * can turn on/off each value.   Fixed a bug with turret rotation where
 * different handedness turrets wouldn't work.   Fixed a bug with two
 * large ships within each other's radius not rendering correctly.
 * 
 * 32    1/27/97 9:08a John
 * Added code to turn zbuffering on/off in call to g3_start_frame
 * 
 * 31    1/09/97 11:35a John
 * Added some 2d functions to get/put screen images.
 * 
 * 30    1/07/97 2:01p John
 * Fairly fast zbuffering for object sorting.
 * 
 * 29    1/06/97 2:44p John
 * Added in slow (but correct) zbuffering
 * 
 * 28    12/11/96 12:41p John
 * Added new code to draw 3d laser using 2d ellipses.
 * 
 * 27    12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 26    11/21/96 11:21a John
 * Made gr_get_string_size handle multi line text.
 * Took out gr_get_multiline_string_size
 * 
 * 25    11/20/96 10:01a Hoffoss
 * A few minor improvements.
 * 
 * 24    11/18/96 4:35p Allender
 * new 16bpp gradient functions
 * 
 * 23    11/18/96 12:36p John
 * Added code to dump screen to a PCX file.
 * 
 * 22    11/18/96 11:40a John
 * Added faster gr_set_color method.
 * 
 * 21    11/15/96 3:34p Allender
 * added bpp variable to the shader structure
 * 
 * 20    11/13/96 6:47p John
 * Added gr_flip function.
 * 
 * 19    11/13/96 10:10a John
 * Increases MAX_WIDTH & HEIGHT for Jasen's massive 1600x1200 display.
 * 
 * 18    10/30/96 10:36a Lawrance
 * added gr_diamond function
 * 
 * 17    10/26/96 2:56p John
 * Got gradient code working.
 * 
 * 16    10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H



/* ========================= pixel plotters =========================
In the 2d/texture mapper, bitmaps to be drawn will be passed by number.
The 2d function will call a bmpman function to get the bitmap into whatever
format it needs.  Then it will render.   The only pixels that will ever 
get drawn go thru the 2d/texture mapper libraries only.   This will make
supporting accelerators and psx easier.   Colors will always be set with
the color set functions.

gr_surface_flip()	switch onscreen, offscreen

gr_set_clip(x,y,w,h)	// sets the clipping region
gr_reset_clip(x,y,w,h)	// sets the clipping region
gr_set_color --? 8bpp, 15bpp?
gr_set_font(int fontnum)
// see GR_ALPHABLEND defines for values for alphablend_mode
// see GR_BITBLT_MODE defines for bitblt_mode.
// Alpha = scaler for intensity
gr_set_bitmap( int bitmap_num, int alphblend_mode, int bitblt_mode, float alpha )	
gr_set_shader( int value )  0=normal -256=darken, 256=brighten
gr_set_palette( ubyte * palette ) 

gr_clear()	// clears entire clipping region
gr_bitmap(x,y)
gr_bitmap_ex(x,y,w,h,sx,sy)
gr_rect(x,y,w,h)
gr_shade(x,y,w,h)
gr_string(x,y,char * text)
gr_line(x1,y1,x2,y2)

 
*/

#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "cfile/cfile.h"
#include "bmpman/bmpman.h"


#define MATRIX_TRANSFORM_TYPE_WORLD 0
#define MATRIX_TRANSFORM_TYPE_VIEW 1


//MAX_POLYGON_NORMS
#define MAX_POLYGON_TRI_POINTS 15000
extern const float Default_min_draw_distance;
extern const float Default_max_draw_distance;
extern float Min_draw_distance;
extern float Max_draw_distance;
extern int Gr_inited;

// This is a structure used by the shader to keep track
// of the values you want to use in the shade primitive.
typedef struct shader {
	uint	screen_sig;					// current mode this is in
	float	r,g,b,c;						// factors and constant
	ubyte	lookup[256];
} shader;

#define AC_TYPE_NONE		0		// Not an alphacolor
#define AC_TYPE_HUD		1		// Doesn't change hue depending on background.  Used for HUD stuff.
#define AC_TYPE_BLEND	2		// Changes hue depending on background.  Used for stars, etc.

// NEVER REFERENCE THESE VALUES OUTSIDE OF THE GRAPHICS LIBRARY!!!
// If you need to get the rgb values of a "color" struct call
// gr_get_colors after calling gr_set_colors_fast.
typedef struct color {
	uint		screen_sig;
	ubyte		red;
	ubyte		green;
	ubyte		blue;
	ubyte		alpha;
	ubyte		ac_type;							// The type of alphacolor.  See AC_TYPE_??? defines
	int		is_alphacolor;
	ubyte		raw8;
	int		alphacolor;
	int		magic;		
} color;

struct index_list{
	index_list():index_buffer(NULL){};
	~index_list(){if(index_buffer)vm_free(index_buffer);};
	void allocate_index_buffer(int size){if(index_buffer)vm_free(index_buffer); index_buffer = (ushort*)vm_malloc(sizeof(ushort) * size);};
	ushort *index_buffer;
};

//this should be basicly just like it is in the VB
//a list of triangles and there assosiated normals

struct poly_list{
	poly_list(): n_prim(0), n_verts(0), vert(NULL), norm(NULL), currently_allocated(0) {};
	~poly_list();
	void allocate(int size);
	void make_index_buffer();
	poly_list& operator = (poly_list&);
	int n_prim;
	int n_verts;
	vertex *vert;
	vec3d *norm;
private:
	int currently_allocated;
};

class geometry_batcher{
	int n_to_render;//the number of primitives to render
	int n_allocated;//the number of verts allocated
	vertex* vert;
	vertex** vert_list;//V's stupid rendering functions need this
	void allocate_internal(int n_verts);
	//makes sure we have enough space in the memory buffer for the geometry we are about to put into it
	//you need to figure out how many verts are going to be requiered
public:
	geometry_batcher():n_to_render(0),n_allocated(0),vert(NULL),vert_list(NULL){};
	~geometry_batcher() { if (vert != NULL) vm_free(vert); if (vert_list != NULL) vm_free(vert_list);};

	void add_allocate(int quad, int n_tri=0);//add this many without loseing what we have
	void allocate(int quad, int n_tri=0);
	//everything exept the draw tri comand requiers the same amount of space
	//so just tell it how many draw_* comands you are going to need seperateing out
	//draw_tri if you for some reason need it
	//this must be called every time the geometry batcher is to be used
	//before the first draw_* comand and after any render comand

	void draw_bitmap(vertex *position, float rad, float depth=0);
	//draws a bitmap into the geometry batcher
	void draw_bitmap(vertex *position, float rad, float angle, float depth);
	//draws a rotated bitmap
	void draw_tri(vertex* verts);
	//draws a simple 3 vert polygon
	void draw_quad(vertex* verts);
	//draws a simple 4 vert polygon
	void draw_beam(vec3d*start,vec3d*end, float width, float intinsity = 1.0f);
	//draws a beam
	float draw_laser(vec3d *p0,float width1,vec3d *p1,float width2, int r, int g, int b);
	//draws a laser

	void render(int flags);
	//draws all of the batched geometry to the back buffer and flushes the cache
	//accepts tmap flags so you can use anything you want realy

	// determine if we even need to try and render this (helpful for particle system)
	int need_to_render() { return n_to_render; };

	void operator =(int){};
};

struct batch_item{
	batch_item():texture(-1){};

	geometry_batcher batch;
	int texture;
};


struct colored_vector{
	colored_vector():pad(1.0f){};
	vec3d vec;
	float pad;	//needed so I can just memcpy it in d3d
	ubyte color[4];
};

bool same_vert(vertex *v1, vertex *v2, vec3d *n1, vec3d *n2);

//finds the first occorence of a vertex within a poly list
int find_first_index(poly_list *plist, int idx);

//given a list (plist) and an indexed list (v) find the index within the indexed list that the vert at position idx within list is at 
int find_first_index_vb(poly_list *plist, int idx, poly_list *v);


struct line_list{
	int n_line;
	vertex *vert;
};

struct light;


#define GR_ALPHABLEND_NONE			0		// no blending
#define GR_ALPHABLEND_FILTER		1		// 50/50 mix of foreground, background, using intensity as alpha

#define GR_BITBLT_MODE_NORMAL		0		// Normal bitblting
#define GR_BITBLT_MODE_RLE			1		// RLE would be faster

// fog modes
#define GR_FOGMODE_NONE				0		// set this to turn off fog
#define GR_FOGMODE_FOG				1		// linear fog

typedef struct screen {
	uint	signature;			// changes when mode or palette or width or height changes
	int	max_w, max_h;		// Width and height
	int max_w_unscaled, max_h_unscaled;		// Width and height, should be 1024x768 or 640x480 in non-standard resolutions
	int	save_max_w, save_max_h;		// Width and height
	int	res;					// GR_640 or GR_1024
	int	mode;					// What mode gr_init was called with.
	float	aspect;				// Aspect ratio
	int	rowsize;				// What you need to add to go to next row (includes bytes_per_pixel)
	int	bits_per_pixel;	// How many bits per pixel it is. (7,8,15,16,24,32)
	int	bytes_per_pixel;	// How many bytes per pixel (1,2,3,4)
	int	offset_x, offset_y;		// The offsets into the screen
	int offset_x_unscaled, offset_y_unscaled;	// Offsets into the screen, in 1024x768 or 640x480 dimensions
	int	clip_width, clip_height;
	int clip_width_unscaled, clip_height_unscaled;	// Height and width of clip aread, in 1024x768 or 640x480 dimensions

	float fog_near, fog_far;

	// the clip_l,r,t,b are used internally.  left and top are
	// actually always 0, but it's nice to have the code work with
	// arbitrary clipping regions.
	int		clip_left, clip_right, clip_top, clip_bottom;
	// same as above except in 1024x768 or 640x480 dimensions
	int		clip_left_unscaled, clip_right_unscaled, clip_top_unscaled, clip_bottom_unscaled;

	int		current_alphablend_mode;		// See GR_ALPHABLEND defines above
	int		current_bitblt_mode;				// See GR_BITBLT_MODE defines above
	int		current_fog_mode;					// See GR_FOGMODE_* defines above
	int		current_bitmap;
	color		current_color;
	color		current_fog_color;				// current fog color
	color		current_clear_color;				// current clear color
	shader	current_shader;
	float		current_alpha;
	void		*offscreen_buffer;				// NEVER ACCESS!  This+rowsize*y = screen offset
	void		*offscreen_buffer_base;			// Pointer to lowest address of offscreen buffer

	int custom_size;
	int		rendering_to_texture;		//wich texture we are rendering to, -1 if the back buffer
	int		rendering_to_face;			//wich face of the texture we are rendering to, -1 if the back buffer

	int static_environment_map;
	int dynamic_environment_map;

	bool recording_state_block;
	int current_state_block;

	void (*gf_start_state_block)();
	int (*gf_end_state_block)();
	void (*gf_set_state_block)(int);

	//switch onscreen, offscreen
	void (*gf_flip)();
	void (*gf_flip_window)(uint _hdc, int x, int y, int w, int h );

	// Sets the current palette
	void (*gf_set_palette)(ubyte * new_pal, int restrict_alphacolor);

	// Fade the screen in/out
	void (*gf_fade_in)(int instantaneous);
	void (*gf_fade_out)(int instantaneous);

	// Flash the screen
	void (*gf_flash)( int r, int g, int b );
	void (*gf_flash_alpha)(int r, int g, int b, int a);

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h, bool resize);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	void (*gf_set_color)( int r, int g, int b );
	void (*gf_get_color)( int * r, int * g, int * b );
	void (*gf_init_color)( color * dst, int r, int g, int b );

	void (*gf_init_alphacolor)( color * dst, int r, int g, int b, int alpha, int type );
	void (*gf_set_color_fast)( color * dst );

	void (*gf_set_font)(int fontnum);

	// Sets the current bitmap
	void (*gf_set_bitmap)( int bitmap_num, int alphablend, int bitbltmode, float alpha);

	// Call this to create a shader.   
	// This function takes a while, so don't call it once a frame!
	// r,g,b, and c should be between -1.0 and 1.0f

	// The matrix is used as follows:
	// Dest(r) = Src(r)*r + Src(g)*r + Src(b)*r + c;
	// Dest(g) = Src(r)*g + Src(g)*g + Src(b)*g + c;
	// Dest(b) = Src(r)*b + Src(g)*b + Src(b)*b + c;
	// For instance, to convert to greyscale, use
	// .3 .3 .3  0
	// To turn everything green, use:
	//  0 .3  0  0
	void (*gf_create_shader)(shader * shade, float r, float g, float b, float c );

	// Initialize the "shader" by calling gr_create_shader()
	// Passing a NULL makes a shader that turns everything black.
	void (*gf_set_shader)( shader * shade );

	// clears entire clipping region to current color
	void (*gf_clear)();

	// void (*gf_bitmap)(int x, int y, bool resize);
	void (*gf_bitmap_ex)(int x, int y, int w, int h, int sx, int sy, bool resize);

	void (*gf_aabitmap)(int x, int y, bool resize, bool mirror);
	void (*gf_aabitmap_ex)(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror);

//	void (*gf_rect)(int x, int y, int w, int h,bool resize);
//	void (*gf_shade)(int x, int y, int w, int h);
	void (*gf_string)(int x, int y, char * text,bool resize);

	// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
	void (*gf_gradient)(int x1, int y1, int x2, int y2, bool resize);
 
	void (*gf_circle)(int x, int y, int r, bool resize);
	void (*gf_curve)(int x, int y, int r, int direction);

	// Integer line. Used to draw a fast but pixely line.  
	void (*gf_line)(int x1, int y1, int x2, int y2, bool resize);

	// Draws an antialiased line is the current color is an 
	// alphacolor, otherwise just draws a fast line.  This
	// gets called internally by g3_draw_line.   This assumes
	// the vertex's are already clipped, so call g3_draw_line
	// not this if you have two 3d points.
	void (*gf_aaline)(vertex *v1, vertex *v2);

	void (*gf_pixel)( int x, int y, bool resize );

	// Scales current bitmap between va and vb with clipping
	void (*gf_scaler)(vertex *va, vertex *vb );

	// Scales current bitmap between va and vb with clipping, draws an aabitmap
	void (*gf_aascaler)(vertex *va, vertex *vb );

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_tmapper)(int nv, vertex *verts[], uint flags );

	void (*gf_tmapper_batch_3d_unlit)( int nverts, vertex *verts, uint flags);	

	// dumps the current screen to a file
	void (*gf_print_screen)(char * filename);

	// Call once before rendering anything.
	void (*gf_start_frame)();

	// Call after rendering is over.
	void (*gf_stop_frame)();

	// Retrieves the zbuffer mode.
	int (*gf_zbuffer_get)();

	// Sets mode.  Returns previous mode.
	int (*gf_zbuffer_set)(int mode);

	// Clears the zbuffer.  If use_zbuffer is FALSE, then zbuffering mode is ignored and zbuffer is always off.
	void (*gf_zbuffer_clear)(int use_zbuffer);
	
	// Saves screen. Returns an id you pass to restore and free.
	int (*gf_save_screen)();
	
	// Resets clip region and copies entire saved screen to the screen.
	void (*gf_restore_screen)(int id);

	// Frees up a saved screen.
	void (*gf_free_screen)(int id);

	// CODE FOR DUMPING FRAMES TO A FILE
	// Begin frame dumping
	void (*gf_dump_frame_start)( int first_frame_number, int nframes_between_dumps );

	// Dump the current frame to file
	void (*gf_dump_frame)();

	// Dump the current frame to file
	void (*gf_dump_frame_stop)();

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// Lock/unlock the screen
	// Returns non-zero if sucessful (memory pointer)
	uint (*gf_lock)();
	void (*gf_unlock)();

	// grab a region of the screen. assumes data is large enough
	void (*gf_get_region)(int front, int w, int h, ubyte *data);

	// set fog attributes
	void (*gf_fog_set)(int fog_mode, int r, int g, int b, float fog_near, float fog_far);	

	// get the current pixel color in the framebuffer 
	void (*gf_get_pixel)(int x, int y, int *r, int *g, int *b);

	// poly culling
	void (*gf_set_cull)(int cull);

	// cross fade
	void (*gf_cross_fade)(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);

	// filtering
	void (*gf_filter_set)(int filter);

	// set a texture into cache. for sectioned bitmaps, pass in sx and sy to set that particular section of the bitmap
	int (*gf_tcache_set)(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int force, int stage);	

	// preload a bitmap into texture memory
	int (*gf_preload)(int bitmap_num, int is_aabitmap);

	// set the color to be used when clearing the background
	void (*gf_set_clear_color)(int r, int g, int b);

	// Here be the bitmap functions
	void (*gf_bm_free_data)(int n);
	void (*gf_bm_create)(int n);
	int (*gf_bm_load)(ubyte type, int n, char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *c_type, int *mm_lvl, int *size);
	void (*gf_bm_init)(int n);
	void (*gf_bm_page_in_start)();
	int (*gf_bm_lock)(char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags);

	bool (*gf_bm_make_render_target)(int n, int &x_res, int &y_res, int flags );
	bool (*gf_bm_set_render_target)(int n, int face);

	void (*gf_translate_texture_matrix)(int unit, vec3d *shift);
	void (*gf_push_texture_matrix)(int unit);
	void (*gf_pop_texture_matrix)(int unit);

	void (*gf_set_texture_addressing)(int);

	int	 (*gf_make_buffer)(poly_list*, uint flags);
	void (*gf_destroy_buffer)(int);
	void (*gf_set_buffer)(int);
	void (*gf_render_buffer)(int, int, ushort*, int);
	int	 (*gf_make_flat_buffer)(poly_list*);
	int	 (*gf_make_line_buffer)(line_list*);
	

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

	int	 (*gf_make_light)(light*, int, int );
	void (*gf_modify_light)(light*, int, int );
	void (*gf_destroy_light)(int);
	void (*gf_set_light)(light*);
	void (*gf_reset_lighting)();
	void (*gf_set_ambient_light)(int,int,int);

	void (*gf_lighting)(bool,bool);
	void (*gf_center_alpha)(int);

	void (*gf_start_clip_plane)();
	void (*gf_end_clip_plane)();

	void (*gf_zbias)(int zbias);
	void (*gf_setup_background_fog)(bool);

	void (*gf_set_fill_mode)(int);
	void (*gf_set_texture_panning)(float u, float v, bool enable);

	void (*gf_draw_line_list)(colored_vector*lines, int num);

	void (*gf_draw_htl_line)(vec3d *start, vec3d* end);
	void (*gf_draw_htl_sphere)(float rad);

//	void (*gf_set_environment_mapping)(int i);

/*	void (*gf_begin_sprites)();//does prep work for sprites
	void (*gf_draw_sprite)(vec3d*);//draws a sprite
	void (*gf_display_sprites))();//actualy darws the drawen sprites
	void (*gf_end_sprites)();//clears the lists and stuff
*/
} screen;

// cpu types
extern int Gr_amd3d;
extern int Gr_katmai;
extern int Gr_cpu;	
extern int Gr_mmx;

// handy macro
#define GR_MAYBE_CLEAR_RES(bmap)		do  { int bmw = -1; int bmh = -1; if(bmap != -1){ bm_get_info( bmap, &bmw, &bmh, NULL, NULL, NULL); if((bmw != gr_screen.max_w) || (bmh != gr_screen.max_h)){gr_clear();} } else {gr_clear();} } while(0);

//Window's interface to set up graphics:
//--------------------------------------
// Call this at application startup

// # Software Re-added by Kazan --- THIS HAS TO STAY -- It is used by standalone!
#define GR_STUB					(100)		
#define GR_DIRECT3D				(102)		// Use Direct3d hardware renderer
#define GR_OPENGL				(104)		// Use OpenGl hardware renderer

// resolution constants   - always keep resolutions in ascending order and starting from 0  
#define GR_NUM_RESOLUTIONS			2
#define GR_640							0		// 640 x 480
#define GR_1024						1		// 1024 x 768

extern bool gr_init(int res, int mode, int depth = 16, int fred_x = -1, int fred_y = -1 );

// Call this when your app ends.
extern void gr_close();

extern screen gr_screen;

#define GR_FILL_MODE_WIRE 1
#define GR_FILL_MODE_SOLID 2

#define GR_ZBUFF_NONE	0
#define GR_ZBUFF_WRITE	(1<<0)
#define GR_ZBUFF_READ	(1<<1)
#define GR_ZBUFF_FULL	(GR_ZBUFF_WRITE|GR_ZBUFF_READ)

bool gr_unsize_screen_pos(int *x, int *y);
bool gr_resize_screen_pos(int *x, int *y);
bool gr_unsize_screen_posf(float *x, float *y);
bool gr_resize_screen_posf(float *x, float *y);

// Returns -1 if couldn't init font, otherwise returns the
// font id number.  If you call this twice with the same typeface,
// it will return the same font number both times.  This font is
// then set to be the current font, and default font if none is 
// yet specified.
int gr_init_font( char * typeface );

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void _cdecl gr_printf( int x, int y, char * format, ... );
// same as above but doesn't resize for non-standard resolutions
extern void _cdecl gr_printf_no_resize( int x, int y, char * format, ... );

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size( int *w, int *h, char * text, int len = 9999 );

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(char *name, ubyte *palette, int restrict_to_128 = 0);

// These two functions use a Windows mono font.  Only for use
// in the editor, please.
void gr_get_string_size_win(int *w, int *h, char *text);
void gr_string_win(int x, int y, char *s );

// set the mouse pointer to a specific bitmap, used for animating cursors
#define GR_CURSOR_LOCK		1
#define GR_CURSOR_UNLOCK	2
void gr_set_cursor_bitmap(int n, int lock = 0);
void gr_unset_cursor_bitmap(int n);
int gr_get_cursor_bitmap();
extern int Web_cursor_bitmap;

// Called by OS when application gets/looses focus
extern void gr_activate(int active);

// Sets up resolution
void gr_init_res(int res, int mode, int fredx = -1, int fredy = -1);

#define GR_CALL(x)			(*x)

// These macros make the function indirection look like the
// old Descent-style gr_xxx calls.

#define gr_print_screen		GR_CALL(gr_screen.gf_print_screen)

//#define gr_flip				GR_CALL(gr_screen.gf_flip)
void gr_flip();
#define gr_flip_window		GR_CALL(gr_screen.gf_flip_window)

//#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
__inline void gr_set_clip(int x, int y, int w, int h, bool resize=true)
{
	(*gr_screen.gf_set_clip)(x,y,w,h,resize);
}
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)
#define gr_set_font			GR_CALL(gr_screen.gf_set_font)

#define gr_init_color		GR_CALL(gr_screen.gf_init_color)
//#define gr_init_alphacolor	GR_CALL(gr_screen.gf_init_alphacolor)
__inline void gr_init_alphacolor( color * dst, int r, int g, int b, int alpha, int type=AC_TYPE_HUD )
{
	(*gr_screen.gf_init_alphacolor)(dst, r, g, b, alpha,type );
}

#define gr_set_color			GR_CALL(gr_screen.gf_set_color)
#define gr_get_color			GR_CALL(gr_screen.gf_get_color)
#define gr_set_color_fast	GR_CALL(gr_screen.gf_set_color_fast)

//#define gr_set_bitmap			GR_CALL(gr_screen.gf_set_bitmap)
__inline void gr_set_bitmap( int bitmap_num, int alphablend=GR_ALPHABLEND_NONE, int bitbltmode=GR_BITBLT_MODE_NORMAL, float alpha=1.0f )
{
	(*gr_screen.gf_set_bitmap)(bitmap_num, alphablend, bitbltmode, alpha);
}

#define gr_create_shader	GR_CALL(gr_screen.gf_create_shader)
#define gr_set_shader		GR_CALL(gr_screen.gf_set_shader)
#define gr_clear				GR_CALL(gr_screen.gf_clear)
//#define gr_aabitmap			GR_CALL(gr_screen.gf_aabitmap)
__inline void gr_aabitmap(int x, int y, bool resize = true, bool mirror = false)
{
	(*gr_screen.gf_aabitmap)(x,y,resize,mirror);
}
//#define gr_aabitmap_ex		GR_CALL(gr_screen.gf_aabitmap_ex)
__inline void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize = true, bool mirror = false)
{
	(*gr_screen.gf_aabitmap_ex)(x,y,w,h,sx,sy,resize,mirror);
}
//#define gr_bitmap_ex		GR_CALL(gr_screen.gf_bitmap_ex)
__inline void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize = true)
{
	(*gr_screen.gf_bitmap_ex)(x, y, w, h, sx, sy, resize);
}

void gr_rect(int x, int y, int w, int h, bool resize = true);
void gr_shade(int x, int y, int w, int h, bool resize = true);

//#define gr_shade				GR_CALL(gr_screen.gf_shade)
//#define gr_string				GR_CALL(gr_screen.gf_string)
__inline void gr_string(int x, int y, char* string, bool resize = true)
{
	(*gr_screen.gf_string)(x,y,string,resize);
}

//#define gr_circle				GR_CALL(gr_screen.gf_circle)
__inline void gr_circle(int xc, int yc, int d, bool resize = true)
{
	(*gr_screen.gf_circle)(xc,yc,d,resize);
}
#define gr_curve				GR_CALL(gr_screen.gf_curve)

//#define gr_line				GR_CALL(gr_screen.gf_line)
__inline void gr_line(int x1, int y1, int x2, int y2, bool resize = true)
{
	(*gr_screen.gf_line)(x1, y1, x2, y2, resize);
}

#define gr_aaline				GR_CALL(gr_screen.gf_aaline)

//#define gr_pixel				GR_CALL(gr_screen.gf_pixel)
__inline void gr_pixel(int x, int y, bool resize = true)
{
	(*gr_screen.gf_pixel)(x, y, resize);
}
#define gr_scaler				GR_CALL(gr_screen.gf_scaler)
#define gr_aascaler			GR_CALL(gr_screen.gf_aascaler)
#define gr_tmapper			GR_CALL(gr_screen.gf_tmapper)
#define gr_tmapper_batch_3d_unlit	GR_CALL(gr_screen.gf_tmapper_batch_3d_unlit)

//#define gr_gradient			GR_CALL(gr_screen.gf_gradient)
__inline void gr_gradient(int x1, int y1, int x2, int y2, bool resize = true)
{
	(*gr_screen.gf_gradient)(x1, y1, x2, y2, resize);
}


#define gr_fade_in			GR_CALL(gr_screen.gf_fade_in)
#define gr_fade_out			GR_CALL(gr_screen.gf_fade_out)
#define gr_flash			GR_CALL(gr_screen.gf_flash)
#define gr_flash_alpha		GR_CALL(gr_screen.gf_flash_alpha)

#define gr_zbuffer_get		GR_CALL(gr_screen.gf_zbuffer_get)
#define gr_zbuffer_set		GR_CALL(gr_screen.gf_zbuffer_set)
#define gr_zbuffer_clear	GR_CALL(gr_screen.gf_zbuffer_clear)

#define gr_save_screen		GR_CALL(gr_screen.gf_save_screen)
#define gr_restore_screen	GR_CALL(gr_screen.gf_restore_screen)
#define gr_free_screen		GR_CALL(gr_screen.gf_free_screen)

#define gr_dump_frame_start	GR_CALL(gr_screen.gf_dump_frame_start)
#define gr_dump_frame_stop		GR_CALL(gr_screen.gf_dump_frame_stop)
#define gr_dump_frame			GR_CALL(gr_screen.gf_dump_frame)

#define gr_set_gamma			GR_CALL(gr_screen.gf_set_gamma)

#define gr_lock				GR_CALL(gr_screen.gf_lock)
#define gr_unlock				GR_CALL(gr_screen.gf_unlock)

#define gr_get_region		GR_CALL(gr_screen.gf_get_region)

//#define gr_fog_set			GR_CALL(gr_screen.gf_fog_set)
__inline void gr_fog_set(int fog_mode, int r, int g, int b, float fog_near = -1.0f, float fog_far = -1.0f)
{
	(*gr_screen.gf_fog_set)(fog_mode, r, g, b, fog_near, fog_far);
}

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)

#define gr_cross_fade		GR_CALL(gr_screen.gf_cross_fade)

#define gr_filter_set		GR_CALL(gr_screen.gf_filter_set)

//#define gr_tcache_set		GR_CALL(gr_screen.gf_tcache_set)
__inline int gr_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int force = 0, int stage = 0)
{
	return (*gr_screen.gf_tcache_set)(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, force, stage);
}

#define gr_preload			GR_CALL(gr_screen.gf_preload)

#define gr_set_clear_color	GR_CALL(gr_screen.gf_set_clear_color)

#define gr_translate_texture_matrix		GR_CALL(gr_screen.gf_translate_texture_matrix)
#define gr_push_texture_matrix			GR_CALL(gr_screen.gf_push_texture_matrix)
#define gr_pop_texture_matrix			GR_CALL(gr_screen.gf_pop_texture_matrix)


// Here be the bitmap functions
#define gr_bm_free_data				GR_CALL(*gr_screen.gf_bm_free_data)
#define gr_bm_create				GR_CALL(*gr_screen.gf_bm_create)
#define gr_bm_init					GR_CALL(*gr_screen.gf_bm_init)
__inline int gr_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp = NULL, int *w = 0, int *h = 0, int *bpp = 0, ubyte *c_type = 0, int *mm_lvl = 0, int *size = 0)
{
	return (*gr_screen.gf_bm_load)(type, n, filename, img_cfp, w, h, bpp, c_type, mm_lvl, size);
}
#define gr_bm_page_in_start			GR_CALL(*gr_screen.gf_bm_page_in_start)
#define gr_bm_lock					GR_CALL(*gr_screen.gf_bm_lock)          

#define gr_bm_make_render_target					GR_CALL(*gr_screen.gf_bm_make_render_target)          
//#define gr_bm_set_render_target					GR_CALL(*gr_screen.gf_bm_set_render_target)          
__inline bool gr_bm_set_render_target(int n, int face = -1)
{
	return (*gr_screen.gf_bm_set_render_target)(n, face);
}

#define gr_set_texture_addressing					 GR_CALL(*gr_screen.gf_set_texture_addressing)            

#define gr_make_buffer					 GR_CALL(*gr_screen.gf_make_buffer)            
#define gr_destroy_buffer				 GR_CALL(*gr_screen.gf_destroy_buffer)
__inline void gr_render_buffer(int start, int n_prim, ushort* index_buffer, int flags = TMAP_FLAG_TEXTURED)
{
	(*gr_screen.gf_render_buffer)(start, n_prim, index_buffer, flags);
}

#define gr_set_buffer				 GR_CALL(*gr_screen.gf_set_buffer)      
      
#define gr_make_flat_buffer					 GR_CALL(*gr_screen.gf_make_flat_buffer)            
#define gr_make_line_buffer					 GR_CALL(*gr_screen.gf_make_line_buffer)            

#define gr_set_proj_matrix				 GR_CALL(*gr_screen.gf_set_proj_matrix)            
#define gr_end_proj_matrix				 GR_CALL(*gr_screen.gf_end_proj_matrix)            
#define gr_set_view_matrix				 GR_CALL(*gr_screen.gf_set_view_matrix)            
#define gr_end_view_matrix				 GR_CALL(*gr_screen.gf_end_view_matrix)            
#define gr_push_scale_matrix			 GR_CALL(*gr_screen.gf_push_scale_matrix)            
#define gr_pop_scale_matrix				 GR_CALL(*gr_screen.gf_pop_scale_matrix)            
#define gr_start_instance_matrix		 GR_CALL(*gr_screen.gf_start_instance_matrix)            
#define gr_start_angles_instance_matrix	 GR_CALL(*gr_screen.gf_start_angles_instance_matrix)            
#define gr_end_instance_matrix			 GR_CALL(*gr_screen.gf_end_instance_matrix)            

#define	gr_make_light GR_CALL			(*gr_screen.gf_make_light)
#define	gr_modify_light GR_CALL			(*gr_screen.gf_modify_light)
#define	gr_destroy_light GR_CALL		(*gr_screen.gf_destroy_light)
#define	gr_set_light GR_CALL			(*gr_screen.gf_set_light)
#define gr_reset_lighting GR_CALL		(*gr_screen.gf_reset_lighting)
#define gr_set_ambient_light			GR_CALL(*gr_screen.gf_set_ambient_light)

#define	gr_set_lighting GR_CALL			(*gr_screen.gf_lighting)
#define	gr_center_alpha GR_CALL			(*gr_screen.gf_center_alpha)

#define	gr_start_clip GR_CALL			(*gr_screen.gf_start_clip_plane)
#define	gr_end_clip GR_CALL				(*gr_screen.gf_end_clip_plane)

#define	gr_zbias GR_CALL				(*gr_screen.gf_zbias)
#define	gr_set_fill_mode GR_CALL		(*gr_screen.gf_set_fill_mode)
#define	gr_set_texture_panning GR_CALL	(*gr_screen.gf_set_texture_panning)

#define	gr_start_state_block GR_CALL	(*gr_screen.gf_start_state_block)
#define	gr_end_state_block GR_CALL		(*gr_screen.gf_end_state_block)
#define	gr_set_state_block GR_CALL		(*gr_screen.gf_set_state_block)

//#define	gr_set_environment_mapping GR_CALL	(*gr_screen.gf_set_environment_mapping)

#define gr_setup_background_fog GR_CALL	(*gr_screen.gf_setup_background_fog)

#define gr_draw_line_list GR_CALL	(*gr_screen.gf_draw_line_list)

#define gr_draw_htl_line GR_CALL(*gr_screen.gf_draw_htl_line)
#define gr_draw_htl_sphere GR_CALL(*gr_screen.gf_draw_htl_sphere)

/*
#define	gr_begin_sprites GR_CALL		(*gr_screen.gf_begin_sprites)
#define	gr_draw_sprites GR_CALL			(*gr_screen.gf_draw_sprites)
#define	gr_end sprites GR_CALL			(*gr_screen.gf_end_sprites)
#define	gr_display_sprites GR_CALL		(*gr_screen.gf_display_sprites)
*/

// new bitmap functions
void gr_bitmap(int x, int y, bool resize = true);
void gr_bitmap_list(bitmap_2d_list* list, int n_bm, bool allow_scaling);
void gr_bitmap_list(bitmap_rect_list* list, int n_bm, bool allow_scaling);

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(vec3d **pts, int num_pts, int thickness,bool resize=true);

#define VERTEX_FLAG_POSITION	 (1<<0)	
#define VERTEX_FLAG_RHW			 (1<<1)	//incompatable with the next normal
#define VERTEX_FLAG_NORMAL		 (1<<2)	
#define VERTEX_FLAG_DIFUSE		 (1<<3)	
#define VERTEX_FLAG_SPECULAR	 (1<<4)	
#define VERTEX_FLAG_UV1			 (1<<5)	//how many UV coords, only use one of these
#define VERTEX_FLAG_UV2			 (1<<6)	
#define VERTEX_FLAG_UV3			 (1<<7)	
#define VERTEX_FLAG_UV4			 (1<<8)	

void poly_tsb_calc(vertex *v0, vertex *v1, vertex *v2, vec3d *o_norm, vec3d *o_stan, vec3d *o_ttan);

#endif

