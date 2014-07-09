/*
 * Created by Mike "Bobboau" Abegg for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */




#include "ship/ship.h"
#include "model/model.h"
#include "model/modelanim.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "network/multi.h"


extern float flFrametime;

SCP_vector<triggered_rotation> Triggered_rotations;

char *Animation_type_names[MAX_TRIGGER_ANIMATION_TYPES] =
{
	"initial",
	"docking-stage-1",
	"docking-stage-2",
	"docking-stage-3",
	"docked",
	"primary-bank",
	"secondary-bank",
	"fighterbay",
	"afterburner",
	"turret-firing",
	"scripted",
	"turret-fired"
};

int model_anim_match_type(char *p)
{	
	int i;

	// standard match
	for (i = 0; i < MAX_TRIGGER_ANIMATION_TYPES; i++) {
		if ( !strnicmp(p, Animation_type_names[i], strlen(Animation_type_names[i])) )
			return i;
	}

	// Goober5000 - misspelling
	if ( !strnicmp(p, "inital", 6) || !strnicmp(p, "\"inital\"", 8) ) {
		Warning(LOCATION, "Spelling error in table file.  Please change \"inital\" to \"initial\".");
		return TRIGGER_TYPE_INITIAL;
	}

	// Goober5000 - deprecation
	if ( !strnicmp(p, "docking", 7) || !strnicmp(p, "\"docking\"", 9) ) {
		mprintf(("The \"docking\" animation type name is deprecated.  Specify \"%s\" instead.\n", Animation_type_names[TRIGGER_TYPE_DOCKING_STAGE_2]));
		return TRIGGER_TYPE_DOCKING_STAGE_2;
	} else if ( !strnicmp(p, "primary_bank", 12) || !strnicmp(p, "\"primary_bank\"", 14) ) {
		mprintf(( "The \"primary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", Animation_type_names[TRIGGER_TYPE_PRIMARY_BANK]));
		return TRIGGER_TYPE_PRIMARY_BANK;
	} else if ( !strnicmp(p, "secondary_bank", 14) || !strnicmp(p, "\"secondary_bank\"", 16) ) {
		mprintf(("The \"secondary_bank\" animation type name is deprecated.  Specify \"%s\" instead.\n", Animation_type_names[TRIGGER_TYPE_SECONDARY_BANK]));
		return TRIGGER_TYPE_SECONDARY_BANK;
	} else if ( !strnicmp(p, "door", 4) || !strnicmp(p, "\"door\"", 6) ) {
		mprintf(("The \"door\" animation type name is deprecated.  Specify \"%s\" instead.\n", Animation_type_names[TRIGGER_TYPE_DOCK_BAY_DOOR]));
		return TRIGGER_TYPE_DOCK_BAY_DOOR;
	} else if ( !strnicmp(p, "turret firing", 13) || !strnicmp(p, "\"turret firing\"", 15) ) {
		mprintf(("The \"turret firing\" animation type name is deprecated.  Specify \"%s\" instead.\n", Animation_type_names[TRIGGER_TYPE_TURRET_FIRING]));
		return TRIGGER_TYPE_TURRET_FIRING;
	}

	// Goober5000 - with quotes
	for (i = 0; i < MAX_TRIGGER_ANIMATION_TYPES; i++) {
		char quoted_name[NAME_LENGTH + 2];
		strcpy(quoted_name, "\"");
		strcat(quoted_name, Animation_type_names[i]);
		strcat(quoted_name, "\"");

		if ( !strnicmp(p, quoted_name, strlen(quoted_name)) ) {
			mprintf(( "Old usage warning: Please remove quotes from animation type %s.\n", quoted_name));
			return i;
		}
	}

	return TRIGGER_TYPE_NONE;
}


/**
 * Calculates the angle at wich the rotation should start to slow down
 * and fills in a bunch of other crap. If anyone wants the calculus behind 
 * these numbers I'll provide it.
 */
