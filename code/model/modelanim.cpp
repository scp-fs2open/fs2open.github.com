/*
 * Created by Mike "Bobboau" Abegg for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile:  $
 * $Revision: 1.2 $
 * $Date: 2006-04-20 06:32:15 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/20 05:42:05  Goober5000
 * change animation.cpp to modelanim.cpp, add CVS info, and add the file to the VC6 project
 * --Goober5000
 *
 */


#define MODEL_LIB

#include "ship/ship.h"
#include "model/model.h"
#include "model/modelsinc.h"
#include "io/timer.h"
#include "globalincs/linklist.h"

extern float flFrametime;

//************************************//
//*** triggered submodel animation ***//
//************************************//


/*
ok a triggered animation works like this, at some point a subobject will be triggered to rotate 
when this happens the following phases of rotation will happen
1) it will accelerate at a constant rate untill it reaches a 
quasi-arbitrary (there are limitations on what it can be) velocity
2) it will maintain a constant rotational velocity untill it reaches the angle at wich it 
needs to start slowing down in order to stop a the right end angle
3)it will slow down at the same rate it has sped up earlier, when the rotational velocity
starts going in the wrong direction it'll be locked at 0 and the angle of the submodel will 
be locked at the angle it's suposed to end at
*/
//-Bobboau
void submodel_trigger_rotate(model_subsystem *psub, ship_subsys *ss){
	triggered_rotation *trigger = &ss->trigger;

	submodel_instance_info *sii = &ss->submodel_info_1;

	bsp_info * sm;

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	sm = &pm->submodel[psub->subobj_num];

//	if ( sm->movement_type != MOVEMENT_TYPE_TRIGGERED ) return;
	//one less thing that can go wrong

	// save last angles
	sii->prev_angs = sii->angs;

	float *current_ang;
	float *current_vel;
	float *rot_accel;
	float *rot_vel;
	float *slow_angle;
//	float *end_time;
	float *end_angle;
	float *direction;

	int looping = 0;	
	//process velocity and position
	//first you accelerate, then you maintain a speed, then you slowdown, then you stay put
	int not_moveing_count = 0;
	for(int i = 0; i<3; i++){
		current_ang = &trigger->current_ang.a1d[i];
		current_vel = &trigger->current_vel.a1d[i];
		rot_accel = &trigger->rot_accel.a1d[i];
		rot_vel = &trigger->rot_vel.a1d[i];
		slow_angle = &trigger->slow_angle.a1d[i];
	//	end_time = &trigger->end_time;
		end_angle = &trigger->end_angle.a1d[i];
		direction = &trigger->direction.a1d[i];

			if(*current_vel != 0.0f || *current_ang*(*direction) <= *slow_angle*(*direction)){
				//our velocity is something other than 0 or we are in the acceleration phase (were velocity starts out at 0)

				//we are moveing
				if(*current_ang*(*direction) <= *slow_angle*(*direction)){
					//while  you are not slowing down

					if(*current_vel*(*direction) < *rot_vel*(*direction) && *rot_accel!=0.0f){
						//while you are speeding up

						*current_vel += *rot_accel*flFrametime;

					}else {
						//when you have reached the target speed
						looping = 1;

						*current_vel = *rot_vel;
					}
				}else{
					//we are slowing down

					if(*current_vel*(*direction) > 0 && *rot_accel!=0.0f){
						//while our velocity is still in the direction we are suposed to be moveing

						*current_vel -= *rot_accel*flFrametime;
					}else {
						//our velocity is in the wrong direction
						//this can happen if we have decelerated too long
						//or if an animation was reversed quickly
						//the way to tell the diference between these two cases is the acceleration

						//if the curent velocityis in the opposite direction as the accelleration then it was interupted
						if(*current_vel / fabs(*current_vel) != *rot_accel / fabs(*rot_accel) ){
							//this is gona be some messy stuff in here to figure out when it should start to slow down again
							//it'll have to make a new slow angle I guess
							//with an initial v in the oposite direction the time it will take for it to stop
							//will be v/a, to get back up to the same speed again we will need twice that 
							//it should be back to were it was in terms of both speed and position then
							//so...
							*slow_angle = *current_ang;
							*rot_vel = -*current_vel;

							//I guess that wasn't so messy after all :D

							//it might hit exactly 0 every now and then, but it will be before the slow angle so it will be fine
							//this assumes that the reversed animation is the same exact animation only played in reverse, 
							//if the speeds or accelerations are diferent then might not work

						}else{
						//our velocity has gone in the opposite direction becase we decelerated too long

							*current_vel=0.0f;
						}
					}
				}

				//we are moveing

				if(*current_ang*(*direction) > *end_angle*(*direction)){
					//if we've over shot the angle, this shouldn't happen but it might if odd values are given
					*current_ang = *end_angle;
					*current_vel=0.0f;
					not_moveing_count++;
				}else{
					*current_ang += (*current_vel)*flFrametime;
				}
			}else{
					//not moveing
					*current_ang=*end_angle;
					not_moveing_count++;
			}

	}
	if(not_moveing_count == 3){
		trigger->instance = -1;
	}

	//objects can be animated along sevral axes at the same time
	//I'm prety sure useing the magnatude of the vectors is at least prety close for any code that might be useing it
	sii->cur_turn_rate = vm_vec_mag(&trigger->current_vel);
	sii->desired_turn_rate = vm_vec_mag(&trigger->rot_vel);
	sii->turn_accel = vm_vec_mag(&trigger->rot_accel);

//	case MOVEMENT_AXIS_X:	
			sii->angs.p = trigger->current_ang.xyz.x - (2*PI2*(float)int(trigger->current_ang.xyz.x/(2*PI2)));

//	case MOVEMENT_AXIS_Y:	
			sii->angs.h = trigger->current_ang.xyz.y - (2*PI2*(float)int(trigger->current_ang.xyz.y/(2*PI2)));

//	case MOVEMENT_AXIS_Z:	
			sii->angs.b = trigger->current_ang.xyz.z - (2*PI2*(float)int(trigger->current_ang.xyz.z/(2*PI2)));


}

