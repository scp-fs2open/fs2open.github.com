/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/Ship.h $
 * $Revision: 2.135 $
 * $Date: 2006-02-16 05:44:53 $
 * $Author: taylor $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.134  2006/01/20 07:10:34  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 2.133  2006/01/16 11:02:23  wmcoolmon
 * Various warning fixes, scripting globals fix; added "plr" and "slf" global variables for in-game hooks; various lua functions; GCC fixes for scripting.
 *
 * Revision 2.132  2006/01/15 18:55:27  taylor
 * fix compile issues from bad constructor
 * make sure ai_actively_pursues gets filled for modular tables too
 * fix NULL ptr reference when parsing ship type which doesn't exist and is set to not create
 *
 * Revision 2.131  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.130  2006/01/14 09:21:27  wmcoolmon
 * New Lua feature - globals control.
 *
 * Revision 2.129  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.128  2006/01/09 04:54:14  phreak
 * Remove tertiary weapons in their current form, I want something more flexable instead of what I had there.
 *
 * Revision 2.127  2006/01/06 04:18:55  wmcoolmon
 * turret-target-order SEXPs, ship thrusters
 *
 * Revision 2.126  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.125  2005/12/13 22:32:30  wmcoolmon
 * Ability to disable damage particle spew on ships
 *
 * Revision 2.124  2005/12/12 21:32:14  taylor
 * allow use of a specific LOD for ship and weapon rendering in the hud targetbox
 *
 * Revision 2.123  2005/12/04 18:58:07  wmcoolmon
 * subsystem + shockwave armor support; countermeasures as weapons
 *
 * Revision 2.122  2005/11/24 08:46:10  Goober5000
 * * cleaned up mission_do_departure
 *   * fixed a hidden crash (array index being -1; would only
 * be triggered for ships w/o subspace drives under certain conditions)
 *   * removed finding a new fighterbay target because it might screw up missions
 *   * improved clarity, code flow, and readability :)
 * * added custom AI flag for disabling warpouts if navigation subsystem fails
 * --Goober5000
 *
 * Revision 2.121  2005/11/23 01:06:58  phreak
 * Added the function to estimate rearm and repair time.
 *
 * Revision 2.120  2005/11/21 23:57:26  taylor
 * some minor thruster cleanup, if you could actually use the term "clean"
 *
 * Revision 2.119  2005/11/21 00:46:05  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.118  2005/10/30 23:45:45  Goober5000
 * stuff for comparing ship classes
 * --Goober5000
 *
 * Revision 2.117  2005/10/29 22:09:31  Goober5000
 * multiple ship docking implemented for initially docked ships
 * --Goober5000
 *
 * Revision 2.116  2005/10/24 07:13:05  Goober5000
 * merge Bobboau's thruster code back in; hopefully this covers everything
 * --Goober5000
 *
 * Revision 2.115  2005/10/20 17:50:02  taylor
 * fix player warpout
 * basic code cleanup (that previous braces change did nothing for readability)
 * spell "plyr" correctly
 * tweak warp shrink time to better match WMC's other changes and avoid the skipping during shrink
 *
 * Revision 2.114  2005/10/20 06:37:34  wmcoolmon
 * Oops, guess I didn't commit this stuff
 *
 * Revision 2.113  2005/10/11 07:43:10  wmcoolmon
 * Topdown updates
 *
 * Revision 2.112  2005/10/11 05:24:34  wmcoolmon
 * Gliding
 *
 * Revision 2.111  2005/10/10 17:21:10  taylor
 * remove NO_NETWORK
 *
 * Revision 2.110  2005/10/09 09:13:29  wmcoolmon
 * Added warpin/warpout speed override values to ships.tbl
 *
 * Revision 2.109  2005/10/09 00:43:09  wmcoolmon
 * Extendable modular tables (XMTs); added weapon dialogs to the Lab
 *
 * Revision 2.108  2005/10/08 05:41:09  wmcoolmon
 * Fix Int3() from ship-vanish
 *
 * Revision 2.107  2005/09/25 05:13:05  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.106  2005/09/24 07:45:31  Goober5000
 * cleaned up some more thruster stuff; honestly, the thruster code is such a
 * mess that it should probably be reverted to the retail version
 * --Goober5000
 *
 * Revision 2.105  2005/09/24 07:07:17  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 2.104  2005/09/24 02:40:10  Goober5000
 * get rid of a whole bunch of Microsoft warnings
 * --Goober5000
 *
 * Revision 2.103  2005/09/20 02:48:37  taylor
 * fix a couple of things that Valgrind complained about
 *
 * Revision 2.102  2005/08/25 22:40:04  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.101  2005/07/24 06:01:37  wmcoolmon
 * Multiple shockwaves support.
 *
 * Revision 2.100  2005/07/21 07:53:14  wmcoolmon
 * Changed $Hull Repair Rate and $Subsystem Repair Rate to be percentages,
 * as well as making them accept all values between -1 and 1
 *
 * Revision 2.99  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.98  2005/07/13 02:30:54  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 2.97  2005/07/13 00:44:21  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.96  2005/07/12 22:14:40  Goober5000
 * removed DECALS_ENABLED
 * --Goober5000
 *
 * Revision 2.95  2005/06/19 02:43:49  taylor
 * WMC's build fix, part deux
 *
 * Revision 2.94  2005/06/07 06:10:51  wmcoolmon
 * This may stop targeting not-targetable ships in EMP
 *
 * Revision 2.93  2005/05/08 20:21:48  wmcoolmon
 * armor.tbl revamp
 *
 * Revision 2.92  2005/04/25 00:31:14  wmcoolmon
 * Dynamically allocated engine washes; subsystem sounds; armor fixes. Line 4268 of ship.cpp, apparently, works properly; bears further looking into.
 *
 * Revision 2.91  2005/04/18 08:35:27  Goober5000
 * model and class changes should be all set now
 * --Goober5000
 *
 * Revision 2.90  2005/04/18 05:27:26  Goober5000
 * removed ship->alt_modelnum as it was essentially duplicates of ship->modelnum; changed the alt modelnum stuff accordingly
 * fixes for ship_model_change and change_ship_type
 * --Goober5000
 *
 * Revision 2.89  2005/04/15 11:32:26  taylor
 * proposes that WMC be subject to a breathalyzer test before commit ;)
 *
 * Revision 2.88  2005/04/15 06:59:05  wmcoolmon
 * One final oops (hopefully).
 *
 * Revision 2.87  2005/04/15 06:23:17  wmcoolmon
 * Local codebase commit; adds armor system.
 *
 * Revision 2.86  2005/04/05 05:53:24  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.85  2005/03/27 12:28:35  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.84  2005/03/25 06:57:38  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.83  2005/03/19 18:02:34  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.82  2005/03/03 07:13:17  wmcoolmon
 * Made HUD shield icon auto-generation off unless "generate icon" ship flag is specified for the ship.
 *
 * Revision 2.81  2005/03/01 06:55:45  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.80  2005/02/19 07:57:03  wmcoolmon
 * Removed trails limit
 *
 * Revision 2.79  2005/02/04 20:06:08  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.78  2005/01/27 04:23:18  wmcoolmon
 * Ship autorepair, requested by TBP
 *
 * Revision 2.77  2005/01/17 23:35:45  argv
 * Surface shields.
 *
 * See forum thread:
 * http://dynamic4.gamespy.com/~freespace/forums/showthread.php?s=&threadid=29643
 *
 * -- _argv[-1]
 *
 * Revision 2.76  2005/01/16 23:18:04  wmcoolmon
 * Added "show ship" ship flag
 *
 * Revision 2.75  2005/01/16 22:39:10  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.74  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.73  2005/01/01 07:18:48  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.72  2004/12/25 09:28:09  wmcoolmon
 * Sync to current NEW_HUD code
 *
 * Revision 2.71  2004/12/14 14:46:12  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.70  2004/12/05 22:01:12  bobboau
 * sevral feature additions that WCS wanted,
 * and the foundations of a submodel animation system,
 * the calls to the animation triggering code (exept the procesing code,
 * wich shouldn't do anything without the triggering code)
 * have been commented out.
 *
 * Revision 2.69  2004/11/21 11:35:17  taylor
 * some weapon-only-used loading fixes and general page-in cleanup
 * page in all ship textures from one function rather than two
 *
 * Revision 2.68  2004/10/15 09:21:55  Goober5000
 * cleaned up some sexp stuff and added wing capability to kamikaze sexp
 * --Goober5000
 *
 * Revision 2.67  2004/10/12 22:47:15  Goober5000
 * added toggle-subsystem-scanning ship flag
 * --Goober5000
 *
 * Revision 2.66  2004/10/11 22:29:24  Goober5000
 * added the no-bank ship flag (which works) and the affected-by-gravity flag
 * (which won't work until I implement gravity points)
 * --Goober5000
 *
 * Revision 2.65  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.64  2004/07/11 03:22:53  bobboau
 * added the working decal code
 *
 * Revision 2.63  2004/05/10 10:51:51  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
 * Revision 2.62  2004/05/10 08:03:31  Goober5000
 * fixored the handling of no lasers and no engines... the tests should check the ship,
 * not the object
 * --Goober5000
 *
 * Revision 2.61  2004/05/03 21:22:23  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.60  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.59  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.58  2004/02/20 04:29:56  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.57  2004/01/31 04:06:29  phreak
 * commented out decal references
 *
 * Revision 2.56  2004/01/29 01:34:02  randomtiger
 * Added malloc montoring system, use -show_mem_usage, debug exes only to get an ingame list of heap usage.
 * Also added -d3d_notmanaged flag to activate non managed D3D path, in experimental stage.
 *
 * Revision 2.55  2004/01/14 07:07:14  Goober5000
 * added error checking for an annoying crash when running an out-of-range
 * sound; also, Phreak misspelled "tertiary"
 * --Goober5000
 *
 * Revision 2.54  2003/12/16 20:55:13  phreak
 * disabled tertiary weapons support pending a rewrite of critical code
 *
 * Revision 2.53  2003/11/11 02:15:41  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.52  2003/10/25 06:56:06  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.51  2003/10/15 22:03:26  Kazan
 * Da Species Update :D
 *
 * Revision 2.50  2003/09/26 14:37:16  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.49  2003/09/13 06:02:03  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.45  2003/08/28 20:42:18  Goober5000
 * implemented rotating barrels for firing primary weapons
 * --Goober5000
 *
 * Revision 2.44  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.43  2003/08/06 17:37:08  phreak
 * preliminary work on tertiary weapons. it doesn't really function yet, but i want to get something committed
 *
 * Revision 2.42  2003/07/15 02:52:40  phreak
 * ships now decloak when firing
 *
 * Revision 2.41  2003/07/04 02:30:54  phreak
 * support for cloaking added.  needs a cloakmap.pcx
 * to cloak the players ship, activate cheats and press tilde + x
 * some more work can be done to smooth out the animation.
 *
 * Revision 2.40  2003/04/29 01:03:21  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.39  2003/03/30 07:27:34  Goober5000
 * resolved a nasty bug that caused some missions to crash
 * --Goober5000
 *
 * Revision 2.38  2003/03/18 08:44:05  Goober5000
 * added explosion-effect sexp and did some other minor housekeeping
 * --Goober5000
 *
 * Revision 2.37  2003/03/06 09:13:43  Goober5000
 * fixed what should be the last bug with bank-specific loadouts
 * --Goober5000
 *
 * Revision 2.36  2003/03/05 12:38:01  Goober5000
 * rewrote the restricted bank loadout code; it should work now
 * --Goober5000
 *
 * Revision 2.35  2003/03/05 09:17:15  Goober5000
 * cleaned out Bobboau's buggy code - about to rewrite with new, bug-free code :)
 * --Goober5000
 *
 * Revision 2.34  2003/03/03 04:28:37  Goober5000
 * fixed the tech room bug!  yay!
 * --Goober5000
 *
 * Revision 2.33  2003/03/01 01:15:38  Goober5000
 * fixed the initial status bug
 *
 * Revision 2.32  2003/02/25 06:22:49  bobboau
 * fixed a bunch of fighter beam bugs,
 * most notabley the sound now works corectly,
 * and they have limeted range with atenuated damage (table option)
 * added bank specific compatabilities
 *
 * Revision 2.31  2003/01/27 07:46:32  Goober5000
 * finished up my fighterbay code - ships will not be able to arrive from or depart into
 * fighterbays that have been destroyed (but you have to explicitly say in the tables
 * that the fighterbay takes damage in order to enable this)
 * --Goober5000
 *
 * Revision 2.30  2003/01/21 17:24:16  Goober5000
 * fixed a few bugs in Bobboau's implementation of the glow sexps; also added
 * help for the sexps in sexp_tree
 * --Goober5000
 *
 * Revision 2.29  2003/01/20 05:40:50  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.28  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.27  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.26  2003/01/18 09:25:40  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.25  2003/01/17 01:48:49  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.24  2003/01/16 06:49:11  Goober5000
 * yay! got texture replacement to work!!!
 * --Goober5000
 *
 * Revision 2.23  2003/01/15 23:23:30  Goober5000
 * NOW the model duplicates work! :p
 * still gotta do the textures, but it shouldn't be hard now
 * --Goober5000
 *
 * Revision 2.22  2003/01/15 08:57:23  Goober5000
 * assigning duplicate models to ships now works; committing so I have a base
 * to fall back to as I work on texture replacement
 * --Goober5000
 *
 * Revision 2.21  2003/01/15 07:09:09  Goober5000
 * changed most references to modelnum to use ship instead of ship_info --
 * this will help with the change-model sexp and any other instances of model
 * changing
 * --Goober5000
 *
 * Revision 2.20  2003/01/06 17:14:52  Goober5000
 * added wing configurable squad logos - put +Squad Logo: filename.pcx as
 * the last entry in each wing that you want (but the player's squad logo will
 * still be the squad logo for the player's wing)
 * --Goober5000
 *
 * Revision 2.19  2003/01/05 01:26:35  Goober5000
 * added capability of is-iff and change-iff to have wings as well as ships
 * as their arguments; also allowed a bunch of sexps to accept the player
 * as an argument where they would previously display a parse error
 * --Goober5000
 *
 * Revision 2.18  2003/01/03 21:58:06  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.17  2002/12/31 18:59:42  Goober5000
 * if it ain't broke, don't fix it
 * --Goober5000
 *
 * Revision 2.15  2002/12/27 02:57:50  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.14  2002/12/24 07:42:28  Goober5000
 * added change-ai-class and is-ai-class, and I think I may also have nailed the
 * is-iff bug; did some other bug hunting as well
 * --Goober5000
 *
 * Revision 2.13  2002/12/23 05:18:52  Goober5000
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
 * Revision 2.12  2002/12/20 07:09:03  Goober5000
 * added capability of storing time_subsys_cargo_revealed
 * --Goober5000
 *
 * Revision 2.11  2002/12/17 02:18:39  Goober5000
 * added functionality and fixed a few things with cargo being revealed and hidden in preparation for the set-scanned and set-unscanned sexp commit
 * --Goober5000
 *
 * Revision 2.10  2002/12/14 17:09:27  Goober5000
 * removed mission flag for fighterbay damage; instead made damage display contingent on whether the fighterbay subsystem is assigned a damage percentage in ships.tbl
 * --Goober5000
 *
 * Revision 2.9  2002/12/13 08:13:28  Goober5000
 * small tweaks and bug fixes for the ballistic primary conversion
 * ~Goober5000~
 *
 * Revision 2.8  2002/12/10 05:43:33  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.7  2002/12/07 01:37:42  bobboau
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.6  2002/11/10 19:57:36  DTP
 * DTP bumped back MAX_SHIP_CLASSES to 130
 *
 * Revision 2.5  2002/10/31 21:56:44  DTP
 * DTP Bumped Max_exited_ships from 200 to double that of MAX_SHIPS = 800 . make sense ehh. for logging effect.
 *
 * Revision 2.4  2002/10/19 19:29:29  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.3  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 08:24:42  DTP
 * Bumped all MAX_SHIPS, and SHIP_LIMIT to 400.(let the mission designers decide what is good, and what is bad
 *
 * Revision 2.1  2002/07/12 16:59:04  penguin
 * Added flags2 (ran out of bits in flags!) to ship struct; bit 0 will be used
 * to toggle Bobboau's lights.
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/10 20:42:45  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 73    9/01/99 10:15a Dave
 * 
 * 72    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 71    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 70    8/26/99 8:52p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 69    8/18/99 12:09p Andsager
 * Add debug if message has no anim for message.  Make messages come from
 * wing leader.
 * 
 * 68    8/16/99 10:04p Andsager
 * Add special-warp-dist and special-warpout-name sexp for Knossos device
 * warpout.
 * 
 * 67    8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 66    8/13/99 10:49a Andsager
 * Knossos and HUGE ship warp out.  HUGE ship warp in.  Stealth search
 * modes dont collide big ships.
 * 
 * 65    8/02/99 10:39p Dave
 * Added colored shields. OoOoOoooOoo
 * 
 * 64    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 63    7/26/99 8:06a Andsager
 * Consistent personas
 * 
 * 62    7/19/99 8:56p Andsager
 * Added Ship_exited red_alert_carry flag and hull strength for RED ALERT
 * carry over of Exited_ships
 * 
 * 61    7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 60    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 59    7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 58    7/08/99 12:06p Andsager
 * Add turret-tagged-only and turret-tagged-clear sexp.
 * 
 * 57    7/08/99 11:45a Dave
 * Bumped up max ship types.
 * 
 * 56    7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 55    7/06/99 10:45a Andsager
 * Modify engine wash to work on any ship that is not small.  Add AWACS
 * ask for help.
 * 
 * 54    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 53    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 52    6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 51    6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 50    6/07/99 4:21p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 49    6/03/99 11:43a Dave
 * Added the ability to use a different model when rendering to the HUD
 * target box.
 * 
 * 48    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 47    5/28/99 9:26a Andsager
 * Added check_world_pt_in_expanded_ship_bbox() function
 * 
 * 46    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 45    5/21/99 5:03p Andsager
 * Add code to display engine wash death.  Modify ship_kill_packet
 * 
 * 44    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 43    5/14/99 11:50a Andsager
 * Added vaporize for SMALL ships hit by HUGE beams.  Modified dying
 * frame.  Enlarged debris shards and range at which visible.
 * 
 * 42    5/12/99 2:55p Andsager
 * Implemented level 2 tag as priority in turret object selection
 * 
 * 41    5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 40    4/28/99 3:11p Andsager
 * Stagger turret weapon fire times.  Make turrets smarter when target is
 * protected or beam protected.  Add weaopn range to weapon info struct.
 * 
 * 39    4/28/99 9:39a Andsager
 * flag for ship_weapon turret lock
 * 
 * 38    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 37    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 36    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 35    4/19/99 12:21p Johnson
 * Allow ships with invisible polygons which do not collide
 * 
 * 34    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 33    4/12/99 10:07p Dave
 * Made network startup more forgiving. Added checkmarks to dogfight
 * screen for players who hit commit.
 * 
 * 32    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 31    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 30    3/30/99 5:40p Dave
 * Fixed reinforcements for TvT in multiplayer.
 * 
 * 29    3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 28    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 27    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 26    3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 25    3/02/99 9:25p Dave
 * Added a bunch of model rendering debug code. Started work on fixing
 * beam weapon wacky firing.
 * 
 * 24    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 23    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 22    2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 21    2/11/99 5:22p Andsager
 * Fixed bugs, generalized block Sexp_variables
 * 
 * 20    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 19    2/03/99 6:06p Dave
 * Groundwork for FS2 PXO usertracker support.  Gametracker support next.
 * 
 * $NoKeywords: $
 */

#ifndef _SHIP_H
#define _SHIP_H



#include "globalincs/globals.h"		// for defintions of token lengths -- maybe move this elsewhere later (Goober5000 - moved to globals.h)
#include "graphics/2d.h"			// for color def
#include "model/model.h"
#include "palman/palman.h"
#include "weapon/trails.h"
#include "ai/ai.h"
#include "network/multi_oo.h"
#include "hud/hudparse.h"
#include "render/3d.h"
#include "weapon/shockwave.h"
#include "species_defs/species_defs.h"
#include "globalincs/pstypes.h"

#include <vector>
#include <string>

struct object;

//	Part of the player died system.
extern vec3d	Dead_camera_pos, Original_vec_to_deader;

//	States for player death sequence, stuffed in Player_died_state.
#define	PDS_NONE		1
#define	PDS_DIED		2
#define	PDS_EJECTED	3

#define SHIP_GUARDIAN_THRESHOLD_DEFAULT	1	// Goober5000

#define	HULL_DAMAGE_THRESHOLD_PERCENT	0.25f	//	Apply damage to hull, not shield if shield < this

// the #defines below are to avoid round-off errors
#define WEAPON_RESERVE_THRESHOLD		0.01f	// energy threshold where ship is considered to have no weapon energy system
#define SUBSYS_MAX_HITS_THRESHOLD		0.01f	// max_hits threshold where subsys is considered to take damage

#define	HP_SCALE						1.2			//	1.2 means die when 20% of hits remaining
#define	MAX_SHIP_HITS				8				// hits to kill a ship
#define	MAX_SHIP_DETAIL_LEVELS	5				// maximum detail levels that a ship can render at
#define	MAX_REINFORCEMENTS		10


// defines for 'direction' parameter of ship_select_next_primary()
#define CYCLE_PRIMARY_NEXT		0
#define CYCLE_PRIMARY_PREV		1

#define BANK_1		0
#define BANK_2		1
#define BANK_3		2
#define BANK_4		3
#define BANK_5		4
#define BANK_6		5
#define BANK_7		6
#define BANK_8		7
#define BANK_9		8

#define TYPE_ATTACK_PROTECT	0
#define TYPE_REPAIR_REARM		1

#define MAX_REINFORCEMENT_MESSAGES	5

#define RF_IS_AVAILABLE			(1<<0)			// reinforcement is now available

typedef struct {
	char	name[NAME_LENGTH];	// ship or wing name (ship and wing names don't collide)
	int	type;						// what operations this reinforcement unit can perform
	int	uses;						// number of times reinforcemnt unit can be used
	int	num_uses;				// number of times this reinforcement was actually used
	int	arrival_delay;			// how long after called does this reinforcement appear
	int	flags;
	char	no_messages[MAX_REINFORCEMENT_MESSAGES][NAME_LENGTH];		// list of messages to possibly send when calling for reinforcement not available
	char	yes_messages[MAX_REINFORCEMENT_MESSAGES][NAME_LENGTH];	// list of messages to acknowledge reinforcement on the way
} reinforcements;

// ship weapon flags
#define SW_FLAG_BEAM_FREE					(1<<0)							// if this is a beam weapon, its free to fire
#define SW_FLAG_TURRET_LOCK				(1<<1)							//	is this turret is free to fire or locked
#define SW_FLAG_TAGGED_ONLY				(1<<2)							// only fire if target is tagged

typedef struct ship_weapon {
	int num_primary_banks;					// Number of primary banks (same as model)
	int num_secondary_banks;				// Number of secondary banks (same as model)
	int num_tertiary_banks;

	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];			// Weapon_info[] index for the weapon in the bank
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];	// Weapon_info[] index for the weapon in the bank

	int current_primary_bank;			// currently selected primary bank
	int current_secondary_bank;		// currently selected secondary bank
	int current_tertiary_bank;

	int next_primary_fire_stamp[MAX_SHIP_PRIMARY_BANKS];			// next time this primary bank can fire
	int next_secondary_fire_stamp[MAX_SHIP_SECONDARY_BANKS];	// next time this secondary bank can fire
	int next_tertiary_fire_stamp;

	// ballistic primary support - by Goober5000
	int primary_bank_ammo[MAX_SHIP_PRIMARY_BANKS];			// Number of missiles left in primary bank
	int primary_bank_start_ammo[MAX_SHIP_PRIMARY_BANKS];	// Number of missiles starting in primary bank
	int primary_bank_capacity[MAX_SHIP_PRIMARY_BANKS];		// Max number of projectiles in bank
	int primary_next_slot[MAX_SHIP_PRIMARY_BANKS];			// Next slot to fire in the bank
	int primary_bank_rearm_time[MAX_SHIP_PRIMARY_BANKS];	// timestamp which indicates when bank can get new projectile

	int secondary_bank_ammo[MAX_SHIP_SECONDARY_BANKS];			// Number of missiles left in secondary bank
	int secondary_bank_start_ammo[MAX_SHIP_SECONDARY_BANKS];	// Number of missiles starting in secondary bank
	int secondary_bank_capacity[MAX_SHIP_SECONDARY_BANKS];		// Max number of missiles in bank
	int secondary_next_slot[MAX_SHIP_SECONDARY_BANKS];			// Next slot to fire in the bank
	int secondary_bank_rearm_time[MAX_SHIP_SECONDARY_BANKS];	// timestamp which indicates when bank can get new missile

	int tertiary_bank_ammo;			// Number of shots left tertiary bank
	int tertiary_bank_start_ammo;	// Number of shots starting in tertiary bank
	int tertiary_bank_capacity;		// Max number of shots in bank
	int tertiary_bank_rearm_time;	// timestamp which indicates when bank can get new something (used for ammopod or boostpod)

	int last_fired_weapon_index;		//	Index of last fired secondary weapon.  Used for remote detonates.
	int last_fired_weapon_signature;	//	Signature of last fired weapon.
	int detonate_weapon_time;			//	time at which last fired weapon can be detonated
	int ai_class;

	int flags;								// see SW_FLAG_* defines above
	bool primary_animation_position[MAX_SHIP_PRIMARY_BANKS];
	bool secondary_animation_position[MAX_SHIP_SECONDARY_BANKS];
	int primary_animation_done_time[MAX_SHIP_PRIMARY_BANKS];
	int  secondary_animation_done_time[MAX_SHIP_SECONDARY_BANKS];
} ship_weapon;

//**************************************************************
//WMC - Damage type handling code

int damage_type_add(char *name);

//**************************************************************
//WMC - Armor stuff

struct ArmorDamageType
{
	friend class ArmorType;
private:
	//Rather than make an extra struct,
	//I just made two arrays
	int					DamageTypeIndex;
	std::vector<int>	Calculations;
	std::vector<float>	Arguments;

public:
	void clear(){DamageTypeIndex=-1;Calculations.clear();Arguments.clear();}
};

class ArmorType
{
private:
	char Name[NAME_LENGTH];

	std::vector<ArmorDamageType> DamageTypes;
public:
	ArmorType(char* in_name);

	//Get
	char *GetNamePtr(){return Name;}
	bool IsName(char *in_name){return (strnicmp(in_name,Name,strlen(Name)) == 0);}
	float GetDamage(float damage_applied, int in_damage_type_idx);
	
	//Set
	void ParseData();
};

extern std::vector<ArmorType> Armor_types;

#define NUM_TURRET_ORDER_TYPES		3
extern char *Turret_target_order_names[NUM_TURRET_ORDER_TYPES];	//aiturret.cpp

// structure definition for a linked list of subsystems for a ship.  Each subsystem has a pointer
// to the static data for the subsystem.  The obj_subsystem data is defined and read in the model
// code.  Other dynamic data (such as current_hits) should remain in this structure.
typedef	struct ship_subsys {
	struct ship_subsys *next, *prev;				//	Index of next and previous objects in list.
	model_subsystem *system_info;					// pointer to static data for this subsystem -- see model.h for definition
	float		current_hits;							// current number of hits this subsystem has left.
	float		max_hits;

	int subsys_guardian_threshold;	// Goober5000

	// turret info
	//Important -WMC
	//With the new turret code, indexes run from 0 to MAX_SHIP_WEAPONS; a value of MAX_SHIP_PRIMARY_WEAPONS
	//or higher, an index into the turret weapons is considered to be an index into the secondary weapons
	//for much of the code. See turret_next_weap_fire_stamp.

	//Note that turret_next_weap_fire_stamp is officially a hack, because turrets use all this crap
	//ideally, they should make use of the ship_weapon structure below
	//int		turret_next_weap_fire_stamp[MAX_SHIP_WEAPONS];	//Fire stamps for all weapons on this turret
	int		turret_best_weapon;				// best weapon for current target; index into prim/secondary banks
	vec3d	turret_last_fire_direction;		//	direction pointing last time this turret fired
	int		turret_next_enemy_check_stamp;	//	time at which to next look for a new enemy.
	int		turret_next_fire_stamp;				// next time this turret can fire
	int		turret_enemy_objnum;					//	object index of ship this turret is firing upon
	int		turret_enemy_sig;						//	signature of object ship this turret is firing upon
	int		turret_next_fire_pos;				// counter which tells us which gun position to fire from next
	float	turret_time_enemy_in_range;		//	Number of seconds enemy in view cone, accuracy improves over time.
	int		turret_targeting_order[NUM_TURRET_ORDER_TYPES];	//Order that turrets target different types of things.
	ship_subsys	*targeted_subsys;					//	subsystem this turret is attacking

	int		turret_pick_big_attack_point_timestamp;	//	Next time to pick an attack point for this turret
	vec3d	turret_big_attack_point;			//	local coordinate of point for this turret to attack on enemy

	// swarm (rapid fire) info
	int		turret_swarm_info_index;	

	// awacs info
	float		awacs_intensity;
	float		awacs_radius;

	ship_weapon	weapons;

	// Data the renderer needs for ship instance specific data, like
	// angles and if it is blown off or not.
	// There are 2 of these because turrets need one for the turret and one for the barrel.
	// Things like radar dishes would only use one.
	submodel_instance_info	submodel_info_1;		// Instance data for main turret or main object
	submodel_instance_info	submodel_info_2;		// Instance data for turret guns, if there is one

	int disruption_timestamp;							// time at which subsystem isn't disrupted

	int subsys_cargo_name;			// cap ship cargo on subsys
	int subsys_cargo_revealed;
	fix time_subsys_cargo_revealed;	// added by Goober5000
} ship_subsys;

// structure for subsystems which tells us the total count of a particular type of subsystem (i.e.
// we might have 3 engines), and the relative strength of the subsystem.  The #defines in model.h
// for SUBSYSTEM_xxx will be used as indices into this array.
typedef struct ship_subsys_info {
	int	num;				// number of subsystems of type on this ship;
	float total_hits;		// total number of hits between all subsystems of this type.
	float current_hits;		// current count of hits for all subsystems of this type.	
} ship_subsys_info;

	
//#define	MAX_SHIP_SUBOBJECTS		50

//extern ship_subobj	Ship_subsystems[MAX_SHIP_SUBOBJECTS];

// states for the flags variable within the ship structure
// low bits are for mission file savable flags..
// FRED needs these to be the low-order bits with no holes,
// because it indexes into an array, Hoffoss says.
#define	SF_IGNORE_COUNT			(1 << 0)		// ignore this ship when counting ship types for goals
#define	SF_REINFORCEMENT			(1 << 1)		// this ship is a reinforcement ship
#define	SF_ESCORT					(1 << 2)		// this ship is an escort ship
#define	SF_NO_ARRIVAL_MUSIC		(1 << 3)		// don't play arrival music when ship arrives
#define	SF_NO_ARRIVAL_WARP		(1 << 4)		// no arrival warp in effect
#define	SF_NO_DEPARTURE_WARP		(1 << 5)		// no departure warp in effect
#define	SF_LOCKED					(1 << 6)		// can't manipulate ship in loadout screens
#define	SF_INVULNERABLE			(1 << 7)

// high bits are for internal flags not saved to mission files
// Go from bit 31 down to bit 3
#define	SF_KILL_BEFORE_MISSION	(1 << 31)
#define	SF_DYING						(1 << 30)
#define	SF_DISABLED					(1 << 29)
#define	SF_DEPART_WARP				(1 << 28)	// ship is departing via warp-out
#define	SF_DEPART_DOCKBAY			(1 << 27)	// ship is departing via docking bay
#define	SF_ARRIVING_STAGE_1		(1 << 26)	// ship is arriving. In other words, doing warp in effect, stage 1
#define	SF_ARRIVING_STAGE_2		(1 << 25)	// ship is arriving. In other words, doing warp in effect, stage 2
#define  SF_ARRIVING             (SF_ARRIVING_STAGE_1|SF_ARRIVING_STAGE_2)
#define	SF_ENGINES_ON				(1 << 24)	// engines sound should play if set
#define	SF_DOCK_LEADER			(1 << 23)	// Goober5000 - this guy is in charge of everybody he's docked to
#define	SF_CARGO_REVEALED			(1 << 22)	// ship's cargo is revealed to all friendly ships
#define	SF_FROM_PLAYER_WING		(1	<< 21)	// set for ships that are members of any player starting wing
#define	SF_PRIMARY_LINKED			(1 << 20)	// ships primary weapons are linked together
#define	SF_SECONDARY_DUAL_FIRE	(1 << 19)	// ship is firing two missiles from the current secondary bank
#define	SF_WARP_BROKEN				(1	<< 18)	// set when warp drive is not working, but is repairable
#define	SF_WARP_NEVER				(1	<< 17)	// set when ship can never warp
#define	SF_TRIGGER_DOWN			(1 << 16)	// ship has its "trigger" held down
#define	SF_AMMO_COUNT_RECORDED	(1	<<	15)	// we've recorded the inital secondary weapon count (which is used to limit support ship rearming)
#define	SF_HIDDEN_FROM_SENSORS	(1	<< 14)	// ship doesn't show up on sensors, blinks in/out on radar
#define	SF_SCANNABLE				(1	<< 13)	// ship is "scannable".  Play scan effect and report as "Scanned" or "not scanned".
#define	SF_WARPED_SUPPORT			(1 << 12)	// set when this is a support ship which was warped in automatically
#define	SF_EXPLODED					(1 << 11)	// ship has exploded (needed for kill messages)
#define	SF_SHIP_HAS_SCREAMED		(1 << 10)	// ship has let out a death scream
#define	SF_RED_ALERT_STORE_STATUS (1 << 9)	// ship status should be stored/restored if red alert mission
#define	SF_VAPORIZE					(1<<8)		// ship is vaporized by beam - alternative death sequence

// MWA -- don't go below whatever bitfield is used for Fred above (currently 7)!!!!

#define	SF_DEPARTING				(SF_DEPART_WARP | SF_DEPART_DOCKBAY)				// ship is departing
#define	SF_CANNOT_WARP				(SF_WARP_BROKEN | SF_WARP_NEVER | SF_DISABLED)	// ship cannot warp out


#define DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE		10000	// Goober5000


// Bits for ship.flags2
#define	SF2_LIGHTS_ON						(1<<0)		// ship has 'GLOW' lights turned on (Bobboau's lights)
#define SF2_PRIMITIVE_SENSORS				(1<<1)		// Goober5000 - primitive sensor display
#define SF2_FRIENDLY_STEALTH_INVIS			(1<<2)		// Goober5000 - when stealth, don't appear on radar even if friendly
#define SF2_STEALTH							(1<<3)		// Goober5000 - is this particular ship stealth
#define SF2_DONT_COLLIDE_INVIS				(1<<4)		// Goober5000 - is this particular ship don't-collide-invisible
#define SF2_NO_SUBSPACE_DRIVE				(1<<5)		// Goober5000 - this ship has no subspace drive
#define SF2_NAVPOINT_CARRY					(1<<6)		// Kazan      - This ship autopilots with the player
#define SF2_NO_BANK							(1<<7)		// Goober5000 - ship doesn't bank when turning
#define SF2_AFFECTED_BY_GRAVITY				(1<<8)		// Goober5000 - ship affected by gravity points
#define SF2_TOGGLE_SUBSYSTEM_SCANNING		(1<<9)		// Goober5000 - switch whether subsystems are scanned
#define SF2_VANISHED				(1<<10)		//WMC - ship has vanished, used mostly for ship_wing_cleanup

// If any of these bits in the ship->flags are set, ignore this ship when targetting
extern int TARGET_SHIP_IGNORE_FLAGS;

#define MAX_DAMAGE_SLOTS	32
#define MAX_SHIP_ARCS		2		// How many "arcs" can be active at once... Must be less than MAX_ARC_EFFECTS in model.h. 
#define NUM_SUB_EXPL_HANDLES	2	// How many different big ship sub explosion sounds can be played.

#define MAX_SHIP_CONTRAILS		12

typedef struct ship_spark {
	vec3d pos;			// position of spark in the submodel's RF
	int submodel_num;	// which submodel is making the spark
	int end_time;
} ship_spark;

#define AWACS_WARN_NONE		(1 << 0)
#define AWACS_WARN_25		(1 << 1)
#define AWACS_WARN_75		(1 << 2)

#define MAX_GLOW_POINTS 32
#define GLOW_POINTS_ALL_ON	0xFFFFFFFF		// (2 raised to MAX_GLOW_POINTS) - 1
#define GLOW_POINTS_ALL_OFF	0x00000000

typedef struct ship {
	int	objnum;
	int	ai_index;			// Index in Ai_info of ai_info associated with this ship.
	int	ship_info_index;	// Index in ship_info for this ship
	int	modelnum;
	int	hotkey;
	int	escort_priority;
	int	score;
	int	respawn_priority;
	
	// BEGIN PACK ubytes and chars
	ubyte	pre_death_explosion_happened;		// If set, it means the 4 or 5 smaller explosions 
	ubyte wash_killed;
	char	cargo1;

	// ship wing status info
	char	wing_status_wing_index;			// wing index (0-4) in wingman status gauge
	char	wing_status_wing_pos;			// wing position (0-5) in wingman status gauge

	// alternate type name index
	char alt_type_index;								// only used for display purposes (read : safe)

	// targeting laser info
	char targeting_laser_bank;						// -1 if not firing, index into polymodel gun points if it _is_ firing
	// corkscrew missile stuff
	ubyte num_corkscrew_to_fire;						// # of corkscrew missiles lef to fire
	// END PACK
	// targeting laser info
	int targeting_laser_objnum;					// -1 if invalid, beam object # otherwise
	// corkscrew missile stuff
	int next_corkscrew_fire;						// next time to fire a corkscrew missile

	int	final_death_time;				// Time until big fireball starts
	int	death_time;				// Time until big fireball starts
	int	end_death_time;				// Time until big fireball starts
	int	really_final_death_time;	// Time until ship breaks up and disappears
	vec3d	deathroll_rotvel;			// Desired death rotational velocity

	int	final_warp_time;	// pops when ship is completely warped out or warped in.  Used for both warp in and out.
	vec3d	warp_effect_pos;		// where the warp in effect comes in at
	vec3d	warp_effect_fvec;		// The warp in effect's forward vector
	int	next_fireball;

	int	next_hit_spark;
	int	num_hits;			//	Note, this is the number of spark emitter positions!
	ship_spark	sparks[MAX_SHIP_HITS];

	int	special_exp_index;
	int special_hitpoint_index;

	float ship_max_shield_strength;
	float ship_max_hull_strength;

	int ship_guardian_threshold;	// Goober5000 - now also determines whether ship is guardian'd


	char	ship_name[NAME_LENGTH];

	int	team;				//	Which team it's on, HOSTILE, FRIENDLY, UNKNOWN, NEUTRAL
	
	fix	time_cargo_revealed;					// time at which the cargo was revealed

	int	arrival_location;
	int	arrival_distance;						// how far away this ship should arrive
	int	arrival_anchor;		// name of object this ship arrives near (or in front of)
	int	arrival_cue;
	int	arrival_delay;
	int	departure_location;	// depart to hyperspace or someplace else (like docking bay)
	int	departure_anchor;		// when docking day -- index of ship to use
	int	departure_cue;			// sexpression to eval when departing
	int	departure_delay;		// time in seconds after sexp is true that we delay.
	int	determination;
	int	wingnum;								// wing number this ship is in.  -1 if in no wing, Wing array index otherwise
	int	orders_accepted;					// set of orders this ship will accept from the player.

	// Subsystem fields.  The subsys_list is a list of all subsystems (which might include multiple types
	// of a particular subsystem, like engines).  The subsys_info struct is information for particular
	// types of subsystems.  (i.e. the list might contain 3 engines.  There will be one subsys_info entry
	// describing the state of all engines combined) -- MWA 4/1/97
	ship_subsys	subsys_list;									//	linked list of subsystems for this ship.
	ship_subsys	*last_targeted_subobject[MAX_PLAYERS];	// Last subobject that has been targeted.  NULL if none;(player specific)
	ship_subsys_info	subsys_info[SUBSYSTEM_MAX];		// info on particular generic types of subsystems	

	// subsystem information - Goober5000, in case of duplicate
	int		n_subsystems;						// this number comes from ships.tbl
	model_subsystem *subsystems;				// see model.h for structure definition

	float	*shield_integrity;					//	Integrity at each triangle in shield mesh.

	// ETS fields
	int	shield_recharge_index;			// index into array holding the shield recharge rate
	int	weapon_recharge_index;			// index into array holding the weapon recharge rate
	int	engine_recharge_index;			// index into array holding the engine recharge rate
	float	weapon_energy;						// Number of EUs in energy reserves
	float	current_max_speed;				// Max ship speed (based on energy diverted to engines)
	int	next_manage_ets;					// timestamp for when ai can next modify ets ( -1 means never )

	uint	flags;								// flag variable to contain ship state (see SF_ #defines)
	uint	flags2;								// another flag variable (see SF2_ #defines)
	int	reinforcement_index;				// index into reinforcement struct or -1
	
	float	afterburner_fuel;					// amount of afterburner fuel remaining (capacity is stored
													// as afterburner_fuel_capacity in ship_info).

	int cmeasure_count;						//	Number of charges of countermeasures this ship can hold.
	int current_cmeasure;					//	Currently selected countermeasure.

	int cmeasure_fire_stamp;				//	Time at which can fire countermeasure.

	float	target_shields_delta;			//	Target for shield recharge system.
	float	target_weapon_energy_delta;	//	Target for recharge system.
	ship_weapon	weapons;

	int	shield_hits;						//	Number of hits on shield this frame.

	float		wash_intensity;
	vec3d	wash_rot_axis;
	int		wash_timestamp;

	// store blast information about shockwaves that hit the ship
//	ship_shockwave	sw;

	int	num_swarm_missiles_to_fire;	// number of swarm missiles that need to be launched
	int	next_swarm_fire;					// timestamp of next swarm missile to fire
	int	next_swarm_path;					// next path number for swarm missile to take
	int	num_turret_swarm_info;			// number of turrets in process of launching swarm

	int	group;								// group ship is in, or -1 if none.  Fred thing
	int	death_roll_snd;					// id of death roll sound, may need to be stopped early	
	int	ship_list_index;					// index of ship in Ship_objs[] array

	int	thruster_bitmap;					// What frame the current thruster bitmap is at for this ship
	float	thruster_frame;					// Used to keep track of which frame the animation should be on.

	int	thruster_glow_bitmap;			// What frame the current thruster engine glow bitmap is at for this ship
	float	thruster_glow_frame;				// Used to keep track of which frame the engine glow animation should be on.
	float	thruster_glow_noise;				// Noise for current frame

	int	thruster_secondary_glow_bitmap;		// Bobboau
	int	thruster_tertiary_glow_bitmap;		// Bobboau

	int	next_engine_stutter;				// timestamp to time the engine stuttering when a ship dies

	float total_damage_received;        // total damage received (for scoring purposes)
	float damage_ship[MAX_DAMAGE_SLOTS];    // damage applied from each player
	int   damage_ship_id[MAX_DAMAGE_SLOTS]; // signature of the damager (corresponds to each entry in damage_ship)
	int	persona_index;						// which persona is this guy.

	int	subsys_disrupted_flags;					// bitflags used to check if SUBYSTEM_* is disrupted or not
	int	subsys_disrupted_check_timestamp;	// timer to control how oftern flags are set/cleared in subsys_disrupted_flags

	uint	create_time;						// time ship was created, set by gettime()

	// keep multiplayer specific stuff below this point	
	int	ts_index;							// index into the team select and Wss_slots array (or -1 if not in one of those arrays)

	int	large_ship_blowup_index;			// -1 if not a large ship exploding, else this is an index used by the shipfx large ship exploding code.
	int	sub_expl_sound_handle[NUM_SUB_EXPL_HANDLES];


	// Stuff for showing electrical arcs on damaged ships
	vec3d	arc_pts[MAX_SHIP_ARCS][2];			// The endpoints of each arc
	int		arc_timestamp[MAX_SHIP_ARCS];		// When this times out, the spark goes away.  -1 is not used
	ubyte		arc_type[MAX_SHIP_ARCS];			// see MARC_TYPE_* defines in model.h
	int		arc_next_time;							// When the next arc will be created.	

	// emp missile stuff
	float emp_intensity;								// <= 0.0f if no emp effect present
	float emp_decr;									// how much to decrement EMP effect per second for this ship

	// contrail stuff
	trail *trail_ptr[MAX_SHIP_CONTRAILS];	

	// tag stuff
	float tag_total;									// total tag time
	float tag_left;									// total tag remaining	
	fix	time_first_tagged;
	float level2_tag_total;							// total tag time
	float level2_tag_left;							// total tag remaining	

	// old-style object update stuff
	np_update		np_updates[MAX_PLAYERS];	// for both server and client

	// lightning timestamp
	int lightning_stamp;

	// AWACS warning flag
	ubyte	awacs_warning_flag;

	// Special warpout objnum (warpout at knossos)
	int special_warp_objnum;

	ship_subsys fighter_beam_turret_data;		//a fake subsystem that pretends to be a turret for fighter beams
	model_subsystem beam_sys_info;
	int was_firing_last_frame[MAX_SHIP_PRIMARY_BANKS];

	// Goober5000 - range of primitive sensors
	int primitive_sensor_range;
	
	// Goober5000 - revised nameplate implementation
	int *replacement_textures;
	int replacement_textures_buf[MAX_MODEL_TEXTURES];	// replacement texture for this ship

	trail *ABtrail_ptr[MAX_SHIP_CONTRAILS];		//after burner trails -Bobboau
	trail_info ab_info[MAX_SHIP_CONTRAILS];
	int ab_count;

//	decal decals[MAX_SHIP_DECALS];	//the decals of the ship
	int glows_active;
	int glowmaps_active;
	int n_decal;

	//cloaking stuff
	vec3d texture_translation_key;		//translate the texture matrix for a cool effect
	vec3d current_translation;
	int cloak_stage;
	fix time_until_full_cloak;
	int cloak_alpha;
	fix time_until_uncloak;

	decal_system ship_decal_system;

#ifdef NEW_HUD
	hud ship_hud;
#endif

	int last_fired_point[MAX_SHIP_PRIMARY_BANKS]; //for fire point cylceing

	bool bay_doors_open;			//are the bay doors open right now
	int bay_number_wanting_open;	//the number of fighters that want my bay doors open
	bool bay_doors_want_open;		//when this value changes I tell my parent
	int bay_doors_open_time;		//the time when the bay doors will be open
	bool bay_doors_open_last_frame;	//start moveing as soon as the bay doors are totaly open
	int launched_from;				//wich bay path I launched from

	//ok, when a fighter is made in a fighter bay it will add one to bay_number_wanting_open
	//when a fighter reaches the second point on a bay path it will subtract one from this variable
	//when nobody wants the bay doors open anymore, the parent ship will close them

	float secondary_point_reload_pct[MAX_SHIP_SECONDARY_BANKS][MAX_SLOTS];	//after fireing a secondary it takes some time for that secondary weapon to reload, this is how far along in that proces it is (from 0 to 1)
	float reload_time[MAX_SHIP_SECONDARY_BANKS]; //how many seconds it will take for any point in a bank to reload
	float primary_rotate_rate[MAX_SHIP_PRIMARY_BANKS];
	float primary_rotate_ang[MAX_SHIP_PRIMARY_BANKS];
/*
	flash_ball	*debris_flare;
	int n_debris_flare;
	float flare_life;
	int flare_bm;
	*/
} ship;

// structure and array def for ships that have exited the game.  Keeps track of certain useful
// information.
#define SEF_DESTROYED			(1<<0)
#define SEF_DEPARTED				(1<<1)
#define SEF_CARGO_KNOWN			(1<<2)
#define SEF_PLAYER_DELETED		(1<<3)			// ship deleted by a player in ship select
#define SEF_BEEN_TAGGED			(1<<4)
#define SEF_RED_ALERT_CARRY	(1<<5)

#define MAX_EXITED_SHIPS	(2*MAX_SHIPS) //DTP changed for MAX_SHIPS sake. double of max_ships.

typedef struct exited_ship {
	char		ship_name[NAME_LENGTH];
	int		obj_signature;
	int		team;
	int		flags;
	fix		time;
	int		hull_strength;
	fix		time_cargo_revealed;
	char	cargo1;
} exited_ship;

extern exited_ship Ships_exited[MAX_EXITED_SHIPS];

// a couple of functions to get at the data
extern void ship_add_exited_ship( ship *shipp, int reason );
extern int ship_find_exited_ship_by_name( char *name );
extern int ship_find_exited_ship_by_signature( int signature);

#define	SIF_DO_COLLISION_CHECK	(1 << 0)
#define	SIF_PLAYER_SHIP			(1 << 1)
#define	SIF_DEFAULT_PLAYER_SHIP	(1 << 2)
#define	SIF_PATH_FIXUP				(1 << 3)		// when set, path verts have been set for this ship's model
#define	SIF_SUPPORT					(1 << 4)		// this ship can perform repair/rearm functions
#define	SIF_AFTERBURNER			(1 << 5)		// this ship has afterburners
#define SIF_BALLISTIC_PRIMARIES (1 << 6)		// this ship can equip ballistic primaries - Goober5000

// If you add a new ship type, then please add the appriopriate type in the ship_count
// structure later in this file!!! and let MWA know!!
#define	SIF_CARGO					(1 << 7)		// is this ship a cargo type ship -- used for docking purposes
#define	SIF_FIGHTER					(1 << 8)		// this ship is a fighter
#define	SIF_BOMBER					(1 << 9)		// this ship is a bomber
#define	SIF_CRUISER					(1 << 10)		// this ship is a cruiser
#define	SIF_FREIGHTER				(1 << 11)	// this ship is a freighter
#define	SIF_CAPITAL					(1 << 12)	// this ship is a capital/installation ship
#define	SIF_TRANSPORT				(1 << 13)	// this ship is a transport
#define	SIF_NAVBUOY					(1	<< 14)	// AL 11-24-97: this is a navbuoy
#define	SIF_SENTRYGUN				(1	<< 15)	// AL 11-24-97: this is a navbuoy with turrets
#define	SIF_ESCAPEPOD				(1 << 16)	// AL 12-09-97: escape pods that fire from big ships
#define	SIF_NO_SHIP_TYPE			(1 << 17)	// made distinct to help trap errors

#define	SIF_SHIP_COPY				(1 << 18)	// this ship is a copy of another ship in the table -- meaningful for scoring and possible other things
#define	SIF_IN_TECH_DATABASE		(1 << 19)	// is ship type to be listed in the tech database?
#define	SIF_IN_TECH_DATABASE_M	(1 << 20)	// is ship type to be listed in the tech database for multiplayer?

#define	SIF_STEALTH					(1 << 21)	// the ship has stealth capabilities
#define	SIF_SUPERCAP				(1 << 22)	// the ship is a supercap
#define	SIF_DRYDOCK					(1 << 23)	// the ship is a drydock
#define	SIF_SHIP_CLASS_DONT_COLLIDE_INVIS	(1 << 24)	// Don't collide with this ship's invisible polygons

#define	SIF_BIG_DAMAGE				(1 << 25)	// this ship is classified as a big damage ship
#define	SIF_HAS_AWACS				(1 << 26)	// ship has an awacs subsystem

#define	SIF_CORVETTE				(1 << 27)	// corvette class (currently this only means anything for briefing icons)
#define	SIF_GAS_MINER				(1 << 28)	// also just for briefing icons
#define	SIF_AWACS					(1 << 29)	// ditto

#define	SIF_KNOSSOS_DEVICE		(1 << 30)	// this is the knossos device

#define	SIF_NO_FRED					(1 << 31)	// not available in fred

// flags2 -- added by Goober5000
#define SIF2_DEFAULT_IN_TECH_DATABASE		(1 << 0)	// default in tech database - Goober5000
#define SIF2_DEFAULT_IN_TECH_DATABASE_M		(1 << 1)	// ditto - Goober5000
#define SIF2_FLASH							(1 << 2)	// makes a flash when it explodes
#define SIF2_SHOW_SHIP_MODEL				(1 << 3)	// Show ship model even in first person view
#define SIF2_SURFACE_SHIELDS                (1 << 4)    // _argv[-1], 16 Jan 2005: Enable surface shields for this ship.
#define SIF2_GENERATE_HUD_ICON				(1 << 5)	// Enable generation of a HUD shield icon
#define SIF2_DISABLE_WEAP_DAMAGE_SCALING	(1 << 6)	// WMC - Disable weapon scaling based on flags

#define	MAX_SHIP_FLAGS	8		//	Number of flags for flags field in ship_info struct
#define	SIF_DEFAULT_VALUE			(SIF_DO_COLLISION_CHECK)
#define SIF2_DEFAULT_VALUE		0

#define	SIF_ALL_SHIP_TYPES		(SIF_CARGO | SIF_FIGHTER | SIF_BOMBER | SIF_CRUISER | SIF_FREIGHTER | SIF_CAPITAL | SIF_TRANSPORT | SIF_SUPPORT | SIF_NO_SHIP_TYPE | SIF_NAVBUOY | SIF_SENTRYGUN | SIF_ESCAPEPOD | SIF_SUPERCAP | SIF_CORVETTE | SIF_GAS_MINER | SIF_AWACS | SIF_KNOSSOS_DEVICE)
#define	SIF_SMALL_SHIP				(SIF_FIGHTER | SIF_BOMBER | SIF_SUPPORT | SIF_ESCAPEPOD )
#define	SIF_BIG_SHIP				(SIF_CRUISER | SIF_FREIGHTER | SIF_TRANSPORT | SIF_CORVETTE | SIF_GAS_MINER | SIF_AWACS)
#define	SIF_HUGE_SHIP				(SIF_CAPITAL | SIF_SUPERCAP | SIF_DRYDOCK | SIF_KNOSSOS_DEVICE)
#define	SIF_NOT_FLYABLE			(SIF_CARGO | SIF_NAVBUOY | SIF_SENTRYGUN)		// AL 11-24-97: this useful to know for targeting reasons
#define	SIF_HARMLESS				(SIF_CARGO | SIF_NAVBUOY | SIF_ESCAPEPOD)		// AL 12-3-97: ships that are not a threat
// for ships of this type, we make beam weapons miss a little bit otherwise they'd be way too powerful
#define	SIF_BEAM_JITTER			(SIF_CARGO | SIF_FIGHTER | SIF_BOMBER | SIF_FREIGHTER | SIF_TRANSPORT | SIF_SENTRYGUN | SIF_NAVBUOY | SIF_ESCAPEPOD)

#define REGULAR_WEAPON	(1<<0)
#define DOGFIGHT_WEAPON (1<<1)

#define	MAX_THRUSTER_PARTICLES 3
typedef struct thruster_particles{
	char		thruster_particle_bitmap01_name[NAME_LENGTH];
	int			thruster_particle_bitmap01;
	int			thruster_particle_bitmap01_nframes;
	float		min_rad;
	float		max_rad;
	int			n_high;
	int			n_low;
	float		variance;
	}thruster_particles;

// defines for ship types.  These defines are distinct from the flag values in the ship_info struct.  These
// values are used for array lookups, etc.
/*
#define MAX_SHIP_TYPE_COUNTS				20
*/
/*
#define SHIP_TYPE_NONE						0
#define SHIP_TYPE_CARGO						1
#define SHIP_TYPE_FIGHTER_BOMBER			2
#define SHIP_TYPE_CRUISER					3
#define SHIP_TYPE_FREIGHTER				4
#define SHIP_TYPE_CAPITAL					5
#define SHIP_TYPE_TRANSPORT				6
#define SHIP_TYPE_REPAIR_REARM			7
#define SHIP_TYPE_NAVBUOY					8
#define SHIP_TYPE_SENTRYGUN				9
#define SHIP_TYPE_ESCAPEPOD				10
#define SHIP_TYPE_SUPERCAP					11
#define SHIP_TYPE_STEALTH					12	// this is really never used, because a stealth ship must also be some other class (fighter, etc.)
#define SHIP_TYPE_FIGHTER					13
#define SHIP_TYPE_BOMBER					14
#define SHIP_TYPE_DRYDOCK					15
#define SHIP_TYPE_AWACS						16
#define SHIP_TYPE_GAS_MINER				17
#define SHIP_TYPE_CORVETTE					18
#define SHIP_TYPE_KNOSSOS_DEVICE			19
*/

#define STI_MSG_COUNTS_FOR_ALONE		(1<<0)
#define STI_MSG_PRAISE_DESTRUCTION		(1<<1)

#define STI_HUD_HOTKEY_ON_LIST			(1<<0)
#define STI_HUD_TARGET_AS_THREAT		(1<<1)
#define STI_HUD_SHOW_ATTACK_DIRECTION	(1<<2)

#define STI_SHIP_SCANNABLE				(1<<0)
#define STI_SHIP_WARP_PUSHES			(1<<1)
#define STI_SHIP_WARP_PUSHABLE			(1<<2)

#define STI_WEAP_BEAMS_EASILY_HIT		(1<<0)

#define STI_AI_ACCEPT_PLAYER_ORDERS		(1<<0)
#define STI_AI_AUTO_ATTACKS				(1<<1)
#define STI_AI_ATTEMPT_BROADSIDE		(1<<2)
#define STI_AI_GUARDS_ATTACK			(1<<3)
#define STI_AI_TURRETS_ATTACK			(1<<4)
#define STI_AI_CAN_FORM_WING			(1<<5)

typedef struct ship_type_info {
	char name[NAME_LENGTH];

	//Messaging?
	int message_bools;

	//HUD
	int hud_bools;

	//Ship
	int ship_bools;	//For lack of better term
	float debris_max_speed;

	//Weapons
	int weapon_bools;
	float ff_multiplier;
	float emp_multiplier;

	//Fog
	float fog_start_dist;
	float fog_complete_dist;

	//AI
	int	ai_valid_goals;
	int ai_player_orders;
	int ai_bools;
	int ai_active_dock;
	int ai_passive_dock;
	std::vector<int> ai_actively_pursues;

	//Regen values - need to be converted after all types have loaded
	std::vector<std::string> ai_actively_pursues_temp;
} ship_type_info;

extern std::vector<ship_type_info> Ship_types;

#define MT_BANK_RIGHT		(1<<0)
#define MT_BANK_LEFT		(1<<1)
#define MT_PITCH_UP			(1<<2)
#define MT_PITCH_DOWN		(1<<3)
#define MT_ROLL_RIGHT		(1<<4)
#define MT_ROLL_LEFT		(1<<5)
#define MT_SLIDE_RIGHT		(1<<6)
#define MT_SLIDE_LEFT		(1<<7)
#define MT_SLIDE_UP			(1<<8)
#define MT_SLIDE_DOWN		(1<<9)
#define MT_FORWARD			(1<<10)
#define MT_REVERSE			(1<<11)

#define MAX_MAN_THRUSTERS	32
typedef struct man_thruster {
	int bmap_id;
	float radius;
	int use_flags;
	vec3d pos, norm;
	man_thruster(){memset(this, 0, sizeof(man_thruster));bmap_id=-1;}
}man_thruster;

// The real FreeSpace ship_info struct.
typedef struct ship_info {
	char		name[NAME_LENGTH];				// name for the ship
	char		short_name[NAME_LENGTH];		// short name, for use in the editor?
	int			species;								// which species this craft belongs to
	int			class_type;						//For type table

	char		*type_str;							// type string used by tooltips
	char		*maneuverability_str;			// string used by tooltips
	char		*armor_str;							// string used by tooltips
	char		*manufacturer_str;				// string used by tooltips
	char		*desc;								// string used by tooltips
	char		*tech_desc;							// string used by tech database

	char     *ship_length;						// string used by multiplayer ship desc
	char     *gun_mounts;			         // string used by multiplayer ship desc
	char     *missile_banks;					// string used by multiplayer ship desc

	char		pof_file[NAME_LENGTH];			// POF file to load/associate with ship
	char		pof_file_hud[NAME_LENGTH];		// POF file to load for the HUD target box
	int		num_detail_levels;				// number of detail levels for this ship
	int		detail_distance[MAX_SHIP_DETAIL_LEVELS];					// distance to change detail levels at
	int		modelnum;							// ship model
	int		modelnum_hud;						// model to use when rendering to the HUD (eg, mini supercap)
	int		hud_target_lod;						// LOD to use for rendering to the HUD targetbox (if not already using special HUD model)
	float		density;								// density of the ship in g/cm^3 (water  = 1)
	float		damp;									// drag
	float		rotdamp;								// rotational drag
	vec3d	max_vel;								//	max velocity of the ship in the linear directions -- read from ships.tbl
	vec3d	afterburner_max_vel;				//	max velocity of the ship in the linear directions when afterburners are engaged -- read from ships.tbl
	vec3d	max_rotvel;							// maximum rotational velocity
	vec3d	rotation_time;						// time to rotate in x/y/z dimension traveling at max rotvel
	float		srotation_time;					//	scalar, computed at runtime as (rotation_time.x + rotation_time.y)/2
	float		max_rear_vel;						// max speed ship can go backwards.
	float		forward_accel;
	float		afterburner_forward_accel;		// forward acceleration with afterburner engaged
	float		forward_decel;
	float		slide_accel;
	float		slide_decel;
	float		warpin_speed;
	script_hook warpin_hook;
	float		warpout_speed;
	script_hook warpout_hook;
	float		warpout_player_speed;

	uint		flags;							//	See SIF_xxxx - changed to uint by Goober5000
	uint		flags2;							//	See SIF2_xxxx - added by Goober5000
	int		ai_class;							//	Index into Ai_classes[].  Defined in ai.tbl
	float		max_speed, min_speed, max_accel;

	// ship explosion info
	shockwave_create_info shockwave;
	int	explosion_propagates;				// If true, then the explosion propagates
	int	shockwave_count;						// the # of total shockwaves
	/*
	float inner_rad;								// radius within which maximum damage is applied
	float	outer_rad;								// radius at which no damage is applied
	float damage;									// maximum damage applied from ship explosion
	float blast;									// maximum blast impulse from ship explosion									
	float	shockwave_speed;						// speed at which shockwave expands, 0 means no shockwave
	char shockwave_pof_file[NAME_LENGTH];			// POF file to load/associate with ship's shockwave
	int shockwave_model;
	char shockwave_name[NAME_LENGTH];
	int shockwave_info_index;*/

	int ispew_max_particles;						//Temp field until someone works on particles -C
	int dspew_max_particles;						//Temp field until someone works on particles -C

	// subsystem information
	int		n_subsystems;						// this number comes from ships.tbl
	model_subsystem *subsystems;				// see model.h for structure definition

	// Energy Transfer System fields
	float		power_output;						// power output of ships reactor (EU/s)
	float		max_overclocked_speed;			// max speed when 100% power output sent to engines
	float		max_weapon_reserve;				// maximum energy that can be stored for primary weapon usage

	// Afterburner fields
	float		afterburner_fuel_capacity;		// maximum afterburner fuel that can be stored
	float		afterburner_burn_rate;			// rate in fuel/second that afterburner consumes fuel
	float		afterburner_recover_rate;		//	rate in fuel/second that afterburner recovers fuel

	int		cmeasure_type;						// Type of countermeasures this ship carries
	int		cmeasure_max;						//	Number of charges of countermeasures this ship can hold.

	int num_primary_banks;										// Actual number of primary banks (property of model)
	int num_secondary_banks;									//	Actual number of secondary banks (property of model)
	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];			// Weapon_info[] index for the weapon in the bank
	
	// Goober5000's ballistic conversion
	int primary_bank_ammo_capacity[MAX_SHIP_PRIMARY_BANKS];	// Capacity of primary ballistic bank
	
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];	// Weapon_info[] index for the weapon in the bank
	int secondary_bank_ammo_capacity[MAX_SHIP_SECONDARY_BANKS];	// Capacity of bank (not number of missiles)

	float	max_hull_strength;				// Max hull strength of this class of ship.
	float	max_shield_strength;

	float	hull_repair_rate;				//How much of the hull is repaired every second
	float	subsys_repair_rate;		//How fast 

	int engine_snd;							// handle to engine sound for ship (-1 if no engine sound)

	vec3d	closeup_pos;					// position for camera when using ship in closeup view (eg briefing and hud target monitor)
	float		closeup_zoom;					// zoom when using ship in closeup view (eg briefing and hud target monitor)

	int		allowed_weapons[MAX_WEAPON_TYPES];	// array which specifies which weapons can be loaded out by the
												// player during weapons loadout.

	// Goober5000 - fix for restricted banks mod
	int restricted_loadout_flag[MAX_SHIP_WEAPONS];
	int allowed_bank_restricted_weapons[MAX_SHIP_WEAPONS][MAX_WEAPON_TYPES];

	ubyte	shield_icon_index;				// index to locate ship-specific animation frames for the shield on HUD
	char	icon_filename[NAME_LENGTH];	// filename for icon that is displayed in ship selection
	char	anim_filename[NAME_LENGTH];	// filename for animation that plays in ship selection
	char	overhead_filename[NAME_LENGTH];	// filename for animation that plays weapons loadout

	int	score;								// default score for this ship

	int	scan_time;							// time to scan this ship (in ms)

	// contrail info
	trail_info ct_info[MAX_SHIP_CONTRAILS];	
	int ct_count;

	// rgb non-dimming pixels for this ship type
	int num_nondark_colors;
	ubyte nondark_colors[MAX_NONDARK_COLORS][3];

	// rgb shield color
	ubyte shield_color[3];

	//optional ABtrail values
	char		ABtrail_bitmap_name[MAX_FILENAME_LEN];
	int			ABbitmap;		//the bitmap used
	float		ABwidth_factor;	//a number that the width (set by the thruster glow width) will be multiplyed by
	float		ABAlpha_factor;	//allows you to set how starting trasparency value
	float		ABlife;			//how long the trails live for

	int			n_thruster_particles;
	int			n_ABthruster_particles;
	thruster_particles	normal_thruster_particles[MAX_THRUSTER_PARTICLES];
	thruster_particles	afterburner_thruster_particles[MAX_THRUSTER_PARTICLES];

	// Bobboau's extra thruster stuff
	thrust_pair			thruster_glow_info;
	thrust_pair_bitmap	thruster_secondary_glow_info;
	thrust_pair_bitmap	thruster_tertiary_glow_info;
	float		thruster01_glow_rad_factor;
	float		thruster02_glow_rad_factor;
	float		thruster03_glow_rad_factor;
	float		thruster_glow_len_factor;

	int splodeing_texture;
	char splodeing_texture_name[NAME_LENGTH];
	int max_decals;

	bool draw_primary_models[MAX_SHIP_PRIMARY_BANKS];
	bool draw_secondary_models[MAX_SHIP_SECONDARY_BANKS];
	bool draw_models; //any weapon mode will be drawn
	float weapon_model_draw_distance;
	
	int armor_type_idx;
	
	bool can_glide;
	
	bool topdown_offset_def;
	vec3d topdown_offset;

	int num_maneuvering;
	man_thruster maneuvering[MAX_MAN_THRUSTERS];
} ship_info;

