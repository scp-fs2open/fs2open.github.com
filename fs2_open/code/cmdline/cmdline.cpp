/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Cmdline/cmdline.cpp $
 * $Revision: 2.141 $
 * $Date: 2006-06-15 00:37:11 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.140  2006/05/27 17:18:56  taylor
 * d'oh!  that was supposed to be off by default!
 *
 * Revision 2.139  2006/05/27 17:17:57  taylor
 * few things:
 *   - comment out that -fixbugs and -nocrash crap, that's just stupid
 *   - move -env off of the experimental list (since it's working pretty well in OGL)
 *   - remove -nobeampierce
 *   - remove -max_subdivide (didn't do anything)
 *   - remove -rt (didn't do anything, and per-coder cmdline options should never be in CVS)
 *   - move -cache_bitmaps to game speed rather than graphics
 *   - add options for env and spec map scaling for envmaps (temporary, to be removed before official 3.6.8)
 *   - add option for new alpha blend mode so that artists can still test with it but not mess up normal users with the bad MediaVPs
 *   - remove -d3d_particle (it's obsolete now)
 *
 * Revision 2.138  2006/05/13 07:29:51  taylor
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
 * Revision 2.137  2006/05/04 21:25:12  phreak
 * the -decal command line doesn't enable decals anymore, just pops up a messagebox in windows saying that decals are disabled from now on.
 *
 * Revision 2.136  2006/04/05 13:47:01  taylor
 * remove -tga16, it's obsolete now
 * add a temporary -no_emissive_light option to not use emission type light in OGL
 *
 * Revision 2.135  2006/03/18 10:17:58  taylor
 * we already have a variable to show the framerate so lets just use the one
 *
 * Revision 2.134  2006/02/24 07:36:49  taylor
 * try and make some sense out of that cmdline option mess, maybe it will stay sane for a few days at least :)
 *
 * Revision 2.133  2006/02/01 23:35:31  phreak
 * -ingame should be -ingame_join
 *
 * Revision 2.132  2006/01/20 07:10:33  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 2.131  2006/01/10 18:37:45  randomtiger
 * Improvements to voice recognition system.
 * Also function put on -voicer launcher option.
 *
 * Revision 2.130  2005/12/14 17:58:26  taylor
 * should have tested that better, it was still finding or ignoring options that it shouldn't if they were in the wrong order on
 *  the same source (source being the two possible cmdline_fso.cfg files and the actual args passed on the cmdline)
 *
 * Revision 2.129  2005/12/07 05:38:32  taylor
 * make sure with cmdline option check that it's the actual option (-spec was getting picked out of the -spec_* options by mistake)
 *
 * Revision 2.128  2005/12/06 03:17:48  taylor
 * cleanup some debug log messages:
 *   note that a nprintf() with "Warning" or "General" is basically the same thing as mprintf()
 *   make sure that OpenAL init failures always get to the debug log
 *
 * Revision 2.127  2005/11/24 06:46:39  phreak
 * Added wiki help for -rearm_timer and -missile_lighting
 *
 * Revision 2.126  2005/11/24 06:37:47  phreak
 * Added -missile_lighting command line
 *
 * Revision 2.125  2005/11/23 00:43:20  phreak
 * Command lines for rearm countdown timer.
 *
 * Revision 2.124  2005/11/21 00:46:06  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.123  2005/11/18 08:03:23  Goober5000
 * Wiki links updated; many thanks to StratComm :)
 * --Goober5000
 *
 * Revision 2.122  2005/11/13 06:55:38  taylor
 * cmdline option cleanup:
 * remove from launcher -pcx32, -cell, -

 * remove from game -loadonlyused, -dxt, -pcx2dds
 * rename -d3dmipmap, -d3d_no_vsync, -dnoshowvid to non API specific sounding names
 * add -nograb into help output under Linux/OSX
 * add -loadallweps and -img2dds
 *
 * Revision 2.121  2005/10/30 20:00:22  taylor
 * same basic cleanup and self-sanity changes
 * split up WinMain() and main() so it doesn't resemble ifdef hell
 * rename WinMainSub() to game_main() and move anything that should have been in WinMain() to WinMain()
 *
 * Revision 2.120  2005/10/30 18:19:58  wmcoolmon
 * Fix scripting.html
 *
 * Revision 2.119  2005/10/30 06:44:56  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.118  2005/10/24 04:48:14  taylor
 * not sure what I was smoking there, max shininess value is 128, not 180
 *
 * Revision 2.117  2005/10/23 20:34:29  taylor
 * some cleanup, fix some general memory leaks, safety stuff and whatever else Valgrind complained about
 *
 * Revision 2.116  2005/10/23 11:45:06  taylor
 * add -ogl_shine to adjust the OGL shininess value so that people can play around and find the best value to use
 *
 * Revision 2.115  2005/10/22 04:28:16  unknownplayer
 * Added -UseNewAI command line option to force the game to always use
 * the SCP AI changes. As of now there's some problem in gr_d3d_set_render_target
 * that crashes the game when it gets to a mission.
 *
 * Revision 2.114  2005/10/12 05:43:40  taylor
 * temporary cmdline option, -y_bug_fix, to switch between original code (default) and previous attempt at the Y-bug fix
 *
 * Revision 2.113  2005/09/30 09:47:06  taylor
 * remove -rlm, it's always on now since there was never a complaint and pretty much everyone uses it
 * add -cache_bitmaps and have bitmap caching between levels off by default
 * when -cache_bitmaps is used then use C-BMP for top-right memory listing, and just BMP otherwise
 *
 * Revision 2.112  2005/09/25 08:23:38  Goober5000
 * remove unneeded #include
 * --Goober5000
 *
 * Revision 2.111  2005/09/24 02:40:09  Goober5000
 * get rid of a whole bunch of Microsoft warnings
 * --Goober5000
 *
 * Revision 2.110  2005/09/21 03:55:31  Goober5000
 * add option for warp flash; mess with the cmdlines a bit
 * --Goober5000
 *
 * Revision 2.109  2005/09/05 09:33:08  taylor
 * merge of OSX tree
 * update cmdline stuff for Linux to be less stupid and provide better feedback to the user
 *
 * Revision 2.108  2005/08/01 10:00:39  taylor
 * allow for both gamedir and userdir cmdline config files
 *
 * Revision 2.107  2005/07/25 05:24:16  Goober5000
 * cleaned up some command line and mission flag stuff
 * --Goober5000
 *
 * Revision 2.106  2005/07/07 16:32:33  taylor
 * compiler warning fixes
 * add -noibx troubleshooting cmdline option to disable use of IBX files
 * don't try to set thuster object number is only one thruster bank is specified (default method should go into affect)
 *
 * Revision 2.105  2005/06/29 18:46:13  taylor
 * add option to not scale up movies to fit window/screen, default is to scale
 * (arguably not a bug but I said I would get it in before 3.6.5 and forgot)
 *
 * Revision 2.104  2005/05/24 07:05:49  wmcoolmon
 * Commented out -fixbugs and -nocrash in preparation for 3.6.7
 *
 * Revision 2.103  2005/05/12 17:49:10  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.102  2005/05/01 07:13:59  wmcoolmon
 * -output_sexps command line
 *
 * Revision 2.101  2005/04/15 11:41:27  taylor
 * stupid <expletive-delete> terminal, I <expletive-deleted> <expletive-deleted>!!!
 *
 * Revision 2.100  2005/04/15 11:36:55  taylor
 * new GCC = new warning messages, yippeeee!!
 *
 * Revision 2.99  2005/03/25 06:57:33  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.98  2005/03/13 23:07:35  taylor
 * enable 32-bit to 16-bit TGA conversion with -tga16 cmdline option (experimental)
 * fix crash when upgrading from original campaign stats file to current
 *
 * Revision 2.97  2005/03/12 03:09:55  wmcoolmon
 * New commandline option "-noparseerrors"
 *
 * Revision 2.96  2005/03/08 03:50:19  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.95  2005/03/03 06:05:26  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.94  2005/02/16 10:00:13  wmcoolmon
 * "-ingame" command line option
 *
 * Revision 2.93  2005/02/10 04:02:37  wmcoolmon
 * Addition of the -clipdist argument.
 *
 * Revision 2.92  2005/01/30 14:09:29  taylor
 * it's -nosound not -noaudio
 *
 * Revision 2.91  2005/01/30 12:50:08  taylor
 * merge with Linux/OSX tree - p0130
 *
 * Revision 2.90  2005/01/29 16:30:47  phreak
 * smart shield command line stuff.  this will be changed in the future, but i want people to test it.
 * -phreak
 *
 * Revision 2.89  2005/01/21 08:29:04  taylor
 * add -rlm cmdline option to switch to local viewpoint lighting calculations (OGL only for now)
 *
 * Revision 2.88  2004/12/24 19:36:12  Goober5000
 * resorted command-line options and added an option for WMC's ballistic gauge
 * --Goober5000
 *
 * Revision 2.87  2004/12/11 09:37:17  wmcoolmon
 * Apparently I never commited -mpnoreturn...
 *
 * Revision 2.86  2004/12/02 11:20:33  taylor
 * text of -no_set_gamma cmdline option shouldn't specify D3D since it works in OGL too
 *
 * Revision 2.85  2004/11/29 18:04:53  taylor
 * little reorg to flags so Experimental won't show up twice
 * add/update the wiki links since none of them worked before
 * remove all options that require an argument since they won't work
 *
 * Revision 2.84  2004/11/23 19:29:13  taylor
 * fix 2d warp in D3D, add cmdline option for 3d warp
 *
 * Revision 2.83  2004/09/10 13:51:45  et1
 * Command line option for TBP for warp stuff, "-tbpwarpeffects"
 *
 * Revision 2.82  2004/08/08 02:56:54  phreak
 * fixed the -orbradar launcher string
 *
 * Revision 2.81  2004/08/02 22:40:59  phreak
 * added -orbradar to enable orb rendering for the radar
 *
 * Revision 2.80  2004/08/01 02:31:18  phreak
 * -phreak command line changed to -dualscanlines
 *
 * Revision 2.79  2004/07/29 03:49:44  Kazan
 * fs2_open.exe was suffering the same problem as fred2 -- it was just a little less obvious
 *
 * Revision 2.78  2004/07/26 20:47:25  Kazan
 * remove MCD complete
 *
 * Revision 2.77  2004/07/25 18:46:28  Kazan
 * -fred_no_warn has become -no_warn and applies to both fred and fs2
 * added new ai directive (last commit) and disabled afterburners while performing AIM_WAYPOINTS or AIM_FLY_TO_SHIP
 * fixed player ship speed bug w/ player-use-ai, now stays in formation correctly and manages speed
 * made -radar_reduce ignore itself if no parameter is given (ignoring launcher bug)
 *
 * Revision 2.76  2004/07/12 16:32:42  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.75  2004/07/05 05:09:15  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.74  2004/06/29 06:00:45  wmcoolmon
 * Added "-load_only_used", which makes FS2 load only used weapon data
 *
 * Revision 2.73  2004/06/26 00:28:05  wmcoolmon
 * Added "-targetinfo" to toggle info next to targeted object.
 *
 * Revision 2.72  2004/06/19 22:15:48  wmcoolmon
 * Added -nomotion debris command line option; also re-ordered categories.
 *
 * Revision 2.71  2004/06/06 12:25:19  randomtiger
 * Added new compression option -pcx32dds, build posted in RSB forum.
 * Changed flag because of launcher bug, have fixed launcher bug, will distribute later.
 * Also removed experimental flag from launcher flag list, stupid people were reporting bugs on unfinished code.
 *
 * Revision 2.70  2004/05/01 22:47:23  randomtiger
 * FS2_open will now take flag settings from cmdline_fso.cfg not cmdline.cfg.
 * This means retail FS2 will not crash because of fs2_open flags.
 * The new version of the launcher (v5) must used to setup flags now.
 * Anyone distributing pre 3.6 builds should make this clear and 3.6 should come with Launcher v5.
 *
 * Revision 2.69  2004/05/01 17:10:18  Kazan
 * Multiple -mod - "-mod ModA,ModB,ModC" in order of priority
 * Giving you:
 * Root: ModA
 * Root: ModB
 * Root: ModC
 * Root: Fs2Default
 * Root: CDRom
 *
 * Revision 2.68  2004/04/26 18:23:47  Kazan
 * -no_fps_capping
 *
 * Revision 2.67  2004/04/26 00:23:25  taylor
 * novbo and snd_preload cmdline options, fix moddir getting appended space
 *
 * Revision 2.66  2004/04/18 19:39:12  randomtiger
 * Added -2d_poof command which allows access to 2D poof rendering
 * Added -radar_reduce to launcher flag description structure
 *
 * Revision 2.65  2004/04/06 01:11:41  Kazan
 * make custom build work again
 *
 * Revision 2.64  2004/04/03 18:11:20  Kazan
 * FRED fixes
 *
 * Revision 2.63  2004/04/02 18:25:16  randomtiger
 * Changed D3D secondry texture system to be off by default to save mem.
 * Added some friendly names for parameters.
 *
 * Revision 2.62  2004/03/20 14:47:12  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.61  2004/03/19 14:51:54  randomtiger
 * New command line parameter: -d3d_lesstmem causes D3D to bypass V's secondry texture system.
 *
 * Revision 2.60  2004/03/19 11:44:04  randomtiger
 * Removed -d3d_notmanaged param.
 * Tided D3D texture code. Merged remaining section code into the rest of the system.
 * Prepared for removal of code causing waste of memory for secondry store of textures.
 *
 * Revision 2.59  2004/03/16 18:37:01  randomtiger
 * Added new launcher flag construction code.
 *
 * Revision 2.58  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.57  2004/02/27 04:09:55  bobboau
 * fixed a Z buffer error in HTL submodel rendering,
 * and glow points,
 * and other stuff
 *
 * Revision 2.56  2004/02/20 21:58:07  randomtiger
 * Added * to - conversion for start mission code to allow launcher missions with '-' in them.
 * Currently the parsing code counts that as a flag and messes it up.
 *
 * Revision 2.55  2004/02/20 21:45:40  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.54  2004/02/16 11:47:31  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.53  2004/02/04 10:14:25  Goober5000
 * changed spec and glow to be off by default; command lines are now -spec and -glow
 * --Goober5000
 *
 * Revision 2.52  2004/01/29 01:34:00  randomtiger
 * Added malloc montoring system, use -show_mem_usage, debug exes only to get an ingame list of heap usage.
 * Also added -d3d_notmanaged flag to activate non managed D3D path, in experimental stage.
 *
 * Revision 2.51  2004/01/24 14:31:26  randomtiger
 * Added the D3D particle code, its not bugfree but works perfectly on my card and helps with the framerate.
 * Its optional and off by default, use -d3d_particle to activiate.
 * Also bumped up D3D ambient light setting, it was way too dark.
 * Its now set to something similar to the original game.
 *
 * Revision 2.50  2003/12/08 22:30:02  randomtiger
 * Put render state and other direct D3D calls repetition check back in, provides speed boost.
 * Fixed bug that caused fullscreen only crash with DXT textures
 * Put dithering back in for tgas and jpgs
 *
 * Revision 2.49  2003/12/04 20:39:09  randomtiger
 * Added DDS image support for D3D
 * Added new command flag '-ship_choice_3d' to activate 3D models instead of ani's in ship choice, feature now off by default
 * Hopefully have fixed D3D text batching bug that caused old values to appear
 * Added Hud_target_object_factor variable to control 3D object sizes of zoom in HUD target
 * Fixed jump nodes not showing
 *
 * Revision 2.48  2003/12/03 19:27:00  randomtiger
 * Changed -t32 flag to -jpgtga
 * Added -query_flag to identify builds with speech not compiled and other problems
 * Now loads up launcher if videocard reg entry not found
 * Now offers to go online to download launcher if its not present
 * Changed target view not to use lower res texture, hi res one is already chached so might as well use it
 *
 * Revision 2.47  2003/11/29 10:52:09  randomtiger
 * Turned off D3D file mapping, its using too much memory which may be hurting older systems and doesnt seem to be providing much of a speed benifit.
 * Added stats command for ingame stats on memory usage.
 * Trys to play intro.mve and intro.avi, just to be safe since its not set by table.
 * Added fix for fonts wrapping round in non standard hi res modes.
 * Changed D3D mipmapping to a good value to suit htl mode.
 * Added new fog colour method which makes use of the bitmap, making this htl feature backcompatible again.
 *
 * Revision 2.46  2003/11/19 20:37:23  randomtiger
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
 * Revision 2.44  2003/11/15 18:09:33  randomtiger
 * Put TGA and JPG stuff on -t32 flag
 * Put 32 bit PCX stuff on -pcx32 (still has bugs)
 * Added multisample checking on device initialisation
 * Changed unrecognised parameter message (as requested) to be more user friendly
 * Speech now chooses voice based on reg value set by launcher v3.1
 *
 * Revision 2.43  2003/11/11 03:56:10  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.42  2003/11/09 06:31:38  Kazan
 * a couple of htl functions being called in nonhtl (ie NULL functions) problems fixed
 * conflicts in cmdline and timerbar.h log entries
 * cvs stopped acting like it was on crack obviously
 *
 * Revision 2.41  2003/11/09 04:09:18  Goober5000
 * edited for language
 * --Goober5000
 *
 * Revision 2.40  2003/11/08 22:25:47  Kazan
 * Timerbar was enabled in both release and debug - so i made it a command line option "-timerbar"
 * DONT MESS WITH OTHER PEOPLES INCLUDE PATHS
 * DONT MESS WITH EXEC NAMES (leave it fs2_open_r and fs2_open_d) or paths!
 *
 * Revision 2.39  2003/11/03 18:07:26  randomtiger
 * Added -d3d_no_vsync command to make turning off vsync optional.
 * Removed 32bit command, it doesnt do anything.
 * Made aa multisample reg check safe.
 *
 * Revision 2.38  2003/10/27 23:04:20  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.37  2003/10/26 00:31:58  randomtiger
 * Fixed hulls not drawing (with Phreaks advise).
 * Put my 32bit PCX loading under PCX_32 compile flag until its working.
 * Fixed a bug with res 640x480 I introduced with my non standard mode code.
 * Changed JPG and TGA loading command line param to "-t32"
 *
 * Revision 2.36  2003/10/24 17:35:04  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.35  2003/10/15 22:03:23  Kazan
 * Da Species Update :D
 *
 * Revision 2.34  2003/10/14 17:39:12  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.33  2003/10/12 03:41:36  Kazan
 * #Kazan# FS2NetD client code gone multithreaded, some Fred2 Open -mod stuff [obvious code.lib] including a change in cmdline.cpp, changed Stick's "-nohtl" to "-htl" - HTL is _OFF_ by default here (Bobboau and I decided this was a better idea for now)
 *
 * Revision 2.32  2003/10/10 03:59:40  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.31  2003/09/25 21:12:22  Kazan
 * ##Kazan## FS2NetD Completed!  Just needs some thorough bug checking (i don't think there are any serious bugs)
 * Also D3D8 Screenshots work now.
 *
 * Revision 2.30  2003/09/23 02:42:52  Kazan
 * ##KAZAN## - FS2NetD Support! (FS2 Open PXO) -- Game Server Listing, and mission validation completed - stats storing to come - needs fs2open_pxo.cfg file [VP-able]
 *
 * Revision 2.29  2003/09/14 19:00:02  wmcoolmon
 * Changed "nospec" and "cell" to "Cmdline_nospec" and "Cmdline_cell"
 *
 * Revision 2.28  2003/09/14 18:32:24  wmcoolmon
 * Added "-safeloading" command line parameter, which uses old fs2_retail-style loading code -C
 *
 * Revision 2.27  2003/09/13 06:02:05  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.25  2003/09/09 17:10:54  matt
 * Added -nospec cmd line param to disable specular -Sticks
 *
 * Revision 2.24  2003/08/22 07:35:08  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.23  2003/08/21 20:54:37  randomtiger
 * Fixed switching - RT
 *
 * Revision 2.22  2003/08/12 03:18:33  bobboau
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
 * Revision 2.21  2003/08/09 06:07:23  bobboau
 * slightly better implementation of the new zbuffer thing, it now checks only three diferent formats defalting to the 16 bit if neither the 24 or 32 bit versions are suported
 *
 * Revision 2.20  2003/03/29 09:42:05  Goober5000
 * made beams default shield piercing again
 * also added a beam no pierce command line flag
 * and fixed something else which I forgot :P
 * --Goober5000
 *
 * Revision 2.19  2003/03/20 08:24:45  unknownplayer
 * Modified the command line options so they are all in lower-case characters.
 * Made a slight AI adjustment to how ships choose to attack turrets (they now have a 25% chance of attacking a beam turret which has fired at them from another ship)
 *
 * Revision 2.18  2003/03/18 10:07:00  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.17  2003/02/22 04:13:17  wmcoolmon
 * Added "-dshowvid" command-line option, which must be set in order for movies to play.
 *
 * Revision 2.16  2002/12/21 13:39:25  DTP
 * did bit more house keeping. modfied Phreaks fps cmdline a bit, so that we dont have to specific build code.libs for fred, but can use the same code.lib for both fs2_open.exe and fred2_open.exe
 *
 * Revision 2.15  2002/12/17 03:08:18  DTP
 * fix to Show_framerate. seems it will call an unresolved external error during fred builds. modified my ifndefs a bit, dsw modified to include preprocessor tag FRED.
 *
 * Revision 2.14  2002/12/17 02:22:34  DTP
 * wrong name :)
 *
 * Revision 2.13  2002/12/17 02:21:06  DTP
 * cleaned up after phreak :). it will make a mess in debug builds. added a couple of ifndef _debugs.
 *
 * Revision 2.12  2002/12/14 17:09:08  phreak
 * added a command line that displays frames per second when enabled.
 * use "-fps"
 *
 * Revision 2.11  2002/11/10 16:32:42  DTP
 * -DTP reworked mod support,
 *
 * Revision 2.10  2002/11/10 16:30:53  DTP
 * -DTP reworked mod support,
 *
 * Revision 2.9  2002/10/27 23:55:36  DTP
 * DTP; started basic implementation of mod-support
 * plain files only for now. fs2_open.exe -mod X will look for files in fs2/ X /all-legal-subdirectories. no checking/creating dirs yet. directories must be there.
 *
 * Revision 2.8  2002/10/22 23:02:39  randomtiger
 * Made Phreaks alternative scanning style optional under the command line tag "-phreak"
 * Fixed bug that changes HUD colour when targetting debris in a full nebula. - RT
 *
 * Revision 2.7  2002/10/19 03:50:28  randomtiger
 * Added special pause mode for easier action screenshots.
 * Added new command line parameter for accessing all single missions in tech room. - RT
 *
 * Revision 2.6  2002/08/27 13:38:57  penguin
 * Moved DirectX8 stuff to directx8 branch; reverted to previous
 *
 * Revision 2.4.2.4  2002/11/10 11:32:29  randomtiger
 *
 * Made D3D8 mipmapping optional on command line flag -d3dmipmip, off by default.
 * When on is now less blury. - RT
 *
 * Revision 2.4.2.3  2002/11/04 23:53:24  randomtiger
 *
 * Added new command line parameter -d3dlauncher which brings up the launcher.
 * This is needed since FS2 DX8 now stores the last successful details in the registry and
 * uses them to choose the adapter and mode to run in unless its windowed or they are not set.
 * Added some code for Antialiasing but it messes up the font but hopefully that can be fixed later. - RT
 *
 * Revision 2.4.2.2  2002/11/04 21:24:59  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.4.2.1  2002/08/27 13:19:02  penguin
 * Moved to directx8 branch
 *
 * Revision 2.5  2002/08/18 19:48:28  randomtiger
 * Added new lib files: strmiids and ddraw to get dshow working
 * Added new command line parameter to active direct show movie play: -dshowvid
 * Uncommented movie_play calls and includes
 *
 * Revision 2.4  2002/08/07 00:44:13  DTP
 * Implented -GF4FIX commandline switch
 *
 * Revision 2.3  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 06:35:15  DTP
 * added -almission commandline arguement, will autoload mission i.e fs2_open.exe -almission kickass will autoload kickass.fs2 which should be a multiplayer mission.
 *
 * Revision 2.1  2002/07/07 19:55:58  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/08 17:29:17  mharris
 * more port tweaks
 *
 * Revision 1.2  2002/05/07 13:22:14  mharris
 * added pstypes.h include
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 7     7/15/99 3:07p Dave
 * 32 bit detection support. Mouse coord commandline.
 * 
 * 6     7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 5     6/22/99 9:37p Dave
 * Put in pof spewing.
 * 
 * 4     1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 3     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 38    10/02/98 3:22p Allender
 * fix up the -connect option and fix the -port option
 * 
 * 37    9/15/98 4:04p Allender
 * added back in the -ip_addr command line switch because it needs to be
 * in the standalone server only executable
 * 
 * 36    9/14/98 11:52a Allender
 * don't use cfile
 * 
 * 35    9/14/98 11:28a Allender
 * support for server bashing of address when received from client.  Added
 * a cmdline.cfg file to process command line arguments from a file
 * 
 * 34    9/08/98 2:20p Allender
 * temporary code to force IP address to a specific value.
 * 
 * 33    8/20/98 5:30p Dave
 * Put in handy multiplayer logfile system. Now need to put in useful
 * applications of it all over the code.
 * 
 * 32    8/07/98 10:39a Allender
 * fixed debug standalone problem where stats would continually get sent
 * to tracker.  more debug code to help find stats problem
 * 
 * 31    7/24/98 11:14a Allender
 * start of new command line options for version 1.04
 * 
 * 30    5/21/98 1:50a Dave
 * Remove obsolete command line functions. Reduce shield explosion packets
 * drastically. Tweak PXO screen even more. Fix file xfer system so that
 * we can guarantee file uniqueness.
 * 
 * 29    5/18/98 9:10p Dave
 * Put in many new PXO features. Fixed skill level bashing in multiplayer.
 * Removed several old command line options. Put in network config files.
 * 
 * 28    5/09/98 7:16p Dave
 * Put in CD checking. Put in standalone host password. Made pilot into
 * popup scrollable.
 * 
 * 27    4/23/98 8:27p Allender
 * basic support for cutscene playback.  Into movie code in place.  Tech
 * room can view cutscenes stored in CDROM_dir variable
 * 
 * 26    4/09/98 5:43p Dave
 * Remove all command line processing from the demo. Began work fixing up
 * the new multi host options screen.
 * 
 * 25    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 24    4/01/98 5:56p Dave
 * Fixed a messaging bug which caused msg_all mode in multiplayer not to
 * work. Compile out a host of multiplayer options not available in the
 * demo.
 * 
 * 23    3/14/98 2:48p Dave
 * Cleaned up observer joining code. Put in support for file xfers to
 * ingame joiners (observers or not). Revamped and reinstalled pseudo
 * lag/loss system.
 * 
 * 22    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 21    1/31/98 4:32p Dave
 * Put in new support for VMT player validation, game logging in and game
 * logging out.
 * 
 * 20    12/10/97 4:45p Dave
 * Added in more detailed support for multiplayer packet lag/loss. Fixed
 * some multiplayer stuff. Added some controls to the standalone.
 * 
 * 19    12/09/97 6:14p Lawrance
 * add -nomusic flag
 * 
 * 18    12/01/97 5:10p Dave
 * Fixed a syntax bug.
 * 
 * 17    12/01/97 4:59p Dave
 * Synchronized multiplayer debris objects. Put in pilot popup in main
 * hall. Optimized simulated multiplayer lag module. Fixed a potential
 * file_xfer bug.
 * 
 * 16    11/28/97 7:04p Dave
 * Emergency checkin due to big system crash.
 * 
 * 15    11/28/97 5:06p Dave
 * Put in facilities for simulating multiplayer lag.
 * 
 * 14    11/24/97 5:42p Dave
 * Fixed a file xfer buffer free/malloc problem. Lengthened command line
 * switch string parse length.
 * 
 * 13    11/12/97 4:39p Dave
 * Put in multiplayer campaign support parsing, loading and saving. Made
 * command-line variables better named. Changed some things on the initial
 * pilot select screen.
 * 
 * 12    11/11/97 4:54p Dave
 * Put in support for single vs. multiplayer pilots. Put in initial player
 * selection screen (no command line option yet). Started work on
 * multiplayer campaign file save gaming.
 * 
 * 11    11/11/97 11:55a Allender
 * initialize network at beginning of application.  create new call to set
 * which network protocol to use
 * 
 * 10    9/18/97 10:12p Dave
 * Added -gimmemedals, which gives the current pilot all the medals in the
 * game (debug)
 * 
 * 9     9/18/97 9:20a Dave
 * Minor modifications
 * 
 * 8     9/15/97 11:40p Lawrance
 * remove demo granularity switch
 * 
 * 7     9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 6     9/03/97 5:03p Lawrance
 * add support for -nosound command line parm
 * 
 * 5     8/22/97 8:52a Dave
 * Removed a return statement which would have broken the parser out too
 * early.
 * 
 * 4     8/21/97 4:55p Dave
 * Added a switch for multiplayer chat streaming. Added a section for
 * global command line vars.
 * 
 * 3     8/06/97 2:26p Dave
 * Made the command line parse more robust. Made it easier to add and
 * process new command-line switches.
 * 
 * 2     8/04/97 3:13p Dave
 * Added command line functions. See cmdline.cpp for directions on adding
 * new switches
 * 
 * 1     8/04/97 9:58a Dave
 * 
 * $NoKeywords: $
 */

