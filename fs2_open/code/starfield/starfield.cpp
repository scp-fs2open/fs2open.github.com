/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/StarField.cpp $
 * $Revision: 2.92 $
 * $Date: 2007-01-14 14:03:40 $
 * $Author: bobboau $
 *
 * Code to handle and draw starfields, background space image bitmaps, floating
 * debris, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.91  2007/01/14 10:26:39  wmcoolmon
 * Attempt to remove various warnings under MSVC 2003, mostly related to casting, but also some instances of inaccessible code.
 *
 * Revision 2.90  2007/01/10 01:45:06  taylor
 * compiler warning fixage
 *
 * Revision 2.89  2007/01/07 12:46:19  taylor
 * fix problem with model unloading that happened because skyboxes aren't reset properly
 * be sure that skybox textures are paged in properly
 * there needs to be a case insensitive string check when looking for existing bitmap filenames
 *
 * Revision 2.88  2007/01/07 03:44:50  Goober5000
 * don't display any error messages when there are no background bitmaps to load
 *
 * Revision 2.87  2006/12/28 00:59:48  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.86  2006/11/24 22:46:25  Goober5000
 * FRED again updates backgrounds while the user is editing them
 *
 * Revision 2.85  2006/11/16 00:51:43  taylor
 * clean up skybox model selection and usage to work better with FRED
 * make sure to go ahead and set the skybox model when it's set/changed in FRED
 * go ahead and load/set the envmap  when it's set/changed in FRED
 * get rid of extra envmap image types, only DDS is actually supported (not sure what the hell I was thinking there)
 *
 * Revision 2.84  2006/11/06 06:46:08  taylor
 * fix some of the envmap issues
 *  - use proper hand-ness for OGL
 *  - fix distortion
 *  - get rid of extra index buffer requirement
 * change starfield bitmaps to use an instance matrix rather than going through all the trouble of resetting the view matrix
 * basic cleanup and get rid of a couple of struct/variable naming issues (compiler sanity)
 * make double sure that we aren't using culling of z-buffering when rendering starfield bitmaps
 *
 * Revision 2.83  2006/11/06 06:19:17  taylor
 * rename set_warp_globals() to model_set_warp_globals()
 * remove two old/unused MR flags (MR_ALWAYS_REDRAW, used for caching that doesn't work; MR_SHOW_DAMAGE, didn't do anything)
 * add MR_FULL_DETAIL to render an object regardless of render/detail box setting
 * change "model_current_LOD" to a global "Interp_detail_level" and the static "Interp_detail_level" to "Interp_detail_level_locked", a bit more descriptive
 * minor bits of cleanup
 * change a couple of vm_vec_scale_add2() calls to just vm_vec_add2() calls in ship.cpp, since that was the final result anyway
 *
 * Revision 2.82  2006/09/11 06:51:17  taylor
 * fixes for stuff_string() bounds checking
 *
 * Revision 2.81  2006/09/04 06:08:20  wmcoolmon
 * Don't force fs2_open to load motion debris.
 *
 * Revision 2.80  2006/08/20 00:49:09  taylor
 * slight optimizations
 * compiler warning fixes
 *
 * Revision 2.79  2006/08/06 18:47:29  Goober5000
 * add the multiple background feature
 * --Goober5000
 *
 * Revision 2.78  2006/07/30 02:20:17  Goober5000
 * bah, revert
 *
 * Revision 2.76  2006/07/06 05:29:39  Goober5000
 * commented annoying redundant warnings
 * --Goober5000
 *
 * Revision 2.75  2006/07/06 04:06:04  Goober5000
 * 1) complete (almost) changeover to reorganized texture mapping system
 * 2) finally fix texture animation; textures now animate at the correct speed
 * --Goober5000
 *
 * Revision 2.74  2006/07/05 23:36:07  Goober5000
 * cvs comment tweaks
 *
 * Revision 2.73  2006/07/04 07:42:50  Goober5000
 * --in preparation for fixing an annoying animated texture bug, reorganize the various texture structs and glow point structs and clarify several parts of the texture code :P
 * --this breaks animated glow maps, and animated regular maps still aren't fixed, but these will be remedied shortly
 * --Goober5000
 *
 * Revision 2.72  2006/05/27 16:42:16  taylor
 * fix slight freakiness with debris vclips (un)loading
 * comment out some code which was only used if neither D3D or OGL
 * fix dumb particle_emit() calls
 *
 * Revision 2.71  2006/05/13 07:29:52  taylor
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
 * Revision 2.70  2006/04/20 06:32:30  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.69  2006/04/14 18:40:44  taylor
 * make sure that modular star tables actually work without going into an endless loop, or constantly complaining on you
 *
 * Revision 2.68  2006/03/20 06:13:35  taylor
 * might as well do star antialiasing for OGL too
 *
 * Revision 2.67  2006/02/25 21:47:19  Goober5000
 * spelling
 *
 * Revision 2.66  2006/02/20 07:30:15  taylor
 * updated to newest dynamic starfield code
 *  - this mainly is to just better support SEXP based starfield bitmap changes (preloading, better in-mission stuff loading)
 *  - also fixes index_buffer related double-free()
 *  - done waste memory for env index buffer if env is not enabled
 *  - address a couple of bm load/release issues and comment a little to tell why
 *
 * Revision 2.65  2006/02/13 00:20:46  Goober5000
 * more tweaks, plus clarification of checks for the existence of files
 * --Goober5000
 *
 * Revision 2.64  2006/02/03 22:29:01  taylor
 * fix sun flares that I had broken previously
 *
 * Revision 2.63  2006/01/30 06:31:30  taylor
 * dynamic starfield bitmaps (if the thought it was freaky before, just take a look at the new and "improved" version ;))
 *
 * Revision 2.62  2006/01/19 16:00:04  wmcoolmon
 * Lua debugging stuff; gr_bitmap_ex stuff for taylor
 *
 * Revision 2.61  2006/01/02 07:13:38  taylor
 * make sure we clear out the vertex used to render a sun (just to be on the safe side)
 * use parse_modular_table() for -str.tbm
 *
 * Revision 2.60  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.59  2005/11/13 06:50:57  taylor
 * don't fail if not a starfield pof to load, regular starfield bitmaps don't hard fail so this shouldn't either
 *
 * Revision 2.58  2005/10/30 20:03:40  taylor
 * add a bunch of Assert()'s and NULL checks to either help debug or avoid errors
 * fix Mantis bug #381
 * fix a small issue with the starfield bitmap removal sexp since it would read one past the array size
 *
 * Revision 2.57  2005/10/16 11:20:43  taylor
 * use unsigned index buffers
 *
 * Revision 2.56  2005/10/09 08:03:21  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.55  2005/07/18 03:45:09  taylor
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.54  2005/06/19 09:03:05  taylor
 * check_values() shouldn't be inline anymore
 * remove useless neb2_get_fog_intensity() call
 * s/alocate/allocate/g
 *
 * Revision 2.53  2005/04/12 05:26:37  taylor
 * many, many compiler warning and header fixes (Jens Granseuer)
 * fix free on possible NULL in modelinterp.cpp (Jens Granseuer)
 *
 * Revision 2.52  2005/04/11 05:50:36  taylor
 * some limits.h fixes to make GCC happier
 * revert timer asm change since it doesn't even get used with Linux and couldn't have been the slowdown problem
 *
 * Revision 2.51  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.50  2005/03/23 20:09:23  phreak
 * Starfield bitmaps should now draw in non-htl mode
 *
 * Revision 2.49  2005/03/20 20:02:29  phreak
 * export the functions that deal with the creation and destruction of the starfield buffer.
 * FRED needs them
 *
 * Revision 2.48  2005/03/12 19:28:47  taylor
 * helps to get the stupid check right
 *
 * Revision 2.47  2005/03/10 21:16:16  taylor
 * fix screwed up stars coloring in OGL
 *
 * Revision 2.46  2005/03/10 08:00:17  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.45  2005/03/07 13:10:22  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.44  2005/03/01 23:08:23  taylor
 * make sure starfield bitmaps render when not in HTL mode
 * slight header fix for osapi.h
 * add some string overflow protection to modelread and bmpman
 * s/NO_NETWORKING/NO_NETWORK/g  (Inferno builds)
 *
 * Revision 2.43  2005/03/01 06:55:45  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.42  2005/02/23 04:57:29  taylor
 * even more bm_unload() -> bm_release() changes
 *
 * Revision 2.41  2005/02/15 00:03:34  taylor
 * don't try and draw starfield bitmaps if they aren't valid
 * make AB thruster stuff in ship_create() a little less weird
 * replace an Int3() with debug warning and fix crash in docking code
 * make D3D Textures[] allocate on use like OGL does, can only use one anyway
 *
 * Revision 2.40  2005/02/04 20:06:09  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.39  2005/01/30 09:27:40  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 2.38  2005/01/29 08:12:20  wmcoolmon
 * Clipping stuff
 *
 * Revision 2.37  2004/07/26 20:47:53  Kazan
 * remove MCD complete
 *
 * Revision 2.36  2004/07/17 19:01:15  taylor
 * stupid braces...
 *
 * Revision 2.35  2004/07/17 18:59:01  taylor
 * fix set_*_matrix overcompensation, makes OGL work again
 *
 * Revision 2.34  2004/07/12 16:33:07  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.33  2004/07/09 05:52:19  wmcoolmon
 * Re-implemented nomotiondebris, as Bobb overwrote it when he commited his changes.
 *
 * Revision 2.32  2004/07/05 05:09:21  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.31  2004/07/01 01:12:33  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.30  2004/06/19 22:16:20  wmcoolmon
 * Added support for -nomotiondebris
 *
 * Revision 2.29  2004/04/01 15:28:42  taylor
 * don't load all starfield bitmaps on level load
 *
 * Revision 2.28  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.27  2004/03/08 21:55:20  phreak
 * split the star and space debris rendering loops into their own separate functions
 * this makes it easier to edit them if we ever need to and it also increases readability
 * i also brought some variable declarations outside of the rendering loops
 *
 * Revision 2.26  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.25  2004/02/15 06:02:32  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.24  2004/02/14 00:18:36  randomtiger
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
 * Revision 2.23  2004/01/19 00:56:10  randomtiger
 * Some more small changes for Fred OGL
 *
 * Revision 2.22  2004/01/18 14:03:23  randomtiger
 * A couple of FRED_OGL changes.
 *
 * Revision 2.21  2004/01/17 21:59:56  randomtiger
 * Some small changes to the main codebase that allow Fred_open OGL to compile.
 *
 * Revision 2.20  2003/11/11 02:15:47  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.19  2003/10/23 23:49:12  phreak
 * added code to render the user-defined skybox
 *
 * Revision 2.18  2003/09/15 12:34:09  fryday
 * Rollback killed my lens-flare. D'oh.
 *
 * Revision 2.17  2003/09/13 06:02:08  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.13  2003/09/09 17:18:25  matt
 * Broke stars last commit, fixed -Sticks
 *
 * Revision 2.12  2003/09/09 17:10:55  matt
 * Added -nospec cmd line param to disable specular -Sticks
 *
 * Revision 2.11  2003/09/05 22:35:33  matt
 * Minor d3d star popping fix
 *
 * Revision 2.10  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.9  2003/08/16 03:52:24  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.8  2003/05/05 21:27:46  phreak
 * if "Show Background" is not set it FRED,
 * it used to show background bitmaps and suns
 * not anymore
 *
 * Revision 2.7  2003/05/05 20:13:07  phreak
 * minor fred bug fixed
 *
 * Revision 2.6  2003/05/03 16:58:40  phreak
 * background stars are now somewhat colored, just here as a sort of useless feature test
 *
 * Revision 2.5  2003/01/19 01:07:42  bobboau
 * redid the way glow maps are handled; you now must set a global variable before you render a poly that uses a glow map, then set it to -1 when you're done with it
 * fixed a few other misc bugs too
 *
 * Revision 2.4  2003/01/06 19:33:21  Goober5000
 * cleaned up some stuff with model_set_thrust and a commented Assert that
 * shouldn't have been
 * --Goober5000
 *
 * Revision 2.3  2002/10/19 19:29:29  bobboau
 * initial commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.2  2002/09/20 20:05:29  phreak
 * glare parser stuff in stars_init()
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/21 15:48:21  mharris
 * Changed "char *name" to "char name[]" since we modify the string (and
 * modifying a constant string breaks unix)
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 34    9/07/99 4:01p Dave
 * Fixed up a string.tbl paroblem (self destruct message). Make sure IPX
 * does everything properly (setting up address when binding). Remove
 * black rectangle background from UI_INPUTBOX.
 * 
 * 33    9/01/99 10:14a Dave
 * Pirate bob.
 * 
 * 32    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 31    8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 30    7/27/99 3:52p Dave
 * Make star drawing a bit more robust to help lame D3D cards.
 * 
 * 29    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 28    7/13/99 2:01p Dave
 * Don't draw background bitmaps in the nebula.
 * 
 * 27    6/08/99 2:34p Jasenw
 * Made perspective bitmaps render in Fred.
 * 
 * 26    6/04/99 1:18p Dave
 * Fixed briefing model rendering problems. Made show background option in
 * fred toggle nebula rendering.
 * 
 * 25    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 24    5/28/99 1:45p Dave
 * Fixed up perspective bitmap drawing.
 * 
 * 23    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 22    5/11/99 10:03a Dave
 * Put a bunch of stuff into tables.
 * 
 * 21    5/11/99 9:10a Dave
 * Move default sun position.
 * 
 * 20    5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 19    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 18    4/25/99 7:43p Dave
 * Misc small bug fixes. Made sun draw properly.
 * 
 * 17    4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 16    4/07/99 6:22p Dave
 * Fred and FreeSpace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 15    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 14    3/20/99 5:09p Dave
 * Fixed release build fred warnings and unhandled exception.
 * 
 * 13    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 12    3/20/99 2:04p Dave
 * Removed unnecessary planet rendering.
 * 
 * 11    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 12    3/15/99 6:45p Daveb
 * Put in rough nebula bitmap support.
 * 
 * 11    3/11/99 5:53p Dave
 * More network optimization. Spliced in Dell OEM planet bitmap crap.
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 8     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 7     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 6     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 5     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 4     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/07/98 11:16a Dave
 * Remove warning.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 106   5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 105   5/13/98 2:53p John
 * Made subspace effect work under software.  Had to add new inner loop to
 * tmapper.  Added glows to end of subspace effect.  Made subspace effect
 * levels use gamepalette-subspace palette.
 * 
 * 104   5/13/98 10:28a John
 * made subpsace forward sliding 40% faster.
 * 
 * 103   5/10/98 4:18p John
 * Reversed the subspace effect direction
 * 
 * 102   5/08/98 8:38p John
 * Subspace tweaks.  Made two layers rotate independentyl.  
 * 
 * 101   5/08/98 1:32p John
 * Added code for using two layered subspace effects.
 * 
 * 100   5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 99    4/22/98 4:09p John
 * String externalization
 * 
 * 98    4/22/98 3:28p John
 * Fixed XSTR bug
 * 
 * 97    4/13/98 4:54p John
 * Made uv rotate independently on subspace effect. Put in DCF function
 * for setting subspace speeds.
 * 
 * 96    4/12/98 5:55p John
 * Made models work with subspace.  Made subspace rotate also.
 * 
 * 95    4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 94    4/08/98 11:31a Dave
 * AL: Fix syntax error for non-demo
 * 
 * 93    4/08/98 10:46a Lawrance
 * #ifdef out asteroid check for demo
 * 
 * 92    4/08/98 9:25a John
 * Made asteroid missions not show suns
 * 
 * 91    4/07/98 4:17p John
 * Made Fred be able to move suns.  Made suns actually affect the lighting
 * in the game.
 * 
 * 90    4/07/98 11:19a Hoffoss
 * Changed code to only use Sun01 for a bitmap by default, as John
 * requested.
 *
 * $NoKeywords: $
 */

