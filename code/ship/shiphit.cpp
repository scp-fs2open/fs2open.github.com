/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipHit.cpp $
 * $Revision: 2.23 $
 * $Date: 2003-11-11 02:15:41 $
 * $Author: Goober5000 $
 *
 * Code to deal with a ship getting hit by something, be it a missile, dog, or ship.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.22  2003/09/26 14:37:16  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.21  2003/09/13 08:27:28  Goober5000
 * added some minor things, such as code cleanup and the following:
 * --turrets will not fire at cargo
 * --MAX_SHIELD_SECTIONS substituted for the number 4 in many places
 * --supercaps have their own default message bitfields (distinguished from capships)
 * --turrets are allowed on fighters
 * --jump speed capped at 65m/s, to avoid ship travelling too far
 * --non-huge weapons now scale their damage, instead of arbitrarily cutting off
 * ----Goober5000
 *
 * Revision 2.20  2003/09/13 06:02:04  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.16  2003/04/29 01:03:21  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.15  2003/03/20 23:30:03  Goober5000
 * comments
 * --Goober500
 *
 * Revision 2.14  2003/03/18 08:44:05  Goober5000
 * added explosion-effect sexp and did some other minor housekeeping
 * --Goober5000
 *
 * Revision 2.13  2003/03/02 06:20:16  penguin
 * Tweaked for gcc
 *  - penguin
 *
 * Revision 2.12  2003/02/26 02:56:55  bobboau
 * fixed the bug with fighter beams not giveing you kills
 *
 * Revision 2.11  2003/02/25 06:22:49  bobboau
 * fixed a bunch of fighter beam bugs,
 * most notabley the sound now works corectly,
 * and they have limeted range with atenuated damage (table option)
 * added bank specific compatabilities
 *
 * Revision 2.10  2003/02/16 05:14:29  bobboau
 * added glow map nebula bug fix for d3d, someone should add a fix for glide too
 * more importantly I (think I) have fixed all major bugs with fighter beams, and added a bit of new functionality
 *
 * Revision 2.9  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.8  2003/01/19 01:07:42  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.7  2003/01/15 23:23:30  Goober5000
 * NOW the model duplicates work! :p
 * still gotta do the textures, but it shouldn't be hard now
 * --Goober5000
 *
 * Revision 2.6  2002/10/19 19:29:29  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.5  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.4  2002/07/29 20:48:51  penguin
 * Moved extern declaration of ssm_create outside of block (it wouldn't compile w/ gcc)
 *
 * Revision 2.3  2002/07/29 08:19:41  DTP
 * Bumped MAX_SUBSYS_LIST from 32 to 200
 *
 * Revision 2.2  2002/07/18 03:25:10  unknownplayer
 * no message
 *
 * Revision 2.1  2002/07/17 20:04:00  wmcoolmon
 * Added SSM code for Tag C from Bobboau. Note that strings.tbl may need to be updated.
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.5  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.4  2002/05/10 20:42:45  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.3  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.2  2002/05/03 22:07:10  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 61    9/14/99 3:26a Dave
 * Fixed laser fogging problem in nebula (D3D)> Fixed multiplayer
 * respawn-too-early problem. Made a few crash points safe.
 * 
 * 60    9/13/99 4:52p Dave
 * RESPAWN FIX
 * 
 * 59    9/09/99 11:17a Jimb
 * Removed bogus Int3() in do_subobj_hit_stuff(). 
 * 
 * 58    9/07/99 4:01p Dave
 * Fixed up a string.tbl paroblem (self destruct message). Make sure IPX
 * does everything properly (setting up address when binding). Remove
 * black rectangle background from UI_INPUTBOX.
 * 
 * 57    9/03/99 11:39p Mikek
 * Fix problem in sm3-01 of dual-fired Helios bombs only doing 1/4 damage.
 * 
 * 56    9/03/99 2:24p Mikek
 * Decrease overall damage done to player at Medium and Hard.  I believe
 * this was the major culprit in Flak cannons being so nasty.  The firing
 * rate seems to not have been an issue.
 * 
 * 55    9/02/99 11:55a Mikek
 * Zero damage to small ship subsystems from shockwaves.
 * 
 * 54    9/01/99 10:15a Dave
 * 
 * 53    8/27/99 10:42a Andsager
 * Don't reposition shockwave for BIG only HUGE ships
 * 
 * 52    8/27/99 1:34a Andsager
 * Modify damage by shockwaves for BIG|HUGE ships.  Modify shockwave damge
 * when weapon blows up.
 * 
 * 51    8/26/99 10:46p Andsager
 * Apply shockwave damage to lethality.
 * 
 * 50    8/26/99 8:52p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 49    8/26/99 5:14p Andsager
 * 
 * 48    8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 47    8/23/99 11:59a Andsager
 * Force choice of big fireball when Knossos destroyed.  Allow logging of
 * ship destroyed when no killer_name (ie, from debug).
 * 
 * 46    8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 45    8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 44    8/20/99 5:09p Andsager
 * Second pass on Knossos device explosion
 * 
 * 43    8/20/99 1:42p Andsager
 * Frist pass on Knossos explosion.
 * 
 * 42    8/06/99 9:46p Dave
 * Hopefully final changes for the demo.
 * 
 * 41    8/03/99 11:02p Dave
 * Maybe fixed sync problems in multiplayer.
 * 
 * 40    8/02/99 1:59p Dave
 * Fixed improper damage application to subobjects on a "big damage" ship.
 * 
 * 39    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 38    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 37    7/09/99 12:51a Andsager
 * Modify engine wash (1) less damage (2) only at a closer range (3) no
 * damage when engine is disabled
 * 
 * 36    6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 35    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 34    6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 33    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 32    5/27/99 12:14p Andsager
 * Some fixes for live debris when more than one subsys on ship with live
 * debris.  Set subsys strength (when 0) blows off subsystem.
 * sexp_hits_left_subsystem works for SUBSYSTEM_UNKNOWN.
 * 
 * 31    5/21/99 5:03p Andsager
 * Add code to display engine wash death.  Modify ship_kill_packet
 * 
 * 30    5/21/99 1:44p Andsager
 * Add engine wash gauge
 * 
 * 29    5/14/99 11:50a Andsager
 * Added vaporize for SMALL ships hit by HUGE beams.  Modified dying
 * frame.  Enlarged debris shards and range at which visible.
 * 
 * 28    5/12/99 2:55p Andsager
 * Implemented level 2 tag as priority in turret object selection
 * 
 * 27    5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 26    4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 25    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 24    4/20/99 3:43p Andsager
 * Added normal parameter to ship_apply_local_damage for case of ship_ship
 * collision.
 * 
 * 23    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 22    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 21    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 21    3/15/99 6:45p Daveb
 * Put in rough nebula bitmap support.
 * 
 * 20    3/10/99 6:51p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 19    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 18    2/26/99 5:39p Johnson
 * Put in some handling code for an assert().
 * 
 * 17    2/11/99 5:22p Andsager
 * Fixed bugs, generalized block Sexp_variables
 * 
 * 16    2/05/99 10:38a Johnson
 * Fixed improper object array reference. 
 * 
 * 15    2/04/99 1:23p Andsager
 * Apply max spark limit to ships created in mission parse
 * 
 * 14    1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 13    1/29/99 2:08a Dave
 * Fixed beam weapon collisions with players. Reduced size of scoring
 * struct for multiplayer. Disabled PXO.
 * 
 * 12    1/27/99 10:25p Andsager
 * Added OEM sparks - make intelligent choice of next spark location for
 * cruiser and cap ships
 * 
 * 11    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 10    1/21/99 4:22p Anoop
 * Removed bogus Int3() from show_dead_message(...)
 * 
 * 9     1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 8     12/23/98 2:53p Andsager
 * Added ship activation and gas collection subsystems, removed bridge
 * 
 * 7     11/17/98 4:27p Andsager
 * Stop sparks from emitting from destroyed subobjects
 * 
 * 6     11/09/98 2:11p Dave
 * Nebula optimizations.
 * 
 * 5     10/26/98 9:42a Dave
 * Early flak gun support.
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
 * 145   9/01/98 6:48p Dave
 * Energy suck weapon. Removed a couple of now-bogus asserts in tracker
 * code.
 * 
 * 144   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 143   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 142   5/24/98 8:49p Allender
 * put in Int3() to try and trap nasty error in multiplayer
 * 
 * 141   5/22/98 12:08p Mike
 * Don't create "Disarmed" event for small ships.
 * 
 * 140   5/22/98 11:00a Mike
 * Balance sm3-09a.
 * 
 * 139   5/18/98 12:41a Allender
 * fixed subsystem problems on clients (i.e. not reporting properly on
 * damage indicator).  Fixed ingame join problem with respawns.  minor
 * comm menu stuff
 * 
 * 138   5/13/98 6:54p Dave
 * More sophistication to PXO interface. Changed respawn checking so
 * there's no window for desynchronization between the server and the
 * clients.
 * 
 * 137   5/11/98 11:37a Mike
 * Don't allow deathroll duration for large ships to be shortened by
 * excess damage.
 * 
 * 136   5/10/98 11:30p Mike
 * Better firing of bombs, less likely to go into strafe mode.
 * 
 * 135   5/07/98 12:24a Hoffoss
 * Finished up sidewinder force feedback support.
 * 
 * 134   5/06/98 8:47a Andsager
 * Initial check in, allow ship sparks to turn on and off depending on
 * damage.
 * 
 * 133   5/06/98 1:12a Allender
 * fix sequencing names, added nprintf to help respawn debugging
 * 
 * 132   5/04/98 4:07p Andsager
 * Reduce deathroll rotvel cap and average by 25%.
 * 
 * 131   5/04/98 11:12a Mike
 * Make kamikaze ships detonate on impact -- no death roll.
 * 
 * 130   4/30/98 12:17a Andsager
 * Increase deathroll time for big ships and decrease max rotvel for big
 * ships
 * 
 * 129   4/27/98 6:02p Dave
 * Modify how missile scoring works. Fixed a team select ui bug. Speed up
 * multi_lag system. Put in new main hall.
 * 
 * 128   4/27/98 2:23p Andsager
 * Create fireballs on destroying subsystems of big ships.  Limit number
 * of sparks on small ships.
 * 
 * 127   4/24/98 5:35p Andsager
 * Fix sparks sometimes drawing not on model.  If ship is sphere in
 * collision, don't draw sparks.  Modify ship_apply_local_damage() to take
 * parameter no_spark.
 * 
 * 126   4/23/98 4:47p Andsager
 * Make deathroll rotvel have z component largest.
 * 
 * 125   4/22/98 9:10a Andsager
 * Resrore z component of deathroll rotvel, as before.  
 * 
 * 124   4/21/98 11:20p Andsager
 * Modify deatroll rotvel. Fix bug getting only positive values.  Add
 * random to current rotvel, make z rotvel always larger than x or y.
 * Move call to shipfx_large_blowup_init into Ship.cpp.
 * 
 * 123   4/21/98 10:38a Mike
 * Don't allow player to take _any_ kind of damage past stage 2 of
 * warpout.
 * 
 * 122   4/20/98 5:10p John
 * Took out cap of number of shards using num_particles.
 * 
 * 121   4/17/98 1:17p Mike
 * Fix bug with uninitialized damage_to_apply.
 * 
 * 120   4/17/98 11:05a Mike
 * Make number of pieces of small debris created by subsystem hits be
 * based on detail level.
 * Fix location of turret subsystem.  Was doubly globalizing, very bad on
 * large ships!
 * 
 * 119   4/16/98 3:40p Mike
 * Make subsys destroyed messages only get sent once.
 * 
 * 118   4/15/98 11:06p Mike
 * Better balance of damage done to subsystems by shockwaves.
 * 
 * 117   4/15/98 10:13p Mike
 * Fix application of damage to subsystems.
 * 
 * 116   4/14/98 9:15p John
 * Added flag to specify which ships get the new large ship exploding
 * effect.
 * 
 * 115   4/14/98 4:56p John
 * Hooked in Andsager's large ship exploding code, but it is temporarily
 * disabled.
 * 
 * 114   4/12/98 11:16a John
 * Made palette red hit effect more proportional to damage.
 * 
 * 113   4/10/98 12:16p Allender
 * fix ship hit kill and debris packets
 * 
 * 112   4/10/98 1:38a Allender
 * fix bug with -1 vs 255 (for current secondary bank).  Undid previous
 * code for ship_kill packets.
 * 
 * 111   4/09/98 5:44p Allender
 * multiplayer network object fixes.  debris and self destructed ships
 * should all sync up.  Fix problem where debris pieces (hull pieces) were
 * not getting a net signature
 * 
 * 110   4/09/98 5:27p Mike
 * Speed up deathrolls for smaller ships a bit.
 * 
 * 109   4/06/98 7:13p Allender
 * generate the death text
 * 
 * 108   4/01/98 1:48p Allender
 * major changes to ship collision in multiplayer.  Clients now do own
 * ship/ship collisions (with their own ship only)  Modifed the hull
 * update packet to be sent quicker when object is target of player.
 * 
 * 107   3/31/98 5:19p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 106   3/30/98 2:38p Mike
 * Add asteroid_density to detail level support.
 * No force explosion damage in training missions.
 * Make cargo deathrolls shorter.
 * 
 * 105   3/30/98 1:08a Lawrance
 * Implement "blast" icon.  Blink HUD icon when player ship is hit by a
 * blast.
 * 
 * 104   3/24/98 4:26p Lawrance
 * Make finding out if player killed self easier and more reliable
 * 
 * 103   3/24/98 2:16p Mike
 * Fix bug with variation in death roll rotational velocity.  It wasn't
 * getting initialized!
 * 
 * 102   3/21/98 3:36p Lawrance
 * Let damage gauge know when player has taken damage.
 * 
 * 101   3/21/98 3:37p Mike
 * Fix/optimize attacking of big ships.
 * 
 * 100   3/19/98 5:35p Lawrance
 * Correctly inform player if killed by ship explosion.
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "object/object.h"
#include "math/fvi.h"
#include "physics/physics.h"
#include "math/vecmat.h"
#include "ship/ship.h"
#include "model/model.h"
#include "io/key.h"
#include "weapon/weapon.h"
#include "radar/radar.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "math/floating.h"
#include "ship/ai.h"
#include "ship/ailocal.h"
#include "fireball/fireballs.h"
#include "debris/debris.h"
#include "hud/hud.h"
#include "io/timer.h"
#include "cfile/cfile.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"
#include "bmpman/bmpman.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "playerman/player.h"
#include "parse/parselo.h"
#include "freespace2/freespace.h"
#include "sound/sound.h"
#include "globalincs/linklist.h"
#include "hud/hudets.h"
#include "hud/hudtarget.h"
#include "ship/aigoals.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "ship/shipfx.h"
#include "parse/sexp.h"
#include "gamesequence/gamesequence.h"
#include "object/objectsnd.h"
#include "cmeasure/cmeasure.h"
#include "anim/animplay.h"
#include "controlconfig/controlsconfig.h"
#include "ship/afterburner.h"
#include "weapon/shockwave.h"
#include "hud/hudsquadmsg.h"
#include "weapon/swarm.h"
#include "ship/shiphit.h"
#include "particle/particle.h"
#include "popup/popup.h"
#include "weapon/emp.h"
#include "weapon/beam.h"
#include "demo/demo.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_respawn.h"
#include "network/multi_pmsg.h"
#endif

//#pragma optimize("", off)
//#pragma auto_inline(off)

struct ssm_firing_info;
extern void ssm_create(vector *target, vector *start, int ssm_index, ssm_firing_info *override);

typedef struct spark_pair {
	int index1, index2;
	float dist;
} spark_pair;

#define MAX_SPARK_PAIRS		((MAX_SHIP_HITS * MAX_SHIP_HITS - MAX_SHIP_HITS) / 2)

#define	BIG_SHIP_MIN_RADIUS	80.0f	//	ship radius above which death rolls can't be shortened by excessive damage

vector	Dead_camera_pos;
vector	Original_vec_to_deader;

//	Decrease damage applied to a subsystem based on skill level.
float	Skill_level_subsys_damage_scale[NUM_SKILL_LEVELS] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};

