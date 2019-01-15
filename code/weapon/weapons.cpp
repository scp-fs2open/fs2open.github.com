/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ai/aibig.h"
#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "cmeasure/cmeasure.h"
#include "debugconsole/console.h"
#include "fireball/fireballs.h"
#include "freespace.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "hud/hud.h"
#include "hud/hudartillery.h"
#include "iff_defs/iff_defs.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "math/staticrand.h"
#include "missionui/missionweaponchoice.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/objcollide.h"
#include "scripting/scripting.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "radar/radar.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "ship/shiphit.h"
#include "weapon/beam.h"	// for BEAM_TYPE_? definitions
#include "weapon/corkscrew.h"
#include "weapon/emp.h"
#include "weapon/flak.h"
#include "weapon/muzzleflash.h"
#include "weapon/swarm.h"
#include "particle/effects/SingleParticleEffect.h"
#include "particle/effects/BeamPiercingEffect.h"
#include "particle/effects/ParticleEmitterEffect.h"
#include "tracing/Monitor.h"
#include "tracing/tracing.h"
#include "weapon.h"


// Since SSMs are parsed after weapons, if we want to allow SSM strikes to be specified by name, we need to store those names until after SSMs are parsed.
typedef struct delayed_ssm_data {
	SCP_string filename;
	int linenum;
	SCP_string ssm_entry;
} delayed_ssm_data;
SCP_map<SCP_string, delayed_ssm_data> Delayed_SSM_data;
SCP_vector<SCP_string> Delayed_SSM_names;

typedef struct delayed_ssm_index_data {
	SCP_string filename;
	int linenum;
} delayed_ssm_index_data;
SCP_map<SCP_string, delayed_ssm_index_data> Delayed_SSM_indices_data;
SCP_vector<SCP_string> Delayed_SSM_indices;


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
const char *Weapon_subtype_names[] = {
	"Laser",
	"Missile",
	"Beam"
};
int Num_weapon_subtypes = sizeof(Weapon_subtype_names)/sizeof(Weapon_subtype_names[0]);

flag_def_list_new<Weapon::Burst_Flags> Burst_fire_flags[] = {
	{ "fast firing",		Weapon::Burst_Flags::Fast_firing,		true, false },
	{ "random length",		Weapon::Burst_Flags::Random_length,		true, false }
};

const size_t Num_burst_fire_flags = sizeof(Burst_fire_flags)/sizeof(flag_def_list_new<Weapon::Burst_Flags>);

weapon_explosions Weapon_explosions;

SCP_vector<lod_checker> LOD_checker;

flag_def_list_new<Weapon::Info_Flags> Weapon_Info_Flags[] = {
    { "spawn",							Weapon::Info_Flags::Spawn,								true, true }, //special case
    { "remote detonate",				Weapon::Info_Flags::Remote,								true, false },
    { "puncture",						Weapon::Info_Flags::Puncture,							true, false },
    { "big ship",						Weapon::Info_Flags::Big_only,							true, false },
    { "huge",							Weapon::Info_Flags::Huge,								true, false },
    { "bomber+",						Weapon::Info_Flags::Bomber_plus,						true, false },
    { "child",							Weapon::Info_Flags::Child,								true, false },
    { "bomb",							Weapon::Info_Flags::Bomb,								true, false },
    { "no dumbfire",					Weapon::Info_Flags::No_dumbfire,						true, false },
    { "in tech database",				Weapon::Info_Flags::In_tech_database,					true, false },
    { "player allowed",					Weapon::Info_Flags::Player_allowed,                     true, false },
    { "particle spew",					Weapon::Info_Flags::Particle_spew,						true, false },
    { "emp",							Weapon::Info_Flags::Emp,								true, false },
    { "esuck",							Weapon::Info_Flags::Energy_suck,						true, false },
    { "flak",							Weapon::Info_Flags::Flak,								true, false },
    { "corkscrew",						Weapon::Info_Flags::Corkscrew,							true, false },
    { "shudder",						Weapon::Info_Flags::Shudder,							true, false },
    { "electronics",					Weapon::Info_Flags::Electronics,						true, false },
    { "lockarm",						Weapon::Info_Flags::Lockarm,							true, false },
    { "beam",							Weapon::Info_Flags::Beam,								true, true }, //special case
    { "stream",							Weapon::Info_Flags::Stream,								true, false },
    { "supercap",						Weapon::Info_Flags::Supercap,							true, false },
    { "countermeasure",					Weapon::Info_Flags::Cmeasure,							true, false },
    { "ballistic",						Weapon::Info_Flags::Ballistic,							true, false },
    { "pierce shields",					Weapon::Info_Flags::Pierce_shields,						true, false },
    { "local ssm",						Weapon::Info_Flags::Local_ssm,							true, false },
    { "tagged only",					Weapon::Info_Flags::Tagged_only,						true, false },
    { "beam no whack",					Weapon::Info_Flags::NUM_VALUES,							false, true }, //special case
    { "cycle",							Weapon::Info_Flags::Cycle,								true, false },
    { "small only",						Weapon::Info_Flags::Small_only,							true, false },
    { "same turret cooldown",			Weapon::Info_Flags::Same_turret_cooldown,				true, false },
    { "apply no light",					Weapon::Info_Flags::Mr_no_lighting,						true, false },
    { "training",						Weapon::Info_Flags::Training,							true, false },
    { "smart spawn",					Weapon::Info_Flags::Smart_spawn,						true, false },
    { "inherit parent target",			Weapon::Info_Flags::Inherit_parent_target,				true, false },
    { "no emp kill",					Weapon::Info_Flags::No_emp_kill,						true, false },
    { "untargeted heat seeker",			Weapon::Info_Flags::Untargeted_heat_seeker,				true, false },
    { "no radius doubling",				Weapon::Info_Flags::Hard_target_bomb,					true, false },
    { "no subsystem homing",			Weapon::Info_Flags::Non_subsys_homing,					true, false },
    { "no lifeleft penalty",			Weapon::Info_Flags::No_life_lost_if_missed,				true, false },
    { "can be targeted",				Weapon::Info_Flags::Can_be_targeted,					true, false },
    { "show on radar",					Weapon::Info_Flags::Shown_on_radar,						true, false },
    { "show friendly on radar",			Weapon::Info_Flags::Show_friendly,						true, false },
    { "capital+",						Weapon::Info_Flags::Capital_plus,						true, false },
    { "chain external model fps",		Weapon::Info_Flags::External_weapon_fp,					true, false },
    { "external model launcher",		Weapon::Info_Flags::External_weapon_lnch,				true, false },
    { "takes blast damage",				Weapon::Info_Flags::Takes_blast_damage,					true, false },
    { "takes shockwave damage",			Weapon::Info_Flags::Takes_shockwave_damage,				true, false },
    { "hide from radar",				Weapon::Info_Flags::Dont_show_on_radar,					true, false },
    { "render flak",					Weapon::Info_Flags::Render_flak,						true, false },
    { "ciws",							Weapon::Info_Flags::Ciws,								true, false },
    { "anti-subsystem beam",			Weapon::Info_Flags::Antisubsysbeam,						true, false },
    { "no primary linking",				Weapon::Info_Flags::Nolink,								true, false },
    { "same emp time for capships",		Weapon::Info_Flags::Use_emp_time_for_capship_turrets,	true, false },
    { "no primary linked penalty",		Weapon::Info_Flags::No_linked_penalty,					true, false },
    { "no homing speed ramp",			Weapon::Info_Flags::No_homing_speed_ramp,				true, false },
    { "pulls aspect seekers",			Weapon::Info_Flags::Cmeasure_aspect_home_on,			true, false },
    { "turret interceptable",			Weapon::Info_Flags::Turret_Interceptable,				true, false },
    { "fighter interceptable",			Weapon::Info_Flags::Fighter_Interceptable,				true, false },
    { "aoe electronics",                Weapon::Info_Flags::Aoe_Electronics,                    true, false },
    { "apply recoil",                   Weapon::Info_Flags::Apply_Recoil,                       true, false },
    { "don't spawn if shot",            Weapon::Info_Flags::Dont_spawn_if_shot,                 true, false },
    { "die on lost lock",               Weapon::Info_Flags::Die_on_lost_lock,                   true, true  }, //special case
};

const size_t num_weapon_info_flags = sizeof(Weapon_Info_Flags) / sizeof(flag_def_list_new<Weapon::Info_Flags>);

int Num_weapon_types = 0;

int Num_weapons = 0;
int Weapons_inited = 0;
int Weapon_expl_initted = 0;

int laser_model_inner = -1;
int laser_model_outer = -1;

int missile_model = -1;

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

// scale factor for big ships getting hit by flak
#define FLAK_DAMAGE_SCALE				0.05f

//default time of a homing weapon to not home
#define HOMING_DEFAULT_FREE_FLIGHT_TIME	0.5f

// time delay between each swarm missile that is fired
#define SWARM_MISSILE_DELAY				150

// homing missiles have an extended lifetime so they don't appear to run out of gas before they can hit a moving target at extreme
// range. Check the comment in weapon_set_tracking_info() for more details
#define LOCKED_HOMING_EXTENDED_LIFE_FACTOR			1.2f

extern int compute_num_homing_objects(object *target_objp);

