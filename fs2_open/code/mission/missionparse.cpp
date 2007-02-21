/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionParse.cpp $
 * $Revision: 2.217 $
 * $Date: 2007-02-21 01:44:02 $
 * $Author: Goober5000 $
 *
 * main upper level code for parsing stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.216  2007/02/20 04:20:18  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 2.215  2007/02/18 06:16:47  Goober5000
 * revert Bobboau's commits for the past two months; these will be added in later in a less messy/buggy manner
 *
 * Revision 2.214  2007/02/13 01:46:03  Goober5000
 * fix a hideous bug with the knossos special warp stuff
 * (how did this ever actually work correctly?)
 *
 * Revision 2.213  2007/02/11 21:26:34  Goober5000
 * massive shield infrastructure commit
 *
 * Revision 2.212  2007/02/10 03:17:31  Goober5000
 * made support ship shield control a bit less of a hack
 *
 * Revision 2.211  2007/02/08 07:39:32  Goober5000
 * fix two bugs:
 * --default ship flags in the iff_defs table were not correctly translated from parse flags to ship/object flags
 * --ships were created with default allowed comm orders regardless of which team they were on
 *
 * Revision 2.210  2007/02/03 23:16:33  Goober5000
 * fix an old subtle bug
 *
 * Revision 2.209  2007/01/14 14:03:33  bobboau
 * ok, something aparently went wrong, last time, so I'm commiting again
 * hopefully it should work this time
 * damnit WORK!!!
 *
 * Revision 2.208  2007/01/07 21:28:10  Goober5000
 * yet more tweaks to the WCS death scream stuff
 * added a ship flag to force screaming
 *
 * Revision 2.207  2007/01/07 12:55:57  taylor
 * safety check to make sure that we don't end up with non-player-usable weapons in the weaponselect pool (Mantis bug #1196)
 *
 * Revision 2.206  2007/01/07 01:00:18  Goober5000
 * convert a mission variable to a mission flag
 *
 * Revision 2.205  2007/01/07 00:24:18  Goober5000
 * move this to the Message location of the mission
 *
 * Revision 2.204  2007/01/07 00:01:28  Goober5000
 * add a feature for specifying the source of Command messages
 *
 * Revision 2.203  2006/12/28 00:59:32  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.202  2006/11/25 06:37:06  Goober5000
 * holy crap, this had the potential to mark the locked-true sexp as known-false, which would seriously screw a lot of stuff up
 *
 * Revision 2.201  2006/11/15 04:02:31  Goober5000
 * a much better fix for built-in arrival messages... it needs a new feature to fully work for custom wings though
 *
 * Revision 2.200  2006/11/06 06:36:44  taylor
 * updated/fixed modelanim code
 *
 * Revision 2.199  2006/11/06 05:44:33  taylor
 * remove a basically useless debug message
 * fix parse_init() so that it doesn't absolutely have to init sexps (doesn't need them when getting basic info, and causes memory fragmentation otherwise)
 * fix multiplayer mission info getting to just get basic info (fixes a couple of minor multi issues that were already fixed in single)
 *
 * Revision 2.198  2006/10/09 20:50:13  Goober5000
 * betterized some things with mission arrivals and departures
 *
 * Revision 2.197  2006/10/09 05:25:18  Goober5000
 * make sexp nodes dynamic
 *
 * Revision 2.196  2006/09/11 06:47:07  taylor
 * fixes for stuff_string() bounds checking
 * compiler warning and stict compiling fixes
 *
 * Revision 2.195  2006/09/11 06:08:09  taylor
 * make Species_info[] and Asteroid_info[] dynamic
 *
 * Revision 2.194  2006/09/04 05:54:14  wmcoolmon
 * Fix bug (under Win32) where FRED would warn about unacceptable accepted orders, but not do anything about them.
 *
 * Revision 2.193  2006/08/21 04:02:41  Goober5000
 * a tweak and a fix
 *
 * Revision 2.192  2006/08/20 00:51:06  taylor
 * maybe optimize the (PI/2), (PI*2) and (RAND_MAX/2) stuff a little bit
 *
 * Revision 2.191  2006/08/19 21:46:05  Goober5000
 * disable duplicate model texture replace
 *
 * Revision 2.190  2006/08/06 18:47:29  Goober5000
 * add the multiple background feature
 * --Goober5000
 *
 * Revision 2.189  2006/07/30 20:01:56  Kazan
 * resolve 1018 and an interface problem in fred2's ship editor
 *
 * Revision 2.188  2006/07/17 00:10:00  Goober5000
 * stage 2 of animation fix (add base frame time to each ship)
 * --Goober5000
 *
 * Revision 2.187  2006/07/13 06:11:52  Goober5000
 * * better formatting for substitute music options
 * * better handling of all special FSO comment tags
 * --Goober5000
 *
 * Revision 2.186  2006/07/06 21:00:13  Goober5000
 * remove obsolete (and hackish) flag
 * --Goober5000
 *
 * Revision 2.185  2006/07/06 20:46:39  Goober5000
 * WCS screaming stuff
 * --Goober5000
 *
 * Revision 2.184  2006/07/06 06:04:51  Goober5000
 * fix subtle change from retail... dunno if it's a bug, but best to stay on the safe side
 * --Goober5000
 *
 * Revision 2.183  2006/07/06 04:06:03  Goober5000
 * 1) complete (almost) changeover to reorganized texture mapping system
 * 2) finally fix texture animation; textures now animate at the correct speed
 * --Goober5000
 *
 * Revision 2.182  2006/07/04 07:42:48  Goober5000
 * --in preparation for fixing an annoying animated texture bug, reorganize the various texture structs and glow point structs and clarify several parts of the texture code :P
 * --this breaks animated glow maps, and animated regular maps still aren't fixed, but these will be remedied shortly
 * --Goober5000
 *
 * Revision 2.181  2006/07/01 00:30:48  Goober5000
 * fix the "Game of TAG" bug; hopefully this won't unfix something else
 * --Goober5000
 *
 * Revision 2.180  2006/06/04 01:01:53  Goober5000
 * add fighterbay restriction code
 * --Goober5000
 *
 * Revision 2.179  2006/06/02 09:05:30  karajorma
 * Team Loadout changes to accept variables names as legitimate values for ship class and quantity in loadout.
 * Added the new alt class system
 *
 * Revision 2.178  2006/05/31 03:05:42  Goober5000
 * some cosmetic changes in preparation for bay arrival/departure code
 * --Goober5000
 *
 * Revision 2.177  2006/05/21 22:57:30  Goober5000
 * fix for Mantis #750
 * --Goober5000
 *
 * Revision 2.176  2006/05/21 03:58:58  Goober5000
 * fix Mantis #834
 * --Goober5000
 *
 * Revision 2.175  2006/05/20 02:03:01  Goober5000
 * fix for Mantis #755, plus make the missionlog #defines uniform
 * --Goober5000
 *
 * Revision 2.174  2006/05/19 04:47:40  Goober5000
 * formatting and clarification, plus fix for Mantis #911
 * --Goober5000
 *
 * Revision 2.173  2006/05/14 15:54:16  karajorma
 * Fixes for the Targetable-as-bomb error from Mantis 0000910 (amongst others)
 *
 * Revision 2.172  2006/05/13 07:29:52  taylor
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
 * Revision 2.171  2006/04/20 06:32:07  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.170  2006/04/16 11:58:11  karajorma
 * Some how managed to fail to commit the primary-locked and secondary-locked ship flags.
 * No idea how. Oh well here they are.
 *
 * Revision 2.169  2006/04/05 16:12:41  karajorma
 * Changes to support the new Enable/Disable-Builtin-Messages SEXP
 *
 * Revision 2.168  2006/03/24 07:38:35  wmcoolmon
 * New subobject animation stuff and Lua functions.
 *
 * Revision 2.167  2006/03/01 00:46:11  Goober5000
 * more FS1 import tweaks
 *
 * Revision 2.166  2006/02/26 23:23:30  wmcoolmon
 * Targetable as bomb SEXPs and dialog stuff; made invulnerable an object flag in both FRED and FS2.
 *
 * Revision 2.165  2006/02/26 08:06:57  taylor
 * another non-linear campaign fix, this one affected PVs
 *
 * Revision 2.164  2006/02/26 05:32:38  Goober5000
 * guess this is still needed
 *
 * Revision 2.163  2006/02/25 22:06:57  Goober5000
 * import tweak
 *
 * Revision 2.162  2006/02/24 07:34:07  taylor
 * fix custom loading screens that I manage to break yet again
 * add a "MaxFPS" registry/ini option to specify a FPS cap, useful if you can't make use of v-sync for some reason
 *
 * Revision 2.161  2006/02/17 08:36:16  Goober5000
 * bother those variables
 *
 * Revision 2.160  2006/02/17 08:13:53  Goober5000
 * fix subtle bug
 * --Goober5000
 *
 * Revision 2.159  2006/02/16 05:18:48  taylor
 * all just getting basic info in parse_mission_info(), this fixes several parsing/warning issues that can show up
 *   from going in the techroom and some similar FRED2 issues as this stuff doesn't get reset between get_mission_info() calls.
 *   this also should make the mission simulator screen load a bit faster
 *
 * Revision 2.158  2006/02/12 05:23:16  Goober5000
 * additional fixes and enhancements for substitute music
 * --Goober5000
 *
 * Revision 2.157  2006/02/12 01:27:47  Goober5000
 * more cool work on importing, music handling, etc.
 * --Goober5000
 *
 * Revision 2.156  2006/02/11 21:24:05  Goober5000
 * this isn't needed now
 * --Goober5000
 *
 * Revision 2.155  2006/02/11 07:31:55  Goober5000
 * two things I missed before
 * --Goober5000
 *
 * Revision 2.154  2006/02/11 02:58:23  Goober5000
 * yet more various and sundry fixes
 * --Goober5000
 *
 * Revision 2.153  2006/02/11 01:02:28  Goober5000
 * more fixage
 * --Goober5000
 *
 * Revision 2.152  2006/02/11 00:13:55  Goober5000
 * more FS1 import goodness
 * --Goober5000
 *
 * Revision 2.151  2006/02/10 23:33:39  Goober5000
 * fixed an old Volition bug unmasked by my parse changes
 * --Goober5000
 *
 * Revision 2.150  2006/02/06 02:06:01  wmcoolmon
 * Various fixes; very beginnings of Directives scripting support
 *
 * Revision 2.149  2006/02/02 08:12:47  Goober5000
 * ugh, more ship/wing fixage
 * --Goober5000
 *
 * Revision 2.148  2006/02/02 05:48:17  Goober5000
 * fixed a FRED problem brought on by the parse reordering
 * --Goober5000
 *
 * Revision 2.147  2006/01/31 17:19:20  taylor
 * oh the shame   (/me imagines the big grin on Goober's face right now :p)
 *
 * Revision 2.146  2006/01/31 15:44:29  taylor
 * we need to skip to the start of the string, not the end
 *
 * Revision 2.145  2006/01/31 04:24:25  Goober5000
 * stuff
 *
 * Revision 2.144  2006/01/31 02:02:33  Goober5000
 * fix a potential problem with replacement range
 * --Goober5000
 *
 * Revision 2.143  2006/01/31 01:53:37  Goober5000
 * update FSM import for FSPort v3.0
 * --Goober5000
 *
 * Revision 2.142  2006/01/30 19:35:41  taylor
 * fix parsing of reinforcements
 *
 * Revision 2.141  2006/01/30 06:30:03  taylor
 * dynamic starfield bitmaps
 *
 * Revision 2.140  2006/01/28 02:59:55  Goober5000
 * fixed the bug where wings couldn't be initially docked to stuff
 * --Goober5000
 *
 * Revision 2.139  2006/01/26 04:01:58  Goober5000
 * spelling
 *
 * Revision 2.138  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.137  2006/01/14 06:25:11  taylor
 * make sure rship gets initialized before use
 *
 * Revision 2.136  2006/01/14 05:36:06  Goober5000
 * tweaky tweak
 * --Goober5000
 *
 * Revision 2.135  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.134  2006/01/04 08:19:21  taylor
 * fixes for regular texture replacement
 *
 * Revision 2.133  2005/12/29 08:08:36  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.132  2005/12/29 00:56:45  phreak
 * loading code for flipped briefing icons.
 *
 * Revision 2.131  2005/12/28 22:31:03  taylor
 * unused variable (to make GCC shut up)
 *
 * Revision 2.130  2005/12/08 15:11:29  taylor
 * a few game_busy() changes
 *
 * Revision 2.129  2005/12/06 02:57:57  taylor
 * just to be on the safe side
 *
 * Revision 2.128  2005/11/24 08:46:10  Goober5000
 * * cleaned up mission_do_departure
 *   * fixed a hidden crash (array index being -1; would only
 * be triggered for ships w/o subspace drives under certain conditions)
 *   * removed finding a new fighterbay target because it might screw up missions
 *   * improved clarity, code flow, and readability :)
 * * added custom AI flag for disabling warpouts if navigation subsystem fails
 * --Goober5000
 *
 * Revision 2.127  2005/11/21 02:43:37  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 * Revision 2.126  2005/11/21 00:46:12  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.125  2005/11/18 07:53:27  Goober5000
 * made arrival invalidation more efficient
 * (probably only a small speedup, but meh)
 * --Goober5000
 *
 * Revision 2.124  2005/10/31 09:09:41  Goober5000
 * bit of tweakage
 * --Goober5000
 *
 * Revision 2.123  2005/10/30 20:42:45  Goober5000
 * fix the support ship spawning bug
 * --Goober5000
 *
 * Revision 2.122  2005/10/30 20:03:39  taylor
 * add a bunch of Assert()'s and NULL checks to either help debug or avoid errors
 * fix Mantis bug #381
 * fix a small issue with the starfield bitmap removal sexp since it would read one past the array size
 *
 * Revision 2.121  2005/10/29 22:09:29  Goober5000
 * multiple ship docking implemented for initially docked ships
 * --Goober5000
 *
 * Revision 2.120  2005/10/22 22:25:23  Goober5000
 * made Cmdline_UseNewAI toggle the mission flag, plus fixed a few other flag-related potential bugs
 * --Goober5000
 *
 * Revision 2.119  2005/10/10 17:21:05  taylor
 * remove NO_NETWORK
 *
 * Revision 2.118  2005/10/09 08:03:20  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.117  2005/10/02 23:01:17  Goober5000
 * removed a redundant action
 * --Goober5000
 *
 * Revision 2.116  2005/09/29 04:26:08  Goober5000
 * parse fixage
 * --Goober5000
 *
 * Revision 2.115  2005/09/27 08:09:13  Goober5000
 * fix debug warning
 * --Goober5000
 *
 * Revision 2.114  2005/09/27 02:36:57  Goober5000
 * clarification
 * --Goober5000
 *
 * Revision 2.113  2005/09/25 18:44:51  taylor
 * fix Subsys_status leak, wasn't a problem in game but can be touchy on exit
 *
 * Revision 2.112  2005/09/25 08:25:15  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.111  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.110  2005/09/24 02:24:29  Goober5000
 * bah
 * --Goober5000
 *
 * Revision 2.109  2005/09/24 01:50:08  Goober5000
 * a bunch of support ship bulletproofing
 * --Goober5000
 *
 * Revision 2.108  2005/08/31 08:09:50  Goober5000
 * All wings will now form on their respective leaders, not just certain starting wings.
 * If this breaks anything, the old code is commented out.  You know who to yell at. ;)
 * --Goober5000
 *
 * Revision 2.107  2005/08/31 07:54:32  Goober5000
 * Wings will now form on their leader, not necessarily the player.  This is to
 * account for cases where the player is not the leader of his wing.  It has the
 * side effect of causing wings that arrive later in the mission to form on *their*
 * leader rather than the player, but this is probably the preferred behavior (at
 * least judging from Mike Kulas's comment of 5/9/98).  This does create a
 * difference from retail AI, but it should be only cosmetic, since ships only form
 * on a wing until they are ordered (via comm or sexp) to do something else.
 * --Goober5000
 *
 * Revision 2.106  2005/08/26 00:56:17  taylor
 * send out multi pings during level load to prevent timeouts on slower computers (does not break network compatibility with older, or retail, clients/servers)
 *
 * Revision 2.105  2005/08/23 09:18:07  Goober5000
 * ensure init/reset of goals works cleanly
 * --Goober5000
 *
 * Revision 2.104  2005/08/23 07:49:24  Goober5000
 * removed a thing I apparently forgot to remove on 2005/07/12
 * --Goober5000
 *
 * Revision 2.103  2005/08/22 22:24:22  Goober5000
 * some tweaks to parselo, plus ensure that Unicode files don't crash
 * --Goober5000
 *
 * Revision 2.102  2005/08/21 06:19:31  Goober5000
 * clarified the comment on my original move of the wing departure information assignment
 * --Goober5000
 *
 * Revision 2.101  2005/07/25 08:22:00  Goober5000
 * more bugs and tweaks
 * --Goober5000
 *
 * Revision 2.100  2005/07/25 05:24:16  Goober5000
 * cleaned up some command line and mission flag stuff
 * --Goober5000
 *
 * Revision 2.99  2005/07/25 03:13:25  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 2.98  2005/07/22 10:18:39  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.97  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.96  2005/07/13 02:32:22  Goober5000
 * this seems like it should be handled by the code already...
 * let's remove it and see what happens :p
 * --Goober5000
 *
 * Revision 2.95  2005/07/13 02:30:53  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 2.94  2005/07/13 02:01:29  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.93  2005/07/13 00:44:22  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.92  2005/05/12 17:49:14  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.91  2005/04/28 05:29:30  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.90  2005/04/28 01:35:26  wmcoolmon
 * stuff_byte to stuff_ubyte; does the same thing, but with a better name.
 *
 * Revision 2.89  2005/04/21 15:59:41  taylor
 * sexp to swap the skybox in mission, not tested in FRED, hopefully I didn't mess this up too much
 *
 * Revision 2.88  2005/04/12 02:07:00  phreak
 * ambient_light_level added to the mission structure
 *
 * Revision 2.87  2005/04/05 05:53:19  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.86  2005/03/27 12:28:33  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.85  2005/03/25 06:57:35  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.84  2005/03/08 03:50:22  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.83  2005/03/03 06:05:29  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.82  2005/03/02 21:24:45  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.81  2005/03/01 06:55:41  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.80  2005/01/31 23:27:54  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.79  2005/01/26 21:42:19  Goober5000
 * important bugfix
 * --Goober5000
 *
 * Revision 2.78  2005/01/21 08:56:50  taylor
 * make Subsys_status dynamic but allocate in blocks for speed and to help prevent memory fragmentation
 *
 * Revision 2.77  2005/01/18 00:14:37  Goober5000
 * clarified a bunch of sexp stuff and fixed a bug
 * --Goober5000
 *
 * Revision 2.76  2005/01/11 21:38:50  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.75  2005/01/03 01:05:08  phreak
 * made custom loading screens work if only one resolution has been specified.
 *
 * Revision 2.74  2004/12/22 21:48:21  taylor
 * safety check against very large missions
 *
 * Revision 2.73  2004/12/15 17:32:19  Goober5000
 * move wing name initialization to ship_level_init
 * --Goober5000
 *
 * Revision 2.72  2004/12/14 14:46:13  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.71  2004/11/21 11:31:46  taylor
 * newline fix (got tired of seeing this)
 *
 * Revision 2.70  2004/10/31 02:04:34  Goober5000
 * added Knossos_warp_ani_used flag for taylor
 * --Goober5000
 *
 * Revision 2.69  2004/10/15 09:21:55  Goober5000
 * cleaned up some sexp stuff and added wing capability to kamikaze sexp
 * --Goober5000
 *
 * Revision 2.68  2004/10/12 22:47:14  Goober5000
 * added toggle-subsystem-scanning ship flag
 * --Goober5000
 *
 * Revision 2.67  2004/10/12 07:34:45  Goober5000
 * added contrail speed threshold
 * --Goober5000
 *
 * Revision 2.66  2004/10/11 22:29:25  Goober5000
 * added the no-bank ship flag (which works) and the affected-by-gravity flag
 * (which won't work until I implement gravity points)
 * --Goober5000
 *
 * Revision 2.65  2004/09/01 00:22:36  Goober5000
 * fixed a small bug in the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.64  2004/07/27 20:09:12  Kazan
 * making another bug a little bit more dead inside :D
 *
 * Revision 2.63  2004/07/26 20:47:37  Kazan
 * remove MCD complete
 *
 * Revision 2.62  2004/07/12 16:32:54  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.61  2004/07/01 01:12:32  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.60  2004/05/11 02:52:12  Goober5000
 * completed the FRED import conversion stuff that I started ages ago
 * --Goober5000
 *
 * Revision 2.59  2004/05/10 10:51:53  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
 * Revision 2.58  2004/05/10 08:03:30  Goober5000
 * fixored the handling of no lasers and no engines... the tests should check the ship,
 * not the object
 * --Goober5000
 *
 * Revision 2.57  2004/05/03 21:22:22  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.56  2004/04/13 05:42:45  Goober5000
 * fixed the custom hitpoints subsystem bug
 * --Goober5000
 *
 * Revision 2.55  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.54  2004/02/07 00:48:53  Goober5000
 * made FS2 able to account for subsystem mismatches between ships.tbl and the
 * model file - e.g. communication vs. communications
 * --Goober5000
 *
 * Revision 2.53  2004/02/05 14:31:44  Goober5000
 * fixed a few random bugs
 * --Goober5000
 *
 * Revision 2.52  2004/02/04 08:41:04  Goober5000
 * made code more uniform and simplified some things,
 * specifically shield percentage and quadrant stuff
 * --Goober5000
 *
 * Revision 2.51  2004/01/30 07:39:08  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.50  2004/01/14 06:34:07  Goober5000
 * made set-support-ship number align with general FS convention
 * --Goober5000
 *
 * Revision 2.49  2003/10/23 23:48:03  phreak
 * added code for the mission parser to recognize user defined skyboxes
 *
 * Revision 2.48  2003/10/16 16:38:16  Kazan
 * couple more types in species_defs.cpp, also finished up "Da Species Upgrade"
 *
 * Revision 2.47  2003/10/15 22:03:25  Kazan
 * Da Species Update :D
 *
 * Revision 2.46  2003/10/08 06:32:06  argv
 * Negative repeat count means repeat indefinitely. A trivial change, it should not break anything.
 *
 * Revision 2.45  2003/09/30 04:05:09  Goober5000
 * updated FRED to import FS1 default weapons loadouts as well as missions
 * --Goober5000
 *
 * Revision 2.44  2003/09/28 21:22:59  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 2.43  2003/09/13 06:02:06  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.40  2003/09/06 20:41:52  wmcoolmon
 * Added "+Subsystem Repair Ceiling:" after "+Hull Repair Ceiling:" (formerly "+Support Repair Ceiling:"
 *
 * Revision 2.39  2003/09/06 19:09:24  wmcoolmon
 * Added optional mission parameter "+Support Repair Ceiling", which sets what percentage a support ship can repair a ship's hull to.
 *
 * Revision 2.38  2003/09/05 10:41:48  Goober5000
 * bah - fixed a dumb bug
 * --Goober5000
 *
 * Revision 2.37  2003/09/05 04:25:29  Goober5000
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
 * Revision 2.36  2003/05/09 23:52:01  phreak
 * changed around code in parse_mission_info() to support user-defined loading screens
 *
 * Revision 2.35  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.34  2003/03/25 07:03:30  Goober5000
 * added beginning functionality for $Texture Replace implementation in FRED
 * --Goober5000
 *
 * Revision 2.33  2003/01/27 07:46:33  Goober5000
 * finished up my fighterbay code - ships will not be able to arrive from or depart into
 * fighterbays that have been destroyed (but you have to explicitly say in the tables
 * that the fighterbay takes damage in order to enable this)
 * --Goober5000
 *
 * Revision 2.32  2003/01/26 00:20:01  Goober5000
 * oops - fixed the nebula bug that wasn't completely fixed :)
 * --Goober5000
 *
 * Revision 2.31  2003/01/19 22:20:23  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.30  2003/01/19 09:14:05  Goober5000
 * fixed fiddly code
 * --Goober5000
 *
 * Revision 2.29  2003/01/19 09:10:40  Goober5000
 * more tweaky bug fixes
 * --Goober5000
 *
 * Revision 2.28  2003/01/19 08:37:52  Goober5000
 * fixed two dumb bugs in the set-support-ship code
 * --Goober5000
 *
 * Revision 2.27  2003/01/19 07:45:38  Goober5000
 * actually added the set-support-ship sexp; much of the other commit was
 * groundwork (data types and stuff)
 * --Goober5000
 *
 * Revision 2.26  2003/01/19 07:02:16  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.25  2003/01/18 23:25:39  Goober5000
 * made "no-subspace-drive" applicable to all ships and fixed a really *STUPID*
 * bug that made FRED keep crashing (missing comma, bleagh!)
 * --Goober5000
 *
 * Revision 2.24  2003/01/18 09:25:41  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.23  2003/01/17 04:57:17  Goober5000
 * Allowed selection of either $Texture Replace, which keeps track of individual
 * replacement textures for a ship, or $Duplicate Model Texture Replace, which
 * duplicates a model and reskins it.  Use $Duplicate Model Texture Replace
 * if you want to substitute an animated texture or a transparent texture.
 * --Goober5000
 *
 * Revision 2.22  2003/01/17 01:48:50  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.21  2003/01/15 08:57:23  Goober5000
 * assigning duplicate models to ships now works; committing so I have a base
 * to fall back to as I work on texture replacement
 * --Goober5000
 *
 * Revision 2.20  2003/01/15 07:09:09  Goober5000
 * changed most references to modelnum to use ship instead of ship_info --
 * this will help with the change-model sexp and any other instances of model
 * changing
 * --Goober5000
 *
 * Revision 2.19  2003/01/15 05:24:23  Goober5000
 * added texture replacement parse - will be implemented later
 * --Goober5000
 *
 * Revision 2.18  2003/01/14 06:13:23  Goober5000
 * argh - fixed the stupid message persona bug with <any wingman>
 * Bobboau, please check your code!
 * --Goober5000
 *
 * Revision 2.17  2003/01/13 02:09:12  wmcoolmon
 * Change mission flag for nebula trails. Also changed code to set flags as necessary
 *
 * Revision 2.16  2003/01/11 01:00:25  wmcoolmon
 * Added code for "Ship Trails override Nebula"
 *
 * Revision 2.15  2003/01/06 20:29:28  Goober5000
 * nasssty bugs, we hates them, yesss we do, preciousssss
 * :)
 * --Goober5000
 *
 * Revision 2.14  2003/01/06 17:19:14  Goober5000
 * hmm... moved +Squad Logo to immediately beneath $Name
 * --Goober5000
 *
 * Revision 2.13  2003/01/06 17:14:52  Goober5000
 * added wing configurable squad logos - put +Squad Logo: filename.pcx as
 * the last entry in each wing that you want (but the player's squad logo will
 * still be the squad logo for the player's wing)
 * --Goober5000
 *
 * Revision 2.12  2003/01/03 21:58:08  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.11  2003/01/02 00:35:21  Goober5000
 * added don't-collide-invisible and collide-invisible sexps
 * --Goober5000
 *
 * Revision 2.10  2003/01/01 23:33:34  Goober5000
 * added ship-vaporize and ship-no-vaporize sexps
 * --Goober5000
 *
 * Revision 2.9  2002/12/27 02:57:51  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.8  2002/12/23 05:18:52  Goober5000
 * Squashed some Volition bugs! :O Some of the sexps for dealing with more than
 * one ship would return after only dealing with the first ship.
 *
 * Also added the following sexps:
 * is-ship-stealthed
 * ship-force-stealth
 * ship-force-nostealth
 * ship-remove-stealth-forcing
 *
 * They toggle the stealth flag on and off.  If a ship is forced stealthy, it won't even
 * show up for friendly ships.
 * --Goober5000
 *
 * Revision 2.7  2002/12/22 22:59:04  Goober5000
 * hack: turn on ship trails if in nebula - restores backward compatibility
 * --Goober5000
 *
 * Revision 2.6  2002/12/13 08:13:29  Goober5000
 * small tweaks and bug fixes for the ballistic primary conversion
 * ~Goober5000~
 *
 * Revision 2.5  2002/12/10 05:43:34  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.4  2002/12/03 23:05:13  Goober5000
 * implemented beam-free-all-by-default mission flag
 *
 * Revision 2.3  2002/12/03 00:00:52  Goober5000
 * fixed player entry delay bug
 *
 * Revision 2.2  2002/11/14 06:15:02  bobboau
 * added nameplate code
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 63    9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 62    9/09/99 3:53a Andsager
 * Only reposition HUGE ships to center of knossos device on warp in
 * 
 * 61    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 60    8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 59    8/25/99 10:06a Jefff
 * vasudan pilots get a vasudan support ship. 
 * 
 * 58    8/24/99 5:27p Andsager
 * Make subsystems with zero strength before mission blown off.  Protect
 * red alert pilot against dying between orders and jump.
 * 
 * 57    8/18/99 10:07p Johnson
 * Fix Fred bug in positioning of Knossos device (when trying to warp in
 * through self)
 * 
 * 56    8/18/99 3:57p Andsager
 * Add warning for invalid alt_name.
 * 
 * 55    8/18/99 3:48p Andsager
 * Make support ship take default name and not 0th alt_name.
 * 
 * 54    8/16/99 3:53p Andsager
 * Add special warp in interface in Fred and saving / reading.
 * 
 * 53    8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 52    8/03/99 5:35p Andsager
 * Dont draw target dot for instructor in training mission
 * 
 * 51    7/30/99 7:01p Dave
 * Dogfight escort gauge. Fixed up laser rendering in Glide.
 * 
 * 50    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 49    7/26/99 5:50p Dave
 * Revised ingame join. Better? We'll see....
 * 
 * 48    7/19/99 3:01p Dave
 * Fixed icons. Added single transport icon.
 * 
 * 47    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 46    7/13/99 5:03p Alanl
 * make sure object sounds get assigned to ships
 * 
 * 45    7/11/99 2:14p Dave
 * Added Fred names for the new icon types.
 * 
 * 44    7/02/99 4:\31p Dave
 * Much more sophisticated lightning support.
 * 
 * 43    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 42    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 41    6/21/99 1:34p Alanl
 * event music tweaks
 * 
 * 40    6/16/99 10:20a Dave
 * Added send-message-list sexpression.
 * 
 * 39    6/14/99 2:06p Andsager
 * Default load brown asteroids
 * 
 * 38    6/10/99 11:06a Andsager
 * Mission designed selection of asteroid types.
 * 
 * 37    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 36    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 35    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 34    4/26/99 12:49p Andsager
 * Add protect object from beam support to Fred
 * 
 * 33    4/16/99 2:34p Andsager
 * Second pass on debris fields
 * 
 * 32    4/15/99 5:00p Andsager
 * Frist pass on Debris field
 * 
 * 31    4/07/99 6:22p Dave
 * Fred and FreeSpace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 30    3/31/99 9:52a Andsager
 * generalization for debris field
 * 
 * 29    3/30/99 5:40p Dave
 * Fixed reinforcements for TvT in multiplayer.
 * 
 * 28    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 27    3/24/99 6:23p Dave
 * Make sure we only apply squadron changes to the player in single-player
 * campaign mode.
 * 
 * 26    3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 25    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 24    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 23    2/24/99 2:25p Dave
 * Fixed up chatbox bugs. Made squad war reporting better. Fixed a respawn
 * bug for dogfight more.
 * 
 * 22    2/23/99 8:11p Dave
 * Tidied up dogfight mode. Fixed TvT ship type problems for alpha wing.
 * Small pass over todolist items.
 * 
 * 21    2/17/99 2:10p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 20    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 19    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 18    2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 17    2/05/99 10:38a Johnson
 * Fixed improper object array reference. 
 * 
 * 16    2/04/99 1:23p Andsager
 * Apply max spark limit to ships created in mission parse
 * 
 * 15    2/03/99 12:42p Andsager
 * Add escort priority.  Modify ship_flags_dlg to include field.  Save and
 * Load.  Add escort priority field to ship.
 * 
 * 14    1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 13    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 12    1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 11    1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 10    11/14/98 5:32p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 9     11/12/98 12:13a Dave
 * Tidied code up for multiplayer test. Put in network support for flak
 * guns.
 * 
 * 8     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 7     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 6     10/29/98 9:23p Dave
 * Removed minor bug concerning externalization of campaign files.
 * 
 * 5     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 4     10/20/98 10:44a Andsager
 * Add comment for creating sparks before mission starts
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 503   9/21/98 8:46p Dave
 * Put in special check in fred for identifying unknown ships.
 * 
 * 502   9/14/98 5:09p Dave
 * Massive hack to always ignore m-clash
 * 
 * 501   9/14/98 3:40p Allender
 * better error checking for invalid number of waves for player wings in a
 * multiplayer game.  Better popup message in FreeSpace side.
 * 
 * 500   9/11/98 2:05p Allender
 * make reinforcements work correctly in multiplayer games.  There still
 * may be a team vs team issue that I haven't thought of yet :-(
 * 
 * 499   8/07/98 9:48a Allender
 * changed how SF_FROM_PLAYER_WING is assigned for team vs. team games
 * 
 * 498   7/16/98 2:22p Allender
 * don't do wave check in single player
 * 
 * 497   7/13/98 5:19p Dave
 * 
 * 496   7/13/98 3:15p Allender
 * don't allow multiple waves for any player wings
 * 
 * 495   7/10/98 12:11a Allender
 * fixed problem where missions files of 0 length would cause game to
 * crash
 * 
 * 494   5/21/98 3:31p Allender
 * fix bug where Ship_obj_list was getting overwritten by the exited ships
 * list
 * 
 * 493   5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 492   5/18/98 4:41p Comet
 * allender: fix problem where ships in wings were not deleted on clients
 * when a new wing create packet came in.  A serious hack of all hacks 
 * 
 * 491   5/18/98 3:37p Jasen
 * move Int3() about too many ships in wing to before where ship actually
 * gets created
 * 
 * 490   5/18/98 1:58a Mike
 * Make Phoenix not be fired at fighters (but yes bombers).
 * Improve positioning of ships in guard mode.
 * Make turrets on player ship not fire near end of support ship docking.
 * 
 * 489   5/15/98 9:52a Allender
 * added code to give Warning when orders accepted don't match the default
 * orders
 * 
 * 488   5/13/98 4:41p Mike
 * Make big ships try a tiny bit to avoid collision with each other when
 * attacking another big ship.  Make ships a little less likely to collide
 * into player when in formation, drop off if player flying wacky.
 * 
 * 487   5/13/98 3:07p Mitri
 * fixed problem with checking current count of the mission against max
 * ships per wing
 * 
 * 486   5/11/98 4:33p Allender
 * fixed ingame join problems -- started to work on new object updating
 * code (currently ifdef'ed out)
 * 
 * 485   5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>


#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "ai/aigoals.h"
#include "playerman/player.h"
#include "starfield/starfield.h"
#include "bmpman/bmpman.h"
#include "lighting/lighting.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "ship/shipfx.h"
#include "debris/debris.h"
#include "starfield/nebula.h"
#include "hud/hudets.h"
#include "mission/missionhotkey.h"
#include "hud/hudescort.h"
#include "asteroid/asteroid.h"
#include "ship/shiphit.h"
#include "math/staticrand.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/redalert.h"
#include "hud/hudwingmanstatus.h"
#include "jumpnode/jumpnode.h"
#include "localization/localize.h"
#include "nebula/neb.h"
#include "demo/demo.h"
#include "nebula/neblightning.h"
#include "math/fvi.h"
#include "weapon/weapon.h"
#include "cfile/cfile.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_respawn.h"
#include "network/multi_endgame.h"
#include "object/parseobjectdock.h"
#include "object/waypoint/waypoint.h"

LOCAL struct {
	char docker[NAME_LENGTH];
	char dockee[NAME_LENGTH];
	char docker_point[NAME_LENGTH];
	char dockee_point[NAME_LENGTH];
} Initially_docked[MAX_SHIPS];

int Total_initially_docked;

mission	The_mission;
char Mission_filename[80];

int Mission_palette;  // index into Nebula_palette_filenames[] of palette file to use for mission
int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
int Num_ai_behaviors = MAX_AI_BEHAVIORS;
int Num_cargo = 0;
int Num_status_names = MAX_STATUS_NAMES;
int Num_arrival_names = MAX_ARRIVAL_NAMES;
int Num_goal_type_names = MAX_GOAL_TYPE_NAMES;
int Num_team_names = MAX_TEAM_NAMES;
int Num_parse_goals;
int Player_starts = 1;
int Num_teams;
fix Entry_delay_time = 0;
int Fred_num_texture_replacements = 0;

ushort Current_file_checksum = 0;
ushort Last_file_checksum = 0;
int    Current_file_length   = 0;

// alternate ship type names
char Mission_alt_types[MAX_ALT_TYPE_NAMES][NAME_LENGTH];
int Mission_alt_type_count = 0;

#define SHIP_WARP_TIME 5.0f		// how many seconds it takes for ship to warp in

// the ship arrival list will contain a list of ships that are yet to arrive.  This
// list could also include ships that are part of wings!

p_object Parse_objects[MAX_PARSE_OBJECTS], Ship_arrival_list;	// for linked list of ships to arrive later
int	Num_parse_objects;

// list for arriving support ship
p_object	Support_ship_pobj;
p_object *Arriving_support_ship;
char Arriving_repair_targets[MAX_AI_GOALS][NAME_LENGTH];
int Num_arriving_repair_targets;

//subsys_status Subsys_status[MAX_SUBSYS_STATUS]; // it's dynamic now - taylor
#define MIN_SUBSYS_STATUS_SIZE		25
subsys_status *Subsys_status = NULL;
int Subsys_index;
int Subsys_status_size;

char Mission_parse_storm_name[NAME_LENGTH] = "none";

team_data Team_data[MAX_TVT_TEAMS];

// variables for player start in single player
char		Player_start_shipname[NAME_LENGTH];
int		Player_start_shipnum;
p_object Player_start_pobject;

// name of all ships to use while parsing a mission (since a ship might be referenced by
// something before that ship has even been loaded yet)
char Parse_names[MAX_SHIPS + MAX_WINGS][NAME_LENGTH];
int Num_parse_names;

texture_replace Fred_texture_replacements[MAX_SHIPS * MAX_MODEL_TEXTURES];

int Num_path_restrictions;
path_restriction_t Path_restrictions[MAX_PATH_RESTRICTIONS];

//XSTR:OFF

char *Nebula_filenames[NUM_NEBULAS] = {
	"Nebula01",
	"Nebula02",
	"Nebula03"	
};

char *Neb2_filenames[NUM_NEBULAS] = {
	"Nebfull01",
	"Nebfull02",
	"Nebfull03"
};

// Note: Nebula_colors[] and Nebula_palette_filenames are linked via index numbers
char *Nebula_colors[NUM_NEBULA_COLORS] = {
	"Red",
	"Blue",
	"Gold",
	"Purple",
	"Maroon",
	"Green",
	"Grey blue",
	"Violet",
	"Grey Green",
};

char *Ai_behavior_names[MAX_AI_BEHAVIORS] = {
	"Chase",
	"Evade",
	"Get behind",
	"Stay Near",
	"Still",
	"Guard",
	"Avoid",
	"Waypoints",
	"Dock",
	"None",
	"Big Ship",
	"Path",
	"Be Rearmed",
	"Safety",
	"Evade Weapon",
	"Strafe",
	"Play Dead",
	"Bay Emerge",
	"Bay Depart",
	"Sentry Gun",
	"Warp Out",
};

char *Cargo_names[MAX_CARGO];
char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

char *Ship_class_names[MAX_SHIP_CLASSES];		// to be filled in from Ship_info array

char *Icon_names[MAX_BRIEF_ICONS] = {
	"Fighter", "Fighter Wing", "Cargo", "Cargo Wing", "Largeship",
	"Largeship Wing", "Capital", "Planet", "Asteroid Field", "Waypoint",
	"Support Ship", "Freighter(no cargo)", "Freighter(has cargo)",
	"Freighter Wing(no cargo)", "Freighter Wing(has cargo)", "Installation",
	"Bomber", "Bomber Wing", "Cruiser", "Cruiser Wing", "Unknown", "Unknown Wing",
	"Player Fighter", "Player Fighter Wing", "Player Bomber", "Player Bomber Wing",
	"Knossos Device", "Transport Wing", "Corvette", "Gas Miner", "Awacs", "Supercap", "Sentry Gun", "Jump Node", "Transport"
};

char *Status_desc_names[MAX_STATUS_NAMES] = {
	"Shields Critical", "Engines Damaged", "Fully Operational",
};

char *Status_type_names[MAX_STATUS_NAMES] = {
	"Damaged", "Disabled", "Corroded",
};

char *Status_target_names[MAX_STATUS_NAMES] = {
	"Weapons", "Engines", "Cable TV",
};

// definitions for arrival locations for ships/wings
char *Arrival_location_names[MAX_ARRIVAL_NAMES] = {
	"Hyperspace", "Near Ship", "In front of ship", "Docking Bay",
};

char *Departure_location_names[MAX_DEPARTURE_NAMES] = {
	"Hyperspace", "Docking Bay",
};

char *Goal_type_names[MAX_GOAL_TYPE_NAMES] = {
	"Primary", "Secondary", "Bonus",
};

char *Reinforcement_type_names[] = {
	"Attack/Protect",
	"Repair/Rearm",
};

char *Old_game_types[OLD_MAX_GAME_TYPES] = {
	"Single Player Only",	
	"Multiplayer Only",
	"Single/Multi Player",
	"Training mission"
};

char *Parse_object_flags[MAX_PARSE_OBJECT_FLAGS] = {
	"cargo-known",
	"ignore-count",
	"protect-ship",
	"reinforcement",
	"no-shields",
	"escort",
	"player-start",
	"no-arrival-music",
	"no-arrival-warp",
	"no-departure-warp",
	"locked",
	"invulnerable",
	"hidden-from-sensors",
	"scannable",
	"kamikaze",
	"no-dynamic",
	"red-alert-carry",
	"beam-protect-ship",
	"guardian",
	"special-warp",
	"vaporize",
	"stealth",
	"friendly-stealth-invisible"
	"don't-collide-invisible",
};

char *Parse_object_flags_2[MAX_PARSE_OBJECT_FLAGS_2] = {
	"primitive-sensors",
	"no-subspace-drive",
	"nav-carry-status",
	"affected-by-gravity",
	"toggle-subsystem-scanning",
	"targetable-as-bomb",
	"no-builtin-messages",
	"primaries-locked", 
	"secondaries-locked",
	"set-class-dynamically",
	"team-loadout-store",
	"no-death-scream",
	"always-death-scream",
};


//XSTR:ON

int Num_reinforcement_type_names = sizeof(Reinforcement_type_names) / sizeof(char *);

vec3d Parse_viewer_pos;
matrix Parse_viewer_orient;

int Loading_screen_bm_index=-1;

// definitions for timestamps for eval'ing arrival/departure cues
int Mission_arrival_timestamp;
int Mission_departure_timestamp;
fix Mission_end_time;

#define ARRIVAL_TIMESTAMP		2000		// every 2 seconds
#define DEPARTURE_TIMESTAMP	2200		// every 2.2 seconds -- just to be a little different

// calculates a "unique" file signature as a ushort (checksum) and an int (file length)
// the amount of The_mission we're going to checksum
// WARNING : do NOT call this function on the server - it will overwrite goals, etc
#define MISSION_CHECKSUM_SIZE (NAME_LENGTH + NAME_LENGTH + 4 + DATE_TIME_LENGTH + DATE_TIME_LENGTH)

// timers used to limit arrival messages and music
#define ARRIVAL_MUSIC_MIN_SEPARATION	60000
#define ARRIVAL_MESSAGE_MIN_SEPARATION 30000

#define ARRIVAL_MESSAGE_DELAY_MIN		2000
#define ARRIVAL_MESSAGE_DELAY_MAX		3000

static int Allow_arrival_music_timestamp;
static int Allow_arrival_message_timestamp;
static int Arrival_message_delay_timestamp;

// multi TvT
static int Allow_arrival_music_timestamp_m[2];
static int Allow_arrival_message_timestamp_m[2];
static int Arrival_message_delay_timestamp_m[2];

extern fix game_get_overall_frametime();	// for texture animation

// local prototypes
void parse_player_info2(mission *pm);
void post_process_mission(mission *pm);
int allocate_subsys_status();
void parse_common_object_data(p_object	*objp);
void parse_asteroid_fields(mission *pm);
int mission_set_arrival_location(int anchor, int location, int distance, int objnum, int path_mask, vec3d *new_pos, matrix *new_orient);
int get_parse_name_index(char *name);
int get_anchor(char *name);
void mission_parse_set_up_initial_docks();
void mission_parse_set_arrival_locations();
void mission_set_wing_arrival_location( wing *wingp, int num_to_set );
int parse_lookup_alt_name(char *name);
void parse_init(bool basic = false);
void parse_object_set_handled_flag_helper(p_object *pobjp, p_dock_function_info *infop);
void parse_object_clear_all_handled_flags();
int parse_object_on_arrival_list(p_object *pobjp);
int add_path_restriction();

// Karajorma 
void process_loadout_objects();
bool attempt_assignation_from_alt_classes(p_object *pobj);
void assign_ship_from_loadout(p_object *pobj, team_data *current_team, int new_ship_loadout_index);

// Goober5000
void mission_parse_mark_non_arrival(p_object *p_objp);
void mission_parse_mark_non_arrival(wing *wingp);
void mission_parse_mark_non_arrivals();

// Goober5000 - FRED import
void convertFSMtoFS2();
void restore_default_weapons(char *ships_tbl);
void restore_one_primary_bank(int *ship_primary_weapons, int *default_primary_weapons);
void restore_one_secondary_bank(int *ship_secondary_weapons, int *default_secondary_weapons);


MONITOR(NumShipArrivals)
MONITOR(NumShipDepartures)


void parse_mission_info(mission *pm, bool basic = false)
{
	int i;
	char game_string[NAME_LENGTH];

	// Goober5000
	skip_to_start_of_string("#Mission Info");

	required_string("#Mission Info");
	
	required_string("$Version:");
	stuff_float(&pm->version);
	if (pm->version != MISSION_VERSION)
		mprintf(("Older mission, should update it (%.2f<-->%.2f)", pm->version, MISSION_VERSION));

	required_string("$Name:");
	stuff_string(pm->name, F_NAME, NAME_LENGTH);

	required_string("$Author:");
	stuff_string(pm->author, F_NAME, NAME_LENGTH);

	required_string("$Created:");
	stuff_string(pm->created, F_DATE, DATE_TIME_LENGTH);

	required_string("$Modified:");
	stuff_string(pm->modified, F_DATE, DATE_TIME_LENGTH);

	required_string("$Notes:");
	stuff_string(pm->notes, F_NOTES, NOTES_LENGTH);

	if (optional_string("$Mission Desc:"))
		stuff_string(pm->mission_desc, F_MULTITEXT, MISSION_DESC_LENGTH);
	else
		strcpy(pm->mission_desc, NOX("No description\n"));

	pm->game_type = MISSION_TYPE_SINGLE;				// default to single player only
	if ( optional_string("+Game Type:")) {
		// HACK HACK HACK -- stuff_string was changed to *not* ignore carriage returns.  Since the
		// old style missions may have carriage returns, deal with it here.
		ignore_white_space();
		stuff_string(game_string, F_NAME, NAME_LENGTH);
		for ( i = 0; i < OLD_MAX_GAME_TYPES; i++ ) {
			if ( !stricmp(game_string, Old_game_types[i]) ) {

				// this block of code is now old mission compatibility code.  We specify game
				// type in a different manner than before.
				if ( i == OLD_GAME_TYPE_SINGLE_ONLY )
					pm->game_type = MISSION_TYPE_SINGLE;
				else if ( i == OLD_GAME_TYPE_MULTI_ONLY )
					pm->game_type = MISSION_TYPE_MULTI;
				else if ( i == OLD_GAME_TYPE_SINGLE_MULTI )
					pm->game_type = (MISSION_TYPE_SINGLE | MISSION_TYPE_MULTI );
				else if ( i == OLD_GAME_TYPE_TRAINING )
					pm->game_type = MISSION_TYPE_TRAINING;
				else
					Int3();

				if ( pm->game_type & MISSION_TYPE_MULTI )
					pm->game_type |= MISSION_TYPE_MULTI_COOP;

				break;
			}
		}
	}

	if ( optional_string("+Game Type Flags:") ) {
		stuff_int(&pm->game_type);
	}

	pm->flags = 0;
	if (optional_string("+Flags:")){
		stuff_int(&pm->flags);
	}

	// nebula mission stuff
	Neb2_awacs = -1.0f;
	if(optional_string("+NebAwacs:")){
		stuff_float(&Neb2_awacs);
	}
	if(optional_string("+Storm:")){
		stuff_string(Mission_parse_storm_name, F_NAME, NAME_LENGTH);

		if (!basic)
			nebl_set_storm(Mission_parse_storm_name);
	}

	// Goober5000 - ship contrail speed threshold
	pm->contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	if (optional_string("$Contrail Speed Threshold:")){
		stuff_int(&pm->contrail_threshold);
	}

	// get the number of players if in a multiplayer mission
	pm->num_players = 1;
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Players:") ) {
			stuff_int( &(pm->num_players) );
		}
	}

	// get the number of respawns
	pm->num_respawns = 0;
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Respawns:") ){
			stuff_int( (int*)&(pm->num_respawns) );
		}
	}

	if ( optional_string("+Red Alert:")) {
		int temp;
		stuff_int(&temp);

		if (temp)
			pm->flags |= MISSION_FLAG_RED_ALERT;
		else
			pm->flags &= ~MISSION_FLAG_RED_ALERT;
	} 
	red_alert_invalidate_timestamp();

	if ( optional_string("+Scramble:")) {
		int temp;
		stuff_int(&temp);

		if (temp)
			pm->flags |= MISSION_FLAG_SCRAMBLE;
		else
			pm->flags &= ~MISSION_FLAG_SCRAMBLE;
	}

	// if we are just requesting basic info then skip everything else.  the reason
	// for this is to make sure that we don't modify things outside of the mission struct
	// that might not get reset afterwards (like what can happen in the techroom) - taylor
	//
	// NOTE: this can be dangerous so be sure that any get_mission_info() call (defaults to basic info) will
	//       only reference data parsed before this point!! (like current FRED2 and game code does)
	if (basic)
		return;


	// set up support ships
	pm->support_ships.arrival_location = ARRIVE_AT_LOCATION;
	pm->support_ships.arrival_anchor = -1;
	pm->support_ships.departure_location = DEPART_AT_LOCATION;
	pm->support_ships.departure_anchor = -1;
	pm->support_ships.max_hull_repair_val = 100.0f;	//ASSUMPTION: full repair capabilities
	pm->support_ships.max_subsys_repair_val = 100.0f;
	pm->support_ships.max_support_ships = -1;	// infinite
	pm->support_ships.ship_class = -1;
	pm->support_ships.tally = 0;
	pm->support_ships.support_available_for_species = 0;
	pm->support_ships.ssi_flags = 0;

	// for each species, store whether support is available
	for (int species = 0; species < (int)Species_info.size(); species++)
	{
		for (int ship_class = 0; ship_class < Num_ship_classes; ship_class++)
		{
			if ((Ship_info[ship_class].flags & SIF_SUPPORT) && (Ship_info[ship_class].species == species))
			{
				pm->support_ships.support_available_for_species |= (1 << species);
				break;
			}
		}
	}

	if ( optional_string("+Disallow Support:"))
	{
		int temp;
		stuff_int(&temp);

		pm->support_ships.max_support_ships = (temp > 0) ? 0 : -1;
	}

	if ( optional_string("+Hull Repair Ceiling:"))
	{
		float temp;
		stuff_float(&temp);

		//ONLY set max_hull_repair_val if the value given is valid -C
		if (temp <= 100.0f && temp >= 0.0f) {
			pm->support_ships.max_hull_repair_val = temp;
		}
	}

	if ( optional_string("+Subsystem Repair Ceiling:"))
	{
		float temp;
		stuff_float(&temp);

		//ONLY set max_subsys_repair_val if the value given is valid -C
		if (temp <= 100.0f && temp >= 0.0f) {
			pm->support_ships.max_subsys_repair_val = temp;
		}
	}

	if (optional_string("+All Teams Attack")) {
		pm->flags |= MISSION_FLAG_ALL_ATTACK;
	}

	//	Maybe delay the player's entry.
	if (optional_string("+Player Entry Delay:")) {
		float	temp;
		
		stuff_float(&temp);
		Assert(temp >= 0.0f);
		Entry_delay_time = fl2f(temp);
	}
	else
	{
		Entry_delay_time = 0;
	}

	if (optional_string("+Viewer pos:")){
		stuff_vector(&Parse_viewer_pos);
	}

	if (optional_string("+Viewer orient:")){
		stuff_matrix(&Parse_viewer_orient);
	}

	// possible squadron reassignment
	strcpy(pm->squad_name, "");
	strcpy(pm->squad_filename, "");
	if(optional_string("+SquadReassignName:")){
		stuff_string(pm->squad_name, F_NAME, NAME_LENGTH);
		if(optional_string("+SquadReassignLogo:")){
			stuff_string(pm->squad_filename, F_NAME, MAX_FILENAME_LEN);
		}
	}	
	// always clear out squad reassignments if not single player
	if(Game_mode & GM_MULTIPLAYER){
		strcpy(pm->squad_name, "");
		strcpy(pm->squad_filename, "");
	//	mprintf(("Ignoring squadron reassignment in parse_mission_info\n"));
	}
	// reassign the player
	else {		
		if(!Fred_running && (Player != NULL) && (strlen(pm->squad_name) > 0) && (Game_mode & GM_CAMPAIGN_MODE)){
			mprintf(("Reassigning player to squadron %s\n", pm->squad_name));
			player_set_squad(Player, pm->squad_name);
			player_set_squad_bitmap(Player, pm->squad_filename);
		}
	}


	// wing stuff by Goober5000 ------------------------------------------
	// the wing name arrays are initialized in ship_level_init
	if (optional_string("$Starting wing names:"))
	{
		stuff_string_list(Starting_wing_names, MAX_STARTING_WINGS);
	}

	if (optional_string("$Squadron wing names:"))
	{
		stuff_string_list(Squadron_wing_names, MAX_SQUADRON_WINGS);
	}

	if (optional_string("$Team-versus-team wing names:"))
	{
		stuff_string_list(TVT_wing_names, MAX_TVT_WINGS);
	}

	// minimal error checking
	if (strcmp(Starting_wing_names[0], TVT_wing_names[0]))
	{
		Error(LOCATION, "The first starting wing and the first team-versus-team wing must have the same wing name.\n");
	}
	// end of wing stuff -------------------------------------------------


	// set up the Num_teams variable accoriding to the game_type variable'
	Num_teams = 1;				// assume 1

	// multiplayer team v. team games have two teams.  If we have three teams, we need to use
	// a new mission mode!
	if ( (pm->game_type & MISSION_TYPE_MULTI) && (pm->game_type & MISSION_TYPE_MULTI_TEAMS) ){
		Num_teams = 2;
	}

	int found640=0, found1024=0;
	strcpy(pm->loading_screen[GR_640],"");
	strcpy(pm->loading_screen[GR_1024],"");
	//custom mission loading background
	if (optional_string("$Load Screen 640:"))
	{
		found640=1;
		stuff_string(pm->loading_screen[GR_640], F_NAME, MAX_FILENAME_LEN);	
	}
	if (optional_string("$Load Screen 1024:"))
	{
		found1024=1;
		stuff_string(pm->loading_screen[GR_1024], F_NAME, MAX_FILENAME_LEN);
	}

	//error testing
	if ((found640) && !(found1024))
	{
		Warning(LOCATION, "Mission: %s\nhas a 640x480 loading screen but no 1024x768 loading screen!",pm->name);
	}
	if (!(found640) && (found1024))
	{
		Warning(LOCATION, "Mission: %s\nhas a 1024x768 loading screen but no 640x480 loading screen!",pm->name);
	}

	strcpy(pm->skybox_model, "");
	if (optional_string("$Skybox model:"))
	{
		stuff_string(pm->skybox_model, F_NAME, MAX_FILENAME_LEN);
	}

	// Goober5000 - AI on a per-mission basis
	The_mission.ai_profile = &Ai_profiles[Default_ai_profile];
	if (optional_string("$AI Profile:"))
	{
		int index;
		char temp[NAME_LENGTH];

		stuff_string(temp, F_NAME, NAME_LENGTH);
		index = ai_profile_lookup(temp);

		if (index >= 0)
			The_mission.ai_profile = &Ai_profiles[index];
	}

	Assert( The_mission.ai_profile != NULL );

	// Kazan - player use AI at start?
	if (pm->flags & MISSION_FLAG_PLAYER_START_AI)
		Player_use_ai = 1;
}