void triggered_rotation::start(queued_animation *q)
{
	instance = q->instance;

	nprintf(("ModelAnim", "Starting animation type %i at %i ...\n", q->type, timestamp()));

	current_snd = -2;
	current_snd_index = start_sound = q->start_sound;
	loop_sound = q->loop_sound;
	end_sound = q->end_sound;
	snd_rad = q->snd_rad;

	for (int axis = 0; axis < 3; axis++) {
		direction.a1d[axis] = (end_angle.a1d[axis] + q->angle.a1d[axis]) - current_ang.a1d[axis];

		if (direction.a1d[axis])
			direction.a1d[axis] /= fabs(direction.a1d[axis]);

		if (!q->absolute) {
			end_angle.a1d[axis] = q->angle.a1d[axis] + end_angle.a1d[axis];
		}

		rot_vel.a1d[axis] = q->vel.a1d[axis] * direction.a1d[axis];
		rot_accel.a1d[axis] = q->accel.a1d[axis] * direction.a1d[axis];

		if (q->accel.a1d[axis] == 0.0f)
			slow_angle.a1d[axis] = end_angle.a1d[axis];
		else
			slow_angle.a1d[axis] = end_angle.a1d[axis] + (((q->vel.a1d[axis] * q->vel.a1d[axis]) / (2.0f * q->accel.a1d[axis])) * direction.a1d[axis]);
	}

	nprintf(("ModelAnim", "Dir=[%f, %f, %f], End=[%f, %f, %f], Vel=[%f, %f, %f], Accel=[%f, %f, %f], Slow=[%f, %f, %f]\n", direction.a1d[0], direction.a1d[1], direction.a1d[2],
				end_angle.a1d[0], end_angle.a1d[1], end_angle.a1d[2], rot_vel.a1d[0], rot_vel.a1d[1], rot_vel.a1d[2], rot_accel.a1d[0], rot_accel.a1d[1], rot_accel.a1d[2],
				slow_angle.a1d[0], slow_angle.a1d[1], slow_angle.a1d[2]));

	has_started = true;
	end_time = q->end_time;
}

void triggered_rotation::apply_trigger_angles(angles *submodel_angles)
{
	// Vasudan Admiral - And now actually APPLY the angle data to the subobjects themselves!
	submodel_angles->p = current_ang.xyz.x;
	submodel_angles->h = current_ang.xyz.y;
	submodel_angles->b = current_ang.xyz.z;

	if (submodel_angles->p >= PI2)
		submodel_angles->p -= PI2;
	else if (submodel_angles->p < 0.0f)
		submodel_angles->p += PI2;

	if (submodel_angles->h >= PI2)
		submodel_angles->h -= PI2;
	else if (submodel_angles->h < 0.0f)
		submodel_angles->h += PI2;

	if (submodel_angles->b >= PI2)
		submodel_angles->b -= PI2;
	else if (submodel_angles->b < 0.0f)
		submodel_angles->b += PI2;
}

void triggered_rotation::set_to_initial(queued_animation *q)
{
	for (int axis = 0; axis < 3; axis++)
	{
		current_ang.a1d[axis] = q->angle.a1d[axis];
	}
}

void triggered_rotation::set_to_final(queued_animation *q)
{
	for (int axis = 0; axis < 3; axis++)
	{
		current_ang.a1d[axis] = q->angle.a1d[axis];
		end_angle.a1d[axis] = current_ang.a1d[axis];
	}
}

triggered_rotation::triggered_rotation()
{
	clear();
}

void triggered_rotation::clear()
{
	for (int i = 0; i < MAX_TRIGGERED_ANIMATIONS; i++)
	{
		queued_animation_init(&queue[i]);
		queued_animation_init(&queue_tmp[i]);
	}

	current_ang = vmd_zero_vector;
	current_vel = vmd_zero_vector;
	rot_accel = vmd_zero_vector;
	rot_vel = vmd_zero_vector;
	slow_angle = vmd_zero_vector;
	start_time = 0;
	end_time = 0;
	end_angle = vmd_zero_vector;
	n_queue = 0;
	instance = -1;
	has_started = false;
	start_sound = -1;
	loop_sound = -1;
	end_sound = -1;
	current_snd = -1;
	current_snd_index = -1;
	snd_rad = 0.0;
	obj_num = -1;
}

triggered_rotation::~triggered_rotation()
{
}