#include <limits.h>

#include "math/vecmat.h"
#include "render/3d.h"
#include "starfield/starfield.h"
#include "freespace2/freespace.h"	
#include "io/timer.h"
#include "starfield/nebula.h"
#include "lighting/lighting.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "starfield/supernova.h"
#include "cmdline/cmdline.h"
#include "parse/parselo.h"
#include "hud/hud.h"
#include "hud/hudtarget.h"


#define MAX_DEBRIS_VCLIPS	4
#define DEBRIS_ROT_MIN				10000
#define DEBRIS_ROT_RANGE			8
#define DEBRIS_ROT_RANGE_SCALER	10000
#define RND_MAX_MASK	0x3fff
#define HALF_RND_MAX 0x2000

typedef struct {
	vec3d pos;
	vec3d last_pos;
	int active;
	int vclip;
	float size;	
} old_debris;

const int MAX_DEBRIS = 200;
const int MAX_STARS = 2000;
const float MAX_DIST = 50.0f;
const float MAX_DIST_RANGE = 60.0f;
const float MIN_DIST_RANGE = 14.0f;
const float BASE_SIZE = 0.12f;
float BASE_SIZE_NEB = 0.5f;

static int Subspace_model_inner = -1;
static int Subspace_model_outer = -1;
static int Rendering_to_env = 0;

poly_list perspective_bitmap_list;
int perspective_bitmap_buffer = -1;

int Num_stars = 500;
fix starfield_timestamp = 0;

#define MAX_FLARE_COUNT 10
#define MAX_FLARE_BMP 6

typedef struct flare_info {
	float pos;
	float scale;
	int tex_num;
} flare_info;

typedef struct flare_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;
} flare_bitmap;


// global info (not individual instances)
typedef struct starfield_bitmap {
	char filename[MAX_FILENAME_LEN];					// bitmap filename
	char glow_filename[MAX_FILENAME_LEN];				// only for suns
	int bitmap_id;										// bitmap handle
	int n_frames;
	int fps;
	int glow_bitmap;									// only for suns
	int glow_n_frames;
	int glow_fps;
	int xparent;	
	float r, g, b, i, spec_r, spec_g, spec_b;			// only for suns
	int glare;											// only for suns
	int flare;											// Is there a lens-flare for this sun?
	flare_info flare_infos[MAX_FLARE_COUNT];			// each flare can use a texture in flare_bmp, with different scale
	flare_bitmap flare_bitmaps[MAX_FLARE_BMP];			// bitmaps for different lens flares (can be re-used)
	int n_flares;										// number of flares actually used
	int n_flare_bitmaps;								// number of flare bitmaps available
	int used_this_level;
	int preload;
} starfield_bitmap;

// starfield bitmap instance
typedef struct starfield_bitmap_instance {
	float scale_x, scale_y;							// x and y scale
	int div_x, div_y;								// # of x and y divisions
	angles ang;										// angles from FRED
	int n_prim;										// number of primitives in buffer
	int star_bitmap_index;							// index into starfield_bitmap array
	ushort *buffer;

	starfield_bitmap_instance() { memset(this, 0, sizeof(starfield_bitmap_instance)); star_bitmap_index = -1; };
} starfield_bitmap_instance;

// for drawing cool stuff on the background - comes from a table
static std::vector<starfield_bitmap> Starfield_bitmaps;
static std::vector<starfield_bitmap_instance> Starfield_bitmap_instances;

// sun bitmaps and sun glow bitmaps
static std::vector<starfield_bitmap> Sun_bitmaps;
static std::vector<starfield_bitmap_instance> Suns;

// Goober5000
int Num_backgrounds = 0;
int Cur_background = -1;
background_t Backgrounds[MAX_BACKGROUNDS];

int last_stars_filled = 0;
color star_colors[8];
color star_aacolors[8];

typedef struct star {
	vec3d pos;
	vec3d last_star_pos;
	color col;
} star;

typedef struct vDist {
	int x;
	int y;
} vDist;

star Stars[MAX_STARS];

old_debris odebris[MAX_DEBRIS];


typedef struct debris_vclip {
	int	bm;
	int	nframes;
	char  name[MAX_FILENAME_LEN];
} debris_vclip;
extern debris_vclip Debris_vclips_normal[];
extern debris_vclip Debris_vclips_nebula[];
extern debris_vclip *Debris_vclips;

//XSTR:OFF
debris_vclip Debris_vclips_normal[MAX_DEBRIS_VCLIPS] = { { -1, -1, "debris01" }, { -1, -1, "debris02" }, { -1, -1, "debris03" }, { -1, -1, "debris04" } };
debris_vclip Debris_vclips_nebula[MAX_DEBRIS_VCLIPS] = { { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" } };
debris_vclip *Debris_vclips = Debris_vclips_normal;
//XSTR:ON

int stars_debris_loaded = 0;	// 0 = not loaded, 1 = normal vclips, 2 = nebula vclips

// background data
int Stars_background_inited = 0;			// if we're inited

int Nmodel_num = -1;							// model num
int Nmodel_flags = DEFALT_NMODEL_FLAGS;							// model flags
int Nmodel_bitmap = -1;						// model texture

int Num_debris_normal = 0;
int Num_debris_nebula = 0;


void stars_release_debris_vclips(debris_vclip *vclips)
{
	int i;

	if (vclips == NULL)
		return;

	for (i = 0; i < MAX_DEBRIS_VCLIPS; i++) {
		if ( (vclips[i].bm >= 0) && bm_release(vclips[i].bm) ) {
			vclips[i].bm = -1;
			vclips[i].nframes = -1;
		}
	}
}

void stars_load_debris_vclips(debris_vclip *vclips)
{
	int i;

	if (vclips == NULL) {
		Int3();
		return;
	}

	for (i = 0; i < MAX_DEBRIS_VCLIPS; i++) {
		vclips[i].bm = bm_load_animation( vclips[i].name, &vclips[i].nframes, NULL, 1 );

		if ( vclips[i].bm < 0 ) {
			// try loading it as a single bitmap
			vclips[i].bm = bm_load(Debris_vclips[i].name);
			vclips[i].nframes = 1;

			if (vclips[i].bm <= 0) {
				mprintf(("Couldn't load motion debris animation/bitmap '%s'\n", vclips[i].name ));
			}
		}
	}
}

void stars_load_debris(int fullneb = 0)
{
	if (Cmdline_nomotiondebris) {
		return;
	}

	// if we're in nebula mode
	if ( fullneb && (stars_debris_loaded != 2) ) {
		stars_release_debris_vclips(Debris_vclips);
		stars_load_debris_vclips(Debris_vclips_nebula);
		Debris_vclips = Debris_vclips_nebula;
		stars_debris_loaded = 2;
	} else if (stars_debris_loaded != 1) {
		stars_release_debris_vclips(Debris_vclips);
		stars_load_debris_vclips(Debris_vclips_normal);
		Debris_vclips = Debris_vclips_normal;
		stars_debris_loaded = 1;
	}
}

// allocates the poly list for the current decal list
void perspective_bitmap_allocate_poly_list(int x, int y)
{
	perspective_bitmap_list.allocate(x*y*6);
}

static void starfield_kill_bitmap_buffer()
{
	//don't bother with the buffers in HTL
	if (Cmdline_nohtl)
		return;

	if (perspective_bitmap_buffer != -1) {
		gr_destroy_buffer(perspective_bitmap_buffer);
		perspective_bitmap_buffer = -1;
	}
}

// draw a perspective bitmap based on angles and radius
#define MAX_PERSPECTIVE_DIVISIONS			5
void stars_project_2d_onto_sphere( vec3d *pnt, float rho, float phi, float theta );
static void starfield_create_perspective_bitmap_buffer(angles *a, float scale_x, float scale_y, int div_x, int div_y, uint tmap_flags, int env, ushort *index_buffer)
{
	float p_phi = 10.0f;
	float p_theta = 10.0f;

	vec3d s_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
	vec3d t_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];

	vertex v;
	matrix m, m_bank;
	int idx, s_idx;	
	float ui, vi;	
	angles bank_first;
	
	//don't bother with the buffers in HTL
	if (Cmdline_nohtl)
		return;

	Assert( index_buffer != NULL );

	// cap division values
	div_x = div_x > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_x;
//	div_x = 1;
	div_y = div_y > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_y;	

	// texture increment values
	ui = 1.0f / (float)div_x;
	vi = 1.0f / (float)div_y;	

	// adjust for aspect ratio
	scale_x *= (gr_screen.clip_aspect + 0.55f);		// fudge factor

	float s_phi = 0.5f + (((p_phi * scale_x) / 360.0f) / 2.0f);
	float s_theta = (((p_theta * scale_y) / 360.0f) / 2.0f);	
	float d_phi = -(((p_phi * scale_x) / 360.0f) / (float)(div_x));
	float d_theta = -(((p_theta * scale_y) / 360.0f) / (float)(div_y));

	// bank matrix
	bank_first.p = 0.0f;
	bank_first.b = a->b;
	bank_first.h = 0.0f;
	vm_angles_2_matrix(&m_bank, &bank_first);

	// convert angles to matrix
	float b_save = a->b;
	a->b = 0.0f;
	vm_angles_2_matrix(&m, a);
	a->b = b_save;	

	// generate the bitmap points	
	for(idx=0; idx<=div_x; idx++){
		for(s_idx=0; s_idx<=div_y; s_idx++){				
			// get world spherical coords			
			stars_project_2d_onto_sphere(&s_points[idx][s_idx], 1000.0f, s_phi + ((float)idx*d_phi), s_theta + ((float)s_idx*d_theta));			
			
			// bank the bitmap first
			vm_vec_rotate(&t_points[idx][s_idx], &s_points[idx][s_idx], &m_bank);

			// rotate on the sphere
			vm_vec_rotate(&s_points[idx][s_idx], &t_points[idx][s_idx], &m);					
		}
	}		

	int start = perspective_bitmap_list.n_verts;
	int s = 0;
	// render all polys
	/*
						y_dim		y_dim*2				y_dim*n
1	(0*y_dim+0)----(1*y_dim+0)----(2*y_dim+0)-- ... --(y_dim*n+0)
	  |					  |
	  |					  |
2	(0*y_dim+1)----(1*y_dim+1)----(2*y_dim+1)-- ... --(y_dim*n+1)
	  |					  |
	  |					  |
3	(0*y_dim+2)----(1*y_dim+2)----(2*y_dim+2)-- ... --(y_dim*n+2)
	  |					  |					  |
	  .					  .					  .
	  .					  .					  .
	  |					  |					  |
	(y_dim-1)------(1*y_dim*2-1)---(y_dim*3-1)-- ... --(y_dim*(n+1)-1)

  (x_pos*y_dim+y_pos) until (y_pos = y_dim)

  */
	for(idx=0; idx<div_x+1; idx++){
		for(s_idx=0; s_idx<div_y+1; s_idx++){						
			// stuff texture coords

			v.u = ui * float(idx);
			v.v = vi * float(s_idx);
			v.spec_r=v.spec_g=v.spec_b=0;
			g3_transfer_vertex(&v, &s_points[idx][s_idx]);
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts++] = v;

			if((idx != div_x) && (s_idx != div_y)){

			//	memset(&index_buffer[s], 0, sizeof(short) * 6);

				index_buffer[s++] = ushort((idx		* (div_y + 1)) + s_idx +		start);	//0
				index_buffer[s++] = ushort(((idx+1)	* (div_y + 1)) + s_idx +		start);	//2
				index_buffer[s++] = ushort(((idx+1)	* (div_y + 1)) + s_idx + 1 +	start);	//3

				index_buffer[s++] = ushort((idx		* (div_y + 1)) + s_idx +		start);	//0
				index_buffer[s++] = ushort(((idx+1)	* (div_y + 1)) + s_idx + 1 +	start);	//3
				index_buffer[s++] = ushort((idx		* (div_y + 1)) + s_idx + 1 +	start);	//1		

			}

/*			v[0].u = ui * float(idx);
			v[0].v = vi * float(s_idx);
			v[0].spec_r=v[2].spec_g=v[3].spec_b=0;
			
			v[1].u = ui * float(idx+1);
			v[1].v = vi * float(s_idx);
			v[1].spec_r=v[2].spec_g=v[3].spec_b=0;

			v[2].u = ui * float(idx+1);
			v[2].v = vi * float(s_idx+1);
			v[2].spec_r=v[2].spec_g=v[3].spec_b=0;


			v[3].u = ui * float(idx);
			v[3].v = vi * float(s_idx+1);
			v[3].spec_r=v[2].spec_g=v[3].spec_b=0;

			v[0].flags = 0;
			v[1].flags = 0;
			v[2].flags = 0;
			v[3].flags = 0;
			verts[0] = &v[0];
			verts[1] = &v[1];
			verts[2] = &v[2];
			verts[3] = &v[3];

			g3_transfer_vertex(verts[0], &s_points[idx][s_idx]);			
			g3_transfer_vertex(verts[1], &s_points[idx+1][s_idx]);			
			g3_transfer_vertex(verts[2], &s_points[idx+1][s_idx+1]);						
			g3_transfer_vertex(verts[3], &s_points[idx][s_idx+1]);						

			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts] = *verts[0];
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts + 1] = *verts[1];
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts + 2] = *verts[2];
			perspective_bitmap_list.n_verts += 3;
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts] = *verts[0];
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts + 1] = *verts[2];
			perspective_bitmap_list.vert[perspective_bitmap_list.n_verts + 2] = *verts[3];
			perspective_bitmap_list.n_verts += 3;
*/
		}
	}
	start += perspective_bitmap_list.n_verts;
//	Assert(perspective_bitmap_list.n_verts == div_x * div_y * 6);

	SAFEPOINT("buffer filled, creating");

}