void parse_player_info(mission *pm)
{
	char alt[NAME_LENGTH];
	Assert(pm != NULL);

// alternate type names begin here	
	mission_parse_reset_alt();
	if(optional_string("#Alternate Types:")){		
		// read them all in
		while(!optional_string("#end")){
			required_string("$Alt:");
			stuff_string(alt, F_NAME, NAME_LENGTH);

			// maybe store it
			mission_parse_add_alt(alt);			
		}
	}
	
	Player_starts = 0;
	required_string("#Players");

	while (required_string_either("#Objects", "$")){
		parse_player_info2(pm);
	}
}

void parse_player_info2(mission *pm)
{
	char str[NAME_LENGTH];
	int nt, i, total, list[MAX_SHIP_CLASSES * 4], list2[MAX_WEAPON_TYPES * 2]; //, num_starting_wings;
	team_data *ptr;
	// char starting_wings[MAX_STARTING_WINGS][NAME_LENGTH];

	// read in a ship/weapon pool for each team.
	for ( nt = 0; nt < Num_teams; nt++ ) {
		int num_ship_choices;

		ptr = &Team_data[nt];
		// get the shipname for single player missions
		// MWA -- make this required later!!!!
		if ( optional_string("$Starting Shipname:") )
			stuff_string( Player_start_shipname, F_NAME, NAME_LENGTH );

		required_string("$Ship Choices:");
		total = stuff_ship_list(list, MAX_SHIP_CLASSES * 4, MISSION_LOADOUT_SHIP_LIST);

		// make sure we have a count which is divisible by four since four values are added for each ship
		Assert((total%4) == 0); 


		num_ship_choices = 0;
		total /= 4;							// there are only 1/4 the ships really on the list.
		for (i=0; i<total; i++) {
			// in a campaign, see if the player is allowed the ships or not.  Remove them from the
			// pool if they are not allowed
			if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
				if ( !Campaign.ships_allowed[list[i*4]] )
					continue;
			}

			ptr->ship_list[num_ship_choices] = list[i * 4];
			strcpy(ptr->ship_list_variables[num_ship_choices], Sexp_variables[list[(i * 4) + 1]].variable_name);
			ptr->ship_count[num_ship_choices] = list[(i * 4) + 2];
			ptr->loadout_total += list[(i * 4) + 2];
			strcpy(ptr->ship_count_variables[num_ship_choices], Sexp_variables[list[(i * 4)+3]].variable_name);
			num_ship_choices++;
		}
		ptr->number_choices = num_ship_choices;

		/* Karajorma - Well this is pretty funny. The code sets the num_starting_wings variable and then never uses it 
		// again! Furthermore there isn't actually any way to set "+Starting Wings:" in the first place. Like the 
		// "+Default_ship:" code below this appears to be either a feature that was never added or a leftover from 
		// FS1 as it was never used in the [V] missions. 

		num_starting_wings = 0;
		if (optional_string("+Starting Wings:"))
			num_starting_wings = stuff_string_list(starting_wings, MAX_STARTING_WINGS);
		*/

		// Karajorma - Unlike the Starting Wings: code this actually does do something if you add the entry to a mission
		// Might be worth adding a box to FRED so people can set this. The Team Loadout code could definitely use it.

		ptr->default_ship = -1;
		if (optional_string("+Default_ship:")) {
			stuff_string(str, F_NAME, NAME_LENGTH);
			ptr->default_ship = ship_info_lookup(str);
			// see if the player's default ship is an allowable ship (campaign only). If not, then what
			// do we do?  choose the first allowable one?
			if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
				if ( !(Campaign.ships_allowed[ptr->default_ship]) ) {
					for (i = 0; i < MAX_SHIP_CLASSES; i++ ) {
						if ( Campaign.ships_allowed[ptr->default_ship] ) {
							ptr->default_ship = i;
							break;
						}
					}
					Assert( i < MAX_SHIP_CLASSES );
				}
			}
		}

		if (ptr->default_ship == -1)  // invalid or not specified, make first in list*/
			ptr->default_ship = ptr->ship_list[0];

		for (i=0; i<MAX_WEAPON_TYPES; i++)
			ptr->weaponry_pool[i] = 0;

		if (optional_string("+Weaponry Pool:")) {
			total = stuff_int_list(list2, MAX_WEAPON_TYPES * 2, WEAPON_POOL_TYPE);

			Assert( !(total & 0x01) );  // make sure we have an even count

			for (i = 0; i < total; i += 2) {
				// in a campaign, see if the player is allowed the weapons or not.  Remove them from the
				// pool if they are not allowed
				if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
					if ( !Campaign.weapons_allowed[list2[i]] )
						continue;
				}

				if ( (list2[i] >= 0) && (list2[i] < MAX_WEAPON_TYPES) ) {
					// always allow the pool to be added in FRED, it is a verbal warning
					// to let the mission dev know about the problem
					if ( (Weapon_info[list2[i]].wi_flags & WIF_PLAYER_ALLOWED) || Fred_running )
						ptr->weaponry_pool[list2[i]] = list2[i+1];
					else
						mprintf(("WARNING:  Weapon '%s' in weapon pool isn't allowed on player loadout! Ignoring it ...\n", Weapon_info[i].name));
				}
			}
		}
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough ship/weapon pools for mission.  There are %d teams and only %d pools.", Num_teams, nt);
}

