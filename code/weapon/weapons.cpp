/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



//#include <stdlib.h>


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
#include "weapon/beam.h"	// for BEAM_TYPE_? definitions
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "parse/scripting.h"


#ifndef NDEBUG
int Weapon_flyby_sound_enabled = 1;
DCF_BOOL( weapon_flyby, Weapon_flyby_sound_enabled )
#endif

static int Weapon_flyby_sound_timer;	

weapon Weapons[MAX_WEAPONS];
weapon_info Weapon_info[MAX_WEAPON_TYPES];

#define		MISSILE_OBJ_USED	(1<<0)			// flag used in missile_obj struct
#define		MAX_MISSILE_OBJS	MAX_WEAPONS		// max number of missiles tracked in missile list
missile_obj Missile_objs[MAX_MISSILE_OBJS];	// array used to store missile object indexes
missile_obj Missile_obj_list;						// head of linked list of missile_obj structs

//WEAPON SUBTYPE STUFF
char *Weapon_subtype_names[] = {
	"Laser",
	"Missile",
	"Beam"
};
int Num_weapon_subtypes = sizeof(Weapon_subtype_names)/sizeof(char *);

flag_def_list Burst_fire_flags[] = {
	{ "fast firing",		WBF_FAST_FIRING,		0 },
	{ "random length",		WBF_RANDOM_LENGTH,		0 }
};

int Num_burst_fire_flags = sizeof(Burst_fire_flags)/sizeof(flag_def_list);

weapon_explosions Weapon_explosions;

SCP_vector<lod_checker> LOD_checker;

int Num_weapon_types = 0;

int Num_weapons = 0;
int Weapons_inited = 0;
int Weapon_expl_initted = 0;

int laser_model_inner = -1;
int laser_model_outer = -1;

int missile_model = -1;

char	*Weapon_names[MAX_WEAPON_TYPES];

int     First_secondary_index = -1;
int		Default_cmeasure_index = -1;

static int *used_weapons = NULL;

int	Num_spawn_types = 0;
char **Spawn_names = NULL;

int Num_player_weapon_precedence;				// Number of weapon types in Player_weapon_precedence
int Player_weapon_precedence[MAX_WEAPON_TYPES];	// Array of weapon types, precedence list for player weapon selection

// Used to avoid playing too many impact sounds in too short a time interval.
// This will elimate the odd "stereo" effect that occurs when two weapons impact at 
// nearly the same time, like from a double laser (also saves sound channels!)
#define	IMPACT_SOUND_DELTA	50		// in milliseconds
int		Weapon_impact_timer;			// timer, initialized at start of each mission

// energy suck defines
#define ESUCK_DEFAULT_WEAPON_REDUCE				(10.0f)
#define ESUCK_DEFAULT_AFTERBURNER_REDUCE		(10.0f)

// scale factor for supercaps taking damage from weapons which are not "supercap" weapons
#define SUPERCAP_DAMAGE_SCALE			0.25f

// scale factor for big ships getting hit by flak
#define FLAK_DAMAGE_SCALE				0.05f

//default time of a homing weapon to not home
#define HOMING_DEFAULT_FREE_FLIGHT_TIME	0.25f

// time delay between each swarm missile that is fired
#define SWARM_MISSILE_DELAY				150

extern int compute_num_homing_objects(object *target_objp);

extern void fs2netd_add_table_validation(char *tblname);


weapon_explosions::weapon_explosions()
{
	ExplosionInfo.clear();
}

int weapon_explosions::GetIndex(char *filename)
{
	if ( filename == NULL ) {
		Int3();
		return -1;
	}

	for (uint i = 0; i < ExplosionInfo.size(); i++) {
		if ( !stricmp(ExplosionInfo[i].lod[0].filename, filename)) {
			return i;
		}
	}

	return -1;
}

int weapon_explosions::Load(char *filename, int expected_lods)
{
	char name_tmp[MAX_FILENAME_LEN] = "";
	int bitmap_id = -1;
	int nframes, nfps;
	weapon_expl_info new_wei;

	Assert( expected_lods <= MAX_WEAPON_EXPL_LOD );

	//Check if it exists
	int idx = GetIndex(filename);

	if (idx != -1)
		return idx;

	new_wei.lod_count = 1;

	strcpy_s(new_wei.lod[0].filename, filename);
	new_wei.lod[0].bitmap_id = bm_load_animation(filename, &new_wei.lod[0].num_frames, &new_wei.lod[0].fps, NULL, 1);

	if (new_wei.lod[0].bitmap_id < 0) {
		Warning(LOCATION, "Weapon explosion '%s' does not have an LOD0 anim!", filename);

		// if we don't have the first then it's only safe to assume that the rest are missing or not usable
		return -1;
	}

	// 2 chars for the lod, 4 for the extension that gets added automatically
	if ( MAX_FILENAME_LEN - (strlen(filename) > 6) ) {
		for (idx = 1; idx < expected_lods; idx++) {
			sprintf(name_tmp, "%s_%d", filename, idx);

			bitmap_id = bm_load_animation(name_tmp, &nframes, &nfps, NULL, 1);

			if (bitmap_id > 0) {
				strcpy_s(new_wei.lod[idx].filename, name_tmp);
				new_wei.lod[idx].bitmap_id = bitmap_id;
				new_wei.lod[idx].num_frames = nframes;
				new_wei.lod[idx].fps = nfps;

				new_wei.lod_count++;
			} else {
				break;
			}
		}

		if (new_wei.lod_count != expected_lods)
			Warning(LOCATION, "For '%s', %i of %i LODs are missing!", filename, expected_lods - new_wei.lod_count, expected_lods);
	}
	else {
		Warning(LOCATION, "Filename '%s' is too long to have any LODs.", filename);
	}

	ExplosionInfo.push_back( new_wei );

	return (int)(ExplosionInfo.size() - 1);
}

void weapon_explosions::PageIn(int idx)
{
	int i;

	if ( (idx < 0) || (idx >= (int)ExplosionInfo.size()) )
		return;

	weapon_expl_info *wei = &ExplosionInfo[idx];

	for ( i = 0; i < wei->lod_count; i++ ) {
		if ( wei->lod[i].bitmap_id >= 0 ) {
			bm_page_in_xparent_texture( wei->lod[i].bitmap_id, wei->lod[i].num_frames );
		}
	}
}

int weapon_explosions::GetAnim(int weapon_expl_index, vec3d *pos, float size)
{
	if ( (weapon_expl_index < 0) || (weapon_expl_index >= (int)ExplosionInfo.size()) )
		return -1;

	//Get our weapon expl for the day
	weapon_expl_info *wei = &ExplosionInfo[weapon_expl_index];

	if (wei->lod_count == 1)
		return wei->lod[0].bitmap_id;

	// now we have to do some work
	vertex v;
	int x, y, w, h, bm_size;
	int must_stop = 0;
	int best_lod = 1;
	int behind = 0;

	// start the frame
	extern int G3_count;

	if(!G3_count){
		g3_start_frame(1);
		must_stop = 1;
	}
	g3_set_view_matrix(&Eye_position, &Eye_matrix, Eye_fov);

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
	if (behind)
		best_lod++;

	// end the frame
	if (must_stop)
		g3_end_frame();

	best_lod = MIN(best_lod, wei->lod_count - 1);
	Assert( (best_lod >= 0) && (best_lod < MAX_WEAPON_EXPL_LOD) );

	return wei->lod[best_lod].bitmap_id;
}


