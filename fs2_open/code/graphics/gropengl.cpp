

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.cpp $
 * $Revision: 2.133 $
 * $Date: 2005-08-29 02:20:56 $
 * $Author: phreak $
 *
 * Code that uses the OpenGL graphics library
 *
 * $Log: not supported by cvs2svn $
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
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
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

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif


#include "globalincs/pstypes.h"
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


#if defined(_WIN32) && !defined(__GNUC__)
#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")
#pragma comment (lib, "glaux")
#endif


#define REQUIRED_GL_MAJOR_VERSION	1
#ifdef GL_NO_HTL
#define REQUIRED_GL_MINOR_VERSION	1
#else
#define REQUIRED_GL_MINOR_VERSION	2
#endif

extern int Cmdline_nohtl;

int vram_full = 0;			// UnknownPlayer

//0==no fog
//1==linear
//2==fog coord EXT
//3==NV Radial
static int OGL_fogmode=0;

#ifdef _WIN32
static HDC dev_context = NULL;
static HGLRC rend_context = NULL;
static PIXELFORMATDESCRIPTOR pfd;
static WORD original_gamma_ramp[3][256];
#endif

int VBO_ENABLED = 0;

extern int Texture_compression_enabled;

static int GL_dump_frames = 0;
static ubyte *GL_dump_buffer = NULL;
static int GL_dump_frame_number = 0;
static int GL_dump_frame_count = 0;
static int GL_dump_frame_count_max = 0;
static int GL_dump_frame_size = 0;
                        
volatile int GL_activate = 0;
volatile int GL_deactivate = 0;

static ubyte *GL_saved_screen = NULL;
static ubyte *GL_saved_mouse_data = NULL;
static int GL_mouse_saved_size = 32; // width and height, so they're equal

static int Gr_opengl_mouse_saved = 0;
static int Gr_opengl_mouse_saved_x1 = 0;
static int Gr_opengl_mouse_saved_y1 = 0;
static int Gr_opengl_mouse_saved_x2 = 0;
static int Gr_opengl_mouse_saved_y2 = 0;

extern int Cmdline_window;
extern int Interp_multitex_cloakmap;
extern int CLOAKMAP;
extern int SPECMAP;
extern int Interp_cloakmap_alpha;

static const char* OGL_extensions;

static float max_aniso=1.0f;			//max anisotropic filtering ratio

static float GL_uv_resize_offset_u = 0.0f;
static float GL_uv_resize_offset_v = 0.0f;

void (*gr_opengl_set_tex_src)(gr_texture_source ts);

extern void opengl_default_light_settings(int amb = 1, int emi = 1, int spec = 1);

extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

//some globals
extern matrix View_matrix;
extern vec3d View_position;
extern matrix Eye_matrix;
extern vec3d Eye_position;
extern vec3d Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;


void opengl_go_fullscreen(HWND wnd)
{
	if (Cmdline_window)
		return;

#ifdef _WIN32
	DEVMODE dm;

	os_suspend();
	SetWindowLong( wnd, GWL_EXSTYLE, 0 );
	SetWindowLong( wnd, GWL_STYLE, WS_POPUP );
	ShowWindow(wnd, SW_SHOWNORMAL );
	SetWindowPos( wnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
	SetActiveWindow(wnd);
	SetForegroundWindow(wnd);

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
	} else {
		// REMOVEME: doesn't really need to be here but for debugging purposes it
		//           could be helpful during the immediate future.
		mprintf(("USING REFRESH_RATE OF: %i\n", dm.dmDisplayFrequency));
	}

	os_resume();  
#else
	if ( (os_config_read_uint(NULL, NOX("Fullscreen"), 1) == 1) && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) ) {
		os_suspend();
		SDL_WM_ToggleFullScreen( SDL_GetVideoSurface() );
		os_resume();
	}
#endif
}

void opengl_minimize()
{
	mprintf(("opengl_minimize\n"));
	
#ifdef _WIN32
	HWND wnd=(HWND)os_get_window();

//	os_suspend();
	ShowWindow(wnd, SW_MINIMIZE);
	ChangeDisplaySettings(NULL,0);
//	os_resume();
#else
	// lets not minimize if we are in windowed mode
	if (!(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN))
		return;

	os_suspend();
	SDL_WM_IconifyWindow();
	os_resume();
#endif
}

/*
static inline void opengl_set_max_anistropy()
{
//	if (GL_Extensions[GL_TEX_FILTER_aniso].enabled)		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
}
*/
gr_texture_source	GL_current_tex_src=TEXTURE_SOURCE_NONE;
gr_alpha_blend		GL_current_alpha_blend=ALPHA_BLEND_NONE;
gr_zbuffer_type		GL_current_ztype=ZBUFFER_TYPE_NONE;

void gr_opengl_set_tex_state_combine_arb(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb(0,0);
			opengl_switch_arb(1,0);
			opengl_switch_arb(2,0);

			gr_tcache_set(-1, -1, NULL, NULL );

			GL_current_tex_src = ts;
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);

			GL_current_tex_src = ts;
			//	opengl_set_max_anistropy();		
		break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb(0,1);
			if(gr_screen.custom_size < 0) {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			} else {
				// If we are using a non standard mode we will need this because textures are being stretched
 				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);

			GL_current_tex_src = ts;
			break;
		default:
			return;
	}
}

void gr_opengl_set_tex_state_combine_ext(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb(0,0);
			opengl_switch_arb(1,0);
			opengl_switch_arb(2,0);
	
			gr_tcache_set(-1, -1, NULL, NULL );

			GL_current_tex_src = ts;
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
				
			GL_current_tex_src = ts;
			//	opengl_set_max_anistropy();		
		break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb(0,1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);

			GL_current_tex_src = ts;
			break;
		default:
			return;
	}
}


void gr_opengl_set_tex_state_no_combine(gr_texture_source ts)
{
	switch (ts) {
		case TEXTURE_SOURCE_NONE:
		
			//glBindTexture(GL_TEXTURE_2D, 0);
			opengl_switch_arb(0,0);
			opengl_switch_arb(1,0);
			opengl_switch_arb(2,0);
	
			gr_tcache_set(-1, -1, NULL, NULL );

			GL_current_tex_src = ts;
			break;
		
		case TEXTURE_SOURCE_DECAL:
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			GL_current_tex_src = ts;

		//	opengl_set_max_anistropy();
			break;
		
		case TEXTURE_SOURCE_NO_FILTERING:
			opengl_switch_arb(0,1);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			GL_current_tex_src = ts;

			break;
		default:
			return;
	}
	GL_current_tex_src=ts;
}

void gr_opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt)
{

	gr_opengl_set_tex_src(ts);


	switch (ab) {
		case ALPHA_BLEND_NONE:
			glBlendFunc(GL_ONE, GL_ZERO);
			GL_current_alpha_blend = ab;
			break;

		case ALPHA_BLEND_ALPHA_ADDITIVE:
			glBlendFunc(GL_ONE, GL_ONE);
			GL_current_alpha_blend = ab;
			break;

		case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			GL_current_alpha_blend = ab;
			break;
		
		case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
			GL_current_alpha_blend = ab;
			break;
	
		default:
			break;
	}
	
	switch (zt) {
		case ZBUFFER_TYPE_NONE:
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_FALSE);
			GL_current_ztype = zt;
			break;

		case ZBUFFER_TYPE_READ:
			glDepthFunc(GL_LESS);
			glDepthMask(GL_FALSE);
			GL_current_ztype = zt;
			break;

		case ZBUFFER_TYPE_WRITE:
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_TRUE);
			GL_current_ztype = zt;
			break;
	
		case ZBUFFER_TYPE_FULL:
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			GL_current_ztype = zt;
			break;
	
		default:
			break;
		}	
}