#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "network/multi.h"
#include "hud/hudconfig.h"
#include "parse/scripting.h"
#include "parse/sexp.h"
#include "globalincs/version.h"
#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <direct.h>
#elif defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef SCP_UNIX
#include "osapi/osapi.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <vector>


// variables
class cmdline_parm {
public:
	cmdline_parm *next, *prev;
	char *name;						// name of parameter, must start with '-' char
	char *help;						// help text for this parameter
	char *args;						// string value for parameter arguements (NULL if no arguements)
	int name_found;				// true if parameter on command line, otherwise false

	cmdline_parm(char *name, char *help);
	~cmdline_parm();
	int found();
	int get_int();
	float get_float();
	char *str();
};

static cmdline_parm Parm_list(NULL, NULL);
static int Parm_list_inited = 0;


extern float VIEWER_ZOOM_DEFAULT;
extern float Viewer_zoom;
extern int Show_framerate;	// from freespace.cpp


enum
{
	// DO NOT CHANGE ANYTHING ABOUT THESE FIRST TWO OR WILL MESS UP THE LAUNCHER
	EASY_DEFAULT  =  1 << 1,
	EASY_ALL_ON   =  1 << 2,

	EASY_MEM_ON   =  1 << 3,
	EASY_MEM_OFF  =  1 << 4,

