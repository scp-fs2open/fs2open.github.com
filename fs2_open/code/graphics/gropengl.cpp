

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


static int GL_initted = 0;

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
static int OGL_fogmode=0;

#ifdef _WIN32
static HDC GL_device_context = NULL;
static HGLRC rend_context = NULL;
static PIXELFORMATDESCRIPTOR pfd;
#endif

static ushort *GL_original_gamma_ramp = NULL;

int VBO_ENABLED = 0;
int Use_PBOs = 0;

static int GL_dump_frames = 0;
static ubyte *GL_dump_buffer = NULL;
static int GL_dump_frame_number = 0;
static int GL_dump_frame_count = 0;
static int GL_dump_frame_count_max = 0;
static int GL_dump_frame_size = 0;

static ubyte *GL_saved_screen = NULL;
static ubyte *GL_saved_mouse_data = NULL;
static int GL_saved_screen_id = -1;
static int GL_mouse_saved_size = 32; // width and height, so they're equal
static GLuint GL_cursor_pbo = 0;
static GLuint GL_screen_pbo = 0;

static int GL_mouse_saved = 0;
static int GL_mouse_saved_x1 = 0;
static int GL_mouse_saved_y1 = 0;
static int GL_mouse_saved_x2 = 0;
static int GL_mouse_saved_y2 = 0;

static int ogl_maybe_pop_arb1 = 0;

void (*opengl_set_tex_src)(gr_texture_source ts);

gr_texture_source	GL_current_tex_src = TEXTURE_SOURCE_NONE;
gr_alpha_blend		GL_current_alpha_blend = ALPHA_BLEND_NONE;
gr_zbuffer_type		GL_current_ztype = ZBUFFER_TYPE_NONE;

void opengl_save_mouse_area(int x, int y, int w, int h);

extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

// some globals
extern matrix View_matrix;
extern vec3d View_position;

extern matrix Eye_matrix;
extern vec3d Eye_position;

extern vec3d Object_position;
extern matrix Object_matrix;

extern float Canv_w2;			// Canvas_width / 2
extern float Canv_h2;			// Canvas_height / 2
extern float View_zoom;

extern vec3d *Interp_pos;
extern vec3d Interp_offset;
extern matrix *Interp_orient;

extern char *Osreg_title;

extern int SPECMAP;

extern GLfloat GL_anisotropy;

extern void opengl_default_light_settings(int amb = 1, int emi = 1, int spec = 1);
extern void opengl_tcache_frame();
extern void opengl_tcache_flush();
extern void opengl_tcache_cleanup();

extern float FreeSpace_gamma;
void gr_opengl_set_gamma(float gamma);

static int GL_fullscreen = 0;
static int GL_windowed = 0;
static int GL_minimized = 0;


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

/*void opengl_set_tex_state_combine(gr_texture_source ts)
{
	if (ts == GL_current_tex_src)
		return;

	switch (ts) {
		case TEXTURE_SOURCE_NONE:
			opengl_switch_arb(-1, 0);
			gr_opengl_tcache_set(-1, -1, NULL, NULL );
			break;

		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
			break;

		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb(0, 1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
			break;

		default:
			return;
	}

	GL_current_tex_src = ts;
}*/

