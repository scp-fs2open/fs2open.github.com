/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Debris/Debris.cpp $
 * $Revision: 2.17 $
 * $Date: 2005-07-22 10:18:37 $
 * $Author: Goober5000 $
 *
 * Code for the pieces of exploding object debris.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.16  2005/07/13 02:50:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.15  2005/07/13 02:01:28  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.14  2005/07/13 00:44:21  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.13  2005/04/05 05:53:15  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.12  2005/03/27 12:28:32  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.11  2005/03/02 21:24:43  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.10  2005/01/30 09:27:39  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 2.9  2004/07/26 20:47:26  Kazan
 * remove MCD complete
 *
 * Revision 2.8  2004/07/12 16:32:44  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.7  2004/07/01 01:52:20  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.6  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * for #pragma warning disable to indicate the message being disabled
 * --Goober5000
 *
 * Revision 2.5  2004/03/05 09:01:59  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.4  2003/10/15 22:03:24  Kazan
 * Da Species Update :D
 *
 * Revision 2.3  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.2  2003/03/19 12:29:01  unknownplayer
 * Woohoo! Killed two birds with one stone!
 * Fixed the 'black screen around dialog boxes' problem and also the much more serious freezing problem experienced by Goober5000. It wasn't a crash, just an infinite loop. DX8 merge is GO! once again :)
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/13 21:43:37  mharris
 * A little more network and sound cleanup
 *
 * Revision 1.3  2002/05/10 20:42:43  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 16    8/01/99 1:13p Dave
 * Fixed objsnd problem with hull debris pieces.
 * 
 * 15    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 14    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 13    5/18/99 11:50a Andsager
 * Remove unused object type OBJ_GHOST_SAVE
 * 
 * 12    5/14/99 11:50a Andsager
 * Added vaporize for SMALL ships hit by HUGE beams.  Modified dying
 * frame.  Enlarged debris shards and range at which visible.
 * 
 * 11    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 10    2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 9     1/20/99 6:04p Dave
 * Another bit of stuff for beam weapons. Ships will properly use them
 * now, although they're really deadly.
 * 
 * 8     12/03/98 3:14p Andsager
 * Check in code that checks rotating submodel actually has ship subsystem
 * 
 * 7     11/19/98 11:07p Andsager
 * Check in of physics and collision detection of rotating submodels
 * 
 * 6     11/13/98 10:13a Andsager
 * simplify collision code
 * 
 * 5     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 4     10/23/98 1:11p Andsager
 * Make ship sparks emit correctly from rotating structures.
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 119   8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 118   5/03/98 5:41p Mike
 * Add Framecount to nprintf.
 * 
 * 117   4/15/98 10:00p Allender
 * make debris have own signature set.
 * 
 * 116   4/15/98 9:42a Adam
 * added 2 more explosion types (1, actually, but placeholder for 2)
 * 
 * 115   4/13/98 4:52p Allender
 * remove AI_frametime and us flFrametime instead.  Make lock warning work
 * in multiplayer for aspect seeking missiles.  Debris fixups
 * 
 * 114   4/10/98 12:16p Allender
 * fix ship hit kill and debris packets
 * 
 * 113   4/09/98 5:43p Allender
 * multiplayer network object fixes.  debris and self destructed ships
 * should all sync up.  Fix problem where debris pieces (hull pieces) were
 * not getting a net signature
 * 
 * 112   4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 111   3/31/98 5:11p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 * 
 * 110   3/26/98 5:21p John
 * Added new code to preload all bitmaps at the start of a level.
 * Commented it out, though.
 * 
 * 109   3/23/98 12:20p Andsager
 * Enable collision from rotation in ship_debris and ship_debris
 * collisions.
 * 
 * 108   3/19/98 12:09p John
 * Fixed a bug using 6 characters.   r_heavy was using local coordinates
 * instead of world so all asteroid-ship and debris-ship hitpos's were in
 * the wrong spot.  Ok, I rearreanged some code to make it clearer also.
 * 
 * 
 * 107   3/12/98 6:47p John
 * MAde arcs on debris further than objrad*50 not render.
 * 
 * 106   3/09/98 2:10p Andsager
 * Put in checks for debris (other) with excessive velocity.
 * 
 * 105   2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 104   2/22/98 12:19p John
 * Externalized some strings
 * 
 * 103   2/20/98 8:31p Lawrance
 * Add radius parm to sound_play_3d()
 * 
 * 102   2/10/98 6:43p Lawrance
 * Moved asteroid code to a separate lib.
 * 
 * 101   2/07/98 2:14p Mike
 * Improve asteroid splitting.  Add ship:asteroid collisions.  Timestamp
 * ship:debris collisions.
 * 
 * 100   2/06/98 7:45p John
 * Reversed order of asteroid models so blown up ones are smaller.  
 * 
 * 99    2/06/98 7:28p John
 * Made debris and particles not get created if > 200 m from eye.   Added
 * max_velocity to asteroid's physics info for aiding in throwing out
 * collision pairs.
 * 
 * 98    2/06/98 3:08p Mike
 * More asteroid stuff, including resolving conflicts between the two
 * asteroid_field structs!
 * 
 * 97    2/06/98 12:25a Mike
 * More asteroid stuff.
 * 
 * 96    2/05/98 9:41p Mike
 * Asteroid work, intermediate checkin to resolve compile errors.
 * 
 * 95    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 94    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 93    2/03/98 6:01p Andsager
 * Fix excessive rotvel in debris_create.  Check using physics function
 * check_rotvel_limit.
 * 
 * 92    2/03/98 11:14a Andsager
 * Temp check in to stop debris being created with excess rotvel
 * 
 * 91    2/02/98 4:45p Mike
 * Increase translational and rotational velocity imparted to debris
 * pieces at request of Adam.
 * 
 * 90    1/30/98 2:56p John
 * Made debris arcs jump around.  Made only 2/3 of the chunks have arcing
 * 
 * 89    1/30/98 11:48a John
 * Made debris arcs cast light.  Added sound effects for them.
 * 
 * 88    1/29/98 5:50p John
 * Made electrical arcing on debris pieces be persistent from frame to
 * frame
 * 
 * 87    1/29/98 8:39a Andsager
 * Changed mass and moment of intertia based area vs. volume
 * 
 * 86    1/27/98 11:02a John
 * Added first rev of sparks.   Made all code that calls model_render call
 * clear_instance first.   Made debris pieces not render by default when
 * clear_instance is called.
 * 
 * 85    1/24/98 4:49p Lawrance
 * Only delete hull piece if you can find an old one that isn't already
 * about to die.
 * 
 * 84    1/23/98 5:06p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * $NoKeywords: $
 */