void triggered_rotation::add_queue(queued_animation *the_queue, int dir)
{
	int i;
	queued_animation new_queue;
	memcpy( &new_queue, the_queue, sizeof(queued_animation) );

	if (dir == -1) {
		new_queue.start = new_queue.reverse_start;
		vm_vec_negate( &new_queue.angle );
	}

	memcpy(queue_tmp, queue, sizeof(queued_animation) * MAX_TRIGGERED_ANIMATIONS);

	if (n_queue > 0) {
		//remove any items on the queue that are the opposite of what we are thinking about doing
		// if we are reversing an animation see if the forward animation is on the queue already, and remove it
		for (i = 0; i < n_queue && (i < MAX_TRIGGERED_ANIMATIONS); i++) {
			if ( (new_queue.type == queue_tmp[i].type) && (new_queue.subtype == queue_tmp[i].subtype) ) {
				// same type, if they have the same values (direction reversed) then replace it
				if (new_queue.instance == queue_tmp[i].instance)
					break;
			}
		}

		if (i != n_queue) {
			// replace if it's not the last item on the list
			if ( i < (MAX_TRIGGERED_ANIMATIONS-1) )
				memcpy( &queue_tmp[i], &queue_tmp[i+1], sizeof(queued_animation) * (MAX_TRIGGERED_ANIMATIONS-(i+1)) );

			// ok these two animations cancelled each other out, so he doesn't get on the queue
			n_queue--;

			return;
		}
	}

	if (new_queue.start == 0) {
		new_queue.start_time = timestamp();
		new_queue.end_time = timestamp( new_queue.end );

		// if there is no delay don't bother with the queue, just start the thing
		start( &new_queue );

		return;
	}

	if (new_queue.instance == instance) {
		// same animation is playing that we are about to think about playing some point in the future
		if ( (direction.xyz.x * rot_vel.xyz.x) == new_queue.vel.xyz.x &&
				(direction.xyz.y * rot_vel.xyz.y) == new_queue.vel.xyz.y &&
				(direction.xyz.z * rot_vel.xyz.z) == new_queue.vel.xyz.z)
		{
			// they're going in opposite directions, one of them is a reversal!
			// so this means that there is some sort of delay that's getting fubared becase of other queue items getting removed due to reversal
			// this animation needs to be started now!
			new_queue.start_time = timestamp();
			new_queue.end_time = timestamp( new_queue.end );

			// if there is no delay don't bother with the queue, just start the thing
			start( &new_queue );

			return;
		}
	}

	Assert( new_queue.start >= 0 );

	// starts that many milliseconds from now
	new_queue.start_time = timestamp( new_queue.start );

	// runs for that long
	new_queue.end_time = timestamp( new_queue.start + new_queue.end );

	if (n_queue > 0) {
		// if we already have something in the queue find the first item on the 
		// queue that is going to start after the new item, 
		for (i = 0; (i < n_queue) && (i < MAX_TRIGGERED_ANIMATIONS) && (new_queue.start_time > queue_tmp[i].start_time); i++);

		if (i >= MAX_TRIGGERED_ANIMATIONS)
			return;

		// then insert the new item before that item
		// from the begining of the queue to the item on the queue that is just before the new item
		if (i)
			memcpy( queue, queue_tmp, sizeof(queued_animation) * i );

		// if there are any items after, copy them from the original queue
		if ( (n_queue >= (i+1)) && (i < (MAX_TRIGGERED_ANIMATIONS - 1)) ) {
			if (n_queue >= MAX_TRIGGERED_ANIMATIONS) {
				// if the queue is full, we don't want to copy past the end of it
				memcpy(&queue[i + 1], &queue_tmp[i], sizeof(queued_animation) * (MAX_TRIGGERED_ANIMATIONS - i - 1));
			} else {
			memcpy( &queue[i+1], &queue_tmp[i], sizeof(queued_animation) * (n_queue - i) );
			}
		}

		// add the new item
		queue[i] = new_queue;
	} else {
		queue[0] = new_queue;
	}

	n_queue++;
}

/**
 * Look at the queue and see if any of the items on it need to be started
 * remove items from the queue that you just executed
 */
void triggered_rotation::process_queue()
{
	int i;

	// if there is nothing to process then bail
	if ( !n_queue )
		return;

	// all items on the queue are in chronological order (or at least they should be)
	// so execute all items who's starting timestamps are less than the current time
	for (i = 0; (i < n_queue) && timestamp_elapsed(queue[i].start_time); i++)
		start( &queue[i] );

	// if no items were processed then bail
	if ( !i )
		return;

	// all the triggered animations associated with this object
	memcpy( queue_tmp, queue, sizeof(queued_animation) * MAX_TRIGGERED_ANIMATIONS );

	// if there are more items on the queue than we just executed reallocate the queue.
	// copy all the items after the last one we executed
	memcpy( queue, &queue_tmp[i], sizeof(queued_animation) * (n_queue-i) );

	// then erase the old queue
	n_queue -= i;

	queue[n_queue].start_time = 0;
	queue[n_queue].end_time = 0;
}

