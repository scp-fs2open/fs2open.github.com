/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/ObjectSort.cpp $
 * $Revision: 2.16 $
 * $Date: 2007-02-18 06:17:10 $
 * $Author: Goober5000 $
 *
 * Sorting code for objects.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.14  2006/05/27 16:59:05  taylor
 * comment out some code which used only if neither D3D nor OGL
 *
 * Revision 2.13  2006/05/13 07:09:25  taylor
 * minor cleanup and a couple extra error checks
 * get rid of some wasteful math from the gr_set_proj_matrix() calls
 *
 * Revision 2.12  2006/04/12 22:23:41  taylor
 * compiler warning fixes to make GCC 4.1 shut the hell up
 *
 * Revision 2.11  2006/04/03 07:48:03  wmcoolmon
 * Miscellaneous minor changes, mostly related to addition of Current_camera variable
 *
 * Revision 2.10  2005/12/29 08:08:39  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.9  2005/05/24 20:55:21  taylor
 * make sure batch rendering of lasers happens after fog is disabled in nebula missions
 *
 * Revision 2.8  2005/04/05 05:53:22  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.7  2005/03/25 06:57:37  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.6  2005/03/16 01:35:59  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.5  2004/07/26 20:47:45  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/10/14 17:39:17  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 11    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 10    2/08/99 5:07p Dave
 * FS2 chat server support. FS2 specific validated missions.
 * 
 * 9     12/10/98 1:25p Dan
 * Fixed fogging in fred.
 * 
 * 8     12/10/98 11:16a Dan
 * Fixed problem where Fred tries to fog objects (which it can't because
 * it runs in the software renderer).
 * 
 * 7     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 6     12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 5     12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 4     12/06/98 6:53p Dave
 * 
 * 3     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 39    3/12/98 1:24p Mike
 * When weapons linked, increase firing delay.
 * Free up weapon slots for AI ships, if necessary.
 * Backside render shield effect.
 * Remove shield hit triangle if offscreen.
 * 
 * 38    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 37    2/13/98 2:48p Lawrance
 * Don't modify max_z position for shockwaves
 * 
 * 36    1/15/98 2:16p John
 * Made bitmaps zbuffer at center minus radius.
 * Made fireballs sort after objects they are around.
 * 
 * 35    10/24/97 7:53p John
 * added new code to detect if object is in view cone to fix clipping
 * problems once and for all.
 * 
 * 34    9/09/97 4:52p John
 * Almost done ship warp in code
 * 
 * 33    8/05/97 1:46p Mike
 * Comment out code that shows spheres where wingmen should be.
 * 
 * 32    7/16/97 5:51p Lawrance
 * make shockwaves translucent
 * 
 * 31    6/30/97 1:58p John
 * made obj_render_all only sort objects in view cone.  Made physics_pause
 * pause movement.
 * 
 * 30    6/27/97 4:44p John
 * changed sorting to be faster and sort by the real z depth (not the
 * rotated z) in preparation for speeding up collision detection by using
 * this.
 * 
 * 29    6/24/97 6:22p John
 * added detail flags.
 * sped up motion debris system a bit.
 * 
 * 28    6/20/97 4:54p John
 * added detail levels.  Started adding some code framework to optimize
 * many objects.
 * 
 * 27    6/19/97 6:21p John
 * optimized object sorting.
 * 
 * 26    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 25    5/02/97 10:57a John
 * moved non-ships zcheck out by radius.
 * 
 * 24    5/02/97 10:56a John
 * put back in code that draws smaller objects that are within a larger
 * object's radius after the large object.
 * 
 * 23    5/02/97 10:26a John
 * made non-ship objects use minimum z for z testing.
 * 
 * 22    3/26/97 12:38p Hoffoss
 * JAS: made object sorting look at gr_zbuffering instead of
 * gr_zbufferinging
 * 
 * 21    3/12/97 9:25a John
 * fixed a bug with zbuffering.  Reenabled it by default.
 * 
 * 20    3/10/97 6:21p John
 * Simplified rendering code a bit.
 * 
 * 19    3/06/97 5:07p Mike
 * turned in full zbuffering for now
 * 
 * 18    3/04/97 12:09a Mike
 * Clean up code.  Make External_view_mode well-behaved.  Add
 * aip->repair_objnum to save/restore.  Add restore_int_if() in state.cpp.
 *
 * $NoKeywords: $
 */