#include "debris/debris.h"
#include "render/3d.h"
#include "fireball/fireballs.h"
#include "radar/radar.h"
#include "gamesnd/gamesnd.h"
#include "object/objectsnd.h"
#include "globalincs/linklist.h"
#include "particle/particle.h"
#include "freespace2/freespace.h"
#include "object/objcollide.h"
#include "io/timer.h"
#include "species_defs/species_defs.h"
#include "ship/ship.h"
#include "radar/radarsetup.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#endif




#define MAX_LIFE									10.0f
#define MIN_RADIUS_FOR_PERSISTANT_DEBRIS	50		// ship radius at which debris from it becomes persistant
#define DEBRIS_SOUND_DELAY						2000	// time to start debris sound after created

// limit the number of hull debris chunks that can exist.  
#define	MAX_HULL_PIECES		10
int		Num_hull_pieces;		// number of hull pieces in existance
debris	Hull_debris_list;		// head of linked list for hull debris chunks, for quick search

debris Debris[MAX_DEBRIS_PIECES];

int Num_debris_pieces = 0;
int Debris_inited = 0;

int Debris_model = -1;
int Debris_vaporize_model = -1;
int Debris_num_submodels = 0;

char Debris_texture_files[MAX_SPECIES][FILESPEC_LENGTH];


int Debris_textures[MAX_SPECIES];

#define	MAX_DEBRIS_DIST					10000.0f			//	Debris goes away if it's this far away.
#define	DEBRIS_DISTANCE_CHECK_TIME		(10*1000)		//	Check every 10 seconds.
#define	DEBRIS_INDEX(dp) (dp-Debris)

#define	MAX_SPEED_SMALL_DEBRIS		200					// maximum velocity of small debris piece
#define	MAX_SPEED_BIG_DEBRIS			150					// maximum velocity of big debris piece
#define	MAX_SPEED_CAPITAL_DEBRIS	100					// maximum velocity of capital debris piece
#define	DEBRIS_SPEED_DEBUG

// ---------------------------------------------------------------------------------------
// debris_start_death_roll()
//
//	Start the sequence of a piece of debris writhing in unholy agony!!!
//
static void debris_start_death_roll(object *debris_obj, debris *debris_p)
{
	if (debris_p->is_hull)	{
		// tell everyone else to blow up the piece of debris
#ifndef NO_NETWORK
		if( MULTIPLAYER_MASTER )
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_NUKE);
#endif

		int fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
		fireball_create( &debris_obj->pos, fireball_type, OBJ_INDEX(debris_obj), debris_obj->radius*1.75f);

		// only play debris destroy sound if hull piece and it has been around for at least 2 seconds
		if ( Missiontime > debris_p->time_started + 2*F1_0 ) {
			snd_play_3d( &Snds[SND_MISSILE_IMPACT1], &debris_obj->pos, &View_position, debris_obj->radius );
			
		}
	}

  	debris_obj->flags |= OF_SHOULD_BE_DEAD;
//	demo_do_flag_dead(OBJ_INDEX(debris_obj));
}

// ---------------------------------------------------------------------------------------
// debris_init()
//
// This will get called at the start of each level.
//
void debris_init()
{
	int i;

	if ( !Debris_inited ) {
		Debris_inited = 1;
	}

	Debris_model = -1;
	Debris_vaporize_model = -1;
	Debris_num_submodels = 0;
		
	// Reset everything between levels
	Num_debris_pieces = 0;
	for (i=0; i<MAX_DEBRIS_PIECES; i++ )	{
		Debris[i].flags = 0;
		Debris[i].sound_delay = 0;
	}
		
	Num_hull_pieces = 0;
	list_init(&Hull_debris_list);
}

// Page in debris bitmaps at level load
void debris_page_in()
{
	int i;

	Debris_model = model_load( NOX("debris01.pof"), 0, NULL );
	if (Debris_model >= 0)	{
		polymodel * pm;
		pm = model_get(Debris_model);
		Debris_num_submodels = pm->n_models;
	}

	Debris_vaporize_model = model_load( NOX("debris02.pof"), 0, NULL );

	for (i=0; i<True_NumSpecies; i++ )	{
		nprintf(( "Paging", "Paging in debris texture '%s'\n", Debris_texture_files[i] ));
		Debris_textures[i] = bm_load( Debris_texture_files[i] );
		if ( Debris_textures[i] < 0 ) { 
			Warning( LOCATION, "Couldn't load species %d debris\ntexture, '%s'\n", i, Debris_texture_files[i] );
		}
		bm_page_in_texture( Debris_textures[i] );
	}
	
}

MONITOR(NumSmallDebrisRend);
MONITOR(NumHullDebrisRend);