bool is_subsys_destroyed(ship *shipp, int submodel)
{
	ship_subsys *subsys;

	if (submodel == -1) {
		false;
	}

	for ( subsys=GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list); subsys = GET_NEXT(subsys) ) {
		if (subsys->system_info->subobj_num == submodel) {
			if (subsys->current_hits > 0) {
				return false;
			} else {
				return true;
			}
		}
	}

	return false;
}

// do_subobj_destroyed_stuff is called when a subobject for a ship is killed.  Separated out
// to separate function on 10/15/97 by MWA for easy multiplayer access.  It does all of the
// cool things like blowing off the model (if applicable, writing the logs, etc)
void do_subobj_destroyed_stuff( ship *ship_p, ship_subsys *subsys, vector* hitpos )
{
	ship_info *sip;
	object *ship_obj;
	model_subsystem *psub;
	vector	g_subobj_pos;
	int type, i, log_index;

	// get some local variables
	sip = &Ship_info[ship_p->ship_info_index];
	ship_obj = &Objects[ship_p->objnum];
	psub = subsys->system_info;
	type = psub->type;
	get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);

	// create fireballs when subsys destroy for large ships.
	object* objp = &Objects[ship_p->objnum];
	if (objp->radius > 100.0f) {
		// number of fireballs determined by radius of subsys
		int num_fireballs;
		if ( psub->radius < 3 ) {
			num_fireballs = 1;
		} else {
			 num_fireballs = 5;
		}

		vector temp_vec, center_to_subsys, rand_vec;
		vm_vec_sub(&center_to_subsys, &g_subobj_pos, &objp->pos);
		for (int i=0; i<num_fireballs; i++) {
			if (i==0) {
				// make first fireball at hitpos
				if (hitpos) {
					temp_vec = *hitpos;
				} else {
					temp_vec = g_subobj_pos;
				}
			} else {
				// make other fireballs at random positions, but try to keep on the surface
				vm_vec_rand_vec_quick(&rand_vec);
				float dot = vm_vec_dotprod(&center_to_subsys, &rand_vec);
				vm_vec_scale_add2(&rand_vec, &center_to_subsys, -dot/vm_vec_mag_squared(&center_to_subsys));
				vm_vec_scale_add(&temp_vec, &g_subobj_pos, &rand_vec, 0.5f*psub->radius);
			}

			// scale fireball size according to size of subsystem, but not less than 10
			float fireball_rad = psub->radius * 0.2f;
			if (fireball_rad < 10) {
				fireball_rad = 10.0f;
			}

			vector fb_vel;
			vm_vec_crossprod(&fb_vel, &objp->phys_info.rotvel, &center_to_subsys);
			vm_vec_add2(&fb_vel, &objp->phys_info.vel);
			fireball_create( &temp_vec, FIREBALL_EXPLOSION_MEDIUM, OBJ_INDEX(objp), fireball_rad, 0, &fb_vel );
		}
	}

#ifndef NO_NETWORK
	if ( MULTIPLAYER_MASTER ) {
		int index;

		index = ship_get_index_from_subsys(subsys, ship_p->objnum);
		
		vector hit;
		if (hitpos) {
			hit = *hitpos;
		} else {
			hit = g_subobj_pos;
		}
		send_subsystem_destroyed_packet( ship_p, index, hit );
	}
#endif

	// next do a quick sanity check on the current hits that we are keeping for the generic subsystems
	// I think that there might be rounding problems with the floats.  This code keeps us safe.
	if ( ship_p->subsys_info[type].num == 1 ) {
		ship_p->subsys_info[type].current_hits = 0.0f;
	} else {
		float hits;
		ship_subsys *ssp;

		hits = 0.0f;
		for ( ssp=GET_FIRST(&ship_p->subsys_list); ssp != END_OF_LIST(&ship_p->subsys_list); ssp = GET_NEXT(ssp) ) {
			if ( ssp->system_info->type == type )
				hits += ssp->current_hits;
		}
		ship_p->subsys_info[type].current_hits = hits;
	}

	// store an event in the event log.  Also, determine if all turrets or all
	// engines have been destroyed (if the subsystem is a turret or engine).
	// put a disabled or disarmed entry in the log if this is the case
	// 
	// MWA -- 1/8/98  A problem was found when trying to determine (via sexpression) when some subsystems
	// were destroyed.  The bottom line is that is the psub->name and psub->subobj_name are different,
	// then direct detection doesn't work.  (This scenario happens mainly with turrets and probably with
	// engines).  So, my solution is to encode the ship_info index, and the subsystem index into one
	// integer, and pass that as the "index" parameter to add_entry.  We'll use that information to
	// print out the info in the mission log.
	Assert( ship_p->ship_info_index < 65535 );

	// get the "index" of this subsystem in the ship info structure.
	for ( i = 0; i < sip->n_subsystems; i++ ) {
		if ( &(sip->subsystems[i]) == psub )
			break;

		// check alt stuff too
		if ( ship_p->alt_modelnum != -1 )
			if ( &(ship_p->subsystems[i]) == psub )
				break;
	}
	Assert( i < sip->n_subsystems );
	Assert( i < 65535 );
	log_index = ((ship_p->ship_info_index << 16) & 0xffff0000) | (i & 0xffff);

	// Don't log or display info about the activation subsytem
	int display = (psub->type != SUBSYSTEM_ACTIVATION);
	if (display) {
		mission_log_add_entry(LOG_SHIP_SUBSYS_DESTROYED, ship_p->ship_name, psub->subobj_name, log_index );
		if ( ship_obj == Player_obj ) {
			snd_play( &Snds[SND_SUBSYS_DIE_1], 0.0f );
			HUD_printf(XSTR( "Your %s subsystem has been destroyed", 499), psub->name);
		}
	}

	if ( psub->type == SUBSYSTEM_TURRET ) {
		if ( ship_p->subsys_info[type].current_hits == 0.0f ) {
			//	Don't create "disarmed" event for small ships.
			if (!(Ship_info[ship_p->ship_info_index].flags & SIF_SMALL_SHIP)) {
				mission_log_add_entry(LOG_SHIP_DISARMED, ship_p->ship_name, NULL );
				// ship_p->flags |= SF_DISARMED;
			}
		}
	} else if (psub->type == SUBSYSTEM_ENGINE ) {
		// when an engine is destroyed, we must change the max velocity of the ship
		// to be some fraction of its normal maximum value

		if ( ship_p->subsys_info[type].current_hits == 0.0f ) {
			mission_log_add_entry(LOG_SHIP_DISABLED, ship_p->ship_name, NULL );
			ship_p->flags |= SF_DISABLED;				// add the disabled flag
		}
	}

	if ( psub->subobj_num > -1 )	{
		shipfx_blow_off_subsystem(ship_obj,ship_p,subsys,&g_subobj_pos);
		subsys->submodel_info_1.blown_off = 1;
	}

	if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >-1) )		{
		subsys->submodel_info_2.blown_off = 1;
	}

	// play sound effect when subsys gets blown up
	int sound_index=-1;
	if ( Ship_info[ship_p->ship_info_index].flags & SIF_HUGE_SHIP ) {
		sound_index=SND_CAPSHIP_SUBSYS_EXPLODE;
	} else if ( Ship_info[ship_p->ship_info_index].flags & SIF_BIG_SHIP ) {
		sound_index=SND_SUBSYS_EXPLODE;
	}
	if ( sound_index >= 0 ) {
		snd_play_3d( &Snds[sound_index], &g_subobj_pos, &View_position );
	}
}

// Return weapon type that is associated with damaging_objp
// input:	damaging_objp		=>	object pointer responsible for damage
//	exit:		-1		=>	no weapon type is associated with damage object
//				>=0	=>	weapon type associated with damage object
int shiphit_get_damage_weapon(object *damaging_objp)
{
	int weapon_info_index = -1;

	if ( damaging_objp ) {
		switch(damaging_objp->type) {
		case OBJ_WEAPON:
			weapon_info_index = Weapons[damaging_objp->instance].weapon_info_index;
			break;
		case OBJ_SHOCKWAVE:
			weapon_info_index = shockwave_weapon_index(damaging_objp->instance);
			break;
		default:
			weapon_info_index = -1;
			break;
		}
	}

	return weapon_info_index;
}

//	Return range at which this object can apply damage.
//	Based on object type and subsystem type.
float subsys_get_range(object *other_obj, ship_subsys *subsys)
{
	float	range;

	if ((other_obj) && (other_obj->type == OBJ_SHOCKWAVE)) {
		range = Shockwaves[other_obj->instance].outer_radius * 0.75f;	//	Shockwaves were too lethal to subsystems.
	} else if ( subsys->system_info->type == SUBSYSTEM_TURRET ) {
		range = subsys->system_info->radius*3;
	} else {
		range = subsys->system_info->radius*2;
	}

	return range;
}

#define MAX_DEBRIS_SHARDS	16		// cap the amount of debris shards that fly off per hit

