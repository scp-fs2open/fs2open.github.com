/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Bmpman/BmpMan.cpp $
 *
 * $Revision: 2.35 $
 * $Date: 2004-11-04 08:33:44 $
 * $Author: taylor $
 *
 * Code to load and manage all bitmaps for the game
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.34  2004/10/31 21:26:27  taylor
 * bmpman merge, EFF animation support, better page in stuff, dozen or so smaller fixes and cleanup
 *
 * Revision 2.33  2004/07/26 20:47:24  Kazan
 * remove MCD complete
 *
 * Revision 2.32  2004/07/12 16:32:42  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.31  2004/06/26 19:23:54  wmcoolmon
 * Minor optimizations
 *
 * Revision 2.30  2004/05/11 23:08:55  taylor
 * do page in unlock but not for gr_preload() stuff
 *
 * Revision 2.29  2004/05/06 22:35:25  taylor
 * DDS mipmap reading, remove unneeded bm_unlock() during page in
 *
 * Revision 2.28  2004/04/26 15:51:26  taylor
 * forgot to remove some safety checks that aren't needed anymore
 *
 * Revision 2.27  2004/04/26 12:44:54  taylor
 * 32-bit targa and jpeg loaders, bug fixes, some preload stuff
 *
 * Revision 2.26  2004/04/09 20:32:31  phreak
 * moved an assignment statement inside an #ifdef block so dds works in release mode
 *
 * Revision 2.25  2004/04/09 20:07:56  phreak
 * fixed DDS not working in OpenGL.  there was a misplaced Assert()
 *
 * Revision 2.24  2004/04/01 15:31:20  taylor
 * don't use interface anis as ship textures
 *
 * Revision 2.23  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.22  2004/02/28 14:14:56  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.21  2004/02/20 21:45:40  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.20  2004/02/14 00:18:29  randomtiger
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
 * Revision 2.19  2004/01/18 14:03:22  randomtiger
 * A couple of FRED_OGL changes.
 *
 * Revision 2.18  2004/01/17 21:59:52  randomtiger
 * Some small changes to the main codebase that allow Fred_open OGL to compile.
 *
 * Revision 2.17  2003/11/16 09:42:38  Goober5000
 * clarified and pruned debug spew messages
 * --Goober5000
 *
 * Revision 2.16  2003/10/24 17:35:04  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.15  2003/08/16 03:52:22  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.14  2003/08/12 03:18:32  bobboau
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
 * Revision 2.13  2003/06/07 21:08:09  phreak
 * bmp_gfx_get_components now works with 32 bit opengl
 *
 * Revision 2.12  2003/03/18 10:07:00  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 *
 * Revision 2.11  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.10  2003/01/18 19:55:16  phreak
 * fixed around the bmpman system to now accept compressed textures
 *
 * Revision 2.9  2003/01/09 22:23:38  inquisitor
 * Rollback to 2.7 to fix glow code issue that phreak noticed.
 * -Quiz
 *
 * Revision 2.7  2003/01/05 23:41:50  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.6  2002/12/02 23:26:08  Goober5000
 * fixed misspelling
 *
 * Revision 2.5  2002/12/02 20:46:41  Goober5000
 * fixed misspelling of "category" as "catagory"
 *
 * Revision 2.4  2002/11/22 20:55:27  phreak
 * changed around some page-in functions to work with opengl
 * changed bm_set_components_opengl
 * -phreak
 *
 * Revision 2.3  2002/11/18 21:27:13  phreak
 * added bm_select_components functions for OpenGL -phreak
 *
 *
 * Revision 2.2.2.12  2002/11/04 16:04:20  randomtiger
 *
 * Tided up some bumpman stuff and added a few function points to gr_screen. - RT
 *
 * Revision 2.2.2.11  2002/11/04 03:02:28  randomtiger
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
 *
 * Revision 2.2  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/21 15:36:25  mharris
 * Added ifdef WIN32
 *
 * Revision 1.3  2002/05/09 13:49:30  mharris
 * Added ifndef NO_DIRECT3D
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 37    9/13/99 11:26p Andsager
 * Add debug code to check for poorly sized anis
 * 
 * 36    9/05/99 11:19p Dave
 * Made d3d texture cache much more safe. Fixed training scoring bug where
 * it would backout scores without ever having applied them in the first
 * place.
 * 
 * 35    8/20/99 2:09p Dave
 * PXO banner cycling.
 * 
 * 34    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 33    8/06/99 1:52p Dave
 * Bumped up MAX_BITMAPS for the demo.
 * 
 * 32    8/02/99 1:49p Dave
 * Fixed low-mem animation problem. Whee!
 * 
 * 31    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 30    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 29    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 28    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 27    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 26    4/27/99 12:16a Dave
 * Fixed beam weapon muzzle glow problem. Fixed premature timeout on the
 * pxo server list screen. Fixed secondary firing for hosts on a
 * standalone. Fixed wacky multiplayer weapon "shuddering" problem.
 * 
 * 25    4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 24    4/08/99 10:42a Johnson
 * Don't try to swizzle a texture to transparent in Fred.
 * 
 * 23    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 22    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 21    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 20    2/08/99 5:07p Dave
 * FS2 chat server support. FS2 specific validated missions.
 * 
 * 19    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 18    2/04/99 6:29p Dave
 * First full working rev of FS2 PXO support.  Fixed Glide lighting
 * problems.
 * 
 * 17    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 16    1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 15    1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 14    1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 13    1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 12    1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 11    12/14/98 4:01p Dave
 * Got multi_data stuff working well with new xfer stuff. 
 * 
 * 10    12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 9     12/01/98 5:53p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 8     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 7     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 6     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 5     11/30/98 5:31p Dave
 * Fixed up Fred support for software mode.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     10/22/98 6:14p Dave
 * Optimized some #includes in Anim folder. Put in the beginnings of
 * parse/localization support for externalized strings and tstrings.tbl
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 106   5/23/98 4:43p John
 * Took out debugging sleep
 * 
 * 105   5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 104   5/20/98 12:59p John
 * Turned optimizations on for debug builds.   Also turning on automatic
 * function inlining.  Turned off the unreachable code warning.
 * 
 * 103   5/20/98 10:20a Hoffoss
 * Fixed warning in the code.
 * 
 * 102   5/19/98 3:45p John
 * fixed bug causing lowmem to drop half of the frames. Also halved fps
 * during lowmem.
 * 
 * 101   5/14/98 3:38p John
 * Added in more non-darkening colors for Adam.  Had to fix some bugs in
 * BmpMan and Ani stuff to get this to work.
 * 
 * 100   4/22/98 9:13p John
 * Added code to replace frames of animations in vram if so desired.
 * 
 * 99    4/17/98 6:56a John
 * Fixed bug where RLE'd user bitmaps caused data to not get freed.
 * (Turned off RLE for use bitmaps).   Made lossy animations reduce
 * resolution by a factor of 2 in low memory conditions.
 * 
 * 98    4/16/98 6:31p Hoffoss
 * Added function to get filename of a bitmap handle, which we don't have
 * yet and I need.
 * 
 * 97    4/01/98 9:27p John
 * Fixed debug info in bmpman.
 * 
 * 96    4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 95    3/31/98 9:55a Lawrance
 * JOHN: get xparency working for user bitmaps
 * 
 * 94    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 93    3/29/98 4:05p John
 * New paging code that loads everything necessary at level startup.
 * 
 * 92    3/27/98 11:20a John
 * commented back in some debug code.
 * 
 * 91    3/26/98 5:21p John
 * Added new code to preload all bitmaps at the start of a level.
 * Commented it out, though.
 * 
 * 90    3/26/98 4:56p Jasen
 * AL: Allow find_block_of() to allocate a series of bitmaps from index 0
 * 
 * 89    3/26/98 10:21a John
 * Made no palette-mapped bitmaps use 0,255,0 as transparent.
 * 
 * 88    3/24/98 5:39p John
 * Added debug code to show bitmap fragmentation.  Made user bitmaps
 * allocate from top of array.
 * 
 * 87    3/22/98 3:28p John
 * Added in stippled alpha for lower details.  Made medium detail use
 * engine glow.
 * 
 * 86    3/11/98 1:55p John
 * Fixed warnings
 * 
 * 85    3/06/98 4:09p John
 * Changed the way we do bitmap RLE'ing... this speds up HUD bitmaps by
 * about 2x
 * 
 * 84    3/02/98 6:00p John
 * Moved MAX_BITMAPS into BmpMan.h so the stuff in the graphics code that
 * is dependent on it won't break if it changes.   Made ModelCache slots
 * be equal to MAX_OBJECTS which is what it is.
 * 
 * 83    3/02/98 9:51a John
 * Added code to print the number of bitmap slots in use between levels.
 * 
 * 82    2/16/98 3:54p John
 * Changed a bunch of mprintfs to categorize to BmpInfo
 * 
 * 81    2/13/98 5:00p John
 * Made user bitmaps not get wrote to level cache file.
 * 
 * 80    2/06/98 8:25p John
 * Added code for new bitmaps since last frame
 * 
 * 79    2/06/98 8:10p John
 * Added code to show amout of texture usage each frame.
 * 
 * 78    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 77    1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 76    1/28/98 6:19p Dave
 * Reduced standalone memory usage ~8 megs. Put in support for handling
 * multiplayer submenu handling for endgame, etc.
 * 
 * 75    1/17/98 12:55p John
 * Fixed bug that I just created that loaded all ani frames.
 * 
 * 74    1/17/98 12:33p John
 * Made the game_busy function be called a constant amount of times per
 * level load, making the bar prediction easier.
 * 
 * 73    1/17/98 12:14p John
 * Added loading... bar to freespace.
 * 
 * 72    1/11/98 3:20p John
 * Made so that if no .clt exists, it will load all the bitmaps
 * 
 * 71    1/11/98 3:06p John
 * Made bitmap loading stop when cache gets full.
 * 
 * 70    1/11/98 2:45p John
 * Changed .lst to .clt
 * 
 * 69    1/11/98 2:14p John
 * Changed a lot of stuff that had to do with bitmap loading.   Made cfile
 * not do callbacks, I put that in global code.   Made only bitmaps that
 * need to load for a level load.
 * 
 * 67    1/09/98 4:07p John
 * Made all bitmap functions return a bitmap "Handle" not number.  This
 * helps to find bm_release errors.
 * 
 * 66    1/09/98 1:38p John
 * Fixed some bugs from previous comment
 * 
 * 65    1/09/98 1:32p John
 * Added some debugging code to track down a weird error.  Namely I fill
 * in the be structure with bogus values when someone frees it.
 * 
 * 64    12/28/97 2:00p John
 * put in another assert checking for invalid lock/unlock sequencing
 * 
 * 63    12/24/97 2:02p John
 * Changed palette translation to be a little faster for unoptimized
 * builds
 * 
 * 62    12/18/97 8:59p Dave
 * Finished putting in basic support for weapon select and ship select in
 * multiplayer.
 * 
 * 61    12/15/97 10:27p John
 * fixed bug where 2 bm_loads of same file both open the header.
 * 
 * 60    12/08/97 2:17p John
 * fixed bug with bmpman and cf_callback.
 * made cf_callback in Freespace set small font back when done.
 * 
 * 59    12/03/97 5:01p Lawrance
 * bump up MAX_BITMAPS to 1500.  People have reached 1000 bitmaps while
 * playing multiple missions.
 * 
 * 58    12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 57    11/30/97 3:57p John
 * Made fixed 32-bpp translucency.  Made BmpMan always map translucent
 * color into 255 even if you aren't supposed to remap and make it's
 * palette black.
 * 
 * 56    10/05/97 10:39a John
 * fixed bug with palette on unmapped bitmaps.  Made unmapped bitmaps get
 * marked with xparent.
 * 
 * 55    9/23/97 11:46a Lawrance
 * fixed bug with rle'ing with spans get big
 * 
 * 54    9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 53    9/19/97 10:18a John
 * fixed bug with aa animations being re-rle'd every 
 * frame.
 * 
 * 
 * 52    9/09/97 10:08a Sandeep
 * Fixed Compiler Level 4 warnings
 * 
 * 51    9/08/97 2:02p John
 * fixed typo in nprintf
 * 
 * 50    9/08/97 1:56p John
 * fixed some memory housekeeping bugs 
 * 
 * 49    9/03/97 4:19p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 48    8/29/97 7:35p Lawrance
 * check if .ani animation is already loaded in bm_load_animation()
 * 
 * 47    8/25/97 11:14p Lawrance
 * added support for .ani files in bm_load_animation()
 * 
 * 46    8/17/97 2:42p Lawrance
 * only flag PCX files as xparent if they have xparent pixels in them
 * 
 * 45    8/15/97 9:57a Lawrance
 * support multiple xparent entries for PCX files
 * 
 * 44    8/05/97 10:18a Lawrance
 * my_rand() being used temporarily instead of rand()
 * 
 * 43    8/01/97 4:30p John
 * 
 * 42    7/29/97 8:34a John
 * took out png stuff
 * 
 * 41    7/18/97 3:27p Lawrance
 * have pcx files use (0,255,0) for transparency
 * 
 * 40    7/16/97 3:07p John
 * 
 * 39    7/10/97 8:34a John
 * Added code to read TGA files.
 * 
 * 38    6/20/97 1:50p John
 * added rle code to bmpman.  made gr8_aabitmap use it.
 * 
 * 37    6/18/97 12:07p John
 * fixed some color bugs
 * 
 * 36    6/17/97 8:58p Lawrance
 * fixed bug with not nulling bm.data with USER bitmaps
 * 
 * 35    6/12/97 2:44a Lawrance
 * changed bm_unlock() to take an index into bm_bitmaps().  Added
 * ref_count to bitmap_entry struct
 * 
 * 34    5/20/97 10:36a John
 * Fixed problem with user bitmaps and direct3d caching.
 * 
 * 33    5/14/97 1:59p John
 * fixed a palette bug with vclips.
 * 
 * 32    3/24/97 4:43p John
 * speed up chunked collision detection by only checking cubes the vector
 * goes through.
 * 
 * 31    3/24/97 3:25p John
 * Cleaned up and restructured model_collide code and fvi code.  In fvi
 * made code that finds uvs work..  Added bm_get_pixel to BmpMan.
 * 
 * 30    3/11/97 2:49p Allender
 * 
 * 29    2/18/97 9:43a Lawrance
 * added Assert() in bm_release
 * 
 * 28    1/22/97 4:29p John
 * maybe fixed bug with code that counts total bytes of texture ram used.
 * 
 * 27    1/22/97 4:19p Lawrance
 * added flags to bm_create to allow transparency
 * 
 * 26    1/21/97 5:24p John
 * fixed another bug with bm_release.
 * 
 * 25    1/21/97 5:12p John
 * fixed bug with case
 * 
 * 24    1/21/97 5:02p John
 * Added code for 8bpp user bitmaps.
 * 
 * 23    1/09/97 11:35a John
 * Added some 2d functions to get/put screen images.
 * 
 * 22    11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 21    11/26/96 9:44a Allender
 * Allow for use of different bitmap palettes
 * 
 * 20    11/25/96 10:36a Allender
 * started working on 32 bpp support.  Added 15 bpp.
 * 
 * 19    11/18/96 1:51p Allender
 * fix up manager code to reread bitmaps if needed in newer bit depth
 * 
 * 18    11/15/96 4:24p Allender
 * more bmpman stuff -- only free bitmap slots when releasing copied
 * texture -- otherwise, only release the data for the bitmap
 * 
 * 17    11/15/96 3:33p Allender
 * added support for converting to 16 bit textures when requested with
 * bm_load.  Added some other management functions
 * 
 * 16    11/13/96 4:51p Allender
 * started overhaul of bitmap manager.  bm_load no longer actually load
 * the data, only the info for the bitmap.  Locking the bitmap now forces
 * load when no data present (or will if bpp changes)
 *
 * $NoKeywords: $
 */