static void starfield_update_index_buffers(int index, int nverts)
{
	Assert( (index >= 0) && (index < (int)Starfield_bitmap_instances.size()) );

	if ( (index < 0) || (index >= (int)Starfield_bitmap_instances.size()) )
		return;

	bool change_up = false;

	change_up = ((nverts <= 0) || (nverts != (Starfield_bitmap_instances[index].n_prim * 3)));

	if ( change_up && (Starfield_bitmap_instances[index].buffer != NULL) ) {
		vm_free(Starfield_bitmap_instances[index].buffer);
		Starfield_bitmap_instances[index].buffer = NULL;
	}

	if (nverts <= 0) {
		Starfield_bitmap_instances[index].n_prim = 0;
		return;
	}

	if (Starfield_bitmap_instances[index].buffer == NULL) {
		Starfield_bitmap_instances[index].buffer = (ushort*) vm_malloc( nverts * sizeof(ushort) );
	}

	Starfield_bitmap_instances[index].n_prim = nverts / 3;
}
	
// take the Starfield_bitmap_instances[] and make all the vertex buffers that you'll need to draw it 
static void starfield_generate_bitmap_instance_buffers()
{
	//don't bother with the buffers in HTL
	if (Cmdline_nohtl)
		return;

	SAFEPOINT("entering starfield_generate_bitmap_instance_vertex_buffers");

	int idx, vert_count = 0;

	for (idx = 0; idx < (int)Starfield_bitmap_instances.size(); idx++) {
		// make sure it's usable
		if (Starfield_bitmap_instances[idx].star_bitmap_index < 0)
			continue;

		int nverts = Starfield_bitmap_instances[idx].div_x * Starfield_bitmap_instances[idx].div_y * 6;
		vert_count += nverts * 2;

		starfield_update_index_buffers(idx, nverts);
	}

	perspective_bitmap_list.allocate(vert_count);
	perspective_bitmap_list.n_verts = 0;

	for (idx = 0; idx < (int)Starfield_bitmap_instances.size(); idx++) {
		// make sure it's usable
		if (Starfield_bitmap_instances[idx].star_bitmap_index < 0)
			continue;

		if (Starfield_bitmap_instances[idx].buffer != NULL) {
			starfield_create_perspective_bitmap_buffer(&Starfield_bitmap_instances[idx].ang, Starfield_bitmap_instances[idx].scale_x,
					Starfield_bitmap_instances[idx].scale_y, Starfield_bitmap_instances[idx].div_x, Starfield_bitmap_instances[idx].div_y,
					TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_FLAG_XPARENT, 0, Starfield_bitmap_instances[idx].buffer);
		}
	}

	if (perspective_bitmap_list.n_verts == 0)
		return;

	starfield_kill_bitmap_buffer();

	perspective_bitmap_buffer = gr_make_buffer(&perspective_bitmap_list, VERTEX_FLAG_POSITION | VERTEX_FLAG_UV1);
	
	SAFEPOINT("leaving starfield_generate_bitmap_instance_vertex_buffers");
}

static void starfield_bitmap_entry_init(starfield_bitmap *sbm)
{
	int i;

	Assert( sbm != NULL );

	memset( sbm, 0, sizeof(starfield_bitmap) );

	sbm->bitmap_id = -1;
	sbm->glow_bitmap = -1;		
	sbm->glow_n_frames = 1;

	for (i = 0; i < MAX_FLARE_BMP; i++) {
		sbm->flare_bitmaps[i].bitmap_id = -1;
	}

	for (i = 0; i < MAX_FLARE_COUNT; i++) {
		sbm->flare_infos[i].tex_num = -1;
	}
}

void parse_startbl(char *longname)
{
	char filename[MAX_FILENAME_LEN], tempf[16];
	starfield_bitmap sbm;
	int idx;

	int rval;
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'.  Code = %i.\n", longname, rval));
		return;
	} 

	read_file_text(longname);
	reset_parse();

	while(!optional_string("#end"))
	{
		starfield_bitmap_entry_init( &sbm );
		
		// intensity alpha bitmap
		if(optional_string("$Bitmap:"))
		{
			stuff_string(sbm.filename, F_NAME, MAX_FILENAME_LEN);
			sbm.xparent = 0;

			if ( (idx = stars_find_bitmap(sbm.filename)) >= 0 ) {
				if (sbm.xparent == Starfield_bitmaps[idx].xparent) {
					if ( !Parsing_modular_table )
						Warning(LOCATION, "Starfield bitmap '%s' listed more than once!!  Only using the first entry!", sbm.filename);
				} else {
					Warning(LOCATION, "Starfield bitmap '%s' already listed as a xparent bitmap!!  Only using the xparent version!", sbm.filename);
				}
			} else {
				Starfield_bitmaps.push_back(sbm);
			}
		}
		// green xparency bitmap
		else if(optional_string("$BitmapX:"))
		{
			stuff_string(sbm.filename, F_NAME, MAX_FILENAME_LEN);
			sbm.xparent = 1;

			if ( (idx = stars_find_bitmap(sbm.filename)) >= 0 ) {
				if (sbm.xparent == Starfield_bitmaps[idx].xparent) {
					if ( !Parsing_modular_table )
						Warning(LOCATION, "Starfield bitmap '%s' listed more than once!!  Only using the first entry!", sbm.filename);
				} else {
					Warning(LOCATION, "Starfield xparent bitmap '%s' already listed as a non-xparent bitmap!!  Only using the non-xparent version!", sbm.filename);
				}
			} else {
				Starfield_bitmaps.push_back(sbm);
			}
		} else {
			if (Parsing_modular_table) {
				break;
			} else {
				Warning(LOCATION, "Unknown section in \"%s\"!  Going to skip it...\n", longname);
				break;
			}
		}
	}

	// sun bitmaps
	while(!optional_string("#end"))
	{
		if(optional_string("$Sun:"))
		{
			starfield_bitmap_entry_init( &sbm );

			stuff_string(sbm.filename, F_NAME, MAX_FILENAME_LEN);

			// associated glow
			required_string("$Sunglow:");
			stuff_string(sbm.glow_filename, F_NAME, MAX_FILENAME_LEN);

			// associated lighting values
			required_string("$SunRGBI:");
			stuff_float(&sbm.r);
			stuff_float(&sbm.g);
			stuff_float(&sbm.b);
			stuff_float(&sbm.i);

			if (optional_string("$SunSpecularRGB:")) {
				stuff_float(&sbm.spec_r);
				stuff_float(&sbm.spec_g);
				stuff_float(&sbm.spec_b);
			} else {
				sbm.spec_r = sbm.r;
				sbm.spec_g = sbm.g;
				sbm.spec_b = sbm.b;
			}

			// lens flare stuff
			if (optional_string("$Flare:"))
			{
				sbm.flare = 1;

				required_string("+FlareCount:");
				stuff_int(&sbm.n_flares);

				// if there's a flare, it has to have at least one texture
				required_string("$FlareTexture1:");
				stuff_string(sbm.flare_bitmaps[0].filename, F_NAME, MAX_FILENAME_LEN);

				sbm.n_flare_bitmaps = 1;

				for (idx = 1; idx < MAX_FLARE_BMP; idx++) {
					// allow 9999 textures (theoretically speaking, that is)
					sprintf(tempf, "$FlareTexture%d:", idx+1);

					if (optional_string(tempf)) {
						sbm.n_flare_bitmaps++;
						stuff_string(sbm.flare_bitmaps[idx].filename, F_NAME, MAX_FILENAME_LEN);
					}
				//	else break; //don't allow flaretexture1 and then 3, etc.
				}

				required_string("$FlareGlow1:");

				required_string("+FlareTexture:");
				stuff_int(&sbm.flare_infos[0].tex_num);

				required_string("+FlarePos:");
				stuff_float(&sbm.flare_infos[0].pos);

				required_string("+FlareScale:");
				stuff_float(&sbm.flare_infos[0].scale);
				
				sbm.n_flares = 1;

				for (idx = 1; idx < MAX_FLARE_COUNT; idx++) {
					// allow a lot of glows
					sprintf(tempf, "$FlareGlow%d:", idx+1);

					if (optional_string(tempf)) {
						sbm.n_flares++;

						required_string("+FlareTexture:");
						stuff_int(&sbm.flare_infos[idx].tex_num);

						required_string("+FlarePos:");
						stuff_float(&sbm.flare_infos[idx].pos);

						required_string("+FlareScale:");
						stuff_float(&sbm.flare_infos[idx].scale);
					}
				//	else break; //don't allow "flare 1" and then "flare 3"
				}
			}

			sbm.glare = !optional_string("$NoGlare:");

			sbm.xparent = 1;

			if ( (idx = stars_find_sun(sbm.filename)) >= 0 ) {
				if ( !Parsing_modular_table )
					Warning(LOCATION, "Sun bitmap '%s' listed more than once!!  Only using the first entry!", sbm.filename);
			} else {
				Sun_bitmaps.push_back(sbm);
			}
		} else {
			if (Parsing_modular_table) {
				break;
			} else {
				Warning(LOCATION, "Unknown section in \"%s\"!  Going to skip it...\n", longname);
				break;
			}
		}
	}

	//Don't parse motion debris if we don't have to.
	if(Cmdline_nomotiondebris)
	{
		return;
	}

	// normal debris pieces
	while(!optional_string("#end"))
	{
		if (Parsing_modular_table && !optional_string("$Debris:")) {
				break;
		} else {
			required_string("$Debris:");
		}

		stuff_string(filename, F_NAME, MAX_FILENAME_LEN);

		if(Num_debris_normal < MAX_DEBRIS_VCLIPS){
			strcpy(Debris_vclips_normal[Num_debris_normal++].name, filename);
		} else {
			Warning(LOCATION, "Could not load normal motion debris '%s'; maximum of %d exceeded.", MAX_DEBRIS_VCLIPS);
		}
	}

	// nebula debris pieces
	while(!optional_string("#end"))
	{
		if (Parsing_modular_table && !optional_string("$DebrisNeb:")) {
			break;
		} else {
			required_string("$DebrisNeb:");
		}

		stuff_string(filename, F_NAME, MAX_FILENAME_LEN);

		if(Num_debris_nebula < MAX_DEBRIS_VCLIPS){
			strcpy(Debris_vclips_nebula[Num_debris_nebula++].name, filename);
		} else {
			Warning(LOCATION, "Could not load nebula motion debris '%s'; maximum of %d exceeded.", MAX_DEBRIS_VCLIPS);
		}
	}
}