void parse_plot_info(mission *pm)
{
	if (optional_string("#Plot Info"))
	{
		char dummy_filespec[FILESPEC_LENGTH];
		char dummy_name[NAME_LENGTH];

		required_string("$Tour:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Pre-Briefing Cutscene:");
		stuff_string(dummy_filespec, F_FILESPEC, FILESPEC_LENGTH);

		required_string("$Pre-Mission Cutscene:");
		stuff_string(dummy_filespec, F_FILESPEC, FILESPEC_LENGTH);

		required_string("$Next Mission Success:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Next Mission Partial:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Next Mission Failure:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);
	}
}

void parse_briefing_info(mission *pm)
{
	char junk[4096];

	if ( !optional_string("#Briefing Info") )
		return;

	required_string("$Briefing Voice 1:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Briefing Voice 2:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Briefing Voice 3:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 1:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 2:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 3:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));
}

// parse the event music and briefing music for the mission
void parse_music(mission *pm, int flags)
{
	int i, index, num;
	char *ch;
	char temp[NAME_LENGTH];

	event_music_reset_choices();

	if ( !optional_string("#Music") ) {
		return;
	}

	required_string("$Event Music:");
	stuff_string(pm->event_music_name, F_NAME, NAME_LENGTH);

	// Goober5000
	if (optional_string("$Substitute Event Music:"))
		stuff_string(pm->substitute_event_music_name, F_NAME, NAME_LENGTH);

	required_string("$Briefing Music:");
	stuff_string(pm->briefing_music_name, F_NAME, NAME_LENGTH);

	// Goober5000
	if (optional_string("$Substitute Briefing Music:"))
		stuff_string(pm->substitute_briefing_music_name, F_NAME, NAME_LENGTH);

	// old stuff, apparently
	if (optional_string("$Debriefing Success Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		event_music_set_score(SCORE_DEBRIEF_SUCCESS, temp);
	}

	// ditto
	if (optional_string("$Debriefing Fail Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		event_music_set_score(SCORE_DEBRIEF_FAIL, temp);
	}


	// Goober5000 - if briefing not specified in import, default to BRIEF1
	if (!stricmp(pm->briefing_music_name, "none") && (flags & MPF_IMPORT_FSM))
		strcpy(pm->briefing_music_name, "BRIEF1");


	// Goober5000 - old way of grabbing substitute music, but here for reverse compatibility
	if (optional_string("$Substitute Music:"))
	{
		stuff_string(pm->substitute_event_music_name, F_NAME, NAME_LENGTH, "," );
		Mp++;
		stuff_string(pm->substitute_briefing_music_name, F_NAME, NAME_LENGTH);
	}

	// Goober5000 - if the mission is being imported, the substitutes are the specified tracks
	// (with FS1 prefixes) and we generate new specified tracks
	if (flags & MPF_IMPORT_FSM)
	{
		// no specified music?
		if (!stricmp(pm->event_music_name, "none"))
			goto done_event_music;

		// set the FS1 equivalent as the substitute
		strcpy(pm->substitute_event_music_name, "FS1-");
		strcat(pm->substitute_event_music_name, pm->event_music_name);

		// if we have Marauder, it's in FS2 as Deuteronomy, so we're done
		if (!stricmp(pm->event_music_name, "7: Marauder") && event_music_get_soundtrack_index("5: Deuteronomy") >= 0)
		{
			strcpy(pm->event_music_name, "5: Deuteronomy");
			goto done_event_music;
		}

		// search for something with the same track number
		strcpy(temp, pm->event_music_name);
		ch = strchr(temp, ':');
		if (ch != NULL)
		{
			*(ch + 1) = '\0';
				
			for (i = 0; i < Num_soundtracks; i++)
			{
				if (!strncmp(temp, Soundtracks[i].name, strlen(temp)))
				{
					strcpy(pm->event_music_name, Soundtracks[i].name);
					goto done_event_music;
				}
			}
		}

		// last resort: pick a random track out of the 7 FS2 soundtracks
		num = (Num_soundtracks < 7) ? Num_soundtracks : 7;
		strcpy(pm->event_music_name, Soundtracks[rand() % num].name);


done_event_music:


		// no specified music?
		if (!stricmp(pm->briefing_music_name, "none"))
			goto done_briefing_music;

		// set the FS1 equivalent as the substitute
		strcpy(pm->substitute_briefing_music_name, "FS1-");
		strcat(pm->substitute_briefing_music_name, pm->briefing_music_name);

		// Choco Mousse is the FS1 title soundtrack, so use Aquitaine in FS2
		if (!stricmp(pm->briefing_music_name, "Choco Mousse") && event_music_get_spooled_music_index("Aquitaine") >= 0)
		{
			strcpy(pm->briefing_music_name, "Aquitaine");
			goto done_briefing_music;
		}

		// we might have a match with the same track number
		if (event_music_get_spooled_music_index(pm->briefing_music_name) >= 0)
			goto done_briefing_music;

		// last resort: pick a random track out of the first 7 FS2 briefings (the regular ones)...
		num = (Num_music_files < 7) ? Num_music_files : 7;
		strcpy(pm->briefing_music_name, Spooled_music[rand() % num].name);


done_briefing_music:

		/* NO-OP */ ;
	}


	// set the soundtrack, preferring the substitute in FS2 (not FRED!)
	index = event_music_get_soundtrack_index(pm->substitute_event_music_name);
	if ((index >= 0) && (Soundtracks[index].flags & EMF_VALID) && !Fred_running)
	{
		event_music_set_soundtrack(pm->substitute_event_music_name);
	}
	else
	{
		event_music_set_soundtrack(pm->event_music_name);
	}

	// set the briefing, preferring the substitute in FS2 (not FRED!)
	index = event_music_get_spooled_music_index(pm->substitute_briefing_music_name);
	if ((index >= 0) && (Spooled_music[index].flags & SMF_VALID) && !Fred_running)
	{
		event_music_set_score(SCORE_BRIEFING, pm->substitute_briefing_music_name);
	}
	else
	{
		event_music_set_score(SCORE_BRIEFING, pm->briefing_music_name);
	}
}

void parse_cmd_brief(mission *pm)
{
	int stage;

	Assert(!Cur_cmd_brief->num_stages);
	stage = 0;

	required_string("#Command Briefing");
	while (optional_string("$Stage Text:")) {
		Assert(stage < CMD_BRIEF_STAGES_MAX);
		Cur_cmd_brief->stage[stage].text = stuff_and_malloc_string(F_MULTITEXT, NULL, CMD_BRIEF_TEXT_MAX);
		Assert(Cur_cmd_brief->stage[stage].text);

		required_string("$Ani Filename:");
		stuff_string(Cur_cmd_brief->stage[stage].ani_filename, F_FILESPEC, MAX_FILENAME_LEN);
		if (optional_string("+Wave Filename:"))
			stuff_string(Cur_cmd_brief->stage[stage].wave_filename, F_FILESPEC, MAX_FILENAME_LEN);
		else
			Cur_cmd_brief->stage[stage].wave_filename[0] = 0;

		stage++;
	}

	Cur_cmd_brief->num_stages = stage;
}

void parse_cmd_briefs(mission *pm)
{
	int i;

	cmd_brief_reset();
	// a hack follows because old missions don't have a command briefing
	if (required_string_either("#Command Briefing", "#Briefing"))
		return;

	for (i=0; i<Num_teams; i++) {
		Cur_cmd_brief = &Cmd_briefs[i];
		parse_cmd_brief(pm);
	}
}

// -------------------------------------------------------------------------------------------------
// parse_briefing()
//
// Parse the data required for the mission briefing
//
// NOTE: This updates the global Briefing struct with all the data necessary to drive the briefing
//
void parse_briefing(mission *pm, int flags)
{
	int nt, i, j, stage_num = 0, icon_num = 0;
	brief_stage *bs;
	brief_icon *bi;
	briefing *bp;

	char not_used_text[MAX_ICON_TEXT_LEN];
	
	brief_reset();

	// MWA -- 2/3/98.  we can now have multiple briefing and debriefings in a mission
	for ( nt = 0; nt < Num_teams; nt++ ) {
		if ( !optional_string("#Briefing") )
			break;

		bp = &Briefings[nt];

		required_string("$start_briefing");
		required_string("$num_stages:");
		stuff_int(&bp->num_stages);
		Assert(bp->num_stages <= MAX_BRIEF_STAGES);

		stage_num = 0;
		while (required_string_either("$end_briefing", "$start_stage")) {
			required_string("$start_stage");
			Assert(stage_num < MAX_BRIEF_STAGES);
			bs = &bp->stages[stage_num++];
			required_string("$multi_text");
			if ( Fred_running )	{
				stuff_string(bs->new_text, F_MULTITEXT, MAX_BRIEF_LEN);
			} else {
				bs->new_text = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_BRIEF_LEN);
			}
			required_string("$voice:");
			stuff_string(bs->voice, F_FILESPEC, MAX_FILENAME_LEN);
			required_string("$camera_pos:");
			stuff_vector(&bs->camera_pos);
			required_string("$camera_orient:");
			stuff_matrix(&bs->camera_orient);
			required_string("$camera_time:");
			stuff_int(&bs->camera_time);

			if ( optional_string("$num_lines:") ) {
				stuff_int(&bs->num_lines);

				if ( Fred_running )	{
					Assert(bs->lines!=NULL);
				} else {
					if ( bs->num_lines > 0 )	{
						bs->lines = (brief_line *)vm_malloc(sizeof(brief_line)*bs->num_lines);
						Assert(bs->lines!=NULL);
					}
				}

				for (i=0; i<bs->num_lines; i++) {
					required_string("$line_start:");
					stuff_int(&bs->lines[i].start_icon);
					required_string("$line_end:");
					stuff_int(&bs->lines[i].end_icon);
				}
			}
			else {
				bs->num_lines = 0;
			}

			required_string("$num_icons:");
			stuff_int(&bs->num_icons);

			if ( Fred_running )	{
				Assert(bs->icons!=NULL);
			} else {
				if ( bs->num_icons > 0 )	{
					bs->icons = (brief_icon *)vm_malloc(sizeof(brief_icon)*bs->num_icons);
					Assert(bs->icons!=NULL);
				}
			}

			if ( optional_string("$flags:") )
				stuff_int(&bs->flags);
			else
				bs->flags = 0;

			if ( optional_string("$formula:") )
				bs->formula = get_sexp_main();
			else
				bs->formula = Locked_sexp_true;

			Assert(bs->num_icons <= MAX_STAGE_ICONS );

			// static alias stuff - stupid, but it seems to be necessary
			static char *temp_team_names[MAX_IFFS];
			for (i = 0; i < Num_iffs; i++)
				temp_team_names[i] = Iff_info[i].iff_name;

			while (required_string_either("$end_stage", "$start_icon"))
			{
				required_string("$start_icon");
				Assert(icon_num < MAX_STAGE_ICONS);
				bi = &bs->icons[icon_num++];

				required_string("$type:");
				stuff_int(&bi->type);

				// Goober5000 - import
				if (flags & MPF_IMPORT_FSM)
				{
					// someone changed the jump node icon to a Knossos, so change it back
					if (bi->type == ICON_KNOSSOS_DEVICE)
						bi->type = ICON_JUMP_NODE;

					// change largeship to transport
					else if (bi->type == ICON_LARGESHIP)
						bi->type = ICON_TRANSPORT;

					// ditto
					else if (bi->type == ICON_LARGESHIP_WING)
						bi->type = ICON_TRANSPORT_WING;
				}

				find_and_stuff("$team:", &bi->team, F_NAME, temp_team_names, Num_iffs, "team name");

				find_and_stuff("$class:", &bi->ship_class, F_NAME, Ship_class_names, Num_ship_classes, "ship class");

				// Goober5000 - import
				if (flags & MPF_IMPORT_FSM)
				{
					// the Faustus is a largeship
					if (!strnicmp(Ship_info[bi->ship_class].name, "GTSC Faustus", 12))
					{
						if (bi->type == ICON_CRUISER)
							bi->type = ICON_LARGESHIP;

						else if (bi->type == ICON_CRUISER_WING)
							bi->type = ICON_LARGESHIP_WING;
					}
					// the Demon is a support ship :p
					else if (!strnicmp(Ship_info[bi->ship_class].name, "SD Demon", 8))
					{
						bi->type = ICON_SUPPORT_SHIP;
					}
					// the Hades is a supercap
					else if (!strnicmp(Ship_info[bi->ship_class].name, "GTD Hades", 9))
					{
						bi->type = ICON_SUPERCAP;
					}
				}

				required_string("$pos:");
				stuff_vector(&bi->pos);

				bi->label[0] = 0;
				if (optional_string("$label:"))
					stuff_string(bi->label, F_MESSAGE, MAX_LABEL_LEN);

				if (optional_string("+id:")) {
					stuff_int(&bi->id);
					if (bi->id >= Cur_brief_id)
						Cur_brief_id = bi->id + 1;

				} else {
					bi->id = -1;
					for (i=0; i<stage_num-1; i++)
						for (j=0; j < bp->stages[i].num_icons; j++)
						{
							if (!stricmp(bp->stages[i].icons[j].label, bi->label))
								bi->id = bp->stages[i].icons[j].id;
						}

					if (bi->id < 0)
						bi->id = Cur_brief_id++;
				}

				bi->flags=0;
				int val;
				required_string("$hlight:");	
				stuff_int(&val);
				if ( val>0 ) {
					bi->flags |= BI_HIGHLIGHT;
				}

				if (optional_string("$mirror:"))
				{
					stuff_int(&val);
					if ( val>0 ) {
						bi->flags |= BI_MIRROR_ICON;
					}	
				}

				required_string("$multi_text");
//				stuff_string(bi->text, F_MULTITEXT, MAX_ICON_TEXT_LEN);
				stuff_string(not_used_text, F_MULTITEXT, MAX_ICON_TEXT_LEN);
				required_string("$end_icon");
			} // end while
			Assert(bs->num_icons == icon_num);
			icon_num = 0;
			required_string("$end_stage");
		}	// end while

		Assert(bp->num_stages == stage_num);
		required_string("$end_briefing");
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough briefings in mission file.  There are %d teams and only %d briefings.", Num_teams, nt );
}

// -------------------------------------------------------------------------------------------------
// parse_debriefing_old()
//
// Parse the data required for the mission debriefings
void parse_debriefing_old(mission *pm)
{
	int	junk;
	char	waste[MAX_DEBRIEF_LEN];
	
	if ( !optional_string("#Debriefing") )
		return;

	required_string("$num_debriefings:");
	stuff_int(&junk);

	while (required_string_either("#Players", "$start_debriefing")) {
		required_string("$start_debriefing");
		required_string("$formula:");
		junk = get_sexp_main();
		required_string("$num_stages:");
		stuff_int(&junk);
		while (required_string_either("$end_debriefing", "$start_stage")) {
			required_string("$start_stage");
			required_string("$multi_text");
			stuff_string(waste, F_MULTITEXT, MAX_DEBRIEF_LEN);
			required_string("$voice:");
			stuff_string(waste, F_FILESPEC, MAX_DEBRIEF_LEN);
			required_string("$end_stage");
		} // end while
		required_string("$end_debriefing");
	}	// end while
}

// -------------------------------------------------------------------------------------------------
// parse_debriefing_new()
//
// Parse the data required for the mission debriefings
void parse_debriefing_new(mission *pm)
{
	int				stage_num, nt;
	debriefing		*db;
	debrief_stage	*dbs;
	
	debrief_reset();

	// next code should be old -- hopefully not called anymore
	//if (!optional_string("#Debriefing_info")) {
	//	parse_debriefing_old(pm);
	//	return;
	//}

	// 2/3/98 -- MWA.  We can now have multiple briefings and debriefings on a team
	for ( nt = 0; nt < Num_teams; nt++ ) {

		if ( !optional_string("#Debriefing_info") )
			break;

		stage_num = 0;

		db = &Debriefings[nt];

		required_string("$Num stages:");
		stuff_int(&db->num_stages);
		Assert(db->num_stages <= MAX_DEBRIEF_STAGES);

		while (required_string_either("#", "$Formula")) {
			Assert(stage_num < MAX_DEBRIEF_STAGES);
			dbs = &db->stages[stage_num++];
			required_string("$Formula:");
			dbs->formula = get_sexp_main();
			required_string("$multi text");
			if ( Fred_running )	{
				stuff_string(dbs->new_text, F_MULTITEXT, MAX_DEBRIEF_LEN);
			} else {
				dbs->new_text = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_DEBRIEF_LEN);
			}
			required_string("$Voice:");
			stuff_string(dbs->voice, F_FILESPEC, MAX_FILENAME_LEN);
			required_string("$Recommendation text:");
			if ( Fred_running )	{
				stuff_string( dbs->new_recommendation_text, F_MULTITEXT, MAX_RECOMMENDATION_LEN);
			} else {
				dbs->new_recommendation_text = stuff_and_malloc_string( F_MULTITEXT, NULL, MAX_RECOMMENDATION_LEN);
			}
		} // end while

		Assert(db->num_stages == stage_num);
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough debriefings for mission.  There are %d teams and only %d debriefings;\n", Num_teams, nt );
}

void position_ship_for_knossos_warpin(p_object *p_objp)
{
	object *objp = p_objp->created_object;
	ship *shipp = &Ships[objp->instance];
	object *knossos_objp;

	// Assume no valid knossos device
	shipp->special_warp_objnum = -1;

	// find knossos device
	int found = FALSE;
	for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		knossos_objp = &Objects[so->objnum];

		if (Ship_info[Ships[knossos_objp->instance].ship_info_index].flags & SIF_KNOSSOS_DEVICE)
		{
			// be close to the right device (allow multiple knossoses)
			if ( vm_vec_dist_quick(&knossos_objp->pos, &p_objp->pos) < 2.0f*(knossos_objp->radius + objp->radius) )
			{
				found = TRUE;
				break;
			}
		}
	}

	if (!found)
		return;

	// set ship special_warp_objnum
	shipp->special_warp_objnum = OBJ_INDEX(knossos_objp);

	// position self for warp on plane of device
	vec3d new_point;
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);

	float dist = fvi_ray_plane(&new_point, &knossos_objp->pos, &knossos_objp->orient.vec.fvec, &p_objp->pos, &p_objp->orient.vec.fvec, 0.0f);
	float desired_dist = -pm->mins.xyz.z;
	vm_vec_scale_add2(&objp->pos, &objp->orient.vec.fvec, (dist - desired_dist));
	
	// if ship is BIG or HUGE, make it go through the center of the knossos
	if (Ship_info[shipp->ship_info_index].flags & SIF_HUGE_SHIP)
	{
		vec3d offset;
		vm_vec_sub(&offset, &knossos_objp->pos, &new_point);
		vm_vec_add2(&knossos_objp->pos, &offset);
	}
}

// Goober5000
// This is conceptually almost the same as obj_move_one_docked_object and is used in the same way.
void parse_dock_one_docked_object(p_object *pobjp, p_object *parent_pobjp)
{
	int dockpoint, parent_dockpoint;
	char *dockpoint_name, *parent_dockpoint_name;
	object *objp, *parent_objp;

	// get the actual in-game objects that will be docked
	objp = pobjp->created_object;
	parent_objp = parent_pobjp->created_object;

	// check valid
	if (!objp || !parent_objp)
	{
		Int3();
		return;
	}

	// get dockpoint names
	dockpoint_name = dock_find_dockpoint_used_by_object(pobjp, parent_pobjp);
	parent_dockpoint_name = dock_find_dockpoint_used_by_object(parent_pobjp, pobjp);

	// check valid
	if (!dockpoint_name || !parent_dockpoint_name)
	{
		Int3();
		return;
	}

	// resolve names to dockpoints
	dockpoint = model_find_dock_name_index(Ship_info[Ships[objp->instance].ship_info_index].model_num, dockpoint_name);
	parent_dockpoint = model_find_dock_name_index(Ship_info[Ships[parent_objp->instance].ship_info_index].model_num, parent_dockpoint_name);

	// check valid
	if ((dockpoint < 0) || (parent_dockpoint < 0))
	{
		Int3();
		return;
	}

	// dock them
	nprintf(("AI", "Initially docked: %s to parent %s\n", Ships[objp->instance].ship_name, Ships[parent_objp->instance].ship_name));
	ai_dock_with_object(objp, dockpoint, parent_objp, parent_dockpoint, 89, AIDO_DOCK_NOW);
}

int parse_create_object_sub(p_object *objp);

// Goober5000
void parse_create_docked_object_helper(p_object *pobjp, p_dock_function_info *infop)
{
	// create the object
	parse_create_object_sub(pobjp);

	// remove the object from the arrival list
	// (don't remove the leader, because he'll be removed the usual way)
	if (pobjp != infop->parameter_variables.objp_value)
	{
		// If an object is present at the beginning of the mission, it may not have been put on the arrival
		// list.  This is totally dependent on the order the objects are parsed in the mission file because
		// that is the order in which they are created.  All the objects in a docked group are created
		// simultaneously when and only when the leader is created.

		// if it's on the list, remove it
		if (parse_object_on_arrival_list(pobjp))
			list_remove(&Ship_arrival_list, pobjp);
	}
}

// Goober5000
// This is a bit tricky because of the way initial docking is now handled.
// Docking groups require special treatment.
int parse_create_object(p_object *pobjp)
{
	object *objp;

	// if this guy is part of a dock group, create the entire group, starting with the leader
	if (object_is_docked(pobjp))
	{
		p_dock_function_info dfi;

		// we should only be calling this for dock leaders, because the dock leader
		// governs the creation of his entire group
		Assert((pobjp->flags & P_SF_DOCK_LEADER));

		// if the leader will be destroyed before the mission starts, then *only* create the leader;
		// don't create the rest of the group (this is what retail did)
		if (pobjp->destroy_before_mission_time >= 0)
			return parse_create_object_sub(pobjp);

		// store the leader as a parameter
		dfi.parameter_variables.objp_value = pobjp;

		// Just as we couldn't create the parse dock trees until we had parsed them all,
		// we can't dock the in-game objects until we have created them all. :p

		// create all the objects
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_create_docked_object_helper);

		// dock all the objects
		dock_dock_docked_objects(pobjp);

		// now clear the handled flags
		parse_object_clear_all_handled_flags();
	}
	// create normally
	else
	{
		parse_create_object_sub(pobjp);
	}

	// get the main object
	objp = pobjp->created_object;

	// warp it in (moved from parse_create_object_sub)
	if ((Game_mode & GM_IN_MISSION) && (!Fred_running) && (!Game_restoring))
	{
		if ((Ships[objp->instance].wingnum < 0) && (pobjp->arrival_location != ARRIVE_FROM_DOCK_BAY))
		{
			shipfx_warpin_start(objp);
		}
	}

	// return the main object's objnum
	return OBJ_INDEX(objp);
}

//	Given a stuffed p_object struct, create an object and fill in the necessary fields.
//	Return object number.
int parse_create_object_sub(p_object *p_objp)
{
	int	i, j, k, objnum, shipnum;
	ai_info *aip;
	ship_subsys *ptr;
	ship *shipp;
	ship_info *sip;
	subsys_status *sssp;
	ship_weapon *wp;

	// texture replacements
	char texture_file[MAX_FILENAME_LEN];
	char *p;
	polymodel *pm;

	MONITOR_INC(NumShipArrivals, 1);

	// base level creation - need ship name in case of duplicate textures
	objnum = ship_create(&p_objp->orient, &p_objp->pos, p_objp->ship_class, p_objp->name);
	Assert(objnum != -1);
	shipnum = Objects[objnum].instance;

	shipp = &Ships[shipnum];
	sip = &Ship_info[shipp->ship_info_index];

	// Goober5000 - make the parse object aware of what it was created as
	p_objp->created_object = &Objects[objnum];
	
	// if arriving through knossos, adjust objpj->pos to plane of knossos and set flag
	// special warp is single player only
	if ((p_objp->flags & P_KNOSSOS_WARP_IN) && !(Game_mode & GM_MULTIPLAYER))
	{
		if (!Fred_running)
			position_ship_for_knossos_warpin(p_objp);
	}

	shipp->group = p_objp->group;
	shipp->team = p_objp->team;
	strcpy(shipp->ship_name, p_objp->name);
	shipp->escort_priority = p_objp->escort_priority;
	shipp->special_exp_index = p_objp->special_exp_index;

	// Goober5000
	shipp->special_hitpoint_index = p_objp->special_hitpoint_index;
	shipp->ship_max_shield_strength = p_objp->parse_max_shield_strength;
	shipp->ship_max_hull_strength = p_objp->parse_max_hull_strength;

	// Goober5000 - ugh, this is really stupid having to do this here; if the
	// ship creation code was better organized this wouldn't be necessary
	if (shipp->special_hitpoint_index >= 0)
	{
		float hull_factor = shipp->ship_max_hull_strength / sip->max_hull_strength;
		ship_subsys *ss;

		for (ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list) && ss != NULL; ss = GET_NEXT(ss))
		{
			ss->max_hits *= hull_factor;

			if (Fred_running)
				ss->current_hits = 0.0f;
			else
				ss->current_hits = ss->max_hits;
		}

		ship_recalc_subsys_strength(shipp);
	}

	// Goober5000 - this is also stupid; this is in ship_set but it needs to be done here because of the special hitpoints mod
	Objects[objnum].hull_strength = shipp->ship_max_hull_strength;

	shipp->respawn_priority = p_objp->respawn_priority;

	// if this is a multiplayer dogfight game, and its from a player wing, make it team traitor
	if ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (p_objp->wingnum >= 0))
	{
		for (i = 0; i < MAX_STARTING_WINGS; i++)
		{
			if (!stricmp(Starting_wing_names[i], Wings[p_objp->wingnum].name))
				shipp->team = Iff_traitor;
		}
	}

	if (!Fred_running)
	{
		ship_assign_sound(&Ships[shipnum]);
	}

	aip = &(Ai_info[shipp->ai_index]);
	aip->behavior = p_objp->behavior;
	aip->mode = aip->behavior;

	// alternate type name
	shipp->alt_type_index = p_objp->alt_type_index;

	aip->ai_class = p_objp->ai_class;
	shipp->weapons.ai_class = p_objp->ai_class;  // Fred uses this instead of above.

	// must reset the number of ai goals when the object is created
	for (i = 0; i < MAX_AI_GOALS; i++)
	{
		aip->goals[i].ai_mode = AI_GOAL_NONE;
		aip->goals[i].signature = -1;
		aip->goals[i].priority = -1;
		aip->goals[i].flags = 0;
	}

	shipp->cargo1 = p_objp->cargo1;

	shipp->arrival_location = p_objp->arrival_location;
	shipp->arrival_distance = p_objp->arrival_distance;
	shipp->arrival_anchor = p_objp->arrival_anchor;
	shipp->arrival_path_mask = p_objp->arrival_path_mask;
	shipp->arrival_cue = p_objp->arrival_cue;
	shipp->arrival_delay = p_objp->arrival_delay;
	shipp->departure_location = p_objp->departure_location;
	shipp->departure_anchor = p_objp->departure_anchor;
	shipp->departure_path_mask = p_objp->departure_path_mask;
	shipp->departure_cue = p_objp->departure_cue;
	shipp->departure_delay = p_objp->departure_delay;
	shipp->determination = p_objp->determination;
	shipp->wingnum = p_objp->wingnum;
	shipp->hotkey = p_objp->hotkey;
	shipp->score = p_objp->score;
	shipp->persona_index = p_objp->persona_index;

	// reset texture animations
	shipp->base_texture_anim_frametime = game_get_overall_frametime();

	// handle the replacement textures
	if (p_objp->num_texture_replacements > 0)
	{
		shipp->replacement_textures = shipp->replacement_textures_buf;
	}

	// now fill them in
	for (i = 0; i < p_objp->num_texture_replacements; i++)
	{
		pm = model_get(sip->model_num);

		// look for textures
		for (j = 0; j < pm->n_textures; j++)
		{
			// get texture file name
			bm_get_filename(pm->maps[j].base_map.texture, texture_file);

			// get rid of file extension
			p = strchr(texture_file, '.');
			if (p)
			{
				mprintf(("ignoring extension on file '%s'\n", texture_file));
				*p = 0;
			}

			// now compare the extension-less texture file names
			if (!stricmp(texture_file, p_objp->replacement_textures[i].old_texture))
			{
				// replace it
				shipp->replacement_textures[j] = p_objp->replacement_textures[i].new_texture_id;
				break;
			}
		}
	}

	// Karajorma - Copy across the alt ship classes for FRED. 
	if (Fred_running)
	{
		shipp->num_alt_class_one = p_objp->num_alt_class_one;
		shipp->num_alt_class_two = p_objp->num_alt_class_two;

		for (int m=0;  m< p_objp->num_alt_class_one ; m++)
		{
			if (p_objp->alt_class_one_variable[m] == -1)
			{
				shipp->alt_class_one[m] = p_objp->alt_class_one[m]; 
				shipp->alt_class_one_variable[m] = -1; 
			}
			else 
			{
				shipp->alt_class_one[m] = -1;
				shipp->alt_class_one_variable[m] = p_objp->alt_class_one_variable[m];
			}
		}

		for (int n=0; n < p_objp->num_alt_class_two ; n++)
		{
			if (p_objp->alt_class_two_variable[n] == -1)
			{
				shipp->alt_class_two[n] = p_objp->alt_class_two[n]; 
				shipp->alt_class_two_variable[n] = -1; 
			}
			else 
			{
				shipp->alt_class_two[n] = -1;
				shipp->alt_class_two_variable[n] = p_objp->alt_class_two_variable[n];
			}
		}
	}


	// check the parse object's flags for possible things to set on this newly created ship
	resolve_parse_flags(&Objects[objnum], p_objp->flags, p_objp->flags2);


	// other flag checks
////////////////////////
	if (p_objp->parse_max_shield_strength == 0.0f)
		Objects[objnum].flags |= OF_NO_SHIELDS;

	// don't set the flag if the mission is ongoing in a multiplayer situation. This will be set by the players in the
	// game only before the game or during respawning.
	// MWA -- changed the next line to remove the !(Game_mode & GM_MULTIPLAYER).  We shouldn't be setting
	// this flag in single player mode -- it gets set in post process mission.
	//if ((p_objp->flags & P_OF_PLAYER_START) && (((Game_mode & GM_MULTIPLAYER) && !(Game_mode & GM_IN_MISSION)) || !(Game_mode & GM_MULTIPLAYER)))
	if ((p_objp->flags & P_OF_PLAYER_START) && (Fred_running || ((Game_mode & GM_MULTIPLAYER) && !(Game_mode & GM_IN_MISSION)))) 
		Objects[objnum].flags |= OF_PLAYER_SHIP;

	// a couple of ai_info flags.  Also, do a reasonable default for the kamikaze damage regardless of
	// whether this flag is set or not
	if (p_objp->flags & P_AIF_KAMIKAZE)
	{
		Ai_info[shipp->ai_index].ai_flags |= AIF_KAMIKAZE;
		Ai_info[shipp->ai_index].kamikaze_damage = p_objp->kamikaze_damage;
	}

	if (p_objp->flags & P_AIF_NO_DYNAMIC)
		Ai_info[shipp->ai_index].ai_flags |= AIF_NO_DYNAMIC;

	if (p_objp->flags & P_SF_RED_ALERT_STORE_STATUS)
	{
		Assert(!(Game_mode & GM_MULTIPLAYER));
		shipp->flags |= SF_RED_ALERT_STORE_STATUS;
	}

	if (p_objp->flags & P_KNOSSOS_WARP_IN)
	{
		Objects[objnum].flags |= OF_SPECIAL_WARP;
		Knossos_warp_ani_used = 1;
	}

	// set the orders that this ship will accept.  It will have already been set to default from the
	// ship create code, so only set them if the parse object flags say they are unique
	if (p_objp->flags & P_SF_USE_UNIQUE_ORDERS)
	{
		shipp->orders_accepted = p_objp->orders_accepted;

		// MWA  5/15/98 -- Added the following debug code because some orders that ships
		// will accept were apparently written out incorrectly with Fred.  This Int3() should
		// trap these instances.
		if (Fred_running)
		{
			int default_orders, remaining_orders;
			
			default_orders = ship_get_default_orders_accepted(&Ship_info[shipp->ship_info_index]);
			remaining_orders = p_objp->orders_accepted & ~default_orders;
			if (remaining_orders)
			{
#ifdef WIN32
				char buf[4096];
				sprintf(buf, "Ship %s can be given orders by the player, that \nare not part of the valid orders for its type. \nThis is most commonly caused by changing ship classes in FRED.\nInvalid orders are: \n", shipp->ship_name);
				flags_to_string(buf, remaining_orders, Player_orders, Num_player_orders);
				strcat(buf, "\nDo you wish to clear these orders? (Changes will take effect on next save)");
				int z = MessageBox(NULL, buf, "Parse Warning", MB_ICONQUESTION | MB_YESNO);

				if (z == IDYES) {
					shipp->orders_accepted &= ~(remaining_orders);
				}
#else
				Warning(LOCATION, "Ship %s has orders which it will accept that are\nnot part of default orders accepted.\n\nPlease reedit this ship and change the orders again\n", shipp->ship_name);
#endif
			}
		}
	}

	if (p_objp->flags & P_SF_DOCK_LEADER)
		shipp->flags |= SF_DOCK_LEADER;

	if (p_objp->flags & P_SF_WARP_BROKEN)
		shipp->flags |= SF_WARP_BROKEN;

	if (p_objp->flags & P_SF_WARP_NEVER)
		shipp->flags |= SF_WARP_NEVER;
