/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Weapon.h $
 * <insert description of file here>
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.7  2003/02/16 05:14:29  bobboau
 * added glow map nebula bug fix for d3d, someone should add a fix for glide too
 * more importantly I (think I) have fixed all major bugs with fighter beams, and added a bit of new functionality
 *
 * Revision 2.6  2002/12/10 05:43:33  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.5  2002/12/07 01:37:43  bobboau
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.4  2002/11/14 04:18:17  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.3  2002/11/11 20:11:02  phreak
 * changed around the 'weapon_info' struct to allow for custom corkscrew missiles
 *
 * Revision 2.2  2002/10/19 19:29:29  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam sheild hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 34    9/06/99 12:46a Andsager
 * Add weapon_explosion_ani LOD
 * 
 * 33    8/27/99 1:34a Andsager
 * Modify damage by shockwaves for BIG|HUGE ships.  Modify shockwave damge
 * when weapon blows up.
 * 
 * 32    8/24/99 10:47a Jefff
 * tech room weapon anims.  added tech anim field to weapons.tbl
 * 
 * 31    8/10/99 5:30p Jefff
 * Added tech_title string to weapon_info.  Changed parser accordingly.
 * 
 * 30    8/02/99 5:16p Dave
 * Bumped up weapon title string length from 32 to 48
 * 
 * 29    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 28    6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 27    6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 26    6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 25    6/04/99 2:16p Dave
 * Put in shrink effect for beam weapons.
 * 
 * 24    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 23    5/27/99 6:17p Dave
 * Added in laser glows.
 * 
 * 22    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 21    4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 20    4/28/99 3:11p Andsager
 * Stagger turret weapon fire times.  Make turrets smarter when target is
 * protected or beam protected.  Add weaopn range to weapon info struct.
 * 
 * 19    4/22/99 11:06p Dave
 * Final pass at beam weapons. Solidified a lot of stuff. All that remains
 * now is to tweak and fix bugs as they come up. No new beam weapon
 * features.
 * 
 * 18    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 17    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 16    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 15    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 14    3/23/99 11:03a Dave
 * Added a few new fields and fixed parsing code for new weapon stuff.
 * 
 * 13    3/19/99 9:52a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 12    1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 11    1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 10    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 9     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 8     1/21/99 10:45a Dave
 * More beam weapon stuff. Put in warmdown time.
 * 
 * 7     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 6     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 5     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 4     10/16/98 3:42p Andsager
 * increase MAX_WEAPONS and MAX_SHIPS and som header files
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 100   9/04/98 3:52p Dave
 * Put in validated mission updating and application during stats
 * updating.
 * 
 * 99    9/01/98 4:25p Dave
 * Put in total (I think) backwards compatibility between mission disk
 * freespace and non mission disk freespace, including pilot files and
 * campaign savefiles.
 * 
 * 98    8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 97    8/25/98 1:49p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 96    8/18/98 10:15a Dave
 * Touchups on the corkscrew missiles. Added particle spewing weapons.
 * 
 * 95    8/17/98 5:07p Dave
 * First rev of corkscrewing missiles.
 * 
 * 94    5/18/98 1:58a Mike
 * Make Phoenix not be fired at fighters (but yes bombers).
 * Improve positioning of ships in guard mode.
 * Make turrets on player ship not fire near end of support ship docking.
 * 
 * 93    5/15/98 5:37p Hoffoss
 * Added new 'tech description' fields to ship and weapon tables, and
 * added usage of it in tech room.
 * 
 * 92    5/06/98 8:06p Dave
 * Made standalone reset properly under weird conditions. Tweak
 * optionsmulti screen. Upped MAX_WEAPONS to 350. Put in new launch
 * countdown anim. Minro ui fixes/tweaks.
 * 
 * 91    5/05/98 11:15p Lawrance
 * Optimize weapon flyby sound determination
 * 
 * 90    5/05/98 4:08p Hoffoss
 * Added WIF_PLAYER_ALLOWED to check what is valid for the weaponry pool
 * in Fred.
 * 
 * 89    4/23/98 5:50p Hoffoss
 * Added tracking of techroom database list info in pilot files, added
 * sexp to add more to list, made mouse usable on ship listing in tech
 * room.
 * 
 * 88    4/19/98 9:21p Sandeep
 * 
 * 87    4/13/98 5:11p Mike
 * More improvement to countermeasure code.
 * 
 * 86    4/10/98 11:02p Mike
 * Make countermeasures less effective against aspect seekers than against
 * heat seekers.
 * Make AI ships match bank with each other when attacking a faraway
 * target.
 * Make ships not do silly loop-de-loop sometimes when attacking a faraway
 * target.
 * 
 * 85    4/10/98 4:52p Hoffoss
 * Made several changes related to tooltips.
 * 
 * 84    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 83    4/01/98 5:35p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 82    3/21/98 3:37p Mike
 * Fix/optimize attacking of big ships.
 * 
 * 81    3/15/98 9:44p Lawrance
 * Get secondary weapons for turrets working
 * 
 * 79    3/12/98 1:24p Mike
 * When weapons linked, increase firing delay.
 * Free up weapon slots for AI ships, if necessary.
 * Backside render shield effect.
 * Remove shield hit triangle if offscreen.
 * 
 * 78    3/11/98 4:34p John
 * Made weapons have glow & thrusters.
 * 
 * 77    3/04/98 8:51a Mike
 * Working on making ships better about firing Synaptic.
 * 
 * 76    3/03/98 5:12p Dave
 * 50% done with team vs. team interface issues. Added statskeeping to
 * secondary weapon blasts. Numerous multiplayer ui bug fixes.
 * 
 * 75    3/02/98 10:00a Sandeep
 * 
 * 74    2/15/98 11:28p Allender
 * allow increase in MAX_WEAPON_TYPES by chaning all bitfield references
 * 
 * 73    2/11/98 1:52p Sandeep
 * 
 * 72    2/02/98 8:47a Andsager
 * Ship death area damage applied as instantaneous damage for small ships
 * and shockwaves for large (>50 radius) ships.
 * 
 * 71    1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 70    1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 69    1/21/98 7:20p Lawrance
 * removed unused mount_size member
 * 
 * 68    1/19/98 10:01p Lawrance
 * Implement "Electronics" missiles
 * 
 * 67    1/17/98 10:04p Lawrance
 * Implementing "must lock to fire" missiles.  Fix couple bugs with rearm
 * time display.
 * 
 * 66    1/17/98 4:45p Mike
 * Better support for AI selection of secondary weapons.
 * 
 * 65    1/16/98 11:43a Mike
 * Fix countermeasures.
 * 
 * 64    1/15/98 11:13a John
 * Added code for specifying weapon trail bitmaps in weapons.tbl
 * 
 * 63    1/06/98 6:58p Lawrance
 * Add wp->turret_subsys, which gets set when a turret fires a weapon.
 * 
 * $NoKeywords: $
 */