#include <ctype.h>

#include "bmpman/bmpman.h"
#include "pcxutils/pcxutils.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "anim/animplay.h"
#include "io/timer.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "anim/packunpack.h"
#include "graphics/grinternal.h"
#include "tgautils/tgautils.h"
#include "ship/ship.h"
#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"
#include "jpgutils/jpgutils.h"
#include "parse/parselo.h"

extern int Cmdline_jpgtga;
extern int Cmdline_pcx32;

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif


// globals
int GLOWMAP = -1;
int SPECMAP = -1;
int ENVMAP = -1;

bitmap_entry bm_bitmaps[MAX_BITMAPS];

int bm_texture_ram = 0;
int bm_inited = 0;
int Bm_paging = 0;
// 16 bit pixel formats
int Bm_pixel_format = BM_PIXEL_FORMAT_ARGB;


// locals
uint Bm_next_signature = 0x1234;
int bm_next_handle = 1;
int Bm_low_mem = 0;
// Bm_max_ram - How much RAM bmpman can use for textures.
// Set to <1 to make it use all it wants.
int Bm_max_ram = 0;		//16*1024*1024;			// Only use 16 MB for textures

#define MAX_BMAP_SECTION_SIZE 256

#define EFF_FILENAME_CHECK { if ( be->type == BM_TYPE_EFF ) strncpy( filename, be->info.eff.filename, MAX_FILENAME_LEN ); else strncpy( filename, be->filename, MAX_FILENAME_LEN ); }

/* - no longer used but keep around just in case - tyalor
// get and put functions for 16 bit pixels - neat bit slinging, huh?
#define BM_SET_R_ARGB(p, r)	{ p[1] &= ~(0x7c); p[1] |= ((r & 0x1f) << 2); }
#define BM_SET_G_ARGB(p, g)	{ p[0] &= ~(0xe0); p[1] &= ~(0x03); p[0] |= ((g & 0x07) << 5); p[1] |= ((g & 0x18) >> 3); }
#define BM_SET_B_ARGB(p, b)	{ p[0] &= ~(0x1f); p[0] |= b & 0x1f; }
#define BM_SET_A_ARGB(p, a)	{ p[1] &= ~(0x80); p[1] |= ((a & 0x01) << 7); }

#define BM_SET_R_D3D(p, r)		{ *p |= (ushort)(( (int)r / Gr_current_red->scale ) << Gr_current_red->shift); }
#define BM_SET_G_D3D(p, g)		{ *p |= (ushort)(( (int)g / Gr_current_green->scale ) << Gr_current_green->shift); }
#define BM_SET_B_D3D(p, b)		{ *p |= (ushort)(( (int)b / Gr_current_blue->scale ) << Gr_current_blue->shift); }
#define BM_SET_A_D3D(p, a)		{ if(a == 0){ *p = (ushort)Gr_current_green->mask; } }

#define BM_SET_R(p, r)	{ switch(Bm_pixel_format){ case BM_PIXEL_FORMAT_ARGB: BM_SET_R_ARGB(((char*)p), r); break; case BM_PIXEL_FORMAT_D3D: BM_SET_R_D3D(p, r); break; default: Int3(); } }
#define BM_SET_G(p, g)	{ switch(Bm_pixel_format){ case BM_PIXEL_FORMAT_ARGB: BM_SET_G_ARGB(((char*)p), g); break; case BM_PIXEL_FORMAT_D3D: BM_SET_G_D3D(p, g); break; default: Int3(); } }
#define BM_SET_B(p, b)	{ switch(Bm_pixel_format){ case BM_PIXEL_FORMAT_ARGB: BM_SET_B_ARGB(((char*)p), b); break; case BM_PIXEL_FORMAT_D3D: BM_SET_B_D3D(p, b); break;  default: Int3(); } }
#define BM_SET_A(p, a)	{ switch(Bm_pixel_format){ case BM_PIXEL_FORMAT_ARGB: BM_SET_A_ARGB(((char*)p), a); break; case BM_PIXEL_FORMAT_D3D: BM_SET_A_D3D(p, a); break;  default: Int3(); } }
*/

// ===========================================
// Mode: 0 = High memory
//       1 = Low memory ( every other frame of ani's)
//       2 = Debug low memory ( only use first frame of each ani )
void bm_set_low_mem( int mode )
{
	Assert( (mode >= 0)  && (mode<=2 ));
	Bm_low_mem = mode;
}


int bm_get_next_handle()
{
	int n = bm_next_handle;
	bm_next_handle++;
	if ( bm_next_handle > 30000 )	{
		bm_next_handle = 1;
	}
	return n;
}

// Frees a bitmaps data if it should, and
// Returns true if bitmap n can free it's data.
static void bm_free_data(int n)
{
	bitmap_entry	*be;
	bitmap			*bmp;

	Assert( n >= 0 && n < MAX_BITMAPS );

	be = &bm_bitmaps[n];
	bmp = &be->bm;

	gr_bm_free_data(n);

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if ( be->type==BM_TYPE_NONE) 
		goto SkipFree;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if ( ( be->type==BM_TYPE_USER ) ) {
	#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
	#endif
		goto SkipFree;
	}

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if ( (bmp->data == 0) ) {
	#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
	#endif
		goto SkipFree;
	}

	// Free up the data now!

	//	mprintf(( "Bitmap %d freed %d bytes\n", n, bm_bitmaps[n].data_size ));
	#ifdef BMPMAN_NDEBUG
		bm_texture_ram -= be->data_size;
	#endif
	free((void *)bmp->data);

SkipFree:

	// Clear out & reset the bitmap data structure
	bmp->flags = 0;
	bmp->bpp = 0;
	bmp->data = 0;
	bmp->palette = NULL;
	#ifdef BMPMAN_NDEBUG
		be->data_size = 0;
	#endif
	be->signature = Bm_next_signature++; 
}