////////////////////////


	// if ship is in a wing, and the wing's no_warp_effect flag is set, then set the equivalent
	// flag for the ship
	if ((shipp->wingnum != -1) && (Wings[shipp->wingnum].flags & WF_NO_ARRIVAL_WARP))
		shipp->flags |= SF_NO_ARRIVAL_WARP;

	if ((shipp->wingnum != -1) && (Wings[shipp->wingnum].flags & WF_NO_DEPARTURE_WARP))
		shipp->flags |= SF_NO_DEPARTURE_WARP;

	// ditto for Kazan
	if ((shipp->wingnum != -1) && (Wings[shipp->wingnum].flags & WF_NAV_CARRY))
		shipp->flags2 |= SF2_NAVPOINT_CARRY;
	
	// if the wing index and wing pos are set for this parse object, set them for the ship.  This
	// is useful in multiplayer when ships respawn
	shipp->wing_status_wing_index = p_objp->wing_status_wing_index;
	shipp->wing_status_wing_pos = p_objp->wing_status_wing_pos;


	// set up the ai_goals for this object -- all ships created here are AI controlled.
	if (p_objp->ai_goals != -1)
	{
		int sexp;

		// set up the ai goals for this object.
		for (sexp = CDR(p_objp->ai_goals); sexp != -1; sexp = CDR(sexp))
			ai_add_ship_goal_sexp(sexp, AIG_TYPE_EVENT_SHIP, aip);

		// free the sexpression nodes only for non-wing ships.  wing code will handle its own case
		if (p_objp->wingnum < 0)
			free_sexp2(p_objp->ai_goals);	// free up sexp nodes for reuse, since they aren't needed anymore.
	}

	Assert(sip->model_num != -1);

	// initialize subsystem statii here.  The subsystems are given a percentage damaged.  So a percent value
	// of 20% means that the subsystem is 20% damaged (*not* 20% of max hits).  This is opposite the way
	// that the initial velocity/hull strength/shields work
	i = p_objp->subsys_count;
	while (i--)
	{
		sssp = &Subsys_status[p_objp->subsys_index + i];
		if (!stricmp(sssp->name, NOX("Pilot")))
		{
			wp = &shipp->weapons;
			if (sssp->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
			{
				for (j=k=0; j<MAX_SHIP_PRIMARY_BANKS; j++)
				{
					if ((sssp->primary_banks[j] >= 0) || Fred_running)
					{
						wp->primary_bank_weapons[k] = sssp->primary_banks[j];						

						// next
						k++;
					}
				}

				if (Fred_running)
					wp->num_primary_banks = sip->num_primary_banks;
				else
					wp->num_primary_banks = k;
			}

			if (sssp->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
			{
				for (j = k = 0; j < MAX_SHIP_SECONDARY_BANKS; j++)
				{
					if ((sssp->secondary_banks[j] >= 0) || Fred_running)
						wp->secondary_bank_weapons[k++] = sssp->secondary_banks[j];
				}

				if (Fred_running)
					wp->num_secondary_banks = sip->num_secondary_banks;
				else
					wp->num_secondary_banks = k;
			}

			// primary weapons too - Goober5000
			for (j = 0; j < wp->num_primary_banks; j++)
			{
				if (Fred_running)
				{
					wp->primary_bank_ammo[j] = sssp->primary_ammo[j];
				}
				else
				{
					int capacity = fl2i(sssp->primary_ammo[j]/100.0f * sip->primary_bank_ammo_capacity[j] + 0.5f);
					wp->primary_bank_ammo[j] = fl2i(capacity / Weapon_info[wp->primary_bank_weapons[j]].cargo_size + 0.5f);
				}
			}

			for (j = 0; j < wp->num_secondary_banks; j++)
			{
				if (Fred_running)
				{
					wp->secondary_bank_ammo[j] = sssp->secondary_ammo[j];
				}
				else
				{
					int capacity = fl2i(sssp->secondary_ammo[j]/100.0f * sip->secondary_bank_ammo_capacity[j] + 0.5f);
					wp->secondary_bank_ammo[j] = fl2i(capacity / Weapon_info[wp->secondary_bank_weapons[j]].cargo_size + 0.5f);
				}
			}

			continue;
		}

		ptr = GET_FIRST(&shipp->subsys_list);
		while (ptr != END_OF_LIST(&shipp->subsys_list))
		{
			// check the mission flag to possibly free all beam weapons - Goober5000, taken from SEXP.CPP
			if (The_mission.flags & MISSION_FLAG_BEAM_FREE_ALL_BY_DEFAULT)
			{
				// mark all turrets as beam free
				if(ptr->system_info->type == SUBSYSTEM_TURRET)
				{
					ptr->weapons.flags |= SW_FLAG_BEAM_FREE;
					ptr->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
				}
			}

			if (!subsystem_stricmp(ptr->system_info->subobj_name, sssp->name))
			{
				if (Fred_running)
				{
					ptr->current_hits = sssp->percent;
					ptr->max_hits = 100.0f;
				}
				else
				{
					ptr->max_hits = ptr->system_info->max_subsys_strength * (shipp->ship_max_hull_strength / sip->max_hull_strength);

					float new_hits;
					new_hits = ptr->max_hits * (100.0f - sssp->percent) / 100.f;
					shipp->subsys_info[ptr->system_info->type].current_hits -= (ptr->max_hits - new_hits);
					if ((100.0f - sssp->percent) < 0.5)
					{
						ptr->current_hits = 0.0f;
						ptr->submodel_info_1.blown_off = 1;
					}
					else
					{
						ptr->current_hits = new_hits;
					}
				}

				if (sssp->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
					for (j=0; j<MAX_SHIP_PRIMARY_BANKS; j++)
						ptr->weapons.primary_bank_weapons[j] = sssp->primary_banks[j];

				if (sssp->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
					for (j=0; j<MAX_SHIP_SECONDARY_BANKS; j++)
						ptr->weapons.secondary_bank_weapons[j] = sssp->secondary_banks[j];

				// Goober5000
				for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++)
					ptr->weapons.primary_bank_ammo[j] = sssp->primary_ammo[j];

				// AL 3-5-98:  This is correct for FRED, but not for FreeSpace... but is this even used?
				//					As far as I know, turrets cannot run out of ammo
				for (j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++)
					ptr->weapons.secondary_bank_ammo[j] = sssp->secondary_ammo[j];

				ptr->subsys_cargo_name = sssp->subsys_cargo_name;

				if (sssp->ai_class != SUBSYS_STATUS_NO_CHANGE)
					ptr->weapons.ai_class = sssp->ai_class;

				ptr->turret_best_weapon = -1;
				ptr->turret_animation_position = 0;	// MA_POS_NOT_SET -> model animation position is not set
				ptr->turret_animation_done_time = 0;
			}

			ptr = GET_NEXT(ptr);
		}
	}
	
	// initial hull strength, shields, and velocity are all expressed as a percentage of the max value/
	// so a initial_hull value of 90% means 90% of max.  This way is opposite of how subsystems are dealt
	// with
	if (Fred_running)
	{
		Objects[objnum].phys_info.speed = (float) p_objp->initial_velocity;
		// shipp->hull_hit_points_taken = (float) p_objp->initial_hull;
		Objects[objnum].hull_strength = (float) p_objp->initial_hull;
		shield_set_quad(&Objects[objnum], 0, (float) p_objp->initial_shields);

	}
	else
	{
		int max_allowed_sparks, num_sparks, i;
		polymodel *pm;

		Objects[objnum].hull_strength = shipp->ship_max_hull_strength * ((float) p_objp->initial_hull / 100.0f);
		for (i = 0; i<MAX_SHIELD_SECTIONS; i++)
			shield_set_quad(&Objects[objnum], i, shield_get_max_quad(&Objects[objnum]) * ((float) p_objp->initial_shields / 100.0f));

		// initial velocities now do not apply to ships which warp in after mission starts
		if (!(Game_mode & GM_IN_MISSION))
		{
			Objects[objnum].phys_info.speed = (float) p_objp->initial_velocity * sip->max_speed / 100.0f;
			Objects[objnum].phys_info.vel.xyz.z = Objects[objnum].phys_info.speed;
			Objects[objnum].phys_info.prev_ramp_vel = Objects[objnum].phys_info.vel;
			Objects[objnum].phys_info.desired_vel = Objects[objnum].phys_info.vel;
		}

		// recalculate damage of subsystems
		ship_recalc_subsys_strength(&Ships[shipnum]);

		// create sparks on a ship whose hull is damaged.  We will create two sparks for every 20%
		// of hull damage done.  100 means no sparks.  between 80 and 100 do two sparks.  60 and 80 is
		// four, etc.
		pm = model_get(sip->model_num);
		max_allowed_sparks = get_max_sparks(&Objects[objnum]);
		num_sparks = (int)((100.0f - p_objp->initial_hull) / 5.0f);
		if (num_sparks > max_allowed_sparks)
			num_sparks = max_allowed_sparks;

		for (i = 0; i < num_sparks; i++)
		{
			vec3d v1, v2;

			// DA 10/20/98 - sparks must be chosen on the hull and not any submodel
			submodel_get_two_random_points(sip->model_num, pm->detail[0], &v1, &v2);
			ship_hit_sparks_no_rotate(&Objects[objnum], &v1);
//			ship_hit_sparks_no_rotate(&Objects[objnum], &v2);
		}
	}

	// in mission, we add a log entry -- set ship positions for ships not in wings, and then do
	// warpin effect
	if ((Game_mode & GM_IN_MISSION) && (!Fred_running))
	{
		mission_log_add_entry(LOG_SHIP_ARRIVED, shipp->ship_name, NULL);

		if (!Game_restoring)
		{
			// if this ship isn't in a wing, determine its arrival location
			if (shipp->wingnum == -1)
			{
				int location;

				// multiplayer clients set the arrival location of ships to be at location since their
				// position has already been determined.  Don't actually set the variable since we
				// don't want the warp effect to show if coming from a dock bay.
				location = p_objp->arrival_location;

				if (MULTIPLAYER_CLIENT)
					location = ARRIVE_AT_LOCATION;

				mission_set_arrival_location(p_objp->arrival_anchor, location, p_objp->arrival_distance, objnum, p_objp->arrival_path_mask, NULL, NULL);

				// Goober5000 - warpin start moved to parse_create_object
			}
		}
		
		// possibly add this ship to a hotkey set
		if ((shipp->wingnum == -1) && (shipp->hotkey != -1))
			mission_hotkey_mf_add(shipp->hotkey, shipp->objnum, HOTKEY_MISSION_FILE_ADDED);
		else if ((shipp->wingnum != -1) && (Wings[shipp->wingnum].hotkey != -1))
			mission_hotkey_mf_add(Wings[shipp->wingnum].hotkey, shipp->objnum, HOTKEY_MISSION_FILE_ADDED);

		// possibly add this ship to the hud escort list
		if (shipp->flags & SF_ESCORT)
			hud_add_remove_ship_escort(objnum, 1);
	}

	// for multiplayer games, make a call to the network code to assign the object signature
	// of the newly created object.  The network host of the netgame will always assign a signature
	// to a newly created object.  The network signature will get to the clients of the game in
	// different manners depending on whether or not an individual ship or a wing was created.
	if (Game_mode & GM_MULTIPLAYER)
	{
		Objects[objnum].net_signature = p_objp->net_signature;

		if ((Game_mode & GM_IN_MISSION) && MULTIPLAYER_MASTER && (p_objp->wingnum == -1))
			send_ship_create_packet(&Objects[objnum], (p_objp == Arriving_support_ship) ? 1 : 0);
	}

	// if recording a demo, post the event
	if(Game_mode & GM_DEMO_RECORD)
		demo_POST_obj_create(p_objp->name, Objects[objnum].signature);

	return objnum;
}

// Karajorma
int parse_alt_class(p_object *p_objp, int num_alts, bool alt_type_1) 
{
	char alt_ship_class[128]; 
	int recorded_as = get_string_or_variable(alt_ship_class);

	int alt_class_index = -1; 
	int variable_index = -1;

	// ship name
	if (recorded_as == FOUND_STRING)
	{
		alt_class_index = ship_info_lookup(alt_ship_class);
	}
	// string variable
	else 
	{
		variable_index = get_index_sexp_variable_name(alt_ship_class);
		alt_class_index = ship_info_lookup(Sexp_variables[variable_index].text);
	}
	
	if (alt_class_index < 0 )
	{
		Warning(LOCATION, "Ship \"%s\" has an invalid Alternate Ship Class type (ships.tbl probably changed). Skipping this entry", p_objp->name); 
	}
	else
	{
		if (alt_type_1)
		{
			p_objp->alt_class_one[num_alts] = alt_class_index; 
			p_objp->alt_class_one_variable[num_alts++] = variable_index; 
			p_objp->num_alt_class_one = num_alts; 
		}
		else 
		{
			p_objp->alt_class_two[num_alts] = alt_class_index; 
			p_objp->alt_class_two_variable[num_alts++] = variable_index; 
			p_objp->num_alt_class_two = num_alts; 
		}
	}	

	return num_alts; 
}

// Goober5000
void resolve_parse_flags(object *objp, int parse_flags, int parse_flags2)
{
	Assert(objp != NULL);
	ship *shipp = &Ships[objp->instance];

	if (parse_flags & P_SF_CARGO_KNOWN)
		shipp->flags |= SF_CARGO_REVEALED;

	if (parse_flags & P_SF_IGNORE_COUNT)
		shipp->flags |= SF_IGNORE_COUNT;

	if (parse_flags & P_OF_PROTECTED)
		objp->flags |= OF_PROTECTED;

	if (parse_flags & P_SF_REINFORCEMENT)
		shipp->flags |= SF_REINFORCEMENT;

	if (parse_flags & P_OF_NO_SHIELDS)
		objp->flags |= OF_NO_SHIELDS;

	if (parse_flags & P_SF_ESCORT)
		shipp->flags |= SF_ESCORT;

	// P_OF_PLAYER_START is handled in parse_create_object_sub

	if (parse_flags & P_SF_NO_ARRIVAL_MUSIC)
		shipp->flags |= SF_NO_ARRIVAL_MUSIC;

	if (parse_flags & P_SF_NO_ARRIVAL_WARP)
		shipp->flags |= SF_NO_ARRIVAL_WARP;

	if (parse_flags & P_SF_NO_DEPARTURE_WARP)
		shipp->flags |= SF_NO_DEPARTURE_WARP;

	if (parse_flags & P_SF_LOCKED)
		shipp->flags |= SF_LOCKED;

	if (parse_flags & P_OF_INVULNERABLE)
		objp->flags |= OF_INVULNERABLE;

	if (parse_flags & P_SF_HIDDEN_FROM_SENSORS)
		shipp->flags |= SF_HIDDEN_FROM_SENSORS;

	if (parse_flags & P_SF_SCANNABLE)
		shipp->flags |= SF_SCANNABLE;

	// P_AIF_KAMIKAZE, P_AIF_NO_DYNAMIC, and P_SF_RED_ALERT_CARRY are handled in parse_create_object_sub
	
	if (parse_flags & P_OF_BEAM_PROTECTED)
		objp->flags |= OF_BEAM_PROTECTED;

	if (parse_flags & P_SF_GUARDIAN)
		shipp->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;

	if (parse_flags & P_SF_VAPORIZE)
		shipp->flags |= SF_VAPORIZE;

	if (parse_flags & P_SF2_STEALTH)
		shipp->flags2 |= SF2_STEALTH;

	if (parse_flags & P_SF2_FRIENDLY_STEALTH_INVIS)
		shipp->flags2 |= SF2_FRIENDLY_STEALTH_INVIS;

	if (parse_flags & P_SF2_DONT_COLLIDE_INVIS)
		shipp->flags2 |= SF2_DONT_COLLIDE_INVIS;

	if (parse_flags2 & P2_SF2_PRIMITIVE_SENSORS)
		shipp->flags2 |= SF2_PRIMITIVE_SENSORS;

	if (parse_flags2 & P2_SF2_NO_SUBSPACE_DRIVE)
		shipp->flags2 |= SF2_NO_SUBSPACE_DRIVE;

	if (parse_flags2 & P2_SF2_NAV_CARRY_STATUS)
		shipp->flags2 |= SF2_NAVPOINT_CARRY;

	if (parse_flags2 & P2_SF2_AFFECTED_BY_GRAVITY)
		shipp->flags2 |= SF2_AFFECTED_BY_GRAVITY;

	if (parse_flags2 & P2_SF2_TOGGLE_SUBSYSTEM_SCANNING)
		shipp->flags2 |= SF2_TOGGLE_SUBSYSTEM_SCANNING;

	if (parse_flags2 & P2_OF_TARGETABLE_AS_BOMB)
		objp->flags |= OF_TARGETABLE_AS_BOMB;

	if (parse_flags2 & P2_SF2_NO_BUILTIN_MESSAGES) 
		shipp->flags2 |= SF2_NO_BUILTIN_MESSAGES;

	if (parse_flags2 & P2_SF2_PRIMARIES_LOCKED) 
		shipp->flags2 |= SF2_PRIMARIES_LOCKED;

	if (parse_flags2 & P2_SF2_SECONDARIES_LOCKED) 
		shipp->flags2 |= SF2_SECONDARIES_LOCKED;

	if (parse_flags2 & P2_SF2_SET_CLASS_DYNAMICALLY) 
		shipp->flags2 |= SF2_SET_CLASS_DYNAMICALLY;

	if (parse_flags2 & P2_SF2_TEAM_LOADOUT_STORE_STATUS) 
		shipp->flags2 |= SF2_TEAM_LOADOUT_STORE_STATUS;

	if (parse_flags2 & P2_SF2_NO_DEATH_SCREAM)
		shipp->flags2 |= SF2_NO_DEATH_SCREAM;
	
	if (parse_flags2 & P2_SF2_ALWAYS_DEATH_SCREAM)
		shipp->flags2 |= SF2_ALWAYS_DEATH_SCREAM;
}

//	Mp points at the text of an object, which begins with the "$Name:" field.
//	Snags all object information.  Creating the ship now only happens after everything has been parsed.
//
// flag is parameter that is used to tell what kind information we are retrieving from the mission.
// if we are just getting player starts, then don't create the objects
int parse_object(mission *pm, int flag, p_object *p_objp)
{
	int	i, j, count, delay, num_alts;
	char name[NAME_LENGTH], flag_strings[MAX_PARSE_OBJECT_FLAGS][NAME_LENGTH];
	char flag_strings_2[MAX_PARSE_OBJECT_FLAGS_2][NAME_LENGTH];

	Assert(pm != NULL);

	// Goober5000
	p_objp->created_object = NULL;
	p_objp->next = NULL;
	p_objp->prev = NULL;

	required_string("$Name:");
	stuff_string(p_objp->name, F_NAME, NAME_LENGTH);
	if (mission_parse_get_parse_object(p_objp->name))
		error_display(0, NOX("Redundant ship name: %s\n"), p_objp->name);


	find_and_stuff("$Class:", &p_objp->ship_class, F_NAME, Ship_class_names, Num_ship_classes, "ship class");
	if (p_objp->ship_class < 0)
	{
		Warning(LOCATION, "Ship \"%s\" has an invalid ship type (ships.tbl probably changed).  Making it type 0", p_objp->name);

		// if fred is running, maybe notify the user that the mission contains MD content
		if (Fred_running)
			Fred_found_unknown_ship_during_parsing = 1;

		p_objp->ship_class = 0;
	}

	// Karajorma - See if there are any alternate classes specified for this ship. 

	// Type 1 Alts
	p_objp->num_alt_class_one = 0;
	num_alts = 0;
	while (optional_string("+Alt_Ship_Class_Type_1:") && (num_alts < MAX_ALT_CLASS_1))
	{		
		num_alts = parse_alt_class(p_objp, num_alts, true); 
	}

	// Type 2 Alts
	p_objp->num_alt_class_two = 0;
	num_alts = 0;
	while (optional_string("+Alt_Ship_Class_Type_2:") && (num_alts < MAX_ALT_CLASS_2))
	{
		parse_alt_class(p_objp, num_alts, false); 
	}

	// if this is a multiplayer dogfight mission, skip support ships
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (Ship_info[p_objp->ship_class].flags & SIF_SUPPORT))
		return 0;

	// optional alternate name type
	p_objp->alt_type_index = -1;
	if(optional_string("$Alt:"))
	{
		// alternate name
		stuff_string(name, F_NAME, NAME_LENGTH);

		// try and find the alternate name
		p_objp->alt_type_index = (char)mission_parse_lookup_alt(name);
		Assert(p_objp->alt_type_index >= 0);
		if(p_objp->alt_type_index < 0)
			mprintf(("Error looking up alternate ship type name!\n"));
		else
			mprintf(("Using alternate ship type name : %s\n", name));
	}

	// static alias stuff - stupid, but it seems to be necessary
	static char *temp_team_names[MAX_IFFS];
	for (i = 0; i < Num_iffs; i++)
		temp_team_names[i] = Iff_info[i].iff_name;

	find_and_stuff("$Team:", &p_objp->team, F_NAME, temp_team_names, Num_iffs, "team name");

	required_string("$Location:");
	stuff_vector(&p_objp->pos);

	required_string("$Orientation:");
	stuff_matrix(&p_objp->orient);

	// legacy code, not even used in FS1
	if (optional_string("$IFF:"))
	{
		stuff_string(name, F_NAME, NAME_LENGTH);
	}

	find_and_stuff("$AI Behavior:",	&p_objp->behavior, F_NAME, Ai_behavior_names, Num_ai_behaviors, "AI behavior");
	p_objp->ai_goals = -1;

	if (optional_string("+AI Class:")) 
	{
		p_objp->ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");
		Assert(p_objp->ai_class > -1);
	}
	else
	{
		p_objp->ai_class = Ship_info[p_objp->ship_class].ai_class;
	}

	if (optional_string("$AI Goals:"))
		p_objp->ai_goals = get_sexp_main();

	if (!required_string_either("$AI Goals:", "$Cargo 1:"))
	{
		required_string("$AI Goals:");
		p_objp->ai_goals = get_sexp_main();
	}

	p_objp->cargo1 = -1;
	int temp;
	find_and_stuff_or_add("$Cargo 1:", &temp, F_NAME, Cargo_names, &Num_cargo, MAX_CARGO, "cargo");
	p_objp->cargo1 = char(temp);
	if (optional_string("$Cargo 2:"))
	{
		stuff_string(name, F_NAME, NAME_LENGTH);
	}

	parse_common_object_data(p_objp);  // get initial conditions and subsys status
	count = 0;
	while (required_string_either("$Arrival Location:", "$Status Description:"))	{
		Assert(count < MAX_OBJECT_STATUS);

		find_and_stuff("$Status Description:", &p_objp->status_type[count], F_NAME, Status_desc_names, Num_status_names, "Status Description");
		find_and_stuff("$Status:", &p_objp->status[count], F_NAME, Status_type_names, Num_status_names, "Status Type");
		find_and_stuff("$Target:", &p_objp->target[count], F_NAME, Status_target_names, Num_status_names, "Target");
		count++;
	}
	p_objp->status_count = count;

	p_objp->arrival_anchor = -1;
	p_objp->arrival_distance = 0;
	p_objp->arrival_path_mask = -1;	// -1 only until resolved

	find_and_stuff("$Arrival Location:", &p_objp->arrival_location, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");

	if (optional_string("+Arrival Distance:"))
	{
		stuff_int(&p_objp->arrival_distance);

		// Goober5000
		if ((p_objp->arrival_distance <= 0) && ((p_objp->arrival_location == ARRIVE_NEAR_SHIP) || (p_objp->arrival_location == ARRIVE_IN_FRONT_OF_SHIP)))
		{
			Warning(LOCATION, "Arrival distance for ship %s cannot be %d.  Setting to 1.\n", p_objp->name, p_objp->arrival_distance);
			p_objp->arrival_distance = 1;
		}
	}

	if (p_objp->arrival_location != ARRIVE_AT_LOCATION)
	{
		required_string("$Arrival Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		p_objp->arrival_anchor = get_anchor(name);
	}

	if (optional_string("+Arrival Paths:"))
	{
		// temporarily use mask to point to the restriction index
		p_objp->arrival_path_mask = add_path_restriction();
	}

	delay = 0;
	if (optional_string("+Arrival Delay:"))
	{
		stuff_int(&delay);
		if (delay < 0)
			Error(LOCATION, "Cannot have arrival delay < 0 (ship %s)", p_objp->name);
	}

	if (!Fred_running)
		p_objp->arrival_delay = -delay;			// use negative numbers to mean we haven't set up a timer yet
	else
		p_objp->arrival_delay = delay;

	required_string("$Arrival Cue:");
	p_objp->arrival_cue = get_sexp_main();
	if (!Fred_running && (p_objp->arrival_cue >= 0))
	{
		// eval the arrival cue.  if the cue is true, set up the timestamp for the arrival delay
		Assert (p_objp->arrival_delay <= 0);

		// don't eval arrival_cues when just looking for player information.
		// evaluate to determine if sexp is always false.
		if (eval_sexp(p_objp->arrival_cue))
			p_objp->arrival_delay = timestamp(-p_objp->arrival_delay * 1000);
	}

	p_objp->departure_anchor = -1;
	p_objp->departure_path_mask = -1;	// -1 only until resolved

	find_and_stuff("$Departure Location:", &p_objp->departure_location, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");

	if (p_objp->departure_location != DEPART_AT_LOCATION)
	{
		required_string("$Departure Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		p_objp->departure_anchor = get_anchor(name);
	}

	if (optional_string("+Departure Paths:"))
	{
		// temporarily use mask to point to the restriction index
		p_objp->departure_path_mask = add_path_restriction();
	}

	delay = 0;
	if (optional_string("+Departure Delay:"))
	{
		stuff_int(&delay);
		if (delay < 0)
			Error(LOCATION, "Cannot have departure delay < 0 (ship %s)", p_objp->name);
	}

	if (!Fred_running)
		p_objp->departure_delay = -delay;
	else
		p_objp->departure_delay = delay;

	required_string("$Departure Cue:");
	p_objp->departure_cue = get_sexp_main();

	if (optional_string("$Misc Properties:"))
		stuff_string(p_objp->misc, F_NAME, NAME_LENGTH);

	required_string("$Determination:");
	stuff_int(&p_objp->determination);

	// set flags
	p_objp->flags = 0;
	if (optional_string("+Flags:"))
	{
		count = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS);
		for (i=0; i<count; i++)
		{
			for (j=0; j<MAX_PARSE_OBJECT_FLAGS; j++)
			{
				if (!stricmp(flag_strings[i], Parse_object_flags[j]))
				{
					p_objp->flags |= (1 << j);
					break;
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS)
				Warning(LOCATION, "Unknown flag in mission file: %s\n", flag_strings[i]);
		}
	}

	// second set - Goober5000
	p_objp->flags2 = 0;
	if (optional_string("+Flags2:"))
	{
		count = stuff_string_list(flag_strings_2, MAX_PARSE_OBJECT_FLAGS_2);
		for (i=0; i<count; i++)
		{
			for (j=0; j<MAX_PARSE_OBJECT_FLAGS_2; j++)
			{
				if (!stricmp(flag_strings_2[i], Parse_object_flags_2[j]))
				{
					p_objp->flags2 |= (1 << j);
					break;
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS_2)
				Warning(LOCATION, "Unknown flag2 in mission file: %s\n", flag_strings_2[i]);
		}
	}

	// set certain flags that used to be in ship_info - Goober5000
	if (Ship_info[p_objp->ship_class].flags & SIF_STEALTH)
		p_objp->flags |= P_SF2_STEALTH;
	if (Ship_info[p_objp->ship_class].flags & SIF_SHIP_CLASS_DONT_COLLIDE_INVIS)
		p_objp->flags |= P_SF2_DONT_COLLIDE_INVIS;


	// always store respawn priority, just for ease of implementation
	p_objp->respawn_priority = 0;
	if(optional_string("+Respawn Priority:"))
		stuff_int(&p_objp->respawn_priority);	

	p_objp->escort_priority = 0;
	if (optional_string("+Escort Priority:"))
	{
		Assert(p_objp->flags & P_SF_ESCORT);
		stuff_int(&p_objp->escort_priority);
	}	

	if (p_objp->flags & P_OF_PLAYER_START)
	{
		p_objp->flags |= P_SF_CARGO_KNOWN;				// make cargo known for players
		Player_starts++;
	}

	p_objp->special_exp_index = -1;
	if (optional_string("+Special Exp index:"))
		stuff_int(&p_objp->special_exp_index);

	p_objp->special_hitpoint_index = -1;
	if (optional_string("+Special Hitpoint index:"))
		stuff_int(&p_objp->special_hitpoint_index);

	// get hitpoint values
	if (p_objp->special_hitpoint_index != -1)
	{
		p_objp->parse_max_shield_strength = (float) atoi(Sexp_variables[p_objp->special_hitpoint_index+SHIELD_STRENGTH].text);
		p_objp->parse_max_hull_strength = (float) atoi(Sexp_variables[p_objp->special_hitpoint_index+HULL_STRENGTH].text);
	}
	else
	{
		p_objp->parse_max_shield_strength = Ship_info[p_objp->ship_class].max_shield_strength;
		p_objp->parse_max_hull_strength = Ship_info[p_objp->ship_class].max_hull_strength;
	}
	Assert(p_objp->parse_max_hull_strength > 0.0f);	// Goober5000: div-0 check (not shield because we might not have one)

	// if the kamikaze flag is set, we should have the next flag
	if (optional_string("+Kamikaze Damage:"))
	{
		int damage;

		stuff_int(&damage);
		p_objp->kamikaze_damage = i2fl(damage);
	}

	p_objp->hotkey = -1;
	if (optional_string("+Hotkey:"))
	{
		stuff_int(&p_objp->hotkey);
		Assert((p_objp->hotkey >= 0) && (p_objp->hotkey < 10));
	}

	// Goober5000
	p_objp->dock_list = NULL;
	while (optional_string("+Docked With:"))
	{
		char docked_with[NAME_LENGTH];
		char docker_point[NAME_LENGTH];
		char dockee_point[NAME_LENGTH];

		// grab docking information
		// (whoever designed the original docking system
		// reversed the dockpoints in the mission file)
		stuff_string(docked_with, F_NAME, NAME_LENGTH);
		required_string("$Docker Point:");
		stuff_string(dockee_point, F_NAME, NAME_LENGTH);
		required_string("$Dockee Point:");
		stuff_string(docker_point, F_NAME, NAME_LENGTH);	

		// make sure we don't overflow the limit
		if (Total_initially_docked >= MAX_SHIPS)
		{
			mprintf(("Too many initially docked instances; skipping...\n"));
			continue;
		}

		// put this information into the Initially_docked array
		strcpy(Initially_docked[Total_initially_docked].docker, p_objp->name);
		strcpy(Initially_docked[Total_initially_docked].dockee, docked_with);
		strcpy(Initially_docked[Total_initially_docked].docker_point, docker_point);
		strcpy(Initially_docked[Total_initially_docked].dockee_point, dockee_point);
		Total_initially_docked++;
	}

	// check the optional parameter for destroying the ship before the mission starts.  If this parameter is
	// here, then we need to destroy the ship N seconds before the mission starts (for debris purposes).
	// store the time value here.  We want to create this object for sure.  Set the arrival cue and arrival
	// delay to bogus values
	p_objp->destroy_before_mission_time = -1;
	if (optional_string("+Destroy At:"))
	{
		stuff_int(&p_objp->destroy_before_mission_time);
		Assert (p_objp->destroy_before_mission_time >= 0);
		p_objp->arrival_cue = Locked_sexp_true;
		p_objp->arrival_delay = timestamp(0);
	}

	// check for the optional "orders accepted" string which contains the orders from the default
	// set that this ship will actually listen to
	if (optional_string("+Orders Accepted:"))
	{
		stuff_int(&p_objp->orders_accepted);
		if (p_objp->orders_accepted != -1)
			p_objp->flags |= P_SF_USE_UNIQUE_ORDERS;
	}

	p_objp->group = 0;
	if (optional_string("+Group:"))
		stuff_int(&p_objp->group);

	p_objp->score = 0;
	if (optional_string("+Score:"))
		stuff_int(&p_objp->score);

	// parse the persona index if present
	p_objp->persona_index = -1;
	if (optional_string("+Persona Index:"))
		stuff_int(&p_objp->persona_index);

	// texture replacement - Goober5000
	p_objp->num_texture_replacements = 0;
	if (optional_string("$Texture Replace:") || optional_string("$Duplicate Model Texture Replace:"))
	{
		char *p;

		while ((p_objp->num_texture_replacements < MAX_MODEL_TEXTURES) && (optional_string("+old:")))
		{
			stuff_string(p_objp->replacement_textures[p_objp->num_texture_replacements].old_texture, F_NAME, MAX_FILENAME_LEN);
			required_string("+new:");
			stuff_string(p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture, F_NAME, MAX_FILENAME_LEN);

			// get rid of extensions
			p = strchr(p_objp->replacement_textures[p_objp->num_texture_replacements].old_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", p_objp->replacement_textures[p_objp->num_texture_replacements].old_texture));
				*p = 0;
			}
			p = strchr(p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture));
				*p = 0;
			}

			// load the texture
			p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture_id = bm_load(p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture);

			if (p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture_id < 0)
			{
				Warning(LOCATION, "Could not load replacement texture %s for ship %s\n", p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture, p_objp->name);
			}

			// *** account for FRED
			if (Fred_running)
			{
				strcpy(Fred_texture_replacements[Fred_num_texture_replacements].ship_name, p_objp->name);
				strcpy(Fred_texture_replacements[Fred_num_texture_replacements].old_texture, p_objp->replacement_textures[p_objp->num_texture_replacements].old_texture);
				strcpy(Fred_texture_replacements[Fred_num_texture_replacements].new_texture, p_objp->replacement_textures[p_objp->num_texture_replacements].new_texture);
				Fred_texture_replacements[Fred_num_texture_replacements].new_texture_id = -1;
				Fred_num_texture_replacements++;
			}

			// increment
			p_objp->num_texture_replacements++;
		}
	}

	p_objp->wingnum = -1;					// set the wing number to -1 -- possibly to be set later
	p_objp->pos_in_wing = -1;				// Goober5000

	// for multiplayer, assign a network signature to this parse object.  Doing this here will
	// allow servers to use the signature with clients when creating new ships, instead of having
	// to pass ship names all the time
	if (Game_mode & GM_MULTIPLAYER)
		p_objp->net_signature = multi_assign_network_signature(MULTI_SIG_SHIP);

	// set the wing_status position to be -1 for all objects.  This will get set to an appropriate
	// value when the wing positions are finally determined.
	p_objp->wing_status_wing_index = -1;
	p_objp->wing_status_wing_pos = -1;
	p_objp->respawn_count = 0;

	// if this if the starting player ship, then copy if to Starting_player_pobject (used for ingame join)
	if (!stricmp(p_objp->name, Player_start_shipname))
	{
		Player_start_pobject = *p_objp;
		Player_start_pobject.flags |= P_SF_PLAYER_START_VALID;
	}
	

	// Goober5000 - preload stuff for certain object flags
	// (done after parsing object, but before creating it)
	if (p_objp->flags & P_KNOSSOS_WARP_IN)
		Knossos_warp_ani_used = 1;

	// this is a valid/legal ship to create
	return 1;
}

// Goober5000 - I split this because 1) it's clearer; and 2) initially multiple docked ships would have been
// insanely difficult otherwise
//
// Maybe create the object.
// Don't create the new ship blindly.  First, check the sexp for the arrival cue
// to determine when this ship should arrive.  If not right away, stick this ship
// onto the ship arrival list to be looked at later.  Also check to see if it should use the
// wings arrival cue.  The ship may get created later depending on whether or not the wing
// is created.
// Always create ships when FRED is running.
void mission_parse_maybe_create_parse_object(p_object *pobjp)
{
	// Bail if it was already created.  This should only happen when we previously
	// created all the objects in a docked group simultaneously.
	if (pobjp->created_object != NULL)
	{
		Assert(object_is_docked(pobjp));
		return;
	}

	// Goober5000
	// shunt this guy to the arrival list if he meets one of the following conditions:
	// 1) he's docked but not the dock leader
	// 2) this is FS2 (i.e. not FRED2) AND he meets one of the following conditions:
	//    a) he's not cued to arrive yet
	//    b) his arrival delay hasn't elapsed
	//    c) he's reinforcement
	if ( (object_is_docked(pobjp) && !(pobjp->flags & P_SF_DOCK_LEADER)) || (!Fred_running && (!eval_sexp(pobjp->arrival_cue) || !timestamp_elapsed(pobjp->arrival_delay) || (pobjp->flags & P_SF_REINFORCEMENT))) )
	{
		// we can't add ships getting destroyed to the arrival list!!!
		Assert (pobjp->destroy_before_mission_time < 0);

		// add to arrival list
		list_append(&Ship_arrival_list, pobjp);
	}
	// ingame joiners bail here.
	else if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
	{
		return;
	}
	// the ship is present at the beginning of the mission, so we create it
	else
	{
		int	real_objnum = parse_create_object(pobjp);	// this object may later get destroyed depending on wing status!!!!

		Subsys_index = pobjp->subsys_index;  // free elements that are no longer needed.

		// if the ship is supposed to be destroyed before the mission, then blow up the ship and mark the pieces
		// as last forever.  Only call this stuff when you are blowing up the ship
		if (pobjp->destroy_before_mission_time >= 0)
		{
			object *objp = &Objects[real_objnum];

			// FreeSpace
			if (!Fred_running)
			{
				int i;
				shipfx_blow_up_model(objp, Ship_info[Ships[objp->instance].ship_info_index].model_num, 0, 0, &objp->pos);
				objp->flags |= OF_SHOULD_BE_DEAD;

				// once the ship is exploded, find the debris pieces belonging to this object, mark them
				// as not to expire, and move them forward in time N seconds
				for (i = 0; i < MAX_DEBRIS_PIECES; i++)
				{
					debris *db;

					db = &Debris[i];
					if (!db->flags & DEBRIS_USED)				// not used, move onto the next one.
						continue;
					if (db->source_objnum != real_objnum)		// not from this ship, move to next one
						continue;

					debris_clear_expired_flag(db);				// mark as don't expire
					db->lifeleft = -1.0f;						// be sure that lifeleft == -1.0 so that it really doesn't expire!

					// now move the debris along its path for N seconds
					objp = &Objects[db->objnum];
					physics_sim(&objp->pos, &objp->orient, &objp->phys_info, (float) pobjp->destroy_before_mission_time);
				}
			}
			// FRED
			else
			{
				// be sure to set the variable in the ships structure for the final death time!!!
				Ships[objp->instance].final_death_time = pobjp->destroy_before_mission_time;
				Ships[objp->instance].flags |= SF_KILL_BEFORE_MISSION;
			}
		}
	}
}

void parse_common_object_data(p_object	*objp)
{
	int i;

	// set some defaults..
	objp->initial_velocity = 0;
	objp->initial_hull = 100;
	objp->initial_shields = 100;

	// now change defaults if present
	if (optional_string("+Initial Velocity:")) {
		stuff_int(&objp->initial_velocity);
	}

	if (optional_string("+Initial Hull:"))
		stuff_int(&objp->initial_hull);
	if (optional_string("+Initial Shields:"))
		stuff_int(&objp->initial_shields);

	objp->subsys_index = Subsys_index;
	objp->subsys_count = 0;
	while (optional_string("+Subsystem:")) {
		i = allocate_subsys_status();

		objp->subsys_count++;
		stuff_string(Subsys_status[i].name, F_NAME, NAME_LENGTH);
		
		if (optional_string("$Damage:"))
			stuff_float(&Subsys_status[i].percent);

		Subsys_status[i].subsys_cargo_name = -1;
		if (optional_string("+Cargo Name:")) {
			char cargo_name[NAME_LENGTH];
			stuff_string(cargo_name, F_NAME, NAME_LENGTH);
			int index = string_lookup(cargo_name, Cargo_names, Num_cargo, "cargo", 0);
			if (index == -1 && (Num_cargo < MAX_CARGO)) {
				index = Num_cargo;
				strcpy(Cargo_names[Num_cargo++], cargo_name);
			}
			Subsys_status[i].subsys_cargo_name = index;
		}

		if (optional_string("+AI Class:"))
			Subsys_status[i].ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");

		if (optional_string("+Primary Banks:"))
			stuff_int_list(Subsys_status[i].primary_banks, MAX_SHIP_PRIMARY_BANKS, WEAPON_LIST_TYPE);

		// Goober5000
		if (optional_string("+Pbank Ammo:"))
			stuff_int_list(Subsys_status[i].primary_ammo, MAX_SHIP_PRIMARY_BANKS, RAW_INTEGER_TYPE);

		if (optional_string("+Secondary Banks:"))
			stuff_int_list(Subsys_status[i].secondary_banks, MAX_SHIP_SECONDARY_BANKS, WEAPON_LIST_TYPE);

		if (optional_string("+Sbank Ammo:"))
			stuff_int_list(Subsys_status[i].secondary_ammo, MAX_SHIP_SECONDARY_BANKS, RAW_INTEGER_TYPE);

	}
}

extern int Multi_ping_timestamp;
void parse_objects(mission *pm, int flag)
{	
	Assert(pm != NULL);

	required_string("#Objects");	

	// parse in objects
	Num_parse_objects = 0;
	while (required_string_either("#Wings", "$Name:"))
	{
		// make sure we're not overflowing
		if (Num_parse_objects >= MAX_PARSE_OBJECTS)
		{
			skip_to_start_of_string("#Wings");
			break;
		}

		// parse a single object
		int valid = parse_object(pm, flag, &Parse_objects[Num_parse_objects]);

		// not all objects are always valid or legal
		if (!valid)
			continue;

		// increment objects parsed
		Num_parse_objects++;

		// send out a ping if we are multi so that psnet2 doesn't kill us off for a long load
		// NOTE that we can't use the timestamp*() functions here since they won't increment
		//      during this loading process
		if (Game_mode & GM_MULTIPLAYER)
		{
			if ((Multi_ping_timestamp == -1) || (Multi_ping_timestamp <= timer_get_milliseconds()))
			{
				multi_ping_send_all();
				Multi_ping_timestamp = timer_get_milliseconds() + 10000; // timeout is 10 seconds between pings
			}
		}
	}

	// Goober5000 - I moved the docking stuff to post_process_ships_wings because of interdependencies
	// between ships and wings.  Neither docking stuff nor ship stuff (for ships present at mission start)
	// will be valid until after post_process_ships_wings is run.

	// Karajorma - Now that we've parsed all the objects we can set the class of those which were flagged 
	// to be set based on the number of ships available in the loadout. 
	if (!Fred_running)
	{
		process_loadout_objects();
	}	
}

// Karajorma - Checks the list of Parse_objects to see if any of them should be reassigned based on the 
// number of ships of that class that were present in the loadout. 
void process_loadout_objects()
{
	int reassignments[MAX_PARSE_OBJECTS];
	int count = 0;
	
	// Loop through all the Parse_objects looking for ships that should be affected by the loadout code.
	for (int i=0; i < Num_parse_objects; i++)
	{
		p_object *pobj = &Parse_objects[i];
		if (pobj->flags2 & P2_SF2_SET_CLASS_DYNAMICALLY)
		{
			bool successful = attempt_assignation_from_alt_classes(pobj);

			if (!successful)
			{
				// We've cycled through all the alt classes and we still haven't found one that matches
				// So we store the ship so we can come back to it later.
				Assert (count < Num_parse_objects);
				reassignments[count++] = i;
			}
		}
	}
	
	// Now we go though the ships we were unable to assign earlier and reassign them on a first come first 
	// served basis.
	for (int m=0; m < count; m++)
	{
		// What is the ship class of the object and what team is it on? 
		p_object *p_obj = &Parse_objects[reassignments[m]];
		team_data *current_team = &Team_data[p_obj->team];
		bool loadout_assigned = false;
		Assert (p_obj->flags2 & P2_SF2_SET_CLASS_DYNAMICALLY);

		// First thing to check is whether we actually have any ships left to assign
		if (current_team->loadout_total == 0)
		{
			/* We'll do this code once the default ship code can be used to set a global default. Till then it's much better
			// to simply use the ship that the mission designer specified. 
			// Check that the team default ship and this ship aren't the same (this may be a different ship
			// from any of the alt ship classes). We don't need to do anything if it is the same. 
			if (p_obj->ship_class != current_team->default_ship)
			{
				swap_parse_object(p_obj, current_team->default_ship);
			}*/

			// If there is nothing left to assign we should use the ship in the mission file
			loadout_assigned = true;
		}
		// We do have ships left in the team loadout that we can assign
		else
		{
			// Go through the loadout until we find an unassigned ship
			for (int j=0; j < current_team->number_choices; j++)
			{
				if (current_team->ship_count[j] > 0)
				{
					assign_ship_from_loadout(p_obj, current_team, j);
					// We will need to assign a new class too (if a p_object the same class was available
					// it should have been assigned by attempt_loadout_assignation_from_defaults()
					Assert (p_obj->ship_class != current_team->ship_list[j]);
					swap_parse_object(p_obj, current_team->ship_list[j]);

					loadout_assigned = true;
					break ;
				}
			}
		}
			
		// We should never reach here with an unassigned loadout
		Assert (loadout_assigned);
	}
}

// Karajorma - There are three places the game looks when attemptsing to determine the ship class for team loadout. 
// The class specified in the mission file and the two alt ship classes. This function checks all three
// and reports false if it can't set the ship class from the values held in any of them. 
bool attempt_assignation_from_alt_classes(p_object *pobj)
{
	team_data *data_for_team = &Team_data[pobj->team];

	int loadout_index = SHIP_CURRENTLY_UNASSIGNED;

	// First lets check if the ship specified in the mission file is of an assignable class
	loadout_index = is_ship_available_from_loadout(data_for_team, pobj->ship_class);
	if ((loadout_index != SHIP_CLASS_NOT_IN_LOADOUT) && (loadout_index != SHIP_CLASS_ALL_ASSIGNED))
	{
		Assert (data_for_team->loadout_total > 0);
		assign_ship_from_loadout(pobj, data_for_team, loadout_index);
			
		// Since the ship in the mission file matched one available in the loadout we need go no further
		return true;
	}

	// Now check if we have any matches from the first set of alt classes. 
	if (pobj->num_alt_class_one > 0)
	{
		int default_ship_index = 0;
		while ((default_ship_index < pobj->num_alt_class_one) && 
			  ((loadout_index == SHIP_CLASS_NOT_IN_LOADOUT) ||(loadout_index == SHIP_CLASS_ALL_ASSIGNED))) 
		{
			loadout_index = is_ship_available_from_loadout(data_for_team, pobj->alt_class_one[default_ship_index]);
			default_ship_index++;
		}
	}
			
	if ((loadout_index != SHIP_CLASS_NOT_IN_LOADOUT) && (loadout_index != SHIP_CLASS_ALL_ASSIGNED))
	{
		// We've found a hit in the suggested_loadouts. Asssign it and swap the ships
		assign_ship_from_loadout(pobj, data_for_team, loadout_index);
		swap_parse_object(pobj, data_for_team->ship_list[loadout_index]);

		// Ship assigned.  
		return true; 
	}

	// If we still haven't successfully assigned the ship we'll attempt to assign it from the second set of alt 
	// classes. If the first entry in the array does contain a ship class we will set the p_object to that value so it
	// makes no sense to check anything other than the first entry
	if (pobj->num_alt_class_two > 0)
	{
		loadout_index = is_ship_available_from_loadout(data_for_team, pobj->alt_class_two[0]);
			
		if ((loadout_index != SHIP_CLASS_NOT_IN_LOADOUT) && (loadout_index != SHIP_CLASS_ALL_ASSIGNED))
		{
			// we don't touch the team loadout data at all but we do need to check if we need to 
			// swap ship class
			if (pobj->ship_class != pobj->alt_class_two[0])
			{
				swap_parse_object(pobj, pobj->alt_class_two[0]);
			}
					
			return true;
		}
		else 
		{
			assign_ship_from_loadout(pobj, data_for_team, loadout_index);
			
			// Do we actually need to change class?
			if (pobj->ship_class != pobj->alt_class_two[0])
			{
				swap_parse_object(pobj, pobj->alt_class_two[0]);
			}
			return true;
		}
	}
	// Ship couldn't be assigned to one of the defaults
	return false;
}

// Karajorma - Checks if any ships of a certain ship class are still available in the team loadout
// Returns the index of the ship in team_data->ship_list if found or SHIP_CLASS_ALL_ASSIGNED/SHIP_CLASS_NOT_IN_LOADOUT
int is_ship_available_from_loadout(team_data *current_team, int ship_class)
{
	// first find out if the ship is even in the loadout
	int index = is_ship_in_loadout(current_team, ship_class);

	if (index != SHIP_CLASS_NOT_IN_LOADOUT)
	{
		// Found the matching ship but are there any left? 
		if (current_team->ship_count[index] > 0)
		{
			return index;
		}
		else 
		{
			return SHIP_CLASS_ALL_ASSIGNED;
		}
	}

	return SHIP_CLASS_NOT_IN_LOADOUT;	
}

// Karajorma - Checks if a certain ship class is in the team loadout. Returns the team_data->ship_index if it is or 
// SHIP_CLASS_NOT_IN_LOADOUT if it isn't.
int is_ship_in_loadout(team_data *current_team, int ship_class)
{
	// Search through the available ships to see if there is a matching ship class in the loadout
	for (int i=0; i < current_team->number_choices; i++)
	{
		if (ship_class == current_team->ship_list[i])
		{
			return i;
		}
	}
	// If we get this far we haven't found a match
	return SHIP_CLASS_NOT_IN_LOADOUT;
}

// Karajorma - Updates the loadout quanities for a ship class.
void assign_ship_from_loadout(p_object *pobj, team_data *current_team, int new_ship_loadout_index)
{
	// Fix the loadout variables to show that the class has less available if there are still ships available
	if (current_team->ship_count[new_ship_loadout_index] > 0)
	{
		current_team->ship_count[new_ship_loadout_index]--;
		current_team->loadout_total--;
	}
}

// Karajorma - Replaces a p_object with a new one based on a Ship_info index.
void swap_parse_object(p_object *p_obj, int new_ship_class)
{
	ship_info *new_ship_info = &Ship_info[new_ship_class];
	ship_info *old_ship_info = &Ship_info[p_obj->ship_class];
	int subsys_ind = p_obj->subsys_index;
	subsys_status *ship_subsystems = &Subsys_status[subsys_ind];

	// Class
	// First things first. Change the class of the p_object
	p_obj->ship_class = new_ship_class;

	// Hitpoints
	// We need to take into account that the ship might have been assigned special hitpoints so we can't 
	// simply swap old for new. 
	Assert (p_obj->parse_max_hull_strength > 0);
	Assert (old_ship_info->max_hull_strength > 0);
	
	//WMC - there was some oddness going on with fl2i and i2fl, even though
	//all of these are floats
	float hp_multiplier = p_obj->parse_max_hull_strength / old_ship_info->max_hull_strength;
	p_obj->parse_max_hull_strength = new_ship_info->max_hull_strength * hp_multiplier;


	// Shields
	// Again we have to watch out for special hitpoints but this time we can't assume that there will be a 
	// shield. So first lets see if there is one. 
	if ((p_obj->parse_max_shield_strength != old_ship_info->max_shield_strength) && 
		(p_obj->parse_max_shield_strength > 0) &&
		(new_ship_info->max_shield_strength > 0))
	{
		// This ship is using special hitpoints to alter the shield strength
		float shield_multiplier = p_obj->parse_max_shield_strength / i2fl(old_ship_info->max_shield_strength);
		p_obj->parse_max_shield_strength = new_ship_info->max_shield_strength * shield_multiplier;
	}
	// Not using special hitpoints or a class which has a shield strength of zero
	else
	{
		p_obj->parse_max_shield_strength = new_ship_info->max_shield_strength;
	}
	
	// Primary weapons
	// First find out what is the correct number for a ship of this class
	int num_pbanks = new_ship_info->num_primary_banks;
	// Now cycle through the primary banks looking for banks that were added or removed
	for (int i=0; i < MAX_SHIP_PRIMARY_BANKS; i++)
	{
		// If we're dealing with a primary bank that actually should exist on this ship
		if ( i < num_pbanks )
		{
			// We only care if a weapon hasn't been parsed in for this bank
			if (ship_subsystems->primary_banks[i] == -1)
			{
				// Give the ship the default weapon for this bank. 
				ship_subsystems->primary_banks[i] = new_ship_info->primary_bank_weapons[i];
			}

		}		
		// Any primary banks the ship doesn't have should be set to -1
		else
		{
			ship_subsystems->primary_banks[i] = -1;
		}
	}

	// Secondary weapons 
	// Again we first have to find out how many we should have
	int num_sbanks = new_ship_info->num_secondary_banks;
	// Now cycle through the secondary banks looking for banks that were added or removed
	for (int j=0; j < MAX_SHIP_SECONDARY_BANKS; j++)
	{
		// If we're dealing with a primary bank that actually should exist on this ship
		if ( j < num_sbanks )
		{
			// We only care if a weapon hasn't been parsed in for this bank
			if (ship_subsystems->secondary_banks[j] == -1){
				// Give the ship the default weapon for this bank. 
				ship_subsystems->secondary_banks[j] = new_ship_info->secondary_bank_weapons[j];
			}

		}		
		// Any secondary banks the ship doesn't have should be set to -1
		else
		{
			ship_subsystems->secondary_banks[j] = -1;
		}
	}
}


p_object *mission_parse_get_parse_object(ushort net_signature)
{
	int i;

	// look for original ships
	for (i = 0; i < Num_parse_objects; i++)
		if(Parse_objects[i].net_signature == net_signature)
			return &Parse_objects[i];

	// boo
	return NULL;
}

// Goober5000 - also get it by name
p_object *mission_parse_get_parse_object(char *name)
{
	int i;

	// look for original ships
	for (i = 0; i < Num_parse_objects; i++)
		if(!stricmp(Parse_objects[i].name, name))
			return &Parse_objects[i];

	// boo
	return NULL;
}

int find_wing_name(char *name)
{
	int	i;

	for (i = 0; i < Num_wings; i++)
	{
		if (!stricmp(name, Wings[i].name))
			return i;
	}

	return -1;
}

// function to create ships in the wing that need to be created
int parse_wing_create_ships( wing *wingp, int num_to_create, int force, int specific_instance )
{
	int wingnum, objnum, num_create_save;
	int time_to_arrive;
	int pre_create_count;
	int i, j;

	// we need to send this in multiplayer
	pre_create_count = wingp->total_arrived_count;

	// force is used to force creation of the wing -- used for multiplayer
	if ( !force ) {
		// we only want to evaluate the arrival cue of the wing if:
		// 1) single player
		// 2) multiplayer and I am the host of the game
		// can't create any ships if the arrival cue is false or the timestamp has not elapsed.

		if ( !eval_sexp(wingp->arrival_cue) ) /* || !timestamp_elapsed(wingp->arrival_delay) ) */
			return 0;

		// once the sexpressions becomes true, then check the arrival delay on the wing.  The first time, the
		// arrival delay will be <= 0 meaning that no timer object has been set yet.  Set up the timestamp
		// which should always give a number >= 0;
		if ( wingp->arrival_delay <= 0 ) {
			wingp->arrival_delay = timestamp( -wingp->arrival_delay * 1000 );
			Assert ( wingp->arrival_delay >= 0 );
		}

		if ( !timestamp_elapsed( wingp->arrival_delay ) )
			return 0;

		// if wing is coming from docking bay, then be sure that ship we are arriving from actually exists
		// (or will exist).
		if ( wingp->arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			int shipnum;
			char *name;

			Assert( wingp->arrival_anchor >= 0 );
			name = Parse_names[wingp->arrival_anchor];

			// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
			if ( mission_parse_get_arrival_ship( name ) )
				return 0;

			// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
			// it is not on the arrival list (as shown by above if statement).
			shipnum = ship_name_lookup( name );
			if ( shipnum == -1 ) {
				int num_remaining;
				// since this wing cannot arrive from this place, we need to mark the wing as destroyed and
				// set the wing variables appropriatly.  Good for directives.

				// set the gone flag
				wingp->flags |= WF_WING_GONE;

				// if the current wave is zero, it never existed
				wingp->flags |= WF_NEVER_EXISTED;

				// mark the number of waves and number of ships destroyed equal to the last wave and the number
				// of ships yet to arrive
				num_remaining = ( (wingp->num_waves - wingp->current_wave) * wingp->wave_count);
				wingp->total_arrived_count += num_remaining;
				wingp->current_wave = wingp->num_waves;

				// replaced following three lines of code with mission log call because of bug with
				// the Ships_exited list.
				//index = ship_find_exited_ship_by_name( name );
				//Assert( index != -1 );
				//if (Ships_exited[index].flags & SEF_DESTROYED ) {
				if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) ) {
					wingp->total_destroyed += num_remaining;
				} else {
					wingp->total_departed += num_remaining;
				}

				mission_parse_mark_non_arrival(wingp);	// Goober5000
				return 0;
			}

			// Goober5000 - check status of fighterbays - if they're destroyed, we can't launch - but we want to reeval later
			if (ship_fighterbays_all_destroyed(&Ships[shipnum]))
				return 0;
		}

		if ( num_to_create == 0 )
			return 0;

		// check the wave_delay_timestamp field.  If it is not valid, make it valid (based on wave delay min
		// and max values).  If it is valid, and not elapsed, then return.  If it is valid and elasped, then
		// continue on.
		if ( !timestamp_valid(wingp->wave_delay_timestamp) ) {

			// if at least one of these is valid, then reset the timestamp.  If they are both zero, we will create the
			// wave
			if ( (wingp->wave_delay_min > 0) || (wingp->wave_delay_max > 0) ) {
				Assert ( wingp->wave_delay_min <= wingp->wave_delay_max );
				time_to_arrive = wingp->wave_delay_min + (int)(frand() * (wingp->wave_delay_max - wingp->wave_delay_min));

				// MWA -- 5/18/98
				// HACK HACK -- in the presense of Mike Comet and Mitri, I have introduced one of the most
				// serious breaches of coding standards.  I'm to lazy to fix this the correct way.  Insert
				// a delay before the next wave of the wing can arrive to that clients in the game have ample
				// time to kill off any ships in the wing before the next wave arrives.
				if ( Game_mode & GM_MULTIPLAYER ){
					time_to_arrive += 7;
				}
				wingp->wave_delay_timestamp = timestamp(time_to_arrive * 1000);
				return 0;
			}

			// if we get here, both min and max values are 0;  See comments above for a most serious hack
			time_to_arrive = 0;
			if ( Game_mode & GM_MULTIPLAYER )
				time_to_arrive += 7;
			time_to_arrive *= 1000;
			wingp->wave_delay_timestamp = timestamp(time_to_arrive);
		}

		// now check to see if the wave_delay_timestamp is elapsed or not
		if ( !timestamp_elapsed(wingp->wave_delay_timestamp) )
			return 0;
	}

	// finally we can create the wing.

	num_create_save = num_to_create;

	wingnum = WING_INDEX(wingp);					// get the wing number

	// if there are no ships to create, then all ships must be player start ships -- do nothing in this case.
	if ( num_to_create == 0 ){
		return 0;
	}

	wingp->current_wave++;						// we are creating new ships
	// we need to create num_to_create ships.  Since the arrival cues for ships in a wing
	// are ignored, then *all* ships must be in the Ship_arrival_list.

	objnum = -1;

	// Goober5000 - we have to do this via the array because we have no guarantee we'll be able to iterate along the list
	// (since created objects plus anything they're docked to will be removed from it)
	for (i = 0; i < Num_parse_objects; i++)
	{
		int index;
		ai_info *aip;
		p_object *p_objp = &Parse_objects[i];

		// ensure on arrival list
		if (!parse_object_on_arrival_list(p_objp))
			continue;

		// compare the wingnums.  When they are equal, we can create the ship.  In the case of
		// wings that have multiple waves, this code implies that we essentially creating clones
		// of the ships that were created in Fred for the wing when more ships for a new wave
		// arrive.  The threshold value of a wing can also make one of the ships in a wing be "cloned"
		// more often than other ships in the wing.  I don't think this matters much.
		if (p_objp->wingnum != wingnum)
			continue;

		// when ingame joining, we need to create a specific ship out of the list of ships for a
		// wing.  specific_instance is a 0 based integer which specified which ship in the wing
		// to create.  So, only create the ship we actually need to.
		if ((Game_mode & GM_MULTIPLAYER) && (specific_instance > 0))
		{
			specific_instance--;
			continue;
		}

		Assert (!(p_objp->flags & P_SF_CANNOT_ARRIVE));		// get allender

		// if we have the maximum number of ships in the wing, we must bail as well
		if (wingp->current_count >= MAX_SHIPS_PER_WING)
		{
			Int3();					// this is bogus -- we should always allow all ships to be created
			num_to_create = 0;
			break;
		}

		// bash the ship name to be the name of the wing + some number if there is > 1 wave in this wing

		// also, if multiplayer, set the parse object's net signature to be wing's net signature
		// base + total_arrived_count (before adding 1)
		if (Game_mode & GM_MULTIPLAYER)
		{
			p_objp->net_signature = (ushort) (wingp->net_signature + wingp->total_arrived_count);
		}

		wingp->total_arrived_count++;
		if (wingp->num_waves > 1)
		{
			sprintf(p_objp->name, NOX("%s %d"), wingp->name, wingp->total_arrived_count);
		}


		objnum = parse_create_object(p_objp);
		aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];

		// copy any goals from the wing to the newly created ship
		for (index = 0; index < MAX_AI_GOALS; index++)
		{
			if (wingp->ai_goals[index].ai_mode != AI_GOAL_NONE)
				ai_copy_mission_wing_goal(&wingp->ai_goals[index], aip);
		}

		Ai_info[Ships[Objects[objnum].instance].ai_index].wing = wingnum;

		if (wingp->flags & WF_NO_DYNAMIC)
			aip->ai_flags |= AIF_NO_DYNAMIC;

		// update housekeeping variables
		wingp->ship_index[wingp->current_count] = Objects[objnum].instance;

		// set up wingman status index
		hud_wingman_status_set_index(wingp->ship_index[wingp->current_count]);

		p_objp->wing_status_wing_index = Ships[Objects[objnum].instance].wing_status_wing_index;
		p_objp->wing_status_wing_pos = Ships[Objects[objnum].instance].wing_status_wing_pos;

		wingp->current_count++;

		// keep any player ship on the parse object list -- used for respawns
		// 5/8/98 -- MWA -- don't remove ships from the list when you are ingame joining
		if (!(p_objp->flags & P_OF_PLAYER_START))
		{
			if ((Game_mode & GM_NORMAL) || !(Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
			{
				// only remove ship if one wave in wing
				if (wingp->num_waves == wingp->current_wave)
				{
					// remove p_objp from the list
					list_remove(&Ship_arrival_list, p_objp);
					
					// free up sexp nodes for reuse
					if (p_objp->ai_goals != -1)
						free_sexp2(p_objp->ai_goals);
				}
			}
		}

		// flag ship with SF_FROM_PLAYER_WING if a member of player starting wings
		if ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
		{
			// different for tvt -- Goober5000
			for (j = 0; j < MAX_TVT_WINGS; j++)
			{
				if (!stricmp(TVT_wing_names[j], wingp->name))
					Ships[Objects[objnum].instance].flags |= SF_FROM_PLAYER_WING;
			}
		}
		else
		{
			for (j = 0; j < MAX_STARTING_WINGS; j++)
			{
				if (!stricmp(Starting_wing_names[j], wingp->name))
					Ships[Objects[objnum].instance].flags |= SF_FROM_PLAYER_WING;
			}
		}

		// keep track of how many ships to create.  Stop when we have done all that we are supposed to do.
		num_to_create--;
		if (num_to_create == 0)
			break;
	}

	// we should always have enough ships in the list!!!
	Assert (num_to_create == 0);

	// possibly play some event driven music here.  Send a network packet indicating the wing was
	// created.  Only do this stuff if actually in the mission.
	if ( (objnum != -1) && (Game_mode & GM_IN_MISSION) ) {		// if true, we have created at least one new ship.
		int ship_num;

		// Goober5000 - make all wings form on their respective leaders
		ai_maybe_add_form_goal( wingp );

/*
		// see if this wing is a player starting wing, and if so, call the maybe_add_form_goal
		// function to possibly make the wing form on its leader
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			if ( Starting_wings[i] == wingnum ){
				break;
			}
		}
		if ( i < MAX_STARTING_WINGS ){
			ai_maybe_add_form_goal( wingp );
		}
*/

		mission_log_add_entry( LOG_WING_ARRIVED, wingp->name, NULL, wingp->current_wave );
		ship_num = wingp->ship_index[0];

		if ( !(Ships[ship_num].flags & SF_NO_ARRIVAL_MUSIC) && !(wingp->flags & WF_NO_ARRIVAL_MUSIC) ) {
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[ship_num].team);	
			}
		}

		// possibly change the location where these ships arrive based on the wings arrival location
		mission_set_wing_arrival_location( wingp, num_create_save );

		// if in multiplayer (and I am the host) and in the mission, send a wing create command to all
		// other players
		if ( MULTIPLAYER_MASTER ){
			send_wing_create_packet( wingp, num_create_save, pre_create_count );
		}

#ifndef NDEBUG
		// test code to check to be sure that all ships in the wing are ignoring the same types
		// of orders from the leader
		if ( Fred_running ) {
			Assert( wingp->ship_index[wingp->special_ship] != -1 );
			int orders = Ships[wingp->ship_index[0]].orders_accepted;
			for (i = 0; i < wingp->current_count; i++ ) {
				if (i == wingp->special_ship)
					continue;

				if ( orders != Ships[wingp->ship_index[i]].orders_accepted ) {
					Warning(LOCATION, "ships in wing %s are ignoring different player orders.  Please find Mark A\nto talk to him about this.", wingp->name );
					break;
				}
			}
		}
#endif

	}

	wingp->wave_delay_timestamp = timestamp(-1);		// we will need to set this up properly for the next wave
	return num_create_save;
}

void parse_wing(mission *pm)
{
	int wingnum, i, wing_goals, delay;
	char name[NAME_LENGTH], ship_names[MAX_SHIPS_PER_WING][NAME_LENGTH];
	char wing_flag_strings[MAX_WING_FLAGS][NAME_LENGTH];
	wing *wingp;

	Assert(pm != NULL);
	wingp = &Wings[Num_wings];

	required_string("$Name:");
	stuff_string(wingp->name, F_NAME, NAME_LENGTH);
	wingnum = find_wing_name(wingp->name);
	if (wingnum != -1)
		error_display(0, NOX("Redundant wing name: %s\n"), wingp->name);
	wingnum = Num_wings;

	wingp->total_arrived_count = 0;
	wingp->total_destroyed = 0;
	wingp->total_departed = 0;	// Goober5000
	wingp->flags = 0;

	// squad logo - Goober5000
	if (optional_string("+Squad Logo:"))
	{
		int flag = -1;

		stuff_string(wingp->wing_squad_filename, F_NAME, MAX_FILENAME_LEN);

		// load it only if FRED isn't running
		if (!Fred_running)
		{
			// check all previous wings to see if we already loaded it (we want to save memory)
			for (i = 0; i < Num_wings; i++)
			{
				// do we have a previous texture?
				if (Wings[i].wing_insignia_texture != -1)
				{
					// if we have a match
					if (!stricmp(Wings[i].wing_squad_filename, wingp->wing_squad_filename))
					{
						flag = i;
						break;
					}
				}
			}

			// if we have loaded it already, just use the old bitmap index
			if (flag != -1)
			{
				wingp->wing_insignia_texture = Wings[flag].wing_insignia_texture;
			}
			else
			{
				wing_load_squad_bitmap(wingp);
			}
		}
	}

	required_string("$Waves:");
	stuff_int(&wingp->num_waves);
	Assert ( wingp->num_waves >= 1 );		// there must be at least 1 wave

	wingp->current_wave = 0;

	required_string("$Wave Threshold:");
	stuff_int(&wingp->threshold);

	required_string("$Special Ship:");
	stuff_int(&wingp->special_ship);

	wingp->arrival_anchor = -1;
	wingp->arrival_distance = 0;
	wingp->arrival_path_mask = -1;	// -1 only until resolved

	find_and_stuff("$Arrival Location:", &wingp->arrival_location, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");

	if ( optional_string("+Arrival Distance:") )
	{
		stuff_int( &wingp->arrival_distance );

		// Goober5000
		if ((wingp->arrival_distance <= 0) && ((wingp->arrival_location == ARRIVE_NEAR_SHIP) || (wingp->arrival_location == ARRIVE_IN_FRONT_OF_SHIP)))
		{
			Warning(LOCATION, "Arrival distance for wing %s cannot be %d.  Setting to 1.\n", wingp->name, wingp->arrival_distance);
			wingp->arrival_distance = 1;
		}
	}

	if ( wingp->arrival_location != ARRIVE_AT_LOCATION )
	{
		required_string("$Arrival Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		wingp->arrival_anchor = get_anchor(name);
	}

	if (optional_string("+Arrival Paths:"))
	{
		// temporarily use mask to point to the restriction index
		wingp->arrival_path_mask = add_path_restriction();
	}

	if (optional_string("+Arrival delay:")) {
		stuff_int(&delay);
		if ( delay < 0 )
			Error(LOCATION, "Cannot have arrival delay < 0 on wing %s", wingp->name );
	} else
		delay = 0;

	if ( !Fred_running ){
		wingp->arrival_delay = -delay;
	} else {
		wingp->arrival_delay = delay;
	}

	required_string("$Arrival Cue:");
	wingp->arrival_cue = get_sexp_main();
	if ( !Fred_running && (wingp->arrival_cue >= 0) ) {
		if ( eval_sexp(wingp->arrival_cue) )			// evaluate to determine if sexp is always false.
			wingp->arrival_delay = timestamp( -wingp->arrival_delay * 1000 );
	}

	
	wingp->departure_anchor = -1;
	wingp->departure_path_mask = -1;	// -1 only until resolved

	find_and_stuff("$Departure Location:", &wingp->departure_location, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");

	if ( wingp->departure_location != DEPART_AT_LOCATION )
	{
		required_string("$Departure Anchor:");
		stuff_string( name, F_NAME, NAME_LENGTH );
		wingp->departure_anchor = get_anchor(name);
	}

	if (optional_string("+Departure Paths:"))
	{
		// temporarily use mask to point to the restriction index
		wingp->departure_path_mask = add_path_restriction();
	}

	if (optional_string("+Departure delay:")) {
		stuff_int(&delay);
		if ( delay < 0 )
			Error(LOCATION, "Cannot have departure delay < 0 on wing %s", wingp->name );
	} else
		delay = 0;


	if ( !Fred_running )
		wingp->departure_delay = -delay;		// use negative numbers to mean that delay timer not yet set
	else
		wingp->departure_delay = delay;

	required_string("$Departure Cue:");
	wingp->departure_cue = get_sexp_main();

	// stores a list of all names of ships in the wing
	required_string("$Ships:");
	wingp->wave_count = stuff_string_list( ship_names, MAX_SHIPS_PER_WING );
	wingp->current_count = 0;

	// get the wings goals, if any
	wing_goals = -1;
	if ( optional_string("$AI Goals:") )
		wing_goals = get_sexp_main();

	wingp->hotkey = -1;
	if (optional_string("+Hotkey:")) {
		stuff_int(&wingp->hotkey);
		Assert((wingp->hotkey >= 0) && (wingp->hotkey < 10));
	}

	if (optional_string("+Flags:")) {
		int count;

		count = stuff_string_list( wing_flag_strings, MAX_WING_FLAGS );
		for (i = 0; i < count; i++ ) {
			if ( !stricmp( wing_flag_strings[i], NOX("ignore-count")) )
				wingp->flags |= WF_IGNORE_COUNT;
			else if ( !stricmp( wing_flag_strings[i], NOX("reinforcement")) )
				wingp->flags |= WF_REINFORCEMENT;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-music")) )
				wingp->flags |= WF_NO_ARRIVAL_MUSIC;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-message")) )
				wingp->flags |= WF_NO_ARRIVAL_MESSAGE;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-warp")) )
				wingp->flags |= WF_NO_ARRIVAL_WARP;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-departure-warp")) )
				wingp->flags |= WF_NO_DEPARTURE_WARP;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-dynamic")) )
				wingp->flags |= WF_NO_DYNAMIC;
			else if ( !stricmp( wing_flag_strings[i], NOX("nav-carry-status")) )
				wingp->flags |= WF_NAV_CARRY;
			else
				Warning(LOCATION, "unknown wing flag\n%s\n\nSkipping.", wing_flag_strings[i]);
		}
	}

	// get the wave arrival delay bounds (if present).  Used as lower and upper bounds (in seconds)
	// which determine when new waves of a wing should arrive.
	wingp->wave_delay_min = 0;
	wingp->wave_delay_max = 0;
	if ( optional_string("+Wave Delay Min:") )
		stuff_int( &(wingp->wave_delay_min) );
	if ( optional_string("+Wave Delay Max:") )
		stuff_int( &(wingp->wave_delay_max) );

	// be sure to set the wave arrival timestamp of this wing to pop right away so that the
	// wing could be created if it needs to be
	wingp->wave_delay_timestamp = timestamp(0);

	// initialize wing goals
	for (i=0; i<MAX_AI_GOALS; i++) {
		wingp->ai_goals[i].ai_mode = AI_GOAL_NONE;
		wingp->ai_goals[i].signature = -1;
		wingp->ai_goals[i].priority = -1;
		wingp->ai_goals[i].flags = 0;
	}

	// 7/13/98 -- MWA
	// error checking against the player ship wings (i.e. starting & tvt) to be sure that wave count doesn't exceed one for
	// these wings.
	if ( Game_mode & GM_MULTIPLAYER ) {
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			if ( !stricmp(Starting_wing_names[i], wingp->name) ) {
				if ( wingp->num_waves > 1 ) {
					// only end the game if we're the server - clients will eventually find out :)
					if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
						multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_WAVE_COUNT);																
					}
					// Error(LOCATION, "Player wings cannot have more than 1 wave.");
				}
			}
		}
		for (i = 0; i < MAX_TVT_WINGS; i++ ) {
			if ( !stricmp(TVT_wing_names[i], wingp->name) ) {
				if ( wingp->num_waves > 1 ) {
					// only end the game if we're the server - clients will eventually find out :)
					if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
						multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_WAVE_COUNT);																
					}
					// Error(LOCATION, "Player wings cannot have more than 1 wave.");
				}
			}
		}
	}

	// Get the next starting signature for this in this wing.  We want to reserve wave_count * num_waves
	// of signature.  These can be used to construct wings for ingame joiners.
	if ( Game_mode & GM_MULTIPLAYER ) {
		int next_signature;

		wingp->net_signature = multi_assign_network_signature( MULTI_SIG_SHIP );
		next_signature = wingp->net_signature + (wingp->wave_count * wingp->num_waves);
		if ( next_signature > SHIP_SIG_MAX )
			Error(LOCATION, "Too many total ships in mission (%d) for network signature assignment", SHIP_SIG_MAX);
		multi_set_network_signature( (ushort)next_signature, MULTI_SIG_SHIP );
	}

	for (i=0; i<MAX_SHIPS_PER_WING; i++)
		wingp->ship_index[i] = -1;

	// set up the ai_goals for this wing -- all ships created from this wing will inherit these goals
	// goals for the wing are stored slightly differently than for ships.  We simply store the index
	// into the sexpression array of each goal (max 10).  When a ship in this wing is created, each
	// goal in the wings goal array is given to the ship.
	if ( wing_goals != -1 ) {
		int sexp, index;

		// this will assign the goals to the wings as well as to any ships in the wing that have been
		// already created.
		index = 0;
		for ( sexp = CDR(wing_goals); sexp != -1; sexp = CDR(sexp) )
			ai_add_wing_goal_sexp(sexp, AIG_TYPE_EVENT_WING, wingnum);  // used by Fred

		//if (Fred_running)
			free_sexp2(wing_goals);  // free up sexp nodes for reuse, since they aren't needed anymore.
	}

	// set the wing number for all ships in the wing
	for (i = 0; i < wingp->wave_count; i++ )
	{
		char *ship_name = ship_names[i];
		int j, assigned = 0;

		// Goober5000 - since the ship/wing creation stuff is reordered to accommodate multiple docking,
		// everything is still only in the parse array at this point (in both FRED and FS2)

		// find the parse object and assign it the wing number
		for (j = 0; j < Num_parse_objects; j++)
		{
			p_object *p_objp = &Parse_objects[j];

			if (!strcmp(ship_name, p_objp->name))
			{
				// get Allender -- ship appears to be in multiple wings
				Assert (p_objp->wingnum == -1);

				p_objp->wingnum = wingnum;
				p_objp->pos_in_wing = i;

				assigned++;
			}
		}

		// error checking
		if (assigned == 0)
			Error(LOCATION, "Cannot load mission -- for wing %s, ship %s is not present in #Objects section.\n", wingp->name, ship_name);
		else if (assigned > 1)
			Error(LOCATION, "Cannot load mission -- for wing %s, ship %s is specified multiple times in wing.\n", wingp->name, ship_name);
	}

	// Goober5000 - wing creation stuff moved to post_process_ships_wings
}