void gr_opengl_activate(int active)
{
	HWND wnd=(HWND)os_get_window();
	if (active) {
		GL_activate++;
		opengl_go_fullscreen(wnd);

#ifdef SCP_UNIX
		// Check again and if we didn't go fullscreen turn on grabbing if possible
		if(!Cmdline_no_grab && !(SDL_GetVideoSurface()->flags & SDL_FULLSCREEN)) {
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
#endif
	} else {
		GL_deactivate++;
		opengl_minimize();

#ifdef SCP_UNIX
		// let go of mouse/keyboard
		if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
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

void gr_opengl_save_mouse_area(int x, int y, int w, int h);
void opengl_tcache_frame ();
void gr_opengl_flip()
{
	if (!OGL_enabled) return;

	gr_reset_clip();

	mouse_eval_deltas();
	
	Gr_opengl_mouse_saved = 0;
	
	if ( mouse_is_visible() ) {
		int mx, my;

		gr_reset_clip();
		mouse_get_pos( &mx, &my );

		gr_opengl_save_mouse_area(mx, my, GL_mouse_saved_size, GL_mouse_saved_size);

		if ( Gr_cursor != -1 ) {
			gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my, false);
		}
	}

	TIMERBAR_END_FRAME();
	TIMERBAR_START_FRAME();

#ifdef _WIN32
	int swap_error=0;

	if(!SwapBuffers(dev_context))
	{
		opengl_minimize();
		swap_error=GetLastError();
		Error(LOCATION,"Unable to swap buffers\nError code: %d",swap_error);
		exit(2);
	}
#else
	SDL_GL_SwapBuffers();
#endif

	opengl_tcache_frame();

	if ( GL_activate )      {
		GL_activate = 0;
		opengl_tcache_flush();
		// gr_opengl_clip_cursor(1); // mouse grab, see opengl_activate
	}
	
	if ( GL_deactivate )      {
		GL_deactivate = 0;
		// gr_opengl_clip_cursor(0);  // mouse grab, see opengl_activate
	}

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

void gr_opengl_set_clip(int x,int y,int w,int h, bool resize)
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

	glDisable(GL_SCISSOR_TEST);
}

void gr_opengl_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;

	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

void gr_opengl_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;	
}

void gr_opengl_set_shader( shader * shade )
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
//WMC - removed for gr_rect in 2d.cpp
/*
void gr_opengl_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int saved_zbuf;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	memset(v,0,sizeof(vertex)*4);
	saved_zbuf = gr_zbuffer_get();
	
	// start the frame, no zbuffering, no culling
	if (!Fred_running)
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
	g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA, 0.1f);		

	if (!Fred_running)
		g3_end_frame();


	// restore zbuffer and culling
	gr_zbuffer_set(saved_zbuf);
	gr_set_cull(1);	
}

void gr_opengl_rect(int x,int y,int w,int h,bool resize)
{
	gr_opengl_rect_internal(x, y, w, h, gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
}
*/
/*
void gr_opengl_shade(int x,int y,int w,int h)
{
	int r,g,b,a;

	r = fl2i(gr_screen.current_shader.r);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;

	g3_draw_2d_rect(x,y,w,h,r,g,b,a);
}
*/

void gr_opengl_aabitmap_ex_internal(int x,int y,int w,int h,int sx,int sy,bool resize)
{
//	mprintf(("gr_opengl_aabitmap_ex_internal: at (%3d,%3d) size (%3d,%3d) name %s\n", 
  //				x, y, w, h, 
 	//			bm_get_filename(gr_screen.current_bitmap)));
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	float u_scale, v_scale;

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		mprintf(( "WARNING: Error setting aabitmap texture!\n" ));
		return;
	}

	gr_opengl_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize ;

	if ( (gr_screen.custom_size != -1) && (resize || (gr_screen.rendering_to_texture != -1)) ) {
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
	x2 = i2fl(x + w + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y2 = i2fl(y + h + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));

	if ( do_resize ) {
		gr_resize_screen_posf( &x1, &y1 );
		gr_resize_screen_posf( &x2, &y2 );
		u1 -= GL_uv_resize_offset_u;
		v1 -= GL_uv_resize_offset_v;
	}

	if ( gr_screen.current_color.is_alphacolor )	{
		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue,gr_screen.current_color.alpha);
	} else {
		glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

	glSecondaryColor3ubvEXT(GL_zero_3ub);

	glBegin (GL_QUADS);
	  glTexCoord2f (u0, v1);
	  glVertex3f (x1, y2, -0.99f);

	  glTexCoord2f (u1, v1);
	  glVertex3f (x2, y2, -0.99f);

	  glTexCoord2f (u1, v0);
	  glVertex3f (x2, y1, -0.99f);

	  glTexCoord2f (u0, v0);
	  glVertex3f (x1, y1, -0.99f);
	glEnd ();

}

void gr_opengl_aabitmap_ex(int x,int y,int w,int h,int sx,int sy,bool resize)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	int clip_left = ((resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

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
	gr_opengl_aabitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy,resize);
}

void gr_opengl_aabitmap(int x, int y, bool resize)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	int clip_left = ((resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

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
	gr_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy,resize);
}


void gr_opengl_string( int sx, int sy, char *s, bool resize = true )
{
	TIMERBAR_PUSH(4);

	int width, spacing, letter;
	int x, y;

	if ( !Current_font || (*s == 0) )	{
		return;
	}

	int clip_left = ((resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	gr_set_bitmap(Current_font->bitmap_id);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}
	
	spacing = 0;

	while (*s)	{
		x += spacing;

		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//not in font, draw as space
		if (letter<0)	{
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

		gr_opengl_aabitmap_ex_internal( xc, yc, wc, hc, u+xd, v+yd, resize);
	}

	TIMERBAR_POP();
}

void gr_opengl_line(int x1,int y1,int x2,int y2, bool resize)
{
	if(resize || gr_screen.rendering_to_texture != -1)
	{
		gr_resize_screen_pos(&x1, &y1);
		gr_resize_screen_pos(&x2, &y2);
	}

	int clipped = 0, swapped=0;

	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);
	
	float sx1, sy1;
	float sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);
	
	if ( x1 == x2 && y1 == y2 ) {
		gr_opengl_set_2d_matrix();

		glBegin (GL_POINTS);
		  glColor4ub (gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
		 
		  glSecondaryColor3ubvEXT(GL_zero_3ub);

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

		glSecondaryColor3ubvEXT(GL_zero_3ub);

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
	  
			glSecondaryColor3ubvEXT(GL_zero_3ub);

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

	gr_opengl_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy), false );

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

	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	int aa = swapped ? 0 : gr_screen.current_color.alpha;
	int ba = swapped ? gr_screen.current_color.alpha : 0;
	
	float sx1, sy1;
	float sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);
	
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

	glSecondaryColor3ubvEXT(GL_zero_3ub);
	
	glBegin (GL_LINES);
	  glColor4ub ((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)ba);
	  glVertex3f (sx2, sy2, -0.99f);
	  glColor4ub ((ubyte)gr_screen.current_color.red, (ubyte)gr_screen.current_color.green, (ubyte)gr_screen.current_color.blue, (ubyte)aa);
	  glVertex3f (sx1, sy1, -0.99f);
	glEnd ();	
}