#ifdef BMPMAN_NDEBUG

int Bm_ram_freed = 0;

static void bm_free_some_ram( int n, int size )
{
/*
	if ( Bm_max_ram < 1 ) return;
	if ( bm_texture_ram + size < Bm_max_ram ) return;

	int current_time = timer_get_milliseconds();

	while( bm_texture_ram + size > Bm_max_ram )	{
		Bm_ram_freed++;

		// Need to free some RAM up!
		int i, oldest=-1, best_val=0;
		for (i = 0; i < MAX_BITMAPS; i++)	{
			if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].first_frame!=bm_bitmaps[n].first_frame) && (bm_bitmaps[i].ref_count==0) && (bm_bitmaps[i].data_size>0) )	{
				int page_func = ( current_time-bm_bitmaps[i].last_used)*bm_bitmaps[i].data_size;
				if ( (oldest==-1) || (page_func>best_val) )	{
					oldest=i;
					best_val = page_func;
				}
			}
		}

		if ( oldest > -1 )	{
			//mprintf(( "Freeing bitmap '%s'\n", bm_bitmaps[oldest].filename ));
			for (i=0; i<bm_bitmaps[oldest].num_frames; i++ )	{
				bm_free_data(bm_bitmaps[oldest].first_frame+i);
			}
		} else {
			//mprintf(( "Couldn't free enough! %d\n", bm_texture_ram ));
			break;
		}
	}	
*/
}

#endif

void *bm_malloc( int n, int size )
{
	Assert( n >= 0 && n < MAX_BITMAPS );
//	mprintf(( "Bitmap %d allocated %d bytes\n", n, size ));
	#ifdef BMPMAN_NDEBUG
	bm_free_some_ram( n, size );
	Assert( bm_bitmaps[n].data_size == 0 );
	bm_bitmaps[n].data_size += size;
	bm_texture_ram += size;
	#endif
	return malloc(size);
}

// kinda like bm_malloc but only keeps track of how much memory is getting used
void bm_update_memory_used( int n, int size )
{
	Assert( n >= 0 && n < MAX_BITMAPS );
	#ifdef BMPMAN_NDEBUG
	Assert( bm_bitmaps[n].data_size == 0 );
	bm_bitmaps[n].data_size += size;
	bm_texture_ram += size;
	#endif
}

void bm_close()
{
	int i;
	if ( bm_inited )	{
		for (i=0; i<MAX_BITMAPS; i++ )	{
			bm_free_data(i);			// clears flags, bbp, data, etc
		}
		bm_inited = 0;
	}
}

void bm_init()
{
	int i;

	mprintf(( "Size of bitmap info = %d KB\n", sizeof( bm_bitmaps )/1024 ));
	mprintf(( "Size of bitmap extra info = %d bytes\n", sizeof( bm_extra_info ) ));
	
	if (!bm_inited)	{
		bm_inited = 1;
		atexit(bm_close);
	}
	
	for (i=0; i<MAX_BITMAPS; i++ ) {
		bm_bitmaps[i].filename[0] = '\0';
		bm_bitmaps[i].type = BM_TYPE_NONE;
		bm_bitmaps[i].info.user.data = NULL;
		bm_bitmaps[i].bm.data = 0;
		bm_bitmaps[i].bm.palette = NULL;
		bm_bitmaps[i].info.eff.type = BM_TYPE_NONE;
		bm_bitmaps[i].info.eff.filename[0] = '\0';
		#ifdef BMPMAN_NDEBUG
			bm_bitmaps[i].data_size = 0;
			bm_bitmaps[i].used_count = 0;
			bm_bitmaps[i].used_last_frame = 0;
			bm_bitmaps[i].used_this_frame = 0;
		#endif

		gr_bm_init(i);

		bm_free_data(i);  	// clears flags, bbp, data, etc
	}
}

// Returns number of bytes of bitmaps locked this frame
// ntotal = number of bytes of bitmaps locked this frame
// nnew = number of bytes of bitmaps locked this frame that weren't locked last frame
void bm_get_frame_usage(int *ntotal, int *nnew)
{
#ifdef BMPMAN_NDEBUG
	int i;
	
	*ntotal = 0;
	*nnew = 0;

	for (i=0; i<MAX_BITMAPS; i++ ) {
		if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].used_this_frame))	{
			if ( !bm_bitmaps[i].used_last_frame )	{
				*nnew += bm_bitmaps[i].bm.w*bm_bitmaps[i].bm.h; 
			}
			*ntotal += bm_bitmaps[i].bm.w*bm_bitmaps[i].bm.h;
		}
		bm_bitmaps[i].used_last_frame = bm_bitmaps[i].used_this_frame;
		bm_bitmaps[i].used_this_frame = 0;
	}
#endif
}

// given a loaded bitmap with valid info, calculate sections
void bm_calc_sections(bitmap *be)
{
#ifdef BMP_SECTIONS
	int idx;

	// number of x and y sections
	be->sections.num_x = (ubyte)(be->w / MAX_BMAP_SECTION_SIZE);
	if((be->sections.num_x * MAX_BMAP_SECTION_SIZE) < be->w){
		be->sections.num_x++;
	}
	be->sections.num_y = (ubyte)(be->h / MAX_BMAP_SECTION_SIZE);
	if((be->sections.num_y * MAX_BMAP_SECTION_SIZE) < be->h){
		be->sections.num_y++;
	}

	// calculate the offsets for each section
	for(idx=0; idx<be->sections.num_x; idx++){
		be->sections.sx[idx] = (ushort)(MAX_BMAP_SECTION_SIZE * idx);
	}
	for(idx=0; idx<be->sections.num_y; idx++){
		be->sections.sy[idx] = (ushort)(MAX_BMAP_SECTION_SIZE * idx);
	}
#endif // BMP_SECTIONS
}

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of 32 (or 8)-bit-per-pixel data.  Right now, the only
// bpp you can pass in is 32 or 8.  On success, it returns the
// bitmap number.  You cannot free that RAM until bm_release
// is called on that bitmap.
int bm_create( int bpp, int w, int h, void * data, int flags )
{
	// Assert((bpp==32)||(bpp==8));
	if(bpp == 8){
		Assert(flags & BMP_AABITMAP);
	} else {
		Assert( (bpp == 16) || (bpp == 32) );
	}

	if ( !bm_inited ) bm_init();

	int n = MAX_BITMAPS;

	for (int i = MAX_BITMAPS-1; i >= 0; i-- ) {
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			n = i;
			break;
		}
	}

	Assert( n > -1 );

	// Out of bitmap slots
	if ( n == -1 ) return -1;

	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

	sprintf( bm_bitmaps[n].filename, "TMP%dx%d", w, h );
	bm_bitmaps[n].type = BM_TYPE_USER;
	bm_bitmaps[n].palette_checksum = 0;

	bm_bitmaps[n].bm.w = (short) w;
	bm_bitmaps[n].bm.h = (short) h;
	bm_bitmaps[n].bm.rowsize = (short) w;
	bm_bitmaps[n].bm.bpp = (unsigned char) bpp;
	bm_bitmaps[n].bm.flags = 0;
	bm_bitmaps[n].bm.flags |= flags;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	bm_bitmaps[n].info.user.bpp = ubyte(bpp);
	bm_bitmaps[n].info.user.data = data;
	bm_bitmaps[n].info.user.flags = ubyte(flags);

	bm_bitmaps[n].signature = Bm_next_signature++;

	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	bm_update_memory_used( n, w * h * (bpp / 8) );

	gr_bm_create(n);

	// fill in section info
	bm_calc_sections(&bm_bitmaps[n].bm);
	
	return bm_bitmaps[n].handle;
}

// sub helper function. Given a raw filename and an extension, try and find the bitmap
// returns -1 if it could not be found
//          0 if it was found as a file
//          1 if it already exists, fills in handle
int Bm_ignore_duplicates = 0;
int bm_load_sub(char *real_filename, const char *ext, int *handle, CFILE **img_cfp, int dir_type = CF_TYPE_ANY)
{	
	int i;
	char filename[MAX_FILENAME_LEN] = "";

	strcpy( filename, real_filename );
	strcat( filename, ext );	
	for (i=0; i<(int)strlen(filename); i++ ){
		filename[i] = char(tolower(filename[i]));
	}		

	// try to find given filename to see if it has been loaded before
	if(!Bm_ignore_duplicates){
		for (i = 0; i < MAX_BITMAPS; i++) {
			if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && !stricmp(filename, bm_bitmaps[i].filename) ) {
				nprintf (("BmpMan", "Found bitmap %s -- number %d\n", filename, i));
				*handle = bm_bitmaps[i].handle;
				return 1;
			}
		}	
	}

	// try and find the file
	CFILE *test = cfopen(filename, "rb", CFILE_NORMAL, dir_type);
	if (test != NULL) {
		*img_cfp = test;
		return 0;
	}

	// could not be found
	return -1;
}

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.  Function doesn't acutally load the data, only
// width, height, and possibly flags.
int bm_load( char * real_filename )
{
	int i, n, first_slot = MAX_BITMAPS;
	int w, h, bpp;
	int rc = 0;
	int bm_size = 0, mm_lvl = 0;
	char filename[MAX_FILENAME_LEN];
	int handle = -1;
	ubyte type = BM_TYPE_NONE;
	ubyte c_type = BM_TYPE_NONE;
	bool found = false;
	CFILE *img_cfp = NULL;

	if ( !bm_inited ) bm_init();

	// nice little trick for keeping standalone memory usage way low - always return a bogus bitmap 
	if(Game_mode & GM_STANDALONE_SERVER){
		strcpy(filename,"test128");
	}

	// if no file was passed then get out now
	if (strlen(real_filename) <= 0) {
		return -1;
	}

	// make sure no one passed an extension
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}

	// Lets find out what type it is
	{
		const int NUM_TYPES	= 4;
		const ubyte type_list[NUM_TYPES] = {BM_TYPE_TGA, BM_TYPE_JPG, BM_TYPE_DDS, BM_TYPE_PCX};
		const char *ext_list[NUM_TYPES] = {".tga", ".jpg", ".dds", ".pcx"};
		
		// Only load TGA and JPG if given flag, support DDS and PCX by default
		i = Cmdline_jpgtga ? 0 : 2; // 2 Means start with DDS and fall back to PCX

		for(; i<NUM_TYPES; i++)
		{
			// if we can't use dds files then skip them
			if ( (gr_screen.mode == GR_OPENGL) && !Texture_compression_enabled && (type_list[i] == BM_TYPE_DDS) )
				continue;

			n = bm_load_sub(filename, ext_list[i], &handle, &img_cfp);
			
			// Image is already loaded
			if(n == 1) {
				return handle;
			}

			// File was found
			if(n == 0)	{
				strcat(filename, ext_list[i]);
				type = type_list[i];
				found = true;
				break;
			}
		}
		
		// No match was found
		if(found == false) {
			return -1;
		}
	}

	Assert(type != BM_TYPE_NONE);

	// Error( LOCATION, "Unknown bitmap type %s\n", filename );

	// Find an open slot
	for (i = 0; i < MAX_BITMAPS; i++) {
		if ( (bm_bitmaps[i].type == BM_TYPE_NONE) && (first_slot == MAX_BITMAPS) ){
			first_slot = i;
		}
	}

	n = first_slot;
	Assert( n < MAX_BITMAPS );	

	if ( n == MAX_BITMAPS ) return -1;


	rc = gr_bm_load( type, n, filename, img_cfp, &w, &h, &bpp, &c_type, &mm_lvl, &bm_size );

	if (rc != 0) {
		if (img_cfp != NULL)
			cfclose(img_cfp);

		return -1;
	}

	if (c_type != BM_TYPE_NONE)
		type = c_type;


	// ensure fields are cleared out from previous bitmap
