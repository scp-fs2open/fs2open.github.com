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
 * $Revision: 2.111 $
 * $Date: 2004-03-05 23:54:48 $
 * $Author: Goober5000 $
 *
 * Ship (and other object) handling functions
 *
 * $Log: not supported by cvs2svn $
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
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
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
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
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
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
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
#include "ship/aigoals.h"
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

#ifndef NO_NETWORK
#include "network/multiutil.h"
#include "network/multimsgs.h"
#endif

#ifdef FS2_DEMO
	#define MAX_SHIP_SUBOBJECTS		360
#else
	#define MAX_SHIP_SUBOBJECTS		2100 			//Reduced from 1000 to 400 by MK on 4/1/98.  DTP; bumped from 700 to 2100
																// Highest I saw was 164 in sm2-03a which Sandeep says has a lot of ships.
																// JAS: sm3-01 needs 460.   You cannot know this number until *all* ships
																// have warped in.   So I put code in the paging code which knows all ships
																// that will warp in.
#endif

extern bool splodeing;

extern float splode_level;

extern int splodeingtexture;

//#define MIN_COLLISION_MOVE_DIST		5.0
//#define COLLISION_VEL_CONST			0.1
#define COLLISION_FRICTION_FACTOR	0.0		// ratio of maximum friction impulse to repulsion impulse
#define COLLISION_ROTATION_FACTOR	1.0		// increase in rotation from collision

int	Ai_render_debug_flag=0;
#ifndef NDEBUG
int	Ship_sphere_check = 0;
int	Ship_auto_repair = 1;		// flag to indicate auto-repair of subsystem should occur
extern void render_path_points(object *objp);
#endif

// mwa -- removed 11/24/97 int	num_ships = 0;
int	num_wings = 0;
int	Num_reinforcements = 0;
ship	Ships[MAX_SHIPS];
ship	*Player_ship;
wing	Wings[MAX_WINGS];
int	Starting_wings[MAX_PLAYER_WINGS];  // wings player starts a mission with (-1 = none)
int	ships_inited = 0;

engine_wash_info Engine_wash_info[MAX_ENGINE_WASH_TYPES];
char get_engine_wash_index(char *engine_wash_name);

// information for ships which have exited the game
exited_ship Ships_exited[MAX_EXITED_SHIPS];
int Num_exited_ships;

int	Num_engine_wash_types;
int	Num_ship_types;
int	Num_ship_subobj_types;
int	Num_ship_subobjects;
int	Player_ship_class;	// needs to be player specific, move to player structure	

#define		SHIP_OBJ_USED	(1<<0)				// flag used in ship_obj struct
#define		MAX_SHIP_OBJS	MAX_SHIPS			// max number of ships tracked in ship list
ship_obj		Ship_objs[MAX_SHIP_OBJS];		// array used to store ship object indexes
ship_obj		Ship_obj_list;							// head of linked list of ship_obj structs

ship_info		Ship_info[MAX_SHIP_TYPES];
ship_subsys		Ship_subsystems[MAX_SHIP_SUBOBJECTS];
ship_subsys		ship_subsys_free_list;
reinforcements	Reinforcements[MAX_REINFORCEMENTS];

int Num_player_ship_precedence;				// Number of ship types in Player_ship_precedence
int Player_ship_precedence[MAX_PLAYER_SHIP_CHOICES];	// Array of ship types, precedence list for player ship/wing selection

static int Laser_energy_out_snd_timer;	// timer so we play out of laser sound effect periodically
static int Missile_out_snd_timer;	// timer so we play out of laser sound effect periodically

// structure used to hold ship counts of particular types.  The order in which these appear is crucial
// since the goal code relies on this placement to find the array index in the Ship_counts array
char *Ship_type_names[MAX_SHIP_TYPE_COUNTS] = {
//XSTR:OFF
	"no type",
	"cargo",
	"fighter/bomber",
	"cruiser",
	"freighter",
	"capital",
	"transport",
	"support",
	"navbuoy",
	"sentry gun",
	"escape pod",
	"super cap",
	"stealth",
	"fighter",
	"bomber",
	"drydock",
	"awacs",
	"gas miner",
	"corvette",
	"knossos device"
//XSTR:ON
};

int Ship_type_flags[MAX_SHIP_TYPE_COUNTS] = {
	0,
	SIF_CARGO,
	SIF_FIGHTER | SIF_BOMBER,
	SIF_CRUISER,
	SIF_FREIGHTER,
	SIF_CAPITAL,
	SIF_TRANSPORT,
	SIF_SUPPORT,
	SIF_NAVBUOY,
	SIF_SENTRYGUN,
	SIF_ESCAPEPOD,
	SIF_SUPERCAP,
	SIF_SHIP_CLASS_STEALTH,
	SIF_FIGHTER,
	SIF_BOMBER,
	SIF_DRYDOCK,
	SIF_AWACS,
	SIF_GAS_MINER,
	SIF_CORVETTE,
	SIF_KNOSSOS_DEVICE,
};

ship_counts Ship_counts[MAX_SHIP_TYPE_COUNTS];

// I don't want to do an AI cargo check every frame, so I made a global timer to limit check to
// every SHIP_CARGO_CHECK_INTERVAL ms.  Didn't want to make a timer in each ship struct.  Ensure
// inited to 1 at mission start.
static int Ship_cargo_check_timer;


// Stuff for showing ship thrusters. 
typedef struct thrust_anim {
	int	num_frames;
	int	first_frame;
	int secondary;
	int tertiary;
	float time;				// in seconds
} thrust_anim;



#if defined(MORE_SPECIES)
// ----------------------------------------------------------------------------
// New species_defs.tbl based code
// 10/15/2003, Kazan
// ----------------------------------------------------------------------------

char	Thrust_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
char	Thrust_secondary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
char	Thrust_tertiary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
char	Thrust_glow_anim_names[NUM_THRUST_GLOW_ANIMS][MAX_FILENAME_LEN];

static thrust_anim	Thrust_anims[NUM_THRUST_ANIMS];
static thrust_anim	Thrust_glow_anims[NUM_THRUST_GLOW_ANIMS];

#else

// ----------------------------------------------------------------------------
// Old volition hardcode
// 10/15/2003, Kazan
// ----------------------------------------------------------------------------

// These are indexed by:  Species*2 + (After_burner_on?1:0)
static thrust_anim	Thrust_anims[NUM_THRUST_ANIMS];
char	Thrust_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN] = {	
//XSTR:OFF
	"thruster01", "thruster01a", 
	"thruster02", "thruster02a", 
	"thruster03", "thruster03a"

//XSTR:ON
};
char	Thrust_secondary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN] = {	
//XSTR:OFF
	"thruster02-01", "thruster02-01a", 
	"thruster02-02", "thruster02-02a", 
	"thruster02-03", "thruster02-03a" 

//XSTR:ON
};
char	Thrust_tertiary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN] = {	
//XSTR:OFF
	"thruster03-01", "thruster03-01a", 
	"thruster03-02", "thruster03-02a", 
	"thruster03-03", "thruster03-03a" 

//XSTR:ON
};

// These are indexed by:  Species*2 + (After_burner_on?1:0)
static thrust_anim	Thrust_glow_anims[NUM_THRUST_GLOW_ANIMS];
char	Thrust_glow_anim_names[NUM_THRUST_GLOW_ANIMS][MAX_FILENAME_LEN] = {	
//XSTR:OFF
	"thrusterglow01", "thrusterglow01a", 
	"thrusterglow02", "thrusterglow02a", 
	"thrusterglow03", "thrusterglow03a" 
//XSTR:ON
};
#endif

static int Thrust_anim_inited = 0;

// a global definition of the IFF colors
color IFF_colors[MAX_IFF_COLORS][2];	// AL 1-2-97: Create two IFF colors, regular and bright

void ship_iff_init_colors()
{
	int i, alpha;
	int iff_bright_delta=4;

	// init IFF colors
	for ( i=0; i<2; i++ ) {

		if ( i == 0 )
			alpha = (HUD_COLOR_ALPHA_MAX - iff_bright_delta) * 16;
		else 
			alpha = HUD_COLOR_ALPHA_MAX * 16;

		gr_init_alphacolor( &IFF_colors[IFF_COLOR_HOSTILE][i],	0xff, 0x00, 0x00, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_FRIENDLY][i],	0x00, 0xff, 0x00, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_NEUTRAL][i],	0xff, 0x00, 0x00, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_UNKNOWN][i],	0xff, 0x00, 0xff, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_SELECTION][i], 0xff, 0xff, 0xff, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_MESSAGE][i],	0x7f, 0x7f, 0x7f, alpha );
		gr_init_alphacolor( &IFF_colors[IFF_COLOR_TAGGED][i],		0xff, 0xff, 0x00, alpha );
	}
}

// set the ship_obj struct fields to default values
void ship_obj_list_reset_slot(int index)
{
	Ship_objs[index].flags = 0;
	Ship_objs[index].next = NULL;
	Ship_objs[index].prev = (ship_obj*)-1;
}