#ifndef _WEAPON_H
#define _WEAPON_H

// use this to extend a beam to "infinity"
#define BEAM_FAR_LENGTH				30000.0f

// define moved to before includes so that we can have it available when ship.h is included below
#define MAX_WEAPON_TYPES				200

// define to compile corkscrew missiles in

#include "object/object.h"
#include "parse/parselo.h"
#include "ship/ship.h"		/* needed for ship_subsys* */
#include "graphics/2d.h"		// needed for color
#include "weapon/shockwave.h"
#include "weapon/trails.h"
#include "ship/ai.h"

#define	WP_UNUSED	-1
#define	WP_LASER		0		// PLEASE NOTE that this flag specifies ballistic primaries as well - Goober5000
#define	WP_MISSILE	1
#define	WP_BEAM		2

#define	WRT_LASER	1
#define	WRT_POF		2

//	Bitflags controlling weapon behavior
#define	MAX_WEAPON_FLAGS	18						//	Maximum number of different bit flags legal to specify in a single weapons.tbl Flags line

#define	WIF_HOMING_HEAT	(1 << 0)				//	if set, this weapon homes via seeking heat
#define	WIF_HOMING_ASPECT	(1 << 1)				//	if set, this weapon homes via chasing aspect
#define	WIF_ELECTRONICS	(1 << 2)				//	Takes out electronics systems.
#define	WIF_SPAWN			(1 << 3)				//	Spawns projectiles on detonation.
#define	WIF_REMOTE			(1 << 4)				//	Can be remotely detonated by parent.
#define	WIF_PUNCTURE		(1 << 5)				//	Punctures armor, damaging subsystems.
#define	WIF_SUPERCAP		(1 << 6)				//	This is a weapon which does supercap class damage (meaning, it applies real damage to supercap ships)
#define	WIF_AREA_EFFECT	(1 << 7)				//	Explosion has an area effect
#define	WIF_SHOCKWAVE		(1 << 8)				//	Explosion has a shockwave
#define  WIF_TURNS			(1 << 9)				// Set this if the weapon ever changes heading.  If you
															// don't set this and the weapon turns, collision detection
															// won't work, I promise!