//	Assert(bm_bitmaps[n].bm.data == 0);
//	Assert(bm_bitmaps[n].bm.palette == NULL);
//	Assert(bm_bitmaps[n].ref_count == 0 );
//	Assert(bm_bitmaps[n].user_data == NULL);
	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );
	
	// Mark the slot as filled, because cf_read might load a new bitmap
	// into this slot.
	bm_bitmaps[n].type = type;
	bm_bitmaps[n].signature = Bm_next_signature++;
	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	strncpy(bm_bitmaps[n].filename, filename, MAX_FILENAME_LEN-1 );
	bm_bitmaps[n].bm.w = short(w);
	bm_bitmaps[n].bm.rowsize = short(w);
	bm_bitmaps[n].bm.h = short(h);
	bm_bitmaps[n].bm.bpp = 0;
	bm_bitmaps[n].bm.flags = 0;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;
	bm_bitmaps[n].num_mipmaps = mm_lvl;
	bm_bitmaps[n].mem_taken = bm_size;

	bm_bitmaps[n].palette_checksum = 0;
	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	// fill in section info
	bm_calc_sections(&bm_bitmaps[n].bm);

	if (img_cfp != NULL)
		cfclose(img_cfp);

	return bm_bitmaps[n].handle;
}

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_load_duplicate(char *filename)
{
	int ret;

	// ignore duplicates
	Bm_ignore_duplicates = 1;
	
	// load
	ret = bm_load(filename);

	// back to normal
	Bm_ignore_duplicates = 0;

	return ret;
}

DCF(bm_frag,"Shows BmpMan fragmentation")
{
	if ( Dc_command )	{

		gr_clear();

		int x=0, y=0;
		int xs=2, ys=2;
		int w=4, h=4;

		for (int i=0; i<MAX_BITMAPS; i++ )	{
			switch( bm_bitmaps[i].type )	{
			case BM_TYPE_NONE:
				gr_set_color(128,128,128);
				break;
			case BM_TYPE_PCX:
				gr_set_color(255,0,0);
				break;
			case BM_TYPE_USER:
				gr_set_color(0,255,0);
				break;
			case BM_TYPE_ANI:
				gr_set_color(0,0,255);
				break;
			}

			gr_rect( x+xs, y+ys, w, h );
			x += w+xs+xs;
			if ( x > 639 )	{
				x = 0;
				y += h + ys + ys;
			}

		}

		gr_flip();
		key_getch();
	}
}

static int find_block_of(int n)
{
	int i, cnt, nstart;

	cnt=0;
	nstart = 0;
	for (i=0; i<MAX_BITMAPS; i++ )	{
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			if (cnt==0) nstart = i;
			cnt++;
		} else
			cnt=0;
		if ( cnt == n ) return nstart;
	}

	// Error( LOCATION, "Couldn't find block of %d frames\n", n );
	return -1;
}

extern int required_string_ex(char *pstr);
extern int optional_string_ex(char *pstr);
extern void stuff_string_ex(char *pstr, int type, char *terminators, int len = 0);
extern void stuff_int_ex(int *i);
void reset_parse_ex(char *text);

int bm_load_and_parse_eff(char *filename, int dir_type, int *nframes, int *nfps, ubyte *type)
{
	int frames = 0, fps = 30;
	char ext[3];
	ubyte c_type = BM_TYPE_NONE;
	char file_text[50];
	char file_text_raw[50];

	memset( ext, 0, 3 );
	memset( file_text, 0, 50 );
	memset( file_text_raw, 0, 50 );

	read_file_text(filename, dir_type, file_text, file_text_raw);

	reset_parse_ex(file_text);

	required_string_ex("$Type:");
	stuff_string_ex(ext, F_NAME, NULL);

	required_string_ex( "$Frames:" );
	stuff_int_ex(&frames);

	if (optional_string_ex( "$FPS:" ));
		stuff_int_ex(&fps);

	mprintf(("EFF = filename: %s, type: %s, frames: %d, fps: %d\n", filename, ext, frames, fps));

	if (!stricmp(NOX("dds"), ext)) {
		c_type = BM_TYPE_DDS;
	} else if (!stricmp(NOX("tga"), ext)) {
		c_type = BM_TYPE_TGA;
	} else if (!stricmp(NOX("jpg"), ext)) {
		c_type = BM_TYPE_JPG;
	} else if (!stricmp(NOX("pcx"), ext)) {
		c_type = BM_TYPE_PCX;
	} else {
		mprintf(("BMPMAN: Unknown file type in EFF parse!\n"));
		return -1;
	}

	// did we do anything?
	if (c_type == BM_TYPE_NONE || frames == 0) {
		mprintf(("BMPMAN: EFF parse ERROR!\n"));
		return -1;
	}

	// make sure we can use the format in question
	if ( !Cmdline_jpgtga && (c_type == BM_TYPE_TGA) || (c_type == BM_TYPE_JPG) ) {
		mprintf(("BMPMAN: EFF is of JPG/TGA format and can't be used!\n"));
		return -1;
	}

	if (type)
		*type = c_type;

	if (nframes)
		*nframes = frames;

	if (nfps)
		*nfps = fps;

	return 0;
}

// ------------------------------------------------------------------
// bm_load_animation()
//
//	input:		filename		=>		filename of animation
//					nframes		=>		OUTPUT parameter:	number of frames in the animation
//					fps			=>		OUTPUT/OPTIONAL parameter: intended fps for the animation
//
// returns:		bitmap number of first frame in the animation
//
int bm_load_animation( char *real_filename, int *nframes, int *fps, int can_drop_frames, int dir_type)
{
	int	i, n;
	anim	the_anim;
	CFILE	*img_cfp = NULL;
	char filename[MAX_FILENAME_LEN];
	int reduced = 0;
	int anim_fps = 0, anim_frames = 0;
	int anim_width = 0, anim_height = 0;
	int handle = -1;
	ubyte type = BM_TYPE_NONE, c_type = BM_TYPE_NONE;
	bool found = false;
	int bpp = 0, mm_lvl = 0, img_size = 0;
	char clean_name[MAX_FILENAME_LEN];

	if ( !bm_inited ) bm_init();

	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load_animation for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}

	// used later if EFF type
	strcpy( clean_name, filename );

	// Lets find out what type it is
	{
		const int NUM_TYPES	= 2;
		const ubyte type_list[NUM_TYPES] = {BM_TYPE_EFF, BM_TYPE_ANI};
		const char *ext_list[NUM_TYPES] = {".eff", ".ani"};

		for(i=0; i<NUM_TYPES; i++) {
			n = bm_load_sub(filename, ext_list[i], &handle, &img_cfp, dir_type);

			// Image is already loaded
			if(n == 1) {
				n = handle % MAX_BITMAPS;
				Assert( bm_bitmaps[n].handle == handle );
				if (nframes) *nframes = bm_bitmaps[n].info.ani.num_frames;
				if (fps) *fps = bm_bitmaps[n].info.ani.fps;
				return handle;
			}

			// File was found
			if(( n == 0) && (img_cfp != NULL))	{
				strcat(filename, ext_list[i]);
				type = type_list[i];
				found = true;
				break;
			}
		}
		
		// No match was found
		if(found == false) {
			return -1;
		}
	}

	// it's an effect file, any readable image type with eff being txt
	if (type == BM_TYPE_EFF) {
		if ( bm_load_and_parse_eff(filename, dir_type, &anim_frames, &anim_fps, &c_type) != 0 ) {
			mprintf(("BMPMAN: Error reading EFF\n"));
			return -1;
		}
	}
	// regular ani file
	else if (type == BM_TYPE_ANI) {
#ifndef NDEBUG
		// for debug of ANI sizes
		strcpy(the_anim.name, real_filename);
#endif
		anim_read_header(&the_anim, img_cfp);

		anim_frames = the_anim.total_frames;
		anim_fps = the_anim.fps;
		anim_width = the_anim.width;
		anim_height = the_anim.height;
	} else {
		return -1;
	}

	if ( can_drop_frames ) {
		if ( Bm_low_mem == 1 ) {
			reduced = 1;
			anim_frames = (anim_frames+1)/2;
			anim_fps = (anim_fps/2);
		} else if ( Bm_low_mem == 2 ) {
			anim_frames = 1;
		}
	}

	n = find_block_of(anim_frames);
	if(n < 0){
		return -1;
	}
	// Assert( n >= 0 );

	int first_handle = bm_get_next_handle();

	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	for ( i = 0; i < anim_frames; i++ ) {
		memset( &bm_bitmaps[n+i], 0, sizeof(bitmap_entry) );
	
		if (type == BM_TYPE_EFF) {
			sprintf(bm_bitmaps[n+i].info.eff.filename, "%s_%.4d", clean_name, i);
			ubyte dds_type = BM_TYPE_NONE;
			gr_bm_load(c_type, n+i, bm_bitmaps[n+i].info.eff.filename, NULL, &anim_width, &anim_height, &bpp, &dds_type, &mm_lvl, &img_size);
			if (dds_type != BM_TYPE_NONE) c_type = dds_type;
			bm_bitmaps[n+i].info.eff.type = c_type;
		}

		bm_bitmaps[n+i].info.ani.first_frame = n;
		bm_bitmaps[n+i].info.ani.num_frames = (ubyte)anim_frames;
		bm_bitmaps[n+i].info.ani.fps = (ubyte)anim_fps;
		bm_bitmaps[n+i].bm.w = short(anim_width);
		bm_bitmaps[n+i].bm.rowsize = short(anim_width);
		bm_bitmaps[n+i].bm.h = short(anim_height);
		if ( reduced )	{
			bm_bitmaps[n+i].bm.w /= 2;
			bm_bitmaps[n+i].bm.rowsize /= 2;
			bm_bitmaps[n+i].bm.h /= 2;
		}
		bm_bitmaps[n+i].bm.flags = 0;
		bm_bitmaps[n+i].bm.bpp = (ubyte)bpp;
		bm_bitmaps[n+i].bm.data = 0;
		bm_bitmaps[n+i].bm.palette = NULL;
		bm_bitmaps[n+i].type = type;
		bm_bitmaps[n+i].palette_checksum = 0;
		bm_bitmaps[n+i].signature = Bm_next_signature++;
		bm_bitmaps[n+i].handle = first_handle*MAX_BITMAPS + n+i;
		bm_bitmaps[n+i].last_used = -1;
		bm_bitmaps[n+i].num_mipmaps = mm_lvl;
		bm_bitmaps[n+i].mem_taken = img_size;

		// fill in section info
		bm_calc_sections(&bm_bitmaps[n+i].bm);

		if ( i == 0 )	{
			sprintf( bm_bitmaps[n+i].filename, "%s", filename );
		} else {
			sprintf( bm_bitmaps[n+i].filename, "%s[%d]", filename, i );
		}

	}

	if (nframes)
		*nframes = anim_frames;

	if (fps)
		*fps = anim_fps;

	if (img_cfp != NULL)
		cfclose(img_cfp);

	return bm_bitmaps[n].handle;
}

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
void bm_get_info( int handle, int *w, int * h, ubyte * flags, int *nframes, int *fps, bitmap_section_info **sections )
{
	bitmap * bmp;

	if ( !bm_inited ) return;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE!	
	
	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_NONE) || (bm_bitmaps[bitmapnum].handle != handle) ) {
		if (w) *w = 0;
		if (h) *h = 0;
		if (flags) *flags = 0;
		if (nframes) *nframes=0;
		if (fps) *fps=0;
		if (sections != NULL) *sections = NULL;
		return;
	}

	bmp = &(bm_bitmaps[bitmapnum].bm);

	if (w) *w = bmp->w;
	if (h) *h = bmp->h;
	if (flags) *flags = bmp->flags;
	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_ANI) || (bm_bitmaps[bitmapnum].type == BM_TYPE_EFF) )	{
		if (nframes) {
			*nframes = bm_bitmaps[bitmapnum].info.ani.num_frames;
		} 
		if (fps) {
			*fps= bm_bitmaps[bitmapnum].info.ani.fps;
		}
	} else {
		if (nframes) {
			*nframes = 1;
		} 
		if (fps) {
			*fps= 0;
		}
	}
	if(sections != NULL){
		*sections = &bm_bitmaps[bitmapnum].bm.sections;
	}
}