void queued_animation_init(queued_animation *qa)
{
	qa->angle = vmd_zero_vector;
	qa->vel = vmd_zero_vector;
	qa->accel = vmd_zero_vector;

	qa->start = 0;
	qa->end = 0;
	qa->type = TRIGGER_TYPE_NONE;
	qa->subtype = ANIMATION_SUBTYPE_ALL;

	qa->absolute = false;
	qa->reverse_start = -1;
	qa->instance = -1;
	qa->real_end_time = 0;

	qa->start_sound = -1;
	qa->loop_sound = -1;
	qa->end_sound = -1;
	qa->snd_rad = 0.0f;
}

void queued_animation_correct(queued_animation *qa)
{
	for (int i = 0; i < 3; i++) {
		if ( qa->accel.a1d[i] == 0.0f )
			continue;

		if ( ((qa->vel.a1d[i] * qa->vel.a1d[i]) / qa->accel.a1d[i]) > fabs(qa->angle.a1d[i]) )
			qa->vel.a1d[i] = fl_sqrt( fabs(qa->accel.a1d[i] * qa->angle.a1d[i]) );
	}
}



//************************************//
//*** triggered submodel animation ***//
//************************************//


/*
ok a triggered animation works like this, at some point a subobject will be triggered to rotate 
when this happens the following phases of rotation will happen
1) it will accelerate at a constant rate until it reaches a 
quasi-arbitrary (there are limitations on what it can be) velocity
2) it will maintain a constant rotational velocity untill it reaches the angle at which it 
needs to start slowing down in order to stop at the right end angle
3)it will slow down at the same rate it has sped up earlier, when the rotational velocity
starts going in the wrong direction it'll be locked at 0 and the angle of the submodel will 
be locked at the angle it's suposed to end at
*/
//-Bobboau