extern void fs2netd_add_table_validation(const char *tblname);


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

	for (size_t i = 0; i < ExplosionInfo.size(); i++) {
		if ( !stricmp(ExplosionInfo[i].lod[0].filename, filename)) {
			return (int)i;
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
	new_wei.lod[0].bitmap_id = bm_load_animation(filename, &new_wei.lod[0].num_frames, &new_wei.lod[0].fps, nullptr, nullptr, true);

	if (new_wei.lod[0].bitmap_id < 0) {
		Warning(LOCATION, "Weapon explosion '%s' does not have an LOD0 anim!", filename);

		// if we don't have the first then it's only safe to assume that the rest are missing or not usable
		return -1;
	}

	// 2 chars for the lod, 4 for the extension that gets added automatically
	if ( (MAX_FILENAME_LEN - strlen(filename)) > 6 ) {
		for (idx = 1; idx < expected_lods; idx++) {
			sprintf(name_tmp, "%s_%d", filename, idx);

			bitmap_id = bm_load_animation(name_tmp, &nframes, &nfps, nullptr, nullptr, true);

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


void parse_weapon_expl_tbl(const char *filename)
{
	uint i;
	lod_checker lod_check;
	
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Start");
		while (required_string_either("#End", "$Name:"))
		{
			memset(&lod_check, 0, sizeof(lod_checker));

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
			if ((lod_check.num_lods > 0) && (lod_check.num_lods < MAX_WEAPON_EXPL_LOD)) {
				// name check, update lod count if it already exists
				for (i = 0; i < LOD_checker.size(); i++) {
					if (!stricmp(LOD_checker[i].filename, lod_check.filename)) {
						LOD_checker[i].num_lods = lod_check.num_lods;
					}
				}

				// old entry not found, add new entry
				if (i == LOD_checker.size()) {
					LOD_checker.push_back(lod_check);
				}
			}
		}
		required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

/**
 * Clear out the Missile_obj_list
 */
void missile_obj_list_init()
{
	int i;

	list_init(&Missile_obj_list);
	for ( i = 0; i < MAX_MISSILE_OBJS; i++ ) {
		Missile_objs[i].flags = 0;
	}
}

/**
 * Add a node from the Missile_obj_list.
 * @note Only called from weapon_create()
 */
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

/**
 * Remove a node from the Missile_obj_list.
 * @note Only called from weapon_delete()
 */
void missle_obj_list_remove(int index)
{
	Assert(index >= 0 && index < MAX_MISSILE_OBJS);
	list_remove(&Missile_obj_list, &Missile_objs[index]);	
	Missile_objs[index].flags = 0;
}

/**
 * Called by the save/restore code to rebuild Missile_obj_list
 */
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

/**
 * Called externally to generate an address from an index into
 * the Missile_objs[] array
 */
missile_obj *missile_obj_return_address(int index)
{
	Assert(index >= 0 && index < MAX_MISSILE_OBJS);
	return &Missile_objs[index];
}

/**
 * Return the index of Weapon_info[].name that is *name.
 */
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
void parse_wi_flags(weapon_info *weaponp, flagset<Weapon::Info_Flags> wi_flags)
{
    const char *spawn_str = NOX("Spawn");
    const size_t spawn_str_len = strlen(spawn_str);

    //Make sure we HAVE flags :p
    if (!optional_string("$Flags:"))
        return;

	// To make sure +override doesn't overwrite previously parsed values we parse the flags into a separate flagset
    SCP_vector<SCP_string> unparsed_or_special;
	flagset<Weapon::Info_Flags> parsed_flags;
    parse_string_flag_list(parsed_flags, Weapon_Info_Flags, num_weapon_info_flags, &unparsed_or_special);

    if (optional_string("+override")) {
        // reseting the flag values if set to override the existing flags
        weaponp->wi_flags = wi_flags;
    }
	// Now add the parsed flags to the weapon flags
	weaponp->wi_flags |= parsed_flags;

    bool set_nopierce = false;

    for (auto flag = unparsed_or_special.begin(); flag != unparsed_or_special.end(); ++flag) {
        SCP_string flag_text = *flag;
        //deal with spawn flag
        if (!strnicmp(spawn_str, flag_text.c_str(), 5))
        {
            if (weaponp->num_spawn_weapons_defined < MAX_SPAWN_TYPES_PER_WEAPON)
            {
                //We need more spawning slots
                //allocate in slots of 10
                if ((Num_spawn_types % 10) == 0) {
                    Spawn_names = (char **)vm_realloc(Spawn_names, (Num_spawn_types + 10) * sizeof(*Spawn_names));
                }

                size_t	skip_length, name_length;
				std::unique_ptr<char[]> temp_string(new char[flag_text.size() + 1]);

                strcpy(temp_string.get(), flag_text.c_str());

                weaponp->wi_flags.set(Weapon::Info_Flags::Spawn);
                weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_type = (short)Num_spawn_types;
                skip_length = spawn_str_len + strspn(&temp_string[spawn_str_len], NOX(" \t"));
                char *num_start = strchr(&temp_string[skip_length], ',');
                if (num_start == NULL) {
                    weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count = DEFAULT_WEAPON_SPAWN_COUNT;
                    name_length = 999;
                }
                else {
                    weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count = (short)atoi(num_start + 1);
                    name_length = num_start - temp_string.get() - skip_length;
                }

                weaponp->total_children_spawned += weaponp->spawn_info[weaponp->num_spawn_weapons_defined].spawn_count;

                Spawn_names[Num_spawn_types] = vm_strndup(&flag_text[skip_length], name_length);
                Num_spawn_types++;
                weaponp->num_spawn_weapons_defined++;
            }
            else {
                Warning(LOCATION, "Illegal to have more than %d spawn types for one weapon.\nIgnoring weapon %s", MAX_SPAWN_TYPES_PER_WEAPON, weaponp->name);
            }
        }
        else if (!stricmp(NOX("beam"), flag_text.c_str())) {
            weaponp->wi_flags.set(Weapon::Info_Flags::Pierce_shields);
        }
        else if (!stricmp(NOX("no pierce shields"), flag_text.c_str())) {
            set_nopierce = true;
        }
        else if (!stricmp(NOX("beam no whack"), flag_text.c_str())) {
            Warning(LOCATION, "The \"beam no whack\" flag has been deprecated.  Set the beam's mass to 0 instead.  This has been done for you.\n");
            weaponp->mass = 0.0f;
        }
        else if (!stricmp(NOX("interceptable"), flag_text.c_str())) {
            weaponp->wi_flags.set(Weapon::Info_Flags::Turret_Interceptable);
            weaponp->wi_flags.set(Weapon::Info_Flags::Fighter_Interceptable);
        }
        else if (!stricmp(NOX("die on lost lock"), flag_text.c_str())) {
            if (!(weaponp->is_locked_homing())) {
                Warning(LOCATION, "\"die on lost lock\" may only be used for Homing Type ASPECT/JAVELIN!");
                weaponp->wi_flags.remove(Weapon::Info_Flags::Die_on_lost_lock);
            }
        }
        else {
            Warning(LOCATION, "Unrecognized flag in flag list for weapon %s: \"%s\"", weaponp->name, (*flag).c_str());
        }
    }

    //Do cleanup and sanity checks
        
	if (set_nopierce)
        weaponp->wi_flags.remove(Weapon::Info_Flags::Pierce_shields);

    if (weaponp->wi_flags[Weapon::Info_Flags::Hard_target_bomb] && !weaponp->wi_flags[Weapon::Info_Flags::Bomb]) {
        weaponp->wi_flags.remove(Weapon::Info_Flags::Hard_target_bomb);
        Warning(LOCATION, "Weapon %s is not a bomb but has \"no radius doubling\" set. Ignoring this flag", weaponp->name);
    }

    if (weaponp->wi_flags[Weapon::Info_Flags::In_tech_database])
        weaponp->wi_flags.set(Weapon::Info_Flags::Default_in_tech_database);

    if (weaponp->wi_flags[Weapon::Info_Flags::Flak]) {
        if (weaponp->wi_flags[Weapon::Info_Flags::Swarm] || weaponp->wi_flags[Weapon::Info_Flags::Corkscrew]) {
            weaponp->wi_flags.remove(Weapon::Info_Flags::Swarm);
            weaponp->wi_flags.remove(Weapon::Info_Flags::Corkscrew);
            Warning(LOCATION, "Swarm, Corkscrew, and Flak are mutually exclusive!  Removing Swarm and Corkscrew attributes from weapon %s.\n", weaponp->name);
        }
    }

    if (weaponp->wi_flags[Weapon::Info_Flags::Swarm] && weaponp->wi_flags[Weapon::Info_Flags::Corkscrew]) {
        weaponp->wi_flags.remove(Weapon::Info_Flags::Corkscrew);
        Warning(LOCATION, "Swarm and Corkscrew are mutually exclusive!  Defaulting to Swarm on weapon %s.\n", weaponp->name);
    }

    if (weaponp->wi_flags[Weapon::Info_Flags::Local_ssm]) {
        if (!weaponp->is_homing() || weaponp->subtype != WP_MISSILE) {
            Warning(LOCATION, "local ssm must be guided missile: %s", weaponp->name);
        }
    }

    if (weaponp->wi_flags[Weapon::Info_Flags::Small_only] && weaponp->wi_flags[Weapon::Info_Flags::Huge])
    {
        Warning(LOCATION, "\"small only\" and \"huge\" flags are mutually exclusive.\nThey are used together in %s\nAI will most likely not use this weapon", weaponp->name);
    }

    if (!weaponp->wi_flags[Weapon::Info_Flags::Spawn] && weaponp->wi_flags[Weapon::Info_Flags::Smart_spawn])
    {
        Warning(LOCATION, "\"smart spawn\" flag used without \"spawn\" flag in %s\n", weaponp->name);
    }

    if (weaponp->wi_flags[Weapon::Info_Flags::Inherit_parent_target] && (!weaponp->wi_flags[Weapon::Info_Flags::Child]))
    {
        Warning(LOCATION, "Weapon %s has the \"inherit parent target\" flag, but not the \"child\" flag.  No changes in behavior will occur.", weaponp->name);
    }

    if (!weaponp->wi_flags[Weapon::Info_Flags::Homing_heat] && weaponp->wi_flags[Weapon::Info_Flags::Untargeted_heat_seeker])
    {
        Warning(LOCATION, "Weapon '%s' has the \"untargeted heat seeker\" flag, but Homing Type is not set to \"HEAT\".", weaponp->name);
    }

    if (!weaponp->wi_flags[Weapon::Info_Flags::Cmeasure] && weaponp->wi_flags[Weapon::Info_Flags::Cmeasure_aspect_home_on])
    {
        weaponp->wi_flags.remove(Weapon::Info_Flags::Cmeasure_aspect_home_on);
        Warning(LOCATION, "Weapon %s has the \"pulls aspect seekers\" flag, but is not a countermeasure.\n", weaponp->name);
    }
}

void parse_shockwave_info(shockwave_create_info *sci, const char *pre_char)
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

	if (sci->outer_rad < sci->inner_rad) {
		Warning(LOCATION, "Shockwave outer radius must be greater than or equal to the inner radius!");
		sci->outer_rad = sci->inner_rad;
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

// function to parse the information for a specific weapon type.	
// return 0 if successful, otherwise return -1
#define WEAPONS_MULTITEXT_LENGTH 2048

int parse_weapon(int subtype, bool replace, const char *filename)
{
	char buf[WEAPONS_MULTITEXT_LENGTH];
	weapon_info *wip = NULL;
	char fname[NAME_LENGTH];
	int iff, idx;
	int primary_rearm_rate_specified=0;
	bool first_time = false;
	bool create_if_not_found  = true;
    flagset<Weapon::Info_Flags> wi_flags;

	required_string("$Name:");
	stuff_string(fname, F_NAME, NAME_LENGTH);
	diag_printf ("Weapon name -- %s\n", fname);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for weapon in non-modular table");
		}
		create_if_not_found = false;
	}

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
			Error(LOCATION, "Too many weapon classes before '%s'; maximum is %d.\n", fname, MAX_WEAPON_TYPES);
		}

		wip = &Weapon_info[Num_weapon_types];
		wip->reset();
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

		// Check if the text exceeds the limits
		auto current_line = wip->desc;
		size_t num_lines = 0;
		while (current_line != nullptr) {
			auto line_end = strchr(current_line, '\n');
			auto line_length = line_end - current_line;
			if (line_end == nullptr) {
				line_length = strlen(current_line);
			}

			if (line_length >= WEAPON_DESC_MAX_LENGTH) {
				error_display(0, "Weapon description line " SIZE_T_ARG " is too long. Maximum is %d.", num_lines + 1, WEAPON_DESC_MAX_LENGTH);
			}

			++num_lines;
			current_line = line_end == nullptr ? nullptr : line_end + 1; // Skip the \n character if it was a complete line
		}
		if (num_lines >= WEAPON_DESC_MAX_LINES) {
			error_display(0, "Weapon description has too many lines. Maximum is %d.", WEAPON_DESC_MAX_LINES);
		}
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
	}

	if (optional_string("$Tech Model:")) {
		stuff_string(wip->tech_model, F_NAME, MAX_FILENAME_LEN);

		if (optional_string("+Closeup_pos:")) {
			stuff_vec3d(&wip->closeup_pos);
		}

		if (optional_string("+Closeup_zoom:")) {
			stuff_float(&wip->closeup_zoom);
		}
	}

	// Weapon fadein effect, used when no ani is specified or weapon_select_3d is active
	wip->selection_effect = Default_weapon_select_effect; // By default, use the FS2 effect
	if(optional_string("$Selection Effect:")) {
		char effect[NAME_LENGTH];
		stuff_string(effect, F_NAME, NAME_LENGTH);
		if (!stricmp(effect, "FS2"))
			wip->selection_effect = 2;
		else if (!stricmp(effect, "FS1"))
			wip->selection_effect = 1;
		else if (!stricmp(effect, "off"))
			wip->selection_effect = 0;
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
		// This might be confusing at first glance. If we're a modular table (!first_time),
		// AND we're providing a new color for the laser (being in this block at all),
		// AND the RGB values for laser_color_1 and laser_color_2 match...
		// THEN we conclude that laser_color_2 wasn't explicitly defined before, and the modder would probably prefer
		// it if the laser didn't suddenly start changing colors from the new to the old over its lifespan. -MageKing17
		bool reset = (!first_time && (
			(wip->laser_color_1.red == wip->laser_color_2.red) &&
			(wip->laser_color_1.green == wip->laser_color_2.green) &&
			(wip->laser_color_1.blue == wip->laser_color_2.blue)));
		stuff_ubyte(&r);
		stuff_ubyte(&g);
		stuff_ubyte(&b);
		gr_init_color( &wip->laser_color_1, r, g, b );
		if (reset) {
			gr_init_color( &wip->laser_color_2, wip->laser_color_1.red, wip->laser_color_1.green, wip->laser_color_1.blue );
		}
	}

	// optional string for cycling laser colors
	if(optional_string("@Laser Color2:")){
		stuff_ubyte(&r);
		stuff_ubyte(&g);
		stuff_ubyte(&b);
		gr_init_color( &wip->laser_color_2, r, g, b );
	} else if (first_time) {
		gr_init_color( &wip->laser_color_2, wip->laser_color_1.red, wip->laser_color_1.green, wip->laser_color_1.blue );
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
		if ((!strcmp(wip->name, "SAAA") && (wip->mass == 4.0f))
			|| (!strcmp(wip->name, "MjolnirBeam") && (wip->mass == 1000.0f))
			|| (!strcmp(wip->name, "MjolnirBeam#home") && (wip->mass == 1000.0f)))
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
		// Min and max delay stuff for weapon fire wait randomization
		if (optional_string("+Max Delay:")) {
			stuff_float(&(wip->max_delay));
			diag_printf("Weapon fire max delay -- %7.3f\n", wip->max_delay);
		}
		if (optional_string("+Min Delay:")) {
			stuff_float(&(wip->min_delay));
			diag_printf("Weapon fire min delay -- %7.3f\n", wip->min_delay);
		}
	}

	if(optional_string("$Damage:")) {
		stuff_float(&wip->damage);
		//WMC - now that shockwave damage can be set for them individually,
		//do this automagically
		if(first_time) {
			wip->shockwave.damage = wip->damage;
		}
	}

	// Attenuation of non-beam primary weapon damage
	if(optional_string("$Damage Time:")) {
		stuff_float(&wip->damage_time);
		if(optional_string("+Attenuation Damage:")){
			stuff_float(&wip->atten_damage);
		} else if (optional_string_either("+Min Damage:", "+Max Damage:")) {
			Warning(LOCATION, "+Min Damage: and +Max Damage: in %s are deprecated, please change to +Attenuation Damage:.", wip->name);
			stuff_float(&wip->atten_damage);
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

	if(optional_string("$Flak Detonation Accuracy:")) {
		stuff_float(&wip->flak_detonation_accuracy);
	}

	if(optional_string("$Flak Targeting Accuracy:")) {
		stuff_float(&wip->flak_targeting_accuracy);
	}

	if(optional_string("$Untargeted Flak Range Penalty:")) {
		stuff_float(&wip->untargeted_flak_range_penalty);
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
			Warning(LOCATION, "Lifetime min for weapon '%s' cannot be less than 0. Setting to 0.\n", wip->name);
		}
	}

	if(optional_string("$Lifetime Max:")) {
		stuff_float(&wip->life_max);

		if(wip->life_max < 0.0f) {
			wip->life_max = 0.0f;
			Warning(LOCATION, "Lifetime max for weapon '%s' cannot be less than 0. Setting to 0.\n", wip->name);
		} else if (wip->life_max < wip->life_min) {
			wip->life_max = wip->life_min + 0.1f;
			Warning(LOCATION, "Lifetime max for weapon '%s' cannot be less than its Lifetime Min (%f) value. Setting to %f.\n", wip->name, wip->life_min, wip->life_max);
		} else {
			wip->lifetime = (wip->life_min+wip->life_max)*0.5f;
		}
	}

	if(wip->life_min >= 0.0f && wip->life_max < 0.0f) {
		wip->lifetime = wip->life_min;
		wip->life_min = -1.0f;
		Warning(LOCATION, "Lifetime min, but not lifetime max, specified for weapon %s. Assuming static lifetime of %.2f seconds.\n", wip->name, wip->lifetime);
	}

	if(optional_string("$Lifetime:")) {
		if(wip->life_min >= 0.0f || wip->life_max >= 0.0f) {
			Warning(LOCATION, "Lifetime min or max specified, but $Lifetime was also specified; min or max will be used.");
		}
		stuff_float(&wip->lifetime);
		if (wip->damage_time > wip->lifetime) {
			Warning(LOCATION, "Lifetime is lower than Damage Time, setting Damage Time to be one half the value of Lifetime.");
			wip->damage_time = 0.5f * wip->lifetime;
		}
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

	if (is_homing || (wip->is_homing()))
	{
		char	temp_type[NAME_LENGTH];

		// the following five items only need to be recorded if the weapon is a homing weapon
		if(optional_string("+Type:"))
		{
			stuff_string(temp_type, F_NAME, NAME_LENGTH);
			if (!stricmp(temp_type, NOX("HEAT")))
			{
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_aspect);
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_javelin);

                wip->wi_flags.set(Weapon::Info_Flags::Homing_heat); 
                wi_flags.set(Weapon::Info_Flags::Homing_heat);
                
				
			}
			else if (!stricmp(temp_type, NOX("ASPECT")))
			{
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_heat);
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_javelin);

                wip->wi_flags.set(Weapon::Info_Flags::Homing_aspect);
                wi_flags.set(Weapon::Info_Flags::Homing_aspect);
			}
			else if (!stricmp(temp_type, NOX("JAVELIN")))
			{
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_heat);
                wip->wi_flags.remove(Weapon::Info_Flags::Homing_aspect);

                wip->wi_flags.set(Weapon::Info_Flags::Homing_javelin);
                wi_flags.set(Weapon::Info_Flags::Homing_javelin);
			}

            wip->wi_flags.set(Weapon::Info_Flags::Turns);
            wi_flags.set(Weapon::Info_Flags::Turns);
			//If you want to add another weapon, remember you need to reset
			//ALL homing flags.
		}

		if (wip->wi_flags[Weapon::Info_Flags::Homing_heat])
		{
			float	view_cone_angle;

			if(optional_string("+Turn Time:")) {
				stuff_float(&wip->turn_time);
			}

			if(optional_string("+View Cone:")) {
				stuff_float(&view_cone_angle);
				wip->fov = cosf(fl_radians(view_cone_angle * 0.5f));
			}

			if (optional_string("+Seeker Strength:"))
			{
				//heat default seeker strength is 3
				stuff_float(&wip->seeker_strength);
                wip->wi_flags.set(Weapon::Info_Flags::Custom_seeker_str);
				if (wip->seeker_strength <= 0)
				{
					Warning(LOCATION,"Seeker Strength for missile \'%s\' must be greater than zero\nReseting value to default.", wip->name);
					wip->seeker_strength = 3.0f;
                    wip->wi_flags.remove(Weapon::Info_Flags::Custom_seeker_str);
				}
			}
			else
			{
				if(!(wip->wi_flags[Weapon::Info_Flags::Custom_seeker_str]))
					wip->seeker_strength = 3.0f;
			}

			if (optional_string("+Target Lead Scaler:"))
			{
				stuff_float(&wip->target_lead_scaler);
                if (wip->target_lead_scaler == 0.0f)
                    wip->wi_flags.remove(Weapon::Info_Flags::Variable_lead_homing);
				else {
                    wip->wi_flags.set(Weapon::Info_Flags::Variable_lead_homing);
                    wi_flags.set(Weapon::Info_Flags::Variable_lead_homing);
				}
			}
		}
		else if ((wip->wi_flags[Weapon::Info_Flags::Homing_aspect]) || (wip->wi_flags[Weapon::Info_Flags::Homing_javelin]))
		{
			if(optional_string("+Turn Time:")) {
				stuff_float(&wip->turn_time);
			}

			if(optional_string("+View Cone:")) {
				float	view_cone_angle;
				stuff_float(&view_cone_angle);
				wip->fov = (float)cos(fl_radians(view_cone_angle * 0.5f));
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
                wip->wi_flags.set(Weapon::Info_Flags::Custom_seeker_str);
				if (wip->seeker_strength <= 0)
				{
					Warning(LOCATION,"Seeker Strength for missile \'%s\' must be greater than zero\nReseting value to default.", wip->name);
					wip->seeker_strength = 2.0f;
					wip->wi_flags.remove(Weapon::Info_Flags::Custom_seeker_str);
				}
			} 
			else
			{
				if(!(wip->wi_flags[Weapon::Info_Flags::Custom_seeker_str]))
					wip->seeker_strength = 2.0f;
			}
			if (optional_string("+Target Lead Scaler:"))
			{
				stuff_float(&wip->target_lead_scaler);
				if (wip->target_lead_scaler == 1.0f)
					wip->wi_flags.remove(Weapon::Info_Flags::Variable_lead_homing);
				else {
					wip->wi_flags.set(Weapon::Info_Flags::Variable_lead_homing);
					wi_flags.set(Weapon::Info_Flags::Variable_lead_homing);
				}
			}

			if (wip->is_locked_homing()) {
				// locked homing missiles have a much longer lifespan than the AI think they do
				wip->max_lifetime = wip->lifetime * LOCKED_HOMING_EXTENDED_LIFE_FACTOR; 
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
		wip->wi_flags.set(Weapon::Info_Flags::Swarm);
		wi_flags.set(Weapon::Info_Flags::Swarm);
	}

	// *Swarm wait token    -Et1
	if((wip->wi_flags[Weapon::Info_Flags::Swarm]) && optional_string( "+SwarmWait:" ))
	{
		float SwarmWait;
		stuff_float( &SwarmWait );
		if( SwarmWait > 0.0f && SwarmWait * wip->swarm_count < wip->fire_wait )
		{
			wip->SwarmWait = int( SwarmWait * 1000 );
		}
	}

	if(optional_string("$Acceleration Time:")) {
		stuff_float(&wip->acceleration_time);
	}

	if(optional_string("$Velocity Inherit:")) {
		stuff_float(&wip->vel_inherit_amount);
		wip->vel_inherit_amount /= 100.0f; // % -> 0..1
	}

	if(optional_string("$Free Flight Time:")) {
		stuff_float(&(wip->free_flight_time));
	} else if(first_time && is_homing) {
		wip->free_flight_time = HOMING_DEFAULT_FREE_FLIGHT_TIME;
	}

	if(optional_string("$Free Flight Speed:")) {
		float temp;
		stuff_float(&temp);
		nprintf(("Warning", "Ignoring free flight speed for weapon '%s'\n", wip->name));
	}
	//Optional one-shot sound to play at the beginning of firing
	parse_game_sound("$PreLaunchSnd:", &wip->pre_launch_snd);

	//Optional delay for Pre-Launch sound
	if(optional_string("+PreLaunchSnd Min Interval:"))
	{
		stuff_int(&wip->pre_launch_snd_min_interval);
	}

	//Launch sound
	parse_game_sound("$LaunchSnd:", &wip->launch_snd);

	//Impact sound
	parse_game_sound("$ImpactSnd:", &wip->impact_snd);

	//Disarmed impact sound
	parse_game_sound("$Disarmed ImpactSnd:", &wip->impact_snd);

	parse_game_sound("$FlyBySnd:", &wip->flyby_snd);

	parse_game_sound("$TrackingSnd:", &wip->hud_tracking_snd);
	
	parse_game_sound("$LockedSnd:", &wip->hud_locked_snd);

	parse_game_sound("$InFlightSnd:", &wip->hud_in_flight_snd);

	if (optional_string("+Inflight sound type:"))
	{
		SCP_string type;

		stuff_string(type, F_NAME);

		if (!stricmp(type.c_str(), "TARGETED"))
		{
			wip->in_flight_play_type = TARGETED;
		}
		else if (!stricmp(type.c_str(), "UNTARGETED"))
		{
			wip->in_flight_play_type = UNTARGETED;
		}
		else if (!stricmp(type.c_str(), "ALWAYS"))
		{
			wip->in_flight_play_type = ALWAYS;
		}
		else
		{
			Warning(LOCATION, "Unknown in-flight sound type \"%s\"!", type.c_str());
			wip->in_flight_play_type = ALWAYS;
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

	parse_wi_flags(wip, wi_flags);

	// be friendly; make sure ballistic flags are synchronized - Goober5000
	// primary
	if (subtype == WP_LASER)
	{
		// ballistic
		if (wip->wi_flags[Weapon::Info_Flags::Ballistic])
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
		if (wip->wi_flags[Weapon::Info_Flags::Ballistic])
		{
			Warning(LOCATION, "Secondary weapon %s can't be ballistic.  Removing this flag...\n", wip->name);
			wip->wi_flags.remove(Weapon::Info_Flags::Ballistic);
		}
	}

	// also make sure EMP is friendly - Goober5000
	if (wip->wi_flags[Weapon::Info_Flags::Emp])
	{
		if (!wip->shockwave.outer_rad)
		{
			Warning(LOCATION, "Outer blast radius of weapon %s is zero - EMP will not work.\nAdd $Outer Radius to weapon table entry.\n", wip->name);
		}
	}

	// also make sure secondaries and ballistic primaries do not have 0 cargo size
	if (subtype == WP_MISSILE || wip->wi_flags[Weapon::Info_Flags::Ballistic])
	{
		if (wip->cargo_size == 0.0f)
		{
			Warning(LOCATION, "Cargo size of weapon %s cannot be 0.  Setting to 1.\n", wip->name);
			wip->cargo_size = 1.0f;
		}
	}

	if ( optional_string("$Trail:") ) {
		trail_info *ti = &wip->tr_info;
        wip->wi_flags.set(Weapon::Info_Flags::Trail);		// missile leaves a trail

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

		if ( required_string("+Bitmap:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
			generic_bitmap_init(&ti->texture, fname);
		}

		if ( optional_string("+Faded Out Sections:") ) {
			stuff_int(&ti->n_fade_out_sections);
		}
	}

	// read in filename for icon that is used in weapons selection
	if ( optional_string("$Icon:") ) {
		stuff_string(wip->icon_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in filename for animation that is used in weapons selection
	if ( optional_string("$Anim:") ) {
		stuff_string(wip->anim_filename, F_NAME, MAX_FILENAME_LEN);
	}

	if (optional_string("$Impact Effect:")) {
		wip->impact_weapon_expl_effect = particle::util::parseEffect(wip->name);
	} else {
		// This is for compatibility with old tables
		// Do not add features here!

		float size = 1.0f;
		int bitmapIndex = -1;

		// $Impact Effect was not found, parse the old values
		if (optional_string("$Impact Explosion:")) {
			stuff_string(fname, F_NAME, NAME_LENGTH);

			if (VALID_FNAME(fname))
			{
				bitmapIndex = bm_load_animation(fname);
				
				if (bitmapIndex < 0)
				{
					Warning(LOCATION, "Couldn't load effect '%s' for weapon '%s'.", fname, wip->name);
				}
			}
		}

		if (optional_string("$Impact Explosion Radius:"))
			stuff_float(&size);

		if (bitmapIndex >= 0 && size > 0.0f)
		{
			using namespace particle;

			// Only beams do this randomization
			if (subtype == WP_BEAM)
			{
				// The original formula is (1.2f + 0.007f * (float)(rand() % 100)) which generates values within [1.2, 1.9)
				auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapIndex, size * 1.2f, size * 1.9f);
				wip->impact_weapon_expl_effect = ParticleManager::get()->addEffect(singleEffect);
			}
			else
			{
				auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapIndex, size, size);
				wip->impact_weapon_expl_effect = ParticleManager::get()->addEffect(singleEffect);
			}
		}
	}

	if ( optional_string("$Shield Impact Explosion Radius:") ) {
		stuff_float(&wip->shield_impact_explosion_radius);
	} else if (first_time) {
		using namespace particle;

		// Default value
		wip->shield_impact_explosion_radius = 1.0f;
		if (wip->impact_weapon_expl_effect.isValid()) {
			auto singleEffect = dynamic_cast<effects::SingleParticleEffect*>(ParticleManager::get()->getEffect(wip->impact_weapon_expl_effect));

			if (singleEffect)
			{
				// Initialize with value of the previously created single particle effect
				wip->shield_impact_explosion_radius = singleEffect->getProperties().m_radius.next();
			}
		}
	}

	if (optional_string("$Dinky Impact Effect:")) {
		wip->dinky_impact_weapon_expl_effect = particle::util::parseEffect(wip->name);
	} else {
		// This is for compatibility with old tables
		// Do not add features here!

		if (optional_string("$Dinky Impact Explosion:")) {
			stuff_string(fname, F_NAME, NAME_LENGTH);

			int bitmapID = -1;
			float size = 1.0f;

			if (VALID_FNAME(fname))
			{
				bitmapID = bm_load_animation(fname);

				if (bitmapID < 0)
				{
					Warning(LOCATION, "Couldn't load effect '%s' for weapon '%s'.", fname, wip->name);
				}
			}

			if (optional_string("$Dinky Impact Explosion Radius:"))
			{
				stuff_float(&size);
			}

			if (bitmapID >= 0 && size > 0.0f)
			{
				using namespace particle;

				// Only beams do this randomization
				if (subtype == WP_BEAM)
				{
					// The original formula is (1.2f + 0.007f * (float)(rand() % 100)) which generates values within [1.2, 1.9)
					auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapID, size * 1.2f, size * 1.9f);
					wip->dinky_impact_weapon_expl_effect = ParticleManager::get()->addEffect(singleEffect);
				}
				else
				{
					auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapID, size, size);
					wip->dinky_impact_weapon_expl_effect = ParticleManager::get()->addEffect(singleEffect);
				}
			}
		}
		else if (first_time) {
			wip->dinky_impact_weapon_expl_effect = wip->impact_weapon_expl_effect;
		}

		// Slight variation from original, as the original size is not known anymore the whole effect is copied
		// If the radius was stull specified it needs to be parsed to not mess up something else
		if (optional_string("$Dinky Impact Explosion Radius:"))
		{
			float temp;
			stuff_float(&temp);
		}
	}

	if (optional_string("$Piercing Impact Effect:")) {
		wip->piercing_impact_effect = particle::util::parseEffect(wip->name);
		if (first_time)
		{
			// The secondary effect is only needed if the old effect got parsed
			wip->piercing_impact_secondary_effect = particle::ParticleEffectHandle::invalid();
		}
	}
	else
	{
		// This is for compatibility with old tables
		// Do not add features here!

		using namespace particle;
		using namespace effects;

		int effectIndex = -1;
		float radius = 0.0f;
		int count = 0;
		float life = 0.0f;
		float velocity = 0.0f;
		float back_velocity = 0.0f;
		float variance = 0.0f;

		particle_emitter emitter;
		memset(&emitter, 0, sizeof(emitter));

		if (optional_string("$Piercing Impact Explosion:"))
		{
			stuff_string(fname, F_NAME, NAME_LENGTH);

			effectIndex = bm_load_animation(fname);

			if (effectIndex < 0)
			{
				Warning(LOCATION, "Failed to load effect '%s' for weapon %s!", fname, wip->name);
			}
		}

		if (optional_string("$Piercing Impact Radius:"))
			stuff_float(&radius);

		if (optional_string("$Piercing Impact Velocity:"))
			stuff_float(&velocity);

		if (optional_string("$Piercing Impact Splash Velocity:"))
			stuff_float(&back_velocity);

		if (optional_string("$Piercing Impact Variance:"))
			stuff_float(&variance);

		if (optional_string("$Piercing Impact Life:"))
			stuff_float(&life);

		if (optional_string("$Piercing Impact Particles:"))
			stuff_int(&count);

		if (effectIndex >= 0 && radius != 0.0f)
		{
			emitter.max_vel = 2.0f * velocity;
			emitter.min_vel = 0.5f * velocity;
			emitter.max_life = 2.0f * life;
			emitter.min_life = 0.25f * life;
			emitter.num_high = 2 * count;
			emitter.num_low = count / 2;
			emitter.normal_variance = variance;
			emitter.max_rad = 2.0f * radius;
			emitter.min_rad = 0.5f * radius;
			emitter.vel = vmd_zero_vector;

			auto emitterEffect = new ParticleEmitterEffect();
			emitterEffect->setValues(emitter, effectIndex, 10.0f);
			wip->piercing_impact_effect = ParticleManager::get()->addEffect(emitterEffect);

			if (back_velocity != 0.0f)
			{
				emitter.max_vel = 2.0f * back_velocity;
				emitter.min_vel = 0.5f * back_velocity;
				emitter.num_high /= 2;
				emitter.num_low /= 2;

				auto secondaryEffect = new ParticleEmitterEffect();
				secondaryEffect->setValues(emitter, effectIndex, 10.0f);
				wip->piercing_impact_secondary_effect = ParticleManager::get()->addEffect(secondaryEffect);
			}
		}
	}

	if (optional_string("$Inflight Effect:")) {
		auto effetIdx = particle::util::parseEffect(wip->name);
		wip->state_effects.insert(std::make_pair(WeaponState::NORMAL, effetIdx));
	}

	if (wip->subtype == WP_MISSILE)
	{
		if (optional_string("$Freeflight Effect:")) {
			auto effetIdx = particle::util::parseEffect(wip->name);
			wip->state_effects.insert(std::make_pair(WeaponState::FREEFLIGHT, effetIdx));
		}
		if (optional_string("$Ignition Effect:")) {
			auto effetIdx = particle::util::parseEffect(wip->name);
			wip->state_effects.insert(std::make_pair(WeaponState::IGNITION, effetIdx));
		}
		if (optional_string("$Homed Flight Effect:")) {
			auto effetIdx = particle::util::parseEffect(wip->name);
			wip->state_effects.insert(std::make_pair(WeaponState::HOMED_FLIGHT, effetIdx));
		}
		if (optional_string("$Unhomed Flight Effect:")) {
			auto effetIdx = particle::util::parseEffect(wip->name);
			wip->state_effects.insert(std::make_pair(WeaponState::UNHOMED_FLIGHT, effetIdx));
		}
	}

	// muzzle flash
	if ( optional_string("$Muzzleflash:") ) {
		stuff_string(fname, F_NAME, NAME_LENGTH);

		// look it up
		wip->muzzle_flash = mflash_lookup(fname);
	}

	// EMP optional stuff (if WIF_EMP is not set, none of this matters, anyway)
	if( optional_string("$EMP Intensity:") ){
		stuff_float(&wip->emp_intensity);
	}
	
	if( optional_string("$EMP Time:") ){
		stuff_float(&wip->emp_time);
	}

	// This is an optional modifier for a weapon that uses the "apply recoil" flag. recoil_force in ship.cpp line 10445 is multiplied by this if defined.
	if (optional_string("$Recoil Modifier:")){
		if (!(wip->wi_flags[Weapon::Info_Flags::Apply_Recoil])){
			Warning(LOCATION, "$Recoil Modifier specified for weapon %s but this weapon does not have the \"apply recoil\" weapon flag set. Automatically setting the flag", wip->name);
            wip->wi_flags.set(Weapon::Info_Flags::Apply_Recoil);
		}
		stuff_float(&wip->recoil_modifier);
	}

	// Energy suck optional stuff (if WIF_ENERGY_SUCK is not set, none of this matters anyway)
	if( optional_string("$Leech Weapon:") ){
		stuff_float(&wip->weapon_reduce);
	}

	if( optional_string("$Leech Afterburner:") ){
		stuff_float(&wip->afterburner_reduce);
	}

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

		if(optional_string("+Area Of Effect")) {
            wip->wi_flags.set(Weapon::Info_Flags::Aoe_Electronics);
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

	if (wip->wi_flags[Weapon::Info_Flags::Local_ssm] && optional_string("$Local SSM:"))
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
		if (!(wip->wi_flags[Weapon::Info_Flags::Cmeasure]))
		{
			Warning(LOCATION,"Weapon \'%s\' has countermeasure information defined, but the \"countermeasure\" flag wasn\'t found in the \'$Flags:\' field.\n", wip->name);
		}

		if (optional_string("+Heat Effectiveness:"))
			stuff_float(&wip->cm_heat_effectiveness);

		if (optional_string("+Aspect Effectiveness:"))
			stuff_float(&wip->cm_aspect_effectiveness);

		if (optional_string("+Effective Radius:"))
			stuff_float(&wip->cm_effective_rad);

		if (optional_string("+Missile Detonation Radius:"))
			stuff_float(&wip->cm_detonation_rad);

		if (optional_string("+Single Missile Kill:"))
			stuff_boolean(&wip->cm_kill_single);

		if (optional_string("+Pulse Interval:")) {
			stuff_int(&wip->cmeasure_timer_interval);
		}
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
		parse_game_sound("+BeamSound:", &wip->b_info.beam_loop_sound);

		// warmup sound
		parse_game_sound("+WarmupSound:", &wip->b_info.beam_warmup_sound);

		// warmdown sound
		parse_game_sound("+WarmdownSound:", &wip->b_info.beam_warmdown_sound);

		// glow bitmap
		if (optional_string("+Muzzleglow:") ) {
			stuff_string(fname, F_NAME, NAME_LENGTH);
			generic_anim_init(&wip->b_info.beam_glow, fname);
		}

		if (optional_string("+Directional Glow:")) {
			stuff_float(&wip->b_info.glow_length);
			wip->b_info.directional_glow = true;
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

		if (optional_string("+Beam Flash Particle Effect:")) {
			wip->flash_impact_weapon_expl_effect = particle::util::parseEffect(wip->name);
		} else {
			// This is for compatibility with old tables
			// Do not add features here!

			int bitmapIndex = -1;
			float size = 0.0f;
			bool defaultEffect = false;

			if (optional_string("+Beam Flash Effect:")) {
				stuff_string(fname, F_NAME, NAME_LENGTH);

				if (VALID_FNAME(fname))
				{
					bitmapIndex = bm_load_animation(fname);

					if (bitmapIndex < 0)
					{
						Warning(LOCATION, "Failed to load effect '%s' for weapon '%s'!", fname, wip->name);
					}
				}
			}

			if (bitmapIndex < 0)
			{
				// Default to the smoke particle effect
				bitmapIndex = bm_load_animation("particlesmoke01");
				defaultEffect = true;
			}

			if (optional_string("+Beam Flash Radius:"))
				stuff_float(&size);

			if (bitmapIndex >= 0 && size > 0.0f)
			{
				using namespace particle;

				if (defaultEffect)
				{
					// 'size * 1.5f * 0.005f' is another weird thing, the original code scales the lifetime of the flash particles based on size
					// so the new effects have to simulate that, but that onyl applies to the default effect, not a custom effect
					// seriously, who though that would be a good idea?
					auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapIndex, size * 1.2f, size * 1.9f, size * 1.5f * 0.005f);
					wip->flash_impact_weapon_expl_effect = ParticleManager::get()->
						addEffect(singleEffect);
				}
				else
				{
					auto singleEffect = effects::SingleParticleEffect::createInstance(bitmapIndex, size * 1.2f, size * 1.9f);
					wip->flash_impact_weapon_expl_effect = ParticleManager::get()->
						addEffect(singleEffect);
				}
			}
		}

		if (optional_string("+Beam Piercing Particle Effect:")) {
			wip->piercing_impact_effect = particle::util::parseEffect(wip->name);
		} else {
			// This is for compatibility with old tables
			// Do not add features here!

			int bitmapIndex = -1;
			float radius = 0.0f;
			float velocity = 0.0f;
			float back_velocity = 0.0f;
			float variance = 0.0f;

			if (optional_string("+Beam Piercing Effect:"))
			{
				stuff_string(fname, F_NAME, NAME_LENGTH);

				if (VALID_FNAME(fname))
				{
					bitmapIndex = bm_load_animation(fname);

					if (bitmapIndex < 0)
					{
						Warning(LOCATION, "Failed to load effect '%s' for weapon %s!", fname, wip->name);
					}
				}
			}

			if (optional_string("+Beam Piercing Radius:"))
				stuff_float(&radius);

			if (optional_string("+Beam Piercing Effect Velocity:"))
				stuff_float(&velocity);

			if (optional_string("+Beam Piercing Splash Effect Velocity:"))
				stuff_float(&back_velocity);

			if (optional_string("+Beam Piercing Effect Variance:"))
				stuff_float(&variance);

			if (bitmapIndex >= 0 && radius > 0.0f)
			{
				using namespace particle;
				using namespace effects;

				auto piercingEffect = new BeamPiercingEffect();
				piercingEffect->setValues(bitmapIndex, radius, velocity, back_velocity, variance);

				wip->piercing_impact_effect = ParticleManager::get()->
					addEffect(piercingEffect);
			}
		}

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

				// invisible textures are okay - see weapon_clean_entries()
				generic_anim_init(&bsip->texture, fname);
			}

			// The E -- Dummied out due to not being used anywhere
			if ( optional_string("+RGBA Inner:") ) {
				ubyte dummy;
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
			}

			// The E -- Dummied out due to not being used anywhere
			if ( optional_string("+RGBA Outer:") ) {
				ubyte dummy;
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
				stuff_ubyte(&dummy);
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
		if (!( wip->wi_flags[Weapon::Info_Flags::Particle_spew] )) {
			Warning(LOCATION, "$Pspew specified for weapon %s but this weapon does not have the \"Particle Spew\" weapon flag set. Automatically setting the flag", wip->name); 
            wip->wi_flags.set(Weapon::Info_Flags::Particle_spew);
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
						spew_index = (int)s;
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
				// for compatibility with existing tables that don't have a type tag
				} else if (wip->particle_spewers[spew_index].particle_spew_type == PSPEW_NONE) { // make sure the omission of type wasn't to edit an existing entry
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
					stuff_vec3d(&wip->particle_spewers[spew_index].particle_spew_offset);
				}

				if (optional_string("+Initial Velocity:")) {
					stuff_vec3d(&wip->particle_spewers[spew_index].particle_spew_velocity);
				}

				if (optional_string("+Bitmap:")) {
					stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
					generic_anim_init(&wip->particle_spewers[spew_index].particle_spew_anim, fname);
				}
			}
		}	
	}
	// check to see if the pspew flag was enabled but no pspew tags were given, for compatability with retail tables
	if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) {
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
        wip->wi_flags.set(Weapon::Info_Flags::Tag);
	}	

	if( optional_string("$SSM:")){
		if (stuff_int_optional(&wip->SSM_index) != 2) {
			// We can't make an SSM lookup yet, because weapons are parsed first, but we can save the data to process later. -MageKing17
			stuff_string(fname, F_NAME, NAME_LENGTH);
			delayed_ssm_data temp_data;
			temp_data.filename = filename;
			temp_data.linenum = get_line_num();
			temp_data.ssm_entry = fname;
			if (Delayed_SSM_data.find(wip->name) == Delayed_SSM_data.end())
				Delayed_SSM_names.push_back(wip->name);
			Delayed_SSM_data[wip->name] = temp_data;
		} else {
			// We'll still want to validate the index later. -MageKing17
			delayed_ssm_index_data temp_data;
			temp_data.filename = filename;
			temp_data.linenum = get_line_num();
			if (Delayed_SSM_indices_data.find(wip->name) == Delayed_SSM_indices_data.end())
				Delayed_SSM_indices.push_back(wip->name);
			Delayed_SSM_indices_data[wip->name] = temp_data;
		}
	}// SSM index -Bobboau

	if(optional_string("$FOF:")){
		stuff_float(&wip->field_of_fire);

		if(optional_string("+FOF Spread Rate:")){
			stuff_float(&wip->fof_spread_rate);
			if(required_string("+FOF Reset Rate:")){
				stuff_float(&wip->fof_reset_rate);
			}

			if(required_string("+Max FOF:")){
				float max_fof;
				stuff_float(&max_fof);
				wip->max_fof_spread = max_fof - wip->field_of_fire;

				if (wip->max_fof_spread <= 0.0f) {
					Warning(LOCATION, "WARNING: +Max FOF must be at least as big as $FOF for '%s'! Defaulting to match $FOF, no spread will occur!", wip->name);
					wip->max_fof_spread = 0.0f;
				}
			}
		}
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

	// New decal system parsing
	if (optional_string("$Impact Decal:")) {
		decals::parseDecalReference(wip->impact_decal, create_if_not_found);
	}

	if (optional_string("$Transparent:")) {
        wip->wi_flags.set(Weapon::Info_Flags::Transparent);

		required_string("+Alpha:");
		stuff_float(&wip->alpha_max);

		if (wip->alpha_max > 1.0f)
			wip->alpha_max = 1.0f;

		if (wip->alpha_max <= 0.0f) {
			Warning(LOCATION, "WARNING:  Alpha is set to 0 or a negative value for '%s'!  Defaulting to 1.0!", wip->name);
		}

		if (optional_string("+Alpha Min:")) {
			stuff_float(&wip->alpha_min);
            CLAMP(wip->alpha_min, 0.0f, 1.0f);
		}

		if (optional_string("+Alpha Cycle:")) {
			stuff_float(&wip->alpha_cycle);

			if (wip->alpha_max == wip->alpha_min)
				Warning(LOCATION, "WARNING:  Alpha is set to cycle for '%s', but max and min values are the same!", wip->name);
		}
	}

	if (optional_string("$Weapon Hitpoints:")) {
		stuff_int(&wip->weapon_hitpoints);
	} else if (first_time && (wip->wi_flags[Weapon::Info_Flags::Turret_Interceptable, Weapon::Info_Flags::Fighter_Interceptable])) {
		wip->weapon_hitpoints = 25;
	}

	// making sure bombs get their hitpoints assigned
	if ((wip->wi_flags[Weapon::Info_Flags::Bomb]) && (wip->weapon_hitpoints == 0)) {
		wip->weapon_hitpoints = 50;
	}

	if(optional_string("$Armor Type:")) {
		stuff_string(buf, F_NAME, WEAPONS_MULTITEXT_LENGTH);
		wip->armor_type_idx = armor_type_get_idx(buf);

		if(wip->armor_type_idx == -1)
			Warning(LOCATION,"Invalid armor name %s specified for weapon %s", buf, wip->name);
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
		parse_string_flag_list(wip->burst_flags, Burst_fire_flags, Num_burst_fire_flags, NULL);
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
	if ((wip->wi_flags[Weapon::Info_Flags::Tag]) && (wip->wi_flags[Weapon::Info_Flags::Tagged_only]))
	{
		Warning(LOCATION, "%s is a tag missile, but the target must be tagged to shoot it", wip->name);
	}

	// if burst delay is longer than firewait skip the whole burst fire option
	if (wip->burst_delay >= wip->fire_wait)
		wip->burst_shots = 0;

	// Set up weapon failure
	if (optional_string("$Failure Rate:")) {
		stuff_float(&wip->failure_rate);
		if (optional_string("+Failure Substitute:")) {
			stuff_string(wip->failure_sub_name, F_NAME);
		}
	}

	/* Generate a substitution pattern for this weapon.
	This pattern is very naive such that it calculates the lowest common denominator as being all of
	the periods multiplied together.
	*/
	while ( optional_string("$substitute:") ) {
		char subname[NAME_LENGTH];
		int period = 0;
		int index = 0;
		int offset = 0;
		stuff_string(subname, F_NAME, NAME_LENGTH);
		if ( optional_string("+period:") ) {
			stuff_int(&period);
			if ( period <= 0 ) {
				Warning(LOCATION, "Substitution '%s' for weapon '%s' requires a period greater than 0. Setting period to 1.", subname, wip->name);
				period = 1;
			}
			if ( optional_string("+offset:") ) {
				stuff_int(&offset);
				if ( offset <= 0 ) {
					Warning(LOCATION, "Period offset for substitution '%s' of weapon '%s' has to be greater than 0. Setting offset to 1.", subname, wip->name);
					offset = 1;
				}
			}
		} else if ( optional_string("+index:") ) {
			stuff_int(&index);
			if ( index < 0 ) {
				Warning(LOCATION, "Substitution '%s' for weapon '%s' requires an index greater than 0. Setting index to 0.", subname, wip->name);
				index = 0;
			}
		}

		// we are going to use weapon substition so, make sure that the pattern array has at least one element
		if ( wip->num_substitution_patterns == 0 ) {
			// pattern is empty, initialize pattern with the weapon being currently parsed.
			strcpy_s(wip->weapon_substitution_pattern_names[0], wip->name);
			wip->num_substitution_patterns++;
		}

		// if tbler specifies a period then determine if we can fit the resulting pattern
		// neatly into the pattern array.
		if ( period > 0 ) {
			if ( (wip->num_substitution_patterns % period) > 0 ) {
				// not neat, need to expand the pattern so that our frequency pattern fits completly.
				size_t current_size = wip->num_substitution_patterns;
				size_t desired_size = current_size*period;
				if (desired_size > MAX_SUBSTITUTION_PATTERNS) {
					Warning(LOCATION, "The period is too large for the number of substitution patterns!  desired size=" SIZE_T_ARG ", max size=%d", desired_size, MAX_SUBSTITUTION_PATTERNS);
				}
				else {
					wip->num_substitution_patterns = desired_size;

					// now duplicate the current pattern into the new area so the current pattern holds
					for ( size_t i = current_size; i < desired_size; i++ ) {
						strcpy_s(wip->weapon_substitution_pattern_names[i], wip->weapon_substitution_pattern_names[i%current_size]);
					}
				}
			}

			/* Apply the substituted weapon at the requested period, barrel
			shifted by offset if needed.*/
			for ( size_t pos = (period + offset - 1) % period;
				pos < wip->num_substitution_patterns; pos += period )
			{
				strcpy_s(wip->weapon_substitution_pattern_names[pos], subname);
			}
		} else {
			// assume that tbler wanted to specify a index for the new weapon.

			// make sure that there is enough room
			if (index >= MAX_SUBSTITUTION_PATTERNS) {
				Warning(LOCATION, "Substitution pattern index exceeds the maximum size!  Index=%d, max size=%d", index, MAX_SUBSTITUTION_PATTERNS);
			} else {
				if ( (size_t)index >= wip->num_substitution_patterns ) {
					// need to make the pattern bigger by filling the extra with the current weapon.
					for ( size_t i = wip->num_substitution_patterns; i < (size_t)index; i++ ) {
						strcpy_s(wip->weapon_substitution_pattern_names[i], subname);
					}
					wip->num_substitution_patterns = index+1;
				}

				strcpy_s(wip->weapon_substitution_pattern_names[index], subname);
			}
		}
	}

	//Optional score for destroying this weapon.
	if (optional_string("$Score:")) {
		stuff_int(&wip->score);
	}

	return WEAPON_INFO_INDEX(wip);
}

/**
 * For all weapons that spawn weapons, given an index at weaponp->spawn_type,
 * convert the strings in Spawn_names to indices in the Weapon_types array.
 */
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

void parse_weaponstbl(const char *filename)
{
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		if (optional_string("#Primary Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if (parse_weapon(WP_LASER, Parsing_modular_table, filename) < 0) {
					continue;
				}
			}
			required_string("#End");
		}

		if (optional_string("#Secondary Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if (parse_weapon(WP_MISSILE, Parsing_modular_table, filename) < 0) {
					continue;
				}
			}
			required_string("#End");
		}

		if (optional_string("#Beam Weapons"))
		{
			while (required_string_either("#End", "$Name:")) {
				// AL 28-3-98: If parse_weapon() fails, try next .tbl weapon
				if (parse_weapon(WP_BEAM, Parsing_modular_table, filename) < 0) {
					continue;
				}
			}
			required_string("#End");
		}

		if (optional_string("#Countermeasures"))
		{
			while (required_string_either("#End", "$Name:"))
			{
				int idx = parse_weapon(WP_MISSILE, Parsing_modular_table, filename);

				if (idx < 0) {
					continue;
				}

				//Make sure cmeasure flag is set
                Weapon_info[idx].wi_flags.set(Weapon::Info_Flags::Cmeasure);

				//Set cmeasure index
				if (!strlen(Default_cmeasure_name)) {
					//We can't be sure that index will be the same after sorting, so save the name
					strcpy_s(Default_cmeasure_name, Weapon_info[idx].name);
				}
			}

			required_string("#End");
		}

		// Read in a list of weapon_info indicies that are an ordering of the player weapon precedence.
		// This list is used to select an alternate weapon when a particular weapon is not available
		// during weapon selection.
		if ((!Parsing_modular_table && required_string("$Player Weapon Precedence:")) || optional_string("$Player Weapon Precedence:"))
		{
			Num_player_weapon_precedence = stuff_int_list(Player_weapon_precedence, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);
		}

		// add tbl/tbm to multiplayer validation list
		fs2netd_add_table_validation(filename);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