void stars_load_all_bitmaps()
{
	int idx, i;
	starfield_bitmap *sb = NULL;
	static int Star_bitmaps_loaded = 0;

	if (Star_bitmaps_loaded)
		return;

	// pre-load all starfield bitmaps.  ONLY SHOULD DO THIS FOR FRED!!
	// this can get nasty when a lot of bitmaps are in use so spare it for
	// the normal game and only do this in FRED
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		sb = &Starfield_bitmaps[idx];

		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", sb->filename);
				}
			}
		}
	}

	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		sb = &Sun_bitmaps[idx];

		// normal bitmap
		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// glow bitmap
		if (sb->glow_bitmap < 0) {
			sb->glow_bitmap = bm_load(sb->glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, 1);

				if (sb->glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
				}
			}
		}

		if (sb->flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if ( !strlen(sb->flare_bitmaps[i].filename) )
					continue;

				if (sb->flare_bitmaps[i].bitmap_id < 0) {
					sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
						continue;
					}
				}
			}
		}
	}

	Star_bitmaps_loaded = 1;
}

void stars_clear_instances()
{
	starfield_kill_bitmap_buffer();

	for (uint i = 0; i < Starfield_bitmap_instances.size(); i++) {
		starfield_update_index_buffers(i, 0);
	}

	Starfield_bitmap_instances.clear();
	Suns.clear();
}

// call on game startup
void stars_init()
{
	// starfield bitmaps
	Num_debris_normal = 0;
	Num_debris_nebula = 0;

	// parse stars.tbl
	parse_startbl("stars.tbl");

	parse_modular_table("*-str.tbm", parse_startbl);
}

// call only from game_shutdown()!!
void stars_close()
{
	stars_clear_instances();

	// any other code goes here
}

// called before mission parse so we can clear out all of the old stuff
void stars_pre_level_init(bool clear_backgrounds)
{
	uint idx, i;
	starfield_bitmap *sb = NULL;

	if (clear_backgrounds)
	{
		// Goober5000 - clear entire array, including backgrounds that will be unused
		for (i = 0; i < MAX_BACKGROUNDS; i++)
		{
			Backgrounds[i].suns.clear();
			Backgrounds[i].bitmaps.clear();
		}
	}

	stars_clear_instances();

	stars_set_background_model(NULL, NULL);

	// mark all starfield and sun bitmaps as unused for this mission and release any current bitmaps
	// NOTE: that because of how we have to load the bitmaps it's important to release all of
	//       them first thing rather than after we have marked and loaded only what's needed
	// NOTE2: there is a reason that we don't check for release before setting the handle to -1 so
	//        be aware that this is NOT a bug. also, bmpman should NEVER return 0 as a valid handle!
	if ( !Fred_running ) {
		for (idx = 0; idx < Starfield_bitmaps.size(); idx++) {
			sb = &Starfield_bitmaps[idx];

			if (sb->bitmap_id > 0) {
				bm_release(sb->bitmap_id);
				sb->bitmap_id = -1;
			}

			sb->used_this_level = 0;
			sb->preload = 0;
		}

		for (idx = 0; idx < Sun_bitmaps.size(); idx++) {
			sb = &Sun_bitmaps[idx];

			if (sb->bitmap_id > 0) {
				bm_release(sb->bitmap_id);
				sb->bitmap_id = -1;
			}

			if (sb->glow_bitmap > 0) {
				bm_release(sb->glow_bitmap);
				sb->glow_bitmap = -1;
			}

			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if (sb->flare_bitmaps[i].bitmap_id > 0) {
					bm_release(sb->flare_bitmaps[i].bitmap_id);
					sb->flare_bitmaps[i].bitmap_id = -1;
				}
			}

			sb->used_this_level = 0;
			sb->preload = 0;
		}
	}
}

// call this in game_post_level_init() so we know whether we're running in full nebula mode or not
void stars_post_level_init()
{
	int i;
	vec3d v;
	float dist, dist_max;
	ubyte red,green,blue,alpha;

	// reset to -1 so we reload it each mission (if we need to)
	Nmodel_num = -1;		
	Nmodel_flags = DEFALT_NMODEL_FLAGS;	
	if(Nmodel_bitmap != -1){
		bm_release(Nmodel_bitmap);
		Nmodel_bitmap = -1;
	}

	stars_set_background_model(The_mission.skybox_model, "", The_mission.skybox_flags);

	stars_load_debris( ((The_mission.flags & MISSION_FLAG_FULLNEB) || Nebula_sexp_used) );

// following code randomly distributes star points within a sphere volume, which
// avoids there being denser areas along the edges and in corners that we had in the
// old rectangular distribution scheme.
	dist_max = (float) (HALF_RND_MAX * HALF_RND_MAX);
	for (i=0; i<MAX_STARS; i++) {
		dist = dist_max;
		while (dist >= dist_max) {
			v.xyz.x = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);
			v.xyz.y = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);
			v.xyz.z = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);

			dist = v.xyz.x * v.xyz.x + v.xyz.y * v.xyz.y + v.xyz.z * v.xyz.z;
		}
		vm_vec_copy_normalize(&Stars[i].pos, &v);

		{
			red= (ubyte)(myrand() % 63 +192);		//192-255
			green= (ubyte)(myrand() % 63 +192);		//192-255
			blue= (ubyte)(myrand() % 63 +192);		//192-255
			alpha = (ubyte)(myrand () % 192 + 24);	//24-216

			gr_init_alphacolor(&Stars[i].col, red, green, blue, alpha, AC_TYPE_BLEND);
		}

	}

	memset( &odebris, 0, sizeof(old_debris) * MAX_DEBRIS );

	
	for (i=0; i<8; i++ )	{
		ubyte intensity = (ubyte)((i + 1) * 24);
		gr_init_alphacolor(&star_aacolors[i], 255, 255, 255, intensity, AC_TYPE_BLEND );
		gr_init_color(&star_colors[i], intensity, intensity, intensity );
	}

	last_stars_filled = 0;

	// if we have no sun instances, create one
	if ( !Suns.size() ) {
		if ( !Sun_bitmaps.size() || !strlen(Sun_bitmaps[0].filename) ) {
			mprintf(("Trying to add default sun but no default exists!!\n"));
		} else {
			mprintf(("Adding default sun.\n"));

			starfield_bitmap_instance def_sun;
		//	memset( &def_sun, 0, sizeof(starfield_bitmap_instance) );

			// stuff some values
			def_sun.star_bitmap_index = 0;
			def_sun.scale_x = 1.0f;
			def_sun.scale_y = 1.0f;
			def_sun.div_x = 1;
			def_sun.div_y = 1;
			def_sun.ang.h = fl_radian(60.0f);

			Suns.push_back(def_sun);
		}
	}

	// FRED doesn't do normal page_in stuff so we need to load up the bitmaps here instead
	if (Fred_running) {
		stars_load_all_bitmaps();
	}

	starfield_generate_bitmap_instance_buffers();
}


extern object * Player_obj;

#define STAR_AMOUNT_DEFAULT 0.75f
#define STAR_DIM_DEFAULT 7800.0f
#define STAR_CAP_DEFAULT 75.0f
#define STAR_MAX_LENGTH_DEFAULT 0.04f		// 312

float Star_amount = STAR_AMOUNT_DEFAULT;
float Star_dim = STAR_DIM_DEFAULT;
float Star_cap = STAR_CAP_DEFAULT;
float Star_max_length = STAR_MAX_LENGTH_DEFAULT;	

#define STAR_FLAG_TAIL			(1<<0)	// Draw a tail when moving
#define STAR_FLAG_DIM			(1<<1)	// Dim as you move
#define STAR_FLAG_ANTIALIAS	(1<<2)	// Draw the star using antialiased lines
#define STAR_FLAG_DEFAULT		(STAR_FLAG_TAIL | STAR_FLAG_DIM)

uint Star_flags = STAR_FLAG_DEFAULT;

//XSTR:OFF
DCF(stars,"Set parameters for starfield")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "tail" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Star_amount = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "len" ))	{
			dc_get_arg(ARG_FLOAT);
			Star_max_length = Dc_arg_float;
		} else if ( !strcmp( Dc_arg, "dim" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				Star_dim = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "flag" ))	{
			dc_get_arg(ARG_STRING);
			if ( !strcmp( Dc_arg, "tail" ))	{
				Star_flags ^= STAR_FLAG_TAIL;
			} else if ( !strcmp( Dc_arg, "dim" ))	{
				Star_flags ^= STAR_FLAG_DIM;
			} else if ( !strcmp( Dc_arg, "aa" ))	{
				Star_flags ^= STAR_FLAG_ANTIALIAS;
			} else {
				Dc_help = 1;	
			}
		} else if ( !strcmp( Dc_arg, "cap" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 255.0f) )	{
				Dc_help = 1;
			} else {
				Star_cap = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "m0" )  )	{
			Star_amount = 0.0f;
			Star_dim = 0.0f;
			Star_cap = 0.0f;
			Star_flags = 0;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "m1" ) || !strcmp( Dc_arg, "default" ))	{
			Star_amount = STAR_AMOUNT_DEFAULT;
			Star_dim = STAR_DIM_DEFAULT;
			Star_cap = STAR_CAP_DEFAULT;
			Star_flags = STAR_FLAG_DEFAULT;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "m2" ))	{
			Star_amount = 0.75f;
			Star_dim = 20.0f;
			Star_cap = 75.0f;
			Star_flags = STAR_FLAG_TAIL|STAR_FLAG_DIM|STAR_FLAG_ANTIALIAS;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "num" ))	{
			dc_get_arg(ARG_INT);
			if ( (Dc_arg_int < 0) || (Dc_arg_int > MAX_STARS) )	{
				Dc_help = 1;
			} else {
				Num_stars = Dc_arg_int;
			} 
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: stars keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "stars default   Resets stars to all default values\n" );
		dc_printf( "stars num X     Sets number of stars to X.  Between 0 and %d.\n", MAX_STARS );
		dc_printf( "stars tail X    Where X is the percent of 'tail' between 0 and 1.0\n" );
		dc_printf( "stars dim X     Where X is the amount stars dim between 0 and 255.0\n" );
		dc_printf( "stars cap X     Where X is the cap of dimming between 0 and 255.\n" );
		dc_printf( "stars len X     Where X is the cap of length.\n" );
		dc_printf( "stars m0        Macro0. Old 'pixel type' crappy stars. flags=none\n" );
		dc_printf( "stars m1        Macro1. (default) tail=.75, dim=20.0, cap=75.0, flags=dim,tail\n" );
		dc_printf( "stars m2        Macro2. tail=.75, dim=20.0, cap=75.0, flags=dim,tail,aa\n" );
		dc_printf( "stars flag X    Toggles flag X, where X is tail or dim or aa (aa=antialias)\n" );
		dc_printf( "\nHINT: set cap to 0 to get dim rate and tail down, then use\n" );
		dc_printf( "cap to keep the lines from going away when moving too fast.\n" );
		dc_printf( "\nUse '? stars' to see current values.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Num_stars: %d\n", Num_stars );
		dc_printf( "Tail: %.2f\n", Star_amount );
		dc_printf( "Dim: %.2f\n", Star_dim );
		dc_printf( "Cap: %.2f\n", Star_cap );
		dc_printf( "Max length: %.2f\n", Star_max_length );
		dc_printf( "Flags:\n" );
		dc_printf( "  Tail: %s\n", (Star_flags&STAR_FLAG_TAIL?"On":"Off") );
		dc_printf( "  Dim: %s\n", (Star_flags&STAR_FLAG_DIM?"On":"Off") );
		dc_printf( "  Antialias: %s\n", (Star_flags&STAR_FLAG_ANTIALIAS?"On":"Off") );
		dc_printf( "\nTHESE AREN'T SAVED TO DISK, SO IF YOU TWEAK\n" );
		dc_printf( "THESE AND LIKE THEM, WRITE THEM DOWN!!\n" );
	}
}
//XSTR:ON

int reload_old_debris = 1;		// If set to one, then reload all the last_pos of the debris

// Call this if camera "cuts" or moves long distances
// so blur effect doesn't draw lines all over the screen.
void stars_camera_cut()
{
	last_stars_filled = 0;
	reload_old_debris = 1;
}

//#define TIME_STAR_CODE		// enable to time star code

extern int Sun_drew;
extern float Viewer_zoom;

// get the world coords of the sun pos on the unit sphere.
void stars_get_sun_pos(int sun_n, vec3d *pos)
{
	vec3d temp;
	matrix rot;

	// sanity
	Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// rotate the sun properly
	temp = vmd_zero_vector;
	temp.xyz.z = 1.0f;
	
	// rotation matrix
	vm_angles_2_matrix(&rot, &Suns[sun_n].ang);
	vm_vec_rotate(pos, &temp, &rot);
}

