
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/Ship.cpp $
 * $Revision: 2.425 $
 * $Date: 2007-07-19 05:43:59 $
 * $Author: turey $
 *
 * Ship (and other object) handling functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.424  2007/07/15 08:19:59  Goober5000
 * fix and clean up the warpout conditions
 *
 * Revision 2.423  2007/07/15 06:29:56  Goober5000
 * restore WMC's ship flag
 *
 * Revision 2.422  2007/07/15 04:19:25  Goober5000
 * partial commit of aldo's eyepoint feature
 * it will need a keystroke to be complete
 *
 * Revision 2.421  2007/07/15 02:45:18  Goober5000
 * fixed a small bug in the lab
 * moved WMC's no damage scaling flag to ai_profiles and made it work correctly
 * removed my old supercap damage scaling change
 * moved Turey's truefire flag to ai_profiles
 *
 * Revision 2.420  2007/07/13 22:28:13  turey
 * Initial commit of Training Weapons / Simulated Hull code.
 *
 * Revision 2.419  2007/07/11 20:11:33  turey
 * Ship Template fixes. It's fully working now, hopefully.
 *
 * Revision 2.418  2007/06/22 04:52:21  turey
 * Added in Ship Templates code, with some syntax and warning cleanup.
 *
 * Revision 2.417  2007/05/26 15:12:01  Goober5000
 * add placeholder stuff for parsing ship-class texture replacements
 *
 * Revision 2.416  2007/05/25 13:58:25  taylor
 * add the rest of the texture page-in code changes that I skipped before (should fix Mantis #1389)
 *
 * Revision 2.415  2007/05/17 14:58:11  taylor
 * fix armor_parse_table() so that it will parse multiple entries properly
 * fix bug introduced from not closing down lcl_ext after parsing armor.tbl
 *
 * Revision 2.414  2007/05/14 23:13:49  Goober5000
 * --grouped the shake/shudder code together a bit better
 * --added a sexp to generate shudder
 * --fixed a minor bug in lock-perspective
 *
 * Revision 2.413  2007/05/09 04:16:06  Backslash
 * Add feature to $Max Glide Speed -- negative number means no speed cap
 * Only show gun muzzle flash effect if in not cockpit view, or if "show ship" flag is set
 * Fix a couple typos ;-)
 *
 * Revision 2.412  2007/04/30 21:30:31  Backslash
 * Backslash's big Gliding commit!  Gliding now obeys physics and collisions, and can be modified with thrusters.  Also has a adjustable maximum speed cap.
 * Added a simple glide indicator.  Fixed a few things involving fspeed vs speed during gliding, including maneuvering thrusters and main engine noise.
 *
 * Revision 2.411  2007/04/13 00:28:00  taylor
 * clean out some old code we no longer use/need
 * change warning messages to not print out current tbl name, since at the point those messages show the tbl has long since been parsed
 * add an extra quick-check to ship_get_subsystem_strength() to avoid div if we don't need it
 * make Ship_subsystems[] dynamic (sort of), it's allocated in sized sets now, so we can easily change size while still working with the existing linked-list code
 *
 * Revision 2.410  2007/03/23 01:50:59  taylor
 * bit of cleanup and minor performance tweaks
 * render ship insignia with a bit of alpha to help with blending/lighting
 * dynamic thruster particle limits
 * update for generic_bitmap/anim changes
 * make use of flag_def_list for ship flags rather than a ton of if-else statements
 * use generic_bitmap and generic_anim where possible to faciliate delayed graphics loading (after all ship related tbls are parsed)
 * use VALID_FNAME()
 *
 * Revision 2.409  2007/03/22 20:45:01  taylor
 * cleanup and fix for weapon cycling (Mantis #1298)
 *
 * Revision 2.408  2007/02/27 01:44:48  Goober5000
 * add two features for WCS: specifyable shield/weapon recharge rates, and removal of linked fire penalty
 *
 * Revision 2.407  2007/02/25 03:57:58  Goober5000
 * use dynamic memory instead of a static buffer for ship-specific replacement textures
 *
 * Revision 2.406  2007/02/21 01:44:02  Goober5000
 * remove duplicate model texture replacement
 *
 * Revision 2.405  2007/02/20 04:20:27  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 2.404  2007/02/18 06:17:34  Goober5000
 * revert Bobboau's commits for the past two months; these will be added in later in a less messy/buggy manner
 *
 * Revision 2.403  2007/02/16 23:18:15  Goober5000
 * this should be based on a flag or something, not automatically tied into the power output
 *
 * Revision 2.402  2007/02/11 21:26:39  Goober5000
 * massive shield infrastructure commit
 *
 * Revision 2.401  2007/02/11 06:19:05  Goober5000
 * invert the do-collision flag into a don't-do-collision flag, plus fixed a wee lab bug
 *
 * Revision 2.400  2007/02/10 06:39:43  Goober5000
 * new feature: shield generators that control whether the shield is up
 *
 * Revision 2.399  2007/02/10 03:20:25  Goober5000
 * small tweak
 *
 * Revision 2.398  2007/02/05 08:26:49  wmcoolmon
 * Make an error message prettier
 *
 * Revision 2.397  2007/01/29 03:39:26  Goober5000
 * --fix the empty ship name / U.R.A. Moron bug caused by WMC's commit
 * --properly update a ship score when its class changes (in FRED or via change-ship-class or via ship loadout)
 *
 * Revision 2.396  2007/01/15 01:37:38  wmcoolmon
 * Fix CVS & correct various warnings under MSVC 2003
 *
 * Revision 2.395  2007/01/14 14:03:36  bobboau
 * ok, something aparently went wrong, last time, so I'm commiting again
 * hopefully it should work this time
 * damnit WORK!!!
 *
 * Revision 2.394  2007/01/08 00:50:59  Goober5000
 * remove WMC's limbo code, per our discussion a few months ago
 * this will later be handled by copying ship stats using sexps or scripts
 *
 * Revision 2.393  2007/01/07 21:28:11  Goober5000
 * yet more tweaks to the WCS death scream stuff
 * added a ship flag to force screaming
 *
 * Revision 2.392  2007/01/07 12:59:54  taylor
 * fix thruster 2 length factor tbl entry so that it has the proper name
 *
 * Revision 2.391  2007/01/07 03:08:12  Goober5000
 * fix bug where built-in lament messages were never played
 *
 * Revision 2.390  2006/12/28 01:22:04  Goober5000
 * removed obsolete limits
 *
 * Revision 2.389  2006/12/28 00:59:48  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.388  2006/12/26 18:14:42  Goober5000
 * allow parsing of similar ship copy names properly (Mantis #1178)
 *
 * Revision 2.387  2006/11/28 05:51:05  Goober5000
 * make an error message more descriptive
 *
 * Revision 2.386  2006/11/12 20:01:55  phreak
 * two fixes:
 *
 * 1) Fix for mantis #1099, part 2.
 *
 * 2) when changing the player's ship using debug_cycle_player_ship(), initialize the
 *  primary bank starting capacity so the support ship will rearm it.
 *
 * Revision 2.385  2006/11/06 06:43:58  taylor
 * if a submodel anim fails to start then move directly to ready position flag (set it up to do this, just forgot to do it ;))  (fix for Mantis bug #1133)
 *
 * Revision 2.384  2006/11/06 06:42:22  taylor
 * make glow_point array for thrusters and glow_point_banks dynamic (a proper fix for old Mantis bug #43)
 *
 * Revision 2.383  2006/11/06 06:32:30  taylor
 * updated/fixed modelanim code
 * add ships.tbl subsystem flag ("+fire-down-normals") which will force a turret to fire down it's barrel line (Mantis bug #591)
 *
 * Revision 2.382  2006/11/06 06:19:17  taylor
 * rename set_warp_globals() to model_set_warp_globals()
 * remove two old/unused MR flags (MR_ALWAYS_REDRAW, used for caching that doesn't work; MR_SHOW_DAMAGE, didn't do anything)
 * add MR_FULL_DETAIL to render an object regardless of render/detail box setting
 * change "model_current_LOD" to a global "Interp_detail_level" and the static "Interp_detail_level" to "Interp_detail_level_locked", a bit more descriptive
 * minor bits of cleanup
 * change a couple of vm_vec_scale_add2() calls to just vm_vec_add2() calls in ship.cpp, since that was the final result anyway
 *
 * Revision 2.381  2006/11/06 02:19:58  Goober5000
 * minor bugfixes
 *
 * Revision 2.380  2006/10/24 23:56:49  Goober5000
 * clarified this; it was confusing before
 *
 * Revision 2.379  2006/10/08 02:05:38  Goober5000
 * fix forum links
 *
 * Revision 2.378  2006/10/06 15:23:42  karajorma
 * Fix for Mantis 1092 - (Support ships damage hull)
 *
 * Revision 2.377  2006/10/06 09:55:36  taylor
 * For redalert stored data be sure that dead ships don't come back, and departed ships come back just as they left (Mantis bug #810)
 *
 * Revision 2.376  2006/10/01 18:53:59  Goober5000
 * check that targetview model files exist as well
 *
 * Revision 2.375  2006/09/22 09:22:08  Backslash
 * Ok, maybe this one will be better.  Sorry about that.
 *
 * Revision 2.374  2006/09/21 13:33:27  taylor
 * revert Backslash's commit, *way* too many changes there for a single friggin line
 *
 * Revision 2.372  2006/09/11 06:48:40  taylor
 * fixes for stuff_string() bounds checking
 * stict compiler build fixes
 *
 * Revision 2.371  2006/09/11 06:08:09  taylor
 * make Species_info[] and Asteroid_info[] dynamic
 *
 * Revision 2.370  2006/09/08 06:19:02  taylor
 * fix for Mantis bug #1038 (glow point bank storage being wrong after changing ships in shipselect)
 * fix things that strict compiling balked at (from compiling with -ansi and -pedantic)
 *
 * Revision 2.369  2006/09/04 18:06:37  Goober5000
 * fix macros
 *
 * Revision 2.368  2006/09/04 06:17:26  wmcoolmon
 * Commit of 'new' BTRL FTL effect work
 *
 * Revision 2.367  2006/09/02 23:41:53  Goober5000
 * fix waypoint time to goal
 *
 * Revision 2.366  2006/08/25 21:19:02  karajorma
 * Fix lack of Wingman Status indicator for Zeta wing in TvT games.
 *
 * Revision 2.365  2006/08/19 21:45:18  Goober5000
 * if a modular table ship pof cannot be found, use the original one
 *
 * Revision 2.364  2006/08/18 18:07:03  karajorma
 * More cut & paste errors fixed and removal of the hardcoded warp out animation. Will now actually use the one specified in the table.
 *
 * Revision 2.363  2006/08/18 04:34:54  Goober5000
 * better handling of ballistic rearm sounds
 * --Goober5000
 *
 * Revision 2.362  2006/08/15 20:00:51  karajorma
 * Another typo
 *
 * Revision 2.361  2006/08/09 17:50:15  karajorma
 * Fix Mantis 1009 - A problem with using the Change-ship-model SEXP
 *
 * Revision 2.360  2006/08/03 01:33:56  Goober5000
 * add a second method for specifying ship copies, plus allow the parser to recognize ship class copy names that aren't consistent with the table
 * --Goober5000
 *
 * Revision 2.359  2006/07/28 02:41:35  taylor
 * check first stage warp arrival against all docked objects so we can not render them all if even one is 1st stage
 *
 * Revision 2.358  2006/07/27 10:43:16  karajorma
 * Fixed an error in ballistic weapons where only the server got the correct number of bullets in multiplayer.
 * Fixed a rounding up error in get_max_ammo_count_for_primary_bank()
 *
 * Revision 2.357  2006/07/24 02:09:26  Goober5000
 * fix a subtle and nasty bug
 * --Goober5000
 *
 * Revision 2.356  2006/07/17 01:12:52  taylor
 * make glow point banks dynamic
 *
 * Revision 2.355  2006/07/17 00:10:00  Goober5000
 * stage 2 of animation fix (add base frame time to each ship)
 * --Goober5000
 *
 * Revision 2.354  2006/07/09 06:12:29  Goober5000
 * bah, make sure we're in the correct mode
 *
 * Revision 2.353  2006/07/09 06:07:13  Goober5000
 * tweak max speed function
 * --Goober5000
 *
 * Revision 2.352  2006/07/09 01:55:41  Goober5000
 * consolidate the "for reals" crap into a proper ship flag; also move the limbo flags over to SF2_*; etc.
 * this should fix Mantis #977
 * --Goober5000
 *
 * Revision 2.351  2006/07/08 04:53:29  Goober5000
 * fix for Mantis #967
 * --Goober5000
 *
 * Revision 2.350  2006/07/06 22:00:39  taylor
 * rest of the map/glow changes
 *  - put glowmap activity back on a per-ship basis (via a SF2_* flag) rather than per-model
 *  - same for glowpoints, back on a per-ship basis
 *  - put specmaps and bumpmap back on a LOD0 and LOD1 affect (got changed to LOD0 only recently)
 *  - fix glowmaps for shockwaves again
 *  - add support for animated specmaps (mainly for TBP and Starfox mods)
 * some minor code cleanup and compiler warning fixes
 *
 * Revision 2.349  2006/07/06 21:24:36  Goober5000
 * fix ship type bug that Taylor mentioned
 * --Goober5000
 *
 * Revision 2.348  2006/07/06 20:46:39  Goober5000
 * WCS screaming stuff
 * --Goober5000
 *
 * Revision 2.347  2006/07/06 04:26:00  Goober5000
 * fix a couple of typos
 * --Goober5000
 *
 * Revision 2.346  2006/07/06 04:06:04  Goober5000
 * 1) complete (almost) changeover to reorganized texture mapping system
 * 2) finally fix texture animation; textures now animate at the correct speed
 * --Goober5000
 *
 * Revision 2.345  2006/07/05 23:35:43  Goober5000
 * cvs comment tweaks
 *
 * Revision 2.344  2006/07/04 07:42:48  Goober5000
 * --in preparation for fixing an annoying animated texture bug, reorganize the various texture structs and glow point structs and clarify several parts of the texture code :P
 * --this breaks animated glow maps, and animated regular maps still aren't fixed, but these will be remedied shortly
 * --Goober5000
 *
 * Revision 2.343  2006/06/27 04:06:18  Goober5000
 * handle docked objects during death roll
 * --Goober5000
 *
 * Revision 2.342  2006/06/24 20:32:00  wmcoolmon
 * New function for scripting
 *
 * Revision 2.341  2006/06/07 05:19:49  wmcoolmon
 * Move fog disappearance factor to objecttypes.tbl
 *
 * Revision 2.340  2006/06/07 04:47:43  wmcoolmon
 * Limbo flag support; removed unneeded muzzle flash flag
 *
 * Revision 2.339  2006/06/05 23:57:50  taylor
 * properly initialize a few entries that were left zero'd by mistake
 *
 * Revision 2.338  2006/06/04 01:01:53  Goober5000
 * add fighterbay restriction code
 * --Goober5000
 *
 * Revision 2.337  2006/06/02 08:49:35  karajorma
 * Support for alt ship classes, team loadout flag and changes to Ships_exited. Fixed assertion upon multiple uses of the change-ship-class SEXP.
 *
 * Revision 2.336  2006/05/31 03:05:42  Goober5000
 * some cosmetic changes in preparation for bay arrival/departure code
 * --Goober5000
 *
 * Revision 2.335  2006/05/27 16:49:05  taylor
 * comment out some pointless checks which look for not using either D3D or OGL
 * don't run through ships on level load setting up the sound environment if sound is disabled
 * minor cleanup
 *
 * Revision 2.334  2006/05/20 02:03:01  Goober5000
 * fix for Mantis #755, plus make the missionlog #defines uniform
 * --Goober5000
 *
 * Revision 2.333  2006/05/13 07:15:51  taylor
 * get rid of some wasteful math for gr_set_proj_matrix() calls
 * fix check that broke praise of kills for player
 * fix knossos warpin effect that always seems to get rendered backwards (I couldn't find anything that broke with this but I suppose it's a mod could have an issue)
 *
 * Revision 2.332  2006/04/18 00:56:28  bobboau
 * bugfix for the animation system
 *
 * Revision 2.331  2006/04/14 18:39:06  taylor
 * giving a tbl name in this warning message is basically useless since there we can't tell which tbl this particular ship is in
 *
 * Revision 2.330  2006/04/07 20:17:33  karajorma
 * Added SEXPs to lock and unlock the primary and secondary weapons
 *
 * Revision 2.329  2006/04/05 16:46:40  karajorma
 * Changes to support the new Enable/Disable-Builtin-Messages SEXP
 *
 * Revision 2.327  2006/04/05 13:45:08  taylor
 * correct some coding mistakes
 *
 * Revision 2.326  2006/04/04 11:38:07  wmcoolmon
 * Maneuvering hruster scaling, gun convergence
 *
 * Revision 2.325  2006/04/03 08:09:32  wmcoolmon
 * Maneuvering thruster fixes
 *
 * Revision 2.324  2006/04/03 07:48:03  wmcoolmon
 * Miscellaneous minor changes, mostly related to addition of Current_camera variable
 *
 * Revision 2.323  2006/04/01 01:21:58  wmcoolmon
 * $Warp time and $Warp speed vars
 *
 * Revision 2.322  2006/03/31 10:20:01  wmcoolmon
 * Prelim. BSG warpin effect stuff
 *
 * Revision 2.321  2006/03/24 07:38:36  wmcoolmon
 * New subobject animation stuff and Lua functions.
 *
 * Revision 2.320  2006/03/18 07:12:08  Goober5000
 * add ship-subsys-targetable and ship-subsys-untargetable
 * --Goober5000
 *
 * Revision 2.319  2006/02/28 05:16:55  Goober5000
 * fix the animation type triggers
 *
 * Revision 2.318  2006/02/27 04:17:11  Goober5000
 * add log entry for jettison-cargo
 *
 * Revision 2.317  2006/02/26 18:49:07  Goober5000
 * some more WCSaga stuff
 *
 * Revision 2.316  2006/02/25 21:47:08  Goober5000
 * spelling
 *
 * Revision 2.315  2006/02/24 05:13:26  wmcoolmon
 * Maybe fix ships-limit bug
 *
 * Revision 2.314  2006/02/24 05:02:34  Goober5000
 * remove my old anti-frustration thing since it's not what retail does
 * --Goober5000
 *
 * Revision 2.313  2006/02/20 07:26:07  taylor
 * div-by-0 bug fixage
 *
 * Revision 2.312  2006/02/17 21:47:47  wmcoolmon
 * Fix a silly bug
 *
 * Revision 2.311  2006/02/16 05:44:53  taylor
 * remove reset of modelnums on level start (more properly moved to model_unload())
 * minor change ship class fixage to clear out old model
 * take better care to make sure that a ship model has the proper refcount (this needs further work, I don't like :V:'s crap here)
 * add <string> to includes in ship.h, fixes a compile issue that I forgot to fix properly before (thanks Jens)
 *
 * Revision 2.310  2006/02/13 00:20:45  Goober5000
 * more tweaks, plus clarification of checks for the existence of files
 * --Goober5000
 *
 * Revision 2.309  2006/02/11 02:58:23  Goober5000
 * yet more various and sundry fixes
 * --Goober5000
 *
 * Revision 2.308  2006/02/06 02:06:02  wmcoolmon
 * Various fixes; very beginnings of Directives scripting support
 *
 * Revision 2.307  2006/02/02 07:13:42  Goober5000
 * bah and double bah
 *
 * Revision 2.306  2006/02/02 07:01:16  Goober5000
 * fixed what I broke for ship types :p
 * --Goober5000
 *
 * Revision 2.305  2006/02/01 05:11:25  Goober5000
 * bettered the ballistic primary check
 * --Goober5000
 *
 * Revision 2.303  2006/01/30 06:34:06  taylor
 * try and detect some bad path info
 *
 * Revision 2.302  2006/01/29 18:01:04  wmcoolmon
 * Apologies, don't know how I missed this...
 *
 * Revision 2.301  2006/01/29 07:42:49  wmcoolmon
 * Make sure stuff is set up properly for ship kills
 *
 * Revision 2.300  2006/01/21 09:36:58  wmcoolmon
 * Texture replacement stuff
 *
 * Revision 2.299  2006/01/17 03:45:32  phreak
 * Rearranged the rearming code so it doesn't potentially take 2 minutes to reload a single bomb.
 *
 * Revision 2.298  2006/01/16 11:02:23  wmcoolmon
 * Various warning fixes, scripting globals fix; added "plr" and "slf" global variables for in-game hooks; various lua functions; GCC fixes for scripting.
 *
 * Revision 2.297  2006/01/15 18:55:27  taylor
 * fix compile issues from bad constructor
 * make sure ai_actively_pursues gets filled for modular tables too
 * fix NULL ptr reference when parsing ship type which doesn't exist and is set to not create
 *
 * Revision 2.296  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.295  2006/01/13 13:06:15  taylor
 * hmm, didn't put much thought in that the first time, this should work closer to the original but still be cleaner
 *
 * Revision 2.294  2006/01/13 11:09:45  taylor
 * fix hud comm message screwups (missing support ship, no coverme, etc) that was part :V: bug and (bigger) part Ship_types related bug
 *
 * Revision 2.293  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.292  2006/01/09 04:54:14  phreak
 * Remove tertiary weapons in their current form, I want something more flexable instead of what I had there.
 *
 * Revision 2.291  2006/01/06 04:18:55  wmcoolmon
 * turret-target-order SEXPs, ship thrusters
 *
 * Revision 2.290  2006/01/05 05:12:11  taylor
 * allow both +Pri style and original style (+Normal, etc) for species_defs TBMs
 * allow for "<none>" as a bitmap/anim name, to have no effect
 * fix ship/weapon thruster rendering to handle missing primary animation and/or glow graphics
 *
 * Revision 2.289  2006/01/02 07:16:43  taylor
 * use cf_find_file_location() for objecttypes.tbl rather than a cfopen()
 * fix a few compiler warning messages
 *
 * Revision 2.288  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.287  2005/12/28 22:21:04  taylor
 * Oops, bit of debug stuff I didn't mean to commit (but something similar is needed so you'll it again)
 *
 * Revision 2.286  2005/12/28 22:17:01  taylor
 * deal with cf_find_file_location() changes
 * add a central parse_modular_table() function which anything can use
 * fix up weapon_expl so that it can properly handle modular tables and LOD count changes
 * add support for for a fireball TBM (handled a little different than a normal TBM is since it only changes rather than adds)
 *
 * Revision 2.285  2005/12/21 08:27:37  taylor
 * add the name of the modular table about to be parsed to the debug log
 * a missing weapon_expl table should just be a note in the debug log rather than a popup warning
 *
 * Revision 2.284  2005/12/16 03:36:21  wmcoolmon
 * Keep a crash from happening if POF file isn't initially specified
 *
 * Revision 2.283  2005/12/13 22:32:30  wmcoolmon
 * Ability to disable damage particle spew on ships
 *
 * Revision 2.282  2005/12/13 20:20:20  wmcoolmon
 * Minor XMT-engine wash fix
 *
 * Revision 2.281  2005/12/13 18:15:26  taylor
 * Hmm, .triggers isn't ever malloc'd for the Ships[] copy so don't try to free it since it's actully trying to free the Ship_info[] memory instead, which is bad :)
 *   (newer glibc just starting going monkey over this, not sure why it didn't screw up before)
 *
 * Revision 2.280  2005/12/12 21:32:14  taylor
 * allow use of a specific LOD for ship and weapon rendering in the hud targetbox
 *
 * Revision 2.279  2005/12/12 05:29:59  taylor
 * double free and invalid ptr reference fixage
 *
 * Revision 2.278  2005/12/08 15:17:35  taylor
 * fix several bad crash related problems from WMC's commits on the 4th
 *
 * Revision 2.277  2005/12/06 03:17:48  taylor
 * cleanup some debug log messages:
 *   note that a nprintf() with "Warning" or "General" is basically the same thing as mprintf()
 *   make sure that OpenAL init failures always get to the debug log
 *
 * Revision 2.276  2005/12/05 09:09:53  wmcoolmon
 * Oops, arand should only be generated if it isn't passed to the function as well.
 *
 * Revision 2.275  2005/12/04 22:44:00  Goober5000
 * fix compiler error
 * --Goober5000
 *
 * Revision 2.274  2005/12/04 18:58:07  wmcoolmon
 * subsystem + shockwave armor support; countermeasures as weapons
 *
 * Revision 2.273  2005/11/24 08:46:10  Goober5000
 * * cleaned up mission_do_departure
 *   * fixed a hidden crash (array index being -1; would only
 * be triggered for ships w/o subspace drives under certain conditions)
 *   * removed finding a new fighterbay target because it might screw up missions
 *   * improved clarity, code flow, and readability :)
 * * added custom AI flag for disabling warpouts if navigation subsystem fails
 * --Goober5000
 *
 * Revision 2.272  2005/11/24 03:07:35  phreak
 * Forgot to add in the part where the rearm timer actually counts down.  Minor oversight
 *
 * Revision 2.271  2005/11/23 06:59:09  taylor
 * yeah I know, I should have noticed that little jewel *before* the commit
 *
 * Revision 2.270  2005/11/23 06:54:28  taylor
 * crash fix for using flak without having something targetted
 *
 * Revision 2.269  2005/11/23 01:06:58  phreak
 * Added the function to estimate rearm and repair time.
 *
 * Revision 2.268  2005/11/21 23:57:26  taylor
 * some minor thruster cleanup, if you could actually use the term "clean"
 *
 * Revision 2.267  2005/11/21 13:04:08  taylor
 * i minus u ;)  (should fix overzealous directives list)
 *
 * Revision 2.266  2005/11/21 02:43:30  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 * Revision 2.265  2005/11/21 00:46:05  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.264  2005/11/17 20:52:02  taylor
 * fix for thruster glows (affecting TBP mainly but OC as well):
 *  - thruster glows should always come from ship entry rather than species (it will be initted by the species entry at first)
 *  - this obviously needs some work done on it but that gets dangerously close to a rewrite and that isn't something I have time for
 *
 * Revision 2.263  2005/11/17 02:31:36  taylor
 * fix crash on kamikaze deaths
 *
 * Revision 2.262  2005/11/13 06:49:04  taylor
 * -loadonlyused in on by default now, can be turned off with -loadallweps
 *
 * Revision 2.261  2005/11/08 01:04:02  wmcoolmon
 * More warnings instead of Int3s/Asserts, better Lua scripting, weapons_expl.tbl is no longer needed nor read, added "$Disarmed ImpactSnd:", fire-beam fix
 *
 * Revision 2.260  2005/11/05 05:11:29  wmcoolmon
 * Slight optimization
 *
 * Revision 2.259  2005/10/30 23:45:45  Goober5000
 * stuff for comparing ship classes
 * --Goober5000
 *
 * Revision 2.258  2005/10/30 20:03:40  taylor
 * add a bunch of Assert()'s and NULL checks to either help debug or avoid errors
 * fix Mantis bug #381
 * fix a small issue with the starfield bitmap removal sexp since it would read one past the array size
 *
 * Revision 2.257  2005/10/30 06:44:58  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.256  2005/10/29 22:09:31  Goober5000
 * multiple ship docking implemented for initially docked ships
 * --Goober5000
 *
 * Revision 2.255  2005/10/26 00:43:05  taylor
 * make sure that XMTs don't try to still create an invalid entry when +nocreate is used
 *
 * Revision 2.254  2005/10/24 12:42:14  taylor
 * init thruster stuff properly so that bmpman doesn't have a fit
 *
 * Revision 2.253  2005/10/24 07:13:04  Goober5000
 * merge Bobboau's thruster code back in; hopefully this covers everything
 * --Goober5000
 *
 * Revision 2.252  2005/10/20 17:50:02  taylor
 * fix player warpout
 * basic code cleanup (that previous braces change did nothing for readability)
 * spell "plyr" correctly
 * tweak warp shrink time to better match WMC's other changes and avoid the skipping during shrink
 *
 * Revision 2.251  2005/10/20 06:37:33  wmcoolmon
 * Oops, guess I didn't commit this stuff
 *
 * Revision 2.250  2005/10/17 01:51:01  wmcoolmon
 * Weapon models now shown in lab
 *
 * Revision 2.249  2005/10/16 23:15:13  wmcoolmon
 * Small fix for amor code
 *
 * Revision 2.248  2005/10/16 18:54:12  Goober5000
 * I need to bone up on my bulletproofing
 * --Goober5000
 *
 * Revision 2.247  2005/10/14 07:22:24  Goober5000
 * removed an unneeded parameter and renamed some stuff
 * --Goober5000
 *
 * Revision 2.246  2005/10/14 07:06:59  Goober5000
 * stuff for WMC: fix sexp description and three warnings; plus add some
 * bulletproofing to name-specified ship_create
 * --Goober5000
 *
 * Revision 2.245  2005/10/14 02:13:52  wmcoolmon
 * armor.tbl work
 *
 * Revision 2.244  2005/10/13 18:47:45  wmcoolmon
 * Fixage for ship_create
 *
 * Revision 2.243  2005/10/11 07:43:10  wmcoolmon
 * Topdown updates
 *
 * Revision 2.242  2005/10/11 05:24:34  wmcoolmon
 * Gliding
 *
 * Revision 2.241  2005/10/10 17:21:10  taylor
 * remove NO_NETWORK
 *
 * Revision 2.240  2005/10/10 01:14:11  wmcoolmon
 * Tidied up code a bit
 *
 * Revision 2.239  2005/10/09 17:38:49  wmcoolmon
 * Added names to the 'limits reached' dialogs in ship/weapons.tbl
 *
 * Revision 2.238  2005/10/09 09:13:29  wmcoolmon
 * Added warpin/warpout speed override values to ships.tbl
 *
 * Revision 2.237  2005/10/09 00:43:09  wmcoolmon
 * Extendable modular tables (XMTs); added weapon dialogs to the Lab
 *
 * Revision 2.236  2005/10/08 05:41:09  wmcoolmon
 * Fix Int3() from ship-vanish
 *
 * Revision 2.235  2005/10/02 23:12:44  Goober5000
 * fixed the CTD when support is called for a ship lacking a rearming dockpoint
 * --Goober5000
 *
 * Revision 2.234  2005/09/29 04:26:08  Goober5000
 * parse fixage
 * --Goober5000
 *
 * Revision 2.233  2005/09/26 06:00:59  Goober5000
 * this should fix the rest of the briefing icon bugs
 * --Goober5000
 *
 * Revision 2.232  2005/09/26 04:08:54  Goober5000
 * some more cleanup
 * --Goober5000
 *
 * Revision 2.231  2005/09/25 20:42:57  Goober5000
 * taylor forgot something ;)
 * --Goober5000
 *
 * Revision 2.230  2005/09/25 08:21:54  Goober5000
 * remove duplicate #include
 * --Goober5000
 *
 * Revision 2.229  2005/09/25 05:13:04  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.228  2005/09/24 07:45:31  Goober5000
 * cleaned up some more thruster stuff; honestly, the thruster code is such a
 * mess that it should probably be reverted to the retail version
 * --Goober5000
 *
 * Revision 2.227  2005/09/24 07:07:16  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 2.226  2005/09/24 01:50:09  Goober5000
 * a bunch of support ship bulletproofing
 * --Goober5000
 *
 * Revision 2.225  2005/09/20 02:48:06  taylor
 * make sure that any extra ship textures get loaded on a class change (that Ares bug which wasn't actually Ares specific)
 *
 * Revision 2.224  2005/09/06 00:32:19  Kazan
 * fixed a bug related to multiplayer table validation and modular tables
 *
 * Revision 2.223  2005/09/05 09:02:08  wmcoolmon
 * Fix the ship vanishing in TOPDOWN mode error
 *
 * Revision 2.222  2005/09/01 04:14:04  taylor
 * various weapon_range cap fixes for primary, secondary weapons and hud targetting info
 *
 * Revision 2.221  2005/08/31 07:01:40  Goober5000
 * remove a redundant function
 * --Goober5000
 *
 * Revision 2.220  2005/08/25 22:40:03  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.219  2005/08/08 03:13:53  taylor
 * be sure to remove ships from a wing when that ship gets vanished
 *
 * Revision 2.218  2005/08/03 22:02:38  Goober5000
 * made "stealth" tag more user-friendly, and fixed a bit of punctuation
 * --Goober5000
 *
 * Revision 2.217  2005/07/30 22:34:42  wmcoolmon
 * Removed turret thing, added some more armor type calculations
 *
 * Revision 2.216  2005/07/25 03:13:24  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 2.215  2005/07/24 06:01:37  wmcoolmon
 * Multiple shockwaves support.
 *
 * Revision 2.214  2005/07/24 00:32:45  wmcoolmon
 * Synced 3D shockwaves' glowmaps with the model, tossed in some medals.tbl
 * support for the demo/FS1
 *
 * Revision 2.213  2005/07/23 08:17:04  wmcoolmon
 * Another try at fixing gun turrets sticking straight up
 *
 * Revision 2.212  2005/07/22 10:18:35  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.211  2005/07/22 04:31:12  Goober5000
 * fixed another bug when changing ship classes
 * --Goober5000
 *
 * Revision 2.210  2005/07/22 03:54:46  taylor
 * better error checking/handling for when you fire primary/beam weapons without a target selected
 *
 * Revision 2.209  2005/07/21 07:53:13  wmcoolmon
 * Changed $Hull Repair Rate and $Subsystem Repair Rate to be percentages,
 * as well as making them accept all values between -1 and 1
 *
 * Revision 2.208  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.207  2005/07/13 02:01:30  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.206  2005/07/13 00:44:21  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.205  2005/07/12 22:14:40  Goober5000
 * removed DECALS_ENABLED
 * --Goober5000
 *
 * Revision 2.204  2005/07/12 21:58:45  Goober5000
 * removed NO_LINKED_PRIMARY_PENALTY as it's better suited for difficulty.tbl or something similar
 * --Goober5000
 *
 * Revision 2.203  2005/07/12 07:03:18  Goober5000
 * remove all restrictions on which ship types can dock with each other
 * --Goober5000
 *
 * Revision 2.202  2005/06/20 04:10:35  taylor
 * little cleaner ship_get_random_targetable_ship(), for Goober's sake ;)
 *
 * Revision 2.201  2005/06/07 06:10:51  wmcoolmon
 * This may stop targeting not-targetable ships in EMP
 *
 * Revision 2.200  2005/05/26 04:30:48  taylor
 * don't page out textures in ship_delete() since it causes some slowdown which can make
 *   the explosion effects skip/stutter
 *
 * Revision 2.199  2005/05/12 17:45:53  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 * reallocate sp->triggers with the real vm_realloc() rather than that manual method
 *
 * Revision 2.198  2005/05/08 20:21:48  wmcoolmon
 * armor.tbl revamp
 *
 * Revision 2.197  2005/04/30 18:18:46  phreak
 * ABbitmap should default to -1
 *
 * Revision 2.196  2005/04/28 05:29:30  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.195  2005/04/28 01:38:32  wmcoolmon
 * parse_ship uses stuff_bool_list; stuff_byte to stuff_ubyte
 *
 * Revision 2.194  2005/04/25 00:31:14  wmcoolmon
 * Dynamically allocated engine washes; subsystem sounds; armor fixes. Line 4268 of ship.cpp, apparently, works properly; bears further looking into.
 *
 * Revision 2.193  2005/04/20 08:26:49  wmcoolmon
 * Fix silly armor.tbl parse error.
 *
 * Revision 2.192  2005/04/19 06:27:54  taylor
 * we might actually be needing to load a model here ;)
 *
 * Revision 2.191  2005/04/18 08:35:27  Goober5000
 * model and class changes should be all set now
 * --Goober5000
 *
 * Revision 2.190  2005/04/18 05:27:26  Goober5000
 * removed ship->alt_modelnum as it was essentially duplicates of ship->modelnum; changed the alt modelnum stuff accordingly
 * fixes for ship_model_change and change_ship_type
 * --Goober5000
 *
 * Revision 2.189  2005/04/16 04:19:21  wmcoolmon
 * Eh, what the hey. Maybe I should've taken the breathalyzer test. :)
 *
 * Revision 2.188  2005/04/16 04:16:57  wmcoolmon
 * More optional tag-making for ships.tbl
 *
 * Revision 2.187  2005/04/16 03:36:13  wmcoolmon
 * Minor changes; made even more fields in ships.tbl optional.
 *
 * Revision 2.186  2005/04/15 23:19:13  wmcoolmon
 * Added type "exponential base"; equation is 'final_damage = (x^damage)'
 *
 * Revision 2.185  2005/04/15 11:32:26  taylor
 * proposes that WMC be subject to a breathalyzer test before commit ;)
 *
 * Revision 2.184  2005/04/15 07:19:29  wmcoolmon
 * It's amazing how many possible bugs you can catch simply by doing something completely different than coding
 *
 * Revision 2.183  2005/04/15 06:40:54  wmcoolmon
 * Er, oops
 *
 * Revision 2.182  2005/04/15 06:23:17  wmcoolmon
 * Local codebase commit; adds armor system.
 *
 * Revision 2.181  2005/04/05 05:53:24  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.180  2005/04/01 07:29:40  taylor
 * some minor Linux fixage
 *
 * Revision 2.179  2005/03/31 12:38:24  Goober5000
 * fix type mismatch
 * --Goober5000
 *
 * Revision 2.178  2005/03/30 05:37:22  wmcoolmon
 * Whoops.
 *
 * Revision 2.177  2005/03/30 02:32:41  wmcoolmon
 * Made it so *Snd fields in ships.tbl and weapons.tbl take the sound name
 * as well as its index (ie "L_sidearm.wav" instead of "76")
 *
 * Revision 2.176  2005/03/27 13:37:15  Goober5000
 * bah
 *
 * Revision 2.175  2005/03/27 12:28:35  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.174  2005/03/25 06:57:37  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.173  2005/03/24 23:27:26  taylor
 * make sounds.tbl dynamic
 * have snd_time_remaining() be less stupid
 * some OpenAL error fixerage
 * be able to turn off some typically useless debug messages
 *
 * Revision 2.172  2005/03/19 18:02:34  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.171  2005/03/16 00:18:31  wmcoolmon
 * Commited placeholder turret funcs.
 *
 * Revision 2.170  2005/03/10 08:00:15  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.169  2005/03/08 02:31:51  bobboau
 * minor change to high level render target code
 *
 * Revision 2.168  2005/03/03 07:13:17  wmcoolmon
 * Made HUD shield icon auto-generation off unless "generate icon" ship flag is specified for the ship.
 *
 * Revision 2.167  2005/03/03 06:05:31  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.166  2005/03/02 21:24:47  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.165  2005/03/01 23:05:38  taylor
 * unbreak Linux build (not valid C/C++ anyway, block scope)
 *
 * Revision 2.164  2005/03/01 06:55:45  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.163  2005/02/19 07:57:02  wmcoolmon
 * Removed trails limit
 *
 * Revision 2.162  2005/02/15 00:03:35  taylor
 * don't try and draw starfield bitmaps if they aren't valid
 * make AB thruster stuff in ship_create() a little less weird
 * replace an Int3() with debug warning and fix crash in docking code
 * make D3D Textures[] allocate on use like OGL does, can only use one anyway
 *
 * Revision 2.161  2005/02/10 14:38:50  taylor
 * fix an issue with bm_set_components()
 * abs is for ints fabsf is for floats (camera.cpp)
 * make the in-cockpit stuff OGL friendly
 *
 * Revision 2.160  2005/02/04 20:06:08  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.159  2005/01/30 09:36:19  Goober5000
 * optional flags default to false I should think
 * --Goober5000
 *
 * Revision 2.158  2005/01/30 01:34:39  wmcoolmon
 * Made a bunch of ship.tbl variables optional
 *
 * Revision 2.157  2005/01/29 08:11:41  wmcoolmon
 * When drawing the viewer_obj, temporarily clip less close up for in-cockpit models.
 *
 * Revision 2.156  2005/01/28 11:57:36  Goober5000
 * fixed Bobboau's spelling of 'relative'
 * --Goober5000
 *
 * Revision 2.155  2005/01/28 11:06:23  Goober5000
 * changed a bunch of transpose-rotate sequences to use unrotate instead
 * --Goober5000
 *
 * Revision 2.154  2005/01/27 04:23:17  wmcoolmon
 * Ship autorepair, requested by TBP
 *
 * Revision 2.153  2005/01/17 23:35:45  argv
 * Surface shields.
 *
 * See forum thread:
 * http://www.hard-light.net/forums/index.php/topic,29643.0.html
 *
 * -- _argv[-1]
 *
 * Revision 2.152  2005/01/16 23:18:03  wmcoolmon
 * Added "show ship" ship flag
 *
 * Revision 2.151  2005/01/16 22:39:10  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.150  2005/01/12 04:45:33  Goober5000
 * fixed a nasty bug where dock pointers would run off a cliff and crash
 * --Goober5000
 *
 * Revision 2.149  2005/01/11 21:38:48  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.148  2005/01/11 04:05:22  taylor
 * fully working (??) -loadonlyused, allocate used_weapons[] and ship_class_used[] only when needed
 *
 * Revision 2.147  2005/01/03 18:47:06  taylor
 * more -loadonlyused fixes
 *
 * Revision 2.146  2005/01/01 07:18:48  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.144  2004/12/25 09:23:10  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD
 *
 * Revision 2.143  2004/12/15 17:32:19  Goober5000
 * move wing name initialization to ship_level_init
 * --Goober5000
 *
 * Revision 2.142  2004/12/14 14:46:12  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.141  2004/12/05 22:01:12  bobboau
 * sevral feature additions that WCS wanted,
 * and the foundations of a submodel animation system,
 * the calls to the animation triggering code (exept the procesing code,
 * wich shouldn't do anything without the triggering code)
 * have been commented out.
 *
 * Revision 2.140  2004/11/21 11:35:17  taylor
 * some weapon-only-used loading fixes and general page-in cleanup
 * page in all ship textures from one function rather than two
 *
 * Revision 2.139  2004/11/01 20:57:04  taylor
 * make use of Knossos_warp_ani_used flag - thanks Goober5000
 *
 * Revision 2.138  2004/10/31 22:04:33  taylor
 * be sure to initialize splodeing_texture, fix issue that would cause loading problems for glows
 *
 * Revision 2.137  2004/10/15 09:21:55  Goober5000
 * cleaned up some sexp stuff and added wing capability to kamikaze sexp
 * --Goober5000
 *
 * Revision 2.136  2004/08/20 05:13:08  Kazan
 * wakka wakka - fix minor booboo
 *
 * Revision 2.135  2004/07/26 20:47:51  Kazan
 * remove MCD complete
 *
 * Revision 2.134  2004/07/25 00:31:31  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 2.133  2004/07/17 18:46:09  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.132  2004/07/17 09:25:59  taylor
 * add CF_SORT_REVERSE to real sort routine, makes CF_SORT_TIME work again
 *
 * Revision 2.131  2004/07/14 01:26:10  wmcoolmon
 * Better -load_only_used handling
 *
 * Revision 2.130  2004/07/12 16:33:05  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.129  2004/07/11 03:22:52  bobboau
 * added the working decal code
 *
 * Revision 2.128  2004/07/10 08:03:13  wmcoolmon
 * Fixed bug with the base ship flag I unwittingly introduced.
 *
 * Revision 2.127  2004/07/01 01:54:32  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.126  2004/06/18 04:59:54  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.125  2004/06/07 07:36:08  wmcoolmon
 * Warpout failure bug fixingness
 *
 * Revision 2.124  2004/05/26 21:02:27  wmcoolmon
 * Added weapons_expl modular table, updated cfilesystem to work with modular tables, fixed loading order, fixed ship loading error messages
 *
 * Revision 2.123  2004/05/26 03:52:07  wmcoolmon
 * Ship & weapon modular table files
 *
 * Revision 2.122  2004/05/26 02:31:39  wmcoolmon
 * Modular ship table support. Uses *-shp.tbm , with the same structure as a normal ships.tbl. Individual sections are optional and entries with the same name will override previous settings. -C
 *
 * Revision 2.121  2004/05/10 13:07:22  Goober5000
 * fixed the AWACS help message
 * --Goober5000
 *
 * Revision 2.120  2004/05/10 10:51:51  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
 * Revision 2.119  2004/05/10 08:03:31  Goober5000
 * fixored the handling of no lasers and no engines... the tests should check the ship,
 * not the object
 * --Goober5000
 *
 * Revision 2.118  2004/05/02 03:05:23  Goober5000
 * added a comment
 * --Goober5000
 *
 * Revision 2.117  2004/04/30 22:20:27  Goober5000
 * extra insurance
 * --Goober5000
 *
 * Revision 2.116  2004/04/13 05:42:44  Goober5000
 * fixed the custom hitpoints subsystem bug
 * --Goober5000
 *
 * Revision 2.115  2004/04/03 02:55:50  bobboau
 * commiting recent minor bug fixes
 *
 * Revision 2.114  2004/03/21 10:34:05  bobboau
 * fixed a texture loading bug
 *
 * Revision 2.113  2004/03/20 21:17:13  bobboau
 * fixed -spec comand line option,
 * probly some other stuf
 *
 * Revision 2.112  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.111  2004/03/05 23:54:48  Goober5000
 * d'oh!
 * --Goober5000
 *
 * Revision 2.110  2004/03/05 23:41:04  Goober5000
 * made awacs only ask for help in the main fs2 campaign
 * --Goober5000
 *
 * Revision 2.109  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.108  2004/02/20 04:29:56  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.107  2004/02/16 11:47:34  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.106  2004/02/14 00:18:35  randomtiger
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
 * Revision 2.105  2004/02/07 00:48:52  Goober5000
 * made FS2 able to account for subsystem mismatches between ships.tbl and the
 * model file - e.g. communication vs. communications
 * --Goober5000
 *
 * Revision 2.104  2004/02/05 14:31:44  Goober5000
 * fixed a few random bugs
 * --Goober5000
 *
 * Revision 2.103  2004/02/04 08:41:01  Goober5000
 * made code more uniform and simplified some things,
 * specifically shield percentage and quadrant stuff
 * --Goober5000
 *
 * Revision 2.102  2004/02/04 04:28:15  Goober5000
 * fixed Asserts in two places and commented out an unneeded variable
 * --Goober5000
 *
 * Revision 2.101  2004/01/31 04:06:29  phreak
 * commented out decal references
 *
 * Revision 2.100  2004/01/30 07:39:06  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.99  2004/01/14 07:07:14  Goober5000
 * added error checking for an annoying crash when running an out-of-range
 * sound; also, Phreak misspelled "tertiary"
 * --Goober5000
 *
 * Revision 2.98  2004/01/14 06:34:07  Goober5000
 * made set-support-ship number align with general FS convention
 * --Goober5000
 *
 * Revision 2.97  2003/12/18 15:35:52  phreak
 * oops switched the signs in a comparison.  thats what was causing debug builds to go haywire
 *
 * Revision 2.96  2003/12/16 20:55:13  phreak
 * disabled tertiary weapons support pending a rewrite of critical code
 *
 * Revision 2.95  2003/12/15 21:36:42  phreak
 * replaced asserts in parse_ship with more descriptive warnings
 *
 * Revision 2.94  2003/11/21 22:30:45  phreak
 * changed PLAYER_MAX_DIST_WARNING to 700000 (700km)
 * changed PLAYER_MAX_DIST_END to 750000           (750km)
 *
 * this makes the playing field much bigger
 * this way you don't get the "return to the field of battle messages"
 * in the future we should have the mission designer set this limit
 *
 * Revision 2.93  2003/11/19 20:37:25  randomtiger
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
 * Revision 2.92  2003/11/17 06:52:52  bobboau
 * got assert to work again
 *
 * Revision 2.91  2003/11/16 09:42:36  Goober5000
 * clarified and pruned debug spew messages
 * --Goober5000
 *
 * Revision 2.90  2003/11/11 02:15:40  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.89  2003/11/09 04:09:17  Goober5000
 * edited for language
 * --Goober5000
 *
 * Revision 2.88  2003/11/01 21:59:22  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.87  2003/10/25 06:56:06  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.86  2003/10/23 18:03:25  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.85  2003/10/15 22:03:26  Kazan
 * Da Species Update :D
 *
 * Revision 2.84  2003/10/12 03:46:23  Kazan
 * #Kazan# FS2NetD client code gone multithreaded, some Fred2 Open -mod stuff [obvious code.lib] including a change in cmdline.cpp, changed Stick's "-nohtl" to "-htl" - HTL is _OFF_ by default here (Bobboau and I decided this was a better idea for now)
 *
 * Revision 2.83  2003/10/07 03:43:21  Goober5000
 * fixed some warnings
 * --Goober5000
 *
 * Revision 2.82  2003/09/26 14:37:16  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.81  2003/09/13 20:59:54  Goober5000
 * fixed case-sensitivity bugs and possibly that Zeta wing bug
 * --Goober5000
 *
 * Revision 2.80  2003/09/13 08:27:28  Goober5000
 * added some minor things, such as code cleanup and the following:
 * --turrets will not fire at cargo
 * --MAX_SHIELD_SECTIONS substituted for the number 4 in many places
 * --supercaps have their own default message bitfields (distinguished from capships)
 * --turrets are allowed on fighters
 * --jump speed capped at 65m/s, to avoid ship travelling too far
 * --non-huge weapons now scale their damage, instead of arbitrarily cutting off
 * ----Goober5000
 *
 * Revision 2.79  2003/09/13 06:02:03  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.75  2003/09/06 20:40:01  wmcoolmon
 * Added ability to limit subsystem repairs
 *
 * Revision 2.74  2003/09/06 19:14:50  wmcoolmon
 * Minor bugfix (changed an == to >=)
 *
 * Revision 2.73  2003/09/06 19:07:31  wmcoolmon
 * Made it possible to limit how much support ships repair a ship's hull.
 *
 * Revision 2.72  2003/09/05 04:25:27  Goober5000
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
 * Revision 2.71  2003/08/28 20:42:18  Goober5000
 * implemented rotating barrels for firing primary weapons
 * --Goober5000
 *
 * Revision 2.70  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.69  2003/08/21 06:11:09  Goober5000
 * Ballistic primaries will now deplete ammo correctly - that is, ammo will be
 * reduced by the number of firing points, not necessarily 1.
 * --Goober5000
 *
 * Revision 2.68  2003/08/21 05:50:00  Goober5000
 * "Fixed" the ballistic primary rearm bug.  I have no idea how... I have to assume
 * it's a bug in MSVC.
 * --Goober5000
 *
 * Revision 2.67  2003/08/06 17:37:08  phreak
 * preliminary work on tertiary weapons. it doesn't really function yet, but i want to get something committed
 *
 * Revision 2.66  2003/07/15 02:52:40  phreak
 * ships now decloak when firing
 *
 * Revision 2.65  2003/07/04 02:30:54  phreak
 * support for cloaking added.  needs a cloakmap.pcx
 * to cloak the players ship, activate cheats and press tilde + x
 * some more work can be done to smooth out the animation.
 *
 * Revision 2.64  2003/06/25 03:19:40  phreak
 * added support for weapons that only fire when tagged
 *
 * Revision 2.63  2003/06/16 03:36:08  phreak
 * fixed a small bug where rotating subobjects wouldn't work on a player ship
 * also changed "+PBank Capacity:" to "$PBank Capacity:" when parsing
 * ballistic primaries. goob knows about this
 *
 * Revision 2.62  2003/06/04 15:32:54  phreak
 * final fix (hopefully) to zero mass value bug
 *
 * Revision 2.61  2003/05/15 19:08:24  phreak
 * clarified an error message in subsys_set
 *
 * Revision 2.60  2003/05/09 23:53:32  phreak
 * possible fix to ships with no mass values
 *
 * Revision 2.59  2003/04/29 01:03:21  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.58  2003/03/30 07:27:34  Goober5000
 * resolved a nasty bug that caused some missions to crash
 * --Goober5000
 *
 * Revision 2.57  2003/03/18 10:07:05  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.56  2003/03/18 01:44:30  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.55  2003/03/06 09:13:43  Goober5000
 * fixed what should be the last bug with bank-specific loadouts
 * --Goober5000
 *
 * Revision 2.54  2003/03/05 12:38:01  Goober5000
 * rewrote the restricted bank loadout code; it should work now
 * --Goober5000
 *
 * Revision 2.53  2003/03/05 09:17:15  Goober5000
 * cleaned out Bobboau's buggy code - about to rewrite with new, bug-free code :)
 * --Goober5000
 *
 * Revision 2.52  2003/03/03 17:15:16  Goober5000
 * fixed custom banks so that they no longer need the ":" between each entry
 * --Goober5000
 *
 * Revision 2.51  2003/03/03 04:28:37  Goober5000
 * fixed the tech room bug!  yay!
 * --Goober5000
 *
 * Revision 2.50  2003/03/01 01:15:38  Goober5000
 * fixed the initial status bug
 *
 * Revision 2.49  2003/02/25 06:22:49  bobboau
 * fixed a bunch of fighter beam bugs,
 * most notabley the sound now works corectly,
 * and they have limeted range with atenuated damage (table option)
 * added bank specific compatabilities
 *
 * Revision 2.48  2003/02/16 05:14:29  bobboau
 * added glow map nebula bug fix for d3d, someone should add a fix for glide too
 * more importantly I (think I) have fixed all major bugs with fighter beams, and added a bit of new functionality
 *
 * Revision 2.47  2003/02/05 06:57:56  Goober5000
 * made my fighterbay code more forgiving of eccentric modeling hacks...
 * ships can now use the Faustus hangar again :)
 * --Goober5000
 *
 * Revision 2.46  2003/01/27 07:46:32  Goober5000
 * finished up my fighterbay code - ships will not be able to arrive from or depart into
 * fighterbays that have been destroyed (but you have to explicitly say in the tables
 * that the fighterbay takes damage in order to enable this)
 * --Goober5000
 *
 * Revision 2.45  2003/01/20 05:40:50  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.44  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.43  2003/01/19 08:37:52  Goober5000
 * fixed two dumb bugs in the set-support-ship code
 * --Goober5000
 *
 * Revision 2.42  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.41  2003/01/19 01:07:42  bobboau
 * redid the way glow maps are handled; you now must set a global variable before you render a poly that uses a glow map, then set it to -1 when you're done with it
 * fixed a few other misc bugs too
 *
 * Revision 2.40  2003/01/18 09:25:40  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.39  2003/01/17 07:59:08  Goober5000
 * fixed some really strange behavior with strings not being truncated at the
 * # symbol
 * --Goober5000
 *
 * Revision 2.38  2003/01/17 01:48:49  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.37  2003/01/16 06:49:11  Goober5000
 * yay! got texture replacement to work!!!
 * --Goober5000
 *
 * Revision 2.36  2003/01/15 23:23:30  Goober5000
 * NOW the model duplicates work! :p
 * still gotta do the textures, but it shouldn't be hard now
 * --Goober5000
 *
 * Revision 2.35  2003/01/15 08:57:23  Goober5000
 * assigning duplicate models to ships now works; committing so I have a base
 * to fall back to as I work on texture replacement
 * --Goober5000
 *
 * Revision 2.34  2003/01/15 07:09:09  Goober5000
 * changed most references to modelnum to use ship instead of ship_info --
 * this will help with the change-model sexp and any other instances of model
 * changing
 * --Goober5000
 *
 * Revision 2.33  2003/01/13 23:20:00  Goober5000
 * bug hunting; fixed the beam whack effect bug
 * --Goober5000
 *
 * Revision 2.32  2003/01/10 04:14:18  Goober5000
 * I found these two beautiful functions in ship.cpp - ship_change_model
 * and change_ship_type - so I made them into sexps :)
 * --Goober5000
 *
 * Revision 2.31  2003/01/06 19:33:21  Goober5000
 * cleaned up some stuff with model_set_thrust and a commented Assert that
 * shouldn't have been
 * --Goober5000
 *
 * Revision 2.30  2003/01/06 18:50:37  Goober5000
 * a wee failsafe added
 * --Goober5000
 *
 * Revision 2.29  2003/01/06 17:14:52  Goober5000
 * added wing configurable squad logos - put +Squad Logo: filename.pcx as
 * the last entry in each wing that you want (but the player's squad logo will
 * still be the squad logo for the player's wing)
 * --Goober5000
 *
 * Revision 2.28  2003/01/05 23:41:51  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.27  2003/01/05 01:26:35  Goober5000
 * added capability of is-iff and change-iff to have wings as well as ships
 * as their arguments; also allowed a bunch of sexps to accept the player
 * as an argument where they would previously display a parse error
 * --Goober5000
 *
 * Revision 2.26  2003/01/03 21:58:06  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.25  2003/01/02 03:09:00  Goober5000
 * this is the way we squash the bugs, squash the bugs, squash the bugs
 * this is the way we squash the bugs, so early in the morning :p
 * --Goober5000
 *
 * Revision 2.24  2002/12/31 19:35:14  Goober5000
 * tweaked stuff
 * --Goober5000
 *
 * Revision 2.23  2002/12/31 18:59:42  Goober5000
 * if it ain't broke, don't fix it
 * --Goober5000
 *
 * Revision 2.21  2002/12/30 06:16:09  Goober5000
 * made force feedback stronger when firing ballistic primaries
 * --Goober5000
 *
 * Revision 2.20  2002/12/27 02:57:50  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.19  2002/12/24 07:42:28  Goober5000
 * added change-ai-class and is-ai-class, and I think I may also have nailed the
 * is-iff bug; did some other bug hunting as well
 * --Goober5000
 *
 * Revision 2.18  2002/12/23 05:18:52  Goober5000
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
 * Revision 2.17  2002/12/20 07:09:03  Goober5000
 * added capability of storing time_subsys_cargo_revealed
 * --Goober5000
 *
 * Revision 2.16  2002/12/17 02:18:39  Goober5000
 * added functionality and fixed a few things with cargo being revealed and hidden in preparation for the set-scanned and set-unscanned sexp commit
 * --Goober5000
 *
 * Revision 2.15  2002/12/15 06:29:24  DTP
 * Bumped fire_dealy on_empty_secondary to 1000
 *
 * Revision 2.14  2002/12/14 17:09:27  Goober5000
 * removed mission flag for fighterbay damage; instead made damage display contingent on whether the fighterbay subsystem is assigned a damage percentage in ships.tbl
 * --Goober5000
 *
 * Revision 2.13  2002/12/10 05:41:38  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Preliminary support added for selecting additional primaries (more than 2) and secondaries (more than 3) if this is implemented later.
 * Transports can now dock with fighters, bombers, & stealth ships.
 *
 * Revision 2.12  2002/12/07 01:37:42  bobboau
 * initial decals code, if you are worried a bug is being caused by the decals code, its only references are in
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in pretty good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.11  2002/11/14 06:15:03  bobboau
 * added nameplate code
 *
 * Revision 2.10  2002/11/14 04:18:17  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.9  2002/11/11 20:05:52  phreak
 * changed a line in ship_fire_secondary to allow for custom
 * amounts of corkscrew missiles fired
 *
 * Revision 2.8  2002/11/06 23:21:06  phreak
 * Fighter flak code, it doesn't blow up in your face anymore!
 *
 * Revision 2.7  2002/10/19 19:29:29  bobboau
 * initial commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.6.2.1  2002/09/24 18:56:45  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.6  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.5  2002/07/30 18:11:25  wmcoolmon
 * Fixed a bug I added in ship_do_rearm_frame
 *
 * Revision 2.4  2002/07/30 17:57:40  wmcoolmon
 * Modified to add hull repair capabilities and toggling of them via a mission flag, MISSION_FLAG_SUPPORT_REPAIRS_HULL
 *
 * Revision 2.3  2002/07/29 08:22:42  DTP
 * Bumped MAX_SHIP_SUBOBJECTS to 2100(2100 /(average fighter has 7 subsystems) = 400 fighters
 *
 * Revision 2.2  2002/07/25 04:50:07  wmcoolmon
 * Added Bobboau's fighter-beam code.
 *
 * Revision 2.1  2002/07/20 23:49:46  DTP
 * Fixed Secondary bank bug, where next valid secondary bank inherits current
 * valid banks FULL fire delay
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.6  2002/05/16 00:43:32  mharris
 * More ifndef NO_NETWORK
 *
 * Revision 1.5  2002/05/13 21:43:38  mharris
 * A little more network and sound cleanup
 *
 * Revision 1.4  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.3  2002/05/10 20:42:45  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/03 22:07:10  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 144   10/13/99 3:43p Jefff
 * fixed unnumbered XSTRs
 * 
 * 143   9/14/99 3:26a Dave
 * Fixed laser fogging problem in nebula (D3D)> Fixed multiplayer
 * respawn-too-early problem. Made a few crash points safe.
 * 
 * 142   9/11/99 4:02p Dave
 * Don't page in model textures when doing ship_model_change() in fred.
 * 
 * 141   9/10/99 9:44p Dave
 * Bumped version # up. Make server reliable connects not have such a huge
 * timeout. 
 * 
 * 140   9/06/99 3:30p Mikek
 * Added system to restrict weapon choices for dogfight missions.
 * 
 * 139   9/01/99 10:15a Dave
 * 
 * 138   9/01/99 8:43a Andsager
 * supress WARNING from no_fred ships flag
 * 
 * 137   8/26/99 8:52p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 136   8/26/99 6:08p Andsager
 * Add debug code for lethality and number of turrets targeting player.
 * 
 * 135   8/26/99 5:14p Andsager
 * 
 * 134   8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 133   8/24/99 4:25p Andsager
 * Add ship-vanish sexp
 * 
 * 132   8/23/99 11:59a Andsager
 * Force choice of big fireball when Knossos destroyed.  Allow logging of
 * ship destroyed when no killer_name (ie, from debug).
 * 
 * 131   8/23/99 11:09a Andsager
 * Round 2 of Knossos explosion
 * 
 * 130   8/20/99 5:09p Andsager
 * Second pass on Knossos device explosion
 * 
 * 129   8/18/99 10:59p Andsager
 * Enable "b" key to target bombers.
 * 
 * 128   8/18/99 12:09p Andsager
 * Add debug if message has no anim for message.  Make messages come from
 * wing leader.
 * 
 * 127   8/16/99 10:04p Andsager
 * Add special-warp-dist and special-warpout-name sexp for Knossos device
 * warpout.
 * 
 * 126   8/16/99 4:06p Dave
 * Big honking checkin.
 * 
 * 125   8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 124   8/13/99 10:49a Andsager
 * Knossos and HUGE ship warp out.  HUGE ship warp in.  Stealth search
 * modes dont collide big ships.
 * 
 * 123   8/05/99 6:19p Dave
 * New demo checksums.
 * 
 * 122   8/05/99 12:57a Andsager
 * the insanity of it all!
 * 
 * 121   8/03/99 11:13a Andsager
 * Bump up number of ship_subsystems for demo
 * 
 * 120   8/02/99 10:39p Dave
 * Added colored shields. OoOoOoooOoo
 * 
 * 119   7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 118   7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 117   7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 116   7/26/99 5:50p Dave
 * Revised ingame join. Better? We'll see....
 * 
 * 115   7/26/99 8:06a Andsager
 * Consistent personas
 * 
 * 114   7/24/99 5:22p Jefff
 * Removed debug rendering of interpolated ships in multiplayer.
 * 
 * 113   7/19/99 8:57p Andsager
 * Added Ship_exited red_alert_carry flag and hull strength for RED ALERT
 * carry over of Exited_ships
 * 
 * 112   7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 111   7/19/99 12:02p Andsager
 * Allow AWACS on any ship subsystem. Fix sexp_set_subsystem_strength to
 * only blow up subsystem if its strength is > 0
 * 
 * 110   7/18/99 9:56p Andsager
 * Make max_ship_subsys 300 again!
 * 
 * 109   7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 108   7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 107   7/15/99 6:36p Jamesa
 * Moved default ship name into the ships.tbl
 * 
 * 104   7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 103   7/13/99 5:03p Alanl
 * make sure object sounds get assigned to ships
 * 
 * 102   7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 101   7/08/99 5:49p Andsager
 * Fixed bug colliding with just warped in Cap ship
 * 
 * 100   7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 99    7/06/99 4:24p Dave
 * Mid-level checkin. Starting on some potentially cool multiplayer
 * smoothness crap.
 * 
 * 98    7/06/99 10:45a Andsager
 * Modify engine wash to work on any ship that is not small.  Add AWACS
 * ask for help.
 * 
 * 97    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 96    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 95    6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 94    6/20/99 12:06a Alanl
 * new event music changes
 * 
 * 93    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 92    6/16/99 10:21a Dave
 * Added send-message-list sexpression.
 * 
 * 91    6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 90    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 89    6/07/99 4:21p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 88    6/04/99 5:08p Andsager
 * Sort of hack to allow minicap and supercap tat the same time.
 * 
 * 87    6/03/99 9:29p Andsager
 * Remove special case for Asteroid ship LOD warning
 * 
 * 86    6/03/99 11:43a Dave
 * Added the ability to use a different model when rendering to the HUD
 * target box.
 * 
 * 85    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 84    5/28/99 9:26a Andsager
 * Added check_world_pt_in_expanded_ship_bbox() function
 * 
 * 83    5/26/99 4:00p Dave
 * Fixed small lighting bug,
 * 
 * 82    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 81    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 80    5/21/99 5:03p Andsager
 * Add code to display engine wash death.  Modify ship_kill_packet
 * 
 * 79    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 78    5/19/99 11:09a Andsager
 * Turn on engine wash.  Check every 1/4 sec.
 * 
 * 77    5/18/99 1:30p Dave
 * Added muzzle flash table stuff.
 * 
 * 76    5/18/99 12:08p Andsager
 * Added observer_process_post to handle observer too far away
 * 
 * 75    5/18/99 11:15a Andsager
 * Fix bug in mulitplayer max rangel
 * 
 * 74    5/18/99 10:08a Andsager
 * Modified single maximum range before blown up to also be multi
 * friendly.
 * 
 * 73    5/14/99 3:01p Andsager
 * Fix bug in ship_do_cap_subsys_cargo_revealed
 * 
 * 72    5/14/99 1:59p Andsager
 * Multiplayer message for subsystem cargo revealed.
 * 
 * 71    5/14/99 11:50a Andsager
 * Added vaporize for SMALL ships hit by HUGE beams.  Modified dying
 * frame.  Enlarged debris shards and range at which visible.
 * 
 * 70    5/12/99 2:55p Andsager
 * Implemented level 2 tag as priority in turret object selection
 * 
 * 69    5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 68    5/10/99 4:54p Dave
 * Fixed particularly hideous subsystem bug related to multiple ship types
 * using the same model.
 * 
 * 67    4/30/99 12:18p Dave
 * Several minor bug fixes.
 * 
 * 66    4/29/99 2:29p Dave
 * Made flak work much better in multiplayer.
 * 
 * 65    4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 64    4/28/99 3:11p Andsager
 * Stagger turret weapon fire times.  Make turrets smarter when target is
 * protected or beam protected.  Add weaopn range to weapon info struct.
 * 
 * 63    4/27/99 12:16a Dave
 * Fixed beam weapon muzzle glow problem. Fixed premature timeout on the
 * pxo server list screen. Fixed secondary firing for hosts on a
 * standalone. Fixed wacky multiplayer weapon "shuddering" problem.
 * 
 * 62    4/23/99 12:30p Andsager
 * Add debug code for showing attack point against big ships.
 * 
 * 61    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 60    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 59    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 58    4/19/99 12:21p Johnson
 * Allow ships with invisible polygons which do not collide
 * 
 * 57    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 56    4/12/99 10:07p Dave
 * Made network startup more forgiving. Added checkmarks to dogfight
 * screen for players who hit commit.
 * 
 * 55    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 54    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 53    3/30/99 5:40p Dave
 * Fixed reinforcements for TvT in multiplayer.
 * 
 * 52    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 51    3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 50    3/26/99 5:23p Andsager
 * Fix bug with special explostions sometimes not generating shockwaves.
 * 
 * 49    3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 48    3/25/99 4:47p Johnson
 * HACK allow Mycernus to dock with Enif
 * 
 * 47    3/25/99 2:38p Johnson
 * Give brad special mission docking stuff
 * 
 * 46    3/25/99 1:30p Johnson
 * Allow Arcadia/Mentu docking
 * 
 * 45    3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 44    3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 43    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 42    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 43    3/12/99 4:30p Anoop
 * Check for OBJ_NONE as well as OBJ_GHOST when firing secondary weapons
 * 
 * 42    3/11/99 2:22p Dave
 * Fixed a countermeasure firing assert for multiplayer.
 * 
 * 41    3/10/99 6:51p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 40    3/10/99 2:29p Dan
 * disable lod warning for asteroid ships
 * 
 * 39    3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 38    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 37    3/05/99 1:33p Dave
 * Upped subsystem max to 700
 * 
 * 36    3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 35    3/02/99 9:25p Dave
 * Added a bunch of model rendering debug code. Started work on fixing
 * beam weapon wacky firing.
 * 
 * 34    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 33    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 32    2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 31    2/21/99 1:48p Dave
 * Some code for monitoring datarate for multiplayer in detail.
 * 
 * 30    2/19/99 3:52p Neilk
 * Put in some proper handling code for undocking dying objects (handle
 * wacky object types like OBJ_GHOST).
 * 
 * 29    2/11/99 5:22p Andsager
 * Fixed bugs, generalized block Sexp_variables
 * 
 * 28    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 27    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 26    2/03/99 12:42p Andsager
 * Add escort priority.  Modify ship_flags_dlg to include field.  Save and
 * Load.  Add escort priority field to ship.
 * 
 * 25    2/02/99 9:36a Andsager
 * Bash hull strength to zero when ship is killed (by sexp)
 * 
 * 24    1/29/99 2:25p Andsager
 * Added turret_swarm_missiles
 * 
 * 23    1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 22    1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 21    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 20    1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 19    1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 18    1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 17    1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 16    1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 15    1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 14    12/23/98 2:53p Andsager
 * Added ship activation and gas collection subsystems, removed bridge
 * 
 * 13    12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 12    12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 11    12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 10    11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 9     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 8     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 7     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 6     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 5     10/23/98 3:03p Andsager
 * Initial support for changing rotation rate.
 * 
 * 4     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 915   8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 914   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 913   8/17/98 5:07p Dave
 * First rev of corkscrewing missiles.
 * 
 * 912   7/15/98 11:29a Allender
 * quick error dialog to prevent the > 50 ships per mission problem
 * 
 * 911   7/06/98 6:11p Dave
 * More object update stuff.
 * 
 * 910   6/30/98 2:23p Dave
 * Revised object update system. Removed updates for all weapons. Put
 * button info back into control info packet.
 * 
 * 909   6/12/98 2:49p Dave
 * Patch 1.02 changes.
 * 
 * 908   6/10/98 6:46p Lawrance
 * increase SHIP_MULTITEXT_LENGTH to 1500
 * 
 * 906   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 905   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 904   5/24/98 10:50p Mike
 * Fix problem with ships with propagating explosions not being able to
 * kamikaze.
 * 
 * 903   5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 902   5/23/98 12:05a Adam
 * change ship_is_getting_locked() to take weapon range into account
 * 
 * 901   5/22/98 5:32p Andsager
 * Make big ship explosion sounds play all the way through.  remove
 * cur_snd from ship struct.
 * 
 * 900   5/21/98 7:11p Sandeep
 * Increased the buffer size a bit during parse ships.tbl to account for
 * slightly lengthy descriptions
 * 
 * 899   5/21/98 3:31p Allender
 * fix bug where Ship_obj_list was getting overwritten by the exited ships
 * list
 * 
 * 898   5/21/98 1:44p Lawrance
 * add ship_obj list validation
 * 
 * 897   5/21/98 11:33a Lawrance
 * Check aspect_locked_time when determining if another ship is seeking
 * lock
 * 
 * 896   5/19/98 8:42p Andsager
 * Add cur_snd (used for sound management of big ship explosions)
 * 
 * 895   5/18/98 2:50p Peter
 * AL: Check to make sure we don't overflow support_ships[] array in
 * ship_find_repair_ship
 * 
 * 894   5/15/98 11:11p Mike
 * Make game a bit easier based on skill level, mainly at Easy and, to a
 * lesser extent, Medium.
 * 
 * 893   5/15/98 6:45p Hoffoss
 * Made some things not appear in the release version of Fred.
 * 
 * 892   5/15/98 5:37p Hoffoss
 * Added new 'tech description' fields to ship and weapon tables, and
 * added usage of it in tech room.
 * 
 * 891   5/15/98 12:07p Allender
 * make messaging come from proper ships when in team vs. team games.
 * 
 * 890   5/14/98 9:38p Andsager
 * Fixed bug in dying_undock_physics when both ships have ovelapping dying
 * times.
 * 
 * 889   5/13/98 6:54p Dave
 * More sophistication to PXO interface. Changed respawn checking so
 * there's no window for desynchronization between the server and the
 * clients.
 * 
 * 888   5/12/98 10:54p Andsager
 * Add new sound manager for big ship sub-explosion sounds
 * 
 * 887   5/12/98 2:34p Adam
 * re-instated the old nicer particle ANI's for ship explosions.
 * 
 * 886   5/12/98 9:15a Andsager
 * Clean up big ship sub-explosion sound.  Make single instance of sounds.
 * Choose random sounds.  Add timestamp to post split sounds.  Make
 * explosion flash depend on wheth ship was visible if within ~1.5 radii.
 * 
 * 885   5/11/98 4:33p Allender
 * fixed ingame join problems -- started to work on new object updating
 * code (currently ifdef'ed out)
 * 
 * 884   5/11/98 4:05p Lawrance
 * Increase sanity timestamp for next_fire_time to 60 seconds
 * 
 * 883   5/10/98 11:30p Mike
 * Better firing of bombs, less likely to go into strafe mode.
 * 
 * 882   5/09/98 4:52p Lawrance
 * Implement padlock view (up/rear/left/right)
 * 
 * 881   5/08/98 5:31p Hoffoss
 * Isolated the joystick force feedback code more from dependence on other
 * libraries.
 * 
 * 880   5/08/98 4:39p Mike
 * Comment out two Asserts that trap a condition that is actually legal. 
 *
 * $NoKeywords: $
 */

#include <setjmp.h>

#include "globalincs/def_files.h"
#include "ship/ship.h"
#include "object/object.h"
#include "weapon/weapon.h"
#include "radar/radar.h"
#include "render/3d.h"
#include "fireball/fireballs.h"
#include "hud/hud.h"
#include "io/timer.h"
#include "mission/missionlog.h"
#include "io/joy_ff.h"
#include "playerman/player.h"
#include "parse/parselo.h"
#include "freespace2/freespace.h"
#include "globalincs/linklist.h"
#include "hud/hudets.h"
#include "hud/hudshield.h"
#include "hud/hudmessage.h"
#include "ai/aigoals.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "ship/shipfx.h"
#include "gamesequence/gamesequence.h"
#include "object/objectsnd.h"
#include "cmeasure/cmeasure.h"
#include "ship/afterburner.h"
#include "weapon/shockwave.h"
#include "hud/hudsquadmsg.h"
#include "weapon/swarm.h"
#include "ship/subsysdamage.h"
#include "mission/missionmessage.h"
#include "lighting/lighting.h"
#include "particle/particle.h"
#include "ship/shiphit.h"
#include "asteroid/asteroid.h"
#include "hud/hudtargetbox.h"
#include "hud/hudwingmanstatus.h"
#include "jumpnode/jumpnode.h"
#include "missionui/redalert.h"
#include "weapon/corkscrew.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "nebula/neb.h"
#include "ship/shipcontrails.h"
#include "demo/demo.h"
#include "weapon/beam.h"
#include "math/staticrand.h"
#include "missionui/missionshipchoice.h"
#include "hud/hudartillery.h"
#include "species_defs/species_defs.h"
#include "weapon/flak.h"								//phreak addded 11/05/02 for flak primaries
#include "mission/missioncampaign.h"
#include "radar/radarsetup.h"
#include "object/objectdock.h"
#include "object/deadobjectdock.h"
#include "iff_defs/iff_defs.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "object/waypoint/waypoint.h"




#define NUM_SHIP_SUBSYSTEM_SETS			15		// number of subobject sets to use (because of the fact that it's a linked list,
												//     we can't easily go fully dynamic)

#define NUM_SHIP_SUBSYSTEMS_PER_SET		200 	// Reduced from 1000 to 400 by MK on 4/1/98.  DTP; bumped from 700 to 2100
												// Reduced to 200 by taylor on 3/13/07  --  it's managed in dynamically allocated sets now
												//    Highest I saw was 164 in sm2-03a which Sandeep says has a lot of ships.
												//    JAS: sm3-01 needs 460.   You cannot know this number until *all* ships
												//    have warped in.   So I put code in the paging code which knows all ships
												//    that will warp in.

static int Num_ship_subsystems = 0;
static int Num_ship_subsystems_allocated = 0;

static ship_subsys *Ship_subsystems[NUM_SHIP_SUBSYSTEM_SETS] = { NULL };
ship_subsys ship_subsys_free_list;


extern bool splodeing;

extern bool Module_ship_weapons_loaded;
extern float splode_level;

extern int splodeingtexture;

extern int Cmdline_nohtl;

//#define MIN_COLLISION_MOVE_DIST		5.0
//#define COLLISION_VEL_CONST			0.1
#define COLLISION_FRICTION_FACTOR	0.0		// ratio of maximum friction impulse to repulsion impulse
#define COLLISION_ROTATION_FACTOR	1.0		// increase in rotation from collision
#define SHIP_REPAIR_SUBSYSTEM_RATE	0.01f

int	Ai_render_debug_flag=0;
#ifndef NDEBUG
int	Ship_sphere_check = 0;
int	Ship_auto_repair = 1;		// flag to indicate auto-repair of subsystem should occur
extern void render_path_points(object *objp);
#endif

// mwa -- removed 11/24/97 int	num_ships = 0;
int	Num_wings = 0;
int	Num_reinforcements = 0;
ship	Ships[MAX_SHIPS];
ship	*Player_ship;
wing	Wings[MAX_WINGS];
int	ships_inited = 0;
int armor_inited = 0;

int	Starting_wings[MAX_STARTING_WINGS];  // wings player starts a mission with (-1 = none)

// Goober5000
int Squadron_wings[MAX_SQUADRON_WINGS];
int TVT_wings[MAX_TVT_WINGS];

// Goober5000
char Starting_wing_names[MAX_STARTING_WINGS][NAME_LENGTH];
char Squadron_wing_names[MAX_SQUADRON_WINGS][NAME_LENGTH];
char TVT_wing_names[MAX_TVT_WINGS][NAME_LENGTH];

std::vector<engine_wash_info> Engine_wash_info;
//char get_engine_wash_index(char *engine_wash_name);
engine_wash_info *get_engine_wash_pointer(char* engine_wash_name);

// information for ships which have exited the game
exited_ship Ships_exited[MAX_EXITED_SHIPS];
int Num_exited_ships;

int	Num_engine_wash_types = 0;
int	Num_ship_classes = 0;
int	Num_ship_subobj_types;
int	Num_ship_subobjects;
int	Player_ship_class;	// needs to be player specific, move to player structure	

#define		SHIP_OBJ_USED	(1<<0)				// flag used in ship_obj struct
#define		MAX_SHIP_OBJS	MAX_SHIPS			// max number of ships tracked in ship list
ship_obj		Ship_objs[MAX_SHIP_OBJS];		// array used to store ship object indexes
ship_obj		Ship_obj_list;							// head of linked list of ship_obj structs

ship_info		Ship_info[MAX_SHIP_CLASSES];
reinforcements	Reinforcements[MAX_REINFORCEMENTS];
std::vector<ship_info> Ship_templates;

static char **tspecies_names = NULL;

std::vector<ship_type_info> Ship_types;

std::vector<ArmorType> Armor_types;

flag_def_list Man_types[] = {
	{ "Bank right",		MT_BANK_RIGHT,	0 },
	{ "Bank left",		MT_BANK_LEFT,	0 },
	{ "Pitch up",		MT_PITCH_UP,	0 },
	{ "Pitch down",		MT_PITCH_DOWN,	0 },
	{ "Roll right",		MT_ROLL_RIGHT,	0 },
	{ "Roll left",		MT_ROLL_LEFT,	0 },
	{ "Slide right",	MT_SLIDE_RIGHT,	0 },
	{ "Slide left",		MT_SLIDE_LEFT,	0 },
	{ "Slide up",		MT_SLIDE_UP,	0 },
	{ "Slide down",		MT_SLIDE_DOWN,	0 },
	{ "Forward",		MT_FORWARD,		0 },
	{ "Reverse",		MT_REVERSE,		0 }
};

int Num_man_types = sizeof(Man_types)/sizeof(flag_def_list);

flag_def_list Man_thruster_flags[] = 
{
	{ "no scale",	MTF_NO_SCALE},
};

int Num_man_thruster_flags = sizeof(Man_thruster_flags) / sizeof(flag_def_list);

// Goober5000 - I figured we should keep this separate
// from Comm_orders, considering how I redid it :p
// (and also because we may want to change either
// the order text or the flag text in the future)
flag_def_list Player_orders[] = {
	// common stuff
	{ "attack ship",		ATTACK_TARGET_ITEM,		0 },
	{ "disable ship",		DISABLE_TARGET_ITEM,	0 },
	{ "disarm ship",		DISARM_TARGET_ITEM,		0 },
	{ "disable subsys",		DISABLE_SUBSYSTEM_ITEM,	0 },
	{ "guard ship",			PROTECT_TARGET_ITEM,	0 },
	{ "ignore ship",		IGNORE_TARGET_ITEM,		0 },
	{ "form on wing",		FORMATION_ITEM,			0 },
	{ "cover me",			COVER_ME_ITEM,			0 },
	{ "attack any",			ENGAGE_ENEMY_ITEM,		0 },
	{ "depart",				DEPART_ITEM,			0 },

	// transports mostly
	{ "dock",				CAPTURE_TARGET_ITEM,	0 },

	// support
	{ "rearm me",			REARM_REPAIR_ME_ITEM,	0 },
	{ "abort rearm",		ABORT_REARM_REPAIR_ITEM,	0 },

	// extra stuff for support
	{ "stay near me",		STAY_NEAR_ME_ITEM,		0 },
	{ "stay near ship",		STAY_NEAR_TARGET_ITEM,	0 },
	{ "keep safe dist",		KEEP_SAFE_DIST_ITEM,	0 }
};

int Num_player_orders = sizeof(Player_orders)/sizeof(flag_def_list);

flag_def_list Subsystem_flags[] = {
	{ "untargetable",		MSS_FLAG_UNTARGETABLE,		0 },
	{ "carry no damage",	MSS_FLAG_CARRY_NO_DAMAGE,	0 },
	{ "use multiple guns",	MSS_FLAG_USE_MULTIPLE_GUNS,	0 },
	{ "fire down normals",	MSS_FLAG_FIRE_ON_NORMAL,	0 }
};

int Num_subsystem_flags = sizeof(Subsystem_flags)/sizeof(flag_def_list);


// NOTE: a var of:
//         "0"    means that it's a SIF_* flag
//         "1"    means that it's a SIF2_* flag
//         "255"  means that the option is obsolete and a warning should be generated
flag_def_list Ship_flags[] = {
	{ "no_collide",					SIF_NO_COLLIDE,				0 },
	{ "player_ship",				SIF_PLAYER_SHIP,			0 },
	{ "default_player_ship",		SIF_DEFAULT_PLAYER_SHIP,	0 },
	{ "repair_rearm",				SIF_SUPPORT,				0 },
	{ "cargo",						SIF_CARGO,					0 },
	{ "fighter",					SIF_FIGHTER,				0 },
	{ "bomber",						SIF_BOMBER,					0 },
	{ "transport",					SIF_TRANSPORT,				0 },
	{ "freighter",					SIF_FREIGHTER,				0 },
	{ "capital",					SIF_CAPITAL,				0 },
	{ "supercap",					SIF_SUPERCAP,				0 },
	{ "drydock",					SIF_DRYDOCK,				0 },
	{ "cruiser",					SIF_CRUISER,				0 },
	{ "navbuoy",					SIF_NAVBUOY,				0 },
	{ "sentrygun",					SIF_SENTRYGUN,				0 },
	{ "escapepod",					SIF_ESCAPEPOD,				0 },
	{ "stealth",					SIF_STEALTH,				0 },
	{ "no type",					SIF_NO_SHIP_TYPE,			0 },
	{ "ship copy",					SIF_SHIP_COPY,				0 },
	{ "in tech database",			SIF_IN_TECH_DATABASE | SIF_IN_TECH_DATABASE_M,	0 },
	{ "in tech database multi",		SIF_IN_TECH_DATABASE_M,		0 },
	{ "dont collide invisible",		SIF_SHIP_CLASS_DONT_COLLIDE_INVIS,	0 },
	{ "big damage",					SIF_BIG_DAMAGE,				0 },
	{ "corvette",					SIF_CORVETTE,				0 },
	{ "gas miner",					SIF_GAS_MINER,				0 },
	{ "awacs",						SIF_AWACS,					0 },
	{ "knossos",					SIF_KNOSSOS_DEVICE,			0 },
	{ "no_fred",					SIF_NO_FRED,				0 },
	{ "flash",						SIF2_FLASH,					1 },
	{ "surface shields",			SIF2_SURFACE_SHIELDS,		1 },
	{ "show ship",					SIF2_SHOW_SHIP_MODEL,		1 },
	{ "generate icon",				SIF2_GENERATE_HUD_ICON,		1 },
	{ "no weapon damage scaling",	SIF2_DISABLE_WEAPON_DAMAGE_SCALING,	1 },
	{ "gun convergence",			SIF2_GUN_CONVERGENCE,		1 },

	// to keep things clean, obsolete options go last
	{ "ballistic primaries",		-1,		255 }
};

const int Num_ship_flags = sizeof(Ship_flags) / sizeof(flag_def_list);


/*
int Num_player_ship_precedence;				// Number of ship types in Player_ship_precedence
int Player_ship_precedence[MAX_PLAYER_SHIP_CHOICES];	// Array of ship types, precedence list for player ship/wing selection
*/
static int Laser_energy_out_snd_timer;	// timer so we play out of laser sound effect periodically
static int Missile_out_snd_timer;	// timer so we play out of laser sound effect periodically

// structure used to hold ship counts of particular types.  The order in which these appear is crucial
// since the goal code relies on this placement to find the array index in the Ship_counts array
//WMC - This should be fixed with new system.

std::vector<ship_counts>	Ship_type_counts;

// I don't want to do an AI cargo check every frame, so I made a global timer to limit check to
// every SHIP_CARGO_CHECK_INTERVAL ms.  Didn't want to make a timer in each ship struct.  Ensure
// inited to 1 at mission start.
static int Ship_cargo_check_timer;

static int Thrust_anim_inited = 0;


// set the ship_obj struct fields to default values
void ship_obj_list_reset_slot(int index)
{
	Ship_objs[index].flags = 0;
	Ship_objs[index].next = NULL;
	Ship_objs[index].prev = (ship_obj*)-1;
}

// if the given ship is in my squadron wings
// Goober5000
int ship_in_my_squadron(ship *shipp)
{
	int i;

	for (i=0; i<MAX_STARTING_WINGS; i++)
	{
		if (shipp->wingnum == Starting_wings[i])
			return 1;
	}

	for (i=0; i<MAX_TVT_WINGS; i++)
	{
		if (shipp->wingnum == TVT_wings[i])
			return 1;
	}

	// not in
	return 0;
}

// ---------------------------------------------------
// ship_obj_list_init()
//
void ship_obj_list_init()
{
	int i;

	list_init(&Ship_obj_list);
	for ( i = 0; i < MAX_SHIP_OBJS; i++ ) {
		ship_obj_list_reset_slot(i);
	}
}

// ---------------------------------------------------
// ship_obj_list_add()
//
// Function to add a node to the Ship_obj_list.  Only
// called from ship_create()
int ship_obj_list_add(int objnum)
{
	int i;

	for ( i = 0; i < MAX_SHIP_OBJS; i++ ) {
		if ( !(Ship_objs[i].flags & SHIP_OBJ_USED) )
			break;
	}
	if ( i == MAX_SHIP_OBJS ) {
		Error(LOCATION, "Fatal Error: Ran out of ship object nodes\n");
		return -1;
	}
	
	Ship_objs[i].flags = 0;
	Ship_objs[i].objnum = objnum;
	list_append(&Ship_obj_list, &Ship_objs[i]);
	Ship_objs[i].flags |= SHIP_OBJ_USED;

	return i;
}

// ---------------------------------------------------
// ship_obj_list_remove()
//
// Function to remove a node from the Ship_obj_list.  Only
// called from ship_delete()
void ship_obj_list_remove(int index)
{
	Assert(index >= 0 && index < MAX_SHIP_OBJS);
	list_remove( Ship_obj_list, &Ship_objs[index]);	
	ship_obj_list_reset_slot(index);
}

// ---------------------------------------------------
// ship_obj_list_rebuild()
//
// Called from the save/restore code to re-create the Ship_obj_list
//
void ship_obj_list_rebuild()
{
	object *objp;

	ship_obj_list_init();

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type == OBJ_SHIP ) {
			Ships[objp->instance].ship_list_index = ship_obj_list_add(OBJ_INDEX(objp));
		}
	}
}

ship_obj *get_ship_obj_ptr_from_index(int index)
{
	Assert(index >= 0 && index < MAX_SHIP_OBJS);
	return &Ship_objs[index];
}


// return number of ships in the game.
int ship_get_num_ships()
{
	int count;
	ship_obj *so;

	count = 0;
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )
		count++;

	return count;
}

engine_wash_info::engine_wash_info()
{
	name[0] = '\0';
	angle = PI / 10.0f;
	radius_mult = 1.0f;
	length = 500.0f;
	intensity = 1.0f;
}

// parse an engine wash info record
void parse_engine_wash(bool replace)
{
	engine_wash_info ewt;
	engine_wash_info *ewp;
	bool create_if_not_found  = true;
	bool first_time = true;

	// name of engine wash info
	required_string("$Name:");
	stuff_string(ewt.name, F_NAME, NAME_LENGTH);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for engine wash in non-modular table");
		}
		create_if_not_found = false;
	}

	//Does this engine wash exist already?
	//If so, load this new info into it
	//Otherwise, increment Num_engine_wash_types
	ewp = get_engine_wash_pointer(ewt.name);
	if(ewp != NULL)
	{
		if(replace)
		{
			nprintf(("Warning", "More than one version of engine wash %s exists; using newer version.", ewt.name));
		}
		else
		{
			Error(LOCATION, "Error:  Engine wash %s already exists.  All engine wash names must be unique.", ewt.name);
		}
		first_time = false;
	}
	else
	{
		//Don't create engine wash if it has +nocreate and is in a modular table.
		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return;
		}
		Engine_wash_info.push_back(ewt);
		ewp = &Engine_wash_info[Num_engine_wash_types++];
	}


	// half angle of cone of wash from thruster
	if(optional_string("$Angle:"))
	{
		stuff_float(&ewp->angle);
		ewp->angle *= (PI / 180.0f);
	}

	// radius multiplier for hemisphere around thruster pt
	if(optional_string("$Radius Mult:")) {
		stuff_float(&ewp->radius_mult);
	}

	// length of cone
	if(optional_string("$Length:")) {
		stuff_float(&ewp->length);
	}

	// intensity inside hemisphere (or at 0 distance from frustated cone)
	if(optional_string("$Intensity:")) {
		stuff_float(&ewp->intensity);
	}
}

char *Warp_types[] = {
	"Default",
	"BTRL"
};

int Num_warp_types = sizeof(Warp_types)/sizeof(char*);

int warptype_match(char *p)
{
	int i;
	for(i = 0; i < Num_warp_types; i++)
	{
		if(!stricmp(Warp_types[i], p))
			return i;
	}

	return -1;
}


// Kazan -- Volition had this set to 1500, Set it to 4K for WC Saga
//#define SHIP_MULTITEXT_LENGTH 1500
#define SHIP_MULTITEXT_LENGTH 4096
char current_ship_table[MAX_PATH_LEN + MAX_FILENAME_LEN];


//Writes default info to a ship entry
//Result: Perfectly valid ship_info entry, just with no name
//Called from parse_ship so that modular tables are cumulative,
//rather than simply replacing the previous entry
void init_ship_entry(ship_info *sip)
{
	int i,j;
	
	sip->name[0] = '\0';
	sprintf(sip->short_name, "AShipClass");
	sip->species = 0;
	sip->class_type = -1;
	
	sip->type_str = sip->maneuverability_str = sip->armor_str = sip->manufacturer_str = NULL;
	sip->desc = NULL;
	sip->tech_desc = NULL;
	sip->ship_length = NULL;
	sip->gun_mounts = NULL;
	sip->missile_banks = NULL;
	
	sip->num_detail_levels = 1;
	sip->detail_distance[0] = 0;
	sip->hud_target_lod = -1;
	strcpy(sip->pof_file, "");
	strcpy(sip->pof_file_hud, "");
	
	sip->num_nondark_colors = 0;
	
	sip->density = 1.0f;
	sip->damp = 0.0f;
	sip->rotdamp = 0.0f;
	vm_vec_zero(&sip->max_vel);
	sip->max_speed = 0.0f;
	vm_vec_zero(&sip->rotation_time);
	vm_vec_zero(&sip->max_rotvel);
	sip->srotation_time = 0.0f;
	sip->max_rear_vel = 0.0f;
	sip->forward_accel = 0.0f;
	sip->forward_decel = 0.0f;
	sip->slide_accel = 0.0f;
	sip->slide_decel = 0.0f;
	
	sip->can_glide = false;
	sip->glide_cap = 0.0f;

	sip->warpin_speed = 0.0f;
	sip->warpout_speed = 0.0f;
	sip->warpin_radius = 0.0f;
	sip->warpout_radius = 0.0f;
	sip->warpin_time = 0;
	sip->warpout_time = 0;
	sip->warpin_type = WT_DEFAULT;
	sip->warpout_type = WT_DEFAULT;
	sip->warpout_player_speed = 0.0f;
	
	sip->explosion_propagates = 0;
	sip->shockwave_count = 1;
/*	sip->inner_rad = 0.0f;
	sip->outer_rad = 0.0f;
	sip->damage = 0.0f;
	sip->blast = 0.0f;
	sip->shockwave_speed = 0.0f;
	sip->shockwave_model = -1;
	strcpy(sip->shockwave_pof_file, "");
	sip->shockwave_info_index = -1;
	strcpy(sip->shockwave_name,"");*/

	//WMC - the rest of these are in ShipFX.cpp
	particle_emitter *pe = &sip->dspew;
	pe->min_rad = 0.7f;				// Min radius
	pe->max_rad = 1.3f;				// Max radius
	pe->min_vel = 3.0f;				// How fast the slowest particle can move
	pe->max_vel = 12.0f;			// How fast the fastest particle can move

	//WMC - this is mostly here, other stuff calculated in shipfx.cpp
	pe = &sip->ispew;
	pe->normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
	pe->min_rad = 0.20f;			// Min radius
	pe->max_rad = 0.50f;			// Max radius
	pe->num_low  = 25;				// Lowest number of particles to create (hardware)
	pe->num_high = 30;				// Highest number of particles to create (hardware)
	pe->normal_variance = 1.0f;		//	How close they stick to that normal 0=good, 1=360 degree
	pe->min_vel = 2.0f;				// How fast the slowest particle can move
	pe->max_vel = 12.0f;			// How fast the fastest particle can move
	pe->min_life = 0.05f;			// How long the particles live
	pe->max_life = 0.55f;			// How long the particles live

	sip->debris_min_lifetime = -1.0f;
	sip->debris_max_lifetime = -1.0f;
	sip->debris_min_speed = -1.0f;
	sip->debris_max_speed = -1.0f;
	sip->debris_min_rotspeed = -1.0f;
	sip->debris_max_rotspeed = -1.0f;
	for ( i = 0; i < MAX_WEAPON_TYPES; i++ )
	{
		sip->allowed_weapons[i] = 0;
	}

	for ( i = 0; i < MAX_SHIP_WEAPONS; i++ )
	{
		sip->restricted_loadout_flag[i] = 0;
		for ( j = 0; j < MAX_WEAPON_TYPES; j++ )
		{
			sip->allowed_bank_restricted_weapons[i][j] = 0;
		}
	}
	
	sip->draw_models = false;
	sip->weapon_model_draw_distance = 200.0f;

	sip->num_primary_banks = 0;
	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ )
	{
		sip->primary_bank_weapons[i] = -1;
		sip->draw_primary_models[i] = false;
		sip->primary_bank_ammo_capacity[i] = 0;
	}
	
	sip->num_secondary_banks = 0;
	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ )
	{
		sip->secondary_bank_weapons[i] = -1;
		sip->draw_secondary_models[i] = false;
		sip->secondary_bank_ammo_capacity[i] = 0;
	}
	
	sip->max_shield_strength = 0.0f;
	sip->shield_color[0] = 255;
	sip->shield_color[1] = 255;
	sip->shield_color[2] = 255;
	
	sip->power_output = 0.0f;
	sip->max_overclocked_speed = 0.0f;
	sip->max_weapon_reserve = 0.0f;
	sip->max_shield_regen_per_second = 0.0f;
	sip->max_weapon_regen_per_second = 0.0f;
	
	sip->max_hull_strength = 100.0f;
	
	sip->hull_repair_rate = 0.0f;
	//-2 represents not set, in which case the default is used for the ship (if it is small)
	sip->subsys_repair_rate = -2.0f;
	
	sip->armor_type_idx = -1;
	sip->flags = SIF_DEFAULT_VALUE;
	sip->flags2 = SIF2_DEFAULT_VALUE;
	sip->ai_class = 0;
	
	sip->afterburner_max_vel.xyz.x = 0.0f;
	sip->afterburner_max_vel.xyz.y = 0.0f;
	sip->afterburner_max_vel.xyz.z = 0.0f;
	
	vm_vec_zero(&sip->afterburner_max_vel);
	sip->afterburner_forward_accel = 0.0f;
	sip->afterburner_fuel_capacity = 0.0f;
	sip->afterburner_burn_rate = 0.0f;
	sip->afterburner_recover_rate = 0.0f;

	generic_bitmap_init(&sip->afterburner_trail, NULL);
	sip->afterburner_trail_width_factor = 1.0f;
	sip->afterburner_trail_alpha_factor = 1.0f;
	sip->afterburner_trail_life = 5.0f;
	
	sip->cmeasure_type = Default_cmeasure_index;
	sip->cmeasure_max = 0;

	sip->scan_time = 2000;
	
	sip->engine_snd = -1;
	
	vm_vec_zero(&sip->closeup_pos);
	sip->closeup_zoom = 0.5f;
	
	sip->topdown_offset_def = false;
	vm_vec_zero(&sip->topdown_offset);
	
	sip->shield_icon_index = 255;		// stored as ubyte
	sip->icon_filename[0] = 0;
	sip->anim_filename[0] = 0;
	sip->overhead_filename[0] = 0;
	sip->score = 0;

	// Bobboau's thruster stuff
	generic_anim_init( &sip->thruster_glow_info.normal );
	generic_anim_init( &sip->thruster_glow_info.afterburn );
	generic_bitmap_init( &sip->thruster_secondary_glow_info.normal );
	generic_bitmap_init( &sip->thruster_secondary_glow_info.afterburn );
	generic_bitmap_init( &sip->thruster_tertiary_glow_info.normal );
	generic_bitmap_init( &sip->thruster_tertiary_glow_info.afterburn );

	// Bobboau's thruster stuff
	sip->thruster01_glow_rad_factor = 1.0f;
	sip->thruster02_glow_rad_factor = 1.0f;
	sip->thruster03_glow_rad_factor = 1.0f;
	sip->thruster02_glow_len_factor = 1.0f;
	
//	sip->thruster_particle_bitmap01 = -1;

//	strcpy(sip->thruster_particle_bitmap01_name,"thrusterparticle");

	sip->splodeing_texture = -1;
	strcpy(sip->splodeing_texture_name, "boom");

	sip->normal_thruster_particles.clear();
	sip->afterburner_thruster_particles.clear();

	memset(&sip->ct_info, 0, sizeof(trail_info) * MAX_SHIP_CONTRAILS);
	sip->ct_count = 0;
	
	sip->n_subsystems = 0;
	sip->subsystems = NULL;

	sip->model_num = -1;
	sip->model_num_hud = -1;
}

// function to parse the information for a specific ship type.	
int parse_ship(bool replace)
{
	char buf[SHIP_MULTITEXT_LENGTH];
	ship_info *sip;
	bool create_if_not_found  = true;
	int rtn = 0;
	char name_tmp[NAME_LENGTH];

	required_string("$Name:");
	stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for ship in non-modular table");
		}
		create_if_not_found = false;
	}

	strcpy(parse_error_text, "\nin ship: ");
	strcat(parse_error_text, buf);

#ifdef NDEBUG
	if (get_pointer_to_first_hash_symbol(buf) && Fred_running)
		rtn = 1;
#endif

	//Remove @ symbol
	//these used to be used to denote weapons that would
	//only be parsed in demo builds
	if ( buf[0] == '@' ) {
		backspace(buf);
	}

	diag_printf ("Ship name -- %s\n", buf);
	//Check if ship exists already
	int ship_id;
	bool first_time = false;
	ship_id = ship_info_lookup( buf );
	
	if(ship_id != -1)
	{
		sip = &Ship_info[ship_id];
		if(!replace)
		{
			Warning(LOCATION, "Error:  Ship name %s already exists in %s.  All ship class names must be unique.", sip->name, current_ship_table);
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return -1;
		}
	}
	else
	{
		//Don't create ship if it has +nocreate and is in a modular table.
		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}

			return -1;
		}
		
		//Check if there are too many ship classes
		if(Num_ship_classes >= MAX_SHIP_CLASSES) {
			Warning(LOCATION, "Too many ship classes before '%s'; maximum is %d, so only the first %d will be used", buf, MAX_SHIP_CLASSES, Num_ship_classes);
			
			//Skip the rest of the ships in non-modular tables, since we can't add them.
			//WMC - nm, skip just one.
			//if(!replace) {
				skip_to_start_of_string_either("$Name:", "#End");
			//}
			return -1;
		}
		
		//Init vars
		sip = &Ship_info[Num_ship_classes];
		first_time = true;
		init_ship_entry(sip);
		strcpy(sip->name, buf);
		Num_ship_classes++;
	}
	
	// Use a template for this ship.
	if( optional_string( "+Use Template:" ) ) {
		// Should never resolve to true, but just in case...
		if( !create_if_not_found ) {
			Warning(LOCATION, "Both '+nocreate' and '+Use Template:' were specified for ship class '%s', ignoring '+Use Template:'", buf);
		}
		else {
			char template_name[SHIP_MULTITEXT_LENGTH];
			stuff_string(template_name, F_NAME, SHIP_MULTITEXT_LENGTH);
			int template_id = ship_template_lookup( template_name);
			if ( template_id != -1 ) {
				first_time = false;
				memcpy(sip, &Ship_templates[template_id], sizeof(ship_info));
				strcpy(sip->name, buf);
			}
			else {
				Warning(LOCATION, "Unable to find ship template '%s' requested by ship class '%s', ignoring template request...", template_name, buf);
			}
		}
	}
	
	rtn = parse_ship_values(sip, false, first_time, replace);

	// if we have a ship copy, then check to be sure that our base ship exists
	// This should really be moved -C
	// Goober5000 - made nonfatal and a bit clearer
	if (sip->flags & SIF_SHIP_COPY)
	{
		strcpy(name_tmp, sip->name);

		if (end_string_at_first_hash_symbol(name_tmp))
		{
			if (ship_info_lookup(name_tmp) < 0)
			{
				Warning(LOCATION, "Ship %s is a copy, but base ship %s couldn't be found.", sip->name, name_tmp);
				sip->flags &= ~SIF_SHIP_COPY;
			}
		}
		else
		{
			Warning(LOCATION, "Ships %s is a copy, but does not use the ship copy name extension.");
			sip->flags &= ~SIF_SHIP_COPY;
		}
	}

	return rtn;	//0 for success
}

// function to parse the information for a specific ship type template.
int parse_ship_template()
{
	char buf[SHIP_MULTITEXT_LENGTH];
	ship_info new_template;
	ship_info *sip = &new_template;
	int rtn = 0;

	required_string("$Template:");
	stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);

	if( optional_string("+nocreate") ) {
			Warning(LOCATION, "+nocreate flag used on ship template. Ship templates can not be modified. Ignoring +nocreate.");
	}

	strcpy(parse_error_text, "\nin ship template: ");
	strcat(parse_error_text, buf);

	diag_printf ("Ship template name -- %s\n", buf);
	//Check if the template exists already
	int template_id;
	template_id = ship_template_lookup( buf );
	
	if( template_id != -1 ) {
		sip = &Ship_templates[template_id];
		Warning(LOCATION, "Error:  Ship template %s already exists. All ship template names must be unique.", sip->name);
		if ( !skip_to_start_of_string_either("$Template:", "#End")) {
			Int3();
		}
		return -1;
	}
	else {
		
		init_ship_entry(sip);
		strcpy(sip->name, buf);
		//Use another template for this template. This allows for template heirarchies. - Turey
		if( optional_string("+Use Template:") ) {
			char template_name[SHIP_MULTITEXT_LENGTH];
			stuff_string(template_name, F_NAME, SHIP_MULTITEXT_LENGTH);
			int template_id = ship_template_lookup( template_name);
			
			if ( template_id != -1 ) {
				memcpy(sip, &Ship_templates[template_id], sizeof(ship_info));
				strcpy(sip->name, buf);
			}
			else {
				Warning(LOCATION, "Unable to find ship template '%s' requested by ship template '%s', ignoring template request...", template_name, buf);
			}
		}
	}

	rtn = parse_ship_values( sip, true, true, false );
		
	// Now that we're done everything, check to see if the template exists already, and if it doesn't, add it to the vector.
	if ( ship_template_lookup( sip->name ) != -1 ) {
		Warning(LOCATION, "Ship Template '%s' already exists, discarding duplicate...", sip->name);
	}
	else {
		Ship_templates.push_back(*sip);
	}

	return rtn;
}

// Puts values into a ship_info.
int parse_ship_values(ship_info* sip, bool isTemplate, bool first_time, bool replace)
{
	char buf[SHIP_MULTITEXT_LENGTH];
	char* info_type_name;
	int i, j, num_allowed;
	int allowed_weapons[MAX_WEAPON_TYPES];
	int pbank_capacity_count, sbank_capacity_count;
	int rtn = 0;
	char name_tmp[NAME_LENGTH];

	if ( !isTemplate ) {
		info_type_name = "Ship Class";
	}
	else {
		info_type_name = "Ship Template";
	}	
	
	if( optional_string("$Short name:") ) {
		stuff_string(sip->short_name, F_NAME, NAME_LENGTH);
	}
	else if (first_time) {
		char *srcpos, *srcend, *destpos, *destend;
		srcpos = sip->name;
		destpos = sip->short_name;
		srcend = srcpos + strlen(sip->name);
		destend = destpos + sizeof(sip->short_name) - 1;
		while( srcpos < srcend ) {
			if(*srcpos != ' ') {
				*destpos++ = *srcpos++;
			}
			else {
				srcpos++;
			}
		}
	}
	diag_printf ("%s short name -- %s\n", info_type_name, sip->short_name);

	Assert( tspecies_names );
	find_and_stuff_optional("$Species:", &sip->species, F_NAME, tspecies_names, Species_info.size(), "species names");

	diag_printf ("%s species -- %s\n", info_type_name, Species_info[sip->species].species_name);

	if ( optional_string("+Type:") ) {
		stuff_malloc_string(&sip->type_str, F_MESSAGE);
	}

	if ( optional_string("+Maneuverability:") ) {
		stuff_malloc_string(&sip->maneuverability_str, F_MESSAGE);
	}

	if ( optional_string("+Armor:") ) {
		stuff_malloc_string(&sip->armor_str, F_MESSAGE);
	}

	if ( optional_string("+Manufacturer:") ) {
		stuff_malloc_string(&sip->manufacturer_str, F_MESSAGE);
	}


	if ( optional_string("+Description:") ) {
		stuff_malloc_string(&sip->desc, F_MULTITEXT, NULL, SHIP_MULTITEXT_LENGTH);
	}

	
	if ( optional_string("+Tech Description:") ) {
		stuff_malloc_string(&sip->tech_desc, F_MULTITEXT, NULL, SHIP_MULTITEXT_LENGTH);
	}

	// Code added here by SS to parse the optional strings for length, gun_mounts, missile_banks

	if ( optional_string("+Length:") ) {
		stuff_malloc_string(&sip->ship_length, F_MESSAGE);
	}
	
	if ( optional_string("+Gun Mounts:") ) {
		stuff_malloc_string(&sip->gun_mounts, F_MESSAGE);
	}
	
	if ( optional_string("+Missile Banks:") ) {
		stuff_malloc_string(&sip->missile_banks, F_MESSAGE);
	}

	// End code by SS

	if( optional_string( "$POF file:" ) ) {
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// assume we're using this file name
		bool valid = true;

		// Goober5000 - if this is a modular table, and we're replacing an existing file name, and the file doesn't exist, don't replace it
		if ( replace ) {
			if ( strlen(sip->pof_file) > 0 ) {
				if ( !cf_exists_full(temp, CF_TYPE_MODELS) ) {
					valid = false;
				}
			}
		}
		if ( valid ) {
			strcpy(sip->pof_file, temp);
		}
	}

	// ship class texture replacement - Goober5000 and taylor
	int PLACEHOLDER_num_texture_replacements = 0;
	char PLACEHOLDER_old_texture[TEXTURE_NAME_LENGTH];
	char PLACEHOLDER_new_texture[TEXTURE_NAME_LENGTH];
	int PLACEHOLDER_new_texture_id;
	if (optional_string("$Texture Replace:"))
	{
		char *p;

		while ((PLACEHOLDER_num_texture_replacements < MAX_MODEL_TEXTURES) && (optional_string("+old:")))
		{
			stuff_string(PLACEHOLDER_old_texture, F_NAME, MAX_FILENAME_LEN);
			required_string("+new:");
			stuff_string(PLACEHOLDER_new_texture, F_NAME, MAX_FILENAME_LEN);

			// get rid of extensions
			p = strchr(PLACEHOLDER_old_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", PLACEHOLDER_old_texture));
				*p = 0;
			}
			p = strchr(PLACEHOLDER_new_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", PLACEHOLDER_new_texture));
				*p = 0;
			}

			// load the texture
			PLACEHOLDER_new_texture_id = bm_load(PLACEHOLDER_new_texture);

			if (PLACEHOLDER_new_texture_id < 0)
			{
				Warning(LOCATION, "Could not load replacement texture %s for %s %s\n", PLACEHOLDER_new_texture, info_type_name, sip->name);
			}

			// increment
			PLACEHOLDER_num_texture_replacements++;
		}
	}

	// optional hud targeting model
	if( optional_string( "$POF target file:") ) {
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// assume we're using this file name
		bool valid = true;

		// Goober5000 - if this is a modular table, and we're replacing an existing file name, and the file doesn't exist, don't replace it
		if ( replace ) {
			if ( strlen(sip->pof_file) > 0 ) {
				if ( !cf_exists_full(temp, CF_TYPE_MODELS) ) {
					valid = false;
				}
			}
		}

		if ( valid ) {
			strcpy(sip->pof_file_hud, temp);
		}
	}

	// optional hud target LOD if not using special hud model
	if ( optional_string( "$POF target LOD:" ) ) {
		stuff_int(&sip->hud_target_lod);
	}

	if( optional_string("$Detail distance:") ) {
		sip->num_detail_levels = stuff_int_list(sip->detail_distance, MAX_SHIP_DETAIL_LEVELS, RAW_INTEGER_TYPE);
	}

	// check for optional pixel colors
	while( optional_string("$ND:") ) {
		ubyte nr, ng, nb;
		stuff_ubyte(&nr);
		stuff_ubyte(&ng);
		stuff_ubyte(&nb);

		if( sip->num_nondark_colors < MAX_NONDARK_COLORS ) {
			sip->nondark_colors[sip->num_nondark_colors][0] = nr;
			sip->nondark_colors[sip->num_nondark_colors][1] = ng;
			sip->nondark_colors[sip->num_nondark_colors++][2] = nb;
		}
	}

	if( optional_string("$Show damage:") ) {
		int bogus_bool;
		stuff_boolean(&bogus_bool);
	}

	//HACK -
	//This should really be reworked so that all particle fields
	//are settable, but erg, just not happening right now -C
	//WMC - oh hey I actually got back to this. Whaddya know.
	if(optional_string("$Impact Spew:"))
	{
		parse_particle_emitter(&sip->ispew);
	}
	if(optional_string("$Damage Spew:"))
	{
		parse_particle_emitter(&sip->dspew);
	}

	//Debris crap
#define SD_INITIAL_VEL			(1<<0)
#define SD_MAX_VEL				(1<<1)
#define SD_INITIAL_ROTVEL		(1<<2)
#define SD_MAX_ROTVEL			(1<<3)
	if(optional_string("$Debris:"))
	{
		if(optional_string("+Min Lifetime:"))	{
			stuff_float(&sip->debris_min_lifetime);
			if(sip->debris_min_lifetime < 0.0f)
				Warning(LOCATION, "Debris min speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Lifetime:"))	{
			stuff_float(&sip->debris_max_lifetime);
			if(sip->debris_max_lifetime < 0.0f)
				Warning(LOCATION, "Debris max speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Min Speed:"))	{
			stuff_float(&sip->debris_min_speed);
			if(sip->debris_min_speed < 0.0f)
				Warning(LOCATION, "Debris min speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Speed:"))	{
			stuff_float(&sip->debris_max_speed);
			if(sip->debris_max_speed < 0.0f)
				Warning(LOCATION, "Debris max speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Min Rotation speed:"))	{
			stuff_float(&sip->debris_min_rotspeed);
			if(sip->debris_min_rotspeed < 0.0f)
				Warning(LOCATION, "Debris min speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Rotation speed:"))	{
			stuff_float(&sip->debris_max_rotspeed);
			if(sip->debris_max_rotspeed < 0.0f)
				Warning(LOCATION, "Debris max speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		/*
		if(optional_string("+Initial Velocity:")) {
			sip->debris_flags |= SD_INITIAL_VEL;
			stuff_vector(&sip->debris_initial_vel);
		}

		if(optional_string("+Max Velocity:")) {
			sip->debris_flags |= SD_MAX_VEL;
			stuff_vector(&sip->debris_max_vel);
		}

		if(optional_string("+Initial Rotvel:")) {
			sip->debris_flags |= SD_INITIAL_ROTVEL;
			stuff_vector(&sip->debris_initial_rotvel);
		}

		if(optional_string("+Max Rotvel:")) {
			sip->debris_flags |= SD_MAX_ROTVEL;
			stuff_vector(&sip->debris_max_rotvel);
		}
		*/
	}
	//WMC - sanity checking
	if(sip->debris_min_speed > sip->debris_max_speed && sip->debris_max_speed >= 0.0f) {
		Warning(LOCATION, "Debris min speed (%f) on %s '%s' is greater than debris max speed (%f), and will be set to debris max speed.", sip->debris_min_speed, info_type_name, sip->name, sip->debris_max_speed);
		sip->debris_min_speed = sip->debris_max_speed;
	}
	if(sip->debris_min_rotspeed > sip->debris_max_rotspeed && sip->debris_max_rotspeed >= 0.0f) {
		Warning(LOCATION, "Debris min rotation speed (%f) on %s '%s' is greater than debris max rotation speed (%f), and will be set to debris max rotation speed.", sip->debris_min_rotspeed, info_type_name, sip->name, sip->debris_max_rotspeed);
		sip->debris_min_rotspeed = sip->debris_max_rotspeed;
	}
	if(sip->debris_min_lifetime > sip->debris_max_lifetime && sip->debris_max_lifetime >= 0.0f) {
		Warning(LOCATION, "Debris min lifetime (%f) on %s '%s' is greater than debris max lifetime (%f), and will be set to debris max lifetime.", sip->debris_min_lifetime, info_type_name, sip->name, sip->debris_max_lifetime);
		sip->debris_min_lifetime = sip->debris_max_lifetime;
	}

	if(optional_string("$Density:"))
		stuff_float( &(sip->density) );
	diag_printf ("%s '%s' density -- %7.3f\n", info_type_name, sip->name, sip->density);

	if(optional_string("$Damp:"))
		stuff_float( &(sip->damp) );
	diag_printf ("%s '%s' damp -- %7.3f\n", info_type_name, sip->name, sip->damp);

	if(optional_string("$Rotdamp:"))
		stuff_float( &(sip->rotdamp) );
	diag_printf ("%s '%s' rotdamp -- %7.3f\n", info_type_name, sip->name, sip->rotdamp);

	if(optional_string("$Max Velocity:"))
		stuff_vector(&sip->max_vel);

	// calculate the max speed from max_velocity
	sip->max_speed = sip->max_vel.xyz.z; // = vm_vec_mag(&sip->max_vel);

	if(optional_string("$Rotation Time:"))
	{
		stuff_vector(&sip->rotation_time);

		sip->srotation_time = (sip->rotation_time.xyz.x + sip->rotation_time.xyz.y)/2.0f;

		sip->max_rotvel.xyz.x = (2 * PI) / sip->rotation_time.xyz.x;
		sip->max_rotvel.xyz.y = (2 * PI) / sip->rotation_time.xyz.y;
		sip->max_rotvel.xyz.z = (2 * PI) / sip->rotation_time.xyz.z;
	}

	// get the backwards velocity;
	if(optional_string("$Rear Velocity:"))
		stuff_float(&sip->max_rear_vel);

	// get the accelerations
	if(optional_string("$Forward accel:"))
		stuff_float(&sip->forward_accel );

	if(optional_string("$Forward decel:"))
		stuff_float(&sip->forward_decel );

	if(optional_string("$Slide accel:"))
		stuff_float(&sip->slide_accel );

	if(optional_string("$Slide decel:"))
		stuff_float(&sip->slide_decel );
		
	if(optional_string("$Glide:"))
	{
		stuff_boolean(&sip->can_glide);
	}

	if(sip->can_glide == true)
	{
		if(optional_string("+Max Glide Speed:"))
			stuff_float(&sip->glide_cap );
	}

	if(optional_string("$Warpin type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		j = warptype_match(buf);
		if(j >= 0) {
			sip->warpin_type = j;
		} else {
			Warning(LOCATION, "Invalid warpin type '%s' specified for %s '%s'", buf, info_type_name, sip->name);
			sip->warpin_type = WT_DEFAULT;
		}
	}

	if(optional_string("$Warpin speed:"))
	{
		stuff_float(&sip->warpin_speed);
		if(sip->warpin_speed == 0.0f) {
			Warning(LOCATION, "Warp-in speed specified as 0 on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpin time:"))
	{
		float t_time;
		stuff_float(&t_time);
		sip->warpin_time = fl2i(t_time*1000.0f);
		if(sip->warpin_time <= 0) {
			Warning(LOCATION, "Warp-in time specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpin radius:"))
	{
		stuff_float(&sip->warpin_radius);
		if(sip->warpin_radius <= 0.0f) {
			Warning(LOCATION, "Warp-in radius specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpin animation:"))
	{
		stuff_string(sip->warpin_anim, F_NAME, MAX_FILENAME_LEN);
	}

	if(optional_string("$Warpout type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		j = warptype_match(buf);
		if(j >= 0) {
			sip->warpout_type = j;
		} else {
			Warning(LOCATION, "Invalid warpout type '%s' specified for %s '%s'", buf, info_type_name, sip->name);
			sip->warpout_type = WT_DEFAULT;
		}
	}

	if(optional_string("$Warpout speed:"))
	{
		stuff_float(&sip->warpout_speed);
		if(sip->warpout_speed == 0.0f) {
			Warning(LOCATION, "Warp-out speed specified as 0 on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpout time:"))
	{
		float t_time;
		stuff_float(&t_time);
		sip->warpout_time = fl2i(t_time*1000.0f);
		if(sip->warpout_time <= 0) {
			Warning(LOCATION, "Warp-out time specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpout radius:"))
	{
		stuff_float(&sip->warpout_radius);
		if(sip->warpout_radius <= 0.0f) {
			Warning(LOCATION, "Warp-out radius specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpout animation:"))
	{
		stuff_string(sip->warpout_anim, F_NAME, MAX_FILENAME_LEN);
	}


	if(optional_string("$Player warpout speed:"))
	{
		stuff_float(&sip->warpout_player_speed);
		if(sip->warpout_player_speed == 0.0f) {
			Warning(LOCATION, "Player warp-out speed cannot be 0; value ignored.");
		}
	}

	// get ship explosion info
	shockwave_create_info *sci = &sip->shockwave;
	if(optional_string("$Expl inner rad:")){
		stuff_float(&sci->inner_rad);
	}

	if(optional_string("$Expl outer rad:")){
		stuff_float(&sci->outer_rad);
	}

	if(optional_string("$Expl damage:")){
		stuff_float(&sci->damage);
	}

	if(optional_string("$Expl blast:")){
		stuff_float(&sci->blast);
	}

	if(optional_string("$Expl Propagates:")){
		stuff_boolean(&sip->explosion_propagates);
	}

	if(optional_string("$Shockwave Damage Type:")) {
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		sci->damage_type_idx = damage_type_add(buf);
	}

	if(optional_string("$Shockwave Speed:")){
		stuff_float( &sci->speed );
	}

	if(optional_string("$Shockwave Count:")){
		stuff_int(&sip->shockwave_count);
	}

	if(optional_string("$Shockwave model:")){
		stuff_string( sci->pof_name, F_NAME, MAX_FILENAME_LEN);
	}
	
	if(optional_string("$Shockwave name:")) {
		stuff_string( sci->name, F_NAME, NAME_LENGTH);
	}

char temp_error[64];
strcpy(temp_error, parse_error_text);

	if (optional_string("$Weapon Model Draw Distance:")) {
		stuff_float( &sip->weapon_model_draw_distance );
	}

	// Goober5000 - fixed Bobboau's implementation of restricted banks
	int bank;

	// Set the weapons filter used in weapons loadout (for primary weapons)
	if (optional_string("$Allowed PBanks:"))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank >= MAX_SHIP_PRIMARY_BANKS)
			{
				Warning(LOCATION, "$Allowed PBanks bank-specific loadout for %s '%s' exceeds permissible number of primary banks.  Ignoring the rest...", info_type_name, sip->name);
				bank--;
				break;
			}

strcat(parse_error_text,"'s primary banks");
			num_allowed = stuff_int_list(allowed_weapons, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

						// actually say which weapons are allowed
			for ( i = 0; i < num_allowed; i++ )
			{
				if ( allowed_weapons[i] >= 0 )		// MK, Bug fix, 9/6/99.  Used to be "allowed_weapons" not "allowed_weapons[i]".
				{
					sip->allowed_bank_restricted_weapons[bank][allowed_weapons[i]] |= REGULAR_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[i] |= REGULAR_WEAPON;
			}
		}
	}

	// Set the weapons filter used in weapons loadout (for primary weapons)
	if (optional_string("$Allowed Dogfight PBanks:"))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank >= MAX_SHIP_PRIMARY_BANKS)
			{
				Warning(LOCATION, "$Allowed Dogfight PBanks bank-specific loadout for %s '%s' exceeds permissible number of primary banks.  Ignoring the rest...", info_type_name, sip->name);
				bank--;
				break;
			}

strcat(parse_error_text,"'s primary dogfight banks");
		num_allowed = stuff_int_list(allowed_weapons, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

			// actually say which weapons are allowed
			for ( i = 0; i < num_allowed; i++ )
			{
				if ( allowed_weapons[i] >= 0 )		// MK, Bug fix, 9/6/99.  Used to be "allowed_weapons" not "allowed_weapons[i]".
				{
					sip->allowed_bank_restricted_weapons[bank][allowed_weapons[i]] |= DOGFIGHT_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[i] |= DOGFIGHT_WEAPON;
			}
		}
	}

	// Get default primary bank weapons
	if(optional_string("$Default PBanks:"))
	{
		strcat(parse_error_text,"'s default primary banks");
		sip->num_primary_banks = stuff_int_list(sip->primary_bank_weapons, MAX_SHIP_PRIMARY_BANKS, WEAPON_LIST_TYPE);
		strcpy(parse_error_text, temp_error);

		// error checking
		for ( i = 0; i < sip->num_primary_banks; i++ )
		{
			Assert(sip->primary_bank_weapons[i] >= 0);
		}
	}

	// optional ballistic primary imformation (Goober5000)......
	if(optional_string("$PBank Capacity:"))
	{
		// get the capacity of each primary bank
strcat(parse_error_text,"'s default primary banks' ammo");
		pbank_capacity_count = stuff_int_list(sip->primary_bank_ammo_capacity, MAX_SHIP_PRIMARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
		if (pbank_capacity_count != sip->num_primary_banks)
		{
			Warning(LOCATION, "Primary bank capacities have not been completely specified for %s '%s'... fix this!!", info_type_name, sip->name);
		}
	}

	if(optional_string("$Show Primary Models:"))
	{
		sip->draw_models = true;
		stuff_bool_list(sip->draw_primary_models, sip->num_primary_banks);
	}

	// Set the weapons filter used in weapons loadout (for secondary weapons)
	if (optional_string("$Allowed SBanks:"))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank >= MAX_SHIP_SECONDARY_BANKS)
			{
				Warning(LOCATION, "$Allowed SBanks bank-specific loadout for %s '%s' exceeds permissible number of secondary banks.  Ignoring the rest...", info_type_name, sip->name);
				bank--;
				break;
			}

strcat(parse_error_text,"'s secondary banks");
		num_allowed = stuff_int_list(allowed_weapons, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

			// actually say which weapons are allowed
			for ( i = 0; i < num_allowed; i++ )
			{
				if ( allowed_weapons[i] >= 0 )		// MK, Bug fix, 9/6/99.  Used to be "allowed_weapons" not "allowed_weapons[i]".
				{
					sip->allowed_bank_restricted_weapons[MAX_SHIP_PRIMARY_BANKS+bank][allowed_weapons[i]] |= REGULAR_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[MAX_SHIP_PRIMARY_BANKS+i] |= REGULAR_WEAPON;
			}
		}
	}

	// Set the weapons filter used in weapons loadout (for secondary weapons)
	if (optional_string("$Allowed Dogfight SBanks:"))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank >= MAX_SHIP_SECONDARY_BANKS)
			{
				Warning(LOCATION, "$Allowed Dogfight SBanks bank-specific loadout for %s '%s' exceeds permissible number of secondary banks.  Ignoring the rest...", info_type_name, sip->name);
				bank--;
				break;
			}

strcat(parse_error_text,"'s secondary dogfight banks");
		num_allowed = stuff_int_list(allowed_weapons, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

			// actually say which weapons are allowed
			for ( i = 0; i < num_allowed; i++ )
			{
				if ( allowed_weapons[i] >= 0 )		// MK, Bug fix, 9/6/99.  Used to be "allowed_weapons" not "allowed_weapons[i]".
				{
					sip->allowed_bank_restricted_weapons[MAX_SHIP_PRIMARY_BANKS+bank][allowed_weapons[i]] |= DOGFIGHT_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[MAX_SHIP_PRIMARY_BANKS+i] |= DOGFIGHT_WEAPON;
			}
		}
	}

	// Get default secondary bank weapons

	if(optional_string("$Default SBanks:"))
	{
		strcat(parse_error_text,"'s default secondary banks");
		sip->num_secondary_banks = stuff_int_list(sip->secondary_bank_weapons, MAX_SHIP_SECONDARY_BANKS, WEAPON_LIST_TYPE);
		strcpy(parse_error_text, temp_error);

		// error checking
		for ( i = 0; i < sip->num_secondary_banks; i++ )
		{
			if(sip->secondary_bank_weapons[i] < 0)
			{
				//Is this error message accurate? The previous one was very vague... -Turey
				Warning(LOCATION, "You're trying to specify default secondaries for %s '%s', when it doesn't have any allowed secondaries. That's a bad idea...", info_type_name, sip->name);
			}
			// Assert(sip->secondary_bank_weapons[i] >= 0);
		}

		// Get the capacity of each secondary bank
		required_string("$SBank Capacity:");
		strcat(parse_error_text,"'s secondary banks capacities");
		sbank_capacity_count = stuff_int_list(sip->secondary_bank_ammo_capacity, MAX_SHIP_SECONDARY_BANKS, RAW_INTEGER_TYPE);
		strcpy(parse_error_text, temp_error);
		if ( sbank_capacity_count != sip->num_secondary_banks )
		{
			Warning(LOCATION, "Secondary bank capacities have not been completely specified for %s '%s'... fix this!!", info_type_name, sip->name);
		}
	}
    
	if(optional_string("$Show Secondary Models:"))
	{
		sip->draw_models = true;
		stuff_bool_list(sip->draw_secondary_models, sip->num_secondary_banks);
	}
	
	// copy to regular allowed_weapons array
	for (i=0; i<MAX_SHIP_WEAPONS; i++)
	{
		for (j=0; j<MAX_WEAPON_TYPES; j++)
		{
			if (sip->allowed_bank_restricted_weapons[i][j] & REGULAR_WEAPON)
				sip->allowed_weapons[j] |= REGULAR_WEAPON;

			if (sip->allowed_bank_restricted_weapons[i][j] & DOGFIGHT_WEAPON)
				sip->allowed_weapons[j] |= DOGFIGHT_WEAPON;
		}
	}

	//Set ship ballistic flag if necessary
	for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++)
	{
		for (j=0; j<MAX_WEAPON_TYPES; j++)
		{
			if(sip->allowed_bank_restricted_weapons[i][j] && (Weapon_info[j].wi_flags2 & WIF2_BALLISTIC))
			{
				sip->flags |= SIF_BALLISTIC_PRIMARIES;
				break;
			}
		}
	}

	if(optional_string("$Shields:"))
		stuff_float(&sip->max_shield_strength);

	// optional shield color
	if(optional_string("$Shield Color:")){
		stuff_ubyte(&sip->shield_color[0]);
		stuff_ubyte(&sip->shield_color[1]);
		stuff_ubyte(&sip->shield_color[2]);
	}

	// The next five fields are used for the ETS
	if (optional_string("$Power Output:"))
		stuff_float(&sip->power_output);

	// Goober5000
	if ( optional_string("$Shield Regeneration Rate:") ) {
		stuff_float(&sip->max_shield_regen_per_second);
	}
	else if ( first_time ) {
		sip->max_shield_regen_per_second = 0.02f;
	}

	// Goober5000
	if ( optional_string("$Weapon Regeneration Rate:") ) {
		stuff_float(&sip->max_weapon_regen_per_second);
	}
	else if ( first_time ) {
		sip->max_weapon_regen_per_second = 0.04f;
	}

	if (optional_string("$Max Oclk Speed:") || optional_string("$Max Overclock Speed:"))
		stuff_float(&sip->max_overclocked_speed);
	else if (first_time)
		sip->max_overclocked_speed = sip->max_vel.xyz.z * 1.5f;

	if (optional_string("$Max Weapon Eng:") || optional_string("$Max Weapon Energy:"))
		stuff_float(&sip->max_weapon_reserve);

	if(optional_string("$Hitpoints:"))
	{
		stuff_float(&sip->max_hull_strength);
		if (sip->max_hull_strength < 0.0f)
		{
			Warning(LOCATION, "Max hull strength on %s '%s' cannot be less than 0.  Defaulting to 100.\n", info_type_name, sip->name, sip->max_hull_strength);
			sip->max_hull_strength = 100.0f;
		}
	}

	//Hull rep rate
	
	if(optional_string("$Hull Repair Rate:"))
	{
		stuff_float(&sip->hull_repair_rate);
		sip->hull_repair_rate *= 0.01f;
		
		//Sanity checking
		if(sip->hull_repair_rate > 1.0f)
			sip->hull_repair_rate = 1.0f;
		else if(sip->hull_repair_rate < -1.0f)
			sip->hull_repair_rate = -1.0f;
	}

	//Subsys rep rate
	if(optional_string("$Subsystem Repair Rate:"))
	{
		stuff_float(&sip->subsys_repair_rate);
		sip->subsys_repair_rate *= 0.01f;
		
		//Sanity checking
		if(sip->subsys_repair_rate > 1.0f)
			sip->subsys_repair_rate = 1.0f;
		else if(sip->subsys_repair_rate < -1.0f)
			sip->subsys_repair_rate = -1.0f;
	}
	
	if(optional_string("$Armor Type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		sip->armor_type_idx = armor_type_get_idx(buf);

		if(sip->armor_type_idx == -1)
			Warning(LOCATION,"Invalid armor name %s specified in %s %s", buf, info_type_name, sip->name);
	}

	if (optional_string("$Flags:"))
	{
		char ship_strings[MAX_SHIP_FLAGS][NAME_LENGTH];
		int num_strings = stuff_string_list(ship_strings, MAX_SHIP_FLAGS);
		int ship_type_index = -1;

		for (i = 0; i < num_strings; i++)
		{
			// get ship type from ship flags
			char *ship_type = ship_strings[i];
			bool flag_found = false;

			// Goober5000 - in retail FreeSpace, some ship classes were specified differently
			// in ships.tbl and the ship type array; this patches those differences so that
			// the ship type lookup will work properly
			if (!stricmp(ship_type, "sentrygun"))
				ship_type = "sentry gun";
			else if (!stricmp(ship_type, "escapepod"))
				ship_type = "escape pod";
			else if (!stricmp(ship_type, "repair_rearm"))
				ship_type = "support";
			else if (!stricmp(ship_type, "supercap"))
				ship_type = "super cap";
			else if (!stricmp(ship_type, "knossos"))
				ship_type = "knossos device";

			// look it up in the object types table
			ship_type_index = ship_type_name_lookup(ship_type);

			// set ship class type
			if ((ship_type_index >= 0) && (sip->class_type < 0))
				sip->class_type = ship_type_index;

			// check various ship flags
			for (int idx = 0; idx < Num_ship_flags; idx++) {
				if ( !stricmp(Ship_flags[idx].name, ship_strings[i]) ) {
					flag_found = true;

					if (Ship_flags[idx].var == 255)
						Warning(LOCATION, "Use of '%s' flag for %s '%s' - this flag is no longer needed.", Ship_flags[idx].name, info_type_name, sip->name);
					else if (Ship_flags[idx].var == 0)
						sip->flags |= Ship_flags[idx].def;
					else if (Ship_flags[idx].var == 1)
						sip->flags2 |= Ship_flags[idx].def;
				}
			}

			if ( !flag_found && (ship_type_index < 0) )
				Warning(LOCATION, "Bogus string in flags for %s '%s': %s\n", info_type_name, sip->name, ship_strings[i]);
		}

		// set original status of tech database flags - Goober5000
		if (sip->flags & SIF_IN_TECH_DATABASE)
			sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE;
		if (sip->flags & SIF_IN_TECH_DATABASE_M)
			sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE_M;
	}

	// Goober5000 - ensure number of banks checks out
	if (sip->num_primary_banks > MAX_SHIP_PRIMARY_BANKS)
	{
		Error(LOCATION, "%s '%s' has too many primary banks (%d).  Maximum for ships is currently %d.\n", info_type_name, sip->name, sip->num_primary_banks, MAX_SHIP_PRIMARY_BANKS);
	}

	find_and_stuff_optional("$AI Class:", &sip->ai_class, F_NAME, Ai_class_names, Num_ai_classes, "AI class names");

	// Get Afterburner information
	// Be aware that if $Afterburner is not 1, the other Afterburner fields are not read in
	int has_afterburner = 0;

	if(optional_string("$Afterburner:"))
		stuff_boolean(&has_afterburner);

	if ( has_afterburner == 1 )
	{
		sip->flags |= SIF_AFTERBURNER;

		if(optional_string("+Aburn Max Vel:")) {
			stuff_vector(&sip->afterburner_max_vel);
		}

		if(optional_string("+Aburn For accel:")) {
			stuff_float(&sip->afterburner_forward_accel);
		}

		if(optional_string("+Aburn Fuel:")) {
			stuff_float(&sip->afterburner_fuel_capacity);
		}

		if(optional_string("+Aburn Burn Rate:")) {
			stuff_float(&sip->afterburner_burn_rate);
		}

		if(optional_string("+Aburn Rec Rate:")) {
			stuff_float(&sip->afterburner_recover_rate);
		}

		// Goober5000: check div-0
		Assert(sip->afterburner_fuel_capacity);
	}
	
	if ( optional_string("$Trails:") ) {
		bool trails_warning = true;

		if (optional_string("+Bitmap:") ) {
			trails_warning = false;
			generic_bitmap_init(&sip->afterburner_trail, NULL);
			stuff_string(sip->afterburner_trail.filename, F_NAME, MAX_FILENAME_LEN);
		}
		
		if ( optional_string("+Width:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_width_factor);
		}
			
		if ( optional_string("+Alpha:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_alpha_factor);
		}
			
		if ( optional_string("+Life:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_life);
		}
		
		if (trails_warning)
			Warning(LOCATION, "%s '%s' has $Trails field specified, but no properties given.", info_type_name, sip->name);
	}

	if(optional_string("$Countermeasure type:")) {
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		int res = weapon_info_lookup(buf);
		if(res == -1) {
			Warning(LOCATION, "Could not find weapon type '%s' to use as countermeasure on %s '%s'", info_type_name, sip->name);
		} else if(Weapon_info[res].wi_flags & WIF_BEAM) {
			Warning(LOCATION, "Attempt made to set a beam weapon as a countermeasure on %s '%s'", info_type_name, sip->name);
		} else {
			sip->cmeasure_type = res;
		}
	}

	if(optional_string("$Countermeasures:"))
		stuff_int(&sip->cmeasure_max);

	if(optional_string("$Scan time:"))
		stuff_int(&sip->scan_time);

	//Parse the engine sound
	parse_sound("$EngineSnd:", &sip->engine_snd, sip->name);

	if( optional_string("$Closeup_pos:") ) {
		stuff_vector(&sip->closeup_pos);
	}
	else if ( first_time && strlen(sip->pof_file) ) {
		//Calculate from the model file. This is inefficient, but whatever
		int model_idx = model_load(sip->pof_file, 0, NULL);
		polymodel *pm = model_get(model_idx);

		//Go through, find best
		sip->closeup_pos.xyz.z = fabsf(pm->maxs.xyz.z);

		float temp = fabsf(pm->mins.xyz.z);
		if( temp > sip->closeup_pos.xyz.z ) {
			sip->closeup_pos.xyz.z = temp;
		}

		//Now multiply by 2
		sip->closeup_pos.xyz.z *= -2.0f;

		//We're done with the model.
		model_unload(model_idx);
	}

	if(optional_string("$Closeup_zoom:"))
		stuff_float(&sip->closeup_zoom);
		
	if(optional_string("$Topdown offset:")) {
		sip->topdown_offset_def = true;
		stuff_vector(&sip->topdown_offset);
	}

	if (optional_string("$Shield_icon:")) {
		stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
		hud_shield_assign_info(sip, name_tmp);
	}

	// read in filename for icon that is used in ship selection
	if ( optional_string("$Ship_icon:") ) {
		stuff_string(sip->icon_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in filename for animation that is used in ship selection
	if ( optional_string("$Ship_anim:") ) {
		stuff_string(sip->anim_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in filename for animation that is used in ship selection
	if ( optional_string("$Ship_overhead:") ) {
		stuff_string(sip->overhead_filename, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("$Score:") ){
		stuff_int( &sip->score );
	}

	if ( first_time ) {
		species_info *species = &Species_info[sip->species];

		sip->thruster_glow_info = species->thruster_info.glow;
		sip->thruster_secondary_glow_info = species->thruster_secondary_glow_info;
		sip->thruster_tertiary_glow_info = species->thruster_tertiary_glow_info;
	}

	if ( optional_string("$Thruster Bitmap 1:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );
	
		if ( VALID_FNAME(name_tmp) )
			generic_anim_init( &sip->thruster_glow_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 1a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_anim_init( &sip->thruster_glow_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster01 Radius factor:") ) {
		stuff_float(&sip->thruster01_glow_rad_factor);
	}

	if ( optional_string("$Thruster Bitmap 2:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_secondary_glow_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 2a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_secondary_glow_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster02 Radius factor:") ) {
		stuff_float(&sip->thruster02_glow_rad_factor);
	}

	if ( optional_string("$Thruster01 Length factor:") ) {
		stuff_float(&sip->thruster02_glow_len_factor);
		Warning(LOCATION, "Depreciated spelling: \"$Thruster01 Length factor:\".  Use \"$Thruster02 Length factor:\" instead.");
	}

	if ( optional_string("$Thruster02 Length factor:") ) {
		stuff_float(&sip->thruster02_glow_len_factor);
	}

	if ( optional_string("$Thruster Bitmap 3:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_tertiary_glow_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 3a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_tertiary_glow_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster03 Radius factor:") ) {
		stuff_float(&sip->thruster03_glow_rad_factor);
	}

	while ( optional_string("$Thruster Particles:") ) {
		bool afterburner = false;
		thruster_particles tpart;

		if ( optional_string("$Thruster Particle Bitmap:") )
			afterburner = false;
		else if ( optional_string("$Afterburner Particle Bitmap:") )
			afterburner = true;
		else
			Error( LOCATION, "formatting error in the thruster's particle section for %s '%s'\n", info_type_name, sip->name );

		generic_anim_init(&tpart.thruster_bitmap, NULL);
		stuff_string(tpart.thruster_bitmap.filename, F_NAME, MAX_FILENAME_LEN);

		required_string("$Min Radius:");
		stuff_float(&tpart.min_rad);
		
		required_string("$Max Radius:");
		stuff_float(&tpart.max_rad);
		
		required_string("$Min created:");
		stuff_int(&tpart.n_low);
		
		required_string("$Max created:");
		stuff_int(&tpart.n_high);
		
		required_string("$Variance:");
		stuff_float(&tpart.variance);

		if (afterburner)
			sip->normal_thruster_particles.push_back( tpart );
		else
			sip->afterburner_thruster_particles.push_back( tpart );
	}

	// if the ship is a stealth ship
	if ( optional_string("$Stealth:") ) {
		sip->flags |= SIF_STEALTH;
	}
	
	else if ( optional_string("$Stealth") ) {
		Warning(LOCATION, "%s '%s' is missing the colon after \"$Stealth\". Note that you may also use the ship flag \"stealth\".", info_type_name, sip->name);
		sip->flags |= SIF_STEALTH;
	}

	// parse contrail info
	if ( optional_string("$max decals:") ){
		stuff_int(&sip->max_decals);
	}
	else if ( first_time ) {
		if( sip->flags & SIF_SMALL_SHIP ) {
			sip->max_decals = 50;
		}
		else if( sip->flags & SIF_BIG_SHIP ) {
			sip->max_decals = 100;
		}
		else if( SIF_HUGE_SHIP ) {
			sip->max_decals = 300;
		}
		else {
			sip->max_decals = 10;
		}
	}

	while ( optional_string("$Trail:") ) {
		// this means you've reached the max # of contrails for a ship
		if (sip->ct_count >= MAX_SHIP_CONTRAILS) {
			Warning(LOCATION, "%s '%s' has more contrails than the max of %d", info_type_name, sip->name, MAX_SHIP_CONTRAILS);
			break;
		}

		trail_info *ci = &sip->ct_info[sip->ct_count++];
		
		required_string("+Offset:");
		stuff_vector(&ci->pt);
		
		required_string("+Start Width:");
		stuff_float(&ci->w_start);
		
		required_string("+End Width:");
		stuff_float(&ci->w_end);
		
		required_string("+Start Alpha:");
		stuff_float(&ci->a_start);
		
		required_string("+End Alpha:");
		stuff_float(&ci->a_end);

		required_string("+Max Life:");
		stuff_float(&ci->max_life);
		
		required_string("+Spew Time:");
		stuff_int(&ci->stamp);		

		required_string("+Bitmap:");
		stuff_string(name_tmp, F_NAME, NAME_LENGTH);
		generic_bitmap_init(&ci->texture, name_tmp);
		generic_bitmap_load(&ci->texture);
	}

	man_thruster *mtp = NULL;
	man_thruster manwich;
	while(optional_string("$Thruster:"))
	{
		int idx = -1;
		if(optional_string("+index:")) {
			stuff_int(&idx);
		}

		if(idx >= 0 && idx < sip->num_maneuvering) {
			mtp = &sip->maneuvering[idx];
		} else if(idx < 0) {
			if(sip->num_maneuvering < MAX_MAN_THRUSTERS) {
				mtp = &sip->maneuvering[sip->num_maneuvering++];
			} else {
				Warning(LOCATION, "Too many maneuvering thrusters on %s '%s'; maximum is %d", info_type_name, sip->name, MAX_MAN_THRUSTERS);
			}
		} else {
			mtp = &manwich;
			Warning(LOCATION, "Invalid index (%d) specified for maneuvering thruster on %s '%s'", idx, info_type_name, sip->name);
		}

		if(optional_string("+Used for:")) {
			parse_string_flag_list(&mtp->use_flags, Man_types, Num_man_types);
		}

		if(optional_string("+Flags:")) {
			parse_string_flag_list(&mtp->flags, Man_thruster_flags, Num_man_thruster_flags);
		}

		if(optional_string("+Position:")) {
			stuff_float_list(mtp->pos.a1d, 3);
		}

		if(optional_string("+Normal:")) {
			stuff_float_list(mtp->norm.a1d, 3);
		}

		if(optional_string("+Texture:"))
		{
			stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
			int tex_fps=0, tex_nframes=0, tex_id=-1;;
			tex_id = bm_load_animation(name_tmp, &tex_nframes, &tex_fps, 1);
			if(tex_id < 0)
				tex_id = bm_load(name_tmp);
			if(tex_id >= 0)
			{
				if(mtp->tex_id >= 0) {
					bm_unload(mtp->tex_id);
				}

				mtp->tex_id = tex_id;
				mtp->tex_fps = tex_fps;
				mtp->tex_nframes = tex_nframes;
			}
		}

		if(optional_string("+Radius:")) {
			stuff_float(&mtp->radius);
		}

		if(optional_string("+Length:")) {
			stuff_float(&mtp->length);
		}

		parse_sound("+StartSnd:", &mtp->start_snd, sip->name);
		parse_sound("+LoopSnd:", &mtp->loop_snd, sip->name);
		parse_sound("+StopSnd:", &mtp->stop_snd, sip->name);
	}
	
	int n_subsystems = 0;
	int cont_flag = 1;
	model_subsystem subsystems[MAX_MODEL_SUBSYSTEMS];		// see model.h for max_model_subsystems
	for (i=0; i<MAX_MODEL_SUBSYSTEMS; i++) {
		subsystems[i].stepped_rotation = NULL;
//		subsystems[idx].ai_rotation = NULL;
	}
	
	float	hull_percentage_of_hits = 100.0f;
	//If the ship already has subsystem entries (ie this is a modular table)
	//make sure hull_percentage_of_hits is set properly
	for(i=0; i < sip->n_subsystems; i++) {
		hull_percentage_of_hits -= sip->subsystems[i].max_subsys_strength / sip->max_hull_strength;
	}

	while (cont_flag) {
		int r = required_string_4("#End", "$Subsystem:", "$Name", "$Template" );
		switch (r) {
		case 0:
			cont_flag = 0;
			break;
		case 1:
		{
			float	turning_rate;
			float	percentage_of_hits;
			model_subsystem *sp = NULL;			// to append on the ships list of subsystems
			
			int sfo_return;
			required_string("$Subsystem:");
			stuff_string(name_tmp, F_NAME, sizeof(name_tmp), ",");
			Mp++;
			for(i = 0;i < sip->n_subsystems; i++)
			{
				if(!stricmp(sip->subsystems[i].subobj_name, name_tmp))
					sp = &sip->subsystems[i];
			}

			if(sp == NULL)
			{
				if( sip->n_subsystems + n_subsystems >= MAX_MODEL_SUBSYSTEMS )
				{
					Warning(LOCATION, "Number of subsystems for %s '%s' (%d) exceeds max of %d; only the first %d will be used", info_type_name, sip->name, sip->n_subsystems, n_subsystems, MAX_MODEL_SUBSYSTEMS);
					break;
				}
				sp = &subsystems[n_subsystems++];			// subsystems a local -- when done, we will malloc and copy
				strcpy(sp->subobj_name, name_tmp);
				
				//Init blank values
				sp->max_subsys_strength = 0.0f;
				sp->turret_turning_rate = 0.0f;
				sp->weapon_rotation_pbank = -1;

				for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++) {
					sp->primary_banks[i] = -1;
					sp->primary_bank_capacity[i] = 0;
				}

				for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++) {
					sp->secondary_banks[i] = -1;
					sp->secondary_bank_capacity[i] = 0;
				}

				sp->engine_wash_pointer = NULL;
				
				sp->alive_snd = -1;
				sp->dead_snd = -1;
				sp->rotation_snd = -1;
				sp->turret_rotation_snd = -1;
				
				sp->flags = 0;
				
				sp->n_triggers = 0;
				sp->triggers = NULL;
				
				sp->model_num = -1;		// init value for later sanity checking!!
				sp->armor_type_idx = -1;
				sp->path_num = -1;
			}
			sfo_return = stuff_float_optional(&percentage_of_hits);
			if(sfo_return==2)
			{
				hull_percentage_of_hits -= percentage_of_hits;
				sp->max_subsys_strength = sip->max_hull_strength * (percentage_of_hits / 100.0f);
				sp->type = SUBSYSTEM_UNKNOWN;
			}
			if(sfo_return > 0)
			{
				sfo_return = stuff_float_optional(&turning_rate);
				if(sfo_return==2)
				{
					// specified as how long to turn 360 degrees in ships.tbl
					if ( turning_rate > 0.0f ){
						sp->turret_turning_rate = PI2 / turning_rate;		
					} else {
						sp->turret_turning_rate = 0.0f;		
					}
				}
				else if(sfo_return==1)
				{
					Warning(LOCATION, "Unneccessary comma at the end of the name specifier for subsystem %s of %s %s", sp->subobj_name, info_type_name, sip->name);
				}
			}

			if(optional_string("$Armor Type:")) {
				stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
				sp->armor_type_idx = armor_type_get_idx(buf);
			}

			//	Get default primary bank weapons
			if (optional_string("$Default PBanks:")){
strcat(parse_error_text,"'s default primary banks");
				stuff_int_list(sp->primary_banks, MAX_SHIP_PRIMARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// get capacity of each primary bank - Goober5000
			if (optional_string("$PBank Capacity:")){
strcat(parse_error_text,"'s primary banks capacities");
				stuff_int_list(sp->primary_bank_capacity, MAX_SHIP_PRIMARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
			}

			//	Get default secondary bank weapons
			if (optional_string("$Default SBanks:")){
strcat(parse_error_text,"'s default secondary banks");
				stuff_int_list(sp->secondary_banks, MAX_SHIP_SECONDARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// Get the capacity of each secondary bank
			if (optional_string("$SBank Capacity:")){
strcat(parse_error_text,"'s secondary banks capacities");
				stuff_int_list(sp->secondary_bank_capacity, MAX_SHIP_SECONDARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// Get optional engine wake info
			if (optional_string("$Engine Wash:")) {
				stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
				// get and set index
				sp->engine_wash_pointer = get_engine_wash_pointer(name_tmp);
			}

			parse_sound("$AliveSnd:", &sp->alive_snd, sp->subobj_name);
			parse_sound("$DeadSnd:", &sp->dead_snd, sp->subobj_name);
			parse_sound("$RotationSnd:", &sp->rotation_snd, sp->subobj_name);
				
			// Get any AWACS info
			sp->awacs_intensity = 0.0f;
			if (optional_string("$AWACS:")) {
				sfo_return = stuff_float_optional(&sp->awacs_intensity);
				if(sfo_return > 0)
					stuff_float_optional(&sp->awacs_radius);
				sip->flags |= SIF_HAS_AWACS;
			}

			if (optional_string("$Flags:")) {
				parse_string_flag_list((int*)&sp->flags, Subsystem_flags, Num_subsystem_flags);
			}

			if (optional_string("+non-targetable"))
			{
				Warning(LOCATION, "Grammar error in table file.  Please change \"+non-targetable\" to \"+untargetable\" in %s '%s', subsystem '%s'.", info_type_name, sip->name, sp->name);
				sp->flags |= MSS_FLAG_UNTARGETABLE;
			}

			bool old_flags = false;
			if (optional_string("+untargetable")) {
				sp->flags |= MSS_FLAG_UNTARGETABLE;
				old_flags = true;
			}

			if (optional_string("+carry-no-damage")) {
				sp->flags |= MSS_FLAG_CARRY_NO_DAMAGE;
				old_flags = true;
			}

			if (optional_string("+use-multiple-guns")) {
				sp->flags |= MSS_FLAG_USE_MULTIPLE_GUNS;
				old_flags = true;
			}

			if (optional_string("+fire-down-normals")) {
				sp->flags |= MSS_FLAG_FIRE_ON_NORMAL;
				old_flags = true;
			}

			if (old_flags) {
				Warning(LOCATION, "Use of deprecated subsystem syntax.  Please use the $Flags: field for subsystem flags.\n\n" \
				"At least one of the following tags was used on %s '%s', subsystem %s:\n" \
				"\t+untargetable\n" \
				"\t+carry-no-damage\n" \
				"\t+use-multiple-guns\n" \
				"\t+fire-down-normals\n", info_type_name, sip->name, sp->name);
			}

			while(optional_string("$animation:"))
			{
				stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
				if(!stricmp(name_tmp, "triggered"))
				{
					queued_animation *current_trigger;

					sp->triggers = (queued_animation*)vm_realloc(sp->triggers, sizeof(queued_animation) * (sp->n_triggers + 1));
					current_trigger = &sp->triggers[sp->n_triggers];
					sp->n_triggers++;
					//add a new trigger

					required_string("$type:");
					char atype[NAME_LENGTH];
					stuff_string(atype, F_NAME, NAME_LENGTH);
					current_trigger->type = model_anim_match_type(atype);

					if(optional_string("+sub_type:")){
						stuff_int(&current_trigger->subtype);
					}else{
						current_trigger->subtype = ANIMATION_SUBTYPE_ALL;
					}


					if(current_trigger->type == TRIGGER_TYPE_INITIAL){
						//the only thing initial animation type needs is the angle, 
						//so to save space lets just make everything optional in this case

						if(optional_string("+delay:"))
							stuff_int(&current_trigger->start); 
						else
							current_trigger->start = 0;
		
						if(optional_string("+absolute_angle:")){
							current_trigger->absolute = true;
							stuff_vector(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radian(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.z);
						}else{
							current_trigger->absolute = false;
							if(!optional_string("+relative_angle:"))
								required_string("+relative_angle:");

							stuff_vector(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radian(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radian(current_trigger->angle.xyz.z);
						}
		
						if(optional_string("+velocity:")){
							stuff_vector(&current_trigger->vel );
							current_trigger->vel.xyz.x = fl_radian(current_trigger->vel.xyz.x);
							current_trigger->vel.xyz.y = fl_radian(current_trigger->vel.xyz.y);
							current_trigger->vel.xyz.z = fl_radian(current_trigger->vel.xyz.z);
						}
		
						if(optional_string("+acceleration:")){
							stuff_vector(&current_trigger->accel );
							current_trigger->accel.xyz.x = fl_radian(current_trigger->accel.xyz.x);
							current_trigger->accel.xyz.y = fl_radian(current_trigger->accel.xyz.y);
							current_trigger->accel.xyz.z = fl_radian(current_trigger->accel.xyz.z);
						}

						if(optional_string("+time:"))
							stuff_int(&current_trigger->end );
						else
							current_trigger->end = 0;
					}else{

						required_string("+delay:");
						stuff_int(&current_trigger->start); 

						current_trigger->reverse_start = -1;	//have some code figure this out for us

						if(optional_string("+reverse_delay:"))stuff_int(&current_trigger->reverse_start);
		
						if(optional_string("+absolute_angle:")){
							current_trigger->absolute = true;
							stuff_vector(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radian(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.z);
						}else{
							current_trigger->absolute = false;
							required_string("+relative_angle:");
							stuff_vector(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radian(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radian(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radian(current_trigger->angle.xyz.z);
						}
		
						required_string("+velocity:");
						stuff_vector(&current_trigger->vel );
						current_trigger->vel.xyz.x = fl_radian(current_trigger->vel.xyz.x);
						current_trigger->vel.xyz.y = fl_radian(current_trigger->vel.xyz.y);
						current_trigger->vel.xyz.z = fl_radian(current_trigger->vel.xyz.z);
		
						required_string("+acceleration:");
						stuff_vector(&current_trigger->accel );
						current_trigger->accel.xyz.x = fl_radian(current_trigger->accel.xyz.x);
						current_trigger->accel.xyz.y = fl_radian(current_trigger->accel.xyz.y);
						current_trigger->accel.xyz.z = fl_radian(current_trigger->accel.xyz.z);

						if(optional_string("+time:"))
							stuff_int(&current_trigger->end );
						else
							current_trigger->end = 0;

						if(optional_string("$Sound:")){
							required_string("+Start:");
							stuff_int(&current_trigger->start_sound );
							required_string("+Loop:");
							stuff_int(&current_trigger->loop_sound );
							required_string("+End:");
							stuff_int(&current_trigger->end_sound );
							required_string("+Radius:");
							stuff_float(&current_trigger->snd_rad );
						}else{
							current_trigger->start_sound = -1;
							current_trigger->loop_sound = -1;
							current_trigger->end_sound = -1;
							current_trigger->snd_rad = 0;
						}
					}

					//make sure that the amount of time it takes to accelerate up and down doesn't make it go farther than the angle
					current_trigger->correct();
				}
				else if(!stricmp(name_tmp, "linked"))
				{
				}
			}
		}
		break;
		case 2:
			cont_flag = 0;
			break;
		case 3:
			if (isTemplate) {
				cont_flag = 0;
				break;
			}
		default:
			Int3();	// Impossible return value from required_string_3.
		}
	}	

	// must be > 0//no it doesn't :P -Bobboau
	// yes it does! - Goober5000
	// (we don't want a div-0 error)
	if (hull_percentage_of_hits <= 0.0f )
	{
		//Warning(LOCATION, "The subsystems defined for %s '%s' can take more (or the same) combined damage than the ship itself. Adjust the tables so that the percentages add up to less than 100", info_type_name, sip->name);
	}
	// when done reading subsystems, malloc and copy the subsystem data to the ship info structure
	int orig_n_subsystems = sip->n_subsystems;
	if ( n_subsystems > 0 ) {
		if(sip->n_subsystems < 1) {
			sip->n_subsystems = n_subsystems;
			sip->subsystems = (model_subsystem *)vm_malloc(sizeof(model_subsystem) * sip->n_subsystems );
		} else {
			sip->n_subsystems += n_subsystems;
			sip->subsystems = (model_subsystem *)vm_realloc(sip->subsystems, sizeof(model_subsystem) * sip->n_subsystems);
		} 
		Assert( sip->subsystems != NULL );
		
		for ( i = 0; i < n_subsystems; i++ ){
			sip->subsystems[orig_n_subsystems+i] = subsystems[i];
		}
	}

	model_anim_fix_reverse_times(sip);

	strcpy(parse_error_text, "");


	return rtn;	//0 for success
}

/*
char get_engine_wash_index(char *engine_wash_name)
{
	int i;

	for (i=0; i<Num_engine_wash_types; i++) {
		if ( !stricmp(engine_wash_name, Engine_wash_info[i].name) ) {
			return (char)i;
		}
	}

	// not found, so return -1
	return -1;
}*/

engine_wash_info *get_engine_wash_pointer(char *engine_wash_name)
{
	for(int i = 0; i < Num_engine_wash_types; i++)
	{
		if(!stricmp(engine_wash_name, Engine_wash_info[i].name))
		{
			return &Engine_wash_info[i];
		}
	}

	//Didn't find anything.
	return NULL;
}

void parse_ship_type()
{
	char name_buf[NAME_LENGTH];
	bool nocreate = false;
	ship_type_info stp_tmp, *stp = NULL;

	required_string("$Name:");
	stuff_string(name_buf, F_NAME, NAME_LENGTH);

	if(optional_string("+nocreate")) {
		nocreate = true;
	}

	int idx = ship_type_name_lookup(name_buf);
	if(idx >= 0)
	{
		stp = &Ship_types[idx];
	}
	else
	{
		if(!nocreate)
		{
			Ship_types.resize(Ship_types.size()+1);
			stp = &Ship_types[Ship_types.size()-1];
		} else {
			stp = &stp_tmp;
		}

		Assert( stp != NULL );
		memset( stp, 0, sizeof(ship_type_info) );
		strcpy(stp->name, name_buf);
	}

	//Okay, now we should have the values to parse
	//But they aren't here!! :O
	//Now they are!! Whee fogging!!

	if(optional_string("$Counts for Alone:")) {
		stuff_boolean_flag(&stp->message_bools, STI_MSG_COUNTS_FOR_ALONE);
	}

	if(optional_string("$Praise Destruction:")) {
		stuff_boolean_flag(&stp->message_bools, STI_MSG_PRAISE_DESTRUCTION);
	}

	if(optional_string("$On Hotkey list:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_HOTKEY_ON_LIST);
	}

	if(optional_string("$Target as Threat:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_TARGET_AS_THREAT);
	}

	if(optional_string("$Show Attack Direction:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_SHOW_ATTACK_DIRECTION);
	}

	if(optional_string("$Scannable:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_SCANNABLE);
	}

	if(optional_string("$Warp Pushes:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_WARP_PUSHES);
	}

	if(optional_string("$Warp Pushable:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_WARP_PUSHABLE);
	}

	if(optional_string("$FF Multiplier:")) {
		stuff_float(&stp->ff_multiplier);
	}

	if(optional_string("$EMP Multiplier:")) {
		stuff_float(&stp->emp_multiplier);
	}

	if(optional_string("$Beams Easily Hit:")) {
			stuff_boolean_flag(&stp->weapon_bools, STI_WEAP_BEAMS_EASILY_HIT);
		}

	if(optional_string("$Fog:"))
	{
		if(optional_string("+Start dist:")) {
			stuff_float(&stp->fog_start_dist);
		}

		if(optional_string("+Compl dist:")) {
			stuff_float(&stp->fog_complete_dist);
		}

		if(optional_string("+Disappear factor:")) {
			stuff_float(&stp->fog_disappear_factor);
		}
	}

	if(optional_string("$AI:"))
	{
		if(optional_string("+Valid goals:")) {
			parse_string_flag_list(&stp->ai_valid_goals, Ai_goal_names, Num_ai_goals);
		}

		if(optional_string("+Accept Player Orders:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_ACCEPT_PLAYER_ORDERS);
		}

		if(optional_string("+Player Orders:")) {
			parse_string_flag_list(&stp->ai_player_orders, Player_orders, Num_player_orders);
		}

		if(optional_string("+Auto attacks:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_AUTO_ATTACKS);
		}

		if(optional_string("+Attempt broadside:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_ATTEMPT_BROADSIDE);
		}

		if(optional_string("+Actively Pursues:")) {
			stuff_string_list(&stp->ai_actively_pursues_temp);
		}

		if(optional_string("+Guards attack this:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_GUARDS_ATTACK);
		}

		if(optional_string("+Turrets attack this:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_TURRETS_ATTACK);
		}

		if(optional_string("+Can form wing:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_CAN_FORM_WING);
		}

		if(optional_string("+Active docks:")) {
			parse_string_flag_list(&stp->ai_active_dock, Dock_type_names, Num_dock_type_names);
		}

		if(optional_string("+Passive docks:")) {
			parse_string_flag_list(&stp->ai_passive_dock, Dock_type_names, Num_dock_type_names);
		}
	}
}
/*
void init_shiptype_defs()
{
	Ship_types.resize(19);

	int idx = 0;
	ship_type_info *sti = NULL;

	sti = &Ship_types[idx++];
	strcpy(sti->name, "Navbuoy");
	sti->debris_max_speed = 200.0f;
	sti->ff_multiplier = 1.0f;
	sti->emp_multiplier = 10.0f;
	sti->fog_start_dist = 10.0f;
	sti->fog_complete_dist = 500.0f;
	sti->ai_bools |= STI_AI_TURRETS_ATTACK;

	sti = &Ship_types[idx++];
	strcpy(sti->name, "Sentry gun");
	sti->message_bools |= STI_MSG_COUNTS_FOR_ALONE;
	sti->hud_bools |= STI_HUD_HOTKEY_ON_LIST | STI_HUD_TARGET_AS_THREAT | STI_HUD_SHOW_ATTACK_DIRECTION;
	sti->debris_max_speed = 200.0f;
	sti->ff_multiplier = 0.10f;
	sti->emp_multiplier = 10.0f;
	sti->fog_start_dist = 10.0f;
	sti->fog_complete_dist = 500.0f;
	sti->ai_bools |= STI_AI_AUTO_ATTACKS | STI_AI_GUARDS_ATTACK | STI_AI_TURRETS_ATTACK;

}*/

void parse_shiptype_tbl(char *longname)
{
	lcl_ext_open();

	if (longname != NULL)
		read_file_text(longname);
	else
		read_file_text_from_array(defaults_get_file("objecttypes.tbl"));

	reset_parse();

	if (optional_string("#Ship Types"))
	{
		while (required_string_either("#End", "$Name:"))
		{
			parse_ship_type();
		}

		required_string("#End");
	}

	lcl_ext_close();
}

// Goober5000 - this works better in its own function
void ship_set_default_player_ship()
{
	int i;

	// already have one
	if(strlen(default_player_ship))
		return;

	// find the first with the default flag
	for(i = 0; i < Num_ship_classes; i++)
	{
		if(Ship_info[i].flags & SIF_DEFAULT_PLAYER_SHIP)
		{
			strcpy(default_player_ship, Ship_info[i].name);
			return;
		}
	}

	// find the first player ship
	for(i = 0; i < Num_ship_classes; i++)
	{
		if(Ship_info[i].flags & SIF_PLAYER_SHIP)
		{
			strcpy(default_player_ship, Ship_info[i].name);
			return;
		}
	}

	// find the first ship
	if(Num_ship_classes > 0)
	{
		strcpy(default_player_ship, Ship_info[0].name);
	}
}

void parse_shiptbl(char* longname)
{
	int i, j;

	strcpy(current_ship_table, longname);
	// open localization
	lcl_ext_open();
	
	read_file_text(longname);
	reset_parse();

	// parse default ship
	//Override default player ship
	if(optional_string("#Default Player Ship"))
	{
		required_string("$Name:");
		stuff_string(default_player_ship, F_NAME, sizeof(default_player_ship));
		required_string("#End");
	}
	//Add engine washes
	//This will override if they already exist
	if(optional_string("#Engine Wash Info"))
	{
		while (required_string_either("#End", "$Name:"))
		{
			parse_engine_wash(Parsing_modular_table);
		}

		required_string("#End");
	}

	if( optional_string("#Ship Templates") ) {
		while ( required_string_either("#End","$Template:") ) {
			
			if ( parse_ship_template() ) {
				continue;
			}
		}

		required_string("#End");
	}

	//Add ship classes
	if(optional_string("#Ship Classes"))
	{

		while (required_string_either("#End","$Name:"))
		{
			if ( parse_ship(Parsing_modular_table) ) {
				continue;
			}
		}

		required_string("#End");
	}

	//Set default player ship
	ship_set_default_player_ship();

	// Goober5000 - validate ballistic primaries
	for(i = 0; i < Num_ship_classes; i++)
	{
		int pbank_capacity_specified = 0;
		ship_info *sip = &Ship_info[i];

		// determine whether this ship had primary capacities specified for it
		for (j = 0; j < sip->num_primary_banks; j++)
		{
			if (sip->primary_bank_ammo_capacity[j] > 0)
			{
				pbank_capacity_specified = 1;
				break;
			}
		}

		// be friendly; ensure ballistic flags check out
		if (pbank_capacity_specified)
		{
			if (!(sip->flags & SIF_BALLISTIC_PRIMARIES))
			{
				Warning(LOCATION, "Pbank capacity specified for non-ballistic-primary-enabled ship %s.\nResetting capacities to 0.\n", sip->name);
	
				for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++)
				{
					sip->primary_bank_ammo_capacity[j] = 0;
				}
			}
		}
		else
		{
			if (sip->flags & SIF_BALLISTIC_PRIMARIES)
			{
				Warning(LOCATION, "Pbank capacity not specified for ballistic-primary-enabled ship %s.\nDefaulting to capacity of 1 per bank.\n", sip->name);
				for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++)
				{
					sip->primary_bank_ammo_capacity[j] = 1;
				}
			}
		}
	}


	// Read in a list of ship_info indicies that are an ordering of the player ship precedence.
	// This list is used to select an alternate ship when a particular ship is not available
	// during ship selection.
	// Guess it isn't -WMC
	/*
	strcpy(parse_error_text,"'player ship precedence");
	if(!Parsing_modular_table)
	{
		required_string("$Player Ship Precedence:");
		Num_player_ship_precedence = stuff_int_list(Player_ship_precedence, MAX_PLAYER_SHIP_CHOICES, SHIP_INFO_TYPE);
	}
	else
	{
		if(optional_string("$Player Ship Precedence:"))
		{
			Num_player_ship_precedence = stuff_int_list(Player_ship_precedence, MAX_PLAYER_SHIP_CHOICES, SHIP_INFO_TYPE);
		}
	}
	strcpy(parse_error_text, "");
	*/

	// close localization
	lcl_ext_close();
}

int ship_show_velocity_dot = 0;


DCF_BOOL( show_velocity_dot, ship_show_velocity_dot )

// Called once at the beginning of the game to parse ships.tbl and stuff the Ship_info[]
// structure
void ship_init()
{
	int rval;

	if ( !ships_inited )
	{
		int num_files;

		//Parse main TBL first
		if (cf_exists_full("objecttypes.tbl", CF_TYPE_TABLES))
			parse_shiptype_tbl("objecttypes.tbl");
		else
			parse_shiptype_tbl(NULL);

		//Then other ones
		num_files = parse_modular_table(NOX("*-obt.tbm"), parse_shiptype_tbl);

		if ( num_files > 0 ) {
			Module_ship_weapons_loaded = true;
		}

		// DO ALL THE STUFF WE NEED TO DO AFTER LOADING Ship_types
		ship_type_info *stp;

		uint i,j;
		int idx;
		for(i = 0; i < Ship_types.size(); i++)
		{
			stp = &Ship_types[i];

			//Handle active pursuit
			for(j = 0; j < stp->ai_actively_pursues_temp.size(); j++)
			{
				idx = ship_type_name_lookup((char*)stp->ai_actively_pursues_temp[j].c_str());
				if(idx >= 0) {
					stp->ai_actively_pursues.push_back(idx);
				}
			}
			stp->ai_actively_pursues_temp.clear();
		}

		Num_engine_wash_types = 0;
		Num_ship_classes = 0;

		//ships.tbl
		if ((rval = setjmp(parse_abort)) != 0) {
			mprintf(("TABLES: Unable to parse '%s'.  Code = %i.\n", current_ship_table, rval));
		} 
		else
		{			
			strcpy(default_player_ship, "");

			// static alias stuff - stupid, but it seems to be necessary
			tspecies_names = (char **) vm_malloc( Species_info.size() * sizeof(char*) );
			for (i = 0; i < Species_info.size(); i++)
				tspecies_names[i] = Species_info[i].species_name;

			//Parse main TBL first
			parse_shiptbl("ships.tbl");

			//Then other ones
			num_files = parse_modular_table(NOX("*-shp.tbm"), parse_shiptbl);

			if ( num_files > 0 ) {
				Module_ship_weapons_loaded = true;
			}
		}

		ships_inited = 1;

		// cleanup
		
		//Unload ship templates, we don't need them anymore.
		Ship_templates.clear();
		
		if(tspecies_names != NULL)
		{
			vm_free(tspecies_names);
			tspecies_names = NULL;
		}

		// NULL out "dynamic" subsystem ptr's
		for (i = 0; i < NUM_SHIP_SUBSYSTEM_SETS; i++)
			Ship_subsystems[i] = NULL;
	}

	//loadup the cloaking map
//	CLOAKMAP = bm_load_animation("cloakmap",&CLOAKFRAMES, &CLOAKFPS,1);
/*	if (CLOAKMAP == -1)
	{
		CLOAKMAP = bm_load("cloakmap");
	}*/

	ship_level_init();	// needed for FRED
}

int Man_thruster_reset_timestamp = 0;

static void ship_clear_subsystems()
{
	int i;

	for (i = 0; i < NUM_SHIP_SUBSYSTEM_SETS; i++) {
		if (Ship_subsystems[i] != NULL) {
			vm_free(Ship_subsystems[i]);
			Ship_subsystems[i] = NULL;
		}
	}

	Num_ship_subsystems = 0;
	Num_ship_subsystems_allocated = 0;
}

static void ship_allocate_subsystems(int num_so, bool page_in = false)
{
	int idx, i;
	int num_subsystems_save = 0;

	// "0" itself is safe
	if (num_so < 0) {
		Int3();
		return;
	}

	// allow a page-in thingy, so that we can grab as much as possible before mission
	// start, but without messing up our count for future things
	if (page_in)
		num_subsystems_save = Num_ship_subsystems;

	Num_ship_subsystems += num_so;

	// bail if we don't actually need any more
	if ( Num_ship_subsystems < Num_ship_subsystems_allocated )
		return;

	mprintf(("Allocating space for at least %i new ship subsystems ... ", num_so));

	// we might need more than one set worth of new subsystems, so make as many as required
	do {
		for (idx = 0; idx < NUM_SHIP_SUBSYSTEM_SETS; idx++) {
			if (Ship_subsystems[idx] == NULL)
				break;
		}

		// safety check, but even if we have this here it will fubar something else later, so we're screwed either way
		if (idx == NUM_SHIP_SUBSYSTEM_SETS) {
			Int3();
			return;
		}

		Ship_subsystems[idx] = (ship_subsys*) vm_malloc( sizeof(ship_subsys) * NUM_SHIP_SUBSYSTEMS_PER_SET );
		memset( Ship_subsystems[idx], 0, sizeof(ship_subsys) * NUM_SHIP_SUBSYSTEMS_PER_SET );

		// append the new set to our free list
		for (i = 0; i < NUM_SHIP_SUBSYSTEMS_PER_SET; i++)
			list_append( &ship_subsys_free_list, &Ship_subsystems[idx][i] );

		Num_ship_subsystems_allocated += NUM_SHIP_SUBSYSTEMS_PER_SET;
	} while ( (Num_ship_subsystems - Num_ship_subsystems_allocated) > 0 );

	if (page_in)
		Num_ship_subsystems = num_subsystems_save;

	mprintf((" a total of %i is now available (%i in-use).\n", Num_ship_subsystems_allocated, Num_ship_subsystems));
}

// This will get called at the start of each level.
void ship_level_init()
{
	int i;

	// Reset everything between levels

	// mwa removed 11/24/97  num_ships = 0;
	Num_exited_ships = 0;
	for (i=0; i<MAX_SHIPS; i++ )
	{
		Ships[i].ship_name[0] = '\0';
		Ships[i].objnum = -1;
	}

	for ( i = 0; i < MAX_EXITED_SHIPS; i++ ) {
		memset ( &Ships_exited[i], 0, sizeof(exited_ship) );
		Ships_exited[i].obj_signature = -1;
	}

	Num_wings = 0;
	for (i = 0; i < MAX_WINGS; i++ )
	{
		Wings[i].num_waves = -1;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;	// Goober5000 - default to no wing insignia
												// don't worry about releasing textures because
												// they are released automatically when the model
												// is unloaded (because they are part of the model)
	}

	for (i=0; i<MAX_STARTING_WINGS; i++)
		Starting_wings[i] = -1;

	for (i=0; i<MAX_SQUADRON_WINGS; i++)
		Squadron_wings[i] = -1;

	for (i=0; i<MAX_TVT_WINGS; i++)
		TVT_wings[i] = -1;

	// Goober5000

	// set starting wing names to default
	strcpy(Starting_wing_names[0], "Alpha");
	strcpy(Starting_wing_names[1], "Beta");
	strcpy(Starting_wing_names[2], "Gamma");

	// set squadron wing names to default
	strcpy(Squadron_wing_names[0], "Alpha");
	strcpy(Squadron_wing_names[1], "Beta");
	strcpy(Squadron_wing_names[2], "Gamma");
	strcpy(Squadron_wing_names[3], "Delta");
	strcpy(Squadron_wing_names[4], "Epsilon");

	// set tvt wing names to default
	strcpy(TVT_wing_names[0], "Alpha");
	strcpy(TVT_wing_names[1], "Zeta");


	// Empty the subsys list
	ship_clear_subsystems();
	list_init( &ship_subsys_free_list );

	Laser_energy_out_snd_timer = 1;
	Missile_out_snd_timer		= 1;

	ship_obj_list_init();

	Ship_cargo_check_timer = 1;

	shipfx_large_blowup_level_init();

	Man_thruster_reset_timestamp = timestamp(0);
}

// function to add a ship onto the exited ships list.  The reason parameter
// tells us why the ship left the mission (i.e. departed or destroyed)
void ship_add_exited_ship( ship *sp, int reason )
{
	int i, entry;

	// reuse oldest slots if none left
	if ( Num_exited_ships == MAX_EXITED_SHIPS ) {
		int oldest_entry;

		// find the oldest entry
		oldest_entry = 0;
		for ( i = 1; i < MAX_SHIPS; i++ ) 
		{
			// Karajorma - Don't remove ships which are being tracked for loadout purposes
			if (!(Ships_exited[i].flags & SEF_SHIP_EXITED_STORE))
			{
				if ( Ships_exited[i].time < Ships_exited[oldest_entry].time ) 
				{
					oldest_entry = i;
				}
			}
		}
		entry = oldest_entry;
	} else {
		entry = Num_exited_ships;
		Num_exited_ships++;
	}

	strcpy( Ships_exited[entry].ship_name, sp->ship_name );
	Ships_exited[entry].obj_signature = Objects[sp->objnum].signature;
	Ships_exited[entry].ship_class = sp->ship_info_index;
	Ships_exited[entry].team = sp->team;
	Ships_exited[entry].ship_class = sp->ship_info_index;
	Ships_exited[entry].flags = reason;
	// if ship is red alert, flag as such
	if (sp->flags & SF_RED_ALERT_STORE_STATUS) {
		Ships_exited[entry].flags |= SEF_RED_ALERT_CARRY;
	}
	// if the ship is team loadout, flag it so it can't be bumped
	if (sp->flags2 & SF2_TEAM_LOADOUT_STORE_STATUS) 
	{
		Ships_exited[entry].flags |= SEF_SHIP_EXITED_STORE;
	}

	Ships_exited[entry].time = Missiontime;
	Ships_exited[entry].hull_strength = int(Objects[sp->objnum].hull_strength);

	Ships_exited[entry].cargo1 = sp->cargo1;

	Ships_exited[entry].time_cargo_revealed = (fix)0;
	if ( sp->flags & SF_CARGO_REVEALED )
	{
		Ships_exited[entry].flags |= SEF_CARGO_KNOWN;
		Ships_exited[entry].time_cargo_revealed = sp->time_cargo_revealed;
	}

	if ( sp->time_first_tagged > 0 )
		Ships_exited[entry].flags |= SEF_BEEN_TAGGED;
}

// function which attempts to find information about an exited ship based on shipname
int ship_find_exited_ship_by_name( char *name )
{
	int i;

	for (i = 0; i < Num_exited_ships; i++) {
		if ( !stricmp(name, Ships_exited[i].ship_name) )
			return i;
	}

	return -1;
}

// function which attempts to find information about an exited ship based on shipname
int ship_find_exited_ship_by_signature( int signature )
{
	int i;

	for (i = 0; i < Num_exited_ships; i++) {
		if ( signature == Ships_exited[i].obj_signature )
			return i;
	}

	return -1;
}


void physics_ship_init(object *objp)
{
	ship_info	*sinfo = &Ship_info[Ships[objp->instance].ship_info_index];
	physics_info	*pi = &objp->phys_info;
	polymodel *pm = model_get(sinfo->model_num);

	// use mass and I_body_inv from POF read into polymodel
	physics_init(pi);
	pi->mass = pm->mass * sinfo->density;

	if (pi->mass==0.0f)
	{
		vec3d size;
		vm_vec_sub(&size,&pm->maxs,&pm->mins);
		float vmass=size.xyz.x*size.xyz.y*size.xyz.z;
		float amass=4.65f*(float)pow(vmass,(2.0f/3.0f));

		nprintf(("Physics", "pi->mass==0.0f. setting to %f",amass));
		Warning(LOCATION, "%s (%s) has no mass! setting to %f", sinfo->name, sinfo->pof_file, amass);
		pm->mass=amass;
		pi->mass=amass*sinfo->density;
	}

	pi->center_of_mass = pm->center_of_mass;
	pi->I_body_inv = pm->moment_of_inertia;
	// scale pm->I_body_inv value by density
	vm_vec_scale( &pi->I_body_inv.vec.rvec, sinfo->density );
	vm_vec_scale( &pi->I_body_inv.vec.uvec, sinfo->density );
	vm_vec_scale( &pi->I_body_inv.vec.fvec, sinfo->density );

	pi->side_slip_time_const = sinfo->damp;
	pi->rotdamp = sinfo->rotdamp;
	pi->max_vel = sinfo->max_vel;
	pi->afterburner_max_vel = sinfo->afterburner_max_vel;
	pi->max_rotvel = sinfo->max_rotvel;
	pi->max_rear_vel = sinfo->max_rear_vel;
	pi->flags |= PF_ACCELERATES;




	pi->forward_accel_time_const=sinfo->forward_accel;
	pi->afterburner_forward_accel_time_const=sinfo->afterburner_forward_accel;
	pi->forward_decel_time_const=sinfo->forward_decel;
	pi->slide_accel_time_const=sinfo->slide_accel;
	pi->slide_decel_time_const=sinfo->slide_decel;

	if ( (pi->max_vel.xyz.x > 0.000001f) || (pi->max_vel.xyz.y > 0.000001f) )
		pi->flags |= PF_SLIDE_ENABLED;

	if ( sinfo->glide_cap > 0.000001f || sinfo->glide_cap < -0.000001f )		//Backslash
		pi->glide_cap = sinfo->glide_cap;
	else
		pi->glide_cap = MAX(MAX(pi->max_vel.xyz.z, sinfo->max_overclocked_speed), pi->afterburner_max_vel.xyz.z);
	// If there's not a value for +Max Glide Speed set in the table, we want this cap to default to the fastest speed the ship can go.
	// However, a negative value means we want no cap, thus allowing nearly infinite maximum gliding speeds.

	vm_vec_zero(&pi->vel);
	vm_vec_zero(&pi->rotvel);
	pi->speed = 0.0f;
	pi->heading = 0.0f;
//	pi->accel = 0.0f;
	vm_set_identity(&pi->last_rotmat);
}

//Function to get the type of the given ship as a string
//WMC - I created you, I can DESTROY you!
int ship_get_type(char* output, ship_info *sip)
{
	if(sip->class_type < 0) {
		strcpy(output, "Unknown");
		return 0;
	}

	strcpy(output, Ship_types[sip->class_type].name);
	return 1;
}

// function to set the orders allowed for a ship -- based on ship type.  This value might get overridden
// by a value in the mission file.
int ship_get_default_orders_accepted( ship_info *sip )
{
	if(sip->class_type >= 0) {
		return Ship_types[sip->class_type].ai_player_orders;
	} else {
		return 0;
	}
}

vec3d get_submodel_offset(int model, int submodel){
	polymodel*pm = model_get(model);
	if(pm->submodel[submodel].parent == -1)
		return pm->submodel[submodel].offset;
	vec3d ret = pm->submodel[submodel].offset;
	vec3d v = get_submodel_offset(model,pm->submodel[submodel].parent);
	vm_vec_add2(&ret, &v);
	return ret;

}

void ship_set(int ship_index, int objnum, int ship_type)
{
	int i;

	object	*objp = &Objects[objnum];
	ship	*shipp = &Ships[ship_index];
	ship_weapon	*swp = &shipp->weapons;
	ship_info	*sip = &(Ship_info[ship_type]);

	// Create n!
	// sprintf(shipp->ship_name, "%s %d", Ship_info[ship_type].name, ship_index); // moved to ship_create()
	Assert(strlen(shipp->ship_name) < NAME_LENGTH - 1);
	shipp->ship_info_index = ship_type;
	shipp->objnum = objnum;
	shipp->group = 0;
	shipp->reinforcement_index = -1;
	shipp->cargo1 = 0;
	shipp->hotkey = -1;
	shipp->score = 0;
	shipp->escort_priority = 0;
	shipp->special_exp_index = -1;
	shipp->special_hitpoint_index = -1;
	shipp->num_hits = 0;
	shipp->flags = 0;
	shipp->flags2 = 0;
	shipp->wash_killed = 0;
	shipp->time_cargo_revealed = 0;
	shipp->time_first_tagged = 0;
	shipp->wash_timestamp = timestamp(0);
	shipp->large_ship_blowup_index = -1;
	shipp->respawn_priority = 0;
	shipp->warp_anim = -1;
	shipp->warp_anim_fps = 0;
	shipp->warp_anim_nframes = 0;
	for (i=0; i<NUM_SUB_EXPL_HANDLES; i++) {
		shipp->sub_expl_sound_handle[i] = -1;
	}

	if ( !Fred_running ) {
		shipp->start_warp_time = timestamp(-1);
		shipp->final_warp_time = timestamp(-1);
		shipp->final_death_time = timestamp(-1);	// There death sequence ain't start et.
		shipp->end_death_time = 0;
		shipp->death_time = -1;
		shipp->really_final_death_time = timestamp(-1);	// There death sequence ain't start et.
		shipp->next_fireball = timestamp(-1);		// When a random fireball will pop up
		shipp->next_hit_spark = timestamp(-1);		// when a hit spot will spark
		for (i=0; i<MAX_SHIP_ARCS; i++ )	{
			shipp->arc_timestamp[i] = timestamp(-1);		// Mark this arc as unused
		}
		shipp->arc_next_time = timestamp(-1);		// No electrical arcs yet.
	} else {		// the values should be different for Fred
		shipp->start_warp_time = -1;
		shipp->final_warp_time = -1;
		shipp->final_death_time = 0;
		shipp->end_death_time = 0;
		shipp->death_time = -1;
		shipp->really_final_death_time = -1;
		shipp->next_fireball = -1;
		shipp->next_hit_spark = -1;
		for (i=0; i<MAX_SHIP_ARCS; i++ )	{
			shipp->arc_timestamp[i] = -1;
		}
		shipp->arc_next_time = -1;
	}
	shipp->team = 0;					//	Default, probably get overridden.
	shipp->arrival_location = 0;
	shipp->arrival_distance = 0;
	shipp->arrival_anchor = -1;
	shipp->arrival_path_mask = 0;
	shipp->arrival_delay = 0;
	shipp->arrival_cue = -1;
	shipp->departure_location = 0;
	shipp->departure_path_mask = 0;
	shipp->departure_delay = 0;
	shipp->departure_cue = -1;
	shipp->shield_hits = 0;							//	No shield hits yet on this baby.
	shipp->current_max_speed = Ship_info[ship_type].max_speed;

	shipp->alt_type_index = -1;

	shipp->lightning_stamp = -1;

	shipp->emp_intensity = -1.0f;
	shipp->emp_decr = 0.0f;

	shipp->targeting_laser_bank = -1;
	shipp->targeting_laser_objnum = -1;

	shipp->determination = 10;
	shipp->wingnum = -1;
	for (i = 0; i < MAX_PLAYERS; i++)
		shipp->last_targeted_subobject[i] = NULL;

	if (Fred_running){
		shipp->ship_max_hull_strength = 100.0f;
	} else {
		shipp->ship_max_hull_strength = sip->max_hull_strength;
	}
	objp->sim_hull_strength = objp->hull_strength = shipp->ship_max_hull_strength;
	
	shipp->afterburner_fuel = sip->afterburner_fuel_capacity;

	shipp->cmeasure_count = sip->cmeasure_max;

	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ )
	{
		swp->primary_bank_ammo[i] = 0;						// added by Goober5000
		swp->next_primary_fire_stamp[i] = timestamp(0);	
		swp->primary_bank_rearm_time[i] = timestamp(0);		// added by Goober5000

		swp->primary_animation_position[i] = MA_POS_NOT_SET;
		swp->secondary_animation_position[i] = MA_POS_NOT_SET;
		swp->primary_animation_done_time[i] = timestamp(0);
		swp->secondary_animation_done_time[i] = timestamp(0);
	}

	shipp->cmeasure_fire_stamp = timestamp(0);

	// handle ballistic primaries - kinda hackish; is this actually necessary?
	// because I think it's not needed - when I accidentally left this unreachable
	// it didn't cause any problems - Goober5000
	for ( i = 0; i < sip->num_primary_banks; i++ )
	{
		float weapon_size;
		weapon_size = Weapon_info[sip->primary_bank_weapons[i]].cargo_size;

		if ( weapon_size > 0.0f )
		{
			if (Fred_running)
			{
				swp->primary_bank_ammo[i] = 100;
			}
			else
			{
				swp->primary_bank_ammo[i] = fl2i(sip->primary_bank_ammo_capacity[i] / weapon_size + 0.5f );
			}
		}
	}

	// conventional secondary banks
	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ )
	{
		if (Fred_running)
		{
			swp->secondary_bank_ammo[i] = 100;
		}
		else
		{
			swp->secondary_bank_ammo[i] = 0;
		}
			
		swp->secondary_bank_ammo[i] = 0;
		swp->secondary_next_slot[i] = 0;
		swp->next_secondary_fire_stamp[i] = timestamp(0);
		swp->secondary_bank_rearm_time[i] = timestamp(0);		// will be able to rearm this bank immediately
	}

	for ( i = 0; i < sip->num_secondary_banks; i++ )
	{
		float weapon_size;
		weapon_size = Weapon_info[sip->secondary_bank_weapons[i]].cargo_size;
		Assert( weapon_size > 0.0f );
		if (Fred_running)
		{
			swp->secondary_bank_ammo[i] = 100;
		}
		else
		{
			swp->secondary_bank_ammo[i] = fl2i(sip->secondary_bank_ammo_capacity[i] / weapon_size + 0.5f );
		}
	}

	swp->current_primary_bank = -1;
	swp->current_secondary_bank = -1;


	if ( sip->num_primary_banks > 0 ) {
		if ( swp->primary_bank_weapons[BANK_1] >= 0 ) {
			swp->current_primary_bank = BANK_1;
		} else {
			swp->current_primary_bank = -1;
		}
	}
	else {
		swp->current_primary_bank = -1;
	}

	if ( sip->num_secondary_banks > 0 ) {
		if ( swp->secondary_bank_weapons[BANK_1] >= 0 ) {
			swp->current_secondary_bank = BANK_1;
		} else {
			swp->current_secondary_bank = -1;
		}
	}
	else {
		swp->current_secondary_bank = -1;
	}

	shipp->current_cmeasure = sip->cmeasure_type;

	ets_init_ship(objp);	// init ship fields that are used for the ETS

	physics_ship_init(objp);
	if (Fred_running) {
		shipp->ship_max_shield_strength = 100.0f;
		shipp->ship_max_hull_strength = 100.0f;
		shield_set_quad(objp, 0, 100.0f);
	} else {
		shipp->ship_max_shield_strength = sip->max_shield_strength;
		shipp->ship_max_hull_strength = sip->max_hull_strength;
		shield_set_strength(objp, sip->max_shield_strength);
	}

	shipp->target_shields_delta = 0.0f;
	shipp->target_weapon_energy_delta = 0.0f;

	ai_object_init(objp, shipp->ai_index);
	shipp->weapons.ai_class = Ai_info[shipp->ai_index].ai_class;
	shipp->shield_integrity = NULL;
//	shipp->sw.blast_duration = -1;	// init shockwave struct

	shipp->ship_guardian_threshold = 0;

	subsys_set(objnum);
	shipp->orders_accepted = ship_get_default_orders_accepted( sip );
	shipp->num_swarm_missiles_to_fire = 0;	
	shipp->num_turret_swarm_info = 0;
	shipp->death_roll_snd = -1;
	shipp->thruster_bitmap = -1;
	shipp->thruster_frame = 0.0f;
	shipp->thruster_glow_bitmap = -1;
	shipp->thruster_glow_noise = 1.0f;
	shipp->thruster_glow_frame = 0.0f;
	shipp->next_engine_stutter = 1;
	shipp->persona_index = -1;
	shipp->flags |= SF_ENGINES_ON;
	shipp->subsys_disrupted_flags=0;
	shipp->subsys_disrupted_check_timestamp=timestamp(0);

	shipp->base_texture_anim_frametime = 0;

	// Bobboau's stuff
	shipp->thruster_secondary_glow_bitmap = -1;
	shipp->thruster_tertiary_glow_bitmap = -1;

	// swarm missile stuff
	shipp->next_swarm_fire = 1;

	// corkscrew missile stuff
	shipp->next_corkscrew_fire = 1;

	// field for score
	shipp->score = sip->score;

	// tag
	shipp->tag_left = -1.0f;
	shipp->level2_tag_left = -1.0f;

	// multiplayer field initializations
	for (i = 0; i < MAX_PLAYERS; i++ ) {
		shipp->np_updates[i].update_stamp = -1;
		shipp->np_updates[i].status_update_stamp = -1;
		shipp->np_updates[i].subsys_update_stamp = -1;
		shipp->np_updates[i].seq = 0;		
	}		
	extern int oo_arrive_time_count[MAX_SHIPS];		
	extern int oo_interp_count[MAX_SHIPS];	
	oo_arrive_time_count[shipp - Ships] = 0;				
	oo_interp_count[shipp - Ships] = 0;	

	shipp->primitive_sensor_range = DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE;

	shipp->special_warp_objnum = -1;

	shipp->current_viewpoint = 0;

	// set awacs warning flags so awacs ship only asks for help once at each level
	shipp->awacs_warning_flag = AWACS_WARN_NONE;

	// Goober5000 - revised texture replacement
	shipp->replacement_textures = NULL;

	shipp->glow_point_bank_active.clear();

//	for(i=0; i<MAX_SHIP_DECALS; i++)
//		shipp->decals[i].is_valid = 0;

	
//	for(i = 1; i < MAX_SHIP_DECALS; i++){
//		shipp->decals[i].timestamp = timestamp();
//		shipp->decals[i].is_valid = 0;
//	}

	shipp->cloak_stage = 0;
	shipp->texture_translation_key=vmd_zero_vector;
	shipp->current_translation=vmd_zero_vector;
	shipp->time_until_full_cloak=timestamp(0);
	shipp->cloak_alpha=255;

//	shipp->ab_count = 0;
	shipp->ship_decal_system.n_decal_textures = 0;
	shipp->ship_decal_system.decals = NULL;
	shipp->ship_decal_system.decals_modified = false;
	shipp->ship_decal_system.max_decals = sip->max_decals;

	// fighter bay door stuff
	shipp->bay_doors_status = MA_POS_NOT_SET;
	shipp->bay_doors_wanting_open = 0;
	shipp->bay_doors_need_open = false;
	shipp->bay_doors_anim_done_time = 0;
	shipp->bay_doors_launched_from = 0;
	shipp->bay_doors_parent_shipnum = -1;

	for(i = 0; i<MAX_SHIP_SECONDARY_BANKS; i++){
		for(int k = 0; k<MAX_SLOTS; k++){
			shipp->secondary_point_reload_pct[i][k] = 1.0f;
		}
	}
	for(i = 0; i<MAX_SHIP_SECONDARY_BANKS; i++){
		if(Weapon_info[swp->secondary_bank_weapons[i]].fire_wait == 0.0){
			shipp->reload_time[i] = 1.0f;
		}else{
			shipp->reload_time[i] = 1.0f/Weapon_info[swp->secondary_bank_weapons[i]].fire_wait;
		}
	}
	for(i = 0; i<MAX_SHIP_PRIMARY_BANKS; i++){
		shipp->primary_rotate_rate[i] = 0.0f;
		shipp->primary_rotate_ang[i] = 0.0f;
	}
/*
	for(i = 0; i < sip->n_subsystems; i++){
		model_subsystem* ms = &sip->subsystems[i];
		
		if(ms->subobj_num >= 0)ms->trigger.snd_pnt = get_submodel_offset(sip->modelnum, ms->subobj_num);
		else ms->trigger.snd_pnt = ms->pnt;
		ms->trigger.obj_num = objnum;
	}
	shipp->flare_life = 0;
	shipp->flare_bm = bm_load("boom");
*/
	//Thrusters
	for(i = 0; i < MAX_MAN_THRUSTERS; i++)
	{
		shipp->thrusters_start[i] = 0;
		shipp->thrusters_sounds[i] = -1;
	}

	// Alternate ship class stuff
	shipp->num_alt_class_one = 0;
	shipp->num_alt_class_two = 0;

	for (i = 0; i < MAX_ALT_CLASS_1; i++)
	{
		shipp->alt_class_one[i] = -1;
		shipp->alt_class_one_variable[i] = -1;
	}

	for (i = 0; i < MAX_ALT_CLASS_2; i++)
	{
		shipp->alt_class_two[i] = -1;
		shipp->alt_class_two_variable[i] = -1;
	}
}

// function which recalculates the overall strength of subsystems.  Needed because
// several places in FreeSpace change subsystem strength and all this data needs to
// be kept up to date.
void ship_recalc_subsys_strength( ship *shipp )
{
	int i;
	ship_subsys *ship_system;

	// fill in the subsys_info fields for all particular types of subsystems
	// make the current strength be 1.0.  If there are initial conditions on the ship, then
	// the mission parse code should take care of setting that.
	for (i = 0; i < SUBSYSTEM_MAX; i++) {
		shipp->subsys_info[i].num = 0;
		shipp->subsys_info[i].total_hits = 0.0f;
		shipp->subsys_info[i].current_hits = 0.0f;
	}

	// count all of the subsystems of a particular type.  For each generic type of subsystem, we store the
	// total count of hits.  (i.e. for 3 engines, we store the sum of the max_hits for each engine)
	for ( ship_system = GET_FIRST(&shipp->subsys_list); ship_system != END_OF_LIST(&shipp->subsys_list); ship_system = GET_NEXT(ship_system) ) {
		int type;

		type = ship_system->system_info->type;
		Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );
		shipp->subsys_info[type].num++;
		shipp->subsys_info[type].total_hits += ship_system->max_hits;
		shipp->subsys_info[type].current_hits += ship_system->current_hits;

		//Get rid of any persistent sounds on the subsystem
		//This is inefficient + sloppy but there's not really an easy way to handle things
		//if a subsystem is brought back from the dead, other than this
		if(ship_system->current_hits < ship_system->max_hits)
		{
			obj_snd_delete_type(shipp->objnum, -1, ship_system);
			if(ship_system->system_info->dead_snd != -1)
				obj_snd_assign(shipp->objnum, ship_system->system_info->dead_snd, &ship_system->system_info->pnt, 1);
		}
		else
		{
			obj_snd_delete_type(shipp->objnum, ship_system->system_info->dead_snd, ship_system);
			if(ship_system->system_info->alive_snd != -1)
				obj_snd_assign(shipp->objnum, ship_system->system_info->alive_snd, &ship_system->system_info->pnt, 1);
		}
	}

	// set any ship flags which should be set.  unset the flags since we might be repairing a subsystem
	// through sexpressions.
	shipp->flags &= ~SF_DISABLED;
	if ( (shipp->subsys_info[SUBSYSTEM_ENGINE].num > 0) && (shipp->subsys_info[SUBSYSTEM_ENGINE].current_hits == 0.0f) ){
		shipp->flags |= SF_DISABLED;
	} else {
		ship_reset_disabled_physics( &Objects[shipp->objnum], shipp->ship_info_index );
	}

	/*
	shipp->flags &= ~SF_DISARMED;
	if ( (shipp->subsys_info[SUBSYSTEM_TURRET].num > 0) && (shipp->subsys_info[SUBSYSTEM_TURRET].current_hits == 0.0f) ){
		shipp->flags |= SF_DISARMED;
	}
	*/

	if (shipp->subsys_info[SUBSYSTEM_SHIELD_GENERATOR].num > 0)
	{
		if (shipp->subsys_info[SUBSYSTEM_SHIELD_GENERATOR].current_hits == 0.0f)
			Objects[shipp->objnum].flags |= OF_NO_SHIELDS;
		else
			Objects[shipp->objnum].flags &= ~OF_NO_SHIELDS;
	}
}

// routine to possibly fixup the model subsystem information for this ship pointer.  Needed when
// ships share the same model.
void ship_copy_subsystem_fixup(ship_info *sip)
{
	int i, model_num;

	model_num = sip->model_num;

	// since we allow a model file to be shared between several ships, we must check to be sure that our
	// subsystems have been loaded properly
	/*
	subsystems_needed = 0;
	for (i = 0; i < sip->n_subsystems; i++ ) {
		if ( sip->subsystems[i].model_num == -1 ){
			subsystems_needed++;
		}
	}
	*/

	// if we need to get information for all our subsystems, we need to find another ship with the same model
	// number as our own and that has the model information
	// if ( subsystems_needed == sip->n_subsystems ) {
		for ( i = 0; i < Num_ship_classes; i++ ) {
			model_subsystem *msp;

			if ( (Ship_info[i].model_num != model_num) || (&Ship_info[i] == sip) ){
				continue;
			}

			// see if this ship has subsystems and a model for the subsystems.  We only need check the first
			// subsystem since previous error checking would have trapped its loading as an error.
			Assert( Ship_info[i].n_subsystems == sip->n_subsystems );

			msp = &Ship_info[i].subsystems[0];
			model_copy_subsystems( sip->n_subsystems, &(sip->subsystems[0]), msp );
			sip->flags |= SIF_PATH_FIXUP;
			break;
		}
	// }

}


// ignore_subsys_info => default parameter with value of 0.  This is
//								 only set to 1 by the save/restore code
void subsys_set(int objnum, int ignore_subsys_info)
{	
	ship	*shipp = &Ships[Objects[objnum].instance];
	ship_info	*sinfo = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];
	model_subsystem *model_system;
	ship_subsys *ship_system;
	int i, j, k;

	// set up the subsystems for this ship.  walk through list of subsystems in the ship-info array.
	// for each subsystem, get a new ship_subsys instance and set up the pointers and other values
	list_init ( &shipp->subsys_list );								// initialize the ship's list of subsystems

	// make sure to have allocated the number of subsystems we require
	ship_allocate_subsystems( sinfo->n_subsystems );

	for ( i = 0; i < sinfo->n_subsystems; i++ )
	{
		model_system = &(sinfo->subsystems[i]);
		if (model_system->model_num < 0) {
			Warning (LOCATION, "Invalid subobj_num or model_num in subsystem '%s' on ship type '%s'.\nNot linking into ship!\n\n(This warning means that a subsystem was present in the table entry and not present in the model\nit should probably be removed from the table or added to the model.)\n", model_system->subobj_name, sinfo->name );
			continue;
		}

		// set up the linked list
		ship_system = GET_FIRST( &ship_subsys_free_list );		// get a new element from the ship_subsystem array
		Assert ( ship_system != &ship_subsys_free_list );		// shouldn't have the dummy element
		list_remove( ship_subsys_free_list, ship_system );	// remove the element from the array
		list_append( &shipp->subsys_list, ship_system );		// link the element into the ship

		ship_system->system_info = model_system;				// set the system_info pointer to point to the data read in from the model

		// zero flags
		ship_system->flags = 0;
		ship_system->weapons.flags = 0;

		// Goober5000
		if (model_system->flags & MSS_FLAG_UNTARGETABLE)
			ship_system->flags |= SSF_UNTARGETABLE;

		// Goober5000 - this has to be moved outside back to parse_create_object, because
		// a lot of the ship creation code is duplicated in several points and overwrites
		// previous things... ugh.
		ship_system->max_hits = model_system->max_subsys_strength;	// * shipp->ship_max_hull_strength / sinfo->max_hull_strength;

		if ( !Fred_running ){
			ship_system->current_hits = ship_system->max_hits;		// set the current hits
		} else {
			ship_system->current_hits = 0.0f;				// Jason wants this to be 0 in Fred.
		}

		ship_system->subsys_guardian_threshold = 0;

		ship_system->turret_next_fire_stamp = timestamp(0);
		ship_system->turret_next_enemy_check_stamp = timestamp(0);
		ship_system->turret_enemy_objnum = -1;
		ship_system->turret_next_fire_stamp = timestamp((int) frand_range(1.0f, 500.0f));	// next time this turret can fire
		ship_system->turret_last_fire_direction = model_system->turret_norm;
		ship_system->turret_next_fire_pos = 0;
		ship_system->turret_time_enemy_in_range = 0.0f;
		ship_system->disruption_timestamp=timestamp(0);
		ship_system->turret_pick_big_attack_point_timestamp = timestamp(0);
		vm_vec_zero(&ship_system->turret_big_attack_point);
		for(j = 0; j < NUM_TURRET_ORDER_TYPES; j++)
		{
			//WMC - Set targeting order to default.
			ship_system->turret_targeting_order[j] = j;
		}

		ship_system->subsys_cargo_name = -1;
		ship_system->time_subsys_cargo_revealed = 0;
		
		j = 0;
		for (k=0; k<MAX_SHIP_PRIMARY_BANKS; k++){
			if (model_system->primary_banks[k] != -1) {
				ship_system->weapons.primary_bank_weapons[j] = model_system->primary_banks[k];
				ship_system->weapons.primary_bank_capacity[j] = model_system->primary_bank_capacity[k];	// added by Goober5000
				ship_system->weapons.next_primary_fire_stamp[j++] = timestamp(0);
			}
		}

		ship_system->weapons.num_primary_banks = j;

		j = 0;
		for (k=0; k<MAX_SHIP_SECONDARY_BANKS; k++){
			if (model_system->secondary_banks[k] != -1) {
				ship_system->weapons.secondary_bank_weapons[j] = model_system->secondary_banks[k];
				ship_system->weapons.secondary_bank_capacity[j] = model_system->secondary_bank_capacity[k];
				ship_system->weapons.next_secondary_fire_stamp[j++] = timestamp(0);
			}
		}

		ship_system->weapons.num_secondary_banks = j;
		ship_system->weapons.current_primary_bank = -1;
		ship_system->weapons.current_secondary_bank = -1;
		
		for (k=0; k<MAX_SHIP_SECONDARY_BANKS; k++) {
			ship_system->weapons.secondary_bank_ammo[k] = (Fred_running ? 100 : ship_system->weapons.secondary_bank_capacity[k]);

			ship_system->weapons.secondary_next_slot[k] = 0;
		}

		// Goober5000
		for (k=0; k<MAX_SHIP_PRIMARY_BANKS; k++)
		{
			ship_system->weapons.primary_bank_ammo[k] = (Fred_running ? 100 : ship_system->weapons.primary_bank_capacity[k]);
		}

		ship_system->weapons.last_fired_weapon_index = -1;
		ship_system->weapons.last_fired_weapon_signature = -1;
		ship_system->weapons.detonate_weapon_time = -1;
		ship_system->weapons.ai_class = sinfo->ai_class;  // assume ai class of ship for turret

		// rapid fire (swarm) stuff
		ship_system->turret_swarm_info_index = -1;

		// AWACS stuff
		ship_system->awacs_intensity = model_system->awacs_intensity;
		ship_system->awacs_radius = model_system->awacs_radius;
		if (ship_system->awacs_intensity > 0) {
			ship_system->system_info->flags |= MSS_FLAG_AWACS;
		}

		ship_system->system_info->model_decal_system.decals = NULL;
		ship_system->system_info->model_decal_system.n_decal_textures = 0;
		ship_system->system_info->model_decal_system.decals_modified = false;
		ship_system->system_info->model_decal_system.max_decals = shipp->ship_decal_system.max_decals / 10;

		// turn_rate, turn_accel
		// model_set_instance_info
		float turn_accel = 0.5f;
		model_set_instance_info(&ship_system->submodel_info_1, model_system->turn_rate, turn_accel);

		// model_clear_instance_info( &ship_system->submodel_info_1 );
		model_clear_instance_info( &ship_system->submodel_info_2 );
	}

	if ( !ignore_subsys_info ) {
		ship_recalc_subsys_strength( shipp );
	}
}


#ifndef NDEBUG

//	Render docking information, NOT while in object's reference frame.
void render_dock_bays(object *objp)
{
	polymodel	*pm;
	dock_bay		*db;

//	sip = &Ship_info[Ships[objp->instance].ship_info_index];
	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	if (pm->docking_bays == NULL)
		return;

	if (pm->docking_bays[0].num_slots != 2)
		return;

	db = &pm->docking_bays[0];

	vertex	v0, v1;
	vec3d	p0, p1, p2, p3, nr;

	vm_vec_unrotate(&p0, &db->pnt[0], &objp->orient);
	vm_vec_add2(&p0, &objp->pos);
	g3_rotate_vertex(&v0, &p0);

	vm_vec_unrotate(&p1, &db->pnt[1], &objp->orient);
	vm_vec_add2(&p1, &objp->pos);
	g3_rotate_vertex(&v1, &p1);

	gr_set_color(255, 0, 0);
	g3_draw_line(&v0, &v1);

	vm_vec_avg(&p2, &p0, &p1);

	vm_vec_unrotate(&nr, &db->norm[0], &objp->orient);
	vm_vec_scale_add(&p3, &p2, &nr, 10.0f);

	g3_rotate_vertex(&v0, &p2);
	g3_rotate_vertex(&v1, &p3);
	gr_set_color(255, 255, 0);
	g3_draw_line(&v0, &v1);
	g3_draw_sphere(&v1, 1.25f);

}

#endif

int Ship_shadows = 0;

DCF_BOOL( ship_shadows, Ship_shadows )

MONITOR( NumShipsRend )

int Show_shield_hits = 0;
DCF_BOOL( show_shield_hits, Show_shield_hits )

int Show_tnorms = 0;
DCF_BOOL( show_tnorms, Show_tnorms )

int Show_paths = 0;
DCF_BOOL( show_paths, Show_paths )

int Show_fpaths = 0;
DCF_BOOL( show_fpaths, Show_fpaths )

void ship_find_warping_ship_helper(object *objp, dock_function_info *infop)
{
	// only check ships
	if (objp->type != OBJ_SHIP)
		return;

	// am I arriving or departing by warp?
	if ( Ships[objp->instance].flags & (SF_ARRIVING|SF_DEPART_WARP) )
	{
#ifndef NDEBUG
		// in debug builds, make sure only one of the docked objects has these flags set
		if (infop->maintained_variables.bool_value)
		{
			//WMC - This is annoying and triggered in sm2-10
			//Warning(LOCATION, "Ship %s and its docked ship %s are arriving or departing at the same time.\n",
			//Ships[infop->maintained_variables.objp_value->instance].ship_name, Ships[objp->instance].ship_name);
		}
#endif
		// we found someone
		infop->maintained_variables.bool_value = true;
		infop->maintained_variables.objp_value = objp;

#ifdef NDEBUG
		// return early in release builds
		infop->early_return_condition = true;
#endif
	}
}

std::vector<man_thruster_renderer> Man_thrusters;

//This batch renders all maneuvering thrusters in the array.
//It also clears the array every 10 seconds to keep mem usage down.
void batch_render_man_thrusters()
{
	man_thruster_renderer *mtr;
	uint mant_size = Man_thrusters.size();

	if (mant_size == 0)
		return;

	for(uint i = 0; i < mant_size; i++)
	{
		mtr = &Man_thrusters[i];
		gr_set_bitmap(mtr->bmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);

		mtr->man_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
		mtr->bmap_id = -1;	//Mark as free
	}

	//WMC - clear maneuvering thruster render queue every 10 seconds
	if(timestamp() - Man_thruster_reset_timestamp > 10000)
	{
		Man_thrusters.clear();
		Man_thruster_reset_timestamp = timestamp();
	}
}

//This function looks for a free slot in the man_thruster batch
//rendering array. Or, it returns a slot with the same bitmap
//ID as the maneuvering thruster.
//
//You could actually batch render anything that uses a simple bitmap
//on a single poly with this system...just plug the bitmap into bmap_frame
//and use as a normal batcher.
//
//Once calling this function, use man_batcher.allocate_add() to allocate or it will crash later.
//Then call man_batcher.draw*()
man_thruster_renderer *man_thruster_get_slot(int bmap_frame)
{
	man_thruster_renderer *mtr;
	uint mant_size = Man_thrusters.size();

	for(uint mi = 0; mi < mant_size; mi++)
	{
		mtr = &Man_thrusters[mi];
		if(mtr->bmap_id == bmap_frame)
			return mtr;
	}
	for(uint mj = 0; mj < mant_size; mj++)
	{
		mtr = &Man_thrusters[mj];
		if(mtr->bmap_id == -1)
		{
			mtr->bmap_id = bmap_frame;
			return mtr;
		}
	}

	Man_thrusters.push_back(man_thruster_renderer(bmap_frame));
	return &Man_thrusters[Man_thrusters.size()-1];
}

//WMC - used for FTL and maneuvering thrusters
geometry_batcher fx_batcher;
void ship_render(object * obj)
{
	int num = obj->instance;
	Assert( num >= 0);
	ship *shipp = &Ships[num];
	ship *warp_shipp = NULL;
	ship_info *sip = &Ship_info[Ships[num].ship_info_index];
	bool reset_proj_when_done = false;
	bool is_first_stage_arrival = false;
	dock_function_info dfi;


#if 0
	// show target when attacking big ship
	vec3d temp, target;
	ai_info *aip = &Ai_info[Ships[obj->instance].ai_index];
	if ( (aip->target_objnum >= 0)  && (Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index].flags & (SIF_SUPERCAP|SIF_CAPITAL|SIF_CRUISER)) ) {
		vm_vec_unrotate(&temp, &aip->big_attack_point, &Objects[aip->target_objnum].orient);
		vm_vec_add(&target, &temp, &Objects[aip->target_objnum].pos);

		vertex v0, v1;
		gr_set_color(128,0,0);
		g3_rotate_vertex( &v0, &obj->pos );
		g3_rotate_vertex( &v1, &target );

		g3_draw_line(&v0, &v1);

		g3_draw_sphere(&v1, 5.0f);
	}
#endif


	if ( obj == Viewer_obj)
	{
		if (ship_show_velocity_dot && (obj==Player_obj) )
		{
			vec3d p0,v;
			vertex v0;

			vm_vec_scale_add( &v, &obj->phys_info.vel, &obj->orient.vec.fvec, 3.0f );
			vm_vec_normalize( &v );
			
					
			vm_vec_scale_add( &p0, &obj->pos, &v, 20.0f);

			g3_rotate_vertex( &v0, &p0 );
			
			gr_set_color(0,128,0);
			g3_draw_sphere( &v0, 0.1f );
		}

		// Show the shield hit effect for the viewer.
		if ( Show_shield_hits )
		{
			shipp = &Ships[num];
			if (shipp->shield_hits)
			{
				create_shield_explosion_all(obj);
				shipp->shield_hits = 0;
			}
		}		

		if (!(sip->flags2 & SIF2_SHOW_SHIP_MODEL) && !(Viewer_mode & VM_TOPDOWN))
		{
			return;
		}

		//For in-ship cockpits. This is admittedly something of a hack
		if (!Cmdline_nohtl) {
			reset_proj_when_done = true;

			gr_end_view_matrix();
			gr_end_proj_matrix();

			gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, 0.05f, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}
	}

	MONITOR_INC( NumShipsRend, 1 );


	memset( &dfi, 0, sizeof(dock_function_info) );

	// look for a warping ship, whether for me or for anybody I'm docked with
	dock_evaluate_all_docked_objects(obj, &dfi, ship_find_warping_ship_helper);

	// if any docked objects are set to stage 1 arrival then set bool
	if (dfi.maintained_variables.bool_value) {
		warp_shipp = &Ships[dfi.maintained_variables.objp_value->instance];

		is_first_stage_arrival = ((warp_shipp->flags & SF_ARRIVING_STAGE_1) > 0);
	}


	// Make ships that are warping in not render during stage 1
	if (!is_first_stage_arrival)
	{				
		if ( Ship_shadows && shipfx_in_shadow( obj ) )	{
			light_set_shadow(1);
		} else {
			light_set_shadow(0);
		}

		ship_model_start(obj);

		uint render_flags = MR_NORMAL;
/*
		// Turn off model caching for the player ship in external view.
		if (obj == Player_obj)	{
			render_flags |= MR_ALWAYS_REDRAW;	
		}

		// Turn off model caching if this is the player's target.
		if ( Player_ai->target_objnum == OBJ_INDEX(obj))	{
			render_flags |= MR_ALWAYS_REDRAW;	
		}	
*/
	#ifndef NDEBUG
		if(Show_paths || Show_fpaths){
			render_flags |= MR_BAY_PATHS;
		}
	#endif

		// Only render electrical arcs if within 500m of the eye (for a 10m piece)
		if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f )	{
			int i;
			for (i=0; i<MAX_SHIP_ARCS; i++ )	{
				if ( timestamp_valid( shipp->arc_timestamp[i] ) )	{
				//	render_flags |= MR_ALWAYS_REDRAW;	// Turn off model caching if arcing.
					model_add_arc(sip->model_num, -1, &shipp->arc_pts[i][0], &shipp->arc_pts[i][1], shipp->arc_type[i]);
				}
			}
		}

	//	if((shipp->end_death_time - shipp->death_time) <= 0)shipp->end_death_time = 0;
	//	if((timestamp() - shipp->death_time) <= 0)shipp->end_death_time = 0;
	/*	if( !( shipp->large_ship_blowup_index >= 0 ) && timestamp_elapsed(shipp->death_time) && shipp->end_death_time){
			splodeingtexture = si->splodeing_texture;
			splodeing = true;
			splode_level = ((float)timestamp() - (float)shipp->death_time) / ((float)shipp->end_death_time - (float)shipp->death_time);
			//splode_level *= 2.0f;
			model_render( shipp->modelnum, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
			splodeing = false;
		}*/
	//	if(splode_level<=0.0f)shipp->end_death_time = 0;

		if ( shipp->large_ship_blowup_index >= 0 )	{
			shipfx_large_blowup_render(shipp);
		} else {

			//	ship_get_subsystem_strength( shipp, SUBSYSTEM_ENGINE)>ENGINE_MIN_STR
			//WMC - I suppose this is a bit hackish.
			physics_info *pi = &Objects[shipp->objnum].phys_info;
			float render_amount;
			fx_batcher.allocate(sip->num_maneuvering);	//Act as if all thrusters are going.

			for(int i = 0; i < sip->num_maneuvering; i++)
			{
				man_thruster *mtp = &sip->maneuvering[i];

				render_amount = 0.0f;

				//WMC - get us a steady value
				vec3d des_vel;
				vm_vec_rotate(&des_vel, &pi->desired_vel, &obj->orient);

				if(pi->desired_rotvel.xyz.x < 0 && (mtp->use_flags & MT_PITCH_UP)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
				} else if(pi->desired_rotvel.xyz.x > 0 && (mtp->use_flags & MT_PITCH_DOWN)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
				} else if(pi->desired_rotvel.xyz.y < 0 && (mtp->use_flags & MT_ROLL_RIGHT)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
				} else if(pi->desired_rotvel.xyz.y > 0 && (mtp->use_flags & MT_ROLL_LEFT)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
				} else if(pi->desired_rotvel.xyz.z < 0 && (mtp->use_flags & MT_BANK_RIGHT)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
				} else if(pi->desired_rotvel.xyz.z > 0 && (mtp->use_flags & MT_BANK_LEFT)) {
					render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
				}
				
				if(pi->flags & PF_GLIDING) {	//Backslash - show thrusters according to thrust amount, not speed
					if(pi->side_thrust > 0 && (mtp->use_flags & MT_SLIDE_RIGHT)) {
						render_amount = pi->side_thrust;
					} else if(pi->side_thrust < 0 && (mtp->use_flags & MT_SLIDE_LEFT)) {
						render_amount = -pi->side_thrust;
					} else if(pi->vert_thrust > 0 && (mtp->use_flags & MT_SLIDE_UP)) {
						render_amount = pi->vert_thrust;
					} else if(pi->vert_thrust < 0 && (mtp->use_flags & MT_SLIDE_DOWN)) {
						render_amount = -pi->vert_thrust;
					} else if(pi->forward_thrust > 0 && (mtp->use_flags & MT_FORWARD)) {
						render_amount = pi->forward_thrust;
					} else if(pi->forward_thrust < 0 && (mtp->use_flags & MT_REVERSE)) {
						render_amount = -pi->forward_thrust;
					}		// I'd almost advocate applying the above method to these all the time even without gliding,
				} else {	// because it looks more realistic, but I don't think the AI uses side_thrust or vert_thrust
					if(des_vel.xyz.x > 0 && (mtp->use_flags & MT_SLIDE_RIGHT)) {
						render_amount = fl_abs(des_vel.xyz.x) / pi->max_vel.xyz.x;
					} else if(des_vel.xyz.x < 0 && (mtp->use_flags & MT_SLIDE_LEFT)) {
						render_amount = fl_abs(des_vel.xyz.x) / pi->max_vel.xyz.x;
					} else if(des_vel.xyz.y > 0 && (mtp->use_flags & MT_SLIDE_UP)) {
						render_amount = fl_abs(des_vel.xyz.y) / pi->max_vel.xyz.y;
					} else if(des_vel.xyz.y < 0 && (mtp->use_flags & MT_SLIDE_DOWN)) {
						render_amount = fl_abs(des_vel.xyz.y) / pi->max_vel.xyz.y;
					} else if(des_vel.xyz.z > 0 && (mtp->use_flags & MT_FORWARD)) {
						render_amount = fl_abs(des_vel.xyz.z) / pi->max_vel.xyz.z;
					} else if(des_vel.xyz.z < 0 && (mtp->use_flags & MT_REVERSE)) {
						render_amount = fl_abs(des_vel.xyz.z) / pi->max_vel.xyz.z;
					}
				}

				if(render_amount > 0.0f)
				{
					//Handle sounds and stuff
					if(shipp->thrusters_start[i] <= 0)
					{
						shipp->thrusters_start[i] = timestamp();
						if(mtp->start_snd >= 0)
							snd_play_3d( &Snds[mtp->start_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
					}

					//Only assign looping sound if
					//it is specified
					//it isn't assigned already
					//start sound doesn't exist or has finished
					if(mtp->loop_snd >= 0
						&& shipp->thrusters_sounds[i] < 0
						&& (mtp->start_snd < 0 || (snd_get_duration(mtp->start_snd) < timestamp() - shipp->thrusters_start[i])) 
						)
					{
						shipp->thrusters_sounds[i] = obj_snd_assign(OBJ_INDEX(obj), mtp->loop_snd, &mtp->pos, 1);
					}

					//Draw graphics
					//Skip invalid ones
					if(mtp->tex_id >= 0)
					{
						float rad = mtp->radius;
						if(rad <= 0.0f)
							rad = 1.0f;

						float len = mtp->length;
						if(len == 0.0f)
							len = rad;

						vec3d start, tmpend, end;
						//Start
						vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
						vm_vec_add2(&start, &obj->pos);

						//End
						if(mtp->flags & MTF_NO_SCALE)
							vm_vec_scale_add(&tmpend, &mtp->pos, &mtp->norm, len);
						else
							vm_vec_scale_add(&tmpend, &mtp->pos, &mtp->norm, len * render_amount);
						vm_vec_unrotate(&end, &tmpend, &obj->orient);
						vm_vec_add2(&end, &obj->pos);

						//Draw
						int bmap_frame = mtp->tex_id;
						if(mtp->tex_nframes > 0)
							bmap_frame += (int)(((float)(timestamp() - shipp->thrusters_start[i]) / 1000.0f) * (float)mtp->tex_fps) % mtp->tex_nframes;

						man_thruster_renderer *mtr = man_thruster_get_slot(bmap_frame);
						mtr->man_batcher.add_allocate(1);
						mtr->man_batcher.draw_beam(&start, &end, rad, 1.0f);
					}

				}
				//We've stopped firing a thruster
				else if(shipp->thrusters_start[i] > 0)
				{
					shipp->thrusters_start[i] = 0;
					if(shipp->thrusters_sounds[i] >= 0)
					{
						obj_snd_delete(OBJ_INDEX(obj), shipp->thrusters_sounds[i]);
						shipp->thrusters_sounds[i] = -1;
					}

					if(mtp->stop_snd >= 0)
					{
						//Get world pos
						vec3d start;
						vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
						vm_vec_add2(&start, &obj->pos);

						snd_play_3d( &Snds[mtp->stop_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
					}
				}
			}

			if ( ((shipp->thruster_bitmap >= 0) || (shipp->thruster_glow_bitmap >= 0)) && (!(shipp->flags & SF_DISABLED)) && (!ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE)) )
			{
				vec3d ft;

				//	Add noise to thruster geometry.
				
				/*
				float ft;

				ft = obj->phys_info.forward_thrust;
				ft *= (1.0f + frand()/5.0f - 1.0f/10.0f);
				if (ft > 1.0f)
					ft = 1.0f;
				*/

				ft.xyz.z = obj->phys_info.forward_thrust;
				ft.xyz.x = obj->phys_info.side_thrust;
				ft.xyz.y = obj->phys_info.vert_thrust;
				ft.xyz.z *= (1.0f + frand()/5.0f - 1.0f/10.0f);
				ft.xyz.y *= (1.0f + frand()/5.0f - 1.0f/10.0f);
				ft.xyz.x *= (1.0f + frand()/5.0f - 1.0f/10.0f);
				if (ft.xyz.z > 1.0f)
					ft.xyz.z = 1.0f;
				if (ft.xyz.x > 1.0f)
					ft.xyz.x = 1.0f;
				if (ft.xyz.y > 1.0f)
					ft.xyz.y = 1.0f;
				if (ft.xyz.z < -1.0f)
					ft.xyz.z = -1.0f;
				if (ft.xyz.x < -1.0f)
					ft.xyz.x = -1.0f;
				if (ft.xyz.y < -1.0f)
					ft.xyz.y = -1.0f;

				//model_set_thrust( shipp->modelnum, &ft, shipp->thruster_bitmap, shipp->thruster_glow_bitmap, shipp->thruster_glow_noise );

				// Bobboau's extra thruster stuff
				{
					bool use_AB = (obj->phys_info.flags & PF_AFTERBURNER_ON) || (obj->phys_info.flags & PF_BOOSTER_ON);

					bobboau_extra_mst_info mst;

					mst.secondary_glow_bitmap = shipp->thruster_secondary_glow_bitmap;
					mst.tertiary_glow_bitmap = shipp->thruster_tertiary_glow_bitmap;
					mst.rovel = &Objects[shipp->objnum].phys_info.rotvel;

					mst.trf1 = sip->thruster01_glow_rad_factor;
					mst.trf2 = sip->thruster02_glow_rad_factor;
					mst.trf3 = sip->thruster03_glow_rad_factor;
					mst.tlf = sip->thruster02_glow_len_factor;

					model_set_thrust(sip->model_num, &ft, shipp->thruster_bitmap, shipp->thruster_glow_bitmap, shipp->thruster_glow_noise, use_AB, &mst);
				}

				render_flags |= MR_SHOW_THRUSTERS;
			}

			// fill the model flash lighting values in
			shipfx_flash_light_model( obj, sip->model_num );

			
			// If the ship is going "through" the warp effect, then
			// set up the model renderer to only draw the polygons in front
			// of the warp in effect
			int clip_started = 0;

			// Warp_shipp points to the ship that is going through a
			// warp... either this ship or the ship it is docked with.
			if ( warp_shipp != NULL )
			{
				if (Ship_info[warp_shipp->ship_info_index].warpout_type == WT_DEFAULT)
				{
					clip_started = 1;
					g3_start_user_clip_plane( &warp_shipp->warp_effect_pos, &warp_shipp->warp_effect_fvec );

					// Turn off model caching while going thru warp effect.
				//	render_flags |= MR_ALWAYS_REDRAW;
				}
			}

			// maybe set squad logo bitmap
			model_set_insignia_bitmap(-1);

			if(Game_mode & GM_MULTIPLAYER){
				// if its any player's object
				int np_index = multi_find_player_by_object( obj );
				if((np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].m_player != NULL)){
					model_set_insignia_bitmap(Net_players[np_index].m_player->insignia_texture);
				}
			}
			// in single player, we want to render model insignias on all ships in alpha beta and gamma
			// Goober5000 - and also on wings that have their logos set
			else {
				// if its an object in my squadron
				if(ship_in_my_squadron(shipp)) {
					model_set_insignia_bitmap(Player->insignia_texture);
				}

				// maybe it has a wing squad logo - Goober5000
				if (shipp->wingnum >= 0)
				{
					// don't override the player's wing
					if (shipp->wingnum != Player_ship->wingnum)
					{
						// if we have a logo texture
						if (Wings[shipp->wingnum].wing_insignia_texture >= 0)
						{
							model_set_insignia_bitmap(Wings[shipp->wingnum].wing_insignia_texture);
						}
					}
				}
			}

			// maybe disable lighting
			// if((The_mission.flags & MISSION_FLAG_FULLNEB) && (neb2_get_fog_intensity(obj) > 0.33f) && (si->flags & SIF_SMALL_SHIP)){
				// render_flags |= MR_NO_LIGHTING;
			// }

			// nebula		
			if(The_mission.flags & MISSION_FLAG_FULLNEB){		
				extern void model_set_fog_level(float l);
				model_set_fog_level(neb2_get_fog_intensity(obj));
			}

			if (shipp->cloak_stage>0)
			{
				//cloaking
				if (shipp->cloak_stage==2)
				{
					model_setup_cloak(&shipp->current_translation,1,shipp->cloak_alpha);
					render_flags |= MR_FORCE_TEXTURE | MR_NO_LIGHTING;
				}
				else
				{
					model_setup_cloak(&shipp->current_translation,0,255);
				}
			}


			//draw weapon models
			if (sip->draw_models) {
				int i,k;
				ship_weapon *swp = &shipp->weapons;
				g3_start_instance_matrix(&obj->pos, &obj->orient, true);
			
				int save_flags = render_flags;
		
				render_flags &= ~MR_SHOW_THRUSTERS;

		//primary weapons
				for (i = 0; i < swp->num_primary_banks; i++) {
					if (Weapon_info[swp->primary_bank_weapons[i]].external_model_num == -1 || !sip->draw_primary_models[i])
						continue;

					w_bank *bank = &model_get(sip->model_num)->gun_banks[i];
					for(k = 0; k < bank->num_slots; k++) {	
						polymodel* pm = model_get(Weapon_info[swp->primary_bank_weapons[i]].external_model_num);
						pm->gun_submodel_rotation = shipp->primary_rotate_ang[i];
						model_render(Weapon_info[swp->primary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k], render_flags);
						pm->gun_submodel_rotation = 0.0f;
					}
				}

		//secondary weapons
		
				for (i = 0; i < swp->num_secondary_banks; i++) {
					if (Weapon_info[swp->secondary_bank_weapons[i]].external_model_num == -1 || !sip->draw_secondary_models[i])
						continue;

					w_bank *bank = &model_get(sip->model_num)->missile_banks[i];
					for(k = 0; k < bank->num_slots; k++) {
						vec3d secondary_weapon_pos = bank->pnt[k];
					//	vm_vec_add(&secondary_weapon_pos, &obj->pos, &bank->pnt[k]);
		
					//	if(shipp->secondary_point_reload_pct[i][k] != 1.0)
					//		vm_vec_scale_add2(&secondary_weapon_pos, &obj->orient.vec.fvec, -(1.0f-shipp->secondary_point_reload_pct[i][k]) * model_get(Weapon_info[swp->secondary_bank_weapons[i]].model_num)->rad);
						if(shipp->secondary_point_reload_pct[i][k] <= 0.0)
							continue;
		
						vec3d dir = ZERO_VECTOR;
						dir.xyz.z = 1.0;
		
						bool clipping = false;
		
					/*	extern int G3_user_clip;
		
						if(!G3_user_clip){
							vec3d clip_pnt;
							vm_vec_rotate(&clip_pnt, &bank->pnt[k], &obj->orient);
							vm_vec_add2(&clip_pnt, &obj->pos);
							g3_start_user_clip_plane(&clip_pnt,&obj->orient.vec.fvec);
							clipping = true;
						}
					*/

						vm_vec_scale_add2(&secondary_weapon_pos, &dir, -(1.0f-shipp->secondary_point_reload_pct[i][k]) * model_get(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num)->rad);

						model_render(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &secondary_weapon_pos, render_flags);
						if(clipping)
							g3_stop_user_clip_plane();
					}
				}
				g3_done_instance(true);
				render_flags = save_flags;
			}

			// small ships
			if ((The_mission.flags & MISSION_FLAG_FULLNEB) && (sip->flags & SIF_SMALL_SHIP)) {			
				// force detail levels
 				float fog_val = neb2_get_fog_intensity(obj);
				if(fog_val >= 0.6f){
					model_set_detail_level(2);
					model_render( sip->model_num, &obj->orient, &obj->pos, render_flags | MR_LOCK_DETAIL, OBJ_INDEX(obj), -1, shipp->replacement_textures );
				} else {
					model_render( sip->model_num, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
				}
			} else {
				model_render( sip->model_num, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
			}

	//		decal_render_all(obj);
	//		mprintf(("out of the decal stuff\n"));

			// always turn off fog after rendering a ship
			gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	/*
			if(shipp->flare_life>0){
				g3_start_instance_matrix(&obj->pos, &obj->orient, true);
				float flalpha = MIN(shipp->flare_life, 1.0f);
				float splode_factor = MIN(pow(shipp->flare_life,6), 1.0f) * 1.25;
				for( int i = 0; i<shipp->n_debris_flare; i++){
					gr_set_bitmap( shipp->flare_bm, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );

					shipp->debris_flare[i].render(model_get(shipp->modelnum)->rad*splode_factor,flalpha, shipp->flare_life);
				}
					g3_done_instance(true);
			}
	*/
			light_set_shadow(0);

			#ifndef NDEBUG
			if (Show_shield_mesh)
				ship_draw_shield( obj);		//	Render the shield.
			#endif

			if ( clip_started )	{
				g3_stop_user_clip_plane();
			}
		} 

	/*	if (Mc.shield_hit_tri != -1) {
			//render_shield_explosion(model_num, orient, pos, &Hit_point, Hit_tri);
			Mc.shield_hit_tri = -1;
		}
	*/

		ship_model_stop(obj);

		if (shipp->shield_hits) {
			create_shield_explosion_all(obj);
			shipp->shield_hits = 0;
		}

	#ifndef NDEBUG
		if (Ai_render_debug_flag || Show_paths) {
			if ( shipp->ai_index != -1 ){
				render_path_points(obj);
			}

			render_dock_bays(obj);
		}
	#endif
		
	#ifndef NDEBUG
		if(Show_tnorms){
			ship_subsys *systemp;
			vec3d tpos, tnorm, temp;
			vec3d v1, v2;
			vertex l1, l2;

			gr_set_color(0, 0, 255);
			systemp = GET_FIRST( &shipp->subsys_list );		
			while ( systemp != END_OF_LIST(&shipp->subsys_list) ) {
				ship_get_global_turret_gun_info(obj, systemp, &tpos, &tnorm, 1, &temp);
				
				v1 = tpos;
				vm_vec_scale_add(&v2, &v1, &tnorm, 20.0f);

				g3_rotate_vertex(&l1, &v1);
				g3_rotate_vertex(&l2, &v2);

				g3_draw_sphere(&l1, 2.0f);
				g3_draw_line(&l1, &l2);

				systemp = GET_NEXT(systemp);
			}
		}
	#endif
	//	mprintf(("ship rendered\n"));

		if (!Cmdline_nohtl && reset_proj_when_done) {
			gr_end_view_matrix();
			gr_end_proj_matrix();

			gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}
	}

	//WMC - Draw animated warp effect (ie BSG thingy)
	//WMC - based on Bobb's secondary thruster stuff
	//which was in turn based on the beam code.
	//I'm gonna need some serious acid to neutralize this base.
	if(shipp->warp_anim >= 0 && shipp->final_warp_time > timestamp())
	{
		fx_batcher.allocate(1);

		float rad = 0.0f;
		ship_info *sip = &Ship_info[shipp->ship_info_index];

		if(shipp->flags & SF_DEPART_WARP)
			rad = sip->warpout_radius;
		else
			rad = sip->warpin_radius;
		if(rad <= 0.0f)
		{
			polymodel *pm = model_get(sip->model_num);
			rad = pm->rad;
		}

		//Do warpout geometry
		vec3d start, end;
		vm_vec_scale_add(&start, &obj->pos, &obj->orient.vec.fvec, rad);
		vm_vec_scale_add(&end, &obj->pos, &obj->orient.vec.fvec, -rad);
		fx_batcher.draw_beam(&start, &end, rad*2.0f, 1.0f);

		//Figure out which frame we're on
		int frame = fl2i((float)((float)(timestamp() - (float)shipp->start_warp_time) / (float)(shipp->final_warp_time - (float)shipp->start_warp_time)) * (float)shipp->warp_anim_nframes);

		//Set the correct frame
		gr_set_bitmap(shipp->warp_anim + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
		
		// turn off zbuffering	
		int saved_zbuffer_mode = gr_zbuffer_get();
		gr_zbuffer_set(GR_ZBUFF_NONE);	

		//Render the warpout effect
		fx_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);

		// restore zbuffer mode
		gr_zbuffer_set(saved_zbuffer_mode);
	}
}

void ship_subsystems_delete(ship *shipp)
{
	if ( NOT_EMPTY(&shipp->subsys_list) )
	{
		ship_subsys *systemp, *temp;

		systemp = GET_FIRST( &shipp->subsys_list );
		while ( systemp != END_OF_LIST(&shipp->subsys_list) ) {
			temp = GET_NEXT( systemp );								// use temporary since pointers will get screwed with next operation
			list_remove( shipp->subsys_list, systemp );			// remove the element
			list_append( &ship_subsys_free_list, systemp );		// and place back onto free list
			systemp = temp;												// use the temp variable to move right along
		}
	}
}

void ship_clear_decals(ship	*shipp)
{
	clear_decals(&shipp->ship_decal_system);

	ship_subsys *sys = GET_FIRST(&shipp->subsys_list);;
	while(sys != END_OF_LIST(&shipp->subsys_list)){
		model_subsystem* psub = sys->system_info;
		if(!psub){
			sys = GET_NEXT(sys);
			continue;
		}
		clear_decals(&psub->model_decal_system);
		sys = GET_NEXT(sys);
	}
}


void ship_delete( object * obj )
{
	ship	*shipp;
	int	num, objnum;

	num = obj->instance;
	Assert( num >= 0);

	objnum = OBJ_INDEX(obj);
	Assert( Ships[num].objnum == objnum );

	shipp = &Ships[num];

	if (shipp->ai_index != -1){
		ai_free_slot(shipp->ai_index);
	}	

	// free up the list of subsystems of this ship.  walk through list and move remaining subsystems
	// on ship back to the free list for other ships to use.
	ship_subsystems_delete(&Ships[num]);

	shipp->objnum = -1;
	// mwa 11/24/97 num_ships--;

	if (shipp->shield_integrity != NULL) {
		vm_free(shipp->shield_integrity);
		shipp->shield_integrity = NULL;
	}

	if (shipp->replacement_textures != NULL) {
		vm_free(shipp->replacement_textures);
		shipp->replacement_textures = NULL;
	}

	// glow point banks
	shipp->glow_point_bank_active.clear();

	if ( shipp->ship_list_index != -1 ) {
		ship_obj_list_remove(shipp->ship_list_index);
		shipp->ship_list_index = -1;
	}

	free_sexp2(shipp->arrival_cue);
	free_sexp2(shipp->departure_cue);

	// call the contrail system
	ct_ship_delete(shipp);

	// remove textures from memory if we are done with them - taylor
//	ship_page_out_textures(shipp->ship_info_index);
	
	ship_clear_decals(shipp);
}

// function used by ship_destroyed and ship_departed which is called if the ship
// is in a wing.  This function updates the ship_index list (i.e. removes its
// entry in the list), and packs the array accordingly.
void ship_wing_cleanup( int shipnum, wing *wingp )
{
	int i, index = -1, team = Ships[shipnum].team;

	// find this ship's position within its wing
	for (i = 0; i < wingp->current_count; i++)
	{
		if (wingp->ship_index[i] == shipnum)
		{
			index = i;
			break;
		}
	}

	// Assert(index != -1);
	// this can happen in multiplayer (dogfight, ingame join specifically)
	if (index == -1)
		return;


	// compress the ship_index array and mark the last entry with a -1
	for (i = index; i < wingp->current_count - 1; i++)
		wingp->ship_index[i] = wingp->ship_index[i+1];

	wingp->current_count--;
	Assert ( wingp->current_count >= 0 );
	wingp->ship_index[wingp->current_count] = -1;


	// if the current count is 0, check to see if the wing departed or was destroyed.
	if (wingp->current_count == 0)
	{
		// if this wing was ordered to depart by the player, set the current_wave equal to the total
		// waves so we can mark the wing as gone and no other ships arrive
		// Goober5000 - also if it's departing... this is sort of, but not exactly, what :V: did;
		// but it seems to be consistent with how it should behave
		if (wingp->flags & (WF_WING_DEPARTING | WF_DEPARTURE_ORDERED))
			wingp->current_wave = wingp->num_waves;

		// Goober5000 - some changes for clarity and closing holes
		// make sure to flag the wing as gone if all of its member ships are gone and no more can arrive
		if ((wingp->current_wave == wingp->num_waves) && (wingp->total_destroyed + wingp->total_departed == wingp->total_arrived_count))
		{
			// mark the wing as gone
			wingp->flags |= WF_WING_GONE;
			wingp->time_gone = Missiontime;

			// if all ships were destroyed, log it as destroyed
			if (wingp->total_destroyed == wingp->total_arrived_count)
			{
				// first, be sure to mark a wing destroyed event if all members of wing were destroyed and on
				// the last wave.  This circumvents a problem where the wing could be marked as departed and
				// destroyed if the last ships were destroyed after the wing's departure cue became true.
				mission_log_add_entry(LOG_WING_DESTROYED, wingp->name, NULL, team);
			}
			// if some ships escaped, log it as departed
			else
			{
				// if the wing wasn't destroyed, and it is departing, then mark it as departed -- in this
				// case, there had better be ships in this wing with departure entries in the log file.  The
				// logfile code checks for this case.  
				mission_log_add_entry(LOG_WING_DEPARTED, wingp->name, NULL, team);

#ifndef NDEBUG
				// apparently, there have been reports of ships still present in the mission when this log
				// entry if written.  Do a sanity check here to find out for sure.
				for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
				{
					// skip the player -- stupid special case.
					if (&Objects[so->objnum] == Player_obj)
						continue;
	
					if ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
						continue;
	
					if ((Ships[Objects[so->objnum].instance].wingnum == WING_INDEX(wingp)) && !(Ships[Objects[so->objnum].instance].flags & (SF_DEPARTING|SF_DYING)) && !(Ships[Objects[so->objnum].instance].flags2 & (SF2_VANISHED)))
					{
						// TODO: I think this Int3() is triggered when a wing whose ships are all docked to ships of another
						// wing departs.  It can be reliably seen in TVWP chapter 1 mission 7, when Torino and Iota wing depart.
						// Not sure how to fix this. -- Goober5000
						Int3();
					}
				}
#endif
			}
		}
	}
}

// function to do management, like log entries and wing cleanup after a ship has been destroyed

void ship_destroyed( int num )
{
	ship		*shipp;
	object	*objp;

	shipp = &Ships[num];
	objp = &Objects[shipp->objnum];

	// add the information to the exited ship list
	ship_add_exited_ship( shipp, SEF_DESTROYED );

	// determine if we need to count this ship as a kill in counting number of kills per ship type
	// look at the ignore flag for the ship (if not in a wing), or the ignore flag for the wing
	// (if the ship is in a wing), and add to the kill count if the flags are not set
	if ( !(shipp->flags & SF_IGNORE_COUNT) ||  ((shipp->wingnum != -1) && !(Wings[shipp->wingnum].flags & WF_IGNORE_COUNT)) )
		ship_add_ship_type_kill_count( shipp->ship_info_index );

	// if ship belongs to a wing -- increment the total number of ships in the wing destroyed
	if ( shipp->wingnum != -1 ) {
		wing *wingp;

		wingp = &Wings[shipp->wingnum];
		wingp->total_destroyed++;
		ship_wing_cleanup( num, wingp );
	}

	//	Note, this call to ai_ship_destroy must come after ship_wing_cleanup for guarded wings to
	//	properly note the destruction of a ship in their wing.
	if ( shipp->ai_index != -1 ) {
		ai_ship_destroy(num, SEF_DESTROYED);		//	Do AI stuff for destruction of ship.
	}

	nprintf(("Alan","SHIP DESTROYED: %s\n", shipp->ship_name));

	if ( (shipp->wing_status_wing_index >= 0) && (shipp->wing_status_wing_pos >= 0) ) {
		nprintf(("Alan","STATUS UPDATED: %s\n", shipp->ship_name));
		hud_set_wingman_status_dead(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
	}

	// let the event music system know an enemy was destoyed (important for deciding when to transition from battle to normal music)
	if (Player_ship != NULL) {
		if (iff_x_attacks_y(Player_ship->team, shipp->team)) {
			event_music_hostile_ship_destroyed();
		}
	}

	ship_clear_decals(shipp);
}

void ship_vanished(object *objp)
{
	// Goober5000 - moved here from sexp_ship_vanish()
	objp->flags |= OF_SHOULD_BE_DEAD;

	// Goober5000
	if (objp->type == OBJ_SHIP)
	{
		ship *sp = &Ships[objp->instance];
		sp->flags2 |= SF2_VANISHED;	//WMC - to fix ship_wing_cleanup

		// demo recording
		if(Game_mode & GM_DEMO_RECORD){
			demo_POST_departed(objp->signature, sp->flags);
		}

		// add the information to the exited ship list
		ship_add_exited_ship( sp, SEF_DEPARTED );

		// update wingman status gauge
		if ( (sp->wing_status_wing_index >= 0) && (sp->wing_status_wing_pos >= 0) ) {
			hud_set_wingman_status_departed(sp->wing_status_wing_index, sp->wing_status_wing_pos);
		}

		// if ship belongs to a wing -- increment the total number of ships in the wing vanished
		if ( sp->wingnum != -1 ) {
			wing *wingp;

			wingp = &Wings[sp->wingnum];
			// don't increment as a destroyed ship since that would make it logged
			ship_wing_cleanup( objp->instance, wingp );
		}

		ai_ship_destroy(objp->instance, SEF_DEPARTED);		// should still do AI cleanup after ship has departed
		ship_clear_decals(sp);
	}
}

void ship_departed( int num )
{
	ship *sp;

	sp = &Ships[num];

	// demo recording
	if(Game_mode & GM_DEMO_RECORD) {
		demo_POST_departed(Objects[Ships[num].objnum].signature, Ships[num].flags);
	}

	// add the information to the exited ship list
	ship_add_exited_ship( sp, SEF_DEPARTED );

	// update wingman status gauge
	if ( (sp->wing_status_wing_index >= 0) && (sp->wing_status_wing_pos >= 0) ) {
		hud_set_wingman_status_departed(sp->wing_status_wing_index, sp->wing_status_wing_pos);
	}

	// see if this ship departed within the radius of a jump node -- if so, put the node name into
	// the secondary mission log field
	jump_node *jnp = jumpnode_get_which_in(&Objects[sp->objnum]);
	if (jnp)
		mission_log_add_entry(LOG_SHIP_DEPARTED, sp->ship_name, jnp->get_name_ptr(), sp->wingnum);
	else
		mission_log_add_entry(LOG_SHIP_DEPARTED, sp->ship_name, NULL, sp->wingnum);

	ai_ship_destroy(num, SEF_DEPARTED);		// should still do AI cleanup after ship has departed

	// don't bother doing this for demo playback - we don't keep track of wing info
	if(!(Game_mode & GM_DEMO_PLAYBACK)){
		if ( sp->wingnum != -1 ) {
			wing *wingp;

			wingp = &Wings[sp->wingnum];
			wingp->total_departed++;
			ship_wing_cleanup( num, wingp );
		}
	}
	ship_clear_decals(sp);
}

// --------------------------------------------------------------------------------------------------------------------
// ship_explode_area_calc_damage
// 
// input			pos1			=>		ship explosion position
//					pos2			=>		other ship position
//					inner_rad	=>		distance from ship center for which full damage is applied
//					outer_rad	=>		distance from ship center for which no damage is applied
//					max_damage	=>		maximum damage applied
//					max_blast	=>		maximum impulse applied from blast
// 
// calculates the blast and damage applied to a ship from another ship blowing up.
//
int ship_explode_area_calc_damage( vec3d *pos1, vec3d *pos2, float inner_rad, float outer_rad, float max_damage, float max_blast, float *damage, float *blast )
{
	float dist;

	dist = vm_vec_dist_quick( pos1, pos2 );

	// check outside outer radius
	if ( dist > outer_rad )
		return -1;

	if ( dist < inner_rad ) {
	// check insider inner radius
		*damage = max_damage;
		*blast = max_blast;
	} else {
	// between inner and outer
		float fraction = 1.0f - (dist - inner_rad) / (outer_rad - inner_rad);
		*damage  = fraction * max_damage;
		*blast   = fraction * max_blast;
	}

	return 1;
}

// --------------------------------------------------------------------------------------------------------------------
// ship_blow_up_area_apply_blast
// this function applies damage to ship close to others when a ship dies and blows up
//
//		inputs:	objp			=>		ship object pointers
//					pos			=>		position of the ship when it finally blows up
//					inner_rad	=>		distance from ship center for which full damage is applied
//					outer_rad	=>		distance from ship center for which no damage is applied
//					damage		=>		maximum damage applied
//					blast			=>		maximum impulse applied from blast

void ship_blow_up_area_apply_blast( object *exp_objp)
{
	ship *shipp;
	ship_info *sip;
	float	inner_rad, outer_rad, max_damage, max_blast, shockwave_speed;
	shockwave_create_info sci;

	//	No area explosion in training missions.
	if (The_mission.game_type & MISSION_TYPE_TRAINING){
		return;
	}

	Assert( exp_objp != NULL );
	Assert( exp_objp->type == OBJ_SHIP );
	Assert( exp_objp->instance >= 0 );

	shipp = &Ships[exp_objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	Assert( (shipp != NULL) && (sip != NULL) );


	if ((exp_objp->hull_strength <= KAMIKAZE_HULL_ON_DEATH) && (Ai_info[Ships[exp_objp->instance].ai_index].ai_flags & AIF_KAMIKAZE) && (shipp->special_exp_index < 0)) {
		float override = Ai_info[shipp->ai_index].kamikaze_damage;

		inner_rad = exp_objp->radius*2.0f;
		outer_rad = exp_objp->radius*4.0f; // + (override * 0.3f);
		max_damage = override;
		max_blast = override * 5.0f;
		shockwave_speed = 100.0f;
	} else {
		if (shipp->special_exp_index != -1) {
			int start = shipp->special_exp_index;
			int propagates;
			inner_rad = (float) atoi(Sexp_variables[start+INNER_RAD].text);
			outer_rad = (float) atoi(Sexp_variables[start+OUTER_RAD].text);
			max_damage = (float) atoi(Sexp_variables[start+DAMAGE].text);
			max_blast = (float) atoi(Sexp_variables[start+BLAST].text);
			propagates = atoi(Sexp_variables[start+PROPAGATE].text);
			if (propagates) {
				shockwave_speed = (float) atoi(Sexp_variables[start+SHOCK_SPEED].text);
			} else {
				shockwave_speed = 0.0f;
			}
		} else {
			inner_rad = sip->shockwave.inner_rad;
			outer_rad = sip->shockwave.outer_rad;
			max_damage = sip->shockwave.damage;
			max_blast  = sip->shockwave.blast;
			shockwave_speed = sip->shockwave.speed;
		}
	}

	// nprintf(("AI", "Frame %i: Area effect blast from ship %s\n", Framecount, Ships[exp_objp->instance].ship_name));

	// account for ships that give no damage when they blow up.
	if ( (max_damage < 0.1f) && (max_blast < 0.1f) ){
		return;
	}

	if ( shockwave_speed > 0 ) {
		strcpy(sci.name, sip->shockwave.name);
		strcpy(sci.pof_name, sip->shockwave.pof_name);
		sci.inner_rad = inner_rad;
		sci.outer_rad = outer_rad;
		sci.blast = max_blast;
		sci.damage = max_damage;
		sci.speed = shockwave_speed;
		sci.rot_angles.p = frand_range(0.0f, 1.99f*PI);
		sci.rot_angles.b = frand_range(0.0f, 1.99f*PI);
		sci.rot_angles.h = frand_range(0.0f, 1.99f*PI);
		shipfx_do_shockwave_stuff(shipp, &sci);
		// shockwave_create(Ships[exp_objp->instance].objnum, &exp_objp->pos, shockwave_speed, inner_rad, outer_rad, max_damage, max_blast, SW_SHIP_DEATH);
	} else {
		object *objp;
		float blast = 0.0f;
		float damage = 0.0f;
		for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) ) {
				continue;
			}
		
			if ( objp == exp_objp ){
				continue;
			}

			// don't blast navbuoys
			if ( objp->type == OBJ_SHIP ) {
				if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
					continue;
				}
			}

			if ( ship_explode_area_calc_damage( &exp_objp->pos, &objp->pos, inner_rad, outer_rad, max_damage, max_blast, &damage, &blast ) == -1 ){
				continue;
			}

			switch ( objp->type ) {
			case OBJ_SHIP:
				ship_apply_global_damage( objp, exp_objp, &exp_objp->pos, damage );
				vec3d force, vec_ship_to_impact;
				vm_vec_sub( &vec_ship_to_impact, &objp->pos, &exp_objp->pos );
				vm_vec_copy_normalize( &force, &vec_ship_to_impact );
				vm_vec_scale( &force, blast );
				ship_apply_whack( &force, &vec_ship_to_impact, objp );
				break;
			case OBJ_ASTEROID:
				asteroid_hit(objp, NULL, NULL, damage);
				break;
			default:
				Int3();
				break;
			}
		}	// end for
	}
}

// Goober5000 - fyi, this function is only ever called once for any ship that dies
// This function relies on the "dead dock" list, which replaces the dock_objnum_when_dead
// used in retail.
void do_dying_undock_physics(object *dying_objp, ship *dying_shipp) 
{
	// this function should only be called for an object that was docked...
	// no harm in calling it if it wasn't, but we want to enforce this
	Assert(object_is_dead_docked(dying_objp));

	object *docked_objp;

	float damage;
	float impulse_mag;

	vec3d impulse_norm, impulse_vec, pos;

	// damage applied to each docked object
	damage = 0.2f * dying_shipp->ship_max_hull_strength;

	// Goober5000 - as with ai_deathroll_start, we can't simply iterate through the dock list while we're
	// unlinking things.  So just repeatedly unlink the first object.
	while (object_is_dead_docked(dying_objp))
	{
		docked_objp = dock_get_first_dead_docked_object(dying_objp);

		// only consider the mass of these two objects, not the whole assembly
		// (this is inaccurate, but the alternative is a huge mess of extra code for a very small gain in realism)
		float docked_mass = dying_objp->phys_info.mass + docked_objp->phys_info.mass;

		// damage this docked object
		ship_apply_global_damage(docked_objp, dying_objp, &dying_objp->pos, damage);

		// do physics
		vm_vec_sub(&impulse_norm, &docked_objp->pos, &dying_objp->pos);
		vm_vec_normalize(&impulse_norm);
		// set for relative separation velocity of ~30
		impulse_mag = 50.f * docked_objp->phys_info.mass * dying_objp->phys_info.mass / docked_mass;
		vm_vec_copy_scale(&impulse_vec, &impulse_norm, impulse_mag);
		vm_vec_rand_vec_quick(&pos);
		vm_vec_scale(&pos, docked_objp->radius);
		// apply whack to docked object
		physics_apply_whack(&impulse_vec, &pos, &docked_objp->phys_info, &docked_objp->orient, docked_objp->phys_info.mass);
		// enhance rotation of the docked object
		vm_vec_scale(&docked_objp->phys_info.rotvel, 2.0f);

		// apply whack to dying object
		vm_vec_negate(&impulse_vec);
		vm_vec_rand_vec_quick(&pos);
		vm_vec_scale(&pos, dying_objp->radius);
		physics_apply_whack(&impulse_vec, &pos, &dying_objp->phys_info, &dying_objp->orient, dying_objp->phys_info.mass);

		// unlink the two objects, since dying_objp has blown up
		dock_dead_undock_objects(dying_objp, docked_objp);
	}
}

//	Do the stuff we do in a frame for a ship that's in its death throes.
void ship_dying_frame(object *objp, int ship_num)
{
	ship *shipp = &Ships[ship_num];

	if ( shipp->flags & SF_DYING )	{
		ship_info *sip = &Ship_info[shipp->ship_info_index];
		int knossos_ship = (sip->flags & SIF_KNOSSOS_DEVICE);

		// bash hull value toward 0 (from self destruct)
		if (objp->hull_strength > 0) {
			int time_left = timestamp_until(shipp->final_death_time);
			float hits_left = objp->hull_strength;

			objp->hull_strength -= hits_left * (1000.0f * flFrametime) / time_left;
		}

		// special case of VAPORIZE
		if (shipp->flags & SF_VAPORIZE) {
			// Assert(sip->flags & SIF_SMALL_SHIP);
			if (timestamp_elapsed(shipp->final_death_time)) {

				// play death sound
				snd_play_3d( &Snds[SND_VAPORIZED], &objp->pos, &View_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );

				// do joystick effect
				if (objp == Player_obj) {
					joy_ff_explode();
				}

				// if dying ship is docked, do damage to docked and physics
				if (object_is_dead_docked(objp))  {
					do_dying_undock_physics(objp, shipp);
				}			

				// do all accounting for respawning client and server side here.
				if (objp == Player_obj) {				
					gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
				}

				// mark object as dead
				objp->flags |= OF_SHOULD_BE_DEAD;

				// Don't blow up model.  Only use debris shards.
				// call ship function to clean up after the ship is destroyed.
				ship_destroyed(ship_num);
				return;
			} else {
				return;
			}
		}

//		if(sp->flare_life>=0 && sp->flare_life<1){
//		}

		// bash the desired rotvel
		objp->phys_info.desired_rotvel = shipp->deathroll_rotvel;

		// Do fireballs for Big ship with propagating explostion, but not Kamikaze
		if (!(Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) && ship_get_exp_propagates(shipp)) {
			if ( timestamp_elapsed(shipp->next_fireball))	{
				vec3d outpnt, pnt1, pnt2;
				polymodel *pm = model_get(sip->model_num);

				// Gets two random points on the surface of a submodel
				submodel_get_two_random_points(pm->id, pm->detail[0], &pnt1, &pnt2 );

				//	vm_vec_avg( &tmp, &pnt1, &pnt2 ); [KNOSSOS get random in plane 1/1.414 in rad
				model_find_world_point(&outpnt, &pnt1, sip->model_num, pm->detail[0], &objp->orient, &objp->pos );

				float rad = objp->radius*0.1f;
				int fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				fireball_create( &outpnt, fireball_type, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
				// start the next fireball up in the next 50 - 200 ms (2-3 per frame)
				shipp->next_fireball = timestamp_rand(333,500);

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, shipp->sub_expl_sound_handle);
			}
		}

		// create little fireballs for knossos as it dies
		if (knossos_ship) {
			if ( timestamp_elapsed(shipp->next_fireball)) {
				vec3d rand_vec, outpnt; // [0-.7 rad] in plane
				vm_vec_rand_vec_quick(&rand_vec);
				float scale = -vm_vec_dotprod(&objp->orient.vec.fvec, &rand_vec) * (0.9f + 0.2f * frand());
				vm_vec_scale_add2(&rand_vec, &objp->orient.vec.fvec, scale);
				vm_vec_normalize_quick(&rand_vec);
				scale = objp->radius * frand() * 0.717f;
				vm_vec_scale(&rand_vec, scale);
				vm_vec_add(&outpnt, &objp->pos, &rand_vec);

				float rad = objp->radius*0.2f;
				int fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				fireball_create( &outpnt, fireball_type, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
				// start the next fireball up in the next 50 - 200 ms (2-3 per frame)
				shipp->next_fireball = timestamp_rand(333,500);

				// emit particles
				particle_emitter	pe;

				pe.num_low = 15;					// Lowest number of particles to create
				pe.num_high = 30;				// Highest number of particles to create
				pe.pos = outpnt;				// Where the particles emit from
				pe.vel = objp->phys_info.vel;	// Initial velocity of all the particles
				pe.min_life = 2.0f;	// How long the particles live
				pe.max_life = 12.0f;	// How long the particles live
				pe.normal = objp->orient.vec.uvec;	// What normal the particle emit around
				pe.normal_variance = 2.0f;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree
				pe.min_vel = 50.0f;
				pe.max_vel = 350.0f;
				pe.min_rad = 30.0f;	// * objp->radius;
				pe.max_rad = 100.0f; // * objp->radius;
				pe.texture_id = particle_get_smoke2_id();
				pe.range = 50.0f;
				particle_emit( &pe );

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, shipp->sub_expl_sound_handle);
			}
		}


		//nprintf(("AI", "Ship.cpp: Frame=%i, Time = %7.3f, Ship %s will die in %7.3f seconds.\n", Framecount, f2fl(Missiontime), shipp->ship_name, (float) timestamp_until(sp->final_death_time)/1000.0f));
		int time_until_minor_explosions = timestamp_until(shipp->final_death_time);
//		if(time_until_minor_explosions < 500)
//			sp->flare_life += flFrametime;
		// Wait until just before death and set off some explosions
		// If it is less than 1/2 second until large explosion, but there is
		// at least 1/10th of a second left, then create 5 small explosions
		if ( (time_until_minor_explosions < 500) && (time_until_minor_explosions > 100) && (!shipp->pre_death_explosion_happened) ) {
			//mprintf(( "Ship almost dying!!\n" ));
			shipp->next_fireball = timestamp(-1);	// never time out again
			shipp->pre_death_explosion_happened=1;		// Mark this event as having occurred

			polymodel *pm = model_get(sip->model_num);

			// Start shockwave for ship with propagating explosion, do now for timing
			if ( ship_get_exp_propagates(shipp) ) {
				ship_blow_up_area_apply_blast( objp );
			}

			for (int zz=0; zz<6; zz++ ) {
				// dont make sequence of fireballs for knossos
				if (knossos_ship) {
					break;
				}
				// Find two random vertices on the model, then average them
				// and make the piece start there.
				vec3d tmp, outpnt, pnt1, pnt2;

				// Gets two random points on the surface of a submodel [KNOSSOS]
				submodel_get_two_random_points(pm->id, pm->detail[0], &pnt1, &pnt2 );

				vm_vec_avg( &tmp, &pnt1, &pnt2 );
				model_find_world_point(&outpnt, &tmp, pm->id, pm->detail[0], &objp->orient, &objp->pos );

				float rad = frand()*0.30f;
				rad += objp->radius*0.40f;
				fireball_create( &outpnt, FIREBALL_EXPLOSION_MEDIUM, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
			}
		}

		if ( timestamp_elapsed(shipp->final_death_time))	{

			shipp->death_time = shipp->final_death_time;
			

			shipp->final_death_time = timestamp(-1);	// never time out again
			//mprintf(( "Ship dying!!\n" ));
			
			// play ship explosion sound effect, pick appropriate explosion sound
			int sound_index;
			if ( sip->flags & (SIF_CAPITAL | SIF_KNOSSOS_DEVICE) ) {
				sound_index=SND_CAPSHIP_EXPLODE;
			} else {
				if ( OBJ_INDEX(objp) & 1 ) {
					sound_index=SND_SHIP_EXPLODE_1;
				} else {
					sound_index=SND_SHIP_EXPLODE_2;
				}
			}

			snd_play_3d( &Snds[sound_index], &objp->pos, &View_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );
			if (objp == Player_obj)
				joy_ff_explode();

			if ( shipp->death_roll_snd != -1 ) {
				snd_stop(shipp->death_roll_snd);
				shipp->death_roll_snd = -1;
			}

			// if dying ship is docked, do damage to docked and physics
			if (object_is_dead_docked(objp))  {
				do_dying_undock_physics(objp, shipp);
			}			

			if (!knossos_ship) {
				// play a random explosion
				particle_emitter	pe;

				pe.num_low = 50;					// Lowest number of particles to create
				pe.num_high = 100;				// Highest number of particles to create
				pe.pos = objp->pos;				// Where the particles emit from
				pe.vel = objp->phys_info.vel;	// Initial velocity of all the particles
				pe.min_life = 0.5f;				// How long the particles live
				pe.max_life = 4.0f;				// How long the particles live
				pe.normal = objp->orient.vec.uvec;	// What normal the particle emit around
				pe.normal_variance = 2.0f;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree
				pe.min_vel = 0.0f;				// How fast the slowest particle can move
				pe.max_vel = 20.0f;				// How fast the fastest particle can move
				pe.min_rad = 0.1f;				// Min radius
				pe.max_rad = 1.5f;				// Max radius
				pe.texture_id = particle_get_smoke2_id();
				particle_emit( &pe );
			}

			// If this is a large ship with a propagating explosion, set it to blow up.
			if ( ship_get_exp_propagates(shipp) )	{
				if (Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) {
					ship_blow_up_area_apply_blast( objp );
				}
				shipfx_large_blowup_init(shipp);
				// need to timeout immediately to keep physics in sync
				shipp->really_final_death_time = timestamp(0);
				polymodel *pm = model_get(sip->model_num);
				shipp->end_death_time = timestamp((int) pm->core_radius);
			} else {
				// only do big fireball if not big ship
				float big_rad;
				int fireball_objnum, fireball_type;
				float explosion_life;
				big_rad = objp->radius*1.75f;
				fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				if (knossos_ship) {
					big_rad = objp->radius * 1.2f;
					fireball_type = FIREBALL_EXPLOSION_LARGE1;
				}
				fireball_objnum = fireball_create( &objp->pos, fireball_type, OBJ_INDEX(objp), big_rad, 0, &objp->phys_info.vel );
				if ( fireball_objnum >= 0 )	{
					explosion_life = fireball_lifeleft(&Objects[fireball_objnum]);
				} else {
					explosion_life = 0.0f;
				}

				// JAS:  I put in all this code because of an item on my todo list that
				// said that the ship destroyed debris shouldn't pop in until the
				// big explosion is 30% done.  I did this on Oct24 and me & Adam 
				// thought it looked dumb since the explosion didn't move with the
				// ship, so instead of just taking this code out, since we might need
				// it in the future, I disabled it.   You can reenable it by changing
				// the commenting on the following two lines.
				shipp->end_death_time = shipp->really_final_death_time = timestamp( fl2i(explosion_life*1000.0f)/5 );	// Wait till 30% of vclip time before breaking the ship up.
				//shipp->really_final_death_time = timestamp(0);	// Make ship break apart the instant the explosion starts
			}

			shipp->flags |= SF_EXPLODED;

			if ( !(ship_get_exp_propagates(shipp)) ) {
				// apply area of effect blast damage from ship explosion
				ship_blow_up_area_apply_blast( objp );
			}
		}

		if ( timestamp_elapsed(shipp->really_final_death_time))	{

			//mprintf(( "Ship really dying!!\n" ));
			// do large_ship_split and explosion
			if ( shipp->large_ship_blowup_index >= 0 )	{
				if ( shipfx_large_blowup_do_frame(shipp, flFrametime) )	{
					// do all accounting for respawning client and server side here.
					if(objp == Player_obj) {				
						gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
					}

					objp->flags |= OF_SHOULD_BE_DEAD;									
					
					ship_destroyed(ship_num);		// call ship function to clean up after the ship is destroyed.
				}
				return;
			} 

			//fireball_create( &objp->pos, FIREBALL_SHIP_EXPLODE1, OBJ_INDEX(objp), objp->radius/2.0f );
			//mprintf(("Frame %i: Died!\n", Framecount));

			shipfx_blow_up_model(objp, sip->model_num, 0, 20, &objp->pos );

			// do all accounting for respawning client and server side here.
			if(objp == Player_obj) {				
				gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
			}

			objp->flags |= OF_SHOULD_BE_DEAD;
								
			ship_destroyed(ship_num);		// call ship function to clean up after the ship is destroyed.
			shipp->really_final_death_time = timestamp( -1 );	// Never time out again!
		}

		// If a ship is dying (and not a capital or big ship) then stutter the engine sound
		if ( timestamp_elapsed(shipp->next_engine_stutter) ) {
			if ( !(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
				shipp->flags ^= SF_ENGINES_ON;			// toggle state of engines
				shipp->next_engine_stutter = timestamp_rand(50, 250);
			}
		}
	}
}

void ship_chase_shield_energy_targets(ship *shipp, object *obj, float frametime)
{
	float delta;
	ship_info *sip;

	if (shipp->flags & SF_DYING)
		return;

	sip = &Ship_info[shipp->ship_info_index];

	delta = frametime * ETS_RECHARGE_RATE * shield_get_max_strength(obj) / 100.0f;

	//	Chase target_shields and target_weapon_energy
	if (shipp->target_shields_delta > 0.0f) {
		if (delta > shipp->target_shields_delta)
			delta = shipp->target_shields_delta;

		shield_add_strength(obj, delta);
		shipp->target_shields_delta -= delta;
	} else if (shipp->target_shields_delta < 0.0f) {
		if (delta < -shipp->target_shields_delta)
			delta = -shipp->target_shields_delta;

		shield_add_strength(obj, -delta);
		shipp->target_shields_delta += delta;
	}

	delta = frametime * ETS_RECHARGE_RATE * sip->max_weapon_reserve / 100.0f;

	if (shipp->target_weapon_energy_delta > 0.0f) {
		if (delta > shipp->target_weapon_energy_delta)
			delta = shipp->target_weapon_energy_delta;

		shipp->weapon_energy += delta;
		shipp->target_weapon_energy_delta -= delta;
	} else if (shipp->target_weapon_energy_delta < 0.0f) {
		if (delta < -shipp->target_weapon_energy_delta)
			delta = -shipp->target_weapon_energy_delta;

		shipp->weapon_energy -= delta;
		shipp->target_weapon_energy_delta += delta;
	}

}

int thruster_glow_anim_load(generic_anim *ga)
{
	int fps = 15;

	ga->first_frame = bm_load(ga->filename);
	if (ga->first_frame < 0)
	{
		Warning(LOCATION, "Couldn't load thruster glow animation '%s'", ga->filename);
		return -1;
	}
	ga->num_frames = NOISE_NUM_FRAMES;

	Assert(fps != 0);
	ga->total_time = (int) i2fl(ga->num_frames)/fps;

	return 0;
}

// loads the animations for ship's afterburners
void ship_init_thrusters()
{
	if ( Thrust_anim_inited == 1 )
		return;

	for (uint i = 0; i < Species_info.size(); i++)
	{
		species_info *species = &Species_info[i];

		// AL 29-3-98: Don't want to include Shivan thrusters in the demo build
#ifdef DEMO // N/A FS2_DEMO
		if (!stricmp(species->species_name, "Shivan"))
			continue;
#endif

		generic_anim_load(&species->thruster_info.flames.normal);
		generic_anim_load(&species->thruster_info.flames.afterburn);

		// Bobboau's extra thruster stuff
		{
			generic_bitmap_load(&species->thruster_secondary_glow_info.normal);
			generic_bitmap_load(&species->thruster_secondary_glow_info.afterburn);
			generic_bitmap_load(&species->thruster_tertiary_glow_info.normal);
			generic_bitmap_load(&species->thruster_tertiary_glow_info.afterburn);
		}

		// glows are handled a bit strangely
		thruster_glow_anim_load(&species->thruster_info.glow.normal);
		thruster_glow_anim_load(&species->thruster_info.glow.afterburn);
	}

	Thrust_anim_inited = 1;
}


// JAS - figure out which thruster bitmap will get rendered next
// time around.  ship_render needs to have shipp->thruster_bitmap set to
// a valid bitmap number, or -1 if we shouldn't render thrusters.
void ship_do_thruster_frame( ship *shipp, object *objp, float frametime )
{
	float rate;
	int framenum;
	int secondary_glow_bitmap, tertiary_glow_bitmap;
	generic_anim *flame_anim, *glow_anim;
	ship_info	*sinfo = &Ship_info[shipp->ship_info_index];
	species_info *species = &Species_info[sinfo->species];

	if (!Thrust_anim_inited)
		ship_init_thrusters();

	if (objp->phys_info.flags & PF_AFTERBURNER_ON)
	{
		flame_anim = &species->thruster_info.flames.afterburn;		// select afterburner flame
		glow_anim = &sinfo->thruster_glow_info.afterburn;			// select afterburner glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.afterburn.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.afterburn.bitmap_id;

		rate = 1.5f;		// go at 1.5x faster when afterburners on
	}
	else if (objp->phys_info.flags & PF_BOOSTER_ON)
	{
		flame_anim = &species->thruster_info.flames.afterburn;		// select afterburner flame
		glow_anim = &sinfo->thruster_glow_info.afterburn;			// select afterburner glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.afterburn.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.afterburn.bitmap_id;

		rate = 2.5f;		// go at 2.5x faster when boosters on
	}
	else
	{
		flame_anim = &species->thruster_info.flames.normal;			// select normal flame
		glow_anim = &sinfo->thruster_glow_info.normal;				// select normal glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.normal.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.normal.bitmap_id;

		// If thrust at 0, go at half as fast, full thrust; full framerate
		// so set rate from 0.5 to 1.0, depending on thrust from 0 to 1
		// rate = 0.5f + objp->phys_info.forward_thrust / 2.0f;
		rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);
	}

//	rate = 0.1f;

	Assert( frametime > 0.0f );

	if (flame_anim->first_frame >= 0) {
		shipp->thruster_frame += frametime * rate;

		// Sanity checks
		if ( shipp->thruster_frame < 0.0f )	shipp->thruster_frame = 0.0f;
		if ( shipp->thruster_frame > 100.0f ) shipp->thruster_frame = 0.0f;

		while ( shipp->thruster_frame > flame_anim->total_time )	{
			shipp->thruster_frame -= flame_anim->total_time;
		}
		framenum = fl2i( (shipp->thruster_frame*flame_anim->num_frames) / flame_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= flame_anim->num_frames ) framenum = flame_anim->num_frames-1;

//		if ( anim_index == 0 )
//			mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  flame_anim->num_frames, anim_index ));
	
		// Get the bitmap for this frame
		shipp->thruster_bitmap = flame_anim->first_frame + framenum;

//		mprintf(( "TF: %.2f\n", shipp->thruster_frame ));
	} else {
		shipp->thruster_frame = 0.0f;
		shipp->thruster_bitmap = -1;
	}

	// Do it for glow bitmaps

	if (glow_anim->first_frame >= 0) {
		shipp->thruster_glow_frame += frametime * rate;

		// Sanity checks
		if ( shipp->thruster_glow_frame < 0.0f )	shipp->thruster_glow_frame = 0.0f;
		if ( shipp->thruster_glow_frame > 100.0f ) shipp->thruster_glow_frame = 0.0f;

		while ( shipp->thruster_glow_frame > glow_anim->total_time )	{
			shipp->thruster_glow_frame -= glow_anim->total_time;
		}
		framenum = fl2i( (shipp->thruster_glow_frame*glow_anim->num_frames) / glow_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= glow_anim->num_frames ) framenum = glow_anim->num_frames-1;

//		if ( anim_index == 0 )
//			mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  glow_anim->num_frames, anim_index ));
	
		// Get the bitmap for this frame
		shipp->thruster_glow_bitmap = glow_anim->first_frame;	// + framenum;
		shipp->thruster_glow_noise = Noise[framenum];
	} else {
		shipp->thruster_glow_frame = 0.0f;
		shipp->thruster_glow_bitmap = -1;
		shipp->thruster_glow_noise = 1.0f;
	}

	// HACK add Bobboau's thruster stuff
	shipp->thruster_secondary_glow_bitmap = secondary_glow_bitmap;
	shipp->thruster_tertiary_glow_bitmap = tertiary_glow_bitmap;
}


// JAS - figure out which thruster bitmap will get rendered next
// time around.  ship_render needs to have shipp->thruster_bitmap set to
// a valid bitmap number, or -1 if we shouldn't render thrusters.
// This does basically the same thing as ship_do_thruster_frame, except it
// operates on a weapon.   This is in the ship code because it needs
// the same thruster animation info as the ship stuff, and I would
// rather extern this one function than all the thruster animation stuff.
void ship_do_weapon_thruster_frame( weapon *weaponp, object *objp, float frametime )
{
	float rate;
	int framenum;
	generic_anim *flame_anim, *glow_anim;

	if (!Thrust_anim_inited)
		ship_init_thrusters();

	species_info *species = &Species_info[weaponp->species];

	// If thrust at 0, go at half as fast, full thrust; full framerate
	// so set rate from 0.5 to 1.0, depending on thrust from 0 to 1
	// rate = 0.5f + objp->phys_info.forward_thrust / 2.0f;
	rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);

	flame_anim = &species->thruster_info.flames.normal;
	glow_anim = &species->thruster_info.glow.normal;

	Assert( frametime > 0.0f );

	if (flame_anim->first_frame >= 0) {
		weaponp->thruster_frame += frametime * rate;

		// Sanity checks
		if ( weaponp->thruster_frame < 0.0f )	weaponp->thruster_frame = 0.0f;
		if ( weaponp->thruster_frame > 100.0f ) weaponp->thruster_frame = 0.0f;

		while ( weaponp->thruster_frame > flame_anim->total_time )	{
			weaponp->thruster_frame -= flame_anim->total_time;
		}
		framenum = fl2i( (weaponp->thruster_frame*flame_anim->num_frames) / flame_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= flame_anim->num_frames ) framenum = flame_anim->num_frames-1;

//		if ( anim_index == 0 )
//			mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  flame_anim->num_frames, anim_index ));
	
		// Get the bitmap for this frame
		weaponp->thruster_bitmap = flame_anim->first_frame + framenum;

//		mprintf(( "TF: %.2f\n", weaponp->thruster_frame ));
	} else {
		weaponp->thruster_frame = 0.0f;
		weaponp->thruster_bitmap = -1;
	}

	// Do it for glow bitmaps

	if (glow_anim->first_frame >= 0) {
		weaponp->thruster_glow_frame += frametime * rate;

		// Sanity checks
		if ( weaponp->thruster_glow_frame < 0.0f )	weaponp->thruster_glow_frame = 0.0f;
		if ( weaponp->thruster_glow_frame > 100.0f ) weaponp->thruster_glow_frame = 0.0f;

		while ( weaponp->thruster_glow_frame > glow_anim->total_time )	{
			weaponp->thruster_glow_frame -= glow_anim->total_time;
		}
		framenum = fl2i( (weaponp->thruster_glow_frame*glow_anim->num_frames) / glow_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= glow_anim->num_frames ) framenum = glow_anim->num_frames-1;

//		if ( anim_index == 0 )
//			mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  glow_anim->num_frames, anim_index ));
	
		// Get the bitmap for this frame
		weaponp->thruster_glow_bitmap = glow_anim->first_frame;	// + framenum;
		weaponp->thruster_glow_noise = Noise[framenum];
	} else {
		weaponp->thruster_glow_frame = 0.0f;
		weaponp->thruster_glow_bitmap = -1;
		weaponp->thruster_glow_noise = 1.0f;
	}
}



// Repair damaged subsystems for a ship, called for each ship once per frame.
// TODO: optimize by only calling ever N seconds and keeping track of elapsed time
//
// NOTE: need to update current_hits in the sp->subsys_list element, and the sp->subsys_info[]
// element.
#define SHIP_REPAIR_SUBSYSTEM_RATE	0.01f	// percent repair per second for a subsystem
#define SUBSYS_REPAIR_THRESHOLD		0.1	// only repair subsystems that have > 10% strength
void ship_auto_repair_frame(int shipnum, float frametime)
{
	ship_subsys		*ssp;
	ship_subsys_info	*ssip;
	ship			*sp;
	ship_info		*sip;
	object			*objp;
	float			real_repair_rate;

	#ifndef NDEBUG
	if ( !Ship_auto_repair )	// only repair subsystems if Ship_auto_repair flag is set
		return;
	#endif

	Assert( shipnum >= 0 && shipnum < MAX_SHIPS);
	sp = &Ships[shipnum];
	sip = &Ship_info[sp->ship_info_index];
	objp = &Objects[sp->objnum];

	//Repair the hull...or maybe unrepair?
	if(sip->hull_repair_rate != 0.0f)
	{
		objp->hull_strength += sp->ship_max_hull_strength * sip->hull_repair_rate * frametime;

		if(objp->hull_strength > sp->ship_max_hull_strength)
		{
			objp->hull_strength = sp->ship_max_hull_strength;
		}
	}

	// only allow for the auto-repair of subsystems on small ships
	//...NOT. Check if var has been changed from def -C
	if ( !(sip->flags & SIF_SMALL_SHIP) && sip->subsys_repair_rate == -2.0f)
		return;
	
	if(sip->subsys_repair_rate == -2.0f)
		real_repair_rate = SHIP_REPAIR_SUBSYSTEM_RATE;
	else
		real_repair_rate = sip->subsys_repair_rate;

	// AL 3-14-98: only allow auto-repair if power output not zero
	if (sip->power_output <= 0)
		return;
	
	// iterate through subsystems, repair as needed based on elapsed frametime
	for ( ssp = GET_FIRST(&sp->subsys_list); ssp != END_OF_LIST(&sp->subsys_list); ssp = GET_NEXT(ssp) ) {
		Assert(ssp->system_info->type >= 0 && ssp->system_info->type < SUBSYSTEM_MAX);
		ssip = &sp->subsys_info[ssp->system_info->type];

		if ( ssp->current_hits != ssp->max_hits ) {		

			// only repair those subsystems which are not destroyed
			if ( ssp->max_hits <= 0 || ssp->current_hits <= 0 )
				continue;

			// do incremental repair on the subsystem
			ssp->current_hits += ssp->max_hits * real_repair_rate * frametime;
			ssip->current_hits += ssip->total_hits * real_repair_rate * frametime;
		
			// check for overflow of current_hits
			if ( ssp->current_hits >= ssp->max_hits ) {
				// TODO: here is hook for when a subsystem is fully repaired (eg add voice)
				ssp->current_hits = ssp->max_hits;
			}
			if ( ssip->current_hits >= ssip->total_hits ) {
				ssip->current_hits = ssip->total_hits;
			}
		}
	}	// end for
}

// this function checks to see how far the player has strayed from his starting location (should be
// single player only).  Issues a warning at some distance.  Makes mission end if he keeps flying away
// 3 strikes and you're out or too far away
#define PLAYER_MAX_DIST_WARNING			700000			// distance in KM at which player gets warning to return to battle
#define PLAYER_DISTANCE_MAX_WARNINGS	3				// maximum number of warnings player can receive before mission ends
#define PLAYER_MAX_DIST_END				750000			// distance from starting loc at which we end mission
#define PLAYER_WARN_DELTA_TIME			10000			//ms
#define PLAYER_DEATH_DELTA_TIME			5000			//ms

void ship_check_player_distance_sub(player *p, int multi_target=-1)
{
	// only check distance for ships
	if ( p->control_mode != PCM_NORMAL )	{
		// already warping out... don't bother checking anymore
		return;
	}

	float dist = vm_vec_dist_quick(&Objects[p->objnum].pos, &vmd_zero_vector);

	int give_warning_to_player = 0;
	if ( dist > PLAYER_MAX_DIST_WARNING ) {
		if (p->distance_warning_count == 0) {
			give_warning_to_player = 1;
		} else {
			if (timestamp_until(p->distance_warning_time) < 0) {
				give_warning_to_player = 1;
			}
		}
	}

	if ( give_warning_to_player ) {
		// increase warning count
		p->distance_warning_count++;
		// set timestamp unless player PLAYER_FLAGS_DIST_TO_BE_KILLED flag is set
		if ( !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
			p->distance_warning_time = timestamp(PLAYER_WARN_DELTA_TIME);
		}
		// issue up to max warnings
		if (p->distance_warning_count <= PLAYER_DISTANCE_MAX_WARNINGS) {
			message_send_builtin_to_player( MESSAGE_STRAY_WARNING, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, multi_target, -1 );
		}

//		HUD_sourced_printf(HUD_SOURCE_TERRAN_CMD, XSTR("Terran Command: You're straying too far from battle pilot, return immediately or be taken from the battlefield.", -1));
		if (p->distance_warning_count > PLAYER_DISTANCE_MAX_WARNINGS) {
			p->flags |= PLAYER_FLAGS_DIST_WARNING;
		}
	}

	if ( !(p->flags & PLAYER_FLAGS_FORCE_MISSION_OVER) && ((p->distance_warning_count > PLAYER_DISTANCE_MAX_WARNINGS) || (dist > PLAYER_MAX_DIST_END)) ) {
//		DKA 5/17/99 - DONT force warpout.  Won't work multiplayer.  Blow up ship.
		if ( !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
			message_send_builtin_to_player( MESSAGE_STRAY_WARNING_FINAL, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, multi_target, -1 );
			p->flags |= PLAYER_FLAGS_DIST_TO_BE_KILLED;
			p->distance_warning_time = timestamp(PLAYER_DEATH_DELTA_TIME);
		}
//		HUD_sourced_printf(HUD_SOURCE_TERRAN_CMD, XSTR("Terran Command: Sorry pilot, removing you from battle because of your insubordination!!!", -1));
//		gameseq_post_event(GS_EVENT_PLAYER_WARPOUT_START_FORCED);

		// get hull strength and blow up
		if ( (p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) && (timestamp_until(p->distance_warning_time) < 0) ) {
			p->flags |= PLAYER_FLAGS_FORCE_MISSION_OVER;
			float damage = 10.0f * Objects[p->objnum].hull_strength;
			ship_apply_global_damage(&Objects[p->objnum], &Objects[p->objnum], NULL, damage);
		}
	}

	// see if player has moved back into "bounds"
	if ( (dist < PLAYER_MAX_DIST_WARNING) && (p->flags & PLAYER_FLAGS_DIST_WARNING) && !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
		p->flags &= ~PLAYER_FLAGS_DIST_WARNING;
		p->distance_warning_count = 1;
	}
}

void ship_check_player_distance()
{
	int idx;

	// multiplayer
	if (Game_mode & GM_MULTIPLAYER) {
		// if I'm the server, check all non-observer players including myself
		if (MULTIPLAYER_MASTER) {
			// warn all players
			for (idx=0; idx<MAX_PLAYERS; idx++) {
				if (MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].type != OBJ_GHOST) ) {
					// if bad, blow him up
					ship_check_player_distance_sub(Net_players[idx].m_player, idx);
				}
			}
		}
	}
	// single player
	else {
		// maybe blow him up
		ship_check_player_distance_sub(Player);
	}		
}

void observer_process_post(object *objp)
{
	Assert(objp != NULL);

	if (objp == NULL)
		return;

	Assert(objp->type == OBJ_OBSERVER);

	if (Game_mode & GM_MULTIPLAYER) {
		// if I'm just an observer
		if (MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM])) {
			float dist = vm_vec_dist_quick(&Player_obj->pos, &vmd_zero_vector);
			// if beyond max dist, reset to 0
			if (dist > PLAYER_MAX_DIST_END) {
				// set me to zero
				if ((Player_obj != NULL) && (Player_obj->type != OBJ_GHOST)) {
					Player_obj->pos = vmd_zero_vector;
				}
			}
		}
	}
}

// reset some physics info when ship's engines goes from disabled->enabled 
void ship_reset_disabled_physics(object *objp, int ship_class)
{
	Assert(objp != NULL);

	if (objp == NULL)
		return;

	objp->phys_info.flags &= ~(PF_REDUCED_DAMP | PF_DEAD_DAMP);
	objp->phys_info.side_slip_time_const = Ship_info[ship_class].damp;
}

// Clear/set the subsystem disrupted flags
void ship_subsys_disrupted_check(ship *sp)
{
	ship_subsys *ss;
	int engines_disabled=0;
	
	if ( sp->subsys_disrupted_flags & (1<<SUBSYSTEM_ENGINE) ) {
		engines_disabled=1;
	}

	sp->subsys_disrupted_flags=0;

	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( !timestamp_elapsed(ss->disruption_timestamp) ) {
			sp->subsys_disrupted_flags |= (1<<ss->system_info->type);
		}
		ss = GET_NEXT( ss );
	}

	if ( engines_disabled ) {
		if ( !(sp->subsys_disrupted_flags & (1<<SUBSYSTEM_ENGINE)) ) {
			if ( !(sp->flags & SF_DISABLED) ) {
				ship_reset_disabled_physics(&Objects[sp->objnum], sp->ship_info_index);
			}
		}
	}
}

// Maybe check ship subsystems for disruption, and set/clear flags
void ship_subsys_disrupted_maybe_check(ship *shipp)
{
	if ( timestamp_elapsed(shipp->subsys_disrupted_check_timestamp) ) {
		ship_subsys_disrupted_check(shipp);
		shipp->subsys_disrupted_check_timestamp=timestamp(250);
	}
}

// Determine if a given subsystem is disrupted (ie inoperable)
// input:	ss		=>		pointer to ship subsystem
// exit:		1		=>		subsystem is disrupted
//				0		=>		subsystem is not disrupted
int ship_subsys_disrupted(ship_subsys *ss)
{
	if ( !ss ) {
		Int3();		// should never happen, get Alan if it does.
		return 0;
	}

	if ( timestamp_elapsed(ss->disruption_timestamp) ) {
		return 0;
	} else {
		return 1;
	}
}

// Disrupt a subsystem (ie make it inoperable for a time)
// input:	ss		=>		ship subsystem to be disrupted
//				time	=>		time in ms that subsystem should be disrupted
void ship_subsys_set_disrupted(ship_subsys *ss, int time)
{
	int time_left=0;

	if ( !ss ) {
		Int3();		// should never happen, get Alan if it does.
		return;
	}

	time_left=timestamp_until(ss->disruption_timestamp);
	if ( time_left < 0 ) {
		time_left=0;
	}

	ss->disruption_timestamp = timestamp(time+time_left);
}

// Determine if a given subsystem is disrupted (ie inoperable)
// input:	sp		=>		pointer to ship containing subsystem
//				type	=>		type of subsystem (SUBSYSTEM_*)
// exit:		1		=>		subsystem is disrupted
//				0		=>		subsystem is not disrupted
//
int ship_subsys_disrupted(ship *sp, int type)
{
	if ( sp->subsys_disrupted_flags & (1<<type) ) {
		return 1;
	} else {
		return 0;
	}
}

float Decay_rate = 1.0f / 120.0f;
DCF(lethality_decay, "time in sec to return from 100 to 0")
{
	dc_get_arg(ARG_FLOAT);
	Decay_rate = Dc_arg_float;
}

float min_lethality = 0.0f;

void lethality_decay(ai_info *aip)
{
	float decay_rate = Decay_rate;
	aip->lethality -= 100.0f * decay_rate * flFrametime;
	aip->lethality = MAX(-10.0f, aip->lethality);

//	if (aip->lethality < min_lethality) {
//		min_lethality = aip->lethality;
//		mprintf(("new lethality low: %.1f\n", min_lethality));
//	}

#ifndef NDEBUG
	if (Objects[Ships[aip->shipnum].objnum].flags & OF_PLAYER_SHIP) {
		if (Framecount % 10 == 0) {
			int num_turrets = 0;
			if ((aip->target_objnum != -1) && (Objects[aip->target_objnum].type == OBJ_SHIP)) {
				//TODO: put this where it belongs, this would involve recompiling *everything* right now
				//-WMC
				int num_turrets_attacking(object *turret_parent, int target_objnum);
				num_turrets = num_turrets_attacking(&Objects[aip->target_objnum], Ships[aip->shipnum].objnum);
			}
			nprintf(("lethality", "Player lethality: %.1f, num turrets targeting player: %d\n", aip->lethality, num_turrets));
		}
	}
#endif
}

void ship_process_pre(object *objp, float frametime)
{
	if ( (objp == NULL) || !frametime )
		return;
}

MONITOR( NumShips )

//	Player ship uses this code, but does a quick out after doing a few things.
// when adding code to this function, decide whether or not a client in a multiplayer game
// needs to execute the code you are adding.  Code which moves things, creates things, etc
// probably doesn't need to be called.  If you don't know -- find Allender!!!
void ship_process_post(object * obj, float frametime)
{
	int	num;
	ship	*shipp;
	ship_info *sip;

	if(obj->type != OBJ_SHIP){
		nprintf(("Network","Ignoring non-ship object in ship_process_post()\n"));
		return;
	}

	MONITOR_INC( NumShips, 1 );	

	num = obj->instance;
	Assert( num >= 0 && num < MAX_SHIPS);
	Assert( obj->type == OBJ_SHIP );
	Assert( Ships[num].objnum == OBJ_INDEX(obj));	

	shipp = &Ships[num];

	sip = &Ship_info[shipp->ship_info_index];

	shipp->shield_hits = 0;

	update_ets(obj, frametime);

	afterburners_update(obj, frametime);

	ship_subsys_disrupted_maybe_check(shipp);

	ship_dying_frame(obj, num);

	shipfx_cloak_frame(shipp, frametime);

	ship_chase_shield_energy_targets(shipp, obj, frametime);

/*	if (timestamp_elapsed(shipp->boost_finish_stamp))
	{
		shipp->boost_pod_engaged=0;
		obj->phys_info.flags &= ~(PF_BOOSTER_ON);
	}*/

	// AL 1-6-98: record the initial ammo counts for ships, which is used as the max limit for rearming
	// Goober5000 - added ballistics support
	if ( !(shipp->flags & SF_AMMO_COUNT_RECORDED) )
	{
		int max_missiles;
		for ( int i=0; i<MAX_SHIP_SECONDARY_BANKS; i++ ) {
			if ( red_alert_mission() )
			{
				max_missiles = get_max_ammo_count_for_bank(shipp->ship_info_index, i, shipp->weapons.secondary_bank_weapons[i]);
				shipp->weapons.secondary_bank_start_ammo[i] = max_missiles;
			}
			else
			{
				shipp->weapons.secondary_bank_start_ammo[i] = shipp->weapons.secondary_bank_ammo[i];
			}
		}

		if ( sip->flags & SIF_BALLISTIC_PRIMARIES )
		{
			for ( int i=0; i<MAX_SHIP_PRIMARY_BANKS; i++ )
			{
				if ( red_alert_mission() )
				{
					max_missiles = get_max_ammo_count_for_primary_bank(shipp->ship_info_index, i, shipp->weapons.primary_bank_weapons[i]);
					shipp->weapons.primary_bank_start_ammo[i] = max_missiles;
				}
				else
				{
					shipp->weapons.primary_bank_start_ammo[i] = shipp->weapons.primary_bank_ammo[i];
				}
			}
		}
		
		shipp->flags |= SF_AMMO_COUNT_RECORDED;
	}

	if(!(Game_mode & GM_STANDALONE_SERVER)) {
		// Plot ship on the radar.  What about multiplayer ships?
		if ( obj != Player_obj )			// don't plot myself.
			radar_plot_object( obj );

		// MWA -- move the spark code to before the check for multiplayer master
		//	Do ship sparks.  Don't do sparks on my ship (since I cannot see it).  This
		// code will do sparks on other ships in multiplayer though.
		// JAS: Actually in external view, you can see sparks, so I don't do sparks
		// on the Viewer_obj, not Player_obj.
		if ( (obj != Viewer_obj) && timestamp_elapsed(Ships[num].next_hit_spark) )	{
			shipfx_emit_spark(num,-1);	// -1 means choose random spark location
		}

		if ( obj != Viewer_obj )	{
			shipfx_do_damaged_arcs_frame( shipp );
		}

		// JAS - flicker the thruster bitmaps
		ship_do_thruster_frame(shipp,obj,frametime);		
	}

	ship_auto_repair_frame(num, frametime);

	// MWA -- move the spark code to before the check for multiplayer master
	//	Do ship sparks.
//	if (timestamp_elapsed(Ships[num].next_hit_spark))	{
//		ship_spark(num);
//		Ships[num].next_hit_spark = timestamp_rand(100,500);
//	}

	shipfx_do_lightning_frame(shipp);

	// if the ship has an EMP effect active, process it
	emp_process_ship(shipp);	

	// call the contrail system
	ct_ship_process(shipp);

	// process engine wash
	void engine_wash_ship_process(ship *shipp);
	engine_wash_ship_process(shipp);

	// update TAG info
	if(shipp->tag_left > 0.0f){
		shipp->tag_left -= flFrametime;
		if(shipp->tag_left <= 0.000001f){
			shipp->tag_left = -1.0f;

			mprintf(("Killing TAG for %s\n", shipp->ship_name));
		}
	}
	
	// update level 2 TAG info
	if(shipp->level2_tag_left > 0.0f){
		shipp->level2_tag_left -= flFrametime;
		if(shipp->level2_tag_left <= 0.000001f){
			shipp->level2_tag_left = -1.0f;

			mprintf(("Killing level 2 TAG for %s\n", shipp->ship_name));
		}
	}
	
	if ( shipp->flags & SF_ARRIVING && Ai_info[shipp->ai_index].mode != AIM_BAY_EMERGE )	{
		// JAS -- if the ship is warping in, just move it forward at a speed
		// fast enough to move 2x its radius in SHIP_WARP_TIME seconds.
		shipfx_warpin_frame( obj, frametime );
	} else if ( shipp->flags & SF_DEPART_WARP ) {
		// JAS -- if the ship is warping out, just move it forward at a speed
		// fast enough to move 2x its radius in SHIP_WARP_TIME seconds.
		shipfx_warpout_frame( obj, frametime );
	} else {
		//	Do AI.

		// for multiplayer people.  return here if in multiplay and not the host
		if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
			model_anim_handle_multiplayer( &Ships[num] );
			return;
		}

		// MWA -- moved the code to maybe fire swarm missiles to after the check for
		// multiplayer master.  Only single player and multi server needs to do this code
		// this code might call ship_fire_secondary which will send the fire packets
		swarm_maybe_fire_missile(num);

		// maybe fire turret swarm missiles
		void turret_swarm_maybe_fire_missile(int num);
		turret_swarm_maybe_fire_missile(num);

		// maybe fire a corkscrew missile (just like swarmers)
		cscrew_maybe_fire_missile(num);

		//rotate player subobjects since its processed by the ai functions
		// AL 2-19-98: Fire turret for player if it exists
		//WMC - changed this to call process_subobjects
		if ( (obj->flags & OF_PLAYER_SHIP) && !Player_use_ai )
		{
			ai_info *aip = &Ai_info[Ships[obj->instance].ai_index];
			if (aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED))
			{
				if (aip->support_ship_objnum >= 0)
				{
					if (vm_vec_dist_quick(&obj->pos, &Objects[aip->support_ship_objnum].pos) < (obj->radius + Objects[aip->support_ship_objnum].radius) * 1.25f)
						return;
				}
			}
			process_subobjects(OBJ_INDEX(obj));
			/*
			model_subsystem	*psub;
			ship_subsys	*pss;
			
			for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) )
			{
				psub = pss->system_info;

				// do solar/radar/gas/activator rotation here
				ship_do_submodel_rotation(shipp, psub, pss);
			}

			
			player_maybe_fire_turret(obj);*/
		}

		// if single player, check player object is not too far from starting location
		// DKA 5/17/99 check SINGLE and MULTI
//		if ( !(Game_mode & GM_MULTIPLAYER) && (obj == Player_obj) )
		if (obj == Player_obj) {
			ship_check_player_distance();
		}

		// update ship lethality
		if ( Ships[num].ai_index >= 0 ){
			if (!physics_paused && !ai_paused){
				lethality_decay(&Ai_info[Ships[num].ai_index]);
			}
		}

	
		// if the ship is an observer ship don't need to do AI
		if ( obj->type == OBJ_OBSERVER)  {
			return;
		}

		// Goober5000 - player may want to use AI
		if ( (Ships[num].ai_index >= 0) && (!(obj->flags & OF_PLAYER_SHIP) || Player_use_ai) ){
			if (!physics_paused && !ai_paused){
				ai_process( obj, Ships[num].ai_index, frametime );
			}
		}
	}			
}


// ------------------------------------------------------------------------
//	ship_set_default_weapons()
//
//	Set the ship level weapons based on the information contained in the ship
// info.  Weapon assignments are checked against the model to ensure the models
// and the ship info weapon data are in synch.
//
//

void ship_set_default_weapons(ship *shipp, ship_info *sip)
{
	int			i;
	polymodel	*pm;
	ship_weapon *swp = &shipp->weapons;
	weapon_info *wip;

	//	Copy primary and secondary weapons from ship_info to ship.
	//	Later, this will happen in the weapon loadout screen.
	for (i=0; i < MAX_SHIP_PRIMARY_BANKS; i++){
		swp->primary_bank_weapons[i] = sip->primary_bank_weapons[i];
	}

	for (i=0; i < MAX_SHIP_SECONDARY_BANKS; i++){
		swp->secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
	}

	// Copy the number of primary and secondary banks to ship, and verify that
	// model is in synch
	pm = model_get( sip->model_num );

	// Primary banks
	if ( pm->n_guns > sip->num_primary_banks ) {
		Assert(pm->n_guns <= MAX_SHIP_PRIMARY_BANKS);
		Warning(LOCATION, "There are %d primary banks in the model file,\nbut only %d primary banks specified for %s\n", pm->n_guns, sip->num_primary_banks, sip->name);
		for ( i = sip->num_primary_banks; i < pm->n_guns; i++ ) {
			// Make unspecified weapon for bank be a Light Laser
			swp->primary_bank_weapons[i] = weapon_info_lookup(NOX("Light Laser"));
			Assert(swp->primary_bank_weapons[i] >= 0);
		}
		sip->num_primary_banks = pm->n_guns;
	}
	else if ( pm->n_guns < sip->num_primary_banks ) {
		Warning(LOCATION, "There are %d primary banks specified for %s\nbut only %d primary banks in the model\n", sip->num_primary_banks, sip->name, pm->n_guns);
		sip->num_primary_banks = pm->n_guns;
	}

	// Secondary banks
	if ( pm->n_missiles > sip->num_secondary_banks ) {
		Assert(pm->n_missiles <= MAX_SHIP_SECONDARY_BANKS);
		Warning(LOCATION, "There are %d secondary banks in model,\nbut only %d secondary banks specified for %s\n", pm->n_missiles, sip->num_secondary_banks, sip->name);
		for ( i = sip->num_secondary_banks; i < pm->n_missiles; i++ ) {
			// Make unspecified weapon for bank be a Rockeye Missile
			swp->secondary_bank_weapons[i] = weapon_info_lookup(NOX("Rockeye Missile"));
			Assert(swp->secondary_bank_weapons[i] >= 0);
		}
		sip->num_secondary_banks = pm->n_missiles;
	}
	else if ( pm->n_missiles < sip->num_secondary_banks ) {
		Warning(LOCATION, "There are %d secondary banks specified for %s,\n but only %d secondary banks in the model.\n", sip->num_secondary_banks, sip->name, pm->n_missiles);
		sip->num_secondary_banks = pm->n_missiles;
	}

	// added ballistic primary support - Goober5000
	swp->num_primary_banks = sip->num_primary_banks;
	for ( i = 0; i < swp->num_primary_banks; i++ )
	{
		wip = &Weapon_info[swp->primary_bank_weapons[i]];

		if ( wip->wi_flags2 & WIF2_BALLISTIC )
		{
			if (Fred_running){
				swp->primary_bank_ammo[i] = 100;
			}
			else
			{
				float capacity, size;
				capacity = (float) sip->primary_bank_ammo_capacity[i];
				size = (float) wip->cargo_size;
				swp->primary_bank_ammo[i] = fl2i((capacity / size)+0.5f);
				swp->primary_bank_start_ammo[i] = swp->primary_bank_ammo[i];
			}

			swp->primary_bank_capacity[i] = sip->primary_bank_ammo_capacity[i];
		}
	}

	swp->num_secondary_banks = sip->num_secondary_banks;
	for ( i = 0; i < swp->num_secondary_banks; i++ ) {
		if (Fred_running){
			swp->secondary_bank_ammo[i] = 100;
		} else {
			wip = &Weapon_info[swp->secondary_bank_weapons[i]];
			float size = (float) wip->cargo_size;
			swp->secondary_bank_ammo[i] = fl2i(sip->secondary_bank_ammo_capacity[i]/size);
			// Karajorma - Support ships will use the wrong values if we don't set this. 
			swp->secondary_bank_start_ammo[i] = swp->secondary_bank_ammo[i];
		}

		swp->secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
	}

	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ ){
		swp->next_primary_fire_stamp[i] = timestamp(0);
	}

	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ ){
		swp->next_secondary_fire_stamp[i] = timestamp(0);
	}
}


//	A faster version of ship_check_collision that does not do checking at the polygon
//	level.  Just checks to see if a vector will intersect a sphere.
int ship_check_collision_fast( object * obj, object * other_obj, vec3d * hitpos)
{
	int num;
	mc_info mc;

	Assert( obj->type == OBJ_SHIP );
	Assert( obj->instance >= 0 );

	num = obj->instance;

	ship_model_start(obj);	// are these needed in this fast case? probably not.

	mc.model_num = Ship_info[Ships[num].ship_info_index].model_num;	// Fill in the model to check
	mc.orient = &obj->orient;					// The object's orient
	mc.pos = &obj->pos;							// The object's position
	mc.p0 = &other_obj->last_pos;			// Point 1 of ray to check
	mc.p1 = &other_obj->pos;					// Point 2 of ray to check
	mc.flags = MC_ONLY_SPHERE;				// flags

	model_collide(&mc);
	if (mc.num_hits)
		*hitpos = mc.hit_point_world;
	
	ship_model_stop(obj);	// are these needed in this fast case? probably not.

	return mc.num_hits;
}

// Ensure create time for ship is unqiue
void ship_make_create_time_unique(ship *shipp)
{
	int		sanity_counter = 0, collision;
	ship		*compare_shipp;
	ship_obj	*so;
	uint		new_create_time;

	new_create_time = shipp->create_time;

	while (1) {

		if ( sanity_counter++ > 50 ) {
			Int3();
			break;
		}

		collision = 0;

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			compare_shipp = &Ships[Objects[so->objnum].instance];

			if ( compare_shipp == shipp ) {
				continue;
			}

			if ( compare_shipp->create_time == new_create_time ) {
				new_create_time++;
				collision = 1;
				break;
			}
		}

		if ( !collision ) {
			shipp->create_time = new_create_time;
			break;
		}
	}
}

int	Ship_subsys_hwm = 0;

void show_ship_subsys_count()
{
	object	*objp;
	int		count = 0;	
	int		o_type = 0;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		o_type = (int)objp->type;
		if (o_type == OBJ_SHIP) {
			count += Ship_info[Ships[o_type].ship_info_index].n_subsystems;
		}
	}

	//nprintf(("AI", "Num subsystems, high water mark = %i, %i\n", count, Ship_subsys_hwm));

	if (count > Ship_subsys_hwm) {
		Ship_subsys_hwm = count;
	}
}

//	Returns object index of ship.
//	-1 means failed.
int ship_create(matrix *orient, vec3d *pos, int ship_type, char *ship_name)
{
	int			i, n, objnum, j, k, t;
	ship_info	*sip;
	ship			*shipp;

	t = ship_get_num_ships();
	
	// The following check caps the number of ships that can be created.  Because Fred needs
	// to create all the ships, regardless of when they arrive/depart, it needs a higher
	// limit than FreeSpace.  On release, however, we will reduce it, thus FreeSpace needs
	// to check against what this limit will be, otherwise testing the missions before
	// release could work fine, yet not work anymore once a release build is made.
	if (Fred_running) {
		if (t >= MAX_SHIPS)
			return -1;

	} else {
		if (t >= SHIPS_LIMIT) {
			Error(LOCATION, XSTR("There is a limit of %d ships in the mission at once.  Please be sure that you do not have more than %d ships present in the mission at the same time.", 1495), SHIPS_LIMIT, SHIPS_LIMIT );
			return -1;
		}
	}

	//nprintf(("AI", "Number of ships = %i\n", t));

	for (n=0; n<MAX_SHIPS; n++){
		if (Ships[n].objnum == -1){
			break;
		}
	}

	if (n == MAX_SHIPS){
		return -1;
	}

	if(!Num_ship_classes){
		return -1;
	}

	//WMC - Invalid shipclass? Get the first ship class.
	if(ship_type < 0 || ship_type >= Num_ship_classes)
		ship_type = 0;

	sip = &(Ship_info[ship_type]);
	shipp = &Ships[n];

	//  check to be sure that this ship falls into a ship size category!!!
	//  get Allender or Mike if you hit this Assert
	//WMC - I hope this isn't really needed anymore. Took it out.

	sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);		// use the highest detail level

	// maybe load an optional hud target model
	if(strlen(sip->pof_file_hud)){
		// check to see if a "real" ship uses this model. if so, load it up for him so that subsystems are setup properly
		int idx;
		for(idx=0; idx<Num_ship_classes; idx++){
			if(!stricmp(Ship_info[idx].pof_file, sip->pof_file_hud)){
				Ship_info[idx].model_num = model_load(Ship_info[idx].pof_file, Ship_info[idx].n_subsystems, &Ship_info[idx].subsystems[0]);
			}
		}

		// mow load it for me with no subsystems
		sip->model_num_hud = model_load(sip->pof_file_hud, 0, NULL);
	}

	polymodel *pm = model_get(sip->model_num);

	ship_copy_subsystem_fixup(sip);
	show_ship_subsys_count();

	if ( sip->num_detail_levels < pm->n_detail_levels )
	{
		Warning(LOCATION, "For ship '%s', detail level\nmismatch (POF needs %d)", sip->name, pm->n_detail_levels );

		for (i=0; i<pm->n_detail_levels; i++ )	{
			sip->detail_distance[i] = 0;
		}
	}
	
	for (i=0; i<sip->num_detail_levels; i++ )	{
		pm->detail_depth[i] = i2fl(sip->detail_distance[i]);
	}

	if ( sip->flags & SIF_NAVBUOY )	{
		// JAS: Nav buoys don't need to do collisions!
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(sip->model_num), OF_RENDERS | OF_PHYSICS );
	} else {
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(sip->model_num), OF_RENDERS | OF_COLLIDES | OF_PHYSICS );
	}
	Assert( objnum >= 0 );

	shipp->ai_index = ai_get_slot(n);
	Assert( shipp->ai_index >= 0 );

	// Goober5000 - if no ship name specified, or if the name is too long,
	// or if the specified ship already exists, or if the specified ship has exited,
	// use a default name
	if ((ship_name == NULL) || (strlen(ship_name) > (NAME_LENGTH - 1)) || (ship_name_lookup(ship_name) >= 0) || (ship_find_exited_ship_by_name(ship_name) >= 0)) {
		sprintf(shipp->ship_name, NOX("%s %d"), Ship_info[ship_type].name, n);
	} else {
		strcpy(shipp->ship_name, ship_name);
	}

	ship_set_default_weapons(shipp, sip);	//	Moved up here because ship_set requires that weapon info be valid.  MK, 4/28/98
	ship_set(n, objnum, ship_type);

	init_ai_object(objnum);
	ai_clear_ship_goals( &Ai_info[Ships[n].ai_index] );		// only do this one here.  Can't do it in init_ai because it might wipe out goals in mission file

	//ship_set_default_weapons(shipp, sip);

	//	Allocate shield and initialize it.
	if (pm->shield.ntris) {
		shipp->shield_integrity = (float *) vm_malloc(sizeof(float) * pm->shield.ntris);
		for (i=0; i<pm->shield.ntris; i++)
			shipp->shield_integrity[i] = 1.0f;
	} else
		shipp->shield_integrity = NULL;

	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		if (pm->n_glow_point_banks)
			shipp->glow_point_bank_active.resize( pm->n_glow_point_banks, val );
	}

	// fix up references into paths for this ship's model to point to a ship_subsys entry instead
	// of a submodel index.  The ship_subsys entry should be the same for *all* instances of the
	// same ship.
	if (!(sip->flags & SIF_PATH_FIXUP))
	{
		for ( i = 0; i < pm->n_paths; i++ )
		{
			for ( j = 0; j < pm->paths[i].nverts; j++ )
			{
				for ( k = 0; k < pm->paths[i].verts[j].nturrets; k++ )
				{
					int ptindex = pm->paths[i].verts[j].turret_ids[k];		// this index is a submodel number (ala bspgen)
					int index;
					ship_subsys *ss;

					// iterate through the ship_subsystems looking for an id that matches
					index = 0;
					ss = GET_FIRST(&Ships[n].subsys_list);
					while ( ss != END_OF_LIST( &Ships[n].subsys_list ) ) {
						if ( ss->system_info->subobj_num == ptindex ) {			// when these are equal, fix up the ref
							pm->paths[i].verts[j].turret_ids[k] = index;				// in path structure to index a ship_subsys
							break;											
						}
						index++;
						ss = GET_NEXT( ss );
					}

					if ( ss == END_OF_LIST(&Ships[n].subsys_list) )
						Warning(LOCATION, "Couldn't fix up turret indices in spline path\n\nModel: %s\nPath: %s\nVertex: %d\nTurret model id:%d\n\nThis probably means that the turret was not specified in the ship table(s).", sip->pof_file, pm->paths[i].name, j, ptindex );
				}
			}
		}
		sip->flags |= SIF_PATH_FIXUP;
	}

	// reset the damage record fields (for scoring purposes)
	shipp->total_damage_received = 0.0f;
	for(i=0;i<MAX_DAMAGE_SLOTS;i++)
	{
		shipp->damage_ship[i] = 0.0f;
		shipp->damage_ship_id[i] = -1;
	}

	// Add this ship to Ship_obj_list
	shipp->ship_list_index = ship_obj_list_add(objnum);

	// Set time when ship is created
	shipp->create_time = timer_get_milliseconds();

	ship_make_create_time_unique(shipp);

	// set the team select index to be -1
	shipp->ts_index = -1;

	shipp->wing_status_wing_index = -1;		// wing index (0-4) in wingman status gauge
	shipp->wing_status_wing_pos = -1;		// wing position (0-5) in wingman status gauge

	//first try at ABtrails -Bobboau	
	shipp->ab_count = 0;
	if (sip->flags & SIF_AFTERBURNER)
	{
		for (i = 0; i < pm->n_thrusters; i++)
		{
			thruster_bank *bank = &pm->thrusters[i];

			for (j = 0; j < bank->num_points; j++)
			{
				// this means you've reached the max # of AB trails for a ship
				Assert(sip->ct_count <= MAX_SHIP_CONTRAILS);
	
				trail_info *ci = &shipp->ab_info[shipp->ab_count];
			//	ci = &sip->ct_info[sip->ct_count++];

				if (bank->points[j].norm.xyz.z > -0.5)
					continue;// only make ab trails for thrusters that are pointing backwards

				ci->pt = bank->points[j].pnt;//offset
				ci->w_start = bank->points[j].radius * sip->afterburner_trail_width_factor;	// width * table loaded width factor
	
				ci->w_end = 0.05f;//end width
	
				ci->a_start = 1.0f * sip->afterburner_trail_alpha_factor;	// start alpha  * table loaded alpha factor
	
				ci->a_end = 0.0f;//end alpha
	
				ci->max_life = sip->afterburner_trail_life;	// table loaded max life
	
				ci->stamp = 60;	//spew time???	

				ci->texture.bitmap_id = sip->afterburner_trail.bitmap_id; // table loaded bitmap used on this ships burner trails
				nprintf(("AB TRAIL", "AB trail point #%d made for '%s'\n", shipp->ab_count, shipp->ship_name));
				shipp->ab_count++;
				Assert(MAX_SHIP_CONTRAILS > shipp->ab_count);
			}
		}
	}//end AB trails -Bobboau

	// call the contrail system
	ct_ship_create(shipp);

	model_anim_set_initial_states(shipp);
/*
	polymodel *pm = model_get(shipp->modelnum);
	if(shipp->debris_flare)vm_free(shipp->debris_flare);
	shipp->debris_flare = (flash_ball*)vm_malloc(sizeof(flash_ball)*pm->num_debris_objects);
	shipp->n_debris_flare = pm->num_debris_objects;

	for(i = 0; i<shipp->n_debris_flare; i++){
		bsp_info *debris = &pm->submodel[pm->debris_objects[i]];
		shipp->debris_flare[i].initialize(debris->bsp_data, 0.1f,0.3f, &vmd_zero_vector, &debris->offset);
	}
	*/
	return objnum;
}

// ----------------------------------------------------------------
// ship_model_change()
//
// Change the ship model for a ship to that for ship class 'ship_type'
//
// input:	n				=>		index of ship in Ships[] array
//				ship_type	=>		ship class (index into Ship_info[])
//
void ship_model_change(int n, int ship_type)
{
	int			i;
	ship_info	*sip;
	ship			*sp;
	polymodel * pm;

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];
	sip = &(Ship_info[ship_type]);

	// get new model
	if (sip->model_num == -1) {
		sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
	}
	pm = model_get(sip->model_num);
	Objects[sp->objnum].radius = model_get_radius(pm->id);

	// page in nondims in game
	if ( !Fred_running )
		model_page_in_textures(sip->model_num, ship_type);

	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		// clear out any old gpb's first, then add new ones if needed
		sp->glow_point_bank_active.clear();

		if (pm->n_glow_point_banks)
			sp->glow_point_bank_active.resize( pm->n_glow_point_banks, val );
	}

	ship_copy_subsystem_fixup(sip);

	if ( sip->num_detail_levels < pm->n_detail_levels )	{
		Warning(LOCATION, "For ship '%s', detail level\nmismatch (POF needs %d)", sip->name, pm->n_detail_levels );

		for (i=0; i<pm->n_detail_levels; i++ )	{
			sip->detail_distance[i] = 0;
		}
	}

	for (i=0; i<sip->num_detail_levels; i++ )	{
		pm->detail_depth[i] = i2fl(sip->detail_distance[i]);
	}

	// reset texture animations
	sp->base_texture_anim_frametime = game_get_overall_frametime();
}

// ----------------------------------------------------------------
// change_ship_type()
//
// Change the ship class on a ship, and changing all required information
// for consistency (ie textures, subsystems, weapons, physics)
//
// input:	n				=>		index of ship in Ships[] array
//				ship_type	=>		ship class (index into Ship_info[])
//
void change_ship_type(int n, int ship_type, int by_sexp)
{
	ship_info	*sip;
	ship			*sp;
	object		*objp;
	float hull_pct, shield_pct;

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];
	sip = &(Ship_info[ship_type]);
	objp = &Objects[sp->objnum];

	// Goober5000 - maintain the original hull and shield percentages when called by sexp
	if (by_sexp)
	{
		// hull
		Assert(sp->ship_max_hull_strength > 0.0f);
		hull_pct = objp->hull_strength / sp->ship_max_hull_strength;

		// shield
		if (!(objp->flags & OF_NO_SHIELDS))
		{
			Assert(shield_get_max_strength(objp) > 0.0f);
			shield_pct = shield_get_strength(objp) / shield_get_max_strength(objp);
		}
		else
			shield_pct = 0.0f;
	}
	// set to 100% otherwise
	else
	{
		hull_pct = 1.0f;
		shield_pct = 1.0f;
	}

	// Goober5000 - extra checks
	Assert(hull_pct > 0.0f && hull_pct <= 1.0f);
	Assert(shield_pct >= 0.0f && shield_pct <= 1.0f);
	if (hull_pct <= 0.0f) hull_pct = 0.1f;
	if (hull_pct > 1.0f) hull_pct = 1.0f;
	if (shield_pct < 0.0f) shield_pct = 0.0f;
	if (shield_pct > 1.0f) shield_pct = 1.0f;


	// point to new ship data
	ship_model_change(n, ship_type);
	sp->ship_info_index = ship_type;


	// set the correct hull strength
	if (Fred_running) {
		sp->ship_max_hull_strength = 100.0f;
		objp->hull_strength = 100.0f;
	} else {
		if (sp->special_hitpoint_index != -1) {
			sp->ship_max_hull_strength = (float) atoi(Sexp_variables[sp->special_hitpoint_index+HULL_STRENGTH].text);
		} else {
			sp->ship_max_hull_strength = sip->max_hull_strength;
		}

		objp->hull_strength = hull_pct * sp->ship_max_hull_strength;
	}


	// set the correct shield strength
	if (Fred_running) {
		sp->ship_max_shield_strength = 100.0f;
		shield_set_quad(objp, 0, 100.0f);
	} else {
		if (sp->special_hitpoint_index != -1) {
			sp->ship_max_shield_strength = (float) atoi(Sexp_variables[sp->special_hitpoint_index+SHIELD_STRENGTH].text);
		} else {
			sp->ship_max_shield_strength = sip->max_shield_strength;
		}

		shield_set_strength(objp, shield_pct * shield_get_max_strength(objp));
	}


	// Goober5000: div-0 checks
	Assert(sp->ship_max_hull_strength > 0.0f);
	Assert(objp->hull_strength > 0.0f);


	// subsys stuff done only after hull stuff is set

	// if the subsystem list is not currently empty, then we need to clear it out first.
	ship_subsystems_delete(sp);

	// fix up the subsystems
	subsys_set( sp->objnum );

	sp->afterburner_fuel = sip->afterburner_fuel_capacity;

	ship_set_default_weapons(sp, sip);
	physics_ship_init(&Objects[sp->objnum]);
	ets_init_ship(&Objects[sp->objnum]);
	// mwa removed the next line in favor of simply setting the ai_class in AI_info.  ai_object_init
	// was trashing mode in ai_info when it was valid due to goals.
	//ai_object_init(&Objects[sp->objnum], sp->ai_index);
//	Ai_info[sp->ai_index].ai_class = sip->ai_class;

	// above removed by Goober5000 in favor of new ship_set_new_ai_class function :)
	ship_set_new_ai_class(n, sip->ai_class);

	// Goober5000: reset ship score too
	sp->score = sip->score;
	
	//======================================================

	// Bobboau's thruster stuff again
	if (sip->afterburner_trail.bitmap_id < 0)
		generic_bitmap_load(&sip->afterburner_trail);

	sp->ab_count = 0;
	if (sip->flags & SIF_AFTERBURNER)
	{
		polymodel *pm = model_get(sip->model_num);

		for (int h = 0; h < pm->n_thrusters; h++)
		{
			for (int j = 0; j < pm->thrusters->num_points; j++)
			{
				// this means you've reached the max # of AB trails for a ship
				Assert(sip->ct_count <= MAX_SHIP_CONTRAILS);
	
				trail_info *ci = &sp->ab_info[sp->ab_count];
			//	ci = &sip->ct_info[sip->ct_count++];

				// only make ab trails for thrusters that are pointing backwards
				if (pm->thrusters[h].points[j].norm.xyz.z > -0.5)
					continue;

				ci->pt = pm->thrusters[h].points[j].pnt;	//offset
				ci->w_start = pm->thrusters[h].points[j].radius * sip->afterburner_trail_width_factor;	// width * table loaded width factor
	
				ci->w_end = 0.05f;//end width
	
				ci->a_start = 1.0f * sip->afterburner_trail_alpha_factor;	// start alpha  * table loaded alpha factor
	
				ci->a_end = 0.0f;//end alpha
	
				ci->max_life = sip->afterburner_trail_life;	// table loaded max life
	
				ci->stamp = 60;	//spew time???	

				ci->texture.bitmap_id = sip->afterburner_trail.bitmap_id; // table loaded bitmap used on this ships burner trails
				nprintf(("AB TRAIL", "AB trail point #%d made for '%s'\n", sp->ab_count, sp->ship_name));
				sp->ab_count++;
				Assert(MAX_SHIP_CONTRAILS > sp->ab_count);
			}
		}
	}//end AB trails -Bobboau

/*
	Goober5000 (4/17/2005) - I'm commenting this out for the time being; it looks like a whole bunch of unneeded
	code.  It should be (and probably is) handled elsewhere, like ship_set or ship_create or something.  Contact
	me if you want to discuss this.

	ship_weapon	*swp;
	swp = &sp->weapons;
	int i;

	for( i = 0; i<MAX_SHIP_PRIMARY_BANKS;i++){
			swp->primary_animation_position[i] = false;
	}
	for( i = 0; i<MAX_SHIP_SECONDARY_BANKS;i++){
			swp->secondary_animation_position[i] = false;
	}
	model_anim_set_initial_states(sp);

	for(i = 0; i<MAX_SHIP_SECONDARY_BANKS; i++){
		if(Weapon_info[swp->secondary_bank_weapons[i]].fire_wait == 0.0){
			sp->reload_time[i] = 1.0f;
		}else{
			sp->reload_time[i] = 1.0f/Weapon_info[swp->secondary_bank_weapons[i]].fire_wait;
		}
	}
*/
}

#ifndef NDEBUG
//	Fire the debug laser
int ship_fire_primary_debug(object *objp)
{
	int	i;
	ship	*shipp = &Ships[objp->instance];
	vec3d wpos;

	if ( !timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[0]) )
		return 0;

	// do timestamp stuff for next firing time
	shipp->weapons.next_primary_fire_stamp[0] = timestamp(250);

	//	Debug code!  Make the single laser fire only one bolt and from the object center!
	for (i=0; i<MAX_WEAPONS; i++)
		if (!stricmp(Weapon_info[i].name, NOX("Debug Laser")))
			break;
	
	vm_vec_add(&wpos, &objp->pos, &(objp->orient.vec.fvec) );
	if (i != MAX_WEAPONS) {
		int weapon_objnum;
		weapon_objnum = weapon_create( &wpos, &objp->orient, i, OBJ_INDEX(objp) );
		weapon_set_tracking_info(weapon_objnum, OBJ_INDEX(objp), Ai_info[shipp->ai_index].target_objnum);
		return 1;
	} else
		return 0;
}
#endif

//	Launch countermeasures from object *objp.  rand_val is used in multiplayer to ensure that all
// clients in the game fire countermeasure the same way
int ship_launch_countermeasure(object *objp, int rand_val)
{
	if(!Countermeasures_enabled) {
		return 0;
	}

	int	check_count, cmeasure_count;
	int cobjnum=-1;
	vec3d	pos;
	ship	*shipp;
	ship_info *sip;

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	int arand;
	if(rand_val < 0) {
		arand = myrand();
	} else {
		arand = rand_val;
	}


	// in the case where the server is an observer, he can launch countermeasures unless we do this.
	if( objp->type == OBJ_OBSERVER){
		return 0;
	}

	if ( !timestamp_elapsed(shipp->cmeasure_fire_stamp) ){
		return 0;
	}

	shipp->cmeasure_fire_stamp = timestamp(CMEASURE_WAIT);	//	Can launch every half second.
#ifndef NDEBUG
	if (Weapon_energy_cheat) {
		shipp->cmeasure_count++;
	}
#endif

	// we might check the count of countermeasures left depending on game state.  Multiplayer clients
	// do not need to check any objects other than themselves for the count
	check_count = 1;

	if ( MULTIPLAYER_CLIENT && (objp != Player_obj) ){
		check_count = 0;
	}

	if (check_count && (shipp->cmeasure_count <= 0) || sip->cmeasure_type < 0)
	{
		if ( objp == Player_obj ) {
			if(sip->cmeasure_max < 1 || sip->cmeasure_type < 0) {
				//TODO: multi-lingual support
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Not equipped with countermeasures", -1));
			} else if(shipp->current_cmeasure < 0) {
				//TODO: multi-lingual support
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No countermeasures selected", -1));
			} else if(shipp->cmeasure_count <= 0) {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No more countermeasure charges.", 485));
			}
			snd_play( &Snds[SND_OUT_OF_MISSLES], 0.0f );
		}

		// if we have a player ship, then send the fired packet anyway so that the player
		// who fired will get his 'out of countermeasures' sound
		cmeasure_count = 0;
		if ( objp->flags & OF_PLAYER_SHIP ){
			goto send_countermeasure_fired;
		}

		return 0;
	}

	cmeasure_count = shipp->cmeasure_count;
	shipp->cmeasure_count--;

	vm_vec_scale_add(&pos, &objp->pos, &objp->orient.vec.fvec, -objp->radius/2.0f);

	// cmeasure_create fires 1 countermeasure.  returns -1 if not fired, otherwise a non-negative
	// value
	//fired = cmeasure_create( objp, &pos, shipp->current_cmeasure, rand_val );
	cobjnum = weapon_create(&pos, &objp->orient, shipp->current_cmeasure, OBJ_INDEX(objp));
	if (cobjnum >= 0)
	{
		cmeasure_set_ship_launch_vel(&Objects[cobjnum], objp, arand);
		nprintf(("Network", "Cmeasure created by %s\n", shipp->ship_name));

		// Play sound effect for counter measure launch
		Assert(shipp->current_cmeasure < Num_weapon_types);
		if ( Weapon_info[shipp->current_cmeasure].launch_snd >= 0 ) {
			snd_play_3d( &Snds[Weapon_info[shipp->current_cmeasure].launch_snd], &pos, &View_position );
		}

send_countermeasure_fired:
		// the new way of doing things
		// if(Netgame.debug_flags & NETD_FLAG_CLIENT_FIRING){
		if(Game_mode & GM_MULTIPLAYER){
			send_NEW_countermeasure_fired_packet( objp, cmeasure_count, arand );
		}
	}

	return (cobjnum >= 0);		// return 0 if not fired, 1 otherwise
}

// internal function.. see if enough time has elapsed to play fail sound again
int ship_maybe_play_primary_fail_sound()
{
	ship_weapon *swp = &Player_ship->weapons;
	int stampval;

	hud_start_flash_weapon(swp->current_primary_bank);

	if ( timestamp_elapsed(Laser_energy_out_snd_timer) )
	{
		// check timestamp according to ballistics
		if (Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].wi_flags2 & WIF2_BALLISTIC)
		{
			stampval = 500;
		}
		else
		{
			stampval = 50;
		}
		Laser_energy_out_snd_timer = timestamp(stampval);
		snd_play( &Snds[SND_OUT_OF_WEAPON_ENERGY]);
		return 1;
	}
	return 0;
}

// internal function.. see if enough time has elapsed to play fail sound again
int ship_maybe_play_secondary_fail_sound(weapon_info *wip)
{
	hud_start_flash_weapon(Player_ship->weapons.num_primary_banks + Player_ship->weapons.current_secondary_bank);

	if ( timestamp_elapsed(Missile_out_snd_timer) ) {
		
		if ( wip->wi_flags & WIF_SWARM ) {
			Missile_out_snd_timer = timestamp(500);
		} else {
			Missile_out_snd_timer = timestamp(50);
		}
		snd_play( &Snds[SND_OUT_OF_MISSLES] );
		return 1;
	}
	return 0;
}

// internal function.. see if weapon for ship can fire based on weapons subystem
// strength.
//
// returns:		1	=>		weapon failed to fire
//					0	=>		weapon can fire
int ship_weapon_maybe_fail(ship *sp)
{
	int	rval;
	float	weapons_subsys_str;

	// If playing on lowest skill level, weapons will not fail due to subsystem damage
	if ( Game_skill_level == 0 ){
		return 0;
	}

	rval = 0;
	weapons_subsys_str = ship_get_subsystem_strength( sp, SUBSYSTEM_WEAPONS );
	if ( weapons_subsys_str < SUBSYS_WEAPONS_STR_FIRE_FAIL ) {
		rval = 1;
	}
	else if ( weapons_subsys_str < SUBSYS_WEAPONS_STR_FIRE_OK ) {
		// chance to fire depends on weapons subsystem strength
		if ( (frand()-0.2f) > weapons_subsys_str )		
			rval = 1;
	}

	if (!rval) {
		// is subsystem disrupted?
		if ( ship_subsys_disrupted(sp, SUBSYSTEM_WEAPONS) ) {
			rval=1;
		}
	}
		
	return rval;
}

// create a moving tracer based upon a weapon which just fired
float t_rad = 0.5f;
float t_len = 10.0f;
float t_vel = 0.2f;
float t_min = 150.0f;
float t_max = 300.0f;
DCF(t_rad, "")
{
	dc_get_arg(ARG_FLOAT);
	t_rad = Dc_arg_float;
}
DCF(t_len, "")
{
	dc_get_arg(ARG_FLOAT);
	t_len = Dc_arg_float;
}
DCF(t_vel, "")
{
	dc_get_arg(ARG_FLOAT);
	t_vel = Dc_arg_float;
}
DCF(t_min, "")
{
	dc_get_arg(ARG_FLOAT);
	t_min = Dc_arg_float;
}
DCF(t_max, "")
{
	dc_get_arg(ARG_FLOAT);
	t_max = Dc_arg_float;
}
void ship_fire_tracer(int weapon_objnum)
{
	particle_info pinfo;
	object *objp = &Objects[weapon_objnum];
	weapon_info *wip = &Weapon_info[Weapons[Objects[weapon_objnum].instance].weapon_info_index];

	// setup particle info
	memset(&pinfo, 0, sizeof(particle_info));
	pinfo.pos = objp->pos;
	pinfo.vel = objp->phys_info.vel;
	vm_vec_scale(&pinfo.vel, t_vel);
	pinfo.lifetime = wip->lifetime;
	pinfo.rad = t_rad;
	pinfo.type = PARTICLE_BITMAP;
	pinfo.optional_data = wip->laser_bitmap.first_frame;
	pinfo.tracer_length = t_len;
	pinfo.reverse = 0;
	pinfo.attached_objnum = -1;
	pinfo.attached_sig = 0;

	// create the particle
	particle_create(&pinfo);
}

// stops a single primary bank-Bobboau
int ship_stop_fire_primary_bank(object * obj, int bank_to_stop)
{
	ship			*shipp;
	ship_weapon	*swp;

	if(obj == NULL){
		return 0;
	}

	if(obj->type != OBJ_SHIP){
		return 0;
	}
//		gr_set_color( 250, 50, 750 );

	shipp = &Ships[obj->instance];

//	mprintf(("stoping weapon on ship %s\n", shipp->ship_name));

	swp = &shipp->weapons;
	if(Ship_info[shipp->ship_info_index].draw_primary_models[bank_to_stop]){
		if(shipp->primary_rotate_rate[bank_to_stop] > 0.0f)
			shipp->primary_rotate_rate[bank_to_stop] -= Weapon_info[swp->primary_bank_weapons[bank_to_stop]].weapon_submodel_rotate_accell*flFrametime;
		if(shipp->primary_rotate_rate[bank_to_stop] < 0.0f)shipp->primary_rotate_rate[bank_to_stop] = 0.0f;
		shipp->primary_rotate_ang[bank_to_stop] += shipp->primary_rotate_rate[bank_to_stop]*flFrametime;
		if(shipp->primary_rotate_ang[bank_to_stop] > PI2)shipp->primary_rotate_ang[bank_to_stop] -= PI2;
		if(shipp->primary_rotate_ang[bank_to_stop] < 0.0f)shipp->primary_rotate_ang[bank_to_stop] += PI2;
	}
	if(shipp->was_firing_last_frame[bank_to_stop] == 0)return 0;


		shipp->was_firing_last_frame[bank_to_stop] = 0;

//		int weapon = swp->primary_bank_weapons[bank_to_stop];
//		weapon_info* winfo_p = &Weapon_info[weapon];
/*
	if ( obj == Player_obj ){
		gr_printf(10, 20 + (bank_to_stop*10), "stoped bank %d", bank_to_stop);
		HUD_printf("stoped bank %d", bank_to_stop);
	}
*/

	return 1;
}


//stuff to do when the ship has stoped fireing all primary weapons-Bobboau
int ship_stop_fire_primary(object * obj)
{
//	gr_set_color( 250, 50, 75 );

	int i, num_primary_banks = 0, bank_to_stop = 0;
	ship			*shipp;
	ship_weapon	*swp;

	if(obj == NULL){
		return 0;
	}

	if(obj->type != OBJ_SHIP){
		return 0;
	}

	shipp = &Ships[obj->instance];

//	mprintf(("stoping weapon on ship %s\n", shipp->ship_name));

//	if(shipp->was_firing_last_frame[bank_to_stop] == 0)return 0;

	swp = &shipp->weapons;

	bank_to_stop = swp->current_primary_bank;

	if ( shipp->flags & SF_PRIMARY_LINKED ) {
		num_primary_banks = swp->num_primary_banks;
	} else {
		num_primary_banks = MIN(1, swp->num_primary_banks);
	}

	for ( i = 0; i < num_primary_banks; i++ ) {	
		// Goober5000 - allow more than two banks
		bank_to_stop = (swp->current_primary_bank+i) % swp->num_primary_banks;
		//only stop if it was fireing last frame
		ship_stop_fire_primary_bank(obj, bank_to_stop);
	}
	for(i = 0; i<swp->num_primary_banks+num_primary_banks;i++)
		ship_stop_fire_primary_bank(obj, i%swp->num_primary_banks);

/*	if ( obj == Player_obj ){
		gr_printf(10, 10, "stoped all");
	}
*/

	return 1;
}



int tracers[MAX_SHIPS][4][4];

float ship_get_subsystem_strength( ship *shipp, int type );

// fires a primary weapon for the given object.  It also handles multiplayer cases.
// in multiplayer, the starting network signature, and number of banks fired are sent
// to all the clients in the game. All the info is passed to send_primary at the end of
// the function.  The check_energy parameter (defaults to 1) tells us whether or not
// we should check the energy.  It will be 0 when a multiplayer client is firing an AI
// primary.
int ship_fire_primary(object * obj, int stream_weapons, int force)
{
	vec3d		gun_point, pnt, firing_pos;
	int			n = obj->instance;
	ship			*shipp;
	ship_weapon	*swp;
	ship_info	*sip;
	ai_info		*aip;
	int			weapon, i, j, w, v, weapon_objnum;
	int			bank_to_fire, num_fired = 0;	
	int			banks_fired, have_timeout;				// used for multiplayer to help determine whether or not to send packet
	have_timeout = 0;			// used to help tell us whether or not we need to send a packet
	banks_fired = 0;			// used in multiplayer -- bitfield of banks that were fired

	int			sound_played;	// used to track what sound is played.  If the player is firing two banks
										// of the same laser, we only want to play one sound
	Assert( obj != NULL );

	if(obj == NULL){
		return 0;
	}

	// in the case where the server is an observer, he can fire (which) would be bad - unless we do this.
	if( obj->type == OBJ_OBSERVER){
		return 0;
	}

	Assert( obj->type == OBJ_SHIP );
	Assert( n >= 0 );
	Assert( Ships[n].objnum == OBJ_INDEX(obj));
	if((obj->type != OBJ_SHIP) || (n < 0) || (n >= MAX_SHIPS) || (Ships[n].objnum != OBJ_INDEX(obj))){
		return 0;
	}
	
	shipp = &Ships[n];
	swp = &shipp->weapons;

	//if (shipp->targeting_laser_objnum != -1)
	//shipp->targeting_laser_objnum = -1; // erase old laser obj num if it has any -Bobboau

	// bogus 
	if((shipp->ship_info_index < 0) || (shipp->ship_info_index >= Num_ship_classes)){
		return 0;
	}
	if((shipp->ai_index < 0) || (shipp->ai_index >= MAX_AI_INFO)){
		return 0;
	}
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	if ( swp->num_primary_banks <= 0 ) {
		return 0;
	}

	if ( swp->current_primary_bank < 0 ){
		return 0;
	}	

	// If the primaries have been locked, bail
	if (shipp->flags2 & SF2_PRIMARIES_LOCKED)
	{
		return 0;
	}

	sound_played = -1;

	// Fire the correct primary bank.  If primaries are linked (SF_PRIMARY_LINKED set), then fire 
	// both primary banks.
	int	num_primary_banks;

	if ( shipp->flags & SF_PRIMARY_LINKED ) {
		num_primary_banks = swp->num_primary_banks;
	} else {
		num_primary_banks = MIN(1, swp->num_primary_banks);
	}

	Assert(num_primary_banks > 0);
	if (num_primary_banks < 1){
		return 0;
	}

	// if we're firing stream weapons, but the trigger is not down, do nothing
	if(stream_weapons && !(shipp->flags & SF_TRIGGER_DOWN)){
		return 0;
	}

	if(num_primary_banks == 1)
		for(i = 0; i<swp->num_primary_banks; i++){
			if(i!=swp->current_primary_bank)ship_stop_fire_primary_bank(obj, i);
		}

	for ( i = 0; i < num_primary_banks; i++ ) {		
		// Goober5000 - allow more than two banks
		bank_to_fire = (swp->current_primary_bank+i) % swp->num_primary_banks;

		
		weapon = swp->primary_bank_weapons[bank_to_fire];
		Assert( weapon >= 0 && weapon < MAX_WEAPONS );		
		if ( (weapon < 0) || (weapon >= MAX_WEAPON_TYPES) ) {
			Int3();		// why would a ship try to fire a weapon that doesn't exist?
			continue;
		}		

		if (swp->primary_animation_position[bank_to_fire] == MA_POS_SET) {
			if ( timestamp_elapsed(swp->primary_animation_done_time[bank_to_fire]) )
				swp->primary_animation_position[bank_to_fire] = MA_POS_READY;
			else
				continue;
		}

		weapon_info* winfo_p = &Weapon_info[weapon];


		if(sip->draw_primary_models[bank_to_fire]){
			if(shipp->primary_rotate_rate[bank_to_fire] < winfo_p->weapon_submodel_rotate_vel)
				shipp->primary_rotate_rate[bank_to_fire] += winfo_p->weapon_submodel_rotate_accell*flFrametime;
			if(shipp->primary_rotate_rate[bank_to_fire] > winfo_p->weapon_submodel_rotate_vel)shipp->primary_rotate_rate[bank_to_fire] = winfo_p->weapon_submodel_rotate_vel;
			shipp->primary_rotate_ang[bank_to_fire] += shipp->primary_rotate_rate[bank_to_fire]*flFrametime;
			if(shipp->primary_rotate_ang[bank_to_fire] > PI2)shipp->primary_rotate_ang[bank_to_fire] -= PI2;
			if(shipp->primary_rotate_ang[bank_to_fire] < 0.0f)shipp->primary_rotate_ang[bank_to_fire] += PI2;

			if(shipp->primary_rotate_rate[bank_to_fire] < winfo_p->weapon_submodel_rotate_vel)continue;
		}
		// if this is a targeting laser, start it up   ///- only targeting laser if it is tag-c, otherwise it's a fighter beam -Bobboau
		if((winfo_p->wi_flags & WIF_BEAM) && (winfo_p->tag_level == 3) && (shipp->flags & SF_TRIGGER_DOWN) && (winfo_p->b_info.beam_type == BEAM_TYPE_C) ){
			ship_start_targeting_laser(shipp);
			continue;
		}

		// if we're firing stream weapons and this is a non stream weapon, skip it
		if(stream_weapons && !(winfo_p->wi_flags & WIF_STREAM)){
			continue;
		}
		// if we're firing non stream weapons and this is a stream weapon, skip it
		if(!stream_weapons && (winfo_p->wi_flags & WIF_STREAM)){
			continue;
		}

		// only non-multiplayer clients (single, multi-host) need to do timestamp checking
		if ( !timestamp_elapsed(swp->next_primary_fire_stamp[bank_to_fire]) ) {
			if (timestamp_until(swp->next_primary_fire_stamp[bank_to_fire]) > 5000){
				swp->next_primary_fire_stamp[bank_to_fire] = timestamp(1000);
			}

			have_timeout = 1;
		//	ship_stop_fire_primary_bank(obj, bank_to_fire);
			continue;
		}

		//nprintf(("AI", "Time = %7.3f, firing %s\n", f2fl(Missiontime), Weapon_info[weapon].name));

		// do timestamp stuff for next firing time
		float next_fire_delay = (float) winfo_p->fire_wait * 1000.0f;
		if (!(obj->flags & OF_PLAYER_SHIP) ) {
			if (shipp->team == Ships[Player_obj->instance].team){
				next_fire_delay *= The_mission.ai_profile->ship_fire_delay_scale_friendly[Game_skill_level];
			} else {
				next_fire_delay *= The_mission.ai_profile->ship_fire_delay_scale_hostile[Game_skill_level];
			}
		}

		polymodel *pm = model_get( sip->model_num );
		
		// Goober5000 (thanks to _argv[-1] for the original idea)
		if (!(The_mission.ai_profile->flags & AIPF_DISABLE_LINKED_FIRE_PENALTY))
		{
			next_fire_delay *= 1.0f + (num_primary_banks - 1) * 0.5f;		//	50% time penalty if banks linked
		}

		//	MK, 2/4/98: Since you probably were allowed to fire earlier, but couldn't fire until your frame interval
		//	rolled around, subtract out up to half the previous frametime.
		//	Note, unless we track whether the fire button has been held down, and not tapped, it's hard to
		//	know how much time to subtract off.  It could be this fire is "late" because the user didn't want to fire.
		if ((next_fire_delay > 0.0f)) {
			if (obj->flags & OF_PLAYER_SHIP) {
				int	t = timestamp_until(swp->next_primary_fire_stamp[bank_to_fire]);
				if (t < 0) {
					float	tx;

					tx = (float) t/-1000.0f;
					if (tx > flFrametime/2.0f){
						tx = 1000.0f * flFrametime * 0.7f;
					}
					next_fire_delay -= tx;
				}
				
				if ((int) next_fire_delay < 1){
					next_fire_delay = 1.0f;
				}
			}

			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
//			if ((winfo_p->wi_flags & WIF_BEAM) && (winfo_p->b_info.beam_type == BEAM_TYPE_C))// fighter beams fire constantly, they only stop if they run out of power -Bobboau
//			swp->next_primary_fire_stamp[bank_to_fire] = timestamp();
		}

		if (winfo_p->wi_flags2 & WIF2_CYCLE){
			Assert(pm->gun_banks[bank_to_fire].num_slots != 0);
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay / pm->gun_banks[bank_to_fire].num_slots));
			//to maintain balance of fighters with more fire points they will fire faster than ships with fewer points
		}else{
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
		}
		// Here is where we check if weapons subsystem is capable of firing the weapon.
		// Note that we can have partial bank firing, if the weapons subsystem is partially
		// functional, which should be cool.  		
		if ( ship_weapon_maybe_fail(shipp) && !force) {
			if ( obj == Player_obj ) {
				if ( ship_maybe_play_primary_fail_sound() ) {
				}
			}
			ship_stop_fire_primary_bank(obj, bank_to_fire);
			continue;
		}		
		

		if ( pm->n_guns > 0 ) {
			int num_slots = pm->gun_banks[bank_to_fire].num_slots;
			
			if(winfo_p->wi_flags & WIF_BEAM){		// the big change I made for fighter beams, if there beams fill out the Fire_Info for a targeting laser then fire it, for each point in the weapon bank -Bobboau
				swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)((float) winfo_p->fire_wait * 1000.0f));//doing that time scale thing on enemy fighter is just ugly with beams, especaly ones that have careful timeing
				beam_fire_info fbfire_info;				

				int points;
				if (winfo_p->b_info.beam_shots){
					if (winfo_p->b_info.beam_shots > num_slots){
						points = num_slots;
					}else{
						points = winfo_p->b_info.beam_shots;
					}
				}else{
					points = num_slots;
				}

				if ( shipp->weapon_energy < points*winfo_p->energy_consumed*flFrametime)
				{
					swp->next_primary_fire_stamp[bank_to_fire] = timestamp(swp->next_primary_fire_stamp[bank_to_fire]*2);
					if ( obj == Player_obj )
					{
						if ( ship_maybe_play_primary_fail_sound() )
						{
							// I guess they just deleted the commented HUD message here (they left
							// it in in other routines)
						}
					}
					ship_stop_fire_primary_bank(obj, bank_to_fire);
					continue;
				}			
				//tp->turret_firing_point[ssp->turret_next_fire_pos % tp->turret_num_firing_points];
				//tp->model_num, tp->turret_gun_sobj
//				shipp->beam_sys_info.turret_norm = obj->orient.vec.fvec;
//				shipp->beam_sys_info.
				shipp->beam_sys_info.turret_norm.xyz.x = 0.0f;
				shipp->beam_sys_info.turret_norm.xyz.y = 0.0f;
				shipp->beam_sys_info.turret_norm.xyz.z = 1.0f;
				shipp->beam_sys_info.model_num = sip->model_num;
				shipp->beam_sys_info.turret_gun_sobj = pm->detail[0];
				shipp->beam_sys_info.turret_num_firing_points = 1;
				shipp->beam_sys_info.turret_fov = (float)cos((winfo_p->field_of_fire != 0.0f)?winfo_p->field_of_fire:180);

//				shipp->beam_sys_info.turret_fov = 0.0f;
				shipp->fighter_beam_turret_data.disruption_timestamp = timestamp(0);
				shipp->fighter_beam_turret_data.turret_next_fire_pos = 0;
				shipp->fighter_beam_turret_data.current_hits = 1.0;
				shipp->fighter_beam_turret_data.system_info = &shipp->beam_sys_info;
				fbfire_info.target_subsys = Ai_info[shipp->ai_index].targeted_subsys;
				fbfire_info.beam_info_index = shipp->weapons.primary_bank_weapons[bank_to_fire];
				fbfire_info.beam_info_override = NULL;
				fbfire_info.shooter = &Objects[shipp->objnum];
				if (aip->target_objnum >= 0) {
					fbfire_info.target = &Objects[aip->target_objnum];
				} else {
					fbfire_info.target = NULL;
				}
				fbfire_info.turret = &shipp->fighter_beam_turret_data;
				fbfire_info.fighter_beam = true;
				fbfire_info.bank = bank_to_fire;

				for ( v = 0; v < points; v++ ){
					if(winfo_p->b_info.beam_shots){
						j = (shipp->last_fired_point[bank_to_fire]+1)%num_slots;
						shipp->last_fired_point[bank_to_fire] = j;
					}else{
						j=v;
					}
					fbfire_info.targeting_laser_offset = pm->gun_banks[bank_to_fire].pnt[j];			
					shipp->beam_sys_info.pnt = pm->gun_banks[bank_to_fire].pnt[j];
					shipp->beam_sys_info.turret_firing_point[0] = pm->gun_banks[bank_to_fire].pnt[j];
			//		winfo_p->b_info.beam_type = BEAM_TYPE_C;
//mprintf(("I am about to fire a fighter beam4\n"));
					beam_fire(&fbfire_info);
					num_fired++;
					//shipp->targeting_laser_objnum = beam_fire_targeting(&fire_info);			
				}

//mprintf(("I have fired a fighter beam, type %d\n", winfo_p->b_info.beam_type));

			}
			else	//if this isn't a fighter beam, do it normally -Bobboau
			{
//Assert (!(winfo_p->wi_flags & WIF_BEAM))


				// The energy-consumption code executes even for ballistic primaries, because
				// there may be a reason why you want to have ballistics consume energy.  Perhaps
				// you can't fire too many too quickly or they'll overheat.  If not, just set
				// the weapon's energy_consumed to 0 and it'll work just fine. - Goober5000

				// fail unless we're forcing (energy based primaries)
				if ( (shipp->weapon_energy < num_slots*winfo_p->energy_consumed) && !force)
				{
					swp->next_primary_fire_stamp[bank_to_fire] = timestamp(swp->next_primary_fire_stamp[bank_to_fire]);
					if ( obj == Player_obj )
					{
						if ( ship_maybe_play_primary_fail_sound() )
						{
							// I guess they just deleted the commented HUD message here (they left
							// it in in other routines)
						}
					}
					ship_stop_fire_primary_bank(obj, bank_to_fire);
					continue;
				}			

				int points = 0, numtimes = 1;

				// ok if this is a cycling weapon use shots as the number of points to fire from at a time
				// otherwise shots is the number of times all points will be fired (used mostly for the 'shotgun' effect)
				if (winfo_p->wi_flags2 & WIF2_CYCLE) {
					numtimes = 1;
					points = MIN(num_slots, winfo_p->shots);
				} else {
					numtimes = winfo_p->shots;
					points = num_slots;
				}

				// ballistics support for primaries - Goober5000
				if ( winfo_p->wi_flags2 & WIF2_BALLISTIC )
				{
					// Make sure this ship is set up for ballistics.
					// If you get this error, add the ballistic primaries tags to ships.tbl.
					Assert ( sip->flags & SIF_BALLISTIC_PRIMARIES );

					// If ship is being repaired/rearmed, it cannot fire ballistics
					if ( aip->ai_flags & AIF_BEING_REPAIRED )
					{
						continue;
					}

					// duplicated from the secondaries firing routine...
					// determine if there is enough ammo left to fire weapons on this bank.  As with primary
					// weapons, we might or might not check ammo counts depending on game mode, who is firing,
					// and if I am a client in multiplayer
					int check_ammo = 1;

					if ( MULTIPLAYER_CLIENT && (obj != Player_obj) )
					{
						check_ammo = 0;
					}

					// not enough ammo
					if ( check_ammo && ( swp->primary_bank_ammo[bank_to_fire] <= 0) )
					{
						if ( obj == Player_obj )
						{
							if ( ship_maybe_play_primary_fail_sound() )
							{
//								HUD_sourced_printf(HUD_SOURCE_HIDDEN, "No %s ammunition left in bank", Weapon_info[swp->primary_bank_weapons[bank_to_fire]].name);
							}
						}
						else
						{
							// TODO:  AI switch primary weapon / re-arm?
						}
						continue;
					}
					
					// deplete ammo
					if ( Weapon_energy_cheat == 0 )
					{
						swp->primary_bank_ammo[bank_to_fire] -= points;

						// make sure we don't go below zero; any such error is excusable
						// because it only happens when the bank is depleted in one shot
						if (swp->primary_bank_ammo[bank_to_fire] < 0)
						{
							swp->primary_bank_ammo[bank_to_fire] = 0;
						}
					}
				}

				// now handle the energy as usual
				// deplete the weapon reserve energy by the amount of energy used to fire the weapon				
				shipp->weapon_energy -= points * winfo_p->energy_consumed;
				
				// Mark all these weapons as in the same group
				int new_group_id = weapon_create_group_id();


//mprintf(("I am going to fire a weapon %d times, from %d points, the last point fired was %d, and that will be point %d\n",numtimes,points,shipp->last_fired_point[bank_to_fire],shipp->last_fired_point[bank_to_fire]%num_slots));
				for( w = 0; w<numtimes; w++ ){
				for ( j = 0; j < points; j++ )
				{
					int pt; //point
					if (winfo_p->wi_flags2 & WIF2_CYCLE){
						//pnt = pm->gun_banks[bank_to_fire].pnt[shipp->last_fired_point[bank_to_fire]+j%num_slots];
						pt = shipp->last_fired_point[bank_to_fire]+j%num_slots;
//mprintf(("fireing from %d\n",shipp->last_fired_point[bank_to_fire]+j%num_slots));
					}else{
						//pnt = pm->gun_banks[bank_to_fire].pnt[j];
						pt = j;
//mprintf(("fireing from %d\n",j));
					}

					int sub_shots = 1;
					polymodel *weapon_model = NULL;
					if(winfo_p->external_model_num >= 0){
						weapon_model = model_get(winfo_p->external_model_num);
						if(weapon_model->n_guns)sub_shots = weapon_model->gun_banks[0].num_slots;
					}

					for(int s = 0; s<sub_shots; s++){
						pnt = pm->gun_banks[bank_to_fire].pnt[pt];
						if(weapon_model && weapon_model->n_guns)vm_vec_add2(&pnt, &weapon_model->gun_banks[0].pnt[s]);

						vm_vec_unrotate(&gun_point, &pnt, &obj->orient);
						vm_vec_add(&firing_pos, &gun_point, &obj->pos);

						matrix firing_orient;
						if(!(sip->flags2 & SIF2_GUN_CONVERGENCE))
						{
							firing_orient = obj->orient;
						}
						else
						{
							vec3d firing_vec;
							vm_vec_unrotate(&firing_vec, &pm->gun_banks[bank_to_fire].norm[pt], &obj->orient);
							vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
						}
						// create the weapon -- the network signature for multiplayer is created inside
						// of weapon_create
						weapon_objnum = weapon_create( &firing_pos, &firing_orient, weapon, OBJ_INDEX(obj), new_group_id );
	
						weapon_set_tracking_info(weapon_objnum, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);				
	
						if (winfo_p->wi_flags & WIF_FLAK)
						{
							object *target;
							vec3d predicted_pos;
							float flak_range=(winfo_p->lifetime)*(winfo_p->max_speed);
							float range_to_target = flak_range;
							float wepstr=ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS);

							if (aip->target_objnum != -1) {
								target = &Objects[aip->target_objnum];
							} else {
								target = NULL;
							}

							if (target != NULL) {
								set_predicted_enemy_pos(&predicted_pos,obj,target,aip);
								range_to_target=vm_vec_dist(&predicted_pos, &obj->pos);
							}

							//if we have a target and its in range
							if ( (target != NULL) && (range_to_target < flak_range) )
							{
								//set flak range to range of ship
								flak_pick_range(&Objects[weapon_objnum], &firing_pos, &predicted_pos,wepstr);
							}
							else
							{
								flak_set_range(&Objects[weapon_objnum], flak_range-20);
							}
	
							if ((winfo_p->muzzle_flash>=0) && (((shipp==Player_ship) && (vm_vec_mag(&Player_obj->phys_info.vel)>=45)) || (shipp!=Player_ship)))
							{
								flak_muzzle_flash(&firing_pos,&obj->orient.vec.fvec, &obj->phys_info, swp->primary_bank_weapons[bank_to_fire]);
							}
						}
						// create the muzzle flash effect
						if((obj != Player_obj) || (sip->flags2 & SIF2_SHOW_SHIP_MODEL) || (Viewer_mode)) {
							// show the flash only if in not cockpit view, or if "show ship" flag is set
							shipfx_flash_create( obj, sip->model_num, &pnt, &obj->orient.vec.fvec, 1, weapon );
						}
	
						// maybe shudder the ship - if its me
						if((winfo_p->wi_flags & WIF_SHUDDER) && (obj == Player_obj) && !(Game_mode & GM_STANDALONE_SERVER)){
							// calculate some arbitrary value between 100
							// (mass * velocity) / 10
							game_shudder_apply(2500, (winfo_p->mass * winfo_p->max_speed) / 10.0f);
						}
	
						num_fired++;
					}
				}
				shipp->last_fired_point[bank_to_fire] = (shipp->last_fired_point[bank_to_fire] + 1) % num_slots;
				}
			}
			

			if(shipp->weapon_energy < 0.0f){
				shipp->weapon_energy = 0.0f;
			}			


			banks_fired |= (1<<bank_to_fire);				// mark this bank as fired.
		}		
		
		
		// Only play the weapon fired sound if it hasn't been played yet.  This is to 
		// avoid playing the same sound multiple times when banks are linked with the
		// same weapon.

		if (!(winfo_p->wi_flags & WIF_BEAM)){	// not a beam weapon?
			if ( sound_played != winfo_p->launch_snd ) {
				sound_played = winfo_p->launch_snd;
				if ( obj == Player_obj ) {
					if ( winfo_p->launch_snd != -1 ) {
						weapon_info *wip;
						ship_weapon *sw_pl;

						// HACK
						if(winfo_p->launch_snd == SND_AUTOCANNON_SHOT){
							snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE );
						} else {
							snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
						}
		//				snd_play( &Snds[winfo_p->launch_snd] );
	
						sw_pl = &Player_ship->weapons;
						if (sw_pl->current_primary_bank >= 0)
						{
							wip = &Weapon_info[sw_pl->primary_bank_weapons[sw_pl->current_primary_bank]];
							int force_level = (int) ((wip->armor_factor + wip->shield_factor * 0.2f) * (wip->damage * wip->damage - 7.5f) * 0.45f + 0.6f) * 10 + 2000;

							// modify force feedback for ballistics: make it stronger
							if (wip->wi_flags2 & WIF2_BALLISTIC)
								joy_ff_play_primary_shoot(force_level * 2);
							// no ballistics
							else
								joy_ff_play_primary_shoot(force_level);
						}
					}
				}else {
					if ( winfo_p->launch_snd != -1 ) {
						snd_play_3d( &Snds[winfo_p->launch_snd], &obj->pos, &View_position );
					}	
				}
			}	
		}

		shipp->was_firing_last_frame[bank_to_fire] = 1;
	}	// end for (go to next primary bank)
	
	// if multiplayer and we're client-side firing, send the packet
	// if((Game_mode & GM_MULTIPLAYER) && (Netgame.debug_flags & NETD_FLAG_CLIENT_FIRING)){
	if(Game_mode & GM_MULTIPLAYER){
		// if i'm a client, and this is not me, don't send
		if(!(MULTIPLAYER_CLIENT && (shipp != Player_ship))){
			send_NEW_primary_fired_packet( shipp, banks_fired );
		}
	}

	// post a primary fired event
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_primary_fired(obj, swp->current_primary_bank, shipp->flags & SF_PRIMARY_LINKED);
	}

   // STATS
   if (obj->flags & OF_PLAYER_SHIP) {
		// in multiplayer -- only the server needs to keep track of the stats.  Call the cool
		// function to find the player given the object *.  It had better return a valid player
		// or our internal structure as messed up.
		if( Game_mode & GM_MULTIPLAYER ) {
			if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
				int player_num;

				player_num = multi_find_player_by_object ( obj );
				Assert ( player_num != -1 );

				Net_players[player_num].m_player->stats.mp_shots_fired += num_fired;
			}
		} else {
			Player->stats.mp_shots_fired += num_fired;
		}
	}

	return num_fired;
}

void ship_start_targeting_laser(ship *shipp)
{	
	int bank0_laser = 0;
	int bank1_laser = 0;

	// determine if either of our banks have a targeting laser
	if((shipp->weapons.primary_bank_weapons[0] >= 0) && (Weapon_info[shipp->weapons.primary_bank_weapons[0]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[0]].b_info.beam_type == BEAM_TYPE_C)){
		bank0_laser = 1;
	}
	if((shipp->weapons.primary_bank_weapons[1] >= 0) && (Weapon_info[shipp->weapons.primary_bank_weapons[1]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[1]].b_info.beam_type == BEAM_TYPE_C)){
		bank1_laser = 1;
	}

	// if primary banks are linked
	if(shipp->flags & SF_PRIMARY_LINKED){
		if(bank0_laser){
			shipp->targeting_laser_bank = 0;
			return;
		} 
		if(bank1_laser){
			shipp->targeting_laser_bank = 1;
			return;
		}
	}
	// if we only have 1 bank selected
	else {
		if(bank0_laser && (shipp->weapons.current_primary_bank == 0)){
			shipp->targeting_laser_bank = 0;
			return;
		}
		if(bank1_laser && (shipp->weapons.current_primary_bank == 1)){
			shipp->targeting_laser_bank = 1;
			return;
		}
	}
}

void ship_stop_targeting_laser(ship *shipp)
{
	shipp->targeting_laser_bank = -1;
	shipp->targeting_laser_objnum = -1; // erase old laser obj num if it has any -Bobboau
}

void ship_process_targeting_lasers()
{
	fighter_beam_fire_info fire_info;
	ship_obj *so;
	ship *shipp;	
	polymodel *m;

	// interate over all ships
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// sanity checks
		if(so->objnum < 0){
			continue;
		}
		if(Objects[so->objnum].type != OBJ_SHIP){
			continue;
		}
		if(Objects[so->objnum].instance < 0){
			continue;
		}
		shipp = &Ships[Objects[so->objnum].instance];

		// if our trigger is no longer down, switch it off
		if(!(shipp->flags & SF_TRIGGER_DOWN)){
			ship_stop_targeting_laser(shipp);
			continue;
		}		

		// if we have a bank to fire - fire it
		if((shipp->targeting_laser_bank >= 0) && (shipp->targeting_laser_bank < 2)){
			// try and get the model
			m = model_get(Ship_info[shipp->ship_info_index].model_num);
			if(m == NULL){
				continue;
			}

			// fire a targeting laser
			fire_info.life_left = 0.0;					//for fighter beams
			fire_info.life_total = 0.0f;					//for fighter beams
			fire_info.warmdown_stamp = -1;				//for fighter beams
			fire_info.warmup_stamp = -1;				//for fighter beams
			fire_info.accuracy = 0.0f;
			fire_info.beam_info_index = shipp->weapons.primary_bank_weapons[(int)shipp->targeting_laser_bank];
			fire_info.beam_info_override = NULL;
			fire_info.shooter = &Objects[shipp->objnum];
			fire_info.target = NULL;
			fire_info.target_subsys = NULL;
			fire_info.turret = NULL;
			fire_info.targeting_laser_offset = m->gun_banks[shipp->targeting_laser_bank].pnt[0];			
			shipp->targeting_laser_objnum = beam_fire_targeting(&fire_info);			

			if (shipp->cloak_stage ==2)
			{
				shipfx_start_cloak(shipp,500);
			}

			// hmm, why didn't it fire?
			if(shipp->targeting_laser_objnum < 0){
				Int3();
				ship_stop_targeting_laser(shipp);
			}
		}
	}}

//	Attempt to detonate weapon last fired by *shipp.
//	Only used for weapons that support remote detonation.
//	Return true if detonated, else return false.
//	Calls weapon_hit() to detonate weapon.
//	If it's a weapon that spawns particles, those will be released.
int maybe_detonate_weapon(ship_weapon *swp, object *src)
{
	int			objnum = swp->last_fired_weapon_index;
	object		*objp;
	weapon_info	*wip;

	objp = &Objects[objnum];

	if (objp->type != OBJ_WEAPON){
		return 0;
	}

	if ((objp->instance < 0) || (objp->instance > MAX_WEAPONS)){
		return 0;
	}

	// check to make sure that the weapon to detonate still exists
	if ( swp->last_fired_weapon_signature != objp->signature ){
		return 0;
	}

	Assert(Weapons[objp->instance].weapon_info_index != -1);
	wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];

	if (wip->wi_flags & WIF_REMOTE) {

		if ((objnum >= 0) && (objnum < MAX_OBJECTS)) {
			int	weapon_sig;

			weapon_sig = objp->signature;

			if (swp->last_fired_weapon_signature == weapon_sig) {				
				weapon_detonate(objp);
				swp->last_fired_weapon_index = -1;

				/*
				if (src == Player_obj) {
					char missile_name[NAME_LENGTH];
					strcpy(missile_name, wip->name);
					end_string_at_first_hash_symbol(missile_name);
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Detonated %s!", 486), missile_name);
				}
				*/

				return 1;
			}
		}
	}

	return 0;
}

//	Maybe detonate secondary weapon that's already out.
//	Return true if we detonate it, false if not.
int ship_fire_secondary_detonate(object *obj, ship_weapon *swp)
{
	if (swp->last_fired_weapon_index != -1)
		if (timestamp_elapsed(swp->detonate_weapon_time)) {
			object	*first_objp = &Objects[swp->last_fired_weapon_index];
			if (maybe_detonate_weapon(swp, obj)) {
				//	If dual fire was set, there could be another weapon to detonate.  Scan all weapons.
				missile_obj	*mo;

				//nprintf(("AI", "Weapon %i detonated\n", first_objp-Objects));

				// check for currently locked missiles (highest precedence)
				for ( mo = GET_FIRST(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
					object	*mobjp;
					Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
					mobjp = &Objects[mo->objnum];
					if ((mobjp != first_objp) && (mobjp->parent_sig == obj->parent_sig)) {
						if (Weapon_info[Weapons[mobjp->instance].weapon_info_index].wi_flags & WIF_REMOTE) {
							//nprintf(("AI", "Also detonating weapon %i whose parent is %s\n", mobjp-Objects, Ships[Objects[mobjp->parent].instance].ship_name));
							weapon_detonate(mobjp);
						}
					}
				}
				
				return 1;
			}
		}

	return 0;
}

// Try to switch to a secondary bank that has ammo
int ship_select_next_valid_secondary_bank(ship_weapon *swp)
{
	int cycled=0;

	int ns = swp->num_secondary_banks;

	if ( ns > 1 ) {
		int i,j=swp->current_secondary_bank+1;
		for (i=0; i<ns; i++) {
			if ( j >= ns ) {
				j=0;
			}

			if ( swp->secondary_bank_ammo[j] > 0 ) {
				swp->current_secondary_bank=j;
				cycled = 1;
				break;
			}

			j++;
		}
	}

	return cycled;
}


extern void ai_maybe_announce_shockwave_weapon(object *firing_objp, int weapon_index);

//	Object *obj fires its secondary weapon, if it can.
//	If its most recently fired weapon is a remotely detonatable weapon, detonate it.
//	Returns number of weapons fired.  Note, for swarmers, returns 1 if it is allowed
//	to fire the missiles when allow_swarm is NOT set.  They don't actually get fired on a call here unless allow_swarm is set.
//	When you want to fire swarmers, you call this function with allow_swarm NOT set and frame interval
//	code comes aruond and fires it.
// allow_swarm -> default value is 0... since swarm missiles are fired over several frames,
//                need to avoid firing when normally called
int ship_fire_secondary( object *obj, int allow_swarm )
{
	int			n, weapon, j, bank, have_timeout, starting_bank_count = -1, num_fired;
	ushort		starting_sig = 0;
	ship			*shipp;
	ship_weapon *swp;
	ship_info	*sip;
	weapon_info	*wip;
	ai_info		*aip;
	polymodel	*pm;
	vec3d		missile_point, pnt, firing_pos;

	Assert( obj != NULL );

	// in the case where the server is an observer, he can fire (which would be bad) - unless we do this.
	if( obj->type == OBJ_OBSERVER ){
		return 0;
	}

	// in the case where the object is a ghost (a delayed fire packet from right before he died, for instance)
	if( (obj->type == OBJ_GHOST) || (obj->type == OBJ_NONE) ){
		return 0;
	}

	Assert( obj->type == OBJ_SHIP );
	if(obj->type != OBJ_SHIP){
		return 0;
	}
	n = obj->instance;
	Assert( n >= 0 && n < MAX_SHIPS );
	if((n < 0) || (n >= MAX_SHIPS)){
		return 0;
	}
	Assert( Ships[n].objnum == OBJ_INDEX(obj));
	if(Ships[n].objnum != OBJ_INDEX(obj)){
		return 0;
	}
	
	shipp = &Ships[n];
	swp = &shipp->weapons;
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	// if no secondary weapons are present on ship, return
	if ( swp->num_secondary_banks <= 0 ){
		return 0;
	}

	// If the secondaries have been locked, bail
	if (shipp->flags2 & SF2_SECONDARIES_LOCKED)
	{
		return 0;
	}

	// If ship is being repaired/rearmed, it cannot fire missiles
	if ( aip->ai_flags & AIF_BEING_REPAIRED ) {
		return 0;
	}

	num_fired = 0;		// tracks how many missiles actually fired

	bank = swp->current_secondary_bank;
	if ( bank < 0 ) {
		return 0;
	}

	if (swp->secondary_animation_position[bank] == MA_POS_SET) {
		if ( timestamp_elapsed(swp->secondary_animation_done_time[bank]) )
			swp->secondary_animation_position[bank] = MA_POS_READY;
		else
			return 0;
	}

	weapon = swp->secondary_bank_weapons[bank];
	Assert( (swp->secondary_bank_weapons[bank] >= 0) && (swp->secondary_bank_weapons[bank] < MAX_WEAPON_TYPES) );
	if((swp->secondary_bank_weapons[bank] < 0) || (swp->secondary_bank_weapons[bank] >= MAX_WEAPON_TYPES)){
		return 0;
	}
	wip = &Weapon_info[weapon];

	have_timeout = 0;			// used to help tell whether or not we have a timeout

	if ( MULTIPLAYER_MASTER ) {
		starting_sig = multi_get_next_network_signature( MULTI_SIG_NON_PERMANENT );
		starting_bank_count = swp->secondary_bank_ammo[bank];
	}

	if (ship_fire_secondary_detonate(obj, swp)) {
		// in multiplayer, master sends a secondary fired packet with starting signature of -1 -- indicates
		// to client code to set the detonate timer to 0.
		if ( MULTIPLAYER_MASTER ) {
			// MWA -- 4/6/98  Assert invalid since the bank count could have gone to 0.
			//Assert(starting_bank_count != 0);
			send_secondary_fired_packet( shipp, 0, starting_bank_count, 1, allow_swarm );
		}
	
		//	For all banks, if ok to fire a weapon, make it wait a bit.
		//	Solves problem of fire button likely being down next frame and
		//	firing weapon despite fire causing detonation of existing weapon.
		if (swp->current_secondary_bank >= 0) {
			if (timestamp_elapsed(swp->next_secondary_fire_stamp[bank])){
				swp->next_secondary_fire_stamp[bank] = timestamp(MAX((int) flFrametime*3000, 250));
			}
		}
		return 0;
	}

	if ( swp->current_secondary_bank < 0 ){
		return 0;
	}

	if ( !timestamp_elapsed(swp->next_secondary_fire_stamp[bank]) && !allow_swarm) {
		if (timestamp_until(swp->next_secondary_fire_stamp[bank]) > 60000){
			swp->next_secondary_fire_stamp[bank] = timestamp(1000);
		}
		have_timeout = 1;
		goto done_secondary;
	}

	// Ensure if this is a "require-lock" missile, that a lock actually exists
	if ( wip->wi_flags & WIF_NO_DUMBFIRE ) {
		if ( aip->current_target_is_locked <= 0 ) {
			if ( obj == Player_obj ) {			
				if ( !Weapon_energy_cheat ) {
					float max_dist;

					max_dist = wip->lifetime * wip->max_speed;
					if (wip->wi_flags2 & WIF2_LOCAL_SSM){
						max_dist= wip->lssm_lock_range;
					}

					if ((aip->target_objnum != -1) && (vm_vec_dist_quick(&obj->pos, &Objects[aip->target_objnum].pos) > max_dist)) {
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Too far from target to acquire lock", 487));
					} else {
						char missile_name[NAME_LENGTH];
						strcpy(missile_name, wip->name);
						end_string_at_first_hash_symbol(missile_name);
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Cannot fire %s without a lock", 488), missile_name);
					}

					snd_play( &Snds[SND_OUT_OF_MISSLES] );
					swp->next_secondary_fire_stamp[bank] = timestamp(800);	// to avoid repeating messages
					return 0;
				}
			} else {
				// multiplayer clients should always fire the weapon here, so return only if not
				// a multiplayer client.
				if ( !MULTIPLAYER_CLIENT ) {
					return 0;
				}
			}
		}
	}

	if (wip->wi_flags2 & WIF2_TAGGED_ONLY)
	{
		if (!ship_is_tagged(&Objects[aip->target_objnum]))
		{
			if (obj==Player_obj)
			{
				if ( !Weapon_energy_cheat )
				{
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("Cannot fire %s if target is not tagged"),wip->name);
					snd_play( &Snds[SND_OUT_OF_MISSLES] );
					swp->next_secondary_fire_stamp[bank] = timestamp(800);	// to avoid repeating messages
					return 0;
				}
			}
			else
			{
				if ( !MULTIPLAYER_CLIENT )
				{
					return 0;
				}
			}
		}
	}




	// if trying to fire a swarm missile, make sure being called from right place
	if ( (wip->wi_flags & WIF_SWARM) && !allow_swarm ) {
		Assert(wip->swarm_count > 0);
		if(wip->swarm_count <= 0){
			shipp->num_swarm_missiles_to_fire += SWARM_DEFAULT_NUM_MISSILES_FIRED;
		} else {
			shipp->num_swarm_missiles_to_fire += wip->swarm_count;
		}
		return 1;		//	Note: Missiles didn't get fired, but the frame interval code will fire them.
	}

	// if trying to fire a corkscrew missile, make sure being called from right place	
	if ( (wip->wi_flags & WIF_CORKSCREW) && !allow_swarm ) {
		//phreak 11-9-02 
		//changed this from 4 to custom number defined in tables
		shipp->num_corkscrew_to_fire = (ubyte)(shipp->num_corkscrew_to_fire + (ubyte)wip->cs_num_fired);		
		return 1;		//	Note: Missiles didn't get fired, but the frame interval code will fire them.
	}	

	swp->next_secondary_fire_stamp[bank] = timestamp((int)(Weapon_info[weapon].fire_wait * 1000.0f));	// They can fire 5 times a second

	// Here is where we check if weapons subsystem is capable of firing the weapon.
	// do only in single plyaer or if I am the server of a multiplayer game
	if ( !(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER ) {
		if ( ship_weapon_maybe_fail(shipp) ) {
			if ( obj == Player_obj ) 
				if ( ship_maybe_play_secondary_fail_sound(wip) ) {
					char missile_name[NAME_LENGTH];
					strcpy(missile_name, Weapon_info[weapon].name);
					end_string_at_first_hash_symbol(missile_name);
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Cannot fire %s due to weapons system damage", 489), missile_name);
				}
			goto done_secondary;
		}
	}

	pm = model_get( sip->model_num );
	if ( pm->n_missiles > 0 ) {
		int check_ammo;		// used to tell if we should check ammo counts or not
		int num_slots;

		if ( bank > pm->n_missiles ) {
			nprintf(("WARNING","WARNING ==> Tried to fire bank %d, but ship has only %d banks\n", bank+1, pm->n_missiles));
			return 0;		// we can make a quick out here!!!
		}

		num_slots = pm->missile_banks[bank].num_slots;

		// determine if there is enough ammo left to fire weapons on this bank.  As with primary
		// weapons, we might or might not check ammo counts depending on game mode, who is firing,
		// and if I am a client in multiplayer
		check_ammo = 1;

		if ( MULTIPLAYER_CLIENT && (obj != Player_obj) ){
			check_ammo = 0;
		}

		if ( check_ammo && ( swp->secondary_bank_ammo[bank] <= 0) ) {
			if ( shipp->objnum == OBJ_INDEX(Player_obj) ) {
				if ( ship_maybe_play_secondary_fail_sound(wip) ) {
//					HUD_sourced_printf(HUD_SOURCE_HIDDEN, "No %s missiles left in bank", Weapon_info[swp->secondary_bank_weapons[bank]].name);
				}
			}
			else {
				// TODO:  AI switch secondary weapon / re-arm?
			}
			goto done_secondary;
		}

		int start_slot, end_slot;

		if ( shipp->flags & SF_SECONDARY_DUAL_FIRE ) {
			start_slot = swp->secondary_next_slot[bank];
			// AL 11-19-97: Ensure enough ammo remains when firing linked secondary weapons
			if ( check_ammo && (swp->secondary_bank_ammo[bank] < 2) ) {
				end_slot = start_slot;
			} else {
				end_slot = start_slot+1;
			}
		} else {
			start_slot = swp->secondary_next_slot[bank];
			end_slot = start_slot;
		}

		int pnt_index=start_slot;
		for ( j = start_slot; j <= end_slot; j++ ) {
			int	weapon_num;

			swp->secondary_next_slot[bank]++;
			if ( swp->secondary_next_slot[bank] > (num_slots-1) ){
				swp->secondary_next_slot[bank] = 0;
			}

			if ( pnt_index >= num_slots ){
				pnt_index = 0;
			}
			shipp->secondary_point_reload_pct[bank][pnt_index] = 0.0f;
			pnt = pm->missile_banks[bank].pnt[pnt_index++];
			vm_vec_unrotate(&missile_point, &pnt, &obj->orient);
			vm_vec_add(&firing_pos, &missile_point, &obj->pos);

			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Weapon_info[weapon].subtype == WP_MISSILE );
			}

			matrix firing_orient;
			if(!(sip->flags2 & SIF2_GUN_CONVERGENCE))
			{
				firing_orient = obj->orient;
			}
			else
			{
				vec3d firing_vec;
				vm_vec_unrotate(&firing_vec, &pm->missile_banks[bank].norm[pnt_index-1], &obj->orient);
				vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
			}

			// create the weapon -- for multiplayer, the net_signature is assigned inside
			// of weapon_create
			weapon_num = weapon_create( &firing_pos, &firing_orient, weapon, OBJ_INDEX(obj), -1, aip->current_target_is_locked);
			weapon_set_tracking_info(weapon_num, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);


			// create the muzzle flash effect
			if((obj != Player_obj) || (sip->flags2 & SIF2_SHOW_SHIP_MODEL) || (Viewer_mode)) {
				// show the flash only if in not cockpit view, or if "show ship" flag is set
				shipfx_flash_create(obj, sip->model_num, &pnt, &obj->orient.vec.fvec, 0, weapon);
			}
/*
			if ( weapon_num != -1 )
				Demo_fire_secondary_requests++;	// testing for demo
*/
			num_fired++;
			swp->last_fired_weapon_index = weapon_num;
			swp->detonate_weapon_time = timestamp(500);		//	Can detonate 1/2 second later.
			if (weapon_num != -1) {
				swp->last_fired_weapon_signature = Objects[weapon_num].signature;
			}

			// subtract the number of missiles fired
			if ( Weapon_energy_cheat == 0 ){
			//	else
			//	{
					swp->secondary_bank_ammo[bank]--;
			//	}
			}
		}
	}

	if ( obj == Player_obj ) {
		if ( Weapon_info[weapon].launch_snd != -1 ) {
			weapon_info *wip;
			ship_weapon *swp;

			snd_play( &Snds[Weapon_info[weapon].launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
			swp = &Player_ship->weapons;
			if (swp->current_secondary_bank >= 0) {
				wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];
				if (Player_ship->flags & SF_SECONDARY_DUAL_FIRE){
					joy_ff_play_secondary_shoot((int) (wip->cargo_size * 2.0f));
				} else {
					joy_ff_play_secondary_shoot((int) wip->cargo_size);
				}
			}
		}

	} else {
		if ( Weapon_info[weapon].launch_snd != -1 ) {
			snd_play_3d( &Snds[Weapon_info[weapon].launch_snd], &obj->pos, &View_position );
		}
	}

done_secondary:

	if(num_fired > 0){
		// if I am the master of a multiplayer game, send a secondary fired packet along with the
		// first network signatures for the newly created weapons.  if nothing got fired, send a failed
		// packet if 
		if ( MULTIPLAYER_MASTER ) {			
			Assert(starting_sig != 0);
			send_secondary_fired_packet( shipp, starting_sig, starting_bank_count, num_fired, allow_swarm );			
		}

		// STATS
		if (obj->flags & OF_PLAYER_SHIP) {
			// in multiplayer -- only the server needs to keep track of the stats.  Call the cool
			// function to find the player given the object *.  It had better return a valid player
			// or our internal structure as messed up.
			if( Game_mode & GM_MULTIPLAYER ) {
				if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
					int player_num;

					player_num = multi_find_player_by_object ( obj );
					Assert ( player_num != -1 );

					Net_players[player_num].m_player->stats.ms_shots_fired += num_fired;
				}				
			} else {
				Player->stats.ms_shots_fired += num_fired;
			}
		}

		if ((shipp->cloak_stage > 0) && (shipp->cloak_stage < 3))
		{
			shipfx_start_cloak(shipp,500);
		}
		
		// maybe announce a shockwave weapon
		ai_maybe_announce_shockwave_weapon(obj, weapon);
	}

	// AL 3-7-98: Move to next valid secondary bank if out of ammo
	//

	//21-07-02 01:24 DTP; COMMENTED OUT some of the mistakes
	//this bug was made by AL, when he assumed he had to take the next fire_wait time remaining and add 250 ms of delay to it, 
	//and put it in the next valid bank. for the player to have a 250 ms of penalty
	//
	//what that caused was that the next valid bank inherited the current valid banks FULL fire delay. since he used the Weapon_info struct that has
	// no information / member that stores the next valids banks remaning fire_wait delay.
	//
	//what he should have done was to check of the next valid bank had any fire delay that had elapsed, if it had elapsed, 
	//then it would have no firedelay. and then add 250 ms of delay. in effect, this way there is no penalty if there is any firedelay remaning in
	//the next valid bank. the delay is there to prevent things like Trible/Quad Fire Trebuchets.
	//
	if ( (obj->flags & OF_PLAYER_SHIP) && (swp->secondary_bank_ammo[bank] <= 0) ) {
		//int fire_wait = (int)(Weapon_info[weapon].fire_wait * 1000.0f);	//DTP commented out, mistake, takes our current firewait time for our current weapon, it should have been our next valid weapon, but the weapon_info contains no Var for NEXT valid bank
		if ( ship_select_next_valid_secondary_bank(swp) ) {			//DTP here we switch to the next valid bank, but we cant call weapon_info on next fire_wait
			//swp->next_secondary_fire_stamp[swp->current_secondary_bank] = MAX(timestamp(250),timestamp(fire_wait));	//	1/4 second delay until can fire	//DTP, Commented out mistake, here AL put the wroung firewait into the correct next_firestamp
			if ( timestamp_elapsed(shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank]) ) {	//DTP, this is simply a copy of the manual cycle functions
				shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(1000);	//Bumped from 250 to 1000 because some people seem to be to triggerhappy :).
			}
						
			if ( obj == Player_obj ) {
				snd_play( &Snds[SND_SECONDARY_CYCLE] );		
			}
		}

	}	

	return num_fired;
}

// Goober5000
int primary_out_of_ammo(ship_weapon *swp, int bank)
{
	// true if both ballistic and ammo <= 0,
	// false if not ballistic or if ballistic and ammo > 0
			
	if ( Weapon_info[swp->primary_bank_weapons[bank]].wi_flags2 & WIF2_BALLISTIC )
	{
		if (swp->primary_bank_ammo[bank] <= 0)
		{
			return 1;
		}
	}

	// note: never out of ammo if not ballistic
	return 0;
}

// ------------------------------------------------------------------------------
// ship_select_next_primary()
//
//	Return true if a new index gets selected.
//
// parameters:		objp      => pointer to object for ship cycling primary
//                direction => forward == CYCLE_PRIMARY_NEXT, backward == CYCLE_PRIMARY_PREV
//
// NOTE: This code can be called for any arbitrary ship.  HUD messages and sounds are only used
//       for the player ship.
int ship_select_next_primary(object *objp, int direction)
{
	ship	*shipp;
	ship_info *sip;
	ship_weapon *swp;
	int new_bank;
	int original_bank;
	unsigned int original_link_flag;
	int i;

	Assert(objp != NULL);
	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS);

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];
	swp = &shipp->weapons;

	Assert(direction == CYCLE_PRIMARY_NEXT || direction == CYCLE_PRIMARY_PREV);

	original_bank = swp->current_primary_bank;
	original_link_flag = shipp->flags & SF_PRIMARY_LINKED;

	// redid case structure to possibly add more primaries in the future - Goober5000
	if ( swp->num_primary_banks == 0 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no primary weapons", 490));
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_primary_banks == 1 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has only one primary weapon: %s", 491),Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].name, swp->current_primary_bank + 1);
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_primary_banks > MAX_SHIP_PRIMARY_BANKS )
	{
		Int3();
		return 0;
	}
	else
	{
		Assert((swp->current_primary_bank >= 0) && (swp->current_primary_bank < swp->num_primary_banks));

		// first check if linked
		if ( shipp->flags & SF_PRIMARY_LINKED )
		{
			shipp->flags &= ~SF_PRIMARY_LINKED;
			if ( direction == CYCLE_PRIMARY_NEXT )
			{
				swp->current_primary_bank = 0;
			}
			else
			{
				swp->current_primary_bank = swp->num_primary_banks - 1;
			}
		}
		// now handle when not linked: cycle and constrain
		else
		{
			if ( direction == CYCLE_PRIMARY_NEXT )
			{
				if ( swp->current_primary_bank < swp->num_primary_banks - 1 )
				{
					swp->current_primary_bank++;
				}
				else
				{
					shipp->flags |= SF_PRIMARY_LINKED;
				}
			}
			else
			{
				if ( swp->current_primary_bank > 0 )
				{
					swp->current_primary_bank--;
				}
				else
				{
					shipp->flags |= SF_PRIMARY_LINKED;
				}
			}
		}
	}

	// test for ballistics - Goober5000
	if ( sip->flags & SIF_BALLISTIC_PRIMARIES )
	{
		// if we can't link, disengage primary linking and change to next available bank
		if (shipp->flags & SF_PRIMARY_LINKED)
		{
			for (i = 0; i < swp->num_primary_banks; i++)
			{
				if (primary_out_of_ammo(swp, i))
				{
					shipp->flags &= ~SF_PRIMARY_LINKED;
					
					if (direction == CYCLE_PRIMARY_NEXT)
					{
						swp->current_primary_bank = 0;
					}
					else
					{
						swp->current_primary_bank = shipp->weapons.num_primary_banks-1;
					}
					break;
				}
			}
		}

		// check to see if we keep cycling...we have to if we're out of ammo in the current bank
		if ( primary_out_of_ammo(swp, swp->current_primary_bank) )
		{
			// cycle around until we find ammunition...
			// we land on the original bank if all banks fail
			Assert(swp->current_primary_bank < swp->num_primary_banks);
			new_bank = swp->current_primary_bank;

			for (i = 1; i < swp->num_primary_banks; i++)
			{
				// cycle in the proper direction
				if ( direction == CYCLE_PRIMARY_NEXT )
				{
					new_bank = (swp->current_primary_bank + i) % swp->num_primary_banks;
				}
				else
				{
					new_bank = (swp->current_primary_bank + swp->num_primary_banks - i) % swp->num_primary_banks;
				}

				// check to see if this is a valid bank
				if (!primary_out_of_ammo(swp, new_bank))
				{
					break;
				}
			}
			// set the new bank; defaults to resetting to the old bank if we completed a full iteration
			swp->current_primary_bank = new_bank;
		}
		
		// make sure we're okay
		Assert((swp->current_primary_bank >= 0) && (swp->current_primary_bank < swp->num_primary_banks));

		// if this ship is ballistics-equipped, and we cycled, then we had to verify some stuff,
		// so we should check if we actually changed banks
		if ( (swp->current_primary_bank != original_bank) || ((shipp->flags & SF_PRIMARY_LINKED) != original_link_flag) )
		{
			if ( objp == Player_obj )
			{
				snd_play( &Snds[SND_PRIMARY_CYCLE], 0.0f );
			}
			ship_primary_changed(shipp);
			return 1;
		}

		// could not select new weapon:
		if ( objp == Player_obj )
		{
			gamesnd_play_error_beep();
		}
		return 0;
	}	// end of ballistics implementation

	if ( objp == Player_obj )
	{
		snd_play( &Snds[SND_PRIMARY_CYCLE], 0.0f );
	}

	ship_primary_changed(shipp);
	return 1;
}

// ------------------------------------------------------------------------------
// ship_select_next_secondary() selects the next secondary bank with missles
//
//	returns:		1	=> The secondary bank was switched
//					0	=> The secondary bank stayed the same
//
// If a secondary bank has no missles left, it is skipped.
//
// NOTE: This can be called for an arbitrary ship.  HUD messages and sounds are only used
//			for the player ship.
int ship_select_next_secondary(object *objp)
{
	Assert(objp != NULL);
	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS);

	int	original_bank, new_bank, i;
	ship	*shipp;
	ship_weapon *swp;

	shipp = &Ships[objp->instance];
	swp = &shipp->weapons;

	// redid the switch structure to allow additional seconary banks if added later - Goober5000
	if ( swp->num_secondary_banks == 0 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no secondary weapons", 492));
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_secondary_banks == 1 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has only one secondary weapon: %s", 493), Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]].name, swp->current_secondary_bank + 1);
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_secondary_banks > MAX_SHIP_SECONDARY_BANKS )
	{
		Int3();
		return 0;
	}
	else
	{
		Assert((swp->current_secondary_bank >= 0) && (swp->current_secondary_bank < swp->num_secondary_banks));

		original_bank = swp->current_secondary_bank;

		for ( i = 1; i < swp->num_secondary_banks; i++ ) {
			new_bank = (swp->current_secondary_bank+i) % swp->num_secondary_banks;
			if ( swp->secondary_bank_ammo[new_bank] <= 0 )
				continue;
			swp->current_secondary_bank = new_bank;
			break;
		}

		if ( swp->current_secondary_bank != original_bank )
		{
			if ( objp == Player_obj )
			{
				snd_play( &Snds[SND_SECONDARY_CYCLE], 0.0f );
			}
			ship_secondary_changed(shipp);
			return 1;
		}
	} // end if

	// If we've reached this point, must have failed
	if ( objp == Player_obj )
	{
		gamesnd_play_error_beep();
	}
	return 0;
}

// Goober5000 - copied from secondary routine
//	Stuff list of weapon indices for object *objp in list *outlist.
//	Return number of weapons in list.
int get_available_primary_weapons(object *objp, int *outlist, int *outbanklist)
{
	int	count = 0;
	int	i;
	ship	*shipp;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	shipp = &Ships[objp->instance];

	for (i=0; i<shipp->weapons.num_primary_banks; i++)
	{
		if (!primary_out_of_ammo(&(shipp->weapons), i))
		{
			outbanklist[count] = i;
			outlist[count++] = shipp->weapons.primary_bank_weapons[i];
		}
	}

	return count;
}

//	Stuff list of weapon indices for object *objp in list *outlist.
//	Return number of weapons in list.
int get_available_secondary_weapons(object *objp, int *outlist, int *outbanklist)
{
	int	count = 0;
	int	i;
	ship	*shipp;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	shipp = &Ships[objp->instance];

	for (i=0; i<shipp->weapons.num_secondary_banks; i++)
		if (shipp->weapons.secondary_bank_ammo[i]) {
			outbanklist[count] = i;
			outlist[count++] = shipp->weapons.secondary_bank_weapons[i];
		}

	return count;
}

//	Return the object index of the ship with name *name.
int wing_name_lookup(char *name, int ignore_count)
{
	int i, wing_limit;

	if (name == NULL)
		return -1;

	if ( Fred_running )
		wing_limit = MAX_WINGS;
	else
		wing_limit = Num_wings;

	if (Fred_running || ignore_count ) {  // current_count not used for Fred..
		for (i=0; i<wing_limit; i++)
			if (Wings[i].wave_count && !stricmp(Wings[i].name, name))
				return i;

	} else {
		for (i=0; i<wing_limit; i++)
			if (Wings[i].current_count && !stricmp(Wings[i].name, name))
				return i;
	}

	return -1;
}

// this function is needed in addition to wing_name_lookup because it does a straight lookup without
// caring about how many ships are in the wing, etc.
int wing_lookup(char *name)
{
   int idx;
	for(idx=0;idx<Num_wings;idx++)
		if(strcmp(Wings[idx].name,name)==0)
		   return idx;

	return -1;
}

//	Return the index of Ship_info[].name that is *token.
int ship_info_lookup_sub(char *token)
{
	int	i;

	for (i = 0; i < Num_ship_classes; i++)
		if (!stricmp(token, Ship_info[i].name))
			return i;

	return -1;
}

// Return the index of Ship_templates[].name that is *token.
int ship_template_lookup(char *token)
{
	int	i;

	for ( i = 0; i < Ship_templates.size(); i++ ) {
		if ( !stricmp(token, Ship_templates[i].name) ) {
			return i;
		}
	}
	return -1;
}

// Goober5000
int ship_info_lookup(char *token)
{
	int idx;
	char *p;
	char name[NAME_LENGTH], temp1[NAME_LENGTH], temp2[NAME_LENGTH];

	// bogus
	if (token == NULL)
		return -1;

	// first try a straightforward lookup
	idx = ship_info_lookup_sub(token);
	if (idx >= 0)
		return idx;

	// ship copy types might be mismatched
	p = get_pointer_to_first_hash_symbol(token);
	if (p == NULL)
		return -1;

	// conversion from FS1 missions
	if (!stricmp(token, "GTD Orion#1 (Galatea)"))
	{
		idx = ship_info_lookup_sub("GTD Orion#Galatea");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("GTD Orion (Galatea)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "GTD Orion#2 (Bastion)"))
	{
		idx = ship_info_lookup_sub("GTD Orion#Bastion");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("GTD Orion (Bastion)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "SF Dragon#2 (weakened)"))
	{
		idx = ship_info_lookup_sub("SF Dragon#weakened");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("SF Dragon (weakened)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "SF Dragon#3 (Player)"))
	{
		idx = ship_info_lookup_sub("SF Dragon#Terrans");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("SF Dragon (Terrans)");
		if (idx >= 0)
			return idx;

		return -1;
	}

	// get first part of new string
	strcpy(temp1, token);
	end_string_at_first_hash_symbol(temp1);

	// get second part
	strcpy(temp2, p + 1);

	// found a hash
	if (*p == '#')
	{
		// assemble using parentheses
		sprintf(name, "%s (%s)", temp1, temp2);
	}
	// found a parenthesis
	else if (*p == '(')
	{
		// chop off right parenthesis (it exists because otherwise the left wouldn't have been flagged)
		char *p2 = strchr(temp2, ')');
		*p2 = '\0';

		// assemble using hash
		sprintf(name, "%s#%s", temp1, temp2);
	}
	// oops
	else
	{
		Warning(LOCATION, "Unrecognized hash symbol.  Contact a programmer!");
		return -1;
	}

	// finally check the new name
	return ship_info_lookup_sub(name);
}

//	Return the ship index of the ship with name *name.
int ship_name_lookup(char *name, int inc_players)
{
	int	i;

	// bogus
	if(name == NULL){
		return -1;
	}

	for (i=0; i<MAX_SHIPS; i++){
		if (Ships[i].objnum >= 0){
			if (Objects[Ships[i].objnum].type == OBJ_SHIP || (Objects[Ships[i].objnum].type == OBJ_START && inc_players)){
				if (!stricmp(name, Ships[i].ship_name)){
					return i;
				}
			}
		}
	}
	
	// couldn't find it
	return -1;
}

int ship_type_name_lookup(char *name)
{
	// bogus
	if(name == NULL || !strlen(name)){
		return -1;
	}

	//Look through Ship_types array
	uint max_size = Ship_types.size();
	for(uint idx=0; idx < max_size; idx++){
		if(!stricmp(name, Ship_types[idx].name)){
			return idx;
		}
	}
	// couldn't find it
	return -1;
}

// checks the (arrival & departure) state of a ship.  Return values:
// -1: has yet to arrive in mission
//  0: is currently in mission
//  1: has been destroyed, departed, or never existed
int ship_query_state(char *name)
{
	int i;

	// bogus
	if(name == NULL){
		return -1;
	}

	for (i=0; i<MAX_SHIPS; i++){
		if (Ships[i].objnum >= 0){
			if ((Objects[Ships[i].objnum].type == OBJ_SHIP) || (Objects[Ships[i].objnum].type == OBJ_START)){
				if (!stricmp(name, Ships[i].ship_name)){
					return 0;
				}
			}
		}
	}

	if (mission_parse_get_arrival_ship(name))
		return -1;

	return 1;
}

//	Note: This is not a general purpose routine.
//	It is specifically used for targeting.
//	It only returns a subsystem position if it has shields.
//	Return true/false for subsystem found/not found.
//	Stuff vector *pos with absolute position.
// subsysp is a pointer to the subsystem.
int get_subsystem_pos(vec3d *pos, object *objp, ship_subsys *subsysp)
{
	model_subsystem	*psub;
	vec3d	pnt;
	ship		*shipp;

	Assert(objp->type == OBJ_SHIP);
	shipp = &Ships[objp->instance];

	Assert ( subsysp != NULL );

	psub = subsysp->system_info;

	vm_vec_unrotate(&pnt, &psub->pnt, &objp->orient);
	vm_vec_add2(&pnt, &objp->pos);

	if ( pos ){
		*pos = pnt;
	}

	return 1;
}

//=================================================
// Takes all the angle info from the ship structure and stuffs it
// into the model data so that the model code has all the correct
// angles and stuff that it needs.    This is a poorly designed 
// system that should be re-engineered so that all the model functions
// accept a list of angles and everyone passes them through, but
// that would require some major code revision.
// So, anytime you are using a model that has rotating parts, you
// need to do a ship_model_start before any model_ functions are
// called and a ship_model_stop after you're done.   Even for 
// collision detection and stuff, not just rendering.
// See John for details.

void ship_model_start(object *objp)
{
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;
	int model_num;

	shipp = &Ships[objp->instance];
	model_num = Ship_info[shipp->ship_info_index].model_num;

	// First clear all the angles in the model to zero
	model_clear_instance(model_num);

	// Go through all subsystems and bash the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
			case SUBSYSTEM_RADAR:
			case SUBSYSTEM_NAVIGATION:
			case SUBSYSTEM_COMMUNICATION:
			case SUBSYSTEM_UNKNOWN:
			case SUBSYSTEM_ENGINE:
			case SUBSYSTEM_SENSORS:
			case SUBSYSTEM_WEAPONS:
			case SUBSYSTEM_SOLAR:
			case SUBSYSTEM_GAS_COLLECT:
			case SUBSYSTEM_ACTIVATION:
			case SUBSYSTEM_SHIELD_GENERATOR:
				break;
			case SUBSYSTEM_TURRET:
				Assert( !(psub->flags & MSS_FLAG_ROTATES) ); // Turrets can't rotate!!! See John!
				break;
			default:
				Error(LOCATION, "Illegal subsystem type.\n");
		}

		if ( psub->subobj_num >= 0 )	{
			model_set_instance(model_num, psub->subobj_num, &pss->submodel_info_1 );
		}

		if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >= 0) )		{
			model_set_instance(model_num, psub->turret_gun_sobj, &pss->submodel_info_2 );
		}

	}
}

//==========================================================
// Clears all the instance specific stuff out of the model info
void ship_model_stop(object *objp)
{
	Assert(objp != NULL);
	Assert(objp->instance >= 0);
	Assert(objp->type == OBJ_SHIP);

	// Then, clear all the angles in the model to zero
	model_clear_instance(Ship_info[Ships[objp->instance].ship_info_index].model_num);
}


//==========================================================
// Finds the number of crew points in a ship
int ship_find_num_crewpoints(object *objp)
{
	int n = 0;
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;

	shipp = &Ships[objp->instance];

	// Go through all subsystems and record the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
		case SUBSYSTEM_TURRET:
			if ( psub->flags & MSS_FLAG_CREWPOINT )
				n++; // fall through

		case SUBSYSTEM_RADAR:
		case SUBSYSTEM_NAVIGATION:
		case SUBSYSTEM_COMMUNICATION:
		case SUBSYSTEM_UNKNOWN:
		case SUBSYSTEM_ENGINE:
		case SUBSYSTEM_GAS_COLLECT:
		case SUBSYSTEM_ACTIVATION:
		case SUBSYSTEM_SHIELD_GENERATOR:
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}
	}
	return n;
}

//==========================================================
// Finds the number of turrets in a ship
int ship_find_num_turrets(object *objp)
{
	int n = 0;
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;

	shipp = &Ships[objp->instance];

	// Go through all subsystems and record the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
		case SUBSYSTEM_TURRET:
			n++; // drop through

		case SUBSYSTEM_RADAR:
		case SUBSYSTEM_NAVIGATION:
		case SUBSYSTEM_COMMUNICATION:
		case SUBSYSTEM_UNKNOWN:
		case SUBSYSTEM_ENGINE:
		case SUBSYSTEM_GAS_COLLECT:
		case SUBSYSTEM_ACTIVATION:
		case SUBSYSTEM_SHIELD_GENERATOR:
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}
	}
	return n;
}

//	Modify the matrix orient by the slew angles a.
void compute_slew_matrix(matrix *orient, angles *a)
{
	matrix	tmp, tmp2;
	angles	t1, t2;

	t1 = t2 = *a;
	t1.h = 0.0f;	t1.b = 0.0f;
	t2.p = 0.0f;	t2.b = 0.0f;

	// put in p & b like normal
	vm_angles_2_matrix(&tmp, &t1 );
	vm_matrix_x_matrix( &tmp2, orient, &tmp);

	// Put in heading separately
	vm_angles_2_matrix(&tmp, &t2 );
	vm_matrix_x_matrix( orient, &tmp2, &tmp );

	vm_orthogonalize_matrix(orient);
}

// calculates the eye position for this ship in the global reference frame.  Uses the
// view_positions array in the model.  The 0th element is the normal viewing position.
// the vector of the eye is returned in the parameter 'eye'.  The orientation of the
// eye is returned in orient.  (NOTE: this is kind of bogus for now since non 0th element
// eyes have no defined up vector)
void ship_get_eye( vec3d *eye_pos, matrix *eye_orient, object *obj )
{
	polymodel *pm = model_get(Ship_info[Ships[obj->instance].ship_info_index].model_num);
	eye *ep = &(pm->view_positions[Ships[obj->instance].current_viewpoint]);
	// vec3d vec;

	// check to be sure that we have a view eye to look at.....spit out nasty debug message
	if ( pm->n_view_positions == 0 ) {
//		nprintf (("Warning", "No eye position found for model %s.  Find artist to get fixed.\n", pm->filename ));
		*eye_pos = obj->pos;
		*eye_orient = obj->orient;
		return;
	}

	// eye points are stored in an array -- the normal viewing position for a ship is the current_eye_index
	// element.
	model_find_world_point( eye_pos, &ep->pnt, pm->id, ep->parent, &obj->orient, &obj->pos );
	// if ( shipp->current_eye_index == 0 ) {
		//vm_vec_scale_add(eye_pos, &viewer_obj->pos, &tm.vec.fvec, 2.0f * viewer_obj->radius + Viewer_external_info.distance);
		*eye_orient = obj->orient;
	//} else {
	// 	model_find_world_dir( &vec, &ep->norm, pm->id, ep->parent, &obj->orient, &obj->pos );
		// kind of bogus, but use the objects uvec to avoid totally stupid looking behavior.
	//	vm_vector_2_matrix(eye_orient,&vec,&obj->orient.uvec,NULL);
	//}

	//	Modify the orientation based on head orientation.
	if ( Viewer_obj == obj ) {
		if ( Viewer_mode & VM_PADLOCK_ANY ) {
			player_get_padlock_orient(eye_orient);
		} else {
			compute_slew_matrix(eye_orient, &Viewer_slew_angles);
		}
	}
}

// of attackers to make this decision.
//
// NOTE: This function takes into account how many ships are attacking a subsystem, and will 
//			prefer an ignored subsystem over a subsystem that is in line of sight, if the in-sight
//			subsystem is attacked by more than MAX_SUBSYS_ATTACKERS
// input:
//				sp					=>		ship pointer to parent of subsystem
//				subsys_type		=>		what kind of subsystem this is
//				attacker_pos	=>		the world coords of the attacker of this subsystem
//
// returns: pointer to subsystem if one found, NULL otherwise
#define MAX_SUBSYS_ATTACKERS 3
ship_subsys *ship_get_best_subsys_to_attack(ship *sp, int subsys_type, vec3d *attacker_pos)
{
	ship_subsys	*ss;
	ship_subsys *best_in_sight_subsys, *lowest_attacker_subsys, *ss_return;
	int			lowest_num_attackers, lowest_in_sight_attackers, num_attackers;
	vec3d		gsubpos;
	ship_obj		*sop;

	lowest_in_sight_attackers = lowest_num_attackers = 1000;
	ss_return = best_in_sight_subsys = lowest_attacker_subsys = NULL;

	for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
		if ( (ss->system_info->type == subsys_type) && (ss->current_hits > 0) ) {

			// get world pos of subsystem
			vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &Objects[sp->objnum].orient);
			vm_vec_add2(&gsubpos, &Objects[sp->objnum].pos);
			
			// now find the number of ships attacking this subsystem by iterating through the ships list,
			// and checking if aip->targeted_subsys matches the subsystem we're checking
			num_attackers = 0;
			sop = GET_FIRST(&Ship_obj_list);
			while(sop != END_OF_LIST(&Ship_obj_list)){
				if ( Ai_info[Ships[Objects[sop->objnum].instance].ai_index].targeted_subsys == ss ) {
					num_attackers++;
				}
				sop = GET_NEXT(sop);
			}

			if ( num_attackers < lowest_num_attackers ) {
				lowest_num_attackers = num_attackers;
				lowest_attacker_subsys = ss;
			}

			if ( ship_subsystem_in_sight(&Objects[sp->objnum], ss, attacker_pos, &gsubpos) ) {
				if ( num_attackers < lowest_in_sight_attackers ) {
					lowest_in_sight_attackers = num_attackers;
					best_in_sight_subsys = ss;
				}
			}
		}
	}

	if ( best_in_sight_subsys == NULL ) {
		// no subsystems are in sight, so return the subsystem with the lowest # of attackers
		ss_return =  lowest_attacker_subsys;
	} else {
		if ( lowest_in_sight_attackers > MAX_SUBSYS_ATTACKERS ) {
			ss_return = lowest_attacker_subsys;
		} else {
			ss_return =  best_in_sight_subsys;
		}
	}

	return ss_return;
}

// function to return a pointer to the 'nth' ship_subsys structure in a ship's linked list
// of ship_subsys'.
// attacker_pos	=>	world pos of attacker (default value NULL).  If value is non-NULL, try
//							to select the best subsystem to attack of that type (using line-of-sight)
//							and based on the number of ships already attacking the subsystem
ship_subsys *ship_get_indexed_subsys( ship *sp, int index, vec3d *attacker_pos )
{
	int count;
	ship_subsys *ss;

	// first, special code to see if the index < 0.  If so, we are looking for one of several possible
	// engines or one of several possible turrets.  If we enter this if statement, we will always return
	// something.
	if ( index < 0 ) {
		int subsys_type;
		
		subsys_type = -index;
		if ( sp->subsys_info[subsys_type].current_hits == 0.0f )		// if there are no hits, no subsystem to attack.
			return NULL;

		if ( attacker_pos != NULL ) {
			ss = ship_get_best_subsys_to_attack(sp, subsys_type, attacker_pos);
			return ss;
		} else {
			// next, scan the list of subsystems and search for the first subsystem of the particular
			// type which has > 0 hits remaining.
			for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
				if ( (ss->system_info->type == subsys_type) && (ss->current_hits > 0) )
					return ss;
			}
		}
		
		Int3();				// maybe we shouldn't get here, but with possible floating point rounding, I suppose we could
		return NULL;
	}


	count = 0;
	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( count == index )
			return ss;
		count++;
		ss = GET_NEXT( ss );
	}
	Int3();			// get allender -- turret ref didn't fixup correctly!!!!
	return NULL;
}

//	Given a pointer to a subsystem and an associated object, return the index.
int ship_get_index_from_subsys(ship_subsys *ssp, int objnum, int error_bypass)
{
	if (ssp == NULL)
		return -1;
	else {
		int	count;
		ship	*shipp;
		ship_subsys	*ss;

		Assert(objnum >= 0);
		Assert(Objects[objnum].instance >= 0);

		shipp = &Ships[Objects[objnum].instance];

		count = 0;
		ss = GET_FIRST(&shipp->subsys_list);
		while ( ss != END_OF_LIST( &shipp->subsys_list ) ) {
			if ( ss == ssp)
				return count;
			count++;
			ss = GET_NEXT( ss );
		}
		if ( !error_bypass )
			Int3();			// get allender -- turret ref didn't fixup correctly!!!!
		return -1;
	}
}

// function which returns the index number of the ship_subsys parameter
int ship_get_subsys_index(ship *sp, char *ss_name, int error_bypass)
{
	int count;
	ship_subsys *ss;

	count = 0;
	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( !subsystem_stricmp(ss->system_info->subobj_name, ss_name) )
			return count;
		count++;
		ss = GET_NEXT( ss );
	}

	if (!error_bypass)
		Int3();

	return -1;
}

// routine to return the strength of a subsystem.  We keep a total hit tally for all subsystems
// which are similar (i.e. a total for all engines).  These routines will return a number between
// 0.0 and 1.0 which is the relative combined strength of the given subsystem type.  The number
// calculated for the engines is slightly different.  Once an engine reaches < 15% of its hits, its
// output drops to that %.  A dead engine has no output.
float ship_get_subsystem_strength( ship *shipp, int type )
{
	float strength;
	ship_subsys *ssp;

	Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );
	if ( shipp->subsys_info[type].total_hits == 0.0f )
		return 1.0f;

	//	For a dying ship, all subsystem strengths are zero.
	if (Objects[shipp->objnum].hull_strength <= 0.0f)
		return 0.0f;

	// short circuit 0
	if (shipp->subsys_info[type].current_hits <= 0.0f)
		return 0.0f;

	strength = shipp->subsys_info[type].current_hits / shipp->subsys_info[type].total_hits;
	Assert( strength != 0.0f );

	if ( (type == SUBSYSTEM_ENGINE) && (strength < 1.0f) ) {
		float percent;

		percent = 0.0f;
		ssp = GET_FIRST(&shipp->subsys_list);
		while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {

			if ( ssp->system_info->type == SUBSYSTEM_ENGINE ) {
				float ratio;

				ratio = ssp->current_hits / ssp->max_hits;
				if ( ratio < ENGINE_MIN_STR )
					ratio = ENGINE_MIN_STR;

				percent += ratio;
			}
			ssp = GET_NEXT( ssp );
		}
		strength = percent / (float)shipp->subsys_info[type].num;
	}

	return strength;
}

// set the strength of a subsystem on a given ship.  The strength passed as a 
// parameter is between 0.0 and 1.0
//
// NOTE: this function was made to be called by the debug function dcf_set_subsys().  If
// you want to use this, be sure that you test it for all cases.
void ship_set_subsystem_strength( ship *shipp, int type, float strength )
{
	float total_current_hits, diff;
	ship_subsys *ssp;

	Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );
	if ( shipp->subsys_info[type].total_hits == 0.0f )
		return;

	total_current_hits = 0.0f;
	ssp = GET_FIRST(&shipp->subsys_list);
	while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {

		if ( ssp->system_info->type == type ) {
			ssp->current_hits = strength * ssp->max_hits;
			total_current_hits += ssp->current_hits;
		}
		ssp = GET_NEXT( ssp );
	}

	// update the objects integrity, needed since we've bashed the strength of a subsysem
	diff = total_current_hits - shipp->subsys_info[type].current_hits;
	Objects[shipp->objnum].hull_strength += diff;
	// fix up the shipp->subsys_info[type] current_hits value
	shipp->subsys_info[type].current_hits = total_current_hits;
}

#define		SHIELD_REPAIR_RATE	0.20f			//	Percent of shield repaired per second.
#define		HULL_REPAIR_RATE		0.15f			//	Percent of hull repaired per second.
#define		SUBSYS_REPAIR_RATE	0.10f			// Percent of subsystems repaired per second.

#define REARM_NUM_MISSILES_PER_BATCH 4		// how many missiles are dropped in per load sound
#define REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH	100	// how many bullets are dropped in per load sound

//calculates approximate time in seconds it would take to rearm and repair object.
float ship_calculate_rearm_duration( object *objp )
{
	ship* sp;
	ship_info* sip;
	ship_subsys* ssp;
	ship_weapon* swp;
	weapon_info* wip;

	float shield_rep_time = 0;
	float subsys_rep_time = 0;
	float hull_rep_time = 0;
	float prim_rearm_time = 0;
	float sec_rearm_time = 0;

	float max_hull_repair;
	float max_subsys_repair;

	int i;
	int num_reloads;

	bool found_first_empty;
	
	Assert(objp->type == OBJ_SHIP);

	sp = &Ships[objp->instance];
	swp = &sp->weapons;
	sip = &Ship_info[sp->ship_info_index];

	//find out time to repair shields
	shield_rep_time = (shield_get_max_strength(objp) - shield_get_strength(objp)) / (shield_get_max_strength(objp) * SHIELD_REPAIR_RATE);
	
	max_hull_repair = sp->ship_max_hull_strength * (The_mission.support_ships.max_hull_repair_val * 0.01f);
	//calculate hull_repair_time;
	if ((The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) && (max_hull_repair > objp->hull_strength))
	{
		hull_rep_time = (max_hull_repair - objp->hull_strength) / (sp->ship_max_hull_strength * HULL_REPAIR_RATE);
	}

	//caluclate subsystem repair time
	ssp = GET_FIRST(&sp->subsys_list);
	while (ssp != END_OF_LIST(&sp->subsys_list))
	{
		max_subsys_repair = ssp->max_hits * (The_mission.support_ships.max_subsys_repair_val * 0.01f);
		if (max_subsys_repair > ssp->current_hits) 
		{
			subsys_rep_time += (max_subsys_repair - ssp->current_hits) / (ssp->max_hits * HULL_REPAIR_RATE);
		}

		ssp = GET_NEXT( ssp );
	}

	//now do the primary rearm time
	found_first_empty = false;
	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			wip = &Weapon_info[swp->primary_bank_weapons[i]];
			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				//check how many full reloads we need
				num_reloads = (swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i])/REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH;

				//take into account a fractional reload
				if ((swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i]) % REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH != 0)
				{
					num_reloads++;
				}

				//don't factor in the time it takes for the first reload, since that is loaded instantly
				num_reloads--;

				if (num_reloads < 0) continue;

				if (!found_first_empty && (swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i]))
				{
					found_first_empty = true;
					prim_rearm_time += (float)snd_get_duration(Snds[SND_MISSILE_START_LOAD].id) / 1000.0f;
				}

				prim_rearm_time += num_reloads * wip->rearm_rate;
			}
		}
	}

	//and on to secondary rearm time
	found_first_empty = false;
	for (i = 0; i < swp->num_secondary_banks; i++)
	{
			wip = &Weapon_info[swp->secondary_bank_weapons[i]];
	
			//check how many full reloads we need
			num_reloads = (swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i])/REARM_NUM_MISSILES_PER_BATCH;

			//take into account a fractional reload
			if ((swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i]) % REARM_NUM_MISSILES_PER_BATCH != 0)
			{
				num_reloads++;
			}

			//don't factor in the time it takes for the first reload, since that is loaded instantly
			num_reloads--;

			if (num_reloads < 0) continue;

			if (!found_first_empty && (swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i]))
			{
				found_first_empty = true;
				sec_rearm_time += (float)snd_get_duration(Snds[SND_MISSILE_START_LOAD].id) / 1000.0f;
			}

			sec_rearm_time += num_reloads * wip->rearm_rate;
	}

	//sum them up and you've got an estimated rearm time.
	//add 1.2 to compensate for release delay
	return shield_rep_time + hull_rep_time + subsys_rep_time + prim_rearm_time + sec_rearm_time + 1.2f;
}



// ==================================================================================
// ship_do_rearm_frame()
//
// function to rearm a ship.  This function gets called from the ai code ai_do_rearm_frame (or
// some function of a similar name).  Returns 1 when ship is fully repaired and rearmed, 0 otherwise
//
int ship_do_rearm_frame( object *objp, float frametime )
{
	int			i, banks_full, primary_banks_full, subsys_type, subsys_all_ok, last_ballistic_idx = 0;
	float			shield_str, repair_delta, repair_allocated, max_hull_repair, max_subsys_repair;
	ship			*shipp;
	ship_weapon	*swp;
	ship_info	*sip;
	ship_subsys	*ssp;
	ai_info		*aip;

	shipp = &Ships[objp->instance];
	swp = &shipp->weapons;
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	// AL 10-31-97: Add missing primary weapons to the ship.  This is required since designers
	//              want to have ships that start with no primaries, but can get them through
	//					 rearm/repair
	if ( swp->num_primary_banks < sip->num_primary_banks ) {
		for ( i = swp->num_primary_banks; i < sip->num_primary_banks; i++ ) {
			swp->primary_bank_weapons[i] = sip->primary_bank_weapons[i];
		}
		swp->num_primary_banks = sip->num_primary_banks;
	}
	// AL 12-30-97: Repair broken warp drive
	if ( shipp->flags & SF_WARP_BROKEN ) {
		// TODO: maybe do something here like informing player warp is fixed?
		// like this? -- Goober5000
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Subspace drive repaired.", -1));
		shipp->flags &= ~SF_WARP_BROKEN;
	}

	// AL 1-16-97: Replenish countermeasures
	shipp->cmeasure_count = sip->cmeasure_max;

	// Do shield repair here
	if ( !(objp->flags & OF_NO_SHIELDS) )
	{
		float max_shields = shield_get_max_strength(objp);

		shield_str = shield_get_strength(objp);
		if ( shield_str < max_shields ) {
			if ( objp == Player_obj ) {
				player_maybe_start_repair_sound();
			}
			shield_str += max_shields * frametime * SHIELD_REPAIR_RATE;
			if ( shield_str > max_shields ) {
				 shield_str = max_shields;
			}
			shield_set_strength(objp, shield_str);
		}
	}

	// Repair the ship integrity (subsystems + hull).  This works by applying the repair points
	// to the subsystems.  Ships integrity is stored is objp->hull_strength, so that always is 
	// incremented by repair_allocated
	repair_allocated = shipp->ship_max_hull_strength * frametime * HULL_REPAIR_RATE;


//	AL 11-24-97: remove increase to hull integrity
//	Comments removed by PhReAk; Note that this is toggled on/off with a mission flag

	//Figure out how much of the ship's hull we can repair
	max_hull_repair = shipp->ship_max_hull_strength * (The_mission.support_ships.max_hull_repair_val * 0.01f);

	//Don't "reverse-repair" the hull if it's already above the max repair threshold
	if (objp->hull_strength > max_hull_repair)
	{
		max_hull_repair = objp->hull_strength;
	}
	
	if(The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL)
	{
		objp->hull_strength += repair_allocated;
		if ( objp->hull_strength > max_hull_repair ) {
			objp->hull_strength = max_hull_repair;
		}

		if ( objp->hull_strength > shipp->ship_max_hull_strength )
		{
			objp->hull_strength = shipp->ship_max_hull_strength;
			repair_allocated -= ( shipp->ship_max_hull_strength - objp->hull_strength);
		}
	}

	// check the subsystems of the ship.
	subsys_all_ok = 1;
	ssp = GET_FIRST(&shipp->subsys_list);
	while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {
		//Figure out how much we *can* repair the current subsystem -C
		max_subsys_repair = ssp->max_hits * (The_mission.support_ships.max_subsys_repair_val * 0.01f);

		if ( ssp->current_hits < max_subsys_repair && repair_allocated > 0 ) {
			subsys_all_ok = 0;
			subsys_type = ssp->system_info->type;

			if ( objp == Player_obj ) {
				player_maybe_start_repair_sound();
			}
			
			repair_delta = max_subsys_repair - ssp->current_hits;
			if ( repair_delta > repair_allocated ) {
				repair_delta = repair_allocated;
			}
			repair_allocated -= repair_delta;
			Assert(repair_allocated >= 0.0f);

			// add repair to current strength of single subsystem
			ssp->current_hits += repair_delta;
			if ( ssp->current_hits > max_subsys_repair ) {
				ssp->current_hits = max_subsys_repair;
			}

			// add repair to aggregate strength of subsystems of that type
			shipp->subsys_info[subsys_type].current_hits += repair_delta;
			if ( shipp->subsys_info[subsys_type].current_hits > shipp->subsys_info[subsys_type].total_hits )
				shipp->subsys_info[subsys_type].current_hits = shipp->subsys_info[subsys_type].total_hits;

			// check to see if this subsystem was totally non functional before -- if so, then
			// reset the flags
			if ( (ssp->system_info->type == SUBSYSTEM_ENGINE) && (shipp->flags & SF_DISABLED) ) {
				shipp->flags &= ~SF_DISABLED;
				ship_reset_disabled_physics(objp, shipp->ship_info_index);
//			} else if ( (ssp->system_info->type == SUBSYSTEM_TURRET) && (shipp->flags & SF_DISARMED) ) {
//				shipp->flags &= ~SF_DISARMED;
			} else if ( (ssp->system_info->type == SUBSYSTEM_SHIELD_GENERATOR) && (objp->flags & OF_NO_SHIELDS) ) {
				objp->flags &= ~OF_NO_SHIELDS;
			}

			break;
		}
		ssp = GET_NEXT( ssp );
	}

	// now deal with rearming the player.  All secondary weapons have a certain rate at which
	// they can be rearmed.  We can rearm multiple banks at once.
	banks_full = 0;
	primary_banks_full = 0;
	if ( subsys_all_ok )
	{
		for (i = 0; i < swp->num_secondary_banks; i++ )
		{
			// Actual loading of missiles is preceded by a sound effect which is the missile
			// loading equipment moving into place
			if ( aip->rearm_first_missile == TRUE )
			{
				swp->secondary_bank_rearm_time[i] = timestamp(snd_get_duration(Snds[SND_MISSILE_START_LOAD].id));			

				if (i == swp->num_secondary_banks - 1) 
					aip->rearm_first_missile = FALSE;
			}
			
			if ( swp->secondary_bank_ammo[i] < swp->secondary_bank_start_ammo[i] )
			{
				float rearm_time;

				if ( objp == Player_obj )
				{
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}

				if ( timestamp_elapsed(swp->secondary_bank_rearm_time[i]) )
				{
					rearm_time = Weapon_info[swp->secondary_bank_weapons[i]].rearm_rate;
					swp->secondary_bank_rearm_time[i] = timestamp((int)(rearm_time * 1000.0f));
					
					snd_play_3d( &Snds[SND_MISSILE_LOAD], &objp->pos, &View_position );
					if (objp == Player_obj)
						joy_ff_play_reload_effect();

					swp->secondary_bank_ammo[i] += REARM_NUM_MISSILES_PER_BATCH;
					if ( swp->secondary_bank_ammo[i] > swp->secondary_bank_start_ammo[i] ) 
					{
						swp->secondary_bank_ammo[i] = swp->secondary_bank_start_ammo[i]; 
					}
				}
				else
				{
				}
			} 
			else
			{
				banks_full++;
			}

			if ((aip->rearm_first_missile == TRUE) && (i == swp->num_secondary_banks - 1) && (banks_full != swp->num_secondary_banks))
					snd_play_3d( &Snds[SND_MISSILE_START_LOAD], &objp->pos, &View_position );
		}	// end for

		// rearm ballistic primaries - Goober5000
		if (sip->flags & SIF_BALLISTIC_PRIMARIES)
		{
			if ( aip->rearm_first_ballistic_primary == TRUE)
			{
				for (i = 1; i < swp->num_primary_banks; i++ )
				{
					if ( Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC )
						last_ballistic_idx = i;
				}
			}

			for (i = 0; i < swp->num_primary_banks; i++ )
			{
				if ( Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC )
				{
					// Actual loading of bullets is preceded by a sound effect which is the bullet
					// loading equipment moving into place
					if ( aip->rearm_first_ballistic_primary == TRUE )
					{
						// Goober5000
						int sound_index;
						if (Snds[SND_BALLISTIC_START_LOAD].id >= 0)
							sound_index = SND_BALLISTIC_START_LOAD;
						else
							sound_index = SND_MISSILE_START_LOAD;

						swp->primary_bank_rearm_time[i] = timestamp(snd_get_duration(Snds[sound_index].id));			

						if (i == last_ballistic_idx) 
							aip->rearm_first_ballistic_primary = FALSE;
					}

					if ( swp->primary_bank_ammo[i] < swp->primary_bank_start_ammo[i] )
					{
						float rearm_time;
	
						if ( objp == Player_obj )
						{
							hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
						}

						if ( timestamp_elapsed(swp->primary_bank_rearm_time[i]) )
						{
							rearm_time = Weapon_info[swp->primary_bank_weapons[i]].rearm_rate;
							swp->primary_bank_rearm_time[i] = timestamp( (int)(rearm_time * 1000.f) );
	
							// Goober5000
							int sound_index;
							if (Snds[SND_BALLISTIC_LOAD].id >= 0)
								sound_index = SND_BALLISTIC_LOAD;
							else
								sound_index = SND_MISSILE_LOAD;

							snd_play_3d( &Snds[sound_index], &objp->pos, &View_position );

								/* don't provide force feedback for primary ballistics loading
								if (objp == Player_obj)
									joy_ff_play_reload_effect();
								*/
	
							swp->primary_bank_ammo[i] += REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH;
							if ( swp->primary_bank_ammo[i] > swp->primary_bank_start_ammo[i] )
							{
								swp->primary_bank_ammo[i] = swp->primary_bank_start_ammo[i]; 
							}
						}
					}
					else
					{
						primary_banks_full++;
					}
				}
				// if the bank is not a ballistic
				else
				{
					primary_banks_full++;
				}

				if ((aip->rearm_first_ballistic_primary == TRUE) && (i == swp->num_primary_banks - 1) && (primary_banks_full != swp->num_primary_banks))
				{
					// Goober5000
					int sound_index;
					if (Snds[SND_BALLISTIC_START_LOAD].id >= 0)
						sound_index = SND_BALLISTIC_START_LOAD;
					else
						sound_index = SND_MISSILE_START_LOAD;

					snd_play_3d( &Snds[sound_index], &objp->pos, &View_position );
				}
			}	// end for
		}	// end if - rearm ballistic primaries
	} // end if (subsys_all_ok)

	if ( banks_full == swp->num_secondary_banks )
	{
		aip->rearm_first_missile = TRUE;
	}

	if ( primary_banks_full == swp->num_primary_banks )
	{
		aip->rearm_first_ballistic_primary = TRUE;
	}

	int shields_full = 0;
	if ( (objp->flags & OF_NO_SHIELDS) ) {
		shields_full = 1;
	} else {
		if ( shield_get_strength(objp) >= shield_get_max_strength(objp) ) 
			shields_full = 1;
	}

	// return 1 if at end of subsystem list, hull damage at 0, and shields full and all secondary banks full.
//	if ( ((ssp = END_OF_LIST(&shipp->subsys_list)) != NULL )&&(objp->hull_strength == shipp->ship_max_hull_strength)&&(shields_full) ) {
	if ( (subsys_all_ok && shields_full && (The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) && (objp->hull_strength >= max_hull_repair) ) || (subsys_all_ok && shields_full && !(The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) ) )
	{
		if ( objp == Player_obj ) {
			player_stop_repair_sound();
		}

		if (!aip->rearm_release_delay)
			aip->rearm_release_delay = timestamp(1200);

		// check both primary and secondary banks are full
		if ( (banks_full == swp->num_secondary_banks) &&
			( !(sip->flags & SIF_BALLISTIC_PRIMARIES) || ((sip->flags & SIF_BALLISTIC_PRIMARIES) && (primary_banks_full == swp->num_primary_banks)) )	)
		{
			if ( timestamp_elapsed(aip->rearm_release_delay) )
				return 1;
		}
		else
		{
			aip->rearm_release_delay = timestamp(1200);
		}
	}

	if (objp == Player_obj) Player_rearm_eta -= frametime;

	return 0;
}

// function which is used to find a repair ship to repair requester_obj.  the way repair ships will work
// is:
// if repair ship present and available, return pointer to that object.
// If repair ship present and busy, possibly return that object if he can satisfy the request soon enough.
// If repair ship present and busy and cannot satisfy request, return NULL to warp a new one in if below max number
// if no repair ship present, return NULL to force a new one to be warped in.
#define	MAX_SUPPORT_SHIPS_PER_TEAM		1

object *ship_find_repair_ship( object *requester_obj )
{
	object *objp;
	ship *requester_ship;
	int	num_support_ships, num_available_support_ships;
	float	min_dist = 99999.0f;
	object	*nearest_support_ship = NULL;
	int		support_ships[MAX_SUPPORT_SHIPS_PER_TEAM];

	Assert(requester_obj->type == OBJ_SHIP);
	Assert((requester_obj->instance >= 0) && (requester_obj->instance < MAX_OBJECTS));

	// if support ships are not allowed, then no support ship can repair!
	if ( !is_support_allowed(requester_obj) ) {
		return NULL;
	}

	num_support_ships = 0;
	num_available_support_ships = 0;

	requester_ship = &Ships[requester_obj->instance];
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ((objp->type == OBJ_SHIP) && !(objp->flags & OF_SHOULD_BE_DEAD))
		{
			ship			*shipp;
			ship_info	*sip;
			float			dist;

			Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

			shipp = &Ships[objp->instance];
			sip = &Ship_info[shipp->ship_info_index];

			if ( shipp->team != requester_ship->team ) {
				continue;
			}

			if ( !(sip->flags & SIF_SUPPORT) ) {
				continue;
			}

			// don't deal with dying support ships
			if ( shipp->flags & (SF_DYING | SF_DEPARTING) ) {
				continue;
			}

			dist = vm_vec_dist_quick(&objp->pos, &requester_obj->pos);
			support_ships[num_support_ships] = OBJ_INDEX(objp);

			if (!(Ai_info[shipp->ai_index].ai_flags & AIF_REPAIRING))
			{
				num_available_support_ships++;
				if (dist < min_dist)
				{
					min_dist = dist;
					nearest_support_ship = objp;
				}
			}

			if ( num_support_ships >= MAX_SUPPORT_SHIPS_PER_TEAM )
			{
				mprintf(("Why is there more than %d support ships in this mission?\n",MAX_SUPPORT_SHIPS_PER_TEAM));
				break;
			}
			else
			{
				support_ships[num_support_ships] = OBJ_INDEX(objp);
				num_support_ships++;
			}
		}
	}

	if (nearest_support_ship != NULL) {
		return nearest_support_ship;
	} else if (num_support_ships >= MAX_SUPPORT_SHIPS_PER_TEAM) {
		Assert(&Objects[support_ships[0]] != NULL);
		return &Objects[support_ships[0]];
	} else {
		Assert(num_support_ships < MAX_SUPPORT_SHIPS_PER_TEAM);
		return NULL;
	}
}



// -------------------------------------------------------------------------------------------------
// ship_close()
//
// called in game_shutdown() to free malloced memory
//
// NOTE: do not call this function.  It is only called from game_shutdown()
int CLOAKMAP=-1;
void ship_close()
{
	int i, n;

	for (i=0; i<MAX_SHIPS; i++ )	{
		ship *shipp = &Ships[i];

		if (shipp->shield_integrity != NULL) {
			vm_free(shipp->shield_integrity);
			shipp->shield_integrity = NULL;
		}

		if (shipp->replacement_textures != NULL) {
			vm_free(shipp->replacement_textures);
			shipp->replacement_textures = NULL;
		}
	}

	// free memory alloced for subsystem storage
	for ( i = 0; i < Num_ship_classes; i++ ) {
		if ( Ship_info[i].subsystems != NULL ) {
			for(n = 0; n < Ship_info[i].n_subsystems; n++) {
				if (Ship_info[i].subsystems[n].triggers != NULL) {
					vm_free(Ship_info[i].subsystems[n].triggers);
					Ship_info[i].subsystems[n].triggers = NULL;
				}
			}

			vm_free(Ship_info[i].subsystems);
			Ship_info[i].subsystems = NULL;
		}

		

		// free info from parsed table data
		if (Ship_info[i].type_str != NULL) {
			vm_free(Ship_info[i].type_str);
			Ship_info[i].type_str = NULL;
		}

		if (Ship_info[i].maneuverability_str != NULL) {
			vm_free(Ship_info[i].maneuverability_str);
			Ship_info[i].maneuverability_str = NULL;
		}

		if (Ship_info[i].armor_str != NULL) {
			vm_free(Ship_info[i].armor_str);
			Ship_info[i].armor_str = NULL;
		}

		if (Ship_info[i].manufacturer_str != NULL) {
			vm_free(Ship_info[i].manufacturer_str);
			Ship_info[i].manufacturer_str = NULL;
		}

		if (Ship_info[i].desc != NULL) {
			vm_free(Ship_info[i].desc);
			Ship_info[i].desc = NULL;
		}

		if (Ship_info[i].tech_desc != NULL) {
			vm_free(Ship_info[i].tech_desc);
			Ship_info[i].tech_desc = NULL;
		}

		if (Ship_info[i].ship_length != NULL) {
			vm_free(Ship_info[i].ship_length);
			Ship_info[i].ship_length = NULL;
		}

		if (Ship_info[i].gun_mounts != NULL) {
			vm_free(Ship_info[i].gun_mounts);
			Ship_info[i].gun_mounts = NULL;
		}

		if (Ship_info[i].missile_banks != NULL) {
			vm_free(Ship_info[i].missile_banks);
			Ship_info[i].missile_banks = NULL;
		}
	}

	// free info from parsed table data
	for (i=0; i<MAX_SHIP_CLASSES; i++) {
		if(Ship_info[i].type_str != NULL){
			vm_free(Ship_info[i].type_str);
			Ship_info[i].type_str = NULL;
		}
		if(Ship_info[i].maneuverability_str != NULL){
			vm_free(Ship_info[i].maneuverability_str);
			Ship_info[i].maneuverability_str = NULL;
		}
		if(Ship_info[i].armor_str != NULL){
			vm_free(Ship_info[i].armor_str);
			Ship_info[i].armor_str = NULL;
		}
		if(Ship_info[i].manufacturer_str != NULL){
			vm_free(Ship_info[i].manufacturer_str);
			Ship_info[i].manufacturer_str = NULL;
		}
		if(Ship_info[i].desc != NULL){
			vm_free(Ship_info[i].desc);
			Ship_info[i].desc = NULL;
		}
		if(Ship_info[i].tech_desc != NULL){
			vm_free(Ship_info[i].tech_desc);
			Ship_info[i].tech_desc = NULL;
		}
		if(Ship_info[i].ship_length != NULL){
			vm_free(Ship_info[i].ship_length);
			Ship_info[i].ship_length = NULL;
		}
		if(Ship_info[i].gun_mounts != NULL){
			vm_free(Ship_info[i].gun_mounts);
			Ship_info[i].gun_mounts = NULL;
		}
		if(Ship_info[i].missile_banks != NULL){
			vm_free(Ship_info[i].missile_banks);
			Ship_info[i].missile_banks = NULL;
		}
	}

	if(CLOAKMAP != -1)
		bm_release(CLOAKMAP);
}	

// -------------------------------------------------------------------------------------------------
// ship_assign_sound()
//
//	Assign object-linked sound to a particular ship
//
void ship_assign_sound(ship *sp)
{
	ship_info	*sip;	
	object *objp;
	vec3d engine_pos;
	ship_subsys *moveup;

	Assert( sp->objnum >= 0 );
	if(sp->objnum < 0){
		return;
	}

	objp = &Objects[sp->objnum];
	sip = &Ship_info[sp->ship_info_index];

	if ( sip->engine_snd != -1 ) {
		vm_vec_copy_scale(&engine_pos, &objp->orient.vec.fvec, -objp->radius/2.0f);		
		
		obj_snd_assign(sp->objnum, sip->engine_snd, &engine_pos, 1);
	}

	// Do subsystem sounds	
	moveup = GET_FIRST(&sp->subsys_list);
	while(moveup != END_OF_LIST(&sp->subsys_list)){
		// Check for any engine sounds		
		if(strstr(moveup->system_info->name, "enginelarge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_LARGE, &moveup->system_info->pnt, 0);
		} else if(strstr(moveup->system_info->name, "enginehuge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_HUGE, &moveup->system_info->pnt, 0);
		}

		//Do any normal subsystem sounds
		if(moveup->current_hits < moveup->max_hits)
		{
			if(moveup->system_info->alive_snd != -1)
				obj_snd_assign(sp->objnum, moveup->system_info->alive_snd, &moveup->system_info->pnt, 1);
		}
		else 
		{
			if(moveup->system_info->dead_snd != -1)
				obj_snd_assign(sp->objnum, moveup->system_info->dead_snd, &moveup->system_info->pnt, 1);
		}

		// next
		moveup = GET_NEXT(moveup);
	}	
}

// -------------------------------------------------------------------------------------------------
// ship_assign_sound_all()
//
//	Assign object-linked sounds to all ships currently in the obj_used_list
//
void ship_assign_sound_all()
{
	object *objp;
	int idx, has_sounds;

	if ( !Sound_enabled )
		return;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {		
		if ( objp->type == OBJ_SHIP && Player_obj != objp) {
			has_sounds = 0;

			// check to make sure this guy hasn't got sounds already assigned to him
			for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
				if(objp->objsnd_num[idx] != -1){
					// skip
					has_sounds = 1;
					break;
				}
			}

			// actually assign the sound
			if(!has_sounds){
				ship_assign_sound(&Ships[objp->instance]);
			}
		}
	}
}


// ---------------------------------------------------------------------------------------
// dcf_set_shield()
//
// Debug console function to set the shield for the player ship
//
DCF(set_shield,"Change player ship shield strength")
{
	ship_info	*sip;
	
	sip = &Ship_info[Ships[Player_obj->instance].ship_info_index];
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);

		if ( Dc_arg_type & ARG_FLOAT ) {
			if ( Dc_arg_float < 0 ) 
				Dc_arg_float = 0.0f;
			if ( Dc_arg_float > 1.0 )
				Dc_arg_float = 1.0f;
			shield_set_strength(Player_obj, Dc_arg_float * shield_get_max_strength(Player_obj));
			dc_printf("Shields set to %.2f\n", shield_get_strength(Player_obj) );
		}
	}

	if ( Dc_help ) {
		dc_printf ("Usage: set_shield [num]\n");
		dc_printf ("[num] --  shield percentage 0.0 -> 1.0 of max\n");
		dc_printf ("with no parameters, displays shield strength\n");
		Dc_status = 0;
	}

	if ( Dc_status )	{
		dc_printf( "Shields are currently %.2f", shield_get_strength(Player_obj) );
	}
}

// ---------------------------------------------------------------------------------------
// dcf_set_hull()
//
// Debug console function to set the hull for the player ship
//
DCF(set_hull, "Change player ship hull strength")
{
	ship_info	*sip;
	
	sip = &Ship_info[Ships[Player_obj->instance].ship_info_index];
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);

		if ( Dc_arg_type & ARG_FLOAT ) {
			if ( Dc_arg_float < 0 ) 
				Dc_arg_float = 0.0f;
			if ( Dc_arg_float > 1.0 )
				Dc_arg_float = 1.0f;
			Player_obj->hull_strength = Dc_arg_float * Player_ship->ship_max_hull_strength;
			dc_printf("Hull set to %.2f\n", Player_obj->hull_strength );
		}
	}

	if ( Dc_help ) {
		dc_printf ("Usage: set_hull [num]\n");
		dc_printf ("[num] --  hull percentage 0.0 -> 1.0 of max\n");
		dc_printf ("with no parameters, displays hull strength\n");
		Dc_status = 0;
	}

	if ( Dc_status )	{
		dc_printf( "Hull is currently %.2f", Player_obj->hull_strength );
	}
}

// ---------------------------------------------------------------------------------------
// dcf_set_subsys()
//
// Debug console function to set the strength of a particular subsystem
//
//XSTR:OFF
DCF(set_subsys, "Set the strength of a particular subsystem on player ship" )
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !stricmp( Dc_arg, "weapons" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_WEAPONS, Dc_arg_float );
				if ( Dc_arg_float < 0.01f )	{
//					Player_ship->flags |= SF_DISARMED;
				} else {
//					Player_ship->flags &= ~SF_DISARMED;
				}
			} 
		} else if ( !stricmp( Dc_arg, "engine" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_ENGINE, Dc_arg_float );
				if ( Dc_arg_float < 0.01f )	{
					Player_ship->flags |= SF_DISABLED;
				} else {
					Player_ship->flags &= ~SF_DISABLED;
				}
			} 
		} else if ( !stricmp( Dc_arg, "sensors" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_SENSORS, Dc_arg_float );
			} 
		} else if ( !stricmp( Dc_arg, "communication" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_COMMUNICATION, Dc_arg_float );
			} 
		} else if ( !stricmp( Dc_arg, "navigation" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_NAVIGATION, Dc_arg_float );
			} 
		} else if ( !stricmp( Dc_arg, "radar" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_RADAR, Dc_arg_float );
			} 
		} else if ( !stricmp( Dc_arg, "shield" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_SHIELD_GENERATOR, Dc_arg_float );
				if ( Dc_arg_float < 0.01f )	{
					Player_obj->flags |= OF_NO_SHIELDS;
				} else {
					Player_obj->flags &= ~OF_NO_SHIELDS;
				}
			}
		} else {
			// print usage
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: set_subsys type X\nWhere X is value between 0 and 1.0, and type can be:\n" );
		dc_printf( "weapons\n" );
		dc_printf( "engine\n" );
		dc_printf( "sensors\n" );
		dc_printf( "communication\n" );
		dc_printf( "navigation\n" );
		dc_printf( "radar\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}
}
//XSTR:ON

// console function to toggle whether auto-repair for subsystems is active
#ifndef NDEBUG
DCF_BOOL( auto_repair, Ship_auto_repair )
#endif

// two functions to keep track of counting ships of particular types.  Maybe we should be rolling this
// thing into the stats section??  The first function adds a ship of a particular type to the overall
// count of ships of that type (called from MissionParse.cpp).  The second function adds to the kill total
// of ships of a particular type.  Note that we use the ship_info flags structure member to determine
// what is happening.
//WMC - ALERT!!!!!!!!!!!
//These two functions did something weird with fighters/bombers. I don't
//think that not doing this will break anything, but it might.
//If it does, get me. OR someone smart.
void ship_add_ship_type_count( int ship_info_index, int num )
{
	int type = ship_class_query_general_type(ship_info_index);

	//Ship has no type or something
	if(type < 0) {
		return;
	}

	//Resize if we need to
	uint oldsize = Ship_type_counts.size();
	if(type >= (int) oldsize) {
		Ship_type_counts.resize(type+1);

		//WMC - Set everything to 0
		for(uint i = oldsize; i < Ship_type_counts.size(); i++) {
			Ship_type_counts[i].killed = 0;
			Ship_type_counts[i].total = 0;
		}
	}

	//Add it
	Ship_type_counts[type].total += num;
}

void ship_add_ship_type_kill_count( int ship_info_index )
{
	int type = ship_class_query_general_type(ship_info_index);

	//Ship has no type or something
	if(type < 0) {
		return;
	}

	//Resize if we need to
	uint oldsize = Ship_type_counts.size();
	if(type >= (int) oldsize) {
		Ship_type_counts.resize(type+1);

		//WMC - Set everything to 0
		for(uint i = oldsize; i < Ship_type_counts.size(); i++) {
			Ship_type_counts[i].killed = 0;
			Ship_type_counts[i].total = 0;
		}
	}

	//Add it
	Ship_type_counts[type].killed++;
}

int ship_query_general_type(int ship)
{
	return ship_query_general_type(&Ships[ship]);
}

int ship_query_general_type(ship *shipp)
{
	return ship_class_query_general_type(shipp->ship_info_index);
}

int ship_class_query_general_type(int ship_class)
{
	//This is quick
	return Ship_info[ship_class].class_type;
}

// returns true if the docker can (is allowed) to dock with dockee
int ship_docking_valid(int docker, int dockee)
{
	// Goober5000
	// So many people have asked for this function to be extended that it's making less
	// and less sense to keep it around.  We should probably just let any ship type
	// dock with any other ship type and assume the mission designer is smart enough not to
	// mess things up.
	return 1;

	/*
	int docker_type, dockee_type;

	Assert(docker >= 0 && docker < MAX_SHIPS);
	Assert(dockee >= 0 && dockee < MAX_SHIPS);
	docker_type = ship_query_general_type(docker);
	dockee_type = ship_query_general_type(dockee);

	// escape pods can dock with transports, freighters, cruisers.
	if ( docker_type == SHIP_TYPE_ESCAPEPOD ) {
		if ( (dockee_type == SHIP_TYPE_TRANSPORT) || (dockee_type == SHIP_TYPE_CRUISER)
			|| (dockee_type == SHIP_TYPE_FREIGHTER) || (dockee_type == SHIP_TYPE_DRYDOCK)
			|| (dockee_type == SHIP_TYPE_CORVETTE) || (dockee_type == SHIP_TYPE_GAS_MINER)
			|| (dockee_type == SHIP_TYPE_AWACS))
		{
			return 1;
		}
	}

	// docker == freighter - navbuoys, sentries, and fighters added by Goober5000
	if (docker_type == SHIP_TYPE_FREIGHTER)
	{
		if ( (dockee_type == SHIP_TYPE_CARGO) || (dockee_type == SHIP_TYPE_CRUISER)
			|| (dockee_type == SHIP_TYPE_CAPITAL) || (dockee_type == SHIP_TYPE_SUPERCAP)
			|| (dockee_type == SHIP_TYPE_DRYDOCK) || (dockee_type == SHIP_TYPE_CORVETTE)
			|| (dockee_type == SHIP_TYPE_GAS_MINER) || (dockee_type == SHIP_TYPE_AWACS)
			|| (dockee_type == SHIP_TYPE_NAVBUOY) || (dockee_type == SHIP_TYPE_SENTRYGUN)
			|| (dockee_type == SHIP_TYPE_FIGHTER_BOMBER) || (dockee_type == SHIP_TYPE_STEALTH))
		{
			return 1;
		}
	}

	// docker == cruiser
	if ( (docker_type == SHIP_TYPE_CRUISER) || (docker_type == SHIP_TYPE_CORVETTE) ||
		(docker_type == SHIP_TYPE_GAS_MINER) || (docker_type == SHIP_TYPE_AWACS))
	{
		if ( (dockee_type == SHIP_TYPE_CARGO) || (dockee_type == SHIP_TYPE_CRUISER)
			|| (dockee_type == SHIP_TYPE_CAPITAL) || (dockee_type == SHIP_TYPE_SUPERCAP)
			|| (dockee_type == SHIP_TYPE_DRYDOCK) || (dockee_type == SHIP_TYPE_CORVETTE)
			|| (dockee_type == SHIP_TYPE_GAS_MINER) || (dockee_type == SHIP_TYPE_AWACS))
		{
			return 1;
		}
	}

	// Transports can now dock with fighter-bomber and stealth - Goober5000
	// Goober5000 - navbuoys, sentries, and fighters added
	if (docker_type == SHIP_TYPE_TRANSPORT)
	{
		if ( (dockee_type == SHIP_TYPE_CARGO) || (dockee_type == SHIP_TYPE_CRUISER)
			|| (dockee_type == SHIP_TYPE_FREIGHTER) || (dockee_type == SHIP_TYPE_TRANSPORT)
			|| (dockee_type == SHIP_TYPE_CAPITAL) || (dockee_type == SHIP_TYPE_ESCAPEPOD) 
			|| (dockee_type == SHIP_TYPE_SUPERCAP) || (dockee_type == SHIP_TYPE_DRYDOCK)
			|| (dockee_type == SHIP_TYPE_CORVETTE) || (dockee_type == SHIP_TYPE_GAS_MINER)
			|| (dockee_type == SHIP_TYPE_AWACS) || (dockee_type == SHIP_TYPE_FIGHTER_BOMBER)
			|| (dockee_type == SHIP_TYPE_STEALTH) || (dockee_type == SHIP_TYPE_NAVBUOY)
			|| (dockee_type == SHIP_TYPE_SENTRYGUN))
		{
				return 1;
		}
	}

	// supply ships
	if (docker_type == SHIP_TYPE_REPAIR_REARM)
	{
		if ((dockee_type == SHIP_TYPE_FIGHTER_BOMBER) || (dockee_type == SHIP_TYPE_STEALTH))
		{
			return 1;
		}
	}

	// fighters, bombers, and stealth - Goober5000
	if ((docker_type == SHIP_TYPE_FIGHTER_BOMBER) || (docker_type == SHIP_TYPE_STEALTH))
	{
		if ( (dockee_type == SHIP_TYPE_CARGO) || (dockee_type == SHIP_TYPE_TRANSPORT)
			|| (dockee_type == SHIP_TYPE_FIGHTER_BOMBER) || (dockee_type == SHIP_TYPE_STEALTH)
			|| (dockee_type == SHIP_TYPE_FREIGHTER) || (dockee_type == SHIP_TYPE_CRUISER)
			|| (dockee_type == SHIP_TYPE_CORVETTE) || (dockee_type == SHIP_TYPE_CAPITAL)
			|| (dockee_type == SHIP_TYPE_SUPERCAP) || (dockee_type == SHIP_TYPE_DRYDOCK)
			|| (dockee_type == SHIP_TYPE_REPAIR_REARM) || (dockee_type == SHIP_TYPE_NAVBUOY)
			|| (dockee_type == SHIP_TYPE_SENTRYGUN))
		{
			return 1;
		}
	}

	return 0;
	*/
}

// function to return a random ship in a starting player wing.  Returns -1 if a suitable
// one cannot be found
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
// input:   persona  => OPTIONAL PARAMETER (default to -1) which persona to get
int ship_get_random_player_wing_ship( int flags, float max_dist, int persona_index, int get_first, int multi_team )
{
	const int MAX_SIZE = MAX_SHIPS_PER_WING * MAX_SQUADRON_WINGS;

	int i, j, ship_index, count;
	int slist[MAX_SIZE], which_one;

	// iterate through starting wings of player.  Add ship indices of ships which meet
	// given criteria
	count = 0;
	for (i = 0; i < Num_wings; i++ ) {
		if (count >= MAX_SIZE)
			break;

		int wingnum = -1;

		// multi-team?
		if(multi_team >= 0){
			if( i == TVT_wings[multi_team] ) {
				wingnum = i;
			} else {
				continue;
			}
		} else {
			// first check for a player starting wing
			for ( j = 0; j < MAX_STARTING_WINGS; j++ ) {
				if ( i == Starting_wings[j] ) {
					wingnum = i;
					break;
				}
			}

			// if not found, then check all squad wings (Goober5000)
			if ( wingnum == -1 ) {
				for ( j = 0; j < MAX_SQUADRON_WINGS; j++ ) {
					if ( i == Squadron_wings[j] ) {
						wingnum = i;
						break;
					}
				}
			}

			if ( wingnum == -1 ){
				continue;
			}
		}

		for ( j = 0; j < Wings[wingnum].current_count; j++ ) {
			if (count >= MAX_SIZE)
				break;

			ship_index = Wings[wingnum].ship_index[j];
			Assert( ship_index != -1 );

			if ( Ships[ship_index].flags & SF_DYING ) {
				continue;
			}
			// see if ship meets our criterea
			if ( (flags == SHIP_GET_NO_PLAYERS || flags == SHIP_GET_UNSILENCED) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) ){
				continue;
			}
			
			if ( (flags == SHIP_GET_UNSILENCED) && (Ships[ship_index].flags2 & SF2_NO_BUILTIN_MESSAGES) )
			{
				continue;
			}

			// don't process ships on a different team
			if(multi_team < 0){
				if ( Player_ship->team != Ships[ship_index].team ){
					continue;
				}
			}

			// see if ship is within max_dist units
			if ( (max_dist > 1.0f) && (multi_team < 0) ) {
				float dist;
				dist = vm_vec_dist_quick(&Objects[Ships[ship_index].objnum].pos, &Player_obj->pos);
				if ( dist > max_dist ) {
					continue;
				}
			}

			// if we should be checking persona's, then don't add ships that don't have the proper persona
			if ( persona_index != -1 ) {
				if ( Ships[ship_index].persona_index != persona_index ){
					continue;
				}
			}

			// return the first ship with correct persona
			if (get_first) {
				return ship_index;
			}

			slist[count] = ship_index;
			count++;
		}
	}

	if ( count == 0 ){
		return -1;
	}

	// now get a random one from the list
	which_one = (rand() % count);
	ship_index = slist[which_one];

	Assert ( Ships[ship_index].objnum != -1 );

	return ship_index;
}

// like above function, but returns a random ship in the given wing -- no restrictions
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
int ship_get_random_ship_in_wing(int wingnum, int flags, float max_dist, int get_first)
{
	int i, ship_index, slist[MAX_SHIPS_PER_WING], count, which_one;

	count = 0;
	for ( i = 0; i < Wings[wingnum].current_count; i++ ) {
		ship_index = Wings[wingnum].ship_index[i];
		Assert( ship_index != -1 );

		if ( Ships[ship_index].flags & SF_DYING ) {
			continue;
		}

		// see if ship meets our criterea
		if ( (flags == SHIP_GET_NO_PLAYERS || flags == SHIP_GET_UNSILENCED) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) )
			continue;

		if ( (flags == SHIP_GET_UNSILENCED) && (Ships[ship_index].flags2 & SF2_NO_BUILTIN_MESSAGES) )
		{
			continue;
		}

		// see if ship is within max_dist units
		if ( max_dist > 0 ) {
			float dist;
			dist = vm_vec_dist_quick(&Objects[Ships[ship_index].objnum].pos, &Player_obj->pos);
			if ( dist > max_dist ) {
				continue;
			}
		}

		// return the first ship in wing
		if (get_first) {
			return ship_index;
		}

		slist[count] = ship_index;
		count++;
	}

	if ( count == 0 ) {
		return -1;
	}

	// now get a random one from the list
	which_one = (rand() % count);
	ship_index = slist[which_one];

	Assert ( Ships[ship_index].objnum != -1 );

	return ship_index;
}


// this function returns a random index into the Ship array of a ship of the given team
// cargo containers are not counted as ships for the purposes of this function.  Why???
// because now it is only used for getting a random ship for a message and cargo containers
// can't send mesages.  This function is an example of kind of bad coding :-(
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
int ship_get_random_team_ship(int team_mask, int flags, float max_dist )
{
	int num, which_one;
	object *objp, *obj_list[MAX_SHIPS];

	// for any allied, go through the ships list and find all of the ships on that team
	num = 0;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type != OBJ_SHIP )
			continue;

		// series of conditionals one per line for easy reading
		// don't process ships on wrong team
		// don't process cargo's or navbuoys
		// don't process player ships if flags are set
		if (!iff_matches_mask(Ships[objp->instance].team, team_mask))
			continue;
		else if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_NOT_FLYABLE )
			continue;
		else if ( (flags == SHIP_GET_NO_PLAYERS) && (objp->flags & OF_PLAYER_SHIP) )
			continue;
		else if ( (flags == SHIP_GET_ONLY_PLAYERS) && !(objp->flags & OF_PLAYER_SHIP) )
			continue;

		if ( Ships[objp->instance].flags & SF_DYING ) {
			continue;
		}

		// see if ship is within max_dist units
		if ( max_dist > 0 ) {
			float dist;
			dist = vm_vec_dist_quick(&objp->pos, &Player_obj->pos);
			if ( dist > max_dist ) {
				continue;
			}
		}

		obj_list[num] = objp;
		num++;
	}

	if ( num == 0 )
		return -1;

	which_one = (rand() % num);
	objp = obj_list[which_one];

	Assert ( objp->instance != -1 );

	return objp->instance;
}

// -----------------------------------------------------------------------
// ship_secondary_bank_has_ammo()
//
// check if currently selected secondary bank has ammo
//
// input:	shipnum	=>	index into Ships[] array for ship to check
//
int ship_secondary_bank_has_ammo(int shipnum)
{
	ship_weapon	*swp;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
	swp = &Ships[shipnum].weapons;
	
	if ( swp->current_secondary_bank == -1 )
		return 0;

	Assert(swp->current_secondary_bank >= 0 && swp->current_secondary_bank < MAX_SHIP_SECONDARY_BANKS );
	if ( swp->secondary_bank_ammo[swp->current_secondary_bank] <= 0 )
		return 0;

	return 1;
}

// ship_primary_bank_has_ammo()
//
// check if currently selected primary bank has ammo
//
// input:	shipnum	=>	index into Ships[] array for ship to check
//
int ship_primary_bank_has_ammo(int shipnum)
{
	ship_weapon	*swp;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
	swp = &Ships[shipnum].weapons;
	
	if ( swp->current_primary_bank == -1 )
	{
		return 0;
	}

	Assert(swp->current_primary_bank >= 0 && swp->current_primary_bank < MAX_SHIP_PRIMARY_BANKS );
	
	return ( primary_out_of_ammo(swp, swp->current_primary_bank) == 0 );
}

// see if there is enough engine power to allow the ship to warp
// returns 1 if ship is able to warp, otherwise return 0
int ship_engine_ok_to_warp(ship *sp)
{
	// disabled ships can't warp
	if (sp->flags & SF_DISABLED)
		return 0;

	float engine_strength = ship_get_subsystem_strength(sp, SUBSYSTEM_ENGINE);

	// if at 0% strength, can't warp
	if (engine_strength <= 0.0f)
		return 0;

	// player ships playing above Very Easy can't warp when below a threshold
	if ((sp == Player_ship) && (Game_skill_level > 0) && (engine_strength < SHIP_MIN_ENGINES_TO_WARP))
		return 0;

	// otherwise, warp is allowed
	return 1;
}

// Goober5000
// see if there is enough navigation power to allow the ship to warp
// returns 1 if ship is able to warp, otherwise return 0
int ship_navigation_ok_to_warp(ship *sp)
{
	// if not using the special flag, warp is always allowed
	if (!(The_mission.ai_profile->flags & AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP))
		return 1;

	float navigation_strength = ship_get_subsystem_strength(sp, SUBSYSTEM_NAVIGATION);

	// if at 0% strength, can't warp
	if (navigation_strength <= 0.0f)
		return 0;

	// player ships playing above Very Easy can't warp when below a threshold
	if ((sp == Player_ship) && (Game_skill_level > 0) && (navigation_strength < SHIP_MIN_NAV_TO_WARP))
		return 0;

	// otherwise, warp is allowed
	return 1;
}

// Calculate the normal vector from a subsystem position and its first path point
// input:	sp	=>	pointer to ship that is parent of subsystem
//				ss =>	pointer to subsystem of interest
//				norm	=> output parameter... vector from subsys to first path point
//
//	exit:		0	=>	a valid vector was placed in norm
//				!0	=> an path normal could not be calculated
//				
int ship_return_subsys_path_normal(ship *shipp, ship_subsys *ss, vec3d *gsubpos, vec3d *norm)
{
	if ( ss->system_info->path_num >= 0 ) {
		polymodel	*pm = NULL;
		model_path	*mp;
		vec3d		*path_point;
		vec3d		gpath_point;
		pm = model_get(Ship_info[shipp->ship_info_index].model_num);
		Assert( pm != NULL );

		if (ss->system_info->path_num > pm->n_paths) {
			// possibly a bad model?
			mprintf(("WARNING: Too many paths in '%s'!  Max is %i and the requested path was %i for subsystem '%s'!\n", pm->filename, pm->n_paths, ss->system_info->path_num, ss->system_info->name));
		//	Int3();
			return 1;
		}

		mp = &pm->paths[ss->system_info->path_num];
		if ( mp->nverts >= 2 ) {
//			path_point = &mp->verts[mp->nverts-1].pos;
			path_point = &mp->verts[0].pos;
			// get path point in world coords
			vm_vec_unrotate(&gpath_point, path_point, &Objects[shipp->objnum].orient);
			vm_vec_add2(&gpath_point, &Objects[shipp->objnum].pos);
			// get unit vector pointing from subsys pos to first path point
			vm_vec_normalized_dir(norm, &gpath_point, gsubpos);
			return 0;
		}
	}
	return 1;
}


//	Determine if the subsystem can be viewed from eye_pos.  The method is to check where the
// vector from eye_pos to the subsystem hits the ship.  If distance from the hit position and
// the center of the subsystem is within a range (currently the subsystem radius) it is considered
// in view (return true).  If not in view, return false.
//
// input:	objp		=>		object that is the ship with the subsystem on it
//				subsys	=>		pointer to the subsystem of interest
//				eye_pos	=>		world coord for the eye looking at the subsystem
//				subsys_pos			=>	world coord for the center of the subsystem of interest
//				do_facing_check	=>	OPTIONAL PARAMETER (default value is 1), do a dot product check to see if subsystem fvec is facing
//											towards the eye position	
//				dot_out	=>		OPTIONAL PARAMETER, output parameter, will return dot between subsys fvec and subsys_to_eye_vec
//									(only filled in if do_facing_check is true)
//				vec_out	=>		OPTIONAL PARAMETER, vector from eye_pos to absolute subsys_pos.  (only filled in if do_facing_check is true)
int ship_subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye_pos, vec3d* subsys_pos, int do_facing_check, float *dot_out, vec3d *vec_out)
{
	float		dist, dot;
	mc_info	mc;
	vec3d	terminus, eye_to_pos, subsys_fvec, subsys_to_eye_vec;

	if (objp->type != OBJ_SHIP)
		return 0;

	// See if we are at least facing the subsystem
	if ( do_facing_check ) {
		if ( ship_return_subsys_path_normal(&Ships[objp->instance], subsys, subsys_pos, &subsys_fvec) ) {
			// non-zero return value means that we couldn't generate a normal from path info... so use inaccurate method
			vm_vec_normalized_dir(&subsys_fvec, subsys_pos, &objp->pos);
		}

		vm_vec_normalized_dir(&subsys_to_eye_vec, eye_pos, subsys_pos);
		dot = vm_vec_dot(&subsys_fvec, &subsys_to_eye_vec);
		if ( dot_out ) {
			*dot_out = dot;
		}

		if (vec_out) {
			*vec_out = subsys_to_eye_vec;
			vm_vec_negate(vec_out);
		}

		if ( dot < 0 )
			return 0;
	}

	// See if ray from eye to subsystem actually hits close enough to the subsystem position
	vm_vec_normalized_dir(&eye_to_pos, subsys_pos, eye_pos);
	vm_vec_scale_add(&terminus, eye_pos, &eye_to_pos, 100000.0f);

	ship_model_start(objp);

	mc.model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num;			// Fill in the model to check
	mc.orient = &objp->orient;										// The object's orientation
	mc.pos = &objp->pos;												// The object's position
	mc.p0 = eye_pos;													// Point 1 of ray to check
	mc.p1 = &terminus;												// Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL;	

	model_collide(&mc);

	ship_model_stop(objp);

	if ( !mc.num_hits ) {
		return 0;
	}	

	// determine if hitpos is close enough to subsystem
	dist = vm_vec_dist(&mc.hit_point_world, subsys_pos);

	if ( dist <= subsys->system_info->radius ) {
		return 1;
	}
	
	return 0;
}

// try to find a subsystem matching 'type' inside the ship, and that is 
// not destroyed.  If cannot find one, return NULL.
ship_subsys *ship_return_next_subsys(ship *shipp, int type, vec3d *attacker_pos)
{
	ship_subsys	*ssp;

	Assert ( type >= 0 && type < SUBSYSTEM_MAX );

	// If aggregate total is 0, that means no subsystem is alive of that type
	if ( shipp->subsys_info[type].total_hits <= 0.0f )
		return NULL;

	// loop through all the subsystems, if we find a match that has some strength, return it
	ssp = ship_get_best_subsys_to_attack(shipp, type, attacker_pos);

	return ssp;
}

// Determine if a ship is threatened by any dumbfire projectiles (laser or missile)
// input:	sp	=>	pointer to ship that might be threatened
// exit:		0 =>	no dumbfire threats
//				1 =>	at least one dumbfire threat
//
// NOTE: Currently this function is only called periodically from the HUD code for the 
//       player ship.
int ship_dumbfire_threat(ship *sp)
{
	if ( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) ) {
		return 0;
	}

	if (ai_endangered_by_weapon(&Ai_info[sp->ai_index]) > 0) {
		return 1;
	} 

	return 0;
}

// Return !0 if there is a missile in the air homing on shipp
int ship_has_homing_missile_locked(ship *shipp)
{
	object		*locked_objp, *A;
	weapon		*wp;
	weapon_info	*wip;
	missile_obj	*mo;

	Assert(shipp->objnum >= 0 && shipp->objnum < MAX_OBJECTS);
	locked_objp = &Objects[shipp->objnum];

	// check for currently locked missiles (highest precedence)
	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		A = &Objects[mo->objnum];

		if (A->type != OBJ_WEAPON)
			continue;

		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));
		wp = &Weapons[A->instance];
		wip = &Weapon_info[wp->weapon_info_index];

		if ( wip->subtype != WP_MISSILE )
			continue;

		if ( !(wip->wi_flags & (WIF_HOMING_ASPECT|WIF_HOMING_HEAT) ) )
			continue;

		if (wp->homing_object == locked_objp) {
			return 1;
		}
	}	// end for 

	return 0;
}

// Return !0 if there is some ship attempting to lock onto shipp
int ship_is_getting_locked(ship *shipp)
{
	ship_obj	*so;
	object	*objp;
	ai_info	*aip;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		aip = &Ai_info[Ships[objp->instance].ai_index];

		if ( aip->target_objnum == shipp->objnum ) {
			if ( aip->aspect_locked_time > 0.1f ) {
				float dist, wep_range;
				dist = vm_vec_dist_quick(&objp->pos, &Objects[shipp->objnum].pos);
				wep_range = ship_get_secondary_weapon_range(&Ships[objp->instance]);
				if ( wep_range > dist ) {
					nprintf(("Alan","AI ship is seeking lock\n"));
					return 1;
				}
			}
		}
	}

	return 0;
}

// Determine if a ship is threatened by attempted lock or actual lock
// input:	sp	=>	pointer to ship that might be threatened
// exit:		0 =>	no lock threats of any kind
//				1 =>	at least one attempting lock (no actual locks)
//				2 =>	at least one lock (possible other attempting locks)
//
// NOTE: Currently this function is only called periodically from the HUD code for the 
//       player ship.
int ship_lock_threat(ship *sp)
{
	if ( ship_has_homing_missile_locked(sp) ) {
		return 2;
	}

	if ( ship_is_getting_locked(sp) ) {
		return 1;
	}

	return 0;
}

// converts a bitmask, such as 0x08, into the bit number this would be (3 in this case)
// NOTE: Should move file to something like Math_utils.
int bitmask_2_bitnum(int num)
{
	int i;

	for (i=0; i<32; i++)
		if (num & (1 << i))
			return i;

	return -1;
}

// Get a text description of a ships orders. 
//
//	input:	outbuf	=>		buffer to hold orders string
//				sp			=>		ship pointer to extract orders from
//
// exit:		NULL		=>		printable orders are not applicable
//				non-NULL	=>		pointer to string that was passed in originally
//
// This function is called from HUD code to get a text description
// of what a ship's orders are.  Feel free to use this function if 
// it suits your needs for something.
//
char *ship_return_orders(char *outbuf, ship *sp)
{
	ai_info	*aip;
	ai_goal	*aigp;
	char		*order_text;
	
	Assert(sp->ai_index >= 0);
	aip = &Ai_info[sp->ai_index];

	// The active goal is always in the first element of aip->goals[]
	aigp = &aip->goals[0];

	if ( aigp->ai_mode < 0 ) 
		return NULL;

	order_text = Ai_goal_text(bitmask_2_bitnum(aigp->ai_mode));
	if ( order_text == NULL )
		return NULL;

	strcpy(outbuf, order_text);
	switch (aigp->ai_mode ) {

		case AI_GOAL_FORM_ON_WING:
		case AI_GOAL_GUARD_WING:
		case AI_GOAL_CHASE_WING:
			if ( aigp->ship_name ) {
				strcat(outbuf, aigp->ship_name);
				strcat(outbuf, XSTR( "'s Wing", 494));
			} else {
				strcpy(outbuf, XSTR( "no orders", 495));
			}
			break;
	
		case AI_GOAL_CHASE:
		case AI_GOAL_DOCK:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_REARM_REPAIR:
			if ( aigp->ship_name ) {
				strcat(outbuf, aigp->ship_name);
			} else {
				strcpy(outbuf, XSTR( "no orders", 495));
			}
			break;

		case AI_GOAL_DESTROY_SUBSYSTEM: {
			char name[NAME_LENGTH];
			if ( aip->targeted_subsys != NULL ) {
				sprintf(outbuf, XSTR( "atk %s %s", 496), aigp->ship_name, hud_targetbox_truncate_subsys_name(aip->targeted_subsys->system_info->name));
				strcat(outbuf, name);
			} else {
				strcpy(outbuf, XSTR( "no orders", 495) );
			}
			break;
		}

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			// don't do anything, all info is in order_text
			break;

		case AI_GOAL_FLY_TO_SHIP:
			strcpy(outbuf, "Flying to ship");
			break;


		default:
			return NULL;
	}

	return outbuf;
}

// return the amount of time until ship reaches its goal (in MM:SS format)
//	input:	outbuf	=>		buffer to hold orders string
//				sp			=>		ship pointer to extract orders from
//
// exit:		NULL		=>		printable orders are not applicable
//				non-NULL	=>		pointer to string that was passed in originally
//
// This function is called from HUD code to get a text description
// of what a ship's orders are.  Feel free to use this function if 
// it suits your needs for something.
char *ship_return_time_to_goal(char *outbuf, ship *sp)
{
	ai_info	*aip;
	int		time, seconds, minutes;
	float		dist = 0.0f;
	object	*objp;	
	float		min_speed, max_speed;

	objp = &Objects[sp->objnum];
	aip = &Ai_info[sp->ai_index];

	min_speed = objp->phys_info.speed;

	// Goober5000 - handle cap
	if (aip->waypoint_speed_cap >= 0)
		max_speed = MIN(sp->current_max_speed, aip->waypoint_speed_cap);
	else
		max_speed = sp->current_max_speed;

	if ( aip->mode == AIM_WAYPOINTS ) {
		waypoint_list	*wpl;
		min_speed = 0.9f * max_speed;
		if (aip->wp_list >= 0) {
			wpl = &Waypoint_lists[aip->wp_list];
			dist += vm_vec_dist_quick(&objp->pos, &wpl->waypoints[aip->wp_index]);
			for (int i=aip->wp_index; i<wpl->count-1; i++) {
				dist += vm_vec_dist_quick(&wpl->waypoints[i], &wpl->waypoints[i+1]);
			}
		}

		if ( dist < 1.0f) {
			return NULL;
		}	

		if ( (Objects[sp->objnum].phys_info.speed <= 0) || (max_speed <= 0.0f) ) {
			time = -1;
		} else {
			float	speed;

			speed = objp->phys_info.speed;

			if (speed < min_speed)
				speed = min_speed;
			time = fl2i(dist/speed);
		}

	} else if ( (aip->mode == AIM_DOCK) && (aip->submode < AIS_DOCK_4) ) {
		time = hud_get_dock_time( objp );
	} else {
		// don't return anytime for time to except for waypoints and actual docking.
		return NULL;
	}

/*
	} else if ( aip->goal_objnum >= 0 ) {
		dist = vm_vec_dist_quick(&Objects[aip->goal_objnum].pos, &objp->pos);
		min_speed = sip->max_speed/4.0f;
	} else if ( aip->target_objnum >= 0 ) {
		if ( aip->guard_objnum < 0 ) {
			dist = vm_vec_dist_quick(&Objects[aip->target_objnum].pos, &objp->pos);
			min_speed = sip->max_speed/4.0f;
		}
	}
*/

	if ( time >= 0 ) {
		minutes = time/60;
		seconds = time%60;
		if ( minutes > 99 ) {
			minutes = 99;
			seconds = 99;
		}
		sprintf(outbuf, NOX("%02d:%02d"), minutes, seconds);
	} else {
		sprintf( outbuf, XSTR( "Unknown", 497) );
	}

	return outbuf;
}


// Called to check if any AI ships might reveal the cargo of any cargo containers.
//
// This is called once a frame, but a global timer 'Ship_cargo_check_timer' will limit this
// function to being called every SHIP_CARGO_CHECK_INTERVAL ms.  I think that should be sufficient.
//
// NOTE: This function uses CARGO_REVEAL_DISTANCE from the HUD code... which is a multiple of
//       the ship radius that is used to determine when cargo is detected.  AI ships do not 
//       have to have the ship targeted to reveal cargo.  The player is ignored in this function.
#define SHIP_CARGO_CHECK_INTERVAL	1000
void ship_check_cargo_all()
{
	object	*cargo_objp;
	ship_obj	*cargo_so, *ship_so;
	ship		*cargo_sp, *ship_sp;
	float		dist_squared, limit_squared;

	// I don't want to do this check every frame, so I made a global timer to limit check to
	// every SHIP_CARGO_CHECK_INTERVAL ms.
	if ( !timestamp_elapsed(Ship_cargo_check_timer) ) {
		return;
	} else {
		Ship_cargo_check_timer = timestamp(SHIP_CARGO_CHECK_INTERVAL);
	}

	// Check all friendly fighter/bombers against all non-friendly cargo containers that don't have
	// cargo revealed

	// for now just locate a capital ship on the same team:
	cargo_so = GET_FIRST(&Ship_obj_list);
	while(cargo_so != END_OF_LIST(&Ship_obj_list)){
		cargo_sp = &Ships[Objects[cargo_so->objnum].instance];
		if ( (Ship_info[cargo_sp->ship_info_index].flags & SIF_CARGO) && (cargo_sp->team != Player_ship->team) ) {
			
			// If the cargo is revealed, continue on to next hostile cargo
			if ( cargo_sp->flags & SF_CARGO_REVEALED ) {
				goto next_cargo;
			}

			// check against friendly fighter/bombers + cruiser/freighter/transport
			// IDEA: could cull down to fighter/bomber if we want this to run a bit quicker
			for ( ship_so=GET_FIRST(&Ship_obj_list); ship_so != END_OF_LIST(&Ship_obj_list); ship_so=GET_NEXT(ship_so) )
			{
				ship_sp = &Ships[Objects[ship_so->objnum].instance];
				// only consider friendly ships
				if (ship_sp->team != Player_ship->team) {
					continue;
				}

				// ignore the player
				if ( ship_so->objnum == OBJ_INDEX(Player_obj) ) {
					continue;
				}

				// if this ship is a small or big ship
				if ( Ship_info[ship_sp->ship_info_index].flags & (SIF_SMALL_SHIP|SIF_BIG_SHIP) ) {
					cargo_objp = &Objects[cargo_sp->objnum];
					// use square of distance, faster than getting real distance (which will use sqrt)
					dist_squared = vm_vec_dist_squared(&cargo_objp->pos, &Objects[ship_sp->objnum].pos);
					limit_squared = (cargo_objp->radius+CARGO_RADIUS_DELTA)*(cargo_objp->radius+CARGO_RADIUS_DELTA);
					if ( dist_squared <= MAX(limit_squared, CARGO_REVEAL_MIN_DIST*CARGO_REVEAL_MIN_DIST) ) {
						ship_do_cargo_revealed( cargo_sp );
						break;	// break out of for loop, move on to next hostile cargo
					}
				}
			} // end for
		}
next_cargo:
		cargo_so = GET_NEXT(cargo_so);
	} // end while
}


// Maybe warn player about this attacking ship.  This is called once per frame, and the
// information about the closest attacking ship comes for free, since this function is called
// from HUD code which has already determined the closest enemy attacker and the distance.
//
// input:	enemy_sp	=>	ship pointer to the TEAM_ENEMY ship attacking the player
//				dist		=>	the distance of the enemy to the player
//
// NOTE: there are no filters on enemy_sp, so it could be any ship type
//
#define PLAYER_ALLOW_WARN_INTERVAL		60000		// minimum time between warnings
#define PLAYER_CHECK_WARN_INTERVAL		300		// how often we check for warnings
#define PLAYER_MAX_WARNINGS				2			// max number of warnings player can receive in a mission
#define PLAYER_MIN_WARN_DIST				100		// minimum distance attacking ship can be from player and still allow warning
#define PLAYER_MAX_WARN_DIST				1000		// maximum distance attacking ship can be from plyaer and still allow warning

void ship_maybe_warn_player(ship *enemy_sp, float dist)
{
	float		fdot; //, rdot, udot;
	vec3d	vec_to_target;
	int		msg_type; //, on_right;

	// First check if the player has reached the maximum number of warnings for a mission
	if ( Player->warn_count >= PLAYER_MAX_WARNINGS ) {
		return;
	}

	// Check if enough time has elapsed since last warning, if not - leave
	if ( !timestamp_elapsed(Player->allow_warn_timestamp) ) {
		return;
	}

	// Check to see if check timer has elapsed.  Necessary, since we don't want to check each frame
	if ( !timestamp_elapsed(Player->check_warn_timestamp ) ) {
		return;
	}
	Player->check_warn_timestamp = timestamp(PLAYER_CHECK_WARN_INTERVAL);

	// only allow warnings if within a certain distance range
	if ( dist < PLAYER_MIN_WARN_DIST || dist > PLAYER_MAX_WARN_DIST ) {
		return;
	}

	// only warn if a fighter or bomber is attacking the player
	if ( !(Ship_info[enemy_sp->ship_info_index].flags & SIF_SMALL_SHIP) ) {
		return;
	}

	// get vector from player to target
	vm_vec_normalized_dir(&vec_to_target, &Objects[enemy_sp->objnum].pos, &Eye_position);

	// ensure that enemy fighter is oriented towards player
	fdot = vm_vec_dot(&Objects[enemy_sp->objnum].orient.vec.fvec, &vec_to_target);
	if ( fdot > -0.7 ) {
		return;
	}

	fdot = vm_vec_dot(&Player_obj->orient.vec.fvec, &vec_to_target);

	msg_type = -1;

	// check if attacking ship is on six.  return if not far enough behind player.
	if ( fdot > -0.7 )
		return;

	msg_type = MESSAGE_CHECK_6;
/*
		goto warn_player_done;
	}

	// see if attacking ship is in front of ship (then do nothing)
	if ( fdot > 0.7 ) {
		return;
	}

	// ok, ship is on 3 or 9.  Find out which
	rdot = vm_vec_dot(&Player_obj->orient.rvec, &vec_to_target);
	if ( rdot > 0 ) {
		on_right = 1;
	} else {
		on_right = 0;
	}

	// now determine if ship is high or low
	udot = vm_vec_dot(&Player_obj->orient.uvec, &vec_to_target);
	if ( udot < -0.8 ) {
		return;	// if ship is attacking from directly below, no warning given
	}

	if ( udot > 0 ) {
		if ( on_right ) {
			msg_type = MESSAGE_CHECK_3_HIGH;
		} else {
			msg_type = MESSAGE_CHECK_9_HIGH;
		}
	} else {
		if ( on_right ) {
			msg_type = MESSAGE_CHECK_3_LOW;
		} else {
			msg_type = MESSAGE_CHECK_9_LOW;
		}
	}

warn_player_done:
*/

	if ( msg_type != -1 ) {
		int ship_index;

		// multiplayer tvt - this is client side.
		if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM) && (Net_player != NULL)){
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED, 0.0f, -1, 0, Net_player->p_info.team );
		} else {
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED );
		}

		if ( ship_index >= 0 ) {
			// multiplayer - make sure I just send to myself
			if(Game_mode & GM_MULTIPLAYER){
				message_send_builtin_to_player(msg_type, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, MY_NET_PLAYER_NUM, -1);
			} else {
				message_send_builtin_to_player(msg_type, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
			}
			Player->allow_warn_timestamp = timestamp(PLAYER_ALLOW_WARN_INTERVAL);
			Player->warn_count++;
//			nprintf(("Alan","Warning given for ship name: %s\n", enemy_sp->ship_name));
		}
	}
}

// player has just killed a ship, maybe offer send a 'good job' message
#define PLAYER_MAX_PRAISES					10			// max number of praises player can receive in a mission
void ship_maybe_praise_player(ship *deader_sp)
{
	if ( myrand()&1 ) {
		return;
	}

	// First check if the player has reached the maximum number of praises for a mission
	if ( Player->praise_count >= PLAYER_MAX_PRAISES ) {
		return;
	}

	// Check if enough time has elapsed since last praise, if not - leave
	if ( !timestamp_elapsed(Player->allow_praise_timestamp) ) {
		return;
	}

	// make sure player is not a traitor
	if (Player_ship->team == Iff_traitor) {
		return;
	}

	// only praise if killing an enemy!
	if ( deader_sp->team == Player_ship->team ) {
		return;
	}

	// don't praise the destruction of navbuoys, cargo or other non-flyable ship types
	if ( (Ship_info[deader_sp->ship_info_index].class_type > 0) && !(Ship_types[Ship_info[deader_sp->ship_info_index].class_type].message_bools & STI_MSG_PRAISE_DESTRUCTION) ) {
		return;
	}

	// There is already a praise pending
	if ( Player->praise_delay_timestamp ) {
		return;
	}

	// We don't want to praise the player right away.. it is more realistic to wait a moment
	Player->praise_delay_timestamp = timestamp_rand(1000, 2000);
}

// player has just killed a ship, maybe offer send a 'good job' message
#define PLAYER_ASK_HELP_INTERVAL			60000		// minimum time between praises
#define PLAYER_MAX_ASK_HELP				10			// max number of warnings player can receive in a mission
#define ASK_HELP_SHIELD_PERCENT			0.1		// percent shields at which ship will ask for help
#define ASK_HELP_HULL_PERCENT				0.3		// percent hull at which ship will ask for help
#define AWACS_HELP_HULL_HI					0.75		// percent hull at which ship will ask for help
#define AWACS_HELP_HULL_LOW				0.25		// percent hull at which ship will ask for help

// -----------------------------------------------------------------------------
void awacs_maybe_ask_for_help(ship *sp, int multi_team_filter)
{
	// Goober5000 - bail if not in main fs2 campaign
	// (stupid coders... it's the FREDder's responsibility to add this message)
	if (stricmp(Campaign.filename, "freespace2") || !(Game_mode & GM_CAMPAIGN_MODE))
		return;

	object *objp;
	int message = -1;
	objp = &Objects[sp->objnum];

	if ( objp->hull_strength < ( (AWACS_HELP_HULL_LOW + 0.01f *(static_rand(objp-Objects) & 5)) * sp->ship_max_hull_strength) ) {
		// awacs ship below 25 + (0-4) %
		if (!(sp->awacs_warning_flag & AWACS_WARN_25)) {
			message = MESSAGE_AWACS_25;
			sp->awacs_warning_flag |=  AWACS_WARN_25;
		}
	} else if ( objp->hull_strength < ( (AWACS_HELP_HULL_HI + 0.01f*(static_rand(objp-Objects) & 5)) * sp->ship_max_hull_strength) ) {
		// awacs ship below 75 + (0-4) %
		if (!(sp->awacs_warning_flag & AWACS_WARN_75)) {
			message = MESSAGE_AWACS_75;
			sp->awacs_warning_flag |=  AWACS_WARN_75;
		}
	}

	if (message >= 0) {
		message_send_builtin_to_player(message, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
		Player->allow_ask_help_timestamp = timestamp(PLAYER_ASK_HELP_INTERVAL);
		Player->ask_help_count++;
	}
}

// -----------------------------------------------------------------------------
void ship_maybe_ask_for_help(ship *sp)
{
	object *objp;
	int multi_team_filter = -1;

	// First check if the player has reached the maximum number of ask_help's for a mission
	if (Player->ask_help_count >= PLAYER_MAX_ASK_HELP)
		return;

	// Check if enough time has elapsed since last help request, if not - leave
	if (!timestamp_elapsed(Player->allow_ask_help_timestamp))
		return;

	// make sure player is on their team and not a traitor
	if ((Player_ship->team != sp->team) || (Player_ship->team == Iff_traitor))
		return;

	objp = &Objects[sp->objnum];

	// don't let the player ask for help!
	if (objp->flags & OF_PLAYER_SHIP)
		return;

	// determine team filter if TvT
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
		multi_team_filter = sp->team;

	// handle awacs ship as a special case
	if (Ship_info[sp->ship_info_index].flags & SIF_HAS_AWACS)
	{
		awacs_maybe_ask_for_help(sp, multi_team_filter);
		return;
	}

	// for now, only have wingman ships request help
	if (!(sp->flags & SF_FROM_PLAYER_WING))
		return;

	// first check if hull is at a critical level
	if (objp->hull_strength < ASK_HELP_HULL_PERCENT * sp->ship_max_hull_strength)
		goto play_ask_help;

	// check if shields are near critical level
	if (objp->flags & OF_NO_SHIELDS)
		return;	// no shields on ship, no don't check shield levels

	if (shield_get_strength(objp) > (ASK_HELP_SHIELD_PERCENT * shield_get_max_strength(objp)))
		return;

play_ask_help:

	Assert(Ship_info[sp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER) );	// get Alan
	if (!(sp->flags2 & SF2_NO_BUILTIN_MESSAGES)) // Karajorma - Only unsilenced ships should ask for help
	{
	message_send_builtin_to_player(MESSAGE_HELP, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_ask_help_timestamp = timestamp(PLAYER_ASK_HELP_INTERVAL);

	// prevent overlap with death message
	if (timestamp_until(Player->allow_scream_timestamp) < 15000)
		Player->allow_scream_timestamp = timestamp(15000);

	Player->ask_help_count++;
	}
}

// The player has just entered death roll, maybe have wingman mourn the loss of the player
void ship_maybe_lament()
{
	int ship_index;

	// no. because in multiplayer, its funny
	if (Game_mode & GM_MULTIPLAYER)
		return;

	if (rand() % 4 == 0)
	{
		ship_index = ship_get_random_player_wing_ship(SHIP_GET_UNSILENCED);
		if (ship_index >= 0)
			message_send_builtin_to_player(MESSAGE_PLAYER_DIED, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
	}
}

#define PLAYER_SCREAM_INTERVAL		60000
#define PLAYER_MAX_SCREAMS				10

// play a death scream for a ship
void ship_scream(ship *sp)
{
	int multi_team_filter = -1;

	// bogus
	if (sp == NULL)
		return;

	// multiplayer tvt
	if ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
		multi_team_filter = sp->team;

	// Bail if the ship is silenced
	if (sp->flags2 & SF2_NO_BUILTIN_MESSAGES)
	{
		return;
	}

	message_send_builtin_to_player(MESSAGE_WINGMAN_SCREAM, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_scream_timestamp = timestamp(PLAYER_SCREAM_INTERVAL);
	Player->scream_count++;

	sp->flags |= SF_SHIP_HAS_SCREAMED;

	// prevent overlap with help messages
	if (timestamp_until(Player->allow_ask_help_timestamp) < 15000)
		Player->allow_ask_help_timestamp = timestamp(15000);
}

// ship has just died, maybe play a scream.
//
// NOTE: this is only called for ships that are not the player ship
extern int Cmdline_wcsaga;
void ship_maybe_scream(ship *sp)
{
	// bail if screaming is disabled
	if (sp->flags2 & SF2_NO_DEATH_SCREAM)
		return;

	// if screaming is enabled, skip all checks
	if (!(sp->flags2 & SF2_ALWAYS_DEATH_SCREAM))
	{
		// for WCSaga, only do a subset of the checks
		if (Cmdline_wcsaga)
		{
			// only scream 50% of the time
			if (rand() & 1)
				return;

			// check if enough time has elapsed since last scream; if not, leave
			if (!timestamp_elapsed(Player->allow_scream_timestamp))
				return;
		}
		// otherwise do all checks
		else
		{
			// bail if this ship isn't from the player wing
			if (!(sp->flags & SF_FROM_PLAYER_WING))
				return;

			// only scream 50% of the time
			if (rand() & 1)
				return;

			// first check if the player has reached the maximum number of screams for a mission
			if (Player->scream_count >= PLAYER_MAX_SCREAMS)
				return;

			// if on different teams (i.e. team v. team games in multiplayer), no scream
			if (Player_ship->team != sp->team)
				return;

			// check if enough time has elapsed since last scream; if not, leave
			if (!timestamp_elapsed(Player->allow_scream_timestamp))
				return;
		}
	}

	ship_scream(sp);
}

// maybe tell player that we've requested a support ship
#define PLAYER_REQUEST_REPAIR_MSG_INTERVAL	240000
void ship_maybe_tell_about_rearm(ship *sp)
{
	weapon_info *wip;

	if (!timestamp_elapsed(Player->request_repair_timestamp))
		return;

	if (Player_ship->team == Iff_traitor)
		return;

	// Silent ships should remain just that
	if (sp->flags2 & SF2_NO_BUILTIN_MESSAGES)
	{
		return;
	}

	// AL 1-4-98:	If ship integrity is low, tell player you want to get repaired.  Otherwise, tell
	// the player you want to get re-armed.

	int message_type = -1;
	int heavily_damaged = (get_hull_pct(&Objects[sp->objnum]) < 0.4);

	if (heavily_damaged || (sp->flags & SF_DISABLED))
	{
		message_type = MESSAGE_REPAIR_REQUEST;
	}
	else
	{
		int i;
		ship_weapon *swp;

		swp = &sp->weapons;
		for (i = 0; i < swp->num_secondary_banks; i++)
		{
			if (swp->secondary_bank_start_ammo[i] > 0)
			{
				if (swp->secondary_bank_ammo[i] / swp->secondary_bank_start_ammo[i] < 0.5f)
				{
					message_type = MESSAGE_REARM_REQUEST;
					break;
				}
			}
		}

		// also check ballistic primaries - Goober5000
		if (sp->flags & SIF_BALLISTIC_PRIMARIES)
		{
			for (i = 0; i < swp->num_primary_banks; i++)
			{
				wip = &Weapon_info[swp->primary_bank_weapons[i]];

				if (wip->wi_flags2 & WIF2_BALLISTIC)
				{
					if (swp->primary_bank_start_ammo[i] > 0)
					{
						if (swp->primary_bank_ammo[i] / swp->primary_bank_start_ammo[i] < 0.3f)
						{
							message_type = MESSAGE_REARM_REQUEST;
							break;
						}
					}
				}
			}
		}
	}

	int multi_team_filter = -1;

	// multiplayer tvt
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
		multi_team_filter = sp->team;


	if (message_type >= 0)
	{
		if (rand() & 1)
			message_send_builtin_to_player(message_type, sp, MESSAGE_PRIORITY_NORMAL, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter);

		Player->request_repair_timestamp = timestamp(PLAYER_REQUEST_REPAIR_MSG_INTERVAL);
	}
}

// The current primary weapon or link status for a ship has changed.. notify clients if multiplayer
//
// input:	sp			=>	pointer to ship that modified primaries
void ship_primary_changed(ship *sp)
{
	int i;
	ship_weapon	*swp;
	swp = &sp->weapons;


	if (sp->flags & SF_PRIMARY_LINKED) {
		// if we are linked now find any body who is down and flip them up
		for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (swp->primary_animation_position[i] == MA_POS_NOT_SET) {
				if ( model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, 1) ) {
					swp->primary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i);
					swp->primary_animation_position[i] = MA_POS_SET;
				} else {
					swp->primary_animation_position[i] = MA_POS_READY;
				}
			}
		}
	} else {
		// find anything that is up that shouldn't be
		for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (i == swp->current_primary_bank) {
				// if the current bank is down raise it up
				if (swp->primary_animation_position[i] == MA_POS_NOT_SET) {
					if ( model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, 1) ) {
						swp->primary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i);
						swp->primary_animation_position[i] = MA_POS_SET;
					} else {
						swp->primary_animation_position[i] = MA_POS_READY;
					}
				}
			} else {
				// everyone else should be down, if they are not make them so
				if (swp->primary_animation_position[i] != MA_POS_NOT_SET) {
					model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, -1);
					swp->primary_animation_position[i] = MA_POS_NOT_SET;
				}
			}
		}
	}

#if 0
	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) )
		return;

	Assert(sp);

	if ( MULTIPLAYER_MASTER )
		send_ship_weapon_change( sp, MULTI_PRIMARY_CHANGED, swp->current_primary_bank, (sp->flags & SF_PRIMARY_LINKED)?1:0 );
#endif
}

// The current secondary weapon or dual-fire status for a ship has changed.. notify clients if multiplayer
//
// input:	sp					=>	pointer to ship that modified secondaries
void ship_secondary_changed(ship *sp)
{
	Assert( sp != NULL );

	int i;
	ship_weapon	*swp = &sp->weapons;

	// find anything that is up that shouldn't be
	if (timestamp() > 10) {
		for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (i == swp->current_secondary_bank) {
				// if the current bank is down raise it up
				if (swp->secondary_animation_position[i] == MA_POS_NOT_SET) {
					if ( model_anim_start_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i, 1) ) {
						swp->secondary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i);
						swp->secondary_animation_position[i] = MA_POS_SET;
					} else {
						swp->secondary_animation_position[i] = MA_POS_READY;
					}
				}
			} else {
				// everyone else should be down, if they are not make them so
				if (swp->secondary_animation_position[i] != MA_POS_NOT_SET) {
					model_anim_start_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i, -1);
					swp->secondary_animation_position[i] = MA_POS_NOT_SET;
				}
			}
		}
	}

#if 0
	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) ){
		return;
	}

	Assert(sp);

	if ( MULTIPLAYER_MASTER )
		send_ship_weapon_change( sp, MULTI_SECONDARY_CHANGED, swp->current_secondary_bank, (sp->flags & SF_SECONDARY_DUAL_FIRE)?1:0 );
#endif
}

int ship_get_SIF(ship *shipp)
{
	return Ship_info[shipp->ship_info_index].flags;
}

int ship_get_SIF(int sh)
{
	return Ship_info[Ships[sh].ship_info_index].flags;
}

int ship_get_by_signature(int signature)
{
	ship_obj *so;
		
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		// if we found a matching ship object signature
		if((Objects[so->objnum].signature == signature) && (Objects[so->objnum].type == OBJ_SHIP)){
			return Objects[so->objnum].instance;
		}
	}

	// couldn't find the ship
	return -1;
}

// function which gets called when the cargo of a ship is revealed.  Happens at two different locations
// (at least when this function was written), one for the player, and one for AI ships.  Need to send stuff
// to clients in multiplayer game.
void ship_do_cargo_revealed( ship *shipp, int from_network )
{
	// don't do anything if we already know the cargo
	if ( shipp->flags & SF_CARGO_REVEALED ){
		return;
	}
	
	nprintf(("Network", "Revealing cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_revealed_packet( shipp );		
	}

	shipp->flags |= SF_CARGO_REVEALED;
	shipp->time_cargo_revealed = Missiontime;	

	// if the cargo is something other than "nothing", then make a log entry
	if ( stricmp(Cargo_names[shipp->cargo1 & CARGO_INDEX_MASK], NOX("nothing")) ){
		mission_log_add_entry(LOG_CARGO_REVEALED, shipp->ship_name, NULL, (shipp->cargo1 & CARGO_INDEX_MASK) );
	}	
}

void ship_do_cap_subsys_cargo_revealed( ship *shipp, ship_subsys *subsys, int from_network )
{
	// don't do anything if we already know the cargo
	if (subsys->flags & SSF_CARGO_REVEALED) {
		return;
	}

	
	nprintf(("Network", "Revealing cap ship subsys cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_revealed_packet( shipp, subsystem_index );		
	}

	subsys->flags |= SSF_CARGO_REVEALED;
	subsys->time_subsys_cargo_revealed = Missiontime;

	// if the cargo is something other than "nothing", then make a log entry
	if ( (subsys->subsys_cargo_name > 0) && stricmp(Cargo_names[subsys->subsys_cargo_name], NOX("nothing")) ){
		mission_log_add_entry(LOG_CAP_SUBSYS_CARGO_REVEALED, shipp->ship_name, subsys->system_info->name, subsys->subsys_cargo_name );
	}	
}

// function which gets called when the cargo of a ship is hidden by the sexp.  Need to send stuff
// to clients in multiplayer game.
void ship_do_cargo_hidden( ship *shipp, int from_network )
{
	// don't do anything if the cargo is already hidden
	if ( !(shipp->flags & SF_CARGO_REVEALED) )
	{
		return;
	}
	
	nprintf(("Network", "Hiding cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_hidden_packet( shipp );		
	}

	shipp->flags &= ~SF_CARGO_REVEALED;

	// don't log that the cargo was hidden and don't reset the time cargo revealed
}

void ship_do_cap_subsys_cargo_hidden( ship *shipp, ship_subsys *subsys, int from_network )
{
	// don't do anything if the cargo is already hidden
	if (!(subsys->flags & SSF_CARGO_REVEALED))
	{
		return;
	}

	
	nprintf(("Network", "Hiding cap ship subsys cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_hidden_packet( shipp, subsystem_index );		
	}

	subsys->flags &= ~SSF_CARGO_REVEALED;

	// don't log that the cargo was hidden and don't reset the time cargo revealed
}

// Return the range of the currently selected secondary weapon
// NOTE: If there is no missiles left in the current bank, range returned is 0
float ship_get_secondary_weapon_range(ship *shipp)
{
	float srange=0.0f;

	ship_weapon	*swp;
	swp = &shipp->weapons;
	if ( swp->current_secondary_bank >= 0 ) {
		weapon_info	*wip;
		int bank=swp->current_secondary_bank;
		wip = &Weapon_info[swp->secondary_bank_weapons[bank]];
		if ( swp->secondary_bank_ammo[bank] > 0 ) {
			srange = wip->max_speed * wip->lifetime;
		}
	}

	return srange;
}

// Goober5000 - added for ballistic primaries
// Determine the number of primary ammo units allowed max for a ship
int get_max_ammo_count_for_primary_bank(int ship_class, int bank, int ammo_type)
{
	float capacity, size;
	
	if (!(Ship_info[ship_class].flags & SIF_BALLISTIC_PRIMARIES))
	{
		return 0;
	}

	capacity = (float) Ship_info[ship_class].primary_bank_ammo_capacity[bank];
	size = (float) Weapon_info[ammo_type].cargo_size;
	return  fl2i((capacity / size)+0.5f);
}

// Determine the number of secondary ammo units (missile/bomb) allowed max for a ship
int get_max_ammo_count_for_bank(int ship_class, int bank, int ammo_type)
{
	float capacity, size;

	capacity = (float) Ship_info[ship_class].secondary_bank_ammo_capacity[bank];
	size = (float) Weapon_info[ammo_type].cargo_size;
	return (int) (capacity / size);
}

// Page in bitmaps for all the ships in this level

void ship_page_in()
{
	int i, j, k;
	int num_subsystems_needed = 0;

	int *ship_class_used = NULL;

	ship_class_used = new int[Num_ship_classes];

	Verify( ship_class_used != NULL );

	// Mark all ship classes as not used
	memset( ship_class_used, 0, Num_ship_classes * sizeof(int) );

	// Mark any support ship types as used
	for (i = 0; i < Num_ship_classes; i++)	{
		if (Ship_info[i].flags & SIF_SUPPORT) {
			nprintf(( "Paging", "Found support ship '%s'\n", Ship_info[i].name ));
			ship_class_used[i]++;

			num_subsystems_needed += Ship_info[i].n_subsystems;
		}
	}
	
	// Mark any ships in the mission as used
	for (i = 0; i < MAX_SHIPS; i++)	{
		if (Ships[i].objnum < 0)
			continue;
	
		nprintf(( "Paging","Found ship '%s'\n", Ships[i].ship_name ));
		ship_class_used[Ships[i].ship_info_index]++;

		// check if we are going to use a Knossos device and make sure the special warp ani gets pre-loaded
		if ( Ship_info[Ships[i].ship_info_index].flags & SIF_KNOSSOS_DEVICE )
			Knossos_warp_ani_used = 1;

		// mark any weapons as being used, saves memory and time if we don't load them all
		ship_weapon *swp = &Ships[i].weapons;

		for (j = 0; j < swp->num_primary_banks; j++)
			weapon_mark_as_used(swp->primary_bank_weapons[j]);

		for (j = 0; j < swp->num_secondary_banks; j++)
			weapon_mark_as_used(swp->secondary_bank_weapons[j]);

		// get weapons for all capship subsystems (turrets)
		ship_subsys *ptr = GET_FIRST(&Ships[i].subsys_list);

		while (ptr != END_OF_LIST(&Ships[i].subsys_list)) {
			for (k = 0; k < MAX_SHIP_PRIMARY_BANKS; k++)
				weapon_mark_as_used(ptr->weapons.primary_bank_weapons[j]);

			for (k = 0; k < MAX_SHIP_SECONDARY_BANKS; k++)
				weapon_mark_as_used(ptr->weapons.secondary_bank_weapons[j]);

			ptr = GET_NEXT(ptr);
		}

		ship_info *sip = &Ship_info[Ships[i].ship_info_index];

		// page in all of the textures if the model is already loaded
		if (sip->model_num >= 0) {
			nprintf(( "Paging", "Paging in textures for ship '%s'\n", Ships[i].ship_name ));
			model_page_in_textures(sip->model_num, Ships[i].ship_info_index);
		}

		// don't need this one anymore, it's already been accounted for
	//	num_subsystems_needed += Ship_info[Ships[i].ship_info_index].n_subsystems;
	}

	// Mark any ships that might warp in in the future as used
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp)) {
		nprintf(( "Paging", "Found future arrival ship '%s'\n", p_objp->name ));
		ship_class_used[p_objp->ship_class]++;

		// This will go through Subsys_index[] and grab all weapons: primary, secondary, and turrets
		for (i = p_objp->subsys_index; i < (p_objp->subsys_index + p_objp->subsys_count); i++) {
			for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
				if (Subsys_status[i].primary_banks[j] >= 0)
					weapon_mark_as_used(Subsys_status[i].primary_banks[j]);
			}

			for (j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++) {
				if (Subsys_status[i].secondary_banks[j] >= 0)
					weapon_mark_as_used(Subsys_status[i].secondary_banks[j]);
			}
		}

		// page in any replacement textures
		if (Ship_info[p_objp->ship_class].model_num >= 0) {
			nprintf(( "Paging", "Paging in textures for future arrival ship '%s'\n", p_objp->name ));
			model_page_in_textures(Ship_info[p_objp->ship_class].model_num, p_objp->ship_class);
		}

		num_subsystems_needed += Ship_info[p_objp->ship_class].n_subsystems;
	}

	// pre-allocate the subsystems, this really only needs to happen for ships
	// which don't exist yet (ie, ships NOT in Ships[])
	ship_allocate_subsystems(num_subsystems_needed, true);

	mprintf(("About to page in ships!\n"));

	// Page in all the ship classes that are used on this level
	int num_ship_types_used = 0;
	int test_id = -1;

	for (i = 0; i < Num_ship_classes; i++) {
		if ( !ship_class_used[i] )
			continue;

		ship_info *sip = &Ship_info[i];
		int model_previously_loaded = -1;
		int ship_previously_loaded = -1;

		num_ship_types_used++;

		// Page in the small hud icons for each ship
		hud_ship_icon_page_in(sip);

		// See if this model was previously loaded by another ship
		for (j = 0; j < Num_ship_classes; j++) {
			if ( (Ship_info[j].model_num > -1) && !stricmp(sip->pof_file, Ship_info[j].pof_file) ) {
				// Model already loaded
				model_previously_loaded = Ship_info[j].model_num;
				ship_previously_loaded = j;

				// the model should already be loaded so this wouldn't take long, but
				// we need to make sure that the load count for the model is correct
				test_id = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
				Assert( test_id == model_previously_loaded );

				break;
			}
		}

		// If the model is previously loaded...
		if (model_previously_loaded >= 0) {
			// If previously loaded model isn't the same ship class...)
			if (ship_previously_loaded != i) {
				// update the model number.
				sip->model_num = model_previously_loaded;

				for (j = 0; j < sip->n_subsystems; j++)
					sip->subsystems[j].model_num = -1;

				ship_copy_subsystem_fixup(sip);

#ifndef NDEBUG
				for (j = 0; j < sip->n_subsystems; j++)
					Assert( sip->subsystems[j].model_num == sip->model_num );
#endif
			} else {
				// Just to be safe (I mean to check that my code works...)
				Assert( sip->model_num >= 0 );
				Assert( sip->model_num == model_previously_loaded );

#ifndef NDEBUG
				for (j = 0; j < sip->n_subsystems; j++) {
					//Assert( sip->subsystems[j].model_num == sip->modelnum );
					if (sip->subsystems[j].model_num != sip->model_num)
						Warning(LOCATION, "Ship '%s' does not have subsystem '%s' linked into the model file, '%s'.", sip->name, sip->subsystems[j].name, sip->pof_file);
				}
#endif
			}
		} else {
			// Model not loaded, so load it
			sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

			Assert( sip->model_num >= 0 );

#ifndef NDEBUG
			// Verify that all the subsystem model numbers are updated
			for (j = 0; j < sip->n_subsystems; j++)
				Assert( sip->subsystems[j].model_num == sip->model_num );	// JAS
#endif
		}

		// more weapon marking, the weapon info in Ship_info[] is the default
		// loadout which isn't specified by missionparse unless it's different
		for (j = 0; j < sip->num_primary_banks; j++)
			weapon_mark_as_used(sip->primary_bank_weapons[j]);

		for (j = 0; j < sip->num_secondary_banks; j++)
			weapon_mark_as_used(sip->secondary_bank_weapons[j]);

		weapon_mark_as_used(sip->cmeasure_type);

		for (j = 0; j < sip->n_subsystems; j++) {
			model_subsystem *msp = &sip->subsystems[j];

			for (k = 0; k < MAX_SHIP_PRIMARY_BANKS; k++)
				weapon_mark_as_used(msp->primary_banks[k]);

			for (k = 0; k < MAX_SHIP_SECONDARY_BANKS; k++)
				weapon_mark_as_used(msp->secondary_banks[k]);
		}

		// Page in the shockwave stuff. -C
		sip->shockwave.load();
	}

	nprintf(( "Paging", "There are %d ship classes used in this mission.\n", num_ship_types_used ));


	// Page in the thruster effects
	//

	// Make sure thrusters are loaded
	if (!Thrust_anim_inited)
		ship_init_thrusters();

	generic_anim *ta;
	thrust_info *thruster;
	for ( i = 0; i < (int)Species_info.size(); i++ ) {
		thruster = &Species_info[i].thruster_info;

		ta = &thruster->flames.normal;
		for ( j = 0; j<ta->num_frames; j++ )	{
			bm_page_in_texture( ta->first_frame + j );
		}

		ta = &thruster->flames.afterburn;
		for ( j = 0; j<ta->num_frames; j++ )	{
			bm_page_in_texture( ta->first_frame + j );
		}

		ta = &thruster->glow.normal;
		// glows are really not anims
		bm_page_in_texture( ta->first_frame );

		ta = &thruster->glow.afterburn;
		// glows are really not anims
		bm_page_in_texture( ta->first_frame );
	}

	// page in insignia bitmaps
	if(Game_mode & GM_MULTIPLAYER){
		for(i=0; i<MAX_PLAYERS; i++){
			if(MULTI_CONNECTED(Net_players[i]) && (Net_players[i].m_player != NULL) && (Net_players[i].m_player->insignia_texture >= 0)){
				bm_page_in_xparent_texture(Net_players[i].m_player->insignia_texture);
			}
		}
	} else {
		if((Player != NULL) && (Player->insignia_texture >= 0)){
			bm_page_in_xparent_texture(Player->insignia_texture);
		}
	}

	// page in wing insignia bitmaps - Goober5000
	for (i = 0; i < MAX_WINGS; i++)
	{
		if (Wings[i].wing_insignia_texture >= 0)
			bm_page_in_xparent_texture(Wings[i].wing_insignia_texture);
	}

	// page in replacement textures - Goober5000
	for (i = 0; i < MAX_SHIPS; i++)
	{
		// is this a valid ship?
		if (Ships[i].objnum >= 0)
		{
			// do we have any textures?
			if (Ships[i].replacement_textures)
			{
				// page in replacement textures
				for (j=0; j<MAX_MODEL_TEXTURES; j++)
				{
					if (Ships[i].replacement_textures[j] != -1)
					{
						bm_page_in_texture( Ships[i].replacement_textures[j] );
					}
				}
			}
		}
	}

	// should never be NULL, this entire function wouldn't work
	delete[] ship_class_used;
	ship_class_used = NULL;

}

// Goober5000 - called from ship_page_in()
void ship_page_in_textures(int ship_index)
{
	int i;
	ship_info *sip;

	if ( (ship_index < 0) || (ship_index > Num_ship_classes) )
		return;


	sip = &Ship_info[ship_index];

	// afterburner
	if ( !generic_bitmap_load(&sip->afterburner_trail) )
		bm_page_in_texture(sip->afterburner_trail.bitmap_id);

	// Bobboau's thruster bitmaps
	// the first set has to be loaded a special way
	if ( !thruster_glow_anim_load(&sip->thruster_glow_info.normal) )
		bm_page_in_texture(sip->thruster_glow_info.normal.first_frame);

	if ( !thruster_glow_anim_load(&sip->thruster_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_glow_info.afterburn.first_frame);

	// everything else is loaded normally
	if ( !generic_bitmap_load(&sip->thruster_secondary_glow_info.normal) )
		bm_page_in_texture(sip->thruster_secondary_glow_info.normal.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_secondary_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_secondary_glow_info.afterburn.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_tertiary_glow_info.normal) )
		bm_page_in_texture(sip->thruster_tertiary_glow_info.normal.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_tertiary_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_tertiary_glow_info.afterburn.bitmap_id);
 
	// splodeing bitmap
	if ( VALID_FNAME(sip->splodeing_texture_name) ) {
		sip->splodeing_texture = bm_load(sip->splodeing_texture_name);
		bm_page_in_texture(sip->splodeing_texture);
	}

	// thruster/particle bitmaps
	for (i = 0; i < (int)sip->normal_thruster_particles.size(); i++) {
		generic_anim_load(&sip->normal_thruster_particles[i].thruster_bitmap);
		bm_page_in_texture(sip->normal_thruster_particles[i].thruster_bitmap.first_frame);
	}

	for (i = 0; i < (int)sip->afterburner_thruster_particles.size(); i++) {
		generic_anim_load(&sip->afterburner_thruster_particles[i].thruster_bitmap);
		bm_page_in_texture(sip->afterburner_thruster_particles[i].thruster_bitmap.first_frame);
	}
}

#define PAGE_OUT_TEXTURE(x) {	\
	if ( (x) >= 0 ) {	\
		if (release) {	\
			bm_release( (x) );	\
			(x) = -1;	\
		} else {	\
			bm_unload( (x) );	\
		}	\
	}	\
}

// unload all textures for a given ship
void ship_page_out_textures(int ship_index, bool release)
{
	int i;
	ship_info *sip;

	if ( (ship_index < 0) || (ship_index > Num_ship_classes) )
		return;


	sip = &Ship_info[ship_index];

	// afterburner
	PAGE_OUT_TEXTURE(sip->afterburner_trail.bitmap_id);

	// thruster bitmaps
	PAGE_OUT_TEXTURE(sip->thruster_glow_info.normal.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_glow_info.afterburn.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_secondary_glow_info.normal.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_secondary_glow_info.afterburn.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_tertiary_glow_info.normal.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_tertiary_glow_info.afterburn.bitmap_id);

	// slodeing bitmap
	PAGE_OUT_TEXTURE(sip->splodeing_texture);

	// thruster/particle bitmaps
	for (i = 0; i < (int)sip->normal_thruster_particles.size(); i++)
		PAGE_OUT_TEXTURE(sip->normal_thruster_particles[i].thruster_bitmap.first_frame);

	for (i = 0; i < (int)sip->afterburner_thruster_particles.size(); i++)
		PAGE_OUT_TEXTURE(sip->afterburner_thruster_particles[i].thruster_bitmap.first_frame);
}

// function to return true if support ships are allowed in the mission for the given object.
//	In single player, must be friendly and not Shivan. (Goober5000 - Shivans can now have support)
//	In multiplayer -- to be coded by Mark Allender after 5/4/98 -- MK, 5/4/98
int is_support_allowed(object *objp)
{
	// check updated mission conditions to allow support

	// none allowed
	if (The_mission.support_ships.max_support_ships == 0)
		return 0;

	// restricted number allowed
	if (The_mission.support_ships.max_support_ships > 0)
	{
		if (The_mission.support_ships.tally >= The_mission.support_ships.max_support_ships)
			return 0;
	}

	ship *shipp = &Ships[objp->instance];

	// make sure, if exiting from bay, that parent ship is in the mission!
	if (The_mission.support_ships.arrival_location == ARRIVE_FROM_DOCK_BAY)
	{
		Assert(The_mission.support_ships.arrival_anchor != -1);

		// ensure it's in-mission
		int temp = ship_name_lookup(Parse_names[The_mission.support_ships.arrival_anchor]);
		if (temp < 0)
		{
			return 0;
		}

		// make sure it's not leaving or blowing up
		if (Ships[temp].flags & (SF_DYING | SF_DEPARTING))
		{
			return 0;
		}

		// also make sure that parent ship's fighterbay hasn't been destroyed
		if (ship_fighterbays_all_destroyed(&Ships[temp]))
		{
			return 0;
		}
	}

	if (Game_mode & GM_NORMAL)
	{
		if ( !(Iff_info[shipp->team].flags & IFFF_SUPPORT_ALLOWED) )
		{
			return 0;
		}
	}
	else
	{
		// multiplayer version behaves differently.  Depending on mode:
		// 1) coop mode -- only available to friendly
		// 2) team v team mode -- availble to either side
		// 3) dogfight -- never

		if(Netgame.type_flags & NG_TYPE_DOGFIGHT)
		{
			return 0;
		}

		if (IS_MISSION_MULTI_COOP)
		{
			if ( !(Iff_info[shipp->team].flags & IFFF_SUPPORT_ALLOWED) )
			{
				return 0;
			}
		}
	}

	// Goober5000 - extra check for existence of support ship
	if ( (The_mission.support_ships.ship_class < 0) &&
		!(The_mission.support_ships.support_available_for_species & (1 << Ship_info[shipp->ship_info_index].species)) )
	{
		return 0;
	}

	// Goober5000 - extra check to make sure this guy has a rearming dockpoint
	if (model_find_dock_index(Ship_info[shipp->ship_info_index].model_num, DOCK_TYPE_REARM) < 0)
	{
		mprintf(("support not allowed for %s because its model lacks a rearming dockpoint\n", shipp->ship_name));
		return 0;
	}

	// Goober5000 - if we got this far, we can request support
	return 1;
}

// returns random index of a visible ship
// if no visible ships are generated in num_ships iterations, it returns -1
int ship_get_random_targetable_ship()
{
	int rand_ship;
	int idx = 0, target_list[MAX_SHIPS];
	ship_obj *so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// make sure the instance is valid
		if ( (Objects[so->objnum].instance < 0) || (Objects[so->objnum].instance >= MAX_SHIPS) )
			continue;

		// skip if we aren't supposed to target it
		if ( (Ships[Objects[so->objnum].instance].flags & TARGET_SHIP_IGNORE_FLAGS) || (Ships[Objects[so->objnum].instance].flags2 & TARGET_SHIP_IGNORE_FLAGS_2) )
			continue;

		if (idx >= MAX_SHIPS) {
			idx = MAX_SHIPS;
			break;
		}

		target_list[idx] = Objects[so->objnum].instance;
		idx++;
	}

	if (idx == 0)
		return -1;

	rand_ship = (rand() % idx);

	return target_list[rand_ship];
}

// forcible jettison cargo from a ship
void object_jettison_cargo(object *objp, object *cargo_objp)
{
	// make sure we are docked
	Assert((objp != NULL) && (cargo_objp != NULL));
	Assert(dock_check_find_direct_docked_object(objp, cargo_objp));

	vec3d impulse, pos;

	// undock the objects
	ai_do_objects_undocked_stuff(objp, cargo_objp);

	// Goober5000 - add log
	mission_log_add_entry(LOG_SHIP_UNDOCKED, Ships[objp->instance].ship_name, Ships[cargo_objp->instance].ship_name);

	// physics stuff
	vm_vec_sub(&pos, &cargo_objp->pos, &objp->pos);
	impulse = pos;
	vm_vec_scale(&impulse, 100.0f);
	vm_vec_normalize(&pos);

	// whack the ship
	physics_apply_whack(&impulse, &pos, &cargo_objp->phys_info, &cargo_objp->orient, cargo_objp->phys_info.mass);
}

float ship_get_exp_damage(object* objp)
{
	Assert(objp->type == OBJ_SHIP);
	float damage; 

	ship *shipp = &Ships[objp->instance];

	if (shipp->special_exp_index != -1) {
		damage = (float) atoi(Sexp_variables[shipp->special_exp_index+DAMAGE].text);
	} else {
		damage = Ship_info[shipp->ship_info_index].shockwave.damage;
	}

	return damage;
}

int ship_get_exp_propagates(ship *sp)
{
	return Ship_info[sp->ship_info_index].explosion_propagates;
}

float ship_get_exp_outer_rad(object *ship_objp)
{
	float outer_rad;
	Assert(ship_objp->type == OBJ_SHIP);

	if (Ships[ship_objp->instance].special_exp_index == -1) {
		outer_rad = Ship_info[Ships[ship_objp->instance].ship_info_index].shockwave.outer_rad;
	} else {
		outer_rad = (float) atoi(Sexp_variables[Ships[ship_objp->instance].special_exp_index+OUTER_RAD].text);
	}

	return outer_rad;
}

int valid_cap_subsys_cargo_list(char *subsys)
{
	if (strstr(subsys, "nav")
		|| strstr(subsys, "comm")
		|| strstr(subsys, "engine")
		|| strstr(subsys, "fighter")	// fighter bays
		|| strstr(subsys, "sensors")
		|| strstr(subsys, "weapons")) {

		return 1;
	}

	return 0;
}

// determine turret status of a given subsystem, returns 0 for no turret, 1 for "fixed turret", 2 for "rotating" turret
int ship_get_turret_type(ship_subsys *subsys)
{
	// not a turret at all
	if(subsys->system_info->type != SUBSYSTEM_TURRET){
		return 0;
	}

	// if it rotates
	if(subsys->system_info->turret_turning_rate > 0.0f){
		return 2;
	}

	// if its fixed
	return 1;
}

ship_subsys *ship_get_subsys(ship *shipp, char *subsys_name)
{
	ship_subsys *lookup;

	// sanity checks
	if((shipp == NULL) || (subsys_name == NULL)){
		return NULL;
	}

	lookup = GET_FIRST(&shipp->subsys_list);
	while(lookup != END_OF_LIST(&shipp->subsys_list)){
		// turret
		if(!subsystem_stricmp(lookup->system_info->subobj_name, subsys_name)){
			return lookup;
		}

		// next
		lookup = GET_NEXT(lookup);
	}

	// didn't find it
	return NULL;
}

int ship_get_num_subsys(ship *shipp)
{
	Assert(shipp != NULL);

	return Ship_info[shipp->ship_info_index].n_subsystems;
}

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
int wing_has_conflicting_teams(int wing_index)
{
	int first_team, idx;

	// sanity checks
	Assert((wing_index >= 0) && (wing_index < Num_wings) && (Wings[wing_index].current_count > 0));
	if((wing_index < 0) || (wing_index >= Num_wings) || (Wings[wing_index].current_count <= 0)){
		return -1;
	}

	// check teams
	Assert(Wings[wing_index].ship_index[0] >= 0);
	if(Wings[wing_index].ship_index[0] < 0){
		return -1;
	}
	first_team = Ships[Wings[wing_index].ship_index[0]].team;
	for(idx=1; idx<Wings[wing_index].current_count; idx++){
		// more sanity checks
		Assert(Wings[wing_index].ship_index[idx] >= 0);
		if(Wings[wing_index].ship_index[idx] < 0){
			return -1;
		}

		// if we've got a team conflict
		if(first_team != Ships[Wings[wing_index].ship_index[idx]].team){
			return 1;
		}
	}

	// no conflict
	return 0;
}

// get the team of a reinforcement item
int ship_get_reinforcement_team(int r_index)
{
	int wing_index;
	p_object *p_objp;

	// sanity checks
	Assert((r_index >= 0) && (r_index < Num_reinforcements));
	if ((r_index < 0) || (r_index >= Num_reinforcements))
		return -1;

	// if the reinforcement is a ship	
	p_objp = mission_parse_get_arrival_ship(Reinforcements[r_index].name);
	if (p_objp != NULL)
		return p_objp->team;

	// if the reinforcement is a ship
	wing_index = wing_lookup(Reinforcements[r_index].name);
	if (wing_index >= 0)
	{		
		// go through the ship arrival list and find any ship in this wing
		for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
		{
			// check by wingnum			
			if (p_objp->wingnum == wing_index)
				return p_objp->team;
		}
	}

	// no team ?
	return -1;
}

// determine if the given texture is used by a ship type. return ship info index, or -1 if not used by a ship
int ship_get_texture(int bitmap)
{
	int idx;

	// check all ship types
	for(idx=0; idx<Num_ship_classes; idx++){
		if((Ship_info[idx].model_num >= 0) && model_find_texture(Ship_info[idx].model_num, bitmap) == 1){
			return idx;
		}
	}

	// couldn't find the texture
	return -1;
}

// update artillery lock info
#define CLEAR_ARTILLERY_AND_CONTINUE()	{ if(aip != NULL){ aip->artillery_objnum = -1; aip->artillery_sig = -1;	aip->artillery_lock_time = 0.0f;} continue; } 
float artillery_dist = 10.0f;
DCF(art, "")
{
	dc_get_arg(ARG_FLOAT);
	artillery_dist = Dc_arg_float;
}
void ship_update_artillery_lock()
{
#if defined(MULTIPLAYER_BETA_BUILD) || defined(FS2_DEMO)
	return;
#else
	ai_info *aip = NULL;
	weapon_info *tlaser = NULL;
	mc_info *cinfo = NULL;
	int c_objnum;
	vec3d temp, local_hit;
	ship *shipp;
	ship_obj *so;

	// update all ships
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ){
		// get the ship
		if((so->objnum >= 0) && (Objects[so->objnum].type == OBJ_SHIP) && (Objects[so->objnum].instance >= 0)){
			shipp = &Ships[Objects[so->objnum].instance];
		} else {
			continue;
		}		

		// get ai info
		if(shipp->ai_index >= 0){
			aip = &Ai_info[shipp->ai_index];
		}

		// if the ship has no targeting laser firing
		if((shipp->targeting_laser_objnum < 0) || (shipp->targeting_laser_bank < 0)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// if he didn't hit any objects this frame
		if(beam_get_num_collisions(shipp->targeting_laser_objnum) <= 0){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// get weapon info for the targeting laser he's firing
		Assert((shipp->weapons.current_primary_bank >= 0) && (shipp->weapons.current_primary_bank < 2));
		if((shipp->weapons.current_primary_bank < 0) || (shipp->weapons.current_primary_bank >= 2)){
			continue;
		}
		Assert(shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] >= 0);
		if(shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] < 0){
			continue;
		}
		Assert((Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].b_info.beam_type == BEAM_TYPE_C));
		if(!(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags & WIF_BEAM) || (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].b_info.beam_type != BEAM_TYPE_C)){
			continue;
		}
		tlaser = &Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]];	

		// get collision info
		if(!beam_get_collision(shipp->targeting_laser_objnum, 0, &c_objnum, &cinfo)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}
		if((c_objnum < 0) || (cinfo == NULL)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// get the position we hit this guy with in his local coords
		vm_vec_sub(&temp, &cinfo->hit_point_world, &Objects[c_objnum].pos);
		vm_vec_rotate(&local_hit, &temp, &Objects[c_objnum].orient);

		// if we are hitting a different guy now, reset the lock
		if((c_objnum != aip->artillery_objnum) || (Objects[c_objnum].signature != aip->artillery_sig)){
			aip->artillery_objnum = c_objnum;
			aip->artillery_sig = Objects[c_objnum].signature;
			aip->artillery_lock_time = 0.0f;
			aip->artillery_lock_pos = local_hit;

			// done
			continue;
		}	

		// otherwise we're hitting the same guy. check to see if we've strayed too far
		if(vm_vec_dist_quick(&local_hit, &aip->artillery_lock_pos) > artillery_dist){
			// hmmm. reset lock time, but don't reset the lock itself
			aip->artillery_lock_time = 0.0f;
			continue;
		}

		// finally - just increment the lock time
		aip->artillery_lock_time += flFrametime;

		// TEST CODE
		if(aip->artillery_lock_time >= 2.0f){
			HUD_printf("Firing artillery");

			vec3d temp;
			vm_vec_unrotate(&temp, &aip->artillery_lock_pos, &Objects[aip->artillery_objnum].orient);
			vm_vec_add2(&temp, &Objects[aip->artillery_objnum].pos);			
			ssm_create(&temp, &Objects[so->objnum].pos, 0, NULL);				

			// reset the artillery			
			aip->artillery_lock_time = 0.0f;			
		}
	}
#endif
}

// checks if a world point is inside the extended bounding box of a ship
// may not work if delta box is large and negative (ie, adjusted box crosses over on itself - min > max)
int check_world_pt_in_expanded_ship_bbox(vec3d *world_pt, object *objp, float delta_box)
{
	Assert(objp->type == OBJ_SHIP);

	vec3d temp, ship_pt;
	polymodel *pm;
	vm_vec_sub(&temp, world_pt, &objp->pos);
	vm_vec_rotate(&ship_pt, &temp, &objp->orient);

	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	return (
			(ship_pt.xyz.x > pm->mins.xyz.x - delta_box) && (ship_pt.xyz.x < pm->maxs.xyz.x + delta_box)
		&& (ship_pt.xyz.y > pm->mins.xyz.y - delta_box) && (ship_pt.xyz.y < pm->maxs.xyz.y + delta_box)
		&& (ship_pt.xyz.z > pm->mins.xyz.z - delta_box) && (ship_pt.xyz.z < pm->maxs.xyz.z + delta_box)
	);
}


// returns true when objp is ship and is tagged
int ship_is_tagged(object *objp)
{
	ship *shipp;
	if (objp->type == OBJ_SHIP) {
		shipp = &Ships[objp->instance];
		if ( (shipp->tag_left > 0) || (shipp->level2_tag_left > 0) ) {
			return 1;
		}
	}

	return 0;
}

// get maximum ship speed (when not warping in or out)
float ship_get_max_speed(ship *shipp)
{
	float max_speed;
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	// Goober5000 - maybe we're using cap-waypoint-speed
	ai_info *aip = &Ai_info[shipp->ai_index];
	if ((aip->mode == AIM_WAYPOINTS || aip->mode == AIM_FLY_TO_SHIP) && aip->waypoint_speed_cap > 0)
		return i2fl(aip->waypoint_speed_cap);

	// max overclock
	max_speed = sip->max_overclocked_speed;

	// normal max speed
	max_speed = MAX(max_speed, sip->max_vel.xyz.z);

	// afterburn
	max_speed = MAX(max_speed, sip->afterburner_max_vel.xyz.z);

	return max_speed;
}

// determin warpout speed of ship
float ship_get_warpout_speed(object *objp)
{
	Assert(objp->type == OBJ_SHIP);

	return shipfx_calculate_warp_dist(objp) / shipfx_calculate_warp_time(objp, WD_WARP_OUT);
}

// returns true if ship is beginning to speed up in warpout 
int ship_is_beginning_warpout_speedup(object *objp)
{
	Assert(objp->type == OBJ_SHIP);

	ai_info *aip;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if (aip->mode == AIM_WARP_OUT) {
		if ( (aip->submode == AIS_WARP_3) || (aip->submode == AIS_WARP_4) || (aip->submode == AIS_WARP_5) ) {
			return 1;
		}
	}

	return 0;
}

// given a ship info type, return a species
int ship_get_species_by_type(int ship_info_index)
{
	// sanity
	if((ship_info_index < 0) || (ship_info_index >= Num_ship_classes)){
		return -1;
	}

	// return species
	return Ship_info[ship_info_index].species;
}

// return the length of a ship
float ship_class_get_length(ship_info *sip)
{
	Assert(sip->model_num >= 0);
	polymodel *pm = model_get(sip->model_num);
	return (pm->maxs.xyz.z - pm->mins.xyz.z);
}

// Goober5000
void ship_set_new_ai_class(int ship_num, int new_ai_class)
{
	Assert(ship_num >= 0);
	Assert(new_ai_class >= 0);

	ai_info *aip = &Ai_info[Ships[ship_num].ai_index];

	// we hafta change a bunch of stuff here...
	aip->ai_class = new_ai_class;
	aip->behavior = new_ai_class;
	aip->ai_courage = Ai_classes[new_ai_class].ai_courage[Game_skill_level];
	aip->ai_patience = Ai_classes[new_ai_class].ai_patience[Game_skill_level];
	aip->ai_evasion = Ai_classes[new_ai_class].ai_evasion[Game_skill_level];
	aip->ai_accuracy = Ai_classes[new_ai_class].ai_accuracy[Game_skill_level];

	Ship_info[Ships[ship_num].ship_info_index].ai_class = new_ai_class;
	Ships[ship_num].weapons.ai_class = new_ai_class;

	// I think that's everything!
}

// Goober5000
void ship_subsystem_set_new_ai_class(int ship_num, char *subsystem, int new_ai_class)
{
	Assert(ship_num >= 0 && ship_num < MAX_SHIPS);
	Assert(subsystem);
	Assert(new_ai_class >= 0);

	ship_subsys *ss;

	// find the ship subsystem by searching ship's subsys_list
	ss = GET_FIRST( &Ships[ship_num].subsys_list );
	while ( ss != END_OF_LIST( &Ships[ship_num].subsys_list ) )
	{
		// if we found the subsystem
		if ( !subsystem_stricmp(ss->system_info->subobj_name, subsystem))
		{
			// set ai class
			ss->weapons.ai_class = new_ai_class;
			return;
		}

		ss = GET_NEXT( ss );
	}
	Int3();	// subsystem not found
}

// Goober5000 - will attempt to load an insignia bitmap and set it as active for the wing
// copied more or less from managepilot.cpp
void wing_load_squad_bitmap(wing *w)
{
	// sanity check
	if(w == NULL)
	{
		return;
	}

	// make sure one is not already set?!?
	Assert (w->wing_insignia_texture == -1);

	// try and set the new one
	if(strlen(w->wing_squad_filename) > 0)
	{
		// load duplicate because it might be the same as the player's squad,
		// and we don't want to overlap and breed nasty errors when we unload
		w->wing_insignia_texture = bm_load_duplicate(w->wing_squad_filename);
		
		// lock is as a transparent texture
		if(w->wing_insignia_texture != -1)
		{
			bm_lock(w->wing_insignia_texture, 16, BMP_TEX_XPARENT);
			bm_unlock(w->wing_insignia_texture);
		}
	}
}

// Goober5000 - needed by new hangar depart code
// check whether this ship has a docking bay
int ship_has_dock_bay(int shipnum)
{
	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);

	polymodel *pm;
				
	pm = model_get( Ship_info[Ships[shipnum].ship_info_index].model_num );
	Assert( pm );

	return ( pm->ship_bay && (pm->ship_bay->num_paths > 0) );
}

// Goober5000 - needed by new hangar depart code
// get first ship in ship list with docking bay
int ship_get_ship_with_dock_bay(int team)
{
	int ship_with_bay = -1;
	ship_obj *so;

	so = GET_FIRST(&Ship_obj_list);
	while(so != END_OF_LIST(&Ship_obj_list))
	{
		if ( ship_has_dock_bay(Objects[so->objnum].instance) && (Ships[Objects[so->objnum].instance].team == team) )
		{
			ship_with_bay = Objects[so->objnum].instance;

			// make sure not dying or departing
			if (Ships[ship_with_bay].flags & (SF_DYING | SF_DEPARTING))
				ship_with_bay = -1;

			// also make sure that the bays are not all destroyed
			if (ship_fighterbays_all_destroyed(&Ships[ship_with_bay]))
				ship_with_bay = -1;

			if (ship_with_bay >= 0)
				break;
		}
		so = GET_NEXT(so);
	}

	// return whatever we got
	return ship_with_bay;
}

// Goober5000 - check if all fighterbays on a ship have been destroyed
int ship_fighterbays_all_destroyed(ship *shipp)
{
	Assert(shipp);
	ship_subsys *subsys;
	int num_fighterbay_subsystems = 0;

	// check all fighterbay systems
	subsys = GET_FIRST(&shipp->subsys_list);
	while(subsys != END_OF_LIST(&shipp->subsys_list))
	{
		// look for fighterbays
		if (ship_subsys_is_fighterbay(subsys))
		{
			num_fighterbay_subsystems++;

			// if fighterbay doesn't take damage, we're good
			if (!ship_subsys_takes_damage(subsys))
				return 0;

			// if fighterbay isn't destroyed, we're good
			if (subsys->current_hits > 0)
				return 0;
		}

		// next item
		subsys = GET_NEXT(subsys);
	}

	// if the ship has no fighterbay subsystems at all, it must be an unusual case,
	// like the Faustus, so pretend it's okay...
	if (num_fighterbay_subsystems == 0)
		return 0;

	// if we got this far, the ship has at least one fighterbay subsystem,
	// and all the ones it has are destroyed
	return 1;
}

// moved here by Goober5000
int ship_subsys_is_fighterbay(ship_subsys *ss)
{
	Assert(ss);

	if ( !strnicmp(NOX("fighter"), ss->system_info->name, 7) ) {
		return 1;
	}

	return 0;
}

// Goober5000
int ship_subsys_takes_damage(ship_subsys *ss)
{
	Assert(ss);

	return (ss->max_hits > SUBSYS_MAX_HITS_THRESHOLD);
}

// Goober5000
void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss)
{
	Assert(shipp);
	Assert(psub);
	Assert(pss);

	// check if we actually can rotate
	if ( !(psub->flags & MSS_FLAG_ROTATES) ){
		return;
	}

	if (psub->flags & MSS_FLAG_TRIGGERED) {
		pss->trigger.process_queue();
		model_anim_submodel_trigger_rotate(psub, pss );
		return;
	
	}

	// check for rotating artillery
	if ( psub->flags & MSS_FLAG_ARTILLERY )
	{
		ship_weapon *swp = &shipp->weapons;

		// rotate only if trigger is down
		if ( !(shipp->flags & SF_TRIGGER_DOWN) )
			return;

		// check linked
		if ( shipp->flags & SF_PRIMARY_LINKED )
		{
			int i, ammo_tally = 0;

			// calculate ammo
			for (i=0; i<swp->num_primary_banks; i++)
				ammo_tally += swp->primary_bank_ammo[i];

			// do not rotate if out of ammo
			if (ammo_tally <= 0)
				return;
		}
		// check unlinked
		else
		{
			// do not rotate if this is not the firing bank or if we have no ammo in this bank
			if ((psub->weapon_rotation_pbank != swp->current_primary_bank) || (swp->primary_bank_ammo[swp->current_primary_bank] <= 0))
				return;
		}
	}

	// if we got this far, we can rotate - so choose which method to use
	if (psub->flags & MSS_FLAG_STEPPED_ROTATE	) {
		submodel_stepped_rotate(psub, &pss->submodel_info_1);
	} else {
		submodel_rotate(psub, &pss->submodel_info_1 );
	}
}

// Goober5000
int ship_has_energy_weapons(ship *shipp)
{
	// (to avoid round-off errors, weapon reserve is not tested for zero)
	return (Ship_info[shipp->ship_info_index].max_weapon_reserve > WEAPON_RESERVE_THRESHOLD);
}

// Goober5000
int ship_has_engine_power(ship *shipp)
{
	return (Ship_info[shipp->ship_info_index].max_speed > 0 );
}

// Goober5000
int ship_starting_wing_lookup(char *wing_name)
{
	for (int i = 0; i < MAX_STARTING_WINGS; i++)
	{
		if (!stricmp(Starting_wing_names[i], wing_name))
			return i;
	}

	return -1;
}

// Goober5000
int ship_squadron_wing_lookup(char *wing_name)
{
	// TvT uses a different set of wing names from everything else
	if ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
	{
		for (int i = 0; i < MAX_TVT_WINGS; i++)
		{
			if (!stricmp(TVT_wing_names[i], wing_name))
				return i;
		}
	}
	else 
	{
		for (int i = 0; i < MAX_SQUADRON_WINGS; i++)
		{
			if (!stricmp(Squadron_wing_names[i], wing_name))
				return i;
		}
	}

	return -1;
}

// Goober5000
int ship_tvt_wing_lookup(char *wing_name)
{
	for (int i = 0; i < MAX_TVT_WINGS; i++)
	{
		if (!stricmp(TVT_wing_names[i], wing_name))
			return i;
	}

	return -1;
}


// Goober5000
// currently only used in FRED, but probably useful elsewhere too
int ship_class_compare(int ship_class_1, int ship_class_2)
{
	// grab priorities
	//WMC - just use table order
	int priority1 = ship_class_query_general_type(ship_class_1);
	int priority2 = ship_class_query_general_type(ship_class_2);
	/*
	int priority1 = Ship_type_priorities[ship_class_query_general_type(ship_class_1)];
	int priority2 = Ship_type_priorities[ship_class_query_general_type(ship_class_2)];
	*/

	// standard compare
	if (priority1 < priority2)
		return -1;
	else if (priority1 > priority2)
		return 1;
	else
		return 0;
}

//**************************************************************
//WMC - Damage type handling code

typedef struct DamageTypeStruct
{
	char name[NAME_LENGTH];
} DamageTypeStruct;

std::vector<DamageTypeStruct>	Damage_types;

//Gives the index into the Damage_types[] vector of a
//specified damage type name
//returns -1 if not found
int damage_type_get_idx(char *name)
{
	//This should never be bigger than INT_MAX anyway
	for(int i = 0; i < (int)Damage_types.size(); i++)
	{
		if(!stricmp(name, Damage_types[i].name))
			return i;
	}

	return -1;
}

//Either loads a new damage type, or returns the index
//of one with the same name as given
int damage_type_add(char *name)
{
	int i = damage_type_get_idx(name);
	if(i != -1)
		return i;

	DamageTypeStruct dts;

	strncpy(dts.name, name, NAME_LENGTH-1);

	if(strlen(name) > NAME_LENGTH - 1)
	{
		Warning(LOCATION, "Damage type name '%s' is too long and has been truncated to '%s'", name, dts.name);
	}

	Damage_types.push_back(dts);
	return Damage_types.size()-1;
}

//**************************************************************
//WMC - All the extra armor crap

//****************************Calculation type addition

//4 steps to add a new one

//Armor types
//STEP 1: Add a define
#define AT_TYPE_ADDITIVE			0
#define AT_TYPE_MULTIPLICATIVE			1
#define AT_TYPE_EXPONENTIAL			2
#define AT_TYPE_EXPONENTIAL_BASE		3
#define AT_TYPE_CUTOFF				4
#define AT_TYPE_REVERSE_CUTOFF			5
#define AT_TYPE_INSTANT_CUTOFF			6
#define AT_TYPE_INSTANT_REVERSE_CUTOFF		7

//STEP 2: Add the name string to the array
char *TypeNames[] = {
	"additive",
	"multiplicative",
	"exponentional",
	"exponential base",
	"cutoff",
	"reverse cutoff",
	"instant cutoff",
	"instant reverse cutoff",
};

//STEP 3: Add the default value
float TypeDefaultValues[] = {
	0.0f,	//additive
	1.0f,	//multiplicatve
	1.0f,	//exp
	1.0f, 	//exp base - Damage will always be one (No mathematical way to do better)
	0.0f,	//cutoff
	0.0f,	//reverse cutoff
	0.0f,	//instant cutoff
	0.0f,	//rev instant cutoff
};

const int Num_armor_calculation_types = sizeof(TypeNames)/sizeof(char*);

int calculation_type_get(char *str)
{
	for(int i = 0; i < Num_armor_calculation_types; i++)
	{
		if(!stricmp(TypeNames[i], str))
			return i;
	}

	return -1;
}

//STEP 4: Add the calculation to the switch statement.
float ArmorType::GetDamage(float damage_applied, int in_damage_type_idx)
{
	//If the weapon has no damage type, just return damage
	if(in_damage_type_idx < 0)
		return damage_applied;
	
	//Initialize vars
	uint i,num;
	ArmorDamageType *adtp = NULL;

	//Find the entry in the weapon that corresponds to the given weapon damage type
	num = DamageTypes.size();
	for(i = 0; i < num; i++)
	{
		if(DamageTypes[i].DamageTypeIndex == in_damage_type_idx)
		{
			adtp = &DamageTypes[i];
			break;
		}
	}

	//curr_arg is a pointer to the current calculation type value
	float	*curr_arg = NULL;

	//Make sure that we _have_ an armor entry for this damage type
	if(adtp != NULL)
	{
		//How many calculations do we have to do?
		num = adtp->Calculations.size();
		//This would be a problem
		Assert(num == adtp->Arguments.size());

		//Used for instant cutoffs, to instantly end the loop
		bool end_now = false;

		//LOOP!
		for(i = 0; i < num; i++)
		{
			//Set curr_arg
			curr_arg = &adtp->Arguments[i];
			switch(adtp->Calculations[i])
			{
				case AT_TYPE_ADDITIVE:
					damage_applied += *curr_arg;
					break;
				case AT_TYPE_MULTIPLICATIVE:
					damage_applied *= *curr_arg;
					break;
				case AT_TYPE_EXPONENTIAL:
					damage_applied = powf(damage_applied, *curr_arg);
					break;
				case AT_TYPE_EXPONENTIAL_BASE:
					damage_applied = powf(*curr_arg, damage_applied);
					break;
				case AT_TYPE_CUTOFF:
					if(damage_applied < *curr_arg)
						damage_applied = 0;
					break;
				case AT_TYPE_REVERSE_CUTOFF:
					if(damage_applied > *curr_arg)
						damage_applied = 0;
					break;
				case AT_TYPE_INSTANT_CUTOFF:
					if(damage_applied < *curr_arg)
					{
						damage_applied = 0;
						end_now = true;
					}
					break;
				case AT_TYPE_INSTANT_REVERSE_CUTOFF:
					if(damage_applied > *curr_arg)
					{
						damage_applied = 0;
						end_now = true;
					}
					break;
			}
			
			if(end_now)
				break;
		}
	}
	
	return damage_applied;
}

//***********************************Member functions

ArmorType::ArmorType(char* in_name)
{
	uint len = strlen(in_name);
	if(len >= NAME_LENGTH) {
		Warning(LOCATION, "Armor name %s is %d characters too long, and will be truncated", in_name, len - NAME_LENGTH);
	}
	
	strncpy(Name, in_name, NAME_LENGTH-1);
}

void ArmorType::ParseData()
{
	ArmorDamageType adt;
	char buf[NAME_LENGTH];
	float temp_float;
	int calc_type = -1;

	//Get the damage types
	required_string("$Damage Type:");
	do
	{
		//Get damage type name
		stuff_string(buf, F_NAME, NAME_LENGTH);
		
		//Clear the struct and set the index
		adt.clear();
		adt.DamageTypeIndex = damage_type_add(buf);

		//Get calculation and argument
		required_string("+Calculation:");
		do
		{
			//+Calculation
			stuff_string(buf, F_NAME, NAME_LENGTH);

			calc_type = calculation_type_get(buf);

			//Make sure we have a valid calculation type
			if(calc_type == -1)
			{
				Warning(LOCATION, "Armor '%s': Armor calculation type '%s' is invalid, and has been skipped", Name, buf);
				required_string("+Value:");
				stuff_float(&temp_float);
			}
			else
			{
				adt.Calculations.push_back(calc_type);
				//+Value
				required_string("+Value:");
				stuff_float(&temp_float);
				adt.Arguments.push_back(temp_float);
			}
		} while(optional_string("+Calculation:"));

		//If we have calculations in this damage type, add it
		if(adt.Calculations.size() > 0)
		{
			if(adt.Calculations.size() != adt.Arguments.size())
			{
				Warning(LOCATION, "Armor '%s', damage type %d: Armor has a different number of calculation types than arguments (%d, %d)", Name, DamageTypes.size(), adt.Calculations.size(), adt.Arguments.size());
			}
			DamageTypes.push_back(adt);
		}

	} while(optional_string("$Damage Type:"));
}

//********************************Global functions

int armor_type_get_idx(char* name)
{
	int i, num;
	num = Armor_types.size();
	for(i = 0; i < num; i++)
	{
		if(Armor_types[i].IsName(name))
			return i;
	}
	
	//Didn't find anything.
	return -1;
}

void parse_armor_type()
{
	char name_buf[NAME_LENGTH];
	ArmorType tat("");
	
	required_string("$Name:");
	stuff_string(name_buf, F_NAME, NAME_LENGTH);
	
	tat = ArmorType(name_buf);
	
	//now parse the actual table
	tat.ParseData();
	
	//Add it to global armor types
	Armor_types.push_back(tat);
}

void armor_parse_table(char* filename)
{
	lcl_ext_open();

	if ( setjmp(parse_abort) != 0 ) {
		mprintf(("Unable to parse %s!\n", filename));
		lcl_ext_close();
		return;
	}

	read_file_text(filename);
	reset_parse();

	//Enumerate through all the armor types and add them.
	while ( optional_string("#Armor Type") ) {
		while ( required_string_either("#End", "$Name:") ) {
			parse_armor_type();
			continue;
		}

		required_string("#End");
	}

	lcl_ext_close();
}

void armor_init()
{
	if (!armor_inited) {
		armor_parse_table("armor.tbl");

		parse_modular_table(NOX("*-amr.tbm"), armor_parse_table);

		armor_inited = 1;
	}
}