// Make some random debris particles.  Previous way was not very random.  Create debris 75% of the time.
// Don't worry about multiplayer since this debris is the small stuff that cannot collide
void create_subsys_debris(object *ship_obj, vector *hitpos)
{
	float show_debris = frand();
	
	if ( show_debris <= 0.75f ) {
		int ndebris;

		ndebris = (int)(show_debris * Detail.num_small_debris) + 1;			// number of pieces of debris to create

		if ( ndebris > MAX_DEBRIS_SHARDS )
			ndebris = MAX_DEBRIS_SHARDS;

		//mprintf(( "Damage = %.1f, ndebris=%d\n", show_debris, ndebris ));
		for (int i=0; i<ndebris; i++ )	{
			debris_create( ship_obj, -1, -1, hitpos, hitpos, 0, 1.0f );
		}
	}
}

void create_vaporize_debris(object *ship_obj, vector *hitpos)
{
	int ndebris;
	float show_debris = frand();

	ndebris = (int)(4.0f * ((0.5f + show_debris) * Detail.num_small_debris)) + 5;			// number of pieces of debris to create

	if ( ndebris > MAX_DEBRIS_SHARDS ) {
		ndebris = MAX_DEBRIS_SHARDS;
	}

	//mprintf(( "Damage = %.1f, ndebris=%d\n", show_debris, ndebris ));
	for (int i=0; i<ndebris; i++ )	{
		debris_create( ship_obj, -1, -1, hitpos, hitpos, 0, 1.4f );
	}
}

#define	MAX_SUBSYS_LIST	200 //DTP MAX SUBSYS LIST BUMPED FROM 32 to 200, ahmm 32???

typedef struct {
	float	dist;
	float	range;
	ship_subsys	*ptr;
} sublist;

// do_subobj_hit_stuff() is called when a collision is detected between a ship and something
// else.  This is where we see if any sub-objects on the ship should take damage.
//
//	Depending on where the collision occurs, the sub-system and surrounding hull will take 
// different amounts of damage.  The amount of damage a sub-object takes depending on how
// close the colliding object is to the center of the sub-object.  The remaining hull damage
// will be returned to the caller via the damage parameter.
//
//
// 0   -> 0.5 radius   : 100% subobject    0%  hull
// 0.5 -> 1.0 radius   :  50% subobject   50%  hull
// 1.0 -> 2.0 radius   :  25% subobject   75%  hull
//     >  2.0 radius   :   0% subobject  100%  hull
//
//
// The weapon damage is not neccesarily distributed evently between sub-systems when more than
// one sub-system is to take damage.  Whenever damage is to be assigned to a sub-system, the above
// percentages are used.  So, if more than one sub-object is taking damage, the second sub-system
// to be assigned damage will take less damage.  Eg. weapon hits in the 25% damage range of two
// subsytems, and the weapon damage is 12.  First subsystem takes 3 points damage.  Second subsystem
// will take 0.25*9 = 2.25 damage.  Should be close enough for most cases, and hull would receive 
// 0.75 * 9 = 6.75 damage.
//
//	Used to use the following constants, but now damage is linearly scaled up to 2x the subsystem
//	radius.  Same damage applied as defined by constants below.
//
//	Returns unapplied damage, which will probably be applied to the hull.
//
// Shockwave damage is handled here.  If other_obj->type == OBJ_SHOCKWAVE, it's a shockwave.
// apply the same damage to all subsystems.
//	Note: A negative damage number means to destroy the corresponding subsystem.  For example, call with -SUBSYSTEM_ENGINE to destroy engine.

float do_subobj_hit_stuff(object *ship_obj, object *other_obj, vector *hitpos, float damage)
{
	vector			g_subobj_pos;
	float				damage_left;
	int				weapon_info_index;
	ship_subsys		*subsys;
	ship				*ship_p;
	sublist			subsys_list[MAX_SUBSYS_LIST];
	vector			hitpos2;

	ship_p = &Ships[ship_obj->instance];

	//	Don't damage player subsystems in a training mission.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
		if (ship_obj == Player_obj){
			return damage;
		}
	}

	//	Shockwave damage is applied like weapon damage.  It gets consumed.
	if ((other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE)) {
		//	MK, 9/2/99.  Shockwaves do zero subsystem damage on small ships.
		if ( Ship_info[ship_p->ship_info_index].flags & (SIF_SMALL_SHIP))
			return damage;
		else {

			damage_left = Shockwaves[other_obj->instance].damage/4.0f;
		}
		hitpos2 = other_obj->pos;
	} else {
		damage_left = damage;
		hitpos2 = *hitpos;
	}

	// scale subsystem damage if appropriate
	weapon_info_index = shiphit_get_damage_weapon(other_obj);
	if ((weapon_info_index >= 0) && (other_obj->type == OBJ_WEAPON)) {
		damage_left *= Weapon_info[weapon_info_index].subsystem_factor;
	}


#ifndef NDEBUG
	float hitpos_dist = vm_vec_dist( hitpos, &ship_obj->pos );	
	if ( hitpos_dist > ship_obj->radius * 2.0f )	{
		mprintf(( "BOGUS HITPOS PASSED TO DO_SUBOBJ_HIT_STUFF (%.1f > %.1f)!\n", hitpos_dist, ship_obj->radius * 2.0f ));
		// Int3();	// Get John ASAP!!!!  Someone passed a local coordinate instead of world for hitpos probably.
	}