// ---------------------------------------------------------------------------------------
// debris_render()
//
//
void debris_render(object * obj)
{
	int			i, num, swapped;
	polymodel	*pm;
	debris		*db;


	swapped = -1;
	pm = NULL;	
	num = obj->instance;

	Assert(num >= 0 && num < MAX_DEBRIS_PIECES );
	db = &Debris[num];

	Assert( db->flags & DEBRIS_USED );

	// Swap in a different texture depending on the species
	if ( (db->species > -1) && (db->species < MAX_SPECIES) )	{

		pm = model_get( db->model_num );

		if ( pm && (pm->n_textures == 1) ) {
			swapped = pm->textures[0];
			pm->textures[0] = Debris_textures[db->species];
		}
	}

	model_clear_instance( db->model_num );

	// Only render electrical arcs if within 500m of the eye (for a 10m piece)
	if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f )	{
		for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
			if ( timestamp_valid( db->arc_timestamp[i] ) )	{
				model_add_arc( db->model_num, db->submodel_num, &db->arc_pts[i][0], &db->arc_pts[i][1], MARC_TYPE_NORMAL );
			}
		}
	}

	if ( db->is_hull )	{
		MONITOR_INC(NumHullDebrisRend,1);
		submodel_render( db->model_num, db->submodel_num, &obj->orient, &obj->pos );
	} else {
		MONITOR_INC(NumSmallDebrisRend,1);
		submodel_render( db->model_num, db->submodel_num, &obj->orient, &obj->pos, MR_NO_LIGHTING );
	}

	if ((swapped!=-1) && pm)	{
		pm->textures[0] = swapped;
	}
}

// Removed the DEBRIS_EXPIRE flag, and remove item from Hull_debris_list
void debris_clear_expired_flag(debris *db)
{
	if ( db->flags & DEBRIS_EXPIRE ) {
		db->flags &= ~DEBRIS_EXPIRE;
		if ( db->is_hull ) {
			Num_hull_pieces--;
			list_remove(Hull_debris_list, db);
			Assert( Num_hull_pieces >= 0 );
		}
	}
}

// ---------------------------------------------------------------------------------------
// debris_delete()
//
// Delete the debris object.  This is only ever called via obj_delete().  Do not call directly.
// Use debris_start_death_roll() if you want to force a debris piece to die.
//
void debris_delete( object * obj )
{
	int		num;
	debris	*db;

	num = obj->instance;
	Assert( Debris[num].objnum == OBJ_INDEX(obj));

	db = &Debris[num];

	Assert( Num_debris_pieces >= 0 );
	if ( db->is_hull && (db->flags & DEBRIS_EXPIRE) ) {
		debris_clear_expired_flag(db);
	}

	db->flags = 0;
	Num_debris_pieces--;
}

//	If debris piece *db is far away from all players, make it go away very soon.
//	In single player game, delete if MAX_DEBRIS_DIST from player.
//	In multiplayer game, delete if MAX_DEBRIS_DIST from all players.
void maybe_delete_debris(debris *db)
{
	object	*objp;

	if (timestamp_elapsed(db->next_distance_check)) {
		if (!(Game_mode & GM_MULTIPLAYER)) {		//	In single player game, just check against player.
			if (vm_vec_dist_quick(&Player_obj->pos, &Objects[db->objnum].pos) > MAX_DEBRIS_DIST)
				db->lifeleft = 0.1f;
			else
				db->next_distance_check = timestamp(DEBRIS_DISTANCE_CHECK_TIME);
		} else {
			for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
				if (objp->flags & OF_PLAYER_SHIP) {
					if (vm_vec_dist_quick(&objp->pos, &Objects[db->objnum].pos) < MAX_DEBRIS_DIST) {
						db->next_distance_check = timestamp(DEBRIS_DISTANCE_CHECK_TIME);
						return;
					}
				}
			}
			db->lifeleft = 0.1f;
		}
	}
}

// broke debris_move into debris_process_pre and debris_process_post as was done with all
// *_move functions on 8/13 by MK and MA.
void debris_process_pre( object *objp, float frame_time)
{
}

MONITOR(NumSmallDebris);
MONITOR(NumHullDebris);

