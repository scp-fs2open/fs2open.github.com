/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Weapons.cpp $
 * <insert description of file here>
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.111  2005/04/28 05:29:30  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.110  2005/04/28 01:39:14  wmcoolmon
 * stuff_byte to stuff_ubyte
 *
 * Revision 2.109  2005/04/25 00:34:00  wmcoolmon
 * Use parse_sound instead of code chunks
 *
 * Revision 2.108  2005/04/24 12:47:36  taylor
 * little cleanup of laser rendering
 *  - fix animated glows
 *  - proper alpha modification (still messed up somewhere though, shows in neb missions)
 *  - remove excess batch_render()
 *
 * Revision 2.107  2005/04/24 07:31:34  Goober5000
 * made swarm/corkscrew/flak conflict more friendly
 * --Goober5000
 *
 * Revision 2.106  2005/04/15 06:23:18  wmcoolmon
 * Local codebase commit; adds armor system.
 *
 * Revision 2.105  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.104  2005/04/02 21:34:08  phreak
 * put First_secondary_index back in so FRED can compile.
 * The weapons list is also sorted whenever all loading is done so lasers and beams always
 * come before missiles.  FRED's loadout code needs this behavior to stick.
 *
 * Revision 2.103  2005/03/30 02:32:41  wmcoolmon
 * Made it so *Snd fields in ships.tbl and weapons.tbl take the sound name
 * as well as its index (ie "L_sidearm.wav" instead of "76")
 *
 * Revision 2.102  2005/03/27 12:28:35  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.101  2005/03/25 06:57:38  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.100  2005/03/24 23:24:01  taylor
 * use SWARM_MISSILE_DELAY again so that it's easier to keep up with
 * fix compiler warnings
 *
 * Revision 2.99  2005/03/19 18:02:35  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.98  2005/03/16 01:35:59  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.97  2005/03/12 21:08:01  phreak
 * doubled default spawn angle from 180 to 360 since it seems to track better
 *
 * Revision 2.96  2005/03/10 08:00:17  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.95  2005/03/08 04:41:39  Goober5000
 * whoops
 *
 * Revision 2.94  2005/03/08 03:50:18  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.93  2005/03/03 16:40:29  taylor
 * animated beam muzzle glows
 *
 * Revision 2.92  2005/03/02 21:24:48  taylor
 * more NO_NETWORK/INF_BUILD goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.91  2005/03/01 06:55:45  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.90  2005/02/20 23:11:51  wmcoolmon
 * Fix0r3d trails
 *
 * Revision 2.89  2005/02/20 07:39:14  wmcoolmon
 * Trails update: Better, faster, stronger, but not much more reliable
 *
 * Revision 2.88  2005/02/19 07:54:33  wmcoolmon
 * Removed trails limit
 *
 * Revision 2.87  2005/02/18 04:54:44  wmcoolmon
 * Added $Tech Model (after +Tech Description)
 *
 * Revision 2.86  2005/02/14 23:59:23  taylor
 * make hudparse GCC 3.4 friendly (WMCoolmon way want to check this with tbl)
 * fix OSX compile problem
 * debug message in weapons_page_in() should have been real debug message
 *
 * Revision 2.85  2005/02/04 20:06:10  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.84  2005/02/03 01:26:45  phreak
 * revert to default d-missile behavior if the electroinics parameters aren't specified.
 * added an option to customize the old-style disruption calculation as well.
 *
 * Revision 2.83  2005/01/28 04:05:05  phreak
 * shockwave weapons now work properly with the electronics (d-missible) tag
 * added in a "randomness factor" for electronics parameters.  This adds or subtracts
 * a random value between 0 and this factor to the disruption time.  This was hard coded
 * to be 4 seconds in retail.
 *
 * Revision 2.82  2005/01/11 04:05:23  taylor
 * fully working (??) -loadonlyused, allocate used_weapons[] and ship_class_used[] only when needed
 *
 * Revision 2.81  2005/01/05 23:17:38  taylor
 * make sure we're not loading weapons that are left over from larger missions
 *
 * Revision 2.80  2005/01/03 18:46:03  taylor
 * stupid mistake
 *
 * Revision 2.79  2004/12/25 09:25:41  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD
 *
 * Revision 2.78  2004/12/23 23:35:02  wmcoolmon
 * Added +Hud Image: for weapons, which replaces the weapon name in the list with an image.
 *
 * Revision 2.77  2004/11/21 11:38:17  taylor
 * support for animated beam sections
 * various weapon-only-used fixes
 * remove the -1 frame fix since it was fixed elsewhere
 *
 * Revision 2.76  2004/11/02 08:32:38  taylor
 * animate weapon glow/bitmaps properly while in flight, make sure
 * we don't exceed bitmap frame numbers, don't try to calculate
 * frame for particle spew animations since particle_render_all()
 * does that for us already
 *
 * Revision 2.75  2004/09/10 13:48:34  et1
 * Implemented "+WeaponMinRange" token
 *
 * Revision 2.74  2004/07/31 08:58:07  et1
 * Implemented "+SwarmWait:"-token and fixed weapon glow tails
 *
 * Revision 2.73  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.72  2004/07/17 09:25:59  taylor
 * add CF_SORT_REVERSE to real sort routine, makes CF_SORT_TIME work again
 *
 * Revision 2.71  2004/07/14 01:27:01  wmcoolmon
 * Better -load_only_used handling; added mark_weapon_used(weapon ID), which does check for IDs of -1.
 *
 * Revision 2.70  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.69  2004/07/11 03:22:54  bobboau
 * added the working decal code
 *
 * Revision 2.68  2004/07/01 01:54:59  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.67  2004/06/29 06:02:34  wmcoolmon
 * Support as well as the extern for "-load_only_used"; this way everything that includes cmdline.h doesn't have to be recompiled.
 *
 * Revision 2.66  2004/06/22 23:14:10  wmcoolmon
 * Nonworking OGG support for sound (not music) added, disabled load-only-used-weapons code, modification to sound system registry code.
 * OGG code has been commented out, so you don't need the SDK yet.
 *
 * Revision 2.65  2004/06/18 04:59:55  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.64  2004/06/05 19:15:39  phreak
 * spawn weapons can now be specified to be spawned at different angles other than
 * the sphere thats used in retail
 *
 * Revision 2.63  2004/05/26 21:02:27  wmcoolmon
 * Added weapons_expl modular table, updated cfilesystem to work with modular tables, fixed loading order, fixed ship loading error messages
 *
 * Revision 2.62  2004/05/26 03:52:08  wmcoolmon
 * Ship & weapon modular table files
 *
 * Revision 2.61  2004/04/06 05:28:07  phreak
 * oops, hanging ')'
 *
 * Revision 2.60  2004/04/06 05:26:59  phreak
 * properly commented Local SSM code.
 * put in some estimation code so the Local SSMs warp in closer to moving ships
 *
 * Revision 2.59  2004/04/06 04:53:21  phreak
 * Local SSMs now working again after the code for setting the jumpout/jumpin
 * times for the missile were removed.
 *
 * Revision 2.58  2004/03/20 14:47:14  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.57  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.56  2004/03/06 23:28:24  bobboau
 * fixed motion debris
 * animated laser textures
 * and added a new error check called a safepoint, mostly for tracking the 'Y bug'
 *
 * Revision 2.55  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.54  2004/02/20 04:29:57  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.53  2004/02/14 00:18:37  randomtiger
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
 * Revision 2.52  2004/02/05 14:31:45  Goober5000
 * fixed a few random bugs
 * --Goober5000
 *
 * Revision 2.51  2004/02/04 04:28:14  Goober5000
 * fixed Asserts in two places and commented out an unneeded variable
 * --Goober5000
 *
 * Revision 2.50  2004/01/30 07:39:09  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.49  2004/01/20 22:10:01  Goober5000
 * heat-seekers now home in on hidden ships
 * --Goober5000
 *
 * Revision 2.48  2003/12/17 20:38:26  phreak
 * fixed some logic when dealing with parsing local ssms
 *
 * Revision 2.47  2003/12/17 16:41:25  phreak
 * "small only" weapons flag added. weapon shoots at small ships like fighters
 *
 * Revision 2.46  2003/12/02 03:16:16  Goober5000
 * fixed CVS log header so that changes would update more cleanly
 * --Goober5000
 *
 * Revision 2.45  2003/11/22 10:36:31  fryday
 * Changed default alpha value for lasers in OpenGL to be like in D3D
 * Fixed Glowmaps being rendered with GL_NEAREST instead of GL_LINEAR.
 * Dynamic Lighting almost there. It looks like some normals are fudged or something.
 *
 * Revision 2.44  2003/11/11 02:15:41  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.43  2003/11/09 07:36:51  Goober5000
 * fixed spelling
 * --Goober5000
 *
 * Revision 2.42  2003/10/25 06:56:07  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.41  2003/10/13 05:57:50  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.40  2003/09/26 14:37:16  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.39  2003/09/13 08:27:27  Goober5000
 * added some minor things, such as code cleanup and the following:
 * --turrets will not fire at cargo
 * --MAX_SHIELD_SECTIONS substituted for the number 4 in many places
 * --supercaps have their own default message bitfields (distinguished from capships)
 * --turrets are allowed on fighters
 * --jump speed capped at 65m/s, to avoid ship travelling too far
 * --non-huge weapons now scale their damage, instead of arbitrarily cutting off
 * ----Goober5000
 *
 * Revision 2.38  2003/09/13 06:02:04  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.35  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.34  2003/08/06 17:36:17  phreak
 * preliminary work on tertiary weapons. it doesn't really function yet, but i want to get something committed
 *
 * Revision 2.33  2003/06/25 03:21:03  phreak
 * added support for weapons that only fire when tagged
 * also added limited firing range for local ssms
 *
 * Revision 2.32  2003/06/14 22:56:17  phreak
 * fixed a bug where local ssms may miss the warphole when entering subspace
 *
 * Revision 2.31  2003/06/13 15:33:57  phreak
 * added a warning that will display if a modder puts in a local ssm that:
 * is a laser or beam
 * doesn't home
 *
 * Revision 2.30  2003/06/12 21:21:26  phreak
 * local ssms fired without lock will not enter subspace at all
 *
 * Revision 2.29  2003/06/12 17:45:54  phreak
 * local ssm warpin is now handled better than what i committed earlier
 *
 * Revision 2.28  2003/06/12 16:54:06  phreak
 * fixed a minor bug where local ssms will warpin at 0,0,0 if their target was destroyed while in subspace
 *
 * Revision 2.27  2003/06/11 03:13:39  phreak
 * i hate to update again so soon, but there was a compile warning that i accidently passed over
 *
 * Revision 2.26  2003/06/11 03:06:07  phreak
 * local subspace missiles are now in game. yay
 *
 * Revision 2.25  2003/05/05 20:55:44  Goober5000
 * fixed small bug inadvertently added by Phreak affecting the custom hitpoints
 * mod; also fixed a small bug in the new disruption mod (disrupting all subsystems,
 * as opposed to only those within the weapon radius)
 * --Goober5000
 *
 * Revision 2.24  2003/05/04 19:51:28  phreak
 * fixed mispelling
 *
 * Revision 2.23  2003/05/03 23:47:04  phreak
 * added multipliers for beam turrets and sensors for disruptor missiles
 *
 * Revision 2.22  2003/05/03 16:48:08  phreak
 * changed around the way disruptor weapons work
 *
 * Revision 2.21  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.20  2003/03/29 09:42:05  Goober5000
 * made beams default shield piercing again
 * also added a beam no pierce command line flag
 * and fixed something else which I forgot :P
 * --Goober5000
 *
 * Revision 2.19  2003/03/19 09:05:26  Goober5000
 * more housecleaning, this time for debug warnings
 * --Goober5000
 *
 * Revision 2.18  2003/03/18 10:07:06  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.4  2002/11/11 20:18:30  phreak
 * updated parse_weapon to parse custom corkscrew missiles
 *
 * Revision 2.3  2002/11/06 23:22:05  phreak
 * Parser error handling for fighter flak, it didn't want flak on player weapons, now it doesn't care
 *
 * Revision 2.2  2002/10/19 19:29:29  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 * Revision 2.1.2.2  2002/09/28 22:13:43  randomtiger
 * Sorted out some bits and pieces. The background nebula blends now which is nice. – RT
 *
 * Revision 2.1.2.1  2002/09/24 18:56:46  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.3  2002/05/10 20:42:45  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 69    9/14/99 3:26a Dave
 * Fixed laser fogging problem in nebula (D3D)> Fixed multiplayer
 * respawn-too-early problem. Made a few crash points safe.
 * 
 * 68    9/14/99 1:32a Andsager
 * Better LOD for weapon explosions when behind.  Move point ahead to get
 * vertex and then find size.
 * 
 * 67    9/07/99 1:10p Mikek
 * Fix code I busted due to adding lifeleft to missiles.
 * 
 * 66    9/07/99 12:20a Andsager
 * LOD less agressive at lower hardware detail level
 * 
 * 65    9/06/99 7:21p Dave
 * Commented out bad lifeleft scaling code in weapon_set_tracking_info()
 * 
 * 64    9/06/99 3:23p Andsager
 * Make fireball and weapon expl ani LOD choice look at resolution of the
 * bitmap
 * 
 * 63    9/06/99 12:46a Andsager
 * Add weapon_explosion_ani LOD
 * 
 * 62    9/05/99 11:24p Mikek
 * Fixed problems caused by earlier checkin (that was rolled back).
 * Problem was wp->target_pos was not set for swarmers.
 * 
 * More tweaking of missile behavior.  Also add 20% to lifetime of a
 * locked missile.
 * 
 * [Rolled back -- MK] 63    9/05/99 2:23p Mikek
 * Make aspect seekers a little less likely to miss their target.
 * Mysterious why they do it so often.  Maybe fix for FS3...
 * 
 * [Rolled back -- MK] 62    9/04/99 12:09p Mikek
 * Limit number of spawned weapons that can home on player based on skill
 * level.  Works same as for non-spawned weapons.  Only do in single
 * player.
 * 
 * 61    8/27/99 1:34a Andsager
 * Modify damage by shockwaves for BIG|HUGE ships.  Modify shockwave damge
 * when weapon blows up.
 * 
 * 60    8/24/99 10:47a Jefff
 * tech room weapon anims.  added tech anim field to weapons.tbl
 * 
 * 59    8/16/99 11:58p Andsager
 * Disable collision on proximity for ships with SIF_DONT_COLLIDE_INVIS
 * hulls.
 * 
 * 58    8/10/99 5:30p Jefff
 * Added tech_title string to weapon_info.  Changed parser accordingly.
 * 
 * 57    8/05/99 2:06a Dave
 * Whee.
 * 
 * 56    8/02/99 5:16p Dave
 * Bumped up weapon title string length from 32 to 48
 * 
 * 55    7/29/99 5:41p Jefff
 * Sound hooks for cmeasure success
 * 
 * 54    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 53    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 52    7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 51    7/16/99 1:50p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 50    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 49    7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 48    7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 47    7/01/99 5:57p Johnson
 * Oops. Fixed big ship damage.
 * 
 * 46    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 45    6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 44    6/22/99 3:24p Danw
 * Fixed incorrect weapon hit sound culling.
 * 
 * 43    6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 42    6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 41    6/11/99 11:13a Dave
 * last minute changes before press tour build.
 * 
 * 40    6/04/99 2:16p Dave
 * Put in shrink effect for beam weapons.
 * 
 * 39    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 38    6/01/99 3:52p Dave
 * View footage screen. Fixed xstrings to not display the & symbol. Popup,
 * dead popup, pxo find player popup, pxo private room popup.
 * 
 * 37    5/27/99 6:17p Dave
 * Added in laser glows.
 * 
 * 36    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 35    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 34    4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 33    4/28/99 3:11p Andsager
 * Stagger turret weapon fire times.  Make turrets smarter when target is
 * protected or beam protected.  Add weaopn range to weapon info struct.
 * 
 * 32    4/22/99 11:06p Dave
 * Final pass at beam weapons. Solidified a lot of stuff. All that remains
 * now is to tweak and fix bugs as they come up. No new beam weapon
 * features.
 * 
 * 31    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 30    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 29    4/07/99 6:22p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 28    4/02/99 1:35p Dave
 * Removed weapon hit packet. No good for causing pain.
 * 
 * 27    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 26    3/31/99 9:26p Dave
 * Don't load beam textures when in Fred.
 * 
 * 25    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 24    3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 23    3/23/99 11:03a Dave
 * Added a few new fields and fixed parsing code for new weapon stuff.
 * 
 * 22    3/19/99 9:52a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 24    3/15/99 6:45p Daveb
 * Put in rough nebula bitmap support.
 * 
 * 23    3/12/99 3:19p Enricco
 * Remove spurious Int3()
 * 
 * 22    3/11/99 5:53p Dave
 * More network optimization. Spliced in Dell OEM planet bitmap crap.
 * 
 * 21    3/10/99 6:51p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 20    2/24/99 4:02p Dave
 * Fixed weapon locking and homing problems for multiplayer dogfight mode.
 * 
 * 19    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 18    1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 17    1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 16    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 15    1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 14    1/21/99 10:45a Dave
 * More beam weapon stuff. Put in warmdown time.
 * 
 * 13    1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 12    1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 11    1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 10    12/01/98 6:12p Johnson
 * Make sure to page in weapon impact animations as xparent textures.
 * 
 * 9     11/20/98 4:08p Dave
 * Fixed flak effect in multiplayer.
 * 
 * 8     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 7     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 6     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 5     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 4     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 3     10/07/98 4:49p Andsager
 * don't do weapon swap (was needed for mission disk)
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 314   9/21/98 11:19p Dave
 * Weapon name fix.
 * 
 * 313   9/19/98 4:33p Adam
 * Changed default values for particle spew (used on Leech Cannon)
 * 
 * 312   9/13/98 10:51p Dave
 * Put in newfangled icons for mission simulator room. New mdisk.vp
 * checksum and file length.
 * 
 * 311   9/13/98 4:29p Andsager
 * Maintain Weapon_info compataiblity with mission disk
 * 
 * 310   9/13/98 4:26p Andsager
 * 
 * 309   9/01/98 4:25p Dave
 * Put in total (I think) backwards compatibility between mission disk
 * freespace and non mission disk freespace, including pilot files and
 * campaign savefiles.
 * 
 * 308   8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 307   8/25/98 1:49p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 306   8/18/98 10:15a Dave
 * Touchups on the corkscrew missiles. Added particle spewing weapons.
 * 
 * 305   8/17/98 5:07p Dave
 * First rev of corkscrewing missiles.
 * 
 * 304   6/30/98 2:23p Dave
 * Revised object update system. Removed updates for all weapons. Put
 * button info back into control info packet.
 * 
 * 303   6/22/98 8:36a Allender
 * revamping of homing weapon system.  don't send as object updates
 * anymore
 * 
 * 302   5/24/98 2:25p Allender
 * be sure that homing missiles die on client when lifeleft gets too
 * negative (lost packets)
 * 
 * 301   5/20/98 5:47p Sandeep
 * 
 * 300   5/18/98 1:58a Mike
 * Make Phoenix not be fired at fighters (but yes bombers).
 * Improve positioning of ships in guard mode.
 * Make turrets on player ship not fire near end of support ship docking.
 * 
 * $NoKeywords: $
 */

//#include <stdlib.h>

#include "PreProcDefines.h"

#include "weapon/weapon.h"
#include "render/3d.h"
#include "object/object.h"
#include "ship/ship.h"
#include "fireball/fireballs.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"
#include "radar/radar.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "cmeasure/cmeasure.h"
#include "weapon/shockwave.h"
#include "math/staticrand.h"
#include "weapon/swarm.h"
#include "ship/shiphit.h"
#include "hud/hud.h"
#include "object/objcollide.h"
#include "ai/aibig.h"
#include "particle/particle.h"
#include "asteroid/asteroid.h"
#include "io/joy_ff.h"
#include "weapon/corkscrew.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "weapon/flak.h"
#include "weapon/muzzleflash.h"
#include "cmdline/cmdline.h"
#include "graphics/grbatch.h"
#include "parse/parselo.h"
#include "radar/radarsetup.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#endif


#ifndef NDEBUG
int Weapon_flyby_sound_enabled = 1;
DCF_BOOL( weapon_flyby, Weapon_flyby_sound_enabled )
#endif

static int Weapon_flyby_sound_timer;	

weapon Weapons[MAX_WEAPONS];
weapon_info Weapon_info[MAX_WEAPON_TYPES];
tertiary_weapon_info Tertiary_weapon_info[MAX_TERTIARY_WEAPON_TYPES];
int Num_tertiary_weapon_types=0;

#define		MISSILE_OBJ_USED	(1<<0)			// flag used in missile_obj struct
#define		MAX_MISSILE_OBJS	MAX_WEAPONS		// max number of missiles tracked in missile list
missile_obj Missile_objs[MAX_MISSILE_OBJS];	// array used to store missile object indexes
missile_obj Missile_obj_list;						// head of linked list of missile_obj structs


// WEAPON EXPLOSION INFO
#define MAX_weapon_expl_lod						4
#define MAX_Weapon_expl_info					20

typedef struct weapon_expl_lod {
	char	filename[MAX_FILENAME_LEN];
	int	bitmap_id;
	int	num_frames;
	int	fps;
} weapon_expl_lod;

typedef struct weapon_expl_info	{
	int					lod_count;	
	weapon_expl_lod		lod[MAX_weapon_expl_lod];
} weapon_expl_info;

weapon_expl_info Weapon_expl_info[MAX_Weapon_expl_info];

int Num_weapon_expl = 0;

int Num_weapon_types = 0;

int Num_weapons = 0;
int Weapons_inited = 0;

int laser_model_inner = -1;
int laser_model_outer = -1;

int missile_model = -1;

char	*Weapon_names[MAX_WEAPON_TYPES];

int     First_secondary_index = -1;

extern int Cmdline_load_only_used;
static int *used_weapons = NULL;

#define	MAX_SPAWN_WEAPONS	10			//	Up to 10 weapons can spawn weapons.

int	Num_spawn_types;
char	Spawn_names[MAX_SPAWN_WEAPONS][NAME_LENGTH];

int Num_player_weapon_precedence;				// Number of weapon types in Player_weapon_precedence
int Player_weapon_precedence[MAX_WEAPON_TYPES];	// Array of weapon types, precedence list for player weapon selection

// Used to avoid playing too many impact sounds in too short a time interval.
// This will elimate the odd "stereo" effect that occurs when two weapons impact at 
// nearly the same time, like from a double laser (also saves sound channels!)
#define	IMPACT_SOUND_DELTA	50		// in milliseconds
int		Weapon_impact_timer;			// timer, initalized at start of each mission