extern int Num_wings;
extern ship Ships[MAX_SHIPS];
extern ship	*Player_ship;

// Data structure to track the active missiles
typedef struct ship_obj {
	ship_obj		 *next, *prev;
	int			flags, objnum;
} ship_obj;
extern ship_obj Ship_obj_list;

typedef struct engine_wash_info
{
	char		name[NAME_LENGTH];
	float		angle;			// half angle of cone around engine thruster
	float		radius_mult;	// multiplier for radius 
	float		length;			// length of engine wash, measured from thruster
	float		intensity;		// intensity of engine wash
	
	engine_wash_info();
} engine_wash_info;

extern std::vector<engine_wash_info> Engine_wash_info;

// flags defined for wings
#define MAX_WING_FLAGS				8				// total number of flags in the wing structure -- used for parsing wing flags
#define WF_WING_GONE					(1<<0)		// all ships were either destroyed or departed
#define WF_WING_DEPARTING			(1<<1)		// wing's departure cue turned true
#define WF_IGNORE_COUNT				(1<<2)		// ignore all ships in this wing for goal counting purposes.
#define WF_REINFORCEMENT			(1<<3)		// is this wing a reinforcement wing
#define WF_RESET_REINFORCEMENT	(1<<4)		// needed when we need to reset the wing's reinforcement flag (after calling it in)
#define WF_NO_ARRIVAL_MUSIC		(1<<5)		// don't play arrival music when wing arrives
#define WF_EXPANDED					(1<<6)		// wing expanded in hotkey select screen
#define WF_NO_ARRIVAL_MESSAGE		(1<<7)		// don't play any arrival message
#define WF_NO_ARRIVAL_WARP			(1<<8)		// don't play warp effect for any arriving ships in this wing.
#define WF_NO_DEPARTURE_WARP		(1<<9)		// don't play warp effect for any departing ships in this wing.
#define WF_NO_DYNAMIC				(1<<10)		// members of this wing relentlessly pursue their ai goals
#define WF_DEPARTURE_ORDERED		(1<<11)		// departure of this wing was ordered by player
#define WF_NEVER_EXISTED			(1<<12)		// this wing never existed because something prevented it from being created (like its mother ship being destroyed)
#define WF_NAV_CARRY				(1<<13)		// Kazan - Wing has nav-carry-status

