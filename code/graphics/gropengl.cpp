

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.cpp $
 * $Revision: 2.174.2.29 $
 * $Date: 2007-11-22 17:51:35 $
 * $Author: phreak $
 *
 * Code that uses the OpenGL graphics library
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.174.2.28  2007/11/22 05:11:38  taylor
 * try to deal better with gamma setting/resetting when minimizing/restoring the game (Mantis #1210)
 *
 * Revision 2.174.2.27  2007/10/04 16:18:18  taylor
 * get rid of some old/obsolete items (Mantis #1489 and #1497)
 *
 * Revision 2.174.2.26  2007/03/22 20:14:16  taylor
 * various bits of bmpman cleanup
 * be sure to clean all three possible buffers with OGL init
 * fix a couple of bmpman loading bugs that messed up animations
 * fix bmpman bug that didn't properly account for free'd texture ram count with unload_fast
 *
 * Revision 2.174.2.25  2007/02/12 07:29:51  taylor
 * fix stupid bug (Mantis #1275)
 *
 * Revision 2.174.2.24  2007/02/11 10:01:11  taylor
 * remove cloakmap stuff, we'll need to redo this later on anyway
 * don't need the AVI movie hacks any longer, so remove it
 * deal better with a strange Windows buffer swap issue
 * various bits of cleanup and performance improvements
 * fix for gr_opengl_flash() that I had screwed up earlier
 *
 * Revision 2.174.2.23  2007/02/10 20:23:23  taylor
 * make sure that we don't set the 2d view matrix and then not reset it (Mantis #1269)
 * clean up some of the fullneb mess that was causing some initial setup issues (colors wrong, etc.)
 *
 * Revision 2.174.2.22  2006/12/26 05:25:18  taylor
 * lots of little cleanup, stale code removal, and small performance adjustments
 * get rid of the default combine texture state, we don't need it in general, and it can screw up fonts
 * get rid of the secondary color support, it doesn't do much in non-HTL mode, screws up various things, and has long since been obsolete but material setup
 * get rid of the old gamma setup, it actually conflicts with newer gamma support
 * default texture wrapping to edge clamp
 * do second gr_clear() on init to be sure and catch double-buffer
 * make sure that our active texture will always get reset to 0, rather than leaving it at whatever was used last
 * fixed that damn FBO bug from it hanging on textures and causing some rendering errors for various people
 * only lock verts once in HTL model rendering
 *
 * Revision 2.174.2.21  2006/12/07 18:07:51  taylor
 * get rid of GL_activate and GL_deactivate, it was just Glide ported stuff that we never used and never needed
 * handle window/fullscreen/minimize changes better, fixes cursor handling mostly (Mantis bug #1146)
 * don't flush on gamma change, we don't really need to any more
 *
 * Revision 2.174.2.20  2006/11/15 00:47:57  taylor
 * properly support the updated window create code (all told: should take of of Mantis bugs #542, #624, #1140, and possibly #962 and #1124)
 *
 * Revision 2.174.2.19  2006/11/06 05:21:32  taylor
 * enable/disable alpha test based on blend mode (found this hidden in an old tree, may help with blending the newer alphablend mode)
 *
 * Revision 2.174.2.18  2006/10/27 06:44:35  taylor
 * grrr ... fix dos EOL chars
 *
 * Revision 2.174.2.17  2006/10/24 13:39:26  taylor
 * don't require hardware GL if running FRED (this is mainly for my benefit since I always forget to add that locally)
 *
 * Revision 2.174.2.16  2006/10/01 19:24:45  taylor
 * bit of cleanup (technically the vertex buffer stuff is part of a much larger change slated for post-3.6.9, but this should be a little faster)
 *
 * Revision 2.174.2.15  2006/09/24 22:50:03  taylor
 * clean up the function ptrs so that it's a bit easier to read
 *
 * Revision 2.174.2.14  2006/09/24 13:26:01  taylor
 * minor clean and code optimizations
 * clean up view/proj matrix fubar that made us need far more matrix levels that actually needed (partial fix for Mantis #563)
 * add debug safety check to make sure that we don't use more than 2 proj matrices (all that GL is required to support)
 * set up a texture matrix for the env map to that it doesn't move/look funky
 *
 * Revision 2.174.2.13  2006/09/20 04:58:13  taylor
 * some gamma ramp fixage, still hasn't gotten a steller review from DaBrain but it does work much better than before, so I'll tweak it later if need be
 *
 * Revision 2.174.2.12  2006/08/29 07:27:43  taylor
 * allow W32 init fallback if desktop depth is less than requested game depth (it may look like crap, but at least it will work)
 *
 * Revision 2.174.2.11  2006/08/22 05:41:35  taylor
 * clean up the grstub mess (for work on standalone server, and just for sanity sake)
 * move color and shader functions to 2d.cpp since they are exactly the same everywhere
 * don't bother with the function pointer for gr_set_font(), it's the same everywhere anyway
 *
 * Revision 2.174.2.10  2006/08/19 04:23:56  taylor
 * OMG!  MEMLEAK!!!!  (maybe no one will notice that it was my fault ;))
 *
 * Revision 2.174.2.9  2006/08/09 14:40:10  taylor
 * very small math optimization
 *
 * Revision 2.174.2.8  2006/07/17 01:05:49  taylor
 * only do i2fl() on the font bitmap once per string rather than doing it per-letter (basically unnoticable performance boost)
 * before doing alpha channel check, make sure the bitmap is valid, since it doesn't actually catch that until later in the texture code
 *
 * Revision 2.174.2.7  2006/07/13 22:06:38  taylor
 * handle non-MVE movies a bit better in OpenGL (don't get freaky with the window, don't lose input, etc.)
 * some cleanup to OpenGL window handling, to fix min/max/full issues, and try to make shutdown a little nicer
 *
 * Revision 2.174.2.6  2006/07/05 23:36:55  Goober5000
 * cvs comment tweaks
 *
 * Revision 2.174.2.5  2006/06/23 09:01:07  taylor
 * be sure to properly reset fullscreen/minimized state vars as we switch between them
 *
 * Revision 2.174.2.4  2006/06/22 14:59:44  taylor
 * fix various things that Valgrind has been complaining about
 *
 * Revision 2.174.2.3  2006/06/18 20:09:03  taylor
 * fix screenshots on big endian
 *
 * Revision 2.174.2.2  2006/06/12 03:37:24  taylor
 * sync current OGL changes:
 *  - go back to using minimize mode which non-active, but doin't minimize when Fred_running
 *  - remove temporary cmdline options (-spec_scale, -env_scale, -alpha_alpha_blend)
 *  - change FBO renderbuffer link around a little to maybe avoid freaky drivers (or freaky code)
 *
 * Revision 2.174.2.1  2006/06/05 23:59:11  taylor
 * this should hopefully fix cursor drift on multi-display configs
 *
 * Revision 2.174  2006/06/01 07:33:59  taylor
 * flip color/depth values, appears some are initting software GL again and this was the problem previously
 *
 * Revision 2.173  2006/05/31 04:02:05  taylor
 * render strings a string at a time rather than a letter at the time (should be at least 3 times faster from state changes alone)
 *
 * Revision 2.172  2006/05/30 03:53:52  taylor
 * z-range for 2D ortho is -1.0 to 1.0, may help avoid some strangeness if we actually get that right. :)
 * minor cleanup of old code and default settings
 * try not to bother with depth test unless we are actually going to need it (small performance boost in some cases)
 * don't clear depth bit in flip(), while technically correct it's also a bit redundant (and comes with a slight performance hit)
 *
 * Revision 2.171  2006/05/27 17:07:48  taylor
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
 * Revision 2.170  2006/05/13 07:29:52  taylor
 * OpenGL envmap support
 * newer OpenGL extension support
 * add GL_ARB_texture_rectangle support for non-power-of-2 textures as interface graphics
 * add cubemap reading and writing support to DDS loader
 * fix bug in DDS loader that made compressed images with mipmaps use more memory than they really required
 * add support for a default envmap named "cubemap.dds"
 * new mission flag "$Environment Map:" to use a pre-existing envmap
 * minor cleanup of compiler warning messages
 * get rid of wasteful math from gr_set_proj_matrix()
 * remove extra gr_set_*_matrix() calls from starfield.cpp as there was no longer a reason for them to be there
 * clean up bmpman flags in reguards to cubemaps and render targets
 * disable D3D envmap code until it can be upgraded to current level of code
 * remove bumpmap code from OpenGL stuff (sorry but it was getting in the way, if it was more than copy-paste it would be worth keeping)
 * replace gluPerspective() call with glFrustum() call, it's a lot less math this way and saves the extra function call
 *
 * Revision 2.169  2006/04/15 00:13:22  phreak
 * gr_flash_alpha(), much like gr_flash(), but allows an alpha value to be passed
 *
 * Revision 2.168  2006/04/13 12:15:58  taylor
 * deal with font rendering issue from float precision on non-standard resolutions. this is only about a 97% fix/hack
 *   but I don't want people complaining about the bad issue until I can figure out a more appropriate solution with
 *   gr_set_clip(), which is the real source of the problem
 *
 * Revision 2.167  2006/04/12 01:10:35  taylor
 * some cleanup and slight reorg
 *  - remove special uv offsets for non-standard res, they were stupid anyway and don't actually fix the problem (which should actually be fixed now)
 *  - avoid some costly math where possible in the drawing functions
 *  - add opengl_error_string(), this is part of a later update but there wasn't a reason to not go ahead and commit this peice now
 *  - minor cleanup to Win32 extension defines
 *  - make opengl_lights[] allocate only when using OGL
 *  - cleanup some costly per-frame lighting stuff
 *  - clamp textures for interface and aabitmap (font) graphics since they shouldn't normally repeat anyway (the default)
 *    (doing this for D3D, if it doesn't already, may fix the blue-lines problem since a similar issue was seen with OGL)
 *
 * Revision 2.166  2006/03/26 08:26:45  taylor
 * make sure to only allow on screen save at the time (I don't like having it restricted but since it's not used for anything else there is little point)
 *
 * Revision 2.165  2006/03/22 18:12:50  taylor
 * minor cleanup
 *
 * Revision 2.164  2006/02/24 07:35:48  taylor
 * add v-sync support for OGL (I skimmped on this a bit but will go back to do something better, "special" extension wise, at a later date)
 *
 * Revision 2.163  2006/02/20 07:23:29  taylor
 * start of some of the newer GL stuff
 *  - initial support for dropping from fullscreen to windowed mode (will get more changes in another day or so, mainly because of FRED2)
 *  - a little cleanup
 *  - init change for WGL to be a little safer and to help report what it tried to use and what it got in the way of pixelformat
 *  - don't use global OGL_enabled to decide if GL is initialized or not (also getting more changes shortly)
 *  - make sure opengl_close() will always execute properly through atexit() (also getting changed later to not run through atexit())
 *
 * Revision 2.162  2006/01/30 06:40:49  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 * some basic lighting code cleanup
 *
 * Revision 2.161  2006/01/24 13:38:30  taylor
 * break out of error check loop after the first check in FRED since it doesn't like this for some reason
 *
 * Revision 2.160  2006/01/22 01:30:33  taylor
 * clear depth buffer on each page flip (may give a slight speed increase)
 * change error check handling again, FRED may like this better but if it starts hanging again I have another fix
 * little safety checks for opengl_close()
 * fix atexit() call for OGL closeout
 *
 * Revision 2.159  2006/01/21 02:22:04  wmcoolmon
 * Scripting updates; Special scripting image list; Better operator meta; Orientation type; Wing type; Texture type. Fix for MSVC7 compiling.
 *
 * Revision 2.158  2006/01/21 00:14:25  taylor
 * don't forcefully disable multisample still (part of an old ATI fix that didn't actually fix anything)
 *
 * Revision 2.157  2006/01/20 17:15:16  taylor
 * gr_*_bitmap_ex() stuff, D3D side is 100% untested to even compile
 * several other very minor changes as well
 *
 * Revision 2.156  2006/01/19 16:00:04  wmcoolmon
 * Lua debugging stuff; gr_bitmap_ex stuff for taylor
 *
 * Revision 2.155  2006/01/02 07:25:11  taylor
 * don't specify a read buffer, something with this is screwing up ATI cards, a debug string should satisfy a hunch
 * ifdef out the bumpmap stuff, it's very wrong anyway :)
 *
 * Revision 2.154  2005/12/29 20:12:51  taylor
 * we are using gouraud lighting here so be sure to set the proper tmap flag (fixes D3D, corrects OGL)
 *
 * Revision 2.153  2005/12/29 08:08:33  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.152  2005/12/29 04:33:15  taylor
 * put texture filter debug message back (was commented out for some reason in phreak's commit)
 *
 * Revision 2.151  2005/12/29 00:52:57  phreak
 * changed around aabitmap calls to accept a "mirror" parameter.  defaults to false, and is only true for mirrored briefing icons.
 * If the mirror param is true, then the picture is mirrored about the y-axis so left becomes right and vice versa.
 *
 * Revision 2.150  2005/12/28 22:24:48  taylor
 * save screenshots into <gamedir>/screenshots/
 * another attempt at fixing popups with single colors (saves memory too, hope it doesn't blow up)
 * add a debug msg about mipmap filter in use
 *
 * Revision 2.149  2005/12/22 19:15:20  taylor
 * one more attempt to fix screenshots on Radeons.
 *
 * Revision 2.148  2005/12/16 16:34:35  taylor
 * ehh, didn't ever fix that to my satisfaction, bastardize it until more work can be done (so people don't start filing bug reports)
 *
 * Revision 2.147  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 2.146  2005/12/15 16:26:35  taylor
 * didn't mean for that to hit CVS, it makes neb missions work a little strange
 *
 * Revision 2.145  2005/12/08 15:07:57  taylor
 * remove GL_NO_HTL define since it's basically useless at this point and can produced non-functioning builds
 * minor cleanup and readability changes
 * get Apple GL version change in CVS finally, the capabilities of an Apple GL version don't neccessarily correspond to it's features
 *
 * Revision 2.144  2005/12/07 05:39:50  taylor
 * bah, X sucks.  I need to beef up error handling there in order to make all of the platforms happy.
 *
 * Revision 2.143  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 2.142  2005/11/18 13:13:47  taylor
 * get rid of the while() loop, OGL doesn't stack errors (this killed FRED2 for some reason even though no error was present)
 *
 * Revision 2.141  2005/11/14 08:39:00  taylor
 * didn't mean for that to hit CVS, stops FRED2 from crashing on exit
 *
 * Revision 2.140  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.139  2005/10/26 20:54:18  taylor
 * D3D missed the non-standard resolution updates for aabitmaps, should fix briefing icon positioning
 *
 * Revision 2.138  2005/10/23 20:34:29  taylor
 * some cleanup, fix some general memory leaks, safety stuff and whatever else Valgrind complained about
 *
 * Revision 2.137  2005/10/23 19:07:18  taylor
 * make AABITMAP use GL_ALPHA rather than GL_LUMINANCE_ALPHA (now 8-bit instead of 16-bit, fixes several minor rendering issues)
 *
 * Revision 2.136  2005/10/23 14:12:35  taylor
 * minor cleanup to screenshot code
 * force front buffer reads for relevant glReadPixels() calls
 *
 * Revision 2.135  2005/09/20 02:46:52  taylor
 * slight speedup for font rendering
 * fix a couple of things that Valgrind complained about
 *
 * Revision 2.134  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 2.133  2005/08/29 02:20:56  phreak
 * Record state changes in gr_opengl_set_state()
 *
 * Revision 2.132  2005/08/25 22:40:03  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.131  2005/08/20 20:34:51  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.130  2005/07/26 08:46:48  taylor
 * Add ability to use specific refresh rate, needs a soon to be updated Launcher to have set via GUI
 *   but this can go in already.  Defaults to, and falls back on, previous behavior and adds some
 *   error checking that wasn't there previously.
 *
 * Revision 2.129  2005/07/20 02:35:51  taylor
 * better UV offsets when using non-standard resolutions (for text mostly), copied from D3D code
 *
 * Revision 2.128  2005/07/18 03:44:01  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.127  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.126  2005/07/07 16:36:57  taylor
 * various compiler warning fixes (some of these from dizzy)
 *
 * Revision 2.125  2005/07/02 19:40:48  taylor
 * ton of non-standard resolution fixes
 * make gr_cross_fade() work properly in OGL
 *
 * Revision 2.124  2005/06/29 18:51:05  taylor
 * revert OGL init changes since ATI drivers suck and don't like it
 *
 * Revision 2.123  2005/06/19 02:37:02  taylor
 * general cleanup, remove some old code
 * speed up gr_opengl_flip() just a tad
 * inverted gamma slider fix that Sticks made to D3D
 * possible fix for ATI green screens
 * move opengl_check_for_errors() out of gropentnl so we can use it everywhere
 * fix logged OGL info from debug builds to be a little more readable
 * if an extension is found but required function is not then fail
 * try to optimize glDrawRangeElements so we are not rendering more than the card is optimized for
 * some 2d matrix usage checks
 *
 * Revision 2.122  2005/06/03 06:51:50  taylor
 * the problem this tried to fix should be properly fixed now but I keep forgetting to remove this block
 *
 * Revision 2.121  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.120  2005/04/24 12:56:42  taylor
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
 * Revision 2.119  2005/04/24 02:38:31  wmcoolmon
 * Moved gr_rect and gr_shade to be API-nonspecific as the OGL/D3D functions were virtually identical
 *
 * Revision 2.118  2005/04/15 11:41:27  taylor
 * stupid <expletive-delete> terminal, I <expletive-deleted> <expletive-deleted>!!!
 *
 * Revision 2.117  2005/04/15 11:36:54  taylor
 * new GCC = new warning messages, yippeeee!!
 *
 * Revision 2.116  2005/04/12 02:04:56  phreak
 * gr_set_ambient_light() function for the ambient light sliders in FRED
 *
 * Revision 2.115  2005/04/05 11:50:57  taylor
 * fix memory error from GL extension list that occurs in certain circumstances
 *
 * Revision 2.114  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.113  2005/04/01 07:25:54  taylor
 * go back to 24-bit depth, 32 didn't help any with ATI bugs
 *
 * Revision 2.112  2005/03/24 23:42:20  taylor
 * s/gr_ogl_/gr_opengl_/g
 * add empty gr_opengl_draw_line_list() so that it's not a NULL pointer
 * make gr_opengl_draw_htl_sphere() just use GLU so we don't need yet another friggin API
 *
 * Revision 2.111  2005/03/22 00:36:48  taylor
 * fix version check for drivers that support OpenGL 2.0+
 *
 * Revision 2.110  2005/03/20 18:05:04  phreak
 * lol forgot to commit the function pointer stuff
 *
 * Revision 2.109  2005/03/20 00:09:07  phreak
 * Added gr_draw_htl_line and gr_draw_htl sphere
 * There still needs to be D3D versions implemented, but OGL is done.
 * Follow that or ask phreak about how its implemented/
 *
 * Revision 2.108  2005/03/19 21:03:54  wmcoolmon
 * OpenGL display lists
 *
 * Revision 2.107  2005/03/19 18:02:33  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.106  2005/03/08 03:50:20  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.105  2005/03/07 13:10:21  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.104  2005/03/05 19:11:07  taylor
 * make sure we don't scale the restore_screen bitmap, it's already the correct size
 *
 * Revision 2.103  2005/03/03 16:16:55  taylor
 * support for TMAP_FLAG_TRILIST
 * extra check to make sure OGL closes down right under Windows
 * increased depth buffer, it was too small to compare when 32bpp was used
 *
 * Revision 2.102  2005/03/03 00:33:41  taylor
 * use the back buffer for screen saves, this didn't work on Windows at one
 *    point but I'm out of things to try and it may be ok after other fixes
 *
 * Revision 2.101  2005/02/27 10:38:06  wmcoolmon
 * Nonstandard res stuff
 *
 * Revision 2.100  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 2.99  2005/02/19 07:50:15  wmcoolmon
 * OpenGL gradient fix for nonstandard resolutions
 *
 * Revision 2.98  2005/02/18 09:51:06  wmcoolmon
 * Updates for better nonstandard res support, as well as a fix to the Perseus crash bug I've been experiencing. Bobb, you might want to take a look at my change to grd3d.cpp
 *
 * Revision 2.97  2005/02/10 04:01:43  wmcoolmon
 * Low-level code for better hi-res support; better error reporting for vertex errors on model load.
 *
 * Revision 2.96  2005/02/05 00:30:49  taylor
 * fix a few things post merge
 *
 * Revision 2.95  2005/02/04 23:29:31  taylor
 * merge with Linux/OSX tree - p0204-3
 *
 * Revision 2.94  2005/01/29 08:04:15  wmcoolmon
 * Ahh, the sweet smell of optimized code
 *
 * Revision 2.93  2005/01/21 08:25:14  taylor
 * fill in gr_opengl_set_texture_addressing()
 * add support for non-power-of-two textures for cards that have it
 * temporary crash fix from multiple mipmap levels in uncompressed formats
 *
 * Revision 2.92  2005/01/14 05:28:58  wmcoolmon
 * gr_curve
 *
 * Revision 2.91  2005/01/03 18:45:22  taylor
 * dynamic allocation of num supported OpenGL lights
 * add config option for more realistic light settings
 * don't render spec maps in nebula to address lighting issue
 *
 * Revision 2.90  2005/01/01 11:24:22  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 2.89  2004/12/22 23:05:48  phreak
 * added a pragma message if DevIL isn't compiled into the build
 *
 * Revision 2.88  2004/12/02 11:18:15  taylor
 * make OGL use same gamma reg setting as D3D since it's the same ramp
 * have OGL respect the -no_set_gamma cmdline option
 * reset gamma to default on OGL exit to make sure the desktop doesn't stay wrong
 *
 * Revision 2.87  2004/11/04 22:49:13  taylor
 * don't set gamma ramp while running FRED
 *
 * Revision 2.86  2004/10/31 21:42:31  taylor
 * Linux tree merge, use linear mag filter, small FRED fix, AA lines (disabled), use rgba colors for 3dunlit, proper gamma adjustment, bmpman merge
 *
 * Revision 2.85  2004/09/24 22:40:23  taylor
 * proper OGL line drawing in HTL... hopefully
 *
 * Revision 2.84  2004/07/29 09:35:29  taylor
 * fix NULL pointer and try to prevent in future, remove excess commands in opengl_cleanup()
 *
 * Revision 2.83  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.82  2004/07/17 18:46:07  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.81  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.80  2004/07/11 03:22:49  bobboau
 * added the working decal code
 *
 * Revision 2.79  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.78  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.77  2004/05/25 00:37:26  wmcoolmon
 * Updated function calls for VC7 use
 *
 * Revision 2.76  2004/05/11 16:48:19  phreak
 * fixed the white boxes appearing on unlit bitmaps
 *
 * Revision 2.75  2004/05/02 16:01:38  taylor
 * don't use HTL line fix with Fred
 *
 * Revision 2.74  2004/04/26 12:43:58  taylor
 * minor fixes, HTL lines, 32-bit texture support
 *
 * Revision 2.73  2004/04/14 10:24:51  taylor
 * fix for lines and shaders - shouldn't be textured
 *
 * Revision 2.72  2004/04/11 13:56:33  randomtiger
 * Adding batching functions here and there and into gr_screen for use with OGL when its ready.
 *
 * Revision 2.71  2004/04/03 20:27:57  phreak
 * OpenGL files spilt up to make developing and finding bugs much easier
 *
 * Revision 2.70  2004/03/29 02:24:20  phreak
 * fixed a minor bug where the version checker
 * would complain even if the correct version of OpenGL was present
 *
 * Revision 2.69  2004/03/20 14:47:13  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.68  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.67  2004/03/08 21:57:04  phreak
 * made a minor logical fix to gr_opengl_tmapper_internal
 *
 * disabled pcx32 and jpgtga flags if they were defined by the user
 *
 * Revision 2.66  2004/03/05 04:33:09  phreak
 * fixed the lack of ships in the hud targetbox
 * prevented other functions calls to glColor3ub() from messing
 * with the color of textures in gr_opengl_render_buffer().  this was causing some
 * minor rendering bugs
 *
 * Revision 2.65  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.64  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.63  2004/02/15 03:04:25  bobboau
 * fixed bug involving 3d shockwaves, note I wasn't able to compile the directshow file, so I ifdefed everything to an older version,
 * you shouldn't see anything diferent, as the ifdef should be set to the way it should be, if it isn't you will get a warning mesage during compile telling you how to fix it
 *
 * Revision 2.62  2004/02/14 00:18:32  randomtiger
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
 * Revision 2.61  2004/02/13 04:17:12  randomtiger
 * Turned off fog in OGL for Fred.
 * Simulated speech doesnt say tags marked by $ now.
 * The following are fixes to issues that came up testing TBP in fs2_open and fred2_open:
 * Changed vm_vec_mag and parse_tmap to fail gracefully on bad data.
 * Error now given on missing briefing icon and bad ship normal data.
 * Solved more species divide by zero error.
 * Fixed neb cube crash.
 *
 * Revision 2.60  2004/02/05 01:41:33  randomtiger
 * Small changes, deleted old dshow stuff
 *
 * Revision 2.59  2004/02/03 18:29:30  randomtiger
 * Fixed OGL fogging in HTL
 * Changed htl laser function to work in D3D, commented out until function flat bug is fixed
 *
 * Revision 2.58  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.57  2004/01/20 22:59:09  Goober5000
 * got rid of some warnings... it actually looks like Gr_gamma_lookup can be changed
 * from int to ubyte, because everything that accesses it converts to a ubyte  O.o
 * --Goober5000
 *
 * Revision 2.56  2004/01/19 00:56:09  randomtiger
 * Some more small changes for Fred OGL
 *
 * Revision 2.55  2004/01/18 14:55:08  randomtiger
 * Few more small changes for Fred OGL
 *
 * Revision 2.54  2004/01/18 13:17:55  randomtiger
 * Changed the #ifndef FRED I added to #ifndef FRED_OGL so they only effect OGL FRED compile
 *
 * Revision 2.53  2004/01/17 21:59:53  randomtiger
 * Some small changes to the main codebase that allow Fred_open OGL to compile.
 *
 * Revision 2.52  2003/12/17 23:25:10  phreak
 * added a MAX_BUFFERS_PER_SUBMODEL define so it can be easily changed if we ever want to change the 16 texture limit
 *
 * Revision 2.51  2003/11/29 16:11:46  fryday
 * Fixed normal loading in OpenGL HT&L.
 * Fixed lighting in OpenGL HT&L, hopefully for the last time.
 * Added a test if a normal is valid during model load, if not, replaced with face normal
 *
 * Revision 2.50  2003/11/25 15:04:45  fryday
 * Got lasers to work in HT&L OpenGL
 * Messed a bit with opengl_tmapper_internal3d, draw_laser functions, and added draw_laser_htl
 *
 * Revision 2.49  2003/11/22 10:36:32  fryday
 * Changed default alpha value for lasers in OpenGL to be like in D3D
 * Fixed Glowmaps being rendered with GL_NEAREST instead of GL_LINEAR.
 * Dynamic Lighting almost there. It looks like some normals are fudged or something.
 *
 * Revision 2.48  2003/11/17 04:25:56  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.47  2003/11/12 00:44:52  Kazan
 * (Kazan) /me slaps forehead... make sure things compile before committing.. sorry guys
 *
 * Revision 2.46  2003/11/11 18:05:02  phreak
 * support for GL_ARB_vertex_buffer_object.  should run real fast now with _VERY_
 * high poly ships
 *
 * Revision 2.45  2003/11/07 20:55:15  phreak
 * reenabled timerbar
 *
 * Revision 2.44  2003/11/06 22:36:04  phreak
 * bob's stack functions implemented in opengl
 *
 * Revision 2.43  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.42  2003/10/29 02:09:18  randomtiger
 * Updated timerbar code to work properly, also added support for it in OGL.
 * In D3D red is general processing (and generic graphics), green is 2D rendering, dark blue is 3D unlit, light blue is HT&L renders and yellow is the presentation of the frame to D3D. OGL is all red for now. Use compile flag TIMERBAR_ON with code lib to activate it.
 * Also updated some more D3D device stuff that might get a bit more speed out of some cards.
 *
 * Revision 2.41  2003/10/25 03:26:39  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.40  2003/10/24 17:35:05  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.39  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.38  2003/10/20 22:32:37  phreak
 * cleaned up a bunch of repeated code
 *
 * Revision 2.37  2003/10/19 01:10:05  phreak
 * clipping planes now work
 *
 * Revision 2.36  2003/10/18 01:22:39  phreak
 * hardware transformation is working.  now lighting needs to be done
 *
 * Revision 2.35  2003/10/14 17:39:13  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.34  2003/10/13 19:39:19  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.33  2003/10/13 05:57:48  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.32  2003/10/10 03:59:41  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.31  2003/10/05 23:40:54  phreak
 * bug squashing.
 * preliminary tnl work done
 *
 * Revision 2.30  2003/10/04 00:26:43  phreak
 * shinemapping completed. it works!
 * -phreak
 *
 * Revision 2.29  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.28  2003/08/21 15:07:45  phreak
 * added specular highlights. still needs shine mapping
 *
 * Revision 2.27  2003/08/06 17:26:20  phreak
 * changed default texture filtering to GL_LINEAR
 *
 * Revision 2.26  2003/08/03 23:35:36  phreak
 * changed some z-buffer stuff so it doesn't clip as noticably
 *
 * Revision 2.25  2003/07/15 02:34:59  phreak
 * fun work optimizing the cloak effect
 *
 * Revision 2.24  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.23  2003/06/08 17:29:51  phreak
 * fixed a compile error that was accidently committed
 *
 * Revision 2.22  2003/06/07 20:51:06  phreak
 * fixed minor fogging bug
 *
 * Revision 2.21  2003/05/21 20:23:00  phreak
 * improved glowmap rendering speed
 *
 * Revision 2.20  2003/05/07 00:52:36  phreak
 * fixed old bug that created "mouse trails" during a popup screen
 * a result of doing something a long time ago that i thought was right, but wasn't
 *
 * Revision 2.19  2003/05/04 23:52:40  phreak
 * added additional error info incase gr_opengl_flip() fails for some unknown reason
 *
 * Revision 2.18  2003/05/04 20:49:18  phreak
 * minor fix in gr_opengl_flip()
 *
 * should be end of the "unable to swap buffer" error
 *
 * Revision 2.17  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.16  2003/03/07 00:15:45  phreak
 * added some hacks that shutdown and restore opengl because of cutscenes
 *
 * Revision 2.15  2003/02/16 18:43:13  phreak
 * tweaked some more stuff
 *
 * Revision 2.14  2003/02/01 02:57:42  phreak
 * started to finalize before 3.50 release.
 * added support for GL_EXT_texture_filter_anisotropic
 *
 * Revision 2.13  2003/01/26 23:30:53  DTP
 * temporary fix, added some // so that we can do debug builds. i expect Goober5000 will fix it soon.
 *
 * Revision 2.12  2003/01/19 22:45:34  Goober5000
 * cleaned up build output a bit
 * --Goober5000
 *
 * Revision 2.11  2003/01/19 01:07:41  bobboau
 * redid the way glow maps are handled; you now must set a global variable before you render a poly that uses a glow map, then set it to -1 when you're done with it
 * fixed a few other misc bugs too
 *
 * Revision 2.10  2003/01/18 19:49:45  phreak
 * texture mapper now supports DXTC compressed textures
 *
 * Revision 2.9  2003/01/09 21:19:54  phreak
 * glowmaps in OpenGL, yay!
 *
 * Revision 2.8  2002/12/23 19:25:39  phreak
 * dumb typo
 *
 * Revision 2.7  2002/12/23 17:52:51  phreak
 * added code that displays an error if user's OGL version is less than 1.2
 *
 * Revision 2.6  2002/12/16 23:28:52  phreak
 * optimized fullscreen/minimized functions.. alot faster
 *
 * Revision 2.5  2002/12/15 18:59:57  DTP
 * fixed a minor glitch, replaced <> with ", so that it takes project glXXX.h, and not compilers glXXX.h
 *
 * Revision 2.4  2002/12/14 16:14:52  phreak
 * copied from grgpenglw32x.cpp.
 *
 * Revision 1.7  2002/12/05 00:49:25  phreak
 * extension framework implemented(no extensions work YET) -phreak
 *
 * Revision 1.6  2002/11/22 20:54:16  phreak
 * finished off all remaining problems with 32 bit mode, fullscreen mode
 * and colors.  Still needs some tweaking, but works near perfect
 *
 * Revision 1.5  2002/11/18 21:31:36  phreak
 * complete overhaul, works in 16 bit mode -phreak
 *
 * Revision 1.4  2002/10/14 19:49:08  phreak
 * screenshots, yay!
 *
 * Revision 1.3  2002/10/13 21:43:24  phreak
 * further optimizations
 *
 * Revision 1.2  2002/10/12 17:48:11  phreak
 * fixed text
 *
 * Revision 1.1  2002/10/11 21:31:17  phreak
 * first run at opengl for w32, only useful in main hall, barracks and campaign room
 *
 * Revision 1.1.1.1  2002/06/06 06:25:26  drevil
 * initial import of June 6th, 2002 sources
 *
 * Revision 1.46  2002/06/05 04:03:32  relnev
 * finished cfilesystem.
 *
 * removed some old code.
 *
 * fixed mouse save off-by-one.
 *
 * sound cleanups.
 *
 * Revision 1.45  2002/06/03 09:25:37  relnev
 * implement mouse cursor and screen save/restore
 *
 * Revision 1.44  2002/06/02 18:46:59  relnev
 * updated
 *
 * Revision 1.43  2002/06/02 11:34:00  relnev
 * adjust z coords
 *
 * Revision 1.42  2002/06/02 10:28:17  relnev
 * fix texture handle leak
 * Revision 2.3.2.2  2002/11/04 16:04:21  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.3.2.1  2002/11/04 03:02:29  randomtiger
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
 * Revision 2.3  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 1.41  2002/06/01 09:00:34  relnev
 * silly debug memmanager
 *
 * Revision 1.40  2002/06/01 07:12:33  relnev
 * a few NDEBUG updates.
 *
 * removed a few warnings.
 *
 * Revision 1.39  2002/06/01 05:33:15  relnev
 * copied more code over.
 *
 * added scissor clipping.
 *
 * Revision 1.38  2002/06/01 03:35:27  relnev
 * fix typo
 *
 * Revision 1.37  2002/06/01 03:32:00  relnev
 * fix texture loading mistake.
 *
 * enable some d3d stuff for opengl also
 *
 * Revision 1.36  2002/05/31 23:25:03  relnev
 * line fixes
 *
 * Revision 1.34  2002/05/31 22:15:22  relnev
 * BGRA
 *
 * Revision 1.33  2002/05/31 22:04:55  relnev
 * use d3d rect_internal
 *
 * Revision 1.32  2002/05/31 06:28:23  relnev
 * more stuff
 *
 * Revision 1.31  2002/05/31 06:04:39  relnev
 * fog
 *
 * Revision 1.30  2002/05/31 03:56:11  theoddone33
 * Change tmapper polygon winding and enable culling
 *
 * Revision 1.29  2002/05/31 03:34:02  theoddone33
 * Fix Keyboard
 * Add titlebar
 *
 * Revision 1.28  2002/05/31 00:06:59  relnev
 * minor change
 *
 * Revision 1.27  2002/05/30 23:46:29  theoddone33
 * some minor key changes (not necessarily fixes)
 *
 * Revision 1.26  2002/05/30 23:33:12  relnev
 * implemented a few more functions.
 *
 * Revision 1.25  2002/05/30 23:01:16  relnev
 * implement gr_opengl_set_state.
 *
 * Revision 1.24  2002/05/30 22:12:57  relnev
 * finish default texture case
 *
 * Revision 1.23  2002/05/30 22:02:30  theoddone33
 * More gl changes
 *
 * Revision 1.22  2002/05/30 21:44:48  relnev
 * implemented some missing texture stuff.
 *
 * enable bitmap polys for opengl.
 *
 * work around greenness in bitmaps.
 *
 * Revision 1.21  2002/05/30 17:29:30  theoddone33
 * Fix some more stubs, change at least one polygon winding since culling is now
 * enabled.
 *
 * Revision 1.20  2002/05/30 16:50:24  theoddone33
 * Keyboard partially fixed
 *
 * Revision 1.19  2002/05/30 08:13:14  relnev
 * fonts are fixed
 *
 * Revision 1.18  2002/05/29 23:37:36  relnev
 * fix bitmap bug
 *
 * Revision 1.17  2002/05/29 23:17:49  theoddone33
 * Non working text code and fixed keys
 *
 * Revision 1.16  2002/05/29 19:45:13  theoddone33
 * More changes on texture loading
 *
 * Revision 1.15  2002/05/29 19:06:48  theoddone33
 * Enable string printing.  Enable texture mapping
 *
 * Revision 1.14  2002/05/29 08:54:40  relnev
 * "fixed" bitmap drawing.
 *
 * copied more d3d code over.
 *
 * Revision 1.13  2002/05/29 06:25:13  theoddone33
 * Keyboard input, mouse tracking now work
 *
 * Revision 1.12  2002/05/29 04:52:45  relnev
 * bitmap
 *
 * Revision 1.11  2002/05/29 04:29:56  relnev
 * removed some unncessary stubbing, implemented opengl rect
 *
 * Revision 1.10  2002/05/29 04:13:27  theoddone33
 * enable opengl_line
 *
 * Revision 1.9  2002/05/29 03:35:51  relnev
 * added rest of init
 *
 * Revision 1.8  2002/05/29 03:30:05  relnev
 * update opengl stubs
 *
 * Revision 1.7  2002/05/29 02:52:32  theoddone33
 * Enable OpenGL renderer
 *
 * Revision 1.6  2002/05/28 04:56:51  theoddone33
 * runs a little bit now
 *
 * Revision 1.5  2002/05/28 04:07:28  theoddone33
 * New graphics stubbing arrangement
 *
 * Revision 1.4  2002/05/27 23:39:34  relnev
 * 0
 *
 * Revision 1.3  2002/05/27 22:35:01  theoddone33
 * more symbols
 *
 * Revision 1.2  2002/05/27 22:32:02  theoddone33
 * throw all d3d stuff at opengl
 *
 * Revision 1.1.1.1  2002/05/03 03:28:09  root
 * Initial import.
 *
 * 
 * 10    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 9     7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 8     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
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
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 13    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 12    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 11    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 10    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 9     12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 8     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 7     9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 6     9/09/97 11:01a Sandeep
 * fixed warning level 4 bugs
 * 
 * 5     7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 4     6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 3     6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 2     6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */


#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/line.h"
#include "nebula/neb.h"
#include "io/mouse.h"
#include "osapi/osregistry.h"
#include "cfile/cfile.h"
#include "io/timer.h"
#include "ddsutils/ddsutils.h"
#include "model/model.h"
#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"


#if defined(_WIN32)
#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#elif defined(__APPLE__)
#include "OpenGL.h"
#else
//#include <GL/glx.h>
typedef int ( * PFNGLXSWAPINTERVALSGIPROC) (int interval);
#endif


#if defined(_WIN32) && !defined(__GNUC__)
#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")
#endif


#define REQUIRED_GL_MAJOR_VERSION	1
#ifdef __APPLE__ // the Apple GL version is more advanced than the version number lets on
#define REQUIRED_GL_MINOR_VERSION	1
#else
#define REQUIRED_GL_MINOR_VERSION	2
#endif


bool GL_initted = 0;

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
int OGL_fogmode = 0;

#ifdef _WIN32
static HDC GL_device_context = NULL;
static HGLRC GL_render_context = NULL;
static PIXELFORMATDESCRIPTOR GL_pfd;
#endif

static ushort *GL_original_gamma_ramp = NULL;

int Use_VBOs = 0;
int Use_PBOs = 0;
int Use_GLSL = 0;

static int GL_dump_frames = 0;
static ubyte *GL_dump_buffer = NULL;
static int GL_dump_frame_number = 0;
static int GL_dump_frame_count = 0;
static int GL_dump_frame_count_max = 0;
static int GL_dump_frame_size = 0;

static ubyte *GL_saved_screen = NULL;
static ubyte *GL_saved_mouse_data = NULL;
static int GL_saved_screen_id = -1;
static GLuint GL_cursor_pbo = 0;
static GLuint GL_screen_pbo = 0;

static int GL_mouse_saved = 0;
static int GL_mouse_saved_x1 = 0;
static int GL_mouse_saved_y1 = 0;
static int GL_mouse_saved_x2 = 0;
static int GL_mouse_saved_y2 = 0;

void opengl_save_mouse_area(int x, int y, int w, int h);

extern char *Osreg_title;

extern GLfloat GL_anisotropy;

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

static int GL_fullscreen = 0;
static int GL_windowed = 0;
static int GL_minimized = 0;

static GLenum GL_read_format = GL_BGRA;


void opengl_go_fullscreen()
{
	if (Cmdline_window || GL_fullscreen || Fred_running)
		return;

#ifdef _WIN32
	DEVMODE dm;
	RECT cursor_clip;
	HWND wnd = (HWND)os_get_window();

	Assert( wnd );

	os_suspend();

	memset((void*)&dm, 0, sizeof(DEVMODE));

	dm.dmSize = sizeof(DEVMODE);
	dm.dmPelsHeight = gr_screen.max_h;
	dm.dmPelsWidth = gr_screen.max_w;
	dm.dmBitsPerPel = gr_screen.bits_per_pixel;
	dm.dmDisplayFrequency = os_config_read_uint( NULL, NOX("OGL_RefreshRate"), 0 );
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (dm.dmDisplayFrequency)
		dm.dmFields |= DM_DISPLAYFREQUENCY;

	if ( (ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL ) {
		if (dm.dmDisplayFrequency) {
			// failed to switch with freq change so try without it just in case
			dm.dmDisplayFrequency = 0;
			dm.dmFields &= ~DM_DISPLAYFREQUENCY;

			if ( (ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL ) {
				Warning( LOCATION, "Unable to go fullscreen on second attempt!" );
			}
		} else {
			Warning( LOCATION, "Unable to go fullscreen!" );
		}
	}

	ShowWindow( wnd, SW_SHOWNORMAL );
	UpdateWindow( wnd );

	SetForegroundWindow( wnd );
	SetActiveWindow( wnd );
	SetFocus( wnd );

	GetWindowRect((HWND)os_get_window(), &cursor_clip);
	ClipCursor(&cursor_clip);
	ShowCursor(FALSE);

	os_resume();  
#else
	if ( (os_config_read_uint(NULL, NOX("Fullscreen"), 1) == 1) && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) ) {
		os_suspend();
	//	SDL_WM_ToggleFullScreen( SDL_GetVideoSurface() );
		if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL | SDL_FULLSCREEN)) == NULL ) {
			mprintf(("Couldn't go fullscreen!\n"));
			if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL)) == NULL ) {
				mprintf(("Couldn't drop back to windowed mode either!\n"));
				exit(1);
			}
		}
		os_resume();
	}
#endif

	gr_opengl_set_gamma(FreeSpace_gamma);

	GL_fullscreen = 1;
	GL_minimized = 0;
	GL_windowed = 0;
}

