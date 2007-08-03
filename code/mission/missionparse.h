/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Source: /cvs/cvsroot/fs2open/fs2_open/code/mission/missionparse.h,v $
 * $Revision: 2.103 $
 * $Author: Goober5000 $
 * $Date: 2007-08-03 01:17:01 $
 *
 * main header file for parsing code  
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.102  2007/07/28 21:17:56  Goober5000
 * make the parse object array dynamic; also made the docking bitstrings dynamic
 *
 * Revision 2.101  2007/07/23 15:16:50  Kazan
 * Autopilot upgrades as described, MSVC2005 project fixes
 *
 * Revision 2.100  2007/02/21 01:44:02  Goober5000
 * remove duplicate model texture replacement
 *
 * Revision 2.99  2007/02/18 06:16:47  Goober5000
 * revert Bobboau's commits for the past two months; these will be added in later in a less messy/buggy manner
 *
 * Revision 2.98  2007/02/11 21:26:35  Goober5000
 * massive shield infrastructure commit
 *
 * Revision 2.97  2007/02/10 03:17:31  Goober5000
 * made support ship shield control a bit less of a hack
 *
 * Revision 2.96  2007/02/08 07:39:32  Goober5000
 * fix two bugs:
 * --default ship flags in the iff_defs table were not correctly translated from parse flags to ship/object flags
 * --ships were created with default allowed comm orders regardless of which team they were on
 *
 * Revision 2.95  2007/01/14 14:03:33  bobboau
 * ok, something aparently went wrong, last time, so I'm commiting again
 * hopefully it should work this time
 * damnit WORK!!!
 *
 * Revision 2.94  2007/01/07 21:28:11  Goober5000
 * yet more tweaks to the WCS death scream stuff
 * added a ship flag to force screaming
 *
 * Revision 2.93  2007/01/07 01:00:18  Goober5000
 * convert a mission variable to a mission flag
 *
 * Revision 2.92  2007/01/07 00:01:28  Goober5000
 * add a feature for specifying the source of Command messages
 *
 * Revision 2.91  2006/09/11 06:50:42  taylor
 * fixes for stuff_string() bounds checking
 *
 * Revision 2.90  2006/07/30 20:01:56  Kazan
 * resolve 1018 and an interface problem in fred2's ship editor
 *
 * Revision 2.89  2006/07/06 21:00:13  Goober5000
 * remove obsolete (and hackish) flag
 * --Goober5000
 *
 * Revision 2.88  2006/07/06 20:46:39  Goober5000
 * WCS screaming stuff
 * --Goober5000
 *
 * Revision 2.87  2006/06/04 01:01:53  Goober5000
 * add fighterbay restriction code
 * --Goober5000
 *
 * Revision 2.86  2006/06/02 09:06:12  karajorma
 * Team Loadout changes to accept variables names as legitimate values for ship class and quantity in loadout.
 * Added the new alt class system
 *
 * Revision 2.85  2006/05/13 07:29:52  taylor
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
 * Revision 2.84  2006/04/20 06:32:07  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.83  2006/04/16 11:58:11  karajorma
 * Some how managed to fail to commit the primary-locked and secondary-locked ship flags.
 * No idea how. Oh well here they are.
 *
 * Revision 2.82  2006/04/05 16:12:41  karajorma
 * Changes to support the new Enable/Disable-Builtin-Messages SEXP
 *
 * Revision 2.81  2006/02/26 23:23:30  wmcoolmon
 * Targetable as bomb SEXPs and dialog stuff; made invulnerable an object flag in both FRED and FS2.
 *
 * Revision 2.80  2006/02/24 07:34:07  taylor
 * fix custom loading screens that I manage to break yet again
 * add a "MaxFPS" registry/ini option to specify a FPS cap, useful if you can't make use of v-sync for some reason
 *
 * Revision 2.79  2006/02/12 01:27:47  Goober5000
 * more cool work on importing, music handling, etc.
 * --Goober5000
 *
 * Revision 2.78  2006/02/02 08:12:47  Goober5000
 * ugh, more ship/wing fixage
 * --Goober5000
 *
 * Revision 2.77  2006/01/31 01:53:37  Goober5000
 * update FSM import for FSPort v3.0
 * --Goober5000
 *
 * Revision 2.76  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.75  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.74  2005/12/29 08:08:36  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.73  2005/11/21 02:43:37  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 * Revision 2.72  2005/11/21 00:46:12  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.71  2005/10/29 22:09:29  Goober5000
 * multiple ship docking implemented for initially docked ships
 * --Goober5000
 *
 * Revision 2.70  2005/09/27 02:36:57  Goober5000
 * clarification
 * --Goober5000
 *
 * Revision 2.69  2005/09/25 18:44:51  taylor
 * fix Subsys_status leak, wasn't a problem in game but can be touchy on exit
 *
 * Revision 2.68  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.67  2005/09/24 01:50:09  Goober5000
 * a bunch of support ship bulletproofing
 * --Goober5000
 *
 * Revision 2.66  2005/08/25 22:40:03  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.65  2005/07/25 05:24:17  Goober5000
 * cleaned up some command line and mission flag stuff
 * --Goober5000
 *
 * Revision 2.64  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.63  2005/07/13 02:01:29  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.62  2005/07/13 00:44:23  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.61  2005/05/11 22:15:26  phreak
 * added mission flag that will not show enemy wing names, just the ship class.
 *
 * Revision 2.60  2005/04/28 05:29:30  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.59  2005/04/12 02:07:00  phreak
 * ambient_light_level added to the mission structure
 *
 * Revision 2.58  2005/04/05 05:53:19  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.57  2005/03/27 12:28:33  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.56  2005/03/25 06:57:35  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.55  2005/01/21 08:56:50  taylor
 * make Subsys_status dynamic but allocate in blocks for speed and to help prevent memory fragmentation
 *
 * Revision 2.54  2005/01/16 22:39:09  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.53  2005/01/11 21:38:50  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.52  2004/12/14 14:46:13  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.51  2004/10/31 02:04:34  Goober5000
 * added Knossos_warp_ani_used flag for taylor
 * --Goober5000
 *
 * Revision 2.50  2004/10/12 22:47:14  Goober5000
 * added toggle-subsystem-scanning ship flag
 * --Goober5000
 *
 * Revision 2.49  2004/10/12 07:34:45  Goober5000
 * added contrail speed threshold
 * --Goober5000
 *
 * Revision 2.48  2004/10/11 22:29:25  Goober5000
 * added the no-bank ship flag (which works) and the affected-by-gravity flag
 * (which won't work until I implement gravity points)
 * --Goober5000
 *
 * Revision 2.47  2004/09/17 07:12:22  Goober5000
 * changed around the logic for the 3D warp effect
 * --Goober5000
 *
 * Revision 2.46  2004/09/01 00:58:46  phreak
 * created the mission flag MISSION_FLAG_USE_NEW_AI
 *
 * this lets old missions use the retail AI so they are balanced.  new missions would
 * be able to select this in fred so designers can take advantage of it.
 *
 * Revision 2.45  2004/08/23 04:32:40  Goober5000
 * warp effect is back to FS2 default
 * --Goober5000
 *
 * Revision 2.44  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.43  2004/06/28 02:13:08  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.42  2004/05/11 02:52:12  Goober5000
 * completed the FRED import conversion stuff that I started ages ago
 * --Goober5000
 *
 * Revision 2.41  2004/05/10 10:51:53  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
 * Revision 2.40  2004/05/10 08:03:30  Goober5000
 * fixored the handling of no lasers and no engines... the tests should check the ship,
 * not the object
 * --Goober5000
 *
 * Revision 2.39  2004/05/03 21:22:22  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.38  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.37  2004/01/30 07:39:08  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.36  2003/10/23 23:48:03  phreak
 * added code for the mission parser to recognize user defined skyboxes
 *
 * Revision 2.35  2003/10/15 22:03:25  Kazan
 * Da Species Update :D
 *
 * Revision 2.34  2003/09/30 04:05:09  Goober5000
 * updated FRED to import FS1 default weapons loadouts as well as missions
 * --Goober5000
 *
 * Revision 2.33  2003/09/28 21:22:59  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 2.32  2003/09/13 08:27:28  Goober5000
 * added some minor things, such as code cleanup and the following:
 * --turrets will not fire at cargo
 * --MAX_SHIELD_SECTIONS substituted for the number 4 in many places
 * --supercaps have their own default message bitfields (distinguished from capships)
 * --turrets are allowed on fighters
 * --jump speed capped at 65m/s, to avoid ship travelling too far
 * --non-huge weapons now scale their damage, instead of arbitrarily cutting off
 * ----Goober5000
 *
 * Revision 2.31  2003/09/13 06:02:06  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.29  2003/09/06 20:41:52  wmcoolmon
 * Added "+Subsystem Repair Ceiling:" after "+Hull Repair Ceiling:" (formerly "+Support Repair Ceiling:"
 *
 * Revision 2.28  2003/09/06 19:09:24  wmcoolmon
 * Added optional mission parameter "+Support Repair Ceiling", which sets what percentage a support ship can repair a ship's hull to.
 *
 * Revision 2.27  2003/05/09 23:51:04  phreak
 * added fields to the "mission" struct to allow for user-specified loading screens
 *
 * Revision 2.26  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.25  2003/03/25 07:03:30  Goober5000
 * added beginning functionality for $Texture Replace implementation in FRED
 * --Goober5000
 *
 * Revision 2.24  2003/03/20 23:20:26  Goober5000
 * comments
 * --Goober500
 *
 * Revision 2.23  2003/03/19 22:49:32  Goober5000
 * added some mission flags
 * --Goober5000
 *
 * Revision 2.22  2003/03/02 02:10:11  Goober5000
 * bumped alternate names from 10 to 25
 * --Goober5000
 *
 * Revision 2.21  2003/01/19 07:02:16  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.20  2003/01/18 23:25:39  Goober5000
 * made "no-subspace-drive" applicable to all ships and fixed a really *STUPID*
 * bug that made FRED keep crashing (missing comma, bleagh!)
 * --Goober5000
 *
 * Revision 2.19  2003/01/18 09:25:41  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.18  2003/01/17 01:48:50  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.17  2003/01/15 05:24:23  Goober5000
 * added texture replacement parse - will be implemented later
 * --Goober5000
 *
 * Revision 2.16  2003/01/13 02:09:12  wmcoolmon
 * Change mission flag for nebula trails. Also changed code to set flags as necessary
 *
 * Revision 2.15  2003/01/11 01:00:25  wmcoolmon
 * Added code for "Ship Trails override Nebula"
 *
 * Revision 2.14  2003/01/03 21:58:08  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.13  2003/01/02 00:35:21  Goober5000
 * added don't-collide-invisible and collide-invisible sexps
 * --Goober5000
 *
 * Revision 2.12  2003/01/01 23:33:33  Goober5000
 * added ship-vaporize and ship-no-vaporize sexps
 * --Goober5000
 *
 * Revision 2.11  2002/12/27 02:57:51  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.10  2002/12/24 07:38:59  Goober5000
 * added a wee cautionary note
 * --Goober5000
 *
 * Revision 2.9  2002/12/23 05:18:52  Goober5000
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
 * Revision 2.8  2002/12/14 17:09:28  Goober5000
 * removed mission flag for fighterbay damage; instead made damage display contingent on whether the fighterbay subsystem is assigned a damage percentage in ships.tbl
 * --Goober5000
 *
 * Revision 2.7  2002/12/14 01:55:04  Goober5000
 * added mission flag to show subsystem damage for fighterbays
 * ~Goober5000~
 *
 * Revision 2.6  2002/12/10 05:43:34  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.5  2002/12/03 23:05:13  Goober5000
 * implemented beam-free-all-by-default mission flag
 *
 * Revision 2.4  2002/11/14 06:15:02  bobboau
 * added nameplate code
 *
 * Revision 2.3  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/30 17:35:22  wmcoolmon
 * Added mission flag "MISSION_FLAG_SUPPORT_REPAIRS_HULL" for toggling Support Ship hull repair on and off
 *
 * Revision 2.1  2002/07/15 02:09:19  wmcoolmon
 * Added support for toggling ship trails
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 26    8/23/99 6:21p Jefff
 * added "no traitor" option to missions (and fred)
 * 
 * 25    8/23/99 5:04p Jefff
 * Added new mission flag to disable built-in messages from playing.
 * Added fred support as well.
 * 
 * 24    8/16/99 3:53p Andsager
 * Add special warp in interface in Fred and saving / reading.
 * 
 * 23    8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 22    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 21    7/26/99 5:50p Dave
 * Revised ingame join. Better? We'll see....
 * 
 * 20    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 19    7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 18    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 17    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 16    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 15    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 14    4/26/99 12:49p Andsager
 * Add protect object from beam support to Fred
 * 
 * 13    3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 12    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 11    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 10    2/23/99 8:11p Dave
 * Tidied up dogfight mode. Fixed TvT ship type problems for alpha wing.
 * Small pass over todolist items.
 * 
 * 9     2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 8     2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 7     2/03/99 12:42p Andsager
 * Add escort priority.  Modify ship_flags_dlg to include field.  Save and
 * Load.  Add escort priority field to ship.
 * 
 * 6     11/14/98 5:32p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 5     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 4     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
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
 * 147   8/31/98 2:06p Dave
 * Make cfile sort the ordering or vp files. Added support/checks for
 * recognizing "mission disk" players.
 * 
 * 146   5/11/98 4:33p Allender
 * fixed ingame join problems -- started to work on new object updating
 * code (currently ifdef'ed out)
 * 
 * 145   5/05/98 11:05p Allender
 * ability to flag mission as "no promotion" where promotions and badges
 * are *not* granted even if they should be.  Slight fix to multiplayer
 * problem where locking_subsys is wrong for players current target
 * 
 * 144   5/04/98 6:06p Lawrance
 * Make red alert mode work!
 * 
 * 143   4/20/98 4:56p Allender
 * allow AI ships to respawn as many times as there are respawns in the
 * mission.  
 * 
 * 142   4/14/98 12:08a Allender
 * save wingman status information in parse object and restore from parse
 * object when respawned
 * 
 * 141   4/13/98 10:25p Hoffoss
 * Added a flag for subspace missions, and for aboard the Galatea or
 * Bastion.
 * 
 * 140   4/06/98 10:24p Dave
 * Fixed up Netgame.respawn for the standalone case.
 * 
 * 139   4/03/98 12:17a Allender
 * new sexpression to detect departed or destroyed.  optionally disallow
 * support ships.  Allow docking with escape pods 
 * 
 * 138   4/02/98 6:31p Lawrance
 * reduce MAX_SUBSYS_STATUS to 125 if DEMO defined
 * 
 * 137   3/26/98 5:24p Allender
 * put in respawn edit box into mission notes dialog.  Made loading of
 * missions/campaign happen when first entering the game setup screen.
 * 
 * 136   3/18/98 10:38p Allender
 * added required "num players" for multiplayer missions.  Put in required
 * "num players" for multiplayer campaigns.  Added campaign editor support
 * to determine "num players"
 * 
 * 135   3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 
 */

#ifndef _PARSE_H
#define _PARSE_H

#include <setjmp.h>
#include "ai/ai.h"
#include "ai/ai_profiles.h"
#include "model/model.h"
#include "object/object.h"
#include "graphics/2d.h"

//WMC - This should be here
#define FS_MISSION_FILE_EXT				NOX(".fs2")

struct wing;
struct p_dock_instance;

#define NUM_NEBULAS			3				// how many background nebulas we have altogether
#define NUM_NEBULA_COLORS	9

// arrival anchor types
// mask should be high enough to avoid conflicting with ship anchors
#define SPECIAL_ARRIVAL_ANCHOR_FLAG				0x1000
#define SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG		0x0100

// update version when mission file format changes, and add approprate code
// to check loaded mission version numbers in the parse code.  Also, be sure
// to update both MissionParse and MissionSave (FRED) when changing the
// mission file format!
#define	MISSION_VERSION 0.10f
#define	FRED_MISSION_VERSION 0.10f

#define WING_PLAYER_BASE	0x80000  // used by Fred to tell ship_index in a wing points to a player

// mission parse flags used for parse_mission() to tell what kind of information to get from the mission file
#define MPF_ONLY_MISSION_INFO	(1 << 0)
#define MPF_IMPORT_FSM			(1 << 1)

// bitfield definitions for missions game types
#define OLD_MAX_GAME_TYPES				4					// needed for compatibility
#define OLD_GAME_TYPE_SINGLE_ONLY	0
#define OLD_GAME_TYPE_MULTI_ONLY		1
#define OLD_GAME_TYPE_SINGLE_MULTI	2
#define OLD_GAME_TYPE_TRAINING		3

#define MAX_MISSION_TYPES				5
#define MISSION_TYPE_SINGLE			(1<<0)
#define MISSION_TYPE_MULTI				(1<<1)
#define MISSION_TYPE_TRAINING			(1<<2)
#define MISSION_TYPE_MULTI_COOP		(1<<3)
#define MISSION_TYPE_MULTI_TEAMS		(1<<4)
#define MISSION_TYPE_MULTI_DOGFIGHT	(1<<5)

#define MISSION_FLAG_SUBSPACE					(1<<0)	// mission takes place in subspace
#define MISSION_FLAG_NO_PROMOTION				(1<<1)	// cannot get promoted or badges in this mission
#define MISSION_FLAG_FULLNEB					(1<<2)	// mission is a full nebula mission
#define MISSION_FLAG_NO_BUILTIN_MSGS			(1<<3)	// disables builtin msgs
#define MISSION_FLAG_NO_TRAITOR					(1<<4)	// player cannot become a traitor
#define MISSION_FLAG_TOGGLE_SHIP_TRAILS			(1<<5)	// toggles ship trails (off in nebula, on outside nebula)
#define MISSION_FLAG_SUPPORT_REPAIRS_HULL		(1<<6)	// Toggles support ship repair of ship hulls
#define MISSION_FLAG_BEAM_FREE_ALL_BY_DEFAULT	(1<<7)	// Beam-free-all by default - Goober5000
#define MISSION_FLAG_CURRENTLY_UNUSED_1			(1<<8)
#define MISSION_FLAG_CURRENTLY_UNUSED_2			(1<<9)
#define MISSION_FLAG_NO_BRIEFING				(1<<10)	// no briefing, jump right into mission - Goober5000
#define MISSION_FLAG_NO_DEBRIEFING				(1<<11)	// no debriefing, just like red-alert - Goober5000
#define MISSION_FLAG_CURRENTLY_UNUSED_3			(1<<12)
#define MISSION_FLAG_ALLOW_DOCK_TREES			(1<<13)	// toggle between hub and tree model for ship docking (see objectdock.cpp) - Gooober5000
#define MISSION_FLAG_2D_MISSION					(1<<14) // Mission is meant to be played top-down style; 2D physics and movement.
#define MISSION_FLAG_CURRENTLY_UNUSED_4			(1<<15)
#define MISSION_FLAG_RED_ALERT					(1<<16)	// a red-alert mission - Goober5000
#define MISSION_FLAG_SCRAMBLE					(1<<17)	// a scramble mission - Goober5000
#define MISSION_FLAG_NO_BUILTIN_COMMAND			(1<<18)	// turns off Command without turning off pilots - Karajorma
#define MISSION_FLAG_PLAYER_START_AI			(1<<19) // Player Starts mission under AI Control (NOT MULTI COMPATABLE) - Kazan
#define MISSION_FLAG_ALL_ATTACK					(1<<20)	// all teams at war - Goober5000
#define MISSION_FLAG_USE_AP_CINEMATICS			(1<<21) // Kazan - use autopilot cinematics

// some mice macros for mission type
#define IS_MISSION_MULTI_COOP			(The_mission.game_type & MISSION_TYPE_MULTI_COOP)
#define IS_MISSION_MULTI_TEAMS		(The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
#define IS_MISSION_MULTI_DOGFIGHT	(The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT)


#define SSIF_NO_SHIELDS		(1<<0)

// Goober5000
typedef struct support_ship_info {
	int		arrival_location;				// arrival location
	int		arrival_anchor;					// arrival anchor
	int		departure_location;				// departure location
	int		departure_anchor;				// departure anchor
	float	max_hull_repair_val;			// % of a ship's hull that can be repaired -C
	float	max_subsys_repair_val;			// same thing, except for subsystems -C
	int		max_support_ships;				// max number of support ships
	int		ship_class;						// ship class of support ship
	int		tally;							// number of support ships so far
	int		support_available_for_species;	// whether support is available for a given species (this is a bitfield)
	int		ssi_flags;						// yarr
} support_ship_info;

typedef struct mission {
	char	name[NAME_LENGTH];
	char	author[NAME_LENGTH];
	float	version;
	char	created[DATE_TIME_LENGTH];
	char	modified[DATE_TIME_LENGTH];
	char	notes[NOTES_LENGTH];
	char	mission_desc[MISSION_DESC_LENGTH];
	int	game_type;
	int	flags;
	int	num_players;									// valid in multiplayer missions -- number of players supported
	uint	num_respawns;									// valid in multiplayer missions -- number of respawns allowed
	support_ship_info	support_ships;		// Goober5000
	char	squad_filename[MAX_FILENAME_LEN];		// if the player has been reassigned to a squadron, this is the filename of the logo, otherwise empty string
	char	squad_name[NAME_LENGTH];				// if the player has been reassigned to a squadron, this is the name of the squadron, otherwise empty string
	char	loading_screen[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];
	char	skybox_model[MAX_FILENAME_LEN];
	char	envmap_name[MAX_FILENAME_LEN];
	int		contrail_threshold;
	int		ambient_light_level;

	// Goober5000
	int	command_persona;
	char command_sender[NAME_LENGTH];

	// Goober5000
	char event_music_name[NAME_LENGTH];
	char briefing_music_name[NAME_LENGTH];
	char substitute_event_music_name[NAME_LENGTH];
	char substitute_briefing_music_name[NAME_LENGTH];

	// Goober5000
	ai_profile_t *ai_profile;
} mission;

// cargo defines
// NOTE: MAX_CARGO MUST REMAIN <= 64 (CARGO_NO_DEPLETE) for NO_DEPLETE to work.
// FURTHER NOTE (Goober5000): If a new flag is added here, the code (particularly in sexp.cpp)
// must be reworked so that all the flags are maintained from function to function
#define CARGO_INDEX_MASK	0xBF
#define CARGO_NO_DEPLETE	0x40		// CARGO_NO_DEPLETE + CARGO_INDEX_MASK must == FF
#define MAX_CARGO				30


// Goober5000 - contrail threshold (previously defined in ShipContrails.cpp)
#define CONTRAIL_THRESHOLD_DEFAULT	45

extern mission The_mission;
extern char Mission_filename[80];  // filename of mission in The_mission (Fred only)

#define	MAX_FORMATION_NAMES	3
#define	MAX_STATUS_NAMES		3
#define	MAX_TEAM_NAMES			4

// defines for arrival locations.  These defines should match their counterparts in the arrival location
// array
#define	MAX_ARRIVAL_NAMES				4
#define	ARRIVE_AT_LOCATION			0
#define	ARRIVE_NEAR_SHIP				1
#define	ARRIVE_IN_FRONT_OF_SHIP		2
#define	ARRIVE_FROM_DOCK_BAY			3

// defines for departure locations.  These defines should match their counterparts in the departure location
// array
#define MAX_DEPARTURE_NAMES			2
#define DEPART_AT_LOCATION				0
#define DEPART_AT_DOCK_BAY				1

#define	MAX_GOAL_TYPE_NAMES	3

// alternate ship type names
#define MAX_ALT_TYPE_NAMES				25
extern char Mission_alt_types[MAX_ALT_TYPE_NAMES][NAME_LENGTH];
extern int Mission_alt_type_count;

// path restrictions
#define MAX_PATH_RESTRICTIONS		10
typedef struct path_restriction_t {
	int num_paths;
	int cached_mask;
	char path_names[MAX_SHIP_BAY_PATHS][MAX_NAME_LEN];
} path_restriction_t;

extern char *Ship_class_names[MAX_SHIP_CLASSES];
extern char *Ai_behavior_names[MAX_AI_BEHAVIORS];
extern char *Formation_names[MAX_FORMATION_NAMES];
extern char *Status_desc_names[MAX_STATUS_NAMES];
extern char *Status_type_names[MAX_STATUS_NAMES];
extern char *Status_target_names[MAX_STATUS_NAMES];
extern char *Arrival_location_names[MAX_ARRIVAL_NAMES];
extern char *Departure_location_names[MAX_DEPARTURE_NAMES];
extern char *Goal_type_names[MAX_GOAL_TYPE_NAMES];

extern char *Reinforcement_type_names[];
extern char *Object_flags[];
extern char *Parse_object_flags[];
extern char *Parse_object_flags_2[];
extern char *Icon_names[];

extern char *Cargo_names[MAX_CARGO];
extern char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

extern char Mission_parse_storm_name[NAME_LENGTH];

extern int	Num_iff;
extern int	Num_ai_behaviors;
extern int	Num_ai_classes;
extern int	Num_cargo;
extern int	Num_status_names;
extern int	Num_arrival_names;
extern int	Num_formation_names;
extern int	Num_goal_type_names;
extern int	Num_team_names;
extern int	Num_reinforcement_type_names;
extern int	Player_starts;
extern fix	Entry_delay_time;
extern int	Fred_num_texture_replacements;	// Goober5000
extern int	Loading_screen_bm_index;

extern ushort Current_file_checksum;
extern int    Current_file_length;

#define SUBSYS_STATUS_NO_CHANGE	-999

typedef struct subsys_status {
	char	name[NAME_LENGTH];
	float	percent;  // percent damaged
	int	primary_banks[MAX_SHIP_PRIMARY_BANKS];
	int primary_ammo[MAX_SHIP_PRIMARY_BANKS];
	int	secondary_banks[MAX_SHIP_SECONDARY_BANKS];
	int	secondary_ammo[MAX_SHIP_SECONDARY_BANKS];
	int	ai_class;
	int	subsys_cargo_name;
} subsys_status;

// Goober5000 - texture replacement info
#define TEXTURE_NAME_LENGTH	128

typedef struct texture_replace {
	char ship_name[NAME_LENGTH];
	char old_texture[TEXTURE_NAME_LENGTH];
	char new_texture[TEXTURE_NAME_LENGTH];
	int new_texture_id;
} texture_replace;

extern texture_replace Fred_texture_replacements[MAX_SHIPS * MAX_MODEL_TEXTURES];


#define MAX_OBJECT_STATUS	10
#define MAX_ALT_CLASS_1		3
#define MAX_ALT_CLASS_2		1	// Karajorma - Team Loadout only needs one of these but I'm generalising the sytstem in case anyone ever needs more

//	a parse object
//	information from a $OBJECT: definition is read into this struct to
// be copied into the real object, ship, etc. structs
typedef struct p_object {
	char	name[NAME_LENGTH];
	p_object *next, *prev;

	vec3d	pos;
	matrix	orient;
	int	ship_class;
	int	team;
	int	behavior;							// ai_class;
	int	ai_goals;							// sexp of lists of goals that this ship should try and do
	char	cargo1;

	int	status_count;
	int	status_type[MAX_OBJECT_STATUS];
	int	status[MAX_OBJECT_STATUS];
	int	target[MAX_OBJECT_STATUS];

	int	subsys_index;						// index into subsys_status array
	int	subsys_count;						// number of elements used in subsys_status array
	int	initial_velocity;
	int	initial_hull;
	int	initial_shields;

	int	arrival_location;
	int	arrival_distance;					// used when arrival location is near or in front of some ship
	int	arrival_anchor;						// ship used for anchoring an arrival point
	int arrival_path_mask;					// Goober5000
	int	arrival_cue;						//	Index in Sexp_nodes of this sexp.
	int	arrival_delay;


	int	departure_location;
	int	departure_anchor;
	int departure_path_mask;				// Goober5000
	int	departure_cue;						//	Index in Sexp_nodes of this sexp.
	int	departure_delay;

	char	misc[NAME_LENGTH];
	int	determination;

	int	wingnum;							// set to -1 if not in a wing -- Wing array index otherwise
	int pos_in_wing;						// Goober5000 - needed for FRED with the new way things work

	int	flags;								// mission savable flags
	int flags2;								// Goober5000
	int	escort_priority;					// priority in escort list
	int	ai_class;
	int	hotkey;								// hotkey number (between 0 and 9) -1 means no hotkey
	int	score;
	int	orders_accepted;					// which orders this ship will accept from the player
	p_dock_instance	*dock_list;				// Goober5000 - parse objects this parse object is docked to
	object *created_object;					// Goober5000
	int	group;								// group object is within or -1 if none.
	int	persona_index;
	float	kamikaze_damage;					// base damage for a kamikaze attack
	int	special_exp_index;
	int special_hitpoint_index;
	ushort net_signature;					// network signature this object can have
	int destroy_before_mission_time;

	char	wing_status_wing_index;			// wing index (0-4) in wingman status gauge
	char	wing_status_wing_pos;			// wing position (0-5) in wingman status gauge

	uint	respawn_count;						// number of respawns for this object.  Applies only to player wing ships in multiplayer
	int	respawn_priority;					// priority this ship has for controlling respawn points

	char	alt_type_index;					// optional alt type index

	float parse_max_hull_strength;
	float parse_max_shield_strength;

	// Goober5000
	int num_texture_replacements;
	texture_replace replacement_textures[MAX_MODEL_TEXTURES];	// replacement textures - Goober5000
	
	// Karajorma - Alternate classes allow the mission designer to specify which classes to try if the one in the 
	// mission file is unacceptable. The method by which this will be done is dependant on the parse_objects flags.
	int num_alt_class_one;						
	int alt_class_one[MAX_ALT_CLASS_1];				// The alt class number (basically the index in Ship_info)
	int alt_class_one_variable[MAX_ALT_CLASS_1];		// The variable backing this entry if any. 

	int num_alt_class_two;  
	int alt_class_two[MAX_ALT_CLASS_2];   
	int alt_class_two_variable[MAX_ALT_CLASS_2];  


} p_object;

// defines for flags used for p_objects when they are created.  Used to help create special
// circumstances for those ships.  This list of bitfield indicators MUST correspond EXACTLY
// (i.e., order and position must be the same) to its counterpart in MissionParse.cpp!!!!

#define MAX_PARSE_OBJECT_FLAGS	24

#define P_SF_CARGO_KNOWN				(1<<0)
#define P_SF_IGNORE_COUNT				(1<<1)
#define P_OF_PROTECTED					(1<<2)
#define P_SF_REINFORCEMENT				(1<<3)
#define P_OF_NO_SHIELDS					(1<<4)
#define P_SF_ESCORT						(1<<5)
#define P_OF_PLAYER_START				(1<<6)
#define P_SF_NO_ARRIVAL_MUSIC			(1<<7)
#define P_SF_NO_ARRIVAL_WARP			(1<<8)
#define P_SF_NO_DEPARTURE_WARP			(1<<9)
#define P_SF_LOCKED						(1<<10)
#define P_OF_INVULNERABLE				(1<<11)
#define P_SF_HIDDEN_FROM_SENSORS		(1<<12)
#define P_SF_SCANNABLE					(1<<13)	// ship is a "scannable" ship
#define P_AIF_KAMIKAZE					(1<<14)
#define P_AIF_NO_DYNAMIC				(1<<15)
#define P_SF_RED_ALERT_STORE_STATUS		(1<<16)
#define P_OF_BEAM_PROTECTED				(1<<17)
#define P_SF_GUARDIAN					(1<<18)
#define P_KNOSSOS_WARP_IN				(1<<19)
#define P_SF_VAPORIZE					(1<<20)
#define P_SF2_STEALTH					(1<<21)
#define P_SF2_FRIENDLY_STEALTH_INVIS	(1<<22)
#define P_SF2_DONT_COLLIDE_INVIS		(1<<23)

// 24 and 25 are currently unused

// the following parse object flags are used internally by FreeSpace
#define P_SF_USE_UNIQUE_ORDERS		(1<<26)	// tells a newly created ship to use the default orders for that ship
#define P_SF_DOCK_LEADER			(1<<27)	// Goober5000 - a docked parse object that is the leader of its group
#define P_SF_CANNOT_ARRIVE			(1<<28)	// used to indicate that this ship's arrival cue will never be true
#define P_SF_WARP_BROKEN			(1<<29)	// warp engine should be broken for this ship
#define P_SF_WARP_NEVER				(1<<30)	// warp drive is destroyed
#define P_SF_PLAYER_START_VALID		(1<<31)	// this is a valid player start object

// more parse flags! -- Goober5000
// same caveat: This list of bitfield indicators MUST correspond EXACTLY
// (i.e., order and position must be the same) to its counterpart in MissionParse.cpp!!!!

#define MAX_PARSE_OBJECT_FLAGS_2	14

#define P2_SF2_PRIMITIVE_SENSORS			(1<<0)
#define P2_SF2_NO_SUBSPACE_DRIVE			(1<<1)
#define P2_SF2_NAV_CARRY_STATUS				(1<<2)
#define P2_SF2_AFFECTED_BY_GRAVITY			(1<<3)
#define P2_SF2_TOGGLE_SUBSYSTEM_SCANNING	(1<<4)
#define P2_OF_TARGETABLE_AS_BOMB			(1<<5)
#define P2_SF2_NO_BUILTIN_MESSAGES			(1<<6)
#define P2_SF2_PRIMARIES_LOCKED				(1<<7)
#define P2_SF2_SECONDARIES_LOCKED			(1<<8)
#define P2_SF2_SET_CLASS_DYNAMICALLY		(1<<9)
#define P2_SF2_TEAM_LOADOUT_STORE_STATUS	(1<<10)
#define P2_SF2_NO_DEATH_SCREAM				(1<<11)
#define P2_SF2_ALWAYS_DEATH_SCREAM			(1<<12)
#define P2_SF2_NAV_NEEDSLINK				(1<<13)

// and again: these flags do not appear in the array
//#define blah							(1<<29)
//#define blah							(1<<30)
#define P2_ALREADY_HANDLED				(1<<31)	// Goober5000 - used for docking currently, but could be used generically


// Goober5000 - this is now dynamic
extern std::vector<p_object> Parse_objects;
#define POBJ_INDEX(pobjp) (pobjp - &Parse_objects[0])	// yes, this arithmetic is valid :D

extern p_object Support_ship_pobj, *Arriving_support_ship;
extern p_object Ship_arrival_list;

// Karajorma - Team Loadout stuff
#define SHIP_CURRENTLY_UNASSIGNED		-1

#define SHIP_CLASS_NOT_IN_LOADOUT		-1
#define SHIP_CLASS_ALL_ASSIGNED			-2

#define TOKEN_LENGTH	32

typedef struct {
	int		default_ship;  // default ship type for player start point (recommended choice)
	int		number_choices; // number of ship choices inside ship_list
	int		loadout_total;	// Total number of ships available of all classes 
	int		ship_list[MAX_SHIP_CLASSES];
	char	ship_list_variables[MAX_SHIP_CLASSES][TOKEN_LENGTH];
	int		ship_count[MAX_SHIP_CLASSES];
	char	ship_count_variables[MAX_SHIP_CLASSES][TOKEN_LENGTH];
	int		weaponry_pool[MAX_WEAPON_TYPES];
	int		weapon_variable[MAX_WEAPON_TYPES];
} team_data;

#define MAX_P_WINGS		16
#define MAX_SHIP_LIST	16

extern team_data Team_data[MAX_TVT_TEAMS];
//extern subsys_status Subsys_status[MAX_SUBSYS_STATUS]; // it's dynamic now - taylor
extern subsys_status *Subsys_status;
extern int Subsys_index;

extern vec3d Parse_viewer_pos;
extern matrix Parse_viewer_orient;

extern int Mission_arrival_timestamp;
extern int Mission_departure_timestamp;
extern fix Mission_end_time;

extern char Parse_names[MAX_SHIPS + MAX_WINGS][NAME_LENGTH];
extern int Num_parse_names;
extern int Num_teams;

extern char			Player_start_shipname[NAME_LENGTH];
extern int			Player_start_shipnum;
extern p_object	Player_start_pobject;

extern int Mission_palette;  // index of palette file to use for mission
extern int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
extern char *Nebula_filenames[NUM_NEBULAS];
extern char *Nebula_colors[NUM_NEBULA_COLORS];
extern p_object *Arriving_support_ship;

extern char Neb2_texture_name[MAX_FILENAME_LEN];


int parse_main(char *mission_name, int flags = 0);
p_object *mission_parse_get_arrival_ship(ushort net_signature);
p_object *mission_parse_get_arrival_ship(char *name);
p_object *mission_parse_get_parse_object(ushort net_signature);
p_object *mission_parse_get_parse_object(char *name);
int parse_create_object(p_object *objp);
void resolve_parse_flags(object *objp, int parse_flags, int parse_flags2);

void mission_parse_close();

// used in squadmate messaging stuff to create wings from reinforcements.
int parse_wing_create_ships(wing *wingp, int num_to_create, int force = 0, int specific_instance = -1 );

// function for getting basic mission data without loading whole mission
int mission_parse_is_multi(char *filename, char *mission_name );
int mission_parse_get_multi_mission_info(char *filename);

// called externally from multiplayer code
int mission_do_departure(object *objp);

// called externally from freespace.cpp
void mission_parse_fixup_players(void);

// get a index to a perminently kept around name of a ship or wing
int get_parse_name_index(char *name);

// called from freespace game level loop
void mission_parse_eval_stuff();

// function to set the ramaing time left in the mission
void mission_parse_set_end_time( int seconds );

// code to bring in a repair ship.
void mission_bring_in_support_ship( object *requester_objp );
int mission_is_support_ship_arriving( void );
void mission_add_to_arriving_support( object *requester_objp );
int mission_is_repair_scheduled( object *objp );
int mission_remove_scheduled_repair( object *objp );
void mission_parse_support_arrived( int objnum );

// alternate name stuff
int mission_parse_lookup_alt(char *name);
void mission_parse_lookup_alt_index(int index, char *out);
int mission_parse_add_alt(char *name);
void mission_parse_reset_alt();

// code to save/restore mission parse stuff
int get_mission_info(char *filename, mission *missionp = NULL, bool basic = true);

// Goober5000
void parse_dock_one_docked_object(p_object *pobjp, p_object *parent_pobjp);

// Goober5000
extern int Knossos_warp_ani_used;

// Karajorma
void swap_parse_object(p_object *p_obj, int ship_class);
int is_ship_available_from_loadout(team_data *current_team, int ship_class);
int is_ship_in_loadout(team_data *current_team, int ship_class);

#endif