#endif

	create_subsys_debris(ship_obj, hitpos);

	//	First, create a list of the N subsystems within range.
	//	Then, one at a time, process them in order.
	int	count = 0;
	for ( subsys=GET_FIRST(&ship_p->subsys_list); subsys != END_OF_LIST(&ship_p->subsys_list); subsys = GET_NEXT(subsys) ) {
		#ifndef NDEBUG
		//	Debug option.  If damage is negative of subsystem type, then just destroy that subsystem.
		if (damage < 0.0f) {
			// single player or multiplayer
			Assert(Player_ai->targeted_subsys != NULL);
			if ( (subsys == Player_ai->targeted_subsys) && (subsys->current_hits > 0) ) {
				Assert(subsys->system_info->type == (int) -damage);
				ship_p->subsys_info[subsys->system_info->type].current_hits -= subsys->current_hits;
				if (ship_p->subsys_info[subsys->system_info->type].current_hits < 0) {
					ship_p->subsys_info[subsys->system_info->type].current_hits = 0.0f;
				}
				subsys->current_hits = 0.0f;
				do_subobj_destroyed_stuff( ship_p, subsys, hitpos );
				continue;
			} else {
				continue;
			}
		}
		#endif
		
		if (subsys->current_hits > 0.0f) {
			float	dist;

			// calculate the distance between the hit and the subsystem center
			get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);
			dist = vm_vec_dist_quick(&hitpos2, &g_subobj_pos);

			float range = subsys_get_range(other_obj, subsys);

			if ( dist < range) {
				subsys_list[count].dist = dist;
				subsys_list[count].range = range;
				subsys_list[count].ptr = subsys;
				count++;

				if (count >= MAX_SUBSYS_LIST){
					break;
				}
			}
		}
	}

	//	Now scan the sorted list of subsystems in range.
	//	Apply damage to the nearest one first, subtracting off damage as we go.
	int	i, j;
	for (j=0; j<count; j++) {
		float	dist, range;
		ship_subsys	*subsys;

		int	min_index = -1;
		float	min_dist = 9999999.9f;

		for (i=0; i<count; i++) {
			if (subsys_list[i].dist < min_dist) {
				min_dist = subsys_list[i].dist;
				min_index = i;
			}
		}
		Assert(min_index != -1);

		float	damage_to_apply = 0.0f;
		subsys = subsys_list[min_index].ptr;
		range = subsys_list[min_index].range;
		dist = subsys_list[min_index].dist;
		subsys_list[min_index].dist = 9999999.9f;	//	Make sure we don't use this one again.

		//	HORRIBLE HACK!
		//	MK, 9/4/99
		//	When Helios bombs are dual fired against the Juggernaut in sm3-01 (FS2), they often
		//	miss their target.  There is code dating to FS1 in the collision code to detect that a bomb or
		//	missile has somehow missed its target.  It gets its lifeleft set to 0.1 and then it detonates.
		//	Unfortunately, the shockwave damage was cut by 4 above.  So boost it back up here.
		if ((dist < 10.0f) && ((other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE))) {
			damage_left *= 4.0f * Weapon_info[weapon_info_index].subsystem_factor;;
		}

//		if (damage_left > 100.0f)
//			nprintf(("AI", "Applying %7.3f damage to subsystem %7.3f units away.\n", damage_left, dist));

		if ( dist < range/2.0f ) {
			damage_to_apply = damage_left;
		} else if ( dist < range ) {
			damage_to_apply = damage_left * (1.0f - dist/range);
		}

		// if we're not in CLIENT_NODAMAGE multiplayer mode (which is a the NEW way of doing things)
#ifndef NO_NETWORK
		if (damage_to_apply > 0.1f && !(MULTIPLAYER_CLIENT) && !(Game_mode & GM_DEMO_PLAYBACK)) {
#else
		if (damage_to_apply > 0.1f && !(Game_mode & GM_DEMO_PLAYBACK)) {
#endif
			//	Decrease damage to subsystems to player ships.
			if (ship_obj->flags & OF_PLAYER_SHIP){
				damage_to_apply *= Skill_level_subsys_damage_scale[Game_skill_level];
			}
		
			subsys->current_hits -= damage_to_apply;
			ship_p->subsys_info[subsys->system_info->type].current_hits -= damage_to_apply;
			damage_left -= damage_to_apply;		// decrease the damage left to apply to the ship subsystems

			if (subsys->current_hits < 0.0f) {
				damage_left -= subsys->current_hits;
				ship_p->subsys_info[subsys->system_info->type].current_hits -= subsys->current_hits;
				subsys->current_hits = 0.0f;					// set to 0 so repair on subsystem takes immediate effect
			}

			if ( ship_p->subsys_info[subsys->system_info->type].current_hits < 0.0f ){
				ship_p->subsys_info[subsys->system_info->type].current_hits = 0.0f;
			}

			// multiplayer clients never blow up subobj stuff on their own
#ifndef NO_NETWORK
			if ( (subsys->current_hits <= 0.0f) && !MULTIPLAYER_CLIENT && !(Game_mode & GM_DEMO_PLAYBACK)){
#else
			if ( (subsys->current_hits <= 0.0f) && !(Game_mode & GM_DEMO_PLAYBACK)){
#endif
				do_subobj_destroyed_stuff( ship_p, subsys, hitpos );
			}

			if (damage_left <= 0)	{ // no more damage to distribute, so stop checking
				damage_left = 0.0f;
				break;
			}
		}
//nprintf(("AI", "j=%i, sys = %s, dam = %6.1f, dam left = %6.1f, subhits = %5.0f\n", j, subsys->system_info->name, damage_to_apply, damage_left, subsys->current_hits));
	}

	//	Note: I changed this to return damage_left and it completely screwed up balance.
	//	It had taken a few MX-50s to destory an Anubis (with 40% hull), then it took maybe ten.
	//	So, I left it alone. -- MK, 4/15/98
	return damage;
}

// Store who/what killed the player, so we can tell the player how he died
void shiphit_record_player_killer(object *killer_objp, player *p)
{
	switch (killer_objp->type) {

	case OBJ_WEAPON:
		p->killer_objtype=OBJ_WEAPON;
		p->killer_weapon_index=Weapons[killer_objp->instance].weapon_info_index;
		p->killer_species = Ship_info[Ships[Objects[killer_objp->parent].instance].ship_info_index].species;

		if ( &Objects[killer_objp->parent] == Player_obj ) {
			// killed by a missile?
			if(Weapon_info[p->killer_weapon_index].subtype == WP_MISSILE){
				p->flags |= PLAYER_FLAGS_KILLED_SELF_MISSILES;
			} else {
				p->flags |= PLAYER_FLAGS_KILLED_SELF_UNKNOWN;
			}
		}

#ifndef NO_NETWORK
		// in multiplayer, record callsign of killer if killed by another player
		if ( (Game_mode & GM_MULTIPLAYER) && ( Objects[killer_objp->parent].flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( &Objects[killer_objp->parent] );
			if ( pnum != -1 ) {
				strcpy(p->killer_parent_name, Net_players[pnum].player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object of weapon for killer of %s\n", p->callsign));
			}
		}
		else
#endif
		{
			strcpy(p->killer_parent_name, Ships[Objects[killer_objp->parent].instance].ship_name);
		}
		break;

	case OBJ_SHOCKWAVE:
		p->killer_objtype=OBJ_SHOCKWAVE;
		p->killer_weapon_index=shockwave_weapon_index(killer_objp->instance);
		p->killer_species = Ship_info[Ships[Objects[killer_objp->parent].instance].ship_info_index].species;

		if ( &Objects[killer_objp->parent] == Player_obj ) {
			p->flags |= PLAYER_FLAGS_KILLED_SELF_SHOCKWAVE;
		}

#ifndef NO_NETWORK
		if ( (Game_mode & GM_MULTIPLAYER) && ( Objects[killer_objp->parent].flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( &Objects[killer_objp->parent] );
			if ( pnum != -1 ) {
				strcpy(p->killer_parent_name, Net_players[pnum].player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object of shockwave for killer of %s\n", p->callsign));
			}
		}
		else
#endif
		{
			strcpy(p->killer_parent_name, Ships[Objects[killer_objp->parent].instance].ship_name);
		}
		break;

	case OBJ_SHIP:
		p->killer_objtype=OBJ_SHIP;
		p->killer_weapon_index=-1;
		p->killer_species = Ship_info[Ships[killer_objp->instance].ship_info_index].species;

		if ( Ships[killer_objp->instance].flags & SF_EXPLODED ) {
			p->flags |= PLAYER_FLAGS_KILLED_BY_EXPLOSION;
		}

		if ( Ships[Objects[p->objnum].instance].wash_killed ) {
			p->flags |= PLAYER_FLAGS_KILLED_BY_ENGINE_WASH;
		}

#ifndef NO_NETWORK
		// in multiplayer, record callsign of killer if killed by another player
		if ( (Game_mode & GM_MULTIPLAYER) && (killer_objp->flags & OF_PLAYER_SHIP) ) {
			int pnum;

			pnum = multi_find_player_by_object( killer_objp );
			if ( pnum != -1 ) {
				strcpy(p->killer_parent_name, Net_players[pnum].player->callsign);
			} else {
				nprintf(("Network", "Couldn't find player object for killer of %s\n", p->callsign));
			}
		}
		else
#endif
		{
			strcpy(p->killer_parent_name, Ships[killer_objp->instance].ship_name);
		}
		break;
	
	case OBJ_DEBRIS:
	case OBJ_ASTEROID:
		if ( killer_objp->type == OBJ_DEBRIS ) {
			p->killer_objtype = OBJ_DEBRIS;
		} else {
			p->killer_objtype = OBJ_ASTEROID;
		}
		p->killer_weapon_index=-1;
		p->killer_species = SPECIES_NONE;
		p->killer_parent_name[0] = '\0';
		break;

	case OBJ_BEAM:
		int beam_obj;
		beam_obj = beam_get_parent(killer_objp);
		p->killer_species = SPECIES_NONE;		
		p->killer_objtype = OBJ_BEAM;
		if(beam_obj != -1){			
			if((Objects[beam_obj].type == OBJ_SHIP) && (Objects[beam_obj].instance >= 0)){
				strcpy(p->killer_parent_name, Ships[Objects[beam_obj].instance].ship_name);
			}
			p->killer_species = Ship_info[Ships[Objects[beam_obj].instance].ship_info_index].species;
		} else {			
			strcpy(p->killer_parent_name, "");
		}
		break;
	
	case OBJ_NONE:
		if ( Game_mode & GM_MULTIPLAYER ) {
			Int3();
		}
		p->killer_objtype=-1;
		p->killer_weapon_index=-1;
		p->killer_parent_name[0]=0;
		p->killer_species = SPECIES_NONE;
		break;

	default:
		Int3();
		break;
	}
}

//	Say dead stuff.
void show_dead_message(object *ship_obj, object *other_obj)
{
	int pnum;
	player *player_p;

	// not doing anything when a non player dies.
	if ( !(ship_obj->flags & OF_PLAYER_SHIP) ){
		return;
	}

	if(other_obj == NULL){
		return;
	}

	// Get a pointer to the player (we are assured a player ship was killed)
	if ( Game_mode & GM_NORMAL ) {
		player_p = Player;
#ifndef NO_NETWORK
	} else {
		// in multiplayer, get a pointer to the player that died.
		pnum = multi_find_player_by_object( ship_obj );
		if ( pnum == -1 ) {
			//Int3();				// this condition is bad bad bad -- get Allender
			return;
		}
		player_p = Net_players[pnum].player;
#endif
	}

#ifndef NO_NETWORK
	// multiplayer clients should already have this information.
	if ( !MULTIPLAYER_CLIENT ){
		shiphit_record_player_killer( other_obj, player_p );
	}
#endif

	// display a hud message is the guy killed isn't me (multiplayer only)
	/*
	if ( (Game_mode & GM_MULTIPLAYER) && (ship_obj != Player_obj) ) {
		char death_text[256];

		player_generate_death_text( player_p, death_text );
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, death_text);
	}
	*/
}

/* JAS: THIS DOESN'T SEEM TO BE USED, SO I COMMENTED IT OUT
//	Apply damage to a ship, destroying if necessary, etc.
//	Returns portion of damage that exceeds ship shields, ie the "unused" portion of the damage.
//	Note: This system does not use the mesh shield.  It applies damage to the overall ship shield.
float apply_damage_to_ship(object *objp, float damage)
{
	float	_ss;

	add_shield_strength(objp, -damage);

	// check if shields are below 0%, if so take leftover damage and apply to ship integrity
	if ((_ss = get_shield_strength(objp)) < 0.0f ) {
		damage = -_ss;
		set_shield_strength(objp, 0.0f);
	} else
		damage = 0.0f;

	return damage;
}
*/

//	Do music processing for a ship hit.
void ship_hit_music(object *ship_obj, object *other_obj)
{
	ship* ship_p = &Ships[ship_obj->instance];

	// Switch to battle track when a ship is hit by fire 
	//
	// If the ship hit has an AI class of none, it is a Cargo, NavBuoy or other non-aggressive
	// ship, so don't start the battle music	
	if (stricmp(Ai_class_names[Ai_info[ship_p->ai_index].ai_class], NOX("none"))) {
		int team_1, team_2;
		// Only start if ship hit and firing ship are from different teams
		team_1 = Ships[ship_obj->instance].team;
		switch ( other_obj->type ) {
			case OBJ_SHIP:
				team_2 = Ships[other_obj->instance].team;
				if ( !stricmp(Ai_class_names[Ai_info[Ships[other_obj->instance].ai_index].ai_class], NOX("none")) ) {
					team_1 = team_2;
				}
				break;

			case OBJ_WEAPON:
				// parent of weapon is object num of ship that fired it
				team_2 = Ships[Objects[other_obj->parent].instance].team;	
				break;

			default:
				team_2 = team_1;
				break;	// Unexpected object type collided with ship, no big deal.
		} // end switch

		if ( team_1 != team_2 )
			event_music_battle_start();
	}
}

//	Make sparks fly off a ship.
// Currently used in misison_parse to create partially damaged ships.
// NOTE: hitpos is in model coordinates on the detail[0] submodel (highest detail hull)
// WILL NOT WORK RIGHT IF ON A ROTATING SUBMODEL
void ship_hit_sparks_no_rotate(object *ship_obj, vector *hitpos)
{
	ship		*ship_p = &Ships[ship_obj->instance];

	int n = ship_p->num_hits;
	if (n >= MAX_SHIP_HITS)	{
		n = rand() % MAX_SHIP_HITS;
	} else {
		ship_p->num_hits++;
	}

	// No rotation.  Just make the spark
	ship_p->sparks[n].pos = *hitpos;
	ship_p->sparks[n].submodel_num = -1;

	shipfx_emit_spark(ship_obj->instance, n);		// Create the first wave of sparks

	if ( n == 0 )	{
		ship_p->next_hit_spark = timestamp(0);		// when a hit spot will spark
	}
}

// find the max number of sparks allowed for ship
// limited for fighter by hull % others by radius.
int get_max_sparks(object* ship_obj)
{
	Assert(ship_obj->type == OBJ_SHIP);
	Assert((ship_obj->instance >= 0) && (ship_obj->instance < MAX_SHIPS));
	if(ship_obj->type != OBJ_SHIP){
		return 1;
	}
	if((ship_obj->instance < 0) || (ship_obj->instance >= MAX_SHIPS)){
		return 1;
	}

	ship *ship_p = &Ships[ship_obj->instance];
	ship_info* si = &Ship_info[ship_p->ship_info_index];
	if (si->flags & SIF_FIGHTER) {
		float hull_percent = ship_obj->hull_strength / ship_p->ship_initial_hull_strength;

		if (hull_percent > 0.8f) {
			return 1;
		} else if (hull_percent > 0.3f) {
			return 2;
		} else {
			return 3;
		}
	} else {
		int num_sparks = (int) (ship_obj->radius * 0.08f);
		if (num_sparks < 3) {
			return 3;
		} else if (num_sparks > MAX_SHIP_HITS) {
			return MAX_SHIP_HITS;
		} else {
			return num_sparks;
		}
	}
}


// helper function to qsort, sorting spark pairs by distance
int spark_compare( const void *elem1, const void *elem2 )
{
	spark_pair *pair1 = (spark_pair *) elem1;
	spark_pair *pair2 = (spark_pair *) elem2;

	Assert(pair1->dist >= 0);
	Assert(pair2->dist >= 0);

	if ( pair1->dist <  pair2->dist ) {
		return -1;
	} else {
		return 1;
	}
}

// for big ships, when all spark slots are filled, make intelligent choice of one to be recycled
int choose_next_spark(object *ship_obj, vector *hitpos)
{
	int i, j, count, num_sparks, num_spark_pairs, spark_num;
	vector world_hitpos[MAX_SHIP_HITS];
	spark_pair spark_pairs[MAX_SPARK_PAIRS];
	ship *shipp = &Ships[ship_obj->instance];

	// only choose next spark when all slots are full
	Assert(get_max_sparks(ship_obj) == Ships[ship_obj->instance].num_hits);

	// get num_sparks
	num_sparks = Ships[ship_obj->instance].num_hits;
	Assert(num_sparks <= MAX_SHIP_HITS);

	// get num_spark_paris -- only sort these
	num_spark_pairs = (num_sparks * num_sparks - num_sparks) / 2;

	// get the world hitpos for all sparks
	bool model_started = false;
	for (spark_num=0; spark_num<num_sparks; spark_num++) {
		if (shipp->sparks[spark_num].submodel_num != -1) {
			if ( !model_started) {
				model_started = true;
				ship_model_start(ship_obj);
			}
			model_find_world_point(&world_hitpos[spark_num], &shipp->sparks[spark_num].pos, shipp->modelnum, shipp->sparks[spark_num].submodel_num, &ship_obj->orient, &ship_obj->pos);
		} else {
			// rotate sparks correctly with current ship orient
			vm_vec_unrotate(&world_hitpos[spark_num], &shipp->sparks[spark_num].pos, &ship_obj->orient);
			vm_vec_add2(&world_hitpos[spark_num], &ship_obj->pos);
		}
	}

	if (model_started) {
		ship_model_stop(ship_obj);
	}

	// check we're not making a spark in the same location as a current one
	for (i=0; i<num_sparks; i++) {
		float dist = vm_vec_dist_squared(&world_hitpos[i], hitpos);
		if (dist < 1) {
			return i;
		}
	}

	// not same location, so maybe do random recyling
	if (frand() > 0.5f) {
		return (rand() % num_sparks);
	}

	// initialize spark pairs
	for (i=0; i<num_spark_pairs; i++) {
		spark_pairs[i].index1 = 0;
		spark_pairs[i].index2 = 0;
		spark_pairs[i].dist = FLT_MAX;
	}

	// set spark pairs
	count = 0;
	for (i=1; i<num_sparks; i++) {
		for (j=0; j<i; j++) {
			spark_pairs[count].index1 = i;
			spark_pairs[count].index2 = j;
			spark_pairs[count++].dist = vm_vec_dist_squared(&world_hitpos[i], &world_hitpos[j]);
		}
	}
	Assert(count == num_spark_pairs);

	// sort pairs
	qsort(spark_pairs, count, sizeof(spark_pair), spark_compare);
	//mprintf(("Min spark pair dist %.1f\n", spark_pairs[0].dist));

	// look through the first few sorted pairs, counting number of indices of closest pair
	int index1 = spark_pairs[0].index1;
	int index2 = spark_pairs[0].index2;
	int count1 = 0;
	int count2 = 0;

	for (i=1; i<6; i++) {
		if (spark_pairs[i].index1 == index1) {
			count1++;
		}
		if (spark_pairs[i].index2 == index1) {
			count1++;
		}
		if (spark_pairs[i].index1 == index2) {
			count2++;
		}
		if (spark_pairs[i].index2 == index2) {
			count2++;
		}
	}

	// recycle spark which has most indices in sorted list of pairs
	if (count1 > count2) {
		return index1;
	} else {
		return index2;
	}
}


//	Make sparks fly off a ship.
void ship_hit_create_sparks(object *ship_obj, vector *hitpos, int submodel_num)
{
	vector	tempv;
	ship		*ship_p = &Ships[ship_obj->instance];

	int n, max_sparks;

	n = ship_p->num_hits;
	max_sparks = get_max_sparks(ship_obj);

	if (n >= max_sparks)	{
		if ( Ship_info[ship_p->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP) ) {
			// large ship, choose intelligently
			n = choose_next_spark(ship_obj, hitpos);
		} else {
			// otherwise, normal choice
			n = rand() % max_sparks;
		}
	} else {
		ship_p->num_hits++;
	}

	ship *pship = &Ships[ship_obj->instance];

	bool instancing = false;
	// decide whether to do instancing
	if (submodel_num != -1) {
		polymodel *pm = model_get(pship->modelnum);
		if (pm->detail[0] != submodel_num) {
			// submodel is not hull
			// OPTIMIZE ... check if submodel can not rotate
			instancing = true;
		}
	}

	if (instancing) {
		// get the hit position in the subobject RF
		ship_model_start(ship_obj);
		vector temp_zero, temp_x, temp_y, temp_z;
		model_find_world_point(&temp_zero, &vmd_zero_vector, pship->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_x, &vmd_x_vector, pship->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_y, &vmd_y_vector, pship->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);
		model_find_world_point(&temp_z, &vmd_z_vector, pship->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);
		ship_model_stop(ship_obj);

		// find submodel x,y,z axes
		vm_vec_sub2(&temp_x, &temp_zero);
		vm_vec_sub2(&temp_y, &temp_zero);
		vm_vec_sub2(&temp_z, &temp_zero);

		// find displacement from submodel origin
		vector diff;
		vm_vec_sub(&diff, hitpos, &temp_zero);

		// find displacement from submodel origin in submodel RF
		ship_p->sparks[n].pos.xyz.x = vm_vec_dotprod(&diff, &temp_x);
		ship_p->sparks[n].pos.xyz.y = vm_vec_dotprod(&diff, &temp_y);
		ship_p->sparks[n].pos.xyz.z = vm_vec_dotprod(&diff, &temp_z);
		ship_p->sparks[n].submodel_num = submodel_num;
		ship_p->sparks[n].end_time = timestamp(-1);
	} else {
		// Rotate hitpos into ship_obj's frame of reference.
		vm_vec_sub(&tempv, hitpos, &ship_obj->pos);
		vm_vec_rotate(&ship_p->sparks[n].pos, &tempv, &ship_obj->orient);
		ship_p->sparks[n].submodel_num = -1;
		ship_p->sparks[n].end_time = timestamp(-1);
	}

	// Create the first wave of sparks
	shipfx_emit_spark(ship_obj->instance, n);

	if ( n == 0 )	{
		ship_p->next_hit_spark = timestamp(0);		// when a hit spot will spark
	}
}

//	Called from ship_hit_kill() when we detect the player has been killed.
void player_died_start(object *killer_objp)
{
	nprintf(("Network", "starting my player death\n"));
	gameseq_post_event(GS_EVENT_DEATH_DIED);	
	
/*	vm_vec_scale_add(&Dead_camera_pos, &Player_obj->pos, &Player_obj->orient.fvec, -10.0f);
	vm_vec_scale_add2(&Dead_camera_pos, &Player_obj->orient.uvec, 3.0f);
	vm_vec_scale_add2(&Dead_camera_pos, &Player_obj->orient.rvec, 5.0f);
*/

	//	Create a good vector for the camera to move along during death sequence.
	object	*other_objp = NULL;

	// on multiplayer clients, there have been occasions where we haven't been able to determine
	// the killer of a ship (due to bogus/mismatched/timed-out signatures on the client side).  If
	// we don't know the killer, use the Player_obj as the other_objp for camera position.
	if ( killer_objp ) {
		switch (killer_objp->type) {
		case OBJ_WEAPON:
		case OBJ_SHOCKWAVE:
			other_objp = &Objects[killer_objp->parent];
			break;
		case OBJ_SHIP:
		case OBJ_DEBRIS:
		case OBJ_ASTEROID:
		case OBJ_NONE:	//	Something that just got deleted due to also dying -- it happened to me! --MK.		
			other_objp = killer_objp;
			break;

		case OBJ_BEAM:
			int beam_obj_parent;
			beam_obj_parent = beam_get_parent(killer_objp);
			if(beam_obj_parent == -1){
				other_objp = killer_objp;
			} else {
				other_objp = &Objects[beam_obj_parent];
			}
			break;

		default:
			Int3();		//	Killed by an object of a peculiar type.  What is it?
			other_objp = killer_objp;	//	Enable to continue, just in case we shipped it with this bug...
		}
	} else {
		other_objp = Player_obj;
	}

	vm_vec_add(&Original_vec_to_deader, &Player_obj->orient.vec.fvec, &Player_obj->orient.vec.rvec);
	vm_vec_scale(&Original_vec_to_deader, 2.0f);
	vm_vec_add2(&Original_vec_to_deader, &Player_obj->orient.vec.uvec);
	vm_vec_normalize(&Original_vec_to_deader);

	vector	vec_from_killer;
	vector	*side_vec;
	float		dist;

	Assert(other_objp != NULL);

	if (Player_obj == other_objp) {
		dist = 50.0f;
		vec_from_killer = Player_obj->orient.vec.fvec;
	} else {
		dist = vm_vec_normalized_dir(&vec_from_killer, &Player_obj->pos, &other_objp->pos);
	}

	if (dist > 100.0f)
		dist = 100.0f;
	vm_vec_scale_add(&Dead_camera_pos, &Player_obj->pos, &vec_from_killer, dist);

	float	dot = vm_vec_dot(&Player_obj->orient.vec.rvec, &vec_from_killer);
	if (fl_abs(dot) > 0.8f)
		side_vec = &Player_obj->orient.vec.fvec;
	else
		side_vec = &Player_obj->orient.vec.rvec;
	
	vm_vec_scale_add2(&Dead_camera_pos, side_vec, 10.0f);

	Player_ai->target_objnum = -1;		//	Clear targeting.  Otherwise, camera pulls away from player as soon as he blows up.

	// stop any playing emp effect
	emp_stop_local();
}


#define	DEATHROLL_TIME						3000			//	generic deathroll is 3 seconds (3 * 1000 milliseconds)
#define	MIN_PLAYER_DEATHROLL_TIME		1000			// at least one second deathroll for a player
#define	DEATHROLL_ROTVEL_CAP				6.3f			// maximum added deathroll rotvel in rad/sec (about 1 rev / sec)
#define	DEATHROLL_ROTVEL_MIN				0.8f			// minimum added deathroll rotvel in rad/sec (about 1 rev / 12 sec)
#define	DEATHROLL_MASS_STANDARD			50				// approximate mass of lightest ship
#define	DEATHROLL_VELOCITY_STANDARD	70				// deathroll rotvel is scaled according to ship velocity
#define	DEATHROLL_ROTVEL_SCALE			4				// constant determines how quickly deathroll rotvel is ramped up  (smaller is faster)

void saturate_fabs(float *f, float max)
{
	if ( fl_abs(*f) > max) {
		if (*f > 0)
			*f = max;
		else
			*f = -max;
	}
}

// function to do generic things when a ship explodes
void ship_generic_kill_stuff( object *objp, float percent_killed )
{
	float rotvel_mag;
	int	delta_time;
	ship	*sp;

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS );
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}
	sp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[sp->ship_info_index];

	// if recording demo
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_ship_kill(objp);
	}

	ai_announce_ship_dying(objp);

	ship_stop_fire_primary(objp);	//mostly for stopping fighter beam looping sounds -Bobboau

	sp->flags |= SF_DYING;
	objp->phys_info.flags |= (PF_DEAD_DAMP | PF_REDUCED_DAMP);
	delta_time = (int) (DEATHROLL_TIME);

	//	For smaller ships, subtract off time proportional to excess damage delivered.
	if (objp->radius < BIG_SHIP_MIN_RADIUS)
		delta_time -=  (int) (1.01f - 4*percent_killed);

	//	Cut down cargo death rolls.  Looks a little silly. -- MK, 3/30/98.
	if (sip->flags & SIF_CARGO) {
		delta_time /= 4;
	}
	
	//	Prevent bogus timestamps.
	if (delta_time < 2)
		delta_time = 2;
	
	if (objp->flags & OF_PLAYER_SHIP) {
		//	Note: Kamikaze ships have no minimum death time.
		if (!(Ai_info[Ships[objp->instance].ai_index].ai_flags & AIF_KAMIKAZE) && (delta_time < MIN_PLAYER_DEATHROLL_TIME))
			delta_time = MIN_PLAYER_DEATHROLL_TIME;
	}

	//nprintf(("AI", "ShipHit.cpp: Frame %i, Gametime = %7.3f, Ship %s will die in %7.3f seconds.\n", Framecount, f2fl(Missiontime), Ships[objp->instance].ship_name, (float) delta_time/1000.0f));

	//	Make big ships have longer deathrolls.
	//	This is debug code by MK to increase the deathroll time so ships have time to evade the shockwave.
	//	Perhaps deathroll time should be specified in ships.tbl.
	float damage = ship_get_exp_damage(objp);

	if (damage >= 250.0f)
		delta_time += 3000 + (int)(damage*4.0f + 4.0f*objp->radius);

	if (Ai_info[sp->ai_index].ai_flags & AIF_KAMIKAZE)
		delta_time = 2;

	// Knossos gets 7-10 sec to die, time for "little" explosions
	if (Ship_info[sp->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
		delta_time = 7000 + (int)(frand() * 3000.0f);
		Ship_info[sp->ship_info_index].explosion_propagates = 0;
	}
	sp->death_time = sp->final_death_time = timestamp(delta_time);	// Give him 3 secs to explode

	if (sp->flags & SF_VAPORIZE) {
		// Assert(Ship_info[sp->ship_info_index].flags & SIF_SMALL_SHIP);

		// LIVE FOR 100 MS
		sp->final_death_time = timestamp(100);
	}

	//nprintf(("AI", "Time = %7.3f: final_death_time set to %7.3f\n", (float) timestamp_ticker/1000.0f, (float) sp->final_death_time/1000.0f));

	sp->pre_death_explosion_happened = 0;				// The little fireballs haven't came in yet.

	sp->next_fireball = timestamp(0);	//start one right away

	ai_deathroll_start(objp);

	// play death roll begin sound
	sp->death_roll_snd = snd_play_3d( &Snds[SND_DEATH_ROLL], &objp->pos, &View_position, objp->radius );
	if (objp == Player_obj)
		joy_ff_deathroll();

	// apply a whack
	//	rotational velocity proportional to original translational velocity, with a bit added in.
	//	Also, preserve half of original rotational velocity.

	// At standard speed (70) and standard mass (50), deathroll rotvel should be capped at DEATHROLL_ROTVEL_CAP
	// Minimum deathroll velocity is set	DEATHROLL_ROTVEL_MIN
	// At lower speed, lower death rotvel (scaled linearly)
	// At higher mass, lower death rotvel (scaled logarithmically)
	// variable scale calculates the deathroll rotational velocity magnitude
	float logval = (float) log10(objp->phys_info.mass / (0.05f*DEATHROLL_MASS_STANDARD));
	float velval = ((vm_vec_mag_quick(&objp->phys_info.vel) + 3.0f) / DEATHROLL_VELOCITY_STANDARD);
	float	p1 = (float) (DEATHROLL_ROTVEL_CAP - DEATHROLL_ROTVEL_MIN);

	rotvel_mag = (float) DEATHROLL_ROTVEL_MIN * 2.0f/(logval + 2.0f);
	rotvel_mag += (float) (p1 * velval/logval) * 0.75f;

	// set so maximum velocity from rotation is less than 200
	if (rotvel_mag*objp->radius > 150) {
		rotvel_mag = 150.0f / objp->radius;
	}

	if (sp->dock_objnum_when_dead != -1) {
		// don't change current rotvel
		sp->deathroll_rotvel = objp->phys_info.rotvel;
	} else {
		// if added rotvel is too random, we should decrease the random component, putting a const in front of the rotvel.
		sp->deathroll_rotvel = objp->phys_info.rotvel;
		sp->deathroll_rotvel.xyz.x += (frand() - 0.5f) * 2.0f * rotvel_mag;
		saturate_fabs(&sp->deathroll_rotvel.xyz.x, 0.75f*DEATHROLL_ROTVEL_CAP);
		sp->deathroll_rotvel.xyz.y += (frand() - 0.5f) * 3.0f * rotvel_mag;
		saturate_fabs(&sp->deathroll_rotvel.xyz.y, 0.75f*DEATHROLL_ROTVEL_CAP);
		sp->deathroll_rotvel.xyz.z += (frand() - 0.5f) * 6.0f * rotvel_mag;
		// make z component  2x larger than larger of x,y
		float largest_mag = max(fl_abs(sp->deathroll_rotvel.xyz.x), fl_abs(sp->deathroll_rotvel.xyz.y));
		if (fl_abs(sp->deathroll_rotvel.xyz.z) < 2.0f*largest_mag) {
			sp->deathroll_rotvel.xyz.z *= (2.0f * largest_mag / fl_abs(sp->deathroll_rotvel.xyz.z));
		}
		saturate_fabs(&sp->deathroll_rotvel.xyz.z, 0.75f*DEATHROLL_ROTVEL_CAP);
		// nprintf(("Physics", "Frame: %i rotvel_mag: %5.2f, rotvel: (%4.2f, %4.2f, %4.2f)\n", Framecount, rotvel_mag, sp->deathroll_rotvel.x, sp->deathroll_rotvel.y, sp->deathroll_rotvel.z));
	}

	
	// blow out his reverse thrusters. Or drag, same thing.
	objp->phys_info.rotdamp = (float) DEATHROLL_ROTVEL_SCALE / rotvel_mag;
	objp->phys_info.side_slip_time_const = 10000.0f;

	vm_vec_zero(&objp->phys_info.max_vel);		// make so he can't turn on his own VOLITION anymore.

	vm_vec_zero(&objp->phys_info.max_rotvel);	// make so he can't change speed on his own VOLITION anymore.

}

// called from ship_hit_kill if the ship is vaporized
void ship_vaporize(ship *shipp)
{
	object *ship_obj;

	// sanity
	Assert(shipp != NULL);
	if(shipp == NULL){
		return;
	}
	Assert((shipp->objnum >= 0) && (shipp->objnum < MAX_OBJECTS));
	if((shipp->objnum < 0) || (shipp->objnum >= MAX_OBJECTS)){
		return;
	}
	ship_obj = &Objects[shipp->objnum];

	// create debris shards
	create_vaporize_debris(ship_obj, &ship_obj->pos);
}

//	*ship_obj was hit and we've determined he's been killed!  By *other_obj!
void ship_hit_kill(object *ship_obj, object *other_obj, float percent_killed, int self_destruct)
{
	ship *sp;
	char *killer_ship_name;
	int killer_damage_percent = 0;
	object *killer_objp = NULL;

	sp = &Ships[ship_obj->instance];
	show_dead_message(ship_obj, other_obj);

	if (ship_obj == Player_obj) {
		player_died_start(other_obj);
	}

	// maybe vaporize him
	if(sp->flags & SF_VAPORIZE){
		ship_vaporize(sp);
	}

	// hehe
	extern void game_tst_mark(object *objp, ship *shipp);
	game_tst_mark(ship_obj, sp);

	// single player and multiplayer masters evaluate the scoring and kill stuff
#ifndef NO_NETWORK
	if ( !MULTIPLAYER_CLIENT && !(Game_mode & GM_DEMO_PLAYBACK)) {
#else
	if ( !(Game_mode & GM_DEMO_PLAYBACK)) {
#endif
		scoring_eval_kill( ship_obj );

		// ship is destroyed -- send this event to the mission log stuff to record this event.  Try to find who
		// killed this ship.  scoring_eval_kill above should leave the obj signature of the ship who killed
		// this guy (or a -1 if no one got the kill).
		killer_ship_name = NULL;
		killer_damage_percent = -1;
		if ( sp->damage_ship_id[0] != -1 ) {
			object *objp;
			int sig;

			sig = sp->damage_ship_id[0];
			for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
				if ( objp->signature == sig ){
					break;
				}
			}
			// if the object isn't around, the try to find the object in the list of ships which has exited			
			if ( objp != END_OF_LIST(&obj_used_list) ) {
				Assert ( (objp->type == OBJ_SHIP ) || (objp->type == OBJ_GHOST) );					// I suppose that this should be true
				killer_ship_name = Ships[objp->instance].ship_name;

				killer_objp = objp;
			} else {
				int ei;

				ei = ship_find_exited_ship_by_signature( sig );
				if ( ei != -1 ){
					killer_ship_name = Ships_exited[ei].ship_name;
				}
			}
			killer_damage_percent = (int)(sp->damage_ship[0] * 100.0f);
		}		

		if(!self_destruct){
#ifndef NO_NETWORK
			// multiplayer
			if(Game_mode & GM_MULTIPLAYER){
				char name1[256] = "";
				char name2[256] = "";
				int np_index;

				// get first name				
				np_index = multi_find_player_by_object(ship_obj);				
				if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].player != NULL)){
					strcpy(name1, Net_players[np_index].player->callsign);
				} else {
					strcpy(name1, sp->ship_name);
				}

				// argh
				if((killer_objp != NULL) || (killer_ship_name != NULL)){

					// second name
					if(killer_objp == NULL){
						strcpy(name2, killer_ship_name);
					} else {
						np_index = multi_find_player_by_object(killer_objp);
						if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].player != NULL)){
							strcpy(name2, Net_players[np_index].player->callsign);
						} else {
							strcpy(name2, killer_ship_name);
						}
					}					
				}

				mission_log_add_entry(LOG_SHIP_DESTROYED, name1, name2, killer_damage_percent);
			}
			else