//uses a simple bucket sort to sort weapons, order of importance is:
//Lasers
//Beams
//Child primary weapons
//Fighter missiles and bombs
//Capital missiles and bombs
//Child secondary weapons
void weapon_sort_by_type()
{
	weapon_info *lasers = NULL, *big_lasers = NULL, *beams = NULL, *missiles = NULL, *big_missiles = NULL, *child_primaries = NULL, *child_secondaries = NULL;
	int num_lasers = 0, num_big_lasers = 0, num_beams = 0, num_missiles = 0, num_big_missiles = 0, num_child_primaries = 0, num_child_secondaries = 0;
	int i, weapon_index;

	// get the initial count of each weapon type
	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		switch (Weapon_info[i].subtype)
		{
			case WP_UNUSED:
				continue;

			case WP_LASER:
				if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Child])
					num_child_primaries++;
				else if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Big_only])
					num_big_lasers++;
				else
					num_lasers++;
				break;
		
			case WP_BEAM:
				num_beams++;
				break;

			case WP_MISSILE:
				if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Child])
					num_child_secondaries++;
				else if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Big_only])
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

	if (num_child_primaries) {
		child_primaries = new weapon_info[num_child_primaries];
		Verify( child_primaries != NULL );
		num_child_primaries = 0;
	}

	if (num_child_secondaries) {
		child_secondaries = new weapon_info[num_child_secondaries];
		Verify( child_secondaries != NULL );
		num_child_secondaries = 0;
	}

	// fill the buckets
	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		switch (Weapon_info[i].subtype)
		{
			case WP_UNUSED:
				continue;

			case WP_LASER:
				if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Child])
					child_primaries[num_child_primaries++] = Weapon_info[i];
				else if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Big_only])
					big_lasers[num_big_lasers++] = Weapon_info[i];
				else
					lasers[num_lasers++] = Weapon_info[i];
				break;
		
			case WP_BEAM:
				beams[num_beams++] = Weapon_info[i];
				break;

			case WP_MISSILE:
				if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Child])
					child_secondaries[num_child_secondaries++] = Weapon_info[i];
				else if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Big_only])
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

	for (i = 0; i < num_child_primaries; i++, weapon_index++)
		Weapon_info[weapon_index] = child_primaries[i];

	// designate start of secondary weapons so that we'll have the correct offset later on
	First_secondary_index = weapon_index;

	for (i = 0; i < num_missiles; i++, weapon_index++)
		Weapon_info[weapon_index] = missiles[i];

	for (i = 0; i < num_big_missiles; i++, weapon_index++)
		Weapon_info[weapon_index] = big_missiles[i];

	for (i = 0; i < num_child_secondaries; i++, weapon_index++)
		Weapon_info[weapon_index] = child_secondaries[i];


	if (lasers)			delete [] lasers;
	if (big_lasers)		delete [] big_lasers;
	if (beams)			delete [] beams;
	if (missiles)		delete [] missiles;
	if (big_missiles)	delete [] big_missiles;
	if (child_primaries)	delete [] child_primaries;
	if (child_secondaries)	delete [] child_secondaries;
}