void stars_add_lights(){
	int idx;
	vec3d sun_pos;
	vec3d sun_dir;
	vertex sun_vex;	
	starfield_bitmap *bm;
	float local_scale = 1.0f;

	// draw all suns
	for (idx = 0; idx < (int)Suns.size(); idx++) {
		// get the instance
		if (Suns[idx].star_bitmap_index < 0)
			return;

		bm = &Sun_bitmaps[Suns[idx].star_bitmap_index];

		// if no bitmap then bail...
		if (bm->bitmap_id < 0)
			continue;

		memset( &sun_vex, 0, sizeof(vertex) );

		// get sun pos
		sun_pos = vmd_zero_vector;
		sun_pos.xyz.y = 1.0f;
		stars_get_sun_pos(idx, &sun_pos);
		
		// get the direction		
		sun_dir = sun_pos;
		vm_vec_normalize(&sun_dir);

		// add the light source corresponding to the sun, except when rendering to an envmap
//		if ( !Rendering_to_env )
			light_add_directional(&sun_dir, bm->i, bm->r, bm->g, bm->b, bm->spec_r, bm->spec_g, bm->spec_b, true);

	}
}
// draw sun
void stars_draw_sun(int show_sun)
{	
	int idx;
	vec3d sun_pos;
	vec3d sun_dir;
	vertex sun_vex;	
	starfield_bitmap *bm;
	float local_scale = 1.0f;

	// should we even be here?
	if (!show_sun)
		return;

	// no suns drew yet
	Sun_drew = 0;

	// draw all suns
	for (idx = 0; idx < (int)Suns.size(); idx++) {
		// get the instance
		if (Suns[idx].star_bitmap_index < 0)
			return;

		bm = &Sun_bitmaps[Suns[idx].star_bitmap_index];

		// if no bitmap then bail...
		if (bm->bitmap_id < 0)
			continue;

		memset( &sun_vex, 0, sizeof(vertex) );

		// get sun pos
		sun_pos = vmd_zero_vector;
		sun_pos.xyz.y = 1.0f;
		stars_get_sun_pos(idx, &sun_pos);
		
		// get the direction		
		sun_dir = sun_pos;
		vm_vec_normalize(&sun_dir);

		// add the light source corresponding to the sun, except when rendering to an envmap
//		if ( !Rendering_to_env )
//			light_add_directional(&sun_dir, bm->i, bm->r, bm->g, bm->b, bm->spec_r, bm->spec_g, bm->spec_b, true);

		// if supernova
		if ( supernova_active() )
			local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());

		// draw the sun itself, keep track of how many we drew
		if (bm->fps) {
			gr_set_bitmap(bm->bitmap_id + ((timestamp() / (int)(bm->fps) % bm->n_frames)), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
		} else {
			gr_set_bitmap(bm->bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
		}

		g3_rotate_faraway_vertex(&sun_vex, &sun_pos);

		if ( !g3_draw_bitmap(&sun_vex, 0, 0.05f * Suns[idx].scale_x * local_scale, TMAP_FLAG_TEXTURED) )
			Sun_drew++;
	}
}
// draw a star's lens-flare
void stars_draw_lens_flare(vertex *sun_vex, int sun_n)
{
	starfield_bitmap *bm;
	int i,j;
	float dx,dy;
	vertex flare_vex = *sun_vex; //copy over to flare_vex to get all sorts of properties

	Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// get the instance
	if (Suns[sun_n].star_bitmap_index < 0) {
		return;
	} else {
		bm = &Sun_bitmaps[Suns[sun_n].star_bitmap_index];
	}

	if (!bm->flare)
		return;
	
	dx = 2.0f*(i2fl(gr_screen.clip_right-gr_screen.clip_left)*0.5f - sun_vex->sx); // (dx,dy) is a 2d vector equal to two times the vector from the sun's position to the center fo the screen
	dy = 2.0f*(i2fl(gr_screen.clip_bottom-gr_screen.clip_top)*0.5f - sun_vex->sy); // meaning it is the vector to the opposite position on the screen

	for (j = 0; j < bm->n_flare_bitmaps; j++)
	{
		// if no bitmap then bail...
		if (bm->flare_bitmaps[j].bitmap_id < 0)
			continue;

		gr_set_bitmap(bm->flare_bitmaps[j].bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);

		for (i = 0; i < bm->n_flares; i++) {
			// draw sorted by texture, to minimize texture changes. not the most efficient way, but better than non-sorted
			if (bm->flare_infos[i].tex_num == j) {
//				gr_set_bitmap(bm->flare_bitmaps[bm->flare_infos[i].tex_num], GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
				flare_vex.sx = sun_vex->sx + dx * bm->flare_infos[i].pos;
				flare_vex.sy = sun_vex->sy + dy * bm->flare_infos[i].pos;
				g3_draw_bitmap(&flare_vex, 0, 0.05f * bm->flare_infos[i].scale, TMAP_FLAG_TEXTURED);
			}
		}
	}
}

// draw the corresponding glow for sun_n
void stars_draw_sun_glow(int sun_n)
{
	starfield_bitmap *bm;		
	vec3d sun_pos, sun_dir;
	vertex sun_vex;	
	float local_scale = 1.0f;

	// sanity
	Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// get the instance
	if (Suns[sun_n].star_bitmap_index < 0)
		return;

	bm = &Sun_bitmaps[Suns[sun_n].star_bitmap_index];

	// if no bitmap then bail...
	if (bm->glow_bitmap < 0)
		return;

	memset( &sun_vex, 0, sizeof(vertex) );

	// get sun pos
	sun_pos = vmd_zero_vector;
	sun_pos.xyz.y = 1.0f;
	stars_get_sun_pos(sun_n, &sun_pos);	

	// get the direction		
	sun_dir = sun_pos;
	vm_vec_normalize(&sun_dir);	

	// if supernova
	if ( supernova_active() )
		local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());

	// draw the sun itself, keep track of how many we drew
	if (bm->glow_fps) {
		gr_set_bitmap(bm->glow_bitmap + ((timestamp() / (int)(bm->glow_fps) % bm->glow_n_frames)), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.5f);
	} else {
		gr_set_bitmap(bm->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.5f);
	}

	g3_rotate_faraway_vertex(&sun_vex, &sun_pos);
	g3_draw_bitmap(&sun_vex, 0, 0.10f * Suns[sun_n].scale_x * local_scale, TMAP_FLAG_TEXTURED);

	if ( bm->flare && !(sun_vex.codes & CC_OFF) ) //if sun isn't off-screen, and is visible (since stars_draw_sun_glow() is called only if it is) then draw the lens-flare
		stars_draw_lens_flare(&sun_vex, sun_n);
}


// draw bitmaps
void stars_draw_bitmaps(int show_bitmaps)
{
	int idx;
	int star_index;

	// should we even be here?
	if ( !show_bitmaps )
		return;

	// if we're in the nebula, don't render any backgrounds
	if (The_mission.flags & MISSION_FLAG_FULLNEB)
		return;

	// detail settings
	if ( !Detail.planets_suns )
		return;

	if ( !Cmdline_nohtl ) {
		if (perspective_bitmap_buffer < 0)
			return;

		gr_set_lighting(false, false);
		gr_start_instance_matrix(&Eye_position, &vmd_identity_matrix);
	}

	// turn off culling
	gr_set_cull(0);

	// turn off zbuffering
	int saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	// render all bitmaps
	gr_set_buffer(perspective_bitmap_buffer);

	for (idx = 0; idx < (int)Starfield_bitmap_instances.size(); idx++) {
		// lookup the info index
		star_index = Starfield_bitmap_instances[idx].star_bitmap_index;

		if (star_index < 0) {
			continue;
		}

		// if no bitmap then bail...
		if (Starfield_bitmaps[star_index].bitmap_id < 0) {
			continue;
		}

		if (!Cmdline_nohtl) {
			if (Starfield_bitmap_instances[idx].buffer == NULL) {
				Int3();
				continue;
			}
		}

		if (Starfield_bitmaps[star_index].xparent) {
			if (Starfield_bitmaps[star_index].fps) {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id + ((timestamp() / (int)(Starfield_bitmaps[star_index].fps) % Starfield_bitmaps[star_index].n_frames)));		
			} else {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id);
			}

			if (Cmdline_nohtl) {
				g3_draw_perspective_bitmap(&Starfield_bitmap_instances[idx].ang, Starfield_bitmap_instances[idx].scale_x, Starfield_bitmap_instances[idx].scale_y, Starfield_bitmap_instances[idx].div_x, Starfield_bitmap_instances[idx].div_y, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_FLAG_XPARENT);
			} else {
				gr_render_buffer(0, Starfield_bitmap_instances[idx].n_prim, Starfield_bitmap_instances[idx].buffer);
			}
		} else {				
			if (Starfield_bitmaps[star_index].fps) {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id + ((timestamp() / (int)(Starfield_bitmaps[star_index].fps) % Starfield_bitmaps[star_index].n_frames)), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);	
			} else {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);	
			}
					
			if (Cmdline_nohtl) {
				g3_draw_perspective_bitmap(&Starfield_bitmap_instances[idx].ang, Starfield_bitmap_instances[idx].scale_x, Starfield_bitmap_instances[idx].scale_y, Starfield_bitmap_instances[idx].div_x, Starfield_bitmap_instances[idx].div_y, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT);
			} else {
				gr_render_buffer(0, Starfield_bitmap_instances[idx].n_prim, Starfield_bitmap_instances[idx].buffer);
			}
		}
	}

	// turn on culling
	gr_set_cull(1);

	// restore zbuffer
	gr_zbuffer_set(saved_zbuffer_mode);

	if ( !Cmdline_nohtl )
		gr_end_instance_matrix();
}

/*
void calculate_bitmap_matrix(starfield_bitmaps *bm, vec3d *v)
{
	vm_vector_2_matrix(&bm->m, v, NULL, NULL);
	vm_orthogonalize_matrix(&bm->m);
}

void calculate_bitmap_points(starfield_bitmaps *bm, float bank)
{
	int i;
	vec3d fvec, uvec, rvec, tmp;
	angles tangles;

	vm_orthogonalize_matrix(&bm->m);
	if (bank) {
		tangles.p = tangles.h = 0.0f;
		tangles.b = bank;
		vm_rotate_matrix_by_angles(&bm->m, &tangles);
	}

	fvec = bm->m.fvec;
	vm_vec_scale(&fvec, bm->dist );
	uvec = bm->m.uvec;
	rvec = bm->m.rvec;

	vm_vec_sub(&tmp, &fvec, &uvec);
	vm_vec_sub(&bm->points[3], &tmp, &rvec);

	vm_vec_sub(&tmp, &fvec, &uvec);
	vm_vec_add(&bm->points[2], &tmp, &rvec);

	vm_vec_add(&tmp, &fvec, &uvec);
	vm_vec_add(&bm->points[1], &tmp, &rvec);

	vm_vec_add(&tmp, &fvec, &uvec);
	vm_vec_sub(&bm->points[0], &tmp, &rvec);

	for (i=0; i<4; i++){
		vm_vec_normalize(&bm->points[i]);
	}
}
*/

extern int Interp_subspace;
extern float Interp_subspace_offset_u;
extern float Interp_subspace_offset_u;
extern float Interp_subspace_offset_v;

float subspace_offset_u = 0.0f;
float subspace_offset_u_inner = 0.0f;
float subspace_offset_v = 0.0f;

float subspace_u_speed = 0.07f;			// how fast u changes
float subspace_v_speed = 0.05f;			// how fast v changes

int Subspace_glow_bitmap = -1;

float Subspace_glow_frame = 0.0f;
float Subspace_glow_rate = 1.0f;


//XSTR:OFF
DCF(subspace_set,"Set parameters for subspace effect")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "u" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				subspace_u_speed = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "v" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				subspace_v_speed = Dc_arg_float;
			} 
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: subspace keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "subspace u X    Where X is how fast u moves.\n", MAX_STARS );
		dc_printf( "subspace v X    Where X is how fast v moves.\n" );
		dc_printf( "\nUse '? subspace' to see current values.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "u: %.2f\n", subspace_u_speed );
		dc_printf( "v: %.2f\n", subspace_v_speed );
	}
}
//XSTR:ON

void subspace_render()
{
	int framenum = 0;

	if ( Subspace_model_inner == -1 )	{
		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner >= 0);
	}

	if ( Subspace_model_outer == -1 )	{
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer >= 0);
	}

	if ( Subspace_glow_bitmap == -1 )	{
		Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
		Assert(Subspace_glow_bitmap >= 0);
	}

	if ( !Rendering_to_env ) {
		Subspace_glow_frame += flFrametime * 1.0f;

		float total_time = i2fl(NOISE_NUM_FRAMES) / 15.0f;

		// Sanity checks
		if ( Subspace_glow_frame < 0.0f )
			Subspace_glow_frame = 0.0f;
		if ( Subspace_glow_frame > 100.0f )
			Subspace_glow_frame = 0.0f;

		while ( Subspace_glow_frame > total_time )	{
			Subspace_glow_frame -= total_time;
		}

		framenum = fl2i( (Subspace_glow_frame*NOISE_NUM_FRAMES) / total_time );

		if ( framenum < 0 )
			framenum = 0;
		if ( framenum >= NOISE_NUM_FRAMES )
			framenum = NOISE_NUM_FRAMES-1;

		subspace_offset_u += flFrametime*subspace_u_speed;
		if (subspace_offset_u > 1.0f )	{
			subspace_offset_u -= 1.0f;
		}

		subspace_offset_u_inner += flFrametime*subspace_u_speed*3.0f;
		if (subspace_offset_u > 1.0f )	{
			subspace_offset_u -= 1.0f;
		}

		subspace_offset_v += flFrametime*subspace_v_speed;
		if (subspace_offset_v > 1.0f )	{
			subspace_offset_v -= 1.0f;
		}
	}
	

	matrix tmp;
	angles angs = { 0.0f, 0.0f, 0.0f };
	angs.b = subspace_offset_v * PI2;
	
	vm_angles_2_matrix(&tmp,&angs);
	
	int saved_gr_zbuffering = 	gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_NONE);