void parse_wings(mission *pm)
{
	required_string("#Wings");
	while (required_string_either("#Events", "$Name:"))
	{
		Assert(Num_wings < MAX_WINGS);
		parse_wing(pm);
		Num_wings++;
	}
}

// Goober5000
void resolve_path_masks(int anchor, int *path_mask)
{
	path_restriction_t *prp;

	// if we have no restrictions, do a quick out
	if (*path_mask < 0)
	{
		*path_mask = 0;
		return;
	}

	// get path restriction info
	prp = &Path_restrictions[*path_mask];

	// uninitialized; compute the mask from scratch
	if (prp->cached_mask & (1 << MAX_SHIP_BAY_PATHS))
	{
		int j, bay_path, modelnum;
		p_object *parent_pobjp;

		// get anchor ship
		Assert(!(anchor & SPECIAL_ARRIVAL_ANCHOR_FLAG));
		parent_pobjp = mission_parse_get_parse_object(Parse_names[anchor]);

		// load model for checking paths
		modelnum = model_load(Ship_info[parent_pobjp->ship_class].pof_file, 0, NULL);

		// resolve names to indexes
		*path_mask = 0;
		for	(j = 0; j < prp->num_paths; j++)
		{
			bay_path = model_find_bay_path(modelnum, prp->path_names[j]);
			if (bay_path < 0)
				continue;

			*path_mask |= (1 << bay_path);
		}

		// unload model
		model_unload(modelnum);

		// cache the result
		prp->cached_mask = *path_mask;
	}
	// already computed; so reuse it
	else
	{
		*path_mask = prp->cached_mask;
	}
}