char* animation_type_names[MAX_TRIGGER_ANIMATION_TYPES] =
{
	"initial",
	"docking",
	"docked",
	"primary_bank",
	"secondary_bank",
	"door",
	"afterburner",
	"turret firing",
	"scripted",
};


//this calculates the angle at wich the rotation should start to slow down
//and basicly fills in a bunch of other crap
//if anyone wants the calculus behind these numbers I'll provide it
void triggered_rotation::start(queued_animation* q){
	start_time = timestamp();
	instance = q->instance;

#ifndef NDEBUG
	mprintf(("animation start at %d\n",timestamp()));
	char ax[]="xyz";
#endif

//	vec3d sp;
//	vm_vec_rotate(&sp,&snd_pnt,&Objects[obj_num].orient);
//	vm_vec_add2(&sp, &Objects[obj_num].pos);
//	if(q->start_sound != -1)current_snd = snd_play_3d(&Snds[q->start_sound], &sp, &View_position, q->snd_rad) ;

	current_snd = -2;
	current_snd_index = start_sound = q->start_sound;
	loop_sound = q->loop_sound;
	end_sound = q->end_sound;
	snd_rad = q->snd_rad;

	for(int axis = 0; axis < 3; axis++){

		direction.a1d[axis] = (end_angle.a1d[axis]+q->angle.a1d[axis])-current_ang.a1d[axis];
		if(direction.a1d[axis])direction.a1d[axis] /= (float)fabs(direction.a1d[axis]);

		if(q->absolute){
			end_angle.a1d[axis] = end_angle.a1d[axis] - (2*PI2*(float)int(end_angle.a1d[axis]/(2*PI2)));
			current_ang.a1d[axis] = current_ang.a1d[axis] - (2*PI2*(float)int(current_ang.a1d[axis]/(2*PI2)));
		}else{
			end_angle.a1d[axis]=q->angle.a1d[axis]+end_angle.a1d[axis];
		}
	
		rot_vel.a1d[axis] = q->vel.a1d[axis]*direction.a1d[axis];
		rot_accel.a1d[axis] = q->accel.a1d[axis]*direction.a1d[axis];
	
		slow_angle.a1d[axis]=end_angle.a1d[axis]-(((q->vel.a1d[axis]*q->vel.a1d[axis])/(2.0f*q->accel.a1d[axis]))*direction.a1d[axis]);

#ifndef NDEBUG
		mprintf(("axis %c: direction=%d, end angle=%f, velocity=%f, acceleration=%f, slow angle=%f\n", ax[axis],(int)direction.a1d[axis],end_angle.a1d[axis],rot_vel.a1d[axis],rot_accel.a1d[axis],slow_angle.a1d[axis]));
#endif
	}
}

