/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _DEBRIS_H
#define _DEBRIS_H

#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"
#include "gamesnd/gamesnd.h"

class object;
struct CFILE;
class model_draw_list;

#define MAX_DEBRIS_ARCS 8		// Must be less than MAX_ARC_EFFECTS in model.h

FLAG_LIST(Debris_Flags) {
	Used,
	OnHullDebrisList,
	DoNotExpire,		// This debris piece has been placed in FRED and should not expire automatically

	NUM_VALUES
};


typedef struct debris {
	flagset<Debris_Flags> flags;	// See DEBRIS_??? defines
	int		source_objnum;			// What object this came from
	int		damage_type_idx;		// Damage type of this debris
	int		ship_info_index;		// Ship info index of the ship type debris came from
	int		team;					// Team of the ship where the debris came from
	gamesnd_id ambient_sound;		// Ambient looping sound
	int		objnum = -1;			// What object this is linked to
	float		lifeleft;			// When 0 or less object dies
	int		model_num;				// What model this uses
	int		model_instance_num;		// What model instance this uses - needed for arcs
	int		submodel_num;			// What submodel this uses
	TIMESTAMP	next_fireball;		// When to start a fireball
	bool	is_hull;				// indicates whether this is a collideable, destructable piece of debris from the model, or just a generic debris fragment
	int		species;				// What species this is from.  -1 if don't care.
	TIMESTAMP	fire_timeout;		// timestamp that holds time for fireballs to stop appearing
	TIMESTAMP	sound_delay;		// timestamp to signal when sound should start
	fix		time_started;			// time when debris was created

	vec3d	arc_pts[MAX_DEBRIS_ARCS][2];	// The endpoints of each arc
	TIMESTAMP	arc_timestamp[MAX_DEBRIS_ARCS];	// When this times out, the spark goes away.  Invalid is not used
	int		arc_frequency;					// Starts at 1000, gets bigger

	int		parent_alt_name;
	float	damage_mult;

	// flags and objnum are the only things that are cleared when debris is deleted, so they are the only values initialized here (plus next and prev)
} debris;

#define	SOFT_LIMIT_DEBRIS_PIECES	64
extern	SCP_vector<debris> Debris;

struct collision_info_struct;

void debris_init();
void debris_render(object * obj, model_draw_list *scene);
void debris_delete( object * obj );
void debris_process_post( object * obj, float frame_time);
object *debris_create( object * source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, bool hull_flag, float exp_force );
int debris_check_collision( object * obj, object * other_obj, vec3d * hitpos, collision_info_struct *debris_hit_info=NULL, vec3d* hitnormal = NULL );
void debris_hit( object * debris_obj, object * other_obj, vec3d * hitpos, float damage );
int debris_get_team(object *objp);

void debris_add_to_hull_list(debris *db);
void debris_remove_from_hull_list(debris *db);

bool debris_is_generic(debris *db);
bool debris_is_vaporized(debris *db);

// creates a burst of generic debris at hitpos from ship_objp, with a random number between min and max
// use_ship_debris is for whether the ship's generic debris should be used, or simply debris01.pof
void create_generic_debris(object* ship_objp, vec3d* hitpos, float min_num_debris, float max_num_debris, float speed_mult, bool use_ship_debris);

#endif // _DEBRIS_H