#endif
			{
				// DKA: 8/23/99 allow message log in single player with no killer name
				//if(killer_ship_name != NULL){
				mission_log_add_entry(LOG_SHIP_DESTROYED, sp->ship_name, killer_ship_name, killer_damage_percent);
				//}
			}
		}

		// maybe praise the player for this kill
 		if ( (killer_damage_percent > 10) && (other_obj != NULL) && (other_obj->parent_sig == Player_obj->signature) ) {
			ship_maybe_praise_player(sp);
		}
	}

	ship_generic_kill_stuff( ship_obj, percent_killed );

#ifndef NO_NETWORK
	// mwa -- removed 2/25/98 -- why is this here?  ship_obj->flags &= ~(OF_PLAYER_SHIP);
	// if it is for observers, must deal with it a separate way!!!!
	if ( MULTIPLAYER_MASTER ) {
		// check to see if this ship needs to be respawned
		multi_respawn_check(ship_obj);		
			
		// send the kill packet to all players
		// maybe send vaporize packet to all players
		send_ship_kill_packet( ship_obj, other_obj, percent_killed, self_destruct );
	}	
#endif

	// If ship from a player wing ship has died, then maybe play a scream
	if ( !(ship_obj->flags & OF_PLAYER_SHIP) && (sp->flags & SF_FROM_PLAYER_WING) ) {
		ship_maybe_scream(sp);
	}

	// If player is dying, have wingman lament (only in single player)
	if ( (Game_mode & GM_NORMAL) && (ship_obj == Player_obj) ) {
		ship_maybe_lament();
	}
}