/**
 * Do any post-parse cleaning on weapon entries
 */
void weapon_clean_entries()
{
	weapon_info *wip;
	int i;

	for (i = 0; i < Num_weapon_types; i++) {
		wip = &Weapon_info[i];

		if (wip->wi_flags[Weapon::Info_Flags::Beam]) {
			// clean up any beam sections which may have been deleted
			int removed = 0;

			for (int s_idx = 0; s_idx < wip->b_info.beam_num_sections; s_idx++) {
				beam_weapon_section_info *bsip = &wip->b_info.sections[s_idx];

				// If this is an invisible beam section, we want to keep it.  Originally invisible sections were initialized as they were parsed,
				// but then they were inadvertently cleaned up in this function.  So let's properly set the filename here while not removing the section.
				if ( !stricmp(bsip->texture.filename, "invisible") ) {
					memset(bsip->texture.filename, 0, MAX_FILENAME_LEN);
				}
				// Now remove empty beam sections as before
				else if ( !strlen(bsip->texture.filename) ) {
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

			if (wip->b_info.beam_num_sections == 0) {
				Warning(LOCATION, "The beam '%s' has 0 usable sections!", wip->name);
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

		if (wip->wi_flags[Weapon::Info_Flags::Beam]) {
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

		if (wip->wi_flags[Weapon::Info_Flags::Trail]) {
			if (wip->tr_info.texture.bitmap_id >= 0) {
				bm_release(wip->tr_info.texture.bitmap_id);
				wip->tr_info.texture.bitmap_id = -1;
			}
		}

		if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) { // tweaked for multiple particle spews -nuke
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

	if (wip->wi_flags[Weapon::Info_Flags::Beam]) {
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

	if ( (wip->wi_flags[Weapon::Info_Flags::Trail]) && (wip->tr_info.texture.bitmap_id < 0) )
		generic_bitmap_load(&wip->tr_info.texture);

	//WMC - Don't try to load an anim if no anim is specified, Mmkay?
	if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) {
		for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {	// looperfied for multiple pspewers -nuke
			if (wip->particle_spewers[s].particle_spew_type != PSPEW_NONE){

				if ((wip->particle_spewers[s].particle_spew_anim.first_frame < 0) 
					&& (wip->particle_spewers[s].particle_spew_anim.filename[0] != '\0') ) {

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

	decals::loadBitmaps(wip->impact_decal);

	// if this weapon isn't already marked as used, then mark it as such now
	// (this should really only happen if the player is cheating)
	if ( !used_weapons[weapon_index] )
		used_weapons[weapon_index]++;
}

/**
 * Checks all of the weapon infos for substitution patterns and caches the weapon_index of any that it finds. 
 */
void weapon_generate_indexes_for_substitution() {
	for (int i = 0; i < MAX_WEAPON_TYPES; i++) {
		weapon_info *wip = &(Weapon_info[i]);

		if ( wip->num_substitution_patterns > 0 ) {
			for ( size_t j = 0; j < wip->num_substitution_patterns; j++ ) {
				int weapon_index = -1;
				if ( stricmp("none", wip->weapon_substitution_pattern_names[j]) != 0 ) {
					weapon_index = weapon_info_lookup(wip->weapon_substitution_pattern_names[j]);

					if ( weapon_index == -1 ) { // invalid sub weapon
						Warning(LOCATION, "Weapon '%s' requests substitution with '%s' which does not seem to exist",
							wip->name, wip->weapon_substitution_pattern_names[j]);
						continue;
					}

					if (Weapon_info[weapon_index].subtype != wip->subtype) {
						// Check to make sure secondaries can't be launched by primaries and vice versa
						Warning(LOCATION, "Weapon '%s' requests substitution with '%s' which is of a different subtype.",
							wip->name, wip->weapon_substitution_pattern_names[j]);
						wip->num_substitution_patterns = 0;
						std::fill(std::begin(wip->weapon_substitution_pattern),
								  std::end(wip->weapon_substitution_pattern),
								  -1);
						break;
					}
				}

				wip->weapon_substitution_pattern[j] = weapon_index;
			}

			memset(wip->weapon_substitution_pattern_names, 0, sizeof(char) * MAX_SUBSTITUTION_PATTERNS * NAME_LENGTH);
		}

		if (wip->failure_rate > 0.0f) {
			if (VALID_FNAME(wip->failure_sub_name)) {
				wip->failure_sub = weapon_info_lookup(wip->failure_sub_name.c_str());

				if (wip->failure_sub == -1) { // invalid sub weapon
					Warning(LOCATION, "Weapon '%s' requests substitution with '%s' which does not seem to exist",
						wip->name, wip->failure_sub_name.c_str());
					wip->failure_rate = 0.0f;
				}

				if (Weapon_info[wip->failure_sub].subtype != wip->subtype) {
					// Check to make sure secondaries can't be launched by primaries and vice versa
					Warning(LOCATION, "Weapon '%s' requests substitution with '%s' which is of a different subtype.",
						wip->name, wip->failure_sub_name.c_str());
					wip->failure_sub = -1;
					wip->failure_rate = 0.0f;
				}
			}

			wip->failure_sub_name.clear();
		}
	}
}

void weapon_do_post_parse()
{
	weapon_info *wip;
	int first_cmeasure_index = -1;
	int i;

	weapon_sort_by_type();	// NOTE: This has to be first thing!
	weapon_clean_entries();
	weapon_generate_indexes_for_substitution();

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
		if ( (first_cmeasure_index < 0) && (wip->wi_flags[Weapon::Info_Flags::Cmeasure]) )
			first_cmeasure_index = i;
	}

	// catch cmeasure fallback
	if (Default_cmeasure_index < 0)
		Default_cmeasure_index = first_cmeasure_index;

	// now we want to resolve the countermeasures by species
	for (SCP_vector<species_info>::iterator ii = Species_info.begin(); ii != Species_info.end(); ++ii)
	{
		if (*ii->cmeasure_name)
		{
			int index = weapon_info_lookup(ii->cmeasure_name);
			if (index < 0)
				Warning(LOCATION, "Could not find weapon type '%s' to use as countermeasure on species '%s'", ii->cmeasure_name, ii->species_name);
			else if (Weapon_info[index].wi_flags[Weapon::Info_Flags::Beam])
				Warning(LOCATION, "Attempt made to set a beam weapon as a countermeasure on species '%s'", ii->species_name);
			else
				ii->cmeasure_index = index;
		}
	}

	// translate all spawn type weapons to referrnce the appropriate spawned weapon entry
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
	for (auto& wi : Weapon_info) {
		wi.reset();
	}
}

/**
 * This will get called once at game startup
 */
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


/**
 * Call from game_shutdown() only!!
 */
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

/**
 * This will get called at the start of each level.
 */
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

	// clear out used_weapons between missions
	memset(used_weapons, 0, Num_weapon_types * sizeof(int));

	Weapon_flyby_sound_timer = timestamp(0);
	Weapon_impact_timer = 1;	// inited each level, used to reduce impact sounds
}

MONITOR( NumWeaponsRend )

const float weapon_glow_scale_f = 2.3f;
const float weapon_glow_scale_r = 2.3f;
const float weapon_glow_scale_l = 1.5f;
const int weapon_glow_alpha = 217; // (0.85 * 255);

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

	if (wp->trail_ptr != NULL) {
		trail_object_died(wp->trail_ptr);
		wp->trail_ptr = NULL;
	}

	if (wp->hud_in_flight_snd_sig.isValid() && snd_is_playing(wp->hud_in_flight_snd_sig))
		snd_stop(wp->hud_in_flight_snd_sig);

	if (wp->model_instance_num >= 0)
		model_delete_instance(wp->model_instance_num);

	if (wp->cmeasure_ignore_list != nullptr) {
		delete wp->cmeasure_ignore_list;
		wp->cmeasure_ignore_list = nullptr;
	}

	if (wp->collisionInfo != nullptr) {
		delete wp->collisionInfo;
		wp->collisionInfo = nullptr;
	}

	wp->objnum = -1;
	Num_weapons--;
	Assert(Num_weapons >= 0);
}

/**
 * Check if missile is newly locked onto the Player, maybe play a launch warning
 */
void weapon_maybe_play_warning(weapon *wp)
{
	if ( wp->homing_object == Player_obj ) {
		if ( !(wp->weapon_flags[Weapon::Weapon_Flags::Lock_warning_played]) ) {
            wp->weapon_flags.set(Weapon::Weapon_Flags::Lock_warning_played);
			// Use heatlock-warning sound for Heat and Javelin for now
			// Possibly add an additional third sound later
			if ( (Weapon_info[wp->weapon_info_index].wi_flags[Weapon::Info_Flags::Homing_heat]) ||
				 (Weapon_info[wp->weapon_info_index].wi_flags[Weapon::Info_Flags::Homing_javelin]) ) {
				snd_play(gamesnd_get_game_sound(ship_get_sound(Player_obj, GameSounds::HEATLOCK_WARN)));
			} else {
				Assert(Weapon_info[wp->weapon_info_index].wi_flags[Weapon::Info_Flags::Homing_aspect]);
				snd_play(gamesnd_get_game_sound(ship_get_sound(Player_obj, GameSounds::ASPECTLOCK_WARN)));
			}
		}
	}
}


/**
 * Detonate all missiles near this countermeasure.
 */
void detonate_nearby_missiles(object* killer_objp, object* missile_objp)
{
	if(killer_objp->type != OBJ_WEAPON || missile_objp->type != OBJ_WEAPON) {
		Int3();
		return;
	}

	weapon_info* killer_infop = &Weapon_info[Weapons[killer_objp->instance].weapon_info_index];

	if (killer_infop->cm_kill_single) {
		weapon* wp = &Weapons[missile_objp->instance];
		if (wp->lifeleft > 0.2f) {
			nprintf(("Countermeasures", "Countermeasure (%s-%i) detonated missile (%s-%i) Frame: %i\n",
						killer_infop->name, killer_objp->signature,
						Weapon_info[Weapons[missile_objp->instance].weapon_info_index].name, missile_objp->signature, Framecount));
			wp->lifeleft = 0.2f;
		}
		return;
	}

	missile_obj* mop = GET_FIRST(&Missile_obj_list);

	while(mop != END_OF_LIST(&Missile_obj_list)) {
		object* objp = &Objects[mop->objnum];
		weapon* wp = &Weapons[objp->instance];

		if (iff_x_attacks_y(Weapons[killer_objp->instance].team, wp->team)) {
			if ( Missiontime - wp->creation_time > F1_0/2) {
				if (vm_vec_dist_quick(&killer_objp->pos, &objp->pos) < killer_infop->cm_detonation_rad) {
					if (wp->lifeleft > 0.2f) { 
						nprintf(("Countermeasures", "Countermeasure (%s-%i) detonated missile (%s-%i) Frame: %i\n",
									killer_infop->name, killer_objp->signature,
									Weapon_info[Weapons[objp->instance].weapon_info_index].name, objp->signature, Framecount));
						wp->lifeleft = 0.2f;
					}
				}
			}
		}

		mop = mop->next;
	}
}

/**
 * Find an object for weapon #num (object *weapon_objp) to home on due to heat.
 */
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
		if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_WEAPON) && (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Cmeasure])))
		{
			//WMC - Spawn weapons shouldn't go for protected ships
			// ditto for untargeted heat seekers - niffiwan
			if ( (objp->flags[Object::Object_Flags::Protected]) &&
				((wp->weapon_flags[Weapon::Weapon_Flags::Spawned]) || (wip->wi_flags[Weapon::Info_Flags::Untargeted_heat_seeker])) )
				continue;

			// Spawned weapons should never home in on their parent - even in multiplayer dogfights where they would pass the iff test below
			if ((wp->weapon_flags[Weapon::Weapon_Flags::Spawned]) && (objp == &Objects[weapon_objp->parent]))
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
                    if ((wip->wi_flags[Weapon::Info_Flags::Huge]) &&
                        (sip->is_small_ship() || !sip->is_flyable() || sip->is_harmless()))
                    {
                        continue;
                    }

					// AL 2-17-98: If ship is immune to sensors, can't home on it (Sandeep says so)!
					if ( sp->flags[Ship::Ship_Flags::Hidden_from_sensors] ) {
						continue;
					}

					// Goober5000: if missiles can't home on sensor-ghosted ships,
					// they definitely shouldn't home on stealth ships
					if ( sp->flags[Ship::Ship_Flags::Stealth] && (The_mission.ai_profile->flags[AI::Profile_Flags::Fix_heat_seeker_stealth_bug]) ) {
						continue;
					}

                    if (wip->wi_flags[Weapon::Info_Flags::Homing_javelin])
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
                    if (wip->wi_flags[Weapon::Info_Flags::Huge, Weapon::Info_Flags::Homing_javelin])
                        continue;
                    
                    //don't look for local ssms that are gone for the time being
					if (Weapons[objp->instance].lssm_stage == 3)
						continue;
				}

				dist = vm_vec_normalized_dir(&vec_to_object, &objp->pos, &weapon_objp->pos);

				if (objp->type == OBJ_WEAPON && (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Cmeasure])) {
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

/**
 * For all homing weapons, see if they should be decoyed by a countermeasure.
 */
void find_homing_object_cmeasures(const SCP_vector<object*> &cmeasure_list)
{
	for (object *weapon_objp = GET_FIRST(&obj_used_list); weapon_objp != END_OF_LIST(&obj_used_list); weapon_objp = GET_NEXT(weapon_objp) ) {
		if (weapon_objp->type == OBJ_WEAPON) {
			weapon *wp = &Weapons[weapon_objp->instance];
			weapon_info	*wip = &Weapon_info[wp->weapon_info_index];

			if (wip->is_homing()) {
				float best_dot = wip->fov;
				for (auto cit = cmeasure_list.cbegin(); cit != cmeasure_list.cend(); ++cit) {
					//don't have a weapon try to home in on itself
					if (*cit == weapon_objp)
						continue;

					weapon *cm_wp = &Weapons[(*cit)->instance];
					weapon_info *cm_wip = &Weapon_info[cm_wp->weapon_info_index];

					//don't have a weapon try to home in on missiles fired by the same team, unless its the traitor team.
					if ((wp->team == cm_wp->team) && (wp->team != Iff_traitor))
						continue;

					vec3d	vec_to_object;
					float dist = vm_vec_normalized_dir(&vec_to_object, &(*cit)->pos, &weapon_objp->pos);

					if (dist < cm_wip->cm_effective_rad)
					{
						float chance;

						if (wp->cmeasure_ignore_list == nullptr) {
							wp->cmeasure_ignore_list = new SCP_vector<int>;
						}
						else {
							bool found = false;
							for (auto ii = wp->cmeasure_ignore_list->cbegin(); ii != wp->cmeasure_ignore_list->cend(); ++ii) {
								if ((*cit)->signature == *ii) {
									nprintf(("CounterMeasures", "Weapon (%s-%04i) already seen CounterMeasure (%s-%04i) Frame: %i\n",
												wip->name, weapon_objp->instance, cm_wip->name, (*cit)->signature, Framecount));
									found = true;
									break;
								}
							}
							if (found) {
								continue;
							}
						}

						if (wip->wi_flags[Weapon::Info_Flags::Homing_aspect]) {
							// aspect seeker this likely to chase a countermeasure
							chance = cm_wip->cm_aspect_effectiveness/wip->seeker_strength;
						} else {
							// heat seeker and javelin HS this likely to chase a countermeasure
							chance = cm_wip->cm_heat_effectiveness/wip->seeker_strength;
						}

						// remember this cmeasure so it can be ignored in future
						wp->cmeasure_ignore_list->push_back((*cit)->signature);

						if (frand() >= chance) {
							// failed to decoy
							nprintf(("CounterMeasures", "Weapon (%s-%04i) ignoring CounterMeasure (%s-%04i) Frame: %i\n",
										wip->name, weapon_objp->instance, cm_wip->name, (*cit)->signature, Framecount));
						}
						else {
							// successful decoy, maybe chase the new cm
							float dot = vm_vec_dot(&vec_to_object, &weapon_objp->orient.vec.fvec);

							if (dot > best_dot)
							{
								best_dot = dot;
								wp->homing_object = (*cit);
								cmeasure_maybe_alert_success((*cit));
								nprintf(("CounterMeasures", "Weapon (%s-%04i) chasing CounterMeasure (%s-%04i) Frame: %i\n",
											wip->name, weapon_objp->instance, cm_wip->name, (*cit)->signature, Framecount));
							}
						}
					}
				}
			}
		}
	}
}

/**
 * Find object with signature "sig" and make weapon home on it.
 */
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

bool aspect_should_lose_target(weapon* wp)
{
	Assert(wp != NULL);
	
	if (wp->homing_object->signature != wp->target_sig) {
		if (wp->homing_object->type == OBJ_WEAPON)
		{
			weapon_info* target_info = &Weapon_info[Weapons[wp->homing_object->instance].weapon_info_index];

			if (target_info->wi_flags[Weapon::Info_Flags::Cmeasure])
			{
				// Check if we can home on this countermeasure
				bool home_on_cmeasure = The_mission.ai_profile->flags[AI::Profile_Flags::Aspect_lock_countermeasure]
					|| target_info->wi_flags[Weapon::Info_Flags::Cmeasure_aspect_home_on];

				if (!home_on_cmeasure)
				{
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * Make weapon num home.  It's also object *obj.
 */
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

	if ((wip->wi_flags[Weapon::Info_Flags::Local_ssm]) && (wp->lssm_stage==5))
		max_speed=wip->lssm_stage5_vel;
	else
		max_speed=wip->max_speed;

	//	If not [free-flight-time] gone by, don't home yet.
	// Goober5000 - this has been fixed back to more closely follow the original logic.  Remember, the retail code
	// had 0.5 second of free flight time, the first half of which was spent ramping up to full speed.
	if ((hobjp == &obj_used_list) || ( f2fl(Missiontime - wp->creation_time) < (wip->free_flight_time / 2) )) {
		if (f2fl(Missiontime - wp->creation_time) > wip->free_flight_time) {
			// If this is a heat seeking homing missile and [free-flight-time] has elapsed since firing
			// and we don't have a target (else we wouldn't be inside the IF), find a new target.
			if (wip->wi_flags[Weapon::Info_Flags::Homing_heat]) {
				find_homing_object(obj, num);
			}
			// modders may want aspect homing missiles to die if they lose their target
			else if (wip->is_locked_homing() && wip->wi_flags[Weapon::Info_Flags::Die_on_lost_lock]) {
				if (wp->lifeleft > 0.5f) {
					wp->lifeleft = frand_range(0.1f, 0.5f); // randomise a bit to avoid multiple missiles detonating in one frame
				}
				return;
			}
		}
		else if (MULTIPLAYER_MASTER && (wip->is_locked_homing()) && (wp->weapon_flags[Weapon::Weapon_Flags::Homing_update_needed])) {
            wp->weapon_flags.remove(Weapon::Weapon_Flags::Homing_update_needed);
			send_homing_weapon_info(num);
		}

		if (wip->acceleration_time > 0.0f) {
			if (Missiontime - wp->creation_time < fl2f(wip->acceleration_time)) {
				float t;

				t = f2fl(Missiontime - wp->creation_time) / wip->acceleration_time;
				obj->phys_info.speed = wp->launch_speed + MAX(0.0f, wp->weapon_max_vel - wp->launch_speed) * t;
			}
		}
		// since free_flight_time can now be 0, guard against that
		else if (wip->free_flight_time > 0.0f) {
			if (obj->phys_info.speed > max_speed) {
				obj->phys_info.speed -= frame_time * (2 / wip->free_flight_time);
			} else if ((obj->phys_info.speed < max_speed / (2 / wip->free_flight_time)) && (wip->wi_flags[Weapon::Info_Flags::Homing_heat])) {
				obj->phys_info.speed = max_speed / (2 / wip->free_flight_time);
			}
		}
		// no free_flight_time, so immediately set desired speed
		else {
			obj->phys_info.speed = max_speed;
		}

		// set velocity using whatever speed we have
		vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);

		return;
	}

	// if we've got this far, this should be valid
	weapon_info* hobj_infop = &Weapon_info[Weapons[hobjp->instance].weapon_info_index];

	if (wip->acceleration_time > 0.0f) {
		if (Missiontime - wp->creation_time < fl2f(wip->acceleration_time)) {
			float t;

			t = f2fl(Missiontime - wp->creation_time) / wip->acceleration_time;
			obj->phys_info.speed = wp->launch_speed + (wp->weapon_max_vel - wp->launch_speed) * t;
			vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);
		}
	}

	// AL 4-8-98: If original target for aspect lock missile is lost, stop homing
	// WCS - or javelin
	if (wip->is_locked_homing()) {
		if ( wp->target_sig > 0 ) {
			if (aspect_should_lose_target(wp))
			{ 
				wp->homing_object = &obj_used_list;
				return;
			}
		}
	}

  	// AL 4-13-98: Stop homing on a subsystem if parent ship has changed
	if (wip->wi_flags[Weapon::Info_Flags::Homing_heat]) {
		if ( wp->target_sig > 0 ) {
			if ( wp->homing_object->signature != wp->target_sig ) {
				wp->homing_subsys = NULL;
			}
		}
	}

	// If target subsys is dead make missile pick random spot on target as attack point.
	if (wp->homing_subsys != NULL) {
		if (wp->homing_subsys->flags[Ship::Subsystem_Flags::Missiles_ignore_if_dead]) {
			if ((wp->homing_subsys->max_hits > 0) && (wp->homing_subsys->current_hits <= 0)) {
				wp->homing_object = &obj_used_list;
				return;
			}
		}
	}

	// Make sure Javelin HS missiles always home on engine subsystems if ships
	if ((wip->wi_flags[Weapon::Info_Flags::Homing_javelin]) &&
		(hobjp->type == OBJ_SHIP) &&
		(wp->target_sig > 0) &&
		(wp->homing_subsys != NULL) &&
		(wp->homing_subsys->system_info->type != SUBSYSTEM_ENGINE)) {
			int sindex = ship_get_by_signature(wp->target_sig);
			if (sindex >= 0) {
				ship *enemy = &Ships[sindex];
				wp->homing_subsys = ship_get_closest_subsys_in_sight(enemy, SUBSYSTEM_ENGINE, &Objects[wp->objnum].pos);
			}
	}

	// If Javelin HS missile doesn't home in on a subsystem but homing in on a
	// ship, lose lock alltogether
	// Javelins can only home in one Engines or bombs.
	if ((wip->wi_flags[Weapon::Info_Flags::Homing_javelin]) &&
		(hobjp->type == OBJ_SHIP) &&
		(wp->target_sig > 0) &&
		(wp->homing_subsys == NULL)) {
			wp->homing_object = &obj_used_list;
			return;
	}

	// TODO maybe add flag to allow WF_LOCKED_HOMING to lose target when target is dead

	switch (hobjp->type) {
	case OBJ_NONE:
		if (wip->is_locked_homing()) {
			find_homing_object_by_sig(obj, wp->target_sig);
		}
		else {
			find_homing_object(obj, num);
		}
		return;
		break;
	case OBJ_SHIP:
		if (hobjp->signature != wp->target_sig) {
			if (wip->is_locked_homing()) {
				find_homing_object_by_sig(obj, wp->target_sig);
			}
			else {
				find_homing_object(obj, num);
			}
			return;
		}
		break;
	case OBJ_WEAPON:
	{
		bool home_on_cmeasure = The_mission.ai_profile->flags[AI::Profile_Flags::Aspect_lock_countermeasure]
			|| hobj_infop->wi_flags[Weapon::Info_Flags::Cmeasure_aspect_home_on];

		// don't home on countermeasures or non-bombs, that's handled elsewhere
		if (((hobj_infop->wi_flags[Weapon::Info_Flags::Cmeasure]) && !home_on_cmeasure))
		{
			break;
		}
		else if (!(hobj_infop->wi_flags[Weapon::Info_Flags::Bomb]))
		{
			break;
		}

		if (wip->is_locked_homing()) {
			find_homing_object_by_sig(obj, wp->target_sig);
		}
		else {
			find_homing_object(obj, num);
		}
		break;
	}
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
				aip->nearest_locked_object = OBJ_INDEX(obj);
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
		if ( (wp->homing_subsys != NULL) && !(wip->wi_flags[Weapon::Info_Flags::Non_subsys_homing]) && hobjp->type == OBJ_SHIP) {
			get_subsystem_world_pos(hobjp, Weapons[num].homing_subsys, &target_pos);
			wp->homing_pos = target_pos;	// store the homing position in weapon data
			Assert( !vm_is_vec_nan(&wp->homing_pos) );
		} else {
			float	fov;
			float	dist;

			dist = vm_vec_dist_quick(&obj->pos, &hobjp->pos);
			if (hobjp->type == OBJ_WEAPON && (hobj_infop->wi_flags[Weapon::Info_Flags::Cmeasure]))
			{
				if (dist < hobj_infop->cm_detonation_rad)
				{
					//	Make this missile detonate soon.  Not right away, not sure why.  Seems better.
					if (iff_x_attacks_y(Weapons[hobjp->instance].team, wp->team)) {
						detonate_nearby_missiles(hobjp, obj);
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

					if ( pick_homing_point && !(wip->wi_flags[Weapon::Info_Flags::Non_subsys_homing]) ) {
						// If *any* player is parent of homing missile, then use position where lock indicator is
						if ( Objects[obj->parent].flags[Object::Object_Flags::Player_ship] ) {
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
							if (wip->is_locked_homing()) {
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
            (wp->homing_object != &obj_used_list) &&
            (wp->homing_object->type == OBJ_SHIP))
        {
            wp->lifeleft = 0.01f;
        }

		//	Only lead target if more than one second away.  Otherwise can miss target.  I think this
		//	is what's causing Harbingers to miss the super destroyer. -- MK, 4/15/98
		if ((old_dot > 0.1f) && (time_to_target > 0.1f)) {
			if (wip->wi_flags[Weapon::Info_Flags::Variable_lead_homing]) {
				vm_vec_scale_add2(&target_pos, &hobjp->phys_info.vel, (0.33f * wip->target_lead_scaler * MIN(time_to_target, 6.0f)));
			} else if (wip->is_locked_homing()) {
				vm_vec_scale_add2(&target_pos, &hobjp->phys_info.vel, MIN(time_to_target, 2.0f));
			}
		}

		//	If a HEAT seeking (rather than ASPECT seeking) homing missile, verify that target is in viewcone.
		if (wip->wi_flags[Weapon::Info_Flags::Homing_heat]) {
			if ((old_dot < wip->fov) && (dist_to_target > wip->shockwave.inner_rad*1.1f)) {	//	Delay finding new target one frame to allow detonation.
				find_homing_object(obj, num);
				return;			//	Maybe found a new homing object.  Return, process more next frame.
			} else	//	Subtract out life based on how far from target this missile points.
				if ((wip->fov < 0.95f) && !(wip->wi_flags[Weapon::Info_Flags::No_life_lost_if_missed])) {
					wp->lifeleft -= flFrametime * (0.95f - old_dot);
				}
		} else if (wip->is_locked_homing()) {	//	subtract life as if max turn is 90 degrees.
			if ((wip->fov < 0.95f) && !(wip->wi_flags[Weapon::Info_Flags::No_life_lost_if_missed]))
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


		if (wip->acceleration_time > 0.0f) {
			// Ramp up speed linearly for the given duration
			if (Missiontime - wp->creation_time < fl2f(wip->acceleration_time)) {
				float t;

				t = f2fl(Missiontime - wp->creation_time) / wip->acceleration_time;
				obj->phys_info.speed = wp->launch_speed + MAX(0.0f, wp->weapon_max_vel - wp->launch_speed) * t;
			}
		} else if (!(wip->wi_flags[Weapon::Info_Flags::No_homing_speed_ramp]) && Missiontime - wp->creation_time < i2f(1)) {
			// Default behavior:
			// For first second of weapon's life, it doesn't fly at top speed.  It ramps up.
			float	t;

			t = f2fl(Missiontime - wp->creation_time);
			obj->phys_info.speed *= t*t;
		}

		Assert( obj->phys_info.speed > 0.0f );

		vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);

		// turn the missile towards the target only if non-swarm.  Homing swarm missiles choose
		// a different vector to turn towards, this is done in swarm_update_direction().
		if ( wp->swarm_index < 0 ) {
			ai_turn_towards_vector(&target_pos, obj, wip->turn_time, nullptr, nullptr, 0.0f, 0, nullptr);
			vel = vm_vec_mag(&obj->phys_info.desired_vel);

			vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, vel);

		}
	}
}

// as Mike K did with ships -- break weapon into process_pre and process_post for code to execute
// before and after physics movement

void weapon_process_pre( object *obj, float  /*frame_time*/)
{
	if(obj->type != OBJ_WEAPON)
		return;

	weapon *wp = &Weapons[obj->instance];
	weapon_info *wip = &Weapon_info[wp->weapon_info_index];

	// if the object is a corkscrew style weapon, process it now
	if((obj->type == OBJ_WEAPON) && (Weapons[obj->instance].cscrew_index >= 0)){
		cscrew_process_pre(obj);
	}

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
		if((wp->homing_object != &obj_used_list) && (wp->homing_object->type != 0))
		{
			if(!IS_VEC_NULL(&wp->homing_pos) && vm_vec_dist(&wp->homing_pos, &obj->pos) <= wip->det_radius)
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

/**
 * Maybe play a "whizz sound" if close enough to view position
 */
void weapon_maybe_play_flyby_sound(object *weapon_objp, weapon *wp)
{
	// don't play flyby sounds too close together
	if ( !timestamp_elapsed(Weapon_flyby_sound_timer) ) {
		return;
	}

	if ( !(wp->weapon_flags[Weapon::Weapon_Flags::Played_flyby_sound]) ) {
		float		dist, dot, radius;

		if ( (Weapon_info[wp->weapon_info_index].wi_flags[Weapon::Info_Flags::Corkscrew]) ) {
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
			
			if ( (dot < -0.80) && (dot > -0.98) ) {
				if(Weapon_info[wp->weapon_info_index].flyby_snd.isValid()) {
					snd_play_3d( gamesnd_get_game_sound(Weapon_info[wp->weapon_info_index].flyby_snd), &weapon_objp->pos, &Eye_position );
				} else {
					if ( Weapon_info[wp->weapon_info_index].subtype == WP_LASER ) {
						snd_play_3d( gamesnd_get_game_sound(GameSounds::WEAPON_FLYBY), &weapon_objp->pos, &Eye_position );
					}
				}
				Weapon_flyby_sound_timer = timestamp(200);
                wp->weapon_flags.set(Weapon::Weapon_Flags::Played_flyby_sound);
			}
		}
	}
}

static void weapon_set_state(weapon_info* wip, weapon* wp, WeaponState state)
{
	if (wp->weapon_state == state)
	{
		// No change
		return;
	}

	auto current_state = wp->weapon_state;

	wp->weapon_state = state;

	if (current_state == WeaponState::INVALID)
	{
		// First weapon state, create the in-flight effect
		auto map_entry = wip->state_effects.find(WeaponState::NORMAL);

		if (map_entry != wip->state_effects.end())
		{
			auto source = particle::ParticleManager::get()->createSource(map_entry->second);

			source.moveToObject(&Objects[wp->objnum], &vmd_zero_vector);
			source.setWeaponState(WeaponState::NORMAL);

			source.finish();
		}
	}

	auto map_entry = wip->state_effects.find(wp->weapon_state);

	if (map_entry != wip->state_effects.end())
	{
		auto source = particle::ParticleManager::get()->createSource(map_entry->second);

		source.moveToObject(&Objects[wp->objnum], &vmd_zero_vector);
		source.setWeaponState(wp->weapon_state);

		source.finish();
	}
}

static void weapon_update_state(weapon* wp)
{
	weapon_info* wip = &Weapon_info[wp->weapon_info_index];

	if (wip->subtype == WP_LASER)
	{
		weapon_set_state(wip, wp, WeaponState::NORMAL);
		return;
	}

	auto infree_flight = false;
	if (wip->free_flight_time)
	{
		fix lifetime = Missiontime - wp->creation_time;
		if (lifetime < fl2f(wip->free_flight_time))
		{
			weapon_set_state(wip, wp, WeaponState::FREEFLIGHT);
			infree_flight = true;
		}
		else if (lifetime >= fl2f(wip->free_flight_time) &&
			(lifetime - Frametime) <= fl2f(wip->free_flight_time) && wp->homing_object != nullptr)
		{
			weapon_set_state(wip, wp, WeaponState::IGNITION);
			infree_flight = true;
		}
	}

	if (!infree_flight)
	{
		if (wp->homing_object == nullptr)
		{
			weapon_set_state(wip, wp, WeaponState::UNHOMED_FLIGHT);
		}
		else
		{
			weapon_set_state(wip, wp, WeaponState::HOMED_FLIGHT);
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

	
	if (wip->wi_flags[Weapon::Info_Flags::Local_ssm])
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
				if ( !MULTIPLAYER_CLIENT || (MULTIPLAYER_CLIENT && (wp->lifeleft < -2.0f)) || (MULTIPLAYER_CLIENT && (wip->wi_flags[Weapon::Info_Flags::Child]))) {					// don't call this function multiplayer client -- host will send this packet to us
					weapon_detonate(obj);					
				}
			} else {
				weapon_detonate(obj);									
			}
			if (wip->is_homing()) {
				Homing_misses++;
			}
		} else {
            obj->flags.set(Object::Object_Flags::Should_be_dead);
		}

		return;
	}

	// plot homing missiles on the radar
	if (((wip->wi_flags[Weapon::Info_Flags::Bomb]) || (wip->wi_flags[Weapon::Info_Flags::Shown_on_radar])) && !(wip->wi_flags[Weapon::Info_Flags::Dont_show_on_radar])) {
		if ( hud_gauge_active(HUD_RADAR) ) {
			radar_plot_object( obj );
		}
	}

	// trail missiles
	if ((wip->wi_flags[Weapon::Info_Flags::Trail]) && !(wip->wi_flags[Weapon::Info_Flags::Corkscrew])) {
		if ( (wp->trail_ptr != NULL ) && (wp->lssm_stage!=3))	{
			vec3d pos;
			
			if (wip->render_type == WRT_LASER) {
				// place tail origin in center of the bolt
				vm_vec_scale_add(&pos, &obj->pos, &obj->orient.vec.fvec, (wip->laser_length / 2));
			} else {
				pos = obj->pos;
			}

			if (trail_stamp_elapsed(wp->trail_ptr)) {

				trail_add_segment( wp->trail_ptr, &pos );
				
				trail_set_stamp(wp->trail_ptr);
			} else {
				trail_set_segment( wp->trail_ptr, &pos );
			}

		}
	}

	if ( wip->wi_flags[Weapon::Info_Flags::Thruster] )	{
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

				parent_aip = NULL;
				if (obj->parent != Player_obj-Objects) {
					parent_aip = &Ai_info[Ships[Objects[obj->parent].instance].ai_index];
				}

				vm_vec_normalized_dir(&tvec, &v0, &Objects[wp->target_num].pos);
				dot = vm_vec_dot(&tvec, &Objects[wp->target_num].orient.vec.fvec);
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

	if(wip->wi_flags[Weapon::Info_Flags::Particle_spew]){
		weapon_maybe_spew_particle(obj);
	}

	// a single player or multiplayer server function -- it affects actual weapon movement.
	if (wip->is_homing() && !(wp->weapon_flags[Weapon::Weapon_Flags::No_homing])) {
		weapon_home(obj, num, frame_time);
		
		// If this is a swarm type missile,  
		if ( wp->swarm_index >= 0 ) {
			swarm_update_direction(obj);
		}

		if( wp->cscrew_index >= 0) {
			cscrew_process_post(obj);			
		}
	} else if (wip->acceleration_time > 0.0f) {
		if (Missiontime - wp->creation_time < fl2f(wip->acceleration_time)) {
			float t;

			t = f2fl(Missiontime - wp->creation_time) / wip->acceleration_time;
			obj->phys_info.speed = wp->launch_speed + MAX(0.0f, wp->weapon_max_vel - wp->launch_speed) * t;
		} else {
			obj->phys_info.speed = wip->max_speed;
			obj->phys_info.flags |= PF_CONST_VEL; // Max speed reached, can use simpler physics calculations now
		}

		vm_vec_copy_scale( &obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.speed);
	}

	//local ssm stuff
	if (wip->wi_flags[Weapon::Info_Flags::Local_ssm])
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

			if (wp->lssm_warp_idx < 0) {
				mprintf(("LSSM: Failed to create warp effect! Please report if this happens frequently.\n"));
				// Abort warping
				wp->lssm_stage = 0;
			} else {
				wp->lssm_stage = 2;
			}
		}

		//its just entered subspace subspace. don't collide or render
		if ((wp->lssm_stage==2) && (fireball_lifeleft_percent(&Objects[wp->lssm_warp_idx]) <= wp->lssm_warp_pct))
		{
            auto flags = obj->flags;
            flags.remove(Object::Object_Flags::Renders);
            flags.remove(Object::Object_Flags::Collides);

			obj_set_flags(obj, flags);
		
			//get the position of the target, and estimate its position when it warps out
			//so we have an idea of where it will be.
			if (wp->target_num >= 0)
			{
				vm_vec_scale_add(&wp->lssm_target_pos, &Objects[wp->target_num].pos, &Objects[wp->target_num].phys_info.vel, (float)wip->lssm_warpin_delay / 1000.0f);
			}
			else
			{
				// Our target is invalid, just jump to our position
				wp->lssm_target_pos = obj->pos;
			}

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
			
			if (wp->lssm_warp_idx < 0) {
				mprintf(("LSSM: Failed to create warp effect! Please report if this happens frequently.\n"));
			}

			obj->orient=orient;
			obj->pos=warpin;
			obj->phys_info.speed=0;
			obj->phys_info.desired_vel = vmd_zero_vector;
			obj->phys_info.vel = obj->phys_info.desired_vel;

			wp->lssm_stage = 4;
		}

		//done warping in.  render and collide it. let the fun begin
		// If the previous fireball creation failed just put it into normal space now
		if ((wp->lssm_stage==4) && (wp->lssm_warp_idx < 0 || fireball_lifeleft_percent(&Objects[wp->lssm_warp_idx]) <=0.5f))
		{
			vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, wip->lssm_stage5_vel );
			obj->phys_info.vel = obj->phys_info.desired_vel;
			obj->phys_info.speed = vm_vec_mag(&obj->phys_info.desired_vel);

			wp->lssm_stage=5;

            auto flags = obj->flags;
            flags.set(Object::Object_Flags::Renders);
            flags.set(Object::Object_Flags::Collides);

			obj_set_flags(obj,flags);
		}
	}

	if (wip->hud_in_flight_snd.isValid() && obj->parent_sig == Player_obj->signature)
	{
		bool play_sound = false;
		switch (wip->in_flight_play_type)
		{
		case TARGETED:
			play_sound = wp->homing_object != &obj_used_list;
			break;
		case UNTARGETED:
			play_sound = wp->homing_object == &obj_used_list;
			break;
		case ALWAYS:
			play_sound = true;
			break;
		default:
			Error(LOCATION, "Unknown in-flight sound status %d!", (int) wip->in_flight_play_type);
			break;
		}

		if (play_sound)
		{
			if (!wp->hud_in_flight_snd_sig.isValid() || !snd_is_playing(wp->hud_in_flight_snd_sig)) {
				wp->hud_in_flight_snd_sig = snd_play_looping(gamesnd_get_game_sound(wip->hud_in_flight_snd));
			}
		}
	}

	weapon_update_state(wp);
}

/**
 * Update weapon tracking information.
 */
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

	if (wp->weapon_flags[Weapon::Weapon_Flags::No_homing]) {
		return;
	}

	if (parent_objnum >= 0) {
		parent_objp = &Objects[parent_objnum];
		Assert(parent_objp->type == OBJ_SHIP);
	} else {
		parent_objp = NULL;
	}

	if (parent_objp != NULL && (Ships[parent_objp->instance].flags[Ship::Ship_Flags::No_secondary_lockon])) {
        wp->weapon_flags.set(Weapon::Weapon_Flags::No_homing);
		wp->homing_object = &obj_used_list;
		wp->homing_subsys = NULL;
		wp->target_num = -1;
		wp->target_sig = -1;

		return;
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

			// Goober5000 - if we're going bonkers, pretend we're not targeting our own team
			ai_info *parent_aip = &Ai_info[Ships[parent_objp->instance].ai_index];
			if (parent_aip->active_goal != AI_GOAL_NONE && parent_aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC) {
				if (parent_aip->goals[parent_aip->active_goal].flags[AI::Goal_Flags::Target_own_team]) {
					targeting_same = 0;
				}
			}
		} else {
			targeting_same = 0;
		}

		if ((target_objnum != -1) && (!targeting_same || (MULTI_DOGFIGHT && (target_team == Iff_traitor))) ) {
			wp->target_num = target_objnum;
			wp->target_sig = Objects[target_objnum].signature;
			wp->nearest_dist = 99999.0f;
			if ( (wip->wi_flags[Weapon::Info_Flags::Homing_aspect]) && target_is_locked) {
				wp->homing_object = &Objects[target_objnum];
				wp->homing_subsys = target_subsys;
				weapon_maybe_play_warning(wp);
			} else if ( (wip->wi_flags[Weapon::Info_Flags::Homing_javelin]) && target_is_locked) {
				if ((Objects[target_objnum].type == OBJ_SHIP) &&
					( (wp->homing_subsys == NULL) ||
					  (wp->homing_subsys->system_info->type != SUBSYSTEM_ENGINE) )) {
						ship *target_ship = &Ships[Objects[target_objnum].instance];
						wp->homing_subsys = ship_get_closest_subsys_in_sight(target_ship, SUBSYSTEM_ENGINE, &Objects[weapon_objnum].pos);
						if (wp->homing_subsys == NULL) {
							wp->homing_object = &obj_used_list;
						} else {
							Assert(wp->homing_subsys->parent_objnum == target_objnum);
							wp->homing_object = &Objects[target_objnum];
							weapon_maybe_play_warning(wp);
						}
				} else {
					wp->homing_object = &Objects[target_objnum];
					wp->homing_subsys = target_subsys;
					weapon_maybe_play_warning(wp);
				}
			} else if ( wip->wi_flags[Weapon::Info_Flags::Homing_heat] ) {
				//	Make a heat seeking missile try to home.  If the target is outside the view cone, it will
				//	immediately drop it and try to find one in its view cone.
				if ((target_objnum != -1) && !(wip->wi_flags[Weapon::Info_Flags::Untargeted_heat_seeker])) {
					wp->homing_object = &Objects[target_objnum];
					wp->homing_subsys = target_subsys;
					weapon_maybe_play_warning(wp);
				} else {
					wp->homing_object = &obj_used_list;
					wp->homing_subsys = NULL;
				}
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
		if (target_is_locked && (wp->target_num != -1) && (wip->is_locked_homing()) ) {
			wp->lifeleft *= LOCKED_HOMING_EXTENDED_LIFE_FACTOR;
			if (MULTIPLAYER_MASTER) {
                wp->weapon_flags.set(Weapon::Weapon_Flags::Homing_update_needed);
			}
		}

		ai_update_danger_weapon(target_objnum, weapon_objnum);		
	}
}

size_t* get_pointer_to_weapon_fire_pattern_index(int weapon_type, ship* shipp, ship_subsys * src_turret)
{
	Assert( shipp != NULL );
	ship_weapon* ship_weapon_p = &(shipp->weapons);
	if(src_turret)
	{
		ship_weapon_p = &src_turret->weapons;
	}
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

/**
 * Create a weapon object
 *
 * @return Index of weapon in the Objects[] array, -1 if the weapon object was not created
 */
int Weapons_created = 0;
int weapon_create( vec3d * pos, matrix * porient, int weapon_type, int parent_objnum, int group_id, int is_locked, int is_spawned, float fof_cooldown, ship_subsys * src_turret)
{
	int			n, objnum;
	int num_deleted;
	object		*objp, *parent_objp=NULL;
	weapon		*wp;
	weapon_info	*wip;

	Assert(weapon_type >= 0 && weapon_type < Num_weapon_types);

	wip = &Weapon_info[weapon_type];

	// beam weapons should never come through here!
	if(wip->wi_flags[Weapon::Info_Flags::Beam])
	{
		Warning(LOCATION, "An attempt to fire a beam ('%s') through weapon_create() was made.\n", wip->name);
		return -1;
	}

	parent_objp = NULL;
	if(parent_objnum >= 0){
		parent_objp = &Objects[parent_objnum];
	}

	if ( (wip->num_substitution_patterns > 0) && (parent_objp != NULL)) {
		// using substitution

		// get to the instance of the gun
		Assertion( parent_objp->type == OBJ_SHIP, "Expected type OBJ_SHIP, got %d", parent_objp->type );
		Assertion( (parent_objp->instance < MAX_SHIPS) && (parent_objp->instance >= 0),
			"Ship index is %d, which is out of range [%d,%d)", parent_objp->instance, 0, MAX_SHIPS);
		ship* parent_shipp = &(Ships[parent_objp->instance]);
		Assert( parent_shipp != NULL );

		size_t *position = get_pointer_to_weapon_fire_pattern_index(weapon_type, parent_shipp, src_turret);
		Assertion( position != NULL, "'%s' is trying to fire a weapon that is not selected", Ships[parent_objp->instance].ship_name );

		size_t curr_pos = *position;
		if (((Weapon_info[weapon_type].subtype == WP_LASER && Player_ship->flags[Ship::Ship_Flags::Primary_linked]) || 
			 (Weapon_info[weapon_type].subtype == WP_MISSILE && Player_ship->flags[Ship::Ship_Flags::Secondary_dual_fire])) && 
			 (curr_pos > 0)) {
			curr_pos--;
		}
		++(*position);
		*position = (*position) % wip->num_substitution_patterns;

		if ( wip->weapon_substitution_pattern[curr_pos] == -1 ) {
			// weapon doesn't want any sub
			return -1;
		} else if ( wip->weapon_substitution_pattern[curr_pos] != weapon_type ) {
			// weapon wants to sub with weapon other than me
			return weapon_create(pos, porient, wip->weapon_substitution_pattern[curr_pos], parent_objnum, group_id, is_locked, is_spawned, fof_cooldown);
		}
	}

	// Let's setup a fast failure check with a uniform distribution.
	if (wip->failure_rate > 0.0f) {
		util::UniformFloatRange rng(0.0f, 1.0f);
		float test = rng.next();
		if (test < wip->failure_rate) {
			if (wip->failure_sub != -1) {
				return weapon_create(pos, porient, wip->failure_sub, parent_objnum, group_id, is_locked, is_spawned, fof_cooldown);
			} else {
				return -1;
			}
		}
	}

	num_deleted = 0;
	if (Num_weapons >= MAX_WEAPONS-5) {

		//No, do remove for AI ships -- MK, 3/12/98  // don't need to try and delete weapons for ai ships
		//if ( !(Objects[parent_objnum].flags[Object::Object_Flags::Player_ship]) )
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

	float combined_fof = wip->field_of_fire;
	// If there is a fof_cooldown value, increase the spread linearly
	if (fof_cooldown != 0.0f) {
		combined_fof = wip->field_of_fire + (fof_cooldown * wip->max_fof_spread);
	}

	if(combined_fof > 0.0f){
		vec3d f;
		vm_vec_random_cone(&f, &orient->vec.fvec, combined_fof);
		vm_vec_normalize(&f);
		vm_vector_2_matrix( orient, &f, NULL, NULL);
	}

	Weapons_created++;
    flagset<Object::Object_Flags> default_flags;
    default_flags.set(Object::Object_Flags::Renders);
    default_flags.set(Object::Object_Flags::Collides);
    default_flags.set(Object::Object_Flags::Physics);

	objnum = obj_create( OBJ_WEAPON, parent_objnum, n, orient, pos, 2.0f, default_flags);
	Assert(objnum >= 0);
	objp = &Objects[objnum];

	// Create laser n!
	wp = &Weapons[n];

	// check if laser or dumbfire missile
	// set physics flag to allow optimization
	if ((wip->subtype == WP_LASER) || ((wip->subtype == WP_MISSILE) && !(wip->is_homing()) && wip->acceleration_time == 0.0f)) {
		// set physics flag
		objp->phys_info.flags |= PF_CONST_VEL;
	}

	wp->start_pos = *pos;
	wp->objnum = objnum;
	wp->model_instance_num = -1;
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
	wp->weapon_flags.reset();
	wp->target_sig = -1;
	wp->cmeasure_ignore_list = nullptr;
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

	// init the weapon state
	wp->weapon_state = WeaponState::INVALID;

	if ( wip->wi_flags[Weapon::Info_Flags::Swarm] ) {
		wp->swarm_index = (short)swarm_create();
	} else {
		wp->swarm_index = -1;
	}		

	// if this is a particle spewing weapon, setup some stuff
	if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) {
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
			if ( wip->wi_flags[Weapon::Info_Flags::Spawn] ){
                multi_set_network_signature( (ushort)(Objects[objnum].net_signature + wip->total_children_spawned), MULTI_SIG_NON_PERMANENT );
			}
		} else {
			Objects[objnum].net_signature = multi_assign_network_signature( MULTI_SIG_NON_PERMANENT );
		}
		// for multiplayer clients, when creating lasers, add some more life to the lasers.  This helps
		// to overcome some problems associated with lasers dying on client machine before they get message
		// from server saying it hit something.
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
		wp->lifeleft = ((rand_val) * (wip->life_max - wip->life_min)) + wip->life_min;
		if((wip->wi_flags[Weapon::Info_Flags::Cmeasure]) && (parent_objp != NULL) && (parent_objp->flags[Object::Object_Flags::Player_ship])) {
			wp->lifeleft *= The_mission.ai_profile->cmeasure_life_scale[Game_skill_level];
		}
	}

	if(wip->wi_flags[Weapon::Info_Flags::Cmeasure]) {
		// For the next two frames, any non-timer-based countermeasures will pulse each frame.
		Cmeasures_homing_check = 2;
		if (wip->cmeasure_timer_interval > 0) {
			// Timer-based countermeasures spawn pulsing as well.
			wp->cmeasure_timer = timestamp();	// Could also use timestamp(0), but it doesn't really matter either way.
		}
	}

	//	Make remote detonate missiles look like they're getting detonated by firer simply by giving them variable lifetimes.
	if (parent_objp != NULL && !(parent_objp->flags[Object::Object_Flags::Player_ship]) && (wip->wi_flags[Weapon::Info_Flags::Remote])) {
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
		// this should have been checked above, but let's be extra sure
		Assert(wip->model_num >= 0);

		objp->radius = model_get_radius(wip->model_num);

		// if we intrinsic-rotate, make sure we have a model instance
		if (model_get(wip->model_num)->flags & PM_FLAG_HAS_INTRINSIC_ROTATE) {
			wp->model_instance_num = model_create_instance(false, wip->model_num);
		}
	} else if ( wip->render_type == WRT_LASER ) {
		objp->radius = wip->laser_head_radius;
	}

	//	Set desired velocity and initial velocity.
	//	For lasers, velocity is always the same.
	//	For missiles, it is a small amount plus the firing ship's velocity.
	//	For missiles, the velocity trends towards some goal.
	//	Note: If you change how speed works here, such as adding in speed of parent ship, you'll need to change the AI code
	//	that predicts collision points.  See Mike Kulas or Dave Andsager.  (Or see ai_get_weapon_speed().)
	if (wip->acceleration_time > 0.0f) {
		vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, 0.01f ); // Tiny initial velocity to avoid possible null vec issues
		objp->phys_info.vel = objp->phys_info.desired_vel;
		objp->phys_info.speed = 0.0f;
		wp->launch_speed = 0.0f;
	} else if (!(wip->is_homing())) {
		vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );
		objp->phys_info.vel = objp->phys_info.desired_vel;
		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.desired_vel);
	} else {		
		//	For weapons that home, set velocity to sum of forward component of parent's velocity and 1/4 weapon's max speed.
		//	Note that it is important to extract the forward component of the parent's velocity to factor out sliding, else
		//	the missile will not be moving forward.
		if(parent_objp != NULL){
			if (wip->free_flight_time > 0.0)
				vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, vm_vec_dot(&parent_objp->phys_info.vel, &parent_objp->orient.vec.fvec) + objp->phys_info.max_vel.xyz.z/4 );
			else
				vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );
		} else {
			if (!is_locked && wip->free_flight_time > 0.0)
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
	if ((parent_objp != NULL) && (The_mission.ai_profile->flags[AI::Profile_Flags::Use_additive_weapon_velocity])) {
		float pspeed = vm_vec_mag( &parent_objp->phys_info.vel );
		vm_vec_scale_add2( &objp->phys_info.vel, &parent_objp->phys_info.vel, wip->vel_inherit_amount );
		wp->weapon_max_vel += pspeed * wip->vel_inherit_amount;
		objp->phys_info.speed = vm_vec_mag(&objp->phys_info.vel);

		if (wip->acceleration_time > 0.0f)
			wp->launch_speed += pspeed;
	}

	// create the corkscrew
	if ( wip->wi_flags[Weapon::Info_Flags::Corkscrew] ) {
		wp->cscrew_index = (short)cscrew_create(objp);
	} else {
		wp->cscrew_index = -1;
	}

	if (wip->wi_flags[Weapon::Info_Flags::Local_ssm])
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
    if ( (wip->wi_flags[Weapon::Info_Flags::Flak]) && !(wip->wi_flags[Weapon::Info_Flags::Render_flak]) ) {
		obj_set_flags(&Objects[wp->objnum], Objects[wp->objnum].flags - Object::Object_Flags::Renders);
	}

	wp->missile_list_index = -1;
	// If this is a missile, then add it to the Missile_obj_list
	if ( wip->subtype == WP_MISSILE ) {
		wp->missile_list_index = missile_obj_list_add(objnum);
	}

	if (wip->wi_flags[Weapon::Info_Flags::Trail] /*&& !(wip->wi_flags[Weapon::Info_Flags::Corkscrew]) */) {
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
	if ( parent_objp != NULL && parent_objp == Player_obj ) {
        wp->weapon_flags.set(Weapon::Weapon_Flags::Played_flyby_sound);
	}

	wp->pick_big_attack_point_timestamp = timestamp(1);

	//	Set detail levels for POF-type weapons.
	if (Weapon_info[wp->weapon_info_index].model_num != -1) {
		polymodel * pm;
		int	i;
		pm = model_get(Weapon_info[wp->weapon_info_index].model_num);

		for (i=0; i<pm->n_detail_levels; i++){
			// for weapons, detail levels are all preset to -1
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
            wip->wi_flags.set(Weapon::Info_Flags::Thruster);
		}
#endif
	}

		// if the weapon was fired locked
	if(is_locked){
        wp->weapon_flags.set(Weapon::Weapon_Flags::Locked_when_fired);
	}

	//if the weapon was spawned from a spawning type weapon
	if(is_spawned){
        wp->weapon_flags.set(Weapon::Weapon_Flags::Spawned);
	}

	wp->alpha_current = -1.0f;
	wp->alpha_backward = 0;

	wp->collisionInfo = nullptr;
	wp->hud_in_flight_snd_sig = sound_handle::invalid();

	Num_weapons++;

	if (Weapons_inherit_parent_collision_group) {
		Objects[objnum].collision_group_id = Objects[parent_objnum].collision_group_id;
	}

	weapon_update_state(wp);

	object* weapon_obj = &Objects[objnum];
	Script_system.SetHookObjects(1, "Weapon", weapon_obj);
	Script_system.RunCondition(CHA_ONWEAPONCREATED, weapon_obj);
	Script_system.RemHookVars(1, "Weapon");

	return objnum;
}

/**
 * Spawn child weapons from object *objp.
 */
void spawn_child_weapons(object *objp)
{
	int	i, j;
	int	child_id;
	int	parent_num;
	ushort starting_sig;
	weapon	*wp = NULL;
	beam	*bp = NULL;
	weapon_info	*wip, *child_wip;
	vec3d	*opos, *fvec;

	Assertion(objp->type == OBJ_WEAPON || objp->type == OBJ_BEAM, "spawn_child_weapons() doesn't make sense for non-weapon non-beam objects; get a coder!\n");
	Assertion(objp->instance >= 0, "spawn_child_weapons() called with an object with an instance of %d; get a coder!\n", objp->instance);
	Assertion(!(objp->type == OBJ_WEAPON) || (objp->instance < MAX_WEAPONS), "spawn_child_weapons() called with a weapon with an instance of %d while MAX_WEAPONS is %d; get a coder!\n", objp->instance, MAX_WEAPONS);
	Assertion(!(objp->type == OBJ_BEAM) || (objp->instance < MAX_BEAMS), "spawn_child_weapons() called with a beam with an instance of %d while MAX_BEAMS is %d; get a coder!\n", objp->instance, MAX_BEAMS);

	if (objp->type == OBJ_WEAPON) {
		wp = &Weapons[objp->instance];
		Assertion((wp->weapon_info_index >= 0) && (wp->weapon_info_index < Num_weapon_types), "Invalid weapon_info_index of %d; get a coder!\n", wp->weapon_info_index);
		wip = &Weapon_info[wp->weapon_info_index];
	} else if (objp->type == OBJ_BEAM) {
		bp = &Beams[objp->instance];
		Assertion((bp->weapon_info_index >= 0) && (bp->weapon_info_index < Num_weapon_types), "Invalid weapon_info_index of %d; get a coder!\n", bp->weapon_info_index);
		wip = &Weapon_info[bp->weapon_info_index];
	} else {	// Let's make sure we don't do screwball things in a release build if this gets called with a non-weapon non-beam.
		return;
	}

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

	opos = &objp->pos;
	fvec = &objp->orient.vec.fvec;

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
				static_rand_cone(objp->net_signature + j, &tvec, fvec, wip->spawn_info[i].spawn_angle);
			} else {
				vm_vec_random_cone(&tvec, fvec, wip->spawn_info[i].spawn_angle);
			}
			vm_vec_scale_add(&pos, opos, &tvec, objp->radius);

			// Let's allow beam-spawn! -MageKing17
			if (child_wip->wi_flags[Weapon::Info_Flags::Beam]) {
				beam_fire_info fire_info;
				memset(&fire_info, 0, sizeof(beam_fire_info));

				fire_info.accuracy = 0.000001f;		// this will guarantee a hit
				fire_info.shooter = &Objects[parent_num];
				fire_info.turret = NULL;
				fire_info.target = NULL;
				fire_info.target_subsys = NULL;
				fire_info.target_pos1 = fire_info.target_pos2 = pos;
				fire_info.bfi_flags |= BFIF_FLOATING_BEAM | BFIF_TARGETING_COORDS;
				fire_info.starting_pos = *opos;
				fire_info.beam_info_index = child_id;
				fire_info.team = static_cast<char>(obj_team(&Objects[parent_num]));

				// fire the beam
				beam_fire(&fire_info);
			} else {
				vm_vector_2_matrix(&orient, &tvec, NULL, NULL);
				weapon_objnum = weapon_create(&pos, &orient, child_id, parent_num, -1, wp->weapon_flags[Weapon::Weapon_Flags::Locked_when_fired], 1);

				//if the child inherits parent target, do it only if the parent weapon was locked to begin with
				if ((child_wip->wi_flags[Weapon::Info_Flags::Inherit_parent_target]) && (wp->homing_object != &obj_used_list))
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

	}

	// in multiplayer, reset the next network signature to the one that was saved.
	if ( Game_mode & GM_MULTIPLAYER ){
		multi_set_network_signature( starting_sig, MULTI_SIG_NON_PERMANENT );
	}
}

/**
 * Figures out whether to play disarmed or armed hit sound, checks that the
 * chosen one exists, and plays it
 */
void weapon_play_impact_sound(weapon_info *wip, vec3d *hitpos, bool is_armed)
{
	if(is_armed)
	{
		if(wip->impact_snd.isValid()) {
			snd_play_3d( gamesnd_get_game_sound(wip->impact_snd), hitpos, &Eye_position );
		}
	}
	else
	{
		if(wip->disarmed_impact_snd.isValid()) {
			snd_play_3d(gamesnd_get_game_sound(wip->disarmed_impact_snd), hitpos, &Eye_position);
		}
	}
}

/**
 * Play a sound effect when a weapon hits a ship
 *
 * To elimate the "stereo" effect of two lasers hitting at nearly
 * the same time, and to reduce the number of sound channels used,
 * only play one impact sound if IMPACT_SOUND_DELTA has elapsed
 *
 * @note Uses Weapon_impact_timer global for timer variable
 */
void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vec3d *hitpos, bool is_armed, int quadrant)
{
	float shield_str;

	// If non-missiles (namely lasers) expire without hitting a ship, don't play impact sound
	if	( wip->subtype != WP_MISSILE ) {		
		if ( !hit_obj ) {
			// flak weapons make sounds		
			if(wip->wi_flags[Weapon::Info_Flags::Flak])
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

		if ( hit_obj->type == OBJ_SHIP && quadrant >= 0 ) {
			shield_str = ship_quadrant_shield_strength(hit_obj, quadrant);
		} else {
			shield_str = 0.0f;
		}

		// play a shield hit if shields are above 10% max in this quadrant
		if ( shield_str > 0.1f ) {
			// Play a shield impact sound effect
			if ( hit_obj == Player_obj ) {
				snd_play_3d( gamesnd_get_game_sound(GameSounds::SHIELD_HIT_YOU), hitpos, &Eye_position );
				// AL 12-15-97: Add missile impact sound even when shield is hit
				if ( wip->subtype == WP_MISSILE ) {
					snd_play_3d( gamesnd_get_game_sound(GameSounds::PLAYER_HIT_MISSILE), hitpos, &Eye_position);
				}
			} else {
				snd_play_3d( gamesnd_get_game_sound(GameSounds::SHIELD_HIT), hitpos, &Eye_position );
			}
		} else {
			// Play a hull impact sound effect
			switch ( wip->subtype ) {
				case WP_LASER:
					if ( hit_obj == Player_obj )
						snd_play_3d( gamesnd_get_game_sound(GameSounds::PLAYER_HIT_LASER), hitpos, &Eye_position );
					else {
						weapon_play_impact_sound(wip, hitpos, is_armed);
					}
					break;
				case WP_MISSILE:
					if ( hit_obj == Player_obj ) 
						snd_play_3d( gamesnd_get_game_sound(GameSounds::PLAYER_HIT_MISSILE), hitpos, &Eye_position);
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

extern bool turret_weapon_has_flags(ship_weapon *swp, Weapon::Info_Flags flags);

/**
 * Distrupt any subsystems that fall into damage sphere of this Electronics missile
 *
 * @param ship_objp	Pointer to ship that holds subsystem
 * @param blast_pos	World pos of weapon blast
 * @param wi_index	Weapon info index of weapon causing blast
 */
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
					if ((psub->type==SUBSYSTEM_TURRET) && turret_weapon_has_flags(&ss->weapons, Weapon::Info_Flags::Beam))
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
				if ((psub->type==SUBSYSTEM_SENSORS) || (psub->flags[Model::Subsystem_Flags::Awacs]))
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

/**
 * Calculate the damage for an object based on the location of an area-effect
 * explosion.
 *
 * @param objp			Object pointer ship receiving blast effect
 * @param pos			World pos of blast center
 * @param inner_rad		Smallest radius at which full damage is done
 * @param outer_rad		Radius at which no damage is done
 * @param max_blast		Maximum blast possible from explosion
 * @param max_damage	Maximum damage possible from explosion
 * @param blast			OUTPUT PARAMETER: receives blast value from explosion
 * @param damage		OUTPUT PARAMETER: receives damage value from explosion
 * @param limit			A limit on the area, needed for shockwave damage
 *
 * @return		No damage occurred, -1
 * @return		Damage occured, 0
 */
int weapon_area_calc_damage(object *objp, vec3d *pos, float inner_rad, float outer_rad, float max_blast, float max_damage, float *blast, float *damage, float limit)
{
	float dist;
	vec3d box_pt;

	// if object receiving the blast is a ship, use the bbox for distances
	// otherwise use the objects radius
	// could possibly exclude SIF_SMALL_SHIP (& other small objects) from using the bbox
	if (objp->type == OBJ_SHIP) {
		int inside = get_nearest_bbox_point(objp, pos, &box_pt);
		if (inside) {
			dist = 0.0001f;
		} else {
			dist = vm_vec_dist_quick(pos, &box_pt);
		}
	} else {
		dist = vm_vec_dist_quick(&objp->pos, pos) - objp->radius;
	}

	if ( (dist > outer_rad) || (dist > limit) ) {
		return -1;	// spheres don't intersect at all
	}

	if ( dist < inner_rad ) {
		// damage is maximum within inner radius
		*damage = max_damage;
		*blast = max_blast;
	} else {
		float dist_to_outer_rad_squared = (outer_rad-dist)*(outer_rad-dist);
		float total_dist_squared = (inner_rad-outer_rad)*(inner_rad-outer_rad);

		// this means the inner and outer radii are basically equal... and since we aren't within the inner radius,
		// we fudge the law of excluded middle to place ourselves outside the outer radius
		if (total_dist_squared < 0.0001f) {
			return -1;	// avoid divide-by-zero; we won't take damage anyway
		}

		// AL 2-24-98: drop off damage relative to square of distance
		Assert(dist_to_outer_rad_squared <= total_dist_squared);
		*damage = max_damage * dist_to_outer_rad_squared/total_dist_squared;
		*blast =  (dist - outer_rad) * max_blast /(inner_rad - outer_rad);
	}

	return 0;
}

/**
 * Apply the blast effects of an explosion to a ship
 *
 * @param force_apply_pos	World pos of where force is applied to object
 * @param ship_objp			Object pointer of ship receiving the blast
 * @param blast_pos			World pos of blast center
 * @param blast				Force of blast
 * @param make_shockwave	Boolean, whether to create a shockwave or not
 */
void weapon_area_apply_blast(vec3d * /*force_apply_pos*/, object *ship_objp, vec3d *blast_pos, float blast, int make_shockwave)
{
	#define	SHAKE_CONST 3000
	vec3d		force, vec_blast_to_ship, vec_ship_to_impact;
	polymodel		*pm;

	// don't waste time here if there is no blast force
	if ( blast == 0.0f )
		return;

	// apply blast force based on distance from center of explosion
	vm_vec_sub(&vec_blast_to_ship, &ship_objp->pos, blast_pos);
	vm_vec_normalize_safe(&vec_blast_to_ship);
	vm_vec_copy_scale(&force, &vec_blast_to_ship, blast );

	vm_vec_sub(&vec_ship_to_impact, blast_pos, &ship_objp->pos);

	pm = model_get(Ship_info[Ships[ship_objp->instance].ship_info_index].model_num);
	Assert ( pm != NULL );

	if (make_shockwave) {
		physics_apply_shock (&force, blast, &ship_objp->phys_info, &ship_objp->orient, &pm->mins, &pm->maxs, pm->rad);
		if (ship_objp == Player_obj) {
			joy_ff_play_vector_effect(&vec_blast_to_ship, blast * 2.0f);
		}
	} else {
		ship_apply_whack( &force, &vec_ship_to_impact, ship_objp);
	}
}

/**
 * Do the area effect for a weapon
 *
 * @param wobjp		Object pointer to weapon causing explosion
 * @param sci		Shockwave info
 * @param pos		World pos of explosion center
 * @param other_obj	Object pointer to ship that weapon impacted on (can be NULL)
 */
void weapon_do_area_effect(object *wobjp, shockwave_create_info *sci, vec3d *pos, object *other_obj)
{
	weapon_info	*wip;
	object		*objp;
	float			damage, blast;

	wip = &Weapon_info[Weapons[wobjp->instance].weapon_info_index];	

	// only blast ships and asteroids
	// And (some) weapons
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) && (objp->type != OBJ_WEAPON) ) {
			continue;
		}
	
		if (objp->type == OBJ_WEAPON) {
			// only apply to missiles with hitpoints
			weapon_info* wip2 = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (wip2->weapon_hitpoints <= 0)
				continue;
			if (!((wip2->wi_flags[Weapon::Info_Flags::Takes_blast_damage]) || (wip->wi_flags[Weapon::Info_Flags::Ciws])))
				continue;
		}

		if ( objp->type == OBJ_SHIP ) {
			// don't blast navbuoys
			if ( ship_get_SIF(objp->instance)[Ship::Info_Flags::Navbuoy] ) {
				continue;
			}
		}

		if ( weapon_area_calc_damage(objp, pos, sci->inner_rad, sci->outer_rad, sci->blast, sci->damage, &blast, &damage, sci->outer_rad) == -1 ){
			continue;
		}

		// scale damage
		damage *= weapon_get_damage_scale(wip, wobjp, other_obj);		

		weapon_info* target_wip;

		switch ( objp->type ) {
		case OBJ_SHIP:
			// If we're doing an AoE Electronics blast, do the electronics stuff (unless it also has the regular "electronics"
			// flag and this is the ship the missile directly impacted; then leave it for the regular code below) -MageKing17
			if ( (wip->wi_flags[Weapon::Info_Flags::Aoe_Electronics]) && !((objp->flags[Object::Object_Flags::Invulnerable]) || ((objp == other_obj) && (wip->wi_flags[Weapon::Info_Flags::Electronics]))) ) {
				weapon_do_electronics_effect(objp, pos, Weapons[wobjp->instance].weapon_info_index);
			}
			ship_apply_global_damage(objp, wobjp, pos, damage);
			weapon_area_apply_blast(NULL, objp, pos, blast, 0);
			break;
		case OBJ_ASTEROID:
			asteroid_hit(objp, NULL, NULL, damage);
			break;
		case OBJ_WEAPON:
			target_wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (target_wip->armor_type_idx >= 0)
				damage = Armor_types[target_wip->armor_type_idx].GetDamage(damage, wip->damage_type_idx, 1.0f);

			objp->hull_strength -= damage;
			if (objp->hull_strength < 0.0f) {
				Weapons[objp->instance].lifeleft = 0.01f;
				Weapons[objp->instance].weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
			}
			break;
		default:
			Int3();
			break;
		} 	

	}	// end for


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

	if((wp->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon])
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
			if(wp->homing_object == &obj_used_list)
				return false;
			if(IS_VEC_NULL(&wp->homing_pos) || vm_vec_dist(&wobj->pos, &wp->homing_pos) > wip->arm_radius)
				return false;
		}
	}

	return true;
}


/**
 * Called when a weapon hits something (or, in the case of
 * missiles explodes for any particular reason)
 */
void weapon_hit( object * weapon_obj, object * other_obj, vec3d * hitpos, int quadrant, vec3d* hitnormal )
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
	weapon_info	*wip;
	weapon *wp;
	bool		hit_target = false;

	object      *other_objp;
	ship_obj	*so;
	ship		*shipp;
	int         objnum;

	Assert((weapon_type >= 0) && (weapon_type < MAX_WEAPON_TYPES));
	if((weapon_type < 0) || (weapon_type >= MAX_WEAPON_TYPES)){
		return;
	}
	wp = &Weapons[weapon_obj->instance];
	wip = &Weapon_info[weapon_type];
	objnum = wp->objnum;

	// check if the weapon actually hit the intended target
	if (wp->homing_object != &obj_used_list)
		if (wp->homing_object == other_obj)
			hit_target = true;

	//This is an expensive check
	bool armed_weapon = weapon_armed(&Weapons[num], hit_target);

	// if this is the player ship, and is a laser hit, skip it. wait for player "pain" to take care of it
	if ((other_obj != Player_obj) || (wip->subtype != WP_LASER) || !MULTIPLAYER_CLIENT) {
		weapon_hit_do_sound(other_obj, wip, hitpos, armed_weapon, quadrant);
	}

	if (wip->impact_weapon_expl_effect.isValid() && armed_weapon) {
		auto particleSource = particle::ParticleManager::get()->createSource(wip->impact_weapon_expl_effect);
		particleSource.moveTo(hitpos);
		particleSource.setOrientationFromVec(&weapon_obj->phys_info.vel);

		if (hitnormal)
		{
			particleSource.setOrientationNormal(hitnormal);
		}

		particleSource.finish();
	} else if (wip->dinky_impact_weapon_expl_effect.isValid() && !armed_weapon) {
		auto particleSource = particle::ParticleManager::get()->createSource(wip->dinky_impact_weapon_expl_effect);
		particleSource.moveTo(hitpos);
		particleSource.setOrientationFromVec(&weapon_obj->phys_info.vel);

		if (hitnormal)
		{
			particleSource.setOrientationNormal(hitnormal);
		}

		particleSource.finish();
	}

	if ((other_obj != nullptr) && (quadrant == -1) && (wip->piercing_impact_effect.isValid() && armed_weapon)) {
		if ((other_obj->type == OBJ_SHIP) || (other_obj->type == OBJ_DEBRIS)) {

			int ok_to_draw = 1;

			if (other_obj->type == OBJ_SHIP) {
				float draw_limit, hull_pct;
				int dmg_type_idx, piercing_type;

				shipp = &Ships[other_obj->instance];

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
				using namespace particle;

				auto primarySource = ParticleManager::get()->createSource(wip->piercing_impact_effect);
				primarySource.moveTo(&weapon_obj->pos);
				primarySource.setOrientationMatrix(&weapon_obj->last_orient);

				if (hitnormal)
				{
					primarySource.setOrientationNormal(hitnormal);
				}

				primarySource.finish();

				if (wip->piercing_impact_secondary_effect.isValid()) {
					auto secondarySource = ParticleManager::get()->createSource(wip->piercing_impact_secondary_effect);
					secondarySource.moveTo(&weapon_obj->pos);
					secondarySource.setOrientationMatrix(&weapon_obj->last_orient);

					if (hitnormal)
					{
						secondarySource.setOrientationNormal(hitnormal);
					}

					secondarySource.finish();
				}
			}
		}
	}

	// For all objects that had this weapon as a target, wipe it out, forcing find of a new enemy
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		other_objp = &Objects[so->objnum];
		Assert(other_objp->instance != -1);
        
		shipp = &Ships[other_objp->instance];
		Assert(shipp->ai_index != -1);
        
		ai_info	*aip = &Ai_info[shipp->ai_index];
        
		if (aip->target_objnum == objnum) {
			set_target_objnum(aip, -1);
			//	If this ship had a dynamic goal of chasing this weapon, clear the dynamic goal.
			if (aip->resume_goal_time != -1)
				aip->active_goal = AI_GOAL_NONE;
		}
        
		if (aip->goal_objnum == objnum) {
			aip->goal_objnum = -1;
			aip->goal_signature = -1;
		}
        
		if (aip->guard_objnum == objnum) {
			aip->guard_objnum = -1;
			aip->guard_signature = -1;
		}
        
		if (aip->hitter_objnum == objnum) {
			aip->hitter_objnum = -1;
        }
	}

    weapon_obj->flags.set(Object::Object_Flags::Should_be_dead);

	//Set shockwaves flag
	int sw_flag = SW_WEAPON;

	if ( ((other_obj) && (other_obj->type == OBJ_WEAPON)) || (Weapons[num].weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon])) {
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
	if(wip->wi_flags[Weapon::Info_Flags::Emp]){
		emp_apply(&weapon_obj->pos, wip->shockwave.inner_rad, wip->shockwave.outer_rad, wip->emp_intensity, wip->emp_time, (wip->wi_flags[Weapon::Info_Flags::Use_emp_time_for_capship_turrets]) != 0);
	}	

	// if this weapon has the "Electronics" flag set, then disrupt subsystems in sphere
	if ((other_obj != NULL) && (wip->wi_flags[Weapon::Info_Flags::Electronics])) {
		if (other_obj->type == OBJ_SHIP) {
			weapon_do_electronics_effect(other_obj, &weapon_obj->pos, Weapons[weapon_obj->instance].weapon_info_index);
		}
	}

	// spawn weapons - note the change from FS 1 multiplayer.
	if (wip->wi_flags[Weapon::Info_Flags::Spawn]){
		if (!((wip->wi_flags[Weapon::Info_Flags::Dont_spawn_if_shot]) && (Weapons[num].weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]))){			// prevent spawning of children if shot down and the dont spawn if shot flag is set (DahBlount)
			spawn_child_weapons(weapon_obj);
		}
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
	if ( (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Corkscrew]) ) {
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

/**
 * Call before weapons_page_in to mark a weapon as used
 */
void weapon_mark_as_used(int weapon_type)
{
	if (weapon_type < 0)
		return;

	if ( used_weapons == NULL )
		return;

	Assert( weapon_type < MAX_WEAPON_TYPES );

	if (weapon_type < Num_weapon_types) {
		used_weapons[weapon_type]++;
		if (Weapon_info[weapon_type].num_substitution_patterns > 0) {
			for (size_t i = 0; i < Weapon_info[weapon_type].num_substitution_patterns; i++) {
				used_weapons[Weapon_info[weapon_type].weapon_substitution_pattern[i]]++;
			}
		}
	}
}

void weapons_page_in()
{
	TRACE_SCOPE(tracing::WeaponPageIn);

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

        wip->wi_flags.remove(Weapon::Info_Flags::Thruster);		// Assume no thrusters
		
		switch (wip->render_type)
		{
			case WRT_POF:
			{
				wip->model_num = model_load( wip->pofbitmap_name, 0, NULL );

				polymodel *pm = model_get( wip->model_num );

				// If it has a model, and the model pof has thrusters, then set
				// the flags
				if (pm->n_thrusters > 0) {
                    wip->wi_flags.set(Weapon::Info_Flags::Thruster);
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
		shockwave_create_info_load(&wip->shockwave);
		shockwave_create_info_load(&wip->dinky_shockwave);

		// trail bitmaps
		if ( (wip->wi_flags[Weapon::Info_Flags::Trail]) && (wip->tr_info.texture.bitmap_id > -1) )
			bm_page_in_texture( wip->tr_info.texture.bitmap_id );

		// if this is a beam weapon, page in its stuff
		if (wip->wi_flags[Weapon::Info_Flags::Beam]) {
			// all beam sections
			for (idx = 0; idx < wip->b_info.beam_num_sections; idx++)
				bm_page_in_texture(wip->b_info.sections[idx].texture.first_frame);

			// muzzle glow
			bm_page_in_texture(wip->b_info.beam_glow.first_frame);

			// particle ani
			bm_page_in_texture(wip->b_info.beam_particle_ani.first_frame);
		}

		if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) {
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

		decals::pageInDecal(wip->impact_decal);
	}
}

/**
 * Page_in function for cheaters, grabs all weapons that weren't already in a mission
 * and loads the models for them.
 *
 * Non-model graphics elements will get loaded when they are rendered for the first time.  
 * Maybe not the best way to do this but faster and a lot less error prone.
 */
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
		
        wip->wi_flags.remove(Weapon::Info_Flags::Thruster);		// Assume no thrusters

		if ( wip->render_type == WRT_POF ) {
			wip->model_num = model_load( wip->pofbitmap_name, 0, NULL );
				
			polymodel *pm = model_get( wip->model_num );
				
			// If it has a model, and the model pof has thrusters, then set
			// the flags
			if ( pm->n_thrusters > 0 )	{
                wip->wi_flags.set(Weapon::Info_Flags::Thruster);
			}
		}
		
		wip->external_model_num = -1;
		
		if ( strlen(wip->external_model_name) )
			wip->external_model_num = model_load( wip->external_model_name, 0, NULL );

		if (wip->external_model_num == -1)
			wip->external_model_num = wip->model_num;
		
		
		//Load shockwaves
		shockwave_create_info_load(&wip->shockwave);
		shockwave_create_info_load(&wip->dinky_shockwave);

		used_weapons[i]++;
	}
}

/* Helper function for l_Weaponclass.isWeaponLoaded()
 * Pages in a single weapon and its substitutes and chilren given the weapon_info index for it
 */
bool weapon_page_in(int weapon_type)
{
	Assert(used_weapons != NULL);

	if (weapon_type < 0 || weapon_type >= Num_weapon_types) {
		return false;
	}

	SCP_vector<int> page_in_weapons;
	page_in_weapons.push_back(weapon_type);
	
	// Make sure substitution weapons are paged in as well
	if (Weapon_info[weapon_type].num_substitution_patterns > 0) {
		for (size_t i = 0; i < Weapon_info[weapon_type].num_substitution_patterns; i++) {
			page_in_weapons.push_back(Weapon_info[weapon_type].weapon_substitution_pattern[i]);
		}
	}

	// this grabs all spawn weapon types (Cluster Baby, etc.) which can't be
	// assigned directly to a ship
	// if it's got a spawn type then grab it
	size_t size = page_in_weapons.size();
	for (size_t x = 0; x < size; x++) {
		if (Weapon_info[page_in_weapons.at(x)].num_spawn_weapons_defined > 0) {
			for (int j = 0; j < Weapon_info[page_in_weapons.at(x)].num_spawn_weapons_defined; j++) {
				page_in_weapons.push_back((int)Weapon_info[page_in_weapons.at(x)].spawn_info[j].spawn_type);
			}
		}
	}

	for (size_t k = 0; k < page_in_weapons.size(); k++) {
		if (used_weapons[page_in_weapons.at(k)]) {
			continue;		// If weapon is already paged_in, we don't need to page it in again
		}

		// Page in bitmaps for the weapon
		weapon_load_bitmaps(page_in_weapons.at(k));

		weapon_info *wip = &Weapon_info[page_in_weapons.at(k)];

		wip->wi_flags.remove(Weapon::Info_Flags::Thruster);		// Assume no thrusters

		switch (wip->render_type)
		{
		case WRT_POF:
		{
			wip->model_num = model_load(wip->pofbitmap_name, 0, NULL);

			polymodel *pm = model_get(wip->model_num);

			// If it has a model, and the model pof has thrusters, then set
			// the flags
			if (pm->n_thrusters > 0) {
				wip->wi_flags.set(Weapon::Info_Flags::Thruster);
			}

			for (int j = 0; j < pm->n_textures; j++)
				pm->maps[j].PageIn();

			break;
		}

		case WRT_LASER:
		{
			bm_page_in_texture(wip->laser_bitmap.first_frame);
			bm_page_in_texture(wip->laser_glow_bitmap.first_frame);

			break;
		}

		default:
			Assertion(wip->render_type != WRT_POF && wip->render_type != WRT_LASER, "Weapon %s does not have a valid rendering type. Type passed: %d\n", wip->name, wip->render_type);	// Invalid weapon rendering type.
		}

		wip->external_model_num = -1;

		if (strlen(wip->external_model_name))
			wip->external_model_num = model_load(wip->external_model_name, 0, NULL);

		if (wip->external_model_num == -1)
			wip->external_model_num = wip->model_num;


		//Load shockwaves
		shockwave_create_info_load(&wip->shockwave);
		shockwave_create_info_load(&wip->dinky_shockwave);

		// trail bitmaps
		if ((wip->wi_flags[Weapon::Info_Flags::Trail]) && (wip->tr_info.texture.bitmap_id > -1))
			bm_page_in_texture(wip->tr_info.texture.bitmap_id);

		// if this is a beam weapon, page in its stuff
		if (wip->wi_flags[Weapon::Info_Flags::Beam]) {
			// all beam sections
			for (int idx = 0; idx < wip->b_info.beam_num_sections; idx++)
				bm_page_in_texture(wip->b_info.sections[idx].texture.first_frame);

			// muzzle glow
			bm_page_in_texture(wip->b_info.beam_glow.first_frame);

			// particle ani
			bm_page_in_texture(wip->b_info.beam_particle_ani.first_frame);
		}

		if (wip->wi_flags[Weapon::Info_Flags::Particle_spew]) {
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

		// Page in decal bitmaps
		decals::pageInDecal(wip->impact_decal);

		used_weapons[page_in_weapons.at(k)]++;	// Ensures weapon can be counted as used
	}

	return true;
}

bool weapon_used(int weapon_type) {
	return used_weapons[weapon_type] > 0;
}

/**
 * Get the "color" of the laser at the given moment (since glowing lasers can cycle colors)
 */
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
	if ( (winfo->laser_color_2.red == winfo->laser_color_1.red) && (winfo->laser_color_2.green == winfo->laser_color_1.green) && (winfo->laser_color_2.blue == winfo->laser_color_1.blue) ) {
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

/**
 * For weapons flagged as particle spewers, spew particles. wheee
 */
void weapon_maybe_spew_particle(object *obj)
{
	weapon *wp;
	weapon_info *wip;
	int idx;

	// check some stuff
	Assert(obj->type == OBJ_WEAPON);
	Assert(obj->instance >= 0);
	Assert(Weapons[obj->instance].weapon_info_index >= 0);
	Assert(Weapon_info[Weapons[obj->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Particle_spew]);
	
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
				if (!vm_vec_same(&wip->particle_spewers[psi].particle_spew_offset, &vmd_zero_vector)) {	// don't xform unused vectors
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

						if (wip->wi_flags[Weapon::Info_Flags::Corkscrew]) {
							vm_vec_add(&particle_pos, &obj->last_pos, &direct);
						} else {
							vm_vec_add(&particle_pos, &obj->pos, &direct);
						}

						// emit the particle
						if (wip->particle_spewers[psi].particle_spew_anim.first_frame < 0) {
							particle::create(&particle_pos,
											 &vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_SMOKE);
						} else {
							particle::create(&particle_pos,
											 &vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_BITMAP,
											 wip->particle_spewers[psi].particle_spew_anim.first_frame);
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
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_SMOKE);
						} else {
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_BITMAP,
											 wip->particle_spewers[psi].particle_spew_anim.first_frame);
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
						
						if (wip->particle_spewers[psi].particle_spew_z_scale != 1.0f) {	// don't do the extra math for spherical effect
							temp_vel = input_vel;
							temp_vel.xyz.z *= wip->particle_spewers[psi].particle_spew_z_scale;	// for an ovoid particle effect to better combine with laser effects
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
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_SMOKE);
						} else {
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_BITMAP,
											 wip->particle_spewers[psi].particle_spew_anim.first_frame);
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
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_SMOKE);
						} else {
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_BITMAP,
											 wip->particle_spewers[psi].particle_spew_anim.first_frame);
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
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_SMOKE);
						} else {
							particle::create(&output_pos,
											 &output_vel,
											 wip->particle_spewers[psi].particle_spew_lifetime,
											 wip->particle_spewers[psi].particle_spew_radius,
											 particle::PARTICLE_BITMAP,
											 wip->particle_spewers[psi].particle_spew_anim.first_frame);
						}
					}
				}
			}
		}
	}
}