// ---------------------------------------------------------------------------------------
// debris_process_post()
//
// Do various updates to debris:  check if time to die, start fireballs
//
// parameters:		obj			=>		pointer to debris object
//						frame_time	=>		time elapsed since last debris_move() called
//
//	Maybe delete debris if it's very far away from player.
void debris_process_post(object * obj, float frame_time)
{
	int i, num;
	num = obj->instance;

	int objnum = OBJ_INDEX(obj);
	Assert( Debris[num].objnum == objnum );
	debris *db = &Debris[num];

	if ( db->is_hull ) {
		MONITOR_INC(NumHullDebris,1);
		radar_plot_object( obj );

		if ( timestamp_elapsed(db->sound_delay) ) {
#ifndef NO_SOUND
			obj_snd_assign(objnum, SND_DEBRIS, &vmd_zero_vector, 0);
#endif
			db->sound_delay = 0;
		}
	} else {
		MONITOR_INC(NumSmallDebris,1);
	}

	if ( db->lifeleft >= 0.0f) {
		db->lifeleft -= frame_time;
		if ( db->lifeleft < 0.0f )	{
			debris_start_death_roll(obj, db);
		}
	}

	maybe_delete_debris(db);	//	Make this debris go away if it's very far away.

	// ================== DO THE ELECTRIC ARCING STUFF =====================
	if ( db->arc_frequency <= 0 )	{
		return;			// If arc_frequency <= 0, this piece has no arcs on it
	}

	if ( !timestamp_elapsed(db->fire_timeout) && timestamp_elapsed(db->next_fireball))	{

		// start the next fireball up in the next 50 - 100 ms
		//db->next_fireball = timestamp_rand(60,80);		

		db->next_fireball = timestamp_rand(db->arc_frequency,db->arc_frequency*2 );
		db->arc_frequency += 100;	

		if (db->is_hull)	{

			int n, n_arcs = ((rand()>>5) % 3)+1;		// Create 1-3 sparks

			vec3d v1, v2, v3, v4;
			submodel_get_two_random_points( db->model_num, db->submodel_num, &v1, &v2 );
			submodel_get_two_random_points( db->model_num, db->submodel_num, &v3, &v4 );

			n = 0;

			int a = 100, b = 1000;
			int lifetime = (myrand()%((b)-(a)+1))+(a);

			// Create the spark effects
			for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
				if ( !timestamp_valid( db->arc_timestamp[i] ) )	{
					//db->arc_timestamp[i] = timestamp_rand(400,1000);	// live up to a second
					db->arc_timestamp[i] = timestamp(lifetime);	// live up to a second

					switch( n )	{
					case 0:
						db->arc_pts[i][0] = v1;
						db->arc_pts[i][1] = v2;
						break;
					case 1:
						db->arc_pts[i][0] = v2;
						db->arc_pts[i][1] = v3;
						break;

					case 2:
						db->arc_pts[i][0] = v2;
						db->arc_pts[i][1] = v4;
						break;

					default:
						Int3();
					}
						
					n++;
					if ( n == n_arcs )
						break;	// Don't need to create anymore
				}
			}

	
			// rotate v2 out of local coordinates into world.
			// Use v2 since it is used in every bolt.  See above switch().
			vec3d snd_pos;
			vm_vec_unrotate(&snd_pos, &v2, &obj->orient);
			vm_vec_add2(&snd_pos, &obj->pos );

			//Play a sound effect
			if ( lifetime > 750 )	{
				// 1.00 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_05], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  500 )	{
				// 0.75 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_04], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  250 )	{
				// 0.50 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_03], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  100 )	{
				// 0.25 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_02], &snd_pos, &View_position, obj->radius );
			} else {
				// 0.10 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_01], &snd_pos, &View_position, obj->radius );
			}

		}



	}

	for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
		if ( timestamp_valid( db->arc_timestamp[i] ) )	{
			if ( timestamp_elapsed( db->arc_timestamp[i] ) )	{
				// Kill off the spark
				db->arc_timestamp[i] = timestamp(-1);
			} else {
				// Maybe move a vertex....  20% of the time maybe?
				int mr = myrand();
				if ( mr < RAND_MAX/5 )	{
					vec3d v1, v2;
					submodel_get_two_random_points( db->model_num, db->submodel_num, &v1, &v2 );
					db->arc_pts[i][mr % 2] = v1;
				}
			}
		}
	}

}

// ---------------------------------------------------------------------------------------
// debris_find_oldest()
//
// Locate the oldest hull debris chunk.  Search through the Hull_debris_list, which is a list
// of all the hull debris chunks.
//
int debris_find_oldest()
{
	int		oldest_index;
	fix		oldest_time;
	debris	*db;

	oldest_index = -1;
	oldest_time = 0x7fffffff;

	for ( db = GET_FIRST(&Hull_debris_list); db != END_OF_LIST(&Hull_debris_list); db = GET_NEXT(db) ) {
		if ( (db->time_started < oldest_time) && !(Objects[db->objnum].flags & OF_SHOULD_BE_DEAD) ) {
			oldest_index = DEBRIS_INDEX(db);
			oldest_time = db->time_started;
		}
	}

	return oldest_index;
}

#define	DEBRIS_ROTVEL_SCALE	5.0f
void calc_debris_physics_properties( physics_info *pi, vec3d *min, vec3d *max );
// ---------------------------------------------------------------------------------------
// debris_create()
//
// Create debris from an object
//
//	exp_force:	Explosion force, used to assign velocity to pieces.
//					1.0f assigns velocity like before.  2.0f assigns twice as much to non-inherited part of velocity
object *debris_create(object *source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, int hull_flag, float exp_force)
{
	int		i, n, objnum, parent_objnum;
	object	*obj;
	ship		*shipp;
	debris	*db;	
	polymodel *pm;
	int vaporize;

	parent_objnum = OBJ_INDEX(source_obj);

	Assert( (source_obj->type == OBJ_SHIP ) || (source_obj->type == OBJ_GHOST));
	Assert( source_obj->instance >= 0 && source_obj->instance < MAX_SHIPS );	
	shipp = &Ships[source_obj->instance];
	vaporize = (shipp->flags &SF_VAPORIZE);

	if ( !hull_flag )	{
		// Make vaporize debris seen from farther away
		float dist = vm_vec_dist_quick( pos, &Eye_position );
		if (vaporize) {
			dist /= 2.0f;
		}
		if ( dist > 200.0f ) {
			//mprintf(( "Not creating debris that is %.1f m away\n", dist ));
			return NULL;
		}
	}

	if ( hull_flag && (Num_hull_pieces >= MAX_HULL_PIECES ) ) {
		// cause oldest hull debris chunk to blow up
		n = debris_find_oldest();
		if ( n >= 0 ) {
			debris_start_death_roll(&Objects[Debris[n].objnum], &Debris[n] );
		}
	}

	for (n=0; n<MAX_DEBRIS_PIECES; n++ ) {
		if ( !(Debris[n].flags & DEBRIS_USED) )
			break;
	}

	if ( n == MAX_DEBRIS_PIECES ) {
		nprintf(("Warning","Frame %i: Could not create debris, no more slots left\n", Framecount));
		return NULL;
	}

	db = &Debris[n];