void opengl_set_tex_state_no_combine(gr_texture_source ts)
{
	if (ts == GL_current_tex_src)
		return;

	switch (ts)
	{
		case TEXTURE_SOURCE_NONE:
			opengl_switch_arb(-1, 0);
			gr_opengl_tcache_set(-1, -1, NULL, NULL );
			break;

		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			break;

		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb(0, 1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			break;

		default:
			return;
	}

	GL_current_tex_src = ts;
}

void opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt)
{

	opengl_set_tex_src(ts);

	if (ab != GL_current_alpha_blend) {
		switch (ab)
		{
			case ALPHA_BLEND_NONE:
				glBlendFunc(GL_ONE, GL_ZERO);
				break;

			case ALPHA_BLEND_ADDITIVE:
				glBlendFunc(GL_ONE, GL_ONE);
				break;

			case ALPHA_BLEND_ALPHA_ADDITIVE:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;

			case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;

			case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
				glBlendFunc(/*GL_SRC_COLOR*/GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
				break;
	
			default:
				break;
		}

		GL_current_alpha_blend = ab;

		if (GL_current_alpha_blend == ALPHA_BLEND_NONE) {
			glDisable(GL_BLEND);
		} else {
			glEnable(GL_BLEND);
		}

	/*	if (GL_current_alpha_blend <= ALPHA_BLEND_ADDITIVE) {
			glDisable(GL_ALPHA_TEST);
		} else {
			glEnable(GL_ALPHA_TEST);
		}*/
	}

	if (zt != GL_current_ztype) {
		switch (zt)
		{
			case ZBUFFER_TYPE_NONE:
				glDepthFunc(GL_ALWAYS);
				glDepthMask(GL_FALSE);
				break;

			case ZBUFFER_TYPE_READ:
				glDepthFunc(GL_LESS);
				glDepthMask(GL_FALSE);
				break;

			case ZBUFFER_TYPE_WRITE:
				glDepthFunc(GL_ALWAYS);
				glDepthMask(GL_TRUE);
				break;

			case ZBUFFER_TYPE_FULL:
				glDepthFunc(GL_LESS);
				glDepthMask(GL_TRUE);
				break;

			default:
				break;
		}

		GL_current_ztype = zt;

		if (GL_current_ztype == ZBUFFER_TYPE_NONE) {
			glDisable(GL_DEPTH_TEST);
		} else {
			glEnable(GL_DEPTH_TEST);
		}
	}
}

void gr_opengl_pixel(int x, int y, bool resize)
{
	gr_line(x,y,x,y,resize);
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
	if (!GL_initted)
		return;

	gr_reset_clip();

	mouse_eval_deltas();
	
	GL_mouse_saved = 0;
	
	if ( mouse_is_visible() ) {
		int mx, my;

		gr_reset_clip();
		mouse_get_pos( &mx, &my );

		opengl_save_mouse_area(mx, my, GL_mouse_saved_size, GL_mouse_saved_size);

		if ( Gr_cursor != -1 ) {
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

void gr_opengl_flip_window(uint _hdc, int x, int y, int w, int h )
{
	// Not used.
}

void gr_opengl_set_clip(int x, int y, int w, int h, bool resize)
{
	// check for sanity of parameters
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	int to_resize = (resize || gr_screen.rendering_to_texture != -1);

	int max_w = ((to_resize) ? gr_screen.max_w_unscaled : gr_screen.max_w);
	int max_h = ((to_resize) ? gr_screen.max_h_unscaled : gr_screen.max_h);

	if (x >= max_w)
		x = max_w - 1;
	if (y >= max_h)
		y = max_h - 1;

	if (x + w > max_w)
		w = max_w - x;
	if (y + h > max_h)
		h = max_h - y;
	
	if (w > max_w)
		w = max_w;
	if (h > max_h)
		h = max_h;

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

	// just return early if we aren't actually going to need the scissor test
	if ( (x == 0) && (y == 0) && (w == max_w) && (h == max_h) ) {
		glDisable(GL_SCISSOR_TEST);
		return;
	}
	
	glEnable(GL_SCISSOR_TEST);
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

	if (gr_screen.custom_size >= 0) {
		gr_unsize_screen_pos( &gr_screen.clip_right_unscaled, &gr_screen.clip_bottom_unscaled );
		gr_unsize_screen_pos( &gr_screen.clip_width_unscaled, &gr_screen.clip_height_unscaled );
	}

	gr_screen.clip_aspect = i2fl(gr_screen.clip_width) / i2fl(gr_screen.clip_height);

	glDisable(GL_SCISSOR_TEST);
}

void gr_opengl_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void opengl_aabitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )
		return;

	float u_scale, v_scale;

	if ( !gr_opengl_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		mprintf(( "WARNING: Error setting aabitmap texture!\n" ));
		return;
	}

	opengl_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	u0 = u_scale * (i2fl(sx) / i2fl(bw));
	v0 = v_scale * (i2fl(sy) / i2fl(bh));

	u1 = u_scale * (i2fl(sx+w) / i2fl(bw));
	v1 = v_scale * (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if ( do_resize ) {
		gr_resize_screen_posf( &x1, &y1 );
		gr_resize_screen_posf( &x2, &y2 );
	}

	Assert( gr_screen.current_color.is_alphacolor );
	glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	if (mirror) {
		float temp = u0;
		u0 = u1;
		u1 = temp;
	}

	glBegin (GL_QUADS);
		glTexCoord2f (u0, v1);
		glVertex2f (x1, y2);

		glTexCoord2f (u1, v1);
		glVertex2f (x2, y2);

		glTexCoord2f (u1, v0);
		glVertex2f (x2, y1);

		glTexCoord2f (u0, v0);
		glVertex2f (x1, y1);
	glEnd ();
}

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh, do_resize;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ( (dx1 > clip_right) || (dx2 < clip_left) ) return;
		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) return;
		if ( dx1 < clip_left ) { sx += clip_left-dx1; dx1 = clip_left; }
		if ( dy1 < clip_top ) { sy += clip_top-dy1; dy1 = clip_top; }
		if ( dx2 > clip_right ) { dx2 = clip_right; }
		if ( dy2 > clip_bottom ) { dy2 = clip_bottom; }


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
		Assert( (dx1 >= clip_left ) && (dx1 <= clip_right) );
		Assert( (dx2 >= clip_left ) && (dx2 <= clip_right) );
		Assert( (dy1 >= clip_top ) && (dy1 <= clip_bottom) );
		Assert( (dy2 >= clip_top ) && (dy2 <= clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	opengl_aabitmap_ex_internal(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize, mirror);
}

void gr_opengl_aabitmap(int x, int y, bool resize, bool mirror)
{
	int w, h, do_resize;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	if ( (dx1 > clip_right) || (dx2 < clip_left) )
		return;

	if ( (dy1 > clip_bottom) || (dy2 < clip_top) )
		return;

	if ( dx1 < clip_left ) {
		sx = clip_left-dx1;
		dx1 = clip_left;
	}

	if ( dy1 < clip_top ) {
		sy = clip_top-dy1;
		dy1 = clip_top;
	}

	if ( dx2 > clip_right )
		dx2 = clip_right;

	if ( dy2 > clip_bottom )
		dy2 = clip_bottom;

	if ( sx < 0 )
		return;

	if ( sy < 0 )
		return;

	if ( sx >= w )
		return;

	if ( sy >= h )
		return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_opengl_aabitmap_ex(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize, mirror);
}


void gr_opengl_string( int sx, int sy, char *s, bool resize = true )
{
	TIMERBAR_PUSH(4);

	int width, spacing, letter;
	int x, y, do_resize;
	float bw, bh;
	float u0, u1, v0, v1;
	int x1, x2, y1, y2;
	float u_scale, v_scale;

	if ( !Current_font || (*s == 0) )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

	if ( !gr_opengl_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		return;
	}

	opengl_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	int ibw, ibh;
	bm_get_info( gr_screen.current_bitmap, &ibw, &ibh );

	bw = i2fl(ibw);
	bh = i2fl(ibh);
	
	// set color!
	if ( gr_screen.current_color.is_alphacolor )	{
		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	} else {
		glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}

	spacing = 0;

	// start rendering...
	glBegin(GL_QUADS);

	// pick out letter coords, draw it, goto next letter and do the same
	while (*s)	{
		x += spacing;

		while (*s == '\n' )	{
			s++;
			y += Current_font->h;
			if (sx == 0x8000) {	// centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}

		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//not in font, draw as space
		if (letter < 0) {
			continue;
		}

		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( x + width < clip_left ) continue;
		if ( y + Current_font->h < clip_top ) continue;
		if ( x > clip_right ) continue;
		if ( y > clip_bottom ) continue;

		xd = yd = 0;
		if ( x < clip_left ) xd = clip_left - x;
		if ( y < clip_top ) yd = clip_top - y;
		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;
		if ( xc + wc > clip_right ) wc = clip_right - xc;
		if ( yc + hc > clip_bottom ) hc = clip_bottom - yc;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		x1 = xc + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
		y1 = yc + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);
		x2 = x1 + wc;
		y2 = y1 + hc;

		if ( do_resize ) {
			gr_resize_screen_pos( &x1, &y1 );
			gr_resize_screen_pos( &x2, &y2 );
		}

		u0 = u_scale * (i2fl(u+xd) / bw);
		v0 = v_scale * (i2fl(v+yd) / bh);

		u1 = u_scale * (i2fl((u+xd)+wc) / bw);
		v1 = v_scale * (i2fl((v+yd)+hc) / bh);

		glTexCoord2f (u0, v1);
		glVertex2i (x1, y2);

		glTexCoord2f (u1, v1);
		glVertex2i (x2, y2);

		glTexCoord2f (u1, v0);
		glVertex2i (x2, y1);

		glTexCoord2f (u0, v0);
		glVertex2i (x1, y1);
	}

	// done!
	glEnd();

	TIMERBAR_POP();
}

void gr_opengl_line(int x1,int y1,int x2,int y2, bool resize)
{
	int do_resize, clipped = 0, swapped = 0;
	float sx1, sy1;
	float sx2, sy2;

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);
	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);


	INT_CLIPLINE(x1, y1, x2, y2, clip_left, clip_top, clip_right, clip_bottom, return, clipped=1, swapped=1);

	sx1 = i2fl(x1 + offset_x);
	sy1 = i2fl(y1 + offset_y);
	sx2 = i2fl(x2 + offset_x);
	sy2 = i2fl(y2 + offset_y);


	if (do_resize) {
		gr_resize_screen_posf(&sx1, &sy1);
		gr_resize_screen_posf(&sx2, &sy2);
	}

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( x1 == x2 && y1 == y2 ) {
		gr_opengl_set_2d_matrix();

		glBegin (GL_POINTS);
			glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

			glVertex3f (sx1, sy1, -0.99f);
		glEnd ();

		gr_opengl_end_2d_matrix();

		return;
	}

	if ( x1 == x2 ) {
		if ( sy1 < sy2 )    {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if ( y1 == y2 )  {
		if ( sx1 < sx2 )    {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	gr_opengl_set_2d_matrix();

	glBegin (GL_LINES);
		glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		glVertex3f (sx2, sy2, -0.99f);
		glVertex3f (sx1, sy1, -0.99f);
	glEnd ();

	gr_opengl_end_2d_matrix();
}

void gr_opengl_aaline(vertex *v1, vertex *v2)
{
#ifdef FRED_OGL_COMMENT_OUT_FOR_NOW
	if(Fred_running && !Cmdline_nohtl)
	{
		glBegin (GL_LINES);
			glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

			glVertex3f (v1->x, v1->y, v1->z);
			glVertex3f (v2->x, v2->y, v2->z);
		glEnd ();

		return;
	}
#endif

// -- AA OpenGL lines.  Looks good but they are kinda slow so this is disabled until an option is implemented - taylor
//	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
//	glEnable( GL_LINE_SMOOTH );
//	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
//	glLineWidth( 1.0 );

	int clipped = 0, swapped = 0;
	float x1 = v1->sx;
	float y1 = v1->sy;
	float x2 = v2->sx;
	float y2 = v2->sy;
	float sx1, sy1;
	float sx2, sy2;


	FL_CLIPLINE(x1, y1, x2, y2, (float)gr_screen.clip_left, (float)gr_screen.clip_top, (float)gr_screen.clip_right, (float)gr_screen.clip_bottom, return, clipped=1, swapped=1);

	sx1 = x1 + (float)gr_screen.offset_x;
	sy1 = y1 + (float)gr_screen.offset_y;
	sx2 = x2 + (float)gr_screen.offset_x;
	sy2 = y2 + (float)gr_screen.offset_y;


	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( (x1 == x2) && (y1 == y2) ) {
		gr_opengl_set_2d_matrix();

		glBegin (GL_POINTS);
			glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

			glVertex3f(sx1, sy1, -0.99f);
		glEnd ();

		gr_opengl_end_2d_matrix();

		return;
	}

	if ( x1 == x2 ) {
		if ( sy1 < sy2 )    {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if ( y1 == y2 )  {
		if ( sx1 < sx2 )    {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	gr_opengl_set_2d_matrix();

	glBegin (GL_LINES);
		glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		glVertex3f (sx2, sy2, -0.99f);
		glVertex3f (sx1, sy1, -0.99f);
	glEnd ();

	gr_opengl_end_2d_matrix();

//	glDisable( GL_LINE_SMOOTH );
}

void gr_opengl_gradient(int x1, int y1, int x2, int y2, bool resize)
{
	int clipped = 0, swapped=0;

	if ( !gr_screen.current_color.is_alphacolor )   {
		gr_line( x1, y1, x2, y2, resize );
		return;
	}

	if (resize || gr_screen.rendering_to_texture != -1) {
		gr_resize_screen_pos( &x1, &y1 );
		gr_resize_screen_pos( &x2, &y2 );
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	int aa = swapped ? 0 : gr_screen.current_color.alpha;
	int ba = swapped ? gr_screen.current_color.alpha : 0;
	
	float sx1, sy1, sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);

	if ( x1 == x2 ) {
		if ( sy1 < sy2 ) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if ( y1 == y2 ) {
		if ( sx1 < sx2 ) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	glBegin (GL_LINES);
		glColor4ub((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)ba);
		glVertex2f(sx2, sy2);

		glColor4ub((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)aa);
		glVertex2f(sx1, sy1);
	glEnd ();	
}

void gr_opengl_circle( int xc, int yc, int d, bool resize )
{
	int p,x, y, r;

	if (resize || gr_screen.rendering_to_texture != -1)
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
		gr_opengl_line( xc-y, yc-x, xc+y, yc-x, false );
		gr_opengl_line( xc-y, yc+x, xc+y, yc+x, false );
                                
		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_opengl_line( xc-x, yc-y, xc+x, yc-y, false );
			gr_opengl_line( xc-x, yc+y, xc+x, yc+y, false );
                                                
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y) {
		gr_opengl_line( xc-x, yc-y, xc+x, yc-y, false );
		gr_opengl_line( xc-x, yc+y, xc+x, yc+y, false );
	}
	return;
}

void gr_opengl_curve( int xc, int yc, int r, int direction)
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
				gr_opengl_line(xc - b + 1, yc-a, xc - b, yc-a, false);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_opengl_line(xc-a+1,yc-b,xc-a,yc-b, false);
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
				gr_opengl_line(xc + b - 1, yc-a, xc + b, yc-a, false);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_opengl_line(xc+a-1,yc-b,xc+a,yc-b, false);
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
				gr_opengl_line(xc - b + 1, yc+a, xc - b, yc+a, false);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_opengl_line(xc-a+1,yc+b,xc-a,yc+b, false);
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
				gr_opengl_line(xc + b - 1, yc+a, xc + b, yc+a, false);

				if (p<0) 
					p=p+(a<<2)+6;
				else	{
					// Draw the second octant
					gr_opengl_line(xc+a-1,yc+b,xc+a,yc+b, false);
					p=p+((a-b)<<2)+10;
					b--;
				}
				a++;
			}
			break;
	}
}

void gr_opengl_stuff_fog_coord(vertex *v)
{
	float d;
	vec3d pos;		// position of the vertex in question
	vec3d final;

	vm_vec_add(&pos, Interp_pos, &Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);
	vm_vec_rotate(&final, &pos, Interp_orient);

	d = vm_vec_dist_squared(&pos, &Eye_position);

	vglFogCoordfEXT(d);
}

/*void gr_opengl_stuff_secondary_color(vertex *v, ubyte fr, ubyte fg, ubyte fb)
{
	GLfloat color[3] = { 0.0f, 0.0f, 0.0f };

	// check for easy out first
	if ( (fr == 0) && (fg == 0) && (fb == 0) ) {
		vglSecondaryColor3fvEXT(color);
		return;
	}

	float d, d_over_far;
	vec3d pos;			// position of the vertex in question

	vm_vec_add(&pos, Interp_pos, &Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);

	d = vm_vec_dist_squared(&pos, &Eye_position);
	
	d_over_far = d / (gr_screen.fog_far * gr_screen.fog_far);

	if ( d_over_far <= (gr_screen.fog_near * gr_screen.fog_near) ) {
		vglSecondaryColor3fvEXT(color);
		return;
	}

	if ( (fr == 255) && (fg == 255) && (fb == 255) ) {
		color[0] = color[1] = color[2] = 1.0f;
	} else {
		color[0] = (GLfloat)fr / 255.0f;
		color[1] = (GLfloat)fg / 255.0f;
		color[2] = (GLfloat)fb / 255.0f;
	}

	if (d_over_far >= 1.0f) {
		// another easy out
		vglSecondaryColor3fvEXT(color);
		return;
	}

	color[0] *= d_over_far;
	color[1] *= d_over_far;
	color[2] *= d_over_far;

	vglSecondaryColor3fvEXT(color);
}*/

void opengl_draw_primitive(int nv, vertex **verts, uint flags, float u_scale, float v_scale, int r, int g, int b, int alpha, int override_primary = 0)
{
	GLenum gl_mode = GL_TRIANGLE_FAN;
	float sx, sy, sz, sw, tu, tv, rhw;
	int i, a;
	vertex *va;

	if (flags & TMAP_FLAG_TRISTRIP)
		gl_mode = GL_TRIANGLE_STRIP;
	else if (flags & TMAP_FLAG_TRILIST)
		gl_mode = GL_TRIANGLES;


	glBegin(gl_mode);

	for (i = nv-1; i >= 0; i--) {
		va = verts[i];

		sw = 1.0f;

		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )      {
			sz = float(1.0 - 1.0 / (1.0 + va->z / 32768.0 ));

			//if ( sz > 0.98f ) {
		//		sz = 0.98f;
		//	}
		} else {
			sz = 0.99f;
		}

		if ( flags & TMAP_FLAG_CORRECT ) {
			rhw = va->sw;
		} else {
			rhw = 1.0f;
		}
		
		if (flags & TMAP_FLAG_ALPHA) {
			a = verts[i]->a;
		} else {
			a = alpha;
		}

		if (flags & TMAP_FLAG_NEBULA ) {
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) ) {
			r = verts[i]->b;
			g = verts[i]->b;
			b = verts[i]->b;
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) ) {
			// Make 0.75 be 256.0f
			r = verts[i]->r;
			g = verts[i]->g;
			b = verts[i]->b;
		} else {
			// use constant RGB values...
		}

		if (!override_primary) {
			glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );
		} else {
			glColor3ub( va->spec_r, va->spec_g, va->spec_b );		
		}

		if ( (gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (OGL_fogmode == 2) ) {
			// this is for GL_EXT_FOG_COORD 
			gr_opengl_stuff_fog_coord(va);
		}

		sx = va->sx + (float)gr_screen.offset_x;
		sy = va->sy + (float)gr_screen.offset_y;

		if (rhw != 1.0f) {
			sx /= rhw;
			sy /= rhw;
			sz /= rhw;
			sw /= rhw;
		}

		if ( flags & TMAP_FLAG_TEXTURED ) {
			tu = va->u * u_scale;
			tv = va->v * v_scale;

			// use opengl hardware multitexturing
			vglMultiTexCoord2fARB(GL_TEXTURE0_ARB,tu,tv);
			vglMultiTexCoord2fARB(GL_TEXTURE1_ARB,tu,tv);
			
			if (GL_supported_texture_units > 2)
				vglMultiTexCoord2fARB(GL_TEXTURE2_ARB,tu,tv);	
		}

		glVertex4f(sx, sy, -sz, sw);
	}

	glEnd();
}

void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage )
{
	if ( !gr_opengl_tcache_set(SPECMAP, tmap_type, u_scale, v_scale, 0, 0, stage) )
		return;

	// render with spec lighting only
	opengl_default_light_settings(0, 0, 1);

	opengl_set_modulate_tex_env();

	opengl_set_state( GL_current_tex_src, ALPHA_BLEND_ADDITIVE, GL_current_ztype);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
}

void opengl_reset_spec_mapping()
{
	// reset lights to default values
	opengl_default_light_settings();

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );
}

void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler)
{
	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;
	
	if ( gr_zbuffering ) {
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) ) {
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	tmap_type = TCACHE_TYPE_NORMAL;

	if ( flags & TMAP_FLAG_TEXTURED ) {
		r = g = b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER ) {
		if ( (gr_screen.current_bitmap >= 0) && bm_has_alpha_channel(gr_screen.current_bitmap) ) {
			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor >= 1.0f ) {
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		} else {
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ADDITIVE;	// ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor < 1.0f ) {
				r = fl2i(r * gr_screen.current_alpha);
				g = fl2i(g * gr_screen.current_alpha);
				b = fl2i(b * gr_screen.current_alpha);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = fl2i(gr_screen.current_alpha * 255.0f);
	}

	
	if ( flags & TMAP_FLAG_TEXTURED ) {
		// use nonfiltered textures for interface graphics
		if (flags & TMAP_FLAG_INTERFACE) {
			tmap_type = TCACHE_TYPE_INTERFACE;
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	} else {
		texture_source = TEXTURE_SOURCE_NONE;
	}

	opengl_set_state( texture_source, alpha_blend, zbuffer_type );
}

void opengl_tmapper_internal( int nv, vertex **verts, uint flags, int is_scaler = 0 )
{
	int i, stage = 0;
	float u_scale = 1.0f, v_scale = 1.0f;
	bool use_spec = false;
	int alpha,tmap_type, r, g, b;


	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags, is_scaler);

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, 0, stage) ) {
			return;
		}
		stage++; // bump!

		// glowmap
		if ( (GLOWMAP > -1) && !Cmdline_noglow ) {
			if ( !gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, 0, 0, stage) ) {
				return;
			}
			opengl_set_additive_tex_env();
			stage++; // bump
		}

		if ( (lighting_is_enabled) && (SPECMAP > -1) && !Cmdline_nospec ) {
			use_spec = true;
			opengl_default_light_settings(1, 1, 0); // don't render with spec lighting here
		} else {
			// reset to defaults
			opengl_default_light_settings();
		}
	}
	
	
	if ( (flags & TMAP_FLAG_PIXEL_FOG) && (Neb2_render_mode != NEB2_RENDER_NONE) && (Neb2_render_mode != NEB2_RENDER_HTL) ) {
		int r, g, b;
		int ra, ga, ba;
		ra = ga = ba = 0;
	
		for (i = nv-1; i >= 0; i--) {
			vertex *va = verts[i];
			float sx, sy;
                
			if (gr_screen.offset_x || gr_screen.offset_y) {
				sx = ((va->sx * 16.0f) + ((float)gr_screen.offset_x * 16.0f)) / 16.0f;
				sy = ((va->sy * 16.0f) + ((float)gr_screen.offset_y * 16.0f)) / 16.0f;
			} else {
				sx = va->sx;
				sy = va->sy;
			}

			neb2_get_pixel((int)sx, (int)sy, &r, &g, &b);

			ra += r;
			ga += g;
			ba += b;
		}
		
		ra /= nv;
		ga /= nv;
		ba /= nv;

		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}

	gr_opengl_set_2d_matrix();

	opengl_draw_primitive(nv, verts, flags, u_scale, v_scale, r, g, b, alpha);

	if (ogl_maybe_pop_arb1) {
		gr_pop_texture_matrix(1);
		ogl_maybe_pop_arb1 = 0;
	}

	if ( use_spec ) {
		opengl_set_spec_mapping(tmap_type, &u_scale, &v_scale);
		opengl_draw_primitive(nv, verts, flags, u_scale, v_scale, r, g, b, alpha, 1);
		opengl_reset_spec_mapping();
	}

	gr_opengl_end_2d_matrix();
}