void gr_opengl_circle( int xc, int yc, int d, bool resize )
{
	int p,x, y, r;

	if(resize || gr_screen.rendering_to_texture != -1)
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


extern vec3d *Interp_pos;
extern vec3d Interp_offset;
extern matrix *Interp_orient;

void gr_opengl_stuff_fog_coord(vertex *v)
{
	float d;
	vec3d pos;			//position of the vertex in question
	vec3d final;
	vm_vec_add(&pos, Interp_pos,&Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);
	vm_vec_rotate(&final, &pos, Interp_orient);
	d=vm_vec_dist_squared(&pos,&Eye_position);
	glFogCoordfEXT(d);
}

void gr_opengl_stuff_secondary_color(vertex *v, ubyte fr, ubyte fg, ubyte fb)
{

	float color[]={	(float)fr/255.0f,
					(float)fg/255.0f,
					(float)fb/255.0f};

	if ((fr==0) && (fg==0) && (fb==0))
	{
		//easy out
		glSecondaryColor3fvEXT(color);
		return;
	}

	float d;
	float d_over_far;
	vec3d pos;			//position of the vertex in question
	vm_vec_add(&pos, Interp_pos,&Interp_offset);
	vm_vec_add2(&pos, &v->real_pos);
	d=vm_vec_dist_squared(&pos,&Eye_position);
	
	d_over_far = d/(gr_screen.fog_far*gr_screen.fog_far);

	if (d_over_far <= (gr_screen.fog_near * gr_screen.fog_near))
	{
		memset(color,0,sizeof(float)*3);
		glSecondaryColor3fvEXT(color);
		return;
	}

	if (d_over_far >= 1.0f)
	{
		//another easy out
		glSecondaryColor3fvEXT(color);
		return;
	}

	color[0]*=d_over_far;
	color[1]*=d_over_far;
	color[2]*=d_over_far;

	glSecondaryColor3fvEXT(color);
	return;
}

static int ogl_maybe_pop_arb1=0;

void opengl_draw_primitive(int nv, vertex ** verts, uint flags, float u_scale, float v_scale, int r, int g, int b, int alpha, int override_primary=0)
{

	if (flags & TMAP_FLAG_TRISTRIP) 
		glBegin(GL_TRIANGLE_STRIP);
	else if (flags & TMAP_FLAG_TRILIST)
		glBegin(GL_TRIANGLES);
	else 
		glBegin(GL_TRIANGLE_FAN);

	for (int i = nv-1; i >= 0; i--) {		
		vertex * va = verts[i];
		float sx, sy, sz;
		float tu, tv;
		float rhw;
		int a;
		
		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )      {
			sz = float(1.0 - 1.0 / (1.0 + va->z / 32768.0 ));
			
			//if ( sz > 0.98f ) {
		//		sz = 0.98f;
		//	}
		} else {
			sz = 0.99f;
		}

		if ( flags & TMAP_FLAG_CORRECT )        {
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
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )   {
			r = Gr_gamma_lookup[verts[i]->b];
			g = Gr_gamma_lookup[verts[i]->b];
			b = Gr_gamma_lookup[verts[i]->b];
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )   {
			// Make 0.75 be 256.0f
			r = Gr_gamma_lookup[verts[i]->r];
			g = Gr_gamma_lookup[verts[i]->g];
			b = Gr_gamma_lookup[verts[i]->b];
		} else {
			// use constant RGB values...
		}

		if (gr_screen.current_bitmap==CLOAKMAP)
		{
			r=g=b=Interp_cloakmap_alpha;
			a=255;
		}

		ubyte sc[3] = { va->spec_r, va->spec_g, va->spec_b };

		if (!override_primary)
		{
			glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );
			glSecondaryColor3ubvEXT( sc );
		}
		else
		{
			glColor3ubv(sc);		
			glSecondaryColor3ubvEXT(sc);
		}

		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (OGL_fogmode == 2)){
			// this is for GL_EXT_FOG_COORD 
			gr_opengl_stuff_fog_coord(va);
		}

		int x, y;
		x = fl2i(va->sx*16.0f);
		y = fl2i(va->sy*16.0f);

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		sx = i2fl(x) / 16.0f;
		sy = i2fl(y) / 16.0f;

		if ( flags & TMAP_FLAG_TEXTURED )       {
			tu = va->u*u_scale;
			tv = va->v*v_scale;

			//use opengl hardware multitexturing
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,tu,tv);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,tu,tv);
			
			if (GL_supported_texture_units>2)			glMultiTexCoord2fARB(GL_TEXTURE2_ARB,tu,tv);
			
		}

		glVertex4f(sx/rhw, sy/rhw, -sz/rhw, 1.0f/rhw);
	}
	glEnd();
}