//	Defines a wing of ships.
typedef struct wing {
	char	name[NAME_LENGTH];
	char	wing_squad_filename[MAX_FILENAME_LEN+1];	// Goober5000
	int	reinforcement_index;					// index in reinforcement struct or -1
	int	hotkey;

	int	num_waves, current_wave;			// members for dealing with waves
	int	threshold;								// when number of ships in the wing reaches this number -- new wave

	fix	time_gone;								// time into the mission when this wing is officiall gone.

	int	wave_count;								// max ships per wave (as defined by the number of ships in the ships list)
	int	total_arrived_count;					// count of number of ships that we have created, regardless of wave
	int	current_count;							// count of number of ships actually in this wing -- used for limit in next array
	int	ship_index[MAX_SHIPS_PER_WING];	// index into ships array of all ships currently in the wing

	int	total_destroyed;						// total number of ships destroyed in the wing (including all waves)
	int	total_departed;						// total number of ships departed in this wing (including all waves)

	int	special_ship;							// the leader of the wing.  An index into ship_index[].

	int	arrival_location;						// arrival and departure information for wings -- similar to info for ships
	int	arrival_distance;						// distance from some ship where this ship arrives
	int	arrival_anchor;						// name of object this ship arrives near (or in front of)
	int	arrival_cue;
	int	arrival_delay;

	int	departure_location;
	int	departure_anchor;						// name of object that we depart to (in case of dock bays)
	int	departure_cue;
	int	departure_delay;

	int	wave_delay_min;						// minimum number of seconds before new wave can arrive
	int	wave_delay_max;						// maximum number of seconds before new wave can arrive
	int	wave_delay_timestamp;				// timestamp used for delaying arrival of next wave

	int flags;

	ai_goal	ai_goals[MAX_AI_GOALS];			// goals for the wing -- converted to ai_goal struct

	ushort	net_signature;						// starting net signature for ships in this wing. assiged at mission load time

	// Goober5000 - if this wing has a unique squad logo
	// it's specified for the wing rather than each individual ship to cut down on the amount
	// of stuff that needs to be sitting in memory at once - each ship uses the wing texture;
	// and it also makes practical sense: no wing has two different squadrons in it :)
	int wing_insignia_texture;
} wing;