	// Create Debris piece n!
	if ( hull_flag ) {
		if (rand() < RAND_MAX/6)	// Make some pieces blow up shortly after explosion.
			db->lifeleft = 2.0f * ((float) myrand()/(float) RAND_MAX) + 0.5f;
		else
			db->lifeleft = -1.0f;		// large hull pieces stay around forever
	}	else {
		db->lifeleft = (i2fl(myrand())/i2fl(RAND_MAX))*2.0f+0.1f;
	}

	// increase lifetime for vaporized debris
	if (vaporize) {
		db->lifeleft *= 3.0f;
	}
	db->flags |= DEBRIS_USED;
	db->is_hull = hull_flag;
	db->source_objnum = parent_objnum;
	db->source_sig = source_obj->signature;
	db->ship_info_index = shipp->ship_info_index;
	db->team = shipp->team;
	db->fire_timeout = 0;	// if not changed, timestamp_elapsed() will return false
	db->time_started = Missiontime;
	db->species = Ship_info[shipp->ship_info_index].species;
	db->next_distance_check = (myrand() % 2000) + 4*DEBRIS_DISTANCE_CHECK_TIME;

	for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
		db->arc_timestamp[i] = timestamp(-1);
		//	vec3d	arc_pts[MAX_DEBRIS_ARCS][2];		// The endpoints of each arc
	}

	if ( db->is_hull )	{
		// Only make 1/2 of the pieces have arcs
		if ( myrand() < RAND_MAX*2/3 )	{
			db->arc_frequency = 1000;
		} else {
			db->arc_frequency = 0;
		}
	} else {
		db->arc_frequency = 0;
	}

	if ( model_num < 0 )	{
		if (vaporize) {
			db->model_num = Debris_vaporize_model;
		} else {
			db->model_num = Debris_model;
		}
		db->submodel_num = (myrand()>>4) % Debris_num_submodels;
	} else {
		db->model_num = model_num;
		db->submodel_num = submodel_num;
	}
	float radius = submodel_get_radius( db->model_num, db->submodel_num );

	db->next_fireball = timestamp_rand(500,2000);	//start one 1/2 - 2 secs later

	if ( pos == NULL )
		pos = &source_obj->pos;

	uint flags = OF_RENDERS | OF_PHYSICS;
	if ( hull_flag )	
		flags |= OF_COLLIDES;
	objnum = obj_create( OBJ_DEBRIS, parent_objnum, n, &source_obj->orient, pos, radius, flags );
	if ( objnum == -1 ) {
		mprintf(("Couldn't create debris object -- out of object slots\n"));
		return NULL;
	}

	db->objnum = objnum;
	
	obj = &Objects[objnum];

	// assign the network signature.  The signature will be 0 for non-hull pieces, but since that
	// is our invalid signature, it should be okay.
	obj->net_signature = 0;
#ifndef NO_NETWORK
	if ( (Game_mode & GM_MULTIPLAYER) && hull_flag ) {
		obj->net_signature = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
	}
#endif

	// -- No long need shield: bset_shield_strength(obj, 100.0f);		//	Hey!  Set to some meaningful value!

	if (source_obj->type == OBJ_SHIP) {
		obj->hull_strength = Ships[source_obj->instance].ship_max_hull_strength/8.0f;
	} else
		obj->hull_strength = 10.0f;

	Num_debris_pieces++;

	vec3d rotvel, radial_vel, to_center;

	if ( exp_center )
		vm_vec_sub( &to_center,pos, exp_center );
	else
		vm_vec_zero(&to_center);

	float scale;

	if ( hull_flag )	{
		float t;
		scale = exp_force * i2fl((myrand()%20) + 10);	// for radial_vel away from location of blast center
		db->sound_delay = timestamp(DEBRIS_SOUND_DELAY);

		// set up physics mass and I_inv for hull debris pieces
		pm = model_get(model_num);
		vec3d *min, *max;
		min = &pm->submodel[submodel_num].min;
		max = &pm->submodel[submodel_num].max;
		calc_debris_physics_properties( &obj->phys_info, min, max );

		// limit the amount of time that fireballs appear
		// let fireball length be linked to radius of ship.  Range is .33 radius => 3.33 radius seconds.
		t = 1000*Objects[db->source_objnum].radius/3 + myrand()%(fl2i(1000*3*Objects[db->source_objnum].radius));
		db->fire_timeout = timestamp(fl2i(t));		// fireballs last from 5 - 30 seconds
		
		if ( Objects[db->source_objnum].radius < MIN_RADIUS_FOR_PERSISTANT_DEBRIS ) {
			db->flags |= DEBRIS_EXPIRE;	// debris can expire
			Num_hull_pieces++;
			list_append(&Hull_debris_list, db);
		} else {
			nprintf(("Alan","A forever chunk of debris was created from ship with radius %f\n",Objects[db->source_objnum].radius));
		}
	}
	else {
		scale = exp_force * i2fl((myrand()%20) + 10);	// for radial_vel away from blast center (non-hull)
	}

	if ( vm_vec_mag_squared( &to_center ) < 0.1f )	{
		vm_vec_rand_vec_quick(&radial_vel);
		vm_vec_scale(&radial_vel, scale );
	}
	else {
		vm_vec_normalize(&to_center);
		vm_vec_copy_scale(&radial_vel, &to_center, scale );
	}

	//	MK: This next line causes debris pieces to get between 50% and 100% of the parent ship's
	//	velocity.  What would be very cool is if the rotational velocity of the parent would become 
	// translational velocity of the debris piece.  This would be based on the location of the debris
	//	piece in the parent object.

	// DA: here we need to vel_from_rot = w x to_center, where w is world is unrotated to world coords and offset is the 
	// displacement fromt the center of the parent object to the center of the debris piece

	vec3d world_rotvel, vel_from_rotvel;
	vm_vec_unrotate ( &world_rotvel, &source_obj->phys_info.rotvel, &source_obj->orient );
	vm_vec_crossprod ( &vel_from_rotvel, &world_rotvel, &to_center );
	vm_vec_scale ( &vel_from_rotvel, DEBRIS_ROTVEL_SCALE);

	vm_vec_add (&obj->phys_info.vel, &radial_vel, &source_obj->phys_info.vel);
	vm_vec_add2(&obj->phys_info.vel, &vel_from_rotvel);