#define	WIF_SWARM			(1 << 10)			// Missile "swarms".. ie changes heading and twists on way to target
#define	WIF_TRAIL			(1 << 11)			//	Has a trail
#define	WIF_BIG_ONLY		(1 << 12)			//	Only big ships (cruiser, capital, etc.) can arm this weapon
#define	WIF_CHILD			(1 << 13)			//	No ship can have this weapon.  It gets created by weapon detonations.
#define	WIF_BOMB				(1 << 14)			// Bomb-type missile, can be targeted
#define	WIF_HUGE				(1 << 15)			//	Huge damage (generally 500+), probably only fired at huge ships.
#define	WIF_NO_DUMBFIRE	(1	<<	16)			// Missile cannot be fired dumbfire (ie requires aspect lock)
#define  WIF_THRUSTER		(1 << 17)			// Has thruster cone and/or glow
#define	WIF_IN_TECH_DATABASE		(1 << 18)
#define	WIF_PLAYER_ALLOWED		(1 << 19)   // allowed to be on starting wing ships/in weaponry pool
#define	WIF_BOMBER_PLUS	(1 << 20)			//	Fire this missile only at a bomber or big ship.  But not a fighter.

#define	WIF_CORKSCREW		(1 << 21)			// corkscrew style missile
#define	WIF_PARTICLE_SPEW	(1 << 22)			// spews particles as it travels
#define	WIF_EMP				(1 << 23)			// weapon explodes with a serious EMP effect
#define  WIF_ENERGY_SUCK	(1 << 24)			// energy suck primary (impact effect)
#define	WIF_FLAK				(1 << 25)			// use for big-ship turrets - flak gun
#define	WIF_BEAM				(1 << 26)			// if this is a beam weapon : NOTE - VERY SPECIAL CASE
#define	WIF_TAG				(1 << 27)			// this weapon has a tag effect when it hits
#define	WIF_SHUDDER			(1 << 28)			// causes the weapon to shudder. shudder is proportional to the mass and damage of the weapon
#define	WIF_MFLASH			(1 << 29)			// has muzzle flash
#define	WIF_LOCKARM			(1 << 30)			// if the missile was fired without a lock, it does significanlty less damage on impact
#define  WIF_STREAM			(1 << 31)			// handled by "trigger down/trigger up" instead of "fire - wait - fire - wait"

#define WIF2_BALLISTIC		(1 << 0)			// ballistic primaries - Goober5000
#define WIF2_PIERCE		(1 << 1)			//shield pierceing -Bobboau

#define	WIF_HOMING					(WIF_HOMING_HEAT | WIF_HOMING_ASPECT)
#define  WIF_HURTS_BIG_SHIPS		(WIF_BOMB | WIF_BEAM | WIF_HUGE | WIF_BIG_ONLY)

#define	WEAPON_EXHAUST_DELTA_TIME	75		//	Delay in milliseconds between exhaust blobs

#define WF_LOCK_WARNING_PLAYED		(1<<0)		// set when a lock warning sound is played for the player
																//	(needed since we don't want to play multiple lock sounds)
#define WF_ALREADY_APPLIED_STATS		(1<<1)		// for use in ship_apply_local and ship_apply_global damage functions
																// so that we don't record multiple hits (stats) for one impact
#define WF_PLAYED_FLYBY_SOUND			(1<<2)		// flyby sound has been played for this weapon
#define WF_CONSIDER_FOR_FLYBY_SOUND	(1<<3)		// consider for flyby
#define WF_DEAD_IN_WATER				(1<<4)		// a missiles engines have died
#define WF_LOCKED_WHEN_FIRED			(1<<5)		// fired with a lock
#define WF_DESTROYED_BY_WEAPON		(1<<6)		// destroyed by damage from other weapon