// function to simply explode a ship where it is currently at
void ship_self_destruct( object *objp )
{	
	Assert ( objp->type == OBJ_SHIP );

	// try and find a player
#ifndef NO_NETWORK
	if((Game_mode & GM_MULTIPLAYER) && (multi_find_player_by_object(objp) >= 0)){
		int np_index = multi_find_player_by_object(objp);
		if((np_index >= 0) && (np_index < MAX_PLAYERS) && (Net_players[np_index].player != NULL)){
			mission_log_add_entry(LOG_SELF_DESTRUCT, Net_players[np_index].player->callsign, NULL );
		} else {
			mission_log_add_entry(LOG_SELF_DESTRUCT, Ships[objp->instance].ship_name, NULL );
		}
	}
	else
#endif
	{
		mission_log_add_entry(LOG_SELF_DESTRUCT, Ships[objp->instance].ship_name, NULL );
	}
	
#ifndef NO_NETWORK
	// check to see if this ship needs to be respawned
	if(MULTIPLAYER_MASTER){
		// player ship?
		int np_index = multi_find_player_by_object(objp);
		if((np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].player != NULL)){
			char msg[512] = "";
			sprintf(msg, "%s %s", Net_players[np_index].player->callsign, XSTR("Self destructed", 1476));

			// send a message
			send_game_chat_packet(Net_player, msg, MULTI_MSG_ALL, NULL, NULL, 2);

			// printf
			if(!(Game_mode & GM_STANDALONE_SERVER)){
				HUD_printf(msg);
			}
		}
	}
#endif

	// self destruct
	ship_hit_kill(objp, NULL, 1.0f, 1);	
}

extern int Homing_hits, Homing_misses;

// Call this instead of physics_apply_whack directly to 
// deal with two ships docking properly.
void ship_apply_whack(vector *force, vector *new_pos, object *objp)
{
	if (objp->type == OBJ_SHIP)	{
		if (Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_SUPPORT | SIF_CARGO)) {
			int	docked_to_objnum;	
			ai_info *aip;

			aip = &Ai_info[Ships[objp->instance].ai_index];
		
			if (aip->ai_flags & AIF_DOCKED) {
				docked_to_objnum = aip->dock_objnum;
				if (aip->dock_objnum != -1) {
					physics_apply_whack(force, new_pos, &Objects[docked_to_objnum].phys_info, &Objects[docked_to_objnum].orient, Objects[docked_to_objnum].phys_info.mass + objp->phys_info.mass);
					return;
				}
			}
		}
	}
	if (objp == Player_obj) {
		nprintf(("Sandeep", "Playing stupid joystick effect\n"));
		vector test;
		vm_vec_unrotate(&test, force, &objp->orient);

		game_whack_apply( -test.xyz.x, -test.xyz.y );
	}
					
	physics_apply_whack(force, new_pos, &objp->phys_info, &objp->orient, objp->phys_info.mass);
}

float Skill_level_player_damage_scale[NUM_SKILL_LEVELS] = {0.25f, 0.5f, 0.65f, 0.85f, 1.0f};


// If a ship is dying and it gets hit, shorten its deathroll.
//	But, if it's a player, don't decrease below MIN_PLAYER_DEATHROLL_TIME
void shiphit_hit_after_death(object *ship_obj, float damage)
{
	float	percent_killed;
	int	delta_time, time_remaining;
	ship	*shipp = &Ships[ship_obj->instance];

	// Since the explosion has two phases (final_death_time and really_final_death_time)
	// we should only shorten the deathroll time if that is the phase we're in.
	// And you can tell by seeing if the timestamp is valid, since it gets set to
	// invalid after it does the first large explosion.
	if ( !timestamp_valid(shipp->final_death_time) )	{
		return;
	}

	// Don't adjust vaporized ship
	if (shipp->flags & SF_VAPORIZE) {
		return;
	}

	//	Don't shorten deathroll on very large ships.
	if (ship_obj->radius > BIG_SHIP_MIN_RADIUS)
		return;

	percent_killed = damage/shipp->ship_initial_hull_strength;
	if (percent_killed > 1.0f)
		percent_killed = 1.0f;

	delta_time = (int) (4 * DEATHROLL_TIME * percent_killed);
	time_remaining = timestamp_until(shipp->final_death_time);

	//nprintf(("AI", "Gametime = %7.3f, Time until %s dies = %7.3f, delta = %7.3f\n", f2fl(Missiontime), Ships[ship_obj->instance].ship_name, (float)time_remaining/1000.0f, delta_time));
	if (ship_obj->flags & OF_PLAYER_SHIP)
		if (time_remaining < MIN_PLAYER_DEATHROLL_TIME)
			return;

	// nprintf(("AI", "Subtracting off %7.3f seconds from deathroll, reducing to %7.3f\n", (float) delta_time/1000.0f, (float) (time_remaining - delta_time)/1000.0f));

	delta_time = time_remaining - delta_time;
	if (ship_obj->flags & OF_PLAYER_SHIP)
		if (delta_time < MIN_PLAYER_DEATHROLL_TIME)
			delta_time = MIN_PLAYER_DEATHROLL_TIME;

	//	Prevent bogus timestamp.
	if (delta_time < 2)
		delta_time = 2;

	shipp->final_death_time = timestamp(delta_time);	// Adjust time until explosion.
}

MONITOR( ShipHits );
MONITOR( ShipNumDied );