void opengl_go_windowed()
{
	if ( !Cmdline_window /*|| GL_windowed*/ || Fred_running )
		return;

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();
	Assert( wnd );

	// if we are already in a windowed state, then just make sure that we are sane and bail
	if (GL_windowed) {
		SetForegroundWindow( wnd );
		SetActiveWindow( wnd );
		SetFocus( wnd );

		ClipCursor(NULL);
		ShowCursor(FALSE);
		return;
	}

	os_suspend();

	ShowWindow( wnd, SW_SHOWNORMAL );
	UpdateWindow( wnd );

	SetForegroundWindow( wnd );
	SetActiveWindow( wnd );
	SetFocus( wnd );

	ClipCursor(NULL);
	ShowCursor(FALSE);

	os_resume();  

#else
	if (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) {
		os_suspend();

	//	SDL_WM_ToggleFullScreen( SDL_GetVideoSurface() );
		if ( (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, 0, SDL_OPENGL)) == NULL ) {
			Warning( LOCATION, "Unable to enter windowed mode!" );
		}

		os_resume();
	}
#endif

	GL_windowed = 1;
	GL_minimized = 0;
	GL_fullscreen = 0;
}

void opengl_minimize()
{
	// don't attempt to minimize if we are already in a window, or already minimized, or when playing a movie
	if (GL_minimized /*|| GL_windowed || Cmdline_window*/ || Fred_running)
		return;

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();
	Assert( wnd );

	// if we are a window then just show the cursor and bail
	if (Cmdline_window || GL_windowed) {
		ClipCursor(NULL);
		ShowCursor(TRUE);
		return;
	}

	os_suspend();

	// restore original gamma settings
	if (GL_original_gamma_ramp != NULL) {
		SetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

	ShowWindow(wnd, SW_MINIMIZE);
	ChangeDisplaySettings(NULL, 0);

	ClipCursor(NULL);
	ShowCursor(TRUE);

	os_resume();
#else
	// lets not minimize if we are in windowed mode
	if ( !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) )
		return;

	os_suspend();

	if (GL_original_gamma_ramp != NULL) {
		SDL_SetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}

	SDL_WM_IconifyWindow();
	os_resume();
#endif

	GL_minimized = 1;
	GL_windowed = 0;
	GL_fullscreen = 0;
}