void parse_weapon_expl_tbl(char *filename)
{
	int rval;
	uint i;
	lod_checker lod_check;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();		

	required_string("#Start");
	while (required_string_either("#End","$Name:"))
	{
		memset( &lod_check, 0, sizeof(lod_checker) );

		// base filename
		required_string("$Name:");
		stuff_string(lod_check.filename, F_NAME, MAX_FILENAME_LEN);

		//Do we have an LOD num
		if (optional_string("$LOD:"))
		{
			stuff_int(&lod_check.num_lods);
		}

		// only bother with this if we have 1 or more lods and less than max lods,
		// otherwise the stardard level loading will take care of the different effects
		if ( (lod_check.num_lods > 0) || (lod_check.num_lods < MAX_WEAPON_EXPL_LOD) ) {
			// name check, update lod count if it already exists
			for (i = 0; i < LOD_checker.size(); i++) {
				if ( !stricmp(LOD_checker[i].filename, lod_check.filename) ) {
					LOD_checker[i].num_lods = lod_check.num_lods;
				}
			}

			// old entry not found, add new entry
			if ( i == LOD_checker.size() ) {
				LOD_checker.push_back(lod_check);
			}
		}
	}
	required_string("#End");

	// close localization
	lcl_ext_close();
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
int weapon_info_lookup(const char *name)
{
	// bogus
	if (name == NULL)
		return -1;

	for (int i=0; i<Num_weapon_types; i++)
		if (!stricmp(name, Weapon_info[i].name))
			return i;

	return -1;
}

#define DEFAULT_WEAPON_SPAWN_COUNT	10

//	Parse the weapon flags.
void parse_wi_flags(weapon_info *weaponp, int wi_flags, int wi_flags2)
{
	//Make sure we HAVE flags :p
	if(!optional_string("$Flags:"))
		return;

	char	weapon_strings[MAX_WEAPON_FLAGS][NAME_LENGTH];
	int	num_strings;

	num_strings = stuff_string_list(weapon_strings, MAX_WEAPON_FLAGS);

	if (optional_string("+override")) {
		// reseting the flag values if set to override the existing flags
		weaponp->wi_flags = wi_flags;
		weaponp->wi_flags2 = wi_flags2;
	}

	bool set_pierce = false;
	bool set_nopierce = false;
	
	for (int i=0; i<num_strings; i++) {
		if (!strnicmp(NOX("Spawn"), weapon_strings[i], 5))
		{
            if (weaponp->num_spawn_weapons_defined < MAX_SPAWN_TYPES_PER_WEAPON)
			{
				//We need more spawning slots
				//allocate in slots of 10
				if((Num_spawn_types % 10) == 0) {
					Spawn_names = (char **)vm_realloc(Spawn_names, (Num_spawn_types + 10) * sizeof(*Spawn_names));
				}

				int	skip_length, name_length;
				char	*temp_string;

				temp_string = weapon_strings[i];

				weaponp->wi_flags |= WIF_SPAWN;
				weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_type = (short)Num_spawn_types;
				skip_length = strlen(NOX("Spawn")) + strspn(&temp_string[strlen(NOX("Spawn"))], NOX(" \t"));
				char *num_start = strchr(&temp_string[skip_length], ',');
				if (num_start == NULL) {
					weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count = DEFAULT_WEAPON_SPAWN_COUNT;
					name_length = 999;
				} else {
					weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count = (short)atoi(num_start+1);
					name_length = num_start - temp_string - skip_length;
				}

                weaponp->total_children_spawned += weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count;

				Spawn_names[Num_spawn_types] = vm_strndup( &weapon_strings[i][skip_length], name_length );
				Num_spawn_types++;
                weaponp->num_spawn_weapons_defined++;
			} else {
				Warning(LOCATION, "Illegal to have more than %d spawn types for one weapon.\nIgnoring weapon %s", MAX_SPAWN_TYPES_PER_WEAPON, weaponp->name);
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
		else if (!stricmp(NOX("Electronics"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_ELECTRONICS;
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
		else if (!stricmp(NOX("countermeasure"), weapon_strings[i]))
			weaponp->wi_flags |= WIF_CMEASURE;
		else if (!stricmp(NOX("ballistic"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_BALLISTIC;
		else if (!stricmp(NOX("pierce shields"), weapon_strings[i]))
			set_pierce = true;
		else if (!stricmp(NOX("no pierce shields"), weapon_strings[i]))	// only for beams
			set_nopierce = true;
		else if (!stricmp(NOX("local ssm"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_LOCAL_SSM;
		else if (!stricmp(NOX("tagged only"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_TAGGED_ONLY;
		else if (!stricmp(NOX("beam no whack"), weapon_strings[i]))
		{
			Warning(LOCATION, "The \"beam no whack\" flag has been deprecated.  Set the beam's mass to 0 instead.  This has been done for you.\n");
			weaponp->mass = 0.0f;
		}
		else if (!stricmp(NOX("cycle"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_CYCLE;
		else if (!stricmp(NOX("small only"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SMALL_ONLY;
		else if (!stricmp(NOX("same turret cooldown"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SAME_TURRET_COOLDOWN;
		else if (!stricmp(NOX("apply no light"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_MR_NO_LIGHTING;
		else if (!stricmp(NOX("training"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_TRAINING;
		else if (!stricmp(NOX("smart spawn"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SMART_SPAWN;
		else if (!stricmp(NOX("inherit parent target"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_INHERIT_PARENT_TARGET;
		else if (!stricmp(NOX("no emp kill"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_NO_EMP_KILL;
		else if (!stricmp(NOX("untargeted heat seeker"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_UNTARGETED_HEAT_SEEKER;
		else if (!stricmp(NOX("no radius doubling"), weapon_strings[i])) {
			if (weaponp->wi_flags & WIF_BOMB) {	
				weaponp->wi_flags2 |= WIF2_HARD_TARGET_BOMB;
			}
			else {
				Warning(LOCATION, "Weapon %s is not a bomb but has \"no radius doubling\" set. Ignoring this flag", weaponp->name);
			}
		}
		else if (!stricmp(NOX("no subsystem homing"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_NON_SUBSYS_HOMING;
		else if (!stricmp(NOX("no lifeleft penalty"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_NO_LIFE_LOST_IF_MISSED;
		else if (!stricmp(NOX("can be targeted"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_CAN_BE_TARGETED;
		else if (!stricmp(NOX("show on radar"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SHOWN_ON_RADAR;
		else if (!stricmp(NOX("show friendly on radar"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_SHOW_FRIENDLY;
		else if (!stricmp(NOX("capital+"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_CAPITAL_PLUS;
		else if (!stricmp(NOX("chain external model fps"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_EXTERNAL_WEAPON_FP;
		else if (!stricmp(NOX("external model launcher"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_EXTERNAL_WEAPON_LNCH;
		else if (!stricmp(NOX("takes blast damage"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_TAKES_BLAST_DAMAGE;
		else if (!stricmp(NOX("takes shockwave damage"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_TAKES_SHOCKWAVE_DAMAGE;
		else if (!stricmp(NOX("hide from radar"), weapon_strings[i]))
			weaponp->wi_flags2 |= WIF2_DONT_SHOW_ON_RADAR;
		else
			Warning(LOCATION, "Bogus string in weapon flags: %s\n", weapon_strings[i]);
	}

	// Goober5000 - fix up pierce/nopierce flags, per Mantis #2442
	if (set_pierce)
		weaponp->wi_flags2 |= WIF2_PIERCE_SHIELDS;
	if (set_nopierce)
		weaponp->wi_flags2 &= ~WIF2_PIERCE_SHIELDS;

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

	if (!(weaponp->wi_flags & WIF_SPAWN) && (weaponp->wi_flags2 & WIF2_SMART_SPAWN))
	{
		Warning(LOCATION,"\"smart spawn\" flag used without \"spawn\" flag in %s\n",weaponp->name);
	}

	if ((weaponp->wi_flags2 & WIF2_INHERIT_PARENT_TARGET) && (!(weaponp->wi_flags & WIF_CHILD)))
	{
		Warning(LOCATION,"Weapon %s has the \"inherit parent target\" flag, but not the \"child\" flag.  No changes in behavior will occur.", weaponp->name);
	}
}

void parse_shockwave_info(shockwave_create_info *sci, char *pre_char)
{
	char buf[NAME_LENGTH];

	sprintf(buf, "%sShockwave damage:", pre_char);
	if(optional_string(buf)) {
		stuff_float(&sci->damage);
	}

	sprintf(buf, "%sShockwave damage type:", pre_char);
	if(optional_string(buf)) {
		stuff_string(buf, F_NAME, NAME_LENGTH);
		sci->damage_type_idx_sav = damage_type_add(buf);
		sci->damage_type_idx = sci->damage_type_idx_sav;
	}

	sprintf(buf, "%sBlast Force:", pre_char);
	if(optional_string(buf)) {
		stuff_float(&sci->blast);
	}

	sprintf(buf, "%sInner Radius:", pre_char);
	if(optional_string(buf)) {
		stuff_float(&sci->inner_rad);
	}

	sprintf(buf, "%sOuter Radius:", pre_char);
	if(optional_string(buf)) {
		stuff_float(&sci->outer_rad);
	}

	sprintf(buf, "%sShockwave Speed:", pre_char);
	if(optional_string(buf)) {
		stuff_float(&sci->speed);
	}

	sprintf(buf, "%sShockwave Rotation:", pre_char);
	if(optional_string(buf)) {
		float angs[3];
		stuff_float_list(angs, 3);
		for(int i = 0; i < 3; i++)
		{
			angs[i] = angs[i] * (PI2/180.0f);
			while(angs[i] < 0)
			{
				angs[i] += PI2;
			}
			while(angs[i] > PI2)
			{
				angs[i] -= PI2;
			}
		}
		sci->rot_angles.p = angs[0];
		sci->rot_angles.b = angs[1];
		sci->rot_angles.h = angs[2];
	}

	sprintf(buf, "%sShockwave Model:", pre_char);
	if(optional_string(buf)) {
		stuff_string(sci->pof_name, F_NAME, MAX_FILENAME_LEN);
	}

	sprintf(buf, "%sShockwave Name:", pre_char);
	if(optional_string(buf)) {
		stuff_string(sci->name, F_NAME, MAX_FILENAME_LEN);
	}
}

void init_weapon_entry(int weap_info_index)
{
	Assert(weap_info_index > -1 && weap_info_index < MAX_WEAPON_TYPES);
	weapon_info *wip = &Weapon_info[weap_info_index];
	int i, j;
	
	wip->wi_flags = WIF_DEFAULT_VALUE;
	wip->wi_flags2 = WIF2_DEFAULT_VALUE;
	
	wip->subtype = WP_UNUSED;
	wip->render_type = WRT_NONE;
	
	wip->title[0] = 0;
	wip->desc = NULL;
	wip->tech_title[0] = 0;
	wip->tech_anim_filename[0] = 0;
	wip->tech_desc = NULL;
	
	wip->tech_model[0] = '\0';
	
	wip->hud_filename[0] = '\0';
	wip->hud_image_index = -1;
	
	wip->pofbitmap_name[0] = '\0';

	wip->model_num = -1;
	wip->hud_target_lod = -1;
	wip->num_detail_levels = -1;
	for ( i = 0; i < MAX_MODEL_DETAIL_LEVELS; i++ )
	{
		wip->detail_distance[i] = -1;
	}

	generic_anim_init(&wip->laser_bitmap);
	generic_anim_init(&wip->laser_glow_bitmap);

	gr_init_color(&wip->laser_color_1, 255, 255, 255);
	gr_init_color(&wip->laser_color_2, 0, 0, 0);
	
	wip->laser_length = 10.0f;
	wip->laser_head_radius = 1.0f;
	wip->laser_tail_radius = 1.0f;
	
	wip->external_model_name[0] = '\0';
	wip->external_model_num = -1;
	
	wip->weapon_submodel_rotate_accell = 10.0f;
	wip->weapon_submodel_rotate_vel = 0.0f;
	
	wip->mass = 1.0f;
	wip->max_speed = 10.0f;
	wip->free_flight_time = 0.0f;
	wip->free_flight_speed = 0.25f;
	wip->fire_wait = 1.0f;
	wip->damage = 0.0f;
	
	wip->damage_type_idx = -1;
	wip->damage_type_idx_sav = -1;

	wip->arm_time = 0;
	wip->arm_dist = 0.0f;
	wip->arm_radius = 0.0f;
	wip->det_range = 0.0f;
	wip->det_radius = 0.0f;
	
	wip->armor_factor = 1.0f;
	wip->shield_factor = 1.0f;
	wip->subsystem_factor = 1.0f;
	
	wip->life_min = -1.0f;
	wip->life_max = -1.0f;
	wip->lifetime = 1.0f;
	wip->energy_consumed = 0.0f;

	wip->cargo_size = 1.0f;
	
	wip->turn_time = 1.0f;
	wip->fov = 0;				//should be cos(pi), not pi
	
	wip->min_lock_time = 0.0f;
	wip->lock_pixels_per_sec = 50;
	wip->catchup_pixels_per_sec = 50;
	wip->catchup_pixel_penalty = 50;
	wip->seeker_strength = 1.0f;
	
	wip->swarm_count = -1;
	// *Default is 150  -Et1
	wip->SwarmWait = SWARM_MISSILE_DELAY;
	
	wip->launch_snd = -1;
	wip->impact_snd = -1;
	wip->disarmed_impact_snd = -1;
	wip->flyby_snd = -1;
	
	wip->rearm_rate = 1.0f;
	
	wip->weapon_range = 999999999.9f;
	// *Minimum weapon range, default is 0 -Et1
	wip->WeaponMinRange = 0.0f;

    wip->num_spawn_weapons_defined = 0;

    for (i = 0; i < MAX_SPAWN_TYPES_PER_WEAPON; i++)
    {
	    wip->spawn_info[i].spawn_type  = -1;
	    wip->spawn_info[i].spawn_angle = 180;
        wip->spawn_info[i].spawn_count = DEFAULT_WEAPON_SPAWN_COUNT;
    }
	
	// Trails
	wip->tr_info.pt = vmd_zero_vector;
	wip->tr_info.w_start = 1.0f;
	wip->tr_info.w_end = 1.0f;
	wip->tr_info.a_start = 1.0f;
	wip->tr_info.a_end = 1.0f;
	wip->tr_info.max_life = 1.0f;
	wip->tr_info.stamp = 0;
	generic_bitmap_init(&wip->tr_info.texture, NULL);
	wip->tr_info.n_fade_out_sections = 0;

	wip->icon_filename[0] = 0;

	wip->anim_filename[0] = 0;

	wip->impact_explosion_radius = 1.0f;
	wip->impact_weapon_expl_index = -1;

	wip->dinky_impact_explosion_radius = 1.0f;
	wip->dinky_impact_weapon_expl_index = -1;

	wip->flash_impact_weapon_expl_index = -1;
	wip->flash_impact_explosion_radius = 0.0f;

	wip->piercing_impact_explosion_radius = 0.0f;
	wip->piercing_impact_particle_count = 0;
	wip->piercing_impact_particle_life = 0.0f;
	wip->piercing_impact_particle_velocity = 0.0f;
	wip->piercing_impact_weapon_expl_index = -1;
	wip->piercing_impact_particle_back_velocity = 0.0f;
	wip->piercing_impact_particle_variance = 0.0f;

	wip->muzzle_flash = -1;

	wip->emp_intensity = EMP_DEFAULT_INTENSITY;
	wip->emp_time = EMP_DEFAULT_TIME;	// Goober5000: <-- Look!  I fixed a Volition bug!  Gimme $5, Dave!
	wip->weapon_reduce = ESUCK_DEFAULT_WEAPON_REDUCE;
	wip->afterburner_reduce = ESUCK_DEFAULT_AFTERBURNER_REDUCE;

	//customizeable corkscrew stuff
	wip->cs_num_fired=4;
	wip->cs_radius=1.25f;
	wip->cs_delay=30;
	wip->cs_crotate=1;
	wip->cs_twist=5.0f;
	
	wip->elec_time=8000;
	wip->elec_eng_mult=1.0f;
	wip->elec_weap_mult=1.0f;
	wip->elec_beam_mult=1.0f;
	wip->elec_sensors_mult=1.0f;
	wip->elec_randomness=2000;
	wip->elec_use_new_style=0;

	wip->lssm_warpout_delay=0;			//delay between launch and warpout (ms)
	wip->lssm_warpin_delay=0;			//delay between warpout and warpin (ms)
	wip->lssm_stage5_vel=0;		//velocity during final stage
	wip->lssm_warpin_radius=0;
	wip->lssm_lock_range=1000000.0f;	//local ssm lock range (optional)

	wip->cm_aspect_effectiveness = 1.0f;
	wip->cm_heat_effectiveness = 1.0f;
	wip->cm_effective_rad = MAX_CMEASURE_TRACK_DIST;

	wip->b_info.beam_type = -1;
	wip->b_info.beam_life = -1.0f;
	wip->b_info.beam_warmup = -1;
	wip->b_info.beam_warmdown = -1;
	wip->b_info.beam_muzzle_radius = 0.0f;
	wip->b_info.beam_particle_count = -1;
	wip->b_info.beam_particle_radius = 0.0f;
	wip->b_info.beam_particle_angle = 0.0f;
	wip->b_info.beam_loop_sound = -1;
	wip->b_info.beam_warmup_sound = -1;
	wip->b_info.beam_warmdown_sound = -1;
	wip->b_info.beam_num_sections = 0;
	wip->b_info.beam_shots = 1;
	wip->b_info.beam_shrink_factor = 0.0f;
	wip->b_info.beam_shrink_pct = 0.0f;
	wip->b_info.range = BEAM_FAR_LENGTH;
	wip->b_info.damage_threshold = 1.0f;
	wip->b_info.beam_width = -1.0f;

	generic_anim_init(&wip->b_info.beam_glow, NULL);
	generic_anim_init(&wip->b_info.beam_particle_ani, NULL);

	for (i = 0; i < MAX_IFFS; i++)
		for (j = 0; j < NUM_SKILL_LEVELS; j++)
			wip->b_info.beam_iff_miss_factor[i][j] = 0.00001f;
	
	//WMC - Okay, so this is needed now
	beam_weapon_section_info *bsip;
	for (i = 0; i < MAX_BEAM_SECTIONS; i++) {
		bsip = &wip->b_info.sections[i];

		generic_anim_init(&bsip->texture, NULL);

		for (j = 0; j < 4; j++) {
			bsip->rgba_inner[j] = 0;
			bsip->rgba_outer[j] = 255;
		}

		bsip->width = 1.0f;
		bsip->flicker = 0.1f;
		bsip->z_add = i2fl(MAX_BEAM_SECTIONS - i - 1);
		bsip->tile_type = 0;
		bsip->tile_factor = 1.0f;
		bsip->translation = 0.0f;
	}

	for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {						// default values for everything -nuke
		wip->particle_spewers[s].particle_spew_type = PSPEW_NONE;				// added by nuke
		wip->particle_spewers[s].particle_spew_count = 1;
		wip->particle_spewers[s].particle_spew_time = 25;
		wip->particle_spewers[s].particle_spew_vel = 0.4f;
		wip->particle_spewers[s].particle_spew_radius = 2.0f;
		wip->particle_spewers[s].particle_spew_lifetime = 0.15f;
		wip->particle_spewers[s].particle_spew_scale = 0.8f;
		wip->particle_spewers[s].particle_spew_z_scale = 1.0f;			// added by nuke
		wip->particle_spewers[s].particle_spew_rotation_rate = 10.0f;
		wip->particle_spewers[s].particle_spew_offset = vmd_zero_vector;
		wip->particle_spewers[s].particle_spew_velocity = vmd_zero_vector;
		generic_anim_init(&wip->particle_spewers[s].particle_spew_anim, NULL);
	}
	
	wip->tag_level = -1;
	wip->tag_time = -1.0f;
	
	wip->SSM_index =-1;				// tag C SSM index, wich entry in the SSM table this weapon calls -Bobboau
	
	wip->field_of_fire = 0.0f;
	
	wip->shots = 1;

	wip->alpha_max = 1.0f;
	wip->alpha_min = 0.0f;
	wip->alpha_cycle = 0.0f;

	// this can get reset after the constructor, so be sure it's correct
	wip->shockwave.damage_type_idx = -1;
	wip->shockwave.damage_type_idx_sav = -1;

	wip->weapon_hitpoints = 0;

	wip->burst_delay = 1.0f; // 1 second, just incase its not defined
	wip->burst_shots = 0;
	wip->burst_flags = 0;

	generic_anim_init( &wip->thruster_flame );
	generic_anim_init( &wip->thruster_glow );
	
	wip->thruster_glow_factor = 1.0f;
	wip->target_lead_scaler = 0.0f;
}

// function to parse the information for a specific weapon type.	
// return 0 if successful, otherwise return -1
#define WEAPONS_MULTITEXT_LENGTH 2048

int parse_weapon(int subtype, bool replace)
{
	char buf[WEAPONS_MULTITEXT_LENGTH];
	weapon_info *wip = NULL;
	char fname[NAME_LENGTH];
	int iff, idx;
	int primary_rearm_rate_specified=0;
	bool first_time = false;
	bool create_if_not_found  = true;
	int wi_flags = WIF_DEFAULT_VALUE;
	int wi_flags2 = WIF2_DEFAULT_VALUE;

	required_string("$Name:");
	stuff_string(fname, F_NAME, NAME_LENGTH);
	diag_printf ("Weapon name -- %s\n", fname);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for weapon in non-modular table");
		}
		create_if_not_found = false;
	}

	strcpy_s(parse_error_text, "");
	strcpy_s(parse_error_text, "\nin weapon: ");
	strcat_s(parse_error_text, fname);
	strcat_s(parse_error_text, "\n");

	//Remove @ symbol
	//these used to be used to denote weapons that would
	//only be parsed in demo builds
	if ( fname[0] == '@' ) {
		backspace(fname);
	}

	int w_id = weapon_info_lookup(fname);

	if(w_id != -1)
	{
		wip = &Weapon_info[w_id];
		if(!replace)
		{
			Warning(LOCATION, "Weapon name %s already exists in weapons.tbl.  All weapon names must be unique; the second entry has been skipped", wip->name);
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return -1;
		}
	}
	else
	{
		//Don't create weapon if it has +nocreate and is in a modular table.
		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}

			return -1;
		}

		if(Num_weapon_types >= MAX_WEAPON_TYPES) {
			Warning(LOCATION, "Too many weapon classes before '%s'; maximum is %d, so only the first %d will be used", fname, MAX_WEAPON_TYPES, Num_weapon_types);
			
			//Skip the rest of the ships in non-modular tables, since we can't add them.
			if(!replace) {
				if ( !skip_to_start_of_string_either("$Name:", "#End")) {
					Int3();
				}
			}
			return -1;
		}

		wip = &Weapon_info[Num_weapon_types];
		init_weapon_entry(Num_weapon_types);
		first_time = true;
		
		strcpy_s(wip->name, fname);
		Num_weapon_types++;
	}

	if(optional_string("$Alt name:"))
		stuff_string(wip->alt_name, F_NAME, NAME_LENGTH);

	//Set subtype
	if(optional_string("$Subtype:"))
	{
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if(!stricmp("Primary", fname)) {
			wip->subtype = WP_LASER;
		} else if(!stricmp("Secondary", fname)) {
			wip->subtype = WP_MISSILE;
		} else {
			Warning(LOCATION, "Unknown subtype on weapon '%s'", wip->name);
		}
	}
	else if(wip->subtype != WP_UNUSED && !first_time)
	{
		if(wip->subtype != subtype) {
			Warning(LOCATION, "Type of weapon %s entry does not agree with original entry type.", wip->name);
		}
	}
	else
	{
		wip->subtype = subtype;
	}

	if (optional_string("+Title:")) {
		stuff_string(wip->title, F_NAME, WEAPON_TITLE_LEN);
	}

	if (optional_string("+Description:")) {
		if (wip->desc != NULL) {
			vm_free(wip->desc);
			wip->desc = NULL;
		}

		stuff_malloc_string(&wip->desc, F_MULTITEXT);
	}

	if (optional_string("+Tech Title:")) {
		stuff_string(wip->tech_title, F_NAME, NAME_LENGTH);
	}

	if (optional_string("+Tech Anim:")) {
		stuff_string(wip->tech_anim_filename, F_NAME, MAX_FILENAME_LEN);
	}

	if (optional_string("+Tech Description:")) {
		if (wip->tech_desc != NULL) {
			vm_free(wip->tech_desc);
			wip->tech_desc = NULL;
		}

		stuff_malloc_string(&wip->tech_desc, F_MULTITEXT);
//		stuff_string(buf, F_MULTITEXT, NULL, WEAPONS_MULTITEXT_LENGTH);
//		wip->tech_desc = vm_strdup(buf);
	}

	if(optional_string("$Tech Model:")) {
		stuff_string(wip->tech_model, F_NAME, MAX_FILENAME_LEN);
	}
		

	//Check for the HUD image string
	if(optional_string("$HUD Image:")) {
		stuff_string(wip->hud_filename, F_NAME, MAX_FILENAME_LEN);
	}

	//	Read the model file.  It can be a POF file or none.
	//	If there is no model file (Model file: = "none") then we use our special
	//	laser renderer which requires inner, middle and outer information.
	if ( optional_string("$Model file:") ) {
		stuff_string(wip->pofbitmap_name, F_NAME, MAX_FILENAME_LEN);

		if ( VALID_FNAME(wip->pofbitmap_name) )
			wip->render_type = WRT_POF;

		diag_printf("Model pof file -- %s\n", wip->pofbitmap_name );
	}

	// a special LOD level to use when rendering the weapon in the hud targetbox
	if ( optional_string( "$POF target LOD:" ) )
		stuff_int(&wip->hud_target_lod);

	if(optional_string("$Detail distance:")) {
		wip->num_detail_levels = stuff_int_list(wip->detail_distance, MAX_MODEL_DETAIL_LEVELS, RAW_INTEGER_TYPE);
	}

	if ( optional_string("$External Model File:") )
		stuff_string(wip->external_model_name, F_NAME, MAX_FILENAME_LEN);	

	if ( optional_string("$Submodel Rotation Speed:") )
		stuff_float(&wip->weapon_submodel_rotate_vel);

	if ( optional_string("$Submodel Rotation Acceleration:") )
		stuff_float(&wip->weapon_submodel_rotate_accell);

	//	No POF or AVI file specified, render as special laser type.(?)
	ubyte r,g,b;

	// laser bitmap itself
	if ( optional_string("@Laser Bitmap:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if (wip->render_type == WRT_POF) {
			mprintf(("WARNING:  Weapon '%s' has both LASER and POF render types!  Will only use POF type!\n", wip->name));
			generic_anim_init(&wip->laser_bitmap, NULL);
		} else {
			generic_anim_init(&wip->laser_bitmap, fname);
			wip->render_type = WRT_LASER;
		}
	}

	// optional laser glow
	if ( optional_string("@Laser Glow:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if (wip->render_type != WRT_LASER) {
			mprintf(("WARNING:  Laser glow specified on non-LASER type weapon (%s)!\n", wip->name));
			Int3();
		} else {
			generic_anim_init(&wip->laser_glow_bitmap, fname);
		}
	}

	if(optional_string("@Laser Color:"))
	{
		stuff_ubyte(&r);
		stuff_ubyte(&g);
		stuff_ubyte(&b);
		gr_init_color( &wip->laser_color_1, r, g, b );
	}

	// optional string for cycling laser colors
	if(optional_string("@Laser Color2:")){
		stuff_ubyte(&r);
		stuff_ubyte(&g);
		stuff_ubyte(&b);
		gr_init_color( &wip->laser_color_2, r, g, b );
	}

	if(optional_string("@Laser Length:")) {
		stuff_float(&wip->laser_length);
	}
	
	if(optional_string("@Laser Head Radius:")) {
		stuff_float(&wip->laser_head_radius);
	}

	if(optional_string("@Laser Tail Radius:")) {
		stuff_float(&wip->laser_tail_radius );
	}

	if(optional_string("$Mass:")) {
		stuff_float( &(wip->mass) );

		// Goober5000 - hack in order to make the beam whack behavior of these three beams match all other beams
		// this relies on Bobboau's beam whack hack in beam_apply_whack()
		if (!strcmp(wip->name, "SAAA") && (wip->mass == 4.0f)
			|| !strcmp(wip->name, "MjolnirBeam") && (wip->mass == 1000.0f)
			|| !strcmp(wip->name, "MjolnirBeam#home") && (wip->mass == 1000.0f))
		{
			wip->mass = 100.0f;
		}

		diag_printf ("Weapon mass -- %7.3f\n", wip->mass);
	}

	if(optional_string("$Velocity:")) {
		stuff_float( &(wip->max_speed) );
		diag_printf ("Weapon mass -- %7.3f\n", wip->max_speed);
	}

	if(optional_string("$Fire Wait:")) {
		stuff_float( &(wip->fire_wait) );
		diag_printf ("Weapon fire wait -- %7.3f\n", wip->fire_wait);
	}

	if(optional_string("$Damage:")) {
		stuff_float(&wip->damage);
		//WMC - now that shockwave damage can be set for them individually,
		//do this automagically
		if(first_time) {
			wip->shockwave.damage = wip->damage;
		}
	}
	
	if(optional_string("$Damage Type:")) {
		//This is checked for validity on every armor type
		//If it's invalid (or -1), then armor has no effect
		stuff_string(buf, F_NAME, WEAPONS_MULTITEXT_LENGTH);
		wip->damage_type_idx_sav = damage_type_add(buf);
		wip->damage_type_idx = wip->damage_type_idx_sav;
	}

	if(optional_string("$Arm time:")) {
		float flit;
		stuff_float(&flit);
		wip->arm_time = fl2f(flit);
	}

	if(optional_string("$Arm distance:")) {
		stuff_float(&wip->arm_dist);
	}

	if(optional_string("$Arm radius:")) {
		stuff_float(&wip->arm_radius);
	}

	if(optional_string("$Detonation Range:")) {
		stuff_float(&wip->det_range);
	}

	if(optional_string("$Detonation Radius:")) {
		stuff_float(&wip->det_radius);
	}

	parse_shockwave_info(&wip->shockwave, "$");

	//Retain compatibility
	if(first_time)
	{
		wip->dinky_shockwave = wip->shockwave;
		wip->dinky_shockwave.damage /= 4.0f;
	}

	if(optional_string("$Dinky shockwave:"))
	{
		parse_shockwave_info(&wip->dinky_shockwave, "+");
	}

	if(optional_string("$Armor Factor:")) {
		stuff_float(&wip->armor_factor);
	}

	if(optional_string("$Shield Factor:")) {
		stuff_float(&wip->shield_factor);
	}

	if(optional_string("$Subsystem Factor:")) {
		stuff_float(&wip->subsystem_factor);
	}

	if(optional_string("$Lifetime Min:")) {
		stuff_float(&wip->life_min);

		if(wip->life_min < 0.0f) {
			wip->life_min = 0.0f;
			Warning(LOCATION, "Lifetime min for weapon '%s' cannot be less than 0. Setting to 0.", wip->name);
		}
	}

	if(optional_string("$Lifetime Max:")) {
		stuff_float(&wip->life_max);

		if(wip->life_max < 0.0f) {
			wip->life_max = 0.0f;
			Warning(LOCATION, "Lifetime max for weapon '%s' cannot be less than 0. Setting to 0.", wip->name);
		}
	}

	if(wip->life_min >= 0.0f && wip->life_max < 0.0f) {
		wip->lifetime = wip->life_min;
		wip->life_min = -1.0f;
		Warning(LOCATION, "Lifetime min, but not lifetime max, specified for weapon %s. Assuming static lifetime of %.2f seconds.", wip->lifetime);
	}

	if(optional_string("$Lifetime:")) {
		if(wip->life_min >= 0.0f || wip->life_max >= 0.0f) {
			Warning(LOCATION, "Lifetime min or max specified, but $Lifetime was also specified; min or max will be used.");
		}
		stuff_float(&wip->lifetime);
	}

	if(optional_string("$Energy Consumed:")) {
		stuff_float(&wip->energy_consumed);
	}

	// Goober5000: cargo size is checked for div-0 errors... see below (must parse flags first)
	if(optional_string("$Cargo Size:"))
	{
		stuff_float(&wip->cargo_size);
	}

	bool is_homing=false;
	if(optional_string("$Homing:")) {
		stuff_boolean(&is_homing);
	}

	if (is_homing || (wip->wi_flags & WIF_HOMING))
	{
		char	temp_type[NAME_LENGTH];

		// the following five items only need to be recorded if the weapon is a homing weapon
		if(optional_string("+Type:"))
		{
			stuff_string(temp_type, F_NAME, NAME_LENGTH);
			if (!stricmp(temp_type, NOX("HEAT")))
			{
				if(wip->wi_flags & WIF_HOMING_ASPECT) {
					wip->wi_flags &= ~WIF_HOMING_ASPECT;
				}
				if(wip->wi_flags & WIF_HOMING_JAVELIN) {
					wip->wi_flags &= ~WIF_HOMING_JAVELIN;
				}

				wip->wi_flags |= WIF_HOMING_HEAT | WIF_TURNS;
				wi_flags |= WIF_HOMING_HEAT | WIF_TURNS;
			}
			else if (!stricmp(temp_type, NOX("ASPECT")))
			{
				if(wip->wi_flags & WIF_HOMING_HEAT) {
					wip->wi_flags &= ~WIF_HOMING_HEAT;
				}
				if(wip->wi_flags & WIF_HOMING_JAVELIN) {
					wip->wi_flags &= ~WIF_HOMING_JAVELIN;
				}

				wip->wi_flags |= WIF_HOMING_ASPECT | WIF_TURNS;
				wi_flags |= WIF_HOMING_ASPECT | WIF_TURNS;
			}
			else if (!stricmp(temp_type, NOX("JAVELIN")))
			{
				if(wip->wi_flags & WIF_HOMING_HEAT) {
					wip->wi_flags &= ~WIF_HOMING_HEAT;
				}
				if(wip->wi_flags & WIF_HOMING_ASPECT) {
					wip->wi_flags &= ~WIF_HOMING_ASPECT;
				}

				wip->wi_flags |= WIF_HOMING_JAVELIN | WIF_TURNS;
				wi_flags |= WIF_HOMING_JAVELIN | WIF_TURNS;
			}
			//If you want to add another weapon, remember you need to reset
			//ALL homing flags.
		}

		if (wip->wi_flags & WIF_HOMING_HEAT)
		{
			float	view_cone_angle;

			if(optional_string("+Turn Time:")) {
				stuff_float(&wip->turn_time);
			}

			if(optional_string("+View Cone:")) {
				stuff_float(&view_cone_angle);
				wip->fov = (float)cos((float)(ANG_TO_RAD(view_cone_angle/2.0f)));
			}

			if (optional_string("+Seeker Strength:"))
			{
				//heat default seeker strength is 3
				stuff_float(&wip->seeker_strength);
				wip->wi_flags2 |= WIF2_CUSTOM_SEEKER_STR;
				if (wip->seeker_strength <= 0)
				{
					Warning(LOCATION,"Seeker Strength for missile \'%s\' must be greater than zero\nReseting value to default.", wip->name);
					wip->seeker_strength = 3.0f;
					wip->wi_flags2 &= ~WIF2_CUSTOM_SEEKER_STR;
				}
			}
			else
			{
				if(!(wip->wi_flags2 & WIF2_CUSTOM_SEEKER_STR))
					wip->seeker_strength = 3.0f;
			}

			if (optional_string("+Target Lead Scaler:"))
			{
				stuff_float(&wip->target_lead_scaler);
				if (wip->target_lead_scaler == 0.0f)
					wip->wi_flags2 &= ~WIF2_VARIABLE_LEAD_HOMING;
				else {
					wip->wi_flags2 |= WIF2_VARIABLE_LEAD_HOMING;
					wi_flags2 |= WIF2_VARIABLE_LEAD_HOMING;
				}
			}
		}
		else if (wip->wi_flags & WIF_HOMING_ASPECT || wip->wi_flags & WIF_HOMING_JAVELIN)
		{
			if(optional_string("+Turn Time:")) {
				stuff_float(&wip->turn_time);
			}

			if(optional_string("+View Cone:")) {
				float	view_cone_angle;
				stuff_float(&view_cone_angle);
				wip->fov = (float)cos((float)(ANG_TO_RAD(view_cone_angle/2.0f)));
			}

			if(optional_string("+Min Lock Time:")) {			// minimum time (in seconds) to achieve lock
				stuff_float(&wip->min_lock_time);
			}

			if(optional_string("+Lock Pixels/Sec:")) {		// pixels/sec moved while locking
				stuff_int(&wip->lock_pixels_per_sec);
			}

			if(optional_string("+Catch-up Pixels/Sec:")) {	// pixels/sec moved while catching-up for a lock
				stuff_int(&wip->catchup_pixels_per_sec);
			}

			if(optional_string("+Catch-up Penalty:")) {
				// number of extra pixels to move while locking as a penalty for catching up for a lock
				stuff_int(&wip->catchup_pixel_penalty);
			}

			if (optional_string("+Seeker Strength:"))
			{
				//aspect default seeker strength is 2
				stuff_float(&wip->seeker_strength);
				wip->wi_flags2 |= WIF2_CUSTOM_SEEKER_STR;
				if (wip->seeker_strength <= 0)
				{
					Warning(LOCATION,"Seeker Strength for missile \'%s\' must be greater than zero\nReseting value to default.", wip->name);
					wip->seeker_strength = 2.0f;
					wip->wi_flags2 &= ~WIF2_CUSTOM_SEEKER_STR;
				}
			} 
			else
			{
				if(!(wip->wi_flags2 & WIF2_CUSTOM_SEEKER_STR))
					wip->seeker_strength = 2.0f;
			}
			if (optional_string("+Target Lead Scaler:"))
			{
				stuff_float(&wip->target_lead_scaler);
				if (wip->target_lead_scaler == 1.0f)
					wip->wi_flags2 &= ~WIF2_VARIABLE_LEAD_HOMING;
				else {
					wip->wi_flags2 |= WIF2_VARIABLE_LEAD_HOMING;
					wi_flags2 |= WIF2_VARIABLE_LEAD_HOMING;
				}
			}
		}
		else
		{
			Error(LOCATION, "Illegal homing type = %s.\nMust be HEAT, ASPECT or JAVELIN.\n", temp_type);
		}

	}

	// swarm missiles
	int s_count;

	if(optional_string("$Swarm:"))
	{
		wip->swarm_count = SWARM_DEFAULT_NUM_MISSILES_FIRED;
		stuff_int(&s_count);
		wip->swarm_count = (short)s_count;

		// flag as being a swarm weapon
		wip->wi_flags |= WIF_SWARM;
		wi_flags |= WIF_SWARM;
	}

	// *Swarm wait token    -Et1
	if((wip->wi_flags & WIF_SWARM) && optional_string( "+SwarmWait:" ))
	{
		float SwarmWait;
		stuff_float( &SwarmWait );
		if( SwarmWait > 0.0f && SwarmWait * wip->swarm_count < wip->fire_wait )
		{
			wip->SwarmWait = int( SwarmWait * 1000 );
		}
	}

	if(optional_string("$Free Flight Time:")) {
		stuff_float(&(wip->free_flight_time));
	} else if(first_time && is_homing) {
		wip->free_flight_time = HOMING_DEFAULT_FREE_FLIGHT_TIME;
	}

	if(optional_string("$Free Flight Speed:")) {
		stuff_float(&wip->free_flight_speed);
		if (wip->free_flight_speed < 0.01f) {
			nprintf(("Warning", "Free Flight Speed value is too low. Resetting to default (25% of maximum)\n"));
			wip->free_flight_speed = 0.25f;
		}
	}
	//Optional one-shot sound to play at the beginning of firing
	parse_sound("$PreLaunchSnd:", &wip->pre_launch_snd, wip->name);

	//Optional delay for Pre-Launch sound
	if(optional_string("+PreLaunchSnd Min Interval:"))
	{
		stuff_int(&wip->pre_launch_snd_min_interval);
	}

	//Launch sound
	parse_sound("$LaunchSnd:", &wip->launch_snd, wip->name);

	//Impact sound
	parse_sound("$ImpactSnd:", &wip->impact_snd, wip->name);

	//Disarmed impact sound
	parse_sound("$Disarmed ImpactSnd:", &wip->impact_snd, wip->name);

	if (subtype == WP_MISSILE)
	{
		parse_sound("$FlyBySnd:", &wip->flyby_snd, wip->name);
	}
	else
	{
		if (optional_string("$FlyBySnd:"))
		{
			Warning(LOCATION, "$FlyBySnd: flag found on %s, but is not used with primary weapons; ignoring...", wip->name);
		}
	}

	if(optional_string("$Model:"))
	{
		wip->render_type = WRT_POF;
		stuff_string(wip->pofbitmap_name, F_NAME, MAX_FILENAME_LEN);
	}

	// handle rearm rate - modified by Goober5000
	primary_rearm_rate_specified = 0;
	float rearm_rate;
	// Anticipate rearm rate for ballistic primaries
	if (optional_string("$Rearm Rate:"))
	{
		if (subtype != WP_MISSILE) {
			primary_rearm_rate_specified = 1;
		}

		stuff_float( &rearm_rate );
		if (rearm_rate > 0.0f)
		{
			wip->rearm_rate = 1.0f/rearm_rate;
		}
		else
		{
			Warning(LOCATION, "Rearm wait of less than 0 on weapon %s; setting to 1", wip->name);
		}
	}


	if (optional_string("+Weapon Range:")) {
		stuff_float(&wip->weapon_range);
	}

	if( optional_string( "+Weapon Min Range:" ) )
	{
		float MinRange;
		stuff_float( &MinRange );

		if( MinRange > 0.0f && MinRange < MIN( wip->max_speed * wip->lifetime, wip->weapon_range ) )
		{
			wip->WeaponMinRange = MinRange;
		}
		else
		{
			Warning(LOCATION, "Invalid minimum range on weapon %s; setting to 0", wip->name);
		}

	}

	parse_wi_flags(wip, wi_flags, wi_flags2);

	// be friendly; make sure ballistic flags are synchronized - Goober5000
	// primary
	if (subtype == WP_LASER)
	{
		// ballistic
		if (wip->wi_flags2 & WIF2_BALLISTIC)
		{
			// rearm rate not specified
			if (!primary_rearm_rate_specified && first_time)
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
		if (!wip->shockwave.outer_rad)
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
			wip->cargo_size = 1.0f;
		}
	}

	if ( optional_string("$Trail:") ) {
		trail_info *ti = &wip->tr_info;
		wip->wi_flags |= WIF_TRAIL;		// missile leaves a trail

		if ( optional_string("+Start Width:") )
			stuff_float(&ti->w_start);

		if ( optional_string("+End Width:") )
			stuff_float(&ti->w_end);

		if ( optional_string("+Start Alpha:") )
			stuff_float(&ti->a_start);

		if ( optional_string("+End Alpha:") )
			stuff_float(&ti->a_end);

		if ( optional_string("+Max Life:") ) {
			stuff_float(&ti->max_life);
			ti->stamp = fl2i(1000.0f*ti->max_life)/(NUM_TRAIL_SECTIONS+1);
		}

		if ( optional_string("+Bitmap:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
			generic_bitmap_init(&ti->texture, fname);
		}

		if ( optional_string("+Faded Out Sections:") ) {
			stuff_int(&ti->n_fade_out_sections);
		}
		// wip->delta_time = fl2i(1000.0f*wip->max_life)/(NUM_TRAIL_SECTIONS+1);		// time between sections.  max_life / num_sections basically.
	}

	// read in filename for icon that is used in weapons selection
	if ( optional_string("$Icon:") ) {
		stuff_string(wip->icon_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in filename for animation that is used in weapons selection
	if ( optional_string("$Anim:") ) {
		stuff_string(wip->anim_filename, F_NAME, MAX_FILENAME_LEN);
	}
/*
	if(optional_string("$Collide Ship:")) {
		wip->sc_collide_ship = Script_system.ParseChunk("$Collide Ship");
	}

	if(optional_string("$Collide Weapon:")) {
		wip->sc_collide_weapon = Script_system.ParseChunk("$Collide Weapon");
	}
*/

	if ( optional_string("$Impact Explosion:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if ( VALID_FNAME(fname) )
			wip->impact_weapon_expl_index = Weapon_explosions.Load(fname);
	}
	
	if ( optional_string("$Impact Explosion Radius:") )
		stuff_float(&wip->impact_explosion_radius);

	if ( optional_string("$Dinky Impact Explosion:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if ( VALID_FNAME(fname) )
			wip->dinky_impact_weapon_expl_index = Weapon_explosions.Load(fname);
	} else if (first_time) {
		wip->dinky_impact_weapon_expl_index = wip->impact_weapon_expl_index;
	}

	if ( optional_string("$Dinky Impact Explosion Radius:") )
		stuff_float(&wip->dinky_impact_explosion_radius);
	else if (first_time)
		wip->dinky_impact_explosion_radius = wip->impact_explosion_radius;

	if ( optional_string("$Piercing Impact Explosion:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if ( VALID_FNAME(fname) )
			wip->piercing_impact_weapon_expl_index = Weapon_explosions.Load(fname);
	}

	if ( optional_string("$Piercing Impact Radius:") )
		stuff_float(&wip->piercing_impact_explosion_radius);

	if ( optional_string("$Piercing Impact Velocity:") )
		stuff_float(&wip->piercing_impact_particle_velocity);

	if ( optional_string("$Piercing Impact Splash Velocity:") )
		stuff_float(&wip->piercing_impact_particle_back_velocity);

	if ( optional_string("$Piercing Impact Variance:") )
		stuff_float(&wip->piercing_impact_particle_variance);

	if ( optional_string("$Piercing Impact Life:") )
		stuff_float(&wip->piercing_impact_particle_life);

	if ( optional_string("$Piercing Impact Particles:") )
		stuff_int(&wip->piercing_impact_particle_count);

	// muzzle flash
	if ( optional_string("$Muzzleflash:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		// look it up
		wip->muzzle_flash = mflash_lookup(fname);
	}

	if (wip->muzzle_flash > -1)
		wip->wi_flags |= WIF_MFLASH;

	// EMP optional stuff (if WIF_EMP is not set, none of this matters, anyway)
	if( optional_string("$EMP Intensity:") ){
		stuff_float(&wip->emp_intensity);
	}
	
	if( optional_string("$EMP Time:") ){
		stuff_float(&wip->emp_time);
	}

	// Energy suck optional stuff (if WIF_ENERGY_SUCK is not set, none of this matters anyway)
	if( optional_string("$Leech Weapon:") ){
		stuff_float(&wip->weapon_reduce);
	}

	if( optional_string("$Leech Afterburner:") ){
		stuff_float(&wip->afterburner_reduce);
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

	if (optional_string("$Corkscrew:"))
	{
		if(optional_string("+Num Fired:")) {
			stuff_int(&wip->cs_num_fired);
		}

		if(optional_string("+Radius:")) {
			stuff_float(&wip->cs_radius);
		}

		if(optional_string("+Fire Delay:")) {
			stuff_int(&wip->cs_delay);
		}
		
		if(optional_string("+Counter rotate:")) {
			stuff_boolean(&wip->cs_crotate);
		}

		if(optional_string("+Twist:")) {
			stuff_float(&wip->cs_twist);
		}
	}

	//electronics tag optional stuff
	//Note that I made all these optional in the interest of modular tables.
	//TODO: Possibly add a warning on first_time define?
	if (optional_string("$Electronics:"))
	{
		if(optional_string("+New Style:")) {
			wip->elec_use_new_style=1;
		}
		else if(optional_string("+Old Style:")) {
			wip->elec_use_new_style=0;
		}
		
		//New only -WMC
		if(optional_string("+Intensity:")) {
			float temp;
			stuff_float(&temp);
			Warning(LOCATION, "+Intensity is deprecated");
		}

		if(optional_string("+Lifetime:")) {
			stuff_int(&wip->elec_time);
		}

		//New only -WMC
		if(optional_string("+Engine Multiplier:")) {
			stuff_float(&wip->elec_eng_mult);
			if(!wip->elec_use_new_style)Warning(LOCATION, "+Engine multiplier may only be used with new style electronics");
		}

		//New only -WMC
		if(optional_string("+Weapon Multiplier:")) {
			stuff_float(&wip->elec_weap_mult);
			if(!wip->elec_use_new_style)Warning(LOCATION, "+Weapon multiplier may only be used with new style electronics");
		}

		//New only -WMC
		if(optional_string("+Beam Turret Multiplier:")) {
			stuff_float(&wip->elec_beam_mult);
			if(!wip->elec_use_new_style)Warning(LOCATION, "+Beam turret multiplier may only be used with new style electronics");
		}

		//New only -WMC
		if(optional_string("+Sensors Multiplier:")) {
			stuff_float(&wip->elec_sensors_mult);
			if(!wip->elec_use_new_style)Warning(LOCATION, "+Sensors multiplier may only be used with new style electronics");
		}
	
		if(optional_string("+Randomness Time:")) {
			stuff_int(&wip->elec_randomness);
		}
	}


	//read in the spawn angle info
	//if the weapon isn't a spawn weapon, then this is not going to be used.
    int num_spawn_angs_defined = 0;
    float dum_float;

	while (optional_string("$Spawn Angle:"))
    {
        stuff_float(&dum_float);

        if (num_spawn_angs_defined < MAX_SPAWN_TYPES_PER_WEAPON)
        {
            wip->spawn_info[num_spawn_angs_defined].spawn_angle = dum_float;
            num_spawn_angs_defined++;
        }
	}

	if (wip->wi_flags2 & WIF2_LOCAL_SSM && optional_string("$Local SSM:"))
	{
		if(optional_string("+Warpout Delay:")) {
			stuff_int(&wip->lssm_warpout_delay);
		}

		if(optional_string("+Warpin Delay:")) {
			stuff_int(&wip->lssm_warpin_delay);
		}

		if(optional_string("+Stage 5 Velocity:")) {
			stuff_float(&wip->lssm_stage5_vel);
		}

		if(optional_string("+Warpin Radius:")) {
			stuff_float(&wip->lssm_warpin_radius);
		}

		if (optional_string("+Lock Range:")) {
			stuff_float(&wip->lssm_lock_range);
		}
	}

	if (optional_string("$Countermeasure:"))
	{
		if (!(wip->wi_flags & WIF_CMEASURE))
		{
			Warning(LOCATION,"Weapon \'%s\' has countermeasure information defined, but the \"countermeasure\" flag wasn\'t found in the \'$Flags:\' field.\n", wip->name);
		}

		if (optional_string("+Heat Effectiveness:"))
			stuff_float(&wip->cm_heat_effectiveness);

		if (optional_string("+Aspect Effectiveness:"))
			stuff_float(&wip->cm_aspect_effectiveness);

		if (optional_string("+Effective Radius:"))
			stuff_float(&wip->cm_effective_rad);
	}

	// beam weapon optional stuff
	if ( optional_string("$BeamInfo:") ) {
		// beam type
		if(optional_string("+Type:")) {
			stuff_int(&wip->b_info.beam_type);
		}

		// how long it lasts
		if(optional_string("+Life:")) {
			stuff_float(&wip->b_info.beam_life);
		}

		// warmup time
		if(optional_string("+Warmup:")) {
			stuff_int(&wip->b_info.beam_warmup);
		}

		// warmdowm time
		if(optional_string("+Warmdown:")) {
			stuff_int(&wip->b_info.beam_warmdown);
		}

		// muzzle glow radius
		if(optional_string("+Radius:")) {
			stuff_float(&wip->b_info.beam_muzzle_radius);
		}

		// particle spew count
		if(optional_string("+PCount:")) {
			stuff_int(&wip->b_info.beam_particle_count);
		}

		// particle radius
		if(optional_string("+PRadius:")) {
			stuff_float(&wip->b_info.beam_particle_radius);
		}

		// angle off turret normal
		if(optional_string("+PAngle:")) {
			stuff_float(&wip->b_info.beam_particle_angle);
		}

		// particle bitmap/ani		
		if ( optional_string("+PAni:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
			generic_anim_init(&wip->b_info.beam_particle_ani, fname);
		}

		// magic miss #
		if(optional_string("+Miss Factor:")) {
			for(idx=0; idx<NUM_SKILL_LEVELS; idx++) {
				float temp;
				if(!stuff_float_optional(&temp)) {
					break;
				}
				// an unspecified Miss Factor should apply to all IFFs
				for(iff=0; iff<Num_iffs; iff++) {
					wip->b_info.beam_iff_miss_factor[iff][idx] = temp;
				}
			}
		}
		// now check miss factors for each IFF
		for(iff=0; iff<Num_iffs; iff++) {
			char miss_factor_string[NAME_LENGTH + 15];
			sprintf(miss_factor_string, "+%s Miss Factor:", Iff_info[iff].iff_name);
			if(optional_string(miss_factor_string)) {
				// this Miss Factor applies only to the specified IFF
				for(idx=0; idx<NUM_SKILL_LEVELS; idx++) {
					if(!stuff_float_optional(&wip->b_info.beam_iff_miss_factor[iff][idx])) {
						break;
					}
				}
			}
		}

		// beam fire sound
		parse_sound("+BeamSound:", &wip->b_info.beam_loop_sound, wip->name);

		// warmup sound
		parse_sound("+WarmupSound:", &wip->b_info.beam_warmup_sound, wip->name);

		// warmdown sound
		parse_sound("+WarmdownSound:", &wip->b_info.beam_warmdown_sound, wip->name);

		// glow bitmap
		if (optional_string("+Muzzleglow:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
			generic_anim_init(&wip->b_info.beam_glow, fname);
		}

		// # of shots (only used for type D beams)
		if(optional_string("+Shots:")) {
			stuff_int(&wip->b_info.beam_shots);
		}

		// make sure that we have at least one shot so that TYPE_D beams will work
		if ( (wip->b_info.beam_type == BEAM_TYPE_D) && (wip->b_info.beam_shots < 1) ) {
			Warning( LOCATION, "Type D beam weapon, '%s', has less than one \"+Shots\" specified!  It must be set to at least 1!!",  wip->name);
			wip->b_info.beam_shots = 1;
		}
		
		// shrinkage
		if(optional_string("+ShrinkFactor:")) {
			stuff_float(&wip->b_info.beam_shrink_factor);
		}
		
		if(optional_string("+ShrinkPct:")) {
			stuff_float(&wip->b_info.beam_shrink_pct);
		}

		if (optional_string("+Range:")) {
			stuff_float(&wip->b_info.range);
		}
		
		if ( optional_string("+Attenuation:") )
			stuff_float(&wip->b_info.damage_threshold);

		if ( optional_string("+BeamWidth:") )
			stuff_float(&wip->b_info.beam_width);

		if ( optional_string("+Beam Flash Effect:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);

			if ( VALID_FNAME(fname) )
				wip->flash_impact_weapon_expl_index = Weapon_explosions.Load(fname);
		}
		
		if ( optional_string("+Beam Flash Radius:") )
			stuff_float(&wip->flash_impact_explosion_radius);

		if ( optional_string("+Beam Piercing Effect:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);

			if ( VALID_FNAME(fname) )
				wip->piercing_impact_weapon_expl_index = Weapon_explosions.Load(fname);
		}
		
		if ( optional_string("+Beam Piercing Radius:") )
			stuff_float(&wip->piercing_impact_explosion_radius);

		if ( optional_string("+Beam Piercing Effect Velocity:") )
			stuff_float(&wip->piercing_impact_particle_velocity);

		if ( optional_string("+Beam Piercing Splash Effect Velocity:") )
			stuff_float(&wip->piercing_impact_particle_back_velocity);

		if ( optional_string("+Beam Piercing Effect Variance:") )
			stuff_float(&wip->piercing_impact_particle_variance);

		// beam sections
		while ( optional_string("$Section:") ) {
			beam_weapon_section_info *bsip = NULL, tbsw;
			bool nocreate = false, remove = false;
			int bsw_index_override = -1;

			if ( optional_string("+Index:") ) {
				stuff_int(&bsw_index_override);

				if ( optional_string("+remove") ) {
					nocreate = true;
					remove = true;
				}
					
				if ( (bsw_index_override < 0) || (!remove && (bsw_index_override >= wip->b_info.beam_num_sections)) )
					Warning(LOCATION, "Invalid +Index value of %d specified for beam section on weapon '%s'; valid values at this point are %d to %d.", bsw_index_override, wip->name, 0, wip->b_info.beam_num_sections -1);
			}

			if ( optional_string("+nocreate") )
				nocreate = true;

			// Where are we saving data?
			if (bsw_index_override >= 0) {
				if (bsw_index_override < wip->b_info.beam_num_sections) {
					bsip = &wip->b_info.sections[bsw_index_override];
				} else {
					if ( !nocreate ) {
						if ( (bsw_index_override == wip->b_info.beam_num_sections) && (bsw_index_override < MAX_BEAM_SECTIONS) ) {
							bsip = &wip->b_info.sections[wip->b_info.beam_num_sections++];
						} else {
							if ( !remove )
								Warning(LOCATION, "Invalid index for manually-indexed beam section %d (max %d) on weapon %s.", bsw_index_override, MAX_BEAM_SECTIONS, wip->name);

							bsip = &tbsw;
							memset( bsip, 0, sizeof(beam_weapon_section_info) );
							generic_anim_init(&bsip->texture, NULL);
						}
					} else {
						if ( !remove )
							Warning(LOCATION, "Invalid index for manually-indexed beam section %d, and +nocreate specified, on weapon %s", bsw_index_override, wip->name);

						bsip = &tbsw;
						memset( bsip, 0, sizeof(beam_weapon_section_info) );
						generic_anim_init(&bsip->texture, NULL);
					}

				}
			} else {
				if (wip->b_info.beam_num_sections < MAX_BEAM_SECTIONS) {
					bsip = &wip->b_info.sections[wip->b_info.beam_num_sections++];
					generic_anim_init(&bsip->texture, NULL);
				} else {
					Warning(LOCATION, "Too many beam sections for weapon %s - max is %d", wip->name, MAX_BEAM_SECTIONS);
					bsip = &tbsw;
					memset( bsip, 0, sizeof(beam_weapon_section_info) );
					generic_anim_init(&bsip->texture, NULL);
				}
			}
			
			// section width
			if ( optional_string("+Width:") )
				stuff_float(&bsip->width);

			// texture
			if ( optional_string("+Texture:") ) {
				stuff_string(fname, F_NAME, NAME_LENGTH);
				generic_anim_init(&bsip->texture, fname);
			}

			// rgba inner
			if ( optional_string("+RGBA Inner:") ) {
				stuff_ubyte(&bsip->rgba_inner[0]);
				stuff_ubyte(&bsip->rgba_inner[1]);
				stuff_ubyte(&bsip->rgba_inner[2]);
				stuff_ubyte(&bsip->rgba_inner[3]);
			}

			// rgba outer
			if ( optional_string("+RGBA Outer:") ) {
				stuff_ubyte(&bsip->rgba_outer[0]);
				stuff_ubyte(&bsip->rgba_outer[1]);
				stuff_ubyte(&bsip->rgba_outer[2]);
				stuff_ubyte(&bsip->rgba_outer[3]);
			}

			// flicker
			if ( optional_string("+Flicker:") ) {
				stuff_float(&bsip->flicker); 
				//Sanity
				if (bsip->flicker < 0.0f || bsip->flicker > 1.0f) {
					mprintf(("WARNING: Invalid value found for +Flicker on section %d of beam %s. Valid range is 0.0 to 1.0, values will be adjusted.\n", wip->b_info.beam_num_sections, wip->name));
					CLAMP(bsip->flicker, 0.0f, 1.0f);
				}
			}

			// zadd
			if ( optional_string("+Zadd:") )
				stuff_float(&bsip->z_add);

 			// beam texture tileing factor -Bobboau
			if ( optional_string("+Tile Factor:") ) {
				stuff_float(&bsip->tile_factor);
				stuff_int(&bsip->tile_type);
			}

			// beam texture moveing stuff -Bobboau
			if ( optional_string("+Translation:") )
				stuff_float(&bsip->translation);

			// if we are actually removing this index then reset it and we'll
			// clean up the entries later
			if (remove) {
				memset( bsip, 0, sizeof(beam_weapon_section_info) );
				generic_anim_init(&bsip->texture, NULL);
			}
		}
	}

	while ( optional_string("$Pspew:") ) {
		int spew_index = -1;
		// check for pspew flag
		if (!( wip->wi_flags & WIF_PARTICLE_SPEW )) {
			Warning(LOCATION, "$Pspew specified for weapon %s but this weapon does not have the \"Particle Spew\" weapon flag set. Automatically setting the flag", wip->name); 
			wip->wi_flags |= WIF_PARTICLE_SPEW;
		}
		// index for xmt edit, replace and remove support
		if (optional_string("+Index:")) {
			stuff_int(&spew_index);
			if (spew_index < 0 || spew_index >= MAX_PARTICLE_SPEWERS) {
				Warning(LOCATION, "+Index in particle spewer out of range. It must be between 0 and %i. Tag will be ignored.", MAX_PARTICLE_SPEWERS);
				spew_index = -1;
			}
		}
		// check for remove flag
		if (optional_string("+Remove")) {
			if (spew_index < 0) {
				Warning(LOCATION, "+Index not specified or is out of range, can not remove spewer.");
			} else { // restore defaults
				wip->particle_spewers[spew_index].particle_spew_type = PSPEW_NONE;
				wip->particle_spewers[spew_index].particle_spew_count = 1;
				wip->particle_spewers[spew_index].particle_spew_time = 25;
				wip->particle_spewers[spew_index].particle_spew_vel = 0.4f;
				wip->particle_spewers[spew_index].particle_spew_radius = 2.0f;
				wip->particle_spewers[spew_index].particle_spew_lifetime = 0.15f;
				wip->particle_spewers[spew_index].particle_spew_scale = 0.8f;
				wip->particle_spewers[spew_index].particle_spew_z_scale = 1.0f;
				wip->particle_spewers[spew_index].particle_spew_rotation_rate = 10.0f;
				wip->particle_spewers[spew_index].particle_spew_offset = vmd_zero_vector;
				wip->particle_spewers[spew_index].particle_spew_velocity = vmd_zero_vector;
				generic_anim_init(&wip->particle_spewers[spew_index].particle_spew_anim, NULL);
			}
		} else { // were not removing the spewer
			if (spew_index < 0) { // index us ether not used or is invalid, so figure out where to put things
				//find a free slot in the pspew info array
				for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {
					if (wip->particle_spewers[s].particle_spew_type == PSPEW_NONE) {
						spew_index = s;
						break;
					}
				}
			}
			// no empty spot found, the modder tried to define too many spewers, or screwed up the xmts, or my code sucks
			if ( spew_index < 0 ) {
				Warning(LOCATION, "Too many particle spewers, max number of spewers is %i.", MAX_PARTICLE_SPEWERS);
			} else { // we have a valid index, now parse the spewer already
				if (optional_string("+Type:")) { // added type field for pspew types, 0 is the default for reverse compatability -nuke
					char temp_pspew_type[NAME_LENGTH];
					stuff_string(temp_pspew_type, F_NAME, NAME_LENGTH);

					if (!stricmp(temp_pspew_type, NOX("DEFAULT"))) {
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_DEFAULT;
					} else if (!stricmp(temp_pspew_type, NOX("HELIX"))) {
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_HELIX;
					} else if (!stricmp(temp_pspew_type, NOX("SPARKLER"))) {	// new types can be added here
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_SPARKLER;
					} else if (!stricmp(temp_pspew_type, NOX("RING"))) {
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_RING;
					} else if (!stricmp(temp_pspew_type, NOX("PLUME"))) {
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_PLUME;
					} else {
						wip->particle_spewers[spew_index].particle_spew_type = PSPEW_DEFAULT;
					}
				// for compatability with existing tables that dont have a type tag
				} else if (wip->particle_spewers[spew_index].particle_spew_type == PSPEW_NONE) { // make sure the ommission of type wanst to edit an existing entry
					wip->particle_spewers[spew_index].particle_spew_type = PSPEW_DEFAULT;
				}

				if (optional_string("+Count:")) {
					stuff_int(&wip->particle_spewers[spew_index].particle_spew_count);
				}

				if (optional_string("+Time:")) {
					stuff_int(&wip->particle_spewers[spew_index].particle_spew_time);
				}

				if (optional_string("+Vel:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_vel);
				}

				if (optional_string("+Radius:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_radius);
				}

				if (optional_string("+Life:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_lifetime);
				}

				if (optional_string("+Scale:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_scale);
				}

				if (optional_string("+Z Scale:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_z_scale);
				}

				if (optional_string("+Rotation Rate:")) {
					stuff_float(&wip->particle_spewers[spew_index].particle_spew_rotation_rate);
				}

				if (optional_string("+Offset:")) {
					stuff_vector(&wip->particle_spewers[spew_index].particle_spew_offset);
				}

				if (optional_string("+Initial Velocity:")) {
					stuff_vector(&wip->particle_spewers[spew_index].particle_spew_velocity);
				}

				if (optional_string("+Bitmap:")) {
					stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
					generic_anim_init(&wip->particle_spewers[spew_index].particle_spew_anim, fname);
				}
			}
		}	
	}
	// check to see if the pspew flag was enabled but no pspew tags were given, for compatability with retail tables
	if (wip->wi_flags & WIF_PARTICLE_SPEW) {
		bool nospew = true;
		for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++)
			if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE) {
				nospew = false;
			}
		if (nospew) { // set first spewer to default
			wip->particle_spewers[0].particle_spew_type = PSPEW_DEFAULT;
		}
	}


	// tag weapon optional stuff
	if( optional_string("$Tag:")){
		stuff_int(&wip->tag_level);
		stuff_float(&wip->tag_time);		
		wip->wi_flags |= WIF_TAG;
	}	

	if( optional_string("$SSM:")){
		stuff_int(&wip->SSM_index);
	}// SSM index -Bobboau

	if( optional_string("$FOF:")){
		stuff_float(&wip->field_of_fire);
	}

	if( optional_string("$Shots:")){
		stuff_int(&wip->shots);
	}

	//Left in for compatibility
	if ( optional_string("$decal:") ) {
		mprintf(("WARNING: The decal system has been deactivated in FSO builds. Entries for weapon %s will be discarded.\n", wip->name));
		required_string("+texture:");
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if ( optional_string("+backface texture:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
		}

		float bogus;
		
		required_string("+radius:");
		stuff_float(&bogus);
		
		if ( optional_string("+burn time:") ) {
			stuff_float(&bogus);
		}
	}


	if (optional_string("$Transparent:")) {
		wip->wi_flags2 |= WIF2_TRANSPARENT;

		required_string("+Alpha:");
		stuff_float(&wip->alpha_max);

		if (wip->alpha_max > 1.0f)
			wip->alpha_max = 1.0f;

		if (wip->alpha_max <= 0.0f) {
			Warning(LOCATION, "WARNING:  Alpha is set to 0 or a negative value for '%s'!  Defaulting to 1.0!", wip->name);
		}

		if (optional_string("+Alpha Min:")) {
			stuff_float(&wip->alpha_min);

			if (wip->alpha_min > 1.0f)
				wip->alpha_min = 1.0f;

			if (wip->alpha_min < 0.0f)
				wip->alpha_min = 0.0f;
		}

		if (optional_string("+Alpha Cycle:")) {
			stuff_float(&wip->alpha_cycle);

			if (wip->alpha_max == wip->alpha_min)
				Warning(LOCATION, "WARNING:  Alpha is set to cycle for '%s', but max and min values are the same!", wip->name);
		}
	}

	if (optional_string("$Weapon Hitpoints:")) {
		stuff_int(&wip->weapon_hitpoints);
	}

	// making sure bombs get their hitpoints assigned
	if ((wip->wi_flags & WIF_BOMB) && (wip->weapon_hitpoints == 0)) {
		wip->weapon_hitpoints = 50;
	}

	if (optional_string("$Burst Shots:")) {
		stuff_int(&wip->burst_shots);
		if (wip->burst_shots > 0)
			wip->burst_shots--;
	}

	if (optional_string("$Burst Delay:")) {
		int temp;
		stuff_int(&temp);
		if (temp > 0) {
			wip->burst_delay = ((float) temp) / 1000.0f;
		}
	}

	if (optional_string("$Burst Flags:")) {
		parse_string_flag_list((int*)&wip->burst_flags, Burst_fire_flags, Num_burst_fire_flags);
	}

	if (optional_string("$Thruster Flame Effect:")) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if (VALID_FNAME(fname))
			generic_anim_init( &wip->thruster_flame, fname );
	}

	if (optional_string("$Thruster Glow Effect:")) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if (VALID_FNAME(fname))
			generic_anim_init( &wip->thruster_glow, fname );
	}

	if (optional_string("$Thruster Glow Radius Factor:")) {
		stuff_float(&wip->thruster_glow_factor);
	}

	//pretty stupid if a target must be tagged to shoot tag missiles at it
	if ((wip->wi_flags & WIF_TAG) && (wip->wi_flags2 & WIF2_TAGGED_ONLY))
	{
		Warning(LOCATION, "%s is a tag missile, but the target must be tagged to shoot it", wip->name);
	}

	// if burst delay is longer than firewait skip the whole burst fire option
	if (wip->burst_delay >= wip->fire_wait)
		wip->burst_shots = 0;

	/* Generate a substitution pattern for this weapon.
	This pattern is very naive such that is calculates the lowest common demoniator as being all of
	the freqencies multiplied together.
	*/
	while ( optional_string("$substitute:") ) {
		char subname[NAME_LENGTH];
		int frequency = 0;
		int index = 0;
		int offset = 0;
		stuff_string(subname, F_NAME, NAME_LENGTH);
		if ( optional_string("+frequency:") ) {
			stuff_int(&frequency);
			if ( frequency <= 0 ) {
				Warning(LOCATION, "Substitution '%s' for weapon '%s' requires a frequency greater than 0. Setting frequency to 1.", subname, wip->name);
				frequency = 1;
			}
			if ( optional_string("+offset:") ) {
				stuff_int(&offset);
				if ( offset <= 0 ) {
					Warning(LOCATION, "Frequency offset for substitution '%s' of weapon '%s' has to be greater than 0. Setting offset to 1.", subname, wip->name);
					offset = 1;
				}
			}
		} else if ( optional_string("+index:") ) {
			stuff_int(&index);
			if ( index < 0 ) {
				Warning(LOCATION, "Substitution '%s' for weapon '%s' requires an index greater than 0. Setting index to 0.", subname, wip->name);
				index = 0;
			}
		} else {
			Warning(LOCATION, "Substitution '%s' for weapon '%s' requires either '+index:' or '+frequency:' to follow. Skipping substitution.", subname, wip->name);
			continue;
		}

		// we are going to use weapon subistution so, make sure that the pattern array has at least one element
		if ( wip->weapon_substitution_pattern_names.empty() ) {
			// pattern is empty, initialize pattern with the weapon being currently parsed.
			wip->weapon_substitution_pattern_names.push_back(wip->name);
		}

		// if tbler specifies a frequency then determine if we can fit the resulting pattern
		// neatly into the pattern array.
		if ( frequency > 0 ) {
			if ( (wip->weapon_substitution_pattern_names.size() % frequency) > 0 ) {
				// not neat, need to expand the pattern so that our freqency pattern fits completly.
				size_t current_size = wip->weapon_substitution_pattern_names.size();
				wip->weapon_substitution_pattern_names.resize(current_size*frequency);

				// now duplicate the current pattern into the new area so the current pattern holds
				for ( size_t i = current_size; i < wip->weapon_substitution_pattern_names.size(); i++) {
					wip->weapon_substitution_pattern_names[i] = wip->weapon_substitution_pattern_names[i%current_size];
				}
			}

			/* Apply the substituted weapon as the requested freqency, barrel
			shifted by offset if needed.*/
			for ( size_t pos = (frequency + offset - 1) % frequency;
				pos < wip->weapon_substitution_pattern_names.size(); pos += frequency )
			{
				if ( pos > 0 ) {
					wip->weapon_substitution_pattern_names[pos] = subname;
				}
			}
		} else {
			// assume that tbler wanted to specify a index for the new weapon.

			// make sure that there is enough room
			if ( !(index < (int)wip->weapon_substitution_pattern_names.size()) ) {
				// need to make the pattern bigger by filling the extra with the current weapon.
				size_t current_size = wip->weapon_substitution_pattern_names.size();
				wip->weapon_substitution_pattern_names.resize(current_size+1, subname);
			}

			wip->weapon_substitution_pattern_names[index] = subname;
		}
	}

	return WEAPON_INFO_INDEX(wip);
}
//Commented out with ifdef
/*
int cmeasure_name_lookup(char *name)
{
	int	i;

	for ( i=0; i < Num_cmeasure_types; i++) {
		if (!stricmp(name, Cmeasure_info[i].cmeasure_name)) {
			return i;
		}
	}

	return -1;
}

void init_cmeasure_entry(int cmeasure_idx)
{
	Assert(cmeasure_idx > -1 && cmeasure_idx < MAX_CMEASURES);
	cmeasure_info *cmeasurep = &Cmeasure_info[cmeasure_idx];

	cmeasurep->max_speed = 20.0f;
	cmeasurep->fire_wait = 0.5f;
	cmeasurep->life_min = 1.0f;
	cmeasurep->life_max = 2.0f;
	cmeasurep->launch_sound = -1;
	cmeasurep->model_num = -1;
	cmeasurep->pof_name[0] = '\0';
}

// function to parse the information for a specific ship type.	
void parse_cmeasure(bool replace)
{
	cmeasure_info *cmeasurep;
	bool create_if_not_found = true;

	cmeasurep = &Cmeasure_info[Num_cmeasure_types];

	char buf[NAME_LENGTH];
	required_string("$Name:");
	stuff_string(buf, F_NAME, NULL);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for countermeasure in non-modular table");
		}
		create_if_not_found = false;
	}

	int c_id = cmeasure_name_lookup(buf);

	if(c_id <= -1)
	{
		if(Num_cmeasure_types >= MAX_CMEASURES) {
			Warning(LOCATION, "Too many countermeasures before '%s'; maximum is %d, so only the first %d will be used", buf, MAX_CMEASURES, Num_cmeasure_types);
			
			//Skip the rest of the ships in non-modular tables, since we can't add them.
			if(!replace) {
				while(skip_to_start_of_string_either("$Name:", "#End"));
			}
			return;
		}

		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
		}

		//Make a new cmeasure
		init_cmeasure_entry(Num_cmeasure_types);
		cmeasurep = &Cmeasure_info[Num_cmeasure_types];
		Num_cmeasure_types++;
		strcpy_s(cmeasurep->cmeasure_name, buf);
	}
	else
	{
		cmeasurep = &Cmeasure_info[c_id];
		if(!replace)
		{
			Warning(LOCATION, "Error:  Countermeasure '%s' already exists.  All countermeasure class names in weapons.tbl must be unique.", cmeasurep->cmeasure_name);
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return;
		}
	}

//$Name:					Type One
//$Velocity:				20.0				;; speed relative to ship, rear-fired until POF info added, MK, 5/22/97
//$Fire Wait:				0.5
//$Lifetime Min:			1.0				;; Minimum lifetime
//$Lifetime Max:			2.0				;; Maximum lifetime.  Actual lifetime is rand(min..max).
//$LaunchSnd:				counter_1.wav,	.8, 10, 300	;; countermeasure 1 fired (sound is 3d)

	if(optional_string("$Velocity:")) {
		stuff_float( &(cmeasurep->max_speed) );
	}

	if(optional_string("$Fire Wait:")) {
		stuff_float( &(cmeasurep->fire_wait) );
	}

	if(optional_string("$Lifetime Min:")) {
		stuff_float(&cmeasurep->life_min);
	}

	if(optional_string("$Lifetime Max:")) {
		stuff_float(&cmeasurep->life_max);
	}

	//Launching sound
	parse_sound("$LaunchSnd:", &cmeasurep->launch_sound, cmeasurep->cmeasure_name);

	if(optional_string("$Model:")) {
		stuff_string(cmeasurep->pof_name, F_NAME, NULL);
	}
}
*/


//	For all weapons that spawn weapons, given an index at weaponp->spawn_type,
// convert the strings in Spawn_names to indices in the Weapon_types array.
void translate_spawn_types()
{
    int	i, j, k;

    for (i = 0; i < Num_weapon_types; i++)
    {
        for (j = 0; j < Weapon_info[i].num_spawn_weapons_defined; j++)
        {
            if ( (Weapon_info[i].spawn_info[j].spawn_type > -1) && (Weapon_info[i].spawn_info[j].spawn_type < Num_spawn_types) )
            {
                int	spawn_type = Weapon_info[i].spawn_info[j].spawn_type;

                Assert( spawn_type < Num_spawn_types );

                for (k = 0; k < Num_weapon_types; k++) 
                {
                    if ( !stricmp(Spawn_names[spawn_type], Weapon_info[k].name) ) 
                    {
                        Weapon_info[i].spawn_info[j].spawn_type = (short)k;

                        if (i == k)
                            Warning(LOCATION, "Weapon %s spawns itself.  Infinite recursion?\n", Weapon_info[i].name);

                        break;
                    }
                }
            }
        }
    }
}

static char Default_cmeasure_name[NAME_LENGTH] = "";

void parse_weaponstbl(char *filename)
{
	int rval;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	if(optional_string("#Primary Weapons"))
	{
		while (required_string_either("#End", "$Name:")) {
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_LASER, Parsing_modular_table) < 0 ) {
				continue;
			}
		}
		required_string("#End");
	}

	if(optional_string("#Secondary Weapons"))
	{
		while (required_string_either("#End", "$Name:")) {
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_MISSILE, Parsing_modular_table) < 0) {
				continue;
			}
		}
		required_string("#End");
	}

	if(optional_string("#Beam Weapons"))
	{
		while (required_string_either("#End", "$Name:")) {
			// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
			if ( parse_weapon(WP_BEAM, Parsing_modular_table) < 0) {
				continue;
			}
		}
		required_string("#End");
	}

	strcpy_s(parse_error_text, "in the counter measure table entry");

	if(optional_string("#Countermeasures"))
	{
		while (required_string_either("#End", "$Name:"))
		{
			int idx = parse_weapon(WP_MISSILE, Parsing_modular_table);

			if(idx < 0) {
				continue;
			}

			//Make sure cmeasure flag is set
			Weapon_info[idx].wi_flags |= WIF_CMEASURE;

			//Set cmeasure index
			if(!strlen(Default_cmeasure_name)) {
				//We can't be sure that index will be the same after sorting, so save the name
				strcpy_s(Default_cmeasure_name, Weapon_info[idx].name);
			}
		}

		required_string("#End");
	}

	strcpy_s(parse_error_text, "");

	// Read in a list of weapon_info indicies that are an ordering of the player weapon precedence.
	// This list is used to select an alternate weapon when a particular weapon is not available
	// during weapon selection.
	if ( (!Parsing_modular_table && required_string("$Player Weapon Precedence:")) || optional_string("$Player Weapon Precedence:") )
	{
		strcpy_s(parse_error_text, "in the player weapon precedence list");
		Num_player_weapon_precedence = stuff_int_list(Player_weapon_precedence, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
		strcpy_s(parse_error_text, "");
	}

	// add tbl/tbm to multiplayer validation list
	fs2netd_add_table_validation(filename);

	// close localization
	lcl_ext_close();
}

void weapon_create_names()
{
	int	i;

	for (i = 0; i < Num_weapon_types; i++)
		Weapon_names[i] = Weapon_info[i].name;
}

//uses a simple bucket sort to sort weapons, order of importance is:
//Lasers
//Beams
//Fighter missiles and bombs
//Capital missiles and bombs
//Child weapons
void weapon_sort_by_type()
{
	weapon_info *lasers = NULL, *big_lasers = NULL, *beams = NULL, *missiles = NULL, *big_missiles = NULL, *child_weapons = NULL;
	int num_lasers = 0, num_big_lasers = 0, num_beams = 0, num_missiles = 0, num_big_missiles = 0, num_child = 0;
	int i, weapon_index;

	// get the initial count of each weapon type
	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		switch (Weapon_info[i].subtype)
		{
			case WP_UNUSED:
				continue;

			case WP_LASER:
				if (Weapon_info[i].wi_flags & WIF_BIG_ONLY)
					num_big_lasers++;
				else
					num_lasers++;
				break;
		
			case WP_BEAM:
				num_beams++;
				break;

			case WP_MISSILE:
				if (Weapon_info[i].wi_flags & WIF_CHILD)
					num_child++;
				else if (Weapon_info[i].wi_flags & WIF_BIG_ONLY)
					num_big_missiles++;
				else
					num_missiles++;
				break;

			default:
				continue;
		}
		
	}

	// allocate the buckets
	if (num_lasers) {
		lasers = new weapon_info[num_lasers];
		Verify( lasers != NULL );
		num_lasers = 0;
	}

	if (num_big_lasers) {
		big_lasers = new weapon_info[num_big_lasers];
		Verify( big_lasers != NULL );
		num_big_lasers = 0;
	}

	if (num_beams) {
		beams = new weapon_info[num_beams];
		Verify( beams != NULL );
		num_beams = 0;
	}

	if (num_missiles) {
		missiles = new weapon_info[num_missiles];
		Verify( missiles != NULL );
		num_missiles = 0;
	}

	if (num_big_missiles) {
		big_missiles = new weapon_info[num_big_missiles];
		Verify( big_missiles != NULL );
		num_big_missiles = 0;
	}

	if (num_child) {
		child_weapons = new weapon_info[num_child];
		Verify( child_weapons != NULL );
		num_child = 0;
	}

	// fill the buckets
	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		switch (Weapon_info[i].subtype)
		{
			case WP_UNUSED:
				continue;

			case WP_LASER:
				if (Weapon_info[i].wi_flags & WIF_BIG_ONLY)
					big_lasers[num_big_lasers++] = Weapon_info[i];
				else
					lasers[num_lasers++] = Weapon_info[i];
				break;
		
			case WP_BEAM:
				beams[num_beams++] = Weapon_info[i];
				break;

			case WP_MISSILE:
				if (Weapon_info[i].wi_flags & WIF_CHILD)
					child_weapons[num_child++] = Weapon_info[i];
				else if (Weapon_info[i].wi_flags & WIF_BIG_ONLY)
					big_missiles[num_big_missiles++] = Weapon_info[i];
				else
					missiles[num_missiles++]=Weapon_info[i];
				break;

			default:
				continue;
		}
	}

	weapon_index = 0;

	// reorder the weapon_info structure according to our rules defined above
	for (i = 0; i < num_lasers; i++, weapon_index++)
		Weapon_info[weapon_index] = lasers[i];

	for (i = 0; i < num_big_lasers; i++, weapon_index++)
		Weapon_info[weapon_index] = big_lasers[i];

	for (i = 0; i < num_beams; i++, weapon_index++)
		Weapon_info[weapon_index] = beams[i];

	// designate start of secondary weapons so that we'll have the correct offset later on
	First_secondary_index = weapon_index;

	for (i = 0; i < num_missiles; i++, weapon_index++)
		Weapon_info[weapon_index] = missiles[i];

	for (i = 0; i < num_big_missiles; i++, weapon_index++)
		Weapon_info[weapon_index] = big_missiles[i];

	for (i = 0; i < num_child; i++, weapon_index++)
		Weapon_info[weapon_index] = child_weapons[i];


	if (lasers)			delete [] lasers;
	if (big_lasers)		delete [] big_lasers;
	if (beams)			delete [] beams;
	if (missiles)		delete [] missiles;
	if (big_missiles)	delete [] big_missiles;
	if (child_weapons)	delete [] child_weapons;
}

// do any post-parse cleaning on weapon entries
void weapon_clean_entries()
{
	weapon_info *wip;
	int i;

	for (i = 0; i < Num_weapon_types; i++) {
		wip = &Weapon_info[i];

		if (wip->wi_flags & WIF_BEAM) {
			// clean up any beam sections which may have been deleted
			int removed = 0;

			for (int s_idx = 0; s_idx < wip->b_info.beam_num_sections; s_idx++) {
				if ( !strlen(wip->b_info.sections[s_idx].texture.filename) ) {
					int new_idx = s_idx + 1;

					while (new_idx < MAX_BEAM_SECTIONS) {
						memcpy( &wip->b_info.sections[new_idx-1], &wip->b_info.sections[new_idx], sizeof(beam_weapon_section_info) );
						new_idx++;
					}

					removed++;
				}
			}

			if (removed) {
				mprintf(("NOTE: weapon-cleanup is removing %i stale beam sections, out of %i original, from '%s'.\n", removed, wip->b_info.beam_num_sections, wip->name));
				wip->b_info.beam_num_sections -= removed;
			}
		}
	}
}

void weapon_release_bitmaps()
{
	int i, j;
	weapon_info *wip;

	// not for FRED...
	if (Fred_running)
		return;

	// if we are just going to load them all again any, keep everything
	if (Cmdline_load_all_weapons)
		return;

	for (i = 0; i < Num_weapon_types; i++) {
		wip = &Weapon_info[i];

		// go ahead and clear out models, the model paging code will actually take care of
		// releasing this stuff if needed, but we have to keep track of current modelnums ourselves
		if (wip->render_type == WRT_POF)
			wip->model_num = -1;

		// we are only interested in what we don't need for this mission
		if ( used_weapons[i] )
			continue;

		if (wip->render_type == WRT_LASER) {
			if (wip->laser_bitmap.first_frame >= 0) {
				bm_release(wip->laser_bitmap.first_frame);
				wip->laser_bitmap.first_frame = -1;
			}
	
			// now for the glow
			if (wip->laser_glow_bitmap.first_frame >= 0) {
				bm_release(wip->laser_glow_bitmap.first_frame);
				wip->laser_glow_bitmap.first_frame = -1;
			}
		}

		if (wip->wi_flags & WIF_BEAM) {
			// particle animation
			if (wip->b_info.beam_particle_ani.first_frame >= 0) {
				bm_release(wip->b_info.beam_particle_ani.first_frame);
				wip->b_info.beam_particle_ani.first_frame = -1;
			}

			// muzzle glow
			if (wip->b_info.beam_glow.first_frame >= 0) {
				bm_release(wip->b_info.beam_glow.first_frame);
				wip->b_info.beam_glow.first_frame = -1;
			}

			// section textures
			for (j = 0; j < wip->b_info.beam_num_sections; j++) {
				beam_weapon_section_info *bsi = &wip->b_info.sections[j];

				if (bsi->texture.first_frame >= 0) {
					bm_release(bsi->texture.first_frame);
					bsi->texture.first_frame = -1;
				}
			}
		}

		if (wip->wi_flags & WIF_TRAIL) {
			if (wip->tr_info.texture.bitmap_id >= 0) {
				bm_release(wip->tr_info.texture.bitmap_id);
				wip->tr_info.texture.bitmap_id = -1;
			}
		}

		if (wip->wi_flags & WIF_PARTICLE_SPEW) { // tweaked for multiple particle spews -nuke
			for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++)  { // just bitmaps that got loaded
				if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE){
					if (wip->particle_spewers[s].particle_spew_anim.first_frame >= 0) {
						bm_release(wip->particle_spewers[s].particle_spew_anim.first_frame);
						wip->particle_spewers[s].particle_spew_anim.first_frame = -1;
					}
				}
			}
		}

		if (wip->thruster_flame.first_frame >= 0) {
			bm_release(wip->thruster_flame.first_frame);
			wip->thruster_flame.first_frame = -1;
		}

		if (wip->thruster_glow.first_frame >= 0) {
			bm_release(wip->thruster_glow.first_frame);
			wip->thruster_glow.first_frame = -1;
		}
	}
}

bool weapon_is_used(int weapon_index)
{
	Assert( (weapon_index >= 0) || (weapon_index < Num_weapon_types) );
	return (used_weapons[weapon_index] > 0);
}

void weapon_load_bitmaps(int weapon_index)
{
	weapon_info *wip;

	// not for FRED...
	if (Fred_running)
		return;

	if ( (weapon_index < 0) || (weapon_index >= Num_weapon_types) ) {
		Int3();
		return;
	}

	wip = &Weapon_info[weapon_index];

	if ( (wip->render_type == WRT_LASER) && (wip->laser_bitmap.first_frame < 0) ) {
		wip->laser_bitmap.first_frame = bm_load(wip->laser_bitmap.filename);

		if (wip->laser_bitmap.first_frame >= 0) {
			wip->laser_bitmap.num_frames = 1;
			wip->laser_bitmap.total_time = 1;
		}
		// fall back to an animated type
		else if ( generic_anim_load(&wip->laser_bitmap) ) {
			mprintf(("Could not find a usable bitmap for '%s'!\n", wip->name));
			Warning(LOCATION, "Could not find a usable bitmap (%s) for weapon '%s'!\n", wip->laser_bitmap.filename, wip->name);
		}

		// now see if we also have a glow
		if ( strlen(wip->laser_glow_bitmap.filename) ) {
			wip->laser_glow_bitmap.first_frame = bm_load(wip->laser_glow_bitmap.filename);

			if (wip->laser_glow_bitmap.first_frame >= 0) {
				wip->laser_glow_bitmap.num_frames = 1;
				wip->laser_glow_bitmap.total_time = 1;
			}
			// fall back to an animated type
			else if ( generic_anim_load(&wip->laser_glow_bitmap) ) {
				mprintf(("Could not find a usable glow bitmap for '%s'!\n", wip->name));
				Warning(LOCATION, "Could not find a usable glow bitmap (%s) for weapon '%s'!\n", wip->laser_glow_bitmap.filename, wip->name);
			}
		}
	}

	if (wip->wi_flags & WIF_BEAM) {
		// particle animation
		if ( (wip->b_info.beam_particle_ani.first_frame < 0) && strlen(wip->b_info.beam_particle_ani.filename) )
			generic_anim_load(&wip->b_info.beam_particle_ani);

		// muzzle glow
		if ( (wip->b_info.beam_glow.first_frame < 0) && strlen(wip->b_info.beam_glow.filename) ) {
			if ( generic_anim_load(&wip->b_info.beam_glow) ) {
				// animated version failed to load, try static instead
				wip->b_info.beam_glow.first_frame = bm_load(wip->b_info.beam_glow.filename);

				if (wip->b_info.beam_glow.first_frame >= 0) {
					wip->b_info.beam_glow.num_frames = 1;
					wip->b_info.beam_glow.total_time = 1;
				} else {
					mprintf(("Could not find a usable muzzle glow bitmap for '%s'!\n", wip->name));
					Warning(LOCATION, "Could not find a usable muzzle glow bitmap (%s) for weapon '%s'!\n", wip->b_info.beam_glow.filename, wip->name);
				}
			}
		}

		// section textures
		for (int i = 0; i < wip->b_info.beam_num_sections; i++) {
			beam_weapon_section_info *bsi = &wip->b_info.sections[i];

			if ( (bsi->texture.first_frame < 0) && strlen(bsi->texture.filename) ) {
				if ( generic_anim_load(&bsi->texture) ) {
					// animated version failed to load, try static instead
					bsi->texture.first_frame = bm_load(bsi->texture.filename);

					if (bsi->texture.first_frame >= 0) {
						bsi->texture.num_frames = 1;
						bsi->texture.total_time = 1;
					} else {
						mprintf(("Could not find a usable beam section (%i) bitmap for '%s'!\n", i, wip->name));
						Warning(LOCATION, "Could not find a usable beam section (%i) bitmap (%s) for weapon '%s'!\n", i, bsi->texture.filename, wip->name);
					}
				}
			}
		}
	}

	if ( (wip->wi_flags & WIF_TRAIL) && (wip->tr_info.texture.bitmap_id < 0) )
		generic_bitmap_load(&wip->tr_info.texture);

	//WMC - Don't try to load an anim if no anim is specified, Mmkay?
	if (wip->wi_flags & WIF_PARTICLE_SPEW) {
		for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {	// looperfied for multiple pspewers -nuke
			if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE){

				if ((wip->particle_spewers[s].particle_spew_anim.first_frame < 0) 
					&& (strlen(wip->particle_spewers[s].particle_spew_anim.filename) > 0) ) {

					wip->particle_spewers[s].particle_spew_anim.first_frame = bm_load(wip->particle_spewers[s].particle_spew_anim.filename);

					if (wip->particle_spewers[s].particle_spew_anim.first_frame >= 0) {
						wip->particle_spewers[s].particle_spew_anim.num_frames = 1;
						wip->particle_spewers[s].particle_spew_anim.total_time = 1;
					}
					// fall back to an animated type
					else if ( generic_anim_load(&wip->particle_spewers[s].particle_spew_anim) ) {
						mprintf(("Could not find a usable particle spew bitmap for '%s'!\n", wip->name));
						Warning(LOCATION, "Could not find a usable particle spew bitmap (%s) for weapon '%s'!\n", wip->particle_spewers[s].particle_spew_anim.filename, wip->name);
					}
				}
			}
		}
	}

	// load alternate thruster textures
	if (strlen(wip->thruster_flame.filename)) {
		generic_anim_load(&wip->thruster_flame);
	}

	if (strlen(wip->thruster_glow.filename)) {
		wip->thruster_glow.first_frame = bm_load(wip->thruster_glow.filename);
		if (wip->thruster_glow.first_frame >= 0) {
			wip->thruster_glow.num_frames = 1;
			wip->thruster_glow.total_time = 1;
		} else {
			generic_anim_load(&wip->thruster_glow);
		}
	}

	// if this weapon isn't already marked as used, then mark it as such now
	// (this should really only happen if the player is cheating)
	if ( !used_weapons[weapon_index] )
		used_weapons[weapon_index]++;
}

/* checks all of the weapon infos for subsitution patterns 
and caches the weapon_index of any that it finds. */
void weapon_generate_indexes_for_subsitution() {
	for (int i = 0; i < MAX_WEAPON_TYPES; i++) {
		weapon_info *wip = &(Weapon_info[i]);

		if ( wip->weapon_substitution_pattern_names.size() > 0 ) {
			wip->weapon_substitution_pattern.resize(wip->weapon_substitution_pattern_names.size());

			for ( size_t j = 0; j < wip->weapon_substitution_pattern_names.size(); j++ ) {
				int weapon_index = -1;
				if ( stricmp("none", wip->weapon_substitution_pattern_names[j].c_str()) != 0 ) {
					weapon_index = weapon_info_lookup(wip->weapon_substitution_pattern_names[j].c_str());
					if ( weapon_index == -1 ) { // invalid sub weapon
						Warning(LOCATION, "Weapon '%s' requests substitution with '%s' which does not seem to exist",
							wip->name, wip->weapon_substitution_pattern_names[j].c_str());
						continue;
					}
				}

				wip->weapon_substitution_pattern[j] = weapon_index;
			}

			wip->weapon_substitution_pattern_names.empty();
		}
	}
}

void weapon_do_post_parse()
{
	weapon_info *wip;
	int first_cmeasure_index = -1;
	int i;

	weapon_sort_by_type();	// NOTE: This has to be first thing!
	weapon_create_names();
	weapon_clean_entries();
	weapon_generate_indexes_for_subsitution();

	Default_cmeasure_index = -1;

	// run through weapons list and deal with individual issues
	for (i = 0; i < Num_weapon_types; i++) {
		wip = &Weapon_info[i];

		// set default counter-measure index from the saved name
		if ( (Default_cmeasure_index < 0) && strlen(Default_cmeasure_name) ) {
			if ( !stricmp(wip->name, Default_cmeasure_name) ) {
				Default_cmeasure_index = i;
			}
		}

		// catch a fall back cmeasure index, just in case
		if ( (first_cmeasure_index < 0) && (wip->wi_flags & WIF_CMEASURE) )
			first_cmeasure_index = i;

		// if we are a "#weak" weapon then popup a warning if we don't have the "player allowed" flag set
		if ( !(wip->wi_flags & WIF_PLAYER_ALLOWED) && stristr(wip->name, "#weak") ) {
			int idx = -1;
			char non_weak[NAME_LENGTH];

			strncpy(non_weak, wip->name, strlen(wip->name) - 5);
			idx = weapon_info_lookup(non_weak);

			// only add the flag if the non-weak version is also player-allowed
			if ( (idx >= 0) && (Weapon_info[idx].wi_flags & WIF_PLAYER_ALLOWED) ) {
				Warning(LOCATION, "Weapon '%s' requires the \"player allowed\" flag, but it's not listed!  Adding it by default.\n", wip->name);
				wip->wi_flags |= WIF_PLAYER_ALLOWED;
			}
		}
	}

	// catch cmeasure fallback
	if (Default_cmeasure_index < 0)
		Default_cmeasure_index = first_cmeasure_index;

	// translate all spawn type weapons to referrence the appropriate spawned weapon entry
	translate_spawn_types();
}

void weapon_expl_info_init()
{
	int i;

	parse_weapon_expl_tbl("weapon_expl.tbl");

	// check for, and load, modular tables
	parse_modular_table(NOX("*-wxp.tbm"), parse_weapon_expl_tbl);

	// we've got our list so pass it off for final checking and loading
	for (i = 0; i < (int)LOD_checker.size(); i++) {
		Weapon_explosions.Load( LOD_checker[i].filename, LOD_checker[i].num_lods );
	}

	// done
	LOD_checker.clear();
}

void weapon_reset_info()
{
	memset( Weapon_info, 0, sizeof(weapon_info) * MAX_WEAPON_TYPES );

	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
		init_weapon_entry(i);
}

// This will get called once at game startup
void weapon_init()
{
	if ( !Weapons_inited ) {
		//Init weapon explosion info
		weapon_expl_info_init();

		// parse weapons.tbl
		weapon_reset_info();

		Num_weapon_types = 0;
		Num_spawn_types = 0;

		parse_weaponstbl("weapons.tbl");

		parse_modular_table(NOX("*-wep.tbm"), parse_weaponstbl);

		// do post-parse cleanup
		weapon_do_post_parse();

		Weapons_inited = 1;
	}

	weapon_level_init();
}


// call from game_shutdown() only!!
void weapon_close()
{
	int i;

	for (i = 0; i<MAX_WEAPON_TYPES; i++) {
		if (Weapon_info[i].desc) {
			vm_free(Weapon_info[i].desc);
			Weapon_info[i].desc = NULL;
		}

		if (Weapon_info[i].tech_desc) {
			vm_free(Weapon_info[i].tech_desc);
			Weapon_info[i].tech_desc = NULL;
		}
	}

	if (used_weapons != NULL) {
		delete[] used_weapons;
		used_weapons = NULL;
	}

	if (Spawn_names != NULL) {
		for (i=0; i<Num_spawn_types; i++) {
			if (Spawn_names[i] != NULL) {
				vm_free(Spawn_names[i]);
				Spawn_names[i] = NULL;
			}
		}

		vm_free(Spawn_names);
		Spawn_names = NULL;
	}
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

	for (i=0; i<MAX_WEAPON_TYPES; i++)	{
		Weapon_info[i].damage_type_idx = Weapon_info[i].damage_type_idx_sav;
		Weapon_info[i].shockwave.damage_type_idx = Weapon_info[i].shockwave.damage_type_idx_sav;
	}

	trail_level_init();		// reset all missile trails

	swarm_level_init();
	missile_obj_list_init();
	
	cscrew_level_init();

	// emp effect
	emp_level_init();

	if (used_weapons == NULL)
		used_weapons = new int[Num_weapon_types];

	Assert( used_weapons != NULL );

	// clear out used_weapons between missions
	if (used_weapons != NULL)
		memset(used_weapons, 0, Num_weapon_types * sizeof(int));

	Weapon_flyby_sound_timer = timestamp(0);
	Weapon_impact_timer = 1;	// inited each level, used to reduce impact sounds
}

MONITOR( NumWeaponsRend )

const float weapon_glow_scale_f = 2.3f;
const float weapon_glow_scale_r = 2.3f;
const float weapon_glow_scale_l = 1.5f;
const int weapon_glow_alpha = 217; // (0.85 * 255);

void weapon_render(object *obj)
{
	int num;
	weapon_info *wip;
	weapon *wp;
	color c;

	MONITOR_INC(NumWeaponsRend, 1);

	Assert(obj->type == OBJ_WEAPON);

	num = obj->instance;
	wp = &Weapons[num];
	wip = &Weapon_info[Weapons[num].weapon_info_index];

	if (wip->wi_flags2 & WIF2_TRANSPARENT) {
		if (wp->alpha_current == -1.0f) {
			wp->alpha_current = wip->alpha_max;
		} else if (wip->alpha_cycle > 0.0f) {
			if (wp->alpha_backward) {
				wp->alpha_current += wip->alpha_cycle;

				if (wp->alpha_current > wip->alpha_max) {
					wp->alpha_current = wip->alpha_max;
					wp->alpha_backward = 0;
				}
			} else {
				wp->alpha_current -= wip->alpha_cycle;

				if (wp->alpha_current < wip->alpha_min) {
					wp->alpha_current = wip->alpha_min;
					wp->alpha_backward = 1;
				}
			}
		}
	}

	switch (wip->render_type)
	{
		case WRT_LASER:
		{
			// turn off fogging for good measure
			gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
			int alpha = 255;
			int framenum = 0;

			if (wip->laser_bitmap.first_frame >= 0) {					
				gr_set_color_fast(&wip->laser_color_1);

				if (wip->laser_bitmap.num_frames > 1) {
					wp->laser_bitmap_frame += flFrametime;

					// Sanity checks
					if (wp->laser_bitmap_frame < 0.0f)
						wp->laser_bitmap_frame = 0.0f;
					if (wp->laser_bitmap_frame > 100.0f)
						wp->laser_bitmap_frame = 0.0f;

					while (wp->laser_bitmap_frame > wip->laser_bitmap.total_time)
						wp->laser_bitmap_frame -= wip->laser_bitmap.total_time;

					framenum = fl2i( (wp->laser_bitmap_frame * wip->laser_bitmap.num_frames) / wip->laser_bitmap.total_time );

					CLAMP(framenum, 0, wip->laser_bitmap.num_frames-1);
				}

				if (wip->wi_flags2 & WIF2_TRANSPARENT)
					alpha = fl2i(wp->alpha_current * 255.0f);

				vec3d headp;
				vm_vec_scale_add(&headp, &obj->pos, &obj->orient.vec.fvec, wip->laser_length);
				wp->weapon_flags &= ~WF_CONSIDER_FOR_FLYBY_SOUND;

				if ( batch_add_laser(wip->laser_bitmap.first_frame + framenum, &headp, wip->laser_head_radius, &obj->pos, wip->laser_tail_radius, alpha, alpha, alpha) ) {
					wp->weapon_flags |= WF_CONSIDER_FOR_FLYBY_SOUND;
				}
			}			

			// maybe draw laser glow bitmap
			if (wip->laser_glow_bitmap.first_frame >= 0) {
    			// get the laser color
				weapon_get_laser_color(&c, obj);

                // *Tail point "getting bigger" as well as headpoint isn't being taken into consideration, so
                //  it caused uneven glow between the head and tail, which really shows in big lasers. So...fixed!    -Et1
				vec3d headp2, tailp;

				vm_vec_scale_add(&headp2, &obj->pos, &obj->orient.vec.fvec, wip->laser_length * weapon_glow_scale_l);
                vm_vec_scale_add(&tailp, &obj->pos, &obj->orient.vec.fvec, wip->laser_length * (1 -  weapon_glow_scale_l) );

				framenum = 0;

				if (wip->laser_glow_bitmap.num_frames > 1) {
					wp->laser_glow_bitmap_frame += flFrametime;

					// Sanity checks
					if (wp->laser_glow_bitmap_frame < 0.0f)
						wp->laser_glow_bitmap_frame = 0.0f;
					if (wp->laser_glow_bitmap_frame > 100.0f)
						wp->laser_glow_bitmap_frame = 0.0f;

					while (wp->laser_glow_bitmap_frame > wip->laser_glow_bitmap.total_time)
						wp->laser_glow_bitmap_frame -= wip->laser_glow_bitmap.total_time;

					framenum = fl2i( (wp->laser_glow_bitmap_frame * wip->laser_glow_bitmap.num_frames) / wip->laser_glow_bitmap.total_time );

					CLAMP(framenum, 0, wip->laser_glow_bitmap.num_frames-1);
				}

				if (wip->wi_flags2 & WIF2_TRANSPARENT) {
					alpha = fl2i(wp->alpha_current * 255.0f);
					alpha -= 38; // take 1.5f into account for the normal glow alpha

					if (alpha < 0)
						alpha = 0;
				} else {
					alpha = weapon_glow_alpha;
				}

				batch_add_laser(wip->laser_glow_bitmap.first_frame + framenum, &headp2, wip->laser_head_radius * weapon_glow_scale_f, &tailp, wip->laser_tail_radius * weapon_glow_scale_r, (c.red*alpha)/255, (c.green*alpha)/255, (c.blue*alpha)/255);
			}

			break;
		}

		case WRT_POF:
		{
			uint render_flags = MR_NORMAL|MR_IS_MISSILE|MR_NO_LIGHTING;

			if (Cmdline_missile_lighting && !(wip->wi_flags2 & WIF2_MR_NO_LIGHTING))
				render_flags &= ~MR_NO_LIGHTING;

			if (wip->wi_flags2 & WIF2_TRANSPARENT) {
				model_set_alpha(wp->alpha_current);
				render_flags |= MR_ALL_XPARENT;
			}

			model_clear_instance(wip->model_num);

			if ( (wip->wi_flags & WIF_THRUSTER) && ((wp->thruster_bitmap > -1) || (wp->thruster_glow_bitmap > -1)) ) {
				float ft;
				mst_info mst;

				//	Add noise to thruster geometry.
				//ft = obj->phys_info.forward_thrust;					
				ft = 1.0f;		// Always use 1.0f for missiles					
				ft *= (1.0f + frand()/5.0f - 1.0f/10.0f);
				if (ft > 1.0f)
					ft = 1.0f;

				mst.length.xyz.x = ft;
				mst.length.xyz.y = ft;
				mst.length.xyz.z = ft;

				mst.primary_bitmap = wp->thruster_bitmap;
				mst.primary_glow_bitmap = wp->thruster_glow_bitmap;
				mst.glow_rad_factor = wip->thruster_glow_factor;
				mst.glow_noise = wp->thruster_glow_noise;

				model_set_thrust(wip->model_num, &mst);

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

			break;
		}

		default:
			Warning(LOCATION, "Unknown weapon rendering type = %i\n", wip->render_type);
	}
}

void weapon_delete(object *obj)
{
	weapon *wp;
	int num;

	Script_system.SetHookObjects(2, "Weapon", obj, "Self", obj);
	Script_system.RunCondition(CHA_ONWEAPONDELETE);
	Script_system.RemHookVars(2, "Weapon", "Self");

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

/*
	if (wp->flak_index >= 0){
		flak_delete(wp->flak_index);
		wp->flak_index = -1;
	}
*/

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
			// Use heatlock-warning sound for Heat and Javelin for now
			// Possibly add an additional third sound later
			if ( Weapon_info[wp->weapon_info_index].wi_flags & WIF_HOMING_HEAT ||
				 Weapon_info[wp->weapon_info_index].wi_flags & WIF_HOMING_JAVELIN ) {
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
void detonate_nearby_missiles(object *killer_objp)
{
	if(killer_objp->type != OBJ_WEAPON) {
		Int3();
		return;
	}

	missile_obj	*mop;

	mop = GET_FIRST(&Missile_obj_list);
	while(mop != END_OF_LIST(&Missile_obj_list)) {
		object	*objp;
		weapon	*wp;

		objp = &Objects[mop->objnum];
		wp = &Weapons[objp->instance];

		if (iff_x_attacks_y(Weapons[killer_objp->instance].team, wp->team)) {
			if ( Missiontime - wp->creation_time > F1_0/2) {
				if (vm_vec_dist_quick(&killer_objp->pos, &objp->pos) < CMEASURE_DETONATE_DISTANCE) {
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
	object      *objp, *old_homing_objp;
	weapon_info *wip;
	weapon      *wp;
    ship        *sp;
    ship_info   *sip;
	float       best_dist;
    int         homing_object_team;
    float       dist;
    float       dot;
    vec3d       vec_to_object;
    ship_subsys *target_engines = NULL;

	wp = &Weapons[num];

	wip = &Weapon_info[Weapons[num].weapon_info_index];

	best_dist = 99999.9f;

	// save the old homing object so that multiplayer servers can give the right information
	// to clients if the object changes
	old_homing_objp = wp->homing_object;

	wp->homing_object = &obj_used_list;

	//	Scan all objects, find a weapon to home on.
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_WEAPON) && (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CMEASURE)))
		{
			//WMC - Countermeasure duds were never implemented fully
			/*
			if (objp->type == OBJ_CMEASURE)
			{
				if (Cmeasures[objp->instance].flags & CMF_DUD_HEAT)
					continue;
			}*/

			//WMC - Spawn weapons shouldn't go for protected ships
			if((objp->flags & OF_PROTECTED) && (wp->weapon_flags & WF_SPAWNED))
				continue;

			// Spawned weapons should never home in on their parent - even in multiplayer dogfights where they would pass the iff test below
			if ((wp->weapon_flags & WF_SPAWNED) && (objp == &Objects[weapon_objp->parent]))
				continue; 

			homing_object_team = obj_team(objp);
			if (iff_x_attacks_y(wp->team, homing_object_team))
			{
				if ( objp->type == OBJ_SHIP )
                {
                    sp  = &Ships[objp->instance];
                    sip = &Ship_info[sp->ship_info_index];

                    //if the homing weapon is a huge weapon and the ship that is being
                    //looked at is not huge, then don't home
                    if ((wip->wi_flags & WIF_HUGE) &&
                        (sip->flags & (SIF_SMALL_SHIP | SIF_NOT_FLYABLE | SIF_HARMLESS)))
                    {
                        continue;
                    }

					// AL 2-17-98: If ship is immune to sensors, can't home on it (Sandeep says so)!
					if ( sp->flags & SF_HIDDEN_FROM_SENSORS ) {
						continue;
					}

					// Goober5000: if missiles can't home on sensor-ghosted ships,
					// they definitely shouldn't home on stealth ships
					if ( sp->flags2 & SF2_STEALTH && (The_mission.ai_profile->flags & AIPF_FIX_HEAT_SEEKER_STEALTH_BUG) ) {
						continue;
					}

                    if (wip->wi_flags & WIF_HOMING_JAVELIN)
                    {
                        target_engines = ship_get_closest_subsys_in_sight(sp, SUBSYSTEM_ENGINE, &weapon_objp->pos);

                        if (!target_engines)
                            continue;
                    }

					//	MK, 9/4/99.
					//	If this is a player object, make sure there aren't already too many homers.
					//	Only in single player.  In multiplayer, we don't want to restrict it in dogfight on team vs. team.
					//	For co-op, it's probably also OK.
					if (!( Game_mode & GM_MULTIPLAYER )) {
						int	num_homers = compute_num_homing_objects(objp);
						if (The_mission.ai_profile->max_allowed_player_homers[Game_skill_level] < num_homers)
							continue;
					}
				}
                else if (objp->type == OBJ_WEAPON)
				{
                    //don't attempt to home on weapons if the weapon is a huge weapon or is a javelin homing weapon.
                    if (wip->wi_flags & (WIF_HUGE | WIF_HOMING_JAVELIN))
                        continue;
                    
                    //don't look for local ssms that are gone for the time being
					if (Weapons[objp->instance].lssm_stage == 3)
						continue;
				}

				dist = vm_vec_normalized_dir(&vec_to_object, &objp->pos, &weapon_objp->pos);

				if (objp->type == OBJ_WEAPON && (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CMEASURE)) {
					dist *= 0.5f;
				}

				dot = vm_vec_dot(&vec_to_object, &weapon_objp->orient.vec.fvec);

				if (dot > wip->fov) {
					if (dist < best_dist) {
						best_dist = dist;
						wp->homing_object	= objp;
						wp->target_sig		= objp->signature;
						wp->homing_subsys	= target_engines;

						cmeasure_maybe_alert_success(objp);
					}
				}
			}
		}
	}

	if (wp->homing_object == Player_obj)
		weapon_maybe_play_warning(wp);

	// if the old homing object is different that the new one, send a packet to clients
	if ( MULTIPLAYER_MASTER && (old_homing_objp != wp->homing_object) ) {
		send_homing_weapon_info( num );
	}
}

//	Scan all countermeasures.  Maybe make weapon_objp home on it.
void find_homing_object_cmeasures_1(object *weapon_objp)
{
	object	*objp;
	weapon	*wp, *cm_wp;
	weapon_info	*wip, *cm_wip;
	float		best_dot, dist, dot;

	wp = &Weapons[weapon_objp->instance];
	wip = &Weapon_info[wp->weapon_info_index];

	best_dot = wip->fov;			//	Note, setting to this avoids comparison below.

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		//first check if its a weapon, then setup the pointers
		if (objp->type == OBJ_WEAPON)
		{
			cm_wp = &Weapons[objp->instance];
			cm_wip = &Weapon_info[cm_wp->weapon_info_index];

			if (cm_wip->wi_flags & WIF_CMEASURE)
			{
				//don't have a weapon try to home in on itself
				if (objp==weapon_objp)
					continue;

				//don't have a weapon try to home in on missiles fired by the same team, unless its the traitor team.
				if ((wp->team == cm_wp->team) && (wp->team != Iff_traitor))
					continue;

				vec3d	vec_to_object;
				dist = vm_vec_normalized_dir(&vec_to_object, &objp->pos, &weapon_objp->pos);

				if (dist < cm_wip->cm_effective_rad)
				{
					float	chance;
					if (wip->wi_flags & WIF_HOMING_ASPECT) {
						// aspect seeker this likely to chase a countermeasure
						chance = cm_wip->cm_aspect_effectiveness/wip->seeker_strength;
					} else {
						// heat seeker and javelin HS this likely to chase a countermeasure
						chance = cm_wip->cm_heat_effectiveness/wip->seeker_strength;
					}
					if ((objp->signature != wp->cmeasure_ignore_objnum) && (objp->signature != wp->cmeasure_chase_objnum))
					{
						if (frand() >= chance) {
							wp->cmeasure_ignore_objnum = objp->signature;	//	Don't process this countermeasure again.
						//	mprintf(("Frame %i: Weapon #%i ignoring cmeasure #%i\n", Framecount, OBJ_INDEX(weapon_objp), objp->signature));
						} else  {
							wp->cmeasure_chase_objnum = objp->signature;	//	Don't process this countermeasure again.
						//	mprintf(("Frame %i: Weapon #%i CHASING cmeasure #%i\n", Framecount, OBJ_INDEX(weapon_objp), objp->signature));
						}
					}
				
					if (objp->signature != wp->cmeasure_ignore_objnum)
					{

						dot = vm_vec_dot(&vec_to_object, &weapon_objp->orient.vec.fvec);

						if (dot > best_dot)
						{
							//nprintf(("Jim", "Frame %i: Weapon #%i homing on cmeasure #%i\n", Framecount, weapon_objp-Objects, objp->signature));
							best_dot = dot;
							wp->homing_object = objp;
							cmeasure_maybe_alert_success(objp);
						}
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

	// if the old homing object is different that the new one, send a packet to clients
	if ( MULTIPLAYER_MASTER && (old_homing_objp != wp->homing_object) ) {
		send_homing_weapon_info( weapon_objp->instance );
	}
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
	if ((hobjp == &obj_used_list) || ( f2fl(Missiontime - wp->creation_time) < wip->free_flight_time )) {
		//	If this is a heat seeking homing missile and 1/2 second has elapsed since firing, find a new target.
        if ((wip->wi_flags & WIF_HOMING_HEAT) &&
            (f2fl(Missiontime - wp->creation_time) > wip->free_flight_time))
        {
            find_homing_object(obj, num);
        }
		else if (MULTIPLAYER_MASTER && (wip->wi_flags & WIF_LOCKED_HOMING) && (wp->weapon_flags & WF_HOMING_UPDATE_NEEDED)) {
			wp->weapon_flags &= ~WF_HOMING_UPDATE_NEEDED; 
			send_homing_weapon_info(num);
		}

		if (obj->phys_info.speed > max_speed) {
			obj->phys_info.speed -= frame_time / wip->free_flight_speed;
			vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);
		} else if ((obj->phys_info.speed < max_speed * wip->free_flight_speed) && (wip->wi_flags & WIF_HOMING_HEAT)) {
			obj->phys_info.speed = max_speed * wip->free_flight_speed;
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

	// AL 4-8-98: If orgiginal target for aspect or javelin HS lock missile is lost, stop homing
	if (wip->wi_flags & WIF_LOCKED_HOMING) {
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

	// If target subsys is dead make missile pick random spot on target as attack point.
	if (wp->homing_subsys != NULL) {
		if (wp->homing_subsys->flags & SSF_MISSILES_IGNORE_IF_DEAD) {
			if ((wp->homing_subsys->max_hits > 0) && (wp->homing_subsys->current_hits <= 0)) {
				wp->homing_object = &obj_used_list;
				return;
			}
		}
	}

	// Make sure Javelin HS missiles always home on engine subsystems if ships
	if (wip->wi_flags & WIF_HOMING_JAVELIN &&
		hobjp->type == OBJ_SHIP &&
		wp->target_sig > 0 &&
		wp->homing_subsys != NULL &&
		wp->homing_subsys->system_info->type != SUBSYSTEM_ENGINE) {
			ship *enemy = &Ships[ship_get_by_signature(wp->target_sig)];
			wp->homing_subsys = ship_get_closest_subsys_in_sight(enemy, SUBSYSTEM_ENGINE, &Objects[wp->objnum].pos);
	}

	// If Javelin HS missile doesn't home in on a subsystem but homing in on a
	// ship, lose lock alltogether
	// Javelins can only home in one Engines or bombs.
	if (wip->wi_flags & WIF_HOMING_JAVELIN &&
		hobjp->type == OBJ_SHIP &&
		wp->target_sig > 0 &&
		wp->homing_subsys == NULL) {
			wp->homing_object = &obj_used_list;
			return;
	}

/*
	if (hobjp->type == OBJ_NONE) {
		find_homing_object(obj, num);
		return;
	}
*/

	switch (hobjp->type) {
	case OBJ_NONE:
		if (wip->wi_flags & WIF_LOCKED_HOMING) {
			find_homing_object_by_sig(obj, wp->target_sig);
		} else {
			find_homing_object(obj, num);
		}
		return;
		break;
	case OBJ_SHIP:
		if (hobjp->signature != wp->target_sig) {
			if (wip->wi_flags & WIF_LOCKED_HOMING) {
				find_homing_object_by_sig(obj, wp->target_sig);
			} else {
				find_homing_object(obj, num);
			}
			return;
		}
		break;
	case OBJ_WEAPON:
		// don't home on countermeasures or non-bombs, that's handled elsewhere
		if ( (Weapon_info[Weapons[hobjp->instance].weapon_info_index].wi_flags & WIF_CMEASURE) || !(Weapon_info[Weapons[hobjp->instance].weapon_info_index].wi_flags & WIF_BOMB) )
			break;

		if (wip->wi_flags & WIF_LOCKED_HOMING) {
			find_homing_object_by_sig(obj, wp->target_sig);
		} else {
			find_homing_object(obj, num);
		}
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
		if ( (wp->homing_subsys != NULL) && !(wip->wi_flags2 & WIF2_NON_SUBSYS_HOMING) ) {
			get_subsystem_world_pos(hobjp, Weapons[num].homing_subsys, &target_pos);
			wp->homing_pos = target_pos;	// store the homing position in weapon data
			Assert( !vm_is_vec_nan(&wp->homing_pos) );
		} else {
			float	fov;
			float	dist;

			dist = vm_vec_dist_quick(&obj->pos, &hobjp->pos);
			if (hobjp->type == OBJ_WEAPON && (Weapon_info[Weapons[hobjp->instance].weapon_info_index].wi_flags & WIF_CMEASURE))
			{
				if (dist < CMEASURE_DETONATE_DISTANCE)
				{
					//	Make this missile detonate soon.  Not right away, not sure why.  Seems better.
					if (iff_x_attacks_y(Weapons[hobjp->instance].team, wp->team)) {
						detonate_nearby_missiles(hobjp);
						//nprintf(("AI", "Frame %i: Weapon %i hit cmeasure, will die!\n", Framecount, wp-Weapons));
						return;
					}
				}
			}

			fov = -1.0f;

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

					if ( pick_homing_point && !(wip->wi_flags2 & WIF2_NON_SUBSYS_HOMING) ) {
						// If *any* player is parent of homing missile, then use position where lock indicator is
						if ( Objects[obj->parent].flags & OF_PLAYER_SHIP ) {
							player *pp;

							// determine the player
							pp = Player;

							if ( Game_mode & GM_MULTIPLAYER ) {
								int pnum;

								pnum = multi_find_player_by_object( &Objects[obj->parent] );
								if ( pnum != -1 ){
									pp = Net_players[pnum].m_player;
								}
							}

							// If player has apect lock, we don't want to find a homing point on the closest
							// octant... setting the timestamp to 0 ensures this.
							if (wip->wi_flags & WIF_LOCKED_HOMING) {
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
		if ((dist_to_target < flFrametime * obj->phys_info.speed * 4.0f + 10.0f) &&
            (old_dot < wip->fov) &&
            (wp->lifeleft > 0.01f) &&
            (wp->homing_object) &&
            (wp->homing_object->type == OBJ_SHIP))
        {
            wp->lifeleft = 0.01f;
        }

		//	Only lead target if more than one second away.  Otherwise can miss target.  I think this
		//	is what's causing Harbingers to miss the super destroyer. -- MK, 4/15/98
		if ((old_dot > 0.1f) && (time_to_target > 0.1f)) {
			if (wip->wi_flags2 & WIF2_VARIABLE_LEAD_HOMING) {
				vm_vec_scale_add2(&target_pos, &hobjp->phys_info.vel, (0.33f * wip->target_lead_scaler * MIN(time_to_target, 6.0f)));
			} else if (wip->wi_flags & WIF_LOCKED_HOMING) {
				vm_vec_scale_add2(&target_pos, &hobjp->phys_info.vel, MIN(time_to_target, 2.0f));
			}
		}

		//nprintf(("AI", "Dot = %7.3f, dist = %7.3f, time_to = %6.3f, deg/sec = %7.3f\n", old_dot, dist_to_target, time_to_target, angles/flFrametime));

		// nprintf(("AI", "Weapon %i, lifeleft = %7.3f, dist = %7.3f, dot = %6.3f\n", num, Weapons[num].lifeleft, vm_vec_dist_quick(&obj->pos, &Weapons[num].homing_object->pos), old_dot));

		//	If a HEAT seeking (rather than ASPECT seeking) homing missile, verify that target is in viewcone.
		if (wip->wi_flags & WIF_HOMING_HEAT) {
			if ((old_dot < wip->fov) && (dist_to_target > wip->shockwave.inner_rad*1.1f)) {	//	Delay finding new target one frame to allow detonation.
				find_homing_object(obj, num);
				return;			//	Maybe found a new homing object.  Return, process more next frame.
			} else	//	Subtract out life based on how far from target this missile points.
				if ((wip->fov < 0.95f) && !(wip->wi_flags2 & WIF2_NO_LIFE_LOST_IF_MISSED)) {
					wp->lifeleft -= flFrametime * (0.95f - old_dot);
					//Should only happen when time is compressed.
					//if (flFrametime * (1.0f - old_dot) > 1.0f)
					//	Int3();
				}
		} else if (wip->wi_flags & WIF_LOCKED_HOMING) {	//	subtract life as if max turn is 90 degrees.
			if ((wip->fov < 0.95f) && !(wip->wi_flags2 & WIF2_NO_LIFE_LOST_IF_MISSED))
				wp->lifeleft -= flFrametime * (0.95f - old_dot);
		} else {
			Warning(LOCATION, "Tried to make weapon '%s' home, but found it wasn't aspect-seeking or heat-seeking or a Javelin!", wip->name);
		}


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
	if(obj->type != OBJ_WEAPON)
		return;

	weapon *wp = &Weapons[obj->instance];
	weapon_info *wip = &Weapon_info[wp->weapon_info_index];

	// if the object is a corkscrew style weapon, process it now
	if((obj->type == OBJ_WEAPON) && (Weapons[obj->instance].cscrew_index >= 0)){
		cscrew_process_pre(obj);
	}

	// if the weapon is a flak weapon, maybe detonate it early
	/*
	if((wip->wi_flags & WIF_FLAK) && (wp->flak_index >= 0)){
		flak_maybe_detonate(obj);		
	}*/

	//WMC - Originally flak_maybe_detonate, moved here.
	if(wp->det_range > 0.0f)
	{
		vec3d temp;
		vm_vec_sub(&temp, &obj->pos, &wp->start_pos);
		if(vm_vec_mag(&temp) >= wp->det_range){
			weapon_detonate(obj);		
		}
	}

	//WMC - Maybe detonate weapon anyway!
	if(wip->det_radius > 0.0f)
	{
		if((wp->homing_object != NULL) && (wp->homing_object->type != 0))
		{
			if(vm_vec_dist(&wp->homing_pos, &obj->pos) <= wip->det_radius)
			{
				weapon_detonate(obj);
			}
		} else if(wp->target_num > -1)
		{
			if(vm_vec_dist(&obj->pos, &Objects[wp->target_num].pos) <= wip->det_radius)
			{
				weapon_detonate(obj);
			}
		}
	}
}

int	Homing_hits = 0, Homing_misses = 0;


MONITOR( NumWeapons )

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

		if ( (Weapon_info[wp->weapon_info_index].wi_flags & WIF_CORKSCREW) ) {
			dist = vm_vec_dist_quick(&weapon_objp->last_pos, &Eye_position);
		} else {
			dist = vm_vec_dist_quick(&weapon_objp->pos, &Eye_position);
		}

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
			if(Game_mode & GM_MULTIPLAYER){				
				if ( !MULTIPLAYER_CLIENT || (MULTIPLAYER_CLIENT && (wp->lifeleft < -2.0f)) || (MULTIPLAYER_CLIENT && (wip->wi_flags & WIF_CHILD))) {					// don't call this function multiplayer client -- host will send this packet to us
					// nprintf(("AI", "Frame %i: Weapon %i detonated, dist = %7.3f!\n", Framecount, obj-Objects));
					weapon_detonate(obj);					
				}
			} else {
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
	if (((wip->wi_flags & WIF_BOMB) || (wip->wi_flags2 & WIF2_SHOWN_ON_RADAR)) && !(wip->wi_flags2 & WIF2_DONT_SHOW_ON_RADAR)) {
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
			if (wp->homing_object == &obj_used_list)
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
			wp->lssm_warp_idx=fireball_create(&warpout, FIREBALL_WARP, FIREBALL_WARP_EFFECT, -1,obj->radius*1.5f,1,&vmd_zero_vector,wp->lssm_warp_time,0,&obj->orient);
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
			wp->lssm_warp_idx=fireball_create(&warpin, FIREBALL_WARP, FIREBALL_WARP_EFFECT, -1,obj->radius*1.5f,0,&vmd_zero_vector,wp->lssm_warp_time,0,&orient);

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

	if (parent_objnum >= 0) {
		parent_objp = &Objects[parent_objnum];
		Assert(parent_objp->type == OBJ_SHIP);
	} else {
		parent_objp = NULL;
	}

	if ( parent_objp == NULL || Ships[parent_objp->instance].ai_index >= 0 ) {
		int target_team = -1;
		if ( target_objnum >= 0 ) {
			int obj_type = Objects[target_objnum].type;
			if ( (obj_type == OBJ_SHIP) || (obj_type == OBJ_WEAPON) ) {
				target_team = obj_team(&Objects[target_objnum]);
			}
		}
	
		// determining if we're targeting the same team
		if (parent_objp != NULL && Ships[parent_objp->instance].team == target_team){
			targeting_same = 1;
		} else {
			targeting_same = 0;
		}

		if ((target_objnum != -1) && (!targeting_same || (MULTI_DOGFIGHT && (target_team == Iff_traitor))) ) {
			wp->target_num = target_objnum;
			wp->target_sig = Objects[target_objnum].signature;
			wp->nearest_dist = 99999.0f;
			if ( (wip->wi_flags & WIF_HOMING_ASPECT) && target_is_locked) {
				wp->homing_object = &Objects[target_objnum];
				wp->homing_subsys = target_subsys;
				weapon_maybe_play_warning(wp);
			} else if ( (wip->wi_flags & WIF_HOMING_JAVELIN) && target_is_locked) {
				if (Objects[target_objnum].type == OBJ_SHIP &&
					(wp->homing_subsys == NULL ||
					wp->homing_subsys->system_info->type != SUBSYSTEM_ENGINE)) {
						ship *target_ship = &Ships[Objects[target_objnum].instance];
						wp->homing_subsys = ship_get_closest_subsys_in_sight(target_ship, SUBSYSTEM_ENGINE, &Objects[weapon_objnum].pos);
						if (wp->homing_subsys == NULL) {
							wp->homing_object = &obj_used_list;
						} else {
							wp->homing_object = &Objects[target_objnum];
							weapon_maybe_play_warning(wp);
						}
				} else {
					wp->homing_object = &Objects[target_objnum];
					wp->homing_subsys = target_subsys;
					weapon_maybe_play_warning(wp);
				}
			} else if ( wip->wi_flags & WIF_HOMING_HEAT ) {
				//	Make a heat seeking missile try to home.  If the target is outside the view cone, it will
				//	immediately drop it and try to find one in its view cone.
				if ((target_objnum != -1) && !(wip->wi_flags2 & WIF2_UNTARGETED_HEAT_SEEKER)) {
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
		if (target_is_locked && (wp->target_num != -1) &&
			(wip->wi_flags & WIF_LOCKED_HOMING) ) {
			wp->lifeleft *= 1.2f;
			if (MULTIPLAYER_MASTER) {
				wp->weapon_flags |= WF_HOMING_UPDATE_NEEDED;
			}
		}

		ai_update_danger_weapon(target_objnum, weapon_objnum);		
	}
}

inline size_t* get_pointer_to_weapon_fire_pattern_index(int weapon_type, ship* shipp) {
	Assert( shipp != NULL );
	ship_weapon* ship_weapon_p = &(shipp->weapons);
	Assert( ship_weapon_p != NULL );

	// search for the corresponding bank pattern index for the weapon_type that is being fired.
	// Note: Because a weapon_type may not be unique to a weapon bank per ship this search may attribute
	// the weapon to the wrong bank.  Hopefully this isn't a problem.
	for ( int pi = 0; pi < MAX_SHIP_PRIMARY_BANKS; pi++ ) {
		if ( ship_weapon_p->primary_bank_weapons[pi] == weapon_type ) {
			return &(ship_weapon_p->primary_bank_pattern_index[pi]);
		}
	}
	for ( int si = 0; si < MAX_SHIP_SECONDARY_BANKS; si++ ) {
		if ( ship_weapon_p->secondary_bank_weapons[si] == weapon_type ) {
			return &(ship_weapon_p->secondary_bank_pattern_index[si]);
		}
	}
	return NULL;
}

// weapon_create() will create a weapon object
//
// Returns:  index of weapon in the Objects[] array, -1 if the weapon object was not created
int Weapons_created = 0;
int weapon_create( vec3d * pos, matrix * porient, int weapon_type, int parent_objnum, int group_id, int is_locked, int is_spawned)
{
	int			n, objnum;
	int num_deleted;
	object		*objp, *parent_objp=NULL;
	weapon		*wp;
	weapon_info	*wip;

	Assert(weapon_type >= 0 && weapon_type < Num_weapon_types);

	wip = &Weapon_info[weapon_type];

	// beam weapons should never come through here!
	if(wip->wi_flags & WIF_BEAM)
	{
		Warning(LOCATION, "An attempt to fire a beam ('%s') through weapon_create() was made.\n", wip->name);
		return -1;
	}

	parent_objp = NULL;
	if(parent_objnum >= 0){
		parent_objp = &Objects[parent_objnum];
	}

	if ( (wip->weapon_substitution_pattern.size() > 0) && (parent_objp != NULL)) {
		// using substitution

		// get to the instance of the gun
		Assertion( parent_objp->type == OBJ_SHIP, "Expected type OBJ_SHIP, got %d", parent_objp->type );
		Assertion( (parent_objp->instance < MAX_SHIPS) && (parent_objp->instance >= 0),
			"Ship index is %d, which is out of range [%d,%d)", parent_objp->instance, 0, MAX_SHIPS);
		ship* parent_shipp = &(Ships[parent_objp->instance]);
		Assert( parent_shipp != NULL );

		size_t *position = get_pointer_to_weapon_fire_pattern_index(weapon_type, parent_shipp);
		Assertion( position != NULL, "'%s' is trying to fire a weapon that is not selected", Ships[parent_objp->instance].ship_name );

		*position = ++(*position) % wip->weapon_substitution_pattern.size();

		if ( wip->weapon_substitution_pattern[*position] == -1 ) {
			// weapon doesn't want any sub
			return -1;
		} else if ( wip->weapon_substitution_pattern[*position] != weapon_type ) {
			// weapon wants to sub with weapon other than me
			return weapon_create(pos, porient, wip->weapon_substitution_pattern[*position], parent_objnum, group_id, is_locked, is_spawned);
		}
	}

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

	// make sure we are loaded and useable
	if ( (wip->render_type == WRT_POF) && (wip->model_num < 0) ) {
		wip->model_num = model_load(wip->pofbitmap_name, 0, NULL);

		if (wip->model_num < 0) {
			Int3();
			return -1;
		}
	}

	// make sure that our textures are loaded as well
	if ( !used_weapons[weapon_type] )
		weapon_load_bitmaps(weapon_type);

	//I am hopeing that this way does not alter the input orient matrix
	//Feild of Fire code -Bobboau
	matrix morient;
	matrix *orient;

	if(porient != NULL) {
		morient = *porient;
	} else {
		morient = vmd_identity_matrix;
	}

	orient = &morient;
	if(wip->field_of_fire){
		vec3d f;
		vm_vec_random_cone(&f, &orient->vec.fvec, wip->field_of_fire);
		vm_vec_normalize(&f);
		vm_vector_2_matrix( orient, &f, NULL, NULL);
	}

	Weapons_created++;
	objnum = obj_create( OBJ_WEAPON, parent_objnum, n, orient, pos, 2.0f, OF_RENDERS | OF_COLLIDES | OF_PHYSICS );
	Assert(objnum >= 0);
	objp = &Objects[objnum];

	// Create laser n!
	wp = &Weapons[n];

	// check if laser or dumbfire missile
	// set physics flag to allow optimization
	if ((wip->subtype == WP_LASER) || ((wip->subtype == WP_MISSILE) && !(wip->wi_flags & WIF_HOMING))) {
		// set physics flag
		objp->phys_info.flags |= PF_CONST_VEL;
	}

	wp->start_pos = *pos;
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
		// ugh - we need to prevent bad array accesses
		wp->team = Iff_traitor;
		wp->species = 0;
	}
	wp->turret_subsys = NULL;
	vm_vec_zero(&wp->homing_pos);
	wp->weapon_flags = 0;
	wp->target_sig = -1;
	wp->cmeasure_ignore_objnum = -1;
	wp->cmeasure_chase_objnum = -1;
	wp->det_range = wip->det_range;

	// Init the thruster info
	wp->thruster_bitmap = -1;
	wp->thruster_frame = 0.0f;
	wp->thruster_glow_bitmap = -1;
	wp->thruster_glow_noise = 1.0f;
	wp->thruster_glow_frame = 0.0f;

	// init the laser info
	wp->laser_bitmap_frame = 0.0f;
	wp->laser_glow_bitmap_frame = 0.0f;

	if ( wip->wi_flags & WIF_SWARM ) {
		wp->swarm_index = (short)swarm_create();
	} else {
		wp->swarm_index = -1;
	}		

	// if this is a particle spewing weapon, setup some stuff
	if (wip->wi_flags & WIF_PARTICLE_SPEW) {
		for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {		// allow for multiple time values
			if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE) {
				wp->particle_spew_time[s] = -1;
				wp->particle_spew_rand = frand_range(0, PI2);	// per weapon randomness
			}
		}
	}

	// assign the network signature.  The starting sig is sent to all clients, so this call should
	// result in the same net signature numbers getting assigned to every player in the game
	if ( Game_mode & GM_MULTIPLAYER ) {
		if(wip->subtype == WP_MISSILE){
			Objects[objnum].net_signature = multi_assign_network_signature( MULTI_SIG_NON_PERMANENT );

			// for weapons that respawn, add the number of respawnable weapons to the net signature pool
			// to reserve N signatures for the spawned weapons
			if ( wip->wi_flags & WIF_SPAWN ){
                multi_set_network_signature( (ushort)(Objects[objnum].net_signature + wip->total_children_spawned), MULTI_SIG_NON_PERMANENT );
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

	//Check if we want to gen a random number
	//This is used for lifetime min/max
	float rand_val;
	if ( Game_mode & GM_NORMAL ){
		rand_val = frand();
	} else {
		rand_val = static_randf(Objects[objnum].net_signature);
	}

	wp->weapon_info_index = weapon_type;
	if(wip->life_min < 0.0f && wip->life_max < 0.0f) {
		wp->lifeleft = wip->lifetime;
	} else {
		wp->lifeleft = (rand_val) * (wip->life_max - wip->life_min) / wip->life_min;
		if((wip->wi_flags & WIF_CMEASURE) && (parent_objp->flags & OF_PLAYER_SHIP)) {
			wp->lifeleft *= The_mission.ai_profile->cmeasure_life_scale[Game_skill_level];
		}
		wp->lifeleft = wip->life_min + wp->lifeleft * (wip->life_max - wip->life_min);
	}

	if(wip->wi_flags & WIF_CMEASURE) {
		//2-frame homing check, to fend off sync errors
		Cmeasures_homing_check = 2;
	}

	//	Make remote detonate missiles look like they're getting detonated by firer simply by giving them variable lifetimes.
	if (parent_objp != NULL && !(parent_objp->flags & OF_PLAYER_SHIP) && (wip->wi_flags & WIF_REMOTE)) {
		wp->lifeleft = wp->lifeleft/2.0f + rand_val * wp->lifeleft/2.0f;
	}

	objp->phys_info.mass = wip->mass;
	objp->phys_info.side_slip_time_const = 0.0f;
	objp->phys_info.rotdamp = 0.0f;
	vm_vec_zero(&objp->phys_info.max_vel);
	objp->phys_info.max_vel.xyz.z = wip->max_speed;
	vm_vec_zero(&objp->phys_info.max_rotvel);
	objp->shield_quadrant[0] = wip->damage;
	if (wip->weapon_hitpoints > 0){
		objp->hull_strength = (float) wip->weapon_hitpoints;
	} else {
		objp->hull_strength = 0.0f;
	}

	if ( wip->render_type == WRT_POF ) {
		objp->radius = model_get_radius(wip->model_num);
	} else if ( wip->render_type == WRT_LASER ) {
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
            if (!is_locked)
            {
			    vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z/4 );
            }
            else
            {
                vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );
            }
		}
		objp->phys_info.vel = objp->phys_info.desired_vel;
		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.vel);
	}

	wp->weapon_max_vel = objp->phys_info.max_vel.xyz.z;

	// Turey - maybe make the initial speed of the weapon take into account the velocity of the parent.
	// Improves aiming during gliding.
	if ((parent_objp != NULL) && (The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY)) {
		vm_vec_add2( &objp->phys_info.vel, &parent_objp->phys_info.vel );
		wp->weapon_max_vel += vm_vec_mag( &parent_objp->phys_info.vel );
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
		obj_set_flags(&Objects[wp->objnum], Objects[wp->objnum].flags & ~(OF_RENDERS));
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
			if (wip->detail_distance[i] >= 0)
				pm->detail_depth[i] = i2fl(wip->detail_distance[i]);
			else
				pm->detail_depth[i] = (objp->radius*20.0f + 20.0f) * i;
		}

#ifndef NDEBUG
		// since debug builds always have cheats enabled, we don't necessarily get the chance
		// to enable thrusters for previously non-loaded weapons (ie, weapons_page_in_cheats())
		// when using cheat-keys, so we need to make sure and enable thrusters here if needed
		if (pm->n_thrusters > 0) {
			wip->wi_flags |= WIF_THRUSTER;
		}
#endif
	}

		// if the weapon was fired locked
	if(is_locked){
		wp->weapon_flags |= WF_LOCKED_WHEN_FIRED;
	}

	//if the weapon was spawned from a spawning type weapon
	if(is_spawned){
		wp->weapon_flags |= WF_SPAWNED;
	}

	wp->alpha_current = -1.0f;
	wp->alpha_backward = 0;

	Num_weapons++;
	return objnum;
}
//	Spawn child weapons from object *objp.
void spawn_child_weapons(object *objp)
{
	int	i, j;
	int	child_id;
	int	parent_num;
	ushort starting_sig;
	weapon	*wp;
	weapon_info	*wip, *child_wip;

	Assert(objp->type == OBJ_WEAPON);
	Assert((objp->instance >= 0) && (objp->instance < MAX_WEAPONS));

	wp = &Weapons[objp->instance];
	Assert((wp->weapon_info_index >= 0) && (wp->weapon_info_index < MAX_WEAPON_TYPES));
	wip = &Weapon_info[wp->weapon_info_index];

	parent_num = objp->parent;

	if (parent_num >= 0) {
		if ((Objects[parent_num].type != objp->parent_type) || (Objects[parent_num].signature != objp->parent_sig)) {
			mprintf(("Warning: Parent of spawn weapon does not exist.  Not spawning.\n"));
			return;
		}
	}

	starting_sig = 0;

	if ( Game_mode & GM_MULTIPLAYER ) {		
		// get the next network signature and save it.  Set the next usable network signature to be
		// the passed in objects signature + 1.  We "reserved" N of these slots when we created objp
		// for it's spawned children.
		starting_sig = multi_get_next_network_signature( MULTI_SIG_NON_PERMANENT );
		multi_set_network_signature( objp->net_signature, MULTI_SIG_NON_PERMANENT );
	}

    for (i = 0; i < wip->num_spawn_weapons_defined; i++)
    {
        for (j = 0; j < wip->spawn_info[i].spawn_count; j++) 
        {
    		int		weapon_objnum;
	    	vec3d	tvec, pos;
		    matrix	orient;

            child_id = wip->spawn_info[i].spawn_type;
			child_wip = &Weapon_info[child_id];

		    // for multiplayer, use the static randvec functions based on the network signatures to provide
		    // the randomness so that it is the same on all machines.
		    if ( Game_mode & GM_MULTIPLAYER ) {
    			static_rand_cone(objp->net_signature + j, &tvec, &objp->orient.vec.fvec, wip->spawn_info[i].spawn_angle);
	    	} else {
		    	vm_vec_random_cone(&tvec, &objp->orient.vec.fvec, wip->spawn_info[i].spawn_angle);
		    }
		    vm_vec_scale_add(&pos, &objp->pos, &tvec, objp->radius);

		    vm_vector_2_matrix(&orient, &tvec, NULL, NULL);
		    weapon_objnum = weapon_create(&pos, &orient, child_id, parent_num, -1, wp->weapon_flags & WF_LOCKED_WHEN_FIRED, 1);

			//if the child inherits parent target, do it only if the parent weapon was locked to begin with
			if ((child_wip->wi_flags2 & WIF2_INHERIT_PARENT_TARGET) && (wp->homing_object != &obj_used_list))
			{
				//Deal with swarm weapons
				if (wp->swarm_index >= 0) {
					swarm_info	*swarmp;
					swarmp = &Swarm_missiles[wp->swarm_index];

					weapon_set_tracking_info(weapon_objnum, parent_num, swarmp->homing_objnum, 1, wp->homing_subsys);
				} else {
					weapon_set_tracking_info(weapon_objnum, parent_num, wp->target_num, 1, wp->homing_subsys);
				}
			}

    		//	Assign a little randomness to lifeleft so they don't all disappear at the same time.
		    if (weapon_objnum != -1) {
			    float rand_val;

			    if ( Game_mode & GM_NORMAL ){
				    rand_val = frand();
			    } else {
    				rand_val = static_randf(objp->net_signature + j);
			    }

			    Weapons[Objects[weapon_objnum].instance].lifeleft *= rand_val*0.4f + 0.8f;
		    }
        }

	}

	// in multiplayer, reset the next network signature to the one that was saved.
	if ( Game_mode & GM_MULTIPLAYER ){
		multi_set_network_signature( starting_sig, MULTI_SIG_NON_PERMANENT );
	}
}

//Figures out whether to play disarmed or armed hit sound, checks that the
//chosen one exists, and plays it
void weapon_play_impact_sound(weapon_info *wip, vec3d *hitpos, bool is_armed)
{
	if(is_armed)
	{
		if(wip->impact_snd != -1) {
			snd_play_3d( &Snds[wip->impact_snd], hitpos, &Eye_position );
		}
	}
	else
	{
		if(wip->disarmed_impact_snd != -1) {
			snd_play_3d(&Snds[wip->disarmed_impact_snd], hitpos, &Eye_position);
		}
	}
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
void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vec3d *hitpos, bool is_armed)
{
	int	is_hull_hit;
	float shield_str;

	// If non-missiles (namely lasers) expire without hitting a ship, don't play impact sound
	if	( wip->subtype != WP_MISSILE ) {		
		if ( !hit_obj ) {
			// flak weapons make sounds		
			if(wip->wi_flags & WIF_FLAK)
			{
				weapon_play_impact_sound(wip, hitpos, is_armed);				
			}
			return;
		}

		switch(hit_obj->type) {
		case OBJ_SHIP:
			// do nothing
			break;

		case OBJ_ASTEROID:
			if ( timestamp_elapsed(Weapon_impact_timer) ) {
				weapon_play_impact_sound(wip, hitpos, is_armed);	
				Weapon_impact_timer = timestamp(IMPACT_SOUND_DELTA);
			}
			return;
			break;

		default:
			return;
		}
	}

	if ( hit_obj == NULL ) {
		weapon_play_impact_sound(wip, hitpos, is_armed);
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
						weapon_play_impact_sound(wip, hitpos, is_armed);
					}
					break;
				case WP_MISSILE:
					if ( hit_obj == Player_obj ) 
						snd_play_3d( &Snds[SND_PLAYER_HIT_MISSILE], hitpos, &Eye_position);
					else {
						weapon_play_impact_sound(wip, hitpos, is_armed);
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
/*
const float weapon_electronics_scale[MAX_SHIP_TYPE_COUNTS]=
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
};*/

extern bool turret_weapon_has_flags(ship_weapon *swp, int flags);

// distrupt any subsystems that fall into damage sphere of this Electronics missile
//
// input:	ship_obj		=>		pointer to ship that holds subsystem
//				blast_pos	=>		world pos of weapon blast
//				wi_index		=>		weapon info index of weapon causing blast
void weapon_do_electronics_effect(object *ship_objp, vec3d *blast_pos, int wi_index)
{
	weapon_info			*wip;
	ship				*shipp;
	ship_subsys			*ss;
	model_subsystem		*psub;
	vec3d				subsys_world_pos;
	float				dist;

	shipp = &Ships[ship_objp->instance];
	wip = &Weapon_info[wi_index];

	for ( ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss) )
	{
		psub = ss->system_info;

		// convert subsys point to world coords
		vm_vec_unrotate(&subsys_world_pos, &psub->pnt, &ship_objp->orient);
		vm_vec_add2(&subsys_world_pos, &ship_objp->pos);

		// see if subsys point is within damage sphere
		dist = vm_vec_dist_quick(blast_pos, &subsys_world_pos);
		if ( dist < wip->shockwave.outer_rad )
		{
			float disrupt_time = (float)wip->elec_time;

			//use new style electronics disruption
			if (wip->elec_use_new_style)
			{
				//if its an engine subsytem, take the multiplier into account
				if (psub->type==SUBSYSTEM_ENGINE)
				{
					disrupt_time*=wip->elec_eng_mult;
				}
	
				//if its a turret or weapon subsytem, take the multiplier into account
				if ((psub->type==SUBSYSTEM_TURRET) || (psub->type==SUBSYSTEM_WEAPONS))
				{
					//disrupt beams
					//WMC - do this even if there are other types of weapons on the turret.
					//I figure, the big fancy electronics on beams will be used for the other
					//weapons as well. No reason having two targeting computers on a turret.
					//Plus, it's easy and fast to code. :)
					if ((psub->type==SUBSYSTEM_TURRET) && turret_weapon_has_flags(&ss->weapons, WIF_BEAM))
					{
						disrupt_time*=wip->elec_beam_mult;
					}
					//disrupt other weapons
					else
					{
						disrupt_time*=wip->elec_weap_mult;
					}
				}
				
				//disrupt sensor and awacs systems.
				if ((psub->type==SUBSYSTEM_SENSORS) || (psub->flags & MSS_FLAG_AWACS))
				{
					disrupt_time*=wip->elec_sensors_mult;
				}
			}
	
			//add a little randomness to the disruption time
			disrupt_time += frand_range(-1.0f, 1.0f) * wip->elec_randomness;
		
			//disrupt this subsystem for the calculated time
			//if it turns out to be less than 0 seconds, don't bother
			if (disrupt_time > 0)
			{
				ship_subsys_set_disrupted(ss, fl2i(disrupt_time));
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

	// don't waste time here if there is no blast force
	if ( blast == 0.0f )
		return;

	// apply blast force based on distance from center of explosion
	vm_vec_sub(&vec_blast_to_ship, &ship_obj->pos, blast_pos);
	vm_vec_normalize_safe(&vec_blast_to_ship);
	vm_vec_copy_scale(&force, &vec_blast_to_ship, blast );


	vm_vec_sub(&vec_ship_to_impact, blast_pos, &ship_obj->pos);

	pm = model_get(Ship_info[Ships[ship_obj->instance].ship_info_index].model_num);
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
void weapon_do_area_effect(object *wobjp, shockwave_create_info *sci, vec3d *pos, object *other_obj)
{
	weapon_info	*wip;
	weapon *wp;
	object		*objp;
	float			damage, blast;

	wip = &Weapon_info[Weapons[wobjp->instance].weapon_info_index];	
	wp = &Weapons[wobjp->instance];
	Assert(sci->inner_rad != 0);	

	// only blast ships and asteroids
	// And (some) weapons
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) && (objp->type != OBJ_WEAPON) ) {
			continue;
		}
	
		if ( objp->type == OBJ_WEAPON ) {
			// only apply to missiles with hitpoints
			weapon_info* wip2 = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (wip2->weapon_hitpoints <= 0 || !(wip2->wi_flags2 & WIF2_TAKES_BLAST_DAMAGE))
				continue;
		}

		if ( objp->type == OBJ_SHIP ) {
			// don't blast navbuoys
			if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
				continue;
			}
		}

		if ( weapon_area_calc_damage(objp, pos, sci->inner_rad, sci->outer_rad, sci->blast, sci->damage, &blast, &damage, sci->outer_rad) == -1 ){
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
		case OBJ_WEAPON:
			objp->hull_strength -= damage;
			if (objp->hull_strength < 0.0f) {
				Weapons[objp->instance].lifeleft = 0.01f;
				Weapons[objp->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
			}
			break;
		default:
			Int3();
			break;
		} 	

	}	// end for

	// if this weapon has the "Electronics" flag set, then disrupt subsystems in sphere
	if ( (other_obj != NULL) && (wip->wi_flags & WIF_ELECTRONICS) ) {
		if ( other_obj->type == OBJ_SHIP ) {
			weapon_do_electronics_effect(other_obj, pos, Weapons[wobjp->instance].weapon_info_index);
		}
	}
}

//	----------------------------------------------------------------------
//	weapon_armed(weapon)
//
//	Call to figure out if a weapon is armed or not
//
//Weapon is armed when...
//1: Weapon is shot down by weapon
//OR
//1: weapon is destroyed before arm time
//2: weapon is destroyed before arm distance from ship
//3: weapon is outside arm radius from target ship
bool weapon_armed(weapon *wp, bool hit_target)
{
	Assert(wp != NULL);

	weapon_info *wip = &Weapon_info[wp->weapon_info_index];

	if((wp->weapon_flags & WF_DESTROYED_BY_WEAPON)
		&& !wip->arm_time
		&& wip->arm_dist == 0.0f
		&& wip->arm_radius == 0.0f)
	{
		return false;
	}
	else
	{
		object *wobj = &Objects[wp->objnum];
		object *pobj;

		if(wobj->parent > -1) {
			pobj = &Objects[wobj->parent];
		} else {
			pobj = NULL;
		}

		if(		((wip->arm_time) && ((Missiontime - wp->creation_time) < wip->arm_time))
			|| ((wip->arm_dist) && (pobj != NULL && pobj->type != OBJ_NONE && (vm_vec_dist(&wobj->pos, &pobj->pos) < wip->arm_dist))))
		{
			return false;
		}
		if(wip->arm_radius && (!hit_target)) {
			if(wp->homing_object == NULL)
				return false;
			if(vm_vec_dist(&wobj->pos, &wp->homing_pos) > wip->arm_radius)
				return false;
		}
	}

	return true;
}


//	----------------------------------------------------------------------
//	weapon_hit()
//
// This function is called when a weapon hits something (or, in the case of
// missiles explodes for any particular reason)
//
void weapon_hit( object * weapon_obj, object * other_obj, vec3d * hitpos, int quadrant )
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
	int			expl_ani_handle;
	object		*weapon_parent_objp;
	weapon_info	*wip;
	weapon *wp;
	bool		hit_target = false;

	Assert((weapon_type >= 0) && (weapon_type < MAX_WEAPONS));
	if((weapon_type < 0) || (weapon_type >= MAX_WEAPONS)){
		return;
	}
	wp = &Weapons[weapon_obj->instance];
	wip = &Weapon_info[weapon_type];
	if(weapon_obj->parent > -1) {
		weapon_parent_objp = &Objects[weapon_obj->parent];
	} else {
		weapon_parent_objp = NULL;
	}

	// check if the weapon actually hit the intended target
	if (wp->homing_object != NULL)
		if (wp->homing_object == other_obj)
			hit_target = true;

	//This is an expensive check
	bool armed_weapon = weapon_armed(&Weapons[num], hit_target);
	// int np_index;

	// if this is the player ship, and is a laser hit, skip it. wait for player "pain" to take care of it
	// if( ((wip->subtype != WP_LASER) || !MULTIPLAYER_CLIENT) && (Player_obj != NULL) && (other_obj == Player_obj) ){
	if ((other_obj != Player_obj) || (wip->subtype != WP_LASER) || !MULTIPLAYER_CLIENT) {
		weapon_hit_do_sound(other_obj, wip, hitpos, armed_weapon);
	}

	if ( wip->impact_weapon_expl_index > -1 && armed_weapon)
	{
		expl_ani_handle = Weapon_explosions.GetAnim(wip->impact_weapon_expl_index, hitpos, wip->impact_explosion_radius);
		particle_create( hitpos, &vmd_zero_vector, 0.0f, wip->impact_explosion_radius, PARTICLE_BITMAP_PERSISTENT, expl_ani_handle );
	}
	else if(wip->dinky_impact_weapon_expl_index > -1 && !armed_weapon)
	{
		expl_ani_handle = Weapon_explosions.GetAnim(wip->dinky_impact_weapon_expl_index, hitpos, wip->dinky_impact_explosion_radius);
		particle_create( hitpos, &vmd_zero_vector, 0.0f, wip->dinky_impact_explosion_radius, PARTICLE_BITMAP_PERSISTENT, expl_ani_handle );
	}

	if((other_obj != NULL) && (quadrant == -1) && (wip->piercing_impact_weapon_expl_index > -1 && armed_weapon)) {
		if ((other_obj->type == OBJ_SHIP) || (other_obj->type == OBJ_DEBRIS)) {

			int ok_to_draw = 1;

			if (other_obj->type == OBJ_SHIP) {
				float draw_limit, hull_pct;
				int dmg_type_idx, piercing_type;

				ship *shipp = &Ships[other_obj->instance];

				hull_pct = other_obj->hull_strength / shipp->ship_max_hull_strength;
				dmg_type_idx = wip->damage_type_idx;
				draw_limit = Ship_info[shipp->ship_info_index].piercing_damage_draw_limit;
				
				if (shipp->armor_type_idx != -1) {
					piercing_type = Armor_types[shipp->armor_type_idx].GetPiercingType(dmg_type_idx);
					if (piercing_type == SADTF_PIERCING_DEFAULT) {
						draw_limit = Armor_types[shipp->armor_type_idx].GetPiercingLimit(dmg_type_idx);
					} else if ((piercing_type == SADTF_PIERCING_NONE) || (piercing_type == SADTF_PIERCING_RETAIL)) {
						ok_to_draw = 0;
					}
				}

				if (hull_pct > draw_limit)
					ok_to_draw = 0;
			}

			if (ok_to_draw) {
				particle_emitter pe;
				vec3d null_v;

				vm_vec_zero(&null_v);
				expl_ani_handle = Weapon_explosions.GetAnim(wip->piercing_impact_weapon_expl_index, hitpos, wip->piercing_impact_explosion_radius);

				pe.max_vel = 2.0f * wip->piercing_impact_particle_velocity;
				pe.min_vel = 0.5f * wip->piercing_impact_particle_velocity;
				pe.max_life = 2.0f * wip->piercing_impact_particle_life;
				pe.min_life = 0.25f * wip->piercing_impact_particle_life;
				pe.num_high = 2 * wip->piercing_impact_particle_count;
				pe.num_low =  wip->piercing_impact_particle_count / 2;
				pe.pos = weapon_obj->pos;
				pe.normal = weapon_obj->last_orient.vec.fvec;
				pe.normal_variance = wip->piercing_impact_particle_variance;
				pe.max_rad = 2.0f * wip->piercing_impact_explosion_radius;
				pe.min_rad = 0.5f * wip->piercing_impact_explosion_radius;
				pe.vel = null_v;

				particle_emit(&pe, PARTICLE_BITMAP, expl_ani_handle, 10.0f);

				if (wip->piercing_impact_particle_back_velocity != 0.0f) {

					pe.max_vel = 2.0f * wip->piercing_impact_particle_back_velocity;
					pe.min_vel = 0.5f * wip->piercing_impact_particle_back_velocity;
					pe.num_high /= 2;
					pe.num_low /= 2;

					particle_emit(&pe, PARTICLE_BITMAP, expl_ani_handle, 10.0f);
				}
			}
		}
	}

	weapon_obj->flags |= OF_SHOULD_BE_DEAD;

	//Set shockwaves flag
	int sw_flag = SW_WEAPON;

	if ( ((other_obj) && (other_obj->type == OBJ_WEAPON)) || (Weapons[num].weapon_flags & WF_DESTROYED_BY_WEAPON)) {
		sw_flag |= SW_WEAPON_KILL;
	}

	//Which shockwave?
	shockwave_create_info *sci = &wip->shockwave;
	if(!armed_weapon) {
		sci = &wip->dinky_shockwave;
	}

	// check if this is an area effect weapon (i.e. has a blast radius)
	if (sci->inner_rad != 0.0f || sci->outer_rad != 0.0f)
	{
		if(sci->speed > 0.0f) {
			shockwave_create(OBJ_INDEX(weapon_obj), hitpos, sci, sw_flag, -1);
		}
		else {
			weapon_do_area_effect(weapon_obj, sci, hitpos, other_obj);
		}
	}

	// check if this is an EMP weapon
	if(wip->wi_flags & WIF_EMP){
		emp_apply(&weapon_obj->pos, wip->shockwave.inner_rad, wip->shockwave.outer_rad, wip->emp_intensity, wip->emp_time);
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

	// send a detonate packet in multiplayer
	if(MULTIPLAYER_MASTER){
		send_weapon_detonate_packet(objp);
	}

	// call weapon hit
	// Wanderer - use last frame pos for the corkscrew missiles
	if ( (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CORKSCREW) ) {
		weapon_hit(objp, NULL, &objp->last_pos);
	} else {
		weapon_hit(objp, NULL, &objp->pos);
	}
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
void weapon_mark_as_used(int weapon_type)
{
	if (weapon_type < 0)
		return;

	if ( used_weapons == NULL )
		return;

	Assert( weapon_type < MAX_WEAPON_TYPES );

	if (weapon_type < Num_weapon_types) {
		used_weapons[weapon_type]++;
	}
}

void weapons_page_in()
{
	int i, j, idx;

	Assert( used_weapons != NULL );

	// for weapons in weaponry pool
	for (i = 0; i < Num_teams; i++) {
		for (j = 0; j < Team_data[i].num_weapon_choices; j++) {
			used_weapons[Team_data[i].weaponry_pool[j]] += Team_data[i].weaponry_count[j];
		}
	}

	// this grabs all spawn weapon types (Cluster Baby, etc.) which can't be
	// assigned directly to a ship
	for (i = 0; i < Num_weapon_types; i++) {
		// we only want entries that already exist
		if ( !used_weapons[i] )
			continue;

		// if it's got a spawn type then grab it
        for (j = 0; j < Weapon_info[i].num_spawn_weapons_defined; j++)
        {
            used_weapons[(int)Weapon_info[i].spawn_info[j].spawn_type]++;
        }
	}

	// release anything loaded that we don't have marked as used for this mission
	if ( !Cmdline_load_all_weapons )
		weapon_release_bitmaps();

	// Page in bitmaps for all used weapons
	for (i = 0; i < Num_weapon_types; i++) {
		if ( !Cmdline_load_all_weapons ) {
			if ( !used_weapons[i] ) {
				nprintf(("Weapons", "Not loading weapon id %d (%s)\n", i, Weapon_info[i].name));
				continue;
			}
		}

		weapon_load_bitmaps(i);

		weapon_info *wip = &Weapon_info[i];

		wip->wi_flags &= (~WIF_THRUSTER);		// Assume no thrusters
		
		switch (wip->render_type)
		{
			case WRT_POF:
			{
				wip->model_num = model_load( wip->pofbitmap_name, 0, NULL );

				polymodel *pm = model_get( wip->model_num );

				// If it has a model, and the model pof has thrusters, then set
				// the flags
				if (pm->n_thrusters > 0) {
					//mprintf(( "Weapon %s has thrusters!\n", wip->name ));
					wip->wi_flags |= WIF_THRUSTER;
				}
		
				for (j = 0; j < pm->n_textures; j++)
					pm->maps[j].PageIn();

				break;
			}

			case WRT_LASER:
			{
				bm_page_in_texture( wip->laser_bitmap.first_frame );
				bm_page_in_texture( wip->laser_glow_bitmap.first_frame );

				break;
			}

			default:
				Assertion(wip->render_type != WRT_POF && wip->render_type != WRT_LASER, "Weapon %s does not have a valid rendering type. Type passed: %d\n", wip->name, wip->render_type);	// Invalid weapon rendering type.
		}

		wip->external_model_num = -1;

		if ( strlen(wip->external_model_name) )
			wip->external_model_num = model_load( wip->external_model_name, 0, NULL );

		if (wip->external_model_num == -1)
			wip->external_model_num = wip->model_num;


		//Load shockwaves
		wip->shockwave.load();
		wip->dinky_shockwave.load();

		//Explosions
		Weapon_explosions.PageIn(wip->impact_weapon_expl_index);
		Weapon_explosions.PageIn(wip->dinky_impact_weapon_expl_index);
		Weapon_explosions.PageIn(wip->flash_impact_weapon_expl_index);
		Weapon_explosions.PageIn(wip->piercing_impact_weapon_expl_index);

		// trail bitmaps
		if ( (wip->wi_flags & WIF_TRAIL) && (wip->tr_info.texture.bitmap_id > -1) )
			bm_page_in_texture( wip->tr_info.texture.bitmap_id );

		// if this is a beam weapon, page in its stuff
		if (wip->wi_flags & WIF_BEAM) {
			// all beam sections
			for (idx = 0; idx < wip->b_info.beam_num_sections; idx++)
				bm_page_in_texture(wip->b_info.sections[idx].texture.first_frame);

			// muzzle glow
			bm_page_in_texture(wip->b_info.beam_glow.first_frame);

			// particle ani
			bm_page_in_texture(wip->b_info.beam_particle_ani.first_frame);
		}

		if (wip->wi_flags & WIF_PARTICLE_SPEW) {
			for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {	// looped, multi particle spew -nuke
				if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE) {
					bm_page_in_texture(wip->particle_spewers[s].particle_spew_anim.first_frame);
				}
			}
		}

		// muzzle flashes
		if (wip->muzzle_flash >= 0)
			mflash_mark_as_used(wip->muzzle_flash);

		bm_page_in_texture(wip->thruster_flame.first_frame);
		bm_page_in_texture(wip->thruster_glow.first_frame);
	}

	// Counter measures
	/*
	for (i=0; i<Num_cmeasure_types; i++ )	{
		cmeasure_info *cmeasurep;

		cmeasurep = &Cmeasure_info[i];

		Assert( strlen(cmeasurep->pof_name) );

		cmeasurep->model_num = model_load( cmeasurep->pof_name, 0, NULL );

		if ( cmeasurep->model_num < 0 ) {
			Error( LOCATION, "Unable to load countermeasure POF for '%s' (%s)", cmeasurep->cmeasure_name, cmeasurep->pof_name);
		}

		polymodel *pm = model_get( cmeasurep->model_num );

		for (j=0; j<pm->n_textures; j++ )	{
			int bitmap_num = pm->original_textures[j];

			if ( bitmap_num > -1 )	{
				bm_page_in_texture( bitmap_num );
			}
		}
	}
*/
}

// page_in function for cheaters, grabs all weapons that weren't already in a mission
// and loads the models for them.  Non-model graphics elements will get loaded when
// they are rendered for the first time.  Maybe not the best way to do this but faster
// and a lot less error prone.
void weapons_page_in_cheats()
{
	int i;

	// don't bother if they are all loaded already
	if ( Cmdline_load_all_weapons )
		return;

	Assert( used_weapons != NULL );

	// force a page in of all muzzle flashes
	mflash_page_in(true);

	// page in models for all weapon types that aren't already loaded
	for (i = 0; i < Num_weapon_types; i++) {
		// skip over anything that's already loaded
		if (used_weapons[i])
			continue;

		weapon_load_bitmaps(i);

		weapon_info *wip = &Weapon_info[i];
		
		wip->wi_flags &= (~WIF_THRUSTER);		// Assume no thrusters

		if ( wip->render_type == WRT_POF ) {
			wip->model_num = model_load( wip->pofbitmap_name, 0, NULL );
				
			polymodel *pm = model_get( wip->model_num );
				
			// If it has a model, and the model pof has thrusters, then set
			// the flags
			if ( pm->n_thrusters > 0 )	{
				//mprintf(( "Weapon %s has thrusters!\n", wip->name ));
				wip->wi_flags |= WIF_THRUSTER;
			}
		}
		
		wip->external_model_num = -1;
		
		if ( strlen(wip->external_model_name) )
			wip->external_model_num = model_load( wip->external_model_name, 0, NULL );

		if (wip->external_model_num == -1)
			wip->external_model_num = wip->model_num;
		
		
		//Load shockwaves
		wip->shockwave.load();
		wip->dinky_shockwave.load();

		used_weapons[i]++;
	}
/*
	// Counter measures
	for (i=0; i<Num_cmeasure_types; i++ )	{
		cmeasure_info *cmeasurep;
		
		cmeasurep = &Cmeasure_info[i];
		
		Assert( strlen(cmeasurep->pof_name) );
		
		cmeasurep->model_num = model_load( cmeasurep->pof_name, 0, NULL );
		
		if ( cmeasurep->model_num < 0 ) {
			Error( LOCATION, "Unable to load countermeasure POF for '%s' (%s)", cmeasurep->cmeasure_name, cmeasurep->pof_name);
		}
	}
*/
}

// call to get the "color" of the laser at the given moment (since glowing lasers can cycle colors)
void weapon_get_laser_color(color *c, object *objp)
{
	weapon *wep;
	weapon_info *winfo;
	float pct;

	// sanity
	if (c == NULL)
		return;

	// sanity
	Assert(objp != NULL);
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);
	Assert(Weapons[objp->instance].weapon_info_index >= 0);

	if ( (objp == NULL) || (objp->type != OBJ_WEAPON) || (objp->instance < 0) || (Weapons[objp->instance].weapon_info_index < 0) )
		return;

	wep = &Weapons[objp->instance];
	winfo = &Weapon_info[wep->weapon_info_index];

	// if we're a one-color laser
	if ( (winfo->laser_color_2.red == 0) && (winfo->laser_color_2.green == 0) && (winfo->laser_color_2.blue == 0) ) {
		*c = winfo->laser_color_1;
		return;
	}

	int r = winfo->laser_color_1.red;
	int g = winfo->laser_color_1.green;
	int b = winfo->laser_color_1.blue;

	// lifetime pct
	pct = 1.0f - (wep->lifeleft / winfo->lifetime);
	CLAMP(pct, 0.0f, 0.5f);

	if (pct > 0.0f) {
		pct *= 2.0f;

		r += fl2i((winfo->laser_color_2.red - winfo->laser_color_1.red) * pct);
		g += fl2i((winfo->laser_color_2.green - winfo->laser_color_1.green) * pct);
		b += fl2i((winfo->laser_color_2.blue - winfo->laser_color_1.blue) * pct);
	}

	// otherwise interpolate between the colors
	gr_init_color( c, r, g, b );
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

	// check some stuff
	Assert(obj->type == OBJ_WEAPON);
	Assert(obj->instance >= 0);
	Assert(Weapons[obj->instance].weapon_info_index >= 0);
	Assert(Weapon_info[Weapons[obj->instance].weapon_info_index].wi_flags & WIF_PARTICLE_SPEW);
	
	wp = &Weapons[obj->instance];
	wip = &Weapon_info[wp->weapon_info_index];
	vec3d spawn_pos, spawn_vel, output_pos, output_vel, input_pos, input_vel;

	for (int psi = 0; psi < MAX_PARTICLE_SPEWERS; psi++) {	// iterate through spewers	-nuke
		if (wip->particle_spewers[psi].particle_spew_type != PSPEW_NONE) {
			// if the weapon's particle timestamp has elapsed
			if ((wp->particle_spew_time[psi] == -1) || timestamp_elapsed(wp->particle_spew_time[psi])) {
				// reset the timestamp
				wp->particle_spew_time[psi] = timestamp(wip->particle_spewers[0].particle_spew_time);

				// turn normals and origins to world space if we need to
				if (!vm_vec_same(&wip->particle_spewers[psi].particle_spew_offset, &vmd_zero_vector)) {	// dont xform unused vectors
					vm_vec_unrotate(&spawn_pos, &wip->particle_spewers[psi].particle_spew_offset, &obj->orient);
				} else {
					spawn_pos = vmd_zero_vector;
				}

				if (!vm_vec_same(&wip->particle_spewers[psi].particle_spew_velocity, &vmd_zero_vector)) {
					vm_vec_unrotate(&spawn_vel, &wip->particle_spewers[psi].particle_spew_velocity, &obj->orient);
				} else {
					spawn_vel = vmd_zero_vector;
				}

				// spew some particles
				if (wip->particle_spewers[psi].particle_spew_type == PSPEW_DEFAULT)	// default pspew type
				{		// do the default pspew
						vec3d direct, direct_temp, particle_pos;
						vec3d null_vec = ZERO_VECTOR;
						vec3d vel;
						float ang;

					for (idx = 0; idx < wip->particle_spewers[psi].particle_spew_count; idx++) {
						// get the backward vector of the weapon
						direct = obj->orient.vec.fvec;
						vm_vec_negate(&direct);

						// randomly perturb x, y and z
						
						// uvec
						ang = frand_range(-PI_2,PI_2);	// fl_radian(frand_range(-90.0f, 90.0f));	-optimized by nuke
						vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.fvec);			
						direct = direct_temp;
						vm_vec_scale(&direct, wip->particle_spewers[psi].particle_spew_scale);

						// rvec
						ang = frand_range(-PI_2,PI_2);	// fl_radian(frand_range(-90.0f, 90.0f));	-optimized by nuke
						vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.rvec);			
						direct = direct_temp;
						vm_vec_scale(&direct, wip->particle_spewers[psi].particle_spew_scale);

						// fvec
						ang = frand_range(-PI_2,PI_2);	// fl_radian(frand_range(-90.0f, 90.0f));	-optimized by nuke
						vm_rot_point_around_line(&direct_temp, &direct, ang, &null_vec, &obj->orient.vec.uvec);			
						direct = direct_temp;
						vm_vec_scale(&direct, wip->particle_spewers[psi].particle_spew_scale);

						// get a velocity vector of some percentage of the weapon's velocity
						vel = obj->phys_info.vel;
						vm_vec_scale(&vel, wip->particle_spewers[psi].particle_spew_vel);

						// maybe add in offset and initial velocity
						if (!vm_vec_same(&spawn_vel, &vmd_zero_vector)) { // add in particle velocity if its available
							vm_vec_add2(&vel, &spawn_vel);
						}
						if (!vm_vec_same(&spawn_pos, &vmd_zero_vector)) { // add offset if available
							vm_vec_add2(&direct, &spawn_pos);
						}

						if (wip->wi_flags & WIF_CORKSCREW) {
							vm_vec_add(&particle_pos, &obj->last_pos, &direct);
						} else {
							vm_vec_add(&particle_pos, &obj->pos, &direct);
						}

						// emit the particle
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle_create(&particle_pos, &vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_SMOKE);
						} else {
							particle_create(&particle_pos, &vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_BITMAP, wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				} else if (wip->particle_spewers[psi].particle_spew_type == PSPEW_HELIX) { // helix
					float segment_length = wip->max_speed * flFrametime; // determine how long the segment is
					float segment_angular_length = PI2 * wip->particle_spewers[psi].particle_spew_rotation_rate * flFrametime; 	// determine how much the segment rotates
					float rotation_value = (wp->lifeleft * PI2 * wip->particle_spewers[psi].particle_spew_rotation_rate) + wp->particle_spew_rand; // calculate a rotational start point based on remaining life
					float inc = 1.0f / wip->particle_spewers[psi].particle_spew_count;	// determine our incriment
					float particle_rot;
					vec3d input_pos_l = ZERO_VECTOR;
					
					for (float is = 0; is < 1; is += inc ) { // use iterator as a scaler
						particle_rot = rotation_value + (segment_angular_length * is); // find what point of the rotation were at
						input_vel.xyz.x = sinf(particle_rot) * wip->particle_spewers[psi].particle_spew_scale; // determine x/y velocity based on scale and rotation
						input_vel.xyz.y = cosf(particle_rot) * wip->particle_spewers[psi].particle_spew_scale;
						input_vel.xyz.z = wip->max_speed * wip->particle_spewers[psi].particle_spew_vel; // velocity inheritance
						vm_vec_unrotate(&output_vel, &input_vel, &obj->orient);				// orient velocity to weapon
						input_pos_l.xyz.x = input_vel.xyz.x * flFrametime * (1.0f - is);	// interpolate particle motion
						input_pos_l.xyz.y = input_vel.xyz.y * flFrametime * (1.0f - is);
						input_pos_l.xyz.z = segment_length * is;							// position particle correctly on the z axis
						vm_vec_unrotate(&input_pos, &input_pos_l, &obj->orient);			// orient to weapon
						vm_vec_sub(&output_pos, &obj->pos, &input_pos);						// translate to world space

						//maybe add in offset and initial velocity
						if (!vm_vec_same(&spawn_vel, &vmd_zero_vector)) { // add particle velocity if needed
							vm_vec_add2(&output_vel, &spawn_vel);
						}
						if (!vm_vec_same(&spawn_pos, &vmd_zero_vector)) { // add offset if needed
							vm_vec_add2(&output_pos, &spawn_pos);
						}

						//emit particles
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_SMOKE);
						} else {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_BITMAP, wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				} else if (wip->particle_spewers[psi].particle_spew_type == PSPEW_SPARKLER) { // sparkler
					vec3d temp_vel;
					output_vel = obj->phys_info.vel;
					vm_vec_scale(&output_vel, wip->particle_spewers[psi].particle_spew_vel);

					for (idx = 0; idx < wip->particle_spewers[psi].particle_spew_count; idx++) {
						// create a random unit vector and scale it
						vm_vec_rand_vec_quick(&input_vel);
						vm_vec_scale(&input_vel, wip->particle_spewers[psi].particle_spew_scale);
						
						if (wip->particle_spewers[psi].particle_spew_z_scale != 1.0f) {	// dont do the extra math for spherical effect
							temp_vel = input_vel;
							temp_vel.xyz.z *= wip->particle_spewers[psi].particle_spew_z_scale;	// for an oviod particle effect to better combine with laser effects
							vm_vec_unrotate(&input_vel, &temp_vel, &obj->orient);				// so it has to be rotated
						}

						vm_vec_add2(&output_vel, &input_vel); // add to weapon velocity
						output_pos = obj->pos;

						// maybe add in offset and initial velocity
						if (!vm_vec_same(&spawn_vel, &vmd_zero_vector)) { // add particle velocity if needed
							vm_vec_add2(&output_vel, &spawn_vel);
						}
						if (!vm_vec_same(&spawn_pos, &vmd_zero_vector)) { // add offset if needed
							vm_vec_add2(&output_pos, &spawn_pos);
						}

						// emit particles
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_SMOKE);
						} else {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_BITMAP, wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				} else if (wip->particle_spewers[psi].particle_spew_type == PSPEW_RING) {
					float inc = PI2 / wip->particle_spewers[psi].particle_spew_count;	

					for (float ir = 0; ir < PI2; ir += inc) { // use iterator for rotation
						input_vel.xyz.x = sinf(ir) * wip->particle_spewers[psi].particle_spew_scale; // generate velocity from rotation data
						input_vel.xyz.y = cosf(ir) * wip->particle_spewers[psi].particle_spew_scale;
						input_vel.xyz.z = obj->phys_info.fspeed * wip->particle_spewers[psi].particle_spew_vel;
						vm_vec_unrotate(&output_vel, &input_vel, &obj->orient); // rotate it to model

						output_pos = obj->pos;

						// maybe add in offset amd iitial velocity
						if (!vm_vec_same(&spawn_vel, &vmd_zero_vector)) { // add particle velocity if needed
							vm_vec_add2(&output_vel, &spawn_vel);
						}
						if (!vm_vec_same(&spawn_pos, &vmd_zero_vector)) { // add offset if needed
							vm_vec_add2(&output_pos, &spawn_pos);
						}

						// emit particles
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_SMOKE);
						} else {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_BITMAP, wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				} else if (wip->particle_spewers[psi].particle_spew_type == PSPEW_PLUME) {
					float ang_rand, len_rand, sin_ang, cos_ang;
					vec3d input_pos_l = ZERO_VECTOR;
					
					for (int i = 0; i < wip->particle_spewers[psi].particle_spew_count; i++) {
						// use polar coordinates to ensure a disk shaped spew plane
						ang_rand = frand_range(-PI,PI);
						len_rand = frand() * wip->particle_spewers[psi].particle_spew_scale;
						sin_ang = sinf(ang_rand);
						cos_ang = cosf(ang_rand);
						// compute velocity
						input_vel.xyz.x = wip->particle_spewers[psi].particle_spew_z_scale * -sin_ang;
						input_vel.xyz.y = wip->particle_spewers[psi].particle_spew_z_scale * -cos_ang;
						input_vel.xyz.z = obj->phys_info.fspeed * wip->particle_spewers[psi].particle_spew_vel;
						vm_vec_unrotate(&output_vel, &input_vel, &obj->orient); // rotate it to model
						// place particle on a disk prependicular to the weapon normal and rotate to model space
						input_pos_l.xyz.x = sin_ang * len_rand;
						input_pos_l.xyz.y = cos_ang * len_rand;
						vm_vec_unrotate(&input_pos, &input_pos_l, &obj->orient); // rotate to world
						vm_vec_sub(&output_pos, &obj->pos, &input_pos); // translate to world
						
						// maybe add in offset amd iitial velocity
						if (!vm_vec_same(&spawn_vel, &vmd_zero_vector)) { // add particle velocity if needed
							vm_vec_add2(&output_vel, &spawn_vel);
						}
						if (!vm_vec_same(&spawn_pos, &vmd_zero_vector)) { // add offset if needed
							vm_vec_add2(&output_pos, &spawn_pos);
						}

						//emit particles
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_SMOKE);
						} else {
							particle_create(&output_pos, &output_vel, wip->particle_spewers[psi].particle_spew_lifetime, wip->particle_spewers[psi].particle_spew_radius, PARTICLE_BITMAP, wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				}
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
	
	// if the hit object was a ship and we're doing damage scaling
	if ( (target->type == OBJ_SHIP) &&
		!(The_mission.ai_profile->flags & AIPF_DISABLE_WEAPON_DAMAGE_SCALING) &&
		!(Ship_info[Ships[target->instance].ship_info_index].flags2 & SIF2_DISABLE_WEAPON_DAMAGE_SCALING)
	) {
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
			// if the supercap is around 3/4 damage, apply nothing
			if(hull_pct <= 0.75f){
				return 0.0f;
			} else {
				total_scale *= SUPERCAP_DAMAGE_SCALE;
			}
		}

		// determine if this is a big damage ship
		is_big_damage_ship = (sip->flags & SIF_BIG_DAMAGE);

		// if this is a large ship, and is being hit by flak
		if(is_big_damage_ship && (wip->wi_flags & WIF_FLAK)){
			total_scale *= FLAK_DAMAGE_SCALE;
		}
		
		// if the weapon is a small weapon being fired at a big ship
		if( is_big_damage_ship && !(wip->wi_flags & (WIF_HURTS_BIG_SHIPS)) ){

			// if the player is firing it
			if ( from_player && !(The_mission.ai_profile->flags2 & AIPF2_PLAYER_WEAPON_SCALE_FIX)) {
				// if it's a laser weapon
				if(wip->subtype == WP_LASER){
					total_scale *= 0.01f;
				} else {
					total_scale *= 0.05f;
				}
			}

			// scale based on hull
			if(hull_pct > 0.1f){
				total_scale *= hull_pct;
			} else {
				return 0.0f;
			}
		}
	}
	
	return total_scale;
}
