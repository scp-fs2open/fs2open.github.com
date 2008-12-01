/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/Sexp_tree.cpp $
 * $Revision: 1.8.2.13 $
 * $Date: 2007-12-20 01:57:39 $
 * $Author: turey $
 *
 * Sexp tree handler class.  Almost everything is handled by this class.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8.2.12  2007/11/26 18:43:52  karajorma
 * Fix Mantis 0397
 *
 * Revision 1.8.2.11  2007/08/15 06:57:00  Goober5000
 * fix problems with the mini help box
 * (interestingly this feature was committed to stable but not HEAD)
 *
 * Revision 1.8.2.10  2007/05/28 18:27:33  wmcoolmon
 * Added armor support for asteroid, debris, ship, and beam damage
 *
 * Revision 1.8.2.9  2007/05/20 21:24:09  wmcoolmon
 * .
 *
 * Revision 1.8.2.8  2007/05/20 21:21:31  wmcoolmon
 * FRED2 support for numbered SEXP operator arguments, minihelp box, fixed "Insert Event" when no events are present.
 *
 * Revision 1.8.2.7  2007/02/20 04:19:10  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 1.8.2.6  2006/10/28 20:54:35  karajorma
 * Adding the network-variable option to SEXP variables. This change will revert variables to the same behaviour they displayed in retail (i.e they don't update for clients) unless a variable is set to be a network-variable.
 *
 * Revision 1.8.2.5  2006/10/09 05:25:07  Goober5000
 * make sexp nodes dynamic
 *
 * Revision 1.8.2.4  2006/10/09 04:44:49  Goober5000
 * preliminary commit of sexp node bump... fred only, just changing some names for clarity
 *
 * Revision 1.8.2.3  2006/09/13 17:30:25  taylor
 * fix for Mantis bug #1006
 *
 * Revision 1.8.2.2  2006/08/06 19:27:12  Goober5000
 * deprecate change-ship-model
 *
 * Revision 1.8.2.1  2006/07/31 21:09:10  karajorma
 * Fix for Mantis 1020.
 *
 * ship-subsys-guardian-threshold will now include subsystems without you having to type them in.
 *
 * Revision 1.8  2006/03/01 04:01:37  Goober5000
 * fix comm message localization
 *
 * Revision 1.7  2006/02/26 01:32:23  Goober5000
 * bah
 *
 * Revision 1.6  2006/02/26 00:43:09  Goober5000
 * fix subsystems for get-object-*
 *
 * Revision 1.5  2006/02/04 07:05:03  Goober5000
 * fixed several IFF bugs in FRED (plus one or two other bugs)
 * --Goober5000
 *
 * Revision 1.4  2006/02/02 07:00:29  Goober5000
 * consolidated comm order stuff
 * --Goober5000
 *
 * Revision 1.3  2006/02/02 06:22:58  Goober5000
 * replaced Fred_comm_orders with proper Comm_orders, just for WMC ;)
 * --Goober5000
 *
 * Revision 1.2  2006/01/30 06:27:59  taylor
 * dynamic starfield bitmaps
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.114  2006/01/16 00:34:34  Goober5000
 * final (I hope) IFF fixage involving the FRED dialogs :p
 * --Goober5000
 *
 * Revision 1.113  2006/01/15 01:01:34  Goober5000
 * forgot one small thing
 * --Goober5000
 *
 * Revision 1.112  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.111  2006/01/14 05:24:18  Goober5000
 * first pass at converting FRED to use new IFF stuff
 * --Goober5000
 *
 * Revision 1.110  2006/01/08 02:56:02  wmcoolmon
 * Oops,. FRED2 stuff for target order
 *
 * Revision 1.109  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.108  2005/11/01 03:38:39  Goober5000
 * allow variables in modes other than event mode (like briefings and debriefings)
 * --Goober5000
 *
 * Revision 1.107  2005/10/29 22:55:51  Goober5000
 * hmm, looks like this never hit CVS
 * --Goober5000
 *
 * Revision 1.106  2005/10/28 05:47:39  phreak
 * Default values for add-sun-bitmap and add-background-bitmap.
 *
 * Revision 1.105  2005/10/28 05:26:54  phreak
 * some more changes to get the variable arguments correctly working for add-bg/sun-bitmap
 *
 * Revision 1.104  2005/10/24 02:54:15  phreak
 * Get the new nebula argument types working in fred.
 *
 * Revision 1.103  2005/10/23 04:47:08  phreak
 * adjust the variable autofill hack since i need to add the scale information for add-sun-bitmap and add-background-bitmap
 *
 * Revision 1.102  2005/10/23 04:20:04  phreak
 * Have "add-background-bitmap" and "add-sun-bitmap" autofill their variable arguments.
 *
 * Revision 1.101  2005/08/23 07:28:02  Goober5000
 * grammar
 * --Goober5000
 *
 * Revision 1.100  2005/06/29 18:57:58  taylor
 * add support for set-skybox-model sexp
 * don't divide Num_sexp_help again since it should be right from sexp.cpp (was making only the first 30 or so sexps have help)
 *
 * Revision 1.99  2005/05/25 07:26:14  Goober5000
 * fixed karajorma's last FRED bug as well as an inadvertent bug I introduced last commit
 * --Goober5000
 *
 * Revision 1.98  2005/05/25 06:04:17  Goober5000
 * removed some duplicate code and fixed a bug
 * --Goober5000
 *
 * Revision 1.97  2005/05/25 05:09:28  Goober5000
 * fixed a bug that caused some sexps' arguments to revert to their defaults
 * --Goober5000
 *
 * Revision 1.96  2005/05/25 03:58:29  Goober5000
 * fixed a bug where manually entering an invalid sexp caused a crash
 * --Goober5000
 *
 * Revision 1.95  2005/05/18 06:00:01  phreak
 * updates since the Sexp_help structure went into sexp.cpp in the main project.
 *
 * Revision 1.94  2005/04/18 03:15:45  Goober5000
 * made error more friendly
 * --Goober5000
 *
 * Revision 1.93  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.92  2005/03/27 13:01:21  Goober5000
 * stuff for FRED
 * --Goober5000
 *
 * Revision 1.91  2005/03/15 07:35:13  Goober5000
 * sexp help for the new sexps
 * --Goober5000
 *
 * Revision 1.90  2005/03/06 22:43:14  wmcoolmon
 * Fixx0red some of the jump node errors
 *
 * Revision 1.89  2005/01/29 05:40:27  Goober5000
 * regular docking should work again in FRED now (multiple docking still isn't done yet)
 * --Goober5000
 *
 * Revision 1.88  2004/12/23 16:03:01  phreak
 * sexp help for scramble-messages and unscramble-messages
 * -phreak
 *
 * Revision 1.87  2004/12/15 19:16:13  Goober5000
 * FRED code for custom wing names
 * --Goober5000
 *
 * Revision 1.86  2004/11/17 22:24:13  Goober5000
 * the FRED part
 * --Goober5000
 *
 * Revision 1.85  2004/10/15 10:05:19  Goober5000
 * fred help for change-alt-name
 * --Goober5000
 *
 * Revision 1.84  2004/10/15 09:26:17  Goober5000
 * small sexp help update
 * --Goober5000
 *
 * Revision 1.83  2004/10/14 01:20:04  Goober5000
 * more ubersexp bugfixing
 * --Goober5000
 *
 * Revision 1.82  2004/09/22 21:54:30  Goober5000
 * two small fixes
 * --Goober5000
 *
 * Revision 1.81  2004/09/22 21:52:23  Goober5000
 * further work on ubersexp framework
 * --Goober5000
 *
 * Revision 1.80  2004/09/22 08:32:06  Goober5000
 * fixed some small problems in the framework - should be working nicely now;
 * next step is the actual sexp implementation :-P
 * --Goober5000
 *
 * Revision 1.79  2004/09/22 06:57:34  Goober5000
 * FRED framework for when-argument
 * --Goober5000
 *
 * Revision 1.78  2004/09/17 08:07:53  Goober5000
 * a bit of reordering
 * --Goober5000
 *
 * Revision 1.77  2004/09/17 07:14:05  Goober5000
 * fixed FRED stuff for warp effect, etc.
 * --Goober5000
 *
 * Revision 1.76  2004/09/17 00:28:32  Goober5000
 * removed player-not-use-ai and changed player-use-ai to take an argument
 * --Goober5000
 *
 * Revision 1.75  2004/09/17 00:18:52  Goober5000
 * sexp help for new hud stuff
 * --Goober5000
 *
 * Revision 1.74  2004/08/23 04:33:41  Goober5000
 * FRED support for ship-tag
 * --Goober5000
 *
 * Revision 1.73  2004/07/29 01:37:26  Kazan
 * Autopilot SEXPs update
 *
 * Revision 1.72  2004/06/15 20:50:28  wmcoolmon
 * hud-set-color, current-speed, and FRED fixes (Wouldn't compile).
 *
 * Revision 1.71  2004/06/09 00:19:48  wmcoolmon
 * hud-set-color SEXP description
 *
 * Revision 1.70  2004/06/01 07:38:39  wmcoolmon
 * New HUD SEXP stuff.
 *
 * Revision 1.69  2004/05/11 02:16:35  Goober5000
 * accidentally committed this in fs2_open, so I should do it here too
 * --Goober5000
 *
 * Revision 1.68  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.67  2004/01/14 06:28:39  Goober5000
 * made set-support-ship number align with general FS convention
 * --Goober5000
 *
 * Revision 1.66  2003/11/11 21:29:58  Goober5000
 * forgot the FRED one :-P
 * --Goober5000
 *
 * Revision 1.65  2003/10/20 11:49:17  Goober5000
 * added min, max, and avg sexps
 * --Goober5000
 *
 * Revision 1.64  2003/09/30 04:06:43  Goober5000
 * removed orphaned sexp help left over from _argv[-1]'s stuff
 * --Goober5000
 *
 * Revision 1.63  2003/09/11 23:21:53  Goober5000
 * Rolled back _argv[-1]'s changes to the sexp infrastructure, which would
 * have broken a huge amount of stuff and created some nasty bugs.
 * Also commented most of _argv[-1]'s new sexps pending consolidation,
 * and added a placeholder help entry for the one good sexp.
 * --Goober5000
 *
 * Revision 1.62  2003/09/05 10:04:33  Goober5000
 * persistent variable checkboxes in FRED
 * --Goober5000
 *
 * Revision 1.61  2003/09/05 07:43:56  Goober5000
 * fix some compile errors
 * --Goober5000
 *
 * Revision 1.60  2003/09/05 05:06:32  Goober5000
 * merged num-ships-in-battle and num-ships-in-battle-team, making the
 * team argument optional
 * --Goober5000
 *
 * Revision 1.59  2003/09/05 04:53:50  Goober5000
 * merge of Phreak's and my Sexp_tree.cpp
 * --Goober5000
 *
 * Revision 1.58  2003/09/05 02:16:18  phreak
 * added an option to loop sounds played by play-sound-from-file, so i updated the sexp help
 *
 * Revision 1.57  2003/08/30 22:00:49  phreak
 * added sexp help for num-ships-in-battle and num-ships-in-battle-team
 *
 * Revision 1.56  2003/08/28 06:29:17  Goober5000
 * ah, 'tworks :)
 * now fixing up the FRED part
 * basically the last few commits were bugfixes
 * --Goober5000
 *
 * Revision 1.55  2003/08/27 02:04:54  Goober5000
 * added percent-ships-disarmed and percent-ships-disabled
 * --Goober5000
 *
 * Revision 1.54  2003/08/27 01:38:00  Goober5000
 * added is-ship-type, is-ship-class, lock-rotating-subsystem, and free-rotating-subsystem;
 * also fixed the argument and return values for various sexps so that they work
 * properly for negative numbers
 * --Goober5000
 *
 * Revision 1.53  2003/08/21 09:00:20  Goober5000
 * fixed set-support-ship sexp
 * --Goober5000
 *
 * Revision 1.52  2003/06/19 18:13:27  phreak
 * added help for turret-tagged-specific and turret-tagged-clear-specific
 *
 * Revision 1.51  2003/05/24 16:48:49  phreak
 * added help info for Sesquipedalian's kamikaze and not-kamikaze sexps
 *
 * Revision 1.50  2003/04/12 19:09:16  Goober5000
 * fixed an omission that caused a subcategory not to show up
 * --Goober5000
 *
 * Revision 1.49  2003/04/05 20:47:58  Goober5000
 * gotta love those compiler errors ;)
 * fixed those and cleaned up conflicts with the missile-locked sexp
 * --Goober5000
 *
 * Revision 1.48  2003/03/30 21:01:19  Goober5000
 * Ugh.  Fixed a dumb bug where FRED wouldn't check the right subsystem for
 * some sexps.
 * --Goober5000
 *
 * Revision 1.47  2003/03/29 11:24:25  sesquipedalian
 * Aaaand nevermind...
 *
 * Revision 1.46  2003/03/29 08:51:54  sesquipedalian
 * Added is-missile-locked sexp
 *
 * Revision 1.45  2003/03/22 07:24:53  Goober5000
 * bleah
 * --Goober5000
 *
 * Revision 1.44  2003/03/21 08:43:23  Goober5000
 * d'oh
 * --Goober5000
 *
 * Revision 1.43  2003/03/21 08:33:34  Goober5000
 * yuck - fixed a nasty error when FRED provides a default value for warp-effect
 * and explosion-effect
 * --Goober5000
 *
 * Revision 1.42  2003/03/21 04:51:33  Goober5000
 * added get-relative-object-*, where * = x, y, and z; these sexps return the
 * world coordinates of a set of relative coordinates to an object; also, fixed many
 * places in sexp.cpp so that now sexps can accept other sexps as parameters,
 * wherease before they weren't able to
 * --Goober5000
 *
 * Revision 1.41  2003/03/20 09:17:16  Goober5000
 * implemented EMP as part of weapon-effect sexp
 * --Goober5000
 *
 * Revision 1.40  2003/03/20 04:27:10  Goober5000
 * extended sexps
 * --Goober5000
 *
 * Revision 1.39  2003/03/20 00:28:37  Goober5000
 * he doesn't make sexps, he makes them better
 * --Goober5000
 * even though I do make sexps :)
 *
 * Revision 1.38  2003/03/20 00:08:09  Goober5000
 * making sexps better
 * --Goober5000
 *
 * Revision 1.37  2003/03/19 06:27:26  Goober5000
 * sexp help
 * --Goober5000
 *
 * Revision 1.36  2003/03/18 08:45:33  Goober5000
 * added explosion-effect sexp and did some other minor housekeeping
 * --Goober5000
 *
 * Revision 1.35  2003/03/03 04:15:24  Goober5000
 * FRED portion of tech room fix, plus some neatening up of the dialogs
 * --Goober5000
 *
 * Revision 1.34  2003/01/26 20:02:06  Goober5000
 * clarified help for damaged-escort-list and damaged-escort-list-all
 * --Goober5000
 *
 * Revision 1.33  2003/01/26 18:37:19  Goober5000
 * changed change-music to change-soundtrack
 * --Goober5000
 *
 * Revision 1.32  2003/01/25 04:17:39  Goober5000
 * added change-music sexp and bumped MAX_SOUNDTRACKS from 10 to 25
 * --Goober5000
 *
 * Revision 1.31  2003/01/21 17:24:16  Goober5000
 * fixed a few bugs in Bobboau's implementation of the glow sexps; also added
 * help for the sexps in sexp_tree
 * --Goober5000
 *
 * Revision 1.30  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 1.29  2003/01/19 07:45:38  Goober5000
 * actually added the set-support-ship sexp; much of the other commit was
 * groundwork (data types and stuff)
 * --Goober5000
 *
 * Revision 1.28  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 1.27  2003/01/10 04:14:18  Goober5000
 * I found these two beautiful functions in ship.cpp - ship_change_model
 * and change_ship_type - so I made them into sexps :)
 * --Goober5000
 *
 * Revision 1.26  2003/01/07 20:06:44  Goober5000
 * added ai-chase-any-except sexp
 * --Goober5000
 *
 * Revision 1.25  2003/01/05 01:26:35  Goober5000
 * added capability of is-iff and change-iff to have wings as well as ships
 * as their arguments; also allowed a bunch of sexps to accept the player
 * as an argument where they would previously display a parse error
 * --Goober5000
 *
 * Revision 1.24  2003/01/04 23:15:39  Goober5000
 * fixed the order sexp
 * --Goober5000
 *
 * Revision 1.23  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 1.22  2003/01/02 00:35:20  Goober5000
 * added don't-collide-invisible and collide-invisible sexps
 * --Goober5000
 *
 * Revision 1.21  2003/01/01 23:33:33  Goober5000
 * added ship-vaporize and ship-no-vaporize sexps
 * --Goober5000
 *
 * Revision 1.20  2002/12/31 08:20:31  Goober5000
 * simplified end-user implementation of ballistic primaries
 * --Goober5000
 *
 * Revision 1.19  2002/12/27 20:00:11  phreak
 * added damage-escort-list to sexp tree help
 *
 * Revision 1.18  2002/12/27 03:14:47  Goober5000
 * removed the existing stealth sexps and replaced them with the following:
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 1.17  2002/12/25 01:22:23  Goober5000
 * meh - changed is-cargo-x to is-cargo
 * --Goober5000
 *
 * Revision 1.16  2002/12/24 07:42:29  Goober5000
 * added change-ai-class and is-ai-class, and I think I may also have nailed the
 * is-iff bug; did some other bug hunting as well
 * --Goober5000
 *
 * Revision 1.15  2002/12/23 23:01:27  Goober5000
 * added set-cargo and is-cargo-x sexps
 * --Goober5000
 *
 * Revision 1.14  2002/12/23 05:18:52  Goober5000
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
 * Revision 1.13  2002/12/22 21:15:27  Goober5000
 * added primaries-depleted and primary-ammo-pct sexps -- useful for ships with
 * ballistic primaries
 * --Goober5000
 *
 * Revision 1.12  2002/12/22 17:23:38  Goober5000
 * Subcategories implemented. :) So far all that's been done is the Change menu, but other
 * subcategorizations are possible.  Here are the instructions from sexp.h...
 * "Adding more subcategories is possible with the new code.  All that needs to be done is
 * to add a #define here (a number from 0x0000 to 0x00ff ORred with the category that it
 * goes under), some appropriate case statements in get_subcategory() (in sexp.cpp) that
 * will return the subcategory for each sexp that uses it, and the submenu name in the
 * op_submenu[] array in sexp_tree.cpp."
 *
 * Please note that I rearranged a whole bunch of sexps in the Operators[] array in sexp.cpp
 * in order to make the subcategories work better, so if you get a whole bunch of differences
 * or even conflicts, just ignore them. :)
 * --Goober5000
 *
 * Revision 1.11  2002/12/21 17:59:32  Goober5000
 * rearranged the sexp list and got the preliminary subcategories working - still need to work on the actual submenu
 * --Goober5000
 *
 * Revision 1.10  2002/12/17 03:19:22  Goober5000
 * help for set-scanned and set-unscanned
 * --Goober5000
 *
 * Revision 1.9  2002/12/13 21:10:01  Goober5000
 * --fixed an annoying minor bug with distance-ship-subsystem where the sexp tree displayed subsystems for the wrong ship
 * --allowed the "hidden sexps" grant-medal, grant-promotion, was-medal-granted, was-promotion-granted, tech-add-ships, and tech-add-weapons to be selectable
 * --cleaned up grammar in the help structure a bit
 * ~Goober5000~
 *
 * Revision 1.8  2002/12/12 19:16:51  Goober5000
 * corrected /t to be \t
 *
 * Revision 1.7  2002/12/12 07:57:58  Goober5000
 * added help for distance-ship-subsystem sexp; fixed up some grammar and spelling
 * ~Goober5000~
 *
 * Revision 1.6  2002/12/10 04:49:00  righteous1
 * Update:  Port CATAGORY spelling update to FRED2_OPEN.
 * Committer:  R1
 *
 * Revision 1.5  2002/11/28 00:25:57  sesquipedalian
 * fixed end-mission help text error
 *
 * Revision 1.4  2002/11/28 00:01:40  sesquipedalian
 * end-mission sexp added
 *
 * Revision 1.3  2002/10/29 22:42:11  sesquipedalian
 * no message
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:02  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 43    9/07/99 9:22p Jefff
 * added 2 more assignable medals
 * 
 * 42    9/07/99 1:05a Andsager
 * Added team-score sexp for multi team vs team missions
 * 
 * 41    8/27/99 4:07p Andsager
 * Add is-ship-visible sexp.  Make ship-vanish sexp SINGLE player only
 * 
 * 40    8/24/99 4:25p Andsager
 * Add ship-vanish sexp
 * 
 * 39    8/16/99 10:04p Andsager
 * Add special-warp-dist and special-warpout-name sexp for Knossos device
 * warpout.
 * 
 * 38    8/09/99 2:00p Dave
 * 2 new sexpressions.
 * 
 * 37    8/02/99 4:26p Dave
 * Added 2 new sexpressions.
 * 
 * 36    8/02/99 1:43p Andsager
 * format fix
 * 
 * 35    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 34    7/24/99 4:56p Dave
 * Added 3 new sexpressions.
 * 
 * 33    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 32    7/20/99 9:19p Andsager
 * Added facing waypoint sexp
 * 
 * 31    7/20/99 9:54a Andsager
 * Add subsys-set-random sexp
 * 
 * 30    7/19/99 12:02p Andsager
 * Allow AWACS on any ship subsystem. Fix sexp_set_subsystem_strength to
 * only blow up subsystem if its strength is > 0
 * 
 * 29    7/13/99 3:37p Andsager
 * Add secondaries-depleted sexp
 * 
 * 28    7/12/99 12:01p Andsager
 * Make message by default come from command.
 * 
 * 27    7/08/99 12:06p Andsager
 * Add turret-tagged-only and turret-tagged-clear sexp.
 * 
 * 26    6/29/99 10:08a Andsager
 * Add guardian sexp
 * 
 * 25    6/23/99 5:51p Andsager
 * Add waypoint-cap-speed.  Checkin stealth ai - inactive.
 * 
 * 24    6/16/99 10:21a Dave
 * Added send-message-list sexpression.
 * 
 * 23    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 22    5/24/99 11:28a Dave
 * Sexpression for adding/removing ships from the hud escort list.
 * 
 * 21    5/20/99 1:40p Andsager
 * Fix find_text() to only look at nodes that are used.
 * 
 * 20    5/04/99 5:21p Andsager
 * 
 * 19    4/28/99 9:33a Andsager
 * Add turret-free and turret-lock (and -all) sexp.  Stargger start time
 * of beam weapons beam-free and beam-free-all.
 * 
 * 18    4/26/99 2:14p Andsager
 * Add beam-protect-ship and beam-unprotect-ship sexp.
 * 
 * 17    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 16    4/02/99 9:54a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 15    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 14    3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 13    3/01/99 10:00a Dave
 * Fxied several dogfight related stats bugs.
 * 
 * 12    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 11    1/26/99 10:09a Andsager
 * Better checking for modifying/deleting variables
 * 
 * 10    1/25/99 5:16p Andsager
 * Handle change of variable type on modify-variable
 * 
 * 9     1/25/99 8:10a Andsager
 * Add sexp_modify_variable().  Changed syntax checking to allow, adding
 * operator return type ambiguous
 * 
 * 8     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 7     1/20/99 9:02a Andsager
 * Fix bug in verify_and_fix_arguments, where list of strings can have
 * NULL strings.
 * 
 * 6     1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 5     12/17/98 2:39p Andsager
 * Added bitmaps for campaign editor.  Changed input into insert() to
 * include bitmaps
 * 
 * 4     12/17/98 2:34p Andsager
 * new bitmap and dialog for campaign editor
 * 
 * 3     10/13/98 9:27a Dave
 * Started neatening up freespace.h
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 179   9/25/98 1:33p Andsager
 * Add color to event editor (root and chain) indicating mission directive
 * 
 * 178   9/15/98 4:26p Allender
 * added sexpression help for some sexpressions
 * 
 * 177   6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 176   5/21/98 12:58a Hoffoss
 * Fixed warnings optimized build turned up.
 * 
 * 175   5/15/98 6:45p Hoffoss
 * Made some things not appear in the release version of Fred.
 * 
 * 174   5/14/98 10:15a Allender
 * add optional argument to prevous-goal/event operators to specify what
 * sexpression should return when being played as a single mission
 * 
 * 173   5/04/98 10:57a Johnson
 * Fixed bug with labeled roots allowing insert.
 * 
 * 172   4/25/98 7:39p Allender
 * fixd some small hotkey stuff.  Worked on turret orientation being
 * correct for multiplayer.  new sexpression called end-campaign will will
 * end the main campaign
 * 
 * 171   4/23/98 5:49p Hoffoss
 * Added tracking of techroom database list info in pilot files, added
 * sexp to add more to list, made mouse usable on ship listing in tech
 * room.
 * 
 * 170   4/15/98 3:46p Hoffoss
 * Fixed bug with getting a default argument value from an opf listing was
 * utilizing temporary memory that was being destroyed before we were
 * finished with it.
 * 
 * 169   4/14/98 5:46p Hoffoss
 * Added special-check operator.
 * 
 * 168   4/14/98 5:24p Hoffoss
 * Added a custom operator for training handling for Mike K.
 * 
 * 167   4/14/98 4:19p Jim
 * Fixed bug with deleting an argument to an operator that you shouldn't
 * be allowed to.
 * 
 * 166   4/07/98 10:51a Allender
 * remove any allied from message senders.  Make heads for mission
 * specific messages play appropriately
 * 
 * 165   4/03/98 12:17a Allender
 * new sexpression to detect departed or destroyed.  optionally disallow
 * support ships.  Allow docking with escape pods 
 * 
 * 164   3/30/98 2:57p Hoffoss
 * Fixed event listing in campaign editor mode.
 * 
 * 163   3/26/98 3:13p Duncan
 * Fixed bug in goal name listing generation function.  Allender forgot
 * about an assumption being made with them when he used it for
 * invalidate-goal.
 * 
 * 162   3/23/98 2:46p Hoffoss
 * Fixed bug with default argument available for OPF_MESSAGE even when
 * there were no messages, and added "#Command" as a message source to
 * listing.
 * 
 * 161   3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "Sexp_tree.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "Management.h"