void gr_opengl_activate(int active)
{
	if (active) {
		if (Cmdline_window)
			opengl_go_windowed();
		else
			opengl_go_fullscreen();

#ifdef SCP_UNIX
		// Check again and if we didn't go fullscreen turn on grabbing if possible
		if(!Cmdline_no_grab && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN)) {
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
#endif
	} else {
		opengl_minimize();

#ifdef SCP_UNIX
		// let go of mouse/keyboard
		if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	}
}

void gr_opengl_clear()
{
	glClearColor(gr_screen.current_clear_color.red / 255.0f, 
		gr_screen.current_clear_color.green / 255.0f, 
		gr_screen.current_clear_color.blue / 255.0f, 1.0f);

	glClear ( GL_COLOR_BUFFER_BIT );
}

void gr_opengl_flip()
{
	if ( !GL_initted )
		return;

	gr_reset_clip();

	mouse_eval_deltas();
	
	GL_mouse_saved = 0;
	
	if ( mouse_is_visible() ) {
		int mx, my;

		gr_reset_clip();
		mouse_get_pos( &mx, &my );

	//	opengl_save_mouse_area(mx, my, Gr_cursor_size, Gr_cursor_size);

		if (Gr_cursor != -1) {
			gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my, false);
		}
	}

	TIMERBAR_END_FRAME();
	TIMERBAR_START_FRAME();

