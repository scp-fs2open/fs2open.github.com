/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CMeasure/CMeasure.cpp $
 * $Revision: 2.6 $
 * $Date: 2005-04-05 05:53:15 $
 * $Author: taylor $
 *
 * Counter measures.  Created by Mike Kulas, May 12, 1997.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2004/07/26 20:47:25  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:43  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/03/18 10:07:00  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/09/24 18:56:41  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     9/05/99 11:25p Mikek
 * Debug code (only on NDEBUG).  Don't drop countermeasures if
 * Ai_firing_enabled not set.
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 44    4/29/98 9:36p Allender
 * ingame join tweaks.  added network message for countermeasures
 * 
 * 43    4/13/98 5:11p Mike
 * More improvement to countermeasure code.
 * 
 * 42    4/13/98 2:14p Mike
 * Countermeasure balance testing for Jim.
 * 
 * 41    4/10/98 11:02p Mike
 * Make countermeasures less effective against aspect seekers than against
 * heat seekers.
 * Make AI ships match bank with each other when attacking a faraway
 * target.
 * Make ships not do silly loop-de-loop sometimes when attacking a faraway
 * target.
 * 
 * 40    3/31/98 5:11p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 * 
 * 39    3/17/98 3:49p John
 * Turned off lighting on counter measures.
 * 
 * 38    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 37    2/23/98 5:44p Johnson
 * Resolve build error.
 * 
 * 36    2/23/98 4:30p Mike
 * Make homing missiles detonate after they pass up their target.  Make
 * countermeasures less effective.
 * 
 * 35    2/09/98 8:04p Lawrance
 * Add objnum to cmeasure struct, correctly set source_objnum
 * 
 * 34    2/05/98 11:20p Lawrance
 * save/restore countermeasure data
 * 
 * 33    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 32    2/04/98 12:20p Mike
 * Make countermeasures detonate in a smaller radius.  Make all in radius,
 * not just homing one, detonate.
 * 
 * 31    1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 30    1/29/98 11:11a John
 * Put in code to show dummy counter measure object.
 * 
 * 29    1/23/98 5:06p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 28    1/23/98 9:43a Mike
 * Debug-C to disallow countermeasure firing.
 * Fix bug in countermeasure tracking by aspect seekers.
 * 
 * 27    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 26    1/16/98 11:43a Mike
 * Fix countermeasures.
 * 
 * 25    12/30/97 6:44p John
 * Made g3_Draw_bitmap functions account for aspect of bitmap.
 * 
 * 24    11/29/97 2:05p John
 * made g3_draw_bitmap and g3_draw_rotated bitmap take w&h, not w/2 & h/2,
 * like they used to incorrectly assume.   Added code to model to read in
 * thruster radius's.
 * 
 * 23    10/27/97 3:24p Lawrance
 * ensure countermeasures keep their initial velocity
 * 
 * 22    9/14/97 4:50p Lawrance
 * added some demo debugging code
 * 
 * 21    9/04/97 5:09p Andsager
 * implement physics using moment of inertia and mass (from BSPgen).
 * Added to phys_info struct.  Updated ship_info, polymodel structs.
 * Updated weapon ($Mass and $Force) and ship ($Mass -> $Density) tables
 * 
 * 20    8/20/97 11:09a Mike
 * Make countermeasure lifetime based on skill level.
 * 
 * 19    8/13/97 9:50p Allender
 * split *_move into *_process_pre and *_process_post functions.
 * process_pre functions called before object is moved.  _process_post
 * functions called after object is moved.  Reordered code in ship_post
 * and weapon_post for multiplayer
 * 
 * 18    8/13/97 4:45p Allender
 * fixed ship_fire_primary and fire_secondary to not take parameter for
 * determining whether to count ammo or not.  Fixed countermeasure firing
 * for multiplayer
 * 
 * 17    8/13/97 12:06p Lawrance
 * supporting multiple types of fireball explosions
 * 
 * 16    8/11/97 6:03p Mike
 * Make ships with no shields not claim to have shields in target box.
 * 
 * 15    8/08/97 4:29p Allender
 * countermeasure stuff for multiplayer
 * 
 * 14    7/31/97 5:55p John
 * made so you pass flags to obj_create.
 * Added new collision code that ignores any pairs that will never
 * collide.
 * 
 * 13    7/15/97 12:03p Andsager
 * New physics stuff
 * 
 * 12    7/11/97 11:54a John
 * added rotated 3d bitmaps.
 * 
 * 11    6/24/97 10:04a Allender
 * major multiplayer improvements.  Better sequencing before game.
 * Dealing with weapon/fireball/counter measure objects between
 * client/host.  
 * 
 * 10    6/24/97 12:38a Lawrance
 * fix minor bug with cycling cmeasure
 * 
 * 9     6/05/97 1:37a Lawrance
 * change syntax of a snd_play()
 * 
 * 8     5/22/97 5:45p Mike
 * Better countermeasure firing, key off availability, specify in
 * weapons.tbl
 * 
 * 7     5/22/97 12:06p Lawrance
 * include Sound.h for playing sound fx
 * 
 * 6     5/22/97 12:04p Lawrance
 * added soundhook for cmeasure cycle
 * 
 * 5     5/15/97 5:05p Mike
 * In the midst of changing subsystem targetnig from type-based to
 * pointer-based.
 * Also allowed you to view a target while dead.
 * 
 * 4     5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 3     5/14/97 10:50a Mike
 * More countermeasure stuff.
 * 
 * 2     5/12/97 5:58p Mike
 * Add countermeasures.
 * 
 * 1     5/12/97 2:23p Mike
 *
 * $NoKeywords: $
 */