/**
 * Debug console functionality
 */
void dcf_pspew();
DCF(pspew_count, "Number of particles spewed at a time")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
			dc_printf("Partical count is %i\n", Weapon_particle_spew_count);
			return;
	}

	dc_stuff_int(&Weapon_particle_spew_count);
	
	dc_printf("Partical count set to %i\n", Weapon_particle_spew_count);
}

DCF(pspew_time, "Time between particle spews")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle spawn period is %i\n", Weapon_particle_spew_time);
		return;
	}

	dc_stuff_int(&Weapon_particle_spew_time);

	dc_printf("Particle spawn period set to %i\n", Weapon_particle_spew_time);
}

DCF(pspew_vel, "Relative velocity of particles (0.0 - 1.0)")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle relative velocity is %f\n", Weapon_particle_spew_vel);
		return;
	}

	dc_stuff_float(&Weapon_particle_spew_vel);

	dc_printf("Particle relative velocity set to %f\n", Weapon_particle_spew_vel);
}

DCF(pspew_size, "Size of spewed particles")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle size is %f\n", Weapon_particle_spew_radius);
		return;
	}

	dc_stuff_float(&Weapon_particle_spew_radius);

	dc_printf("Particle size set to %f\n", Weapon_particle_spew_radius);
}

DCF(pspew_life, "Lifetime of spewed particles")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle lifetime is %f\n", Weapon_particle_spew_lifetime);
		return;
	}

	dc_stuff_float(&Weapon_particle_spew_lifetime);

	dc_printf("Particle lifetime set to %f\n", Weapon_particle_spew_lifetime);
}