typedef struct weapon {
	int		weapon_info_index;			// index into weapon_info array
	int		objnum;							// object number for this weapon
	int		team;								// The team of the ship that fired this
	int		species;							// The species of the ship that fired this
	float		lifeleft;						// life left on this weapon	
	int		target_num;						//	Object index of target
	int		target_sig;						//	So we know if the target is the same one we've been tracking
	float		nearest_dist;					//	nearest distance yet attained to target
	fix		creation_time;					//	time at which created, stuffed Missiontime
	int		weapon_flags;					//	bit flags defining behavior, see WF_xxxx
	object*	homing_object;					//	object this weapon is homing on.
	ship_subsys*	homing_subsys;			// subsystem this weapon is homing on
	vector	homing_pos;						// world position missile is homing on
	short		swarm_index;					// index into swarm missile info, -1 if not WIF_SWARM
	int		missile_list_index;			// index for missiles into Missile_obj_list, -1 weapon not missile
	int		trail_num;						// -1 if no trail, else index into Trails array
	ship_subsys *turret_subsys;			// points to turret that fired weapon, otherwise NULL
	int		group_id;						// Which group this is in.

	// Stuff for thruster glows
	int		thruster_bitmap;					// What frame the current thruster bitmap is at for this weapon
	float		thruster_frame;					// Used to keep track of which frame the animation should be on.
	int		thruster_glow_bitmap;			// What frame the current thruster engine glow bitmap is at for this weapon
	float		thruster_glow_frame;				// Used to keep track of which frame the engine glow animation should be on.
	float		thruster_glow_noise;				// Noise for current frame

	int		pick_big_attack_point_timestamp;	//	Timestamp at which to pick a new point to attack.
	vector	big_attack_point;				//	Target-relative location of attack point.

	int		cmeasure_ignore_objnum;		//	Ignoring this countermeasure.  It's failed to attract this weapon.
	int		cmeasure_chase_objnum;		//	Chasing this countermeasure.  Don't maybe ignore in future.

	// corkscrew info (taken out for now)
	short	cscrew_index;						// corkscrew info index

	// particle spew info
	int		particle_spew_time;			// time to spew next bunch of particles	

	// flak info
	short flak_index;							// flak info index
	int frame;				//for animated laser bitmaps
	int gframe;				//for animated laser glow bitmaps
} weapon;


#ifdef FS2_DEMO
	#define MAX_WEAPONS	100
#else
	// upped 5/6/98 from 200 - DB
	#define MAX_WEAPONS	350
#endif

// info specific to beam weapons
#define MAX_BEAM_SECTIONS				5
typedef struct beam_weapon_section_info {
	float width;							// width of the section
	int texture;							// texture bitmap
	ubyte rgba_inner[4];					// for non-textured beams
	ubyte rgba_outer[4];					// for non-textured beams
	float flicker;							// how much it flickers (0.0 to 1.0)
	float z_add;							// is this necessary?
	float tile_factor;						// texture tile factor -Bobboau
	int tile_type;							// is this beam tiled by it's length, or not
	float translation;						// makes the beam texture move -Bobboau
} beam_weapon_section_info;

typedef struct beam_weapon_info {
	int	beam_type;						// beam type
	float	beam_life;						// how long it lasts
	int	beam_warmup;					// how long it takes to warmup (in ms)
	int	beam_warmdown;					// how long it takes to warmdown (in ms)
	float	beam_muzzle_radius;			// muzzle glow radius
	int	beam_particle_count;			// beam spew particle count
	float	beam_particle_radius;		// radius of beam particles
	float	beam_particle_angle;			// angle of beam particle spew cone
	int	beam_particle_ani;			// particle ani
	float	beam_miss_factor[NUM_SKILL_LEVELS];				// magic # which makes beams miss more. by skill level
	int	beam_loop_sound;				// looping beam sound
	int	beam_warmup_sound;			// warmup sound
	int	beam_warmdown_sound;			// warmdown sound
	int	beam_num_sections;			// the # of visible "sections" on the beam
	int	beam_glow_bitmap;				// muzzle glow bitmap
	int	beam_shots;						// # of shots the beam takes
	float	beam_shrink_factor;			// what percentage of total beam lifetime when the beam starts shrinking
	float beam_shrink_pct;				// what percent/second the beam shrinks at
	beam_weapon_section_info sections[MAX_BEAM_SECTIONS];			// info on the visible sections of the beam 	
	float			range;				//how far it will shoot-Bobboau
	float			damage_threshold;	//point at wich damage will start being atenuated from 0.0 to 1.0
} beam_weapon_info;

extern weapon Weapons[MAX_WEAPONS];

#define WEAPON_TITLE_LEN			48