#include "globalincs/systemvars.h"
#include "cmeasure/cmeasure.h"
#include "freespace2/freespace.h"
#include "model/model.h"
#include "ship/ship.h"
#include "math/staticrand.h"
#include "object/object.h"



cmeasure_info Cmeasure_info[MAX_CMEASURE_TYPES];
cmeasure Cmeasures[MAX_CMEASURES];

int	Num_cmeasure_types = 0;
int	Num_cmeasures = 0;
int	Cmeasure_inited = 0;
int	Cmeasures_homing_check = 0;
int	Countermeasures_enabled = 1;			//	Debug, set to 0 means no one can fire countermeasures.

// This will get called at the start of each level.
void cmeasure_init()
{
	int i;

	if ( !Cmeasure_inited )
		Cmeasure_inited = 1;

	// Reset everything between levels
	Num_cmeasures = 0;

	for (i=0; i<MAX_CMEASURES; i++ )	{
		Cmeasures[i].subtype = CMEASURE_UNUSED;
	}
		
}

void cmeasure_render(object * objp)
{
	// JAS TODO: Replace with proper fireball
	cmeasure			*cmp;
	cmeasure_info	*cmip;
	
	cmp = &Cmeasures[objp->instance];
	cmip = &Cmeasure_info[cmp->subtype];

	if ( cmp->subtype == CMEASURE_UNUSED )	{
		Int3();	//	Hey, what are we doing in here?
		return;
	}

//	float				size = -1.0f;
//	vertex			p;
//	g3_rotate_vertex(&p, &objp->pos );
//	if ( rand() > RAND_MAX/2 )	{
//		gr_set_color( 255, 0, 0 );
//	} else {
//		gr_set_color( 255, 255, 255 );
//	}
//	g3_draw_sphere(&p, 100.0f );
	
	if ( cmip->model_num > -1 )	{
		model_clear_instance(cmip->model_num);
		model_render(cmip->model_num, &objp->orient, &objp->pos, MR_NO_LIGHTING );
	} else {
		mprintf(( "Not rendering countermeasure because model_num is negative\n" ));
	}


/*
	// JAS TODO: Replace with proper fireball
	int				framenum = -1;
	float				size = -1.0f;
	vertex			p;
	cmeasure			*cmp;

	fireball_data	*fd;

	cmp = &Cmeasures[objp->instance];
	fd = &Fireball_data[FIREBALL_SHIP_EXPLODE1];

	switch (cmp->subtype) {
	case CMEASURE_UNUSED:
		Int3();	//	Hey, what are we doing in here?
		break;
	default:
		framenum = (int) (fd->num_frames * Cmeasures[objp->instance].lifeleft*4) % fd->num_frames;
		size = objp->radius;
		break;
	}

	Assert(framenum != -1);
	Assert(size != -1.0f);

	gr_set_bitmap(fd->bitmap_id + framenum);
	g3_rotate_vertex(&p, &objp->pos );
	g3_draw_bitmap(&p, 0, size*0.5f, TMAP_FLAG_TEXTURED );
*/
}

void cmeasure_delete( object * objp )
{
	int num;

	num = objp->instance;

//	Assert( Cmeasures[num].objnum == OBJ_INDEX(objp));

	Cmeasures[num].subtype = CMEASURE_UNUSED;
	Num_cmeasures--;
	Assert( Num_cmeasures >= 0 );
}

// broke cmeasure_move into two functions -- process_pre and process_post (as was done with
// all *_move functions).  Nothing to do for process_pre

void cmeasure_process_pre( object *objp, float frame_time)
{
}

void cmeasure_process_post(object * objp, float frame_time)
{
	int num;
	num = objp->instance;
	
//	Assert( Cmeasures[num].objnum == objnum );
	cmeasure *cmp = &Cmeasures[num];

	if ( cmp->lifeleft >= 0.0f) {
		cmp->lifeleft -= frame_time;
		if ( cmp->lifeleft < 0.0f )	{
			objp->flags |= OF_SHOULD_BE_DEAD;
//			demo_do_flag_dead(OBJ_INDEX(objp));
		}
	}

}