DCF(pspew_scale, "How far away particles are from the weapon path")
{
	if (dc_optional_string_either("help", "--help")) {
		dcf_pspew();
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle scale is %f\n", Weapon_particle_spew_scale);
	}

	dc_stuff_float(&Weapon_particle_spew_scale);

	dc_printf("Particle scale set to %f\n", Weapon_particle_spew_scale);
}

// Help and Status provider
DCF(pspew, "Particle spew help and status provider")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle spew settings\n\n");

		dc_printf(" Count   (pspew_count) : %d\n", Weapon_particle_spew_count);
		dc_printf(" Time     (pspew_time) : %d\n", Weapon_particle_spew_time);
		dc_printf(" Velocity  (pspew_vel) : %f\n", Weapon_particle_spew_vel);
		dc_printf(" Size     (pspew_size) : %f\n", Weapon_particle_spew_radius);
		dc_printf(" Lifetime (pspew_life) : %f\n", Weapon_particle_spew_lifetime);
		dc_printf(" Scale   (psnew_scale) : %f\n", Weapon_particle_spew_scale);
		return;
	}

	dc_printf("Available particlar spew commands:\n");
	dc_printf("pspew_count : %s\n", dcmd_pspew_count.help);
	dc_printf("pspew_time  : %s\n", dcmd_pspew_time.help);
	dc_printf("pspew_vel   : %s\n", dcmd_pspew_vel.help);
	dc_printf("pspew_size  : %s\n", dcmd_pspew_size.help);
	dc_printf("pspew_life  : %s\n", dcmd_pspew_life.help);
	dc_printf("pspew_scale : %s\n\n", dcmd_pspew_scale.help);

	dc_printf("To view status of all pspew settings, type in 'pspew --status'.\n");
	dc_printf("Passing '--status' as an argument to any of the individual spew commands will show the status of that variable only.\n\n");

	dc_printf("These commands adjust the various properties of the particle spew system, which is used by weapons when they are fired, are in-flight, and die (either by impact or by end of life time.\n");
	dc_printf("Generally, a large particle count with small size and scale will result in a nice dense particle spew.\n");
	dc_printf("Be advised, this effect is applied to _ALL_ weapons, and as such may drastically reduce framerates on lower powered platforms.\n");
}