//ok we're making some assumptions for now.  mainly it must not be multitextured, lit or fogged.
void opengl_tmapper_internal3d( int nv, vertex **verts, uint flags )
{
	int i, alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	vertex *va;

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0) ) {
			return;
		}
	}

	int cull = gr_set_cull(0);

	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	if ( !(flags & (TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD)) )
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );

	if (flags & TMAP_FLAG_TRILIST)
		gl_mode = GL_TRIANGLES;
	else if (flags & TMAP_FLAG_TRISTRIP)
		gl_mode = GL_TRIANGLE_STRIP;


	glBegin(gl_mode);

	for (i=0; i < nv; i++) {
		va = verts[i];

		if ( flags & (TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD) )
			glColor4ub(va->r, va->g, va->b, (ubyte)alpha);

		glTexCoord2f(va->u, va->v);
		glVertex3f(va->x, va->y, va->z);
	}

	glEnd();

	gr_set_cull(cull);
}

void gr_opengl_tmapper( int nverts, vertex **verts, uint flags )
{
	if ((!Cmdline_nohtl) && (flags & TMAP_HTL_3D_UNLIT))
		opengl_tmapper_internal3d( nverts, verts, flags );
	else
		opengl_tmapper_internal( nverts, verts, flags );

}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_opengl_scaler(vertex *va, vertex *vb )
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
	v->spec_r=0;
	v->spec_g=0;
	v->spec_b=0;

	vl[1] = &v[1];	
	v[1].sx = clipped_x1;
	v[1].sy = clipped_y0;
	v[1].sw = va->sw;
	v[1].z = va->z;
	v[1].u = clipped_u1;
	v[1].v = clipped_v0;
	v[1].spec_r=0;
	v[1].spec_g=0;
	v[1].spec_b=0;

	vl[2] = &v[2];	
	v[2].sx = clipped_x1;
	v[2].sy = clipped_y1;
	v[2].sw = va->sw;
	v[2].z = va->z;
	v[2].u = clipped_u1;
	v[2].v = clipped_v1;
	v[2].spec_r=0;
	v[2].spec_g=0;
	v[2].spec_b=0;

	vl[3] = &v[3];	
	v[3].sx = clipped_x0;
	v[3].sy = clipped_y1;
	v[3].sw = va->sw;
	v[3].z = va->z;
	v[3].u = clipped_u0;
	v[3].v = clipped_v1;
	v[3].spec_r=0;
	v[3].spec_g=0;
	v[3].spec_b=0;

	opengl_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
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
	if ( Use_PBOs ) {
		vglGenBuffersARB(1, &pbo);

		if (!pbo) {
			if (fout != NULL)
				fclose(fout);

			return;
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
		vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, (gr_screen.max_w * gr_screen.max_h * 4), NULL, GL_STATIC_READ);

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		// map the image data so that we can save it to file
		pixels = (GLubyte*) vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
	} else {
		pixels = (GLubyte*) vm_malloc_q(gr_screen.max_w * gr_screen.max_h * 4);

		if (pixels == NULL) {
			if (fout != NULL)
				fclose(fout);

			return;
		}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
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

	if ( pbo ) {
		vglUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		pixels = NULL;
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		vglDeleteBuffersARB(1, &pbo);
	}

	// done!
	fclose(fout);

	if (pixels != NULL)
		vm_free(pixels);
}