#ifdef _WIN32
	SwapBuffers(GL_device_context);
#else
	SDL_GL_SwapBuffers();
#endif

	opengl_tcache_frame();

#ifndef NDEBUG
	int ic = opengl_check_for_errors();

	if (ic) {
		mprintf(("!!DEBUG!! OpenGL Errors this frame: %i\n", ic));
	}
#endif
}

void gr_opengl_set_clip(int x, int y, int w, int h, bool resize)
{
	// check for sanity of parameters
	if (x < 0) {
		x = 0;
	}

	if (y < 0) {
		y = 0;
	}

	int to_resize = (resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)));

	int max_w = ((to_resize) ? gr_screen.max_w_unscaled : gr_screen.max_w);
	int max_h = ((to_resize) ? gr_screen.max_h_unscaled : gr_screen.max_h);

	if (x >= max_w) {
		x = max_w - 1;
	}

	if (y >= max_h) {
		y = max_h - 1;
	}

	if (x + w > max_w) {
		w = max_w - x;
	}

	if (y + h > max_h) {
		h = max_h - y;
	}
	
	if (w > max_w) {
		w = max_w;
	}

	if (h > max_h) {
		h = max_h;
	}

	gr_screen.offset_x_unscaled = x;
	gr_screen.offset_y_unscaled = y;
	gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_right_unscaled = w-1;
	gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_bottom_unscaled = h-1;
	gr_screen.clip_width_unscaled = w;
	gr_screen.clip_height_unscaled = h;

	if (to_resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	} else {
		gr_unsize_screen_pos( &gr_screen.offset_x_unscaled, &gr_screen.offset_y_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.offset_x = x;
	gr_screen.offset_y = y;
	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;
	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;
	gr_screen.clip_width = w;
	gr_screen.clip_height = h;

	gr_screen.clip_aspect = i2fl(w) / i2fl(h);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	// just return early if we aren't actually going to need the scissor test
	if ( (x == 0) && (y == 0) && (w == max_w) && (h == max_h) ) {
		GL_state.ScissorTest(GL_FALSE);
		return;
	}

	GL_state.ScissorTest(GL_TRUE);
	glScissor(x, gr_screen.max_h-y-h, w, h);
}

void gr_opengl_reset_clip()
{
	gr_screen.offset_x = gr_screen.offset_x_unscaled = 0;
	gr_screen.offset_y = gr_screen.offset_y_unscaled = 0;
	gr_screen.clip_left = gr_screen.clip_left_unscaled = 0;
	gr_screen.clip_top = gr_screen.clip_top_unscaled = 0;
	gr_screen.clip_right = gr_screen.clip_right_unscaled = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.clip_bottom_unscaled = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.clip_width_unscaled = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.clip_height_unscaled = gr_screen.max_h;

	if (gr_screen.custom_size) {
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.clip_aspect = i2fl(gr_screen.clip_width) / i2fl(gr_screen.clip_height);
	gr_screen.clip_center_x = (gr_screen.clip_left + gr_screen.clip_right) * 0.5f;
	gr_screen.clip_center_y = (gr_screen.clip_top + gr_screen.clip_bottom) * 0.5f;

	GL_state.ScissorTest(GL_FALSE);
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_opengl_print_screen(char *filename)
{
	char tmp[MAX_PATH_LEN];
	ubyte tga_hdr[18];
	int i;
	ushort width, height;
	GLubyte *pixels = NULL;
	GLuint pbo = 0;

	// save to a "screenshots" directory and tack on the filename
#ifdef SCP_UNIX
	snprintf( tmp, MAX_PATH_LEN-1, "%s/%s/screenshots/%s.tga", detect_home(), Osreg_user_dir, filename);
	_mkdir( tmp );
#else
	_getcwd( tmp, MAX_PATH_LEN-1 );
	strcat( tmp, "\\screenshots\\" );
	_mkdir( tmp );

	strcat( tmp, filename );
	strcat( tmp, ".tga" );
#endif

	FILE *fout = fopen(tmp, "wb");

	if (fout == NULL) {
		return;
	}

//	glReadBuffer(GL_FRONT);

	// now for the data
	if (Use_PBOs) {
		Assert( !pbo );
		vglGenBuffersARB(1, &pbo);

		if ( !pbo ) {
			if (fout != NULL)
				fclose(fout);

			return;
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
		vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, (gr_screen.max_w * gr_screen.max_h * 4), NULL, GL_STATIC_READ);

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		// map the image data so that we can save it to file
		pixels = (GLubyte*) vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
	} else {
		pixels = (GLubyte*) vm_malloc_q(gr_screen.max_w * gr_screen.max_h * 4);

		if (pixels == NULL) {
			if (fout != NULL) {
				fclose(fout);
			}

			return;
		}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
		glFlush();
	}

	// Write the TGA header
	width = INTEL_SHORT((ushort)gr_screen.max_w);
	height = INTEL_SHORT((ushort)gr_screen.max_h);

	memset( tga_hdr, 0, sizeof(tga_hdr) );

	tga_hdr[2] = 2;		// ImageType    2 = 24bpp, uncompressed
	memcpy( tga_hdr + 12, &width, sizeof(ushort) );		// Width
	memcpy( tga_hdr + 14, &height, sizeof(ushort) );	// Height
	tga_hdr[16] = 24;	// PixelDepth

	fwrite( tga_hdr, sizeof(tga_hdr), 1, fout );

	// now for the data, we convert it from 32-bit to 24-bit
	for (i = 0; i < (gr_screen.max_w * gr_screen.max_h * 4); i += 4) {
#if BYTE_ORDER == BIG_ENDIAN
		int pix, *pix_tmp;

		pix_tmp = (int*)(pixels + i);
		pix = INTEL_INT(*pix_tmp);

		fwrite( &pix, 1, 3, fout );
#else
		fwrite( pixels + i, 1, 3, fout );
#endif
	}

	if (pbo) {
		vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		pixels = NULL;
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		vglDeleteBuffersARB(1, &pbo);
	}

	// done!
	fclose(fout);

	if (pixels != NULL) {
		vm_free(pixels);
	}
}

void gr_opengl_cleanup(int minimize)
{	
	if ( !GL_initted ) {
		return;
	}

	if ( !Fred_running ) {
		gr_reset_clip();
		gr_clear();
		gr_flip();
		gr_clear();
		gr_flip();
		gr_clear();
	}

	GL_initted = false;

	opengl_tcache_flush();

#ifdef _WIN32
	HWND wnd = (HWND)os_get_window();

	DBUGFILE_OUTPUT_0("");

	if (GL_render_context) {
		if ( !wglMakeCurrent(NULL, NULL) ) {
			DBUGFILE_OUTPUT_0("");
			MessageBox(wnd, "SHUTDOWN ERROR", "error", MB_OK);
		}

		if ( !wglDeleteContext(GL_render_context) ) {
			DBUGFILE_OUTPUT_0("");
			MessageBox(wnd, "Unable to delete rendering context", "error", MB_OK);
		}

		GL_render_context = NULL;
	}
#endif

	DBUGFILE_OUTPUT_0("opengl_minimize");
	opengl_minimize();

	if (minimize) {
#ifdef _WIN32
		if ( !Cmdline_window ) {
			ChangeDisplaySettings(NULL, 0);
		}
#endif
	}
}

void gr_opengl_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
//	mprintf(("gr_opengl_fog_set(%d,%d,%d,%d,%f,%f)\n",fog_mode,r,g,b,fog_near,fog_far));

	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));
	
	if (fog_mode == GR_FOGMODE_NONE) {
		if ( GL_state.Fog() ) {
			GL_state.Fog(GL_FALSE);
		}

		gr_screen.current_fog_mode = fog_mode;
		
		return;
	}
	
  	if (gr_screen.current_fog_mode != fog_mode) {
	  	if (OGL_fogmode == 3) {
			glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
			glFogf(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
		}
		// Um.. this is not the correct way to fog in software, probably doesnt matter though
		else if ( (OGL_fogmode == 2) && Cmdline_nohtl ) {
			glFogf(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
			fog_near *= fog_near;		// it's faster this way
			fog_far *= fog_far;		
		} else {
			glFogf(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
		}

		GL_state.Fog(GL_TRUE); 
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, fog_near);
		glFogf(GL_FOG_END, fog_far);

		gr_screen.current_fog_mode = fog_mode;
	}
	
	if ( (gr_screen.current_fog_color.red != r) ||
			(gr_screen.current_fog_color.green != g) ||
			(gr_screen.current_fog_color.blue != b) )
	{
		GLfloat fc[4];
		
		gr_init_color( &gr_screen.current_fog_color, r, g, b );
	
		fc[0] = (float)r/255.0f;
		fc[1] = (float)g/255.0f;
		fc[2] = (float)b/255.0f;
		fc[3] = 1.0f;
		
		glFogfv(GL_FOG_COLOR, fc);
	}

}

int gr_opengl_set_cull(int cull)
{
	GLboolean enabled = GL_FALSE;

	if (cull) {
		enabled = GL_state.CullFace(GL_TRUE);
		GL_state.FrontFaceValue(GL_CCW);
		GL_state.CullFaceValue(GL_BACK);
	} else {
		enabled = GL_state.CullFace(GL_FALSE);
	}

	return (enabled) ? 1 : 0;
}

void gr_opengl_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

int gr_opengl_zbuffer_get()
{
	if ( !gr_global_zbuffering ) {
		return GR_ZBUFF_NONE;
	}

	return gr_zbuffering_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if (gr_zbuffering_mode == GR_ZBUFF_NONE) {
		gr_zbuffering = 0;
		GL_state.DepthTest(GL_FALSE);
	} else {
		gr_zbuffering = 1;
		GL_state.DepthTest(GL_TRUE);
	}

	return tmp;
}

void gr_opengl_zbuffer_clear(int mode)
{
	if (mode) {
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		GL_state.SetZbufferType(ZBUFFER_TYPE_FULL);

		glClear(GL_DEPTH_BUFFER_BIT);
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;

		GL_state.DepthTest(GL_FALSE);
	}
}

// I feel dirty...
static void opengl_make_gamma_ramp(float gamma, ushort *ramp)
{
	int x, y;
	ushort base_ramp[256];

	Assert( ramp != NULL );

	// generate the base ramp values first off

	// if no gamma set then just do this quickly
	if (gamma <= 0.0f) {
		memset( ramp, 0, 3 * 256 * sizeof(ushort) );
		return;
	}
	// identity gamma, avoid all of the math
	else if ( (gamma == 1.0f) || (GL_original_gamma_ramp == NULL) ) {
		if (GL_original_gamma_ramp != NULL) {
			memcpy( ramp, GL_original_gamma_ramp, 3 * 256 * sizeof(ushort) );
		}
		// set identity if no original ramp
		else {
			for (x = 0; x < 256; x++) {
				ramp[x]	= (x << 8) | x;
				ramp[x + 256] = (x << 8) | x;
				ramp[x + 512] = (x << 8) | x;
			}
		}

		return;
	}
	// for everything else we need to actually figure it up
	else {
		double g = 1.0 / (double)gamma;
		int val;

		Assert( GL_original_gamma_ramp != NULL );

		for (x = 0; x < 256; x++) {
			val = (int) (pow(x/255.0, g) * 65535.0 + 0.5);
			CLAMP( val, 0, 65535 );

			base_ramp[x] = (ushort)val;
		}

		for (y = 0; y < 3; y++) {
			for (x = 0; x < 256; x++) {
				val = (base_ramp[x] * 2) - GL_original_gamma_ramp[x + y * 256];
				CLAMP( val, 0, 65535 );

				ramp[x + y * 256] = (ushort)val;
			}
		}
	}
}

void gr_opengl_set_gamma(float gamma)
{
	ushort *gamma_ramp = NULL;

	Gr_gamma = gamma;
	Gr_gamma_int = int (Gr_gamma*10);

	// new way - but not while running FRED
	if (!Fred_running && !Cmdline_no_set_gamma) {
		gamma_ramp = (ushort*) vm_malloc_q( 3 * 256 * sizeof(ushort) );

		if (gamma_ramp == NULL) {
			Int3();
			return;
		}

		memset( gamma_ramp, 0, 3 * 256 * sizeof(ushort) );

		// Create the Gamma lookup table
		opengl_make_gamma_ramp(gamma, gamma_ramp);

#ifdef _WIN32
		SetDeviceGammaRamp( GL_device_context, gamma_ramp );
#else
		SDL_SetGammaRamp( gamma_ramp, (gamma_ramp+256), (gamma_ramp+512) );
#endif

		vm_free(gamma_ramp);
	}
}

void gr_opengl_get_region(int front, int w, int h, ubyte *data)
{

//	if (front) {
//		glReadBuffer(GL_FRONT);
//	} else {
		glReadBuffer(GL_BACK);
//	}

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);
	
	if (gr_screen.bits_per_pixel == 16) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
	} else if (gr_screen.bits_per_pixel == 32) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	}


}