extern wing Wings[MAX_WINGS];

extern int Starting_wings[MAX_STARTING_WINGS];
extern int Squadron_wings[MAX_SQUADRON_WINGS];
extern int TVT_wings[MAX_TVT_WINGS];

extern char Starting_wing_names[MAX_STARTING_WINGS][NAME_LENGTH];
extern char Squadron_wing_names[MAX_SQUADRON_WINGS][NAME_LENGTH];
extern char TVT_wing_names[MAX_TVT_WINGS][NAME_LENGTH];

extern int ai_paused;
extern int CLOAKMAP;

extern int Num_reinforcements;
extern int Num_ship_classes;
extern ship_info Ship_info[MAX_SHIP_CLASSES];
extern reinforcements Reinforcements[MAX_REINFORCEMENTS];

// structure definition for ship type counts.  Used to give a count of the number of ships
// of a particular type, and the number of times that a ship of that particular type has been
// killed.  When changing any info here, be sure to update the ship_type_names array in Ship.cpp
// the order of the types here MUST match the order of the types in the array
typedef struct ship_counts {
	int	total;
	int	killed;
	ship_counts(){total=0;killed=0;}
} ship_counts;

extern std::vector<ship_counts> Ship_type_counts;

//extern int Ship_type_flags[MAX_SHIP_TYPE_COUNTS];					// SIF_* flags for each ship type
//extern ship_counts	Ship_counts[MAX_SHIP_TYPE_COUNTS];
//extern int Ship_type_priorities[MAX_SHIP_TYPE_COUNTS];