// Goober5000
// resolve arrival/departure path masks
// NB: between parsing and the time this function is run, the path_mask variables store the index of the path info;
// at all other times, they store the masks of the bay paths as expected
void post_process_path_stuff()
{
	int i;
	p_object *pobjp;
	wing *wingp;

	// take care of parse objects (ships)
	for (i = 0; i < Num_parse_objects; i++)
	{
		pobjp = &Parse_objects[i];

		resolve_path_masks(pobjp->arrival_anchor, &pobjp->arrival_path_mask);
		resolve_path_masks(pobjp->departure_anchor, &pobjp->departure_path_mask);
	}

	// take care of wings
	for (i = 0; i < Num_wings; i++)
	{
		wingp = &Wings[i];

		resolve_path_masks(wingp->arrival_anchor, &wingp->arrival_path_mask);
		resolve_path_masks(wingp->departure_anchor, &wingp->departure_path_mask);
	}
}

// Goober5000
void post_process_ships_wings()
{
	int i;

	// Goober5000 - first, resolve the path masks.  Needs to be done first because
	// mission_parse_maybe_create_parse_object relies on it.
	post_process_path_stuff();

	// Goober5000 - now that we've parsed all the objects, resolve the initially docked references.
	// This must be done before anything that relies on the dock references but can't be done until
	// both ships and wings have been parsed.
	mission_parse_set_up_initial_docks();

	// Goober5000 - now create all objects that we can.  This must be done before any ship stuff
	// but can't be done until the dock references are resolved.  This was originally done
	// in parse_object().
	for (i = 0; i < Num_parse_objects; i++)
	{
		mission_parse_maybe_create_parse_object(&Parse_objects[i]);
	}


	// ----------------- at this point the ships have been created -----------------
	// Now set up the wings.  This must be done after both dock stuff and ship stuff.


	// Goober5000 - for FRED, the ships are initialized after the wings, so we must now tell the wings
	// where their ships are
	if (Fred_running)
	{
		// even though the ships are already created, only the parse objects know the wing info
		for (i = 0; i < Num_parse_objects; i++)
		{
			p_object *p_objp = &Parse_objects[i];

			// link the ship into the wing's array of ship indices (previously done in parse_wing
			// in a rather less backwards way)
			if (p_objp->wingnum >= 0)
			{
				int shipnum = ship_name_lookup(p_objp->name);

				Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
				Assert(p_objp->pos_in_wing >= 0 && p_objp->pos_in_wing < MAX_SHIPS_PER_WING);

				Wings[p_objp->wingnum].ship_index[p_objp->pos_in_wing] = shipnum;
			}
		}
	}
	// Goober5000 - for FS2, the wings are initialized first, so all we have to do is create their ships
	else
	{
		for (i = 0; i < Num_wings; i++)
		{
			wing *wingp = &Wings[i];

			// create the wing if is isn't a reinforcement.
			if (!(wingp->flags & WF_REINFORCEMENT))
				parse_wing_create_ships(wingp, wingp->wave_count);
		}
	}
}

// mission events are sexpressions which cause things to happen based on the outcome
// of other events in a mission.  Essentially scripting the different things that can happen
// in a mission

void parse_event(mission *pm)
{
	char buf[NAME_LENGTH];
	mission_event *event;

	event = &Mission_events[Num_mission_events];
	event->chain_delay = -1;

	required_string( "$Formula:" );
	event->formula = get_sexp_main();

	if (optional_string("+Name:")){
		stuff_string(event->name, F_NAME, NAME_LENGTH);
	} else {
		event->name[0] = 0;
	}

	if ( optional_string("+Repeat Count:")){
		stuff_int( &(event->repeat_count) );
	} else {
		event->repeat_count = 1;
	}

	event->interval = -1;
	if ( optional_string("+Interval:")){
		stuff_int( &(event->interval) );
	}

	event->score = 0;
	if ( optional_string("+Score:") ){
		stuff_int(&event->score);
	}

	if ( optional_string("+Chained:") ){
		stuff_int(&event->chain_delay);
	}

	if ( optional_string("+Objective:") ) {
		stuff_string(buf, F_NAME, NAME_LENGTH);
		event->objective_text = vm_strdup(buf);
	} else {
		event->objective_text = NULL;
	}

	if ( optional_string("+Objective key:") ) {
		stuff_string(buf, F_NAME, NAME_LENGTH);
		event->objective_key_text = vm_strdup(buf);
	} else {
		event->objective_key_text = NULL;
	}

	event->team = -1;
	if( optional_string("+Team:") ) {
		stuff_int(&event->team);
	}

	event->timestamp = timestamp(-1);

	// sanity check on the repeat count variable
	// _argv[-1] - negative repeat count is now legal; means repeat indefinitely.
	if ( event->repeat_count == 0 ){
		Error (LOCATION, "Repeat count for mission event %s is 0.\nMust be >= 1 or negative!", event->name );
	}
}

void parse_events(mission *pm)
{
	required_string("#Events");

	while (required_string_either( "#Goals", "$Formula:")) {
		Assert( Num_mission_events < MAX_MISSION_EVENTS );
		parse_event(pm);
		Num_mission_events++;
	}
}

void parse_goal(mission *pm)
{
	int dummy;

	mission_goal	*goalp;

	goalp = &Mission_goals[Num_goals++];

	Assert(Num_goals < MAX_GOALS);
	Assert(pm != NULL);

	find_and_stuff("$Type:", &goalp->type, F_NAME, Goal_type_names, Num_goal_type_names, "goal type");

	required_string("+Name:");
	stuff_string(goalp->name, F_NAME, NAME_LENGTH);

	// backwards compatibility for old Fred missions - all new missions should use $MessageNew
	if(optional_string("$Message:")){
		stuff_string(goalp->message, F_NAME, MAX_GOAL_TEXT);
	} else {
		required_string("$MessageNew:");
		stuff_string(goalp->message, F_MULTITEXT, MAX_GOAL_TEXT);
	}

	if (optional_string("$Rating:")){
		stuff_int(&dummy);  // not used
	}

	required_string("$Formula:");
	goalp->formula = get_sexp_main();

	goalp->flags = 0;
	if ( optional_string("+Invalid:") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+Invalid") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+No music") )
		goalp->flags |= MGF_NO_MUSIC;

	goalp->score = 0;
	if ( optional_string("+Score:") ){
		stuff_int(&goalp->score);
	}

	goalp->team = 0;
	if ( optional_string("+Team:") ){
		stuff_int( &goalp->team );
	}
}

void parse_goals(mission *pm)
{
	required_string("#Goals");

	while (required_string_either("#Waypoints", "$Type:")){
		parse_goal(pm);
	}
}

void parse_waypoint_list(mission *pm)
{
	waypoint_list	*wpl;


	Assert(Num_waypoint_lists < MAX_WAYPOINT_LISTS);
	Assert(pm != NULL);
	wpl = &Waypoint_lists[Num_waypoint_lists];

	required_string("$Name:");
	stuff_string(wpl->name, F_NAME, NAME_LENGTH);

	required_string("$List:");
	wpl->count = stuff_vector_list(wpl->waypoints, MAX_WAYPOINTS_PER_LIST);

	Num_waypoint_lists++;
}

void parse_waypoints(mission *pm)
{
	vec3d pos;

	required_string("#Waypoints");

	jump_node *jnp;
	char file_name[MAX_FILENAME_LEN] = { 0 };

	while (optional_string("$Jump Node:")) {
		stuff_vector(&pos);
		jnp = new jump_node(&pos);
		Assert(jnp != NULL);

		if (optional_string("$Jump Node Name:") || optional_string("+Jump Node Name:")) {
			stuff_string(jnp->get_name_ptr(), F_NAME, NAME_LENGTH);
		}

		if(optional_string("+Model File:")){
			stuff_string(file_name, F_NAME, MAX_FILENAME_LEN);
			jnp->set_model(file_name);
		}

		if(optional_string("+Alphacolor:")) {
			ubyte r,g,b,a;
			stuff_ubyte(&r);
			stuff_ubyte(&g);
			stuff_ubyte(&b);
			stuff_ubyte(&a);
			jnp->set_alphacolor(r, g, b, a);
		}

		if(optional_string("+Hidden:")) {
			int hide;
			stuff_boolean(&hide);
			jnp->show(!hide);
		}
	}

	while (required_string_either("#Messages", "$Name:"))
		parse_waypoint_list(pm);
}

void parse_messages(mission *pm)
{
	required_string("#Messages");

	// command stuff by Goober5000 ---------------------------------------
	strcpy(pm->command_sender, DEFAULT_COMMAND);
	if (optional_string("$Command Sender:"))
	{
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);

		if (*temp == '#')
			strcpy(pm->command_sender, &temp[1]);
		else
			strcpy(pm->command_sender, temp);
	}

	pm->command_persona = Default_command_persona;
	if (optional_string("$Command Persona:"))
	{
		int idx;
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);

		idx = message_persona_name_lookup(temp);
		if (idx >= 0)
			pm->command_persona = idx;
		else
			Warning(LOCATION, "Supplied Command Persona is invalid!  Defaulting to %s.", Personas[Default_command_persona].name);
	}
	// end of command stuff ----------------------------------------------


	mprintf(("Starting mission message count : %d\n", Num_message_waves));

	// the message_parse function can be found in MissionMessage.h.  The format in the
	// mission file takes the same format as the messages in messages,tbl.  Make parsing
	// a whole lot easier!!!
	while ( required_string_either("#Reinforcements", "$Name")){
		message_parse();		// call the message parsing system
	}

	mprintf(("Ending mission message count : %d\n", Num_message_waves));
}

void parse_reinforcement(mission *pm)
{
	reinforcements *ptr;
	p_object *rforce_obj = NULL;
	int instance = -1;

	Assert(Num_reinforcements < MAX_REINFORCEMENTS);
	Assert(pm != NULL);
	ptr = &Reinforcements[Num_reinforcements];

	required_string("$Name:");
	stuff_string(ptr->name, F_NAME, NAME_LENGTH);	

	find_and_stuff("$Type:", &ptr->type, F_NAME, Reinforcement_type_names, Num_reinforcement_type_names, "reinforcement type");

	required_string("$Num times:");
	stuff_int(&ptr->uses);
	ptr->num_uses = 0;

	// reset the flags to 0
	ptr->flags = 0;

	if ( optional_string("+Arrival delay:") ){
		stuff_int( &(ptr->arrival_delay) );
	}

	if ( optional_string("+No Messages:") ){
		stuff_string_list( ptr->no_messages, MAX_REINFORCEMENT_MESSAGES );
	}

	if ( optional_string("+Yes Messages:") ){
		stuff_string_list( ptr->yes_messages, MAX_REINFORCEMENT_MESSAGES );
	}	

	// sanity check on the names of reinforcements
	rforce_obj = mission_parse_get_parse_object(ptr->name);

	if (rforce_obj == NULL) {
		if ((instance = wing_name_lookup(ptr->name, 1)) == -1) {
			Warning(LOCATION, "Reinforcement %s not found as ship or wing", ptr->name);
			return;
		}
	} else {
		instance = rforce_obj->wingnum;
	}

	// now, if the reinforcement is a wing, then set the number of waves of the wing == number of
	// uses of the reinforcement
	if (instance >= 0) {
		Wings[instance].num_waves = ptr->uses;
	}

	Num_reinforcements++;
}

void parse_reinforcements(mission *pm)
{
	Num_reinforcements = 0;
	required_string("#Reinforcements");

	while (required_string_either("#Background bitmaps", "$Name:"))
		parse_reinforcement(pm);
}

void parse_bitmap(mission *pm)
{
	/*
	char name[NAME_LENGTH];
	int z;
	starfield_bitmaps *ptr;

	Assert(Num_starfield_bitmaps < MAX_STARFIELD_BITMAPS);
	Assert(pm != NULL);
	ptr = &Starfield_bitmaps[Num_starfield_bitmaps];

	required_string("$Bitmap:");
	stuff_string(name, F_NAME, NULL);
	for (z=0; z<Num_starfield_bitmap_lists; z++)	{
		if (!stricmp(name, Starfield_bitmap_list[z].name)){
			break;
		}
	}

	if ( z >= Num_starfield_bitmap_lists )	{
		Warning( LOCATION, "Bitmap specified in mission not in game!\n" );
		z = 0;
	}
	
	ptr->bitmap_index = z;
	required_string("$Orientation:");
	stuff_matrix(&ptr->m);

	required_string("$Rotation rate:");
	stuff_float(&ptr->rot);

	required_string("$Distance:");
	stuff_float(&ptr->dist);

	required_string("$Light:");
	stuff_int(&ptr->light);
	Num_starfield_bitmaps++;
	calculate_bitmap_points(ptr);
	*/
	Int3();
}

void parse_one_background(background_t *background)
{
	// clear here too because this function can be called from more than one place
	background->suns.clear();
	background->bitmaps.clear();

	// parse suns
	while (optional_string("$Sun:"))
	{
		starfield_list_entry sle;

		// filename
		stuff_string(sle.filename, F_NAME, MAX_FILENAME_LEN);

		// angles
		required_string("+Angles:");
		stuff_float(&sle.ang.p);
		stuff_float(&sle.ang.b);
		stuff_float(&sle.ang.h);

		// scale
		required_string("+Scale:");
		stuff_float(&sle.scale_x);
		sle.scale_y = sle.scale_x;
		sle.div_x = 1;
		sle.div_y = 1;

		// add it
		background->suns.push_back(sle);
	}

	// parse starfields
	while (optional_string("$Starbitmap:"))
	{
		starfield_list_entry sle;

		// filename
		stuff_string(sle.filename, F_NAME, MAX_FILENAME_LEN);

		// angles
		required_string("+Angles:");
		stuff_float(&sle.ang.p);
		stuff_float(&sle.ang.b);
		stuff_float(&sle.ang.h);

		// scale
		if (optional_string("+Scale:"))
		{
			stuff_float(&sle.scale_x);
			sle.scale_y = sle.scale_x;
			sle.div_x = 1;
			sle.div_y = 1;
		}
		else
		{
			required_string("+ScaleX:");
			stuff_float(&sle.scale_x);

			required_string("+ScaleY:");
			stuff_float(&sle.scale_y);

			required_string("+DivX:");
			stuff_int(&sle.div_x);

			required_string("+DivY:");
			stuff_int(&sle.div_y);
		}

		// add it
		background->bitmaps.push_back(sle);
	}
}

void parse_bitmaps(mission *pm)
{
	char str[MAX_FILENAME_LEN];
	int z;

	required_string("#Background bitmaps");

	required_string("$Num stars:");
	stuff_int(&Num_stars);
	if (Num_stars >= MAX_STARS)
		Num_stars = MAX_STARS;

	required_string("$Ambient light level:");
	stuff_int(&pm->ambient_light_level);

	if (pm->ambient_light_level == 0)
	{
		pm->ambient_light_level = 0x00787878;
	}

	// This should call light_set_ambient() to
	// set the ambient light

	Nebula_index = -1;
	Mission_palette = 1;

	// neb2 info
	strcpy(Neb2_texture_name, "Eraseme3");
	Neb2_poof_flags = ((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5));
	if(optional_string("+Neb2:")){
		stuff_string(Neb2_texture_name, F_NAME, MAX_FILENAME_LEN);

		required_string("+Neb2Flags:");			
		stuff_int(&Neb2_poof_flags);

		// initialize neb effect. its gross to do this here, but Fred is dumb so I have no choice ... :(
		if(Fred_running && (pm->flags & MISSION_FLAG_FULLNEB)){
			neb2_post_level_init();
		}
	}

	if(pm->flags & MISSION_FLAG_FULLNEB){
		// no regular nebula stuff
		nebula_close();
	} else {
		if (optional_string("+Nebula:")) {
			stuff_string(str, F_NAME, MAX_FILENAME_LEN);
			
			// parse the proper nebula type (full or not)	
			for (z=0; z<NUM_NEBULAS; z++){
				if(pm->flags & MISSION_FLAG_FULLNEB){
					if (!stricmp(str, Neb2_filenames[z])) {
						Nebula_index = z;
						break;
					}
				} else {
					if (!stricmp(str, Nebula_filenames[z])) {
						Nebula_index = z;
						break;
					}
				}
			}

			if (optional_string("+Color:")) {
				stuff_string(str, F_NAME, MAX_FILENAME_LEN);
				for (z=0; z<NUM_NEBULA_COLORS; z++){
					if (!stricmp(str, Nebula_colors[z])) {
						Mission_palette = z;
						break;
					}
				}
			}

			if (optional_string("+Pitch:")){
				stuff_int(&Nebula_pitch);
			} else {
				Nebula_pitch = 0;
			}

			if (optional_string("+Bank:")){
				stuff_int(&Nebula_bank);
			} else {
				Nebula_bank = 0;
			}

			if (optional_string("+Heading:")){
				stuff_int(&Nebula_heading);
			} else {
				Nebula_heading = 0;
			}						
		}

		if (Nebula_index >= 0){		
			nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);
		} else {
			nebula_close();		
		}
	}	

	// Goober5000
	Num_backgrounds = 0;
	while (optional_string("$Bitmap List:") || check_for_string("$Sun:") || check_for_string("$Starbitmap:"))
	{
		// don't allow overflow; just make sure the last background is the last read
		if (Num_backgrounds >= MAX_BACKGROUNDS)
		{
			Warning(LOCATION, "Too many backgrounds in mission!  Max is %d.", MAX_BACKGROUNDS);
			Num_backgrounds = MAX_BACKGROUNDS - 1;
		}

		parse_one_background(&Backgrounds[Num_backgrounds]);
		Num_backgrounds++;
	}

	// Goober5000
	stars_load_first_valid_background();

	if (optional_string("$Environment Map:")) {
		stuff_string(pm->envmap_name, F_NAME, MAX_FILENAME_LEN);
	}

	// bypass spurious stuff from e.g. FS1 missions
	skip_to_start_of_string("#");
}

void parse_asteroid_fields(mission *pm)
{
	int i, count, subtype;

	Assert(pm != NULL);
	for (i=0; i<MAX_ASTEROID_FIELDS; i++)
		Asteroid_field.num_initial_asteroids = 0;

	i = 0;
	count = 0;

	if (!optional_string("#Asteroid Fields"))
		return;

	while (required_string_either("#", "$density:")) {
		float speed, density;
		int type;


		Assert(i < 1);
		required_string("$Density:");
		stuff_float(&density);

		Asteroid_field.num_initial_asteroids = (int) density;

		Asteroid_field.field_type = FT_ACTIVE;
		if (optional_string("+Field Type:")) {
			stuff_int( &type );
			Asteroid_field.field_type = (field_type_t)type;
		}

		Asteroid_field.debris_genre = DG_ASTEROID;
		if (optional_string("+Debris Genre:")) {
			stuff_int( &type );
			Asteroid_field.debris_genre = (debris_genre_t)type;
		}

		Asteroid_field.field_debris_type[0] = -1;
		Asteroid_field.field_debris_type[1] = -1;
		Asteroid_field.field_debris_type[2] = -1;
		if (Asteroid_field.debris_genre == DG_SHIP) {
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[0]);
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[1]);
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[2]);
			}
		} else {
			// debris asteroid
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
		}

		// backward compatibility
		if ( (Asteroid_field.debris_genre == DG_ASTEROID) && (count == 0) ) {
			Asteroid_field.field_debris_type[0] = 0;
		}

		required_string("$Average Speed:");
		stuff_float(&speed);

		vm_vec_rand_vec_quick(&Asteroid_field.vel);
		vm_vec_scale(&Asteroid_field.vel, speed);

		Asteroid_field.speed = speed;

		required_string("$Minimum:");
		stuff_vector(&Asteroid_field.min_bound);

		required_string("$Maximum:");
		stuff_vector(&Asteroid_field.max_bound);

		if (optional_string("+Inner Bound:")) {
			Asteroid_field.has_inner_bound = 1;

			required_string("$Minimum:");
			stuff_vector(&Asteroid_field.inner_min_bound);

			required_string("$Maximum:");
			stuff_vector(&Asteroid_field.inner_max_bound);
		} else {
			Asteroid_field.has_inner_bound = 0;
		}
		i++;
	}
}

void parse_variables()
{
	int i, j, k, num_variables;

	if (! optional_string("#Sexp_variables") ) {
		return;
	} else {
		num_variables = stuff_sexp_variable_list();
	}

	// yeesh - none of this should be done in FRED :)
	if (!Fred_running)
	{
		// Goober5000 - now set the default value, if it's a campaign-persistent variable
		// look through all previous missions (by doing it this way, we will continually
		// overwrite the variable with the most recent information)
		for (i=0; i<Campaign.num_missions; i++)
		{
			if (Campaign.missions[i].completed != 1)
				continue;

			// loop through this particular previous mission's variables
			for (j=0; j<Campaign.missions[i].num_saved_variables; j++)
			{
				// loop through the current mission's variables
				for (k=0; k<num_variables; k++)
				{
					// if the active mission has a variable with the same name as a campaign
					// variable AND it is not a block variable, override its initial value
					// with the previous mission's value
					if (!(stricmp(Sexp_variables[k].variable_name, Campaign.missions[i].saved_variables[j].variable_name))
						&& !(Campaign.missions[i].saved_variables[j].type & SEXP_VARIABLE_BLOCK))
					{
						Sexp_variables[k].type = Campaign.missions[i].saved_variables[j].type;
						strcpy(Sexp_variables[k].text, Campaign.missions[i].saved_variables[j].text);
					}
				}
			}
		}

		// Goober5000 - next, see if any player-persistent variables are set
		for (i=0; i<Player->num_variables; i++)
		{
			// loop through the current mission's variables
			for (j=0; j<num_variables; j++)
			{
				// if the active mission has a variable with the same name as a player
				// variable AND it is not a block variable, override its initial value
				// with the previous mission's value
				if (!(stricmp(Sexp_variables[j].variable_name, Player->player_variables[i].variable_name))
					&& !(Player->player_variables[i].type & SEXP_VARIABLE_BLOCK))
				{
					Sexp_variables[j].type = Player->player_variables[i].type;
					strcpy(Sexp_variables[j].text, Player->player_variables[i].text);
				}
			}
		}
	}
}

void parse_mission(mission *pm, int flags)
{
	int i;

	Player_starts = Num_cargo = Num_waypoint_lists = Num_goals = Num_wings = 0;
	Player_start_shipnum = -1;
	*Player_start_shipname = 0;		// make the string 0 length for checking later
	memset( &Player_start_pobject, 0, sizeof(Player_start_pobject) );
	Fred_num_texture_replacements = 0;

	// initialize the initially_docked array.
	for ( i = 0; i < MAX_SHIPS; i++ ) {
		Initially_docked[i].docker[0] = '\0';
		Initially_docked[i].dockee[0] = '\0';
		Initially_docked[i].docker_point[0] = '\0';
		Initially_docked[i].dockee_point[0] = '\0';
	}
	Total_initially_docked = 0;

	list_init(&Ship_arrival_list);	// init list for arrival ships

	parse_init();

	Subsys_index = 0;
	Subsys_status_size = 0;

	if (Subsys_status != NULL) {
		vm_free( Subsys_status );
		Subsys_status = NULL;
	}

	parse_mission_info(pm); 

	Current_file_checksum = netmisc_calc_checksum(pm,MISSION_CHECKSUM_SIZE);

	if (flags & MPF_ONLY_MISSION_INFO)
		return;

	parse_plot_info(pm);
	parse_variables();
	parse_briefing_info(pm);	// TODO: obsolete code, keeping so we don't obsolete existing mission files
	parse_cmd_briefs(pm);
	parse_briefing(pm, flags);
	parse_debriefing_new(pm);
	parse_player_info(pm);
	parse_objects(pm, flags);
	parse_wings(pm);
	parse_events(pm);
	parse_goals(pm);
	parse_waypoints(pm);
	parse_messages(pm);
	parse_reinforcements(pm);
	parse_bitmaps(pm);
	parse_asteroid_fields(pm);
	parse_music(pm, flags);

	post_process_mission(pm);
}

void post_process_mission(mission *pm)
{
	int			i;
	int			indices[MAX_SHIPS], objnum;
	ship_weapon	*swp;
	ship_obj *so;

	// Goober5000 - must be done before all other post processing
	post_process_ships_wings();

	// the player_start_shipname had better exist at this point!
	Player_start_shipnum = ship_name_lookup( Player_start_shipname );
	Assert ( Player_start_shipnum != -1 );
	Assert ( Player_start_pobject.flags & P_SF_PLAYER_START_VALID );

	// Assign objnum, shipnum, etc. to the player structure
	objnum = Ships[Player_start_shipnum].objnum;
	Player_obj = &Objects[objnum];
	if (!Fred_running){
		Player->objnum = objnum;
	}

	Player_obj->flags |= OF_PLAYER_SHIP;			// make this object a player controlled ship.
	Player_ship = &Ships[Player_start_shipnum];
	Player_ai = &Ai_info[Player_ship->ai_index];

	Player_ai->targeted_subsys = NULL;
	Player_ai->targeted_subsys_parent = -1;

	// determine if player start has initial velocity and set forward cruise percent to relect this
	if ( Player_obj->phys_info.vel.xyz.z > 0.0f )
		Player->ci.forward_cruise_percent = Player_obj->phys_info.vel.xyz.z / Player_ship->current_max_speed * 100.0f;

	// now that we have the player ship...
	// if the player doesn't have shields, support shouldn't either
	if (Player_obj->flags & OF_NO_SHIELDS)
		pm->support_ships.ssi_flags |= SSIF_NO_SHIELDS;

	// set up wing indexes
	for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
		Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);
	}

	for (i = 0; i < MAX_SQUADRON_WINGS; i++ ) {
		Squadron_wings[i] = wing_name_lookup(Squadron_wing_names[i], 1);
	}

	for (i = 0; i < MAX_TVT_WINGS; i++ ) {
		TVT_wings[i] = wing_name_lookup(TVT_wing_names[i], 1);
	}

	// when TVT, hack starting wings to be team wings
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
		Assert(MAX_TVT_WINGS <= MAX_STARTING_WINGS);
		for (i=0; i<MAX_STARTING_WINGS; i++)
		{
			if (i<MAX_TVT_WINGS)
				Starting_wings[i] = TVT_wings[i];
			else
				Starting_wings[i] = -1;
		}
	}

	init_ai_system();

	// Goober5000 - this needs to be called only once after parsing of objects and wings is complete
	// (for individual invalidation, see mission_parse_mark_non_arrival)
	mission_parse_mark_non_arrivals();

	// deal with setting up arrival location for all ships.  Must do this now after all ships are created
	mission_parse_set_arrival_locations();

	// clear out information about arriving support ships
	Arriving_support_ship = NULL;
	Num_arriving_repair_targets = 0;

	// convert all ship name indices to ship indices now that mission has been loaded
	if (Fred_running) {
		for (i=0; i<Num_parse_names; i++) {
			indices[i] = ship_name_lookup(Parse_names[i], 1);
			if (indices[i] < 0)
				Warning(LOCATION, "Ship name \"%s\" referenced, but this ship doesn't exist", Parse_names[i]);
		}

		for (i=0; i<MAX_SHIPS; i++) {
			if ((Ships[i].objnum >= 0) && (Ships[i].arrival_anchor >= 0) && (Ships[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHOR_FLAG))
				Ships[i].arrival_anchor = indices[Ships[i].arrival_anchor];

			if ( (Ships[i].objnum >= 0) && (Ships[i].departure_anchor >= 0) )
				Ships[i].departure_anchor = indices[Ships[i].departure_anchor];
		}

		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count  && (Wings[i].arrival_anchor >= 0) && (Wings[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHOR_FLAG))
				Wings[i].arrival_anchor = indices[Wings[i].arrival_anchor];

			if (Wings[i].wave_count  && (Wings[i].departure_anchor >= 0) )
				Wings[i].departure_anchor = indices[Wings[i].departure_anchor];
		}

	}

	// before doing anything else, we must validate all of the sexpressions that were loaded into the mission.
	// Loop through the Sexp_nodes array and send the top level functions to the check_sexp_syntax parser

	for (i = 0; i < Num_sexp_nodes; i++) {
		if ( is_sexp_top_level(i) && (!Fred_running || (i != Sexp_clipboard))) {
			int result, bad_node, op;

			op = get_operator_index(CTEXT(i));
			Assert(op != -1);  // need to make sure it is an operator before we treat it like one..
			result = check_sexp_syntax( i, query_operator_return_type(op), 1, &bad_node);

			// entering this if statement will result in program termination!!!!!
			// print out an error based on the return value from check_sexp_syntax()
			if ( result ) {
				char sexp_str[8192], text[8192];

				convert_sexp_to_string( i, sexp_str, SEXP_ERROR_CHECK_MODE);
				sprintf(text, "%s.\n\nIn sexpression: %s\n(Error appears to be: %s)",
					sexp_error_message(result), sexp_str, Sexp_nodes[bad_node].text);

				if (!Fred_running)
					Error( LOCATION, text );
				else
					Warning( LOCATION, text );
			}
		}
	}

	ai_post_process_mission();


	// we must also count all of the ships of particular types.  We count all of the ships that do not have
	// their SF_IGNORE_COUNT flag set.  We don't count ships in wings when the equivalent wing flag is set.
	// in counting ships in wings, we increment the count by the wing's wave count to account for everyone.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		int siflags, num, shipnum;

		shipnum = Objects[so->objnum].instance;
		// pass over non-ship objects and player ship objects
		if ( Ships[shipnum].objnum == -1 || (Objects[Ships[shipnum].objnum].flags & OF_PLAYER_SHIP) )
			continue;
		if ( Ships[shipnum].flags & SF_IGNORE_COUNT )
			continue;
		if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].flags & WF_IGNORE_COUNT) )
			continue;

		siflags = Ship_info[Ships[shipnum].ship_info_index].flags;
		
		// determine the number of times we need to add this ship into the count