void triggered_rotation::set_to_end(queued_animation* q){
	for(int axis = 0; axis < 3; axis++){
		current_ang.a1d[axis] += q->angle.a1d[axis];
	}
}

triggered_rotation::triggered_rotation(){
	vec3d v = ZERO_VECTOR;
	current_ang = v;	
	current_vel = v;	
	rot_accel = v;	
	rot_vel = v;	
	slow_angle = v;	
	end_time = 0;	
	end_angle = v;
	n_queue = 0;
	instance = -1;
}

triggered_rotation::~triggered_rotation(){
	//origonaly the que was a dynamic array
}



void triggered_rotation::add_queue(queued_animation* the_queue, int direction){
	static queued_animation scrap_queue;
	scrap_queue = *the_queue;

	queued_animation* new_queue = &scrap_queue;

	if(direction == -1)new_queue->start = new_queue->reverse_start;

	if(direction == -1){
		scrap_queue.angle.xyz.x *= direction;
		scrap_queue.angle.xyz.y *= direction;
		scrap_queue.angle.xyz.z *= direction;
	}

	queued_animation old[MAX_TRIGGERED_ANIMATIONS];		//all the triggered animations assosiated with this object

	memcpy(old, queue, sizeof(queued_animation)*MAX_TRIGGERED_ANIMATIONS);

	int i;
	if(n_queue > 0){
		//remove any items on the queue that are the opposite of what we are thinking about doing
//		if(direction = -1){
			//if we are reverseing an animation see if the forward animation is on the queue already
			//if it is remove it
			for(i = 0; i < n_queue && i < MAX_TRIGGERED_ANIMATIONS; i++){
				if(new_queue->type == old[i].type && new_queue->subtype == old[i].subtype){
					//same type, if they have the same values (direction reversed) this bitch is gone!
					if(new_queue->instance == old[i].instance){
						//that's it that's everything is the same
						break;
					}
				}
			}
			if(i!=n_queue){
				//we've got to remove item number i
				if(i!=MAX_TRIGGERED_ANIMATIONS-1)
					//if it's not the last item on the list
					//copy everything after it over top of it
					memcpy(&old[i], &old[i+1], sizeof(queued_animation)*(MAX_TRIGGERED_ANIMATIONS-(i+1)));
				n_queue--;
				//ok these two animations basicly caceled each other out, 
				//so he doesn't get on the queue
				return;
			}
	//	}
	}

	if(new_queue->start == 0){
		new_queue->start += timestamp();
		new_queue->end += new_queue->start;
		start(new_queue);	//if there is no delay don't bother with the queue, just start the thing
		return;
	}

	if(new_queue->instance == instance){
		//same animation is playing that we are about to think about playing some point in the future
		if(this->direction.xyz.x * rot_vel.xyz.x == new_queue->vel.xyz.x && this->direction.xyz.y * rot_vel.xyz.y == new_queue->vel.xyz.y && this->direction.xyz.z * rot_vel.xyz.z == new_queue->vel.xyz.z){
			//there going in opposite directions
			//one of them is a reversal!
			//so this means thata there is some sort of delay that's getting fubared becase of other queue items getting removed due to reversal
			//this animation needs to be started now!
			new_queue->start =  start_time + new_queue->real_end_time - timestamp();
			new_queue->end += new_queue->start;
			start(new_queue);	//if there is no delay don't bother with the queue, just start the thing
			return;
		}
	}

	//starts that many seconds from now
	new_queue->start += timestamp();
	//runs for that long
	new_queue->end += new_queue->start;


	if(n_queue > 0){
		int i;
		//if we already have something in the queue find the first item on the 
		//queue that is going to start after the new item, 
		for(i = 0; new_queue->start > old[i].start && i < n_queue && i < MAX_TRIGGERED_ANIMATIONS; i++);
		if(i >= MAX_TRIGGERED_ANIMATIONS)return;
		//then incert the new item before that item
		//from the begining of the queue to the item on the queue that is just before the new item
		if(i)memcpy(queue, old, sizeof(queued_animation)*(i));
		//if there are any items after copy them from the origonal queue
		if(n_queue >= i+1)
			memcpy(&queue[i+1], &old[i], sizeof(queued_animation)*(n_queue - i));
		//add the new item
		queue[i] = *new_queue;
	}else{
		queue[0] = *new_queue;
	}
	n_queue++;
}