typedef struct weapon_info {
	char	name[NAME_LENGTH];				// name of this weapon
	char	title[WEAPON_TITLE_LEN];		// official title of weapon (used by tooltips)
	char	*desc;								// weapon's description (used by tooltips)
	int	subtype;								// one of the WP_* macros above
	int	render_type;						//	rendering method, laser, pof, avi
	char	pofbitmap_name[NAME_LENGTH];	// Name of the pof representing this if POF, or bitmap filename if bitmap
	int	model_num;							// modelnum of weapon -- -1 if no model

	char	*tech_desc;								// weapon's description (in tech database)
	char	tech_anim_filename[NAME_LENGTH];	// weapon's tech room animation
	char	tech_title[NAME_LENGTH];			// weapon's name (in tech database)

	int	laser_bitmap;						// Which bitmap renders for laser, -1 if none
	int	laser_bitmap_nframes;						//number of frames, 1 if it is not an ani 
	int	laser_bitmap_fps;					//framerate, irellivent ifnframes is < 2 
	int	laser_glow_bitmap;				// optional bitmap for laser glow
	int	laser_glow_bitmap_nframes;				//number of frames, 1 if it is not an ani
	int	laser_glow_bitmap_fps;				//framerate, irellivent ifnframes is < 2
	float laser_length;
	color	laser_color_1;						// for cycling between glow colors
	color	laser_color_2;						// for cycling between glow colors
	float	laser_head_radius, laser_tail_radius;

	float	max_speed;							// initial speed of the weapon
	float mass;									// mass of the weapon
	float fire_wait;							// fire rate -- amount of time before you can refire the weapon
	float	blast_force;						// force this weapon exhibits when hitting an object
	float	damage;								//	damage of weapon (for missile, damage within inner radius)
	float	inner_radius, outer_radius;	// damage radii for missiles (0 means impact only)
	float	shockwave_speed;					// speed of shockwave ( 0 means none )
	float	armor_factor, shield_factor, subsystem_factor;	//	in 0.0..2.0, scale of damage done to type of thing
	float	lifetime;							//	How long this thing lives.
	float energy_consumed;					// Energy used up when weapon is fired
	int	wi_flags;							//	bit flags defining behavior, see WIF_xxxx
	int wi_flags2;							// stupid int wi_flags, only 32 bits... argh - Goober5000
	float turn_time;
	float	cargo_size;							// cargo space taken up by individual weapon (missiles only)
	float rearm_rate;							// rate per second at which secondary weapons are loaded during rearming
	float	weapon_range;						// max range weapon can be effectively fired.  (May be less than life * speed)

	// spawn weapons
	short	spawn_type;							//	Type of weapon to spawn when detonated.
	short	spawn_count;						//	Number of weapons of spawn_type to spawn.

	// swarm count
	short swarm_count;						// how many swarm missiles are fired for this weapon

	//	Specific to ASPECT homing missiles.
	float	min_lock_time;						// minimum time (in seconds) to achieve lock
	int	lock_pixels_per_sec;				// pixels/sec moved while locking
	int	catchup_pixels_per_sec;			// pixels/sec moved while catching-up for a lock				
	int	catchup_pixel_penalty;			// number of extra pixels to move while locking as a penalty for catching up for a lock			

	//	Specific to HEAT homing missiles.
	float	fov;

	int	launch_snd;
	int	impact_snd;
	int	flyby_snd;							//	whizz-by sound, transmitted through weapon's portable atmosphere.
	
	// Specific to weapons with TRAILS:
	trail_info tr_info;			

	char	icon_filename[NAME_LENGTH];	// filename for icon that is displayed in weapon selection
	char	anim_filename[NAME_LENGTH];	// filename for animation that plays in weapon selection

	int	impact_weapon_expl_index;		// Index into Weapon_expl_info of which ANI should play when this thing impacts something
	float	impact_explosion_radius;		// How big the explosion should be

	// EMP effect
	float emp_intensity;					// intensity of the EMP effect
	float emp_time;						// time of the EMP effect

	// Energy suck effect
	float weapon_reduce;					// how much energy removed from weapons systems
	float afterburner_reduce;			// how much energy removed from weapons systems

	// Beam weapon effect	
	beam_weapon_info	b_info;			// this must be valid if the weapon is a beam weapon WIF_BEAM or WIF_BEAM_SMALL

	// tag stuff
	float	tag_time;						// how long the tag lasts		
	int tag_level;							// tag level (1 - 3)

	// muzzle flash
	int muzzle_flash;						// muzzle flash stuff
	
	// SSM
	int SSM_index;							// wich entry in the SSM,tbl it uses -Bobboau

	int Weapon_particle_spew_count;
	int Weapon_particle_spew_time;
	float Weapon_particle_spew_vel;
	float Weapon_particle_spew_radius;
	float Weapon_particle_spew_lifetime;
	float Weapon_particle_spew_scale;
	int Weapon_particle_spew_bitmap;
	char Weapon_particle_spew_bitmap_name[NAME_LENGTH];			//p_spew stuff -Bobboau
	int Weapon_particle_spew_nframes;
	int Weapon_particle_spew_fps;


	// Corkscrew info - phreak 11/9/02
	int cs_num_fired;
	float cs_radius;
	float cs_twist;
	int cs_crotate;
	int cs_delay;

	int decal_texture;
	int decal_backface_texture;
	float decal_rad;

} weapon_info;