// Use the below macros when you want to find the index of an array element in the
// Wings[] or Ships[] arrays.
#define WING_INDEX(wingp) (wingp-Wings)
#define SHIP_INDEX(shipp) (shipp-Ships)


extern void ship_init();				// called once	at game start
extern void ship_level_init();		// called before the start of each level

//returns -1 if failed
extern int ship_create(matrix * orient, vec3d * pos, int ship_type, char *ship_name = NULL);
extern void change_ship_type(int n, int ship_type, int by_sexp = 0);
extern void ship_model_change(int n, int ship_type, int changing_ship_class = 0);
extern void ship_process_pre( object * objp, float frametime );
extern void ship_process_post( object * objp, float frametime );
extern void ship_render( object * objp );
extern void ship_delete( object * objp );
extern int ship_check_collision_fast( object * obj, object * other_obj, vec3d * hitpos );
extern int ship_get_num_ships();

extern int ship_fire_primary_debug(object *objp);	//	Fire the debug laser.
extern int ship_stop_fire_primary(object * obj);
extern int ship_fire_primary(object * objp, int stream_weapons, int force = 0);
extern int ship_fire_secondary(object * objp, int allow_swarm = 0 );
extern int ship_launch_countermeasure(object *objp, int rand_val = -1);

// for special targeting lasers
extern void ship_start_targeting_laser(ship *shipp);
extern void ship_stop_targeting_laser(ship *shipp);
extern void ship_process_targeting_lasers();