//look at the queue and see if any of the items on it need to be started
//remove items from the queue that you just executed
void triggered_rotation::proces_queue(){
	int i;

	if(!n_queue)return;
	//if there is nothing on the queue just quit right now

	//all items on the que are in cronological order (or at least they should be)
	//so execute all items who's starting timestamps are less than the current time
	for(i = 0; queue[i].start<=timestamp() && i<n_queue; i++){
		start(&queue[i]);
	}
	//if no items were procesed quit
	if(!i)return;

	queued_animation old[MAX_TRIGGERED_ANIMATIONS];		//all the triggered animations assosiated with this object

	memcpy(old, queue, sizeof(queued_animation)*MAX_TRIGGERED_ANIMATIONS);

//	if(n_queue > i){

		//if there are more items on the queue than we just executed reallocate the queue 
		//copy all the items after the last one we executed
		memcpy(queue, &old[i], sizeof(queued_animation)*(n_queue-i));
//	}
	//then erase the old queue
	n_queue-=i;
	queue[n_queue].start = -1;
}


queued_animation::queued_animation(float an,float v,float a,int s, int e, int t, int st, int axis)
:start(s),end(e),absolute(false),type(t),subtype(st){
	angle.a1d[axis]=an; 
	angle.a1d[(axis+1)%3]=0; 
	angle.a1d[(axis+2)%3]=0; 
	vel.a1d[axis]=v; 
	vel.a1d[(axis+1)%3]=0; 
	vel.a1d[(axis+2)%3]=0; 
	accel.a1d[axis]=a; 
	accel.a1d[(axis+1)%3]=0; 
	accel.a1d[(axis+2)%3]=0;
}

void queued_animation::corect(){
	for(int i = 0; i<3; i++)
		if((vel.a1d[i]*vel.a1d[i])/accel.a1d[i] > fabs(angle.a1d[i]))
			vel.a1d[i] = (float)sqrt(fabs(accel.a1d[i] * angle.a1d[i]));
}



int match_animation_type(char *p)
{	
	int i;

	// standard match
	for(i = 0; i < MAX_TRIGGER_ANIMATION_TYPES; i++)
	{
		if (!strnicmp(p, animation_type_names[i], strlen(animation_type_names[i])))
			return i;
	}

	// Goober5000 - misspelling
	if (!strnicmp(p, "inital", 6) || !strnicmp(p, "\"inital\"", 8))
	{
		Warning(LOCATION, "Spelling error in table file.  Please change \"inital\" to \"initial\".");
		return TRIGGER_TYPE_INITIAL;
	}

	// Goober5000 - with quotes
	for(i = 0; i < MAX_TRIGGER_ANIMATION_TYPES; i++)
	{
		char name[NAME_LENGTH];

		strcpy(name, "\"");
		strcat(name, animation_type_names[i]);
		strcat(name, "\"");

		if (!strnicmp(p, name, strlen(name)))
		{
			Warning(LOCATION, "Old usage warning: Please remove quotes from animation type %s.", name);
			return i;
		}
	}

	return -1;
}







//************************************//
//*** ship related animation stuff ***//
//************************************//


void ship_start_animation_type(ship *shipp, int animation_type, int subtype, int direction){

	ship_subsys	*pss;
	model_subsystem	*psub;
	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects
		if ( pss->current_hits <= 0.0f ) 
			continue;
		if(psub->flags & MSS_FLAG_TRIGGERED){
			for(int i = 0; i<psub->n_triggers; i++){
				if(psub->triggers[i].type == animation_type && (psub->triggers[i].subtype == ANIMATION_SUBTYPE_ALL || psub->triggers[i].subtype == subtype)){
/*					queued_animation var =psub->triggers[i];
					var.angle.xyz.x *= direction;
					var.angle.xyz.y *= direction;
					var.angle.xyz.z *= direction;
					*/
					psub->triggers[i].instance = i;
					pss->trigger.add_queue(&psub->triggers[i], direction);
				}
			}
		}
		//
	}

}