// Data structure to track the active missiles
typedef struct missile_obj {
	missile_obj *next, *prev;
	int			flags, objnum;
} missile_obj;
extern missile_obj Missile_obj_list;

extern weapon_info Weapon_info[MAX_WEAPON_TYPES];

extern int Num_weapon_types;			// number of weapons in the game
extern int First_secondary_index;
extern int Num_weapons;


extern int Num_player_weapon_precedence;				// Number of weapon types in Player_weapon_precedence
extern int Player_weapon_precedence[MAX_WEAPON_TYPES];	// Array of weapon types, precedence list for player weapon selection
extern char	*Weapon_names[MAX_WEAPON_TYPES];

#define WEAPON_INDEX(wp)				(wp-Weapons)
#define WEAPON_INFO_INDEX(wip)		(wip-Weapon_info)


int weapon_info_lookup(char *name);
void weapon_init();					// called at game startup
void weapon_level_init();			// called before the start of each level
void weapon_render(object * obj);
void weapon_delete( object * obj );
void weapon_process_pre( object *obj, float frame_time);
void weapon_process_post( object *obj, float frame_time);

// Group_id:  If you should quad lasers, they should all have the same group id.  
// This will be used to optimize lighting, since each group only needs to cast one light.
// Call this to get a new group id, then pass it to each weapon_create call for all the
// weapons in the group.   Number will be between 0 and WEAPON_MAX_GROUP_IDS and will
// get reused.
int weapon_create_group_id();

// How many unique groups of weapons there can be at one time. 
#define WEAPON_MAX_GROUP_IDS 256

// Passing a group_id of -1 means it isn't in a group.  See weapon_create_group_id for more 
// help on weapon groups.
int weapon_create( vector * pos, matrix * orient, int weapon_type, int parent_obj, int secondary_flag, int group_id=-1, int is_locked = 0);
void weapon_set_tracking_info(int weapon_objnum, int parent_objnum, int target_objnum, int target_is_locked = 0, ship_subsys *target_subsys = NULL);

// for weapons flagged as particle spewers, spew particles. wheee
void weapon_maybe_spew_particle(object *obj);


void weapon_hit( object * weapon_obj, object * other_obj, vector * hitpos );
int weapon_name_lookup(char *name);
void spawn_child_weapons( object *objp );

// call to detonate a weapon. essentially calls weapon_hit() with other_obj as NULL, and sends a packet in multiplayer
void weapon_detonate(object *objp);

void	weapon_area_apply_blast(vector *force_apply_pos, object *ship_objp, vector *blast_pos, float blast, int make_shockwave);
int	weapon_area_calc_damage(object *objp, vector *pos, float inner_rad, float outer_rad, float max_blast, float max_damage,
										float *blast, float *damage, float limit);

void	missile_obj_list_rebuild();	// called by save/restore code only
missile_obj *missile_obj_return_address(int index);
void find_homing_object_cmeasures();

// THE FOLLOWING FUNCTION IS IN SHIP.CPP!!!!
// JAS - figure out which thruster bitmap will get rendered next
// time around.  ship_render needs to have shipp->thruster_bitmap set to
// a valid bitmap number, or -1 if we shouldn't render thrusters.
// This does basically the same thing as ship_do_thruster_frame, except it
// operates on a weapon.   This is in the ship code because it needs
// the same thruster animation info as the ship stuff, and I would
// rather extern this one function than all the thruster animation stuff.
void ship_do_weapon_thruster_frame( weapon *weaponp, object *objp, float frametime );

// call to get the "color" of the laser at the given moment (since glowing lasers can cycle colors)
void weapon_get_laser_color(color *c, object *objp);

void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vector *hitpos);

// return a scale factor for damage which should be applied for 2 collisions
float weapon_get_damage_scale(weapon_info *wip, object *wep, object *target);

// return handle to explosion ani
int weapon_get_expl_handle(int weapon_expl_index, vector *pos, float size);

#endif