int maybe_shockwave_damage_adjust(object *ship_obj, object *other_obj, float *damage)
{
	ship_subsys *subsys;
	ship *shipp;
	float dist, nearest_dist = FLT_MAX;
	vector g_subobj_pos;
	float max_damage;
	shockwave *sw;

	Assert(ship_obj->type == OBJ_SHIP);

	if (!other_obj) {
		return 0;
	}

	if (other_obj->type != OBJ_SHOCKWAVE) {
		return 0;
	}

	if (!(Ship_info[Ships[ship_obj->instance].ship_info_index].flags & SIF_HUGE_SHIP)) {
		return 0;
	}

	shipp = &Ships[ship_obj->instance];
	sw = &Shockwaves[other_obj->instance];

	// find closest subsystem distance to shockwave origin
	for (subsys=GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list); subsys = GET_NEXT(subsys) ) {
		get_subsystem_world_pos(ship_obj, subsys, &g_subobj_pos);
		dist = vm_vec_dist_quick(&g_subobj_pos, &other_obj->pos);

		if (dist < nearest_dist) {
			nearest_dist = dist;
		}
	}

	// get max damage and adjust if needed to account for shockwave created from destroyed weapon
	max_damage = sw->damage;
	if (sw->flags & SW_WEAPON_KILL) {
		max_damage *= 4.0f;
	}

	// scale damage
	// floor of 25%, max if within inner_radius, linear between
	if (nearest_dist > sw->outer_radius) {
		*damage = max_damage / 4.0f;
	} else if (nearest_dist < sw->inner_radius) {
		*damage = max_damage;
	} else {
		*damage = max_damage * (1.0f - 0.75f * (nearest_dist - sw->inner_radius) / (sw->outer_radius - sw->inner_radius));
	}

	return 1;
}

// ------------------------------------------------------------------------
// ship_do_damage()
//
// Do damage assessment on a ship.    This should only be called
// internally by ship_apply_global_damage and ship_apply_local_damage
//
// 
//	input:	ship_obj		=>		object pointer for ship receiving damage
//				other_obj	=>		object pointer to object causing damage
//				hitpos		=>		impact world pos on the ship 
//				TODO:	get a better value for hitpos
//				damage		=>		damage to apply to the ship
//				quadrant	=> which part of shield takes damage, -1 if not shield hit
//				wash_damage	=>		1 if damage is done by engine wash
// Goober5000 - sanity checked this whole function in the case that other_obj is null, which
// will happen with the explosion-effect sexp
void ai_update_lethality(object *ship_obj, object *weapon_obj, float damage);
static void ship_do_damage(object *ship_obj, object *other_obj, vector *hitpos, float damage, int quadrant, int wash_damage=0)
{
//	mprintf(("doing damage\n"));

	ship *shipp;	
	float subsystem_damage = damage;			// damage to be applied to subsystems

	Assert(ship_obj->instance >= 0);
	Assert(ship_obj->type == OBJ_SHIP);
	shipp = &Ships[ship_obj->instance];

	// maybe adjust damage done by shockwave for BIG|HUGE
	maybe_shockwave_damage_adjust(ship_obj, other_obj, &damage);

	// update lethality of ship doing damage
	int update_lethality = FALSE;
	update_lethality = ((other_obj != NULL) && (other_obj->type == OBJ_WEAPON) && (other_obj->instance >= 0) && (other_obj->instance < MAX_WEAPONS));
	update_lethality = update_lethality || ((other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE) && (other_obj->instance >= 0) && (other_obj->instance < MAX_SHOCKWAVES));
	if (update_lethality) {
		ai_update_lethality(ship_obj, other_obj, damage);
	}
//mprintf(("lethality updated\n"));

	// if this is a weapon
	if((other_obj != NULL) && (other_obj->type == OBJ_WEAPON) && (other_obj->instance >= 0) && (other_obj->instance < MAX_WEAPONS)){
		damage *= weapon_get_damage_scale(&Weapon_info[Weapons[other_obj->instance].weapon_info_index], other_obj, ship_obj);
	}

	MONITOR_INC( ShipHits, 1 );

	//	Don't damage player ship in the process of warping out.
	if ( Player->control_mode >= PCM_WARPOUT_STAGE2 )	{
		if ( ship_obj == Player_obj ){
			return;
		}
	}

	if ( (other_obj != NULL) && (other_obj->type == OBJ_WEAPON) ) {		
		// for tvt and dogfight missions, don't scale damage
#ifndef NO_NETWORK
		if( (Game_mode & GM_MULTIPLAYER) && ((Netgame.type_flags & NG_TYPE_TEAM) || (Netgame.type_flags & NG_TYPE_DOGFIGHT)) ){
		} 
		else
#endif
		{
			// Do a little "skill" balancing for the player in single player and coop multiplayer
			if (ship_obj->flags & OF_PLAYER_SHIP)	{
				damage *= Skill_level_player_damage_scale[Game_skill_level];
				subsystem_damage *= Skill_level_player_damage_scale[Game_skill_level];
			}		
		}
	}
//mprintf(("skill balencing done\n"));

	// if this is not a laser, or i'm not a multiplayer client
	// apply pain to me

	// Goober5000: make sure other_obj doesn't cause a read violation!
	if (other_obj)
	{

#ifndef NO_NETWORK

if(other_obj->type == OBJ_BEAM){
Assert((beam_get_weapon_info_index(other_obj) > -1) && (beam_get_weapon_info_index(other_obj) < Num_weapon_types));
	if((other_obj != NULL) && ((Weapon_info[beam_get_weapon_info_index(other_obj)].subtype != WP_LASER) || !MULTIPLAYER_CLIENT) && (Player_obj != NULL) && (ship_obj == Player_obj)){
//mprintf(("sending pain\n"));
		ship_hit_pain(damage);
	}	
}
if(other_obj->type == OBJ_WEAPON){
Assert((Weapons[other_obj->instance].weapon_info_index > -1) && (Weapons[other_obj->instance].weapon_info_index < Num_weapon_types));
	if((other_obj != NULL) && ((Weapon_info[Weapons[other_obj->instance].weapon_info_index].subtype != WP_LASER) || !MULTIPLAYER_CLIENT) && (Player_obj != NULL) && (ship_obj == Player_obj)){
//mprintf(("sending pain\n"));
		ship_hit_pain(damage);
	}
}
#else
if(other_obj->type == OBJ_BEAM){
Assert((beam_get_weapon_info_index(other_obj) > -1) && (beam_get_weapon_info_index(other_obj) < Num_weapon_types));
	if((other_obj != NULL) && (Weapon_info[beam_get_weapon_info_index(other_obj)].subtype != WP_LASER) && (Player_obj != NULL) && (ship_obj == Player_obj)){
//mprintf(("sending pain\n"));
		ship_hit_pain(damage);
	}	
}
if(other_obj->type == OBJ_WEAPON){
Assert((Weapons[other_obj->instance].weapon_info_index > -1) && (Weapons[other_obj->instance].weapon_info_index < Num_weapons));
	if((other_obj != NULL) && (Weapon_info[Weapons[other_obj->instance].weapon_info_index].subtype != WP_LASER) && (Player_obj != NULL) && (ship_obj == Player_obj)){
//mprintf(("sending pain\n"));
		ship_hit_pain(damage);
	}
}
#endif

	}	// read violation sanity check

//mprintf(("pain sent\n"));

	// If the ship is invulnerable, do nothing
	if (ship_obj->flags & OF_INVULNERABLE)	{
		return;
	}
//mprintf(("not invulnerable\n"));

	//	if ship is already dying, shorten deathroll.
	if (shipp->flags & SF_DYING) {
		shiphit_hit_after_death(ship_obj, damage);
		return;
	}
//mprintf(("hitting a dead ship\n"));
	
	//	If we hit the shield, reduce it's strength and found
	// out how much damage is left over.
	if ( quadrant > -1 && !(ship_obj->flags & OF_NO_SHIELDS) )	{
//		mprintf(("applying damage ge to shield\n"));
		float shield_factor = -1.0f;
		int	weapon_info_index;		

		weapon_info_index = shiphit_get_damage_weapon(other_obj);
		if ( weapon_info_index >= 0 ) {
			shield_factor = Weapon_info[weapon_info_index].shield_factor;
		}

		if ( shield_factor >= 0 ) {
			damage *= shield_factor;
			subsystem_damage *= shield_factor;
		}

		if ( damage > 0 ) {
			float pre_shield = damage;

			damage = apply_damage_to_shield(ship_obj, quadrant, damage);

			if(damage > 0.0f){
				subsystem_damage *= (damage / pre_shield);
			} else {
				subsystem_damage = 0.0f;
			}
		}

		// if shield damage was increased, don't carry over leftover damage at scaled level
		if ( shield_factor > 1 ) {
			damage /= shield_factor;

			subsystem_damage /= shield_factor;
		}
	}
			
	// Apply leftover damage to the ship's subsystem and hull.
	if ( (damage > 0.0f) || (subsystem_damage > 0.0f) )	{
		int	weapon_info_index;		
//mprintf(("applying damage to hull\n"));
		float pre_subsys = subsystem_damage;
		subsystem_damage = do_subobj_hit_stuff(ship_obj, other_obj, hitpos, subsystem_damage);
		if(subsystem_damage > 0.0f){
			damage *= (subsystem_damage / pre_subsys);
		} else {
			damage = 0.0f;
		}

		// continue with damage?
		if(damage > 0.0){
			weapon_info_index = shiphit_get_damage_weapon(other_obj);
			if ( weapon_info_index >= 0 ) {
				if (Weapon_info[weapon_info_index].wi_flags & WIF_PUNCTURE) {
					damage /= 4;
				}

				damage *= Weapon_info[weapon_info_index].armor_factor;
			}

			// if ship is flagged as can not die, don't let it die
			if (ship_obj->flags & OF_GUARDIAN) {
				float min_hull_strength = 0.01f * Ships[ship_obj->instance].ship_initial_hull_strength;
				if ( (ship_obj->hull_strength - damage) < min_hull_strength ) {
					// find damage needed to take object to min hull strength
					damage = ship_obj->hull_strength - min_hull_strength;

					// make sure damage is positive
					damage = max(0, damage);
				}
			}

#ifndef NO_NETWORK
			// multiplayer clients don't do damage
			if(((Game_mode & GM_MULTIPLAYER) && MULTIPLAYER_CLIENT) || (Game_mode & GM_DEMO_PLAYBACK)){
#else
			if (Game_mode & GM_DEMO_PLAYBACK){
#endif
			} else {
				ship_obj->hull_strength -= damage;		
			}

			// let damage gauge know that player ship just took damage
			if ( Player_obj == ship_obj ) {
				hud_gauge_popup_start(HUD_DAMAGE_GAUGE, 5000);
			}
		
			// DB - removed 1/12/99 - scoring code properly bails if MULTIPLAYER_CLIENT
			// in multiplayer, if I am not the host, get out of this function here!!
			//if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
				//return;
			//}		

			if (other_obj)
			{
				switch (other_obj->type)
				{
					case OBJ_SHOCKWAVE:
						scoring_add_damage(ship_obj,other_obj,damage);
						break;
					case OBJ_ASTEROID:
						// don't call scoring for asteroids
						break;
					case OBJ_WEAPON:
						if((other_obj->parent < 0) || (other_obj->parent >= MAX_OBJECTS)){
							scoring_add_damage(ship_obj, NULL, damage);
						} else {
							scoring_add_damage(ship_obj, &Objects[other_obj->parent], damage);
						}
						break;
					case OBJ_BEAM://give kills for fighter beams-Bobboau
					  {
//						object beam_obj = Objects[beam_get_parent(other_obj)];
						if((other_obj->parent < 0) || (other_obj->parent >= MAX_OBJECTS)){
							scoring_add_damage(ship_obj, NULL, damage);
						} else {
							scoring_add_damage(ship_obj, &Objects[other_obj->parent], damage);
						}
						break;
					  }
					default:
						break;
				}
			}
//mprintf(("\n"));
			if (ship_obj->hull_strength <= 0.0f) {
//mprintf(("doing vaporizeing stuff\n"));
				MONITOR_INC( ShipNumDied, 1 );

				ship_info	*sip = &Ship_info[shipp->ship_info_index];

				// If massive beam hitting small ship, vaporize  otherwise normal damage pipeline
				// Only vaporize once
				// multiplayer clients should skip this
#ifndef NO_NETWORK
				if(!MULTIPLAYER_CLIENT)
#endif
				{
					if ( !(shipp->flags & SF_VAPORIZE) ) {
						// Only small ships can be vaporized
						if (sip->flags & (SIF_SMALL_SHIP)) {
							if ((other_obj != NULL) && (other_obj->type == OBJ_BEAM)) {
								int beam_weapon_info_index = beam_get_weapon_info_index(other_obj);
								if ( (beam_weapon_info_index > -1) && (Weapon_info[beam_weapon_info_index].wi_flags & (WIF_HUGE)) ) {								
									// Flag as vaporized
									shipp->flags |= SF_VAPORIZE;								
								}
							}
						}
					}
				}
				
				// maybe engine wash death
				if (wash_damage) {
					shipp->wash_killed = 1;
				}

				float percent_killed = -ship_obj->hull_strength/shipp->ship_initial_hull_strength;
				if (percent_killed > 1.0f){
					percent_killed = 1.0f;
				}

#ifndef NO_NETWORK
				if ( !(shipp->flags & SF_DYING) && !MULTIPLAYER_CLIENT && !(Game_mode & GM_DEMO_PLAYBACK)){  // if not killed, then kill
#else
				if ( !(shipp->flags & SF_DYING) && !(Game_mode & GM_DEMO_PLAYBACK)){  // if not killed, then kill
#endif
					ship_hit_kill(ship_obj, other_obj, percent_killed, 0);
				}
			}
		}
	}
//mprintf(("fun stuff with weapons\n"));
	// if the hitting object is a weapon, maybe do some fun stuff here
	if((other_obj != NULL) && (other_obj->type == OBJ_WEAPON))
	{
		weapon_info *wip;
		Assert(other_obj->instance >= 0);
		if(other_obj->instance < 0){
			return;
		}
		Assert(Weapons[other_obj->instance].weapon_info_index >= 0);
		if(Weapons[other_obj->instance].weapon_info_index < 0){
			return;
		}
		wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];

		// if its a leech weapon - NOTE - unknownplayer: Perhaps we should do something interesting like direct the leeched energy into the attacker ?
		if(wip->wi_flags & WIF_ENERGY_SUCK){
			// reduce afterburner fuel
			shipp->afterburner_fuel -= wip->afterburner_reduce;
			shipp->afterburner_fuel = (shipp->afterburner_fuel < 0.0f) ? 0.0f : shipp->afterburner_fuel;

			// reduce weapon energy
			shipp->weapon_energy -= wip->weapon_reduce;
			shipp->weapon_energy = (shipp->weapon_energy < 0.0f) ? 0.0f : shipp->weapon_energy;
		}
	}
}

// This gets called to apply damage when something hits a particular point on a ship.
// This assumes that whoever called this knows if the shield got hit or not.
// hitpos is in world coordinates.
// if quadrant is not -1, then that part of the shield takes damage properly.
void ship_apply_local_damage(object *ship_obj, object *other_obj, vector *hitpos, float damage, int quadrant, bool create_spark, int submodel_num, vector *hit_normal)
{
	ship *ship_p	= &Ships[ship_obj->instance];	

	//	If got hit by a weapon, tell the AI so it can react.  Only do this line in single player,
	// or if I am the master in a multiplayer game
#ifndef NO_NETWORK
	if ( other_obj->type == OBJ_WEAPON && ( !(Game_mode & GM_MULTIPLAYER) || ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)) )) {
#else
	if ( other_obj->type == OBJ_WEAPON ) {
#endif
		weapon	*wp;

		wp = &Weapons[other_obj->instance];
		//	If weapon hits ship on same team and that ship not targeted and parent of weapon not player,
		//	don't do damage.
		//	Ie, player can always do damage.  AI can only damage team if that ship is targeted.
		if (wp->target_num != ship_obj-Objects) {
			if ((ship_p->team == wp->team) && !(Objects[other_obj->parent].flags & OF_PLAYER_SHIP) ) {
				/*char	ship_name[64];

				if (other_obj->parent_type == OBJ_SHIP) {
					strcpy(ship_name, Ships[Objects[other_obj->parent].instance].ship_name);
				} else
					strcpy(ship_name, XSTR("[not a ship]",-1));
				*/
				// nprintf(("AI", "Ignoring hit on %s by weapon #%i, parent = %s\n", ship_p->ship_name, other_obj-Objects, ship_name));
				return;
			}
		}
	}
//mprintf(("not a weapon\n"));
	// only want to check the following in single player or if I am the multiplayer game server
#ifndef NO_NETWORK
	if ( !MULTIPLAYER_CLIENT && !(Game_mode & GM_DEMO_PLAYBACK) && ((other_obj->type == OBJ_SHIP) || (other_obj->type == OBJ_WEAPON)) ){
#else
	if ( !(Game_mode & GM_DEMO_PLAYBACK) && ((other_obj->type == OBJ_SHIP) || (other_obj->type == OBJ_WEAPON))) {
#endif
		ai_ship_hit(ship_obj, other_obj, hitpos, quadrant, hit_normal);
	}
//mprintf(("ai hit stuff\n"));

	//	Cut damage done on the player by 4x in training missions, but do full accredidation
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ){
		if (ship_obj == Player_obj){
			damage /= 4.0f;
		}
	}	
//mprintf(("cutting damage done stuff\n"));

	// send a packet in multiplayer -- but don't sent it if the ship is already dying.  Clients can
	// take care of dealing with ship hits after a ship is already dead.
	// if ( (MULTIPLAYER_MASTER) && !(ship_p->flags & SF_DYING) ){
		// if this is a player ship which is not mine, send him a ship hit packet
		// int np_index = multi_find_player_by_object(ship_obj);
		// if((np_index > 0) && (np_index < MAX_PLAYERS) && (np_index != MY_NET_PLAYER_NUM) && MULTI_CONNECTED(Net_players[np_index])){
			// send_ship_hit_packet( ship_obj, other_obj, hitpos, damage, quadrant, submodel_num, &Net_players[np_index]);
		// }
	// }

	// maybe tag the ship
#ifndef NO_NETWORK
	if(!MULTIPLAYER_CLIENT && (other_obj->type == OBJ_WEAPON) && (Weapon_info[Weapons[other_obj->instance].weapon_info_index].wi_flags & WIF_TAG)) {
#else
	if((other_obj->type == OBJ_WEAPON) && (Weapon_info[Weapons[other_obj->instance].weapon_info_index].wi_flags & WIF_TAG)) {
#endif
//mprintf(("doing TAG stuff\n"));
		if (Weapon_info[Weapons[other_obj->instance].weapon_info_index].tag_level == 1) {
			Ships[ship_obj->instance].tag_left = Weapon_info[Weapons[other_obj->instance].weapon_info_index].tag_time;
			Ships[ship_obj->instance].tag_total = Ships[ship_obj->instance].tag_left;
			if (Ships[ship_obj->instance].time_first_tagged == 0) {
				Ships[ship_obj->instance].time_first_tagged = Missiontime;
			}
//			mprintf(("TAGGED %s for %f seconds\n", Ships[ship_obj->instance].ship_name, Ships[ship_obj->instance].tag_left));
		} else if (Weapon_info[Weapons[other_obj->instance].weapon_info_index].tag_level == 2) {
			Ships[ship_obj->instance].level2_tag_left = Weapon_info[Weapons[other_obj->instance].weapon_info_index].tag_time;
			Ships[ship_obj->instance].level2_tag_total = Ships[ship_obj->instance].level2_tag_left;
			if (Ships[ship_obj->instance].time_first_tagged == 0) {
				Ships[ship_obj->instance].time_first_tagged = Missiontime;
			}
//			mprintf(("Level 2 TAGGED %s for %f seconds\n", Ships[ship_obj->instance].ship_name, Ships[ship_obj->instance].level2_tag_left));
		} else if (Weapon_info[Weapons[other_obj->instance].weapon_info_index].tag_level == 3) {
		// tag C creates an SSM strike, yay -Bobboau
			struct ssm_firing_info;

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Firing artillery", 1570));

			vector temp;
			vm_vec_unrotate(&temp, &ship_obj->pos, &Objects[other_obj->instance].orient);
			//vm_vec_add2(&temp, &Objects[aip->artillery_objnum].pos);			
			ssm_create(&temp, &Objects[ship_obj->instance].pos, Weapon_info[Weapons[other_obj->instance].weapon_info_index].SSM_index, NULL);				
		}
	}


#ifndef NDEBUG
	if (other_obj->type == OBJ_WEAPON) {
		weapon_info	*wip = &Weapon_info[Weapons[other_obj->instance].weapon_info_index];
		if (wip->wi_flags & WIF_HOMING) {
			Homing_hits++;
			// nprintf(("AI", " Hit!  Hits = %i/%i\n", Homing_hits, (Homing_hits + Homing_misses)));
		}
	}
#endif

	#ifndef NO_SOUND
	if ( Event_Music_battle_started == 0 )	{
		ship_hit_music(ship_obj, other_obj);
	}
	#endif
	

	if (damage < 0.0f){
		damage = 0.0f;
	}

	// evaluate any possible player stats implications
	scoring_eval_hit(ship_obj,other_obj);
//mprintf(("stats evaluated\n"));

	ship_do_damage(ship_obj, other_obj, hitpos, damage, quadrant );
//mprintf(("damage done\n"));

	// DA 5/5/98: move ship_hit_create_sparks() after do_damage() since number of sparks depends on hull strength
	// doesn't hit shield and we want sparks
	if ((quadrant == MISS_SHIELDS) && create_spark)	{
		// check if subsys destroyed
		if ( !is_subsys_destroyed(ship_p, submodel_num) ) {
			ship_hit_create_sparks(ship_obj, hitpos, submodel_num);
		}
		//fireball_create( hitpos, FIREBALL_SHIP_EXPLODE1, OBJ_INDEX(ship_obj), 0.25f );
	}
//mprintf(("totaly done\n"));

}



// This gets called to apply damage when a damaging force hits a ship, but at no 
// point in particular.  Like from a shockwave.   This routine will see if the
// shield got hit and if so, apply damage to it.
// You can pass force_center==NULL if you the damage doesn't come from anywhere,
// like for debug keys to damage an object or something.  It will 
// assume damage is non-directional and will apply it correctly.   
void ship_apply_global_damage(object *ship_obj, object *other_obj, vector *force_center, float damage )
{				
	vector tmp, world_hitpos;

	if ( force_center )	{
		int shield_quad;
		vector local_hitpos;

		// find world hitpos
		vm_vec_sub( &tmp, force_center, &ship_obj->pos );
		vm_vec_normalize_safe( &tmp );
		vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &tmp, ship_obj->radius );

		// Rotate world_hitpos into local coordinates (local_hitpos)
		vm_vec_sub(&tmp, &world_hitpos, &ship_obj->pos );
		vm_vec_rotate( &local_hitpos, &tmp, &ship_obj->orient );

		// shield_quad = quadrant facing the force_center
		shield_quad = get_quadrant(&local_hitpos);

		// world_hitpos use force_center for shockwave
		if ((other_obj != NULL) && (other_obj->type == OBJ_SHOCKWAVE) && (Ship_info[Ships[ship_obj->instance].ship_info_index].flags & SIF_HUGE_SHIP))
		{
			world_hitpos = *force_center;
		}

		// Do damage on local point		
		ship_do_damage(ship_obj, other_obj, &world_hitpos, damage, shield_quad );
	} else {
		// Since an force_center wasn't specified, this is probably just a debug key
		// to kill an object.   So pick a shield quadrant and a point on the
		// radius of the object.   
		vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &ship_obj->orient.vec.fvec, ship_obj->radius );

		for (int i=0; i<MAX_SHIELD_SECTIONS; i++){
			ship_do_damage(ship_obj, other_obj, &world_hitpos, damage/MAX_SHIELD_SECTIONS, i);
		}
	}

	// AL 3-30-98: Show flashing blast icon if player ship has taken blast damage
	if ( ship_obj == Player_obj ) {
		// only show blast icon if playing on medium skill or lower -> unknownplayer: why? I think this should be changed.
		//if ( Game_skill_level <= 2 ) {
			hud_start_text_flash(XSTR("Blast", 1428), 2000);
		//}
	}

	// evaluate any player stats scoring conditions (specifically, blasts from remotely detonated secondary weapons)
	scoring_eval_hit(ship_obj,other_obj,1);	
}