int gr_opengl_supports_res_ingame(int res)
{
	STUB_FUNCTION;
	
	return 1;
}

int gr_opengl_supports_res_interface(int res)
{
	STUB_FUNCTION;
	
	return 1;
}

void gr_opengl_cleanup(int minimize)
{	
	if ( !GL_initted )
		return;

	if (!Fred_running) {
		gr_reset_clip();
		gr_clear();
		gr_flip();
		gr_clear();
	}

	GL_initted = 0;

	opengl_tcache_flush();

#ifdef _WIN32
	HWND wnd=(HWND)os_get_window();

	DBUGFILE_OUTPUT_0("");
	if (rend_context)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			DBUGFILE_OUTPUT_0("");
			MessageBox(wnd, "SHUTDOWN ERROR", "error", MB_OK);
		}
		if (!wglDeleteContext(rend_context))
		{
			DBUGFILE_OUTPUT_0("");
			MessageBox(wnd, "Unable to delete rendering context", "error", MB_OK);
		}
		rend_context=NULL;
	}
#endif

	DBUGFILE_OUTPUT_0("opengl_minimize");
	opengl_minimize();
	if (minimize)
	{
#ifdef _WIN32
		if (!Cmdline_window)
			ChangeDisplaySettings(NULL, 0);
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
		if (gr_screen.current_fog_mode != fog_mode) {
			glDisable(GL_FOG);
		}
		gr_screen.current_fog_mode = fog_mode;
		
		return;
	}
	
  	if (gr_screen.current_fog_mode != fog_mode) {
	  	if (OGL_fogmode==3) {
			glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
		}
		// Um.. this is not the correct way to fog in software, probably doesnt matter though
		else if (OGL_fogmode==2 && Cmdline_nohtl)
		{
			glFogf(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
			fog_near*=fog_near;		//its faster this way
			fog_far*=fog_far;		
		}

		glEnable(GL_FOG); 
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

void gr_opengl_get_pixel(int x, int y, int *r, int *g, int *b)
{
	// Not used.
}

static int CullEnabled = 0;
int gr_opengl_set_cull(int cull)
{
	int last_state = CullEnabled;

	if (cull) {
		glEnable (GL_CULL_FACE);
		glFrontFace (GL_CCW);
		glCullFace (GL_BACK);
		CullEnabled = 1;
	} else {
		glDisable (GL_CULL_FACE);
		CullEnabled = 0;
	}

	return last_state;
}

void gr_opengl_filter_set(int filter)
{
}

// cross fade
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
   	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f - pct );
	gr_bitmap(x1, y1);

  	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct );
	gr_bitmap(x2, y2);
}