void opengl_save_mouse_area(int x, int y, int w, int h)
{
	int cursor_size;

	GL_CHECK_FOR_ERRORS("start of save_mouse_area()");

	// lazy - taylor
	cursor_size = (Gr_cursor_size * Gr_cursor_size);

	// no reason to be bigger than the cursor, should never be smaller
	if (w != Gr_cursor_size)
		w = Gr_cursor_size;
	if (h != Gr_cursor_size)
		h = Gr_cursor_size;

	GL_mouse_saved_x1 = x;
	GL_mouse_saved_y1 = y;
	GL_mouse_saved_x2 = x+w-1;
	GL_mouse_saved_y2 = y+h-1;

	CLAMP(GL_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(GL_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( Use_PBOs ) {
		// since this is used a lot, and is pretty small in size, we just create it once and leave it until exit
		if (!GL_cursor_pbo) {
			vglGenBuffersARB(1, &GL_cursor_pbo);
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
			vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, cursor_size * 4, NULL, GL_STATIC_READ);
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	} else {
		// this should really only have to be malloc'd once
		if (GL_saved_mouse_data == NULL)
			GL_saved_mouse_data = (ubyte*)vm_malloc_q(cursor_size * 4);

		if (GL_saved_mouse_data == NULL)
			return;

		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, GL_saved_mouse_data);
	}

	GL_CHECK_FOR_ERRORS("end of save_mouse_area()");

	GL_mouse_saved = 1;
}

int gr_opengl_save_screen()
{
	int i;
	ubyte *sptr = NULL, *dptr = NULL;
	ubyte *opengl_screen_tmp = NULL;
	int width_times_pixel, mouse_times_pixel;

	gr_opengl_reset_clip();

	if (GL_saved_screen || GL_screen_pbo) {
		// already have a screen saved so just bail...
		return -1;
	}

	GL_saved_screen = (ubyte*)vm_malloc_q( gr_screen.max_w * gr_screen.max_h * 4 );

	if (!GL_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
 		return -1;
	}

	GLboolean save_state = GL_state.DepthTest(GL_FALSE);
	glReadBuffer(GL_FRONT_LEFT);

	if ( Use_PBOs ) {
		GLubyte *pixels = NULL;

		vglGenBuffersARB(1, &GL_screen_pbo);

		if (!GL_screen_pbo) {
			if (GL_saved_screen) {
				vm_free(GL_saved_screen);
				GL_saved_screen = NULL;
			}

			return -1;
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_screen_pbo);
		vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, gr_screen.max_w * gr_screen.max_h * 4, NULL, GL_STATIC_READ);

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (Gr_cursor_size * 4);

		sptr = (ubyte *)pixels;
		dptr = (ubyte *)&GL_saved_screen[gr_screen.max_w * gr_screen.max_h * 4];

		for (i = 0; i < gr_screen.max_h; i++) {
			dptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			sptr += width_times_pixel;
		}

		vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

		if (GL_mouse_saved && GL_cursor_pbo) {
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);

			pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

			sptr = (ubyte *)pixels;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < Gr_cursor_size; i++) {
				memcpy(dptr, sptr, mouse_times_pixel);
				sptr += mouse_times_pixel;
				dptr -= width_times_pixel;
			}

			vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		}

		vglDeleteBuffersARB(1, &GL_screen_pbo);
		GL_screen_pbo = 0;

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	} else {
		opengl_screen_tmp = (ubyte*)vm_malloc_q( gr_screen.max_w * gr_screen.max_h * 4 );

		if (!opengl_screen_tmp) {
			if (GL_saved_screen) {
				vm_free(GL_saved_screen);
				GL_saved_screen = NULL;
			}

			mprintf(( "Couldn't get memory for temporary saved screen!\n" ));
			GL_state.DepthTest(save_state);
	 		return -1;
	 	}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_read_format, GL_UNSIGNED_INT_8_8_8_8_REV, opengl_screen_tmp);

		sptr = (ubyte *)&opengl_screen_tmp[gr_screen.max_w * gr_screen.max_h * 4];
		dptr = (ubyte *)GL_saved_screen;

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (Gr_cursor_size * 4);

		for (i = 0; i < gr_screen.max_h; i++) {
			sptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			dptr += width_times_pixel;
		}

		vm_free(opengl_screen_tmp);

		if (GL_mouse_saved && GL_saved_mouse_data) {
			sptr = (ubyte *)GL_saved_mouse_data;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < Gr_cursor_size; i++) {
				memcpy(dptr, sptr, mouse_times_pixel);
				sptr += mouse_times_pixel;
				dptr -= width_times_pixel;
			}
		}

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	}

	GL_state.DepthTest(save_state);

	return GL_saved_screen_id;
}