#include "parse/sexp.h"
#include "OperatorArgTypeSelect.h"
#include "globalincs/linklist.h"
#include "EventEditor.h"
#include "MissionGoalsDlg.h"
#include "ai/aigoals.h"
#include "mission/missionmessage.h"
#include "mission/missioncampaign.h"
#include "CampaignEditorDlg.h"
#include "hud/hudsquadmsg.h"
#include "IgnoreOrdersDlg.h"
#include "stats/medals.h"
#include "controlconfig/controlsconfig.h"
#include "hud/hudgauges.h"
#include "starfield/starfield.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "jumpnode/jumpnode.h"
#include "AddVariableDlg.h"
#include "ModifyVariableDlg.h"
#include "gamesnd/eventmusic.h"	// for change-soundtrack
#include "menuui/techmenu.h"	// for intel stuff
#include "weapon/emp.h"
#include "gamesnd/gamesnd.h"
#include "weapon/weapon.h"
#include "hud/hudartillery.h"
#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"

#define TREE_NODE_INCREMENT	100

#define MAX_OP_MENUS	30
#define MAX_SUBMENUS	(MAX_OP_MENUS * MAX_OP_MENUS)

#define ID_VARIABLE_MENU	0xda00
#define ID_ADD_MENU			0xdc00
#define ID_REPLACE_MENU		0xde00
// note: stay below 0xe000 so we don't collide with MFC defines..

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//********************sexp_tree********************

BEGIN_MESSAGE_MAP(sexp_tree, CTreeCtrl)
	//{{AFX_MSG_MAP(sexp_tree)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static int Add_count, Replace_count;
static int Modify_variable;

extern sexp_help_struct Sexp_help[];
extern op_menu_struct op_menu[];
extern op_menu_struct op_submenu[];

// constructor
sexp_tree::sexp_tree()
{
	select_sexp_node = -1;
	root_item = -1;
	m_mode = 0;
	m_dragging = FALSE;
	m_p_image_list = NULL;
	help_box = NULL;
	clear_tree();
}

// clears out the tree, so all the nodes are unused.
void sexp_tree::clear_tree(char *op)
{
	mprintf(("Resetting dynamic tree node limit from %d to %d...\n", tree_nodes.size(), 0));

	total_nodes = flag = 0;
	tree_nodes.clear();

	if (op) {
		DeleteAllItems();
		if (strlen(op)) {
			set_node(allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
			build_tree();
		}
	}
}

void sexp_tree::reset_handles()
{
	uint i;

	for (i=0; i<tree_nodes.size(); i++)
		tree_nodes[i].handle = NULL;
}

// initializes and creates a tree from a given sexp startpoint.
void sexp_tree::load_tree(int index, char *deflt)
{
	int cur;

	clear_tree();
	root_item = 0;
	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), deflt);  // setup a default tree if none
		build_tree();
		return;
	}

	if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {  // handle numbers allender likes to use so much..
		cur = allocate_node(-1);
		if (atoi(Sexp_nodes[index].text))
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "true");
		else
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "false");

		build_tree();
		return;
	}

	// assumption: first token is an operator.  I require this because it would cause problems
	// with child/parent relations otherwise, and it should be this way anyway, since the
	// return type of the whole sexp is boolean, and only operators can satisfy this.
	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	load_branch(index, -1);
	build_tree();
}

void get_combined_variable_name(char *combined_name, const char *sexp_var_name)
{
	int sexp_var_index = get_index_sexp_variable_name(sexp_var_name);
	Assert(sexp_var_index > -1);

	sprintf(combined_name, "%s(%s)", Sexp_variables[sexp_var_index].variable_name, Sexp_variables[sexp_var_index].text);
}

// creates a tree from a given Sexp_nodes[] point under a given parent.  Recursive.
// Returns the allocated current node.
int sexp_tree::load_branch(int index, int parent)
{
	int cur = -1;
	char combined_var_name[2*TOKEN_LENGTH + 2];

	while (index != -1) {
		Assert(Sexp_nodes[index].type != SEXP_NOT_USED);
		if (Sexp_nodes[index].subtype == SEXP_ATOM_LIST) {
			load_branch(Sexp_nodes[index].first, parent);  // do the sublist and continue

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR) {
			cur = allocate_node(parent);
			if ((index == select_sexp_node) && !flag) {  // translate sexp node to our node
				select_sexp_node = cur;
				flag = 1;
			}

			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), Sexp_nodes[index].text);
			load_branch(Sexp_nodes[index].rest, cur);  // operator is new parent now
			return cur;  // 'rest' was just used, so nothing left to use.

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {
			cur = allocate_node(parent);
			if (Sexp_nodes[index].type & SEXP_FLAG_VARIABLE) {
				get_combined_variable_name(combined_var_name, Sexp_nodes[index].text);
				set_node(cur, (SEXPT_VARIABLE | SEXPT_NUMBER | SEXPT_VALID), combined_var_name);
			} else {
				set_node(cur, (SEXPT_NUMBER | SEXPT_VALID), Sexp_nodes[index].text);
			}

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_STRING) {
			cur = allocate_node(parent);
			if (Sexp_nodes[index].type & SEXP_FLAG_VARIABLE) {
				get_combined_variable_name(combined_var_name, Sexp_nodes[index].text);
				set_node(cur, (SEXPT_VARIABLE | SEXPT_STRING | SEXPT_VALID), combined_var_name);
			} else {
				set_node(cur, (SEXPT_STRING | SEXPT_VALID), Sexp_nodes[index].text);
			}

		} else
			Assert(0);  // unknown and/or invalid sexp type

		if ((index == select_sexp_node) && !flag) {  // translate sexp node to our node
			select_sexp_node = cur;
			flag = 1;
		}

		index = Sexp_nodes[index].rest;
		if (index == -1)
			return cur;
	}

	return cur;
}

int sexp_tree::query_false(int node)
{
	if (node < 0)
		node = root_item;

	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	if (get_operator_const(tree_nodes[node].text) == OP_FALSE){
		return TRUE;
	}

	return FALSE;
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int sexp_tree::save_tree(int node)
{
	if (node < 0)
		node = root_item;

	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	return save_branch(node);
}

// get variable name from sexp_tree node .text
void var_name_from_sexp_tree_text(char *var_name, const char *text)
{
	int var_name_length = strcspn(text, "(");
	Assert(var_name_length < TOKEN_LENGTH - 1);

	strncpy(var_name, text, var_name_length);
	var_name[var_name_length] = '\0';
}

#define NO_PREVIOUS_NODE -9
// called recursively to save a tree branch and everything under it
int sexp_tree::save_branch(int cur, int at_root)
{
	int start, node = -1, last = NO_PREVIOUS_NODE;
	char var_name_text[TOKEN_LENGTH];

	start = -1;
	while (cur != -1) {
		if (tree_nodes[cur].type & SEXPT_OPERATOR) {
			node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, save_branch(tree_nodes[cur].child));

			if ((tree_nodes[cur].parent >= 0) && !at_root) {
				node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, node, -1);
			}
		} else if (tree_nodes[cur].type & SEXPT_NUMBER) {
			// allocate number, maybe variable
			if (tree_nodes[cur].type & SEXPT_VARIABLE) {
				var_name_from_sexp_tree_text(var_name_text, tree_nodes[cur].text);
				node = alloc_sexp(var_name_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_NUMBER, -1, -1);
			} else {
				node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_NUMBER, -1, -1);
			}
		} else if (tree_nodes[cur].type & SEXPT_STRING) {
			// allocate string, maybe variable
			if (tree_nodes[cur].type & SEXPT_VARIABLE) {
				var_name_from_sexp_tree_text(var_name_text, tree_nodes[cur].text);
				node = alloc_sexp(var_name_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_STRING, -1, -1);
			} else {
				node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_STRING, -1, -1);
			}
		} else if (tree_nodes[cur].type & SEXPT_STRING) {
			Assert( !(tree_nodes[cur].type & SEXPT_VARIABLE) );
			Int3();
		} else {
			Assert(0); // unknown and/or invalid type
		}

		if (last == NO_PREVIOUS_NODE){
			start = node;
		} else if (last >= 0){
			Sexp_nodes[last].rest = node;
		}

		last = node;
		Assert(last != NO_PREVIOUS_NODE);  // should be impossible
		cur = tree_nodes[cur].next;
		if (at_root){
			return start;
		}
	}

	return start;
}

// find the next free tree node and return its index.
int sexp_tree::find_free_node()
{
	int i;

	for (i = 0; i < (int)tree_nodes.size(); i++)
	{
		if (tree_nodes[i].type == SEXPT_UNUSED)
			return i;
	}

	return -1;
}

// allocate a node.  Remains used until freed.
int sexp_tree::allocate_node()
{
	int node = find_free_node();

	// need more tree nodes?
	if (node < 0)
	{
		int old_size = tree_nodes.size();

		Assert(TREE_NODE_INCREMENT > 0);

		// allocate in blocks of TREE_NODE_INCREMENT
		tree_nodes.resize(tree_nodes.size() + TREE_NODE_INCREMENT);

		mprintf(("Bumping dynamic tree node limit from %d to %d...\n", old_size, tree_nodes.size()));

#ifndef NDEBUG
		for (int i = old_size; i < (int)tree_nodes.size(); i++)
		{
			sexp_tree_item *item = &tree_nodes[i];
			Assert(item->type == SEXPT_UNUSED);
		}
#endif

		// our new sexp is the first out of the ones we just created
		node = old_size;
	}

	// reset the new node
	tree_nodes[node].type = SEXPT_UNINIT;
	tree_nodes[node].parent = -1;
	tree_nodes[node].child = -1;
	tree_nodes[node].next = -1;
	tree_nodes[node].flags = 0;
	strcpy(tree_nodes[node].text, "<uninitialized tree node>");
	tree_nodes[node].handle = NULL;

	total_nodes++;
	return node;
}

// allocate a child node under 'parent'.  Appends to end of list.
int sexp_tree::allocate_node(int parent, int after)
{
	int i, index = allocate_node();

	if (parent != -1) {
		i = tree_nodes[parent].child;
		if (i == -1) {
			tree_nodes[parent].child = index;

		} else {
			while ((i != after) && (tree_nodes[i].next != -1))
				i = tree_nodes[i].next;

			tree_nodes[index].next = tree_nodes[i].next;
			tree_nodes[i].next = index;
		}
	}

	tree_nodes[index].parent = parent;
	return index;
}

// free a node and all its children.  Also clears pointers to it, if any.
//   node = node chain to free
//   cascade =  0: free just this node and children under it. (default)
//             !0: free this node and all siblings after it.
//
void sexp_tree::free_node(int node, int cascade)
{
	int i;

	// clear the pointer to node
	i = tree_nodes[node].parent;
	Assert(i != -1);
	if (tree_nodes[i].child == node)
		tree_nodes[i].child = tree_nodes[node].next;

	else {
		i = tree_nodes[i].child;
		while (tree_nodes[i].next != -1) {
			if (tree_nodes[i].next == node) {
				tree_nodes[i].next = tree_nodes[node].next;
				break;
			}

			i = tree_nodes[i].next;
		}
	}

	if (!cascade)
		tree_nodes[node].next = -1;

	// now free up the node and its children
	free_node2(node);
}

// more simple node freer, which works recursively.  It frees the given node and all siblings
// that come after it, as well as all children of these.  Doesn't clear any links to any of
// these freed nodes, so make sure all links are broken first. (i.e. use free_node() if you can)
//
void sexp_tree::free_node2(int node)
{
	Assert(node != -1);
	Assert(tree_nodes[node].type != SEXPT_UNUSED);
	Assert(total_nodes > 0);
	*modified = 1;
	tree_nodes[node].type = SEXPT_UNUSED;
	total_nodes--;
	if (tree_nodes[node].child != -1)
		free_node2(tree_nodes[node].child);

	if (tree_nodes[node].next != -1)
		free_node2(tree_nodes[node].next);
}

// initialize the data for a node.  Should be called right after a new node is allocated.
void sexp_tree::set_node(int node, int type, char *text)
{
	Assert(type != SEXPT_UNUSED);
	Assert(tree_nodes[node].type != SEXPT_UNUSED);
	tree_nodes[node].type = type;
	size_t max_length;
	if (type & SEXPT_VARIABLE) {
		max_length = 2 * TOKEN_LENGTH + 2;
	} else {
		max_length = TOKEN_LENGTH;
	}
	Assert(strlen(text) < max_length);
	strcpy(tree_nodes[node].text, text);
}

void sexp_tree::post_load()
{
	if (!flag)
		select_sexp_node = -1;
}

// build or rebuild a CTreeCtrl object with the current tree data
void sexp_tree::build_tree()
{
	if (!flag)
		select_sexp_node = -1;

	DeleteAllItems();
	add_sub_tree(0, TVI_ROOT);
}

// Create the CTreeCtrl tree from the tree data.  The tree data should already be setup by
// this point.
void sexp_tree::add_sub_tree(int node, HTREEITEM root)
{
//	char str[80];
	int node2;

	Assert(node >= 0 && node < (int)tree_nodes.size());
	node2 = tree_nodes[node].child;

	// check for single argument operator case (prints as one line)
/*	if (node2 != -1 && tree_nodes[node2].child == -1 && tree_nodes[node2].next == -1) {
		sprintf(str, "%s %s", tree_nodes[node].text, tree_nodes[node2].text);
		tree_nodes[node].handle = insert(str, root);
		tree_nodes[node].flags = OPERAND | EDITABLE;
		tree_nodes[node2].flags = COMBINED;
		return;
	}*/

	// bitmap to draw in tree
	int bitmap;

	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		tree_nodes[node].flags = OPERAND;
		bitmap = BITMAP_OPERATOR;
	} else {
		if (tree_nodes[node].type & SEXPT_VARIABLE) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = BITMAP_VARIABLE;
		} else {
			tree_nodes[node].flags = EDITABLE;
			bitmap = get_data_image(node);
		}
	}

	root = tree_nodes[node].handle = insert(tree_nodes[node].text, bitmap, bitmap, root);

	node = node2;
	while (node != -1) {
		Assert(node >= 0 && node < (int)tree_nodes.size());
		Assert(tree_nodes[node].type & SEXPT_VALID);
		if (tree_nodes[node].type & SEXPT_OPERATOR)	{
			add_sub_tree(node, root);

		} else {
			Assert(tree_nodes[node].child == -1);
			if (tree_nodes[node].type & SEXPT_VARIABLE) {
				tree_nodes[node].handle = insert(tree_nodes[node].text, BITMAP_VARIABLE, BITMAP_VARIABLE, root);
				tree_nodes[node].flags = NOT_EDITABLE;
			} else {
				int bmap = get_data_image(node);
				tree_nodes[node].handle = insert(tree_nodes[node].text, bmap, bmap, root);
				tree_nodes[node].flags = EDITABLE;
			}
		}

		node = tree_nodes[node].next;
	}
}