/*	if ( !D3D_enabled && !OGL_enabled )	{

		int render_flags = MR_NO_LIGHTING | MR_ALWAYS_REDRAW;

		Interp_subspace = 1;	
		Interp_subspace_offset_u = 1.0f - subspace_offset_u;
		Interp_subspace_offset_v = 0.0f;

		vec3d temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;
		model_set_thrust( Subspace_model_inner, &temp, -1, Subspace_glow_bitmap, Noise[framenum] );
		render_flags |= MR_SHOW_THRUSTERS;
		model_set_alpha(1.0f);	

		if (!Cmdline_nohtl)	gr_set_texture_panning(Interp_subspace_offset_v, Interp_subspace_offset_u, true);
		model_render( Subspace_model_outer, &tmp, &Eye_position, render_flags );	//MR_NO_CORRECT|MR_SHOW_OUTLINE 
		if (!Cmdline_nohtl)	gr_set_texture_panning(0, 0, false);

	} else {
*/
		int render_flags = MR_NO_LIGHTING | MR_ALL_XPARENT; // | MR_ALWAYS_REDRAW;

		Interp_subspace = 1;
		Interp_subspace_offset_u = 1.0f - subspace_offset_u;
		Interp_subspace_offset_v = 0.0f;

		vec3d temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;

		model_set_thrust( Subspace_model_inner, &temp, -1, Subspace_glow_bitmap, Noise[framenum] );

		render_flags |= MR_SHOW_THRUSTERS;
		model_set_alpha(1.0f);	

		if (!Cmdline_nohtl)
			gr_set_texture_panning(Interp_subspace_offset_v, Interp_subspace_offset_u, true);

		model_render( Subspace_model_outer, &tmp, &Eye_position, render_flags );	//MR_NO_CORRECT|MR_SHOW_OUTLINE

		if (!Cmdline_nohtl)
			gr_set_texture_panning(0, 0, false);
		
		Interp_subspace = 1;	
		Interp_subspace_offset_u = 1.0f - subspace_offset_u_inner;
		Interp_subspace_offset_v = 0.0f;	

		angs.b = -subspace_offset_v * PI2;

		vm_angles_2_matrix(&tmp,&angs);

		model_set_outline_color(255,255,255);

//		vec3d temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;

		model_set_thrust( Subspace_model_inner, &temp, -1, Subspace_glow_bitmap, Noise[framenum] );
		render_flags |= MR_SHOW_THRUSTERS;

		model_set_alpha(1.0f);	

		if (!Cmdline_nohtl)
			gr_set_texture_panning(Interp_subspace_offset_v, Interp_subspace_offset_u, true);

		model_render( Subspace_model_inner, &tmp, &Eye_position, render_flags  );	//MR_NO_CORRECT|MR_SHOW_OUTLINE

		if (!Cmdline_nohtl)
			gr_set_texture_panning(0, 0, false);
//	}

	Interp_subspace = 0;
	gr_zbuffer_set(saved_gr_zbuffering);
}


#ifdef NEW_STARS
//each star is actualy a line
//so each star point is actualy two points
struct star_point{
	star_point(){
		memset(p1.color,INT_MAX,4);
//		memset(p2.color,0,4);
		p2.color[0]=64;
		p2.color[1]=64;
		p2.color[2]=64;
		p2.color[3]=64;
	}
	colored_vector p1,p2;
	void set(vertex* P1,vertex* P2, color*Col){
		
//		if(resize || gr_screen.rendering_to_texture != -1)
/*		{
			extern bool gr_resize_screen_posf(float *x, float *y);

			gr_resize_screen_posf(&P1->x, &P1->y);
			gr_resize_screen_posf(&P2->x, &P2->y);
		}
*/ 
#ifndef NO_DIRECT3D
		if(gr_screen.mode != GR_OPENGL){
			extern float D3D_line_offset;
			P1->sw = 0.99f;
			P2->sw = 0.99f;

			P1->sx = i2fl(P1->sx + gr_screen.offset_x)+D3D_line_offset;
			P1->sy = i2fl(P1->sy + gr_screen.offset_y)+D3D_line_offset;

			P2->sx = i2fl(P2->sx + gr_screen.offset_x)+D3D_line_offset;
			P2->sy = i2fl(P2->sy + gr_screen.offset_y)+D3D_line_offset;
	}
#endif

		p1.vec.set_screen_vert(*P1);

		p2.vec.set_screen_vert(*P2);
		if(gr_screen.mode == GR_OPENGL)
			memcpy(p1.color, &Col->red,3);
		else {
			memcpy(&p1.color[1], &Col->red,3);
		}
	};
};

class star_point_list{
	int n_points;
	star_point *point_list;
public:
	star_point_list():n_points(0),point_list(NULL){};
	~star_point_list(){if(point_list)delete[]point_list;};

	void allocate(int size){
		if(size<=n_points)return;
		if(point_list)delete[]point_list;
		point_list = new star_point[size];
		n_points = size;
	}
	star_point* operator[] (int idx){
		return &point_list[idx];
	}
	colored_vector* get_buffer(){return &point_list->p1;};
};

star_point_list star_list;

void new_stars_draw_stars()//don't use me yet, I'm still haveing my API interface figured out-Bobboau
{
	star_list.allocate(Num_stars);
	//make sure our star buffer is big enough

/*	
	if(!last_stars_filled){
		star* star = Stars;
		for(int i = 0; i< Num_stars; i++){
			vertex p2;
			star++;
			g3_rotate_faraway_vertex(&p2, &star->pos);
			star->last_star_pos = p2;
		}
	}
*/
	//Num_stars = 1;
	int i;
	star *sp;
	float dist = 0.0f;
	float ratio;
	int color;
	float colorf;
	vertex p1, p2;			
	int can_draw = 1;

	if ( !last_stars_filled )	{
		for (sp=Stars,i=0; i<Num_stars; i++, sp++ ) {
			vertex p2;
			g3_rotate_faraway_vertex(&p2, &sp->pos);
			sp->last_star_pos.xyz.x = p2.x;
			sp->last_star_pos.xyz.y = p2.y;
			sp->last_star_pos.xyz.z = p2.z;
		}
	}

	int tmp_num_stars;

	tmp_num_stars = (Detail.num_stars*Num_stars)/MAX_DETAIL_LEVEL;
	if (tmp_num_stars < 0 )	{
		tmp_num_stars = 0;
	} else if ( tmp_num_stars > Num_stars )	{
		tmp_num_stars = Num_stars;
	}
		int stars_to_draw = 0;
	for (sp=Stars,i=0; i<tmp_num_stars; i++, sp++ ) {			

		can_draw=1;
		memset(&p1, 0, sizeof(vertex));
		memset(&p2, 0, sizeof(vertex));

		// This makes a star look "proper" by not translating the
		// point around the viewer's eye before rotation.  In other
		// words, when the ship translates, the stars do not change.

		g3_rotate_faraway_vertex(&p2, &sp->pos);
		if ( p2.codes )	{
			can_draw = 0;
		} else {
			g3_project_vertex(&p2);
			if ( p2.flags & PF_OVERFLOW )	{
				can_draw = 0;
			}
		}

		

		if ( can_draw && (Star_flags & (STAR_FLAG_TAIL|STAR_FLAG_DIM)) )	{

			dist = vm_vec_dist_quick( &sp->last_star_pos, (vec3d *)&p2.x );

			if ( dist > Star_max_length )	{
 				ratio = Star_max_length / dist;
				dist = Star_max_length;
			} else {
				ratio = 1.0f;
			}
			
			ratio *= Star_amount;

			p1.x = p2.x + (sp->last_star_pos.xyz.x-p2.x)*ratio;
			p1.y = p2.y + (sp->last_star_pos.xyz.y-p2.y)*ratio;
			p1.z = p2.z + (sp->last_star_pos.xyz.z-p2.z)*ratio;

			p1.flags = 0;	// not projected
			g3_code_vertex( &p1 );

			if ( p1.codes )	{
				can_draw = 0;
			} else {
				g3_project_vertex(&p1);
				if ( p1.flags & PF_OVERFLOW )	{
					can_draw = 0;
				}
			}
		}

		sp->last_star_pos.xyz.x = p2.x;
		sp->last_star_pos.xyz.y = p2.y;
		sp->last_star_pos.xyz.z = p2.z;

		if ( !can_draw )	continue;

		if ( Star_flags & STAR_FLAG_DIM )	{
			colorf = 255.0f - dist*Star_dim;
			if ( colorf < Star_cap )
				colorf = Star_cap;
			color = (fl2i(colorf)*(i&7))/256;
		} else {
			color = i & 7;
		}

		p1.sx-=1.5f;
		p1.sy-=1.5f;
		p2.sx+=1.5f;
		p2.sy+=1.5f;
				star_list[stars_to_draw++]->set(&p2,&p1,&sp->col);

	}
	gr_set_color_fast( &Stars->col );
	gr_zbuffer_set(GR_ZBUFF_NONE);
	gr_draw_line_list(star_list.get_buffer(), stars_to_draw);
	
}
#endif	// NEW_STARS


void stars_draw_stars()
{
	//Num_stars = 1;
	int i;
	star *sp;
	float dist = 0.0f;
	float ratio;
	int color;
	vDist vDst;
	float colorf;
	vertex p1, p2;			
	int can_draw = 1;

	if ( !last_stars_filled )	{
		for (sp=Stars,i=0; i<Num_stars; i++, sp++ ) {
			vertex p2;
			g3_rotate_faraway_vertex(&p2, &sp->pos);
			sp->last_star_pos.xyz.x = p2.x;
			sp->last_star_pos.xyz.y = p2.y;
			sp->last_star_pos.xyz.z = p2.z;
		}
	}

	int tmp_num_stars;

	tmp_num_stars = (Detail.num_stars*Num_stars)/MAX_DETAIL_LEVEL;
	if (tmp_num_stars < 0 )	{
		tmp_num_stars = 0;
	} else if ( tmp_num_stars > Num_stars )	{
		tmp_num_stars = Num_stars;
	}

	for (sp=Stars,i=0; i<tmp_num_stars; i++, sp++ ) {			

		can_draw=1;
		memset(&p1, 0, sizeof(vertex));
		memset(&p2, 0, sizeof(vertex));

		// This makes a star look "proper" by not translating the
		// point around the viewer's eye before rotation.  In other
		// words, when the ship translates, the stars do not change.

		g3_rotate_faraway_vertex(&p2, &sp->pos);
		if ( p2.codes )	{
			can_draw = 0;
		} else {
			g3_project_vertex(&p2);
			if ( p2.flags & PF_OVERFLOW )	{
				can_draw = 0;
			}
		}

		

		if ( can_draw && (Star_flags & (STAR_FLAG_TAIL|STAR_FLAG_DIM)) )	{

			dist = vm_vec_dist_quick( &sp->last_star_pos, (vec3d *)&p2.x );

			if ( dist > Star_max_length )	{
 				ratio = Star_max_length / dist;
				dist = Star_max_length;
			} else {
				ratio = 1.0f;
			}
			
			ratio *= Star_amount;

			p1.x = p2.x + (sp->last_star_pos.xyz.x-p2.x)*ratio;
			p1.y = p2.y + (sp->last_star_pos.xyz.y-p2.y)*ratio;
			p1.z = p2.z + (sp->last_star_pos.xyz.z-p2.z)*ratio;

			p1.flags = 0;	// not projected
			g3_code_vertex( &p1 );

			if ( p1.codes )	{
				can_draw = 0;
			} else {
				g3_project_vertex(&p1);
				if ( p1.flags & PF_OVERFLOW )	{
					can_draw = 0;
				}
			}
		}

		sp->last_star_pos.xyz.x = p2.x;
		sp->last_star_pos.xyz.y = p2.y;
		sp->last_star_pos.xyz.z = p2.z;

		if ( !can_draw )	continue;

		if ( Star_flags & STAR_FLAG_DIM )	{
			colorf = 255.0f - dist*Star_dim;
			if ( colorf < Star_cap )
				colorf = Star_cap;
			color = (fl2i(colorf)*(i&7))/256;
		} else {
			color = i & 7;
		}

		if ( (Star_flags & STAR_FLAG_ANTIALIAS) || (D3D_enabled) || (OGL_enabled) ) {
			gr_set_color_fast( &sp->col );

//			if the two points are the same, fudge it, since some D3D cards (G200 and G400) are lame.				
			if( (fl2i(p1.sx) == fl2i(p2.sx)) && (fl2i(p1.sy) == fl2i(p2.sy)) ){					
				p1.sx += 1.0f;
			}
			
			vDst.x = fl2i(p1.sx) - fl2i(p2.sx);
			vDst.y = fl2i(p1.sy) - fl2i(p2.sy);
				

			if( ((vDst.x * vDst.x) + (vDst.y * vDst.y)) <= 4 )
			{
				p1.sx = p2.sx;
				p1.sy = p2.sy;

				p1.sx += 1.0f;
			}
				gr_aaline(&p1,&p2);
			
		} else {
			gr_set_color_fast( &sp->col );

			if ( Star_flags & STAR_FLAG_TAIL )	{
				gr_line(fl2i(p1.sx), fl2i(p1.sy), fl2i(p2.sx), fl2i(p2.sy), false);
			} else {
				gr_pixel( fl2i(p2.sx), fl2i(p2.sy), false );
			}
		}
	}
}