void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage )
{
	if ( !gr_tcache_set(SPECMAP, tmap_type, u_scale, v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, stage )) {
		//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
		return;
	}

	// render with spec lighting only
	opengl_default_light_settings(0, 0, 1);

	gr_opengl_set_modulate_tex_env();

	glBlendFunc(GL_ONE,GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
}

void opengl_reset_spec_mapping()
{
	// reset lights to default values
	opengl_default_light_settings();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler)
{
	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;
	
	if ( gr_zbuffering )    {
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)   )       {
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}
	
	tmap_type = TCACHE_TYPE_NORMAL;

	if ( flags & TMAP_FLAG_TEXTURED )       {
		r = g = b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )        
	{
		if (1) {
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;
			
			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;
			
			alpha = 255;
			
			if ( factor <= 1.0f )   {
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
				
			if ( factor > 1.0f )    {
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		if(Bm_pixel_format == BM_PIXEL_FORMAT_ARGB) {
			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		} else {
			alpha_blend = ALPHA_BLEND_NONE;
		}
		alpha = 255;
	}

	if(flags & TMAP_FLAG_INTERFACE){
		tmap_type = TCACHE_TYPE_INTERFACE;
	}

	texture_source = TEXTURE_SOURCE_NONE;
	
	if ( flags & TMAP_FLAG_TEXTURED )       {
		// use nonfiltered textures for interface graphics
		if(flags & TMAP_FLAG_INTERFACE){
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}

	gr_opengl_set_state( texture_source, alpha_blend, zbuffer_type );
}

void gr_opengl_tmapper_internal( int nv, vertex ** verts, uint flags, int is_scaler )
{
	int i, stage = 0;
	float u_scale = 1.0f, v_scale = 1.0f;
	bool use_spec = false;

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3 ();
	}

	
	int alpha,tmap_type, r, g, b;

	gr_opengl_set_2d_matrix();

	opengl_setup_render_states(r,g,b,alpha,tmap_type,flags,is_scaler);

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, stage )) {
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}
		stage++; // bump!

		// cloakmap
		if ( Interp_multitex_cloakmap > 0 ) {
			SPECMAP = -1;	// don't add a spec map if we are cloaked
			GLOWMAP = -1;	// don't use a glowmap either, shouldn't see them

			if ( !gr_tcache_set(Interp_multitex_cloakmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, stage )) {
				//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
				return;
			}
			gr_opengl_set_additive_tex_env();
			stage++; // bump
		}

		// glowmap
		if ( (GLOWMAP > -1) && !Cmdline_noglow ) {
			if ( !gr_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, stage )) {
				//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
				return;
			}
			gr_opengl_set_additive_tex_env();
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
	
	
	if (flags & TMAP_FLAG_PIXEL_FOG) {
		int r, g, b;
		int ra, ga, ba;
		ra = ga = ba = 0;
	
		for (i=nv-1;i>=0 ;i--)	
		{
			vertex *va = verts[i];
			float sx, sy;
                
			int x, y;
			x = fl2i(va->sx*16.0f);
			y = fl2i(va->sy*16.0f);

			x += gr_screen.offset_x*16;
			y += gr_screen.offset_y*16;

			sx = i2fl(x) / 16.0f;
			sy = i2fl(y) / 16.0f;

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

	if (CLOAKMAP==gr_screen.current_bitmap)
		glBlendFunc(GL_ONE, GL_ONE);

	opengl_draw_primitive(nv, verts, flags, u_scale, v_scale, r, g, b, alpha);

	if (ogl_maybe_pop_arb1)
	{
		gr_pop_texture_matrix(1);
		ogl_maybe_pop_arb1=0;
	}

	if ( use_spec ) {
		opengl_set_spec_mapping(tmap_type, &u_scale, &v_scale);
		opengl_draw_primitive(nv, verts, flags, u_scale, v_scale, r, g, b, alpha, 1);
		opengl_reset_spec_mapping();
	}

	gr_opengl_end_2d_matrix();
}


//ok we're making some assumptions for now.  mainly it must not be multitextured, lit or fogged.
void gr_opengl_tmapper_internal3d( int nv, vertex ** verts, uint flags, int is_scaler )
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3 ();
	}
		
	glDisable(GL_CULL_FACE);
	
	int alpha,tmap_type, r, g, b;

	opengl_setup_render_states(r,g,b,alpha,tmap_type,flags,is_scaler);

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))
		{
			//mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}
	}

	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	glSecondaryColor3ubvEXT(GL_zero_3ub);

	vertex *va;
	
	if (flags & TMAP_FLAG_TRISTRIP)
		glBegin(GL_TRIANGLE_STRIP);
	else if (flags & TMAP_FLAG_TRILIST)
		glBegin(GL_TRIANGLES);
	else
		glBegin(GL_TRIANGLE_FAN);

	for (i=0; i < nv; i++) {
		va=verts[i];

		if(flags & TMAP_FLAG_RGB)
			glColor3ub(va->r, va->g, va->b);
		//	glColor3ub((ubyte)Gr_gamma_lookup[va->r], (ubyte)Gr_gamma_lookup[va->g], (ubyte)Gr_gamma_lookup[va->b]);

		glTexCoord2f(va->u, va->v);
		glVertex3f(va->x,va->y,va->z);
	}
	glEnd();
}

void gr_opengl_tmapper_batch_3d_unlit( int nverts, vertex *verts, uint flags)
{
	// Batching code goes here
	// See D3D code to see what should be happening
	// This function should only ever be called by batch_render()
}