// construct tree nodes for an sexp, adding them to the list and returning first node
int sexp_tree::load_sub_tree(int index, bool valid, char *text)
{
	int cur;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | (valid ? SEXPT_VALID : 0)), text);  // setup a default tree if none
		return cur;
	}

	// assumption: first token is an operator.  I require this because it would cause problems
	// with child/parent relations otherwise, and it should be this way anyway, since the
	// return type of the whole sexp is boolean, and only operators can satisfy this.
	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	cur = load_branch(index, -1);
	return cur;
}

void sexp_tree::setup_selected(HTREEITEM h)
{
	uint i;

	item_index = -1;
	item_handle = h;
	if (!h)
		item_handle = GetSelectedItem();

	for (i=0; i<tree_nodes.size(); i++)
		if (tree_nodes[i].handle == item_handle) {
			item_index = i;
			break;
		}
}

// Goober5000
int get_sexp_id(char *sexp_name)
{
	for (int i = 0; i < Num_operators; i++)
	{
		if (!stricmp(sexp_name, Operators[i].text))
			return Operators[i].value;
	}
	return -1;
}

// Goober5000
int category_of_subcategory(int subcategory_id)
{
	return (subcategory_id & OP_CATEGORY_MASK);
}

// Goober5000
int get_category_id(char *category_name)
{
	for (int i = 0; i < Num_op_menus; i++)
	{
		if (!stricmp(category_name, op_menu[i].name))
		{
			return op_menu[i].id;
		}
	}
	return -1;
}

// Goober5000
int has_submenu(char *category_name)
{
	int category_id = get_category_id(category_name);
	if (category_id != -1)
	{
		for (int i = 0; i < Num_submenus; i++)
		{
			if (category_of_subcategory(op_submenu[i].id) == category_id)
				return 1;
		}
	}
	return 0;
}

// handler for right mouse button clicks.
void sexp_tree::right_clicked(int mode)
{
	char buf[256];
	int i, j, z, count, op, add_type, replace_type, type, subcategory_id;
	sexp_list_item *list;
	UINT _flags;
	HTREEITEM h;
	POINT click_point, mouse;
	CMenu menu, *mptr, *popup_menu, *add_data_menu = NULL, *replace_data_menu = NULL;
	CMenu *add_op_menu, add_op_submenu[MAX_OP_MENUS];
	CMenu *replace_op_menu, replace_op_submenu[MAX_OP_MENUS];
	CMenu *insert_op_menu, insert_op_submenu[MAX_OP_MENUS];
	CMenu *replace_variable_menu = NULL;
	CMenu add_op_subcategory_menu[MAX_SUBMENUS];
	CMenu replace_op_subcategory_menu[MAX_SUBMENUS];
	CMenu insert_op_subcategory_menu[MAX_SUBMENUS];

	m_mode = mode;
	add_instance = replace_instance = -1;
	Assert(Num_operators <= MAX_OPERATORS);
	Assert(Num_op_menus < MAX_OP_MENUS);
	Assert(Num_submenus < MAX_SUBMENUS);

	GetCursorPos(&mouse);
	click_point = mouse;
	ScreenToClient(&click_point);
	h = HitTest(CPoint(click_point), &_flags);  // find out what they clicked on

	if (h && menu.LoadMenu(IDR_MENU_EDIT_SEXP_TREE)) {
		update_help(h);
		popup_menu = menu.GetSubMenu(0);
		ASSERT(popup_menu != NULL);
		//SelectDropTarget(h);  // WTF: Why was this here???

		add_op_menu = replace_op_menu = insert_op_menu = NULL;

		// get pointers to several key popup menus we'll need to modify
		i = popup_menu->GetMenuItemCount();
		while (i--) {
			if ( (mptr = popup_menu->GetSubMenu(i)) > 0 ) {
				popup_menu->GetMenuString(i, buf, sizeof(buf), MF_BYPOSITION);

				if (!stricmp(buf, "add operator")) {
					add_op_menu = mptr;

				} else if (!stricmp(buf, "replace operator")) {
					replace_op_menu = mptr;

				} else if (!stricmp(buf, "add data")) {
					add_data_menu = mptr;

				} else if (!stricmp(buf, "replace data")) {
					replace_data_menu = mptr;

				} else if (!stricmp(buf, "insert operator")) {
					insert_op_menu = mptr;

				} else if (!stricmp(buf, "replace variable")) {
					replace_variable_menu = mptr;
				}
			}
		}

		// add popup menus for all the operator categories
		for (i=0; i<Num_op_menus; i++)
		{
			add_op_submenu[i].CreatePopupMenu();
			replace_op_submenu[i].CreatePopupMenu();
			insert_op_submenu[i].CreatePopupMenu();

			add_op_menu->AppendMenu(MF_POPUP, (UINT) add_op_submenu[i].m_hMenu, op_menu[i].name);
			replace_op_menu->AppendMenu(MF_POPUP, (UINT) replace_op_submenu[i].m_hMenu, op_menu[i].name);
			insert_op_menu->AppendMenu(MF_POPUP, (UINT) insert_op_submenu[i].m_hMenu, op_menu[i].name);
		}

		// get rid of the placeholders we needed to ensure popup menus stayed popup menus,
		// i.e. MSDEV will convert empty popup menus into normal menu items.
		add_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		insert_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_variable_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);

		// get item_index
		item_index = -1;
		for (i=0; i<(int)tree_nodes.size(); i++) {
			if (tree_nodes[i].handle == h) {
				item_index = i;
				break;
			}
		}

		/*
		Goober5000 - allow variables in all modes;
		the restriction seems unnecessary IMHO
		
		// Do SEXP_VARIABLE stuff here.
		if (m_mode != MODE_EVENTS)
		{
			// only allow variables in event mode
			menu.EnableMenuItem(ID_SEXP_TREE_ADD_VARIABLE, MF_GRAYED);
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_GRAYED);
		}
		else
		*/
		{
			menu.EnableMenuItem(ID_SEXP_TREE_ADD_VARIABLE, MF_ENABLED);
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_ENABLED);
			
			// check not root (-1)
			if (item_index >= 0) {
				// get type of sexp_tree item clicked on
				int type = get_type(h);

				int parent = tree_nodes[item_index].parent;
				if (parent >= 0) {
					op = get_operator_index(tree_nodes[parent].text);
					Assert(op >= 0);
					int first_arg = tree_nodes[parent].child;

					// get arg count of item to replace
					Replace_count = 0;
					int temp = first_arg;
					while (temp != item_index) {
						Replace_count++;
						temp = tree_nodes[temp].next;

						// DB - added 3/4/99
						if(temp == -1){
							break;
						}
					}

					int op_type = query_operator_argument_type(op, Replace_count); // check argument type at this position

					// special case dont allow replace data for variable names
					// Goober5000 - why?  the only place this happens is when replacing the ambiguous argument in
					// modify-variable with a variable, which seems legal enough.
					//if (op_type != OPF_AMBIGUOUS) {

						// Goober5000 - given the above, we have to figure out what type this stands for
						if (op_type == OPF_AMBIGUOUS)
						{
							type = tree_nodes[first_arg].type;
						}
						
						if ( (type & SEXPT_STRING) || (type & SEXPT_NUMBER) ) {

							int max_sexp_vars = MAX_SEXP_VARIABLES;
							// prevent collisions in id numbers: ID_VARIABLE_MENU + 512 = ID_ADD_MENU
							Assert(max_sexp_vars < 512);

							for (int idx=0; idx<max_sexp_vars; idx++) {
								if (Sexp_variables[idx].type & SEXP_VARIABLE_SET) {
									// skip block variables
									if (Sexp_variables[idx].type & SEXP_VARIABLE_BLOCK) {
										continue; 
									}

									UINT flag = MF_STRING | MF_GRAYED;
									// maybe gray flag MF_GRAYED

									// get type -- gray "string" or number accordingly
									if ( type & SEXPT_STRING ) {
										if ( Sexp_variables[idx].type & SEXP_VARIABLE_STRING ) {
											flag &= ~MF_GRAYED;
										}
									} else if ( type & SEXPT_NUMBER ) {
										if ( Sexp_variables[idx].type & SEXP_VARIABLE_NUMBER ) {
											flag &= ~MF_GRAYED;
										}
									}

									// if modify-variable and changing variable, enable all variables
									if (op_type == OPF_VARIABLE_NAME) {
										Modify_variable = 1;
										flag &= ~MF_GRAYED;
									} else {
										Modify_variable = 0;
									}

									// enable navsystem always
									if (op_type == OPF_NAV_POINT)
										flag &= ~MF_GRAYED;

									char buf[128];
									// append list of variable names and values
									// set id as ID_VARIABLE_MENU + idx
									sprintf(buf, "%s (%s)", Sexp_variables[idx].variable_name, Sexp_variables[idx].text);

									replace_variable_menu->AppendMenu(flag, (ID_VARIABLE_MENU + idx), buf);
								}
							}
						}
					//}
				}
			}

			// cant modify if no variables
			if (sexp_variable_count() == 0) {
				menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_GRAYED);
			}
		}

		// add all the submenu items first
		for (i=0; i<Num_submenus; i++)
		{
			add_op_subcategory_menu[i].CreatePopupMenu();
			replace_op_subcategory_menu[i].CreatePopupMenu();
			insert_op_subcategory_menu[i].CreatePopupMenu();
			
			for (j=0; j<Num_op_menus; j++)
			{
				if (op_menu[j].id == category_of_subcategory(op_submenu[i].id))
				{
					add_op_submenu[j].AppendMenu(MF_POPUP, (UINT) add_op_subcategory_menu[i].m_hMenu, op_submenu[i].name);
					replace_op_submenu[j].AppendMenu(MF_POPUP, (UINT) replace_op_subcategory_menu[i].m_hMenu, op_submenu[i].name);
					insert_op_submenu[j].AppendMenu(MF_POPUP, (UINT) insert_op_subcategory_menu[i].m_hMenu, op_submenu[i].name);
					break;	// only 1 category valid
				}
			}
		}

		// add operator menu items to the various CATEGORY submenus they belong in
		for (i=0; i<Num_operators; i++)
		{
			// add only if it is not in a subcategory
			subcategory_id = get_subcategory(Operators[i].value);
			if (subcategory_id == -1)
			{
				// put it in the appropriate menu
				for (j=0; j<Num_op_menus; j++)
				{
					if (op_menu[j].id == (Operators[i].value & OP_CATEGORY_MASK))
					{
// Commented out by Goober5000 to allow these sexps to be selectable
/*#ifdef NDEBUG
						switch (Operators[i].value) {
							case OP_WAS_PROMOTION_GRANTED:
							case OP_WAS_MEDAL_GRANTED:
							case OP_GRANT_PROMOTION:
							case OP_GRANT_MEDAL:
							case OP_TECH_ADD_SHIP:
							case OP_TECH_ADD_WEAPON:
							case OP_TECH_ADD_INTEL:
							case OP_TECH_RESET_TO_DEFAULT:
								j = Num_op_menus;  // don't allow these operators in final release
						}
#endif*/
						if (j < Num_op_menus) {
							add_op_submenu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value, Operators[i].text);
							replace_op_submenu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text);
							insert_op_submenu[j].AppendMenu(MF_STRING, Operators[i].value | OP_INSERT_FLAG, Operators[i].text);
						}

						break;	// only 1 category valid
					}
				}
			}
			// if it is in a subcategory, handle it
			else
			{
				// put it in the appropriate submenu
				for (j=0; j<Num_submenus; j++)
				{
					if (op_submenu[j].id == subcategory_id)
					{
						add_op_subcategory_menu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value, Operators[i].text);
						replace_op_subcategory_menu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text);
						insert_op_subcategory_menu[j].AppendMenu(MF_STRING, Operators[i].value | OP_INSERT_FLAG, Operators[i].text);
						break;	// only 1 subcategory valid
					}
				}
			}
		}

		// find local index (i) of current item (from its handle)
		SelectItem(item_handle = h);
		for (i=0; i<(int)tree_nodes.size(); i++) {
			if (tree_nodes[i].handle == h) {
				break;
			}
		}

		// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
		if ((item_index == -1) && (m_mode & ST_LABELED_ROOT)) {
			if (m_mode & ST_ROOT_EDITABLE) {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_ENABLED);
			} else {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
			}

			// disable copy, insert op
			menu.EnableMenuItem(ID_EDIT_COPY, MF_GRAYED);
			for (j=0; j<Num_operators; j++) {
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
			}

			gray_menu_tree(popup_menu);
			popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
			return;
		}

		Assert(item_index != -1);  // handle not found, which should be impossible.
		if (!(tree_nodes[item_index].flags & EDITABLE)) {
			menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
		}

		if (tree_nodes[item_index].parent == -1) {  // root node
			menu.EnableMenuItem(ID_DELETE, MF_GRAYED);  // can't delete the root item.
		}

/*		if ((tree_nodes[item_index].flags & OPERAND) && (tree_nodes[item_index].flags & EDITABLE))  // expandable?
			menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);

		z = tree_nodes[item_index].child;
		if (z != -1 && tree_nodes[z].next == -1 && tree_nodes[z].child == -1)
			menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);

		z = tree_nodes[tree_nodes[item_index].parent].child;
		if (z != -1 && tree_nodes[z].next == -1 && tree_nodes[z].child == -1)
			menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);*/

		// change enabled status of 'add' type menu options.
		add_type = 0;
		if (tree_nodes[item_index].flags & OPERAND)	{
			add_type = OPR_STRING;
			int child = tree_nodes[item_index].child;
			Add_count = count_args(child);
			op = get_operator_index(tree_nodes[item_index].text);
			Assert(op >= 0);

			// get listing of valid argument values and add to menus
			type = query_operator_argument_type(op, Add_count);
			list = get_listing_opf(type, item_index, Add_count);
			if (list) {
				sexp_list_item *ptr;

				int data_idx = 0;
				ptr = list;
				while (ptr) {
					if (ptr->op >= 0) {
						// enable operators with correct return type
						menu.EnableMenuItem(Operators[ptr->op].value, MF_ENABLED);

					} else {
						// add data
						if ( (data_idx + 3) % 30) {
							add_data_menu->AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_MENU + data_idx, ptr->text);
						} else {
							add_data_menu->AppendMenu(MF_MENUBARBREAK | MF_STRING | MF_ENABLED, ID_ADD_MENU + data_idx, ptr->text);
						}
					}

					data_idx++;
					ptr = ptr->next;
				}
			}

			// special handling for the non-string formats
			if (type == OPF_NONE) {  // an argument can't be added
				add_type = 0;

			} else if (type == OPF_NULL) {  // arguments with no return values
				add_type = OPR_NULL;

			// Goober5000
			} else if (type == OPF_FLEXIBLE_ARGUMENT) {
				add_type = OPR_FLEXIBLE_ARGUMENT;
		
			} else if (type == OPF_NUMBER) {  // takes numbers
				add_type = OPR_NUMBER;
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);

			} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
				add_type = OPR_POSITIVE;
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);

			} else if (type == OPF_BOOL) {  // takes true/false bool values
				add_type = OPR_BOOL;

			} else if (type == OPF_AI_GOAL) {
				add_type = OPR_AI_GOAL;
			}

			// add_type unchanged from above
			if (add_type == OPR_STRING) {
				menu.EnableMenuItem(ID_ADD_STRING, MF_ENABLED);
			}

			list->destroy();
		}

		// disable operators that do not have arguments available
		for (j=0; j<Num_operators; j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value, MF_GRAYED);
			}
		}


		// change enabled status of 'replace' type menu options.
		replace_type = 0;
		int parent = tree_nodes[item_index].parent;
		if (parent >= 0) {
			replace_type = OPR_STRING;
			op = get_operator_index(tree_nodes[parent].text);
			Assert(op >= 0);
			int first_arg = tree_nodes[parent].child;
			count = count_args(tree_nodes[parent].child);

			// already at minimum number of arguments?
			if (count <= Operators[op].min) {
				menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
			}

			// get arg count of item to replace
			Replace_count = 0;
			int temp = first_arg;
			while (temp != item_index) {
				Replace_count++;
				temp = tree_nodes[temp].next;

				// DB - added 3/4/99
				if(temp == -1){
					break;
				}
			}

			// maybe gray delete
			for (i=Replace_count+1; i<count; i++) {
				if (query_operator_argument_type(op, i-1) != query_operator_argument_type(op, i)) {
					menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
					break;
				}
			}

			type = query_operator_argument_type(op, Replace_count); // check argument type at this position

			// special case reset type for ambiguous
			if (type == OPF_AMBIGUOUS) {
				type = get_modify_variable_type(parent);
			}

			list = get_listing_opf(type, parent, Replace_count);

			// special case dont allow replace data for variable names
			if ( (type != OPF_VARIABLE_NAME) && list) {
				sexp_list_item *ptr;

				int data_idx = 0;
				ptr = list;
				while (ptr) {
					if (ptr->op >= 0) {
						menu.EnableMenuItem(Operators[ptr->op].value | OP_REPLACE_FLAG, MF_ENABLED);

					} else {
						if ( (data_idx + 3) % 30)
							replace_data_menu->AppendMenu(MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text);
						else
							replace_data_menu->AppendMenu(MF_MENUBARBREAK | MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text);
					}

					data_idx++;
					ptr = ptr->next;
				}
			}

			if (type == OPF_NONE) {  // takes no arguments
				replace_type = 0;

			} else if (type == OPF_NUMBER) {  // takes numbers
				replace_type = OPR_NUMBER;
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);

			} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
				replace_type = OPR_POSITIVE;
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);

			} else if (type == OPF_BOOL) {  // takes true/false bool values
				replace_type = OPR_BOOL;

			} else if (type == OPF_NULL) {  // takes operator that doesn't return a value
				replace_type = OPR_NULL;

			// Goober5000
			} else if (type == OPF_FLEXIBLE_ARGUMENT) {
				replace_type = OPR_FLEXIBLE_ARGUMENT;

			} else if (type == OPF_AI_GOAL) {
				replace_type = OPR_AI_GOAL;
			}

			// default to string
			if (replace_type == OPR_STRING) {
				menu.EnableMenuItem(ID_REPLACE_STRING, MF_ENABLED);
			}

			// modify string or number if (modify_variable)
			if ( !stricmp(Operators[op].text, "modify-variable") ) {
				int modify_type = get_modify_variable_type(parent);

				if (modify_type == OPF_NUMBER) {
					menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
					menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
				}
				// no change for string type
			}

			list->destroy();

		} else {  // top node, so should be a Boolean type.
			if (m_mode == MODE_EVENTS) {  // return type should be null
				replace_type = OPR_NULL;
				for (j=0; j<Num_operators; j++)
					if (query_operator_return_type(j) == OPR_NULL)
						menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_ENABLED);

			} else {
				replace_type = OPR_BOOL;
				for (j=0; j<Num_operators; j++)
					if (query_operator_return_type(j) == OPR_BOOL)
						menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_ENABLED);
			}
		}

		// disable operators that do not have arguments available
		for (j=0; j<Num_operators; j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_GRAYED);
			}
		}


		// change enabled status of 'insert' type menu options.
		z = tree_nodes[item_index].parent;
		Assert(z >= -1);
		if (z != -1) {
			op = get_operator_index(tree_nodes[z].text);
			Assert(op != -1);
			j = tree_nodes[z].child;
			count = 0;
			while (j != item_index) {
				count++;
				j = tree_nodes[j].next;
			}

			type = query_operator_argument_type(op, count); // check argument type at this position

		} else {
			if (m_mode == MODE_EVENTS)
				type = OPF_NULL;
			else
				type = OPF_BOOL;
		}

		for (j=0; j<Num_operators; j++) {
			z = query_operator_return_type(j);
			if (!sexp_query_type_match(type, z) || (Operators[j].min < 1))
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);

			z = query_operator_argument_type(j, 0);
			if ((type == OPF_NUMBER) && (z == OPF_POSITIVE))
				z = OPF_NUMBER;

			// Goober5000's number hack
			if ((type == OPF_POSITIVE) && (z == OPF_NUMBER))
				z = OPF_POSITIVE;

			if (z != type)
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
		}

		// disable operators that do not have arguments available
		for (j=0; j<Num_operators; j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
			}
		}


		// disable non campaign operators if in campaign mode
		for (j=0; j<Num_operators; j++) {
			z = 0;
			if (m_mode == MODE_CAMPAIGN) {
				if (Operators[j].value & OP_NONCAMPAIGN_FLAG)
					z = 1;

			} else {
				if (Operators[j].value & OP_CAMPAIGN_ONLY_FLAG)
					z = 1;
			}

			if (z) {
				menu.EnableMenuItem(Operators[j].value, MF_GRAYED);
				menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_GRAYED);
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
			}
		}

		if ((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED)) {
			Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);

			if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
				j = get_operator_const(CTEXT(Sexp_clipboard));
				Assert(j);
				z = query_operator_return_type(j);

				if ((z == OPR_POSITIVE) && (replace_type == OPR_NUMBER))
					z = OPR_NUMBER;

				// Goober5000's number hack
				if ((z == OPR_NUMBER) && (replace_type == OPR_POSITIVE))
					z = OPR_POSITIVE;

				if (replace_type == z)
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				z = query_operator_return_type(j);
				if ((z == OPR_POSITIVE) && (add_type == OPR_NUMBER))
					z = OPR_NUMBER;

				if (add_type == z)
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
				if ((replace_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				else if (replace_type == OPR_NUMBER)
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				if ((add_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

				else if (add_type == OPR_NUMBER)
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
				if (replace_type == OPR_STRING)
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				if (add_type == OPR_STRING)
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

			} else
				Int3();  // unknown and/or invalid sexp type
		}

		if (!(menu.GetMenuState(ID_DELETE, MF_BYCOMMAND) & MF_GRAYED))
			menu.EnableMenuItem(ID_EDIT_CUT, MF_ENABLED);

		gray_menu_tree(popup_menu);
		popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
	}
}