extern int ship_select_next_primary(object *objp, int direction);
extern int  ship_select_next_secondary(object *objp);

// Goober5000
extern int get_available_primary_weapons(object *objp, int *outlist, int *outbanklist);

extern int get_available_secondary_weapons(object *objp, int *outlist, int *outbanklist);
extern void ship_recalc_subsys_strength( ship *shipp );
extern void subsys_set(int objnum, int ignore_subsys_info = 0);
extern void physics_ship_init(object *objp);

//	Note: This is not a general purpose routine.
//	It is specifically used for targeting.
//	It only returns a subsystem position if it has shields.
//	Return true/false for subsystem found/not found.
//	Stuff vector *pos with absolute position.
extern int get_subsystem_pos(vec3d *pos, object *objp, ship_subsys *subsysp);

extern int ship_info_lookup(char *name = NULL);
extern int ship_info_base_lookup(int si_index);
extern int ship_name_lookup(char *name, int inc_players = 0);	// returns the index into Ship array of name
extern int ship_type_name_lookup(char *name);

extern int wing_lookup(char *name);

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
extern int wing_has_conflicting_teams(int wing_index);

// next function takes optional second parameter which says to ignore the current count of ships
// in the wing -- used to tell is the wing exists or not, not whether it exists and has ships currently
// present.
extern int wing_name_lookup(char *name, int ignore_count = 0);