void gr_opengl_restore_screen(int bmp_id)
{
	gr_reset_clip();

	if ( !GL_saved_screen ) {
		gr_clear();
		return;
	}

	Assert( (bmp_id < 0) || (bmp_id == GL_saved_screen_id) );

	if (GL_saved_screen_id < 0)
		return;

	gr_set_bitmap(GL_saved_screen_id);
	gr_bitmap(0, 0, false);	// don't scale here since we already have real screen size
}

void gr_opengl_free_screen(int bmp_id)
{
	if (!GL_saved_screen)
		return;

	vm_free(GL_saved_screen);
	GL_saved_screen = NULL;

	Assert( (bmp_id < 0) || (bmp_id == GL_saved_screen_id) );

	if (GL_saved_screen_id < 0)
		return;

	bm_release(GL_saved_screen_id);
	GL_saved_screen_id = -1;
}

static void opengl_flush_frame_dump()
{
	char filename[MAX_FILENAME_LEN];

	Assert( GL_dump_buffer != NULL);

	for (int i = 0; i < GL_dump_frame_count; i++) {
		sprintf(filename, NOX("frm%04d.tga"), GL_dump_frame_number );
		GL_dump_frame_number++;

		CFILE *f = cfopen(filename, "wb", CFILE_NORMAL, CF_TYPE_DATA);

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 2, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)gr_screen.max_w, f );	//	Width;
		cfwrite_ushort( (ushort)gr_screen.max_h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGR_EXT, GL_UNSIGNED_BYTE, GL_dump_buffer);

		// save the data out
		cfwrite( GL_dump_buffer, GL_dump_frame_size, 1, f );

		cfclose(f);

	}

	GL_dump_frame_count = 0;
}

void gr_opengl_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if ( GL_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}

	GL_dump_frames = 1;
	GL_dump_frame_number = first_frame;
	GL_dump_frame_count = 0;
	GL_dump_frame_count_max = frames_between_dumps; // only works if it's 1
	GL_dump_frame_size = gr_screen.max_w * gr_screen.max_h * 3;
	
	if ( !GL_dump_buffer ) {
		int size = GL_dump_frame_count_max * GL_dump_frame_size;

		GL_dump_buffer = (ubyte *)vm_malloc(size);

		if ( !GL_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

void gr_opengl_dump_frame_stop()
{
	if ( !GL_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	opengl_flush_frame_dump();
	
	GL_dump_frames = 0;

	if ( GL_dump_buffer )	{
		vm_free(GL_dump_buffer);
		GL_dump_buffer = NULL;
	}
}

void gr_opengl_dump_frame()
{
	GL_dump_frame_count++;

	if ( GL_dump_frame_count == GL_dump_frame_count_max ) {
		opengl_flush_frame_dump();
	}
}

//fill mode, solid/wire frame
void gr_opengl_set_fill_mode(int mode)
{
	if (mode == GR_FILL_MODE_SOLID) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		return;
	}

	if (mode == GR_FILL_MODE_WIRE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		return;
	}

	// default setting
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void gr_opengl_zbias(int bias)
{
	if (bias) {
		GL_state.PolygonOffsetFill(GL_TRUE);
		glPolygonOffset(0.0, -i2fl(bias));
	} else {
		GL_state.PolygonOffsetFill(GL_FALSE);
	}
}

void gr_opengl_push_texture_matrix(int unit)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units)
		return;

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glMatrixMode(current_matrix);
}

void gr_opengl_pop_texture_matrix(int unit)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units)
		return;

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(current_matrix);
}

void gr_opengl_translate_texture_matrix(int unit, vec3d *shift)
{
	GLint current_matrix;

	if (unit > GL_supported_texture_units) {
		/*tex_shift=*shift;*/
		return;
	}

	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	vglActiveTextureARB(GL_TEXTURE0_ARB+unit);

	glMatrixMode(GL_TEXTURE);
	glTranslated(shift->xyz.x, shift->xyz.y, shift->xyz.z);

	glMatrixMode(current_matrix);

//	tex_shift=vmd_zero_vector;
}

void gr_opengl_setup_background_fog(bool set)
{
	if (Cmdline_nohtl) {
		return;
	}
}

void gr_opengl_set_line_width(float width)
{
	glLineWidth(width);
}

// Returns the human readable error string if there is an error or NULL if not
const char *opengl_error_string()
{
	GLenum error = GL_NO_ERROR;

	error = glGetError();

	if ( error != GL_NO_ERROR ) {
		return (const char *)gluErrorString(error);
	}

	return NULL;
}