// counts the number of arguments an operator has.  Call this with the node of the first
// argument of the operator
int sexp_tree::count_args(int node)
{
	int count = 0;

	while (node != -1) {
		count++;
		node = tree_nodes[node].next;
	}

	return count;
}

// identify what type of argument this is.  You call it with the node of the first argument
// of an operator.  It will search through enough of the arguments to determine what type of
// data they are.
int sexp_tree::identify_arg_type(int node)
{
	int type = -1;

	while (node != -1) {
		Assert(tree_nodes[node].type & SEXPT_VALID);
		switch (SEXPT_TYPE(tree_nodes[node].type)) {
			case SEXPT_OPERATOR:
				type = get_operator_const(tree_nodes[node].text);
				Assert(type);
				return query_operator_return_type(type);

			case SEXPT_NUMBER:
				return OPR_NUMBER;

			case SEXPT_STRING:  // either a ship or a wing
				type = SEXP_ATOM_STRING;
				break;  // don't return, because maybe we can narrow selection down more.
		}

		node = tree_nodes[node].next;
	}

	return type;
}

// determine if an item should be editable.  This doesn't actually edit the label.
int sexp_tree::edit_label(HTREEITEM h)
{
	uint i;

	for (i=0; i<tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	// Check if tree root
	if (i == tree_nodes.size()) {
		if (m_mode & ST_ROOT_EDITABLE) {
			return 1;
		}

		return 0;
	}

	// Operators are editable
	if (tree_nodes[i].type & SEXPT_OPERATOR) {
		return 1;
	}

	// Variables must be edited through dialog box
	if (tree_nodes[i].type & SEXPT_VARIABLE) {
		return 0;
	}

	// Don't edit if not flaged as editable
	if (!(tree_nodes[i].flags & EDITABLE)) {
		return 0;
	}

	// Otherwise, allow editing
	return 1;

/*
	if (tree_nodes[i].flags & OPERAND) {
		data = tree_nodes[i].child;

		SetItemText(h, tree_nodes[i].text);
		tree_nodes[i].flags = OPERAND;
		item_handle = tree_nodes[data].handle = insert(tree_nodes[data].text, tree_nodes[data].type, tree_nodes[data].flags, h);
		tree_nodes[data].flags = EDITABLE;
		Expand(h, TVE_EXPAND);
		SelectItem(item_handle);
		return 2;
	}
*/
}

int sexp_tree::end_label_edit(HTREEITEM h, char *str)
{
	int len, r = 1;
	uint node;

	*modified = 1;
	if (!str)
		return 0;

	for (node=0; node<tree_nodes.size(); node++)
		if (tree_nodes[node].handle == h)
			break;

	if (node == tree_nodes.size()) {
		if (m_mode == MODE_EVENTS) {
			item_index = GetItemData(h);
			Assert(Event_editor_dlg);
			node = Event_editor_dlg->handler(ROOT_RENAMED, item_index, str);
			return 1;

		} else
			Int3();  // root labels shouldn't have been editable!
	}

	Assert(node < tree_nodes.size());
	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		str = match_closest_operator(str, node);
		if (!str) return 0;	// Goober5000 - avoids crashing

		SetItemText(h, str);
		item_index = node;
		add_or_replace_operator(get_operator_index(str), 1);
		r = 0;
	}

	// Error checking would not hurt here
	len = strlen(str);
	if (len >= TOKEN_LENGTH)
		len = TOKEN_LENGTH - 1;

	strncpy(tree_nodes[node].text, str, len);
	tree_nodes[node].text[len] = 0;

/*	node = tree_nodes[node].parent;
	if (node != -1) {
		child = tree_nodes[node].child;
		if (child != -1 && tree_nodes[child].next == -1 && tree_nodes[child].child == -1) {
			merge_operator(child);
			return 0;
		}
	}*/

	return r;
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
//
char *sexp_tree::match_closest_operator(char *str, int node)
{
	int z, i, op, arg_num, opf, opr;
	char *sub_best = NULL, *best = NULL;

	z = tree_nodes[node].parent;
	if (z < 0) {
		return str;
	}

	op = get_operator_index(tree_nodes[z].text);
	if (op < 0)
		return str;

	// determine which argument we are of the parent
	arg_num = find_argument_number(z, node); 

	opf = query_operator_argument_type(op, arg_num); // check argument type at this position
	opr = query_operator_return_type(op);
	for (i=0; i<Num_operators; i++) {
		if (sexp_query_type_match(opf, opr)) {
			if ( (stricmp(str, Operators[i].text) <= 0) && (!best || (stricmp(str, best) < 0)) )
				best = Operators[i].text;
			
			if ( !sub_best || (stricmp(Operators[i].text, sub_best) > 0) )
				sub_best = Operators[i].text;
		}
	}

	if (!best)
		best = sub_best;  // no best found, use our plan #2 best found.

	Assert(best);  // we better have some valid operator at this point.
	return best;

/*	char buf[256];
	int child;

	if (tree_nodes[node].flags == EDITABLE)  // data
		node = tree_nodes[node].parent;

	if (node != -1) {
		child = tree_nodes[node].child;
		if (child != -1 && tree_nodes[child].next == -1 && tree_nodes[child].child == -1) {
			sprintf(buf, "%s %s", tree_nodes[node].text, tree_nodes[child].text);
			SetItemText(tree_nodes[node].handle, buf);
			tree_nodes[node].flags = OPERAND | EDITABLE;
			tree_nodes[child].flags = COMBINED;
			DeleteItem(tree_nodes[child].handle);
			tree_nodes[child].handle = NULL;
			return;
		}
	}*/
}

// this really only handles messages generated by the right click popup menu
BOOL sexp_tree::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int i, z, id, node, op, type;
	sexp_list_item *list, *ptr;
	HTREEITEM h;

	if ((item_index >= 0) && (item_index < total_nodes))
		item_handle = tree_nodes[item_index].handle;

	id = LOWORD(wParam);


	// Add variable
	if (id == ID_SEXP_TREE_ADD_VARIABLE) {
		CAddVariableDlg dlg;
		dlg.DoModal();

		if ( dlg.m_create ) {

			// set type
			int type;
			if ( dlg.m_type_number ) {
				type = SEXP_VARIABLE_NUMBER;
			} else {
				type = SEXP_VARIABLE_STRING;
			}

			if ( dlg.m_type_network_variable ) {
				type |= SEXP_VARIABLE_NETWORK;
			}

			if ( dlg.m_type_campaign_persistent ) {
				type |= SEXP_VARIABLE_CAMPAIGN_PERSISTENT;
			} else if ( dlg.m_type_player_persistent ) {
				type |= SEXP_VARIABLE_PLAYER_PERSISTENT;
			}

			// add variable
			sexp_add_variable(dlg.m_default_value, dlg.m_variable_name, type);

			// sort variable
			sexp_variable_sort();
		}
		return 1;
	}

	// Modify variable
	if (id == ID_SEXP_TREE_MODIFY_VARIABLE) {
		CModifyVariableDlg dlg;

		// get sexp_variable index for item index
		dlg.m_start_index = get_item_index_to_var_index();

		// get pointer to tree
		dlg.m_p_sexp_tree = this;

		dlg.DoModal();

		Assert( !(dlg.m_deleted && dlg.m_do_modify) );

		if (dlg.m_deleted) {
			// find index in sexp_variable list
			int sexp_var_index = get_index_sexp_variable_name(dlg.m_cur_variable_name);
			Assert(sexp_var_index != -1);

			// delete from list
			sexp_variable_delete(sexp_var_index);

			// sort list
			sexp_variable_sort();

			// delete from sexp_tree, replacing with "number" or "string" as needed
			// further error checking from add_data()
			delete_sexp_tree_variable(dlg.m_cur_variable_name);

			return 1;
		}

		if (dlg.m_do_modify) {
			// check sexp_tree -- warn on type
			// find index and change either (1) name, (2) type, (3) value
			int sexp_var_index = get_index_sexp_variable_name(dlg.m_old_var_name);
			Assert(sexp_var_index != -1);

			// save old name, since name may be modified
			char old_name[TOKEN_LENGTH];
			strcpy(old_name, Sexp_variables[sexp_var_index].variable_name);

			// set type
			int type;
			if (dlg.m_type_number) {
				type = SEXP_VARIABLE_NUMBER;
			} else {
				type = SEXP_VARIABLE_STRING;
			}

			if ( dlg.m_type_network_variable ) {
				type |= SEXP_VARIABLE_NETWORK;
			}

			if ( dlg.m_type_campaign_persistent ) {
				type |= SEXP_VARIABLE_CAMPAIGN_PERSISTENT;
			} else if ( dlg.m_type_player_persistent ) {
				type |= SEXP_VARIABLE_PLAYER_PERSISTENT;
			}

			// update sexp_variable
			sexp_fred_modify_variable(dlg.m_default_value, dlg.m_cur_variable_name, sexp_var_index, type);

			// modify sexp_tree
			modify_sexp_tree_variable(old_name, sexp_var_index);

			// Don't sort until after modify, since modify uses index
			if (dlg.m_modified_name) {
				sexp_variable_sort();
			}

			return 1;
		}

		// no change
		return 1;
	}


	// check if REPLACE_VARIABLE_MENU
	if ( (id >= ID_VARIABLE_MENU) && (id < ID_VARIABLE_MENU + 511)) {

		Assert(item_index >= 0);

		// get index into list of type valid variables
		int var_idx = id - ID_VARIABLE_MENU;
		Assert( (var_idx >= 0) && (var_idx < MAX_SEXP_VARIABLES) );

		int type = get_type(item_handle);
		Assert( (type & SEXPT_NUMBER) || (type & SEXPT_STRING) );

		// dont do type check for modify-variable
		if (Modify_variable) {
			if (Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_NUMBER;
			} else if (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) {
				type = SEXPT_STRING;
			} else {
				Int3();	// unknown type
			}

		} else {	
			// verify type in tree is same as type in Sexp_variables array
			if (type & SEXPT_NUMBER) {
				Assert(Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER);
			}

			if (type & SEXPT_STRING) {
				Assert( (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) );
			}
		}

		// Replace data
		replace_variable_data(var_idx, (type | SEXPT_VARIABLE));

		return 1;
	}


	if ((id >= ID_ADD_MENU) && (id < ID_ADD_MENU + 511)) {
		Assert(item_index >= 0);
		op = get_operator_index(tree_nodes[item_index].text);
		Assert(op >= 0);

		type = query_operator_argument_type(op, Add_count);
		list = get_listing_opf(type, item_index, Add_count);
		Assert(list);

		id -= ID_ADD_MENU;
		ptr = list;
		while (id) {
			id--;
			ptr = ptr->next;
			Assert(ptr);
		}

		Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
		expand_operator(item_index);
		add_data(ptr->text, ptr->type);
		list->destroy();
		return 1;
	}

	if ((id >= ID_REPLACE_MENU) && (id < ID_REPLACE_MENU + 511)) {
		Assert(item_index >= 0);
		Assert(tree_nodes[item_index].parent >= 0);
		op = get_operator_index(tree_nodes[tree_nodes[item_index].parent].text);
		Assert(op >= 0);

		type = query_operator_argument_type(op, Replace_count); // check argument type at this position
		list = get_listing_opf(type, tree_nodes[item_index].parent, Replace_count);
		Assert(list);

		id -= ID_REPLACE_MENU;
		ptr = list;
		while (id) {
			id--;
			ptr = ptr->next;
			Assert(ptr);
		}

		Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
		expand_operator(item_index);
		replace_data(ptr->text, ptr->type);
		list->destroy();
		return 1;
	}

	for (op=0; op<Num_operators; op++) {
		if (id == Operators[op].value) {
			add_or_replace_operator(op);
			return 1;
		}

		if (id == (Operators[op].value | OP_REPLACE_FLAG)) {
			add_or_replace_operator(op, 1);
			return 1;
		}

		if (id == (Operators[op].value | OP_INSERT_FLAG)) {
			int flags;

			z = tree_nodes[item_index].parent;
			flags = tree_nodes[item_index].flags;
			node = allocate_node(z, item_index);
			set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text);
			tree_nodes[node].flags = flags;
			if (z >= 0)
				h = tree_nodes[z].handle;

			else {
				h = GetParentItem(tree_nodes[item_index].handle);
				if (m_mode == MODE_GOALS) {
					Assert(Goal_editor_dlg);
					Goal_editor_dlg->insert_handler(item_index, node);
					SetItemData(h, node);

				} else if (m_mode == MODE_EVENTS) {
					Assert(Event_editor_dlg);
					Event_editor_dlg->insert_handler(item_index, node);
					SetItemData(h, node);

				} else if (m_mode == MODE_CAMPAIGN) {
					Campaign_tree_formp->insert_handler(item_index, node);
					SetItemData(h, node);

				} else {
					h = TVI_ROOT;
					root_item = node;
				}
			}

			item_handle = tree_nodes[node].handle = insert(Operators[op].text, BITMAP_OPERATOR, BITMAP_OPERATOR, h, tree_nodes[item_index].handle);
			move_branch(item_index, node);

			item_index = node;
			for (i=1; i<Operators[op].min; i++)
				add_default_operator(op, i);

			Expand(item_handle, TVE_EXPAND);
			*modified = 1;
			return 1;
		}
	}

	switch (id) {
		case ID_EDIT_COPY:
			// If a clipboard already exist, unmark it as persistent and free old clipboard
			if (Sexp_clipboard != -1) {
				sexp_unmark_persistent(Sexp_clipboard);
				free_sexp2(Sexp_clipboard);
			}

			// Allocate new clipboard and mark persistent
			Sexp_clipboard = save_branch(item_index, 1);
			sexp_mark_persistent(Sexp_clipboard);
			return 1;

		case ID_EDIT_PASTE:
			// the following assumptions are made..
			Assert((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED));
			Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);

			if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
				expand_operator(item_index);
				replace_operator(CTEXT(Sexp_clipboard));
				if (Sexp_nodes[Sexp_clipboard].rest != -1) {
					load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
					i = tree_nodes[item_index].child;
					while (i != -1) {
						add_sub_tree(i, tree_nodes[item_index].handle);
						i = tree_nodes[i].next;
					}
				}

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
				Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
				expand_operator(item_index);
				replace_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
				Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
				expand_operator(item_index);
				replace_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));

			} else
				Assert(0);  // unknown and/or invalid sexp type

			return 1;

		case ID_EDIT_PASTE_SPECIAL:  // add paste, instead of replace.
			// the following assumptions are made..
			Assert((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED));
			Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);

			if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
				expand_operator(item_index);
				add_operator(CTEXT(Sexp_clipboard));
				if (Sexp_nodes[Sexp_clipboard].rest != -1) {
					load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
					i = tree_nodes[item_index].child;
					while (i != -1) {
						add_sub_tree(i, tree_nodes[item_index].handle);
						i = tree_nodes[i].next;
					}
				}

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
				Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
				expand_operator(item_index);
				add_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
				Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
				expand_operator(item_index);
				add_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));

			} else
				Assert(0);  // unknown and/or invalid sexp type

			return 1;

/*		case ID_SPLIT_LINE:
			if ((tree_nodes[item_index].flags & OPERAND) && (tree_nodes[item_index].flags & EDITABLE))  // expandable?
				expand_operator(item_index);
			else
				merge_operator(item_index);

			return 1;*/

		case ID_EXPAND_ALL:
			expand_branch(item_handle);
			return 1;

		case ID_EDIT_TEXT:
			if (edit_label(item_handle)) {
				*modified = 1;
				EditLabel(item_handle);
			}

			return 1;

		case ID_ADD_STRING:	{
			int node;
			
			node = add_data("string", (SEXPT_STRING | SEXPT_VALID));
			EditLabel(tree_nodes[node].handle);
			return 1;
		}

		case ID_ADD_NUMBER:	{
			int node;

			node = add_data("number", (SEXPT_NUMBER | SEXPT_VALID));
			EditLabel(tree_nodes[node].handle);
			return 1;
		}

		case ID_EDIT_CUT:
			if (Sexp_clipboard != -1) {
				sexp_unmark_persistent(Sexp_clipboard);
				free_sexp2(Sexp_clipboard);
			}

			Sexp_clipboard = save_branch(item_index, 1);
			sexp_mark_persistent(Sexp_clipboard);
			// fall through to ID_DELETE case.

		case ID_DELETE:	{
			int parent, node;
			HTREEITEM h;

			if ((m_mode & ST_ROOT_DELETABLE) && (item_index == -1)) {
				item_index = GetItemData(item_handle);
				if (m_mode == MODE_GOALS) {
					Assert(Goal_editor_dlg);
					node = Goal_editor_dlg->handler(ROOT_DELETED, item_index);

				} else if (m_mode == MODE_EVENTS) {
					Assert(Event_editor_dlg);
					node = Event_editor_dlg->handler(ROOT_DELETED, item_index);

				} else {
					Assert(m_mode == MODE_CAMPAIGN);
					node = Campaign_tree_formp->handler(ROOT_DELETED, item_index);
				}

				Assert(node >= 0);
				free_node2(node);
				DeleteItem(item_handle);
				*modified = 1;
				return 1;
			}

			Assert(item_index >= 0);
			h = GetParentItem(item_handle);
			parent = tree_nodes[item_index].parent;
			if ((parent == -1) && (m_mode == MODE_EVENTS))
				Int3();  // no longer used, temporary to check if called still.

			Assert(parent != -1 && tree_nodes[parent].handle == h);
			free_node(item_index);
			DeleteItem(item_handle);

			node = tree_nodes[parent].child;
/*			if (node != -1 && tree_nodes[node].next == -1 && tree_nodes[node].child == -1) {
				sprintf(buf, "%s %s", tree_nodes[parent].text, tree_nodes[node].text);
				SetItem(h, TVIF_TEXT, buf, 0, 0, 0, 0, 0);
				tree_nodes[parent].flags = OPERAND | EDITABLE;
				tree_nodes[node].flags = COMBINED;
				DeleteItem(tree_nodes[node].handle);
			}*/

			*modified = 1;
			return 1;
		}
	}
	
	return CTreeCtrl::OnCommand(wParam, lParam);
}

// adds to or replaces (based on passed in flag) the current operator
void sexp_tree::add_or_replace_operator(int op, int replace_flag)
{
	int i, op_index, op2;

	op_index = item_index;
	if (replace_flag) {
		if (tree_nodes[item_index].flags & OPERAND) {  // are both operators?
			op2 = get_operator_index(tree_nodes[item_index].text);
			Assert(op2 >= 0);
			i = count_args(tree_nodes[item_index].child);
			if ((i >= Operators[op].min) && (i <= Operators[op].max)) {  // are old num args valid?
				while (i--)
					if (query_operator_argument_type(op2, i) != query_operator_argument_type(op, i))  // does each arg match expected type?
						break;

				if (i < 0) {  // everything is ok, so we can keep old arguments with new operator
					set_node(item_index, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text);
					SetItemText(tree_nodes[item_index].handle, Operators[op].text);
					tree_nodes[item_index].flags = OPERAND;
					return;
				}
			}
		}

		replace_operator(Operators[op].text);

	} else
		add_operator(Operators[op].text);

	// fill in all the required (minimum) arguments with default values
	for (i=0; i<Operators[op].min; i++)
		add_default_operator(op, i);

	Expand(item_handle, TVE_EXPAND);
}