//		if ( Ships[i].wingnum == -1 )
			num = 1;
//		else
//			num = Wings[Ships[i].wingnum].num_waves;

		ship_add_ship_type_count( Ships[shipnum].ship_info_index, num );
	}

	// now go through the list of ships yet to arrive
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		int siflags, num;

		// go through similar motions as above
		if ( p_objp->flags & P_SF_IGNORE_COUNT )
			continue;
		if ( (p_objp->wingnum != -1) && (Wings[p_objp->wingnum].flags & WF_IGNORE_COUNT) )
			continue;

		siflags = Ship_info[p_objp->ship_class].flags;

		if ( p_objp->wingnum == -1 )
			num = 1;
		else
			num = Wings[p_objp->wingnum].num_waves - 1;			// subtract one since we already counted the first wave
		
		ship_add_ship_type_count( p_objp->ship_class, num );
	}

	// set player weapons that are selected by default
	// AL 09/17/97: I added this code to select the first primary/secondary weapons, 
	// since I noticed the player ship sometimes doesn't get default weapons selected		

	// DB: modified 4/23/98 to take multiplayer into account. Under certain circumstances, multiplayer netplayer ships
	//     had their current_primary_bank and current_secondary_bank set to -1 (from ship_set()) and left there since
	//     Player_ship is not the only one we need to need about.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		ship *shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags & SF_FROM_PLAYER_WING ) )
			continue;			

		swp = &shipp->weapons;
	
		// swp = &Player_ship->weapons;
		if ( swp->num_primary_banks > 0 ) {
			swp->current_primary_bank = 0;			// currently selected primary bank
	//		ship_primary_changed(shipp);
		}

		if ( swp->num_secondary_banks > 0 ) {
			swp->current_secondary_bank = 0;			// currently selected secondary bank
	//		ship_secondary_changed(shipp);
		}
	}

	ets_init_ship(Player_obj);	// init ETS data for the player

	// put the timestamp stuff here for now
	Mission_arrival_timestamp = timestamp( ARRIVAL_TIMESTAMP );
	Mission_departure_timestamp = timestamp( DEPARTURE_TIMESTAMP );
	Mission_end_time = -1;

	if(Game_mode & GM_MULTIPLAYER){ 
		multi_respawn_build_points();
	}	

	// maybe reset hotkey defaults when loading new mission
	if ( Last_file_checksum != Current_file_checksum ){
		mission_hotkey_reset_saved();
	}

	Allow_arrival_music_timestamp=timestamp(0);
	Allow_arrival_message_timestamp=timestamp(0);
	Arrival_message_delay_timestamp = timestamp(-1);

	int idx;
	for(idx=0; idx<2; idx++){
		Allow_arrival_music_timestamp_m[idx]=timestamp(0);
		Allow_arrival_message_timestamp_m[idx]=timestamp(0);
		Arrival_message_delay_timestamp_m[idx] = timestamp(-1);
	}	

	Last_file_checksum = Current_file_checksum;
}

int get_mission_info(char *filename, mission *mission_p, bool basic)
{

	char real_fname[MAX_FILENAME_LEN];
	strcpy(real_fname, filename);
	char *p = strchr(real_fname, '.');
	if (p) *p = 0; // remove any extension
	strcat(real_fname, FS_MISSION_FILE_EXT);  // append mission extension

	int rval;

	// if mission_p is NULL, make it point to The_mission
	if ( mission_p == NULL )
		mission_p = &The_mission;

	if ((rval = setjmp(parse_abort)) != 0) {
		nprintf(("Error", "Error abort!  Code = %d", rval));
		return rval;
	
	} else {
		int filelength;

		// open localization
		lcl_ext_open();

		CFILE *ftemp = cfopen(real_fname, "rt");
		if (!ftemp){
			// close localization
			lcl_ext_close();

			return -1;
		}

		// 7/9/98 -- MWA -- check for 0 length file.
		filelength = cfilelength(ftemp);
		cfclose(ftemp);
		if ( filelength == 0 ){
			// close localization
			lcl_ext_close();	

			return -1;
		}

		read_file_text(real_fname, CF_TYPE_MISSIONS);
		memset( mission_p, 0, sizeof(mission) );
		parse_init(basic);
		parse_mission_info(mission_p, basic);

		// close localization
		lcl_ext_close();
	}

	return 0;
}

// Goober5000 - changed and moved from parselo.cpp
// Initialize the mission parse process.
void parse_init(bool basic)
{
	reset_parse();

	for (int i = 0; i < MAX_CARGO; i++)
		Cargo_names[i] = Cargo_names_buf[i]; // make a pointer array for compatibility

	Total_goal_ship_names = 0;

	// if we are just wanting basic info then we shouldn't need sexps
	// (prevents memory fragmentation with the now dynamic Sexp_nodes[])
	if ( !basic )
		init_sexp();
}

// mai parse routine for parsing a mission.  The default parameter flags tells us which information
// to get when parsing the mission.  0 means get everything (default).  Other flags just gets us basic
// info such as game type, number of players etc.
// Goober5000 - allow for import
int parse_main(char *mission_name, int flags)
{
	int rval, i;

	// fill in Ship_class_names array with the names from the ship_info struct;
	Num_parse_names = 0;
	Num_path_restrictions = 0;
	Assert(Num_ship_classes <= MAX_SHIP_CLASSES);	// Goober5000 - should be <=

	for (i = 0; i < Num_ship_classes; i++)
		Ship_class_names[i] = Ship_info[i].name;

	if ((rval = setjmp(parse_abort)) != 0) {
		nprintf(("Error", "Error abort!  Code = %i.", rval));
		return rval;
	
	} else {
		// open localization
		lcl_ext_open();

		// don't do this for imports
		if (!(flags & MPF_IMPORT_FSM))
		{
			CFILE *ftemp = cfopen(mission_name, "rt", CFILE_NORMAL, CF_TYPE_MISSIONS);

			// fail situation.
			if (!ftemp) {
				if (!Fred_running)
					Error( LOCATION, "Couldn't open mission '%s'\n", mission_name );

				Current_file_length = -1;
				Current_file_checksum = 0;
	
				// close localization
				lcl_ext_close();

				return -1;
			}

			Current_file_length = cfilelength(ftemp);
			cfclose(ftemp);
		}

		// import?
		if (flags & MPF_IMPORT_FSM)
		{
			read_file_text(mission_name, CF_TYPE_ANY);
			convertFSMtoFS2();
		}
		else
		{
			read_file_text(mission_name, CF_TYPE_MISSIONS);
		}

		memset(&The_mission, 0, sizeof(The_mission));
		parse_mission(&The_mission, flags);
		display_parse_diagnostics();

		// close localization
		lcl_ext_close();
	}

	if (!Fred_running)
		strcpy(Mission_filename, mission_name);

	return 0;
}

void mission_parse_close()
{
	// free subsystems
	if (Subsys_status != NULL)
	{
		vm_free(Subsys_status);
		Subsys_status = NULL;
	}

	// free parse object dock lists
	for (int i = 0; i < Num_parse_objects; i++)
	{
		dock_free_instances(&Parse_objects[i]);
	}
}

// sets the arrival location of the ships in wingp.  pass num_to_set since the threshold value
// for wings may have us create more ships in the wing when there are still some remaining
void mission_set_wing_arrival_location( wing *wingp, int num_to_set )
{
	int index;

	// get the starting index into the ship_index array of the first ship whose location we need set.

	index = wingp->current_count - num_to_set;
	if ( (wingp->arrival_location == ARRIVE_FROM_DOCK_BAY) || (wingp->arrival_location == ARRIVE_AT_LOCATION) ) {
		while ( index < wingp->current_count ) {
			object *objp;

			objp = &Objects[Ships[wingp->ship_index[index]].objnum];
			mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(objp), wingp->arrival_path_mask, NULL, NULL);

			index++;
		}
	} else {
		object *leader_objp;
		vec3d pos;
		matrix orient;
		int wing_index;

		// wing is not arriving from a docking bay -- possibly move them based on arriving near
		// or in front of some other ship.
		index = wingp->current_count - num_to_set;
		leader_objp = &Objects[Ships[wingp->ship_index[index]].objnum];
		if (mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(leader_objp), wingp->arrival_path_mask, &pos, &orient)) {
			// modify the remaining ships created
			index++;
			wing_index = 1;
			while ( index < wingp->current_count ) {
				object *objp;

				objp = &Objects[Ships[wingp->ship_index[index]].objnum];

				// change the position of the next ships in the wing.  Use the cool function in AiCode.cpp which
				// Mike K wrote to give new positions to the wing members.
				get_absolute_wing_pos( &objp->pos, leader_objp, wing_index++, 0);
				memcpy( &objp->orient, &orient, sizeof(matrix) );

				index++;
			}
		}
	}

	// create warp effect if in mission and not arriving from docking bay
	if ( (Game_mode & GM_IN_MISSION) && (wingp->arrival_location != ARRIVE_FROM_DOCK_BAY) ) {
		for ( index = wingp->current_count - num_to_set; index < wingp->current_count; index ++ ) {
			shipfx_warpin_start( &Objects[Ships[wingp->ship_index[index]].objnum] );
		}
	}
}

// this function is called after a mission is parsed to set the arrival locations of all ships in the
// mission to the apprioriate spot.  Mainly needed because ships might be in dock bays to start
// the mission, so their AI mode must be set appropriately.
void mission_parse_set_arrival_locations()
{
	int i;
	object *objp;

	if ( Fred_running )
		return;

	obj_merge_created_list();
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		ship *shipp;

		if ( objp->type != OBJ_SHIP ) 
			continue;

		shipp = &Ships[objp->instance];
		// if the ship is in a wing -- ignore the info and let the wing info handle it
		if ( shipp->wingnum != -1 )
			continue;

		// call function to set arrival location for this ship.
		mission_set_arrival_location( shipp->arrival_anchor, shipp->arrival_location, shipp->arrival_distance, OBJ_INDEX(objp), shipp->arrival_path_mask, NULL, NULL);
	}

	// do the wings
	for ( i = 0; i < Num_wings; i++ ) {

		// if wing has no ships, then don't process it.
		if ( Wings[i].current_count == 0 )
			continue;

		mission_set_wing_arrival_location( &Wings[i], Wings[i].current_count );
	}
}

// Goober5000
bool sexp_is_locked_false(int node)
{
	// dunno why these are different, but they are
	if (Fred_running)
		return (node == Locked_sexp_false);
	else
		return (Sexp_nodes[node].value == SEXP_KNOWN_FALSE);
}

// Goober5000
// NOTE - in both retail and SCP, the dock "leader" is defined as the only guy in his
// group with a non-false arrival cue
void parse_object_mark_dock_leader_helper(p_object *pobjp, p_dock_function_info *infop)
{
	int cue_to_check;

	// if this guy is part of a wing, he uses his wing's arrival cue
	if (pobjp->wingnum >= 0)
	{
		cue_to_check = Wings[pobjp->wingnum].arrival_cue;
	}
	// check the object's arrival cue
	else
	{
		cue_to_check = pobjp->arrival_cue;
	}

	// is he a leader (using the definition above)?
	if (!sexp_is_locked_false(cue_to_check))
	{
		p_object *existing_leader;

		// increment number of leaders found
		infop->maintained_variables.int_value++;

		// see if we already found a leader
		existing_leader = infop->maintained_variables.objp_value;
		if (existing_leader != NULL)
		{
			// keep existing leader if he has a higher priority than us
			if (ship_class_compare(pobjp->ship_class, existing_leader->ship_class) < 0)
			{
				// set my arrival cue to false
				free_sexp2(pobjp->arrival_cue);
				pobjp->arrival_cue = Locked_sexp_false;
				return;
			}

			// otherwise, unmark the existing leader and set his arrival cue to false
			existing_leader->flags &= ~P_SF_DOCK_LEADER;
			free_sexp2(existing_leader->arrival_cue);
			existing_leader->arrival_cue = Locked_sexp_false;
		}

		// mark and save me as the leader
		pobjp->flags |= P_SF_DOCK_LEADER;
		infop->maintained_variables.objp_value = pobjp;
	}
}

// Goober5000
void parse_object_set_handled_flag_helper(p_object *pobjp, p_dock_function_info *infop)
{
	pobjp->flags2 |= P2_ALREADY_HANDLED;
}

// Goober5000
void parse_object_clear_handled_flag_helper(p_object *pobjp, p_dock_function_info *infop)
{
	pobjp->flags2 &= ~P2_ALREADY_HANDLED;
}

// Goober5000
void parse_object_clear_all_handled_flags()
{
	// clear flag for all ships
	for (int i = 0; i < Num_parse_objects; i++)
	{
		p_object *pobjp = &Parse_objects[i];
		p_dock_function_info dfi;

		// since we're going through all objects, this object may not be docked
		if (!object_is_docked(pobjp))
			continue;

		// has this object (by extension, this group of docked objects) been cleared already?
		if (!(pobjp->flags2 & P2_ALREADY_HANDLED))
			continue;

		// clear the handled flag for this group
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_clear_handled_flag_helper);
	}
}

// Goober5000
// This function iterates through the Initially_docked array and builds the dock trees
// for each parse object.  This can only be done after all objects and wings have been parsed.
void mission_parse_set_up_initial_docks()
{
	int i;

	// build trees
	for (i = 0; i < Total_initially_docked; i++)
	{
		char *docker_point, *dockee_point;
		p_object *docker, *dockee;

		// resolve the docker and dockee
		docker = mission_parse_get_parse_object(Initially_docked[i].docker);
		dockee = mission_parse_get_parse_object(Initially_docked[i].dockee);
		Assert((docker != NULL) && (dockee != NULL));

		// resolve the dockpoints
		docker_point = Initially_docked[i].docker_point;
		dockee_point = Initially_docked[i].dockee_point;

		// if they're not already docked, dock them
		if (!dock_check_find_direct_docked_object(docker, dockee))
		{
			dock_dock_objects(docker, docker_point, dockee, dockee_point);
		}
	}

	// now resolve the leader of each tree
	for (i = 0; i < Num_parse_objects; i++)
	{
		p_object *pobjp = &Parse_objects[i];
		p_dock_function_info dfi;

		// since we're going through all objects, this object may not be docked
		if (!object_is_docked(pobjp))
			continue;

		// has this object (by extension, this group of docked objects) been handled already?
		if (pobjp->flags2 & P2_ALREADY_HANDLED)
			continue;

		// find the dock leader(s)
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_mark_dock_leader_helper);

		// display an error if necessary
		if (dfi.maintained_variables.int_value == 0)
		{
			Warning(LOCATION, "No dock leaders found in the docking group containing %s.  The group will not appear in-mission!\n", pobjp->name);
		}
		else if (dfi.maintained_variables.int_value > 1)
		{
			Warning(LOCATION, "There are multiple dock leaders in the docking group containing the leader %s!  Setting %s as the sole leader...\n", dfi.maintained_variables.objp_value->name, dfi.maintained_variables.objp_value->name);
		}

		// clear dfi stuff
		dfi.maintained_variables.int_value = 0;
		dfi.maintained_variables.objp_value = NULL;

		// set the handled flag for this group
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_set_handled_flag_helper);
	}

	// now clear the handled flags
	parse_object_clear_all_handled_flags();
}

// function which returns true or false if the given mission support multiplayers
int mission_parse_is_multi(char *filename, char *mission_name)
{
	int rval, game_type;
	int filelength;
	CFILE *ftemp;

	// new way of getting information.  Open the file, and just get the name and the game_type flags.
	// return the flags if a multiplayer mission

	game_type = 0;

	ftemp = cfopen(filename, "rt");
	if (!ftemp)
		return 0;

	// 7/9/98 -- MWA -- check for 0 length file.
	filelength = cfilelength(ftemp);
	cfclose(ftemp);
	if ( filelength == 0 )
		return 0;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		Error(LOCATION, "Bogus!  Trying to get multi game type on mission %s returned as a mission from cf_get_filelist\n");
	} else	{
		read_file_text(filename, CF_TYPE_MISSIONS);
		reset_parse();
		if ( skip_to_string("$Name:") != 1 ) {
			nprintf(("Network", "Unable to process %s because we couldn't find $Name:", filename));

			// close localization
			lcl_ext_close();

			return 0;
		}
		stuff_string( mission_name, F_NAME, NAME_LENGTH );
		if ( skip_to_string("+Game Type Flags:") != 1 ) {
			nprintf(("Network", "Unable to process %s because we couldn't find +Game Type Flags:\n", filename));

			// close localization
			lcl_ext_close();

			return 0;
		}
		stuff_int(&game_type);
	}
	if ( game_type & MISSION_TYPE_MULTI ){
		// close localization
		lcl_ext_close();

		return game_type;
	}

	// close localization
	lcl_ext_close();
	
	return 0;
}

// function which gets called to retrieve useful information about a mission.  We will get the
// name, description, and number of players for a mission.  Probably used for multiplayer only?
// The calling function can use the information in The_mission to get the name/description of the mission
// if needed.

int mission_parse_get_multi_mission_info( char *filename )
{
	if ( get_mission_info(filename, &The_mission) )
		return -1;

	Assert( The_mission.game_type & MISSION_TYPE_MULTI );		// assume multiplayer only for now?

	// return the number of parse_players.  later, we might want to include (optionally?) the number
	// of other ships in the main players wing (usually wing 'alpha') for inclusion of number of
	// players allowed.

	return The_mission.num_players;
}

// return the parse object on the ship arrival list associated with the given name
p_object *mission_parse_get_arrival_ship(char *name)
{
	p_object *p_objp;

	if (name == NULL)
		return NULL;

	for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (!stricmp(p_objp->name, name))
			return p_objp;	// still on the arrival list
	}

	return NULL;
}

// return the parse object on the ship arrival list associated with the given signature
p_object *mission_parse_get_arrival_ship(ushort net_signature)
{
	p_object *p_objp;

	for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp !=END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (p_objp->net_signature == net_signature)
			return p_objp;	// still on the arrival list
	}

	return NULL;
}

// mission_set_arrival_location() sets the arrival location of a parse object according to the arrival location
// of the object.  Returns true if object set to new position, false if not.
int mission_set_arrival_location(int anchor, int location, int dist, int objnum, int path_mask, vec3d *new_pos, matrix *new_orient)
{
	int shipnum, anchor_objnum;
	vec3d anchor_pos, rand_vec, new_fvec;
	matrix orient;

	if ( location == ARRIVE_AT_LOCATION )
		return 0;

	Assert(anchor >= 0);

	// this ship might possibly arrive at another location.  The location is based on the
	// proximity of some ship (and some other special tokens)
	if (anchor & SPECIAL_ARRIVAL_ANCHOR_FLAG)
	{
		bool get_players = (anchor & SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG) > 0;

		// filter out iff
		int iff_index = anchor;
		iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_FLAG;
		iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG;

		// get ship
		shipnum = ship_get_random_team_ship(iff_get_mask(iff_index), get_players ? SHIP_GET_ONLY_PLAYERS : SHIP_GET_ANY_SHIP);
	}
	// if we didn't find the arrival anchor in the list of special nodes, then do a
	// ship name lookup on the anchor
	else
	{
		shipnum = ship_name_lookup(Parse_names[anchor]);
	}

	// if we didn't get an object from one of the above functions, then make the object
	// arrive at its placed location
	if (shipnum < 0)
	{
		Assert ( location != ARRIVE_FROM_DOCK_BAY );		// bogus data somewhere!!!  get mwa
		nprintf (("allender", "couldn't find ship for arrival anchor -- using location ship created at"));
		return 0;
	}


	// take the shipnum and get the position.  once we have positions, we can determine where
	// to make this ship appear
	Assert ( shipnum != -1 );
	anchor_objnum = Ships[shipnum].objnum;
	anchor_pos = Objects[anchor_objnum].pos;

	// if arriving from docking bay, then set ai mode and call function as per AL's instructions.
	if ( location == ARRIVE_FROM_DOCK_BAY ) {
		vec3d pos, fvec;

		// if we get an error, just let the ship arrive(?)
		if ( ai_acquire_emerge_path(&Objects[objnum], anchor_objnum, path_mask, &pos, &fvec) == -1 ) {
			Int3();			// get MWA or AL -- not sure what to do here when we cannot acquire a path
			return 0;
		}
		Objects[objnum].pos = pos;
		Objects[objnum].orient.vec.fvec = fvec;
	} else {

		// AL: ensure dist > 0 (otherwise get errors in vecmat)
		// TODO: maybe set distance to 2x ship radius of ship appearing in front of?
		if ( dist <= 0 )
		{
			// Goober5000 - default to 100
			Error(LOCATION, "Distance of %d is invalid in mission_set_arrival_location.  Defaulting to 100.\n", dist);
			dist = 100;
			//return 0;
		}
		
		// get a vector which is the ships arrival position based on the type of arrival
		// this ship should have.  Arriving near a ship we use a random normalized vector
		// scaled by the distance given by the designer.  Arriving in front of a ship means
		// entering the battle in the view cone.
		if ( location == ARRIVE_NEAR_SHIP ) {
			// get a random vector -- use static randvec if in multiplayer
			if ( Game_mode & GM_NORMAL )
				vm_vec_rand_vec_quick(&rand_vec);
			else
				static_randvec( Objects[objnum].net_signature, &rand_vec );
		} else if ( location == ARRIVE_IN_FRONT_OF_SHIP ) {
			vec3d t1, t2, t3;
			int r1, r2;
			float x;

			// cool function by MK to give a reasonable random vector "in front" of a ship
			// rvec and uvec are the right and up vectors.
			// If these are not available, this would be an expensive method.
			//x = cos(angle)
			x = (float)cos(ANG_TO_RAD(45));
			if ( Game_mode & GM_NORMAL ) {
				r1 = rand() < RAND_MAX_2 ? -1 : 1;
				r2 = rand() < RAND_MAX_2 ? -1 : 1;
			} else {
				// in multiplayer, use the static rand functions so that all clients can get the
				// same information.
				r1 = static_rand(Objects[objnum].net_signature) < RAND_MAX_2 ? -1 : 1;
				r2 = static_rand(Objects[objnum].net_signature+1) < RAND_MAX_2 ? -1 : 1;
			}

			vm_vec_copy_scale(&t1, &(Objects[anchor_objnum].orient.vec.fvec), x);
			vm_vec_copy_scale(&t2, &(Objects[anchor_objnum].orient.vec.rvec), (1.0f - x) * r1);
			vm_vec_copy_scale(&t3, &(Objects[anchor_objnum].orient.vec.uvec), (1.0f - x) * r2);

			vm_vec_add(&rand_vec, &t1, &t2);
			vm_vec_add2(&rand_vec, &t3);
			vm_vec_normalize(&rand_vec);
		}

		// add in the radius of the two ships involved.  This will make the ship arrive further than
		// specified, but will appear more accurate since we are pushing the edge of the model to the
		// specified distance.  large objects appears to be a lot closer without the following line because
		// the object centers were at the correct distance, but the model itself was much closer to the
		// target ship.
		dist += (int)Objects[objnum].radius + (int)Objects[anchor_objnum].radius;
		vm_vec_scale_add(&Objects[objnum].pos, &anchor_pos, &rand_vec, (float)dist);

		// I think that we will always want to orient the ship that is arriving to face towards
		// the ship it is arriving near/in front of.  The effect will be cool!
		//
		// calculate the new fvec of the ship arriving and use only that to get the matrix.  isn't a big
		// deal not getting bank.
		vm_vec_sub(&new_fvec, &anchor_pos, &Objects[objnum].pos );
		vm_vector_2_matrix( &orient, &new_fvec, NULL, NULL );
		Objects[objnum].orient = orient;
	}

	// set the new_pos parameter since it might be used outside the function (i.e. when dealing with wings).
	if ( new_pos )
		memcpy(new_pos, &Objects[objnum].pos, sizeof(vec3d) );

	if ( new_orient )
		memcpy( new_orient, &Objects[objnum].orient, sizeof(matrix) );

	return 1;
}

// mark a reinforcement as available
void mission_parse_mark_reinforcement_available(char *name)
{
	int i;
	reinforcements *rp;

	for (i = 0; i < Num_reinforcements; i++) {
		rp = &Reinforcements[i];
		if ( !stricmp(rp->name, name) ) {
			if ( !(rp->flags & RF_IS_AVAILABLE) ) {
				rp->flags |= RF_IS_AVAILABLE;

				// tell all of the clients.
				if ( MULTIPLAYER_MASTER ) {
					send_reinforcement_avail( i );
				}
			}
			return;
		}
	}

	Assert ( i < Num_reinforcements );
}

// mission_did_ship_arrive takes a parse object and checked the arrival cue and delay and
// creates the object if necessary.  Returns -1 if not created.  objnum of created ship otherwise
int mission_did_ship_arrive(p_object *objp)
{
	int should_arrive;

	// find out in the arrival cue became true
	should_arrive = eval_sexp(objp->arrival_cue);

	// we must first check to see if this ship is a reinforcement or not.  If so, then don't
	// process
	if ( objp->flags & P_SF_REINFORCEMENT ) {

		// if this ship did arrive, mark the reinforcement as available, and tell clients if in multiplayer
		// mode
		if ( should_arrive ) {
			mission_parse_mark_reinforcement_available(objp->name);
		}
		return -1;
	}

	if ( should_arrive ) { 		// has the arrival criteria been met?
		int object_num;		

		Assert ( !(objp->flags & P_SF_CANNOT_ARRIVE) );		// get allender

		// check to see if the delay field <= 0.  if so, then create a timestamp and then maybe
		// create the object
		if ( objp->arrival_delay <= 0 ) {
			objp->arrival_delay = timestamp( -objp->arrival_delay * 1000 );
			Assert( objp->arrival_delay >= 0 );
		}
		
		// if the timestamp hasn't elapsed, move onto the next ship.
		if ( !timestamp_elapsed(objp->arrival_delay) )
			return -1;

		// check to see if this ship is to arrive via a docking bay.  If so, and the ship to arrive from
		// doesn't exist, don't create.
		if ( objp->arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			int shipnum;
			char *name;

			Assert( objp->arrival_anchor >= 0 );
			name = Parse_names[objp->arrival_anchor];
	
			// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
			if ( mission_parse_get_arrival_ship( name ) )
				return -1;

			// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
			// it is not on the arrival list (as shown by above if statement).
			shipnum = ship_name_lookup( name );
			if ( shipnum == -1 ) {
				mission_parse_mark_non_arrival(objp);	// Goober5000
				return -1;
			}

			// Goober5000: aha - also don't create if fighterbay is destroyed
			if (ship_fighterbays_all_destroyed(&Ships[shipnum]))
				return -1;
		}

		// create the ship
		object_num = parse_create_object(objp);

		// since this ship is not in a wing, create a SHIP_ARRIVE entry
		//mission_log_add_entry( LOG_SHIP_ARRIVE, objp->name, NULL );
		Assert(object_num >= 0 && object_num < MAX_OBJECTS);
		
		// Play the music track for an arrival
		if ( !(Ships[Objects[object_num].instance].flags & SF_NO_ARRIVAL_MUSIC) )
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[Objects[object_num].instance].team);
			}
		return object_num;
	}
	/* Goober5000 - this is redundant, especially with the streamlined marking
	else
	{
		// check to see if the arrival cue of this ship is known false -- if so, then remove
		// the parse object from the ship
		if ( Sexp_nodes[objp->arrival_cue].value == SEXP_KNOWN_FALSE )
			objp->flags |= P_SF_CANNOT_ARRIVE;
	}
	*/

	return -1;

}

// Goober5000
void mission_maybe_make_ship_arrive(p_object *p_objp)
{
	// try to create ship
	int objnum = mission_did_ship_arrive(p_objp);
	if (objnum < 0)
		return;

	// remove from arrival list
	if (p_objp == Arriving_support_ship)
		mission_parse_support_arrived(objnum);
	else
		list_remove(&Ship_arrival_list, p_objp);
}

// Goober5000
void mission_parse_mark_non_arrival(p_object *p_objp)
{
	// mark the flag
	p_objp->flags |= P_SF_CANNOT_ARRIVE;
}

// Goober5000
void mission_parse_mark_non_arrival(wing *wingp)
{
	int wingnum = WING_INDEX(wingp);

	// look through all ships yet to arrive...
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		// ...and mark the ones in this wing
		if (p_objp->wingnum == wingnum)
			p_objp->flags |= P_SF_CANNOT_ARRIVE;
	}	
}

// Goober5000 - now only called upon mission start
// function to set a flag on all parse objects on ship arrival list which cannot
// arrive in the mission
void mission_parse_mark_non_arrivals()
{
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (p_objp->wingnum != -1)
		{
			if (Sexp_nodes[Wings[p_objp->wingnum].arrival_cue].value == SEXP_KNOWN_FALSE)
				p_objp->flags |= P_SF_CANNOT_ARRIVE;
		}
		else
		{
			if (Sexp_nodes[p_objp->arrival_cue].value == SEXP_KNOWN_FALSE)
				p_objp->flags |= P_SF_CANNOT_ARRIVE;
		}
	}
}

// function to deal with support ship arrival.  objnum is the object number of the arriving support
// ship.  This function can get called from either single or multiplayer.  Needed to that clients
// can know when to abort rearm.
void mission_parse_support_arrived( int objnum )
{
	int i;

	// when the support ship arrives, the shipname it is supposed to repair is in the 'misc'
	// field of the parse object.  If the ship still exists, call ai function which actually
	// issues the goal for the repair
	for ( i = 0; i < Num_arriving_repair_targets; i++ ) {
		int shipnum;

		shipnum = ship_name_lookup( Arriving_repair_targets[i] );

		if ( shipnum != -1 ) {
			object *requester_objp, *support_objp;

			support_objp = &Objects[objnum];
			requester_objp = &Objects[Ships[shipnum].objnum];
			ai_add_rearm_goal( requester_objp, support_objp );
		}
	}

	Ships[Objects[objnum].instance].flags |= SF_WARPED_SUPPORT;

	Arriving_support_ship = NULL;
	Num_arriving_repair_targets = 0;
}

// Goober5000
int parse_object_on_arrival_list(p_object *pobjp)
{
	return (pobjp->next != NULL) && (pobjp->prev != NULL);
}

// mission_eval_arrivals will check the lists of arriving ships and wings,
// creating new ships/wings if the arrival criteria have been met
void mission_eval_arrivals()
{
	int i;
	int rship = -1;
	wing *wingp;

	// before checking arrivals, check to see if we should play a message concerning arrivals
	// of other wings.  We use the timestamps to delay the arrival message slightly for
	// better effect
	if (timestamp_valid(Arrival_message_delay_timestamp) && timestamp_elapsed(Arrival_message_delay_timestamp) && !((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)))
	{
		int rship, use_terran_cmd;

		// use terran command 25% of time
		use_terran_cmd = ((frand() - 0.75) > 0.0f)?1:0;

		rship = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED );
		if ((rship < 0) || use_terran_cmd)
			message_send_builtin_to_player(MESSAGE_ARRIVE_ENEMY, NULL, MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1);
		else if (rship >= 0)
			message_send_builtin_to_player(MESSAGE_ARRIVE_ENEMY, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1);

		Arrival_message_delay_timestamp = timestamp(-1);		// make the stamp invalid
	}