/**
 * Return a scale factor for damage which should be applied for 2 collisions
 */
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
	if((wep->parent >= 0) && (wep->parent < MAX_OBJECTS) && (Objects[wep->parent].flags[Object::Object_Flags::Player_ship])){
		from_player = 1;
	}
		
	// if this is a lockarm weapon, and it was fired unlocked
	if((wip->wi_flags[Weapon::Info_Flags::Lockarm]) && !(wp->weapon_flags[Weapon::Weapon_Flags::Locked_when_fired])){		
		total_scale *= 0.1f;
	}
	
	// if the hit object was a ship and we're doing damage scaling
	if ( (target->type == OBJ_SHIP) &&
		!(The_mission.ai_profile->flags[AI::Profile_Flags::Disable_weapon_damage_scaling]) &&
		!(Ship_info[Ships[target->instance].ship_info_index].flags[Ship::Info_Flags::Disable_weapon_damage_scaling])
	) {
		ship_info *sip;

		// get some info on the ship
		Assert((target->instance >= 0) && (target->instance < MAX_SHIPS));
		if((target->instance < 0) || (target->instance >= MAX_SHIPS)){
			return total_scale;
		}
		sip = &Ship_info[Ships[target->instance].ship_info_index];

		// get hull pct of the ship currently
		hull_pct = get_hull_pct(target);

		// if it has hit a supercap ship and is not a supercap class weapon
		if((sip->flags[Ship::Info_Flags::Supercap]) && !(wip->wi_flags[Weapon::Info_Flags::Supercap])){
			// if the supercap is around 3/4 damage, apply nothing
			if(hull_pct <= 0.75f){
				return 0.0f;
			} else {
				total_scale *= SUPERCAP_DAMAGE_SCALE;
			}
		}

		// determine if this is a big damage ship
		is_big_damage_ship = (sip->flags[Ship::Info_Flags::Big_damage]);

		// if this is a large ship, and is being hit by flak
		if(is_big_damage_ship && (wip->wi_flags[Weapon::Info_Flags::Flak])){
			total_scale *= FLAK_DAMAGE_SCALE;
		}
		
		// if the weapon is a small weapon being fired at a big ship
		if( is_big_damage_ship && !(wip->hurts_big_ships()) ){

			// if the player is firing it
			if ( from_player && !(The_mission.ai_profile->flags[AI::Profile_Flags::Player_weapon_scale_fix])) {
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

void pause_in_flight_sounds()
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].objnum != -1)
		{
			weapon* wp = &Weapons[i];

			if (wp->hud_in_flight_snd_sig.isValid() && snd_is_playing(wp->hud_in_flight_snd_sig)) {
				// Stop sound, it will be restarted in the first frame after the game is unpaused
				snd_stop(wp->hud_in_flight_snd_sig);
			}
		}
	}
}