void model_anim_submodel_trigger_rotate(model_subsystem *psub, ship_subsys *ss)
{
	Assert( psub != NULL );
	Assert( ss != NULL );
	Assert( psub->flags & MSS_FLAG_TRIGGERED );
	Assert( ss->triggered_rotation_index >= 0 );

	triggered_rotation *trigger = &Triggered_rotations[ss->triggered_rotation_index];
	submodel_instance_info *sii = &ss->submodel_info_1;	
	int not_moving_count = 0;

	if ( psub->model_num < 0 ) {
		Int3();
		return;
	}

	if ( psub->subobj_num < 0 ) {
		Int3();
		return;
	}

	if ( !trigger->has_started )
		return;

	// save last angles
	sii->prev_angs = sii->angs;

	// process velocity and position
	// first you accelerate, then you maintain a speed, then you slow down, then you stay put
	for (int i = 0; i < 3; i++) {
		// are we moving?
		if ( (trigger->current_vel.a1d[i] != 0.0f) || ((trigger->current_ang.a1d[i] * trigger->direction.a1d[i]) <= (trigger->slow_angle.a1d[i] * trigger->direction.a1d[i])) ) {
			// yep...

			// our velocity is something other than 0 or we are in the acceleration phase (where velocity starts out at 0)

			// while you are not slowing down...
			if ( (trigger->current_ang.a1d[i] * trigger->direction.a1d[i]) <= (trigger->slow_angle.a1d[i] * trigger->direction.a1d[i]) ) {
				// speeding up
				if ( (trigger->current_vel.a1d[i] * trigger->direction.a1d[i]) < (trigger->rot_vel.a1d[i] * trigger->direction.a1d[i]) && (trigger->rot_accel.a1d[i] != 0.0f) ) {
					trigger->current_vel.a1d[i] += (trigger->rot_accel.a1d[i] * flFrametime);
				}
				// have reached target speed
				else {
					trigger->current_vel.a1d[i] = trigger->rot_vel.a1d[i];
				}
			}
			// we are slowing down...
			else {
				// velocity matches desired heading
				if ( ((trigger->current_vel.a1d[i] * trigger->direction.a1d[i]) > 0) && (trigger->rot_accel.a1d[i] != 0.0f) ) {
					trigger->current_vel.a1d[i] -= (trigger->rot_accel.a1d[i] * flFrametime);
				}
				// velocity does not match desired heading
				else {
					// this can happen if we have decelerated too long or if an animation was reversed quickly
					// the way to tell the difference between these two cases is the acceleration

					// if the curent velocity is in the opposite direction as the acceleration then it was interupted
					if ( (trigger->current_vel.a1d[i] / fabs(trigger->current_vel.a1d[i])) != (trigger->rot_accel.a1d[i] / fabs(trigger->rot_accel.a1d[i])) ) {
						// this is gona be some messy stuff in here to figure out when it should start to slow down again
						// it'll have to make a new slow angle I guess
						// with an initial v in the opposite direction the time it will take for it to stop
						// will be v/a, to get back up to the same speed again we will need twice that 
						// it should be back to where it was in terms of both speed and position then
						// so...
						trigger->slow_angle.a1d[i] = trigger->current_ang.a1d[i];
						trigger->rot_vel.a1d[i] = -(trigger->current_vel.a1d[i]);

						// I guess that wasn't so messy after all :D

						// it might hit exactly 0 every now and then, but it will be before the slow angle so it will be fine
						// this assumes that the reversed animation is the same exact animation only played in reverse, 
						// if the speeds or accelerations are diferent then it might not work
					} else {
						// our velocity has gone in the opposite direction because we decelerated too long
						trigger->current_vel.a1d[i] = 0.0f;
					}
				}
			}

			// if we've over shot the angle, this shouldn't happen but it might if odd values are given
			if ( (trigger->current_ang.a1d[i] * trigger->direction.a1d[i]) >= (trigger->end_angle.a1d[i] * trigger->direction.a1d[i]) ) {
				trigger->current_ang.a1d[i] = trigger->end_angle.a1d[i];
				trigger->current_vel.a1d[i] = 0.0f;
				not_moving_count++;
			} else {
				trigger->current_ang.a1d[i] += (trigger->current_vel.a1d[i] * flFrametime);
			}
		} else {
			// not moving
			trigger->current_ang.a1d[i] = trigger->end_angle.a1d[i];
			not_moving_count++;
		}
	}

	if (not_moving_count == 3) {
		trigger->has_started = false;
		trigger->instance = -1;
	}

	// objects can be animated along several axes at the same time
	// I'm prety sure using the magnitude of the vectors is at least pretty close for any code that might be using it
	sii->cur_turn_rate = vm_vec_mag(&trigger->current_vel);
	sii->desired_turn_rate = vm_vec_mag(&trigger->rot_vel);
	sii->turn_accel = vm_vec_mag(&trigger->rot_accel);

	// the extra math here is/was useless, it just returns the exact same value (or really just 0 in the old code)
	sii->angs.p = trigger->current_ang.xyz.x; //- (2.0f * PI2 * (trigger->current_ang.xyz.x / (2.0f * PI2)));
	sii->angs.h = trigger->current_ang.xyz.y; //- (2.0f * PI2 * (trigger->current_ang.xyz.y / (2.0f * PI2)));
	sii->angs.b = trigger->current_ang.xyz.z; //- (2.0f * PI2 * (trigger->current_ang.xyz.z / (2.0f * PI2)));

	if (sii->angs.p >= PI2)
		sii->angs.p -= PI2;
	else if (sii->angs.p < 0.0f)
		sii->angs.p += PI2;

	if (sii->angs.h >= PI2)
		sii->angs.h -= PI2;
	else if (sii->angs.h < 0.0f)
		sii->angs.h += PI2;

	if (sii->angs.b >= PI2)
		sii->angs.b -= PI2;
	else if (sii->angs.b < 0.0f)
		sii->angs.b += PI2;
}

//************************************//
//*** ship related animation stuff ***//
//************************************//

// Checks if the given subtype matches a particular animation
bool subtype_check(model_subsystem *psub, queued_animation *anim, int subtype)
{
	Assert( psub != NULL );
	Assert( anim != NULL );

	if ( (subtype == ANIMATION_SUBTYPE_ALL) || (anim->subtype == ANIMATION_SUBTYPE_ALL) )
		return true;

	// Fighterbay door animations can have negative subtypes, so handle those
	// separately here
	if (anim->type == TRIGGER_TYPE_DOCK_BAY_DOOR) {
		int anim_subtype = anim->subtype -1; // in the tables, bay door +sub_types are 1-based so change to 0-based

		if (anim_subtype < 0) {
			if (abs(anim_subtype) != subtype) {
				return true;
			}
		} else {
			if (anim_subtype == subtype) {
				return true;
			}
		}
	} else {
		if ( anim->subtype == subtype )
			return true;
	}

	return false;
}