extern int Player_ship_class;

#define MAX_PLAYER_SHIP_CHOICES	15
/*
extern int Num_player_ship_precedence;				// Number of ship types in Player_ship_precedence
extern int Player_ship_precedence[MAX_PLAYER_SHIP_CHOICES];	// Array of ship types, precedence list for player ship/wing selection
*/

//	Do the special effect for energy dissipating into the shield for a hit.
//	model_num	= index in Polygon_models[]
//	centerp		= pos of object, sort of the center of the shield
//	tcp			= hit point, probably the global hit_point set in polygon_check_face
//	tr0			= index of polygon in shield pointer in polymodel.
extern void create_shield_explosion(int objnum, int model_num, matrix *orient, vec3d *centerp, vec3d *tcp, int tr0);

//	Initialize shield hit system.
extern void shield_hit_init();
extern void create_shield_explosion_all(object *objp);
extern void shield_frame_init();
extern void add_shield_point(int objnum, int tri_num, vec3d *hit_pos);
extern void add_shield_point_multi(int objnum, int tri_num, vec3d *hit_pos);
extern void shield_point_multi_setup();
extern void shield_hit_close();

void ship_draw_shield( object *objp);

float apply_damage_to_shield(object *objp, int quadrant, float damage);
float compute_shield_strength(object *objp);