//this finds the actual amount of time that motion of an animation type will take to stop, 
//not for gameplay purposes but for stuff that is involved in coordinating the animation itself

//the time it takes to speed up or slow down is v/a
//in this time the animation covers an angle = to (v^2)/(2a) (for both directions so v^2/a)
//so wee need the time it takes for the angle moveing at a constant velosity to cover theda - v^2/a
//v*t = theda - (v^2)/(2*a) => t = -(v^2 - 2*a*theda)/(2*a*v)
//so finaly v/a * 2 - (v^2 - 2*a*theda)/(2*a*v) => (3*v^2 + 2*a*theda)/(2*a*v)

//time = (3*v^2 + 2*a*theda)/(2*a*v)

int animation_instance_actual_time(queued_animation *properties){
	int ret = 0;
	int temp = 0;
	for(int a = 0; a<3; a++){
		temp = int(
			(3.0f*properties->vel.a1d[a]*properties->vel.a1d[a] + 2.0f*properties->accel.a1d[a]*fabs(properties->angle.a1d[a]))
			/
			(2*properties->accel.a1d[a]*properties->vel.a1d[a]) 
			*1000.0f) + properties->start;
		if(temp > ret)ret = temp;
	}
	return ret;
}

int ship_get_actual_animation_time_type(ship *shipp, int animation_type, int subtype){

	ship_subsys	*pss;
	model_subsystem	*psub;
	int ret = 0;
	int temp_ret = 0;;
	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects
		if ( pss->current_hits <= 0.0f ) 
			continue;
		if(psub->flags & MSS_FLAG_TRIGGERED){
			for(int i = 0; i<psub->n_triggers; i++){
				if(psub->triggers[i].type == animation_type && (psub->triggers[i].subtype == ANIMATION_SUBTYPE_ALL || psub->triggers[i].subtype == subtype)){
					temp_ret = animation_instance_actual_time(&psub->triggers[i]);
					if(temp_ret > ret)ret = temp_ret;
				}
			}
		}
		//
	}
	return ret;

}


int ship_get_actual_animation_time_type(ship_info *sip, int animation_type, int subtype){

	int ret = 0;
	int temp_ret = 0;;
	for ( int s = 0; s<sip->n_subsystems; s++) {

		for(int i = 0; i<sip->subsystems[s].n_triggers; i++){
			if(sip->subsystems[s].triggers[i].type == animation_type && (sip->subsystems[s].triggers[i].subtype == ANIMATION_SUBTYPE_ALL || sip->subsystems[s].triggers[i].subtype == subtype)){
				for(int a = 0; a<3; a++){
					temp_ret = animation_instance_actual_time(&sip->subsystems[s].triggers[i]);
					
					if(temp_ret > ret)ret = temp_ret;
				}
			}
		}
		//
	}
	return ret;

}

void ship_fix_reverse_times(ship_info *sip){

	int ani_time = 0;
	for(int animation_type = 0; animation_type<MAX_TRIGGER_ANIMATION_TYPES; animation_type++){
		//for each animation type
		ani_time = ship_get_actual_animation_time_type(sip, animation_type, -1);
		//figure out how long it's going to take for the animation to get done with

		for ( int s = 0; s<sip->n_subsystems; s++) {
			//for each subsystem

			for(int i = 0; i<sip->subsystems[s].n_triggers; i++){
				//for each trigger
				if(sip->subsystems[s].triggers[i].type == animation_type){
					//if the trigger is of the current type
					if(sip->subsystems[s].triggers[i].reverse_start == -1)
						//if there isn't a user defined overide already present
						sip->subsystems[s].triggers[i].reverse_start = ani_time - animation_instance_actual_time(&sip->subsystems[s].triggers[i]);
					sip->subsystems[s].triggers[i].real_end_time = animation_instance_actual_time(&sip->subsystems[s].triggers[i]);
				}
			}
			//
		}
	}
}