float Skill_level_cmeasure_life_scale[NUM_SKILL_LEVELS] = {3.0f, 2.0f, 1.5f, 1.25f, 1.0f};

// creates one countermeasure.  A ship fires 1 of these per launch.  rand_val is used
// in multiplayer.  If -1, then create a random number.  If non-negative, use this
// number for static_rand functions
int cmeasure_create( object * source_obj, vec3d * pos, int cm_type, int rand_val )
{
	int		n, objnum, parent_objnum, arand;
	object	* obj;
	ship		*shipp;
	cmeasure	*cmp;
	cmeasure_info	*cmeasurep;

#ifndef NDEBUG
	if (!Countermeasures_enabled || !Ai_firing_enabled)
		return -1;
#endif

	Cmeasures_homing_check = 2;		//	Tell homing code to scan everything for two frames.  If only one frame, get sync problems due to objects being created at end of frame!

	parent_objnum = OBJ_INDEX(source_obj);

	Assert( source_obj->type == OBJ_SHIP );	
	Assert( source_obj->instance >= 0 && source_obj->instance < MAX_SHIPS );	
	
	shipp = &Ships[source_obj->instance];

	if ( Num_cmeasures >= MAX_CMEASURES)
		return -1;

	for (n=0; n<MAX_CMEASURES; n++ )	
		if ( Cmeasures[n].subtype == CMEASURE_UNUSED)
			break;
	if ( n == MAX_CMEASURES)
		return -1;

	nprintf(("Network", "Cmeasure created by %s\n", Ships[source_obj->instance].ship_name));

	cmp = &Cmeasures[n];
	cmeasurep = &Cmeasure_info[cm_type];

	if ( pos == NULL )
		pos = &source_obj->pos;

	objnum = obj_create( OBJ_CMEASURE, parent_objnum, n, &source_obj->orient, pos, 1.0f, OF_RENDERS | OF_PHYSICS );
	
	Assert( objnum >= 0 && objnum < MAX_OBJECTS );

	// Create Debris piece n!
	if ( rand_val == -1 )
		arand = myrand();				// use a random number to get lifeleft, and random vector for displacement from ship
	else
		arand = rand_val;

	cmp->lifeleft = static_randf(arand) * (cmeasurep->life_max - cmeasurep->life_min) / cmeasurep->life_min;
	if (source_obj->flags & OF_PLAYER_SHIP){
		cmp->lifeleft *= Skill_level_cmeasure_life_scale[Game_skill_level];
	}
	cmp->lifeleft = cmeasurep->life_min + cmp->lifeleft * (cmeasurep->life_max - cmeasurep->life_min);

	//	cmp->objnum = objnum;
	cmp->team = shipp->team;
	cmp->subtype = cm_type;
	cmp->objnum = objnum;
	cmp->source_objnum = parent_objnum;
	cmp->source_sig = Objects[objnum].signature;

	cmp->flags = 0;

	nprintf(("Jim", "Frame %i: Launching countermeasure #%i\n", Framecount, Objects[objnum].signature));

	obj = &Objects[objnum];
	
	Num_cmeasures++;

	vec3d vel, rand_vec;

	vm_vec_scale_add(&vel, &source_obj->phys_info.vel, &source_obj->orient.vec.fvec, -25.0f);

	static_randvec(arand+1, &rand_vec);

	vm_vec_scale_add2(&vel, &rand_vec, 2.0f);

	obj->phys_info.vel = vel;

	vm_vec_zero(&obj->phys_info.rotvel);

	// blow out his reverse thrusters. Or drag, same thing.
	obj->phys_info.rotdamp = 10000.0f;
	obj->phys_info.side_slip_time_const = 10000.0f;

	vm_vec_zero(&obj->phys_info.max_vel);		// make so he can't turn on his own VOLITION anymore.
	obj->phys_info.max_vel.xyz.z = -25.0f;
	vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.max_vel.xyz.z );

	vm_vec_zero(&obj->phys_info.max_rotvel);	// make so he can't change speed on his own VOLITION anymore.

//	obj->phys_info.flags |= PF_USE_VEL;

	return arand;										// need to return this value for multiplayer purposes
}

void cmeasure_select_next(object *objp)
{
	ship	*shipp;

	Assert(objp->type == OBJ_SHIP);

	shipp = &Ships[objp->instance];
	shipp->current_cmeasure++;

	if (shipp->current_cmeasure >= Num_cmeasure_types)
		shipp->current_cmeasure = 0;

	//snd_play( &Snds[SND_CMEASURE_CYCLE] );

	mprintf(("Countermeasure type set to %i in frame %i\n", shipp->current_cmeasure, Framecount));
}