// Returns true if the shield presents any opposition to something 
// trying to force through it.
// If quadrant is -1, looks at entire shield, otherwise
// just one quadrant
int ship_is_shield_up( object *obj, int quadrant );

//=================================================
// These two functions transfer instance specific angle
// data into and out of the model structure, which contains
// angles, but not for each instance of model being used. See
// the actual functions in ship.cpp for more details.
extern void ship_model_start(object *objp);
extern void ship_model_stop(object *objp);

//============================================
extern int ship_find_num_crewpoints(object *objp);
extern int ship_find_num_turrets(object *objp);

extern void ship_get_eye( vec3d *eye_pos, matrix *eye_orient, object *obj );		// returns in eye the correct viewing position for the given object
extern ship_subsys *ship_get_indexed_subsys( ship *sp, int index, vec3d *attacker_pos = NULL );	// returns index'th subsystem of this ship
extern int ship_get_index_from_subsys(ship_subsys *ssp, int objnum, int error_bypass = 0);
extern int ship_get_subsys_index(ship *sp, char *ss_name, int error_bypass = 0);		// returns numerical index in linked list of subsystems
extern float ship_get_subsystem_strength( ship *shipp, int type );
extern ship_subsys *ship_get_subsys(ship *shipp, char *subsys_name);

// subsys disruption
extern int ship_subsys_disrupted(ship_subsys *ss);
extern int ship_subsys_disrupted(ship *sp, int type);
extern void ship_subsys_set_disrupted(ship_subsys *ss, int time);

extern int	ship_do_rearm_frame( object *objp, float frametime );
extern float ship_calculate_rearm_duration( object *objp );
extern void	ship_wing_cleanup( int shipnum, wing *wingp );

extern object *ship_find_repair_ship( object *requester );
extern void ship_close();	// called in game_shutdown() to free malloced memory


extern void ship_assign_sound_all();
extern void ship_assign_sound(ship *sp);

extern void ship_add_ship_type_count( int ship_info_index, int num );
extern void ship_add_ship_type_kill_count( int ship_info_index );

extern int ship_get_type(char* output, ship_info* sip);
extern int ship_get_default_orders_accepted( ship_info *sip );
extern int ship_query_general_type(int ship);
extern int ship_class_query_general_type(int ship_class);
extern int ship_query_general_type(ship *shipp);
extern int ship_docking_valid(int docker, int dockee);
extern int get_quadrant(vec3d *hit_pnt);						//	Return quadrant num of last hit ponit.

extern void ship_obj_list_rebuild();	// only called by save/restore code
extern int ship_query_state(char *name);

// Goober5000
int ship_primary_bank_has_ammo(int shipnum);	// check if current primary bank has ammo
int ship_secondary_bank_has_ammo(int shipnum);	// check if current secondary bank has ammo

void ship_departed( int num );
int ship_engine_ok_to_warp(ship *sp);		// check if ship has engine power to warp
int ship_navigation_ok_to_warp(ship *sp);	// check if ship has navigation power to warp

int ship_return_subsys_path_normal(ship *sp, ship_subsys *ss, vec3d *gsubpos, vec3d *norm);
int ship_subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye_pos, vec3d* subsys_pos, int do_facing_check=1, float *dot_out=NULL, vec3d *vec_out=NULL);
ship_subsys *ship_return_next_subsys(ship *shipp, int type, vec3d *attacker_pos);

// defines and definition for function to get a random ship of a particular team (any ship,
// any ship but player ships, or only players)
#define SHIP_GET_ANY_SHIP				0
#define SHIP_GET_NO_PLAYERS			1
#define SHIP_GET_ONLY_PLAYERS			2

extern int ship_get_random_team_ship( int team, int flags = SHIP_GET_ANY_SHIP, float max_dist=0.0f );
extern int ship_get_random_player_wing_ship( int flags = SHIP_GET_ANY_SHIP, float max_dist=0.0f, int persona_index = -1, int get_first=0, int multi_team = -1 );
extern int ship_get_random_ship_in_wing(int wingnum, int flags = SHIP_GET_ANY_SHIP, float max_dist=0.0f, int get_first=0 );

// return ship index
int ship_get_random_targetable_ship();

extern int ship_get_by_signature(int signature);

#ifndef NDEBUG
extern int Ai_render_debug_flag;
extern int Show_shield_mesh, New_shield_system;
extern int Ship_auto_repair;	// flag to indicate auto-repair of subsystem should occur
#endif

void ship_subsystem_delete(ship *shipp);
void ship_set_default_weapons(ship *shipp, ship_info *sip);
float ship_quadrant_shield_strength(object *hit_objp, vec3d *hitpos);

int ship_dumbfire_threat(ship *sp);
int ship_lock_threat(ship *sp);

int	bitmask_2_bitnum(int num);
char	*ship_return_orders(char *outbuf, ship *sp);
char	*ship_return_time_to_goal(char *outbuf, ship *sp);
void	ship_check_cargo_all();	// called from game_simulation_frame

void	ship_maybe_warn_player(ship *enemy_sp, float dist);
void	ship_maybe_praise_player(ship *deader_sp);
void	ship_maybe_ask_for_help(ship *sp);
void	ship_scream(ship *sp);
void	ship_maybe_scream(ship *sp);
void	ship_maybe_tell_about_rearm(ship *sp);
void	ship_maybe_lament();

void ship_primary_changed(ship *sp);
void ship_secondary_changed(ship *sp);

// get the Ship_info flags for a given ship
int ship_get_SIF(ship *shipp);
int ship_get_SIF(int sh);

extern void ship_do_cargo_revealed( ship *shipp, int from_network = 0 );
extern void ship_do_cargo_hidden( ship *shipp, int from_network = 0 );
extern void ship_do_cap_subsys_cargo_revealed( ship *shipp, ship_subsys *subsys, int from_network = 0);
extern void ship_do_cap_subsys_cargo_hidden( ship *shipp, ship_subsys *subsys, int from_network = 0);

float ship_get_secondary_weapon_range(ship *shipp);

// Goober5000
int primary_out_of_ammo(ship_weapon *swp, int bank);
int get_max_ammo_count_for_primary_bank(int ship_class, int bank, int ammo_type);

int get_max_ammo_count_for_bank(int ship_class, int bank, int ammo_type);

int is_support_allowed(object *objp);

// Given an object and a turret on that object, return the actual firing point of the gun
// and its normal.   This uses the current turret angles.  We are keeping track of which
// gun to fire next in the ship specific info for this turret subobject.  Use this info
// to determine which position to fire from next.
//	Stuffs:
//		*gpos: absolute position of gun firing point
//		*gvec: vector fro *gpos to *targetp
void ship_get_global_turret_gun_info(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, int use_angles, vec3d *targetp);

//	Given an object and a turret on that object, return the global position and forward vector
//	of the turret.   The gun normal is the unrotated gun normal, (the center of the FOV cone), not
// the actual gun normal given using the current turret heading.  But it _is_ rotated into the model's orientation
//	in global space.
void ship_get_global_turret_info(object *objp, model_subsystem *tp, vec3d *gpos, vec3d *gvec);

// return 1 if objp is in fov of the specified turret, tp.  Otherwise return 0.
//	dist = distance from turret to center point of object
int object_in_turret_fov(object *objp, model_subsystem *tp, vec3d *tvec, vec3d *tpos, float dist);

// forcible jettison cargo from a ship
void object_jettison_cargo(object *objp, object *cargo_objp);

// get damage done by exploding ship, takes into account mods for individual ship
float ship_get_exp_damage(object* objp);

// get whether ship has shockwave, takes into account mods for individual ship
int ship_get_exp_propagates(ship *sp);

// get outer radius of damage, takes into account mods for individual ship
float ship_get_exp_outer_rad(object *ship_objp);

// externed by Goober5000
extern int ship_explode_area_calc_damage( vec3d *pos1, vec3d *pos2, float inner_rad, float outer_rad, float max_damage, float max_blast, float *damage, float *blast );

// returns whether subsys is allowed to have cargo
int valid_cap_subsys_cargo_list(char *subsys_name);

// determine turret status of a given subsystem, returns 0 for no turret, 1 for "fixed turret", 2 for "rotating" turret
int ship_get_turret_type(ship_subsys *subsys);

// get ship by object signature, returns OBJECT INDEX
int ship_get_by_signature(int sig);

// get the team of a reinforcement item
int ship_get_reinforcement_team(int r_index);

// determine if the given texture is used by a ship type. return ship info index, or -1 if not used by a ship
int ship_get_texture(int bitmap);

// page in bitmaps for all ships on a given level
void ship_page_in();

// Goober5000 - helper for above
void ship_page_in_model_textures(int modelnum, int ship_index = -1);

// fixer for above - taylor
void ship_page_out_model_textures(int modelnum, int ship_index = -1);

// update artillery lock info
void ship_update_artillery_lock();

// checks if a world point is inside the extended bounding box of a ship
int check_world_pt_in_expanded_ship_bbox(vec3d *world_pt, object *objp, float delta_box);

// returns true if objp is ship and is tagged
int ship_is_tagged(object *objp);

// returns max normal speed of ship (overclocked / afterburned)
float ship_get_max_speed(ship *shipp);

// returns warpout speed of ship
float ship_get_warpout_speed(object *objp);

// returns true if ship is beginning to speed up in warpout
int ship_is_beginning_warpout_speedup(object *objp);

// given a ship info type, return a species
int ship_get_species_by_type(int ship_info_index);

// return the length of the ship
float ship_get_length(ship* shipp);

// Goober5000 - used by change-ai-class
extern void ship_set_new_ai_class(int ship_num, int new_ai_class);
extern void ship_subsystem_set_new_ai_class(int ship_num, char *subsystem, int new_ai_class);

// wing squad logos - Goober5000
extern void wing_load_squad_bitmap(wing *w);

// Goober5000 - needed by new hangar depart code
extern int ship_has_dock_bay(int shipnum);

// Goober5000 - needed by new hangar depart code
extern int ship_get_ship_with_dock_bay(int team);

// Goober5000 - moved here from hudbrackets.cpp
extern int ship_subsys_is_fighterbay(ship_subsys *ss);

// Goober5000
extern int ship_fighterbays_all_destroyed(ship *shipp);

// Goober5000
extern int ship_subsys_takes_damage(ship_subsys *ss);

//phreak
extern int ship_fire_tertiary(object *objp);

// Goober5000 - handles submodel rotation, incorporating conditions such as gun barrels when firing
extern void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss);

// Goober5000 - shortcut hud stuff
extern int ship_has_energy_weapons(ship *shipp);
extern int ship_has_engine_power(ship *shipp);

//starts an animation of a certan type that may be assosiated with a submodel of a ship
void ship_start_animation_type(ship *shipp, int animation_type, int subtype, int direction);
//how long untill the animation is done
int ship_get_animation_time_type(ship *shipp, int animation_type, int subtype);

void ship_animation_set_inital_states(ship *shipp);

// Goober5000
int ship_starting_wing_lookup(char *wing_name);
int ship_squadron_wing_lookup(char *wing_name);
int ship_tvt_wing_lookup(char *wing_name);

// Goober5000
int ship_class_compare(int ship_class_1, int ship_class_2);

void ship_vanished(object *objp);

int armor_type_get_idx(char* name);

void armor_init();

#endif