uint bm_get_signature( int handle )
{
	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	return bm_bitmaps[bitmapnum].signature;
}

extern int palman_is_nondarkening(int r,int g, int b);
static void bm_convert_format( int bitmapnum, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	int idx;

	if(Pofview_running || Is_standalone){
		Assert(bmp->bpp == 8);

		return;
	} 
	else 
	{
		if(flags & BMP_AABITMAP){
			Assert(bmp->bpp == 8);
		} else {
			Assert( (bmp->bpp == 16) || (bmp->bpp == 32) );
		}
	}

	// maybe swizzle to be an xparent texture
	if(!(bmp->flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_XPARENT)){
		for(idx=0; idx<bmp->w*bmp->h; idx++){			
			// if the pixel is transparent
			if ( ((ushort*)bmp->data)[idx] == Gr_t_green.mask)	{
				((ushort*)bmp->data)[idx] = 0;
			}
		}

		bmp->flags |= BMP_TEX_XPARENT;
	}	
}

// basically, map the bitmap into the current palette. used to be done for all pcx's, now just for
// Fred, since its the only thing that uses the software tmapper
void bm_swizzle_8bit_for_fred(bitmap_entry *be, bitmap *bmp, ubyte *data, ubyte *palette)
{		
/* 2004/10/17 - taylor - no longer needed since FRED is OGL now
	int pcx_xparent_index = -1;
	int i;
	int r, g, b;
	ubyte palxlat[256];

	for (i=0; i<256; i++ ) {
		r = palette[i*3];
		g = palette[i*3+1];
		b = palette[i*3+2];
		if ( g == 255 && r == 0 && b == 0 ) {
			palxlat[i] = 255;
			pcx_xparent_index = i;
		} else {			
			palxlat[i] = (ubyte)(palette_find( r, g, b ));			
		}
	}		
	for (i=0; i<bmp->w * bmp->h; i++ ) {		
		ubyte c = palxlat[data[i]];			
		data[i] = c;		
	}			
	be->palette_checksum = gr_palette_checksum;
*/
}

void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data, *palette;
	ubyte pal[768];
	palette = NULL;
	int pcx_error;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	if ( (bpp == 16) && Cmdline_pcx32) {
		bpp = 32;
	}

	data = (ubyte *)bm_malloc(bitmapnum, bmp->w * bmp->h * (bpp / 8));
	palette = (bpp == 8) ? pal : NULL;
	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = (bpp == 8) ? gr_palette : NULL;
	memset( data, 0, bmp->w * bmp->h * (bpp / 8) );

	Assert( &be->bm == bmp );
	#ifdef BMPMAN_NDEBUG
		Assert( be->data_size > 0 );
	#endif

	// some sanity checks on flags
	Assert(!((flags & BMP_AABITMAP) && (flags & BMP_TEX_ANY)));						// no aabitmap textures
	Assert(!((flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_NONDARK)));			// can't be a transparent texture and a nondarkening texture 

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	if(bpp == 8){
		pcx_error = pcx_read_bitmap_8bpp( filename, data, palette );
		if ( pcx_error != PCX_ERROR_NONE )	{
			// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
			//Error( LOCATION, "Couldn't open '%s'\n", filename );
			//return -1;
		}

		// now swizzle the thing into the proper format
		if(Pofview_running){
			bm_swizzle_8bit_for_fred(be, bmp, data, palette);
		}
	} else if (bpp == 32) {
		pcx_error = pcx_read_bitmap_32( filename, data );
		if ( pcx_error != PCX_ERROR_NONE )	{
			// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
			//Error( LOCATION, "Couldn't open '%s'\n", filename );
			//return -1;
		}
	} else {	
		// load types
		if(flags & BMP_AABITMAP){
			pcx_error = pcx_read_bitmap_16bpp_aabitmap( filename, data );
		} else if(flags & BMP_TEX_NONDARK){
			pcx_error = pcx_read_bitmap_16bpp_nondark( filename, data );
		} else {
			pcx_error = pcx_read_bitmap_16bpp( filename, data );
		}
	}

	if ( pcx_error != PCX_ERROR_NONE )	{
		// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
		//Error( LOCATION, "Couldn't open '%s'\n", filename );
		//return -1;
	}

	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif		
	
	bmp->flags = 0;	

	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim				*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte				*frame_data;
	int				size, i;
	int				first_frame, nframes;	

	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
		anim_free(the_anim);
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}

	bm = &bm_bitmaps[first_frame].bm;
	size = bm->w * bm->h * (bpp / 8);

	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bm = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_free_data( first_frame+i );

		bm->flags = 0;

		// briefing editor in Fred2 uses aabitmaps (ani's) - force to 8 bit
		bm->bpp = Is_standalone ? (ubyte)8 : bpp;

		bm->data = (ptr_u)bm_malloc(first_frame + i, size);

		frame_data = anim_get_next_raw_buffer(the_anim_instance, 0 ,flags & BMP_AABITMAP ? 1 : 0, bm->bpp);

		if ( frame_data == NULL ) {
			// Error(LOCATION,"Fatal error locking .ani file: %s\n", be->filename);
		}		
		
		ubyte *dptr, *sptr;

		sptr = frame_data;
		dptr = (ubyte *)bm->data;

		if ( (bm->w!=the_anim->width) || (bm->h!=the_anim->height) )	{
			// Scale it down
			// Int3();			// not ready yet - should only be ingame
	
			// 8 bit
			if(bpp == 8){
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
				for (h = 0; h < bm->h; h++) {
					ubyte *drow = &dptr[bm->w * h];
					ubyte *srow = &sptr[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}			
			}
			// 16 bpp
			else {
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
				for (h = 0; h < bm->h; h++) {
					ushort *drow = &((ushort*)dptr)[bm->w * h];
					ushort *srow = &((ushort*)sptr)[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}			
			}			
		} else {
			// 1-to-1 mapping
			memcpy(dptr, sptr, size);
		}		

		bm_convert_format( first_frame+i, bm, bpp, flags );

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);
		}

		//mprintf(( "Checksum = %d\n", be->palette_checksum ));
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
}