void gr_opengl_set_clear_color(int r, int g, int b)
{
	gr_init_color (&gr_screen.current_clear_color, r, g, b);
}

void gr_opengl_shade(int x, int y, int w, int h, bool resize)
{
	if (resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	int x1 = (gr_screen.offset_x + x);
	int y1 = (gr_screen.offset_y + y);
	int x2 = (gr_screen.offset_x + w);
	int y2 = (gr_screen.offset_y + h);

	if ( (x1 >= gr_screen.max_w) || (y1 >= gr_screen.max_h) )
		return;

	CLAMP(x2, x1+1, gr_screen.max_w);
	CLAMP(y2, y1+1, gr_screen.max_h);

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	gr_opengl_set_2d_matrix();

	glColor4ub( (GLubyte)gr_screen.current_shader.r, (GLubyte)gr_screen.current_shader.g,
				(GLubyte)gr_screen.current_shader.b, (GLubyte)gr_screen.current_shader.c );

	glBegin(GL_QUADS);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
		glVertex2i(x1, y1);
	glEnd();

	gr_opengl_end_2d_matrix();
}

void gr_opengl_flash(int r, int g, int b)
{
	if ( !(r || g || b) )
		return;

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );

	int x1 = (gr_screen.clip_left+gr_screen.offset_x);
	int y1 = (gr_screen.clip_top+gr_screen.offset_y);
	int x2 = (gr_screen.clip_right+gr_screen.offset_x);
	int y2 = (gr_screen.clip_bottom+gr_screen.offset_y);

	glColor4ub( (GLubyte)r, (GLubyte)g, (GLubyte)b, 255 );

	glBegin(GL_QUADS);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
		glVertex2i(x1, y1);
	glEnd();
}