void stars_draw_debris()
{
	int i;
	float vdist;
	vec3d tmp;
	vertex p;
	gr_set_color( 0, 0, 0 );

	// turn off fogging
	if (The_mission.flags & MISSION_FLAG_FULLNEB) {
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	old_debris * d = odebris; 

	for (i=0; i<MAX_DEBRIS; i++, d++ ) {
		if (!d->active)	{
			d->pos.xyz.x = f2fl(myrand() - RAND_MAX_2);
			d->pos.xyz.y = f2fl(myrand() - RAND_MAX_2);
			d->pos.xyz.z = f2fl(myrand() - RAND_MAX_2);

			vm_vec_normalize(&d->pos);

			vm_vec_scale(&d->pos, MAX_DIST);
			vm_vec_add2(&d->pos, &Eye_position );
			d->active = 1;
			d->vclip = i % MAX_DEBRIS_VCLIPS;	//rand()

			// if we're in full neb mode
			if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
				d->size = i2fl(myrand() % 4)*BASE_SIZE_NEB;
			} else {
				d->size = i2fl(myrand() % 4)*BASE_SIZE;
			}

			vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
		}

		if ( reload_old_debris ) {
			vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
		}
			
		g3_rotate_vertex(&p, &d->pos);

		if (p.codes == 0 && Debris_vclips[d->vclip].bm > -1) {
			int frame = Missiontime / (DEBRIS_ROT_MIN + (i % DEBRIS_ROT_RANGE) * DEBRIS_ROT_RANGE_SCALER);
			frame %= Debris_vclips[d->vclip].nframes;

			if ( (The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) ) {
				gr_set_bitmap( Debris_vclips[d->vclip].bm + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.3f);	
			} else {
				gr_set_bitmap( Debris_vclips[d->vclip].bm + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);	
			}

			vm_vec_add( &tmp, &d->last_pos, &Eye_position );
			g3_draw_laser( &d->pos,d->size,&tmp,d->size, TMAP_FLAG_TEXTURED|TMAP_FLAG_XPARENT, 25.0f );					
		}

		vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );

		vdist = vm_vec_mag_quick(&d->last_pos);

		if ( (vdist < MIN_DIST_RANGE) || (vdist > MAX_DIST_RANGE) )
			d->active = 0;
	}

	reload_old_debris = 0;
}

void stars_draw(int show_stars, int show_suns, int show_nebulas, int show_subspace, int env)
{
	int gr_zbuffering_save = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	Rendering_to_env = env;

//	stars_add_lights();

	if (show_subspace)
		subspace_render();

	if (Num_stars >= MAX_STARS)
		Num_stars = MAX_STARS;

#ifdef TIME_STAR_CODE
	fix xt1, xt2;
	xt1 = timer_get_fixed_seconds();
#endif

	if ( !env && show_stars && ( Game_detail_flags & DETAIL_FLAG_STARS) && !(The_mission.flags & MISSION_FLAG_FULLNEB) && (supernova_active() < 3) ) {
		stars_draw_stars();
	}

	if ( show_nebulas && (Game_detail_flags & DETAIL_FLAG_NEBULAS) && (Neb2_render_mode != NEB2_RENDER_POF) && (Neb2_render_mode != NEB2_RENDER_LAME))	{
		nebula_render();
	}

	// draw background stuff	
	if ( (Neb2_render_mode != NEB2_RENDER_POLY) && (Neb2_render_mode != NEB2_RENDER_LAME) && show_stars ) {
		// semi-hack, do we don't fog the background
		int neb_save = Neb2_render_mode;
		Neb2_render_mode = NEB2_RENDER_NONE;
		stars_draw_background();
		Neb2_render_mode = neb_save;
	}
	else if ( !show_subspace ) // don't render the background pof when rendering subspace
	{
		stars_draw_background();
	}


	last_stars_filled = 1;

#ifdef TIME_STAR_CODE
	xt2 = timer_get_fixed_seconds();
	mprintf(( "Stars: %d\n", xt2-xt1 ));
#endif

	if ( !Rendering_to_env && (Game_detail_flags & DETAIL_FLAG_MOTION) && (!Fred_running) && (supernova_active() < 3) && (!Cmdline_nomotiondebris) )	{
		stars_draw_debris();
	}

	//if we're not drawing them, quit here
	if (show_suns){
		stars_draw_sun( show_suns );	
		stars_draw_bitmaps( show_suns );
	}

	gr_zbuffer_set( gr_zbuffering_save );
	Rendering_to_env = 0;
}

void stars_preload_sun_bitmap(char *fname)
{
	int idx;

	if (fname == NULL)
		return;

	idx = stars_find_sun(fname);

	if (idx == -1) {
		return;
	}

	Sun_bitmaps[idx].preload = 1;
}

void stars_preload_background_bitmap(char *fname)
{
	int idx;

	if (fname == NULL)
		return;

	idx = stars_find_bitmap(fname);

	if (idx == -1) {
		return;
	}

	Starfield_bitmaps[idx].preload = 1;
}

void stars_page_in()
{
	int idx, i;
	starfield_bitmap_instance *sbi;
	starfield_bitmap *sb;

	// Initialize the subspace stuff

	if ( Game_subspace_effect )	{
		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner >= 0);
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer >= 0);

		polymodel *pm;
		
		pm = model_get(Subspace_model_inner);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (idx = 0; idx < pm->n_textures; idx++) {
			bm_page_in_texture( pm->maps[idx].base_map.original_texture );
		}

		pm = model_get(Subspace_model_outer);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (idx = 0; idx < pm->n_textures; idx++) {
			bm_page_in_texture(pm->maps[idx].base_map.original_texture);
		}

		if (Subspace_glow_bitmap < 0) {
			Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
		}

		bm_page_in_xparent_texture(Subspace_glow_bitmap);	
	} else {
		Subspace_model_inner = -1;
		Subspace_model_outer = -1;

		if (Subspace_glow_bitmap > 0) {
			bm_release(Subspace_glow_bitmap);
			Subspace_glow_bitmap = -1;
		}
	}

	// extra SEXP related checks to preload anything that might get used from there
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		sb = &Starfield_bitmaps[idx];

		if (sb->used_this_level)
			continue;

		if (sb->preload) {
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load(sb->filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->bitmap_id < 0) {
					sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

					if (sb->bitmap_id < 0) {
						Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", sb->filename);
					}
				}
			}

			// this happens whether it loaded properly or not, no harm should come from it
			if (sb->xparent) {
				bm_page_in_xparent_texture(sb->bitmap_id);
			} else {
				bm_page_in_texture(sb->bitmap_id);
			}

			sb->used_this_level++;
		}
	}

	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		sb = &Sun_bitmaps[idx];

		if (sb->used_this_level)
			continue;

		if (sb->preload) {
			// normal bitmap
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load(sb->filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->bitmap_id < 0) {
					sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

					if (sb->bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
					}
				}
			}

			// glow bitmap
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load(sb->glow_filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->glow_bitmap < 0) {
					sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, 1);

					if (sb->glow_bitmap < 0) {
						Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
					}
				}
			}

			if (sb->flare) {
				for (i = 0; i < MAX_FLARE_BMP; i++) {
					if ( !strlen(sb->flare_bitmaps[i].filename) )
						continue;

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

						if (sb->flare_bitmaps[i].bitmap_id < 0) {
							Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
							continue;
						}
					}

					bm_page_in_texture(sb->flare_bitmaps[i].bitmap_id);
				}
			}

			bm_page_in_texture(sb->bitmap_id);
			bm_page_in_texture(sb->glow_bitmap);

			sb->used_this_level++;
		}
	}

	// load and page in needed starfield bitmaps
	for (idx = 0; idx < (int)Starfield_bitmap_instances.size(); idx++) {
		sbi = &Starfield_bitmap_instances[idx];

		if (sbi->star_bitmap_index < 0)
			continue;

		sb = &Starfield_bitmaps[sbi->star_bitmap_index];

		if (sb->used_this_level)
			continue;

		if (sb->bitmap_id < 0 ) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// this happens whether it loaded properly or not, no harm should come from it
		if (sb->xparent) {
			bm_page_in_xparent_texture(sb->bitmap_id);
		} else {
			bm_page_in_texture(sb->bitmap_id);
		}

		sb->used_this_level++;
	}

	// now for sun bitmaps and glows
	for (idx = 0; idx < (int)Suns.size(); idx++) {
		sbi = &Suns[idx];

		if (sbi->star_bitmap_index < 0)
			continue;

		sb = &Sun_bitmaps[sbi->star_bitmap_index];

		if (sb->used_this_level)
			continue;

		// normal bitmap
		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// glow bitmap
		if (sb->glow_bitmap < 0) {
			sb->glow_bitmap = bm_load(sb->glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, 1);

				if (sb->glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
				}
			}
		}

		if (sb->flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if ( !strlen(sb->flare_bitmaps[i].filename) )
					continue;

				if (sb->flare_bitmaps[i].bitmap_id < 0) {
					sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
						continue;
					}
				}

				bm_page_in_texture(sb->flare_bitmaps[i].bitmap_id);
			}
		}

		bm_page_in_texture(sb->bitmap_id);
		bm_page_in_texture(sb->glow_bitmap);

		sb->used_this_level++;
	}


	if (Cmdline_nomotiondebris)
		return;

	for (idx = 0; idx < MAX_DEBRIS_VCLIPS; idx++) {
		if(Debris_vclips[idx].bm > -1) {
			bm_page_in_xparent_texture(Debris_vclips[idx].bm, Debris_vclips[idx].nframes);
		}
	}	
}

// background nebula models and planets
void stars_draw_background()
{	

	if (Nmodel_num < 0)
		return;

	if (Nmodel_bitmap > -1) {
		model_set_forced_texture(Nmodel_bitmap);
		Nmodel_flags |= MR_FORCE_TEXTURE;
	}

	// draw the model at the player's eye with no z-buffering
	model_set_alpha(1.0f);	
//	light_filter_push(-1,NULL,0.0f);
	if(!(Nmodel_flags & MR_NO_LIGHTING ))gr_set_lighting(true, true);
//	void light_set_all_relevent();
//	light_set_all_relevent();

	model_render(Nmodel_num, &vmd_identity_matrix, &Eye_position, Nmodel_flags, -1);	

//	light_filter_pop();
	if(!(Nmodel_flags & MR_NO_ZBUFFER ))gr_zbuffer_clear(TRUE);

	if(!(Nmodel_flags & MR_NO_LIGHTING ))gr_set_lighting(false, false);
//	model_render(Nmodel_num, &vmd_identity_matrix, &Eye_position, flags);	

	if (Nmodel_bitmap > -1)
		model_set_forced_texture(-1);
}

// call this to set a specific model as the background model
void stars_set_background_model(char *model_name, char *texture_name, int flags)
{
	if (Nmodel_bitmap >= 0) {
		bm_unload(Nmodel_bitmap);
		Nmodel_bitmap = -1;
	}
	
	if (Nmodel_num >= 0) {
		model_unload(Nmodel_num);
		Nmodel_num = -1;
	}

	if ( (model_name == NULL) || (strlen(model_name) < 1) )
		return;

	Nmodel_flags = flags;
	Nmodel_num = model_load(model_name, 0, NULL, 0);
	Nmodel_bitmap = bm_load(texture_name);

	if (Nmodel_num >= 0)
		model_page_in_textures(Nmodel_num);
}

// lookup a starfield bitmap, return index or -1 on fail
int stars_find_bitmap(char *name)
{
	int idx;

	if (name == NULL)
		return -1;

	// lookup
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		if ( !stricmp(name, Starfield_bitmaps[idx].filename) ) {
			return idx;
		}
	}

	// not found 
	return -1;
}

// lookup a sun by bitmap filename, return index or -1 on fail
int stars_find_sun(char *name)
{
	int idx;

	if (name == NULL)
		return -1;

	// lookup
	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		if ( !stricmp(name, Sun_bitmaps[idx].filename) ) {
			return idx;
		}
	}

	// not found 
	return -1;
}

// add an instance for a sun (something actually used in a mission)
// NOTE that we assume a duplicate is ok here
int stars_add_sun_entry(starfield_list_entry *sun)
{
	int idx, i;
	starfield_bitmap_instance sbi;

	Assert(sun != NULL);

	// copy information
	memset(&sbi, 0, sizeof(starfield_bitmap_instance));
	sbi.ang.p = sun->ang.p;
	sbi.ang.b = sun->ang.b;
	sbi.ang.h = sun->ang.h;
	sbi.scale_x = sun->scale_x;
	sbi.scale_y = sun->scale_y;
	sbi.div_x = sun->div_x;
	sbi.div_y = sun->div_y;

	idx = stars_find_sun(sun->filename);

	if (idx == -1) {
		Warning(LOCATION, "Trying to add a sun that does not exist in stars.tbl!");
		return 0;
	}

	sbi.star_bitmap_index = idx;

	// make sure any needed bitmaps are loaded
	if (Sun_bitmaps[idx].bitmap_id < 0) {
		// normal bitmap
		Sun_bitmaps[idx].bitmap_id = bm_load(Sun_bitmaps[idx].filename);

			// maybe didn't load a static image so try for an animated one
		if (Sun_bitmaps[idx].bitmap_id < 0) {
			Sun_bitmaps[idx].bitmap_id = bm_load_animation(Sun_bitmaps[idx].filename, &Sun_bitmaps[idx].n_frames, &Sun_bitmaps[idx].fps, 1);

			if (Sun_bitmaps[idx].bitmap_id < 0) {
				Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", Sun_bitmaps[idx].filename);
				return 0;
			}
		}

		// glow bitmap
		if (Sun_bitmaps[idx].glow_bitmap < 0) {
			Sun_bitmaps[idx].glow_bitmap = bm_load(Sun_bitmaps[idx].glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (Sun_bitmaps[idx].glow_bitmap < 0) {
				Sun_bitmaps[idx].glow_bitmap = bm_load_animation(Sun_bitmaps[idx].glow_filename, &Sun_bitmaps[idx].glow_n_frames, &Sun_bitmaps[idx].glow_fps, 1);

				if (Sun_bitmaps[idx].glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", Sun_bitmaps[idx].glow_filename);
				}
			}
		}

		if (Sun_bitmaps[idx].flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if ( !strlen(Sun_bitmaps[idx].flare_bitmaps[i].filename) )
					continue;

				if (Sun_bitmaps[idx].flare_bitmaps[i].bitmap_id < 0) {
					Sun_bitmaps[idx].flare_bitmaps[i].bitmap_id = bm_load(Sun_bitmaps[idx].flare_bitmaps[i].filename);

					if (Sun_bitmaps[idx].flare_bitmaps[i].bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", Sun_bitmaps[idx].flare_bitmaps[i].filename);
						continue;
					}
				}
			}
		}
	}

	// now check if we can make use of a previously discarded instance entry
	// this should never happen with FRED
	if ( !Fred_running ) {
		for (i = 0; i < (int)Suns.size(); i++) {
			if ( Suns[i].star_bitmap_index < 0 ) {
				Suns[i] = sbi;
				goto Done;
			}
		}
	}

	// ... or add a new one
	Suns.push_back(sbi);

Done:
	starfield_generate_bitmap_instance_buffers();

	return 1;
}