void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	// int idx;	
	// ushort bit_16;

	// Unload any existing data
	bm_free_data( bitmapnum );	

	if ((bpp != be->info.user.bpp) && !(flags & BMP_AABITMAP))
		bpp = be->info.user.bpp;

	switch( bpp )	{
	case 32:	// user 32-bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;
		bmp->data = (ptr_u)be->info.user.data;
		break;

	case 16:			// user 16 bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = (ptr_u)be->info.user.data;								
		break;	
	
	case 8:			// Going from 8 bpp to something (probably only for aabitmaps)
		/*
		Assert(flags & BMP_AABITMAP);
		bmp->bpp = 16;
		bmp->data = (uint)malloc(bmp->w * bmp->h * 2);
		bmp->flags = be->info.user.flags;
		bmp->palette = NULL;

		// go through and map the pixels
		for(idx=0; idx<bmp->w * bmp->h; idx++){			
			bit_16 = (ushort)((ubyte*)be->info.user.data)[idx];			
			Assert(bit_16 <= 255);

			// stuff the final result
			memcpy((char*)bmp->data + (idx * 2), &bit_16, sizeof(ushort));
		}
		*/		
		Assert(flags & BMP_AABITMAP);
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = (ptr_u)be->info.user.data;								
		break;
		
	// default:
		// Error( LOCATION, "Unhandled user bitmap conversion from %d to %d bpp", be->info.user.bpp, bmp->bpp );
	}

	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int d_size, byte_size;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	// support true color targas when requested
	if (Cmdline_jpgtga) {
		bpp = 32;
	}

	if(Is_standalone){
		Assert(bpp == 8);
	} 
	else 
	{
		Assert( (bpp == 16) || (bpp == 32) );
	}

	// should never try to make an aabitmap out of a targa
	Assert(!(flags & BMP_AABITMAP));

	// allocate bitmap data
	byte_size = (bpp >> 3);
	Assert(byte_size);

	data = (ubyte*)bm_malloc(bitmapnum, bmp->w * bmp->h * byte_size);

	if (data) {
		memset( data, 0, bmp->w * bmp->h * byte_size);
		d_size = byte_size;
 	} else {
		return;
 	}

	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert( &be->bm == bmp );
	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif
	
	int tga_error;

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	if (bpp == 32) {
		tga_error = targa_read_bitmap_32( filename, data, NULL, d_size);
	} else {
		tga_error = targa_read_bitmap( filename, data, NULL, d_size);
	}

	if ( tga_error != TARGA_ERROR_NONE )	{
		// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
		//Error( LOCATION, "Couldn't open '%s'\n", filename );
		//return -1;
		data = NULL;
		return;
	}

	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif		
	
	bmp->flags = 0;	
	
	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

//lock a dds file
void bm_lock_dds( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int error;
	ubyte dds_bpp = 0;
	char filename[MAX_FILENAME_LEN];

	Assert(Texture_compression_enabled);
	Assert(be->mem_taken > 0);

	data = (ubyte*)malloc(be->mem_taken);

	if ( data == NULL )
		return;

	memset( data, 0, be->mem_taken );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	error = dds_read_bitmap( filename, &data, &dds_bpp );

	if (error != DDS_ERROR_NONE) {
		Error(LOCATION, "error loading %s -- %s", filename, dds_error_string(error));
	}

	bmp->bpp = dds_bpp;
	bmp->data = (ptr_u)data;
	bmp->flags = 0;
}

// lock a JPEG file
void bm_lock_jpg( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int d_size = 0;
	int jpg_error = JPEG_ERROR_INVALID;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	// support true color jpegs when requested
	if (Cmdline_jpgtga) {
		bpp = 32;
	} else {
		// not supported yet
		return;
	}

	// should never try to make an aabitmap out of a jpeg
	Assert(!(flags & BMP_AABITMAP));

	// allocate bitmap data 
	if (bpp == 32) {
		data = (ubyte*)bm_malloc(bitmapnum, bmp->w * bmp->h * 4);

		Assert(data);

		if (data) {
			memset( data, 0, bmp->w * bmp->h * 4);
			d_size = 4;
		} else {
			return;
		}
	}
 
	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert( &be->bm == bmp );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	if (bpp == 32) {
		jpg_error = jpeg_read_bitmap_32( filename, data, NULL, d_size);
	}
 
	if ( jpg_error != JPEG_ERROR_NONE )	{
		if (data != NULL) {
			free(data);
			data = NULL;
		}
		return;
	}

	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif
}

MONITOR( NumBitmapPage );
MONITOR( SizeBitmapPage );

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
bitmap * bm_lock( int handle, ubyte bpp, ubyte flags )
{
	bitmap			*bmp;
	bitmap_entry	*be;

	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

//	flags &= (~BMP_RLE);

	// to fix a couple of OGL bpp passes, force 8bit on AABITMAP - taylor
	if (flags & BMP_AABITMAP)
		bpp = 8;

	// if we're on a standalone server, aways for it to lock to 8 bits
	if(Is_standalone){
		bpp = 8;
		flags = 0;
	} 
	// otherwise do it as normal
	else {

		if(Pofview_running){
			Assert( bpp == 8 );
			Assert( (bm_bitmaps[bitmapnum].type == BM_TYPE_PCX) || (bm_bitmaps[bitmapnum].type == BM_TYPE_ANI) || (bm_bitmaps[bitmapnum].type == BM_TYPE_TGA));
		} 
		else 
		{
			if(flags & BMP_AABITMAP){
				Assert( bpp == 8 );
			} else if ((flags & BMP_TEX_NONCOMP) && (!(flags & BMP_TEX_COMP))) {
				Assert( bpp >= 16 );  // cheating but bpp passed isn't what we normally end up with
			}
			else if (flags & BMP_TEX_DXT1){
				Assert( bpp >= 16 ); // cheating but bpp passed isn't what we normally end up with
			}
			else if (flags & (BMP_TEX_DXT3 | BMP_TEX_DXT5)){
				Assert( bpp >= 16 ); // cheating but bpp passed isn't what we normally end up with
			}
			else
			{
				Assert(0);		//?
			}
		}
	}

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	// If you hit this assert, chances are that someone freed the
	// wrong bitmap and now someone is trying to use that bitmap.
	// See John.
	Assert( be->type != BM_TYPE_NONE );		

	// Increment ref count for bitmap since lock was made on it.
	Assert(be->ref_count >= 0);
	be->ref_count++;					// Lock it before we page in data; this prevents a callback from freeing this
											// as it gets read in

	// Mark this bitmap as used this frame
	#ifdef BMPMAN_NDEBUG
	if ( be->used_this_frame < 255 )	{
		be->used_this_frame++;
	}
	#endif

	// read the file data
	if ( gr_bm_lock( be->filename, handle, bitmapnum, be, bmp, bpp, flags ) == -1 ) {
		// oops, this isn't good - reset and return NULL
		bm_unlock( bitmapnum );
		bm_unload( bitmapnum );

		return NULL;
	}

	MONITOR_INC( NumBitmapPage, 1 );
	MONITOR_INC( SizeBitmapPage, bmp->w*bmp->h );

	if ( be->type == BM_TYPE_ANI ) {
		int i,first = bm_bitmaps[bitmapnum].info.ani.first_frame;

		for ( i=0; i< bm_bitmaps[first].info.ani.num_frames; i++ )	{
			// Mark all the bitmaps in this bitmap or animation as recently used
			bm_bitmaps[first+i].last_used = timer_get_milliseconds();

			// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
			#ifdef BMPMAN_NDEBUG
				bm_bitmaps[first+i].used_count++;
			#endif

			bm_bitmaps[first+i].used_flags = flags;
		}
	} else {
		// Mark all the bitmaps in this bitmap or animation as recently used
		be->last_used = timer_get_milliseconds();

		// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
		#ifdef BMPMAN_NDEBUG
			be->used_count++;
		#endif
		be->used_flags = flags;
	}

	return bmp;
}

// Unlocks a bitmap
//
// Decrements the ref_count member of the bitmap_entry struct.  A bitmap can only be unloaded
// when the ref_count is 0.
//
void bm_unlock( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );	// INVALID BITMAP HANDLE

	Assert(bitmapnum >= 0 && bitmapnum < MAX_BITMAPS);

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	be->ref_count--;
	Assert(be->ref_count >= 0);		// Trying to unlock data more times than lock was called!!!
}


void bm_update()
{
}

char *bm_get_filename(int handle)
{
	int n;

	n = handle % MAX_BITMAPS;
	Assert(bm_bitmaps[n].handle == handle);		// INVALID BITMAP HANDLE
	return bm_bitmaps[n].filename;
}

void bm_get_palette(int handle, ubyte *pal, char *name)
{
	char *filename;
	int w,h;

	int n= handle % MAX_BITMAPS;
	Assert( bm_bitmaps[n].handle == handle );		// INVALID BITMAP HANDLE

	filename = bm_bitmaps[n].filename;

	if (name)	{
		strcpy( name, filename );
	}

	int pcx_error=pcx_read_header( filename, NULL, &w, &h, pal );
	if ( pcx_error != PCX_ERROR_NONE ){
		// Error(LOCATION, "Couldn't open '%s'\n", filename );
	}
}

// --------------------------------------------------------------------------------------
// bm_release()  - unloads the bitmap's data and entire slot, so bitmap 'n' won't be valid anymore
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			nothing

void bm_release(int handle)
{
	bitmap_entry	*be;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];

	if ( bm_bitmaps[n].type == BM_TYPE_NONE ) {
		return;	// Already been released?
	}

	if ( bm_bitmaps[n].type != BM_TYPE_USER )	{
		return;
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return;
	}

	bm_free_data(n);

	if ( bm_bitmaps[n].type == BM_TYPE_USER )	{
		bm_bitmaps[n].info.user.data = NULL;
		bm_bitmaps[n].info.user.bpp = 0;
	}

	bm_bitmaps[n].type = BM_TYPE_NONE;

	// Fill in bogus structures!

	// For debugging:
	strcpy( bm_bitmaps[n].filename, "IVE_BEEN_RELEASED!" );
	bm_bitmaps[n].signature = 0xDEADBEEF;									// a unique signature identifying the data
	bm_bitmaps[n].palette_checksum = 0xDEADBEEF;							// checksum used to be sure bitmap is in current palette

	// bookeeping
	#ifdef BMPMAN_NDEBUG
	bm_bitmaps[n].data_size = -1;									// How much data this bitmap uses
	#endif
	bm_bitmaps[n].ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

	// Bitmap info
	bm_bitmaps[n].bm.w = bm_bitmaps[n].bm.h = -1;
	
	// Stuff needed for animations
	// Stuff needed for user bitmaps
	memset( &bm_bitmaps[n].info, 0, sizeof(bm_extra_info) );

	bm_bitmaps[n].handle = -1;
}