int opengl_check_for_errors(char *err_at)
{
	const char *error_str = NULL;
	int num_errors = 0;

	error_str = opengl_error_string();

	if (error_str) {
		if (err_at != NULL) {
			nprintf(("OpenGL", "OpenGL Error from %s: %s\n", err_at, error_str));
		} else {
			nprintf(("OpenGL", "OpenGL Error: %s\n", error_str));
		}

		num_errors++;
	}

	return num_errors;
}

void opengl_set_vsync(int status)
{
	if ( (status < 0) || (status > 1) ) {
		Int3();
		return;
	}

#if defined(__APPLE__)
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, (long*)&status);
#elif defined(_WIN32)
	vwglSwapIntervalEXT(status);
#else
	// NOTE: this may not work well with the closed NVIDIA drivers since those use the
	//       special "__GL_SYNC_TO_VBLANK" environment variable to manage sync
	vglXSwapIntervalSGI(status);
#endif

	GL_CHECK_FOR_ERRORS("end of set_vsync()");
}

void opengl_setup_viewport()
{
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_framebuffer) {
		glOrtho(0, gr_screen.max_w, 0, gr_screen.max_h, -1.0, 1.0);
	} else {
		glOrtho(0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// NOTE: This should only ever be called through os_cleanup(), or when switching video APIs
void gr_opengl_shutdown()
{
	if (GL_cursor_pbo) {
		vglDeleteBuffersARB(1, &GL_cursor_pbo);
		GL_cursor_pbo = 0;
	}

	if (GL_saved_mouse_data != NULL) {
		vm_free(GL_saved_mouse_data);
		GL_saved_mouse_data = NULL;
	}

	opengl_tcache_shutdown();
	opengl_light_shutdown();
	opengl_tnl_shutdown();
	opengl_shader_shutdown();

	GL_initted = false;

#ifdef _WIN32
	// restore original gamma settings
	if (GL_original_gamma_ramp != NULL) {
		SetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

	// swap out our window mode and un-jail the cursor
	ShowWindow((HWND)os_get_window(), SW_HIDE);
	ClipCursor(NULL);
	ChangeDisplaySettings( NULL, 0 );
#else
	if (GL_original_gamma_ramp != NULL) {
		SDL_SetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}
#endif

	if (GL_original_gamma_ramp != NULL) {
		vm_free(GL_original_gamma_ramp);
		GL_original_gamma_ramp = NULL;
	}

#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);

	if (GL_render_context) {
		wglDeleteContext(GL_render_context);
		GL_render_context = NULL;
	}

	GL_device_context = NULL;
#endif
}

// NOTE: This should only ever be called through atexit()!!!
void opengl_close()
{
//	if ( !GL_initted )
//		return;
}

int opengl_init_display_device()
{
	int bpp = gr_screen.bits_per_pixel;

	if ( (bpp != 16) && (bpp != 32) ) {
		Int3();
		return 1;
	}


	// screen format
	switch (bpp) {
		case 16: {
			Gr_red.bits = 5;
			Gr_red.shift = 11;
			Gr_red.scale = 8;
			Gr_red.mask = 0xF800;

			Gr_green.bits = 6;
			Gr_green.shift = 5;
			Gr_green.scale = 4;
			Gr_green.mask = 0x7E0;

			Gr_blue.bits = 5;
			Gr_blue.shift = 0;
			Gr_blue.scale = 8;
			Gr_blue.mask = 0x1F;		

			break;
		}

		case 32: {
			Gr_red.bits = 8;
			Gr_red.shift = 16;
			Gr_red.scale = 1;
			Gr_red.mask = 0xff0000;

			Gr_green.bits = 8;
			Gr_green.shift = 8;
			Gr_green.scale = 1;
			Gr_green.mask = 0x00ff00;

			Gr_blue.bits = 8;
			Gr_blue.shift = 0;
			Gr_blue.scale = 1;
			Gr_blue.mask = 0x0000ff;

			Gr_alpha.bits = 8;
			Gr_alpha.shift = 24;
			Gr_alpha.mask = 0xff000000;
			Gr_alpha.scale = 1;

			break;
		}
	}

	// texture format
	Gr_t_red.bits = 5;
	Gr_t_red.mask = 0x7c00;
	Gr_t_red.shift = 10;
	Gr_t_red.scale = 8;
	
	Gr_t_green.bits = 5;
	Gr_t_green.mask = 0x03e0;
	Gr_t_green.shift = 5;
	Gr_t_green.scale = 8;
	
	Gr_t_blue.bits = 5;
	Gr_t_blue.mask = 0x001f;
	Gr_t_blue.shift = 0;
	Gr_t_blue.scale = 8;
	
	Gr_t_alpha.bits = 1;
	Gr_t_alpha.mask = 0x8000;
	Gr_t_alpha.scale = 255;
	Gr_t_alpha.shift = 15;

	// alpha-texture format
	Gr_ta_red.bits = 4;
	Gr_ta_red.mask = 0x0f00;
	Gr_ta_red.shift = 8;
	Gr_ta_red.scale = 17;
	
	Gr_ta_green.bits = 4;
	Gr_ta_green.mask = 0x00f0;
	Gr_ta_green.shift = 4;
	Gr_ta_green.scale = 17;
	
	Gr_ta_blue.bits = 4;
	Gr_ta_blue.mask = 0x000f;
	Gr_ta_blue.shift = 0;
	Gr_ta_blue.scale = 17;
	
	Gr_ta_alpha.bits = 4;
	Gr_ta_alpha.mask = 0xf000;
	Gr_ta_alpha.shift = 12;
	Gr_ta_alpha.scale = 17;

	// allocate storage for original gamma settings
	if ( !Cmdline_no_set_gamma && (GL_original_gamma_ramp == NULL) ) {
		GL_original_gamma_ramp = (ushort*) vm_malloc_q( 3 * 256 * sizeof(ushort) );

		if (GL_original_gamma_ramp == NULL) {
			mprintf(("  Unable to allocate memory for gamma ramp!  Disabling...\n"));
			Cmdline_no_set_gamma = 1;
		} else {
			// assume identity ramp by default, to be overwritten by true ramp later
			for (int x = 0; x < 256; x++) {
				GL_original_gamma_ramp[x] = GL_original_gamma_ramp[x + 256] = GL_original_gamma_ramp[x + 512] = (x << 8) | x;
			}
		}
	}


	// now init the display device
#ifdef _WIN32
	int PixelFormat;
	HWND wnd = 0;
	PIXELFORMATDESCRIPTOR pfd_test;

	mprintf(("  Initializing WGL...\n"));

	memset(&GL_pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset(&pfd_test, 0, sizeof(PIXELFORMATDESCRIPTOR));

	GL_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	GL_pfd.nVersion = 1;
	GL_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	GL_pfd.iPixelType = PFD_TYPE_RGBA;
	GL_pfd.cColorBits = (ubyte)bpp;
	GL_pfd.cRedBits = (ubyte)Gr_red.bits;
	GL_pfd.cGreenBits = (ubyte)Gr_green.bits;
	GL_pfd.cBlueBits = (ubyte)Gr_blue.bits;
	GL_pfd.cAlphaBits = (bpp == 32) ? (ubyte)Gr_alpha.bits : 0;
	GL_pfd.cDepthBits = (bpp == 32) ? 24 : 16;


	wnd = (HWND)os_get_window();

	Assert( wnd != NULL );

	extern uint os_get_dc();
	GL_device_context = (HDC)os_get_dc();

	if ( !GL_device_context ) {
		MessageBox(wnd, "Unable to get device context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	PixelFormat = ChoosePixelFormat(GL_device_context, &GL_pfd);

	if ( !PixelFormat ) {
		MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
		return 1;
	} else {
		DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);

		// make sure that we are hardware accelerated and not using the generic implementation
		if ( !Fred_running && (pfd_test.dwFlags & PFD_GENERIC_FORMAT) && !(pfd_test.dwFlags & PFD_GENERIC_ACCELERATED) ) {
			Assert( bpp == 32 );

			// if we failed at 32-bit then we are probably a 16-bit desktop, so try and init a 16-bit visual instead
			GL_pfd.cAlphaBits = 0;
			GL_pfd.cDepthBits = 16;
			// NOTE: the bit values for colors should get updated automatically by ChoosePixelFormat()

			PixelFormat = ChoosePixelFormat(GL_device_context, &GL_pfd);

			if (!PixelFormat) {
				MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
				return 1;
			}

			// double-check that we are correct now
			DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);

			if ( (pfd_test.dwFlags & PFD_GENERIC_FORMAT) && !(pfd_test.dwFlags & PFD_GENERIC_ACCELERATED) ) {
				MessageBox(wnd, "Unable to get proper pixel format for OpenGL W32!", "Error", MB_ICONERROR | MB_OK);
				return 1;
			}
		}
	}

	if ( !SetPixelFormat(GL_device_context, PixelFormat, &GL_pfd) ) {
		MessageBox(wnd, "Unable to set pixel format for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	if ( !(GL_render_context = wglCreateContext(GL_device_context)) ) {
		MessageBox(wnd, "Unable to create rendering context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	if ( !wglMakeCurrent(GL_device_context, GL_render_context) ) {
		MessageBox(wnd, "Unable to make current thread for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	mprintf(("  Requested WGL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", Gr_red.bits, Gr_green.bits, Gr_blue.bits, GL_pfd.cColorBits, (GL_pfd.dwFlags & PFD_DOUBLEBUFFER) > 0));

	// now report back as to what we ended up getting
	int r = 0, g = 0, b = 0, depth = 0, db = 1;

	DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &GL_pfd);

	r = GL_pfd.cRedBits;
	g = GL_pfd.cGreenBits;
	b = GL_pfd.cBlueBits;
	depth = GL_pfd.cColorBits;
	db = ((GL_pfd.dwFlags & PFD_DOUBLEBUFFER) > 0);

	mprintf(("  Actual WGL Video values    = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", r, g, b, depth, db));

	// get the default gamma ramp so that we can restore it on close
	if (GL_original_gamma_ramp != NULL) {
		GetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );
	}

#else

	int flags = SDL_OPENGL;
	int r = 0, g = 0, b = 0, depth = 0, db = 1;

	mprintf(("  Initializing SDL...\n"));

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		fprintf (stderr, "Couldn't init SDL: %s", SDL_GetError());
		return 1;
	}

	// grab mouse/key unless told otherwise, ignore when we are going fullscreen
	if ( (Cmdline_window || os_config_read_uint(NULL, "Fullscreen", 1) == 0) && !Cmdline_no_grab ) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, Gr_red.bits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, Gr_green.bits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, Gr_blue.bits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (bpp == 32) ? 24 : 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, db);

	mprintf(("  Requested SDL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", Gr_red.bits, Gr_green.bits, Gr_blue.bits, (bpp == 32) ? 24 : 16, db));

	if (SDL_SetVideoMode(gr_screen.max_w, gr_screen.max_h, bpp, flags) == NULL) {
		fprintf (stderr, "Couldn't set video mode: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &db);

	mprintf(("  Actual SDL Video values    = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", r, g, b, depth, db));

	SDL_ShowCursor(0);

	/* might as well put this here */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	if (GL_original_gamma_ramp != NULL) {
		SDL_GetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
	}
#endif

	return 0;
}


void opengl_setup_function_pointers()
{
	// *****************************************************************************
	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.

	gr_screen.gf_flip				= gr_opengl_flip;
	gr_screen.gf_set_clip			= gr_opengl_set_clip;
	gr_screen.gf_reset_clip			= gr_opengl_reset_clip;
	
	gr_screen.gf_clear				= gr_opengl_clear;
//	gr_screen.gf_bitmap				= gr_opengl_bitmap;
	gr_screen.gf_bitmap_ex			= gr_opengl_bitmap_ex;
	gr_screen.gf_aabitmap			= gr_opengl_aabitmap;
	gr_screen.gf_aabitmap_ex		= gr_opengl_aabitmap_ex;
	
//	gr_screen.gf_rect				= gr_opengl_rect;
//	gr_screen.gf_shade				= gr_opengl_shade;
	gr_screen.gf_string				= gr_opengl_string;
	gr_screen.gf_circle				= gr_opengl_circle;
	gr_screen.gf_curve				= gr_opengl_curve;

	gr_screen.gf_line				= gr_opengl_line;
	gr_screen.gf_aaline				= gr_opengl_aaline;
	gr_screen.gf_pixel				= gr_opengl_pixel;
	gr_screen.gf_scaler				= gr_opengl_scaler;
	gr_screen.gf_tmapper			= gr_opengl_tmapper;

	gr_screen.gf_gradient			= gr_opengl_gradient;

	gr_screen.gf_set_palette		= gr_opengl_set_palette;
	gr_screen.gf_print_screen		= gr_opengl_print_screen;

	gr_screen.gf_fade_in			= gr_opengl_fade_in;
	gr_screen.gf_fade_out			= gr_opengl_fade_out;
	gr_screen.gf_flash				= gr_opengl_flash;
	gr_screen.gf_flash_alpha		= gr_opengl_flash_alpha;
	
	gr_screen.gf_zbuffer_get		= gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set		= gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear		= gr_opengl_zbuffer_clear;
	
	gr_screen.gf_save_screen		= gr_opengl_save_screen;
	gr_screen.gf_restore_screen		= gr_opengl_restore_screen;
	gr_screen.gf_free_screen		= gr_opengl_free_screen;
	
	gr_screen.gf_dump_frame_start	= gr_opengl_dump_frame_start;
	gr_screen.gf_dump_frame_stop	= gr_opengl_dump_frame_stop;
	gr_screen.gf_dump_frame			= gr_opengl_dump_frame;
	
	gr_screen.gf_set_gamma			= gr_opengl_set_gamma;

	gr_screen.gf_fog_set			= gr_opengl_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region			= gr_opengl_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data			= gr_opengl_bm_free_data;
	gr_screen.gf_bm_create				= gr_opengl_bm_create;
	gr_screen.gf_bm_init				= gr_opengl_bm_init;
	gr_screen.gf_bm_load				= gr_opengl_bm_load;
	gr_screen.gf_bm_page_in_start		= gr_opengl_bm_page_in_start;
	gr_screen.gf_bm_lock				= gr_opengl_bm_lock;
	gr_screen.gf_bm_make_render_target	= gr_opengl_bm_make_render_target;
	gr_screen.gf_bm_set_render_target	= gr_opengl_bm_set_render_target;

	gr_screen.gf_set_cull			= gr_opengl_set_cull;

	gr_screen.gf_cross_fade			= gr_opengl_cross_fade;

	gr_screen.gf_tcache_set			= gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color	= gr_opengl_set_clear_color;

	gr_screen.gf_preload			= gr_opengl_preload;

	gr_screen.gf_push_texture_matrix		= gr_opengl_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix			= gr_opengl_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix	= gr_opengl_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing	= gr_opengl_set_texture_addressing;
	gr_screen.gf_zbias					= gr_opengl_zbias;
	gr_screen.gf_set_fill_mode			= gr_opengl_set_fill_mode;
	gr_screen.gf_set_texture_panning	= gr_opengl_set_texture_panning;

	gr_screen.gf_make_buffer		= gr_opengl_make_buffer;
	gr_screen.gf_destroy_buffer		= gr_opengl_destroy_buffer;
	gr_screen.gf_render_buffer		= gr_opengl_render_buffer;
	gr_screen.gf_set_buffer			= gr_opengl_set_buffer;

	gr_screen.gf_start_instance_matrix			= gr_opengl_start_instance_matrix;
	gr_screen.gf_end_instance_matrix			= gr_opengl_end_instance_matrix;
	gr_screen.gf_start_angles_instance_matrix	= gr_opengl_start_instance_angles;

	gr_screen.gf_make_light			= gr_opengl_make_light;
	gr_screen.gf_modify_light		= gr_opengl_modify_light;
	gr_screen.gf_destroy_light		= gr_opengl_destroy_light;
	gr_screen.gf_set_light			= gr_opengl_set_light;
	gr_screen.gf_reset_lighting		= gr_opengl_reset_lighting;
	gr_screen.gf_set_ambient_light	= gr_opengl_set_ambient_light;

	gr_screen.gf_start_clip_plane	= gr_opengl_start_clip_plane;
	gr_screen.gf_end_clip_plane		= gr_opengl_end_clip_plane;

	gr_screen.gf_lighting			= gr_opengl_set_lighting;

	gr_screen.gf_set_proj_matrix	= gr_opengl_set_projection_matrix;
	gr_screen.gf_end_proj_matrix	= gr_opengl_end_projection_matrix;

	gr_screen.gf_set_view_matrix	= gr_opengl_set_view_matrix;
	gr_screen.gf_end_view_matrix	= gr_opengl_end_view_matrix;

	gr_screen.gf_push_scale_matrix	= gr_opengl_push_scale_matrix;
	gr_screen.gf_pop_scale_matrix	= gr_opengl_pop_scale_matrix;
	gr_screen.gf_center_alpha		= gr_opengl_center_alpha;

	gr_screen.gf_setup_background_fog	= gr_opengl_setup_background_fog;

	gr_screen.gf_start_state_block	= gr_opengl_start_state_block;
	gr_screen.gf_end_state_block	= gr_opengl_end_state_block;
	gr_screen.gf_set_state_block	= gr_opengl_set_state_block;

	gr_screen.gf_draw_line_list		= gr_opengl_draw_line_list;

	gr_screen.gf_set_line_width		= gr_opengl_set_line_width;

	gr_screen.gf_line_htl			= gr_opengl_line_htl;
	gr_screen.gf_sphere_htl			= gr_opengl_sphere_htl;

	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.
	// *****************************************************************************
}


bool gr_opengl_init()
{
	char *ver;
	int major = 0, minor = 0;

	if ( !GL_initted )
		atexit(opengl_close);

	if (GL_initted) {
		gr_opengl_cleanup();
		GL_initted = false;
	}

	mprintf(( "Initializing OpenGL graphics device at %ix%i with %i-bit color...\n", gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel ));

	if ( opengl_init_display_device() ) {
		Error(LOCATION, "Unable to initialize display device!\n");
	}

	// version check
	ver = (char *)glGetString(GL_VERSION);
	sscanf(ver, "%d.%d", &major, &minor);

	if ( (major < REQUIRED_GL_MAJOR_VERSION) || ((major <= REQUIRED_GL_MAJOR_VERSION) && (minor < REQUIRED_GL_MINOR_VERSION)) ) {
		Error(LOCATION, "Current GL Version of %i.%i is less than the required version of %i.%i.\nSwitch video modes or update your drivers.", major, minor, REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
	}

	GL_initted = true;

	// this MUST be done before any other gr_opengl_* or opengl_* funcion calls!!
	opengl_setup_function_pointers();

	mprintf(( "  OpenGL Vendor     : %s\n", glGetString(GL_VENDOR) ));
	mprintf(( "  OpenGL Renderer   : %s\n", glGetString(GL_RENDERER) ));
	mprintf(( "  OpenGL Version    : %s\n", ver ));
	mprintf(( "\n" ));

	if (Cmdline_window) {
		opengl_go_windowed();
	} else {
		opengl_go_fullscreen();
	}

	// initialize the extensions and make sure we aren't missing something that we need
	opengl_extensions_init();

	// setup the lighting stuff that will get used later
	opengl_light_init();
	
	// init state system (must come AFTER light is set up)
	GL_state.init();

	// ready the texture system
	opengl_tcache_init();

	extern void opengl_tnl_init();
	opengl_tnl_init();

	// setup default shaders, and shader related items
	opengl_shader_init();

	// must be called after extensions are setup
	opengl_set_vsync( !Cmdline_no_vsync );

	GLint max_texture_units = GL_supported_texture_units;

	if (Use_GLSL) {
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_texture_units);
	}

	GL_state.Texture.init(max_texture_units);

	opengl_set_texture_target();
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable();
	GL_state.Texture.SetWrapS(GL_CLAMP_TO_EDGE);
	GL_state.Texture.SetWrapT(GL_CLAMP_TO_EDGE);
	GL_state.Texture.SetWrapR(GL_CLAMP_TO_EDGE);

	opengl_setup_viewport();

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);

	glDepthRange(0.0, 1.0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glFlush();

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	Mouse_hidden++;
	gr_opengl_reset_clip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();
	gr_opengl_flip();
	gr_opengl_clear();
	Mouse_hidden--;


	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &GL_max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &GL_max_elements_indices);

	mprintf(( "  Max texture units: %i (%i)\n", GL_supported_texture_units, max_texture_units ));
	mprintf(( "  Max elements vertices: %i\n", GL_max_elements_vertices ));
	mprintf(( "  Max elements indices: %i\n", GL_max_elements_indices ));
	mprintf(( "  Max texture size: %ix%i\n", GL_max_texture_width, GL_max_texture_height ));
	mprintf(( "  Can use compressed textures: %s\n", Use_compressed_textures ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Texture compression available: %s\n", Texture_compression_available ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Using %s texture filter.\n", (GL_mipmap_filter) ? NOX("trilinear") : NOX("bilinear") ));

	if (Use_GLSL) {
		mprintf(( "  Using GLSL for model rendering.\n" ));
		mprintf(( "  Shader Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION_ARB) ));
	}

	// This stops fred crashing if no textures are set
	gr_screen.current_bitmap = -1;

	TIMERBAR_SET_DRAW_FUNC(opengl_render_timer_bar);

	mprintf(("... OpenGL init is complete!\n"));

    if (Cmdline_ati_color_swap)
        GL_read_format = GL_RGBA;

	return true;
}

DCF(ogl_minimize, "Minimizes opengl")
{
	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Command only available in OpenGL mode.\n");
		return;
	}

	if (Dc_command) {
		dc_get_arg(ARG_TRUE);

		if ( Dc_arg_type & ARG_TRUE ) {
			opengl_minimize();
		}
	}

	if (Dc_help)
		dc_printf("If set to true then the OpenGL window will minimize.\n");
}

DCF(ogl_anisotropy, "toggles anisotropic filtering")
{
	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Can only set anisotropic filter in OpenGL mode.\n");
		return;
	}

	if ( Dc_command && !Is_Extension_Enabled(OGL_EXT_TEXTURE_FILTER_ANISOTROPIC) ) {
		dc_printf("Error: Anisotropic filter is not settable!\n");
		return;
	}

	if ( Dc_command ) {
		dc_get_arg(ARG_INT | ARG_NONE);

		if ( Dc_arg_type & ARG_NONE ) {
			GL_anisotropy = 1.0f;
		//	opengl_set_anisotropy();
			dc_printf("Anisotropic filter value reset to default level.\n");
		}

		if ( Dc_arg_type & ARG_INT ) {
			GL_anisotropy = (GLfloat)Dc_arg_float;
		//	opengl_set_anisotropy( (float)Dc_arg_float );
		}
	}

	if ( Dc_status ) {
		dc_printf("Current anisotropic filter value is %i\n", (int)GL_anisotropy);
	}

	if (Dc_help) {
		dc_printf("Sets OpenGL anisotropic filtering level.\n");
		dc_printf("Valid values are 1 to %i, or 0 to turn off.\n", (int)opengl_get_max_anisotropy());
	}
}