#ifdef DEBRIS_SPEED_DEBUG
	// check that debris is not created with too high a velocity
	if (hull_flag) {
		int ship_info_flag = Ship_info[Ships[source_obj->instance].ship_info_index].flags;
		if (ship_info_flag & (SIF_SMALL_SHIP | SIF_NOT_FLYABLE | SIF_HARMLESS)) {
			if (vm_vec_mag_squared(&obj->phys_info.vel) > MAX_SPEED_SMALL_DEBRIS*MAX_SPEED_SMALL_DEBRIS) {
				float scale = MAX_SPEED_SMALL_DEBRIS / vm_vec_mag(&obj->phys_info.vel);
				vm_vec_scale(&obj->phys_info.vel, scale);
			}
		} else if (ship_info_flag & SIF_BIG_SHIP) {
			if (vm_vec_mag_squared(&obj->phys_info.vel) > MAX_SPEED_BIG_DEBRIS*MAX_SPEED_BIG_DEBRIS) {
				float scale = MAX_SPEED_BIG_DEBRIS / vm_vec_mag(&obj->phys_info.vel);
				vm_vec_scale(&obj->phys_info.vel, scale);
			}
		} else if (ship_info_flag & SIF_HUGE_SHIP) {
			if (vm_vec_mag_squared(&obj->phys_info.vel) > MAX_SPEED_CAPITAL_DEBRIS*MAX_SPEED_CAPITAL_DEBRIS) {
				float scale = MAX_SPEED_CAPITAL_DEBRIS / vm_vec_mag(&obj->phys_info.vel);
				vm_vec_scale(&obj->phys_info.vel, scale);
			}
		} else {
			Warning(LOCATION, "Ship has info flag that is not among the following:  SMALL, NOT_FLYABLE, HARMLESS, BIG, CAPITAL, SUPERCAP");
		}
	}
#endif

//	vm_vec_scale_add(&obj->phys_info.vel, &radial_vel, &source_obj->phys_info.vel, frand()/2.0f + 0.5f);
//	nprintf(("Andsager","object vel from rotvel: %0.2f, %0.2f, %0.2f\n",vel_from_rotvel.x, vel_from_rotvel.y, vel_from_rotvel.z));

// make sure rotational velocity does not get too high
	if (radius < 1.0) {
		radius = 1.0f;
	}

	scale = ( 6.0f + i2fl(myrand()%4) ) / radius;

	vm_vec_rand_vec_quick(&rotvel);
	vm_vec_scale(&rotvel, scale);

	obj->phys_info.flags |= PF_DEAD_DAMP;
	obj->phys_info.rotvel = rotvel;
	check_rotvel_limit( &obj->phys_info );


	// blow out his reverse thrusters. Or drag, same thing.
	obj->phys_info.rotdamp = 10000.0f;
	obj->phys_info.side_slip_time_const = 10000.0f;
	obj->phys_info.flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);	// set damping equal for all axis and not changable

	vm_vec_zero(&obj->phys_info.max_vel);		// make so he can't turn on his own VOLITION anymore.
	vm_vec_zero(&obj->phys_info.max_rotvel);	// make so he can't change speed on his own VOLITION anymore.


	// ensure vel is valid
	Assert( !vm_is_vec_nan(&obj->phys_info.vel) );

//	if ( hull_flag )	{
//		vm_vec_zero(&obj->phys_info.vel);
//		vm_vec_zero(&obj->phys_info.rotvel);
//	}

	return obj;
}

// ---------------------------------------------------------------------------------------
// debris_hit()
//
//	Alas, poor debris_obj got whacked.  Fortunately, we know who did it, where and how hard, so we
//	can do something about it.
//
void debris_hit(object *debris_obj, object *other_obj, vec3d *hitpos, float damage)
{
	debris	*debris_p = &Debris[debris_obj->instance];


	// Do a little particle spark shower to show we hit
	{
		particle_emitter	pe;

		pe.pos = *hitpos;								// Where the particles emit from
		pe.vel = debris_obj->phys_info.vel;		// Initial velocity of all the particles

		vec3d tmp_norm;
		vm_vec_sub( &tmp_norm, hitpos, &debris_obj->pos );
		vm_vec_normalize_safe(&tmp_norm);
			
		pe.normal = tmp_norm;			// What normal the particle emit around
		pe.normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
		pe.min_rad = 0.20f;				// Min radius
		pe.max_rad = 0.40f;				// Max radius

		// Sparks for first time at this spot
		pe.num_low = 10;				// Lowest number of particles to create
		pe.num_high = 10;			// Highest number of particles to create
		pe.normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
		pe.min_vel = 0.0f;				// How fast the slowest particle can move
		pe.max_vel = 10.0f;				// How fast the fastest particle can move
		pe.min_life = 0.25f;			// How long the particles live
		pe.max_life = 0.75f;			// How long the particles live
		particle_emit( &pe, PARTICLE_FIRE, 0 );
	}

#ifndef NO_NETWORK
	// multiplayer clients bail here
	if(MULTIPLAYER_CLIENT){
		return;
	}
#endif

	if ( damage < 0.0f ) {
		damage = 0.0f;
	}

	debris_obj->hull_strength -= damage;

	if (debris_obj->hull_strength < 0.0f) {
		debris_start_death_roll(debris_obj, debris_p );
	} else {
		// otherwise, give all the other players an update on the debris
#ifndef NO_NETWORK
		if(MULTIPLAYER_MASTER){
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_UPDATE);
		}
#endif
	}
}