// --------------------------------------------------------------------------------------
// bm_unload()  - unloads the data, but not the bitmap info.
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			0		=>		unload failed
//						1		=>		unload successful
//
int bm_unload( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if ( be->type == BM_TYPE_NONE ) {
		return 0;		// Already been released
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE!

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	// be sure that all frames of an ani are unloaded - taylor
	if ( be->type == BM_TYPE_ANI ) {
		int i,first = bm_bitmaps[n].info.ani.first_frame;

		// for the unload all case, don't try to unload every frame of every frame
		// all additional frames automatically get unloaded with the first one
		if (n > bm_bitmaps[n].info.ani.first_frame)
			return 1;

		for ( i=0; i< bm_bitmaps[first].info.ani.num_frames; i++ )	{
			nprintf(("BmpMan", "unloading %s frame %d.  %dx%dx%d\n", be->filename, i, bmp->w, bmp->h, bmp->bpp));
			bm_free_data(first+i);		// clears flags, bbp, data, etc
		}
	} else {
		nprintf(("BmpMan", "unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
		bm_free_data(n);		// clears flags, bbp, data, etc
	}

	return 1;
}


// unload all used bitmaps
void bm_unload_all()
{
	int i;

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			bm_unload(bm_bitmaps[i].handle);
		}
	}
}


DCF(bmpman,"Shows/changes bitmap caching parameters and usage")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "flush" ))	{
			dc_printf( "Total RAM usage before flush: %d bytes\n", bm_texture_ram );
			int i;
			for (i = 0; i < MAX_BITMAPS; i++)	{
				if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
					bm_free_data(i);
				}
			}
			dc_printf( "Total RAM after flush: %d bytes\n", bm_texture_ram );
		} else if ( !strcmp( Dc_arg, "ram" ))	{
			dc_get_arg(ARG_INT);
			Bm_max_ram = Dc_arg_int*1024*1024;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: BmpMan keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "BmpMan flush    Unloads all bitmaps.\n" );
		dc_printf( "BmpMan ram x    Sets max mem usage to x MB. (Set to 0 to have no limit.)\n" );
		dc_printf( "\nUse '? BmpMan' to see status of Bitmap manager.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Total RAM usage: %d bytes\n", bm_texture_ram );


		if ( Bm_max_ram > 1024*1024 )
			dc_printf( "Max RAM allowed: %.1f MB\n", i2fl(Bm_max_ram)/(1024.0f*1024.0f) );
		else if ( Bm_max_ram > 1024 )
			dc_printf( "Max RAM allowed: %.1f KB\n", i2fl(Bm_max_ram)/(1024.0f) );
		else if ( Bm_max_ram > 0 )
			dc_printf( "Max RAM allowed: %d bytes\n", Bm_max_ram );
		else
			dc_printf( "No RAM limit\n" );


	}
}

// Marks a texture as being used for this level
void bm_page_in_texture( int bitmapnum, int nframes )
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	for (i=0; i<nframes;i++ )	{
		bm_bitmaps[n+i].preloaded = 1;

	//	bm_bitmaps[n+i].preload_count++;

		bm_bitmaps[n+i].used_flags = BMP_TEX_OTHER;

		//check if its compressed
		if (OGL_enabled) {
			switch (bm_bitmaps[n+i].type) {
				case BM_TYPE_DXT1:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT1;
					continue;
				
				case BM_TYPE_DXT3:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT3;
					continue;

				case BM_TYPE_DXT5:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT5;
					continue;
			}
		}
	}
}

// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_nondarkening_texture( int bitmapnum, int nframes )
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	for (i=0; i<nframes;i++ )	{
		bm_bitmaps[n+i].preloaded = 4;

	//	bm_bitmaps[n+i].preload_count++;

		bm_bitmaps[n+i].used_flags = BMP_TEX_NONDARK;
	}
}

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_xparent_texture( int bitmapnum, int nframes)
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	for (i=0; i<nframes;i++ )	{
		bm_bitmaps[n+i].preloaded = 3;

	//	bm_bitmaps[n+i].preload_count++;

		bm_bitmaps[n+i].used_flags = BMP_TEX_XPARENT;

		//check if its compressed
		if (OGL_enabled) {
			switch (bm_bitmaps[n+i].type) {
				case BM_TYPE_DXT1:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT1;
					continue;
					
				case BM_TYPE_DXT3:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT3;
					continue;

				case BM_TYPE_DXT5:
					bm_bitmaps[n+i].used_flags = BMP_TEX_DXT5;
					continue;
			}
		}
	}
}

// Marks an aabitmap as being used for this level
void bm_page_in_aabitmap( int bitmapnum, int nframes )
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	for (i=0; i<nframes;i++ )	{
		bm_bitmaps[n+i].preloaded = 2;

	//	bm_bitmaps[n+i].preload_count++;
	
		bm_bitmaps[n+i].used_flags = BMP_AABITMAP;
	}
}



// Tell the bitmap manager to start keeping track of what bitmaps are used where.
void bm_page_in_start()
{
	int i;

	Bm_paging = 1;

	// Mark all as inited
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
		//	bm_unload(bm_bitmaps[i].handle);
		}
		bm_bitmaps[i].preloaded = 0;
	//	bm_bitmaps[i].preload_count = 0;
		#ifdef BMPMAN_NDEBUG
			bm_bitmaps[i].used_count = 0;
		#endif
		bm_bitmaps[i].used_flags = 0;
	}

	gr_bm_page_in_start();
}

void bm_page_in_stop()
{	
	int i;	
	int ship_info_index;

	nprintf(( "BmpInfo","BMPMAN: Loading all used bitmaps.\n" ));

	// Load all the ones that are supposed to be loaded for this level.
	int n = 0;

	#ifdef BMPMAN_NDEBUG
	Bm_ram_freed = 0;
	#endif

	int bm_preloading = 1;

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			if ( bm_bitmaps[i].preloaded )	{
#ifdef BMPMAN_SPECIAL_NONDARK
				// if this is a texture, check to see if a ship uses it
				ship_info_index = ship_get_texture(bm_bitmaps[i].handle);
				// use the colors from this ship
				if((ship_info_index >= 0) && (Ship_info[ship_info_index].num_nondark_colors > 0)){
					// mprintf(("Using custom pixels for %s\n", Ship_info[ship_info_index].name));
					palman_set_nondarkening(Ship_info[ship_info_index].nondark_colors, Ship_info[ship_info_index].num_nondark_colors);
				}
				// use the colors from the default table
				else {
					// mprintf(("Using default pixels\n"));
					palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
				}
#endif

				// if preloaded == 3, load it as an xparent texture				
				if(bm_bitmaps[i].used_flags == BMP_AABITMAP){
					bm_lock( bm_bitmaps[i].handle, 8, bm_bitmaps[i].used_flags );
				} else if (bm_bitmaps[i].used_flags == BMP_TEX_DXT1) {
					bm_lock( bm_bitmaps[i].handle, 24, bm_bitmaps[i].used_flags );
				} else if (bm_bitmaps[i].used_flags == (BMP_TEX_DXT3 | BMP_TEX_DXT5)) {
					bm_lock( bm_bitmaps[i].handle, 32, bm_bitmaps[i].used_flags );
				} else {
					bm_lock( bm_bitmaps[i].handle, 16, bm_bitmaps[i].used_flags );
				}

				bm_unlock( bm_bitmaps[i].handle );

				if ( bm_preloading )	{
					if ( !gr_preload(bm_bitmaps[i].handle, (bm_bitmaps[i].preloaded==2) ) )	{
						mprintf(( "Out of VRAM.  Done preloading.\n" ));
						bm_preloading = 0;
					}
				}

				n++;

				#ifdef BMPMAN_NDEBUG
				if ( Bm_ram_freed )	{
					nprintf(( "BmpInfo","BMPMAN: Not enough cache memory to load all level bitmaps\n" ));
					break;
				}
				#endif
			} else {
				bm_unload(bm_bitmaps[i].handle);
			}
		}
		game_busy();
	}
	nprintf(( "BmpInfo","BMPMAN: Loaded %d bitmaps that are marked as used for this level.\n", n ));

	int total_bitmaps = 0;
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			total_bitmaps++;
		}
		if ( bm_bitmaps[i].type == BM_TYPE_USER )	{
			mprintf(( "User bitmap '%s'\n", bm_bitmaps[i].filename ));
		}
	}	

	mprintf(( "Bmpman: %d/%d bitmap slots in use.\n", total_bitmaps, MAX_BITMAPS ));
	//mprintf(( "Bmpman: Usage went from %d KB to %d KB.\n", usage_before/1024, usage_after/1024 ));

	Bm_paging = 0;
}

/* for unloading bitmaps while a mission is going, not currently used
void bm_page_out( int bitmap_id )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( n >= 0 && n < MAX_BITMAPS );
	Assert( bm_bitmaps[n].handle == bitmap_id );	// INVALID BITMAP HANDLE
	
	if (bm_bitmaps[n].preload_count < 1)
		return;

	mprintf(("PAGE-OUT: %s - preload_count: %d\n", bm_bitmaps[n].filename, bm_bitmaps[n].preload_count - 1));

	if ( --bm_bitmaps[n].preload_count == 0 ) {
		bm_unload( bitmap_id );
	}
}
*/

int bm_get_cache_slot( int bitmap_id, int separate_ani_frames )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( bm_bitmaps[n].handle == bitmap_id );		// INVALID BITMAP HANDLE

	bitmap_entry	*be = &bm_bitmaps[n];

	if ( (!separate_ani_frames) && ((be->type == BM_TYPE_ANI) || (be->type == BM_TYPE_EFF)) )	{
		return be->info.ani.first_frame;
	} 

	return n;

}

void (*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) = NULL;
/*
void bm_set_components_argb(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	// rgba 
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((ushort*)pixel) &= ~(0x8000);
	if(*av){
		*((ushort*)pixel) |= 0x8000;
	}
}

void bm_set_components_d3d(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	// rgba 
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	if(*av == 0){ 
		*((ushort*)pixel) = (ushort)Gr_current_green->mask;
	}
}
*/
void bm_set_components_argb_d3d_16_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	if(*av == 0){				
		*((ushort*)pixel) = (ushort)Gr_current_green->mask;
	}			
}

void bm_set_components_argb_d3d_32_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((uint*)pixel) |= (uint)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((uint*)pixel) |= (uint)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((uint*)pixel) |= (uint)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	if(*av == 0){				
		*((uint*)pixel) = (uint)Gr_current_green->mask;		
	}
}

void bm_set_components_argb_d3d_16_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((ushort*)pixel) &= ~(Gr_current_alpha->mask);
	if(*av){
		*((ushort*)pixel) |= (ushort)(Gr_current_alpha->mask);
	} else {
		*((ushort*)pixel) = 0;
	}
}

void bm_set_components_argb_d3d_32_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((ushort*)pixel) &= ~(Gr_current_alpha->mask);
	if(*av){
		*((ushort*)pixel) |= (ushort)(Gr_current_alpha->mask);
	} else {
		*((ushort*)pixel) = 0;
	}
}
/*
void bm_set_components_opengl(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	// rgba 
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((ushort*)pixel) &= ~(0x8000);
	if (*((ushort*)pixel) == (ushort)Gr_current_green->mask) {
		*((ushort*)pixel) = 0;
	} else {
		if(*av){
			*((ushort*)pixel) |= 0x8000;
		}
	}
}
*/