#include "object/object.h"
#include "render/3d.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "ship/ship.h"



typedef struct sorted_obj {
	object			*obj;					// a pointer to the original object
	float				z, min_z, max_z;	// The object's z values relative to viewer
} sorted_obj;

int Num_sorted_objects = 0;
sorted_obj Sorted_objects[MAX_OBJECTS];
int Object_sort_order[MAX_OBJECTS];


// Used to (fairly) quicky find the 8 extreme
// points around an object.
vec3d check_offsets[8] = { 
  { { { -1.0f, -1.0f, -1.0f } } },
  { { { -1.0f, -1.0f,  1.0f } } },
  { { { -1.0f,  1.0f, -1.0f } } },
  { { { -1.0f,  1.0f,  1.0f } } },
  { { {  1.0f, -1.0f, -1.0f } } },
  { { {  1.0f, -1.0f,  1.0f } } },
  { { {  1.0f,  1.0f, -1.0f } } },
  { { {  1.0f,  1.0f,  1.0f } } }
};

// See if an object is in the view cone.
// Returns:
// 0 if object isn't in the view cone
// 1 if object is in cone 
// This routine could possibly be optimized.  Right now, for an
// offscreen object, it has to rotate 8 points to determine it's
// offscreen.  Not the best considering we're looking at a sphere.
int obj_in_view_cone( object * objp )
{
	int i;
	vec3d tmp,pt; 
	ubyte codes;

// Use this to hack out player for testing.
// if ( objp == Player_obj ) return 0;

// OLD INCORRECT CODE!!!
//	g3_rotate_vector(&tmp,&objp->pos);
//	codes=g3_code_vector_radius(&tmp, objp->radius);
//	if ( !codes )	{
//		return 1;		// center is in, so return 1
//	}
//	return 0;

// This I commented out because it will quickly out for
// objects in the center, but cause one more rotation
// for objects outside the center.  So I figured it
// would be best to slow down objects inside by a bit
// and not penalize the offscreen ones, which require
// 8 rotatations to throw out.
//	g3_rotate_vector(&tmp,&objp->pos);
//	codes=g3_code_vector(&tmp);
//	if ( !codes )	{
//		//mprintf(( "Center is in, so render it\n" ));
//		return 1;		// center is in, so return 1
//	}

	// Center isn't in... are other points?

	ubyte and_codes = 0xff;

	for (i=0; i<8; i++ )	{
		vm_vec_scale_add( &pt, &objp->pos, &check_offsets[i], objp->radius );
		g3_rotate_vector(&tmp,&pt);
		codes=g3_code_vector(&tmp);
		if ( !codes )	{
			//mprintf(( "A point is inside, so render it.\n" ));
			return 1;		// this point is in, so return 1
		}
		and_codes &= codes;
	}

	if (and_codes)	{
		//mprintf(( "All points offscreen, so don't render it.\n" ));
		return 0;	//all points off screen
	}

	//mprintf(( "All points inside, so render it, but doesn't need clipping.\n" ));
	return 1;	
}