// add an instance for a starfield bitmap (something actually used in a mission)
// NOTE that we assume a duplicate is ok here
int stars_add_bitmap_entry(starfield_list_entry *sle)
{
	int idx;
	starfield_bitmap_instance sbi;

	Assert( sle != NULL );

	// copy information
	memset(&sbi, 0, sizeof(starfield_bitmap_instance));
	sbi.ang.p = sle->ang.p;
	sbi.ang.b = sle->ang.b;
	sbi.ang.h = sle->ang.h;
	sbi.scale_x = sle->scale_x;
	sbi.scale_y = sle->scale_y;
	sbi.div_x = sle->div_x;
	sbi.div_y = sle->div_y;

	idx = stars_find_bitmap(sle->filename);

	if (idx == -1) {
		Warning(LOCATION, "Trying to add a bitmap that does not exist in stars.tbl!");
		return 0;
	}

	sbi.star_bitmap_index = idx;

	// make sure any needed bitmaps are loaded
	if (Starfield_bitmaps[idx].bitmap_id < 0) {
		Starfield_bitmaps[idx].bitmap_id = bm_load(Starfield_bitmaps[idx].filename);

		// maybe didn't load a static image so try for an animated one
		if (Starfield_bitmaps[idx].bitmap_id < 0) {
			Starfield_bitmaps[idx].bitmap_id = bm_load_animation(Starfield_bitmaps[idx].filename, &Starfield_bitmaps[idx].n_frames, &Starfield_bitmaps[idx].fps, 1);

			if (Starfield_bitmaps[idx].bitmap_id < 0) {
				Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", Starfield_bitmaps[idx].filename);
				return 0;
			}
		}
	}

	// now check if we can make use of a previously discarded instance entry
	for (int i = 0; i < (int)Starfield_bitmap_instances.size(); i++) {
		if ( Starfield_bitmap_instances[i].star_bitmap_index < 0 ) {
			starfield_update_index_buffers(i, 0);
			Starfield_bitmap_instances[i] = sbi;
			goto Done;
		}
	}

	// ... or add a new one
	Starfield_bitmap_instances.push_back(sbi);

Done:
	starfield_generate_bitmap_instance_buffers();

	return 1;
}

// get the number of entries that each vector contains
// "sun" will get sun instance counts, otherwise it gets normal starfield bitmap instance counts
// "bitmap_count" will get number of starfield_bitmap entries rather than starfield_bitmap_instance entries
int stars_get_num_entries(bool sun, bool bitmap_count)
{
	// try for instance counts first
	if (!bitmap_count) {
		if (sun) {
			return (int)Suns.size();
		} else {
			return (int)Starfield_bitmap_instances.size();
		}
	}
	// looks like we want bitmap counts (probably only FRED uses this)
	else {
		if (sun) {
			return (int)Sun_bitmaps.size();
		} else {
			return (int)Starfield_bitmaps.size();
		}
	}
}


// get a starfield_bitmap entry providing only an instance
starfield_bitmap *stars_get_bitmap_entry(int index, bool sun)
{
	int max_index = (sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (sun && (Suns[index].star_bitmap_index >= 0)) {
		return &Sun_bitmaps[Suns[index].star_bitmap_index];
	} else if (!sun && (Starfield_bitmap_instances[index].star_bitmap_index >= 0)) {
		return &Starfield_bitmaps[Starfield_bitmap_instances[index].star_bitmap_index];
	}

	return NULL;
}

bool stars_sun_has_glare(int index)
{
	starfield_bitmap *sb = stars_get_bitmap_entry(index, true);
	return (sb && sb->glare);
}

// get a starfield_bitmap_instance, obviously
starfield_bitmap_instance *stars_get_instance(int index, bool sun)
{
	int max_index = (sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (sun) {
		return &Suns[index];
	} else {
		return &Starfield_bitmap_instances[index];
	}
}

// set an instace to not render
void stars_mark_instance_unused(int index, bool sun)
{
	int max_index = (sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return;

	if (sun) {
		Suns[index].star_bitmap_index =  -1;
	} else {
		Starfield_bitmap_instances[index].star_bitmap_index = -1;
	}

	starfield_generate_bitmap_instance_buffers();
}

// retrieves the name from starfield_bitmap for the instance index
// NOTE: it's unsafe to return NULL here so use <none> for invalid entries
const char *stars_get_name_from_instance(int index, bool sun)
{
	int max_index = (sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NOX("<none>");

	if (sun && (Suns[index].star_bitmap_index >= 0)) {
		return Sun_bitmaps[Suns[index].star_bitmap_index].filename;
	} else if (!sun && (Starfield_bitmap_instances[index].star_bitmap_index >= 0)) {
		return Starfield_bitmaps[Starfield_bitmap_instances[index].star_bitmap_index].filename;
	}

	return NOX("<none>");
}

// WMC/Goober5000
void stars_set_nebula(bool activate)
{
	if (activate)
	{
		The_mission.flags |= MISSION_FLAG_FULLNEB;
		Toggle_text_alpha = TOGGLE_TEXT_NEBULA_ALPHA;
		HUD_contrast = 1;
		if(Cmdline_nohtl || Fred_running) {
			Neb2_render_mode = NEB2_RENDER_POF;
			stars_set_background_model(BACKGROUND_MODEL_FILENAME, Neb2_texture_name);
		} else {
			Neb2_render_mode = NEB2_RENDER_HTL;
		}
		Debris_vclips = Debris_vclips_nebula;
		neb2_eye_changed();
	}
	else
	{
		The_mission.flags &= ~MISSION_FLAG_FULLNEB;
		Toggle_text_alpha = TOGGLE_TEXT_NORMAL_ALPHA;
		Neb2_render_mode = NEB2_RENDER_NONE;
		Debris_vclips = Debris_vclips_normal;
		HUD_contrast = 0;
	}
}

// retrieves the name from starfield_bitmap, really only used by FRED2
// NOTE: it is unsafe to return NULL here, but because that's bad anyway it really shouldn't happen, so we do return NULL.
const char *stars_get_name_FRED(int index, bool sun)
{
	if (!Fred_running)
		return NULL;

	int max_index = (sun) ? (int)Sun_bitmaps.size() : (int)Starfield_bitmaps.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (sun) {
		return Sun_bitmaps[index].filename;
	} else {
		return Starfield_bitmaps[index].filename;
	}
}

// modify an existing starfield bitmap instance, or add a new one if needed
void stars_modify_entry_FRED(int index, const char *name, starfield_list_entry *sbi_new, bool sun)
{
	if (!Fred_running)
		return;

	starfield_bitmap_instance sbi;
	int idx;
	int add_new = index > ((sun) ? (int)Sun_bitmaps.size() : (int)Starfield_bitmaps.size());

	Assert( index >= 0 );
	Assert( sbi_new != NULL );

	memcpy( &sbi, sbi_new, sizeof(starfield_bitmap_instance) );

	if (sun) {
		idx = stars_find_sun((char*)name);
	} else {
		idx = stars_find_bitmap((char*)name);
	}

	// this shouldn't ever happen from FRED since you select the name from a list of those available
	if (idx == -1)
		return;
//		Int3();

	sbi.star_bitmap_index = idx;

	if (add_new) {
		if (sun) {
			Suns.push_back( sbi );
		} else {
			Starfield_bitmap_instances.push_back( sbi );
		}
	} else {
		if (sun) {
			Suns[index] = sbi;
		} else {
			Starfield_bitmap_instances[index] = sbi;
		}
	}

	starfield_generate_bitmap_instance_buffers();
}

// erase an instance, note that this is very slow so it should only be done in FRED
void stars_delete_instance_FRED(int index, bool sun)
{
	if (!Fred_running)
		return;

	int max_index = (sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return;

	if (sun) {
		Suns.erase( Suns.begin() + index );
	} else {
		Starfield_bitmap_instances.erase( Starfield_bitmap_instances.begin() + index );
	}

	starfield_generate_bitmap_instance_buffers();
}

// Goober5000
void stars_load_first_valid_background()
{
	int background_idx = stars_get_first_valid_background();

#ifndef NDEBUG
	if (background_idx < 0 && !Fred_running)
	{
		int i;
		bool at_least_one_bitmap = false;
		for (i = 0; i < Num_backgrounds; i++)
		{
			if (Backgrounds[i].bitmaps.size() > 0)
				at_least_one_bitmap = true;
		}

		if (at_least_one_bitmap)
		{
			if (Num_backgrounds == 1)
				Warning(LOCATION, "Unable to find a sufficient number of bitmaps for this mission's background.  The background will not be displayed.");	
			else if (Num_backgrounds > 1)
				Warning(LOCATION, "Unable to find a sufficient number of bitmaps for any background listed in this mission.  No background will be displayed.");
		}
	}
#endif

	stars_load_background(background_idx);
}

// Goober5000
int stars_get_first_valid_background()
{
	int i;
	uint j;

	if (Num_backgrounds == 0)
		return -1;

	// get the first background with > 50% of its suns and > 50% of its bitmaps present
	for (i = 0; i < Num_backgrounds; i++)
	{
		uint total_suns = 0;
		uint total_bitmaps = 0;
		background_t *background = &Backgrounds[i];

		for (j = 0; j < background->suns.size(); j++)
		{
			if (stars_find_sun(background->suns[j].filename) >= 0)
				total_suns++;
		}

		for (j = 0; j < background->bitmaps.size(); j++)
		{
			if (stars_find_bitmap(background->bitmaps[j].filename) >= 0)
				total_bitmaps++;
		}

		// add 1 so rounding will work properly
		if ((total_suns >= (background->suns.size() + 1) / 2) && (total_bitmaps >= (background->bitmaps.size() + 1) / 2))
			return i;
	}

	// didn't find a valid entry
	return -1;
}

// Goober5000
void stars_load_background(int background_idx)
{
	uint j;

	stars_clear_instances();
	Cur_background = background_idx;

	if (Cur_background >= 0)
	{
		background_t *background = &Backgrounds[Cur_background];

		for (j = 0; j < background->suns.size(); j++)
		{
			if (!stars_add_sun_entry(&background->suns[j]) && !Fred_running)
				Warning(LOCATION, "Failed to add sun '%s' to the mission!", background->suns[j].filename);
		}

		for (j = 0; j < background->bitmaps.size(); j++)
		{
			if (!stars_add_bitmap_entry(&background->bitmaps[j]) && !Fred_running)
				Warning(LOCATION, "Failed to add starfield bitmap '%s' to the mission!", background->bitmaps[j].filename);
		}
	}
}

// Goober5000
void stars_copy_background(background_t *dest, background_t *src)
{
	uint i;

	dest->suns.clear();
	dest->bitmaps.clear();

	for (i = 0; i < src->suns.size(); i++)
	{
		starfield_list_entry sle;
		memcpy(&sle, &src->suns[i], sizeof(starfield_list_entry));
		dest->suns.push_back(sle);
	}

	for (i = 0; i < src->bitmaps.size(); i++)
	{
		starfield_list_entry sle;
		memcpy(&sle, &src->bitmaps[i], sizeof(starfield_list_entry));
		dest->bitmaps.push_back(sle);
	}
}

// Goober5000
void stars_swap_backgrounds(int idx1, int idx2)
{
	background_t temp;
	stars_copy_background(&temp, &Backgrounds[idx1]);
	stars_copy_background(&Backgrounds[idx1], &Backgrounds[idx2]);
	stars_copy_background(&Backgrounds[idx2], &temp);
}

// Goober5000
bool stars_background_empty(int idx)
{
	return !(Backgrounds[idx].suns.size() > 0 || Backgrounds[idx].bitmaps.size() > 0);
}

// Goober5000
void stars_pack_backgrounds()
{
	int i, j;

	// move all empty entries to the end, and recount
	Num_backgrounds = 0;
	for (i = 0; i < MAX_BACKGROUNDS; i++)
	{
		if (!stars_background_empty(i))
		{
			Num_backgrounds++;
			continue;
		}

		for (j = i + 1; j < MAX_BACKGROUNDS; j++)
		{
			if (!stars_background_empty(j))
			{
				stars_swap_backgrounds(i, j);
				Num_backgrounds++;
				break;
			}
		}
	}
}