	// Add new flags here

	// Combos
	EASY_MEM_ALL_ON  = EASY_ALL_ON  | EASY_MEM_ON,
	EASY_DEFAULT_MEM = EASY_DEFAULT | EASY_MEM_OFF,
};

typedef struct
{
	// DO NOT CHANGE THE SIZE OF THIS STRING!
	char name[32];

} EasyFlag;

EasyFlag easy_flags[] =
{
	{ "Custom" },
	{ "Default FS2 (All features off)" },
	{ "All features on" },
	{ "High memory usage features on" },
	{ "High memory usage features off" }
};

// DO NOT CHANGE **ANYTHING** ABOUT THIS STRUCTURE AND ITS CONTENT
typedef struct
{
	char  name[20];		// The actual flag
	char  desc[40];		// The text that will appear in the launcher (unless its blank, other name is shown)
	bool  fso_only;		// true if this is a fs2_open only feature
	int   on_flags;		// Easy flag which will turn this feature on
	int   off_flags;	// Easy flag which will turn this feature off
	char  type[16];		// Launcher uses this to put flags under different headings
	char  web_url[256];	// Link to documentation of feature (please use wiki or somewhere constant)

} Flag;

// Please group them by type, ie graphics, gameplay etc, maximum 20 different types
Flag exe_params[] = 
{
	{ "-spec",				"Enable specular",							true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-spec", },
	{ "-glow",				"Enable glowmaps",							true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-glow", },
	{ "-env",				"Enable evironment maps",					true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-env", },
	{ "-jpgtga",			"Enable jpg,tga textures",					true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-jpgtga", },
	{ "-mipmap",			"Enable mipmapping",						true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mipmap", },
	{ "-nomotiondebris",	"Disable motion debris",					true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomotiondebris",},
	{ "-2d_poof",			"Stops fog intersect hull",					true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-2d_poof", },
	{ "-noscalevid",		"Disable scale-to-window for movies",		true,	0,					EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noscalevid", },
	{ "-missile_lighting",	"Apply Lighting to Missiles"	,			true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-missile_lighting", },

	{ "-img2dds",			"Compress non-compressed images",			true,	0,					EASY_DEFAULT,		"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-img2dds", },
	{ "-no_vsync",			"Disable vertical sync",					true,	0,					EASY_DEFAULT,		"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_vsync", },
	{ "-cache_bitmaps",		"Cache bitmaps between missions",			true,	0,					EASY_DEFAULT_MEM,	"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-cache_bitmaps", },

	{ "-dualscanlines",		"Another pair of scanning lines",			true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dualscanlines", },
	{ "-targetinfo",		"Enable info next to target",				true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-targetinfo", },
	{ "-orbradar",			"Enables 3d radar",							true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-orbradar", },
	{ "-rearm_timer",		"Enable Rearm/Repair Completion Timer",		true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-rearm_timer", },
	{ "-ballistic_gauge",	"Enable the analog ballistic ammo gauge",	true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ballistic_gauge", },

	{ "-ship_choice_3d",	"Use models for ship selection",			true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ship_choice_3d", },
	{ "-3dwarp",			"Enable 3d warp",							true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-3dwarp", },
	{ "-warp_flash",		"Enable flash upon warp",					true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-warp_flash", },
	{ "-tbp",				"Toggle features for The Babylon Project",	true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-tbp", }, // TBP warp effects -Et1
	{ "-wcsaga",			"Toggle features for Wing Commander Saga",	true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-wcsaga", },

	{ "-snd_preload",		"Preload mission game sounds",				true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-snd_preload", },
	{ "-nosound",			"Disable sound and music",					false,	0,					EASY_DEFAULT,		"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nosound", },
	{ "-nomusic",			"Disable music",							false,	0,					EASY_DEFAULT,		"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomusic", },

	{ "-standalone",		"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-standalone", },
	{ "-startgame",			"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-startgame", },
	{ "-closed",			"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-closed", },
	{ "-restricted",		"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-restricted", },
	{ "-multilog",			"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-multilog", },
	{ "-clientdamage",		"",											false,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-clientdamage", },
	{ "-mpnoreturn",		"Disables flight deck option",				true,	0,					EASY_DEFAULT,		"Multi",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mpnoreturn", },
//#ifdef WIN32
//	{ "-fixbugs",			"Fix bugs",									true,	0,					EASY_DEFAULT,		"Troubleshoot",	"", },
//	{ "-nocrash",			"Disable crashing",							true,	0,					EASY_DEFAULT,		"Troubleshoot",	"", },
//#endif
	{ "-oldfire",			"",											true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-oldfire", },
	{ "-nohtl",				"Software mode (very slow)",				true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nohtl", },
	{ "-no_set_gamma",		"Disable setting of gamma",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_set_gamma", },
	{ "-nomovies",			"Disable video playback",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomovies", },
	{ "-noparseerrors",		"Disable parsing errors",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noparseerrors", },
	{ "-safeloading",		"",											true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-safeloading", },
	{ "-query_speech",		"Does this build have speech?",				true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-query_speech", },
	{ "-d3d_bad_tsys",		"Enable inefficient textures",				false,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-d3d_bad_tsys", },
	{ "-novbo",				"Disable OpenGL VBO",						true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-novbo",	},
	{ "-noibx",				"Don't use cached index buffers (IBX)",		true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noibx",	},
	{ "-loadallweps",		"Load all weapons, even those not used",	true,	0,					EASY_DEFAULT,		"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-loadallweps", },

	{ "-alpha_env",			"Use specular alpha for env mapping",		true,	0,					EASY_DEFAULT_MEM,	"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-alpha_env", },
	{ "-ingame_join",		"Allows ingame joining",					true,	0,					EASY_DEFAULT,		"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ingame_join", },
	{ "-voicer",			"Voice recognition",						true,	0,					EASY_DEFAULT,		"Experimental",	"", },

	{ "-fps",				"Show frames per second on HUD",			false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fps", },
	{ "-pos",				"Show position of camera",					false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pos", },
	{ "-window",			"Run in window",							true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-window", },
	{ "-timerbar",			"",											true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-timerbar", },
	{ "-stats",				"Show statistics",							true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-stats", },
	{ "-coords",			"Show coordinates",							false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-coords", },
	{ "-show_mem_usage",	"Show memory usage",						true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-show_mem_usage", },
	{ "-pofspew",			"",											false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pofspew", },
	{ "-tablecrcs",			"",											true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-tablecrcs", },
	{ "-missioncrcs",		"",											true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-missioncrcs", },
	{ "-dis_collisions",	"Disable collisions",						true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_collisions", },
	{ "-dis_weapons",		"Disable weapon rendering",					true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_weapons", },
	{ "-output_sexps",		"Outputs SEXPs to sexps.html",				true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_sexps", },
	{ "-output_scripting",	"Outputs scripting to scripting.html",		true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_scripting", },
	{ "-save_render_target",	"Save render targets to file",			true,	0,					EASY_DEFAULT,		"Dev Tool",		"", },
#ifdef SCP_UNIX
	{ "-nograb",			"Don't grab mouse/keyboard in a window",	true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nograb", },
#endif
};

// here are the command line parameters that we will be using for FreeSpace

// RETAIL options ----------------------------------------------
cmdline_parm connect_arg("-connect", NULL);			// Cmdline_connect_addr
cmdline_parm gamename_arg("-gamename", NULL);		// Cmdline_game_name
cmdline_parm gamepassword_arg("-password", NULL);	// Cmdline_game_password
cmdline_parm allowabove_arg("-allowabove", NULL);	// Cmdline_rank_above
cmdline_parm allowbelow_arg("-allowbelow", NULL);	// Cmdline_rank_below
cmdline_parm standalone_arg("-standalone", NULL);
cmdline_parm nosound_arg("-nosound", NULL);			// Cmdline_freespace_no_sound
cmdline_parm nomusic_arg("-nomusic", NULL);			// Cmdline_freespace_no_music
cmdline_parm startgame_arg("-startgame", NULL);		// Cmdline_start_netgame
cmdline_parm gameclosed_arg("-closed", NULL);		// Cmdline_closed_game
cmdline_parm gamerestricted_arg("-restricted", NULL);	// Cmdline_restricted_game
cmdline_parm port_arg("-port", NULL);
cmdline_parm multilog_arg("-multilog", NULL);		// Cmdline_multi_log
cmdline_parm server_firing_arg("-oldfire", NULL);	// Cmdline_server_firing
cmdline_parm client_dodamage("-clientdamage", NULL);	// Cmdline_client_dodamage
cmdline_parm pof_spew("-pofspew", NULL);			// Cmdline_spew_pof_info
cmdline_parm mouse_coords("-coords", NULL);			// Cmdline_mouse_coords
cmdline_parm timeout("-timeout", NULL);				// Cmdline_timeout
cmdline_parm window("-window", NULL);				// Cmdline_window

char *Cmdline_connect_addr = NULL;
char *Cmdline_game_name = NULL;
char *Cmdline_game_password = NULL;
char *Cmdline_rank_above = NULL;
char *Cmdline_rank_below = NULL;
int Cmdline_cd_check = 1;
int Cmdline_client_dodamage = 0;
int Cmdline_closed_game = 0;
int Cmdline_freespace_no_music = 0;
int Cmdline_freespace_no_sound = 0;
int Cmdline_gimme_all_medals = 0;
int Cmdline_mouse_coords = 0;
int Cmdline_multi_log = 0;
int Cmdline_multi_stream_chat_to_file = 0;
int Cmdline_network_port = -1;
int Cmdline_restricted_game = 0;
int Cmdline_server_firing = 0;
int Cmdline_spew_pof_info = 0;
int Cmdline_start_netgame = 0;
int Cmdline_timeout = -1;
int Cmdline_use_last_pilot = 0;
int Cmdline_window = 0;


// FSO options -------------------------------------------------

// Graphics related
cmdline_parm spec_exp_arg("-spec_exp", NULL);		// comand line FOV -Bobboau
cmdline_parm clip_dist_arg("-clipdist", NULL);		// Cmdline_clip_dist
cmdline_parm fov_arg("-fov", NULL);					// Cmdline_fov  -- comand line FOV -Bobboau
cmdline_parm ogl_spec_arg("-ogl_spec", NULL);		// Cmdline_ogl_spec
cmdline_parm spec_static_arg("-spec_static", NULL);
cmdline_parm spec_point_arg("-spec_point", NULL);
cmdline_parm spec_tube_arg("-spec_tube", NULL);
cmdline_parm poof_2d_arg("-2d_poof", NULL);			// Cmdline_2d_poof
cmdline_parm alpha_env("-alpha_env", NULL);			// Cmdline_alpha_env
cmdline_parm ambient_factor_arg("-ambient_factor", NULL);	// Cmdline_ambient_factor
cmdline_parm cell_arg("-cell", NULL);				// Cmdline_cell
cmdline_parm decals("-decals", NULL);				// Cmdline_decals
cmdline_parm env("-env", NULL);						// Cmdline_env
cmdline_parm jpgtga_arg("-jpgtga", NULL);			// Cmdline_jpgtga
cmdline_parm mipmap_arg("-mipmap", NULL);			// Cmdline_mipmap
cmdline_parm missile_lighting_arg("-missile_lighting", NULL);	// Cmdline_missile_lighting
cmdline_parm glow_arg("-glow", NULL); 				// Cmdline_noglow  -- use Bobs glow code
cmdline_parm nomotiondebris_arg("-nomotiondebris", NULL); // Cmdline_nomotiondebris  -- Removes those ugly floating rocks -C
cmdline_parm noscalevid_arg("-noscalevid", NULL);	// Cmdline_noscalevid  -- disable video scaling that fits to window
cmdline_parm spec_arg("-spec", NULL);				// Cmdline_nospec  -- use specular highlighting -Sticks
cmdline_parm pcx32_arg("-pcx32", NULL);				// Cmdline_pcx32
cmdline_parm noemissive_arg("-no_emissive_light", NULL);		// Cmdline_no_emissive  -- don't use emissive light in OGL
cmdline_parm spec_scale_arg("-spec_scale", NULL);	// Cmdline_spec_scale -- TEMPORARY - REMOVEME!!!
cmdline_parm env_scale_arg("-env_scale", NULL);		// Cmdline_env_scale -- TEMPORARY - REMOVEME!!!
cmdline_parm alpha_alpha_blend_arg("-alpha_alpha_blend", NULL);	// Cmdline_alpha_alpha_blend -- TEMPORARY - REMOVEME!!!

float Cmdline_clip_dist = Default_min_draw_distance;
float Cmdline_fov = 0.75f;
float Cmdline_ogl_spec = 80.0f;
float Cmdline_spec_scale = 1.0f; // TEMPORARY - REMOVEME!!!
float Cmdline_env_scale = 2.0f; // TEMPORARY - REMOVEME!!!
int Cmdline_2d_poof = 0;
int Cmdline_alpha_env = 0;
int Cmdline_ambient_factor = 128;
int Cmdline_cell = 0;
int Cmdline_decals = 0;
int Cmdline_env = 0;
int Cmdline_jpgtga = 0;
int Cmdline_mipmap = 0;
int Cmdline_missile_lighting = 0;
int Cmdline_noglow = 1;
int Cmdline_nomotiondebris = 0;
int Cmdline_noscalevid = 0;
int Cmdline_nospec = 1;
int Cmdline_pcx32 = 0;
int Cmdline_no_emissive = 0;
int Cmdline_alpha_alpha_blend = 0; // TEMPORARY - REMOVEME!!!

// Game Speed related
cmdline_parm cache_bitmaps_arg("-cache_bitmaps", NULL);	// Cmdline_cache_bitmaps
cmdline_parm img2dds_arg("-img2dds", NULL);			// Cmdline_img2dds
cmdline_parm no_fpscap("-no_fps_capping", NULL);	// Cmdline_NoFPSCap
cmdline_parm no_vsync_arg("-no_vsync", NULL);		// Cmdline_no_vsync

int Cmdline_cache_bitmaps = 0;	// caching of bitmaps between missions (faster loads, can hit swap on reload with <512 Meg RAM though) - taylor
int Cmdline_img2dds = 0;
int Cmdline_NoFPSCap = 0; // Disable FPS capping - kazan
int Cmdline_no_vsync = 0;

// HUD related
cmdline_parm ballistic_gauge("-ballistic_gauge", NULL);	// Cmdline_ballistic_gauge
cmdline_parm dualscanlines_arg("-dualscanlines", NULL); // Cmdline_dualscanlines  -- Change to phreaks options including new targetting code
cmdline_parm orb_radar("-orbradar", NULL);			// Cmdline_orb_radar
cmdline_parm rearm_timer_arg("-rearm_timer", NULL);	// Cmdline_rearm_timer
cmdline_parm targetinfo_arg("-targetinfo", NULL);	// Cmdline_targetinfo  -- Adds ship name/class to right of target box -C
cmdline_parm Radar_Range_Clamp("-radar_reduce", NULL);

int Cmdline_ballistic_gauge = 0;	// WMCoolmon's gauge thingy
int Cmdline_dualscanlines = 0;
int Cmdline_orb_radar = 0;
int Cmdline_rearm_timer = 0;
int Cmdline_targetinfo = 0;

// Gameplay related
cmdline_parm use_3dwarp("-3dwarp", NULL);			// Cmdline_3dwarp
cmdline_parm ship_choice_3d_arg("-ship_choice_3d", NULL);	// Cmdline_ship_choice_3d
cmdline_parm use_warp_flash("-warp_flash", NULL);	// Cmdline_warp_flash

int Cmdline_3dwarp = 0;
int Cmdline_ship_choice_3d = 0;
int Cmdline_warp_flash = 0;

// Audio related
cmdline_parm query_speech_arg("-query_speech", NULL);	// Cmdline_query_speech
cmdline_parm snd_preload_arg("-snd_preload", NULL);		// Cmdline_snd_preload
cmdline_parm voice_recognition_arg("-voicer", NULL);	// Cmdline_voice_recognition

int Cmdline_query_speech = 0;
int Cmdline_snd_preload = 0; // preload game sounds during mission load
int Cmdline_voice_recognition = 0;

// MOD related
cmdline_parm mod_arg("-mod", NULL);		// Cmdline_mod  -- DTP modsupport
cmdline_parm tbp("-tbp", NULL);			// Cmdline_tbp  -- TBP warp effects -Et1
cmdline_parm wcsaga("-wcsaga", NULL);	// Cmdline_wcsaga

char *Cmdline_mod = NULL; //DTP for mod arguement
int Cmdline_tbp = 0;
int Cmdline_wcsaga = 0;

// Multiplayer/Network related
cmdline_parm almission_arg("-almission", NULL);		// Cmdline_almission  -- DTP for autoload Multi mission
cmdline_parm ingamejoin_arg("-ingame_join", NULL);	// Cmdline_ingamejoin
cmdline_parm mpnoreturn_arg("-mpnoreturn", NULL);	// Cmdline_mpnoreturn  -- Removes 'Return to Flight Deck' in respawn dialog -C
cmdline_parm MissionCRCs("-missioncrcs", NULL);		// Cmdline_SpewMission_CRCs
cmdline_parm TableCRCs("-tablecrcs", NULL);			// Cmdline_SpewTable_CRCs

char *Cmdline_almission = NULL;	//DTP for autoload multi mission.
int Cmdline_ingamejoin = 0;
int Cmdline_mpnoreturn = 0;
int Cmdline_SpewMission_CRCs = 0; // Kazan for making valid mission lists
int Cmdline_SpewTable_CRCs = 0;

// Troubleshooting
cmdline_parm d3d_lesstmem_arg("-d3d_bad_tsys", NULL);	// Cmdline_d3d_lesstmem
cmdline_parm fred2_htl_arg("-fredhtl", NULL);		// Cmdline_FRED2_htl
cmdline_parm loadallweapons_arg("-loadallweps", NULL);	// Cmdline_load_all_weapons
cmdline_parm htl_arg("-nohtl", NULL);				// Cmdline_nohtl  -- don't use HT&L
cmdline_parm noibx_arg("-noibx", NULL);				// Cmdline_noibx
cmdline_parm nomovies_arg("-nomovies", NULL);		// Cmdline_nomovies  -- Allows video streaming
cmdline_parm no_set_gamma_arg("-no_set_gamma", NULL);	// Cmdline_no_set_gamma
cmdline_parm no_vbo_arg("-novbo", NULL);			// Cmdline_novbo
cmdline_parm safeloading_arg("-safeloading", NULL);	// Cmdline_safeloading  -- Uses old loading method -C

int Cmdline_d3d_lesstmem = 0;
int Cmdline_FRED2_htl = 0; // turn HTL on in fred - Kazan
int Cmdline_load_all_weapons = 0;
int Cmdline_nohtl = 0;
int Cmdline_noibx = 0;
int Cmdline_nomovies = 0;
int Cmdline_no_set_gamma = 0;
int Cmdline_novbo = 0; // turn off OGL VBO support, troubleshooting
int Cmdline_safeloading = 0;

// Developer/Testing related
cmdline_parm start_mission_arg("-start_mission", NULL);	// Cmdline_start_mission
cmdline_parm allslev_arg("-allslev", NULL);			// Cmdline_allslev  -- Give access to all single player missions
cmdline_parm dis_collisions("-dis_collisions", NULL);	// Cmdline_dis_collisions
cmdline_parm dis_weapons("-dis_weapons", NULL);		// Cmdline_dis_weapons
cmdline_parm noparseerrors_arg("-noparseerrors", NULL);	// Cmdline_noparseerrors  -- turns off parsing errors -C
cmdline_parm nowarn_arg("-no_warn", NULL);			// Cmdline_nowarn
cmdline_parm fps_arg("-fps", NULL);					// Cmdline_show_fps
cmdline_parm show_mem_usage_arg("-show_mem_usage", NULL);	// Cmdline_show_mem_usage
cmdline_parm pos_arg("-pos", NULL);					// Cmdline_show_pos
cmdline_parm stats_arg("-stats", NULL);				// Cmdline_show_stats
cmdline_parm timerbar_arg("-timerbar", NULL);		// Cmdline_timerbar
cmdline_parm save_render_targets_arg("-save_render_target", NULL);	// Cmdline_save_render_targets
#ifdef SCP_UNIX
cmdline_parm no_grab("-nograb", NULL);				// Cmdline_no_grab
#endif

char *Cmdline_start_mission = NULL;
int Cmdline_allslev = 0;
int Cmdline_dis_collisions = 0;
int Cmdline_dis_weapons = 0;
int Cmdline_noparseerrors = 0;
int Cmdline_nowarn = 0; // turn warnings off in FRED
int Cmdline_show_mem_usage = 0;
int Cmdline_show_pos = 0;
int Cmdline_show_stats = 0;
int Cmdline_timerbar = 0;
int Cmdline_save_render_targets = 0;
#ifdef SCP_UNIX
int Cmdline_no_grab = 0;
#endif

// Other
cmdline_parm get_flags_arg("-get_flags", NULL);
cmdline_parm output_sexp_arg("-output_sexps", NULL); //WMC - outputs all SEXPs to sexps.html
cmdline_parm output_scripting_arg("-output_scripting", NULL);	//WMC

// Totally useless crap...
/*#ifdef WIN32
cmdline_parm fix_bugs("-fixbugs", NULL);
cmdline_parm disable_crashing("-nocrash", NULL);
#endif*/




#ifndef NDEBUG
// NOTE: this assumes that os_init() has already been called but isn't a fatal error if it hasn't
void cmdline_debug_print_cmdline()
{
	cmdline_parm *parmp;
	int found = 0;

	mprintf(("Passed cmdline options:"));

	for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
		if ( parmp->name_found ) {
			if ( parmp->args != NULL ) {
				mprintf(("\n  %s %s", parmp->name, parmp->args));
			} else {
				mprintf(("\n  %s", parmp->name));
			}
			found++;
		}
	}

	if ( !found )
		mprintf(("\n  <none>"));

	mprintf(("\n"));
}
#endif

//	Return true if this character is an extra char (white space and quotes)
int is_extra_space(char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == 0x0a) || (ch == '\'') || (ch == '\"'));
}


// eliminates all leading and trailing extra chars from a string.  Returns pointer passed in.
char *drop_extra_chars(char *str)
{
	int s, e;

	s = 0;
	while (str[s] && is_extra_space(str[s]))
		s++;

	e = strlen(str);
	while (e > s) {
		if (!is_extra_space(str[e])){
			break;
		}

		e--;
	}

	if (e > s){
		memmove(str, str + s, e - s + 1);
	}

	str[e - s + 1] = 0;
	return str;
}


// internal function - copy the value for a parameter agruement into the cmdline_parm arg field
void parm_stuff_args(cmdline_parm *parm, char *cmdline)
{
	char buffer[1024];
	memset(buffer, 0, 1024);
	char *dest = buffer;

	cmdline += strlen(parm->name);

	while ((*cmdline != 0) && (*cmdline != '-')) {
		*dest++ = *cmdline++;
	}

	drop_extra_chars(buffer);

	// mwa 9/14/98 -- made it so that newer command line arguments found will overwrite
	// the old arguments
//	Assert(parm->args == NULL);
	if ( parm->args != NULL ) {
		delete( parm->args );
		parm->args = NULL;
	}

	int size = strlen(buffer) + 1;
	if (size > 0) {
		parm->args = new char[size];
		memset(parm->args, 0, size);
		strcpy(parm->args, buffer);
	}
}


// internal function - parse the command line, extracting parameter arguements if they exist
// cmdline - command line string passed to the application
void os_parse_parms(char *cmdline)
{
	// locate command line parameters
	cmdline_parm *parmp;
	char *cmdline_offset;
	size_t get_new_offset = 0;

	for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
		// while going through the cmdline make sure to grab only the option that we are looking
		// for but if one similar then keep searching for the exact match
		do {
			cmdline_offset = strstr(cmdline + get_new_offset, parmp->name);

			if (cmdline_offset && (*(cmdline_offset + strlen(parmp->name))) && !is_extra_space(*(cmdline_offset + strlen(parmp->name))) ) {
				// the new offset should be our currently location + the length of the current option
				get_new_offset = (strlen(cmdline) - strlen(cmdline_offset) + strlen(parmp->name));
			} else {
				get_new_offset = 0;
			}
		} while ( get_new_offset );

		if (cmdline_offset) {
			parmp->name_found = 1;
			parm_stuff_args(parmp, cmdline_offset);
		}
	}
}


// validate the command line parameters.  Display an error if an unrecognized parameter is located.
void os_validate_parms(char *cmdline)
{
	cmdline_parm *parmp;
	char seps[] = " ,\t\n";
	char *token;
	int parm_found;

   token = strtok(cmdline, seps);
   while(token != NULL) {
	
		if (token[0] == '-') {
			parm_found = 0;
			for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
				if (!stricmp(parmp->name, token)) {
					parm_found = 1;
					break;
				}
			}

			if (parm_found == 0) {
#ifdef _WIN32
				// Changed this to MessageBox, this is a user error not a developer
				char buffer[128];
				sprintf(buffer,"Unrecogzined command line parameter %s, continue?",token);
				if( MessageBox(NULL, buffer, "Warning", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
					exit(0);
#elif defined(APPLE_APP)
				CFStringRef message;
				char buffer[128];
				CFOptionFlags result;

				snprintf(buffer, 128, "Unrecognized command line parameter, \"%s\", continue?", token);
				message = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingASCII);

				if ( CFUserNotificationDisplayAlert(0, kCFUserNotificationPlainAlertLevel, NULL, NULL, NULL, CFSTR("Unknown Command"), message, NULL, CFSTR("Quit"), NULL, &result) )
					exit(0);

				if (result != kCFUserNotificationDefaultResponse)
					exit(0);
#else
				// if we got a -help, --help, or -h then show the help text, otherwise show unknown option
				if ( !stricmp(token, "-help") || !stricmp(token, "--help") || !stricmp(token, "-h") ) {
					printf("FS2 Open: The Source Code Project, version %i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
					printf("Website: http://scp.indiegames.us\n");
					printf("Mantis (bug reporting): http://lore.maxgaming.net/~scp/mantis/\n\n");
					printf("Usage: fs2_open [options]\n");

					// not the prettiest thing but the job gets done
					static const int STR_SIZE = 25;  // max len of exe_params.name + 5 spaces
					int p=0, sp=0;
					while (exe_params[p].name[0] == '-') {
						sp = strlen(exe_params[p].name);
						printf("    [ %s ]%*s- %s\n", exe_params[p].name, (STR_SIZE - sp - 1), NOX(" "), exe_params[p].desc);
						p++;
					}
				} else {
					printf("Unrecognized command line parameter \"%s\".  Exiting...\n", token);
				}
				printf("\n");
				exit(0);
#endif
			}
		}

		token = strtok(NULL, seps);
	}
}


// Call once to initialize the command line system
//
// cmdline - command line string passed to the application
void os_init_cmdline(char *cmdline)
{
	FILE *fp;

	// read the cmdline.cfg file from the data folder, and pass the command line arguments to
	// the the parse_parms and validate_parms line.  Read these first so anything actually on
	// the command line will take precedence
#ifdef _WIN32
	fp = fopen("data\\cmdline_fso.cfg", "rt");
#elif defined(APPLE_APP)
	extern char full_path[1024];
	char *c = NULL, data_path[1024];

	c = strstr(full_path, ".app");
	if ( c != NULL ) {
		while (c && (*c != '/'))
			c--;
		
		*c = '\0';
	}
	snprintf(data_path, 1024, "%s/data/cmdline_fso.cfg", full_path);

	fp = fopen(data_path, "rt");
#else
	fp = fopen("data/cmdline_fso.cfg", "rt");
#endif

	// if the file exists, get a single line, and deal with it
	if ( fp ) {
		char buf[1024], *p;

		fgets(buf, 1024, fp);

		// replace the newline character with a NULL
		if ( (p = strrchr(buf, '\n')) != NULL ) {
			*p = '\0';
		}

#ifdef SCP_UNIX
		// append a space for the os_parse_parms() check
		strcat(buf, " ");
#endif

		os_parse_parms(buf);
		os_validate_parms(buf);
		fclose(fp);
	}

#ifdef SCP_UNIX
	// parse user specific cmdline config file (will supersede options in global file)
	char cmdname[MAX_PATH];

	snprintf(cmdname, MAX_PATH, "%s/%s/data/cmdline_fso.cfg", detect_home(), Osreg_user_dir);
	fp = fopen(cmdname, "rt");

	if ( !fp ) {
		// try for non "_fso", for older code versions
		snprintf(cmdname, MAX_PATH, "%s/%s/data/cmdline.cfg", detect_home(), Osreg_user_dir);
		fp = fopen(cmdname, "rt");
	}

	// if the file exists, get a single line, and deal with it
	if ( fp ) {
		char buf[1024], *p;

		fgets(buf, 1024, fp);

		// replace the newline character with a NULL
		if ( (p = strrchr(buf, '\n')) != NULL ) {
			*p = '\0';
		}

		// append a space for the os_parse_parms() check
		strcat(buf, " ");

		os_parse_parms(buf);
		os_validate_parms(buf);
		fclose(fp);
	}
#endif

	os_parse_parms(cmdline);
	os_validate_parms(cmdline);
}


// arg constructor
// name_ - name of the parameter, must start with '-' character
// help_ - help text for this parameter
cmdline_parm::cmdline_parm(char *name_, char *help_)
{
	name = name_;
	help = help_;
	args = NULL;
	name_found = 0;

	if (Parm_list_inited == 0) {
		list_init(&Parm_list);
		Parm_list_inited = 1;
	}

	if (name != NULL) {
		list_append(&Parm_list, this);
	}
}


// destructor - frees any allocated memory
cmdline_parm::~cmdline_parm()
{
#ifndef FRED
	if (args) {
		delete [] args;
		args = NULL;
	}
#endif
}


// returns - true if the parameter exists on the command line, otherwise false
int cmdline_parm::found()
{
	return name_found;
}

void output_sexp_html(int sexp_idx, FILE *fp)
{
	if(sexp_idx < 0 || sexp_idx > Num_operators)
		return;

	bool printed=false;

	for(int i = 0; i < Num_sexp_help; i++)
	{
		if(Sexp_help[i].id == Operators[sexp_idx].value)
		{
			char* new_buf = new char[2*strlen(Sexp_help[i].help)];
			char* dest_ptr = new_buf;
			char* curr_ptr = Sexp_help[i].help;
			char* end_ptr = curr_ptr + strlen(Sexp_help[i].help);
			while(curr_ptr < end_ptr)
			{
				if(*curr_ptr == '\n')
				{
					strcpy(dest_ptr, "\n<br>");
					dest_ptr+=5;
				}
				else
				{
					*dest_ptr++ = *curr_ptr;
				}
				curr_ptr++;
			}
			*dest_ptr = '\0';

			fprintf(fp, "<dt><b>%s</b></dt>\n<dd>%s</dd>\n", Operators[sexp_idx].text, new_buf);
			delete[] new_buf;

			printed = true;
		}
	}

	if(!printed)
		fprintf(fp, "<dt><b>%s</b></dt>\n<dd>Min arguments: %d, Max arguments: %d</dd>\n", Operators[sexp_idx].text, Operators[sexp_idx].min, Operators[sexp_idx].max);
}


// returns - the interger representation for the parameter arguement
int cmdline_parm::get_int()
{
	Assert(args);
	return atoi(args);
}


// returns - the float representation for the parameter arguement
float cmdline_parm::get_float()
{
	Assert(args);
	return (float)atof(args);
}


// returns - the string value for the parameter arguement
char *cmdline_parm::str()
{
	Assert(args);
	return args;
}

// external entry point into this modules

bool SetCmdlineParams()
// Sets externed variables used for communication cmdline information
{
	//getcwd(FreeSpace_Directory, 256); // set the directory to our fs2 root
	if (no_fpscap.found())
	{
		Cmdline_NoFPSCap = 1;
	}

	if(loadallweapons_arg.found())
	{
		Cmdline_load_all_weapons = 1;
	}

	if(poof_2d_arg.found())
	{
		Cmdline_2d_poof = 1;
	}

	if(voice_recognition_arg.found())
	{
		Cmdline_voice_recognition = 1;
	}

	if (Radar_Range_Clamp.found())
	{
		if (Radar_Range_Clamp.get_float() > 0.0f)
			Radar_ranges[RR_MAX_RANGES-1] = Radar_Range_Clamp.get_float();
	}

	if (nowarn_arg.found())
	{
		Cmdline_nowarn = 1;
	}

	if (fred2_htl_arg.found())
	{
		Cmdline_FRED2_htl = 1;
	}

	if (timerbar_arg.found()) {
		Cmdline_timerbar = 1;
	}

	if (MissionCRCs.found()) {
		Cmdline_SpewMission_CRCs = 1;
	}

	if (TableCRCs.found()) {
		Cmdline_SpewTable_CRCs = 1;
	}

	// is this a standalone server??
	if (standalone_arg.found()) {
		Is_standalone = 1;
	}

	if(mpnoreturn_arg.found()) {
		Cmdline_mpnoreturn = 1;
	}

	// run with no sound
	if ( nosound_arg.found() ) {
		Cmdline_freespace_no_sound = 1;
		// and since music is automatically unusable...
		Cmdline_freespace_no_music = 1; 
	}

	// run with no music
	if ( nomusic_arg.found() ) {
		Cmdline_freespace_no_music = 1;
	}

	// should we start a network game
	if ( startgame_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}

	// closed network game
	if ( gameclosed_arg.found() ) {
		Cmdline_closed_game = 1;
	}

	// restircted network game
	if ( gamerestricted_arg.found() ) {
		Cmdline_restricted_game = 1;
	}

	// get the name of the network game
	if ( gamename_arg.found() ) {
		Cmdline_game_name = gamename_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_GAMENAME_LEN ) {
			Cmdline_game_name[MAX_GAMENAME_LEN-1] = '\0';
		}
	}

	// get the password for a pssword game
	if ( gamepassword_arg.found() ) {
		Cmdline_game_password = gamepassword_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_PASSWD_LEN ) {
			Cmdline_game_name[MAX_PASSWD_LEN-1] = '\0';
		}
	}

	// set the rank above/below arguments
	if ( allowabove_arg.found() ) {
		Cmdline_rank_above = allowabove_arg.str();
	}
	if ( allowbelow_arg.found() ) {
		Cmdline_rank_below = allowbelow_arg.str();
	}

	// get the port number for games
	if ( port_arg.found() ) {
		Cmdline_network_port = port_arg.get_int();
	}

	// the connect argument specifies to join a game at this particular address
	if ( connect_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_connect_addr = connect_arg.str();
	}

	// see if the multilog flag was set
	if ( multilog_arg.found() ){
		Cmdline_multi_log = 1;
	}	

	// maybe use old-school server-side firing
	if (server_firing_arg.found() ){
		Cmdline_server_firing = 1;
	}

	// maybe use old-school client damage
	if(client_dodamage.found()){
		Cmdline_client_dodamage = 1;
	}	

	// spew pof info
	if(pof_spew.found()){
		Cmdline_spew_pof_info = 1;
	}

	// mouse coords
	if(mouse_coords.found()){
		Cmdline_mouse_coords = 1;
	}

	// net timeout
	if(timeout.found()){
		Cmdline_timeout = timeout.get_int();
	}

	// d3d windowed
	if(window.found()){
		Cmdline_window = 1;
	}
	if(almission_arg.found()){//DTP for autoload mission // developer oritentated
		Cmdline_almission = almission_arg.str();
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}

	if (allslev_arg.found() ) {
		Cmdline_allslev = 1;
	}

	if(dualscanlines_arg.found() ) {
		Cmdline_dualscanlines = 1;
	}

	if(targetinfo_arg.found())
	{
		Cmdline_targetinfo = 1;
	}

	if(nomovies_arg.found() ) {
		Cmdline_nomovies = 1;
	}

	if ( noscalevid_arg.found() ) {
		Cmdline_noscalevid = 1;
	}

	if(noparseerrors_arg.found()) {
		Cmdline_noparseerrors = 1;
	}


	if(mod_arg.found() ) {
		Cmdline_mod = mod_arg.str();

		// be sure that this string fits in our limits
		/* This has to be disabled because the max size is going to be mods*MAX_FILENAME_LEN
		if ( strlen(Cmdline_mod) > MAX_FILENAME_LEN ) {
			Cmdline_mod[MAX_FILENAME_LEN-1] = '\0';
		}*/

		// strip off blank space it it's there
		if ( Cmdline_mod[strlen(Cmdline_mod)-1] == ' ' ) {
			Cmdline_mod[strlen(Cmdline_mod)-1] = '\0';
		}

		// Ok - mod stacking support
		int len = strlen(Cmdline_mod);
		char *modlist = new char[len+2];
		memset(modlist, 0, len+2);
		strcpy(modlist, Cmdline_mod);

		//modlist[len]= '\0'; // double null termination at the end

		// null terminate each individual
		for (int i = 0; i < len; i++)
		{
			if (modlist[i] == ',')
				modlist[i] = '\0';
		}
		
		//copy over - we don't have to delete[] Cmdline_mod because it's a pointer to an automatic global char*
		Cmdline_mod = modlist;
	}


	if (fps_arg.found())
	{
		Show_framerate = 1;
	}

	if(pos_arg.found())
	{
		Cmdline_show_pos = 1;
	}

	if ( safeloading_arg.found() ) {
		Cmdline_safeloading = 1;
	}

	if ( nomotiondebris_arg.found() ) {
		Cmdline_nomotiondebris = 1;
	}

	if( mipmap_arg.found() ) {
		Cmdline_mipmap = 1;
	}

	if( stats_arg.found() ) {
		Cmdline_show_stats = 1;
	}

	if ( fov_arg.found() ) {
		Viewer_zoom = VIEWER_ZOOM_DEFAULT = Cmdline_fov = fov_arg.get_float();
	}

	if( clip_dist_arg.found() ) {
		Min_draw_distance = Cmdline_clip_dist = clip_dist_arg.get_float();
	}

	if (orb_radar.found())
	{
		Cmdline_orb_radar = 1;
	}

    // TBP warp effects -Et1
    if( tbp.found() )
    {
        Cmdline_tbp = 1;
    }

	if ( use_3dwarp.found() ) {
		Cmdline_3dwarp = 1;
	}

	if ( use_warp_flash.found() ) {
		Cmdline_warp_flash = 1;
	}

	// TEMPORARY - REMOVEME!!!
	if ( spec_scale_arg.found() ) {
		Cmdline_spec_scale = spec_scale_arg.get_float();

		if (Cmdline_spec_scale != 1.0f && Cmdline_spec_scale != 2.0f && Cmdline_spec_scale != 4.0f) {
			if (Cmdline_spec_scale < 1.0f)
				Cmdline_spec_scale = 1.0f;
			else if (Cmdline_spec_scale >= 3.0f)
				Cmdline_spec_scale = 4.0f;
			else if (Cmdline_spec_scale < 2.0f)
				Cmdline_spec_scale = 2.0f;
			else if (Cmdline_spec_scale > 2.0f)
				Cmdline_spec_scale = 2.0f;
		}
	}

	// TEMPORARY - REMOVEME!!!
	if ( env_scale_arg.found() ) {
		Cmdline_env_scale = env_scale_arg.get_float();

		if (Cmdline_env_scale != 1.0f && Cmdline_env_scale != 2.0f && Cmdline_env_scale != 4.0f) {
			if (Cmdline_env_scale < 1.0f)
				Cmdline_env_scale = 1.0f;
			else if (Cmdline_env_scale >= 3.0f)
				Cmdline_env_scale = 4.0f;
			else if (Cmdline_env_scale < 2.0f)
				Cmdline_env_scale = 2.0f;
			else if (Cmdline_env_scale > 2.0f)
				Cmdline_env_scale = 2.0f;
		}
	}

	// TEMPORARY - REMOVEME!!!
	if ( alpha_alpha_blend_arg.found() ) {
		Cmdline_alpha_alpha_blend = 1;
	}

	// specular comand lines
	if ( spec_exp_arg.found() ) {
		specular_exponent_value = spec_exp_arg.get_float();
	}

	if ( spec_point_arg.found() ) {
		static_point_factor = spec_point_arg.get_float();
	}

	if ( spec_static_arg.found() ) {
		static_light_factor = spec_static_arg.get_float();
	}

	if ( spec_tube_arg.found() ) {
		static_tube_factor = spec_tube_arg.get_float();
	}

	if ( cell_arg.found() ) {
		Cmdline_cell = 1;
	}

	if ( spec_arg.found() )
	{
		Cmdline_nospec = 0;
	}

	if ( htl_arg.found() ) 
	{
		Cmdline_nohtl = 1;
	}

	if( jpgtga_arg.found() )
	{	  
		Cmdline_jpgtga = 1;
	}

	if( no_set_gamma_arg.found() )
	{
		Cmdline_no_set_gamma = 1;
	}

	if(no_vsync_arg.found() )
	{
		Cmdline_no_vsync = 1;
	}

#ifdef SCP_UNIX
	// no key/mouse grab
	if(no_grab.found()){
		Cmdline_no_grab = 1;
	}
#endif

	if(pcx32_arg.found() )
	{
		Cmdline_pcx32 = 1;
	}

	if ( img2dds_arg.found() ) {
		Cmdline_img2dds = 1;
		// we also can use -jpgtga without the bad memory usage
		Cmdline_jpgtga = 1;
	}

	if(glow_arg.found() )
	{
		Cmdline_noglow = 0;
	}

	if(query_speech_arg.found() )
	{
		Cmdline_query_speech = 1;
	}

	if(ship_choice_3d_arg.found() )
	{
		Cmdline_ship_choice_3d = 1;
	}

	if(show_mem_usage_arg.found()) {
		Cmdline_show_mem_usage = 1;
	}

	if(ingamejoin_arg.found())
	{
		Cmdline_ingamejoin = 1;
	}

	if(start_mission_arg.found())
	{
		Cmdline_start_mission = start_mission_arg.str();

		if(Cmdline_start_mission != NULL && strlen(Cmdline_start_mission) > 0)
		{
			char *temp = Cmdline_start_mission;

			while(*temp)
			{
				if(*temp == '*')
					*temp = '-';

				temp++;
			}
		}
	}

	if(ambient_factor_arg.found())
	{
		Cmdline_ambient_factor = ambient_factor_arg.get_int();
	}

	if(get_flags_arg.found())
	{
		mprintf(("I got to get_flags_arg.found()!!\n"));
		FILE *fp = fopen("flags.lch","w");

		if(fp == NULL)
		{
			MessageBox(NULL,"Error creating flag list for launcher", "Error", MB_OK);
			return false; 
		}

		int easy_flag_size	= sizeof(EasyFlag);
		int flag_size		= sizeof(Flag);

		int num_easy_flags	= sizeof(easy_flags) / easy_flag_size;
		int num_flags		= sizeof(exe_params) / flag_size;

		// Launcher will check its using structures of the same size
		fwrite(&easy_flag_size, sizeof(int), 1, fp);
		fwrite(&flag_size, sizeof(int), 1, fp);

		fwrite(&num_easy_flags, sizeof(int), 1, fp);
		fwrite(&easy_flags, sizeof(easy_flags), 1, fp);

		fwrite(&num_flags, sizeof(int), 1, fp);
		fwrite(&exe_params, sizeof(exe_params), 1, fp);

		fclose(fp);
		return false; 
	}

	if(output_scripting_arg.found())
	{
		Output_scripting_meta = true;
	}

	if(output_sexp_arg.found())
	{
		extern int Num_op_menus;
		extern int Num_submenus;

		FILE *fp = fopen("sexps.html","w");

		if(fp == NULL)
		{
			MessageBox(NULL,"Error creating SEXP operator list", "Error", MB_OK);
			return false; 
		}

		//Header
		fprintf(fp, "<html>\n<head>\n\t<title>SEXP Output: %d.%d.%d</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>Sexp Output - Build %d.%d.%d</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);

		std::vector<int> done_sexp_ids;
		int x,y,z;

		//Output an overview
		fputs("<dl>", fp);
		for(x = 0; x < Num_op_menus; x++)
		{
			fprintf(fp, "<dt><a href=\"#%d\">%s</a></dt>", (op_menu[x].id & OP_CATEGORY_MASK), op_menu[x].name);
			for(y = 0; y < Num_submenus; y++)
			{
				if(((op_submenu[y].id & OP_CATEGORY_MASK) == op_menu[x].id))
				{
					fprintf(fp, "<dd><a href=\"#%d\">%s</a></dd>", op_submenu[y].id & (OP_CATEGORY_MASK | SUBCATEGORY_MASK), op_submenu[y].name);
				}
			}
		}
		fputs("</dl>", fp);

		//Output the full descriptions
		fputs("<dl>", fp);
		for(x = 0; x < Num_op_menus; x++)
		{
			fprintf(fp, "<dt id=\"%d\"><h2>%s</h2></dt>\n", (op_menu[x].id & OP_CATEGORY_MASK), op_menu[x].name);
			fputs("<dd>", fp);
			fputs("<dl>", fp);
			for(y = 0; y < Num_submenus; y++)
			{
				if(((op_submenu[y].id & OP_CATEGORY_MASK) == op_menu[x].id))
				{
					fprintf(fp, "<dt id=\"%d\"><h3>%s</h3></dt>\n", op_submenu[y].id & (OP_CATEGORY_MASK | SUBCATEGORY_MASK), op_submenu[y].name);
					fputs("<dd>", fp);
					fputs("<dl>", fp);
					for(z = 0; z < Num_operators; z++)
					{
						if(((Operators[z].value & OP_CATEGORY_MASK) == op_menu[x].id)
							&& (get_subcategory(Operators[z].value) != -1)
							&& (get_subcategory(Operators[z].value) == op_submenu[y].id))
						{
							output_sexp_html(z, fp);
						}
					}
					fputs("</dl>", fp);
					fputs("</dd>", fp);
				}
			}
			for(z = 0; z < Num_operators; z++)
			{
				if(((Operators[z].value & OP_CATEGORY_MASK) == op_menu[x].id)
					&& (get_subcategory(Operators[z].value) == -1))
				{
					output_sexp_html(z, fp);
				}
			}
			fputs("</dl>", fp);
			fputs("</dd>", fp);
		}
		for(z = 0; z < Num_operators; z++)
		{
			if(!(Operators[z].value & OP_CATEGORY_MASK))
			{
				output_sexp_html(z, fp);
			}
		}
		fputs("</dl>", fp);
		fputs("</body>\n</html>\n", fp);

		fclose(fp);
		return false;
	}

	Cmdline_d3d_lesstmem = !d3d_lesstmem_arg.found();

	if ( no_vbo_arg.found() )
	{
		Cmdline_novbo = 1;
	}

	if ( snd_preload_arg.found() )
	{
		Cmdline_snd_preload = 1;
	}

	if ( alpha_env.found() ) {
		Cmdline_alpha_env = 1;
	}

	if ( env.found() ) {
		Cmdline_env = 1;
	}

	if ( decals.found() ) {
		
		#ifdef WIN32
		MessageBox(NULL, "Decals have been disabled in builds after May 4, 2006.  Please remove the \"-decals\" command line from the launcher to prevent this message from appearing the the future.", "", MB_OK | MB_ICONINFORMATION);
		#endif
	}

	if ( ballistic_gauge.found() ) {
		Cmdline_ballistic_gauge = 1;
	}

	if (wcsaga.found())
	{
		Cmdline_wcsaga = 1;
	}

	if ( cache_bitmaps_arg.found() ) {
		Cmdline_cache_bitmaps = 1;
	}

	if(dis_collisions.found())
		Cmdline_dis_collisions = 1;

	if(dis_weapons.found())
		Cmdline_dis_weapons = 1;

	if ( noibx_arg.found() ) {
		Cmdline_noibx = 1;
	}

	if ( noemissive_arg.found() ) {
		Cmdline_no_emissive = 1;
	}

	if ( ogl_spec_arg.found() ) {
		Cmdline_ogl_spec = ogl_spec_arg.get_float();

		if ( Cmdline_ogl_spec < 0.0f )
			Cmdline_ogl_spec = 0.0f;

		if ( Cmdline_ogl_spec > 128.0f )
			Cmdline_ogl_spec = 128.0f;
	}

	if (rearm_timer_arg.found())
	{
		Cmdline_rearm_timer = 1;
	}

	if (missile_lighting_arg.found())
	{
		Cmdline_missile_lighting = 1;
	}

	if (save_render_targets_arg.found()) {
		Cmdline_save_render_targets = 1;
	}

#if 0
//#ifdef WIN32
	extern uint os_get_window();
	if( fix_bugs.found() )
	{
		MessageBox((HWND)os_get_window(), "Could not get lock on RAID controller, driver may be too old. Auto-bugfixing is disabled", "FS2_Open Warning", MB_ICONWARNING);
	}

	if( disable_crashing.found())
	{
		if(IDRETRY == MessageBox((HWND)os_get_window(), "Error: A CTD bug occured before fs2_open could initialize the anti-crashing subsystem. FS2_Open will now crash. Press retry to try to initialize the subsystem again, or cancel to give up and crash", "FS2_Open Error", MB_ICONWARNING | MB_RETRYCANCEL))
		{
			MessageBox((HWND)os_get_window(), "Sorry, it didn't really work. FS2_Open still won't start, but it will close unexpectedly. Press OK to continue closing unexpectedly.", "FS2_Open Error", MB_ICONWARNING);
		}
		else
		{
			/*char Filepath[1024];
			char* Filename;
			GetModuleFileName(hInst, Filepath, sizeof(Filepath));
			Filename = strrchr(Filepath, '\\');
			if(Filename == NULL)
			{
				Filename = strrchr(Filepath, '/');
				if(Filename == NULL)
				{
					Filename = strrchr(Filepath, ':');
					if(Filename == NULL)
					{
						Filename = Filepath;
					}
				}
			}
			strcat(Filename, " - Application Error");*/
			char Filename[128];
			strcpy(Filename, "fs2_open.exe - Application Error");

			if(IDCANCEL == MessageBox((HWND)os_get_window(), "The instruction at \"0x00001337\" referenced memory at \"0x00000000\". The memory could not be \"read\"\n\nClick on OK to terminate the program\nClick on CANCEL to debug the program", Filename, MB_ICONERROR | MB_OKCANCEL))
			{
				MessageBox((HWND)os_get_window(), "What!? Debug something yourself? No, what you want to do is send an error report to Microsoft.", "Error", MB_ICONERROR);
			}
		}
		return false;
	}
#endif

	return true; 
}


int fred2_parse_cmdline(int argc, char *argv[])
{
	if (argc > 1) {
		// kind of silly -- combine arg list into single string for parsing,
		// but it fits with the win32-centric existing code.
		char *cmdline = NULL;
		unsigned int arglen = 0;
		int i;
		for (i = 1;  i < argc;  i++)
			arglen += strlen(argv[i]);
		if (argc > 2)
			arglen += argc + 2; // leave room for the separators
		cmdline = new char [arglen+1];
		i = 1;
		memset(cmdline, 0, arglen+1); // clear it out

		strcpy(cmdline, argv[i]);
		for (i=2; i < argc;  i++) {
			strcat(cmdline, " ");
			strcat(cmdline, argv[i]);
		}
		os_init_cmdline(cmdline);
		delete [] cmdline;
	} else {
		// no cmdline args
		os_init_cmdline("");
	}

	return SetCmdlineParams();
}


int parse_cmdline(char *cmdline)
{
//	mprintf(("I got to parse_cmdline()!!\n"));

	os_init_cmdline(cmdline);

	// --------------- Kazan -------------
	// If you're looking for the list of if (someparam.found()) { cmdline_someparam = something; } look above at this function
	// I did this because of fred2_parse_cmdline()
	return SetCmdlineParams();
}