// if the given ship is in alpha/beta/gamma/zeta wings
int ship_in_abgz(ship *shipp)
{
	if(!strcmp(shipp->ship_name, "Alpha 1")) return 1;
	if(!strcmp(shipp->ship_name, "Alpha 2")) return 1;
	if(!strcmp(shipp->ship_name, "Alpha 3")) return 1;
	if(!strcmp(shipp->ship_name, "Alpha 4")) return 1;

	if(!strcmp(shipp->ship_name, "Beta 1")) return 1;
	if(!strcmp(shipp->ship_name, "Beta 2")) return 1;
	if(!strcmp(shipp->ship_name, "Beta 3")) return 1;
	if(!strcmp(shipp->ship_name, "Beta 4")) return 1;

	if(!strcmp(shipp->ship_name, "Gamma 1")) return 1;
	if(!strcmp(shipp->ship_name, "Gamma 2")) return 1;
	if(!strcmp(shipp->ship_name, "Gamma 3")) return 1;
	if(!strcmp(shipp->ship_name, "Gamma 4")) return 1;

	if(!strcmp(shipp->ship_name, "Zeta 1")) return 1;
	if(!strcmp(shipp->ship_name, "Zeta 2")) return 1;
	if(!strcmp(shipp->ship_name, "Zeta 3")) return 1;
	if(!strcmp(shipp->ship_name, "Zeta 4")) return 1;

	// added by Goober5000 in case we change this in the future,
	// and because Zeta wing might be a 6-fighter single-player wing
	if(!strcmp(shipp->ship_name, "Alpha 5")) return 1;
	if(!strcmp(shipp->ship_name, "Alpha 6")) return 1;
	if(!strcmp(shipp->ship_name, "Beta 5")) return 1;
	if(!strcmp(shipp->ship_name, "Beta 6")) return 1;
	if(!strcmp(shipp->ship_name, "Gamma 5")) return 1;
	if(!strcmp(shipp->ship_name, "Gamma 6")) return 1;
	if(!strcmp(shipp->ship_name, "Zeta 5")) return 1;
	if(!strcmp(shipp->ship_name, "Zeta 6")) return 1;

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
	list_remove(&Ship_obj_list, &Ship_objs[index]);	
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

// parse an engine wash info record
void parse_engine_wash()
{
	engine_wash_info *ewp;
	ewp = &Engine_wash_info[Num_engine_wash_types];

	// name of engine wash info
	required_string("$Name:");
	stuff_string(ewp->name, F_NAME, NULL);

	// half angle of cone of wash from thruster
	required_string("$Angle:");
	stuff_float(&ewp->angle);
	ewp->angle *= (PI / 180.0f);

	// radius multiplier for hemisphere around thruster pt
	required_string("$Radius Mult:");
	stuff_float(&ewp->radius_mult);

	// length of cone
	required_string("$Length:");
	stuff_float(&ewp->length);

	// intensity inside hemisphere (or at 0 distance from frustated cone)
	required_string("$Intensity:");
	stuff_float(&ewp->intensity);
}


// Kazan -- Volition had this set to 1500, Set it to 4K for WC Saga
//#define SHIP_MULTITEXT_LENGTH 1500
#define SHIP_MULTITEXT_LENGTH 4096


// function to parse the information for a specific ship type.	
int parse_ship()
{
	char buf[SHIP_MULTITEXT_LENGTH + 1];
	ship_info *sip;
	int cont_flag = 1;
	float	hull_percentage_of_hits = 100.0f;
	int n_subsystems = 0;
	model_subsystem subsystems[MAX_MODEL_SUBSYSTEMS];		// see model.h for max_model_subsystems
	for (int idx=0; idx<MAX_MODEL_SUBSYSTEMS; idx++) {
		subsystems[idx].stepped_rotation = NULL;
//		subsystems[idx].ai_rotation = NULL;
	}
	int i, j, num_allowed, rtn = 0;
	int allowed_weapons[MAX_WEAPON_TYPES];
	int pbank_capacity_specified, pbank_capacity_count, sbank_capacity_count;

	sip = &Ship_info[Num_ship_types];

	//	Defaults!
	//	These should be specified in ships.tbl eventually!
	//	End of defaults.

	required_string("$Name:");
	stuff_string(sip->name, F_NAME, NULL);

	strcpy(parse_error_text, "\nin ship: ");
	strcat(parse_error_text, sip->name);
	// AL 28-3-98: If this is a demo build, we only want to parse weapons that are preceded with
	//             the '@' symbol
#ifdef DEMO // not needed FS2_DEMO (using separate table file)
	if ( sip->name[0] != '@' ) {
		// advance to next weapon, and return -1
		if ( skip_to_start_of_strings("$Name:", "#End") != 1 ) {
			Int3();
		}
		return -1;
	}
#endif

#ifdef NDEBUG
	if (get_pointer_to_first_hash_symbol(sip->name) && Fred_running)
		rtn = 1;
#endif

	if ( sip->name[0] == '@' ) {
		char old_name[NAME_LENGTH];
		strcpy(old_name, sip->name);
		strcpy(sip->name, old_name+1);
	}

	diag_printf ("Ship name -- %s\n", sip->name);
	if ( ship_info_lookup( sip->name ) != -1 ){
		Error(LOCATION, "Error:  Ship name %s already exists in ships.tbl.  All ship class names must be unique.", sip->name);
	}

	required_string("$Short name:");
	stuff_string(sip->short_name, F_NAME, NULL);
	diag_printf ("Ship short name -- %s\n", sip->short_name);

#if defined(MORE_SPECIES)

	// static alias stuff
	static char *tspecies_names[MAX_SPECIES_NAMES] = { Species_names[0], Species_names[1], Species_names[2], Species_names[3],
													   Species_names[4], Species_names[5], Species_names[6], Species_names[7] };

	find_and_stuff("$Species:", &sip->species, F_NAME, tspecies_names, MAX_SPECIES_NAMES, "species names");
#else
	find_and_stuff("$Species:", &sip->species, F_NAME, Species_names, MAX_SPECIES_NAMES, "species names");
#endif
	diag_printf ("Ship species -- %s\n", Species_names[sip->species]);

	sip->type_str = sip->maneuverability_str = sip->armor_str = sip->manufacturer_str = NULL;
	if (optional_string("+Type:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->type_str = strdup(buf);
	}

	if (optional_string("+Maneuverability:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->maneuverability_str = strdup(buf);
	}

	if (optional_string("+Armor:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->armor_str = strdup(buf);
	}

	if (optional_string("+Manufacturer:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->manufacturer_str = strdup(buf);
	}

	sip->desc = NULL;
	if (optional_string("+Description:")) {
		stuff_string(buf, F_MULTITEXT, NULL);
		sip->desc = strdup(buf);
	}

	sip->tech_desc = NULL;
	if (optional_string("+Tech Description:")) {
		stuff_string(buf, F_MULTITEXT, NULL, SHIP_MULTITEXT_LENGTH);
		sip->tech_desc = strdup(buf);
	}

	// Code added here by SS to parse the optional strings for length, gun_mounts, missile_banks

	sip->ship_length = NULL;
	if (optional_string("+Length:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->ship_length = strdup(buf);
	}
	
	sip->gun_mounts = NULL;
	if (optional_string("+Gun Mounts:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->gun_mounts = strdup(buf);
	}
	
	sip->missile_banks = NULL;
	if (optional_string("+Missile Banks:")) {
		stuff_string(buf, F_MESSAGE, NULL);
		sip->missile_banks = strdup(buf);
	}


	// End code by SS

	sip->num_detail_levels = 0;

	required_string( "$POF file:" );
	stuff_string( sip->pof_file, F_NAME, NULL );

	// optional hud targeting model
	strcpy(sip->pof_file_hud, "");
	if(optional_string( "$POF target file:")){
		stuff_string(sip->pof_file_hud, F_NAME, NULL);
	}

	required_string("$Detail distance:");
	sip->num_detail_levels = stuff_int_list(sip->detail_distance, MAX_SHIP_DETAIL_LEVELS, RAW_INTEGER_TYPE);

	// check for optional pixel colors
	sip->num_nondark_colors = 0;
	while(optional_string("$ND:")){		
		ubyte nr, ng, nb;
		stuff_byte(&nr);
		stuff_byte(&ng);
		stuff_byte(&nb);

		if(sip->num_nondark_colors < MAX_NONDARK_COLORS){
			sip->nondark_colors[sip->num_nondark_colors][0] = nr;
			sip->nondark_colors[sip->num_nondark_colors][1] = ng;
			sip->nondark_colors[sip->num_nondark_colors++][2] = nb;
		}
	}

	required_string("$Show damage:");
	int bogus_bool;
	stuff_boolean(&bogus_bool);

	required_string("$Density:");
	stuff_float( &(sip->density) );
	diag_printf ("Ship density -- %7.3f\n", sip->density);

	required_string("$Damp:");
	stuff_float( &(sip->damp) );
	diag_printf ("Ship damp -- %7.3f\n", sip->damp);

	required_string("$Rotdamp:");
	stuff_float( &(sip->rotdamp) );
	diag_printf ("Ship rotdamp -- %7.3f\n", sip->rotdamp);

	required_string("$Max Velocity:");
	stuff_vector(&sip->max_vel);

	// calculate the max speed from max_velocity
	sip->max_speed = vm_vec_mag(&sip->max_vel);

	required_string("$Rotation Time:");
	stuff_vector(&sip->rotation_time);

	sip->srotation_time = (sip->rotation_time.xyz.x + sip->rotation_time.xyz.y)/2.0f;

	sip->max_rotvel.xyz.x = (2 * PI) / sip->rotation_time.xyz.x;
	sip->max_rotvel.xyz.y = (2 * PI) / sip->rotation_time.xyz.y;
	sip->max_rotvel.xyz.z = (2 * PI) / sip->rotation_time.xyz.z;

	// get the backwards velocity;
	required_string("$Rear Velocity:");
	stuff_float(&sip->max_rear_vel);

	// get the accelerations
	required_string("$Forward accel:");
	stuff_float(&sip->forward_accel );

	required_string("$Forward decel:");
	stuff_float(&sip->forward_decel );

	required_string("$Slide accel:");
	stuff_float(&sip->slide_accel );

	required_string("$Slide decel:");
	stuff_float(&sip->slide_decel );

	// get ship explosion info
	required_string("$Expl inner rad:");
	stuff_float(&sip->inner_rad);

	required_string("$Expl outer rad:");
	stuff_float(&sip->outer_rad);

	required_string("$Expl damage:");
	stuff_float(&sip->damage);

	required_string("$Expl blast:");
	stuff_float(&sip->blast);

	required_string("$Expl Propagates:");
	stuff_boolean(&sip->explosion_propagates);

	required_string("$Shockwave Speed:");
	stuff_float( &sip->shockwave_speed );

	sip->shockwave_count = 1;
	if(optional_string("$Shockwave Count:")){
		stuff_int(&sip->shockwave_count);
	}

	sip->shockwave_moddel = -1;
	strcpy(sip->shockwave_pof_file, "");
	if(optional_string("$Shockwave_model:")){
		stuff_string( sip->shockwave_pof_file, F_NAME, NULL);
	}

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

char temp_error[64];
strcpy(temp_error, parse_error_text);

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
			if (bank > UPPER_BOUND_SUPPORTED_PRIMARY_BANK)
			{
				Warning(LOCATION, "$Allowed PBanks bank-specific loadout exceeds permissible number of primary banks.  Ignoring the rest...");
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
			if (bank > UPPER_BOUND_SUPPORTED_PRIMARY_BANK)
			{
				Warning(LOCATION, "$Allowed Dogfight PBanks bank-specific loadout exceeds permissible number of primary banks.  Ignoring the rest...");
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
	for ( i = 0; i < MAX_PRIMARY_BANKS; i++ )
	{
		sip->primary_bank_weapons[i] = -1;
	}

	required_string("$Default PBanks:");
strcat(parse_error_text,"'s default primary banks");
	sip->num_primary_banks = stuff_int_list(sip->primary_bank_weapons, MAX_PRIMARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

	// error checking
	for ( i = 0; i < sip->num_primary_banks; i++ )
	{
		Assert(sip->primary_bank_weapons[i] >= 0);
	}

	// set primary capacities to zero first - in case of bugs :) 
	for (i = 0; i < MAX_PRIMARY_BANKS; i++)
	{
		sip->primary_bank_ammo_capacity[i] = 0;
	}

	// optional ballistic primary imformation (Goober5000)......
	pbank_capacity_specified = 0;
	if(optional_string("$PBank Capacity:"))
	{
		pbank_capacity_specified = 1;
		// get the capacity of each primary bank
strcat(parse_error_text,"'s default primary banks' ammo");
		pbank_capacity_count = stuff_int_list(sip->primary_bank_ammo_capacity, MAX_PRIMARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
		if ( pbank_capacity_count != sip->num_primary_banks )
		{
			Warning(LOCATION, "Primary bank capacities have not been completely specified for ship class %s... fix this!!", sip->name);
		}
	}

	// Set the weapons filter used in weapons loadout (for secondary weapons)
	if (optional_string("$Allowed SBanks:"))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank > UPPER_BOUND_SUPPORTED_SECONDARY_BANK)
			{
				Warning(LOCATION, "$Allowed SBanks bank-specific loadout exceeds permissible number of secondary banks.  Ignoring the rest...");
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
					sip->allowed_bank_restricted_weapons[MAX_SUPPORTED_PRIMARY_BANKS+bank][allowed_weapons[i]] |= REGULAR_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[MAX_SUPPORTED_PRIMARY_BANKS+i] |= REGULAR_WEAPON;
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
			if (bank > UPPER_BOUND_SUPPORTED_SECONDARY_BANK)
			{
				Warning(LOCATION, "$Allowed Dogfight SBanks bank-specific loadout exceeds permissible number of secondary banks.  Ignoring the rest...");
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
					sip->allowed_bank_restricted_weapons[MAX_SUPPORTED_PRIMARY_BANKS+bank][allowed_weapons[i]] |= DOGFIGHT_WEAPON;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[MAX_SUPPORTED_PRIMARY_BANKS+i] |= DOGFIGHT_WEAPON;
			}
		}
	}

	// Get default secondary bank weapons
	for ( i = 0; i < MAX_SECONDARY_BANKS; i++ )
	{
		sip->secondary_bank_weapons[i] = -1;
	}
	required_string("$Default SBanks:");
strcat(parse_error_text,"'s default secondary banks");
	sip->num_secondary_banks = stuff_int_list(sip->secondary_bank_weapons, MAX_SECONDARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);

	// error checking
	for ( i = 0; i < sip->num_secondary_banks; i++ )
	{
		if(sip->secondary_bank_weapons[i] < 0)
		{
			Warning(LOCATION, "%s has no secondary weapons, this cannot be!", sip->name);
		}
		// Assert(sip->secondary_bank_weapons[i] >= 0);
	}

	// Get the capacity of each secondary bank
	required_string("$SBank Capacity:");
strcat(parse_error_text,"'s secondary banks capacities");
	sbank_capacity_count = stuff_int_list(sip->secondary_bank_ammo_capacity, MAX_SECONDARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
	if ( sbank_capacity_count != sip->num_secondary_banks )
	{
		Warning(LOCATION, "Secondary bank capacities have not been completely specified for ship class %s... fix this!!", sip->name);
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

	required_string("$Shields:");
	stuff_float(&sip->initial_shield_strength);

	// optional shield color
	sip->shield_color[0] = 255;
	sip->shield_color[1] = 255;
	sip->shield_color[2] = 255;
	if(optional_string("$Shield Color:")){
		stuff_byte(&sip->shield_color[0]);
		stuff_byte(&sip->shield_color[1]);
		stuff_byte(&sip->shield_color[2]);
	}

	// The next three fields are used for the ETS
	required_string("$Power Output:");
	stuff_float(&sip->power_output);

	required_string("$Max Oclk Speed:");
	stuff_float(&sip->max_overclocked_speed);

	required_string("$Max Weapon Eng:");
	stuff_float(&sip->max_weapon_reserve);

	required_string("$Hitpoints:");
	stuff_float(&sip->initial_hull_strength);
	if (sip->initial_hull_strength == 0.0f)
	{
		Warning(LOCATION, "Initial hull strength on ship %s cannot be 0.  Defaulting to 100.\n", sip->name);
		sip->initial_hull_strength = 100.0f;
	}

	required_string("$Flags:");
	char	ship_strings[MAX_SHIP_FLAGS][NAME_LENGTH];
	int num_strings = stuff_string_list(ship_strings, MAX_SHIP_FLAGS);
	sip->flags = SIF_DEFAULT_VALUE;
	for ( i=0; i<num_strings; i++ )
	{
		if (!stricmp(NOX("no_collide"), ship_strings[i]))
			sip->flags &= ~SIF_DO_COLLISION_CHECK;
		else if (!stricmp(NOX("player_ship"), ship_strings[i]))
			sip->flags |= SIF_PLAYER_SHIP;
		else if (!stricmp(NOX("default_player_ship"), ship_strings[i]))
			sip->flags |= SIF_DEFAULT_PLAYER_SHIP;
		else if ( !stricmp(NOX("repair_rearm"), ship_strings[i]))
			sip->flags |= SIF_SUPPORT;
		else if ( !stricmp(NOX("cargo"), ship_strings[i]))
			sip->flags |= SIF_CARGO;
		else if ( !stricmp( NOX("fighter"), ship_strings[i]))
			sip->flags |= SIF_FIGHTER;
		else if ( !stricmp( NOX("bomber"), ship_strings[i]))
			sip->flags |= SIF_BOMBER;
		else if ( !stricmp( NOX("transport"), ship_strings[i]))
			sip->flags |= SIF_TRANSPORT;
		else if ( !stricmp( NOX("freighter"), ship_strings[i]))
			sip->flags |= SIF_FREIGHTER;
		else if ( !stricmp( NOX("capital"), ship_strings[i]))
			sip->flags |= SIF_CAPITAL;
		else if (!stricmp( NOX("supercap"), ship_strings[i]))
			sip->flags |= SIF_SUPERCAP;
		else if (!stricmp( NOX("drydock"), ship_strings[i]))
			sip->flags |= SIF_DRYDOCK;
		else if ( !stricmp( NOX("cruiser"), ship_strings[i]))
			sip->flags |= SIF_CRUISER;
		else if ( !stricmp( NOX("navbuoy"), ship_strings[i]))
			sip->flags |= SIF_NAVBUOY;
		else if ( !stricmp( NOX("sentrygun"), ship_strings[i]))
			sip->flags |= SIF_SENTRYGUN;
		else if ( !stricmp( NOX("escapepod"), ship_strings[i]))
			sip->flags |= SIF_ESCAPEPOD;
		else if ( !stricmp( NOX("no type"), ship_strings[i]))
			sip->flags |= SIF_NO_SHIP_TYPE;
		else if ( !stricmp( NOX("ship copy"), ship_strings[i]))
			sip->flags |= SIF_SHIP_COPY;
		else if ( !stricmp( NOX("in tech database"), ship_strings[i]))
			sip->flags |= SIF_IN_TECH_DATABASE | SIF_IN_TECH_DATABASE_M;
		else if ( !stricmp( NOX("in tech database multi"), ship_strings[i]))
			sip->flags |= SIF_IN_TECH_DATABASE_M;
		else if ( !stricmp( NOX("dont collide invisible"), ship_strings[i]))
			sip->flags |= SIF_SHIP_CLASS_DONT_COLLIDE_INVIS;
		else if ( !stricmp( NOX("big damage"), ship_strings[i]))
			sip->flags |= SIF_BIG_DAMAGE;
		else if ( !stricmp( NOX("corvette"), ship_strings[i]))
			sip->flags |= SIF_CORVETTE;
		else if ( !stricmp( NOX("gas miner"), ship_strings[i]))
			sip->flags |= SIF_GAS_MINER;
		else if ( !stricmp( NOX("awacs"), ship_strings[i]))
			sip->flags |= SIF_AWACS;
		else if ( !stricmp( NOX("knossos"), ship_strings[i]))
			sip->flags |= SIF_KNOSSOS_DEVICE;
		else if ( !stricmp( NOX("no_fred"), ship_strings[i]))
			sip->flags |= SIF_NO_FRED;
		else if ( !stricmp( NOX("ballistic primaries"), ship_strings[i]))
			sip->flags |= SIF_BALLISTIC_PRIMARIES;
		else
			Warning(LOCATION, "Bogus string in ship flags: %s\n", ship_strings[i]);
	}

	// set original status of tech database flags - Goober5000
	if (sip->flags & SIF_IN_TECH_DATABASE)
		sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE;
	if (sip->flags & SIF_IN_TECH_DATABASE_M)
		sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE_M;

	// be friendly; ensure ballistic flags check out
	if (pbank_capacity_specified)
	{
		if (!(sip->flags & SIF_BALLISTIC_PRIMARIES))
		{
			Warning(LOCATION, "Pbank capacity specified for non-ballistic-primary-enabled ship %s.\nResetting capacities to 0.\n", sip->name);

			for (i = 0; i < MAX_PRIMARY_BANKS; i++)
			{
				sip->primary_bank_ammo_capacity[i] = 0;
			}
		}
	}
	else
	{
		if (sip->flags & SIF_BALLISTIC_PRIMARIES)
		{
			Warning(LOCATION, "Pbank capacity not specified for ballistic-primary-enabled ship %s.\nDefaulting to capacity of 1 per bank.\n", sip->name);
			for (i = 0; i < MAX_PRIMARY_BANKS; i++)
			{
				sip->primary_bank_ammo_capacity[i] = 1;
			}
		}
	}

	find_and_stuff("$AI Class:", &sip->ai_class, F_NAME, Ai_class_names, Num_ai_classes, "AI class names");

	// Get Afterburner information
	// Be aware that if $Afterburner is not 1, the other Afterburner fields are not read in
	required_string("$Afterburner:");
	int has_afterburner;
	stuff_boolean(&has_afterburner);
	if ( has_afterburner == 1 ) {
		sip->flags |= SIF_AFTERBURNER;

		required_string("+Aburn Max Vel:");
		stuff_vector(&sip->afterburner_max_vel);

		required_string("+Aburn For accel:");
		stuff_float(&sip->afterburner_forward_accel);

		required_string("+Aburn Fuel:");
		stuff_float(&sip->afterburner_fuel_capacity);

		required_string("+Aburn Burn Rate:");
		stuff_float(&sip->afterburner_burn_rate);

		required_string("+Aburn Rec Rate:");
		stuff_float(&sip->afterburner_recover_rate);

		// Goober5000: check div-0
		Assert(sip->afterburner_fuel_capacity);
	} else {
	
//		mprintf(("no AB or ABtrails\n"));
		sip->afterburner_max_vel.xyz.x = 0.0f;
		sip->afterburner_max_vel.xyz.y = 0.0f;
		sip->afterburner_max_vel.xyz.z = 0.0f;

	}
	
	if(optional_string("$Trails:")){//optional values aplyed to ABtrails -Bobboau
//		mprintf(("ABtrails\n"));
		char bitmap_name[MAX_FILENAME_LEN] = "";

		required_string("+Bitmap:");
		stuff_string(bitmap_name, F_NAME, NULL);
		sip->ABbitmap = bm_load(bitmap_name);
		
		required_string("+Width:");
		stuff_float(&sip->ABwidth_factor);
			
		required_string("+Alpha:");
		stuff_float(&sip->ABAlpha_factor);
			
		required_string("+Life:");
		stuff_float(&sip->ABlife);
	}else{
//		mprintf(("no ABtrails\n"));
		sip->ABbitmap = -1;	//defalts for no ABtrails-Bobboau
		sip->ABwidth_factor = 1.0f;
		sip->ABAlpha_factor = 1.0f;
		sip->ABlife = 5.0f;
	}


	required_string("$Countermeasures:");
	stuff_int(&sip->cmeasure_max);

	required_string("$Scan time:");
	stuff_int(&sip->scan_time);

	required_string("$EngineSnd:");
	stuff_int(&sip->engine_snd);
	
	// bah: ensure the sounds are in range
	if (sip->engine_snd < -1 || sip->engine_snd >= MAX_GAME_SOUNDS)
	{
		Warning(LOCATION, "$EngineSnd sound index out of range on ship %s.  Must be between 0 and %d.  Forcing to -1.\n", sip->name, MAX_GAME_SOUNDS);
		sip->engine_snd = -1;
	}

	required_string("$Closeup_pos:");
	stuff_vector(&sip->closeup_pos);

	required_string("$Closeup_zoom:");
	stuff_float(&sip->closeup_zoom);

	sip->shield_icon_index = 255;		// stored as ubyte
	if (optional_string("$Shield_icon:")) {
		char tmpbuf[NAME_LENGTH];
		stuff_string(tmpbuf, F_NAME, NULL);
		hud_shield_assign_info(sip, tmpbuf);
	}

	// read in filename for icon that is used in ship selection
	sip->icon_filename[0] = 0;
	if ( optional_string("$Ship_icon:") ) {
		stuff_string(sip->icon_filename, F_NAME, NULL);
	}

	// read in filename for animation that is used in ship selection
	sip->anim_filename[0] = 0;
	if ( optional_string("$Ship_anim:") ) {
		stuff_string(sip->anim_filename, F_NAME, NULL);
	}

	// read in filename for animation that is used in ship selection
	sip->overhead_filename[0] = 0;
	if ( optional_string("$Ship_overhead:") ) {
		stuff_string(sip->overhead_filename, F_NAME, NULL);
	}

	sip->score = 0;
	if ( optional_string("$Score:") ){
		stuff_int( &sip->score );
	}


	sip->thruster_glow1 = -1;
	sip->thruster_glow1a = -1;
	sip->thruster_glow2 = -1;
	sip->thruster_glow2a = -1;
	sip->thruster_glow3 = -1;
	sip->thruster_glow3a = -1;
//	sip->thruster_particle_bitmap01 = -1;

//	strcpy(sip->thruster_particle_bitmap01_name,"thrusterparticle");

	strcpy(sip->splodeing_texture_name, "boom");

	i = sip->species*2;
	strcpy(sip->thruster_bitmap1, Thrust_glow_anim_names[i]);
	strcpy(sip->thruster_bitmap2, Thrust_secondary_anim_names[i]);
	strcpy(sip->thruster_bitmap3, Thrust_tertiary_anim_names[i++]);
	strcpy(sip->thruster_bitmap1a, Thrust_glow_anim_names[i]);
	strcpy(sip->thruster_bitmap2a, Thrust_secondary_anim_names[i]);
	strcpy(sip->thruster_bitmap3a, Thrust_tertiary_anim_names[i]);

	 sip->thruster01_rad_factor = 1.0f;
	 sip->thruster02_rad_factor = 1.0f;
	 sip->thruster02_len_factor = 1.0f;
	 sip->thruster03_rad_factor = 1.0f;

	if ( optional_string("$Thruster Bitmap 1:") ){
		stuff_string( sip->thruster_bitmap1, F_NAME, NULL );
	}
	if ( optional_string("$Thruster Bitmap 1a:") ){
		stuff_string( sip->thruster_bitmap1a, F_NAME, NULL );
	}
	if ( optional_string("$Thruster01 Radius factor:") ){
		stuff_float(&sip->thruster01_rad_factor);
	}

	if ( optional_string("$Thruster Bitmap 2:") ){
		stuff_string( sip->thruster_bitmap2, F_NAME, NULL );
	}
	if ( optional_string("$Thruster Bitmap 2a:") ){
		stuff_string( sip->thruster_bitmap2a, F_NAME, NULL );
	}
	if ( optional_string("$Thruster02 Radius factor:") ){
		stuff_float(&sip->thruster02_rad_factor);
	}
	if ( optional_string("$Thruster01 Length factor:") ){
		stuff_float(&sip->thruster02_len_factor);
	}

	if ( optional_string("$Thruster Bitmap 3:") ){
		stuff_string( sip->thruster_bitmap3, F_NAME, NULL );
	}
	if ( optional_string("$Thruster Bitmap 3a:") ){
		stuff_string( sip->thruster_bitmap3a, F_NAME, NULL );
	}
	if ( optional_string("$Thruster03 Radius factor:") ){
		stuff_float(&sip->thruster03_rad_factor);
	}

	sip->n_thruster_particles = 0;
	sip->n_ABthruster_particles = 0;
	while(optional_string("$Thruster Particles:")){
		thruster_particles* t;
		if ( optional_string("$Thruster Particle Bitmap:") ){
			t = &sip->normal_thruster_particles[sip->n_thruster_particles++];
		}else if ( optional_string("$Afterburner Particle Bitmap:") ){
			t = &sip->afterburner_thruster_particles[sip->n_ABthruster_particles++];
		}else{
			Error( LOCATION, "formatting error in the thruster's particle section for ship %s\n", sip->name );
			break;
		}

		stuff_string(t->thruster_particle_bitmap01_name, F_NAME, NULL );

			required_string("$Min Radius:");
		stuff_float(&t->min_rad);
			required_string("$Max Radius:");
		stuff_float(&t->max_rad);
			required_string("$Min created:");
		stuff_int(&t->n_low);
			required_string("$Max created:");
		stuff_int(&t->n_high);
			required_string("$Variance:");
		stuff_float(&t->variance);
	}

	// if the ship is a stealth ship
	if ( optional_string("$Stealth:") ){
		sip->flags |= SIF_SHIP_CLASS_STEALTH;
	}


	// parse contrail info
	char trail_name[MAX_FILENAME_LEN] = "";
	trail_info *ci;
	memset(&sip->ct_info, 0, sizeof(trail_info) * MAX_SHIP_CONTRAILS);
	sip->ct_count = 0;
	while(optional_string("$Trail:")){
		// this means you've reached the max # of contrails for a ship
		if (sip->ct_count >= MAX_SHIP_CONTRAILS)
		{
			Warning(LOCATION, "%s has more contrails than the max of %d", sip->name, MAX_SHIP_CONTRAILS);
		}

		ci = &sip->ct_info[sip->ct_count++];
		
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
		stuff_string(trail_name, F_NAME, NULL);
		ci->bitmap = bm_load(trail_name);
	}

	while (cont_flag) {
		int r = required_string_3("#End", "$Subsystem:", "$Name" );
		switch (r) {
		case 0:
			cont_flag = 0;
			break;
		case 1:
		{
			float	turning_rate;
			float	percentage_of_hits;
			model_subsystem *sp;			// to append on the ships list of subsystems

			Assert ( n_subsystems < MAX_MODEL_SUBSYSTEMS );
			sp = &subsystems[n_subsystems++];			// subsystems a local -- when done, we will malloc and copy
			required_string("$Subsystem:");
			stuff_string(sp->subobj_name, F_NAME, ",");
			Mp++;
			stuff_float(&percentage_of_hits);

	//		if (percentage_of_hits == 0.0f)
			{
	//			Warning(LOCATION, "Subsystem %s on ship %s has 0 hitpoints.\n", sp->subobj_name, sp->name);
			}

			stuff_float(&turning_rate);
			hull_percentage_of_hits -= percentage_of_hits;
			sp->max_subsys_strength = sip->initial_hull_strength * (percentage_of_hits / 100.0f);
			sp->type = SUBSYSTEM_UNKNOWN;
			// specified as how long to turn 360 degrees in ships.tbl
			if ( turning_rate > 0.0f ){
				sp->turret_turning_rate = PI2 / turning_rate;		
			} else {
				sp->turret_turning_rate = 0.0f;		
			}

			for (i=0; i<MAX_PRIMARY_BANKS; i++) {
				sp->primary_banks[i] = -1;
				sp->primary_bank_capacity[i] = 0;
			}
			for (i=0; i<MAX_SECONDARY_BANKS; i++) {
				sp->secondary_banks[i] = -1;
				sp->secondary_bank_capacity[i] = 0;
			}

			//	Get default primary bank weapons
			if (optional_string("$Default PBanks:")){
strcat(parse_error_text,"'s default primary banks");
				stuff_int_list(sp->primary_banks, MAX_PRIMARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// get capacity of each primary bank - Goober5000
			if (optional_string("$PBank Capacity:")){
strcat(parse_error_text,"'s primary banks capacities");
				stuff_int_list(sp->primary_bank_capacity, MAX_PRIMARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
			}

			//	Get default secondary bank weapons
			if (optional_string("$Default SBanks:")){
strcat(parse_error_text,"'s default secondary banks");
				stuff_int_list(sp->secondary_banks, MAX_SECONDARY_BANKS, WEAPON_LIST_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// Get the capacity of each secondary bank
			if (optional_string("$SBank Capacity:")){
strcat(parse_error_text,"'s secondary banks capacities");
				stuff_int_list(sp->secondary_bank_capacity, MAX_SECONDARY_BANKS, RAW_INTEGER_TYPE);
strcpy(parse_error_text, temp_error);
			}

			// Get optional engine wake info
			if (optional_string("$Engine Wash:")) {
				char engine_wash_name[32];
				stuff_string(engine_wash_name, F_NAME, NULL);
				// get and set index
				sp->engine_wash_index = get_engine_wash_index(engine_wash_name);
			} else {
				sp->engine_wash_index = -1;
			}
				
			// Get any AWACS info
			sp->awacs_intensity = 0.0f;
			if(optional_string("$AWACS:")){
				stuff_float(&sp->awacs_intensity);
				stuff_float(&sp->awacs_radius);
				sip->flags |= SIF_HAS_AWACS;
			}

			sp->turret_weapon_type = sp->primary_banks[0];  // temporary, will be obsolete later.
			if ( sp->turret_weapon_type < 0 ) {
				sp->turret_weapon_type = sp->secondary_banks[0];
			}
			sp->model_num = -1;		// init value for later sanity checking!!
		}
		break;
		case 2:
			cont_flag = 0;
			break;
		default:
			Int3();	// Impossible return value from required_string_3.
		}
	}	

	// must be > 0//no it doesn't :P -Bobboau
	// yes it does! - Goober5000
	// (we don't want a div-0 error)
	if (hull_percentage_of_hits < 0.0f )
	{
		Warning(LOCATION, "The subsystems defined for the %s can take more combined damage than the ship itself. Adjust the tables so that the percentages add up to less than 100", sip->name);
	}
	// when done reading subsystems, malloc and copy the subsystem data to the ship info structure
	sip->n_subsystems = n_subsystems;
	if ( n_subsystems > 0 ) {
		sip->subsystems = (model_subsystem *)malloc(sizeof(model_subsystem) * n_subsystems );
		Assert( sip->subsystems != NULL );
	}
	else {
		sip->subsystems = NULL;
	}
		
	for ( i = 0; i < n_subsystems; i++ ){
		sip->subsystems[i] = subsystems[i];
	}

	// if we have a ship copy, then check to be sure that our base ship exists
	if ( sip->flags & SIF_SHIP_COPY ) {
		int index;

		index = ship_info_base_lookup( Num_ship_types );		// Num_ship_types is our current entry into the array
		if ( index == -1 ) {
			char name[NAME_LENGTH];

			strcpy( name, sip->name );
			end_string_at_first_hash_symbol(name);
			Error(LOCATION, "Ship %s is a copy, but base ship %s couldn't be found.", sip->name, name);
		}
	}

	strcpy(parse_error_text, "");

	return rtn;
}

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
}

void parse_shiptbl()
{
	// open localization
	lcl_ext_open();
	
	read_file_text("ships.tbl");
	reset_parse();

	// parse default ship
	required_string("#Default Player Ship");
	required_string("$Name:");
	stuff_string(default_player_ship, F_NAME, NULL, 254);
	required_string("#End");

	Num_engine_wash_types = 0;
	Num_ship_types = 0;

	required_string("#Engine Wash Info");
	while (required_string_either("#End", "$Name:")) {
		Assert( Num_engine_wash_types < MAX_ENGINE_WASH_TYPES );

		parse_engine_wash();
		Num_engine_wash_types++;
	}

	required_string("#End");
	required_string("#Ship Classes");

	while (required_string_either("#End","$Name:")) {
		Assert( Num_ship_types <= MAX_SHIP_TYPES );	// Goober5000 - should be <=

		if ( parse_ship() ) {
			continue;
		}

		Num_ship_types++;
	}

	required_string("#End");

	// Read in a list of ship_info indicies that are an ordering of the player ship precedence.
	// This list is used to select an alternate ship when a particular ship is not available
	// during ship selection.
	required_string("$Player Ship Precedence:");
strcpy(parse_error_text,"'player ship precedence");
	Num_player_ship_precedence = stuff_int_list(Player_ship_precedence, MAX_PLAYER_SHIP_CHOICES, SHIP_INFO_TYPE);
strcpy(parse_error_text, "");

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

	if ( !ships_inited ) {
		
		if ((rval = setjmp(parse_abort)) != 0) {
			Error(LOCATION, "Error parsing 'ships.tbl'\r\nError code = %i.\r\n", rval);
		} else {			
			parse_shiptbl();
			ships_inited = 1;
		}

		ship_iff_init_colors();
	}

	//loadup the cloaking map
//	CLOAKMAP = bm_load_animation("cloakmap",&CLOAKFRAMES, &CLOAKFPS,1);
	if (CLOAKMAP == -1)
	{
		CLOAKMAP = bm_load("cloakmap");
	}

	ship_level_init();	// needed for FRED
}


// This will get called at the start of each level.
void ship_level_init()
{
	int i;

	// Reset everything between levels


	// Mark all the models as invalid, since all the models get paged out 
	// between levels.
	for (i=0; i<MAX_SHIP_TYPES; i++ )	{
		Ship_info[i].modelnum = -1;
		Ship_info[i].modelnum_hud = -1;
	}

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

	num_wings = 0;
	for (i = 0; i < MAX_WINGS; i++ )
	{
		Wings[i].num_waves = -1;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;	// Goober5000 - default to no wing insignia
												// don't worry about releasing textures because
												// they are released automatically when the model
												// is unloaded (because they are part of the model)
	}

	for (i=0; i<MAX_PLAYER_WINGS; i++)
		Starting_wings[i] = -1;

	// Empty the subsys list
	memset( Ship_subsystems, 0, sizeof(ship_subsys)*MAX_SHIP_SUBOBJECTS );

	list_init( &ship_subsys_free_list );
	for ( i = 0; i < MAX_SHIP_SUBOBJECTS; i++ )
		list_append( &ship_subsys_free_list, &Ship_subsystems[i] );

	Laser_energy_out_snd_timer = 1;
	Missile_out_snd_timer		= 1;

	for (i = 0; i < MAX_SHIP_TYPE_COUNTS; i++ ) {
		Ship_counts[i].total = 0;
		Ship_counts[i].killed = 0;
	}

	ship_obj_list_init();

	Ship_cargo_check_timer = 1;

	shipfx_large_blowup_level_init();
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
		for ( i = 1; i < MAX_SHIPS; i++ ) {
			if ( Ships_exited[i].time < Ships_exited[oldest_entry].time ) {
				oldest_entry = i;
			}
		}
		entry = oldest_entry;
	} else {
		entry = Num_exited_ships;
		Num_exited_ships++;
	}

	strcpy( Ships_exited[entry].ship_name, sp->ship_name );
	Ships_exited[entry].obj_signature = Objects[sp->objnum].signature;
	Ships_exited[entry].team = sp->team;
	Ships_exited[entry].flags = reason;
	// if ship is red alert, flag as such
	if (sp->flags & SF_RED_ALERT_STORE_STATUS) {
		Ships_exited[entry].flags |= SEF_RED_ALERT_CARRY;
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
	polymodel *pm = model_get( Ships[objp->instance].modelnum );

	// use mass and I_body_inv from POF read into polymodel
	physics_init(pi);
	pi->mass = pm->mass * sinfo->density;

	if (pi->mass==0.0f)
	{
		vector size;
		vm_vec_sub(&size,&pm->maxs,&pm->mins);
		float vmass=size.xyz.x*size.xyz.y*size.xyz.z;
		float amass=4.65f*(float)pow(vmass,(2.0f/3.0f));

		nprintf(("Physics", "pi->mass==0.0f. setting to %f",amass));
		Warning(LOCATION, "%s (%s) has no mass! setting to %f", sinfo->name, sinfo->pof_file, amass);
		pm->mass=amass;
		pi->mass=amass*sinfo->density;
	}

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

	vm_vec_zero(&pi->vel);
	vm_vec_zero(&pi->rotvel);
	pi->speed = 0.0f;
	pi->heading = 0.0f;
//	pi->accel = 0.0f;
	vm_set_identity(&pi->last_rotmat);
}

// function to set the orders allowed for a ship -- based on ship type.  This value might get overridden
// by a value in the mission file.
int ship_get_default_orders_accepted( ship_info *sip )
{
	int ship_info_flag;

	ship_info_flag = sip->flags;

	if ( ship_info_flag & SIF_FIGHTER )
		return FIGHTER_MESSAGES;
	else if ( ship_info_flag & SIF_BOMBER )
		return BOMBER_MESSAGES;
	else if ( ship_info_flag & (SIF_CRUISER | SIF_GAS_MINER | SIF_AWACS | SIF_CORVETTE) )
		return CRUISER_MESSAGES;
	else if ( ship_info_flag & SIF_FREIGHTER )
		return FREIGHTER_MESSAGES;
	else if ( ship_info_flag & SIF_CAPITAL )
		return CAPITAL_MESSAGES;
	else if ( ship_info_flag & SIF_SUPERCAP )
		return SUPERCAP_MESSAGES;
	else if ( ship_info_flag & SIF_TRANSPORT )
		return TRANSPORT_MESSAGES;
	else if ( ship_info_flag & SIF_SUPPORT )
		return SUPPORT_MESSAGES;
	else
		return 0;
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
	shipp->dock_objnum_when_dead = -1;
	shipp->large_ship_blowup_index = -1;
	shipp->respawn_priority = 0;
	for (i=0; i<NUM_SUB_EXPL_HANDLES; i++) {
		shipp->sub_expl_sound_handle[i] = -1;
	}

	if ( !Fred_running ) {
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
	shipp->team = TEAM_FRIENDLY;					//	Default friendly, probably get overridden.
	shipp->arrival_location = 0;
	shipp->arrival_distance = 0;
	shipp->arrival_anchor = -1;
	shipp->arrival_delay = 0;
	shipp->arrival_cue = -1;
	shipp->departure_location = 0;
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
		shipp->ship_initial_hull_strength = 100.0f;
	} else {
		shipp->ship_initial_hull_strength = sip->initial_hull_strength;
	}
	objp->hull_strength = shipp->ship_initial_hull_strength;
	
	shipp->afterburner_fuel = sip->afterburner_fuel_capacity;

	shipp->cmeasure_count = sip->cmeasure_max;

	for ( i = 0; i < MAX_PRIMARY_BANKS; i++ )
	{
		swp->primary_bank_ammo[i] = 0;						// added by Goober5000
		swp->next_primary_fire_stamp[i] = timestamp(0);	
		swp->primary_bank_rearm_time[i] = timestamp(0);		// added by Goober5000
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
	for ( i = 0; i < MAX_SECONDARY_BANKS; i++ )
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

	shipp->current_cmeasure = 0;

	ets_init_ship(objp);	// init ship fields that are used for the ETS

	physics_ship_init(objp);
	if (Fred_running) {
		shipp->ship_initial_shield_strength = 100.0f;
		shipp->ship_initial_hull_strength = 100.0f;
		objp->shield_quadrant[0] = 100.0f;
	} else {
		shipp->ship_initial_shield_strength = sip->initial_shield_strength;
		shipp->ship_initial_hull_strength = sip->initial_hull_strength;
		set_shield_strength(objp, shipp->ship_initial_shield_strength);
	}

	shipp->target_shields_delta = 0.0f;
	shipp->target_weapon_energy_delta = 0.0f;

	ai_object_init(objp, shipp->ai_index);
	shipp->weapons.ai_class = Ai_info[shipp->ai_index].ai_class;
	shipp->shield_integrity = NULL;
//	shipp->sw.blast_duration = -1;	// init shockwave struct
	subsys_set(objnum);
	shipp->orders_accepted = ship_get_default_orders_accepted( sip );
	shipp->num_swarm_missiles_to_fire = 0;	
	shipp->num_turret_swarm_info = 0;
	shipp->death_roll_snd = -1;
	shipp->thruster_bitmap = -1;
	shipp->secondary_thruster_bitmap = -1;
	shipp->tertiary_thruster_bitmap = -1;
	shipp->thruster_frame = 0.0f;
	shipp->thruster_glow_bitmap = -1;
	shipp->thruster_glow_noise = 1.0f;
	shipp->thruster_glow_frame = 0.0f;
	shipp->next_engine_stutter = 1;
	shipp->persona_index = -1;
	shipp->flags |= SF_ENGINES_ON;
	shipp->subsys_disrupted_flags=0;
	shipp->subsys_disrupted_check_timestamp=timestamp(0);

	// swarm missile stuff
	shipp->next_swarm_fire = 1;

	// corkscrew missile stuff
	shipp->next_corkscrew_fire = 1;

	// field for score
	shipp->score = sip->score;

	// tag
	shipp->tag_left = -1.0f;
	shipp->level2_tag_left = -1.0f;

#ifndef NO_NETWORK
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
#endif

	shipp->primitive_sensor_range = DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE;

	shipp->special_warp_objnum = -1;

	// set awacs warning flags so awacs ship only asks for help once at each level
	shipp->awacs_warning_flag = AWACS_WARN_NONE;

	// initialize revised texture replacements - Goober5000
	shipp->replacement_textures = NULL;
	for (i=0; i<MAX_MODEL_TEXTURES; i++)
	{
		shipp->replacement_textures_buf[i] = -1;
	}

//	for(i=0; i<MAX_SHIP_DECALS; i++)
//		shipp->decals[i].is_valid = 0;

	
//	for(i = 1; i < MAX_SHIP_DECALS; i++){
//		shipp->decals[i].timestamp = timestamp();
//		shipp->decals[i].is_valid = 0;
//	}
	for(i = 0; i<32; i++){
		(shipp->glows_active |= (1 << i));
	}
	shipp->glowmaps_active = 1;

	shipp->cloak_stage = 0;
	shipp->texture_translation_key=vmd_zero_vector;
	shipp->current_translation=vmd_zero_vector;
	shipp->time_until_full_cloak=timestamp(0);
	shipp->cloak_alpha=255;
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
	}

	// set any ship flags which should be set.  unset the flags since we might be repairing a subsystem
	// through sexpressions.
	shipp->flags &= ~SF_DISABLED;
	if ( (shipp->subsys_info[SUBSYSTEM_ENGINE].num > 0) && (shipp->subsys_info[SUBSYSTEM_ENGINE].current_hits == 0.0f) ){
		shipp->flags |= SF_DISABLED;
	}

	/*
	shipp->flags &= ~SF_DISARMED;
	if ( (shipp->subsys_info[SUBSYSTEM_TURRET].num > 0) && (shipp->subsys_info[SUBSYSTEM_TURRET].current_hits == 0.0f) ){
		shipp->flags |= SF_DISARMED;
	}
	*/
}

// routine to possibly fixup the model subsystem information for this ship pointer.  Needed when
// ships share the same model.
void ship_copy_subsystem_fixup(ship_info *sip)
{
	int i, model_num;

	model_num = sip->modelnum;

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
		for ( i = 0; i < Num_ship_types; i++ ) {
			model_subsystem *msp;

			if ( (Ship_info[i].modelnum != model_num) || (&Ship_info[i] == sip) ){
				continue;
			}

			// see if this ship has subsystems and a model for the subsystems.  We only need check the first
			// subsystem since previous error checking would have trapped it's loading as an error.
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
	model_subsystem *sp;
	ship_subsys *ship_system;
	int i, j, k;

	// set up the subsystems for this ship.  walk through list of subsystems in the ship-info array.
	// for each subsystem, get a new ship_subsys instance and set up the pointers and other values
	list_init ( &shipp->subsys_list );								// initialize the ship's list of subsystems

	for ( i = 0; i < sinfo->n_subsystems; i++ )
	{
		// if duplicate, use the duplicate info
		if (shipp->alt_modelnum != -1)
		{
			sp = &(shipp->subsystems[i]);
		}
		else
		{
			sp = &(sinfo->subsystems[i]);
		}

		if ( sp->model_num == -1 ) {
			Warning (LOCATION, "Invalid subobj_num or model_num in subsystem %s on ship type %s.\nNot linking into ship!\n\n(This warning means that a subsystem was present in ships.tbl and not present in the model\nit should probably be removed from the table or added to the model.)\n", sp->subobj_name, sinfo->name );
			continue;
		}

		// set up the linked list
		ship_system = GET_FIRST( &ship_subsys_free_list );		// get a new element from the ship_subsystem array
		Assert ( ship_system != &ship_subsys_free_list );		// shouldn't have the dummy element
		list_remove( &ship_subsys_free_list, ship_system );	// remove the element from the array
		list_append( &shipp->subsys_list, ship_system );		// link the element into the ship

		ship_system->system_info = sp;						// set the system_info pointer to point to the data read in from the model

		ship_system->max_hits = sp->max_subsys_strength * shipp->ship_initial_hull_strength / sinfo->initial_hull_strength;

		if ( !Fred_running ){
			ship_system->current_hits = ship_system->max_hits;		// set the current hits
		} else {
			ship_system->current_hits = 0.0f;				// Jason wants this to be 0 in Fred.
		}
		ship_system->turret_next_fire_stamp = timestamp(0);
		ship_system->turret_next_enemy_check_stamp = timestamp(0);
		ship_system->turret_enemy_objnum = -1;
		ship_system->turret_next_fire_stamp = timestamp((int) frand_range(1.0f, 500.0f));	// next time this turret can fire
		ship_system->turret_last_fire_direction = sp->turret_norm;
		ship_system->turret_next_fire_pos = 0;
		ship_system->turret_time_enemy_in_range = 0.0f;
		ship_system->disruption_timestamp=timestamp(0);
		ship_system->turret_pick_big_attack_point_timestamp = timestamp(0);
		vm_vec_zero(&ship_system->turret_big_attack_point);

		ship_system->subsys_cargo_name = -1;
		ship_system->subsys_cargo_revealed = 0;
		ship_system->time_subsys_cargo_revealed = 0;
		
		// zero flags
		ship_system->weapons.flags = 0;

		j = 0;
		for (k=0; k<MAX_PRIMARY_BANKS; k++){
			if (sp->primary_banks[k] != -1) {
				ship_system->weapons.primary_bank_weapons[j] = sp->primary_banks[k];
				ship_system->weapons.primary_bank_capacity[j] = sp->primary_bank_capacity[k];	// added by Goober5000
				ship_system->weapons.next_primary_fire_stamp[j++] = timestamp(0);
			}
		}

		ship_system->weapons.num_primary_banks = j;

		j = 0;
		for (k=0; k<MAX_SECONDARY_BANKS; k++){
			if (sp->secondary_banks[k] != -1) {
				ship_system->weapons.secondary_bank_weapons[j] = sp->secondary_banks[k];
				ship_system->weapons.secondary_bank_capacity[j] = sp->secondary_bank_capacity[k];
				ship_system->weapons.next_secondary_fire_stamp[j++] = timestamp(0);
			}
		}

		ship_system->weapons.num_secondary_banks = j;
		ship_system->weapons.current_primary_bank = -1;
		ship_system->weapons.current_secondary_bank = -1;
		
		for (k=0; k<MAX_SECONDARY_BANKS; k++) {
			ship_system->weapons.secondary_bank_ammo[k] = (Fred_running ? 100 : ship_system->weapons.secondary_bank_capacity[k]);

			ship_system->weapons.secondary_next_slot[k] = 0;
		}

		// Goober5000
		for (k=0; k<MAX_PRIMARY_BANKS; k++)
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
		ship_system->awacs_intensity = sp->awacs_intensity;
		ship_system->awacs_radius = sp->awacs_radius;
		if (ship_system->awacs_intensity > 0) {
			ship_system->system_info->flags |= MSS_FLAG_AWACS;
		}

		// turn_rate, turn_accel
		// model_set_instance_info
		float turn_accel = 0.5f;
		model_set_instance_info(&ship_system->submodel_info_1, sp->turn_rate, turn_accel);

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
//	ship_info	*sip;
	polymodel	*pm;
	dock_bay		*db;

//	sip = &Ship_info[Ships[objp->instance].ship_info_index];
	pm = model_get( Ships[objp->instance].modelnum );

	if (pm->docking_bays == NULL)
		return;

	if (pm->docking_bays[0].num_slots != 2)
		return;

	db = &pm->docking_bays[0];

	vertex	v0, v1;
	vector	p0, p1, p2, p3, nr;

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

DCF_BOOL( ship_shadows, Ship_shadows );

MONITOR( NumShipsRend );	

int Show_shield_hits = 0;
DCF_BOOL( show_shield_hits, Show_shield_hits );

int Show_tnorms = 0;
DCF_BOOL( show_tnorms, Show_tnorms );

int Show_paths = 0;
DCF_BOOL( show_paths, Show_paths );

int Show_fpaths = 0;
DCF_BOOL( show_fpaths, Show_fpaths );

void ship_render(object * obj)
{
	int num;
	ship_info * si;
	ship * shipp;

	num = obj->instance;

	Assert( num >= 0);	

#if 0
	// show target when attacking big ship
	vector temp, target;
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


	if ( obj == Viewer_obj ) {
		if (ship_show_velocity_dot && (obj==Player_obj) )	{
			vector p0,v;
			vertex v0;

			vm_vec_scale_add( &v, &obj->phys_info.vel, &obj->orient.vec.fvec, 3.0f );
			vm_vec_normalize( &v );
			
					
			vm_vec_scale_add( &p0, &obj->pos, &v, 20.0f);

			g3_rotate_vertex( &v0, &p0 );
			
			gr_set_color(0,128,0);
			g3_draw_sphere( &v0, 0.1f );
		}

		// Show the shield hit effect for the viewer.
		if ( Show_shield_hits )	{
			shipp = &Ships[num];
			if (shipp->shield_hits) {
				create_shield_explosion_all(obj);
				shipp->shield_hits = 0;
			}
		}		

		return;
	}

	MONITOR_INC( NumShipsRend, 1 );	

	shipp = &Ships[num];
	si = &Ship_info[Ships[num].ship_info_index];

	// Make ships that are warping in not render during stage 1
	if ( shipp->flags & SF_ARRIVING_STAGE_1 ){				
		return;
	}

	if ( Ship_shadows && shipfx_in_shadow( obj ) )	{
		light_set_shadow(1);
	} else {
		light_set_shadow(0);
	}

	ship_model_start(obj);

	uint render_flags = MR_NORMAL;

	// Turn off model caching for the player ship in external view.
	if (obj == Player_obj)	{
		render_flags |= MR_ALWAYS_REDRAW;	
	}

	// Turn off model caching if this is the player's target.
	if ( Player_ai->target_objnum == OBJ_INDEX(obj))	{
		render_flags |= MR_ALWAYS_REDRAW;	
	}	

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
				render_flags |= MR_ALWAYS_REDRAW;	// Turn off model caching if arcing.
				model_add_arc( shipp->modelnum, -1, &shipp->arc_pts[i][0], &shipp->arc_pts[i][1], shipp->arc_type[i] );
			}
		}
	}

//	if((shipp->end_death_time - shipp->death_time) <= 0)shipp->end_death_time = 0;
//	if((timestamp() - shipp->death_time) <= 0)shipp->end_death_time = 0;
	if( !( shipp->large_ship_blowup_index > -1 ) && timestamp_elapsed(shipp->death_time) && shipp->end_death_time){
		splodeingtexture = si->splodeing_texture;
		splodeing = true;
		splode_level = ((float)timestamp() - (float)shipp->death_time) / ((float)shipp->end_death_time - (float)shipp->death_time);
		//splode_level *= 2.0f;
		model_render( shipp->modelnum, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
		splodeing = false;
	}
//	if(splode_level<=0.0f)shipp->end_death_time = 0;

	if ( shipp->large_ship_blowup_index > -1 )	{
		shipfx_large_blowup_render(shipp);
	} else {

		//	ship_get_subsystem_strength( shipp, SUBSYSTEM_ENGINE)>ENGINE_MIN_STR

		if ( (shipp->thruster_bitmap > -1) && (!(shipp->flags & SF_DISABLED)) && (!ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE)) ) {
			vector	ft;//model_set_thrust now uses a vector as a scaleing paramiter

			//	Add noise to thruster geometry.
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

/*			if (ft.xyz.x || ft.xyz.y || ft.xyz.z) {
				model_set_thrust( shipp->modelnum, &ft, shipp->thruster_bitmap, shipp->thruster_glow_bitmap, shipp->thruster_glow_noise, (obj->phys_info.flags & PF_AFTERBURNER_ON || obj->phys_info.flags & PF_BOOSTER_ON), shipp->secondary_thruster_bitmap, shipp->tertiary_thruster_bitmap, &Objects[shipp->objnum].phys_info.rotvel, si->thruster01_rad_factor, si->thruster02_rad_factor, si->thruster02_len_factor, si->thruster02_rad_factor );
				render_flags |= MR_SHOW_THRUSTERS;
			}
			*/
			//had a conflict, had to chose one, and I don't like ^that^ one
			//model_set_thrust( shipp->modelnum, &ft, shipp->thruster_bitmap, shipp->thruster_glow_bitmap, shipp->thruster_glow_noise );
			model_set_thrust( shipp->modelnum, &ft, shipp->thruster_bitmap, shipp->thruster_glow_bitmap,
				shipp->thruster_glow_noise, (obj->phys_info.flags & PF_AFTERBURNER_ON || obj->phys_info.flags & PF_BOOSTER_ON),
				shipp->secondary_thruster_bitmap, shipp->tertiary_thruster_bitmap,
				&Objects[shipp->objnum].phys_info.rotvel, si->thruster01_rad_factor,
				si->thruster02_rad_factor, si->thruster02_len_factor, si->thruster02_rad_factor );
			render_flags |= MR_SHOW_THRUSTERS;
		}

		// fill the model flash lighting values in
		shipfx_flash_light_model( obj, shipp );

		object *docked_objp = NULL;
		ship * docked_shipp = NULL;
		ship * warp_shipp = shipp;
			 
		// check to see if departing ship is docked with anything.
		docked_objp = ai_find_docked_object( obj );
		if ( docked_objp ) {
			docked_shipp = &Ships[docked_objp->instance];

			if ( docked_shipp->flags & (SF_DEPART_WARP|SF_ARRIVING) )	{
				warp_shipp = docked_shipp;
			}
		}

		// Warp_shipp points to the ship that is going through a
		// warp... either this ship or the ship it is docked with.
		
		// If the ship is going "through" the warp effect, then
		// set up the model renderer to only draw the polygons in front
		// of the warp in effect
		int clip_started = 0;

		if ( warp_shipp->flags & (SF_ARRIVING|SF_DEPART_WARP) ) {

			clip_started = 1;
			g3_start_user_clip_plane( &warp_shipp->warp_effect_pos, &warp_shipp->warp_effect_fvec );

			// Turn off model caching while going thru warp effect.
			render_flags |= MR_ALWAYS_REDRAW;	
		}

		// maybe set squad logo bitmap
		model_set_insignia_bitmap(-1);

#ifndef NO_NETWORK
		if(Game_mode & GM_MULTIPLAYER){
			// if its any player's object
			int np_index = multi_find_player_by_object( obj );
			if((np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].player != NULL)){
				model_set_insignia_bitmap(Net_players[np_index].player->insignia_texture);
			}
		}
		// in single player, we want to render model insignias on all ships in alpha beta and gamma
		// Goober5000 - and also on wings that have their logos set
		else
#endif
		{
			// if its an object in my squadron
			if(ship_in_abgz(shipp)){
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

		// small ships
		if((The_mission.flags & MISSION_FLAG_FULLNEB) && (si->flags & SIF_SMALL_SHIP)){			
			// force detail levels
			float fog_val = neb2_get_fog_intensity(obj);
			if(fog_val >= 0.6f){
				model_set_detail_level(2);
				model_render( shipp->modelnum, &obj->orient, &obj->pos, render_flags | MR_LOCK_DETAIL, OBJ_INDEX(obj), -1, shipp->replacement_textures );
			} else {
				model_render( shipp->modelnum, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
			}
		} else {
			model_render( shipp->modelnum, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->replacement_textures );
		}

//		decal_render_all(obj);
//		mprintf(("out of the decal stuff\n"));

		// always turn off fog after rendering a ship
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

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
		vector tpos, tnorm, temp;
		vector v1, v2;
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
}

void ship_subsystem_delete(ship *shipp)
{
	ship_subsys *systemp, *temp;

	systemp = GET_FIRST( &shipp->subsys_list );
	while ( systemp != END_OF_LIST(&shipp->subsys_list) ) {
		temp = GET_NEXT( systemp );								// use temporary since pointers will get screwed with next operation
		list_remove( &shipp->subsys_list, systemp );			// remove the element
		list_append( &ship_subsys_free_list, systemp );		// and place back onto free list
		systemp = temp;												// use the temp variable to move right along
	}

	// Goober5000 - free stuff used for alt models if we have an alt model
	if (shipp->alt_modelnum != -1)
	{
		free(shipp->subsystems);
		shipp->n_subsystems = 0;
		shipp->subsystems = NULL;
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
	ship_subsystem_delete(&Ships[num]);

	shipp->objnum = -1;
	// mwa 11/24/97 num_ships--;

	if (model_get(shipp->modelnum)->shield.ntris) {
		free(shipp->shield_integrity);
		shipp->shield_integrity = NULL;
	}

	if ( shipp->ship_list_index != -1 ) {
		ship_obj_list_remove(shipp->ship_list_index);
		shipp->ship_list_index = -1;
	}

	free_sexp2(shipp->arrival_cue);
	free_sexp2(shipp->departure_cue);

	// call the contrail system
	ct_ship_delete(shipp);
}

// function used by ship_destroyed and ship_departed which is called if the ship
// is in a wing.  This function updates the ship_index list (i.e. removes it's
// entry in the list), and packs the array accordingly.
void ship_wing_cleanup( int shipnum, wing *wingp )
{
	int i, index = -1, team;

	team = Ships[shipnum].team;
	// compress the ship_index array and mark the last entry with a -1
	for (i = 0; i < wingp->current_count; i++ ) {
		if ( wingp->ship_index[i] == shipnum ) {
			index = i;
			break;
		}
	}

	// Assert(index != -1);
	
	// this can happen in multiplayer (dogfight, ingame join specifically)
	if(index == -1){
		return;
	}

	for ( i = index; i < wingp->current_count - 1; i++ ){
		wingp->ship_index[i] = wingp->ship_index[i+1];
	}

	wingp->current_count--;
	Assert ( wingp->current_count >= 0 );
	wingp->ship_index[wingp->current_count] = -1;

	// if the current count is 0, check to see if the wing departed or was destroyed.
	if ( wingp->current_count == 0 ) {

		// if this wing was ordered to depart by the player, set the current_wave equal to the total
		// waves so we can mark the wing as gone and no other ships arrive
		if ( wingp->flags & WF_DEPARTURE_ORDERED ) 
			wingp->current_wave = wingp->num_waves;

		// first, be sure to mark a wing destroyed event if all members of wing were destroyed and on
		// the last wave.  This circumvents a problem where the wing could be marked as departed and
		// destroyed if the last ships were destroyed after the wing's departure cue became true.

		// if the wing wasn't destroyed, and it is departing, then mark it as departed -- in this
		// case, there had better be ships in this wing with departure entries in the log file.  The
		// logfile code checks for this case.  
		if ( (wingp->current_wave == wingp->num_waves) && (wingp->total_destroyed == wingp->total_arrived_count) ) {
			mission_log_add_entry(LOG_WING_DESTROYED, wingp->name, NULL, team);
			wingp->flags |= WF_WING_GONE;
			wingp->time_gone = Missiontime;
		} else if ( (wingp->flags & WF_WING_DEPARTING) || (wingp->current_wave == wingp->num_waves) ) {
#ifndef NDEBUG
			ship_obj *so;


			// apparently, there have been reports of ships still present in the mission when this log
			// entry if written.  Do a sanity check here to find out for sure.
			for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {

				// skip the player -- stupid special case.
				if ( &Objects[so->objnum] == Player_obj )
					continue;

#ifndef NO_NETWORK
				if ( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN) )
					continue;
#endif
				if ( (Ships[Objects[so->objnum].instance].wingnum == WING_INDEX(wingp)) && !(Ships[Objects[so->objnum].instance].flags & (SF_DEPARTING|SF_DYING)) )
					Int3();
			}
#endif

			if ( wingp->flags & (WF_WING_DEPARTING|WF_DEPARTURE_ORDERED) )
				mission_log_add_entry(LOG_WING_DEPART, wingp->name, NULL, team);

			wingp->flags |= WF_WING_GONE;
			wingp->time_gone = Missiontime;
		}
	}
}

// function to do management, like log entries and wing cleanup after a ship has been destroyed

void ship_destroyed( int num )
{
	ship		*shipp;
	ship_subsys *temp_subsys;
	object	*objp;

	shipp = &Ships[num];
	objp = &Objects[shipp->objnum];

	// anti-frustration: beam-lock the ship - Goober5000 :)
	temp_subsys = GET_FIRST(&shipp->subsys_list);
	while(temp_subsys != END_OF_LIST(&shipp->subsys_list))
	{
		// mark all turrets as beam locked
		if(temp_subsys->system_info->type == SUBSYSTEM_TURRET)
		{
			temp_subsys->weapons.flags &= ~SW_FLAG_BEAM_FREE;
		}

		// next item
		temp_subsys = GET_NEXT(temp_subsys);
	}

	// add the information to the exited ship list
	ship_add_exited_ship( shipp, SEF_DESTROYED );

	// determine if we need to count this ship as a kill in counting number of kills per ship type
	// look at the ignore flag for the ship (if not in a wing), or the ignore flag for the wing
	// (if the ship is in a wing), and add to the kill count if the flags are not set
	if ( !(shipp->flags & SF_IGNORE_COUNT) ||  ((shipp->wingnum != -1) && !(Wings[shipp->wingnum].flags & WF_IGNORE_COUNT)) )
		ship_add_ship_type_kill_count( Ship_info[shipp->ship_info_index].flags );

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

	// let the event music system know a hostile was destoyed (important for deciding when to transition from battle to normal music)
	if (Player_ship != NULL) {
		if (shipp->team != Player_ship->team) {
			event_music_hostile_ship_destroyed();
		}
	}
}

void ship_vanished(int num)
{
	ship *sp;
	object *objp;	

	sp = &Ships[num];
	objp = &Objects[sp->objnum];

	// demo recording
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_departed(Objects[Ships[num].objnum].signature, Ships[num].flags);
	}

	// add the information to the exited ship list
	ship_add_exited_ship( sp, SEF_DEPARTED );

	// update wingman status gauge
	if ( (sp->wing_status_wing_index >= 0) && (sp->wing_status_wing_pos >= 0) ) {
		hud_set_wingman_status_departed(sp->wing_status_wing_index, sp->wing_status_wing_pos);
	}

	ai_ship_destroy(num, SEF_DEPARTED);		// should still do AI cleanup after ship has departed
}

void ship_departed( int num )
{
	ship *sp;
	int i;

	sp = &Ships[num];

	// demo recording
	if(Game_mode & GM_DEMO_RECORD){
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
	for ( i = 0; i < Num_jump_nodes; i++ ) {
		float radius, dist;
		vector ship_pos, node_pos;

		ship_pos = Objects[sp->objnum].pos;
		node_pos = Objects[Jump_nodes[i].objnum].pos;
		radius = model_get_radius( Jump_nodes[i].modelnum );
		dist = vm_vec_dist( &ship_pos, &node_pos );
		if ( dist <= radius ) {
			mission_log_add_entry(LOG_SHIP_DEPART, sp->ship_name, Jump_nodes[i].name, sp->wingnum);
			break;
		}
		dist = 1.0f;
	}

	if ( i == Num_jump_nodes ){
		mission_log_add_entry(LOG_SHIP_DEPART, sp->ship_name, NULL, sp->wingnum);
	}
		
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
int ship_explode_area_calc_damage( vector *pos1, vector *pos2, float inner_rad, float outer_rad, float max_damage, float max_blast, float *damage, float *blast )
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
	ship_info	*sip;
	Assert( exp_objp->type == OBJ_SHIP );
	float	inner_rad, outer_rad, max_damage, max_blast, shockwave_speed;
	shockwave_create_info sci;

	//	No area explosion in training missions.
	if (The_mission.game_type & MISSION_TYPE_TRAINING){
		return;
	}
		
	if ((exp_objp->hull_strength <= KAMIKAZE_HULL_ON_DEATH) && (Ai_info[Ships[exp_objp->instance].ai_index].ai_flags & AIF_KAMIKAZE) && (Ships[exp_objp->instance].special_exp_index < 0)) {
		float override = Ai_info[Ships[exp_objp->instance].ai_index].kamikaze_damage;

		inner_rad = exp_objp->radius*2.0f;
		outer_rad = exp_objp->radius*4.0f; // + (override * 0.3f);
		max_damage = override;
		max_blast = override * 5.0f;
		shockwave_speed = 100.0f;
	} else {
		sip = &Ship_info[Ships[exp_objp->instance].ship_info_index];

		if (Ships[exp_objp->instance].special_exp_index != -1) {
			int start = Ships[exp_objp->instance].special_exp_index;
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
			inner_rad = sip->inner_rad;
			outer_rad = sip->outer_rad;
			max_damage = sip->damage;
			max_blast  = sip->blast;
			shockwave_speed = sip->shockwave_speed;
		}
	}

	// nprintf(("AI", "Frame %i: Area effect blast from ship %s\n", Framecount, Ships[exp_objp->instance].ship_name));

	// account for ships that give no damage when they blow up.
	if ( (max_damage < 0.1f) && (max_blast < 0.1f) ){
		return;
	}

	if ( shockwave_speed > 0 ) {
		sci.inner_rad = inner_rad;
		sci.outer_rad = outer_rad;
		sci.blast = max_blast;
		sci.damage = max_damage;
		sci.speed = shockwave_speed;
		sci.rot_angle = frand_range(0.0f, 359.0f);
		shipfx_do_shockwave_stuff(&Ships[exp_objp->instance], &sci);
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
				vector force, vec_ship_to_impact;
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

void do_dying_undock_physics(object* objp, ship* sp) 
{
	Assert(sp->dock_objnum_when_dead >= 0);
	if(sp->dock_objnum_when_dead < 0){
		return;
	}
	object* dock_obj = &Objects[sp->dock_objnum_when_dead];

	// sanity checks
	Assert(objp->type == OBJ_SHIP);
	Assert(dock_obj->type == OBJ_SHIP);
	if((objp->type != OBJ_SHIP) || (dock_obj->type != OBJ_SHIP)){
		return;
	}

	float damage = 0.2f*sp->ship_initial_hull_strength;
	ship_apply_global_damage(dock_obj, objp, &objp->pos, damage);

	// do physics
	vector impulse_norm, impulse_vec, pos;
	vm_vec_sub(&impulse_norm, &dock_obj->pos, &objp->pos);
	vm_vec_normalize(&impulse_norm);
	// set for relative separation velocity of ~30
	float impulse_mag = 50.f*dock_obj->phys_info.mass*objp->phys_info.mass/(dock_obj->phys_info.mass + objp->phys_info.mass);
	vm_vec_copy_scale(&impulse_vec, &impulse_norm, impulse_mag);
	vm_vec_rand_vec_quick(&pos);
	vm_vec_scale(&pos, dock_obj->radius);
	// apply whack to dock obj
	physics_apply_whack(&impulse_vec, &pos, &dock_obj->phys_info, &dock_obj->orient, dock_obj->phys_info.mass);
	// enhance rotation of the docked ship
	vm_vec_scale(&dock_obj->phys_info.rotvel, 2.0f);

	// apply whack to ship
	vm_vec_negate(&impulse_vec);
	vm_vec_rand_vec_quick(&pos);
	vm_vec_scale(&pos, objp->radius);
	physics_apply_whack(&impulse_vec, &pos, &objp->phys_info, &objp->orient, objp->phys_info.mass);

	// reset dock_objnum_when_dead to -1 for dockee, since docker has blown up.
	if (Ships[dock_obj->instance].dock_objnum_when_dead == sp->objnum) {
		Ships[dock_obj->instance].dock_objnum_when_dead = -1;
	}
}

//	Do the stuff we do in a frame for a ship that's in its death throes.
void ship_dying_frame(object *objp, int ship_num)
{
	ship	*sp;
	sp = &Ships[ship_num];
	int knossos_ship = false;

	if ( sp->flags & SF_DYING )	{
		knossos_ship = (Ship_info[sp->ship_info_index].flags & SIF_KNOSSOS_DEVICE);

		// bash hull value toward 0 (from self destruct)
		if (objp->hull_strength > 0) {
			int time_left = timestamp_until(sp->final_death_time);
			float hits_left = objp->hull_strength;

			objp->hull_strength -= hits_left * (1000.0f * flFrametime) / time_left;
		}

		// special case of VAPORIZE
		if (sp->flags & SF_VAPORIZE) {
			// Assert(Ship_info[sp->ship_info_index].flags & SIF_SMALL_SHIP);
			if (timestamp_elapsed(sp->final_death_time)) {

				// play death sound
				snd_play_3d( &Snds[SND_VAPORIZED], &objp->pos, &View_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );

				// do joystick effect
				if (objp == Player_obj) {
					joy_ff_explode();
				}

				// if dying ship is docked, do damage to docked and physics
				if (sp->dock_objnum_when_dead != -1)  {
					do_dying_undock_physics(objp, sp);
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

		// bash the desired rotvel
		objp->phys_info.desired_rotvel = sp->deathroll_rotvel;

		// Do fireballs for Big ship with propagating explostion, but not Kamikaze
		if (!(Ai_info[sp->ai_index].ai_flags & AIF_KAMIKAZE) && ship_get_exp_propagates(sp)) {
			if ( timestamp_elapsed(Ships[ship_num].next_fireball))	{
				vector outpnt, pnt1, pnt2;
				polymodel *pm = model_get(sp->modelnum);

				// Gets two random points on the surface of a submodel
				submodel_get_two_random_points(sp->modelnum, pm->detail[0], &pnt1, &pnt2 );

				//	vm_vec_avg( &tmp, &pnt1, &pnt2 ); [KNOSSOS get random in plane 1/1.414 in rad
				model_find_world_point(&outpnt, &pnt1, sp->modelnum, pm->detail[0], &objp->orient, &objp->pos );

				float rad = objp->radius*0.1f;
				int fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				fireball_create( &outpnt, fireball_type, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
				// start the next fireball up in the next 50 - 200 ms (2-3 per frame)
				sp->next_fireball = timestamp_rand(333,500);

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, sp->sub_expl_sound_handle);
			}
		}

		// create little fireballs for knossos as it dies
		if (knossos_ship) {
			if ( timestamp_elapsed(Ships[ship_num].next_fireball)) {
				vector rand_vec, outpnt; // [0-.7 rad] in plane
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
				sp->next_fireball = timestamp_rand(333,500);

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
				particle_emit( &pe, PARTICLE_SMOKE2, 0, 50 );

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, sp->sub_expl_sound_handle);
			}
		}


		//nprintf(("AI", "Ship.cpp: Frame=%i, Time = %7.3f, Ship %s will die in %7.3f seconds.\n", Framecount, f2fl(Missiontime), Ships[ship_num].ship_name, (float) timestamp_until(sp->final_death_time)/1000.0f));
		int time_until_minor_explosions = timestamp_until(sp->final_death_time);

		// Wait until just before death and set off some explosions
		// If it is less than 1/2 second until large explosion, but there is
		// at least 1/10th of a second left, then create 5 small explosions
		if ( (time_until_minor_explosions < 500) && (time_until_minor_explosions > 100) && (!sp->pre_death_explosion_happened) ) {
			//mprintf(( "Ship almost dying!!\n" ));
			sp->next_fireball = timestamp(-1);	// never time out again
			sp->pre_death_explosion_happened=1;		// Mark this event as having occurred

			polymodel *pm = model_get(sp->modelnum);

			// Start shockwave for ship with propagating explosion, do now for timing
			if ( ship_get_exp_propagates(sp) ) {
				ship_blow_up_area_apply_blast( objp );
			}

			for (int zz=0; zz<6; zz++ ) {
				// dont make sequence of fireballs for knossos
				if (knossos_ship) {
					break;
				}
				// Find two random vertices on the model, then average them
				// and make the piece start there.
				vector tmp, outpnt, pnt1, pnt2;

				// Gets two random points on the surface of a submodel [KNOSSOS]
				submodel_get_two_random_points(sp->modelnum, pm->detail[0], &pnt1, &pnt2 );

				vm_vec_avg( &tmp, &pnt1, &pnt2 );
				model_find_world_point(&outpnt, &tmp, sp->modelnum, pm->detail[0], &objp->orient, &objp->pos );

				float rad = frand()*0.30f;
				rad += objp->radius*0.40f;
				fireball_create( &outpnt, FIREBALL_EXPLOSION_MEDIUM, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
			}

			// if ship is docked, undock now.
			if (sp->dock_objnum_when_dead != -1)  {				
				// other ship undocks
				//	These asserts should no longer be needed and they cause a problem that is not obvious how to fix.
				//Assert( !(Ai_info[Ships[dock_obj->instance].ai_index].ai_flags & AIF_DOCKED) );
				//Assert( Ai_info[Ships[dock_obj->instance].ai_index].dock_objnum == -1 );
				// MWA  Ai_info[Ships[dock_obj->instance].ai_index].ai_flags &= ~AIF_DOCKED;
				// MWA  Ai_info[Ships[dock_obj->instance].ai_index].dock_objnum = -1;
				// MWA Ai_info[Ships[dock_obj->instance].ai_index].mode = AIM_NONE;
			}
		}

		if ( timestamp_elapsed(sp->final_death_time))	{

			sp->death_time = sp->final_death_time;
			

			sp->final_death_time = timestamp(-1);	// never time out again
			//mprintf(( "Ship dying!!\n" ));
			
			// play ship explosion sound effect, pick appropriate explosion sound
			int sound_index;
			if ( Ship_info[sp->ship_info_index].flags & (SIF_CAPITAL | SIF_KNOSSOS_DEVICE) ) {
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

			if ( sp->death_roll_snd != -1 ) {
				snd_stop(sp->death_roll_snd);
				sp->death_roll_snd = -1;
			}

			// if dying ship is docked, do damage to docked and physics
			if (sp->dock_objnum_when_dead != -1)  {
				do_dying_undock_physics(objp, sp);
			}			

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

			if (!knossos_ship) {
				particle_emit( &pe, PARTICLE_SMOKE2, 0 );
			}

			// If this is a large ship with a propagating explosion, set it to blow up.
			if ( ship_get_exp_propagates(sp) )	{
				if (Ai_info[sp->ai_index].ai_flags & AIF_KAMIKAZE) {
					ship_blow_up_area_apply_blast( objp );
				}
				shipfx_large_blowup_init(sp);
				// need to timeout immediately to keep physics in sync
				sp->really_final_death_time = timestamp(0);
				polymodel *pm = model_get(sp->modelnum);
				sp->end_death_time = timestamp((int) pm->core_radius);
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
				if ( fireball_objnum > -1 )	{
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
				sp->end_death_time = sp->really_final_death_time = timestamp( fl2i(explosion_life*1000.0f)/5 );	// Wait till 30% of vclip time before breaking the ship up.
				//sp->really_final_death_time = timestamp(0);	// Make ship break apart the instant the explosion starts
			}

			sp->flags |= SF_EXPLODED;

			if ( !(ship_get_exp_propagates(sp)) ) {
				// apply area of effect blast damage from ship explosion
				ship_blow_up_area_apply_blast( objp );
			}
		}

		if ( timestamp_elapsed(sp->really_final_death_time))	{

			//mprintf(( "Ship really dying!!\n" ));
			// do large_ship_split and explosion
			if ( sp->large_ship_blowup_index > -1 )	{
				if ( shipfx_large_blowup_do_frame(sp, flFrametime) )	{
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

			shipfx_blow_up_model(objp, Ships[ship_num].modelnum, 0, 20, &objp->pos );

			// do all accounting for respawning client and server side here.
			if(objp == Player_obj) {				
				gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
			}

			objp->flags |= OF_SHOULD_BE_DEAD;
								
			ship_destroyed(ship_num);		// call ship function to clean up after the ship is destroyed.
			sp->really_final_death_time = timestamp( -1 );	// Never time out again!
		}

		// If a ship is dying (and not a capital or big ship) then stutter the engine sound
		if ( timestamp_elapsed(sp->next_engine_stutter) ) {
			if ( !(Ship_info[sp->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
				sp->flags ^= SF_ENGINES_ON;			// toggle state of engines
				sp->next_engine_stutter = timestamp_rand(50, 250);
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

	delta = frametime * ETS_RECHARGE_RATE * shipp->ship_initial_shield_strength / 100.0f;

	//	Chase target_shields and target_weapon_energy
	if (shipp->target_shields_delta > 0.0f) {
		if (delta > shipp->target_shields_delta)
			delta = shipp->target_shields_delta;

		add_shield_strength(obj, delta);
		shipp->target_shields_delta -= delta;
	} else if (shipp->target_shields_delta < 0.0f) {
		if (delta < -shipp->target_shields_delta)
			delta = -shipp->target_shields_delta;

		add_shield_strength(obj, -delta);
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


// loads the animations for ship's afterburners
void ship_init_thrusters()
{
	int			fps, i;
	thrust_anim	*ta;

	if ( Thrust_anim_inited == 1 )
		return;

	// AL 29-3-98: Don't want to include Shivan thrusters in the demo build
	int num_thrust_anims = NUM_THRUST_ANIMS;
	#ifdef DEMO // N/A FS2_DEMO
		num_thrust_anims = NUM_THRUST_ANIMS - 2;
	#endif

#if (MORE_SPECIES)
	for ( i = 0; i < num_thrust_anims && i < (True_NumSpecies * 2); i++ ) {
#else
	for ( i = 0; i < num_thrust_anims; i++ ) {
#endif
		ta = &Thrust_anims[i];
		ta->first_frame = bm_load_animation(Thrust_anim_names[i],  &ta->num_frames, &fps, 1);
		ta->secondary = bm_load(Thrust_secondary_anim_names[i]);
		ta->tertiary = bm_load(Thrust_tertiary_anim_names[i]);
		
		if ( ta->first_frame == -1 ) {
			Error(LOCATION,"Error loading animation file: %s\n",Thrust_anim_names[i]);
			return;
		}
		Assert(fps != 0);
		ta->time = i2fl(ta->num_frames)/fps;
	}

	// AL 29-3-98: Don't want to include Shivan thrusters in the demo build
	int num_thrust_glow_anims = NUM_THRUST_GLOW_ANIMS;
	#ifdef DEMO // N/A FS2_DEMO
		num_thrust_glow_anims = NUM_THRUST_GLOW_ANIMS - 2;
	#endif

#if (MORE_SPECIES)
	for ( i = 0; i < num_thrust_glow_anims && i < (True_NumSpecies * 2); i++ ) {
#else
	for ( i = 0; i < num_thrust_glow_anims; i++ ) {
#endif
		ta = &Thrust_glow_anims[i];
		ta->num_frames = NOISE_NUM_FRAMES;
		fps = 15;
		ta->first_frame = bm_load( Thrust_glow_anim_names[i] );
		if ( ta->first_frame == -1 ) {
			Error(LOCATION,"Error loading bitmap file: %s\n",Thrust_glow_anim_names[i]);
			return;
		}
		Assert(fps != 0);
		ta->time = i2fl(ta->num_frames)/fps;
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
	int anim_index;
	thrust_anim *the_anim;
	ship_info	*sinfo = &Ship_info[shipp->ship_info_index];

	bool AB = false;

	if ( !Thrust_anim_inited )	ship_init_thrusters();

	// The animations are organized by:
	// Species*2 + (After_burner_on?1:0)
	anim_index = sinfo->species*2;

	if ( objp->phys_info.flags & PF_AFTERBURNER_ON )	{
		anim_index++;		//	select afterburner anim.
		rate = 1.5f;		// go at 1.5x faster when afterburners on
		AB = true;
	} 
	else if (objp->phys_info.flags & PF_BOOSTER_ON)
	{
		anim_index++;		//	select afterburner anim.
		rate = 2.5f;		// go at 2.5x faster when booster pod on
		AB = true;
	}else {
		// If thrust at 0, go at half as fast, full thrust; full framerate
		// so set rate from 0.5 to 1.0, depending on thrust from 0 to 1
		// rate = 0.5f + objp->phys_info.forward_thrust / 2.0f;
		rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);
	}

//	rate = 0.1f;

	Assert( anim_index > -1 );
	Assert( anim_index < NUM_THRUST_ANIMS );

	the_anim = &Thrust_anims[anim_index];

	Assert( frametime > 0.0f );
	shipp->thruster_frame += frametime * rate;

	// Sanity checks
	if ( shipp->thruster_frame < 0.0f )	shipp->thruster_frame = 0.0f;
	if ( shipp->thruster_frame > 100.0f ) shipp->thruster_frame = 0.0f;

	while ( shipp->thruster_frame > the_anim->time )	{
		shipp->thruster_frame -= the_anim->time;
	}
	framenum = fl2i( (shipp->thruster_frame*the_anim->num_frames) / the_anim->time );
	if ( framenum < 0 ) framenum = 0;
	if ( framenum >= the_anim->num_frames ) framenum = the_anim->num_frames-1;

//	if ( anim_index == 0 )
//		mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  the_anim->num_frames, anim_index ));
	
	// Get the bitmap for this frame
	shipp->thruster_bitmap = the_anim->first_frame + framenum;
//	shipp->secondary_thruster_bitmap = the_anim->secondary;
//	shipp->tertiary_thruster_bitmap = the_anim->tertiary;
	

//	mprintf(( "TF: %.2f\n", shipp->thruster_frame ));

	// Do it for glow bitmaps
	the_anim = &Thrust_glow_anims[anim_index];

	Assert( frametime > 0.0f );
	shipp->thruster_glow_frame += frametime * rate;

	// Sanity checks
	if ( shipp->thruster_glow_frame < 0.0f )	shipp->thruster_glow_frame = 0.0f;
	if ( shipp->thruster_glow_frame > 100.0f ) shipp->thruster_glow_frame = 0.0f;

	while ( shipp->thruster_glow_frame > the_anim->time )	{
		shipp->thruster_glow_frame -= the_anim->time;
	}
	framenum = fl2i( (shipp->thruster_glow_frame*the_anim->num_frames) / the_anim->time );
	if ( framenum < 0 ) framenum = 0;
	if ( framenum >= the_anim->num_frames ) framenum = the_anim->num_frames-1;

//	if ( anim_index == 0 )
//		mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  the_anim->num_frames, anim_index ));
	
	// Get the bitmap for this frame
//	shipp->thruster_glow_bitmap = the_anim->first_frame;	// + framenum;
	shipp->thruster_glow_noise = Noise[framenum];


	if(AB)shipp->thruster_glow_bitmap = sinfo->thruster_glow1a;
	else shipp->thruster_glow_bitmap = sinfo->thruster_glow1;
	if(AB)shipp->secondary_thruster_bitmap = sinfo->thruster_glow2a;
	else shipp->secondary_thruster_bitmap = sinfo->thruster_glow2;
	if(AB)shipp->tertiary_thruster_bitmap = sinfo->thruster_glow3a;
	else shipp->tertiary_thruster_bitmap = sinfo->thruster_glow3;
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
	int anim_index;
	thrust_anim *the_anim;

	if ( !Thrust_anim_inited )	ship_init_thrusters();

	// The animations are organized by:
	// Species*2 + (After_burner_on?1:0)
	anim_index = weaponp->species*2;

	// If thrust at 0, go at half as fast, full thrust; full framerate
	// so set rate from 0.5 to 1.0, depending on thrust from 0 to 1
	// rate = 0.5f + objp->phys_info.forward_thrust / 2.0f;
	rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);

	Assert( anim_index > -1 );
	Assert( anim_index < NUM_THRUST_ANIMS );

	the_anim = &Thrust_anims[anim_index];

	Assert( frametime > 0.0f );
	weaponp->thruster_frame += frametime * rate;

	// Sanity checks
	if ( weaponp->thruster_frame < 0.0f )	weaponp->thruster_frame = 0.0f;
	if ( weaponp->thruster_frame > 100.0f ) weaponp->thruster_frame = 0.0f;

	while ( weaponp->thruster_frame > the_anim->time )	{
		weaponp->thruster_frame -= the_anim->time;
	}
	framenum = fl2i( (weaponp->thruster_frame*the_anim->num_frames) / the_anim->time );
	if ( framenum < 0 ) framenum = 0;
	if ( framenum >= the_anim->num_frames ) framenum = the_anim->num_frames-1;

//	if ( anim_index == 0 )
//		mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  the_anim->num_frames, anim_index ));
	
	// Get the bitmap for this frame
	weaponp->thruster_bitmap = the_anim->first_frame + framenum;

//	mprintf(( "TF: %.2f\n", weaponp->thruster_frame ));

	// Do it for glow bitmaps
	the_anim = &Thrust_glow_anims[anim_index];

	Assert( frametime > 0.0f );
	weaponp->thruster_glow_frame += frametime * rate;

	// Sanity checks
	if ( weaponp->thruster_glow_frame < 0.0f )	weaponp->thruster_glow_frame = 0.0f;
	if ( weaponp->thruster_glow_frame > 100.0f ) weaponp->thruster_glow_frame = 0.0f;

	while ( weaponp->thruster_glow_frame > the_anim->time )	{
		weaponp->thruster_glow_frame -= the_anim->time;
	}
	framenum = fl2i( (weaponp->thruster_glow_frame*the_anim->num_frames) / the_anim->time );
	if ( framenum < 0 ) framenum = 0;
	if ( framenum >= the_anim->num_frames ) framenum = the_anim->num_frames-1;

//	if ( anim_index == 0 )
//		mprintf(( "Frame = %d/%d, anim=%d\n", framenum+1,  the_anim->num_frames, anim_index ));
	
	// Get the bitmap for this frame
	weaponp->thruster_glow_bitmap = the_anim->first_frame;	// + framenum;
	weaponp->thruster_glow_noise = Noise[framenum];
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
	ship_subsys			*ssp;
	ship_subsys_info	*ssip;
	ship					*sp;
	ship_info			*sip;

	#ifndef NDEBUG
	if ( !Ship_auto_repair )	// only repair subsystems if Ship_auto_repair flag is set
		return;
	#endif

	Assert( shipnum >= 0 && shipnum < MAX_SHIPS);
	sp = &Ships[shipnum];
	sip = &Ship_info[sp->ship_info_index];

	// only allow for the auto-repair of subsystems on small ships
	if ( !(sip->flags & SIF_SMALL_SHIP) )
		return;

	// AL 3-14-98: only allow auto-repair if power output not zero
	if ( sip->power_output <= 0 )
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
			ssp->current_hits += ssp->max_hits * SHIP_REPAIR_SUBSYSTEM_RATE * frametime;
			ssip->current_hits += ssip->total_hits * SHIP_REPAIR_SUBSYSTEM_RATE * frametime;
		
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

#ifndef NO_NETWORK
	// multiplayer
	if (Game_mode & GM_MULTIPLAYER) {
		// if I'm the server, check all non-observer players including myself
		if (MULTIPLAYER_MASTER) {
			// warn all players
			for (idx=0; idx<MAX_PLAYERS; idx++) {
				if (MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Objects[Net_players[idx].player->objnum].type != OBJ_GHOST) ) {
					// if bad, blow him up
					ship_check_player_distance_sub(Net_players[idx].player, idx);
				}
			}
		}
	}
	// single player
	else 
#endif
	{
		// maybe blow him up
		ship_check_player_distance_sub(Player);
	}		
}

void observer_process_post(object *objp)
{
	Assert(objp->type == OBJ_OBSERVER);

#ifndef NO_NETWORK
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
#endif
}

// reset some physics info when ship's engines goes from disabled->enabled 
void ship_reset_disabled_physics(object *objp, int ship_class)
{
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
	aip->lethality = max(-10.0f, aip->lethality);

//	if (aip->lethality < min_lethality) {
//		min_lethality = aip->lethality;
//		mprintf(("new lethality low: %.1f\n", min_lethality));
//	}

#ifndef NDEBUG
	if (Objects[Ships[aip->shipnum].objnum].flags & OF_PLAYER_SHIP) {
		if (Framecount % 10 == 0) {
			int num_turrets = 0;
			if ((aip->target_objnum != -1) && (Objects[aip->target_objnum].type == OBJ_SHIP)) {
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
}

MONITOR( NumShips );	

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
		for ( int i=0; i<MAX_SECONDARY_BANKS; i++ ) {
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
			for ( int i=0; i<MAX_PRIMARY_BANKS; i++ )
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

#ifndef NO_NETWORK
	if(!(Game_mode & GM_STANDALONE_SERVER))
#endif
	{
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
		// fast enough to move 2x it's radius in SHIP_WARP_TIME seconds.
		shipfx_warpin_frame( obj, frametime );
	} else if ( shipp->flags & SF_DEPART_WARP ) {
		// JAS -- if the ship is warping out, just move it forward at a speed
		// fast enough to move 2x it's radius in SHIP_WARP_TIME seconds.
		shipfx_warpout_frame( obj, frametime );
	} else {
		//	Do AI.

#ifndef NO_NETWORK
		// for multiplayer people.  return here if in multiplay and not the host
		if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) )
			return;	
#endif

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
		if ( obj->flags & OF_PLAYER_SHIP ) {

			model_subsystem	*psub;
			ship_subsys	*pss;
			
			for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) )
			{
				psub = pss->system_info;

				// do solar/radar/gas/activator rotation here
				ship_do_submodel_rotation(shipp, psub, pss);
			}

			player_maybe_fire_turret(obj);
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
	polymodel	*po;
	ship_weapon *swp = &shipp->weapons;
	weapon_info *wip;

	//	Copy primary and secondary weapons from ship_info to ship.
	//	Later, this will happen in the weapon loadout screen.
	for (i=0; i < MAX_PRIMARY_BANKS; i++){
		swp->primary_bank_weapons[i] = sip->primary_bank_weapons[i];
	}

	for (i=0; i < MAX_SECONDARY_BANKS; i++){
		swp->secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
	}

	// Copy the number of primary and secondary banks to ship, and verify that
	// model is in synch
	po = model_get( shipp->modelnum );

	// Primary banks
	if ( po->n_guns > sip->num_primary_banks ) {
		Assert(po->n_guns <= MAX_PRIMARY_BANKS);
		Warning(LOCATION, "There are %d primary banks in the model file,\nbut only %d primary banks in ships.tbl for %s\n", po->n_guns, sip->num_primary_banks, sip->name);
		for ( i = sip->num_primary_banks; i < po->n_guns; i++ ) {
			// Make unspecified weapon for bank be a Light Laser
			swp->primary_bank_weapons[i] = weapon_info_lookup(NOX("Light Laser"));
			Assert(swp->primary_bank_weapons[i] >= 0);
		}
		sip->num_primary_banks = po->n_guns;
	}
	else if ( po->n_guns < sip->num_primary_banks ) {
		Warning(LOCATION, "There are %d primary banks in ships.tbl for %s\nbut only %d primary banks in the model\n", sip->num_primary_banks, sip->name, po->n_guns);
		sip->num_primary_banks = po->n_guns;
	}

	// Secondary banks
	if ( po->n_missiles > sip->num_secondary_banks ) {
		Assert(po->n_missiles <= MAX_SECONDARY_BANKS);
		Warning(LOCATION, "There are %d secondary banks in model,\nbut only %d secondary banks in ships.tbl for %s\n", po->n_missiles, sip->num_secondary_banks, sip->name);
		for ( i = sip->num_secondary_banks; i < po->n_missiles; i++ ) {
			// Make unspecified weapon for bank be a Rockeye Missile
			swp->secondary_bank_weapons[i] = weapon_info_lookup(NOX("Rockeye Missile"));
			Assert(swp->secondary_bank_weapons[i] >= 0);
		}
		sip->num_secondary_banks = po->n_missiles;
	}
	else if ( po->n_missiles < sip->num_secondary_banks ) {
		Warning(LOCATION, "There are %d secondary banks in ships.tbl for %s,\n but only %d secondary banks in the model.\n", sip->num_secondary_banks, sip->name, po->n_missiles);
		sip->num_secondary_banks = po->n_missiles;
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
				swp->primary_bank_ammo[i] = sip->primary_bank_ammo_capacity[i];
			}

			swp->primary_bank_capacity[i] = sip->primary_bank_ammo_capacity[i];
		}
	}

	swp->num_secondary_banks = sip->num_secondary_banks;
	for ( i = 0; i < swp->num_secondary_banks; i++ ) {
		if (Fred_running){
			swp->secondary_bank_ammo[i] = 100;
		} else {
			swp->secondary_bank_ammo[i] = sip->secondary_bank_ammo_capacity[i];
		}

		swp->secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
	}

	for ( i = 0; i < MAX_PRIMARY_BANKS; i++ ){
		swp->next_primary_fire_stamp[i] = timestamp(0);
	}

	for ( i = 0; i < MAX_SECONDARY_BANKS; i++ ){
		swp->next_secondary_fire_stamp[i] = timestamp(0);
	}
}


//	A faster version of ship_check_collision that does not do checking at the polygon
//	level.  Just checks to see if a vector will intersect a sphere.
int ship_check_collision_fast( object * obj, object * other_obj, vector * hitpos)
{
	int num;
	mc_info mc;

	Assert( obj->type == OBJ_SHIP );
	Assert( obj->instance >= 0 );

	num = obj->instance;

	ship_model_start(obj);	// are these needed in this fast case? probably not.

	mc.model_num = Ships[num].modelnum;	// Fill in the model to check
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

// ensure that the subsys path is at least SUBSYS_PATH_DIST from the 
// second last to last point.
void ship_maybe_fixup_subsys_path(polymodel *pm, int path_num)
{
	vector	*v1, *v2, dir;
	float		dist;
	int		index_1, index_2;

	model_path *mp;
	mp = &pm->paths[path_num];

	Assert(mp != NULL);
	Assert(mp->nverts > 1);
	
	index_1 = 1;
	index_2 = 0;

	v1 = &mp->verts[index_1].pos;
	v2 = &mp->verts[index_2].pos;
	
	dist = vm_vec_dist(v1, v2);
	if ( dist < SUBSYS_PATH_DIST-10 ) {
		vm_vec_normalized_dir(&dir, v2, v1);
		vm_vec_scale_add(v2, v1, &dir, SUBSYS_PATH_DIST);
	}
}

// fill in the path_num field inside the model_subsystem struct.  This is an index into
// the pm->paths[] array, which is a path that provides a frontal approach to a subsystem
// (used for attacking purposes)
//
// NOTE: path_num in model_subsystem has the follows the following convention:
//			> 0	=> index into pm->paths[] for model that subsystem sits on
//			-1		=> path is not yet determined (may or may not exist)
//			-2		=> path doesn't yet exist for this subsystem
void ship_set_subsys_path_nums(ship_info *sip, polymodel *pm)
{
	int i,j,found_path;

	for ( i = 0; i < sip->n_subsystems; i++ ) {
		sip->subsystems[i].path_num = -1;
	}

	for ( i = 0; i < sip->n_subsystems; i++ ) {
		found_path = 0;
		for ( j = 0; j < pm->n_paths; j++ ) {
			if ( (sip->subsystems[i].subobj_num != -1) && (sip->subsystems[i].subobj_num == pm->paths[j].parent_submodel) ) {
				found_path = 1;
			} else if ( !subsystem_stricmp(sip->subsystems[i].subobj_name, pm->paths[j].parent_name) ) {
				found_path = 1;
			}
	
			if ( found_path ) {
				if ( pm->n_paths > j ) {
					sip->subsystems[i].path_num = j;
					ship_maybe_fixup_subsys_path(pm, j);
					break;
				}
			}
		}

		// If a path num wasn't located, then set value to -2
		if ( sip->subsystems[i].path_num == -1 )
			sip->subsystems[i].path_num = -2;
	}
}

// Determine the path indices (indicies into pm->paths[]) for the paths used for approaching/departing
// a fighter bay on a capital ship.
void ship_set_bay_path_nums(ship_info *sip, polymodel *pm)
{
	int	bay_num, i;
	char	bay_num_str[3];

	if ( pm->ship_bay != NULL ) {
		free(pm->ship_bay);
		pm->ship_bay = NULL;
	}

	// currently only capital ships have fighter bays
	if ( !(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		return;
	}

	// malloc out storage for the path information
	pm->ship_bay = (ship_bay*)malloc(sizeof(ship_bay));
	Assert(pm->ship_bay != NULL);

	pm->ship_bay->num_paths = 0;
	// TODO: determine if zeroing out here is affecting any earlier initializations
	pm->ship_bay->arrive_flags = 0;	// bitfield, set to 1 when that path number is reserved for an arrival
	pm->ship_bay->depart_flags = 0;	// bitfield, set to 1 when that path number is reserved for a departure


	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	for ( i = 0; i < pm->n_paths; i++ ) {
		if ( !strnicmp(pm->paths[i].name, NOX("$bay"), 4) ) {
			strncpy(bay_num_str, pm->paths[i].name+4, 2);
			bay_num_str[2] = 0;
			bay_num = atoi(bay_num_str);
			Assert(bay_num >= 1 && bay_num <= MAX_SHIP_BAY_PATHS);
			pm->ship_bay->paths[bay_num-1] = i;
			pm->ship_bay->num_paths++;
		}
	}
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

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_SHIP) {
			count += Ship_info[Ships[objp->type].ship_info_index].n_subsystems;
		}
	}

	//nprintf(("AI", "Num subsystems, high water mark = %i, %i\n", count, Ship_subsys_hwm));

	if (count > Ship_subsys_hwm) {
		Ship_subsys_hwm = count;
	}
}

//	Returns object index of ship.
//	-1 means failed.
int ship_create(matrix *orient, vector *pos, int ship_type, char *ship_name)
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

	Assert((ship_type >= 0) && (ship_type < Num_ship_types));
	sip = &(Ship_info[ship_type]);
	shipp = &Ships[n];

	//  check to be sure that this ship falls into a ship size category!!!
	//  get Allender or Mike if you hit this Assert
	Assert( sip->flags & (SIF_SMALL_SHIP | SIF_BIG_SHIP | SIF_CAPITAL | SIF_NO_SHIP_TYPE | SIF_NOT_FLYABLE | SIF_ESCAPEPOD | SIF_SUPERCAP | SIF_DRYDOCK | SIF_KNOSSOS_DEVICE) );

	sip->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);		// use the highest detail level
	shipp->modelnum = sip->modelnum;

	if(strcmp(sip->shockwave_pof_file,""))
	sip->shockwave_moddel = model_load(sip->shockwave_pof_file, 0, NULL);
	else sip->shockwave_moddel = -1;

	// alt stuff
	shipp->alt_modelnum = -1;
	shipp->n_subsystems = 0;
	shipp->subsystems = NULL;

	// check for texture_replacement - Goober5000
	if (ship_name)
	{
		// do we have a replacement?
		for (i=0; i<Num_texture_replacements; i++)
		{
			if (!stricmp(ship_name, Texture_replace[i].ship_name))
			{
				// allocate space for subsystems
				shipp->n_subsystems = sip->n_subsystems;
				if ( shipp->n_subsystems > 0 )
				{
					shipp->subsystems = (model_subsystem *)malloc(sizeof(model_subsystem) * shipp->n_subsystems );
					Assert( shipp->subsystems != NULL );
				}
		
				// copy original subsys data
				for ( i = 0; i < shipp->n_subsystems; i++ )
				{
					shipp->subsystems[i] = sip->subsystems[i];
				}

				// now load the duplicate model
				shipp->modelnum = model_load(sip->pof_file, shipp->n_subsystems, &shipp->subsystems[0], 1, 1);
				shipp->alt_modelnum = shipp->modelnum;
				model_duplicate_reskin(shipp->modelnum, ship_name);
				break;
			}
		}
	}

	// maybe load an optional hud target model
	if(strlen(sip->pof_file_hud)){
		// check to see if a "real" ship uses this model. if so, load it up for him so that subsystems are setup properly
		int idx;
		for(idx=0; idx<Num_ship_types; idx++){
			if(!stricmp(Ship_info[idx].pof_file, sip->pof_file_hud)){
				Ship_info[idx].modelnum = model_load(Ship_info[idx].pof_file, Ship_info[idx].n_subsystems, &Ship_info[idx].subsystems[0]);
			}
		}

		// mow load it for me with no subsystems
		sip->modelnum_hud = model_load(sip->pof_file_hud, 0, NULL);
	}

	// we must do stuff for both the original and alternate models - Goober5000
	polymodel *pm_orig = model_get(sip->modelnum);
	polymodel *pm_alt = NULL;

	if (shipp->alt_modelnum != -1)
	{
		pm_alt = model_get(shipp->alt_modelnum);
	}


	ship_copy_subsystem_fixup(sip);

	show_ship_subsys_count();
	if (pm_alt)
	{
		show_ship_subsys_count();	// Goober5000 - double if two models
	}

	if ( sip->num_detail_levels < pm_orig->n_detail_levels )
	{
		Warning(LOCATION, "For ship '%s', detail level\nmismatch (POF needs %d)", sip->name, pm_orig->n_detail_levels );

		for (i=0; i<pm_orig->n_detail_levels; i++ )	{
			sip->detail_distance[i] = 0;
		}
	}
	
	// Goober5000 - one for each model
	for (i=0; i<sip->num_detail_levels; i++ )	{
		pm_orig->detail_depth[i] = i2fl(sip->detail_distance[i]);
	}

	if (pm_alt)
	{
		for (i=0; i<sip->num_detail_levels; i++ )	{
			pm_alt->detail_depth[i] = i2fl(sip->detail_distance[i]);
		}
	}


	if ( sip->flags & SIF_NAVBUOY )	{
		// JAS: Nav buoys don't need to do collisions!
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(shipp->modelnum), OF_RENDERS | OF_PHYSICS );
	} else {
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(shipp->modelnum), OF_RENDERS | OF_COLLIDES | OF_PHYSICS );
	}
	Assert( objnum >= 0 );

	shipp->ai_index = ai_get_slot(n);
	Assert( shipp->ai_index >= 0 );

	sprintf(shipp->ship_name, NOX("%s %d"), Ship_info[ship_type].name, n);

	ship_set_default_weapons(shipp, sip);	//	Moved up here because ship_set requires that weapon info be valid.  MK, 4/28/98
	ship_set(n, objnum, ship_type);

	// fill in the path_num field inside the model_subsystem struct.  This is an index into
	// the pm->paths[] array, which is a path that provides a frontal approach to a subsystem
	// (used for attacking purposes)
	//
	// NOTE: path_num in model_subsystem has the follows the following convention:
	//			> 0	=> index into pm->paths[] for model that subsystem sits on
	//			-1		=> path is not yet determined (may or may not exist)
	//			-2		=> path doesn't yet exist for this subsystem
	ship_set_subsys_path_nums(sip, pm_orig);
	if (pm_alt)
	{
		ship_set_subsys_path_nums(sip, pm_alt);
	}

	// set the path indicies for fighter bays on the ship (currently, only capital ships have fighter bays)
	ship_set_bay_path_nums(sip, pm_orig);
	if (pm_alt)
	{
		ship_set_bay_path_nums(sip, pm_alt);
	}

	init_ai_object(objnum);
	ai_clear_ship_goals( &Ai_info[Ships[n].ai_index] );		// only do this one here.  Can't do it in init_ai because it might wipe out goals in mission file

	//ship_set_default_weapons(shipp, sip);

	//	Allocate shield and initialize it.
	if (pm_orig->shield.ntris) {
		shipp->shield_integrity = (float *)malloc(sizeof(float)*pm_orig->shield.ntris);
		for (i=0; i<pm_orig->shield.ntris; i++)
			shipp->shield_integrity[i] = 1.0f;

	} else
		shipp->shield_integrity = NULL;
	
	// fix up references into paths for this ship's model to point to a ship_subsys entry instead
	// of a submodel index.  The ship_subsys entry should be the same for *all* instances of the
	// same ship.

	// evaluate for both models!
	for (int temp = 0; temp < 2; temp++)
	{
		polymodel *pm = NULL;
		if (temp == 0) pm = pm_orig;
		if (temp == 1) pm = pm_alt;

		if (!pm)
			continue;

		if ( !(sip->flags & SIF_PATH_FIXUP) || (temp == 1))
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
							Warning(LOCATION, "Couldn't fix up turret indices in spline path\n\nModel: %s\nPath: %s\nVertex: %d\nTurret model id:%d\n\nThis probably means the turret was not specified in ships.tbl", sip->pof_file, pm->paths[i].name, j, ptindex );
					}
				}
			}
			sip->flags |= SIF_PATH_FIXUP;
		}
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

/*	sip->thruster_glow1 = bm_load(sip->thruster_bitmap1);
	sip->thruster_glow1a = bm_load(sip->thruster_bitmap1a);
	sip->thruster_glow2 = bm_load(sip->thruster_bitmap2);
	sip->thruster_glow2a = bm_load(sip->thruster_bitmap2a);
	sip->thruster_glow3 = bm_load(sip->thruster_bitmap3);
	sip->thruster_glow3a = bm_load(sip->thruster_bitmap3a);
*/
	trail_info *ci;
	//first try at ABtrails -Bobboau	
	shipp->ab_count = 0;
	if(sip->flags & SIF_AFTERBURNER)
	{
		for(int h = 0; h < pm_orig->n_thrusters; h++)
		{
			for(int j = 0; j < pm_orig->thrusters->num_slots; j++)
			{
				// this means you've reached the max # of AB trails for a ship
				Assert(sip->ct_count <= MAX_SHIP_CONTRAILS);
	
				ci = &shipp->ab_info[shipp->ab_count++];
			//	ci = &sip->ct_info[sip->ct_count++];

			if(sip->ct_count >= MAX_SHIP_CONTRAILS)break;
			for(int j = 0; j < pm_orig->thrusters->num_slots; j++){
			// this means you've reached the max # of AB trails for a ship
			Assert(sip->ct_count < MAX_SHIP_CONTRAILS);

			ci = &shipp->ab_info[shipp->ab_count++];
			if(sip->ct_count >= MAX_SHIP_CONTRAILS)break;
		//	ci = &sip->ct_info[sip->ct_count++];

			if(pm_orig->thrusters[h].pnt[j].xyz.z < 0.5)continue;// only make ab trails for thrusters that are pointing backwards

			ci->pt = pm_orig->thrusters[h].pnt[j];//offset
				ci->w_start = pm_orig->thrusters[h].radius[j] * sip->ABwidth_factor;//width * table loaded width factor
	
				ci->w_end = 0.05f;//end width
	
				ci->a_start = 1.0f * sip->ABAlpha_factor;//start alpha  * table loaded alpha factor
	
				ci->a_end = 0.0f;//end alpha
	
				ci->max_life = sip->ABlife;// table loaded max life
	
				ci->stamp = 60;	//spew time???	

			ci->bitmap = sip->ABbitmap; //table loaded bitmap used on this ships burner trails
			mprintf(("ab trail point %d made\n", shipp->ab_count));
			}
			}
		}
	}//end AB trails -Bobboau

	// call the contrail system
	ct_ship_create(shipp);


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
void ship_model_change(int n, int ship_type, int force_ship_info_stuff)
{
	int			i;
	ship_info	*sip;
	ship			*sp;

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];
	sip = &(Ship_info[ship_type]);

	sp->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);		// use the highest detail level

	Objects[sp->objnum].radius = model_get_radius(sp->modelnum);

	// page in nondims in game
	if(!Fred_running){
		model_page_in_textures(sp->modelnum, ship_type);
	}

	// usually don't do this stuff in-game, it'll mess up existing ship info stuff
	if (Fred_running || force_ship_info_stuff)
	{
		sip->modelnum = sp->modelnum;

		// reset alt stuff
		sp->alt_modelnum = -1;
		sp->n_subsystems = 0;
		if (sp->subsystems != NULL)
		{
			free(sp->subsystems);
			sp->subsystems = NULL;
		}

		polymodel * pm;
		pm = model_get(sp->modelnum);

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
	}
	// this we want to do in game if there's not a ship class change
	else
	{
		sp->alt_modelnum = sp->modelnum;
		sp->n_subsystems = sip->n_subsystems;

		// redo the memory thing with subsystems
		if (sp->subsystems != NULL)
		{
			free( sp->subsystems );
			sp->subsystems = NULL;
		}
		if ( sp->n_subsystems > 0 )
		{
			sp->subsystems = (model_subsystem *)malloc(sizeof(model_subsystem) * sp->n_subsystems );
			Assert( sp->subsystems != NULL );
		}
		
		// recopy from ship_info
		for ( i = 0; i < sp->n_subsystems; i++ )
		{
			sp->subsystems[i] = sip->subsystems[i];
		}

		// set subsystem model data
		model_copy_subsystems( sp->n_subsystems, sp->subsystems, sip->subsystems );
	}
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

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];
	sip = &(Ship_info[ship_type]);
	objp = &Objects[sp->objnum];

	// point to new ship data
	sp->ship_info_index = ship_type;

	ship_model_change(n, ship_type, 1);

	// set the correct hull strength
	if (Fred_running) {
		sp->ship_initial_hull_strength = 100.0f;
		objp->hull_strength = 100.0f;
	} else {
		if (sp->special_hitpoint_index != -1) {
			sp->ship_initial_hull_strength = (float) atoi(Sexp_variables[sp->special_hitpoint_index+HULL_STRENGTH].text);
		} else {
			sp->ship_initial_hull_strength = sip->initial_hull_strength;
		}

		// Goober5000: don't set hull strength if called by sexp
		if (!by_sexp)
		{
			objp->hull_strength = sp->ship_initial_hull_strength;
		}
	}

	// set the correct shields strength
	if (Fred_running) {
		if (sp->ship_initial_shield_strength)
			sp->ship_initial_shield_strength = 100.0f;
		objp->shield_quadrant[0] = 100.0f;
	} else {
		if (sp->special_hitpoint_index != -1) {
			sp->ship_initial_shield_strength = (float) atoi(Sexp_variables[sp->special_hitpoint_index+SHIELD_STRENGTH].text);
		} else {
			sp->ship_initial_shield_strength = sip->initial_shield_strength;
		}

		// Goober5000: don't set shield strength if called by sexp
		if (!by_sexp)
		{
			set_shield_strength(objp, sp->ship_initial_shield_strength);
		}
	}

	// Goober5000: div-0 checks
	Assert(sp->ship_initial_hull_strength > 0.0f);
	Assert(objp->hull_strength > 0.0f);

	// subsys stuff done only after hull stuff is set

	// if the subsystem list is not currently empty, then we need to clear it out first.
	if ( NOT_EMPTY(&sp->subsys_list) ) {
		ship_subsys *ship_system, *tmp;

		for ( ship_system = GET_FIRST(&sp->subsys_list); ship_system != END_OF_LIST(&sp->subsys_list);  ) {
			tmp = GET_NEXT(ship_system);
			list_remove( &sp->subsys_list, ship_system );
			list_append( &ship_subsys_free_list, ship_system );
			ship_system = tmp;
		}
	}
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
}

#ifndef NDEBUG
//	Fire the debug laser
int ship_fire_primary_debug(object *objp)
{
	int	i;
	ship	*shipp = &Ships[objp->instance];
	vector wpos;

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
		weapon_objnum = weapon_create( &wpos, &objp->orient, i, OBJ_INDEX(objp), 0 );
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
	int	fired, check_count, cmeasure_count;
	vector	pos;
	ship	*shipp;

	shipp = &Ships[objp->instance];

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
	fired = -1;
	check_count = 1;
#ifndef NO_NETWORK
	if ( MULTIPLAYER_CLIENT && (objp != Player_obj) ){
		check_count = 0;
	}
#endif

	if (check_count && (shipp->cmeasure_count <= 0) ) {
		if ( objp == Player_obj ) {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No more countermeasure charges.", 485));
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
	fired = cmeasure_create( objp, &pos, shipp->current_cmeasure, rand_val );

	// Play sound effect for counter measure launch
	Assert(shipp->current_cmeasure < Num_cmeasure_types);
	if ( Cmeasure_info[shipp->current_cmeasure].launch_sound != -1 ) {
		snd_play_3d( &Snds[Cmeasure_info[shipp->current_cmeasure].launch_sound], &pos, &View_position );
	}

	
send_countermeasure_fired:
#ifndef NO_NETWORK
	// the new way of doing things
	// if(Netgame.debug_flags & NETD_FLAG_CLIENT_FIRING){
	if(Game_mode & GM_MULTIPLAYER){
		send_NEW_countermeasure_fired_packet( objp, cmeasure_count, fired );
	}
	// }
	// the old way of doing things
	//else {
	 //	if ( MULTIPLAYER_MASTER ){
		//	send_countermeasure_fired_packet( objp, cmeasure_count, fired );
		//}
	//}
#endif

	return (fired>0);		// return 0 if not fired, 1 otherwise
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
	pinfo.optional_data = wip->laser_bitmap;
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

	if(shipp->was_firing_last_frame[bank_to_stop] == 0)return 0;

	swp = &shipp->weapons;

		shipp->was_firing_last_frame[bank_to_stop] = 0;

		int weapon = swp->primary_bank_weapons[bank_to_stop];
		weapon_info* winfo_p = &Weapon_info[weapon];


		shipp->life_left[bank_to_stop] = winfo_p->b_info.beam_life;					//for fighter beams
		shipp->life_total[bank_to_stop] = winfo_p->b_info.beam_life;					//for fighter beams
		shipp->warmdown_stamp[bank_to_stop] = -1;				//for fighter beams
		shipp->warmup_stamp[bank_to_stop] = -1;				//for fighter beams

		snd_stop(shipp->fighter_beam_loop_sound[bank_to_stop]);
		shipp->fighter_beam_loop_sound[bank_to_stop] = -1;//stops the beam looping sound -bobboau

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

	int num_primary_banks = 0, bank_to_stop = 0;
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
		num_primary_banks = min(1, swp->num_primary_banks);
	}

	for ( int i = 0; i < num_primary_banks; i++ ) {	
		// Goober5000 - allow more than two banks
		bank_to_stop = (swp->current_primary_bank+i) % swp->num_primary_banks;
		//only stop if it was fireing last frame
		if(shipp->was_firing_last_frame[bank_to_stop] ){
			ship_stop_fire_primary_bank(obj, bank_to_stop);
		}
	}

/*	if ( obj == Player_obj ){
		gr_printf(10, 10, "stoped all");
	}
*/

	return 1;
}



//	Multiplicative delay factors for increasing skill levels.
float Ship_fire_delay_scale_hostile[NUM_SKILL_LEVELS] =  {4.0f, 2.5f, 1.75f, 1.25f, 1.0f};
float Ship_fire_delay_scale_friendly[NUM_SKILL_LEVELS] = {2.0f, 1.4f, 1.25f, 1.1f, 1.0f};

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
	vector		gun_point, pnt, firing_pos;
	int			n = obj->instance;
	ship			*shipp;
	ship_weapon	*swp;
	ship_info	*sip;
	ai_info		*aip;
	int			weapon, i, j, weapon_objnum;
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
	if((shipp->ship_info_index < 0) || (shipp->ship_info_index >= Num_ship_types)){
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

	sound_played = -1;

	// Fire the correct primary bank.  If primaries are linked (SF_PRIMARY_LINKED set), then fire 
	// both primary banks.
	int	num_primary_banks;

	if ( shipp->flags & SF_PRIMARY_LINKED ) {
		num_primary_banks = swp->num_primary_banks;
	} else {
		num_primary_banks = min(1, swp->num_primary_banks);
	}

	Assert(num_primary_banks > 0);
	if (num_primary_banks < 1){
		return 0;
	}

	// if we're firing stream weapons, but the trigger is not down, do nothing
	if(stream_weapons && !(shipp->flags & SF_TRIGGER_DOWN)){
		return 0;
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
		weapon_info* winfo_p = &Weapon_info[weapon];

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
				next_fire_delay *= Ship_fire_delay_scale_friendly[Game_skill_level];
			} else {
				next_fire_delay *= Ship_fire_delay_scale_hostile[Game_skill_level];
			}
		}

		polymodel *po = model_get( shipp->modelnum );
		
		next_fire_delay *= 1.0f + (num_primary_banks - 1) * 0.5f;		//	50% time penalty if banks linked

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
			if ((winfo_p->wi_flags & WIF_BEAM) && (winfo_p->b_info.beam_type == BEAM_TYPE_C))// fighter beams fire constantly, they only stop if they run out of power -Bobboau
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp();
		}

		if (winfo_p->wi_flags2 & WIF2_CYCLE){
			Assert(po->gun_banks[bank_to_fire].num_slots != 0);
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay / po->gun_banks[bank_to_fire].num_slots));
			//to maintain balence of fighters with more fire points they will fire faster that ships with fewer points
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
		
		if (winfo_p->wi_flags & WIF_BEAM){	// beam weapon?
			if ( obj == Player_obj ) {//beam sounds for the player
				sound_played = winfo_p->launch_snd;
				if ( winfo_p->launch_snd != -1 ) {
					weapon_info *wip;
					ship_weapon *swp;

					if(!(snd_is_playing(shipp->fighter_beam_loop_sound[bank_to_fire])) ){
						shipp->fighter_beam_loop_sound[bank_to_fire] = -1;
					}
					if(shipp->fighter_beam_loop_sound[bank_to_fire] == -1 ){
						shipp->fighter_beam_loop_sound[bank_to_fire] = snd_play_looping( &Snds[winfo_p->launch_snd], 0.0f, -1, -1, 1.0f, SND_PRIORITY_MUST_PLAY );
					}
	
					swp = &Player_ship->weapons;
					if (swp->current_primary_bank >= 0)
					{
						wip = &Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]];
						int force_level = (int) ((wip->armor_factor + wip->shield_factor * 0.2f) * (wip->damage * wip->damage - 7.5f) * 0.45f + 0.6f) * 10 + 2000;

						// modify force feedback for ballistics: make it stronger...
						// will we ever have a ballistic beam? probably not, but just in case
						if (wip->wi_flags2 & WIF2_BALLISTIC)
							joy_ff_play_primary_shoot(force_level * 2);
						// no ballistics
						else
							joy_ff_play_primary_shoot(force_level);
					}
				}
			}else{//beam sounds for other fighters
				//if the fighter doesn't have a fighter beam sound from being fired last frame give it one
		
		/*		if(!(snd_is_playing(shipp->fighter_beam_loop_sound[bank_to_fire])) ){
					snd_stop(shipp->fighter_beam_loop_sound[bank_to_fire]);
					shipp->fighter_beam_loop_sound[bank_to_fire] = -1;
				}
*/
				vector pos, temp = obj->pos, temp2 = obj->pos;

				vm_vec_unrotate(&temp2, &temp, &obj->orient);
				vm_vec_add2(&temp, &temp2);
				vm_vec_scale_add(&temp, &temp2, &obj->orient.vec.fvec, Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].b_info.range);

				switch(vm_vec_dist_to_line(&View_position, &obj->pos, &temp, &pos, NULL)){
					// behind the beam, so use the start pos
				case -1:
					pos = obj->pos;
					break;
			
					// use the closest point
				case 0:
					// already calculated in vm_vec_dist_to_line(...)
					break;

					// past the beam, so use the shot pos
				case 1:
					pos = temp;
					break;
				}

				if((shipp->fighter_beam_loop_sound[bank_to_fire] == -1)){				
					shipp->fighter_beam_loop_sound[bank_to_fire] = snd_play_3d( &Snds[winfo_p->launch_snd], &pos, &View_position, 0.0f, NULL, 1, 1.0, SND_PRIORITY_SINGLE_INSTANCE, NULL, 1.0f, 1);
				}else{//the fighter has a beam sound already, update it
					snd_update_3d_pos(shipp->fighter_beam_loop_sound[bank_to_fire], &Snds[winfo_p->launch_snd], &pos);
				}

	
			}//end of sound beam stuff
		}

		if ( po->n_guns > 0 ) {
			int num_slots = po->gun_banks[bank_to_fire].num_slots;
			
			if(winfo_p->wi_flags & WIF_BEAM){		// the big change I made for fighter beams, if there beams fill out the Fire_Info for a targeting laser then fire it, for each point in the weapon bank -Bobboau
//mprintf(("I am going to fire a fighter beam\n"));
								// fail unless we're forcing (energy based primaries)
				if ( (shipp->weapon_energy < num_slots*winfo_p->energy_consumed*flFrametime) && !force) {
					swp->next_primary_fire_stamp[bank_to_fire] = timestamp(swp->next_primary_fire_stamp[bank_to_fire]);
					if ( obj == Player_obj ) {
						if ( ship_maybe_play_primary_fail_sound() ) {
						}
					}
					ship_stop_fire_primary_bank(obj, bank_to_fire);
					continue;
				}			


				// deplete the weapon reserve energy by the amount of energy used to fire the weapon and the number of points and do it by the time it's been fireing becase this is a beam -Bobboau
				shipp->weapon_energy -= num_slots*winfo_p->energy_consumed*flFrametime;
				
				fighter_beam_fire_info fbfire_info;
/*
				if(shipp->was_firing_last_frame[bank_to_fire] == 1){
					if(shipp->warmup_stamp[bank_to_fire] < timestamp())
						shipp->warmup_stamp[bank_to_fire] = -1;

					shipp->life_left[bank_to_fire] -= flFrametime;
				}else{
					shipp->warmup_stamp[bank_to_fire] = timestamp() + winfo_p->b_info.beam_warmup;
					shipp->life_left[bank_to_fire] = winfo_p->b_info.beam_life;					//for fighter beams
					shipp->life_total[bank_to_fire] = winfo_p->b_info.beam_life;					//for fighter beams
					shipp->warmdown_stamp[bank_to_fire] = -1;				//for fighter beams
				
				/*	if ( obj == Player_obj ){
						HUD_printf("first frame");
					}*/

//				}
				


					shipp->warmup_stamp[bank_to_fire] = -1;
					shipp->life_left[bank_to_fire] = 0.0f;					//for fighter beams
					shipp->life_total[bank_to_fire] = 0.0f;					//for fighter beams
					shipp->warmdown_stamp[bank_to_fire] = -1;				//for fighter beams

				//	if ( obj == Player_obj ){
						fbfire_info.fighter_beam_loop_sound = -1;
				//	}else{
				//		fbfire_info.fighter_beam_loop_sound = shipp->fighter_beam_loop_sound[bank_to_fire];		//fighterbeam loop sound -Bobboau
				//	}
				fbfire_info.life_left = shipp->life_left[bank_to_fire];					//for fighter beams
				fbfire_info.life_total = shipp->life_total[bank_to_fire];					//for fighter beams
				fbfire_info.warmdown_stamp = shipp->warmdown_stamp[bank_to_fire];				//for fighter beams
				fbfire_info.warmup_stamp = shipp->warmup_stamp[bank_to_fire];				//for fighter beams
//mprintf(("preliminary fighter beam data\n"));

			//	if ( obj == Player_obj ){
			//		HUD_printf("warmup %d, life left %0.3f", shipp->warmup_stamp[bank_to_fire], shipp->life_left[bank_to_fire]);
			//	}

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

				int j; // fireing point cycleing for TBP

//mprintf(("I am about to fire a fighter beam\n"));

				for ( int v = 0; v < points; v++ ){
//mprintf(("I am about to fire a fighter beam2\n"));
					if(winfo_p->b_info.beam_shots){
						j = ( (timestamp()/(int)((winfo_p->fire_wait * 2000.0f) / num_slots)) + v)%num_slots;
					}else{
						j=v;
					}
//mprintf(("I am about to fire a fighter beam3\n"));					
					fbfire_info.accuracy = 0.0f;
					fbfire_info.beam_info_index = shipp->weapons.primary_bank_weapons[bank_to_fire];
					fbfire_info.beam_info_override = NULL;
					fbfire_info.shooter = &Objects[shipp->objnum];
					fbfire_info.target = NULL;
					fbfire_info.target_subsys = NULL;
					fbfire_info.turret = NULL;
					fbfire_info.targeting_laser_offset = po->gun_banks[bank_to_fire].pnt[j];			
					winfo_p->b_info.beam_type = BEAM_TYPE_C;
//mprintf(("I am about to fire a fighter beam4\n"));
					beam_fire_targeting(&fbfire_info);
					num_fired++;
					//shipp->targeting_laser_objnum = beam_fire_targeting(&fire_info);			
				}

//mprintf(("I have fired a fighter beam, type %d\n", winfo_p->b_info.beam_type));

			}
			else	//if this insn't a fighter beam, do it normaly -Bobboau
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

#ifndef NO_NETWORK
					if ( MULTIPLAYER_CLIENT && (obj != Player_obj) )
					{
						check_ammo = 0;
					}
#endif

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
						swp->primary_bank_ammo[bank_to_fire] -= num_slots;

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
				shipp->weapon_energy -= num_slots*winfo_p->energy_consumed;
				
				// Mark all these weapons as in the same group
				int new_group_id = weapon_create_group_id();

				int points = 0, numtimes = 1;

				//ok if this is a cycleing weapon use shots as the number of points to fire from at a time
				//otherwise shots is the number of times all points will be fired (used mostly for the 'shotgun' effect)
				if (winfo_p->wi_flags2 & WIF2_CYCLE){
					if (winfo_p->shots > num_slots){
						points = num_slots;
					}else{
						points = winfo_p->shots;
					}
				}else{
					numtimes = winfo_p->shots;
					points = num_slots;
				}


//mprintf(("I am going to fire a weapon %d times, from %d points, the last point fired was %d, and that will be point %d\n",numtimes,points,shipp->last_fired_point[bank_to_fire],shipp->last_fired_point[bank_to_fire]%num_slots));
				for(int w = 0; w<numtimes; w++){
				for ( j = 0; j < points; j++ )
				{
					int pt; //point
					if (winfo_p->wi_flags2 & WIF2_CYCLE){
						//pnt = po->gun_banks[bank_to_fire].pnt[shipp->last_fired_point[bank_to_fire]+j%num_slots];
						pt = shipp->last_fired_point[bank_to_fire]+j%num_slots;
//mprintf(("fireing from %d\n",shipp->last_fired_point[bank_to_fire]+j%num_slots));
					}else{
						//pnt = po->gun_banks[bank_to_fire].pnt[j];
						pt = j;
//mprintf(("fireing from %d\n",j));
					}

					pnt = po->gun_banks[bank_to_fire].pnt[pt];

					vm_vec_unrotate(&gun_point, &pnt, &obj->orient);
					vm_vec_add(&firing_pos, &gun_point, &obj->pos);
			
					// create the weapon -- the network signature for multiplayer is created inside
					// of weapon_create
					weapon_objnum = weapon_create( &firing_pos, &obj->orient, weapon, OBJ_INDEX(obj),0, new_group_id );

					weapon_set_tracking_info(weapon_objnum, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);				

					if (winfo_p->wi_flags & WIF_FLAK)
					{
						object* target=&Objects[aip->target_objnum];
						vector predicted_pos;
						float flak_range=(winfo_p->lifetime)*(winfo_p->max_speed);
						float range_to_target;
						float wepstr=ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS);
												
						set_predicted_enemy_pos(&predicted_pos,obj,target,aip);
						
						range_to_target=vm_vec_dist(&predicted_pos, &obj->pos);

						//if we have a target and its in range
						if ((target) && (range_to_target < flak_range))
						{
							//set flak range to range of ship
							flak_pick_range(&Objects[weapon_objnum],&predicted_pos,wepstr);
						}
						else
						{
							flak_set_range(&Objects[weapon_objnum],&firing_pos,flak_range-20);
						}

						if ((winfo_p->muzzle_flash>=0) && (((shipp==Player_ship) && (vm_vec_mag(&Player_obj->phys_info.vel)>=45)) || (shipp!=Player_ship)))
						{
							flak_muzzle_flash(&firing_pos,&obj->orient.vec.fvec, swp->primary_bank_weapons[bank_to_fire]);
						}
					}
					// create the muzzle flash effect
					shipfx_flash_create( obj, shipp, &pnt, &obj->orient.vec.fvec, 1, weapon );
	
					// maybe shudder the ship - if its me
					if((winfo_p->wi_flags & WIF_SHUDDER) && (obj == Player_obj) && !(Game_mode & GM_STANDALONE_SERVER)){
						// calculate some arbitrary value between 100
						// (mass * velocity) / 10
						game_shudder_apply(500, (winfo_p->mass * winfo_p->max_speed) / 10.0f);
					}

					num_fired++;
				}
				}
			}
			
			shipp->last_fired_point[bank_to_fire] = (shipp->last_fired_point[bank_to_fire] + 1) % num_slots;

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
						ship_weapon *swp;

						// HACK
						if(winfo_p->launch_snd == SND_AUTOCANNON_SHOT){
							snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE );
						} else {
							snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
						}
		//				snd_play( &Snds[winfo_p->launch_snd] );
	
						swp = &Player_ship->weapons;
						if (swp->current_primary_bank >= 0)
						{
							wip = &Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]];
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

				Net_players[player_num].player->stats.mp_shots_fired += num_fired;
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
			m = model_get(shipp->modelnum);
			if(m == NULL){
				continue;
			}

			// fire a targeting laser
			fire_info.life_left = 0.0;					//for fighter beams
			fire_info.life_total = 0.0f;					//for fighter beams
			fire_info.warmdown_stamp = -1;				//for fighter beams
			fire_info.warmup_stamp = -1;				//for fighter beams
			fire_info.accuracy = 0.0f;
			fire_info.beam_info_index = shipp->weapons.primary_bank_weapons[shipp->targeting_laser_bank];
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
	polymodel	*po;
	vector		missile_point, pnt, firing_pos;

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

	// If ship is being repaired/rearmed, it cannot fire missiles
	if ( aip->ai_flags & AIF_BEING_REPAIRED ) {
		return 0;
	}

	num_fired = 0;		// tracks how many missiles actually fired

	bank = swp->current_secondary_bank;
	if ( bank < 0 ) {
		return 0;
	}

	weapon = swp->secondary_bank_weapons[bank];
	Assert( (swp->secondary_bank_weapons[bank] >= 0) && (swp->secondary_bank_weapons[bank] < MAX_WEAPON_TYPES) );
	if((swp->secondary_bank_weapons[bank] < 0) || (swp->secondary_bank_weapons[bank] >= MAX_WEAPON_TYPES)){
		return 0;
	}
	wip = &Weapon_info[swp->secondary_bank_weapons[bank]];

	have_timeout = 0;			// used to help tell whether or not we have a timeout
#ifndef NO_NETWORK
	if ( MULTIPLAYER_MASTER ) {
		starting_sig = multi_get_next_network_signature( MULTI_SIG_NON_PERMANENT );
		starting_bank_count = swp->secondary_bank_ammo[bank];
	}
#endif

	if (ship_fire_secondary_detonate(obj, swp)) {
		// in multiplayer, master sends a secondary fired packet with starting signature of -1 -- indicates
		// to client code to set the detonate timer to 0.
#ifndef NO_NETWORK
		if ( MULTIPLAYER_MASTER ) {
			// MWA -- 4/6/98  Assert invalid since the bank count could have gone to 0.
			//Assert(starting_bank_count != 0);
			send_secondary_fired_packet( shipp, 0, starting_bank_count, 1, allow_swarm );
		}
#endif
	
		//	For all banks, if ok to fire a weapon, make it wait a bit.
		//	Solves problem of fire button likely being down next frame and
		//	firing weapon despite fire causing detonation of existing weapon.
		if (swp->current_secondary_bank >= 0) {
			if (timestamp_elapsed(swp->next_secondary_fire_stamp[bank])){
				swp->next_secondary_fire_stamp[bank] = timestamp(max((int) flFrametime*3000, 250));
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
#ifndef NO_NETWORK
				if ( !MULTIPLAYER_CLIENT )
#endif
				{
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
#ifndef NO_NETWORK
				if ( !MULTIPLAYER_CLIENT )
#endif
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
#ifndef NO_NETWORK
	if ( !(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER )
#endif
	{
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

	po = model_get( shipp->modelnum );
	if ( po->n_missiles > 0 ) {
		int check_ammo;		// used to tell if we should check ammo counts or not
		int num_slots;

		if ( bank > po->n_missiles ) {
			nprintf(("WARNING","WARNING ==> Tried to fire bank %d, but ship has only %d banks\n", bank+1, po->n_missiles));
			return 0;		// we can make a quick out here!!!
		}

		num_slots = po->missile_banks[bank].num_slots;

		// determine if there is enough ammo left to fire weapons on this bank.  As with primary
		// weapons, we might or might not check ammo counts depending on game mode, who is firing,
		// and if I am a client in multiplayer
		check_ammo = 1;
#ifndef NO_NETWORK
		if ( MULTIPLAYER_CLIENT && (obj != Player_obj) ){
			check_ammo = 0;
		}
#endif

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
			pnt = po->missile_banks[bank].pnt[pnt_index++];
			vm_vec_unrotate(&missile_point, &pnt, &obj->orient);
			vm_vec_add(&firing_pos, &missile_point, &obj->pos);

#ifndef NO_NETWORK
			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Weapon_info[weapon].subtype == WP_MISSILE );
			}
#endif

			// create the weapon -- for multiplayer, the net_signature is assigned inside
			// of weapon_create
			weapon_num = weapon_create( &firing_pos, &obj->orient, weapon, OBJ_INDEX(obj), 0, -1, aip->current_target_is_locked);
			weapon_set_tracking_info(weapon_num, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);

			// create the muzzle flash effect
			shipfx_flash_create( obj, shipp, &pnt, &obj->orient.vec.fvec, 0, weapon );

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
				/*if (shipp->tertiary_weapon_info_idx >= 0)
				{
					tertiary_weapon_info *twip=&Tertiary_weapon_info[shipp->tertiary_weapon_info_idx];
					if ((twip->type == TWT_AMMO_POD) && (shipp->ammopod_current_secondary==bank) && (shipp->ammopod_current_ammo > 0))
					{
						shipp->ammopod_current_ammo--;
					}
					else
					{
						swp->secondary_bank_ammo[bank]--;
					}
				}*/
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
#ifndef NO_NETWORK
		// if I am the master of a multiplayer game, send a secondary fired packet along with the
		// first network signatures for the newly created weapons.  if nothing got fired, send a failed
		// packet if 
		if ( MULTIPLAYER_MASTER ) {			
			Assert(starting_sig != 0);
			send_secondary_fired_packet( shipp, starting_sig, starting_bank_count, num_fired, allow_swarm );			
		}
#endif
		// STATS
		if (obj->flags & OF_PLAYER_SHIP) {
#ifndef NO_NETWORK
			// in multiplayer -- only the server needs to keep track of the stats.  Call the cool
			// function to find the player given the object *.  It had better return a valid player
			// or our internal structure as messed up.
			if( Game_mode & GM_MULTIPLAYER ) {
				if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
					int player_num;

					player_num = multi_find_player_by_object ( obj );
					Assert ( player_num != -1 );

					Net_players[player_num].player->stats.ms_shots_fired += num_fired;
				}				
			}
			else
#endif
			{
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
			//swp->next_secondary_fire_stamp[swp->current_secondary_bank] = max(timestamp(250),timestamp(fire_wait));	//	1/4 second delay until can fire	//DTP, Commented out mistake, here AL put the wroung firewait into the correct next_firestamp
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
	else if ( swp->num_primary_banks > MAX_SUPPORTED_PRIMARY_BANKS )
	{
		Error (LOCATION, "Exceeded supported number of primary banks.\n");
	}
	else
	{
		Assert((swp->current_primary_bank >= 0)/* && (swp->current_primary_bank <= UPPER_BOUND_PRIMARY_BANK)*/);//no idea what UPPER_BOUND_PRIMARY_BANK it's only refrence is in two asserts, commenting it out so we can compile

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
				swp->current_primary_bank = UPPER_BOUND_SUPPORTED_PRIMARY_BANK;
			}
		}
		// now handle when not linked: cycle and constrain
		else
		{
			if ( direction == CYCLE_PRIMARY_NEXT )
			{
				if ( swp->current_primary_bank < UPPER_BOUND_SUPPORTED_PRIMARY_BANK )
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
						swp->current_primary_bank = UPPER_BOUND_SUPPORTED_PRIMARY_BANK;
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
		Assert((swp->current_primary_bank >= 0) /*&& (swp->current_primary_bank <= UPPER_BOUND_PRIMARY_BANK)*/);//no idea what UPPER_BOUND_PRIMARY_BANK it's only refrence is in two asserts, commenting it out so we can compile

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
	else if ( swp->num_secondary_banks > MAX_SUPPORTED_SECONDARY_BANKS )
	{
		Error (LOCATION, "Exceeded supported number of secondary banks.\n");
	}
	else
	{
		Assert(swp->current_secondary_bank < swp->num_secondary_banks);
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

	if ( Fred_running )
		wing_limit = MAX_WINGS;
	else
		wing_limit = num_wings;

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
	for(idx=0;idx<num_wings;idx++)
		if(strcmp(Wings[idx].name,name)==0)
		   return idx;

	return -1;
}

//	Return the index of Ship_info[].name that is *name.
int ship_info_lookup(char *name)
{
	int	i;

	// bogus
	if(name == NULL){
		return -1;
	}

	for (i=0; i < Num_ship_types; i++)
		if (!stricmp(name, Ship_info[i].name))
			return i;

	return -1;
}

//	Return the index of Ship_info[].name which is the *base* ship of a ship copy
int ship_info_base_lookup(int si_index)
{
	int	i;
	char name[NAME_LENGTH];

	strcpy( name, Ship_info[si_index].name );
	Assert( get_pointer_to_first_hash_symbol(name) );						// get allender -- something bogus with ship copy
	end_string_at_first_hash_symbol(name);

	i = ship_info_lookup( name );
	Assert( i != -1 );				// get allender -- there had better be a base ship!

	return i;
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
	int idx;

	// bogus
	if(name == NULL){
		return -1;
	}

	// look through the Ship_type_names array
	for(idx=0; idx<MAX_SHIP_TYPE_COUNTS; idx++){
		if(!stricmp(name, Ship_type_names[idx])){
			return idx;
		}
	}

	// couldn't find it
	return -1;
}

// checks the (arrival & departure) state of a ship.  Return values:
// -1: has yet to arrive in mission
//  0: is currently in mission
//  1: has been destroyed, departed, or never existsed
int ship_query_state(char *name)
{
	int i;
	p_object *objp;

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

	objp = GET_FIRST(&ship_arrival_list);
	while (objp != END_OF_LIST(&ship_arrival_list)) {
		if (!stricmp(name, objp->name)){
			return -1;
		}

		objp = GET_NEXT(objp);
	}

	return 1;
}

//	Note: This is not a general purpose routine.
//	It is specifically used for targeting.
//	It only returns a subsystem position if it has shields.
//	Return true/false for subsystem found/not found.
//	Stuff vector *pos with absolute position.
// subsysp is a pointer to the subsystem.
int get_subsystem_pos(vector *pos, object *objp, ship_subsys *subsysp)
{
	matrix	m;
	model_subsystem	*psub;
	vector	pnt;
	ship		*shipp;

	Assert(objp->type == OBJ_SHIP);
	shipp = &Ships[objp->instance];

	Assert ( subsysp != NULL );

	psub = subsysp->system_info;
	vm_copy_transpose_matrix(&m, &objp->orient);

	vm_vec_rotate(&pnt, &psub->pnt, &m);
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

	shipp = &Ships[objp->instance];

	// First clear all the angles in the model to zero
	model_clear_instance(shipp->modelnum);

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
			break;
		case SUBSYSTEM_TURRET:
			Assert( !(psub->flags & MSS_FLAG_ROTATES) ); // Turrets can't rotate!!! See John!
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}


		if ( psub->subobj_num > -1 )	{
			model_set_instance(shipp->modelnum, psub->subobj_num, &pss->submodel_info_1 );
		}

		if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >-1) )		{
			model_set_instance(shipp->modelnum, psub->turret_gun_sobj, &pss->submodel_info_2 );
		}

	}
}

//==========================================================
// Clears all the instance specific stuff out of the model info
void ship_model_stop(object *objp)
{
	ship		*shipp;

	shipp = &Ships[objp->instance];

	// Then, clear all the angles in the model to zero
	model_clear_instance(shipp->modelnum);
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
// view_positions array in the model.  The 0th element is the noral viewing position.
// the vector of the eye is returned in the parameter 'eye'.  The orientation of the
// eye is returned in orient.  (NOTE: this is kind of bogus for now since non 0th element
// eyes have no defined up vector)
void ship_get_eye( vector *eye_pos, matrix *eye_orient, object *obj )
{
	ship *shipp;
	polymodel *pm;
	eye *ep;
	// vector vec;

	shipp = &Ships[obj->instance];
	pm = model_get( shipp->modelnum );

	// check to be sure that we have a view eye to look at.....spit out nasty debug message
	if ( pm->n_view_positions == 0 ) {
//		nprintf (("Warning", "No eye position found for model %s.  Find artist to get fixed.\n", pm->filename ));
		*eye_pos = obj->pos;
		*eye_orient = obj->orient;
		return;
	}
	ep = &(pm->view_positions[0] );

	// eye points are stored in an array -- the normal viewing position for a ship is the current_eye_index
	// element.
	model_find_world_point( eye_pos, &ep->pnt, shipp->modelnum, ep->parent, &obj->orient, &obj->pos );
	// if ( shipp->current_eye_index == 0 ) {
		*eye_orient = obj->orient;
	//} else {
	// 	model_find_world_dir( &vec, &ep->norm, shipp->modelnum, ep->parent, &obj->orient, &obj->pos );
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
ship_subsys *ship_get_best_subsys_to_attack(ship *sp, int subsys_type, vector *attacker_pos)
{
	ship_subsys	*ss;
	ship_subsys *best_in_sight_subsys, *lowest_attacker_subsys, *ss_return;
	int			lowest_num_attackers, lowest_in_sight_attackers, num_attackers;
	vector		gsubpos;
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
ship_subsys *ship_get_indexed_subsys( ship *sp, int index, vector *attacker_pos )
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
// calculated for the engines is slightly different.  Once an engine reaches < 15% of it's hits, it's
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

	strength = shipp->subsys_info[type].current_hits / shipp->subsys_info[type].total_hits;

	if ( strength == 0.0f )		// short circuit 0
		return strength;

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

// ==================================================================================
// ship_do_rearm_frame()
//
// function to rearm a ship.  This function gets called from the ai code ai_do_rearm_frame (or
// some function of a similar name).  Returns 1 when ship is fully repaired and rearmed, 0 otherwise
//

#define REARM_NUM_MISSILES_PER_BATCH 4		// how many missiles are dropped in per load sound
#define REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH	100	// how many bullets are dropped in per load sound

int ship_do_rearm_frame( object *objp, float frametime )
{
	int			i, banks_full, primary_banks_full, subsys_type, subsys_all_ok;
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
		shield_str = get_shield_strength(objp);
		if ( shield_str < shipp->ship_initial_shield_strength ) {
			if ( objp == Player_obj ) {
				player_maybe_start_repair_sound();
			}
			shield_str += shipp->ship_initial_shield_strength * frametime * SHIELD_REPAIR_RATE;
			if ( shield_str > shipp->ship_initial_shield_strength ) {
				 shield_str = shipp->ship_initial_shield_strength;
			}
			set_shield_strength(objp, shield_str);
		}
	}

	// Repair the ship integrity (subsystems + hull).  This works by applying the repair points
	// to the subsystems.  Ships integrity is stored is objp->hull_strength, so that always is 
	// incremented by repair_allocated
	repair_allocated = shipp->ship_initial_hull_strength * frametime * HULL_REPAIR_RATE;


//	AL 11-24-97: remove increase to hull integrity
//	Comments removed by PhReAk; Note that this is toggled on/off with a mission flag

	//Figure out how much of the ship's hull we can repair
	max_hull_repair = shipp->ship_initial_hull_strength * (The_mission.support_ships.max_hull_repair_val * 0.01f);
	
	if(The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL)
	{
		objp->hull_strength += repair_allocated;
		if ( objp->hull_strength > max_hull_repair ) {
			objp->hull_strength = max_hull_repair;
		}

		if ( objp->hull_strength > shipp->ship_initial_hull_strength )
		{
			objp->hull_strength = shipp->ship_initial_hull_strength;
			repair_allocated -= ( shipp->ship_initial_hull_strength - objp->hull_strength);
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
			if ( swp->secondary_bank_ammo[i] < swp->secondary_bank_start_ammo[i] )
			{
				float rearm_time;

				if ( objp == Player_obj )
				{
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}

				if ( timestamp_elapsed(swp->secondary_bank_rearm_time[i]) )
				{
					// Have to do some gymnastics to play the sound effects properly.  There is a
					// one time sound effect which is the missile loading start, then for each missile
					// loaded there is a sound effect.  These are only played for the player.
					//
					rearm_time = Weapon_info[swp->secondary_bank_weapons[i]].rearm_rate;
					if ( aip->rearm_first_missile == TRUE )
					{
						rearm_time *= 3;
					}

					swp->secondary_bank_rearm_time[i] = timestamp( (int)(rearm_time * 1000.f) );

					// Actual loading of missiles is preceded by a sound effect which is the missile
					// loading equipment moving into place
					if ( aip->rearm_first_missile == TRUE )
					{
						snd_play_3d( &Snds[SND_MISSILE_START_LOAD], &objp->pos, &View_position );
						aip->rearm_first_missile = FALSE;
					}
					else
					{
						snd_play_3d( &Snds[SND_MISSILE_LOAD], &objp->pos, &View_position );
						if (objp == Player_obj)
							joy_ff_play_reload_effect();

						swp->secondary_bank_ammo[i] += REARM_NUM_MISSILES_PER_BATCH;
						if ( swp->secondary_bank_ammo[i] > swp->secondary_bank_start_ammo[i] ) 
							swp->secondary_bank_ammo[i] = swp->secondary_bank_start_ammo[i]; 
					}
				}

			} else
				banks_full++;
		}	// end for

		// rearm ballistic primaries - Goober5000
		if (sip->flags & SIF_BALLISTIC_PRIMARIES)
		{
			for (i = 0; i < swp->num_primary_banks; i++ )
			{
				if ( Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC )
				{
					if ( swp->primary_bank_ammo[i] < swp->primary_bank_start_ammo[i] )
					{
						float rearm_time;
	
						if ( objp == Player_obj )
						{
							hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
						}

						if ( timestamp_elapsed(swp->primary_bank_rearm_time[i]) )
						{
							// Have to do some gymnastics to play the sound effects properly.  There is a
							// one time sound effect which is the ballistic loading start, then for each ballistic
							// loaded there is a sound effect.  These are only played for the player.
							//
							rearm_time = Weapon_info[swp->primary_bank_weapons[i]].rearm_rate;
							if ( aip->rearm_first_ballistic_primary == TRUE )
							{
								rearm_time *= 3;
							}

							swp->primary_bank_rearm_time[i] = timestamp( (int)(rearm_time * 1000.f) );
	
							// Actual loading of ballistics is preceded by a sound effect which is the ballistic
							// loading equipment moving into place
							if ( aip->rearm_first_ballistic_primary == TRUE )
							{
								snd_play_3d( &Snds[SND_BALLISTIC_START_LOAD], &objp->pos, &View_position );
								aip->rearm_first_ballistic_primary = FALSE;
							}
							else
							{
								snd_play_3d( &Snds[SND_BALLISTIC_LOAD], &objp->pos, &View_position );

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
		if ( get_shield_strength(objp) >= shipp->ship_initial_shield_strength ) 
			shields_full = 1;
	}

	// return 1 if at end of subsystem list, hull damage at 0, and shields full and all secondary banks full.
//	if ( ((ssp = END_OF_LIST(&shipp->subsys_list)) != NULL )&&(objp->hull_strength == shipp->ship_initial_hull_strength)&&(shields_full) ) {
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
	if ( !is_support_allowed(requester_obj) )
		return NULL;

	num_support_ships = 0;
	num_available_support_ships = 0;

	requester_ship = &Ships[requester_obj->instance];
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ((objp->type == OBJ_SHIP) && !(objp->flags & OF_SHOULD_BE_DEAD)) {
			ship			*shipp;
			ship_info	*sip;
			float			dist;

			Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

			shipp = &Ships[objp->instance];
			sip = &Ship_info[shipp->ship_info_index];

			if ( shipp->team != requester_ship->team )
				continue;

			if ( !(sip->flags & SIF_SUPPORT) )
				continue;

			// don't deal with dying support ships
			if ( shipp->flags & (SF_DYING | SF_DEPARTING) )
				continue;

			dist = vm_vec_dist_quick(&objp->pos, &requester_obj->pos);
			support_ships[num_support_ships] = objp-Objects;

			if (!(Ai_info[shipp->ai_index].ai_flags & AIF_REPAIRING)) {
				num_available_support_ships++;
				if (dist < min_dist) {
					min_dist = dist;
					nearest_support_ship = objp;
				}
			}

			if ( num_support_ships >= MAX_SUPPORT_SHIPS_PER_TEAM ) {
				mprintf(("Why is there more than %d support ships in this mission?\n",MAX_SUPPORT_SHIPS_PER_TEAM));
				break;
			} else {
				support_ships[num_support_ships] = OBJ_INDEX(objp);
				num_support_ships++;
			}
		}
	}

	if (nearest_support_ship != NULL)
		return nearest_support_ship;
	else if (num_support_ships >= MAX_SUPPORT_SHIPS_PER_TEAM) {
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
	int i;

	for (i=0; i<MAX_SHIPS; i++ )	{
		if ( Ships[i].shield_integrity != NULL && Ships[i].objnum != -1 ) {
			free( Ships[i].shield_integrity );
			Ships[i].shield_integrity = NULL;
		}
	}

	// free memory alloced for subsystem storage
	for ( i = 0; i < Num_ship_types; i++ ) {
		if ( Ship_info[i].subsystems != NULL ) {
			free(Ship_info[i].subsystems);
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
	vector engine_pos;
	ship_subsys *moveup;

	Assert( sp->objnum >= 0 );
	if(sp->objnum < 0){
		return;
	}
	objp = &Objects[sp->objnum];
	sip = &Ship_info[sp->ship_info_index];

	if ( sip->engine_snd != -1 ) {
		vm_vec_copy_scale(&engine_pos, &objp->orient.vec.fvec, -objp->radius/2.0f);		

#ifndef NO_SOUND			
		obj_snd_assign(sp->objnum, sip->engine_snd, &engine_pos, 1);
#endif
	}

	// if he's got any specific engine subsystems. go for it.	
	moveup = GET_FIRST(&sp->subsys_list);
	while(moveup != END_OF_LIST(&sp->subsys_list)){
		// check the name of the subsystem
#ifndef NO_SOUND			
		if(strstr(moveup->system_info->name, "enginelarge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_LARGE, &moveup->system_info->pnt, 0);
		} else if(strstr(moveup->system_info->name, "enginehuge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_HUGE, &moveup->system_info->pnt, 0);
		}
#endif

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
			set_shield_strength(Player_obj, Dc_arg_float * Player_ship->ship_initial_shield_strength);
			dc_printf("Shields set to %.2f\n", get_shield_strength(Player_obj) );
		}
	}

	if ( Dc_help ) {
		dc_printf ("Usage: set_shield [num]\n");
		dc_printf ("[num] --  shield percentage 0.0 -> 1.0 of max\n");
		dc_printf ("with no parameters, displays shield strength\n");
		Dc_status = 0;
	}

	if ( Dc_status )	{
		dc_printf( "Shields are currently %.2f", get_shield_strength(Player_obj) );
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
			Player_obj->hull_strength = Dc_arg_float * Player_ship->ship_initial_hull_strength;
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
			} 
		} else if ( !stricmp( Dc_arg, "engine" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_ENGINE, Dc_arg_float );
				if ( Dc_arg_float < ENGINE_MIN_STR )	{
					Player_ship->flags |= SF_DISABLED;				// add the disabled flag
				} else {
					Player_ship->flags &= (~SF_DISABLED);				// add the disabled flag
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
DCF_BOOL( auto_repair, Ship_auto_repair );
#endif

// two functions to keep track of counting ships of particular types.  Maybe we should be rolling this
// thing into the stats section??  The first function adds a ship of a particular type to the overall
// count of ships of that type (called from MissionParse.cpp).  The second function adds to the kill total
// of ships of a particular type.  Note that we use the ship_info flags structure member to determine
// what is happening.
void ship_add_ship_type_count( int ship_info_flag, int num )
{
	if ( ship_info_flag & SIF_CARGO )
		Ship_counts[SHIP_TYPE_CARGO].total += num;
	else if ( (ship_info_flag & SIF_FIGHTER) || (ship_info_flag & SIF_BOMBER) )
		Ship_counts[SHIP_TYPE_FIGHTER_BOMBER].total += num;
	else if ( ship_info_flag & SIF_CRUISER )
		Ship_counts[SHIP_TYPE_CRUISER].total += num;
	else if ( ship_info_flag & SIF_CORVETTE )
		Ship_counts[SHIP_TYPE_CORVETTE].total += num;
	else if ( ship_info_flag & SIF_GAS_MINER )
		Ship_counts[SHIP_TYPE_GAS_MINER].total += num;
	else if ( ship_info_flag & SIF_AWACS )
		Ship_counts[SHIP_TYPE_AWACS].total += num;
	else if ( ship_info_flag & SIF_FREIGHTER )
		Ship_counts[SHIP_TYPE_FREIGHTER].total += num;
	else if ( ship_info_flag & SIF_CAPITAL )
		Ship_counts[SHIP_TYPE_CAPITAL].total += num;
	else if ( ship_info_flag & SIF_TRANSPORT )
		Ship_counts[SHIP_TYPE_TRANSPORT].total += num;
	else if ( ship_info_flag & SIF_SUPPORT )
		Ship_counts[SHIP_TYPE_REPAIR_REARM].total += num;
	else if ( ship_info_flag & SIF_NO_SHIP_TYPE )
		Ship_counts[SHIP_TYPE_NONE].total += num;
	else if ( ship_info_flag & SIF_NAVBUOY ) {
		Ship_counts[SHIP_TYPE_NAVBUOY].total += num;
	} else if ( ship_info_flag & SIF_SENTRYGUN ) {
		Ship_counts[SHIP_TYPE_SENTRYGUN].total += num;
	} else if ( ship_info_flag & SIF_ESCAPEPOD ) {
		Ship_counts[SHIP_TYPE_ESCAPEPOD].total += num;
	} else if ( ship_info_flag & SIF_SUPERCAP ) {
		Ship_counts[SHIP_TYPE_SUPERCAP].total += num;
	} else if ( ship_info_flag & SIF_DRYDOCK ) {
		Ship_counts[SHIP_TYPE_DRYDOCK].total += num;
	} else if ( ship_info_flag & SIF_KNOSSOS_DEVICE){
		Ship_counts[SHIP_TYPE_KNOSSOS_DEVICE].total += num;
	}
	else
		Int3();		//get allender -- unknown ship type
}

void ship_add_ship_type_kill_count( int ship_info_flag )
{
	if ( ship_info_flag & SIF_CARGO )
		Ship_counts[SHIP_TYPE_CARGO].killed++;
	else if ( (ship_info_flag & SIF_FIGHTER) || (ship_info_flag & SIF_BOMBER) )
		Ship_counts[SHIP_TYPE_FIGHTER_BOMBER].killed++;
	else if ( ship_info_flag & SIF_CRUISER )
		Ship_counts[SHIP_TYPE_CRUISER].killed++;
	else if ( ship_info_flag & SIF_CORVETTE )
		Ship_counts[SHIP_TYPE_CORVETTE].killed++;
	else if ( ship_info_flag & SIF_AWACS )
		Ship_counts[SHIP_TYPE_AWACS].killed++;
	else if ( ship_info_flag & SIF_GAS_MINER )
		Ship_counts[SHIP_TYPE_GAS_MINER].killed++;
	else if ( ship_info_flag & SIF_FREIGHTER )
		Ship_counts[SHIP_TYPE_FREIGHTER].killed++;
	else if ( ship_info_flag & SIF_CAPITAL )
		Ship_counts[SHIP_TYPE_CAPITAL].killed++;
	else if ( ship_info_flag & SIF_TRANSPORT )
		Ship_counts[SHIP_TYPE_TRANSPORT].killed++;
	else if ( ship_info_flag & SIF_SUPPORT )
		Ship_counts[SHIP_TYPE_REPAIR_REARM].killed++;
	else if ( ship_info_flag & SIF_SENTRYGUN )
		Ship_counts[SHIP_TYPE_SENTRYGUN].killed++;
	else if ( ship_info_flag & SIF_ESCAPEPOD )
		Ship_counts[SHIP_TYPE_ESCAPEPOD].killed++;
	else if ( ship_info_flag & SIF_NO_SHIP_TYPE )
		Ship_counts[SHIP_TYPE_NONE].killed++;
	else if ( ship_info_flag & SIF_SUPERCAP ) 
		Ship_counts[SHIP_TYPE_SUPERCAP].killed++;
	else if ( ship_info_flag & SIF_DRYDOCK ) 
		Ship_counts[SHIP_TYPE_DRYDOCK].killed++;
	else if ( ship_info_flag & SIF_KNOSSOS_DEVICE )
		Ship_counts[SHIP_TYPE_KNOSSOS_DEVICE].killed++;
	else
		Int3();		//get allender -- unknown ship type
}

int ship_query_general_type(int ship)
{
	return ship_query_general_type(&Ships[ship]);
}

int ship_query_general_type(ship *shipp)
{
	int flags;

	flags = Ship_info[shipp->ship_info_index].flags;
	switch (flags & SIF_ALL_SHIP_TYPES) {
		case SIF_CARGO:
			return SHIP_TYPE_CARGO;

		case SIF_FIGHTER:
		case SIF_BOMBER:
			return SHIP_TYPE_FIGHTER_BOMBER;

		case SIF_CRUISER:
			return SHIP_TYPE_CRUISER;

		case SIF_FREIGHTER:
			return SHIP_TYPE_FREIGHTER;

		case SIF_CAPITAL:
			return SHIP_TYPE_CAPITAL;

		case SIF_TRANSPORT:
			return SHIP_TYPE_TRANSPORT;

		case SIF_NO_SHIP_TYPE:
			return SHIP_TYPE_NONE;

		case SIF_SUPPORT:
			return SHIP_TYPE_REPAIR_REARM;

		case SIF_NAVBUOY:
			return SHIP_TYPE_NAVBUOY;

		case SIF_SENTRYGUN:
			return SHIP_TYPE_SENTRYGUN;

		case SIF_ESCAPEPOD:
			return SHIP_TYPE_ESCAPEPOD;

		case SIF_SUPERCAP:
			return SHIP_TYPE_SUPERCAP;

		case SIF_DRYDOCK:
			return SHIP_TYPE_DRYDOCK;

		case SIF_CORVETTE:
			return SHIP_TYPE_CORVETTE;
		
		case SIF_AWACS:
			return SHIP_TYPE_AWACS;

		case SIF_GAS_MINER:
			return SHIP_TYPE_GAS_MINER;

		case SIF_KNOSSOS_DEVICE:
			return SHIP_TYPE_KNOSSOS_DEVICE;
	}

	Error(LOCATION, "Ship type flag is unknown.  Flags value is 0x%x", flags);
	return SHIP_TYPE_NONE;
}

// returns true if the docker can (is allowed) to dock with dockee
int ship_docking_valid(int docker, int dockee)
{
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
}

// function to return a random ship in a starting player wing.  Returns -1 if a suitable
// one cannot be found
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
// input:   persona  => OPTIONAL PARAMETER (default to -1) which persona to get
int ship_get_random_player_wing_ship( int flags, float max_dist, int persona_index, int get_first, int multi_team )
{
	int i, j, ship_index, count;
	int slist[MAX_SHIPS_PER_WING * MAX_STARTING_WINGS], which_one;

	// iterate through starting wings of player.  Add ship indices of ships which meet
	// given criteria
	count = 0;
	for (i = 0; i < num_wings; i++ ) {
		int wingnum;

		wingnum = -1;

		// multi-team?
		if(multi_team >= 0){
			if(!stricmp(Wings[i].name, multi_team == 0 ? "alpha" : "zeta")){
				wingnum = i;
			} else {
				continue;
			}
		} else {
			// first check for a player starting wing (alpha, beta, gamma)
			for ( j = 0; j < MAX_PLAYER_WINGS; j++ ) {
				if ( i == Starting_wings[j] ) {
					wingnum = i;
					break;
				}
			}

			// if not found, the delta and epsilon count too
			if ( wingnum == -1 ) {
				if ( !stricmp(Wings[i].name, NOX("delta")) || !stricmp(Wings[i].name, NOX("epsilon")) ) {
					wingnum = i;
				}
			}

			if ( wingnum == -1 ){
				continue;
			}
		}

		for ( j = 0; j < Wings[wingnum].current_count; j++ ) {
			ship_index = Wings[wingnum].ship_index[j];
			Assert( ship_index != -1 );

			if ( Ships[ship_index].flags & SF_DYING ) {
				continue;
			}

			// see if ship meets our criterea
			if ( (flags == SHIP_GET_NO_PLAYERS) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) ){
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
		if ( (flags == SHIP_GET_NO_PLAYERS) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) )
			continue;

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
int ship_get_random_team_ship( int team, int flags, float max_dist )
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
		if ( Ships[objp->instance].team != team )
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

	Assert(swp->current_secondary_bank >= 0 && swp->current_secondary_bank < MAX_SECONDARY_BANKS );
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

	Assert(swp->current_primary_bank >= 0 && swp->current_primary_bank < MAX_PRIMARY_BANKS );
	
	return ( primary_out_of_ammo(swp, swp->current_primary_bank) == 0 );
}

// see if there is enough engine power to allow the ship to warp
// returns 1 if ship is able to warp, otherwise return 0
int ship_can_warp(ship *sp)
{
	float	engine_str;

	engine_str = ship_get_subsystem_strength( sp, SUBSYSTEM_ENGINE );
	// Note that ship can always warp at lowest skill level
	if ( (Game_skill_level > 0) && (engine_str >= SHIP_MIN_ENGINES_TO_WARP) ){
		return 1;
	} else {
		return 0;
	}
}


// Calculate the normal vector from a subsystem position and it's first path point
// input:	sp	=>	pointer to ship that is parent of subsystem
//				ss =>	pointer to subsystem of interest
//				norm	=> output parameter... vector from subsys to first path point
//
//	exit:		0	=>	a valid vector was placed in norm
//				!0	=> an path normal could not be calculated
//				
int ship_return_subsys_path_normal(ship *sp, ship_subsys *ss, vector *gsubpos, vector *norm)
{
	if ( ss->system_info->path_num >= 0 ) {
		polymodel	*pm;
		model_path	*mp;
		vector		*path_point;
		vector		gpath_point;
		pm = model_get(sp->modelnum);
		mp = &pm->paths[ss->system_info->path_num];
		if ( mp->nverts >= 2 ) {
//			path_point = &mp->verts[mp->nverts-1].pos;
			path_point = &mp->verts[0].pos;
			// get path point in world coords
			vm_vec_unrotate(&gpath_point, path_point, &Objects[sp->objnum].orient);
			vm_vec_add2(&gpath_point, &Objects[sp->objnum].pos);
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
int ship_subsystem_in_sight(object* objp, ship_subsys* subsys, vector *eye_pos, vector* subsys_pos, int do_facing_check, float *dot_out, vector *vec_out)
{
	float		dist, dot;
	mc_info	mc;
	vector	terminus, eye_to_pos, subsys_fvec, subsys_to_eye_vec;

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

	mc.model_num = Ships[objp->instance].modelnum;			// Fill in the model to check
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
ship_subsys *ship_return_next_subsys(ship *shipp, int type, vector *attacker_pos)
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

// Return the shield strength in the quadrant hit on hit_objp, based on global hitpos
//
// input:	hit_objp	=>	object pointer to ship getting hit
//				hitpos	=> global position of impact
//
// exit:		strength of shields in the quadrant that was hit as a percentage, between 0 and 1.0
//
// Assumes: that hitpos is a valid global hit position
float ship_quadrant_shield_strength(object *hit_objp, vector *hitpos)
{
	int			quadrant_num, i;
	float			max_quadrant;
	vector		tmpv1, tmpv2;

	// If ship doesn't have shield mesh, then return
	if ( hit_objp->flags & OF_NO_SHIELDS ) {
		return 0.0f;
	}

	// Check if all the shield quadrants are all already 0, if so return 0
	for ( i = 0; i < 4; i++ ) {
		if ( hit_objp->shield_quadrant[i] > 0 )
			break;
	}

	if ( i == 4 ) {
		return 0.0f;
	}

	// convert hitpos to position in model coordinates
	vm_vec_sub(&tmpv1, hitpos, &hit_objp->pos);
	vm_vec_rotate(&tmpv2, &tmpv1, &hit_objp->orient);
	quadrant_num = get_quadrant(&tmpv2);
	//nprintf(("Alan","Quadrant hit: %d\n", quadrant_num));

	if ( quadrant_num < 0 )
		quadrant_num = 0;

	max_quadrant = get_max_shield_quad(hit_objp);
	if ( max_quadrant <= 0 ) {
		return 0.0f;
	}

	Assert(hit_objp->shield_quadrant[quadrant_num] <= max_quadrant);

	return hit_objp->shield_quadrant[quadrant_num]/max_quadrant;
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
#ifndef NO_NETWORK
	if ( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) ) {
		return 0;
	}
#endif

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
				strcat(outbuf, XSTR( " Wing", 494));
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

		default:
			return NULL;
	}

	return outbuf;
}

// return the amount of time until ship reaches it's goal (in MM:SS format)
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
	float		min_speed;

	objp = &Objects[sp->objnum];
	aip = &Ai_info[sp->ai_index];

	min_speed = objp->phys_info.speed;

	if ( aip->mode == AIM_WAYPOINTS ) {
		waypoint_list	*wpl;
		min_speed = 0.9f * sp->current_max_speed;
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

		if ( (Objects[sp->objnum].phys_info.speed <= 0) || (sp->current_max_speed <= 0.0f) ) {
			time = -1;
		} else {
			float	speed;

			speed = objp->phys_info.speed;

			if (speed < min_speed)
				speed = min_speed;
			time = fl2i(dist/speed);
		}

	} else if ( (aip->mode == AIM_DOCK) && (aip->submode < AIS_DOCK_4) ) {
		time = hud_support_get_dock_time( OBJ_INDEX(objp) );
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
		if ( (Ship_info[cargo_sp->ship_info_index].flags & SIF_CARGO) && !(cargo_sp->team & TEAM_FRIENDLY) ) {
			
			// If the cargo is revealed, continue on to next hostile cargo
			if ( cargo_sp->flags & SF_CARGO_REVEALED ) {
				goto next_cargo;
			}

			// check against friendly fighter/bombers + cruiser/freighter/transport
			// IDEA: could cull down to fighter/bomber if we want this to run a bit quicker
			for ( ship_so=GET_FIRST(&Ship_obj_list); ship_so != END_OF_LIST(&Ship_obj_list); ship_so=GET_NEXT(ship_so) ) {
				ship_sp = &Ships[Objects[ship_so->objnum].instance];
				// only consider friendly ships
				if ( !(ship_sp->team & TEAM_FRIENDLY) ) {
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
					if ( dist_squared <= max(limit_squared, CARGO_REVEAL_MIN_DIST*CARGO_REVEAL_MIN_DIST) ) {
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
	vector	vec_to_target;
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
#ifndef NO_NETWORK
		if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM) && (Net_player != NULL)){
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS, 0.0f, -1, 0, Net_player->p_info.team );
		} 
		else 
#endif
		{
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS );
		}

		if ( ship_index >= 0 ) {
			// multiplayer - make sure I just send to myself
#ifndef NO_NETWORK
			if(Game_mode & GM_MULTIPLAYER){
				message_send_builtin_to_player(msg_type, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, MY_NET_PLAYER_NUM, -1);
			} 
			else 
#endif
			{
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

	if ( !(Player_ship->team & TEAM_FRIENDLY) ) {
		return;
	}

	if ( deader_sp->team == Player_ship->team ) {	// only praise if killing an enemy!
		return;
	}

	// don't praise the destruction of navbuoys, cargo or other non-flyable ship types
	if ( Ship_info[deader_sp->ship_info_index].flags & SIF_NOT_FLYABLE ) {
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
	if (stricmp(Campaign.filename, "freespace2"))
		return;

	object *objp;
	int message = -1;
	objp = &Objects[sp->objnum];

	if ( objp->hull_strength < ( (AWACS_HELP_HULL_LOW + 0.01f *(static_rand(objp-Objects) & 5)) * sp->ship_initial_hull_strength) ) {
		// awacs ship below 25 + (0-4) %
		if (!(sp->awacs_warning_flag & AWACS_WARN_25)) {
			message = MESSAGE_AWACS_25;
			sp->awacs_warning_flag |=  AWACS_WARN_25;
		}
	} else if ( objp->hull_strength < ( (AWACS_HELP_HULL_HI + 0.01f*(static_rand(objp-Objects) & 5)) * sp->ship_initial_hull_strength) ) {
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
	if ( Player->ask_help_count >= PLAYER_MAX_ASK_HELP ) {
		return;
	}

	// Check if enough time has elapsed since last help request, if not - leave
	if ( !timestamp_elapsed(Player->allow_ask_help_timestamp) ) {
		return;
	}

	if ( !(Player_ship->team & TEAM_FRIENDLY) ) {
		return;
	}

	Assert(sp->team & TEAM_FRIENDLY );
	objp = &Objects[sp->objnum];

	if ( objp->flags & OF_PLAYER_SHIP )	{// don't let the player ask for help!
		return;
	}

#ifndef NO_NETWORK
	// determine team filter if TvT
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
		if(sp->team == TEAM_FRIENDLY){
			multi_team_filter = 0;
		} else if(sp->team == TEAM_HOSTILE){
			multi_team_filter = 1;
		}
	}
#endif

	// handle awacs ship as a special case
	if (Ship_info[sp->ship_info_index].flags & SIF_HAS_AWACS) {
		awacs_maybe_ask_for_help(sp, multi_team_filter);
		return;
	}

	// for now, only have wingman ships request help
	if ( !(sp->flags & SF_FROM_PLAYER_WING) ) {
		return;
	}

	// first check if hull is at a critical level
	if ( objp->hull_strength < ASK_HELP_HULL_PERCENT * sp->ship_initial_hull_strength ) {
		goto play_ask_help;
	}

	// check if shields are near critical level
	if ( objp->flags & OF_NO_SHIELDS ) {
		return;	// no shields on ship, no don't check shield levels
	}

	if ( get_shield_strength(objp) > (ASK_HELP_SHIELD_PERCENT * sp->ship_initial_shield_strength) ) {
		return;
	}

play_ask_help:

	Assert(Ship_info[sp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER) );	// get Alan
	message_send_builtin_to_player(MESSAGE_HELP, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_ask_help_timestamp = timestamp(PLAYER_ASK_HELP_INTERVAL);

	if ( timestamp_until(Player->allow_scream_timestamp) < 15000 ) {
		Player->allow_scream_timestamp = timestamp(15000);	// prevent overlap with death message
	}

	Player->ask_help_count++;
}

// The player has just entered death roll, maybe have wingman mourn the loss of the player
void ship_maybe_lament()
{
	int ship_index;

	// no. because in multiplayer, its funny
	if(Game_mode & GM_MULTIPLAYER){
		return;
	}

	if ( rand()%4 == 0 ) {
		ship_index = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS );
		if ( ship_index >= 0 ) {
			message_send_builtin_to_player(MESSAGE_PLAYED_DIED, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
		}
	}
}

#define PLAYER_SCREAM_INTERVAL		60000
#define PLAYER_MAX_SCREAMS				10

// play a death scream for a ship
void ship_scream(ship *sp)
{
	int multi_team_filter = -1;

	// bogus
	if(sp == NULL){
		return;
	}

#ifndef NO_NETWORK
	// multiplayer tvt
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
		if(sp->team == TEAM_FRIENDLY){
			multi_team_filter = 0;
		} else if(sp->team == TEAM_HOSTILE){
			multi_team_filter = 1;
		}
	}
#endif

	message_send_builtin_to_player(MESSAGE_WINGMAN_SCREAM, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_scream_timestamp = timestamp(PLAYER_SCREAM_INTERVAL);
	Player->scream_count++;
	sp->flags |= SF_SHIP_HAS_SCREAMED;

	// prevent overlap with help messages
	if ( timestamp_until(Player->allow_ask_help_timestamp) < 15000 ) {
		Player->allow_ask_help_timestamp = timestamp(15000);	// prevent overlap with death message
	}
}

// ship has just died, maybe play a scream.
//
// NOTE: this is only called for ships that are in a player wing (and not player ship)
void ship_maybe_scream(ship *sp)
{
	if ( rand()&1 )
		return;

	// First check if the player has reached the maximum number of screams for a mission
	if ( Player->scream_count >= PLAYER_MAX_SCREAMS ) {
		return;
	}

	// if on different teams (i.e. team v. team games in multiplayer), no scream
	if ( sp->team != Player_ship->team ) {
		return;
	}

	// Check if enough time has elapsed since last scream, if not - leave
	if ( !timestamp_elapsed(Player->allow_scream_timestamp) ) {
		return;
	}

	ship_scream(sp);
}

// maybe tell player that we've requested a support ship
#define PLAYER_REQUEST_REPAIR_MSG_INTERVAL	240000
void ship_maybe_tell_about_rearm(ship *sp)
{
	weapon_info *wip;

	if ( !timestamp_elapsed(Player->request_repair_timestamp) ) {
		return;
	}

	if ( !(Player_ship->team & TEAM_FRIENDLY) ) {
		return;
	}

	// AL 1-4-98:	If ship integrity is low, tell player you want to get repaired.  Otherwise, tell
	// the player you want to get re-armed.

	int message_type = -1;
	int heavily_damaged = 0;
	if ( Objects[sp->objnum].hull_strength/sp->ship_initial_hull_strength < 0.4 ) {
		heavily_damaged = 1;
	}

	if ( heavily_damaged || (sp->flags & SF_DISABLED) ) {
		message_type = MESSAGE_REPAIR_REQUEST;
	} else {
		int i;
		ship_weapon *swp;

		swp = &sp->weapons;
		for ( i = 0; i < swp->num_secondary_banks; i++ )
		{
			if (swp->secondary_bank_start_ammo[i] > 0)
			{
				if ( swp->secondary_bank_ammo[i]/swp->secondary_bank_start_ammo[i] < 0.5f )
				{
					message_type = MESSAGE_REARM_REQUEST;
					break;
				}
			}
		}

		// also check ballistic primaries - Goober5000
		if (sp->flags & SIF_BALLISTIC_PRIMARIES)
		{
			for ( i = 0; i < swp->num_primary_banks; i++ )
			{
				wip = &Weapon_info[swp->primary_bank_weapons[i]];

				if (wip->wi_flags2 & WIF2_BALLISTIC)
				{
					if (swp->primary_bank_start_ammo[i] > 0)
					{
						if ( swp->primary_bank_ammo[i]/swp->primary_bank_start_ammo[i] < 0.3f )
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

#ifndef NO_NETWORK
	// multiplayer tvt
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
		if(sp->team == TEAM_FRIENDLY){
			multi_team_filter = 0;
		} else if(sp->team == TEAM_HOSTILE){
			multi_team_filter = 1;
		}
	}
#endif

	if ( message_type >= 0 ) {
		if ( rand() & 1 ) {
			message_send_builtin_to_player(message_type, sp, MESSAGE_PRIORITY_NORMAL, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter);
		}
		Player->request_repair_timestamp = timestamp(PLAYER_REQUEST_REPAIR_MSG_INTERVAL);
	}
}

// The current primary weapon or link status for a ship has changed.. notify clients if multiplayer
//
// input:	sp			=>	pointer to ship that modified primaries
void ship_primary_changed(ship *sp)
{
#if 0
	ship_weapon	*swp;

	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) )
		return;

	Assert(sp);
	swp = &sp->weapons;

	
	if ( MULTIPLAYER_MASTER )
		send_ship_weapon_change( sp, MULTI_PRIMARY_CHANGED, swp->current_primary_bank, (sp->flags & SF_PRIMARY_LINKED)?1:0 );
#endif
}

// The current secondary weapon or dual-fire status for a ship has changed.. notify clients if multiplayer
//
// input:	sp					=>	pointer to ship that modified secondaries
void ship_secondary_changed(ship *sp)
{
#if 0
	ship_weapon	*swp;

	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) ){
		return;
	}

	Assert(sp);
	swp = &sp->weapons;

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

#ifndef NO_NETWORK
	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_revealed_packet( shipp );		
	}
#endif

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
	if ( subsys->subsys_cargo_revealed ) {
		return;
	}

	
	nprintf(("Network", "Revealing cap ship subsys cargo for %s\n", shipp->ship_name));

#ifndef NO_NETWORK
	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_revealed_packet( shipp, subsystem_index );		
	}
#endif

	subsys->subsys_cargo_revealed = 1;
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

#ifndef NO_NETWORK
	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_hidden_packet( shipp );		
	}
#endif

	shipp->flags &= ~SF_CARGO_REVEALED;

	// don't log that the cargo was hidden and don't reset the time cargo revealed
}

void ship_do_cap_subsys_cargo_hidden( ship *shipp, ship_subsys *subsys, int from_network )
{
	// don't do anything if the cargo is already hidden
	if ( !subsys->subsys_cargo_revealed )
	{
		return;
	}

	
	nprintf(("Network", "Hiding cap ship subsys cargo for %s\n", shipp->ship_name));

#ifndef NO_NETWORK
	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_hidden_packet( shipp, subsystem_index );		
	}
#endif

	subsys->subsys_cargo_revealed = 0;

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
	return (int) (capacity / size);
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
	int i,j;
	int num_subsystems_needed = 0;

	int ship_class_used[MAX_SHIP_TYPES];

	// Mark all ship classes as not used
	for (i=0; i<MAX_SHIP_TYPES; i++ )	{
		ship_class_used[i] = 0;
	}

	// Mark any support ship types as used
	// 
	for (i=0; i<Num_ship_types; i++ )	{
		if ( Ship_info[i].flags & SIF_SUPPORT )	{
			nprintf(( "Paging", "Found support ship '%s'\n", Ship_info[i].name ));
			ship_class_used[i]++;

			num_subsystems_needed += Ship_info[i].n_subsystems;
		}
	}
	
	// Mark any ships in the mission as used
	//
	for (i=0; i<MAX_SHIPS; i++)	{
		if (Ships[i].objnum > -1)	{
			nprintf(( "Paging","Found ship '%s'\n", Ships[i].ship_name ));
			ship_class_used[Ships[i].ship_info_index]++;

			num_subsystems_needed += Ship_info[Ships[i].ship_info_index].n_subsystems;
		}
	}

	// Mark any ships that might warp in in the future as used
	//
	p_object * p_objp;
	for( p_objp = GET_FIRST(&ship_arrival_list); p_objp != END_OF_LIST(&ship_arrival_list); p_objp = GET_NEXT(p_objp) )	{
		nprintf(( "Paging","Found future arrival ship '%s'\n", p_objp->name ));
		ship_class_used[p_objp->ship_class]++;

		num_subsystems_needed += Ship_info[p_objp->ship_class].n_subsystems;
	}


	// Page in all the ship classes that are used on this level
	//
	int num_ship_types_used = 0;

	for (i=0; i<MAX_SHIP_TYPES; i++ )	{
		if ( ship_class_used[i]  )	{
			ship_info *sip = &Ship_info[i];

			num_ship_types_used++;

			// Page in the small hud icons for each ship
			hud_ship_icon_page_in(sip);

			// See if this model was previously loaded by another ship
			int model_previously_loaded = -1;
			int ship_previously_loaded = -1;
			for (j=0; j<MAX_SHIP_TYPES; j++ )	{
				if ( (Ship_info[j].modelnum > -1) && !stricmp(sip->pof_file, Ship_info[j].pof_file) )	{
					// Model already loaded
					model_previously_loaded = Ship_info[j].modelnum;
					ship_previously_loaded = j;
					break;
				}
			}

			// If the model is previously loaded...
			if ( model_previously_loaded > -1 )	{

				// If previously loaded model isn't the same ship class...)
				if ( ship_previously_loaded != i )	{

					// update the model number.
					sip->modelnum = model_previously_loaded;

					for ( j = 0; j < sip->n_subsystems; j++ )	{
						sip->subsystems[j].model_num = -1;
					}

					ship_copy_subsystem_fixup(sip);

					#ifndef NDEBUG
						for ( j = 0; j < sip->n_subsystems; j++ )	{
							Assert( sip->subsystems[j].model_num == sip->modelnum );
						}
					#endif

				} else {
					// Just to be safe (I mean to check that my code works...)
					Assert( sip->modelnum > -1 );
					Assert( sip->modelnum == model_previously_loaded );

					#ifndef NDEBUG
						for ( j = 0; j < sip->n_subsystems; j++ )	{
							Assert( sip->subsystems[j].model_num == sip->modelnum );
						}
					#endif
				}
			} else {
				// Model not loaded... so load it and page in its textures
				sip->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

				Assert( sip->modelnum > -1 );

				// Verify that all the subsystem model numbers are updated
				#ifndef NDEBUG
					for ( j = 0; j < sip->n_subsystems; j++ )	{
						Assert( sip->subsystems[j].model_num == sip->modelnum );	// JAS
					}
				#endif
			}
		}
	}

	for (i=0; i<MAX_SHIP_TYPES; i++ )	{
		if ( ship_class_used[i]  )	{
			ship_info *sip = &Ship_info[i];

			if ( sip->modelnum > -1 )	{
				nprintf(( "Paging", "Paging in textures for model '%s'\n", sip->pof_file ));

				ship_page_in_model_textures(sip->modelnum);
			} else {
				nprintf(( "Paging", "Couldn't load model '%s'\n", sip->pof_file ));
			}
				if(strcmp(sip->thruster_bitmap1, "none"))sip->thruster_glow1 = bm_load(sip->thruster_bitmap1);
				else sip->thruster_glow1 = -1;
				if(strcmp(sip->thruster_bitmap1a, "none"))sip->thruster_glow1a = bm_load(sip->thruster_bitmap1a);
				else sip->thruster_glow1a = -1;
				if(strcmp(sip->thruster_bitmap2, "none"))sip->thruster_glow2 = bm_load(sip->thruster_bitmap2);
				else sip->thruster_glow2 = -1;
				if(strcmp(sip->thruster_bitmap2a, "none"))sip->thruster_glow2a = bm_load(sip->thruster_bitmap2a);
				else sip->thruster_glow2a = -1;
				if(strcmp(sip->thruster_bitmap3, "none"))sip->thruster_glow3 = bm_load(sip->thruster_bitmap3);
				else sip->thruster_glow3 = -1;
				if(strcmp(sip->thruster_bitmap3a, "none"))sip->thruster_glow3a = bm_load(sip->thruster_bitmap3a);
				else sip->thruster_glow3a = -1;
				
				if(strcmp(sip->splodeing_texture_name, "none"))sip->splodeing_texture = bm_load(sip->splodeing_texture_name);
				else sip->splodeing_texture = -1;

				int idontcare = 0;
				for(int k = 0; k<sip->n_thruster_particles; k++){
					if(strcmp(sip->normal_thruster_particles[k].thruster_particle_bitmap01_name, "none"))sip->normal_thruster_particles[k].thruster_particle_bitmap01 = bm_load_animation(sip->normal_thruster_particles[k].thruster_particle_bitmap01_name, &sip->normal_thruster_particles[k].thruster_particle_bitmap01_nframes, &idontcare, 1);
					else sip->normal_thruster_particles[k].thruster_particle_bitmap01 = -1;
				}
				for( k = 0; k<sip->n_thruster_particles; k++){
					if(strcmp(sip->afterburner_thruster_particles[k].thruster_particle_bitmap01_name, "none"))sip->afterburner_thruster_particles[k].thruster_particle_bitmap01 = bm_load_animation(sip->afterburner_thruster_particles[k].thruster_particle_bitmap01_name, &sip->afterburner_thruster_particles[k].thruster_particle_bitmap01_nframes, &idontcare, 1);
					else sip->afterburner_thruster_particles[k].thruster_particle_bitmap01 = -1;
				}
		}
	}

	nprintf(( "Paging", "There are %d ship classes used in this mission.\n", num_ship_types_used ));
	mprintf(( "This mission requires %d Ship_subsystems. See #define MAX_SHIP_SUBOBJECTS.\n", num_subsystems_needed ));

	// JAS: If you hit this, then MAX_SHIP_SUBOBJECTS is set too low.
	// I added this code in to detect an error that wasn't getting detected any other
	// way.
	Assert(num_subsystems_needed < MAX_SHIP_SUBOBJECTS );	

	// Page in the thruster effects
	//

	// Make sure thrusters are loaded
	if ( !Thrust_anim_inited )	ship_init_thrusters();

	for ( i = 0; i < NUM_THRUST_ANIMS; i++ ) {
		thrust_anim	*ta = &Thrust_anims[i];
		for ( j = 0; j<ta->num_frames; j++ )	{
			bm_page_in_texture( ta->first_frame + j );
		}
	}

	for ( i = 0; i < NUM_THRUST_GLOW_ANIMS; i++ ) {
		thrust_anim	*ta = &Thrust_glow_anims[i];
		// glows are really not anims
		bm_page_in_texture( ta->first_frame );
	}

	// page in insignia bitmaps
#ifndef NO_NETWORK
	if(Game_mode & GM_MULTIPLAYER){
		for(i=0; i<MAX_PLAYERS; i++){
			if(MULTI_CONNECTED(Net_players[i]) && (Net_players[i].player != NULL) && (Net_players[i].player->insignia_texture >= 0)){
				bm_page_in_xparent_texture(Net_players[i].player->insignia_texture);
			}
		}
	}
	else
#endif
	{
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

	// page in duplicate model textures - Goober5000
	for (i = 0; i < MAX_SHIPS; i++)
	{
		// is this a valid ship?
		if ( Ships[i].objnum != -1)
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

			// do we have a replacement model?
			if ( Ships[i].alt_modelnum != -1 )
			{
				Assert(Ships[i].modelnum == Ships[i].alt_modelnum);

				nprintf(( "Paging", "Paging in textures for model duplicate for ship '%s'\n", Ships[i].ship_name ));
				mprintf(( "Paging in textures for model duplicate for ship '%s'\n", Ships[i].ship_name ));

				ship_page_in_model_textures(Ships[i].alt_modelnum);		
			}
		}
	}


}

// Goober5000 - called from ship_page_in()
void ship_page_in_model_textures(int modelnum)
{
	int i;
	polymodel *pm = model_get(modelnum);
				
	for (i=0; i<pm->n_textures; i++ )
	{
		int bitmap_num = pm->original_textures[i];

		if ( bitmap_num > -1 )
		{
			// see about different kinds of textures... load frames, too, in case we have an ani

			// transparent?
			if (pm->transparent[i])
			{
				bm_page_in_xparent_texture( bitmap_num, pm->num_frames[i] );
			}
			else
			{
				bm_page_in_texture( bitmap_num, pm->num_frames[i] );
			}
		}
	}
}

// function to return true if support ships are allowed in the mission for the given object.
//	In single player, must be friendly and not Shivan.
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

	if ( Game_mode & GM_NORMAL ) {
		if (Ships[objp->instance].team != TEAM_FRIENDLY){
			return 0;
		}

		switch (Ship_info[Ships[objp->instance].ship_info_index].species) {
		case SPECIES_TERRAN:
			break;
		case SPECIES_VASUDAN:
			break;
		case SPECIES_SHIVAN:
			return 0;
		case SPECIES_NONE:
			break;
		}

		return 1;
#ifndef NO_NETWORK
	} else {
		// multiplayer version behaves differently.  Depending on mode:
		// 1) coop mode -- only available to friendly
		// 2) team v team mode -- availble to either side
		// 3) dogfight -- never

		if(Netgame.type_flags & NG_TYPE_DOGFIGHT){
			return 0;
		}

		if ( IS_MISSION_MULTI_COOP ) {
			if ( Ships[objp->instance].team != TEAM_FRIENDLY ){
				return 0;
			}
		}

		return 1;
#endif
	}

}

// return ship index
int ship_get_random_ship()
{
	int num_ships;
	int rand_ship;
	int idx;
	ship_obj *so;

	// get the # of ships on the list
	num_ships = ship_get_num_ships();

	// get a random ship on the list
	rand_ship = (int)frand_range(0.0f, (float)(num_ships - 1));
	if(rand_ship < 0){
		rand_ship = 0;
	} 
	if(rand_ship > num_ships){
		rand_ship = num_ships;
	}

	// find this guy
	so = GET_FIRST(&Ship_obj_list);
	for(idx=0; idx<rand_ship; idx++) {
		so = GET_NEXT(so);
	}

	return Objects[so->objnum].instance;
}

// forcible jettison cargo from a ship
void ship_jettison_cargo(ship *shipp)
{
	object *objp;
	object *cargo_objp;
	vector impulse, pos;

	// make sure we are docked with a valid object
	if(shipp->objnum < 0){
		return;
	}
	objp = &Objects[shipp->objnum];
	if(Ai_info[shipp->ai_index].dock_objnum == -1){
		return;
	}
	if(Objects[Ai_info[shipp->ai_index].dock_objnum].type != OBJ_SHIP){
		Int3();
		return;
	}
	if(Ai_info[Ships[Objects[Ai_info[shipp->ai_index].dock_objnum].instance].ai_index].dock_objnum != OBJ_INDEX(objp)){
		return;
	}
	cargo_objp = &Objects[Ai_info[shipp->ai_index].dock_objnum];

	// undock the objects
	ai_do_objects_undocked_stuff( objp, cargo_objp );
	
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
		damage = Ship_info[shipp->ship_info_index].damage;
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
		outer_rad = Ship_info[Ships[ship_objp->instance].ship_info_index].outer_rad;
	} else {
		outer_rad = (float) atoi(Sexp_variables[Ships[ship_objp->instance].special_exp_index+OUTER_RAD].text);
	}

	return outer_rad;
}

int valid_cap_subsys_cargo_list(char *subsys)
{
	if (strstr(subsys, "nav")
		|| strstr(subsys, "comm")
		|| strstr(subsys, "engines")
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

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
int wing_has_conflicting_teams(int wing_index)
{
	int first_team, idx;

	// sanity checks
	Assert((wing_index >= 0) && (wing_index < num_wings) && (Wings[wing_index].current_count > 0));
	if((wing_index < 0) || (wing_index >= num_wings) || (Wings[wing_index].current_count <= 0)){
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
	p_object *objp;

	// sanity checks
	Assert((r_index >= 0) && (r_index < Num_reinforcements));
	if((r_index < 0) || (r_index >= Num_reinforcements)){
		return -1;
	}

	// if the reinforcement is a ship	
	objp = mission_parse_get_arrival_ship( Reinforcements[r_index].name );
	if(objp != NULL){
		return objp->team;
	}

	// if the reinforcement is a ship
	wing_index = wing_lookup(Reinforcements[r_index].name);
	if(wing_index >= 0){		
		// go through the ship arrival list and find the first ship in this wing
		objp = GET_FIRST(&ship_arrival_list);
		while( objp != END_OF_LIST(&ship_arrival_list) )	{
			// check by wingnum			
			if (objp->wingnum == wing_index) {
				return objp->team;
			}

			// next
			objp = GET_NEXT(objp);
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
	for(idx=0; idx<Num_ship_types; idx++){
		if((Ship_info[idx].modelnum >= 0) && model_find_texture(Ship_info[idx].modelnum, bitmap) == 1){
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
	vector temp, local_hit;
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

			vector temp;
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
int check_world_pt_in_expanded_ship_bbox(vector *world_pt, object *objp, float delta_box)
{
	Assert(objp->type == OBJ_SHIP);

	vector temp, ship_pt;
	polymodel *pm;
	vm_vec_sub(&temp, world_pt, &objp->pos);
	vm_vec_rotate(&ship_pt, &temp, &objp->orient);

	pm = model_get(Ships[objp->instance].modelnum);

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

	int ship_info_index = shipp->ship_info_index;

	// max overclock
	max_speed = Ship_info[ship_info_index].max_overclocked_speed;

	// normal max speed
	max_speed = max(max_speed, Ship_info[ship_info_index].max_vel.xyz.z);

	// afterburn
	max_speed = max(max_speed, Ship_info[ship_info_index].afterburner_max_vel.xyz.z);

	// maybe cap-waypoint-speed has set it higher - Goober5000
	max_speed = max(max_speed, Ai_info[shipp->ai_index].waypoint_speed_cap);

	return max_speed;
}

// determin warp speed of ship
float ship_get_warp_speed(object *objp)
{
	Assert(objp->type == OBJ_SHIP);
	float shipfx_calculate_warp_speed(object *);
	return shipfx_calculate_warp_speed(objp);
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
	if((ship_info_index < 0) || (ship_info_index >= Num_ship_types)){
		return -1;
	}

	// return species
	return Ship_info[ship_info_index].species;
}

// return the length of a ship
float ship_get_length(ship* shipp)
{
	polymodel *pm = model_get(shipp->modelnum);
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

// Goober5000: check ship's iff, currently only called from is-iff in sexp.cpp
int ship_is_iff(int ship_num, int check_team)
{
	Assert(ship_num >= 0 && ship_num < MAX_SHIPS);

	return (Ships[ship_num].team == check_team);
}

// Goober5000: change ship's iff, currently only called from change-iff in sexp.cpp
void ship_change_iff(int ship_num, int new_team)
{
	Assert(ship_num >= 0 && ship_num < MAX_SHIPS);

	Ships[ship_num].team = new_team;

#ifndef NO_NETWORK
	// send a network packet if we need to
	if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Ships[ship_num].objnum >= 0)){
		send_change_iff_packet(Objects[Ships[ship_num].objnum].net_signature, new_team);
	}
#endif
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
				
	pm = model_get( Ships[shipnum].modelnum );
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

//phreak
int ship_fire_tertiary(object *objp)
{
	return 1;
	Assert(objp->type == OBJ_SHIP);
	ship *shipp=&Ships[objp->instance];
	ship_weapon *sw=&shipp->weapons;
	tertiary_weapon_info* twip;
	
	

	Assert(shipp);
	Assert(twip);
	Assert(sw);

	if (sw->tertiary_bank_weapon < 0)
		return 0;

	twip=&Tertiary_weapon_info[sw->tertiary_bank_weapon];

	switch (twip->type)
	{
		case TWT_CLOAK_DEVICE:
		{
			if (shipp->cloak_stage==0)
				shipfx_start_cloak(shipp,twip->cloak_warmup,1,1);

			if (shipp->cloak_stage==2)
				shipfx_stop_cloak(shipp,twip->cloak_cooldown);
		}
	
		case TWT_BOOST_POD:
		{
		//	if (shipp->boost_pod_engaged)
		//	{
		//		return 1;
		//	}

			if (sw->tertiary_bank_ammo==0)
			{
				HUD_printf("No booster shots remaining");
				return 1;
			}	

		//	shipp->boost_pod_engaged=1;
			sw->tertiary_bank_ammo--;
		//	shipp->boost_finish_stamp=timestamp(twip->boost_lifetime);

			objp->phys_info.booster_max_vel.xyz.z=twip->boost_speed;
			objp->phys_info.booster_forward_accel_time_const=twip->boost_acceleration;
			
			objp->phys_info.flags &= ~(PF_AFTERBURNER_ON);
			objp->phys_info.flags |= PF_BOOSTER_ON;

			HUD_printf("Booster engaged.");
		}
	}


	return 1;
}

// Goober5000
void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss)
{
	Assert(shipp);
	Assert(psub);
	Assert(pss);

	// check if we actually can rotate
	if ( !(psub->flags & MSS_FLAG_ROTATES) )
		return;

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


void ship_info_close(){
	for(int i = 0; i<MAX_SHIP_TYPES; i++){
		safe_kill(Ship_info[i].type_str);
		safe_kill(Ship_info[i].maneuverability_str);
		safe_kill(Ship_info[i].armor_str);
		safe_kill(Ship_info[i].manufacturer_str);
		safe_kill(Ship_info[i].desc);
		safe_kill(Ship_info[i].tech_desc);
		safe_kill(Ship_info[i].ship_length);
		safe_kill(Ship_info[i].gun_mounts);
		safe_kill(Ship_info[i].missile_banks);
	}
}