void gr_opengl_flash_alpha(int r, int g, int b, int a)
{
	if ( !(r || g || b || a) )
		return;

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(a, 0, 255);

	opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	int x1 = (gr_screen.clip_left+gr_screen.offset_x);
	int y1 = (gr_screen.clip_top+gr_screen.offset_y);
	int x2 = (gr_screen.clip_right+gr_screen.offset_x);
	int y2 = (gr_screen.clip_bottom+gr_screen.offset_y);

	glColor4ub( (GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)a );

	glBegin(GL_QUADS);
		glVertex2i(x1, y2);
		glVertex2i(x2, y2);
		glVertex2i(x2, y1);
		glVertex2i(x1, y1);
	glEnd();
}

int gr_opengl_zbuffer_get()
{
	if ( !gr_global_zbuffering )
		return GR_ZBUFF_NONE;

	return gr_zbuffering_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if (gr_zbuffering_mode == GR_ZBUFF_NONE ) {
		gr_zbuffering = 0;
		glDisable(GL_DEPTH_TEST);
	} else {
		gr_zbuffering = 1;
		glEnable(GL_DEPTH_TEST);
	}

	return tmp;
}

void gr_opengl_zbuffer_clear(int mode)
{
	if (mode) {
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
		glDisable(GL_DEPTH_TEST);
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
			goto Done;
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


Done:
	// Flush any existing textures
//	opengl_tcache_flush();
	return;
}

void gr_opengl_fade_in(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_fade_out(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_get_region(int front, int w, int h, ubyte *data)
{

//	if (front) {
//		glReadBuffer(GL_FRONT);
//	} else {
		glReadBuffer(GL_BACK);
//	}

	opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
	
	if (gr_screen.bits_per_pixel == 16) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
	} else if (gr_screen.bits_per_pixel == 32) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	}


}


#define MAX_MOUSE_SAVE_SIZE (32*32)

void opengl_save_mouse_area(int x, int y, int w, int h)
{
	int cursor_size;

	// lazy - taylor
	cursor_size = (GL_mouse_saved_size * GL_mouse_saved_size);

	// no reason to be bigger than the cursor, should never be smaller
	if (w != GL_mouse_saved_size)
		w = GL_mouse_saved_size;
	if (h != GL_mouse_saved_size)
		h = GL_mouse_saved_size;

	GL_mouse_saved_x1 = x;
	GL_mouse_saved_y1 = y;
	GL_mouse_saved_x2 = x+w-1;
	GL_mouse_saved_y2 = y+h-1;

	CLAMP(GL_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(GL_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(GL_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	Assert( (w * h) <= MAX_MOUSE_SAVE_SIZE );

	opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	if ( Use_PBOs ) {
		// since this is used a lot, and is pretty small in size, we just create it once and leave it until exit
		if (!GL_cursor_pbo) {
			vglGenBuffersARB(1, &GL_cursor_pbo);
			vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
			vglBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, cursor_size * 4, NULL, GL_STATIC_READ);
		}

		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_cursor_pbo);
		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		vglBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	} else {
		// this should really only have to be malloc'd once
		if (GL_saved_mouse_data == NULL)
			GL_saved_mouse_data = (ubyte*)vm_malloc_q(cursor_size * 4);

		if (GL_saved_mouse_data == NULL)
			return;

		glReadBuffer(GL_BACK);
		glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_saved_mouse_data);
	}

	GL_mouse_saved = 1;
}

void opengl_free_mouse_area()
{
	if (GL_saved_mouse_data != NULL) {
		vm_free(GL_saved_mouse_data);
		GL_saved_mouse_data = NULL;
	}
}

int gr_opengl_save_screen()
{
	int i;
	ubyte *sptr, *dptr;
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

	glDisable(GL_DEPTH_TEST);
	glReadBuffer(GL_FRONT_LEFT);

	if ( Use_PBOs ) {
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

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		GLubyte *pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (GL_mouse_saved_size * 4);

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

			GLubyte *pixels = (GLubyte*)vglMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

			sptr = (ubyte *)pixels;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < GL_mouse_saved_size; i++) {
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
	 		return -1;
	 	}

		glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, opengl_screen_tmp);

		sptr = (ubyte *)&opengl_screen_tmp[gr_screen.max_w * gr_screen.max_h * 4];
		dptr = (ubyte *)GL_saved_screen;

		width_times_pixel = (gr_screen.max_w * 4);
		mouse_times_pixel = (GL_mouse_saved_size * 4);

		for (i = 0; i < gr_screen.max_h; i++) {
			sptr -= width_times_pixel;
			memcpy(dptr, sptr, width_times_pixel);
			dptr += width_times_pixel;
		}

		vm_free(opengl_screen_tmp);

		if (GL_mouse_saved && GL_saved_mouse_data) {
			sptr = (ubyte *)GL_saved_mouse_data;
			dptr = (ubyte *)&GL_saved_screen[(GL_mouse_saved_x1 + GL_mouse_saved_y2 * gr_screen.max_w) * 4];

			for (i = 0; i < GL_mouse_saved_size; i++) {
				memcpy(dptr, sptr, mouse_times_pixel);
				sptr += mouse_times_pixel;
				dptr -= width_times_pixel;
			}
		}

		GL_saved_screen_id = bm_create(32, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);
	}

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

uint gr_opengl_lock()
{
	return 1;
}

void gr_opengl_unlock()
{
}

// RT Not sure if we should use opengl_zbias, untested so use stub for now
void gr_opengl_zbias_stub(int bias)
{
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


void opengl_zbias(int bias)
{
	if (bias) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0, -i2fl(bias));
	} else {
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void opengl_bitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, bool resize)
{
	if ( (w < 1) || (h < 1) )
		return;

	float u_scale, v_scale;
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	if ( !gr_opengl_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_INTERFACE, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		return;
	}

	opengl_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );


	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	u0 = u_scale*i2fl(sx)/i2fl(bw);
	v0 = v_scale*i2fl(sy)/i2fl(bh);

	u1 = u_scale*i2fl(sx+w)/i2fl(bw);
	v1 = v_scale*i2fl(sy+h)/i2fl(bh);

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if ( do_resize ) {
		gr_resize_screen_posf( &x1, &y1 );
		gr_resize_screen_posf( &x2, &y2 );
	}

	glColor4f( 1.0f, 1.0f, 1.0f, gr_screen.current_alpha );

	glBegin(GL_QUADS);
		glTexCoord2f(u0, v1);
		glVertex2f(x1, y2);

		glTexCoord2f(u1, v1);
		glVertex2f(x2, y2);

		glTexCoord2f(u1, v0);
		glVertex2f(x2, y1);

		glTexCoord2f(u0, v0);
		glVertex2f(x1, y1);
	glEnd ();
}