// ---------------------------------------------------------------------------------------
// debris_check_collision()
//
//	See if poor debris object *obj got whacked by evil *other_obj at point *hitpos.
// NOTE: debris_hit_info pointer NULL for debris:weapon collision, otherwise debris:ship collision.
//	Return true if hit, else return false.
//
#pragma warning ( push )
#pragma warning ( disable : 4701 )	// possible use of variable without initialization
int debris_check_collision(object *pdebris, object *other_obj, vec3d *hitpos, collision_info_struct *debris_hit_info)
{
	mc_info	mc;
	int		num;

	Assert( pdebris->type == OBJ_DEBRIS );

	num = pdebris->instance;
	Assert( num >= 0 );

	Assert( Debris[num].objnum == OBJ_INDEX(pdebris));	

	// debris_hit_info NULL - so debris-weapon collision
	if ( debris_hit_info == NULL ) {
		// debris weapon collision
		Assert( other_obj->type == OBJ_WEAPON );
		mc.model_num = Debris[num].model_num;	// Fill in the model to check
		mc.submodel_num = Debris[num].submodel_num;
		model_clear_instance( mc.model_num );
		mc.orient = &pdebris->orient;					// The object's orient
		mc.pos = &pdebris->pos;							// The object's position
		mc.p0 = &other_obj->last_pos;				// Point 1 of ray to check
		mc.p1 = &other_obj->pos;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL);

		if (model_collide(&mc)) {
			*hitpos = mc.hit_point_world;
		}

		return mc.num_hits;
	}
	
	// debris ship collision -- use debris_hit_info to calculate physics
	object *ship_obj = other_obj;
	Assert( ship_obj->type == OBJ_SHIP );

	object *heavy = debris_hit_info->heavy;
	object *light = debris_hit_info->light;
	object *heavy_obj = heavy;
	object *light_obj = light;

	vec3d zero, p0, p1;
	vm_vec_zero(&zero);
	vm_vec_sub(&p0, &light->last_pos, &heavy->last_pos);
	vm_vec_sub(&p1, &light->pos, &heavy->pos);

	mc.pos = &zero;								// The object's position
	mc.p0 = &p0;									// Point 1 of ray to check
	mc.p1 = &p1;									// Point 2 of ray to check

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
		
	// Collision detection from rotation enabled if at rotaion is less than 30 degree in frame
	// This should account for all ships
	if ( (vm_vec_mag_squared(&heavy->phys_info.rotvel) * flFrametime*flFrametime) < (PI*PI/36) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		debris_hit_info->collide_rotate = 1;
		vm_vec_rotate(&p0_temp, &p0, &heavy->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&debris_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&debris_hit_info->light_rel_vel, 1/flFrametime);
	} else {
		debris_hit_info->collide_rotate = 0;
		vm_vec_sub(&debris_hit_info->light_rel_vel, &light->phys_info.vel, &heavy->phys_info.vel);
	}

	int mc_ret_val = 0;

	if ( debris_hit_info->heavy == ship_obj ) {	// ship is heavier, so debris is sphere. Check sphere collision against ship poly model
		mc.model_num = Ships[ship_obj->instance].modelnum;		// Fill in the model to check
		mc.orient = &ship_obj->orient;								// The object's orient
		mc.radius = pdebris->radius;
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		// copy important data
		int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
		vec3d copy_p0, copy_p1;
		copy_p0 = *mc.p0;
		copy_p1 = *mc.p1;

		// first test against the sphere - if this fails then don't do any submodel tests
		mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

		int submodel_list[MAX_ROTATING_SUBMODELS];
		int num_rotating_submodels = 0;
		polymodel *pm;

		ship_model_start(ship_obj);

		if (model_collide(&mc)) {

			// Set earliest hit time
			debris_hit_info->hit_time = FLT_MAX;

			// Do collision the cool new way
			if ( debris_hit_info->collide_rotate ) {
				// We collide with the sphere, find the list of rotating submodels and test one at a time
				model_get_rotating_submodel_list(submodel_list, &num_rotating_submodels, heavy_obj);

				// Get polymodel and turn off all rotating submodels, collide against only 1 at a time.
				pm = model_get(Ships[heavy_obj->instance].modelnum);

				// turn off all rotating submodels and test for collision
				int i;
				for (i=0; i<num_rotating_submodels; i++) {
					pm->submodel[submodel_list[i]].blown_off = 1;
				}

				// reset flags to check MC_CHECK_MODEL | MC_CHECK_SPHERELINE and maybe MC_CHECK_INVISIBLE_FACES and MC_SUBMODEL_INSTANCE
				mc.flags = copy_flags | MC_SUBMODEL_INSTANCE;

				// check each submodel in turn
				for (i=0; i<num_rotating_submodels; i++) {
					// turn on submodel for collision test
					pm->submodel[submodel_list[i]].blown_off = 0;

					// set angles for last frame (need to set to prev to get p0)
					angles copy_angles = pm->submodel[submodel_list[i]].angs;

					// find the start and end positions of the sphere in submodel RF
					pm->submodel[submodel_list[i]].angs = pm->submodel[submodel_list[i]].sii->prev_angs;
					world_find_model_point(&p0, &light_obj->last_pos, pm, submodel_list[i], &heavy_obj->last_orient, &heavy_obj->last_pos);

					pm->submodel[submodel_list[i]].angs = copy_angles;
					world_find_model_point(&p1, &light_obj->pos, pm, submodel_list[i], &heavy_obj->orient, &heavy_obj->pos);

					mc.p0 = &p0;
					mc.p1 = &p1;
					// mc.pos = zero	// in submodel RF

					mc.orient = &vmd_identity_matrix;
					mc.submodel_num = submodel_list[i];

					if ( model_collide(&mc) ) {
						if ( mc.hit_dist < debris_hit_info->hit_time ) {
							mc_ret_val = 1;

							// set up debris_hit_info common
							set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_ROT_HIT);
							model_find_world_point(&debris_hit_info->hit_pos, &mc.hit_point, mc.model_num, mc.hit_submodel, &heavy_obj->orient, &zero);

							// set up debris_hit_info for rotating submodel
							if (debris_hit_info->edge_hit == 0) {
								model_find_obj_dir(&debris_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
							}

							// find position in submodel RF of light object at collison
							vec3d int_light_pos, diff;
							vm_vec_sub(&diff, mc.p1, mc.p0);
							vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
							model_find_world_point(&debris_hit_info->light_collision_cm_pos, &int_light_pos, mc.model_num, mc.hit_submodel, &heavy_obj->orient, &zero);
						}
					}
					// Don't look at this submodel again
					pm->submodel[submodel_list[i]].blown_off = 1;
				}

			}

			// Recover and do usual ship_ship collision, but without rotating submodels
			mc.flags = copy_flags;
			*mc.p0 = copy_p0;
			*mc.p1 = copy_p1;
			mc.orient = &heavy_obj->orient;

			// usual ship_ship collision test
			if ( model_collide(&mc) )	{
				// check if this is the earliest hit
				if (mc.hit_dist < debris_hit_info->hit_time) {
					mc_ret_val = 1;

					set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

					// get collision normal if not edge hit
					if (debris_hit_info->edge_hit == 0) {
						model_find_obj_dir(&debris_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
					}

					// find position in submodel RF of light object at collison
					vec3d diff;
					vm_vec_sub(&diff, mc.p1, mc.p0);
					vm_vec_scale_add(&debris_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

				}
			}

			ship_model_stop( ship_obj );
		}

	} else {
		// Debris is heavier obj
		mc.model_num = Debris[num].model_num;		// Fill in the model to check
		mc.submodel_num = Debris[num].submodel_num;
		model_clear_instance( mc.model_num );
		mc.orient = &pdebris->orient;				// The object's orient
		mc.radius = model_get_core_radius( Ships[ship_obj->instance].modelnum );

		// check for collision between debris model and ship sphere
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_SPHERELINE);

		mc_ret_val = model_collide(&mc);

		if (mc_ret_val) {
			set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

			// set normal if not edge hit
			if ( !debris_hit_info->edge_hit ) {
				vm_vec_unrotate(&debris_hit_info->collision_normal, &mc.hit_normal, &heavy->orient);
			}

			// find position in submodel RF of light object at collison
			vec3d diff;
			vm_vec_sub(&diff, mc.p1, mc.p0);
			vm_vec_scale_add(&debris_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

		}
	}


	if ( mc_ret_val )	{

		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos
		// get heavy cm pos - already have light_cm_pos
		debris_hit_info->heavy_collision_cm_pos = zero;

		// get r_heavy and r_light
		debris_hit_info->r_heavy = debris_hit_info->hit_pos;
		vm_vec_sub(&debris_hit_info->r_light, &debris_hit_info->hit_pos, &debris_hit_info->light_collision_cm_pos);

		// set normal for edge hit
		if ( debris_hit_info->edge_hit ) {
			vm_vec_copy_normalize(&debris_hit_info->collision_normal, &debris_hit_info->r_light);
			vm_vec_negate(&debris_hit_info->collision_normal);
		}

		// get world hitpos
		vm_vec_add(hitpos, &debris_hit_info->heavy->pos, &debris_hit_info->r_heavy);

		return 1;
	} else {
		// no hit
		return 0;
	}
}
#pragma warning ( pop )


// ---------------------------------------------------------------------------------------
// debris_get_team()
//
//	Return the team field for a debris object
//
int debris_get_team(object *objp)
{
	Assert( objp->type == OBJ_DEBRIS );
	Assert( objp->instance >= 0 && objp->instance < MAX_DEBRIS_PIECES );
	return Debris[objp->instance].team;
}

// fills in debris physics properties when created, specifically mass and moment of inertia
void calc_debris_physics_properties( physics_info *pi, vec3d *mins, vec3d *maxs )
{
	float dx, dy, dz, mass;
	dx = maxs->xyz.x - mins->xyz.x;
	dy = maxs->xyz.y - mins->xyz.y;
	dz = maxs->xyz.z - mins->xyz.z;

	// John, with new bspgen, just set pi->mass = mass
	mass = 0.12f * dx * dy * dz;
	pi->mass = (float) pow(mass, 0.6666667f) * 4.65f;

	pi->I_body_inv.vec.rvec.xyz.x = 12.0f / (pi->mass *  (dy*dy + dz*dz));
	pi->I_body_inv.vec.rvec.xyz.y = 0.0f;
	pi->I_body_inv.vec.rvec.xyz.z = 0.0f;

	pi->I_body_inv.vec.uvec.xyz.x = 0.0f;
	pi->I_body_inv.vec.uvec.xyz.y = 12.0f / (pi->mass *  (dx*dx + dz*dz));
	pi->I_body_inv.vec.uvec.xyz.z = 0.0f;

	pi->I_body_inv.vec.fvec.xyz.x = 0.0f;
	pi->I_body_inv.vec.fvec.xyz.y = 0.0f;
	pi->I_body_inv.vec.fvec.xyz.z = 12.0f / (pi->mass *  (dx*dx + dy*dy));
}