// energy suck defines
#define ESUCK_DEFAULT_WEAPON_REDUCE				(10.0f)
#define ESUCK_DEFAULT_AFTERBURNER_REDUCE		(10.0f)

// Goober5000 - as a rule of thumb, it looks like these are just the complement of the cutoff
// (i.e. supercap used to be cut off at 75% but now uses the existing scale of 25%)

// scale factor for supercaps taking damage from weapons which are not "supercap" weapons
#define SUPERCAP_DAMAGE_SCALE			0.25f

// scale factor for capital ships - added by Goober5000 to accompany SUPERCAP_DAMAGE_SCALE
#define CAPITAL_DAMAGE_SCALE			0.90f

// scale factor for big ships getting hit by flak
#define FLAK_DAMAGE_SCALE				0.05f

// time delay between each swarm missile that is fired
#define SWARM_MISSILE_DELAY				150

extern int Max_allowed_player_homers[];
extern int compute_num_homing_objects(object *target_objp);

// 
void parse_weapon_expl_tbl(char* longname)
{
	int	rval, idx;
	char base_filename[256] = "";

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		Error(LOCATION, "Unable to parse %s!  Code = %i.\n", rval, longname);
	}
	else {
		read_file_text(NOX(longname));
		reset_parse();		
	}

	required_string("#Start");
	while (required_string_either("#End","$Name:")) {
		Assert( Num_weapon_expl < MAX_Weapon_expl_info);

		// base filename
		required_string("$Name:");
		stuff_string(base_filename, F_NAME, NULL);

		// # of lod levels - make sure old fireball.tbl is compatible
		Weapon_expl_info[Num_weapon_expl].lod_count = 1;
		if(optional_string("$LOD:")){
			stuff_int(&Weapon_expl_info[Num_weapon_expl].lod_count);
		}

		// stuff default filename
		strcpy(Weapon_expl_info[Num_weapon_expl].lod[0].filename, base_filename);

		// stuff LOD level filenames
		for(idx=1; idx<Weapon_expl_info[Num_weapon_expl].lod_count; idx++){
			if(idx >= MAX_weapon_expl_lod){
				break;
			}

			sprintf(Weapon_expl_info[Num_weapon_expl].lod[idx].filename, "%s_%d", base_filename, idx);
		}

		Num_weapon_expl++;
	}
	required_string("#End");

	// close localization
	lcl_ext_close();
}

int get_weapon_expl_info_index(char *filename)
{
	for (int i=0; i<MAX_Weapon_expl_info; i++) {
		if ( stricmp(Weapon_expl_info[i].lod[0].filename, filename) == 0) {
			return i;
		}
	}

	return -1;
}
// ----------------------------------------------------------------------
// missile_obj_list_init()
//
// Clear out the Missile_obj_list
//
void missile_obj_list_init()
{
	int i;

	list_init(&Missile_obj_list);
	for ( i = 0; i < MAX_MISSILE_OBJS; i++ ) {
		Missile_objs[i].flags = 0;
	}
}

// ---------------------------------------------------
// missile_obj_list_add()
//
// Function to add a node from the Missile_obj_list.  Only
// called from weapon_create()
int missile_obj_list_add(int objnum)
{
	int i;

	for ( i = 0; i < MAX_MISSILE_OBJS; i++ ) {
		if ( !(Missile_objs[i].flags & MISSILE_OBJ_USED) )
			break;
	}
	if ( i == MAX_MISSILE_OBJS ) {
		Error(LOCATION, "Fatal Error: Ran out of missile object nodes\n");
		return -1;
	}
	
	Missile_objs[i].flags = 0;
	Missile_objs[i].objnum = objnum;
	list_append(&Missile_obj_list, &Missile_objs[i]);
	Missile_objs[i].flags |= MISSILE_OBJ_USED;

	return i;
}

// ---------------------------------------------------
// missle_obj_list_remove()
//
// Function to remove a node from the Missile_obj_list.  Only
// called from weapon_delete()
void missle_obj_list_remove(int index)
{
	Assert(index >= 0 && index < MAX_MISSILE_OBJS);
	list_remove(&Missile_obj_list, &Missile_objs[index]);	
	Missile_objs[index].flags = 0;
}

// ---------------------------------------------------
// missile_obj_list_rebuild()
//
// Called by the save/restore code to rebuild Missile_obj_list
//
void missile_obj_list_rebuild()
{
	object *objp;

	missile_obj_list_init();

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type == OBJ_WEAPON && Weapon_info[Weapons[objp->instance].weapon_info_index].subtype == WP_MISSILE ) {
			Weapons[objp->instance].missile_list_index = missile_obj_list_add(OBJ_INDEX(objp));
		}
	}
}

// If this is a player countermeasure, let the player know he evaded a missile
void weapon_maybe_alert_cmeasure_success(object *objp)
{
	if ( objp->type == OBJ_CMEASURE ) {
		cmeasure *cmp;
		cmp = &Cmeasures[objp->instance];
		if ( cmp->source_objnum == OBJ_INDEX(Player_obj) ) {
			hud_start_text_flash(XSTR("Evaded", 1430), 800);
			snd_play(&Snds[SND_MISSILE_EVADED_POPUP]);
#ifndef NO_NETWORK
		} else if ( Objects[cmp->source_objnum].flags & OF_PLAYER_SHIP ) {
			send_countermeasure_success_packet( cmp->source_objnum );
#endif
		}
	}
}

// ---------------------------------------------------
// missile_obj_return_address()
//
// Called externally to generate an address from an index into
// the Missile_objs[] array
//
missile_obj *missile_obj_return_address(int index)
{
	Assert(index >= 0 && index < MAX_MISSILE_OBJS);
	return &Missile_objs[index];
}

//	Return the index of Weapon_info[].name that is *name.
int weapon_info_lookup(char *name)
{
	int	i;

	// bogus
	if (!name)
		return -1;

	for (i=0; i<Num_weapon_types; i++)
		if (!stricmp(name, Weapon_info[i].name))
			return i;

	return -1;
}

#define DEFAULT_WEAPON_SPAWN_COUNT	10