// Sorts all the objects by Z and renders them
extern int Fred_active;
extern int Cmdline_nohtl;
void obj_render_all(void (*render_function)(object *objp), bool *draw_viewer_last )
{
	object *objp;
	int i, j, incr;
	float fog_near, fog_far;
#ifdef DYN_CLIP_DIST
	float closest_obj = Max_draw_distance;
	float farthest_obj = Min_draw_distance;
#endif

	objp = Objects;
	Num_sorted_objects = 0;
	for (i=0;i<=Highest_object_index;i++,objp++) {
		if ( (objp->type != OBJ_NONE) && (objp->flags&OF_RENDERS) )	{
			objp->flags &= ~OF_WAS_RENDERED;

			if ( obj_in_view_cone(objp) )	{
				sorted_obj * osp = &Sorted_objects[Num_sorted_objects];
				Object_sort_order[Num_sorted_objects] = Num_sorted_objects;
				Num_sorted_objects++;

				osp->obj = objp;
				vec3d to_obj;
				vm_vec_sub( &to_obj, &objp->pos, &Eye_position );
				osp->z = vm_vec_dot( &Eye_matrix.vec.fvec, &to_obj );
/*
				if ( objp->type == OBJ_SHOCKWAVE )
					osp->z -= 2*objp->radius;
*/
				// Make warp in effect draw after any ship in it
				if ( objp->type == OBJ_FIREBALL )	{
					//if ( fireball_is_warp(objp) )	{
					osp->z -= 2*objp->radius;
					//}
				}
					
				osp->min_z = osp->z - objp->radius;
				osp->max_z = osp->z + objp->radius;
#ifdef DYN_CLIP_DIST
				if(objp != Viewer_obj)
				{
					if(osp->min_z < closest_obj)
						closest_obj = osp->min_z;
					if(osp->max_z > farthest_obj)
						farthest_obj = osp->max_z;
				}
#endif
			}
		}	
	}

	if(!Num_sorted_objects)
		return;

#ifdef DYN_CLIP_DIST
	if (!Cmdline_nohtl)
	{
		if(closest_obj < Min_draw_distance)
			closest_obj = Min_draw_distance;
		if(farthest_obj > Max_draw_distance)
			farthest_obj = Max_draw_distance;

		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, closest_obj, farthest_obj);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
#endif


	// Sort them by their maximum z value
	if ( Num_sorted_objects > 1 ) {
		incr = Num_sorted_objects / 2;
		while( incr > 0 )	{
			for (i=incr; i<Num_sorted_objects; i++ )	{
				j = i - incr; 
				while (j>=0 )	{
					// compare element j and j+incr
					if ( (Sorted_objects[Object_sort_order[j]].max_z < Sorted_objects[Object_sort_order[j+incr]].max_z)  ) {
						// If not in correct order, them swap 'em
						int tmp;
						tmp = Object_sort_order[j];	
						Object_sort_order[j] = Object_sort_order[j+incr];
						Object_sort_order[j+incr] = tmp;
						j -= incr;
					} else {
						break;
					}
				}
			}
			incr = incr / 2;
		}
	}

	gr_zbuffer_set( GR_ZBUFF_FULL );	

	// now draw them
 	for (i=0; i<Num_sorted_objects; i++)	{
		sorted_obj * os = &Sorted_objects[Object_sort_order[i]];
		os->obj->flags |= OF_WAS_RENDERED;
		//This is for ship cockpits. Bobb, feel free to optimize this any way you see fit
		if(os->obj == Viewer_obj
			&& os->obj->type == OBJ_SHIP
			&& (!Viewer_mode || (Viewer_mode & VM_PADLOCK_ANY) || (Viewer_mode & VM_OTHER_SHIP) || (Viewer_mode & VM_TRACK))
			&& (Ship_info[Ships[os->obj->instance].ship_info_index].flags2 & SIF2_SHOW_SHIP_MODEL))
		{
			(*draw_viewer_last) = true;
			continue;
		}

		// if we're fullneb, fire up the fog - this also generates a fog table
		if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !Fred_running){
			// get the fog values
			neb2_get_fog_values(&fog_near, &fog_far, os->obj);

			// only reset fog if the fog mode has changed - since regenerating a fog table takes
			// a bit of time
			if((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)){
		 		gr_fog_set(GR_FOGMODE_FOG, gr_screen.current_fog_color.red, gr_screen.current_fog_color.green, gr_screen.current_fog_color.blue, fog_near, fog_far);
			}

			// maybe skip rendering an object because its obscured by the nebula
			if(neb2_skip_render(os->obj, os->z)){
				continue;
			}
		}

		(*render_function)(os->obj);
	}

	//WMC - draw maneuvering thrusters
	extern void batch_render_man_thrusters();
	batch_render_man_thrusters();

	// if we're fullneb, switch off the fog effet
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

//	if(!Cmdline_nohtl)gr_set_lighting(false,false);
	// lasers have to be drawn without fog! - taylor
	batch_render_all();

/*	Show spheres where wingmen should be flying
	{
		extern void render_wing_phantoms_all();
		render_wing_phantoms_all();
	}
	*/
}