// for selecting pixel formats
void BM_SELECT_SCREEN_FORMAT()
{
	Gr_current_red = &Gr_red;
	Gr_current_green = &Gr_green;
	Gr_current_blue = &Gr_blue;
	Gr_current_alpha = &Gr_alpha;

	// setup pointers
#ifndef NO_DIRECT3D
	if (gr_screen.mode == GR_DIRECT3D) {
		if (gr_screen.bits_per_pixel == 32) {
			bm_set_components = bm_set_components_argb_d3d_32_screen;
		} else {
			bm_set_components = bm_set_components_argb_d3d_16_screen;
		}
		bm_set_components = bm_set_components_argb_d3d_16_screen;
	} else
#endif // !NO_DIRECT3D
	if (gr_screen.mode == GR_OPENGL) {
		if (gr_screen.bits_per_pixel == 32) {
				bm_set_components = bm_set_components_argb_d3d_32_screen;
		} else {
				bm_set_components = bm_set_components_argb_d3d_16_screen;
		}
	}
}

void BM_SELECT_TEX_FORMAT()
{
	Gr_current_red = &Gr_t_red; 
	Gr_current_green = &Gr_t_green; 
	Gr_current_blue = &Gr_t_blue; 
	Gr_current_alpha = &Gr_t_alpha;

	// setup pointers
#ifndef NO_DIRECT3D
	if (gr_screen.mode == GR_DIRECT3D) {
		if (gr_screen.bits_per_pixel == 32) {
			bm_set_components = bm_set_components_argb_d3d_32_tex;
		} else {
			bm_set_components = bm_set_components_argb_d3d_16_tex;
		}
		bm_set_components = bm_set_components_argb_d3d_16_tex;
	} else
#endif // !NO_DIRECT3D
	if (gr_screen.mode == GR_OPENGL) {
		if (gr_screen.bits_per_pixel == 32) {
				bm_set_components = bm_set_components_argb_d3d_32_tex;
		} else {
				bm_set_components = bm_set_components_argb_d3d_16_tex;
		}
	}
}

void BM_SELECT_ALPHA_TEX_FORMAT()
{
	Gr_current_red = &Gr_ta_red; 
	Gr_current_green = &Gr_ta_green; 
	Gr_current_blue = &Gr_ta_blue; 
	Gr_current_alpha = &Gr_ta_alpha;

	// setup pointers
#ifndef NO_DIRECT3D
	if(gr_screen.mode == GR_DIRECT3D){
		if (gr_screen.bits_per_pixel == 32) {
			bm_set_components = bm_set_components_argb_d3d_32_tex;
		} else {
			bm_set_components = bm_set_components_argb_d3d_16_tex;
		}
		bm_set_components = bm_set_components_argb_d3d_16_tex;
	} else
#endif // !NO_DIRECT3D
	if (gr_screen.mode == GR_OPENGL) {
		if (gr_screen.bits_per_pixel == 32) {
				bm_set_components = bm_set_components_argb_d3d_32_tex;
		} else {
				bm_set_components = bm_set_components_argb_d3d_16_tex;
		}
	}
}

// set the rgba components of a pixel, any of the parameters can be -1
/*
void bm_set_components(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	int bit_32 = 0;

	// pick a byte size - 32 bits only if 32 bit mode d3d and screen format
	if(D3D_32bit && (Gr_current_red == &Gr_red)){
		bit_32 = 1;
	}	
	
	if(bit_32){
		*((uint*)pixel) |= (uint)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	} else {
		*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	}		
	if(bit_32){
		*((uint*)pixel) |= (uint)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	} else {
		*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	}		
	if(bit_32){
		*((uint*)pixel) |= (uint)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	} else {
		*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	}
	
	// NOTE - this is a semi-hack. For direct3d we don't use an alpha bit, so if *av == 0, we just set the whole pixel to be Gr_green.mask
	//        ergo, we need to do this _last_
	switch(Bm_pixel_format){
	// glide has an alpha channel so we have to unset ir or set it each time
	case BM_PIXEL_FORMAT_ARGB:
		Assert(!bit_32);
		*((ushort*)pixel) &= ~(0x8000);
		if(*av){
			*((ushort*)pixel) |= 0x8000;
		}
		break;
	
	// this d3d format has no alpha channel, so only make it "transparent", never make it "non-transparent"
	case BM_PIXEL_FORMAT_D3D:			
		Assert(!bit_32);
		if(*av == 0){ 
			*((ushort*)pixel) = (ushort)Gr_current_green->mask;
		}
		break;

	// nice 1555 texture format
	case BM_PIXEL_FORMAT_ARGB_D3D:						
		// if we're writing to normal texture format
		if(Gr_current_red == &Gr_t_red){					
			Assert(!bit_32);
			*((ushort*)pixel) &= ~(Gr_current_alpha->mask);
			if(*av){
				*((ushort*)pixel) |= (ushort)(Gr_current_alpha->mask);
			} else {
				*((ushort*)pixel) = 0;
			}
		}
		// otherwise if we're writing to screen format, still do it the green mask way
		else {			
			if(*av == 0){
				if(bit_32){
					*((uint*)pixel) = (uint)Gr_current_green->mask;
				} else {
					*((ushort*)pixel) = (ushort)Gr_current_green->mask;
				}
			}
		}			
		break;
	}	
}
*/

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a)
{
	int bit_32 = 0;

	if((gr_screen.bits_per_pixel==32) && (Gr_current_red == &Gr_red)){
		bit_32 = 1;
	}


	if(r != NULL){
		if(bit_32){
			*r = ubyte(( (*((uint*)pixel) & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		} else {
			*r = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		}
	}
	if(g != NULL){
		if(bit_32){
			*g = ubyte(( (*((uint*)pixel) & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		} else {
			*g = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		}
	}
	if(b != NULL){
		if(bit_32){
			*b = ubyte(( (*((uint*)pixel) & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		} else {
			*b = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		}
	}

	// get the alpha value
	if(a != NULL){		
		*a = 1;

	//	switch(Bm_pixel_format){
		// glide has an alpha channel so we have to unset ir or set it each time
	//	case BM_PIXEL_FORMAT_ARGB:			
			Assert(!bit_32);
			if(!( ((ushort*)pixel)[0] & 0x8000)){
				*a = 0;
			} 
	//		break;
/*
		// this d3d format has no alpha channel, so only make it "transparent", never make it "non-transparent"
		case BM_PIXEL_FORMAT_D3D:
			Assert(!bit_32);
			if( *((ushort*)pixel) == Gr_current_green->mask){ 
				*a = 0;
			}
			break;

		// nice 1555 texture format mode
		case BM_PIXEL_FORMAT_ARGB_D3D:	
			// if we're writing to a normal texture, use nice alpha bits
			if(Gr_current_red == &Gr_t_red){				
				Assert(!bit_32);

				if(!(*((ushort*)pixel) & Gr_current_alpha->mask)){
					*a = 0;
				}
			}
			// otherwise do it as normal
			else {
				if(bit_32){
					if(*((int*)pixel) == Gr_current_green->mask){ 
						*a = 0;
					}
				} else {
					if(*((ushort*)pixel) == Gr_current_green->mask){ 
						*a = 0;
					}
				}
			}
		}*/
	}
}

// get filename
void bm_get_filename(int bitmapnum, char *filename)
{
	int n = bitmapnum % MAX_BITMAPS;

	// return filename
	strcpy(filename, bm_bitmaps[n].filename);
}

// given a bitmap and a section, return the size (w, h)
void bm_get_section_size(int bitmapnum, int sx, int sy, int *w, int *h)
{
	int bw, bh;
	bitmap_section_info *sections;

	// bogus input?
	Assert((w != NULL) && (h != NULL));
	if((w == NULL) || (h == NULL)){
		return;
	}

	// get bitmap info
	bm_get_info(bitmapnum, &bw, &bh, NULL, NULL, NULL, &sections);

	// determine the width and height of this section
	*w = sx < (sections->num_x - 1) ? MAX_BMAP_SECTION_SIZE : bw - sections->sx[sx];
	*h = sy < (sections->num_y - 1) ? MAX_BMAP_SECTION_SIZE : bh - sections->sy[sy];										
}

// needed only for compressed bitmaps
int bm_is_compressed(int num)
{
	int n = num % MAX_BITMAPS;

	//duh
	if (!Texture_compression_enabled)
		return 0;

	Assert(num == bm_bitmaps[n].handle);

	switch (bm_bitmaps[n].type) {
		case BM_TYPE_DXT1:
			return DDS_DXT1;
		case BM_TYPE_DXT3:
			return DDS_DXT3;
		case BM_TYPE_DXT5:
			return DDS_DXT5;
	}
	return 0;
}

// the only real purpose for this is to return the correct TCACHE_TYPE for compressed graphics,
// uncompressed graphics are assumed to be of type NORMAL.  The only other real format to check
// for is TCACHE_TYPE_SECTIONED - taylor
int bm_get_tcache_type(int num)
{
	if ( bm_is_compressed(num) )
		return TCACHE_TYPE_COMPRESSED;

	return TCACHE_TYPE_NORMAL;
}

int bm_get_size(int num)
{
	int n = num % MAX_BITMAPS;

	Assert(num == bm_bitmaps[n].handle);
	
	if (!bm_is_compressed(num))
		return 0;

	return bm_bitmaps[n].mem_taken;
}

int bm_get_num_mipmaps(int num)
{
	int n = num % MAX_BITMAPS;

	Assert( num == bm_bitmaps[n].handle );

	if (bm_bitmaps[n].num_mipmaps == 0)
		return 1;

	return bm_bitmaps[n].num_mipmaps;
}

// list of all bitmaps loaded, but not necessarily in memory
// used to debug bmpman after a mission load
void bm_print_bitmaps()
{
#ifdef BMPMAN_NDEBUG
	int i;

	for (i=0; i<MAX_BITMAPS; i++) {
		if (bm_bitmaps[i].type != BM_TYPE_NONE) {
			if (bm_bitmaps[i].data_size) {
				nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d (%s) size: %.3fM\n", i, bm_bitmaps[i].filename, bm_bitmaps[i].handle, bm_bitmaps[i].data_size ? NOX("*LOCKED*") : NOX(""), ((float)bm_bitmaps[i].data_size/1024.0f)/1024.0f));
			} else {
				nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d\n", i, bm_bitmaps[i].filename, bm_bitmaps[i].handle));
			}
		}
	}
	nprintf(("BMP DEBUG", "BMPMAN = LOCKED memory usage: %.3fM\n", ((float)bm_texture_ram/1024.0f)/1024.0f));
#endif
}