// initialize node, type operator
//
void sexp_list_item::set_op(int op_num)
{
	int i;

	if (op_num >= FIRST_OP) {  // do we have an op value instead of an op number (index)?
		for (i=0; i<Num_operators; i++)
			if (op_num == Operators[i].value)
				op_num = i;  // convert op value to op number
	}

	op = op_num;
	text = Operators[op].text;
	type = (SEXPT_OPERATOR | SEXPT_VALID);
}

// initialize node, type data
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::set_data(char *str, int t)
{
	op = -1;
	text = str;
	type = t;
}

// initialize node, type data, allocating memory for the text
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::set_data_dup(char *str, int t)
{
	op = -1;
	text = strdup(str);
	flags |= SEXP_ITEM_F_DUP;
	type = t;
}

// add a node to end of list
//
void sexp_list_item::add_op(int op_num)
{
	sexp_list_item *item, *ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = item;
	item->set_op(op_num);
}

// add a node to end of list
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::add_data(char *str, int t)
{
	sexp_list_item *item, *ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = item;
	item->set_data(str, t);
}

// add a node to end of list, allocating memory for the text
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::add_data_dup(char *str, int t)
{
	sexp_list_item *item, *ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = item;
	item->set_data(strdup(str), t);
	item->flags |= SEXP_ITEM_F_DUP;
}

// add an sexp list to end of another list (join lists)
//
void sexp_list_item::add_list(sexp_list_item *list)
{
	sexp_list_item *ptr;

	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = list;
}

// free all nodes of list
//
void sexp_list_item::destroy()
{
	sexp_list_item *ptr, *ptr2;

	ptr = this;
	while (ptr) {
		ptr2 = ptr->next;
		if (ptr->flags & SEXP_ITEM_F_DUP)
			free(ptr->text);

		delete ptr;
		ptr = ptr2;
	}
}

int sexp_tree::add_default_operator(int op, int argnum)
{
	char buf[256];
	int index;
	sexp_list_item item;
	HTREEITEM h;

	h = item_handle;
	index = item_index;
	item.text = buf;
	if (get_default_value(&item, op, argnum))
		return -1;

	if (item.type & SEXPT_OPERATOR) {
		Assert((item.op >= 0) && (item.op < Num_operators));
		add_or_replace_operator(item.op);
		item_index = index;
		item_handle = h;

	} else {
		// special case for modify-variable (data added 1st arg is variable)
		//if ( !stricmp(Operators[op].text, "modify-variable") ) {
		if ( query_operator_argument_type(op, argnum) == OPF_VARIABLE_NAME)
		{
			if ((argnum == 0 && !stricmp(Operators[op].text, "modify-variable")) ||
				(argnum == 8 && !stricmp(Operators[op].text, "add-background-bitmap")) ||
				(argnum == 5 && !stricmp(Operators[op].text, "add-sun-bitmap")))
			{

				int sexp_var_index = get_index_sexp_variable_name(item.text);
				Assert(sexp_var_index != -1);
				int type = SEXPT_VALID | SEXPT_VARIABLE;
				if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
					type |= SEXPT_STRING;
				} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
					type |= SEXPT_NUMBER;
				} else {
					Int3();
				}

				char node_text[2*TOKEN_LENGTH + 2];
				sprintf(node_text, "%s(%s)", item.text, Sexp_variables[sexp_var_index].text);
				add_variable_data(node_text, type);
			} else {
				// the the variable name
				char buf2[256];
				Assert(argnum == 1);
				sexp_list_item temp_item;
				temp_item.text = buf2;
				get_default_value(&temp_item, op, 0);
				int sexp_var_index = get_index_sexp_variable_name(temp_item.text);
				Assert(sexp_var_index != -1);

				// from name get type
				int temp_type = Sexp_variables[sexp_var_index].type;
				int type = 0;
				if (temp_type & SEXP_VARIABLE_NUMBER) {
					type = SEXPT_VALID | SEXPT_NUMBER;
				} else if (temp_type & SEXP_VARIABLE_STRING) {
					type = SEXPT_VALID | SEXPT_STRING;
				} else {
					Int3();
				}
				add_data(item.text, type);
			}
		} else {
			add_data(item.text, item.type);
		}
	}

	return 0;
}