//	Parse the weapon flags.
void parse_wi_flags(weapon_info *weaponp)
{
	char	weapon_strings[MAX_WEAPON_FLAGS][NAME_LENGTH];
	int	num_strings;

	required_string("$Flags:");

	num_strings = stuff_string_list(weapon_strings, MAX_WEAPON_FLAGS);
	
	for (int i=0; i<num_strings; i++) {
		if (!stricmp(NOX("Electronics"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_ELECTRONICS;		
		else if (!strnicmp(NOX("Spawn"), weapon_strings[i], 5)) {
			if (weaponp->spawn_type == -1) {
				int	skip_length, name_length;
				char	*temp_string;

				temp_string = weapon_strings[i];

				weaponp->wi_flags |= WIF_SPAWN;
				weaponp->spawn_type = (short)Num_spawn_types;
				skip_length = strlen(NOX("Spawn")) + strspn(&temp_string[strlen(NOX("Spawn"))], NOX(" \t"));
				char *num_start = strchr(&temp_string[skip_length], ',');
				if (num_start == NULL) {
					weaponp->spawn_count = DEFAULT_WEAPON_SPAWN_COUNT;
					name_length = 999;
				} else {
					weaponp->spawn_count = (short)atoi(num_start+1);
					name_length = num_start - temp_string - skip_length;
				}

				strncpy(Spawn_names[Num_spawn_types++], &(weapon_strings[i][skip_length]), name_length);
				Assert(Num_spawn_types < MAX_SPAWN_WEAPONS);
			} else {
				Warning(LOCATION, "Illegal to have two spawn types for one weapon.\nIgnoring weapon %s", weaponp->name);
			}
		} else if (!stricmp(NOX("Remote Detonate"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_REMOTE;
		else if (!stricmp(NOX("Puncture"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_PUNCTURE;		
		else if (!stricmp(NOX("Big Ship"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_BIG_ONLY;
		else if (!stricmp(NOX("Huge"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_HUGE;
		else if (!stricmp(NOX("Bomber+"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_BOMBER_PLUS;
		else if (!stricmp(NOX("child"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_CHILD;
		else if (!stricmp(NOX("Bomb"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_BOMB;
		else if (!stricmp(NOX("No Dumbfire"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_NO_DUMBFIRE;
		else if (!stricmp(NOX("In tech database"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_IN_TECH_DATABASE;
		else if (!stricmp(NOX("Player allowed"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_PLAYER_ALLOWED;		
		else if (!stricmp(NOX("Particle Spew"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_PARTICLE_SPEW;
		else if (!stricmp(NOX("EMP"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_EMP;
		else if (!stricmp(NOX("Esuck"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_ENERGY_SUCK;
		else if (!stricmp(NOX("Flak"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_FLAK;
		else if (!stricmp(NOX("Corkscrew"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_CORKSCREW;
		else if (!stricmp(NOX("Shudder"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_SHUDDER;		
		else if (!stricmp(NOX("lockarm"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_LOCKARM;		
		else if (!stricmp(NOX("beam"), weapon_strings[i]))
		{
			weaponp->wi_flags |= WIF_BEAM;

			// IMPORTANT: beams pierce shields by default :rolleyes: :p - Goober5000
			weaponp->wi_flags2 |= WIF2_PIERCE_SHIELDS;
		}
		else if (!stricmp(NOX("stream"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_STREAM;
		else if (!stricmp(NOX("supercap"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_SUPERCAP;
		else if (!stricmp(NOX("ballistic"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_BALLISTIC;
		else if (!stricmp(NOX("pierce shields"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_PIERCE_SHIELDS;
		else if (!stricmp(NOX("no pierce shields"), weapon_strings[i]))	// only for beams
			weaponp->wi_flags2 &= ~WIF2_PIERCE_SHIELDS;
		else if (!stricmp(NOX("local ssm"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_LOCAL_SSM;
		else if (!stricmp(NOX("tagged only"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_TAGGED_ONLY;
		else if (!stricmp(NOX("beam no whack"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_BEAM_NO_WHACK;
		else if (!stricmp(NOX("cycle"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_CYCLE;
		else if (!stricmp(NOX("small only"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SMALL_ONLY;
		else if (!stricmp(NOX("same turret cooldown"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SAME_TURRET_COOLDOWN;
		else
			Warning(LOCATION, "Bogus string in weapon flags: %s\n", weapon_strings[i]);

		// there might be a command line option to disable beam shield piercing
		if (Cmdline_beams_no_pierce_shields)
		{
			weaponp->wi_flags2 &= ~WIF2_PIERCE_SHIELDS;
		}
	}	

	// set default tech room status - Goober5000
	if (weaponp->wi_flags & WIF_IN_TECH_DATABASE)
		weaponp->wi_flags2 |= WIF2_DEFAULT_IN_TECH_DATABASE;

	// SWARM, CORKSCREW and FLAK should be mutually exclusive
	if (weaponp->wi_flags & WIF_FLAK)
	{
		if ((weaponp->wi_flags & WIF_SWARM) || (weaponp->wi_flags & WIF_CORKSCREW))
		{
			Warning(LOCATION, "Swarm, Corkscrew, and Flak are mutually exclusive!  Removing Swarm and Corkscrew attributes.\n");
			weaponp->wi_flags &= ~WIF_SWARM;
			weaponp->wi_flags &= ~WIF_CORKSCREW;
		}
	}
	else
	{
		if ((weaponp->wi_flags & WIF_SWARM) && (weaponp->wi_flags & WIF_CORKSCREW))
		{
			Warning(LOCATION, "Swarm and Corkscrew are mutually exclusive!  Defaulting to Swarm.\n");
			weaponp->wi_flags &= ~WIF_CORKSCREW;
		}
	}

	if (weaponp->wi_flags2 & WIF2_LOCAL_SSM)
	{
		if (!(weaponp->wi_flags & WIF_HOMING) || (weaponp->subtype !=WP_MISSILE))
		{
			Warning(LOCATION, "local ssm must be guided missile: %s", weaponp->name);
		}
	}

	if ((weaponp->wi_flags2 & WIF2_SMALL_ONLY) && (weaponp->wi_flags & WIF_HUGE))
	{
		Warning(LOCATION,"\"small only\" and \"huge\" flags are mutually exclusive.\nThey are used together in %s\nAI will most likely not use this weapon",weaponp->name);
	}

}

// function to parse the information for a specific weapon type.	
// return 0 if successful, otherwise return -1
#define WEAPONS_MULTITEXT_LENGTH 2048

int parse_weapon(int subtype, bool replace)
{
	char buf[WEAPONS_MULTITEXT_LENGTH];
	weapon_info *wip;
	char fname[255] = "";
	int idx;
	int primary_rearm_rate_specified;

	wip = &Weapon_info[Num_weapon_types];

	wip->wi_flags = WIF_DEFAULT_VALUE;
	wip->wi_flags2 = WIF2_DEFAULT_VALUE;

	required_string("$Name:");
	stuff_string(wip->name, F_NAME, NULL);
	diag_printf ("Weapon name -- %s\n", wip->name);

	strcpy(parse_error_text, "");
	strcpy(parse_error_text, "\nin weapon: ");
	strcat(parse_error_text, wip->name);
	// AL 28-3-98: If this is a demo build, we only want to parse weapons that are preceded with
	//             the '@' symbol
	// WMC 27-4-05: No need now. :)
/*	#ifdef DEMO // not needed FS2_DEMO (separate table file)
		if ( wip->name[0] != '@' ) {
			// advance to next weapon, and return -1

			if ( skip_to_start_of_strings("$Name:", "#End") != 1 ) {
				Int3();
			}
			return -1;
		}
	#endif
*/

	if ( wip->name[0] == '@' ) {
		char old_name[NAME_LENGTH];
		strcpy(old_name, wip->name);
		strcpy(wip->name, old_name+1);
	}

	int w_id = weapon_name_lookup(wip->name);
	if(w_id != -1)
	{
		if(replace)
		{
			wip = &Weapon_info[w_id];
		}
		else
		{
			nprintf(("Warning", "Error: Weapon %s already exists. Weapon names should be unique.", wip->name));
		}
	}
	else
	{
		Num_weapon_types++;
	}
	//Set subtype
	wip->subtype = subtype;

	wip->title[0] = 0;
	if (optional_string("+Title:")) {
		stuff_string(wip->title, F_NAME, NULL, WEAPON_TITLE_LEN);
	}

	wip->desc = NULL;
	if (optional_string("+Description:")) {
		stuff_string(buf, F_MULTITEXT, NULL);
		wip->desc = strdup(buf);
	}

	wip->tech_title[0] = 0;
	if (optional_string("+Tech Title:")) {
		stuff_string(wip->tech_title, F_NAME, NULL, NAME_LENGTH);
	}

	wip->tech_anim_filename[0] = 0;
	if (optional_string("+Tech Anim:")) {
		stuff_string(wip->tech_anim_filename, F_NAME, NULL, NAME_LENGTH);
	}

	wip->tech_desc = NULL;
	if (optional_string("+Tech Description:")) {
		stuff_string(buf, F_MULTITEXT, NULL, WEAPONS_MULTITEXT_LENGTH);
		wip->tech_desc = strdup(buf);
	}

	if(optional_string("$Tech Model:"))
		stuff_string(wip->tech_model, F_NAME, NULL);
	else
		wip->tech_model[0] = '\0';

	//Check for the HUD image string
	if(optional_string("$HUD Image:"))
	{
		stuff_string(wip->hud_filename, F_NAME, NULL);
	}
	else
	{
		wip->hud_filename[0] = '\0';
	}
	wip->hud_image_index = -1;

	//	Read the model file.  It can be a POF file or none.
	//	If there is no model file (Model file: = "none") then we use our special
	//	laser renderer which requires inner, middle and outer information.
	required_string("$Model file:");
	stuff_string(wip->pofbitmap_name, F_NAME, NULL);
	diag_printf ("Model pof file -- %s\n", wip->pofbitmap_name );

		if (optional_string("$External Model File:")) {
			stuff_string(wip->external_model_name, F_NAME, NULL);	
		}else{
			strcpy(wip->external_model_name,"");
		}
		wip->external_model_num = -1;

	wip->weapon_submodel_rotate_accell = 10.0f;
	wip->weapon_submodel_rotate_vel = 0.0f;

	if (optional_string("$Submodel Rotation Speed:"))
		stuff_float(&wip->weapon_submodel_rotate_vel);
	if (optional_string("$Submodel Rotation Acceleration:"))
		stuff_float(&wip->weapon_submodel_rotate_accell);

	if ( stricmp(wip->pofbitmap_name, NOX("none")) ) {
		wip->model_num = -1;				
		wip->render_type = WRT_POF;
		wip->laser_bitmap = -1;
	} else {
		//	No POF or AVI file specified, render as special laser type.
		ubyte r,g,b;

		wip->render_type = WRT_LASER;
		wip->model_num = -1;

		// laser bitmap itself
		required_string("@Laser Bitmap:");
		stuff_string(wip->pofbitmap_name, F_NAME, NULL);
		wip->laser_bitmap = -1;
		if(!Fred_running){
			wip->laser_bitmap = bm_load( wip->pofbitmap_name );
			if(wip->laser_bitmap < 0){	//if it couldn't find the pcx look for an ani-Bobboau
				nprintf(("General","couldn't find pcx for %s \n", wip->name));
					wip->laser_bitmap = bm_load_animation(wip->pofbitmap_name, &wip->laser_bitmap_nframes, &wip->laser_bitmap_fps, 1);				
				if(wip->laser_bitmap < 0){
					nprintf(("General","couldn't find ani for %s \n", wip->name));
				Warning( LOCATION, "Couldn't open texture '%s'\nreferenced by weapon '%s'\n", wip->pofbitmap_name, wip->name );
				}else{
					nprintf(("General","found ani %s for %s, with %d frames and %d fps \n", wip->pofbitmap_name, wip->name,wip->laser_bitmap_nframes, wip->laser_bitmap_fps));
				}
			}else{
				wip->laser_bitmap_nframes = 1;
				wip->laser_bitmap_fps = 0;
			}

		}

		// optional laser glow
		wip->laser_glow_bitmap = -1;
		if(optional_string("@Laser Glow:")){
			stuff_string(fname, F_NAME, NULL);		
			if(!Fred_running){
				wip->laser_glow_bitmap = bm_load( fname );
				if(wip->laser_glow_bitmap < 0){	//if it couldn't find the pcx look for an ani-Bobboau
					nprintf(("General","couldn't find pcx for %s \n", wip->name));
					wip->laser_glow_bitmap = bm_load_animation(fname, &wip->laser_glow_bitmap_nframes, &wip->laser_glow_bitmap_fps, 1);				
					if(wip->laser_glow_bitmap < 0){
						nprintf(("General","couldn't find ani for %s \n", wip->name));
					Warning( LOCATION, "Couldn't open glow texture '%s'\nreferenced by weapon '%s'\n", fname, wip->name );
					}else{
						nprintf(("General","found ani %s for %s, with %d frames and %d fps \n", fname, wip->name,wip->laser_glow_bitmap_nframes, wip->laser_glow_bitmap_fps));
					}
				}else{
					wip->laser_glow_bitmap_nframes = 0;
					wip->laser_glow_bitmap_fps = 0;
				}

				/* there's no purpose to this, it just gets unloaded before use anyway - taylor
				// might as well lock it down as an aabitmap now
				if(wip->laser_glow_bitmap >= 0){	//locking all frames if it is a ani-Bobboau
					for(int i = 0; i>wip->laser_glow_bitmap_nframes; i++){
						bm_lock((wip->laser_glow_bitmap + i), 8, BMP_AABITMAP);
						bm_unlock((wip->laser_glow_bitmap + i));
						nprintf(("General","locking glow for weapon %s \n", wip->name));
					//	mprintf(("locking glow for weapon"));
					}
				}
				*/
			}
		}
		
		required_string("@Laser Color:");
		stuff_ubyte(&r);	stuff_ubyte(&g);	stuff_ubyte(&b);
		gr_init_color( &wip->laser_color_1, r, g, b );

		// optional string for cycling laser colors
		gr_init_color(&wip->laser_color_2, 0, 0, 0);
		if(optional_string("@Laser Color2:")){
			stuff_ubyte(&r);	stuff_ubyte(&g);	stuff_ubyte(&b);
			gr_init_color( &wip->laser_color_2, r, g, b );
		}

		required_string("@Laser Length:");
		stuff_float(&wip->laser_length);
		
		required_string("@Laser Head Radius:");
		stuff_float(&wip->laser_head_radius);

		required_string("@Laser Tail Radius:");
		stuff_float(&wip->laser_tail_radius );
	}

	required_string("$Mass:");
	stuff_float( &(wip->mass) );
	diag_printf ("Weapon mass -- %7.3f\n", wip->mass);

	required_string("$Velocity:");
	stuff_float( &(wip->max_speed) );
	diag_printf ("Weapon mass -- %7.3f\n", wip->max_speed);

	required_string("$Fire Wait:");
	stuff_float( &(wip->fire_wait) );
	diag_printf ("Weapon fire wait -- %7.3f\n", wip->fire_wait);

	required_string("$Damage:");
	stuff_float(&wip->damage);
	
	wip->armor_damage_index = -1;
	if(optional_string("$Armor Damage Index:"))
		stuff_int(&wip->armor_damage_index);
	//This is checked for validity on every armor type
	//If it's invalid (or -1), then armor has no effect

	// secondary weapons require these values
	wip->blast_force = 0;
	wip->inner_radius = 0;
	wip->outer_radius = 0;
	wip->shockwave_speed = 0;
	if (subtype == WP_MISSILE) {
		required_string("$Blast Force:");
		stuff_float( &(wip->blast_force) );
		diag_printf ("Weapon blast force -- %7.3f\n", wip->blast_force);

		required_string("$Inner Radius:");
		stuff_float( &(wip->inner_radius) );
		if ( wip->inner_radius != 0 ) {
			wip->wi_flags |= WIF_AREA_EFFECT;
		}
		diag_printf ("Weapon inner blast radius -- %7.3f\n", wip->inner_radius);

		required_string("$Outer Radius:");
		stuff_float( &(wip->outer_radius) );
		if ( wip->outer_radius != 0 ) {
			wip->wi_flags |= WIF_AREA_EFFECT;
		}
		diag_printf ("Weapon outer blast radius -- %7.3f\n", wip->outer_radius);

		required_string("$Shockwave Speed:");
		stuff_float( &(wip->shockwave_speed) );
		if ( wip->shockwave_speed != 0 ) {
			wip->wi_flags |= WIF_SHOCKWAVE;
		}
		diag_printf ("Shockwave speed -- %7.3f\n", wip->shockwave_speed);

		wip->shockwave_model = -1;
		strcpy(wip->shockwave_pof_name,"");
		if(optional_string("$Shockwave_model:")){
			stuff_string( wip->shockwave_pof_name, F_NAME, NULL);
		}

	} 
	// for primary weapons they're optional
	else {
		if(optional_string("$Blast Force:")){
			stuff_float( &(wip->blast_force) );
			diag_printf ("Weapon blast force -- %7.3f\n", wip->blast_force);
		}

		if(optional_string("$Inner Radius:")){
			stuff_float( &(wip->inner_radius) );
			if ( wip->inner_radius != 0 ) {
				wip->wi_flags |= WIF_AREA_EFFECT;
			}
			diag_printf ("Weapon inner blast radius -- %7.3f\n", wip->inner_radius);
		}

		if(optional_string("$Outer Radius:")){
			stuff_float( &(wip->outer_radius) );
			if ( wip->outer_radius != 0 ) {
				wip->wi_flags |= WIF_AREA_EFFECT;
			}
			diag_printf ("Weapon outer blast radius -- %7.3f\n", wip->outer_radius);
		}

		if(optional_string("$Shockwave Speed:")){
			stuff_float( &(wip->shockwave_speed) );
			if ( wip->shockwave_speed != 0 ) {
				wip->wi_flags |= WIF_SHOCKWAVE;
			}
			diag_printf ("Shockwave speed -- %7.3f\n", wip->shockwave_speed);
		}

		wip->shockwave_model = -1;
		strcpy(wip->shockwave_pof_name,"");
		if(optional_string("$Shockwave_model:")){
			stuff_string( wip->shockwave_pof_name, F_NAME, NULL);
		}
	}

	required_string("$Armor Factor:");
	stuff_float(&wip->armor_factor);

	required_string("$Shield Factor:");
	stuff_float(&wip->shield_factor);

	required_string("$Subsystem Factor:");
	stuff_float(&wip->subsystem_factor);

	required_string("$Lifetime:");
	stuff_float(&wip->lifetime);

	required_string("$Energy Consumed:");
	stuff_float(&wip->energy_consumed);

	// Goober5000: cargo size is checked for div-0 errors... see below (must parse flags first)
	required_string("$Cargo Size:");
	stuff_float(&wip->cargo_size);

	int is_homing=0;
	required_string("$Homing:");
	stuff_boolean(&is_homing);

	if (is_homing == 1) {
		char	temp_type[128];

		// the following five items only need to be recorded if the weapon is a homing weapon
		required_string("+Type:");
		stuff_string(temp_type, F_NAME, NULL);

		if (!stricmp(temp_type, NOX("HEAT"))) {
			float	view_cone_angle;

			wip->wi_flags |= WIF_HOMING_HEAT | WIF_TURNS;

			required_string("+Turn Time:");			
			stuff_float(&wip->turn_time);

			required_string("+View Cone:");
			stuff_float(&view_cone_angle);

			wip->fov = (float)cos((float)(ANG_TO_RAD(view_cone_angle/2.0f)));

		} else if (!stricmp(temp_type, NOX("ASPECT"))) {
			wip->wi_flags |= WIF_HOMING_ASPECT | WIF_TURNS;

			required_string("+Turn Time:");			
			stuff_float(&wip->turn_time);

			required_string("+Min Lock Time:");			// minimum time (in seconds) to achieve lock
			stuff_float(&wip->min_lock_time);

			required_string("+Lock Pixels/Sec:");		// pixels/sec moved while locking
			stuff_int(&wip->lock_pixels_per_sec);

			required_string("+Catch-up Pixels/Sec:");	// pixels/sec moved while catching-up for a lock
			stuff_int(&wip->catchup_pixels_per_sec);

			required_string("+Catch-up Penalty:");		// number of extra pixels to move while locking as a penalty for catching up for a lock
			stuff_int(&wip->catchup_pixel_penalty);
		} else
			Error(LOCATION, "Illegal homing type = %s.\nMust be HEAT or ASPECT.\n", temp_type);

	}

	// swarm missiles
	int s_count;

	wip->swarm_count = -1;

    // *Default is 150  -Et1
    wip->SwarmWait = SWARM_MISSILE_DELAY;

	if(optional_string("$Swarm:")){
		wip->swarm_count = SWARM_DEFAULT_NUM_MISSILES_FIRED;
		stuff_int(&s_count);
		wip->swarm_count = (short)s_count;

		// flag as being a swarm weapon
		wip->wi_flags |= WIF_SWARM;


        // *Swarm wait token    -Et1

        if( optional_string( "+SwarmWait:" ) )
        {

            float SwarmWait;

            stuff_float( &SwarmWait );

            if( SwarmWait > 0.0f && SwarmWait * wip->swarm_count < wip->fire_wait )
            {

                wip->SwarmWait = int( SwarmWait * 1000 );

            }


        }

	}

	//Launch sound
	parse_sound("$LaunchSnd:", &wip->launch_snd, wip->name);

	//Impact sound
	parse_sound("$ImpactSnd:", &wip->impact_snd, wip->name);

	if (subtype == WP_MISSILE)
	{
		parse_sound("$FlyBySnd:", &wip->flyby_snd, wip->name);
	}
	else
	{
		if (optional_string("$FlyBySnd:"))
		{
			Warning(LOCATION, "$FlyBySnd: flag not used with primary weapons; ignoring...");
		}
	}

	// handle rearm rate - modified by Goober5000
	primary_rearm_rate_specified = 0;
	if (subtype == WP_MISSILE)		// secondary weapons are required to have a rearm rate
	{
		required_string( "$Rearm Rate:");

		stuff_float( &wip->rearm_rate );
		if (wip->rearm_rate > 0.1f)
		{
			wip->rearm_rate = 1.0f/wip->rearm_rate;
		}
		else
		{
			wip->rearm_rate = 1.0f;
		}
	}
	else
	{
		// Anticipate rearm rate for ballistic primaries
		if (optional_string("$Rearm Rate:"))
		{
			primary_rearm_rate_specified = 1;

			stuff_float( &wip->rearm_rate );
			if (wip->rearm_rate > 0.1f)
			{
				wip->rearm_rate = 1.0f/wip->rearm_rate;
			}
			else
			{
				wip->rearm_rate = 1.0f;
			}
		}
	}

	wip->weapon_range = 999999999.9f;
	if (optional_string("+Weapon Range:")) {
		stuff_float(&wip->weapon_range);
	}

    // *Mínimum weapon range, default is 0 -Et1
    wip->WeaponMinRange = 0.0f;

    if( optional_string( "+Weapon Min Range:" ) )
    {

        float MinRange;

        stuff_float( &MinRange );

        if( MinRange > 0.0f && MinRange < MIN( wip->max_speed * wip->lifetime, wip->weapon_range ) )
        {

            wip->WeaponMinRange = MinRange;

        }

    }

	wip->spawn_type = -1;
	parse_wi_flags(wip);

	// be friendly; make sure ballistic flags are synchronized - Goober5000
	// primary
	if (subtype == WP_LASER)
	{
		// ballistic
		if (wip->wi_flags2 & WIF2_BALLISTIC)
		{
			// rearm rate not specified
			if (!primary_rearm_rate_specified)
			{
				Warning(LOCATION, "$Rearm Rate for ballistic primary %s not specified.  Defaulting to 100...\n", wip->name);
				wip->rearm_rate = 100.0f;
			}
		}
		// not ballistic
		else
		{
			// rearm rate specified
			if (primary_rearm_rate_specified)
			{
				Warning(LOCATION, "$Rearm Rate specified for non-ballistic primary %s\n", wip->name);
			}
		}

	}
	// secondary
	else
	{
		// ballistic
		if (wip->wi_flags2 & WIF2_BALLISTIC)
		{
			Warning(LOCATION, "Secondary weapon %s can't be ballistic.  Removing this flag...\n", wip->name);
			wip->wi_flags2 &= ~WIF2_BALLISTIC;
		}
	}

	// also make sure EMP is friendly - Goober5000
	if (wip->wi_flags & WIF_EMP)
	{
		if (!wip->outer_radius)
		{
			Warning(LOCATION, "Outer blast radius of weapon %s is zero - EMP will not work.\nAdd $Outer Radius to weapon table entry.\n", wip->name);
		}
	}

	// also make sure secondaries and ballistic primaries do not have 0 cargo size
	if (subtype == WP_MISSILE || wip->wi_flags2 & WIF2_BALLISTIC)
	{
		if (wip->cargo_size == 0.0f)
		{
			Warning(LOCATION, "Cargo size of weapon %s cannot be 0.  Setting to 1.\n", wip->name);
			wip->cargo_size = 1;
		}
	}

	char trail_name[MAX_FILENAME_LEN] = "";
	trail_info *ti = &wip->tr_info;
	memset(ti, 0, sizeof(trail_info));
	if(optional_string("$Trail:")){	
		wip->wi_flags |= WIF_TRAIL;		// missile leaves a trail

		required_string("+Start Width:");
		stuff_float(&ti->w_start);

		required_string("+End Width:");
		stuff_float(&ti->w_end);

		required_string("+Start Alpha:");
		stuff_float(&ti->a_start);

		required_string("+End Alpha:");
		stuff_float(&ti->a_end);		

		required_string("+Max Life:");
		stuff_float(&ti->max_life);

		ti->stamp = fl2i(1000.0f*ti->max_life)/(NUM_TRAIL_SECTIONS+1);

		required_string("+Bitmap:");
		stuff_string(trail_name, F_NAME, NULL);
		ti->bitmap = bm_load(trail_name);
		// wip->delta_time = fl2i(1000.0f*wip->max_life)/(NUM_TRAIL_SECTIONS+1);		// time between sections.  max_life / num_sections basically.
	}

	// read in filename for icon that is used in weapons selection
	wip->icon_filename[0] = 0;
	if ( optional_string("$Icon:") ) {
		stuff_string(wip->icon_filename, F_NAME, NULL);
	}

	// read in filename for animation that is used in weapons selection
	wip->anim_filename[0] = 0;
	if ( optional_string("$Anim:") ) {
		stuff_string(wip->anim_filename, F_NAME, NULL);
	}

	wip->impact_weapon_expl_index = -1;
	if ( optional_string("$Impact Explosion:") ) {
		char impact_ani_file[FILESPEC_LENGTH];
		stuff_string(impact_ani_file, F_NAME, NULL);
		if ( stricmp(impact_ani_file,NOX("none")))	{
			wip->impact_weapon_expl_index = get_weapon_expl_info_index(impact_ani_file);
			//int num_frames, fps;
			//wip->impact_explosion_ani = bm_load_animation( impact_ani_file, &num_frames, &fps, 1 );

			required_string("$Impact Explosion Radius:");
			stuff_float(&wip->impact_explosion_radius);
		}
	}

	// muzzle flash
	char mflash_string[255] = "";
	wip->muzzle_flash = -1;
	if( optional_string("$Muzzleflash:") ){
		stuff_string(mflash_string, F_NAME, NULL);

		// look it up
		wip->muzzle_flash = mflash_lookup(mflash_string);

		if(wip->muzzle_flash >= 0){			
			wip->wi_flags |= WIF_MFLASH;
		}
	}

	// EMP optional stuff (if WIF_EMP is not set, none of this matters, anyway)
	if( optional_string("$EMP Intensity:") ){
		stuff_float(&wip->emp_intensity);
	} else {
		wip->emp_intensity = EMP_DEFAULT_INTENSITY;
	}
	if( optional_string("$EMP Time:") ){
		stuff_float(&wip->emp_time);
	} else {
		wip->emp_time = EMP_DEFAULT_TIME;	// Goober5000: <-- Look!  I fixed a Volition bug!  Gimme $5, Dave!
	}

	// Energy suck optional stuff (if WIF_ENERGY_SUCK is not set, none of this matters anyway)
	if( optional_string("$Leech Weapon:") ){
		stuff_float(&wip->weapon_reduce);
	} else {
		wip->weapon_reduce = ESUCK_DEFAULT_WEAPON_REDUCE;
	}

	if( optional_string("$Leech Afterburner:") ){
		stuff_float(&wip->afterburner_reduce);
	} else {
		wip->afterburner_reduce = ESUCK_DEFAULT_AFTERBURNER_REDUCE;
	}

/*
	int Corkscrew_missile_delay			= 30;			// delay between missile firings
	int Corkscrew_num_missiles_fired		= 4;			// # of missiles fire in one shot
	float Corkscrew_radius					= 1.25f;		// radius of the corkscrew itself
	float Corkscrew_twist					= 5.0f;		// in degrees/second
	int Corkscrew_helix						= 1;			// attempt to point the missile in the right direction
	int Corkscrew_counterrotate			= 1;			// counterrotate every other missile
	int Corkscrew_shrink						= 0;			// shrink the radius of every successive missile
	float Corkscrew_shrink_val				= 0.3f;		// rate at which the radius shrinks
	int Corkscrew_down_first				= 1;			// have the corkscrew go "down" first
*/
	//customizeable corkscrew stuff
	wip->cs_num_fired=4;
	wip->cs_radius=1.25f;
	wip->cs_delay=30;
	wip->cs_crotate=1;
	wip->cs_twist=5.0f;

	if (optional_string("$Corkscrew:"))
	{
		required_string("+Num Fired:");
		stuff_int(&wip->cs_num_fired);

		required_string("+Radius:");
		stuff_float(&wip->cs_radius);

		required_string("+Fire Delay:");
		stuff_int(&wip->cs_delay);
		
		required_string("+Counter rotate:");
		stuff_boolean(&wip->cs_crotate);

		required_string("+Twist:");
		stuff_float(&wip->cs_twist);

	}

	//electronics tag optional stuff
	wip->elec_intensity=1.0f;
	wip->elec_time=6000;
	wip->elec_eng_mult=1.0f;
	wip->elec_weap_mult=1.0f;
	wip->elec_beam_mult=1.0f;
	wip->elec_sensors_mult=1.0f;
	wip->elec_randomness=4000;
	wip->elec_use_new_style=0;

	if (optional_string("$Electronics:"))
	{
		if (!required_string_either("+New Style:", "+Old Style:"))
		{
			required_string("+New Style:");
			wip->elec_use_new_style=1;

			required_string("+Intensity:");
			stuff_float(&wip->elec_intensity);

			required_string("+Lifetime:");
			stuff_int(&wip->elec_time);

			required_string("+Engine Multiplier:");
			stuff_float(&wip->elec_eng_mult);

			required_string("+Weapon Multiplier:");
			stuff_float(&wip->elec_weap_mult);

			required_string("+Beam Turret Multiplier:");
			stuff_float(&wip->elec_beam_mult);

			required_string("+Sensors Multiplier:");
			stuff_float(&wip->elec_sensors_mult);
	
			required_string("+Randomness Time:");
			stuff_int(&wip->elec_randomness);
		}
		else
		{
			required_string("+Old Style:");
			wip->elec_use_new_style=0;
			
			required_string("+Lifetime:");
			stuff_int(&wip->elec_time);

			required_string("+Randomness Time:");
			stuff_int(&wip->elec_randomness);
		}
	}


	//read in the spawn angle info
	//if the weapon isn't a spawn weapon, then this is not going to be used.
	wip->spawn_angle = 360;
	if (optional_string("$Spawn Angle:"))
	{
		stuff_float(&wip->spawn_angle);
	}

	wip->lssm_warpout_delay=0;			//delay between launch and warpout (ms)
	wip->lssm_warpin_delay=0;			//delay between warpout and warpin (ms)
	wip->lssm_stage5_vel=0;		//velocity during final stage
	wip->lssm_warpin_radius=0;
	wip->lssm_lock_range=1000000.0f;	//local ssm lock range (optional)
	if (wip->wi_flags2 & WIF2_LOCAL_SSM)
	{
		required_string("$Local SSM:");

		required_string("+Warpout Delay:");
		stuff_int(&wip->lssm_warpout_delay);

		required_string("+Warpin Delay:");
		stuff_int(&wip->lssm_warpin_delay);

		required_string("+Stage 5 Velocity:");
		stuff_float(&wip->lssm_stage5_vel);

		required_string("+Warpin Radius:");
		stuff_float(&wip->lssm_warpin_radius);

		if (optional_string("+Lock Range:"))
			stuff_float(&wip->lssm_lock_range);
	}


	// beam weapon optional stuff
	wip->b_info.beam_type = -1;
	wip->b_info.beam_life = -1.0f;
	wip->b_info.beam_warmup = -1;
	wip->b_info.beam_warmdown = -1;
	wip->b_info.beam_muzzle_radius = 0.0f;
	wip->b_info.beam_particle_count = -1;
	wip->b_info.beam_particle_radius = 0.0f;
	wip->b_info.beam_particle_angle = 0.0f;
	wip->b_info.beam_particle_ani = -1;	
	wip->b_info.beam_loop_sound = -1;
	wip->b_info.beam_warmup_sound = -1;
	wip->b_info.beam_warmdown_sound = -1;
	wip->b_info.beam_num_sections = 0;
	wip->b_info.beam_glow_bitmap = -1;
	wip->b_info.beam_glow_nframes = 1;
	wip->b_info.beam_glow_fps = 0;
	wip->b_info.beam_shots = 0;
	wip->b_info.beam_shrink_factor = 0.0f;
	wip->b_info.beam_shrink_pct = 0.0f;
	wip->b_info.range = BEAM_FAR_LENGTH;
	wip->b_info.damage_threshold = 1.0f;

	if( optional_string("$BeamInfo:")){
		// beam type
		required_string("+Type:");
		stuff_int(&wip->b_info.beam_type);

		// how long it lasts
		required_string("+Life:");
		stuff_float(&wip->b_info.beam_life);

		// warmup time
		required_string("+Warmup:");
		stuff_int(&wip->b_info.beam_warmup);

		// warmdowm time
		required_string("+Warmdown:");
		stuff_int(&wip->b_info.beam_warmdown);

		// muzzle glow radius
		required_string("+Radius:");
		stuff_float(&wip->b_info.beam_muzzle_radius);

		// particle spew count
		required_string("+PCount:");
		stuff_int(&wip->b_info.beam_particle_count);

		// particle radius
		required_string("+PRadius:");
		stuff_float(&wip->b_info.beam_particle_radius);

		// angle off turret normal
		required_string("+PAngle:");
		stuff_float(&wip->b_info.beam_particle_angle);

		// particle bitmap/ani		
		required_string("+PAni:");
		stuff_string(fname, F_NAME, NULL);
		if(!Fred_running){
			int num_frames, fps;
			wip->b_info.beam_particle_ani = bm_load_animation(fname, &num_frames, &fps, 1);
		}

		// magic miss #
		required_string("+Miss Factor:");		
		for(idx=0; idx<NUM_SKILL_LEVELS; idx++){
			wip->b_info.beam_miss_factor[idx] = 0.00001f;
			stuff_float(&wip->b_info.beam_miss_factor[idx]);
		}

		// beam fire sound
		required_string("+BeamSound:");
		stuff_int(&wip->b_info.beam_loop_sound);

		// warmup sound
		required_string("+WarmupSound:");
		stuff_int(&wip->b_info.beam_warmup_sound);

		// warmdown sound
		required_string("+WarmdownSound:");
		stuff_int(&wip->b_info.beam_warmdown_sound);

		// glow bitmap
		required_string("+Muzzleglow:");
		stuff_string(fname, F_NAME, NULL);
		if(!Fred_running){
			wip->b_info.beam_glow_bitmap = bm_load(fname);

			if (wip->b_info.beam_glow_bitmap == -1) {
				wip->b_info.beam_glow_bitmap = bm_load_animation(fname, &wip->b_info.beam_glow_nframes, &wip->b_info.beam_glow_fps);
			}
		}

		// # of shots (only used for type D beams)
		required_string("+Shots:");
		stuff_int(&wip->b_info.beam_shots);

		// shrinkage
		required_string("+ShrinkFactor:");
		stuff_float(&wip->b_info.beam_shrink_factor);
		required_string("+ShrinkPct:");
		stuff_float(&wip->b_info.beam_shrink_pct);

		if (optional_string("+Range:"))
		{
			stuff_float(&wip->b_info.range);
		}
		if (optional_string("+Attenuation:"))
		{
			stuff_float(&wip->b_info.damage_threshold);
		}
		// beam sections
		while( optional_string("$Section:") ){
			beam_weapon_section_info i;
			char tex_name[255] = "";
			
			// section width
			required_string("+Width:");
			stuff_float(&i.width);

			// texture
			required_string("+Texture:");
			stuff_string(tex_name, F_NAME, NULL);
			i.texture = -1;
			i.nframes = 1;
			i.fps = 1;

			if(!Fred_running){
				i.texture = bm_load(tex_name);

				if (i.texture < 0) {
					i.texture = bm_load_animation(tex_name, &i.nframes, &i.fps);
				}

				/* there's no purpose to this, it just gets unloaded before use anyway - taylor
				if(i.texture >= 0){
	
					bm_lock(i.texture, 16, BMP_TEX_OTHER);
	
					bm_unlock(i.texture);
	
				} */
			}

			// rgba inner
			required_string("+RGBA Inner:");
			stuff_ubyte(&i.rgba_inner[0]);
			stuff_ubyte(&i.rgba_inner[1]);
			stuff_ubyte(&i.rgba_inner[2]);
			stuff_ubyte(&i.rgba_inner[3]);

			// rgba outer
			required_string("+RGBA Outer:");
			stuff_ubyte(&i.rgba_outer[0]);
			stuff_ubyte(&i.rgba_outer[1]);
			stuff_ubyte(&i.rgba_outer[2]);
			stuff_ubyte(&i.rgba_outer[3]);

			// flicker
			required_string("+Flicker:");
			stuff_float(&i.flicker); 

			// zadd
			required_string("+Zadd:");
			stuff_float(&i.z_add);

			i.tile_type = 0;
			i.tile_factor = 1.0f;
			if( optional_string("+Tile Factor:")){ //beam texture tileing factor -Bobboau
				stuff_float(&(i.tile_factor));
				stuff_int(&(i.tile_type));
			}

			i.translation = 0.0f;
			if( optional_string("+Translation:")){ //beam texture moveing stuff -Bobboau
				stuff_float(&(i.translation));			
			}


			// maybe copy it
			if(wip->b_info.beam_num_sections < MAX_BEAM_SECTIONS - 1){
				wip->b_info.sections[wip->b_info.beam_num_sections++] = i;
			}
		}		
	}

	if(wip->wi_flags & WIF_PARTICLE_SPEW){
		if(optional_string("$Pspew:")){
			required_string("+Count:");
			stuff_int(&wip->Weapon_particle_spew_count);
			required_string("+Time:");
			stuff_int(&wip->Weapon_particle_spew_time);
			required_string("+Vel:");
			stuff_float(&wip->Weapon_particle_spew_vel);
			required_string("+Radius:");
			stuff_float(&wip->Weapon_particle_spew_radius);
			required_string("+Life:");
			stuff_float(&wip->Weapon_particle_spew_lifetime);
			required_string("+Scale:");
			stuff_float(&wip->Weapon_particle_spew_scale);
			required_string("+Bitmap:");
			stuff_string(wip->Weapon_particle_spew_bitmap_name, F_NAME, NULL);
		
			wip->Weapon_particle_spew_bitmap = bm_load( wip->Weapon_particle_spew_bitmap_name );
			if(wip->Weapon_particle_spew_bitmap < 0){	//if it couldn't find the pcx look for an ani-Bobboau
				nprintf(("General","couldn't find particle pcx for %s \n", wip->name));
				wip->Weapon_particle_spew_bitmap = bm_load_animation(wip->Weapon_particle_spew_bitmap_name, &wip->Weapon_particle_spew_nframes, &wip->Weapon_particle_spew_fps, 1);				
				if(wip->Weapon_particle_spew_bitmap < 0){
					nprintf(("General","couldn't find ani for %s \n", wip->name));
				Warning( LOCATION, "Couldn't open paticle texture '%s'\nreferenced by weapon '%s'\n", wip->Weapon_particle_spew_bitmap_name, wip->name );
				}else{
					nprintf(("General","found ani %s for %s, with %d frames and %d fps \n", wip->Weapon_particle_spew_bitmap_name, wip->name, wip->Weapon_particle_spew_nframes, wip->Weapon_particle_spew_fps));
				}
			}else{
				wip->Weapon_particle_spew_nframes = 1;
				wip->Weapon_particle_spew_fps = 0;
			}

		}else{
			wip->Weapon_particle_spew_count = 1;
			wip->Weapon_particle_spew_time = 25;
			wip->Weapon_particle_spew_vel = 0.4f;
			wip->Weapon_particle_spew_radius = 2.0f;
			wip->Weapon_particle_spew_lifetime = 0.15f;
			wip->Weapon_particle_spew_scale = 0.8f;
			wip->Weapon_particle_spew_bitmap = -1;
		}
	}

	// tag weapon optional stuff
	wip->tag_level = -1;
	wip->tag_time = -1.0f;
	if( optional_string("$Tag:")){
		stuff_int(&wip->tag_level);
		stuff_float(&wip->tag_time);		
		wip->wi_flags |= WIF_TAG;
	}	

	wip->SSM_index =-1;				// tag C SSM index, wich entry in the SSM table this weapon calls -Bobboau
	if( optional_string("$SSM:")){
		stuff_int(&wip->SSM_index);
	}// SSM index -Bobboau


	wip->field_of_fire = 0.0f;
	if( optional_string("$FOF:")){
		stuff_float(&wip->field_of_fire);
	}

	wip->shots = 1;
	if( optional_string("$Shots:")){
		stuff_int(&wip->shots);
	}


	wip->decal_texture = -1;
	wip->decal_glow_texture = -1;
	wip->decal_burn_texture = -1;
	wip->decal_backface_texture = -1;
	wip->decal_rad = -1;
	wip->decal_burn_time = 1000;
	if( optional_string("$decal:")){
		char tex_name[64], temp[64];
		required_string("+texture:");
		stuff_string(tex_name, F_NAME, NULL);
		wip->decal_texture = bm_load(tex_name);
		strcpy(temp, tex_name);
		strcat(tex_name, "-glow");
		wip->decal_glow_texture = bm_load(tex_name);
		strcpy(tex_name, temp);
		strcat(tex_name, "-burn");
		wip->decal_burn_texture = bm_load(tex_name);
		if( optional_string("+backface texture:")){
			stuff_string(tex_name, F_NAME, NULL);
			wip->decal_backface_texture = bm_load(tex_name);
		}
		required_string("+radius:");
		stuff_float(&wip->decal_rad);
		if( optional_string("+burn time:")){
			stuff_int(&wip->decal_burn_time);
		}
	}

	//pretty stupid if a target must be tagged to shoot tag missiles at it
	if ((wip->wi_flags & WIF_TAG) && (wip->wi_flags2 & WIF2_TAGGED_ONLY))
	{
		Warning(LOCATION, "%s is a tag missile, but the target must be tagged to shoot it", wip->name);
	}

	return 0;
}

// function to parse the information for a specific ship type.	
void parse_cmeasure()
{
	cmeasure_info *cmeasurep;

	cmeasurep = &Cmeasure_info[Num_cmeasure_types];

	required_string("$Name:");
	stuff_string(cmeasurep->cmeasure_name, F_NAME, NULL);

/*$Name:					Type One
$Velocity:				20.0				;; speed relative to ship, rear-fired until POF info added, MK, 5/22/97
$Fire Wait:				0.5
$Lifetime Min:			1.0				;; Minimum lifetime
$Lifetime Max:			2.0				;; Maximum lifetime.  Actual lifetime is rand(min..max).
$LaunchSnd:				counter_1.wav,	.8, 10, 300	;; countermeasure 1 fired (sound is 3d)
*/

	required_string("$Velocity:");
	stuff_float( &(cmeasurep->max_speed) );

	required_string("$Fire Wait:");
	stuff_float( &(cmeasurep->fire_wait) );

	required_string("$Lifetime Min:");
	stuff_float(&cmeasurep->life_min);

	required_string("$Lifetime Max:");
	stuff_float(&cmeasurep->life_max);

	//Launching sound
	parse_sound("$LaunchSnd:", &cmeasurep->launch_sound, cmeasurep->cmeasure_name);

	required_string("$Model:");
	stuff_string(cmeasurep->pof_name, F_NAME, NULL);
	cmeasurep->model_num = -1;		
}


//	For all weapons that spawn weapons, given an index at weaponp->spawn_type,
// convert the strings in Spawn_names to indices in the Weapon_types array.
void translate_spawn_types()
{
	int	i,j;

	for (i=0; i<Num_weapon_types; i++)
		if (Weapon_info[i].spawn_type != -1) {
			int	spawn_type = Weapon_info[i].spawn_type;

			for (j=0; j<Num_weapon_types; j++)
				if (!stricmp(Spawn_names[spawn_type], Weapon_info[j].name)) {
					Weapon_info[i].spawn_type = (short)j;
					if (i == j){
						Warning(LOCATION, "Weapon %s spawns itself.  Infinite recursion?\n", Weapon_info[i].name);
					}
				}
		}
}

void parse_tertiary_cloak(tertiary_weapon_info* twip)
{
	required_string("+Warmup Time:");
	stuff_int(&twip->cloak_warmup);

	required_string("+Cooldown Time:");
	stuff_int(&twip->cloak_cooldown);

	required_string("+Lifetime:");
	stuff_int(&twip->cloak_lifetime);

}

void parse_tertiary_ammo(tertiary_weapon_info* twip)
{
	required_string("+Capacity:");
	stuff_int(&twip->ammopod_capacity);
}

void parse_tertiary_boost(tertiary_weapon_info* twip)
{
	required_string("+Num Shots:");
	stuff_int(&twip->boost_num_shots);

	required_string("+Lifetime:");
	stuff_int(&twip->boost_lifetime);

	required_string("+Max Speed:");
	stuff_float(&twip->boost_speed);

	required_string("+Acceleration:");
	stuff_float(&twip->boost_acceleration);
}

void parse_tertiary_jammer(tertiary_weapon_info* twip)
{
}

void parse_tertiary_turbo(tertiary_weapon_info* twip)
{
}

void parse_tertiary_reactor(tertiary_weapon_info* twip)
{
	required_string("+Additional Weapon Power:");
	stuff_float(&twip->reactor_add_weap_pwr);

	required_string("+Additional Shield Power:");
	stuff_float(&twip->reactor_add_shield_pwr);
}

void parse_tertiary()
{
	tertiary_weapon_info* twip=&Tertiary_weapon_info[Num_tertiary_weapon_types];
	
	required_string("$Name:");
	stuff_string(twip->name,F_NAME,NULL);

	required_string("$Type:");
	stuff_int(&twip->type);

	switch (twip->type)
	{
		case TWT_CLOAK_DEVICE:
			parse_tertiary_cloak(twip);
			break;

		case TWT_AMMO_POD:
			parse_tertiary_ammo(twip);
			break;

		case TWT_BOOST_POD:
			parse_tertiary_boost(twip);
			break;

		case TWT_RADAR_JAMMER:
		case TWT_SUPER_JAMMER:
			parse_tertiary_jammer(twip);
			break;

		case TWT_TURBOCHARGER:
			parse_tertiary_turbo(twip);
			break;

		case TWT_EXTRA_REACTOR:
			parse_tertiary_reactor(twip);
			break;

		default:
		Int3();
	}

}

char current_weapon_table[MAX_PATH_LEN + MAX_FILENAME_LEN];
void parse_weaponstbl(char* longname, bool is_chunk)
{
	strcpy(current_weapon_table, longname);
	// open localization
	lcl_ext_open();

	read_file_text(longname);
	reset_parse();
	
	if(!is_chunk)
	{
		required_string("#Primary Weapons");

		while (required_string_either("#End", "$Name:")) {
			Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_LASER, false) ) {
				continue;
			}
		}
		required_string("#End");


		required_string("#Secondary Weapons");
		while (required_string_either("#End", "$Name:")) {
			Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_MISSILE, false) ) {
				continue;
			}
		}
		required_string("#End");

		required_string("#Beam Weapons");
		while (required_string_either("#End", "$Name:")) {
			Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_BEAM, false) ) {
				continue;
			}
		}
		required_string("#End");

		strcpy(parse_error_text, "in the counter measure table entry");

		required_string("#Countermeasures");
		while (required_string_either("#End", "$Name:")) {
			Assert( Num_cmeasure_types < MAX_CMEASURE_TYPES );
			parse_cmeasure();
			Num_cmeasure_types++;
		}

		strcpy(parse_error_text, "");

		required_string("#End");
	}
	else
	{
		if(optional_string("#Primary Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if ( parse_weapon(WP_LASER, true) ) {
					continue;
				}
			}
			required_string("#End");
		}


		if(optional_string("#Secondary Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if ( parse_weapon(WP_MISSILE, true) ) {
					continue;
				}
			}
			required_string("#End");
		}

		if(optional_string("#Beam Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				Assert( Num_weapon_types <= MAX_WEAPON_TYPES );	// Goober5000 - should be <=
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if ( parse_weapon(WP_BEAM, true) ) {
					continue;
				}
			}
			required_string("#End");
		}

		strcpy(parse_error_text, "in the counter measure table entry");

		if(optional_string("#Countermeasures"))
		{
			while (required_string_either("#End", "$Name:")) {
				Assert( Num_cmeasure_types < MAX_CMEASURE_TYPES );
				parse_cmeasure();
				Num_cmeasure_types++;
			}

			required_string("#End");
		}

		strcpy(parse_error_text, "");
	}

	//maybe do some tertiary parsing
	if (optional_string("#Tertiary Weapons"))
	{
		while (required_string_either("#End", "$Name:")) {
			Assert( Num_tertiary_weapon_types < MAX_TERTIARY_WEAPON_TYPES );
			parse_tertiary();
			Num_tertiary_weapon_types++;
		}

		strcpy(parse_error_text, "");
		required_string("#End");
	}

	// Read in a list of weapon_info indicies that are an ordering of the player weapon precedence.
	// This list is used to select an alternate weapon when a particular weapon is not available
	// during weapon selection.
	if(!is_chunk)
	{
		required_string("$Player Weapon Precedence:");
	}
	if(!is_chunk || optional_string("$Player Weapon Precedence:"))
	{
		strcpy(parse_error_text, "in the player weapon precedence list");
		Num_player_weapon_precedence = stuff_int_list(Player_weapon_precedence, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
		strcpy(parse_error_text, "");
	}

	translate_spawn_types();

	// close localization
	lcl_ext_close();
}

void create_weapon_names()
{
	int	i;

	for (i=0; i<Num_weapon_types; i++)
		Weapon_names[i] = Weapon_info[i].name;
}

void sort_weapons_by_type()
{
	weapon_info* lasers = new weapon_info[MAX_WEAPON_TYPES]; int num_lasers=0;
	weapon_info* beams = new weapon_info[MAX_WEAPON_TYPES]; int num_beams=0;
	weapon_info* missiles = new weapon_info[MAX_WEAPON_TYPES]; int num_missiles=0;
	int i;

	for (i=0; i < MAX_WEAPON_TYPES; i++)
	{
		switch (Weapon_info[i].subtype)
		{
			case WP_LASER:
				lasers[num_lasers++]=Weapon_info[i];
				break;
		
			case WP_BEAM:
				beams[num_beams++]=Weapon_info[i];
				break;

			case WP_MISSILE:
				missiles[num_missiles++]=Weapon_info[i];
				break;
			default:
				continue;
		}
	}

	for (i=0; i < num_lasers; i++)
	{
		Weapon_info[i] = lasers[i];
	}

	for (i=0; i < num_beams; i++)
	{
		Weapon_info[i+num_lasers] = beams[i];
	}

	First_secondary_index = num_lasers+num_beams;

	for (i=0; i < num_missiles; i++)
	{
		Weapon_info[i+num_lasers+num_beams] = missiles[i];
	}

	delete [] lasers;
	delete [] beams;
	delete [] missiles;
}

void reset_weapon_info()
{
	memset(Weapon_info, 0, sizeof(weapon_info)*MAX_WEAPON_TYPES);

	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		Weapon_info[i].subtype = WP_UNUSED;
	}
}

void weapons_info_close();
// This will get called once at game startup
void weapon_init()
{
	atexit(weapons_info_close);
	int rval;
	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];

	if ( !Weapons_inited ) {
#ifndef FS2_DEMO
		// parse weapon_exp.tbl
		Num_weapon_expl = 0;
		parse_weapon_expl_tbl("weapon_expl.tbl");

		int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, "*-wxp.tbm", CF_SORT_REVERSE);
		for(int i = 0; i < num_files; i++)
		{
			//HACK HACK HACK
			modular_tables_loaded = true;
			strcat(tbl_file_names[i], ".tbm");
			parse_weapon_expl_tbl(tbl_file_names[i]);
		}

#endif


		// parse weapons.tbl
		if ((rval = setjmp(parse_abort)) != 0) {
			Error(LOCATION, "Error parsing '%s'\r\nError code = %i.\r\n", rval, current_weapon_table);
		}
		else
		{	
			reset_weapon_info();
			Num_weapon_types = 0;
			Num_spawn_types = 0;
			parse_weaponstbl("weapons.tbl", false);

			int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, "*-wep.tbm", CF_SORT_REVERSE);
			for(int i = 0; i < num_files; i++)
			{
				//HACK HACK HACK
				modular_tables_loaded = true;
				strcat(tbl_file_names[i], ".tbm");
				parse_weaponstbl(tbl_file_names[i], true);
			}
			create_weapon_names();
			sort_weapons_by_type();
			Weapons_inited = 1;
		}
	}

	weapon_level_init();
}


// This will get called at the start of each level.
void weapon_level_init()
{
	int i;

	// Reset everything between levels
	Num_weapons = 0;
	for (i=0; i<MAX_WEAPONS; i++)	{
		Weapons[i].objnum = -1;
		Weapons[i].weapon_info_index = -1;
	}

	trail_level_init();		// reset all missile trails

	swarm_level_init();
	missile_obj_list_init();
	
	cscrew_level_init();

	// emp effect
	emp_level_init();

	if (Cmdline_load_only_used) {
		if (used_weapons == NULL)
			used_weapons = new int[Num_weapon_types];

		Assert( used_weapons != NULL );

		// clear out used_weapons between missions
		if (used_weapons != NULL)
			memset(used_weapons, 0, Num_weapon_types * sizeof(int));
	}

	Weapon_flyby_sound_timer = timestamp(0);
	Weapon_impact_timer = 1;	// inited each level, used to reduce impact sounds
}

MONITOR( NumWeaponsRend );	
float add_laser(int texture, vec3d *p0,float width1,vec3d *p1,float width2, int r, int g, int b);

float weapon_glow_scale_f = 2.3f;
float weapon_glow_scale_r = 2.3f;
float weapon_glow_scale_l = 1.5f;
float weapon_glow_alpha = 0.85f;
void weapon_render(object *obj)
{
	int num, frame = 0;
	weapon_info *wip;
	weapon *wp;
	color c;

	MONITOR_INC(NumWeaponsRend, 1);

	Assert(obj->type == OBJ_WEAPON);

	num = obj->instance;
	wp = &Weapons[num];
	wip = &Weapon_info[Weapons[num].weapon_info_index];

	switch (wip->render_type) {
		case WRT_LASER: {
			// turn off fogging for good measure
			gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

			if (wip->laser_bitmap >= 0) {					
				gr_set_color_fast(&wip->laser_color_1);
				if(wip->laser_bitmap_nframes > 1){
					frame = (timestamp() / (int)(wip->laser_bitmap_fps)) % wip->laser_bitmap_nframes;
		//			HUD_printf("frame %d", wp->frame);
				}
			//	gr_set_bitmap(wip->laser_bitmap + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.99999f);

				vec3d headp;
				vm_vec_scale_add(&headp, &obj->pos, &obj->orient.vec.fvec, wip->laser_length);
				wp->weapon_flags &= ~WF_CONSIDER_FOR_FLYBY_SOUND;
			//	if ( g3_draw_laser(&headp, wip->laser_head_radius, &obj->pos, wip->laser_tail_radius,  TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT) ) {
				if(	add_laser(wip->laser_bitmap + frame, &headp, wip->laser_head_radius, &obj->pos, wip->laser_tail_radius, 255, 255, 255)){
					wp->weapon_flags |= WF_CONSIDER_FOR_FLYBY_SOUND;
				}
			}			

			// maybe draw laser glow bitmap
			if(wip->laser_glow_bitmap >= 0)
            
            {

    			// get the laser color
				weapon_get_laser_color(&c, obj);

                // *Tail point "getting bigger" as well as headpoint isn't being taken into consideration, so
                //  it caused uneven glow between the head and tail, which really shows in big lasers. So...fixed!    -Et1

				vec3d headp2, tailp;

				vm_vec_scale_add(&headp2, &obj->pos, &obj->orient.vec.fvec, wip->laser_length * weapon_glow_scale_l);

                vm_vec_scale_add( &tailp, &obj->pos, &obj->orient.vec.fvec, wip->laser_length * ( 1 -  weapon_glow_scale_l ) );


				if(wip->laser_glow_bitmap_nframes > 1){//set the proper bitmap
					frame = (timestamp() / (int)(wip->laser_glow_bitmap_fps)) % wip->laser_glow_bitmap_nframes;
				}
			//	gr_set_bitmap(wip->laser_glow_bitmap + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, weapon_glow_alpha);

				int alpha = fl2i(weapon_glow_alpha * 255.0f);
			//	g3_draw_laser_rgb(&headp2, wip->laser_head_radius * weapon_glow_scale_f, &tailp /*&obj->pos*/, wip->laser_tail_radius * weapon_glow_scale_r, c.red, c.green, c.blue,  TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT  | TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
			//	add_laser(wip->laser_glow_bitmap + frame, &headp2, wip->laser_head_radius * weapon_glow_scale_f, &tailp /*&obj->pos*/, wip->laser_tail_radius * weapon_glow_scale_r, fl2i(c.red*weapon_glow_alpha), fl2i(c.green*weapon_glow_alpha), fl2i(c.blue*weapon_glow_alpha));
				add_laser(wip->laser_glow_bitmap + frame, &headp2, wip->laser_head_radius * weapon_glow_scale_f, &tailp /*&obj->pos*/, wip->laser_tail_radius * weapon_glow_scale_r, (c.red*alpha)/255, (c.green*alpha)/255, (c.blue*alpha)/255);
			}					
			break;
		}

		case WRT_POF:	{
				uint render_flags = MR_NORMAL|MR_IS_MISSILE|MR_NO_LIGHTING;

				model_clear_instance(wip->model_num);

				if ( (wip->wi_flags & WIF_THRUSTER) && (wp->thruster_bitmap > -1) ) {
					float	ft;

					//	Add noise to thruster geometry.
					//ft = obj->phys_info.forward_thrust;					
					ft = 1.0f;		// Always use 1.0f for missiles					
					ft *= (1.0f + frand()/5.0f - 1.0f/10.0f);
					if (ft > 1.0f)
						ft = 1.0f;

					vec3d temp;
					temp.xyz.x = ft;
					temp.xyz.y = ft;
					temp.xyz.z = ft;

					model_set_thrust( wip->model_num, &temp, wp->thruster_bitmap, wp->thruster_glow_bitmap, wp->thruster_glow_noise);
					render_flags |= MR_SHOW_THRUSTERS;
				}


				//don't render local ssm's when they are still in subspace
				if (wp->lssm_stage==3)
					break;

				int clip_plane=0;
				
				//start a clip plane
				if ((wp->lssm_stage==2))
				{
					object *wobj=&Objects[wp->lssm_warp_idx];		//warphole object
					clip_plane=1;


					g3_start_user_clip_plane(&wobj->pos,&wobj->orient.vec.fvec);
				
				}


				model_render(wip->model_num, &obj->orient, &obj->pos, render_flags);

				if (clip_plane)
				{
					g3_stop_user_clip_plane();
				}
				// render a missile plume as well
				/*
				static int plume = -1;	
				extern float Interp_thrust_twist;
				extern float Interp_thrust_twist2;
				if(plume == -1){
					plume = model_load("plume01.pof", -1, NULL);
				}
				if(plume != -1){
					Interp_thrust_twist = tw;
					Interp_thrust_twist2 = tw2;
					model_set_alpha(plume_alpha);
					model_render(plume, &obj->orient, &obj->pos, MR_ALL_XPARENT);
					Interp_thrust_twist = -1.0f;
					Interp_thrust_twist2 = -1.0f;
				}
				*/
			}
			break;

		default:
			Warning(LOCATION, "Unknown weapon rendering type = %i\n", wip->render_type);
	}

//	batch_render();		// why is this here? - taylor
}

void weapon_delete(object *obj)
{
	weapon *wp;
	int num;

	num = obj->instance;

	Assert( Weapons[num].objnum == OBJ_INDEX(obj));
	wp = &Weapons[num];

	Assert(wp->weapon_info_index >= 0);
	wp->weapon_info_index = -1;
	if (wp->swarm_index >= 0) {
		swarm_delete(wp->swarm_index);
		wp->swarm_index = -1;
	}

	if(wp->cscrew_index >= 0) {
		cscrew_delete(wp->cscrew_index);
		wp->cscrew_index = -1;
	}

	if (wp->missile_list_index >= 0) {
		missle_obj_list_remove(wp->missile_list_index);
		wp->missile_list_index = -1;
	}

	if (wp->flak_index >= 0){
		flak_delete(wp->flak_index);
		wp->flak_index = -1;
	}

	if (wp->trail_ptr != NULL) {
		trail_object_died(wp->trail_ptr);
		wp->trail_ptr = NULL;
	}

	wp->objnum = -1;
	Num_weapons--;
	Assert(Num_weapons >= 0);
}

// Check if missile is newly locked onto the Player, maybe play a launch warning
void weapon_maybe_play_warning(weapon *wp)
{
	if ( wp->homing_object == Player_obj ) {
		if ( !(wp->weapon_flags & WF_LOCK_WARNING_PLAYED) ) {
			wp->weapon_flags |= WF_LOCK_WARNING_PLAYED;
			if ( Weapon_info[wp->weapon_info_index].wi_flags & WIF_HOMING_HEAT ) {
				snd_play(&Snds[SND_HEATLOCK_WARN]);
			} else {
				Assert(Weapon_info[wp->weapon_info_index].wi_flags & WIF_HOMING_ASPECT);
				snd_play(&Snds[SND_ASPECTLOCK_WARN]);
			}
		}
	}
}

#define	CMEASURE_DETONATE_DISTANCE		40.0f

//	Detonate all missiles near this countermeasure.
void detonate_nearby_missiles(cmeasure *cmp)
{
	missile_obj	*mop;
	vec3d		cmeasure_pos;

	cmeasure_pos = Objects[cmp->objnum].pos;

	mop = GET_FIRST(&Missile_obj_list);
	while(mop != END_OF_LIST(&Missile_obj_list)) {
		object	*objp;
		weapon	*wp;

		objp = &Objects[mop->objnum];
		wp = &Weapons[objp->instance];

		if (wp->team != cmp->team) {
			if ( Missiontime - wp->creation_time > F1_0/2) {
				if (vm_vec_dist_quick(&cmeasure_pos, &objp->pos) < CMEASURE_DETONATE_DISTANCE) {
					if (wp->lifeleft > 0.2f) { 
						//nprintf(("Jim", "Frame %i: Cmeasure #%i detonating weapon #%i\n", Framecount, cmp-Cmeasures, wp-Weapons));
						wp->lifeleft = 0.2f;
						// nprintf(("AI", "Frame %i: Flagging weapon %i for detonation.\n", Framecount, wp-Weapons));
					}
				}
			}
		}

		mop = mop->next;
	}
}

//	Find an object for weapon #num (object *weapon_objp) to home on due to heat.
void find_homing_object(object *weapon_objp, int num)
{
	object		*objp, *old_homing_objp;
	weapon_info	*wip;
	weapon		*wp;
	float			best_dist;

	wp = &Weapons[num];

	wip = &Weapon_info[Weapons[num].weapon_info_index];

	best_dist = 99999.9f;

	// save the old homing object so that multiplayer servers can give the right information
	// to clients if the object changes
	old_homing_objp = wp->homing_object;

	wp->homing_object = &obj_used_list;

	//	Scan all objects, find a weapon to home on.
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_CMEASURE)) {
			if (objp->type == OBJ_CMEASURE)
				if (Cmeasures[objp->instance].flags & CMF_DUD_HEAT)
					continue;

			int homing_object_team = obj_team(objp);
			if ( (homing_object_team != wp->team) || (homing_object_team == TEAM_TRAITOR) ) {
				float		dist;
				float		dot;
				vec3d	vec_to_object;

				if ( objp->type == OBJ_SHIP ) {
					/* Goober5000: commented this out because if they home in on stealth,
					// they should home in on hidden ships too (sorry Sandeep)
					// AL 2-17-98: If ship is immune to sensors, can't home on it (Sandeep says so)!
					if ( Ships[objp->instance].flags & SF_HIDDEN_FROM_SENSORS ) {
						continue;
					}
					*/

					//	MK, 9/4/99.
					//	If this is a player object, make sure there aren't already too many homers.
					//	Only in single player.  In multiplayer, we don't want to restrict it in dogfight on team vs. team.
					//	For co-op, it's probably also OK.
					if (!( Game_mode & GM_MULTIPLAYER )) {
						int	num_homers = compute_num_homing_objects(objp);
						if (Max_allowed_player_homers[Game_skill_level] < num_homers)
							continue;
					}
				}

				//don't look for local ssms that are gone for the time being
				if (objp->type == OBJ_WEAPON)
				{
					if (Weapons[objp->instance].lssm_stage==3)
						continue;
				}

				dist = vm_vec_normalized_dir(&vec_to_object, &objp->pos, &weapon_objp->pos);

				if (objp->type == OBJ_CMEASURE)
					dist *= 0.5f;

				dot = vm_vec_dot(&vec_to_object, &weapon_objp->orient.vec.fvec);

				if (dot > wip->fov) {
					if (dist < best_dist) {
						best_dist = dist;
						wp->homing_object = objp;
						wp->target_sig = objp->signature;

						weapon_maybe_alert_cmeasure_success(objp);
					}
				}
			}
		}
	}

//	if (wp->homing_object->type == OBJ_CMEASURE)
//		nprintf(("AI", "Frame %i: Weapon #%i homing on cmeasure #%i\n", Framecount, num, objp-Objects));

	if (wp->homing_object == Player_obj)
		weapon_maybe_play_warning(wp);

#ifndef NO_NETWORK
	// if the old homing object is different that the new one, send a packet to clients
	if ( MULTIPLAYER_MASTER && (old_homing_objp != wp->homing_object) ) {
		send_homing_weapon_info( num );
	}
#endif
}

//	Scan all countermeasures.  Maybe make weapon_objp home on it.
void find_homing_object_cmeasures_1(object *weapon_objp)
{
	object	*objp;
	weapon	*wp;
	weapon_info	*wip;
	float		best_dot, dist, dot;

	wp = &Weapons[weapon_objp->instance];
	wip = &Weapon_info[wp->weapon_info_index];

	best_dot = wip->fov;			//	Note, setting to this avoids comparison below.

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_CMEASURE) {
			vec3d	vec_to_object;
			dist = vm_vec_normalized_dir(&vec_to_object, &objp->pos, &weapon_objp->pos);

			if (dist < MAX_CMEASURE_TRACK_DIST) {
				float	chance;
				if (wip->wi_flags & WIF_HOMING_ASPECT) {
					chance = 1.0f/2.0f;	//	aspect seeker this likely to chase a countermeasure
				} else {
					chance = 1.0f/1.5f;	//	heat seeker this likely to chase a countermeasure
				}
				if ((objp->signature != wp->cmeasure_ignore_objnum) && (objp->signature != wp->cmeasure_chase_objnum)) {
					if (frand() < chance) {
						wp->cmeasure_ignore_objnum = objp->signature;	//	Don't process this countermeasure again.
						//nprintf(("Jim", "Frame %i: Weapon #%i ignoring cmeasure #%i\n", Framecount, OBJ_INDEX(weapon_objp), objp->signature));
					} else  {
						wp->cmeasure_chase_objnum = objp->signature;	//	Don't process this countermeasure again.
						//nprintf(("Jim", "Frame %i: Weapon #%i CHASING cmeasure #%i\n", Framecount, OBJ_INDEX(weapon_objp), objp->signature));
					}
				}
				
				if (objp->signature != wp->cmeasure_ignore_objnum) {

					dot = vm_vec_dot(&vec_to_object, &weapon_objp->orient.vec.fvec);

					if (dot > best_dot) {
						//nprintf(("Jim", "Frame %i: Weapon #%i homing on cmeasure #%i\n", Framecount, weapon_objp-Objects, objp->signature));
						best_dot = dot;
						wp->homing_object = objp;
						weapon_maybe_alert_cmeasure_success(objp);
					}
				}
			}
		}
	}
}


//	Someone launched countermeasures.
//	For all heat-seeking homing objects, see if should favor tracking a countermeasure instead.
void find_homing_object_cmeasures()
{
	object	*weapon_objp;

	// nprintf(("AI", "Scanning for countermeasures in frame %i\n", Framecount));

	if (Cmeasures_homing_check == 0)
		return;

	if (Cmeasures_homing_check <= 0)
		Cmeasures_homing_check = 1;

	Cmeasures_homing_check--;

	for (weapon_objp = GET_FIRST(&obj_used_list); weapon_objp != END_OF_LIST(&obj_used_list); weapon_objp = GET_NEXT(weapon_objp) ) {
		if (weapon_objp->type == OBJ_WEAPON) {
			weapon_info	*wip = &Weapon_info[Weapons[weapon_objp->instance].weapon_info_index];

			if (wip->wi_flags & WIF_HOMING)
				find_homing_object_cmeasures_1(weapon_objp);
		}
	}

}

//	Find object with signature "sig" and make weapon home on it.
void find_homing_object_by_sig(object *weapon_objp, int sig)
{
	ship_obj		*sop;
	weapon		*wp;
	object		*old_homing_objp;

	wp = &Weapons[weapon_objp->instance];

	// save the old object so that multiplayer masters know whether to send a homing update packet
	old_homing_objp = wp->homing_object;

	sop = GET_FIRST(&Ship_obj_list);
	while(sop != END_OF_LIST(&Ship_obj_list)) {
		object	*objp;

		objp = &Objects[sop->objnum];
		if (objp->signature == sig) {
			wp->homing_object = objp;
			wp->target_sig = objp->signature;
			break;
		}

		sop = sop->next;
	}

#ifndef NO_NETWORK
	// if the old homing object is different that the new one, send a packet to clients
	if ( MULTIPLAYER_MASTER && (old_homing_objp != wp->homing_object) ) {
		send_homing_weapon_info( weapon_objp->instance );
	}
#endif
}

//	Make weapon num home.  It's also object *obj.
void weapon_home(object *obj, int num, float frame_time)
{
	weapon		*wp;
	weapon_info	*wip;
	object		*hobjp;

	Assert(obj->type == OBJ_WEAPON);
	Assert(obj->instance == num);
	wp = &Weapons[num];
	wip = &Weapon_info[wp->weapon_info_index];
	hobjp = Weapons[num].homing_object;

	//local ssms home only in stages 1 and 5
	if ( (wp->lssm_stage==2) || (wp->lssm_stage==3) || (wp->lssm_stage==4))
		return;

	float max_speed;

	if ((wip->wi_flags2 & WIF2_LOCAL_SSM) && (wp->lssm_stage==5))
		max_speed=wip->lssm_stage5_vel;
	else
		max_speed=wip->max_speed;

	//	If not 1/2 second gone by, don't home yet.
	if ((hobjp == &obj_used_list) || ( f2fl(Missiontime - wp->creation_time) < 0.25f )) {
		//	If this is a heat seeking homing missile and 1/2 second has elapsed since firing
		//	and we don't have a target (else we wouldn't be inside the IF), find a new target.
		if (wip->wi_flags & WIF_HOMING_HEAT)
			if ( f2fl(Missiontime - wp->creation_time) > 0.5f )
				find_homing_object(obj, num);

		if (obj->phys_info.speed > max_speed) {
			obj->phys_info.speed -= frame_time * 4;
			vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);
		} else if ((obj->phys_info.speed < max_speed/4) && (wip->wi_flags & WIF_HOMING_HEAT)) {
			obj->phys_info.speed = max_speed/4;
			vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);
		}

/*	Removed code that makes bombs drop for a bit.  They looked odd and it was confusing.  People wondered where their weapons went.
		//	Make bombs drop down for first second of life.
		if (wip->wi_flags & WIF_BOMB) {
			if (wip->lifetime - wp->lifeleft < 0.5f) {
				float	time_scale = wip->lifetime - wp->lifeleft;
				vm_vec_scale_add2(&obj->phys_info.desired_vel, &obj->orient.vec.uvec, (time_scale - 0.5f) * MAX(10.0f, obj->phys_info.speed/2.0f));
			}
		}
*/
		return;
	}

	// AL 4-8-98: If orgiginal target for aspect lock missile is lost, stop homing
	if (wip->wi_flags & WIF_HOMING_ASPECT) {
		if ( wp->target_sig > 0 ) {
			if ( wp->homing_object->signature != wp->target_sig ) {
				wp->homing_object = &obj_used_list;
				return;
			}
		}
	}

  	// AL 4-13-98: Stop homing on a subsystem if parent ship has changed
	if (wip->wi_flags & WIF_HOMING_HEAT) {
		if ( wp->target_sig > 0 ) {
			if ( wp->homing_object->signature != wp->target_sig ) {
				wp->homing_subsys = NULL;
			}
		}
	}

/*
	if (hobjp->type == OBJ_NONE) {
		find_homing_object(obj, num);
		return;
	}
*/

	switch (hobjp->type) {
	case OBJ_NONE:
		if (wip->wi_flags & WIF_HOMING_ASPECT)
			find_homing_object_by_sig(obj, wp->target_sig);
		else
			find_homing_object(obj, num);
		return;
		break;
	case OBJ_SHIP:
		if (hobjp->signature != wp->target_sig) {
			if (wip->wi_flags & WIF_HOMING_ASPECT)
				find_homing_object_by_sig(obj, wp->target_sig);
			else
				find_homing_object(obj, num);
			return;
		}
		break;
	case OBJ_WEAPON:
		// only allowed to home on bombs
		Assert(Weapon_info[Weapons[hobjp->instance].weapon_info_index].wi_flags & WIF_BOMB);
		if (wip->wi_flags & WIF_HOMING_ASPECT)
			find_homing_object_by_sig(obj, wp->target_sig);
		else
			find_homing_object(obj, num);
		break;
	case OBJ_CMEASURE:
		break;
	default:
		return;
	}

	//	See if this weapon is the nearest homing object to the object it is homing on.
	//	If so, update some fields in the target object's ai_info.
	if (hobjp != &obj_used_list) {
		float	dist;

		dist = vm_vec_dist_quick(&obj->pos, &hobjp->pos);

		if (hobjp->type == OBJ_SHIP) {
			ai_info	*aip;

			aip = &Ai_info[Ships[hobjp->instance].ai_index];

			if ((aip->nearest_locked_object == -1) || (dist < aip->nearest_locked_distance)) {
				aip->nearest_locked_object = obj-Objects;
				aip->nearest_locked_distance = dist;
			}
		}
	}

	//	If the object it is homing on is still valid, home some more!
	if (hobjp != &obj_used_list) {
		float		old_dot, vel;
		vec3d	vec_to_goal;
		vec3d	target_pos;	// position of what the homing missile is seeking

		vm_vec_zero(&target_pos);

		// the homing missile may be seeking a subsystem on a ship.  If so, we need to calculate the
		// world coordinates of that subsystem so the homing missile can seek it out.
		//	For now, March 7, 1997, MK, heat seeking homing missiles will be able to home on
		//	any subsystem.  Probably makes sense for them to only home on certain kinds of subsystems.
		if ( wp->homing_subsys != NULL ) {
			get_subsystem_world_pos(hobjp, Weapons[num].homing_subsys, &target_pos);
			wp->homing_pos = target_pos;	// store the homing position in weapon data
			Assert( !vm_is_vec_nan(&wp->homing_pos) );
		} else {
			float	fov;
			float	dist;

			dist = vm_vec_dist_quick(&obj->pos, &hobjp->pos);
			if (hobjp->type == OBJ_CMEASURE) {
				if (dist < CMEASURE_DETONATE_DISTANCE) {
					cmeasure	*cmp;

					cmp = &Cmeasures[hobjp->instance];

					//	Make this missile detonate soon.  Not right away, not sure why.  Seems better.
					if (cmp->team != wp->team) {
						detonate_nearby_missiles(cmp);
						//nprintf(("AI", "Frame %i: Weapon %i hit cmeasure, will die!\n", Framecount, wp-Weapons));
						return;
					}
				}
			}

			fov = 0.8f;
			if (wip->fov > 0.8f)
				fov = wip->fov;

			int pick_homing_point = 0;
			if ( IS_VEC_NULL(&wp->homing_pos) ) {
				pick_homing_point = 1;
			}

			//	Update homing position if it hasn't been set, you're within 500 meters, or every half second, approximately.
			//	For large objects, don't lead them.
			if (hobjp->radius < 40.0f) {
				target_pos = hobjp->pos;
				wp->homing_pos = target_pos;
			} else if ( pick_homing_point || (dist < 500.0f) || (rand_chance(flFrametime, 2.0f)) ) {

				if (hobjp->type == OBJ_SHIP) {
					if ( !pick_homing_point ) {
						// ensure that current attack point is only updated in world coords (ie not pick a different vertex)
						wp->pick_big_attack_point_timestamp = 0;
					}

					if ( pick_homing_point ) {
						// If *any* player is parent of homing missile, then use position where lock indicator is
						if ( Objects[obj->parent].flags & OF_PLAYER_SHIP ) {
							player *pp;

							// determine the player
							pp = Player;
#ifndef NO_NETWORK
							if ( Game_mode & GM_MULTIPLAYER ) {
								int pnum;

								pnum = multi_find_player_by_object( &Objects[obj->parent] );
								if ( pnum != -1 ){
									pp = Net_players[pnum].m_player;
								}
							}
#endif

							// If player has apect lock, we don't want to find a homing point on the closest
							// octant... setting the timestamp to 0 ensures this.
							if (wip->wi_flags & WIF_HOMING_ASPECT) {
								wp->pick_big_attack_point_timestamp = 0;
							} else {
								wp->pick_big_attack_point_timestamp = 1;
							}

							if ( pp && pp->locking_subsys ) {
								wp->big_attack_point = pp->locking_subsys->system_info->pnt;
							} else {
								vm_vec_zero(&wp->big_attack_point);
							}
						}
					}

					ai_big_pick_attack_point(hobjp, obj, &target_pos, fov);

				} else {
					target_pos = hobjp->pos;
				}

				wp->homing_pos = target_pos;
				Assert( !vm_is_vec_nan(&wp->homing_pos) );
				// nprintf(("AI", "Attack point = %7.3f %7.3f %7.3f\n", target_pos.xyz.x, target_pos.xyz.y, target_pos.xyz.z));
			} else
				target_pos = wp->homing_pos;
		}

		//	Couldn't find a lock.
		if (IS_VEC_NULL(&target_pos))
			return;

		//	Cause aspect seeking weapon to home at target's predicted position.
		//	But don't use predicted position if dot product small or negative.
		//	If do this, with a ship headed towards missile, could choose a point behind missile.
		float	dist_to_target, time_to_target;
		
		dist_to_target = vm_vec_normalized_dir(&vec_to_goal, &target_pos, &obj->pos);
		time_to_target = dist_to_target/max_speed;

		vec3d	tvec;
		tvec = obj->phys_info.vel;
		vm_vec_normalize(&tvec);

		old_dot = vm_vec_dot(&tvec, &vec_to_goal);

		//	If a weapon has missed its target, detonate it.
		//	This solves the problem of a weapon circling the center of a subsystem that has been blown away.
		//	Problem: It does not do impact damage, just proximity damage.
		if ((dist_to_target < flFrametime * obj->phys_info.speed * 4.0f + 10.0f) && (old_dot < 0.0f)) {
			int kill_missile = TRUE;
			if (wp->homing_object) {
				if (wp->homing_object->type == OBJ_SHIP) {
					ship *shipp = &Ships[wp->homing_object->instance];
					if (shipp->flags2 & SF2_DONT_COLLIDE_INVIS) {
						kill_missile = FALSE;
					}
				}
			}
			
			if (kill_missile && (wp->lifeleft > 0.01f)) {
				wp->lifeleft = 0.01f;
			}
		}

		//	Only lead target if more than one second away.  Otherwise can miss target.  I think this
		//	is what's causing Harbingers to miss the super destroyer. -- MK, 4/15/98
		if ((wip->wi_flags & WIF_HOMING_ASPECT) && (old_dot > 0.1f) && (time_to_target > 0.1f))
			vm_vec_scale_add2(&target_pos, &hobjp->phys_info.vel, MIN(time_to_target, 2.0f));

		//nprintf(("AI", "Dot = %7.3f, dist = %7.3f, time_to = %6.3f, deg/sec = %7.3f\n", old_dot, dist_to_target, time_to_target, angles/flFrametime));

		// nprintf(("AI", "Weapon %i, lifeleft = %7.3f, dist = %7.3f, dot = %6.3f\n", num, Weapons[num].lifeleft, vm_vec_dist_quick(&obj->pos, &Weapons[num].homing_object->pos), old_dot));

		//	If a HEAT seeking (rather than ASPECT seeking) homing missile, verify that target is in viewcone.
		if (wip->wi_flags & WIF_HOMING_HEAT) {
			if ((old_dot < wip->fov) && (dist_to_target > wip->inner_radius*1.1f)) {	//	Delay finding new target one frame to allow detonation.
				find_homing_object(obj, num);
				return;			//	Maybe found a new homing object.  Return, process more next frame.
			} else	//	Subtract out life based on how far from target this missile points.
				if (wip->fov < 0.95f) {
					wp->lifeleft -= flFrametime * (0.95f - old_dot);
					//Should only happen when time is compressed.
					//if (flFrametime * (1.0f - old_dot) > 1.0f)
					//	Int3();
				}
		} else if (wip->wi_flags & WIF_HOMING_ASPECT) {	//	subtract life as if max turn is 90 degrees.
			if (wip->fov < 0.95f)
				wp->lifeleft -= flFrametime * (0.95f - old_dot);
		} else
			Assert(0);	//	Hmm, a homing missile, but not aspect or heat?


		//	Control speed based on dot product to goal.  If close to straight ahead, move
		//	at max speed, else move slower based on how far from ahead.
		if (old_dot < 0.90f) {
			obj->phys_info.speed = MAX(0.2f, old_dot* (float) fabs(old_dot));
			if (obj->phys_info.speed < max_speed*0.75f)
				obj->phys_info.speed = max_speed*0.75f;
		} else
			obj->phys_info.speed = max_speed;

		//	For first second of weapon's life, it doesn't fly at top speed.  It ramps up.
		if (Missiontime - wp->creation_time < i2f(1)) {
			float	t;

			t = f2fl(Missiontime - wp->creation_time);
			obj->phys_info.speed *= t*t;
		}

		Assert( obj->phys_info.speed > 0.0f );

		vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);

		// turn the missile towards the target only if non-swarm.  Homing swarm missiles choose
		// a different vector to turn towards, this is done in swarm_update_direction().
//		if ( !(wip->wi_flags & WIF_SWARM) ) {
		if ( wp->swarm_index < 0 ) {
			// nprintf(("AI", "Dot, dist = %7.3f, %7.3f, target pos = %7.3f %7.3f %7.3f\n", old_dot, vm_vec_dist_quick(&obj->pos, &target_pos), target_pos.xyz.x, target_pos.xyz.y, target_pos.xyz.z));
			ai_turn_towards_vector(&target_pos, obj, frame_time, wip->turn_time, NULL, NULL, 0.0f, 0, NULL);
			vel = vm_vec_mag(&obj->phys_info.desired_vel);

			vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, vel);

		}

/*		//	If this weapon shot past its target, make it detonate.
		if ((old_dot < 0.0f) && (dist_to_target < 50.0f)) {
			if (wp->lifeleft > 0.01f)
				wp->lifeleft = 0.01f;
		}
*/	}
}

// as Mike K did with ships -- break weapon into process_pre and process_post for code to execute
// before and after physics movement

void weapon_process_pre( object *obj, float frame_time)
{
	// if the object is a corkscrew style weapon, process it now
	if((obj->type == OBJ_WEAPON) && (Weapons[obj->instance].cscrew_index >= 0)){
		cscrew_process_pre(obj);
	}

	// if the weapon is a flak weapon, maybe detonate it early
	if((obj->type == OBJ_WEAPON) && (Weapon_info[Weapons[obj->instance].weapon_info_index].wi_flags & WIF_FLAK) && (Weapons[obj->instance].flak_index >= 0)){
		flak_maybe_detonate(obj);		
	}
}

int	Homing_hits = 0, Homing_misses = 0;


MONITOR( NumWeapons );	

// maybe play a "whizz sound" if close enough to view position
void weapon_maybe_play_flyby_sound(object *weapon_objp, weapon *wp)
{	
	// do a quick out if not a laser
	if ( Weapon_info[wp->weapon_info_index].subtype != WP_LASER ) {
		return;
	}

	// don't play flyby sounds too close together
	if ( !timestamp_elapsed(Weapon_flyby_sound_timer) ) {
		return;
	}

	if ( !(wp->weapon_flags & WF_PLAYED_FLYBY_SOUND) && (wp->weapon_flags & WF_CONSIDER_FOR_FLYBY_SOUND) ) {
		float		dist, dot, radius;

		dist = vm_vec_dist_quick(&weapon_objp->pos, &Eye_position);

		if ( Viewer_obj ) {
			radius = Viewer_obj->radius;
		} else {
			radius = 0.0f;
		}

		if ( (dist > radius) && (dist < 55) ) {
			vec3d	vec_to_weapon;

			vm_vec_sub(&vec_to_weapon, &weapon_objp->pos, &Eye_position);
			vm_vec_normalize(&vec_to_weapon);

			// ensure laser is in front of eye
			dot = vm_vec_dot(&vec_to_weapon, &Eye_matrix.vec.fvec);
			if ( dot < 0.1 ) {
				return;
			}

			// ensure that laser is moving in similar direction to fvec
			dot = vm_vec_dot(&vec_to_weapon, &weapon_objp->orient.vec.fvec);
			
//			nprintf(("Alan", "Weapon dot: %.2f\n", dot));
			if ( (dot < -0.80) && (dot > -0.98) ) {
				snd_play_3d( &Snds[SND_WEAPON_FLYBY], &weapon_objp->pos, &Eye_position );
				Weapon_flyby_sound_timer = timestamp(200);
				wp->weapon_flags |= WF_PLAYED_FLYBY_SOUND;
			}
		}
	}
}

// process a weapon after physics movement.  MWA reorders some of the code on 8/13 for multiplayer.  When
// adding something to this function, decide whether or not a client in a multiplayer game needs to do
// what is normally done in a single player game.  Things like plotting an object on a radar, effect
// for exhaust are things that are done on all machines.  Things which calculate weapon targets, new
// velocities, etc, are server only functions and should go after the if ( !MULTIPLAYER_MASTER ) statement
// See Allender if you cannot decide what to do.
void weapon_process_post(object * obj, float frame_time)
{
	int			num;	
	weapon_info	*wip;
	weapon		*wp;

	MONITOR_INC( NumWeapons, 1 );	
	
	Assert(obj->type == OBJ_WEAPON);

	num = obj->instance;

#ifndef NDEBUG
	int objnum;
	objnum = OBJ_INDEX(obj);
	Assert( Weapons[num].objnum == objnum );
#endif

	wp = &Weapons[num];

	wp->lifeleft -= frame_time;

	wip = &Weapon_info[wp->weapon_info_index];

	
	if (wip->wi_flags2 & WIF2_LOCAL_SSM)
	{
		if ((wp->lssm_stage != 5) && (wp->lssm_stage != 0))
		{
			wp->lifeleft += frame_time;
		}
	}


	// check life left.  Multiplayer client code will go through here as well.  We must be careful in weapon_hit
	// when killing a missile that spawn child weapons!!!!
	if ( wp->lifeleft < 0.0f ) {
		if ( wip->subtype & WP_MISSILE ) {
#ifndef NO_NETWORK
			if(Game_mode & GM_MULTIPLAYER){				
				if ( !MULTIPLAYER_CLIENT || (MULTIPLAYER_CLIENT && (wp->lifeleft < -2.0f)) || (MULTIPLAYER_CLIENT && (wip->wi_flags & WIF_CHILD))) {					// don't call this function multiplayer client -- host will send this packet to us
					// nprintf(("AI", "Frame %i: Weapon %i detonated, dist = %7.3f!\n", Framecount, obj-Objects));
					weapon_detonate(obj);					
				}
			}
			else
#endif
			{
				// nprintf(("AI", "Frame %i: Weapon %i detonated, dist = %7.3f!\n", Framecount, obj-Objects));
				weapon_detonate(obj);									
			}
			if (wip->wi_flags & WIF_HOMING) {
				Homing_misses++;
				// nprintf(("AI", "Miss!  Hits = %i/%i\n", Homing_hits, (Homing_hits + Homing_misses)));
			}
		} else {
			obj->flags |= OF_SHOULD_BE_DEAD;
//			demo_do_flag_dead(OBJ_INDEX(obj));
		}

		return;
	}

	// plot homing missiles on the radar
	if (wip->wi_flags & WIF_HOMING) {
		if ( hud_gauge_active(HUD_RADAR) ) {
			radar_plot_object( obj );
		}
	}

	// trail missiles
	if ((wip->wi_flags & WIF_TRAIL) && !(wip->wi_flags & WIF_CORKSCREW)) {
		if ( (wp->trail_ptr != NULL ) && (wp->lssm_stage!=3))	{
			if (trail_stamp_elapsed(wp->trail_ptr)) {

				trail_add_segment( wp->trail_ptr, &obj->pos );
				
				trail_set_stamp(wp->trail_ptr);
			} else {
				trail_set_segment( wp->trail_ptr, &obj->pos );
			}

		}
	}

	if ( wip->wi_flags & WIF_THRUSTER )	{
		ship_do_weapon_thruster_frame( wp, obj, flFrametime );	
	}

	// maybe play a "whizz sound" if close enough to view position
	#ifndef NDEBUG
	if ( Weapon_flyby_sound_enabled ) {
		weapon_maybe_play_flyby_sound(obj, wp);
	}
	#else
		weapon_maybe_play_flyby_sound(obj, wp);
	#endif	
	
	//	If our target is still valid, then update some info.
	if (wp->target_num != -1) {
		if (Objects[wp->target_num].signature == wp->target_sig) {
			float		cur_dist;
			vec3d	v0;

			vm_vec_avg(&v0, &obj->pos, &obj->last_pos);

			cur_dist = vm_vec_dist_quick(&v0, &Objects[wp->target_num].pos);

			if (cur_dist < wp->nearest_dist) {
				wp->nearest_dist = cur_dist;
			} else if (cur_dist > wp->nearest_dist + 1.0f) {
				float		dot;
				vec3d	tvec;
				ai_info	*parent_aip;
				float		lead_scale = 0.0f;

				parent_aip = NULL;
				if (obj->parent != Player_obj-Objects) {
					parent_aip = &Ai_info[Ships[Objects[obj->parent].instance].ai_index];
					lead_scale = parent_aip->lead_scale;
				}

				vm_vec_normalized_dir(&tvec, &v0, &Objects[wp->target_num].pos);
				dot = vm_vec_dot(&tvec, &Objects[wp->target_num].orient.vec.fvec);
				// nprintf(("AI", "Miss dot = %7.3f, dist = %7.3f, lead_scale = %7.3f\n", dot, cur_dist, lead_scale));
				wp->target_num = -1;

				//	Learn!  If over-shooting or under-shooting, compensate.
				//	Really need to compensate for left/right errors.  This does no good against someone circling
				//	in a plane perpendicular to the attacker's forward vector.
				if (parent_aip != NULL) {
					if (cur_dist > 100.0f)
						parent_aip->lead_scale = 0.0f;

					if (dot < -0.1f){
						parent_aip->lead_scale += cur_dist/2000.0f;
					} else if (dot > 0.1f) {
						parent_aip->lead_scale -= cur_dist/2000.0f;
					}
					
					if (fl_abs(parent_aip->lead_scale) > 1.0f){
						parent_aip->lead_scale *= 0.9f;
					}
				}
			}
		}
	}

	if(wip->wi_flags & WIF_PARTICLE_SPEW){
		weapon_maybe_spew_particle(obj);
	}

	// a single player or multiplayer server function -- it affects actual weapon movement.
	if (wip->wi_flags & WIF_HOMING) {
		weapon_home(obj, num, frame_time);

/*		if (wip->wi_flags & WIF_BOMB) {
			if (wip->lifetime - obj->lifeleft < 1.0f) {
				
			}
		}
*/		
		// If this is a swarm type missile, 
//		if ( wip->wi_flags & WIF_SWARM ) 
		if ( wp->swarm_index >= 0 ) {
			swarm_update_direction(obj, frame_time);
		}

		if( wp->cscrew_index >= 0) {
			cscrew_process_post(obj);			
		}
	}

	//local ssm stuff
	if (wip->wi_flags2 & WIF2_LOCAL_SSM)
	{

		//go into subspace if the missile is locked and its time to warpout
		if ((wp->lssm_stage==1) && (timestamp_elapsed(wp->lssm_warpout_time)))
		{
			//if we don't have a lock at this point, just stay in normal space
			if (wp->target_num == -1)
			{
				wp->lssm_stage=0;
				return;
			}

			//point where to warpout
			vec3d warpout;

			//create a warp effect
			vm_vec_copy_scale(&warpout,&obj->phys_info.vel,3.0f);

			//set the time the warphole stays open, minimum of 7 seconds
			wp->lssm_warp_time = ((obj->radius * 2) / (obj->phys_info.speed)) +1.5f;
			wp->lssm_warp_time = MAX(wp->lssm_warp_time,7.0f);

			//calculate the percerentage of the warpholes life at which the missile is fully in subspace.
			wp->lssm_warp_pct = 1.0f - (3.0f/wp->lssm_warp_time);

			//create the warphole
			vm_vec_add2(&warpout,&obj->pos);
			wp->lssm_warp_idx=fireball_create(&warpout, FIREBALL_WARP_EFFECT, -1,obj->radius*1.5f,1,&vmd_zero_vector,wp->lssm_warp_time,0,&obj->orient);
			wp->lssm_stage=2;
		}

		//its just entered subspace subspace. don't collide or render
		if ((wp->lssm_stage==2) && (fireball_lifeleft_percent(&Objects[wp->lssm_warp_idx]) <= wp->lssm_warp_pct))
		{
			uint flags=obj->flags & ~(OF_RENDERS | OF_COLLIDES);

			obj_set_flags(obj, flags);
		
			//get the position of the target, and estimate its position when it warps out
			//so we have an idea of where it will be.
			vm_vec_scale_add(&wp->lssm_target_pos,&Objects[wp->target_num].pos,&Objects[wp->target_num].phys_info.vel,(float)wip->lssm_warpin_delay/1000.0f);

			wp->lssm_stage=3;

		}
	
		//time to warp in.
		if ((wp->lssm_stage==3) && (timestamp_elapsed(wp->lssm_warpin_time)))
		{

			vec3d warpin;
			object* target_objp=wp->homing_object;
			vec3d fvec;
			matrix orient;

			//spawn the ssm at a random point in a circle around the target
			vm_vec_random_in_circle(&warpin, &wp->lssm_target_pos, &target_objp->orient, wip->lssm_warpin_radius + target_objp->radius,1);
	
			//orient the missile properly
			vm_vec_sub(&fvec,&wp->lssm_target_pos, &warpin);
			vm_vector_2_matrix(&orient,&fvec,NULL,NULL);

			//create a warpin effect
			wp->lssm_warp_idx=fireball_create(&warpin, FIREBALL_WARP_EFFECT, -1,obj->radius*1.5f,0,&vmd_zero_vector,wp->lssm_warp_time,0,&orient);

			obj->orient=orient;
			obj->pos=warpin;
			obj->phys_info.speed=0;
			obj->phys_info.desired_vel = vmd_zero_vector;
			obj->phys_info.vel = obj->phys_info.desired_vel;
		
			wp->lssm_stage=4;

		}
	

		//done warping in.  render and collide it. let the fun begin
		if ((wp->lssm_stage==4) && (fireball_lifeleft_percent(&Objects[wp->lssm_warp_idx]) <=0.5f))
		{
			vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, wip->lssm_stage5_vel );
			obj->phys_info.vel = obj->phys_info.desired_vel;
			obj->phys_info.speed = vm_vec_mag(&obj->phys_info.desired_vel);

			wp->lssm_stage=5;

			uint flags=obj->flags | OF_RENDERS | OF_COLLIDES;

			obj_set_flags(obj,flags);
		}
	}

}

//	Update weapon tracking information.
void weapon_set_tracking_info(int weapon_objnum, int parent_objnum, int target_objnum, int target_is_locked, ship_subsys *target_subsys)
{
	int			ai_index;
	object		*parent_objp;
	weapon		*wp;
	weapon_info	*wip;
	int targeting_same = 0;

	if ( weapon_objnum < 0 ) {
		return;
	}

	Assert(Objects[weapon_objnum].type == OBJ_WEAPON);

	wp = &Weapons[Objects[weapon_objnum].instance];
	wip = &Weapon_info[wp->weapon_info_index];
	parent_objp = &Objects[parent_objnum];

	Assert(parent_objp->type == OBJ_SHIP);
	ai_index = Ships[parent_objp->instance].ai_index;

	if ( ai_index >= 0 ) {
		int target_team = -1;
		if ( target_objnum >= 0 ) {
			int obj_type = Objects[target_objnum].type;
			if ( (obj_type == OBJ_SHIP) || (obj_type == OBJ_WEAPON) ) {
				target_team = obj_team(&Objects[target_objnum]);
			}
		}
	
		// determining if we're targeting the same team
		if(Ships[parent_objp->instance].team == target_team){
			targeting_same = 1;
		} else {
			targeting_same = 0;
		}

#ifndef NO_NETWORK
		if ((target_objnum != -1) && (!targeting_same || ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (target_team == TEAM_TRAITOR))) ) {
#else
		if ((target_objnum != -1) && (!targeting_same)) {
#endif
			wp->target_num = target_objnum;
			wp->target_sig = Objects[target_objnum].signature;
			wp->nearest_dist = 99999.0f;
			if ( (wip->wi_flags & WIF_HOMING_ASPECT) && target_is_locked) {
				wp->homing_object = &Objects[target_objnum];
				wp->homing_subsys = target_subsys;
				weapon_maybe_play_warning(wp);
			} else if ( wip->wi_flags & WIF_HOMING_HEAT ) {
				//	Make a heat seeking missile try to home.  If the target is outside the view cone, it will
				//	immediately drop it and try to find one in its view cone.
				if (target_objnum != -1) {
					wp->homing_object = &Objects[target_objnum];
					weapon_maybe_play_warning(wp);
				} else
					wp->homing_object = &obj_used_list;

				wp->homing_subsys = target_subsys;
			}
		} else {
			wp->target_num = -1;
			wp->target_sig = -1;
		}

		//	If missile is locked on target, increase its lifetime by 20% since missiles can be fired at limit of range
		//	as defined by velocity*lifeleft, but missiles often slow down a bit, plus can be fired at a moving away target.
		//	Confusing to many players when their missiles run out of gas before getting to target.	
		// DB - removed 7:14 pm 9/6/99. was totally messing up lifetimes for all weapons.
		//	MK, 7:11 am, 9/7/99.  Put it back in, but with a proper check here to make sure it's an aspect seeker and
		//	put a sanity check in the color changing laser code that was broken by this code.
		if (target_is_locked && (wp->target_num != -1) && (wip->wi_flags & WIF_HOMING_ASPECT) ) {
			wp->lifeleft *= 1.2f;
		}

		ai_update_danger_weapon(target_objnum, weapon_objnum);		
	}
}


// weapon_create() will create a weapon object
//
// Returns:  index of weapon in the Objects[] array, -1 if the weapon object was not created
int Weapons_created = 0;
int weapon_create( vec3d * pos, matrix * porient, int weapon_id, int parent_objnum, int secondary_flag, int group_id, int is_locked )
{
	int			n, objnum;
	int num_deleted;
	object		*objp, *parent_objp;
	weapon		*wp;
	weapon_info	*wip;

	wip = &Weapon_info[weapon_id];

	//I am hopeing that this way does not alter the input orient matrix
	//Feild of Fire code -Bobboau
	matrix morient;
	matrix *orient;
	morient = *porient;
	orient = &morient;
	if(wip->field_of_fire){
		vec3d f;
		vm_vec_random_cone(&f, &orient->vec.fvec, wip->field_of_fire);
		vm_vec_normalize(&f);
		vm_vector_2_matrix( orient, &f, NULL, NULL);
	}

	Assert(weapon_id >= 0 && weapon_id < Num_weapon_types);

	// beam weapons should never come through here!
	Assert(!(Weapon_info[weapon_id].wi_flags & WIF_BEAM));

	num_deleted = 0;
	if (Num_weapons >= MAX_WEAPONS-5) {

		//No, do remove for AI ships -- MK, 3/12/98  // don't need to try and delete weapons for ai ships
		//if ( !(Objects[parent_objnum].flags & OF_PLAYER_SHIP) )
		//	return -1;

		num_deleted = collide_remove_weapons();
		nprintf(("WARNING", "Deleted %d weapons because of lack of slots\n", num_deleted));
		if (num_deleted == 0){
			return -1;
		}
	}

	for (n=0; n<MAX_WEAPONS; n++ ){
		if (Weapons[n].weapon_info_index < 0){
			break;
		}
	}

	if (n == MAX_WEAPONS) {
		// if we supposedly deleted weapons above, what happened here!!!!
		if (num_deleted){
			Int3();				// get allender -- something funny is going on!!!
		}

		return -1;
	}

	Weapons_created++;
	objnum = obj_create( OBJ_WEAPON, parent_objnum, n, orient, pos, 2.0f, OF_RENDERS | OF_COLLIDES | OF_PHYSICS );
	Assert(objnum >= 0);
	objp = &Objects[objnum];

	parent_objp = NULL;
	if(parent_objnum >= 0){
		parent_objp = &Objects[parent_objnum];
	}

	// Create laser n!
	wp = &Weapons[n];

	// check if laser or dumbfire missile
	// set physics flag to allow optimization
	if ((wip->subtype == WP_LASER) || ((wip->subtype == WP_MISSILE) && !(wip->wi_flags & WIF_HOMING))) {
		// set physics flag
		objp->phys_info.flags |= PF_CONST_VEL;
	}

	wp->weapon_info_index = weapon_id;
	wp->lifeleft = wip->lifetime;

	wp->objnum = objnum;
	wp->homing_object = &obj_used_list;		//	Assume not homing on anything.
	wp->homing_subsys = NULL;
	wp->creation_time = Missiontime;
	wp->group_id = group_id;

	// we don't necessarily need a parent
	if(parent_objp != NULL){
		Assert(parent_objp->type == OBJ_SHIP);	//	Get Mike, a non-ship has fired a weapon!
		Assert((parent_objp->instance >= 0) && (parent_objp->instance < MAX_SHIPS));
		wp->team = Ships[parent_objp->instance].team;
		wp->species = Ship_info[Ships[parent_objp->instance].ship_info_index].species;
	} else {
		wp->team = 0;
		wp->species = 0;
	}
	wp->turret_subsys = NULL;
	vm_vec_zero(&wp->homing_pos);
	wp->weapon_flags = 0;
	wp->target_sig = -1;
	wp->cmeasure_ignore_objnum = -1;
	wp->cmeasure_chase_objnum = -1;

	// Init the thruster info
	wp->thruster_bitmap = -1;
	wp->thruster_frame = 0.0f;
	wp->thruster_glow_bitmap = -1;
	wp->thruster_glow_noise = 1.0f;
	wp->thruster_glow_frame = 0.0f;

	if ( wip->wi_flags & WIF_SWARM ) {
		wp->swarm_index = (short)swarm_create();
	} else {
		wp->swarm_index = -1;
	}		

	// if this is a particle spewing weapon, setup some stuff
	if(wip->wi_flags & WIF_PARTICLE_SPEW){
		wp->particle_spew_time = -1;		
	}

#ifndef NO_NETWORK
	// assign the network signature.  The starting sig is sent to all clients, so this call should
	// result in the same net signature numbers getting assigned to every player in the game
	if ( Game_mode & GM_MULTIPLAYER ) {
		if(wip->subtype == WP_MISSILE){
			Objects[objnum].net_signature = multi_assign_network_signature( MULTI_SIG_NON_PERMANENT );

			// for weapons that respawn, add the number of respawnable weapons to the net signature pool
			// to reserve N signatures for the spawned weapons
			if ( wip->wi_flags & WIF_SPAWN ){
				multi_set_network_signature( (ushort)(Objects[objnum].net_signature + wip->spawn_count), MULTI_SIG_NON_PERMANENT );
			}
		} else {
			Objects[objnum].net_signature = multi_assign_network_signature( MULTI_SIG_NON_PERMANENT );
		}
		// for multiplayer clients, when creating lasers, add some more life to the lasers.  This helps
		// to overcome some problems associated with lasers dying on client machine before they get message
		// from server saying it hit something.
		// removed 1/13/98 -- MWA if ( MULTIPLAYER_CLIENT && (wip->subtype == WP_LASER) )
		//	removed 1/13/98 -- MWA	wp->lifeleft += 1.5f;
	}
#endif

	//	Make remote detonate missiles look like they're getting detonated by firer simply by giving them variable lifetimes.
	if (!(Objects[parent_objnum].flags & OF_PLAYER_SHIP) && (wip->wi_flags & WIF_REMOTE)) {
		float rand_val;

		if ( Game_mode & GM_NORMAL ){
			rand_val = frand();
		} else {
			rand_val = static_randf(Objects[objnum].net_signature);
		}

		wp->lifeleft = wp->lifeleft/2.0f + rand_val * wp->lifeleft/2.0f;
	}

	objp->phys_info.mass = wip->mass;
	objp->phys_info.side_slip_time_const = 0.0f;
	objp->phys_info.rotdamp = 0.0f;
	vm_vec_zero(&objp->phys_info.max_vel);
	objp->phys_info.max_vel.xyz.z = wip->max_speed;
	vm_vec_zero(&objp->phys_info.max_rotvel);
	objp->shield_quadrant[0] = wip->damage;
	if (wip->wi_flags & WIF_BOMB){
		objp->hull_strength = 50.0f;
	} else {
		objp->hull_strength = 0.0f;
	}

	if ( wip->subtype == WP_MISSILE ){
		objp->radius = model_get_radius(wip->model_num);
	} else if ( wip->subtype == WP_LASER ) {
		objp->radius = wip->laser_head_radius;
	}

	//	Set desired velocity and initial velocity.
	//	For lasers, velocity is always the same.
	//	For missiles, it is a small amount plus the firing ship's velocity.
	//	For missiles, the velocity trends towards some goal.
	//	Note: If you change how speed works here, such as adding in speed of parent ship, you'll need to change the AI code
	//	that predicts collision points.  See Mike Kulas or Dave Andsager.  (Or see ai_get_weapon_speed().)
	if (!(wip->wi_flags & WIF_HOMING)) {
		vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );
		objp->phys_info.vel = objp->phys_info.desired_vel;
		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.desired_vel);
	} else {		
		//	For weapons that home, set velocity to sum of forward component of parent's velocity and 1/4 weapon's max speed.
		//	Note that it is important to extract the forward component of the parent's velocity to factor out sliding, else
		//	the missile will not be moving forward.
		if(parent_objp != NULL){
			vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, vm_vec_dot(&parent_objp->phys_info.vel, &parent_objp->orient.vec.fvec) + objp->phys_info.max_vel.xyz.z/4 );
		} else {
			vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z/4 );
		}
		objp->phys_info.vel = objp->phys_info.desired_vel;
		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.vel);
	}

	// create the corkscrew
	if ( wip->wi_flags & WIF_CORKSCREW ) {
		wp->cscrew_index = (short)cscrew_create(objp);
	} else {
		wp->cscrew_index = -1;
	}

	if (wip->wi_flags2 & WIF2_LOCAL_SSM)
	{

		Assert(parent_objp);		//local ssms must have a parent

		wp->lssm_warpout_time=timestamp(wip->lssm_warpout_delay);
		wp->lssm_warpin_time=timestamp(wip->lssm_warpout_delay + wip->lssm_warpin_delay);
		wp->lssm_stage=1;
	}
	else{
		wp->lssm_stage=-1;
	}


	// if this is a flak weapon shell, make it so
	// NOTE : this function will change some fundamental things about the weapon object
	if ( wip->wi_flags & WIF_FLAK ){
		flak_create(wp);
	} else {
		wp->flak_index = -1;
	}	

	wp->missile_list_index = -1;
	// If this is a missile, then add it to the Missile_obj_list
	if ( wip->subtype == WP_MISSILE ) {
		wp->missile_list_index = missile_obj_list_add(objnum);
	}

	if (wip->wi_flags & WIF_TRAIL /*&& !(wip->wi_flags & WIF_CORKSCREW) */) {
		wp->trail_ptr = trail_create(&wip->tr_info);		

		if ( wp->trail_ptr != NULL )	{
			// Add two segments.  One to stay at launch pos, one to move.
			trail_add_segment( wp->trail_ptr, &objp->pos );
			trail_add_segment( wp->trail_ptr, &objp->pos );
		}
	}
	else
	{
		//If a weapon has no trails, make sure we don't try to do anything with them.
		wp->trail_ptr = NULL;
	}

	// Ensure weapon flyby sound doesn't get played for player lasers
	if ( parent_objp == Player_obj ) {
		wp->weapon_flags |= WF_PLAYED_FLYBY_SOUND;
	}

	wp->pick_big_attack_point_timestamp = timestamp(1);

	//	Set detail levels for POF-type weapons.
	if (Weapon_info[wp->weapon_info_index].model_num != -1) {
		polymodel * pm;
		int	i;
		pm = model_get(Weapon_info[wp->weapon_info_index].model_num);

		for (i=0; i<pm->n_detail_levels; i++){
			pm->detail_depth[i] = (objp->radius*20.0f + 20.0f) * i;
		}
	}

		// if the weapon was fired locked
	if(is_locked){
		wp->weapon_flags |= WF_LOCKED_WHEN_FIRED;
	}

	Num_weapons++;
	return objnum;
}
//	Spawn child weapons from object *objp.
void spawn_child_weapons(object *objp)
{
	int	i;
	int	child_id;
	int	parent_num;
	ushort starting_sig;
	weapon	*wp;
	weapon_info	*wip;

	Assert(objp->type == OBJ_WEAPON);
	Assert((objp->instance >= 0) && (objp->instance < MAX_WEAPONS));

	wp = &Weapons[objp->instance];
	Assert((wp->weapon_info_index >= 0) && (wp->weapon_info_index < MAX_WEAPON_TYPES));
	wip = &Weapon_info[wp->weapon_info_index];

	child_id = wip->spawn_type;

	parent_num = objp->parent;

	if ((Objects[parent_num].type != objp->parent_type) || (Objects[parent_num].signature != objp->parent_sig)) {
		mprintf(("Warning: Parent of spawn weapon does not exist.  Not spawning.\n"));
		return;
	}

	starting_sig = 0;
#ifndef NO_NETWORK
	if ( Game_mode & GM_MULTIPLAYER ) {		
		// get the next network signature and save it.  Set the next usable network signature to be
		// the passed in objects signature + 1.  We "reserved" N of these slots when we created objp
		// for it's spawned children.
		starting_sig = multi_get_next_network_signature( MULTI_SIG_NON_PERMANENT );
		multi_set_network_signature( objp->net_signature, MULTI_SIG_NON_PERMANENT );
	}
#endif

	for (i=0; i<wip->spawn_count; i++) {
		int		weapon_objnum;
		vec3d	tvec, pos;
		matrix	orient;

		// for multiplayer, use the static randvec functions based on the network signatures to provide
		// the randomness so that it is the same on all machines.
		if ( Game_mode & GM_MULTIPLAYER ){
			static_rand_cone(objp->net_signature + i, &tvec,&objp->orient.vec.fvec,wip->spawn_angle,&objp->orient);
		} else {
			vm_vec_random_cone(&tvec,&objp->orient.vec.fvec,wip->spawn_angle,&objp->orient);
		}
		vm_vec_scale_add(&pos, &objp->pos, &tvec, objp->radius);

		vm_vector_2_matrix(&orient, &tvec, NULL, NULL);
		weapon_objnum = weapon_create(&pos, &orient, child_id, parent_num, 1, -1, wp->weapon_flags & WF_LOCKED_WHEN_FIRED);

		//	Assign a little randomness to lifeleft so they don't all disappear at the same time.
		if (weapon_objnum != -1) {
			float rand_val;

			if ( Game_mode & GM_NORMAL ){
				rand_val = frand();
			} else {
				rand_val = static_randf(objp->net_signature + i);
			}

			Weapons[Objects[weapon_objnum].instance].lifeleft *= rand_val*0.4f + 0.8f;
		}

	}

#ifndef NO_NETWORK
	// in multiplayer, reset the next network signature to the one that was saved.
	if ( Game_mode & GM_MULTIPLAYER ){
		multi_set_network_signature( starting_sig, MULTI_SIG_NON_PERMANENT );
	}
#endif
}

// -----------------------------------------------------------------------
// weapon_hit_do_sound()
//
// Play a sound effect when a weapon hits a ship
//
// To elimate the "stereo" effect of two lasers hitting at nearly
// the same time, and to reduce the number of sound channels used,
// only play one impact sound if IMPACT_SOUND_DELTA has elapsed
//
// Note: Uses Weapon_impact_timer global for timer variable
//
void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vec3d *hitpos)
{
	int	is_hull_hit;
	float shield_str;

	// If non-missiles (namely lasers) expire without hitting a ship, don't play impact sound
	if	( wip->subtype != WP_MISSILE ) {		
		if ( !hit_obj ) {
			// flak weapons make sounds		
			if(wip->wi_flags & WIF_FLAK){
				snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );				
			}
			return;
		}

		switch(hit_obj->type) {
		case OBJ_SHIP:
			// do nothing
			break;

		case OBJ_ASTEROID:
			if ( timestamp_elapsed(Weapon_impact_timer) ) {
				snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );
				Weapon_impact_timer = timestamp(IMPACT_SOUND_DELTA);
			}
			return;
			break;

		default:
			return;
		}
	}

	if ( hit_obj == NULL ) {
		snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );
		return;
	}

	if ( timestamp_elapsed(Weapon_impact_timer) ) {

		is_hull_hit = 1;
		if ( hit_obj->type == OBJ_SHIP ) {
			shield_str = ship_quadrant_shield_strength(hit_obj, hitpos);
		} else {
			shield_str = 0.0f;
		}

		// play a shield hit if shields are above 10% max in this quadrant
		if ( shield_str > 0.1f ) {
			is_hull_hit = 0;
		}

		if ( !is_hull_hit ) {
			// Play a shield impact sound effect
			if ( hit_obj == Player_obj ) {
				snd_play_3d( &Snds[SND_SHIELD_HIT_YOU], hitpos, &Eye_position );
				// AL 12-15-97: Add missile impact sound even when shield is hit
				if ( wip->subtype == WP_MISSILE ) {
					snd_play_3d( &Snds[SND_PLAYER_HIT_MISSILE], hitpos, &Eye_position);
				}
			} else {
				snd_play_3d( &Snds[SND_SHIELD_HIT], hitpos, &Eye_position );
			}
		} else {
			// Play a hull impact sound effect
			switch ( wip->subtype ) {
				case WP_LASER:
					if ( hit_obj == Player_obj )
						snd_play_3d( &Snds[SND_PLAYER_HIT_LASER], hitpos, &Eye_position );
					else {
						if ( wip->impact_snd != -1 ) {
							snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );
						}
					}
					break;
				case WP_MISSILE:
					if ( hit_obj == Player_obj ) 
						snd_play_3d( &Snds[SND_PLAYER_HIT_MISSILE], hitpos, &Eye_position);
					else {
						if ( wip->impact_snd != -1 ) {
							snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );
						}
					}
					break;
				default:	
					nprintf(("Warning","WARNING ==> Cannot determine sound to play for weapon impact\n"));
					break;
			} // end switch
		}

		Weapon_impact_timer = timestamp(IMPACT_SOUND_DELTA);
	}
}

const float weapon_electronics_scale[MAX_SHIP_TYPES]=
{
	0.0f,	//SHIP_TYPE_NONE
	10.0f,	//SHIP_TYPE_CARGO
	4.0f,	//SHIP_TYPE_FIGHTER_BOMBER
	0.9f,	//SHIP_TYPE_CRUISER
	1.75f,	//SHIP_TYPE_FREIGHTER
	0.2f,	//SHIP_TYPE_CAPITAL
	2.0f,	//SHIP_TYPE_TRANSPORT
	3.5f,	//SHIP_TYPE_REPAIR_REARM
	10.0f,	//SHIP_TYPE_NAVBUOY
	10.0f,	//SHIP_TYPE_SENTRYGUN
	10.0f,	//SHIP_TYPE_ESCAPEPOD
	0.075f,	//SHIP_TYPE_SUPERCAP
	4.0f,	//SHIP_TYPE_STEALTH
	4.0f,	//SHIP_TYPE_FIGHTER
	4.0f,	//SHIP_TYPE_BOMBER
	0.5f,	//SHIP_TYPE_DRYDOCK
	0.8f,	//SHIP_TYPE_AWACS
	1.0f,	//SHIP_TYPE_GAS_MINER
	0.3333f,	//SHIP_TYPE_CORVETTE
	0.10f,	//SHIP_TYPE_KNOSSOS_DEVICE
};

extern bool turret_weapon_has_flags(ship_weapon *swp, int flags);

// distrupt any subsystems that fall into damage sphere of this Electronics missile
//
// input:	ship_obj		=>		pointer to ship that holds subsystem
//				blast_pos	=>		world pos of weapon blast
//				wi_index		=>		weapon info index of weapon causing blast
void weapon_do_electronics_affect(object *ship_objp, vec3d *blast_pos, int wi_index)
{
	weapon_info			*wip;
	ship					*shipp;
	ship_subsys			*ss;
	model_subsystem	*psub;
	vec3d				subsys_world_pos;
	float					dist;

	shipp = &Ships[ship_objp->instance];
	wip = &Weapon_info[wi_index];

	int ship_type=ship_query_general_type(shipp);
	float base_time=((float)wip->elec_time*(weapon_electronics_scale[ship_type]*wip->elec_intensity));
	float sub_time;

	for ( ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss) )
	{
		psub = ss->system_info;
		sub_time=base_time;

		// convert subsys point to world coords
		vm_vec_unrotate(&subsys_world_pos, &psub->pnt, &ship_objp->orient);
		vm_vec_add2(&subsys_world_pos, &ship_objp->pos);

		// see if subsys point is within damage sphere
		dist = vm_vec_dist_quick(blast_pos, &subsys_world_pos);
		if ( dist < wip->outer_radius )
		{
			//use new style electronics disruption
			if (wip->elec_use_new_style)
			{
				//if its an engine subsytem, take the multiplier into account
				if (psub->type==SUBSYSTEM_ENGINE)
				{
					sub_time*=wip->elec_eng_mult;
				}
	
				//if its a turret or weapon subsytem, take the multiplier into account
				if ((psub->type==SUBSYSTEM_TURRET) || (psub->type==SUBSYSTEM_WEAPONS))
				{
					//disrupt beams
					//WMC - do this even if there are other types of weapons on the turret.
					//I figure, the big fancy electronics on beams will be used for the other
					//weapons as well. No reason having two targeting computers on a turret.
					//Plus, it's easy and fast to code. :)
					if ((psub->type==SUBSYSTEM_TURRET)&& turret_weapon_has_flags(&ss->weapons, WIF_BEAM))
					{
						sub_time*=wip->elec_beam_mult;
					}
					//disrupt other weapons
					else
					{
						sub_time*=wip->elec_weap_mult;
					}
				}
				
				//disrupt sensor and awacs systems.
				if ((psub->type==SUBSYSTEM_SENSORS) || (psub->flags & MSS_FLAG_AWACS))
				{
					sub_time*=wip->elec_sensors_mult;
				}
	
				//add a little randomness to the disruption time, unless the disuruption time is zero for some reason
				//perhaps a multiplier was zero or the scale was too small.
				if (sub_time > 0) 
				{
					sub_time+=frand_range(-1.0f, 1.0f) * wip->elec_randomness;
				}
		
				//disrupt this subsystem for the calculated time, plus or minus some time
				//if it turns out to be less than 0 seconds, don't bother
				if (sub_time > 0)
				{
					ship_subsys_set_disrupted(ss, fl2i(sub_time));
				}
			}

			//use the old style disruption effect
			else 
			{
				sub_time=wip->elec_time + frand_range(-1.0f, 1.0f)*wip->elec_randomness;
				
				//disrupt this subsystem for the calculated time, plus or minus some time
				//if it turns out to be less than 0 seconds, don't bother
				if (sub_time > 0)
				{
					ship_subsys_set_disrupted(ss, fl2i(sub_time));
				}
			}
		}
	}
}

//	----------------------------------------------------------------------
//	weapon_area_calc_damage()
//
// Calculate teh damage for an object based on the location of an area-effect
// explosion.
//
// input:		objp			=>		object pointer ship receiving blast effect
//					pos			=>		world pos of blast center
//					inner_rad	=>		smallest radius at which full damage is done
//					outer_rad	=>		radius at which no damage is done
//					max_blast	=>		maximum blast possible from explosion
//					max_damage	=>		maximum damage possible from explosion
//					blast			=>		OUTPUT PARAMETER: receives blast value from explosion
//					damage		=>		OUTPUT PARAMETER: receives damage value from explosion
//					limit			=>		a limit on the area, needed for shockwave damage
//
//	returns:		no damage occurred	=>		-1
//					damage occured			=>		0
//
int weapon_area_calc_damage(object *objp, vec3d *pos, float inner_rad, float outer_rad, float max_blast, float max_damage, float *blast, float *damage, float limit)
{
	float			dist, max_dist, min_dist;

 	// only blast ships and asteroids
	if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID)) {
		return -1;
	}

	max_dist = objp->radius + outer_rad;
	dist = vm_vec_dist_quick(&objp->pos, pos);	
	if ( (dist > max_dist) || (dist > (limit+objp->radius)) ) {
		return -1;	// spheres don't intersect at all
	}

	if ( dist < (inner_rad+objp->radius) ) {
		// damage is maximum within inner radius
		*damage = max_damage;
		*blast = max_blast;
	} else {
		float dist_to_outer_rad_squared, total_dist_squared;
		min_dist = dist - objp->radius;
		Assert(min_dist < outer_rad);
		dist_to_outer_rad_squared = (outer_rad-min_dist)*(outer_rad-min_dist);
		total_dist_squared = (inner_rad-outer_rad)*(inner_rad-outer_rad);
		// AL 2-24-98: drop off damage relative to square of distance
		Assert(dist_to_outer_rad_squared <= total_dist_squared);
		*damage = max_damage * dist_to_outer_rad_squared/total_dist_squared;


//		*damage = (min_dist - outer_rad) * max_damage/(inner_rad - outer_rad);
		*blast =  (min_dist - outer_rad) * max_blast /(inner_rad - outer_rad);
	}

	// nprintf(("AI", "Frame %i: Damage = %7.3f, %7.3f meters away.\n", Framecount, *damage, dist));

	return 0;
}

//	----------------------------------------------------------------------
//	weapon_area_apply_blast()
//
// Apply the blast effects of an explosion to a ship
//
// input:	force_apply_pos	=>		world pos of where force is applied to object
//				ship_obj				=>		object pointer of ship receiving the blast
//				blast_pos			=>		world pos of blast center
//				blast					=>		force of blast
//				make_shockwave		=>		boolean, whether to create a shockwave or not
//
void weapon_area_apply_blast(vec3d *force_apply_pos, object *ship_obj, vec3d *blast_pos, float blast, int make_shockwave)
{
	#define	SHAKE_CONST 3000
	vec3d		force, vec_blast_to_ship, vec_ship_to_impact;
	polymodel		*pm;

	// apply blast force based on distance from center of explosion
	vm_vec_sub(&vec_blast_to_ship, &ship_obj->pos, blast_pos);
	vm_vec_normalize_safe(&vec_blast_to_ship);
	vm_vec_copy_scale(&force, &vec_blast_to_ship, blast );


	vm_vec_sub(&vec_ship_to_impact, blast_pos, &ship_obj->pos);

	pm = model_get(Ships[ship_obj->instance].modelnum);
	Assert ( pm != NULL );

	if (make_shockwave) {
		physics_apply_shock (&force, blast, &ship_obj->phys_info, &ship_obj->orient, &pm->mins, &pm->maxs, pm->rad);
		if (ship_obj == Player_obj) {
			joy_ff_play_vector_effect(&vec_blast_to_ship, blast * 2.0f);
		}
	} else {
		ship_apply_whack( &force, &vec_ship_to_impact, ship_obj);
	}
}

//	----------------------------------------------------------------------
//	weapon_do_area_effect()
//
// Do the area effect for a weapon
//
// input:	wobjp			=>		object pointer to weapon causing explosion
//				pos			=>		world pos of explosion center
//				other_obj	=>		object pointer to ship that weapon impacted on (can be NULL)
//
void weapon_do_area_effect(object *wobjp, vec3d *pos, object *other_obj)
{
	weapon_info	*wip;
	weapon *wp;
	object		*objp;
	float			damage, blast;

	wip = &Weapon_info[Weapons[wobjp->instance].weapon_info_index];	
	wp = &Weapons[wobjp->instance];
	Assert(wip->inner_radius != 0);	

	// only blast ships and asteroids
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) ) {
			continue;
		}
	
		if ( objp->type == OBJ_SHIP ) {
			// don't blast navbuoys
			if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
				continue;
			}
		}

		if ( weapon_area_calc_damage(objp, pos, wip->inner_radius, wip->outer_radius, wip->blast_force, wip->damage, &blast, &damage, wip->outer_radius) == -1 ){
			continue;
		}

		// scale damage
		damage *= weapon_get_damage_scale(wip, wobjp, other_obj);		

		switch ( objp->type ) {
		case OBJ_SHIP:
			ship_apply_global_damage(objp, wobjp, pos, damage);
			weapon_area_apply_blast(NULL, objp, pos, blast, 0);
			break;
		case OBJ_ASTEROID:
			asteroid_hit(objp, NULL, NULL, damage);
			break;
		default:
			Int3();
			break;
		} 	

	}	// end for

	// if this weapon has the "Electronics" flag set, then disrupt subsystems in sphere
//	if ( (other_obj != NULL) && (wip->wi_flags & WIF_ELECTRONICS) ) {
//		if ( other_obj->type == OBJ_SHIP ) {
//			weapon_do_electronics_affect(other_obj, pos, Weapons[wobjp->instance].weapon_info_index);
//		}
//	}
}


//	----------------------------------------------------------------------
//	weapon_hit()
//
// This function is called when a weapon hits something (or, in the case of
// missiles explodes for any particular reason)
//
void weapon_hit( object * weapon_obj, object * other_obj, vec3d * hitpos )
{
	Assert(weapon_obj != NULL);
	if(weapon_obj == NULL){
		return;
	}
	Assert((weapon_obj->type == OBJ_WEAPON) && (weapon_obj->instance >= 0) && (weapon_obj->instance < MAX_WEAPONS));
	if((weapon_obj->type != OBJ_WEAPON) || (weapon_obj->instance < 0) || (weapon_obj->instance >= MAX_WEAPONS)){
		return;
	}

	int			num = weapon_obj->instance;
	int			weapon_type = Weapons[num].weapon_info_index;
	object		*weapon_parent_objp;
	weapon_info	*wip;
	// int np_index;

	Assert((weapon_type >= 0) && (weapon_type < MAX_WEAPONS));
	if((weapon_type < 0) || (weapon_type >= MAX_WEAPONS)){
		return;
	}
	wip = &Weapon_info[weapon_type];
	weapon_parent_objp = &Objects[weapon_obj->parent];

	// if this is the player ship, and is a laser hit, skip it. wait for player "pain" to take care of it
	// if( ((wip->subtype != WP_LASER) || !MULTIPLAYER_CLIENT) && (Player_obj != NULL) && (other_obj == Player_obj) ){
#ifndef NO_NETWORK
	if ((other_obj != Player_obj) || (wip->subtype != WP_LASER) || !MULTIPLAYER_CLIENT)
#endif
	{
		weapon_hit_do_sound(other_obj, wip, hitpos);
	}

	if ( wip->impact_weapon_expl_index > -1 )	{
		int expl_ani_handle = weapon_get_expl_handle(wip->impact_weapon_expl_index, hitpos, wip->impact_explosion_radius);
		particle_create( hitpos, &vmd_zero_vector, 0.0f, wip->impact_explosion_radius, PARTICLE_BITMAP_PERSISTENT, expl_ani_handle );
	}

	weapon_obj->flags |= OF_SHOULD_BE_DEAD;

	int sw_flag = SW_WEAPON;

	// check if this is an area effect weapon
	if ( wip->wi_flags & WIF_AREA_EFFECT ) {
	// if ( wip->subtype & WP_MISSILE && wip->wi_flags & WIF_AREA_EFFECT ) {
		if ( wip->wi_flags & WIF_SHOCKWAVE ) {
			float actual_damage = wip->damage;
			// Shockwaves caused by weapons hitting weapons are 1/4 as powerful
			if ( ((other_obj) && (other_obj->type == OBJ_WEAPON)) || (Weapons[num].weapon_flags & WF_DESTROYED_BY_WEAPON)) {
				actual_damage /= 4.0f;
				sw_flag |= SW_WEAPON_KILL;
			}
			shockwave_create_info sci;
			sci.blast = wip->blast_force;
			sci.damage = actual_damage;
			sci.inner_rad = wip->inner_radius;
			sci.outer_rad = wip->outer_radius;
			sci.speed = wip->shockwave_speed;
			sci.rot_angle = 0.0f;

			shockwave_create(OBJ_INDEX(weapon_obj), hitpos, &sci, sw_flag, -1, wip->shockwave_model);
//			snd_play_3d( &Snds[SND_SHOCKWAVE_IMPACT], hitpos, &Eye_position );
		}
		else {
			weapon_do_area_effect(weapon_obj, hitpos, other_obj);
		}
	}

	if (wip->wi_flags & WIF_ELECTRONICS)
	{
		float blast,damage;
		for ( object *objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
		{
			if (objp->type != OBJ_SHIP)
			{
				continue;
			}
			if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY )
			{
				continue;
			}
			if ( weapon_area_calc_damage(objp, hitpos, wip->inner_radius, wip->outer_radius, wip->blast_force, wip->damage, &blast, &damage, wip->outer_radius) == -1 ){
				continue;
			}

			weapon_do_electronics_affect(objp, hitpos, Weapons[weapon_obj->instance].weapon_info_index);
		}
	}

	// check if this is an EMP weapon
	if(wip->wi_flags & WIF_EMP){
		emp_apply(&weapon_obj->pos, wip->inner_radius, wip->outer_radius, wip->emp_intensity, wip->emp_time);
	}	

	// spawn weapons - note the change from FS 1 multiplayer.
	if (wip->wi_flags & WIF_SPAWN){
		spawn_child_weapons(weapon_obj);
	}	
}

void weapon_detonate(object *objp)
{
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert((objp->type == OBJ_WEAPON) && (objp->instance >= 0));
	if((objp->type != OBJ_WEAPON) || (objp->instance < 0)){
		return;
	}	

#ifndef NO_NETWORK
	// send a detonate packet in multiplayer
	if(MULTIPLAYER_MASTER){
		send_weapon_detonate_packet(objp);
	}
#endif

	// call weapon hit
	weapon_hit(objp, NULL, &objp->pos);
}

//	Return the Weapon_info[] index of the weapon with name *name.
int weapon_name_lookup(char *name)
{
	int	i;

	for ( i=0; i < Num_weapon_types; i++) {
		if (!stricmp(name, Weapon_info[i].name)) {
			return i;
		}
	}

	return -1;
}

// Group_id:  If you should quad lasers, they should all have the same group id.  
// This will be used to optimize lighting, since each group only needs to cast one light.
// Call this to get a new group id, then pass it to each weapon_create call for all the
// weapons in the group.   Number will be between 0 and WEAPON_MAX_GROUP_IDS and will
// get reused.
int weapon_create_group_id()
{
	static int current_id = 0;

	int n = current_id;
	
	current_id++;
	if ( current_id >= WEAPON_MAX_GROUP_IDS )	{
		current_id = 0;
	}
	return n;
}

//Call before weapons_page_in to mark a weapon as used
void weapon_mark_as_used(int weapon_id)
{
	if (weapon_id < 0)
		return;

	if ( used_weapons == NULL )
		return;

	Assert( weapon_id < MAX_WEAPON_TYPES );

	if (weapon_id < Num_weapon_types) {
		used_weapons[weapon_id]++;
	}
}

void weapons_page_in()
{
	int i, j, idx;

	if (Cmdline_load_only_used) {
		Assert( used_weapons != NULL );

		// for weapons in weaponry pool
		for (i = 0; i < Num_teams; i++) {
			for (j = 0; j < Num_weapon_types; j++) {
				used_weapons[j] += Team_data[i].weaponry_pool[j];
			}
		}

		// this grabs all spawn weapon types (Cluster Baby, etc.) which can't be
		// assigned directly to a ship
		for (i = 0; i < Num_weapon_types; i++) {
			// we only want entries that already exist
			if (!used_weapons[i])
				continue;

			// if it's got a spawn type then grab it
			if (Weapon_info[i].spawn_type > -1)
				used_weapons[(int)Weapon_info[i].spawn_type]++;
		}
	}

	// Page in bitmaps for all used weapons
	for (i=0; i<Num_weapon_types; i++ )	{
		if (Cmdline_load_only_used) {
			if (!used_weapons[i]) {
				nprintf(("Weapons", "Not loading weapon id %d (%s)\n", i, Weapon_info[i].name));
				continue;
			}
		}

		weapon_info *wip = &Weapon_info[i];

		wip->wi_flags &= (~WIF_THRUSTER);		// Assume no thrusters
		
		switch( wip->render_type )	{
			case WRT_POF:
				{
					wip->model_num = model_load( wip->pofbitmap_name, 0, NULL );

					polymodel *pm = model_get( wip->model_num );

					// If it has a model, and the model pof has thrusters, then set
					// the flags
					if ( pm->n_thrusters > 0 )	{
						//mprintf(( "Weapon %s has thrusters!\n", wip->name ));
						wip->wi_flags |= WIF_THRUSTER;
					}
		
					for (j=0; j<pm->n_textures; j++ )	{
						int bitmap_num = pm->original_textures[j];

						if ( bitmap_num > -1 )	{
							bm_page_in_texture( bitmap_num );
						}
					}
				}
				break;

			case WRT_LASER:
				{
					bm_page_in_texture( wip->laser_bitmap );

					if(wip->laser_glow_bitmap >= 0){
						bm_page_in_texture(wip->laser_glow_bitmap);
					}
				}
				break;

			default:
				Int3();	// Invalid weapon rendering type.
		}

		wip->external_model_num = -1;
		if(strcmp(wip->external_model_name,""))wip->external_model_num = model_load( wip->external_model_name, 0, NULL );
		if(wip->external_model_num == -1)wip->external_model_num = wip->model_num;
		// If this has an impact vclip page it in.
//		if ( wip->impact_explosion_ani > -1 )	{
//			int nframes, fps;
//			bm_get_info( wip->impact_explosion_ani, NULL, NULL, NULL, &nframes, &fps );
//			bm_page_in_xparent_texture( wip->impact_explosion_ani, nframes );
//		}


		if(strcmp(wip->shockwave_pof_name,""))
		wip->shockwave_model = model_load(wip->shockwave_pof_name, 0, NULL);
		else wip->shockwave_model = -1;

		// trail bitmaps
		if ( (wip->wi_flags & WIF_TRAIL) && (wip->tr_info.bitmap > -1) )	{
			bm_page_in_texture( wip->tr_info.bitmap );
		}

		// if this is a beam weapon, page in its stuff
		if(wip->wi_flags & WIF_BEAM){
			// all beam sections
			for(idx=0; idx<wip->b_info.beam_num_sections; idx++){
				if((idx < MAX_BEAM_SECTIONS) && (wip->b_info.sections[idx].texture >= 0)){
					bm_page_in_texture(wip->b_info.sections[idx].texture, wip->b_info.sections[idx].nframes);
				}
			}

			// muzzle glow
			if(wip->b_info.beam_glow_bitmap >= 0){
				bm_page_in_texture(wip->b_info.beam_glow_bitmap);
			}

			// particle ani
			if(wip->b_info.beam_particle_ani >= 0){
				int nframes, fps;
				bm_get_info( wip->b_info.beam_particle_ani, NULL, NULL, NULL, &nframes, &fps );
				bm_page_in_texture( wip->b_info.beam_particle_ani, nframes );
			}
		}
	
		if(wip->wi_flags & WIF_PARTICLE_SPEW){
			if(wip->Weapon_particle_spew_nframes > 1){
				bm_page_in_texture(wip->Weapon_particle_spew_bitmap, wip->Weapon_particle_spew_nframes);//page in the bitmap-Bobboau
			}else{
				bm_page_in_texture(wip->Weapon_particle_spew_bitmap);//page in the bitmap-Bobboau
			}
		}
	//page in decal textures
		if(wip->decal_texture != -1){
			bm_page_in_xparent_texture( wip->decal_texture, 1);
			if(wip->decal_backface_texture != -1){
				bm_page_in_xparent_texture( wip->decal_backface_texture);
			}

		}

	}

	// explosion ani's
	for (i=0; i<Num_weapon_expl; i++) {
		int bitmap_handle, nframes, fps;

		for (j=0; j<Weapon_expl_info[i].lod_count; j++) {
			//load ani
			bitmap_handle = bm_load_animation(Weapon_expl_info[i].lod[j].filename, &nframes, &fps, 1);
			Weapon_expl_info[i].lod[j].bitmap_id = bitmap_handle;
			Weapon_expl_info[i].lod[j].fps = fps;
			Weapon_expl_info[i].lod[j].num_frames = nframes;

			// page it in
			bm_page_in_xparent_texture(bitmap_handle, nframes);
		}
	}

	// Counter measures
	for (i=0; i<Num_cmeasure_types; i++ )	{
		cmeasure_info *cmeasurep;

		cmeasurep = &Cmeasure_info[i];
	
		cmeasurep->model_num = model_load( cmeasurep->pof_name, 0, NULL );

		polymodel *pm = model_get( cmeasurep->model_num );

		for (j=0; j<pm->n_textures; j++ )	{
			int bitmap_num = pm->original_textures[j];

			if ( bitmap_num > -1 )	{
				bm_page_in_texture( bitmap_num );
			}
		}
		Assert( cmeasurep->model_num > -1 );
	}


}

// call to get the "color" of the laser at the given moment (since glowing lasers can cycle colors)
void weapon_get_laser_color(color *c, object *objp)
{
	weapon *wep;
	weapon_info *winfo;
	float pct;

	// sanity
	if(c == NULL){
		return;
	}

	// sanity
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);
	Assert(Weapons[objp->instance].weapon_info_index >= 0);
	if((objp->type != OBJ_WEAPON) || (objp->instance < 0) || (Weapons[objp->instance].weapon_info_index < 0)){
		return;
	}
	wep = &Weapons[objp->instance];
	winfo = &Weapon_info[wep->weapon_info_index];

	// if we're a one-color laser
	if((winfo->laser_color_2.red == 0) && (winfo->laser_color_2.green == 0) && (winfo->laser_color_2.blue == 0)){
		*c = winfo->laser_color_1;
	}

	// lifetime pct
	pct = 1.0f - (wep->lifeleft / winfo->lifetime);
	if(pct > 0.5f){
		pct = 0.5f;
	} else if (pct < 0.0f)
		pct = 0.0f;

	pct *= 2.0f;
	
	// otherwise interpolate between the colors
	gr_init_color( c, (int)((float)winfo->laser_color_1.red + (((float)winfo->laser_color_2.red - (float)winfo->laser_color_1.red) * pct)), 
							(int)((float)winfo->laser_color_1.green + (((float)winfo->laser_color_2.green - (float)winfo->laser_color_1.green) * pct)), 
							(int)((float)winfo->laser_color_1.blue + (((float)winfo->laser_color_2.blue - (float)winfo->laser_color_1.blue) * pct)) );
}

// default weapon particle spew data

int Weapon_particle_spew_count = 1;
int Weapon_particle_spew_time = 25;
float Weapon_particle_spew_vel = 0.4f;
float Weapon_particle_spew_radius = 2.0f;
float Weapon_particle_spew_lifetime = 0.15f;
float Weapon_particle_spew_scale = 0.8f;

// for weapons flagged as particle spewers, spew particles. wheee
void weapon_maybe_spew_particle(object *obj)
{
	weapon *wp;
	weapon_info *wip;
	int idx;
	vec3d direct, direct_temp, particle_pos;
	vec3d null_vec = ZERO_VECTOR;
	vec3d vel;
	float ang;

	// check some stuff
	Assert(obj->type == OBJ_WEAPON);
	Assert(obj->instance >= 0);
	Assert(Weapons[obj->instance].weapon_info_index >= 0);
	Assert(Weapon_info[Weapons[obj->instance].weapon_info_index].wi_flags & WIF_PARTICLE_SPEW);
	
	wp = &Weapons[obj->instance];	
	wip = &Weapon_info[wp->weapon_info_index];
	// if the weapon's particle timestamp has elapse`d
	if((wp->particle_spew_time == -1) || timestamp_elapsed(wp->particle_spew_time)){
		// reset the timestamp
		wp->particle_spew_time = timestamp(wip->Weapon_particle_spew_time);

		// spew some particles
		for(idx=0; idx<wip->Weapon_particle_spew_count; idx++){
			// get the backward vector of the weapon
			direct = obj->orient.vec.fvec;
			vm_vec_negate(&direct);

			//	randomly perturb x, y and z
			
			// uvec
			ang = fl_radian(frand_range(-90.0f, 90.0f));
			vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.fvec);			
			direct = direct_temp;
			vm_vec_scale(&direct, wip->Weapon_particle_spew_scale);

			// rvec
			ang = fl_radian(frand_range(-90.0f, 90.0f));
			vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.rvec);			
			direct = direct_temp;
			vm_vec_scale(&direct, wip->Weapon_particle_spew_scale);

			// fvec
			ang = fl_radian(frand_range(-90.0f, 90.0f));
			vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.uvec);			
			direct = direct_temp;
			vm_vec_scale(&direct, wip->Weapon_particle_spew_scale);

			// get a velovity vector of some percentage of the weapon's velocity
			vel = obj->phys_info.vel;
			vm_vec_scale(&vel, wip->Weapon_particle_spew_vel);

			// emit the particle
			vm_vec_add(&particle_pos, &obj->pos, &direct);

			if(wip->Weapon_particle_spew_bitmap < 0){
				particle_create(&particle_pos, &vel, wip->Weapon_particle_spew_lifetime, wip->Weapon_particle_spew_radius, PARTICLE_SMOKE);
			}else{
				particle_create(&particle_pos, &vel, wip->Weapon_particle_spew_lifetime, wip->Weapon_particle_spew_radius, PARTICLE_BITMAP, wip->Weapon_particle_spew_bitmap);
			}
		}
	}
}

// debug console functionality
void pspew_display_dcf()
{
	dc_printf("Particle spew settings\n\n");
	dc_printf("Particle spew count (pspew_count) : %d\n", Weapon_particle_spew_count);
	dc_printf("Particle spew time (pspew_time) : %d\n", Weapon_particle_spew_time);
	dc_printf("Particle spew velocity (pspew_vel) : %f\n", Weapon_particle_spew_vel);
	dc_printf("Particle spew size (pspew_size) : %f\n", Weapon_particle_spew_radius);
	dc_printf("Particle spew lifetime (pspew_life) : %f\n", Weapon_particle_spew_lifetime);
	dc_printf("Particle spew scale (psnew_scale) : %f\n", Weapon_particle_spew_scale);
}

DCF(pspew_count, "Number of particles spewed at a time")
{	
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		Weapon_particle_spew_count = Dc_arg_int;
	}

	pspew_display_dcf();
}

DCF(pspew_time, "Time between particle spews")
{	
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		Weapon_particle_spew_time = Dc_arg_int;
	}

	pspew_display_dcf();
}

DCF(pspew_vel, "Relative velocity of particles (0.0 - 1.0)")
{	
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Weapon_particle_spew_vel = Dc_arg_float;
	}

	pspew_display_dcf();
}

DCF(pspew_size, "Size of spewed particles")
{	
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Weapon_particle_spew_radius = Dc_arg_float;
	}

	pspew_display_dcf();
}

DCF(pspew_life, "Lifetime of spewed particles")
{	
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Weapon_particle_spew_lifetime = Dc_arg_float;
	}

	pspew_display_dcf();
}

DCF(pspew_scale, "How far away particles are from the weapon path")
{	
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Weapon_particle_spew_scale = Dc_arg_float;
	}

	pspew_display_dcf();
}

// return a scale factor for damage which should be applied for 2 collisions
float weapon_get_damage_scale(weapon_info *wip, object *wep, object *target)
{
	weapon *wp;	
	int from_player = 0;
	float total_scale = 1.0f;
	float hull_pct;
	int is_big_damage_ship = 0;

	// Goober5000 - additional sanity (target can be NULL)
	Assert(wip);
	Assert(wep);

	// sanity
	if((wip == NULL) || (wep == NULL) || (target == NULL)){
		return 1.0f;
	}

	// don't scale any damage if its not a weapon	
	if((wep->type != OBJ_WEAPON) || (wep->instance < 0) || (wep->instance >= MAX_WEAPONS)){
		return 1.0f;
	}
	wp = &Weapons[wep->instance];

	// was the weapon fired by the player
	from_player = 0;
	if((wep->parent >= 0) && (wep->parent < MAX_OBJECTS) && (Objects[wep->parent].flags & OF_PLAYER_SHIP)){
		from_player = 1;
	}
		
	// if this is a lockarm weapon, and it was fired unlocked
	if((wip->wi_flags & WIF_LOCKARM) && !(wp->weapon_flags & WF_LOCKED_WHEN_FIRED)){		
		total_scale *= 0.1f;
	}
	
	// if the hit object was a ship
	if(target->type == OBJ_SHIP){
		ship *shipp;
		ship_info *sip;

		// get some info on the ship
		Assert((target->instance >= 0) && (target->instance < MAX_SHIPS));
		if((target->instance < 0) || (target->instance >= MAX_SHIPS)){
			return total_scale;
		}
		shipp = &Ships[target->instance];
		sip = &Ship_info[Ships[target->instance].ship_info_index];

		// get hull pct of the ship currently
		hull_pct = get_hull_pct(target);

		// if it has hit a supercap ship and is not a supercap class weapon
		if((sip->flags & SIF_SUPERCAP) && !(wip->wi_flags & WIF_SUPERCAP)){
			// Goober5000 - now weapons scale universally
			total_scale *= hull_pct * SUPERCAP_DAMAGE_SCALE;

			/*// if the supercap is around 3/4 damage, apply nothing
			if(hull_pct <= 0.75f){
				return 0.0f;
			} else {
				total_scale *= SUPERCAP_DAMAGE_SCALE;
			}*/
		}

		// determine if this is a big damage ship
		is_big_damage_ship = (sip->flags & SIF_BIG_DAMAGE);

		// if this is a large ship, and is being hit by flak
		if(is_big_damage_ship && (wip->wi_flags & WIF_FLAK)){
			total_scale *= FLAK_DAMAGE_SCALE;
		}
		
		/* Goober5000 - commented this, since it's kinda redundant with the next thingy
		// if the player is firing small weapons at a big ship
		if( from_player && is_big_damage_ship && !(wip->wi_flags & (WIF_HURTS_BIG_SHIPS)) )
		{
			// if its a laser weapon
			if(wip->subtype == WP_LASER){
				total_scale *= 0.01f;
			} else {
				total_scale *= 0.05f;
			}
		}*/

		// if the weapon is a small weapon being fired at a big ship
		if( is_big_damage_ship && !(wip->wi_flags & (WIF_HURTS_BIG_SHIPS)) ){
			// Goober5000 - now weapons scale universally
			total_scale *= hull_pct * CAPITAL_DAMAGE_SCALE;
			/*if(hull_pct > 0.1f){
				total_scale *= hull_pct;
			} else {
				return 0.0f;
			}*/
		}
	}
	
	return total_scale;
}

int weapon_get_expl_handle(int weapon_expl_index, vec3d *pos, float size)
{
	weapon_expl_info *wei = &Weapon_expl_info[weapon_expl_index];

	if (wei->lod_count == 1) {
		return wei->lod[0].bitmap_id;
	}

	// now we have to do some work
	vertex v;
	int x, y, w, h, bm_size;
	int must_stop = 0;
	int best_lod = 1;
	int behind = 0;

	// start the frame
	extern float Viewer_zoom;
	extern int G3_count;

	if(!G3_count){
		g3_start_frame(1);
		must_stop = 1;
	}
	g3_set_view_matrix(&Eye_position, &Eye_matrix, Viewer_zoom);

	// get extents of the rotated bitmap
	g3_rotate_vertex(&v, pos);

	// if vertex is behind, find size if in front, then drop down 1 LOD
	if (v.codes & CC_BEHIND) {
		float dist = vm_vec_dist_quick(&Eye_position, pos);
		vec3d temp;

		behind = 1;
		vm_vec_scale_add(&temp, &Eye_position, &Eye_matrix.vec.fvec, dist);
		g3_rotate_vertex(&v, &temp);

		// if still behind, bail and go with default
		if (v.codes & CC_BEHIND) {
			behind = 0;
		}
	}

	if (!g3_get_bitmap_dims(wei->lod[0].bitmap_id, &v, size, &x, &y, &w, &h, &bm_size)) {
		if (Detail.hardware_textures == 4) {
			// straight LOD
			if(w <= bm_size/8){
				best_lod = 3;
			} else if(w <= bm_size/2){
				best_lod = 2;
			} else if(w <= 1.3f*bm_size){
				best_lod = 1;
			} else {
				best_lod = 0;
			}
		} else {
			// less aggressive LOD for lower detail settings
			if(w <= bm_size/8){
				best_lod = 3;
			} else if(w <= bm_size/3){
				best_lod = 2;
			} else if(w <= (1.15f*bm_size)){
				best_lod = 1;
			} else {
				best_lod = 0;
			}		
		}
	}

	// if it's behind, bump up LOD by 1
	if (behind) {
		best_lod++;
	}

	// end the frame
	if(must_stop){
		g3_end_frame();
	}

	best_lod = MIN(best_lod, wei->lod_count - 1);
	return wei->lod[best_lod].bitmap_id;
}

void weapons_info_close(){
	for(int i = 0; i<MAX_WEAPON_TYPES; i++){
		if(Weapon_info[i].desc)free(Weapon_info[i].desc);
		if(Weapon_info[i].tech_desc)free(Weapon_info[i].tech_desc);
	}

	if (used_weapons != NULL) {
		delete[] used_weapons;
		used_weapons = NULL;
	}
}