//these are penguins bitmap functions
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh, do_resize;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ( (dx1 > clip_right) || (dx2 < clip_left) ) return;
		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) return;
		if ( dx1 < clip_left ) { sx += clip_left-dx1; dx1 = clip_left; }
		if ( dy1 < clip_top ) { sy += clip_top-dx1; dy1 = clip_top; }
		if ( dx2 > clip_right ) { dx2 = clip_right; }
		if ( dy2 > clip_bottom ) { dy2 = clip_bottom; }

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
		Assert( (dx1 >= clip_left ) && (dx1 <= clip_right) );
		Assert( (dx2 >= clip_left ) && (dx2 <= clip_right) );
		Assert( (dy1 >= clip_top ) && (dy1 <= clip_bottom) );
		Assert( (dy2 >= clip_top ) && (dy2 <= clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	opengl_bitmap_ex_internal(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize);
}

/*void gr_opengl_bitmap(int x, int y, bool resize)
{
	int w, h, do_resize;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );

	if ( (gr_screen.custom_size != -1) && (resize || gr_screen.rendering_to_texture != -1) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	if ( (dx1 > clip_right) || (dx2 < clip_left) ) return;
	if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) return;
	if ( dx1 < clip_left ) { sx = clip_left-dx1; dx1 = clip_left; }
	if ( dy1 < clip_top ) { sy = clip_top-dy1; dy1 = clip_top; }
	if ( dx2 > clip_right )	{ dx2 = clip_right; }
	if ( dy2 > clip_bottom ) { dy2 = clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_opengl_bitmap_ex(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize);
}*/

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

void opengl_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static float pre_set_colours[MAX_NUM_TIMERBARS][3] = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.2f, 1.0f, 0.8f }, 
		{ 1.0f, 0.0f, 8.0f }, 
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.2f, 0.2f },
		{ 1.0f, 1.0f, 1.0f }
	};

	static float max_fw = (float) gr_screen.max_w; 
	static float max_fh = (float) gr_screen.max_h; 

	x *= max_fw;
	y *= max_fh;
	w *= max_fw;
	h *= max_fh;

	y += 5;

	opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	glColor3fv(pre_set_colours[colour]);

	glBegin(GL_QUADS);
		glVertex2f(x,y);
		glVertex2f(x,y+h);
		glVertex2f(x+w,y+h);
		glVertex2f(x+w,y);
	glEnd();
}

void gr_opengl_setup_background_fog(bool set)
{
	if (Cmdline_nohtl)
		return;

}

void gr_opengl_draw_line_list(colored_vector *lines, int num)
{
	if (Cmdline_nohtl)
		return;
}

// Returns the human readable error string if there is an error or NULL if not
const char *opengl_error_string()
{
	GLenum error = GL_NO_ERROR;

	error = glGetError();

	if ( error != GL_NO_ERROR )
		return (const char *)gluErrorString(error);

	return NULL;
}

int opengl_check_for_errors()
{
	const char *error_str = NULL;
	int num_errors = 0;

	do {
		error_str = opengl_error_string();

		if (error_str) {
			nprintf(("OpenGL", "OpenGL Error: %s\n", error_str));
			num_errors++;
		}
	} while ((error_str != NULL) && !Fred_running);

	return num_errors;
}

void opengl_set_vsync(int status)
{
	Assert( (status == 0) || (status == 1) );

	if ( (status < 0) || (status > 1) )
		return;

#if defined(__APPLE__)
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, (long*)&status);
#elif defined(_WIN32)
	vwglSwapIntervalEXT(status);
#else
	// NOTE: this may not work well with the closed NVIDIA drivers since those use the
	//       special "__GL_SYNC_TO_VBLANK" environment variable to manage sync
	vglXSwapIntervalSGI(status);
#endif

	opengl_check_for_errors();
}

// NOTE: This should only ever be called through os_cleanup(), or when switching video APIs
void gr_opengl_shutdown()
{
#ifdef _WIN32
	// restore original gamma settings
	if (GL_original_gamma_ramp != NULL)
		SetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );

	// swap out our window mode and un-jail the cursor
	ShowWindow((HWND)os_get_window(), SW_HIDE);
	ClipCursor(NULL);
	ChangeDisplaySettings( NULL, 0 );
#else
	if (GL_original_gamma_ramp != NULL)
		SDL_SetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
#endif

	if (GL_original_gamma_ramp != NULL) {
		vm_free(GL_original_gamma_ramp);
		GL_original_gamma_ramp = NULL;
	}
}

// NOTE: This should only ever be called through atexit()!!!
void opengl_close()
{
//	if (!GL_initted)
//		return;

	if (currently_enabled_lights != NULL) {
		vm_free(currently_enabled_lights);
		currently_enabled_lights = NULL;
	}

	if (opengl_lights != NULL) {
		vm_free(opengl_lights);
		opengl_lights = NULL;
	}

	if (GL_cursor_pbo) {
		vglDeleteBuffersARB(1, &GL_cursor_pbo);
	}

	opengl_free_mouse_area();

	opengl_tcache_cleanup();

#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);

	if (rend_context) {
		wglDeleteContext(rend_context);
		rend_context=NULL;
	}
#endif

	GL_initted = 0;
}

