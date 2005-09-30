/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Freespace2/FreeSpace.cpp $
 * $Revision: 2.183 $
 * $Date: 2005-09-30 09:47:06 $
 * $Author: taylor $
 *
 * Freespace main body
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.182  2005/09/27 05:25:18  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 * Revision 2.181  2005/09/26 02:15:02  Goober5000
 * okay, this should all be working :)
 * --Goober5000
 *
 * Revision 2.180  2005/09/25 18:44:51  taylor
 * fix Subsys_status leak, wasn't a problem in game but can be touchy on exit
 *
 * Revision 2.179  2005/09/25 08:25:14  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.178  2005/09/25 05:13:05  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.177  2005/09/18 20:41:10  wmcoolmon
 * Fixed silly i variable placement mistake
 *
 * Revision 2.176  2005/09/06 00:32:19  Kazan
 * fixed a bug related to multiplayer table validation and modular tables
 *
 * Revision 2.175  2005/09/05 09:38:18  taylor
 * merge of OSX tree
 * a lot of byte swaps were still missing, will hopefully be fully network compatible now
 *
 * Revision 2.174  2005/08/26 00:56:17  taylor
 * send out multi pings during level load to prevent timeouts on slower computers (does not break network compatibility with older, or retail, clients/servers)
 *
 * Revision 2.173  2005/08/25 22:40:02  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.172  2005/08/20 20:34:49  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.171  2005/08/14 17:20:55  Kazan
 * diabled NEW_MALLOC on windows - it was causing crashing - must have been corrupting it's own heap
 *
 * Revision 2.170  2005/08/11 12:22:26  taylor
 * don't do any ambient sound or music stuff in game_pause()/unpause() on options screens when in mission so we can
 *   avoid having mainhall ambient sounds and music start playing and then not stop until you exit the mission
 *
 * Revision 2.169  2005/07/25 05:55:40  Goober5000
 * forgot this
 * --Goober5000
 *
 * Revision 2.168  2005/07/22 10:18:38  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.167  2005/07/18 14:09:03  taylor
 * better "is_640x480" detection, 800x600 res was using GR_1024 where it should have used GR_640
 *
 * Revision 2.166  2005/07/18 03:44:00  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.165  2005/07/16 07:03:49  wmcoolmon
 * Unlike my previous commit, this one does not cause a segmentation
 * fault. :)
 *
 * Revision 2.164  2005/07/15 23:19:10  wmcoolmon
 * I can't compile the source to test this, but this should switch the
 * hi-res check to "2_ChoosePilot-m" rather than sparky_hi_fs2.
 *
 * Revision 2.163  2005/07/13 02:50:52  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.162  2005/07/13 02:30:52  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 2.161  2005/07/13 00:44:21  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.160  2005/07/09 04:34:43  taylor
 * make sure to tack on a / on the dot dir so that recursive directory making will work properly (code thought it was a file rather than a dir)
 *
 * Revision 2.159  2005/07/07 16:33:39  taylor
 * add value of GL_vertex_data_in to OGL card memory usage
 *
 * Revision 2.158  2005/06/22 15:16:40  taylor
 * make sure we only have one extension on mission names
 *
 * Revision 2.157  2005/06/19 09:00:09  taylor
 * minor sanity checking for geometry_batcher
 * make particle batchers allocate dynamically
 * handle cases where a particle graphic couldn't be loaded
 *
 * Revision 2.156  2005/06/19 02:28:55  taylor
 * add a _fast version of bm_unload() to be used in modelinterp and future graphics API code
 * clean up some modelinterp code to not use memcpy() everywhere so it's more platform compatible and matches old code (Jens Granseuer)
 * NaN check to catch shards-of-death and prevent hitting an Assert() (Jens Granseuer)
 * fix jumpnode code to catch model errors and close a memory leak
 * make the call to bm_unload_all() after model_free_all() since we will get bmpman screwups otherwise
 * don't show hardware sound RAM when using OpenAL build, it will always be 0
 * print top-right memory figures in debug builds slighly further over when 1024+ res
 *
 * Revision 2.155  2005/06/03 06:39:25  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.154  2005/06/03 02:28:54  wmcoolmon
 * Small extern fix
 *
 * Revision 2.153  2005/05/30 05:30:02  taylor
 * make sure we stop mission in the redalert case too
 *
 * Revision 2.152  2005/05/15 21:40:31  phreak
 * custom loading screen fixage.
 *
 * Revision 2.151  2005/05/13 20:55:19  phreak
 * possible fix for seeing suns through ships.  shouldn't break anything.
 *
 * Revision 2.150  2005/05/12 17:40:48  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 * use correct GL VRAM texture number in OGL mode rather than the D3D number
 *
 * Revision 2.149  2005/05/12 03:51:17  Goober5000
 * whoops
 * --Goober5000
 *
 * Revision 2.148  2005/05/11 02:16:17  phreak
 * clean out some old code
 *
 * Revision 2.147  2005/05/11 02:15:07  phreak
 * fix the problem where the fade in/out doesn't work if looking at a sun.
 *
 * Revision 2.146  2005/05/08 20:39:10  wmcoolmon
 * Code betterness
 *
 * Revision 2.145  2005/05/08 20:33:42  wmcoolmon
 * Turret debugging code; Medals stuff; GUI_system change
 *
 * Revision 2.144  2005/04/28 05:29:29  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.143  2005/04/25 00:21:30  wmcoolmon
 * Center title screen, removal of unnecessary #ifdefs
 *
 * Revision 2.142  2005/04/21 15:58:07  taylor
 * initial changes to mission loading and status in debug builds
 *  - move bmpman page in init to an earlier stage to avoid unloading sexp loaded images
 *  - small changes to progress reports in debug builds so that it's easier to tell what's slow
 *  - initialize the loading screen before mission_parse() so that we'll be about to get a more accurate load time
 * fix memory leak in gamesnd (yes, I made a mistake ;))
 * make sure we unload models on game shutdown too
 *
 * Revision 2.141  2005/04/16 03:36:12  wmcoolmon
 * Minor changes; made even more fields in ships.tbl optional.
 *
 * Revision 2.140  2005/04/15 06:59:05  wmcoolmon
 * One final oops (hopefully).
 *
 * Revision 2.139  2005/04/12 02:10:09  phreak
 * ambient light is set in mission
 *
 * Revision 2.138  2005/04/11 05:45:38  taylor
 * _endthread() doesn't take an argument so do support one (Jens Granseuer)
 * debug variable fixes in freespace.cpp (Jens Granseuer)
 * as a safety catch we should Assert() on future usage of things we don't yet support in *nix _splitpath()
 *
 * Revision 2.137  2005/04/05 05:53:16  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.136  2005/03/25 06:57:33  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.135  2005/03/24 23:31:45  taylor
 * make sounds.tbl dynamic
 * "filename" will never be larger than 33 chars so having it 260 is a waste (freespace.cpp)
 *
 * Revision 2.134  2005/03/16 01:35:58  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.133  2005/03/14 06:36:30  wmcoolmon
 * Added memory display thingy to loading screen, if command line is specified and build is a debug build. Also made screenshots go up to 9999
 *
 * Revision 2.132  2005/03/13 08:04:43  taylor
 * fix various compiler warning messages
 * make sure screenshot count doesn't get too high
 * fix movies playing twice in redalert missions
 * reset time compression for redalert missions and cmdbriefings, just in case
 *
 * Revision 2.131  2005/03/10 08:00:02  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.130  2005/03/07 13:10:20  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.129  2005/03/03 12:58:25  Kazan
 * !Om_tracker_flag = non-pxo
 *
 * Revision 2.128  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.127  2005/03/03 05:32:11  taylor
 * well that was good and wrong
 *
 * Revision 2.126  2005/03/02 21:24:43  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.125  2005/03/01 06:55:40  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.124  2005/02/23 04:51:55  taylor
 * some bm_unload() -> bm_release() changes to save bmpman slots
 *
 * Revision 2.123  2005/02/14 23:54:10  taylor
 * make loading screen shader a bit taller
 * add i.o to credits for Linux and OSX code
 * add libjpeg and ogg stuff to credits for license compliance
 * replace an Int3() with a debug message in the hud code
 *
 * Revision 2.122  2005/02/04 10:12:29  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.121  2005/01/30 09:27:39  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 2.120  2005/01/29 09:19:45  argv
 * Fixed compile errors due to several source files not having been updated to
 * reference "Min/Max_draw_distance" instead of "MIN/MAX_DRAW_DISTANCE".
 *
 * -- _argv[-1]
 *
 * Revision 2.119  2005/01/16 22:39:08  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.118  2005/01/08 10:00:59  wmcoolmon
 * Sound quality in Freespace 2 is now controlled by SoundSampleBits, and SoundSampleRate. Also, some sounds will use hardware rather than software buffers if available.
 *
 * Revision 2.117  2005/01/01 07:18:47  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.116  2004/12/25 09:28:41  wmcoolmon
 * Sync to current NEW_HUD code
 *
 * Revision 2.115  2004/12/25 09:24:54  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD
 *
 * Revision 2.114  2004/12/10 17:21:00  taylor
 * dymanic allocation of Personas
 *
 * Revision 2.113  2004/12/02 11:18:14  taylor
 * make OGL use same gamma reg setting as D3D since it's the same ramp
 * have OGL respect the -no_set_gamma cmdline option
 * reset gamma to default on OGL exit to make sure the desktop doesn't stay wrong
 *
 * Revision 2.112  2004/11/26 08:41:11  taylor
 * couple of LAN game vs. FS2NetD fixes
 *
 * Revision 2.111  2004/10/31 21:31:34  taylor
 * bump COUNT_ESTIMATE, reset time compression at the start of a mission, new pilot file support, add feature_disabled popup
 *
 * Revision 2.110  2004/08/23 07:48:08  Goober5000
 * fix0red some warnings
 * --Goober5000
 *
 * Revision 2.109  2004/08/05 02:15:27  phreak
 * forgot to commit orb change
 *
 * Revision 2.108  2004/07/26 21:19:05  Goober5000
 * updated launcher link
 * --Goober5000
 *
 * Revision 2.107  2004/07/26 20:47:28  Kazan
 * remove MCD complete
 *
 * Revision 2.106  2004/07/26 17:54:04  Kazan
 * Autopilot system completed -- i am dropping plans for GUI nav map
 * Fixed FPS counter during time compression
 *
 * Revision 2.105  2004/07/18 04:07:26  Kazan
 * nothing major
 *
 * Revision 2.104  2004/07/17 18:46:06  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.103  2004/07/12 16:32:46  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.102  2004/07/12 03:19:15  Kazan
 * removed a couple pointless useless messages from the debug console
 *
 * Revision 2.101  2004/07/11 03:22:47  bobboau
 * added the working decal code
 *
 * Revision 2.100  2004/07/08 22:08:21  wmcoolmon
 * Moved set_current_hud, as well as added a conditional to hud_positions_it; it shouldn't and won't be executed on a standalone server.
 *
 * Revision 2.99  2004/07/01 16:38:18  Kazan
 * working on autonav
 *
 * Revision 2.98  2004/07/01 02:06:31  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.97  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.96  2004/06/22 23:14:09  wmcoolmon
 * Nonworking OGG support for sound (not music) added, disabled load-only-used-weapons code, modification to sound system registry code.
 * OGG code has been commented out, so you don't need the SDK yet.
 *
 * Revision 2.95  2004/06/19 12:45:47  randomtiger
 * Changed startup code to catch D3D9 request from launcher.
 * No actual startup code has been implemented.
 *
 * Revision 2.94  2004/06/18 04:59:53  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.93  2004/05/30 08:04:48  wmcoolmon
 * Final draft of the HUD parsing system structure. May change how individual coord positions are specified in the TBL. -C
 *
 * Revision 2.92  2004/05/27 00:49:25  wmcoolmon
 * Made HUD.tbl obsolete. Info is now taken directly from $Shield_icon in ships.tbl
 * Now this table can be used for something more useful...say, hud gauge positions?
 *
 * Revision 2.91  2004/05/03 21:22:20  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.90  2004/04/27 20:41:58  taylor
 * add NULL check to game_hacked_data() to prevent crash
 *
 * Revision 2.89  2004/04/26 18:23:48  Kazan
 * -no_fps_capping
 *
 * Revision 2.88  2004/04/26 00:25:08  taylor
 * don't preload every sound (fixes some slowdowns), snd_preload option support
 *
 * Revision 2.87  2004/04/14 23:17:26  taylor
 * let language reg setting override auto detection
 *
 * Revision 2.86  2004/04/08 05:00:13  righteous1
 * Backing out mainhall audio fix attempt.
 *
 * Revision 2.85  2004/04/07 03:31:53  righteous1
 * Updated to add alt_tab_pause() function to draw pause screen and discontinue sounds when the game is minimized. -R1
 *
 * Revision 2.84  2004/03/30 08:30:38  taylor
 * handle movie file extensions better
 *
 * Revision 2.83  2004/03/28 17:49:54  taylor
 * runtime language selection, mantis:0000133
 *
 * Revision 2.82  2004/03/20 14:47:13  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.81  2004/03/19 14:51:55  randomtiger
 * New command line parameter: -d3d_lesstmem causes D3D to bypass V's secondry texture system.
 *
 * Revision 2.80  2004/03/16 18:37:02  randomtiger
 * Added new launcher flag construction code.
 *
 * Revision 2.79  2004/03/08 22:16:32  phreak
 * fugly hack to get skyboxes working in HT&L.  please port stars_draw() to use HT&L
 * so this hack can be removed
 *
 * Revision 2.78  2004/03/08 22:02:38  Kazan
 * Lobby GUI screen restored
 *
 * Revision 2.77  2004/03/08 18:36:20  randomtiger
 * Added complete stub system to replace software.
 *
 * Revision 2.76  2004/03/07 23:07:19  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 2.75  2004/03/05 21:19:38  Kazan
 * Fixed mission validation (was returning false positives)
 *
 * Revision 2.74  2004/03/05 09:02:01  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.73  2004/02/20 18:04:27  randomtiger
 * Changed memory leak detection to happen after atexit all other calls.
 * This will mean it doesnt report leaks that are cleared by just before the game quits by preset functions.
 *
 * Revision 2.72  2004/02/20 04:29:53  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.71  2004/02/16 21:22:15  randomtiger
 * Use _REPORT_MEM_LEAKS compile flag in code.lib to get a report of memory leaks from malloc calls.
 *
 * Revision 2.70  2004/02/16 11:47:32  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.69  2004/02/14 00:18:30  randomtiger
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
 * Revision 2.68  2004/01/29 01:34:01  randomtiger
 * Added malloc montoring system, use -show_mem_usage, debug exes only to get an ingame list of heap usage.
 * Also added -d3d_notmanaged flag to activate non managed D3D path, in experimental stage.
 *
 * Revision 2.67  2004/01/24 15:52:25  randomtiger
 * I have submitted the new movie playing code despite the fact in D3D it sometimes plays behind the main window.
 * In OGL it works perfectly and in both API's it doesnt switch to the desktop anymore so hopefully people will not experience the crashes etc that the old system used to suffer from.
 *
 * Revision 2.66  2004/01/23 00:14:27  randomtiger
 * Fixed serious memory leak causes by use of -pcx32 flag
 *
 * Revision 2.65  2004/01/21 17:32:05  phreak
 * added version string message if build is an inferno build
 *
 * Revision 2.64  2003/12/03 19:27:00  randomtiger
 * Changed -t32 flag to -jpgtga
 * Added -query_flag to identify builds with speech not compiled and other problems
 * Now loads up launcher if videocard reg entry not found
 * Now offers to go online to download launcher if its not present
 * Changed target view not to use lower res texture, hi res one is already chached so might as well use it
 *
 * Revision 2.63  2003/11/29 10:52:09  randomtiger
 * Turned off D3D file mapping, its using too much memory which may be hurting older systems and doesnt seem to be providing much of a speed benifit.
 * Added stats command for ingame stats on memory usage.
 * Trys to play intro.mve and intro.avi, just to be safe since its not set by table.
 * Added fix for fonts wrapping round in non standard hi res modes.
 * Changed D3D mipmapping to a good value to suit htl mode.
 * Added new fog colour method which makes use of the bitmap, making this htl feature backcompatible again.
 *
 * Revision 2.62  2003/11/19 20:37:23  randomtiger
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
 * Revision 2.61  2003/11/17 06:52:51  bobboau
 * got assert to work again
 *
 * Revision 2.60  2003/11/16 04:09:25  Goober5000
 * language
 *
 * Revision 2.59  2003/11/11 03:56:10  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.58  2003/11/11 02:15:43  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.57  2003/11/10 02:44:36  matt
 * Fixed the projection matrix issues -Sticks
 *
 * Revision 2.56  2003/11/09 06:31:39  Kazan
 * a couple of htl functions being called in nonhtl (ie NULL functions) problems fixed
 * conflicts in cmdline and timerbar.h log entries
 * cvs stopped acting like it was on crack obviously
 *
 * Revision 2.55  2003/11/07 18:31:02  randomtiger
 * Fixed a nohtl call to htl funcs (crash with NULL pointer)
 * Fixed a bug with 32bit PCX code.
 * Fixed a bug in the d3d_string batch system that was messing up screen shaking.
 * Added a couple of checks to try and stop timerbar push and pop overloads, check returns missing pops if you use the system.
 * Put in a higher res icon until we get something better sorted out.
 *
 * Revision 2.54  2003/11/06 22:47:37  phreak
 * added gr_start_**_matrix() and gr_end_**_matrix() around where 3d stuff is rendered
 *
 * Revision 2.53  2003/10/29 02:09:17  randomtiger
 * Updated timerbar code to work properly, also added support for it in OGL.
 * In D3D red is general processing (and generic graphics), green is 2D rendering, dark blue is 3D unlit, light blue is HT&L renders and yellow is the presentation of the frame to D3D. OGL is all red for now. Use compile flag TIMERBAR_ON with code lib to activate it.
 * Also updated some more D3D device stuff that might get a bit more speed out of some cards.
 *
 * Revision 2.52  2003/10/27 23:04:21  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.51  2003/10/26 00:31:58  randomtiger
 * Fixed hulls not drawing (with Phreaks advise).
 * Put my 32bit PCX loading under PCX_32 compile flag until its working.
 * Fixed a bug with res 640x480 I introduced with my non standard mode code.
 * Changed JPG and TGA loading command line param to "-t32"
 *
 * Revision 2.50  2003/10/16 17:36:29  randomtiger
 * D3D now has its own gamma system (stored in GammaD3D reg entry) that effects everything.
 * Put in Bobs specular fog fix.
 *
 * Revision 2.49  2003/10/16 00:17:12  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
 * Revision 2.48  2003/10/15 22:03:24  Kazan
 * Da Species Update :D
 *
 * Revision 2.47  2003/10/14 17:39:12  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.46  2003/09/26 14:37:13  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.45  2003/09/25 21:12:23  Kazan
 * ##Kazan## FS2NetD Completed!  Just needs some thorough bug checking (i don't think there are any serious bugs)
 * Also D3D8 Screenshots work now.
 *
 * Revision 2.44  2003/09/24 19:35:57  Kazan
 * ##KAZAN## FS2 Open PXO --- W00t! Stats Storage, everything but table verification completed!
 *
 * Revision 2.43  2003/09/23 02:42:52  Kazan
 * ##KAZAN## - FS2NetD Support! (FS2 Open PXO) -- Game Server Listing, and mission validation completed - stats storing to come - needs fs2open_pxo.cfg file [VP-able]
 *
 * Revision 2.42  2003/09/14 19:01:16  wmcoolmon
 * Changed "cell" to "Cmdline_cell" -C
 *
 * Revision 2.41  2003/09/07 18:14:52  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.40  2003/09/05 04:25:29  Goober5000
 * well, let's see here...
 *
 * * persistent variables
 * * rotating gun barrels
 * * positive/negative numbers fixed
 * * sexps to trigger whether the player is controlled by AI
 * * sexp for force a subspace jump
 *
 * I think that's it :)
 * --Goober5000
 *
 * Revision 2.39  2003/08/22 07:35:08  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.38  2003/08/21 20:54:38  randomtiger
 * Fixed switching - RT
 *
 * Revision 2.37  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.36  2003/08/12 03:18:33  bobboau
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
 * Revision 2.35  2003/08/09 06:07:23  bobboau
 * slightly better implementation of the new zbuffer thing, it now checks only three diferent formats defalting to the 16 bit if neither the 24 or 32 bit versions are suported
 *
 * Revision 2.34  2003/08/06 17:42:47  phreak
 * since the launcher has a space between 1024 and 768 for the videocard string, i changed the detection to reflect this
 * also changed intro.mve to intro.avi since we can't use MVEs
 *
 * Revision 2.33  2003/07/06 00:19:24  randomtiger
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
 * Revision 2.32  2003/05/21 20:26:07  phreak
 * improved chase camera code.. you can now fight using this camera
 *
 * Revision 2.31  2003/05/09 23:54:59  phreak
 * game_loading_callback_init() now displays a user-defined loading screen
 *
 * Revision 2.30  2003/03/29 08:52:59  sesquipedalian
 * Added is-missile-locked sexp
 *
 * Revision 2.29  2003/03/22 06:11:51  Goober5000
 * added play-sound-from-table, play-sound-from-file, and close-sound-from-file
 * --Goober5000
 *
 * Revision 2.28  2003/03/18 10:07:01  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.27  2003/03/02 05:23:34  penguin
 * 1. Renamed or, og, ob vars to o_r, o_g, o_b  ("or" caused gcc to choke).
 * 2. Added #ifndef NO_SOFTWARE_RENDERING around software rendering code
 * 3. Added #ifdef _MSC_VER around MSVC-specific exception handing code
 *  - penguin
 *
 * Revision 2.26  2003/02/23 20:55:57  wmcoolmon
 * Refixed the splash screen loading code so that it still works with the demo VPs (Loading "2_PreLoad" and "PreLoad") and put logo support back in. Logo support does nothing unless "PreLoadLogo" and/or"2_PreLoadLogo" exist within the search path.
 *
 * Revision 2.25  2003/02/16 19:00:56  phreak
 * stupid typo -- my bad
 *
 * Revision 2.24  2003/02/16 18:53:59  phreak
 * fixed around splash screen
 * 2_splash.pcx is loaded in 1024x768
 * splash.pcx is loaded in 640x480 or 2_splash.pcx isn't found
 *
 * Revision 2.23  2003/01/30 23:18:21  phreak
 * cleaned up debug output
 *
 * Revision 2.22  2003/01/26 20:04:32  wmcoolmon
 * Minor mod to preload screen code, added separate 1024x768 logo
 * -C
 *
 * Revision 2.21  2003/01/26 02:58:33  wmcoolmon
 * Added title screen support to retail version. When FS2 loads, it will build a list of PCX files in the current directory, then attempt to display one. If no PCX files exist, it will try to display one of the title screens defined in the code. Freespace 2 will then attempt to superimpose a logo in the upper-left hand corner of the screen. If neither can be displayed, FS2 will function normally.
 *
 * Revision 2.20  2003/01/20 05:40:49  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.19  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.18  2003/01/15 21:26:35  anonymous
 * fixed the demo compilation. Define FS2_DEMO globally to compile as a demo. Make sure warp.pof is in your data/models directory.
 *
 * Revision 2.17  2003/01/14 04:00:16  Goober5000
 * allowed for up to 256 main halls
 * --Goober5000
 *
 * Revision 2.16  2002/12/21 13:39:03  DTP
 * did bit more house keeping. modfied Phreaks fps cmdline a bit, so that we dont have to specific build code.libs for fred, but can use the same code.lib for both fs2_open.exe and fred2_open.exe
 *
 * Revision 2.15  2002/12/18 22:21:23  phreak
 * tidied up game_init() a bit.. took out a bunch of unneeded ifdefs like E3_BUILD
 * Added option to access OpenGL from the registry keys.  searches for "OpenGL-"
 * whoever is in charge of the launcher might want to know this.
 *
 * Revision 2.14  2002/11/18 21:34:16  phreak
 * made ogl be used if "NDEBUG" is not defined and "USE_OPENGL" is defined  - phreak
 *
 * Revision 2.13  2002/10/22 17:42:09  randomtiger
 * Fixed lighting bug that caused special pause to crash on debug build.
 * Also added TAB functionality for special pause that toggles HUD. - RT
 *
 * Revision 2.12  2002/10/19 03:50:28  randomtiger
 * Added special pause mode for easier action screenshots.
 * Added new command line parameter for accessing all single missions in tech room. - RT
 *
 * Revision 2.11  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.10  2002/09/20 20:09:01  phreak
 * did glare stuff in game_sunspot_process()
 *
 *
 *
 * Revision 2.51 2002/08/04 2`:08:55 PhReAk
 * Draw mem usage - will be needed for increasing original limits
 *
 * Revision 2.51 2002/08/04 20:04:35 PhReAk
 * Draw FPS when running debug build
 *
 * Revision 2.9  2002/08/28 10:51:04  randomtiger
 * Woh! I sure didnt modify all these files it says I did.
 * I will put this down to the branch! Note: I did start from a fresh checkout!
 *
 * Revision 2.8  2002/08/27 13:38:57  penguin
 * Moved DirectX8 stuff to directx8 branch; reverted to previous
 *
 * Revision 2.6.2.6  2002/11/04 03:02:28  randomtiger
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
 * Revision 2.6.2.5  2002/10/26 09:55:41  unknownplayer
 *
 * Fixed the nebula flicker bug. Check the NO_DIRECT3D conditional compile
 * sections in the future for bits of code which may cause problems due to hacks
 * correcting for DirectX5 that no longer apply.
 *
 * Revision 2.6.2.4  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.6.2.2  2002/08/28 12:39:36  randomtiger
 * OK, this should be a commit to the DX branch or Im going to be in a lot of trouble.
 * The movie and dx8show files have been cleaned up big time.
 * My debug system is in but has NO EFFECT at all unless a compiler flag is turned on, check h file for details.
 * Aside from that a few changes to help the movie code work properly.
 * Works on most things including GF4 and Voodoo 3. However may not work properly on a voodoo 2.
 * Im going to leave this as a bug for now, serves you right for buying voodoo!
 *
 * Revision 2.6.2.1  2002/08/27 13:21:45  penguin
 * Moved to directx8 branch
 *
 * Revision 2.7  2002/08/18 19:48:29  randomtiger
 * Added new lib files: strmiids and ddraw to get dshow working
 * Added new command line parameter to active direct show movie play: -dshowvid
 * Uncommented movie_play calls and includes
 *
 * Revision 2.6  2002/08/13 03:34:00  penguin
 * 1. Disable CD checking
 * 2. Add CVS tag to version string
 *
 * Revision 2.5  2002/08/04 05:11:05  penguin
 * Don't write version to registry; change way version string is formatted
 *
 * Revision 2.4  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.3  2002/07/29 20:12:31  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.2  2002/07/22 01:39:24  penguin
 * Added ifndef NO_STANDALONE
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.16  2002/06/03 03:39:28  mharris
 * Added pause code back in (now that fAppActive is working, and we're
 * getting the correct keycode for the pause key)
 *
 * Revision 1.15  2002/05/26 14:09:03  mharris
 * Testing
 *
 * Revision 1.14  2002/05/21 15:38:49  mharris
 * Minor debug tweaks
 *
 * Revision 1.13  2002/05/17 23:39:07  mharris
 * temp workaround to test on linux
 *
 * Revision 1.12  2002/05/17 06:45:53  mharris
 * More porting tweaks.  It links!  but segfaults...
 *
 * Revision 1.11  2002/05/17 02:57:52  mharris
 * Rework that annoying try/catch block.  Commented out most of it except
 * when in WIN32.  No exceptions are thrown anywhere; why do we need it?
 *
 * Revision 1.10  2002/05/16 00:41:00  mharris
 * Added ifdef WIN32 around CD stuff.  More NO_SOUND and NO_NETWORK
 * tweaks.
 *
 * Revision 1.9  2002/05/15 14:59:29  mharris
 * Added another ifndef NO_SOUND
 * For non-Win32, use stat() rather than access() to check
 * for sparky_hi_fs2.vp
 *
 * Revision 1.8  2002/05/13 21:43:37  mharris
 * A little more network and sound cleanup
 *
 * Revision 1.7  2002/05/13 21:09:28  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.6  2002/05/10 20:42:43  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.5  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.4  2002/05/09 13:49:30  mharris
 * Added ifndef NO_DIRECT3D
 *
 * Revision 1.3  2002/05/08 17:29:17  mharris
 * more port tweaks
 *
 * Revision 1.2  2002/05/07 02:56:50  mharris
 * porting...
 *
 * Revision 1.1  2002/05/02 18:03:06  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 201   6/16/00 3:15p Jefff
 * sim of the year dvd version changes, a few german soty localization
 * fixes
 * 
 * 200   11/03/99 11:06a Jefff
 * 1.2 checksums
 * 
 * 199   10/26/99 5:07p Jamest
 * fixed jeffs dumb debug code
 * 
 * 198   10/25/99 5:53p Jefff
 * call control_config_common_init() on startup
 * 
 * 197   10/14/99 10:18a Daveb
 * Fixed incorrect CD checking problem on standalone server.
 * 
 * 196   10/13/99 9:22a Daveb
 * Fixed Fred jumpnode placing bug. Fixed 1024 glide tiled texture problem
 * related to movies. Fixed launcher spawning from PXO screen.
 * 
 * 195   10/06/99 11:05a Jefff
 * new oem upsell 3 hotspot coords
 * 
 * 194   10/06/99 10:31a Jefff
 * OEM updates
 * 
 * 193   10/01/99 9:10a Daveb
 * V 1.1 PATCH
 * 
 * 192   9/15/99 4:57a Dave
 * Updated ships.tbl checksum
 * 
 * 191   9/15/99 3:58a Dave
 * Removed framerate warning at all times.
 * 
 * 190   9/15/99 3:16a Dave
 * Remove mt-011.fs2 from the builtin mission list.
 * 
 * 189   9/15/99 1:45a Dave
 * Don't init joystick on standalone. Fixed campaign mode on standalone.
 * Fixed no-score-report problem in TvT
 * 
 * 188   9/14/99 6:08a Dave
 * Updated (final) single, multi, and campaign list.
 * 
 * 187   9/14/99 3:26a Dave
 * Fixed laser fogging problem in nebula (D3D)> Fixed multiplayer
 * respawn-too-early problem. Made a few crash points safe.
 * 
 * 186   9/13/99 4:52p Dave
 * RESPAWN FIX
 * 
 * 185   9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 184   9/10/99 11:53a Dave
 * Shutdown graphics before sound to eliminate apparent lockups when
 * Directsound decides to be lame. Fix TOPMOST problem with D3D windows.
 * 
 * 183   9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 182   9/08/99 10:29p Dave
 * Make beam sound pausing and unpausing much safer.
 * 
 * 181   9/08/99 10:01p Dave
 * Make sure game won't run in a drive's root directory. Make sure
 * standalone routes suqad war messages properly to the host.
 * 
 * 180   9/08/99 3:22p Dave
 * Updated builtin mission list.
 * 
 * 179   9/08/99 12:01p Jefff
 * fixed Game_builtin_mission_list typo on Training-2.fs2
 * 
 * 178   9/08/99 9:48a Andsager
 * Add force feedback for engine wash.
 * 
 * 177   9/07/99 4:01p Dave
 * Fixed up a string.tbl paroblem (self destruct message). Make sure IPX
 * does everything properly (setting up address when binding). Remove
 * black rectangle background from UI_INPUTBOX.
 * 
 * 176   9/13/99 2:40a Dave
 * Comment in full 80 minute CD check for RELEASE_REAL builds.
 * 
 * 175   9/06/99 6:38p Dave
 * Improved CD detection code.
 * 
 * 174   9/06/99 1:30a Dave
 * Intermediate checkin. Started on enforcing CD-in-drive to play the
 * game.
 * 
 * 173   9/06/99 1:16a Dave
 * Make sure the user sees the intro movie.
 * 
 * 172   9/04/99 8:00p Dave
 * Fixed up 1024 and 32 bit movie support.
 * 
 * 171   9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 170   9/01/99 10:49p Dave
 * Added nice SquadWar checkbox to the client join wait screen.
 * 
 * 169   9/01/99 10:14a Dave
 * Pirate bob.
 * 
 * 168   8/29/99 4:51p Dave
 * Fixed damaged checkin.
 * 
 * 167   8/29/99 4:18p Andsager
 * New "burst" limit for friendly damage.  Also credit more damage done
 * against large friendly ships.
 * 
 * 166   8/27/99 6:38p Alanl
 * crush the blasted repeating messages bug
 * 
 * 164   8/26/99 9:09p Dave
 * Force framerate check in everything but a RELEASE_REAL build.
 * 
 * 163   8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 162   8/24/99 8:55p Dave
 * Make sure nondimming pixels work properly in tech menu.
 * 
 * 161   8/24/99 1:49a Dave
 * Fixed client-side afterburner stuttering. Added checkbox for no version
 * checking on PXO join. Made button info passing more friendly between
 * client and server.
 * 
 * 160   8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 159   8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 158   8/20/99 2:09p Dave
 * PXO banner cycling.
 * 
 * 157   8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 156   8/19/99 10:12a Alanl
 * preload mission-specific messages on machines greater than 48MB
 * 
 * 155   8/16/99 4:04p Dave
 * Big honking checkin.
 * 
 * 154   8/11/99 5:54p Dave
 * Fixed collision problem. Fixed standalone ghost problem.
 * 
 * 153   8/10/99 7:59p Jefff
 * XSTR'ed some stuff
 * 
 * 152   8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 151   8/10/99 3:44p Jefff
 * loads Intelligence information on startup
 * 
 * 150   8/09/99 3:47p Dave
 * Fixed incorrect nebula regeneration. Default HUD to low-contrast in
 * non-nebula missions.
 * 
 * 149   8/09/99 2:21p Andsager
 * Fix patching from multiplayer direct to launcher update tab.
 * 
 * 148   8/09/99 10:36a Dave
 * Version info for game.
 * 
 * 147   8/06/99 9:46p Dave
 * Hopefully final changes for the demo.
 * 
 * 146   8/06/99 3:34p Andsager
 * Make title version info "(D)" -> "D"  show up nicely
 * 
 * 145   8/06/99 2:59p Adamp
 * Fixed NT launcher/update problem.
 * 
 * 144   8/06/99 1:52p Dave
 * Bumped up MAX_BITMAPS for the demo.
 * 
 * 143   8/06/99 12:17p Andsager
 * Demo: down to just 1 demo dog
 * 
 * 142   8/05/99 9:39p Dave
 * Yet another new checksum.
 * 
 * 141   8/05/99 6:19p Dave
 * New demo checksums.
 * 
 * 140   8/05/99 5:31p Andsager
 * Up demo version 1.01
 * 
 * 139   8/05/99 4:22p Andsager
 * No time limit on upsell screens.  Reverse order of display of upsell
 * bitmaps.
 * 
 * 138   8/05/99 4:17p Dave
 * Tweaks to client interpolation.
 * 
 * 137   8/05/99 3:52p Danw
 * 
 * 136   8/05/99 3:01p Danw
 * 
 * 135   8/05/99 2:43a Anoop
 * removed duplicate definition.
 * 
 * 134   8/05/99 2:13a Dave
 * Fixed build error.
 * 
 * 133   8/05/99 2:05a Dave
 * Whee.
 * 
 * 132   8/05/99 1:22a Andsager
 * fix upsell bug.
 * 
 * 131   8/04/99 9:51p Andsager
 * Add title screen to demo
 * 
 * 130   8/04/99 6:47p Jefff
 * fixed link error resulting from #ifdefs
 * 
 * 129   8/04/99 6:26p Dave
 * Updated ship tbl checksum.
 * 
 * 128   8/04/99 5:40p Andsager
 * Add multiple demo dogs
 * 
 * 127   8/04/99 5:36p Andsager
 * Show upsell screens at end of demo campaign before returning to main
 * hall.
 * 
 * 126   8/04/99 11:42a Danw
 * tone down EAX reverb
 * 
 * 125   8/04/99 11:23a Dave
 * Updated demo checksums.
 * 
 * 124   8/03/99 11:02p Dave
 * Maybe fixed sync problems in multiplayer.
 * 
 * 123   8/03/99 6:21p Jefff
 * minor text change
 * 
 * 122   8/03/99 3:44p Andsager
 * Launch laucher if trying to run FS without first having configured
 * system.
 * 
 * 121   8/03/99 12:45p Dave
 * Update checksums.
 * 
 * 120   8/02/99 9:13p Dave
 * Added popup tips.
 * 
 * 119   7/30/99 10:31p Dave
 * Added comm menu to the configurable hud files.
 * 
 * 118   7/30/99 5:17p Andsager
 * first fs2demo checksums
 * 
 * 117   7/29/99 3:09p Anoop
 * 
 * 116   7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 115   7/27/99 8:59a Andsager
 * Make major, minor version consistent for all builds.  Only show major
 * and minor for launcher update window.
 * 
 * 114   7/26/99 5:50p Dave
 * Revised ingame join. Better? We'll see....
 * 
 * 113   7/26/99 5:27p Andsager
 * Add training mission as builtin to demo build
 * 
 * 112   7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 111   7/22/99 4:00p Dave
 * Fixed beam weapon muzzle glow rendering. Externalized hud shield info.
 * 
 * 110   7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 109   7/20/99 1:49p Dave
 * Peter Drake build. Fixed some release build warnings.
 * 
 * 108   7/19/99 2:26p Andsager
 * set demo multiplayer missions
 * 
 * 107   7/18/99 5:19p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 106   7/16/99 1:50p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 105   7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 104   7/15/99 2:13p Dave
 * Added 32 bit detection.
 * 
 * 103   7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 102   7/14/99 11:02a Dave
 * Skill level default back to easy. Blech.
 * 
 * 101   7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 100   7/08/99 4:43p Andsager
 * New check for sparky_hi and print if not found.
 * 
 * 99    7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 98    7/06/99 4:24p Dave
 * Mid-level checkin. Starting on some potentially cool multiplayer
 * smoothness crap.
 * 
 * 97    7/06/99 3:35p Andsager
 * Allow movie to play before red alert mission.
 * 
 * 96    7/03/99 5:50p Dave
 * Make rotated bitmaps draw properly in padlock views.
 * 
 * 95    7/02/99 9:55p Dave
 * Player engine wash sound.
 * 
 * 94    7/02/99 4:30p Dave
 * Much more sophisticated lightning support.
 * 
 * 93    6/29/99 7:52p Dave
 * Put in exception handling in FS2.
 * 
 * 92    6/22/99 9:37p Dave
 * Put in pof spewing.
 * 
 * 91    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 90    6/15/99 1:56p Andsager
 * For release builds, allow start up in high res only with
 * sparky_hi._fs2.vp
 * 
 * 89    6/15/99 9:34a Dave
 * Fixed key checking in single threaded version of the stamp notification
 * screen. 
 * 
 * 88    6/09/99 2:55p Andsager
 * Allow multiple asteroid subtypes (of large, medium, small) and follow
 * family.
 * 
 * 87    6/08/99 1:14a Dave
 * Multi colored hud test.
 * 
 * 86    6/04/99 9:52a Dave
 * Fixed some rendering problems.
 * 
 * 85    6/03/99 10:15p Dave
 * Put in temporary main hall screen.
 * 
 * 84    6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 83    6/01/99 3:52p Dave
 * View footage screen. Fixed xstrings to not display the & symbol. Popup,
 * dead popup, pxo find player popup, pxo private room popup.
 * 
 * 82    5/26/99 1:28p Jasenw
 * changed coords for loading ani
 * 
 * 81    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 80    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 
 */

static const char RCS_Name[] = "$Name: not supported by cvs2svn $";

#ifdef _WIN32
 #include <direct.h>
 #include <io.h>
#ifndef _MINGW
 #include <crtdbg.h>
#endif // !_MINGW
#else
 #include <unistd.h>
 #include <sys/stat.h>
#endif


#include "anim/animplay.h"
#include "asteroid/asteroid.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "cmeasure/cmeasure.h"
#include "cutscene/cutscenes.h"
#include "cutscene/movie.h"
#include "debris/debris.h"
#include "debugconsole/dbugfile.h"
#include "exceptionhandler/exceptionhandler.h"
#include "fireball/fireballs.h"
#include "freespace2/freespace.h"
#include "freespace2/levelpaging.h"
#include "gamehelp/contexthelp.h"
#include "gamehelp/gameplayhelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "globalincs/version.h"
#include "graphics/font.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudshield.h"
#include "hud/hudtargetbox.h"
#include "hud/hudparse.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/key.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "localization/localize.h"
#include "math/staticrand.h"
#include "menuui/barracks.h"
#include "menuui/credits.h"
#include "menuui/mainhallmenu.h"
#include "menuui/optionsmenu.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "menuui/snazzyui.h"
#include "menuui/techmenu.h"
#include "menuui/trainingmenu.h"
#include "menuui/storybook.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionhotkey.h"
#include "mission/missionload.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missiontraining.h"
#include "missionui/missionbrief.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/missiondebrief.h"
#include "missionui/missionloopbrief.h"
#include "missionui/missionpause.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "missionui/redalert.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "network/stand_gui.h"
#include "object/objcollide.h"
#include "object/objectsnd.h"
#include "observer/observer.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "parse/encrypt.h"
#include "parse/sexp.h"
#include "particle/particle.h"
#include "playerman/managepilot.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "radar/radar.h"
#include "render/3d.h"
#include "ship/afterburner.h"
#include "ship/awacs.h"
#include "ship/ship.h"
#include "ship/shipcontrails.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "sound/audiostr.h"
#include "sound/fsspeech.h"
#include "sound/sound.h"
#include "starfield/starfield.h"
#include "starfield/supernova.h"
#include "stats/medals.h"
#include "stats/stats.h"
#include "weapon/beam.h"
#include "weapon/emp.h"
#include "weapon/flak.h"
#include "weapon/muzzleflash.h"
#include "weapon/shockwave.h"
#include "weapon/weapon.h"
#include "radar/radarsetup.h"
#include "camera/camera.h"
#include "lab/lab.h"
#include "lab/wmcgui.h"	//So that GUI_System can be initialized
#include "jumpnode/jumpnode.h"
#include "autopilot/autopilot.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multi_dogfight.h"
#include "network/multi_endgame.h"
#include "network/multi_ingame.h"
#include "network/multi_log.h"
#include "network/multi_pause.h"
#include "network/multi_rate.h"
#include "network/multi_respawn.h"
#include "network/multi_voice.h"
#include "network/multimsgs.h"
#include "network/multiteamselect.h"
#include "network/multiui.h"
#include "network/multiutil.h"
extern int Om_tracker_flag; // needed for FS2OpenPXO config
#include "network/fs2ox.h"
#endif




#ifdef NDEBUG
#ifdef FRED
#error macro FRED is defined when trying to build release Fred.  Please undefine FRED macro in build settings
#endif
#endif

//	Revision history.
//	Full version:
//    1.00.04	5/26/98	MWA -- going final (12 pm)
//    1.00.03	5/26/98	MWA -- going final (3 am)
//    1.00.02	5/25/98	MWA -- going final
//    1.00.01	5/25/98	MWA -- going final
//		0.90		5/21/98	MWA -- getting ready for final.
//		0.10		4/9/98.  Set by MK.
//
//	Demo version: (obsolete since DEMO codebase split from tree)
//		0.03		4/10/98	AL.	Interplay rev
//		0.02		4/8/98	MK.	Increased when this system was modified.
//		0.01		4/7/98?	AL.	First release to Interplay QA.
//
//	OEM version:
//		1.00		5/28/98	AL.	First release to Interplay QA.


#ifdef NO_SOUND
// defined here to avoid link errors
game_snd Snds[MIN_GAME_SOUNDS];
game_snd Snds_iface[MIN_INTERFACE_SOUNDS];
int Snds_iface_handle[MIN_INTERFACE_SOUNDS];

// dummy callback -- real one is in gamesnd.cpp
void common_play_highlight_sound()
{
}
#endif

//  This function is defined in code\network\multiutil.cpp so will be linked from multiutil.obj
//  it's required fro the -missioncrcs command line option - Kazan
void multi_spew_pxo_checksums(int max_files, char *outfile);
void multi_spew_table_checksums(int max_files, char *outfile);

extern bool frame_rate_display;

bool cube_map_drawen = false;

void game_level_init(int seed = -1);
void game_reset_view_clip();
void game_reset_shade_frame();
void game_post_level_init();
void game_do_frame();
void game_update_missiontime();	// called from game_do_frame() and navmap_do_frame()
void game_reset_time();
void game_show_framerate();			// draws framerate in lower right corner

int Game_no_clear = 0;

int Pofview_running = 0;
int Nebedit_running = 0;

typedef struct big_expl_flash {
	float max_flash_intensity;	// max intensity
	float cur_flash_intensity;	// cur intensity
	int	flash_start;		// start time
} big_expl_flash;

#define FRAME_FILTER 16

#define DEFAULT_SKILL_LEVEL	1
int	Game_skill_level = DEFAULT_SKILL_LEVEL;

float VIEWER_ZOOM_DEFAULT = 0.75f;			//	Default viewer zoom, 0.625 as per multi-lateral agreement on 3/24/97
float Viewer_zoom = VIEWER_ZOOM_DEFAULT;

#define EXE_FNAME			("fs2.exe")

#define LAUNCHER_FNAME	("Launcher.exe")

// JAS: Code for warphole camera.
// Needs to be cleaned up.
vec3d Camera_pos = { 0.0f, 0.0f, 0.0f };
vec3d Camera_velocity = { 0.0f, 0.0f, 0.0f };
vec3d Camera_desired_velocity = { 0.0f, 0.0f, 0.0f };
matrix Camera_orient = IDENTITY_MATRIX;
float Camera_damping = 1.0f;
float Camera_time = 0.0f;
float Warpout_time = 0.0f;
int Warpout_forced = 0;		// Set if this is a forced warpout that cannot be cancelled.
int Warpout_sound = -1;
void camera_move();
int Use_joy_mouse = 0;
int Use_palette_flash = 1;
#ifndef NDEBUG
int Use_fullscreen_at_startup = 0;
#endif
int Show_area_effect = 0;
object	*Last_view_target = NULL;

int dogfight_blown = 0;

int	frame_int = -1;
float frametimes[FRAME_FILTER];
float frametotal = 0.0f;
float flRealframetime;
float flFrametime;

#ifndef NDEBUG
	int	Show_framerate = 1;
	int	Show_mem = 1;
#else 
	int	Show_framerate = 0;
	int	Show_mem = 0;
#endif

int	Framerate_cap = 120;

// for the model page in system
extern void model_page_in_start();

int	Show_cpu = 0;
int	Show_target_debug_info = 0;
int	Show_target_weapons = 0;
int	Game_font = -1;
#ifndef NDEBUG
static int Show_player_pos = 0;		// debug console command to show player world pos on HUD
#endif

int Debug_octant = -1;

fix Game_time_compression = F1_0;
fix Desired_time_compression = Game_time_compression;
fix Time_compression_change_rate = 0;
bool Time_compression_locked = false; //Can the user change time with shift- controls?

// auto-lang stuff
int detect_lang();

// table checksums that will be used for pilot files
uint Weapon_tbl_checksum = 0;
uint Ships_tbl_checksum = 0;

// if the ships.tbl the player has is valid
int Game_ships_tbl_valid = 0;

// if the weapons.tbl the player has is valid
int Game_weapons_tbl_valid = 0;

//#ifndef NDEBUG
int Test_begin = 0;
extern int	Player_attacking_enabled;
int Show_net_stats;
//#endif

int Pre_player_entry;

int	Fred_running = 0;
char Game_current_mission_filename[MAX_FILENAME_LEN];
int game_single_step = 0;
int last_single_step=0;

extern int MSG_WINDOW_X_START;	// used to position mission_time and shields output
extern int MSG_WINDOW_Y_START;
extern int MSG_WINDOW_HEIGHT;

int game_zbuffer = 1;
//static int Game_music_paused;
static int Game_paused;

int Game_level_seed;

#define EXPIRE_BAD_CHECKSUM			1
#define EXPIRE_BAD_TIME					2

extern void ssm_init();
extern void ssm_level_init();
extern void ssm_process();

// static variable to contain the time this version was built
// commented out for now until
// I figure out how to get the username into the file
//LOCAL char freespace_build_time[] = "Compiled on:"__DATE__" "__TIME__" by "__USER__;

// defines and variables used for dumping frame for making trailers.
#ifndef NDEBUG
int Debug_dump_frames = 0;			// Set to 0 to not dump frames, else equal hz to dump. (15 or 30 probably)
int Debug_dump_trigger = 0;
int Debug_dump_frame_count;
int Debug_dump_frame_num = 0;
#define DUMP_BUFFER_NUM_FRAMES	1			// store every 15 frames
#endif

// amount of time to wait after the player has died before we display the death died popup
#define PLAYER_DIED_POPUP_WAIT		2500
int Player_died_popup_wait = -1;
int Player_multi_died_check = -1;

int Multi_ping_timestamp = -1;

// builtin mission list stuff
#ifdef FS2_DEMO
	int Game_builtin_mission_count = 6;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		{ "SPDemo-01.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "SPDemo-02.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "DemoTrain.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "Demo.fc2",						(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					""		},
		{ "MPDemo-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "Demo-DOG-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI),								""		},
	};
#elif defined(PD_BUILD)
	int Game_builtin_mission_count = 4;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION),												""		},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION),												""		},		
		{ "sm1-01",							(FSB_FROM_VOLITION),												""		},
		{ "sm1-05",							(FSB_FROM_VOLITION),												""		},		
	};
#elif defined(MULTIPLAYER_BETA)
	int Game_builtin_mission_count = 17;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// multiplayer beta
		{ "md-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-06.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-07.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "mt-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "mt-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "templar-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-02.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-03a.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-04a.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar.fc2",					(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN_FILE),	""		},	
	};
#elif defined(OEM_BUILD)
	int Game_builtin_mission_count = 17;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// oem version - act 1 only
		{ "freespace2oem.fc2",			(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					"" },
			
		// act 1
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-1.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-2.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-3.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-104.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-105.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-106.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	}
	};
#else 
	int Game_builtin_mission_count = 92;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// single player campaign
		{ "freespace2.fc2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					"" },
			
		// act 1
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-3.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-1.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-2.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-3.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-104.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-105.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-106.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},

		// act 2
		{ "sm2-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

		// act 3
		{ "sm3-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "loop2-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},		
		{ "loop2-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

		// multiplayer missions

		// gauntlet
		{ "g-shi.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "g-ter.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "g-vas.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

		// coop
		{ "m-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},		

		// dogfight
		{ "mdh-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "osdog.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

		// TvT		
		{ "mt-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-06.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-07.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-08.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-09.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-10.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},				

		// campaign
		{ "templar.fc2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN_FILE),					"" },
		{ "templar-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-02.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-03.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-04.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
	};
#endif


// Internal function prototypes
void game_maybe_draw_mouse(float frametime);
void init_animating_pointer();
void load_animating_pointer(char *filename, int dx, int dy);
void unload_animating_pointer();
void game_do_training_checks();
void game_shutdown(void);
void game_show_event_debug(float frametime);
void game_event_debug_init();
void game_frame(int paused = false);
void demo_upsell_show_screens();
void game_start_subspace_ambient_sound();
void game_stop_subspace_ambient_sound();
void verify_ships_tbl();
void verify_weapons_tbl();
void display_title_screen();

// loading background filenames
static char *Game_loading_bground_fname[GR_NUM_RESOLUTIONS] = {
	"LoadingBG",		// GR_640
	"2_LoadingBG"		// GR_1024
};


static char *Game_loading_ani_fname[GR_NUM_RESOLUTIONS] = {
	"Loading.ani",		// GR_640
	"2_Loading.ani"		// GR_1024
};

#if defined(OEM_BUILD)
static char *Game_title_screen_fname[GR_NUM_RESOLUTIONS] = {
	"OEMPreLoad",
	"2_OEMPreLoad"
};
#else
static char *Game_title_screen_fname[GR_NUM_RESOLUTIONS] = {
	"PreLoad",
	"2_PreLoad"
};
#endif
static char *Game_logo_screen_fname[GR_NUM_RESOLUTIONS] = {
	"PreLoadLogo",
	"2_PreLoadLogo"
};

// cdrom stuff
char Game_CDROM_dir[MAX_PATH_LEN];
int init_cdrom();

// How much RAM is on this machine. Set in WinMain
uint Freespace_total_ram = 0;

// game flash stuff
float Game_flash_red = 0.0f;
float Game_flash_green = 0.0f;
float Game_flash_blue = 0.0f;
float Sun_spot = 0.0f;
big_expl_flash Big_expl_flash = {0.0f, 0.0f, 0};

// game shudder stuff (in ms)
int Game_shudder_time = -1;
int Game_shudder_total = 0;
float Game_shudder_intensity = 0.0f;			// should be between 0.0 and 100.0

#ifndef NO_SOUND
// EAX stuff
sound_env Game_sound_env;
sound_env Game_default_sound_env = {SND_ENV_BATHROOM, 0.2F,0.2F,1.0F};
int Game_sound_env_update_timestamp;
#endif


// WARPIN CRAP BEGIN --------------------------------------------------------------------------------------------


// WARPIN CRAP END --------------------------------------------------------------------------------------------

fs_builtin_mission *game_find_builtin_mission(char *filename)
{
	int idx;

	// look through all existing builtin missions
	for(idx=0; idx<Game_builtin_mission_count; idx++){
		if(!stricmp(Game_builtin_mission_list[idx].filename, filename)){
			return &Game_builtin_mission_list[idx];
		}
	}

	// didn't find it
	return NULL;
}

int game_get_default_skill_level()
{
	return DEFAULT_SKILL_LEVEL;
}

// Resets the flash
void game_flash_reset()
{
	Game_flash_red = 0.0f;
	Game_flash_green = 0.0f;
	Game_flash_blue = 0.0f;
	Sun_spot = 0.0f;
	Big_expl_flash.max_flash_intensity = 0.0f;
	Big_expl_flash.cur_flash_intensity = 0.0f;
	Big_expl_flash.flash_start = 0;
}

float Gf_critical = -1.0f;					// framerate we should be above on the average for this mission
float Gf_critical_time = 0.0f;			// how much time we've been at the critical framerate

void game_framerate_check_init()
{
	// zero critical time
	Gf_critical_time = 0.0f;
		
	// nebula missions
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		Gf_critical = 15.0f;			
	} else {
		Gf_critical = 25.0f;
	}
}

extern float Framerate;
void game_framerate_check()
{
	int y_start = 100;
	
	// if the current framerate is above the critical level, add frametime
	if(Framerate >= Gf_critical){
		Gf_critical_time += flFrametime;
	}	

	if((!Show_framerate) || (!Cmdline_show_fps)) {
		return;
	}

	// display if we're above the critical framerate
	if(Framerate < Gf_critical){
		gr_set_color_fast(&Color_bright_red);
		gr_string(200, y_start, "Framerate warning");

		y_start += 10;
	}

	// display our current pct of good frametime
	if(f2fl(Missiontime) >= 0.0f){
		float pct = (Gf_critical_time / f2fl(Missiontime)) * 100.0f;

		if(pct >= 85.0f){
			gr_set_color_fast(&Color_bright_green);
		} else {
			gr_set_color_fast(&Color_bright_red);
		}

		gr_printf(200, y_start, "%d%%", (int)pct);

		y_start += 10;
	}
}


// Adds a flash effect.  These can be positive or negative.
// The range will get capped at around -1 to 1, so stick 
// with a range like that.
void game_flash( float r, float g, float b )
{
	Game_flash_red += r;
	Game_flash_green += g;
	Game_flash_blue += b;

	if ( Game_flash_red < -1.0f )	{
		Game_flash_red = -1.0f;
	} else if ( Game_flash_red > 1.0f )	{
		Game_flash_red = 1.0f;
	}

	if ( Game_flash_green < -1.0f )	{
		Game_flash_green = -1.0f;
	} else if ( Game_flash_green > 1.0f )	{
		Game_flash_green = 1.0f;
	}

	if ( Game_flash_blue < -1.0f )	{
		Game_flash_blue = -1.0f;
	} else if ( Game_flash_blue > 1.0f )	{
		Game_flash_blue = 1.0f;
	}

}

// Adds a flash for Big Ship explosions
// cap range from 0 to 1
void big_explosion_flash(float flash)
{
	Big_expl_flash.flash_start = timestamp(1);

	if (flash > 1.0f) {
		flash = 1.0f;
	} else if (flash < 0.0f) {
		flash = 0.0f;
	}

	Big_expl_flash.max_flash_intensity = flash;
	Big_expl_flash.cur_flash_intensity = 0.0f;
}

//	Amount to diminish palette towards normal, per second.
#define	DIMINISH_RATE	0.75f
#define	SUN_DIMINISH_RATE	6.00f

int Sun_drew = 0;

float sn_glare_scale = 1.7f;
DCF(sn_glare, "")
{
	dc_get_arg(ARG_FLOAT);
	sn_glare_scale = Dc_arg_float;
}

float Supernova_last_glare = 0.0f;
extern starfield_bitmap *stars_lookup_sun(starfield_bitmap_instance *s);
void game_sunspot_process(float frametime)
{
	int n_lights, idx;
	int sn_stage;
	float Sun_spot_goal = 0.0f;

	// supernova
	sn_stage = supernova_active();
	if(sn_stage){		
		// sunspot differently based on supernova stage
		switch(sn_stage){
		// approaching. player still in control
		case 1:			
			float pct;
			pct = (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME));

			vec3d light_dir;				
			light_get_global_dir(&light_dir, 0);
			float dot;
			dot = vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec );
			
			if(dot >= 0.0f){
				// scale it some more
				dot = dot * (0.5f + (pct * 0.5f));
				dot += 0.05f;					

				Sun_spot_goal += (dot * sn_glare_scale);
			}

			// draw the sun glow
			Assert(Viewer_obj);
			if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, 0 ) )	{
				// draw the glow for this sun
				stars_draw_sun_glow(0);	
			}

			Supernova_last_glare = Sun_spot_goal;
			break;

		// camera cut. player not in control. note : at this point camera starts out facing the sun. so we can go nice and bright
		case 2: 					
		case 3:
			Sun_spot_goal = 0.9f;
			Sun_spot_goal += (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME)) * 0.1f;

			if(Sun_spot_goal > 1.0f){
				Sun_spot_goal = 1.0f;
			}

			Sun_spot_goal *= sn_glare_scale;
			Supernova_last_glare = Sun_spot_goal;
			break;		

		// fade to white. display dead popup
		case 4:
		case 5:
			Supernova_last_glare += (2.0f * flFrametime);
			if(Supernova_last_glare > 2.0f){
				Supernova_last_glare = 2.0f;
			}

			Sun_spot_goal = Supernova_last_glare;
			break;
		}
	
		Sun_drew = 0;				
	} else {
		if ( Sun_drew )	{
			starfield_bitmap* sbm;
			// check sunspots for all suns
			n_lights = light_get_global_count();

			// check
			for(idx=0; idx<n_lights; idx++){
				//(vec3d *eye_pos, matrix *eye_orient)
				if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, idx ) )	{

					vec3d light_dir;				
					light_get_global_dir(&light_dir, idx);

					sbm=stars_lookup_sun(&Suns[idx]);
					if (!sbm)
						continue;
					
					//only do sunglare stuff if this sun has one
					if (sbm->glare)
					{
						float dot = vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec )*0.5f+0.5f;

						Sun_spot_goal += (float)pow(dot,85.0f);
					}
					// draw the glow for this sun
					stars_draw_sun_glow(idx);				
				} else {
					Sun_spot_goal = 0.0f;
				}
			}

			Sun_drew = 0;
		} else {
			Sun_spot_goal = 0.0f;
		}
	}

	float dec_amount = frametime*SUN_DIMINISH_RATE;

	if ( Sun_spot < Sun_spot_goal )	{
		Sun_spot += dec_amount;
		if ( Sun_spot > Sun_spot_goal )	{
			Sun_spot = Sun_spot_goal;
		}
	} else if ( Sun_spot > Sun_spot_goal )	{
		Sun_spot -= dec_amount;
		if ( Sun_spot < Sun_spot_goal )	{
			Sun_spot = Sun_spot_goal;
		}
	}
}


// Call once a frame to diminish the
// flash effect to 0.
void game_flash_diminish(float frametime)
{
	float dec_amount = frametime*DIMINISH_RATE;

	if ( Game_flash_red > 0.0f ) {
		Game_flash_red -= dec_amount;		
		if ( Game_flash_red < 0.0f )
			Game_flash_red = 0.0f;
	} else {
		Game_flash_red += dec_amount;		
		if ( Game_flash_red > 0.0f )
			Game_flash_red = 0.0f;
	} 

	if ( Game_flash_green > 0.0f ) {
		Game_flash_green -= dec_amount;		
		if ( Game_flash_green < 0.0f )
			Game_flash_green = 0.0f;
	} else {
		Game_flash_green += dec_amount;		
		if ( Game_flash_green > 0.0f )
			Game_flash_green = 0.0f;
	} 

	if ( Game_flash_blue > 0.0f ) {
		Game_flash_blue -= dec_amount;		
		if ( Game_flash_blue < 0.0f )
			Game_flash_blue = 0.0f;
	} else {
		Game_flash_blue += dec_amount;		
		if ( Game_flash_blue > 0.0f )
			Game_flash_blue = 0.0f;
	} 

	// update big_explosion_cur_flash
#define	TIME_UP		1500
#define	TIME_DOWN	2500
	int duration = TIME_UP + TIME_DOWN;
	int time = timestamp_until(Big_expl_flash.flash_start);
	if (time > -duration) {
		time = -time;
		if (time < TIME_UP) {
			Big_expl_flash.cur_flash_intensity = Big_expl_flash.max_flash_intensity * time / (float) TIME_UP;
		} else {
			time -= TIME_UP;
			Big_expl_flash.cur_flash_intensity = Big_expl_flash.max_flash_intensity * ((float) TIME_DOWN - time) / (float) TIME_DOWN;
		}
	}
	
	if ( Use_palette_flash )	{
		int r,g,b;
		static int o_r=0, o_g=0, o_b=0;

		// Change the 200 to change the color range of colors.
		r = fl2i( Game_flash_red*128.0f );  
		g = fl2i( Game_flash_green*128.0f );   
		b = fl2i( Game_flash_blue*128.0f );  

		if ( Sun_spot > 0.0f )	{
			r += fl2i(Sun_spot*128.0f);
			g += fl2i(Sun_spot*128.0f);
			b += fl2i(Sun_spot*128.0f);
		}

		if ( Big_expl_flash.cur_flash_intensity  > 0.0f ) {
			r += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
			g += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
			b += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
		}

		if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
		if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
		if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;

		if ( (r!=0) || (g!=0) || (b!=0) ) {
			gr_flash( r, g, b );

			//mprintf(( "Flash! %d,%d,%d\n", r, g, b ));

			o_r = r;
			o_g = g;
			o_b = b;
		}
	}
	
}


void game_level_close()
{
	// save player-persistent variables
	mission_campaign_save_player_persistent_variables();	// Goober5000

	// De-Initialize the game subsystems
	sexp_music_close();	// Goober5000
	event_music_level_close();
	game_stop_looped_sounds();
	snd_stop_all();
#ifndef NO_SOUND
	obj_snd_level_close();					// uninit object-linked persistant sounds
#endif
	gamesnd_unload_gameplay_sounds();	// unload gameplay sounds from memory
	anim_level_close();						// stop and clean up any anim instances
	message_mission_shutdown();			// called after anim_level_close() to make sure instances are clear
	shockwave_level_close();
	fireball_level_close();	
	shield_hit_close();
	mission_event_shutdown();
	asteroid_level_close();
	jumpnode_level_close();
	model_cache_reset();						// Reset/free all the model caching stuff
	flak_level_close();						// unload flak stuff
	neb2_level_close();						// shutdown gaseous nebula stuff
	ct_level_close();
	beam_level_close();
	mflash_level_close();
	mission_brief_common_reset();		// close out parsed briefing/mission stuff
	cameras_close();
	subtitles_close();
	trail_level_close();

	audiostream_unpause_all();
	Game_paused = 0;
}

void init_decals();
// intializes game stuff and loads the mission.  Returns 0 on failure, 1 on success
// input: seed =>	DEFAULT PARAMETER (value -1).  Only set by demo playback code.
void game_level_init(int seed)
{
	// seed the random number generator
	if ( seed == -1 ) {
		// if no seed was passed, seed the generator either from the time value, or from the
		// netgame security flags -- ensures that all players in multiplayer game will have the
		// same randon number sequence (with static rand functions)
#ifndef NO_NETWORK
		if ( Game_mode & GM_NORMAL ) {
			Game_level_seed = time(NULL);
		} else {
			Game_level_seed = Netgame.security;
		}
#else
		Game_level_seed = time(NULL);
#endif  // ifndef NO_NETWORK
	} else {
		// mwa 9/17/98 -- maybe this assert isn't needed????
		Assert( !(Game_mode & GM_MULTIPLAYER) );
		Game_level_seed = seed;
	}
	srand( Game_level_seed );

	// semirand function needs to get re-initted every time in multiplayer
	if ( Game_mode & GM_MULTIPLAYER ){
		init_semirand();
	}

	Framecount = 0;
	game_reset_view_clip();
	game_reset_shade_frame();

	Key_normal_game = (Game_mode & GM_NORMAL);
	Cheats_enabled = 0;

	Game_shudder_time = -1;

	// Initialize the game subsystems
//	timestamp_reset();			// Must be inited before everything else
	if(!Is_standalone){
		game_reset_time();			// resets time, and resets saved time too
	}

	Multi_ping_timestamp = -1;

	obj_init();						// Must be inited before the other systems

	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		model_page_in_start();		// mark any existing models as unused but don't unload them yet
		mprintf(( "Beginning level bitmap paging...\n" ));
		bm_page_in_start();
	} else {
		model_free_all();			// Free all existing models if standalone server
	}

	mission_brief_common_init();		// Free all existing briefing/debriefing text
	weapon_level_init();
	init_decals();

	NavSystem_Init();				// zero out the nav system
	
	ai_level_init();				//	Call this before ship_init() because it reads ai.tbl.
	ship_level_init();
	player_level_init();	
	shipfx_flash_init();			// Init the ship gun flash system.
	game_flash_reset();			// Reset the flash effect
	particle_init();				// Reset the particle system
	fireball_init();
	debris_init();
	cmeasure_init();
	shield_hit_init();				//	Initialize system for showing shield hits
	radar_mission_init();
	mission_init_goals();
	mission_log_init();
	messages_init();
#ifndef NO_SOUND
	obj_snd_level_init();					// init object-linked persistant sounds
#endif
	anim_level_init();
	shockwave_level_init();
	afterburner_level_init();
	scoring_level_init( &Player->stats );
	key_level_init();
	asteroid_level_init();
	control_config_clear_used_status();
	collide_ship_ship_sounds_init();
	Missiontime = 0;
	Pre_player_entry = 1;			//	Means the player has not yet entered.
	Entry_delay_time = 0;			//	Could get overwritten in mission read.
	fireball_preload();				//	page in warphole bitmaps
	observer_init();
	flak_level_init();				// initialize flak - bitmaps, etc
	ct_level_init();					// initialize ships contrails, etc
	awacs_level_init();				// initialize AWACS
	beam_level_init();				// initialize beam weapons
	mflash_level_init();
	ssm_level_init();	
	supernova_level_init();
	cameras_init();
	subtitles_init();



	// multiplayer dogfight hack
	dogfight_blown = 0;

	shipfx_engine_wash_level_init();

	nebl_level_init();

	Last_view_target = NULL;
	Game_paused = 0;

	Game_no_clear = 0;

	// campaign wasn't ended
	Campaign_ended_in_mission = 0;

	cube_map_drawen = false;


}

// called when a mission is over -- does server specific stuff.
void freespace_stop_mission()
{	
	game_level_close();
	Game_mode &= ~GM_IN_MISSION;
}

#ifndef NO_NETWORK
// called at frame interval to process networking stuff
void game_do_networking()
{
	Assert( Net_player != NULL );
	if (!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// see if this player should be reading/writing data.  Bit is set when at join
	// screen onward until quits back to main menu.
	if ( !(Net_player->flags & NETINFO_FLAG_DO_NETWORKING) ){
		return;
	}

	if(gameseq_get_state()!=GS_STATE_MULTI_PAUSED){
		multi_do_frame();
	} else {
		multi_pause_do_frame();
	}	
}
#endif // ifndef NO_NETWORK


// Loads the best palette for this level, based
// on nebula color and hud color.  You could just call palette_load_table with
// the appropriate filename, but who wants to do that.
void game_load_palette()
{
	char palette_filename[1024];

	// We only use 3 hud colors right now
	// Assert( HUD_config.color >= 0 );
	// Assert( HUD_config.color <= 2 );

	Assert( Mission_palette >= 0 );
	Assert( Mission_palette <= 98 );

	// if ( The_mission.flags & MISSION_FLAG_SUBSPACE )	{
		strcpy( palette_filename, NOX("gamepalette-subspace") );
	// } else {
		// sprintf( palette_filename, NOX("gamepalette%d-%02d"), HUD_config.color+1, Mission_palette+1 );
	// }

	mprintf(( "Loading palette %s\n", palette_filename ));

	// palette_load_table(palette_filename);
}

void game_post_level_init()
{
	// Stuff which gets called after mission is loaded.  Because player isn't created until
	// after mission loads, some things must get initted after the level loads

	model_level_post_init();

 	HUD_init();
	hud_setup_escort_list();
	mission_hotkey_set_defaults();	// set up the default hotkeys (from mission file)

	stars_level_init();	

	// While trying to track down the nebula bug I encountered a cool effect -
	// comment this out to fly a mission in a void. Maybe we should develop this
	// into a full effect or something, because it is seriously cool.
	neb2_level_init();		

#ifndef NDEBUG
	game_event_debug_init();
#endif

	training_mission_init();
	asteroid_create_all();
	
	game_framerate_check_init();
}


// An estimate as to how high the count passed to game_loading_callback will go.
// This is just a guess, it seems to always be about the same.   The count is
// proportional to the code being executed, not the time, so this works good
// for a bar, assuming the code does about the same thing each time you
// load a level.   You can find this value by looking at the return value
// of game_busy_callback(NULL), which I conveniently print out to the
// debug output window with the '=== ENDING LOAD ==' stuff.   
//#define COUNT_ESTIMATE 3706
//#define COUNT_ESTIMATE 1111
#define COUNT_ESTIMATE 2311

int Game_loading_callback_inited = 0;

int Game_loading_background = -1;
anim * Game_loading_ani = NULL;
anim_instance	*Game_loading_ani_instance;
int Game_loading_frame=-1;

static int Game_loading_ani_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		63, 316  // GR_640
	},
	{
		101, 505	// GR_1024
	}
};

#ifndef NDEBUG
extern char Processing_filename[MAX_PATH_LEN];
static int busy_shader_created = 0;
shader busy_shader;
#endif
// This gets called 10x per second and count is the number of times 
// game_busy() has been called since the current callback function
// was set.
void game_loading_callback(int count)
{	
#ifndef NO_NETWORK
	game_do_networking();
#endif

	Assert( Game_loading_callback_inited==1 );
	Assert( Game_loading_ani != NULL );

	int do_flip = 0;

	int framenum = ((Game_loading_ani->total_frames*count) / COUNT_ESTIMATE)+1;
	if ( framenum > Game_loading_ani->total_frames-1 )	{
		framenum = Game_loading_ani->total_frames-1;
	} else if ( framenum < 0 )	{
		framenum = 0;
	}

	static int last_cbitmap = -1;
	int cbitmap = -1;
	while ( Game_loading_frame < framenum )	{
		Game_loading_frame++;
		cbitmap = anim_get_next_frame(Game_loading_ani_instance);
	}
	

	if ( cbitmap > -1 )	{
		if ( Game_loading_background > -1 )	{
			gr_set_bitmap( Game_loading_background );
			gr_bitmap(0,0);
		}

		//mprintf(( "Showing frame %d/%d [ Bitmap=%d ]\n", Game_loading_frame ,  Game_loading_ani->total_frames, cbitmap ));
		gr_set_bitmap( cbitmap );
		gr_bitmap(Game_loading_ani_coords[gr_screen.res][0],Game_loading_ani_coords[gr_screen.res][1]);

		if ( (last_cbitmap > -1) && (last_cbitmap != cbitmap) )
			bm_release(last_cbitmap);
	
		do_flip = 1;
	}

#ifndef NDEBUG
	// print the current filename being processed by game_busy(), the shader here is a quick hack
	// since the background isn't always drawn so we can't clear the text away from the previous
	// filename. the shader is completely opaque to hide the old text. must easier and faster than
	// redrawing the entire screen every flip - taylor
	if (!busy_shader_created) {
		gr_create_shader(&busy_shader, 5, 5, 5, 255);
		busy_shader_created = 1;
	}

	if (Processing_filename[0] != '\0') {
		if ( cbitmap == -1 && last_cbitmap >-1 ){
			if ( Game_loading_background > -1 )	{
				gr_set_bitmap( Game_loading_background );
				gr_bitmap(0,0);
			}
			gr_set_bitmap( last_cbitmap );
			gr_bitmap(Game_loading_ani_coords[gr_screen.res][0],Game_loading_ani_coords[gr_screen.res][1]);
		}

		gr_set_shader(&busy_shader);
		gr_shade(0, 0, gr_screen.clip_width_unscaled, 17); // make sure it goes across the entire width

		gr_set_color_fast(&Color_white);
		gr_string(5, 5, Processing_filename);

		do_flip = 1;
		memset( Processing_filename, 0, MAX_PATH_LEN );
	}
#endif

#ifndef NDEBUG
	if(Cmdline_show_mem_usage)
	{
#ifdef _WIN32
		void memblockinfo_sort();
		void memblockinfo_sort_get_entry(int index, char *filename, int *size);

		char mem_buffer[1000];
		char filename[35];
		int size;
		int i;
	  	memblockinfo_sort();
		for(i = 0; i < 30; i++)
		{
			memblockinfo_sort_get_entry(i, filename, &size);

			size /= 1024;

			if(size == 0)
				break;

			char *short_name = strrchr(filename, '\\');
			if(short_name == NULL)
				short_name = filename;
			else
				short_name++;

			sprintf(mem_buffer,"%s:\t%d K", short_name, size);
			gr_string( 20, 220 + (i*10), mem_buffer);
		}
		sprintf(mem_buffer,"Total RAM:\t%d K", TotalRam / 1024);
		gr_string( 20, 230 + (i*10), mem_buffer);
#endif	// _WIN32
	}
#endif	// !NDEBUG

	if (cbitmap != -1)
		last_cbitmap = cbitmap;

	if (do_flip) {
		gr_flip();
	}
}

void game_loading_callback_init()
{
	Assert( Game_loading_callback_inited==0 );

	Game_loading_background = bm_load(The_mission.loading_screen[gr_screen.res]);
	
	if (Game_loading_background < 0)
	{
		Game_loading_background = bm_load(Game_loading_bground_fname[gr_screen.res]);
	}
	//common_set_interface_palette("InterfacePalette");  // set the interface palette


	Game_loading_ani = anim_load( Game_loading_ani_fname[gr_screen.res]);
	Assert( Game_loading_ani != NULL );
	Game_loading_ani_instance = init_anim_instance(Game_loading_ani, 16);
	Assert( Game_loading_ani_instance != NULL );
	Game_loading_frame = -1;

	Game_loading_callback_inited = 1;
	Mouse_hidden = 1;
	game_busy_callback( game_loading_callback, (COUNT_ESTIMATE/Game_loading_ani->total_frames)+1 );	


}

void game_loading_callback_close()
{
	Assert( Game_loading_callback_inited==1 );

	// Make sure bar shows all the way over.
	game_loading_callback(COUNT_ESTIMATE);
	
	int real_count = game_busy_callback( NULL );
 	Mouse_hidden = 0;

	Game_loading_callback_inited = 0;
	
#ifndef NDEBUG
	mprintf(( "=================== ENDING LOAD ================\n" ));
	mprintf(( "Real count = %d,  Estimated count = %d\n", real_count, COUNT_ESTIMATE ));
	mprintf(( "================================================\n" ));
#else
	// to remove warnings in release build
	real_count = 0;
#endif

	free_anim_instance(Game_loading_ani_instance);
	Game_loading_ani_instance = NULL;
	anim_free(Game_loading_ani);
	Game_loading_ani = NULL;

	bm_release( Game_loading_background );
	common_free_interface_palette();		// restore game palette
	Game_loading_background = -1;

	gr_set_font( FONT1 );
}

// Update the sound environment (ie change EAX settings based on proximity to large ships)
//
void game_maybe_update_sound_environment()
{
	// do nothing for now
}

// Assign the sound environment for the game, based on the current mission
//
void game_assign_sound_environment()
{
/*
	if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
		Game_sound_env.id = SND_ENV_DRUGGED;
		Game_sound_env.volume = 0.800f;
		Game_sound_env.damping = 1.188f;
		Game_sound_env.decay = 6.392f;
#ifndef FS2_DEMO
	} else if (Num_asteroids > 30) {
		Game_sound_env.id = SND_ENV_AUDITORIUM;
		Game_sound_env.volume = 0.603f;
		Game_sound_env.damping = 0.5f;
		Game_sound_env.decay = 4.279f;
#endif
	} else {
		Game_sound_env = Game_default_sound_env;
	}
	*/
#ifndef NO_SOUND
	Game_sound_env = Game_default_sound_env;
	Game_sound_env_update_timestamp = timestamp(1);
#endif
}

// function which gets called before actually entering the mission.  It is broken down into a funciton
// since it will get called in one place from a single player game and from another place for
// a multiplayer game
void freespace_mission_load_stuff()
{
	// called if we're not on a freespace dedicated (non rendering, no pilot) server
	// IE : we _don't_ want to load any sounds or bitmap/texture info on this machine.
	if(!(Game_mode & GM_STANDALONE_SERVER)){	
	
		mprintf(( "=================== STARTING LEVEL DATA LOAD ==================\n" ));

	//	game_loading_callback_init();

		game_busy( NOX("** setting up event music **") );
		event_music_level_init(-1);	// preloads the first 2 seconds for each event music track

		game_busy( NOX("** unloading interface sounds **") );
		gamesnd_unload_interface_sounds();		// unload interface sounds from memory

		game_busy( NOX("** preloading common game sounds **") );
		gamesnd_preload_common_sounds();			// load in sounds that are expected to play

		if (Cmdline_snd_preload) {
			game_busy( NOX("** preloading gameplay sounds **") );
			gamesnd_load_gameplay_sounds();			// preload in gameplay sounds if wanted
		}

		game_busy( NOX("** assigning sound environment for mission **") );
		ship_assign_sound_all();	// assign engine sounds to ships
		game_assign_sound_environment();	 // assign the sound environment for this mission

		// call function in missionparse.cpp to fixup player/ai stuff.
		game_busy( NOX("** fixing up player/ai stuff **") );
		mission_parse_fixup_players();

		// Load in all the bitmaps for this level
		level_page_in();

		game_busy( NOX("** finished with level_page_in() **") );

		game_loading_callback_close();	
	} 
	// the only thing we need to call on the standalone for now.
	else {
		// call function in missionparse.cpp to fixup player/ai stuff.
		mission_parse_fixup_players();

		// Load in all the bitmaps for this level
		level_page_in();
	}
}

uint load_gl_init;
uint load_mission_load;
uint load_post_level_init;
uint load_mission_stuff;

// tells the server to load the mission and initialize structures
int game_start_mission()
{	
	mprintf(( "=================== STARTING LEVEL LOAD ==================\n" ));


	char temp_fname[MAX_FILENAME_LEN];
	strcpy(temp_fname, Game_current_mission_filename);
	char *p = strchr(temp_fname, '.');
	if (p) *p = 0; // remove any extension
	strcat(temp_fname, FS_MISSION_FILE_EXT);  // append mission extension
	get_mission_info(temp_fname, &The_mission);

	game_loading_callback_init();

	game_busy( NOX("** starting game_level_init() **") );
	load_gl_init = time(NULL);
	game_level_init();
	load_gl_init = time(NULL) - load_gl_init;
	
#ifndef NO_NETWORK
	if (Game_mode & GM_MULTIPLAYER) {
		Player->flags |= PLAYER_FLAGS_IS_MULTI;

		// clear multiplayer stats
		init_multiplayer_stats();
	}
#endif

	game_busy( NOX("** starting mission_load() **") );
	load_mission_load = time(NULL);
	if (mission_load()) {
#ifndef NO_NETWORK
		if ( !(Game_mode & GM_MULTIPLAYER) ) {
			popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "Attempt to load the mission failed", 169));
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		} else {
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_LOAD_FAIL);
		}
#else
			popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "Attempt to load the mission failed", 169));
			gameseq_post_event(GS_EVENT_MAIN_MENU);
#endif // ifndef NO_NETWORK
		return 0;
	}
	load_mission_load = time(NULL) - load_mission_load;

	// If this is a red alert mission in campaign mode, bash wingman status
	if ( (Game_mode & GM_CAMPAIGN_MODE) && red_alert_mission() ) {
		red_alert_bash_wingman_status();
	}

	// the standalone server in multiplayer doesn't do any rendering, so we will not even bother loading the palette
	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		mprintf(( "=================== LOADING GAME PALETTE ================\n" ));
		// game_load_palette();
	}

	game_busy( NOX("** starting game_post_level_init() **") );
	load_post_level_init = time(NULL);
	game_post_level_init();
	load_post_level_init = time(NULL) - load_post_level_init;

#ifndef NDEBUG
	{
		void Do_model_timings_test();
		Do_model_timings_test();	
	}
#endif

	load_mission_stuff = time(NULL);
	freespace_mission_load_stuff();
	load_mission_stuff = time(NULL) - load_mission_stuff;

	//set the inital animation positions
/*
		ship_obj *moveup = GET_FIRST(&Ship_obj_list);
		ship *shipp;
		while((moveup != END_OF_LIST(&Ship_obj_list)) && (moveup != NULL)){
			// bogus
			if((moveup->objnum < 0) || (moveup->objnum >= MAX_OBJECTS) || (Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0) || (Objects[moveup->objnum].instance >= MAX_SHIPS) || (Ships[Objects[moveup->objnum].instance].ship_info_index < 0) || (Ships[Objects[moveup->objnum].instance].ship_info_index >= Num_ship_types)){
				moveup = GET_NEXT(moveup);
				continue;
			}
			shipp = &Ships[Objects[moveup->objnum].instance];

			ship_animation_set_inital_states(shipp);

			moveup = GET_NEXT(moveup);
		}
*/
	bm_print_bitmaps();

	return 1;
}

int Interface_framerate = 0;
#ifndef NDEBUG

DCF_BOOL( mouse_control, Use_mouse_to_fly )
DCF_BOOL( show_framerate, Show_framerate )
DCF_BOOL( show_target_debug_info, Show_target_debug_info )
DCF_BOOL( show_target_weapons, Show_target_weapons )
DCF_BOOL( lead_target_cheat, Players[Player_num].lead_target_cheat )
#ifndef NO_SOUND
DCF_BOOL( sound, Sound_enabled )
#endif
DCF_BOOL( zbuffer, game_zbuffer )
DCF_BOOL( shield_system, New_shield_system )
DCF_BOOL( show_shield_mesh, Show_shield_mesh)
DCF_BOOL( player_attacking, Player_attacking_enabled )
DCF_BOOL( show_waypoints, Show_waypoints )
DCF_BOOL( show_area_effect, Show_area_effect )
DCF_BOOL( show_net_stats, Show_net_stats )
DCF_BOOL( log, Log_debug_output_to_file )
extern int Training_message_method;
DCF_BOOL( training_msg_method, Training_message_method )
DCF_BOOL( show_player_pos, Show_player_pos )
DCF_BOOL(i_framerate, Interface_framerate )

DCF(show_mem,"Toggles showing mem usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_mem = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_mem = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_mem ^= 1;	

		if ( Show_mem )	{
			Show_cpu = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_mem\nSets show_mem to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	
	}
}

DCF(show_cpu,"Toggles showing cpu usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_cpu = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_cpu = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_cpu ^= 1;	

		if ( Show_cpu )	{
			Show_mem = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_cpu\nSets show_cpu to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	

	}
}

#endif

			int Game_init_seed;

DCF(use_joy_mouse,"Makes joystick move mouse cursor")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_joy_mouse = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_joy_mouse = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_joy_mouse ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: use_joy_mouse [bool]\nSets use_joy_mouse to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "use_joy_mouse is %s\n", (Use_joy_mouse?"TRUE":"FALSE") );	

	os_config_write_uint( NULL, NOX("JoystickMovesCursor"), Use_joy_mouse );
}

DCF(palette_flash,"Toggles palette flash effect on/off")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_palette_flash = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_palette_flash = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_palette_flash ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: palette_flash [bool]\nSets palette_flash to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "palette_flash is %s\n", (Use_palette_flash?"TRUE":"FALSE") );	
}

int Use_low_mem = 0;

DCF(low_mem,"Uses low memory settings regardless of RAM")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_low_mem = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_low_mem = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_low_mem ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: low_mem [bool]\nSets low_mem to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "low_mem is %s\n", (Use_low_mem?"TRUE":"FALSE") );	

	os_config_write_uint( NULL, NOX("LowMem"), Use_low_mem );
}


#ifndef NDEBUG

DCF(force_fullscreen, "Forces game to startup in fullscreen mode")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_fullscreen_at_startup = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_fullscreen_at_startup = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_fullscreen_at_startup ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: force_fullscreen [bool]\nSets force_fullscreen to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "force_fullscreen is %s\n", (Use_fullscreen_at_startup?"TRUE":"FALSE") );	
	os_config_write_uint( NULL, NOX("ForceFullscreen"), Use_fullscreen_at_startup );
}
#endif

int	Framerate_delay = 0;

float Freespace_gamma = 1.0f;

DCF(gamma,"Sets Gamma factor")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_FLOAT )	{
			Freespace_gamma = Dc_arg_float;
		} else {
			dc_printf( "Gamma reset to 1.0f\n" );
			Freespace_gamma = 1.0f;
		}
		if ( Freespace_gamma < 0.1f )	{
			Freespace_gamma = 0.1f;
		} else if ( Freespace_gamma > 5.0f )	{
			Freespace_gamma = 5.0f;
		}
		gr_set_gamma(Freespace_gamma);

		char tmp_gamma_string[32];
		sprintf( tmp_gamma_string, NOX("%.2f"), Freespace_gamma );
		os_config_write_string( NULL, NOX("Gamma"), tmp_gamma_string );
	}

	if ( Dc_help )	{
		dc_printf( "Usage: gamma <float>\n" );
		dc_printf( "Sets gamma in range 1-3, no argument resets to default 1.2\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Gamma = %.2f\n", Freespace_gamma );
	}
}

void run_launcher()
{
#ifdef _WIN32
	const char *launcher_link = "explorer.exe \"http://www.randomtiger.pwp.blueyonder.co.uk/freespace/Launcher5.rar\"";

	int download = MessageBox((HWND)os_get_window(), 
		"Run the fs2_open launcher to fix your problem. "
		"Would you like to download the latest version of the launcher? "
		"You must have at least version 5.0 to run fs2_open versions above 3.6.", 
		"Question", MB_YESNO | MB_ICONQUESTION);

	if(download == IDYES)
	{
		// Someone should change this to the offical link
		WinExec(launcher_link, SW_SHOW);
		return;
	}

	// This now crashes the launcher since fs2_open is still open
	return;

	// fire up the UpdateLauncher executable
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset( &si, 0, sizeof(STARTUPINFO) );
	si.cb = sizeof(si);

	BOOL launcher_ran = CreateProcess(	LAUNCHER_FNAME,	// pointer to name of executable module 
								NULL,							// pointer to command line string
								NULL,							// pointer to process security attributes 
								NULL,							// pointer to thread security attributes 
								FALSE,							// handle inheritance flag 
								CREATE_DEFAULT_ERROR_MODE,		// creation flags 
								NULL,							// pointer to new environment block 
								NULL,	// pointer to current directory name 
								&si,	// pointer to STARTUPINFO 
								&pi 	// pointer to PROCESS_INFORMATION  
							);			

	// If the Launcher could not be started up, let the user know and give them the option of downloading it
	if (!launcher_ran) 
	{
		download = MessageBox((HWND)os_get_window(), 
			"The Launcher could not be started. You cannot run fs2_open without it. "
			"Would you like to download it?", "FS2_Open Startup Error", MB_YESNO | MB_ICONQUESTION);

		if(download == IDYES)
		{
			// Someone should change this to the offical link
			WinExec(launcher_link, SW_SHOW);
		}
	}
#endif
}

#ifdef APPLE_APP
char full_path[1024];
#endif

void game_init()
{


	char *ptr;

	Game_current_mission_filename[0] = 0;

	// seed the random number generator
	Game_init_seed = time(NULL);
	srand( Game_init_seed );

	Framerate_delay = 0;

	#ifndef NDEBUG
	load_filter_info();
	#endif

	// encrypt stuff
	encrypt_init();

	// Initialize the timer before the os
	timer_init();

	int s1, e1;
	// int s2, e2;

	char whee[1024];

#ifdef APPLE_APP
	// some OSX hackery to drop us out of the APP the binary is run from
	char *c = NULL;
	c = strstr(full_path, ".app");
	if ( c != NULL ) {
		while (c && (*c != '/'))
			c--;

		*c = '\0';
	}
	strncpy(whee, full_path, 1024);
#else
	GetCurrentDirectory(1024, whee);
#endif
	strcat(whee, DIR_SEPARATOR_STR);
	strcat(whee, EXE_FNAME);

	//Initialize the libraries
	s1 = timer_get_milliseconds();
#ifdef _WIN32
	if(cfile_init(whee, Game_CDROM_dir)){			// initialize before calling any cfopen stuff!!!
#else
	if(cfile_init(whee, NULL)){			// initialize before calling any cfopen stuff!!!
#endif
		exit(1);
	}		
	e1 = timer_get_milliseconds();

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
	if (Is_standalone) {
		std_init_standalone();
	}
	else
#else
	Assert(!Is_standalone);
#endif
	{		
		os_init( Osreg_class_name, Osreg_app_name );
	}

	// initialize localization module. Make sure this is down AFTER initialzing OS.
	lcl_init( detect_lang() );	
	lcl_xstr_init();

	// verify that he has a valid ships.tbl (will Game_ships_tbl_valid if so)
	verify_ships_tbl();

	// verify that he has a valid weapons.tbl
	verify_weapons_tbl();

	// commented out by penguin --
	//   we don't want to cause problems w/ retail version...
	//   we probably need our own registry key (and a launcher to write them)

//  	// Output version numbers to registry for auto patching purposes
	Use_joy_mouse = 0;		//os_config_read_uint( NULL, NOX("JoystickMovesCursor"), 1 );
	//Use_palette_flash = os_config_read_uint( NULL, NOX("PaletteFlash"), 0 );
	Use_low_mem = os_config_read_uint( NULL, NOX("LowMem"), 0 );

#ifndef NDEBUG
	Use_fullscreen_at_startup = os_config_read_uint( NULL, NOX("ForceFullscreen"), 1 );
#endif

	Asteroids_enabled = 1;		

/////////////////////////////
// SOUND INIT START
/////////////////////////////

	int use_a3d = 0;
	int use_eax = 0;

	ptr = os_config_read_string(NULL, NOX("Soundcard"), NULL);
	mprintf(("soundcard = %s\n", ptr ? ptr : "<nothing>"));
	if (ptr) {
		if (!stricmp(ptr, NOX("no sound"))) {
			Cmdline_freespace_no_sound = 1;

		} else if (!stricmp(ptr, NOX("Aureal A3D"))) {
			use_a3d = 1;
		} else if (!stricmp(ptr, NOX("EAX"))) {
			use_eax = 1;
		}
	}
#ifndef SCP_UNIX
	else
	{
		run_launcher();
		exit(0);
	}
#endif

	if (!Is_standalone)
	{
		UserSampleRate = (ushort) os_config_read_uint(NULL, "SoundSampleRate", 22050);
		UserSampleBits = (ushort) os_config_read_uint(NULL, "SoundSampleBits", 16);
		snd_init(use_a3d, use_eax, UserSampleRate, UserSampleBits);
	}

	if(fsspeech_init() == false) {
		mprintf(("Failed to init speech\n"));

		if(Cmdline_query_speech)
		{
			if(!fsspeech_was_compiled()) 
				MessageBox((HWND)os_get_window(), "Speech is not compiled in this build in code.lib", "FS2_Open Warning", MB_ICONWARNING);
			else
				MessageBox((HWND)os_get_window(), "Speech is compiled, but failed to init", "FS2_Open Warning", MB_ICONWARNING);	
		}
	} else if(Cmdline_query_speech) {
		// Its bad practice to use a negative type, this is an exceptional case
		fsspeech_play(-1,"Welcome to FS2 open");
		MessageBox((HWND)os_get_window(), "Speech is compiled and initialised and should be working", "FS2_Open Info", MB_OK);
	}
 

/////////////////////////////
// SOUND INIT END
/////////////////////////////

	// check for hi res pack file 
	int has_sparky_hi = 0;

	CFILE *the_mask = cfopen("2_ChoosePilot-m.pcx", "r");
	if(the_mask != NULL)
	{
		cfclose(the_mask);
		has_sparky_hi = 1;
	}

	if(!Is_standalone)
	{
		int width, height, cdepth;
		char Device_init_error[512];		

		// We cannot continue without this, quit, but try to help the user out first
		ptr = os_config_read_string(NULL, NOX("VideocardFs2open"), NULL); 

		if(ptr == NULL)
		{
			strcpy(Device_init_error, "Cant get 'VideocardFs2open' reg entry.");
		}
		else 
		{
			bool is_640x480 = true; // default to 640x480 res

			// NOTE: The "ptr+5" is to skip over the initial "????-" in the video string.
			//       If the format of that string changes you'll have to change this too!!!
			if ( sscanf(ptr+5, "(%dx%d)", &width, &height) == 2 ) {
				// if we are less than 1024x768 res then stay at low-res, otherwise bump it
				is_640x480 = ( (width < 1024) && (height < 768) );
			} else {
				mprintf(("Couldn't determine if we are really using 640x480 res or not!  Going with default..."));
			}

			int res = (!is_640x480 && has_sparky_hi) ? GR_1024 : GR_640;
			
			// D3D9 should run on a seperate path to D3D8
			// This will allow users to play D3D8 while we develope D3D9
			if (strstr(ptr, NOX("D3D9-") ))	
			{
				int adapter, aatype;

				if(sscanf(ptr, "D3D9-(%dx%d)x%d bit ad%d aa%d", &width, &height, &cdepth, &adapter, &aatype)  != 5) 
					strcpy(Device_init_error, "Cant understand 'VideocardFs2open' D3D9 reg entry.");
				else
				{
					sprintf(Device_init_error, 
						"This build does not support D3D9.\n\n"
						"Adapter num: %d\n"
						"Mode:        %dx%dx%d\n"
						"AA type:     %d\n", adapter, width, height, cdepth, aatype);
				}
			}
			else if (strstr(ptr, NOX("D3D8-") ))	
			{
				if(sscanf(ptr, "D3D8-(%dx%d)x%d bit", &width, &height, &cdepth)  != 3) 
					strcpy(Device_init_error, "Cant understand 'VideocardFs2open' D3D8 reg entry.");
				else
					gr_init(res, GR_DIRECT3D, cdepth, width, height);
			} 
			else if (strstr(ptr, NOX("OGL -") ))
			{
				if(sscanf(ptr, "OGL -(%dx%d)x%d bit", &width, &height, &cdepth)  != 3) 
					strcpy(Device_init_error, "Cant understand 'VideocardFs2open' OGL reg entry.");
				else
					gr_init(res, GR_OPENGL, cdepth, width, height);
			} 
			else if (strstr(ptr, NOX("3DFX Glide"))) 
			{
				strcpy(Device_init_error, "Glide is not supported any more.");
			} 
			else if(strstr(ptr, NOX("Direct 3D -")))
			{
				strcpy(Device_init_error, "Direct3D5 not supported any more.");
			}
			else 
			{
				strcpy(Device_init_error, "Software mode is unsupported. GET A PROGRAMMER!!");
			}
		}
			
		extern int Gr_inited;
		if(!Gr_inited)
		{
#ifdef _WIN32
			ClipCursor(NULL);
			ShowCursor(TRUE);
			ShowWindow((HWND)os_get_window(),SW_MINIMIZE);
			MessageBox( NULL, Device_init_error, "Error intializing graphics", MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
			run_launcher();
#elif defined(SCP_UNIX)
			fprintf(stderr, "Error initializing graphics: %s\n\n", Device_init_error);
			
			// the default entry should have been created already if it didn't exist, so if we're here then
			// the current value is invalid and we need to replace it
			os_config_write_string(NULL, NOX("VideocardFs2open"), NOX("OGL -(640x480)x16 bit"));
			
			// courtesy
			fprintf(stderr, "The default video entry is now in place.  Please try running the game again...\n");
			fprintf(stderr, "(edit ~/.fs2_open/fs2_open.ini to change from default resolution)\n");
#endif
			exit(1);
			return;
		}
	}
	else
	{
		gr_init(GR_640, GR_STUB, 16, 640, 480);
	}

	// this has to be set after gr_init() is done on *nix
	os_set_title(Osreg_title);

//	ENVMAP = bm_load("environment");

	// Set the gamma
	if( (gr_screen.mode == GR_DIRECT3D) || (gr_screen.mode == GR_OPENGL) )
	{
		// D3D's gamma system now works differently. 1.0 is the default value
		ptr = os_config_read_string(NULL, NOX("GammaD3D"), NOX("1.0"));
		Freespace_gamma = (float)atof(ptr);
	}
	else
	{
		ptr = os_config_read_string(NULL, NOX("Gamma"), NOX("1.80"));
		Freespace_gamma = (float)atof(ptr);
		
		// Keep the old system for the benifit of OGL
		if ( Freespace_gamma < 0.1f )	{
			Freespace_gamma = 0.1f;
		} else if ( Freespace_gamma > 5.0f )	{
			Freespace_gamma = 5.0f;
		}
		char tmp_gamma_string[32];
		sprintf( tmp_gamma_string, NOX("%.2f"), Freespace_gamma );
		os_config_write_string( NULL, NOX("Gamma"), tmp_gamma_string );
	}

	

//#if defined(FS2_DEMO) || defined(OEM_BUILD)
	// add title screen
	if(!Is_standalone){
		// #Kazan# - moved this down - WATCH THESE calls - anything that shares code between standalone and normal
		// cannot make gr_* calls in standalone mode because all gr_ calls are NULL pointers
		gr_set_gamma(Freespace_gamma);
		display_title_screen();
	}
//#endif
	
#ifndef NO_NETWORK
	// attempt to load up master tracker registry info (login and password)
	Multi_tracker_id = -1;		

	// should we be using this or not?
	Om_tracker_flag = os_config_read_uint( "PXO", "FS2OpenPXO" , 0 );
	// pxo login and password
	ptr = os_config_read_string(NOX("PXO"),NOX("Login"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO login data\n"));
		strcpy(Multi_tracker_login,"");
	} else {		
		strcpy(Multi_tracker_login,ptr);
	}
	ptr = os_config_read_string(NOX("PXO"),NOX("Password"),NULL);
	if(ptr == NULL){		
		nprintf(("Network","Error reading PXO password\n"));
		strcpy(Multi_tracker_passwd,"");
	} else {		
		strcpy(Multi_tracker_passwd,ptr);
	}	

	// pxo squad name and password
	ptr = os_config_read_string(NOX("PXO"),NOX("SquadName"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO squad name\n"));
		strcpy(Multi_tracker_squad_name, "");
	} else {		
		strcpy(Multi_tracker_squad_name, ptr);
	}
#endif  // ifndef NO_NETWORK

	// If less than 48MB of RAM, use low memory model.
	if (
#ifdef _WIN32
		(Freespace_total_ram < 48*1024*1024) ||
#endif
		Use_low_mem )	{
		mprintf(( "Using normal memory settings...\n" ));
		bm_set_low_mem(1);		// Use every other frame of bitmaps
	} else {
		mprintf(( "Using high memory settings...\n" ));
		bm_set_low_mem(0);		// Use all frames of bitmaps
	}

	//WMC - Initialize my new GUI system
	//This may seem scary, but it should take up 0 processing time and very little memory
	//as long as it's not being used.
	//Otherwise, it just keeps the parsed interface.tbl in memory.
	GUI_system.ParseClassInfo("interface.tbl");

	// load non-darkening pixel defs
	palman_load_pixels();

	extern void iff_init();
	iff_init();						// Goober5000 - this must be done even before species_defs :p

	extern void species_init();
	species_init();					// Load up the species defs - this needs to be done FIRST -- Kazan

	brief_parse_icon_tbl();

	// hud shield icon stuff
	//hud_shield_game_init(); No longer needed; see ships.tbl -C

	control_config_common_init();				// sets up localization stuff in the control config

	parse_rank_tbl();
#ifndef FS2_DEMO
	parse_medal_tbl();
	//WMC - gave badge_stuff a deconstructor
/*
	void medal_close();
	atexit(medal_close);
*/
#endif
	cutscene_init();
	key_init();
	mouse_init();
	gamesnd_parse_soundstbl();

	if (!Is_standalone)
		select_radar_mode(Cmdline_orb_radar);

	gameseq_init();

#ifndef NO_NETWORK
	multi_init();	
#endif  // ifndef NO_NETWORK

	// standalone's don't use hte joystick and it seems to sometimes cause them to not get shutdown properly
	if(!Is_standalone){
		joy_init();
	}

	player_controls_init();
	model_init();	

	event_music_init();

	obj_init();	
	mflash_game_init();	
	armor_init();
	weapon_init();	
	ai_init();		
	ship_init();						// read in ships.tbl	

	player_init();	
	mission_campaign_init();		// load in the default campaign	
	anim_init();
	context_help_init();			
	techroom_intel_init();			// parse species.tbl, load intel info  
	if(!Is_standalone)
	{
		hud_positions_init();		//Setup hud positions
	}
	
#ifndef NO_NETWORK
	// initialize psnet
	psnet_init( Multi_options_g.protocol, Multi_options_g.port );						// initialize the networking code		
#endif
	init_animating_pointer();	
	asteroid_init();
	mission_brief_common_init();	// Mark all the briefing structures as empty.		
	gr_font_init();					// loads up all fonts		

	neb2_init();						// fullneb stuff
	nebl_init();
	stars_init();
	ssm_init();	
	player_tips_init();				// helpful tips
	beam_init();
	
	// load the list of pilot pic filenames (for barracks and pilot select popup quick reference)
	pilot_load_pic_list();	
	pilot_load_squad_pic_list();

	load_animating_pointer(NOX("cursor"), 0, 0);	

	// initialize alpha colors
	alpha_colors_init();	

	if(Cmdline_cell)cell_shaded_lightmap = bm_load("cellmap");
	Viewer_mode = 0;
//	Game_music_paused = 0;
	Game_paused = 0;

#ifdef _WIN32
	timeBeginPeriod(1);	
#endif

	nprintf(("General", "Ships.tbl is : %s\n", Game_ships_tbl_valid ? "VALID" : "INVALID!!!!"));
	nprintf(("General", "Weapons.tbl is : %s\n", Game_weapons_tbl_valid ? "VALID" : "INVALID!!!!"));

	mprintf(("cfile_init() took %d\n", e1 - s1));
	// mprintf(("1000 cfopens() took %d\n", e2 - s2));	


}

char transfer_text[128];

float	Start_time = 0.0f;

float Framerate = 0.0f;

float Timing_total = 0.0f;
float Timing_render2 = 0.0f;
float Timing_render3 = 0.0f;
float Timing_flip = 0.0f;
float Timing_clear = 0.0f;

MONITOR(NumPolysDrawn);
MONITOR(NumPolys);
MONITOR(NumVerts);
MONITOR(BmpUsed);
MONITOR(BmpNew);


int Mem_starttime_phys;
int Mem_starttime_pagefile;
int Mem_starttime_virtual;

void game_get_framerate()
{	
 //	if(The_mission.flags & MISSION_FLAG_FULLNEB)
 //		gr_set_color(0,0,255);
 //	else
	gr_set_color_fast(&HUD_color_debug);
#ifdef WMC
	//WMC - this code spits out the target of all turrets
	if(Player_ai->target_objnum != -1
		&& Objects[Player_ai->target_objnum].type == OBJ_SHIP)
	{
		//Debug crap
		int t = 0;
		ship_subsys	*pss;

		object *objp = &Objects[Player_ai->target_objnum];
		for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) )
		{
			if(pss->system_info->type == SUBSYSTEM_TURRET)
			{
				if(pss->turret_enemy_objnum == -1)
					gr_printf(10, t*10, "Turret %d: <None>", t);
				else if(Objects[pss->turret_enemy_objnum].type == OBJ_SHIP)
					gr_printf(10, t*10, "Turret %d: %s", t, Ships[Objects[pss->turret_enemy_objnum].instance].ship_name);
				else
					gr_printf(10, t*10, "Turret %d: <Object %d>", t, pss->turret_enemy_objnum);

				t++;
			}
		}
	}
#endif
	char text[128] = "";

	//Show the viewin pos

	if ( frame_int == -1 )	{
		int i;
		for (i=0; i<FRAME_FILTER; i++ )	{
			frametimes[i] = 0.0f;
		}
		frametotal = 0.0f;
		frame_int = 0;
	}
	frametotal -= frametimes[frame_int];
	frametotal += flFrametime;
	frametimes[frame_int] = flFrametime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if ( frametotal != 0.0 )	{
		if ( Framecount >= FRAME_FILTER )
			Framerate = FRAME_FILTER / frametotal;
		else
			Framerate = Framecount / frametotal;
		sprintf( text, NOX("FPS: %.1f"), Framerate * (Game_time_compression>>16));
	} else {
		sprintf( text, NOX("FPS: ?") );
	}
	Framecount++;

	if ((Show_framerate) || (Cmdline_show_fps))	{
		gr_string( 20, 100, text );
	}

	if(Cmdline_show_stats)
	{
#ifdef _WIN32
		char mem_buffer[50];

		if(gr_screen.mode == GR_DIRECT3D)
		{
			extern void d3d_string_mem_use(int x, int y);
			d3d_string_mem_use(20, 110);
		}

		MEMORYSTATUS mem_stats;
		GlobalMemoryStatus(&mem_stats);

		sprintf(mem_buffer,"Using Physical: %d Meg",(Mem_starttime_phys - mem_stats.dwAvailPhys)/1024/1024);
		gr_string( 20, 120, mem_buffer);
		sprintf(mem_buffer,"Using Pagefile: %d Meg",(Mem_starttime_pagefile - mem_stats.dwAvailPageFile)/1024/1024);
		gr_string( 20, 130, mem_buffer);
		sprintf(mem_buffer,"Using Virtual:  %d Meg",(Mem_starttime_virtual - mem_stats.dwAvailVirtual)/1024/1024);
		gr_string( 20, 140, mem_buffer);

		sprintf(mem_buffer,"Physical Free: %d / %d Meg",mem_stats.dwAvailPhys/1024/1024, mem_stats.dwTotalPhys/1024/1024);
		gr_string( 20, 160, mem_buffer);
		sprintf(mem_buffer,"Pagefile Free: %d / %d Meg",mem_stats.dwAvailPageFile/1024/1024, mem_stats.dwTotalPageFile/1024/1024);
		gr_string( 20, 170, mem_buffer);
		sprintf(mem_buffer,"Virtual Free:  %d / %d Meg",mem_stats.dwAvailVirtual/1024/1024, mem_stats.dwTotalVirtual/1024/1024);
		gr_string( 20, 180, mem_buffer);
#elif defined(SCP_UNIX)
	STUB_FUNCTION;
#endif
	}

#ifndef NDEBUG
	if(Cmdline_show_mem_usage)
	{
#ifdef _WIN32
		void memblockinfo_sort();
		void memblockinfo_sort_get_entry(int index, char *filename, int *size);

		char mem_buffer[1000];
		char filename[MAX_PATH];
		int size;
	  	memblockinfo_sort();
		for(int i = 0; i < 30; i++)
		{
			memblockinfo_sort_get_entry(i, filename, &size);

			size /= 1024;

			if(size == 0)
				break;

			char *short_name = strrchr(filename, '\\');
			if(short_name == NULL)
				short_name = filename;
			else
				short_name++;

			sprintf(mem_buffer,"%s:\t%d K", short_name, size);
			gr_string( 20, 220 + (i*10), mem_buffer);
		}
		sprintf(mem_buffer,"Total RAM:\t%d K", TotalRam / 1024);
		gr_string( 20, 230 + (i*10), mem_buffer);
#endif
	}
#endif
}

void game_show_framerate()
{	
	float	cur_time;

	cur_time = f2fl(timer_get_approx_seconds());
	if (cur_time - Start_time > 30.0f) {
		mprintf(("%i frames executed in %7.3f seconds, %7.3f frames per second.\n", Framecount, cur_time - Start_time, Framecount/(cur_time - Start_time)));
		Start_time += 1000.0f;
	}

	//mprintf(( "%s\n", text ));
/*	if(frame_rate_display && (flFrametime != 0.0f)){
		gr_set_color( 128, 255, 192 );
		gr_printf(15, 500, "fps: %f", 1.0f / flFrametime);
	}
*/

#ifndef NDEBUG
	if ( Debug_dump_frames )
		return;
#endif	

	// possibly show control checking info
	control_check_indicate();

//	int bitmaps_used_this_frame, bitmaps_new_this_frame;
//	MONITOR_INC(BmpUsed, bitmaps_used_this_frame);
// MONITOR_INC(BmpNew, bitmaps_new_this_frame);

#ifndef NDEBUG
	if ( Show_cpu == 1 ) {
		
		int sx,sy,dy;
		sx = 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

#ifndef NO_DIRECT3D
		{
			extern int D3D_textures_in;
			extern int D3D_textures_in_frame;
			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (D3D_textures_in)/1024 );
			sy += dy;
			gr_printf( sx, sy, NOX("VRAM: +%d KB\n"), (D3D_textures_in_frame)/1024 );
			sy += dy;
		}
#endif

//		gr_printf( sx, sy, "BPP: %d", gr_screen.bits_per_pixel );
//		sy += dy;
		gr_printf( sx, sy, NOX("DMA: %s"), transfer_text );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYP: %d"), modelstats_num_polys );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYD: %d"), modelstats_num_polys_drawn );
		sy += dy;
		gr_printf( sx, sy, NOX("VERTS: %d"), modelstats_num_verts );
		sy += dy;

		{

			extern int Num_pairs;		// Number of object pairs that were checked.
			gr_printf( sx, sy, NOX("PAIRS: %d"), Num_pairs );
			sy += dy;

			extern int Num_pairs_checked;	// What percent of object pairs were checked.
			gr_printf( sx, sy, NOX("FVI: %d"), Num_pairs_checked );
			sy += dy;
			Num_pairs_checked = 0;

		}

		gr_printf( sx, sy, NOX("Snds: %d"), snd_num_playing() );
		sy += dy;

		if ( Timing_total > 0.01f )	{
			gr_printf(  sx, sy, NOX("CLEAR: %.0f%%"), Timing_clear*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND2D: %.0f%%"), Timing_render2*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND3D: %.0f%%"), Timing_render3*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("FLIP: %.0f%%"), Timing_flip*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("GAME: %.0f%%"), (Timing_total-(Timing_render2+Timing_render3+Timing_flip+Timing_clear))*100.0f/Timing_total );
			sy += dy;
		}
	}
	 	
	if ( Show_mem  ) {
		
		int sx,sy,dy;
		sx = GR_1024 ? 870 : 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

		{
			extern int TotalRam;
			gr_printf( sx, sy, NOX("DYN: %d KB\n"), TotalRam/1024 );
			sy += dy;
		}	

		{
			extern int Model_ram;
			gr_printf( sx, sy, NOX("POF: %d KB\n"), Model_ram/1024 );
			sy += dy;
		}	

		gr_printf( sx, sy, NOX("%s: %d KB\n"), (Cmdline_cache_bitmaps) ? NOX("C-BMP") : NOX("BMP"), bm_texture_ram/1024 );
		sy += dy;

#ifndef NO_SOUND
		gr_printf( sx, sy, NOX("S-SRAM: %d KB\n"), Snd_sram/1024 );		// mem used to store game sound
		sy += dy;
#ifndef USE_OPENAL
		gr_printf( sx, sy, NOX("S-HRAM: %d KB\n"), Snd_hram/1024 );		// mem used to store game sound
		sy += dy;
#endif
#endif

#ifndef NO_DIRECT3D
		if ( gr_screen.mode == GR_DIRECT3D ) {
			extern int D3D_textures_in;
			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (D3D_textures_in)/1024 );
			sy += dy;
		} else
#endif
		{
			extern int GL_textures_in;
			extern int GL_vertex_data_in;
			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (GL_textures_in + GL_vertex_data_in)/1024 );
			sy += dy;
		}
	}


	if ( Show_player_pos ) {
		int sx, sy;
		sx = 320;
		sy = 100;
		gr_printf(sx, sy, NOX("Player Pos: (%d,%d,%d)"), fl2i(Player_obj->pos.xyz.x), fl2i(Player_obj->pos.xyz.y), fl2i(Player_obj->pos.xyz.z));
	}

	MONITOR_INC(NumPolys, modelstats_num_polys);
	MONITOR_INC(NumPolysDrawn, modelstats_num_polys_drawn );
	MONITOR_INC(NumVerts, modelstats_num_verts );

	modelstats_num_polys = 0;
	modelstats_num_polys_drawn = 0;
	modelstats_num_verts = 0;
	modelstats_num_sortnorms = 0;
#endif
}

extern int Cmdline_show_pos;
void game_show_eye_pos(vec3d *eye_pos, matrix* eye_orient)
{
	if(!Cmdline_show_pos)
		return;

	//Do stuff
	int font_height = 2*gr_get_font_height();
	angles rot_angles;

	gr_set_color_fast(&HUD_color_debug);

	//Position
	gr_printf(20, 100 - font_height, "X:%f Y:%f Z:%f", eye_pos->xyz.x, eye_pos->xyz.y, eye_pos->xyz.z);
	font_height -= font_height/2;

	//Orientation
	vm_extract_angles_matrix(&rot_angles, eye_orient);
	rot_angles.p *= (180/PI);
	rot_angles.b *= (180/PI);
	rot_angles.h *= (180/PI);
	gr_printf(20, 100 - font_height, "Xr:%f Yr:%f Zr:%f", rot_angles.p, rot_angles.b, rot_angles.h);
}

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
void game_show_standalone_framerate()
{
	float frame_rate=30.0f;
	if ( frame_int == -1 )	{
		int i;
		for (i=0; i<FRAME_FILTER; i++ )	{
			frametimes[i] = 0.0f;
		}
		frametotal = 0.0f;
		frame_int = 0;
	}
	frametotal -= frametimes[frame_int];
	frametotal += flFrametime;
	frametimes[frame_int] = flFrametime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if ( frametotal != 0.0 )	{
		if ( Framecount >= FRAME_FILTER ){
			frame_rate = FRAME_FILTER / frametotal;
		} else {
			frame_rate = Framecount / frametotal;
		}
	}
	std_set_standalone_fps(frame_rate);
	Framecount++;
}
#endif

// function to show the time remaining in a mission.  Used only when the end-mission sexpression is used
void game_show_time_left()
{
	int diff;

	// mission_end_time is a global from missionparse.cpp that contains the mission time at which the
	// mission should end (in fixed seconds).  There is code in missionparse.cpp which actually handles
	// checking how much time is left

	if ( Mission_end_time == -1 ){
		return;
	}

	diff = f2i(Mission_end_time - Missiontime);
	// be sure to bash to 0.  diff could be negative on frame that we quit mission
	if ( diff < 0 ){
		diff = 0;
	}

	hud_set_default_color();
	gr_printf( 5, 40, XSTR( "Mission time remaining: %d seconds", 179), diff );
}

//========================================================================================
//=================== NEW DEBUG CONSOLE COMMANDS TO REPLACE OLD DEBUG PAUSE MENU =========
//========================================================================================

#ifndef NDEBUG

DCF(ai_pause,"Pauses ai")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	ai_paused = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) ai_paused = 0;	
		else if ( Dc_arg_type & ARG_NONE ) ai_paused = !ai_paused;	

		if (ai_paused)	{	
			obj_init_all_ships_physics();
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: ai_paused [bool]\nSets ai_paused to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "ai_paused is %s\n", (ai_paused?"TRUE":"FALSE") );	
}

DCF(single_step,"Single steps the game")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	game_single_step = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) game_single_step = 0;	
		else if ( Dc_arg_type & ARG_NONE ) game_single_step = !game_single_step;	

		last_single_step = 0;	// Make so single step waits a frame before stepping

	}	
	if ( Dc_help )	dc_printf( "Usage: single_step [bool]\nSets single_step to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "single_step is %s\n", (game_single_step?"TRUE":"FALSE") );	
}

DCF_BOOL(physics_pause, physics_paused)
DCF_BOOL(ai_rendering, Ai_render_debug_flag)
DCF_BOOL(ai_firing, Ai_firing_enabled )

// Create some simple aliases to these commands...
debug_command dc_s("s","shortcut for single_step",dcf_single_step);
debug_command dc_p("p","shortcut for physics_pause", dcf_physics_pause );
debug_command dc_r("r","shortcut for ai_rendering", dcf_ai_rendering );
debug_command dc_f("f","shortcut for ai_firing", dcf_ai_firing);
debug_command dc_a("a","shortcut for ai_pause", dcf_ai_pause);
#endif

//========================================================================================
//========================================================================================


void game_training_pause_do()
{
	int key;

	key = game_check_key();
	if (key > 0){
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	gr_flip();
}


void game_increase_skill_level()
{
	Game_skill_level++;
	if (Game_skill_level >= NUM_SKILL_LEVELS){
		Game_skill_level = 0;
	}
}

int	Player_died_time;

int View_percent = 100;


DCF(view, "Sets the percent of the 3d view to render.")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 5 ) || (Dc_arg_int <= 100) ) {
			View_percent = Dc_arg_int;
		} else {
			dc_printf( "Illegal value for view. (Must be from 5-100) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: view [n]\nwhere n is percent of view to show (5-100).\n");
	}
	
	if ( Dc_status ) {
		dc_printf("View is set to %d%%\n", View_percent );
	}
}


// Set the clip region for the 3d rendering window
void game_reset_view_clip()
{
	Cutscene_bar_flags = CUB_NONE;
	Cutscene_delta_time = 1.0f;
	Cutscene_bars_progress = 1.0f;
}
void game_set_view_clip(float frametime)
{
	if ((Game_mode & GM_DEAD) || (supernova_active() >= 2))
	{
		// Set the clip region for the letterbox "dead view"
		int yborder = gr_screen.max_h/4;

		//	Numeric constants encouraged by J "pig farmer" S, who shall remain semi-anonymous.
		// J.S. I've changed my ways!! See the new "no constants" code!!!
		gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2, false );	
	}
	else if((Cutscene_bar_flags & CUB_GRADUAL) && Cutscene_bars_progress < 1.0f)
	{
		//Determine how far along we are
		Assert(Cutscene_delta_time > 0.0f);

		Cutscene_bars_progress += frametime / Cutscene_delta_time;
		if(Cutscene_bars_progress >= 1.0f)
		{
			//Reset this stuff
			Cutscene_delta_time = 1.0f;
			Cutscene_bars_progress = 1.0f;
		}

		//Figure out where the bars should be
		int yborder;
		if(Cutscene_bar_flags & CUB_CUTSCENE)
			yborder = fl2i(Cutscene_bars_progress*(gr_screen.max_h/8));
		else
			yborder = gr_screen.max_h/6 - fl2i(Cutscene_bars_progress*(gr_screen.max_h/8));

		//Set teh clipping
		gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2, false );	
	}
	else if(Cutscene_bar_flags & CUB_CUTSCENE)
	{
		int yborder = gr_screen.max_h/6;

		gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2, false );	
	}
	else {
		// Set the clip region for normal view
		if ( View_percent >= 100 )	{
			gr_reset_clip();
		} else {
			int xborder, yborder;

			if ( View_percent < 5 )	{
				View_percent = 5;
			}

			float fp = i2fl(View_percent)/100.0f;
			int fi = fl2i(fl_sqrt(fp)*100.0f);
			if ( fi > 100 ) fi=100;
			
			xborder = ( gr_screen.max_w*(100-fi) )/200;
			yborder = ( gr_screen.max_h*(100-fi) )/200;

			gr_set_clip(xborder, yborder, gr_screen.max_w-xborder*2,gr_screen.max_h-yborder*2, false );
		}
	}
}


void show_debug_stuff()
{
	int	i;
	int	laser_count = 0, missile_count = 0;

	for (i=0; i<MAX_OBJECTS; i++) {
		if (Objects[i].type == OBJ_WEAPON){
			if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_LASER){
				laser_count++;
			} else if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_MISSILE){
				missile_count++;
			}
		}
	}

	nprintf(("Mike", "Frame: %i Lasers: %4i, Missiles: %4i\n", Framecount, laser_count, missile_count));
}

extern int Tool_enabled;
int tst = 0;
int tst_time = 0;
int tst_big = 0;
vec3d tst_pos;
int tst_bitmap = -1;
float tst_x, tst_y;
float tst_offset, tst_offset_total;
int tst_mode;
int tst_stamp;
void game_tst_frame_pre()
{
	// start tst
	if(tst == 3){
		tst = 0;

		// screen position
		vertex v;
		g3_rotate_vertex(&v, &tst_pos);
		g3_project_vertex(&v);	
	
		// offscreen
		if(!((v.sx >= 0) && (v.sx <= gr_screen.max_w) && (v.sy >= 0) && (v.sy <= gr_screen.max_h))){
			return;
		}	

		// big ship? always tst
		if(tst_big){
			// within 3000 meters
			if( vm_vec_dist_quick(&tst_pos, &Eye_position) <= 3000.0f){
				tst = 2;				
			}
		} else {			
			// within 300 meters
			if( (vm_vec_dist_quick(&tst_pos, &Eye_position) <= 300.0f) && ((tst_time == 0) || ((time(NULL) - tst_time) >= 10)) ){
				tst = 2;				
			} 
		}			
	}

}
void game_tst_frame()
{
	int left = 0;

	if(!Tool_enabled){
		return;
	}
	
	// setup tst
	if(tst == 2){		
		tst_time = time(NULL);

		// load the tst bitmap		
		switch((int)frand_range(0.0f, 3.0)){
		case 0:			
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 0\n"));
			break;

		case 1:
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 1\n"));
			break;

		case 2:
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 2\n"));
			break;
			
		default:			
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 3\n"));
			break;
		}

		if(tst_bitmap < 0){
			tst = 0;
			return;
		}		

		// get the tst bitmap dimensions
		int w, h;
		bm_get_info(tst_bitmap, &w, &h);

		// tst y
		tst_y = frand_range(0.0f, (float)gr_screen.max_h - h);

		snd_play(&Snds[SND_VASUDAN_BUP]);

		// tst x and direction
		tst_mode = 0;
		if(left){
			tst_x = (float)-w;
			tst_offset_total = (float)w;
			tst_offset = (float)w;
		} else {
			tst_x = (float)gr_screen.max_w;
			tst_offset_total = (float)-w;
			tst_offset = (float)w;
		}

		tst = 1;
	}

	// run tst
	if(tst == 1){
		float diff = (tst_offset_total / 0.5f) * flFrametime;

		// move the bitmap
		if(tst_mode == 0){
			tst_x += diff;
			
			tst_offset -= fl_abs(diff);
		} else if(tst_mode == 2){
			tst_x -= diff;
			
			tst_offset -= fl_abs(diff);
		}

		// draw the bitmap
		gr_set_bitmap(tst_bitmap);
		gr_bitmap((int)tst_x, (int)tst_y);

		if(tst_mode == 1){
			if(timestamp_elapsed_safe(tst_stamp, 1100)){
				tst_mode = 2;
			}
		} else {
			// if we passed the switch point
			if(tst_offset <= 0.0f){
				// switch modes
				switch(tst_mode){
				case 0:
					tst_mode = 1;
					tst_stamp = timestamp(1000);
					tst_offset = fl_abs(tst_offset_total);
					break;				

				case 2:				
					tst = 0;
					return;
				}
			}				
		}
	}
}
void game_tst_mark(object *objp, ship *shipp)
{
	ship_info *sip;	

	if(!Tool_enabled){
		return;
	}

	// bogus
	if((objp == NULL) || (shipp == NULL) || (shipp->ship_info_index < 0) || (shipp->ship_info_index >= Num_ship_types)){
		return;
	}
	sip = &Ship_info[shipp->ship_info_index];

	// already tst
	if(tst){
		return;
	}

	tst_pos = objp->pos;
	if(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
		tst_big = 1;
	}
	tst = 3;
}

extern void render_shields();

void player_repair_frame(float frametime)
{
#ifndef NO_NETWORK
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		int idx;
		for(idx=0;idx<MAX_PLAYERS;idx++){
			net_player *np;

			np = &Net_players[idx];

			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != NULL) && (Net_player->player_id != Net_players[idx].player_id) && (Net_players[idx].m_player != NULL) && (Net_players[idx].m_player->objnum >= 0) && (Net_players[idx].m_player->objnum < MAX_OBJECTS)){

				// don't rearm/repair if the player is dead or dying/departing
				if ( !NETPLAYER_IS_DEAD(np) && !(Ships[Objects[np->m_player->objnum].instance].flags & (SF_DYING|SF_DEPARTING)) ) {
					ai_do_repair_frame(&Objects[Net_players[idx].m_player->objnum],&Ai_info[Ships[Objects[Net_players[idx].m_player->objnum].instance].ai_index],frametime);
				}
			}
		}
	}	
#endif  // ifndef NO_NETWORK
	if ( (Player_obj != NULL) && (Player_obj->type == OBJ_SHIP) && !(Game_mode & GM_STANDALONE_SERVER) && (Player_ship != NULL) && !(Player_ship->flags & SF_DYING) ) {
		ai_do_repair_frame(Player_obj, &Ai_info[Ships[Player_obj->instance].ai_index], frametime);
	}
}


//#ifndef NDEBUG
#define NUM_FRAMES_TEST		300
#define NUM_MIXED_SOUNDS	16
void do_timing_test(float frame_time)
{
	static int framecount = 0;
	static int test_running = 0;
	static float test_time = 0.0f;

	static int snds[NUM_MIXED_SOUNDS];
	int i;

	if ( test_running ) {
		framecount++;
		test_time += frame_time;
		if ( framecount >= NUM_FRAMES_TEST ) {
			test_running = 0;
			nprintf(("General", "%d frames took %.3f seconds\n", NUM_FRAMES_TEST, test_time));
			for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
				snd_stop(snds[i]);
		}
	}

	if ( Test_begin == 1 ) {
		framecount = 0;
		test_running = 1;
		test_time = 0.0f;
		Test_begin = 0;

		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = -1;

		// start looping digital sounds
		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = snd_play_looping( &Snds[i], 0.0f, -1, -1);
	}
	

}
//#endif

DCF(dcf_fov, "Change the field of view")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			Viewer_zoom = VIEWER_ZOOM_DEFAULT;
			dc_printf( "Zoom factor reset\n" );
		}
		if ( Dc_arg_type & ARG_FLOAT )	{
			if (Dc_arg_float < 0.25f) {
				Viewer_zoom = 0.25f;
				dc_printf("Zoom factor pinned at 0.25.\n");
			} else if (Dc_arg_float > 1.25f) {
				Viewer_zoom = 1.25f;
				dc_printf("Zoom factor pinned at 1.25.\n");
			} else {
				Viewer_zoom = Dc_arg_float;
			}
		}
	}

	if ( Dc_help )	
		dc_printf( "Usage: fov [factor]\nFactor is the zoom factor btwn .25 and 1.25\nNo parameter resets it to default.\n" );

	if ( Dc_status )				
		dc_printf("Zoom factor set to %6.3f (original = 0.5, John = 0.75)", Viewer_zoom);
}


DCF(framerate_cap, "Sets the framerate cap")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 1 ) || (Dc_arg_int <= 120) ) {
			Framerate_cap = Dc_arg_int;
		} else {
			dc_printf( "Illegal value for framerate cap. (Must be from 1-120) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: framerate_cap [n]\nwhere n is the frames per second to cap framerate at.\n");
		dc_printf("If n is 0 or omitted, then the framerate cap is removed\n");
		dc_printf("[n] must be from 1 to 120.\n");
	}
	
	if ( Dc_status ) {
		if ( Framerate_cap )
			dc_printf("Framerate cap is set to %d fps\n", Framerate_cap );
		else
			dc_printf("There is no framerate cap currently active.\n");
	}
}

#define	MIN_DIST_TO_DEAD_CAMERA		50.0f
int Show_viewing_from_self = 0;

void say_view_target()
{
	object	*view_target;

	if ((Viewer_mode & VM_OTHER_SHIP) && (Player_ai->target_objnum != -1))
		view_target = &Objects[Player_ai->target_objnum];
	else
		view_target = Player_obj;

	if (Game_mode & GM_DEAD) {
		if (Player_ai->target_objnum != -1)
			view_target = &Objects[Player_ai->target_objnum];
	}

	if (!(Game_mode & GM_DEAD_DIED) && ((Game_mode & (GM_DEAD_BLEW_UP)) || ((Last_view_target != NULL) && (Last_view_target != view_target)))) {
		if (view_target != Player_obj){

			char *view_target_name = NULL;
			switch(Objects[Player_ai->target_objnum].type) {
			case OBJ_SHIP:
				view_target_name = Ships[Objects[Player_ai->target_objnum].instance].ship_name;
				break;
			case OBJ_WEAPON:
				view_target_name = Weapon_info[Weapons[Objects[Player_ai->target_objnum].instance].weapon_info_index].name;
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
			case OBJ_JUMP_NODE: {
				char	jump_node_name[128];
				strcpy(jump_node_name, XSTR( "jump node", 184));
				view_target_name = jump_node_name;
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
				}

			default:
				Int3();
				break;
			}

			if ( view_target_name ) {
				HUD_fixed_printf(0.0f, XSTR( "Viewing %s%s\n", 185), (Viewer_mode & VM_OTHER_SHIP) ? XSTR( "from ", 186) : "", view_target_name);
				Show_viewing_from_self = 1;
			}
		} else {
#ifndef NO_NETWORK
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) && (Player_obj->type == OBJ_OBSERVER)){
				HUD_fixed_printf(2.0f,XSTR( "Viewing from observer\n", 187));
				Show_viewing_from_self = 1;
			} else {
				if (Show_viewing_from_self)
					HUD_fixed_printf(2.0f, XSTR( "Viewing from self\n", 188));
			}
#else
				if (Show_viewing_from_self)
					HUD_fixed_printf(2.0f, XSTR( "Viewing from self\n", 188));
#endif  // NO_NETWORK
		}
	}

	Last_view_target = view_target;
}


float Game_hit_x = 0.0f;
float Game_hit_y = 0.0f;

// Reset at the beginning of each frame
void game_whack_reset()
{
	Game_hit_x = 0.0f;
	Game_hit_y = 0.0f;
}

// Apply a 2d whack to the player
void game_whack_apply( float x, float y )
{
	// Do some force feedback
	joy_ff_play_dir_effect(x * 80.0f, y * 80.0f);

	// Move the eye 
	Game_hit_x += x;
	Game_hit_y += y;

//	mprintf(( "WHACK = %.1f, %.1f\n", Game_hit_x, Game_hit_y ));
}

// call to apply a "shudder"
void game_shudder_apply(int time, float intensity)
{
	Game_shudder_time = timestamp(time);
	Game_shudder_total = time;
	Game_shudder_intensity = intensity;
}

#define FF_SCALE	10000
void apply_hud_shake(matrix *eye_orient)
{
	if (Viewer_obj == Player_obj) {
		physics_info	*pi = &Player_obj->phys_info;

		angles	tangles;

		tangles.p = 0.0f;
		tangles.h = 0.0f;
		tangles.b = 0.0f;

		//	Make eye shake due to afterburner
		if ( !timestamp_elapsed(pi->afterburner_decay) ) {			
			int		dtime;

			dtime = timestamp_until(pi->afterburner_decay);
			
			int r1 = myrand();
			int r2 = myrand();
			tangles.p += 0.07f * (float) (r1-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/ABURN_DECAY_TIME));
			tangles.h += 0.07f * (float) (r2-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/ABURN_DECAY_TIME));
		}

		// Make eye shake due to engine wash
		extern int Wash_on;
		if (Player_obj->type == OBJ_SHIP && (Ships[Player_obj->instance].wash_intensity > 0) && Wash_on ) {
			int r1 = myrand();
			int r2 = myrand();
			tangles.p += 0.07f * Ships[Player_obj->instance].wash_intensity * (float) (r1-RAND_MAX/2)/RAND_MAX;
			tangles.h += 0.07f * Ships[Player_obj->instance].wash_intensity * (float) (r2-RAND_MAX/2)/RAND_MAX;

			// get the   intensity
			float intensity = FF_SCALE * Ships[Player_obj->instance].wash_intensity;

			// vector rand_vec
			vec3d rand_vec;
			vm_vec_rand_vec_quick(&rand_vec);

			// play the effect
			joy_ff_play_dir_effect(intensity*rand_vec.xyz.x, intensity*rand_vec.xyz.y);
		}

	
		// make hud shake due to shuddering
		if(Game_shudder_time != -1){
			// if the timestamp has elapsed
			if(timestamp_elapsed(Game_shudder_time)){
				Game_shudder_time = -1;
			} 
			// otherwise apply some shudder
			else {
				int dtime;

				dtime = timestamp_until(Game_shudder_time);
			
				int r1 = myrand();
				int r2 = myrand();
				tangles.p += (Game_shudder_intensity / 200.0f) * (float) (r1-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/(float)Game_shudder_total));
				tangles.h += (Game_shudder_intensity / 200.0f) * (float) (r2-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/(float)Game_shudder_total));
			}
		}

		matrix	tm, tm2;
		vm_angles_2_matrix(&tm, &tangles);
		Assert(vm_vec_mag(&tm.vec.fvec) > 0.0f);
		Assert(vm_vec_mag(&tm.vec.rvec) > 0.0f);
		Assert(vm_vec_mag(&tm.vec.uvec) > 0.0f);
		vm_matrix_x_matrix(&tm2, eye_orient, &tm);
		*eye_orient = tm2;
	}
}

extern void compute_slew_matrix(matrix *orient, angles *a);	// TODO: move code to proper place and extern in header file

//	Player's velocity just before he blew up.  Used to keep camera target moving.
vec3d	Dead_player_last_vel = {1.0f, 1.0f, 1.0f};


inline void render_environment(int&i, matrix*new_orient, float new_zoom){
vec3d nv = ZERO_VECTOR;
	bm_set_render_target(
		(Game_subspace_effect)?
			gr_screen.dynamic_environment_map 
		: 
			gr_screen.static_environment_map
		, i); 
	gr_clear();

	g3_set_view_matrix( &nv, new_orient, new_zoom ); 
	gr_set_proj_matrix( (PI/2.0f), 1.0f, Min_draw_distance, 30000.0f); 
	gr_set_view_matrix(&Eye_position, &Eye_matrix); 
	if ( Game_subspace_effect ) { 
		stars_draw(0,0,0,1,1); 
	} else { 
		stars_draw(1,1,1,0,1); 
	} 
	gr_end_view_matrix(); 
	gr_end_proj_matrix(); 
	i++; 
}

extern float View_zoom, Canv_w2, Canv_h2;
void setup_environment_mapping(vec3d *eye_pos, matrix *eye_orient){
	if(gr_screen.mode != GR_DIRECT3D)return;
	if(!Cmdline_nohtl){
//		matrix old_orient = Eye_matrix;
//		vec3d old_position = Eye_position;
		matrix new_orient;
		float old_zoom = View_zoom, new_zoom = 0.925f;
//		vec3d nv = ZERO_VECTOR;
		int i = 0;

		{
			new_orient.vec.fvec.xyz.x = 1.0f;new_orient.vec.fvec.xyz.y = 0.0f;new_orient.vec.fvec.xyz.z = 0.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 1.0f;new_orient.vec.uvec.xyz.z = 0.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	

			new_orient.vec.fvec.xyz.x = -1.0f;new_orient.vec.fvec.xyz.y = 0.0f;new_orient.vec.fvec.xyz.z = 0.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 1.0f;new_orient.vec.uvec.xyz.z = 0.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	

			new_orient.vec.fvec.xyz.x = 0.0f;new_orient.vec.fvec.xyz.y = 1.0f;new_orient.vec.fvec.xyz.z = 0.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 0.0f;new_orient.vec.uvec.xyz.z = -1.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	

			new_orient.vec.fvec.xyz.x = 0.0f;new_orient.vec.fvec.xyz.y = -1.0f;new_orient.vec.fvec.xyz.z = 0.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 0.0f;new_orient.vec.uvec.xyz.z = 1.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	

			new_orient.vec.fvec.xyz.x = 0.0f;new_orient.vec.fvec.xyz.y = 0.0f;new_orient.vec.fvec.xyz.z = 1.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 1.0f;new_orient.vec.uvec.xyz.z = 0.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	

			new_orient.vec.fvec.xyz.x = 0.0f;new_orient.vec.fvec.xyz.y = 0.0f;new_orient.vec.fvec.xyz.z = -1.0f;
			new_orient.vec.uvec.xyz.x = 0.0f;new_orient.vec.uvec.xyz.y = 1.0f;new_orient.vec.uvec.xyz.z = 0.0f;
			vm_fix_matrix(&new_orient);
			render_environment(i,&new_orient, new_zoom);	
				
		}

//		View_zoom = old_zoom;
		g3_set_view_matrix( eye_pos, eye_orient, old_zoom );
		gr_set_proj_matrix( ((4.0f/9.0f)*(PI)*View_zoom), Canv_w2/Canv_h2, 1.0f, 30000.0f);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);

		bm_set_render_target(-1);

		gr_end_view_matrix();
		gr_end_proj_matrix();
//		first_frame = false;
	}
//	}
}

//	Set eye_pos and eye_orient based on view mode.
void game_render_frame_setup(vec3d *eye_pos, matrix *eye_orient)
{
	vec3d	eye_dir;

	static int last_Viewer_mode = 0;
	static int last_Game_mode = 0;
	static int last_Viewer_objnum = -1;
	static float last_Viewer_zoom = Viewer_zoom;

	//First, make sure we take into account 2D Missions.
	//These replace the normal player in-cockpit view with a topdown view.
	if(The_mission.flags & MISSION_FLAG_2D_MISSION)
	{
		if(!Viewer_mode)
		{
			Viewer_mode = VM_TOPDOWN;
		}
	}

	// This code is supposed to detect camera "cuts"... like going between
	// different views.

	// determine if we need to regenerate the nebula
	if(	(!(last_Viewer_mode & VM_EXTERNAL) && (Viewer_mode & VM_EXTERNAL)) ||							// internal to external 
			((last_Viewer_mode & VM_EXTERNAL) && !(Viewer_mode & VM_EXTERNAL)) ||							// external to internal
			(!(last_Viewer_mode & VM_DEAD_VIEW) && (Viewer_mode & VM_DEAD_VIEW)) ||							// non dead-view to dead-view
			((last_Viewer_mode & VM_DEAD_VIEW) && !(Viewer_mode & VM_DEAD_VIEW)) ||							// dead-view to non dead-view
			(!(last_Viewer_mode & VM_WARP_CHASE) && (Viewer_mode & VM_WARP_CHASE)) ||						// non warp-chase to warp-chase
			((last_Viewer_mode & VM_WARP_CHASE) && !(Viewer_mode & VM_WARP_CHASE)) ||						// warp-chase to non warp-chase
			(!(last_Viewer_mode & VM_OTHER_SHIP) && (Viewer_mode & VM_OTHER_SHIP)) ||						// non other-ship to other-ship
			((last_Viewer_mode & VM_OTHER_SHIP) && !(Viewer_mode & VM_OTHER_SHIP)) ||						// other-ship to non-other ship
			(!(last_Viewer_mode & VM_FREECAMERA) && (Viewer_mode & VM_FREECAMERA)) ||
			((last_Viewer_mode & VM_FREECAMERA) && !(Viewer_mode & VM_FREECAMERA)) ||
			(!(last_Viewer_mode & VM_TOPDOWN) && (Viewer_mode & VM_TOPDOWN)) ||
			((last_Viewer_mode & VM_TOPDOWN) && !(Viewer_mode & VM_TOPDOWN)) ||
			(last_Viewer_zoom != Viewer_zoom) ||
			((Viewer_mode & VM_OTHER_SHIP) && (last_Viewer_objnum != Player_ai->target_objnum)) 		// other ship mode, but targets changes
			) {

		// regenerate the nebula
		neb2_eye_changed();
	}		

	if ( (last_Viewer_mode != Viewer_mode) || (last_Game_mode != Game_mode) || (last_Viewer_zoom != Viewer_zoom))	{
		//mprintf(( "************** Camera cut! ************\n" ));
		last_Viewer_mode = Viewer_mode;
		last_Game_mode = Game_mode;
		last_Viewer_zoom = Viewer_zoom;

		// Camera moved.  Tell stars & debris to not do blurring.
		stars_camera_cut();		
	}

	say_view_target();

	if ( Viewer_mode & VM_PADLOCK_ANY ) {
		player_display_packlock_view();
	}
	
	if(!Time_compression_locked)
		game_set_view_clip(flFrametime);
	else
		game_set_view_clip(flRealframetime);

	if (Game_mode & GM_DEAD) {
		vec3d	vec_to_deader, view_pos;
		float		dist;

		Viewer_mode |= VM_DEAD_VIEW;

		if (Player_ai->target_objnum != -1) {
			int view_from_player = 1;

			if (Viewer_mode & VM_OTHER_SHIP) {
				//	View from target.
				Viewer_obj = &Objects[Player_ai->target_objnum];

				last_Viewer_objnum = Player_ai->target_objnum;

				if ( Viewer_obj->type == OBJ_SHIP ) {
					ship_get_eye( eye_pos, eye_orient, Viewer_obj );
					view_from_player = 0;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if ( view_from_player ) {
				//	View target from player ship.
				Viewer_obj = NULL;
				*eye_pos = Player_obj->pos;
				vm_vec_normalized_dir(&eye_dir, &Objects[Player_ai->target_objnum].pos, eye_pos);
				vm_vector_2_matrix(eye_orient, &eye_dir, NULL, NULL);
			}
		} else {
			dist = vm_vec_normalized_dir(&vec_to_deader, &Player_obj->pos, &Dead_camera_pos);
			
			if (dist < MIN_DIST_TO_DEAD_CAMERA)
				dist += flFrametime * 16.0f;

			vm_vec_scale(&vec_to_deader, -dist);
			vm_vec_add(&Dead_camera_pos, &Player_obj->pos, &vec_to_deader);
			
			view_pos = Player_obj->pos;

			if (!(Game_mode & GM_DEAD_BLEW_UP)) {
				Viewer_mode &= ~(VM_EXTERNAL | VM_CHASE);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, 25.0f * flFrametime);
				Dead_player_last_vel = Player_obj->phys_info.vel;
				//nprintf(("AI", "Player death roll vel = %7.3f %7.3f %7.3f\n", Player_obj->phys_info.vel.x, Player_obj->phys_info.vel.y, Player_obj->phys_info.vel.z));
			} else if (Player_ai->target_objnum != -1) {
				view_pos = Objects[Player_ai->target_objnum].pos;
			} else {
				//	Make camera follow explosion, but gradually slow down.
				vm_vec_scale_add2(&Player_obj->pos, &Dead_player_last_vel, flFrametime);
				view_pos = Player_obj->pos;
				vm_vec_scale(&Dead_player_last_vel, 0.99f);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, MIN(25.0f, vm_vec_mag_quick(&Dead_player_last_vel)) * flFrametime);
			}

			*eye_pos = Dead_camera_pos;

			vm_vec_normalized_dir(&eye_dir, &Player_obj->pos, eye_pos);

			vm_vector_2_matrix(eye_orient, &eye_dir, NULL, NULL);
			Viewer_obj = NULL;
		}
	} 

	// if supernova shockwave
	if(supernova_camera_cut()){
		// no viewer obj
		Viewer_obj = NULL;

		// call it dead view
		Viewer_mode |= VM_DEAD_VIEW;

		// set eye pos and orient
		supernova_set_view(eye_pos, eye_orient);
	} else {	
		//	If already blown up, these other modes can override.
		if (!(Game_mode & (GM_DEAD | GM_DEAD_BLEW_UP))) {
			Viewer_mode &= ~VM_DEAD_VIEW;

			if(!(Viewer_mode & VM_FREECAMERA))
				Viewer_obj = Player_obj;
 
			if (Viewer_mode & VM_OTHER_SHIP) {
				if (Player_ai->target_objnum != -1){
					Viewer_obj = &Objects[Player_ai->target_objnum];
					last_Viewer_objnum = Player_ai->target_objnum;
				} else {
					Viewer_mode &= ~VM_OTHER_SHIP;
					last_Viewer_objnum = -1;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if(Viewer_mode & VM_FREECAMERA) {
				Viewer_obj = NULL;
				*eye_pos = *Free_camera->get_position();
				*eye_orient = *Free_camera->get_orientation();
			} else if (Viewer_mode & VM_EXTERNAL) {
				matrix	tm, tm2;

				vm_angles_2_matrix(&tm2, &Viewer_external_info.angles);
				vm_matrix_x_matrix(&tm, &Viewer_obj->orient, &tm2);

				vm_vec_scale_add(eye_pos, &Viewer_obj->pos, &tm.vec.fvec, 2.0f * Viewer_obj->radius + Viewer_external_info.distance);

				vm_vec_sub(&eye_dir, &Viewer_obj->pos, eye_pos);
				vm_vec_normalize(&eye_dir);
				vm_vector_2_matrix(eye_orient, &eye_dir, &Viewer_obj->orient.vec.uvec, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(eye_orient, &Viewer_slew_angles);

			} else if ( Viewer_mode & VM_CHASE ) {
				vec3d	move_dir;
				vec3d aim_pt;
								
				

				if ( Viewer_obj->phys_info.speed < 0.1 )
					move_dir = Viewer_obj->orient.vec.fvec;
				else {
					move_dir = Viewer_obj->phys_info.vel;
					vm_vec_normalize(&move_dir);
				}

				//create a better 3rd person view if this is the player ship
				if (Viewer_obj==Player_obj)
				{
					//get a point 1000m forward of ship
					vm_vec_copy_scale(&aim_pt,&Viewer_obj->orient.vec.fvec,1000.0f);
					vm_vec_add2(&aim_pt,&Viewer_obj->pos);

					vm_vec_scale_add(eye_pos, &Viewer_obj->pos, &move_dir, -2.25f * Viewer_obj->radius - Viewer_chase_info.distance);
					vm_vec_scale_add2(eye_pos, &Viewer_obj->orient.vec.uvec, .625f * Viewer_obj->radius);
					vm_vec_sub(&eye_dir, &aim_pt, eye_pos);
					vm_vec_normalize(&eye_dir);
				}
				else
				{
					vm_vec_scale_add(eye_pos, &Viewer_obj->pos, &move_dir, -3.0f * Viewer_obj->radius - Viewer_chase_info.distance);
					vm_vec_scale_add2(eye_pos, &Viewer_obj->orient.vec.uvec, 0.75f * Viewer_obj->radius);
					vm_vec_sub(&eye_dir, &Viewer_obj->pos, eye_pos);
					vm_vec_normalize(&eye_dir);
				}
					
				// JAS: I added the following code because if you slew up using
				// Descent-style physics, eye_dir and Viewer_obj->orient.vec.uvec are
				// equal, which causes a zero-length vector in the vm_vector_2_matrix
				// call because the up and the forward vector are the same.   I fixed
				// it by adding in a fraction of the right vector all the time to the
				// up vector.
				vec3d tmp_up = Viewer_obj->orient.vec.uvec;
				vm_vec_scale_add2( &tmp_up, &Viewer_obj->orient.vec.rvec, 0.00001f );

				vm_vector_2_matrix(eye_orient, &eye_dir, &tmp_up, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(eye_orient, &Viewer_slew_angles);
			} else if ( Viewer_mode & VM_WARP_CHASE ) {
					*eye_pos = Camera_pos;

					ship * shipp = &Ships[Player_obj->instance];

					vm_vec_sub(&eye_dir, &shipp->warp_effect_pos, eye_pos);
					vm_vec_normalize(&eye_dir);
					vm_vector_2_matrix(eye_orient, &eye_dir, &Player_obj->orient.vec.uvec, NULL);
					Viewer_obj = NULL;
			} else if (Viewer_mode & VM_TOPDOWN) {
					angles rot_angles = {PI/2.0f,0.0f,0.0f};
					eye_pos->xyz.x = Viewer_obj->pos.xyz.x;
					eye_pos->xyz.y = Viewer_obj->pos.xyz.y + Viewer_obj->radius * 25.0f;
					eye_pos->xyz.z = Viewer_obj->pos.xyz.z;
					vm_angles_2_matrix(eye_orient, &rot_angles);
					Viewer_obj = NULL;
			} else {
				// get an eye position based upon the correct type of object
				switch(Viewer_obj->type){
				case OBJ_SHIP:
					// make a call to get the eye point for the player object
					ship_get_eye( eye_pos, eye_orient, Viewer_obj );
					break;
				case OBJ_OBSERVER:
					// make a call to get the eye point for the player object
					observer_get_eye( eye_pos, eye_orient, Viewer_obj );				
					break;
				default :
					Int3();
				}

				#ifdef JOHNS_DEBUG_CODE
				john_debug_stuff(&eye_pos, &eye_orient);
				#endif
			}
		}
	}

	if(!(Viewer_mode & VM_FREECAMERA))
		apply_hud_shake(eye_orient);

	// setup neb2 rendering
	neb2_render_setup(eye_pos, eye_orient);
	if(!Time_compression_locked)
		game_set_view_clip(flFrametime);
	else
		game_set_view_clip(flRealframetime);
}

#ifndef NDEBUG
extern void ai_debug_render_stuff();
#endif

int Game_subspace_effect = 0;
DCF_BOOL( subspace, Game_subspace_effect );

//stuff for ht&l. vars and such
extern float View_zoom, Canv_h2, Canv_w2;
extern int Cmdline_nohtl;

// Does everything needed to render a frame
void game_render_frame( vec3d *eye_pos, matrix *eye_orient )
{
	int dont_offset;

	g3_start_frame(game_zbuffer);
	g3_set_view_matrix( eye_pos, eye_orient, Viewer_zoom );
	
	// maybe offset the HUD (jitter stuff)
#ifndef NO_NETWORK
	dont_offset = ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER));
#else
	dont_offset = 0;
#endif
	HUD_set_offsets(Viewer_obj, !dont_offset);
	
	// for multiplayer clients, call code in Shield.cpp to set up the Shield_hit array.  Have to
	// do this becaues of the disjointed nature of this system (in terms of setup and execution).
	// must be done before ships are rendered
#ifndef NO_NETWORK
	if ( MULTIPLAYER_CLIENT ) {
		shield_point_multi_setup();
	}
#endif

	// don't set proj/view matrix before this call, breaks OGL
/*	gr_setup_background_fog(true);
	if ( Game_subspace_effect )	{
		stars_draw(0,0,0,1,0);
	} else {
		stars_draw(1,1,1,0,0);
	}
	gr_setup_background_fog(false);
*/
	if ( Game_subspace_effect )	{
		stars_draw(0,0,0,1,0);
	} else {
		stars_draw(1,1,1,0,0);
	}
	// Do the sunspot
	game_sunspot_process(flFrametime);
	

	gr_set_ambient_light(The_mission.ambient_light_level & 0xff, 
							(The_mission.ambient_light_level >> 8) & 0xff,
							(The_mission.ambient_light_level >> 16) & 0xff);

	if((!cube_map_drawen || Game_subspace_effect) && Cmdline_env){
		setup_environment_mapping(eye_pos, eye_orient);
		cube_map_drawen = true;
	}
#ifndef DYN_CLIP_DIST
	if (!Cmdline_nohtl) {
		gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * View_zoom,  gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
#endif
	
	bool draw_viewer_last;
	obj_render_all(obj_render, &draw_viewer_last);

	mflash_render_all();						// render all muzzle flashes	
	//	Why do we not show the shield effect in these modes?  Seems ok.
	//if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED | VM_CHASE | VM_DEAD_VIEW))) {
	render_shields();
	//}
	particle_render_all();					// render particles after everything else.
#ifdef DYN_CLIP_DIST
	if(!Cmdline_nohtl)
	{
		gr_end_proj_matrix();
		gr_end_view_matrix();
		gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * View_zoom,  gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
#endif
	gr_set_ambient_light(120,120,120);

	beam_render_all();						// render all beam weapons
	trail_render_all();						// render missilie trails after everything else.	

	// render nebula lightning
	nebl_render_all();

	// render local player nebula
	neb2_render_player();

	//Draw the viewer 'cause we didn't before.
	//This is so we can change the minimum clipping distance without messing everything up.
	if(draw_viewer_last && Viewer_obj)
	{
		gr_zbuffer_clear(TRUE);
		ship_render(Viewer_obj);
	}


#ifndef NDEBUG
	ai_debug_render_stuff();
#endif

#ifndef RELEASE_REAL
//	game_framerate_check();
#endif

#ifndef NDEBUG
#ifndef NO_SOUND
	extern void snd_spew_debug_info();
	snd_spew_debug_info();
#endif
#endif

	if(!Cmdline_nohtl)
	{
		gr_end_proj_matrix();
		gr_end_view_matrix();
	}
	//================ END OF 3D RENDERING STUFF ====================

	if(!(Viewer_mode & VM_FREECAMERA))
	{
		hud_show_radar();
	}

#ifndef NO_NETWORK
	if( (Game_detail_flags & DETAIL_FLAG_HUD) && !(Game_mode & GM_MULTIPLAYER) || ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER) ) ) {
#else
	if (Game_detail_flags & DETAIL_FLAG_HUD) {
#endif
		hud_maybe_clear_head_area();
		anim_render_all(0, flFrametime);
	}
		

#ifndef NO_NETWORK
	extern int Multi_display_netinfo;
	if(Multi_display_netinfo){
		extern void multi_display_netinfo();
		multi_display_netinfo();
	}	
#endif

	game_tst_frame_pre();

#ifndef NDEBUG
	do_timing_test(flFrametime);

#ifndef NO_NETWORK
	extern int OO_update_index;	
	multi_rate_display(OO_update_index, 375, 0);

	// test
	extern void oo_display();
	oo_display();			
#endif
#endif
	
	g3_end_frame();
}

//#define JOHNS_DEBUG_CODE	1

#ifdef JOHNS_DEBUG_CODE
void john_debug_stuff(vec3d *eye_pos, matrix *eye_orient)
{
	//if ( keyd_pressed[KEY_LSHIFT] )		
	{
		ship_subsys *tsys = Players[Player_num].targeted_subobject;
		if ( tsys )	{
			model_subsystem *turret = tsys->system_info;

			if (turret->type == SUBSYSTEM_TURRET )	{
				vec3d fvec, uvec;
				object * tobj = &Objects[Players[Player_num].targeted_subobject_parent];

				ship_model_start(tobj);

				model_find_world_point(eye_pos, &turret->turret_firing_point[0], turret->model_num, turret->turret_gun_sobj, &tobj->orient, &tobj->pos );
				model_find_world_dir(&fvec, &turret->turret_matrix.vec.fvec, turret->model_num, turret->turret_gun_sobj, &tobj->orient, NULL );
				model_find_world_dir(&uvec, &turret->turret_matrix.vec.uvec, turret->model_num, turret->turret_gun_sobj, &tobj->orient, NULL );
				
				vm_vector_2_matrix( eye_orient, &fvec, &uvec, NULL );

				ship_model_stop(tobj);

				Viewer_obj = NULL;
			}
		}

	}
}
#endif

// following function for dumping frames for purposes of building trailers.
#ifndef NDEBUG

// function to toggle state of dumping every frame into PCX when playing the game
DCF(dump_frames, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames_trigger, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30_trigger, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Triggered frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Triggered frame dumping is now OFF\n" );
		}
		
	}
}

void game_maybe_dump_frame()
{
	if ( !Debug_dump_frames ){
		return;
	}

	if( Debug_dump_trigger && !keyd_pressed[KEY_Q] ){
		return;
	}

	game_stop_time();

	gr_dump_frame();
	Debug_dump_frame_num++;

	game_start_time();
}
#endif

extern int Player_dead_state;

//	Flip the page and time how long it took.
void game_flip_page_and_time_it()
{	
	fix t1, t2,d;
	int t;
	t1 = timer_get_fixed_seconds();
	gr_flip();
	t2 = timer_get_fixed_seconds();
	d = t2 - t1;
	t = (gr_screen.max_w*gr_screen.max_h*gr_screen.bytes_per_pixel)/1024;
	sprintf( transfer_text, NOX("%d MB/s"), fixmuldiv(t,65,d) );
}

void game_simulation_frame()
{
//	mprintf(("about to render a frame\n"));

#ifndef NO_NETWORK
	// blow ships up in multiplayer dogfight
	if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (f2fl(Missiontime) >= 2.0f) && !dogfight_blown){
		// blow up all non-player ships
		ship_obj *moveup = GET_FIRST(&Ship_obj_list);
		ship *shipp;
		ship_info *sip;
		while((moveup != END_OF_LIST(&Ship_obj_list)) && (moveup != NULL)){
			// bogus
			if((moveup->objnum < 0) || (moveup->objnum >= MAX_OBJECTS) || (Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0) || (Objects[moveup->objnum].instance >= MAX_SHIPS) || (Ships[Objects[moveup->objnum].instance].ship_info_index < 0) || (Ships[Objects[moveup->objnum].instance].ship_info_index >= Num_ship_types)){
				moveup = GET_NEXT(moveup);
				continue;
			}
			shipp = &Ships[Objects[moveup->objnum].instance];
			sip = &Ship_info[shipp->ship_info_index];

			// only blow up small ships			
			if((sip->flags & SIF_SMALL_SHIP) && (multi_find_player_by_object(&Objects[moveup->objnum]) < 0) ){							
				// function to simply explode a ship where it is currently at
				ship_self_destruct( &Objects[moveup->objnum] );					
			}

			moveup = GET_NEXT(moveup);
		}

		dogfight_blown = 1;
	}
#endif  // ifndef NO_NETWORK

	// process AWACS stuff - do this first thing
	awacs_process();
//mprintf(("awacs procesed\n"));

	//Do autopilot stuff
	NavSystem_Do();

	// single player, set Player hits_this_frame to 0
	if ( !(Game_mode & GM_MULTIPLAYER) && Player ) {
		Player->damage_this_burst -= (flFrametime * MAX_BURST_DAMAGE  / (0.001f * BURST_DURATION));
		Player->damage_this_burst = MAX(Player->damage_this_burst, 0.0f);
	}

	// supernova
	supernova_process();
	if(supernova_active() >= 5){
		return;
	}

	// fire targeting lasers now so that 
	// 1 - created this frame
	// 2 - collide this frame
	// 3 - render this frame
	// 4 - ignored and deleted next frame
	// the basic idea being that because it con be confusing to deal with them on a multi-frame basis, they are only valid for
	// frame
	ship_process_targeting_lasers();	
//mprintf(("targeting lasers procesed\n"));

	// do this here so that it works for multiplayer
	if ( Viewer_obj ) {
		// get viewer direction
		int viewer_direction = PHYSICS_VIEWER_REAR;

		if(Viewer_mode == 0){
			viewer_direction = PHYSICS_VIEWER_FRONT;
		}
		if(Viewer_mode & VM_PADLOCK_UP){
			viewer_direction = PHYSICS_VIEWER_UP;
		}
		else if(Viewer_mode & VM_PADLOCK_REAR){
			viewer_direction = PHYSICS_VIEWER_REAR;
		} 
		else if(Viewer_mode & VM_PADLOCK_LEFT){
			viewer_direction = PHYSICS_VIEWER_LEFT;
		} 
		else if(Viewer_mode & VM_PADLOCK_RIGHT){
			viewer_direction = PHYSICS_VIEWER_RIGHT;
		}

		physics_set_viewer( &Viewer_obj->phys_info, viewer_direction );
	} else {
		physics_set_viewer( NULL, PHYSICS_VIEWER_FRONT );
	}
//mprintf(("view stuff done\n"));

#define	VM_PADLOCK_UP					(1 << 7)
#define	VM_PADLOCK_REAR				(1 << 8)
#define	VM_PADLOCK_LEFT				(1 << 9)
#define	VM_PADLOCK_RIGHT				(1 << 10)
		

#ifndef NO_NETWORK
	// evaluate mission departures and arrivals before we process all objects.
	if ( !(Game_mode & GM_MULTIPLAYER) || ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && !multi_endgame_ending()) ) {

		// we don't want to evaluate mission stuff when any ingame joiner in multiplayer is receiving
		// ships/wing packets.
		if ( !((Game_mode & GM_MULTIPLAYER) && (Netgame.flags & NG_FLAG_INGAME_JOINING_CRITICAL)) && !(Game_mode & GM_DEMO_PLAYBACK)){
			mission_parse_eval_stuff();
		}

		// if we're an observer, move ourselves seperately from the standard physics
		if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
			obj_observer_move(flFrametime);
		}
		
		// move all the objects now
		obj_move_all(flFrametime);

		// check for cargo reveal (this has an internal timestamp, so only runs every N ms)
		// AL: 3-15-98: It was decided to not let AI ships inspect cargo
		//	ship_check_cargo_all();
		if(!(Game_mode & GM_DEMO_PLAYBACK)){
			mission_eval_goals();
		}
	}
#else  // NO_NETWORK
	// mharris FIXME -- this is too fragile - too much of this stuff is
	// copied from the ifndef block above...
	if ( !(Game_mode & GM_DEMO_PLAYBACK)) {
		mission_parse_eval_stuff();
	}

	obj_move_all(flFrametime);

	if ( !(Game_mode & GM_DEMO_PLAYBACK)) {
		mission_eval_goals();
	}
#endif  // ifndef NO_NETWORK
//mprintf(("arivals\\departures done\n"));

	// always check training objectives, even in multiplayer missions. we need to do this so that the directives gauge works properly on clients
	if(!(Game_mode & GM_DEMO_PLAYBACK)){
		training_check_objectives();
	}
//	mprintf(("objectives procesed\n"));

	
#ifndef NO_NETWORK
	// do all interpolation now
	if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && !multi_endgame_ending() && !(Netgame.flags & NG_FLAG_SERVER_LOST)) {
		// client side processing of warping in effect stages
		multi_do_client_warp(flFrametime);     
	
		// client side movement of an observer
		if((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)){
			obj_observer_move(flFrametime);   
		}

		// move all objects - does interpolation now as well
		obj_move_all(flFrametime);


	}
//	mprintf(("interpolation procesed\n"));

#endif  // ifndef NO_NETWORK

	// only process the message queue when the player is "in" the game
	if ( !Pre_player_entry ){
		message_queue_process();				// process any messages send to the player
	}
//mprintf(("message queue procesed\n"));

	if(!(Game_mode & GM_DEMO_PLAYBACK)){
		message_maybe_distort();				// maybe distort incoming message if comms damaged
		player_repair_frame(flFrametime);	//	AI objects get repaired in ai_process, called from move code...deal with player.
		player_process_pending_praise();		// maybe send off a delayed praise message to the player
		player_maybe_play_all_alone_msg();	// mabye tell the player he is all alone	
	}
//mprintf(("assorted player stuff procesed\n"));

	if(!(Game_mode & GM_STANDALONE_SERVER)){		
		// process some stuff every frame (before frame is rendered)
		emp_process_local();

		hud_update_frame();						// update hud systems
//mprintf(("HUD updated\n"));



		if (!physics_paused)	{
			// Move particle system
			particle_move_all(flFrametime);	

			// Move missile trails
			trail_move_all(flFrametime);		

			// process muzzle flashes
			mflash_process_all();

			// Flash the gun flashes
			shipfx_flash_do_frame(flFrametime);			

			shockwave_move_all(flFrametime);	// update all the shockwaves
		}
//mprintf(("physics stuff procesed\n"));

		// subspace missile strikes
		ssm_process();
//mprintf(("SSM procesed\n"));

#ifndef NO_SOUND
		obj_snd_do_frame();						// update the object-linked persistant sounds
#endif
		game_maybe_update_sound_environment();
		snd_update_listener(&View_position, &Player_obj->phys_info.vel, &Player_obj->orient);

// AL: debug code used for testing ambient subspace sound (ie when enabling subspace through debug console)
#ifndef NDEBUG
		if ( Game_subspace_effect ) {
			game_start_subspace_ambient_sound();
		}
//mprintf(("sound procesed\n"));
#endif
	}
//	mprintf(("frame rendered\n"));
}

// Maybe render and process the dead-popup
void game_maybe_do_dead_popup(float frametime)
{
	if ( popupdead_is_active() ) {
		int leave_popup=1;
		int choice = popupdead_do_frame(frametime);

		if ( Game_mode & GM_NORMAL ) {
			switch(choice) {
			case 0:
				// CD CHECK				
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_ENTER_GAME);
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}					
				break;

			case 1:
				gameseq_post_event(GS_EVENT_END_GAME);
				break;

			case 2:
				// CD CHECK
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_START_GAME);					
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}					
				break;

			// this should only happen during a red alert mission
			case 3:				
				if (The_mission.flags & MISSION_FLAG_RED_ALERT)
				{
					// choose the previous mission
					mission_campaign_previous_mission();
				}
				else
				{
					// bogus?
					Int3();
				}
				
				// CD CHECK
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_START_GAME);
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}				
				break;

			default:
				leave_popup=0;
				break;
			}
		} else {
#ifndef NO_NETWORK
			switch( choice ) {

			case POPUPDEAD_DO_MAIN_HALL:
				multi_quit_game(PROMPT_NONE,-1);
				break;

			case POPUPDEAD_DO_RESPAWN:				
				multi_respawn_normal();
				event_music_player_respawn();
				break;

			case POPUPDEAD_DO_OBSERVER:
				multi_respawn_observer();
				event_music_player_respawn_as_observer();
				break;

			default:
				leave_popup = 0;
				break;
			}
#else
			// should never really be here if NO_NETWORK...
			Assert( Game_mode & GM_NORMAL );
#endif  // ifndef NO_NETWORK
		}

		if ( leave_popup ) {
			popupdead_close();
		}
	}
}

// returns true if player is actually in a game_play stats
int game_actually_playing()
{
	int state;

	state = gameseq_get_state();
	if ( (state != GS_STATE_GAME_PLAY) && (state != GS_STATE_DEATH_DIED) && (state != GS_STATE_DEATH_BLEW_UP) )
		return 0;
	else
		return 1;
}

// Draw the 2D HUD gauges
void game_render_hud_2d()
{
	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return;
	}
	
	HUD_render_2d(flFrametime);
	gr_reset_clip();
}

// Draw the 3D-dependant HUD gauges
void game_render_hud_3d(vec3d *eye_pos, matrix *eye_orient)
{
	g3_start_frame(0);		// 0 = turn zbuffering off
	g3_set_view_matrix( eye_pos, eye_orient, Viewer_zoom );

	if ( (Game_detail_flags & DETAIL_FLAG_HUD) && (supernova_active() < 3)/* && !(Game_mode & GM_MULTIPLAYER) || ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER) )*/ ) {
		HUD_render_3d(flFrametime);
	}

	// Diminish the palette effect
	game_flash_diminish(flFrametime);

	g3_end_frame();
}

//100% blackness
void game_reset_shade_frame()
{
	Fade_type = FI_NONE;
	Fade_delta_time = 1.0f;
	gr_create_shader(&Viewer_shader, 0, 0, 0, 0);
}
void game_shade_frame(float frametime)
{
	if(Viewer_shader.c == 0.0f && Fade_type != FI_FADEOUT)
		return;

	//Fade in or out if necessary
	if(Fade_type == FI_FADEOUT)
	{
		Viewer_shader.c += frametime * (255.0f / Fade_delta_time);
	}
	else if(Fade_type == FI_FADEIN)
	{
		Viewer_shader.c -= frametime * (255.0f / Fade_delta_time);
	}

	//Limit and set fade type if done
	if(Viewer_shader.c < 0.0f)
	{
		Viewer_shader.c = 0.0f;

		if(Fade_type == FI_FADEIN)
			Fade_type = FI_NONE;
	}
	if(Viewer_shader.c > 255.0f)
	{
		Viewer_shader.c = 255.0f;

		if(Fade_type == FI_FADEOUT)
			Fade_type = FI_NONE;
	}


	//Set the shader
	gr_set_shader(&Viewer_shader);
	gr_shade(gr_screen.clip_left, gr_screen.clip_top, gr_screen.clip_width, gr_screen.clip_height, false);
}

void game_frame(int paused)
{
	int actually_playing;
	fix total_time1, total_time2;
	fix render2_time1=0, render2_time2=0;
	fix render3_time1=0, render3_time2=0;
	fix flip_time1=0, flip_time2=0;
	fix clear_time1=0, clear_time2=0;
	vec3d eye_pos;
	matrix eye_orient;

#ifndef NDEBUG
	if (Framerate_delay) {
		int	start_time = timer_get_milliseconds();
		while (timer_get_milliseconds() < start_time + Framerate_delay)
			;
	}
#endif

#ifdef DEMO_SYSTEM
	demo_do_frame_start();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif
	
		// start timing frame
		timing_frame_start();
	
		total_time1 = timer_get_fixed_seconds();

	if(paused)
	{
		// Reset the lights here or they just keep on increasing
		light_reset();
	}
	else
	{
		// var to hold which state we are in
		actually_playing = game_actually_playing();
	
		
	#ifndef NO_NETWORK
		if ((!(Game_mode & GM_MULTIPLAYER)) || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER))) {
			if (!(Game_mode & GM_STANDALONE_SERVER)){
				Assert( OBJ_INDEX(Player_obj) >= 0 );
			}
		}
	#else
		Assert( OBJ_INDEX(Player_obj) >= 0 );
	#endif  // ifndef NO_NETWORK
	
		if (Missiontime > Entry_delay_time){
			Pre_player_entry = 0;
		} else {
			; //nprintf(("AI", "Framecount = %i, time = %7.3f\n", Framecount, f2fl(Missiontime)));
		}
	
		//	Note: These are done even before the player enters, else buffers can overflow.
		if (! (Game_mode & GM_STANDALONE_SERVER) && !(Viewer_mode & VM_FREECAMERA)){
			radar_frame_init();
		}
	
		shield_frame_init();
	
		//Do camera stuff
		//This is for the warpout cam
		if ( Player->control_mode != PCM_NORMAL )
			camera_move();

		//Do ingame cutscenes stuff
		if(!Time_compression_locked)
		{
			cameras_do_frame(flFrametime);
		}
		else
		{
			cameras_do_frame(flRealframetime);
		}
	
		if ( !Pre_player_entry && actually_playing ) {		   		
			if (! (Game_mode & GM_STANDALONE_SERVER) ) {
	
				if( (!popup_running_state()) && (!popupdead_is_active()) ){
					game_process_keys();
	
					// don't read flying controls if we're playing a demo back
					if(!(Game_mode & GM_DEMO_PLAYBACK)){
						read_player_controls( Player_obj, flFrametime);
					}
				}
				
	#ifndef NO_NETWORK
				// if we're not the master, we may have to send the server-critical ship status button_info bits
				if ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER)){
					multi_maybe_send_ship_status();
				}
	#endif  // ifndef NO_NETWORK
			}
		}
	
		// Reset the whack stuff
		game_whack_reset();
	
		// These two lines must be outside of Pre_player_entry code,
		// otherwise too many lights are added.
		light_reset();
	
	#ifndef NO_NETWORK
		if ((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
			return;
		}
	#endif  // ifndef NO_NETWORK
		
		game_simulation_frame(); 
		
		// if not actually in a game play state, then return.  This condition could only be true in 
		// a multiplayer game.
		if (!actually_playing ) {
			Assert( Game_mode & GM_MULTIPLAYER );
			return;
		}

	}

	if (!Pre_player_entry) {
		if (! (Game_mode & GM_STANDALONE_SERVER)) {
			clear_time1 = timer_get_fixed_seconds();
			// clear the screen to black
			gr_reset_clip();
			if ( (Game_detail_flags & DETAIL_FLAG_CLEAR) ) {
				gr_clear();
			}

			clear_time2 = timer_get_fixed_seconds();
			render3_time1 = timer_get_fixed_seconds();
			game_render_frame_setup(&eye_pos, &eye_orient);
			game_render_frame( &eye_pos, &eye_orient );

#ifndef NO_NETWORK
			// save the eye position and orientation
			if ( Game_mode & GM_MULTIPLAYER ) {
				Net_player->s_info.eye_pos = eye_pos;
				Net_player->s_info.eye_orient = eye_orient;
			}
#endif  // ifndef NO_NETWORK

			if(!(Viewer_mode & VM_FREECAMERA))
			{
				hud_show_target_model();
			}
			

			// check to see if we should display the death died popup
			if(Game_mode & GM_DEAD_BLEW_UP){				
#ifndef NO_NETWORK
				if(Game_mode & GM_MULTIPLAYER){
					// catch the situation where we're supposed to be warping out on this transition
					if(Net_player->flags & NETINFO_FLAG_WARPING_OUT){
						gameseq_post_event(GS_EVENT_DEBRIEF);
					} else if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
						Player_died_popup_wait = -1;
						popupdead_start();
					}
				} else {
#endif  // ifndef NO_NETWORK

					if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
						Player_died_popup_wait = -1;
						popupdead_start();
					}

#ifndef NO_NETWORK
				}
#endif
			}

			// hack - sometimes this seems to slip by in multiplayer. this should guarantee that we catch it
			if((Game_mode & GM_MULTIPLAYER) && (Player_multi_died_check != -1) && (Game_mode & GM_DEAD_BLEW_UP) ){
				if(fl_abs(time(NULL) - Player_multi_died_check) > 4.0f){
					if(!popupdead_is_active()){
						popupdead_start();
					}

					Player_multi_died_check = -1;
				}
			}
	
			render3_time2 = timer_get_fixed_seconds();
			render2_time1 = timer_get_fixed_seconds();

			gr_reset_clip();
			game_get_framerate();
			game_show_framerate();
			game_show_eye_pos(&eye_pos, &eye_orient);

			game_show_time_left();

#ifdef NEW_HUD
			//Display all new hud gauges. Note this is somewhat inefficient;
			//set_current_hud really only needs to be called when the viewer
			//object changes, however, it works fine this way.
			if(Viewer_obj && Viewer_obj->type == OBJ_SHIP)
			{
				set_current_hud(&Ships[Viewer_obj->instance]);
				Ships[Viewer_obj->instance].ship_hud.show();
			}
#endif

			// Draw the 2D HUD gauges
			if(!(Viewer_mode & VM_FREECAMERA))
			{
				if(supernova_active() <	3){
					game_render_hud_2d();
				}

				// Draw 3D HUD gauges			
				game_render_hud_3d(&eye_pos, &eye_orient);
			}

			gr_reset_clip();
			if(!Time_compression_locked)
			{
				subtitles_do_frame(flFrametime);
			}
			else
			{
				subtitles_do_frame(flRealframetime);
			}
			
			if(!Time_compression_locked)
			{
				game_set_view_clip(flFrametime);
				game_shade_frame(flFrametime);
			}
			else
			{
				game_set_view_clip(flRealframetime);
				game_shade_frame(flRealframetime);
			}

			game_tst_frame();

			render2_time2 = timer_get_fixed_seconds();

			// maybe render and process the dead popup
			game_maybe_do_dead_popup(flFrametime);

			// start timing frame
			timing_frame_stop();
			// timing_display(30, 10);			

			// If a regular popup is active, don't flip (popup code flips)
			if( !popup_running_state() ){
				flip_time1 = timer_get_fixed_seconds();
				game_flip_page_and_time_it();
				flip_time2 = timer_get_fixed_seconds();
			}

#ifndef NDEBUG
			game_maybe_dump_frame();			// used to dump pcx files for building trailers
#endif		
		} else {

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
			game_show_standalone_framerate();
#endif
		}
	}

	game_do_training_checks();
	asteroid_frame();

	// process lightning (nebula only)
	nebl_process();

	total_time2 = timer_get_fixed_seconds();
	
	// Got some timing numbers
	Timing_total = f2fl( total_time2 - total_time1 ) * 1000.0f;
	Timing_clear = f2fl( clear_time2 - clear_time1 ) * 1000.0f;
	Timing_render2 = f2fl( render2_time2- render2_time1 ) * 1000.0f;
	Timing_render3 = f2fl( render3_time2- render3_time1 ) * 1000.0f;
	Timing_flip = f2fl( flip_time2 - flip_time1 ) * 1000.0f;

#ifdef DEMO_SYSTEM
	demo_do_frame_end();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif
}

#define	MAX_FRAMETIME	(F1_0/4)		// Frametime gets saturated at this.  Changed by MK on 11/1/97.
												//	Some bug was causing Frametime to always get saturated at 2.0 seconds after the player
												//	died.  This resulted in screwed up death sequences.

fix Last_time = 0;						// The absolute time of game at end of last frame (beginning of this frame)
fix Last_delta_time = 0;				// While game is paused, this keeps track of how much elapsed in the frame before paused.
static int timer_paused=0;
#if defined(TIMER_TEST) && !defined(NDEBUG)
static int stop_count,start_count;
static int time_stopped,time_started;
#endif
int saved_timestamp_ticker = -1;

void game_reset_time()
{
#ifndef NO_NETWORK
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
		return ;
	}
#endif
	
	//	Last_time = timer_get_fixed_seconds();
	game_start_time();
	timestamp_reset();
	game_stop_time();
}

void game_stop_time()
{
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		// Save how much time progressed so far in the frame so we can
		// use it when we unpause.
		Last_delta_time = time - Last_time;		

		//mprintf(("Last_time in game_stop_time = %7.3f\n", f2fl(Last_delta_time)));
		if (Last_delta_time < 0) {
			#if defined(TIMER_TEST) && !defined(NDEBUG)
			Int3();		//get Matt!!!!
			#endif
			Last_delta_time = 0;
		}
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_stopped = time;
		#endif

		// Stop the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		saved_timestamp_ticker = timestamp_ticker;
	}
	timer_paused++;

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	stop_count++;
	#endif
}

void game_start_time()
{
	timer_paused--;
	Assert(timer_paused >= 0);
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		if (Last_time < 0)
			Int3();		//get Matt!!!!
		}
		#endif
		// Take current time, and set it backwards to account for time	
		// that the frame already executed, so that timer_get_fixed_seconds() - Last_time
		// will be correct when it goes to calculate the frametime next
		// frame.
		Last_time = time - Last_delta_time;		
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_started = time;
		#endif

		// Restore the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		Assert( saved_timestamp_ticker > -1 );		// Called out of order, get JAS
		timestamp_ticker = saved_timestamp_ticker;
		saved_timestamp_ticker = -1;
	}

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	start_count++;
	#endif
}

void lock_time_compression(bool is_locked)
{
	Time_compression_locked = is_locked;
}

void change_time_compression(float multiplier)
{
	fix modified = F1_0;

	modified = fl2f( f2fl(Game_time_compression) * multiplier );

	Desired_time_compression = Game_time_compression = modified;
	Time_compression_change_rate = 0;
}

void set_time_compression(float multiplier, float change_time)
{
	if(change_time <= 0.0f)
	{
		Game_time_compression = Desired_time_compression = fl2f(multiplier);
		Time_compression_change_rate = 0;
		return;
	}

	Desired_time_compression = fl2f(multiplier);
	Time_compression_change_rate = fl2f( f2fl(Desired_time_compression - Game_time_compression) / change_time );
}

void game_set_frametime(int state)
{
	fix thistime;
	float frame_cap_diff;

	thistime = timer_get_fixed_seconds();

	if ( Last_time == 0 )	
		Frametime = F1_0 / 30;
	else
		Frametime = thistime - Last_time;

//	Frametime = F1_0 / 30;

#ifndef NDEBUG
	fix	debug_frametime = Frametime;	//	Just used to display frametime.
#endif

	//	If player hasn't entered mission yet, make frame take 1/4 second.
	if ((Pre_player_entry) && (state == GS_STATE_GAME_PLAY))
		Frametime = F1_0/4;
#ifndef NDEBUG
	else if ((Debug_dump_frames) && (state == GS_STATE_GAME_PLAY)) {				// note link to above if!!!!!
	
		fix frame_speed = F1_0 / Debug_dump_frames;

		if (Frametime > frame_speed ){
			nprintf(("warning","slow frame: %x\n",Frametime));
		} else {			
			do {
				thistime = timer_get_fixed_seconds();
				Frametime = thistime - Last_time;
			} while (Frametime < frame_speed );			
		}
		Frametime = frame_speed;
	}
#endif

	Assert( Framerate_cap > 0 );

	// Cap the framerate so it doesn't get too high.
	if (!Cmdline_NoFPSCap)
	{
		fix cap;

		cap = F1_0/Framerate_cap;
		if (Frametime < cap) {
			thistime = cap - Frametime;
//  			mprintf(("Sleeping for %6.3f seconds.\n", f2fl(thistime)));
			Sleep( DWORD(f2fl(thistime) * 1000.0f) );
			Frametime = cap;
			thistime = timer_get_fixed_seconds();
		}
	}

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
	if((Game_mode & GM_STANDALONE_SERVER) && 
		(f2fl(Frametime) < ((float)1.0/(float)Multi_options_g.std_framecap))){

		frame_cap_diff = ((float)1.0/(float)Multi_options_g.std_framecap) - f2fl(Frametime);		
		Sleep((DWORD)(frame_cap_diff*1000)); 				
		
		thistime += fl2f((frame_cap_diff));		

		Frametime = thistime - Last_time;
   }
#endif

	// If framerate is too low, cap it.
	if (Frametime > MAX_FRAMETIME)	{
#ifndef NDEBUG
		mprintf(("Frame %2i too long!!: frametime = %.3f (%.3f)\n", Framecount, f2fl(Frametime), f2fl(debug_frametime)));
#endif
		Frametime = MAX_FRAMETIME;
	}

	flRealframetime = f2fl(Frametime);

	//Handle changes in time compression
	if(Game_time_compression != Desired_time_compression)
	{
		bool ascending = Desired_time_compression > Game_time_compression;
		if(Time_compression_change_rate)
			Game_time_compression += fixmul(Time_compression_change_rate, Frametime);
		if((ascending && Game_time_compression > Desired_time_compression)
			|| (!ascending && Game_time_compression < Desired_time_compression))
			Game_time_compression = Desired_time_compression;
	}

	Frametime = fixmul(Frametime, Game_time_compression);

	Last_time = thistime;
	//mprintf(("Frame %i, Last_time = %7.3f\n", Framecount, f2fl(Last_time)));

	flFrametime = f2fl(Frametime);
	//if(!(Game_mode & GM_PLAYING_DEMO)){
	timestamp_inc(flFrametime);

/*	if ((Framecount > 0) && (Framecount < 10)) {
		mprintf(("Frame %2i: frametime = %.3f (%.3f)\n", Framecount, f2fl(Frametime), f2fl(debug_frametime)));
	}
*/
}

// This is called from game_do_frame(), and from navmap_do_frame() 
void game_update_missiontime()
{
	// TODO JAS: Put in if and move this into game_set_frametime, 
	// fix navmap to call game_stop/start_time
	//if ( !timer_paused )	
		Missiontime += Frametime;
}

void game_do_frame()
{	
	game_set_frametime(GS_STATE_GAME_PLAY);
	game_update_missiontime();

//	if (Player_ship->flags & SF_DYING)
//		flFrametime /= 15.0;

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_multi_set_standalone_missiontime(f2fl(Missiontime));
	}
#endif

	if ( game_single_step && (last_single_step == game_single_step) ) {
		os_set_title( NOX("SINGLE STEP MODE (Pause exits, any other key steps)") );
		while( key_checkch() == 0 )
			os_sleep(10);
		os_set_title( XSTR( "FreeSpace", 171) );
  		Last_time = timer_get_fixed_seconds();
	}

	last_single_step = game_single_step;

	if ((gameseq_get_state() == GS_STATE_GAME_PLAY) && Use_mouse_to_fly){
		Keep_mouse_centered = 1;  // force mouse to center of our window (so we don't hit movement limits)
	}
	game_frame();

	Keep_mouse_centered = 0;
	monitor_update();			// Update monitor variables
}

#ifndef NO_NETWORK
void multi_maybe_do_frame()
{
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) && !Multi_pause_status){
		game_do_frame(); 
	}
}
#endif  // ifndef NO_NETWORK

int Joymouse_button_status = 0;

// Flush all input devices
void game_flush()
{
	key_flush();
	mouse_flush();
	joy_flush();
	snazzy_flush();

	Joymouse_button_status = 0;

	//mprintf(("Game flush!\n" ));
}

// function for multiplayer only which calls game_do_state_common() when running the
// debug console
void game_do_dc_networking()
{
	Assert( Game_mode & GM_MULTIPLAYER );

	game_do_state_common( gameseq_get_state() );
}

// Call this whenever in a loop, or when you need to check for a keystroke.
int game_check_key()
{
	int k;

	k = game_poll();

	// convert keypad enter to normal enter
	if ((k & KEY_MASK) == KEY_PADENTER)
		k = (k & ~KEY_MASK) | KEY_ENTER;

	return k;
}

#ifdef FS2_DEMO

#define DEMO_TRAILER_TIMEOUT_MS		45000			// 45 seconds of no input, play trailer
static int Demo_show_trailer_timestamp = 0;

void demo_reset_trailer_timer()
{
	Demo_show_trailer_timestamp = timer_get_milliseconds();
}

void demo_maybe_show_trailer(int k)
{
	/*
	// if key pressed, reset demo trailer timer
	if ( k > 0 ) {
		demo_reset_trailer_timer();
		return;
	}

	// if mouse moved, reset demo trailer timer
	int dx = 0, dy = 0;

	mouse_get_delta(&dx, &dy);
	if ( (dx > 0) || (dy > 0) ) {
		demo_reset_trailer_timer();
		return;
	}

	// if joystick has moved, reset demo trailer timer
	dx = 0;
	dy = 0;
	joy_get_delta(&dx, &dy);
	if ( (dx > 0) || (dy > 0) ) {
		demo_reset_trailer_timer();
		return;
	}

	// NOTE: reseting the trailer timer on mouse/joystick presses is handled in
	//       the low-level code.  Ugly, I know... but was the simplest and most
	//       robust solution.
		
	// if 30 seconds since last demo trailer time reset, launch movie
	if ( os_foreground() ) {
		int now = timer_get_milliseconds();
		if ( (now - Demo_show_trailer_timestamp) > DEMO_TRAILER_TIMEOUT_MS ) {
//		if ( (now - Demo_show_trailer_timestamp) > 10000 ) {
			// play movie here
			movie_play( NOX("fstrailer2.mve") );
			demo_reset_trailer_timer();
		}
	}
	*/
}

#endif

// same as game_check_key(), except this is used while actually in the game.  Since there
// generally are differences between game control keys and general UI keys, makes sense to
// have seperate functions for each case.  If you are not checking a game control while in a
// mission, you should probably be using game_check_key() instead.
int game_poll()
{
	int k, state;

	if (!os_foreground()) {		
		game_stop_time();
//		os_sleep(100);
		os_sleep(1);
		game_start_time();

		// If we're in a single player game, pause it.
		if (!(Game_mode & GM_MULTIPLAYER)){
			if ( (gameseq_get_state() == GS_STATE_GAME_PLAY) && (!popup_active()) && (!popupdead_is_active()) )	{
				game_process_pause_key();
			}
		}
	}

   k = key_inkey();
	//if (k)
	//	mprintf(("game_poll(): got key 0x%02x\n", k));

#ifdef FS2_DEMO
	demo_maybe_show_trailer(k);
#endif

	// Move the mouse cursor with the joystick.
	if (os_foreground() && (!Mouse_hidden) && (Use_joy_mouse) )	{
		// Move the mouse cursor with the joystick
		int mx, my, dx, dy;
		int jx, jy, jz, jr;

		joy_get_pos( &jx, &jy, &jz, &jr );

		dx = fl2i(f2fl(jx)*flFrametime*500.0f);
		dy = fl2i(f2fl(jy)*flFrametime*500.0f);

		if ( dx || dy )	{
			mouse_get_real_pos( &mx, &my );
			mouse_set_pos( mx+dx, my+dy );
		}

		int j, m;
		j = joy_down(0);
		m = mouse_down(MOUSE_LEFT_BUTTON);

		if ( j != Joymouse_button_status )	{
			//mprintf(( "Joy went from %d to %d, mouse is %d\n", Joymouse_button_status, j, m ));
			Joymouse_button_status = j;
			if ( j && (!m) )	{
				mouse_mark_button( MOUSE_LEFT_BUTTON, 1 );
			} else if ( (!j) && (m) )	{
				mouse_mark_button( MOUSE_LEFT_BUTTON, 0 );
			}
		}
	}

#ifndef NO_NETWORK
	// if we should be ignoring keys because of some multiplayer situations
	if((Game_mode & GM_MULTIPLAYER) && multi_ignore_controls(k)){
		return 0;
	}
#endif  // ifndef NO_NETWORK

	// If a popup is running, don't process all the Fn keys
	if( popup_active() ) {
		return k;
	}

	state = gameseq_get_state();

//	if ( k ) nprintf(( "General", "Key = %x\n", k ));

	switch (k) {
		case KEY_DEBUGGED + KEY_BACKSP:
			if(!(Game_mode & GM_MULTIPLAYER))
			{
				gameseq_post_event(GS_EVENT_LAB);
				k = 0;
			}
			break;

		case KEY_F1:
			launch_context_help();
			k = 0;
			break;

		case KEY_F2:
//			if (state != GS_STATE_INITIAL_PLAYER_SELECT) {

			// don't allow f2 while warping out in multiplayer	
#ifndef NO_NETWORK
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_WARPING_OUT)){
				break;
			}
#endif

			switch (state) {
				case GS_STATE_INITIAL_PLAYER_SELECT:
				case GS_STATE_OPTIONS_MENU:
				case GS_STATE_HUD_CONFIG:
				case GS_STATE_CONTROL_CONFIG:
				case GS_STATE_DEATH_DIED:
				case GS_STATE_DEATH_BLEW_UP:		
				case GS_STATE_VIEW_MEDALS:
					break;

				default:
					gameseq_post_event(GS_EVENT_OPTIONS_MENU);
					k = 0;
					break;
			}

			break;

			// hotkey selection screen -- only valid from briefing and beyond.
		case KEY_F3:	
			#ifndef FS2_DEMO
				if ( (state == GS_STATE_TEAM_SELECT) || (state == GS_STATE_BRIEFING) || (state == GS_STATE_SHIP_SELECT) || (state == GS_STATE_WEAPON_SELECT) || (state == GS_STATE_GAME_PLAY) || (state == GS_STATE_GAME_PAUSED) ) {
					gameseq_post_event( GS_EVENT_HOTKEY_SCREEN );
					k = 0;
				}
			#endif
			break;

		case KEY_DEBUGGED + KEY_F3:
			gameseq_post_event( GS_EVENT_TOGGLE_FULLSCREEN );
			break;

		case KEY_F4:
			if(Game_mode & GM_MULTIPLAYER){
				if((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_MULTI_PAUSED)){
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				} 
			} else {
				if ((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_DEATH_DIED) || (state == GS_STATE_DEATH_BLEW_UP) || (state == GS_STATE_GAME_PAUSED) ) {
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				}
			}
			break;

		case KEY_ESC | KEY_SHIFTED:
			// make sure to quit properly out of multiplayer
#ifndef NO_NETWORK
			if(Game_mode & GM_MULTIPLAYER){
				multi_quit_game(PROMPT_NONE);
			}
#endif
			gameseq_post_event( GS_EVENT_QUIT_GAME );
			k = 0;

			break;

		case KEY_DEBUGGED + KEY_P:			
			break;			

		case KEY_PRINT_SCRN: 
			{
				static int counter = os_config_read_uint(NULL, "ScreenshotNum", 0);
				char tmp_name[MAX_FILENAME_LEN];

				game_stop_time();

				// we could probably go with .3 here for 1,000 shots but people really need to clean out
				// their directories better than that so it's 100 for now.
				sprintf( tmp_name, NOX("screen%.4i"), counter );
				counter++;

				// we've got two character precision so we can only have 100 shots at a time, reset if needed
				//Now we have four digit precision :) -WMC
				if (counter > 9999)
				{
					//This should pop up a dialogue or something ingame.
					Warning(LOCATION, "Screenshot count has reached max of 9999. Resetting to 0.");
					counter = 0;
				}

				mprintf(( "Dumping screen to '%s'\n", tmp_name ));
				gr_print_screen(tmp_name);

				game_start_time();
				os_config_write_uint(NULL, "ScreenshotNum", counter);
			}

			k = 0;
			break;

		case KEY_SHIFTED | KEY_ENTER: {

#if !defined(NDEBUG)

			if ( Game_mode & GM_NORMAL ){
				game_stop_time();
			}

			// if we're in multiplayer mode, do some special networking
			if(Game_mode & GM_MULTIPLAYER){
				debug_console(game_do_dc_networking);
			} else {				
				debug_console();
			}

			game_flush();

			if ( Game_mode & GM_NORMAL )
				game_start_time();

#endif

			break;
		}
	}

	return k;
}

void os_close()
{
	gameseq_post_event(GS_EVENT_QUIT_GAME);
}

void apply_physics( float damping, float desired_vel, float initial_vel, float t, float * new_vel, float * delta_pos );


void camera_set_position( vec3d *pos )
{
	Camera_pos = *pos;
}

void camera_set_orient( matrix *orient )
{
	Camera_orient = *orient;
}

void camera_set_velocity( vec3d *vel, int instantaneous )
{
	Camera_desired_velocity.xyz.x = 0.0f;
	Camera_desired_velocity.xyz.y = 0.0f;
	Camera_desired_velocity.xyz.z = 0.0f;

	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.vec.rvec, vel->xyz.x );
	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.vec.uvec, vel->xyz.y );
	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.vec.fvec, vel->xyz.z );

	if ( instantaneous )	{
		Camera_velocity = Camera_desired_velocity;
	}

}

//
void camera_move()
{
	vec3d new_vel, delta_pos;

	apply_physics( Camera_damping, Camera_desired_velocity.xyz.x, Camera_velocity.xyz.x, flFrametime, &new_vel.xyz.x, &delta_pos.xyz.x );
	apply_physics( Camera_damping, Camera_desired_velocity.xyz.y, Camera_velocity.xyz.y, flFrametime, &new_vel.xyz.y, &delta_pos.xyz.y );
	apply_physics( Camera_damping, Camera_desired_velocity.xyz.z, Camera_velocity.xyz.z, flFrametime, &new_vel.xyz.z, &delta_pos.xyz.z );

	Camera_velocity = new_vel;

//	mprintf(( "Camera velocity = %.1f,%.1f, %.1f\n", Camera_velocity.xyz.x, Camera_velocity.xyz.y, Camera_velocity.xyz.z ));

	vm_vec_add2( &Camera_pos, &delta_pos );

	float ot = Camera_time+0.0f;

	Camera_time += flFrametime;

	if ( (ot < 0.667f) && ( Camera_time >= 0.667f ) )	{
		vec3d tmp;
		
		tmp.xyz.z = 4.739f;		// always go this fast forward.

		// pick x and y velocities so they are always on a 
		// circle with a 25 m radius.

		float tmp_angle = frand()*PI2;
	
		tmp.xyz.x = 22.0f * (float)sin(tmp_angle);
		tmp.xyz.y = -22.0f * (float)cos(tmp_angle);

		//mprintf(( "Angle = %.1f, vx=%.1f, vy=%.1f\n", tmp_angle, tmp.xyz.x, tmp.xyz.y ));

		//mprintf(( "Changing velocity!\n" ));
		camera_set_velocity( &tmp, 0 );
	}

	if ( (ot < 3.0f ) && ( Camera_time >= 3.0f ) )	{
		vec3d tmp = { 0.0f, 0.0f, 0.0f };
		camera_set_velocity( &tmp, 0 );
	}
	
}

void end_demo_campaign_do()
{
#if defined(FS2_DEMO)
	// show upsell screens
	demo_upsell_show_screens();
#elif defined(OEM_BUILD)
	// show oem upsell screens
	oem_upsell_show_screens();
#endif

	// drop into main hall
	gameseq_post_event( GS_EVENT_MAIN_MENU );
}

// All code to process events.   This is the only place
// that you should change the state of the game.
void game_process_event( int current_state, int event )
{
	mprintf(("Got event %s (%d) in state %s (%d)\n", GS_event_text[event], event, GS_state_text[current_state], current_state));

	switch (event) {
		case GS_EVENT_SIMULATOR_ROOM:
			gameseq_set_state(GS_STATE_SIMULATOR_ROOM);
			break;

		case GS_EVENT_MAIN_MENU:
			gameseq_set_state(GS_STATE_MAIN_MENU);		
			break;

		case GS_EVENT_OPTIONS_MENU:
			gameseq_push_state( GS_STATE_OPTIONS_MENU );
			break;

		case GS_EVENT_BARRACKS_MENU:
			gameseq_set_state(GS_STATE_BARRACKS_MENU);		
			break;

		case GS_EVENT_TECH_MENU:
			gameseq_set_state(GS_STATE_TECH_MENU);		
			break;
		case GS_EVENT_LAB:
			gameseq_push_state(GS_STATE_LAB);
			break;
		case GS_EVENT_TRAINING_MENU:
			gameseq_set_state(GS_STATE_TRAINING_MENU);		
			break;

		case GS_EVENT_START_GAME:
			Select_default_ship = 0;			
			Player_multi_died_check = -1;
			gameseq_set_state(GS_STATE_CMD_BRIEF);
			break;

		case GS_EVENT_START_BRIEFING:
			gameseq_set_state(GS_STATE_BRIEFING);		
			break;

		case GS_EVENT_DEBRIEF:
			// did we end the campaign in the main freespace 2 single player campaign?
			if(Campaign_ended_in_mission && (Game_mode & GM_CAMPAIGN_MODE) && !stricmp(Campaign.filename, "freespace2")) {
				gameseq_post_event(GS_EVENT_END_CAMPAIGN);
			} else {
				gameseq_set_state(GS_STATE_DEBRIEF);		
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_SHIP_SELECTION:
			gameseq_set_state( GS_STATE_SHIP_SELECT );
			break;

		case GS_EVENT_WEAPON_SELECTION:
			gameseq_set_state( GS_STATE_WEAPON_SELECT );
			break;

		case GS_EVENT_ENTER_GAME:		
#ifdef DEMO_SYSTEM
			// maybe start recording a demo
			if(Demo_make){
				demo_start_record("test.fsd");
			}
#endif

#ifndef NO_NETWORK
			if (Game_mode & GM_MULTIPLAYER) {
				// if we're respawning, make sure we change the view mode so that the hud shows up
				if (current_state == GS_STATE_DEATH_BLEW_UP) {
					Viewer_mode = 0;
				}

				gameseq_set_state(GS_STATE_GAME_PLAY);
			} else {
				gameseq_set_state(GS_STATE_GAME_PLAY, 1);
			}

			Player_multi_died_check = -1;

			// clear multiplayer button info			
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));
#else
				gameseq_set_state(GS_STATE_GAME_PLAY, 1);
#endif

			Start_time = f2fl(timer_get_approx_seconds());
			//Framecount = 0;
			mprintf(("Entering game at time = %7.3f\n", Start_time));
			break;


		case GS_EVENT_START_GAME_QUICK:
			Select_default_ship = 1;
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			break;


		case GS_EVENT_END_GAME:
			if ( (current_state == GS_STATE_GAME_PLAY) || (current_state == GS_STATE_DEATH_DIED) ||
				(current_state == GS_STATE_DEATH_BLEW_UP) ||	(current_state == GS_STATE_DEBRIEF) || (current_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) {
					gameseq_set_state(GS_STATE_MAIN_MENU);

			} else
				Int3();

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_QUIT_GAME:
			main_hall_stop_music();
			main_hall_stop_ambient();
			gameseq_set_state(GS_STATE_QUIT_GAME);

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_GAMEPLAY_HELP:
			gameseq_push_state( GS_STATE_GAMEPLAY_HELP );
			break;

		case GS_EVENT_PAUSE_GAME:
			gameseq_push_state(GS_STATE_GAME_PAUSED);
			break;

		case GS_EVENT_DEBUG_PAUSE_GAME:
			gameseq_push_state(GS_STATE_DEBUG_PAUSED);
			break;

		case GS_EVENT_TRAINING_PAUSE:
			gameseq_push_state(GS_STATE_TRAINING_PAUSED);
			break;

		case GS_EVENT_PREVIOUS_STATE:
			gameseq_pop_state();
			break;

		case GS_EVENT_LOAD_MISSION_MENU:
			gameseq_set_state(GS_STATE_LOAD_MISSION_MENU);
			break;

		case GS_EVENT_MISSION_LOG_SCROLLBACK:
			gameseq_push_state( GS_STATE_MISSION_LOG_SCROLLBACK );
			break;

		case GS_EVENT_HUD_CONFIG:
			gameseq_push_state( GS_STATE_HUD_CONFIG );
			break;

		case GS_EVENT_CONTROL_CONFIG:
			gameseq_push_state( GS_STATE_CONTROL_CONFIG );
			break;	

		case GS_EVENT_DEATH_DIED:
			gameseq_set_state( GS_STATE_DEATH_DIED );
			break;

		case GS_EVENT_DEATH_BLEW_UP:
			if (  current_state == GS_STATE_DEATH_DIED )	{
				gameseq_set_state( GS_STATE_DEATH_BLEW_UP );
				event_music_player_death();

				// multiplayer clients set their extra check here
				if(Game_mode & GM_MULTIPLAYER){
					// set the multi died absolute last chance check					
					Player_multi_died_check = time(NULL);
				}					
			} else {
				mprintf(( "Ignoring GS_EVENT_DEATH_BLEW_UP because we're in state %d\n", current_state ));
			}
			break;

		case GS_EVENT_NEW_CAMPAIGN:
			if (!mission_load_up_campaign()){
				readyroom_continue_campaign();
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_CAMPAIGN_CHEAT:
			if (!mission_load_up_campaign()){
				/*
				// bash campaign value
				extern char Main_hall_campaign_cheat[512];
				int idx;
				
				// look for the mission
				for(idx=0; idx<Campaign.num_missions; idx++){
					if(!stricmp(Campaign.missions[idx].name, Main_hall_campaign_cheat)){
						Campaign.next_mission = idx;
						Campaign.prev_mission = idx - 1;
						break;
					}
				}
				*/

				// continue
				readyroom_continue_campaign();
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_CAMPAIGN_ROOM:
			gameseq_set_state(GS_STATE_CAMPAIGN_ROOM);
			break;

		case GS_EVENT_CMD_BRIEF:
			gameseq_set_state(GS_STATE_CMD_BRIEF);
			break;

		case GS_EVENT_RED_ALERT:
			gameseq_set_state(GS_STATE_RED_ALERT);
			break;

		case GS_EVENT_STORYBOOK:
			gameseq_set_state(GS_STATE_STORYBOOK);
			break;

		case GS_EVENT_CREDITS:
			gameseq_set_state( GS_STATE_CREDITS );
			break;

		case GS_EVENT_VIEW_MEDALS:
			gameseq_push_state( GS_STATE_VIEW_MEDALS );
			break;

		case GS_EVENT_SHOW_GOALS:
			gameseq_push_state( GS_STATE_SHOW_GOALS );	// use push_state() since we might get to this screen through a variety of states
			break;

		case GS_EVENT_HOTKEY_SCREEN:
			gameseq_push_state( GS_STATE_HOTKEY_SCREEN );	// use push_state() since we might get to this screen through a variety of states
			break;
		
	// multiplayer stuff follow these comments
		case GS_EVENT_NET_CHAT:
			gameseq_set_state( GS_STATE_NET_CHAT );
			break;

		case GS_EVENT_MULTI_JOIN_GAME:
			gameseq_set_state( GS_STATE_MULTI_JOIN_GAME );
			break;

		case GS_EVENT_MULTI_HOST_SETUP:
			gameseq_set_state( GS_STATE_MULTI_HOST_SETUP );
			break;

		case GS_EVENT_MULTI_CLIENT_SETUP:
			gameseq_set_state( GS_STATE_MULTI_CLIENT_SETUP );
			break;

  		case GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN:
			gameseq_set_state(GS_STATE_VIEW_CUTSCENES);
			break;

		case GS_EVENT_MULTI_STD_WAIT:
			gameseq_set_state( GS_STATE_MULTI_STD_WAIT );
			break;

		case GS_EVENT_STANDALONE_MAIN:
			gameseq_set_state( GS_STATE_STANDALONE_MAIN );
			break;   

		case GS_EVENT_MULTI_PAUSE:
			gameseq_push_state( GS_STATE_MULTI_PAUSED );
			break;			

		case GS_EVENT_INGAME_PRE_JOIN:
			gameseq_set_state( GS_STATE_INGAME_PRE_JOIN );
			break;
		
		case GS_EVENT_EVENT_DEBUG:
			gameseq_push_state(GS_STATE_EVENT_DEBUG);
			break;

		// Start a warpout where player automatically goes 70 no matter what
		// and can't cancel out of it.
		case GS_EVENT_PLAYER_WARPOUT_START_FORCED:
			Warpout_forced = 1;							// If non-zero, bash the player to speed and go through effect

			// Same code as in GS_EVENT_PLAYER_WARPOUT_START only ignores current mode
			Player->saved_viewer_mode = Viewer_mode;
			Player->control_mode = PCM_WARPOUT_STAGE1;
			Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
			Warpout_time = 0.0f;			// Start timer!
			break;

		case GS_EVENT_PLAYER_WARPOUT_START:
			if ( Player->control_mode != PCM_NORMAL )	{
				mprintf(( "Player isn't in normal mode; cannot warp out.\n" ));
			} else {
				Player->saved_viewer_mode = Viewer_mode;
				Player->control_mode = PCM_WARPOUT_STAGE1;
				Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
				Warpout_time = 0.0f;			// Start timer!
				Warpout_forced = 0;				// If non-zero, bash the player to speed and go through effect
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_STOP:
			if ( Player->control_mode != PCM_NORMAL )	{
				if ( !Warpout_forced )	{		// cannot cancel forced warpout
					Player->control_mode = PCM_NORMAL;
					Viewer_mode = Player->saved_viewer_mode;
					hud_subspace_notify_abort();
					mprintf(( "Player put back to normal mode.\n" ));
					if ( Warpout_sound > -1 )	{
						snd_stop( Warpout_sound );
						Warpout_sound = -1;
					}
				}
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1:		// player ship got up to speed
			if ( Player->control_mode != PCM_WARPOUT_STAGE1 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage1.\n" ));
			} else {
				mprintf(( "Hit target speed.  Starting warp effect and moving to stage 2!\n" ));
				shipfx_warpout_start( Player_obj );
				Player->control_mode = PCM_WARPOUT_STAGE2;
				Player->saved_viewer_mode = Viewer_mode;
				Viewer_mode |= VM_WARP_CHASE;
				
				vec3d tmp = Player_obj->pos;
				matrix tmp_m;
				ship_get_eye( &tmp, &tmp_m, Player_obj );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.vec.rvec, 0.0f );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.vec.uvec, 0.952f );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.vec.fvec, -1.782f );
				Camera_time = 0.0f;
				camera_set_position( &tmp );
				camera_set_orient( &Player_obj->orient );
				vec3d tmp_vel = { 0.0f, 5.1919f, 14.7f };

				//mprintf(( "Rad = %.1f\n", Player_obj->radius ));
				camera_set_velocity( &tmp_vel, 1);
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2:		// player ship got into the warp effect
			if ( Player->control_mode != PCM_WARPOUT_STAGE2 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage2.\n" ));
			} else {
				mprintf(( "Hit warp effect.  Moving to stage 3!\n" ));
				Player->control_mode = PCM_WARPOUT_STAGE3;
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE:	// player ship got through the warp effect
			mprintf(( "Player warped out.  Going to debriefing!\n" ));
			Player->control_mode = PCM_NORMAL;
			Viewer_mode = Player->saved_viewer_mode;
			Warpout_sound = -1;

			// we have a special debriefing screen for multiplayer furballs
			if((Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT)){
				gameseq_post_event(GS_EVENT_MULTI_DOGFIGHT_DEBRIEF);
			}
			// do the normal debriefing for all other situations
			else {
				gameseq_post_event(GS_EVENT_DEBRIEF);
			}
			break;

		case GS_EVENT_STANDALONE_POSTGAME:
			gameseq_set_state(GS_STATE_STANDALONE_POSTGAME);
			break;

		case GS_EVENT_INITIAL_PLAYER_SELECT:
			gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			break;

		case GS_EVENT_GAME_INIT:
#ifdef OEM_BUILD
			gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
#else			
			// see if the command line option has been set to use the last pilot, and act acoordingly
			if( player_select_get_last_pilot() ) {	
				// always enter the main menu -- do the automatic network startup stuff elsewhere
				// so that we still have valid checks for networking modes, etc.
				gameseq_set_state(GS_STATE_MAIN_MENU);
			} else {
				gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			}
#endif
			break;

		case GS_EVENT_MULTI_MISSION_SYNC:
			gameseq_set_state(GS_STATE_MULTI_MISSION_SYNC);
			break;		

		case GS_EVENT_MULTI_START_GAME:
			gameseq_set_state(GS_STATE_MULTI_START_GAME);
			break;

		case GS_EVENT_MULTI_HOST_OPTIONS:
			gameseq_set_state(GS_STATE_MULTI_HOST_OPTIONS);
			break;

		case GS_EVENT_MULTI_DOGFIGHT_DEBRIEF:
			gameseq_set_state(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			break;

		case GS_EVENT_TEAM_SELECT:
			gameseq_set_state(GS_STATE_TEAM_SELECT);
			break;

		case GS_EVENT_END_CAMPAIGN:			
			gameseq_set_state(GS_STATE_END_OF_CAMPAIGN);
			break;		

		case GS_EVENT_END_DEMO:
			gameseq_set_state(GS_STATE_END_DEMO);
			break;

		case GS_EVENT_LOOP_BRIEF:
			gameseq_set_state(GS_STATE_LOOP_BRIEF);
			break;

		default:
			Int3();
			break;
	}
}

// Called when a state is being left.
// The current state is still at old_state, but as soon as
// this function leaves, then the current state will become
// new state.     You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
void game_leave_state( int old_state, int new_state )
{
	int end_mission = 1;

	switch (new_state) {
		case GS_STATE_GAME_PAUSED:
		case GS_STATE_DEBUG_PAUSED:
		case GS_STATE_OPTIONS_MENU:
		case GS_STATE_CONTROL_CONFIG:		
		case GS_STATE_MISSION_LOG_SCROLLBACK:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_SHOW_GOALS:
		case GS_STATE_HOTKEY_SCREEN:		
		case GS_STATE_MULTI_PAUSED:
		case GS_STATE_TRAINING_PAUSED:
		case GS_STATE_EVENT_DEBUG:				
		case GS_STATE_GAMEPLAY_HELP:
		case GS_STATE_LAB:
			end_mission = 0;  // these events shouldn't end a mission
			break;
	}

	switch (old_state) {
		case GS_STATE_BRIEFING:
			brief_stop_voices();
			if ( (new_state != GS_STATE_OPTIONS_MENU) && (new_state != GS_STATE_WEAPON_SELECT)
				  && (new_state != GS_STATE_SHIP_SELECT) && (new_state != GS_STATE_HOTKEY_SCREEN)
				  && (new_state != GS_STATE_TEAM_SELECT) ){
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			
#ifndef NO_NETWORK
			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
				cfclose(Multi_chat_stream);
			}
#endif  // ifndef NO_NETWORK
			break;

		case GS_STATE_DEBRIEF:
			if ( (new_state != GS_STATE_VIEW_MEDALS) && (new_state != GS_STATE_OPTIONS_MENU) ) {
				debrief_close();				
				fsspeech_stop();
			}
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_close();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_close();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_close();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_close();
			break;

		case GS_STATE_CMD_BRIEF:
			if (new_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_hold();

			} else {
				cmd_brief_close();
				if (new_state == GS_STATE_MAIN_MENU)
					freespace_stop_mission();	
			}

			break;

		case GS_STATE_RED_ALERT:
			red_alert_close();
			if (new_state == GS_STATE_MAIN_MENU) {
				freespace_stop_mission();
			}
			break;

		case GS_STATE_STORYBOOK:
			storybook_close();
			break;

		case GS_STATE_SHIP_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_WEAPON_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_WEAPON_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_TEAM_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_WEAPON_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}					
			break;

		case GS_STATE_MAIN_MENU:
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
			mht_close();
#else
			main_hall_close();
#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_start_time();
#ifndef NO_NETWORK
			if(new_state == GS_STATE_MULTI_JOIN_GAME){
				multi_join_clear_game_list();
			}
#endif  // ifndef NO_NETWORK
			options_menu_close();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(new_state != GS_STATE_VIEW_MEDALS){
				barracks_close();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_close();
			break;

		case GS_STATE_TRAINING_MENU:
			training_menu_close();
			break;

		case GS_STATE_GAME_PLAY:
			if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
				player_save_target_and_weapon_link_prefs();
				game_stop_looped_sounds();
			}

#ifndef NO_SOUND
			sound_env_disable();
#endif
			joy_ff_stop_effects();

			// stop game time under certain conditions
			if ( end_mission || (Game_mode & GM_NORMAL) || ((Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MULTI_PAUSED)) ){
				game_stop_time();
			}

			if (end_mission) {
			// shut down any recording or playing demos
#ifdef DEMO_SYSTEM
				demo_close();
#endif

				// when in multiplayer and going back to the main menu, send a leave game packet
				// right away (before calling stop mission).  stop_mission was taking to long to
				// close mission down and I want people to get notified ASAP.
#ifndef NO_NETWORK
				if ( (Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MAIN_MENU) ){
					multi_quit_game(PROMPT_NONE);
				}
#endif  // ifndef NO_NETWORK

				freespace_stop_mission();			
				set_time_compression(1.0f);
			}
			break;

		case GS_STATE_TECH_MENU:
			techroom_close();
			break;

		case GS_STATE_TRAINING_PAUSED:
			Training_num_lines = 0;
			// fall through to GS_STATE_GAME_PAUSED

		case GS_STATE_GAME_PAUSED:
			game_start_time();
			if ( end_mission ) {
				pause_close(0);
			}
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_start_time();
				pause_debug_close();
			#endif
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_close();
			break;

#ifndef NO_NETWORK
		case GS_STATE_NET_CHAT:
			fs2ox_close();
			break;

		// join/start a game
		case GS_STATE_MULTI_JOIN_GAME:
			if(new_state != GS_STATE_OPTIONS_MENU){
				multi_join_game_close();
			}
			break;

		case GS_STATE_MULTI_HOST_SETUP:
		case GS_STATE_MULTI_CLIENT_SETUP:
			// if this is just the host going into the options screen, don't do anything
			if((new_state == GS_STATE_MULTI_HOST_OPTIONS) || (new_state == GS_STATE_OPTIONS_MENU)){
				break;
			}

			// close down the proper state
			if(old_state == GS_STATE_MULTI_HOST_SETUP){
				multi_create_game_close();
			} else {
				multi_game_client_setup_close();
			}

			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				if( (new_state != GS_STATE_TEAM_SELECT) && (Multi_chat_stream!=NULL) ) {
					cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
					cfclose(Multi_chat_stream);
				}
			}			
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_CONTROL_CONFIG:
			control_config_close();
			break;

		case GS_STATE_DEATH_DIED:
			Game_mode &= ~GM_DEAD_DIED;
			
			// early end while respawning or blowing up in a multiplayer game
			if((Game_mode & GM_MULTIPLAYER) && ((new_state == GS_STATE_DEBRIEF) || (new_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) ){
				game_stop_time();
				freespace_stop_mission();
			}
			break;

		case GS_STATE_DEATH_BLEW_UP:
			Game_mode &= ~GM_DEAD_BLEW_UP;

			// for single player, we might reload mission, etc.  For multiplayer, look at my new state
			// to determine if I should do anything.
			if ( !(Game_mode & GM_MULTIPLAYER) ) {
				if ( end_mission ){
					freespace_stop_mission();
				}
			} else {
				// if we are not respawing as an observer or as a player, our new state will not
				// be gameplay state.
				if ( (new_state != GS_STATE_GAME_PLAY) && (new_state != GS_STATE_MULTI_PAUSED) ) {
					game_stop_time();									// hasn't been called yet!!
					freespace_stop_mission();
				}
			}
			break;


		case GS_STATE_CREDITS:
			credits_close();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_close();
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_close();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			if ( new_state != GS_STATE_OPTIONS_MENU ) {
				mission_hotkey_close();
			}
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_MISSION_SYNC:
			// if we're moving into the options menu, don't do anything
			if(new_state == GS_STATE_OPTIONS_MENU){
				break;
			}

			Assert( Game_mode & GM_MULTIPLAYER );
			multi_sync_close();
			if ( new_state == GS_STATE_GAME_PLAY ){
				// palette_restore_palette();

				// change a couple of flags to indicate our state!!!
				Net_player->state = NETPLAYER_STATE_IN_MISSION;
				send_netplayer_update_packet();

				// set the game mode
				Game_mode |= GM_IN_MISSION;
			}			
			break;		
#endif  // ifndef NO_NETWORK
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_close();
			break;

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_close();
	  		break;

		case GS_STATE_STANDALONE_MAIN:			
			standalone_main_close();
			if(new_state == GS_STATE_MULTI_STD_WAIT){		
				init_multiplayer_stats();										
			}			
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_close();
			break;
#endif

#ifndef NO_NETWORK
		case GS_STATE_MULTI_PAUSED:
			// if ( end_mission ){
				pause_close(1);
			// }
			break;			

		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_close();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_INITIAL_PLAYER_SELECT:			
			player_select_close();			
			break;		

#ifndef NO_NETWORK
		case GS_STATE_MULTI_START_GAME:
			multi_start_game_close();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_close();
			break;				
#endif  // ifndef NO_NETWORK

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_close();
			break;

		case GS_STATE_LOOP_BRIEF:
			loop_brief_close();
			break;

		case GS_STATE_LAB:
			lab_close();
			break;
	}
}

// Called when a state is being entered.
// The current state is set to the state we're entering at
// this point, and old_state is set to the state we're coming
// from.    You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.

void game_enter_state( int old_state, int new_state )
{
	switch (new_state) {
		case GS_STATE_MAIN_MENU:				
#ifndef NO_NETWORK
			// in multiplayer mode, be sure that we are not doing networking anymore.
			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Net_player != NULL );
				Net_player->flags &= ~NETINFO_FLAG_DO_NETWORKING;
			}
#endif  // ifndef NO_NETWORK

			set_time_compression(1.0f);
	
			// determine which ship this guy is currently based on
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
			mht_init();
#else
			main_hall_init(Player->main_hall);
#endif
			break;

		case GS_STATE_BRIEFING:
			main_hall_stop_music();
			main_hall_stop_ambient();
			
			if (Game_mode & GM_NORMAL) {
				// AL: Don't call freespace_start_mission() if re-entering from ship or weapon select
				// MWA: or from options or hotkey screens
				// JH: or if the command brief state already did this
				if ( (old_state != GS_STATE_OPTIONS_MENU) && (old_state != GS_STATE_HOTKEY_SCREEN)
					&& (old_state != GS_STATE_SHIP_SELECT) && (old_state != GS_STATE_WEAPON_SELECT)
					&& (old_state != GS_STATE_CMD_BRIEF) ) {
					if ( !game_start_mission() )			// this should put us into a new state on failure!
						break;
				}
			}

			// red alert gamestate has own movie playing so check for it and skip all of this if needed.
			if ( red_alert_mission() ) {
				gameseq_post_event(GS_EVENT_RED_ALERT);
			} else {
				// maybe play a movie before the briefing.  don't play if entering briefing screen from ship or weapon select.
				if ( (old_state == GS_STATE_DEBRIEF) || (old_state == GS_STATE_SIMULATOR_ROOM) || (old_state == GS_STATE_MAIN_MENU))
					mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);

				set_time_compression(1.0f);

				brief_init();
			}

			break;

		case GS_STATE_DEBRIEF:
			game_stop_looped_sounds();
			mission_goal_fail_incomplete();				// fail all incomplete goals before entering debriefing
			if ( (old_state != GS_STATE_VIEW_MEDALS) && (old_state != GS_STATE_OPTIONS_MENU) ){
				debrief_init();
			}
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_init();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_init();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_init();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_init();
			break;

		case GS_STATE_RED_ALERT:
			set_time_compression(1.0f);
			mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);
			red_alert_init();
			break;

		case GS_STATE_STORYBOOK:
			storybook_init();
			break;

		case GS_STATE_CMD_BRIEF: {
			int team_num = 0;  // team number used as index for which cmd brief to use.

			if (old_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_unhold();

			} else {
				main_hall_stop_music();
				main_hall_stop_ambient();

				if (Game_mode & GM_NORMAL) {
					// AL: Don't call freespace_start_mission() if re-entering from ship or weapon select
					// MWA: or from options or hotkey screens
					// JH: or if the command brief state already did this
					if ( (old_state != GS_STATE_OPTIONS_MENU) && (old_state != GS_STATE_HOTKEY_SCREEN)
						&& (old_state != GS_STATE_SHIP_SELECT) && (old_state != GS_STATE_WEAPON_SELECT) ) {
						if ( !game_start_mission() )			// this should put us into a new state on failure!
							break;
					}
				}

				if ( red_alert_mission() ) {
					gameseq_post_event(GS_EVENT_RED_ALERT);
				} else {
					// maybe play a movie before the briefing.  don't play if entering briefing screen from ship or weapon select.
					if ( (old_state == GS_STATE_DEBRIEF) || (old_state == GS_STATE_SIMULATOR_ROOM) || (old_state == GS_STATE_MAIN_MENU))
						mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);

					set_time_compression(1.0f);

					cmd_brief_init(team_num);
				}
			}

			break;
		}

		case GS_STATE_SHIP_SELECT:
			ship_select_init();
			break;

		case GS_STATE_WEAPON_SELECT:
			weapon_select_init();
			break;

#ifndef NO_NETWORK
		case GS_STATE_TEAM_SELECT:		
			multi_ts_init();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_GAME_PAUSED:
			game_stop_time();
			pause_init(0);
			break;

		case GS_STATE_DEBUG_PAUSED:
	//		game_stop_time();
	//		os_set_title("FreeSpace - PAUSED");
	//		break;
	//
		case GS_STATE_TRAINING_PAUSED:
			#ifndef NDEBUG
				game_stop_time();
				pause_debug_init();
			#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_stop_time();
			options_menu_init();
			break;
 
		case GS_STATE_GAME_PLAY:
			// reset time compression to default level so it's right at the beginning of a mission - taylor
			if(old_state != GS_STATE_GAME_PAUSED)
			{
				//Game_time_compression = F1_0;
			}

			// coming from the gameplay state or the main menu, we might need to load the mission
			if ( (Game_mode & GM_NORMAL) && ((old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_GAME_PLAY) || (old_state == GS_STATE_DEATH_BLEW_UP)) ) {
				if ( !game_start_mission() )		// this should put us into a new state.
					// Failed!!!
					break;
			}

			// if we are coming from the briefing, ship select, weapons loadout, or main menu (in the
			// case of quick start), then do bitmap loads, etc  Don't do any of the loading stuff
			// if we are in multiplayer -- this stuff is all handled in the multi-wait section
			if ( !(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_BRIEFING) || (old_state == GS_STATE_SHIP_SELECT) ||
				(old_state == GS_STATE_WEAPON_SELECT) || (old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_MULTI_STD_WAIT)	|| (old_state == GS_STATE_SIMULATOR_ROOM) ) {
					// JAS: Used to do all paging here.

					#ifndef NDEBUG
					//XSTR:OFF
						HUD_printf("Skill level is set to ** %s **", Skill_level_names(Game_skill_level));
					//XSTR:ON
					#endif

					main_hall_stop_music();
					main_hall_stop_ambient();
					event_music_first_pattern();	// start the first pattern
			}

			// special code that restores player ship selection and weapons loadout when doing a quick start
			if ( !(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_DEATH_BLEW_UP)  || (old_state == GS_STATE_GAME_PLAY) ) {
				if ( !stricmp(Player_loadout.filename, Game_current_mission_filename) ) {
					wss_direct_restore_loadout();
				}
			}

			// single-player, quick-start after just died... we need to set weapon linking and kick off the event music
			if (!(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_DEATH_BLEW_UP) ) {
				event_music_first_pattern();	// start the first pattern
			}

			if ( !(Game_mode & GM_STANDALONE_SERVER) && (old_state != GS_STATE_GAME_PAUSED) && (old_state != GS_STATE_MULTI_PAUSED) ) {
				event_music_first_pattern();	// start the first pattern
			}			
			player_restore_target_and_weapon_link_prefs();

			//Set the current hud
#ifdef NEW_HUD
			set_current_hud(Player_ship);
#else
			set_current_hud(Player_ship->ship_info_index);
#endif

			Game_mode |= GM_IN_MISSION;

#ifndef NDEBUG
			// required to truely make mouse deltas zeroed in debug mouse code
void mouse_force_pos(int x, int y);
			mouse_force_pos(gr_screen.max_w / 2, gr_screen.max_h / 2);
#endif

			game_flush();

			// only start time if in single player, or coming from multi wait state
			if (
					(
						(Game_mode & GM_NORMAL) && 
						(old_state != GS_STATE_VIEW_CUTSCENES)
					) || (
						(Game_mode & GM_MULTIPLAYER) && (
							(old_state == GS_STATE_MULTI_PAUSED) ||
							(old_state == GS_STATE_MULTI_MISSION_SYNC)
						)
					)
				)
					game_start_time();

			// when coming from the multi paused state, reset the timestamps
#ifndef NO_NETWORK
			if ( (Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_MULTI_PAUSED) ){
				multi_reset_timestamps();
			}

			if ((Game_mode & GM_MULTIPLAYER) && (old_state != GS_STATE_DEATH_BLEW_UP) ) {
				// initialize all object update details
				multi_oo_gameplay_init();
			}
	
			// under certain circumstances, the server should reset the object update rate limiting stuff
			if( ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)) &&
				 (old_state == GS_STATE_MULTI_PAUSED) || (old_state == GS_STATE_MULTI_MISSION_SYNC) ){
				
				// reinitialize the rate limiting system for all clients
				multi_oo_rate_init_all();
			}

			// multiplayer clients should always re-initialize their control info rate limiting system			
			if((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				multi_oo_rate_init_all();
			}
			
			// reset ping times
			if(Game_mode & GM_MULTIPLAYER){
				multi_ping_reset_players();
			}
#endif  // ifndef NO_NETWORK

			Game_subspace_effect = 0;
			if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
				Game_subspace_effect = 1;
				if( !(Game_mode & GM_STANDALONE_SERVER) ){	
					game_start_subspace_ambient_sound();
				}
			}

#ifndef NO_SOUND
			sound_env_set(&Game_sound_env);
#endif
			joy_ff_mission_init(Ship_info[Player_ship->ship_info_index].rotation_time);

#ifndef NO_NETWORK
			// clear multiplayer button info			i
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));
#endif
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_init();
			break;

#ifndef NO_NETWORK
		case GS_STATE_NET_CHAT:
			fs2ox_init();
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			multi_join_clear_game_list();

			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_join_game_init();
			}

			break;

		case GS_STATE_MULTI_HOST_SETUP:		
			// don't reinitialize if we're coming back from the host options screen
			if ((old_state != GS_STATE_MULTI_HOST_OPTIONS) && (old_state != GS_STATE_OPTIONS_MENU)) {
				multi_create_game_init();
			}

			break;

		case GS_STATE_MULTI_CLIENT_SETUP:		
			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_game_client_setup_init();
			}

			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_CONTROL_CONFIG:
			control_config_init();
			break;

		case GS_STATE_TECH_MENU:
			techroom_init();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(old_state != GS_STATE_VIEW_MEDALS){
				barracks_init();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_init();
			break;

		case GS_STATE_DEATH_DIED:
 			Player_died_time = timestamp(10);

			if(!(Game_mode & GM_MULTIPLAYER)){
				player_show_death_message();
			}
			Game_mode |= GM_DEAD_DIED;
			break;

		case GS_STATE_DEATH_BLEW_UP:
			if ( !popupdead_is_active() ) {
				Player_ai->target_objnum = -1;
			}

			// stop any local EMP effect
			emp_stop_local();

			Players[Player_num].flags &= ~PLAYER_FLAGS_AUTO_TARGETING;	//	Prevent immediate switch to a hostile ship.
			Game_mode |= GM_DEAD_BLEW_UP;		
			Show_viewing_from_self = 0;

			// timestamp how long we should wait before displaying the died popup
			if ( !popupdead_is_active() ) {
				Player_died_popup_wait = timestamp(PLAYER_DIED_POPUP_WAIT);
			}
			break;

		case GS_STATE_GAMEPLAY_HELP:
			gameplay_help_init();
			break;

		case GS_STATE_CREDITS:
			main_hall_stop_music();
			main_hall_stop_ambient();
			credits_init();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_init(Player);
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_init();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			mission_hotkey_init();
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_MISSION_SYNC:
			// if we're coming from the options screen, don't do any
			if(old_state == GS_STATE_OPTIONS_MENU){
				break;
			}

			switch(Multi_sync_mode){
			case MULTI_SYNC_PRE_BRIEFING:
				// if moving from game forming to the team select state						
				multi_sync_init();			
				break;
			case MULTI_SYNC_POST_BRIEFING:
				// if moving from briefing into the mission itself			
				multi_sync_init();
			
				// tell everyone that we're now loading data
				Net_player->state = NETPLAYER_STATE_DATA_LOAD;
				send_netplayer_update_packet();

				// JAS: Used to do all paging here!!!!
								
				Net_player->state = NETPLAYER_STATE_WAITING;			
				send_netplayer_update_packet();				
				Missiontime = 0;
				set_time_compression(1.0f);
				break;
			case MULTI_SYNC_INGAME:
				multi_sync_init();
				break;
			}
			break;		
#endif  // ifndef NO_NETWORK
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_init();
			break;

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_init();
			break;

		case GS_STATE_STANDALONE_MAIN:
			// don't initialize if we're coming from one of these 2 states unless there are no 
			// players left (reset situation)
			if((old_state != GS_STATE_STANDALONE_POSTGAME) || multi_endgame_ending()){
				standalone_main_init();
			}
			break;	

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_init();
			break;
#endif  // ifndef NO_STANDALONE

#ifndef NO_NETWORK
		case GS_STATE_MULTI_PAUSED:
			pause_init(1);
			break;
		
		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_init();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_INITIAL_PLAYER_SELECT:
			player_select_init();
			break;		

#ifndef NO_NETWORK
		case GS_STATE_MULTI_START_GAME:
			multi_start_game_init();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_init();
			break;		
#endif  // ifndef NO_NETWORK

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_init();
			break;		

		case GS_STATE_LOOP_BRIEF:
			loop_brief_init();
			break;

		case GS_STATE_LAB:
			lab_init();
			break;
	} // end switch
}

// do stuff that may need to be done regardless of state
void game_do_state_common(int state,int no_networking)
{
	game_maybe_draw_mouse(flFrametime);		// determine if to draw the mouse this frame
	snd_do_frame();								// update sound system
	event_music_do_frame();						// music needs to play across many states

#ifndef NO_NETWORK
	multi_log_process();	

	if (no_networking) {
		return;
	}

	// maybe do a multiplayer frame based on game mode and state type	
	if (Game_mode & GM_MULTIPLAYER) {
		switch (state) {
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_GAMEPLAY_HELP:
			case GS_STATE_HOTKEY_SCREEN:
			case GS_STATE_HUD_CONFIG:
			case GS_STATE_CONTROL_CONFIG:
			case GS_STATE_MISSION_LOG_SCROLLBACK:
			case GS_STATE_SHOW_GOALS:
			case GS_STATE_VIEW_CUTSCENES:
			case GS_STATE_EVENT_DEBUG:
				multi_maybe_do_frame();
				break;
		}
		
		game_do_networking();
	}
#endif  // ifndef NO_NETWORK
}

// Called once a frame.
// You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
int Game_do_state_should_skip = 0;
void game_do_state(int state)
{
	// always lets the do_state_common() function determine if the state should be skipped
	Game_do_state_should_skip = 0;
	
	// legal to set the should skip state anywhere in this function
	game_do_state_common(state);	// do stuff that may need to be done regardless of state

	if(Game_do_state_should_skip){
		return;
	}
	
	switch (state) {
		case GS_STATE_MAIN_MENU:
			game_set_frametime(GS_STATE_MAIN_MENU);
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
			mht_do();
#else
			main_hall_do(flFrametime);
#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			game_set_frametime(GS_STATE_OPTIONS_MENU);
			options_menu_do_frame(flFrametime);
			break;

		case GS_STATE_BARRACKS_MENU:
			game_set_frametime(GS_STATE_BARRACKS_MENU);
			barracks_do_frame(flFrametime);
			break;

		case GS_STATE_TRAINING_MENU:
			game_set_frametime(GS_STATE_TRAINING_MENU);
			training_menu_do_frame(flFrametime);
			break;

		case GS_STATE_TECH_MENU:
			game_set_frametime(GS_STATE_TECH_MENU);
			techroom_do_frame(flFrametime);
			break;

		case GS_STATE_GAMEPLAY_HELP:
			game_set_frametime(GS_STATE_GAMEPLAY_HELP);
			gameplay_help_do_frame(flFrametime);
			break;

		case GS_STATE_GAME_PLAY:	// do stuff that should be done during gameplay
			game_do_frame();
			break;

		case GS_STATE_GAME_PAUSED:

			if(pause_get_type() == PAUSE_TYPE_VIEWER)	{

				read_player_controls( Player_obj, flFrametime);
			//	game_process_keys();
				game_frame(true);
			}
				
			pause_do(0);
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_set_frametime(GS_STATE_DEBUG_PAUSED);
				pause_debug_do();
			#endif
			break;

		case GS_STATE_TRAINING_PAUSED:
			game_training_pause_do();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			game_set_frametime(GS_STATE_LOAD_MISSION_MENU);
			mission_load_menu_do();
			break;
		
		case GS_STATE_BRIEFING:
			game_set_frametime(GS_STATE_BRIEFING);
			brief_do_frame(flFrametime);
			break;

		case GS_STATE_DEBRIEF:
			game_set_frametime(GS_STATE_DEBRIEF);
			debrief_do_frame(flFrametime);
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			game_set_frametime(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			multi_df_debrief_do();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_SHIP_SELECT:
			game_set_frametime(GS_STATE_SHIP_SELECT);
			ship_select_do(flFrametime);
			break;

		case GS_STATE_WEAPON_SELECT:
			game_set_frametime(GS_STATE_WEAPON_SELECT);
			weapon_select_do(flFrametime);
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			game_set_frametime(GS_STATE_MISSION_LOG_SCROLLBACK);
			hud_scrollback_do_frame(flFrametime);
			break;

		case GS_STATE_HUD_CONFIG:
			game_set_frametime(GS_STATE_HUD_CONFIG);
			hud_config_do_frame(flFrametime);
			break;

#ifndef NO_NETWORK
		case GS_STATE_NET_CHAT:
			game_set_frametime(GS_STATE_NET_CHAT);
			fs2ox_do_frame();
			break;


		case GS_STATE_MULTI_JOIN_GAME:
			game_set_frametime(GS_STATE_MULTI_JOIN_GAME);
			multi_join_game_do_frame();
			break;

		case GS_STATE_MULTI_HOST_SETUP:
			game_set_frametime(GS_STATE_MULTI_HOST_SETUP);
			multi_create_game_do();
			break;

		case GS_STATE_MULTI_CLIENT_SETUP:
			game_set_frametime(GS_STATE_MULTI_CLIENT_SETUP);
			multi_game_client_setup_do_frame();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_CONTROL_CONFIG:
			game_set_frametime(GS_STATE_CONTROL_CONFIG);
			control_config_do_frame(flFrametime);
			break;	

		case GS_STATE_DEATH_DIED:
			game_do_frame();			
			break;

		case GS_STATE_DEATH_BLEW_UP:
			game_do_frame();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			game_set_frametime(GS_STATE_SIMULATOR_ROOM);
			sim_room_do_frame(flFrametime);
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			game_set_frametime(GS_STATE_CAMPAIGN_ROOM);
			campaign_room_do_frame(flFrametime);
			break;

		case GS_STATE_RED_ALERT:
			game_set_frametime(GS_STATE_RED_ALERT);
			red_alert_do_frame(flFrametime);
			break;

		case GS_STATE_CMD_BRIEF:
			game_set_frametime(GS_STATE_CMD_BRIEF);
			cmd_brief_do_frame(flFrametime);
			break;

		case GS_STATE_STORYBOOK:
			game_set_frametime(GS_STATE_STORYBOOK);
			storybook_do_frame(flFrametime);
			break;

		case GS_STATE_CREDITS:
			game_set_frametime(GS_STATE_CREDITS);
			credits_do_frame(flFrametime);
			break;

		case GS_STATE_VIEW_MEDALS:
			game_set_frametime(GS_STATE_VIEW_MEDALS);
			medal_main_do();
			break;

		case GS_STATE_SHOW_GOALS:
			game_set_frametime(GS_STATE_SHOW_GOALS);
			mission_show_goals_do_frame(flFrametime);
			break;

		case GS_STATE_HOTKEY_SCREEN:
			game_set_frametime(GS_STATE_HOTKEY_SCREEN);
			mission_hotkey_do_frame(flFrametime);
			break;	
   
		case GS_STATE_VIEW_CUTSCENES:
			game_set_frametime(GS_STATE_VIEW_CUTSCENES);
			cutscenes_screen_do_frame();
			break;

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
		case GS_STATE_MULTI_STD_WAIT:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			multi_standalone_wait_do();
			break;

		case GS_STATE_STANDALONE_MAIN:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			standalone_main_do();
			break;	
#endif  // ifndef NO_STANDALONE

#ifndef NO_NETWORK
		case GS_STATE_MULTI_PAUSED:
			game_set_frametime(GS_STATE_MULTI_PAUSED);
			pause_do(1);
			break;

		case GS_STATE_TEAM_SELECT:
			game_set_frametime(GS_STATE_TEAM_SELECT);
			multi_ts_do();
			break;

		case GS_STATE_INGAME_PRE_JOIN:
			game_set_frametime(GS_STATE_INGAME_PRE_JOIN);
			multi_ingame_select_do();
			break;
#endif  // ifndef NO_NETWORK

		case GS_STATE_EVENT_DEBUG:
	#ifndef NDEBUG
			game_set_frametime(GS_STATE_EVENT_DEBUG);
			game_show_event_debug(flFrametime);
	#endif
			break;

#if !defined(NO_NETWORK) && !defined(NO_STANDALONE)
		case GS_STATE_STANDALONE_POSTGAME:
			game_set_frametime(GS_STATE_STANDALONE_POSTGAME);
			multi_standalone_postgame_do();
			break;
#endif  // ifndef NO_STANDALONE

		case GS_STATE_INITIAL_PLAYER_SELECT:
			game_set_frametime(GS_STATE_INITIAL_PLAYER_SELECT);
			player_select_do();
			break;

#ifndef NO_NETWORK
		case GS_STATE_MULTI_MISSION_SYNC:
			game_set_frametime(GS_STATE_MULTI_MISSION_SYNC);
			multi_sync_do();
			break;		

		case GS_STATE_MULTI_START_GAME:
			game_set_frametime(GS_STATE_MULTI_START_GAME);
			multi_start_game_do();
			break;
		
		case GS_STATE_MULTI_HOST_OPTIONS:
			game_set_frametime(GS_STATE_MULTI_HOST_OPTIONS);
			multi_host_options_do();
			break;		
#endif  // ifndef NO_NETWORK

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_do();
			break;		

		case GS_STATE_END_DEMO:
			game_set_frametime(GS_STATE_END_DEMO);
			end_demo_campaign_do();
			break;

		case GS_STATE_LOOP_BRIEF:
			game_set_frametime(GS_STATE_LOOP_BRIEF);
			loop_brief_do();
			break;

		case GS_STATE_LAB:
			game_set_frametime(GS_STATE_LAB);
			lab_do_frame(flFrametime);
			break;

   } // end switch(gs_current_state)
}


#ifdef _WIN32
// return 0 if there is enough RAM to run FreeSpace, otherwise return -1
int game_do_ram_check(int ram_in_bytes)
{
	if ( ram_in_bytes < 30*1024*1024 )	{
		int allowed_to_run = 1;
		if ( ram_in_bytes < 25*1024*1024 ) {
			allowed_to_run = 0;
		}

		char tmp[1024];
		int Freespace_total_ram_MB;
		Freespace_total_ram_MB = fl2i(ram_in_bytes/(1024*1024));

		if ( allowed_to_run ) {

			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n\nPress 'OK' to continue running with less than the minimum required memory\n", 193), Freespace_total_ram_MB, Freespace_total_ram_MB);

			int msgbox_rval;
			msgbox_rval = MessageBox( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OKCANCEL );
			if ( msgbox_rval == IDCANCEL ) {
				return -1;
			}

		} else {
			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n", 195), Freespace_total_ram_MB, Freespace_total_ram_MB);
			MessageBox( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OK );
			return -1;
		}
	}

	return 0;
}


#if 0 // no updater for fs2
// Check if there is a freespace.exe in the /update directory (relative to where fs.exe is installed).
// If so, copy it over and remove the update directory.
void game_maybe_update_launcher(char *exe_dir)
{
	char src_filename[MAX_PATH];
	char dest_filename[MAX_PATH];

	strcpy(src_filename, exe_dir);
	strcat(src_filename, NOX("\\update\\freespace.exe"));

	strcpy(dest_filename, exe_dir);
	strcat(dest_filename, NOX("\\freespace.exe"));

	// see if src_filename exists
	FILE *fp;
	fp = fopen(src_filename, "rb");
	if ( !fp ) {
		return;
	}
	fclose(fp);

	SetFileAttributes(dest_filename, FILE_ATTRIBUTE_NORMAL);

	// copy updated freespace.exe to freespace exe dir
	if ( CopyFile(src_filename, dest_filename, 0) == 0 ) {
		MessageBox( NULL, XSTR("Unable to copy freespace.exe from update directory to installed directory.  You should copy freespace.exe from the update directory (located in your FreeSpace install directory) to your install directory", 988), NULL, MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		return;
	}

	// delete the file in the update directory
	DeleteFile(src_filename);

	// safe to assume directory is empty, since freespace.exe should only be the file ever in the update dir
	char update_dir[MAX_PATH];
	strcpy(update_dir, exe_dir);
	strcat(update_dir, NOX("\\update"));
	RemoveDirectory(update_dir);
}
#endif // no launcher

#endif // ifdef WIN32

void game_spew_pof_info_sub(int model_num, polymodel *pm, int sm, CFILE *out, int *out_total, int *out_destroyed_total)
{
	int i;
	int sub_total = 0;
	int sub_total_destroyed = 0;
	int total = 0;
	char str[255] = "";		
	
	// get the total for all his children
	for (i=pm->submodel[sm].first_child; i >= 0; i = pm->submodel[i].next_sibling )	{
		game_spew_pof_info_sub(model_num, pm, i, out, &sub_total, &sub_total_destroyed);
	}	

	// find the # of faces for this _individual_ object	
	total = submodel_get_num_polys(model_num, sm);
	if(strstr(pm->submodel[sm].name, "-destroyed")){
		sub_total_destroyed = total;
	}
	
	// write out total
	sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[sm].name, total);
	cfputs(str, out);		

	*out_total += total + sub_total;
	*out_destroyed_total += sub_total_destroyed;
}

#define BAIL()			do { int _idx; for(_idx=0; _idx<num_files; _idx++){ if(pof_list[_idx] != NULL){vm_free(pof_list[_idx]); pof_list[_idx] = NULL;}} return;} while(0);
void game_spew_pof_info()
{
	char *pof_list[1000];
	int num_files;	
	CFILE *out;
	int idx, model_num, i, j;
	polymodel *pm;
	int total, root_total, model_total, destroyed_total, counted;
	char str[255] = "";

	// get file list
	num_files = cf_get_file_list(1000, pof_list, CF_TYPE_MODELS, "*.pof");

	// spew info on all the pofs
	if(!num_files){
		return;
	}

	// go
	out = cfopen("pofspew.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		BAIL();
	}	
	counted = 0;	
	for(idx=0; idx<num_files; idx++, counted++){
		sprintf(str, "%s.pof", pof_list[idx]);
		model_num = model_load(str, 0, NULL);
		if(model_num >= 0){
			pm = model_get(model_num);

			// if we have a real model
			if(pm != NULL){				
				cfputs(str, out);
				cfputs("\n", out);
				
				// go through and print all raw submodels
				cfputs("RAW\n", out);
				total = 0;
				model_total = 0;				
				for (i=0; i<pm->n_models; i++)	{					
					total = submodel_get_num_polys(model_num, i);					
					
					model_total += total;
					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[i].name, total);
					cfputs(str, out);
				}				
				sprintf(str, "Model total %d\n", model_total);				
				cfputs(str, out);				

				// now go through and do it by LOD
				cfputs("BY LOD\n\n", out);				
				for(i=0; i<pm->n_detail_levels; i++){
					sprintf(str, "LOD %d\n", i);
					cfputs(str, out);

					// submodels
					root_total = submodel_get_num_polys(model_num, pm->detail[i] );
					total = 0;
					destroyed_total = 0;
					for (j=pm->submodel[pm->detail[i]].first_child; j >= 0; j = pm->submodel[j].next_sibling )	{
						game_spew_pof_info_sub(model_num, pm, j, out, &total, &destroyed_total);
					}

					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[pm->detail[i]].name, root_total);
					cfputs(str, out);

					sprintf(str, "TOTAL: %d\n", total + root_total);					
					cfputs(str, out);
					sprintf(str, "TOTAL not counting destroyed faces %d\n", (total + root_total) - destroyed_total);
					cfputs(str, out);
					sprintf(str, "TOTAL destroyed faces %d\n\n", destroyed_total);
					cfputs(str, out);
				}				
				cfputs("------------------------------------------------------------------------\n\n", out);				
			}
		}

		if(counted >= MAX_POLYGON_MODELS - 5){
			model_free_all();
			counted = 0;
		}
	}

	cfclose(out);
	model_free_all();
	BAIL();
}

DCF(pofspew, "")
{
	game_spew_pof_info();
}

#ifdef _WIN32
int PASCAL WinMainSub(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow)
#else
int WinMainSub(int argc, char *argv[])
#endif
{



#ifdef _DEBUG
#ifdef _WIN32
	void memblockinfo_output_memleak();
	atexit(memblockinfo_output_memleak);
#endif
#endif

	int state;		

#ifdef _WIN32
	// Don't let more than one instance of Freespace run.
	HWND hwnd = FindWindow( NOX( "FreeSpaceClass" ), NULL );
	if ( hwnd )	{
		SetForegroundWindow(hwnd);
		return 0;
	}

	// Find out how much RAM is on this machine
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	Freespace_total_ram = ms.dwTotalPhys;

	Mem_starttime_phys      = ms.dwAvailPhys;
	Mem_starttime_pagefile  = ms.dwAvailPageFile;
	Mem_starttime_virtual   = ms.dwAvailVirtual;

	if ( game_do_ram_check(Freespace_total_ram) == -1 ) {
		return 0;
	}

	if ( ms.dwTotalVirtual < 1024 )	{
		MessageBox( NULL, XSTR( "FreeSpace requires virtual memory to run.\r\n", 196), XSTR( "No Virtual Memory", 197), MB_OK );
		return 0;
	}

	if (!vm_init(24*1024*1024)) {
		MessageBox( NULL, XSTR( "Not enough memory to run Freespace.\r\nTry closing down some other applications.\r\n", 198), XSTR( "Not Enough Memory", 199), MB_OK );
		return 0;
	}
		
	char *tmp_mem = (char *) vm_malloc(16 * 1024 * 1024);
	if (!tmp_mem) {
		MessageBox(NULL, XSTR( "Not enough memory to run Freespace.\r\nTry closing down some other applications.\r\n", 198), XSTR( "Not Enough Memory", 199), MB_OK);
		return 0;
	}

	vm_free(tmp_mem);
	tmp_mem = NULL;

	//=====================================================
	// Make sure we're running in the right directory.
	char exe_dir[1024];

	if ( GetModuleFileName( hInst, exe_dir, 1023 ) > 0 )	{
		char *p = exe_dir + strlen(exe_dir);

		// chop off the filename
		while( (p>exe_dir) && (*p!='\\') && (*p!='/') && (*p!=':') )	{
			p--;
		}
		*p = 0;

		// Set directory
		if ( strlen(exe_dir) > 0 )	{
			SetCurrentDirectory(exe_dir);
		}

#if 0 // no launcher for fs2_open
		// check for updated freespace.exe
		game_maybe_update_launcher(exe_dir);
#endif

	}
#else
	vm_init(0); 
	
	// create user's directory
	char userdir[MAX_PATH];

	snprintf(userdir, MAX_PATH, "%s/%s/", detect_home(), Osreg_user_dir);
	_mkdir(userdir);
#endif

	
	#ifndef NDEBUG				
	{
		extern void windebug_memwatch_init();
		windebug_memwatch_init();
	}
	#endif


#ifdef _WIN32
	if(parse_cmdline(szCmdLine) == false)
	{
		return 0;
	}
#else
   parse_cmdline(argc, argv);
#endif

#ifndef NO_NETWORK
#ifdef STANDALONE_ONLY_BUILD
	Is_standalone = 1;
	nprintf(("Network", "Standalone running"));
#else
	if (Is_standalone){
		nprintf(("Network", "Standalone running"));
	}
#endif
#endif  // ifndef NO_NETWORK


#ifdef _WIN32
	init_cdrom();
#endif

	game_init();

	game_stop_time();

	if (Cmdline_SpewMission_CRCs) // -missioncrcs
	{
#ifndef NO_NETWORK
		multi_spew_pxo_checksums(1024, "FS2MissionsCRCs.txt");
#endif
		game_shutdown();
		return 0;
	}


	if (Cmdline_SpewTable_CRCs)
	{
#ifndef NO_NETWORK
		multi_spew_table_checksums(50, "FS2TablesCRCs.txt");
#endif
		game_shutdown();
		return 0;
	}
	// maybe spew pof stuff
	if(Cmdline_spew_pof_info){
		game_spew_pof_info();
		game_shutdown();
		return 1;
	}

// Windows code now plays the intro movie in game_init

	movie_play( NOX("intro.mve"));

/* not using this anymore, it's not doing anything anyway - taylor
#ifndef _WIN32
	// non-demo, non-standalone, play the intro movie
	if(!Is_standalone){
#ifndef DEMO
#ifdef RELEASE_REAL
		char *plist[5];
		if( (cf_get_file_list(2, plist, CF_TYPE_MULTI_PLAYERS, NOX("*.plr"))	<= 0) && (cf_get_file_list(2, plist, CF_TYPE_SINGLE_PLAYERS, NOX("*.plr"))	<= 0) ){
			// prompt for cd 2
#ifdef _WIN32
#if defined(OEM_BUILD)
			game_do_cd_check_specific(FS_CDROM_VOLUME_1, 1);
#else
			game_do_cd_check_specific(FS_CDROM_VOLUME_2, 2);
#endif // defined(OEM_BUILD)
#endif // ifdef WIN32
		}
#endif // ifdef RELEASE_REAL
	}

#endif // ifndef DEMO
#endif // !_WIN32
*/

	if (Is_standalone){
		gameseq_post_event(GS_EVENT_STANDALONE_MAIN);
	} else {
		gameseq_post_event(GS_EVENT_GAME_INIT);		// start the game rolling -- check for default pilot, or go to the pilot select screen
	}

	while (1) {
		// only important for non THREADED mode
		os_poll();

		state = gameseq_process_events();
		if ( state == GS_STATE_QUIT_GAME ){
			break;
		}


	} 

#ifdef FS2_DEMO
	if(!Is_standalone){
		demo_upsell_show_screens();
	}
#elif defined(OEM_BUILD)
	// show upsell screens on exit
	oem_upsell_show_screens();
#endif

	game_shutdown();

#ifdef _WIN32
	return 1;
#else
	return 0;
#endif
}

#ifdef _WIN32
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{

#ifdef APPLE_APP
	// Finder sets the working directory to the root of the drive so we have to get a little creative
	// to find out where on the disk we should be running from for CFILE's sake.
	strncpy(full_path, *argv, 1024);
#endif

#ifdef _WIN32
#ifdef _DEBUG
	void memblockinfo_output_memleak();
	atexit(memblockinfo_output_memleak);
#endif
	::CoInitialize(NULL);
#endif

	DBUGFILE_INIT();
	int result = -1;

#ifdef _WIN32
#if defined(_MSC_VER)
	__try	{
#endif
		result = WinMainSub(hInst, hPrev, szCmdLine, nCmdShow);
#if defined(_MSC_VER)
	}	__except(RecordExceptionInfo(GetExceptionInformation(), "Freespace 2 Main Thread"))	{

		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
#endif

#else
	try {
		result = WinMainSub(argc, argv);
	} catch ( ... ) {
		fprintf(stderr, "Caught exception in main\n");
		result = 1;
	}
#endif

	DBUGFILE_DEINIT()


#ifdef _WIN32
	::CoUninitialize();

#ifndef _MINGW
	_CrtDumpMemoryLeaks();
#endif // !_MINGW
#endif

	return result;
}

#if 0  // don't have an updater for fs2_open
// launch the fslauncher program on exit
void game_launch_launcher_on_exit()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char cmd_line[2048];
	char original_path[1024] = "";
	
	memset( &si, 0, sizeof(STARTUPINFO) );
	si.cb = sizeof(si);

	// directory
	_getcwd(original_path, 1023);

	// set up command line
	strcpy(cmd_line, original_path);
	strcat(cmd_line, DIR_SEPARATOR_STR);
	strcat(cmd_line, LAUNCHER_FNAME);
	strcat(cmd_line, " -straight_to_update");		

	BOOL ret = CreateProcess(	NULL,									// pointer to name of executable module 
										cmd_line,							// pointer to command line string
										NULL,									// pointer to process security attributes 
										NULL,									// pointer to thread security attributes 
										FALSE,								// handle inheritance flag 
										CREATE_DEFAULT_ERROR_MODE,		// creation flags 
										NULL,									// pointer to new environment block 
										NULL,									// pointer to current directory name 
										&si,									// pointer to STARTUPINFO 
										&pi									// pointer to PROCESS_INFORMATION  
										);			
	// to eliminate build warnings
	ret;
}
#endif


// game_shutdown()
//
// This function is called when FreeSpace terminates normally.  
//
void game_shutdown(void)
{
#ifdef _WIN32
	timeEndPeriod(1);
#endif


	fsspeech_deinit();

	// don't ever flip a page on the standalone!
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		gr_reset_clip();
		gr_clear();
		gr_flip();
	}

   // if the player has left the "player select" screen and quit the game without actually choosing
	// a player, Player will be NULL, in which case we shouldn't write the player file out!
	if (!(Game_mode & GM_STANDALONE_SERVER) && (Player!=NULL) && !Is_standalone){
		write_pilot_file();
		mission_campaign_savefile_save();
	}

#ifndef NO_NETWORK
	// load up common multiplayer icons
	multi_unload_common_icons();
#endif  // ifndef NO_NETWORK
	
	shockwave_close();			// release any memory used by shockwave system	
	fireball_close();				// free fireball system
	particle_close();			// close out the particle system
	ship_close();					// free any memory that was allocated for the ships
	hud_free_scrollback_list();// free space allocated to store hud messages in hud scrollback
	unload_animating_pointer();// frees the frames used for the animating mouse pointer
	mission_campaign_close();	// close out the campaign stuff
	message_mission_close();	// clear loaded table data from message.tbl
	mission_parse_close();		// clear out any extra memory that may be in use by mission parsing
#ifndef NO_NETWORK
	multi_voice_close();			// close down multiplayer voice (including freeing buffers, etc)
	multi_log_close();
#ifdef MULTI_USE_LAG
	multi_lag_close();
#endif
#endif  // ifndef NO_NETWORK

	// the menu close functions will unload the bitmaps if they were displayed during the game
#if !defined(PRESS_TOUR_BUILD) && !defined(PD_BUILD)
	main_hall_close();
#endif
	training_menu_close();
	gr_close();

	// free left over memory from table parsing
	player_tips_close();

	joy_close();

	audiostream_close();
	snd_close();
	event_music_close();
	gamesnd_close();		// close out gamesnd, needs to happen *after* other sounds are closed
#ifndef NO_NETWORK
	psnet_close();
#endif

	model_free_all();
	bm_unload_all();			// unload/free bitmaps, has to be called *after* model_free_all()!

	os_cleanup();

#if 0  // don't have an updater for fs2_open
	// HACKITY HACK HACK
	// if this flag is set, we should be firing up the launcher when exiting freespace
	extern int Multi_update_fireup_launcher_on_exit;
	if(Multi_update_fireup_launcher_on_exit){
		game_launch_launcher_on_exit();
	}
#endif
}

// game_stop_looped_sounds()
//
// This function will call the appropriate stop looped sound functions for those
// modules which use looping sounds.  It is not enough just to stop a looping sound
// at the DirectSound level, the game is keeping track of looping sounds, and this
// function is used to inform the game that looping sounds are being halted.
//
void game_stop_looped_sounds()
{
	hud_stop_looped_locking_sounds();
	hud_stop_looped_engine_sounds();
	afterburner_stop_sounds();
	player_stop_looped_sounds();
#ifndef NO_SOUND
	obj_snd_stop_all();		// stop all object-linked persistant sounds
#endif
	game_stop_subspace_ambient_sound();
	snd_stop(Radar_static_looping);
	Radar_static_looping = -1;
	snd_stop(Target_static_looping);
	shipfx_stop_engine_wash_sound();
	Target_static_looping = -1;
}

//////////////////////////////////////////////////////////////////////////
//
// Code for supporting an animating mouse pointer
//
//
//////////////////////////////////////////////////////////////////////////

typedef struct animating_obj
{
	int	first_frame;
	int	num_frames;
	int	current_frame;
	float time;
	float elapsed_time;
} animating_obj;

static animating_obj Animating_mouse;

// ----------------------------------------------------------------------------
// init_animating_pointer()
//
// Called by load_animating_pointer() to ensure the Animating_mouse struct
// gets properly initialized
//
void init_animating_pointer()
{
	Animating_mouse.first_frame	= -1;
	Animating_mouse.num_frames		= 0;
	Animating_mouse.current_frame	= -1;
	Animating_mouse.time				= 0.0f;
	Animating_mouse.elapsed_time	= 0.0f;
}

// ----------------------------------------------------------------------------
// load_animating_pointer()
//
// Called at game init to load in the frames for the animating mouse pointer
//
// input:	filename	=>	filename of animation file that holds the animation
// 
void load_animating_pointer(char *filename, int dx, int dy)
{
	int				fps;
	animating_obj *am;

	init_animating_pointer();

//TEMP
	mprintf(("loading animated cursor \"%s\"\n", filename));


	am = &Animating_mouse;
	am->first_frame = bm_load_animation(filename, &am->num_frames, &fps);
	if ( am->first_frame == -1 ) 
		Error(LOCATION, "Could not load animation %s for the mouse pointer\n", filename);
	am->current_frame = 0;
	am->time = am->num_frames / i2fl(fps);
}

// ----------------------------------------------------------------------------
// unload_animating_pointer()
//
// Called at game shutdown to free the memory used to store the animation frames
//
void unload_animating_pointer()
{
	int				i;
	animating_obj	*am;

	am = &Animating_mouse;
	for ( i = 0; i < am->num_frames; i++ ) {
		Assert( (am->first_frame+i) >= 0 );

		// if we are the current cursor then reset to avoid gr_close() issues - taylor
		gr_unset_cursor_bitmap(am->first_frame + i);
	}

	// this will release all of the frames at once
	if (am->first_frame >= 0)
		bm_release(am->first_frame);

	am->first_frame	= -1;
	am->num_frames		= 0;
	am->current_frame = -1;
}

// draw the correct frame of the game mouse... called from game_maybe_draw_mouse()
void game_render_mouse(float frametime)
{
	int				mx, my;
	animating_obj	*am;

	// if animating cursor exists, play the next frame
	am = &Animating_mouse;
	if ( am->first_frame != -1 ) {
		mouse_get_pos(&mx, &my);
		am->elapsed_time += frametime;
		am->current_frame = fl2i( ( am->elapsed_time / am->time ) * (am->num_frames-1) );
		if ( am->current_frame >= am->num_frames ) {
			am->current_frame = 0;
			am->elapsed_time = 0.0f;
		}
		gr_set_cursor_bitmap(am->first_frame + am->current_frame);
	}
}

// ----------------------------------------------------------------------------
// game_maybe_draw_mouse()
//
// determines whether to draw the mouse pointer at all, and what frame of
// animation to use if the mouse is animating
//
// Sets mouse.cpp globals Mouse_hidden and Mouse_moved based on the state of the game.
//
// input:	frametime => elapsed frame time in seconds since last call
//
void game_maybe_draw_mouse(float frametime)
{
	int game_state;

	game_state = gameseq_get_state();

	switch ( game_state ) {
		case GS_STATE_GAME_PAUSED:
		// case GS_STATE_MULTI_PAUSED:
		case GS_STATE_GAME_PLAY:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_DEATH_BLEW_UP:
			if ( popup_active() || popupdead_is_active() ) {
				Mouse_hidden = 0;
			} else {
				Mouse_hidden = 1;	
			}
			break;

		default:
			Mouse_hidden = 0;
			break;
	}	// end switch

	if ( !Mouse_hidden ) 
		game_render_mouse(frametime);

}

void game_do_training_checks()
{
	int i, s;
	float d;
	waypoint_list *wplp;

	if (Training_context & TRAINING_CONTEXT_SPEED) {
		s = (int) Player_obj->phys_info.fspeed;
		if ((s >= Training_context_speed_min) && (s <= Training_context_speed_max)) {
			if (!Training_context_speed_set) {
				Training_context_speed_set = 1;
				Training_context_speed_timestamp = timestamp();
			}

		} else
			Training_context_speed_set = 0;
	}

	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		wplp = &Waypoint_lists[Training_context_path];
		if (wplp->count > Training_context_goal_waypoint) {
			i = Training_context_goal_waypoint;
			do {
				d = vm_vec_dist(&wplp->waypoints[i], &Player_obj->pos);
				if (d <= Training_context_distance) {
					Training_context_at_waypoint = i;
					if (Training_context_goal_waypoint == i) {
						Training_context_goal_waypoint++;
						snd_play(&Snds[SND_CARGO_REVEAL], 0.0f);
					}

					break;
				}

				i++;
				if (i == wplp->count)
					i = 0;

			} while (i != Training_context_goal_waypoint);
		}
	}

	if ((Players_target == UNINITIALIZED) || (Player_ai->target_objnum != Players_target) || (Player_ai->targeted_subsys != Players_targeted_subsys)) {
		Players_target = Player_ai->target_objnum;
		Players_targeted_subsys = Player_ai->targeted_subsys;
		Players_target_timestamp = timestamp();
	}
	// following added by Sesquipedalian for is_missile_locked
	if ((Players_mlocked == UNINITIALIZED) || (Player_ai->current_target_is_locked != Players_mlocked)) {
		Players_mlocked = Player_ai->current_target_is_locked;
		Players_mlocked_timestamp = timestamp();
	}

}

/////////// Following is for event debug view screen

#ifndef NDEBUG

#define EVENT_DEBUG_MAX	5000
#define EVENT_DEBUG_EVENT 0x8000

int Event_debug_index[EVENT_DEBUG_MAX];
int ED_count;

void game_add_event_debug_index(int n, int indent)
{
	if (ED_count < EVENT_DEBUG_MAX)
		Event_debug_index[ED_count++] = n | (indent << 16);
}

void game_add_event_debug_sexp(int n, int indent)
{
	if (n < 0)
		return;

	if (Sexp_nodes[n].first >= 0) {
		game_add_event_debug_sexp(Sexp_nodes[n].first, indent);
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
		return;
	}

	game_add_event_debug_index(n, indent);
	if (Sexp_nodes[n].subtype == SEXP_ATOM_OPERATOR)
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent + 1);
	else
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
}

void game_event_debug_init()
{
	int e;

	ED_count = 0;
	for (e=0; e<Num_mission_events; e++) {
		game_add_event_debug_index(e | EVENT_DEBUG_EVENT, 0);
		game_add_event_debug_sexp(Mission_events[e].formula, 1);
	}
}

void game_show_event_debug(float frametime) 
{
	char buf[256];
	int i, k, z;
	int font_height, font_width;	
	int y_index, y_max;
	static int scroll_offset = 0;
	
	k = game_check_key();
	if (k)
		switch (k) {
			case KEY_UP:
			case KEY_PAD8:
				scroll_offset--;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_DOWN:
			case KEY_PAD2:
				scroll_offset++;
				break;

			case KEY_PAGEUP:
				scroll_offset -= 20;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_PAGEDOWN:
				scroll_offset += 20;	// not font-independent, hard-coded since I counted the lines!
				break;

			default:
				gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
				key_flush();
				break;
		} // end switch

	gr_clear();
	gr_set_color_fast(&Color_bright);
	gr_set_font(FONT1);
	gr_printf(0x8000, 5, NOX("EVENT DEBUG VIEW"));

	gr_set_color_fast(&Color_normal);
	gr_set_font(FONT1);
	gr_get_string_size(&font_width, &font_height, NOX("test"));
	y_max = gr_screen.max_h - font_height - 5;
	y_index = 45;

	k = scroll_offset;
	while (k < ED_count) {
		if (y_index > y_max)
			break;

		z = Event_debug_index[k];
		if (z & EVENT_DEBUG_EVENT) {
			z &= 0x7fff;
			sprintf(buf, NOX("%s%s (%s) %s%d %d"), (Mission_events[z].flags & MEF_CURRENT) ? NOX("* ") : "",
				Mission_events[z].name, Mission_events[z].result ? NOX("True") : NOX("False"),
				(Mission_events[z].chain_delay < 0) ? "" : NOX("x "),
				Mission_events[z].repeat_count, Mission_events[z].interval);

		} else {
			i = (z >> 16) * 3;
			buf[i] = 0;
			while (i--)
				buf[i] = ' ';

			strcat(buf, Sexp_nodes[z & 0x7fff].text);
			switch (Sexp_nodes[z & 0x7fff].value) {
				case SEXP_TRUE:
					strcat(buf, NOX(" (True)"));
					break;

				case SEXP_FALSE:
					strcat(buf, NOX(" (False)"));
					break;

				case SEXP_KNOWN_TRUE:
					strcat(buf, NOX(" (Always true)"));
					break;

				case SEXP_KNOWN_FALSE:
					strcat(buf, NOX(" (Always false)"));
					break;

				case SEXP_CANT_EVAL:
					strcat(buf, NOX(" (Can't eval)"));
					break;

				case SEXP_NAN:
				case SEXP_NAN_FOREVER:
					strcat(buf, NOX(" (Not a number)"));
					break;
			}
		}

		gr_printf(10, y_index, buf);
		y_index += font_height;
		k++;
	}

	gr_flip();
}

#endif // NDEBUG

#ifndef NDEBUG
FILE * Time_fp;
FILE * Texture_fp;

int Tmap_npixels=0;
int Tmap_num_too_big = 0;
int Num_models_needing_splitting = 0;

void Time_model( int modelnum )
{
//	mprintf(( "Timing ship '%s'\n", si->name ));

	vec3d eye_pos, model_pos;
	matrix eye_orient, model_orient;

	polymodel *pm = model_get( modelnum );

	int l = strlen(pm->filename);
	while( (l>0) )	{
		if ( (l == '/') || (l=='\\') || (l==':'))	{
			l++;
			break;
		}
		l--;
	}
	char *pof_file = &pm->filename[l];

	int model_needs_splitting = 0;

	//fprintf( Texture_fp, "Model: %s\n", pof_file );
	int i;
	for (i=0; i<pm->n_textures; i++ )	{
		char filename[1024];
		ubyte pal[768];

		int bmp_num = pm->original_textures[i];
		if ( bmp_num > -1 )	{
			bm_get_palette(pm->original_textures[i], pal, filename );		
			int w,h;
			bm_get_info( pm->original_textures[i],&w, &h );


			if ( (w > 512) || (h > 512) )	{
				fprintf( Texture_fp, "%s\t%s\t%d\t%d\n", pof_file, filename, w, h );
				Tmap_num_too_big++;
				model_needs_splitting++;
			}
		} else {
			//fprintf( Texture_fp, "\tTexture %d is bogus\n", i );
		}
	}

	if ( model_needs_splitting )	{
		Num_models_needing_splitting++;
	}
	eye_orient = model_orient = vmd_identity_matrix;
	eye_pos = model_pos = vmd_zero_vector;

	eye_pos.xyz.z = -pm->rad*2.0f;

	vec3d eye_to_model;

	vm_vec_sub( &eye_to_model, &model_pos, &eye_pos );
	vm_vector_2_matrix( &eye_orient, &eye_to_model, NULL, NULL );

	fix t1 = timer_get_fixed_seconds();

	angles ta;
	ta.p = ta.b = ta.h = 0.0f; 
	int framecount = 0;

	Tmap_npixels = 0;

	int bitmaps_used_this_frame, bitmaps_new_this_frame;
		
	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);

	modelstats_num_polys = modelstats_num_verts = 0;

	while( ta.h < PI2 )	{

		matrix m1;
		vm_angles_2_matrix(&m1, &ta );
		vm_matrix_x_matrix( &model_orient, &vmd_identity_matrix, &m1 );

		gr_reset_clip();
//		gr_clear();

		g3_start_frame(1);

		g3_set_view_matrix( &eye_pos, &eye_orient, Viewer_zoom );	

		model_clear_instance( modelnum );
		model_set_detail_level(0);		// use highest detail level
		model_render( modelnum, &model_orient, &model_pos, MR_LOCK_DETAIL);	//|MR_NO_POLYS );

		g3_end_frame();
//		gr_flip();

		framecount++;
		ta.h += 0.1f;

		int k = key_inkey();
		if ( k == KEY_ESC ) {
			exit(1);
		}
	}

	fix t2 = timer_get_fixed_seconds();

	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);

	modelstats_num_polys /= framecount;
	modelstats_num_verts /= framecount;

	Tmap_npixels /=framecount;


	mprintf(( "'%s' is %.2f FPS\n", pof_file, i2fl(framecount)/f2fl(t2-t1) ));
	fprintf( Time_fp, "\"%s\"\t%.0f\t%d\t%d\t%d\t%d\n", pof_file, i2fl(framecount)/f2fl(t2-t1), bitmaps_used_this_frame, modelstats_num_polys, modelstats_num_verts, Tmap_npixels );
}

int Time_models = 0;
DCF_BOOL( time_models, Time_models );

void Do_model_timings_test()
{
	

	if ( !Time_models ) return;

	mprintf(( "Timing models!\n" ));

	int i;

	ubyte model_used[MAX_POLYGON_MODELS];
	int model_id[MAX_POLYGON_MODELS];
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		model_used[i] = 0;
	}
	
	// Load them all
	for (i=0; i<Num_ship_types; i++ )	{
		Ship_info[i].modelnum = model_load( Ship_info[i].pof_file, 0, NULL );

		model_used[Ship_info[i].modelnum%MAX_POLYGON_MODELS]++;
		model_id[Ship_info[i].modelnum%MAX_POLYGON_MODELS] = Ship_info[i].modelnum;
	}

	Texture_fp = fopen( NOX("ShipTextures.txt"), "wt" );
	if ( !Texture_fp ) return;

	Time_fp = fopen( NOX("ShipTimings.txt"), "wt" );
	if ( !Time_fp ) return;

	fprintf( Time_fp, "Name\tFPS\tTRAM\tPolys\tVerts\tPixels\n" );
//	fprintf( Time_fp, "FPS\tTRAM\tPolys\tVerts\tPixels\n" );
	
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		if ( model_used[i] )	{
			Time_model( model_id[i] );
		}
	}
	
	fprintf( Texture_fp, "Number too big: %d\n", Tmap_num_too_big );
	fprintf( Texture_fp, "Number of models needing splitting: %d\n", Num_models_needing_splitting );
	
	fclose(Time_fp);
	fclose(Texture_fp);

	exit(1);
}
#endif

// Call this function when you want to inform the player that a feature is not
// enabled in the DEMO version of FreSpace
void game_feature_not_in_demo_popup()
{
	popup(PF_USE_AFFIRMATIVE_ICON|PF_BODY_BIG, 1, POPUP_OK, XSTR( "Sorry, this feature is available only in the retail version", 200));
}

// Call this function when you want to inform the player that a feature is disabled in this build
void game_feature_disabled_popup()
{
	popup(PF_USE_AFFIRMATIVE_ICON|PF_BODY_BIG, 1, POPUP_OK, XSTR( "Sorry, the requested feature is currently disabled in this build", -1));
}

// format the specified time (fixed point) into a nice string
void game_format_time(fix m_time,char *time_str)
{
	float mtime;
	int hours,minutes,seconds;
	char tmp[10];

	mtime = f2fl(m_time);		

	// get the hours, minutes and seconds	
	hours = (int)(mtime / 3600.0f);
	if(hours > 0){
		mtime -= (3600.0f * (float)hours);
	}
	seconds = (int)mtime%60;
	minutes = (int)mtime/60;			

	// print the hour if necessary
	if(hours > 0){		
		sprintf(time_str,XSTR( "%d:", 201),hours);
		// if there are less than 10 minutes, print a leading 0
		if(minutes < 10){
			strcpy(tmp,NOX("0"));
			strcat(time_str,tmp);
		}		
	}	
	
	// print the minutes
	if(hours){
		sprintf(tmp,XSTR( "%d:", 201),minutes);
		strcat(time_str,tmp);
	} else {
		sprintf(time_str,XSTR( "%d:", 201),minutes);
	}

	// print the seconds
	if(seconds < 10){
		strcpy(tmp,NOX("0"));
		strcat(time_str,tmp);
	} 
	sprintf(tmp,"%d",seconds);
	strcat(time_str,tmp);
}

//	Stuff version string in *str.
void get_version_string(char *str)
{
//XSTR:OFF
if ( FS_VERSION_BUILD == 0 ) {
	sprintf(str,"V%d.%d", FS_VERSION_MAJOR, FS_VERSION_MINOR);
} else {
	sprintf(str,"V%d.%d.%d", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD );
}

#ifdef INF_BUILD
	strcat(str, " Inferno");
#endif

#ifdef FS2_DEMO
	strcat(str, " Demo");
#endif

#ifdef _DEBUG
	strcat(str, " Dbg");
#endif

	// append the CVS "release" version in the $Name variable, but
	// only do this if it's been tagged
	int rcs_name_len = strlen(RCS_Name);
	if (rcs_name_len > 11) {
		char buffer[100];
		strcpy(buffer, RCS_Name + 7);
		buffer[rcs_name_len-9] = 0;

		strcat(str, " Build: ");
        strcat(str, buffer);
	} else {
		strcat(str, " (fs2_open)");
	}

	// Lets get some more info in here
	switch(gr_screen.mode)
	{
		case GR_DIRECT3D: strcat(str, " D3D"); break;
		case GR_OPENGL:	  strcat(str, " OGL"); break;
	}

	extern int Cmdline_nohtl;
	if(!Cmdline_nohtl) strcat(str, " HT&L"); 

//XSTR:ON
	/*
	HMODULE hMod;
	DWORD bogus_handle;
	char myname[_MAX_PATH];
	int namelen, major, minor, build, waste;
	unsigned int buf_size;
	DWORD version_size;
	char *infop;
	VOID *bufp;
	BOOL result;

	// Find my EXE file name
	hMod = GetModuleHandle(NULL);
	namelen = GetModuleFileName( hMod, myname, _MAX_PATH );

	version_size = GetFileVersionInfoSize(myname, &bogus_handle );
	infop = (char *)vm_malloc(version_size);
	result = GetFileVersionInfo( myname, 0, version_size, (LPVOID)infop );

	// get the product version
	result = VerQueryValue((LPVOID)infop, TEXT("\\StringFileInfo\\040904b0\\ProductVersion"), &bufp, &buf_size );
	sscanf( (char *)bufp, "%d, %d, %d, %d", &major, &minor, &build, &waste );
#ifdef DEMO
	sprintf(str,"Dv%d.%02d",major, minor);
#else
	sprintf(str,"v%d.%02d",major, minor);
#endif
	*/
}

void get_version_string_short(char *str)
{
	sprintf(str,"v%d.%d", FS_VERSION_MAJOR, FS_VERSION_MINOR);
}

// ----------------------------------------------------------------
//
// OEM UPSELL SCREENS BEGIN
//
// ----------------------------------------------------------------
#if defined(OEM_BUILD)

#define NUM_OEM_UPSELL_SCREENS				3
#define OEM_UPSELL_SCREEN_DELAY				10000

static int Oem_upsell_bitmaps_loaded = 0;
static int Oem_upsell_bitmaps[GR_NUM_RESOLUTIONS][NUM_OEM_UPSELL_SCREENS];
static int Oem_upsell_screen_number = 0;
static int Oem_upsell_show_next_bitmap_time;

//XSTR:OFF
static char *Oem_upsell_bitmap_filenames[GR_NUM_RESOLUTIONS][NUM_OEM_UPSELL_SCREENS] = 
{
	{	"OEMUpSell02",
		"OEMUpSell01",
		"OEMUpSell03",
	},
	{	"2_OEMUpSell02",
		"2_OEMUpSell01",
		"2_OEMUpSell03",
	},
};
//XSTR:ON

static int Oem_normal_cursor = -1;
static int Oem_web_cursor = -1;
//#define OEM_UPSELL_URL		"http://www.interplay-store.com/"
#define OEM_UPSELL_URL		"http://www.interplay.com/cgi-bin/oemlinks.pl/pid=483421&cid=18384"

void oem_upsell_next_screen()
{
	Oem_upsell_screen_number++;
	if ( Oem_upsell_screen_number == (NUM_OEM_UPSELL_SCREENS-1) ) {
		// extra long delay, mouse shown on last upsell
		Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY*2;
		Mouse_hidden = 0;

	} else {
		Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY;
	}
}

void oem_upsell_load_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_OEM_UPSELL_SCREENS; i++ ) {
		Oem_upsell_bitmaps[gr_screen.res][i] = bm_load(Oem_upsell_bitmap_filenames[gr_screen.res][i]);
	}
}

void oem_upsell_unload_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_OEM_UPSELL_SCREENS; i++ ) {
		if(Oem_upsell_bitmaps[gr_screen.res][i] >= 0){
			bm_release(Oem_upsell_bitmaps[gr_screen.res][i]);
		}
	}

	// unloaded
	Oem_upsell_bitmaps_loaded = 0;
}

// clickable hotspot on 3rd OEM upsell screen
static int Oem_upsell3_button_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		28, 350, 287, 96					// x, y, w, h
	},
	{	// GR_1024
		45, 561, 460, 152					// x, y, w, h
	}
};

void oem_upsell_show_screens()
{
	int current_time, k;
	int done = 0;

	if ( !Oem_upsell_bitmaps_loaded ) {
		oem_upsell_load_bitmaps();
		Oem_upsell_bitmaps_loaded = 1;
	}

	// may use upsell screens more than once
	Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY;
	Oem_upsell_screen_number = 0;
	
	key_flush();
	Mouse_hidden = 1;

	// set up cursors
	int nframes;						// used to pass, not really needed (should be 1)
	Oem_normal_cursor = gr_get_cursor_bitmap();
	Oem_web_cursor = bm_load_animation("cursorweb", &nframes);
	Assert(Oem_web_cursor >= 0);
	if (Oem_web_cursor < 0) {
		Oem_web_cursor = Oem_normal_cursor;
	}

	while(!done) {

		//oem_reset_trailer_timer();

		current_time = timer_get_milliseconds();

		os_poll();
		k = key_inkey();

		// advance screen on keypress or timeout
		if (( k > 0 ) || (mouse_up_count(MOUSE_LEFT_BUTTON) > 0) || (current_time > Oem_upsell_show_next_bitmap_time)) {
			oem_upsell_next_screen();
		}

		// check if we are done
		if ( Oem_upsell_screen_number >= NUM_OEM_UPSELL_SCREENS ) {
			Oem_upsell_screen_number--;
			done = 1;
		} else {
			if ( Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number] < 0 ) {
				done = 1;
			}
		}

		// show me the upsell
		if ( Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number] >= 0 ) {		
			gr_set_bitmap(Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number]);
			gr_bitmap(0,0);
		}

		// if this is the 3rd upsell, make it clickable, d00d
		if ( Oem_upsell_screen_number == NUM_OEM_UPSELL_SCREENS-1 ) {
			int mx, my;
			int button_state = mouse_get_pos_unscaled(&mx, &my);
			if ( (mx >= Oem_upsell3_button_coords[gr_screen.res][0]) && (mx <= Oem_upsell3_button_coords[gr_screen.res][0] + Oem_upsell3_button_coords[gr_screen.res][2])
				&& (my >= Oem_upsell3_button_coords[gr_screen.res][1]) && (my <= Oem_upsell3_button_coords[gr_screen.res][1] + Oem_upsell3_button_coords[gr_screen.res][3]) )
			{
				// switch cursors
				gr_set_cursor_bitmap(Oem_web_cursor); //, GR_CURSOR_LOCK);

				// check for clicks
				if (button_state & MOUSE_LEFT_BUTTON) {
					// launch URL
					multi_pxo_url(OEM_UPSELL_URL);
					done = 1;
				} 
			} else {
				// switch cursor back to normal one
				gr_set_cursor_bitmap(Oem_normal_cursor); //, GR_CURSOR_UNLOCK);
			}
		}

		if ( done ) {
			if (gameseq_get_state() != GS_STATE_END_DEMO) {
				gr_fade_out(0);
				Sleep(300);
			}
		}

		gr_flip();
	}

	// unload bitmap
	oem_upsell_unload_bitmaps();

	// switch cursor back to normal one
	gr_set_cursor_bitmap(Oem_normal_cursor); //, GR_CURSOR_UNLOCK);

}

#endif // defined(OEM_BUILD)
// ----------------------------------------------------------------
//
// OEM UPSELL SCREENS END
//
// ----------------------------------------------------------------



// ----------------------------------------------------------------
//
// DEMO UPSELL SCREENS BEGIN
//
// ----------------------------------------------------------------

#ifdef FS2_DEMO

//#define NUM_DEMO_UPSELL_SCREENS				4

#define NUM_DEMO_UPSELL_SCREENS				2
#define DEMO_UPSELL_SCREEN_DELAY				3000

static int Demo_upsell_bitmaps_loaded = 0;
static int Demo_upsell_bitmaps[GR_NUM_RESOLUTIONS][NUM_DEMO_UPSELL_SCREENS];
static int Demo_upsell_screen_number = 0;
static int Demo_upsell_show_next_bitmap_time;

//XSTR:OFF
static char *Demo_upsell_bitmap_filenames[GR_NUM_RESOLUTIONS][NUM_DEMO_UPSELL_SCREENS] = 
{
	{	"UpSell02",
		"UpSell01",
	},
	{	"2_UpSell02",
		"2_UpSell01",
	},
	// "DemoUpsell3",
	// "DemoUpsell4",
};
//XSTR:ON

void demo_upsell_next_screen()
{
	Demo_upsell_screen_number++;
	if ( Demo_upsell_screen_number == (NUM_DEMO_UPSELL_SCREENS-1) ) {
		Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY*4;
	} else {
		Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY;
	}

	/*
	if ( Demo_upsell_screen_number < NUM_DEMO_UPSELL_SCREENS ) {
		if ( Demo_upsell_bitmap_filenames[gr_screen.res][Demo_upsell_screen_number] >= 0 ) {
#ifndef HARDWARE_ONLY
			palette_use_bm_palette(Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number]);
#endif
		}
	}
	*/
}

void demo_upsell_load_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_DEMO_UPSELL_SCREENS; i++ ) {
		Demo_upsell_bitmaps[gr_screen.res][i] = bm_load(Demo_upsell_bitmap_filenames[gr_screen.res][i]);
	}
}

void demo_upsell_unload_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_DEMO_UPSELL_SCREENS; i++ ) {
		if(Demo_upsell_bitmaps[gr_screen.res][i] >= 0){
			bm_release(Demo_upsell_bitmaps[gr_screen.res][i]);
		}
	}

	// unloaded
	Demo_upsell_bitmaps_loaded = 0;
}

void demo_upsell_show_screens()
{
	int current_time, k;
	int done = 0;

	if ( !Demo_upsell_bitmaps_loaded ) {
		demo_upsell_load_bitmaps();
		Demo_upsell_bitmaps_loaded = 1;
	}

	// may use upsell screens more than once
	Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY;
	Demo_upsell_screen_number = 0;
	
	key_flush();
	Mouse_hidden = 1;

	while(!done) {

		demo_reset_trailer_timer();

		current_time = timer_get_milliseconds();

// #ifndef THREADED
		os_poll();
// #endif
		k = key_inkey();

		// don't time out, wait for keypress
		/*
		if ( current_time > Demo_upsell_show_next_bitmap_time ) {
			demo_upsell_next_screen();
			k = 0;
		}*/

		if ( k > 0 ) {
			demo_upsell_next_screen();
		}

		if ( Demo_upsell_screen_number >= NUM_DEMO_UPSELL_SCREENS ) {
			Demo_upsell_screen_number--;
			done = 1;
		} else {
			if ( Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number] < 0 ) {
				done = 1;
			}
		}

		if ( Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number] >= 0 ) {		
			gr_set_bitmap(Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number]);
			gr_bitmap(0,0);
		}

		if ( done ) {
			if (gameseq_get_state() != GS_STATE_END_DEMO) {
				gr_fade_out(0);
				Sleep(300);
			}
		}

		gr_flip();
	}

	// unload bitmap
	demo_upsell_unload_bitmaps();
}

#endif // DEMO

// ----------------------------------------------------------------
//
// DEMO UPSELL SCREENS END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
//
// Subspace Ambient Sound START
//
// ----------------------------------------------------------------

static int Subspace_ambient_left_channel = -1;
static int Subspace_ambient_right_channel = -1;

// 
void game_start_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel < 0 ) {
		Subspace_ambient_left_channel = snd_play_looping(&Snds[SND_SUBSPACE_LEFT_CHANNEL], -1.0f);
	}

	if ( Subspace_ambient_right_channel < 0 ) {
		Subspace_ambient_right_channel = snd_play_looping(&Snds[SND_SUBSPACE_RIGHT_CHANNEL], 1.0f);
	}
}

void game_stop_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel >= 0 ) {
		snd_stop(Subspace_ambient_left_channel);
		Subspace_ambient_left_channel = -1;
	}

	if ( Subspace_ambient_right_channel >= 0 ) {
		snd_stop(Subspace_ambient_right_channel);
		Subspace_ambient_right_channel = -1;
	}
}

// ----------------------------------------------------------------
//
// Subspace Ambient Sound END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
//
// CDROM detection code START
//
// ----------------------------------------------------------------

#define CD_SIZE_72_MINUTE_MAX			(697000000)

uint game_get_cd_used_space(char *path)
{
#ifdef _WIN32
	uint total = 0;
	char use_path[512] = "";
	char sub_path[512] = "";
	WIN32_FIND_DATA	find;
	HANDLE find_handle;

	// recurse through all files and directories
	strcpy(use_path, path);
	strcat(use_path, "*.*");
	find_handle = FindFirstFile(use_path, &find);

	// bogus
	if(find_handle == INVALID_HANDLE_VALUE){
		return 0;
	}	

	// whee
	do {
		// subdirectory. make sure to ignore . and ..
		if((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && stricmp(find.cFileName, ".") && stricmp(find.cFileName, "..")){
			// subsearch
			strcpy(sub_path, path);
			strcat(sub_path, find.cFileName);
			strcat(sub_path, DIR_SEPARATOR_STR);
			total += game_get_cd_used_space(sub_path);	
		} else {
			total += (uint)find.nFileSizeLow;
		}				
	} while(FindNextFile(find_handle, &find));	

	// close
	FindClose(find_handle);

	// total
	return total;
#else
	if (path == NULL) {
		// bail
		mprintf(("NULL path passed to game_get_cd_used_space.\n"));
		return 0;
	}

	STUB_FUNCTION;

	return 0;
#endif // _WIN32
}


// if volume_name is non-null, the CD name must match that
int find_freespace_cd(char *volume_name)
{
#ifdef _WIN32
	char oldpath[MAX_PATH];
	char volume[256];
	int i;
	int cdrom_drive=-1;
	int volume_match = 0;
	_finddata_t find;
	int find_handle;

	GetCurrentDirectory(MAX_PATH, oldpath);

	for (i = 0; i < 26; i++) 
	{
//XSTR:OFF
		char path[]="d:\\";
//XSTR:ON

		path[0] = (char)('A'+i);
		if (GetDriveType(path) == DRIVE_CDROM) {
			cdrom_drive = -3;
			if ( GetVolumeInformation(path, volume, 256, NULL, NULL, NULL, NULL, 0) == TRUE ) {
				nprintf(("CD", "CD volume: %s\n", volume));
			
				// check for any CD volume
				int volume1_present = 0;
				int volume2_present = 0;
				int volume3_present = 0;		

				char full_check[512] = "";

				// look for setup.exe
				strcpy(full_check, path);
				strcat(full_check, "setup.exe");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume1_present = 1;				
					_findclose(find_handle);				
				}

				// look for intro.mve
				strcpy(full_check, path);
				strcat(full_check, "intro.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume2_present = 1;
					_findclose(find_handle);						
				}				

				// look for endpart1.mve
				strcpy(full_check, path);
				strcat(full_check, "endpart1.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume3_present = 1;
					_findclose(find_handle);				
				}				
			
				// see if we have the specific CD we're looking for
				if ( volume_name ) {
					// volume 1
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_1) && volume1_present) {
						volume_match = 1;
					}
					// volume 2
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_2) && volume2_present) {
						volume_match = 1;
					}
					// volume 3
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_3) && volume3_present) {
						volume_match = 1;
					}
				} else {										
					if ( volume1_present || volume2_present || volume3_present ) {
						volume_match = 1;
					}
				}
				
				// here's where we make sure that CD's 2 and 3 are not just ripped - check to make sure its capacity is > 697,000,000 bytes				
				if ( volume_match ){
#ifdef RELEASE_REAL					
					// we don't care about CD1 though. let it be whatever size it wants, since the game will demand CD's 2 and 3 at the proper time
					if(volume2_present || volume3_present) {
						// first step - check to make sure its a cdrom
						if(GetDriveType(path) != DRIVE_CDROM){							
							break;
						}

#if !defined(OEM_BUILD)
						// oem not on 80 min cds, so dont check tha size
						// check its size
						uint used_space = game_get_cd_used_space(path);											
						if(used_space < CD_SIZE_72_MINUTE_MAX){							
							break;
						}
#endif // !defined(OEM_BUILD)
					}					

					cdrom_drive = i;
					break;
#else
					cdrom_drive = i;
					break;
#endif // RELEASE_REAL
				}
			}
		}
	}	

	SetCurrentDirectory(oldpath);
	return cdrom_drive;
#else
	STUB_FUNCTION;

	if (volume_name != NULL) {
		// volume specific checks
		STUB_FUNCTION;
	}

	return 0;
#endif // _WIN32
}

int set_cdrom_path(int drive_num)
{
	int rval;

	if (drive_num < 0) {			//no CD
//		#ifndef NDEBUG
//		strcpy(CDROM_dir,"j:\\FreeSpaceCD\\");				//set directory
//		rval = 1;
//		#else
		strcpy(Game_CDROM_dir,"");				//set directory
		rval = 0;
//		#endif
	} else {
		sprintf(Game_CDROM_dir,NOX("%c:\\"), 'a' + drive_num );			//set directory
		rval = 1;
	}

	return rval;
}

int init_cdrom()
{
	int i, rval;

	//scan for CD, etc.

	rval = 1;

	#ifndef DEMO
	i = find_freespace_cd();

	rval = set_cdrom_path(i);

	/*
	if ( rval ) {
		nprintf(("CD", "Using %s for FreeSpace CD\n", CDROM_dir));
	} else {
		nprintf(("CD", "FreeSpace CD not found\n"));
	}
	*/
	#endif

	return rval;
}

int Last_cd_label_found = 0;
char Last_cd_label[256];

int game_cd_changed()
{
#ifdef _WIN32
	char label[256];
	int found;
	int changed = 0;
	
	if ( strlen(Game_CDROM_dir) == 0 ) {
		init_cdrom();
	}

	found = GetVolumeInformation(Game_CDROM_dir, label, 256, NULL, NULL, NULL, NULL, 0);

	if ( found != Last_cd_label_found )	{
		Last_cd_label_found = found;
		if ( found )	{
			mprintf(( "CD '%s' was inserted\n", label ));
			changed = 1;
		} else {
			mprintf(( "CD '%s' was removed\n", Last_cd_label ));
			changed = 1;
		}
	} else {
		if ( Last_cd_label_found )	{
			if ( !stricmp( Last_cd_label, label ))	{
				//mprintf(( "CD didn't change\n" ));
			} else {
				mprintf(( "CD was changed from '%s' to '%s'\n", Last_cd_label, label ));
				changed = 1;
			}
		} else {
			// none found before, none found now.
			//mprintf(( "still no CD...\n" ));
		}
	}
	
	Last_cd_label_found = found;
	if ( found )	{
		strcpy( Last_cd_label, label );
	} else {
		strcpy( Last_cd_label, "" );
	}

	return changed;
#else
	STUB_FUNCTION;

	return 0;
#endif // _WIN32
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check(char *volume_name)
{	
#if !defined(GAME_CD_CHECK)
	return 1;
#else
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		// standalone mode
		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "FreeSpace 2 CD not found\n\nInsert a FreeSpace 2 CD to continue", 202));
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
#endif
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check_specific(char *volume_name, int cdnum)
{	
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
#if defined(DVD_MESSAGE_HACK)
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert DVD", 1468));
#else
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert CD %d", 1468), cdnum);
#endif
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
}

// only need to do this in RELEASE_REAL
int game_do_cd_mission_check(char *filename)
{	
#ifdef RELEASE_REAL
/*	int cd_num;
	int cd_present = 0;
	int cd_drive_num;
	fs_builtin_mission *m = game_find_builtin_mission(filename);

	// check for changed CD
	if(game_cd_changed()){
		cfile_refresh();
	}

	// multiplayer
	if((Game_mode & GM_MULTIPLAYER) || Is_standalone){
		return 1;
	}

	// not builtin, so do a general check (any FS2 CD will do)
	if(m == NULL){
		return game_do_cd_check();
	}

	// does not have any CD requirement, do a general check
	if(strlen(m->cd_volume) <= 0){
		return game_do_cd_check();
	}

	// get the volume
	if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_1)){
		cd_num = 1;
	} else if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_2)){
		cd_num = 2;
	} else if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_3)){
		cd_num = 3; 
	} else {
		return game_do_cd_check();
	}

	// did we find the cd?
	if(find_freespace_cd(m->cd_volume) >= 0){
		return 1;
	}

	// make sure the volume exists
	int num_attempts = 0;
	int refresh_files = 0;
	while(1){
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(m->cd_volume);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		// no CD found, so prompt user
#if defined(DVD_MESSAGE_HACK)
		popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert DVD", 1468));
#else
		popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert CD %d", 1468), cd_num);
#endif

		refresh_files = 1;
		if ( popup_rval != 1 ) {
			cd_present = 0;
			break;
		}

		if ( num_attempts++ > 5 ) {
			cd_present = 0;
			break;
		}
	}	

	return cd_present;
*/

	return 1;

#else
	return 1;
#endif // RELEASE_REAL
}

// ----------------------------------------------------------------
//
// CDROM detection code END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
// Language autodetection stuff
//

// this layout *must* match Lcl_languages in localize.cpp in order for the
// correct language to be detected
int Lang_auto_detect_checksums[LCL_NUM_LANGUAGES] = {
	589986744,						// English
	-1132430286,					// German
	0,								// French
};

// default setting is "-1" to use registry setting with English as fall back
// DO NOT change that default setting here or something uncouth might happen
// int the localization code
int detect_lang()
{
	uint file_checksum;
	int idx;

	// if the reg is set then let lcl_init() figure out what to do
	if (os_config_read_string( NULL, NOX("Language"), NULL ) != NULL)
		return -1;

	// try and open the file to verify
	CFILE *detect = cfopen("font01.vf", "rb");

	// will use default setting if something went wrong
	if (!detect)
		return -1;

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;

	// now compare the checksum/filesize against known #'s
	for (idx=0; idx<LCL_NUM_LANGUAGES; idx++) {
		if (Lang_auto_detect_checksums[idx] == (int)file_checksum) {
			mprintf(( "AutoLang: Language auto-detection successful...\n" ));
			return idx;
		}
	}

	// notify if a match was not found, include detected checksum
	mprintf(( "ERROR: Unknown Language Checksum: %i\n", (int)file_checksum ));
	mprintf(( "Using default language settings...\n" ));

	return -1;
}

//
// Eng Auto Lang stuff
// ----------------------------------------------------------------


// ----------------------------------------------------------------
// SHIPS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_SHIPS_TBL_CHECKSUMS		1

#ifdef FS2_DEMO
int Game_ships_tbl_checksums[NUM_SHIPS_TBL_CHECKSUMS] = {
	//-463907578,						// US - beta 1
	1696074201,						// FS2 demo
};
#else
int Game_ships_tbl_checksums[NUM_SHIPS_TBL_CHECKSUMS] = {
//	-1022810006,					// 1.0 FULL
	-1254285366						// 1.2 FULL (German)
};
#endif

void verify_ships_tbl()
{	
#if defined(NDEBUG) || defined(NO_NETWORK)
	Game_ships_tbl_valid = 1;
#else
	// this is still needed for LAN play but skip it if we are using FS2NetD
	if (!Om_tracker_flag) {
		Game_ships_tbl_valid = 1;
	} else {
		uint file_checksum;		
		int idx;

		// detect if the packfile exists
		CFILE *detect = cfopen("ships.tbl", "rb");
		Game_ships_tbl_valid = 0;	 
	
		// not mission-disk
		if(!detect){
			Game_ships_tbl_valid = 0;
			return;
		}	

		// get the long checksum of the file
		file_checksum = 0;
		cfseek(detect, 0, SEEK_SET);	
		cf_chksum_long(detect, &file_checksum);
		cfclose(detect);
		detect = NULL;	

		// now compare the checksum/filesize against known #'s
		for(idx=0; idx<NUM_SHIPS_TBL_CHECKSUMS; idx++){
			if(Game_ships_tbl_checksums[idx] == (int)file_checksum){
				Game_ships_tbl_valid = 1;
				return;
			}
		}
	}
#endif
}

DCF(shipspew, "display the checksum for the current ships.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("ships.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// ----------------------------------------------------------------
// WEAPONS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_WEAPONS_TBL_CHECKSUMS		1

#ifdef FS2_DEMO
int Game_weapons_tbl_checksums[NUM_WEAPONS_TBL_CHECKSUMS] = {
	//141718090,				// US - beta 1
	-266420030,				// demo 1
};
#else
int Game_weapons_tbl_checksums[NUM_WEAPONS_TBL_CHECKSUMS] = {
//	399297860,				// 1.0 FULL	
	-553984927				// 1.2 FULL (german)
};
#endif

void verify_weapons_tbl()
{
	// detect if the packfile exists
	CFILE *detect = cfopen("weapons.tbl", "rb");

	// not mission-disk
	if (!detect) {
		// TODO: some error message
		Weapon_tbl_checksum = 0;
		return;
	}

	cfseek(detect, 0, SEEK_SET);
	cf_chksum_long(detect, &Weapon_tbl_checksum);
	cfclose(detect);
	detect = NULL;

#if defined(NDEBUG) || defined(NO_NETWORK)
	Game_weapons_tbl_valid = 1;
#else
	// this is still needed for LAN play but skip it if we are using FS2NetD
	if (!Om_tracker_flag) { //!Om_tracker_flag means we're not playing on PXO
		Game_weapons_tbl_valid = 1;
	} else {
		int idx;

		// now compare the checksum/filesize against known #'s
		for(idx=0; idx<NUM_WEAPONS_TBL_CHECKSUMS; idx++){
			if(Game_weapons_tbl_checksums[idx] == (int)Weapon_tbl_checksum){
				Game_weapons_tbl_valid = 1;
				return;
			}
		}
	}
#endif
}

DCF(wepspew, "display the checksum for the current weapons.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("weapons.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// if the game is running using hacked data
void multi_update_valid_tables(); // in multiutil.obj
extern bool modular_tables_loaded;
extern bool module_ship_weapons_loaded;

int game_hacked_data()
{
	int retval = 0;

#ifndef NO_NETWORK
	//omfg modular tables
	if(module_ship_weapons_loaded)
	{
		return 1;
	}

	// LAN game, only check if weapon and ship are valid since we can't or don't
	// won't to validate against PXO server
	if (!Om_tracker_flag) {
		if (!Game_weapons_tbl_valid || !Game_ships_tbl_valid)
			retval = 1;
		
		return retval;
	}

	if (!cf_exist( "tvalid.cfg", CF_TYPE_DATA ))
	{
		// create the tvalid.cfg
		multi_update_valid_tables();
	}

	CFILE *tvalid_cfg = cfopen("tvalid.cfg", "rt", CFILE_NORMAL, CF_TYPE_DATA);

	// if it's null then multi_update_valid_tables() didn't do it's job correctly
	if (tvalid_cfg == NULL) {
		ml_printf("Can't open tvalid.cfg for crc verification, assuming HACKED!\n");
		return 1; // assume hacked
	}

	char *buffer = new char[cfilelength(tvalid_cfg)];
	cfread(buffer, 1, cfilelength(tvalid_cfg), tvalid_cfg);	
	
	if (strstr(buffer, "invalid")) // got hacked data
		retval = 1;

	if (buffer != NULL)
		delete[] buffer;

	if (tvalid_cfg != NULL)
		cfclose(tvalid_cfg);
#endif

	// not hacked
	return retval;
}


//#define MAX_SPLASHSCREENS 64
//char Splash_screens[MAX_SPLASHSCREENS][MAX_FILENAME_LEN];


void display_title_screen()
{
	int title_bitmap = -1;
	int title_logo =-1;
/*	_finddata_t find;
	long		find_handle;
	char current_dir[256];

	//Get the find string
	_getcwd(current_dir, 256);
	strcat(current_dir, DIR_SEPARATOR_STR);
	strcat(current_dir, "*.pcx");

	//Let the search begin!
	find_handle = _findfirst(current_dir, &find);
	int i = 0;
	if(find_handle != -1)
	{
		char *p;

		do {
			if(!(find.attrib & _A_SUBDIR) && (strlen(find.name) < MAX_FILENAME_LEN)) {
				p = strchr( find.name, '.' );
				if(p) {
					*p = '\0';
				}

				if(stricmp(find.name, Game_logo_screen_fname[gr_screen.res])
					&& stricmp(find.name, Game_title_screen_fname[gr_screen.res]))
				{
					strcpy(Splash_screens[i], find.name);
					i++;
				}

				if(i == MAX_SPLASHSCREENS) {
					break;
				}
			}
		} while(!_findnext(find_handle, &find));
	}

	if(i) {
		srand(time(NULL));
		title_bitmap = bm_load(Splash_screens[rand() % i]);

	} else {
		title_bitmap = bm_load(Game_title_screen_fname[gr_screen.res]);
	}
	
	if (title_bitmap == -1 && title_logo == -1) {
//		return;
	}
	*/
#ifndef NO_DIRECT3D
	// d3d	
	if(gr_screen.mode == GR_DIRECT3D){
		extern void d3d_start_frame();
		d3d_start_frame();
	}
#endif

	title_logo = bm_load(Game_logo_screen_fname[gr_screen.res]);
	title_bitmap = bm_load(Game_title_screen_fname[gr_screen.res]);

	if(title_bitmap != -1)
	{
		// set
		gr_set_bitmap(title_bitmap);

		//Get bitmap's width and height
		int width, height;
		bm_get_info(title_bitmap, &width, &height);

		//Draw it in the center of the screen
		gr_bitmap((gr_screen.max_w_unscaled - width)/2, (gr_screen.max_h_unscaled - height)/2);
	}

	if(title_logo != -1)
	{
		gr_set_bitmap(title_logo);

		gr_bitmap(0,0);

	}

#ifndef NO_DIRECT3D
	// d3d	
	if(gr_screen.mode == GR_DIRECT3D){
		extern void d3d_stop_frame();
		d3d_stop_frame();
	}
#endif

	// flip
	gr_flip();

//	bm_set_render_target(-1);//big_ole_honkin_hack_test
	if(title_bitmap != -1)
	{
		bm_release(title_bitmap);
	}

	if(title_logo != -1)
	{
		bm_release(title_logo);
	}
}

// return true if the game is running with "low memory", which is less than 48MB
bool game_using_low_mem()
{
	if (Use_low_mem == 0) {
		return false;
	} else {
		return true;
	}
}

//  place calls here that need to take effect immediately when the game is
//  minimized.  Called from osapi.cpp 
void game_pause()
{
	// Protection against flipping out -- Kazan
	if (!GameState_Stack_Valid())
		return;

	if (!(Game_mode & GM_MULTIPLAYER)){
		switch ( gameseq_get_state() )
		{
			case GS_STATE_MAIN_MENU:
				main_hall_pause(); // not an instant shutoff of misc anims and sounds
				break;

			case GS_STATE_BRIEFING:
				brief_pause();
				break;

			case GS_STATE_DEBRIEF:
				debrief_pause();
				break;

			case GS_STATE_CMD_BRIEF:
				cmd_brief_pause();
				break;

			case GS_STATE_RED_ALERT:
				red_alert_voice_pause();
				break;

			// anything that would leave the ambient mainhall sound going
			case GS_STATE_TECH_MENU:
			case GS_STATE_BARRACKS_MENU:
				main_hall_stop_ambient();
				main_hall_stop_music(); // not an instant shutoff
				break;

			// things that would get music except if they are called while in-mission
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_HUD_CONFIG:
				if ( !(Game_mode & GM_IN_MISSION) ) {
					main_hall_stop_ambient();
					main_hall_stop_music(); // not an instant shutoff
				}
				break;

			// only has the ambient sound, no music
			case GS_STATE_INITIAL_PLAYER_SELECT:
				main_hall_stop_ambient();
				break;

			// pause_init is a special case and we don't unpause it ourselves
			case GS_STATE_GAME_PLAY:
				if ( (!popup_active()) && (!popupdead_is_active()) )
					pause_init(0);
				break;

			default:
				audiostream_pause_all();
		}
	}
}

// calls to be executed when the game is restored from minimized or inactive state
void game_unpause()
{
	if (!GameState_Stack_Valid())
		return;

	// automatically recover from everything but an in-mission pause
	if (!(Game_mode & GM_MULTIPLAYER)) {
		switch ( gameseq_get_state() )
		{
			case GS_STATE_MAIN_MENU:
				main_hall_unpause();
				break;

			case GS_STATE_BRIEFING:
				brief_unpause();
				break;

			case GS_STATE_DEBRIEF:
				debrief_unpause();
				break;

			case GS_STATE_CMD_BRIEF:
				cmd_brief_unpause();
				break;

			case GS_STATE_RED_ALERT:
				red_alert_voice_unpause();
				break;

			// anything that would leave the ambient mainhall sound going
			case GS_STATE_TECH_MENU:
			case GS_STATE_BARRACKS_MENU:
				main_hall_start_ambient();
				main_hall_start_music(); // not an instant shutoff
				break;

			// things that would get music except if they are called while in-mission
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_HUD_CONFIG:
				if ( !(Game_mode & GM_IN_MISSION) ) {
					main_hall_start_ambient();
					main_hall_start_music(); // not an instant shutoff
				}
				break;

			// only has the ambient sound, no music
			case GS_STATE_INITIAL_PLAYER_SELECT:
				main_hall_start_ambient();
				break;

			// if in a game then do nothing, pause_init() should have been called
			// and will get cleaned up elsewhere
			case GS_STATE_GAME_PLAY:
				break;

			default:
				audiostream_unpause_all();
		}
	}
}