//	if ( !timestamp_elapsed(Mission_arrival_timestamp) )
//		return;

	// check the arrival list
	// Goober5000 - we can't run through the list the usual way because we might
	// remove a bunch of objects and completely screw up the list linkage
	for (i = 0; i < Num_parse_objects; i++)
	{
		p_object *pobjp = &Parse_objects[i];

		// make sure we're on the arrival list
		if (!parse_object_on_arrival_list(pobjp))
			continue;

		// if this object has a wing, don't create it -- let code for wings determine if it should be created
		if (pobjp->wingnum >= 0)
			continue;

		// make it arrive
		mission_maybe_make_ship_arrive(pobjp);
	}

	// check the support ship arrival list
	if (Arriving_support_ship)
	{
		// make it arrive (support ships are not put on the arrival list)
		mission_maybe_make_ship_arrive(Arriving_support_ship);
	}

	// Goober5000 - it's not very efficient to do it this way; I changed it to use
	// mission_parse_mark_non_arrival(object or wing) when something is invalidated
	//mission_parse_mark_non_arrivals();			// mark parse objects which can no longer arrive


	// we must also check to see if there are waves of a wing that must
	// reappear if all the ships of the current wing have been destroyed or
	// have departed. If this is the case, then create the next wave.
	for (i = 0; i < Num_wings; i++)
	{
		wingp = &Wings[i];

		// should we process this wing anymore
		if (wingp->flags & WF_WING_GONE)
			continue;

		// if we have a reinforcement wing, then don't try to create new ships automatically.
		if (wingp->flags & WF_REINFORCEMENT)
		{
			// check to see in the wings arrival cue is true, and if so, then mark the reinforcement
			// as available
			if (eval_sexp(wingp->arrival_cue))
				mission_parse_mark_reinforcement_available(wingp->name);

			// reinforcement wings skip the rest of the loop
			continue;
		}
		
		// don't do evaluations for departing wings
		if (wingp->flags & WF_WING_DEPARTING)
			continue;

		// must check to see if we are at the last wave.  Code above to determine when a wing is gone only
		// gets run when a ship is destroyed (not every N seconds like it used to).  Do a quick check here.
		if (wingp->current_wave == wingp->num_waves)
			continue;

		// If the current wave of this wing is 0, then we haven't created the ships in the wing yet.
		// If the threshold of the wing has been reached, then we need to create more ships.
		if ((wingp->current_wave == 0) || (wingp->current_count <= wingp->threshold))
		{
			// Call parse_wing_create_ships to try and create it.  That function will eval the arrival
			// cue of the wing and create the ships if necessary.
			int created = parse_wing_create_ships(wingp, wingp->wave_count);

			// if we didn't create any ships, nothing more to do for this wing
			if (created <= 0)
				continue;

			// If this wing was a reinforcement wing, then we need to reset the reinforcement flag for the wing
			// so the user can call in another set if need be.
			if (wingp->flags & WF_RESET_REINFORCEMENT)
			{
				wingp->flags &= ~WF_RESET_REINFORCEMENT;
				wingp->flags |= WF_REINFORCEMENT;
			}

			// probably send a message to the player when this wing arrives.
			// if no message, nothing more to do for this wing
			if (wingp->flags & WF_NO_ARRIVAL_MESSAGE)
				continue;

			// multiplayer team vs. team
			if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
			{
				// send a hostile wing arrived message
				rship = wingp->ship_index[wingp->special_ship];

				int multi_team_filter = Ships[rship].team;

				// there are two timestamps at work here.  One to control how often the player receives
				// messages about incoming hostile waves, and the other to control how long after
				// the wing arrives does the player actually get the message.
				if (timestamp_elapsed(Allow_arrival_message_timestamp_m[multi_team_filter]))
				{
					if (!timestamp_valid(Arrival_message_delay_timestamp_m[multi_team_filter]))
					{
						Arrival_message_delay_timestamp_m[multi_team_filter] = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX);
					}
					Allow_arrival_message_timestamp_m[multi_team_filter] = timestamp(ARRIVAL_MESSAGE_MIN_SEPARATION);
						
					// send to the proper team
					message_send_builtin_to_player(MESSAGE_ARRIVE_ENEMY, NULL, MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter);
				}
			}
			// does the player attack this ship?
			else if (iff_x_attacks_y(Player_ship->team, Ships[wingp->ship_index[0]].team))
			{
				// there are two timestamps at work here.  One to control how often the player receives
				// messages about incoming hostile waves, and the other to control how long after
				// the wing arrives does the player actually get the message.
				if (timestamp_elapsed(Allow_arrival_message_timestamp))
				{
					if (!timestamp_valid(Arrival_message_delay_timestamp))
					{
						Arrival_message_delay_timestamp = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX);
					}
					Allow_arrival_message_timestamp = timestamp(ARRIVAL_MESSAGE_MIN_SEPARATION);
				}
			}
			// everything else
			else
			{
				rship = ship_get_random_ship_in_wing(i, SHIP_GET_UNSILENCED);
				if (rship >= 0)
				{
					int j;
					char message_name[NAME_LENGTH + 10];
					sprintf(message_name, "%s Arrived", wingp->name);

					// see if this wing has an arrival message associated with it
					for (j = 0; j < MAX_BUILTIN_MESSAGE_TYPES; j++)
					{
						if (!stricmp(message_name, Builtin_message_types[j]))
						{
							message_send_builtin_to_player(j, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1);
							break;
						}
					}
				}
			}
		}
	}

	Mission_arrival_timestamp = timestamp(ARRIVAL_TIMESTAMP);
}

// Goober5000
// this only checks the warp drive; we might be able to depart some other way (e.g. by entering a docking bay)
int ship_can_use_warp_drive(ship *shipp)
{
	// must *have* a subspace drive
	if (shipp->flags2 & SF2_NO_SUBSPACE_DRIVE)
		return 0;

	// navigation must work
	if (!ship_navigation_ok_to_warp(shipp))
		return 0;

	return 1;
}

// called to make object objp depart.
int mission_do_departure(object *objp)
{
	Assert (objp->type == OBJ_SHIP);
	int location, anchor, path_mask;
	ship *shipp = &Ships[objp->instance];

	// Goober5000 - if this is a ship which has no subspace drive, departs to hyperspace, and belongs to a wing,
	// then use the wing departure information
	if ((shipp->flags2 & SF2_NO_SUBSPACE_DRIVE) && (shipp->departure_location == DEPART_AT_LOCATION) && (shipp->wingnum >= 0))
	{
		wing *wingp = &Wings[shipp->wingnum];

		location = wingp->departure_location;
		anchor = wingp->departure_anchor;
		path_mask = wingp->departure_path_mask;
	}
	else
	{
		location = shipp->departure_location;
		anchor = shipp->departure_anchor;
		path_mask = shipp->departure_path_mask;
	}

	// if departing to a docking bay, try to find the anchor ship to depart to.  If not found, then
	// just make it warp out like anything else.
	if (location == DEPART_AT_DOCK_BAY)
	{
		int anchor_shipnum;
		char *name;

		Assert(anchor >= 0);
		name = Parse_names[anchor];

		// see if ship is yet to arrive.  If so, then warp.
		if (mission_parse_get_arrival_ship(name))
		{
			goto try_to_warp;
		}

		// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
		// it is not on the arrival list (as shown by above if statement).
		anchor_shipnum = ship_name_lookup(name);
		if (anchor_shipnum < 0)
		{
			goto try_to_warp;
		}

//		if ( ( aip->goal_signature != Objects[aip->goal_objnum].signature) )
//		{
//		}

		// make sure ship not dying or departing
		if (Ships[anchor_shipnum].flags & (SF_DYING | SF_DEPARTING))
		{
			goto try_to_warp;
		}

		// make sure fighterbays aren't destroyed
		if (ship_fighterbays_all_destroyed(&Ships[anchor_shipnum]))
		{
			goto try_to_warp;
		}

		// find a path
		if (ai_acquire_depart_path(objp, Ships[anchor_shipnum].objnum, path_mask) >= 0)
		{
			MONITOR_INC(NumShipDepartures,1);

			return 1;
		}
	}

try_to_warp:
	ai_info *aip = &Ai_info[shipp->ai_index];

	// Goober5000 - make sure we can actually warp
	if (ship_can_use_warp_drive(shipp))
	{
		ai_set_mode_warp_out(objp, aip);
		MONITOR_INC(NumShipDepartures,1);

		return 1;
	}
	else
	{
		shipp->flags &= ~SF_DEPARTING;

		// find something else to do
		aip->mode = AIM_NONE;

		return 0;
	}
}

// put here because mission_eval_arrivals is here.  Might move these to a better location
// later -- MWA
void mission_eval_departures()
{
	int i, j;
	object *objp;
	wing *wingp;

//	if ( !timestamp_elapsed(Mission_departure_timestamp) )
//		return;

	// scan through the active ships an evaluate their departure cues.  For those
	// ships whose time has come, set their departing flag.

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_SHIP) {
			ship	*shipp;

			Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

			shipp = &Ships[objp->instance];
			
			// don't process a ship that is already departing or dying or disabled
			// AL 12-30-97: Added SF_CANNOT_WARP to check
			// Goober5000 - fixed so that it WILL eval when SF_CANNOT_WARP if departing to dockbay
			if ( (shipp->flags & (SF_DEPARTING | SF_DYING)) || ((shipp->flags & SF_CANNOT_WARP) && (shipp->departure_location != DEPART_AT_DOCK_BAY)) || ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) ) {
				continue;
			}

			// don't process ships that are part of a wing -- handled in seperate case
			if ( shipp->wingnum != -1 )
				continue;

//				&& (!timestamp_valid(shipp->departure_delay) || timestamp_elapsed(shipp->departure_delay)) )
			// when the departure cue becomes true, set off the departure delay timer.  We store the
			// timer as -seconds in FreeSpace which indicates that the timer has not been set.  If the timer
			// is not set, then turn it into a valid timer and keep evaluating the timer until it is elapsed
			if ( eval_sexp(shipp->departure_cue) ) {
				if ( shipp->departure_delay <= 0 )
					shipp->departure_delay = timestamp(-shipp->departure_delay * 1000 );
				if ( timestamp_elapsed(shipp->departure_delay) )
					mission_do_departure( objp );
			}
		}
	}

	// now scan through the list of wings and check their departure cues.  For wings with
	// that cue being true, we must update internal variables to indicate that the wing is
	// departed and that no further waves of this wing will appear

	for ( i = 0; i < Num_wings; i++ ) {
		wingp = &Wings[i];

		// should we process this wing anymore
		if ( wingp->flags & WF_WING_DEPARTING )
			continue;

		// evaluate the sexpression.  If true, mark all the ships in this wing as departing and increment
		// the num departed in the wing structure.  Then add number of remaining waves * ships/wave to
		// departed count to get total count of ships in the wing which departed.  (We are counting ships
		// that have not yet arrived as departed if they never arrive -- this may be bad, but for some reason
		// seems like the right thing to do).
 //&& (!timestamp_valid(wingp->departure_delay) || timestamp_elapsed(wingp->departure_delay)) ) {

		if ( eval_sexp(wingp->departure_cue) ) {
			// if we haven't set up the departure timer yet (would be <= 0) setup the timer to pop N seconds
			// later
			if ( wingp->departure_delay <= 0 )
				wingp->departure_delay = timestamp( -wingp->departure_delay * 1000 );
			if ( !timestamp_elapsed(wingp->departure_delay) )
				continue;

			wingp->flags |= WF_WING_DEPARTING;
			for ( j = 0; j < wingp->current_count; j++ ) {
				ship *shipp;

				shipp = &Ships[wingp->ship_index[j]];
				if ( (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_DYING) )
					continue;

//				shipp->flags |= SF_DEPARTING;
//				shipp->final_depart_time = timestamp(3*1000);

				Assert ( shipp->objnum != -1 );
				objp = &Objects[shipp->objnum];
				
				// copy the wing's departure information to the ship
				shipp->departure_location = Wings[shipp->wingnum].departure_location;
				shipp->departure_anchor = Wings[shipp->wingnum].departure_anchor;
				shipp->departure_path_mask = Wings[shipp->wingnum].departure_path_mask;

				mission_do_departure( objp );
				// don't add to wingp->total_departed here -- this is taken care of in ship code.
			}

			// MWA 2/25/98 -- don't do the follwoing wing member updates.  It makes the accurate counts
			// sort of messed up and causes problems for the event log.  The code in ship_wing_cleanup()
			// now keys off of the WF_WING_DEPARTING flag instead of the counts below.

			/*
			// now be sure that we update wing structure members if there are any remaining waves left
			if ( wingp->current_wave < wingp->num_waves ) {
				int num_remaining;

				num_remaining = ( (wingp->num_waves - wingp->current_wave) * wingp->wave_count);
				wingp->total_departed += num_remaining;
				wingp->total_arrived_count += num_remaining;
				wingp->current_wave = wingp->num_waves;
			}
			*/

		}
	}
	Mission_departure_timestamp = timestamp(DEPARTURE_TIMESTAMP);
}

// function called from high level game loop to do mission evaluation stuff
void mission_parse_eval_stuff()
{
	mission_eval_arrivals();
	mission_eval_departures();
}

int allocate_subsys_status()
{
	int i;
	// set primary weapon ammunition here, but does it actually matter? - Goober5000

	Assert(Subsys_index >= 0 /*&& Subsys_index < MAX_SUBSYS_STATUS*/);

	// we allocate in blocks of MIN_SUBSYS_STATUS_SIZE so if we need more then make more
	if ( (Subsys_status == NULL) || (Subsys_index >= (Subsys_status_size - 1)) ) {
		Assert( MIN_SUBSYS_STATUS_SIZE > 0 );

		Subsys_status_size += MIN_SUBSYS_STATUS_SIZE;
		Subsys_status = (subsys_status*)vm_realloc(Subsys_status, sizeof(subsys_status) * Subsys_status_size );
	}

	Verify( Subsys_status != NULL );

	memset( &Subsys_status[Subsys_index], 0, sizeof(subsys_status) );

	Subsys_status[Subsys_index].percent = 0.0f;

	Subsys_status[Subsys_index].primary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].primary_ammo[0] = 100; // *
	
	for (i=1; i<MAX_SHIP_PRIMARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].primary_banks[i] = -1;  // none
		Subsys_status[Subsys_index].primary_ammo[i] = 100;	// *
	}

	Subsys_status[Subsys_index].secondary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].secondary_ammo[0] = 100;
	
	for (i=1; i<MAX_SHIP_SECONDARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].secondary_banks[i] = -1;
		Subsys_status[Subsys_index].secondary_ammo[i] = 100;
	}

	Subsys_status[Subsys_index].ai_class = SUBSYS_STATUS_NO_CHANGE;
	return Subsys_index++;
}

// find (or add) the name in the list and return an index to it.
int get_parse_name_index(char *name)
{
	int i;

	for (i=0; i<Num_parse_names; i++)
		if (!stricmp(name, Parse_names[i]))
			return i;

	Assert(i < MAX_SHIPS + MAX_WINGS);
	Assert(strlen(name) < NAME_LENGTH);
	strcpy(Parse_names[i], name);
	return Num_parse_names++;
}

// Goober5000
int add_path_restriction()
{
	int i, j;

	// parse it
	path_restriction_t temp;
	temp.cached_mask = (1 << MAX_SHIP_BAY_PATHS);	// uninitialized value (too high)
	temp.num_paths = stuff_string_list(temp.path_names, MAX_SHIP_BAY_PATHS);

	// no restriction?
	if (temp.num_paths == 0)
		return -1;

	// first, see if it's duplicated anywhere
	for (i = 0; i < Num_path_restrictions; i++)
	{
		// must have same number of allowed paths
		if (temp.num_paths != Path_restrictions[i].num_paths)
			continue;

		// see if path names match
		for (j = 0; j < temp.num_paths; j++)
		{
			// no match, so skip this
			if (stricmp(temp.path_names[j], Path_restrictions[i].path_names[j]))
				goto continue_outer_loop;
		}

		// match!
		return i;

continue_outer_loop:
		;
	}

	// no match, so add a new restriction

	// check limit
	if (Num_path_restrictions >= MAX_PATH_RESTRICTIONS)
	{
		Warning(LOCATION, "Maximum number of path restrictions reached");
		return -1;
	}

	// add this restriction at the new index
	int index = Num_path_restrictions++;
	Path_restrictions[index] = temp;

	return index;
}

// Goober5000 - look for <any friendly>, <any hostile player>, etc.
int get_special_anchor(char *name)
{
	char tmp[NAME_LENGTH + 15];
	char *iff_name;
	int iff_index;
	
	if (strnicmp(name, "<any ", 5))
		return -1;

	strcpy(tmp, name+5);
	iff_name = strtok(tmp, " >");

	// hack substitute "hostile" for "enemy"
	if (!stricmp(iff_name, "enemy"))
		iff_name = "hostile";

	iff_index = iff_lookup(iff_name);
	if (iff_index < 0)
		return -1;

	// restrict to players?
	if (stristr(name+5, "player") != NULL)
		return (iff_index | SPECIAL_ARRIVAL_ANCHOR_FLAG | SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG);
	else
		return (iff_index | SPECIAL_ARRIVAL_ANCHOR_FLAG);
}

int get_anchor(char *name)
{
	int special_anchor = get_special_anchor(name);

	if (special_anchor >= 0)
		return special_anchor;

	return get_parse_name_index(name);
}

// function to fixup the goals/ai references for player objects in the mission
void mission_parse_fixup_players()
{
	object *objp;

	// merge created list to have all objects on used list
	obj_merge_created_list();
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) && (objp->flags & OF_PLAYER_SHIP) ) {
			game_busy( NOX("** fixing up player/ai stuff **") );	// animate the loading screen, doesn't nothing if the screen is not active
			ai_clear_ship_goals( &Ai_info[Ships[objp->instance].ai_index] );
			init_ai_object( OBJ_INDEX(objp) );
		}
	}
}

// code to warp in a new support ship.  It works by finding the average position of all ships
// in the mission, creating a vector from that position to the player, and scaling out behind the
// player some distance.  Should be sufficient.

#define WARP_IN_MIN_DISTANCE	1000.0f
#define WARP_IN_TIME_MIN		3000				// warps in min 3 seconds later
#define WARP_IN_TIME_MAX		6000				// warps in max 6 seconds later

// function which adds requester_objp onto the queue of ships for the arriving support ship to service
void mission_add_to_arriving_support( object *requester_objp )
{
	int i;
	ship *shipp;

	Assert ( Arriving_support_ship );

	if ( Num_arriving_repair_targets == MAX_AI_GOALS ) {
		// Int3();			// get allender -- ship isn't going to get repair, but I hope they never queue up this far!!!
		mprintf(("Reached MAX_AI_GOALS trying to add repair request!\n"));
		return;
	}

	shipp = &Ships[requester_objp->instance];
	// check for duplicates before adding
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !stricmp(Arriving_repair_targets[i], shipp->ship_name) ){
			break;
		}
	}
	if ( i != Num_arriving_repair_targets ){		// found the ship before reaching the end -- ignore it!
		return;
	}

	strcpy( Arriving_repair_targets[Num_arriving_repair_targets], Ships[requester_objp->instance].ship_name );
	Num_arriving_repair_targets++;

	if ( MULTIPLAYER_MASTER ){
		multi_maybe_send_repair_info( requester_objp, NULL, REPAIR_INFO_WARP_ADD );
	}	
}

extern int pp_collide_any(vec3d *curpos, vec3d *goalpos, float radius, object *ignore_objp1, object *ignore_objp2, int big_only_flag);

//	Set the warp in position for a support ship relative to an object.
//	Caller tries several positions, passing vector in x, y, z.
int get_warp_in_pos(vec3d *pos, object *objp, float x, float y, float z)
{
	float	rand_val;

	if ( Game_mode & GM_NORMAL )
		rand_val = frand();
	else
		rand_val = static_randf(objp->net_signature);

	rand_val = 1.0f + (rand_val - 0.5f)*0.2f;

	*pos = objp->pos;

	vm_vec_scale_add2( pos, &objp->orient.vec.rvec, x*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.uvec, y*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.fvec, z*rand_val*800.0f);

	return pp_collide_any(&objp->pos, pos, objp->radius, objp, NULL, 1);
}

// modified by Goober5000 to allow more flexibility in support ships
void mission_bring_in_support_ship( object *requester_objp )
{
	vec3d center, warp_in_pos;
	//float mag;
	p_object *pobj;
	ship *requester_shipp;
	int i, requester_species;

	Assert ( requester_objp->type == OBJ_SHIP );
	requester_shipp = &Ships[requester_objp->instance];	//	MK, 10/23/97, used to be ->type, bogus, no?

	// if the support ship is already arriving, add the requester to the list
	if ( Arriving_support_ship ) {
		mission_add_to_arriving_support( requester_objp );
		return;
	}
	
	// create a parse object, and put it onto the ship arrival list.  This whole thing kind of stinks.
	// I want to put it into a parse object since it needs to arrive just a little later than
	// this function is called.  I have to make some assumptions in the code about values for the parse
	// object since I'm no longer working with a mission file.  These exceptions will be noted with
	// comments

	Arriving_support_ship = &Support_ship_pobj;
	pobj = Arriving_support_ship;

	// get average position of all ships
	obj_get_average_ship_pos( &center );
	vm_vec_sub( &warp_in_pos, &center, &(requester_objp->pos) );

	// be sure to account for case as player being only ship left in mission
	/*
	if ( !(IS_VEC_NULL( warp_in_pos)) ) {
		mag = vm_vec_mag( &warp_in_pos );
		if ( mag < WARP_IN_MIN_DISTANCE )
			vm_vec_scale( &warp_in_pos, WARP_IN_MIN_DISTANCE/mag);
		else
			vm_vec_scale( &warp
	} else {
		// take -player_pos.vec.fvec scaled by 1000.0f;
		warp_in_pos = Player_obj->orient.vec.fvec;
		vm_vec_scale( &warp_in_pos, -1000.0f );
	}
	*/

	//	Choose position to warp in ship.
	//	Temporary, but changed by MK because it used to be exactly behind the player.
	//	This could cause an Assert if the player immediately targeted it (before moving).
	//	Tend to put in front of the player to aid him in flying towards the ship.

	if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.1f, 1.0f))
		if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.2f, -1.0f))
			if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.2f, -1.0f))
				if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.1f, 1.0f))
					get_warp_in_pos(&warp_in_pos, requester_objp, 0.1f, 1.0f, 0.2f);

	// position for ship if it warps in
	pobj->pos = warp_in_pos;

	// tally the ship
	The_mission.support_ships.tally++;

	// create a name for the ship.  use "Support #".  look for collisions until one isn't found anymore
	i = 1;
	do {
		sprintf(pobj->name, NOX("Support %d"), i);
		if ( (ship_name_lookup(pobj->name) == -1) && (ship_find_exited_ship_by_name(pobj->name) == -1) )
			break;
		i++;
	} while(1);

	vm_set_identity( &(pobj->orient) );

	// *sigh*.  Gotta get the ship class.  For now, this will amount to finding a ship in the ship_info
	// array with the same team as the requester of type SIF_SUPPORT.  Might need to be changed, but who knows

	// Goober5000 - who knew of the SCP release? ;) only determine ship class if not set by SEXP
	pobj->ship_class = The_mission.support_ships.ship_class;
	if (pobj->ship_class < 0)
	{
		requester_species = Ship_info[requester_shipp->ship_info_index].species;

		// 5/6/98 -- MWA  Don't need to do anything for multiplayer.  I think that we always want to use
		// the species of the caller ship.

		// get index of correct species support ship
		for (i=0; i < Num_ship_classes; i++) {
			if ( (Ship_info[i].species == requester_species) && (Ship_info[i].flags & SIF_SUPPORT) )
				break;
		}

		if ( i < Num_ship_classes )
			pobj->ship_class = i;
		else
			Int3();				// BOGUS!!!!  gotta figure something out here
	}

	// set support ship hitpoints
	pobj->parse_max_hull_strength = Ship_info[i].max_hull_strength;
	pobj->parse_max_shield_strength = Ship_info[i].max_shield_strength;

	pobj->team = requester_shipp->team;

	pobj->behavior = AIM_NONE;		// ASSUMPTION:  the mission file has the string "None" which maps to AIM_NONE

	// set the ai_goals to -1.  We will put the requester object shipname in repair target array and then take
	// care of setting up the goal when creating the ship!!!!
	pobj->ai_goals = -1;
	Num_arriving_repair_targets = 0;
	mission_add_to_arriving_support( requester_objp );

	// need to set ship's cargo to nothing.  scan the cargo_names array looking for the string nothing.
	// add it if not found
	for (i = 0; i < Num_cargo; i++ )
		if ( !stricmp(Cargo_names[i], NOX("nothing")) )
			break;

	if ( i == Num_cargo ) {
		strcpy(Cargo_names[i], NOX("Nothing"));
		Num_cargo++;
	}
	pobj->cargo1 = char(i);

	pobj->status_count = 0;

	// Goober5000 - take some stuff from mission flags
	pobj->arrival_location = The_mission.support_ships.arrival_location;
	pobj->arrival_anchor = The_mission.support_ships.arrival_anchor;
	pobj->departure_location = The_mission.support_ships.departure_location;
	pobj->departure_anchor = The_mission.support_ships.departure_anchor;

	pobj->arrival_path_mask = 0;
	pobj->departure_path_mask = 0;

//	pobj->arrival_location = ARRIVE_AT_LOCATION;
	pobj->arrival_distance = 0;
//	pobj->arrival_anchor = -1;
	pobj->arrival_cue = Locked_sexp_true;
	pobj->arrival_delay = timestamp_rand(WARP_IN_TIME_MIN, WARP_IN_TIME_MAX);

	pobj->subsys_count = 0;				// number of elements used in subsys_status array
	pobj->initial_velocity = 100;		// start at 100% velocity
	pobj->initial_hull = 100;			// start at 100% hull	
	pobj->initial_shields = 100;		// and 100% shields

//	pobj->departure_location = DEPART_AT_LOCATION;
//	pobj->departure_anchor = -1;
	pobj->departure_cue = Locked_sexp_false;
	pobj->departure_delay = 0;

	pobj->determination = 10;			// ASSUMPTION:  mission file always had this number written out
	pobj->wingnum = -1;

	pobj->flags = 0;
	pobj->flags2 = 0;

	if (The_mission.support_ships.ssi_flags & SSIF_NO_SHIELDS)
		pobj->flags |= P_OF_NO_SHIELDS;

	if ( Ships[Player_obj->instance].flags2 & SF2_NO_SUBSPACE_DRIVE )
		pobj->flags2 |= P2_SF2_NO_SUBSPACE_DRIVE;	// support ships have no subspace drive when player has not subspace drive

	pobj->ai_class = Ship_info[pobj->ship_class].ai_class;
	pobj->hotkey = -1;
	pobj->score = 0;

	pobj->dock_list = NULL;
	pobj->created_object = NULL;
	pobj->group = -1;
	pobj->persona_index = -1;
	pobj->net_signature = multi_assign_network_signature(MULTI_SIG_SHIP);
	pobj->wing_status_wing_index = -1;
	pobj->wing_status_wing_pos = -1;
	pobj->respawn_count = 0;
	pobj->alt_type_index = -1;
	pobj->num_texture_replacements = 0;
}

// returns true if a support ship is currently in the process of warping in.
int mission_is_support_ship_arriving()
{
	if ( Arriving_support_ship )
		return 1;
	else
		return 0;
}

// returns true if the given ship is scheduled to be repaired by the arriving support ship
int mission_is_repair_scheduled( object *objp )
{
	char *name;
	int i;

	if ( !Arriving_support_ship )
		return 0;

	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !strcmp( name, Arriving_repair_targets[i]) )
			return 1;
	}

	return 0;
}

// function which removed the given ship from the list of ships that are to get repair
// by arriving support ship
int mission_remove_scheduled_repair( object *objp )
{
	char *name;
	int i, index;

	if ( !Arriving_support_ship )
		return 0;

	// itereate through the target list looking for this ship name.  If not found, we
	// can simply return.
	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (index = 0; index < Num_arriving_repair_targets; index++ ) {
		if ( !strcmp( name, Arriving_repair_targets[index]) )
			break;
	}
	if ( index == Num_arriving_repair_targets )
		return 0;

	// ship is found -- compress the array
	for ( i = index; i < Num_arriving_repair_targets - 1; i++ )
		strcpy( Arriving_repair_targets[i], Arriving_repair_targets[i+1] );

	Num_arriving_repair_targets--;

	if ( MULTIPLAYER_MASTER )
		multi_maybe_send_repair_info( objp, NULL, REPAIR_INFO_WARP_REMOVE );

	return 1;
}

// alternate name stuff
int mission_parse_lookup_alt(char *name)
{
	int idx;

	// sanity
	if(name == NULL){
		return -1;
	}

	// lookup
	for(idx=0; idx<Mission_alt_type_count; idx++){
		if(!strcmp(Mission_alt_types[idx], name)){
			return idx;
		}
	}

	// could not find
	return -1;
}

static int mission_parse_lookup_alt_index_warn = 1;
void mission_parse_lookup_alt_index(int index, char *out)
{
	// sanity
	if(out == NULL){
		return;
	}
	if((index < 0) || (index > Mission_alt_type_count)){
		if (mission_parse_lookup_alt_index_warn) {
			Warning(LOCATION, "Ship with invalid alt_name.  Get a programmer");
			mission_parse_lookup_alt_index_warn = 0;
		}
		return;
	}

	// stuff it
	strcpy(out, Mission_alt_types[index]);
}

int mission_parse_add_alt(char *name)
{
	// sanity
	if(name == NULL){
		return -1;
	}

	// maybe add
	if(Mission_alt_type_count < MAX_ALT_TYPE_NAMES){
		// stuff the name
		strncpy(Mission_alt_types[Mission_alt_type_count++], name, NAME_LENGTH);

		// done
		return Mission_alt_type_count - 1;
	}

	return -1;
}

void mission_parse_reset_alt()
{
	Mission_alt_type_count = 0;
}

int is_training_mission()
{
	return (The_mission.game_type & MISSION_TYPE_TRAINING);
}

// Goober5000
// go through all the displayed text in one section and fix "
// the section and text delimiters should all be different
void conv_fix_punctuation_section(char *str, char *section_start, char *section_end, char *text_start, char *text_end)
{
	char *s1, *s2, *t1, *t2;

	s1 = strstr(str, section_start);
	s2 = strstr(s1, section_end);

	t1 = s1;
	
	while (1)
	{
		t1 = strstr(t1+1, text_start);
		if (!t1 || t1 > s2) return;

		t2 = strstr(t1, text_end);
		if (!t2 || t2 > s2) return;

		replace_all(t1, "\"", "$quote", MISSION_TEXT_SIZE - (str - Mission_text), (t2 - t1));
	}	
}
	
// Goober5000
void conv_fix_punctuation()
{
	// command briefings
	conv_fix_punctuation_section(Mission_text, "#Command Briefing", "#Briefing", "$Stage Text:", "$end_multi_text");

	// briefings
	conv_fix_punctuation_section(Mission_text, "#Briefing", "#Debriefing_info", "$multi_text", "$end_multi_text");

	// debriefings
	conv_fix_punctuation_section(Mission_text, "#Debriefing_info", "#Players", "$Multi text", "$end_multi_text");

	// messages
	conv_fix_punctuation_section(Mission_text, "#Messages", "#Reinforcements", "$Message:", "\n");
}

// Goober5000
void convertFSMtoFS2()
{
	// fix punctuation
	conv_fix_punctuation();
}

// Goober5000
void restore_default_weapons(char *ships_tbl)
{
	int i, j, si_subsys;
	char *ch, *subsys;
	char ship_class[NAME_LENGTH];
	ship_subsys *ss;
	ship_info *sip;

	// guesstimate that this actually is a ships.tbl
	if (!strstr(ships_tbl, "#Ship Classes"))
	{
		MessageBox(NULL, "This is not a ships.tbl file.  Aborting conversion...", "Error", MB_OK);
		return;
	}

	// for every ship
	for (i = 0; i < MAX_SHIPS; i++)
	{
		// ensure the ship slot is used in this mission
		if (Ships[i].objnum >= 0)
		{
			// get ship_info
			sip = &Ship_info[Ships[i].ship_info_index];

			// find the ship class
			ch = strstr(ships_tbl, ship_class);
			if (!ch) continue;

			// check pbanks (capital ships have these specified but empty)
			Mp = strstr(ch, "$Default PBanks");
			Mp = strchr(Mp, '(');
			restore_one_primary_bank(Ships[i].weapons.primary_bank_weapons, sip->primary_bank_weapons);

			// check sbanks (capital ships have these specified but empty)
			Mp = strstr(ch, "$Default SBanks");
			Mp = strchr(Mp, '(');
			restore_one_secondary_bank(Ships[i].weapons.secondary_bank_weapons, sip->secondary_bank_weapons);

			// see if we have any turrets
			ch = strstr(ch, "$Subsystem");
			for (ss = GET_FIRST(&Ships[i].subsys_list); ss != END_OF_LIST(&Ships[i].subsys_list); ss = GET_NEXT(ss))
			{
				// we do
				if (ss->system_info->type == SUBSYSTEM_TURRET)
				{
					// find it in the ship_info subsys list
					si_subsys = -1;
					for (j = 0; j < sip->n_subsystems; j++)
					{
						if (!subsystem_stricmp(ss->system_info->subobj_name, sip->subsystems[j].subobj_name))
						{
							si_subsys = j;
							break;
						}
					}
					if (si_subsys < 0) continue;

					// find it in the file - make sure it belongs to *this* ship
					subsys = stristr(ch, ss->system_info->subobj_name);
					if (!subsys) continue;
					if (subsys > strstr(ch, "$Name")) continue;

					// check pbanks - make sure they are *this* subsystem's banks
					Mp = strstr(subsys, "$Default PBanks");
					if (Mp < strstr(subsys + 1, "$Subsystem"))
					{
						Mp = strchr(Mp, '(');
						restore_one_primary_bank(ss->weapons.primary_bank_weapons, sip->subsystems[si_subsys].primary_banks);
					}

					// check sbanks - make sure they are *this* subsystem's banks
					Mp = strstr(subsys, "$Default SBanks");
					if (Mp < strstr(subsys + 1, "$Subsystem"))
					{
						Mp = strchr(Mp, '(');
						restore_one_secondary_bank(ss->weapons.secondary_bank_weapons, sip->subsystems[si_subsys].secondary_banks);
					}
				}
			}
		}
	}
}

// Goober5000
void restore_one_primary_bank(int *ship_primary_weapons, int *default_primary_weapons)
{
	int i, count, original_weapon;
	char weapon_list[MAX_SHIP_PRIMARY_BANKS][NAME_LENGTH];

	// stuff weapon list
	count = stuff_string_list(weapon_list, MAX_SHIP_PRIMARY_BANKS);

	// check for default weapons - if same as default, overwrite with the one from the table
	for (i = 0; i < count; i++)
	{
		if (ship_primary_weapons[i] == default_primary_weapons[i])
		{
			if ((original_weapon = weapon_info_lookup(weapon_list[i])) >= 0)
			ship_primary_weapons[i] = original_weapon;
		}
	}
}

// Goober5000
void restore_one_secondary_bank(int *ship_secondary_weapons, int *default_secondary_weapons)
{
	int i, count, original_weapon;
	char weapon_list[MAX_SHIP_SECONDARY_BANKS][NAME_LENGTH];

	// stuff weapon list
	count = stuff_string_list(weapon_list, MAX_SHIP_SECONDARY_BANKS);

	// check for default weapons - if same as default, overwrite with the one from the table
	for (i = 0; i < count; i++)
	{
		if (ship_secondary_weapons[i] == default_secondary_weapons[i])
		{
			if ((original_weapon = weapon_info_lookup(weapon_list[i])) >= 0)
			ship_secondary_weapons[i] = original_weapon;
		}
	}
}