void weapon_pause_sounds()
{
	// Pause all beam sounds
	beam_pause_sounds();

	// Pause in-flight sounds
	pause_in_flight_sounds();
}

void weapon_unpause_sounds()
{
	// Pause all beam sounds
	beam_unpause_sounds();
}

void shield_impact_explosion(vec3d *hitpos, object *objp, float radius, int idx) {
	int expl_ani_handle = Weapon_explosions.GetAnim(idx, hitpos, radius);
	particle::create(hitpos,
					 &vmd_zero_vector,
					 0.0f,
					 radius,
					 particle::PARTICLE_BITMAP_PERSISTENT,
					 expl_ani_handle,
					 objp);
}

void weapon_render(object* obj, model_draw_list *scene)
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

	if (wip->wi_flags[Weapon::Info_Flags::Transparent]) {
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
			if(wip->laser_length < 0.0001f)
				return;

			int alpha = 255;
			int framenum = 0;

			if (wip->laser_bitmap.first_frame >= 0) {					
				gr_set_color_fast(&wip->laser_color_1);

				if (wip->laser_bitmap.num_frames > 1) {
					wp->laser_bitmap_frame += flFrametime;

					framenum = bm_get_anim_frame(wip->laser_bitmap.first_frame, wp->laser_bitmap_frame, wip->laser_bitmap.total_time, true);
				}

				if (wip->wi_flags[Weapon::Info_Flags::Transparent])
					alpha = fl2i(wp->alpha_current * 255.0f);

				vec3d headp;

				vm_vec_scale_add(&headp, &obj->pos, &obj->orient.vec.fvec, wip->laser_length);

				batching_add_laser(wip->laser_bitmap.first_frame + framenum, &headp, wip->laser_head_radius, &obj->pos, wip->laser_tail_radius, alpha, alpha, alpha);
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

				if (wip->wi_flags[Weapon::Info_Flags::Transparent]) {
					alpha = fl2i(wp->alpha_current * 255.0f);
					alpha -= 38; // take 1.5f into account for the normal glow alpha

					if (alpha < 0)
						alpha = 0;
				} else {
					alpha = weapon_glow_alpha;
				}

				batching_add_laser(wip->laser_glow_bitmap.first_frame + framenum, &headp2, wip->laser_head_radius * weapon_glow_scale_f, &tailp, wip->laser_tail_radius * weapon_glow_scale_r, (c.red*alpha)/255, (c.green*alpha)/255, (c.blue*alpha)/255);
			}

			break;
		}

	case WRT_POF:
		{
			model_render_params render_info;

			uint render_flags = MR_NORMAL|MR_IS_MISSILE|MR_NO_BATCH;

			if (wip->wi_flags[Weapon::Info_Flags::Mr_no_lighting])
				render_flags |= MR_NO_LIGHTING;

			if (wip->wi_flags[Weapon::Info_Flags::Transparent]) {
				render_info.set_alpha(wp->alpha_current);
				render_flags |= MR_ALL_XPARENT;
			}

			model_clear_instance(wip->model_num);

			if ( (wip->wi_flags[Weapon::Info_Flags::Thruster]) && ((wp->thruster_bitmap > -1) || (wp->thruster_glow_bitmap > -1)) ) {
				float ft;
				mst_info mst;

				//	Add noise to thruster geometry.
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

				render_info.set_thruster_info(mst);

				render_flags |= MR_SHOW_THRUSTERS;
			}


			//don't render local ssm's when they are still in subspace
			if (wp->lssm_stage==3)
				break;

			// start a clip plane
			if ( wp->lssm_stage == 2 ) {
				object *wobj=&Objects[wp->lssm_warp_idx];		//warphole object

				render_info.set_clip_plane(wobj->pos, wobj->orient.vec.fvec);
			}

			render_info.set_flags(render_flags);

			model_render_queue(&render_info, scene, wip->model_num, &obj->orient, &obj->pos);

			break;
		}

	default:
		Warning(LOCATION, "Unknown weapon rendering type = %i for weapon %s\n", wip->render_type, wip->name);
	}
}

// Called by hudartillery.cpp after SSMs have been parsed to make sure that $SSM: entries defined in weapons are valid.
void validate_SSM_entries()
{
	int wi;
	SCP_vector<SCP_string>::const_iterator it;
	weapon_info *wip;

	for (it = Delayed_SSM_names.begin(); it != Delayed_SSM_names.end(); ++it) {
		delayed_ssm_data *dat = &Delayed_SSM_data[*it];
		wi = weapon_info_lookup(it->c_str());
		Assertion(wi >= 0, "Trying to validate non-existant weapon '%s'; get a coder!\n", it->c_str());
		wip = &Weapon_info[wi];
		nprintf(("parse", "Starting validation of '%s' [wip->name is '%s'], currently has an SSM_index of %d.\n", it->c_str(), wip->name, wip->SSM_index));
		wip->SSM_index = ssm_info_lookup(dat->ssm_entry.c_str());
		if (wip->SSM_index < 0) {
			Warning(LOCATION, "Unknown SSM entry '%s' in specification for %s (%s:line %d).\n", dat->ssm_entry.c_str(), it->c_str(), dat->filename.c_str(), dat->linenum);
		}
		nprintf(("parse", "Validation complete, SSM_index is %d.\n", wip->SSM_index));
	}

	// This information is no longer relevant, so might as well clear it out.
	Delayed_SSM_data.clear();
	Delayed_SSM_names.clear();

	for (it = Delayed_SSM_indices.begin(); it != Delayed_SSM_indices.end(); ++it) {
		delayed_ssm_index_data *dat = &Delayed_SSM_indices_data[*it];
		wi = weapon_info_lookup(it->c_str());
		Assertion(wi >= 0, "Trying to validate non-existant weapon '%s'; get a coder!\n", it->c_str());
		wip = &Weapon_info[wi];
		nprintf(("parse", "Starting validation of '%s' [wip->name is '%s'], currently has an SSM_index of %d.\n", it->c_str(), wip->name, wip->SSM_index));
		if (wip->SSM_index < -1 || wip->SSM_index >= static_cast<int>(Ssm_info.size())) {
			Warning(LOCATION, "Invalid SSM index '%d' (should be 0-" SIZE_T_ARG ") in specification for %s (%s:line %d).\n", wip->SSM_index, Ssm_info.size() - 1, it->c_str(), dat->filename.c_str(), dat->linenum);
			wip->SSM_index = -1;
		}
		nprintf(("parse", "Validation complete, SSM-index is %d.\n", wip->SSM_index));
	}
}

weapon_info::weapon_info()
{
	this->reset();
}

void weapon_info::reset()
{
	// INITIALIZE NEW FIELDS HERE
	// The order should match the order in the struct!
	int i, j;

    this->wi_flags.reset();

	this->subtype = WP_UNUSED;
	this->render_type = WRT_NONE;

	memset(this->name, 0, sizeof(this->name));
	memset(this->alt_name, 0, sizeof(this->alt_name));
	memset(this->title, 0, sizeof(this->title));
	this->desc = NULL;

	memset(this->tech_title, 0, sizeof(this->tech_title));
	memset(this->tech_anim_filename, 0, sizeof(this->tech_anim_filename));
	this->tech_desc = NULL;
	memset(this->tech_model, 0, sizeof(this->tech_model));

	memset(this->hud_filename, 0, sizeof(this->hud_filename));
	this->hud_image_index = -1;

	memset(this->pofbitmap_name, 0, sizeof(this->pofbitmap_name));

	this->model_num = -1;
	this->hud_target_lod = -1;
	this->num_detail_levels = -1;
	for (i = 0; i < MAX_MODEL_DETAIL_LEVELS; i++)
	{
		this->detail_distance[i] = -1;
	}

	vm_vec_zero(&this->closeup_pos);
	this->closeup_zoom = 1.0f;

	generic_anim_init(&this->laser_bitmap);
	generic_anim_init(&this->laser_glow_bitmap);

	gr_init_color(&this->laser_color_1, 255, 255, 255);
	gr_init_color(&this->laser_color_2, 255, 255, 255);

	this->laser_length = 10.0f;
	this->laser_head_radius = 1.0f;
	this->laser_tail_radius = 1.0f;

	memset(this->external_model_name, 0, sizeof(this->external_model_name));
	this->external_model_num = -1;

	this->weapon_submodel_rotate_accell = 10.0f;
	this->weapon_submodel_rotate_vel = 0.0f;

	this->mass = 1.0f;
	this->max_speed = 10.0f;
	this->acceleration_time = 0.0f;
	this->vel_inherit_amount = 1.0f;
	this->free_flight_time = 0.0f;
	this->fire_wait = 1.0f;
	this->max_delay = 0.0f;
	this->min_delay = 0.0f;
	this->damage = 0.0f;
	this->damage_time = -1.0f;
	this->atten_damage = -1.0f;

	this->damage_type_idx = -1;
	this->damage_type_idx_sav = -1;

	this->armor_type_idx = -1;

	this->arm_time = 0;
	this->arm_dist = 0.0f;
	this->arm_radius = 0.0f;
	this->det_range = 0.0f;
	this->det_radius = 0.0f;
	this->flak_targeting_accuracy = 60.0f; // Standard value as defined in flak.cpp
	this->flak_detonation_accuracy = 65.0f;
	this->untargeted_flak_range_penalty = 20.0f;

	this->armor_factor = 1.0f;
	this->shield_factor = 1.0f;
	this->subsystem_factor = 1.0f;

	this->life_min = -1.0f;
	this->life_max = -1.0f;
	this->lifetime = 1.0f;
	this->energy_consumed = 0.0f;

	this->cargo_size = 1.0f;

	this->turn_time = 1.0f;
	this->fov = 0;				//should be cos(pi), not pi

	this->min_lock_time = 0.0f;
	this->lock_pixels_per_sec = 50;
	this->catchup_pixels_per_sec = 50;
	this->catchup_pixel_penalty = 50;
	this->seeker_strength = 1.0f;

	this->swarm_count = -1;
	// *Default is 150  -Et1
	this->SwarmWait = SWARM_MISSILE_DELAY;

	this->pre_launch_snd = gamesnd_id();
	this->pre_launch_snd_min_interval = 0;

	this->launch_snd = gamesnd_id();
	this->impact_snd = gamesnd_id();
	this->disarmed_impact_snd = gamesnd_id();
	this->flyby_snd = gamesnd_id();

	this->rearm_rate = 1.0f;

	this->weapon_range = 999999999.9f;
	// *Minimum weapon range, default is 0 -Et1
	this->WeaponMinRange = 0.0f;

	this->num_spawn_weapons_defined = 0;

	for (i = 0; i < MAX_SPAWN_TYPES_PER_WEAPON; i++)
	{
		this->spawn_info[i].spawn_type = -1;
		this->spawn_info[i].spawn_angle = 180;
		this->spawn_info[i].spawn_count = DEFAULT_WEAPON_SPAWN_COUNT;
	}

	// Trails
	this->tr_info.pt = vmd_zero_vector;
	this->tr_info.w_start = 1.0f;
	this->tr_info.w_end = 1.0f;
	this->tr_info.a_start = 1.0f;
	this->tr_info.a_end = 1.0f;
	this->tr_info.max_life = 1.0f;
	this->tr_info.stamp = 0;
	generic_bitmap_init(&this->tr_info.texture, NULL);
	this->tr_info.n_fade_out_sections = 0;

	memset(this->icon_filename, 0, sizeof(this->icon_filename));

	memset(this->anim_filename, 0, sizeof(this->anim_filename));

	this->shield_impact_explosion_radius = 1.0f;

	this->impact_weapon_expl_effect = particle::ParticleEffectHandle::invalid();

	this->dinky_impact_weapon_expl_effect = particle::ParticleEffectHandle::invalid();

	this->flash_impact_weapon_expl_effect = particle::ParticleEffectHandle::invalid();

	this->piercing_impact_effect           = particle::ParticleEffectHandle::invalid();
	this->piercing_impact_secondary_effect = particle::ParticleEffectHandle::invalid();

	this->muzzle_flash = -1;

	this->emp_intensity = EMP_DEFAULT_INTENSITY;
	this->emp_time = EMP_DEFAULT_TIME;	// Goober5000: <-- Look!  I fixed a Volition bug!  Gimme $5, Dave!
	this->recoil_modifier = 1.0f;
	this->weapon_reduce = ESUCK_DEFAULT_WEAPON_REDUCE;
	this->afterburner_reduce = ESUCK_DEFAULT_AFTERBURNER_REDUCE;

	//customizeable corkscrew stuff
	this->cs_num_fired = 4;
	this->cs_radius = 1.25f;
	this->cs_delay = 30;
	this->cs_crotate = 1;
	this->cs_twist = 5.0f;

	this->elec_time = 8000;
	this->elec_eng_mult = 1.0f;
	this->elec_weap_mult = 1.0f;
	this->elec_beam_mult = 1.0f;
	this->elec_sensors_mult = 1.0f;
	this->elec_randomness = 2000;
	this->elec_use_new_style = 0;

	this->lssm_warpout_delay = 0;			//delay between launch and warpout (ms)
	this->lssm_warpin_delay = 0;			//delay between warpout and warpin (ms)
	this->lssm_stage5_vel = 0;		//velocity during final stage
	this->lssm_warpin_radius = 0;
	this->lssm_lock_range = 1000000.0f;	//local ssm lock range (optional)

	this->cm_aspect_effectiveness = 1.0f;
	this->cm_heat_effectiveness = 1.0f;
	this->cm_effective_rad = MAX_CMEASURE_TRACK_DIST;
	this->cm_detonation_rad = CMEASURE_DETONATE_DISTANCE;
	this->cm_kill_single = false;
	this->cmeasure_timer_interval = 0;

	this->b_info.beam_type = -1;
	this->b_info.beam_life = -1.0f;
	this->b_info.beam_warmup = -1;
	this->b_info.beam_warmdown = -1;
	this->b_info.beam_muzzle_radius = 0.0f;
	this->b_info.beam_particle_count = -1;
	this->b_info.beam_particle_radius = 0.0f;
	this->b_info.beam_particle_angle = 0.0f;
	this->b_info.beam_loop_sound = gamesnd_id();
	this->b_info.beam_warmup_sound = gamesnd_id();
	this->b_info.beam_warmdown_sound = gamesnd_id();
	this->b_info.beam_num_sections = 0;
	this->b_info.glow_length = 0;
	this->b_info.directional_glow = false;
	this->b_info.beam_shots = 1;
	this->b_info.beam_shrink_factor = 0.0f;
	this->b_info.beam_shrink_pct = 0.0f;
	this->b_info.range = BEAM_FAR_LENGTH;
	this->b_info.damage_threshold = 1.0f;
	this->b_info.beam_width = -1.0f;

	generic_anim_init(&this->b_info.beam_glow, NULL);
	generic_anim_init(&this->b_info.beam_particle_ani, NULL);

	for (i = 0; i < MAX_IFFS; i++)
		for (j = 0; j < NUM_SKILL_LEVELS; j++)
			this->b_info.beam_iff_miss_factor[i][j] = 0.00001f;

	//WMC - Okay, so this is needed now
	beam_weapon_section_info *bsip;
	for (i = 0; i < MAX_BEAM_SECTIONS; i++) {
		bsip = &this->b_info.sections[i];

		generic_anim_init(&bsip->texture, NULL);

		bsip->width = 1.0f;
		bsip->flicker = 0.1f;
		bsip->z_add = i2fl(MAX_BEAM_SECTIONS - i - 1);
		bsip->tile_type = 0;
		bsip->tile_factor = 1.0f;
		bsip->translation = 0.0f;
	}

	for (size_t s = 0; s < MAX_PARTICLE_SPEWERS; s++) {						// default values for everything -nuke
		this->particle_spewers[s].particle_spew_type = PSPEW_NONE;				// added by nuke
		this->particle_spewers[s].particle_spew_count = 1;
		this->particle_spewers[s].particle_spew_time = 25;
		this->particle_spewers[s].particle_spew_vel = 0.4f;
		this->particle_spewers[s].particle_spew_radius = 2.0f;
		this->particle_spewers[s].particle_spew_lifetime = 0.15f;
		this->particle_spewers[s].particle_spew_scale = 0.8f;
		this->particle_spewers[s].particle_spew_z_scale = 1.0f;			// added by nuke
		this->particle_spewers[s].particle_spew_rotation_rate = 10.0f;
		this->particle_spewers[s].particle_spew_offset = vmd_zero_vector;
		this->particle_spewers[s].particle_spew_velocity = vmd_zero_vector;
		generic_anim_init(&this->particle_spewers[s].particle_spew_anim, NULL);
	}

	this->tag_level = -1;
	this->tag_time = -1.0f;

	this->SSM_index = -1;				// tag C SSM index, wich entry in the SSM table this weapon calls -Bobboau

	this->field_of_fire = 0.0f;
	this->fof_spread_rate = 0.0f;
	this->fof_reset_rate = 0.0f;
	this->max_fof_spread = 0.0f;

	this->shots = 1;

	this->alpha_max = 1.0f;
	this->alpha_min = 0.0f;
	this->alpha_cycle = 0.0f;

	shockwave_create_info_init(&this->shockwave);
	shockwave_create_info_init(&this->dinky_shockwave);

	this->weapon_hitpoints = 0;

	this->burst_delay = 1.0f; // 1 second, just incase its not defined
	this->burst_shots = 0;
    this->burst_flags.reset();

	generic_anim_init(&this->thruster_flame);
	generic_anim_init(&this->thruster_glow);

	this->thruster_glow_factor = 1.0f;
	this->target_lead_scaler = 0.0f;

	this->failure_rate = 0.0f;
	this->failure_sub_name.clear();
	this->failure_sub = -1;

	this->selection_effect = Default_weapon_select_effect;

	this->hud_locked_snd = gamesnd_id();
	this->hud_tracking_snd = gamesnd_id();
	this->hud_in_flight_snd = gamesnd_id();

	// Reset using default constructor
	this->impact_decal = decals::creation_info();
}
const char* weapon_info::get_display_string() {
	if (has_alternate_name()) {
		return alt_name;
	} else {
		return name;
	}
}
bool weapon_info::has_alternate_name() {
	return alt_name[0] != '\0';
}