void ship_apply_wash_damage(object *ship_obj, object *other_obj, float damage)
{
	vector world_hitpos, direction_vec, rand_vec;

	// Since an force_center wasn't specified, this is probably just a debug key
	// to kill an object.   So pick a shield quadrant and a point on the
	// radius of the object
	vm_vec_rand_vec_quick(&rand_vec);
	vm_vec_scale_add(&direction_vec, &ship_obj->orient.vec.fvec, &rand_vec, 0.5f);
	vm_vec_normalize_quick(&direction_vec);
	vm_vec_scale_add( &world_hitpos, &ship_obj->pos, &direction_vec, ship_obj->radius );

	// Do damage to hull and not to shields
	ship_do_damage(ship_obj, other_obj, &world_hitpos, damage, -1, 1);

	// AL 3-30-98: Show flashing blast icon if player ship has taken blast damage
	if ( ship_obj == Player_obj ) {
		// only show blast icon if playing on medium skill or lower
		//if ( Game_skill_level <= 2 ) {
			hud_start_text_flash(XSTR("Engine Wash", 1429), 2000);
		//}
	}

	// evaluate any player stats scoring conditions (specifically, blasts from remotely detonated secondary weapons)
	scoring_eval_hit(ship_obj,other_obj,1);
}

// player pain
void ship_hit_pain(float damage)
{
	game_flash( damage/15.0f, -damage/30.0f, -damage/30.0f );

	// kill any active popups when you get hit.
	if ( Game_mode & GM_MULTIPLAYER ){
		popup_kill_any_active();
	}
}