void gr_opengl_tmapper( int nverts, vertex **verts, uint flags )
{
	if ((!Cmdline_nohtl) && (flags & TMAP_HTL_3D_UNLIT))
		gr_opengl_tmapper_internal3d( nverts, verts, flags, 0 );
	else
		gr_opengl_tmapper_internal(nverts,verts,flags,0);

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

	gr_opengl_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_opengl_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_opengl_init_color(color *c, int r, int g, int b)
{
	c->screen_sig = gr_screen.signature;
	c->red = (unsigned char)r;
	c->green = (unsigned char)g;
	c->blue = (unsigned char)b;
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

void gr_opengl_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_opengl_init_color( clr, r, g, b );

	clr->alpha = (ubyte)alpha;
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_opengl_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_opengl_init_color( &gr_screen.current_color, r, g, b );	
}

void gr_opengl_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )       {
			gr_opengl_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_opengl_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

void gr_opengl_print_screen(char *filename)
{
	char tmp[MAX_FILENAME_LEN];
	ubyte *buf = NULL;

	strcpy( tmp, filename );
	strcat( tmp, NOX(".tga"));

	CFILE *f = cfopen(tmp, "wb", CFILE_NORMAL, CF_TYPE_ROOT);

	if (f == NULL)
		return;

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

	buf = (ubyte*)vm_malloc(gr_screen.max_w * gr_screen.max_h * 3);

	if (buf == NULL)
		return;

	memset(buf, 0, gr_screen.max_w * gr_screen.max_h * 3);

	glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGR_EXT, GL_UNSIGNED_BYTE, buf);

	cfwrite(buf, gr_screen.max_w * gr_screen.max_h * 3, 1, f);

	cfclose(f);

	vm_free(buf);
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

void opengl_tcache_cleanup ();
void gr_opengl_cleanup(int minimize)
{	
	if ( !OGL_enabled )
		return;

	if (!Fred_running) {
		gr_reset_clip();
		gr_clear();
		gr_flip();
	}

	OGL_enabled = 0;

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
	  	if (OGL_fogmode==3)
			glFogf(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);

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
			(gr_screen.current_fog_color.blue != b) ) {
		GLfloat fc[4];
		
		gr_opengl_init_color( &gr_screen.current_fog_color, r, g, b );
	
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

void gr_opengl_set_cull(int cull)
{
	if (cull) {
		glEnable (GL_CULL_FACE);
		glFrontFace (GL_CCW);
	} else {
		glDisable (GL_CULL_FACE);
	}
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

void gr_opengl_flash(int r, int g, int b)
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);
	
	if ( r || g || b ) {
		gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
		
		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
		
		glColor4ub((ubyte)r, (ubyte)g, (ubyte)b, 255);
		glBegin (GL_QUADS);
		  glVertex3f (x1, y2, -0.99f);

		  glVertex3f (x2, y2, -0.99f);

		  glVertex3f (x2, y1, -0.99f);

		  glVertex3f (x1, y1, -0.99f);
		glEnd ();	  
	}
}

int gr_opengl_zbuffer_get()
{
	if ( !gr_global_zbuffering )    {
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if (gr_zbuffering_mode == GR_ZBUFF_NONE )      {
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}

void gr_opengl_zbuffer_clear(int mode)
{
	if (mode) {
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;
		
		gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
		glClear ( GL_DEPTH_BUFFER_BIT );
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

// copied out of grd3d.cpp
inline WORD ogl_ramp_val(uint i, float recip_gamma, int scale = 65535)
{
    return static_cast<WORD>(scale*pow(i/255.0f, 1.0f/recip_gamma));
}

void gr_opengl_set_gamma(float gamma)
{
	int i;
	WORD g, gamma_ramp[3][256];

	Gr_gamma = gamma;
	Gr_gamma_int = int (Gr_gamma*10);

	// old way - not sure if this is still needed but keep it for now
	// Create the Gamma lookup table
	for (i=0;i<256; i++) {
		int v = fl2i(pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f);
		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )     {
			v = 0;
		}
		Gr_gamma_lookup[i] = v;
	}

	// new way - but not while running FRED
	if (!Fred_running && !Cmdline_no_set_gamma) {
		// Create the Gamma lookup table
		for (i = 0; i < 256; i++) {
			g = ogl_ramp_val(i, gamma);
		  	gamma_ramp[0][i] = gamma_ramp[1][i] = gamma_ramp[2][i] = g;
		}

#ifdef _WIN32
		SetDeviceGammaRamp( dev_context, gamma_ramp );
#else
		SDL_SetGammaRamp( gamma_ramp[0], gamma_ramp[1], gamma_ramp[2] );
#endif
	}

	// Flush any existing textures
	opengl_tcache_flush();
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

	gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
	
	if (gr_screen.bits_per_pixel == 16) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data);
	} else if (gr_screen.bits_per_pixel == 32) {
		glReadPixels(0, gr_screen.max_h-h, w, h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	}


}


#define MAX_MOUSE_SAVE_SIZE (32*32)

void gr_opengl_save_mouse_area(int x, int y, int w, int h)
{
	GLenum fmt;
	int cursor_size;

	if (gr_screen.bits_per_pixel == 32) {
		fmt = GL_UNSIGNED_INT_8_8_8_8_REV;
	} else {
		fmt = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	}

	// lazy - taylor
	cursor_size = (GL_mouse_saved_size * GL_mouse_saved_size);

	// no reason to be bigger than the cursor, should never be smaller
	if (w != GL_mouse_saved_size)
		w = GL_mouse_saved_size;
	if (h != GL_mouse_saved_size)
		h = GL_mouse_saved_size;

	Gr_opengl_mouse_saved_x1 = x;
	Gr_opengl_mouse_saved_y1 = y;
	Gr_opengl_mouse_saved_x2 = x+w-1;
	Gr_opengl_mouse_saved_y2 = y+h-1;
        
	CLAMP(Gr_opengl_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_opengl_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_opengl_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(Gr_opengl_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );
        
	Assert( (w * h) <= MAX_MOUSE_SAVE_SIZE );

	gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
        
	// this should really only have to be malloc'd once
	if (GL_saved_mouse_data == NULL)
		GL_saved_mouse_data = (ubyte*)vm_malloc(cursor_size * gr_screen.bytes_per_pixel);

	if (GL_saved_mouse_data == NULL)
		return;

	glReadBuffer(GL_BACK);
	glReadPixels(x, gr_screen.max_h-y-1-h, w, h, GL_BGRA, fmt, GL_saved_mouse_data);
        
	Gr_opengl_mouse_saved = 1;
}

void gr_opengl_free_mouse_area()
{
	if (GL_saved_mouse_data != NULL) {
		vm_free(GL_saved_mouse_data);
		GL_saved_mouse_data = NULL;
	}
}

int gr_opengl_save_screen()
{
	GLenum fmt = 0;
	int rc = -1;
	ubyte *sptr, *dptr;
	ubyte *opengl_screen_tmp = NULL;

	gr_reset_clip();

	if (gr_screen.bits_per_pixel == 32) {
		fmt = GL_UNSIGNED_INT_8_8_8_8_REV;
	} else {
		fmt = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	}

	if (!GL_saved_screen)
		GL_saved_screen = (ubyte*)vm_malloc( gr_screen.max_w * gr_screen.max_h * gr_screen.bytes_per_pixel );

	if (!GL_saved_screen) 
 	{
		mprintf(( "Couldn't get memory for saved screen!\n" ));
 		return -1;
 	}

	opengl_screen_tmp = (ubyte*)vm_malloc( gr_screen.max_w * gr_screen.max_h * gr_screen.bytes_per_pixel );

	if (!opengl_screen_tmp) 
 	{
		mprintf(( "Couldn't get memory for temporary saved screen!\n" ));
 		return -1;
 	}

	glDisable(GL_TEXTURE_2D);

#ifdef _WIN32
	glReadBuffer(GL_FRONT);
#else
	glReadBuffer(GL_BACK);
#endif
	glReadPixels(0, 0, gr_screen.max_w, gr_screen.max_h, GL_BGRA, fmt, opengl_screen_tmp);

	glEnable(GL_TEXTURE_2D);

	sptr = (ubyte *)&opengl_screen_tmp[gr_screen.max_w*gr_screen.max_h*gr_screen.bytes_per_pixel];
	dptr = (ubyte *)GL_saved_screen;

	int width_times_pixel = (gr_screen.max_w * gr_screen.bytes_per_pixel);
	int mouse_times_pixel = (GL_mouse_saved_size * gr_screen.bytes_per_pixel);

	for (int j = 0; j < gr_screen.max_h; j++)
	{
		sptr -= width_times_pixel;
		memcpy(dptr, sptr, width_times_pixel);
		dptr += width_times_pixel;
	}
        
	vm_free(opengl_screen_tmp);

	if (Gr_opengl_mouse_saved && GL_saved_mouse_data)
	{
		sptr = (ubyte *)GL_saved_mouse_data;
		dptr = (ubyte *)&GL_saved_screen[(Gr_opengl_mouse_saved_x1+Gr_opengl_mouse_saved_y2*gr_screen.max_w)*gr_screen.bytes_per_pixel];

		for (int i = 0; i < GL_mouse_saved_size; i++)
		{
			memcpy(dptr, sptr, mouse_times_pixel);
			sptr += mouse_times_pixel;
			dptr -= width_times_pixel;
		}
	}

	rc = bm_create(gr_screen.bits_per_pixel, gr_screen.max_w, gr_screen.max_h, GL_saved_screen, 0);

	return rc;
}

void gr_opengl_restore_screen(int bmp_id)
{
	gr_reset_clip();

	if ( !GL_saved_screen ) {
		gr_clear();
		return;
	}

	gr_set_bitmap(bmp_id);
	gr_bitmap(0,0,false);	// don't scale here since we already have real screen size
}

void gr_opengl_free_screen(int bmp_id)
{
	if (!GL_saved_screen)
		return;

	vm_free(GL_saved_screen);
	GL_saved_screen = NULL;

	bm_release(bmp_id);
}

static void gr_opengl_flush_frame_dump()
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
	gr_opengl_flush_frame_dump();
	
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
		gr_opengl_flush_frame_dump();
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
       
void gr_opengl_bitmap_internal(int x,int y,int w,int h,int sx,int sy)
{

	//int i,j,k;
	unsigned int tex=0;			//gl texture number

	bitmap * bmp;
	int size;


	int htemp=(int)pow((double)2,ceil(log10((double)h)/log10((double)2)));
	int wtemp=(int)pow((double)2,ceil(log10((double)w)/log10((double)2)));
	//gr_opengl_set_state(TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

	glGenTextures(1, &tex);

	Assert(tex !=0);
  	//Assert(opengl_screen != NULL);
	//Assert(opengl_bmp_buffer != NULL);

	// mharris mod - not sure if this is right...
	bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );

	if (bmp == NULL) {
		return;
	}

	size=w*h*4;

	/*mprintf(("gr_opengl_bitmap_ex_internal: at (%3d,%3d) size (%3d,%3d) name %s -- temp (%d,%d)\n", 
  				x, y, w, h, 
 				bm_get_filename(gr_screen.current_bitmap),wtemp,htemp));*/

	const ushort * sptr = (const ushort*)bmp->data;

	glColor3ub(255,255,255);

	glBindTexture(GL_TEXTURE_2D,tex);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (bmp->bpp == 32) {
		glTexImage2D(GL_TEXTURE_2D, 0, 4, wtemp, htemp, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,w,h, GL_BGRA,GL_UNSIGNED_INT_8_8_8_8_REV, sptr);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, 4, wtemp, htemp, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,w,h, GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV, sptr);
	}
	
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2i(x,y);

		glTexCoord2f(0,i2fl(h)/i2fl(htemp));
		glVertex2i(x,y+h);

		glTexCoord2f(i2fl(w)/i2fl(wtemp),i2fl(h)/i2fl(htemp));
		glVertex2i(x+w,y+h);

		glTexCoord2f(i2fl(w)/i2fl(wtemp),0);
		glVertex2i(x+w,y);
	glEnd();

	bm_unlock(gr_screen.current_bitmap);

	glDeleteTextures(1, &tex);

//	// set the raster pos
/*	int gl_y_origin = y+h;

	glRasterPos2i(gr_screen.offset_x + x, gl_y_origin);

	// put the bitmap into the GL framebuffer
	// BAD BAD BAD - change this asap
	glDrawPixels(w, h,					// width, height
					 GL_RGBA,			// format
					 GL_UNSIGNED_BYTE,	// type
					 data);
	*/
}


//these are penguins bitmap functions
void gr_opengl_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
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

	gr_opengl_bitmap_internal(x,y,w,h,sx,sy);

}

void gr_opengl_bitmap(int x, int y)
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

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr_opengl_bitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_opengl_push_texture_matrix(int unit)
{
	if (unit > GL_supported_texture_units) return;
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(current_matrix);
}

void gr_opengl_pop_texture_matrix(int unit)
{
	if (unit > GL_supported_texture_units) return;
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(current_matrix);
}

void gr_opengl_translate_texture_matrix(int unit, vec3d *shift)
{
	if (unit > GL_supported_texture_units){ /*tex_shift=*shift;*/ return;}
	GLint current_matrix;
	glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
	glActiveTextureARB(GL_TEXTURE0_ARB+unit);
	glMatrixMode(GL_TEXTURE);
	glTranslated(shift->xyz.x, shift->xyz.y, shift->xyz.z);	
	glMatrixMode(current_matrix);
//	tex_shift=vmd_zero_vector;
}




void opengl_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static float pre_set_colours[MAX_NUM_TIMERBARS][3] = 
	{
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

	gr_opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);

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

int opengl_check_for_errors()
{
	int ec = 0, error = GL_NO_ERROR;

	do {
		error = glGetError();
		
		if (error != GL_NO_ERROR) {
			nprintf(("OpenGL", "!!ERROR!!: %s\n", gluErrorString(error)));
			ec++;
		}
	} while (error != GL_NO_ERROR);

	return ec;
}

void gr_opengl_close()
{
	if (currently_enabled_lights != NULL) {
		vm_free(currently_enabled_lights);
		currently_enabled_lights = NULL;
	}

	gr_opengl_free_mouse_area();

#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);

	if (rend_context) {
		wglDeleteContext(rend_context);
		rend_context=NULL;
	}

	// restore original gamma settings
	SetDeviceGammaRamp( dev_context, original_gamma_ramp );
#endif
}

void opengl_setup_function_pointers()
{
	// *****************************************************************************
	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.

	gr_screen.gf_flip = gr_opengl_flip;
	gr_screen.gf_flip_window = gr_opengl_flip_window;
	gr_screen.gf_set_clip = gr_opengl_set_clip;
	gr_screen.gf_reset_clip = gr_opengl_reset_clip;
	gr_screen.gf_set_font = grx_set_font;
	
	gr_screen.gf_set_color = gr_opengl_set_color;
	gr_screen.gf_set_bitmap = gr_opengl_set_bitmap;
	gr_screen.gf_create_shader = gr_opengl_create_shader;
	gr_screen.gf_set_shader = gr_opengl_set_shader;
	gr_screen.gf_clear = gr_opengl_clear;
//	gr_screen.gf_bitmap = gr_opengl_bitmap;
//	gr_screen.gf_bitmap_ex = gr_opengl_bitmap_ex;
	gr_screen.gf_aabitmap = gr_opengl_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_opengl_aabitmap_ex;
	
//	gr_screen.gf_rect = gr_opengl_rect;
//	gr_screen.gf_shade = gr_opengl_shade;
	gr_screen.gf_string = gr_opengl_string;
	gr_screen.gf_circle = gr_opengl_circle;
	gr_screen.gf_curve = gr_opengl_curve;

	gr_screen.gf_line = gr_opengl_line;
	gr_screen.gf_aaline = gr_opengl_aaline;
	gr_screen.gf_pixel = gr_opengl_pixel;
	gr_screen.gf_scaler = gr_opengl_scaler;
	gr_screen.gf_tmapper = gr_opengl_tmapper;
	gr_screen.gf_tmapper_batch_3d_unlit = gr_opengl_tmapper_batch_3d_unlit;

	gr_screen.gf_gradient = gr_opengl_gradient;

	gr_screen.gf_set_palette = gr_opengl_set_palette;
	gr_screen.gf_get_color = gr_opengl_get_color;
	gr_screen.gf_init_color = gr_opengl_init_color;
	gr_screen.gf_init_alphacolor = gr_opengl_init_alphacolor;
	gr_screen.gf_set_color_fast = gr_opengl_set_color_fast;
	gr_screen.gf_print_screen = gr_opengl_print_screen;

	gr_screen.gf_fade_in = gr_opengl_fade_in;
	gr_screen.gf_fade_out = gr_opengl_fade_out;
	gr_screen.gf_flash = gr_opengl_flash;
	
	gr_screen.gf_zbuffer_get = gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_opengl_zbuffer_clear;
	
	gr_screen.gf_save_screen = gr_opengl_save_screen;
	gr_screen.gf_restore_screen = gr_opengl_restore_screen;
	gr_screen.gf_free_screen = gr_opengl_free_screen;
	
	gr_screen.gf_dump_frame_start = gr_opengl_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_opengl_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_opengl_dump_frame;
	
	gr_screen.gf_set_gamma = gr_opengl_set_gamma;
	
	gr_screen.gf_lock = gr_opengl_lock;
	gr_screen.gf_unlock = gr_opengl_unlock;
	
	gr_screen.gf_fog_set = gr_opengl_fog_set;	

	// UnknownPlayer : Don't recognize this - MAY NEED DEBUGGING
	gr_screen.gf_get_region = gr_opengl_get_region;

	gr_screen.gf_bm_free_data				= gr_opengl_bm_free_data;
	gr_screen.gf_bm_create					= gr_opengl_bm_create;
	gr_screen.gf_bm_init					= gr_opengl_bm_init;
	gr_screen.gf_bm_load					= gr_opengl_bm_load;
	gr_screen.gf_bm_page_in_start			= gr_opengl_bm_page_in_start;
	gr_screen.gf_bm_lock					= gr_opengl_bm_lock;

	gr_screen.gf_get_pixel = gr_opengl_get_pixel;

	gr_screen.gf_set_cull = gr_opengl_set_cull;

	gr_screen.gf_cross_fade = gr_opengl_cross_fade;

	gr_screen.gf_filter_set = gr_opengl_filter_set;

	gr_screen.gf_tcache_set = gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color = gr_opengl_set_clear_color;

	gr_screen.gf_preload = gr_opengl_preload;

	gr_screen.gf_push_texture_matrix = gr_opengl_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_opengl_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_opengl_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing = gr_opengl_set_texture_addressing;
	gr_screen.gf_zbias = gr_opengl_zbias_stub;
	gr_screen.gf_set_fill_mode = gr_opengl_set_fill_mode;
	gr_screen.gf_set_texture_panning = gr_opengl_set_texture_panning;

	gr_screen.gf_make_buffer = gr_opengl_make_buffer;
	gr_screen.gf_destroy_buffer = gr_opengl_destroy_buffer;
	gr_screen.gf_render_buffer = gr_opengl_render_buffer;
	gr_screen.gf_set_buffer = gr_opengl_set_buffer;

	gr_screen.gf_start_instance_matrix = gr_opengl_start_instance_matrix;
	gr_screen.gf_end_instance_matrix = gr_opengl_end_instance_matrix;
	gr_screen.gf_start_angles_instance_matrix = gr_opengl_start_instance_angles;

	gr_screen.gf_make_light = gr_opengl_make_light;
	gr_screen.gf_modify_light = gr_opengl_modify_light;
	gr_screen.gf_destroy_light = gr_opengl_destroy_light;
	gr_screen.gf_set_light = gr_opengl_set_light;
	gr_screen.gf_reset_lighting = gr_opengl_reset_lighting;
	gr_screen.gf_set_ambient_light = gr_opengl_set_ambient_light;

	gr_screen.gf_start_clip_plane = gr_opengl_start_clip_plane;
	gr_screen.gf_end_clip_plane = gr_opengl_end_clip_plane;

	gr_screen.gf_lighting = gr_opengl_set_lighting;

	gr_screen.gf_set_proj_matrix=gr_opengl_set_projection_matrix;
	gr_screen.gf_end_proj_matrix=gr_opengl_end_projection_matrix;

	gr_screen.gf_set_view_matrix=gr_opengl_set_view_matrix;
	gr_screen.gf_end_view_matrix=gr_opengl_end_view_matrix;

	gr_screen.gf_push_scale_matrix = gr_opengl_push_scale_matrix;
	gr_screen.gf_pop_scale_matrix = gr_opengl_pop_scale_matrix;
	gr_screen.gf_center_alpha = gr_opengl_center_alpha;

	gr_screen.gf_setup_background_fog = gr_opengl_setup_background_fog;
//	gr_screen.gf_set_environment_mapping = gr_opengl_render_to_env;;

	gr_screen.gf_bm_make_render_target = gr_opengl_bm_make_render_target;
	gr_screen.gf_bm_set_render_target = gr_opengl_bm_set_render_target;

	gr_screen.gf_start_state_block = gr_opengl_start_state_block;
	gr_screen.gf_end_state_block = gr_opengl_end_state_block;
	gr_screen.gf_set_state_block = gr_opengl_set_state_block;

	gr_screen.gf_draw_line_list = gr_opengl_draw_line_list;

	gr_screen.gf_draw_htl_line = gr_opengl_draw_htl_line;
	gr_screen.gf_draw_htl_sphere = gr_opengl_draw_htl_sphere;

	// NOTE: All function pointers here should have a Cmdline_nohtl check at the top
	//       if they shouldn't be run in non-HTL mode, Don't keep separate entries.
	// *****************************************************************************
}

void gr_opengl_render_to_env(int FACE);

extern char *Osreg_title;
void gr_opengl_init(int reinit)
{
	char *extlist;
	char *curext;
	char *ver;
	int major = 0, minor = 0;
	int bpp = gr_screen.bits_per_pixel;

	if(!Cmdline_nohtl) {
		opengl_init_vertex_buffers();
	}

	//shut these command line parameters down if they are in use
	Cmdline_batch_3dunlit = 0;

#ifdef GL_NO_HTL
	// turn off HT&L and VBO if we can't support it with built libs
	Cmdline_nohtl = 1;
	Cmdline_novbo = 1;
#endif

	switch( bpp )	{
	case 16:
	/*
		Gr_red=Gr_t_red;
		Gr_green=Gr_t_green;
		Gr_blue=Gr_t_blue;
		Gr_alpha=Gr_t_alpha;
	*/
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

	case 32:
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

		Gr_alpha.bits=8;
		Gr_alpha.shift=24;
		Gr_alpha.mask=0xff000000;
		Gr_alpha.scale=1;


		break;

	default:
		mprintf(("Illegal BPP passed to gr_opengl_init() -- %d",bpp));
		Int3();	// Illegal bpp
	}

	Gr_t_red.bits=5;
	Gr_t_red.mask = 0x7c00;
	Gr_t_red.shift = 10;
	Gr_t_red.scale = 8;

	Gr_t_green.bits=5;
	Gr_t_green.mask = 0x03e0;
	Gr_t_green.shift = 5;
	Gr_t_green.scale = 8;
	
	Gr_t_blue.bits=5;
	Gr_t_blue.mask = 0x001f;
	Gr_t_blue.shift = 0;
	Gr_t_blue.scale = 8;

	Gr_t_alpha.bits=1;
	Gr_t_alpha.mask=0x8000;
	Gr_t_alpha.scale=255;
	Gr_t_alpha.shift=15;

	/*
Gr_ta_red: bits=0, mask=f00, scale=17, shift=8
Gr_ta_green: bits=0, mask=f00, scale=17, shift=4
Gr_ta_blue: bits=0, mask=f00, scale=17, shift=0
Gr_ta_alpha: bits=0, mask=f000, scale=17, shift=c
*/
	// DDOI - set these so no one else does!
	Gr_ta_red.bits=4;
	Gr_ta_red.mask = 0x0f00;
	Gr_ta_red.shift = 8;
	Gr_ta_red.scale = 17;

	Gr_ta_green.bits=4;
	Gr_ta_green.mask = 0x00f0;
	Gr_ta_green.shift = 4;
	Gr_ta_green.scale = 17;
	
	Gr_ta_blue.bits=4;
	Gr_ta_blue.mask = 0x000f;
	Gr_ta_blue.shift = 0;
	Gr_ta_blue.scale = 17;

	Gr_ta_alpha.bits=4;
	Gr_ta_alpha.mask = 0xf000;
	Gr_ta_alpha.shift = 12;
	Gr_ta_alpha.scale = 17;

	if ( OGL_enabled )	{
		gr_opengl_cleanup();
		OGL_enabled = 0;
	}

	mprintf(( "Initializing opengl graphics device...\nRes:%dx%dx%d\n",gr_screen.max_w,gr_screen.max_h,gr_screen.bits_per_pixel ));

#ifdef _WIN32
	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cColorBits = (ubyte)bpp;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cDepthBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cRedBits = (ubyte)Gr_red.bits;
	pfd.cRedShift = (ubyte)Gr_red.shift;
	pfd.cBlueBits = (ubyte)Gr_blue.bits;
	pfd.cBlueShift = (ubyte)Gr_blue.shift;
	pfd.cGreenBits = (ubyte)Gr_green.bits;
	pfd.cGreenShift = (ubyte)Gr_green.shift;
//	pfd.cAlphaBits = (ubyte)Gr_alpha.bits;
//	pfd.cAlphaShift = (ubyte)Gr_alpha.shift;


	int PixelFormat;

	HWND wnd=(HWND)os_get_window();

	Assert(wnd);
	dev_context=GetDC(wnd);

	if (!dev_context)
	{
		MessageBox(wnd, "Unable to get Device context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);		
	}

	PixelFormat=ChoosePixelFormat(dev_context, &pfd);
	if (!PixelFormat)
	{
		MessageBox(wnd, "Unable to choose pixel format for OpenGL W32!","error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	if (!SetPixelFormat(dev_context, PixelFormat, &pfd))
	{
		MessageBox(wnd, "Unable to set pixel format for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	rend_context=wglCreateContext(dev_context);
	if (!rend_context)
	{
		MessageBox(wnd, "Unable to create rendering context for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}
	
	if (!wglMakeCurrent(dev_context, rend_context))
	{
		MessageBox(wnd, "Unable to make current thread for OpenGL W32!", "error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	// get the default gamma ramp so that we can restore it on close
	GetDeviceGammaRamp( dev_context, &original_gamma_ramp );
#else
	if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0)
	{
		fprintf (stderr, "Couldn't init SDL: %s", SDL_GetError());
		exit (1);
	}

	int flags = SDL_OPENGL;

	// grab mouse/key unless told otherwise, ignore when we are going fullscreen
	if ( (Cmdline_window || os_config_read_uint(NULL, "Fullscreen", 1) == 0) && !Cmdline_no_grab ) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}

	if (SDL_SetVideoMode (gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel, flags) == NULL) {
		fprintf (stderr, "Couldn't set video mode: %s", SDL_GetError ());
		exit (1);
	}

	int tmp_red = 0;
	int tmp_green = 0;
	int tmp_blue = 0;
	int tmp_depth = 0;
	int tmp_db = 0;

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &tmp_red);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &tmp_green);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &tmp_blue);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &tmp_depth);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &tmp_db);

	mprintf(("Actual SDL Video values = R: %d, G: %d, B: %d, depth: %d, double-buffer: %d\n", tmp_red, tmp_green, tmp_blue, tmp_depth, tmp_db));

	SDL_ShowCursor(0);

	/* might as well put this here */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	HWND wnd = 0;
#endif

	// version check
	ver = (char *)glGetString(GL_VERSION);
	sscanf(ver, "%d.%d", &major, &minor);

	if ( (major <= REQUIRED_GL_MAJOR_VERSION) && (minor < REQUIRED_GL_MINOR_VERSION) ) {
		Error(LOCATION,"Current GL Version of %i.%i is less than the required version of %i.%i\nSwitch video modes or update your drivers.", major, minor, REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
	}

	OGL_enabled = 1;
	if (!reinit)
	{
		mprintf(("OPENGL INITED!\n"));
		mprintf(("\n"));
		mprintf(( "  Vendor     : %s\n", glGetString( GL_VENDOR ) ));
		mprintf(( "  Renderer   : %s\n", glGetString( GL_RENDERER ) ));
		mprintf(( "  Version    : %s\n", ver ));
		mprintf(( "  Extensions : \n" ));

		//print out extensions
		OGL_extensions=(const char*)glGetString(GL_EXTENSIONS);

		// we use the "+1" here to have an extra NULL char on the end (with the memset())
		// this is to fix memory errors when the last char in extlist is the same as the token
		// we are looking for and ultra evil strtok() may still return non-NULL at EOS
		extlist = (char*)vm_malloc(strlen(OGL_extensions) + 1);
		memset(extlist, 0, strlen(OGL_extensions) + 1);

		if (extlist != NULL) {
			memcpy(extlist, OGL_extensions, strlen(OGL_extensions));

			curext=strtok(extlist, " ");

			while (curext)
			{
				mprintf(( "      %s\n", curext ));
				curext=strtok(NULL, " ");
			}

			vm_free(extlist);
			extlist = NULL;
		}
		mprintf(("\n"));
	}

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	if (!Cmdline_window && !reinit)
	{
		opengl_go_fullscreen(wnd);
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, gr_screen.max_w, gr_screen.max_h,0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DITHER);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);
		
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	
	glEnable(GL_TEXTURE_2D);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glDepthRange(0.0, 1.0);
	
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glFlush();
	
	Bm_pixel_format = BM_PIXEL_FORMAT_ARGB;
		
	if(!Fred_running)
		OGL_fogmode=1;

	if (!reinit)
	{
		//start extension
		opengl_get_extensions();
		opengl_tcache_init (1);
	}

	gr_opengl_clear();

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

//	RunGLTest( 0, NULL, 0, 0, 0, 0.0, 0 );

	opengl_setup_function_pointers();

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	//if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Texture_compression_enabled=((GL_Extensions[GL_TEX_COMP_S3TC].enabled) && (GL_Extensions[GL_COMP_TEX].enabled));

	//setup anisotropic filtering if found
	if (GL_Extensions[GL_TEX_FILTER_ANSIO].enabled)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	}

	//allow VBOs to be used
	if ( (!Cmdline_nohtl) && (!Cmdline_novbo) && (GL_Extensions[GL_ARB_VBO_BIND_BUFFER].enabled))
		VBO_ENABLED = 1;
	
//	glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
//	glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	//setup the best fog function found
	//start with NV Radial Fog (GeForces only)  -- needs t&l, will have to wait
	//if (GL_Extensions[GL_NV_RADIAL_FOG].enabled)
	//	OGL_fogmode=3;
	/*else*/
	if (GL_Extensions[GL_FOG_COORDF].enabled && !Fred_running)
		OGL_fogmode=2;

	if (GL_Extensions[GL_SECONDARY_COLOR_3UBV].enabled)
		glEnable(GL_COLOR_SUM_EXT);

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &GL_supported_texture_units);

	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &GL_max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &GL_max_elements_indices);

	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled) {
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_combine_arb;
	} else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled) {
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_combine_ext;
	} else {
		gr_opengl_set_tex_src = gr_opengl_set_tex_state_no_combine;
	}

	// setup the lighting stuff that will get used later
	opengl_init_light();

	glDisable(GL_LIGHTING); //making sure of it

	mprintf(( "  Max texture units: %i\n", GL_supported_texture_units ));
	mprintf(( "  Max elements vertices: %i\n", GL_max_elements_vertices ));
	mprintf(( "  Max elements indices: %i\n", GL_max_elements_indices ));
	mprintf(( "  Max texture size: %ix%i\n", GL_max_texture_width, GL_max_texture_height ));
	mprintf(( "  Texture compression enabled: %s\n", Texture_compression_enabled ? NOX("YES") : NOX("NO") ));
	mprintf(( "\n" ));

	// This stops fred crashing if no textures are set
	gr_screen.current_bitmap = -1;

	if (gr_screen.custom_size != -1) {
		// taken from D3D version
		GL_uv_resize_offset_u = GL_uv_resize_offset_v = 0.0044f;
		gr_unsize_screen_posf( &GL_uv_resize_offset_u, &GL_uv_resize_offset_v );
	}

	TIMERBAR_SET_DRAW_FUNC(opengl_render_timer_bar);	

	atexit( gr_opengl_close );
}

DCF(min_ogl, "minimizes opengl")
{
	if (Dc_command)
	{
		if (gr_screen.mode != GR_OPENGL)
			return;
		opengl_minimize();
	}
	if (Dc_help)
		dc_printf("minimizes opengl\n");
}

DCF(aniso, "toggles anisotropic filtering")
{
	if (Dc_command)
	{
		if (max_aniso==1.0f) max_aniso=2.0f;
		else max_aniso=1.0f;
	}
	if (Dc_help) dc_printf("toggles anisotropic filtering");
}