//this tells you how long an animation is going to take to get done with
//this is for things that can't happen untill animations are done
//this is for gameplay purposes, this isn't the actual time
int ship_get_animation_time_type(ship *shipp, int animation_type, int subtype){

	ship_subsys	*pss;
	model_subsystem	*psub;
	int ret = 0;
	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects
		if ( pss->current_hits <= 0.0f ) 
			continue;
		if(psub->flags & MSS_FLAG_TRIGGERED){
			for(int i = 0; i<psub->n_triggers; i++){
				if(psub->triggers[i].type == animation_type && (psub->triggers[i].subtype == ANIMATION_SUBTYPE_ALL || psub->triggers[i].subtype == subtype)){

					int ani_time = 0;

					if(pss->trigger.current_vel.a1d[0] != 0.0f || pss->trigger.current_vel.a1d[1] != 0.0f || pss->trigger.current_vel.a1d[2] != 0.0f){
						//if the subobject is moveing then things get realy complecated

						int a_time = 0;
						int real_time = animation_instance_actual_time(&psub->triggers[i]);
						int pad = real_time - psub->triggers[i].end;
						for(int a = 0; a<3; a++){
							float direction = pss->trigger.direction.a1d[a];

							if(pss->trigger.current_ang.a1d[a] * direction > pss->trigger.slow_angle.a1d[a] * direction){
								//if it's in the final slowdown phase then it realy isn't _that_ bad
								a_time = int( ( (sqrt(2.0f*pss->trigger.rot_accel.a1d[a]*(pss->trigger.end_angle.a1d[a] - pss->trigger.current_ang.a1d[a])+pss->trigger.current_vel.a1d[a]*pss->trigger.current_vel.a1d[a])-pss->trigger.current_vel.a1d[a])/pss->trigger.rot_accel.a1d[a] ) * 1000.0f);
								if(ani_time < a_time)ani_time = a_time;
							}else{
								if(pss->trigger.current_vel.a1d[a] * direction > pss->trigger.rot_vel.a1d[a] * direction){
									//if vi is greater than v
									a_time = int((pss->trigger.current_vel.a1d[a]*(pss->trigger.current_vel.a1d[a]+2))/(2.0f*pss->trigger.rot_accel.a1d[a]) * 1000.0f);
									if(ani_time < a_time)ani_time = a_time;
								}else{
									//if vi  is lessthan or equil to v
									a_time = int( ((3.0f*pss->trigger.rot_vel.a1d[a])/(2.0f*pss->trigger.rot_accel.a1d[a]) + 
										(pss->trigger.current_vel.a1d[a]*pss->trigger.current_vel.a1d[a])/(4.0f*pss->trigger.rot_accel.a1d[a]*pss->trigger.rot_vel.a1d[a]) +
										(pss->trigger.end_angle.a1d[a] - pss->trigger.current_ang.a1d[a])/(pss->trigger.current_vel.a1d[a]*direction) -
										(pss->trigger.current_vel.a1d[a]/(pss->trigger.rot_accel.a1d[a]*direction))) * 1000.0f);
									if(ani_time < a_time)ani_time = a_time;
								}
							}
						}
						if(ani_time) ani_time += pad;
					}else{
						//if it isn't moveing then it's trivial
						//no currently playing animation
						ani_time = psub->triggers[i].end + psub->triggers[i].start;
					}
					if(ret < ani_time)ret = ani_time;
				}
			}
		}
		//
	}
	return ret + timestamp();

}

void ship_animation_set_initial_states(ship *shipp){

	ship_weapon	*swp = &shipp->weapons;
	int i;

	for(i = 0; i<MAX_SHIP_PRIMARY_BANKS;i++){
		swp->primary_animation_done_time[i] = 0;
	}
	for( i = 0; i<MAX_SHIP_SECONDARY_BANKS;i++){
		swp->secondary_animation_done_time[i] = 0;
	}

	ship_primary_changed(shipp);
	ship_secondary_changed(shipp);

	ship_subsys	*pss;
	model_subsystem	*psub;
	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

			for(int i = 0; i<psub->n_triggers; i++){
				if(psub->type == SUBSYSTEM_TURRET){
					//specal case for turrets
					pss->submodel_info_1.angs.h = psub->triggers[i].angle.xyz.y;
					pss->submodel_info_2.angs.p = psub->triggers[i].angle.xyz.x;
				}else{
					if(
						psub->triggers[i].type == TRIGGER_TYPE_INITIAL
						){
						pss->trigger.set_to_end(&psub->triggers[i]);
					}
				}
			}
		//

	}

}