int opengl_init_display_device()
{
	int bpp = gr_screen.bits_per_pixel;

	Assert( (bpp == 16) || (bpp == 32) );

	if ( (bpp != 16) && (bpp != 32) )
		return 1;


	// screen format
	switch ( bpp ) {
		case 16:
		{
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

		case 32:
		{
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

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset(&pfd_test, 0, sizeof(PIXELFORMATDESCRIPTOR));
	
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = (ubyte)bpp;
	pfd.cRedBits = (ubyte)Gr_red.bits;
	pfd.cGreenBits = (ubyte)Gr_green.bits;
	pfd.cBlueBits = (ubyte)Gr_blue.bits;
	pfd.cAlphaBits = (bpp == 32) ? (ubyte)Gr_alpha.bits : 0;
	pfd.cDepthBits = (bpp == 32) ? 24 : 16;


	wnd = (HWND)os_get_window();

	Assert( wnd != NULL );

	GL_device_context = GetDC(wnd);

	if (!GL_device_context) {
		MessageBox(wnd, "Unable to get Device context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	PixelFormat = ChoosePixelFormat(GL_device_context, &pfd);

	if (!PixelFormat) {
		MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
		return 1;
	} else {
		DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_test);

		// make sure that we are hardware accelerated and not using the generic implementation
		if ( !Fred_running && (pfd_test.dwFlags & PFD_GENERIC_FORMAT) && !(pfd_test.dwFlags & PFD_GENERIC_ACCELERATED) ) {
			Assert( bpp == 32 );

			// if we failed at 32-bit then we are probably a 16-bit desktop, so try and init a 16-bit visual instead
			pfd.cAlphaBits = 0;
			pfd.cDepthBits = 16;
			// NOTE: the bit values for colors should get updated automatically by ChoosePixelFormat()

			PixelFormat = ChoosePixelFormat(GL_device_context, &pfd);

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

	if (!SetPixelFormat(GL_device_context, PixelFormat, &pfd)) {
		MessageBox(wnd, "Unable to set pixel format for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	rend_context = wglCreateContext(GL_device_context);

	if (!rend_context) {
		MessageBox(wnd, "Unable to create rendering context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	if (!wglMakeCurrent(GL_device_context, rend_context)) {
		MessageBox(wnd, "Unable to make current thread for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		return 1;
	}

	mprintf(("  Requested WGL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", Gr_red.bits, Gr_green.bits, Gr_blue.bits, pfd.cDepthBits, (pfd.dwFlags & PFD_DOUBLEBUFFER) > 0));

	// now report back as to what we ended up getting
	int r = 0, g = 0, b = 0, depth = 0, db = 1;

	DescribePixelFormat(GL_device_context, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	r = pfd.cRedBits;
	g = pfd.cGreenBits;
	b = pfd.cBlueBits;
	depth = pfd.cDepthBits;
	db = ((pfd.dwFlags & PFD_DOUBLEBUFFER) > 0);

	mprintf(("  Actual WGL Video values    = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", r, g, b, depth, db));

	// get the default gamma ramp so that we can restore it on close
	if (GL_original_gamma_ramp != NULL)
		GetDeviceGammaRamp( GL_device_context, GL_original_gamma_ramp );

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

	if (GL_original_gamma_ramp != NULL)
		SDL_GetGammaRamp( GL_original_gamma_ramp, (GL_original_gamma_ramp+256), (GL_original_gamma_ramp+512) );
#endif

	return 0;
}


void opengl_setup_function_pointers()
{
	// *****************************************************************************
	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.

	gr_screen.gf_flip				= gr_opengl_flip;
	gr_screen.gf_flip_window		= gr_opengl_flip_window;
	gr_screen.gf_set_clip			= gr_opengl_set_clip;
	gr_screen.gf_reset_clip			= gr_opengl_reset_clip;
	
	gr_screen.gf_set_bitmap			= gr_opengl_set_bitmap;
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
	
	gr_screen.gf_lock				= gr_opengl_lock;
	gr_screen.gf_unlock				= gr_opengl_unlock;
	
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

	gr_screen.gf_get_pixel			= gr_opengl_get_pixel;

	gr_screen.gf_set_cull			= gr_opengl_set_cull;

	gr_screen.gf_cross_fade			= gr_opengl_cross_fade;

	gr_screen.gf_filter_set			= gr_opengl_filter_set;

	gr_screen.gf_tcache_set			= gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color	= gr_opengl_set_clear_color;

	gr_screen.gf_preload			= gr_opengl_preload;

	gr_screen.gf_push_texture_matrix		= gr_opengl_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix			= gr_opengl_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix	= gr_opengl_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing	= gr_opengl_set_texture_addressing;
	gr_screen.gf_zbias					= gr_opengl_zbias_stub;
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

	gr_screen.gf_draw_htl_line		= gr_opengl_draw_htl_line;
	gr_screen.gf_draw_htl_sphere	= gr_opengl_draw_htl_sphere;

	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.
	// *****************************************************************************
}


void gr_opengl_init(int reinit)
{
	char *ver;
	int major = 0, minor = 0;

	if ( !GL_initted ) {
		atexit(opengl_close);
	}

	if ( GL_initted )	{
		gr_opengl_cleanup();
		GL_initted = 0;
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

	GL_initted = 1;
	// set global too
	OGL_enabled = 1;

	// this MUST be done before any other gr_opengl_* or opengl_* funcion calls!!
	opengl_setup_function_pointers();

	if (!reinit) {
		mprintf(( "  OpenGL Vendor     : %s\n", glGetString( GL_VENDOR ) ));
		mprintf(( "  OpenGL Renderer   : %s\n", glGetString( GL_RENDERER ) ));
		mprintf(( "  OpenGL Version    : %s\n", ver ));
		mprintf(( "\n" ));

		// initialize the extensions and make sure we aren't missing something that we need
		opengl_get_extensions();

		// ready the texture system
		opengl_tcache_init();

		if (Cmdline_window) {
			opengl_go_windowed();
		} else {
			opengl_go_fullscreen();
		}
	}

	// must be called after extensions are setup
	opengl_set_vsync( !Cmdline_no_vsync );

	opengl_switch_arb(-1, 0);
	opengl_set_texture_target();
	opengl_switch_arb(0, 1);

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

	// if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Use_compressed_textures = Is_Extension_Enabled(OGL_EXT_TEXTURE_COMPRESSION_S3TC);
	Texture_compression_available = Is_Extension_Enabled(OGL_ARB_TEXTURE_COMPRESSION);

	//allow VBOs to be used
	if ( !Cmdline_nohtl && !Cmdline_novbo && Is_Extension_Enabled(OGL_ARB_VERTEX_BUFFER_OBJECT) )
		VBO_ENABLED = 1;

	if ( Is_Extension_Enabled(OGL_ARB_PIXEL_BUFFER_OBJECT) )
		Use_PBOs = 1;

	// setup the best fog function found
	// start with NV Radial Fog (GeForces only)  -- needs t&l, will have to wait
	/*if ( Is_Extension_Enabled(GL_NV_RADIAL_FOG) && !Fred_running ) {
		OGL_fogmode = 3;
	} else*/ if ( Is_Extension_Enabled(OGL_EXT_FOG_COORD) && !Fred_running ) {
		OGL_fogmode = 2;
	} else if ( !Fred_running ) {
		OGL_fogmode = 1;
	}

	// if we can't do cubemaps then turn off Cmdline_env
	if ( Cmdline_env && !Is_Extension_Enabled(OGL_ARB_TEXTURE_CUBE_MAP) )
		Cmdline_env = 0;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &GL_supported_texture_units);
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &GL_max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &GL_max_elements_indices);

//	if ( Is_Extension_Enabled(OGL_ARB_TEXTURE_ENV_COMBINE) ) {
//		opengl_set_tex_src = opengl_set_tex_state_combine;
//	} else {
		opengl_set_tex_src = opengl_set_tex_state_no_combine;
//	}

	// setup the lighting stuff that will get used later
	opengl_init_light();

	glDisable(GL_LIGHTING); // just making sure of it

	mprintf(( "  Max texture units: %i\n", GL_supported_texture_units ));
	mprintf(( "  Max elements vertices: %i\n", GL_max_elements_vertices ));
	mprintf(( "  Max elements indices: %i\n", GL_max_elements_indices ));
	mprintf(( "  Max texture size: %ix%i\n", GL_max_texture_width, GL_max_texture_height ));
	mprintf(( "  Can use compressed textures: %s\n", Use_compressed_textures ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Texture compression available: %s\n", Texture_compression_available ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Using %s texture filter.\n", (GL_mipmap_filter) ? NOX("trilinear") : NOX("bilinear") ));

	// This stops fred crashing if no textures are set
	gr_screen.current_bitmap = -1;

	TIMERBAR_SET_DRAW_FUNC(opengl_render_timer_bar);

	mprintf(("... OpenGL init is complete!\n"));
}

DCF(ogl_minimize, "Minimizes opengl")
{
	if ( gr_screen.mode != GR_OPENGL ) {
		dc_printf("Command only available in OpenGL mode.\n");
		return;
	}

	if (Dc_command) {
		dc_get_arg(ARG_TRUE);

		if ( Dc_arg_type & ARG_TRUE )
			opengl_minimize();
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
		dc_get_arg(ARG_FLOAT | ARG_NONE);

		if ( Dc_arg_type & ARG_NONE ) {
			GL_anisotropy = 0.0f;
			opengl_set_anisotropy();
			dc_printf("Anisotropic filter value reset to default level.\n");
		}

		if ( Dc_arg_type & ARG_FLOAT ) {
			opengl_set_anisotropy(Dc_arg_float);
		}
	}

	if ( Dc_status ) {
		dc_printf("Current anisotropic filter value is %f\n", GL_anisotropy);
	}

	if (Dc_help) {
		dc_printf("Sets OpenGL anisotropic filtering level.\n");
		dc_printf("Valid values are 1.0 to %f.\n", (float)opengl_get_max_anisotropy());
	}
}
