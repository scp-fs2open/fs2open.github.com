/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/Object/DeadObjectDock.h $
 * $Revision: 1.1.2.3 $
 * $Date: 2006-06-27 04:10:23 $
 * $Author: Goober5000 $
 *
 * New docking system for dead objects, used only for death rolls
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.2.2  2006/06/27 04:08:42  Goober5000
 * cvs header tweak
 * --Goober5000
 *
 * Revision 1.1.2.1  2006/06/27 04:07:10  Goober5000
 * handle docked objects during death roll
 * --Goober5000
 *
 */

#ifndef _DEAD_OBJECT_DOCK_H
#define _DEAD_OBJECT_DOCK_H

#include "globalincs/globals.h"
#include "objectdock.h"


// get the first object in objp's dock list
object *dock_get_first_dead_docked_object(object *objp);

// add objp1 and objp2 to each others' dock lists; currently only called by ai_deathroll_start
void dock_dead_dock_objects(object *objp1, int dockpoint1, object *objp2, int dockpoint2);

// remove objp1 and objp2 from each others' dock lists; currently only called by do_dying_undock_physics
void dock_dead_undock_objects(object *objp1, object *objp2);

#endif	// _DEAD_OBJECT_DOCK_H