bool model_anim_start_type(ship_subsys *pss, int animation_type, int subtype, int direction, bool instant)
{
	Assert( pss != NULL );

	if (pss->max_hits > 0 && pss->current_hits <= 0.0f) //allow subobjects with hitpoints disabled -nuke
		return false;

	model_subsystem *psub = pss->system_info;
	bool retval = false;

	if ( !(psub->flags & MSS_FLAG_TRIGGERED) )
		return false;
	Assert(pss->triggered_rotation_index >= 0);
	triggered_rotation *trigger = &Triggered_rotations[pss->triggered_rotation_index];

	for (int i = 0; i < psub->n_triggers; i++) {
		if ( (psub->triggers[i].type == animation_type) && subtype_check(psub, &psub->triggers[i], subtype) ) {
			// rotate instantly; don't use the queue
			if (instant) {
				trigger->set_to_final(&psub->triggers[i]);
				trigger->apply_trigger_angles(&pss->submodel_info_1.angs);

				retval = true;
			}
			// rotate normally
			else {
				psub->triggers[i].instance = i;
				trigger->add_queue(&psub->triggers[i], direction);

				retval = true;
			}
		}
	}

	return retval;
}

bool model_anim_start_type(ship *shipp, int animation_type, int subtype, int direction, bool instant)
{
	// this makes the logic for docking triggers a bit cleaner
	if (shipp == NULL)
		return false;

	ship_subsys	*pss;
	bool retval = false;

	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		bool rc = model_anim_start_type(pss, animation_type, subtype, direction, instant);

		if (rc)
			retval = rc;
	}

	return retval;
}

/**
 * @brief Finds the actual amount of time that motion of an animation type will take to stop, 
 * not for gameplay purposes but for stuff that is involved in coordinating the animation itself.
 *
 * @details The time it takes to speed up or slow down is v/a in this time the animation covers 
 * an angle = to (v^2)/(2a) (for both directions so v^2/a) so wee need the time it takes for the 
 * angle moving at a constant velosity to cover theda - v^2/a
 * v*t = theda - (v^2)/(2*a) => t = -(v^2 - 2*a*theda)/(2*a*v)
 * so finally v/a * 2 - (v^2 - 2*a*theda)/(2*a*v) => (3*v^2 + 2*a*theda)/(2*a*v)
 * time = (3*v^2 + 2*a*theda)/(2*a*v)
 */
int model_anim_instance_get_actual_time(queued_animation *properties)
{
	int ret = 0;
	int temp = 0;

	for (int a = 0; a < 3; a++) {
		temp = fl2i( ((3.0f * properties->vel.a1d[a] * properties->vel.a1d[a]) + (2.0f * properties->accel.a1d[a] * fabs(properties->angle.a1d[a])))
						/ (2.0f * properties->accel.a1d[a] * properties->vel.a1d[a]) * 1000.0f )
						+ properties->start;

		if (temp > ret)
			ret = temp;
	}

	return ret;
}

int model_anim_get_actual_time_type(ship *shipp, int animation_type, int subtype)
{
	ship_subsys	*pss;
	model_subsystem	*psub;
	int ret = 0;
	int temp_ret = 0;
	int i;

	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects (but allow subobjects with hitpoints disabled -nuke)
		if ( pss->max_hits > 0 && pss->current_hits <= 0.0f ) 
			continue;

		// not a triggered animation, skip it
		if ( !(psub->flags & MSS_FLAG_TRIGGERED) )
			continue;

		for (i = 0; i < psub->n_triggers; i++) {
			if ( (psub->triggers[i].type == animation_type) && subtype_check(psub, &psub->triggers[i], subtype) ) {
				temp_ret = model_anim_instance_get_actual_time(&psub->triggers[i]);

				if (temp_ret > ret)
					ret = temp_ret;
			}
		}
	}

	return ret;
}

int model_anim_get_actual_time_type(ship_info *sip, int animation_type, int subtype)
{
	model_subsystem *psub;
	int ret = 0;
	int temp_ret = 0;
	int n, i;

	for (n = 0; n < sip->n_subsystems; n++) {
		psub = &sip->subsystems[n];

		for (i = 0; i < psub->n_triggers; i++) {
			if ( (psub->triggers[i].type == animation_type) && subtype_check(psub, &psub->triggers[i], subtype) ) {
				temp_ret = model_anim_instance_get_actual_time(&psub->triggers[i]);

				if (temp_ret > ret)
					ret = temp_ret;
			}
		}
	}

	return ret;
}