int sexp_tree::get_default_value(sexp_list_item *item, int op, int i)
{
	char *str = NULL;
	int type, index;
	sexp_list_item *list;
	HTREEITEM h;

	h = item_handle;
	index = item_index;
	type = query_operator_argument_type(op, i);
	switch (type) {
		case OPF_NULL:
			item->set_op(OP_NOP);
			return 0;

		case OPF_BOOL:
			item->set_op(OP_TRUE);
			return 0;

		case OPF_ANYTHING:
			item->set_data("<any data>");
			return 0;

		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_AMBIGUOUS:
			// if the top level operators is an AI goal, and we are adding the last number required,
			// assume that this number is a priority and make it 89 instead of 1.
			if ((query_operator_return_type(op) == OPR_AI_GOAL) && (i == (Operators[op].min - 1)))
			{
				item->set_data("89", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (((Operators[op].value == OP_HAS_DOCKED_DELAY) || (Operators[op].value == OP_HAS_UNDOCKED_DELAY)) && (i == 2))
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ( (Operators[op].value == OP_SHIP_TYPE_DESTROYED) || (Operators[op].value == OP_GOOD_SECONDARY_TIME) )
			{
				item->set_data("100", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ( (Operators[op].value == OP_SET_SUPPORT_SHIP) )
			{
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ( (Operators[op].value == OP_SHIP_TAG) && (i == 1) )
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ( (Operators[op].value == OP_EXPLOSION_EFFECT) )
			{
				int temp;
				char str[TOKEN_LENGTH];

				switch (i)
				{
					case 3:
						temp = 10;
						break;
					case 4:
						temp = 10;
						break;
					case 5:
						temp = 100;
						break;
					case 6:
						temp = 10;
						break;
					case 7:
						temp = 100;
						break;
					case 10:
						temp = SND_SHIP_EXPLODE_1;
						break;
					case 11:
						temp = (int)EMP_DEFAULT_INTENSITY;
						break;
					case 12:
						temp = (int)EMP_DEFAULT_TIME;
						break;
					default:
						temp = 0;
						break;
				}

				// Goober5000 - set_data_dup is required if we're passing a variable
				sprintf(str, "%d", temp);
				item->set_data_dup(str, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ( (Operators[op].value == OP_WARP_EFFECT) )
			{
				int temp;
				char str[TOKEN_LENGTH];

				switch (i)
				{
					case 6:
						temp = 100;
						break;
					case 7:
						temp = 10;
						break;
					case 8:
						temp = SND_CAPITAL_WARP_IN;
						break;
					case 9:
						temp = SND_CAPITAL_WARP_OUT;
						break;
					default:
						temp = 0;
						break;
				}

				// Goober5000 - set_data_dup is required if we're passing a variable
				sprintf(str, "%d", temp);
				item->set_data_dup(str, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_ADD_BACKGROUND_BITMAP))
			{
				int temp = 0;
				char str[TOKEN_LENGTH];

				switch (i)
				{
					case 4:
					case 5:
						temp = 100;
						break;

					case 6:
					case 7:
						temp = 1;
						break;
				}

				sprintf(str, "%d", temp);
				item->set_data_dup(str, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_ADD_SUN_BITMAP))
			{
				int temp = 0;
				char str[TOKEN_LENGTH];

				if (i==4) temp = 100;

				sprintf(str, "%d", temp);
				item->set_data_dup(str, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_MODIFY_VARIABLE)) {
				if (get_modify_variable_type(index) == OPF_NUMBER) {
					item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
				}
				else {					
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
				}
			}
			else
			{
				item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
			}

			return 0;
	}

	list = get_listing_opf(type, index, i);

	// Goober5000 - the way this is done is really stupid, so stupid hacks are needed to deal with it
	// this particular hack is necessary because the argument string should never be a default
	if (list && !strcmp(list->text, SEXP_ARGUMENT_STRING))
	{
		sexp_list_item *first_ptr;

		first_ptr = list;
		list = list->next;

		if (first_ptr->flags & SEXP_ITEM_F_DUP)
			free(first_ptr->text);

		delete first_ptr;
	}

	if (list) {
		char *ptr;

		ptr = item->text;
		*item = *list;
		item->text = ptr;
		strcpy(item->text, list->text);

		list->destroy();
		return 0;
	}

	// catch anything that doesn't have a default value.  Just describe what should be here instead
	switch (type) {
		case OPF_SHIP:
		case OPF_SHIP_NOT_PLAYER:
		case OPF_SHIP_WING:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING_POINT:
			str = "<name of ship here>";
			break;

		case OPF_ORDER_RECIPIENT:
			str = "<all fighters>";
			break;

		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
			str = SEXP_NONE_STRING;
			break;

		case OPF_WING:
			str = "<name of wing here>";
			break;

		case OPF_DOCKER_POINT:
			str = "<docker point>";
			break;

		case OPF_DOCKEE_POINT:
			str = "<dockee point>";
			break;

		case OPF_SUBSYSTEM:
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_SUBSYS_OR_GENERIC:
			str = "<name of subsystem>";
			break;

		case OPF_POINT:
			str = "<waypoint>";
			break;

		case OPF_MESSAGE:
			str = "<Message>";
			break;

		case OPF_WHO_FROM:
			//str = "<any allied>";
			str = "<any wingman>";
			break;
			
		case OPF_WAYPOINT_PATH:
			str = "<waypoint path>";
			break;

		case OPF_MISSION_NAME:
			str = "<mission name>";
			break;

		case OPF_GOAL_NAME:
			str = "<goal name>";
			break;

		case OPF_SHIP_TYPE:
			str = "<ship type here>";
			break;

		case OPF_EVENT_NAME:
			str = "<event name>";
			break;

		case OPF_HUGE_WEAPON:
			str = "<huge weapon type>";
			break;

		case OPF_JUMP_NODE_NAME:
			str = "<Jump node name>";
			break;

		case OPF_NAV_POINT:
			str = "<Nav 1>";
			break;

		case OPF_ANYTHING:
			str = "<any data>";
			break;

		case OPF_PERSONA:
			str = "<persona name>";
			break;

		default:
			str = "<new default required!>";
			break;
	}

	item->set_data(str, (SEXPT_STRING | SEXPT_VALID));
	return 0;
}

int sexp_tree::query_default_argument_available(int op)
{
	int i;

	Assert(op >= 0);
	for (i=0; i<Operators[op].min; i++)
		if (!query_default_argument_available(op, i))
			return 0;

	return 1;
}

int sexp_tree::query_default_argument_available(int op, int i)
{	
	int j, type;
	object *ptr;

	type = query_operator_argument_type(op, i);
	switch (type) {
		case OPF_NONE:
		case OPF_NULL:
		case OPF_BOOL:
		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_IFF:
		case OPF_AI_CLASS:
		case OPF_WHO_FROM:
		case OPF_PRIORITY:
		case OPF_SHIP_TYPE:
		case OPF_SUBSYSTEM:		
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_DOCKER_POINT:
		case OPF_DOCKEE_POINT:
		case OPF_AI_GOAL:
		case OPF_KEYPRESS:
		case OPF_AI_ORDER:
		case OPF_SKILL_LEVEL:
		case OPF_MEDAL_NAME:
		case OPF_WEAPON_NAME:
		case OPF_INTEL_NAME:
		case OPF_SHIP_CLASS_NAME:
		case OPF_HUD_GAUGE_NAME:
		case OPF_HUGE_WEAPON:
		case OPF_JUMP_NODE_NAME:
		case OPF_AMBIGUOUS:
		case OPF_CARGO:
		case OPF_ARRIVAL_LOCATION:
		case OPF_DEPARTURE_LOCATION:
		case OPF_ARRIVAL_ANCHOR_ALL:
		case OPF_SUPPORT_SHIP_CLASS:
		case OPF_SHIP_WITH_BAY:
		case OPF_SOUNDTRACK_NAME:
		case OPF_STRING:
		case OPF_FLEXIBLE_ARGUMENT:
		case OPF_ANYTHING:
		case OPF_SKYBOX_MODEL_NAME:
		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
		case OPF_SUBSYS_OR_GENERIC:
		case OPF_BACKGROUND_BITMAP:
		case OPF_SUN_BITMAP:
		case OPF_NEBULA_STORM_TYPE:
		case OPF_NEBULA_POOF:
		case OPF_TURRET_TARGET_ORDER:
			return 1;

		case OPF_SHIP:
		case OPF_SHIP_NOT_PLAYER:
		case OPF_SHIP_WING:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING_POINT:
		case OPF_ORDER_RECIPIENT:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_WING:
			for (j=0; j<MAX_WINGS; j++)
				if (Wings[j].wave_count)
					return 1;

			return 0;

		case OPF_PERSONA:
			if (Num_personas)
				return 1;
			return 0;
			

		case OPF_POINT:
		case OPF_WAYPOINT_PATH:
			if (Num_waypoint_lists)
				return 1;
			return 0;

		case OPF_MISSION_NAME:
			if (m_mode != MODE_CAMPAIGN) {
				if (!(*Mission_filename))
					return 0;

				return 1;
			}

			if (Campaign.num_missions > 0)
				return 1;

			return 0;

		case OPF_GOAL_NAME: {
			int value;

			value = Operators[op].value;

			if (m_mode == MODE_CAMPAIGN)
				return 1;

			// need to be sure that previous-goal functions are available.  (i.e. we are providing a default argument for them)
			else if ((value == OP_PREVIOUS_GOAL_TRUE) || (value == OP_PREVIOUS_GOAL_FALSE) || (value == OP_PREVIOUS_GOAL_INCOMPLETE) || Num_goals)
				return 1;

			return 0;
		}

		case OPF_EVENT_NAME: {
			int value;

			value = Operators[op].value;
			if (m_mode == MODE_CAMPAIGN)
				return 1;

			// need to be sure that previous-event functions are available.  (i.e. we are providing a default argument for them)
			else if ((value == OP_PREVIOUS_EVENT_TRUE) || (value == OP_PREVIOUS_EVENT_FALSE) || (value == OP_PREVIOUS_EVENT_INCOMPLETE) || Num_mission_events)
				return 1;

			return 0;
		}

		case OPF_MESSAGE:
			if (m_mode == MODE_EVENTS) {
				Assert(Event_editor_dlg);
				if (Event_editor_dlg->current_message_name(0))
					return 1;

			} else {
				if (Num_messages > Num_builtin_messages)
					return 1;
			}

			return 0;

		case OPF_VARIABLE_NAME:
			if (sexp_variable_count() > 0) {
				return 1;
			} else {
				return 0;
			}

		case OPF_NAV_POINT:
			return 1;

		case OPF_SSM_CLASS:
			if (Ssm_info_count > 0)
				return 1;
			else
				return 0;

		default:
			Int3();

	}

	return 0;
}

// expand a combined line (one with an operator and its one argument on the same line) into
// 2 lines.
void sexp_tree::expand_operator(int node)
{
	int data;
	HTREEITEM h;

	if (tree_nodes[node].flags & COMBINED) {
		node = tree_nodes[node].parent;
		Assert((tree_nodes[node].flags & OPERAND) && (tree_nodes[node].flags & EDITABLE));
	}

	if ((tree_nodes[node].flags & OPERAND) && (tree_nodes[node].flags & EDITABLE)) {  // expandable?
		Assert(tree_nodes[node].type & SEXPT_OPERATOR);
		h = tree_nodes[node].handle;
		data = tree_nodes[node].child;
		Assert(data != -1 && tree_nodes[data].next == -1 && tree_nodes[data].child == -1);

		SetItem(h, TVIF_TEXT, tree_nodes[node].text, 0, 0, 0, 0, 0);
		tree_nodes[node].flags = OPERAND;
		int bmap = get_data_image(data);
		tree_nodes[data].handle = insert(tree_nodes[data].text, bmap, bmap, h);
		tree_nodes[data].flags = EDITABLE;
		Expand(h, TVE_EXPAND);
	}
}

// expand a CTreeCtrl branch and all of its children
void sexp_tree::expand_branch(HTREEITEM h)
{
	Expand(h, TVE_EXPAND);
	h = GetChildItem(h);
	while (h) {
		expand_branch(h);
		h = GetNextSiblingItem(h);
	}
}

void sexp_tree::merge_operator(int node)
{
/*	char buf[256];
	int child;

	if (tree_nodes[node].flags == EDITABLE)  // data
		node = tree_nodes[node].parent;

	if (node != -1) {
		child = tree_nodes[node].child;
		if (child != -1 && tree_nodes[child].next == -1 && tree_nodes[child].child == -1) {
			sprintf(buf, "%s %s", tree_nodes[node].text, tree_nodes[child].text);
			SetItemText(tree_nodes[node].handle, buf);
			tree_nodes[node].flags = OPERAND | EDITABLE;
			tree_nodes[child].flags = COMBINED;
			DeleteItem(tree_nodes[child].handle);
			tree_nodes[child].handle = NULL;
			return;
		}
	}*/
}

// add a data node under operator pointed to by item_index
int sexp_tree::add_data(char *data, int type)
{
	int node;

	expand_operator(item_index);
	node = allocate_node(item_index);
	set_node(node, type, data);
	int bmap = get_data_image(node);
	tree_nodes[node].handle = insert(data, bmap, bmap, tree_nodes[item_index].handle);
	tree_nodes[node].flags = EDITABLE;
	*modified = 1;
	return node;
}

// add a (variable) data node under operator pointed to by item_index
int sexp_tree::add_variable_data(char *data, int type)
{
	int node;

	Assert(type & SEXPT_VARIABLE);

	expand_operator(item_index);
	node = allocate_node(item_index);
	set_node(node, type, data);
	tree_nodes[node].handle = insert(data, BITMAP_VARIABLE, BITMAP_VARIABLE, tree_nodes[item_index].handle);
	tree_nodes[node].flags = NOT_EDITABLE;
	*modified = 1;
	return node;
}

// add an operator under operator pointed to by item_index.  Updates item_index to point
// to this new operator.
void sexp_tree::add_operator(char *op, HTREEITEM h)
{
	int node;
	
	if (item_index == -1) {
		node = allocate_node(-1);
		set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		item_handle = tree_nodes[node].handle = insert(op, BITMAP_OPERATOR, BITMAP_OPERATOR, h);

	} else {
		expand_operator(item_index);
		node = allocate_node(item_index);
		set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		item_handle = tree_nodes[node].handle = insert(op, BITMAP_OPERATOR, BITMAP_OPERATOR, tree_nodes[item_index].handle);
	}

	tree_nodes[node].flags = OPERAND;
	item_index = node;
	*modified = 1;
}

// add an operator with one argument under operator pointed to by item_index.  This function
// exists because the one arg case is a special case.  The operator and argument is
// displayed on the same line.
/*void sexp_tree::add_one_arg_operator(char *op, char *data, int type)
{
	char str[80];
	int node1, node2;
	
	expand_operator(item_index);
	node1 = allocate_node(item_index);
	node2 = allocate_node(node1);
	set_node(node1, SEXPT_OPERATOR, op);
	set_node(node2, type, data);
	sprintf(str, "%s %s", op, data);
	tree_nodes[node1].handle = insert(str, tree_nodes[item_index].handle);
	tree_nodes[node1].flags = OPERAND | EDITABLE;
	tree_nodes[node2].flags = COMBINED;
	*modified = 1;
}*/

/*
int sexp_tree::verify_tree(int *bypass)
{
	return verify_tree(0, bypass);
}

// check the sexp tree for errors.  Return -1 if error, or 0 if no errors.  If an error
// is found, item_index = node of error.
int sexp_tree::verify_tree(int node, int *bypass)
{
	int i, type, count, op, type2, op2, argnum = 0;

	if (!total_nodes)
		return 0;  // nothing to check

	Assert(node >= 0 && node < tree_nodes.size());
	Assert(tree_nodes[node].type == SEXPT_OPERATOR);

	op = get_operator_index(tree_nodes[node].text);
	if (op == -1)
		return node_error(node, "Unknown operator", bypass);

	count = count_args(tree_nodes[node].child);
	if (count < Operators[op].min)
		return node_error(node, "Too few arguments for operator", bypass);
	if (count > Operators[op].max)
		return node_error(node, "Too many arguments for operator", bypass);

	node = tree_nodes[node].child;  // get first argument
	while (node != -1) {
		type = query_operator_argument_type(op, argnum);
		Assert(tree_nodes[node].type & SEXPT_VALID);
		if (tree_nodes[node].type == SEXPT_OPERATOR) {
			if (verify_tree(node) == -1)
				return -1;

			op2 = get_operator_index(tree_nodes[node].text);  // no error checking, because it was done in the call above.
			type2 = query_operator_return_type(op2);

		} else if (tree_nodes[node].type == SEXPT_NUMBER) {
			char *ptr;

			type2 = OPR_NUMBER;
			ptr = tree_nodes[node].text;
			while (*ptr)
				if (!isdigit(*ptr++))
					return node_error(node, "Number is invalid", bypass);

		} else if (tree_nodes[node].type == SEXPT_STRING) {
			type2 = SEXP_ATOM_STRING;

		} else
			Assert(0);  // unknown and invalid sexp node type.

		switch (type) {
			case OPF_NUMBER:
				if (type2 != OPR_NUMBER)
					return node_error(node, "Number or number return type expected here", bypass);

				break;

			case OPF_SHIP:
				if (type2 == SEXP_ATOM_STRING)
					if (ship_name_lookup(tree_nodes[node].text, 1) == -1)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Ship name expected here", bypass);

				break;

			case OPF_WING:
				if (type2 == SEXP_ATOM_STRING)
					if (wing_name_lookup(tree_nodes[node].text) == -1)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Wing name expected here", bypass);

				break;

			case OPF_SHIP_WING:
				if (type2 == SEXP_ATOM_STRING)
					if (ship_name_lookup(tree_nodes[node].text, 1) == -1)
						if (wing_name_lookup(tree_nodes[node].text) == -1)
							type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Ship or wing name expected here", bypass);

				break;

			case OPF_BOOL:
				if (type2 != OPR_BOOL)
					return node_error(node, "Boolean return type expected here", bypass);

				break;

			case OPF_NULL:
				if (type2 != OPR_NULL)
					return node_error(node, "No return type operator expected here", bypass);

				break;

			case OPF_POINT:
				if (type2 != SEXP_ATOM_STRING || verify_vector(tree_nodes[node].text))
					return node_error(node, "3d coordinate expected here", bypass);

				break;

			case OPF_SUBSYSTEM:
			case OPF_AWACS_SUBSYSTEM:
			case OPF_ROTATING_SUBSYSTEM:
				if (type2 == SEXP_ATOM_STRING)
					if (ai_get_subsystem_type(tree_nodes[node].text) == SUBSYSTEM_UNKNOWN)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Subsystem name expected here", bypass);

				break;

			case OPF_IFF:
				if (type2 == SEXP_ATOM_STRING) {
					for (i=0; i<Num_team_names; i++)
						if (!stricmp(Team_names[i], tree_nodes[node].text))
							break;
				}

				if (i == Num_team_names)
					return node_error(node, "Iff team type expected here", bypass);

				break;

			case OPF_AI_GOAL:
				if (type2 != OPR_AI_GOAL)
					return node_error(node, "Ai goal return type expected here", bypass);

				break;

			case OPF_FLEXIBLE_ARGUMENT:
				if (type2 != OPR_FLEXIBLE_ARGUMENT)
					return node_error(node, "Flexible argument return type expected here", bypass);
				
				break;

			case OPF_ANYTHING:
				break;

			case OPF_DOCKER_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Docker docking point name expected here", bypass);

				break;

			case OPF_DOCKEE_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Dockee docking point name expected here", bypass);

				break;
		}

		node = tree_nodes[node].next;
		argnum++;
	}

	return 0;
}
*/

// display an error message and position to point of error (a node)
int sexp_tree::node_error(int node, char *msg, int *bypass)
{
	char text[512];

	if (bypass)
		*bypass = 1;

	item_index = node;
	item_handle = tree_nodes[node].handle;
	if (tree_nodes[node].flags & COMBINED)
		item_handle = tree_nodes[tree_nodes[node].parent].handle;

	ensure_visible(node);
	SelectItem(item_handle);
	sprintf(text, "%s\n\nContinue checking for more errors?", msg);
	if (MessageBox(text, "Sexp error", MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
		return -1;
	else
		return 0;
}

void sexp_tree::hilite_item(int node)
{
	ensure_visible(node);
	SelectItem(tree_nodes[node].handle);
}

// because the MFC function EnsureVisible() doesn't do what it says it does, I wrote this.
void sexp_tree::ensure_visible(int node)
{
	Assert(node != -1);
	if (tree_nodes[node].parent != -1)
		ensure_visible(tree_nodes[node].parent);  // expand all parents first

	if (tree_nodes[node].child != -1)  // expandable?
		Expand(tree_nodes[node].handle, TVE_EXPAND);  // expand this item
}

void sexp_tree::link_modified(int *ptr)
{
	modified = ptr;
}

void get_variable_default_text_from_variable_text(char *text, char *default_text)
{
	int len;
	char *start;

	// find '('
	start = strstr(text, "(");
	Assert(start);
	start++;

	// get length and copy all but last char ")"
	len = strlen(start);
	strncpy(default_text, start, len-1);

	// add null termination
	default_text[len-1] = '\0';
}

void get_variable_name_from_sexp_tree_node_text(const char *text, char *var_name)
{
	int length;
	length = strcspn(text, "(");

	strncpy(var_name, text, length);
	var_name[length] = '\0';
}

int sexp_tree::get_modify_variable_type(int parent)
{
	Assert(parent != -1);
	int sexp_var_index = -1;

	if ( !stricmp(tree_nodes[parent].text, "modify-variable") ) {
		Assert(tree_nodes[parent].child != -1);
		sexp_var_index = get_tree_name_to_sexp_variable_index(tree_nodes[tree_nodes[parent].child].text);
		Assert(sexp_var_index != -1);
	} else {
		Int3();  // should not be called otherwise
	}

	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
		return OPF_NUMBER;
	} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
		return OPF_AMBIGUOUS;
	} else {
		Int3();
		return 0;
	}
}


void sexp_tree::verify_and_fix_arguments(int node)
{
	int op, arg_num, type, tmp;
	static int flag = 0;
	sexp_list_item *list, *ptr;
	bool is_variable_arg = false; 

	if (flag)
		return;

	flag++;
	op = get_operator_index(tree_nodes[node].text);
	if (op < 0)
		return;

	tmp = item_index;
	item_index = node;

	arg_num = 0;
	item_index = tree_nodes[node].child;
	while (item_index >= 0) {
		// get listing of valid argument values for node item_index
		type = query_operator_argument_type(op, arg_num);
		// special case for modify-variable
		if (type == OPF_AMBIGUOUS) {
			is_variable_arg = true;
			type = get_modify_variable_type(node);
		}
		if (query_restricted_opf_range(type)) {
			list = get_listing_opf(type, node, arg_num);
			if (!list && (arg_num >= Operators[op].min)) {
				free_node(item_index, 1);
				item_index = tmp;
				flag--;
				return;
			}

			if (list) {
				// get a pointer to tree_nodes[item_index].text for normal value
				// or default variable value if variable
				char *text_ptr;
				char default_variable_text[TOKEN_LENGTH];
				if (tree_nodes[item_index].type & SEXPT_VARIABLE) {
					// special case for SEXPs which can modify a variable 
					if ( (!stricmp(Operators[op].text, "modify-variable")) ||
						 ((!stricmp(Operators[op].text, "add-background-bitmap")) && (arg_num == 8)) ||
						 ((!stricmp(Operators[op].text, "add-sun-bitmap")) && (arg_num == 5))	){
						// make text_ptr to start - before '('
						get_variable_name_from_sexp_tree_node_text(tree_nodes[item_index].text, default_variable_text);
						text_ptr = default_variable_text;
					} else {
						// only the type needs checking for variables. It's up the to the FREDder to ensure the value is valid
						get_variable_name_from_sexp_tree_node_text(tree_nodes[item_index].text, default_variable_text);						
						int sexp_var_index = get_index_sexp_variable_name(default_variable_text);
						bool types_match = false; 
						Assert(sexp_var_index != -1);

						switch (type) {
							case OPF_NUMBER:
							case OPF_POSITIVE:
								if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
									types_match = true; 
								}
								break; 

							default: 
								if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
									types_match = true; 
								}
						}
						
						if (types_match) {
							// on to the next argument
							item_index = tree_nodes[item_index].next;
							arg_num++;
							continue; 
						}
						else {
							// shouldn't really be getting here unless someone has been hacking the mission in a text editor
							get_variable_default_text_from_variable_text(tree_nodes[item_index].text, default_variable_text);
							text_ptr = default_variable_text;
						}
					}
				} else {
					text_ptr = tree_nodes[item_index].text;
				}

				ptr = list;
				while (ptr) {

					if (ptr->text != NULL) {
						// make sure text is not NULL
						// check that proposed text is valid for operator
						if ( !stricmp(ptr->text, text_ptr) )
							break;

						ptr = ptr->next;
					} else {
						// text is NULL, so set ptr to NULL to end loop
						ptr = NULL;
					}
				}

				if (!ptr) {  // argument isn't in list of valid choices, 
					if (list->op >= 0) {
						replace_operator(list->text);
					} else {
						replace_data(list->text, list->type);
					}
				}

			} else {
				bool invalid = false;
				if (type == OPF_AMBIGUOUS) {
					if (SEXPT_TYPE(tree_nodes[item_index].type) == SEXPT_OPERATOR) {
						invalid = true;
					}
				} else {
					if (SEXPT_TYPE(tree_nodes[item_index].type) != SEXPT_OPERATOR) {
						invalid = true;
					}
				}

				if (invalid) {
					replace_data("<Invalid>", (SEXPT_STRING | SEXPT_VALID));
				}
			}

			if (tree_nodes[item_index].type & SEXPT_OPERATOR)
				verify_and_fix_arguments(item_index);
			
		}
		
		//fix the node if it is the argument for modify-variable
		if (is_variable_arg //&& 
		//	!(tree_nodes[item_index].type & SEXPT_OPERATOR || tree_nodes[item_index].type & SEXPT_VARIABLE ) 
			) {
			switch (type) {
				case OPF_AMBIGUOUS:
					tree_nodes[item_index].type |= SEXPT_STRING;
					tree_nodes[item_index].type &= ~SEXPT_NUMBER;
					break; 

				case OPF_NUMBER:
					tree_nodes[item_index].type |= SEXPT_NUMBER; 
					tree_nodes[item_index].type &= ~SEXPT_STRING;
					break;

				default:
					Int3();
			}
		}

		item_index = tree_nodes[item_index].next;
		arg_num++;
	}

	item_index = tmp;
	flag--;
}

void sexp_tree::replace_data(char *data, int type)
{
	int node;
	HTREEITEM h;

	node = tree_nodes[item_index].child;
	if (node != -1)
		free_node2(node);

	tree_nodes[item_index].child = -1;
	h = tree_nodes[item_index].handle;
	while (ItemHasChildren(h))
		DeleteItem(GetChildItem(h));

	set_node(item_index, type, data);
	SetItemText(h, data);
	int bmap = get_data_image(item_index);
	SetItemImage(h, bmap, bmap);
	tree_nodes[item_index].flags = EDITABLE;

	// check remaining data beyond replaced data for validity (in case any of it is dependent on data just replaced)
	verify_and_fix_arguments(tree_nodes[item_index].parent);

	*modified = 1;
	update_help(GetSelectedItem());
}


// Replaces data with sexp_variable type data
void sexp_tree::replace_variable_data(int var_idx, int type)
{
	int node;
	HTREEITEM h;
	char buf[128];

	Assert(type & SEXPT_VARIABLE);

	node = tree_nodes[item_index].child;
	if (node != -1)
		free_node2(node);

	tree_nodes[item_index].child = -1;
	h = tree_nodes[item_index].handle;
	while (ItemHasChildren(h)) {
		DeleteItem(GetChildItem(h));
	}

	// Assemble name
	sprintf(buf, "%s(%s)", Sexp_variables[var_idx].variable_name, Sexp_variables[var_idx].text);

	set_node(item_index, type, buf);
	SetItemText(h, buf);
	SetItemImage(h, BITMAP_VARIABLE, BITMAP_VARIABLE);
	tree_nodes[item_index].flags = NOT_EDITABLE;

	// check remaining data beyond replaced data for validity (in case any of it is dependent on data just replaced)
	verify_and_fix_arguments(tree_nodes[item_index].parent);

	*modified = 1;
	update_help(GetSelectedItem());
}



void sexp_tree::replace_operator(char *op)
{
	int node;
	HTREEITEM h;

	node = tree_nodes[item_index].child;
	if (node != -1)
		free_node2(node);

	tree_nodes[item_index].child = -1;
	h = tree_nodes[item_index].handle;
	while (ItemHasChildren(h))
		DeleteItem(GetChildItem(h));

	set_node(item_index, (SEXPT_OPERATOR | SEXPT_VALID), op);
	SetItemText(h, op);
	tree_nodes[item_index].flags = OPERAND;
	*modified = 1;
	update_help(GetSelectedItem());

	// hack added at Allender's request.  If changing ship in an ai-dock operator, re-default
	// docking point.
}

/*void sexp_tree::replace_one_arg_operator(char *op, char *data, int type)
{
	char str[80];
	int node;
	HTREEITEM h;

	node = tree_nodes[item_index].child;
	if (node != -1)
		free_node2(node);

	tree_nodes[item_index].child = -1;
	h = tree_nodes[item_index].handle;
	while (ItemHasChildren(h))
		DeleteItem(GetChildItem(h));
	
	node = allocate_node(item_index);
	set_node(item_index, SEXPT_OPERATOR, op);
	set_node(node, type, data);
	sprintf(str, "%s %s", op, data);
	SetItemText(h, str);
	tree_nodes[item_index].flags = OPERAND | EDITABLE;
	tree_nodes[node].flags = COMBINED;
	*modified = 1;
	update_help(GetSelectedItem());
}*/

// moves a whole sexp tree branch to a new position under 'parent' and after 'after'.
// The expansion state is preserved, and node handles are updated.
void sexp_tree::move_branch(int source, int parent)
{
	int node;

	// if no source, skip everything
	if (source != -1) {
		node = tree_nodes[source].parent;
		if (node != -1) {
			if (tree_nodes[node].child == source)
				tree_nodes[node].child = tree_nodes[source].next;
			else {
				node = tree_nodes[node].child;
				while (tree_nodes[node].next != source) {
					node = tree_nodes[node].next;
					Assert(node != -1);
				}

				tree_nodes[node].next = tree_nodes[source].next;
			}
		}

		tree_nodes[source].parent = parent;
		tree_nodes[source].next = -1;
		if (parent) {
			if (tree_nodes[parent].child == -1)
				tree_nodes[parent].child = source;
			else {
				node = tree_nodes[parent].child;
				while (tree_nodes[node].next != -1)
					node = tree_nodes[node].next;

				tree_nodes[node].next = source;
			}

			move_branch(tree_nodes[source].handle, tree_nodes[parent].handle);

		} else
			move_branch(tree_nodes[source].handle);
	}
}

HTREEITEM sexp_tree::move_branch(HTREEITEM source, HTREEITEM parent, HTREEITEM after)
{
	uint i;
	int image1, image2;
	HTREEITEM h = 0, child, next;

	if (source) {
		for (i=0; i<tree_nodes.size(); i++)
			if (tree_nodes[i].handle == source)
				break;

		if (i < tree_nodes.size()) {
			GetItemImage(source, image1, image2);
			h = insert(GetItemText(source), image1, image2, parent, after);
			tree_nodes[i].handle = h;

		} else {
			GetItemImage(source, image1, image2);
  			h = insert(GetItemText(source), image1, image2, parent, after);
		}

		SetItemData(h, GetItemData(source));
		child = GetChildItem(source);
		while (child) {
			next = GetNextSiblingItem(child);
			move_branch(child, h);
			child = next;
		}

		if (GetItemState(source, TVIS_EXPANDED) & TVIS_EXPANDED)
			Expand(h, TVE_EXPAND);

		DeleteItem(source);
	}

	return h;
}

void sexp_tree::copy_branch(HTREEITEM source, HTREEITEM parent, HTREEITEM after)
{
	uint i;
	int image1, image2;
	HTREEITEM h, child;

	if (source) {
		for (i=0; i<tree_nodes.size(); i++)
			if (tree_nodes[i].handle == source)
				break;

		if (i < tree_nodes.size()) {
			GetItemImage(source, image1, image2);
			h = insert(GetItemText(source), image1, image2, parent, after);
			tree_nodes[i].handle = h;

		} else {
			GetItemImage(source, image1, image2);
  			h = insert(GetItemText(source), image1, image2, parent, after);
		}

		SetItemData(h, GetItemData(source));
		child = GetChildItem(source);
		while (child) {
			copy_branch(child, h);
			child = GetNextSiblingItem(child);
		}

		if (GetItemState(source, TVIS_EXPANDED) & TVIS_EXPANDED)
			Expand(h, TVE_EXPAND);
	}
}

void sexp_tree::swap_roots(HTREEITEM one, HTREEITEM two)
{
	HTREEITEM h;

	Assert(!GetParentItem(one));
	Assert(!GetParentItem(two));
//	copy_branch(one, TVI_ROOT, two);
//	move_branch(two, TVI_ROOT, one);
//	DeleteItem(one);
	h = move_branch(one, TVI_ROOT, two);
	SelectItem(h);
	SelectItem(h);
	*modified = 1;
}

void sexp_tree::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UINT flags;

//	ScreenToClient(&m_pt);
	ASSERT(!m_dragging);
	m_h_drag = HitTest(m_pt, &flags);
	m_h_drop = NULL;

	if (!m_mode || GetParentItem(m_h_drag))
		return;

	ASSERT(m_p_image_list == NULL);
	m_p_image_list = CreateDragImage(m_h_drag);  // get the image list for dragging
	if (!m_p_image_list)
		return;

	m_p_image_list->DragShowNolock(TRUE);
	m_p_image_list->SetDragCursorImage(0, CPoint(0, 0));
	m_p_image_list->BeginDrag(0, CPoint(0,0));
	m_p_image_list->DragMove(m_pt);
	m_p_image_list->DragEnter(this, m_pt);
	SetCapture();
	m_dragging = TRUE;
}

void sexp_tree::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_pt = point;
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void sexp_tree::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM hitem;
	UINT flags;

	if (m_dragging) {
		ASSERT(m_p_image_list != NULL);
		m_p_image_list->DragMove(point);
		if ((hitem = HitTest(point, &flags)) != NULL)
			if (!GetParentItem(hitem)) {
				m_p_image_list->DragLeave(this);
				SelectDropTarget(hitem);
				m_h_drop = hitem;
				m_p_image_list->DragEnter(this, point);
			}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void sexp_tree::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int index1, index2;

	if (m_dragging) {
		ASSERT(m_p_image_list != NULL);
		m_p_image_list->DragLeave(this);
		m_p_image_list->EndDrag();
		delete m_p_image_list;
		m_p_image_list = NULL;

		if (m_h_drop && m_h_drag != m_h_drop) {
			Assert(m_h_drag);
			index1 = GetItemData(m_h_drag);
			index2 = GetItemData(m_h_drop);
			swap_roots(m_h_drag, m_h_drop);
			if (m_mode == MODE_GOALS) {
				Assert(Goal_editor_dlg);
				Goal_editor_dlg->swap_handler(index1, index2);

			} else if (m_mode == MODE_EVENTS) {
				Assert(Event_editor_dlg);
				Event_editor_dlg->swap_handler(index1, index2);

			} else if (m_mode == MODE_CAMPAIGN) {
				Campaign_tree_formp->swap_handler(index1, index2);

			} else
				Assert(0);

		} else
			MessageBeep(0);

		ReleaseCapture();
		m_dragging = FALSE;
		SelectDropTarget(NULL);
	}

	CTreeCtrl::OnLButtonUp(nFlags, point);
}

const static UINT Numbered_data_bitmaps[] = {
	IDB_DATA_00,
	IDB_DATA_05,
	IDB_DATA_10,
	IDB_DATA_15,
	IDB_DATA_20,
	IDB_DATA_25,
	IDB_DATA_30,
	IDB_DATA_35,
	IDB_DATA_40,
	IDB_DATA_45,
	IDB_DATA_50,
	IDB_DATA_55,
	IDB_DATA_60,
	IDB_DATA_65,
	IDB_DATA_70,
	IDB_DATA_75,
	IDB_DATA_80,
	IDB_DATA_85,
	IDB_DATA_90,
	IDB_DATA_95
};

void sexp_tree::setup(CEdit *ptr)
{
	CImageList *pimagelist;
	CBitmap bitmap;

	help_box = ptr;
	if (help_box) {
		int stops[2] = { 10, 30 };

		help_box -> SetTabStops(2, (LPINT) stops);
	}

	pimagelist = GetImageList(TVSIL_NORMAL);
	if (!pimagelist) {
		pimagelist = new CImageList();
		pimagelist->Create(16, 16, TRUE/*bMask*/, 2, 22);

		//*****Add generic images
		bitmap.LoadBitmap(IDB_OPERATOR);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_DATA);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_VARIABLE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_ROOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_ROOT_DIRECTIVE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CHAINED);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CHAINED_DIRECTIVE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_GREEN_DOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_BLACK_DOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		//*****Add numbered data entries
		int num = sizeof(Numbered_data_bitmaps)/sizeof(UINT);
		int i = 0;
		for(i = 0; i < num; i++)
		{
			bitmap.LoadBitmap(Numbered_data_bitmaps[i]);
			pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
			bitmap.DeleteObject();
		}

		SetImageList(pimagelist, TVSIL_NORMAL);
	}
}
//#define BITMAP_OPERATOR 0
//#define BITMAP_DATA 1
//#define BITMAP_VARIABLE 2
//#define BITMAP_ROOT 3
//#define BITMAP_ROOT_DIRECTIVE 4
//#define BITMAP_CHAIN 5
//#define BITMAP_CHAIN_DIRECTIVE 6
//#define BITMAP_GREEN_DOT 7
//#define BITMAP_BLACK_DOT 8


HTREEITEM sexp_tree::insert(LPCTSTR lpszItem, int image, int sel_image, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(lpszItem, image, sel_image, hParent, hInsertAfter);

}

void sexp_tree::OnDestroy() 
{
	CImageList *pimagelist;

	pimagelist = GetImageList(TVSIL_NORMAL);
	if (pimagelist) {
		pimagelist->DeleteImageList();
		delete pimagelist;
	}

	CTreeCtrl::OnDestroy();
}

HTREEITEM sexp_tree::handle(int node)
{
	return tree_nodes[node].handle;
}

char *sexp_tree::help(int code)
{
	int i;

	i = Num_sexp_help;
	while (i--) {
		if (Sexp_help[i].id == code)
			break;
	}

	if (i >= 0)
		return Sexp_help[i].help;

	return NULL;
}

// get type of item clicked on
int sexp_tree::get_type(HTREEITEM h)
{
	uint i;

	// get index into sexp_tree 
	for (i=0; i<tree_nodes.size(); i++)
		if (tree_nodes[i].handle == h)
			break;

	if ( (i >= tree_nodes.size()) ) {
		// Int3();	// This would be the root of the tree  -- ie, event name
		return -1;
	}

	return tree_nodes[i].type;
}


void sexp_tree::update_help(HTREEITEM h)
{
	char *str;
	int i, j, z, c, code, index, sibling_place;
	CString text;

/* Goober5000 - this is just annoying
	for (i=0; i<Num_operators; i++)
		for (j=0; j<Num_op_menus; j++)
			if ((Operators[i].value & OP_CATEGORY_MASK) == op_menu[j].id) {
				if (!help(Operators[i].value))
					Int3();  // Allender!  If you add new sexp operators, add help for them too! :)
			}
*/
	help_box = (CEdit *) GetParent()->GetDlgItem(IDC_HELP_BOX);
	if (!help_box || !::IsWindow(help_box->m_hWnd))
		return;

	mini_help_box = (CEdit *) GetParent()->GetDlgItem(IDC_MINI_HELP_BOX);
	if (!mini_help_box || !::IsWindow(mini_help_box->m_hWnd))
		return;

	for (i=0; i<(int)tree_nodes.size(); i++)
		if (tree_nodes[i].handle == h)
			break;

	if ((i >= (int)tree_nodes.size()) || !tree_nodes[i].type) {
		help_box->SetWindowText("");
		mini_help_box->SetWindowText("");
		return;
	}

	if (SEXPT_TYPE(tree_nodes[i].type) != SEXPT_OPERATOR)
	{
		z = tree_nodes[i].parent;
		if (z < 0) {
			Warning(LOCATION, "Sexp data \"%s\" has no parent!", tree_nodes[i].text);
			return;
		}

		code = get_operator_const(tree_nodes[z].text);
		index = get_operator_index(tree_nodes[z].text);
		sibling_place = get_sibling_place(i) + 1;	//We want it to start at 1

		//*****Minihelp box
		if((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_NUMBER) || (SEXPT_TYPE(tree_nodes[i].type) == SEXPT_STRING) && sibling_place > 0)
		{
			char buffer[10240] = {""};

			//Get the help for the current operator
			char *helpstr = help(code);
			bool display_number = true;

			//If a help string exists, try to display it
			if(helpstr != NULL)
			{
				char searchstr[32];
				char *loc=NULL, *loc2=NULL;

				if(loc == NULL)
				{
					sprintf(searchstr, "\n%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}

				if(loc == NULL)
				{
					sprintf(searchstr, "\t%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if(loc == NULL)
				{
					sprintf(searchstr, " %d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if(loc == NULL)
				{
					sprintf(searchstr, "%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}

				if(loc != NULL)
				{
					//Skip whitespace
					while(*loc=='\r' || *loc == '\n' || *loc == ' ' || *loc == '\t') loc++;

					//Find EOL
					loc2 = strpbrk(loc, "\r\n");
					if(loc2 != NULL)
					{
						size_t size = loc2-loc;
						strncpy(buffer, loc, size);
						if(size < sizeof(buffer))
						{
							buffer[size] = '\0';
						}
						display_number = false;
					}
					else
					{
						strcpy(buffer, loc);
						display_number = false;
					}
				}
			}

			//Display argument number
			if(display_number)
			{
				sprintf(buffer, "%d:", sibling_place);
			}

			mini_help_box->SetWindowText(buffer);
		}

		if (index >= 0) {
			c = 0;
			j = tree_nodes[z].child;
			while ((j >= 0) && (j != i)) {
				j = tree_nodes[j].next;
				c++;
			}

			Assert(j >= 0);
			if (query_operator_argument_type(index, c) == OPF_MESSAGE) {
				for (j=0; j<Num_messages; j++)
					if (!stricmp(Messages[j].name, tree_nodes[i].text)) {
						text.Format("Message Text:\r\n%s", Messages[j].message);
						help_box->SetWindowText(text);
						return;
					}
			}
		}

		i = z;
	}

	code = get_operator_const(tree_nodes[i].text);
	str = help(code);
	if (!str)
		str = "No help available";

	help_box->SetWindowText(str);
}

// find list of sexp_tree nodes with text
// stuff node indices into find[]
int sexp_tree::find_text(char *text, int *find)
{
	uint i;
	int find_count;

	// initialize find
	for (i=0; i<MAX_SEARCH_MESSAGE_DEPTH; i++) {
		find[i] = -1;
	}

	find_count = 0;

	for (i=0; i<tree_nodes.size(); i++) {
		// only look at used and editable nodes
		if ((tree_nodes[i].flags & EDITABLE && (tree_nodes[i].type != SEXPT_UNUSED))) {
			// find the text
			if ( !stricmp(tree_nodes[i].text, text)  ) {
				find[find_count++] = i;

				// don't exceed max count - array bounds
				if (find_count == MAX_SEARCH_MESSAGE_DEPTH) {
					break;
				}
			}
		}
	}

	return find_count;
}
			

void sexp_tree::OnKeydown(NMHDR *pNMHDR, LRESULT *pResult) 
{
	int key;
	TV_KEYDOWN *pTVKeyDown = (TV_KEYDOWN *) pNMHDR;

	key = pTVKeyDown->wVKey;
	if (key == VK_SPACE)
		EditLabel(GetSelectedItem());

	*pResult = 0;
}

// Determine if a given opf code has a restricted argument range (i.e. has a specific, limited
// set of argument values, or has virtually unlimited possibilities.  For example, boolean values
// only have true or false, so it is restricted, but a number could be anything, so it's not.
//
int sexp_tree::query_restricted_opf_range(int opf)
{
	switch (opf) {
		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_WHO_FROM:

		// Goober5000 - these are needed too (otherwise the arguments revert to their defaults)
		case OPF_STRING:
		case OPF_ANYTHING:
			return 0;
	}

	return 1;
}

// generate listing of valid argument values.
// opf = operator format to generate list for
// parent_node = the parent node we are generating list for
// arg_index = argument number of parent this argument will go at
//
// Goober5000 - add the listing from get_listing_opf_sub to the end of a new list containing
// the special argument item, but only if it's a child of a when-argument (or similar) sexp.
// Also only do this if the list has at least one item, because otherwise the argument code
// would have nothing to select from.
sexp_list_item *sexp_tree::get_listing_opf(int opf, int parent_node, int arg_index)
{
	sexp_list_item head;
	sexp_list_item *list = NULL;
	int i, current_node, w_arg, e_arg;

	// get the current node
	current_node = tree_nodes[parent_node].child;
	for (i = 0; i < arg_index; i++)
		current_node = tree_nodes[current_node].next;

	switch (opf) {
		case OPF_NONE:
			list = NULL;
			break;

		case OPF_NULL:
			list = get_listing_opf_null();
			break;

		case OPF_BOOL:
			list = get_listing_opf_bool(parent_node);
			break;

		case OPF_NUMBER:
			list = get_listing_opf_number();
			break;

		case OPF_SHIP:
			list = get_listing_opf_ship(parent_node);
			break;

		case OPF_WING:
			list = get_listing_opf_wing();
			break;
		
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_SUBSYSTEM:
			list = get_listing_opf_subsystem(parent_node, arg_index);
			break;

		case OPF_POINT:
			list = get_listing_opf_point();
			break;

		case OPF_IFF:
			list = get_listing_opf_iff();
			break;

		case OPF_AI_CLASS:
			list = get_listing_opf_ai_class();
			break;

		case OPF_SUPPORT_SHIP_CLASS:
			list = get_listing_opf_support_ship_class();
			break;

		case OPF_SSM_CLASS:
			list = get_listing_opf_ssm_class();
			break;

		case OPF_ARRIVAL_LOCATION:
			list = get_listing_opf_arrival_location();
			break;

		case OPF_DEPARTURE_LOCATION:
			list = get_listing_opf_departure_location();
			break;

		case OPF_ARRIVAL_ANCHOR_ALL:
			list = get_listing_opf_arrival_anchor_all();
			break;

		case OPF_SHIP_WITH_BAY:
			list = get_listing_opf_ship_with_bay();
			break;

		case OPF_SOUNDTRACK_NAME:
			list = get_listing_opf_soundtrack_name();
			break;

		case OPF_AI_GOAL:
			list = get_listing_opf_ai_goal(parent_node);
			break;

		case OPF_FLEXIBLE_ARGUMENT:
			list = get_listing_opf_flexible_argument();
			break;

		case OPF_DOCKER_POINT:
			list = get_listing_opf_docker_point(parent_node);
			break;

		case OPF_DOCKEE_POINT:
			list = get_listing_opf_dockee_point(parent_node);
			break;

		case OPF_MESSAGE:
			list = get_listing_opf_message();
			break;

		case OPF_WHO_FROM:
			list = get_listing_opf_who_from();
			break;

		case OPF_PRIORITY:
			list = get_listing_opf_priority();
			break;

		case OPF_WAYPOINT_PATH:
			list = get_listing_opf_waypoint_path();
			break;

		case OPF_POSITIVE:
			list = get_listing_opf_positive();
			break;

		case OPF_MISSION_NAME:
			list = get_listing_opf_mission_name();
			break;

		case OPF_SHIP_POINT:
			list = get_listing_opf_ship_point();
			break;

		case OPF_GOAL_NAME:
			list = get_listing_opf_goal_name(parent_node);
			break;

		case OPF_SHIP_WING:
			list = get_listing_opf_ship_wing();
			break;

		case OPF_SHIP_WING_POINT:
			list = get_listing_opf_ship_wing_point();
			break;

		case OPF_SHIP_WING_POINT_OR_NONE:
			list = get_listing_opf_ship_wing_point_or_none();
			break;

		case OPF_ORDER_RECIPIENT:
			list = get_listing_opf_order_recipient();
			break;

		case OPF_SHIP_TYPE:
			list = get_listing_opf_ship_type();
			break;

		case OPF_KEYPRESS:
			list = get_listing_opf_keypress();
			break;

		case OPF_EVENT_NAME:
			list = get_listing_opf_event_name(parent_node);
			break;

		case OPF_AI_ORDER:
			list = get_listing_opf_ai_order();
			break;

		case OPF_SKILL_LEVEL:
			list = get_listing_opf_skill_level();
			break;

		case OPF_CARGO:
			list = get_listing_opf_cargo();
			break;

		case OPF_STRING:
			list = get_listing_opf_string();
			break;

		case OPF_MEDAL_NAME:
			list = get_listing_opf_medal_name();
			break;

		case OPF_WEAPON_NAME:
			list = get_listing_opf_weapon_name();
			break;

		case OPF_INTEL_NAME:
			list = get_listing_opf_intel_name();
			break;

		case OPF_SHIP_CLASS_NAME:
			list = get_listing_opf_ship_class_name();
			break;

		case OPF_HUD_GAUGE_NAME:
			list = get_listing_opf_hud_gauge_name();
			break;

		case OPF_HUGE_WEAPON:
			list = get_listing_opf_huge_weapon();
			break;

		case OPF_SHIP_NOT_PLAYER:
			list = get_listing_opf_ship_not_player();
			break;

		case OPF_SHIP_OR_NONE:
			list = get_listing_opf_ship_or_none();
			break;

		case OPF_SUBSYSTEM_OR_NONE:
			list = get_listing_opf_subsystem_or_none(parent_node, arg_index);
			break;

		case OPF_SUBSYS_OR_GENERIC:
			list = get_listing_opf_subsys_or_generic(parent_node, arg_index);
			break;

		case OPF_JUMP_NODE_NAME:
			list = get_listing_opf_jump_nodes();
			break;

		case OPF_VARIABLE_NAME:
			list = get_listing_opf_variable_names();
			break;

		case OPF_AMBIGUOUS:
			list = NULL;
			break;

		case OPF_ANYTHING:
			list = NULL;
			break;

		case OPF_SKYBOX_MODEL_NAME:
			list = get_listing_opf_skybox_model();
			break;

		case OPF_BACKGROUND_BITMAP:
			list = get_listing_opf_background_bitmap();
			break;

		case OPF_SUN_BITMAP:
			list = get_listing_opf_sun_bitmap();
			break;

		case OPF_NEBULA_STORM_TYPE:
			list = get_listing_opf_nebula_storm_type();
			break;

		case OPF_NEBULA_POOF:
			list = get_listing_opf_nebula_poof();
			break;

		case OPF_TURRET_TARGET_ORDER:
			list = get_listing_opf_turret_target_order();
			break;

		case OPF_PERSONA:
			list = get_listing_opf_persona();
			break;

		default:
			Int3();  // unknown OPF code
			list = NULL;
			break;
	}

	// skip the special argument if we aren't at the right spot in when-argument or
	// every-time-argument... meaning if either w_arg or e_arg is >= 1, we can continue
	w_arg = find_ancestral_argument_number(OP_WHEN_ARGUMENT, current_node);
	e_arg = find_ancestral_argument_number(OP_EVERY_TIME_ARGUMENT, current_node);
	if (w_arg < 1 && e_arg < 1 /* && the same for any future _ARGUMENT sexps */ ) {
		return list;
	}

	// also skip for OPF_NULL, because it takes no data (though it can take plenty of operators)
	if (list == NULL || opf == OPF_NULL) {
		return list;
	}

	// the special item is a string and should not be added for numeric lists
	if (opf != OPF_NUMBER && opf != OPF_POSITIVE) {
		head.add_data(SEXP_ARGUMENT_STRING);
	}

	// append other list
	head.add_list(list);

	// return listing
	return head.next;
}

// Goober5000
int sexp_tree::find_argument_number(int parent_node, int child_node)
{
	int arg_num, current_node;

	// code moved/adapted from match_closest_operator
	arg_num = 0;
	current_node = tree_nodes[parent_node].child;
	while (current_node >= 0)
	{
		// found?
		if (current_node == child_node)
			return arg_num;

		// continue iterating
		arg_num++;
		current_node = tree_nodes[current_node].next;
	}	

	// not found
	return -1;
}

// Goober5000
// backtrack through parents until we find the operator matching
// parent_op, then find the argument we went through
int sexp_tree::find_ancestral_argument_number(int parent_op, int child_node)
{
	if(child_node == -1)
		return -1;

	int parent_node;
	int current_node;

	current_node = child_node;
	parent_node = tree_nodes[current_node].parent;

	while (parent_node >= 0)
	{
		// check if the parent operator is the one we're looking for
		if (get_operator_const(tree_nodes[parent_node].text) == parent_op)
			return find_argument_number(parent_node, current_node);

		// continue iterating up the tree
		current_node = parent_node;
		parent_node = tree_nodes[current_node].parent;
	}

	// not found
	return -1;
}

/**
* Gets the proper data image for the tree item's place
* in its parent hierarchy.
*/
int sexp_tree::get_data_image(int node)
{
	int count = get_sibling_place(node) + 1;

	if (count <= 0) {
		return BITMAP_DATA;
	}

	if (count % 5) {
		return BITMAP_DATA;
	}

	int idx = (count % 100) / 5;

	int num = sizeof(Numbered_data_bitmaps)/sizeof(UINT);

	if (idx > num) {
		return BITMAP_DATA;
	}

	return BITMAP_NUMBERED_DATA + idx;
}

int sexp_tree::get_sibling_place(int node)
{
	if(tree_nodes[node].parent < 0 || tree_nodes[node].parent > (int)tree_nodes.size())
		return -1;
	
	sexp_tree_item *myparent = &tree_nodes[tree_nodes[node].parent];

	if(myparent->child == -1)
		return -1;

	sexp_tree_item *mysibling = &tree_nodes[myparent->child];

	int count = 0;
	while(true)
	{
		if(mysibling == &tree_nodes[node])
			break;

		if(mysibling->next == -1)
			break;

		count++;
		mysibling = &tree_nodes[mysibling->next];
	}

	return count;
}


sexp_list_item *sexp_tree::get_listing_opf_null()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_operators; i++)
		if (query_operator_return_type(i) == OPR_NULL)
			head.add_op(i);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_flexible_argument()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_operators; i++)
		if (query_operator_return_type(i) == OPR_FLEXIBLE_ARGUMENT)
			head.add_op(i);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_bool(int parent_node)
{
	int i, only_basic;
	sexp_list_item head;

	// search for the previous goal/event operators.  If found, only add the true/false
	// sexpressions to the list
	only_basic = 0;
	if ( parent_node != -1 ) {
		int op;

		op = get_operator_const(tree_nodes[parent_node].text);
		if ( (op == OP_PREVIOUS_GOAL_TRUE) || (op == OP_PREVIOUS_GOAL_FALSE) || (op == OP_PREVIOUS_EVENT_TRUE) || (op == OP_PREVIOUS_EVENT_FALSE) )
			only_basic = 1;

	}

	for (i=0; i<Num_operators; i++) {
		if (query_operator_return_type(i) == OPR_BOOL) {
			if ( !only_basic || (only_basic && ((Operators[i].value == OP_TRUE) || (Operators[i].value == OP_FALSE))) ) {
				head.add_op(i);
			}
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_positive()
{
	int i, z;
	sexp_list_item head;

	for (i=0; i<Num_operators; i++) {
		z = query_operator_return_type(i);
		// Goober5000's number hack
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE))
			head.add_op(i);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_number()
{
	int i, z;
	sexp_list_item head;

	for (i=0; i<Num_operators; i++) {
		z = query_operator_return_type(i);
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE))
			head.add_op(i);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship(int parent_node)
{
	object *ptr;
	sexp_list_item head;
	int op = 0, dock_ship = -1, require_cap_ship = 0;

	// look at the parent node and get the operator.  Some ship lists should be filtered based
	// on what the parent operator is
	if ( parent_node >= 0 ) {
		op = get_operator_const(tree_nodes[parent_node].text);

		// prune out to only capital ships
		if (!stricmp(tree_nodes[parent_node].text, "cap-subsys-cargo-known-delay")) {
			require_cap_ship = 1;
		}

		// get the dock_ship number of if this goal is an ai dock goal.  used to prune out unwanted ships out
		// of the generated ship list
		dock_ship = -1;
		if ( op == OP_AI_DOCK ) {
			int z;

			z = tree_nodes[parent_node].parent;
			Assert(z >= 0);
			Assert(!stricmp(tree_nodes[z].text, "add-ship-goal") || !stricmp(tree_nodes[z].text, "add-wing-goal") || !stricmp(tree_nodes[z].text, "add-goal"));

			z = tree_nodes[z].child;
			Assert(z >= 0);

			dock_ship = ship_name_lookup(tree_nodes[z].text, 1);
			Assert( dock_ship != -1 );
		}
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if ( op == OP_AI_DOCK ) {
				// only include those ships in the list which the given ship can dock with.
				if ( (dock_ship != ptr->instance) && ship_docking_valid(dock_ship , ptr->instance) )
					head.add_data(Ships[ptr->instance].ship_name );

			} else {
				if ( !require_cap_ship || (Ship_info[Ships[ptr->instance].ship_info_index].flags & SIF_HUGE_SHIP) ) {
					head.add_data(Ships[ptr->instance].ship_name);
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_wing()
{
	int i;
	sexp_list_item head;

	for (i=0; i<MAX_WINGS; i++){
		if (Wings[i].wave_count){
			head.add_data(Wings[i].name);
		}
	}

	return head.next;
}

// specific types of subsystems we're looking for
#define OPS_CAP_CARGO		1	
#define OPS_STRENGTH			2
#define OPS_BEAM_TURRET		3
#define OPS_AWACS				4
#define OPS_ROTATE			5
sexp_list_item *sexp_tree::get_listing_opf_subsystem(int parent_node, int arg_index)
{
	int op, child, sh;
	int special_subsys = 0;
	sexp_list_item head;
	ship_subsys *subsys;

	// determine if the parent is one of the set subsystem strength items.  If so,
	// we want to append the "Hull" name onto the end of the menu
	Assert(parent_node >= 0);	
	
	// get the operator type of the node
	op = get_operator_const(tree_nodes[parent_node].text);

	// first child node
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);

	switch(op)
	{
		// where we care about hull strength
		case OP_REPAIR_SUBSYSTEM:
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
			special_subsys = OPS_STRENGTH;
			break;

		// awacs subsystems
		case OP_AWACS_SET_RADIUS:
			special_subsys = OPS_AWACS;
			break;

		// rotating
		case OP_LOCK_ROTATING_SUBSYSTEM:
		case OP_FREE_ROTATING_SUBSYSTEM:
		case OP_REVERSE_ROTATING_SUBSYSTEM:
		case OP_ROTATING_SUBSYS_SET_TURN_TIME:
			special_subsys = OPS_ROTATE;
			break;

		// where we care about capital ship subsystem cargo
		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
			special_subsys = OPS_CAP_CARGO;
			
			// get the next sibling
			child = tree_nodes[child].next;		
			break;

		// where we care about turrets carrying beam weapons
		case OP_BEAM_FIRE:
			special_subsys = OPS_BEAM_TURRET;

			// if this is arg index 3 (targeted ship)
			if(arg_index == 3)
			{
				child = tree_nodes[child].next;
				Assert(child >= 0);			
				child = tree_nodes[child].next;			
			}
			else
			{
				Assert(arg_index == 1);
			}
			break;

		// these sexps check the subsystem of the *second entry* on the list, not the first
		case OP_DISTANCE_SUBSYSTEM:
		case OP_SET_CARGO:
		case OP_IS_CARGO:
		case OP_CHANGE_AI_CLASS:
		case OP_IS_AI_CLASS:
		case OP_MISSILE_LOCKED:
		case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
			// iterate to the next field
			child = tree_nodes[child].next;
			break;

		// this sexp checks the subsystem of the *fourth entry* on the list
		case OP_QUERY_ORDERS:
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			break;

		// this sexp checks the subsystem of the *ninth entry* on the list
		case OP_WEAPON_CREATE:
			// iterate to the next field eight times
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
			break;
	}			

	// now find the ship and add all relevant subsystems
	Assert(child >= 0);
	sh = ship_name_lookup(tree_nodes[child].text, 1);
	if (sh >= 0) {
		subsys = GET_FIRST(&Ships[sh].subsys_list);
		while (subsys != END_OF_LIST(&Ships[sh].subsys_list)) {
			// add stuff
			switch(special_subsys){
			// subsystem cargo
			case OPS_CAP_CARGO:					
				if (valid_cap_subsys_cargo_list(subsys->system_info->subobj_name) ) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// beam fire
			case OPS_BEAM_TURRET:
				head.add_data(subsys->system_info->subobj_name);
				break;

			// awacs level
			case OPS_AWACS:
				if (subsys->system_info->flags & MSS_FLAG_AWACS) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// rotating
			case OPS_ROTATE:
				if (subsys->system_info->flags & MSS_FLAG_ROTATES) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// everything else
			default:
				head.add_data(subsys->system_info->subobj_name);
				break;
			}

			// next subsystem
			subsys = GET_NEXT(subsys);
		}
	}
	
	// if one of the subsystem strength operators, append the Hull string and the Simulated Hull string
	if(special_subsys == OPS_STRENGTH){
		head.add_data(SEXP_HULL_STRING);
		head.add_data(SEXP_SIM_HULL_STRING);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_point()
{
	char buf[NAME_LENGTH+8];
	int i, j;
	sexp_list_item head;

	for (i=0; i<Num_waypoint_lists; i++)
		for (j=0; j<Waypoint_lists[i].count; j++) {
			sprintf(buf, "%s:%d", Waypoint_lists[i].name, j + 1);
			head.add_data_dup(buf);
		}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_iff()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_iffs; i++)
		head.add_data(Iff_info[i].iff_name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ai_class()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_ai_classes; i++)
		head.add_data(Ai_class_names[i]);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_support_ship_class()
{
	int i;
	sexp_list_item head;

	head.add_data("<any support ship class>");

	for (i=0; i<Num_ship_classes; i++)
	{
		if (Ship_info[i].flags & SIF_SUPPORT)
		{
			head.add_data(Ship_info[i].name);
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ssm_class()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Ssm_info_count; i++)
	{
		head.add_data(Ssm_info[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_with_bay()
{
	object *objp;
	sexp_list_item head;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) )
		{
			// determine if this ship has a docking bay
			if (ship_has_dock_bay(objp->instance))
			{
				head.add_data(Ships[objp->instance].ship_name);
			}
		}
	}

	head.add_data("<no anchor>");

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_soundtrack_name()
{
	int i;
	sexp_list_item head;

	head.add_data("<No Music>");

	for (i=0; i<Num_soundtracks; i++)
	{
		head.add_data(Soundtracks[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_arrival_location()
{
	int i;
	sexp_list_item head;

	for (i=0; i<MAX_ARRIVAL_NAMES; i++)
		head.add_data(Arrival_location_names[i]);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_departure_location()
{
	int i;
	sexp_list_item head;

	for (i=0; i<MAX_DEPARTURE_NAMES; i++)
		head.add_data(Departure_location_names[i]);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_arrival_anchor_all()
{
	int i, restrict_to_players;
	object *objp;
	sexp_list_item head;

	for (restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++)
	{
		for (i = 0; i < Num_iffs; i++)
		{
			char tmp[NAME_LENGTH + 15];
			stuff_special_arrival_anchor_name(tmp, i, restrict_to_players, 0);

			head.add_data(tmp);
		}
	}

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) )
		{
			head.add_data(Ships[objp->instance].ship_name);
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ai_goal(int parent_node)
{
	int i, n, w, z, child;
	sexp_list_item head;

	Assert(parent_node >= 0);
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);
	n = ship_name_lookup(tree_nodes[child].text, 1);
	if (n >= 0) {
		// add operators if it's an ai-goal and ai-goal is allowed for that ship
		for (i=0; i<Num_operators; i++) {
			if ( (query_operator_return_type(i) == OPR_AI_GOAL) && query_sexp_ai_goal_valid(Operators[i].value, n) )
				head.add_op(i);
		}

	} else {
		z = wing_name_lookup(tree_nodes[child].text);
		if (z >= 0) {
			for (w=0; w<Wings[z].wave_count; w++) {
				n = Wings[z].ship_index[w];
				// add operators if it's an ai-goal and ai-goal is allowed for that ship
				for (i=0; i<Num_operators; i++) {
					if ( (query_operator_return_type(i) == OPR_AI_GOAL) && query_sexp_ai_goal_valid(Operators[i].value, n) )
						head.add_op(i);
				}
			}
		// when dealing with the special argument add them all. It's up to the FREDder to ensure invalid orders aren't given
		} else if (!strcmp(tree_nodes[child].text, SEXP_ARGUMENT_STRING)) {
			for (i=0; i<Num_operators; i++) {
				if (query_operator_return_type(i) == OPR_AI_GOAL) {
					head.add_op(i);
				}
			}
		} else
			return NULL;  // no valid ship or wing to check against, make nothing available
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_docker_point(int parent_node)
{
	int i, z, sh;
	sexp_list_item head;

	Assert(parent_node >= 0);
	Assert(!stricmp(tree_nodes[parent_node].text, "ai-dock"));

	z = tree_nodes[parent_node].parent;
	Assert(z >= 0);
	Assert(!stricmp(tree_nodes[z].text, "add-ship-goal") || !stricmp(tree_nodes[z].text, "add-wing-goal") || !stricmp(tree_nodes[z].text, "add-goal"));

	z = tree_nodes[z].child;
	Assert(z >= 0);

	sh = ship_name_lookup(tree_nodes[z].text, 1);
	if (sh >= 0) {
		z = get_docking_list(Ship_info[Ships[sh].ship_info_index].model_num);
		for (i=0; i<z; i++)
			head.add_data(Docking_bay_list[i]);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_dockee_point(int parent_node)
{
	int i, z, sh;
	sexp_list_item head;

	Assert(parent_node >= 0);
	Assert(!stricmp(tree_nodes[parent_node].text, "ai-dock"));

	z = tree_nodes[parent_node].child;
	Assert(z >= 0);

	sh = ship_name_lookup(tree_nodes[z].text, 1);
	if (sh >= 0) {
		z = get_docking_list(Ship_info[Ships[sh].ship_info_index].model_num);
		for (i=0; i<z; i++)
			head.add_data(Docking_bay_list[i]);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_message()
{
	char *str;
	int i;
	sexp_list_item head;

	if (m_mode == MODE_EVENTS) {
		Assert(Event_editor_dlg);
		// this for looks a litle strange, but had to do it get rid of a warning.  Conditional
		//uses last statement is sequence, i.e. same as for (i=0, str, i++)
		for (i=0; str = Event_editor_dlg->current_message_name(i), str; i++)
			head.add_data(str);

	} else {
		for (i=Num_builtin_messages; i<Num_messages; i++)
			head.add_data(Messages[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_persona()
{
	int i;
	sexp_list_item head;

	for (i = 0; i < Num_personas; i++) {
		if (Personas[i].flags & PERSONA_FLAG_WINGMAN) {
			head.add_data (Personas[i].name);
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_who_from()
{
	object *ptr;
	sexp_list_item head;

	//head.add_data("<any allied>");
	head.add_data("#Command");
	head.add_data("<any wingman>");

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START))
			if (!(Ship_info[Ships[get_ship_from_obj(ptr)].ship_info_index].flags & SIF_NOT_FLYABLE))
				head.add_data(Ships[ptr->instance].ship_name);

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_priority()
{
	sexp_list_item head;

	head.add_data("High");
	head.add_data("Normal");
	head.add_data("Low");
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_waypoint_path()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_waypoint_lists; i++)
		head.add_data(Waypoint_lists[i].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_point()
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_point());
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_point()
{
	int i;
	sexp_list_item head;

	for (i = 0; i < Num_iffs; i++)
	{
		char tmp[NAME_LENGTH + 7];
		sprintf(tmp, "<any %s>", Iff_info[i].iff_name);
		strlwr(tmp);
		head.add_data_dup(tmp);
	}
	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	head.add_list(get_listing_opf_point());
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_point_or_none()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship_wing_point());

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_mission_name()
{
	int i;
	sexp_list_item head;

	if ((m_mode == MODE_CAMPAIGN) && (Cur_campaign_mission >= 0)) {
		for (i=0; i<Campaign.num_missions; i++)
			if ( (i == Cur_campaign_mission) || (Campaign.missions[i].level < Campaign.missions[Cur_campaign_mission].level) )
				head.add_data(Campaign.missions[i].name);

	} else
		head.add_data(Mission_filename);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_goal_name(int parent_node)
{
	int i, m;
	sexp_list_item head;

	if (m_mode == MODE_CAMPAIGN) {
		int child;

		Assert(parent_node >= 0);
		child = tree_nodes[parent_node].child;
		Assert(child >= 0);

		for (m=0; m<Campaign.num_missions; m++)
			if (!stricmp(Campaign.missions[m].name, tree_nodes[child].text))
				break;

		if (m < Campaign.num_missions) {
			if (Campaign.missions[m].num_goals < 0)  // haven't loaded goal names yet.
				read_mission_goal_list(m);

			for (i=0; i<Campaign.missions[m].num_goals; i++)
				head.add_data(Campaign.missions[m].goals[i].name);
		}

	} else {
		for (i=0; i<Num_goals; i++)
			head.add_data(Mission_goals[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing()
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_order_recipient()
{
	sexp_list_item head;

	head.add_data("<all fighters>");

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_type()
{
	unsigned int i;
	sexp_list_item head;

	for (i=0; i<Ship_types.size(); i++){
		head.add_data(Ship_types[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_keypress()
{
	int i;
	sexp_list_item head;

	for (i=0; i<CCFG_MAX; i++) {
		if (Control_config[i].key_default > 0) {
			head.add_data_dup(textify_scancode(Control_config[i].key_default));
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_event_name(int parent_node)
{
	int i, m;
	sexp_list_item head;


	if (m_mode == MODE_CAMPAIGN) {
		int child;

		Assert(parent_node >= 0);
		child = tree_nodes[parent_node].child;
		Assert(child >= 0);

		for (m=0; m<Campaign.num_missions; m++)
			if (!stricmp(Campaign.missions[m].name, tree_nodes[child].text))
				break;

		if (m < Campaign.num_missions) {
			if (Campaign.missions[m].num_events < 0)  // haven't loaded goal names yet.
				read_mission_goal_list(m);

			for (i=0; i<Campaign.missions[m].num_events; i++)
				head.add_data(Campaign.missions[m].events[i].name);
		}

	} else {
		for (i=0; i<Num_mission_events; i++)
			head.add_data(Mission_events[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ai_order()
{
	int i;
	sexp_list_item head;

	for (i=0; i<NUM_COMM_ORDER_ITEMS; i++)
		head.add_data(Comm_orders[i].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_skill_level()
{
	int i;
	sexp_list_item head;

	for (i=0; i<NUM_SKILL_LEVELS; i++)
		head.add_data(Skill_level_names(i, 0));

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_cargo()
{
	sexp_list_item head;

	head.add_data("Nothing");
	for (int i=0; i<Num_cargo; i++)
	{
		if (stricmp(Cargo_names[i], "nothing"))
			head.add_data(Cargo_names[i]);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_string()
{
	sexp_list_item head;

	head.add_data("<any string>");

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_medal_name()
{
	int i;
	sexp_list_item head;

	for (i=0; i<MAX_ASSIGNABLE_MEDALS; i++)
		head.add_data(Medals[i].name);

	// also add SOC crest (index 17) and Wings (index 13)
	head.add_data(Medals[13].name);
	head.add_data(Medals[17].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_weapon_name()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_weapon_types; i++)
		head.add_data(Weapon_info[i].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_intel_name()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Intel_info_size; i++)
		head.add_data(Intel_info[i].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_class_name()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_ship_classes; i++)
		head.add_data(Ship_info[i].name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_hud_gauge_name()
{
	int i;
	sexp_list_item head;

	for (i=0; i<NUM_HUD_GAUGES; i++)
		head.add_data(HUD_gauge_text[i]);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_huge_weapon()
{
	int i;
	sexp_list_item head;

	for (i=0; i<Num_weapon_types; i++) {
		if (Weapon_info[i].wi_flags & WIF_HUGE)
			head.add_data(Weapon_info[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_not_player()
{
	object *ptr;
	sexp_list_item head;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_SHIP)
			head.add_data(Ships[ptr->instance].ship_name);

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_ship_or_none()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship());

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_subsystem_or_none(int parent_node, int arg_index)
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_subsys_or_generic(int parent_node, int arg_index)
{
	sexp_list_item head;

	head.add_data(SEXP_ALL_ENGINES_STRING);
	head.add_data(SEXP_ALL_TURRETS_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_jump_nodes()
{
	sexp_list_item head;

	for ( jump_node *jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() ) {	
		head.add_data( jnp->get_name_ptr());
	}

	return head.next;
}

// creates list of Sexp_variables
sexp_list_item *sexp_tree::get_listing_opf_variable_names()
{
	int i;
	sexp_list_item head;

	for (i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			head.add_data( Sexp_variables[i].variable_name );
		}
	}

	return head.next;
}

// get default skybox model name
sexp_list_item *sexp_tree::get_listing_opf_skybox_model()
{

	sexp_list_item head;
	head.add_data("default");
	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_background_bitmap()
{
	sexp_list_item head;
	int i;

	for (i=0; i < stars_get_num_entries(false, true); i++)
 	{
		head.add_data( (char*)stars_get_name_FRED(i, false) );
 	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_sun_bitmap()
{
	sexp_list_item head;
	int i;

	for (i=0; i < stars_get_num_entries(true, true); i++)
 	{
		head.add_data( (char*)stars_get_name_FRED(i, true) );
 	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_nebula_storm_type()
{
	sexp_list_item head;
	int i;

	head.add_data("none");

	for (i=0; i < Num_storm_types; i++)
	{
		head.add_data(Storm_types[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_nebula_poof()
{
	sexp_list_item head;
	int i;

	for (i=0; i < MAX_NEB2_POOFS; i++)
	{
		head.add_data(Neb2_poof_filenames[i]);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_turret_target_order()
{
	int i;
	sexp_list_item head;

	for (i=0; i<NUM_TURRET_ORDER_TYPES; i++)
		head.add_data(Turret_target_order_names[i]);

	return head.next;
}


// Deletes sexp_variable from sexp_tree.
// resets tree to not include given variable, and resets text and type
void sexp_tree::delete_sexp_tree_variable(const char *var_name)
{
	char search_str[64];
	char replace_text[TOKEN_LENGTH];
	
	sprintf(search_str, "%s(", var_name);

	// store old item index
	int old_item_index = item_index;

	for (uint idx=0; idx<tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if ( strstr(tree_nodes[idx].text, search_str) != NULL ) {

				// check type is number or string
				Assert( (tree_nodes[idx].type & SEXPT_NUMBER) || (tree_nodes[idx].type & SEXPT_STRING) );

				// reset type as not variable
				int type = tree_nodes[idx].type &= ~SEXPT_VARIABLE;

				// reset text
				if (tree_nodes[idx].type & SEXPT_NUMBER) {
					strcpy(replace_text, "number");
				} else {
					strcpy(replace_text, "string");
				}

				// set item_index and replace data
				item_index = idx;
				replace_data(replace_text, type);
			}
		}
	}

	// restore item_index
	item_index = old_item_index;
}


// Modify sexp_tree for a change in sexp_variable (name, type, or default value)
void sexp_tree::modify_sexp_tree_variable(const char *old_name, int sexp_var_index)
{
	char search_str[64];
	int type;

	Assert(Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_SET);
	Assert( (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) || (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) );

	// Get type for sexp_tree node
	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
		type = (SEXPT_NUMBER | SEXPT_VALID);
	} else {
		type = (SEXPT_STRING | SEXPT_VALID);
	}
															
	// store item index;
	int old_item_index = item_index;

	// Search string in sexp_tree nodes
	sprintf(search_str, "%s(", old_name);

	for (uint idx=0; idx<tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if ( strstr(tree_nodes[idx].text, search_str) != NULL ) {
				// temp set item_index
				item_index = idx;

				// replace variable data
				replace_variable_data(sexp_var_index, (type | SEXPT_VARIABLE));
			}
		}
	}

	// restore item_index
	item_index = old_item_index;
}


// convert from item_index to sexp_variable index, -1 if not
int sexp_tree::get_item_index_to_var_index()
{
	// check valid item index and node is a variable
	if ( (item_index > 0) && (tree_nodes[item_index].type & SEXPT_VARIABLE) ) {

		return get_tree_name_to_sexp_variable_index(tree_nodes[item_index].text);
	} else {
		return -1;
	}
}

int sexp_tree::get_tree_name_to_sexp_variable_index(const char *tree_name)
{
	char var_name[TOKEN_LENGTH];

	int chars_to_copy = strcspn(tree_name, "(");
	Assert(chars_to_copy < TOKEN_LENGTH - 1);

	// Copy up to '(' and add null termination
	strncpy(var_name, tree_name, chars_to_copy);
	var_name[chars_to_copy] = '\0';

	// Look up index
	return get_index_sexp_variable_name(var_name);
}

int sexp_tree::get_variable_count(const char *var_name)
{
	uint idx;
	int count = 0;
	char compare_name[64];

	// get name to compare
	strcpy(compare_name, var_name);
	strcat(compare_name, "(");

	// look for compare name
	for (idx=0; idx<tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if ( strstr(tree_nodes[idx].text, compare_name) ) {
				count++;
			}
		}
	}

	return count;
}