void model_anim_fix_reverse_times(ship_info *sip)
{
	model_subsystem *psub;
	int i, j;
	int ani_time = 0;
	int type = 0;

	for (type = 0; type < MAX_TRIGGER_ANIMATION_TYPES; type++) {
		// figure out how long it's going to take for the animation to complete
		ani_time = model_anim_get_actual_time_type(sip, type, ANIMATION_SUBTYPE_ALL);

		for (i = 0; i < sip->n_subsystems; i++) {
			psub = &sip->subsystems[i];

			for (j = 0; j < psub->n_triggers; j++) {
				if (psub->triggers[j].type == type) {
					// if there isn't a user defined overide already present
					if (psub->triggers[j].reverse_start == -1)
						psub->triggers[j].reverse_start = ani_time - model_anim_instance_get_actual_time(&psub->triggers[j]);

					if (psub->triggers[j].reverse_start < 0) {
						mprintf(("WARNING:  Animation trigger #%i on subsystem '%s', for ship '%s', has a negative reverse_start value!  Capping it at 0!\n", j, psub->subobj_name, sip->name));
						psub->triggers[j].reverse_start = 0;
					}

					psub->triggers[j].real_end_time = model_anim_instance_get_actual_time(&psub->triggers[j]);
				}
			}
		}
	}
}

/**
 * Needs to always return a valid timestamp, even if it's just the current one
 */
int model_anim_get_time_type(ship_subsys *pss, int animation_type, int subtype)
{
	Assert( pss != NULL );

	if (pss->max_hits > 0 && pss->current_hits <= 0.0f) //allow subobjects with hitpoints disabled -nuke
		return timestamp();

	model_subsystem *psub = pss->system_info;
	int i, ret = 0;

	if ( !(psub->flags & MSS_FLAG_TRIGGERED) )
		return timestamp();
	Assert(pss->triggered_rotation_index >= 0);
	triggered_rotation *tr = &Triggered_rotations[pss->triggered_rotation_index];

	for (i = 0; i < psub->n_triggers; i++) {
		if ( (psub->triggers[i].type == animation_type) &&
			((psub->triggers[i].subtype == ANIMATION_SUBTYPE_ALL) || (psub->triggers[i].subtype == subtype)) )
		{
			int ani_time = 0;

			if ( (tr->current_vel.a1d[0] != 0.0f) || (tr->current_vel.a1d[1] != 0.0f) || (tr->current_vel.a1d[2] != 0.0f)) {
				// if the subobject is moving then things get really complicated
				int a_time = 0;
				int real_time = model_anim_instance_get_actual_time(&psub->triggers[i]);
				int pad = real_time - psub->triggers[i].end;

				for (int a = 0; a < 3; a++) {
					float end_angle = (tr->current_ang.a1d[a]  + (((tr->rot_vel.a1d[a]*tr->rot_vel.a1d[a]) - (tr->current_vel.a1d[a]*tr->current_vel.a1d[a])) / (2*tr->rot_accel.a1d[a])));

					if (end_angle > tr->slow_angle.a1d[a]) {
						//T(total) =  (2V(maximum) - V(initial))/a + (S(turnpoint) - S(initial) + (V(initial)^2 - V(maximum)^2)/2a) / V(maximum)
						a_time = fl2i(((((2*tr->rot_vel.a1d[a]) - tr->current_ang.a1d[a])/tr->rot_accel.a1d[a]) + tr->slow_angle.a1d[a] - tr->current_ang.a1d[a] + (((tr->current_vel.a1d[a]*tr->current_vel.a1d[a]) - (tr->rot_vel.a1d[a]*tr->rot_vel.a1d[a])) / (2*tr->rot_accel.a1d[a])))*1000.0f);
						if (ani_time < a_time)
							ani_time = a_time;
					} else {
						//T(total)  = 2 * sqrt((S(final) - S(initial))/a - ( (V(initial)/a)^2 ) / 2 ) - V(initial)/a
						a_time = fl2i((2 * fl_sqrt(((tr->end_angle.a1d[a] - tr->current_ang.a1d[a])/tr->rot_accel.a1d[a]) - ((tr->current_vel.a1d[a] * tr->current_vel.a1d[a]) / (tr->rot_accel.a1d[a] * tr->rot_accel.a1d[a])) / 2) - (tr->current_vel.a1d[a] / tr->rot_accel.a1d[a]))*1000.0f);
						if (ani_time < a_time)
							ani_time = a_time;
					}
				}

				if (ani_time)
					ani_time += pad;
			} else {
				// if it isn't moving then it's trivial.
				// no currently playing animation
				ani_time = psub->triggers[i].end + psub->triggers[i].start;
			}

			if (ret < ani_time)
				ret = ani_time;
		}
	}

	return timestamp(ret);
}

/** 
 * @brief How long an animation is going to take to complete.
 * @details For things that can't happen until animations are done this is for gameplay purposes, this isn't the actual time
 * @param shipp
 * @param animation_type
 * @param subtype
 * @return Should already be a valid timestamp
 */
int model_anim_get_time_type(ship *shipp, int animation_type, int subtype)
{
	ship_subsys	*pss;
	int ani_time = 0, ret = 0;


	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		ani_time = model_anim_get_time_type(pss, animation_type, subtype);

		if (ret < ani_time)
			ret = ani_time;
	}

	return ret;
}

void model_anim_set_initial_states(ship *shipp)
{
	ship_weapon	*swp = &shipp->weapons;
	ship_subsys	*pss;
	model_subsystem	*psub;
	int i;

	for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
		swp->primary_animation_done_time[i] = 0;

	for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++)
		swp->secondary_animation_done_time[i] = 0;

	ship_primary_changed(shipp);
	ship_secondary_changed(shipp);

	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		for (i = 0; i < psub->n_triggers; i++) {
			if (psub->triggers[i].type == TRIGGER_TYPE_INITIAL) {
				if (psub->type == SUBSYSTEM_TURRET) {
					// special case for turrets
					pss->submodel_info_2.angs.p = psub->triggers[i].angle.xyz.x;
					pss->submodel_info_1.angs.h = psub->triggers[i].angle.xyz.y;
				} else {
					Assert(pss->triggered_rotation_index >= 0);
					triggered_rotation *tr = &Triggered_rotations[pss->triggered_rotation_index];

					tr->set_to_initial(&psub->triggers[i]);
					tr->apply_trigger_angles(&pss->submodel_info_1.angs);
				}
			}
		}
	}
}

/**
 * Handles multiplayer-safe, client-side, animations
 */
void model_anim_handle_multiplayer(ship *shipp)
{
	Assert( shipp != NULL );

	ship_subsys *pss;
	model_subsystem *psub;

	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		Int3();
		return;
	}

	if (Net_player->flags & NETINFO_FLAG_AM_MASTER)
		return;

	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects  (but allow subobjects with hitpoints disabled -nuke)
		if ( pss->max_hits > 0 && pss->current_hits <= 0.0f ) 
			continue;

		// not a triggered animation, skip it
		if ( !(psub->flags & MSS_FLAG_TRIGGERED) )
			continue;
		Assert(pss->triggered_rotation_index >= 0);

		for (int i = 0; i < psub->n_triggers; i++) {
			switch (psub->triggers[i].type)
			{
				case TRIGGER_TYPE_PRIMARY_BANK:
				case TRIGGER_TYPE_SECONDARY_BANK:
				case TRIGGER_TYPE_AFTERBURNER:
				{
					Triggered_rotations[pss->triggered_rotation_index].process_queue();
					model_anim_submodel_trigger_rotate(psub, pss );

					break;
				}

				default:
					break;
			}
		}
	}
}

// Goober5000 - stack based animation for reversing a sequence of animations

SCP_map<int, animation_stack> Animation_map;

bool model_anim_push_and_start_type(int stack_unique_id, ship *shipp, int animation_type, int subtype, int direction, bool instant)
{
	stack_item item;
	item.shipp = shipp;
	item.animation_type = animation_type;
	item.subtype = subtype;
	item.direction = direction;
	item.instant = instant;

	Animation_map[stack_unique_id].push_back(item);

	return model_anim_start_type(shipp, animation_type, subtype, direction, instant);
}

bool model_anim_pop_and_start_type(int stack_unique_id)
{
	animation_stack stack = Animation_map[stack_unique_id];

	if (stack.empty())
		return false;

	stack_item item = stack.back();
	stack.pop_back();

	return model_anim_start_type(item.shipp, item.animation_type, item.subtype, item.direction * -1, item.instant);
}
